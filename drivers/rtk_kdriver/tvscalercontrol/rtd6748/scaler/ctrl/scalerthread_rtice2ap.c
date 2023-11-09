
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
/*#include <unistd.h>*/
/*#include <fcntl.h>*/
/*#include <stdio.h>*/
/*#include <pthread.h>*/
/*#include <sys/ioctl.h>*/
/*#include <stdlib.h>*/
/*#include <string.h>*/
/*#include <assert.h>*/
#include <linux/version.h>
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
//#include <mach/RPCDriver.h>
#include <linux/pageremap.h>
/*#include <pthread.h>*/

#include <linux/kthread.h>  /* for threads*/
#include <linux/time.h>   /* for using jiffies*/

#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalercolorlib_IPQ.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv002.h>

#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/vip/di_ma.h>
#include <tvscalercontrol/vip/pcid.h>
#include <tvscalercontrol/vip/valc.h>
#include <tvscalercontrol/vip/still_logo_detect.h>


#include <tvscalercontrol/scaler/scalerthread_rtice2ap.h>
#include <scaler/vipCommon.h>
#include <scaler/vipRPCCommon.h>

#include <tvscalercontrol/vdc/video_factory.h>
#include <tvscalercontrol/vdc/yc_separation_vpq.h>
#include <tvscalercontrol/vdc/yc_separation_vpq_table.h>

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
	#include <scaler/scalerDrvCommon.h>
#else
	#include <scalercommon/scalerCommon.h>
	#include <scalercommon/scalerDrvCommon.h>
#endif

#include <tvscalercontrol/vdc/video.h>
#include "tvscalercontrol/vip/localcontrast.h"
#include "tvscalercontrol/vip/icm.h"
#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <scaler_vbedev.h>
#include <scaler_vpqdev_extern.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/vip/tc_hdr.h>
#include <tvscalercontrol/vip/frequency_detection.h>
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/ST2094.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/sys_reg_reg.h>
//#include <rbus/sw_def_vip_reg.h>

#include <tvscalercontrol/vip/ai_pq.h>
#include <rtk_kdriver/rtk_pwm.h>
#include <vgip_isr/scalerAI.h>
#include <memc_isr/Platform/memc_change_size.h>
#include "vgip_isr/scalerVIP.h"
#include "vgip_isr/scalerDCC.h"
#include <tvscalercontrol/vip/srnn.h>
#include <tvscalercontrol/vip/SIP.h>
#include <tvscalercontrol/vip/scalerPQMaskColorLib.h>
#include <tvscalercontrol/vip/scalerPQMaskCtrl.h>
#include "scaler_vpqmemcdev.h"
#include "tvscalercontrol/vip/GSR2.h"

/*#include "../../../rtice2ap_driver.h"*/
extern struct semaphore LC_Semaphore;
extern unsigned int vpq_project_id;
extern unsigned int vpqex_project_id;
extern unsigned char set_DCR_PWM_Enable;
extern unsigned short OETF_curve_ST2094[1025];	/* in ST2094.c*/
extern unsigned int knee_curve_ST2094[1025];	/* in ST2094.c*/
extern int y_film_result[78];
extern unsigned char Gloabal_SLD_En;
extern  int thl_count_static_time;
extern  int thl_stop_global_drop_count_frame;

extern UINT32 before_phyAddr ;
extern UINT32 after_phyAddr ;

extern int thl_temporal_dY;
extern int thl_counter_NEWS_case;
extern int SLD_cmd_2;

extern int cmd_dY_preCur_mouseCon;
extern int cmd_LC_period;
extern int cmd_thl_counter_UR_con;
extern int cmd_counter_frame_highY_blk;
extern int cmd_lock_fw_gain_frame;
extern int cmd_pixel_gain_step;
extern int cmd_thl_counter_blk_pre_cur_same_mouseCon;
extern int cmd_thl_error_time_box_logo_case;
extern int cmd_thl_counter_box_logo_case;
extern int cmd_thl_error_time_NEWS_case;
extern int cmd_thl_counter_NEWS_case;
extern int cmd_thl_error_time_noise_case;
extern int cmd_thl_counter_noise_case;
extern int cmd_thl_dC;
extern int cmd_thl_dUDL;
extern int cmd_rgb_box_leave_idx_delete_step;
extern int cmd_box_logo_pixel_gain;
extern int cmd_box_logo_maxGain_time;
extern int cmd_box_lock_frame;
extern void scalerAI_enableClock(int enable);

INT32 filename_memc = 0;
INT32 start_offset_byte = 0;

//extern  void dump_logo_detect_to_file(unsigned char start_byte);
//extern void dump_sld_calc_buffer_to_file(void);
extern void Scaler_Get_Logo_Detect_VirAddr(void);

#define printf(fmt, args...)	VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_ScalerCtrl_Rtice2ap_DEBUG,0,fmt,##args)

//unsigned char gVipDebugLogCtrls[256] = {[0 ... 255]=0xff};//Debug log controls for VIPprintf
unsigned char gVipDebugLogCtrls[256] = {//Debug log controls for VIPprintf
	/*  0*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	/* 16*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	/* 32*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	/* 48*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	/* 64*/ 0xfC, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	/* 80*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	/* 96*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	/*112*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	/*128*/ 0xff, 0xff, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00,
	/*144*/ 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00,
	/*160*/ 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00,
	/*176*/ 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00,
	/*192*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	/*208*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	/*224*/ 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	/*240*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

EXPORT_SYMBOL(gVipDebugLogCtrls);

unsigned int gDebugPrintDelayTime = 1;//Interval count for controling print freq of VIPprintf

#define SCALERLIB_DEBUG

#ifdef SCALERLIB_DEBUG
  #define SLRLIB_PRINTF(format, args...) 		printf(format, ##args)
#else
  #define SLRLIB_PRINTF(format, args...)
#endif

#define ICM_TBL_Buff_Num 2
//static char New_idx = -1;
static unsigned char New_idx = 0;//fix tjw
static unsigned int* tICM_buff[ICM_TBL_Buff_Num] = {NULL};
static unsigned char PQ_FLOW_Mode = 0;
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

#define MEMC_ENVIRONMENT_CHECK_NUM		(8*3)//ENVIRONMENT_CHECK_NUM
#define MEMC_AUTOTEST_INTERNAL_PTG_NUM	(7*3)//INTERNAL_AUTOTEST_NUM
#define MEMC_AUTOTEST_REAL_PATTERN_NUM	(1*3)//REALPATTERN_AUTOTEST_NUM
#define MEMC_AUTOREGRESSION_NUM			(4*3)//AUTOREGRESSION_TYPE_NUM

unsigned char HDR_3DLUT_getFromReg = 0;
/*NNSR*/
int nnsr_size = 0;
/******/
int rtice_set_IdxTable2Buff(unsigned short mode, unsigned short size, unsigned char *num_type, unsigned char *buf);	/*return idx table size*/
int rtice_SetGet_PQA_Table_Info(VIP_TOOL_ACCESS_INFO_Item mode, unsigned char isSet_Flag, unsigned short size, unsigned char *num_type, unsigned char *buf);	/*return idx table size*/
int rtice_SetGet_YUV2RGB_Table_Info(VIP_TOOL_ACCESS_INFO_Item mode, unsigned char isSet_Flag, unsigned short size,
	unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, SLR_VIP_TABLE *vipTable_ShareMem);	/*return idx table size*/
int rtice_Set_YUV2RGB_Sat(unsigned short satGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table);
int rtice_Set_YUV2RGB_Hue(unsigned short hueGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table);
int rtice_Set_YUV2RGB_Contrast(unsigned short ContrastGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table);
// lti, cti, move to vip table
//extern UINT8 VipPQ_GetNewDctiTableSize(void);
//extern UINT8 VipPQ_GetDltiTableSize(void);
//extern UINT8 VipPQ_GetINewDctiTableSize(void);
//extern UINT8 VipPQ_GetVctiTableSize(void);
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
extern UINT8 VipPQ_GetSU_table_8tap_TableSize(void);
extern UINT8 VipPQ_GetSU_table_6tap_TableSize(void);
extern UINT8 VipPQ_GetDirsu_TableSize(void);
extern void fwif_color_sld_still_image(void);
extern void Get_Environment_Check_Result(unsigned short *u16_result);
extern void Get_Internal_Autotest_Result(unsigned short *u16_result);
extern void Get_RealPattern_Autotest_Result(unsigned short *u16_result);
extern void Get_AutoRegression_Result(unsigned short *u16_result);


extern short sRGB_APPLY[3][3];		//20150506 roger for sRGB
extern unsigned short tYUV2RGB_COEF[YUV2RGB_INPUT_MODE_MAX][tUV2RGB_COEF_Items_Max]; /* get YUV2RGB matrix from misc, elieli*/
extern void drvif_module_film_read_result(void);

extern unsigned char SLD_SW_En;
extern unsigned char LSC_by_memc_logo_en;
extern int memc_logo_to_demura_wait_time;
extern int memc_logo_to_demura_drop_time;
extern int memc_logo_to_demura_drop_limit;
extern int memc_logo_to_demura_drop_limit_gain;
extern unsigned char memc_logo_filter_SLD_APL_SE_en;
extern int memc_logo_to_demura_drop_limit_gain_SP;
extern unsigned short SLD_APL_reset_cnt_th;
extern int memc_logo_to_demura_ascend_speed;
extern unsigned char memc_logo_read_en;
extern unsigned char logo_to_demura_en;
extern unsigned char memc_logo_fill_hole_en;
extern unsigned char memc_logo_filter_en;
extern unsigned int memc_logo_to_demura_update_cnt;
extern unsigned char* LD_virAddr;
extern unsigned char sld_ddr_offset_auto_get;
extern unsigned int sld_ddr_offset;
extern unsigned char SLD_BY_MEMC_DEMURA_LOGO_ONLY;
extern unsigned char memc_logo_LD_APL_check;
extern int memc_logo_to_demura_APL_max_th;
extern unsigned char fill_hole_th;
extern int target_offset_table[PQ_LEVEL_TYPE_NUM][PQ_SCENE_TYPE_NUM][PQ_IP_TYPE_NUM][PQ_TUNING_NUM];



/*rtice_ap_buffer = (char *)kmalloc(rtice_data_size, GFP_KERNEL);*/
#define	vip_malloc(x)	kmalloc(x, GFP_KERNEL)
#define	vip_free(x)	kfree(x)

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
{TOOLACCESS_ScaleUPH_8tap,					VIP_UINT8,	1,																			"ScaleUPH_8tap"						}, /**/
{TOOLACCESS_ScaleUPV_6tap,					VIP_UINT8,	1,																			"ScaleUPV"							}, /**/
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
{TOOLACCESS_VD_PQ_TableControl,					VIP_UINT8,	PRJ_TBL_ITEM_MAX,															"VD_PQ_TableControl"},

{TOOLACCESS_DCC_PQ_current_mean,				VIP_UINT32,	1,																			"DCC_PQ_current_mean" 			}, /* read only*/
{TOOLACCESS_DCC_PQ_current_dev,					VIP_UINT32,	1,																			"DCC_PQ_current_dev"			}, /* read only*/
{TOOLACCESS_DCC_PQ_Level,					VIP_UINT8,	1,																			"DCC_PQ_Level"					}, /**/
{TOOLACCESS_DCC_PQ_Cubic_index,					VIP_UINT8,	1,																			"DCC_PQ_Cubic_index" 			}, /**/
{TOOLACCESS_DCC_PQ_Cubic_low, 					VIP_UINT8,	1,																			"DCC_PQ_Cubic_low" 				}, /**/
{TOOLACCESS_DCC_PQ_Cubic_high,					VIP_UINT8,	1,																			"DCC_PQ_Cubic_high" 			}, /**/


/*{TOOLACCESS_DisplayInformation_table,			VIP_UINT32,	DisplayInfo_Coef_MAX,														"DisplayInformation_table"			}, */ /* 19*/
{TOOLACCESS_VIP_QUALITY_Coef,          			VIP_UINT8,	VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Coef_MAX,					"VIP_QUALITY_Coef"          		}, /* 20*/
{TOOLACCESS_VIP_QUALITY_Extend_Coef,   			VIP_UINT8,	VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend_Coef_MAX,					"VIP_QUALITY_Extend_Coef"   		}, /* 21*/
{TOOLACCESS_VIP_QUALITY_Extend2_Coef,  			VIP_UINT8,	VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend2_Coef_MAX,					"VIP_QUALITY_Extend2_Coef"  		}, /* 22*/
{TOOLACCESS_VIP_QUALITY_Extend3_Coef,  			VIP_UINT8,	VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend3_Coef_MAX,					"VIP_QUALITY_Extend3_Coef"  		}, /* 23*/
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
{TOOLACCESS_MEMC_ENVIRONMENT_CHECK, 		VIP_UINT16, MEMC_ENVIRONMENT_CHECK_NUM, 														"TOOLACCESS_MEMC_Environment_Check" 	},
{TOOLACCESS_MEMC_AUTOTEST_INTERNAL_PTG, 	VIP_UINT16, MEMC_AUTOTEST_INTERNAL_PTG_NUM, 													"TOOLACCESS_MEMC_AutoTest_InternalPTG"	},
{TOOLACCESS_MEMC_AUTOTEST_REAL_PATTERN, 	VIP_UINT16, MEMC_AUTOTEST_REAL_PATTERN_NUM, 													"TOOLACCESS_MEMC_AutoTest_RealPattern"	},
{TOOLACCESS_MEMC_AUTOREGRESSION,			VIP_UINT16, MEMC_AUTOREGRESSION_NUM,															"TOOLACCESS_MEMC_AutoRegression"		},

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
extern unsigned char vpq_HDR_ioctl_get_stop_run_by_idx(unsigned char cmd_idx);
extern void vpq_HDR_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop);
extern unsigned char vpq_dolbyHDR_ioctl_get_stop_run_by_idx(unsigned char cmd_idx);
extern void vpq_dolbyHDR_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop);

extern void scalerVIP_Set_pq_reset(unsigned char bEnable);

// 0622 lsy
// lesley 1024
//extern UINT8 vpq_stereo_face;
//extern int scalerAI_pq_mode_ctrl(unsigned char stereo_face, unsigned char dcValue);
// end lesley 1024
extern RTK_AI_PQ_mode aipq_mode;
extern RTK_AI_PQ_mode aipq_mode_pre;
extern int scalerAI_pq_mode_ctrl(RTK_AI_PQ_mode ai_pq_mode, unsigned char dcValue);
// end 0622 lsy
extern int force_scene;
// 0506 lsy
extern int force_sqm;
// end 0506 lsy

extern GAMUT_3D_LUT_17x17x17_T g_buf3DLUT_LGDB;
extern VIP_OPS_CTRL_Item OPS_CTRL_item;
extern unsigned char MEX_MODE_Force_En_All;
VIP_DRV_DataAccess_Debug vip_data_access_debug = {0,0,0,0,0,0,0,0,0,0};
unsigned char SLD_Hybrid_mode_logo_debug_show = 0;
unsigned char SLD_swap_location = 0;
extern unsigned char logo_flag_map_raw[192*540];
extern unsigned char logo_flag_map_no_fill_hole[481 * 271];
extern unsigned char logo_flag_map[481*271];
extern unsigned char logo_flag_map_buf[481*271];
extern short logo_demura_counter[481*271];
extern short demura_tbl[5][481*271];
extern short demura_apply[3][241*136];
extern unsigned int blk_apl_average[32*18];
extern unsigned int blk_apl_maxfilter[32*18];
extern unsigned int blk_apl_interp[481*271];

extern sld_work_debug_struct sld_work_debug;
extern sld_work_filter_struct sld_work_filter;

#ifdef CONFIG_CUSTOMER_TV002
FREQ_DET_RTICE g_Freq_Det_TV002_Save;
#endif
//extern unsigned char Scaler_MEMC_Set_Performance_Checking_Database_index;
extern unsigned short VIP_HDR_EOTF_ByLuminace_CTRL[2];
extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL;
int rtice_get_vip_table(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf)
{
	int ret;
	int i = 0;
	unsigned int temp_size = 0;
	int *pwINT32Temp = NULL;
	unsigned int *pwUINT32Temp = NULL;
	unsigned short *pwUINT16Temp = NULL;
	unsigned char *pUINT8Temp = NULL;
	unsigned short UINT16Temp = 0;
	short *pwINT16Temp = NULL;
	unsigned int pwCopyTemp=0;
	RPC_DCC_Advance_control_table *Advance_control_table_by_source = NULL;
	_clues *smartPic_clue = fwif_color_Get_SmartPic_clue();
	_RPC_clues *RPC_smartPic_clue = (_RPC_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);
	unsigned char bTemp = 0;
	unsigned char *buf2_tmp = NULL;
	int *pwIntTemp = NULL;
	int intCopyTemp = 0;
	unsigned int *buf_tmp = NULL;
	unsigned char *buf_tmp8 = NULL;
	//char *pUcTemp = NULL;
	unsigned int *pUiTemp = NULL;
	SLR_VIP_TABLE *vipTable_ShareMem = fwif_color_GetShare_Memory_VIP_TABLE_Struct();
	unsigned int size, start_addr;
	unsigned char curve_index;
	unsigned char crtl_table[13];
	unsigned char tablesize[21];
	unsigned int DCC_HISTO_MEAN_VALUE;
	_system_setting_info *system_setting_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *RPC_system_setting_info = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	VIP_DCC_AdaptCtrl_Level_Table *DCC_AdaptCtrl_Level_Table = NULL;
	DRV_TC_HDR_CTRL *pTC_HDR_CTRL=NULL;
	VIP_ST2094_CTRL *ST2094_CTRL = NULL;
	char item_name[10] = "";

	unsigned char HY,VY,HC,VC;
	ST2094_metadata *ST2094_metadata;
	static char OSD_HDRMODE_DEMO_init = 0;
	
	unsigned char u8_i;
	unsigned short u16_Environment_Check[MEMC_ENVIRONMENT_CHECK_NUM]={0};
	unsigned short u16_AutoTest_Internal_Result[MEMC_AUTOTEST_INTERNAL_PTG_NUM]={0};
	unsigned short u16_AutoTest_RealPatten_Result[MEMC_AUTOTEST_REAL_PATTERN_NUM]={0};
	unsigned short u16_AutoRegression_Result[MEMC_AUTOREGRESSION_NUM]={0};

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	int j = 0, k = 0, ofst = 0;
	unsigned int u32tmpValue = 0;
	PQMASK_COLOR_MGR_T *pPQMaskColorMgr;
	PQMASK_PERF_T *pPQMaskPerf;
	PQMASK_GLB_STATUS_T *pPQMaskGlbStatus;
	PQMASK_GLB_CTRL_T *pPQMaskGlbCtrl;
	PQMASK_BUFINFO_T *pPQMaskBufInfo;
	PQMASK_ONLINE_MONITOR_CTRL_T *pPQMaskMonitorCtrl;
	PQMASK_ONLINE_MONITOR_STATUS_T *pPQMaskMonitorStatus;

#endif

	stopUpdateFrequencyOffset = 180;

	if(gVip_Table == NULL)
		return -ID_SIZE_ERR;

	size = rtice_param.size;
	start_addr = rtice_param.saddr;
	
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

	case TOOLACCESS_ScaleUPH_8tap:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetScaleUPH_8tap();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_ScaleUPV_6tap:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetScaleUPV_6tap();
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

	case TOOLACCESS_VD_PQ_TableControl:
		if (size > PRJ_TBL_ITEM_MAX*sizeof(unsigned char))
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *)VPQ_PRJ_TABLE_SEL[drvif_module_vpq_get_ProjectId()], size);
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
		memcpy(buf, (unsigned char *)buf_tmp, size);

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
		memcpy(buf, (unsigned char *)buf_tmp, size);

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
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Coef_MAX)
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->VIP_QUALITY_Coef[0][0]), size);
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_VIP_QUALITY_Extend_Coef:       /*  21*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend_Coef_MAX)
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->VIP_QUALITY_Extend_Coef[0][0]), size);
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_VIP_QUALITY_Extend2_Coef:       /*  22*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend2_Coef_MAX)
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->VIP_QUALITY_Extend2_Coef[0][0]), size);
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_VIP_QUALITY_Extend3_Coef:       /*  23*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend3_Coef_MAX)
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->VIP_QUALITY_Extend3_Coef[0][0]), size);
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_tICM_ini:       /*  24*/
		if (size > sizeof(unsigned short)*VIP_ICM_TBL_X*VIP_ICM_TBL_Y*VIP_ICM_TBL_Z)
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->tICM_ini[0][0][0]), size);
		*num_type = VIP_UINT16;
		break;
	case TOOLACCESS_SHPTable:       /*  25*/
		if (size > sizeof(VIP_Sharpness_Table))
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->Ddomain_SHPTable[0]), size);
		*num_type = VIP_INT16;
		break;
	case TOOLACCESS_Hist_Y_Mean_Value:/* 26*/
		if (size > sizeof(unsigned int)*1)
			return -ID_SIZE_ERR;
		temp_size = 1;
		buf_tmp = &(smartPic_clue->Adapt_DCC_Info.Adapt_Mean_Dev_info[DCC_Tool_mean]);
		fwif_color_ChangeUINT32Endian(buf_tmp,temp_size, 1);
		memcpy(buf, (unsigned char *)buf_tmp, size);
		*num_type = VIP_UINT32;
		break;
	case TOOLACCESS_TABLE_NTSC_3D:/*  27*/
		if (size > sizeof(unsigned char)*VD_3D_TABLE_SIZE*VD_MOTION_STEP*2)
			return -ID_SIZE_ERR;
		ret = Scaler_SendRPC(SCALERIOC_SET_TABLE_NTSC_3D, 0, 0);
		if (0 != ret) {
			return READ_ERROR;
		}
		memcpy(buf, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_TABLE_NTSC_3D), size);
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_MANUAL_NR:/* 29*/
		if (size > sizeof(DRV_NR_Item)*PQA_TABLE_MAX*DRV_NR_Level_MAX)
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->Manual_NR_Table[0][0]), size);
		*num_type = VIP_UINT8;
		break;

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

		temp_size = 17;//(17 * sizeof(int))/sizeof(unsigned int);
		fwif_color_ChangeUINT32Endian(buf_tmp,temp_size, 1);
		memcpy(buf, (unsigned char *)buf_tmp, size);

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

		memcpy(buf_tmp, &(smartPic_clue->Y_Main_Hist_Ratio[0]), sizeof(int)*TV006_VPQ_chrm_bin);
		memcpy(buf_tmp+TV006_VPQ_chrm_bin, &(smartPic_clue->Hue_Main_His_Ratio[0]), sizeof(int)*COLOR_HUE_HISTOGRAM_LEVEL);
		memcpy(buf_tmp+TV006_VPQ_chrm_bin+COLOR_HUE_HISTOGRAM_LEVEL, &(smartPic_clue->Sat_Main_His_Ratio[0]), sizeof(int)*COLOR_AutoSat_HISTOGRAM_LEVEL);
		memcpy(buf_tmp+TV006_VPQ_chrm_bin+COLOR_HUE_HISTOGRAM_LEVEL+COLOR_AutoSat_HISTOGRAM_LEVEL, &(smartPic_clue->Hist_Y_Mean_Value), sizeof(int)*1);
		memcpy(buf_tmp+TV006_VPQ_chrm_bin+COLOR_HUE_HISTOGRAM_LEVEL+COLOR_AutoSat_HISTOGRAM_LEVEL+1, &(smartPic_clue->skintoneflag), sizeof(int)*1);
		memcpy(buf_tmp+TV006_VPQ_chrm_bin+COLOR_HUE_HISTOGRAM_LEVEL+COLOR_AutoSat_HISTOGRAM_LEVEL+2, &(smartPic_clue->PureColor), sizeof(int)*1);

		temp_size = 91;//(91 * sizeof(int))/sizeof(unsigned int);
		fwif_color_ChangeUINT32Endian(buf_tmp,temp_size, 1);
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

		memcpy(buf_tmp, &(smartPic_clue->VD_Signal_Status_value_avg), sizeof(int)*1);
		memcpy(buf_tmp+1, &(smartPic_clue->VD_Noise_level_compute), sizeof(int)*1);

		fwif_color_ChangeUINT32Endian(buf_tmp, 2, 1);
		memcpy(buf, (unsigned char *)buf_tmp, size);
		*num_type = VIP_UINT32;
		vip_free(buf_tmp);
		buf_tmp = NULL;
		break;

	case TOOLACCESS_ISR_Print_Ctrl:
		if (size > sizeof(VIP_ISR_Printf_Flag))
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *) &(system_setting_info->ISR_Printf_Flag), size);
		fwif_color_ChangeUINT32Endian((unsigned int *)buf, ISR_PRINT_CTRL_ITEM_NUMBER, 1);
		*num_type = VIP_UINT32;
		break;

		case TOOLACCESS_Debug_Buff_8:
			if(size > Debug_Buff_Num*sizeof(char))
				return -ID_SIZE_ERR;
			memcpy(buf,(unsigned char*)&(system_setting_info->Debug_Buff_8[0]), size);
			*num_type = VIP_UINT8;
			break;

		case TOOLACCESS_Debug_Buff_16:
			if(size > Debug_Buff_Num*sizeof(short))
				return -ID_SIZE_ERR;
			memcpy(buf,(unsigned char*)&(system_setting_info->Debug_Buff_16[0]), size);

			fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);

			*num_type = VIP_UINT16;
			break;

		case TOOLACCESS_Debug_Buff_32:
			if(size > Debug_Buff_Num*sizeof(int))
				return -ID_SIZE_ERR;
			memcpy(buf,(unsigned char*)&(system_setting_info->Debug_Buff_32[0]), size);

			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(int), 1);

			*num_type = VIP_UINT32;
			break;

		case TOOLACCESS_Debug_HDR:
			if(size > Debug_HDR_ITEM_NUMBER*sizeof(int))
				return -ID_SIZE_ERR;
			memcpy(buf,(unsigned char*)&(RPC_system_setting_info->HDR_info.EOTF_OETF[0]), size);

			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(int), 1);

			*num_type = VIP_UINT32;
			break;
			
		case TOOLACCESS_MEMC_ENVIRONMENT_CHECK:
			if(size > MEMC_ENVIRONMENT_CHECK_NUM*sizeof(unsigned short))
				return -ID_SIZE_ERR;

			Get_Environment_Check_Result((unsigned short*)u16_Environment_Check);

			// unsigned int to unsigned short
			for(u8_i=0;u8_i<size && u8_i<MEMC_ENVIRONMENT_CHECK_NUM;u8_i++){
				buf[2*u8_i] = (u16_Environment_Check[u8_i]>>8)&0xff;
				buf[2*u8_i+1] = (u16_Environment_Check[u8_i])&0xff;
			}
			
			*num_type = VIP_UINT16;
			break;

		case TOOLACCESS_MEMC_AUTOTEST_INTERNAL_PTG:
			if(size > MEMC_AUTOTEST_INTERNAL_PTG_NUM*sizeof(unsigned short))
				return -ID_SIZE_ERR;
			
			Get_Internal_Autotest_Result((unsigned short*)u16_AutoTest_Internal_Result);	

			// unsigned int to unsigned short
			for(u8_i=0;u8_i<size && u8_i<MEMC_AUTOTEST_INTERNAL_PTG_NUM;u8_i++){
				buf[2*u8_i] = (u16_AutoTest_Internal_Result[u8_i]>>8)&0xff;
				buf[2*u8_i+1] = (u16_AutoTest_Internal_Result[u8_i])&0xff;
			}

			*num_type = VIP_UINT16;
			break;
		case TOOLACCESS_MEMC_AUTOTEST_REAL_PATTERN:
			if(size > MEMC_AUTOTEST_REAL_PATTERN_NUM*sizeof(unsigned short))
				return -ID_SIZE_ERR;
			
			Get_RealPattern_Autotest_Result((unsigned short*)u16_AutoTest_RealPatten_Result);	

			// unsigned int to unsigned short
			for(u8_i=0;u8_i<size && u8_i<MEMC_AUTOTEST_REAL_PATTERN_NUM;u8_i++){
				buf[2*u8_i] = (u16_AutoTest_RealPatten_Result[u8_i]>>8)&0xff;
				buf[2*u8_i+1] = (u16_AutoTest_RealPatten_Result[u8_i])&0xff;
			}

			*num_type = VIP_UINT16;
			break;
			
		case TOOLACCESS_MEMC_AUTOREGRESSION:
			if(size > MEMC_AUTOREGRESSION_NUM*sizeof(unsigned short))
				return -ID_SIZE_ERR;
			
			Get_AutoRegression_Result((unsigned short*)u16_AutoRegression_Result);
			
			// unsigned int to unsigned short
			for(u8_i=0;u8_i<size && u8_i<MEMC_AUTOREGRESSION_NUM;u8_i++){
				buf[2*u8_i] = (u16_AutoRegression_Result[u8_i]>>8)&0xff;
				buf[2*u8_i+1] = (u16_AutoRegression_Result[u8_i])&0xff;
			}
	
			*num_type = VIP_UINT16;
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
		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Curve_Control_Coef[bTemp]), sizeof(VIP_DCC_Curve_Control_Coef)*DCC_SELECT_MAX);
		buf2_tmp[35] = system_setting_info->OSD_Info.OSD_DCC_Mode;
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

		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Boundary_check_Table[bTemp]), size);
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DCC_Level_and_Blend_Coef_Table:     /* 1102*/
		if (size > sizeof(VIP_DCC_Level_and_Blend_Coef_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.Level_and_Blend_Coef_Table;

		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table[bTemp]), size);
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DCC_Hist_Factor_Table:   			/* 1103*/
		if (size > sizeof(VIP_DCC_Hist_Factor_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.hist_adjust_table;

		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCCHist_Factor_Table[bTemp]), size);
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_DCC_AdaptCtrl_Level_Table:  		/* 1104*/
		if (size > sizeof(VIP_DCC_AdaptCtrl_Level_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.AdaptCtrl_Level_Table;

		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_AdaptCtrl_Level_Table[bTemp]), size);
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

		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisMean_th), 10);
		memcpy(buf2_tmp+sizeof(USER_CURVE_DCC_HisMean_th_ITEM), (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisSeg_Weighting), 11);

		DCC_HISTO_MEAN_VALUE = smartPic_clue->Hist_Y_Mean_Value;

		memcpy(crtl_table, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisMean_th), 10);

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
		buf2_tmp[23] = (unsigned char)DCC_HISTO_MEAN_VALUE;
		buf2_tmp[24] = user_curve_1;
		buf2_tmp[25] = user_curve_2;
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

		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].user_define_curve_dcc_table),temp_size*sizeof(unsigned int));
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
		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].user_define_curve_dcc_table[rtice_param.mode0][0]),temp_size*sizeof(unsigned int));
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

		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][0]), temp_size*sizeof(unsigned int));
		memcpy(buf2_tmp+(temp_size*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_case_num), sizeof(unsigned int));
		memcpy(buf2_tmp+((temp_size+1)*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_weight), sizeof(unsigned int));

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

		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_curve), (DCC_CURVE_Segment_NUM*sizeof(unsigned int)));
		memcpy(buf2_tmp+(DCC_CURVE_Segment_NUM*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_case_num), sizeof(unsigned int));
		memcpy(buf2_tmp+((DCC_CURVE_Segment_NUM+1)*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_weight), sizeof(unsigned int));
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

		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_histogram), (Histogram_adjust_bin_num*sizeof(unsigned int)));
		memcpy(buf2_tmp+(Histogram_adjust_bin_num*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_case_num), sizeof(unsigned int));
		memcpy(buf2_tmp+((Histogram_adjust_bin_num+1)*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_weight), sizeof(unsigned int));

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
		memcpy(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Histogram_Curve), size);
		*num_type = VIP_UINT32;
		break;

	case TOOLACCESS_DCC_S_Curve:  /*id:1113*/
		if (size > sizeof(unsigned int)*DCC_Curve_Node_MAX)
			return -ID_SIZE_ERR;

		temp_size = DCC_Curve_Node_MAX;
		fwif_color_ChangeUINT32Endian((unsigned int *)&(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.S_Curve),temp_size, 1);
		memcpy(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.S_Curve), size);

		*num_type = VIP_UINT32;
		break;

	case TOOLACCESS_DCC_Histogram_Adjust:  /*id:1114*/
		if (size > sizeof(unsigned int)*COLOR_HISTOGRAM_LEVEL)
			return -ID_SIZE_ERR;

		temp_size = COLOR_HISTOGRAM_LEVEL;
		fwif_color_ChangeUINT32Endian((unsigned int *)&(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.HistCnt_Of_Adj),temp_size, 1);
		memcpy(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.HistCnt_Of_Adj), size);

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
		memcpy(pwIntTemp, (unsigned char *) &(RPC_smartPic_clue->RPC_Adapt_DCC_Info.Apply_Curve), size);
		fwif_color_ChangeUINT32Endian( (unsigned int *)pwIntTemp,temp_size, 1);
		memcpy(buf, (unsigned char *) pwIntTemp, size);
		vip_free(pwIntTemp);
		  pwIntTemp = NULL;
//		fwif_color_ChangeUINT32Endian((unsigned int *)&(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Apply_Curve),temp_size, 1);
//		memcpy(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Apply_Curve), size);

		*num_type = VIP_UINT32;

		break;

	case TOOLACCESS_DCC_Curve_Boundary_Type:			/* 1117*/
			if(size > sizeof(unsigned char))
				return -ID_SIZE_ERR;
 			pwCopyTemp = smartPic_clue->Remmping_Src_Timing_index;

			memcpy(buf,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_init_boundary_Type),size);
			*num_type = VIP_UINT8;

			break;

	case TOOLACCESS_DCC_Curve_Boundary:				/* 1118*/
			if(size > sizeof(RPC_DCC_Curve_boundary_table))
				return -ID_SIZE_ERR;

			pwCopyTemp = (smartPic_clue->Remmping_Src_Timing_index);

			Advance_control_table_by_source = &(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp]);
			bTemp = Advance_control_table_by_source->DCC_init_boundary_Type;
			memcpy(buf,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Curve_boundary_table[bTemp]),size);
			*num_type = VIP_UINT8;

			break;
	case TOOLACCESS_DCC_panel_compensation_Curve: /*1119*/
		if(size > sizeof(unsigned int)*Curve_num_Max)
			return -ID_SIZE_ERR;

		buf2_tmp = (unsigned char *)vip_malloc( Curve_num_Max*sizeof(unsigned int));
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

		temp_size = Curve_num_Max;
		memcpy(buf2_tmp,(unsigned char*)&(vipTable_ShareMem->panel_compensation_curve),size);

		fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,temp_size, 1);
		memcpy(buf, (unsigned char *) buf2_tmp, size);

		*num_type = VIP_UINT32;
		vip_free(buf2_tmp);
		buf2_tmp = NULL;

		break;

	case TOOLACCESS_DCC_DataBaseHistogrm :/* 1120*/
		if (size > sizeof(unsigned int)*Histogram_adjust_bin_num)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ENABLE_SCALER_DRIVER_BASE
		temp_size = Histogram_adjust_bin_num;
		fwif_color_ChangeUINT32Endian(&(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Database_entry_Histogram),temp_size, 1);
		memcpy(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Database_entry_Histogram), size);
#else
		temp_size = Histogram_adjust_bin_num;
		fwif_color_ChangeUINT32Endian((unsigned int*)&(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Database_entry_Histogram),temp_size, 1);
		memcpy(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Database_entry_Histogram), size);
#endif
		*num_type = VIP_UINT32;

		break;
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

		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_adjust), (DCC_CURVE_User_Gain_Parameter*sizeof(unsigned int)));
		memcpy(buf2_tmp+(DCC_CURVE_User_Gain_Parameter*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_case_num), sizeof(unsigned int));
		memcpy(buf2_tmp+((DCC_CURVE_User_Gain_Parameter+1)*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_weight), sizeof(unsigned int));

#ifdef CONFIG_ENABLE_SCALER_DRIVER_BASE
			temp_size = (DCC_CURVE_User_Gain_Parameter+2);
			fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,temp_size, 1);
			memcpy(buf, (unsigned char *)buf2_tmp, size);
#else
			temp_size = (DCC_CURVE_User_Gain_Parameter+2);
			fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,temp_size, 1);
			memcpy(buf, (unsigned char *)buf2_tmp, size);
#endif
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


		memcpy(buf2_tmp,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Advance_Level_Control_Flag),Advance_Level_Control_Item_MAX);
		memcpy(buf2_tmp+Advance_Level_Control_Item_MAX,(unsigned char*) &(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_LPF_Control),LPF_control_Item_MAX);
		memcpy(buf2_tmp+Advance_Level_Control_Item_MAX+LPF_control_Item_MAX,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_init_boundary_Type),1);

		//fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,size, 1);
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



		memcpy(buf2_tmp,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Curve_Blending_Step_table),mean_value_index_MAX*Variance_index_MAX);
		memcpy(buf2_tmp+mean_value_index_MAX*Variance_index_MAX,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Curve_Blending_DT_table),mean_value_index_MAX*Variance_index_MAX);
		//fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,size, 1);
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

		memcpy(buf2_tmp,(unsigned char*)&(system_setting_info->DCC_FW_ONOFF),1);
		memcpy(buf2_tmp+1,(unsigned char*)&(smartPic_clue->PureColor),1);
		memcpy(buf2_tmp+2,(unsigned char*)&(smartPic_clue->Total_Hist_Cnt_less_than_one),1);
		memcpy(buf2_tmp+3,(unsigned char*)&(smartPic_clue->Hist_Y_Total_sum_less_than_Total_HistCnt_thl),1);

		//fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,size, 1);
		memcpy(buf, (unsigned char *)buf2_tmp, size);

		*num_type = VIP_UINT8;
		vip_free(buf2_tmp);
		buf2_tmp = NULL;

		break;
	/* case 1110:*/
	/*TOOLACCESS_DCC_Histogram_Curve = 1111, size:129*/
	/*TOOLACCESS_DCC_S_Curve = 1112,  size:129									*/ /*read only*/
	/*TOOLACCESS_DCC_Histogram_Adjust = 1113, size :32							*/ /*read only*/
    /*TOOLACCESS_DCC_Adaptive_User_Curve = 1114,						*/ /*read only*/
	/*TOOLACCESS_DCC_APL = 1115,										*/ /*read only*/

	case TOOLACCESS_VIP_I_Main_Dither_Hist_Get: /*id: 1150 */
		if(size > sizeof(int)*(I_Main_DITHER_Y_HIST_NUM*2+4))	// hist, hist_ratio, max, min, toal_MSB, total_LSB
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc((I_Main_DITHER_Y_HIST_NUM*2+4)*sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwUINT32Temp, &RPC_smartPic_clue->I_Main_Dither_Hist[0], sizeof(int)*I_Main_DITHER_Y_HIST_NUM);
		memcpy(pwUINT32Temp+I_Main_DITHER_Y_HIST_NUM, &RPC_smartPic_clue->I_Main_Dither_Hist_ratio[0], sizeof(int)*I_Main_DITHER_Y_HIST_NUM);
		*(pwUINT32Temp+I_Main_DITHER_Y_HIST_NUM*2+0) = RPC_smartPic_clue->I_Main_Dither_MAX;
		*(pwUINT32Temp+I_Main_DITHER_Y_HIST_NUM*2+1) = RPC_smartPic_clue->I_Main_Dither_MIN;
		*(pwUINT32Temp+I_Main_DITHER_Y_HIST_NUM*2+2) = RPC_smartPic_clue->I_Main_Dither_Total_MSB;
		*(pwUINT32Temp+I_Main_DITHER_Y_HIST_NUM*2+3) = RPC_smartPic_clue->I_Main_Dither_Total_LSB;

		fwif_color_ChangeUINT32Endian(pwUINT32Temp, I_Main_DITHER_Y_HIST_NUM*2+4, 1);
		memcpy(buf, (unsigned char *)pwUINT32Temp, size);

		*num_type = VIP_UINT32;
		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		break;

	case TOOLACCESS_ICM_Global_Adjust: /* 1200*/
		if (size > sizeof(RPC_ICM_Global_Ctrl))
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *)fwif_color_GetICM_Global_Ctrl_Struct(), size);
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
		//pwUINT32Temp = (unsigned int *)vip_malloc(80000 * sizeof(unsigned char));		
		pwUINT32Temp = (unsigned int *)dvr_malloc_specific(80000 * sizeof(unsigned char), GFP_DCU2_FIRST);
		if (pwUINT32Temp == NULL){
			//IoReg_Write32(0xb802c200,0x00000002);
			return -ID_SIZE_ERR;
		}else{
			//IoReg_Write32(0xb802c200,0x00000004);
		}

		/* read star and end point*/
		pwUINT32Temp[0] = g_h_sta;
		pwUINT32Temp[1] = g_h_end;
		pwUINT32Temp[2] = g_s_sta;
		pwUINT32Temp[3] = g_s_end;
		pwUINT32Temp[4] = g_i_sta;
		pwUINT32Temp[5] = g_i_end;

		if (size > sizeof(unsigned int)*((g_i_end - g_i_sta + 1) * (g_s_end - g_s_sta + 1) * (g_h_end - g_h_sta + 1) * 2 + 6)) {
			vip_free(pwUINT32Temp);
			pwUINT32Temp = NULL;
			return -ID_SIZE_ERR;
		}

		fwif_color_icm_SramBlockAccessSpeedup(pwUINT32Temp+6, 0,  g_h_sta,  g_h_end,  g_s_sta,  g_s_end,  g_i_sta,  g_i_end);
		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/4, 1);

		memcpy(buf, (unsigned char *)pwUINT32Temp, size);

		*num_type = VIP_UINT32;

		//vip_free(pwUINT32Temp);
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

        case TOOLACCESS_FIML_Result:
		if (size > sizeof(int)*78)
			return -ID_SIZE_ERR;

		//pwIntTemp = (int *)vip_malloc( sizeof(int)*78);

		//if (pwIntTemp == NULL)
		//	return -ID_SIZE_ERR;

                drvif_module_film_read_result();

		memcpy(buf, (unsigned char *)y_film_result, size);

		fwif_color_ChangeINT32Endian((int *)buf, size>>2, 1);
		//fwif_color_ChangeINT32Endian((int *)buf, size/4, 1);

		*num_type = VIP_INT32;

		vip_free(pwIntTemp);

		break;

	case TOOLACCESS_PCID_RgnTbl: /*1252 get*/
		
		if (size > sizeof(unsigned int)*PCID_TBL_LEN)
			return -ID_SIZE_ERR;

		pUiTemp = (unsigned int *)vip_malloc(sizeof(unsigned int)*PCID_TBL_LEN);

		if(pUiTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_get_pcid_RgnTableValue(pUiTemp, rtice_param.mode0, rtice_param.mode1);

		memcpy(buf, (unsigned char*)pUiTemp, size);
		fwif_color_ChangeINT32Endian((unsigned int *)buf, size>>2, 1);

		*num_type = VIP_UINT32;

		vip_free(pUiTemp);

		break;

	case TOOLACCESS_PCID_RgnWtTbl: /*1253 get*/
		if (size > sizeof(unsigned int)*PCID_WTTBL_LEN)
			return -ID_SIZE_ERR;

		pUiTemp = (unsigned int *)vip_malloc(sizeof(unsigned int)*PCID_WTTBL_LEN);

		if(pUiTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_get_pcid_RgnWtTableValue(pUiTemp);

		memcpy(buf, (unsigned char*)pUiTemp, size);
		fwif_color_ChangeINT32Endian((unsigned int *)buf, size>>2, 1);

		*num_type = VIP_UINT32;

		vip_free(pUiTemp);
		break;

	case TOOLACCESS_VALC_Tbl: /*1254 get*/

		if (size > sizeof(unsigned int)*VALC_TBL_LEN)
			return -ID_SIZE_ERR;

		pUiTemp = (unsigned int *)vip_malloc(sizeof(unsigned int)*VALC_TBL_LEN);

		if(pUiTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_get_valc_table(pUiTemp, rtice_param.mode0, rtice_param.mode1);

		memcpy(buf, (unsigned char*)pUiTemp, size);
		fwif_color_ChangeINT32Endian((unsigned int *)buf, size>>2, 1);

		*num_type = VIP_UINT32;

		vip_free(pUiTemp);

		break;

	case TOOLACCESS_MEMC_CHANGE_SIZE: /*1256 get*/

		if(size > sizeof(MEMC_SIZE_PARAM))
			return -ID_SIZE_ERR;

		pUiTemp = (unsigned int *)vip_malloc(sizeof(MEMC_SIZE_PARAM)/sizeof(unsigned int));

		if(pUiTemp == NULL)
			return -ID_SIZE_ERR;

//merlin7_bring_up			MEMC_RTICE_GetParam((MEMC_SIZE_PARAM*)pUiTemp);

		memcpy(buf, (unsigned int*)pUiTemp, size);

		*num_type = VIP_INT32;

		vip_free(pUiTemp);

		break;
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
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

		#if 0// test
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		
		{
		extern unsigned char get_svp_protect_status_aipq(void);
		buf[0] = get_svp_protect_status_aipq();
		}

		*num_type = VIP_UINT8;
		#endif


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
		#if 1 // 0506 lsy

		if (size > sizeof(DRV_AI_PQ_Ctrl_table))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DRV_AI_PQ_Ctrl_table));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		scaler_AI_PQ_Ctrl_Get((DRV_AI_PQ_Ctrl_table*)pwIntTemp);

		#else
		if (size > sizeof(DRV_AI_SCENE_Ctrl_table))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DRV_AI_SCENE_Ctrl_table));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		scaler_AI_Scene_Ctrl_Get((DRV_AI_SCENE_Ctrl_table*)pwIntTemp);
		#endif
		
		fwif_color_ChangeINT32Endian(pwIntTemp, size>>2, 1);

		memcpy(buf, (unsigned char *)pwIntTemp, size);

		*num_type = VIP_INT32;

		vip_free(pwIntTemp);



		break;

	case TOOLACCESS_AI_SCENE_Ctrl_test:   	/*  1271*/

		// lesley test
		buf[0] = force_scene;
		buf[1] = force_sqm;//0506 lsy
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_AI_SCENE_Target: /*1272 get*/
		if (size > PQ_IP_TYPE_NUM*PQ_TUNING_NUM*sizeof(int))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(PQ_IP_TYPE_NUM*PQ_TUNING_NUM*sizeof(int));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		scalerAI_scene_PQ_target_get(rtice_param.mode0, pwIntTemp);

		fwif_color_ChangeINT32Endian(pwIntTemp, size>>2, 1);

		memcpy(buf, (unsigned char *)pwIntTemp, size);
		*num_type = VIP_INT32;

		vip_free(pwIntTemp);
		pwUINT32Temp = NULL;
		break;

	case TOOLACCESS_AI_PQMASK_Control: /*1275 get*/

		if(rtice_param.mode0 == 0) { // performance

			if (size > PQMASK_PERF_NUM*sizeof(PQMASK_PERF_T))
				return -ID_SIZE_ERR;

			pPQMaskPerf = (PQMASK_PERF_T *)scalerPQMask_GetCtrlStruct(PQMASK_STRUCT_PERF);

			if( pPQMaskPerf == NULL )
				return READ_ERROR;

			memcpy((void *)buf, (void *)pPQMaskPerf, size);
			fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(unsigned int), 1);

		} else if(rtice_param.mode0 == 1) { // get color mgr informations

			if (size > 27*sizeof(unsigned int))
				return -ID_SIZE_ERR;

			pPQMaskColorMgr = (PQMASK_COLOR_MGR_T *)scalerPQMaskColor_GetStruct(PQMASK_COLORSTRUCT_MGR);

			if( pPQMaskColorMgr == NULL )
				return READ_ERROR;

			ofst = 0;

			// semantic / depth fusion parameter
			/* 0 */
			u32tmpValue = pPQMaskColorMgr->Param.FusionRatio_Dep;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 1 */
			u32tmpValue = pPQMaskColorMgr->Param.FusionRatio_Sem;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			// temporal filter parameter
			/* 2 */
			u32tmpValue = pPQMaskColorMgr->Param.FusionRatio_Pre;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 3 */
			u32tmpValue = pPQMaskColorMgr->Param.FusionRatio_Cur;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			// spatial filter parameter
			/* 4 */
			u32tmpValue = pPQMaskColorMgr->Param.SpatialFltCycle;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 5 */
			u32tmpValue = pPQMaskColorMgr->Param.SEHorCoef[0];
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 6 */
			u32tmpValue = pPQMaskColorMgr->Param.SEHorCoef[1];
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 7 */
			u32tmpValue = pPQMaskColorMgr->Param.SEHorCoef[2];
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 8 */
			u32tmpValue = pPQMaskColorMgr->Param.SEHorCoef[3];
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 9 */
			u32tmpValue = pPQMaskColorMgr->Param.SEVerCoef[0];
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 10 */
			u32tmpValue = pPQMaskColorMgr->Param.SEVerCoef[1];
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 11 */
			u32tmpValue = pPQMaskColorMgr->Param.SEVerCoef[2];
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 12 */
			u32tmpValue = pPQMaskColorMgr->Param.SEVerCoef[3];
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 13 */
			u32tmpValue = pPQMaskColorMgr->Param.HorOddType;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 14 */
			u32tmpValue = pPQMaskColorMgr->Param.VerOddType;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			// scene change
			/* 15 */
			u32tmpValue = pPQMaskColorMgr->Param.bSceneChangeEnable;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 16 */
			u32tmpValue = pPQMaskColorMgr->Param.SceneChangeRstFrm;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 17 */
			u32tmpValue = pPQMaskColorMgr->Param.ScFusionRatio_toRst_Pre;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 18 */
			u32tmpValue = pPQMaskColorMgr->Param.ScFusionRatio_toRst_Cur;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 19 */
			u32tmpValue = pPQMaskColorMgr->Param.ScFusionRatio_toTarget_Pre;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 20 */
			u32tmpValue = pPQMaskColorMgr->Param.ScFusionRatio_toTarget_Cur;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			// cg pattern
			/* 21 */
			u32tmpValue = pPQMaskColorMgr->Param.bGenre_CG_Enable;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 22 */
			u32tmpValue = pPQMaskColorMgr->Param.Genre_CG_toTargetFrm;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 23 */
			u32tmpValue = pPQMaskColorMgr->Param.Genre_CG_FunctionRatio_Pre;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 24 */
			u32tmpValue = pPQMaskColorMgr->Param.Genre_CG_FunctionRatio_Cur;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			// image histogram by depth threshold
			/* 25 */
			u32tmpValue = pPQMaskColorMgr->ThlLow;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 26 */
			u32tmpValue = pPQMaskColorMgr->ThlHigh;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;

			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);

		} else if(rtice_param.mode0 == 2) { // get pqmask color mgr status

			if (size > 12*sizeof(unsigned int))
				return -ID_SIZE_ERR;

			pPQMaskColorMgr = (PQMASK_COLOR_MGR_T *)scalerPQMaskColor_GetStruct(PQMASK_COLORSTRUCT_MGR);

			if( pPQMaskColorMgr == NULL )
				return READ_ERROR;
			
			ofst = 0;

			// scene change
			/* 0 */
			u32tmpValue = pPQMaskColorMgr->Status.bSceneChange;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 1 */
			u32tmpValue = pPQMaskColorMgr->Status.SCRstFrm;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 2 */
			u32tmpValue = pPQMaskColorMgr->Status.LocalContrastRstWeight;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			// Force current output
			/* 3 */
			u32tmpValue = pPQMaskColorMgr->Status.ForceCurFrm;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			// palette control
			/* 4 */
			u32tmpValue = pPQMaskColorMgr->Status.bReloadDepPalette;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 5 */
			u32tmpValue = pPQMaskColorMgr->Status.bReloadSemPalette;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			// AI Genre palette control
			/* 6 */
			u32tmpValue = pPQMaskColorMgr->Status.Genre_CG_Frm;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 7 */
			u32tmpValue = pPQMaskColorMgr->Status.AiGenre;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 8 */
			u32tmpValue = pPQMaskColorMgr->Status.bGenerCGStatusChange;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 9 */
			u32tmpValue = pPQMaskColorMgr->Status.Genre_CG_Frm;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 10 */
			u32tmpValue = pPQMaskColorMgr->Status.bReloadDepthPalette_AiGenre_CG;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 11 */
			u32tmpValue = pPQMaskColorMgr->Status.bReloadSemanticPalette_AiGenre_CG;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;

			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);

		} else if(rtice_param.mode0 == 3) { // get

			if (size > 8*sizeof(unsigned int))
				return -ID_SIZE_ERR;
			
			pPQMaskGlbCtrl = (PQMASK_GLB_CTRL_T *)scalerPQMask_GetCtrlStruct(PQMASK_STRUCT_GLB_CTRL);
			pPQMaskGlbStatus = (PQMASK_GLB_STATUS_T *)scalerPQMask_GetStatusStruct(PQMASK_STRUCT_GLB_STATUS);

			if(pPQMaskGlbCtrl == NULL || pPQMaskGlbStatus == NULL)
				return READ_ERROR;

			ofst = 0;

			/* 0 */
			u32tmpValue = pPQMaskGlbStatus->PQMaskStatus;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 1 */
			u32tmpValue = pPQMaskGlbCtrl->bEnableSeProc;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 2 */
			u32tmpValue = pPQMaskGlbCtrl->LimitFPS;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 3 */
			u32tmpValue = pPQMaskGlbCtrl->ErrRecoverRstTimes;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 4 */
			u32tmpValue = pPQMaskGlbCtrl->SemCapImgLimitFPS;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 5 */
			u32tmpValue = pPQMaskGlbCtrl->bIsDepthReady;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 6 */
			u32tmpValue = pPQMaskGlbCtrl->bIsSemReady;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 7 */
			u32tmpValue = pPQMaskGlbStatus->bAllowExec;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;

			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);

		} else if(rtice_param.mode0 == 4) { // get buffer information

			if (size > 67*sizeof(unsigned int))
				return -ID_SIZE_ERR;

			pPQMaskBufInfo = (PQMASK_BUFINFO_T *)scalerPQMask_GetStatusStruct(PQMASK_STRUCT_BUFINFO);

			if(pPQMaskBufInfo == NULL)
				return READ_ERROR;

			ofst = 0;

			// module buffer info, 8*3*2=48, 0~47
			for(i=0; i<PQMASK_MODULE_NUM; i++) {
				for(j=0; j<PQMASK_MODULE_DMA_NUM; j++) {
					u32tmpValue = pPQMaskBufInfo->ModuleDma[i][j].AddrInfo.PhysAddr;
					memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
					ofst++;
					u32tmpValue = pPQMaskBufInfo->ModuleDma[i][j].AddrInfo.Size;
					memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
					ofst++;
				}
			}

			// se process buffer info, 9*2=18, 48~65
			for(i=0; i<PQMASK_SE_BUF_NUM; i++) {
				u32tmpValue = pPQMaskBufInfo->SeBuffer[i].AddrInfo.PhysAddr;
				memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
				ofst++;
				u32tmpValue = pPQMaskBufInfo->SeBuffer[i].AddrInfo.Size;
				memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
				ofst++;
			}

			// totoal size, 1 = 66
			u32tmpValue = pPQMaskBufInfo->Size;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;

			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);

		} else if(rtice_param.mode0 == 5) { // get online monitor control

			// online monitor control
			if (size > 6*sizeof(unsigned int))
				return -ID_SIZE_ERR;

			pPQMaskMonitorCtrl = (PQMASK_ONLINE_MONITOR_CTRL_T *)scalerPQMask_GetCtrlStruct(PQMASK_STRUCT_MONITOR_CTRL);
			

			ofst = 0;

			/* 0 */
			u32tmpValue = pPQMaskMonitorCtrl->bEnable;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 1 */
			u32tmpValue = pPQMaskMonitorCtrl->bLogEnable;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 2 */
			u32tmpValue = pPQMaskMonitorCtrl->LogPeriod;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 3 */
			u32tmpValue = pPQMaskMonitorCtrl->Depth_TargetFps;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 4 */
			u32tmpValue = pPQMaskMonitorCtrl->Depth_MeasureRstFrm;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 5 */
			u32tmpValue = pPQMaskMonitorCtrl->SEProc_TargetTime;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;

			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);

		} else if(rtice_param.mode0 == 6) { // get online monitor status

			// online monitor control
			if (size > 8*sizeof(unsigned int))
				return -ID_SIZE_ERR;

			pPQMaskMonitorStatus = 	(PQMASK_ONLINE_MONITOR_STATUS_T *)scalerPQMask_GetStatusStruct(PQMASK_STRUCT_MONITOR_STATUS);

			ofst = 0;

			/* 0 */
			u32tmpValue = pPQMaskMonitorStatus->Depth_FpsError;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 1 */
			u32tmpValue = pPQMaskMonitorStatus->SEProc_TimeError;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 2 */
			u32tmpValue = pPQMaskMonitorStatus->LogCounter;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 3 */
			u32tmpValue = pPQMaskMonitorStatus->Depth_MeasureStartTime;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 4 */
			u32tmpValue = pPQMaskMonitorStatus->Depth_MeauserEndTime;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 5 */
			u32tmpValue = pPQMaskMonitorStatus->Depth_Counter;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 6 */
			u32tmpValue = pPQMaskMonitorStatus->SECurProcStart;
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;
			/* 7 */
			u32tmpValue = pPQMaskMonitorStatus->SECurProcEnd;
		} else if(rtice_param.mode0 == 7) { // get pqmask timer status

#if 0
			extern struct timer_list PQMaskTimer;

			if (size > 1*sizeof(unsigned int))
				return -ID_SIZE_ERR;

			ofst = 0;

			u32tmpValue = timer_pending(&PQMaskTimer);
			memcpy(((unsigned int*)buf)+ofst, &u32tmpValue, sizeof(unsigned int));
			ofst++;

			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);
#endif
		} else if(rtice_param.mode0 == 8) { // non

			return -ID_SIZE_ERR;

		} else if(rtice_param.mode0 == 9) { // get pqmask palette setting

			if( size > sizeof(unsigned int)*PQMASK_COLOR_PALETTE_MAX*MODULE_GRP_MAX*PQMASK_MAPCURVE_LEN )
				return -ID_SIZE_ERR;

			pUiTemp = (unsigned int *)scalerPQMaskColor_GetStruct(PQMASK_COLORSTRUCT_PALETTE);
			memcpy((void *)buf, (void *)pUiTemp, size);

			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);

		}

		*num_type = VIP_UINT32;
		break;

	case TOOLACCESS_AI_PQMASK_SemanticWeight: /*1276 get*/

#if 0
		rtd_printk(KERN_EMERG,"DEBUG", "read semantic weight\n");
#endif

		if (size > PQMASK_MODULE_NUM*PQMASK_LABEL_NUM*sizeof(unsigned char))
			return -ID_SIZE_ERR;

		pUINT8Temp = (unsigned char *)vip_malloc(PQMASK_MODULE_NUM*PQMASK_LABEL_NUM*sizeof(unsigned char));

		if(pUINT8Temp == NULL)
			return -ID_SIZE_ERR;

		scalerPQMaskColor_GetSemanticWeight(pUINT8Temp);

		memcpy((unsigned char *)buf, (unsigned char *)pUINT8Temp, size);

		vip_free(pUINT8Temp);
		pUINT8Temp = NULL;

		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_AI_PQMASK_DepthWeight: /*1277 get*/

#if 0
		rtd_printk(KERN_EMERG,"DEBUG", "read depth weight\n");
#endif

		if (size > PQMASK_MODULE_NUM*PQMASK_MAPCURVE_LEN*sizeof(unsigned char))
			return -ID_SIZE_ERR;

		pUINT8Temp = (unsigned char *)vip_malloc(PQMASK_MODULE_NUM*PQMASK_MAPCURVE_LEN*sizeof(unsigned char));

		if(pUINT8Temp == NULL)
			return -ID_SIZE_ERR;

		scalerPQMaskColor_GetDepthWeight(pUINT8Temp);

		memcpy((unsigned char *)buf, (unsigned char *)pUINT8Temp, size);

		vip_free(pUINT8Temp);
		pUINT8Temp = NULL;

		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_AI_PQMASK_HistInfo: /*1278 get*/

		if (size > sizeof(PQMASK_HIST_INFO_T))
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(sizeof(PQMASK_HIST_INFO_T));

		if(pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		pPQMaskColorMgr = (PQMASK_COLOR_MGR_T *)scalerPQMaskColor_GetStruct(PQMASK_COLORSTRUCT_MGR);

		if( rtice_param.mode0 == 0 ) {
			memcpy((void *)pwUINT32Temp, (void *)&pPQMaskColorMgr->SemanticHist.Info, sizeof(PQMASK_HIST_INFO_T));
		} else if( rtice_param.mode0 == 1 ) {
			memcpy((void *)pwUINT32Temp, (void *)&pPQMaskColorMgr->DepthHist.Info, sizeof(PQMASK_HIST_INFO_T));
		} else if( rtice_param.mode0 == 2 ) { // this information is generated by ap
			memcpy((void *)pwUINT32Temp, (void *)&pPQMaskColorMgr->ImgBySemanticHist[0][0].Info, sizeof(PQMASK_HIST_INFO_T));
		} else if( rtice_param.mode0 == 3 ) {
			memcpy((void *)pwUINT32Temp, (void *)&pPQMaskColorMgr->ImgByDepthHistInfo, sizeof(PQMASK_HIST_INFO_T));
		}

		memcpy((void *)buf, (void *)pwUINT32Temp, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(unsigned int), 1);

		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		*num_type = VIP_UINT32;

		break;

	case TOOLACCESS_AI_PQMASK_HistData: /*1279 get*/

		// check the max size
		if (size > sizeof(unsigned int)*PQMASK_HISTO_BIN*PQMASK_LABEL_NUM*PQMASK_IMGCH_NUM)
			return -ID_SIZE_ERR;
		pwUINT32Temp = (unsigned int *)vip_malloc(sizeof(unsigned int)*PQMASK_HISTO_BIN*PQMASK_LABEL_NUM*PQMASK_IMGCH_NUM);

		if(pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		pPQMaskColorMgr = (PQMASK_COLOR_MGR_T *)scalerPQMaskColor_GetStruct(PQMASK_COLORSTRUCT_MGR);

		if( rtice_param.mode0 == 0 ) { // semantic histogram
			if( rtice_param.mode1 == 0 )
				memcpy((void *)pwUINT32Temp, (void *)(pPQMaskColorMgr->SemanticHist.Data.Histogram), sizeof(unsigned int)*PQMASK_HISTO_BIN);
			else
				memcpy((void *)pwUINT32Temp, (void *)(pPQMaskColorMgr->SemanticHist.Data.NorHist), sizeof(unsigned int)*PQMASK_HISTO_BIN);
		} else if( rtice_param.mode0 == 1 ) { // depth histogram
			if( rtice_param.mode1 == 0 )
				memcpy((void *)pwUINT32Temp, (void *)(pPQMaskColorMgr->DepthHist.Data.Histogram), sizeof(unsigned int)*PQMASK_HISTO_BIN);
			else
				memcpy((void *)pwUINT32Temp, (void *)(pPQMaskColorMgr->DepthHist.Data.NorHist), sizeof(unsigned int)*PQMASK_HISTO_BIN);
		} else if( rtice_param.mode0 == 2 ) { // source image yuv data by semantic label histogram, total 6 labels, 3 channel, 32 bin hist
			if( rtice_param.mode1 == 0 ) {
				for(i=0; i<PQMASK_LABEL_NUM; i++) {
					for(j=0; j<PQMASK_IMGCH_NUM; j++) {
						memcpy( (void *)(pwUINT32Temp+(i*PQMASK_IMGCH_NUM*PQMASK_HISTO_BIN + j*PQMASK_HISTO_BIN)),
								(void *)(pPQMaskColorMgr->ImgBySemanticHist[i][j].Data.Histogram),
								PQMASK_HISTO_BIN*sizeof(unsigned int)
						);
					}
				}
			} else {
				ofst = 0;
				for(i=0; i<PQMASK_LABEL_NUM; i++) {
					for(j=0; j<PQMASK_IMGCH_NUM; j++) {
						for(k=0; k<PQMASK_HISTO_BIN; k++) {
							pwUINT32Temp[ofst] = pPQMaskColorMgr->ImgBySemanticHist[i][j].Data.Histogram[k];
							ofst++;
						}
					}
				}
			}
		} else if( rtice_param.mode0 == 3 ) { // source image yuv data by depth(far/mid/near) histogram
			if( rtice_param.mode1 == 0 ) {
				for(i=0; i<PQMASK_DEPDIST_NUM; i++) {
					for(j=0; j<PQMASK_IMGCH_NUM; j++) {
						memcpy( (void *)(pwUINT32Temp+(i*PQMASK_IMGCH_NUM*PQMASK_HISTO_BIN + j*PQMASK_HISTO_BIN)),
								(void *)(pPQMaskColorMgr->ImgByDepthHist[i][j].Data.Histogram),
								PQMASK_HISTO_BIN*sizeof(unsigned int)
						);
					}
				}
			} else {
				ofst = 0;
				for(i=0; i<PQMASK_DEPDIST_NUM; i++) {
					for(j=0; j<PQMASK_IMGCH_NUM; j++) {
						for(k=0; k<PQMASK_HISTO_BIN; k++) {
							pwUINT32Temp[ofst] = pPQMaskColorMgr->ImgByDepthHist[i][j].Data.Histogram[k];
							ofst++;
						}
					}
				}
			}
		} else {
			rtd_pr_vpq_emerg("id = %d, mode0 = %d, mode1 = %d error\n", rtice_param.id, rtice_param.mode0, rtice_param.mode1);
			memset(pwUINT32Temp, 0x0, size);
		}

#if 0
		if( rtice_param.mode0 == 0 ) {
			rtd_printk(KERN_EMERG,"DEBUG", "SEMANTIC HISTOGRAM\n");
		} else if( rtice_param.mode0 == 1 ) {
			rtd_printk(KERN_EMERG,"DEBUG", "DEPTH HISTOGRAM\n");
		} else if( rtice_param.mode0 == 2 ) {
			rtd_printk(KERN_EMERG,"DEBUG", "IMG BY SEMANTIC HISTOGRAM\n");
		} else if( rtice_param.mode0 == 3 ) {
			rtd_printk(KERN_EMERG,"DEBUG", "IMG BY DEPTH HISTOGRAM\n");
		}

		if( rtice_param.mode0 == 0 || rtice_param.mode0 == 1 )
			for(i=0; i<PQMASK_HISTO_BIN; i++) {
				rtd_printk(KERN_EMERG,"DEBUG", "[%2d] %u\n", i, pwUINT32Temp[i]);
		} else if( rtice_param.mode0 == 2 ) {
			for(i=0; i<PQMASK_LABEL_NUM; i++) {
				rtd_printk(KERN_EMERG,"DEBUG", "label = %d\n", i);
				for(j=0; j<PQMASK_IMGCH_NUM; j++) {
					rtd_printk(KERN_EMERG,"DEBUG", "channel = %d\n", j);
					for(k=0; k<PQMASK_HISTO_BIN; k++) {
						rtd_printk(KERN_EMERG,"DEBUG", "[%2d] %u\n", k, pwUINT32Temp[i*PQMASK_IMGCH_NUM*PQMASK_HISTO_BIN + j*PQMASK_HISTO_BIN + k]);
					}
				}
			}
		} else if( rtice_param.mode0 == 3 ) {
			for(i=0; i<PQMASK_DEPDIST_NUM; i++) {
				rtd_printk(KERN_EMERG,"DEBUG", "section = %d\n", i);
				for(j=0; j<PQMASK_IMGCH_NUM; j++) {
					rtd_printk(KERN_EMERG,"DEBUG", "channel = %d\n", j);
					for(k=0; k<PQMASK_HISTO_BIN; k++) {
						rtd_printk(KERN_EMERG,"DEBUG", "[%2d] %u\n", k, pwUINT32Temp[i*PQMASK_DEPDIST_NUM*PQMASK_HISTO_BIN + j*PQMASK_HISTO_BIN + k]);
					}
				}
			}
		}
#endif

		memcpy((void *)buf, (void *)pwUINT32Temp, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(unsigned int), 1);

		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		*num_type = VIP_UINT32;

		break;
	case TOOLACCESS_PQMASK_BRINGUP_VERIFY:
	{
		extern void scalerPQMask_SetDMAPTG(unsigned int ModuleSel, unsigned int PatternSel);
		#if PQMASK_FUNC_DUMPBUF
		extern void PQMaskDumpModuleBufCheck(unsigned int bufSel, unsigned int Num, char *fileName);
		#endif
	
		buf[0] = 0;
	
		if( rtice_param.mode0 == 0 ) {
			if( rtice_param.mode1 == 1 ) {
				scalerPQMask_init();
				buf[0] = 1;
			}
		} else if( rtice_param.mode0 == 1 ) {
			scalerPQMask_SetDMAPTG( (rtice_param.mode1 & 0xf), ((rtice_param.mode1>>4) & 0xf));
			buf[0] = 2;
		} else if( rtice_param.mode0 == 2 ) {
			#if PQMASK_FUNC_DUMPBUF
			PQMaskDumpModuleBufCheck( (rtice_param.mode1 & 0xf), 0, "RTICE");
			#endif
			buf[0] = 3;
		} else {
			buf[0] = 4;
		}
		*(num_type) = VIP_UINT8;
		break;
	}

	case TOOLACCESS_AI_PQMASK_SQM_TABLE:
	{
		extern short AI_DepthWeightTbl_SQM_TBL[AI_RESOLUTION_TYPE_NUM][PQMASK_MODULE_NUM];

		if (size+start_addr > sizeof(AI_DepthWeightTbl_SQM_TBL))
			return -ID_SIZE_ERR;

		if (size > 0) {
			memcpy(buf, (unsigned short *)AI_DepthWeightTbl_SQM_TBL+start_addr, size);
			fwif_color_ChangeINT16Endian((short*)buf, size/sizeof(short), 1);
		}
		*(num_type) = VIP_INT16;
	}
	break;
#endif
        /*NNSR*/
        case TOOLACCESS_PQ_SR_MODE_Ctrl:

		if (size > sizeof(UINT8))
			return -ID_SIZE_ERR;

		buf[0] = fwif_color_Get_PQ_SR_MODE();
		*num_type = VIP_UINT8;


                break;

        case TOOLACCESS_NNSR_Blanding_Idx:

		if (size > sizeof(UINT8))
			return -ID_SIZE_ERR;

		//buf[0] = fwif_color_Get_PQ_SR_MODE();
		*num_type = VIP_UINT8;


                break;


        case TOOLACCESS_NNSR_Weight_Table_Idx:

		if (size > sizeof(UINT8))
			return -ID_SIZE_ERR;

		buf[0] = fwif_color_Get_PQ_SR_MODE();
		*num_type = VIP_UINT8;


                break;

	case TOOLACCESS_NNSR_Weight_Tbl:
		
		if(rtice_param.mode0 ==2)
			nnsr_size = SRNN_Weight_Num_mode2;
		else if(rtice_param.mode0 ==3)
			nnsr_size = SRNN_Weight_Num_mode3;
		else if(rtice_param.mode0 ==4)
			nnsr_size = SRNN_Weight_Num_mode4;
		else if(rtice_param.mode0 ==5)
			nnsr_size = SRNN_Weight_Num_mode5;
		else if(rtice_param.mode0 ==6)
			nnsr_size = SRNN_Weight_Num_mode6;
		else if(rtice_param.mode0 ==7)
			nnsr_size = SRNN_Weight_Num_mode7;	
		
		if(size>sizeof(unsigned short)*nnsr_size)
			 return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(nnsr_size * sizeof(unsigned short));
		
		if (pwUINT16Temp == NULL)
			return -ID_SIZE_ERR;
		fwif_color_srnn_m_disp_en_ctrl(0);		
		drvif_srnn_model_get(pwUINT16Temp,rtice_param.mode0);
		fwif_color_srnn_m_disp_en_ctrl(1);
		fwif_color_ChangeUINT16Endian(pwUINT16Temp, size/sizeof(unsigned short), 1);		
		memcpy(buf, pwUINT16Temp, size);
		*num_type = VIP_UINT16;		
		
		vip_free(pwUINT16Temp);
		pwUINT16Temp = NULL;
		

		break;
	   case TOOLACCESS_NNSR_Weight_Blend:
	   	
		if(rtice_param.mode0 ==2)
			nnsr_size = SRNN_Weight_Num_mode2;
		else if(rtice_param.mode0 ==3)
			nnsr_size = SRNN_Weight_Num_mode3;
		else if(rtice_param.mode0 ==4)
			nnsr_size = SRNN_Weight_Num_mode4;
		else if(rtice_param.mode0 ==5)
			nnsr_size = SRNN_Weight_Num_mode5;
		else if(rtice_param.mode0 ==6)
			nnsr_size = SRNN_Weight_Num_mode6;
		else if(rtice_param.mode0 ==7)
			nnsr_size = SRNN_Weight_Num_mode7;

		/*
		if(size>sizeof(unsigned short)*nnsr_size)
			 return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(nnsr_size * sizeof(unsigned short));

		if (pwUINT16Temp == NULL)
			return -ID_SIZE_ERR;

		vip_free(pwUINT16Temp);
		*/
		*num_type = VIP_UINT16;
 		break;
	
	case TOOLACCESS_NNSR_test_set:
	{
		extern unsigned char weight_blending_ctrl[5];

		if(rtice_param.mode0 ==2)
			nnsr_size = SRNN_Weight_Num_mode2;
		else if(rtice_param.mode0 ==3)
			nnsr_size = SRNN_Weight_Num_mode3;
		else if(rtice_param.mode0 ==4)
			nnsr_size = SRNN_Weight_Num_mode4;
		else if(rtice_param.mode0 ==5)
			nnsr_size = SRNN_Weight_Num_mode5;
		else if(rtice_param.mode0 ==6)
			nnsr_size = SRNN_Weight_Num_mode6;
		else if(rtice_param.mode0 ==7)
			nnsr_size = SRNN_Weight_Num_mode7;	

		if(size>sizeof(weight_blending_ctrl))
			 return -ID_SIZE_ERR;

		memcpy(buf, &weight_blending_ctrl[0], size);

		*num_type = VIP_UINT8;
	}
	break;
	case TOOLACCESS_siw_PCLRC_Table:

		if (size > 12544*sizeof(unsigned char))
			return -ID_SIZE_ERR;

		pUINT8Temp = (unsigned char *)vip_malloc(12544*sizeof(unsigned char));


		if(pUINT8Temp == NULL){

                        return -ID_SIZE_ERR;

		}

                drvif_color_enable_PCLRC_CLK(TRUE);
                //if panel_pclrc.enable=0  will cause hw write table no used
                drvif_color_set_enable_PCLRC_Funciton(1,_PCLRC_PCLRC_LOD);
                drvif_color_PCLRC_Register_Setting();
                drvif_color_get_siw_PCLRC_valuetable((unsigned char *)pUINT8Temp);
       
		memcpy(buf, (unsigned char *)pUINT8Temp, size);

		vip_free(pUINT8Temp);
		pUINT8Temp = NULL;



		*num_type = VIP_UINT8;



                break;
        case TOOLACCESS_BIT_TEST:
/*
                if (size > sizeof(UINT8))
                        return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0){//V4L2_CID_EXT_VPQ_SHARPNESS
                        fwif_color_Get_sharpness_table_TV006();
                        if(fwif_color_bit_test_sharpness_table_TV006(0)<0){
                                buf[0] = 4;
                        }
                        else
                        {
                                buf[0] = 8;
                        }
                                
		}
		else if (rtice_param.mode0 == 1){//V4L2_CID_EXT_VPQ_SUPER_RESOLUTION
                        fwif_color_Get_sharpness_table_TV006();
                        if(fwif_color_bit_test_sharpness_table_TV006(1)<0){
                                buf[0] = 14;
                        }
                        else
                        {
                                buf[0] = 18;
                        }
                                
		}
                else if (rtice_param.mode0 == 2){//V4L2_CID_EXT_VPQ_NOISE_REDUCTION
                        fwif_color_get_PQA_motion_threshold_TV006();
                        if(fwif_color_bit_test_NR_Table_TV006()<0){
                                buf[0] = 24;
                        }else{
                                buf[0] = 28;
                        }
                }
                else if (rtice_param.mode0 == 3){//V4L2_CID_EXT_VPQ_MPEG_NOISE_REDUCTION
                        fwif_color_get_PQA_motion_threshold_TV006();
                        if(fwif_color_bit_test_NR_Table_TV006()<0){
                                buf[0] = 34;
                        }else{
                                buf[0] = 38;
                        }
                }
                else if (rtice_param.mode0 == 4){//V4L2_CID_EXT_VPQ_GAMUT_3DLUT
                        if(fwif_color_bit_test_3d_lut_Table_TV006()<0){
                                buf[0] = 44;
                        }else{
                                buf[0] = 48;
                        }
                }
                else if (rtice_param.mode0 == 5){//V4L2_CID_EXT_VPQ_GAMUT_MATRIX_POST
                        if(fwif_color_bit_test_GAMUT_MATRIX_POST()<0){ 
                                buf[0] = 54;
                        }else{
                                buf[0] = 58;
                        }
                }
                else if (rtice_param.mode0 == 6){ //V4L2_CID_EXT_VPQ_DECONTOUR
                        if(fwif_color_bit_test_decontour()<0){
                                buf[0] = 64;
                        }else{
                                buf[0] = 68;
                        }
                }
                else if (rtice_param.mode0 == 7){ //V4L2_CID_EXT_VPQ_LOCALCONTRAST_DATA
                        if(fwif_color_bit_test_local_contrast()<0){ 
                                buf[0] = 74;
                        }else{
                                buf[0] = 78;
                        }
                }
                else if (rtice_param.mode0 == 8){ //V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_BYPASS_LUT
                                buf[0] = 88; //no need to compare 
                } else if (rtice_param.mode0 == 9) { // V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST
                	extern VIP_DCC_Chroma_Compensation_TABLE Chroma_Compensation_TABLE;
					extern VIP_DCC_Color_Independent_Blending_Table Color_independent;
					char tmp1 = drvif_color_get_dcc_Color_Independent_TV006(&Color_independent);
					char tmp2 = drvif_color_get_dcc_chroma_compensation_Curve_Mode_TV006(&Chroma_Compensation_TABLE);
						
					if (tmp1 == 8 && tmp2 == 8) {
						buf[0] = 98;	// match
					} else if (tmp1 == 4 || tmp2 == 4) {
						buf[0] = 94;	// not match
					} else {
						buf[0] = 90;	// error
					}
                } else if (rtice_param.mode0 == 10) { // V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_LUT
					char tmp = fwif_color_compare_dcc_DB_tv006();
                	if (tmp == 8) {
						buf[0] = 108;	// match
					} else if (tmp == 4) {
						buf[0] = 104;	// not match
					} else {
						buf[0] = 100;	// error
					}					
                } else if (rtice_param.mode0 == 11) { // V4L2_CID_EXT_VPQ_CM_DB_DATA
					char tmp = fwif_color_get_ICM_DB_tv006();
                	if (tmp == 8) {
						buf[0] = 118;	// match
					} else if (tmp == 4) {
						buf[0] = 114;	// not match
					} else {
						buf[0] = 110;	// error
					}					

                } else if (rtice_param.mode0 == 12) { // V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_COLOR_GAIN
                	unsigned char aplColorGain;
                	fwif_color_access_apl_color_gain_tv006(0, (int*)(&aplColorGain), ACCESS_MODE_READ);
                	buf[0] = aplColorGain;
				} else {
					buf[0] = 255;	// unknown mode0
				}
				*num_type = VIP_UINT8;

*/
                break;
             
	case TOOLACCESS_DisplayInformation_table:   /*  1203*/
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
			buf[0] = 0;//VipPQ_GetDltiTableSize();

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Coef_Dcti:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetDCti();
		else
			buf[0] = 0;	// VipPQ_GetNewDctiTableSize();

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
			buf[0] = 0; //  VipPQ_GetINewDctiTableSize();

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_VDcti:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetVDCti();
		else
			buf[0] =  0;	// VipPQ_GetVctiTableSize();

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
/*
	case TOOLACCESS_Function_Coef_Ma_Chroma_Error:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetMA_Chroma_Error();
		else
			buf[0] = 10;

		*num_type = VIP_UINT8;

		break;
*/
	case TOOLACCESS_Function_Coef_NM_Level:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = 0;//Scaler_GetRTNR_Noise_Measure();
		else
			buf[0] = 0;//VipPQ_GetRTNR_NM_TableSize(); merlin3 remove this item

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
			return -ID_SIZE_ERR;

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
			buf[0] = Scaler_GetDiSmd();
		else
			buf[0] = VipPQ_GetSMD_TableSize();

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
#if 0/*SLD, hack, elieli*/ //k5l remove

	case TOOLACCESS_Function_Extend_SLD:

			if (size > sizeof(unsigned char))
				return -ID_SIZE_ERR;

			if (rtice_param.mode0 == 0)
				buf[0] = Scaler_Get_SLD();
			else
				buf[0] = SLD_Table_NUM;

			*num_type = VIP_UINT8;

			break;
#endif
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

	case TOOLACCESS_Function_Extend2_Egsm_PostSHP_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_getdESM();
		else
			buf[0] = 5;
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
			buf[0] = Scaler_GetScaleUPH_8tap();
		else
			buf[0] = VipPQ_GetSU_table_8tap_TableSize();

		*num_type = VIP_UINT8;

		break;


	case TOOLACCESS_Function_Extend2_SU_V_6Tab:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetScaleUPV_6tap();
		else
			buf[0] = VipPQ_GetSU_table_6tap_TableSize();

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
			buf[0] = 9;

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
#if 0 	//k5l hw remove
	case TOOLACCESS_Function_Extend3_blue_stretch:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_Get_blue_stretch();
		else
			buf[0] = Blue_Stretch_MAX;

		break;
#endif
	case TOOLACCESS_Function_Extend3_MB_SU_Peaking:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetMBSUPeaking();
		else
			buf[0] = MBSU_table_num;

		break;


	case TOOLACCESS_Function_Coef_ALL:
		if (size > VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Coef_MAX)
			return -ID_SIZE_ERR;

	/*IoReg_Write32(NR_DCH1_IMPULSE_SMOOTHTHL_VADDR, size);*/
		if (rtice_param.mode0 == 0){
			memcpy(buf, (unsigned char *) &(gVip_Table->VIP_QUALITY_Coef), size);
		}else if (rtice_param.mode0 == 2){
			if (size > sizeof(unsigned char))
				return -ID_SIZE_ERR;
			rtice_set_Function_Coef(rtice_param, buf, 0);
		}else {
			tablesize[0] = 0;//VipPQ_GetDltiTableSize(); /*dlti*/
			tablesize[1] =  0;	// VipPQ_GetNewDctiTableSize(); /*dcti*/
			tablesize[2] = 0; /*idlti*/
			tablesize[3] = 0;	//  VipPQ_GetINewDctiTableSize();/*idcti*/
			tablesize[4] =  0;	// VipPQ_GetVctiTableSize(); /*vdcti*/
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

			buf2_tmp = (unsigned char *)vip_malloc(VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Coef_MAX);
			if (buf2_tmp == NULL)
				return -ID_SIZE_ERR;

			for (i = 0; i < VIP_QUALITY_SOURCE_NUM; i++) {
				memcpy(buf2_tmp+(i*VIP_QUALITY_Coef_MAX), &tablesize, VIP_QUALITY_Coef_MAX);
			}

			memcpy(buf, (unsigned char *)(buf2_tmp), size);
			vip_free(buf2_tmp);
			buf2_tmp = NULL;


		}
		*num_type = VIP_UINT8;



		break;
	case TOOLACCESS_Function_Extend_ALL:
		if (size > VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend_Coef_MAX)
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0){
			memcpy(buf, (unsigned char *) &(gVip_Table->VIP_QUALITY_Extend_Coef), size);
		}else if (rtice_param.mode0 == 2){
			if (size > sizeof(unsigned char))
				return -ID_SIZE_ERR;
			rtice_set_Function_Extend_Coef(rtice_param, buf, 0);
		}else {

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
			//tablesize[13] = SLD_Table_NUM;/*SLD*///k7 delete
			tablesize[14] = 10;/*Output_InvOutput_GAMMA*/
			tablesize[15] = 0;/*Tone mapping */
			tablesize[16] = 0;/*Reserve*/
			tablesize[17] = 0;/*Reserve*/
			tablesize[18] = 0;/*Reserve*/
			tablesize[19] = 0;/*Reserve*/
			tablesize[20] = 0;/*Reserve*/

			buf2_tmp = (unsigned char *)vip_malloc(VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend_Coef_MAX);

			if (buf2_tmp == NULL)
				return -ID_SIZE_ERR;

			for (i = 0; i < VIP_QUALITY_SOURCE_NUM; i++) {
				memcpy(buf2_tmp+(i*VIP_QUALITY_Extend_Coef_MAX), &tablesize, VIP_QUALITY_Extend_Coef_MAX);

			}

			memcpy(buf, (unsigned char *)(buf2_tmp), size);
			vip_free(buf2_tmp);
			buf2_tmp = NULL;


		}
		*num_type = VIP_UINT8;



		break;
	case TOOLACCESS_Function_Extend2_ALL:
		if (size > VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend2_Coef_MAX)
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0){
			memcpy(buf, (unsigned char *) &(gVip_Table->VIP_QUALITY_Extend2_Coef), size);
		}else if (rtice_param.mode0 == 2){
			if (size > sizeof(unsigned char))
				return -ID_SIZE_ERR;
			rtice_set_Function_Extend2_Coef(rtice_param, buf, 0);
		}else {

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
			tablesize[14] = VipPQ_GetSU_table_8tap_TableSize();/*SU_H_8tab*/
			tablesize[15] = VipPQ_GetSU_table_6tap_TableSize();/*SU_V_6tab*/
			tablesize[16] = 0;/*OSD_Sharpness*/
			tablesize[17] = VipPQ_GetFIR_Coef_Ctrl_TableSize();/*SU_Dir*/
			tablesize[18] = VipPQ_GetDirsu_TableSize();/*SU_Dir_Weighing*/
			tablesize[19] = 0;/*Reserve*/
			tablesize[20] = 0;/*Reserve*/

			buf2_tmp = (unsigned char *)vip_malloc(VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend2_Coef_MAX);

			if (buf2_tmp == NULL)
				return -ID_SIZE_ERR;

			for (i = 0; i < VIP_QUALITY_SOURCE_NUM; i++) {
				memcpy(buf2_tmp+(i*VIP_QUALITY_Extend2_Coef_MAX), &tablesize, VIP_QUALITY_Extend2_Coef_MAX);
			}

			memcpy(buf, (unsigned char *)(buf2_tmp), size);
			vip_free(buf2_tmp);
			buf2_tmp = NULL;


		}
		*num_type = VIP_UINT8;



		break;
	case TOOLACCESS_Function_Extend3_ALL:
		if (size > VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend3_Coef_MAX)
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0){
			memcpy(buf, (unsigned char *) &(gVip_Table->VIP_QUALITY_Extend3_Coef), size);
		}else if (rtice_param.mode0 == 2){
			if (size > sizeof(unsigned char))
				return -ID_SIZE_ERR;
			rtice_set_Function_Extend3_Coef(rtice_param, buf, 0);
		}else {

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

			buf2_tmp = (unsigned char *)vip_malloc(VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend3_Coef_MAX);

			if (buf2_tmp == NULL)
				return -ID_SIZE_ERR;

			for (i = 0; i < VIP_QUALITY_SOURCE_NUM; i++) {
				memcpy(buf2_tmp+(i*VIP_QUALITY_Extend3_Coef_MAX), &tablesize, VIP_QUALITY_Extend3_Coef_MAX);
			}

			memcpy(buf, (unsigned char *)(buf2_tmp), size);
			vip_free(buf2_tmp);
			buf2_tmp = NULL;


		}
		*num_type = VIP_UINT8;

		break;

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

	case TOOLACCESS_SoftwareBaseLocalContrastCurve:
	{
		if (size > sizeof(unsigned int)*(33))
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(33 * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		if (down_trylock(&LC_Semaphore) == 0) //get semaphore ok
		{
			drvif_color_get_SLC_Curve(pwUINT32Temp, rtice_param.mode0);
			up(&LC_Semaphore);
		}
		else
		{
			memset(pwUINT32Temp, 0, size);
		}

		memcpy(buf, (unsigned char*)pwUINT32Temp, size);
		fwif_color_ChangeINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);
		vip_free(pwUINT32Temp);
		*num_type = VIP_UINT32;

	}
	break;

	case TOOLACCESS_SoftwareBaseLocalContrastHistogram:
	{
		if (size > sizeof(unsigned int)*(16))
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(16 * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		if (down_trylock(&LC_Semaphore) == 0) //get semaphore ok
		{
			drvif_color_get_SLC_Hist(pwUINT32Temp, rtice_param.mode0, rtice_param.mode1);
			up(&LC_Semaphore);
		}
		else
		{
			memset(pwUINT32Temp, 0, size);
		}

		memcpy(buf, (unsigned char*)pwUINT32Temp, size);
		fwif_color_ChangeINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);
		vip_free(pwUINT32Temp);
		*num_type = VIP_UINT32;

	}
	break;

	case TOOLACCESS_ColorFacTable:
	{
		extern StructColorDataFacModeType g_curColorFacTableEx;
		unsigned char *tmp = (unsigned char *)&g_curColorFacTableEx;
		
		if (size+start_addr > sizeof(g_curColorFacTableEx))
			return -ID_SIZE_ERR;
		
		memcpy(buf, tmp+start_addr, size);
		
		*num_type = VIP_UINT8;
	}
	break;

	case TOOLACCESS_SRGB_33_Matrix:

		if (size > 9 * sizeof(short))
			return -ID_SIZE_ERR;

		pwINT16Temp = (short *)vip_malloc(9 * sizeof(short));

		if (pwINT16Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwINT16Temp, (char *)sRGB_APPLY, size);

		fwif_color_ChangeUINT16Endian(pwINT16Temp, 9, 1);

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

		*num_type = VIP_UINT16;
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

	case TOOLACCESS_ICM_K5_STRUCTURE:
	{
		extern UINT8 g_Color_Mode;
		extern CHIP_CM_REGION_T g_cm_rgn_adv;
		extern CHIP_CM_REGION_T g_cm_rgn_exp;
		extern CHIP_COLOR_CONTROL_T g_cm_ctrl_adv;
		extern CHIP_COLOR_CONTROL_T g_cm_ctrl_exp;
		CHIP_CM_REGION_T *cm_rgn_tmp;
		CHIP_COLOR_CONTROL_T *cm_ctrl_tmp;
		UINT8 i;
		UINT32 index = 0;
		if (size > TV006_ICM_DB_ITEM_NUM * sizeof(short))
			return -ID_SIZE_ERR;

		pwINT16Temp = (short *)vip_malloc(TV006_ICM_DB_ITEM_NUM * sizeof(short));

		if (pwINT16Temp == NULL)
			return -ID_SIZE_ERR;

		if (g_Color_Mode == COLOR_MODE_EXP) {
			cm_rgn_tmp = &g_cm_rgn_exp;
			cm_ctrl_tmp = &g_cm_ctrl_exp;
		} else {
			cm_rgn_tmp = &g_cm_rgn_adv;
			cm_ctrl_tmp = &g_cm_ctrl_adv;
		}

		//copy K5 sturcture to UINT16
		/*copy stColorRegion*/
		for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++)
		{
			pwINT16Temp[index++] = cm_rgn_tmp->stColorRegionType.stColorRegion[i].enable;
			pwINT16Temp[index++] = cm_rgn_tmp->stColorRegionType.stColorRegion[i].regionNum;
			memcpy((pwINT16Temp + index), &(cm_rgn_tmp->stColorRegionType.stColorRegion[i].hue_x[0]), 8 * sizeof(short));
			index = index + 8;
			memcpy((pwINT16Temp + index), &(cm_rgn_tmp->stColorRegionType.stColorRegion[i].hue_g[0]), 8 * sizeof(short));
			index = index + 8;
			memcpy((pwINT16Temp + index), &(cm_rgn_tmp->stColorRegionType.stColorRegion[i].sat_x[0]), 8 * sizeof(short));
			index = index + 8;
			memcpy((pwINT16Temp + index), &(cm_rgn_tmp->stColorRegionType.stColorRegion[i].sat_g[0]), 8 * sizeof(short));
			index = index + 8;
			memcpy((pwINT16Temp + index), &(cm_rgn_tmp->stColorRegionType.stColorRegion[i].Int_x[0]), 8 * sizeof(short));
			index = index + 8;
			memcpy((pwINT16Temp + index), &(cm_rgn_tmp->stColorRegionType.stColorRegion[i].Int_g[0]), 8 * sizeof(short));
			index = index + 8;
			memcpy((pwINT16Temp + index), &(cm_rgn_tmp->stColorRegionType.stColorRegion[i].r_offset_by_hue[0]), 8 * sizeof(short));
			index = index + 8;
			memcpy((pwINT16Temp + index), &(cm_rgn_tmp->stColorRegionType.stColorRegion[i].g_offset_by_hue[0]), 8 * sizeof(short));
			index = index + 8;
			memcpy((pwINT16Temp + index), &(cm_rgn_tmp->stColorRegionType.stColorRegion[i].b_offset_by_hue[0]), 8 * sizeof(short));
			index = index + 8;
		}
		/*copy gain_by_sat(int)_for_rgb_offset*/
		memcpy((pwINT16Temp + index), &(cm_rgn_tmp->stColorRegionType.gain_by_sat_for_rgb_offset[0]), 8 * sizeof(short));
		index = index + 8;
		memcpy((pwINT16Temp + index), &(cm_rgn_tmp->stColorRegionType.gain_by_int_for_rgb_offset[0]), 8 * sizeof(short));
		index = index + 8;

		/*copy keepY*/
		pwINT16Temp[index] = cm_rgn_tmp->stColorRegionType.keepY;

		for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++)
		{
			pwINT16Temp[TV006_ICM_DB_COLOR_REGION_NUM + i * 5] = cm_ctrl_tmp->stColor[i].masterGain;
			pwINT16Temp[TV006_ICM_DB_COLOR_REGION_NUM + 1 + i * 5] = cm_ctrl_tmp->stColor[i].RGBOffset_masterGain;
			pwINT16Temp[TV006_ICM_DB_COLOR_REGION_NUM + 2 + i * 5] = cm_ctrl_tmp->stColor[i].stColorGain.stColorGainHue;
			pwINT16Temp[TV006_ICM_DB_COLOR_REGION_NUM + 3 + i * 5] = cm_ctrl_tmp->stColor[i].stColorGain.stColorGainSat;
			pwINT16Temp[TV006_ICM_DB_COLOR_REGION_NUM + 4 + i * 5] = cm_ctrl_tmp->stColor[i].stColorGain.stColorGainInt;
		}
		fwif_color_ChangeUINT16Endian(pwINT16Temp, TV006_ICM_DB_ITEM_NUM, 1);

		//snt to tool
		memcpy(buf, (char *)pwINT16Temp, size);

		*num_type = VIP_UINT16;
		vip_free(pwINT16Temp);
		pwINT16Temp = NULL;

		break;
	}

	case TOOLACCESS_LOGO_DETECT_GET:
	{
		if (size > sizeof(unsigned char)*4)
			return -ID_SIZE_ERR;
		buf[0] = 0;
		buf[1] = 0;
		buf[2] = 0;
		buf[3] = 0;
		*num_type = VIP_UINT8;
		break;
	}

	case TOOLACCESS_GSR_BLK_APL_GET:
	{
/*
		if (size > sizeof(unsigned short)*60*34)
			return -ID_SIZE_ERR;

		rtd_pr_vpq_info("TOOLACCESS_GSR_BLK_APL_GET\n");

		pwUINT16Temp = (unsigned short *)vip_malloc(sizeof(unsigned short)*60*34);

		if(pwUINT16Temp == NULL)
			return -ID_SIZE_ERR;

		if(rtice_param.mode0 == 0)
		{
			for(j=0; j<34; j++)
			{
				for(i=0; i<60; i++)
				{
					pwUINT16Temp[j*60+i] = pApl[j][i];
				}
			}
		}
		else
		{
			for(i=0; i<2040; i++)
				pwUINT16Temp[j*60+i] = LC_APL[i];
		}
		fwif_color_ChangeUINT16Endian(pwUINT16Temp, 60*34, 1);
		memcpy(buf, pwUINT16Temp, size);

		vip_free(pwUINT16Temp);
		pwUINT16Temp = NULL;

		*num_type = VIP_UINT16;
*/
		break;
	}

	case TOOLACCESS_DSE_Gain:
	{
		if (size > sizeof(unsigned short)* (VPQ_SaturationLUT_Seg_Max * 3))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0) {
			//VPQ_SAT_LUT_T lut;
			extern VPQ_SAT_LUT_T g_sat_LUT;
			extern VPQ_SAT_LUT_T g_sat_gain;
			extern VPQ_SAT_LUT_T g_total_sat_gain;
			pwINT16Temp = (unsigned short *)vip_malloc((VPQ_SaturationLUT_Seg_Max * 3) * sizeof(unsigned short));
			if (pwINT16Temp) {
				//sat gain
				//fwif_color_access_Sat_Gain(_MAIN_DISPLAY, &lut, ACCESS_MODE_GET);
				memcpy(pwINT16Temp, g_sat_gain.uSaturationY, VPQ_SaturationLUT_Seg_Max * sizeof(unsigned short));

				//DSE
				//fwif_color_access_SaturationLUT(_MAIN_DISPLAY, &lut, ACCESS_MODE_GET);
				memcpy(pwINT16Temp + VPQ_SaturationLUT_Seg_Max, g_sat_LUT.uSaturationY, VPQ_SaturationLUT_Seg_Max * sizeof(unsigned short));


				//total gain
				//fwif_color_access_Total_Sat_Gain(_MAIN_DISPLAY, &lut, ACCESS_MODE_GET);
				memcpy(pwINT16Temp + (VPQ_SaturationLUT_Seg_Max * 2), g_total_sat_gain.uSaturationY, VPQ_SaturationLUT_Seg_Max * sizeof(unsigned short));


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
	case TOOLACCESS_OD_Table_33x33:
	{
		extern unsigned char od_target_table_g[OD_table_length];
		if (size > sizeof(unsigned char)*OD_table_length)
			return -ID_SIZE_ERR;

		drvif_color_od_table_33x33_get(rtice_param.mode0, od_target_table_g);

		memcpy(buf, od_target_table_g, size);

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_OD_Table_17x17:
	{
		extern unsigned char od_target_table_g[OD_table_length];
		if (size > sizeof(unsigned char)*OD_table_length)
			return -ID_SIZE_ERR;

		// add for rtice tool
		drvif_color_od_table_17x17_get(od_target_table_g);

		memcpy(buf, od_target_table_g, size);

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_DI_PAN_detect:
	{
		if (size > sizeof(unsigned short)*pan_detect_var)
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(pan_detect_var * sizeof(unsigned short));
		if(pwUINT16Temp == NULL)
			return -ID_SIZE_ERR;

		drvif_color_ma_Pan_Detection_get(pwUINT16Temp);

		fwif_color_ChangeUINT16Endian(pwUINT16Temp, pan_detect_var, 1);

		memcpy(buf, pwUINT16Temp, size);
		vip_free(pwUINT16Temp);
		pwUINT16Temp = NULL;

		*num_type = VIP_UINT16;
		break;
	}
        case TOOLACCESS_VPQ_IOCTL_DataAccess_cmd:
        {

		if ((size +  start_addr)> sizeof(VIP_DRV_DataAccess_Debug))
			return -ID_SIZE_ERR;

		//pwUINT32Temp = (unsigned int *)vip_malloc(sizeof(VIP_DRV_DataAccess_Debug));
		//if(pwUINT32Temp == NULL)
			//return -ID_SIZE_ERR;

                /*pwUINT32Temp[0] = vip_data_access_debug.input_x;
                pwUINT32Temp[1] = vip_data_access_debug.input_y;
                pwUINT32Temp[2] = vip_data_access_debug.uzu_x;
                pwUINT32Temp[3] = vip_data_access_debug.uzu_y;
                pwUINT32Temp[4] = vip_data_access_debug.output_x;
                pwUINT32Temp[5] = vip_data_access_debug.output_y;
                pwUINT32Temp[6] = vip_data_access_debug.i_cross_bar_en;
                pwUINT32Temp[7] = vip_data_access_debug.uzd_cross_bar_en;
                pwUINT32Temp[8] = vip_data_access_debug.d_cross_bar_en;
                pwUINT32Temp[9] = vip_data_access_debug.debug_en;*/
               
		pwUINT32Temp = ((unsigned int*)&vip_data_access_debug) + start_addr; 
		memcpy(buf, (unsigned char*)pwUINT32Temp, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(unsigned int), 1);

		//vip_free(pwUINT32Temp);
		//pwUINT32Temp = NULL;

		*num_type = VIP_UINT32;
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
	case TOOLACCESS_VPQ_HDR_IOCTL_StopRun_cmd:
	{
		int i;

		if (size > sizeof(unsigned char)*256)
			return -ID_SIZE_ERR;

		for (i = 0; i < 256 && i < size/sizeof(unsigned char); i++)
			buf[i] = vpq_HDR_ioctl_get_stop_run_by_idx(i);

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_VPQ_dolbyHDR_IOCTL_StopRun_cmd:
	{
		int i;

		if (size > sizeof(unsigned char)*256)
			return -ID_SIZE_ERR;

		for (i = 0; i < 256 && i < size/sizeof(unsigned char); i++)
			buf[i] = vpq_dolbyHDR_ioctl_get_stop_run_by_idx(i);

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
		if (size > sizeof(DRV_TC_HDR_CTRL))
			return -ID_SIZE_ERR;
		pTC_HDR_CTRL = (DRV_TC_HDR_CTRL*)fwif_color_Get_TC_CTRL();

		pTC_HDR_CTRL->TC_Debug_Log = RPC_system_setting_info->HDR_info.Debug_Log_2;
		pTC_HDR_CTRL->TC_Debug_Delay = RPC_system_setting_info->HDR_info.Debug_Log_Delay_Frame;

		memcpy(	&buf[0], (unsigned char*)pTC_HDR_CTRL, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		*num_type = VIP_UINT32;

		break;

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

	case TOOLACCESS_LOCALCONTRAST_DEMO:
		buf[0] = 1;
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_CHANGE_PQ_FLOW:
		buf[0] = PQ_FLOW_Mode;
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DM_HDR_3D_Lut_TBL:
		if (size > (sizeof(unsigned int) * (VIP_DM_HDR_3D_LUT_UI_TBL_SIZE)))
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(VIP_DM_HDR_3D_LUT_UI_TBL_SIZE * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(VIP_DM_HDR_3D_LUT_UI_TBL_SIZE * sizeof(short));
		if (pwUINT16Temp == NULL) {
			vip_free(pwUINT32Temp);
			return -ID_SIZE_ERR;
		}

		//fwif_color_get_Read_DM_HDR_3dLUT(buf2_tmp, pwUINT32Temp);

		fwif_color_rtice_DM2_3D_17x17x17_LUT_Get(pwUINT16Temp, HDR_3DLUT_getFromReg);

		for(i=0;i<size/sizeof(int);i++)
			pwUINT32Temp[i] = pwUINT16Temp[i];

		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/sizeof(int), 1);

		memcpy(buf, pwUINT32Temp, size);
		*num_type = VIP_UINT32;

		vip_free(pwUINT16Temp);
		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;
		pwUINT16Temp = NULL;
		break;

	case TOOLACCESS_DM2_HDR_EOTF_TBL:
	case TOOLACCESS_DM2_HDR_OETF_TBL:
	case TOOLACCESS_DM2_HDR_Tone_Mapping_TBL:
	case TOOLACCESS_DM2_HDR_24x24x24_3D_TBL:
	case TOOLACCESS_DM2_HDR_Hist_RGB_max:
	case TOOLACCESS_DM2_HDR_Hist_Auto_Mode:
	case TOOLACCESS_DM2_HDR_Hist_Auto_Mode_TBL:
	case TOOLACCESS_DM2_HDR_Sat_Hist_TBL:
	case TOOLACCESS_DM2_HDR_Hue_Hist_TBL:
	case TOOLACCESS_DM2_HDR_Sat_Hue_Hist_Skip:
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

		if (rtice_param.mode0 == 1) {
			for (i=0;i<VIP_D_3D_LUT_SIZE;i++) {
				pwUINT32Temp[i*3+0] = g_buf3DLUT_LGDB.pt[i].r_data;
				pwUINT32Temp[i*3+1] = g_buf3DLUT_LGDB.pt[i].g_data;
				pwUINT32Temp[i*3+2] = g_buf3DLUT_LGDB.pt[i].b_data;
			}
		} else {
			fwif_color_get_Read_D_3dLUT(buf2_tmp, pwUINT32Temp);
			fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/sizeof(int), 1);
		}
		memcpy(buf, pwUINT32Temp, size);
		*num_type = VIP_UINT32;

		vip_free(buf2_tmp);
		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		break;

	case TOOLACCESS_DM2_EOTF_byLuminance_CTRL:
		if (size > (sizeof(unsigned short)*2))
			return -ID_SIZE_ERR;

		memcpy((unsigned char*)&buf[0], (unsigned char*)&VIP_HDR_EOTF_ByLuminace_CTRL[0], size);

		fwif_color_ChangeUINT16Endian((unsigned short*)&buf[0], size/sizeof(short), 1);

		*num_type = VIP_UINT16;

		break;

	case TOOLACCESS_DM2_OETF_byLuminance_CTRL:
		if (size > (sizeof(unsigned short)))
			return -ID_SIZE_ERR;

		memcpy((unsigned char*)&buf[0], (unsigned char*)&(HDR10_OETF_ByLuminace_TBL.panel_Luminace), size);

		fwif_color_ChangeUINT16Endian((unsigned short*)&buf[0], size/sizeof(short), 1);

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

		rtice_Get_VIP_System_Info(system_setting_info, RPC_system_setting_info, RPC_smartPic_clue, gVip_Table, pwUINT32Temp);
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

	case TOOLACCESS_VIP_DeMura_TBL_Mode:
	case TOOLACCESS_VIP_DeMura_TBL:
	case TOOLACCESS_VIP_DeMura_INI:
	case TOOLACCESS_VIP_DeMura_Adaptive:
	case TOOLACCESS_VIP_DeMura_Channel_Mode:
	case TOOLACCESS_VIP_DeMura_READ_INX_DATA:
		if(rtice_SetGet_DeMura(&rtice_param, num_type, buf, system_setting_info, 0) < 0)
			return -ID_SIZE_ERR;

		break;
	case TOOLACCESS_Sharpness_Table_TV006:
	{
		if (size > sizeof(short)*600)
			return -ID_SIZE_ERR;

		pwINT16Temp = (unsigned short *)vip_malloc(sizeof(short)*600);
		if(pwINT16Temp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_Get_sharpness_table_TV006(pwINT16Temp);
		fwif_color_ChangeINT16Endian(pwINT16Temp, size/sizeof(short), 1);
		memcpy(buf, pwINT16Temp, size);

		vip_free(pwINT16Temp);
		pwINT16Temp = NULL;

		*num_type = VIP_INT16;

		break;
	}
	case TOOLACCESS_NR_Table_TV006:
	{
		if (size > (sizeof(UINT16)*295))
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(sizeof(short)*295);
		if(pwUINT16Temp == NULL)
			return -ID_SIZE_ERR;
		fwif_color_get_PQA_motion_threshold_TV006();

		drvif_color_get_DB_NR(pwUINT16Temp);

		fwif_color_ChangeUINT16Endian(pwUINT16Temp, size/sizeof(short), 1);
		memcpy(buf, pwUINT16Temp, size);

		vip_free(pwUINT16Temp);
		pwUINT16Temp = NULL;

		*num_type = VIP_UINT16;

		break;
	}

		case TOOLACCESS_OPS_En:

			if (size > (sizeof(short)))
				return -ID_SIZE_ERR;

			pwUINT16Temp = &OPS_CTRL_item.OPS_enable;

			memcpy(buf, pwUINT16Temp, size);
			fwif_color_ChangeUINT16Endian((short*)buf, 1, 1);

			*num_type = VIP_UINT16;

		break;

		case TOOLACCESS_OPS_TPC:

			if ((size+rtice_param.saddr) > (sizeof(VIP_OPS_TPC_Item)))
				return -ID_SIZE_ERR;

			pwUINT16Temp = (unsigned short*)&OPS_CTRL_item.OPS_TPC_Item;
			pwUINT16Temp = pwUINT16Temp + rtice_param.saddr;

			memcpy(buf, pwUINT16Temp, size);
			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			*num_type = VIP_UINT16;

		break;

		case TOOLACCESS_OPS_LBC:

			if ((size+rtice_param.saddr) > (sizeof(VIP_OPS_LBC_Item)))
				return -ID_SIZE_ERR;

			pwUINT16Temp = (unsigned short*)&OPS_CTRL_item.OPS_LBC_Item;
			pwUINT16Temp = pwUINT16Temp + rtice_param.saddr;

			memcpy(buf, pwUINT16Temp, size);
			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			*num_type = VIP_UINT16;

		break;

		case TOOLACCESS_OPS_CPC:

			if ((size+rtice_param.saddr) > (sizeof(VIP_OPS_CPC_Item)))
				return -ID_SIZE_ERR;

			pwUINT16Temp = (unsigned short*)&OPS_CTRL_item.OPS_CPC_Item;
			pwUINT16Temp = pwUINT16Temp + rtice_param.saddr;

			memcpy(buf, pwUINT16Temp, size);
			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			*num_type = VIP_UINT16;

		break;

		case TOOLACCESS_OPS_PLC_Curve:

			if ((size+rtice_param.saddr) > (sizeof(short)*8*4))
				return -ID_SIZE_ERR;

			buf_tmp8 = (unsigned char *)vip_malloc(8 * sizeof(unsigned char));
			if (buf_tmp8 == NULL)
				return -ID_SIZE_ERR;

#ifdef _VIP_Mer7_Compile_Error__
			//panel_get_PLC_curve(buf_tmp8);
#endif
			for (i=0;i<8;i++)
				OPS_CTRL_item.PLC_curve_read[i] = buf_tmp8[i];

			pwUINT16Temp = (unsigned short*)&OPS_CTRL_item.PLC_curve_TPC[0];
			pwUINT16Temp = pwUINT16Temp + rtice_param.saddr;

			memcpy(buf, pwUINT16Temp, size);
			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			*num_type = VIP_UINT16;
			vip_free(buf_tmp8);
		break;

		case TOOLACCESS_OPS_Debug:

			if ((size+rtice_param.saddr) > (sizeof(VIP_OPS_Debug_Item)))
				return -ID_SIZE_ERR;

			pwUINT16Temp = (unsigned short*)&OPS_CTRL_item.OPS_Debug_Item;
			pwUINT16Temp = pwUINT16Temp + rtice_param.saddr;

			memcpy(buf, pwUINT16Temp, size);
			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			*num_type = VIP_UINT16;

		break;

		case TOOLACCESS_Force_MEX_En_ALL:

			if ((size+rtice_param.saddr) > (sizeof(char)))
				return -ID_SIZE_ERR;

			buf[0] = MEX_MODE_Force_En_All; 

			*num_type = VIP_UINT8;

		break;

/*
	case TOOLACCESS_VIP_SLD:
		if (size > (sizeof(int)*8))
			return -ID_SIZE_ERR;

		*num_type = VIP_INT32;
		break;
*/
	case TOOLACCESS_VIP_SLD:/*k7, SLD*/
		if (size > (sizeof(int)*50))
			return -ID_SIZE_ERR;

		pwINT32Temp = (int *)vip_malloc(50 * sizeof(int));
		if (pwINT32Temp == NULL)
			return -ID_SIZE_ERR;

		pwINT32Temp[0] = cmd_dY_preCur_mouseCon;
		pwINT32Temp[1] = cmd_LC_period;
		pwINT32Temp[2] = cmd_thl_counter_UR_con;
		pwINT32Temp[3] = cmd_counter_frame_highY_blk;
		pwINT32Temp[4] = cmd_lock_fw_gain_frame;
		pwINT32Temp[5] = cmd_pixel_gain_step;
		pwINT32Temp[6] = cmd_thl_counter_blk_pre_cur_same_mouseCon;
		pwINT32Temp[7] = cmd_thl_error_time_box_logo_case;
		pwINT32Temp[8] = cmd_thl_counter_box_logo_case;
		pwINT32Temp[9] = cmd_thl_error_time_NEWS_case;
		pwINT32Temp[10] = cmd_thl_counter_NEWS_case;
		pwINT32Temp[11] = cmd_thl_error_time_noise_case;
		pwINT32Temp[12] = cmd_thl_counter_noise_case;
		pwINT32Temp[13] = cmd_thl_dC;
		pwINT32Temp[14] = cmd_thl_dUDL;
		pwINT32Temp[15] = cmd_rgb_box_leave_idx_delete_step;
		pwINT32Temp[16] = cmd_box_logo_pixel_gain;
		pwINT32Temp[17] = cmd_box_logo_maxGain_time;
		pwINT32Temp[18] = cmd_box_lock_frame;


		memcpy(	&buf[0], (unsigned char*)pwINT32Temp, size);
		fwif_color_ChangeINT32Endian(( int*)&buf[0], size/sizeof(int), 1);
		*num_type = VIP_INT32;
		vip_free(pwINT32Temp);

		break;

	case TOOLACCESS_VIP_SLD_BLK_APL:/*k7, SLD*/
		//if (size > (sizeof(int)*50))	// size check in driver
			//return -ID_SIZE_ERR;

		pwINT16Temp = (unsigned short *)vip_malloc(size);
		if (pwINT16Temp == NULL) {
			*num_type = VIP_INT16;
			break;
		} 
		if (fwif_color_fwif_color_Get_SLD_APL(pwINT16Temp, size/sizeof(short)) < 0)
		{
			vip_free(pwINT16Temp);
			return -ID_SIZE_ERR;
		}
		memcpy(&buf[0], (unsigned char*)pwINT16Temp, size);
		fwif_color_ChangeINT16Endian(( short*)&buf[0], size/sizeof(short), 1);
		*num_type = VIP_INT16;
		vip_free(pwINT16Temp);

		break;

	case TOOLACCESS_VIP_LEA_apply_filter:
		if ((rtice_param.saddr*sizeof(short)+size) > sizeof(sld_work_filter_struct))
			return -ID_SIZE_ERR;

		pwUINT16Temp = &sld_work_filter.filter_w[0];

		memcpy(buf, pwUINT16Temp+rtice_param.saddr, size);
		fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);
		
		*num_type = VIP_INT16;

		break;

	case TOOLACCESS_VIP_LEA_debug:
		if ((rtice_param.saddr*sizeof(short)+size) > sizeof(sld_work_debug_struct))
			return -ID_SIZE_ERR;

		pwUINT16Temp = &sld_work_debug.pr_En;

		memcpy(buf, pwUINT16Temp+rtice_param.saddr, size);
		fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);
		
		*num_type = VIP_INT16;

		break;

	case TOOLACCESS_VIP_LEA:
		if ((rtice_param.saddr*sizeof(int) + size) > (sizeof(int)*26))
			return -ID_SIZE_ERR;

		pwINT32Temp = (int *)vip_malloc(26 * sizeof( int));

		if (pwINT32Temp == NULL) {
			rtd_pr_vpq_emerg("TOOLACCESS_VIP_LEA, pwINT32Temp = NULL\n");
			break;
		} 
		
		pwINT32Temp[0]	= LSC_by_memc_logo_en;
		pwINT32Temp[1]	= memc_logo_to_demura_wait_time;
		pwINT32Temp[2]	= memc_logo_to_demura_drop_time;
		pwINT32Temp[3]	= memc_logo_to_demura_drop_limit;
		pwINT32Temp[4]	= memc_logo_to_demura_ascend_speed;
		pwINT32Temp[5]	= memc_logo_read_en;
		pwINT32Temp[6]	= logo_to_demura_en;
		pwINT32Temp[7]	= memc_logo_fill_hole_en;
		pwINT32Temp[8]	= memc_logo_filter_en;
		pwINT32Temp[9]	= memc_logo_to_demura_update_cnt;
		pwINT32Temp[10] = (unsigned int)before_phyAddr;
		pwINT32Temp[11] = (unsigned int)after_phyAddr;
		pwINT32Temp[12] = (unsigned long)LD_virAddr;
		pwINT32Temp[13] = sld_ddr_offset_auto_get;
		pwINT32Temp[14] = (unsigned int)sld_ddr_offset;
		pwINT32Temp[15] = (unsigned int)dvr_to_phys((void*)LD_virAddr);
		pwINT32Temp[16] = (unsigned int)SLD_SW_En;
		pwINT32Temp[17] = (unsigned int)SLD_BY_MEMC_DEMURA_LOGO_ONLY;
		pwINT32Temp[18] = (unsigned int)memc_logo_LD_APL_check;
		pwINT32Temp[19] = (unsigned int)memc_logo_to_demura_APL_max_th;
		pwINT32Temp[20] = (unsigned int)fill_hole_th;
		pwINT32Temp[21] = (unsigned int)memc_logo_to_demura_drop_limit_gain;
		pwINT32Temp[22] = (unsigned int)memc_logo_filter_SLD_APL_SE_en;
		pwINT32Temp[23] = (unsigned int)memc_logo_to_demura_drop_limit_gain_SP;
		pwINT32Temp[24] = (unsigned int)SLD_APL_reset_cnt_th;
		pwINT32Temp[25] = (unsigned int)Gloabal_SLD_En;
		
		memcpy( &buf[0], (unsigned char*)(pwINT32Temp+rtice_param.saddr), size);
		fwif_color_ChangeINT32Endian((int*)&buf[0], size/sizeof(int), 1);
		*num_type = VIP_INT32;
		vip_free(pwINT32Temp);

		break;

	case TOOLACCESS_VIP_LEA_logo_flag_map_raw:
		if ((rtice_param.saddr+size) > (sizeof(char)*192*540))
			return -ID_SIZE_ERR;

		buf_tmp8 = &logo_flag_map_raw[0];

		memcpy(buf, buf_tmp8+rtice_param.saddr, size);

		*num_type = VIP_INT8;

		break;

	case TOOLACCESS_VIP_LEA_logo_flag_map:
		if ((rtice_param.saddr+size) > (sizeof(char)*271*481))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 1)
			buf_tmp8 = &logo_flag_map_no_fill_hole[0];
		else
			buf_tmp8 = &logo_flag_map[0];

		memcpy(buf, buf_tmp8+rtice_param.saddr, size);

		*num_type = VIP_INT8;

		break;

	case TOOLACCESS_VIP_LEA_logo_flag_map_buf:
		if ((rtice_param.saddr+size) > (sizeof(char)*271*481))
			return -ID_SIZE_ERR;

		buf_tmp8 = &logo_flag_map_buf[0];

		memcpy(buf, buf_tmp8+rtice_param.saddr, size);

		*num_type = VIP_INT8;

		break;

	case TOOLACCESS_VIP_LEA_logo_demura_counter:
		if ((rtice_param.saddr*sizeof(short)+size) > (sizeof(short)*271*481))
			return -ID_SIZE_ERR;

		pwUINT16Temp = &logo_demura_counter[0];

		memcpy(buf, pwUINT16Temp+rtice_param.saddr, size);
		fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);
		
		*num_type = VIP_INT16;

		break;

	case TOOLACCESS_VIP_LEA_demura_tbl:
		if ((rtice_param.saddr*sizeof(short)+size) > (sizeof(short)*271*481))
			return -ID_SIZE_ERR;
		
		i = rtice_param.mode0;

		if (rtice_param.mode1 ==1) {
			if (i>=5)
				i = 0; 
			pwUINT16Temp = &demura_tbl[i][0];
		} else {
			if (i>=3)
				i = 0;			
			pwUINT16Temp = &demura_apply[i][0];
		}
		memcpy(buf, pwUINT16Temp+rtice_param.saddr, size);
		fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);
		
		*num_type = VIP_INT16;

		break;		

	case TOOLACCESS_VIP_LEA_LD_APL:
		if ((rtice_param.saddr*sizeof(int)+size) > sizeof(blk_apl_maxfilter))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 1)
			pwUINT32Temp = &blk_apl_average[0];
		else
			pwUINT32Temp = &blk_apl_maxfilter[0];
		
		memcpy(buf, pwUINT32Temp+rtice_param.saddr, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(short), 1);
		
		*num_type = VIP_INT32;

		break;		

	case TOOLACCESS_VIP_LEA_blk_apl_interp:
		if ((rtice_param.saddr*sizeof(int)+size) > sizeof(blk_apl_interp))
			return -ID_SIZE_ERR;
		pwUINT32Temp = &blk_apl_interp[0];
		memcpy(buf, pwUINT32Temp+rtice_param.saddr, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(short), 1);
		
		*num_type = VIP_INT32;

		break;	

	case TOOLACCESS_VIP_SLD_Hybrid_logo_Debug_Show:
		if (size > sizeof(char))
			return -ID_SIZE_ERR;
		memcpy(buf, &SLD_Hybrid_mode_logo_debug_show, size);
		
		*num_type = VIP_INT8;

		break;

	case TOOLACCESS_VIP_SLD_swap_location:
		if (size > sizeof(char))
			return -ID_SIZE_ERR;
		SLD_swap_location = drvif_color_Get_SLD_swap_location();
		memcpy(buf, &SLD_swap_location, size);
		
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_VIP_SLD_drop_gain:
		if (size > sizeof(int))
			return -ID_SIZE_ERR;
		//memcpy(buf, &memc_logo_to_demura_drop_limit, size);
		buf[0] = (unsigned char)memc_logo_to_demura_drop_limit;
		
		*num_type = VIP_INT8;

		break;

	case TOOLACCESS_VIP_SLD_Global_SLD_CTRL:
		if (size+start_addr > sizeof(VIP_SLD2GlobalSLD_CTRL))
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short*)drvif_fwif_color_get_SLD2GlobalSLD_CTRL();
		
		memcpy(buf, pwUINT16Temp+start_addr, size);
		
		fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);

		*num_type = VIP_INT16;

		break;

	case TOOLACCESS_VIP_SLD_Global_SLD_INFO:
		if (size+start_addr > sizeof(VIP_SLD2GlobalSLD_INFO))
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short*)drvif_fwif_color_get_SLD2GlobalSLD_INFO();
		
		memcpy(buf, pwUINT16Temp+start_addr, size);

		fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);
		
		*num_type = VIP_INT16;

		break;

	case TOOLACCESS_VIP_SLD_Global_SLD_TBL:
		if (size+start_addr > sizeof(DRV_GSR2_Table))
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short*)drvif_fwif_color_get_SLD2GlobalSLD_TBL();
		
		memcpy(buf, pwUINT16Temp+start_addr, size);

		fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);
		
		*num_type = VIP_INT16;

		break;

	case TOOLACCESS_VIP_Force_vTop:
		if(rtice_SetGet_Froce_vTop_TBL(&rtice_param, num_type, buf, system_setting_info, 0) < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_VIP_ISR_set_PWM_Enable:
		if(size > sizeof(char))
			return -ID_SIZE_ERR;
		*buf = set_DCR_PWM_Enable;
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_VIP_set_DCR_Parameter:
		if(size > sizeof(int) * 7)
			return -ID_SIZE_ERR;
		pwUINT32Temp = (unsigned int *)vip_malloc(sizeof(int)*7);
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		*pwUINT32Temp = system_setting_info->OSD_Info.isIncreaseMode_Flag;
		*(pwUINT32Temp+1) = system_setting_info->OSD_Info.backlightLevel_uiMax;
		*(pwUINT32Temp+2) = system_setting_info->OSD_Info.backlightLevel_uiMin;
		*(pwUINT32Temp+3) = system_setting_info->OSD_Info.backlightLevel_actMax;
		*(pwUINT32Temp+4) = system_setting_info->OSD_Info.backlightLevel_actMin;
		*(pwUINT32Temp+5) = system_setting_info->OSD_Info.OSD_DCR_Mode;
		*(pwUINT32Temp+6) = system_setting_info->OSD_Info.OSD_Backlight;
		fwif_color_ChangeUINT32Endian(pwUINT32Temp, 7, 1);
		memcpy(buf, pwUINT32Temp, size);
		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		*num_type = VIP_UINT32;

	break;

	case TOOLACCESS_VIP_ICM_DMA_DEBUG:
	{
		break;
	}
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

	case TOOLACCESS_Set_VIP_Disable_IPQ:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetIPQ_EN();
		*num_type = VIP_UINT8;
	break;

	case TOOLACCESS_Set_VIP_Disable_IPQCurve:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetIPQCurve_EN();
		*num_type = VIP_UINT8;
	break;
    
	case TOOLACCESS_VPQ_DynamicOptimizeSystem:
	{
		extern unsigned char DynamicOptimizeSystem[255];
		if (size > (sizeof(DynamicOptimizeSystem)))
			return -ID_SIZE_ERR;

		memcpy(buf, DynamicOptimizeSystem, size);
		*num_type = VIP_UINT8;
		break;
	}
	break;

	case TOOLACCESS_SQM_PQA_PARAM:
	{
		unsigned int param[2];
		extern unsigned int g_PQA_IIR_Weight;
		extern unsigned int g_PQA_SRNN_cnt_th;

		if (size+start_addr*sizeof(unsigned int) > sizeof(param))
			return -ID_SIZE_ERR;

		param[0] = g_PQA_IIR_Weight;
		param[1] = g_PQA_SRNN_cnt_th;

		if (size >= sizeof(unsigned int)) {
			memcpy(buf, param+start_addr, size);
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);
		}
		*(num_type) = VIP_UINT32;
	}
	break;

/************************************************************/
/******************* For TV002 use*****************************/
/************************************************************/
	case TOOLACCESS_TV002_STYLE:
		if (size > (sizeof(char)))
			return -ID_SIZE_ERR;

		*buf = Scaler_access_Project_TV002_Style(0, 0);

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
	case TOOLACCESS_DBC_LUT_TV002:
	case TOOLACCESS_DBC_DCC_S_High_CMPS_LUT_TV002:
	case TOOLACCESS_DBC_DCC_S_Index_CMPS_LUT_TV002:
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

	case TOOLACCESS_SIP_1_DEBUG:
	{
#ifdef VIP_SUPPORT_SIP_1_SPI_2
		if (start_addr*sizeof(int)+size > (sizeof(int))*2)
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			int var[2];
			extern int SIP1_init_status;
			var[0] = rtice_Get_SIP_1_Enable();
			var[1] = SIP1_init_status;
			memcpy(buf, var+start_addr, size);
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
		}

		*num_type = VIP_INT32;
#endif
	}
	break;

#ifdef CONFIG_CUSTOMER_TV002
	case TOOLACCESS_WBBL_enable_TV002:
	{
		extern ST_WBBL WBBL_table;
		if (size > (sizeof(unsigned char)))
			return -ID_SIZE_ERR;
		//*buf = WBBL_table.WBBL_enable;
		//memcpy(&(HELLO), buf, sizeof(unsigned char));
		memcpy(buf, &(WBBL_table.WBBL_enable),sizeof(unsigned char));
		//memcpy(&(WBBL_table.WBBL_enable), buf, sizeof(unsigned char));
		*num_type = VIP_UINT8;
	}
	break;
	case TOOLACCESS_WBBL_table_TV002:
	{
		extern ST_WBBL WBBL_table;
		extern ST_WB_BS Blue_Stretch;

		if (size > ((WBBL_POSITION_MAX+WBBL_COLOR_MAX*WBBL_POSITION_MAX)*sizeof(unsigned short) + sizeof(ST_WB_BS)))
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(((WBBL_POSITION_MAX+WBBL_COLOR_MAX*WBBL_POSITION_MAX)*sizeof(unsigned short) + sizeof(ST_WB_BS)));

		if (pwUINT16Temp == NULL)
			return -ID_SIZE_ERR;

		*pwUINT16Temp = WBBL_table.SBC_BL[0];
		*(pwUINT16Temp+1) = WBBL_table.SBC_BL[1];
		*(pwUINT16Temp+2) = WBBL_table.SBC_BL[2];
		*(pwUINT16Temp+3) = WBBL_table.WBBL_Gain[0][0];
		*(pwUINT16Temp+4) = WBBL_table.WBBL_Gain[0][1];
		*(pwUINT16Temp+5) = WBBL_table.WBBL_Gain[0][2];
		*(pwUINT16Temp+6) = WBBL_table.WBBL_Gain[1][0];
		*(pwUINT16Temp+7) = WBBL_table.WBBL_Gain[1][1];
		*(pwUINT16Temp+8) = WBBL_table.WBBL_Gain[1][2];
		*(pwUINT16Temp+9) = WBBL_table.WBBL_Gain[2][0];
		*(pwUINT16Temp+10) = WBBL_table.WBBL_Gain[2][1];
		*(pwUINT16Temp+11) = WBBL_table.WBBL_Gain[2][2];
		*(pwUINT16Temp+12) = Blue_Stretch.WB_A_Gain[0];
		*(pwUINT16Temp+13) = Blue_Stretch.WB_A_Gain[1];
		*(pwUINT16Temp+14) = Blue_Stretch.WB_A_Gain[2];
		*(pwUINT16Temp+15) = Blue_Stretch.WB_A_Offset[0];
		*(pwUINT16Temp+16) = Blue_Stretch.WB_A_Offset[1];
		*(pwUINT16Temp+17) = Blue_Stretch.WB_A_Offset[2];

		fwif_color_ChangeUINT16Endian(pwUINT16Temp, size/sizeof(unsigned short), 1);

		memcpy(buf, pwUINT16Temp, size);
		vip_free(pwUINT16Temp);
		pwUINT16Temp = NULL;
		*num_type = VIP_UINT16;


	}
	break;

	case TOOLACCESS_Black_Adjust_TV002:
	{
		extern VIP_DCC_Black_Adjust_Ctrl g_black_adjust_tv002;

		if (size > sizeof(g_black_adjust_tv002))
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(sizeof(g_black_adjust_tv002));
		if (pwUINT16Temp != NULL) {
			memcpy(pwUINT16Temp, &g_black_adjust_tv002, sizeof(g_black_adjust_tv002));
			fwif_color_ChangeUINT16Endian(pwUINT16Temp, sizeof(g_black_adjust_tv002)/sizeof(unsigned short), 1);
			memcpy(buf, pwUINT16Temp, size);
			vip_free(pwUINT16Temp);
		} else
			return -ID_SIZE_ERR;

		*(num_type) = VIP_UINT16;
	}
	break;

	case TOOLACCESS_PQ_Extend_Enable_TV002:
	{
		extern unsigned char g_bEnable_PQ_extend_data;

		if (size > sizeof(g_bEnable_PQ_extend_data))
			return -ID_SIZE_ERR;

		memcpy(buf, &g_bEnable_PQ_extend_data, size);

		*(num_type) = VIP_UINT8;
	}
	break;

	case TOOLACCESS_PQ_Extend_Index_TV002:
	{
		extern unsigned char save_PQ_Extend_Data_Index[PQ_EXTEND_DATA_ENUM_MAX_NUM];

		if (size > sizeof(save_PQ_Extend_Data_Index))
			return -ID_SIZE_ERR;

		memcpy(buf, save_PQ_Extend_Data_Index, size);

		*(num_type) = VIP_UINT8;
	}
	break;

	case TOOLACCESS_PQ_DBC_Item_Check_TV002:
	{
		BL_DBC_ITEM_CHECK_T DBC_Item_Check = {0};
		extern BL_DBC_ALL_PARAM_T g_dbc_param;
		extern unsigned short g_LDPixelGainSave;

		if (start_addr*sizeof(unsigned short)+size > sizeof(DBC_Item_Check))
			return -ID_SIZE_ERR;

		if (size) {
			DBC_Item_Check.DBC_APL_CTL = g_dbc_param.dbc_lib.DBC_APL_CTL;
			DBC_Item_Check.DBC_MAX_CTL = g_dbc_param.dbc_lib.DBC_MAX_CTL;
			DBC_Item_Check.DBC_BK_CTL = g_dbc_param.dbc_lib.DBC_BK_CTL;
			DBC_Item_Check.DBC_BK2_CTL = g_dbc_param.dbc_lib.DBC_BK2_CTL;
			DBC_Item_Check.DBC_GFX_GAIN = g_dbc_param.dbc_lib.DBC_GFX_GAIN;
			DBC_Item_Check.DBC4PS_BL_CONT = g_dbc_param.dbc_lib.DBC4PS_BL_CONT;
			DBC_Item_Check.BL_GAIN = g_dbc_param.dbc_blend_out.BL_GAIN;
			DBC_Item_Check.HEAT_BLSAVE = g_dbc_param.ld_lib.HEAT_BLSAVE;
			DBC_Item_Check.LD_PIXEL_GAIN = g_LDPixelGainSave;

			memcpy(buf, ((unsigned short *)&DBC_Item_Check)+start_addr, size);
			fwif_color_ChangeUINT16Endian((unsigned short *)buf, size/sizeof(unsigned short), 1);
		}

		*(num_type) = VIP_UINT16;
	}
	break;

	case TOOLACCESS_PQ_SC_Item_Check_TV002:
	{
		unsigned int SC_ITEM[3] = {0};

		if (start_addr*sizeof(unsigned int)+size > sizeof(SC_ITEM))
			return -ID_SIZE_ERR;

		if (size) {
			SC_ITEM[0] = scalerDCC_get_Adapt_Mean_info();
			SC_ITEM[1] = scalerDCC_get_Adapt_Dev_info();
			SC_ITEM[2] = scalerVIP_get_motion_info();

			memcpy(buf, &SC_ITEM[start_addr], size);
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);
		}

		*(num_type) = VIP_UINT32;
	}
	break;

	case TOOLACCESS_PQ_DCON_DBRI_TV002:
	{
		extern short g_TV002_Con_Offset;
		extern short g_TV002_Bri_Offset;

		if (start_addr*sizeof(short)+size > sizeof(short)*2)
			return -ID_SIZE_ERR;

		if (size >= sizeof(short)) {
			short tmp[2];
			tmp[0] = g_TV002_Con_Offset;
			tmp[1] = g_TV002_Bri_Offset;
			memcpy(buf, tmp+start_addr, size);
			fwif_color_ChangeINT16Endian((short *)buf, size/sizeof(short), 1);
		}

		*(num_type) = VIP_INT16;
	}
	break;

	case TOOLACCESS_PQ_DYNAMIC_SHP_TV002:
	{
		extern bool g_TV002_Dynamic_Shp_Gain_En;
		extern bool g_TV002_Dynamic_Shp_Gain_By_Y_En;
		extern bool g_TV002_Dynamic_SNR_Weight_En;
		extern int g_TV002_Dynamic_Shp_Gain;
		extern int g_TV002_Dynamic_Shp_Gain_By_Y;
		extern int g_TV002_Dynamic_SNR_Weight;

		if (start_addr*sizeof(int)+size > sizeof(int)*6)
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			int tmp[6];
			tmp[0] = g_TV002_Dynamic_Shp_Gain_En;
			tmp[1] = g_TV002_Dynamic_Shp_Gain_By_Y_En;
			tmp[2] = g_TV002_Dynamic_SNR_Weight_En;
			tmp[3] = g_TV002_Dynamic_Shp_Gain;
			tmp[4] = g_TV002_Dynamic_Shp_Gain_By_Y;
			tmp[5] = g_TV002_Dynamic_SNR_Weight;
			memcpy(buf, tmp+start_addr, size);
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
		}

		*(num_type) = VIP_INT32;
	}
	break;

	case TOOLACCESS_PQ_ZERO_DIMMING_IIR_CTRL_TV002:
	{
		extern int ZD_APL_Diff_th1 ,ZD_APL_Diff_th2, ZD_IIR_Speed_low, ZD_IIR_Speed_high, ZD_RGB_Max_Limit_en, ZD_RGB_Max_Limit_th;
		extern int ZD_IIR_down_shift, ZD_IIR_up_shift;
		if (start_addr*sizeof(int)+size > sizeof(int)*8)
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			int tmp[8];
			tmp[0] = ZD_APL_Diff_th1; tmp[1] = ZD_APL_Diff_th2; tmp[2] = ZD_IIR_Speed_low; tmp[3] = ZD_IIR_Speed_high; tmp[4] = ZD_RGB_Max_Limit_en; tmp[5] = ZD_RGB_Max_Limit_th;
			tmp[6] = ZD_IIR_down_shift; tmp[7] = ZD_IIR_up_shift;
			memcpy(buf, tmp+start_addr, size);
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
		}
		*(num_type) = VIP_INT32;
	}
	break;

	case TOOLACCESS_OSD_Alpha_Detect_TV002:
	{
		int alpha;
		extern int GDMA_GetAlphaOsdDetectionParameter(int *data);

		if (size > sizeof(alpha))
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			GDMA_GetAlphaOsdDetectionParameter(&alpha);
			memcpy(buf, &alpha, size);
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
		}

		*(num_type) = VIP_INT32;
	}
	break;

	case TOOLACCESS_OSD_Alpha_Ratio_TV002:
	{
		int ratio;
		extern int GDMA_GetAlphaOsdDetectionratio(int *ratio);

		if (size > sizeof(ratio))
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			if (GDMA_GetAlphaOsdDetectionratio(&ratio) != 0)
				ratio = 0xdead;
			memcpy(buf, &ratio, size);
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
		}

		*(num_type) = VIP_INT32;
	}
	break;

	case TOOLACCESS_PQ_DSLC_CTRL_TV002:
	{
		extern DYNAMIC_SLC_CTRL_ST DSLC_TBL[SLC_Table_NUM];
		DYNAMIC_SLC_CTRL_RTICE_ST *pRtice = NULL;
		DYNAMIC_SLC_CTRL_ST *pDrv = NULL;

		if (start_addr*sizeof(short)+size > sizeof(DYNAMIC_SLC_CTRL_RTICE_ST))
			return -ID_SIZE_ERR;

		if (size >= sizeof(short)) {
			pRtice = (DYNAMIC_SLC_CTRL_RTICE_ST *)vip_malloc(sizeof(DYNAMIC_SLC_CTRL_RTICE_ST));
			pDrv = &DSLC_TBL[0];

			if (pRtice != NULL) {
				drvif_convert_DSLC_st(0, pDrv, pRtice);
				memcpy(buf, (short *)pRtice+start_addr, size);
				fwif_color_ChangeINT16Endian((short *)buf, size/sizeof(short), 1);
				vip_free(pRtice);
			}
		}

		*(num_type) = VIP_INT16;
	}
	break;

	case TOOLACCESS_PQ_FREQ_DET_TV002:
	{
		if (start_addr*sizeof(int)+size > sizeof(g_Freq_Det_TV002_Save))
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			memcpy(buf, (int *)&g_Freq_Det_TV002_Save+start_addr, size);
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
		}

		*(num_type) = VIP_INT32;
	}
	break;

	case TOOLACCESS_PQ_VGIP_ISR_STATUS_TV002:
	{
		extern unsigned char g_status_VGIP_ISR;
		
		if (size > sizeof(g_status_VGIP_ISR))
			return -ID_SIZE_ERR;
		
		memcpy(buf, &g_status_VGIP_ISR, size);
		
		*(num_type) = VIP_UINT8;
	}
	break;

	case TOOLACCESS_PQ_SRS_SHP_OUT:
	{
		unsigned char level = Scaler_Get_Freq_Det_Out_TV002();
		
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		
		memcpy(buf, &level, size);
		
		*(num_type) = VIP_UINT8;
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_UINT8:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -READ_ERROR;

		if (start_addr+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(unsigned char)) {
			memcpy(buf, &p[start_addr], size);
		}

		*(num_type) = VIP_UINT8;
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_UINT16:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -READ_ERROR;

		if (start_addr+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(unsigned short)) {
			memcpy(buf, &p[start_addr], size);
			fwif_color_ChangeUINT16Endian((unsigned short *)buf, size/sizeof(unsigned short), 1);
		}

		*(num_type) = VIP_UINT16;
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_UINT32:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -READ_ERROR;

		if (start_addr+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(unsigned int)) {
			memcpy(buf, &p[start_addr], size);
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);
		}

		*(num_type) = VIP_UINT32;
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_SINT8:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -READ_ERROR;

		if (start_addr+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(unsigned char)) {
			memcpy(buf, &p[start_addr], size);
		}

		*(num_type) = VIP_INT8;
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_SINT16:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -READ_ERROR;

		if (start_addr+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(short)) {
			memcpy(buf, &p[start_addr], size);
			fwif_color_ChangeINT16Endian((short *)buf, size/sizeof(short), 1);
		}

		*(num_type) = VIP_INT16;
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_SINT32:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -READ_ERROR;

		if (start_addr+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			memcpy(buf, &p[start_addr], size);
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
		}

		*(num_type) = VIP_INT32;
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_UINT16_ALIGN:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -READ_ERROR;

		if (start_addr*2+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(unsigned short)) {
			memcpy(buf, &p[start_addr*2], size);
			fwif_color_ChangeUINT16Endian((unsigned short *)buf, size/sizeof(unsigned short), 1);
		}

		*(num_type) = VIP_UINT16;
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_UINT32_ALIGN:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -READ_ERROR;

		if (start_addr*4+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(unsigned int)) {
			memcpy(buf, &p[start_addr*4], size);
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);
		}

		*(num_type) = VIP_UINT32;
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_SINT16_ALIGN:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -READ_ERROR;

		if (start_addr*2+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(short)) {
			memcpy(buf, &p[start_addr*2], size);
			fwif_color_ChangeINT16Endian((short *)buf, size/sizeof(short), 1);
		}

		*(num_type) = VIP_INT16;
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_SINT32_ALIGN:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -READ_ERROR;

		if (start_addr*4+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			memcpy(buf, &p[start_addr*4], size);
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
		}

		*(num_type) = VIP_INT32;
	}
	break;
#endif

	case TOOLACCESS_Function_MEMC_Performance_Checking_Database:
	{
		//unsigned char pqCopyChar = 0;
		if(size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		*buf = Scaler_MEMC_Get_Performance_Checking_Database_index();
		//memcpy(	&buf, (unsigned char*)pqCopyChar, 1);
		//fwif_color_ChangeUINT32Endian((unsigned int*)&buf, size/sizeof(int), 1);
		*num_type = VIP_UINT8;
		
		break;
	}

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

	case TOOLACCESS_RADCR_BL_MAP_TBL:
	{
		extern RADCR_BL_MAP_ST RADCR_BL_Mapping_table;
		if (start_addr*sizeof(unsigned char)+size > sizeof(RADCR_BL_MAP_ST))
			return -ID_SIZE_ERR;

		if (size) {
			memcpy(buf, RADCR_BL_Mapping_table.bl_mapping+start_addr, size);
		}
		
		*num_type = VIP_UINT8;
		break;
	}

	default:
		return -ID_SIZE_ERR;
		break;
	}
	return size;
}
//extern unsigned int LG_LUT_3D_EXTEND[LUT3D_EXTEND_TBL_Num][LUT3D_TBL_ITEM];
extern unsigned int D_3D_DRV_LUT[VIP_D_3D_LUT_DRV_TBL_SIZE];
//extern unsigned int LUT_3D[LUT3D_TBL_Num][LUT3D_TBL_ITEM];

int rtice_set_vip_table(ap_data_param rtice_param, unsigned char *buf)
{
	int ret;
	unsigned int i;
	/*int i = 0, idx;*/
	int *pwINT32Temp = NULL;
	unsigned int *pwUINT32Temp = NULL;
	unsigned short *pwUINT16Temp = NULL;
	short *pwINT16Temp = NULL;
	unsigned char *pUINT8Temp = NULL;
	unsigned int pwCopyTemp=0;
	RPC_DCC_Advance_control_table *Advance_control_table_by_source = NULL;
	/*int *pwINT32Temp = NULL;*/
	/*RPC_DCC_Advance_control_table *Advance_control_table_by_source = NULL;*/
	/*unsigned int pwCopyTemp = 0;*/
	/*int dhue = 0;*/
	/*int dsatbysat[SATSEGMAX];*/
	/*int ditnbyitn[ITNSEGMAX];*/
	/*int h_sta,  h_end,  s_sta,  s_end,  i_sta,  i_end;*/
	/*unsigned int *pWriteBuf = NULL;*/
	_clues *smartPic_clue = fwif_color_Get_SmartPic_clue();
	_RPC_clues *RPC_smartPic_clue = (_RPC_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);
	unsigned char bTemp = 0;
	/*unsigned char *buf2_tmp = NULL;*/
	int *pwIntTemp = NULL;
	/*unsigned int *buf_tmp = NULL;*/
	//unsigned char *pUcTemp = NULL;
	unsigned int * pUiTemp = NULL;
	unsigned int size, start_addr,temp_size;
	_system_setting_info *system_setting_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *RPC_system_setting_info = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	unsigned char HY,VY,HC,VC;

	SLR_VIP_TABLE *vipTable_ShareMem = fwif_color_GetShare_Memory_VIP_TABLE_Struct();
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	VIP_DCC_AdaptCtrl_Level_Table *DCC_AdaptCtrl_Level_Table = NULL;
	DRV_TC_HDR_CTRL *pTC_HDR_CTRL=NULL;
	VIP_ST2094_CTRL *ST2094_CTRL = NULL;
	int idx = fwif_color_3d_lut_access_index(ACCESS_MODE_GET, 0);

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	unsigned int ofst = 0;
	unsigned int u32tmpValue = 0;
	PQMASK_COLOR_MGR_T *pPQMaskColorMgr;
	PQMASK_GLB_CTRL_T *pPQMaskGlbCtrl;
	PQMASK_ONLINE_MONITOR_CTRL_T *pPQMaskMonitorCtrl;
#endif

	if( (vipTable_ShareMem == NULL) || (gVip_Table == NULL) || (system_setting_info == NULL) || (smartPic_clue ==NULL)||(RPC_smartPic_clue==NULL) ){
		return -ID_SIZE_ERR;
	}

	stopUpdateFrequencyOffset = 180;

	size = rtice_param.size;
	start_addr = rtice_param.saddr;

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

	case TOOLACCESS_ScaleUPH_8tap:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetScaleUPH_8tap(buf[0]);
		break;

	case TOOLACCESS_ScaleUPV_6tap:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetScaleUPV_6tap(buf[0]);
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

	/*case TOOLACCESS_VD_SetVflag_Remg_Thr:*/
	//	if (size > sizeof(unsigned char))
	//		return -ID_SIZE_ERR;
	//	drvif_module_vdc_H2V_Vflag_Remg_thr_Factory_Mode(buf[0]);
	//	break;

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

	case TOOLACCESS_VD_PQ_TableControl:
		if (size > PRJ_TBL_ITEM_MAX*sizeof(unsigned char))
			return -ID_SIZE_ERR;
		memcpy((unsigned char *)VPQ_PRJ_TABLE_SEL[drvif_module_vpq_get_ProjectId()], buf, size);
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

	case TOOLACCESS_VIP_QUALITY_Coef:       /*  20*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Coef_MAX)
			return -ID_SIZE_ERR;
		memcpy(vipTable_ShareMem->VIP_QUALITY_Coef, buf, size);
		break;
	case TOOLACCESS_VIP_QUALITY_Extend_Coef:       /*  21*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend_Coef_MAX)
			return -ID_SIZE_ERR;
		memcpy(vipTable_ShareMem->VIP_QUALITY_Extend_Coef, buf, size);
		break;
	case TOOLACCESS_VIP_QUALITY_Extend2_Coef:       /*  22*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend2_Coef_MAX)
			return -ID_SIZE_ERR;
		memcpy(vipTable_ShareMem->VIP_QUALITY_Extend2_Coef, buf, size);
		break;
	case TOOLACCESS_VIP_QUALITY_Extend3_Coef:       /*  23*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend3_Coef_MAX)
			return -ID_SIZE_ERR;
		memcpy(vipTable_ShareMem->VIP_QUALITY_Extend3_Coef, buf, size);
		break;
	case TOOLACCESS_tICM_ini:       /*  24*/
		if (size > sizeof(unsigned short)*VIP_ICM_TBL_X*VIP_ICM_TBL_Y*VIP_ICM_TBL_Z)
			return -ID_SIZE_ERR;
		memcpy((unsigned char *)&(vipTable_ShareMem->tICM_ini[0][0][0]), buf, size);
		break;
	case TOOLACCESS_SHPTable:       /*  25*/
		if (size > sizeof(VIP_Sharpness_Table))
			return -ID_SIZE_ERR;
		memcpy((unsigned char *) &(vipTable_ShareMem->Ddomain_SHPTable[0]), buf, size);
		break;
	case TOOLACCESS_Hist_Y_Mean_Value:/* 26*/
		if (size > sizeof(unsigned int)*1)
			return -ID_SIZE_ERR;
		break;
	case TOOLACCESS_TABLE_NTSC_3D:  /*  27*/
		if (size > sizeof(unsigned char)*VD_3D_TABLE_SIZE*VD_MOTION_STEP*2)
			return -ID_SIZE_ERR;
		memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_TABLE_NTSC_3D), buf, size);
	ret = Scaler_SendRPC(SCALERIOC_SET_TABLE_NTSC_3D, 0, 0);
	if (0 != ret) {
			//SLRLIB_PRINTF("ret = %d, Get SCALERIOC_SET_TABLE_NTSC_3D fail !!!\n", ret);
			return -1;
		}
		break;
	case TOOLACCESS_MANUAL_NR:/* 29*/
		if (size > sizeof(DRV_NR_Item)*PQA_TABLE_MAX*DRV_NR_Level_MAX)
			return -ID_SIZE_ERR;
		memcpy((unsigned char *)&(vipTable_ShareMem->Manual_NR_Table[0][0]), buf, size);

		break;

	case TOOLACCESS_ISR_Print_Ctrl:
		if (size > sizeof(VIP_ISR_Printf_Flag))
			return -ID_SIZE_ERR;
		fwif_color_ChangeUINT32Endian((unsigned int *)buf, ISR_PRINT_CTRL_ITEM_NUMBER, 1);
		memcpy((unsigned char *) &(system_setting_info->ISR_Printf_Flag), buf, size);
		break;

		case TOOLACCESS_Debug_Buff_8:
			if(size > Debug_Buff_Num*sizeof(char))
				return -ID_SIZE_ERR;
			memcpy((unsigned char*)&(system_setting_info->Debug_Buff_8[0]),buf, size);

			break;

		case TOOLACCESS_Debug_Buff_16:
			if(size > Debug_Buff_Num*sizeof(short))
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);

			memcpy((unsigned char*)&(system_setting_info->Debug_Buff_16[0]),buf, size);

			break;

		case TOOLACCESS_Debug_Buff_32:
			if(size > Debug_Buff_Num*sizeof(int))
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(int), 1);

			memcpy((unsigned char*)&(system_setting_info->Debug_Buff_32[0]),buf, size);

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
		if (size > sizeof(VIP_DCC_Curve_Control_Coef))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Table;

		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Curve_Control_Coef[bTemp]), buf, size);
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Boundary_check_Table:   		/* 1101*/
		if (size > sizeof(VIP_DCC_Boundary_check_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.Boundary_Check_Table;

		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Boundary_check_Table[bTemp]), buf, size);
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Level_and_Blend_Coef_Table:     /* 1102 , DCC_Misc*/
		if (size > sizeof(VIP_DCC_Level_and_Blend_Coef_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.Level_and_Blend_Coef_Table;

		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table[bTemp]), buf, size);
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Hist_Factor_Table:   			/* 1103*/
		if (size > sizeof(VIP_DCC_Hist_Factor_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.hist_adjust_table;

		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCCHist_Factor_Table[bTemp]), buf , size);
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_AdaptCtrl_Level_Table:  		/* 1104*/
		if (size > sizeof(VIP_DCC_AdaptCtrl_Level_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.AdaptCtrl_Level_Table;

		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_AdaptCtrl_Level_Table[bTemp]), buf , size);
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
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisMean_th), buf, 10);
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisSeg_Weighting), buf+sizeof(USER_CURVE_DCC_HisMean_th_ITEM) , 11);
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_UserDef_Curve_All_TABLE: /*1107*/
		if (size > (sizeof(unsigned int)*(USER_DEFINE_CURVE_DCC_CURVE_MAX*USER_DEFINE_CURVE_DCC_SEGMENT_NUM)))
			return -ID_SIZE_ERR;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.User_Curve_Table;
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].user_define_curve_dcc_table) , (unsigned char *)buf, size);

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
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].user_define_curve_dcc_table[rtice_param.mode0][0]), (unsigned char*)buf, size);
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
		memcpy((unsigned char*)&(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][0]), (unsigned char*)buf, size);
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

		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_curve), (unsigned char*)buf, size);
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

		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_histogram), (unsigned char *)buf, size);
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Histogram_Curve:  /*id:1112*/
		if (size > (sizeof(unsigned int)*DCC_Curve_Node_MAX))
			return -ID_SIZE_ERR;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);
		memcpy((unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Histogram_Curve), (unsigned char *)buf, size);
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;


	case TOOLACCESS_DCC_Histogram_Adjust:  /*id:1114*/
		if (size > sizeof(unsigned int)*COLOR_HISTOGRAM_LEVEL)
			return -ID_SIZE_ERR;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);
		memcpy((unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.HistCnt_Of_Adj), (unsigned char *)buf, size);
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;


	case TOOLACCESS_DCC_Apply_Curve: /*1116*/
		if (size > (sizeof(unsigned int)*DCC_Curve_Node_MAX))
			return -ID_SIZE_ERR;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);

		memcpy((unsigned char *) &(RPC_smartPic_clue->RPC_Adapt_DCC_Info.Apply_Curve), (unsigned char *)buf, size);


		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Curve_Boundary_Type:			/* 1117*/
			if(size > sizeof(unsigned char))
				return -ID_SIZE_ERR;
			pwCopyTemp = smartPic_clue->Remmping_Src_Timing_index;
			memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_init_boundary_Type),buf,size);
			fwif_color_set_DCC_SC_for_Coef_Change(1,1);

			break;

	case TOOLACCESS_DCC_Curve_Boundary:				/* 1118*/
			if(size > sizeof( RPC_DCC_Curve_boundary_table))
				return -ID_SIZE_ERR;
			pwCopyTemp = smartPic_clue->Remmping_Src_Timing_index;
			Advance_control_table_by_source = &(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp]);
			bTemp = Advance_control_table_by_source->DCC_init_boundary_Type;

			if(bTemp < Curve_boundary_table_MAX)
				memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Curve_boundary_table[bTemp]),buf,size);
			fwif_color_set_DCC_SC_for_Coef_Change(1,1);

			break;
	case TOOLACCESS_DCC_panel_compensation_Curve: /*1119*/
		if(size > sizeof(unsigned int)*Curve_num_Max)
			return -ID_SIZE_ERR;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);
		memcpy((unsigned char*)&(vipTable_ShareMem->panel_compensation_curve),buf,size);
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

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);

		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_adjust), (unsigned char*)buf, size);
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_VIP_DCC_Advance_Level_Control_Flag: 			/* 1122*/
		if (size > (Advance_Level_Control_Item_MAX+LPF_control_Item_MAX+1))
			return -ID_SIZE_ERR;
		pwCopyTemp = smartPic_clue->Remmping_Src_Timing_index;
		if(pwCopyTemp >= Advance_control_table_MAX)
			pwCopyTemp = Advance_control_table_MAX-1;

		memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Advance_Level_Control_Flag),(unsigned char*)buf,Advance_Level_Control_Item_MAX);
		memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_LPF_Control),(unsigned char*)buf+Advance_Level_Control_Item_MAX,LPF_control_Item_MAX);
		memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_init_boundary_Type),(unsigned char*)buf+Advance_Level_Control_Item_MAX+LPF_control_Item_MAX,1);

		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);
		break;


	case TOOLACCESS_VIP_DCC_Advance_Level_Control_Step_DT_Table: 			/* 1123*/
		if (size > (mean_value_index_MAX*Variance_index_MAX*2))
			return -ID_SIZE_ERR;
		pwCopyTemp = smartPic_clue->Remmping_Src_Timing_index;
		if(pwCopyTemp >= Advance_control_table_MAX)
			pwCopyTemp = Advance_control_table_MAX-1;

		memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Curve_Blending_Step_table),(unsigned char*)buf,mean_value_index_MAX*Variance_index_MAX);
		memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Curve_Blending_DT_table),(unsigned char*)buf+mean_value_index_MAX*Variance_index_MAX,mean_value_index_MAX*Variance_index_MAX);

		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);
		break;




	case TOOLACCESS_ICM_Global_Adjust:					/* 1200*/
		if (size > sizeof(RPC_ICM_Global_Ctrl))
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian((int *)buf, size/4, 1);
		memcpy((unsigned char *)fwif_color_GetICM_Global_Ctrl_Struct(), buf, size);

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
		//pwUINT32Temp = (unsigned int *)vip_malloc(80000 * sizeof(unsigned char));
		pwUINT32Temp = (unsigned int *)dvr_malloc_specific(80000 * sizeof(unsigned char), GFP_DCU2_FIRST);
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

		//vip_free(pwUINT32Temp);
		dvr_free(pwUINT32Temp);
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

	case TOOLACCESS_PCID_RgnTbl: /*1252 set*/
				
		if (size > sizeof(unsigned int)*PCID_TBL_LEN)
			return -ID_SIZE_ERR;

		pUiTemp = (unsigned int *)vip_malloc(sizeof(unsigned int)*PCID_TBL_LEN);

		if(pUiTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian((unsigned int *)buf, size>>2, 1);
		memcpy((unsigned char *)pUiTemp, (unsigned char *)buf, size);

		fwif_color_set_pcid_RgnTableValue(pUiTemp, rtice_param.mode0, rtice_param.mode1);

		vip_free(pUiTemp);

		break;

	case TOOLACCESS_PCID_RgnWtTbl:/*1253 set*/
		if (size > sizeof(unsigned int)*PCID_WTTBL_LEN)
			return -ID_SIZE_ERR;

		pUiTemp = (unsigned int *)vip_malloc(sizeof(unsigned int)*PCID_WTTBL_LEN);

		if(pUiTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian((unsigned int *)buf, size>>2, 1);
		memcpy((unsigned char *)pUiTemp, (unsigned char *)buf, size);

		fwif_color_set_pcid_RgnWtTableValue(pUiTemp);

		vip_free(pUiTemp);
		break;

	case TOOLACCESS_VALC_Tbl: /*1254 set*/

		if (size > sizeof(unsigned int)*VALC_TBL_LEN)
			return -ID_SIZE_ERR;

		pUiTemp = (unsigned int *)vip_malloc(sizeof(unsigned int)*VALC_TBL_LEN);

		if(pUiTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian((unsigned int *)buf, size>>2, 1);
		memcpy((unsigned char *)pUiTemp, (unsigned char *)buf, size);

		fwif_color_set_valc_table(pUiTemp, rtice_param.mode0, rtice_param.mode1);

		vip_free(pUiTemp);

		break;

	case TOOLACCESS_MEMC_CHANGE_SIZE: /*1256 set*/

		if(size > sizeof(MEMC_SIZE_PARAM))
			return -ID_SIZE_ERR;

		pUiTemp = (unsigned int *)vip_malloc(sizeof(MEMC_SIZE_PARAM)/sizeof(unsigned int));

		if(pUiTemp == NULL)
			return -ID_SIZE_ERR;

		memcpy((unsigned int*)pUiTemp, buf, size);

//merlin7_bring_up			MEMC_RTICE_SetParam((MEMC_SIZE_PARAM*)pUiTemp);
//merlin7_bring_up			MEMC_RTICE_Trigger();

		memcpy(buf, (unsigned int*)pUiTemp, size);

		vip_free(pUiTemp);

		break;
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
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

		#if 0 // test
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		
		{
		extern void set_dtv_securestatus_aipq(unsigned char status);
		set_dtv_securestatus_aipq(buf[0]);
		
		}
		#endif

		break;

// lesley 1024
	case TOOLACCESS_AI_Ctrl_Draw: /*1261 set*/
		// 0622 lsy
		//if (size > sizeof(unsigned char))
		//	return -ID_SIZE_ERR;
		//vpq_stereo_face = buf[0];
		//scalerAI_pq_mode_ctrl(vpq_stereo_face, 0);

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

		if(buf[0]==0){
			aipq_mode.clock_status=0;
			scalerAI_enableClock(0);
		}else{
			scalerAI_enableClock(1);
			aipq_mode.clock_status=1;
		}
		
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
	case TOOLACCESS_AI_DB: /*1266 set*/
		if (size > sizeof(DB_AI_RTK))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DB_AI_RTK));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian(( int*)buf,size>>2, 1);

		memcpy((unsigned char *)pwIntTemp, (unsigned char *)buf, size);

        drivef_tool_ai_db_set((DB_AI_RTK *)pwIntTemp);

		vip_free(pwIntTemp);

		pwIntTemp = NULL;

		break;

	case TOOLACCESS_AI_SCENE_Ctrl: /*1270 get*/
		#if 1 // 0506 lsy

		if (size > sizeof(DRV_AI_PQ_Ctrl_table))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DRV_AI_PQ_Ctrl_table));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian(( int*)buf,size>>2, 1);

		memcpy((unsigned char *)pwIntTemp, (unsigned char *)buf, size);

		scaler_AI_PQ_Ctrl_Set((DRV_AI_PQ_Ctrl_table *)pwIntTemp);

		#else
		if (size > sizeof(DRV_AI_SCENE_Ctrl_table))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DRV_AI_SCENE_Ctrl_table));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian(( int*)buf,size>>2, 1);

		memcpy((unsigned char *)pwIntTemp, (unsigned char *)buf, size);

		scaler_AI_Scene_Ctrl_Set((DRV_AI_SCENE_Ctrl_table *)pwIntTemp);
		#endif

		vip_free(pwIntTemp);

		pwIntTemp = NULL;

		break;

	case TOOLACCESS_AI_SCENE_Ctrl_test:   	/*  1271*/

		// lesley test
		force_scene = buf[0];
		force_sqm = buf[1];//0506 lsy

		break;

	case TOOLACCESS_AI_SCENE_Target: /*1272 set*/
		if (size > PQ_IP_TYPE_NUM*PQ_TUNING_NUM*sizeof(int))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(PQ_IP_TYPE_NUM*PQ_TUNING_NUM*sizeof(int));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian(( int*)buf,size>>2, 1);

		memcpy((unsigned char *)pwIntTemp, (unsigned char *)buf, size);

		scalerAI_scene_PQ_target_set(rtice_param.mode0, pwIntTemp);

		vip_free(pwIntTemp);
		pwIntTemp = NULL;

		break;

	case TOOLACCESS_AI_PQMASK_Control: /*1275 set*/

		if(rtice_param.mode0 == 1) { // set some color mgr setting

			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size>>2, 1);

			pPQMaskColorMgr = (PQMASK_COLOR_MGR_T *)scalerPQMaskColor_GetStruct(PQMASK_COLORSTRUCT_MGR);

			if( pPQMaskColorMgr == NULL )
				return WRITE_ERROR;

			ofst = 0;

			// semantic / depth fusion parameter
			/* 0 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.FusionRatio_Dep = u32tmpValue & 0xff;
			ofst++;
			/* 1 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.FusionRatio_Sem = u32tmpValue & 0xff;
			ofst++;
			// temporal filter parameter
			/* 2 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.FusionRatio_Pre = u32tmpValue & 0xff;
			ofst++;
			/* 3 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.FusionRatio_Cur = u32tmpValue & 0xff;
			ofst++;
			// spatial filter parameter
			/* 4 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.SpatialFltCycle = u32tmpValue & 0xff;
			ofst++;
			/* 5 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.SEHorCoef[0] = u32tmpValue;
			ofst++;
			/* 6 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.SEHorCoef[1] = u32tmpValue;
			ofst++;
			/* 7 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.SEHorCoef[2] = u32tmpValue;
			ofst++;
			/* 8 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.SEHorCoef[3] = u32tmpValue;
			ofst++;
			/* 9 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.SEVerCoef[0] = u32tmpValue;
			ofst++;
			/* 10 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.SEVerCoef[1] = u32tmpValue;
			ofst++;
			/* 11 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.SEVerCoef[2] = u32tmpValue;
			ofst++;
			/* 12 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.SEVerCoef[3] = u32tmpValue;
			ofst++;
			/* 13 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.HorOddType = u32tmpValue & 0xff;
			ofst++;
			/* 14 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.VerOddType = u32tmpValue & 0xff;
			ofst++;
			// scene change
			/* 15 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.bSceneChangeEnable = u32tmpValue;
			ofst++;
			/* 16 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.SceneChangeRstFrm = u32tmpValue;
			ofst++;
			/* 17 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.ScFusionRatio_toRst_Pre = u32tmpValue & 0xff;
			ofst++;
			/* 18 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.ScFusionRatio_toRst_Cur = u32tmpValue & 0xff;
			ofst++;
			/* 19 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.ScFusionRatio_toTarget_Pre = u32tmpValue & 0xff;
			ofst++;
			/* 20 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.ScFusionRatio_toTarget_Cur = u32tmpValue & 0xff;
			ofst++;
			// cg pattern
			/* 21 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.bGenre_CG_Enable = u32tmpValue;
			ofst++;
			/* 22 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.Genre_CG_toTargetFrm = u32tmpValue;
			ofst++;
			/* 23 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.Genre_CG_FunctionRatio_Pre = u32tmpValue & 0xff;
			ofst++;
			/* 24 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Param.Genre_CG_FunctionRatio_Cur = u32tmpValue & 0xff;
			ofst++;
			// image histogram by depth threshold
			/* 25 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->ThlLow = u32tmpValue & 0xff;
			ofst++;
			/* 26 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->ThlHigh = u32tmpValue & 0xff;
			ofst++;

		} else if(rtice_param.mode0 == 2) { // set

			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size>>2, 1);

			pPQMaskColorMgr = (PQMASK_COLOR_MGR_T *)scalerPQMaskColor_GetStruct(PQMASK_COLORSTRUCT_MGR);

			if( pPQMaskColorMgr == NULL )
				return WRITE_ERROR;
			
			ofst = 0;

			// scene change
			/* 0 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Status.bSceneChange = u32tmpValue & 0xff;
			ofst++;
			/* 1 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskColorMgr->Status.SCRstFrm = u32tmpValue & 0xff;
			ofst++;
			/* 2 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskColorMgr->Status.LocalContrastRstWeight = u32tmpValue;
			ofst++;
			// Force current output
			/* 3 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskColorMgr->Status.ForceCurFrm = u32tmpValue & 0xff;
			ofst++;
			// palette control
			/* 4 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskColorMgr->Status.bReloadDepPalette = u32tmpValue & 0xff;
			ofst++;
			/* 5 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskColorMgr->Status.bReloadSemPalette = u32tmpValue & 0xff;
			ofst++;
			// AI Genre palette control
			/* 6 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskColorMgr->Status.Genre_CG_Frm = u32tmpValue & 0xff;
			ofst++;
			/* 7 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskColorMgr->Status.AiGenre = u32tmpValue & 0xff;
			ofst++;
			/* 8 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskColorMgr->Status.bGenerCGStatusChange = u32tmpValue & 0xff;
			ofst++;
			/* 9 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskColorMgr->Status.Genre_CG_Frm = u32tmpValue & 0xff;
			ofst++;
			/* 10 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskColorMgr->Status.bReloadDepthPalette_AiGenre_CG = u32tmpValue & 0xff;
			ofst++;
			/* 11 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskColorMgr->Status.bReloadSemanticPalette_AiGenre_CG = u32tmpValue & 0xff;
			ofst++;

		} else if(rtice_param.mode0 == 3) { // set

			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size>>2, 1);

			pPQMaskGlbCtrl = (PQMASK_GLB_CTRL_T *)scalerPQMask_GetCtrlStruct(PQMASK_STRUCT_GLB_CTRL);

			if(pPQMaskGlbCtrl == NULL)
				return WRITE_ERROR;

			ofst = 0;

			/* 0 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskGlbStatus->PQMaskStatus = u32tmpValue;
			ofst++;
			/* 1 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskGlbCtrl->bEnableSeProc = u32tmpValue & 0xff;
			ofst++;
			/* 2 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskGlbCtrl->LimitFPS = u32tmpValue & 0xff;
			ofst++;
			/* 3 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskGlbCtrl->ErrRecoverRstTimes = u32tmpValue & 0xff;
			ofst++;
			/* 4 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskGlbCtrl->SemCapImgLimitFPS = u32tmpValue & 0xff;
			ofst++;
			/* 5 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskGlbCtrl->bIsDepthReady = u32tmpValue & 0xff;
			ofst++;
			/* 6 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskGlbCtrl->bIsSemReady = u32tmpValue & 0xff;
			ofst++;
			/* 7 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			// pPQMaskGlbStatus->bAllowExec = u32tmpValue & 0xff;
			ofst++;

		} else if(rtice_param.mode0 == 5) {

			// online monitor control
			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size>>2, 1);
			
			pPQMaskMonitorCtrl = (PQMASK_ONLINE_MONITOR_CTRL_T *)scalerPQMask_GetCtrlStruct(PQMASK_STRUCT_MONITOR_CTRL);

			if (pPQMaskMonitorCtrl == NULL)
				return WRITE_ERROR;

			ofst = 0;

			/* 0 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskMonitorCtrl->bEnable = u32tmpValue & 0xff;
			ofst++;
			/* 1 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskMonitorCtrl->bLogEnable = u32tmpValue & 0xff;
			ofst++;
			/* 2 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskMonitorCtrl->LogPeriod = u32tmpValue & 0xff;
			ofst++;
			/* 3 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskMonitorCtrl->Depth_TargetFps = u32tmpValue & 0xff;
			ofst++;
			/* 4 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskMonitorCtrl->Depth_MeasureRstFrm = u32tmpValue & 0xff;
			ofst++;
			/* 5 */
			memcpy(&u32tmpValue, (((unsigned int *)buf)+ofst), sizeof(unsigned int));
			pPQMaskMonitorCtrl->SEProc_TargetTime = u32tmpValue & 0xff;
			ofst++;
			
		} else if(rtice_param.mode0 == 7) { // set pqmask timer

#if 0
			extern struct timer_list PQMaskTimer;

			PQMaskTimer.expires = jiffies + 1*HZ;
			add_timer(&PQMaskTimer);
#endif

		}

		break;

	case TOOLACCESS_AI_PQMASK_SemanticWeight: /*1276 set*/
		if (size > PQMASK_MODULE_NUM*PQMASK_LABEL_NUM*sizeof(unsigned char))
			return -ID_SIZE_ERR;

		pUINT8Temp = (unsigned char *)vip_malloc(PQMASK_MODULE_NUM*PQMASK_LABEL_NUM*sizeof(unsigned char));

		if(pUINT8Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy((unsigned char *)pUINT8Temp, (unsigned char *)buf, size);

		scalerPQMaskColor_SetSemanticWeight(pUINT8Temp);

		vip_free(pUINT8Temp);
		pUINT8Temp = NULL;

		break;

	case TOOLACCESS_AI_PQMASK_DepthWeight: /*1277 set*/		
		if (size > PQMASK_MODULE_NUM*PQMASK_MAPCURVE_LEN*sizeof(unsigned char))
			return -ID_SIZE_ERR;

		pUINT8Temp = (unsigned char *)vip_malloc(PQMASK_MODULE_NUM*PQMASK_MAPCURVE_LEN*sizeof(unsigned char));

		if(pUINT8Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy((unsigned char *)pUINT8Temp, (unsigned char *)buf, size);

		scalerPQMaskColor_SetDepthWeight(pUINT8Temp);

		vip_free(pUINT8Temp);
		pUINT8Temp = NULL;

		break;

	case TOOLACCESS_AI_PQMASK_HistInfo: /*1278 set*/

		if (size > sizeof(PQMASK_HIST_INFO_T))
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(sizeof(PQMASK_HIST_INFO_T));

		if(pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy((void *)pwUINT32Temp, (void *)buf, size);
		fwif_color_ChangeUINT32Endian((unsigned int *)pwUINT32Temp, size/sizeof(unsigned int), 1);

		pPQMaskColorMgr = (PQMASK_COLOR_MGR_T *)scalerPQMaskColor_GetStruct(PQMASK_COLORSTRUCT_MGR);

		if( rtice_param.mode0 == 0 ) {
			// semantic histogram information
			memcpy((void *)&pPQMaskColorMgr->SemanticHist.Info, (void *)pwUINT32Temp, sizeof(PQMASK_HIST_INFO_T));
		} else if( rtice_param.mode0 == 1 ) {
			// depth histogram information
			memcpy((void *)&pPQMaskColorMgr->DepthHist.Info, (void *)pwUINT32Temp, sizeof(PQMASK_HIST_INFO_T));
		} else if( rtice_param.mode0 == 2 ) {
			memcpy((void *)&pPQMaskColorMgr->ImgBySemanticHist[0][0].Info, (void *)pwUINT32Temp, sizeof(PQMASK_HIST_INFO_T));
		} else if( rtice_param.mode0 == 3 ) {
			memcpy((void *)&pPQMaskColorMgr->ImgByDepthHistInfo, (void *)pwUINT32Temp, sizeof(PQMASK_HIST_INFO_T));
		}

#if 0
		rtd_printk(KERN_EMERG,"DEBUG", "pwUINT32Temp[0] = %d\n", pwUINT32Temp[0]);
		rtd_printk(KERN_EMERG,"DEBUG", "pwUINT32Temp[1] = %d\n", pwUINT32Temp[1]);
		rtd_printk(KERN_EMERG,"DEBUG", "pwUINT32Temp[2] = %d\n", pwUINT32Temp[2]);
		rtd_printk(KERN_EMERG,"DEBUG", "pwUINT32Temp[3] = %d\n", pwUINT32Temp[3]);
		rtd_printk(KERN_EMERG,"DEBUG", "pwUINT32Temp[4] = %d\n", pwUINT32Temp[4]);
		if( rtice_param.mode0 == 0 ) {
			rtd_printk(KERN_EMERG,"DEBUG", "sem enable = %d\n", pPQMaskColorMgr->SemanticHist.Info.enable);
			rtd_printk(KERN_EMERG,"DEBUG", "sem width = %d\n", pPQMaskColorMgr->SemanticHist.Info.width);
			rtd_printk(KERN_EMERG,"DEBUG", "sem height = %d\n", pPQMaskColorMgr->SemanticHist.Info.height);
			rtd_printk(KERN_EMERG,"DEBUG", "sem hJmp = %d\n", pPQMaskColorMgr->SemanticHist.Info.hJmp);
			rtd_printk(KERN_EMERG,"DEBUG", "sem vJmp = %d\n", pPQMaskColorMgr->SemanticHist.Info.vJmp);
		} else {
			rtd_printk(KERN_EMERG,"DEBUG", "dep enable = %d\n", pPQMaskColorMgr->SemanticHist.Info.enable);
			rtd_printk(KERN_EMERG,"DEBUG", "dep width = %d\n", pPQMaskColorMgr->SemanticHist.Info.width);
			rtd_printk(KERN_EMERG,"DEBUG", "dep height = %d\n", pPQMaskColorMgr->SemanticHist.Info.height);
			rtd_printk(KERN_EMERG,"DEBUG", "dep hJmp = %d\n", pPQMaskColorMgr->SemanticHist.Info.hJmp);
			rtd_printk(KERN_EMERG,"DEBUG", "dep vJmp = %d\n", pPQMaskColorMgr->SemanticHist.Info.vJmp);
		}
#endif

		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		break;

	case TOOLACCESS_PQMASK_BRINGUP_VERIFY:
	{
		break;
	}

	case TOOLACCESS_AI_PQMASK_SQM_TABLE:
	{
		extern short AI_DepthWeightTbl_SQM_TBL[AI_RESOLUTION_TYPE_NUM][PQMASK_MODULE_NUM];

		if (size+start_addr > sizeof(AI_DepthWeightTbl_SQM_TBL))
			return -ID_SIZE_ERR;

		if (size > 0) {
			fwif_color_ChangeINT16Endian((short*)buf, size/sizeof(short), 1);
			memcpy((unsigned short *)AI_DepthWeightTbl_SQM_TBL+start_addr, buf, size);
			scalerPQMaskColor_SetReloadPalette(PQMASK_COLOR_PALETTE_DEPTH);
		}
	}
	break;
#endif

        case TOOLACCESS_PQ_SR_MODE_Ctrl:

		if (size > sizeof(UINT8))
			return -ID_SIZE_ERR;
                // 0 : SR MODE disable
                // 1 : NNSR MODE
                // 2 : SSRS MODE
                fwif_color_Set_PQ_SR_MODE(buf[0]);
                break;



	case TOOLACCESS_NNSR_Blanding_Idx:
	{
	if (size > sizeof(unsigned char))
		return -ID_SIZE_ERR;

	if (size)
		drvif_srnn_bbbsr_table((DRV_srnn_table *)&gVip_Table->VIP_SRNN_Table[buf[0]]);
	
	break;
	}
	
	case TOOLACCESS_NNSR_Weight_Table_Idx:
	{
		extern unsigned int srnn_model_para[17][1324];
		
		if (size > sizeof(UINT8))
			return -ID_SIZE_ERR;

		//drvif_srnn_model_set((unsigned int *)&srnn_model_para[buf[0]]);
		
		break;
	}

	case TOOLACCESS_NNSR_Weight_Tbl:

		
		if(rtice_param.mode0 ==2)
			nnsr_size = SRNN_Weight_Num_mode2;
		else if(rtice_param.mode0 ==3)
			nnsr_size = SRNN_Weight_Num_mode3;
		else if(rtice_param.mode0 ==4)
			nnsr_size = SRNN_Weight_Num_mode4;
		else if(rtice_param.mode0 ==5)
			nnsr_size = SRNN_Weight_Num_mode5;
		else if(rtice_param.mode0 ==6)
			nnsr_size = SRNN_Weight_Num_mode6;
		else if(rtice_param.mode0 ==7)
			nnsr_size = SRNN_Weight_Num_mode7;	
		else
			return -ID_SIZE_ERR;
		

		if(size>sizeof(unsigned short)*nnsr_size)
			 return -ID_SIZE_ERR;
		
		//drvif_srnn_test_set(1);

		pwUINT16Temp = (unsigned short *)vip_malloc(nnsr_size * sizeof(unsigned short));
		
		if (pwUINT16Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwUINT16Temp, buf, size);
		fwif_color_ChangeUINT16Endian(pwUINT16Temp, size/sizeof(unsigned short), 1);
		fwif_color_srnn_m_disp_en_ctrl(0);

		fwif_color_set_NNSR_model_TBL(rtice_param.mode0, pwUINT16Temp);

		//drvif_srnn_model_set(pwUINT16Temp,rtice_param.mode0);

		fwif_color_srnn_m_disp_en_ctrl(1);

		
		vip_free(pwUINT16Temp);
		pwUINT16Temp = NULL;
		
		break;
		
       case TOOLACCESS_NNSR_test_set:
		if(rtice_param.mode0 ==2)
			nnsr_size = SRNN_Weight_Num_mode2;
		else if(rtice_param.mode0 ==3)
			nnsr_size = SRNN_Weight_Num_mode3;
		else if(rtice_param.mode0 ==4)
			nnsr_size = SRNN_Weight_Num_mode4;
		else if(rtice_param.mode0 ==5)
			nnsr_size = SRNN_Weight_Num_mode5;
		else if(rtice_param.mode0 ==6)
			nnsr_size = SRNN_Weight_Num_mode6;
		else if(rtice_param.mode0 ==7)
			nnsr_size = SRNN_Weight_Num_mode7;	
		else
			return -ID_SIZE_ERR;
		
		if(size>sizeof(unsigned char)*5)
			 return -ID_SIZE_ERR;
		
		pUINT8Temp = (unsigned char *)vip_malloc(5 * sizeof(unsigned char));

		
		if (pUINT8Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pUINT8Temp, buf, size);
		//fwif_color_ChangeUINT8Endian(pUINT8Temp, size/sizeof(unsigned char), 1);
		//fwif_color_Set_NN_SR_Weights_Blend(rtice_param.mode0, 0);
		fwif_color_Set_NN_SR_Weights_Blend_for_rtice(rtice_param.mode0, pUINT8Temp);
	
		vip_free(pUINT8Temp);
		pUINT8Temp = NULL;
        break;

	case TOOLACCESS_NNSR_load_weight_tbl:
		if(rtice_param.mode0 ==2)
			nnsr_size = SRNN_Weight_Num_mode2;
		else if(rtice_param.mode0 ==3)
			nnsr_size = SRNN_Weight_Num_mode3;
		else if(rtice_param.mode0 ==4)
			nnsr_size = SRNN_Weight_Num_mode4;
		else if(rtice_param.mode0 ==5)
			nnsr_size = SRNN_Weight_Num_mode5;
		else if(rtice_param.mode0 ==6)
			nnsr_size = SRNN_Weight_Num_mode6;
		else if(rtice_param.mode0 ==7)
			nnsr_size = SRNN_Weight_Num_mode7;	
		else
			return -ID_SIZE_ERR;
		

		if(size>sizeof(unsigned short)*nnsr_size)
			 return -ID_SIZE_ERR;


		vip_free(pwUINT16Temp);
		pwUINT16Temp = NULL;
		break;
        case TOOLACCESS_siw_PCLRC_Table:
		if (size > 12544*sizeof(UINT8))
			return -ID_SIZE_ERR;

		pUINT8Temp = (unsigned char *)vip_malloc(12544*sizeof(UINT8));



		if(pUINT8Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy((unsigned char *)pUINT8Temp, (unsigned char *)buf, size);
                drvif_color_enable_PCLRC_CLK(TRUE);
                //if panel_pclrc.enable=0  will cause hw write table no used
                drvif_color_set_enable_PCLRC_Funciton(1,_PCLRC_PCLRC_LOD);
                drvif_color_PCLRC_Register_Setting();
                drvif_color_set_siw_PCLRC_valuetable((unsigned char *)pUINT8Temp);
       
		vip_free(pUINT8Temp);
		pwIntTemp = NULL;
                break;


        case TOOLACCESS_BIT_TEST:
/*
                if (size > sizeof(UINT8))
                        return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0){
                        fwif_color_Get_sharpness_table_TV006();
                        fwif_color_bit_test_sharpness_table_TV006(0);
		}
                else if (rtice_param.mode0 == 1){
                        fwif_color_Get_sharpness_table_TV006();
                        fwif_color_bit_test_sharpness_table_TV006(1);
		}
                else if (rtice_param.mode0 == 2){
                        fwif_color_get_PQA_motion_threshold_TV006();
                        fwif_color_bit_test_NR_Table_TV006();
		}
*/
                break;
	case TOOLACCESS_AutoMA_API_CTRL:
	if (size >  sizeof(VIP_AutoMA_API_CTRL))
		return -ID_SIZE_ERR;
	memcpy((unsigned char *) &(system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL), buf, size);

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
/*
	case TOOLACCESS_Function_Coef_Ma_Chroma_Error:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetMA_Chroma_Error(buf[0]); merlin4 remove

		break;
*/
	case TOOLACCESS_Function_Coef_NM_Level:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		//Scaler_SetRTNR_Noise_Measure(buf[0]); //merlin3 not use

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

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_Set_I_De_XC(buf[0]);

		break;
	case TOOLACCESS_Function_Extend_I_De_Contour:

		Scaler_Set_I_De_Contour(buf[0]);

			break;
#if 0/*SLD, hack, elieli*/ //k5l remove

	case TOOLACCESS_Function_Extend_SLD:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_Set_SLD(buf[0]);

			break;
#endif
	case TOOLACCESS_Function_Extend2_SU_H: //4tab

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);

		Scaler_SetScaleUP_YCHV(buf[0],VY,HC,VC);

		break;

	case TOOLACCESS_Function_Extend2_SU_V://4tab

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

	case TOOLACCESS_Function_Extend2_Egsm_PostSHP_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_setdESM(buf[0]);

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

		Scaler_SetScaleUPH_8tap(buf[0]);

		break;
	case TOOLACCESS_Function_Extend2_SU_V_6Tab:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetScaleUPV_6tap(buf[0]);

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
#if 0 //k5l hw remove
	case TOOLACCESS_Function_Extend3_blue_stretch:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_set_blue_stretch(buf[0]);

		break;
#endif
	case TOOLACCESS_Function_Extend3_MB_SU_Peaking:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetMBSUPeaking(buf[0]);

		break;

	case TOOLACCESS_Function_Coef_ALL:

		if (rtice_param.mode0 == 0) {//set total
			if (size > VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Coef_MAX)
				return -ID_SIZE_ERR;

			memcpy((unsigned char *) &(gVip_Table->VIP_QUALITY_Coef[0][0]), buf, size);

		} else if (rtice_param.mode0 == 1) { //set row

			if (size > VIP_QUALITY_Coef_MAX)
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
//			Scaler_SetMA_Chroma_Error(buf[12]); // merlin4 remove
			Scaler_SetRTNR_Noise_Measure(buf[13]);
			Scaler_SetMADI_HME(buf[14]);
			Scaler_SetMADI_HMC(buf[15]);
			Scaler_SetMADI_PAN(buf[16]);
			Scaler_SetDiSmd(buf[17]);
			Scaler_SetNew_UVC(buf[18]);
			//19 not use
			Scaler_SetVDCti_LPF(buf[20]);
		}
			}

		break;
	case TOOLACCESS_Function_Extend_ALL:

		if (rtice_param.mode0 == 0) {
			if (size > VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend_Coef_MAX)
				return -ID_SIZE_ERR;

			memcpy((unsigned char *) &(gVip_Table->VIP_QUALITY_Extend_Coef[0][0]), buf, size);

		}else if (rtice_param.mode0 == 1) {
			if (size > VIP_QUALITY_Extend_Coef_MAX)
				return -ID_SIZE_ERR;

			Scaler_SetCDSTable(buf[0]);//[0]
			Scaler_SetEMF_Mk2(buf[1]);//[1]
			//not used in vip handle//[2]
			//not used in vip handle//[3]
			//not used in vip handle//[4]
			//not used in vip handle//[5]
	#ifdef LocalDimming
			Scaler_SetLocalDimmingEnable(buf[6]);
			Scaler_SetLocalDimmingTable(buf[7]);
	#endif
		}
		break;

	case TOOLACCESS_Function_Extend2_ALL:

		if (rtice_param.mode0 == 0) {
			if (size > VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend2_Coef_MAX)
				return -ID_SIZE_ERR;

			memcpy((unsigned char *) &(gVip_Table->VIP_QUALITY_Extend2_Coef[0][0]), buf, size);

		} else if(rtice_param.mode0 == 1){
			if (size > VIP_QUALITY_Extend2_Coef_MAX)
				return -ID_SIZE_ERR;

			Scaler_SetScalePK(buf[2]);
			Scaler_SetSUPKType(buf[3]);
			Scaler_SetUnsharpMask(buf[4]);
			Scaler_setdESM(buf[5]);
			Scaler_setiESM(buf[6]);
			Scaler_SetScaleDOWNHTable(buf[10]);
			Scaler_SetScaleDOWNVTable(buf[11]);
			Scaler_SetScaleDOWN444To422(buf[12]);
			Scaler_SetScaleUPH_8tap(buf[14]);
			Scaler_SetScaleUPV_6tap(buf[15]);
			Scaler_SetScaleUPDir(buf[17]);
		}

		break;
	case TOOLACCESS_Function_Extend3_ALL:

		if (rtice_param.mode0 == 0) {
			if (size > VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_Extend3_Coef_MAX)
				return -ID_SIZE_ERR;

			memcpy((unsigned char *) &(gVip_Table->VIP_QUALITY_Extend3_Coef[0][0]), buf, size);

		} else if(rtice_param.mode0 == 1){
			if (size > VIP_QUALITY_Extend3_Coef_MAX)
				return -ID_SIZE_ERR;

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
			//Scaler_set_blue_stretch(buf[13]);//k5l hw remove
			Scaler_SetMBSUPeaking(buf[20]);


		}

		break;
	case TOOLACCESS_Function_Extend4_ALL:

		break;

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

	case TOOLACCESS_SoftwareBaseLocalContrastCurve:
	{
		if (size > sizeof(unsigned int)*(33))
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(33 * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy((unsigned char*)pwUINT32Temp, buf, size);
		fwif_color_ChangeINT32Endian((unsigned int *)pwUINT32Temp, size/sizeof(unsigned int), 1);

		if (down_trylock(&LC_Semaphore) == 0) //get semaphore ok
		{
			drvif_color_set_SLC_Curve(pwUINT32Temp, rtice_param.mode0);
			up(&LC_Semaphore);
		}

		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;
	}
	break;

	case TOOLACCESS_ColorFacTable:
	{
		extern StructColorDataFacModeType g_curColorFacTableEx;
		unsigned char *tmp = (unsigned char *)&g_curColorFacTableEx;

		if (size+start_addr > sizeof(g_curColorFacTableEx))
			return -ID_SIZE_ERR;

		memcpy(tmp+start_addr, buf, size);
	}
	break;

	case TOOLACCESS_SRGB_33_Matrix:
		if (size > 9 * (sizeof(short)))
			return -ID_SIZE_ERR;
		pwINT16Temp = (short *)vip_malloc(9 * sizeof(short));

		if (pwINT16Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy((unsigned char *)pwINT16Temp, (unsigned char *)buf, size);

		fwif_color_ChangeUINT16Endian((short *)pwINT16Temp , size/2, 1);

		memcpy((unsigned char *)sRGB_APPLY, (unsigned char *)pwINT16Temp, size);

		fwif_sRGB_SetMatrix(SLR_MAIN_DISPLAY, sRGB_APPLY, 1);

		vip_free(pwINT16Temp);
		pwINT16Temp = NULL;

		break;

	case TOOLACCESS_ICM_GainAdjust:
		return -ID_SIZE_ERR; //don't allow write
		break;

	case TOOLACCESS_ICM_K5_STRUCTURE:
	{
		extern void vpq_update_icm(unsigned char waiting);
		extern UINT8 g_Color_Mode;
		extern CHIP_CM_REGION_T g_cm_rgn_adv;
		extern CHIP_CM_REGION_T g_cm_rgn_exp;
		extern CHIP_COLOR_CONTROL_T g_cm_ctrl_adv;
		extern CHIP_COLOR_CONTROL_T g_cm_ctrl_exp;
		CHIP_CM_REGION_T *cm_rgn_tmp;
		CHIP_COLOR_CONTROL_T *cm_ctrl_tmp;
		extern COLORELEM_TAB_T icm_tab_elem_of_vip_table;
		extern COLORELEM_TAB_T icm_tab_elem_write;
		UINT16 i;
		UINT32 index = 0;
		if (size > TV006_ICM_DB_ITEM_NUM * (sizeof(short)))
			return -ID_SIZE_ERR;

		pwINT16Temp = (short *)vip_malloc(TV006_ICM_DB_ITEM_NUM * sizeof(short));
		if (pwINT16Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy((unsigned char *)pwINT16Temp, (unsigned char *)buf, (sizeof(short)) * TV006_ICM_DB_ITEM_NUM);
		fwif_color_ChangeUINT16Endian((short *)pwINT16Temp , TV006_ICM_DB_ITEM_NUM, 1);

		if (g_Color_Mode == COLOR_MODE_EXP) {
			cm_rgn_tmp = &g_cm_rgn_exp;
			cm_ctrl_tmp = &g_cm_ctrl_exp;
		} else {
			cm_rgn_tmp = &g_cm_rgn_adv;
			cm_ctrl_tmp = &g_cm_ctrl_adv;
		}

		//get 683 UINT16 para to ICM K5 structure
		/*copy stColorRegion*/
		for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++)
		{
			cm_rgn_tmp->stColorRegionType.stColorRegion[i].enable = pwINT16Temp[index++];
			cm_rgn_tmp->stColorRegionType.stColorRegion[i].regionNum = pwINT16Temp[index++];
			memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].hue_x[0]), (pwINT16Temp + index), 8 * sizeof(short));
			index = index + 8;
			memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].hue_g[0]), (pwINT16Temp + index), 8 * sizeof(short));
			index = index + 8;
			memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].sat_x[0]), (pwINT16Temp + index), 8 * sizeof(short));
			index = index + 8;
			memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].sat_g[0]), (pwINT16Temp + index), 8 * sizeof(short));
			index = index + 8;
			memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].Int_x[0]), (pwINT16Temp + index), 8 * sizeof(short));
			index = index + 8;
			memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].Int_g[0]), (pwINT16Temp + index), 8 * sizeof(short));
			index = index + 8;
			memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].r_offset_by_hue[0]), (pwINT16Temp + index), 8 * sizeof(short));
			index = index + 8;
			memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].g_offset_by_hue[0]), (pwINT16Temp + index), 8 * sizeof(short));
			index = index + 8;
			memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].b_offset_by_hue[0]), (pwINT16Temp + index), 8 * sizeof(short));
			index = index + 8;
		}
		/*copy gain_by_sat(int)_for_rgb_offset*/
		memcpy(&(cm_rgn_tmp->stColorRegionType.gain_by_sat_for_rgb_offset[0]), (pwINT16Temp + index), 8 * sizeof(short));
		index = index + 8;
		memcpy(&(cm_rgn_tmp->stColorRegionType.gain_by_int_for_rgb_offset[0]), (pwINT16Temp + index), 8 * sizeof(short));
		index = index + 8;
		/*copy keepY*/
		cm_rgn_tmp->stColorRegionType.keepY = pwINT16Temp[index];

		for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++)
		{
			cm_ctrl_tmp->stColor[i].masterGain = pwINT16Temp[TV006_ICM_DB_COLOR_REGION_NUM + i * 5];
			cm_ctrl_tmp->stColor[i].RGBOffset_masterGain = pwINT16Temp[TV006_ICM_DB_COLOR_REGION_NUM + 1 + i * 5];
			cm_ctrl_tmp->stColor[i].stColorGain.stColorGainHue = pwINT16Temp[TV006_ICM_DB_COLOR_REGION_NUM + 2 + i * 5];
			cm_ctrl_tmp->stColor[i].stColorGain.stColorGainSat = pwINT16Temp[TV006_ICM_DB_COLOR_REGION_NUM + 3 + i * 5];
			cm_ctrl_tmp->stColor[i].stColorGain.stColorGainInt = pwINT16Temp[TV006_ICM_DB_COLOR_REGION_NUM + 4 + i * 5];
		}

		#if 0	/*debug*/
		{
		for (i = 0; i < TV006_ICM_DB_ITEM_NUM; i++) {
			printk("[ICM DB], pwINT16Temp[%d] = %d\n", i, pwINT16Temp[i]);
			if (i < TV006_ICM_DB_COLOR_REGION_NUM) printk("[ICM DB], rgn[%d] = %d\n", i, (*((short *)(&g_cm_rgn_adv_k5) + i)));
		}
		}
		#endif

		//set ICM & RGB offset
		fwif_color_calc_ICM_gain_table_driver_tv006(&(cm_rgn_tmp->stColorRegionType), cm_ctrl_tmp, &icm_tab_elem_of_vip_table, &icm_tab_elem_write);
		vpq_update_icm(TRUE);
		fwif_color_icm_set_pillar();
		fwif_color_set_ICM_RGB_offset_tv006();


		vip_free(pwINT16Temp);
		pwINT16Temp = NULL;
		break;
	}


	case TOOLACCESS_LOGO_DETECT_GET:
	{
#if 0
		//extern unsigned char LD_flag;
		// buf[0] : 0 : do noting 1 : dump logo 2: dump interal array
		// buf[1] : dumepr number
		// buf[2] : offset
		// buf[3] : remap the virtual address before dump
		UINT8 dump_num = 0;
		UINT8 garbage_offset = 0;
		if (size > sizeof(char) * 4)
			return -ID_SIZE_ERR;

		if (buf[0] == 1)
		{
			if( buf[3] != 1 )
			{
				Scaler_Get_Logo_Detect_VirAddr();
			}
			dump_num = buf[1];
			while(dump_num > 0)
			{
				garbage_offset = buf[2];
				dump_logo_detect_to_file(garbage_offset);
				msleep(10);
				dump_num--;
			}
			//LD_flag = 1;
			printk("TOOLACCESS_LOGO_DETECT_GET %d logo file\n", buf[1]);
		}

             #ifdef VPQ_COMPILER_ERROR_ENABLE_MERLIN6 // sld_demura

		if (buf[0] == 2)
		{
			dump_num = buf[1];
			while(dump_num > 0)
			{
				dump_sld_calc_buffer_to_file();
				dump_num--;
			}
			printk("TOOLACCESS_LOGO_DETECT_GET %d buffer film\n", buf[1]);
		}
            #endif
#endif
		break;
	}

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
			fwif_color_set_DSE(SLR_MAIN_DISPLAY, &lut);

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
	case TOOLACCESS_OD_Table_33x33:
	{
		extern unsigned char od_delta_table_g[OD_table_length];
		extern unsigned char od_target_table_g[OD_table_length];
		if (size > sizeof(unsigned char)*OD_table_length)
			return -ID_SIZE_ERR;

		memcpy(od_target_table_g, buf, OD_table_length*sizeof(unsigned char));
		fwif_color_od_table_transform(0, rtice_param.mode0, od_target_table_g, od_delta_table_g);
		drvif_color_od_table_33x33_set(od_delta_table_g, 0, 255);// 0:delta mode 1:target mode(output mode)
		rtd_printk(KERN_EMERG,"VPQ_OD", "%s, od_delta_table_g updated\n", __func__);

		break;
	}
	case TOOLACCESS_OD_Table_17x17:
	{
		extern unsigned char od_delta_table_g[OD_table_length];
		if (size > sizeof(unsigned char)*OD_table_length)
			return -ID_SIZE_ERR;

		fwif_color_od_table_transform(0, 0, buf, od_delta_table_g);
		drvif_color_od_table_17x17_set(od_delta_table_g, 0, 255);// 0:delta mode 1:target mode(output mode)
		rtd_printk(KERN_EMERG,"VPQ_OD", "%s, od_delta_table_g updated\n", __func__);

		break;
	}
	case TOOLACCESS_DI_PAN_detect:
	{
		if (size > sizeof(unsigned short)*pan_detect_var)
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(pan_detect_var * sizeof(unsigned short));
		if(pwUINT16Temp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(unsigned short), 1);

		memcpy((unsigned char*)pwUINT16Temp, buf, size);

		drvif_color_ma_Pan_Detection_set(pwUINT16Temp);

		vip_free(pwUINT16Temp);
		pwUINT16Temp = NULL;

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
	case TOOLACCESS_VPQ_HDR_IOCTL_StopRun_cmd:
	{
		int i;
		if (size > sizeof(unsigned char)*256)
			return -ID_SIZE_ERR;

		for (i = 0; i < 256 && i < size/sizeof(unsigned char); i++)
			vpq_HDR_ioctl_set_stop_run_by_idx(i, buf[i]);

		break;
	}
	case TOOLACCESS_VPQ_dolbyHDR_IOCTL_StopRun_cmd:
	{
		int i;
		if (size > sizeof(unsigned char)*256)
			return -ID_SIZE_ERR;

		for (i = 0; i < 256 && i < size/sizeof(unsigned char); i++)
			vpq_dolbyHDR_ioctl_set_stop_run_by_idx(i, buf[i]);

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
			if (pwINT16Temp != NULL) {
				memcpy((unsigned char *)pwINT16Temp, (unsigned char *)buf, size);
				fwif_color_ChangeUINT16Endian(pwINT16Temp, 9, 1);
				(fwif_color_get_demo_callback(DEMO_CALLBACKID_OVERSCAN))((void *)pwINT16Temp);
				vip_free(pwINT16Temp);
				pwINT16Temp = NULL;
			}
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
		if (size > sizeof(DRV_TC_HDR_CTRL))
			return -ID_SIZE_ERR;
		pTC_HDR_CTRL = (DRV_TC_HDR_CTRL*)fwif_color_Get_TC_CTRL();
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		/*fwif_color_set_TC_v130_metadata2reg_init(TC_HDR_CTRL.TC_CTRL[TC_HDR_display_OETF], TC_HDR_CTRL.TC_CTRL[TC_HDR_display_Brightness],
			TC_HDR_CTRL.TC_CTRL[TC_HDR_display_proc_mode], 0, 0, TC_HDR_CTRL.TC_CTRL[TC_HDR_display_yuv_range]);*/

		memcpy(	(unsigned char*)pTC_HDR_CTRL, &buf[0], size);

		RPC_system_setting_info->HDR_info.Debug_Log_2 = (pTC_HDR_CTRL->TC_Debug_Log & 0xFF);
		RPC_system_setting_info->HDR_info.Debug_Log_Delay_Frame = (pTC_HDR_CTRL->TC_Debug_Delay & 0xFF);

		break;

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
		if (drvif_fwif_color_Get_ST2094_TBL()->EOTF_Gamma22 == 1)
			drvif_DM2_GAMMA_Enable(1);
		else
			drvif_DM2_GAMMA_Enable(0);

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
		if (drvif_fwif_color_Get_ST2094_TBL()->EOTF_Gamma22 == 1)
			drvif_DM2_GAMMA_Enable(1);
		else
			drvif_DM2_GAMMA_Enable(0);

		break;
	}

	case TOOLACCESS_OSD_HDRMODE_DEMO:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		fwif_color_set_OSD_enhance_enable(buf[0]);
		break;

	case TOOLACCESS_LOCALCONTRAST_DEMO:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_color_LC_DemoSplit_Screen(buf[0]);
		break;

	case TOOLACCESS_CHANGE_PQ_FLOW:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (buf[0] == 0) {
			PQ_FLOW_Mode = 0;
			vpq_project_id = 0x00060000;
			vpqex_project_id = 0x00060000;
			Scaler_color_Set_HDR_AutoRun(0);
			vpq_ioctl_set_stop_run_by_idx(0, 0);

		} else if (buf[0] == 1) {
			PQ_FLOW_Mode = 1;
			vpq_project_id = 0x00060000;
			vpqex_project_id = 0x00010000;
			Scaler_color_Set_HDR_AutoRun(1);
			vpq_ioctl_set_stop_run_by_idx(0, 1);

		}

		break;

	case TOOLACCESS_DM_HDR_3D_Lut_TBL:
		if (size > (sizeof(unsigned int) * (VIP_DM_HDR_3D_LUT_UI_TBL_SIZE)))
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(VIP_DM_HDR_3D_LUT_UI_TBL_SIZE * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(VIP_DM_HDR_3D_LUT_UI_TBL_SIZE * sizeof(short));
		if (pwUINT16Temp == NULL){
			vip_free(pwUINT32Temp);
			pwUINT32Temp = NULL;
			return -ID_SIZE_ERR;
		}

		memcpy(pwUINT32Temp, buf, size);
		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/sizeof(int), 1);

		for(i=0;i<size/sizeof(int);i++)
			pwUINT16Temp[i] = pwUINT32Temp[i];

		fwif_color_rtice_DM2_3D_17x17x17_LUT_Set(1, pwUINT16Temp);

		//fwif_color_set_DM_HDR_3dLUT(255, pwUINT32Temp);

		vip_free(pwUINT32Temp);
		vip_free(pwUINT16Temp);
		pwUINT32Temp = NULL;
		pwUINT16Temp = NULL;
		break;

	case TOOLACCESS_DM2_HDR_EOTF_TBL:
	case TOOLACCESS_DM2_HDR_OETF_TBL:
	case TOOLACCESS_DM2_HDR_Tone_Mapping_TBL:
	case TOOLACCESS_DM2_HDR_24x24x24_3D_TBL:
	case TOOLACCESS_DM2_HDR_Hist_RGB_max:
	case TOOLACCESS_DM2_HDR_Hist_Auto_Mode:
	case TOOLACCESS_DM2_HDR_Hist_Auto_Mode_TBL:
	case TOOLACCESS_DM2_HDR_Sat_Hist_TBL:
	case TOOLACCESS_DM2_HDR_Hue_Hist_TBL:	
	case TOOLACCESS_DM2_HDR_Sat_Hue_Hist_Skip:
		ret = rtice_SetGet_HDR_DM2(rtice_param, NULL, buf, system_setting_info, 1);
		if (ret < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_D_3D_Lut_TBL:
	{
		//SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

		if (size > (sizeof(unsigned int) * (VIP_D_3D_LUT_UI_TBL_SIZE)))
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(VIP_D_3D_LUT_UI_TBL_SIZE * sizeof(int));

		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwUINT32Temp, buf, size);

		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/sizeof(int), 1);

		//fwif_color_set_D_3dLUT(255, pwUINT32Temp);
		fwif_color_D_LUT_By_DMA(0, 1, pwUINT32Temp);

		if (idx < LUT3D_TBL_Num) {
			memcpy(gVip_Table->LUT_3D[idx], D_3D_DRV_LUT, sizeof(D_3D_DRV_LUT));
		}/* else if (idx < IDX_3DLUT_MAX) {
			memcpy(LG_LUT_3D_EXTEND[idx-IDX_3DLUT_BIG_TABLE_MAX], D_3D_DRV_LUT, sizeof(D_3D_DRV_LUT));
		}*/ //no more used

		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		break;
	}
	
	case TOOLACCESS_DM2_EOTF_byLuminance_CTRL:
		if (size > (sizeof(unsigned short)*2))
			return -ID_SIZE_ERR;

		fwif_color_ChangeUINT16Endian((unsigned short*)&buf[0], size/sizeof(short), 1);
		memcpy((unsigned char*)&VIP_HDR_EOTF_ByLuminace_CTRL[0], (unsigned char*)&buf[0], size);
		break;

	case TOOLACCESS_DM2_OETF_byLuminance_CTRL:
		if (size > (sizeof(unsigned short)*1))
			return -ID_SIZE_ERR;

		fwif_color_ChangeUINT16Endian((unsigned short*)&buf[0], size/sizeof(short), 1);
		memcpy((unsigned char*)&(HDR10_OETF_ByLuminace_TBL.panel_Luminace), (unsigned char*)&buf[0], size);
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

        case TOOLACCESS_Sharpness_Table_TV006:

		if (size > (sizeof(CHIP_SHARPNESS_T)+sizeof(CHIP_SR_UI_T)))
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(sizeof(CHIP_SHARPNESS_T)+sizeof(CHIP_SR_UI_T));
		if(pwUINT16Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwUINT16Temp,buf , size);

		fwif_color_ChangeUINT16Endian(pwUINT16Temp, size/sizeof(short), 1);

                fwif_color_set_sharpness_table_TV006((CHIP_SHARPNESS_UI_T *)pwUINT16Temp, (CHIP_SR_UI_T*)&pwUINT16Temp[545]);


		vip_free(pwUINT16Temp);
		pwUINT16Temp = NULL;


                break;

        case TOOLACCESS_NR_Table_TV006:

		if (size > (sizeof(short)*295))
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(sizeof(short)*295);
		if(pwUINT16Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwUINT16Temp,buf , size);

		fwif_color_ChangeUINT16Endian(pwUINT16Temp, size/sizeof(short), 1);

                fwif_color_set_DB_NR_tv006(pwUINT16Temp ); //for rtice use



		vip_free(pwUINT16Temp);
		pwUINT16Temp = NULL;

                break;

		case TOOLACCESS_OPS_En:

			if (size > (sizeof(short)))
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT16Endian((short*)buf, 1, 1);
			pwUINT16Temp = (unsigned short*)buf;

			OPS_CTRL_item.OPS_enable = *pwUINT16Temp;
			
			fwif_color_OPS_Debug_init();

			fwif_color_OPS_init();
			// wait DB
			//msleep(50);
			

		break;

		case TOOLACCESS_OPS_TPC:

			if ((size+rtice_param.saddr) > (sizeof(VIP_OPS_TPC_Item)))
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);
			pwUINT16Temp = (unsigned short*)buf;
			memcpy(((unsigned short*)&OPS_CTRL_item.OPS_TPC_Item) + rtice_param.saddr, pwUINT16Temp, size);

		break;

		case TOOLACCESS_OPS_LBC:

			if ((size+rtice_param.saddr) > (sizeof(VIP_OPS_LBC_Item)))
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);
			pwUINT16Temp = (unsigned short*)buf;
			memcpy(((unsigned short*)&OPS_CTRL_item.OPS_LBC_Item) + rtice_param.saddr, pwUINT16Temp, size);

		break;

		case TOOLACCESS_OPS_CPC:

			if ((size+rtice_param.saddr) > (sizeof(VIP_OPS_CPC_Item)))
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);
			pwUINT16Temp = (unsigned short*)buf;
			memcpy(((unsigned short*)&OPS_CTRL_item.OPS_CPC_Item) + rtice_param.saddr, pwUINT16Temp, size);

		break;

		case TOOLACCESS_OPS_Debug:

			if ((size+rtice_param.saddr) > (sizeof(VIP_OPS_Debug_Item)))
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);
			pwUINT16Temp = (unsigned short*)buf;
			memcpy(((unsigned short*)&OPS_CTRL_item.OPS_Debug_Item) + rtice_param.saddr, pwUINT16Temp, size);

		break;

		case TOOLACCESS_Force_MEX_En_ALL:

			if ((size+rtice_param.saddr) > (sizeof(unsigned char)))
				return -ID_SIZE_ERR;
			
			MEX_MODE_Force_En_All = buf[0];
			fwif_color_MEX_MODE_init();
	
		break;
		
	case TOOLACCESS_VIP_DeMura_TBL_Mode:
	case TOOLACCESS_VIP_DeMura_TBL:
	case TOOLACCESS_VIP_DeMura_INI:
	case TOOLACCESS_VIP_DeMura_Adaptive:
	case TOOLACCESS_VIP_DeMura_Channel_Mode:
	case TOOLACCESS_VIP_DeMura_READ_INX_DATA:
		if(rtice_SetGet_DeMura(&rtice_param, 0, buf, system_setting_info, 1) < 0)
			return -ID_SIZE_ERR;
		break;
/*
	case TOOLACCESS_VIP_SLD:
		if (size > (sizeof(int)*8))
			return -ID_SIZE_ERR;

		pwINT32Temp = (int *)vip_malloc(sizeof(int)*8);

		if(pwINT32Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwINT32Temp,(int *)buf , size);

		fwif_color_ChangeINT32Endian(pwINT32Temp, size/sizeof(int), 1);

		thl_count_static_time = pwINT32Temp[5];
		thl_stop_global_drop_count_frame = pwINT32Temp[6];

                  fwif_k6_sld(pwINT32Temp[0], pwINT32Temp[1], pwINT32Temp[2], pwINT32Temp[3], (unsigned char)pwINT32Temp[4],(unsigned int)pwINT32Temp[7]); //for rtice use

	         //fwif_color_sld_still_image(&(pwINT32Temp[5]),&(pwINT32Temp[6]));

		vip_free(pwINT32Temp);
		pwINT32Temp = NULL;
		break;
*/

	case TOOLACCESS_VIP_SLD:
		if (size > (sizeof(int)*50))
			return -ID_SIZE_ERR;

		pwINT32Temp = (int *)vip_malloc(sizeof(int)*50);

		if(pwINT32Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwINT32Temp,(int *)buf , size);
		fwif_color_ChangeINT32Endian(pwINT32Temp, size/sizeof(int), 1);


		cmd_dY_preCur_mouseCon		     							= pwINT32Temp[0];
		cmd_LC_period				     								= pwINT32Temp[1];
		cmd_thl_counter_UR_con		     								= pwINT32Temp[2];
		cmd_counter_frame_highY_blk		 						= pwINT32Temp[3];
		cmd_lock_fw_gain_frame										= pwINT32Temp[4];
		cmd_pixel_gain_step											= pwINT32Temp[5];
		cmd_thl_counter_blk_pre_cur_same_mouseCon				= pwINT32Temp[6];
		cmd_thl_error_time_box_logo_case							= pwINT32Temp[7];
		cmd_thl_counter_box_logo_case								= pwINT32Temp[8];
		cmd_thl_error_time_NEWS_case								= pwINT32Temp[9];
		cmd_thl_counter_NEWS_case									= pwINT32Temp[10];
		cmd_thl_error_time_noise_case								= pwINT32Temp[11];
		cmd_thl_counter_noise_case									= pwINT32Temp[12];
		cmd_thl_dC													= pwINT32Temp[13];
		cmd_thl_dUDL												= pwINT32Temp[14];
		cmd_rgb_box_leave_idx_delete_step							= pwINT32Temp[15];
		cmd_box_logo_pixel_gain									= pwINT32Temp[16];
		cmd_box_logo_maxGain_time								= pwINT32Temp[17];
		cmd_box_lock_frame											= pwINT32Temp[18];

		vip_free(pwINT32Temp);
		pwINT32Temp = NULL;
		break;
	
	case TOOLACCESS_VIP_LEA_apply_filter:
		if ((rtice_param.saddr*sizeof(short)+size) > sizeof(sld_work_filter_struct))
			return -ID_SIZE_ERR;

		pwUINT16Temp = &sld_work_filter.filter_w[0];

		fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);

		memcpy(pwUINT16Temp+rtice_param.saddr, buf, size);
		break;
	
	case TOOLACCESS_VIP_LEA_debug:
		if ((rtice_param.saddr*sizeof(short)+size) > sizeof(sld_work_debug_struct))
			return -ID_SIZE_ERR;

		pwUINT16Temp = &sld_work_debug.pr_En;

		fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);

		memcpy(pwUINT16Temp+rtice_param.saddr, buf, size);
		break;

	case TOOLACCESS_VIP_LEA:
		if ((rtice_param.saddr*sizeof(int) + size) > (sizeof(int)*26))
			return -ID_SIZE_ERR;

		pwINT32Temp = (int *)vip_malloc(sizeof(int)*26);

		if(pwINT32Temp == NULL) {
			rtd_pr_vpq_emerg("TOOLACCESS_VIP_LEA, pwINT32Temp = NULL\n");			
			return -ID_SIZE_ERR;
		}

		pwINT32Temp[0]  = LSC_by_memc_logo_en;
		pwINT32Temp[1]  = memc_logo_to_demura_wait_time;
		pwINT32Temp[2]  = memc_logo_to_demura_drop_time;
		pwINT32Temp[3]  = memc_logo_to_demura_drop_limit;
		pwINT32Temp[4]  = memc_logo_to_demura_ascend_speed;
		pwINT32Temp[5]  = memc_logo_read_en;
		pwINT32Temp[6]  = logo_to_demura_en;
		pwINT32Temp[7]  = memc_logo_fill_hole_en;
		pwINT32Temp[8]  = memc_logo_filter_en;
		pwINT32Temp[9]  = memc_logo_to_demura_update_cnt;
		pwINT32Temp[10] = (unsigned int)before_phyAddr;
		pwINT32Temp[11] = (unsigned int)after_phyAddr;
		pwINT32Temp[12] = (unsigned long)LD_virAddr;
		pwINT32Temp[13] = sld_ddr_offset_auto_get;
		pwINT32Temp[14] = (unsigned int)sld_ddr_offset;
		pwINT32Temp[15] = (unsigned int)dvr_to_phys((void*)LD_virAddr);
		pwINT32Temp[16] = (unsigned int)SLD_SW_En;
		pwINT32Temp[17] = (unsigned int)SLD_BY_MEMC_DEMURA_LOGO_ONLY;
		pwINT32Temp[18] = (unsigned int)memc_logo_LD_APL_check;
		pwINT32Temp[19] = (unsigned int)memc_logo_to_demura_APL_max_th;
		pwINT32Temp[20] = (unsigned int)fill_hole_th;
		pwINT32Temp[21] = (unsigned int)memc_logo_to_demura_drop_limit_gain;
		pwINT32Temp[22] = (unsigned int)memc_logo_filter_SLD_APL_SE_en;
		pwINT32Temp[23] = (unsigned int)memc_logo_to_demura_drop_limit_gain_SP;
		pwINT32Temp[24] = (unsigned int)SLD_APL_reset_cnt_th;
		pwINT32Temp[25] = (unsigned int)Gloabal_SLD_En;
		
		memcpy(pwINT32Temp+rtice_param.saddr, (int *)buf , size);
		fwif_color_ChangeINT32Endian(pwINT32Temp, size/sizeof(int), 1);

		/*
		filename_memc = pwINT32Temp[3];
	       start_offset_byte= pwINT32Temp[4];

		if (pwINT32Temp[0] == 1) {
			Scaler_Get_Logo_Detect_VirAddr();
			dump_num_ = pwINT32Temp[1];
			while(dump_num_ > 0) {
				garbage_offset_ = pwINT32Temp[2];
				dump_logo_detect_to_file(garbage_offset_);
				msleep(10);
				dump_num_--;
			}
			printk("TOOLACCESS_LOGO_DETECT_GET %d film\n", pwINT32Temp[1]);
		} else {

		}
		*/

		LSC_by_memc_logo_en = pwINT32Temp[0];
		memc_logo_to_demura_wait_time = pwINT32Temp[1];
		memc_logo_to_demura_drop_time = pwINT32Temp[2];
		memc_logo_to_demura_drop_limit = pwINT32Temp[3];
		memc_logo_to_demura_ascend_speed = pwINT32Temp[4];
		memc_logo_read_en = pwINT32Temp[5];
		logo_to_demura_en = pwINT32Temp[6];
		memc_logo_fill_hole_en = pwINT32Temp[7];
		memc_logo_filter_en = pwINT32Temp[8];
		memc_logo_to_demura_update_cnt = pwINT32Temp[9];
		//before_phyAddr = pwINT32Temp[10];
		//after_phyAddr = pwINT32Temp[11];
		//LD_virAddr = pwINT32Temp[12];
		sld_ddr_offset_auto_get = pwINT32Temp[13];
		sld_ddr_offset = pwINT32Temp[14];
		SLD_SW_En = pwINT32Temp[16];
		SLD_BY_MEMC_DEMURA_LOGO_ONLY = pwINT32Temp[17];
		memc_logo_LD_APL_check = pwINT32Temp[18];
		memc_logo_to_demura_APL_max_th = pwINT32Temp[19];
		fill_hole_th = pwINT32Temp[20];
		//
		memc_logo_to_demura_drop_limit_gain = pwINT32Temp[21];
		memc_logo_filter_SLD_APL_SE_en = pwINT32Temp[22];
		memc_logo_to_demura_drop_limit_gain_SP = pwINT32Temp[23];
		SLD_APL_reset_cnt_th = pwINT32Temp[24];
		Gloabal_SLD_En = pwINT32Temp[25];
		
		vip_free(pwINT32Temp);
		pwINT32Temp = NULL;
		break;

	case TOOLACCESS_VIP_SLD_Hybrid_logo_Debug_Show:
	{
		if(size > sizeof(char))
			return -ID_SIZE_ERR;
		SLD_Hybrid_mode_logo_debug_show = *buf;
		drvif_color_set_SLD_Hybrid_debug_logo_show_rtice(SLD_Hybrid_mode_logo_debug_show);
		break;
	}

	case TOOLACCESS_VIP_SLD_swap_location:
	{
		if(size > sizeof(char))
			return -ID_SIZE_ERR;
		SLD_swap_location= *buf;
		drvif_color_set_SLD_swap_location(SLD_swap_location);
		break;
	}

	case TOOLACCESS_VIP_SLD_drop_gain:
	{
		if(size > sizeof(char))
			return -ID_SIZE_ERR;
		
		memc_logo_to_demura_drop_limit= buf[0];
		
		break;
	}

	case TOOLACCESS_VIP_SLD_Global_SLD_CTRL:
		if (size+start_addr > sizeof(VIP_SLD2GlobalSLD_CTRL))
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short*)drvif_fwif_color_get_SLD2GlobalSLD_CTRL();

		fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);

		memcpy(pwUINT16Temp+start_addr, buf, size);

		break;

	case TOOLACCESS_VIP_SLD_Global_SLD_TBL:
		if (size+start_addr > sizeof(DRV_GSR2_Table))
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short*)drvif_fwif_color_get_SLD2GlobalSLD_TBL();

		fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);
		
		memcpy(pwUINT16Temp+start_addr, buf, size);

		break;

	case TOOLACCESS_VIP_Force_vTop:
		if(rtice_SetGet_Froce_vTop_TBL(&rtice_param, 0, buf, system_setting_info, 1) < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_VIP_ISR_set_PWM_Enable:
	{
		if(size > sizeof(char))
			return -ID_SIZE_ERR;
		set_DCR_PWM_Enable = *buf;
		break;
	}

	case TOOLACCESS_VIP_set_DCR_Parameter:
	{
		if(size > sizeof(int) * 7)
			return -ID_SIZE_ERR;
		pwUINT32Temp = (unsigned int *)vip_malloc(sizeof(int)*7);

		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwUINT32Temp, buf, size);
		fwif_color_ChangeUINT32Endian(pwUINT32Temp, 7, 1);

		system_setting_info->OSD_Info.isIncreaseMode_Flag	=	*pwUINT32Temp;
		system_setting_info->OSD_Info.backlightLevel_uiMax	=	*(pwUINT32Temp+1);
		system_setting_info->OSD_Info.backlightLevel_uiMin	=	*(pwUINT32Temp+2);
		system_setting_info->OSD_Info.backlightLevel_actMax	=	*(pwUINT32Temp+3);
		system_setting_info->OSD_Info.backlightLevel_actMin	=	*(pwUINT32Temp+4);
		system_setting_info->OSD_Info.OSD_DCR_Mode	=	*(pwUINT32Temp+5);
		system_setting_info->OSD_Info.OSD_Backlight	=	*(pwUINT32Temp+6);
		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;
		break;
	}


	case TOOLACCESS_VIP_ICM_DMA_DEBUG:
	{
		break;
	}
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
		void drvif_color_D_3dLUT_Random(void);
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

	case TOOLACCESS_Set_VIP_Disable_IPQ:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetIPQ_EN(buf[0]);
	break;

	case TOOLACCESS_Set_VIP_Disable_IPQCurve:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetIPQCurve_EN(buf[0]);

	break;
    
	case TOOLACCESS_SQM_PQA_PARAM:
	{
		unsigned int param[2];
		extern unsigned int g_PQA_IIR_Weight;
		extern unsigned int g_PQA_SRNN_cnt_th;

		if (size+start_addr*sizeof(unsigned int) > sizeof(param))
			return -ID_SIZE_ERR;

		param[0] = g_PQA_IIR_Weight;
		param[1] = g_PQA_SRNN_cnt_th;

		if (size >= sizeof(unsigned int)) {
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);
			memcpy(param+start_addr, buf, size);
			
			g_PQA_IIR_Weight = param[0];
			g_PQA_SRNN_cnt_th = param[1];
		}
	}
	break;

/************************************************************/
/******************* For TV002 use*****************************/
/************************************************************/
	case TOOLACCESS_TV002_STYLE:
		if (size > (sizeof(char)))
			return -ID_SIZE_ERR;

		 Scaler_access_Project_TV002_Style(1, *buf);

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
	case TOOLACCESS_DBC_LUT_TV002:
	case TOOLACCESS_DBC_DCC_S_High_CMPS_LUT_TV002:
	case TOOLACCESS_DBC_DCC_S_Index_CMPS_LUT_TV002:
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

	case TOOLACCESS_SIP_1_DEBUG:
	{
#ifdef VIP_SUPPORT_SIP_1_SPI_2
		if (start_addr*sizeof(int)+size > (sizeof(int)*2))
			return -ID_SIZE_ERR;

		if (size >= sizeof(int) && start_addr == 0) {
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
			rtice_Set_SIP_1_Enable(*((unsigned int *)buf));
		}
#endif
	}
	break;

#ifdef CONFIG_CUSTOMER_TV002
	case TOOLACCESS_WBBL_enable_TV002:
	{
		extern ST_WBBL WBBL_table;
		if (size > (sizeof(unsigned char)))
			return -ID_SIZE_ERR;

		memcpy(&(WBBL_table.WBBL_enable), buf, sizeof(unsigned char));
		fwif_WBBL_TV002();
	}
	break;

	case TOOLACCESS_WBBL_table_TV002:
	{
		extern ST_WBBL WBBL_table;
		extern ST_WB_BS Blue_Stretch;

		if (size > ((WBBL_POSITION_MAX+WBBL_COLOR_MAX*WBBL_POSITION_MAX)*sizeof(unsigned short) + sizeof(ST_WB_BS)))
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(((WBBL_POSITION_MAX+WBBL_COLOR_MAX*WBBL_POSITION_MAX)*sizeof(unsigned short) + sizeof(ST_WB_BS)));

		if (pwUINT16Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwUINT16Temp, buf, size);
		fwif_color_ChangeUINT16Endian(pwUINT16Temp, size/sizeof(unsigned short), 1);

		WBBL_table.SBC_BL[0] = *pwUINT16Temp;
		WBBL_table.SBC_BL[1] = *(pwUINT16Temp+1);
		WBBL_table.SBC_BL[2] = *(pwUINT16Temp+2);
		WBBL_table.WBBL_Gain[0][0] = *(pwUINT16Temp+3);
		WBBL_table.WBBL_Gain[0][1] = *(pwUINT16Temp+4);
		WBBL_table.WBBL_Gain[0][2] = *(pwUINT16Temp+5);
		WBBL_table.WBBL_Gain[1][0] = *(pwUINT16Temp+6);
		WBBL_table.WBBL_Gain[1][1] = *(pwUINT16Temp+7);
		WBBL_table.WBBL_Gain[1][2] = *(pwUINT16Temp+8);
		WBBL_table.WBBL_Gain[2][0] = *(pwUINT16Temp+9);
		WBBL_table.WBBL_Gain[2][1] = *(pwUINT16Temp+10);
		WBBL_table.WBBL_Gain[2][2] = *(pwUINT16Temp+11);
		Blue_Stretch.WB_A_Gain[0] = *(pwUINT16Temp+12);
		Blue_Stretch.WB_A_Gain[1] = *(pwUINT16Temp+13);
		Blue_Stretch.WB_A_Gain[2] = *(pwUINT16Temp+14);
		Blue_Stretch.WB_A_Offset[0] = *(pwUINT16Temp+15);
		Blue_Stretch.WB_A_Offset[1] = *(pwUINT16Temp+16);
		Blue_Stretch.WB_A_Offset[2] = *(pwUINT16Temp+17);

		vip_free(pwUINT16Temp);
		pwUINT16Temp = NULL;

		fwif_WBBL_TV002();
		fwif_Blue_Stretch_TV002();
	}
	break;

	case TOOLACCESS_Black_Adjust_TV002:
	{
		extern VIP_DCC_Black_Adjust_Ctrl g_black_adjust_tv002;

		if (size > sizeof(g_black_adjust_tv002))
			return -ID_SIZE_ERR;

		pwUINT16Temp = (unsigned short *)vip_malloc(sizeof(g_black_adjust_tv002));
		if (pwUINT16Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwUINT16Temp, buf, size);
		fwif_color_ChangeUINT16Endian(pwUINT16Temp, size/sizeof(unsigned short), 1);
		memcpy(&g_black_adjust_tv002, pwUINT16Temp, size);
		vip_free(pwUINT16Temp);
	}
	break;

	case TOOLACCESS_PQ_Extend_Enable_TV002:
	{
		extern unsigned char g_bEnable_PQ_extend_data;

		if (size > sizeof(g_bEnable_PQ_extend_data))
			return -ID_SIZE_ERR;

		memcpy(&g_bEnable_PQ_extend_data, buf, size);
	}
	break;

	case TOOLACCESS_PQ_Extend_Index_TV002:
	case TOOLACCESS_PQ_DBC_Item_Check_TV002:
	case TOOLACCESS_PQ_SC_Item_Check_TV002:
	{
		//do noting
	}
	break;

	case TOOLACCESS_PQ_DCON_DBRI_TV002:
	{
		extern short g_TV002_Con_Offset;
		extern short g_TV002_Bri_Offset;

		if (start_addr*sizeof(short)+size > sizeof(short)*2)
			return -ID_SIZE_ERR;

		if (size >= sizeof(short)) {
			short tmp[2];
			tmp[0] = g_TV002_Con_Offset;
			tmp[1] = g_TV002_Bri_Offset;
			fwif_color_ChangeINT16Endian((short *)buf, size/sizeof(short), 1);
			memcpy(tmp+start_addr, buf, size);
			g_TV002_Con_Offset = tmp[0];
			g_TV002_Bri_Offset = tmp[1];
		}
	}
	break;

	case TOOLACCESS_PQ_DYNAMIC_SHP_TV002:
	{
		extern bool g_TV002_Dynamic_Shp_Gain_En;
		extern bool g_TV002_Dynamic_Shp_Gain_By_Y_En;
		extern bool g_TV002_Dynamic_SNR_Weight_En;
		extern int g_TV002_Dynamic_Shp_Gain;
		extern int g_TV002_Dynamic_Shp_Gain_By_Y;
		extern int g_TV002_Dynamic_SNR_Weight;

		if (start_addr*sizeof(int)+size > sizeof(int)*6)
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			int tmp[6];
			tmp[0] = g_TV002_Dynamic_Shp_Gain_En;
			tmp[1] = g_TV002_Dynamic_Shp_Gain_By_Y_En;
			tmp[2] = g_TV002_Dynamic_SNR_Weight_En;
			tmp[3] = g_TV002_Dynamic_Shp_Gain;
			tmp[4] = g_TV002_Dynamic_Shp_Gain_By_Y;
			tmp[5] = g_TV002_Dynamic_SNR_Weight;
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
			memcpy(tmp+start_addr, buf, size);

			g_TV002_Dynamic_Shp_Gain_En = tmp[0];
			g_TV002_Dynamic_Shp_Gain_By_Y_En = tmp[1];
			g_TV002_Dynamic_SNR_Weight_En = tmp[2];
			//g_TV002_Dynamic_Shp_Gain = tmp[3];
			//g_TV002_Dynamic_Shp_Gain_By_Y = tmp[4];
			//g_TV002_Dynamic_SNR_Weight = tmp[5];

			if (g_TV002_Dynamic_Shp_Gain_En == 0) {
				g_TV002_Dynamic_Shp_Gain = 128;
				fwif_color_set_sharpness_level_only_gain(Scaler_GetSharpnessTable(), Scaler_GetSharpness());
			}

			if (g_TV002_Dynamic_Shp_Gain_By_Y_En == 0) {
				g_TV002_Dynamic_Shp_Gain_By_Y = 128;
				fwif_color_Set_Sharpness_gain_by_y_only_gain(Scaler_GetSharpnessTable());
			}

			if (g_TV002_Dynamic_SNR_Weight_En == 0) {
				g_TV002_Dynamic_SNR_Weight = 128;
				fwif_color_nr_curvemapping_weight_set(Scaler_GetDNR_table(), Scaler_GetDNR());
			}

		}
	}
	break;

	case TOOLACCESS_PQ_ZERO_DIMMING_IIR_CTRL_TV002:
	{
		extern int ZD_APL_Diff_th1 ,ZD_APL_Diff_th2, ZD_IIR_Speed_low, ZD_IIR_Speed_high, ZD_RGB_Max_Limit_en, ZD_RGB_Max_Limit_th;
		extern int ZD_IIR_down_shift, ZD_IIR_up_shift;
		if (start_addr*sizeof(int)+size > sizeof(int)*8)
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			int tmp[8];
			tmp[0] = ZD_APL_Diff_th1; tmp[1] = ZD_APL_Diff_th2; tmp[2] = ZD_IIR_Speed_low; tmp[3] = ZD_IIR_Speed_high; tmp[4] = ZD_RGB_Max_Limit_en; tmp[5] = ZD_RGB_Max_Limit_th;
			tmp[6] = ZD_IIR_down_shift; tmp[7] = ZD_IIR_up_shift;
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
			memcpy(tmp+start_addr, buf, size);
			ZD_APL_Diff_th1 = tmp[0]; ZD_APL_Diff_th2 = tmp[1]; ZD_IIR_Speed_low = tmp[2]; ZD_IIR_Speed_high = tmp[3];	ZD_RGB_Max_Limit_en = tmp[4]; ZD_RGB_Max_Limit_th = tmp[5];
			ZD_IIR_down_shift = tmp[6]; ZD_IIR_up_shift = tmp[7];
		}
	}
	break;

	case TOOLACCESS_OSD_Alpha_Detect_TV002:
	{
		int alpha;
		extern int GDMA_SetAlphaOsdDetectionParameter(int data);

		if (size > sizeof(alpha))
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
			memcpy(&alpha, buf, size);
			if (GDMA_SetAlphaOsdDetectionParameter(alpha) != 0)
				return -WRITE_ERROR;
		}
	}
	break;

	case TOOLACCESS_OSD_Alpha_Ratio_TV002:
	{
		//do nothing
	}
	break;

	case TOOLACCESS_PQ_DSLC_CTRL_TV002:
	{
		extern DYNAMIC_SLC_CTRL_ST DSLC_TBL[SLC_Table_NUM];
		DYNAMIC_SLC_CTRL_RTICE_ST *pRtice = NULL;
		DYNAMIC_SLC_CTRL_ST *pDrv = NULL;

		if (start_addr*sizeof(short)+size > sizeof(DYNAMIC_SLC_CTRL_RTICE_ST))
			return -ID_SIZE_ERR;

		if (size >= sizeof(short)) {
			pRtice = (DYNAMIC_SLC_CTRL_RTICE_ST *)vip_malloc(sizeof(DYNAMIC_SLC_CTRL_RTICE_ST));
			pDrv = &DSLC_TBL[0];

			if (pRtice != NULL) {
				drvif_convert_DSLC_st(0, pDrv, pRtice);
				fwif_color_ChangeINT16Endian((short *)buf, size/sizeof(short), 1);
				memcpy((short *)pRtice+start_addr, buf, size);

				if (pRtice->u8LC_Hist_H != pDrv->u8LC_Hist_H || pRtice->u8LC_Hist_V != pDrv->u8LC_Hist_V) {
					drvif_convert_DSLC_st(1, pDrv, pRtice);
				} else {
					drvif_convert_DSLC_st(1, pDrv, pRtice);
					Scaler_Set_Dynamic_SLC_Table(Scaler_Get_Dynamic_SLC_Table());
				}
				vip_free(pRtice);
			}
		}
	}
	break;

	case TOOLACCESS_PQ_FREQ_DET_TV002:
	{
		//do nothing
	}
	break;

	case TOOLACCESS_PQ_VGIP_ISR_STATUS_TV002:
	{
		//do nothing
	}
	break;

	case TOOLACCESS_PQ_SRS_SHP_OUT:
	{
		//do nothing
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_UINT8:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -WRITE_ERROR;

		if (start_addr+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(unsigned char)) {
			memcpy(&p[start_addr], buf, size);
		}
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_UINT16:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -WRITE_ERROR;

		if (start_addr+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(unsigned short)) {
			fwif_color_ChangeUINT16Endian((unsigned short *)buf, size/sizeof(unsigned short), 1);
			memcpy(&p[start_addr], buf, size);
		}
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_UINT32:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -WRITE_ERROR;

		if (start_addr+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(unsigned int)) {
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);
			memcpy(&p[start_addr], buf, size);
		}
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_SINT8:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -WRITE_ERROR;

		if (start_addr+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(char)) {
			memcpy(&p[start_addr], buf, size);
		}
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_SINT16:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -WRITE_ERROR;

		if (start_addr+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(short)) {
			fwif_color_ChangeINT16Endian((short *)buf, size/sizeof(short), 1);
			memcpy(&p[start_addr], buf, size);
		}
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_SINT32:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -WRITE_ERROR;

		if (start_addr+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
			memcpy(&p[start_addr], buf, size);
		}
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_UINT16_ALIGN:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -WRITE_ERROR;

		if (start_addr*2+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(unsigned short)) {
			fwif_color_ChangeUINT16Endian((unsigned short *)buf, size/sizeof(unsigned short), 1);
			memcpy(&p[start_addr*2], buf, size);
		}
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_UINT32_ALIGN:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -WRITE_ERROR;

		if (start_addr*4+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(unsigned int)) {
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(unsigned int), 1);
			memcpy(&p[start_addr*4], buf, size);
		}
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_SINT16_ALIGN:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -WRITE_ERROR;

		if (start_addr*2+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(short)) {
			fwif_color_ChangeINT16Endian((short *)buf, size/sizeof(short), 1);
			memcpy(&p[start_addr*2], buf, size);
		}
	}
	break;

	case TOOLACCESS_PQ_SoLib_ShareMem_SINT32_ALIGN:
	{
		unsigned char *p = Scaler_Get_SoPQLib_ShareMem();

		if (p == NULL)
			return -WRITE_ERROR;

		if (start_addr*4+size > Scaler_Get_SoPQLib_ShareMem_Size())
			return -ID_SIZE_ERR;

		if (size >= sizeof(int)) {
			fwif_color_ChangeINT32Endian((int *)buf, size/sizeof(int), 1);
			memcpy(&p[start_addr*4], buf, size);
		}
	}
	break;
#endif

	case TOOLACCESS_Function_MEMC_Performance_Checking_Database:
	{
		if(size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_MEMC_Set_Performance_Checking_Database_index(*buf);

		break;
	}

/************************************************************/
/******************* For TV002 use*****************************/
/************************************************************/
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

	case TOOLACCESS_RADCR_BL_MAP_TBL:
	{
		extern RADCR_BL_MAP_ST RADCR_BL_Mapping_table;
		if (start_addr*sizeof(unsigned char)+size > sizeof(RADCR_BL_MAP_ST))
			return -ID_SIZE_ERR;

		if (size) {
			memcpy(RADCR_BL_Mapping_table.bl_mapping+start_addr, buf, size);
		}

		break;
	}

	default:
		return -ID_SIZE_ERR;
		break;
	}
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
	_clues *smartPic_clue = fwif_color_Get_SmartPic_clue();
	_system_setting_info *system_setting_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	StructColorDataType *pTable = NULL;
	SLR_VIP_TABLE *shareMem_VIP_Table = NULL;

	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	shareMem_VIP_Table = fwif_color_GetShare_Memory_VIP_TABLE_Struct();

	if (pTable == NULL || shareMem_VIP_Table == NULL)
		return -ID_SIZE_ERR;

	which_table = pTable->PQA_Table;
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
			memcpy((unsigned char *)PQA_table, buf, size);

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
			memcpy(buf, (unsigned char *)PQA_table, size);
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
			memcpy(buf, (unsigned char *) &smartPic_clue->PQA_ISR_Cal_Info.Input_Level[0], size);
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
			memcpy(buf, (unsigned char *) &smartPic_clue->PQA_ISR_Cal_Info.Level_Index[0], size);
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(int), 1);
			*(num_type) = VIP_UINT32;
		}
	} else if (mode == TOOLACCESS_PQA_Input_Table) {
		if (isSet_Flag == 1) {
			if (size%(sizeof(int)*PQA_I_LEVEL_MAX) != 0)
				return -ID_SIZE_ERR;
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(int), 1);
			memcpy((unsigned char *)PQA_input_table, buf, size);
		} else {
			/*get size must be smaller than table size in share memory*/
			if (size > PQA_I_ITEM_MAX*PQA_I_LEVEL_MAX*sizeof(int))
				return -ID_SIZE_ERR;
			memcpy(buf, (unsigned char *)PQA_input_table, size);
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(int), 1);
			*(num_type) = VIP_UINT32;
		}
	} else if (mode == TOOLACCESS_PQA_FlowCtrl_Input_Type) {
		if (isSet_Flag == 1) {
			if (size > sizeof(flowCtrl_Itype)) /*input item table and flag table*/
				return -ID_SIZE_ERR;
			memcpy((unsigned char*)&(flowCtrl_Itype[0][0]), buf,  size);
			memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type_setFlag[0]), (unsigned char*)&(flowCtrl_Itype[0][0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
			memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[0]), (unsigned char*)&(flowCtrl_Itype[1][0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
			memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
		} else {
			if (size > sizeof(flowCtrl_Itype)) /*input item table and flag table*/
				return -ID_SIZE_ERR;
			for (i = 0; i < PQA_INPUT_TYPE_MAX; i++)
				if (system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type_setFlag[i] == 0)	/* read Flow_Ctrl_Input_Type[i], while set flag is on, avoid reading error, elieli*/
					system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[i] = system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[i];
			/*memcpy((unsigned char *) &system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[0], (unsigned char *) &system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[0], sizeof(char)*PQA_INPUT_TYPE_MAX);*/
			memcpy((unsigned char*)&(flowCtrl_Itype[0][0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type_setFlag[0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
			memcpy((unsigned char*)&(flowCtrl_Itype[1][0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
			memcpy(buf, (unsigned char *) &(flowCtrl_Itype[0][0]),  size);
			*(num_type) = VIP_UINT8;
		}
	} else if (mode == TOOLACCESS_PQA_FlowCtrl_Input_Item) {
		if (isSet_Flag == 1) {
			if (size > sizeof(flowCtrl_Iitem)) /*input item table and flag table*/
				return -ID_SIZE_ERR;
			memcpy((unsigned char*)&(flowCtrl_Iitem[0][0]), buf,  size);
			memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item_setFlag[0]), (unsigned char*)&(flowCtrl_Iitem[0][0]), sizeof(char)*PQA_INPUT_ITEM_MAX);
			memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item[0]), (unsigned char*)&(flowCtrl_Iitem[1][0]), sizeof(char)*PQA_INPUT_ITEM_MAX);
			for (i = 0; i < PQA_INPUT_ITEM_MAX; i++)
				smartPic_clue->PQA_ISR_Cal_Info.Input_Level[i] = (UINT16)system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item[i];
		} else {
			if (size > sizeof(flowCtrl_Iitem)) /*input item table and flag table*/
				return -ID_SIZE_ERR;
			for (i = 0; i < PQA_INPUT_ITEM_MAX; i++)
				if (system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item_setFlag[i] == 0)	/* read Flow_Ctrl_Input_Item[i], while set flag is on, avoid reading error, elieli*/
					system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item[i] = (UINT16)smartPic_clue->PQA_ISR_Cal_Info.Input_Level[i];
			memcpy((unsigned char*)&(flowCtrl_Iitem[0][0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item_setFlag[0]), sizeof(char)*PQA_INPUT_ITEM_MAX);
			memcpy((unsigned char*)&(flowCtrl_Iitem[1][0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item[0]), sizeof(char)*PQA_INPUT_ITEM_MAX);
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
			fwif_color_ChangeUINT16Endian_Copy((unsigned short *)&system_setting_info->OSD_Info.Saturation_Gain, 1, (unsigned short *)buf, 1);
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
			fwif_color_ChangeUINT16Endian_Copy((unsigned short *)&system_setting_info->OSD_Info.Contrast_Gain, 1, (unsigned short *)buf, 1);
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

	if (vpq_project_id == 0x00060000 && vpqex_project_id == 0x00060000)
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

	if (vpq_project_id == 0x00060000 && vpqex_project_id == 0x00060000)
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

	if (vpq_project_id == 0x00060000 && vpqex_project_id == 0x00060000)
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

	if (vpq_project_id == 0x00060000 && vpqex_project_id == 0x00060000)
		fwif_color_setCon_Bri_Color_Tint_tv006(SLR_MAIN_DISPLAY, system_setting_info);	/*only main ch for tool now*/
	else
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(0, SLR_MAIN_DISPLAY, system_setting_info);
	return 0;
}

void rtice_Covert_ICM_Gain_Struct(CHIP_COLOR_ICM_GAIN_ADJUST_T *pDB, CHIP_COLOR_ICM_GAIN_ADJUST_RTICE_T *pRTICE, unsigned char flag)
{
	int rgn, pt;

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

int rtice_Get_VIP_System_Info(_system_setting_info *system_setting_info, _RPC_system_setting_info *RPC_system_setting_info, _RPC_clues *RPC_smartPic_clue, SLR_VIP_TABLE *gVip_Table, unsigned int* pVIPSysInfo)
{
	unsigned char i;
	int decode_bit_rate;
	int inputplugin_bit_rate;
	int vo_bit_rate;
	if (system_setting_info == NULL || RPC_system_setting_info == NULL) {
		for (i=0;i<VIP_System_Info_Items_Num;i++)
			pVIPSysInfo[i] = 0;
	} else {
		decode_bit_rate = Scaler_ChangeUINT32Endian(RPC_smartPic_clue->VCPU_setting_info.decode_bit_rate);
		inputplugin_bit_rate = Scaler_ChangeUINT32Endian(RPC_smartPic_clue->VCPU_setting_info.inputplugin_bit_rate);

		if((decode_bit_rate==0) && (inputplugin_bit_rate==0))
			vo_bit_rate = 0;
		else if (inputplugin_bit_rate == 0)
			vo_bit_rate = decode_bit_rate;
		else
			vo_bit_rate = inputplugin_bit_rate;

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

		pVIPSysInfo[16] = vo_bit_rate;

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
		pVOInfo[31] = info->color_transf;
		pVOInfo[32] = info->coeffs_overscan;
		pVOInfo[33] = info->dispXY0;
		pVOInfo[34] = info->dispXY1;
		pVOInfo[35] = info->dispXY2;
		pVOInfo[36] = info->whitePointXY;
		pVOInfo[37] = info->maxL;
		pVOInfo[38] = info->minL;
		//pVOInfo[39] = info->DolbyVision;	//SLR_VOINFO has no this item
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
//	unsigned int *pBuff;
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

	if (New_idx>=ICM_TBL_Buff_Num)
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
	char which_idx, which_buff;//fix tjw
	unsigned char idx;
	unsigned int size, isbufNull_flag;
	int g_h_sta = 0,  g_h_end = 0,  g_s_sta = 0,  g_s_end = 0,  g_i_sta = 0,  g_i_end = 0;
	unsigned int* pBuff_use = NULL;

	size = rtice_param.size;

	/* mode0 = 1, use pre buff, mode0 = 0, use cur buff*/
	which_buff = rtice_param.mode0;
	which_idx = New_idx - which_buff;
	if (which_idx < 0)
		idx = which_idx + ICM_TBL_Buff_Num;
	else
		idx = which_idx;

	pBuff_use = tICM_buff[idx];

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
	extern unsigned char VIP_DM2_Sat__Hue_Hist_Skip;
	unsigned int* pwUINT32Temp;
	unsigned short* pwUINT16Temp;
	unsigned char* pwUINT8Temp;
	short* pwINT16Temp;
	char ret = 0;
	unsigned int size;
	unsigned char dummy = 0;

	size = rtice_param.size;

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

		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_Hist_Auto_Mode) {
			if (size > (sizeof(unsigned char) * 2))
				return -ID_SIZE_ERR;
			fwif_color_set_DM2_Hist_AutoMode_Enable_Flag(buf[0]);
			fwif_color_set_DM2_Hist_AutoMode_Counter(buf[1]);
		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_Sat_Hue_Hist_Skip) {
			if (size > sizeof(unsigned char))
				return -ID_SIZE_ERR;
			VIP_DM2_Sat__Hue_Hist_Skip = buf[0];
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

		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_Tone_Mapping_TBL) {
			dummy++;
			if (size > (sizeof(unsigned short) * (ToneMapping_size * 3)))
				ret = -1;
			dummy++;
			pwINT16Temp = (unsigned short *)vip_malloc((ToneMapping_size * 3) * sizeof(short));
			if (pwINT16Temp == NULL)
				ret = -2;
			dummy++;
			if (pwINT16Temp != NULL && (size == (sizeof(short) * (ToneMapping_size * 3))) && ret >= 0) {
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

		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_Hist_RGB_max) {
			if (size > (sizeof(unsigned int) * (HDR_Histogram_size + 3)))	// histogram size = 128, RGB max = 3
				return -ID_SIZE_ERR;
			pwUINT32Temp = (unsigned int *)vip_malloc((HDR_Histogram_size + 3) * sizeof(int));
			if (pwUINT32Temp == NULL)
				return -ID_SIZE_ERR;

			ret = fwif_color_get_DM2_HDR_histogram_TV006(pwUINT32Temp);
			fwif_color_ChangeUINT32Endian(pwUINT32Temp, (HDR_Histogram_size + 3), 1);
			memcpy(buf, (char*) pwUINT32Temp, size);

			*num_type = VIP_UINT32;
			vip_free(pwUINT32Temp);

		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_Hist_Auto_Mode) {
			if (size > (sizeof(unsigned char) * 2))
				return -ID_SIZE_ERR;
			pwUINT8Temp = (unsigned char *)vip_malloc(2 * sizeof(char));
			if (pwUINT8Temp == NULL)
				return -ID_SIZE_ERR;

			pwUINT8Temp[0] = fwif_color_get_DM2_Hist_AutoMode_Enable_Flag();
			pwUINT8Temp[1] = fwif_color_get_DM2_Hist_AutoMode_Counter();

			memcpy(buf, pwUINT8Temp, size);

			*num_type = VIP_UINT8;
                        vip_free(pwUINT8Temp);

		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_Hist_Auto_Mode_TBL) {
			if (size > (sizeof(int) * DM2_Hist_MODE_MAX * HDR_Histogram_size))
				return -ID_SIZE_ERR;

			pwUINT32Temp = fwif_color_DM2_Hist_AutoMode_TBL_Get(0);

			memcpy(buf, (unsigned int*)pwUINT32Temp, size);
			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(int), 1);

			*num_type = VIP_UINT32;
		}  else if (rtice_param.id == TOOLACCESS_DM2_HDR_Sat_Hist_TBL) {
			if (size > (sizeof(int) * HDR_Sat_Histogram_size))
				return -ID_SIZE_ERR;

			pwUINT32Temp = (unsigned int *)vip_malloc(HDR_Sat_Histogram_size * sizeof(int));
			if (pwUINT32Temp == NULL)
				return -ID_SIZE_ERR;

			fwif_color_DM2_Sat_Hist_TBL_Get(pwUINT32Temp);
			
			memcpy(buf, pwUINT32Temp, size);
			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(int), 1);
			vip_free(pwUINT32Temp);

			*num_type = VIP_UINT32;

		}  else if (rtice_param.id == TOOLACCESS_DM2_HDR_Hue_Hist_TBL) {
			if (size > (sizeof(int) * HDR_Hue_Histogram_size))
				return -ID_SIZE_ERR;

			pwUINT32Temp = (unsigned int *)vip_malloc(HDR_Hue_Histogram_size * sizeof(int));
			if (pwUINT32Temp == NULL)
				return -ID_SIZE_ERR;

			fwif_color_DM2_Hue_Hist_TBL_Get(pwUINT32Temp);
			
			memcpy(buf, pwUINT32Temp, size);
			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(int), 1);
			vip_free(pwUINT32Temp);

			*num_type = VIP_UINT32;

		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_Sat_Hue_Hist_Skip) {
			if (size > sizeof(unsigned char))
				return -ID_SIZE_ERR;
			buf[0] = VIP_DM2_Sat__Hue_Hist_Skip;
			*num_type = VIP_UINT8;
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

DRV_DeMura_DMA_TBL DMA_TBL_Buff;
#define INX_SPI_ADDR 0	//0x2480
#define INX_SPI_SIZE (0x74720 + 0x2480)
unsigned char INNX_Demura_TBL[INX_SPI_SIZE] = {0};

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
	unsigned int sizeDemuraTbl_aligned;
	unsigned int  *vir_addr_aligned = NULL;
#ifdef CONFIG_ARM64 //ARM32 compatible
	unsigned long va_temp;
#else
	unsigned int va_temp;
#endif
	static short *pINNX_Demura_DeLut = NULL;

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
			fwif_color_DeMura_init(1, 0);

		} else if (rtice_param->id == TOOLACCESS_VIP_DeMura_Adaptive) {
			if (rtice_param->size > adaptiveTBL_Size || rtice_param->saddr + rtice_param->size > adaptiveTBL_Size) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura adative error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}

			memcpy(&DeMura_TBL.DeMura_Adaptive_TBL + rtice_param->saddr, buf, rtice_param->size);

		} else if (rtice_param->id == TOOLACCESS_VIP_DeMura_TBL_Mode) {
			if (rtice_param->size > sizeof(char)) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura TOOLACCESS_VIP_DeMura_TBL_Mode error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}

			fwif_color_set_DeMura_TBL_Mode(*buf);
			fwif_color_Get_DeMura_CTRL(&ori_ctrl_TBL);
			memcpy((unsigned char*)&DeMura_TBL.DeMura_CTRL_TBL, (unsigned char*)&ori_ctrl_TBL, sizeof(DRV_DeMura_CTRL_TBL));
			fwif_color_DeMura_init(1, 0);

		} else if (rtice_param->id == TOOLACCESS_VIP_DeMura_INI) {
			if (rtice_param->size > sizeof(char)) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura TOOLACCESS_VIP_DeMura_INI error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}

			fwif_color_DeMura_init(1, 0);

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
			fwif_color_DeMura_init(1, 0);

		} else if (rtice_param->id == TOOLACCESS_VIP_DeMura_READ_INX_DATA) {
			if (rtice_param->size > sizeof(char)) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura TOOLACCESS_VIP_DeMura_READ_INX_DATA error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			} else {
				pINNX_Demura_DeLut = (short *)dvr_malloc_specific(INNX_Demura_DecodeLUT_Size * sizeof(short), GFP_DCU2_FIRST);
				if (pINNX_Demura_DeLut == NULL) {
					return -1;
				} else {
					extern void vpq_INNX_Demura_data_to_dma(VIP_DeMura_TBL * Demura_TBL, char *pDeLut);
					extern void Scaler_color_write_data_to_bin_file(unsigned char *data, unsigned int size, unsigned char* ouputfilename);
					extern void Scaler_color_read_data_from_bin_file(unsigned char *data, unsigned int size, unsigned char* sourcefile);
					unsigned int ret;
					//Scaler_color_write_data_to_bin_file(INNX_Demura_TBL, INX_SPI_SIZE, "/tmp/demura.bin");
					if (buf[0] == 222) Scaler_color_read_data_from_bin_file(INNX_Demura_TBL, INX_SPI_SIZE, "/tmp/d.bin");
					ret = fwif_color_INNX_Demura_TBL(INNX_Demura_TBL, INX_SPI_SIZE, pINNX_Demura_DeLut);

					if (ret != 0) {
						dvr_free((void *)pINNX_Demura_DeLut);
						return -1;
					}

					//vpq_INNX_Demura_data_to_dma(&DeMura_TBL, pINNX_Demura_DeLut);
					#if 1
					fwif_color_set_INX_Demura_ctrl(&DeMura_TBL);
					fwif_color_DeMura_encode_all_para(pINNX_Demura_DeLut, DeMura_TBL_481x271, 0, 5, &DeMura_TBL, INNX_Demura_W16);
					#else
					fwif_color_DeMura_encode_diff_input_line_size((pINNX_Demura_DeLut + INNX_Demura_DecodeLUT_Low_sta_addr),
																	(pINNX_Demura_DeLut + INNX_Demura_DecodeLUT_Mid_sta_addr),
																	(pINNX_Demura_DeLut + INNX_Demura_DecodeLUT_Mid2_sta_addr),
																	(pINNX_Demura_DeLut + INNX_Demura_DecodeLUT_Mid3_sta_addr),
																	(pINNX_Demura_DeLut + INNX_Demura_DecodeLUT_High_sta_addr),
																	DeMura_TBL_481x271,
																	0,
																	&DeMura_TBL,
																	INNX_Demura_W16);
					#endif

					ret = fwif_color_DeMura_init(1, 0);
					if (ret != 0) {
						dvr_free((void *)pINNX_Demura_DeLut);
						return -1;
					}

				}

				dvr_free((void *)pINNX_Demura_DeLut);
			}
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

			switch( DeMura_TBL.DeMura_CTRL_TBL.demura_table_level )
			{
				case 0: // 3 planes
					if (DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate == DeMura_Ch_Y_Mode)
						tbl_line_size = VIP_DemuraTBL_LineSize_Y_481x271_3Plane;
					else // RGB mode
						tbl_line_size = VIP_DemuraTBL_LineSize_RGB_481x271_3Plane;
					break;
				case 1: // 4 planes
					if (DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate == DeMura_Ch_Y_Mode)
						tbl_line_size = VIP_DemuraTBL_LineSize_Y_481x271_4Plane;
					else // RGB mode
						tbl_line_size = VIP_DemuraTBL_LineSize_RGB_481x271_4Plane;
					break;
				case 2: // 5 planes
					if (DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate == DeMura_Ch_Y_Mode)
						tbl_line_size = VIP_DemuraTBL_LineSize_Y_481x271_5Plane;
					else // RGB mode
						tbl_line_size = VIP_DemuraTBL_LineSize_RGB_481x271_5Plane;
					break;
				default:
					rtd_pr_vpq_emerg("[%s] rtice_SetGet_DeMura plane number invalid, table level = %d\n", __func__, DeMura_TBL.DeMura_CTRL_TBL.demura_table_level);
					vip_free(pDemura_TBL_tmp);
					return -1;
			}

			vir_addr = dvr_remap_uncached_memory(base_addr, sizeDemuraTbl, __builtin_return_address(0));

			sizeDemuraTbl_aligned = sizeDemuraTbl;
#ifdef CONFIG_ARM64 //ARM32 compatible
			va_temp = (unsigned long)vir_addr;
#else
			va_temp = (unsigned int)vir_addr;
#endif
			vir_addr_aligned = (unsigned int*)va_temp;

#ifdef CONFIG_BW_96B_ALIGNED
			sizeDemuraTbl_aligned = dvr_size_alignment(sizeDemuraTbl);
#ifdef CONFIG_ARM64 //ARM32 compatible
			va_temp = (unsigned long)vir_addr;
#else //CONFIG_ARM64
			va_temp = (unsigned int)vir_addr;
#endif //CONFIG_ARM64
			vir_addr_aligned = (unsigned int*)dvr_memory_alignment((unsigned long)va_temp, sizeDemuraTbl_aligned);
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

		} else if (rtice_param->id == TOOLACCESS_VIP_DeMura_READ_INX_DATA) {
			if (rtice_param->size > sizeof(char)) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura TOOLACCESS_VIP_DeMura_READ_INX_DATA error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			} else {
				extern int spi_read_internal(unsigned int type, unsigned long offset, unsigned int count, void *buf);
				//unsigned char i;
				static unsigned char first_read = 1;

				buf[0] = 255; /* init */
				if (first_read) {
					first_read = 0;
					buf[0] = spi_read_internal( 1, INX_SPI_ADDR, INX_SPI_SIZE, INNX_Demura_TBL);
				} else {
					if (INNX_Demura_TBL[0] == 0x99) buf[0] = 0;
				}

				//for (i = 0; i < 50; i++)
					//rtd_pr_vpq_emerg("roger, SPI[%d] = [0x%x]\n", i, INNX_Demura_TBL[i]);
			}
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
/************************************************************/
/******************* For TV002 use*****************************/
/************************************************************/
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
	unsigned int VIP_DBC_LUT_SIZE = sizeof(char) * DBC_LUT_APL_MAX * DBC_LUT_DEV_MAX;
	unsigned int VIP_DBC_DCC_S_Hifh_CMPS_LUT_SIZE = sizeof(char) * DBC_LUT_APL_MAX * DBC_LUT_DEV_MAX;
	unsigned int VIP_DBC_DCC_S_Index_CMPS_LUT_SIZE = sizeof(char) * DBC_LUT_APL_MAX * DBC_LUT_DEV_MAX;

	SLR_VIP_TABLE_CUSTOM_TV002* custom_TBL_TV002 = Scaler_get_tv002_Custom_struct();
	unsigned char DCC_S_High_CMPS_TBL_SEL = custom_TBL_TV002->DBC.DBC_STATUS.OSD_PowerSave;

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

		} else if ( id== TOOLACCESS_DBC_LUT_TV002) {
			if (size > VIP_DBC_LUT_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = &custom_TBL_TV002->DBC.DBC_LUT_TBL.TBL[0][0];
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(pwUINT8Temp, buf, size);

		} else if ( id== TOOLACCESS_DBC_DCC_S_High_CMPS_LUT_TV002) {
			if (size > VIP_DBC_DCC_S_Hifh_CMPS_LUT_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = &custom_TBL_TV002->DBC.DBC_LUT_TBL.DCC_S_High_CMPS_TBL[DCC_S_High_CMPS_TBL_SEL][0][0];
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(pwUINT8Temp, buf, size);

		} else if ( id== TOOLACCESS_DBC_DCC_S_Index_CMPS_LUT_TV002) {
			if (size > VIP_DBC_DCC_S_Index_CMPS_LUT_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = &custom_TBL_TV002->DBC.DBC_LUT_TBL.DCC_S_Index_CMPS_TBL[DCC_S_High_CMPS_TBL_SEL][0][0];
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

		} else if ( id== TOOLACCESS_DBC_LUT_TV002) {
			if (size > VIP_DBC_LUT_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = &custom_TBL_TV002->DBC.DBC_LUT_TBL.TBL[0][0];
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(buf, pwUINT8Temp, size);
			*num_type = VIP_UINT8;

		} else if ( id== TOOLACCESS_DBC_DCC_S_High_CMPS_LUT_TV002) {
			if (size > VIP_DBC_DCC_S_Hifh_CMPS_LUT_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = &custom_TBL_TV002->DBC.DBC_LUT_TBL.DCC_S_High_CMPS_TBL[DCC_S_High_CMPS_TBL_SEL][0][0];
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(buf, pwUINT8Temp, size);
			*num_type = VIP_UINT8;
		} else if ( id== TOOLACCESS_DBC_DCC_S_Index_CMPS_LUT_TV002) {
			if (size > VIP_DBC_DCC_S_Index_CMPS_LUT_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = &custom_TBL_TV002->DBC.DBC_LUT_TBL.DCC_S_Index_CMPS_TBL[DCC_S_High_CMPS_TBL_SEL][0][0];
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(buf, pwUINT8Temp, size);
			*num_type = VIP_UINT8;
		} else {
			return -ID_SIZE_ERR;
		}
	}
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

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);
			memcpy(pwUINT16Temp, buf, size);
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

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);
			memcpy(pwUINT16Temp, buf, size);
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
/************************************************************/
/******************* For TV002 use*****************************/
/************************************************************/

int rtice_set_Function_Coef(ap_data_param rtice_param, unsigned char *buf, unsigned char isSet_Flag)
{

	if(isSet_Flag == 1){
		switch (rtice_param.mode1) {
			case  VIP_QUALITY_FUNCTION_DLTi:// [0]
					Scaler_SetDLti(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_DCTi:// [1]
					Scaler_SetDCti(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_I_DLTi:// [2]
					Scaler_SetIDLti(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_I_DCTi:// [3]
					Scaler_SetIDCti(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_V_DCTi:// [4]
					Scaler_SetVDCti(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_UV_DELAY_ENABLE:// [5]
					Scaler_SetColor_UV_Delay_Enable(buf[0]) ;
				break;
			case  VIP_QUALITY_FUNCTION_UV_DELAY:
					Scaler_SetColor_UV_Delay(buf[0]);// [6]
				break;
			case  VIP_QUALITY_FUNCTION_YUV2RGB:// [7]
					Scaler_set_YUV2RGB(buf[0], 0, 0);
				break;
			case  VIP_QUALITY_FUNCTION_Film:// [8]
					Scaler_SetFilm_Mode(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_Intra:// [9]
					Scaler_SetIntra(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_MA:// [10]
					Scaler_SetMADI(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_tnr_xc:// [11]
					Scaler_SetTNRXC(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_MA_Chroma_Error:// [12]
					//Scaler_SetMA_Chroma_Error(buf[0]);
				break;
			//[13] not defined
			case  VIP_QUALITY_FUNCTION_MADI_HME:// [14]
					Scaler_SetMADI_HME(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_MADI_HMC:// [15]
					Scaler_SetMADI_HMC(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_MADI_PAN:// [16]
					Scaler_SetMADI_PAN(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_DI_SMD_Level:// [17]
					Scaler_SetDiSmd(buf[0]);
				break;
			case  VIP_QUALITY_FUNCTION_COLOR_NEW_UVC:// [18]
					Scaler_SetNew_UVC(buf[0]);
				break;
			case VIP_QUALITY_FUNCTION_UV_DELAY_TOP:// [19]
				//not used in vip handle!
				break;
			case  VIP_QUALITY_FUNCTION_V_DCTi_LPF:// [20]
				Scaler_SetVDCti_LPF(buf[0]);
				break;

			default:
				return -WRITE_ERROR;
			break;

		}
	} else {
		switch (rtice_param.mode1) {
			case  VIP_QUALITY_FUNCTION_DLTi:// [0]
					buf[0] = Scaler_GetDLti();
				break;
			case  VIP_QUALITY_FUNCTION_DCTi:// [1]
					buf[0] = Scaler_GetDCti();
				break;
			case  VIP_QUALITY_FUNCTION_I_DLTi:// [2]
					buf[0] = Scaler_GetIDLti();
				break;
			case  VIP_QUALITY_FUNCTION_I_DCTi:// [3]
					buf[0] = Scaler_GetIDCti();
				break;
			case  VIP_QUALITY_FUNCTION_V_DCTi:// [4]
					buf[0] = Scaler_GetVDCti();
				break;
			case  VIP_QUALITY_FUNCTION_UV_DELAY_ENABLE:// [5]
					buf[0] = Scaler_GetColor_UV_Delay_Enable() ;
				break;
			case  VIP_QUALITY_FUNCTION_UV_DELAY:
					buf[0] = Scaler_GetColor_UV_Delay();// [6]
				break;
			case  VIP_QUALITY_FUNCTION_YUV2RGB:// [7]
					buf[0] = Scaler_get_YUV2RGB();
				break;
			case  VIP_QUALITY_FUNCTION_Film:// [8]
					buf[0] = Scaler_GetFilm_Mode();
				break;
			case  VIP_QUALITY_FUNCTION_Intra:// [9]
					buf[0] = Scaler_GetIntra();
				break;
			case  VIP_QUALITY_FUNCTION_MA:// [10]
					buf[0] = Scaler_GetMADI();
				break;
			case  VIP_QUALITY_FUNCTION_tnr_xc:// [11]
					buf[0] = Scaler_GetTNRXC_Ctrl();
				break;
			case  VIP_QUALITY_FUNCTION_MA_Chroma_Error:// [12]
					//buf[0] = Scaler_GetMA_Chroma_Error();
				break;
			//[13] not defined
			case  VIP_QUALITY_FUNCTION_MADI_HME:// [14]
					buf[0] = Scaler_GetMADI_HME();
				break;
			case  VIP_QUALITY_FUNCTION_MADI_HMC:// [15]
					buf[0] = Scaler_GetMADI_HMC();
				break;
			case  VIP_QUALITY_FUNCTION_MADI_PAN:// [16]
					buf[0] = Scaler_GetMADI_PAN();
				break;
			case  VIP_QUALITY_FUNCTION_DI_SMD_Level:// [17]
					buf[0] = Scaler_GetDiSmd();
				break;
			case  VIP_QUALITY_FUNCTION_COLOR_NEW_UVC:// [18]
					buf[0] = Scaler_GetNew_UVC();
				break;
			case VIP_QUALITY_FUNCTION_UV_DELAY_TOP:// [19]
				buf[0] = 255;
				//not used in vip handle!
				break;
			case  VIP_QUALITY_FUNCTION_V_DCTi_LPF:// [20]
					buf[0] = Scaler_GetVDCti_LPF();
				break;

			default:
				buf[0] = 255;
				return -WRITE_ERROR;
			break;
		}
	}

	return 0;
}

int rtice_set_Function_Extend_Coef(ap_data_param rtice_param, unsigned char *buf, unsigned char isSet_Flag)
{
	if(isSet_Flag == 1){
		switch (rtice_param.mode1) {
			case VIP_QUALITY_FUNCTION_CDS_Table://[0]
				Scaler_SetCDSTable(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_EMFMk2://[1]
				Scaler_SetEMF_Mk2(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_SKIPIR_Dering://[2]
				//not used in vip handle
			break;
			case VIP_QUALITY_FUNCTION_SKIPIR_dnoise://[3]
				//not used in vip handle
			break;
			case VIP_QUALITY_FUNCTION_SKIPIR_dnoise_table://[4]
				//not used in vip handle
			break;
			case VIP_QUALITY_FUNCTION_ADAPTIVE_GAMMA://[5]
				//not used in vip handle
			break;
		#ifdef LocalDimming
			case VIP_QUALITY_FUNCTION_LD_Enable://[6]
				Scaler_SetLocalDimmingEnable(buf[0]);
			break;

			case VIP_QUALITY_FUNCTION_LD_Table://[7]
				Scaler_SetLocalDimmingTable(buf[0]);
			break;
		#endif
			case VIP_QUALITY_FUNCTION_LC_Enable://[8]
				Scaler_SetLocalContrastEnable(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_LC_Table://[9]
				Scaler_SetLocalContrastTable(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_3dLUT_Table://[10]
				Scaler_Set3dLUT(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_I_De_XC://[11]
				Scaler_Set_I_De_XC(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_I_De_Contour://[12]
				Scaler_Set_I_De_Contour(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_SLD://[13]
				//Scaler_Set_SLD(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_Output_InvOutput_GAMMA://[14]
				Scaler_color_colorwrite_Output_InvOutput_gamma(buf[0]);
			break;
			default:
				return -WRITE_ERROR;
			break;
		}

	}else{

		switch (rtice_param.mode1) {
			case VIP_QUALITY_FUNCTION_CDS_Table://[0]
				buf[0] = Scaler_GetCDSTable();
			break;
			case VIP_QUALITY_FUNCTION_EMFMk2://[1]
				buf[0] = Scaler_GetEMF_Mk2();
			break;
			case VIP_QUALITY_FUNCTION_SKIPIR_Dering://[2]
				buf[0] = 255;
				//not used in vip handle
			break;
			case VIP_QUALITY_FUNCTION_SKIPIR_dnoise://[3]
				buf[0] = 255;
				//not used in vip handle
			break;
			case VIP_QUALITY_FUNCTION_SKIPIR_dnoise_table://[4]
				buf[0] = 255;
				//not used in vip handle
			break;
			case VIP_QUALITY_FUNCTION_ADAPTIVE_GAMMA://[5]
				buf[0] = 255;
				//not used in vip handle
			break;
		#ifdef LocalDimming
			case VIP_QUALITY_FUNCTION_LD_Enable://[6]
				buf[0] = Scaler_GetLocalDimmingEnable();
			break;

			case VIP_QUALITY_FUNCTION_LD_Table://[7]
				buf[0] = Scaler_GetLocalDimmingTable();
			break;
		#endif
			case VIP_QUALITY_FUNCTION_LC_Enable://[8]
				buf[0] = Scaler_GetLocalContrastTable();
			break;
			case VIP_QUALITY_FUNCTION_LC_Table://[9]
				buf[0] = Scaler_GetLocalContrastEnable();
			break;
			case VIP_QUALITY_FUNCTION_3dLUT_Table://[10]
				buf[0] = Scaler_Get3dLUT();
			break;
			case VIP_QUALITY_FUNCTION_I_De_XC://[11]
				buf[0] = Scaler_Get_I_De_XC();
			break;
			case VIP_QUALITY_FUNCTION_I_De_Contour://[12]
				buf[0] = Scaler_Get_I_De_Contour();
			break;
			case VIP_QUALITY_FUNCTION_SLD://[13]
				//buf[0] = Scaler_Get_SLD();
			break;
			case VIP_QUALITY_FUNCTION_Output_InvOutput_GAMMA://[14]
				buf[0] = Scaler_Get_Output_InvOutput_gamma();
			break;
			default:
				buf[0] = 255;
				return -WRITE_ERROR;
			break;
		}

	}
	return 0;

}

int rtice_set_Function_Extend2_Coef(ap_data_param rtice_param, unsigned char *buf, unsigned char isSet_Flag)
{
	unsigned char HY,VY,HC,VC;

	if(isSet_Flag == 1){
		switch (rtice_param.mode1) {
			case VIP_QUALITY_FUNCTION_SU_H://[0]
				Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);
				Scaler_SetScaleUP_YCHV(buf[0],VY,HC,VC);
			break;
			case VIP_QUALITY_FUNCTION_SU_V://[1]
				Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);
				Scaler_SetScaleUP_YCHV(HY,buf[0],HC,VC);
			break;
			case VIP_QUALITY_FUNCTION_S_PK://[2]
				Scaler_SetScalePK(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_SUPK_MASK://[3]
				Scaler_SetSUPKType(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_UNSHARP_MASK://[4]
				Scaler_SetUnsharpMask(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_EGSM_Postshp_Table://[5]
				Scaler_setdESM(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_iESM_table://[6]
				Scaler_setiESM(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_TwoStep_SU_Table://[7]
				Scaler_SetScaleUP_TwoStep(buf[0]);
			break;
#ifdef ENABLE_SUPER_RESOLUTION
		case VIP_QUALITY_FUNCTION_SuperResolution_edge_gain://[8]
				Scaler_SetSR_edge_gain(buf[0]);//not support!!no driver!!
			break;
			case VIP_QUALITY_FUNCTION_SNR_SuperResolution_texture_gain://[9]
				Scaler_SetSR_texture_gain(buf[0]);//not support!!no driver!!
			break;
#endif
			case VIP_QUALITY_FUNCTION_SD_H_Table://[10]
				Scaler_SetScaleDOWNHTable(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_SD_V_Table://[11]
				Scaler_SetScaleDOWNVTable(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_SD_444To422://[12]
				Scaler_SetScaleDOWN444To422(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_ColorSpace_Control://[13]
			//not used in vip handle
			break;
			case VIP_QUALITY_FUNCTION_SU_H_8TAP://[14]
				Scaler_SetScaleUPH_8tap(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_SU_V_6TAP://[15]
				Scaler_SetScaleUPV_6tap(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_OSD_SHP://[16]
				Scaler_SetScaleOSD_Sharpness(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_SU_DIR://[17]
				Scaler_SetScaleUPDir(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_SU_DIR_Weighting://[18]
				Scaler_SetScaleUPDir_Weighting(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_SU_C_H://[19]
				Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);
				Scaler_SetScaleUP_YCHV(HY,VY,buf[0],VC);
			break;
			case VIP_QUALITY_FUNCTION_SU_C_V://[20]
				Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);
				Scaler_SetScaleUP_YCHV(HY,VY,HC,buf[0]);
			break;

			default:
				return -WRITE_ERROR;
			break;
		}
	}else{
		switch (rtice_param.mode1) {
			case VIP_QUALITY_FUNCTION_SU_H://[0]
				Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);
				buf[0] = HY;
			break;
			case VIP_QUALITY_FUNCTION_SU_V://[1]
				Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);
				buf[0] = VY;
			break;
			case VIP_QUALITY_FUNCTION_S_PK://[2]
				buf[0] = Scaler_GetScalePK();
			break;
			case VIP_QUALITY_FUNCTION_SUPK_MASK://[3]
				buf[0] = Scaler_GetSUPKType();
			break;
			case VIP_QUALITY_FUNCTION_UNSHARP_MASK://[4]
				buf[0] = Scaler_GetUnsharpMask();
			break;
			case VIP_QUALITY_FUNCTION_EGSM_Postshp_Table://[5]
				buf[0] = Scaler_getdESM();
			break;
			case VIP_QUALITY_FUNCTION_iESM_table://[6]
				buf[0] = Scaler_getiESM();
			break;
			case VIP_QUALITY_FUNCTION_TwoStep_SU_Table://[7]
				buf[0] = Scaler_GetScaleUP_TwoStep();
			break;
#ifdef ENABLE_SUPER_RESOLUTION
			case VIP_QUALITY_FUNCTION_SuperResolution_edge_gain://[8]
				buf[0] = 	Scaler_GetSR_edge_gain();
			break;
			case VIP_QUALITY_FUNCTION_SNR_SuperResolution_texture_gain://[9]
				buf[0] =	Scaler_GetSR_texture_gain();
			break;
#endif
			case VIP_QUALITY_FUNCTION_SD_H_Table://[10]
				buf[0] = Scaler_GetScaleDOWNHTable();
			break;
			case VIP_QUALITY_FUNCTION_SD_V_Table://[11]
				buf[0] = Scaler_GetScaleDOWNVTable();
			break;
			case VIP_QUALITY_FUNCTION_SD_444To422://[12]
				buf[0] = Scaler_GetScaleDOWN444To422();
			break;
			case VIP_QUALITY_FUNCTION_ColorSpace_Control://[13]
				buf[0] = 255;
				//not used in vip handle
			break;
			case VIP_QUALITY_FUNCTION_SU_H_8TAP://[14]
				buf[0] = Scaler_GetScaleUPH_8tap();
			break;
			case VIP_QUALITY_FUNCTION_SU_V_6TAP://[15]
				buf[0] = Scaler_GetScaleUPV_6tap();
			break;
			case VIP_QUALITY_FUNCTION_OSD_SHP://[16]
				buf[0] = Scaler_GetScaleOSD_Sharpness();
			break;
			case VIP_QUALITY_FUNCTION_SU_DIR://[17]
				buf[0] = Scaler_GetScaleUPDir();
			break;
			case VIP_QUALITY_FUNCTION_SU_DIR_Weighting://[18]
				buf[0] = Scaler_GetScaleUPDir_Weighting();
			break;
			case VIP_QUALITY_FUNCTION_SU_C_H://[19]
				Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);
				buf[0] = HC;
			break;
			case VIP_QUALITY_FUNCTION_SU_C_V://[20]
				Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);
				buf[0] = VC;
			break;

			default:
				buf[0] = 255;
				return -WRITE_ERROR;
			break;
		}
	}
	return 0;

}

int rtice_set_Function_Extend3_Coef(ap_data_param rtice_param, unsigned char *buf, unsigned char isSet_Flag)
{

	if(isSet_Flag == 1){
		switch (rtice_param.mode1) {

			case VIP_QUALITY_FUNCTION_VD_ConBriHueSat_Table://[0]
				Scaler_module_vdc_SetConBriHueSat_Table(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_ICM://[1]
				Scaler_set_ICM_table(buf[1], 0);
			break;
			case VIP_QUALITY_FUNCTION_Gamma://[2]
				Scaler_SetGamma(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_S_Gamma_Index://[3]
				Scaler_SetSindex(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_S_Gamma_Low://[4]
				Scaler_SetSlow(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_S_Gamma_High://[5]
				Scaler_SetShigh(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_DCC_Table_Select://[6]
				Scaler_SetDCC_Table(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_DCC_Skin_Tone_Table_Select://[7]
				Scaler_Set_DCC_Color_Independent_Table(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_DCC_Chroma_comp_t_sel://[8]
				Scaler_Set_DCC_chroma_compensation_Table(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_SharpTable://[9]
				Scaler_SetSharpnessTable(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_PQA_Table://[10]
				scaler_set_PQA_table(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_PQA_Input_Table://[11]
				scaler_set_PQA_Input_table(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_MB_Peaking://[12]
				Scaler_SetMBPeaking(buf[12]);
			break;
			case VIP_QUALITY_FUNCTION_blue_stretch://[13]
				//Scaler_set_blue_stretch(buf[0]);
			break;
			case VIP_QUALITY_FUNCTION_HDR://[14]
			//not used in vip handle
			break;
			case VIP_QUALITY_FUNCTION_MB_SU_Peaking://[20]
				Scaler_SetMBSUPeaking(buf[0]);
			break;

			default:
				return -WRITE_ERROR;
			break;

		}
	}else{
		switch (rtice_param.mode1) {
			case VIP_QUALITY_FUNCTION_VD_ConBriHueSat_Table://[0]
				buf[0] = Scaler_module_vdc_GetConBriHueSat_Table();
			break;
			case VIP_QUALITY_FUNCTION_ICM://[1]
				buf[0] = Scaler_get_ICM_table();
			break;
			case VIP_QUALITY_FUNCTION_Gamma://[2]
				buf[0] = Scaler_GetGamma();
			break;
			case VIP_QUALITY_FUNCTION_S_Gamma_Index://[3]
				buf[0] = Scaler_GetSindex();
			break;
			case VIP_QUALITY_FUNCTION_S_Gamma_Low://[4]
				buf[0] = Scaler_GetSlow();
			break;
			case VIP_QUALITY_FUNCTION_S_Gamma_High://[5]
				buf[0] = Scaler_GetShigh();
			break;
			case VIP_QUALITY_FUNCTION_DCC_Table_Select://[6]
				buf[0] = Scaler_GetDCC_Table();
			break;
			case VIP_QUALITY_FUNCTION_DCC_Skin_Tone_Table_Select://[7]
				buf[0] = Scaler_Get_DCC_Color_Independent_Table();
			break;
			case VIP_QUALITY_FUNCTION_DCC_Chroma_comp_t_sel://[8]
				buf[0] = Scaler_Get_DCC_chroma_compensation_Table();
			break;
			case VIP_QUALITY_FUNCTION_SharpTable://[9]
				buf[0] = Scaler_GetSharpnessTable();
			break;
			case VIP_QUALITY_FUNCTION_PQA_Table://[10]
				buf[0] = scaler_get_PQA_table();
			break;
			case VIP_QUALITY_FUNCTION_PQA_Input_Table://[11]
				buf[0] = scaler_get_PQA_Input_table();
			break;
			case VIP_QUALITY_FUNCTION_MB_Peaking://[12]
				buf[0] = Scaler_GetMBPeaking();
			break;
			case VIP_QUALITY_FUNCTION_blue_stretch://[13]
				//buf[0] = Scaler_Get_blue_stretch();
			break;
			case VIP_QUALITY_FUNCTION_HDR://[14]
				buf[0] = 255;
				//not used in vip handle
			break;
			case VIP_QUALITY_FUNCTION_MB_SU_Peaking://[20]
				buf[0] = Scaler_GetMBSUPeaking();
			break;

			default:
				buf[0] = 255;
				return -WRITE_ERROR;
			break;
		}
	}
	return 0;

}

int  rtice_Set_D_LUT_9to17(unsigned int *array)
{
	unsigned int *D_3D_LUT_TMP;
	unsigned int i, j, k, index;
	index = 0;

	D_3D_LUT_TMP = dvr_malloc(sizeof(unsigned int)*VIP_D_3D_LUT_UI_TBL_SIZE);

	if (D_3D_LUT_TMP == NULL)
		return -1;

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

#ifdef VIP_SUPPORT_SIP_1_SPI_2
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

int SIP1_init_status = -255;
int rtice_Load_SIP_1_bin(void) {
	#define SIP1_BIN_PATH "/mnt/vendor/factory/SIP1.bin"
	unsigned char *buf = NULL;
	struct file* filp = NULL;
	unsigned long outfileOffset = 0;
	int size_ret = 0;
	static unsigned char SIP1_inited = 0;

	buf = (unsigned char *)vip_malloc(VIP_SIP_1_Size_default);

	if (buf == NULL)
		return -1;

	filp = file_open(SIP1_BIN_PATH, O_RDONLY, 0);
	if (filp == NULL) {
		vip_free(buf);
		return -2;
	}

	size_ret = file_read(filp, outfileOffset, buf, VIP_SIP_1_Size_default);

	if (SIP1_inited == 0) {
		if (Scaler_fwif_color_VPQ_SIP_init() != 0) {
			file_close(filp);
			vip_free(buf);
			return -3;
		}
	}

	if (size_ret == VIP_SIP_1_Size_default) {
		Scaler_fwif_color_set_VPQ_SIP_1_ISR_DMA(0, (unsigned int *)buf);
	}

	file_close(filp);
	vip_free(buf);
	return size_ret;
}

int rtice_Get_SIP_1_Enable(void) {
	if (!SYS_REG_SYS_CLKEN4_get_clken_d_s1_f2p_ssrs(IoReg_Read32(SYS_REG_SYS_CLKEN4_reg)) ||
		!SYS_REG_SYS_CLKEN4_get_clken_d_s1_f4p_ssrs(IoReg_Read32(SYS_REG_SYS_CLKEN4_reg)))
		return 0;
	else
		return (IoReg_Read32(SW_DEF_SIP1_4000_reg)&0x1);
}

void rtice_Set_SIP_1_Enable(int enable) {
	if (!SYS_REG_SYS_CLKEN4_get_clken_d_s1_f2p_ssrs(IoReg_Read32(SYS_REG_SYS_CLKEN4_reg)) ||
		!SYS_REG_SYS_CLKEN4_get_clken_d_s1_f4p_ssrs(IoReg_Read32(SYS_REG_SYS_CLKEN4_reg))) {
		if (enable) {
			SIP1_init_status = rtice_Load_SIP_1_bin();
			if (SIP1_init_status == VIP_SIP_1_Size_default) {
				IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL3_reg, 0x007b0833); //memc golden delay
				IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL2_reg, PPOVERLAY_Double_Buffer_CTRL2_memc_dtgreg_dbuf_set_mask);
			}
		}
	} else {
		if (enable) {
			if (enable == 2)
				SIP1_init_status = rtice_Load_SIP_1_bin();
			IoReg_SetBits(SW_DEF_SIP1_4000_reg, _BIT0);
		} else {
			IoReg_ClearBits(SW_DEF_SIP1_4000_reg, _BIT0);
		}
		IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL3_reg, 0x007b0833); //memc golden delay
		IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL2_reg, PPOVERLAY_Double_Buffer_CTRL2_memc_dtgreg_dbuf_set_mask);
	}
}
#endif
