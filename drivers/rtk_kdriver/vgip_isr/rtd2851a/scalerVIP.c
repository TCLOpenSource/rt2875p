/***************************************************************************
                          scalerVIP.c  -  description
                             -------------------
    begin                : Mon Dec 22 2008
    copyright           : (C) 2008 by hsliao
    email                : hsliao@realtek.com
 ***************************************************************************/
//oliver-
/*
#include <linux/version.h>	// kernel version

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	#ifndef __KERNEL__
		#define __KERNEL__
	#endif
#endif

#include <linux/module.h>	// normal module use
#include <linux/kernel.h>	// rtd_pr_vpq_isr_info()
*/

/*******************************************************************************
 * Header include
******************************************************************************/
//#include <stdio.h> /***/
//#include <stdlib.h> /***/
//#include <string.h> /***/
//#include <rtd_types.h> /***/
//#include <rtd_gpio.h> /***/
#include <asm/div64.h>
#include <rtk_kdriver/io.h>

#include "vgip_isr/scalerVideo.h"
#include "vgip_isr/scalerVIP.h"
#include "vgip_isr/scalerDI.h"
#include "vgip_isr/scalerDCC.h"
#include <scaler/scalerDrvCommon.h>
#include <scaler/vipCommon.h>
#include <scaler/vipRPCCommon.h>
#include <scaler/vdcCommon.h>

#include <rbus/vdtop_reg.h>
#include <rbus/vdpq_reg.h>
#include <rbus/color_sharp_reg.h>
#include <rbus/color_dcc_reg.h>
#include <rbus/color_icm_reg.h>
#include <rbus/color_reg.h>
#include <rbus/histogram_reg.h>
#include <rbus/rgb2yuv_reg.h>
#include <rbus/scaledown_reg.h>
#include <rbus/scaleup_reg.h>
#include <rbus/yuv2rgb_reg.h>
#include <rbus/od_reg.h>
#include <rbus/vodma_reg.h>
#include <rbus/smartfit_reg.h>
#include <rbus/di_reg.h>
#include <rbus/nr_reg.h>
#include <rbus/mpegnr_reg.h>
#include <rbus/idcti_reg.h>
#include <rbus/vgip_reg.h>
#include <rbus/profile_reg.h>
#include <rbus/color_dlcti_reg.h>
#include <rbus/smartfit_reg.h>
#include <rbus/iedge_smooth_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/gamma_reg.h>
#include <rbus/inv_gamma_reg.h>
#include <rbus/dm_reg.h>
#include <rbus/hdr_all_top_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/hsd_dither_reg.h>
#include <rbus/color_mb_peaking_reg.h>
//#include <rbus/sub_dither_reg.h>
#include <rbus/color_temp_reg.h>
#include <rbus/dither_reg.h>
#include <rbus/memc_mux_reg.h>

#include <mach/rtk_log.h>



#include <tvscalercontrol/vip/vip_reg_def.h>
#include "tvscalercontrol/scaler/scalerstruct.h"
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/scalerdrv/scalerip.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/color.h>


#ifdef CONFIG_HW_SUPPORT_MEMC
#include "memc_isr/scalerMEMC.h"
#include "tvscalercontrol/scaler_vpqmemcdev.h"
#endif
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv002.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <scaler_vscdev.h>

#include <rtk_kdriver/rtk_semaphore.h>
#include "vgip_isr/scalerAI.h"
/*******************************************************************************
* Macro
******************************************************************************/

//#define false FALSE
//#define true TRUE
#undef ROSPrintf
#define ROSPrintf(fmt, args...)						VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VGIP_Video_DEBUG, MA_print_count, fmt,##args)
#define ROS_XCprintf_Info(fmt, args...)				VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VIP_XC_interface1_DEBUG, MA_print_count, fmt, ##args)

//#define VIP_DEBUG_ENABLE
#ifdef VIP_DEBUG_ENABLE
  #define VIP_DEBUG_PRINTF	rtd_pr_vpq_isr_info
#else
  #define VIP_DEBUG_PRINTF(x,y...)
#endif

#define detect_pattern 0
#define motion_concentric_circles_log 0
#define black_white_pattern_log 0

unsigned char h_pan_vfir_value=0;
unsigned char Corn_tone_result = 0; // added by Jerry Wu 20100128
unsigned char Line_girl_tone_result =0; // added by Jerry Wu 20100128
unsigned char m_film_clear_isr = 0;
unsigned char ini_ReadSrcDet=0;
unsigned char pq_reset=0;

#define ABS(x,y)((x > y) ? (x-y) : (y-x))
#define abs_value(x) ( (x>0) ? (x) : (-(x)) )
#define CLAMP(a,b,c) ( (a)<(b) ? (b) : ((a)>(c)?(c):(a)) )    //!< clamp a to the range of [b;c]
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

unsigned char gSmd_weak_move=80,gSmd_offset=0x0f,gSmd_gain=2;

unsigned short PF_Trans_Sum=0;

unsigned int MA_print_count = 0;

#define Print_Profile_Data 0
unsigned char Profile_Initial_Flag = 0;
unsigned int Profile_Print_Count = 0;
unsigned int Profile_Print_Count_1 = 0;
unsigned int Profile_Print_Count_2 = 0;
extern unsigned int static_move;
unsigned char di_double_buff_apply =0;
int pure_color_weighting = 0;

#ifdef ISR_RTNR_SWITCH_CORRECTION_BIT_FUNCTION_ENABLE
static unsigned char pre_correction_flag=99;
#endif
static unsigned short log_count=0;

unsigned char vd_pwr_ok = 0;//check_vdc_power_ok

unsigned short Difference_H[16], Difference_V[16];
unsigned short very_motion_cnt=0;
unsigned short very_still_cnt=0;

//get R,G,B data for ambilight sensor with profile, elieli 20130131
#ifdef ISR_AMBILIGHT_ENABLE
#define AMBI_DEBUG_TPV 0
#define AMBI_PROFILE_DETECT_DEBUG_TPV 0
static unsigned char m_AmbilightMode = 0 ;

unsigned char Profile_RGB_data_flag_V=0, Profile_RGB_data_flag_H=0,flag_profile_read_status=0;
static unsigned short Profile_Y_V[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, Profile_U_V[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static unsigned short Profile_V_V[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, Profile_Y_H[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static unsigned short Profile_U_H[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, Profile_V_H[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static unsigned short Profile_R_V[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, Profile_G_V[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static unsigned short Profile_B_V[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, Profile_R_H[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static unsigned short Profile_G_H[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, Profile_B_H[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif
DRV_SNR_Veritcal_NR vlpf_table[2] =
{
	{0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0}, //hdmi hd ori
	{1, 0, 0, 0, 0, 0, 0, 0, 170, 6, 32, 1, 255, 6, 0, 0, 0, 0, 0, 1, 0}, //hdmi hd sd2hd
	//{1, 0, 0, 0, 0, 0, 0, 0, 233, 6, 32, 0, 255, 6, 0, 1, 0, },


};

unsigned int step_count=0;
#define BADUP_SCALING_ENTER 2500;
#define BADUP_SCALING_LEAVE 4100;
unsigned int BadUpScalingThreshold = BADUP_SCALING_ENTER;

//=================================================================================//
//=============== LeoChen add for DCR initial setting =================
//=================================================================================//
//static unsigned char Profile_Motion_Detect_ON = _ON;		//roger add 20110826, for motion detect for all source
static unsigned char Profile_Motion_Detect_ON = _OFF;		//roger add 20110826, for motion detect for all source
static unsigned char Profile_Motion_Detect_Value = 0;		//roger add 20110826, range: 0-100
//======= DCR ========================
typedef enum _SEG{
    Segment_A,
    Segment_B,
    Segment_C,
    Segment_D,
    Segment_E,
    Segment_F,
    Segment_G,
    Segment_MAX,
}SEG;

unsigned char *DCR_TABLE = NULL;

static unsigned int m_isIncreaseMode=0,m_DCR_UI_Max_Value=0,m_DCR_UI_Min_Value=0;
static unsigned int m_BL_LV_From_User=0,m_BL_LV_Act_Max=0,m_BL_LV_Act_Min=0;

static unsigned char MV_ref[DCR_NODE_NUM]= {30,40,70,90,100},BL_ref[DCR_NODE_NUM]={30,40,70,90,100};
static unsigned char BL_Duty_ref[DCR_NODE_NUM]= {30,40,70,90,100},Duty_ref[DCR_NODE_NUM]={30,40,70,90,100};
static unsigned short m_frame_need_SC=8,m_frame_need_NML=20,m_stable_buf_thl=5,m_DZ_break_thl=10;
static unsigned char DCR_set_mapping=FALSE,m_DCR_refresh=FALSE;
static unsigned char m_DCR_On_Off=FALSE,m_last_DCR_On_Off=FALSE;
static unsigned char m_DCR_mode = DCR_MASTER_ISR;
unsigned char VIP_DCR_powersave_enable=0;

// Profiling
//global for others to use it. 090713 March
//unsigned short H_Block[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//unsigned short V_Block[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static unsigned int ProfSum = 0;

static unsigned int Profile1_Y_Sum = 0;
static unsigned int Profile1_U_Sum = 0;
static unsigned int Profile1_V_Sum = 0;
static unsigned int Profile2_Y_Sum = 0;
static unsigned int Profile2_U_Sum = 0;
static unsigned int Profile2_V_Sum = 0;
static unsigned int Profile3_Y_Sum = 0;
static unsigned int Profile3_U_Sum = 0;
static unsigned int Profile3_V_Sum = 0;

static unsigned int Profile1_H_Block_size = 0;
static unsigned char Noise_Level=0;
// Input source detection
unsigned int Step_Hist[20] = {0};
unsigned int PMH_Hist[6] = {0};
static unsigned int Y_Main_Hist_pre[128] = {0};

//Elsie 20140307
//    0<--------------------------->255
//natural<----------------------->artificial
//note:
//*********************** ID PATERN FOR  DEMO*******************************8//
extern unsigned char DynamicOptimizeSystem[200];
extern unsigned char CITestPatch[1];
extern unsigned char LGBirdCRTNRPatch, LGBirdCRTNRPatchPre;
extern unsigned char LGBirdCRTNRPatchTrigger;
extern unsigned char LGPTFatoryMultiC34;
extern unsigned char LGTVPALBGE52;
extern unsigned char SandM_Chroma_Zone_Plate;

unsigned char ID_CB_NO[20];
unsigned char DI_Video_Broken = 0;
unsigned char DI_Res_Pat_pan_65ppf= 0;
unsigned char GMV_status_for_Filim = 0;

unsigned char g_Inv_Gamma = 255;
unsigned char BlackDetection_en = 0;

#if CB_flag
unsigned short CB_TSB3_CH3_y1[][32]=
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {0, 0, 0, 0, 0, 2, 7, 9, 23, 59, 33, 30, 22, 34, 50, 59, 64, 68, 65, 66, 68, 65, 56, 51, 40, 35, 37, 16, 16, 13, 0, 0, },
	/*1*/ {0, 0, 0, 0, 0, 2, 7, 9, 23, 59, 33, 30, 22, 34, 50, 59, 64, 68, 65, 66, 68, 65, 56, 51, 40, 35, 37, 16, 16, 13, 0, 0, },
	/*2*/ {0, 0, 0, 0, 0, 2, 7, 9, 23, 59, 33, 30, 22, 34, 50, 59, 64, 68, 65, 66, 68, 65, 56, 51, 40, 35, 37, 16, 16, 13, 0, 0, },
	/*3*/ {0, 0, 0, 0, 0, 2, 7, 9, 23, 59, 33, 30, 22, 34, 50, 59, 64, 68, 65, 66, 68, 65, 56, 51, 40, 35, 37, 16, 16, 13, 0, 0, },
	/*4*/ {0, 0, 0, 0, 0, 2, 7, 9, 23, 59, 33, 30, 22, 34, 50, 59, 64, 68, 65, 66, 68, 65, 56, 51, 40, 35, 37, 16, 16, 13, 0, 0, },
};
unsigned short CB_TSB3_CH3_hue1[][24] =
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {4, 4, 12, 157, 20, 5, 3, 18, 146, 320, 179, 23, 0, 0, 0, 0, 0, 0, 0, 1, 87, 5, 3, 3, },
	/*1*/ {4, 4, 12, 157, 20, 5, 3, 18, 146, 320, 179, 23, 0, 0, 0, 0, 0, 0, 0, 1, 87, 5, 3, 3, },
	/*2*/ {4, 4, 12, 157, 20, 5, 3, 18, 146, 320, 179, 23, 0, 0, 0, 0, 0, 0, 0, 1, 87, 5, 3, 3, },
	/*3*/ {4, 4, 12, 157, 20, 5, 3, 18, 146, 320, 179, 23, 0, 0, 0, 0, 0, 0, 0, 1, 87, 5, 3, 3, },
	/*4*/ {4, 4, 12, 157, 20, 5, 3, 18, 146, 320, 179, 23, 0, 0, 0, 0, 0, 0, 0, 1, 87, 5, 3, 3, },
};

unsigned short CB_TSB3_CH3_y2[][32]=
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {0, 0, 0, 1, 1, 7, 11, 10, 14, 40, 24, 16, 11, 17, 29, 56, 70, 72, 64, 53, 54, 62, 64, 76, 69, 60, 53, 18, 16, 18, 0, 0, },
	/*1*/ {0, 0, 0, 1, 1, 7, 11, 10, 14, 40, 24, 16, 11, 17, 29, 56, 70, 72, 64, 53, 54, 62, 64, 76, 69, 60, 53, 18, 16, 18, 0, 0, },
	/*2*/ {0, 0, 0, 1, 1, 7, 11, 10, 14, 40, 24, 16, 11, 17, 29, 56, 70, 72, 64, 53, 54, 62, 64, 76, 69, 60, 53, 18, 16, 18, 0, 0, },
	/*3*/ {0, 0, 0, 1, 1, 7, 11, 10, 14, 40, 24, 16, 11, 17, 29, 56, 70, 72, 64, 53, 54, 62, 64, 76, 69, 60, 53, 18, 16, 18, 0, 0, },
	/*4*/ {0, 0, 0, 1, 1, 7, 11, 10, 14, 40, 24, 16, 11, 17, 29, 56, 70, 72, 64, 53, 54, 62, 64, 76, 69, 60, 53, 18, 16, 18, 0, 0, },
};
unsigned short CB_TSB3_CH3_hue2[][24] =
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {4, 5, 12, 188, 23, 4, 3, 19, 129, 346, 171, 17, 6, 0, 0, 0, 0, 0, 0, 0, 50, 6, 4, 3, },
	/*1*/ {4, 5, 12, 188, 23, 4, 3, 19, 129, 346, 171, 17, 6, 0, 0, 0, 0, 0, 0, 0, 50, 6, 4, 3, },
	/*2*/ {4, 5, 12, 188, 23, 4, 3, 19, 129, 346, 171, 17, 6, 0, 0, 0, 0, 0, 0, 0, 50, 6, 4, 3, },
	/*3*/ {4, 5, 12, 188, 23, 4, 3, 19, 129, 346, 171, 17, 6, 0, 0, 0, 0, 0, 0, 0, 50, 6, 4, 3, },
	/*4*/ {4, 5, 12, 188, 23, 4, 3, 19, 129, 346, 171, 17, 6, 0, 0, 0, 0, 0, 0, 0, 50, 6, 4, 3, },
};

unsigned short CB_TSB3_CH3_y3[][32]=
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {0, 0, 0, 0, 2, 14, 18, 18, 21, 24, 24, 17, 19, 21, 27, 48, 61, 66, 57, 57, 66, 72, 78, 78, 62, 48, 47, 12, 11, 16, 0, 0, },
	/*1*/ {0, 0, 0, 0, 2, 14, 18, 18, 21, 24, 24, 17, 19, 21, 27, 48, 61, 66, 57, 57, 66, 72, 78, 78, 62, 48, 47, 12, 11, 16, 0, 0, },
	/*2*/ {0, 0, 0, 0, 2, 14, 18, 18, 21, 24, 24, 17, 19, 21, 27, 48, 61, 66, 57, 57, 66, 72, 78, 78, 62, 48, 47, 12, 11, 16, 0, 0, },
	/*3*/ {0, 0, 0, 0, 2, 14, 18, 18, 21, 24, 24, 17, 19, 21, 27, 48, 61, 66, 57, 57, 66, 72, 78, 78, 62, 48, 47, 12, 11, 16, 0, 0, },
	/*4*/ {0, 0, 0, 0, 2, 14, 18, 18, 21, 24, 24, 17, 19, 21, 27, 48, 61, 66, 57, 57, 66, 72, 78, 78, 62, 48, 47, 12, 11, 16, 0, 0, },
};
unsigned short CB_TSB3_CH3_hue3[][24] =
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {3, 4, 12, 268, 42, 8, 4, 18, 112, 311, 146, 28, 0, 0, 0, 0, 0, 0, 0, 0, 25, 4, 3, 3, },
	/*1*/ {3, 4, 12, 268, 42, 8, 4, 18, 112, 311, 146, 28, 0, 0, 0, 0, 0, 0, 0, 0, 25, 4, 3, 3, },
	/*2*/ {3, 4, 12, 268, 42, 8, 4, 18, 112, 311, 146, 28, 0, 0, 0, 0, 0, 0, 0, 0, 25, 4, 3, 3, },
	/*3*/ {3, 4, 12, 268, 42, 8, 4, 18, 112, 311, 146, 28, 0, 0, 0, 0, 0, 0, 0, 0, 25, 4, 3, 3, },
	/*4*/ {3, 4, 12, 268, 42, 8, 4, 18, 112, 311, 146, 28, 0, 0, 0, 0, 0, 0, 0, 0, 25, 4, 3, 3, },
};


unsigned short CB_SONY_CH84_y1[][32]=
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {0, 0, 0, 1, 35, 14, 7, 8, 13, 21, 33, 45, 49, 68, 133, 124, 343, 29, 25, 19, 13, 7, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
	/*1*/ {0, 0, 0, 1, 35, 14, 7, 8, 13, 21, 33, 45, 49, 68, 133, 124, 343, 29, 25, 19, 13, 7, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
	/*2*/ {0, 0, 0, 1, 35, 14, 7, 8, 13, 21, 33, 45, 49, 68, 133, 124, 343, 29, 25, 19, 13, 7, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
	/*3*/ {0, 0, 0, 1, 35, 14, 7, 8, 13, 21, 33, 45, 49, 68, 133, 124, 343, 29, 25, 19, 13, 7, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
	/*4*/ {0, 0, 0, 1, 35, 14, 7, 8, 13, 21, 33, 45, 49, 68, 133, 124, 343, 29, 25, 19, 13, 7, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
};
unsigned short CB_SONY_CH84_hue1[][24] =
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {5, 7, 22, 347, 197, 8, 13, 1, 0, 0, 0, 0, 3, 1, 0, 0, 0, 0, 363, 12, 4, 2, 2, 2, },
	/*1*/ {5, 7, 22, 347, 197, 8, 13, 1, 0, 0, 0, 0, 3, 1, 0, 0, 0, 0, 363, 12, 4, 2, 2, 2, },
	/*2*/ {5, 7, 22, 347, 197, 8, 13, 1, 0, 0, 0, 0, 3, 1, 0, 0, 0, 0, 363, 12, 4, 2, 2, 2, },
	/*3*/ {5, 7, 22, 347, 197, 8, 13, 1, 0, 0, 0, 0, 3, 1, 0, 0, 0, 0, 363, 12, 4, 2, 2, 2, },
	/*4*/ {5, 7, 22, 347, 197, 8, 13, 1, 0, 0, 0, 0, 3, 1, 0, 0, 0, 0, 363, 12, 4, 2, 2, 2, },
};

unsigned short CB_SONY_CH84_y2[][32]=
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {0, 0, 0, 0, 23, 14, 8, 8, 8, 11, 16, 23, 33, 43, 80, 124, 495, 30, 26, 23, 16, 7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
	/*1*/ {0, 0, 0, 0, 23, 14, 8, 8, 8, 11, 16, 23, 33, 43, 80, 124, 495, 30, 26, 23, 16, 7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
	/*2*/ {0, 0, 0, 0, 23, 14, 8, 8, 8, 11, 16, 23, 33, 43, 80, 124, 495, 30, 26, 23, 16, 7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
	/*3*/ {0, 0, 0, 0, 23, 14, 8, 8, 8, 11, 16, 23, 33, 43, 80, 124, 495, 30, 26, 23, 16, 7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
	/*4*/ {0, 0, 0, 0, 23, 14, 8, 8, 8, 11, 16, 23, 33, 43, 80, 124, 495, 30, 26, 23, 16, 7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
};
unsigned short CB_SONY_CH84_hue2[][24] =
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {2, 3, 25, 419, 254, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 256, 8, 2, 1, 1, 1, },
	/*1*/ {2, 3, 25, 419, 254, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 256, 8, 2, 1, 1, 1, },
	/*2*/ {2, 3, 25, 419, 254, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 256, 8, 2, 1, 1, 1, },
	/*3*/ {2, 3, 25, 419, 254, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 256, 8, 2, 1, 1, 1, },
	/*4*/ {2, 3, 25, 419, 254, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 256, 8, 2, 1, 1, 1, },
};

unsigned short CB_USB_Red_Flower_Bee_y1[][32]=
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {0, 0, 0, 0, 0, 1, 17, 80, 317, 139, 47, 28, 23, 18, 19, 26, 42, 66, 55, 52, 14, 13, 13, 14, 2, 0, 0, 0, 0, 0, 1, 0, },
	/*1*/ {0, 0, 0, 0, 0, 1, 17, 80, 317, 139, 47, 28, 23, 18, 19, 26, 42, 66, 55, 52, 14, 13, 13, 14, 2, 0, 0, 0, 0, 0, 1, 0, },
	/*2*/ {0, 0, 0, 0, 0, 1, 17, 80, 317, 139, 47, 28, 23, 18, 19, 26, 42, 66, 55, 52, 14, 13, 13, 14, 2, 0, 0, 0, 0, 0, 1, 0, },
	/*3*/ {0, 0, 0, 0, 0, 1, 17, 80, 317, 139, 47, 28, 23, 18, 19, 26, 42, 66, 55, 52, 14, 13, 13, 14, 2, 0, 0, 0, 0, 0, 1, 0, },
	/*4*/ {0, 0, 0, 0, 0, 1, 17, 80, 317, 139, 47, 28, 23, 18, 19, 26, 42, 66, 55, 52, 14, 13, 13, 14, 2, 0, 0, 0, 0, 0, 1, 0, },
};
unsigned short CB_USB_Red_Flower_Bee_hue1[][24] =
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {397, 327, 63, 78, 108, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, },
	/*1*/ {397, 327, 63, 78, 108, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, },
	/*2*/ {397, 327, 63, 78, 108, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, },
	/*3*/ {397, 327, 63, 78, 108, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, },
	/*4*/ {397, 327, 63, 78, 108, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, },
};

unsigned short CB_USB_Red_Flower_Bee_y2[][32]=
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {0, 0, 0, 0, 3, 8, 24, 80, 325, 136, 43, 25, 21, 17, 18, 26, 41, 62, 53, 51, 13, 13, 13, 13, 2, 0, 0, 0, 0, 0, 1, 0, },
	/*1*/ {0, 0, 0, 0, 3, 8, 24, 80, 325, 136, 43, 25, 21, 17, 18, 26, 41, 62, 53, 51, 13, 13, 13, 13, 2, 0, 0, 0, 0, 0, 1, 0, },
	/*2*/ {0, 0, 0, 0, 3, 8, 24, 80, 325, 136, 43, 25, 21, 17, 18, 26, 41, 62, 53, 51, 13, 13, 13, 13, 2, 0, 0, 0, 0, 0, 1, 0, },
	/*3*/ {0, 0, 0, 0, 3, 8, 24, 80, 325, 136, 43, 25, 21, 17, 18, 26, 41, 62, 53, 51, 13, 13, 13, 13, 2, 0, 0, 0, 0, 0, 1, 0, },
	/*4*/ {0, 0, 0, 0, 3, 8, 24, 80, 325, 136, 43, 25, 21, 17, 18, 26, 41, 62, 53, 51, 13, 13, 13, 13, 2, 0, 0, 0, 0, 0, 1, 0, },
};
unsigned short CB_USB_Red_Flower_Bee_hue2[][24] =
{ //#0  ntsc 0 IRE ,#1  ntsc 7.5 IRE , #2 cvbs pal  ,#3 hdmi_sd , #4 hdmi_hd
	/*0*/ {399, 332, 56, 78, 108, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, },
	/*1*/ {399, 332, 56, 78, 108, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, },
	/*2*/ {399, 332, 56, 78, 108, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, },
	/*3*/ {399, 332, 56, 78, 108, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, },
	/*4*/ {399, 332, 56, 78, 108, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, },
};

#endif


//******************************** ID PATTERN FOR DEMO************************************************8

#define pthread_mutex_lock(a)  do {} while(0)
#define pthread_mutex_unlock(a) do {} while(0)

#define rtdf_outb 			IoReg_WriteByte0
#define rtdf_outl 				rtd_outl
//#define rtd_outw 				IoReg_WriteWord0
#define rtdf_inl				rtd_inl

//yuan1024 rtnr n field avg
unsigned char VD_noise_status_Num = 1;

//unsigned char 	SceneChange=0, SceneChange_NR=0;
unsigned char 	SubSceneChange=0;

//unsigned char 	Main_Set_SC=0;
unsigned int 	Total_HistCnt_Sub=0;

static unsigned int RTNR_MAD_count_Y_sum_avg = 0;

//=============== Mpeg NR =================================
// static unsigned int       MpegEnable_flag=0;
static unsigned int       array_xstep[3]={0x1000,0x1c71,0x2aab};
static unsigned char 	  kernel_quality_init=0;

//=============== Skin Tone Detection =================
unsigned char        skin_tone_for_panasonic=0;
unsigned char        skin_tone_begin=0;
unsigned char        start_to_detected = 0;
unsigned char		skin_tone_found;
unsigned char		new_skin_tone_flag=0;
unsigned char		skin_tone_huu_bin_ratio=0;

//========= User Defined Curve & Pure Color  Detection ==========
unsigned int  hue_hist_2[24];

//=============== for MEMC Info jzl 20151231 =================
SET_MEMC_SCALER_VIP_INFO setScalerVIPInfo = {0};

//=============== for MEMC info ==============================
//#define complement2(arg) (((~arg)&0x7FF)+1)

//=====================================================
//==================== new RTNR adaptive table =================
//=====================================================
#ifdef ISR_NEW_RTNR_ENABLE
//Ypbpr/HDMI
static unsigned char New_RTNR_Coef_1[8][8] = {
		/*edge		0	1	2	3	4	5	6	7	*/
		/*sad  */
		/*	0  */{	0,	1,	2,	3,	4,	5,	7,	7},
		/*	1  */{	1,	1,	2,	3,	5,	6,	7,	7},
		/*	2  */{	2,	2,	3,	4,	5,	7,	7,	7},
		/*	3  */{	2,	3,	4,	5,	6,	7,	7,	7},
		/*	4  */{	4,	6,	6,	6,	6,	7,	7,	7},
		/*	5  */{	5,	5,	7,	7,	7,	7,	7,	7},
		/*	6  */{	7,	7,	7,	7,	7,	7,	7,	7},
		/*	7  */{	7,	7,	7,	7,	7,	7,	7,	7},
};

static unsigned char TV_AutoNr_RTNR_Coef_A[8][8]={
			/*edge		0	1	2	3	4	5	6	7	*/
			/*sad  */
			/*	0  */{	1,	1,	3,	4,	5,	6,	7,	7},
			/*	1  */{	1,	1,	3,	5,	6,	7,	7,	7},
			/*	2  */{	2,	2,	4,	5,	6,	7,	7,	7},
			/*	3  */{	4,	5,	5,	6,	6,	7,	7,	7},
			/*	4  */{	6,	6,	6,	6,	6,	7,	7,	7},
			/*	5  */{	7,	7,	7,	7,	7,	7,	7,	7},
			/*	6  */{	7,	7,	7,	7,	7,	7,	7,	7},
			/*	7  */{	7,	7,	7,	7,	7,	7,	7,	7},

			};
static unsigned char TV_AutoNr_RTNR_Coef_B[8][8]={
			/*edge		0	1	2	3	4	5	6	7	*/
			/*sad  */
			/*	0  */{	2,	2,	3,	3,	5,	6,	7,	7},
			/*	1  */{	2,	2,	4,	5,	6,	7,	7,	7},
			/*	2  */{	3,	4,	4,	5,	6,	7,	7,	7},
			/*	3  */{	4,	5,	5,	6,	6,	7,	7,	7},
			/*	4  */{	5,	6,	6,	6,	6,	7,	7,	7},
			/*	5  */{	7,	7,	7,	7,	7,	7,	7,	7},
			/*	6  */{	7,	7,	7,	7,	7,	7,	7,	7},
			/*	7  */{	7,	7,	7,	7,	7,	7,	7,	7},

			};

// CVBS
static unsigned char CVBS_AutoNr_RTNR_Coef_A[8][8]={
			/*edge		0	1	2	3	4	5	6	7	*/
			/*sad  */
			/*	0  */{	0,	1,	2,	4,	5,	6,	7,	7},
			/*	1  */{	1,	2,	3,	5,	6,	7,	7,	7},
			/*	2  */{	2,	3,	4,	5,	6,	7,	7,	7},
			/*	3  */{	4,	5,	5,	6,	6,	7,	7,	7},
			/*	4  */{	6,	6,	6,	6,	6,	7,	7,	7},
			/*	5  */{	7,	7,	7,	7,	7,	7,	7,	7},
			/*	6  */{	7,	7,	7,	7,	7,	7,	7,	7},
			/*	7  */{	7,	7,	7,	7,	7,	7,	7,	7},
			};
static unsigned char CVBS_AutoNr_RTNR_Coef_B[8][8]={
			/*edge		0	1	2	3	4	5	6	7	*/
			/*sad  */
			/*	0  */{	1,	2,	3,	5,	6,	7,	7,	7},
			/*	1  */{	2,	2,	4,	5,	6,	7,	7,	7},
			/*	2  */{	3,	3,	4,	5,	6,	7,	7,	7},
			/*	3  */{	4,	4,	5,	6,	7,	7,	7,	7},
			/*	4  */{	5,	6,	6,	7,	7,	7,	7,	7},
			/*	5  */{	7,	7,	7,	7,	7,	7,	7,	7},
			/*	6  */{	7,	7,	7,	7,	7,	7,	7,	7},
			/*	7  */{	7,	7,	7,	7,	7,	7,	7,	7},
			};
static unsigned char CVBS_AutoNr_RTNR_Coef_C[8][8]={
			/*edge		0	1	2	3	4	5	6	7	*/
			/*sad  */
			/*	0  */{	2,	2,	3,	3,	5,	6,	7,	7},
			/*	1  */{	2,	2,	4,	5,	6,	7,	7,	7},
			/*	2  */{	3,	4,	4,	5,	6,	7,	7,	7},
			/*	3  */{	4,	5,	5,	6,	6,	7,	7,	7},
			/*	4  */{	5,	6,	6,	6,	6,	7,	7,	7},
			/*	5  */{	7,	7,	7,	7,	7,	7,	7,	7},
			/*	6  */{	7,	7,	7,	7,	7,	7,	7,	7},
			/*	7  */{	7,	7,	7,	7,	7,	7,	7,	7},
			};
#endif
//==========================================================================
//==========================================================================

//=======================
static unsigned int andMASK_Table1[32]=	{
							0xfffffffe, 0xfffffffc, 0xfffffff8, 0xfffffff0,
							0xffffffe0, 0xffffffc0, 0xffffff80, 0xffffff00,
							0xfffffe00, 0xfffffc00, 0xfffff800, 0xfffff000,
							0xffffe000, 0xffffc000, 0xffff8000, 0xffff0000,
							0xfffe0000, 0xfffc0000, 0xfff80000, 0xfff00000,
							0xffe00000, 0xffc00000, 0xff800000, 0xff000000,
							0xfe000000, 0xfc000000, 0xf8000000, 0xf0000000,
							0xe0000000, 0xc0000000, 0x80000000, 0x00000000
							};
static unsigned int andMASK_Table2[32]=	{
							0x00000000, 0x00000001, 0x00000003, 0x00000007,
							0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
							0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
							0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
							0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
							0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
							0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
							0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff
							};
unsigned int PQA_Sort_Tmp[2][PQA_ITEM_MAX];
//=======================

//=================================================================================//
extern _system_setting_info		*system_info_structure_table;
extern _RPC_system_setting_info	*RPC_system_info_structure_table;
extern SLR_VIP_TABLE			*g_Share_Memory_VIP_TABLE_Struct_isr;
extern SLR_VIP_TABLE_CUSTOM_TV001	 *g_Share_Memory_VIP_TABLE_Custom_Struct_isr;
extern _clues *SmartPic_clue;
extern _RPC_clues *RPC_SmartPic_clue;
extern VIP_table_crc_value *vip_table_crc_isr;
extern VIP_DeMura_TBL DeMura_TBL;

Stop_Polling_t	 Stop_Polling;
Start_Print_info_t Start_Print={1};//for scalerDi.cpp ROSprintrf( %0) issue !!!

//=================================================================================//
//	TG45_Flag
//=================================================================================//
//static unsigned char TG45_Flag = 0;
unsigned char TG45_Flag = VIP_TG45_FLAG_NUM;
static unsigned char pre_TG45_Flag = 0;
//=================================================================================//

unsigned char Enable_Hist_MAD_Calculate = 1;
unsigned char colorless_flag = 0;
unsigned short hue_hist_ratio[24]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short Y_hist_ratio[TV006_VPQ_chrm_bin]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short Pre_Hue_hist_ratio[24] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short Pre_Y_hist_ratio[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned int RTNR_MAD_count_Y_avg=0,RTNR_MAD_count_U_avg=0,RTNR_MAD_count_V_avg=0;
unsigned int RTNR_MAD_count_Y2_avg=0,RTNR_MAD_count_U2_avg=0,RTNR_MAD_count_V2_avg=0;
unsigned int RTNR_MAD_count_Y3_avg=0,RTNR_MAD_count_U3_avg=0,RTNR_MAD_count_V3_avg=0;
unsigned int hmcnr_sttistic[11];
unsigned short center=0,L1=0,R1=0,others=0;

unsigned int sat_hist_ratio[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned int sat_hist[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned int sat_ratio_mean_value;

//for T2 2644
#define meanSat_meanValue_printf_log_flag 0
unsigned int edge_motion_count, edge_motion_count_ave, total_motion_count, total_motion_count_ave, noise_flat_motion_count, noise_flatl_motion_count_ave;

unsigned short 	*intelligent_picture_table=NULL;
unsigned char Sony_DCC_API_ON = 0, TV_002_style_ON = 0;
unsigned char Sony_Radio_Caculator_flag, Sony_Radio_Caculator_RTNR_offset=0;

//========profile block motion========
unsigned char block_motion_count_H = 0;
unsigned char block_motion_count_V = 0;
unsigned char block_noise_count_H = 0;
unsigned char block_noise_count_V = 0;

//========3D mode========
unsigned char is3DLRmode = FALSE;


/*******************************************************************************
* Program
******************************************************************************/

extern unsigned int scalerInfo_getPanelInfo_3DPanelType(void);
extern unsigned char scalerInfo_check_is_3dMode(void);
extern unsigned char VODMA_3D_Check_Is_Idma3dMode(void);

unsigned char BusyCheckRegBit(unsigned int addr, unsigned int value, unsigned int timeoutcnt)
{
	do {
		if(scaler_rtd_inl(addr) & value) {
			return TRUE;
		}
	} while(timeoutcnt-- != 0);

	return FALSE;
}


void drif_color_Edge_Noise_level(unsigned char ch, _rtnr_enl *ENL)
{
	if(ch==1)//sub
	{
	//  scalerDrvPrint("\n drif_color_enl() SUB!\n");
	 return ;
	}
	else//main
	{
	//  scalerDrvPrint("\n enl()\n");
		di_im_di_rtnr_et_count_y_RBUS im_di_rtnr_et_count_y_reg;
		di_im_di_rtnr_et_sum_y_RBUS im_di_rtnr_et_sum_y_reg;
//		di_im_di_rtnr_et_sum_u_RBUS im_di_rtnr_et_sum_u_reg;
//		di_im_di_rtnr_et_sum_v_RBUS im_di_rtnr_et_sum_v_reg;
		im_di_rtnr_et_count_y_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_COUNT_Y_reg);
		im_di_rtnr_et_sum_y_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_SUM_Y_reg);
		//im_di_rtnr_et_sum_u_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_SUM_U_reg);
		//im_di_rtnr_et_sum_v_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_SUM_V_reg);

		(ENL->ENL_Y)=  (im_di_rtnr_et_sum_y_reg.cp_temporal_same_et_sad_sum_y)/(im_di_rtnr_et_count_y_reg.cp_temporal_same_et_cnt_y+1);
		//(ENL->ENL_U)=  (im_di_rtnr_et_sum_u_reg.cp_temporal_same_et_sad_sum_u)/(im_di_rtnr_et_count_y_reg.cp_temporal_same_et_cnt_y+1);
		//(ENL->ENL_V)=  (im_di_rtnr_et_sum_v_reg.cp_temporal_same_et_sad_sum_v)/(im_di_rtnr_et_count_y_reg.cp_temporal_same_et_cnt_y+1);

		//  if(abc_cnt>=30)
		//  {
		//   scalerDrvPrint("\n******\nVIP: cnt=%d, ENL_Y=%d, ENL_U=%d, ENL_V=%d \n\n******\n", abc_cnt, ENL_Y,ENL_U,ENL_V);
		//   abc_cnt = 0;
		//  }
		//  abc_cnt++;
	}
}

void drvif_color_ClueGather( unsigned char ch, _clues* clueA)
{
	_rtnr_enl VIP_ENL={0};


	di_im_di_si_film_motion_next_h_t_RBUS next_h_t;
	di_im_di_si_film_motion_next_h_m_RBUS next_h_m;
	di_im_di_si_film_motion_next_h_b_RBUS next_h_b;

	di_im_di_si_film_motion_pre_h_t_RBUS pre_h_t;
	di_im_di_si_film_motion_pre_h_m_RBUS pre_h_m;
	di_im_di_si_film_motion_pre_h_b_RBUS pre_h_b;

	di_im_di_si_film_motion_next_v_l_RBUS next_v_l;
	di_im_di_si_film_motion_next_v_m_RBUS next_v_m;
	di_im_di_si_film_motion_next_v_r_RBUS next_v_r;

	di_im_di_si_film_motion_pre_v_l_RBUS pre_v_l;
	di_im_di_si_film_motion_pre_v_m_RBUS pre_v_m;
	di_im_di_si_film_motion_pre_v_r_RBUS pre_v_r;

	di_im_di_si_film_motion_next_c_RBUS next_c;
	di_im_di_si_film_motion_pre_c_RBUS pre_c;

	di_im_di_si_ma_total_fm_sum_RBUS total_fm_sum;
	di_im_di_si_ma_large_fm_pixel_RBUS large_fm_pixel;
	di_im_di_si_ma_large_fm_sum_RBUS large_fm_sum;

	next_h_t.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);
	next_h_m.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);
	next_h_b.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);

	pre_h_t.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);
	pre_h_m.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);
	pre_h_b.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);

	next_v_l.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);
	next_v_m.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);
	next_v_r.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);

	pre_v_l.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);
	pre_v_m.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);
	pre_v_r.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);

	next_c.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);
	pre_c.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);

	total_fm_sum.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);
	large_fm_pixel.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);
	large_fm_sum.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);

	clueA->film_motion_next_h_t = next_h_t.film_motionstatus_699_680;
	clueA->film_motion_next_h_m = next_h_m.film_motionstatus_679_660;
	clueA->film_motion_next_h_b = next_h_b.film_motionstatus_659_640;

	clueA->film_motion_pre_h_t = pre_h_t.film_motionstatus_539_520;
	clueA->film_motion_pre_h_m = pre_h_m.film_motionstatus_519_500;
	clueA->film_motion_pre_h_b = pre_h_b.film_motionstatus_499_480;

	clueA->film_motion_next_v_l = next_v_l.film_motionstatus_639_620;
	clueA->film_motion_next_v_m = next_v_m.film_motionstatus_619_600;
	clueA->film_motion_next_v_r = next_v_r.film_motionstatus_599_580;

	clueA->film_motion_pre_v_l = pre_v_l.film_motionstatus_479_460;
	clueA->film_motion_pre_v_m = pre_v_m.film_motionstatus_459_440;
	clueA->film_motion_pre_v_r = pre_v_r.film_motionstatus_439_420;

	clueA->film_motion_next_c = next_c.film_motionstatus_579_560;
	clueA->film_motion_pre_c = pre_c.film_motionstatus_419_400;

	clueA->ma_total_fm_sum = total_fm_sum.total_frame_motion_sum;
	clueA->ma_large_fm_pixel = large_fm_pixel.large_frame_motion_pixel;
	clueA->ma_large_fm_sum = large_fm_sum.large_frame_motion_sum;


	drif_color_Edge_Noise_level(ch, &VIP_ENL);
	clueA->rtnr_noise = VIP_ENL.ENL_Y;
}

/**
 * Detect color histogram
 *
 * @param	en_test: debug mode
 * @return	void
 */
void scalerVIP_colorHistDetect(unsigned int en_test)
{
	static unsigned int DarkSum = 0;
	static unsigned int BritSum = 0;
	static unsigned char flag_g_PureColor = 0;

	unsigned char it = 0;
	unsigned int Sum_Hist = 0;
	unsigned char en_DumpHist = 0;
	unsigned int ratio_Hist = 0;
	unsigned char th_HistChk = 0;
	unsigned char th_HistChk_in = 0;
	unsigned char th_HistChk_out = 0;
	unsigned char en_HistTest = 0;
	//unsigned char flag_PureColor = 0;
	unsigned int tmp_max = 0;
	unsigned int *reg_HistCnt=(SmartPic_clue->Y_Main_Hist);

	//scalerDrvPrint( "enter scalerVIP_colorHistDetect\n" );

	//----dump histogram data and calculate sumation-----
	//en_DumpHist = (en_test<<28)>>31; //bit 3
	en_DumpHist = (en_test & _BIT3)>>3; //bit 3

	for ( it=0 ; it<32 ; it++ ) {
		Sum_Hist += reg_HistCnt[it];
		//if ( en_DumpHist == 1 ) {
		//	scalerDrvPrint( "Hist BIN%d=%d\n", it, reg_HistCnt[it] );
		//}
	}

	//if ( en_DumpHist == 1 ) {
	//	scalerDrvPrint( "Hist Sum=%d\n", Sum_Hist );
	//}

	DarkSum = (reg_HistCnt[0]<<2)+(reg_HistCnt[1]<<1)+reg_HistCnt[2]+reg_HistCnt[3];
	BritSum = (reg_HistCnt[31]<<2)+(reg_HistCnt[30]<<1)+reg_HistCnt[29]+reg_HistCnt[28];


	//th_HistChk_in = (en_test<<16)>>24;//bit8~15
	th_HistChk_in = 80;
	//th_HistChk_out = (en_test<<8)>>24;//bit16~23
	th_HistChk_out = 75;
	//en_HistTest = (en_test<<27)>>31;//bit 4
	en_HistTest = (en_test & _BIT4)>>4;//bit 4

	//-----set th----
	if ( flag_g_PureColor ==0 )
		th_HistChk = th_HistChk_in;
	else
		th_HistChk = th_HistChk_out;

	//scalerDrvPrint( "=============th_HistChk_in=%d================\n", th_HistChk_in );

	for ( it=7 ; it<22 ; it++ ) {
		if ( Sum_Hist != 0 ) {
			ratio_Hist = (reg_HistCnt[it]+reg_HistCnt[it+1]) * 100 / Sum_Hist;
			//scalerDrvPrint( "=============RatioHist=%d================\n", ratio_Hist );
		}
		if ( ratio_Hist > tmp_max )
			tmp_max = ratio_Hist;
	}

	if ( tmp_max>th_HistChk && Sum_Hist )
		flag_g_PureColor = 1;
	else
		flag_g_PureColor = 0;
}


unsigned char scalerVIP_Saturation_Detection(void)//modify for mac3 icm
{
	unsigned int m_nRatio=0;
	unsigned char m_nCount=2,nIndex;
	unsigned char Saturation_detect_flag = 0,sat_thl=98;

	for(nIndex = 0 ;nIndex <m_nCount;nIndex++)
		m_nRatio += (SmartPic_clue->Sat_Main_His_Ratio)[nIndex];

	if(m_nRatio > sat_thl)
	{
		Saturation_detect_flag = 1;
	}
	else
	{
		Saturation_detect_flag = 0;
	}
	return Saturation_detect_flag;

	return 0 ;

}

unsigned char VD_noise_Status_sum_avg()
{
	static unsigned int VD_noise_count_Status_sum_avg = 0;
	static unsigned char VD_noise_count_Status_value[255] = {0};  //store value
	unsigned char countt;
	static unsigned int counter_noise = 0;
	static unsigned char VD_noise_flag = 1;
	static unsigned int VD_noise_count_Status_sum = 0;

	if ((counter_noise < (VD_noise_status_Num+1)) && VD_noise_flag)
	{
		VD_noise_count_Status_value[counter_noise % (VD_noise_status_Num+1)] = (unsigned char)VDPQ_rtd_inl(VDTOP_NOISE_STATUS_reg);
		VD_noise_count_Status_sum = VD_noise_count_Status_sum + VD_noise_count_Status_value[counter_noise % (VD_noise_status_Num+1)];
		VD_noise_count_Status_sum_avg = VD_noise_count_Status_sum /(counter_noise+1);
		//ROSPrintf(" yuan1024VD_noise_count_Status_sum_avg222222222222  = %d  %d\n",VD_noise_count_Status_sum_avg, VD_noise_count_Status_sum);
	}
	else
	{
		countt = counter_noise % (VD_noise_status_Num+1);

		if(VD_noise_count_Status_sum > VD_noise_count_Status_value[countt])
			VD_noise_count_Status_sum = VD_noise_count_Status_sum - VD_noise_count_Status_value[countt];
		else
			VD_noise_count_Status_sum = 0;

		VD_noise_count_Status_value[countt] = (unsigned char)VDPQ_rtd_inl(VDTOP_NOISE_STATUS_reg);

		VD_noise_count_Status_sum = ( VD_noise_count_Status_sum + VD_noise_count_Status_value[countt]);
		VD_noise_count_Status_sum_avg = VD_noise_count_Status_sum / (VD_noise_status_Num+1);
		VD_noise_flag = 0;
	}

	counter_noise ++;
	#if 0
	if(counter_noise % 60 == 0)
	{
		ROSPrintf("-----------------yuan1024-----------------------\n");
		ROSPrintf(" yuan1024 VD_noise_status_Num  =%d\n",VD_noise_status_Num+1);
		ROSPrintf(" yuan1024 VD_noise_count_Status_sum_avg  = %d\n",VD_noise_count_Status_sum_avg);
		ROSPrintf("-----------------yuan1024-----------------------\n");


	}
	#endif

	return VD_noise_count_Status_sum_avg;

}

//====================================================
#define Field_count_check 5
#define Adap_newRTNR_Count_Num 5
#define MAD_th 5


	//=============黑白(灰階)畫面
	//+++++++++++++做vertical lpf
	unsigned int Gray_solution_table[5][6] =
	{
//		en,		hfd_statistic_max	hfd_statistic_min	vfir_weight_max	vfir_weight_min
		{1,		20000,			4000,			5,				9,				},	//NTSC
		{1,		20000,			4000,			5,				9,				},	//PAL
		{1,		20000,			4000,			5,				9,				},	//SD
		{1,		20000,			4000,			5,				9,				},	//HD
		{1,		20000,			4000,			5,				9,				},	//VGA
	};

unsigned char drvif_color_check_source_remmping(unsigned char display)
{
	unsigned char which_source = 255;
	unsigned char remmping_source = 255;

	which_source = (RPC_system_info_structure_table->VIP_source);

	if(which_source==VIP_QUALITY_TV_NTSC||which_source==VIP_QUALITY_CVBS_NTSC)
		remmping_source = 0;
	else if(which_source==VIP_QUALITY_TV_PAL||which_source==VIP_QUALITY_CVBS_PAL)
		remmping_source = 1;
	else if(((which_source>=VIP_QUALITY_YPbPr_480I) && (which_source<=VIP_QUALITY_YPbPr_576P))||
		    ((which_source>=VIP_QUALITY_HDMI_480I) 	&& (which_source<=VIP_QUALITY_HDMI_576P) )||
			((which_source>=VIP_QUALITY_DTV_480I) 	&& (which_source<=VIP_QUALITY_DTV_576P)  )||
			((which_source>=VIP_QUALITY_PVR_480I) 	&& (which_source<=VIP_QUALITY_PVR_576P)  ))
	{
		remmping_source = 2;
	}
	else if(((which_source>=VIP_QUALITY_YPbPr_720P) && (which_source<=VIP_QUALITY_YPbPr_1080P))||
			((which_source>=VIP_QUALITY_HDMI_720P) 	&& (which_source<=VIP_QUALITY_HDMI_1080P))||
			((which_source>=VIP_QUALITY_DTV_720P) 	&& (which_source<=VIP_QUALITY_DTV_1080P))||
			((which_source>=VIP_QUALITY_PVR_720P) 	&& (which_source<=VIP_QUALITY_PVR_1080P)))
	{
		remmping_source = 3;
	}
	else
	{
		remmping_source = 2;
	}

    return(remmping_source);
}


void drvif_color_inewdcti_set(unsigned char display, unsigned char data_sel,  unsigned char maxlen, unsigned char psmth, unsigned char lpmode, unsigned char engdiv, unsigned char offdiv, unsigned char uvgain, unsigned char dcti_mode, unsigned char cur_sel, unsigned char dcti_en)
{
	idcti_i_dcti_ctrl_1_RBUS ich1_newdcti_reg;

	if (display == SLR_MAIN_DISPLAY)
	{

		ich1_newdcti_reg.regValue=scaler_rtd_inl(IDCTI_I_DCTI_CTRL_1_reg);


		if(ich1_newdcti_reg.data_sel < data_sel)
			ich1_newdcti_reg.data_sel++;
		else if(ich1_newdcti_reg.data_sel > data_sel)
			ich1_newdcti_reg.data_sel--;

		if(ich1_newdcti_reg.maxlen < maxlen)
			ich1_newdcti_reg.maxlen++;
		else if(ich1_newdcti_reg.maxlen > maxlen)
			ich1_newdcti_reg.maxlen--;

		if(ich1_newdcti_reg.psmth < psmth)
			ich1_newdcti_reg.psmth++;
		else if(ich1_newdcti_reg.psmth > psmth)
			ich1_newdcti_reg.psmth--;


		if(ich1_newdcti_reg.lp_mode < lpmode)
			ich1_newdcti_reg.lp_mode++;
		else if(ich1_newdcti_reg.lp_mode > lpmode)
			ich1_newdcti_reg.lp_mode--;

		if(ich1_newdcti_reg.engdiv < engdiv)
			ich1_newdcti_reg.engdiv++;
		else if(ich1_newdcti_reg.engdiv > engdiv)
			ich1_newdcti_reg.engdiv--;



		if(ich1_newdcti_reg.offdiv < offdiv)
			ich1_newdcti_reg.offdiv++;
		else if(ich1_newdcti_reg.offdiv > offdiv)
			ich1_newdcti_reg.offdiv--;

		if(ich1_newdcti_reg.uvgain < uvgain)
			ich1_newdcti_reg.uvgain++;
		else if(ich1_newdcti_reg.uvgain > uvgain)
			ich1_newdcti_reg.uvgain--;

		if(ich1_newdcti_reg.dcti_mode < dcti_mode)
			ich1_newdcti_reg.dcti_mode++;
		else if(ich1_newdcti_reg.dcti_mode > dcti_mode)
			ich1_newdcti_reg.dcti_mode--;

		if(ich1_newdcti_reg.cur_sel < cur_sel)
			ich1_newdcti_reg.cur_sel++;
		else if(ich1_newdcti_reg.cur_sel > cur_sel)
			ich1_newdcti_reg.cur_sel--;

		if(ich1_newdcti_reg.dcti_en < dcti_en)
			ich1_newdcti_reg.dcti_en++;
		else if(ich1_newdcti_reg.dcti_en > dcti_en)
			ich1_newdcti_reg.dcti_en--;


		scaler_rtd_outl(IDCTI_I_DCTI_CTRL_1_reg, ich1_newdcti_reg.regValue);

	}

}

//JZ Mac3
void drvif_color_inewdcti_table_isr(DRV_VipNewIDcti_Table* ptr,unsigned char offset_gain)
{
	idcti_i_dcti_ctrl_1_RBUS ich1_newdcti_reg_1;
	idcti_i_dcti_ctrl_2_RBUS ich1_newdcti_reg_2;
	idcti_i_dcti_ctrl_3_RBUS ich1_newdcti_reg_3;
	idcti_i_dcti_ctrl_4_RBUS ich1_newdcti_reg_4;
	idcti_i_dcti_ctrl_5_RBUS ich1_newdcti_reg_5;
	idcti_i_dcti_ctrl_6_RBUS ich1_newdcti_reg_6;
	idcti_i_dcti_ctrl_7_RBUS ich1_newdcti_reg_7;

	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ColorBar==TRUE))
	{
		ROSPrintf("1.offset_gain=%d\n",offset_gain);
	}

	if(!ptr)
		return;

//Ctl1
	ich1_newdcti_reg_1.regValue= scaler_rtd_inl(IDCTI_I_DCTI_CTRL_1_reg);

	ich1_newdcti_reg_1.data_sel = ptr->VipNewIDcti_ctl1.data_sel;
	ich1_newdcti_reg_1.maxlen = ptr->VipNewIDcti_ctl1.maxlen;
	ich1_newdcti_reg_1.psmth = ptr->VipNewIDcti_ctl1.psmth;
	ich1_newdcti_reg_1.lp_mode = ptr->VipNewIDcti_ctl1.lp_mode;
	ich1_newdcti_reg_1.engdiv = ptr->VipNewIDcti_ctl1.engdiv;
	ich1_newdcti_reg_1.offdiv= ptr->VipNewIDcti_ctl1.offdiv;

	ich1_newdcti_reg_1.uvgain= ((ptr->VipNewIDcti_ctl1.uvgain)+offset_gain)>127?127:((ptr->VipNewIDcti_ctl1.uvgain)+offset_gain);

	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ColorBar==TRUE))
	{
		ROSPrintf("2.uvgain=%d, offset_gain=%d\n",(ptr->VipNewIDcti_ctl1.uvgain),offset_gain);
	}

	/*
	if(ich1_newdcti_reg_1.uvgain>127)
	{
		ich1_newdcti_reg_1.uvgain = 127;
	}
	*/
	ich1_newdcti_reg_1.dcti_mode= ptr->VipNewIDcti_ctl1.dcti_mode;
	ich1_newdcti_reg_1.uvalign_en= ptr->VipNewIDcti_ctl1.uvalign_en;
	ich1_newdcti_reg_1.cur_sel = ptr->VipNewIDcti_ctl1.cur_sel;
	ich1_newdcti_reg_1.dcti_en = ptr->VipNewIDcti_ctl1.dcti_en;

	scaler_rtd_outl(IDCTI_I_DCTI_CTRL_1_reg, ich1_newdcti_reg_1.regValue);

//Ctl2
	ich1_newdcti_reg_2.regValue= scaler_rtd_inl(IDCTI_I_DCTI_CTRL_2_reg);

	ich1_newdcti_reg_2.uvsync_en= ptr->VipNewIDcti_ctl2.uvsync_en;
	ich1_newdcti_reg_2.tran_mode= ptr->VipNewIDcti_ctl2.tran_mode;
	ich1_newdcti_reg_2.rate_steep= ptr->VipNewIDcti_ctl2.rate_steep;
	ich1_newdcti_reg_2.th_steep= ptr->VipNewIDcti_ctl2.th_steep;
	ich1_newdcti_reg_2.th_coring= ptr->VipNewIDcti_ctl2.th_coring;
	ich1_newdcti_reg_2.steep_mode= ptr->VipNewIDcti_ctl2.steep_mode;
	ich1_newdcti_reg_2.hp_mode = ptr->VipNewIDcti_ctl2.hp_mode;
	ich1_newdcti_reg_2.vlp_mode= ptr->VipNewIDcti_ctl2.vlp_mode;
	ich1_newdcti_reg_2.median_mode= ptr->VipNewIDcti_ctl2.median_mode;
	ich1_newdcti_reg_2.blending_mode = ptr->VipNewIDcti_ctl2.blending_mode;

	scaler_rtd_outl(IDCTI_I_DCTI_CTRL_2_reg, ich1_newdcti_reg_2.regValue);

//Ctl3
	ich1_newdcti_reg_3.regValue= scaler_rtd_inl(IDCTI_I_DCTI_CTRL_3_reg);

	ich1_newdcti_reg_3.maxminlen= ptr->VipNewIDcti_ctl3.maxminlen;
	ich1_newdcti_reg_3.rate_tran= ptr->VipNewIDcti_ctl3.rate_tran;
	ich1_newdcti_reg_3.th_tran= ptr->VipNewIDcti_ctl3.th_tran;
	ich1_newdcti_reg_3.region_mode= ptr->VipNewIDcti_ctl3.region_mode;
	ich1_newdcti_reg_3.rate_smooth= ptr->VipNewIDcti_ctl3.rate_smooth;
	ich1_newdcti_reg_3.th_smooth= ptr->VipNewIDcti_ctl3.th_smooth;

	scaler_rtd_outl(IDCTI_I_DCTI_CTRL_3_reg, ich1_newdcti_reg_3.regValue);

//Ctl4
	ich1_newdcti_reg_4.regValue= scaler_rtd_inl(IDCTI_I_DCTI_CTRL_4_reg);

	ich1_newdcti_reg_4.rate_align_weight= ptr->VipNewIDcti_ctl4.rate_align_weight;
	ich1_newdcti_reg_4.rate_align= ptr->VipNewIDcti_ctl4.rate_align;
	ich1_newdcti_reg_4.th_align= ptr->VipNewIDcti_ctl4.th_align;
	ich1_newdcti_reg_4.align_mingain= ptr->VipNewIDcti_ctl4.align_mingain;
	ich1_newdcti_reg_4.th_gtran= ptr->VipNewIDcti_ctl4.th_gtran;

	scaler_rtd_outl(IDCTI_I_DCTI_CTRL_4_reg, ich1_newdcti_reg_4.regValue);

//Ctl5
	ich1_newdcti_reg_5.regValue= scaler_rtd_inl(IDCTI_I_DCTI_CTRL_5_reg);

	ich1_newdcti_reg_5.statistic_en= ptr->VipNewIDcti_ctl5.statistic_en;
	ich1_newdcti_reg_5.th_uvmindiff= ptr->VipNewIDcti_ctl5.th_uvmindiff;
	ich1_newdcti_reg_5.rate_stair2= ptr->VipNewIDcti_ctl5.rate_stair2;
	ich1_newdcti_reg_5.rate_stair1= ptr->VipNewIDcti_ctl5.rate_stair1;
	ich1_newdcti_reg_5.th_stair= ptr->VipNewIDcti_ctl5.th_stair;
	ich1_newdcti_reg_5.th_align_weight= ptr->VipNewIDcti_ctl5.th_align_weight;

	scaler_rtd_outl(IDCTI_I_DCTI_CTRL_5_reg, ich1_newdcti_reg_5.regValue);

//Ctl6
	ich1_newdcti_reg_6.regValue= scaler_rtd_inl(IDCTI_I_DCTI_CTRL_6_reg);

	ich1_newdcti_reg_6.statistic_hist= ptr->VipNewIDcti_ctl6.statistic_hist;
	ich1_newdcti_reg_6.rate_engsync= ptr->VipNewIDcti_ctl6.rate_engsync;
	ich1_newdcti_reg_6.rate_uvmindiff= ptr->VipNewIDcti_ctl6.rate_uvmindiff;
	ich1_newdcti_reg_6.th_engsync= ptr->VipNewIDcti_ctl6.th_engsync;

	scaler_rtd_outl(IDCTI_I_DCTI_CTRL_6_reg, ich1_newdcti_reg_6.regValue);

//Ctl7
	ich1_newdcti_reg_7.regValue= scaler_rtd_inl(IDCTI_I_DCTI_CTRL_7_reg);

	ich1_newdcti_reg_7.debug_shiftbit= ptr->VipNewIDcti_ctl7.debug_shiftbit;
	ich1_newdcti_reg_7.debug_mode= ptr->VipNewIDcti_ctl7.debug_mode;
	ich1_newdcti_reg_7.high_bound= ptr->VipNewIDcti_ctl7.high_bound;
	ich1_newdcti_reg_7.low_bound= ptr->VipNewIDcti_ctl7.low_bound;

	scaler_rtd_outl(IDCTI_I_DCTI_CTRL_7_reg, ich1_newdcti_reg_7.regValue);


}


//JZ Mac3
void drvif_color_dnewdcti_table_isr(DRV_VipNewDDcti_Table* ptr, unsigned char offset_gain)
{
	color_sharp_shp_dcti_ctrl_1_RBUS dm_dcti_REG_1;
	color_sharp_shp_dcti_ctrl_2_RBUS dm_dcti_REG_2;
	color_sharp_shp_dcti_ctrl_3_RBUS dm_dcti_REG_3;
	color_sharp_shp_dcti_ctrl_4_RBUS dm_dcti_REG_4;
	color_sharp_shp_dcti_ctrl_5_RBUS dm_dcti_REG_5;
	color_sharp_shp_dcti_ctrl_6_RBUS dm_dcti_REG_6;
	color_sharp_shp_dcti_ctrl_7_RBUS dm_dcti_REG_7;

	if(!ptr)
		return;

//ctl1

	dm_dcti_REG_1.regValue= scaler_rtd_inl(COLOR_SHARP_SHP_DCTI_CTRL_1_reg);

	dm_dcti_REG_1.data_sel = ptr->VipNewDDcti_ctl1.data_sel;
	dm_dcti_REG_1.maxlen = ptr->VipNewDDcti_ctl1.maxlen;
	dm_dcti_REG_1.psmth = ptr->VipNewDDcti_ctl1.psmth;
	dm_dcti_REG_1.lpmode = ptr->VipNewDDcti_ctl1.lp_mode;
	dm_dcti_REG_1.engdiv = ptr->VipNewDDcti_ctl1.engdiv;
	dm_dcti_REG_1.offdiv= ptr->VipNewDDcti_ctl1.offdiv;
	dm_dcti_REG_1.uvgain= ptr->VipNewDDcti_ctl1.uvgain;
	dm_dcti_REG_1.dcti_mode= ptr->VipNewDDcti_ctl1.dcti_mode;
	dm_dcti_REG_1.uvalign_en= ptr->VipNewDDcti_ctl1.uvalign_en;
	dm_dcti_REG_1.cur_sel = ptr->VipNewDDcti_ctl1.cur_sel;

	scaler_rtd_outl(COLOR_SHARP_SHP_DCTI_CTRL_1_reg, dm_dcti_REG_1.regValue);

//ctl2
	dm_dcti_REG_2.regValue= scaler_rtd_inl(COLOR_SHARP_SHP_DCTI_CTRL_2_reg);

	dm_dcti_REG_2.uvsync_en= ptr->VipNewDDcti_ctl2.uvsync_en;
	dm_dcti_REG_2.tran_mode= ptr->VipNewDDcti_ctl2.tran_mode;
	dm_dcti_REG_2.rate_steep= ptr->VipNewDDcti_ctl2.rate_steep;
	dm_dcti_REG_2.th_steep= ptr->VipNewDDcti_ctl2.th_steep;
	dm_dcti_REG_2.th_coring= ptr->VipNewDDcti_ctl2.th_coring;
	dm_dcti_REG_2.steep_mode= ptr->VipNewDDcti_ctl2.steep_mode;
	dm_dcti_REG_2.hp_mode = ptr->VipNewDDcti_ctl2.hp_mode;
	dm_dcti_REG_2.vlp_mode= ptr->VipNewDDcti_ctl2.vlp_mode;
	dm_dcti_REG_2.median_mode= ptr->VipNewDDcti_ctl2.median_mode;
	dm_dcti_REG_2.blending_mode = ptr->VipNewDDcti_ctl2.blending_mode;

	scaler_rtd_outl(COLOR_SHARP_SHP_DCTI_CTRL_2_reg, dm_dcti_REG_2.regValue);

//ctl3
	dm_dcti_REG_3.regValue= scaler_rtd_inl(COLOR_SHARP_SHP_DCTI_CTRL_3_reg);

	dm_dcti_REG_3.maxminlen= ptr->VipNewDDcti_ctl3.maxminlen;
	dm_dcti_REG_3.rate_tran= ptr->VipNewDDcti_ctl3.rate_tran;
	dm_dcti_REG_3.th_tran= ptr->VipNewDDcti_ctl3.th_tran;
	dm_dcti_REG_3.region_mode= ptr->VipNewDDcti_ctl3.region_mode;
	dm_dcti_REG_3.rate_smooth= ptr->VipNewDDcti_ctl3.rate_smooth;
	dm_dcti_REG_3.th_smooth= ptr->VipNewDDcti_ctl3.th_smooth;

	scaler_rtd_outl(COLOR_SHARP_SHP_DCTI_CTRL_3_reg, dm_dcti_REG_3.regValue);

//ctl4
	dm_dcti_REG_4.regValue= scaler_rtd_inl(COLOR_SHARP_SHP_DCTI_CTRL_4_reg);

	dm_dcti_REG_4.rate_align_weight= ptr->VipNewDDcti_ctl4.rate_align_weight;
	dm_dcti_REG_4.rate_align= ptr->VipNewDDcti_ctl4.rate_align;
	dm_dcti_REG_4.th_align= ptr->VipNewDDcti_ctl4.th_align;
	dm_dcti_REG_4.align_mingain= ptr->VipNewDDcti_ctl4.align_mingain;
	dm_dcti_REG_4.th_gtran= ptr->VipNewDDcti_ctl4.th_gtran;

	scaler_rtd_outl(COLOR_SHARP_SHP_DCTI_CTRL_4_reg, dm_dcti_REG_4.regValue);

//ctl5
	dm_dcti_REG_5.regValue= scaler_rtd_inl(COLOR_SHARP_SHP_DCTI_CTRL_5_reg);

	dm_dcti_REG_5.th_uvmindiff= ptr->VipNewDDcti_ctl5.th_uvmindiff;
	dm_dcti_REG_5.rate_stair2= ptr->VipNewDDcti_ctl5.rate_stair2;
	dm_dcti_REG_5.rate_stair1= ptr->VipNewDDcti_ctl5.rate_stair1;
	dm_dcti_REG_5.th_stair= ptr->VipNewDDcti_ctl5.th_stair;
	dm_dcti_REG_5.th_align_weight= ptr->VipNewDDcti_ctl5.th_align_weight;

	scaler_rtd_outl(COLOR_SHARP_SHP_DCTI_CTRL_5_reg, dm_dcti_REG_5.regValue);

//ctl6
	dm_dcti_REG_6.regValue= scaler_rtd_inl(COLOR_SHARP_SHP_DCTI_CTRL_6_reg);

	dm_dcti_REG_6.rate_engsync= ptr->VipNewDDcti_ctl6.rate_engsync;
	dm_dcti_REG_6.rate_uvmindiff= ptr->VipNewDDcti_ctl6.rate_uvmindiff;
	dm_dcti_REG_6.th_engsync= ptr->VipNewDDcti_ctl6.th_engsync;

	scaler_rtd_outl(COLOR_SHARP_SHP_DCTI_CTRL_6_reg, dm_dcti_REG_6.regValue);

//ctl7
	dm_dcti_REG_7.regValue= scaler_rtd_inl(COLOR_SHARP_SHP_DCTI_CTRL_7_reg);
	dm_dcti_REG_7.debug_shiftbit= ptr->VipNewDDcti_ctl7.debug_shiftbit;
	dm_dcti_REG_7.debug_mode= ptr->VipNewDDcti_ctl7.debug_mode;

	scaler_rtd_outl(COLOR_SHARP_SHP_DCTI_CTRL_7_reg, dm_dcti_REG_7.regValue);
}

//=====================================================
void drvif_color_DCTI_for_ColorBar(unsigned char color_bar_flag,unsigned char colorbar_score)
{
	static unsigned char pre_status=0, pre_offset_gain=0,pre_i_table_select=0,pre_d_table_select;
	char which_source=(RPC_system_info_structure_table->VIP_source);
	unsigned char i_table_select=0 , d_table_select=0, status=0, offset_gain=0;

	DRV_VipNewIDcti_Table* I_DCTI_Table = NULL;
	DRV_VipNewDDcti_Table* D_DCTI_Table = NULL;
	DRV_VipNewDcti_auto_adjust *VipNewDcti_auto_adjust = NULL;

	if((which_source>=VIP_QUALITY_TV_NTSC && which_source<=VIP_QUALITY_TV_PAL) )
		return; // rf don't enter color bar detected
	if((color_bar_flag==true) || (DynamicOptimizeSystem[191]==1)||(DynamicOptimizeSystem[193]==1)||(DynamicOptimizeSystem[194]==1)||(DynamicOptimizeSystem[192]==1)||(DynamicOptimizeSystem[196]==1))//change 132 -->192 for color bar condition//for_LG_demo
 	{

 		/*
 		if((which_source>=VIP_QUALITY_CVBS_NTSC) && (which_source<=VIP_QUALITY_TV_PAL))
		{
			i_table_select = 8;
			d_table_select = 8;
		}

		else if((which_source>=VIP_QUALITY_TV_NTSC) && (which_source<=VIP_QUALITY_TV_PAL))
		{
			i_table_select = 9;
			d_table_select = 9;
		}
		*/

		if((which_source>=VIP_QUALITY_HDMI_480I && which_source<=VIP_QUALITY_HDMI_576P)||(which_source>=VIP_QUALITY_YPbPr_480I && which_source<=VIP_QUALITY_YPbPr_576P))
		{
			i_table_select = 8;
			d_table_select = 8;
		}
		/*
		else if((which_source>=VIP_QUALITY_HDMI_720P && which_source<=VIP_QUALITY_HDMI_1080P)||(which_source>=VIP_QUALITY_YPbPr_720P && which_source<=VIP_QUALITY_YPbPr_1080P))
		{
			i_table_select = 8;
			d_table_select = 8;
		}
		*/
		else
		{
			i_table_select = system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.iDcti_table_select;
			d_table_select = system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.dDcti_table_select;
		}
	}
	else
	{
		i_table_select = system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.iDcti_table_select;
		d_table_select = system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.dDcti_table_select;
	}



	i_table_select = (i_table_select>=DCTI_TABLE_LEVEL_MAX)?(DCTI_TABLE_LEVEL_MAX-1):i_table_select;
	d_table_select = (d_table_select>=DCTI_TABLE_LEVEL_MAX)?(DCTI_TABLE_LEVEL_MAX-1):d_table_select;

	VipNewDcti_auto_adjust = &(system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.auto_adjust_Table[i_table_select]);
	if(VipNewDcti_auto_adjust->VipNewDcti_auto_setting.table_mode_flag == 1 )
	{
		offset_gain = 0;
		i_table_select = drvif_color_DCTI_auto_compute(VipNewDcti_auto_adjust,colorbar_score);
	}
	else
	{
		offset_gain = drvif_color_DCTI_auto_compute(VipNewDcti_auto_adjust,colorbar_score);
	}

	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ColorBar==TRUE))
	{
		ROSPrintf("offset_gain=%d,colorbar_score=%d\n ",offset_gain,colorbar_score);
		ROSPrintf("i_table_select=%d\n ",i_table_select);
	}

	if((pre_offset_gain!=offset_gain) || (pre_i_table_select!=i_table_select) || (pre_d_table_select!=d_table_select))
	{
		//ROSPrintf("pre_offset_gain=%d,offset_gain=%d\n ",pre_offset_gain,offset_gain);
		//ROSPrintf("pre_i_table_select=%d,i_table_select=%d\n ",pre_i_table_select,i_table_select);
		//ROSPrintf("pre_d_table_select=%d,d_table_select=%d\n ",pre_d_table_select,d_table_select);
		status=1;
	}
	else
	{
		status=0;
	}

	if(status==pre_status)
	{
		return;
	}
	else
	{
		//ROSPrintf("status=%d,pre_status=%d\n ",status,pre_status);
	}

	pre_status = status;
	pre_offset_gain = offset_gain;
	pre_i_table_select = i_table_select;
	pre_d_table_select = d_table_select;


	I_DCTI_Table = &(system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.iDcti_Table[i_table_select]);
	D_DCTI_Table = &(system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.dDcti_Table[d_table_select]);


	if(system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.iDcti_table_select<DCTI_TABLE_LEVEL_MAX)	//bypass=255
		drvif_color_inewdcti_table_isr(I_DCTI_Table,offset_gain);
	if(system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.dDcti_table_select<DCTI_TABLE_LEVEL_MAX)	//bypass=255
		drvif_color_dnewdcti_table_isr(D_DCTI_Table,offset_gain);

	#if 0
		static unsigned int print_cnt_JZ = 0;
		if(print_cnt_JZ % 100 == 0)
		ROSPrintf("[DCTI] vd= %d,dd=%d, ii=%d, fg=%d, src=%d\n",
		vdcti_level, dcti_level, idcti_level,color_bar_flag,which_source);
		print_cnt_JZ ++;
	#endif
}

unsigned char drvif_color_DCTI_auto_compute(DRV_VipNewDcti_auto_adjust* ptr,unsigned char colorbar_score)
{
	int i =0,score=0;
	unsigned char low_bound=0,up_bound=0,output_level=0;
	unsigned char index=0;

	low_bound = ptr->VipNewDcti_auto_setting.low_bound;
	up_bound = ptr->VipNewDcti_auto_setting.up_bound;
	score = colorbar_score - low_bound;
	if(score<0)
	{
		score=0;
	}

	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ColorBar==TRUE))
	{
		ROSPrintf("score=%d, up_bound=%d\n",score,up_bound);
	}

	if( score <= up_bound )
	{
		for(i=0;i<DCTI_TABLE_LEVEL_MAX;i++)
		{
			if(score>=(ptr->th_table[i]))
			{
				index = i ;
			}

			if(i<(DCTI_TABLE_LEVEL_MAX-1))
			{
				if((ptr->th_table[i+1])<(ptr->th_table[i]))
				{
					if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ColorBar==TRUE))
					{
						ROSPrintf("Warring!!!,th[%d]=%d > th[%d]=%d\n",i,i+1,(ptr->th_table[i]),(ptr->th_table[i+1]));
					}
					return 0;
				}
			}

			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ColorBar==TRUE))
			{
				ROSPrintf("check i=%d,index=%d \n",i,index);
			}
		}

		if(index >= 9)
		{
			output_level = ptr->level_table[9];
		}
		else if (index==0)
		{
			if((ptr->th_table[0])==0)
			{
				output_level = (ptr->level_table[0]);
			}
			else
			{
				if( score<=(ptr->th_table[0]) )
				{
					if((ptr->level_table[0])==0)
					{
						output_level = (ptr->level_table[0]);
					}
					else
					{
						output_level = ((score-0)*((ptr->level_table[0])-0))/((ptr->th_table[0])-0);
					}

				}
				else
				{
					output_level = (ptr->level_table[0])+((score-(ptr->th_table[0]))*((ptr->level_table[1])-(ptr->level_table[index])))/((ptr->th_table[1])-(ptr->th_table[0]));
				}
			}
		}
		else
		{
			if(((ptr->th_table[index])-(ptr->th_table[index-1]))==0)
			{
				output_level = ptr->level_table[index];
			}
			else
			{
				output_level = (ptr->level_table[index])+(((score-(ptr->level_table[index]))*((ptr->level_table[index+1])-(ptr->level_table[index])))/((ptr->th_table[index+1])-(ptr->th_table[index])));
			}

		}
	}
	else
	{
		output_level = ptr->level_table[DCTI_TABLE_LEVEL_9];

	}

	return output_level;

}

void drvif_color_DELAY_for_ColorBar(unsigned char color_bar_flag,unsigned char colorbar_score)
{
	unsigned char which_source=(RPC_system_info_structure_table->VIP_source);
	static unsigned int ori_uv_delay =0;
	unsigned char m_nDelayValue =0;
	unsigned char cur_source =0;
	static unsigned char pre_source =255;
	color_dlcti_uv_delay_RBUS uv_delay_reg;


	SLR_VIP_TABLE *m_Share_Memory_VIP_TABLE_Struct= scaler_GetShare_Memory_VIP_TABLE_Struct();
        if(m_Share_Memory_VIP_TABLE_Struct==NULL){
		return;
	}

	m_nDelayValue=m_Share_Memory_VIP_TABLE_Struct->VIP_QUALITY_Coef[which_source][VIP_QUALITY_FUNCTION_UV_DELAY];
	cur_source=(RPC_system_info_structure_table->VIP_source);


	uv_delay_reg.regValue=rtd_inl(COLOR_DLCTI_UV_Delay_reg);

	if(pre_source !=cur_source)
	{
		ori_uv_delay = uv_delay_reg.regValue;

	}
	pre_source = cur_source;

	if((color_bar_flag)==true )
 	{


		if(which_source==VIP_QUALITY_CVBS_NTSC)
		{

			uv_delay_reg.u_delay=25>>4;
			uv_delay_reg.v_delay=25%16;
		}

	}
	else
	{
		uv_delay_reg.u_delay=m_nDelayValue>>4;
		uv_delay_reg.v_delay=m_nDelayValue%16;
	}

	if (which_source==VIP_QUALITY_HDMI_480I)
	{
		if((color_bar_flag)==true )
			uv_delay_reg.regValue = 5;
		else
			uv_delay_reg.regValue = ori_uv_delay;
	}



	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ColorBar==TRUE))
	{
		ROSPrintf("u_delay = %d\n ",uv_delay_reg.u_delay);
		ROSPrintf("v_delay = %d\n ",uv_delay_reg.v_delay);
	}



	rtd_outl(COLOR_DLCTI_UV_Delay_reg,uv_delay_reg.regValue);




}

int drvif_Get_Purecolor_Weighting()
{
	return pure_color_weighting;
}


void scalerVIP_NetFlix_Smooth_TimingChange_for_RTNR()
{
	di_im_di_rtnr_control_RBUS im_di_rtnr_control_reg;
	_RPC_clues* RPC_SmartPic_clue=NULL;

	RPC_SmartPic_clue = scaler_GetShare_Memory_RPC_SmartPic_Clue_Struct();

	if(RPC_SmartPic_clue == NULL) {
		if(MA_print_count%360==0)
			ROSPrintf("scalerVIP_NetFlix_Smooth_TimingChange_for_RTNR Share Memory PQ data ERROR,SmartPic_clue = %p, \n",RPC_SmartPic_clue);
		return;
	}
	im_di_rtnr_control_reg.regValue = rtd_inl(DI_IM_DI_RTNR_CONTROL_reg);

	if(RPC_SmartPic_clue->S_NetFlix_info.NetFlix_smooth_Toggle_info.TimingChange_flag_for_RTNR==1)
	{
		im_di_rtnr_control_reg.cp_rtnr_y_enable = 0;
		im_di_rtnr_control_reg.cp_rtnr_c_enable = 0;
		RPC_SmartPic_clue->S_NetFlix_info.NetFlix_smooth_Toggle_info.TimingChange_flag_for_RTNR =0;
	}
	else
	{
		im_di_rtnr_control_reg.cp_rtnr_y_enable = 1;
		if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_400_mode)
			im_di_rtnr_control_reg.cp_rtnr_c_enable = 0x0;
		else
			im_di_rtnr_control_reg.cp_rtnr_c_enable = 1;
	}
}

void scalerVIP_Set_Decontour(void)
{
    unsigned char meanHistY = 0;
    unsigned char meanSat = 0;
	static unsigned char NaturalImage = 1;
	_clues* SmartPic_clue=NULL;
	static unsigned int blend_lowbd_org = 10;
	ipq_decontour_de_contour_ctrl_RBUS decontour_ctrl_reg;

	SmartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

    if (SmartPic_clue == NULL) {
    	return;
    }

    meanHistY = SmartPic_clue->Hist_Y_Mean_Value;
    meanSat = SmartPic_clue->sat_ratio_mean_value;

	#if 0	//log
	{
	static unsigned int rc = 0;
	rc++;
	if (rc % 120 == 0)
		rtd_pr_vpq_isr_info("roger, meanHistY[%d], meanSat[%d], GMV[%d]\n", meanHistY, meanSat, scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio());
	}
	#endif
	
	if((meanHistY >= 20)&&(meanSat <=50)&&(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio() <= 10)) { ////FOR GRAY BAR, add HistY >=20 condition for lotte water
		if (NaturalImage) { 
			decontour_ctrl_reg.regValue = rtd_inl(IPQ_DECONTOUR_De_Contour_CTRL_reg);
			blend_lowbd_org = decontour_ctrl_reg.decont_blend_lowbd;
			decontour_ctrl_reg.decont_blend_lowbd = 255;
			rtd_outl(IPQ_DECONTOUR_De_Contour_CTRL_reg, decontour_ctrl_reg.regValue);
			drvif_color_Set_I_De_Contour_DB_Apply();
		}
		NaturalImage = 0;
	} else {
		if (NaturalImage == 0) {
			decontour_ctrl_reg.regValue = rtd_inl(IPQ_DECONTOUR_De_Contour_CTRL_reg);
			decontour_ctrl_reg.decont_blend_lowbd = blend_lowbd_org;
			rtd_outl(IPQ_DECONTOUR_De_Contour_CTRL_reg, decontour_ctrl_reg.regValue);
			drvif_color_Set_I_De_Contour_DB_Apply();
		}
		NaturalImage = 1;
	}
}

static void scalerVIP_Adaptive_Demura_Scale_By_APL(_system_setting_info* sysInfo, _RPC_system_setting_info* RPC_sysInfo, _clues* smartPic_clue, _RPC_clues* RPC_smartPic_clue)
{
	demura_demura_control_RBUS demura_demura_control_reg;
	unsigned int APL;
	unsigned char i;

	if (DeMura_TBL.DeMura_Adaptive_TBL.adaptiveScale_En != 1)
		return;	/* disable dynamic scale*/

	demura_demura_control_reg.regValue = IoReg_Read32(DEMURA_DEMURA_CONTROL_reg);

	APL = SmartPic_clue->Adapt_DCC_Info.Adapt_Mean_Dev_info[DCC_Tool_mean];

	for (i=0;i<VIP_Demura_Adaptive_Scale_LV_NUM;i++)
		if (APL < DeMura_TBL.DeMura_Adaptive_TBL.adaptiveScale_APL_th[i])
			break;
	if (i >= VIP_Demura_Adaptive_Scale_LV_NUM)
		i = VIP_Demura_Adaptive_Scale_LV_NUM - 1;

	demura_demura_control_reg.demura_table_scale = DeMura_TBL.DeMura_Adaptive_TBL.adaptiveScale[i];

	IoReg_Write32(DEMURA_DEMURA_CONTROL_reg, demura_demura_control_reg.regValue);

	//rtd_pr_vpq_isr_info("Adaptive Demura, i=%d, APL=%d \n", i, APL);

}

void DI_SW_Detection_set_check_ISR(void)
{
	extern unsigned char DI_SW_Detection_set_check;

	if (DI_SW_Detection_set_check) {
		if (DI_i2r_db_reg_ctl_get_reg_sm_size_change_sw_detect_en(IoReg_Read32(DI_i2r_db_reg_ctl_reg))) {
			if (!Get_DI_RTNR_DMA_force2D_Status()) {
				IoReg_ClearBits(DI_i2r_db_reg_ctl_reg, DI_i2r_db_reg_ctl_reg_sm_size_change_sw_detect_en_mask);
				DI_SW_Detection_set_check = 0;
			}
		} else
			DI_SW_Detection_set_check = 0;
	}
}

//*************************************************************
//== DCC, DCR, AUTO_NR and RTK_DEMO MODE=======
//*************************************************************
void scalerVIP_colorMaAutoISR(SCALER_CHANNEL_NUM channel)
{
	//IoReg_SetBits(0xb802b000,_BIT4);

	SLR_VIP_TABLE* Share_Memory_VIP_TABLE_Struct = NULL;
	_clues* SmartPic_clue=NULL;
	_RPC_clues* RPC_SmartPic_clue=NULL;
	_system_setting_info* system_info_structure_table = NULL;
	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;
	_VIP_NetFlix_smooth_Toggle_info* VCPU_smooth_Toggle_info = NULL;


	unsigned int en_test;
	//unsigned char black_white_detected;
	//unsigned char flag_chessboard=0;
	static unsigned char isEvenFrame = 0;
	char InputSourceDetFlag;
	//unsigned char isColorBar_flag = 0;	//Elsie 20130521
	static unsigned char preSource = 255;
	DRV_RPC_AutoMA_Flag *AutoMA_Struct = NULL;
	unsigned char DCTI_auto_table_select=0;
	DRV_VipNewDcti_auto_adjust *VipNewDcti_auto_adjust_table = NULL;
	di_im_di_control_RBUS im_di_control_reg;
	di_im_di_film_new_function_main_RBUS di_im_di_film_new_function_main_reg;
	rgbw_d_rgbw_adaptwhite_2_RBUS rgbw_d_rgbw_adaptwhite_2_REG;
	void *VIP_customer_TBL=NULL;
	int data_sel = 0;
	static int hist[9][8] = {0};

	static unsigned char isTNRXC = 1;


	MA_print_count++;//for debug print

//******************************************************************
//******************Pattern detector debug !!**************************
//******************************************************************
//--------------------------------------------------------------------------//
	Share_Memory_VIP_TABLE_Struct = scaler_GetShare_Memory_VIP_TABLE_Struct();
	SmartPic_clue = scaler_GetShare_Memory_SmartPic_Clue_Struct();
	RPC_SmartPic_clue = scaler_GetShare_Memory_RPC_SmartPic_Clue_Struct();
	system_info_structure_table = scaler_GetShare_Memory_system_setting_info_Struct();
	RPC_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();
	VIP_customer_TBL = scalerVIP_Get_Share_Memory_VIP_TABLE_Custom_Struct();

	if((Share_Memory_VIP_TABLE_Struct == NULL) || (SmartPic_clue == NULL)|| (RPC_SmartPic_clue == NULL) || (system_info_structure_table == NULL) || (VIP_customer_TBL==NULL) || (RPC_system_info_structure_table==NULL))
	{
		if(MA_print_count%360==0)
		{
			ROSPrintf("\n====Share Memory PQ data ERROR====\n\n");
			ROSPrintf(" Mem0 = %p\n, Mem1 = %p\n",Share_Memory_VIP_TABLE_Struct, SmartPic_clue);
			ROSPrintf(" Mem2 = %p\n, Mem3 = %p\n",system_info_structure_table,VIP_customer_TBL);
			ROSPrintf(" Mem4 = %p\n, Mem5 = %p\n",RPC_system_info_structure_table, RPC_SmartPic_clue);
			ROSPrintf("\n===============================\n\n");
		}
		return;
	}


	//get VCPU smoothtoggle info
	VCPU_smooth_Toggle_info = &(RPC_SmartPic_clue->S_NetFlix_info.NetFlix_smooth_Toggle_info);

	if( (VCPU_smooth_Toggle_info->m_film_clear) && !(VCPU_smooth_Toggle_info->smooth_toggle_update_flag) ){
		di_im_di_film_new_function_main_reg.regValue = rtd_inl(DI_IM_DI_FILM_NEW_FUNCTION_MAIN_reg);
		im_di_control_reg.regValue = rtd_inl(DI_IM_DI_CONTROL_reg);
		VCPU_smooth_Toggle_info->m_film_clear = 0;
		im_di_control_reg.film_enable = VCPU_smooth_Toggle_info->m_film;
		im_di_control_reg.write_enable_5=1;
		di_im_di_film_new_function_main_reg.fw_film_en = VCPU_smooth_Toggle_info->g_fw_film_en;
		VCPU_smooth_Toggle_info->m_film = 0;
		rtd_outl(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
		rtd_outl(DI_IM_DI_FILM_NEW_FUNCTION_MAIN_reg, di_im_di_film_new_function_main_reg.regValue);
	}



#ifdef ENABLE_VIP_TABLE_CHECKSUM
/*======== checksum========*/
	vip_color_dcc_d_dcc_fwused_1_RBUS dcc_fwused_1_reg;
	dcc_fwused_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_FWUSED_1_reg);
	VIP_table_crc_value_ISR video_vip_table_crc;

	static unsigned char vit_table_checksum_flag = 1;
	if(vit_table_checksum_flag)
	{

		scalerVIP_check_VIPTable_crc(&video_vip_table_crc, Share_Memory_VIP_TABLE_Struct);
		ROSPrintf("[%s %d] VIP_table_checksum: share_memory %x, video_fw %x\n", __FILE__, __LINE__, vip_table_crc_isr->checkSum, video_vip_table_crc.checkSum);
		vit_table_checksum_flag = 0;
	}

	if(dcc_fwused_1_reg.VipInfo == 1 && MA_print_count%360==0)
	{
		scalerVIP_check_VIPTable_crc(&video_vip_table_crc, Share_Memory_VIP_TABLE_Struct);
		ROSPrintf("[%s %d] VIP_table_checksum: share_memory %x, video_fw %x\n", __FILE__, __LINE__, vip_table_crc_isr->checkSum, video_vip_table_crc.checkSum);
	}
/*========================*/
#endif


#if 0
	if(MA_print_count%360==0)
	{
		ROSPrintf("  Input_src_Type= %d \n",system_info_structure_table->Input_src_Type);
		ROSPrintf("  Input_src_Form= %d \n",system_info_structure_table->Input_src_Form);
		ROSPrintf("  Input_VO_Form= %d \n",system_info_structure_table->Input_VO_Form);
		ROSPrintf("  Input_TVD_Form= %d \n",system_info_structure_table->Input_TVD_Form);
		ROSPrintf(" vdc_color_standard = %d \n",system_info_structure_table->vdc_color_standard);
		ROSPrintf("  Timing= %d \n",system_info_structure_table->Timing);
		ROSPrintf("  HDMI_video_format= %d \n",system_info_structure_table->HDMI_video_format);
		ROSPrintf("  VIP_source= %d \n",system_info_structure_table->VIP_source);
	}
#endif

	vd_pwr_ok = scalerVIP_check_vdc_hw_pwr();
	scalerVIP_NetFlix_smoothtoggle_enable(RPC_SmartPic_clue,RPC_system_info_structure_table);//if AP have infomation, have to change status from AP, CSFC 20140704

	scaler_Set_VIP_info_structure(channel,SmartPic_clue);//set information!!priority first!!CSFC 20150319
	AutoMA_Struct = &(system_info_structure_table->S_RPC_AutoMA_Flag);

	scalerVIP_Set_Start_Print_Info();
	scalerVIP_Set_Stop_Polling();
	if(Start_Print.Delay_Time ==0)
	{
		Start_Print.Delay_Time =1;
	}

	#if 0
	if(MA_print_count%360==0)
	{
		ROSPrintf("[Young]ISR flag \n");
		ROSPrintf(" %d  ,%d  ,%d  ,%d  ,%d  ,\n",AutoMA_Struct->AutoMA_En,AutoMA_Struct->AutoMA1_En,AutoMA_Struct->AutoMA2_En,AutoMA_Struct->AutoMA3_En,AutoMA_Struct->AutoMA4_En);
		ROSPrintf(" %d  ,%d  ,%d  ,%d  ,%d  ,\n",AutoMA_Struct->AutoMA_API_CTRL.DI_MiddleWare_Function_En,AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_SMD_weake_move_Detecte_En,AutoMA_Struct->AutoMA_API_CTRL.compute_PQA_level_En,AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Profile_Block_Motion_En,AutoMA_Struct->AutoMA_API_CTRL.noise_estimation_MAD_et_En);
		ROSPrintf(" %d  ,%d  ,%d  ,%d  ,%d  ,\n",AutoMA_Struct->AutoMA_API_CTRL.Y_U_V_Mad_Calculate_En,AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_ET_Y_MOTION_En,AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_FMV_HMCbin_hist_ratio_Cal_En,AutoMA_Struct->AutoMA_API_CTRL.drvif_color_ClueGather_En,AutoMA_Struct->AutoMA_API_CTRL.skin_tone_Level_detect_En);
		ROSPrintf(" %d  ,%d  ,%d  ,%d  ,%d  ,\n",AutoMA_Struct->AutoMA_API_CTRL.pure_color_detect_En,AutoMA_Struct->AutoMA_API_CTRL.motion_purecolor_concentric_circles_detect_En,AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_rtnr_noise_measure_En,AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_color_noise_mpeg_detect_En,AutoMA_Struct->AutoMA_API_CTRL.DI_detect_Champagnet_En);
		ROSPrintf(" %d  ,%d  ,%d  ,%d  ,%d  ,\n",AutoMA_Struct->AutoMA_API_CTRL.motion_concentric_circles_detect_En,AutoMA_Struct->AutoMA_API_CTRL.motion_purecolor_concentric_circles_detect_En,AutoMA_Struct->AutoMA_API_CTRL.motion_move_concentric_circles_detect_En,AutoMA_Struct->AutoMA_API_CTRL.drvif_color_colorbar_dectector_by_SatHueProfile_En,AutoMA_Struct->AutoMA_API_CTRL.black_white_pattern_detect_En);
		ROSPrintf(" %d  ,%d  ,%d  ,%d  ,%d  ,\n",AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Fade_Detection_En,AutoMA_Struct->AutoMA_API_CTRL.histogram_new_III_dcc_EN,AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_DCR_EN,AutoMA_Struct->AutoMA_API_CTRL.p_film_detection_En,AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_PQ_Adaptive_Table_En);
		ROSPrintf(" %d  ,%d  ,%d  ,%d  ,%d  ,\n",AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_MA_IEdgeSmooth_En,AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Dynamic_Vertical_NR_En,AutoMA_Struct->AutoMA_API_CTRL.drvif_color_DCTI_for_ColorBar_En,AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_color_noise_mpeg_apply_En,AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_RTNR_correctionbit_period_En);

	}
	#endif

	//ini setting
	if(system_info_structure_table->ISR_Printf_Flag.Delay_Time==0)
		system_info_structure_table->ISR_Printf_Flag.Delay_Time=200;



	#if 0
	if(MA_print_count%360==0)
	{
		ROSPrintf("\n==== system_info_structure_table->HDR_flag == %d  ===	\n",system_info_structure_table->HDR_flag );
	}
	if(system_info_structure_table->HDR_flag == 1)
	{
		AutoMA_Struct->AutoMA2_En =FALSE ;
		AutoMA_Struct->AutoMA5_En = FALSE;
		AutoMA_Struct->AutoMA6_En = FALSE;


		AutoMA_Struct->AutoMA_API_CTRL.DI_MiddleWare_Function_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_SMD_weake_move_Detecte_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Profile_Block_Motion_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_FMV_HMCbin_hist_ratio_Cal_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.drvif_color_ClueGather_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.skin_tone_Level_detect_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.pure_color_detect_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.DI_detect_Champagnet_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.motion_concentric_circles_detect_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.motion_purecolor_concentric_circles_detect_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.motion_move_concentric_circles_detect_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.drvif_color_colorbar_dectector_by_SatHueProfile_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.black_white_pattern_detect_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Fade_Detection_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_DCR_EN = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.p_film_detection_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_MA_IEdgeSmooth_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Dynamic_Vertical_NR_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.drvif_color_DCTI_for_ColorBar_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_color_noise_mpeg_apply_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_RTNR_correctionbit_period_En = FALSE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_DI_Dynamic_VDNoise_FrameMotion_En = FALSE;
	}
	else
	{
		AutoMA_Struct->AutoMA2_En =TRUE ;
		AutoMA_Struct->AutoMA5_En = TRUE;
		AutoMA_Struct->AutoMA6_En = TRUE;

		AutoMA_Struct->AutoMA_API_CTRL.DI_MiddleWare_Function_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_SMD_weake_move_Detecte_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Profile_Block_Motion_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_FMV_HMCbin_hist_ratio_Cal_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.drvif_color_ClueGather_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.skin_tone_Level_detect_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.pure_color_detect_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.DI_detect_Champagnet_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.motion_concentric_circles_detect_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.motion_purecolor_concentric_circles_detect_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.motion_move_concentric_circles_detect_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.drvif_color_colorbar_dectector_by_SatHueProfile_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.black_white_pattern_detect_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Fade_Detection_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_DCR_EN = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.p_film_detection_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_MA_IEdgeSmooth_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Dynamic_Vertical_NR_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.drvif_color_DCTI_for_ColorBar_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_color_noise_mpeg_apply_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_RTNR_correctionbit_period_En = TRUE;
		AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_DI_Dynamic_VDNoise_FrameMotion_En = TRUE;
	}
	#endif
//--------------------------------------------------------------------------//

#if 0 //VIP_100_4 project
scalerVIP_100_4_Bright_Plus();
#endif

#if 1
	//scalerVIP_HDR_apply();
	//scalerVIP_HDR_clk_reset(system_info_structure_table);
#else
	/*unsigned int lineCnt_in = rtd_inl(VODMA_VODMA_LINE_ST_reg) & 0xfff;
	unsigned int vo_vend = (rtd_inl(VODMA_VODMA_V1VGIP_VACT1_reg) >> 16) & 0xfff;*/
	dolby_v_top_top_d_buf_RBUS top_d_buf_reg;
	dm_hdr_dm_double_buffer_ctrl_RBUS dm_double_buffer_ctrl_reg;

	if(system_info_structure_table->HDR_info.HDR_Setting_Status == 1){
		/*// front porch
		if (lineCnt_in < (vo_vend-1)) {
			 //ROSPrintf("\n== OFF lineCnt_in =%d ,vend	=%d \n", lineCnt_in, (vo_vend - 1));
		}else*/
		{
			top_d_buf_reg.regValue = rtd_inl(DOLBY_V_TOP_TOP_D_BUF_reg);
			top_d_buf_reg.dolby_double_apply = 1;

			dm_double_buffer_ctrl_reg.regValue = rtd_inl(DM_HDR_DM_Double_Buffer_CTRL_reg);
			dm_double_buffer_ctrl_reg.dm_db_apply = 1;

			rtd_outl(DOLBY_V_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);
			rtd_outl(DM_HDR_DM_Double_Buffer_CTRL_reg, dm_double_buffer_ctrl_reg.regValue);

			system_info_structure_table->HDR_info.HDR_Setting_Status = 0;

		}
	}
#endif

//*********************************************************************************************
//***************** ISR Detector Information start*****************************************
//*********************************************************************************************

//---------------------------------------------------------------------------------------//
// === RGBW histogram for adaptive white (henry 20170629)
//---------------------------------------------------------------------------------------//

	rgbw_d_rgbw_adaptwhite_2_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_2_reg);

	if(rgbw_d_rgbw_adaptwhite_2_REG.hist_en == 1)
	{
		rgbw_d_rgbw_adaptwhite_2_REG.hist_shift_sel = 0;
		rgbw_d_rgbw_adaptwhite_2_REG.hist_mode_sel = 1;

		for(data_sel = 0; data_sel < 9; data_sel++)
		{
		    rgbw_d_rgbw_adaptwhite_2_REG.hist_data_sel = data_sel;
		    IoReg_Write32(RGBW_D_RGBW_ADAPTWHITE_2_reg, rgbw_d_rgbw_adaptwhite_2_REG.regValue);

		    fwif_color_set_RGBW_histogram(hist[data_sel]);
		}

		fwif_color_set_RGBW_adaptive_white(hist);

	}

//---------------------------------------------------------------------------------------//
// === source and mode information
//---------------------------------------------------------------------------------------//
	SmartPic_clue->Remmping_Src_Timing_index = drvif_color_check_source_remmping(SLR_MAIN_DISPLAY);
//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
// === DI SIS Initial and Update
//---------------------------------------------------------------------------------------//
	if(AutoMA_Struct->AutoMA_API_CTRL.DI_MiddleWare_Function_En==TRUE)
	{
	scalerVIP_DI_MiddleWare_StatusInit(RPC_system_info_structure_table->VIP_source);
	scalerVIP_DI_MiddleWare_HistUpdate((SmartPic_clue->Y_Main_Hist));
	scalerVIP_DI_MiddleWare_StatusUpdate();
	scalerVIP_DI_MiddleWare_SetStatus(STATUS_FADING, SmartPic_clue->Fade_flag);
	scalerVIP_DI_MiddleWare_SetStatus(STATUS_STILL_CONCENTRIC_CIRCLES, (SmartPic_clue->neckin_concentric_flag)); // Sony W1 CG NO.13
	scalerVIP_DI_MiddleWare_SetStatus(STATUS_MOVING_CAN, scalerVideo_Get_VD_MiddleWare(VD_STATUS_MOVING_CAN));   // 20151014, willy, use API to get VD status
	//scalerVIP_DI_MiddleWare_SetStatus(STATUS_MOVING_CAN, MovingCan_From_ScalerVideo); // flag from scalerVideo
	}
	if (Start_Print.DI_1==TRUE)
	{
		scalerVIP_DI_MiddleWare_DebugMode(STATUS_HMC, debug_print_delay_time);
		scalerVIP_DI_MiddleWare_DebugMode(STATUS_HMCNR, debug_print_delay_time);
	}
	if (Start_Print.DI_2==TRUE)
	{
		scalerVIP_DI_MiddleWare_DebugMode(STATUS_SPAN, debug_print_delay_time);
		scalerVIP_DI_MiddleWare_DebugMode(STATUS_SLOWMOTION, debug_print_delay_time);
		scalerVIP_DI_MiddleWare_DebugMode(STATUS_MOTION, debug_print_delay_time);
		scalerVIP_DI_MiddleWare_DebugMode(STATUS_SMD, debug_print_delay_time);
	}
	if (Start_Print.Snr==TRUE)
	{
		scalerVIP_DI_MiddleWare_DebugMode(STATUS_NOISE, debug_print_delay_time);
	}
	//for debug
	if((system_info_structure_table->ISR_Printf_Flag.Flag1 & _BIT0) != 0)
	{
		scalerVIP_DI_MiddleWare_DebugMode(STATUS_PAN, system_info_structure_table->ISR_Printf_Flag.Delay_Time);
	}
	if(AutoMA_Struct->AutoMA2_En ==TRUE){

		switch(RPC_system_info_structure_table->VIP_source)
		{
			case VIP_QUALITY_CVBS_NTSC:
			case VIP_QUALITY_CVBS_PAL:
			case VIP_QUALITY_YPbPr_480I:
			//case VIP_QUALITY_YPbPr_576I:
			case VIP_QUALITY_HDMI_480I:
			case VIP_QUALITY_HDMI_576I:
				if( DynamicOptimizeSystem[139] != 1 )
					scalerVIP_DI_MiddleWare_PAN_Apply(); // rord, 20151005, for code flow issue

				scalerVIP_DI_MiddleWare_HMC_Apply();
 				break;



 			case VIP_QUALITY_HDMI_1080I:
				scalerVIP_DI_MiddleWare_PAN_Apply(); // rord, 20151005, for code flow issue
				scalerVIP_DI_MiddleWare_HMC_Apply();

			default:

 				break;

		}

	//mark scalerVIP_DI_MiddleWare_FILM_Apply due to HQV film 2:2 is fail 20170209
	/*
	if(!(RPC_system_info_structure_table->Project_ID == VIP_Project_ID_TV006 && !RPC_system_info_structure_table->PQ_demo_flag)) //20150819 remove mark due to HD 2:2 fail if do scalerVIP_DI_MiddleWare_FILM_Apply
	{
		scalerVIP_DI_MiddleWare_FILM_Apply();
	}
	*/

//	crt_sys_dclkss_RBUS dclkss_reg;
/* //rord.tsao mark 2015/0710
	di_im_di_hmc_pan_control_RBUS di_im_di_hmc_pan_control_reg;
	di_im_di_hmc_pan_control_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_CONTROL_reg);
	unsigned char hmcnr_fw_en = ((di_im_di_hmc_pan_control_reg.dummy18024660_31_24 & 0x80)>>7); // bit31
	switch(RPC_system_info_structure_table->VIP_source)
	{
		case VIP_QUALITY_CVBS_NTSC:
		case VIP_QUALITY_CVBS_PAL:
		case VIP_QUALITY_YPbPr_480I:
		case VIP_QUALITY_YPbPr_576I:
		case VIP_QUALITY_HDMI_480I:
		case VIP_QUALITY_HDMI_576I:
			scalerVIP_DI_MiddleWare_HMCNR_Apply(hmcnr_fw_en);

		break;
		case VIP_QUALITY_HDMI_1080I:
			scalerVIP_DI_MiddleWare_HMCNR_Apply(hmcnr_fw_en);
			break;

		default:
			scalerVIP_DI_MiddleWare_HMCNR_Apply(hmcnr_fw_en);
			break;

	}
*/
	//scalerVIP_DI_MiddleWare_MCNR_Ctrl(0);

	scalerVIP_DI_MiddleWare_MCNR_Ctrl2(0);

	//scalerVIP_BTR_Detecte();
	scalerVIP_SMD_weake_move_Detecte();

// chen 170522
//	scalerVIP_DI_MiddleWare_ZOOMMOTION_Apply(16);
// end chen 170522
	scalerVIP_DI_MiddleWare_SLOWMOTION_Apply(16);
	scalerVIP_DI_MiddleWare_FADING_Apply(5);
	scalerVIP_DI_MiddleWare_SPECIAL_Apply((SmartPic_clue->Remmping_Src_Timing_index));
	scalerVIP_DI_MiddleWare_SMD_Apply();
	}
#if 0
	//============= DI detect test ================= //
	if((MA_print_count%Start_Print.Delay_Time==0)&&(Start_Print.ID_Pattern_139==TRUE))
	{
		ROSPrintf("STATUS_MOTION=%d,S_Noise=%d,S_Noise_R=%d,T_Noise=%d,T_Noise_R=%d,\n",
			scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION),scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL),
			scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL_RELIABLE),scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_TEMPORAL_NOISE_LEVEL),
			scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_TEMPORAL_NOISE_RELIABLE));
	}

	/*if((MA_print_count%Start_Print.Delay_Time==0)&&(Start_Print.Reverse_5==TRUE))
	{
		scalerVIP_DI_MiddleWare_DebugMode(STATUS_NOISE, Start_Print.Delay_Time);
	}*/


	/*if((MA_print_count%Start_Print.Delay_Time==0)&&(Start_Print.ID_Pattern_147==TRUE))
	{
		ROSPrintf("input_src_form=%d,Input_src_Type=%d,Input_TVD_Form=%d,T_Noise=%d,Input_VO_Form=%d,\n",
			system_info_structure_table->Input_src_Form, system_info_structure_table->Input_src_Type, system_info_structure_table->Input_TVD_Form
			,system_info_structure_table->Input_VO_Form);
	}*/
#endif
//============= DI detect test ================= //

#if 0
if ((MA_print_count % 30) == 0)
{
	ROSPrintf("[SIS System-1]MOTION:%d, HMC: %d, PAN: %d:%d, VPAN: %d, ZOOM: %d, SLOW:%d\n", scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION), scalerVIP_DI_MiddleWare_GetStatus(STATUS_HMC),
		scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN), (scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN_LEVEL)-16), scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN),
		scalerVIP_DI_MiddleWare_GetStatus(STATUS_ZOOMMOTION),	scalerVIP_DI_MiddleWare_GetStatus(STATUS_SLOWMOTION));

	ROSPrintf("[SIS System-2]FADE:%d, HF:%d/%d, FILM: %d, SCC:%d\n", scalerVIP_DI_MiddleWare_GetStatus(STATUS_FADING), scalerVIP_DI_MiddleWare_GetStatus(STATUS_HIGHFREQUENCY_LEVEL), HIGHFREQUENCY_SCALE,
		scalerVIP_DI_MiddleWare_GetStatus(STATUS_FILM), scalerVIP_DI_MiddleWare_GetStatus(STATUS_STILL_CONCENTRIC_CIRCLES));

	ROSPrintf("[SIS System-3]NR_SPT_R: %d, NR_SPT_LV: %d, NR_TMP_R: %d, NR_TMP_LV: %d\n", scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL_RELIABLE),
		scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL), scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_TEMPORAL_NOISE_RELIABLE),
		scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_TEMPORAL_NOISE_LEVEL));
}

if (scalerVIP_DI_MiddleWare_GetStatus(STATUS_SCENECHANGE))
{
	ROSPrintf("[SIS System] Scene Change Detected!!\n");
	scalerVIP_DI_MiddleWare_DebugMode(STATUS_SCENECHANGE, 1);
}
#endif

//for lg demo BW not enough, default disable!! sync. from Mac3 CSFC 20150312
#if 0//LG_Demo_flag
	od_od_cap_status_RBUS od_cap_status_reg;
	od_od_cap_ctl0_RBUS cap_ctl0_reg;
	od_od_ctrl0_RBUS od_ctrl0_reg;

	od_ctrl0_reg.regValue = rtd_inl(OD_OD_CTRL0_reg);
	od_cap_status_reg.regValue = rtd_inl(OD_OD_CAP_STATUS_reg);

	if ((od_cap_status_reg.cap_fifo_overflow == 1)&&(od_ctrl0_reg.od_ctrl==1))
	{
		rtd_clearbits(OD_OD_CAP_CTL0_reg, _BIT24);
		rtd_setbits(OD_OD_CAP_CTL0_reg, _BIT24);

		rtd_setbits(OD_OD_CAP_STATUS_reg, _BIT2);

		ROSPrintf("[OD] Bandwidth capture error, workaround solution!!!\n");
	}

	cap_ctl0_reg.regValue = rtd_inl(OD_OD_CAP_CTL0_reg);
	if(cap_ctl0_reg.cap_en==0)
		rtd_setbits(OD_OD_CAP_CTL0_reg, _BIT24);
#endif


//---------------------------------------------------------------------------------------//
// === Check DI DMA error flag and disable OD
// NOTE: need to check and fix, CSFC mark 20140429
//---------------------------------------------------------------------------------------//
//if ((MA_print_count % 20) == 0)
	//scalerVIP_OD_CheckBandwidth();

//====== Custom Lead =========
//---------------------------------------------------------------------------------------//
	if(RPC_system_info_structure_table->Project_ID == VIP_Project_ID_TV015)
	{
#ifdef ISR_TV015_DI_FUNCTION_ENABLE
	    scalerVIP_TV015_Lead();
#endif
	}




//---------------------------------------------------------------------------------------//
//==read profile (Elsie 20130521 seperated from still motion judge)==
//---------------------------------------------------------------------------------------//
#ifdef ISR_AMBILIGHT_ENABLE
	if(m_AmbilightMode == 1)
		SmartPic_clue->Profile_Cal_INFO.ProfileFlag_Y = 0;
	else
#endif
	{
		#if 1 //new
		scalerVIP_scalerColor_profile();
		#else //old
		SmartPic_clue->Profile_Cal_INFO.ProfileFlag_Y = scalerVIP_ReadProfile(SmartPic_clue);
		#endif
	}

	if (BlackDetection_en)
		scalerVIP_PF_BlackDetection();


//---------------------------------------------------------------------------------------//
//==NR detector
//---------------------------------------------------------------------------------------//
	if(system_info_structure_table->Input_src_Form == _SRC_FROM_VDC && vd_pwr_ok)
		SmartPic_clue->VD_Signal_Status_value_avg = VD_noise_Status_sum_avg();  //yuan1024
		//drvif_color_compute_noise_level(SLR_MAIN_DISPLAY); //CSFC mark from 20140429, need sync. to new PQA table
	else
		SmartPic_clue->VD_Signal_Status_value_avg = 0;

	if(AutoMA_Struct->AutoMA_API_CTRL.compute_PQA_level_En==TRUE)
	{
		scalerVIP_compute_PQA_level(Share_Memory_VIP_TABLE_Struct, SmartPic_clue, system_info_structure_table);
	}
//---------------------------------------------------------------------------------------//
//================= Noise estimation (20131105 jimmy.lin) =========================//
//---------------------------------------------------------------------------------------//
#ifdef ISR_AMBILIGHT_ENABLE
	if( (m_AmbilightMode != 1 )&&(AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Profile_Block_Motion_En==TRUE)) // profile unavailable
#else
	if(AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Profile_Block_Motion_En==TRUE) // profile unavailable
#endif
	{
		//Return : MOTION_NOISE_TYPE
		SmartPic_clue->block_motion_status = scalerVIP_Profile_Block_Motion(SmartPic_clue);
	}

	//Return:MOTION_NOISE_TYPE
	if((RPC_system_info_structure_table->Project_ID != VIP_Project_ID_TV003 )&& (AutoMA_Struct->AutoMA_API_CTRL.noise_estimation_MAD_et_En==TRUE))//for TV003, not need use this function
	{
		if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_MOTION_RTNR]==1)
		SmartPic_clue->MAD_motion_status = scalerVIP_noise_estimation_MAD_et((SmartPic_clue->SCALER_DISP_INFO.IphActWid_H), (SmartPic_clue->SCALER_DISP_INFO.IpvActLen_V));
		else
			SmartPic_clue->MAD_motion_status =0;

	}
//---------------------------------------------------------------------------------------//
	if(Enable_Hist_MAD_Calculate)
	{
		if (AutoMA_Struct->AutoMA_API_CTRL.Y_U_V_Mad_Calculate_En==TRUE)
		scalerVIP_Y_U_V_Mad_Calculate(Share_Memory_VIP_TABLE_Struct, SmartPic_clue, system_info_structure_table/*, RPC_system_info_structure_table*/);
		/* use "scalerVIP_ET_Y_MOTION_En" to control HDR setting status, elieli*/
		/*if (AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_ET_Y_MOTION_En==TRUE)
		scalerVIP_ET_Y_MOTION();*/
	}
	scalerVIP_HMCNR_statistic();
//---------------------------------------------------------------------------------------//
//=====Y, Hue, Sat Histogram detector information====
//---------------------------------------------------------------------------------------//
	scalerVIP_YHist_HueHist_Ratio_Calculate();
	scalerVIP_Sat_Hist_Calculate();
	SmartPic_clue->saturationflag = scalerVIP_Saturation_Detection(); //remove this for video book sony screen flash on/off
//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
//=====DI FMV HMC MV Histogram detector information====
//---------------------------------------------------------------------------------------//
	if (AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_FMV_HMCbin_hist_ratio_Cal_En==TRUE)
	scalerVIP_FMV_HMCbin_hist_ratio_Cal(Share_Memory_VIP_TABLE_Struct, SmartPic_clue, system_info_structure_table);
//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
//=====Motion detector=====
//=====only for interlace signal motion detector =======
//---------------------------------------------------------------------------------------//
{
	di_im_di_rtnr_control_RBUS im_di_rtnr_control_reg;
	im_di_rtnr_control_reg.regValue = rtd_inl(DI_IM_DI_RTNR_CONTROL_reg);
	if(im_di_rtnr_control_reg.cp_rtnr_progressive != 1)
	{
		//if(system_info_structure_table->Input_src_Form == _SRC_FROM_VDC)
		//	SmartPic_clue->di_motion_status = scalerVideo_MotionDecision();
		//else
			SmartPic_clue->di_motion_status = scalerVideo_MotionDecision();
	}
	if(AutoMA_Struct->AutoMA_API_CTRL.drvif_color_ClueGather_En ==TRUE)
		drvif_color_ClueGather(channel, SmartPic_clue);//for getting MAD et and film motion information
}
//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
//==read from input source detection (Jimmy 20140213 added for Sirius) ==
//== This function can detector status for ( SD in -> HD out )
//== NOTE : Need to fix and test this function, CSFC mark on 20140429 ==
//---------------------------------------------------------------------------------------//
switch(RPC_system_info_structure_table->VIP_source)
{

	case VIP_QUALITY_DTV_576I:
		scalerVIP_DI_GMV_AVG_for_film();
		break;
	case VIP_QUALITY_HDMI_1080I:
	case VIP_QUALITY_DTV_1080I:
	case VIP_QUALITY_YPbPr_1080I:
	case VIP_QUALITY_PVR_1080I:
		InputSourceDetFlag = scalerVIP_ReadSrcDet(2, 20, 0);

		//rtd_pr_vpq_isr_info("\n h0: %d ,h1 %d ,h2 %d ",Step_Hist[0],Step_Hist[1],Step_Hist[2]);
		//if(rtd_inl(0xb802e4f4)==14)
		//	rtd_pr_vpq_isr_info("\n step_count: %d ,average %d  ",step_count);

		if(step_count<BadUpScalingThreshold)
		{
				// sd2hd by player
			//scalerVIP_color_DRV_SNR_Veritcal_NR(&vlpf_table[1]);
			BadUpScalingThreshold = BADUP_SCALING_LEAVE;
		}
		else
		{
			//ROSPrintf("\n SD2HD: 0 ,step_count %d  ",step_count);
			//scalerVIP_color_DRV_SNR_Veritcal_NR(&vlpf_table[0]);

		}
		scalerVIP_Teeth_detcted();
		break;
	case VIP_QUALITY_HDMI_1080P:
	case VIP_QUALITY_YPbPr_1080P:
		InputSourceDetFlag = scalerVIP_ReadSrcDet(2, 20, 0);

		break;

	default:

		break;

}

	//if( InputSourceDetFlag == SUCCESS )
		//scalerVIP_Input_Source_Detection(2);
//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
//==skin tone detector
//---------------------------------------------------------------------------------------//
	//skin_tone_detect(_CHANNEL1, scaler_disp_info[_CHANNEL1].DispWid, scaler_disp_info[_CHANNEL1].DispLen);
	SmartPic_clue->skintoneflag=skin_tone_found;
	if (AutoMA_Struct->AutoMA_API_CTRL.skin_tone_Level_detect_En==TRUE)
	skin_tone_Level_detect(_CHANNEL1,
		Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DISP_WID),
		Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DISP_LEN));
//---------------------------------------------------------------------------------------//

//=======================================================
//===  Pure Color Detection== < if pure_color ==1, DCC.DCL_B.DCL_W all off >
//=======================================================
	//if(Sony_DCC_API_ON || TV_057_style_ON)
	//	pure_color = pure_color_detect_Sony(0, scaler_disp_info[SLR_MAIN_DISPLAY].DispWid, scaler_disp_info[SLR_MAIN_DISPLAY].DispLen);
	//else
		//pure_color = pure_color_detect_Sony(0, scaler_disp_info[SLR_MAIN_DISPLAY].DispWid, scaler_disp_info[SLR_MAIN_DISPLAY].DispLen);//for_LG_demo
	if (AutoMA_Struct->AutoMA_API_CTRL.pure_color_detect_En==TRUE){
		SmartPic_clue->PureColor = pure_color_detect(0, SmartPic_clue->SCALER_DISP_INFO.DispWid, SmartPic_clue->SCALER_DISP_INFO.DispLen);
	}
	if ((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Dcc_2==TRUE))
	{
		ROSPrintf("==== pure_color =%d=====\n",(SmartPic_clue->PureColor));
		ROSPrintf("==== Saturation_detect_flag =%d=====\n",SmartPic_clue->saturationflag);
	}
//---------------------------------------------------------------------------------------//
//==Color Histogram Detector==
//---------------------------------------------------------------------------------------//
	en_test = 0;
	if (AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_colorHistDetect_En==TRUE)
		scalerVIP_colorHistDetect(en_test);
//---------------------------------------------------------------------------------------//


//---------------------------------------------------------------------------------------//
//==Profile Motion detector==
//---------------------------------------------------------------------------------------//
	if (Profile_Motion_Detect_ON)
	{
		if ((SmartPic_clue->Profile_Cal_INFO.ProfileFlag_Y) == 1)
			scalerVIP_Profile_Motion_Detect(channel, SmartPic_clue);
	}
//---------------------------------------------------------------------------------------//

//*********************************************************************************************
//~~~~~ ISR Detector Information end~~~~
//*********************************************************************************************



//*********************************************************************************************
//***************************** detection ******************************************************
//*********************************************************************************************
	//scalerVIP_rtnr_noise_measure();

	// 20170714 jimmy, toggle dexc by USB playback speed
	/* move to "scalerVIP_Dynamic_DeXCXL_CTRL()" */
	//scalerVIP_Set_DeXC_OnOff_by_PlaySpeed(RPC_system_info_structure_table);


//---------------------------------------------------------------------------------------//
//============AUTO MA 5 : MPEG NR Function (detection)===========
//---------------------------------------------------------------------------------------//
	if(AutoMA_Struct->AutoMA5_En==TRUE){
		scalerVIP_color_noise_mpeg_detect();
		scalerVIP_color_noise_mpeg_v_detect();
	}
//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
//=== DI detect champange ====
//---------------------------------------------------------------------------------------//
      if (AutoMA_Struct->AutoMA_API_CTRL.DI_detect_Champagnet_En==TRUE)
	 SmartPic_clue->Champagne_flag = DI_detect_Champagne(SmartPic_clue->SCALER_DISP_INFO.DispWid, SmartPic_clue->SCALER_DISP_INFO.DispLen);
//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
//===concentric circles detection===
//---------------------------------------------------------------------------------------//
	//if (AutoMA_Struct->AutoMA_API_CTRL.motion_concentric_circles_detect_En==TRUE)
	SmartPic_clue->neckin_concentric_flag = motion_concentric_circles_detect(SmartPic_clue->SCALER_DISP_INFO.DispWid, SmartPic_clue->SCALER_DISP_INFO.DispLen);
	if (AutoMA_Struct->AutoMA_API_CTRL.motion_purecolor_concentric_circles_detect_En==TRUE)
	SmartPic_clue->PureColor_neckin_concentric_flag = motion_purecolor_concentric_circles_detect(SmartPic_clue->SCALER_DISP_INFO.DispWid, SmartPic_clue->SCALER_DISP_INFO.DispLen);

	if (AutoMA_Struct->AutoMA_API_CTRL.motion_move_concentric_circles_detect_En==TRUE)
	SmartPic_clue->Motion_concentric_flag = motion_move_concentric_circles_detect(SmartPic_clue->SCALER_DISP_INFO.DispWid, SmartPic_clue->SCALER_DISP_INFO.DispLen);

//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
//================colorbar detection (Elsie 20130521)=============(modify CSFC 201501)
//---------------------------------------------------------------------------------------//
	//isColorBar_flag = scalerVIP_ColorbarDetector();
	//SmartPic_clue->Colorbar_flag = drvif_color_colorbar_dectector(SmartPic_clue);
	DCTI_auto_table_select = system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.iDcti_table_select;
	VipNewDcti_auto_adjust_table = &(system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.auto_adjust_Table[DCTI_auto_table_select]);
	if (AutoMA_Struct->AutoMA_API_CTRL.drvif_color_colorbar_dectector_by_SatHueProfile_En==TRUE)
	SmartPic_clue->colorbar_score= drvif_color_colorbar_dectector_by_SatHueProfile(SmartPic_clue);//20141226, CSFC
	if((SmartPic_clue->colorbar_score)>(VipNewDcti_auto_adjust_table->VipNewDcti_auto_setting.low_bound))
	{
		if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ColorBar==TRUE))
		{
			ROSPrintf(" low_bound=%d !!\n",(VipNewDcti_auto_adjust_table->VipNewDcti_auto_setting.low_bound));
			ROSPrintf(" Enable DCTI enhance for colobar !!\n");
		};

		SmartPic_clue->Colorbar_flag = true;
	}
	else
	{
		SmartPic_clue->Colorbar_flag = false;
	}

//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
//==================black-white detection (only CVBS)=======================
//---------------------------------------------------------------------------------------//
	if (AutoMA_Struct->AutoMA_API_CTRL.black_white_pattern_detect_En==TRUE)
	SmartPic_clue->CVBS_black_white_falg= black_white_pattern_detect();
//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
//================= Fade Detection (20131105 jimmy.lin) ==========================//
//---------------------------------------------------------------------------------------//
	if(((SmartPic_clue->Profile_Cal_INFO.ProfileFlag_Y)==SUCCESS)&&(AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Fade_Detection_En==TRUE))
	{
		SmartPic_clue->Fade_flag = scalerVIP_Fade_Detection();
	}

//*********************************************************************************************
//~~~~~~end detection~~~~~~
//*********************************************************************************************


//*********************************************************************************************
//**************** Some Basic algo. function  *********************************
//*********************************************************************************************

//---------------------------------------------------------------------------------------//
//== DCC ==
//---------------------------------------------------------------------------------------//
	if (Stop_Polling.Stop_Dcc==FALSE) //Stop_polling =TRUE : STOP DCC ,
	{
		if (AutoMA_Struct->AutoMA_API_CTRL.histogram_new_III_dcc_EN==TRUE && RPC_system_info_structure_table->Project_ID != VIP_Project_ID_TV006)
		{
			if(COLOR_HISTOGRAM_LEVEL ==128){
				histogram_new_III_dcc(channel,7, 7);
			} else {
				histogram_new_III_dcc(channel,5, 7);
			}
		}
	}
//---------------------------------------------------------------------------------------//

#if 0
//scalerVIP_DI_Dynamic_Status_Adjust_by_Global_Motion(motion_level_flag);
// VD save initial register value when source change, willy 20150715
if(system_info_structure_table->Input_src_Form == _SRC_FROM_VDC && vd_pwr_ok)
{
    scalerVideo_NoistStatusUpdate(); // move to VGIPISR?
}

//if(RPC_system_info_structure_table->Project_ID == VIP_Project_ID_TV006 &&
//   ( (RPC_system_info_structure_table->VIP_source==VIP_QUALITY_CVBS_NTSC)||
//   (RPC_system_info_structure_table->VIP_source==VIP_QUALITY_CVBS_PAL)) ){
if(RPC_system_info_structure_table->Project_ID == VIP_Project_ID_TV006 ){

    scalerVIP_Identify_Pattern_forLGM2();
    scalerVIP_Set_Parameter_forLGM2(1);
}
else
{
    scalerVIP_Identify_Pattern_forLG_K2L();
    scalerVIP_Set_Parameter_forLG_K2L();
}
#endif

//---------------------------------------------------------------------------------------//
//=== DCR Function ===
//---------------------------------------------------------------------------------------//

	//if(MA_print_count%1000==0)
		//ROSPrintf(" yuan,project_ID=%d,%d \n",project_ID,VIP_Project_ID_TV003);
	//if(project_ID ==PRJ_TV003)
	//scalerVIP_DCR();

if((Stop_Polling.Stop_Dcr== FALSE)&&(AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_DCR_EN==TRUE)) //Stop_polling =TRUE : STOP  DCR,
{
	//if(MA_print_count%250==0)
	//	rtd_pr_vpq_isr_emerg("scalerVIP_DCR_EN\n");

	if(system_info_structure_table->RTK_DCR_Enable==1)
	{
		scalerVIP_DCR();
	}
}
//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
//=== film detector ===
//---------------------------------------------------------------------------------------//
#if 0 //mark for non use
if((Stop_Polling.Stop_Film_Mode_Detector==FALSE)&&(AutoMA_Struct->AutoMA_API_CTRL.p_film_detection_En==TRUE)) //Stop_polling[4] =TRUE : STOP  Film_Mode
{
	p_film_detection();
}
#endif
//---------------------------------------------------------------------------------------//

//*********************************************************************************************
//~~~ end some basic function ~~~~
//*********************************************************************************************



//*********************************************************************************************
//***************************** Apply : inherit from detection  block *********************************
//*********************************************************************************************
//---------------------------------------------------------------------------------------//
//====== RTNR correction bit period ======
//---------------------------------------------------------------------------------------//
	if(AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_RTNR_correctionbit_period_En==TRUE) //Stop_polling =TRUE : STOP  Auto NR function ,
		scalerVIP_RTNR_correctionbit_period();
//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
//======== Auto NR function ===========
//---------------------------------------------------------------------------------------//
	if((Stop_Polling.Stop_Auto_Nr==FALSE)&&(AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_PQ_Adaptive_Table_En==TRUE)) //Stop_polling =TRUE : STOP  Auto NR function ,
	{
		// set rtnrC running flag for scalerVideo.c "void LG_XCRemove_by_RTNR_C(UINT8 format, UINT8 MA_motion)", reset condition,  elieli
		scalerVIP_Reset_PQA_TBL_ContentCheck_Flag(&(SmartPic_clue->PQA_ISR_Cal_Info.ContentCheck_Flag[0]));

		scalerVIP_PQ_Adaptive_Table(PQA_MODE_WRITE, Share_Memory_VIP_TABLE_Struct, SmartPic_clue, system_info_structure_table);
		scalerVIP_PQ_Adaptive_Table(PQA_MODE_OFFSET, Share_Memory_VIP_TABLE_Struct, SmartPic_clue, system_info_structure_table);
		if((RPC_SmartPic_clue->S_NetFlix_info.Netflix_Enable)==1)	//Netflix
		{
			scalerVIP_NetFlix_Smooth_TimingChange_for_RTNR();
		}
	}

	//if(MA_print_count%250==0)
		//ROSPrintf("PQA_CC=%d,%d\n",SmartPic_clue->PQA_ISR_Cal_Info.ContentCheck_Flag[0],SmartPic_clue->PQA_ISR_Cal_Info.ContentCheck_Flag[1]);

	if(AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_Dynamic_Vertical_NR_En==TRUE)
    scalerVIP_Dynamic_Vertical_NR();

//---------------------------------------------------------------------------------------//
//====================AUTO MA 1 start for DCTI=================================
//---------------------------------------------------------------------------------------//
if((Stop_Polling.Stop_Dcti_for_colorbar==FALSE)&&(AutoMA_Struct->AutoMA_API_CTRL.drvif_color_DCTI_for_ColorBar_En==TRUE)) //Stop_polling =TRUE : STOP  DCTI_for colorbar
{
	if(AutoMA_Struct->AutoMA1_En)
	{
		//drvif_color_DELAY_for_ColorBar(SmartPic_clue->Colorbar_flag,SmartPic_clue->colorbar_score);
		//drvif_color_DCTI_for_ColorBar(SmartPic_clue->Colorbar_flag,SmartPic_clue->colorbar_score);
	}
}
//---------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------//
//====================Auto MA 5 application: for MPEG NR====================
//---------------------------------------------------------------------------------------//
	if((Stop_Polling.Stop_Noise_Mpeg==FALSE)&&(AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_color_noise_mpeg_apply_En==TRUE)) //Stop_polling =TRUE : STOP  noise_mpeg,
	{
		//Elsie 20130425: detect/apply seperation
		if(AutoMA_Struct->AutoMA5_En)
		{
			if(RPC_system_info_structure_table->Project_ID != VIP_Project_ID_TV003)//for TV003, Need to remove when bug fix!!
			{
				scalerVIP_color_noise_mpeg_apply();
				scalerVIP_color_noise_mpeg_v_apply();
			}
		}
	}
//---------------------------------------------------------------------------------------//
    if(RPC_system_info_structure_table->Project_ID == VIP_Project_ID_TV015)
    {
#ifdef ISR_TV015_DI_FUNCTION_ENABLE
		scalerVIP_TV015_Adjust();
#endif
    }

//---------------------------------------------------------------------------------------//
//====== DI Dynamic VDNoise adjustment ======
//---------------------------------------------------------------------------------------//
if((Stop_Polling.Stop_VD_Noise==FALSE)&&(AutoMA_Struct->AutoMA_API_CTRL.scalerVIP_DI_Dynamic_VDNoise_FrameMotion_En==TRUE)) //Stop_polling =TRUE : STOP  Dynamic VD_Noise
{
    if( (RPC_system_info_structure_table->VIP_source==VIP_QUALITY_TV_NTSC) ||
        (RPC_system_info_structure_table->VIP_source==VIP_QUALITY_TV_NTSC443) ||
        (RPC_system_info_structure_table->VIP_source==VIP_QUALITY_TV_PAL) ||
        (RPC_system_info_structure_table->VIP_source==VIP_QUALITY_TV_SECAN) ||
        (RPC_system_info_structure_table->VIP_source==VIP_QUALITY_TV_PAL_M)  )
        /*
    if( Scaler_InputSrcGetMainChType()==_SRC_CVBS ||
        Scaler_InputSrcGetMainChType()==_SRC_TV ||
        Scaler_InputSrcGetMainChType()==_SRC_SV||
        Scaler_InputSrcGetMainChType()==_SRC_SCART)
        */
{
    	if (vd_pwr_ok)
            scalerVIP_DI_Dynamic_VDNoise_FrameMotion();
}
}
//---------------------------------------------------------------------------------------//
//====== DI Dynamic Status adjustment ======
//---------------------------------------------------------------------------------------//


//---------------------------------------------------------------------------------------//
//====== Set Info to MEMC  jzl 20151231======

	scalerVIP_Set_Info_toMEMC();
#ifdef CONFIG_HW_SUPPORT_MEMC
	scalerVIP_Set_MEMC_Info_toVCPU();
#endif
//---------------------------------------------------------------------------------------//

	scalerVIP_Set_Decontour();


#if 1

//if(system_info_structure_table->Project_ID == VIP_Project_ID_TV006 &&
//   ( (system_info_structure_table->VIP_source==VIP_QUALITY_CVBS_NTSC)||
//   (system_info_structure_table->VIP_source==VIP_QUALITY_CVBS_PAL)) ){
if(RPC_system_info_structure_table->Project_ID == VIP_Project_ID_TV006 ){

    scalerVIP_Identify_Pattern_forLGM2();
    scalerVIP_Set_Parameter_forLGM2(1);
}
else
{
    scalerVIP_Identify_Pattern_forLG_K2L();
    scalerVIP_Set_Parameter_forLG_K2L();
}

#endif
//scalerVIP_DI_MCNR_RTNR_bottom_fresh_line_patch(system_info_structure_table->Input_src_Form != _SRC_FROM_VO);// only merlin need

#if 0//CB_flag
			scalerVIP_Identify_Pattern_forCB();
			scalerVIP_Set_Parameter_forCB();
#endif

#ifdef CONFIG_HW_SUPPORT_MEMC
	scalerVIP_IP_OnOff_By_MEMC_Film(RPC_system_info_structure_table); //rord.tsao 2016/0325
#endif //CONFIG_HW_SUPPORT_MEMC

#ifdef CONFIG_HW_SUPPORT_I_DE_XC //mac6 removed de_xcxl
	scalerVIP_Dynamic_DeXCXL_CTRL(RPC_system_info_structure_table, Share_Memory_VIP_TABLE_Struct, RPC_SmartPic_clue);
#endif //CONFIG_HW_SUPPORT_I_DE_XC

	scalerVIP_Adaptive_Demura_Scale_By_APL(system_info_structure_table, RPC_system_info_structure_table, SmartPic_clue, RPC_SmartPic_clue);

	if(preSource != (RPC_system_info_structure_table->VIP_source))
	{
		isTNRXC = (rtd_inl(DI_IM_DI_TNR_XC_CONTROL_reg) & _BIT0);
	}

	if(RPC_SmartPic_clue->SceneChange)
	{
		rtd_clearbits(DI_IM_DI_TNR_XC_CONTROL_reg, _BIT0);	//disable TNR XC when SceneChange

		if(RPC_system_info_structure_table->VIP_source==VIP_QUALITY_CVBS_SECAN)
			rtd_setbits(DI_IM_DI_TNR_XC_CONTROL_reg, _BIT0);

	}
/*	for LG-PAL_SceneChange, marked by nick187 @ 2014/03/15
	else
	{
		rtd_maskl(DI_IM_DI_TNR_XC_CONTROL_reg, 0xfffffffe, isTNRXC);
	}
*/

	//for smoothtoogle sharpness change
	scalerVIP_VO_resolution_change_hack();

//*********************************************************************************************
// ~~~ end apply ~~~~
//*********************************************************************************************

//********DI_SW_Detect***********
	if(RPC_system_info_structure_table->Project_ID == VIP_Project_ID_TV030){
		if(system_info_structure_table->Input_src_Type == _SRC_HDMI){
			if(DI_i2r_db_reg_ctl_get_reg_sm_size_change_sw_detect_en(rtd_inl(DI_i2r_db_reg_ctl_reg)) == 1){
				ROSPrintf("SWdetect=1\n");
				rtd_clearbits(DI_i2r_db_reg_ctl_reg, DI_i2r_db_reg_ctl_reg_sm_size_change_sw_detect_en_mask);//clear _BIT1
			}else{
				if(MA_print_count%500==0)
					ROSPrintf("SWdetect=0\n");
				}
		}
	} else {
		DI_SW_Detection_set_check_ISR();
	}

	if (!(vsc_get_adaptivestream_flag(SLR_MAIN_DISPLAY) && Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC))
		IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_apply_mask);

//*****update pre data ********
	pre_TG45_Flag = TG45_Flag;

	isEvenFrame = (isEvenFrame+1) % 2;
	preSource = (RPC_system_info_structure_table->VIP_source);

	RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.AutoMA_Running_flag = 1;
/*
	if(MA_print_count%360==0)
	{
		ROSPrintf("scalerVIP_colorMaAutoISR End\n");
	}
*/
}

#ifdef ISR_TV015_DI_FUNCTION_ENABLE
void scalerVIP_TV015_Adjust(void)
{

	//DI  chroma error for Grayscale dot crawing  de cvbs adc limitation
	scalerVIP_DI_Chroma_Error_de_DotCrawing();

}
void scalerVIP_DI_Chroma_Error_de_DotCrawing(void)
{
	//Mac7p chromaerror_en function removed, no more use
	/*
	if(RPC_system_info_structure_table->VIP_source == VIP_QUALITY_CVBS_NTSC || RPC_system_info_structure_table->VIP_source == VIP_QUALITY_CVBS_PAL)
	{
		di_im_di_framesobel_statistic_RBUS FrameSobel_statistic_Reg;
		FrameSobel_statistic_Reg.regValue = scaler_rtd_inl(DI_IM_DI_FRAMESOBEL_STATISTIC_reg);
		if((SmartPic_clue->Sat_Main_His_Ratio)[0] >=994)
			FrameSobel_statistic_Reg.chromaerror_en = 0;
		else
			FrameSobel_statistic_Reg.chromaerror_en = 1;
		scaler_rtd_outl(DI_IM_DI_FRAMESOBEL_STATISTIC_reg, FrameSobel_statistic_Reg.regValue);
	}
	*/
}

#endif

#ifdef ISR_AMBILIGHT_ENABLE
void scalerVIP_TV015_Lead(void)
{
	//====== ambilight sensor =========
	//m_AmbilightMode = 1;
	//scalerVIP_AmbiLight(_CHANNEL1);
}

void scalerVIP_AmbiLight(unsigned char display)
{
	// using profile to get YUV data for ambilight sensor, elieli
		static unsigned char color_flag = 0;
		//ROSPrintf(" color_flag =%d,=%d\n",color_flag,flag_profile_read_status);
		if(color_flag == 0)
			flag_profile_read_status = drvif_color_dragon_profile_dectector_ambilight(display, &Profile_Y_H[0], &Profile_Y_V[0], &PF_Trans_Sum, 30, 13, 3);	// Erin tmp 100310
		else if(color_flag == 1)
			flag_profile_read_status = drvif_color_dragon_profile_dectector_ambilight(display, &Profile_U_H[0], &Profile_U_V[0], &PF_Trans_Sum, 30, 13, 3);
		else if(color_flag == 2)
			flag_profile_read_status = drvif_color_dragon_profile_dectector_ambilight(display, &Profile_V_H[0], &Profile_V_V[0], &PF_Trans_Sum, 30, 13, 3);

		//ROSPrintf("color_flag=%d,=%d\n",color_flag,flag_profile_read_status);

		if(color_flag == flag_profile_read_status)
		{
			color_flag = (flag_profile_read_status>=2)? 0: flag_profile_read_status+1;
			if(color_flag == 0)
			{
				scalerVIP_yuv2rgb_convert();
				Profile_RGB_data_flag_H = 1;
				Profile_RGB_data_flag_V = 1;
			}
		}
	//==============================================

}
#endif

//Elsie 20130425: seperated from Auto MA
void scalerVIP_DCR(void)
{
	if(RPC_system_info_structure_table == NULL)
		return;
/* ========================== ===================*/
/* === Get DCR Table form VIP Table ====================== */
/* ============================================== */
	scalerVIP_setDcrTable();
	scalerVIP_Set_DCR_BackLightMappingRange();

/* ========================== ======*/
/* === DCR Driver ====================== */
/* =================================*/
	scalerVIP_DCR_OLD();
//********************************************************************************
}

//============================Elsie 20130425: color mpeg========================
// moved to scalerMpegnr.c
//============================Elsie 20130425: color mpeg end=====================

//=========================== Pure Color Detection ===========================
unsigned char pure_color_detect(unsigned char channel, unsigned int width, unsigned int height)
{
	unsigned char MainDCC_SC = (RPC_SmartPic_clue->SceneChange);
	unsigned int hist_sum;//, hue_hist_sum;
	unsigned int i;//, hue_hist[24];
	unsigned int hist_sum_max=0;
	int max_index=0;//, hue_max_index=0;
	unsigned int pure_color_weighting_of_max_bin_ratio = 0;
	unsigned int pure_color_weighting_of_first_and_second_bin_ratio = 0;
	unsigned int *reg_HistCnt=(SmartPic_clue->Y_Main_Hist);
	int weighting_gap = 0;
	static int purecolor_weighting_buffer[32] = {[0 ... 31]=0x20}; //buffer = 32 frames

	pure_color_weighting = 0;

	hist_sum = 0;

	for(i = 0; i< (COLOR_HISTOGRAM_LEVEL - 1); i++) {
		hist_sum = reg_HistCnt[i]+reg_HistCnt[i+1];

		if(hist_sum>hist_sum_max) {
			max_index = i;
			hist_sum_max = hist_sum;
		}
	}

	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Dcc_2==TRUE)) {
		ROSPrintf("====== pure_color_detect ========\n");
		ROSPrintf("hist_sum_max=%d \n",hist_sum_max);
		ROSPrintf("Total_HistCnt=%d \n",(SmartPic_clue->Hist_Y_Total_sum));
		ROSPrintf("((Total_HistCnt*31)>>5)=%d \n",(((SmartPic_clue->Hist_Y_Total_sum)*31)>>5));
		ROSPrintf("DCC_Histogram_Peak_White=%d \n",(SmartPic_clue->Hist_Peak_White_IRE_Value));
		ROSPrintf("Histogram_Mean_value=%d \n",(SmartPic_clue->Hist_Y_Mean_Value));
	}

	//if the highest 2 bins take higher percentage, higher pure color weighting will be set.

	if( (hist_sum_max > (((SmartPic_clue->Hist_Y_Total_sum)*31)>>5)) )
	{
		if(hist_sum_max >= (SmartPic_clue->Hist_Y_Total_sum*127)>>7)//tolerence some noise
			pure_color_weighting_of_max_bin_ratio = 32;
		else if(hist_sum_max <= (((SmartPic_clue->Hist_Y_Total_sum)*31)>>5))
			pure_color_weighting_of_max_bin_ratio = 0;
		else if(hist_sum_max < ((SmartPic_clue->Hist_Y_Total_sum*127)>>7) && hist_sum_max > (((SmartPic_clue->Hist_Y_Total_sum)*31)>>5))
		{
			weighting_gap = (((SmartPic_clue->Hist_Y_Total_sum*127)>>7) - (((SmartPic_clue->Hist_Y_Total_sum)*31)>>5)) >> 5;

			for( i = 0; i < 32; i++)
			{
				if((((SmartPic_clue->Hist_Y_Total_sum)*31)>>5) + weighting_gap * i <= hist_sum_max
					&& (((SmartPic_clue->Hist_Y_Total_sum)*31)>>5) + weighting_gap * (i+1) > hist_sum_max)
				{
					pure_color_weighting_of_max_bin_ratio = i;
				}
				else if((((SmartPic_clue->Hist_Y_Total_sum)*31)>>5) + weighting_gap * (i+1) <= hist_sum_max)
					pure_color_weighting_of_max_bin_ratio = 32;
			}
		}
	}
	else
	{
		pure_color_weighting_of_max_bin_ratio = 0;
	}

	//if it enter the purecolor above and the highest two bins are closer, it means the 1 bins are not the noise(there are two Y histogram). Then, pure color weighting will be set lower.
	pure_color_weighting_of_first_and_second_bin_ratio = 0;
	if(hist_sum_max > (((SmartPic_clue->Hist_Y_Total_sum)*31)>>5))
	{
		if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Dcc_2==TRUE))
		{
			ROSPrintf("max_index = %d\n", max_index);
			ROSPrintf("reg_HistCnt[max_index] = %d\n", reg_HistCnt[max_index]);
			ROSPrintf("reg_HistCnt[max_index+1] = %d\n", reg_HistCnt[max_index+1]);
		}
		if(reg_HistCnt[max_index] > ((reg_HistCnt[max_index+1]) >> 1))
		{
			if(reg_HistCnt[max_index+1] == 0)
				pure_color_weighting_of_first_and_second_bin_ratio = 32;
			else
				pure_color_weighting_of_first_and_second_bin_ratio = (reg_HistCnt[max_index] / reg_HistCnt[max_index+1]);
		}
		else if(reg_HistCnt[max_index+1] >= ((reg_HistCnt[max_index]) >> 1))
		{
			if(reg_HistCnt[max_index] == 0)
				pure_color_weighting_of_first_and_second_bin_ratio = 32;
			else
				pure_color_weighting_of_first_and_second_bin_ratio = (reg_HistCnt[max_index+1] / reg_HistCnt[max_index]);
		}

		if(pure_color_weighting_of_first_and_second_bin_ratio >= 32)
			pure_color_weighting_of_first_and_second_bin_ratio = 32;
		else if(pure_color_weighting_of_first_and_second_bin_ratio <= 0)
			pure_color_weighting_of_first_and_second_bin_ratio = 0;
	}

	//combine two weighting above together
	pure_color_weighting = (pure_color_weighting_of_max_bin_ratio + pure_color_weighting_of_first_and_second_bin_ratio) >> 1;
	if(pure_color_weighting >= 32)
		pure_color_weighting = 32;
	else if (pure_color_weighting <= 0)
		pure_color_weighting = 0;

	//20171005 hugh: for CVBS, there is some noise that will effect purecolor weighting and lead to flick
	if(MainDCC_SC == 0)
	{
		//update weighting buffer
		int tmp_value = 0;
		for(i = 0; i < 32; ++i)
		{
			tmp_value = purecolor_weighting_buffer[i];
			if(i < 31)
				purecolor_weighting_buffer[i] = purecolor_weighting_buffer[i+1];
			else// i=31
				purecolor_weighting_buffer[i] = pure_color_weighting;
		}
		//smooth weighting by 32 frames
		for(i = 0; i < 32; ++i)
			pure_color_weighting +=  purecolor_weighting_buffer[i];
		pure_color_weighting = pure_color_weighting >> 5;
	}
	else
	{
		//SC == 1, reset pure color weighting buffer
		for(i = 0; i < 32; ++i)
			purecolor_weighting_buffer[i] = pure_color_weighting;
	}
	if(pure_color_weighting >= 32)
		pure_color_weighting = 32;
	else if (pure_color_weighting <= 0)
		pure_color_weighting = 0;

	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Dcc_2==TRUE))
	{
		ROSPrintf("====== pure_color_detect ========\n");
		ROSPrintf("pure_color_weighting_of_max_bin_ratio=%d \n",pure_color_weighting_of_max_bin_ratio);
		ROSPrintf("pure_color_weighting_of_first_and_second_bin_ratio=%d \n",pure_color_weighting_of_first_and_second_bin_ratio);
		ROSPrintf("pure_color_weighting=%d \n",pure_color_weighting);
	}

	if(pure_color_weighting > 0)
		return 1;
	else
		return 0;


}

#ifdef ISR_SONY_DETECT_ENABLE
unsigned char pure_color_detect_Sony(unsigned char channel, unsigned int width, unsigned int height)
{
	unsigned int hist_sum;//, hue_hist_sum;
	unsigned int i;//, hue_hist[24];
	unsigned int hist_sum_max=0;
	//unsigned int hue_hist_sum_max=0;
	int max_index=0;//, hue_max_index=0;
	unsigned int *reg_HistCnt=(SmartPic_clue->Y_Main_Hist);

	if(sat_hist_ratio[0]>850)
		return FALSE;

	hist_sum = 0;
	hist_sum_max=0;
	for(i = 0; i< 23; i++)
	{
		hist_sum = hue_hist_ratio[i]+hue_hist_ratio[i+1];
		if(hist_sum>hist_sum_max)
		{
			max_index = i;
			hist_sum_max = hist_sum;
		}
	}
	if((hist_sum_max < 900))
		return FALSE;

	hist_sum_max=0;
	hist_sum = 0;
	for(i = 0; i< 31; i++)
	{
		hist_sum = reg_HistCnt[i]+reg_HistCnt[i+1];
		if(hist_sum>hist_sum_max)
		{
			max_index = i;
			hist_sum_max = hist_sum;
		}
	}

	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Dcc_2==TRUE))
	{
		ROSPrintf("==== pure_color_detect_Sony======\n");
		ROSPrintf("hist_sum_max=%d \n",hist_sum_max);
		ROSPrintf("Total_HistCnt=%d \n",(SmartPic_clue->Hist_Y_Total_sum));
		ROSPrintf("((Total_HistCnt*15)>>4)=%d \n",(((SmartPic_clue->Hist_Y_Total_sum)*15)>>4));
	}

	//o------ judge the pure color ------o
	if( (hist_sum_max > (((SmartPic_clue->Hist_Y_Total_sum)*15)>>4)) )
		return 1;
	else
		return 0;

}
#endif

void skin_tone_Level_detect(unsigned char channel, unsigned int width, unsigned int height)
{
	unsigned int i;
	unsigned int Total_hue_hist_sum=0, Total_hue_hist_ratio;
	unsigned char max_index=0, secondindex=0;
	unsigned int hue_max_bin_cnt=0,hue_second_bin_cnt=0;
	unsigned char hue_max_bin_ratio=0,hue_second_bin_ratio=0;
	unsigned char hue_zero_bin_counter =0;

	unsigned int hue_his_sum_2_7=0, hue_his_ratio_2_7=0;

	unsigned short Y_his_max_ratio = 0 ;
	unsigned int Y_his_sum_12_18=0, Y_his_ratio_12_18=0;

	static unsigned int skin_tone_count=0;
	unsigned int *reg_HistCnt=(SmartPic_clue->Y_Main_Hist);


	for(i=12;i<=18;i++)
	{
		Y_his_sum_12_18+=reg_HistCnt[i];
	}

	Y_his_max_ratio = SmartPic_clue->Hist_Y_MAX_bin_ratio;// his max ration
	Y_his_ratio_12_18 = Y_his_sum_12_18*100/((SmartPic_clue->Hist_Y_Total_sum)+1);//his local ratio

	for(i=0;i<24;i++)
	{
		//Hue_His_Bin[i] = scaler_rtd_inl( COLOR_ICM_HUE_COUNT_00_reg + (i<<2));

		Total_hue_hist_sum+= (SmartPic_clue->Hue_Main_His[i]); //total hue cnt

		if((SmartPic_clue->Hue_Main_His[i])==0)
			hue_zero_bin_counter++; //zero bin cnt

		if(hue_max_bin_cnt<=(SmartPic_clue->Hue_Main_His[i])) //decide max and second hue bin info
		{
			hue_second_bin_cnt = hue_max_bin_cnt;
			hue_max_bin_cnt = (SmartPic_clue->Hue_Main_His[i]);
			max_index=i;
		}
		else if(hue_second_bin_cnt<=(SmartPic_clue->Hue_Main_His[i]))
		{
			hue_second_bin_cnt = (SmartPic_clue->Hue_Main_His[i]);
			secondindex = i;
		}

		if( (i>=4) && (i<=7) )
		{
			hue_his_sum_2_7+=(SmartPic_clue->Hue_Main_His[i]);
		}
#if 0
		if(MA_print_count%120==0)
		{
			ROSPrintf("Hue_His_Bin[%d]=%d \n",i, (SmartPic_clue->Hue_Main_His[i]));
		}
#endif

	}

	Total_hue_hist_ratio = (Total_hue_hist_sum*100)/(width*height+1);//hue cnt ratio by total pixcel

	hue_max_bin_ratio=(hue_max_bin_cnt*100)/(Total_hue_hist_sum+1);
	hue_second_bin_ratio=(hue_second_bin_cnt*100)/(Total_hue_hist_sum+1);

	hue_his_sum_2_7+=1;
	hue_his_ratio_2_7 =(hue_his_sum_2_7*100)/(Total_hue_hist_sum+1);

	if( (Y_his_max_ratio>=40) && (Y_his_ratio_12_18>=70) && (Total_hue_hist_ratio>=15))
	{
		if((hue_his_ratio_2_7>=68)&&(hue_max_bin_ratio>=50))
		{
			skin_tone_huu_bin_ratio = hue_his_ratio_2_7;
		}
		new_skin_tone_flag = 1;

	}
	else
	{
		skin_tone_huu_bin_ratio = 0;
		new_skin_tone_flag = 0;
	}


#if 0
	if(skin_tone_count%120==0)
	{
		ROSPrintf("Total_pixel=%d,Total_hue_hist_sum=%d \n",width*height, Total_hue_hist_sum );
		ROSPrintf("hue_zero_bin_counter=%d,Total_hue_hist_ratio=%d \n",hue_zero_bin_counter,Total_hue_hist_ratio);
		ROSPrintf("max: index=%d, cnt=%d,hue_max_bin_ratio=%d \n",max_index,hue_max_bin_cnt,hue_max_bin_ratio );
		ROSPrintf("second: index=%d, cnt=%d, hue_second_bin_ratio=%d \n",secondindex,hue_second_bin_cnt, hue_second_bin_ratio);
		ROSPrintf("hue_his_sum_4_7=%d, hue_his_ratio_4_7=%d \n",hue_his_sum_2_7,hue_his_ratio_2_7);

		ROSPrintf("Y_his_max_ratio=%d, Y_his_ratio_12_18=%d \n",Y_his_max_ratio,Y_his_ratio_12_18);


		ROSPrintf("!!!new_skin_tone_flag = %d \n",new_skin_tone_flag);

	}
#endif

	skin_tone_count++;


}




//=========================== Skin Tone Detection ===========================
#ifdef ISR_SKIN_TONE_DETECT_FUNCTION_ENABLE
#define Print_skin_tone 0

void skin_tone_detect(unsigned char channel, unsigned int width, unsigned int height)
{
	unsigned int hist_sum, hist_sum_local, hist_ratio=0, hist_local_ratio=0, hist2_sum,hist_bin01_ratio;
	unsigned int i;//, hue_hist[24];
	unsigned char histogram_non_zero[24], non_zero_sum =0, corn_check =0; //, line_girl_check =0;
	static unsigned int skin_tone_count=0;

//===== default judge bound =====
	unsigned char Hue_histogram_weight[24], non_zero_count =0;
	unsigned char Hue_up_bound_count = 0,  Hue_center_bound_count = 0, Hue_low_bound_count = 0, Hue_high_bound_count =0 ;
	unsigned char Hue_count_up_bound=20 ,Hue_count_low_bound=10, Hue_count_high_bound =20 ;
	unsigned char sum_high_bound= 0;


	unsigned int hist_sum2, hist_ratio2=0, hist_local_ratio2=0;//sony demo
	di_im_di_hfd_statistic_RBUS	im_di_hfd_statistic_Reg;//sony demo
	im_di_hfd_statistic_Reg.regValue = rtd_inl(DI_IM_DI_HFD_STATISTIC_reg);//sony demo
	unsigned char result_check=0;
	unsigned char max_index=0;
	unsigned int tmp=0;
	unsigned int Hue_hist[COLOR_HUE_HISTOGRAM_LEVEL];

	hist2_sum = 0;

	for(i=0;i<24;i++)
	{
		Hue_hist[i] = (SmartPic_clue->Hue_Main_His)[i];

		histogram_non_zero[i] = 0;
		hist2_sum+= Hue_hist[i];
		if(Hue_hist[i]>tmp)
		{
			max_index=i;
			tmp=Hue_hist[i];
		}
	}

	hist2_sum+=1;


//	      scalerDrvPrint("\n>>>>>>>>>>>>>>>> skin_tone_begin = %d <<<<<<<<<<<<<<<\n",skin_tone_begin);


	//}


//	for(i = 0; i< 24; i++)
//		scalerDrvPrint("\n>>>>nick187print>>>>>> hue2_%d = %d <<<<<<\n",i,100*hue_hist_2[i]/hist2_sum);


	// following section to detect the corn move hue color
	non_zero_sum = 0;
	non_zero_count = 0;
	for(i = 0; i< 24; i++)
	{
		if(((unsigned char)(100*Hue_hist[i] /hist2_sum)) == 0)
		{
			histogram_non_zero[i] = 0;
			Hue_histogram_weight[i] = 0;
		}
		else
		{
			histogram_non_zero[i] = 1;
			Hue_histogram_weight[i]=(unsigned char)(100*Hue_hist[i] /hist2_sum);
		}

		Hue_histogram_weight[i] = (Hue_histogram_weight[i]>100)?0:Hue_histogram_weight[i];

		non_zero_count += histogram_non_zero[i];

		non_zero_sum += histogram_non_zero[i];
		if(non_zero_sum == 0)
		{
			switch(i)
				{
				case 4:// hue2[0]~[4] doesn't have value,
				case 9:// hue2[9] reset to 0, hue2[5]~[8] this for yelllow and green corn
				case 13: //hue[8]~[13]  always 0
				case 17://hue2[17] reset to 0, hue2[14]~[16] for cyan sky   //20100611 nick187
				case 23://hue2[16]~[23] always 0
						corn_check += 1;

						break;
				default:
						corn_check += 0;
						break;
				}
		}
		else
		{
			if( i == 5) // hue2[5]~[7] this for yelllow and green cone
				corn_check += 1; // hue2[5] has value, hue2[14] has value
			if(i ==14)//  hue2[14]~[15] for cyan sky
				corn_check += 1; // hue2[5] has value, hue2[14] has value


			non_zero_sum = 0;
		}


	}


	hist_bin01_ratio=Hue_hist[1]*100/(hist2_sum);


	hist_sum = Hue_hist[2]+Hue_hist[3]+Hue_hist[4];
		hist_sum2 = Hue_hist[3]+Hue_hist[4]+Hue_hist[5];//sony demo

	if(hist_bin01_ratio>58)
		hist_sum_local = hist_sum+Hue_hist[5];
	else
		hist_sum_local = Hue_hist[0]+Hue_hist[1]+hist_sum+Hue_hist[5];

	if (hist2_sum)
		hist_ratio = hist_sum*100/ (width*height+1);      //(hist2_sum); avoid divide by zero

	if (hist_sum_local)  // hist_sum_local could be zero when system booting
		hist_local_ratio = hist_sum*100/hist_sum_local;

	//sony demo
    	if (hist2_sum)
			hist_ratio2 = hist_sum2*100/ (width*height+1);
        //sony demo
		if (hist_sum_local)  // hist_sum_local could be zero when system booting
			hist_local_ratio2 = hist_sum2*100/hist_sum_local;

	//LG demo 45and 46 flag
	Hue_up_bound_count = 0;
	Hue_low_bound_count= 0;
	Hue_center_bound_count = 0;
	Hue_high_bound_count= 0;
	sum_high_bound=0;
	for(i = 0; i< 24; i++)
	{
		if(Hue_histogram_weight[i]<=Hue_count_up_bound && Hue_histogram_weight[i]>Hue_count_low_bound)
		{
		 	Hue_center_bound_count++;
		}
		else if(Hue_histogram_weight[i]<=Hue_count_low_bound && Hue_histogram_weight[i]!=0)
		{
		 	Hue_low_bound_count++;
		}
		else if( Hue_histogram_weight[i]>Hue_count_up_bound)
		{
		 	Hue_up_bound_count++;
		}

		if(Hue_histogram_weight[i]>Hue_count_high_bound)
		{
			Hue_high_bound_count++;
			sum_high_bound = sum_high_bound+Hue_histogram_weight[i];
		}

	}

	#define SKIN_TONE_MV_THL 25
	if( (hist_local_ratio>75) && (hist_ratio>8) && (Hue_hist[3]>Hue_hist[2]) && (Hue_hist[3]>Hue_hist[1]) && (Hue_hist[3]>Hue_hist[4]) && (Hue_hist[3]>Hue_hist[5]) &&((SmartPic_clue->Hist_Y_Mean_Value) > SKIN_TONE_MV_THL))
	{ //org (hist_ratio>10)  change by Jerry Wu 20101103, condition:max hue_hist_2[3]
		Line_girl_tone_result = 0;
		Corn_tone_result = 0;
		(SmartPic_clue->skintoneflag) = 1;
		start_to_detected =1;
		result_check=1;
		//avoid moving moon pattern
		if(((SmartPic_clue->Hist_Y_Mean_Value)==37||(SmartPic_clue->Hist_Y_Mean_Value)==36))
		{
			if((Hue_hist[3]<((Hue_hist[2]*225)>>7)))
				(SmartPic_clue->skintoneflag) = 0;
		}
		// oliver-
	//	if((skin_tone_count % 90) == 0)//sony demo
		{
			//ROSPrintf("[skin]section 0!! \n");
			VIP_DEBUG_PRINTF("[skin]section 0!! \n");
			VIP_DEBUG_PRINTF("[skin]section 0,Histogram_Mean_value =%d\n",(SmartPic_clue->Hist_Y_Mean_Value));
		}
	}
	else if( (hist_local_ratio>75) && (hist_ratio>8) && (Hue_hist[3]>Hue_hist[2]) && ((Hue_hist[3]*2)>Hue_hist[1]) && (Hue_hist[3]>Hue_hist[4]) && (Hue_hist[3]>Hue_hist[5])&&((SmartPic_clue->Hist_Y_Mean_Value) > SKIN_TONE_MV_THL) )
	{// (hist_ratio>10) change by Jerry Wu 20101103,
		Line_girl_tone_result = 0;
		Corn_tone_result = 0;
		(SmartPic_clue->skintoneflag) = 1;
		start_to_detected =1;
		result_check=2;
	//oliver-
	//	if((skin_tone_count % 90) == 0)//sony demo
		{
			//ROSPrintf("[skin]section 1!! \n");
			VIP_DEBUG_PRINTF("[skin]section 1!! \n");
			VIP_DEBUG_PRINTF("[skin]section 1,Histogram_Mean_value=%d\n",(SmartPic_clue->Hist_Y_Mean_Value));
		}
	}
	else if( (hist_local_ratio>75) && (hist_ratio>45) && (Hue_hist[2]>Hue_hist[3]) && ((Hue_hist[3]*2)>Hue_hist[1]) && (Hue_hist[3]>Hue_hist[4]) && (Hue_hist[3]>Hue_hist[5])&&((SmartPic_clue->Hist_Y_Mean_Value) > SKIN_TONE_MV_THL) )
	{ // for sony little baby, 20100126 Jerry Wu
		Line_girl_tone_result = 0;
		Corn_tone_result = 0;
		(SmartPic_clue->skintoneflag) = 1;
		start_to_detected =1;
		result_check=3;
		// oliver-
		//if((skin_tone_count % 90) == 0)//sony demo
		{
			//ROSPrintf("[skin]section 2!! \n");
			VIP_DEBUG_PRINTF("[skin]section 2!! \n");
			VIP_DEBUG_PRINTF("[skin]section 2,Histogram_Mean_value=%d\n",(SmartPic_clue->Hist_Y_Mean_Value));
		}
	}
	else if( (hist_local_ratio>40) &&(hist_local_ratio<50)&& (hist_ratio>10) && (hist_ratio<20) && (Hue_hist[3]>Hue_hist[2]) && ( (Hue_hist[3]>Hue_hist[4]) && (Hue_hist[3]>Hue_hist[5]) )&&((SmartPic_clue->Hist_Y_Mean_Value) > SKIN_TONE_MV_THL))
	{ // for sony Red swather girl, 20100127 Jerry Wu
		Line_girl_tone_result = 0;
		Corn_tone_result = 0;
		(SmartPic_clue->skintoneflag) = 1;
		start_to_detected =1;
		result_check=4;
		// oliver-
		//if((skin_tone_count % 90) == 0)//sony demo
		{
			//ROSPrintf("[skin]section 3!! \n");
			VIP_DEBUG_PRINTF("[skin]section 3!! \n");
			VIP_DEBUG_PRINTF("[skin]section 3,Histogram_Mean_value=%d\n",(SmartPic_clue->Hist_Y_Mean_Value));
		}
	}
	else if( (hist_local_ratio>75) && (hist_ratio>8) && (Hue_hist[4]>Hue_hist[3]) && (Hue_hist[4]>Hue_hist[2]) && (Hue_hist[4]>Hue_hist[5]) && (Hue_hist[4]>Hue_hist[6])&&((SmartPic_clue->Hist_Y_Mean_Value) > SKIN_TONE_MV_THL) )
	{ //org (hist_ratio>10)  change by Jerry Wu 20101103, condition:max hue_hist_2[3]
		Line_girl_tone_result = 0;
		Corn_tone_result = 0;
		(SmartPic_clue->skintoneflag) = 1;
		start_to_detected =1;
		result_check=5;
		if(((SmartPic_clue->Hist_Y_Mean_Value)==40||(SmartPic_clue->Hist_Y_Mean_Value)==41))
		{
			if(hist_ratio<=9)
				(SmartPic_clue->skintoneflag) = 0;
		}

		// oliver-
		//if((skin_tone_count % 90) == 0)//sony demo
		{
			//ROSPrintf("[skin]section 4!! \n");
	   	 	VIP_DEBUG_PRINTF("[skin]section 4!! \n");
			VIP_DEBUG_PRINTF("[skin]section 4,Histogram_Mean_value=%d\n",(SmartPic_clue->Hist_Y_Mean_Value));
		}
	}
	else if(hist_local_ratio2>78 &&((Hue_hist[3]>Hue_hist[0]) && (Hue_hist[3]>Hue_hist[1]) && (Hue_hist[3]>Hue_hist[2]) && (Hue_hist[3]>Hue_hist[4]))&&((SmartPic_clue->Hist_Y_Mean_Value) > 63) )
	{
		(SmartPic_clue->skintoneflag) = 1;
		skin_tone_for_panasonic =1;
		result_check=6;
		//if((skin_tone_count % 90) == 0)//panasonic style for skin tone
		{
			//ROSPrintf("[skin]section 5!! \n");
	   	 	VIP_DEBUG_PRINTF("[skin]section 4!! \n");
			VIP_DEBUG_PRINTF("[skin]section 4,Histogram_Mean_value=%d\n",(SmartPic_clue->Hist_Y_Mean_Value));
		}
	}
	else if(hist_local_ratio2>80 &&((Hue_hist[4]>Hue_hist[1]) && (Hue_hist[4]>Hue_hist[2]) && (Hue_hist[4]>Hue_hist[3]) && (Hue_hist[4]>Hue_hist[5]))&&((SmartPic_clue->Hist_Y_Mean_Value) > 63) )
	{
		(SmartPic_clue->skintoneflag) = 1;
		skin_tone_for_panasonic =1;
		result_check=7;
		//if((skin_tone_count % 90) == 0)//panasonic style for skin tone
		{
			//ROSPrintf("[skin]section 6!! \n");
	   	 	VIP_DEBUG_PRINTF("[skin]section 4!! \n");
			VIP_DEBUG_PRINTF("[skin]section 4,Histogram_Mean_value=%d\n",(SmartPic_clue->Hist_Y_Mean_Value));
		}
	}

	else if(corn_check >=7)
	{ //this function implement in drvif_color_ma_hfd_problem
		Line_girl_tone_result = 0;
		Corn_tone_result = 1;
		(SmartPic_clue->skintoneflag) = 0;
		start_to_detected =1;
		//if((skin_tone_count % 90) == 0)//sony demo
		{
			//ROSPrintf("[skin]section 8!! \n");
			VIP_DEBUG_PRINTF("[skin]section 6!!,Histogram_Mean_value=%d \n",(SmartPic_clue->Hist_Y_Mean_Value));
			VIP_DEBUG_PRINTF("--------------------------------------------Corn Detect--------------------------\n");
		}
	}
	else
	{
		if((skin_tone_count % 10) == 0)//sony demo
		{
			VIP_DEBUG_PRINTF("[skin]Yes.corn_check =%d, hfd=%d\n",corn_check,im_di_hfd_statistic_Reg.regValue);
		}


		Line_girl_tone_result = 0;
		Corn_tone_result = 0;
		(SmartPic_clue->skintoneflag) = 0;
		skin_tone_for_panasonic =0;
		result_check=0;
	}

	//max index must be 1,2,3,4,5,6
	if(max_index>6)
		(SmartPic_clue->skintoneflag) = 0;

#if 0
	if(MA_print_count%100==0)
		ROSPrintf("r=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		result_check,(SmartPic_clue->Hist_Y_Mean_Value),hist_local_ratio,hist_ratio,max_index,
		Hue_hist[0],Hue_hist[1],Hue_hist[2],Hue_hist[3],Hue_hist[4],Hue_hist[5],Hue_hist[6]);
#endif
	skin_tone_begin = 1;
	skin_tone_count++;

}
#endif

//if( ((SmartPic_clue->Hist_Y_Mean_Value)<47&&(SmartPic_clue->Hist_Y_Mean_Value)>40) && (im_di_hfd_statistic_Reg.regValue>=105000)  && (hist2_sum<=40000) && (Center_Difference>=15 && Center_Difference<=130))//sony demo
unsigned int mcc_detect_table[6][7] =
{
	{51,		43,		80000,		1700000,		0,		145,		10},		//NTSC // cychen lg deno 45 -> temp disable	// mcc_detect_table[NTSC][3]= 120000 from 40000
	{51,		42,		100000,		40000,		0,		140,		10},		//PAL // cychen lg deno 45 -> temp disable 43->42
	{47,		40,		90000,		40000,		0,		130,		10},		//SD
	{47,		40,		90000,		40000,		0,		130,		10},		//HD
	{47,		40,		100000,		40000,		15,		130,		10},		//VGA
	{47,		40,		100000,		40000,		0,		180,		10},		//HDMI PAL
};

unsigned char motion_concentric_circles_detect(unsigned int width, unsigned int height)
{

	unsigned int hist_local_sum, hist_sum_localrange, hist_ratio_allrange=0, hist_local_ratio=0, hist2_sum;
	unsigned int i;//, hue_hist[24];
	unsigned char radio_color_dete_config=0,curr_motion_status_flag=0;
    //unsigned char motion_condiction_flag=0;	//sony demo
	unsigned int local_HistCnt_ratio[8] = {0};
	unsigned int HistCnt_sum, HistCnt_flag=0, local_ratio_sum=0,local_ratio_average_high=0;
	unsigned char curr_motion_status = 0, ii = 0;

	unsigned int Center_Difference = 0;
	static unsigned int pre_Center_Difference = 0;

	static unsigned int counter_file=0;//sony demo
	static unsigned int sat_hist_sum_bin3to31=0;//2014/03/05 since Sirius
	static unsigned char pre_Saturation_detect_flag=1;
	unsigned char still_concentric_circles_detect_flag=0;

	unsigned int Hue_hist[COLOR_HUE_HISTOGRAM_LEVEL];
	unsigned int *reg_HistCnt=(SmartPic_clue->Y_Main_Hist);
	unsigned int printf_Flag=0,print_delay=0;

	 di_im_di_hfd_statistic_RBUS	 im_di_hfd_statistic_Reg;//sony demo
	 _system_setting_info *system_info_structure_table=NULL;
	system_info_structure_table = scaler_GetShare_Memory_system_setting_info_Struct();

	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;

	im_di_hfd_statistic_Reg.regValue = rtd_inl(DI_IM_DI_HFD_STATISTIC_reg);//sony demo

	curr_motion_status = scalerVideo_GetCurMotionStatus();


	hist2_sum = 0;
	for(i=0;i<24;i++)
	{
		Hue_hist[i] =(SmartPic_clue->Hue_Main_His)[i];
		hist2_sum+= Hue_hist[i];//already read info. from skin tone detector
	}
	hist2_sum+=1;

	HistCnt_sum = 0;
	for(i=0;i<32;i++)
	{
		HistCnt_sum+= reg_HistCnt[i];//already read info. from skin tone detector
	}
	HistCnt_sum+=1;

	if(HistCnt_sum>0)
	{
		local_HistCnt_ratio[0]=100*(reg_HistCnt[0]+reg_HistCnt[1]+reg_HistCnt[2]+reg_HistCnt[3])/HistCnt_sum;
		local_HistCnt_ratio[1]=100*(reg_HistCnt[4]+reg_HistCnt[5]+reg_HistCnt[6]+reg_HistCnt[7])/HistCnt_sum;
		local_HistCnt_ratio[2]=100*(reg_HistCnt[8]+reg_HistCnt[9]+reg_HistCnt[10]+reg_HistCnt[11])/HistCnt_sum;
		local_HistCnt_ratio[3]=100*(reg_HistCnt[12]+reg_HistCnt[13]+reg_HistCnt[14]+reg_HistCnt[15])/HistCnt_sum;
		local_HistCnt_ratio[4]=100*(reg_HistCnt[16]+reg_HistCnt[17]+reg_HistCnt[18]+reg_HistCnt[19])/HistCnt_sum;
		local_HistCnt_ratio[5]=100*(reg_HistCnt[20]+reg_HistCnt[21]+reg_HistCnt[22]+reg_HistCnt[23])/HistCnt_sum;
		local_HistCnt_ratio[6]=100*(reg_HistCnt[24]+reg_HistCnt[25]+reg_HistCnt[26]+reg_HistCnt[27])/HistCnt_sum;
		local_HistCnt_ratio[7]=100*(reg_HistCnt[28]+reg_HistCnt[29]+reg_HistCnt[30]+reg_HistCnt[31])/HistCnt_sum;
	}

/*
	for(i=0;i<8;i++)//sony demo
	{
		if(local_HistCnt_ratio[i]<0)
			local_HistCnt_ratio[i]=0;
	}
*/
	hist_local_sum = reg_HistCnt[9]+reg_HistCnt[10]+reg_HistCnt[14];//sony demo

	for(i=1;i<17;i++)
	{
		hist_sum_localrange = reg_HistCnt[i];//sony demo
	}

	if (hist2_sum) //(hist2_sum); avoid divide by zero
		hist_ratio_allrange = hist_local_sum*100/hist2_sum;

	if (hist_sum_localrange)  // hist_sum_local could be zero when system booting
		hist_local_ratio = hist_local_sum*100/hist_sum_localrange;

	local_ratio_sum=local_HistCnt_ratio[1]+local_HistCnt_ratio[2]+local_HistCnt_ratio[3]+local_HistCnt_ratio[4]+local_HistCnt_ratio[5];//sony demo
	local_ratio_average_high=(local_HistCnt_ratio[6]+local_HistCnt_ratio[7]);//sony demo

//==========HistCnt_flag condiction===================================
	if( (SmartPic_clue->Hist_Y_Mean_Value)<mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][0]&&(SmartPic_clue->Hist_Y_Mean_Value)>mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][1] )
	{
		if(local_ratio_sum>80&&local_ratio_average_high<10&&(local_HistCnt_ratio[1]+local_HistCnt_ratio[2])<=40&&(local_HistCnt_ratio[0]>0 && local_HistCnt_ratio[0]<10))//sony demo
		{

			HistCnt_flag=1;
		}
		else
		{
			HistCnt_flag=0;
		}
	}
	else
	{
		HistCnt_flag=0;
	}
//===========================================================


//==========Saturation_detect_flag condiction============================
	if( (SmartPic_clue->saturationflag)==0)//add for hue_hist_2=0; no data
	{
		radio_color_dete_config=0;
		pre_Saturation_detect_flag = radio_color_dete_config;
	}
	else if( (SmartPic_clue->saturationflag)==3 && pre_Saturation_detect_flag==0)
	{
		radio_color_dete_config=0;
	}
	else
	{
		radio_color_dete_config=1;
		pre_Saturation_detect_flag=radio_color_dete_config;
	}
//================================================================

//===========curr_motion_status condiction==================================
	if( (curr_motion_status<3&&curr_motion_status>0)||curr_motion_status==7)//add for hue_hist_2=0; no data
	{
		curr_motion_status_flag=1;
	}
	else
	{
		curr_motion_status_flag=0;
	}
//================================================================

//===========motion
	Center_Difference = 0;
	for(ii = 4; ii < 12 ; ii++)
	{
		Center_Difference = Center_Difference + Difference_H[ii] + Difference_V[ii];
	}
//===============


//================================================================
	/*START: new "low-sat." condition for still_concentric_circles_detect_flag @ 2014/03/05 since Sirius*/
	sat_hist_sum_bin3to31=0;
	for(i=3;i<32;i++)
		sat_hist_sum_bin3to31 += sat_hist_ratio[i];
	/*END*/
	//scalerVIP_StillMotion(SmartPic_clue); //rord.tsao mark 2015/06/18
	scalerVIP_Difference_H_V_Calculate();

//#if motion_concentric_circles_log
	if((printf_Flag==4) && (MA_print_count%print_delay==0))
		{

			ROSPrintf("ooo--- Histogram_Mean_value=%d ---ooo\n",(SmartPic_clue->Hist_Y_Mean_Value));
			ROSPrintf("ooo--- im_di_hfd_statistic_Reg=%d ---ooo\n",im_di_hfd_statistic_Reg.regValue);
			ROSPrintf("ooo--- Center_Difference=%d 	---ooo\n",Center_Difference/*,	scalerVIP_StillMotion()*/);
			ROSPrintf("ooo--- hist2_sum=%d 	%d---ooo\n",hist2_sum,SmartPic_clue->RTNR_MAD_count_Y_avg_ratio);
			ROSPrintf("ooo--- %d 	%d 		%d	 	%d---ooo\n",((SmartPic_clue->Hist_Y_Mean_Value)<mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][0]&&(SmartPic_clue->Hist_Y_Mean_Value)>mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][1]),(im_di_hfd_statistic_Reg.regValue>=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][2]), (hist2_sum<=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][3]),(Center_Difference>=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][4] && Center_Difference<=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][5]));
			ROSPrintf("ooo--- sat_hist_sum_bin3to31=%d---ooo\n",sat_hist_sum_bin3to31);
			ROSPrintf("ooo-----------------------------------------------ooo\n");
			if(((SmartPic_clue->Hist_Y_Mean_Value)<mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][0]&&(SmartPic_clue->Hist_Y_Mean_Value)>mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][1])==0)
				ROSPrintf("ooo---1. %d < %d < %d---ooo\n",mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][1],(SmartPic_clue->Hist_Y_Mean_Value),mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][0]);
			if((im_di_hfd_statistic_Reg.regValue>=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][2])==0)
				ROSPrintf("ooo---2. %d >= %d---ooo\n",im_di_hfd_statistic_Reg.regValue,mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][2]);
			if((hist2_sum<=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][3])==0)
				ROSPrintf("ooo---3. %d <= %d---ooo\n",hist2_sum,mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][3]);
			if((Center_Difference>=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][4] && Center_Difference<=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][5])==0)
				ROSPrintf("ooo---4. %d <= %d <= %d---ooo\n",mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][4] ,Center_Difference, mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][5]);
			if((sat_hist_sum_bin3to31<=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][6])==0)
				ROSPrintf("ooo---5. %d <= %d ---ooo\n",sat_hist_sum_bin3to31,mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][6]);

		}
//#endif

//	if( ((SmartPic_clue->Hist_Y_Mean_Value)<47&&(SmartPic_clue->Hist_Y_Mean_Value)>40) && curr_motion_status_flag==1 && HistCnt_flag==1 && (im_di_hfd_statistic_Reg.regValue>=120000)   )//sony demo
	if( ((SmartPic_clue->Hist_Y_Mean_Value)<mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][0] &&
		(SmartPic_clue->Hist_Y_Mean_Value)>mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][1]) &&
		(im_di_hfd_statistic_Reg.regValue>=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][2]) &&
		(hist2_sum<=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][3]) &&
		(Center_Difference>=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][4] &&
		Center_Difference<=mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][5]) &&
		sat_hist_sum_bin3to31 <= mcc_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][6])
	{
		still_concentric_circles_detect_flag=1;
	}
	else
	{
		still_concentric_circles_detect_flag=0;
	}
	pre_Center_Difference = Center_Difference;
	counter_file++;

	return still_concentric_circles_detect_flag;

}


unsigned char motion_purecolor_concentric_circles_detect(unsigned int width, unsigned int height)//sony demo
{


	unsigned int hist2_sum;
	unsigned int i;//, hue_hist[24];
	static unsigned int local_hue_his1=0,local_hue_his2=0,local_hue_his3=0;
	unsigned char radio_color_dete_config=0,curr_motion_status_flag=0;
	unsigned int local_HistCnt_ratio[8] = {0};
	unsigned int HistCnt_sum, HistCnt_flag=0, local_ratio_sum=0;
	unsigned char curr_motion_status = 0;

	static unsigned int counter_file=0;
	unsigned char motion_purecolor_concentric_circles_detect_flag=0;//sony demo
	unsigned int *reg_HistCnt=(SmartPic_clue->Y_Main_Hist);

	di_im_di_hfd_statistic_RBUS	im_di_hfd_statistic_Reg;
	im_di_hfd_statistic_Reg.regValue = rtd_inl(DI_IM_DI_HFD_STATISTIC_reg);



    curr_motion_status = scalerVideo_GetCurMotionStatus();
	// oliver-
	//static unsigned short H_Block_Pre1[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//static unsigned short V_Block_Pre1[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//static unsigned short H_Block_Pre2[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//static unsigned short V_Block_Pre2[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	hist2_sum = 0;
	for(i=0;i<24;i++)
	{
		//hue_hist_2[i] = scaler_rtd_inl( COLOR_ICM_HUE_COUNT_00_reg + (i<<2));
		hist2_sum+= hue_hist_2[i];//already read info. from skin tone detector
	}
	hist2_sum+=1;

	HistCnt_sum = 0;
	for(i=0;i<32;i++)
	{
		//hue_hist_2[i] = scaler_rtd_inl( COLOR_ICM_HUE_COUNT_00_reg + (i<<2));
		HistCnt_sum+= reg_HistCnt[i];//already read info. from skin tone detector
	}
	HistCnt_sum+=1;

	local_HistCnt_ratio[0]=100*(reg_HistCnt[0]+reg_HistCnt[1]+reg_HistCnt[2]+reg_HistCnt[3])/HistCnt_sum;
	local_HistCnt_ratio[1]=100*(reg_HistCnt[4]+reg_HistCnt[5]+reg_HistCnt[6]+reg_HistCnt[7])/HistCnt_sum;
	local_HistCnt_ratio[2]=100*(reg_HistCnt[8]+reg_HistCnt[9]+reg_HistCnt[10]+reg_HistCnt[11])/HistCnt_sum;
	local_HistCnt_ratio[3]=100*(reg_HistCnt[12]+reg_HistCnt[13]+reg_HistCnt[14]+reg_HistCnt[15])/HistCnt_sum;
	local_HistCnt_ratio[4]=100*(reg_HistCnt[16]+reg_HistCnt[17]+reg_HistCnt[18]+reg_HistCnt[19])/HistCnt_sum;
	local_HistCnt_ratio[5]=100*(reg_HistCnt[20]+reg_HistCnt[21]+reg_HistCnt[22]+reg_HistCnt[23])/HistCnt_sum;
	local_HistCnt_ratio[6]=100*(reg_HistCnt[24]+reg_HistCnt[25]+reg_HistCnt[26]+reg_HistCnt[27])/HistCnt_sum;
	local_HistCnt_ratio[7]=100*(reg_HistCnt[28]+reg_HistCnt[29]+reg_HistCnt[30]+reg_HistCnt[31])/HistCnt_sum;

/*
	for(i=0;i<8;i++)
	{
		if(local_HistCnt_ratio[i]<0)
			local_HistCnt_ratio[i]=0;
	}
*/
    if(hist2_sum!=0)
    {
		local_hue_his1 =100*hue_hist_2[9]/hist2_sum;
		local_hue_his2 =100*hue_hist_2[17]/hist2_sum;
		local_hue_his3 =100*hue_hist_2[2]/hist2_sum;
    }


	local_ratio_sum=local_HistCnt_ratio[4]+local_HistCnt_ratio[5]+local_HistCnt_ratio[6]+local_HistCnt_ratio[7];

//==========HistCnt_flag condiction===================================
	if( (SmartPic_clue->Hist_Y_Mean_Value)<30 )
	{
		if(local_ratio_sum<3)
		{

			HistCnt_flag=1;
	}
		else
	{
			HistCnt_flag=0;
		}
	}
	else
	{
		HistCnt_flag=0;
	}
//===========================================================


//==========Saturation_detect_flag condiction============================
if(hist2_sum!=0)
{
	if( (local_hue_his1>=82 && local_hue_his1<=88 ) || (local_hue_his2>=78 && local_hue_his2<=87) || (local_hue_his3>=80 && local_hue_his3<=93) )//add for hue_hist_2=0; no data
	{
		radio_color_dete_config=1;
	}
	else
	{
		radio_color_dete_config=0;
	}
}
//================================================================

//===========curr_motion_status condiction==================================
	if( curr_motion_status<=1)//add for hue_hist_2=0; no data
	{
		curr_motion_status_flag=1;
	}
	else
	{
		curr_motion_status_flag=0;
	}
//================================================================

//================================================================


	if( radio_color_dete_config==1 && curr_motion_status_flag==1 && HistCnt_flag==1 )
	{

		if(counter_file%60==0)
		{
			//ROSPrintf("ooo---motion_purecolor_concentric_circles_detect=1---ooo\n");
		}
		motion_purecolor_concentric_circles_detect_flag=1;
		counter_file++;
		//return 1;
	}
	else
	{
/*
		if(counter_file%60==0)
		{
			ROSPrintf("ooo---motion_purecolor_concentric_circles_detect=0---ooo\n");
			ROSPrintf("ooo--- local_hue_his1=%d,local_hue_his2=%d, local_hue_his3=%d ---ooo\n",local_hue_his1,local_hue_his2,local_hue_his3);
			ROSPrintf("ooo--- hist2_sum=%d, color_dete_config=%d ---ooo\n",hist2_sum,radio_color_dete_config);
			ROSPrintf("ooo--- curr_motion_status = %d,curr_motion_status_flag=%d ---ooo\n",curr_motion_status,curr_motion_status_flag);
			ROSPrintf("ooo--- local_ratio_sum=%d, local_ratio_average_high=%d ---ooo\n",local_ratio_sum);
			ROSPrintf("ooo--- Histogram_Mean_value=%d ,HistCnt_flag=%d---ooo\n",(SmartPic_clue->Hist_Y_Mean_Value),HistCnt_flag);
			ROSPrintf("ooo--- im_di_hfd_statistic_Reg=%d ---ooo\n",im_di_hfd_statistic_Reg.regValue);

		}
*/
		motion_purecolor_concentric_circles_detect_flag=0;
		counter_file++;
		//return 0;
	}

	return motion_purecolor_concentric_circles_detect_flag;


}


unsigned char motion_move_concentric_circles_detect(unsigned int width, unsigned int height)
{
	unsigned int hist_local_sum, hist_sum_localrange, hist_ratio_allrange=0, hist_local_ratio=0, hist2_sum;
	unsigned int i;//, hue_hist[24];
	unsigned char radio_color_dete_config=0,curr_motion_status_flag=0;
	unsigned int local_HistCnt_ratio[8] = {0};
	unsigned int HistCnt_sum, HistCnt_flag=0, local_ratio_sum=0,local_ratio_average_high=0;
	unsigned char curr_motion_status = 0;
	static unsigned char pre_Saturation_detect_flag=1;

	static unsigned int counter_file=0;
	unsigned char motion_move_concentric_circles_detect_flag=0;
	unsigned int *reg_HistCnt=(SmartPic_clue->Y_Main_Hist);

	di_im_di_hfd_statistic_RBUS	im_di_hfd_statistic_Reg;
	im_di_hfd_statistic_Reg.regValue = rtd_inl(DI_IM_DI_HFD_STATISTIC_reg);



       curr_motion_status = scalerVideo_GetCurMotionStatus();
	// oliver-
	//static unsigned short H_Block_Pre1[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//static unsigned short V_Block_Pre1[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//static unsigned short H_Block_Pre2[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//static unsigned short V_Block_Pre2[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	hist2_sum = 0;
	for(i=0;i<24;i++)
	{
		//hue_hist_2[i] = scaler_rtd_inl( COLOR_ICM_HUE_COUNT_00_reg + (i<<2));
		hist2_sum+= hue_hist_2[i];//already read info. from skin tone detector
	}
	hist2_sum+=1;

	HistCnt_sum = 0;
	for(i=0;i<32;i++)
	{
		//hue_hist_2[i] = scaler_rtd_inl( COLOR_ICM_HUE_COUNT_00_reg + (i<<2));
		HistCnt_sum+= reg_HistCnt[i];//already read info. from skin tone detector
	}
	HistCnt_sum+=1;


	local_HistCnt_ratio[0]=100*(reg_HistCnt[0]+reg_HistCnt[1]+reg_HistCnt[2]+reg_HistCnt[3])/HistCnt_sum;
	local_HistCnt_ratio[1]=100*(reg_HistCnt[4]+reg_HistCnt[5]+reg_HistCnt[6]+reg_HistCnt[7])/HistCnt_sum;
	local_HistCnt_ratio[2]=100*(reg_HistCnt[8]+reg_HistCnt[9]+reg_HistCnt[10]+reg_HistCnt[11])/HistCnt_sum;
	local_HistCnt_ratio[3]=100*(reg_HistCnt[12]+reg_HistCnt[13]+reg_HistCnt[14]+reg_HistCnt[15])/HistCnt_sum;
	local_HistCnt_ratio[4]=100*(reg_HistCnt[16]+reg_HistCnt[17]+reg_HistCnt[18]+reg_HistCnt[19])/HistCnt_sum;
	local_HistCnt_ratio[5]=100*(reg_HistCnt[20]+reg_HistCnt[21]+reg_HistCnt[22]+reg_HistCnt[23])/HistCnt_sum;
	local_HistCnt_ratio[6]=100*(reg_HistCnt[24]+reg_HistCnt[25]+reg_HistCnt[26]+reg_HistCnt[27])/HistCnt_sum;
	local_HistCnt_ratio[7]=100*(reg_HistCnt[28]+reg_HistCnt[29]+reg_HistCnt[30]+reg_HistCnt[31])/HistCnt_sum;
/*
		for(i=0;i<8;i++)
		{
		if(local_HistCnt_ratio[i]<0)
			local_HistCnt_ratio[i]=0;
	}
*/
	hist_local_sum = reg_HistCnt[9]+reg_HistCnt[10]+reg_HistCnt[14];

	for(i=1;i<17;i++)
	{
		hist_sum_localrange = reg_HistCnt[i];
	}

	if (hist2_sum) //(hist2_sum); avoid divide by zero
		hist_ratio_allrange = hist_local_sum*100/hist2_sum;

	if (hist_sum_localrange)  // hist_sum_local could be zero when system booting
		hist_local_ratio = hist_local_sum*100/hist_sum_localrange;

	local_ratio_sum=local_HistCnt_ratio[1]+local_HistCnt_ratio[2]+local_HistCnt_ratio[3]+local_HistCnt_ratio[4]+local_HistCnt_ratio[5];
	local_ratio_average_high=(local_HistCnt_ratio[6]+local_HistCnt_ratio[7]);

//==========HistCnt_flag condiction===================================
	if( (SmartPic_clue->Hist_Y_Mean_Value)<47&&(SmartPic_clue->Hist_Y_Mean_Value)>40 )
	{
		if(local_ratio_sum>80 && local_ratio_average_high<=6 && local_HistCnt_ratio[0]<3 )
		{

			HistCnt_flag=1;
	}
	else
	{
			HistCnt_flag=0;
		}
	}
	else
	{
		HistCnt_flag=0;
	}
//===========================================================


//==========Saturation_detect_flag condiction============================
	if( (SmartPic_clue->saturationflag)==0)//add for hue_hist_2=0; no data
	{
		radio_color_dete_config=0;
		pre_Saturation_detect_flag = radio_color_dete_config;
	}
	else if( (SmartPic_clue->saturationflag)==3 && pre_Saturation_detect_flag==0)
	{
		radio_color_dete_config=0;
	}
	else
	{
		radio_color_dete_config=1;
		pre_Saturation_detect_flag=radio_color_dete_config;
	}
//================================================================

//===========curr_motion_status condiction==================================
	if(system_info_structure_table->Input_src_Form == _SRC_FROM_VDC)
	{
		if(scalerVideo_readMode()==VDC_MODE_PALI)
		{
			if( curr_motion_status==7)//add for hue_hist_2=0; no data
			{
				curr_motion_status_flag=1;
			}
			else
			{
				curr_motion_status_flag=0;
			}
		}

		if(scalerVideo_readMode()==VDC_MODE_NTSC)
			{
			if( curr_motion_status>=5)//add for hue_hist_2=0; no data
			{
				curr_motion_status_flag=1;
			}
			else
			{
				curr_motion_status_flag=0;
			}
		}
	}


//================================================================

//================================================================


#if 0
static unsigned int print_cnt = 0;
if(print_cnt % 60 == 0)
	ROSPrintf("[nick187] curr_ms=%d, DCC_MC=%d, lrah=%d, lHr[0]=%d, hfd_statistic=%d\n",
	curr_motion_status_flag, (SmartPic_clue->Hist_Y_Mean_Value), local_ratio_average_high, local_HistCnt_ratio[0], im_di_hfd_statistic_Reg.regValue);
print_cnt ++;
#endif

	if( curr_motion_status_flag==1 && HistCnt_flag==1 && (im_di_hfd_statistic_Reg.regValue>=50000)   )
	{

		if(counter_file%60==0)
		{
			//ROSPrintf("ooo---motion_move_concentric_circles_detect=1---ooo\n");
		}
		motion_move_concentric_circles_detect_flag=1;
		counter_file++;
		//return 1;
	}
	else
	{
/*
		if(counter_file%60==0)
		{
			ROSPrintf("ooo---motion_move_concentric_circles_detect=0---ooo\n");
			ROSPrintf("ooo--- Saturation_detect_flag=%d ---ooo\n",(SmartPic_clue->saturationflag));
			ROSPrintf("ooo--- curr_motion_status = %d,curr_motion_status_flag=%d ---ooo\n",curr_motion_status,curr_motion_status_flag);
			ROSPrintf("ooo--- hist_ratio_allrange=%d, hist_local_ratio=%d ---ooo\n",hist_ratio_allrange,hist_local_ratio);
			ROSPrintf("ooo--- local_ratio_sum=%d, local_ratio_average_high=%d ---ooo\n",local_ratio_sum,local_ratio_average_high);
			ROSPrintf("ooo--- Histogram_Mean_value=%d ,HistCnt_flag=%d---ooo\n",(SmartPic_clue->Hist_Y_Mean_Value),HistCnt_flag);
			ROSPrintf("ooo--- im_di_hfd_statistic_Reg=%d ---ooo\n",im_di_hfd_statistic_Reg.regValue);

		for(i=0;i<8;i++)
		{
				ROSPrintf("ooo--- local_HistCnt_ratio[%d]=%d ---ooo\n",i,local_HistCnt_ratio[i]);
		}

		}
*/
		motion_move_concentric_circles_detect_flag=0;
		counter_file++;
		//return 0;
	}

	return motion_move_concentric_circles_detect_flag;


}





void DI_Champagne_problem(void)
{

		di_im_di_ma_frame_motion_th_a_RBUS frame_motion_a_reg;// frame motion a
		di_im_di_ma_frame_motion_th_b_RBUS frame_motion_b_reg;// frame motion b
		di_im_di_ma_frame_motion_th_c_RBUS frame_motion_c_reg;// frame motion c
		di_im_di_ma_field_teeth_th_a_RBUS  field_teeth_th_a_reg;

		//if((SmartPic_clue->Champagne_flag)==1)// && film_motion_top > 4 && film_motion_bot > 4 && film_motion_left > 4 && film_motion_right > 4)
		//{
			frame_motion_a_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
			frame_motion_b_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg);
			frame_motion_c_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg);
			field_teeth_th_a_reg.regValue = rtd_inl(DI_IM_DI_MA_FIELD_TEETH_TH_A_reg);

			frame_motion_a_reg.ma_framemtha = 0x04;
			frame_motion_a_reg.ma_framehtha = 0x05;
			frame_motion_a_reg.ma_framerlvtha = 0x01;
			frame_motion_a_reg.ma_framestha = 0x01;

			frame_motion_b_reg.ma_framemthb = 0x14;
			frame_motion_b_reg.ma_framehthb = 0x0a;
			frame_motion_b_reg.ma_framerlvthb = 0x02;
			frame_motion_b_reg.ma_framesthb = 0x02;

			frame_motion_c_reg.ma_framemthc = 0x32;
			frame_motion_c_reg.ma_framehthc = 0x14;
			frame_motion_c_reg.ma_framerlvthc = 0x01;
			frame_motion_c_reg.ma_framesthc = 0x01;

			field_teeth_th_a_reg.ma_fieldteethstha = 0x32;
			field_teeth_th_a_reg.ma_fieldteethmtha = 0x14;
			field_teeth_th_a_reg.ma_fieldteethhtha = 0x03;
			field_teeth_th_a_reg.ma_fieldteethrlvtha = 0x03;

			/*
			frame_motion_a_reg.regValue =   0x04050101; //0x04050202;
			frame_motion_b_reg.regValue =   0x140a0202; //0x140a060c;
			frame_motion_c_reg.regValue =   0x32140101; //0x140a060c;
			field_teeth_th_a_reg.regValue = 0x32140303; //0x3a141018;
			*/

			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, frame_motion_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, frame_motion_b_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, frame_motion_c_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FIELD_TEETH_TH_A_reg, field_teeth_th_a_reg.regValue);
		//}
		//else
		//{
		//	scalerVIP_colorMAQuickSetTh(2);
		//}


}


unsigned char DI_detect_Champagne(unsigned int width, unsigned int height)
{

	unsigned int cur_total_motion;
	static unsigned int counter_file=0;
	unsigned int film_motion_c, film_motion_top, film_motion_bot, film_motion_left, film_motion_right;
	unsigned int motion_h_09, motion_h_10, motion_h_11;

	unsigned int hist_local_sum, hist_sum_localrange, hist_ratio_allrange=0, hist_local_ratio=0, hist2_sum;
	int i;
	unsigned int local_HistCnt_ratio[8],HistCnt_sum;
	unsigned char curr_motion_status = 0;

	unsigned char his_flag=0, Mean_value_flag=0, motion_flag=0;
	unsigned char DI_detect_Champagne_flag=0;
	unsigned int *reg_HistCnt=(SmartPic_clue->Y_Main_Hist);


	di_im_di_si_ma_total_fm_sum_RBUS ma_total_fm_sum;
	di_im_di_si_film_motion_c_RBUS di_si_film_motion_c;
	di_im_di_si_film_motion_h_t_RBUS di_si_film_motion_h_t;
	di_im_di_si_film_motion_h_b_RBUS di_si_film_motion_h_b;
	di_im_di_si_film_motion_v_l_RBUS di_si_film_motion_v_l;
	di_im_di_si_film_motion_v_r_RBUS di_si_film_motion_v_r;
	di_im_di_si_film_motion_h_09_RBUS di_si_film_motion_h_09;
	di_im_di_si_film_motion_h_10_RBUS di_si_film_motion_h_10;
	di_im_di_si_film_motion_h_11_RBUS di_si_film_motion_h_11;

	di_im_di_hfd_statistic_RBUS	im_di_hfd_statistic_Reg;


	im_di_hfd_statistic_Reg.regValue = rtd_inl(DI_IM_DI_HFD_STATISTIC_reg);

	//di_im_di_si_film_motion_h_09_RBUS im_di_si_film_motion_h_09_reg;
	//di_im_di_si_film_motion_h_10_RBUS im_di_si_film_motion_h_10_reg;
	//di_im_di_si_film_motion_h_11_RBUS im_di_si_film_motion_h_11_reg;

	ma_total_fm_sum.regValue = rtd_inl(DI_IM_DI_SI_MA_TOTAL_FM_SUM_reg);
	di_si_film_motion_c.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_C_reg);
	di_si_film_motion_h_t.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_T_reg);
	di_si_film_motion_h_b.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_B_reg);
	di_si_film_motion_v_l.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_L_reg);
	di_si_film_motion_v_r.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_R_reg);

	di_si_film_motion_h_09.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_09_reg);
	di_si_film_motion_h_10.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_10_reg);
	di_si_film_motion_h_11.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_11_reg);


	cur_total_motion = ma_total_fm_sum.total_frame_motion_sum;
	film_motion_c = di_si_film_motion_c.film_motionstatus_259_240;
	film_motion_top = di_si_film_motion_h_t.film_motionstatus_379_360;
	film_motion_bot = di_si_film_motion_h_b.film_motionstatus_339_320;
	film_motion_left = di_si_film_motion_v_l.film_motionstatus_319_300;
	film_motion_right = di_si_film_motion_v_r.film_motionstatus_279_260;

	motion_h_09 = di_si_film_motion_h_09.film_motionstatus_199_180;
	motion_h_10 = di_si_film_motion_h_10.film_motionstatus_219_200;
	motion_h_11 = di_si_film_motion_h_11.film_motionstatus_239_220;


	curr_motion_status = scalerVideo_GetCurMotionStatus();

	hist2_sum = 0;
	for(i=0;i<24;i++)
	{
		//hue_hist_2[i] = scaler_rtd_inl( COLOR_ICM_HUE_COUNT_00_reg + (i<<2));
		hist2_sum+= hue_hist_2[i];//already read info. from skin tone detector
	}
	hist2_sum+=1;

	hist_local_sum = hue_hist_2[3]+hue_hist_2[4];
	hist_sum_localrange = hue_hist_2[2]+hist_local_sum+hue_hist_2[5]+hue_hist_2[6];

	if (hist2_sum) //(hist2_sum); avoid divide by zero
		hist_ratio_allrange = hist_local_sum*100/hist2_sum;

	if (hist_sum_localrange)  // hist_sum_local could be zero when system booting
		hist_local_ratio = hist_local_sum*100/hist_sum_localrange;

	HistCnt_sum = 0;
	for(i=0;i<32;i++)
	{
		//hue_hist_2[i] = scaler_rtd_inl( COLOR_ICM_HUE_COUNT_00_reg + (i<<2));
		HistCnt_sum+= reg_HistCnt[i];//already read info. from skin tone detector
	}
	HistCnt_sum+=1;

//===================FOR add condiction avoid restaurant  (not OK)==========================================

		local_HistCnt_ratio[0]=100*(reg_HistCnt[0]+reg_HistCnt[1]+reg_HistCnt[2]+reg_HistCnt[3])/HistCnt_sum;
		local_HistCnt_ratio[1]=100*(reg_HistCnt[4]+reg_HistCnt[5]+reg_HistCnt[6]+reg_HistCnt[7])/HistCnt_sum;
		local_HistCnt_ratio[2]=100*(reg_HistCnt[8]+reg_HistCnt[9]+reg_HistCnt[10]+reg_HistCnt[11])/HistCnt_sum;
		local_HistCnt_ratio[3]=100*(reg_HistCnt[12]+reg_HistCnt[13]+reg_HistCnt[14]+reg_HistCnt[15])/HistCnt_sum;
		local_HistCnt_ratio[4]=100*(reg_HistCnt[16]+reg_HistCnt[17]+reg_HistCnt[18]+reg_HistCnt[19])/HistCnt_sum;
		local_HistCnt_ratio[5]=100*(reg_HistCnt[20]+reg_HistCnt[21]+reg_HistCnt[22]+reg_HistCnt[23])/HistCnt_sum;
		local_HistCnt_ratio[6]=100*(reg_HistCnt[24]+reg_HistCnt[25]+reg_HistCnt[26]+reg_HistCnt[27])/HistCnt_sum;
		local_HistCnt_ratio[7]=100*(reg_HistCnt[28]+reg_HistCnt[29]+reg_HistCnt[30]+reg_HistCnt[31])/HistCnt_sum;


	if(local_HistCnt_ratio[3]>=70 && local_HistCnt_ratio[3]<=90)
	{
		his_flag=1;
	}
	else
	{
		his_flag=0;
	}

	if((SmartPic_clue->Hist_Y_Mean_Value)>=38 && (SmartPic_clue->Hist_Y_Mean_Value)<=47)
	{
		Mean_value_flag=1;
	}
	else
	{
		Mean_value_flag=0;
	}

	if(motion_h_09<=300 && motion_h_10<=80 && motion_h_11<=60 && film_motion_left<=20 && film_motion_bot<=100 )
	{
		motion_flag=1;
	}
	else
	{
		motion_flag=0;
	}


	counter_file++;


	if( his_flag==1 && Mean_value_flag==1 && motion_flag==1 && (SmartPic_clue->di_motion_status)<=3 && im_di_hfd_statistic_Reg.regValue<=1000)
	{

		DI_detect_Champagne_flag =1;
	}
	else
	{
		DI_detect_Champagne_flag =0;
	}


	if(counter_file%10==0)
	{
		if(DI_detect_Champagne_flag==1)
		{
			VIP_DEBUG_PRINTF("oo---DI_detect_Champagne_flag=%d---oo\n",DI_detect_Champagne_flag);
		}
		else
		{

			VIP_DEBUG_PRINTF("oo---DI_detect_Champagne_flag=%d---oo\n",DI_detect_Champagne_flag);
/*
		ROSPrintf("o---film_motion_left = %d, MA_data[1] = %d, MA_data[2] = %d  ---o\n", film_motion_left, film_motion_bot, film_motion_c);
		ROSPrintf("oo---motion_h_09=%d, motion_h_10=%d, motion_h_11=%d---oo\n",motion_h_09,motion_h_10,motion_h_11);
		ROSPrintf("oo---Histogram_Mean_value=%d---oo\n",(SmartPic_clue->Hist_Y_Mean_Value));
		ROSPrintf("oo---motion_flag=%d, Mean_value_flag=%d,his_flag=%d---oo\n",motion_flag,Mean_value_flag,his_flag);
		for(i=0;i<8;i++)
		{
			ROSPrintf("ooo--- local_HistCnt_ratio[%d]=%d ---ooo\n",i,local_HistCnt_ratio[i]);
		}

		for(i=0;i<COLOR_HISTOGRAM_LEVEL;i++)
		{
			ROSPrintf(">>>>>>>>>>>>>>>> reg_HistCnt[%d] = %d <<<<<<<<<<<<<<<\n",i,reg_HistCnt[i]);
		}

		for(i=0; i<24; i++)
		{
			ROSPrintf(">>>>>>>>>>>>>>>> hue_hist_2[%d] = %d <<<<<<<<<<<<<<<\n",i,hue_hist_2[i]);
		}
*/
		}

}



	return DI_detect_Champagne_flag;


}

unsigned char Sony_radio_detect(unsigned char channel, unsigned int width, unsigned int height)
{
	unsigned int hist_local_sum, hist_sum_localrange, hist_ratio_allrange=0, hist_local_ratio=0, hist2_sum;
	unsigned int i;//, hue_hist[24];
	unsigned char radio_color_dete_config=0;
	unsigned int local_HistCnt_ratio[8],HistCnt_sum;
	unsigned char curr_motion_status = 0;
	static unsigned char pre_Saturation_detect_flag=1;

	unsigned int *reg_HistCnt=(SmartPic_clue->Y_Main_Hist);
	unsigned char his_up_th=0, his_low_th=0;
	unsigned char which_source=255;

    curr_motion_status = scalerVideo_GetCurMotionStatus();

	hist2_sum = 0;
	for(i=0;i<24;i++)
	{
		//hue_hist_2[i] = scaler_rtd_inl( COLOR_ICM_HUE_COUNT_00_reg + (i<<2));
		hist2_sum+= hue_hist_2[i];//already read info. from skin tone detector
	}
	hist2_sum+=1;

	hist_local_sum = hue_hist_2[3]+hue_hist_2[4];
	hist_sum_localrange = hue_hist_2[2]+hist_local_sum+hue_hist_2[5]+hue_hist_2[6];

	if (hist2_sum) //(hist2_sum); avoid divide by zero
		hist_ratio_allrange = hist_local_sum*100/hist2_sum;

	if (hist_sum_localrange)  // hist_sum_local could be zero when system booting
		hist_local_ratio = hist_local_sum*100/hist_sum_localrange;

	HistCnt_sum = 0;
	for(i=0;i<32;i++)
	{
		//hue_hist_2[i] = scaler_rtd_inl( COLOR_ICM_HUE_COUNT_00_reg + (i<<2));
		HistCnt_sum+= reg_HistCnt[i];//already read info. from skin tone detector
	}
	HistCnt_sum+=1;

//===================FOR add condiction avoid restaurant  (not OK)==========================================
	if(HistCnt_sum>0)
	{
		local_HistCnt_ratio[0]=100*(reg_HistCnt[0]+reg_HistCnt[1]+reg_HistCnt[2]+reg_HistCnt[3])/HistCnt_sum;
		local_HistCnt_ratio[1]=100*(reg_HistCnt[4]+reg_HistCnt[5]+reg_HistCnt[6]+reg_HistCnt[7])/HistCnt_sum;
		local_HistCnt_ratio[2]=100*(reg_HistCnt[8]+reg_HistCnt[9]+reg_HistCnt[10]+reg_HistCnt[11])/HistCnt_sum;
		local_HistCnt_ratio[3]=100*(reg_HistCnt[12]+reg_HistCnt[13]+reg_HistCnt[14]+reg_HistCnt[15])/HistCnt_sum;
		local_HistCnt_ratio[4]=100*(reg_HistCnt[16]+reg_HistCnt[17]+reg_HistCnt[18]+reg_HistCnt[19])/HistCnt_sum;
		local_HistCnt_ratio[5]=100*(reg_HistCnt[20]+reg_HistCnt[21]+reg_HistCnt[22]+reg_HistCnt[23])/HistCnt_sum;
		local_HistCnt_ratio[6]=100*(reg_HistCnt[24]+reg_HistCnt[25]+reg_HistCnt[26]+reg_HistCnt[27])/HistCnt_sum;
		local_HistCnt_ratio[7]=100*(reg_HistCnt[28]+reg_HistCnt[29]+reg_HistCnt[30]+reg_HistCnt[31])/HistCnt_sum;
	}
//=====================================================================================


	if( (SmartPic_clue->saturationflag)==0)//add for hue_hist_2=0; no data
	{
		radio_color_dete_config=0;
		pre_Saturation_detect_flag = radio_color_dete_config;
	}
	else if( (SmartPic_clue->saturationflag)==3 && pre_Saturation_detect_flag==0)
	{
		radio_color_dete_config=0;
	}
	else
	{
		radio_color_dete_config=1;
		pre_Saturation_detect_flag=radio_color_dete_config;
	}

	which_source=(RPC_system_info_structure_table->VIP_source);

	if((which_source>=VIP_QUALITY_HDMI_480I && which_source<=VIP_QUALITY_HDMI_576P)||(which_source>=VIP_QUALITY_YPbPr_480I && which_source<=VIP_QUALITY_YPbPr_576P))
	{
		his_up_th = 13;
		his_low_th = 7;
	}
	else if((which_source>=VIP_QUALITY_HDMI_720P && which_source<=VIP_QUALITY_HDMI_1080P))
	{
		his_up_th = 19;
		his_low_th = 16;
	}

	//if((SmartPic_clue->Hist_Y_Mean_Value)<12 && curr_motion_status<3 && radio_color_dete_config==0 )//sony demo
	if((SmartPic_clue->Hist_Y_Mean_Value)<=his_up_th && (SmartPic_clue->Hist_Y_Mean_Value)>=his_low_th && curr_motion_status<3 && radio_color_dete_config==0 )// for sony demo 480i,blu-ray 4:3,16:9
	{
		if(MA_print_count%30 ==0)
		{
			//ROSPrintf("ooo---Sony_radio_detect=1---ooo\n");
		}
		return 1;
	}
	else
	{
		//ROSPrintf("ooo---Sony_radio_detect=0---ooo\n");
		//ROSPrintf("ooo--- Saturation_detect_flag=%d, radio_color_dete_config=%d, pre_Saturation_detect_flag=%d ---ooo\n",(SmartPic_clue->saturationflag),radio_color_dete_config,pre_Saturation_detect_flag);
		//ROSPrintf("ooo--- curr_motion_status = %d ---ooo\n",curr_motion_status);
		//ROSPrintf("ooo--- hist_ratio_allrange=%d, hist_local_ratio=%d ---ooo\n",hist_ratio_allrange,hist_local_ratio);
		//ROSPrintf("ooo--- Histogram_Mean_value=%d ---ooo\n",(SmartPic_clue->Hist_Y_Mean_Value));

		return 0;
	}

}

unsigned int black_white_detect_table[5][6] =
{
	{300,	1,		60000,		2		},	//NTSC
	{300,	1,		60000,		2		},	//PAL
	{1,		1,		40000,		40000	},	//SD
	{1,		1,		40000,		40000	},	//HD
	{1,		1,		40000,		40000	},	//VGA
};
unsigned char black_white_pattern_detect(void)
{
	color_icm_dm_uv_coring_RBUS dm_uv_coring_REG;
	unsigned int MA_hist2_sum;
	unsigned int hist2_sum;
	unsigned char i;
	unsigned char u_coring,v_coring;
	hist2_sum = 0;
	for(i=0;i<24;i++)
	{
		//hue_hist_2[i] = scaler_rtd_inl( COLOR_ICM_HUE_COUNT_00_reg + (i<<2));
		hist2_sum+= hue_hist_2[i];//already read info. from skin tone detector
	}
	hist2_sum+=1;

	MA_hist2_sum = Lerp(SmartPic_clue->RTNR_MAD_count_Y_avg_ratio, black_white_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][0], black_white_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][1], black_white_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][2], black_white_detect_table[(SmartPic_clue->Remmping_Src_Timing_index)][3], 0, 0);

	dm_uv_coring_REG.regValue = rtd_inl(COLOR_ICM_DM_UV_Coring_reg);
	u_coring = dm_uv_coring_REG.u_coring;
	v_coring = dm_uv_coring_REG.v_coring;

#if black_white_pattern_log
	if(MA_print_count%30 ==0)
	{
		ROSPrintf(" RTNR_MAD_count_Y_sum_avg  = %d\n",RTNR_MAD_count_Y);
		ROSPrintf(" u_coring  = %d\n",u_coring);
		ROSPrintf(" MA_hist2_sum  = %d\n",MA_hist2_sum);
		ROSPrintf(" hist2_sum  = %d\n",hist2_sum);
		ROSPrintf(" (MA_hist2_sum - ((MA_hist2_sum*u_coring)>>7))  = %d\n",(MA_hist2_sum - ((MA_hist2_sum*u_coring)>>7)));
		if((hist2_sum<(MA_hist2_sum - ((MA_hist2_sum*u_coring)>>7))) == 0)
			ROSPrintf("1. %d < %d\n",hist2_sum ,(MA_hist2_sum - ((MA_hist2_sum*u_coring)>>7)));
	}
#endif

	if((hist2_sum<(MA_hist2_sum - ((MA_hist2_sum*u_coring)>>7))))
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

//------------------------------------------------------
// Erin move 100305

//------------------------------------------------------
/**
 * Color histogram ISR
 *
 * @param	void
 * @return	void
 */
//short SHP_Coef [VIP_QUALITY_SOURCE_NUM][VIP_QUALITY_D_PEAKING_NUM];

//--------------------------------------------------
// Move from Pacific Start , Erin 100305
//--------------------------------------------------

unsigned short H_Block_Pre1[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short V_Block_Pre1[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short H_Block_Pre2[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short V_Block_Pre2[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int drvif_color_colorbar_dectector(_clues* SmartPic_clue)//planck_HDMI
{
	unsigned char i;
	unsigned short Left_Total=0;
	unsigned short Right_Total=0;
	unsigned char Left_high=0;
	unsigned char imcolorbar_H_Ratio = 0;
	unsigned char imcolorbar_V_Ratio = 0;
	unsigned char imcolorbar_H_diff = 0;
	unsigned char imcolorbar_V_diff = 0;
	unsigned char imcolorbar_Hue = 0;
	unsigned short Max=0;
	unsigned short Min=65535;
	unsigned short H_Block_Diff[16] ;
	unsigned short V_Block_Diff[16] ;
	unsigned char H_Block_Diff_Count=0;
	unsigned char V_Block_Diff_Count=0;
	static unsigned char print_cnt=0;
	//static int imcolorbar_H_diff_temp = 0, imcolorbar_V_diff_temp=0;

	unsigned short *H_Block = &(SmartPic_clue->Profile_Cal_INFO.H_Block_Y[0]);
	unsigned short *V_Block = &(SmartPic_clue->Profile_Cal_INFO.V_Block_Y[0]);

	for ( i=0 ; i<24 ; i++ )
	{
		if(hue_hist_2[i] != 0){
			imcolorbar_Hue = 1;
			break;
		}
		else{
			imcolorbar_Hue = 0;
		}
	}
	for(i=1;i<10;i++)
	{
		if(H_Block[i]<=Min)
			Min=H_Block[i];
		if(H_Block[i]>=Max)
			Max=H_Block[i];
	}
	if( (Max-Min)*100/(Min+1)<10 )
		imcolorbar_H_Ratio=1;

	if((print_cnt % 120) == 0)
	{
		//ROSPrintf("Max_H=%d\n",Max);
		//ROSPrintf("Min_H=%d\n",Min);
	}

	Max=0;
	Min=65535;
	for(i=0;i<15;i++)
	{
		if(V_Block[i]<=Min)
			Min=V_Block[i];
		if(V_Block[i]>=Max)
			Max=V_Block[i];
	}
	if( (Max-Min)*100/(Min+1)>4 )
		imcolorbar_V_Ratio=1;

	for(i=0;i<8;i++)
	{
		Left_Total+=V_Block[7-i];
		Right_Total+=V_Block[i+8];
	}

if((print_cnt % 300) == 0){

	//ROSPrintf("Max_V=%d\n",Max);
	//ROSPrintf("Min_V=%d\n",Min);

	//ROSPrintf("Left_Total=%d\n",Left_Total);
	//ROSPrintf("Right_Total=%d\n",Right_Total);
}

	if(((Left_Total*103)/(Right_Total+1))>200)
		Left_high=1;

	for(i=0;i<16;i++)
	{
		H_Block_Diff[i] = ABS(H_Block[i], H_Block_Pre2[i] );
		V_Block_Diff[i] = ABS(V_Block[i], V_Block_Pre2[i] );

		if(H_Block_Diff[i] <= 15){
			H_Block_Diff_Count++;
		}
		if(V_Block_Diff[i] <= 15){
			V_Block_Diff_Count++;
		}
	}
	if(H_Block_Diff_Count >= 15){
		imcolorbar_H_diff = 1;
	}
	if(V_Block_Diff_Count >= 15){
		imcolorbar_V_diff = 1;
	}

	for ( i=0 ; i<16 ; i++ )
	{
		H_Block_Pre2[i] = H_Block_Pre1[i];
		V_Block_Pre2[i] = V_Block_Pre1[i];
		H_Block_Pre1[i] = H_Block[i];
		V_Block_Pre1[i] = V_Block[i];
	}

#if 0
	if((print_cnt % 300) == 0){
		for(i=0;i<16;i++)
		{
			ROSPrintf("H_Block_Diff[%d]=%d, V_Block_Diff[%d]=%d\n",i,H_Block_Diff[i],i,V_Block_Diff[i]);
		}
		ROSPrintf("H_Block_Diff_Count=%d, V_Block_Diff_Count=%d\n",H_Block_Diff_Count,V_Block_Diff_Count);
		ROSPrintf("imcolorbar_H_diff_temp=%d, imcolorbar_V_diff_temp=%d\n",imcolorbar_H_diff_temp,imcolorbar_V_diff_temp);
		ROSPrintf("imcolorbar_H_diff=%d, imcolorbar_V_diff=%d, imcolorbar_Hue= %d\n",imcolorbar_H_diff,imcolorbar_V_diff,imcolorbar_Hue);
		ROSPrintf("imcolorbar_H_Ratio=%d, imcolorbar_V_Ratio=%d, Left_high= %d\n",imcolorbar_H_Ratio,imcolorbar_V_Ratio,Left_high);

		if(( imcolorbar_H_diff_temp>=2 || imcolorbar_V_diff_temp>=2))
		{
			ROSPrintf("diff_temp = 1\n");
		}

		if((imcolorbar_H_diff ==1 || imcolorbar_V_diff ==1))
		{
			ROSPrintf("diff = 1\n");
		}

		if(( (imcolorbar_H_diff ==1) || (imcolorbar_V_diff ==1) || ((imcolorbar_H_diff_temp>=2) || (imcolorbar_V_diff_temp>=2))))
		{
			ROSPrintf("mix_diff = 1\n");
		}

	}
#endif

	print_cnt++;
	if((imcolorbar_H_Ratio ==1)  && (imcolorbar_V_Ratio ==1) && (Left_high==1) && ( (imcolorbar_H_diff ==1) || (imcolorbar_V_diff ==1) ) && imcolorbar_Hue==1 )
	{
		if((print_cnt % 120) == 0){
		/*
			ROSPrintf(" i'm a vertical colobar!!\n");
		*/
				print_cnt = 0;
		};
		return 1;
	}
	else
	{
			if(((print_cnt % 120) == 0)){
			/*
				ROSPrintf(" Not a vertical colobar!!\n");
				ROSPrintf("H_Block_Diff_Count=%d, V_Block_Diff_Count=%d\n",H_Block_Diff_Count,V_Block_Diff_Count);
				ROSPrintf("imcolorbar_H_diff_temp=%d, imcolorbar_V_diff_temp=%d\n",imcolorbar_H_diff_temp,imcolorbar_V_diff_temp);
				ROSPrintf("imcolorbar_H_diff=%d, imcolorbar_V_diff=%d, imcolorbar_Hue= %d\n",imcolorbar_H_diff,imcolorbar_V_diff,imcolorbar_Hue);
				ROSPrintf("imcolorbar_H_Ratio=%d, imcolorbar_V_Ratio=%d, Left_high= %d\n",imcolorbar_H_Ratio,imcolorbar_V_Ratio,Left_high);
				ROSPrintf("Max_H=%d\n",Max);
				ROSPrintf("Min_H=%d\n",Min);
			*/
				print_cnt = 0;
			}

		return 0;
	}

}


int drvif_color_colorbar_dectector_by_SatHueProfile(_clues* SmartPic_clue)//planck_HDMI
{
	unsigned char i;
	unsigned char imcolorbar_H_diff = 0;
	unsigned char imcolorbar_V_diff = 0;
	unsigned char imcolorbar_Hue = 0;

	unsigned short Max_range[6]={0,0,0,0,0,0};
	unsigned short Min_range[6]={65535,65535,65535,65535,65535,65535};
	unsigned short diff[6]={0,0,0,0,0,0},max_ratio=0,min_ratio=0;
	unsigned short diff_min_index=6,min_diff=65535,diff_max_index=6,max_diff=0;

	unsigned short H_Block_Diff[16] ;
	unsigned short V_Block_Diff[16] ;
	unsigned char H_Block_Diff_Count=0;
	unsigned char V_Block_Diff_Count=0;

	int symmetry_score = 0, hue_score=0, color_bar_score =0, HV_diff_score=0;
	static short debounce = 5, pre_color_bar_score=255;
	unsigned short colorbar_hue_ratio_total = 0, avg_hue_ratio=0, diff_sum_hue_ratio=0;
	unsigned short hue_ratio[6];
	unsigned char sat_flag = 0;

	unsigned short *H_Block = &(SmartPic_clue->Profile_Cal_INFO.H_Block_Y[0]);
	unsigned short *V_Block = &(SmartPic_clue->Profile_Cal_INFO.V_Block_Y[0]);

	//== symmetry judge for H====
	for(i=0;i<16;i++)
	{
		//up
	 	if((i>=1) && (i<=7))
	  	{
	  		if(H_Block[i]<=Min_range[0])
				Min_range[0]=H_Block[i];
			if(H_Block[i]>=Max_range[0])
				Max_range[0]=H_Block[i];
	 	}
		//middle
		if((i>=6) && (i<=10))
	  	{
	  		if(H_Block[i]<=Min_range[1])
				Min_range[1]=H_Block[i];
			if(H_Block[i]>=Max_range[1])
				Max_range[1]=H_Block[i];
	 	}
		//down
		if((i>=8) && (i<=14))
	  	{
	  		if(H_Block[i]<=Min_range[2])
				Min_range[2]=H_Block[i];
			if(H_Block[i]>=Max_range[2])
				Max_range[2]=H_Block[i];
	 	}
		//extend_up
		if((i>=1) && (i<=10))
	  	{
	  		if(H_Block[i]<=Min_range[3])
				Min_range[3]=H_Block[i];
			if(H_Block[i]>=Max_range[3])
				Max_range[3]=H_Block[i];
	 	}
		//extend_down
		if((i>=6) && (i<=14))
	  	{
	  		if(H_Block[i]<=Min_range[4])
				Min_range[4]=H_Block[i];
			if(H_Block[i]>=Max_range[4])
				Max_range[4]=H_Block[i];
	 	}
		//up_two_bin
	 	if(/*(i>=0) && */(i<=1))
	  	{
	  		if(H_Block[i]<=Min_range[5])
				Min_range[5]=H_Block[i];
			if(H_Block[i]>=Max_range[5])
				Max_range[5]=H_Block[i];
	 	}
	}

	for(i=0;i<6;i++)
	{
		diff[i]=Max_range[i]-Min_range[i];
		if(diff[i]<=min_diff)
		{
			min_diff = diff[i];
			diff_min_index = i;
		}
		if(diff[i]>=max_diff)
		{
			max_diff = diff[i];
			diff_max_index = i;
		}
	}

	min_ratio = (min_diff*1000)/(Min_range[diff_min_index]+1);
	max_ratio = (max_diff*1000)/(Min_range[diff_max_index]+1);

	if(min_ratio<=10)
	{
		symmetry_score = min_ratio;
	}
	else
	{
		symmetry_score = max_ratio - min_ratio;
		if(symmetry_score<=0)
		{
			symmetry_score =0;
		}
		else if(symmetry_score >= 64)
		{
			symmetry_score =64;
		}
	}

	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ColorBar==TRUE))
	{
		for(i=0;i<6;i++)
		{
			ROSPrintf("diff[%d]=%d\n",i,diff[i]);
		}
		ROSPrintf("max_rario=%d, %d\n",max_ratio,Min_range[diff_min_index]);
		ROSPrintf("min_rario=%d, %d\n",min_ratio,Max_range[diff_min_index]);
	}
	//========================

	//======== motion judge =============
	for(i=0;i<16;i++)
	{
		H_Block_Diff[i] = ABS(H_Block[i], H_Block_Pre2[i] );
		V_Block_Diff[i] = ABS(V_Block[i], V_Block_Pre2[i] );

		if(H_Block_Diff[i] <= 15){
			H_Block_Diff_Count++;
		}
		if(V_Block_Diff[i] <= 15){
			V_Block_Diff_Count++;
		}
	}

	if(H_Block_Diff_Count >= 10)
	{
		imcolorbar_H_diff = 1;
	}

	if(V_Block_Diff_Count >= 10)
	{
		imcolorbar_V_diff = 1;
	}

	HV_diff_score = (H_Block_Diff_Count + V_Block_Diff_Count) - 20;

	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ColorBar==TRUE))
	{
		ROSPrintf("H_Block_Diff_Count=%d, V_Block_Diff_Count=%d\n",H_Block_Diff_Count,V_Block_Diff_Count);
	}
	//===============================

	//====== hue judge= =======

	//R-> hue[2], G->hue[10], B->hue[17]~[18], C->hue[13]~[14], M->hue[21]~[22] ,Y->hue[6]

	hue_ratio[0]=(SmartPic_clue->Hue_Main_His_Ratio)[1]+(SmartPic_clue->Hue_Main_His_Ratio)[2];//R
	hue_ratio[1]=(SmartPic_clue->Hue_Main_His_Ratio)[9]+(SmartPic_clue->Hue_Main_His_Ratio)[10];//G
	hue_ratio[2]=(SmartPic_clue->Hue_Main_His_Ratio)[17]+(SmartPic_clue->Hue_Main_His_Ratio)[18];//B
	hue_ratio[3]=(SmartPic_clue->Hue_Main_His_Ratio)[13]+(SmartPic_clue->Hue_Main_His_Ratio)[14];//C
	hue_ratio[4]=(SmartPic_clue->Hue_Main_His_Ratio)[21]+(SmartPic_clue->Hue_Main_His_Ratio)[22];//M
	hue_ratio[5]=(SmartPic_clue->Hue_Main_His_Ratio)[5]+(SmartPic_clue->Hue_Main_His_Ratio)[6];//Y

	for ( i=0 ; i<6 ; i++ )
	{
		if(hue_ratio[i] != 0)
		{
			imcolorbar_Hue = 1;
			break;
		}
		else
		{
			imcolorbar_Hue = 0;
		}
	}

	for(i = 0; i < 6; i++)
	{
		colorbar_hue_ratio_total += hue_ratio[i];
	}

	avg_hue_ratio = colorbar_hue_ratio_total/6;

	for(i = 0; i < 6; i++)
	{
		diff_sum_hue_ratio += ABS(hue_ratio[i],avg_hue_ratio);
	}

	//more score, less colorbar
	if(colorbar_hue_ratio_total<=800)
	{
		hue_score = 64;
	}
	else
	{
		hue_score = diff_sum_hue_ratio-70;
		if(hue_score<=0)
		{
			hue_score =0;
		}
		else if(hue_score>=64)
		{
			hue_score =64;
		}
	}
	//=====================

	//====== sat judge= =======
	if(SmartPic_clue->sat_ratio_mean_value>=43)
	{
		sat_flag = 1;
	}
	else
	{
		sat_flag = 0;
	}
	//=====================

	//color_bar_score => 0 ~ 160, more score, more colorbar
	color_bar_score = ((64- hue_score)+(64-symmetry_score))*sat_flag*imcolorbar_Hue + HV_diff_score;

	if(color_bar_score<=0)
	{
		color_bar_score =0;
	}

	if( abs_value(pre_color_bar_score-color_bar_score)>1 )
	{
		debounce++;
	}
	else
	{
		debounce--;
	}

	if(debounce<0)
		debounce =0;

	if(debounce>5)
		debounce =5;

	if(debounce>=5)
	{
		pre_color_bar_score = color_bar_score;
	}

	for ( i=0 ; i<16 ; i++ )
	{
		H_Block_Pre2[i] = H_Block_Pre1[i];
		V_Block_Pre2[i] = V_Block_Pre1[i];
		H_Block_Pre1[i] = H_Block[i];
		V_Block_Pre1[i] = V_Block[i];
	}

#if 1
if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ColorBar==TRUE))
{
	ROSPrintf("=== Profile ===\n");
	ROSPrintf(" DispLen=%d, DispWid=%d\n",SmartPic_clue->SCALER_DISP_INFO.DispLen,SmartPic_clue->SCALER_DISP_INFO.DispWid);
	for(i=0;i<16;i++)
	{
		//ROSPrintf("[%d ] H = %d, H_Pre2=%d, H_diff=%d, H_Pre1=%d \n",i,H_Block[i],H_Block_Pre2[i],H_Block_Diff[i],H_Block_Pre1[i]);
		ROSPrintf("[%d ] H = %d\n",i,H_Block[i]);

	}
/*
	for(i=0;i<16;i++)
	{
		//ROSPrintf("[%d ] V = %d, V_Pre2=%d, V_diff=%d, V_Pre1=%d \n",i,V_Block[i],V_Block_Pre2[i],V_Block_Diff[i],V_Block_Pre1[i]);
		ROSPrintf("[%d ] V = %d\n",i,V_Block[i]);
	}
*/
	for(i=0;i<24;i++)
	{
		ROSPrintf("[%d ] HUE = %d, ratio=%d\n",i,hue_hist_2[i], (SmartPic_clue->Hue_Main_His_Ratio)[i]);
	}

	for(i = 0; i < 6; i++)
	{
		ROSPrintf("hue_ratio[%d] = %d\n",i,hue_ratio[i]);
	}

	for(i = 0; i < 32; i++)
	{
		ROSPrintf("[%d ] sat = %d\n",i,(SmartPic_clue->Sat_Main_His_Ratio)[i]);
	}
	ROSPrintf("sat_ratio_mean_value=%d\n",SmartPic_clue->sat_ratio_mean_value);
	ROSPrintf("sat_sumcnt_ratio sat_=%d, hue = %d\n",SmartPic_clue->sat_sumcnt_ratio,SmartPic_clue->hue_sumcnt_ratio);
	ROSPrintf("colorbar_hue_ratio_total=%d\n",colorbar_hue_ratio_total);
	ROSPrintf("avg_hue_ratio=%d,diff_sum_hue_ratio=%d\n",avg_hue_ratio,diff_sum_hue_ratio);
	ROSPrintf("sat_flag=%d,time_H_diff=%d\n",sat_flag, imcolorbar_H_diff);
	ROSPrintf("H_Block_Diff_Count=%d, V_Block_Diff_Count=%d\n",H_Block_Diff_Count,V_Block_Diff_Count);
	ROSPrintf("HV_diff_score=%d\n",HV_diff_score);
	ROSPrintf("symmetry_score=%d\n",symmetry_score);
	ROSPrintf("hue_score=%d\n",hue_score);
	ROSPrintf("color_bar_score=%d,pre_color_bar_score=%d\n",color_bar_score,pre_color_bar_score);

	ROSPrintf("=== detector flag ===\n");
	ROSPrintf("imcolorbar_Hue= %d\n",imcolorbar_Hue);


}
#endif

	return pre_color_bar_score;

}

void scalerVIP_DCR_OLD()
{
	if(DCR_TABLE == NULL)
		return;

	if(DCR_set_mapping==FALSE){
		int i;
		for(i=0; i<5;i++) {
			MV_ref[i] = DCR_TABLE[0*DCT_TABLE_NUM+i];
			BL_ref[i] = DCR_TABLE[1*DCT_TABLE_NUM+i];
			BL_Duty_ref[i]=DCR_TABLE[2*DCT_TABLE_NUM+i];
			Duty_ref[i]=DCR_TABLE[3*DCT_TABLE_NUM+i];

			if((MA_print_count % 300) == 0){
				rtd_pr_vpq_isr_notice("\n [DCR] MV_ref[%d]=%d, BL_ref[%d]=%d,",i,MV_ref[i],i,BL_ref[i]);
				rtd_pr_vpq_isr_notice("\n [DCR] BL_Duty_ref[%d]=%d, Duty_ref[%d]=%d\n",i,BL_Duty_ref[i],i,Duty_ref[i]);
			}
		}

		m_DCR_mode = DCR_TABLE[4*DCT_TABLE_NUM+0];
		m_frame_need_SC=DCR_TABLE[4*DCT_TABLE_NUM+1];
		m_frame_need_NML=DCR_TABLE[4*DCT_TABLE_NUM+2];
		m_stable_buf_thl=DCR_TABLE[4*DCT_TABLE_NUM+3];
		m_DZ_break_thl=DCR_TABLE[4*DCT_TABLE_NUM+4];
		if((MA_print_count % 300) == 0)
			rtd_pr_vpq_isr_notice("\n [DCR] DCR_TABLE[4]=%d,  %d,	%d,  %d\n",m_frame_need_SC,m_frame_need_NML,m_stable_buf_thl,m_DZ_break_thl);

		DCR_set_mapping=TRUE;
	}

	/* ======================= */
	/* ======= On/Off DCR ======= */
	/* ======================= */
	if(m_DCR_On_Off == TRUE) {
		drvif_color_ma_DCR_new(m_frame_need_SC, m_frame_need_NML, m_stable_buf_thl, m_DZ_break_thl);
	}

	m_last_DCR_On_Off=m_DCR_On_Off;

	log_count++;

}

//20121106 roger add
unsigned char scalerVIP_Get_Profile_Motion()
{
	return Profile_Motion_Detect_Value;
}

//roger add 20110830, for motion detect for all source
void scalerVIP_Profile_Motion_Detect(unsigned char display, _clues* SmartPic_clue)
{
	static unsigned short H_Block_Pre_PMD[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	//roger add 20110826, PMD = profile motion detect
	static unsigned short V_Block_Pre_PMD[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	//roger add 20110826, PMD = profile motion detect
	unsigned int H_PMDV_temp = 0;
	unsigned int V_PMDV_temp = 0;
	unsigned int H_diff_Sum = 0;
	unsigned int V_diff_Sum = 0;
	int i;
	unsigned short *H_Block = &(SmartPic_clue->Profile_Cal_INFO.H_Block_Y[0]);
	unsigned short *V_Block = &(SmartPic_clue->Profile_Cal_INFO.V_Block_Y[0]);

	for (i = 0; i < 16; i++)
	{
		H_diff_Sum = H_diff_Sum + ABS(H_Block[i], H_Block_Pre_PMD[i]);
 		V_diff_Sum = V_diff_Sum + ABS(V_Block[i], V_Block_Pre_PMD[i]);
	}

	//H: xxxK, V: xxxK is the sum of the white patten of block 0 to 15, but it's only for reference
	//the range of H_PMDV is 50, the range of V_PMDV is also 50
	switch(SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:		//H: 114K, V: 38K
		case _MODE_576I:		//H: 139K, V: 46K
			H_PMDV_temp = H_diff_Sum >> 4;
			V_PMDV_temp = V_diff_Sum >> 3;
			break;
		case _MODE_480P:	//H: 237K, V: 79K
		case _MODE_576P:	//H: 286K, V: 95K
			H_PMDV_temp = H_diff_Sum >> 5;
			V_PMDV_temp = V_diff_Sum >> 4;
			break;
		case _MODE_720P50:	//H: 351K, V: 131K
		case _MODE_720P60:	//H: 351K, V: 131K
			H_PMDV_temp = H_diff_Sum >> 5;
			V_PMDV_temp = V_diff_Sum >> 4;
			break;
		case _MODE_1080I25:	//H: 261K, V: 131K
		case _MODE_1080I30:	//H: 261K, V: 131K
			H_PMDV_temp = H_diff_Sum >> 6;
			V_PMDV_temp = V_diff_Sum >> 4;
			break;
		case _MODE_1080P50:	//H: 531K, V: 131K
		case _MODE_1080P60:	//H: 531K, V: 131K
			H_PMDV_temp = H_diff_Sum >> 6;
			V_PMDV_temp = V_diff_Sum >> 4;
			break;
		default:
			H_PMDV_temp = H_diff_Sum >> 5;
			V_PMDV_temp = V_diff_Sum >> 4;
			break;
	}

	#ifdef CONFIG_USE_RT_STORAGE_NO_DDR
	//20121106 roger add, ONLY for Pacific' no DDR set. In DDR set, don't use this
	switch(SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:
		case _MODE_576I:
		case _MODE_1080I25:
		case _MODE_1080I30:
			//rtd_log(_LOGGER_INFO, "scalerVIP_Profile_Motion_Detect, i-signal re-adjust H_PMDV!! H_PMDV_temp = %d\n", H_PMDV_temp);
			if (H_PMDV_temp > 15)
				H_PMDV_temp = H_PMDV_temp - 15;
			else
				H_PMDV_temp = 0;
			break;
		default:
			break;
	}
	#endif

	H_PMDV_temp = (H_PMDV_temp > 50) ? 50 : H_PMDV_temp;
	V_PMDV_temp = (V_PMDV_temp > 50) ? 50 : V_PMDV_temp;
	Profile_Motion_Detect_Value = H_PMDV_temp + V_PMDV_temp;
	if (Profile_Motion_Detect_Value == 0) Profile_Motion_Detect_Value = 1;		//avoid the value=0

	for (i = 0; i < 16; i++)
	{
		H_Block_Pre_PMD[i] = H_Block[i];
		V_Block_Pre_PMD[i] = V_Block[i];
	}

#if 0	//debug message
	static unsigned int print_cnt = 0;
	print_cnt++;
	if ((print_cnt % 10) == 0)
	{
		//for (i = 0; i < 16; i++)
			//rtd_pr_vpq_isr_info("H_Block[%d] = %d, V_Block[%d] = %d\n", i, H_Block[i], i, V_Block[i]);

		rtd_log(_LOGGER_INFO, "source = %d\n", info->ucMode_Curr);
		rtd_log(_LOGGER_INFO, "H_diff_Sum = %d, \nV_diff_Sum = %d\n", H_diff_Sum, V_diff_Sum);
		rtd_log(_LOGGER_INFO, "H_PMDV_temp = %d, \nV_PMDV_temp = %d, \nProfile_Motion_Detect_Value = %d\n", H_PMDV_temp, V_PMDV_temp, Profile_Motion_Detect_Value);
	}
#endif

	return;
}

/*=============================================
/ Function name:drvif_color_ma_DCR_new
/ Input:
/       [Frame_need_SC] : how many frame need to achieve target when scene change.
/       [Frame_need_NML] : how many frame need to achieve target nomally.
/       [stable_buf_thl] : Define Dead Zone thl.
/       [DZ_break_thl] : how many frame to break Dead Zone.
=============================================*/
void drvif_color_ma_DCR_new(unsigned short Frame_need_SC, unsigned short Frame_need_NML, unsigned short stable_buf_thl, unsigned short DZ_break_thl)
{
        /* BL_prev : previous Backlight */
        /* BL_current : current Backlight */
        /* BL_desired : desired calculating Backlight */
        /* BL_desired_prev : previous desired calculating Backlight */
        /* BL_target : tracking target Backlight */
        static unsigned char   BL_max=100,BL_min=0;
        static unsigned int mappingValue=255;
        static int BL_current=100,BL_prev=100,BL_desired=100,BL_desired_prev=100,BL_target=100,MV=0,BL_Duty=0;
        static unsigned short Frame_need=30;
        static unsigned char SegWide=10,dead_zone_break_count=0;
        static unsigned char frame_count=0,frame_leave=60,t=0,vel=0;
        static int   diff=0,stable_buffer=0;
        static unsigned char   Segment=Segment_F;
        static unsigned char is_DeadZone=false;
		#if 0
		static unsigned short DCR_count = 0;
		DCR_count++;
		if (DCR_count % 60 == 0)
			ROSPrintf("\n drvif_color_ma_DCR_new, MV = %d, m_BL_LV_From_User = %d, BL_desired = %d, m_DCR_mode = %d\n", (SmartPic_clue->Hist_Y_Mean_Value), m_BL_LV_From_User, BL_desired, m_DCR_mode);
		#endif
	BL_max = m_DCR_UI_Max_Value;
	BL_min = m_DCR_UI_Min_Value;
        MV=(SmartPic_clue->Hist_Y_Mean_Value);
        frame_count++;

        if(m_last_DCR_On_Off==0){
            BL_current=m_BL_LV_From_User;
        }

        /*======================*/
        /*     DCR Calculating Backlight       */
        /*======================*/
        /* DCR fomulation */
        if(MV<MV_ref[0])
            Segment = Segment_A;
        else if(MV<MV_ref[1] && MV>=MV_ref[0])
            Segment = Segment_B;
        else if(MV<MV_ref[2] && MV>=MV_ref[1])
            Segment = Segment_C;
        else if(MV<MV_ref[3] && MV>=MV_ref[2])
            Segment = Segment_D;
        else if(MV<MV_ref[4] && MV>=MV_ref[3])
            Segment = Segment_E;
        else if(MV>=MV_ref[4])
            Segment = Segment_F;


        /* DCR fomulation */
        switch(Segment) {
                case Segment_A:
                                BL_desired = BL_ref[0];
                    break;
                case Segment_B:
                case Segment_C:
                case Segment_D:
                case Segment_E:
                                BL_desired=(BL_ref[Segment]-BL_ref[Segment-1])*((MV)-MV_ref[Segment-1])/(MV_ref[Segment]-MV_ref[Segment-1])+BL_ref[Segment-1];
                    break;
                case Segment_F:
                                BL_desired = BL_ref[4];
                    break;
                default:
                                BL_desired = BL_ref[4];
                    break;
        }

		//20120523 roger add for slave mode
		if (m_DCR_mode == DCR_SLAVE_ISR)
			BL_desired = (BL_desired * m_BL_LV_From_User) / 100;

        if((RPC_SmartPic_clue->SceneChange)==1) {/* 20100901 LeoChen add */

            Frame_need=Frame_need_SC;
        } else {
            Frame_need=Frame_need_NML;
        }
        /*=============*/
        /*     DCR Tracking     */
        /*=============*/
        /* ========== accumulate difference of backlight  ========== */
        stable_buffer+=(BL_desired-BL_desired_prev);
        /* ========== RDBL : Reliable Desired Backlight  ========== */
        if(1){//RDBL enable

                /* ==========  Define Dead zone for stable desired backlight  ========== */
                if(stable_buffer>stable_buf_thl||stable_buffer<-stable_buf_thl){//break dead zone

                        is_DeadZone=false;
                        BL_target=BL_desired; /* update desired backlight */
                        stable_buffer=0; /* reset accumulating */
                        dead_zone_break_count=0;
                }
                else if(stable_buffer!=0) {//dead zone with umstable desired backlight

                        is_DeadZone=true;
                        if(dead_zone_break_count==DZ_break_thl) {

                                BL_target=BL_desired;/* update desired backlight */
                                stable_buffer=0; /* reset accumulating */
                                frame_leave=Frame_need-DZ_break_thl;
                                frame_leave=frame_leave>0?frame_leave:-frame_leave;
                                dead_zone_break_count=0;
                        } else {
                                dead_zone_break_count++;
                        }
                } else {//dead zone with stable desired backlight

                        is_DeadZone=true;
                        dead_zone_break_count=0;
                }
        } else {
                BL_target=BL_desired;
                stable_buf_thl=0;
        }

        /*  ========== calculating difference between desired and present  ========== */
        diff=ABS(BL_target,BL_current);
#ifdef DCR_debug_en
	if((MA_print_count % 300) == 0) {
               rtd_pr_vpq_isr_notice("\n >>>>>>>>>>>> &&&&&&&&DCR1  BL_target %d\n",BL_target);
               rtd_pr_vpq_isr_notice("\n >>>>>>>>>>>> &&&&&&&&DCR1  BL_current %d\n",BL_current);
        }
#endif
        if(diff!=0) {
                if(is_DeadZone){//dead zone

                        frame_leave--;
                        frame_leave=frame_leave<1?1:frame_leave; /* avoid divide 0 */
                } else {
                        frame_leave=Frame_need;
                        stable_buffer=0;

                }
                /* ========== Adaptive SegWide ========== */
                if(frame_leave<diff) {//stage1

                        vel=diff/(frame_leave+1);
                } else {//stage2

                        SegWide=/*(diff==0)?frame_leave:*/(frame_leave/diff);
                        t=(t<(SegWide))?(t+1):0;
                        vel=(t==0)?1:0;
                }

                if((BL_target)>(BL_current))
                        BL_current=BL_current+vel;
                else
                        BL_current=BL_current-vel;
        }


        /*  ========== update BL_desired_prev  ========== */
        BL_desired_prev=BL_desired;
        BL_current=(BL_current>=BL_max)?BL_max:((BL_current<=BL_min)?BL_min:BL_current);

#ifdef DCR_debug_en
        if((MA_print_count % 300) == 0) {
               rtd_pr_vpq_isr_notice("\n >>>>>>>>>>>> &&&&&&&&Histogram_Mean_value %d\n",(SmartPic_clue->Hist_Y_Mean_Value));
               rtd_pr_vpq_isr_notice("\n >>>>>>>>>>>> &&&&&&&&DCR1  BL_desired %d\n",BL_desired);
               rtd_pr_vpq_isr_notice("\n >>>>>>>>>>>> &&&&&&&&DCR1  BL_current %d\n",BL_current);
        }
#endif

        /*===================*/
        /*== DCR Writing to BL Reg==*/
        /*===================*/
#ifdef DCR_debug_en
	if((MA_print_count % 300) == 0)
		rtd_pr_vpq_isr_notice("\n >>>>>>>>>>>> &&&&&&&&DCR	BL_current=%d,BL_prev=%d\n",BL_current, BL_prev);

	if((MA_print_count % 300) == 0){
		rtd_pr_vpq_isr_notice("\n >>>>>>>>>>>> &&&&&&&&DCR  BL_Duty=%d, mode= %d\n",BL_Duty,m_isIncreaseMode);
		rtd_pr_vpq_isr_notice("\n >>>>>>>>>>>> &&&&&&&&DCR  m_BL_LV_Act_Max=%d, m_BL_LV_Act_Min= %d\n",m_BL_LV_Act_Max, m_BL_LV_Act_Min);
		rtd_pr_vpq_isr_notice("\n >>>>>>>>>>>> &&&&&&&&DCR  BL_max=%d, BL_min= %d\n",BL_max, BL_min);
        }
#endif
    	if((BL_current != BL_prev) || (m_DCR_refresh == TRUE)) {// || (m_last_DCR_On_Off==0))

			 //ROSPrintf("Histogram_Mean_value %d\n",(SmartPic_clue->Hist_Y_Mean_Value));
			 //ROSPrintf("DCR  BL_current %d\n",BL_current);
			 m_DCR_refresh = FALSE;

                /* ====   RE-MAPPING  :  mapping from backlight to PWM Duty  ==== */
                if(BL_current<BL_Duty_ref[0])
                    Segment = Segment_A;
                else if(BL_current<BL_Duty_ref[1] && BL_current>=BL_Duty_ref[0])
                    Segment = Segment_B;
                else if(BL_current<BL_Duty_ref[2] && BL_current>=BL_Duty_ref[1])
                    Segment = Segment_C;
                else if(BL_current<BL_Duty_ref[3] && BL_current>=BL_Duty_ref[2])
                    Segment = Segment_D;
                else if(BL_current<BL_Duty_ref[4] && BL_current>=BL_Duty_ref[3])
                    Segment = Segment_E;
                else if(BL_current>=BL_Duty_ref[4])
                    Segment = Segment_F;

                switch(Segment) {
                        case Segment_A:
                                        BL_Duty = Duty_ref[0];
                            break;
                        case Segment_B:
                        case Segment_C:
                        case Segment_D:
                        case Segment_E:
                                        BL_Duty=(Duty_ref[Segment]-Duty_ref[Segment-1])*((BL_current)-BL_Duty_ref[Segment-1])/(BL_Duty_ref[Segment]-BL_Duty_ref[Segment-1])+Duty_ref[Segment-1];
                            break;
                        case Segment_F:
                                        BL_Duty = Duty_ref[4];
                            break;
                        default:
                                        BL_Duty = Duty_ref[4];
                            break;
                }

                if(m_isIncreaseMode) {
		   			mappingValue = (BL_Duty-BL_min)*(m_BL_LV_Act_Max - m_BL_LV_Act_Min)/(BL_max-BL_min+1) + m_BL_LV_Act_Min;
                } else {
		  			mappingValue = m_BL_LV_Act_Max-((BL_Duty-BL_min)*(m_BL_LV_Act_Max - m_BL_LV_Act_Min)/(BL_max-BL_min+1) + m_BL_LV_Act_Min);
                }

	      mappingValue &= 0xFF;
		  //ROSPrintf("DCR  mappingValue %d\n",mappingValue);

           	SmartPic_clue->DCR_GDBC_mappingValue = mappingValue;
#ifdef DCR_debug_en
		rtd_pr_vpq_isr_notice("\n >>>>>>>>>>>> &&&&&&&&DCR  mappingValue=%d, BL_Duty= %d\n",mappingValue, BL_Duty);
#endif
                 BL_prev=BL_current;
    	}
}
//===============end_DCR=============

int abs_dcc( int input )
{
    return ( input < 0 ) ? -input : input;
}

unsigned int Lerp(unsigned int X_current, unsigned int X_1, unsigned int X_0, unsigned int Y_1, unsigned int Y_0, unsigned int x_offset_1, unsigned int x_offset_0)
{
	unsigned int Lerp_value;

	if ((Y_1 - Y_0) == 0)
		Lerp_value = Y_0;
	else
	{
		if (X_current <= (X_0 + x_offset_0))
			Lerp_value = Y_0;
		else if (X_current >= (X_1 - x_offset_1))
			Lerp_value = Y_1;
		else
		{
			//Lerp_value = (Y_0 + (((X_current - (X_0 + x_offset_0)) * (Y_1 - Y_0) * 8) / ((X_1 - x_offset_1) - (X_0 + x_offset_0))) / 8);
			if(Y_1 > Y_0)	//	if positive lerp
				Lerp_value = (Y_0 + (((X_current - X_0 - x_offset_0) * (Y_1 - Y_0) * 8) / (X_1 - X_0 - x_offset_1 - x_offset_0)) / 8);
			else	//	if negative lerp
				Lerp_value = (Y_0 - (((X_current - X_0 - x_offset_0) * (Y_0 - Y_1) * 8) / (X_1 - X_0 - x_offset_1 - x_offset_0)) / 8);
		}
	}
	return Lerp_value;
}

//=========================== Pure Color Detection ===========================
unsigned int di_pow(unsigned int x, unsigned int p)
{
	int i=0;
	unsigned int r = 1;

	  if(p == 0) return 1;
	  if(x == 0) return 0;


	for(i=0;i<p;i++)
	{
		r*=x;
	}
	return r;
/*
	  unsigned int r = 1;
	  for(;;)
	  {
	  	if(p & 1)
			r *= x;
	    	if((p >>= 1) == 0)
			return r;
	   	 x *= x;
	  }
	  */
}


unsigned int DCCsqrt_table[4096]={ // this table means SQRT(X)*1024
0x0000,0x0400,0x05a8,0x06ed,0x0800,0x08f1,0x09cc,0x0a95,
0x0b50,0x0c00,0x0ca6,0x0d44,0x0ddb,0x0e6c,0x0ef7,0x0f7d,
0x1000,0x107e,0x10f8,0x116f,0x11e3,0x1254,0x12c2,0x132e,
0x1398,0x1400,0x1465,0x14c8,0x152a,0x158a,0x15e8,0x1645,
0x16a0,0x16fa,0x1752,0x17aa,0x1800,0x1854,0x18a8,0x18fa,
0x194c,0x199c,0x19ec,0x1a3a,0x1a88,0x1ad5,0x1b21,0x1b6c,
0x1bb6,0x1c00,0x1c48,0x1c90,0x1cd8,0x1d1e,0x1d64,0x1daa,
0x1dee,0x1e33,0x1e76,0x1eb9,0x1efb,0x1f3d,0x1f7e,0x1fbf,
0x2000,0x203f,0x207f,0x20bd,0x20fc,0x2139,0x2177,0x21b4,
0x21f0,0x222d,0x2268,0x22a4,0x22df,0x2319,0x2353,0x238d,
0x23c6,0x2400,0x2438,0x2471,0x24a9,0x24e0,0x2518,0x254f,
0x2585,0x25bc,0x25f2,0x2628,0x265d,0x2693,0x26c8,0x26fc,
0x2731,0x2765,0x2799,0x27cc,0x2800,0x2833,0x2865,0x2898,
0x28ca,0x28fc,0x292e,0x2960,0x2991,0x29c2,0x29f3,0x2a24,
0x2a54,0x2a85,0x2ab5,0x2ae5,0x2b14,0x2b44,0x2b73,0x2ba2,
0x2bd1,0x2c00,0x2c2e,0x2c5c,0x2c8a,0x2cb8,0x2ce6,0x2d13,
0x2d41,0x2d6e,0x2d9b,0x2dc8,0x2df4,0x2e21,0x2e4d,0x2e79,
0x2ea5,0x2ed1,0x2efd,0x2f28,0x2f54,0x2f7f,0x2faa,0x2fd5,
0x3000,0x302a,0x3055,0x307f,0x30a9,0x30d3,0x30fd,0x3127,
0x3150,0x317a,0x31a3,0x31cc,0x31f5,0x321e,0x3247,0x3270,
0x3298,0x32c1,0x32e9,0x3311,0x3339,0x3361,0x3389,0x33b0,
0x33d8,0x3400,0x3427,0x344e,0x3475,0x349c,0x34c3,0x34ea,
0x3510,0x3537,0x355d,0x3584,0x35aa,0x35d0,0x35f6,0x361c,
0x3642,0x3667,0x368d,0x36b2,0x36d8,0x36fd,0x3722,0x3747,
0x376c,0x3791,0x37b6,0x37db,0x3800,0x3824,0x3848,0x386d,
0x3891,0x38b5,0x38d9,0x38fd,0x3921,0x3945,0x3969,0x398c,
0x39b0,0x39d3,0x39f7,0x3a1a,0x3a3d,0x3a60,0x3a83,0x3aa6,
0x3ac9,0x3aec,0x3b0f,0x3b31,0x3b54,0x3b76,0x3b99,0x3bbb,
0x3bdd,0x3c00,0x3c22,0x3c44,0x3c66,0x3c87,0x3ca9,0x3ccb,
0x3ced,0x3d0e,0x3d30,0x3d51,0x3d72,0x3d94,0x3db5,0x3dd6,
0x3df7,0x3e18,0x3e39,0x3e5a,0x3e7b,0x3e9c,0x3ebc,0x3edd,
0x3efd,0x3f1e,0x3f3e,0x3f5f,0x3f7f,0x3f9f,0x3fbf,0x3fdf,
0x4000,0x401f,0x403f,0x405f,0x407f,0x409f,0x40be,0x40de,
0x40fe,0x411d,0x413c,0x415c,0x417b,0x419a,0x41ba,0x41d9,
0x41f8,0x4217,0x4236,0x4255,0x4273,0x4292,0x42b1,0x42d0,
0x42ee,0x430d,0x432b,0x434a,0x4368,0x4387,0x43a5,0x43c3,
0x43e1,0x4400,0x441e,0x443c,0x445a,0x4478,0x4495,0x44b3,
0x44d1,0x44ef,0x450c,0x452a,0x4548,0x4565,0x4583,0x45a0,
0x45be,0x45db,0x45f8,0x4615,0x4633,0x4650,0x466d,0x468a,
0x46a7,0x46c4,0x46e1,0x46fe,0x471b,0x4737,0x4754,0x4771,
0x478d,0x47aa,0x47c7,0x47e3,0x4800,0x481c,0x4838,0x4855,
0x4871,0x488d,0x48a9,0x48c6,0x48e2,0x48fe,0x491a,0x4936,
0x4952,0x496e,0x498a,0x49a5,0x49c1,0x49dd,0x49f9,0x4a14,
0x4a30,0x4a4b,0x4a67,0x4a83,0x4a9e,0x4ab9,0x4ad5,0x4af0,
0x4b0b,0x4b27,0x4b42,0x4b5d,0x4b78,0x4b93,0x4bae,0x4bca,
0x4be5,0x4c00,0x4c1a,0x4c35,0x4c50,0x4c6b,0x4c86,0x4ca1,
0x4cbb,0x4cd6,0x4cf1,0x4d0b,0x4d26,0x4d40,0x4d5b,0x4d75,
0x4d90,0x4daa,0x4dc4,0x4ddf,0x4df9,0x4e13,0x4e2d,0x4e48,
0x4e62,0x4e7c,0x4e96,0x4eb0,0x4eca,0x4ee4,0x4efe,0x4f18,
0x4f32,0x4f4c,0x4f65,0x4f7f,0x4f99,0x4fb3,0x4fcc,0x4fe6,
0x5000,0x5019,0x5033,0x504c,0x5066,0x507f,0x5099,0x50b2,
0x50cb,0x50e5,0x50fe,0x5117,0x5130,0x514a,0x5163,0x517c,
0x5195,0x51ae,0x51c7,0x51e0,0x51f9,0x5212,0x522b,0x5244,
0x525d,0x5276,0x528f,0x52a7,0x52c0,0x52d9,0x52f2,0x530a,
0x5323,0x533c,0x5354,0x536d,0x5385,0x539e,0x53b6,0x53cf,
0x53e7,0x5400,0x5418,0x5430,0x5449,0x5461,0x5479,0x5491,
0x54a9,0x54c2,0x54da,0x54f2,0x550a,0x5522,0x553a,0x5552,
0x556a,0x5582,0x559a,0x55b2,0x55ca,0x55e2,0x55fa,0x5611,
0x5629,0x5641,0x5659,0x5670,0x5688,0x56a0,0x56b7,0x56cf,
0x56e6,0x56fe,0x5716,0x572d,0x5745,0x575c,0x5773,0x578b,
0x57a2,0x57ba,0x57d1,0x57e8,0x5800,0x5817,0x582e,0x5845,
0x585c,0x5874,0x588b,0x58a2,0x58b9,0x58d0,0x58e7,0x58fe,
0x5915,0x592c,0x5943,0x595a,0x5971,0x5988,0x599f,0x59b5,
0x59cc,0x59e3,0x59fa,0x5a11,0x5a27,0x5a3e,0x5a55,0x5a6b,
0x5a82,0x5a99,0x5aaf,0x5ac6,0x5adc,0x5af3,0x5b09,0x5b20,
0x5b36,0x5b4d,0x5b63,0x5b7a,0x5b90,0x5ba6,0x5bbd,0x5bd3,
0x5be9,0x5c00,0x5c16,0x5c2c,0x5c42,0x5c58,0x5c6f,0x5c85,
0x5c9b,0x5cb1,0x5cc7,0x5cdd,0x5cf3,0x5d09,0x5d1f,0x5d35,
0x5d4b,0x5d61,0x5d77,0x5d8d,0x5da3,0x5db9,0x5dce,0x5de4,
0x5dfa,0x5e10,0x5e26,0x5e3b,0x5e51,0x5e67,0x5e7c,0x5e92,
0x5ea8,0x5ebd,0x5ed3,0x5ee9,0x5efe,0x5f14,0x5f29,0x5f3f,
0x5f54,0x5f6a,0x5f7f,0x5f95,0x5faa,0x5fbf,0x5fd5,0x5fea,
0x6000,0x6015,0x602a,0x603f,0x6055,0x606a,0x607f,0x6094,
0x60aa,0x60bf,0x60d4,0x60e9,0x60fe,0x6113,0x6128,0x613d,
0x6152,0x6168,0x617d,0x6192,0x61a7,0x61bb,0x61d0,0x61e5,
0x61fa,0x620f,0x6224,0x6239,0x624e,0x6263,0x6277,0x628c,
0x62a1,0x62b6,0x62ca,0x62df,0x62f4,0x6309,0x631d,0x6332,
0x6347,0x635b,0x6370,0x6384,0x6399,0x63ad,0x63c2,0x63d7,
0x63eb,0x6400,0x6414,0x6428,0x643d,0x6451,0x6466,0x647a,
0x648e,0x64a3,0x64b7,0x64cb,0x64e0,0x64f4,0x6508,0x651d,
0x6531,0x6545,0x6559,0x656e,0x6582,0x6596,0x65aa,0x65be,
0x65d2,0x65e6,0x65fa,0x660f,0x6623,0x6637,0x664b,0x665f,
0x6673,0x6687,0x669b,0x66af,0x66c3,0x66d6,0x66ea,0x66fe,
0x6712,0x6726,0x673a,0x674e,0x6761,0x6775,0x6789,0x679d,
0x67b1,0x67c4,0x67d8,0x67ec,0x6800,0x6813,0x6827,0x683b,
0x684e,0x6862,0x6875,0x6889,0x689d,0x68b0,0x68c4,0x68d7,
0x68eb,0x68fe,0x6912,0x6925,0x6939,0x694c,0x6960,0x6973,
0x6986,0x699a,0x69ad,0x69c1,0x69d4,0x69e7,0x69fb,0x6a0e,
0x6a21,0x6a35,0x6a48,0x6a5b,0x6a6e,0x6a82,0x6a95,0x6aa8,
0x6abb,0x6ace,0x6ae2,0x6af5,0x6b08,0x6b1b,0x6b2e,0x6b41,
0x6b54,0x6b67,0x6b7a,0x6b8d,0x6ba1,0x6bb4,0x6bc7,0x6bda,
0x6bed,0x6c00,0x6c12,0x6c25,0x6c38,0x6c4b,0x6c5e,0x6c71,
0x6c84,0x6c97,0x6caa,0x6cbc,0x6ccf,0x6ce2,0x6cf5,0x6d08,
0x6d1a,0x6d2d,0x6d40,0x6d53,0x6d65,0x6d78,0x6d8b,0x6d9e,
0x6db0,0x6dc3,0x6dd6,0x6de8,0x6dfb,0x6e0d,0x6e20,0x6e33,
0x6e45,0x6e58,0x6e6a,0x6e7d,0x6e8f,0x6ea2,0x6eb4,0x6ec7,
0x6ed9,0x6eec,0x6efe,0x6f11,0x6f23,0x6f36,0x6f48,0x6f5a,
0x6f6d,0x6f7f,0x6f92,0x6fa4,0x6fb6,0x6fc9,0x6fdb,0x6fed,
0x7000,0x7012,0x7024,0x7036,0x7049,0x705b,0x706d,0x707f,
0x7091,0x70a4,0x70b6,0x70c8,0x70da,0x70ec,0x70fe,0x7110,
0x7123,0x7135,0x7147,0x7159,0x716b,0x717d,0x718f,0x71a1,
0x71b3,0x71c5,0x71d7,0x71e9,0x71fb,0x720d,0x721f,0x7231,
0x7243,0x7255,0x7267,0x7279,0x728a,0x729c,0x72ae,0x72c0,
0x72d2,0x72e4,0x72f5,0x7307,0x7319,0x732b,0x733d,0x734e,
0x7360,0x7372,0x7384,0x7395,0x73a7,0x73b9,0x73ca,0x73dc,
0x73ee,0x7400,0x7411,0x7423,0x7434,0x7446,0x7458,0x7469,
0x747b,0x748c,0x749e,0x74b0,0x74c1,0x74d3,0x74e4,0x74f6,
0x7507,0x7519,0x752a,0x753c,0x754d,0x755f,0x7570,0x7581,
0x7593,0x75a4,0x75b6,0x75c7,0x75d8,0x75ea,0x75fb,0x760d,
0x761e,0x762f,0x7641,0x7652,0x7663,0x7674,0x7686,0x7697,
0x76a8,0x76ba,0x76cb,0x76dc,0x76ed,0x76fe,0x7710,0x7721,
0x7732,0x7743,0x7754,0x7766,0x7777,0x7788,0x7799,0x77aa,
0x77bb,0x77cc,0x77dd,0x77ee,0x7800,0x7811,0x7822,0x7833,
0x7844,0x7855,0x7866,0x7877,0x7888,0x7899,0x78aa,0x78bb,
0x78cc,0x78dd,0x78ee,0x78fe,0x790f,0x7920,0x7931,0x7942,
0x7953,0x7964,0x7975,0x7986,0x7996,0x79a7,0x79b8,0x79c9,
0x79da,0x79eb,0x79fb,0x7a0c,0x7a1d,0x7a2e,0x7a3e,0x7a4f,
0x7a60,0x7a71,0x7a81,0x7a92,0x7aa3,0x7ab3,0x7ac4,0x7ad5,
0x7ae5,0x7af6,0x7b07,0x7b17,0x7b28,0x7b39,0x7b49,0x7b5a,
0x7b6b,0x7b7b,0x7b8c,0x7b9c,0x7bad,0x7bbd,0x7bce,0x7bde,
0x7bef,0x7c00,0x7c10,0x7c21,0x7c31,0x7c41,0x7c52,0x7c62,
0x7c73,0x7c83,0x7c94,0x7ca4,0x7cb5,0x7cc5,0x7cd5,0x7ce6,
0x7cf6,0x7d07,0x7d17,0x7d27,0x7d38,0x7d48,0x7d58,0x7d69,
0x7d79,0x7d89,0x7d9a,0x7daa,0x7dba,0x7dcb,0x7ddb,0x7deb,
0x7dfb,0x7e0c,0x7e1c,0x7e2c,0x7e3c,0x7e4d,0x7e5d,0x7e6d,
0x7e7d,0x7e8d,0x7e9e,0x7eae,0x7ebe,0x7ece,0x7ede,0x7eee,
0x7efe,0x7f0f,0x7f1f,0x7f2f,0x7f3f,0x7f4f,0x7f5f,0x7f6f,
0x7f7f,0x7f8f,0x7f9f,0x7faf,0x7fbf,0x7fcf,0x7fdf,0x7fef,
0x8000,0x800f,0x801f,0x802f,0x803f,0x804f,0x805f,0x806f,
0x807f,0x808f,0x809f,0x80af,0x80bf,0x80cf,0x80df,0x80ef,
0x80ff,0x810e,0x811e,0x812e,0x813e,0x814e,0x815e,0x816d,
0x817d,0x818d,0x819d,0x81ad,0x81bc,0x81cc,0x81dc,0x81ec,
0x81fc,0x820b,0x821b,0x822b,0x823b,0x824a,0x825a,0x826a,
0x8279,0x8289,0x8299,0x82a8,0x82b8,0x82c8,0x82d7,0x82e7,
0x82f7,0x8306,0x8316,0x8326,0x8335,0x8345,0x8354,0x8364,
0x8374,0x8383,0x8393,0x83a2,0x83b2,0x83c1,0x83d1,0x83e0,
0x83f0,0x8400,0x840f,0x841f,0x842e,0x843e,0x844d,0x845c,
0x846c,0x847b,0x848b,0x849a,0x84aa,0x84b9,0x84c9,0x84d8,
0x84e7,0x84f7,0x8506,0x8516,0x8525,0x8534,0x8544,0x8553,
0x8562,0x8572,0x8581,0x8591,0x85a0,0x85af,0x85be,0x85ce,
0x85dd,0x85ec,0x85fc,0x860b,0x861a,0x862a,0x8639,0x8648,
0x8657,0x8667,0x8676,0x8685,0x8694,0x86a3,0x86b3,0x86c2,
0x86d1,0x86e0,0x86ef,0x86ff,0x870e,0x871d,0x872c,0x873b,
0x874a,0x8759,0x8769,0x8778,0x8787,0x8796,0x87a5,0x87b4,
0x87c3,0x87d2,0x87e1,0x87f0,0x8800,0x880f,0x881e,0x882d,
0x883c,0x884b,0x885a,0x8869,0x8878,0x8887,0x8896,0x88a5,
0x88b4,0x88c3,0x88d2,0x88e1,0x88f0,0x88ff,0x890e,0x891c,
0x892b,0x893a,0x8949,0x8958,0x8967,0x8976,0x8985,0x8994,
0x89a3,0x89b2,0x89c0,0x89cf,0x89de,0x89ed,0x89fc,0x8a0b,
0x8a19,0x8a28,0x8a37,0x8a46,0x8a55,0x8a64,0x8a72,0x8a81,
0x8a90,0x8a9f,0x8aad,0x8abc,0x8acb,0x8ada,0x8ae8,0x8af7,
0x8b06,0x8b15,0x8b23,0x8b32,0x8b41,0x8b50,0x8b5e,0x8b6d,
0x8b7c,0x8b8a,0x8b99,0x8ba8,0x8bb6,0x8bc5,0x8bd4,0x8be2,
0x8bf1,0x8c00,0x8c0e,0x8c1d,0x8c2b,0x8c3a,0x8c49,0x8c57,
0x8c66,0x8c74,0x8c83,0x8c91,0x8ca0,0x8caf,0x8cbd,0x8ccc,
0x8cda,0x8ce9,0x8cf7,0x8d06,0x8d14,0x8d23,0x8d31,0x8d40,
0x8d4e,0x8d5d,0x8d6b,0x8d7a,0x8d88,0x8d97,0x8da5,0x8db4,
0x8dc2,0x8dd1,0x8ddf,0x8ded,0x8dfc,0x8e0a,0x8e19,0x8e27,
0x8e36,0x8e44,0x8e52,0x8e61,0x8e6f,0x8e7d,0x8e8c,0x8e9a,
0x8ea9,0x8eb7,0x8ec5,0x8ed4,0x8ee2,0x8ef0,0x8eff,0x8f0d,
0x8f1b,0x8f2a,0x8f38,0x8f46,0x8f54,0x8f63,0x8f71,0x8f7f,
0x8f8e,0x8f9c,0x8faa,0x8fb8,0x8fc7,0x8fd5,0x8fe3,0x8ff1,
0x9000,0x900e,0x901c,0x902a,0x9038,0x9047,0x9055,0x9063,
0x9071,0x907f,0x908d,0x909c,0x90aa,0x90b8,0x90c6,0x90d4,
0x90e2,0x90f0,0x90ff,0x910d,0x911b,0x9129,0x9137,0x9145,
0x9153,0x9161,0x916f,0x917e,0x918c,0x919a,0x91a8,0x91b6,
0x91c4,0x91d2,0x91e0,0x91ee,0x91fc,0x920a,0x9218,0x9226,
0x9234,0x9242,0x9250,0x925e,0x926c,0x927a,0x9288,0x9296,
0x92a4,0x92b2,0x92c0,0x92ce,0x92dc,0x92ea,0x92f8,0x9306,
0x9314,0x9321,0x932f,0x933d,0x934b,0x9359,0x9367,0x9375,
0x9383,0x9391,0x939f,0x93ac,0x93ba,0x93c8,0x93d6,0x93e4,
0x93f2,0x9400,0x940d,0x941b,0x9429,0x9437,0x9445,0x9452,
0x9460,0x946e,0x947c,0x948a,0x9497,0x94a5,0x94b3,0x94c1,
0x94cf,0x94dc,0x94ea,0x94f8,0x9506,0x9513,0x9521,0x952f,
0x953c,0x954a,0x9558,0x9566,0x9573,0x9581,0x958f,0x959c,
0x95aa,0x95b8,0x95c5,0x95d3,0x95e1,0x95ee,0x95fc,0x960a,
0x9617,0x9625,0x9633,0x9640,0x964e,0x965c,0x9669,0x9677,
0x9684,0x9692,0x96a0,0x96ad,0x96bb,0x96c8,0x96d6,0x96e4,
0x96f1,0x96ff,0x970c,0x971a,0x9727,0x9735,0x9742,0x9750,
0x975d,0x976b,0x9779,0x9786,0x9794,0x97a1,0x97af,0x97bc,
0x97ca,0x97d7,0x97e5,0x97f2,0x9800,0x980d,0x981a,0x9828,
0x9835,0x9843,0x9850,0x985e,0x986b,0x9879,0x9886,0x9893,
0x98a1,0x98ae,0x98bc,0x98c9,0x98d6,0x98e4,0x98f1,0x98ff,
0x990c,0x9919,0x9927,0x9934,0x9942,0x994f,0x995c,0x996a,
0x9977,0x9984,0x9992,0x999f,0x99ac,0x99ba,0x99c7,0x99d4,
0x99e2,0x99ef,0x99fc,0x9a09,0x9a17,0x9a24,0x9a31,0x9a3f,
0x9a4c,0x9a59,0x9a66,0x9a74,0x9a81,0x9a8e,0x9a9b,0x9aa9,
0x9ab6,0x9ac3,0x9ad0,0x9ade,0x9aeb,0x9af8,0x9b05,0x9b12,
0x9b20,0x9b2d,0x9b3a,0x9b47,0x9b54,0x9b62,0x9b6f,0x9b7c,
0x9b89,0x9b96,0x9ba3,0x9bb1,0x9bbe,0x9bcb,0x9bd8,0x9be5,
0x9bf2,0x9c00,0x9c0d,0x9c1a,0x9c27,0x9c34,0x9c41,0x9c4e,
0x9c5b,0x9c68,0x9c75,0x9c83,0x9c90,0x9c9d,0x9caa,0x9cb7,
0x9cc4,0x9cd1,0x9cde,0x9ceb,0x9cf8,0x9d05,0x9d12,0x9d1f,
0x9d2c,0x9d39,0x9d46,0x9d53,0x9d60,0x9d6d,0x9d7a,0x9d87,
0x9d94,0x9da1,0x9dae,0x9dbb,0x9dc8,0x9dd5,0x9de2,0x9def,
0x9dfc,0x9e09,0x9e16,0x9e23,0x9e30,0x9e3d,0x9e4a,0x9e57,
0x9e64,0x9e71,0x9e7e,0x9e8b,0x9e98,0x9ea4,0x9eb1,0x9ebe,
0x9ecb,0x9ed8,0x9ee5,0x9ef2,0x9eff,0x9f0c,0x9f18,0x9f25,
0x9f32,0x9f3f,0x9f4c,0x9f59,0x9f66,0x9f72,0x9f7f,0x9f8c,
0x9f99,0x9fa6,0x9fb3,0x9fbf,0x9fcc,0x9fd9,0x9fe6,0x9ff3,
0xa000,0xa00c,0xa019,0xa026,0xa033,0xa03f,0xa04c,0xa059,
0xa066,0xa073,0xa07f,0xa08c,0xa099,0xa0a6,0xa0b2,0xa0bf,
0xa0cc,0xa0d9,0xa0e5,0xa0f2,0xa0ff,0xa10b,0xa118,0xa125,
0xa132,0xa13e,0xa14b,0xa158,0xa164,0xa171,0xa17e,0xa18a,
0xa197,0xa1a4,0xa1b0,0xa1bd,0xa1ca,0xa1d6,0xa1e3,0xa1f0,
0xa1fc,0xa209,0xa216,0xa222,0xa22f,0xa23c,0xa248,0xa255,
0xa261,0xa26e,0xa27b,0xa287,0xa294,0xa2a0,0xa2ad,0xa2ba,
0xa2c6,0xa2d3,0xa2df,0xa2ec,0xa2f8,0xa305,0xa312,0xa31e,
0xa32b,0xa337,0xa344,0xa350,0xa35d,0xa369,0xa376,0xa382,
0xa38f,0xa39b,0xa3a8,0xa3b5,0xa3c1,0xa3ce,0xa3da,0xa3e7,
0xa3f3,0xa400,0xa40c,0xa418,0xa425,0xa431,0xa43e,0xa44a,
0xa457,0xa463,0xa470,0xa47c,0xa489,0xa495,0xa4a2,0xa4ae,
0xa4ba,0xa4c7,0xa4d3,0xa4e0,0xa4ec,0xa4f9,0xa505,0xa511,
0xa51e,0xa52a,0xa537,0xa543,0xa54f,0xa55c,0xa568,0xa574,
0xa581,0xa58d,0xa59a,0xa5a6,0xa5b2,0xa5bf,0xa5cb,0xa5d7,
0xa5e4,0xa5f0,0xa5fc,0xa609,0xa615,0xa621,0xa62e,0xa63a,
0xa646,0xa653,0xa65f,0xa66b,0xa678,0xa684,0xa690,0xa69d,
0xa6a9,0xa6b5,0xa6c1,0xa6ce,0xa6da,0xa6e6,0xa6f2,0xa6ff,
0xa70b,0xa717,0xa724,0xa730,0xa73c,0xa748,0xa754,0xa761,
0xa76d,0xa779,0xa785,0xa792,0xa79e,0xa7aa,0xa7b6,0xa7c3,
0xa7cf,0xa7db,0xa7e7,0xa7f3,0xa800,0xa80c,0xa818,0xa824,
0xa830,0xa83c,0xa849,0xa855,0xa861,0xa86d,0xa879,0xa885,
0xa892,0xa89e,0xa8aa,0xa8b6,0xa8c2,0xa8ce,0xa8da,0xa8e6,
0xa8f3,0xa8ff,0xa90b,0xa917,0xa923,0xa92f,0xa93b,0xa947,
0xa953,0xa960,0xa96c,0xa978,0xa984,0xa990,0xa99c,0xa9a8,
0xa9b4,0xa9c0,0xa9cc,0xa9d8,0xa9e4,0xa9f0,0xa9fc,0xaa09,
0xaa15,0xaa21,0xaa2d,0xaa39,0xaa45,0xaa51,0xaa5d,0xaa69,
0xaa75,0xaa81,0xaa8d,0xaa99,0xaaa5,0xaab1,0xaabd,0xaac9,
0xaad5,0xaae1,0xaaed,0xaaf9,0xab05,0xab11,0xab1d,0xab29,
0xab35,0xab41,0xab4d,0xab58,0xab64,0xab70,0xab7c,0xab88,
0xab94,0xaba0,0xabac,0xabb8,0xabc4,0xabd0,0xabdc,0xabe8,
0xabf4,0xac00,0xac0b,0xac17,0xac23,0xac2f,0xac3b,0xac47,
0xac53,0xac5f,0xac6b,0xac76,0xac82,0xac8e,0xac9a,0xaca6,
0xacb2,0xacbe,0xacc9,0xacd5,0xace1,0xaced,0xacf9,0xad05,
0xad11,0xad1c,0xad28,0xad34,0xad40,0xad4c,0xad57,0xad63,
0xad6f,0xad7b,0xad87,0xad92,0xad9e,0xadaa,0xadb6,0xadc2,
0xadcd,0xadd9,0xade5,0xadf1,0xadfd,0xae08,0xae14,0xae20,
0xae2c,0xae37,0xae43,0xae4f,0xae5b,0xae66,0xae72,0xae7e,
0xae8a,0xae95,0xaea1,0xaead,0xaeb8,0xaec4,0xaed0,0xaedc,
0xaee7,0xaef3,0xaeff,0xaf0a,0xaf16,0xaf22,0xaf2e,0xaf39,
0xaf45,0xaf51,0xaf5c,0xaf68,0xaf74,0xaf7f,0xaf8b,0xaf97,
0xafa2,0xafae,0xafba,0xafc5,0xafd1,0xafdd,0xafe8,0xaff4,
0xb000,0xb00b,0xb017,0xb022,0xb02e,0xb03a,0xb045,0xb051,
0xb05c,0xb068,0xb074,0xb07f,0xb08b,0xb097,0xb0a2,0xb0ae,
0xb0b9,0xb0c5,0xb0d0,0xb0dc,0xb0e8,0xb0f3,0xb0ff,0xb10a,
0xb116,0xb121,0xb12d,0xb139,0xb144,0xb150,0xb15b,0xb167,
0xb172,0xb17e,0xb189,0xb195,0xb1a0,0xb1ac,0xb1b8,0xb1c3,
0xb1cf,0xb1da,0xb1e6,0xb1f1,0xb1fd,0xb208,0xb214,0xb21f,
0xb22b,0xb236,0xb242,0xb24d,0xb259,0xb264,0xb270,0xb27b,
0xb286,0xb292,0xb29d,0xb2a9,0xb2b4,0xb2c0,0xb2cb,0xb2d7,
0xb2e2,0xb2ee,0xb2f9,0xb305,0xb310,0xb31b,0xb327,0xb332,
0xb33e,0xb349,0xb355,0xb360,0xb36b,0xb377,0xb382,0xb38e,
0xb399,0xb3a4,0xb3b0,0xb3bb,0xb3c7,0xb3d2,0xb3dd,0xb3e9,
0xb3f4,0xb400,0xb40b,0xb416,0xb422,0xb42d,0xb438,0xb444,
0xb44f,0xb45a,0xb466,0xb471,0xb47c,0xb488,0xb493,0xb49f,
0xb4aa,0xb4b5,0xb4c1,0xb4cc,0xb4d7,0xb4e2,0xb4ee,0xb4f9,
0xb504,0xb510,0xb51b,0xb526,0xb532,0xb53d,0xb548,0xb554,
0xb55f,0xb56a,0xb575,0xb581,0xb58c,0xb597,0xb5a3,0xb5ae,
0xb5b9,0xb5c4,0xb5d0,0xb5db,0xb5e6,0xb5f1,0xb5fd,0xb608,
0xb613,0xb61e,0xb62a,0xb635,0xb640,0xb64b,0xb657,0xb662,
0xb66d,0xb678,0xb684,0xb68f,0xb69a,0xb6a5,0xb6b0,0xb6bc,
0xb6c7,0xb6d2,0xb6dd,0xb6e8,0xb6f4,0xb6ff,0xb70a,0xb715,
0xb720,0xb72c,0xb737,0xb742,0xb74d,0xb758,0xb763,0xb76f,
0xb77a,0xb785,0xb790,0xb79b,0xb7a6,0xb7b2,0xb7bd,0xb7c8,
0xb7d3,0xb7de,0xb7e9,0xb7f4,0xb800,0xb80b,0xb816,0xb821,
0xb82c,0xb837,0xb842,0xb84d,0xb858,0xb864,0xb86f,0xb87a,
0xb885,0xb890,0xb89b,0xb8a6,0xb8b1,0xb8bc,0xb8c7,0xb8d3,
0xb8de,0xb8e9,0xb8f4,0xb8ff,0xb90a,0xb915,0xb920,0xb92b,
0xb936,0xb941,0xb94c,0xb957,0xb962,0xb96d,0xb978,0xb983,
0xb98f,0xb99a,0xb9a5,0xb9b0,0xb9bb,0xb9c6,0xb9d1,0xb9dc,
0xb9e7,0xb9f2,0xb9fd,0xba08,0xba13,0xba1e,0xba29,0xba34,
0xba3f,0xba4a,0xba55,0xba60,0xba6b,0xba76,0xba81,0xba8c,
0xba97,0xbaa2,0xbaad,0xbab8,0xbac3,0xbace,0xbad8,0xbae3,
0xbaee,0xbaf9,0xbb04,0xbb0f,0xbb1a,0xbb25,0xbb30,0xbb3b,
0xbb46,0xbb51,0xbb5c,0xbb67,0xbb72,0xbb7d,0xbb88,0xbb92,
0xbb9d,0xbba8,0xbbb3,0xbbbe,0xbbc9,0xbbd4,0xbbdf,0xbbea,
0xbbf5,0xbc00,0xbc0a,0xbc15,0xbc20,0xbc2b,0xbc36,0xbc41,
0xbc4c,0xbc57,0xbc61,0xbc6c,0xbc77,0xbc82,0xbc8d,0xbc98,
0xbca3,0xbcad,0xbcb8,0xbcc3,0xbcce,0xbcd9,0xbce4,0xbcef,
0xbcf9,0xbd04,0xbd0f,0xbd1a,0xbd25,0xbd30,0xbd3a,0xbd45,
0xbd50,0xbd5b,0xbd66,0xbd70,0xbd7b,0xbd86,0xbd91,0xbd9c,
0xbda6,0xbdb1,0xbdbc,0xbdc7,0xbdd2,0xbddc,0xbde7,0xbdf2,
0xbdfd,0xbe08,0xbe12,0xbe1d,0xbe28,0xbe33,0xbe3d,0xbe48,
0xbe53,0xbe5e,0xbe68,0xbe73,0xbe7e,0xbe89,0xbe93,0xbe9e,
0xbea9,0xbeb4,0xbebe,0xbec9,0xbed4,0xbedf,0xbee9,0xbef4,
0xbeff,0xbf0a,0xbf14,0xbf1f,0xbf2a,0xbf34,0xbf3f,0xbf4a,
0xbf55,0xbf5f,0xbf6a,0xbf75,0xbf7f,0xbf8a,0xbf95,0xbf9f,
0xbfaa,0xbfb5,0xbfbf,0xbfca,0xbfd5,0xbfdf,0xbfea,0xbff5,
0xc000,0xc00a,0xc015,0xc01f,0xc02a,0xc035,0xc03f,0xc04a,
0xc055,0xc05f,0xc06a,0xc075,0xc07f,0xc08a,0xc095,0xc09f,
0xc0aa,0xc0b5,0xc0bf,0xc0ca,0xc0d4,0xc0df,0xc0ea,0xc0f4,
0xc0ff,0xc109,0xc114,0xc11f,0xc129,0xc134,0xc13e,0xc149,
0xc154,0xc15e,0xc169,0xc173,0xc17e,0xc189,0xc193,0xc19e,
0xc1a8,0xc1b3,0xc1bd,0xc1c8,0xc1d3,0xc1dd,0xc1e8,0xc1f2,
0xc1fd,0xc207,0xc212,0xc21d,0xc227,0xc232,0xc23c,0xc247,
0xc251,0xc25c,0xc266,0xc271,0xc27b,0xc286,0xc290,0xc29b,
0xc2a5,0xc2b0,0xc2bb,0xc2c5,0xc2d0,0xc2da,0xc2e5,0xc2ef,
0xc2fa,0xc304,0xc30f,0xc319,0xc324,0xc32e,0xc339,0xc343,
0xc34e,0xc358,0xc363,0xc36d,0xc377,0xc382,0xc38c,0xc397,
0xc3a1,0xc3ac,0xc3b6,0xc3c1,0xc3cb,0xc3d6,0xc3e0,0xc3eb,
0xc3f5,0xc400,0xc40a,0xc414,0xc41f,0xc429,0xc434,0xc43e,
0xc449,0xc453,0xc45d,0xc468,0xc472,0xc47d,0xc487,0xc492,
0xc49c,0xc4a6,0xc4b1,0xc4bb,0xc4c6,0xc4d0,0xc4da,0xc4e5,
0xc4ef,0xc4fa,0xc504,0xc50e,0xc519,0xc523,0xc52e,0xc538,
0xc542,0xc54d,0xc557,0xc562,0xc56c,0xc576,0xc581,0xc58b,
0xc595,0xc5a0,0xc5aa,0xc5b4,0xc5bf,0xc5c9,0xc5d4,0xc5de,
0xc5e8,0xc5f3,0xc5fd,0xc607,0xc612,0xc61c,0xc626,0xc631,
0xc63b,0xc645,0xc650,0xc65a,0xc664,0xc66f,0xc679,0xc683,
0xc68e,0xc698,0xc6a2,0xc6ac,0xc6b7,0xc6c1,0xc6cb,0xc6d6,
0xc6e0,0xc6ea,0xc6f5,0xc6ff,0xc709,0xc713,0xc71e,0xc728,
0xc732,0xc73d,0xc747,0xc751,0xc75b,0xc766,0xc770,0xc77a,
0xc784,0xc78f,0xc799,0xc7a3,0xc7ae,0xc7b8,0xc7c2,0xc7cc,
0xc7d7,0xc7e1,0xc7eb,0xc7f5,0xc800,0xc80a,0xc814,0xc81e,
0xc828,0xc833,0xc83d,0xc847,0xc851,0xc85c,0xc866,0xc870,
0xc87a,0xc884,0xc88f,0xc899,0xc8a3,0xc8ad,0xc8b7,0xc8c2,
0xc8cc,0xc8d6,0xc8e0,0xc8ea,0xc8f5,0xc8ff,0xc909,0xc913,
0xc91d,0xc928,0xc932,0xc93c,0xc946,0xc950,0xc95a,0xc965,
0xc96f,0xc979,0xc983,0xc98d,0xc997,0xc9a2,0xc9ac,0xc9b6,
0xc9c0,0xc9ca,0xc9d4,0xc9df,0xc9e9,0xc9f3,0xc9fd,0xca07,
0xca11,0xca1b,0xca26,0xca30,0xca3a,0xca44,0xca4e,0xca58,
0xca62,0xca6c,0xca76,0xca81,0xca8b,0xca95,0xca9f,0xcaa9,
0xcab3,0xcabd,0xcac7,0xcad1,0xcadc,0xcae6,0xcaf0,0xcafa,
0xcb04,0xcb0e,0xcb18,0xcb22,0xcb2c,0xcb36,0xcb40,0xcb4a,
0xcb55,0xcb5f,0xcb69,0xcb73,0xcb7d,0xcb87,0xcb91,0xcb9b,
0xcba5,0xcbaf,0xcbb9,0xcbc3,0xcbcd,0xcbd7,0xcbe1,0xcbeb,
0xcbf5,0xcc00,0xcc0a,0xcc14,0xcc1e,0xcc28,0xcc32,0xcc3c,
0xcc46,0xcc50,0xcc5a,0xcc64,0xcc6e,0xcc78,0xcc82,0xcc8c,
0xcc96,0xcca0,0xccaa,0xccb4,0xccbe,0xccc8,0xccd2,0xccdc,
0xcce6,0xccf0,0xccfa,0xcd04,0xcd0e,0xcd18,0xcd22,0xcd2c,
0xcd36,0xcd40,0xcd4a,0xcd54,0xcd5e,0xcd68,0xcd72,0xcd7c,
0xcd86,0xcd90,0xcd99,0xcda3,0xcdad,0xcdb7,0xcdc1,0xcdcb,
0xcdd5,0xcddf,0xcde9,0xcdf3,0xcdfd,0xce07,0xce11,0xce1b,
0xce25,0xce2f,0xce39,0xce43,0xce4c,0xce56,0xce60,0xce6a,
0xce74,0xce7e,0xce88,0xce92,0xce9c,0xcea6,0xceb0,0xceba,
0xcec3,0xcecd,0xced7,0xcee1,0xceeb,0xcef5,0xceff,0xcf09,
0xcf13,0xcf1d,0xcf26,0xcf30,0xcf3a,0xcf44,0xcf4e,0xcf58,
0xcf62,0xcf6c,0xcf75,0xcf7f,0xcf89,0xcf93,0xcf9d,0xcfa7,
0xcfb1,0xcfbb,0xcfc4,0xcfce,0xcfd8,0xcfe2,0xcfec,0xcff6,
0xd000,0xd009,0xd013,0xd01d,0xd027,0xd031,0xd03b,0xd044,
0xd04e,0xd058,0xd062,0xd06c,0xd076,0xd07f,0xd089,0xd093,
0xd09d,0xd0a7,0xd0b0,0xd0ba,0xd0c4,0xd0ce,0xd0d8,0xd0e1,
0xd0eb,0xd0f5,0xd0ff,0xd109,0xd112,0xd11c,0xd126,0xd130,
0xd13a,0xd143,0xd14d,0xd157,0xd161,0xd16b,0xd174,0xd17e,
0xd188,0xd192,0xd19b,0xd1a5,0xd1af,0xd1b9,0xd1c3,0xd1cc,
0xd1d6,0xd1e0,0xd1ea,0xd1f3,0xd1fd,0xd207,0xd211,0xd21a,
0xd224,0xd22e,0xd238,0xd241,0xd24b,0xd255,0xd25f,0xd268,
0xd272,0xd27c,0xd285,0xd28f,0xd299,0xd2a3,0xd2ac,0xd2b6,
0xd2c0,0xd2c9,0xd2d3,0xd2dd,0xd2e7,0xd2f0,0xd2fa,0xd304,
0xd30d,0xd317,0xd321,0xd32b,0xd334,0xd33e,0xd348,0xd351,
0xd35b,0xd365,0xd36e,0xd378,0xd382,0xd38b,0xd395,0xd39f,
0xd3a8,0xd3b2,0xd3bc,0xd3c6,0xd3cf,0xd3d9,0xd3e3,0xd3ec,
0xd3f6,0xd400,0xd409,0xd413,0xd41c,0xd426,0xd430,0xd439,
0xd443,0xd44d,0xd456,0xd460,0xd46a,0xd473,0xd47d,0xd487,
0xd490,0xd49a,0xd4a3,0xd4ad,0xd4b7,0xd4c0,0xd4ca,0xd4d4,
0xd4dd,0xd4e7,0xd4f0,0xd4fa,0xd504,0xd50d,0xd517,0xd521,
0xd52a,0xd534,0xd53d,0xd547,0xd551,0xd55a,0xd564,0xd56d,
0xd577,0xd581,0xd58a,0xd594,0xd59d,0xd5a7,0xd5b0,0xd5ba,
0xd5c4,0xd5cd,0xd5d7,0xd5e0,0xd5ea,0xd5f4,0xd5fd,0xd607,
0xd610,0xd61a,0xd623,0xd62d,0xd637,0xd640,0xd64a,0xd653,
0xd65d,0xd666,0xd670,0xd679,0xd683,0xd68c,0xd696,0xd6a0,
0xd6a9,0xd6b3,0xd6bc,0xd6c6,0xd6cf,0xd6d9,0xd6e2,0xd6ec,
0xd6f5,0xd6ff,0xd708,0xd712,0xd71b,0xd725,0xd72f,0xd738,
0xd742,0xd74b,0xd755,0xd75e,0xd768,0xd771,0xd77b,0xd784,
0xd78e,0xd797,0xd7a1,0xd7aa,0xd7b4,0xd7bd,0xd7c7,0xd7d0,
0xd7da,0xd7e3,0xd7ed,0xd7f6,0xd800,0xd809,0xd812,0xd81c,
0xd825,0xd82f,0xd838,0xd842,0xd84b,0xd855,0xd85e,0xd868,
0xd871,0xd87b,0xd884,0xd88e,0xd897,0xd8a0,0xd8aa,0xd8b3,
0xd8bd,0xd8c6,0xd8d0,0xd8d9,0xd8e3,0xd8ec,0xd8f5,0xd8ff,
0xd908,0xd912,0xd91b,0xd925,0xd92e,0xd938,0xd941,0xd94a,
0xd954,0xd95d,0xd967,0xd970,0xd979,0xd983,0xd98c,0xd996,
0xd99f,0xd9a9,0xd9b2,0xd9bb,0xd9c5,0xd9ce,0xd9d8,0xd9e1,
0xd9ea,0xd9f4,0xd9fd,0xda07,0xda10,0xda19,0xda23,0xda2c,
0xda35,0xda3f,0xda48,0xda52,0xda5b,0xda64,0xda6e,0xda77,
0xda81,0xda8a,0xda93,0xda9d,0xdaa6,0xdaaf,0xdab9,0xdac2,
0xdacb,0xdad5,0xdade,0xdae8,0xdaf1,0xdafa,0xdb04,0xdb0d,
0xdb16,0xdb20,0xdb29,0xdb32,0xdb3c,0xdb45,0xdb4e,0xdb58,
0xdb61,0xdb6a,0xdb74,0xdb7d,0xdb86,0xdb90,0xdb99,0xdba2,
0xdbac,0xdbb5,0xdbbe,0xdbc8,0xdbd1,0xdbda,0xdbe4,0xdbed,
0xdbf6,0xdc00,0xdc09,0xdc12,0xdc1b,0xdc25,0xdc2e,0xdc37,
0xdc41,0xdc4a,0xdc53,0xdc5d,0xdc66,0xdc6f,0xdc78,0xdc82,
0xdc8b,0xdc94,0xdc9e,0xdca7,0xdcb0,0xdcb9,0xdcc3,0xdccc,
0xdcd5,0xdcde,0xdce8,0xdcf1,0xdcfa,0xdd04,0xdd0d,0xdd16,
0xdd1f,0xdd29,0xdd32,0xdd3b,0xdd44,0xdd4e,0xdd57,0xdd60,
0xdd69,0xdd73,0xdd7c,0xdd85,0xdd8e,0xdd98,0xdda1,0xddaa,
0xddb3,0xddbd,0xddc6,0xddcf,0xddd8,0xdde2,0xddeb,0xddf4,
0xddfd,0xde06,0xde10,0xde19,0xde22,0xde2b,0xde35,0xde3e,
0xde47,0xde50,0xde59,0xde63,0xde6c,0xde75,0xde7e,0xde87,
0xde91,0xde9a,0xdea3,0xdeac,0xdeb5,0xdebf,0xdec8,0xded1,
0xdeda,0xdee3,0xdeed,0xdef6,0xdeff,0xdf08,0xdf11,0xdf1a,
0xdf24,0xdf2d,0xdf36,0xdf3f,0xdf48,0xdf52,0xdf5b,0xdf64,
0xdf6d,0xdf76,0xdf7f,0xdf89,0xdf92,0xdf9b,0xdfa4,0xdfad,
0xdfb6,0xdfbf,0xdfc9,0xdfd2,0xdfdb,0xdfe4,0xdfed,0xdff6,
0xe000,0xe009,0xe012,0xe01b,0xe024,0xe02d,0xe036,0xe03f,
0xe049,0xe052,0xe05b,0xe064,0xe06d,0xe076,0xe07f,0xe088,
0xe092,0xe09b,0xe0a4,0xe0ad,0xe0b6,0xe0bf,0xe0c8,0xe0d1,
0xe0db,0xe0e4,0xe0ed,0xe0f6,0xe0ff,0xe108,0xe111,0xe11a,
0xe123,0xe12c,0xe136,0xe13f,0xe148,0xe151,0xe15a,0xe163,
0xe16c,0xe175,0xe17e,0xe187,0xe190,0xe199,0xe1a3,0xe1ac,
0xe1b5,0xe1be,0xe1c7,0xe1d0,0xe1d9,0xe1e2,0xe1eb,0xe1f4,
0xe1fd,0xe206,0xe20f,0xe218,0xe221,0xe22b,0xe234,0xe23d,
0xe246,0xe24f,0xe258,0xe261,0xe26a,0xe273,0xe27c,0xe285,
0xe28e,0xe297,0xe2a0,0xe2a9,0xe2b2,0xe2bb,0xe2c4,0xe2cd,
0xe2d6,0xe2df,0xe2e8,0xe2f1,0xe2fa,0xe303,0xe30c,0xe315,
0xe31f,0xe328,0xe331,0xe33a,0xe343,0xe34c,0xe355,0xe35e,
0xe367,0xe370,0xe379,0xe382,0xe38b,0xe394,0xe39d,0xe3a6,
0xe3af,0xe3b8,0xe3c1,0xe3ca,0xe3d3,0xe3dc,0xe3e5,0xe3ee,
0xe3f7,0xe400,0xe408,0xe411,0xe41a,0xe423,0xe42c,0xe435,
0xe43e,0xe447,0xe450,0xe459,0xe462,0xe46b,0xe474,0xe47d,
0xe486,0xe48f,0xe498,0xe4a1,0xe4aa,0xe4b3,0xe4bc,0xe4c5,
0xe4ce,0xe4d7,0xe4e0,0xe4e9,0xe4f2,0xe4fa,0xe503,0xe50c,
0xe515,0xe51e,0xe527,0xe530,0xe539,0xe542,0xe54b,0xe554,
0xe55d,0xe566,0xe56f,0xe578,0xe580,0xe589,0xe592,0xe59b,
0xe5a4,0xe5ad,0xe5b6,0xe5bf,0xe5c8,0xe5d1,0xe5da,0xe5e3,
0xe5eb,0xe5f4,0xe5fd,0xe606,0xe60f,0xe618,0xe621,0xe62a,
0xe633,0xe63c,0xe644,0xe64d,0xe656,0xe65f,0xe668,0xe671,
0xe67a,0xe683,0xe68c,0xe694,0xe69d,0xe6a6,0xe6af,0xe6b8,
0xe6c1,0xe6ca,0xe6d3,0xe6db,0xe6e4,0xe6ed,0xe6f6,0xe6ff,
0xe708,0xe711,0xe71a,0xe722,0xe72b,0xe734,0xe73d,0xe746,
0xe74f,0xe758,0xe760,0xe769,0xe772,0xe77b,0xe784,0xe78d,
0xe795,0xe79e,0xe7a7,0xe7b0,0xe7b9,0xe7c2,0xe7cb,0xe7d3,
0xe7dc,0xe7e5,0xe7ee,0xe7f7,0xe800,0xe808,0xe811,0xe81a,
0xe823,0xe82c,0xe834,0xe83d,0xe846,0xe84f,0xe858,0xe861,
0xe869,0xe872,0xe87b,0xe884,0xe88d,0xe895,0xe89e,0xe8a7,
0xe8b0,0xe8b9,0xe8c1,0xe8ca,0xe8d3,0xe8dc,0xe8e5,0xe8ed,
0xe8f6,0xe8ff,0xe908,0xe911,0xe919,0xe922,0xe92b,0xe934,
0xe93c,0xe945,0xe94e,0xe957,0xe960,0xe968,0xe971,0xe97a,
0xe983,0xe98b,0xe994,0xe99d,0xe9a6,0xe9ae,0xe9b7,0xe9c0,
0xe9c9,0xe9d2,0xe9da,0xe9e3,0xe9ec,0xe9f5,0xe9fd,0xea06,
0xea0f,0xea18,0xea20,0xea29,0xea32,0xea3b,0xea43,0xea4c,
0xea55,0xea5e,0xea66,0xea6f,0xea78,0xea80,0xea89,0xea92,
0xea9b,0xeaa3,0xeaac,0xeab5,0xeabe,0xeac6,0xeacf,0xead8,
0xeae0,0xeae9,0xeaf2,0xeafb,0xeb03,0xeb0c,0xeb15,0xeb1d,
0xeb26,0xeb2f,0xeb38,0xeb40,0xeb49,0xeb52,0xeb5a,0xeb63,
0xeb6c,0xeb74,0xeb7d,0xeb86,0xeb8f,0xeb97,0xeba0,0xeba9,
0xebb1,0xebba,0xebc3,0xebcb,0xebd4,0xebdd,0xebe5,0xebee,
0xebf7,0xec00,0xec08,0xec11,0xec1a,0xec22,0xec2b,0xec34,
0xec3c,0xec45,0xec4e,0xec56,0xec5f,0xec68,0xec70,0xec79,
0xec82,0xec8a,0xec93,0xec9c,0xeca4,0xecad,0xecb5,0xecbe,
0xecc7,0xeccf,0xecd8,0xece1,0xece9,0xecf2,0xecfb,0xed03,
0xed0c,0xed15,0xed1d,0xed26,0xed2e,0xed37,0xed40,0xed48,
0xed51,0xed5a,0xed62,0xed6b,0xed74,0xed7c,0xed85,0xed8d,
0xed96,0xed9f,0xeda7,0xedb0,0xedb8,0xedc1,0xedca,0xedd2,
0xeddb,0xede4,0xedec,0xedf5,0xedfd,0xee06,0xee0f,0xee17,
0xee20,0xee28,0xee31,0xee3a,0xee42,0xee4b,0xee53,0xee5c,
0xee65,0xee6d,0xee76,0xee7e,0xee87,0xee8f,0xee98,0xeea1,
0xeea9,0xeeb2,0xeeba,0xeec3,0xeecc,0xeed4,0xeedd,0xeee5,
0xeeee,0xeef6,0xeeff,0xef08,0xef10,0xef19,0xef21,0xef2a,
0xef32,0xef3b,0xef43,0xef4c,0xef55,0xef5d,0xef66,0xef6e,
0xef77,0xef7f,0xef88,0xef90,0xef99,0xefa2,0xefaa,0xefb3,
0xefbb,0xefc4,0xefcc,0xefd5,0xefdd,0xefe6,0xefee,0xeff7,
0xf000,0xf008,0xf011,0xf019,0xf022,0xf02a,0xf033,0xf03b,
0xf044,0xf04c,0xf055,0xf05d,0xf066,0xf06e,0xf077,0xf07f,
0xf088,0xf090,0xf099,0xf0a1,0xf0aa,0xf0b2,0xf0bb,0xf0c3,
0xf0cc,0xf0d4,0xf0dd,0xf0e5,0xf0ee,0xf0f6,0xf0ff,0xf107,
0xf110,0xf118,0xf121,0xf129,0xf132,0xf13a,0xf143,0xf14b,
0xf154,0xf15c,0xf165,0xf16d,0xf176,0xf17e,0xf187,0xf18f,
0xf198,0xf1a0,0xf1a9,0xf1b1,0xf1ba,0xf1c2,0xf1cb,0xf1d3,
0xf1dc,0xf1e4,0xf1ec,0xf1f5,0xf1fd,0xf206,0xf20e,0xf217,
0xf21f,0xf228,0xf230,0xf239,0xf241,0xf24a,0xf252,0xf25a,
0xf263,0xf26b,0xf274,0xf27c,0xf285,0xf28d,0xf296,0xf29e,
0xf2a6,0xf2af,0xf2b7,0xf2c0,0xf2c8,0xf2d1,0xf2d9,0xf2e1,
0xf2ea,0xf2f2,0xf2fb,0xf303,0xf30c,0xf314,0xf31c,0xf325,
0xf32d,0xf336,0xf33e,0xf347,0xf34f,0xf357,0xf360,0xf368,
0xf371,0xf379,0xf381,0xf38a,0xf392,0xf39b,0xf3a3,0xf3ac,
0xf3b4,0xf3bc,0xf3c5,0xf3cd,0xf3d6,0xf3de,0xf3e6,0xf3ef,
0xf3f7,0xf400,0xf408,0xf410,0xf419,0xf421,0xf429,0xf432,
0xf43a,0xf443,0xf44b,0xf453,0xf45c,0xf464,0xf46d,0xf475,
0xf47d,0xf486,0xf48e,0xf496,0xf49f,0xf4a7,0xf4b0,0xf4b8,
0xf4c0,0xf4c9,0xf4d1,0xf4d9,0xf4e2,0xf4ea,0xf4f2,0xf4fb,
0xf503,0xf50c,0xf514,0xf51c,0xf525,0xf52d,0xf535,0xf53e,
0xf546,0xf54e,0xf557,0xf55f,0xf567,0xf570,0xf578,0xf580,
0xf589,0xf591,0xf599,0xf5a2,0xf5aa,0xf5b2,0xf5bb,0xf5c3,
0xf5cb,0xf5d4,0xf5dc,0xf5e4,0xf5ed,0xf5f5,0xf5fd,0xf606,
0xf60e,0xf616,0xf61f,0xf627,0xf62f,0xf638,0xf640,0xf648,
0xf651,0xf659,0xf661,0xf66a,0xf672,0xf67a,0xf682,0xf68b,
0xf693,0xf69b,0xf6a4,0xf6ac,0xf6b4,0xf6bd,0xf6c5,0xf6cd,
0xf6d6,0xf6de,0xf6e6,0xf6ee,0xf6f7,0xf6ff,0xf707,0xf710,
0xf718,0xf720,0xf728,0xf731,0xf739,0xf741,0xf74a,0xf752,
0xf75a,0xf762,0xf76b,0xf773,0xf77b,0xf784,0xf78c,0xf794,
0xf79c,0xf7a5,0xf7ad,0xf7b5,0xf7bd,0xf7c6,0xf7ce,0xf7d6,
0xf7de,0xf7e7,0xf7ef,0xf7f7,0xf800,0xf808,0xf810,0xf818,
0xf821,0xf829,0xf831,0xf839,0xf842,0xf84a,0xf852,0xf85a,
0xf863,0xf86b,0xf873,0xf87b,0xf883,0xf88c,0xf894,0xf89c,
0xf8a4,0xf8ad,0xf8b5,0xf8bd,0xf8c5,0xf8ce,0xf8d6,0xf8de,
0xf8e6,0xf8ef,0xf8f7,0xf8ff,0xf907,0xf90f,0xf918,0xf920,
0xf928,0xf930,0xf939,0xf941,0xf949,0xf951,0xf959,0xf962,
0xf96a,0xf972,0xf97a,0xf982,0xf98b,0xf993,0xf99b,0xf9a3,
0xf9ab,0xf9b4,0xf9bc,0xf9c4,0xf9cc,0xf9d4,0xf9dd,0xf9e5,
0xf9ed,0xf9f5,0xf9fd,0xfa06,0xfa0e,0xfa16,0xfa1e,0xfa26,
0xfa2f,0xfa37,0xfa3f,0xfa47,0xfa4f,0xfa58,0xfa60,0xfa68,
0xfa70,0xfa78,0xfa80,0xfa89,0xfa91,0xfa99,0xfaa1,0xfaa9,
0xfab1,0xfaba,0xfac2,0xfaca,0xfad2,0xfada,0xfae2,0xfaeb,
0xfaf3,0xfafb,0xfb03,0xfb0b,0xfb13,0xfb1c,0xfb24,0xfb2c,
0xfb34,0xfb3c,0xfb44,0xfb4c,0xfb55,0xfb5d,0xfb65,0xfb6d,
0xfb75,0xfb7d,0xfb85,0xfb8e,0xfb96,0xfb9e,0xfba6,0xfbae,
0xfbb6,0xfbbe,0xfbc7,0xfbcf,0xfbd7,0xfbdf,0xfbe7,0xfbef,
0xfbf7,0xfc00,0xfc08,0xfc10,0xfc18,0xfc20,0xfc28,0xfc30,
0xfc38,0xfc40,0xfc49,0xfc51,0xfc59,0xfc61,0xfc69,0xfc71,
0xfc79,0xfc81,0xfc8a,0xfc92,0xfc9a,0xfca2,0xfcaa,0xfcb2,
0xfcba,0xfcc2,0xfcca,0xfcd2,0xfcdb,0xfce3,0xfceb,0xfcf3,
0xfcfb,0xfd03,0xfd0b,0xfd13,0xfd1b,0xfd23,0xfd2c,0xfd34,
0xfd3c,0xfd44,0xfd4c,0xfd54,0xfd5c,0xfd64,0xfd6c,0xfd74,
0xfd7c,0xfd84,0xfd8d,0xfd95,0xfd9d,0xfda5,0xfdad,0xfdb5,
0xfdbd,0xfdc5,0xfdcd,0xfdd5,0xfddd,0xfde5,0xfded,0xfdf5,
0xfdfd,0xfe06,0xfe0e,0xfe16,0xfe1e,0xfe26,0xfe2e,0xfe36,
0xfe3e,0xfe46,0xfe4e,0xfe56,0xfe5e,0xfe66,0xfe6e,0xfe76,
0xfe7e,0xfe86,0xfe8e,0xfe97,0xfe9f,0xfea7,0xfeaf,0xfeb7,
0xfebf,0xfec7,0xfecf,0xfed7,0xfedf,0xfee7,0xfeef,0xfef7,
0xfeff,0xff07,0xff0f,0xff17,0xff1f,0xff27,0xff2f,0xff37,
0xff3f,0xff47,0xff4f,0xff57,0xff5f,0xff67,0xff6f,0xff77,
0xff7f,0xff87,0xff8f,0xff97,0xff9f,0xffa7,0xffaf,0xffb7,
0xffbf,0xffc7,0xffcf,0xffd7,0xffdf,0xffe7,0xffef,0xfff7};





//--------------------------------------------------------------------------------------
// Copy functions from VIP libs for scalerVIP.c , because it can't call lib function in kernel space
// Start
//--------------------------------------------------------------------------------------

//>>>>>>>>>>>>>>>  old RTNR start <<<<<<<<<<<<<<<<<<<<<<<<<<<
void drvif_color_rtnr_y_th_isr(unsigned char channel, unsigned char *Th_Y)
{
//write register
	//RTD_Log(LOGGER_DEBUG,"\n jason==> Start RTNR_Y setting ... \n");


	di_im_di_rtnr_control_RBUS im_di_rtnr_control_reg;
	di_im_di_rtnr_y_16_k0_k7_RBUS im_di_rtnr_y_k_reg;
	di_im_di_rtnr_y_16_th0_th3_RBUS im_di_rtnr_y_16_th0_th3_reg;
	di_im_di_rtnr_y_16_th4_th7_RBUS im_di_rtnr_y_16_th4_th7_reg;
	di_im_di_rtnr_y_16_th8_th11_RBUS im_di_rtnr_y_16_th8_th11_reg;
	di_im_di_rtnr_y_16_th12_th14_RBUS im_di_rtnr_y_16_th12_th14_reg;

	im_di_rtnr_control_reg.regValue = rtd_inl(DI_IM_DI_RTNR_CONTROL_reg);
	im_di_rtnr_y_k_reg.regValue = rtd_inl(DI_IM_DI_RTNR_Y_16_K0_K7_reg);
	im_di_rtnr_y_16_th0_th3_reg.regValue = rtd_inl(DI_IM_DI_RTNR_Y_16_TH0_TH3_reg);
	im_di_rtnr_y_16_th4_th7_reg.regValue = rtd_inl(DI_IM_DI_RTNR_Y_16_TH4_TH7_reg);
	im_di_rtnr_y_16_th8_th11_reg.regValue = rtd_inl(DI_IM_DI_RTNR_Y_16_TH8_TH11_reg);
	im_di_rtnr_y_16_th12_th14_reg.regValue = rtd_inl(DI_IM_DI_RTNR_Y_16_TH12_TH14_reg);

	if(channel == _CHANNEL1)
	{
		im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly0 = Th_Y[0];
		im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly1 = Th_Y[1];
		im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly2 = Th_Y[2];
		im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly3 = Th_Y[3];
		im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly4 = Th_Y[4];
		im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly5 = Th_Y[5];
		im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly6 = Th_Y[6];
		im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly7 = Th_Y[7];
		im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly8 = Th_Y[8];
		im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly9 = Th_Y[9];
		im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly10 = Th_Y[10];
		im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly11 = Th_Y[11];
		im_di_rtnr_y_16_th12_th14_reg.cp_temporalthly12 = Th_Y[12];
		im_di_rtnr_y_16_th12_th14_reg.cp_temporalthly13 = Th_Y[13];
		im_di_rtnr_y_16_th12_th14_reg.cp_temporalthly14 = Th_Y[14];
	/*
		if(Th_Y[0]!=0)//GET_RTNR_MODE(channel) flag must be turned on by menucofig
		{
			im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly0 = Th_Y[0];
			im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly1 = Th_Y[1];
			im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly2 = Th_Y[2];
			im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly3 = Th_Y[3];
			im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly4 = Th_Y[4];
			im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly5 = Th_Y[5];
			im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly6 = Th_Y[6];
		}
		else
		{
			im_di_rtnr_y_k_reg.cp_temporal_y_k0 = 7;//no rtnr effect
			im_di_rtnr_y_k_reg.cp_temporal_y_k1=  7;
			im_di_rtnr_y_k_reg.cp_temporal_y_k2 = 7;
			im_di_rtnr_y_k_reg.cp_temporal_y_k3 = 7;
			im_di_rtnr_y_k_reg.cp_temporal_y_k4 = 7;
			im_di_rtnr_y_k_reg.cp_temporal_y_k5 = 7;
			im_di_rtnr_y_k_reg.cp_temporal_y_k6 = 7;
			im_di_rtnr_y_k_reg.cp_temporal_y_k7 = 7;
		}
	*/

		scaler_rtd_outl(DI_IM_DI_RTNR_CONTROL_reg,im_di_rtnr_control_reg.regValue);
		scaler_rtd_outl(DI_IM_DI_RTNR_Y_16_K0_K7_reg,im_di_rtnr_y_k_reg.regValue);
		scaler_rtd_outl(DI_IM_DI_RTNR_Y_16_TH0_TH3_reg,im_di_rtnr_y_16_th0_th3_reg.regValue);
		scaler_rtd_outl(DI_IM_DI_RTNR_Y_16_TH4_TH7_reg,im_di_rtnr_y_16_th4_th7_reg.regValue);
		scaler_rtd_outl(DI_IM_DI_RTNR_Y_16_TH8_TH11_reg,im_di_rtnr_y_16_th8_th11_reg.regValue);
		scaler_rtd_outl(DI_IM_DI_RTNR_Y_16_TH12_TH14_reg,im_di_rtnr_y_16_th12_th14_reg.regValue);
		//WaitFor_IEN_STOP1();//20080516 jason for double buffer

	}


}

void drvif_color_rtnr_c_th_isr(unsigned char channel, unsigned char *Th_C)
{


	di_im_di_rtnr_control_RBUS im_di_rtnr_control_reg;
	di_im_di_rtnr_c_k_RBUS im_di_rtnr_c_k_reg;
	di_im_di_rtnr_c_th0_th3_RBUS im_di_rtnr_c_th0_th3_reg;
	di_im_di_rtnr_c_th4_th6_RBUS im_di_rtnr_c_th4_th6_reg;


	im_di_rtnr_control_reg.regValue = rtd_inl(DI_IM_DI_RTNR_CONTROL_reg);
	im_di_rtnr_c_k_reg.regValue = rtd_inl(DI_IM_DI_RTNR_C_K_reg);
	im_di_rtnr_c_th0_th3_reg.regValue = rtd_inl(DI_IM_DI_RTNR_C_TH0_TH3_reg);
	im_di_rtnr_c_th4_th6_reg.regValue = rtd_inl(DI_IM_DI_RTNR_C_TH4_TH6_reg);

	if(channel == _CHANNEL1)
	{
		im_di_rtnr_c_th0_th3_reg.cp_temporalthlc0 = Th_C[0];
		im_di_rtnr_c_th0_th3_reg.cp_temporalthlc1 = Th_C[1];
		im_di_rtnr_c_th0_th3_reg.cp_temporalthlc2 = Th_C[2];
		im_di_rtnr_c_th0_th3_reg.cp_temporalthlc3 = Th_C[3];
		im_di_rtnr_c_th4_th6_reg.cp_temporalthlc4 = Th_C[4];
		im_di_rtnr_c_th4_th6_reg.cp_temporalthlc5 = Th_C[5];
		im_di_rtnr_c_th4_th6_reg.cp_temporalthlc6 = Th_C[6];
	/*
		if(Th_C[0]!=0)//GET_RTNR_MODE(channel) flag must be turned on by menucofig
		{
			im_di_rtnr_c_th0_th3_reg.cp_temporalthlc0 = Th_C[0];
			im_di_rtnr_c_th0_th3_reg.cp_temporalthlc1 = Th_C[1];
			im_di_rtnr_c_th0_th3_reg.cp_temporalthlc2 = Th_C[2];
			im_di_rtnr_c_th0_th3_reg.cp_temporalthlc3 = Th_C[3];
			im_di_rtnr_c_th4_th6_reg.cp_temporalthlc4 = Th_C[4];
			im_di_rtnr_c_th4_th6_reg.cp_temporalthlc5 = Th_C[5];
			im_di_rtnr_c_th4_th6_reg.cp_temporalthlc6 = Th_C[6];

		}
		else//th1 = 0 or project no support RTNR
		{
			im_di_rtnr_c_k_reg.cp_temporal_c_k0 = 7;//no rtnr effect
			im_di_rtnr_c_k_reg.cp_temporal_c_k1=  7;
			im_di_rtnr_c_k_reg.cp_temporal_c_k2 = 7;
			im_di_rtnr_c_k_reg.cp_temporal_c_k3 = 7;
			im_di_rtnr_c_k_reg.cp_temporal_c_k4 = 7;
			im_di_rtnr_c_k_reg.cp_temporal_c_k5 = 7;
			im_di_rtnr_c_k_reg.cp_temporal_c_k6 = 7;
			im_di_rtnr_c_k_reg.cp_temporal_c_k7 = 7;
		}
	*/
		//IoReg_Write32(DI_IM_DI_RTNR_CONTROL_reg,RecursiveTemporalNRCtrl_reg.regValue);
		scaler_rtd_outl(DI_IM_DI_RTNR_CONTROL_reg,im_di_rtnr_control_reg.regValue);
		scaler_rtd_outl(DI_IM_DI_RTNR_C_K_reg,im_di_rtnr_c_k_reg.regValue);
		scaler_rtd_outl(DI_IM_DI_RTNR_C_TH0_TH3_reg,im_di_rtnr_c_th0_th3_reg.regValue);
		scaler_rtd_outl(DI_IM_DI_RTNR_C_TH4_TH6_reg,im_di_rtnr_c_th4_th6_reg.regValue);
	}


}

#ifdef ISR_OLD_RTNR_ENABLE

void drvif_color_rtnr_y_k(unsigned char channel, unsigned int Y_K0_K7, unsigned int Y_K8_K15)
{
//write register

	if(channel == _CHANNEL1)
	{
		scaler_rtd_maskl( DI_IM_DI_RTNR_Y_16_K0_K7_reg, ~(0xffffffff), (unsigned int)(Y_K0_K7));
		scaler_rtd_maskl( DI_IM_DI_RTNR_Y_16_K8_K15_reg, ~(0xffffffff), (unsigned int)(Y_K8_K15));

	}

}


void drvif_color_rtnr_c_k(unsigned char channel, unsigned int K_C)
{
//write register
	if(channel == _CHANNEL1)
	{

		scaler_rtd_maskl( DI_IM_DI_RTNR_C_K_reg, ~(0xffffffff), (unsigned int)(K_C));	// nick187 marked @ 20110223

	}

}
#endif
//>>>>>>>>>>>>>>>  old RTNR  end <<<<<<<<<<<<<<<<<<<<<<<<<<<




//>>>>>>>>>>>>>>>  new RTNR start <<<<<<<<<<<<<<<<<<<<<<<<<<<
// chen 170809
//#if 0
#ifdef ISR_NEW_RTNR_ENABLE
void fwif_color_new_rtnr_adjust(unsigned char channel, unsigned char RTNR_level)
{
		int i;
		unsigned char *rtnr_2D_kmap=NULL;

		if (channel == _CHANNEL1)
		{
			if(Scaler_InputSrcGetMainChType()==_SRC_HDMI || Scaler_InputSrcGetMainChType()==_SRC_COMPONENT || Scaler_InputSrcGetMainChType()==_SRC_YPBPR)
			{
				switch (RTNR_level)
				{
					case 0:
						rtnr_2D_kmap = &New_RTNR_Coef_1[0][0];
						break;
					case 1:
						rtnr_2D_kmap = &TV_AutoNr_RTNR_Coef_A[0][0];
						break;
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
						rtnr_2D_kmap = &TV_AutoNr_RTNR_Coef_B[0][0];
				    	break;
					case 9:
							//rtnr_2D_kmap = &TV_AutoNr_RTNR_Coef_C[0][0];
							//break;
					case 10:
							//rtnr_2D_kmap = &TV_AutoNr_RTNR_Coef_D[0][0];
							//break;
					default://default off
						rtnr_2D_kmap = &TV_AutoNr_RTNR_Coef_A[0][0];
					break;
				}
			}
			else if(Scaler_InputSrcGetMainChType()==_SRC_CVBS)
			{

				switch (RTNR_level)
				{
					case 0:
						rtnr_2D_kmap = &CVBS_AutoNr_RTNR_Coef_A[0][0];
						break;
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
						rtnr_2D_kmap = &CVBS_AutoNr_RTNR_Coef_B[0][0];
						break;
					break;
					case 7:
						rtnr_2D_kmap = &CVBS_AutoNr_RTNR_Coef_C[0][0];
						break;
					case 8:
						//	rtnr_2D_kmap = &CVBS_AutoNr_RTNR_Coef_C[0][0];
			    		//	break;
					case 9:
						//	rtnr_2D_kmap = &CVBS_AutoNr_RTNR_Coef_C[0][0];
						//	break;
					case 10:
						//	rtnr_2D_kmap = &CVBS_AutoNr_RTNR_Coef_D[0][0];
						//	break;
					default://default off
						rtnr_2D_kmap = &CVBS_AutoNr_RTNR_Coef_B[0][0];
					break;
				}

			}
		}

	if(rtnr_2D_kmap!=NULL)
	{
		for(i=0;i<8;i++)
		{
			drvif_color_newrtnr_rtnr_k(i,&rtnr_2D_kmap[i*8]);
		}
	}


}

void drvif_color_newrtnr_rtnr_k(unsigned char row_select, unsigned char *th)
{

	di_im_di_rtnr_new_k_00_RBUS rtnr_new_k_00;
	di_im_di_rtnr_new_k_10_RBUS rtnr_new_k_10;
	di_im_di_rtnr_new_k_20_RBUS rtnr_new_k_20;
	di_im_di_rtnr_new_k_30_RBUS rtnr_new_k_30;
	di_im_di_rtnr_new_k_40_RBUS rtnr_new_k_40;
	di_im_di_rtnr_new_k_50_RBUS rtnr_new_k_50;
	di_im_di_rtnr_new_k_60_RBUS rtnr_new_k_60;
	di_im_di_rtnr_new_k_70_RBUS rtnr_new_k_70;

	switch (row_select)
	{
	case 0:


		rtnr_new_k_00.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_K_00_reg);

		rtnr_new_k_00.rtnr_k_00 = th[0];
		rtnr_new_k_00.rtnr_k_01 = th[1];
		rtnr_new_k_00.rtnr_k_02 = th[2];
		rtnr_new_k_00.rtnr_k_03 = th[3];
		rtnr_new_k_00.rtnr_k_04 = th[4];
		rtnr_new_k_00.rtnr_k_05 = th[5];
		rtnr_new_k_00.rtnr_k_06 = th[6];
		rtnr_new_k_00.rtnr_k_07 = th[7];

		//for(int i=1;i<8;i++)
		//printf("k_th0[%d]=%x\n",i,th[i]);

		rtd_outl(DI_IM_DI_RTNR_NEW_K_00_reg, rtnr_new_k_00.regValue);
		break;

	case 1:


		rtnr_new_k_10.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_K_10_reg);

		rtnr_new_k_10.rtnr_k_10 = th[0];
		rtnr_new_k_10.rtnr_k_11 = th[1];
		rtnr_new_k_10.rtnr_k_12 = th[2];
		rtnr_new_k_10.rtnr_k_13 = th[3];
		rtnr_new_k_10.rtnr_k_14 = th[4];
		rtnr_new_k_10.rtnr_k_15 = th[5];
		rtnr_new_k_10.rtnr_k_16 = th[6];
		rtnr_new_k_10.rtnr_k_17 = th[7];

		//for(int i=1;i<8;i++)
		//printf("k_th1[%d]=%x\n",i,th[i]);

		rtd_outl(DI_IM_DI_RTNR_NEW_K_10_reg, rtnr_new_k_10.regValue);
		break;

	case 2:


		rtnr_new_k_20.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_K_20_reg);

		rtnr_new_k_20.rtnr_k_20 = th[0];
		rtnr_new_k_20.rtnr_k_21 = th[1];
		rtnr_new_k_20.rtnr_k_22 = th[2];
		rtnr_new_k_20.rtnr_k_23 = th[3];
		rtnr_new_k_20.rtnr_k_24 = th[4];
		rtnr_new_k_20.rtnr_k_25 = th[5];
		rtnr_new_k_20.rtnr_k_26 = th[6];
		rtnr_new_k_20.rtnr_k_27 = th[7];

		//for(int i=1;i<8;i++)
		//printf("k_th2[%d]=%x\n",i,th[i]);

		rtd_outl(DI_IM_DI_RTNR_NEW_K_20_reg, rtnr_new_k_20.regValue);
		break;

	case 3:


		rtnr_new_k_30.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_K_30_reg);

		rtnr_new_k_30.rtnr_k_30 = th[0];
		rtnr_new_k_30.rtnr_k_31 = th[1];
		rtnr_new_k_30.rtnr_k_32 = th[2];
		rtnr_new_k_30.rtnr_k_33 = th[3];
		rtnr_new_k_30.rtnr_k_34 = th[4];
		rtnr_new_k_30.rtnr_k_35 = th[5];
		rtnr_new_k_30.rtnr_k_36 = th[6];
		rtnr_new_k_30.rtnr_k_37 = th[7];

		//for(int i=1;i<8;i++)
		//printf("k_th3[%d]=%x\n",i,th[i]);

		rtd_outl(DI_IM_DI_RTNR_NEW_K_30_reg, rtnr_new_k_30.regValue);
		break;

	case 4:

		rtnr_new_k_40.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_K_40_reg);

		rtnr_new_k_40.rtnr_k_40 = th[0];
		rtnr_new_k_40.rtnr_k_41 = th[1];
		rtnr_new_k_40.rtnr_k_42 = th[2];
		rtnr_new_k_40.rtnr_k_43 = th[3];
		rtnr_new_k_40.rtnr_k_44 = th[4];
		rtnr_new_k_40.rtnr_k_45 = th[5];
		rtnr_new_k_40.rtnr_k_46 = th[6];
		rtnr_new_k_40.rtnr_k_47 = th[7];

		//for(int i=1;i<8;i++)
		//printf("k_th4[%d]=%x\n",i,th[i]);

		rtd_outl(DI_IM_DI_RTNR_NEW_K_40_reg, rtnr_new_k_40.regValue);
		break;

	case 5:


		rtnr_new_k_50.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_K_50_reg);

		rtnr_new_k_50.rtnr_k_50 =th[0];
		rtnr_new_k_50.rtnr_k_51 =th[1];
		rtnr_new_k_50.rtnr_k_52 =th[2];
		rtnr_new_k_50.rtnr_k_53 =th[3];
		rtnr_new_k_50.rtnr_k_54 =th[4];
		rtnr_new_k_50.rtnr_k_55 =th[5];
		rtnr_new_k_50.rtnr_k_56 =th[6];
		rtnr_new_k_50.rtnr_k_57 =th[7];

		//for(int i=1;i<8;i++)
		//printf("k_th5[%d]=%x\n",i,th[i]);

		rtd_outl(DI_IM_DI_RTNR_NEW_K_50_reg, rtnr_new_k_50.regValue);
		break;

	case 6:


		rtnr_new_k_60.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_K_60_reg);

		rtnr_new_k_60.rtnr_k_60 =th[0];
		rtnr_new_k_60.rtnr_k_61 =th[1];
		rtnr_new_k_60.rtnr_k_62 =th[2];
		rtnr_new_k_60.rtnr_k_63 =th[3];
		rtnr_new_k_60.rtnr_k_64 =th[4];
		rtnr_new_k_60.rtnr_k_65 =th[5];
		rtnr_new_k_60.rtnr_k_66 =th[6];
		rtnr_new_k_60.rtnr_k_67 =th[7];

		//for(int i=1;i<8;i++)
		//printf("k_th6[%d]=%x\n",i,th[i]);

		rtd_outl(DI_IM_DI_RTNR_NEW_K_60_reg, rtnr_new_k_60.regValue);
		break;

	case 7:


		rtnr_new_k_70.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_K_70_reg);

		rtnr_new_k_70.rtnr_k_70 = th[0];
		rtnr_new_k_70.rtnr_k_71 = th[1];
		rtnr_new_k_70.rtnr_k_72 = th[2];
		rtnr_new_k_70.rtnr_k_73 = th[3];
		rtnr_new_k_70.rtnr_k_74 = th[4];
		rtnr_new_k_70.rtnr_k_75 = th[5];
		rtnr_new_k_70.rtnr_k_76 = th[6];
		rtnr_new_k_70.rtnr_k_77 = th[7];

		//for(int i=1;i<8;i++)
		//printf("k_th7[%d]=%x\n",i,th[i]);

		rtd_outl(DI_IM_DI_RTNR_NEW_K_70_reg, rtnr_new_k_70.regValue);
		break;

	default:
		break;
	}
}

void drvif_color_newrtnr_sad_th(unsigned char edge, unsigned char *th)
{

	if (edge)
	{
		di_im_di_rtnr_new_sad_edge_th_1_RBUS rtnr_new_sad_edge_th_1;
		di_im_di_rtnr_new_sad_edge_th_2_RBUS rtnr_new_sad_edge_th_2;

		rtnr_new_sad_edge_th_1.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_EDGE_TH_1_reg);
		rtnr_new_sad_edge_th_2.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_EDGE_TH_2_reg);

		rtnr_new_sad_edge_th_1.rtnr_calculate_motion_sad_edge_th1 = th[0];
		rtnr_new_sad_edge_th_1.rtnr_calculate_motion_sad_edge_th2 = th[1];
		rtnr_new_sad_edge_th_1.rtnr_calculate_motion_sad_edge_th3 = th[2];
		rtnr_new_sad_edge_th_1.rtnr_calculate_motion_sad_edge_th4 = th[3];
		rtnr_new_sad_edge_th_2.rtnr_calculate_motion_sad_edge_th5 = th[4];
		rtnr_new_sad_edge_th_2.rtnr_calculate_motion_sad_edge_th6 = th[5];
		rtnr_new_sad_edge_th_2.rtnr_calculate_motion_sad_edge_th7 = th[6];

		//for(int i=1;i<7;i++)
		//printf("sad_edge_th[%d]=%x\n",i,th[i]);

		rtd_outl(DI_IM_DI_RTNR_NEW_SAD_EDGE_TH_1_reg, rtnr_new_sad_edge_th_1.regValue);
		rtd_outl(DI_IM_DI_RTNR_NEW_SAD_EDGE_TH_2_reg, rtnr_new_sad_edge_th_2.regValue);
	}
	else
	{
		di_im_di_rtnr_new_sad_nonedge_th_1_RBUS rtnr_new_sad_nonedge_th_1;
		di_im_di_rtnr_new_sad_nonedge_th_2_RBUS rtnr_new_sad_nonedge_th_2;

		rtnr_new_sad_nonedge_th_1.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_TH_1_reg);
		rtnr_new_sad_nonedge_th_2.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_TH_2_reg);

		rtnr_new_sad_nonedge_th_1.rtnr_calculate_motion_sad_nonedge_th1 = th[0];
		rtnr_new_sad_nonedge_th_1.rtnr_calculate_motion_sad_nonedge_th2 = th[1];
		rtnr_new_sad_nonedge_th_1.rtnr_calculate_motion_sad_nonedge_th3 = th[2];
		rtnr_new_sad_nonedge_th_1.rtnr_calculate_motion_sad_nonedge_th4 = th[3];
		rtnr_new_sad_nonedge_th_2.rtnr_calculate_motion_sad_nonedge_th5 = th[4];
		rtnr_new_sad_nonedge_th_2.rtnr_calculate_motion_sad_nonedge_th6 = th[5];
		rtnr_new_sad_nonedge_th_2.rtnr_calculate_motion_sad_nonedge_th7 = th[6];

		//for(int i=1;i<7;i++)
		//printf("sad_noedge_th[%d]=%x\n",i,th[i]);

		rtd_outl(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_TH_1_reg, rtnr_new_sad_nonedge_th_1.regValue);
		rtd_outl(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_TH_2_reg, rtnr_new_sad_nonedge_th_2.regValue);
	}
}


void drvif_color_newrtnr_sta_th(unsigned char *th)
{

	di_im_di_rtnr_new_sad_count_th_RBUS rtnr_new_sad_count_th;
	di_im_di_rtnr_new_sad_nonedge_th_2_RBUS rtnr_new_sad_nonedge_th_2;
	rtnr_new_sad_count_th.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_COUNT_TH_reg);
	rtnr_new_sad_nonedge_th_2.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_TH_2_reg);

	rtnr_new_sad_count_th.rtnr_statistic_motion_count_th1 = th[0];
	rtnr_new_sad_count_th.rtnr_statistic_motion_count_th2 = th[1];
	rtnr_new_sad_count_th.rtnr_statistic_motion_count_th3 = th[2];
	rtnr_new_sad_count_th.rtnr_statistic_motion_count_th4 = th[3];
	rtnr_new_sad_count_th.rtnr_statistic_motion_count_th5 = th[4];
	rtnr_new_sad_count_th.rtnr_statistic_motion_count_th6 = th[5];
//	rtnr_new_sad_count_th.rtnr_statistic_motion_count_th7 = th[6];
#if 0	//20150706 roger mark for compiler error
	rtnr_new_sad_nonedge_th_2.rtnr_statistic_motion_count_th7 = th[6];
#endif
	//for(int i=1;i<7;i++)
		//printf("sad_sta_th[%d]=%x\n",i,th[i]);

	rtd_outl(DI_IM_DI_RTNR_NEW_SAD_COUNT_TH_reg, rtnr_new_sad_count_th.regValue);
	rtd_outl(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_TH_2_reg, rtnr_new_sad_nonedge_th_2.regValue);
}

void drvif_color_newrtnr_edgetypediff_th(unsigned char *th)
{

	di_im_di_rtnr_new_edgetypediff_th_1_RBUS rtnr_new_edgetypediff_th_1;
	di_im_di_rtnr_new_edgetypediff_th_2_RBUS rtnr_new_edgetypediff_th_2;

	rtnr_new_edgetypediff_th_1.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_EDGETYPEDIFF_TH_1_reg);
	rtnr_new_edgetypediff_th_2.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_EDGETYPEDIFF_TH_2_reg);

	rtnr_new_edgetypediff_th_1.rtnr_edgetypediff_count_th1 = th[0];
	rtnr_new_edgetypediff_th_1.rtnr_edgetypediff_count_th2 = th[1];
	rtnr_new_edgetypediff_th_1.rtnr_edgetypediff_count_th3 = th[2];
	rtnr_new_edgetypediff_th_1.rtnr_edgetypediff_count_th4 = th[3];
	rtnr_new_edgetypediff_th_2.rtnr_edgetypediff_count_th5 = th[4];
	rtnr_new_edgetypediff_th_2.rtnr_edgetypediff_count_th6 = th[5];
	rtnr_new_edgetypediff_th_2.rtnr_edgetypediff_count_th7 = th[6];

	//for(int i=1;i<7;i++)
		//printf("sad_dgetype_th[%d]=%x\n",i,th[i]);

	rtd_outl(DI_IM_DI_RTNR_NEW_EDGETYPEDIFF_TH_1_reg, rtnr_new_edgetypediff_th_1.regValue);
	rtd_outl(DI_IM_DI_RTNR_NEW_EDGETYPEDIFF_TH_2_reg, rtnr_new_edgetypediff_th_2.regValue);
}
#endif

//#endif
// end chen 170809


//>>>>>>>>>>>>>>>  new RTNR end <<<<<<<<<<<<<<<<<<<<<<<<<<<

#ifdef ISR_AMBILIGHT_ENABLE
unsigned char drvif_color_dragon_profile_dectector_ambilight(unsigned char display, unsigned short *H_ambBlock, unsigned short *V_ambBlock, unsigned short *Profile_transition_sum, unsigned char hx_offset, unsigned char vy_offset, unsigned char Color_sel)
{

//This function will return profile data
//in Profile block, input is always 8 bit(truncated from 10 bit)
//	scalerDrvPrint("=================================  Start of Colobar_dectector =================================\n");
	unsigned int h_tmp, v_tmp;
	//unsigned short Active_Region[4];
	unsigned short IphActWid_H=0,IpvActLen_V=0;
	static unsigned char ini_flag = 1;
	static unsigned char color_set = 0;
	unsigned char ret, color, i;
	unsigned char h_shift_num=0, v_shift_num=0;
	unsigned char block_num = 4; //16 block in profile
	unsigned short *H_Block = &(SmartPic_clue->Profile_Cal_INFO.H_Block_Y[0]);
	unsigned short *V_Block = &(SmartPic_clue->Profile_Cal_INFO.V_Block_Y[0]);
	vgip_vgip_chn1_act_hsta_width_RBUS vgip_chn1_act_hsta_width_reg;
	vgip_vgip_chn1_act_vsta_length_RBUS vgip_chn1_act_vsta_length_reg;
	#if AMBI_PROFILE_DETECT_DEBUG_TPV
	static unsigned char profile_dectector_count = 0;
	profile_dectector_count++;
	if((profile_dectector_count % 300) == 0)
	ROSPrintf("\n===1\n");
	#endif

	vgip_chn1_act_hsta_width_reg.regValue  = scaler_rtd_inl(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);
	vgip_chn1_act_vsta_length_reg.regValue = scaler_rtd_inl(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);
	// color select, 0=y, 1=u, 2=v, 3=y, then u, then v.
	if(Color_sel<3)
		color = Color_sel;
	else
		color = color_set;

	profile_ich1_hist_profile_ctrl_RBUS	hist_profile_ctrl_reg;
	profile_ich1_hist_profile_size_RBUS hist_profile_size_reg;
	profile_ich1_hist_profile_starth_RBUS hist_profile_starth_reg;
	profile_ich1_hist_profile_startv_RBUS hist_profile_startv_reg;

	profile_ich1_hist_profile_data_00_RBUS hist_profile_data_00_reg;
	profile_ich1_hist_profile_data_01_RBUS hist_profile_data_01_reg;
	profile_ich1_hist_profile_data_02_RBUS hist_profile_data_02_reg;
	profile_ich1_hist_profile_data_03_RBUS hist_profile_data_03_reg;
	profile_ich1_hist_profile_data_04_RBUS hist_profile_data_04_reg;
	profile_ich1_hist_profile_data_05_RBUS hist_profile_data_05_reg;
	profile_ich1_hist_profile_data_06_RBUS hist_profile_data_06_reg;
	profile_ich1_hist_profile_data_07_RBUS hist_profile_data_07_reg;
	profile_ich1_hist_profile_data_08_RBUS hist_profile_data_08_reg;
	profile_ich1_hist_profile_data_09_RBUS hist_profile_data_09_reg;
	profile_ich1_hist_profile_data_10_RBUS hist_profile_data_10_reg;
	profile_ich1_hist_profile_data_11_RBUS hist_profile_data_11_reg;
	profile_ich1_hist_profile_data_12_RBUS hist_profile_data_12_reg;
	profile_ich1_hist_profile_data_13_RBUS hist_profile_data_13_reg;
	profile_ich1_hist_profile_data_14_RBUS hist_profile_data_14_reg;
	profile_ich1_hist_profile_data_15_RBUS hist_profile_data_15_reg;

	profile_ich1_hist_profile_data_16_RBUS hist_profile_data_16_reg;
	profile_ich1_hist_profile_data_17_RBUS hist_profile_data_17_reg;
	profile_ich1_hist_profile_data_18_RBUS hist_profile_data_18_reg;
	profile_ich1_hist_profile_data_19_RBUS hist_profile_data_19_reg;
	profile_ich1_hist_profile_data_20_RBUS hist_profile_data_20_reg;
	profile_ich1_hist_profile_data_21_RBUS hist_profile_data_21_reg;
	profile_ich1_hist_profile_data_22_RBUS hist_profile_data_22_reg;
	profile_ich1_hist_profile_data_23_RBUS hist_profile_data_23_reg;



	hist_profile_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_CTRL_reg);
	hist_profile_size_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_size_reg);
	hist_profile_starth_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_StartH_reg);
	hist_profile_startv_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_StartV_reg);

	//drvif_color_activeregion_dragon(display, &Active_Region[0]);
	IphActWid_H=(SmartPic_clue->SCALER_DISP_INFO.IphActWid_H);
	IpvActLen_V=(SmartPic_clue->SCALER_DISP_INFO.IpvActLen_V);
//	scalerDrvPrint("Active_Region[0]=%d\n",Active_Region[0]);
//	scalerDrvPrint("Active_Region[1]=%d\n",Active_Region[1]);
//	scalerDrvPrint("Active_Region[2]=%d\n",Active_Region[2]);
//	scalerDrvPrint("Active_Region[3]=%d\n",Active_Region[3]);

	//Set block size
    switch(SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:
		case _MODE_576I:
		case _MODE_480P:
		case _MODE_576P:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x2;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x1;
			h_shift_num = 9;
			v_shift_num = 10;
			#if AMBI_PROFILE_DETECT_DEBUG_TPV
			if((profile_dectector_count % 300) == 0)
			ROSPrintf("\n===1.1\n");
			#endif
			break;
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1080I25:
		case _MODE_1080I30:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x3;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x2;
			h_shift_num = 10;
			v_shift_num = 11;
			#if AMBI_PROFILE_DETECT_DEBUG_TPV
			if((profile_dectector_count % 300) == 0)
			ROSPrintf("\n===1.2\n");
			#endif
			break;
		case _MODE_1080P50:
		case _MODE_1080P60:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x3;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x3;
			h_shift_num = 10;
			v_shift_num = 12;
			#if AMBI_PROFILE_DETECT_DEBUG_TPV
			if((profile_dectector_count % 300) == 0)
			ROSPrintf("\n===1.3\n");
			#endif
			break;
		default:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x3;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x2;
			h_shift_num = 10;
			v_shift_num = 11;
			#if AMBI_PROFILE_DETECT_DEBUG_TPV
			if((profile_dectector_count % 300) == 0)
			ROSPrintf("\n===1.4\n");
			#endif
			break;
	}
	#if AMBI_PROFILE_DETECT_DEBUG_TPV
	if((profile_dectector_count % 300) == 0)
	ROSPrintf("\n====2=  h_shift=%d,v_shift= %d\n"  ,h_shift_num,v_shift_num);
	#endif
	hist_profile_size_reg.ch1_pf_hsize = (IphActWid_H>>block_num) - ((2*hx_offset)>>block_num) - 1;//according to spec., size should be minus 1
	hist_profile_size_reg.ch1_pf_vsize = (IpvActLen_V>>block_num) - ((2*vy_offset)>>block_num) - 1;//according to spec., size should be minus 1
	//Set Start Position
	hist_profile_starth_reg.ch1_pf_starth = hx_offset;
	hist_profile_startv_reg.ch1_pf_startv = vy_offset;
	#if AMBI_PROFILE_DETECT_DEBUG_TPV
	if((profile_dectector_count % 300) == 0)
	ROSPrintf("\n====2.1=  hsize=%d ,vsize=%d\n"  ,hist_profile_size_reg.ch1_pf_hsize,hist_profile_size_reg.ch1_pf_vsize);
	#endif
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_size_reg, hist_profile_size_reg.regValue);
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_StartH_reg, hist_profile_starth_reg.regValue);
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_StartV_reg, hist_profile_startv_reg.regValue);


	if(ini_flag)
	{
		//enable profile
		hist_profile_ctrl_reg.ch1_pf_field_sel = 0x2;//next field
		hist_profile_ctrl_reg.ch1_pf_enable_profile = 0x1;
		ini_flag = 0;
	}


	// add for truncate HNSR & VNSR value before read his_profile, csfanchiang 20100908
	hist_profile_ctrl_reg.ch1_pf_yuv_sel = color;

#ifdef VPQ_COMPILER_ERROR_ENABLE
	scaler_rtd_outl(PROFILE_ICH1_HIST_PROFILE_CTRL_reg, hist_profile_ctrl_reg.regValue);
#endif

	hist_profile_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_CTRL_reg);
	#if AMBI_PROFILE_DETECT_DEBUG_TPV
	if((profile_dectector_count % 60) == 0)
	ROSPrintf("\n===3====  IphActWid_H=%d ,IpvActLen_V=%d \n",IphActWid_H,IpvActLen_V);
	#endif
	if(hist_profile_ctrl_reg.ch1_pf_enable_profile==0)//wait for finish flag
	{
		hist_profile_data_00_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_00_reg);
		hist_profile_data_01_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_01_reg);
		hist_profile_data_02_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_02_reg);
		hist_profile_data_03_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_03_reg);
		hist_profile_data_04_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_04_reg);
		hist_profile_data_05_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_05_reg);
		hist_profile_data_06_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_06_reg);
		hist_profile_data_07_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_07_reg);
		hist_profile_data_08_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_08_reg);
		hist_profile_data_09_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_09_reg);
		hist_profile_data_10_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_10_reg);
		hist_profile_data_11_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_11_reg);
		hist_profile_data_12_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_12_reg);
		hist_profile_data_13_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_13_reg);
		hist_profile_data_14_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_14_reg);
		hist_profile_data_15_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_15_reg);

		hist_profile_data_16_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_16_reg);
		hist_profile_data_17_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_17_reg);
		hist_profile_data_18_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_18_reg);
		hist_profile_data_19_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_19_reg);
		hist_profile_data_20_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_20_reg);
		hist_profile_data_21_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_21_reg);
		hist_profile_data_22_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_22_reg);
		hist_profile_data_23_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_Data_23_reg);

		H_ambBlock[0]   = hist_profile_data_00_reg.ch1_pf_h00;
		H_ambBlock[1]   = hist_profile_data_00_reg.ch1_pf_h01;
		H_ambBlock[2]   = hist_profile_data_01_reg.ch1_pf_h02;
		H_ambBlock[3]   = hist_profile_data_01_reg.ch1_pf_h03;
		H_ambBlock[4]   = hist_profile_data_02_reg.ch1_pf_h04;
		H_ambBlock[5]   = hist_profile_data_02_reg.ch1_pf_h05;
		H_ambBlock[6]   = hist_profile_data_03_reg.ch1_pf_h06;
		H_ambBlock[7]   = hist_profile_data_03_reg.ch1_pf_h07;
		H_ambBlock[8]   = hist_profile_data_04_reg.ch1_pf_h08;
		H_ambBlock[9]   = hist_profile_data_04_reg.ch1_pf_h09;
		H_ambBlock[10] = hist_profile_data_05_reg.ch1_pf_h10;
		H_ambBlock[11] = hist_profile_data_05_reg.ch1_pf_h11;
		H_ambBlock[12] = hist_profile_data_06_reg.ch1_pf_h12;
		H_ambBlock[13] = hist_profile_data_06_reg.ch1_pf_h13;
		H_ambBlock[14] = hist_profile_data_07_reg.ch1_pf_h14;
		H_ambBlock[15] = hist_profile_data_07_reg.ch1_pf_h15;

		V_ambBlock[0]   = hist_profile_data_08_reg.ch1_pf_v00;
		V_ambBlock[1]   = hist_profile_data_08_reg.ch1_pf_v01;
		V_ambBlock[2]   = hist_profile_data_09_reg.ch1_pf_v02;
		V_ambBlock[3]   = hist_profile_data_09_reg.ch1_pf_v03;
		V_ambBlock[4]   = hist_profile_data_10_reg.ch1_pf_v04;
		V_ambBlock[5]   = hist_profile_data_10_reg.ch1_pf_v05;
		V_ambBlock[6]   = hist_profile_data_11_reg.ch1_pf_v06;
		V_ambBlock[7]   = hist_profile_data_11_reg.ch1_pf_v07;
		V_ambBlock[8]   = hist_profile_data_12_reg.ch1_pf_v08;
		V_ambBlock[9]   = hist_profile_data_12_reg.ch1_pf_v09;
		V_ambBlock[10] = hist_profile_data_13_reg.ch1_pf_v10;
		V_ambBlock[11] = hist_profile_data_13_reg.ch1_pf_v11;
		V_ambBlock[12] = hist_profile_data_14_reg.ch1_pf_v12;
		V_ambBlock[13] = hist_profile_data_14_reg.ch1_pf_v13;
		V_ambBlock[14] = hist_profile_data_15_reg.ch1_pf_v14;
		V_ambBlock[15] = hist_profile_data_15_reg.ch1_pf_v15;

		if(hist_profile_ctrl_reg.ch1_pf_yuv_sel == 0)
		{
		H_Block[0]   = hist_profile_data_00_reg.ch1_pf_h00;
		H_Block[1]   = hist_profile_data_00_reg.ch1_pf_h01;
		H_Block[2]   = hist_profile_data_01_reg.ch1_pf_h02;
		H_Block[3]   = hist_profile_data_01_reg.ch1_pf_h03;
		H_Block[4]   = hist_profile_data_02_reg.ch1_pf_h04;
		H_Block[5]   = hist_profile_data_02_reg.ch1_pf_h05;
		H_Block[6]   = hist_profile_data_03_reg.ch1_pf_h06;
		H_Block[7]   = hist_profile_data_03_reg.ch1_pf_h07;
		H_Block[8]   = hist_profile_data_04_reg.ch1_pf_h08;
		H_Block[9]   = hist_profile_data_04_reg.ch1_pf_h09;
		H_Block[10] = hist_profile_data_05_reg.ch1_pf_h10;
		H_Block[11] = hist_profile_data_05_reg.ch1_pf_h11;
		H_Block[12] = hist_profile_data_06_reg.ch1_pf_h12;
		H_Block[13] = hist_profile_data_06_reg.ch1_pf_h13;
		H_Block[14] = hist_profile_data_07_reg.ch1_pf_h14;
		H_Block[15] = hist_profile_data_07_reg.ch1_pf_h15;

		V_Block[0]   = hist_profile_data_08_reg.ch1_pf_v00;
		V_Block[1]   = hist_profile_data_08_reg.ch1_pf_v01;
		V_Block[2]   = hist_profile_data_09_reg.ch1_pf_v02;
		V_Block[3]   = hist_profile_data_09_reg.ch1_pf_v03;
		V_Block[4]   = hist_profile_data_10_reg.ch1_pf_v04;
		V_Block[5]   = hist_profile_data_10_reg.ch1_pf_v05;
		V_Block[6]   = hist_profile_data_11_reg.ch1_pf_v06;
		V_Block[7]   = hist_profile_data_11_reg.ch1_pf_v07;
		V_Block[8]   = hist_profile_data_12_reg.ch1_pf_v08;
		V_Block[9]   = hist_profile_data_12_reg.ch1_pf_v09;
		V_Block[10] = hist_profile_data_13_reg.ch1_pf_v10;
		V_Block[11] = hist_profile_data_13_reg.ch1_pf_v11;
		V_Block[12] = hist_profile_data_14_reg.ch1_pf_v12;
		V_Block[13] = hist_profile_data_14_reg.ch1_pf_v13;
		V_Block[14] = hist_profile_data_15_reg.ch1_pf_v14;
		V_Block[15] = hist_profile_data_15_reg.ch1_pf_v15;
		}


		#if AMBI_PROFILE_DETECT_DEBUG_TPV
		if((profile_dectector_count % 60) == 0)
		ROSPrintf("\n===4\n");
		#endif
		// caculate average,
		for(i=0;i<16;i++)
		{
			h_tmp = H_ambBlock[i];
			v_tmp = V_ambBlock[i];
			#if AMBI_PROFILE_DETECT_DEBUG_TPV
			if((profile_dectector_count % 60) == 0)
		        ROSPrintf("\n== yuv_sel = %d , H_Block[%d] = %d , V_Block[%d] = %d \n",color,i,H_ambBlock[i],i,V_ambBlock[i]);
			#endif
			if(((hist_profile_size_reg.ch1_pf_vsize+1)*(IphActWid_H-(2*hx_offset))) == 0)
				H_ambBlock[i] = 0;
			else
				H_ambBlock[i] = (unsigned short)((h_tmp<<h_shift_num) / ((hist_profile_size_reg.ch1_pf_vsize+1)*(IphActWid_H-(2*hx_offset))))<<2;//left shift 2 for 8 bit to 10bit
			if(((hist_profile_size_reg.ch1_pf_hsize+1)*(IpvActLen_V-(2*vy_offset))) == 0)
				V_ambBlock[i] = 0;
			else
				V_ambBlock[i] = (unsigned short)((v_tmp<<v_shift_num) / ((hist_profile_size_reg.ch1_pf_hsize+1)*(IpvActLen_V-(2*vy_offset))))<<2;

			#if AMBI_PROFILE_DETECT_DEBUG_TPV
			if((profile_dectector_count % 60) == 0)
		        ROSPrintf("\n== H_Block[%d] = %d , V_Block[%d] = %d \n",i,H_ambBlock[i],i,V_ambBlock[i]);
			#endif
		}
	#if AMBI_PROFILE_DETECT_DEBUG_TPV
	if((profile_dectector_count % 60) == 0)
	ROSPrintf("\n===5\n");
	#endif
		/*
		*Profile_transition_sum    = hist_profile_data_16_reg.ch1_pf_h00_right;
		*Profile_transition_sum  += hist_profile_data_16_reg.ch1_pf_h01_right;
		*Profile_transition_sum  += hist_profile_data_17_reg.ch1_pf_h02_right;
		*Profile_transition_sum  += hist_profile_data_17_reg.ch1_pf_h03_right;
		*Profile_transition_sum  += hist_profile_data_18_reg.ch1_pf_h04_right;
		*Profile_transition_sum  += hist_profile_data_18_reg.ch1_pf_h05_right;
		*Profile_transition_sum  += hist_profile_data_19_reg.ch1_pf_h06_right;
		*Profile_transition_sum  += hist_profile_data_19_reg.ch1_pf_h07_right;
		*Profile_transition_sum  += hist_profile_data_20_reg.ch1_pf_h08_right;
		*Profile_transition_sum  += hist_profile_data_20_reg.ch1_pf_h09_right;
		*Profile_transition_sum  += hist_profile_data_21_reg.ch1_pf_h10_right;
		*Profile_transition_sum  += hist_profile_data_21_reg.ch1_pf_h11_right;
		*Profile_transition_sum  += hist_profile_data_22_reg.ch1_pf_h12_right;
		*Profile_transition_sum  += hist_profile_data_22_reg.ch1_pf_h13_right;
		*Profile_transition_sum  += hist_profile_data_23_reg.ch1_pf_h14_right;
		*Profile_transition_sum  += hist_profile_data_23_reg.ch1_pf_h15_right;
		*/
		//enable profile
		hist_profile_ctrl_reg.ch1_pf_field_sel = 0x2;//next field
//		hist_profile_ctrl_reg.profile_ch_sel = 0x0;//ch1
		hist_profile_ctrl_reg.ch1_pf_enable_profile = 0x1;

		ret = color;
		// if Color_sel == 3, get yuv data, elieli 20130131
		if(Color_sel>=3)
			color = (color>=2)? 0:color+1;

		color_set = color;
		hist_profile_ctrl_reg.ch1_pf_yuv_sel = color;

		scaler_rtd_outl(PROFILE_ICH1_HIST_PROFILE_CTRL_reg, hist_profile_ctrl_reg.regValue);
	}
	else
	{
		ret = 3;//there's no new h and v data -> do no detection
	}
	#if AMBI_PROFILE_DETECT_DEBUG_TPV
	if((profile_dectector_count % 60) == 0)
		ROSPrintf("\n === profile_sel =%d,=%d\n",color,hist_profile_ctrl_reg.ch1_pf_enable_profile);
	#endif
	return (ret);
}
#endif

void scaler_Set_VIP_info_structure(unsigned char display,_clues* SmartPic_clue)
{
	unsigned int ulTemp=0;

	//== Get disp info from scaler_disp_info. ==
	SmartPic_clue->SCALER_DISP_INFO.input_src	= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_INPUT_SRC);
	SmartPic_clue->SCALER_DISP_INFO.disp_status	= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_STATUS);
	SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr	= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MODE_CURR);
	SmartPic_clue->SCALER_DISP_INFO.display		= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISPLAY);
	SmartPic_clue->SCALER_DISP_INFO.DispWid		= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_WID);
	SmartPic_clue->SCALER_DISP_INFO.DispLen		= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DISP_LEN);
	SmartPic_clue->SCALER_DISP_INFO.CapWid		= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_CAP_WID);
	SmartPic_clue->SCALER_DISP_INFO.CapLen		= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_CAP_LEN);
	SmartPic_clue->SCALER_DISP_INFO.IpvActSTA_H	= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPH_ACT_STA);
	SmartPic_clue->SCALER_DISP_INFO.IpvActSTA_V	= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_STA);
	SmartPic_clue->SCALER_DISP_INFO.IphActWid_H	= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPH_ACT_WID);
	SmartPic_clue->SCALER_DISP_INFO.IpvActLen_V	= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_LEN);

	SmartPic_clue->SCALER_DISP_INFO.IphActSTA_Pre	= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPH_ACT_STA_PRE);//scaler_disp_info[display].IphActSTA_Pre;
	SmartPic_clue->SCALER_DISP_INFO.IpvActSTA_Pre	= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_STA_PRE);//scaler_disp_info[display].IpvActSTA_Pre;
	SmartPic_clue->SCALER_DISP_INFO.IphActWid_Pre	= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPH_ACT_WID_PRE);//scaler_disp_info[display].IphActWid_Pre;
	SmartPic_clue->SCALER_DISP_INFO.IpvActLen_Pre	= Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_LEN_PRE);//scaler_disp_info[display].IpvActLen_Pre;

	//== set system VGIP H and V DELAY Setting ==
	ulTemp = scaler_rtd_inl(VGIP_VGIP_CHN1_DELAY_reg);
	SmartPic_clue->SCALER_DISP_INFO.HDelay = (unsigned short)ulTemp  & 0x0fff;
	SmartPic_clue->SCALER_DISP_INFO.VDelay = (unsigned short)(ulTemp >> 16) & 0x0fff;

}


unsigned char drvif_color_activeregion_dragon(unsigned char display, unsigned short *Region)
{

	vgip_vgip_chn1_act_hsta_width_RBUS vgip_chn1_act_hsta_width_reg;
	vgip_vgip_chn1_act_vsta_length_RBUS vgip_chn1_act_vsta_length_reg;
	//forster modified 071128
	unsigned int ulTemp;
	unsigned short HDelay;
	unsigned short VDelay;

	vgip_chn1_act_hsta_width_reg.regValue  = scaler_rtd_inl(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);
	vgip_chn1_act_vsta_length_reg.regValue = scaler_rtd_inl(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);
	ulTemp = scaler_rtd_inl(VGIP_VGIP_CHN1_DELAY_reg);
	VDelay = (unsigned short)ulTemp  & 0x0fff;
	HDelay = (unsigned short)(ulTemp >> 16) & 0x0fff;

	Region[0] = (vgip_chn1_act_hsta_width_reg.ch1_ih_act_sta);//Active_HStart
	Region[0] += HDelay;
	Region[1] = (vgip_chn1_act_hsta_width_reg.ch1_ih_act_wid);//Active_HWidtht
	Region[2] = (vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta);//Active_VStart
	Region[2] += VDelay;
	Region[3] = (vgip_chn1_act_vsta_length_reg.ch1_iv_act_len);//Active_VLines
/*
	if(MA_print_count%150==0)
	{
		ROSPrintf("Region= %d, %d, %d, %d\n",Region[0],Region[1],Region[2],Region[3]);
		ROSPrintf("ulTemp=%d, VDelay=%d,HDelay=%d\n",ulTemp,VDelay,HDelay);

		ROSPrintf("DispLen=%d,DispWid=%d\n",scaler_disp_info[SLR_MAIN_DISPLAY].DispLen,scaler_disp_info[SLR_MAIN_DISPLAY].DispWid);
		ROSPrintf("MEM_ACT_WID=%d, MEM_ACT_LEN=%d\n",scaler_disp_info[SLR_MAIN_DISPLAY].MEM_ACT_WID,scaler_disp_info[SLR_MAIN_DISPLAY].MEM_ACT_LEN);
		ROSPrintf("MEM_ACT_HSTA=%d, MEM_ACT_VSTA=%d\n",scaler_disp_info[SLR_MAIN_DISPLAY].MEM_ACT_HSTA,scaler_disp_info[SLR_MAIN_DISPLAY].MEM_ACT_VSTA);
		ROSPrintf("CapWid=%d, CapLen=%d\n",scaler_disp_info[SLR_MAIN_DISPLAY].CapWid,scaler_disp_info[SLR_MAIN_DISPLAY].CapLen);
		ROSPrintf("disp_status=%x\n",scaler_disp_info[SLR_MAIN_DISPLAY].disp_status);
		ROSPrintf("IphActWid_Pre=%d, IpvActLen_Pre=%d\n",scaler_disp_info[SLR_MAIN_DISPLAY].IphActWid_Pre,scaler_disp_info[SLR_MAIN_DISPLAY].IpvActLen_Pre);
		ROSPrintf("IpvActSTA_H=%d, IpvActSTA_V=%d\n",scaler_disp_info[SLR_MAIN_DISPLAY].IpvActSTA_H,scaler_disp_info[SLR_MAIN_DISPLAY].IpvActSTA_V);
		ROSPrintf("IphActWid_H=%d, IpvActLen_V=%d\n",scaler_disp_info[SLR_MAIN_DISPLAY].IphActWid_H,scaler_disp_info[SLR_MAIN_DISPLAY].IpvActLen_V);
	}
*/
	//20071126 for CSW ypbpr display adjustment
	/*
	#ifndef CONFIG_YPBPR_OPEN_H_V_POSITION // CSW+ for project instead of xoceco
		Region[0]+=0x32;
	#endif
       */
	return(1);
}

#ifdef ISR_SHARPNESS_BY_MEAN_ENABLE
void scalerVIP_Adjusted_Shp_by_HistMeanValue_HDMI()//for T2 @ HDMI, elieli 20130925
{
	unsigned char y_th[5]={15,20,25,30,40};
	unsigned char reduce_ratio[5]={108,115,128,128,128};//max ratio = 128
	unsigned char y_th0, y_th1, r0, r1;
	unsigned char moving_step = 1;
	unsigned char factor_index, factor;
	unsigned char text_pos_goal, text_neg_goal, edge_pos_goal, edge_neg_goal, ver_pos_goal, ver_neg_goal;
	unsigned char text_pos_cur, text_neg_cur, edge_pos_cur, edge_neg_cur, ver_pos_cur, ver_neg_cur;
	unsigned int DCC_Histogram_Mean_value = 0;

	DCC_Histogram_Mean_value = SmartPic_clue->Hist_Y_Mean_Value;

	color_sharp_dm_peaking_gain_RBUS dm_peaking_gain_reg;
	dm_peaking_gain_reg.regValue = rtd_inl(COLOR_SHARP_DM_PEAKING_GAIN_reg);
	color_sharp_dm_segpk_edgpk3_RBUS dm_segpk_edgpk3;
	dm_segpk_edgpk3.regValue = rtd_inl(COLOR_SHARP_DM_SEGPK_EDGPK3_reg);
	color_sharp_dm_segpk_vpk2_RBUS dm_segpk_vpk2;
	dm_segpk_vpk2.regValue = rtd_inl(COLOR_SHARP_DM_SEGPK_VPK2_reg);

	if(DCC_Histogram_Mean_value<=y_th[0])
		factor_index = 0;
	else if(DCC_Histogram_Mean_value<=y_th[1])
		factor_index = 1;
	else if(DCC_Histogram_Mean_value<=y_th[2])
		factor_index = 2;
	else if(DCC_Histogram_Mean_value<=y_th[3])
		factor_index = 3;
	else if(DCC_Histogram_Mean_value<=y_th[4])
		factor_index = 4;
	else
		factor_index = 5;

	if(factor_index==0)
		factor = reduce_ratio[0];
	else if(factor_index == 5)
		factor = reduce_ratio[4];
	else
	{
		y_th0 = y_th[factor_index-1];
		y_th1 = y_th[factor_index];
		r0 = reduce_ratio[factor_index-1];
		r1 = reduce_ratio[factor_index];
		if(r1 >= r0)
			factor = (DCC_Histogram_Mean_value - y_th0) * (r1 - r0) / (y_th1 - y_th0) + r0;
		else
			factor =  r0 - (DCC_Histogram_Mean_value - y_th0) * (r0 - r1) / (y_th1 - y_th0);
	}

	text_pos_cur = dm_peaking_gain_reg.gain_pos;
	text_neg_cur = dm_peaking_gain_reg.gain_neg;
	edge_pos_cur = dm_segpk_edgpk3.gain_pos;
	edge_neg_cur = dm_segpk_edgpk3.gain_neg;
	ver_pos_cur = dm_segpk_vpk2.vpk_gain_pos;
	ver_neg_cur = dm_segpk_vpk2.vpk_gain_neg;

	text_pos_goal = (intelligent_picture_table[INTELLIGENT_PICTURE_TABLE_TEXTURE_POS_GAIN] * factor)>>7;//max ratio is 128
	text_neg_goal = (intelligent_picture_table[INTELLIGENT_PICTURE_TABLE_TEXTURE_NEG_GAIN] * factor)>>7;
	edge_pos_goal = (intelligent_picture_table[INTELLIGENT_PICTURE_TABLE_EDGE_POS_GAIN] * factor)>>7;
	edge_neg_goal = (intelligent_picture_table[INTELLIGENT_PICTURE_TABLE_EDGE_NEG_GAIN] * factor)>>7;
	ver_pos_goal = (intelligent_picture_table[INTELLIGENT_PICTURE_TABLE_VERTICAL_POS_GAIN] * factor)>>7;
	ver_neg_goal = (intelligent_picture_table[INTELLIGENT_PICTURE_TABLE_VERTICAL_NEG_GAIN] * factor)>>7;

	if(text_pos_cur>text_pos_goal)
		text_pos_cur = text_pos_cur - moving_step;
	else if(text_pos_cur<text_pos_goal)
		text_pos_cur = text_pos_cur + moving_step;
	else
		text_pos_cur = text_pos_cur;

	if(text_neg_cur>text_neg_goal)
		text_neg_cur = text_neg_cur - moving_step;
	else if(text_neg_cur<text_neg_goal)
		text_neg_cur = text_neg_cur + moving_step;
	else
		text_neg_cur = text_neg_cur;

	if(edge_pos_cur>edge_pos_goal)
		edge_pos_cur = edge_pos_cur - moving_step;
	else if(edge_pos_cur<edge_pos_goal)
		edge_pos_cur = edge_pos_cur + moving_step;
	else
		edge_pos_cur = edge_pos_cur;

	if(edge_neg_cur>edge_neg_goal)
		edge_neg_cur = edge_neg_cur - moving_step;
	else if(edge_neg_cur<edge_neg_goal)
		edge_neg_cur = edge_neg_cur + moving_step;
	else
		edge_neg_cur = edge_neg_cur;

	if(ver_pos_cur>ver_pos_goal)
		ver_pos_cur = ver_pos_cur - moving_step;
	else if(ver_pos_cur<ver_pos_goal)
		ver_pos_cur = ver_pos_cur + moving_step;
	else
		ver_pos_cur = ver_pos_cur;

	if(ver_neg_cur>ver_neg_goal)
		ver_neg_cur = ver_neg_cur - moving_step;
	else if(ver_neg_cur<ver_neg_goal)
		ver_neg_cur = ver_neg_cur + moving_step;
	else
		ver_neg_cur = ver_neg_cur;

	dm_peaking_gain_reg.gain_pos = text_pos_cur;
	dm_peaking_gain_reg.gain_neg = text_neg_cur;
	dm_segpk_edgpk3.gain_pos = edge_pos_cur;
	dm_segpk_edgpk3.gain_neg = edge_neg_cur;
	dm_segpk_vpk2.vpk_gain_pos = ver_pos_cur;
	dm_segpk_vpk2.vpk_gain_neg = ver_neg_cur;

	rtd_outl(COLOR_SHARP_DM_PEAKING_GAIN_reg, dm_peaking_gain_reg.regValue);
	rtd_outl(COLOR_SHARP_DM_SEGPK_EDGPK3_reg, dm_segpk_edgpk3.regValue);
	rtd_outl(COLOR_SHARP_DM_SEGPK_VPK2_reg, dm_segpk_vpk2.regValue);

#if 0
	if(MA_print_count%100==0)
	{
		ROSPrintf("==========%d,%d,%d,%d,%d,%d,============\n",text_pos_goal, text_neg_goal, edge_pos_goal, edge_neg_goal, ver_pos_goal, ver_neg_goal);
		ROSPrintf("==========%d,%d,%d,%d,%d,%d,============\n",text_pos_cur, text_neg_cur, edge_pos_cur, edge_neg_cur, ver_pos_cur, ver_neg_cur);
		ROSPrintf("==========%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,============\n",intelligent_picture_table[0],intelligent_picture_table[1],
			intelligent_picture_table[2],intelligent_picture_table[3],intelligent_picture_table[4],intelligent_picture_table[5],intelligent_picture_table[6],
			intelligent_picture_table[7],intelligent_picture_table[8],intelligent_picture_table[9]);
		ROSPrintf("==========%d,%d,coRatio=%d,%d============\n",DCC_Histogram_Mean_value,factor,factor_index);
	}
#endif
}
#endif

#ifdef ISR_RTNR_SWITCH_CORRECTION_BIT_FUNCTION_ENABLE
void drvif_color_switch_rtnr_rounding_correction_bit(unsigned char switch_on)
{
	//switch correction_bit for remnant shade.
	di_im_di_rtnr_control_RBUS di_im_di_rtnr_control_RBUS_reg;
	di_im_di_rtnr_control_RBUS_reg.regValue = rtd_inl(DI_IM_DI_RTNR_CONTROL_reg);

	if((switch_on ==1)&&(pre_correction_flag==99))
	{
		if((di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction == 0)&&(di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en == 0)){
			pre_correction_flag = 0;
		}
		else if((di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction == 0)&&(di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en == 1)){
			pre_correction_flag = 1;
		}
		else if((di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction == 1)&&(di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en == 0)){
			pre_correction_flag = 2;
		}
		else if((di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction == 1)&&(di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en == 1)){
			pre_correction_flag = 3;
		}
	}

	if(switch_on ==1)
	{
		if(di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction == 1)//correction1 is enable
		{
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 0;
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 1;
			rtd_outl(DI_IM_DI_RTNR_CONTROL_reg, di_im_di_rtnr_control_RBUS_reg.regValue);
		}
		else if(di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en == 1)//correction2 is enable
		{
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 1;
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 0;
			rtd_outl(DI_IM_DI_RTNR_CONTROL_reg, di_im_di_rtnr_control_RBUS_reg.regValue);
		}
		else
		{
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 0;
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 1;
			rtd_outl(DI_IM_DI_RTNR_CONTROL_reg, di_im_di_rtnr_control_RBUS_reg.regValue);
		}
	}
	else{
		if(pre_correction_flag == 0){
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 0;
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 0;
			rtd_outl(DI_IM_DI_RTNR_CONTROL_reg, di_im_di_rtnr_control_RBUS_reg.regValue);
			pre_correction_flag = 99;
		}
		else if(pre_correction_flag == 1){
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 0;
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 1;
			rtd_outl(DI_IM_DI_RTNR_CONTROL_reg, di_im_di_rtnr_control_RBUS_reg.regValue);
			pre_correction_flag = 99;
		}
		else if(pre_correction_flag == 2){
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 1;
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 0;
			rtd_outl(DI_IM_DI_RTNR_CONTROL_reg, di_im_di_rtnr_control_RBUS_reg.regValue);
			pre_correction_flag = 99;
		}
		else if(pre_correction_flag == 3){
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 1;
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 1;
			rtd_outl(DI_IM_DI_RTNR_CONTROL_reg, di_im_di_rtnr_control_RBUS_reg.regValue);
			pre_correction_flag = 99;
		}
	}
}
#endif


//--------------------------------------------------------------------------------------
// Copy functions from VIP libs for scalerVIP.c , because it can't call lib function in kernel space
// End
//--------------------------------------------------------------------------------------

char scalerVIP_ReadProfile(_clues* SmartPic_clue)
{
	profile_ich1_hist_profile_ctrl_RBUS hist_profile_ctrl_reg;
	profile_ich1_hist_profile_size_RBUS hist_profile_size_reg;
	profile_ich1_hist_profile_starth_RBUS hist_profile_starth_reg;
	profile_ich1_hist_profile_startv_RBUS hist_profile_startv_reg;
	vgip_vgip_chn1_act_hsta_width_RBUS vgip_chn1_act_hsta_width_reg;
	vgip_vgip_chn1_act_vsta_length_RBUS vgip_chn1_act_vsta_length_reg;
	unsigned short IWid, ILen;	//I-domain width & length
	unsigned char Profile_Block_Num = 16;//set profile h&V block number
	unsigned char ii;
	unsigned char profsum_shift=0;

	unsigned short *H_Block = &(SmartPic_clue->Profile_Cal_INFO.H_Block_Y[0]);
	unsigned short *V_Block = &(SmartPic_clue->Profile_Cal_INFO.V_Block_Y[0]);

	hist_profile_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_CTRL_reg);
	hist_profile_size_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_size_reg);

	vgip_chn1_act_hsta_width_reg.regValue  = scaler_rtd_inl(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);
	vgip_chn1_act_vsta_length_reg.regValue = scaler_rtd_inl(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);

	IWid = vgip_chn1_act_hsta_width_reg.ch1_ih_act_wid; //Active_HWidtht
	ILen = vgip_chn1_act_vsta_length_reg.ch1_iv_act_len; //Active_VLines

	//IWid = scaler_disp_info[_CHANNEL1].DispWid;
	//ILen = scaler_disp_info[_CHANNEL1].DispLen;

	//Set block size
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:
		case _MODE_576I:
		case _MODE_480P:
		case _MODE_576P:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x2;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x1;
			profsum_shift = 9;
			break;
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1080I25:
		case _MODE_1080I30:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x3;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x2;
			profsum_shift = 10;
			break;
		case _MODE_1080P50:
		case _MODE_1080P60:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x3;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x3;
			profsum_shift = 10;
			break;
	}

	hist_profile_size_reg.ch1_pf_hsize = (IWid/Profile_Block_Num) -1;//according to spec., size should be minus 1
	//if( isInterlaced )
	if( (SmartPic_clue->SCALER_DISP_INFO.disp_status & _BIT8) != 0) // interlaced
		hist_profile_size_reg.ch1_pf_vsize = (ILen/Profile_Block_Num)*2 -1;//according to spec., size should be minus 1
	else
		hist_profile_size_reg.ch1_pf_vsize = (ILen/Profile_Block_Num) -1;//according to spec., size should be minus 1

	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_size_reg, hist_profile_size_reg.regValue);

	//Set Start Position
	hist_profile_starth_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_StartH_reg);
	hist_profile_startv_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_StartV_reg);
	hist_profile_starth_reg.ch1_pf_starth = 0;
	hist_profile_startv_reg.ch1_pf_startv = 0;
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_StartH_reg, hist_profile_starth_reg.regValue);
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_StartV_reg, hist_profile_startv_reg.regValue);

	if ( hist_profile_ctrl_reg.ch1_pf_enable_profile == 0 ) //wait for finish flag
	{

		profile_ich1_hist_profile_data_00_RBUS data_00;
		profile_ich1_hist_profile_data_01_RBUS data_01;
		profile_ich1_hist_profile_data_02_RBUS data_02;
		profile_ich1_hist_profile_data_03_RBUS data_03;
		profile_ich1_hist_profile_data_04_RBUS data_04;
		profile_ich1_hist_profile_data_05_RBUS data_05;
		profile_ich1_hist_profile_data_06_RBUS data_06;
		profile_ich1_hist_profile_data_07_RBUS data_07;
		profile_ich1_hist_profile_data_08_RBUS data_08;
		profile_ich1_hist_profile_data_09_RBUS data_09;
		profile_ich1_hist_profile_data_10_RBUS data_10;
		profile_ich1_hist_profile_data_11_RBUS data_11;
		profile_ich1_hist_profile_data_12_RBUS data_12;
		profile_ich1_hist_profile_data_13_RBUS data_13;
		profile_ich1_hist_profile_data_14_RBUS data_14;
		profile_ich1_hist_profile_data_15_RBUS data_15;

		data_00.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_00_reg);
		data_01.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_01_reg);
		data_02.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_02_reg);
		data_03.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_03_reg);
		data_04.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_04_reg);
		data_05.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_05_reg);
		data_06.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_06_reg);
		data_07.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_07_reg);
		data_08.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_08_reg);
		data_09.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_09_reg);
		data_10.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_10_reg);
		data_11.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_11_reg);
		data_12.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_12_reg);
		data_13.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_13_reg);
		data_14.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_14_reg);
		data_15.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_15_reg);

		H_Block[0]   = data_00.ch1_pf_h00;
		H_Block[1]   = data_00.ch1_pf_h01;
		H_Block[2]   = data_01.ch1_pf_h02;
		H_Block[3]   = data_01.ch1_pf_h03;
		H_Block[4]   = data_02.ch1_pf_h04;
		H_Block[5]   = data_02.ch1_pf_h05;
		H_Block[6]   = data_03.ch1_pf_h06;
		H_Block[7]   = data_03.ch1_pf_h07;
		H_Block[8]   = data_04.ch1_pf_h08;
		H_Block[9]   = data_04.ch1_pf_h09;
		H_Block[10] = data_05.ch1_pf_h10;
		H_Block[11] = data_05.ch1_pf_h11;
		H_Block[12] = data_06.ch1_pf_h12;
		H_Block[13] = data_06.ch1_pf_h13;
		H_Block[14] = data_07.ch1_pf_h14;
		H_Block[15] = data_07.ch1_pf_h15;

		V_Block[0]   = data_08.ch1_pf_v00;
		V_Block[1]   = data_08.ch1_pf_v01;
		V_Block[2]   = data_09.ch1_pf_v02;
		V_Block[3]   = data_09.ch1_pf_v03;
		V_Block[4]   = data_10.ch1_pf_v04;
		V_Block[5]   = data_10.ch1_pf_v05;
		V_Block[6]   = data_11.ch1_pf_v06;
		V_Block[7]   = data_11.ch1_pf_v07;
		V_Block[8]   = data_12.ch1_pf_v08;
		V_Block[9]   = data_12.ch1_pf_v09;
		V_Block[10] = data_13.ch1_pf_v10;
		V_Block[11] = data_13.ch1_pf_v11;
		V_Block[12] = data_14.ch1_pf_v12;
		V_Block[13] = data_14.ch1_pf_v13;
		V_Block[14] = data_15.ch1_pf_v14;
		V_Block[15] = data_15.ch1_pf_v15;

		//enable profile
		hist_profile_ctrl_reg.ch1_pf_field_sel = 0x2;	//next field
		hist_profile_ctrl_reg.ch1_pf_enable_profile = 0x1;
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_CTRL_reg, hist_profile_ctrl_reg.regValue);

		ProfSum = 0;
		for( ii=0; ii<16; ii++ ) {
			ProfSum += H_Block[ii];
		}
		//ProfSum = ProfSum << profsum_shift;
		//ROSPrintf( "[ScalerVIP] ========Profile OK, ProfSum=%d========\n", ProfSum );

		return SUCCESS;
	}
	else
	{
		//ROSPrintf( "[ScalerVIP] ==========Profile NOT ready==========\n" );
		return FAILED;
	}
}

void scalerVIP_scalerColor_profile(void)
{
	_system_setting_info* system_info_structure_table = NULL;
	_clues* SmartPic_clue=NULL;
	unsigned char Profile1_YUV_Sel, Profile2_YUV_Sel, Profile3_YUV_Sel ;

	system_info_structure_table = scaler_GetShare_Memory_system_setting_info_Struct();
	SmartPic_clue = scaler_GetShare_Memory_SmartPic_Clue_Struct();

	if((system_info_structure_table == NULL) || (SmartPic_clue == NULL) ) {
		if(MA_print_count%360==0)
			ROSPrintf("scalerVIP_scalerColor_profile Share Memory PQ data ERROR,system_info_structure_table = %p, SmartPic_clue = %p\n",system_info_structure_table, SmartPic_clue);
		return;
	}


	scalerVIP_fwif_color_set_profile();

	Profile1_YUV_Sel = system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[0].VipICH1_Hist_Profile_CTRL_t.ch1_pf_yuv_sel_t;
	Profile2_YUV_Sel = system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[1].VipICH1_Hist_Profile_CTRL_t.ch1_pf_yuv_sel_t;
	Profile3_YUV_Sel = system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[2].VipICH1_Hist_Profile_CTRL_t.ch1_pf_yuv_sel_t;

	#if Print_Profile_Data
  	if (Profile_Print_Count_2%500==0)
	{

		ROSPrintf( "Profile1_YUV_Sel =%d \n", Profile1_YUV_Sel);
		ROSPrintf( "Profile2_YUV_Sel =%d \n", Profile2_YUV_Sel);
		ROSPrintf( "Profile3_YUV_Sel =%d \n", Profile3_YUV_Sel);

		Profile_Print_Count_2 = 0;
	}

	Profile_Print_Count_2++;
	#endif

	//Profile 1
	#if 1   		// Profile1's ch1_pf_yuv_sel needs to set 0 to run Y for detected_black_region function, etc.
	SmartPic_clue->Profile_Cal_INFO.ProfileFlag_Y = scalerVIP_ReadProfile1_Y(SmartPic_clue);
	ProfSum = Profile1_Y_Sum;
	#else
	if(Profile1_YUV_Sel ==0)
	{
		SmartPic_clue->Profile_Cal_INFO.ProfileFlag_Y = scalerVIP_ReadProfile1_Y(SmartPic_clue);
		ProfSum = Profile1_Y_Sum;
	}
	else if(Profile1_YUV_Sel ==1)
	{
		SmartPic_clue->Profile_Cal_INFO.ProfileFlag_U = scalerVIP_ReadProfile1_U(SmartPic_clue);
		ProfSum = Profile1_U_Sum;
	}
	else if(Profile1_YUV_Sel ==2)
	{
		SmartPic_clue->Profile_Cal_INFO.ProfileFlag_V = scalerVIP_ReadProfile1_V(SmartPic_clue);
		ProfSum = Profile1_V_Sum;
	}
	else
	{
		SmartPic_clue->Profile_Cal_INFO.ProfileFlag_Y = scalerVIP_ReadProfile1_Y(SmartPic_clue);
		ProfSum = Profile1_Y_Sum;
	}
	#endif

	//Profile 2
	if(Profile2_YUV_Sel ==0)
		SmartPic_clue->Profile_Cal_INFO.Profile2_Flag_Y = scalerVIP_ReadProfile2_Y(SmartPic_clue);
	else if(Profile2_YUV_Sel ==1)
		SmartPic_clue->Profile_Cal_INFO.Profile2_Flag_U = scalerVIP_ReadProfile2_U(SmartPic_clue);
	else if(Profile2_YUV_Sel ==2)
		SmartPic_clue->Profile_Cal_INFO.Profile2_Flag_V = scalerVIP_ReadProfile2_V(SmartPic_clue);
	else
		SmartPic_clue->Profile_Cal_INFO.Profile2_Flag_Y = scalerVIP_ReadProfile2_Y(SmartPic_clue);

	//Profile 3
	if(Profile3_YUV_Sel ==0)
		SmartPic_clue->Profile_Cal_INFO.Profile3_Flag_Y = scalerVIP_ReadProfile3_Y(SmartPic_clue);
	else if(Profile3_YUV_Sel ==1)
		SmartPic_clue->Profile_Cal_INFO.Profile3_Flag_U = scalerVIP_ReadProfile3_U(SmartPic_clue);
	else if(Profile3_YUV_Sel ==2)
		SmartPic_clue->Profile_Cal_INFO.Profile3_Flag_V = scalerVIP_ReadProfile3_V(SmartPic_clue);
	else
		SmartPic_clue->Profile_Cal_INFO.Profile3_Flag_Y = scalerVIP_ReadProfile3_Y(SmartPic_clue);

}


void scalerVIP_fwif_color_set_profile(void)
{
	unsigned  short Profile1_Start_Offset_H, Profile1_Start_Offset_V;
	unsigned  short Profile2_Start_Offset_H, Profile2_Start_Offset_V;
	unsigned  short Profile3_Start_Offset_H, Profile3_Start_Offset_V;
	unsigned short Profile1_IWid, Profile1_ILen;				//I-domain width & length
	unsigned short Profile2_IWid, Profile2_ILen;
	unsigned short Profile3_IWid, Profile3_ILen;
	unsigned char Profile_Block_Num = 16;			//set profile h&V block number

	DRV_Vip_Profile  Vip_Profile_Coef = {0};
	_system_setting_info* system_info_structure_table = NULL;
	_clues* SmartPic_clue=NULL;

	system_info_structure_table = scaler_GetShare_Memory_system_setting_info_Struct();
	SmartPic_clue = scaler_GetShare_Memory_SmartPic_Clue_Struct();

	if((system_info_structure_table == NULL) || (SmartPic_clue == NULL) ) {
		if(MA_print_count%360==0)
			ROSPrintf("Share Memory PQ data ERROR,system_info_structure_table = %p, SmartPic_clue = %p\n",system_info_structure_table,SmartPic_clue);
		return;
	}


	//  Initial parameters from Misc Table.
	if(Profile_Initial_Flag==0)
	{
		//Profile 1
		Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.en_4k2k_mode = system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[0].VipICH1_Hist_Profile_CTRL_t.en_4k2k_mode_t;
		Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.en_3d_mode =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[0].VipICH1_Hist_Profile_CTRL_t.en_3d_mode_t;
		Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.ch1_pf_yuv_sel =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[0].VipICH1_Hist_Profile_CTRL_t.ch1_pf_yuv_sel_t;
		Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.ch1_pf_field_sel =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[0].VipICH1_Hist_Profile_CTRL_t.ch1_pf_field_sel_t;
		Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.ch1_pf_overflow_sel =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[0].VipICH1_Hist_Profile_CTRL_t.ch1_pf_overflow_sel_t;
		Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.ch1_pf_enable_profile =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[0].VipICH1_Hist_Profile_CTRL_t.ch1_pf_enable_profile_t;

		//Profile 2
		Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.en_4k2k_mode = system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[1].VipICH1_Hist_Profile_CTRL_t.en_4k2k_mode_t;  //  Profile2 has no en_4k2k_mode.
		Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.en_3d_mode =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[1].VipICH1_Hist_Profile_CTRL_t.en_3d_mode_t;	    	//  Profile2 has no en_3d_mode.
		Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.ch1_pf_yuv_sel =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[1].VipICH1_Hist_Profile_CTRL_t.ch1_pf_yuv_sel_t;
		Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.ch1_pf_field_sel =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[1].VipICH1_Hist_Profile_CTRL_t.ch1_pf_field_sel_t;
		Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.ch1_pf_overflow_sel =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[1].VipICH1_Hist_Profile_CTRL_t.ch1_pf_overflow_sel_t;
		Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.ch1_pf_enable_profile =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[1].VipICH1_Hist_Profile_CTRL_t.ch1_pf_enable_profile_t;

		//Profile 3
		Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.en_4k2k_mode = system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[2].VipICH1_Hist_Profile_CTRL_t.en_4k2k_mode_t;	//  Profile3 has no en_4k2k_mode.
		Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.en_3d_mode =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[2].VipICH1_Hist_Profile_CTRL_t.en_3d_mode_t;		//  Profile3 has no en_3d_mode.
		Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.ch1_pf_yuv_sel =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[2].VipICH1_Hist_Profile_CTRL_t.ch1_pf_yuv_sel_t;
		Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.ch1_pf_field_sel =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[2].VipICH1_Hist_Profile_CTRL_t.ch1_pf_field_sel_t;
		Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.ch1_pf_overflow_sel =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[2].VipICH1_Hist_Profile_CTRL_t.ch1_pf_overflow_sel_t;
		Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.ch1_pf_enable_profile =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[2].VipICH1_Hist_Profile_CTRL_t.ch1_pf_enable_profile_t;

	}

	#if Print_Profile_Data
  	if (Profile_Print_Count_1%3000 ==0)
	{
			ROSPrintf( "Profile0 - en_4k2k_mode =%d \n", Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.en_4k2k_mode);
			ROSPrintf( "en_3d_mode  =%d \n", Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.en_3d_mode);
			ROSPrintf( "ch1_pf_yuv_sel =%d \n", Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.ch1_pf_yuv_sel);
			ROSPrintf( "ch1_pf_field_sel =%d \n", Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.ch1_pf_field_sel);
			ROSPrintf( "ch1_pf_overflow_sel =%d \n", Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.ch1_pf_overflow_sel);
			ROSPrintf( "ch1_pf_enable_profile =%d \n\n", Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.ch1_pf_enable_profile);

			ROSPrintf( "Profile1 -en_4k2k_mode =%d \n", Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.en_4k2k_mode);
			ROSPrintf( "en_3d_mode =%d \n", Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.en_3d_mode);
			ROSPrintf( "ch1_pf_yuv_sel =%d \n",Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.ch1_pf_yuv_sel);
			ROSPrintf( "ch1_pf_field_sel  =%d \n",Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.ch1_pf_field_sel);
			ROSPrintf( "ch1_pf_overflow_sel =%d \n", Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.ch1_pf_overflow_sel);
			ROSPrintf( "ch1_pf_enable_profile =%d \n\n", Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.ch1_pf_enable_profile);

			ROSPrintf( "Profile2 -en_4k2k_mode =%d \n", Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.en_4k2k_mode);
			ROSPrintf( "en_3d_mode  =%d \n", Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.en_3d_mode);
			ROSPrintf( "ch1_pf_yuv_sel =%d \n",  Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.ch1_pf_yuv_sel);
			ROSPrintf( "ch1_pf_field_sel =%d \n", Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.ch1_pf_field_sel);
			ROSPrintf( "ch1_pf_overflow_sel =%d \n",Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.ch1_pf_overflow_sel);
			ROSPrintf( "ch1_pf_enable_profile =%d \n\n", Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.ch1_pf_enable_profile);

			Profile_Print_Count_1 = 0;
	}

	Profile_Print_Count_1++;
	#endif

	// Get H & V Start Offset from Misc Table.
	Profile1_Start_Offset_H =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[0].VipICH1_Hist_Profile_StartH_t.ch1_pf_starth_t;
	Profile1_Start_Offset_V =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[0].VipICH1_Hist_Profile_StartV_t.ch1_pf_startv_t;

	Profile2_Start_Offset_H =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[1].VipICH1_Hist_Profile_StartH_t.ch1_pf_starth_t;
	Profile2_Start_Offset_V =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[1].VipICH1_Hist_Profile_StartV_t.ch1_pf_startv_t;

	Profile3_Start_Offset_H =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[2].VipICH1_Hist_Profile_StartH_t.ch1_pf_starth_t;
	Profile3_Start_Offset_V =  system_info_structure_table->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[2].VipICH1_Hist_Profile_StartV_t.ch1_pf_startv_t;

	if((Profile1_Start_Offset_H > 1000)||(Profile1_Start_Offset_V > 500))
	{
		Profile1_Start_Offset_H = 0;
		Profile1_Start_Offset_V = 0;
	}

	if((Profile2_Start_Offset_H > 1000)||(Profile2_Start_Offset_V > 500))
	{
		Profile2_Start_Offset_H = 0;
		Profile2_Start_Offset_V = 0;
	}

	if((Profile3_Start_Offset_H > 1000)||(Profile3_Start_Offset_V > 500))
	{
		Profile3_Start_Offset_H = 0;
		Profile3_Start_Offset_V = 0;
	}

	//Set Profile Start Position
	Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_StartH.ch1_pf_starth  = Profile1_Start_Offset_H;
	Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_StartV.ch1_pf_startv = Profile1_Start_Offset_V;

	Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_StartH.ch1_pf_starth  = Profile2_Start_Offset_H;
	Vip_Profile_Coef.Vip_Profile_Info[1].VipICH1_Hist_Profile_StartV.ch1_pf_startv = Profile2_Start_Offset_V;

	Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_StartH.ch1_pf_starth  = Profile3_Start_Offset_H;
	Vip_Profile_Coef.Vip_Profile_Info[2].VipICH1_Hist_Profile_StartV.ch1_pf_startv = Profile3_Start_Offset_V;

	//Set Profile Block Size
	// Profile 1
	Profile1_IWid = SmartPic_clue->SCALER_DISP_INFO.IphActWid_H;
	Profile1_ILen = SmartPic_clue->SCALER_DISP_INFO.IpvActLen_V;

	Profile1_IWid -= Profile1_Start_Offset_H*2;
	Profile1_ILen	-= Profile1_Start_Offset_V*2;

	Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_Size.ch1_pf_hsize  = (Profile1_IWid/Profile_Block_Num) -1;		//according to spec., size should be minus 1

	if( (SmartPic_clue->SCALER_DISP_INFO.disp_status & _BIT8) != 0 ) 	// Interlaced
		Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_Size.ch1_pf_vsize = (Profile1_ILen/Profile_Block_Num)*2 -1;	//according to spec., size should be minus 1
	else
		Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_Size.ch1_pf_vsize = (Profile1_ILen/Profile_Block_Num) -1;	//according to spec., size should be minus 1

	// Profile 2
	Profile2_IWid = SmartPic_clue->SCALER_DISP_INFO.IphActWid_H;
	Profile2_ILen = SmartPic_clue->SCALER_DISP_INFO.IpvActLen_V;

	Profile2_IWid -= Profile1_Start_Offset_H*2;
	Profile2_ILen	-= Profile1_Start_Offset_V*2;

	Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_Size.ch1_pf_hsize  = (Profile2_IWid/Profile_Block_Num) -1;		//according to spec., size should be minus 1

	if( (SmartPic_clue->SCALER_DISP_INFO.disp_status & _BIT8) != 0 ) 	// Interlaced
		Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_Size.ch1_pf_vsize = (Profile2_ILen/Profile_Block_Num)*2 -1;	//according to spec., size should be minus 1
	else
		Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_Size.ch1_pf_vsize = (Profile2_ILen/Profile_Block_Num) -1;	//according to spec., size should be minus 1

	// Profile 3
	Profile3_IWid = SmartPic_clue->SCALER_DISP_INFO.IphActWid_H;
	Profile3_ILen = SmartPic_clue->SCALER_DISP_INFO.IpvActLen_V;

	Profile3_IWid -= Profile1_Start_Offset_H*2;
	Profile3_ILen	-= Profile1_Start_Offset_V*2;

	Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_Size.ch1_pf_hsize  = (Profile3_IWid/Profile_Block_Num) -1;		//according to spec., size should be minus 1

	if( (SmartPic_clue->SCALER_DISP_INFO.disp_status & _BIT8) != 0 ) 	// Interlaced
		Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_Size.ch1_pf_vsize = (Profile3_ILen/Profile_Block_Num)*2 -1;	//according to spec., size should be minus 1
	else
		Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_Size.ch1_pf_vsize = (Profile3_ILen/Profile_Block_Num) -1;	//according to spec., size should be minus 1

	scalerVIP_drvif_color_profile(&Vip_Profile_Coef);

	Profile1_H_Block_size = Profile1_IWid*(Vip_Profile_Coef.Vip_Profile_Info[0].VipICH1_Hist_Profile_Size.ch1_pf_vsize+1);
}


void scalerVIP_drvif_color_profile(DRV_Vip_Profile *ptr)
{

	profile_ich1_hist_profile_ctrl_RBUS hist_profile_ctrl_reg;
	profile_ich1_hist_profile_size_RBUS hist_profile_size_reg;
	profile_ich1_hist_profile_starth_RBUS hist_profile_starth_reg;
	profile_ich1_hist_profile_startv_RBUS hist_profile_startv_reg;

	profile_ich1_hist_profile2_ctrl_RBUS hist_profile2_ctrl_reg;
	profile_ich1_hist_profile2_size_RBUS hist_profile2_size_reg;
	profile_ich1_hist_profile2_starth_RBUS hist_profile2_starth_reg;
	profile_ich1_hist_profile2_startv_RBUS hist_profile2_startv_reg;

	profile_ich1_hist_profile3_ctrl_RBUS hist_profile3_ctrl_reg;
	profile_ich1_hist_profile3_size_RBUS hist_profile3_size_reg;
	profile_ich1_hist_profile3_starth_RBUS hist_profile3_starth_reg;
	profile_ich1_hist_profile3_startv_RBUS hist_profile3_startv_reg;

	// Profile1
	hist_profile_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_CTRL_reg);
	hist_profile_size_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_size_reg);
	hist_profile_starth_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_StartH_reg);
	hist_profile_startv_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_StartV_reg);

	// Profile2
	hist_profile2_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile2_CTRL_reg);
	hist_profile2_size_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile2_size_reg);
	hist_profile2_starth_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile2_StartH_reg);
	hist_profile2_startv_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile2_StartV_reg);

	// Profile3
	hist_profile3_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile3_CTRL_reg);
	hist_profile3_size_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile3_size_reg);
	hist_profile3_starth_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile3_StartH_reg);
	hist_profile3_startv_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile3_StartV_reg);

	if(Profile_Initial_Flag==0)
	{
		// Profile1
		hist_profile_ctrl_reg.en_4k2k_mode = ptr->Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.en_4k2k_mode;
		hist_profile_ctrl_reg.en_3d_mode =ptr->Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.en_3d_mode;
		hist_profile_ctrl_reg.ch1_pf_yuv_sel = ptr->Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.ch1_pf_yuv_sel;
		hist_profile_ctrl_reg.ch1_pf_field_sel = ptr->Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.ch1_pf_field_sel;
		hist_profile_ctrl_reg.ch1_pf_overflow_sel = ptr->Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.ch1_pf_overflow_sel;
		hist_profile_ctrl_reg.ch1_pf_enable_profile = ptr->Vip_Profile_Info[0].VipICH1_Hist_Profile_CTRL.ch1_pf_enable_profile;

		// Profile2
		hist_profile2_ctrl_reg.ch1_pf2_yuv_sel = ptr->Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.ch1_pf_yuv_sel;
		hist_profile2_ctrl_reg.ch1_pf2_field_sel = ptr->Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.ch1_pf_field_sel;
		hist_profile2_ctrl_reg.ch1_pf2_overflow_sel = ptr->Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.ch1_pf_overflow_sel;
		hist_profile2_ctrl_reg.ch1_pf2_enable_profile = ptr->Vip_Profile_Info[1].VipICH1_Hist_Profile_CTRL.ch1_pf_enable_profile;

		// Profile3
		hist_profile3_ctrl_reg.ch1_pf3_yuv_sel = ptr->Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.ch1_pf_yuv_sel;
		hist_profile3_ctrl_reg.ch1_pf3_field_sel = ptr->Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.ch1_pf_field_sel;
		hist_profile3_ctrl_reg.ch1_pf3_overflow_sel = ptr->Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.ch1_pf_overflow_sel;
		hist_profile3_ctrl_reg.ch1_pf3_enable_profile = ptr->Vip_Profile_Info[2].VipICH1_Hist_Profile_CTRL.ch1_pf_enable_profile;
	}

	// Profile1
	hist_profile_size_reg.ch1_pf_vsize = ptr->Vip_Profile_Info[0].VipICH1_Hist_Profile_Size.ch1_pf_vsize;
	hist_profile_size_reg.ch1_pf_hsize = ptr->Vip_Profile_Info[0].VipICH1_Hist_Profile_Size.ch1_pf_hsize;
	hist_profile_starth_reg.ch1_pf_starth= ptr->Vip_Profile_Info[0].VipICH1_Hist_Profile_StartH.ch1_pf_starth;
	hist_profile_startv_reg.ch1_pf_startv = ptr->Vip_Profile_Info[0].VipICH1_Hist_Profile_StartV.ch1_pf_startv;

	// Profile2
	hist_profile2_size_reg.ch1_pf2_vsize = ptr->Vip_Profile_Info[1].VipICH1_Hist_Profile_Size.ch1_pf_vsize;
	hist_profile2_size_reg.ch1_pf2_hsize = ptr->Vip_Profile_Info[1].VipICH1_Hist_Profile_Size.ch1_pf_hsize;
	hist_profile2_starth_reg.ch1_pf2_starth= ptr->Vip_Profile_Info[1].VipICH1_Hist_Profile_StartH.ch1_pf_starth;
	hist_profile2_startv_reg.ch1_pf2_startv = ptr->Vip_Profile_Info[1].VipICH1_Hist_Profile_StartV.ch1_pf_startv;

	// Profile3
	hist_profile3_size_reg.ch1_pf3_vsize = ptr->Vip_Profile_Info[2].VipICH1_Hist_Profile_Size.ch1_pf_vsize;
	hist_profile3_size_reg.ch1_pf3_hsize = ptr->Vip_Profile_Info[2].VipICH1_Hist_Profile_Size.ch1_pf_hsize;
	hist_profile3_starth_reg.ch1_pf3_starth= ptr->Vip_Profile_Info[2].VipICH1_Hist_Profile_StartH.ch1_pf_starth;
	hist_profile3_startv_reg.ch1_pf3_startv = ptr->Vip_Profile_Info[2].VipICH1_Hist_Profile_StartV.ch1_pf_startv;


	if(Profile_Initial_Flag==0)
	{
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_CTRL_reg, hist_profile_ctrl_reg.regValue);
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile2_CTRL_reg, hist_profile2_ctrl_reg.regValue);
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile3_CTRL_reg, hist_profile3_ctrl_reg.regValue);
		Profile_Initial_Flag=1;
	}

	// Profile1
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_size_reg, hist_profile_size_reg.regValue);
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_StartH_reg, hist_profile_starth_reg.regValue);
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_StartV_reg, hist_profile_startv_reg.regValue);

	// Profile2
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile2_size_reg, hist_profile2_size_reg.regValue);
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile2_StartH_reg, hist_profile2_starth_reg.regValue);
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile2_StartV_reg, hist_profile2_startv_reg.regValue);

	// Profile3
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile3_size_reg, hist_profile3_size_reg.regValue);
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile3_StartH_reg, hist_profile3_starth_reg.regValue);
	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile3_StartV_reg, hist_profile3_startv_reg.regValue);

}


char scalerVIP_ReadProfile1_Y(_clues* SmartPic_clue)
{
	profile_ich1_hist_profile_ctrl_RBUS hist_profile_ctrl_reg;
	unsigned char profsum_shift=10;
	unsigned long long Y_Sum = 0;
	unsigned char ii;
	#if Print_Profile_Data
	unsigned char jj;
	#endif

	unsigned short *H_Block_Y = &(SmartPic_clue->Profile_Cal_INFO.H_Block_Y[0]);
	unsigned short *V_Block_Y = &(SmartPic_clue->Profile_Cal_INFO.V_Block_Y[0]);

	hist_profile_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_CTRL_reg);

	// H & V Shift right hnsr bits each line.
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:
		case _MODE_576I:
		case _MODE_480P:
		case _MODE_576P:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x2;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x1;
			profsum_shift = 9;
			break;
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1080I25:
		case _MODE_1080I30:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x3;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x2;
			profsum_shift = 10;
			break;
		case _MODE_1080P50:
		case _MODE_1080P60:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x3;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x3;
			profsum_shift = 10;
			break;
	}

	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_CTRL_reg, hist_profile_ctrl_reg.regValue);

	if ( hist_profile_ctrl_reg.ch1_pf_enable_profile == 0 ) //wait for finish flag
	{

		profile_ich1_hist_profile_data_00_RBUS data_00;
		profile_ich1_hist_profile_data_01_RBUS data_01;
		profile_ich1_hist_profile_data_02_RBUS data_02;
		profile_ich1_hist_profile_data_03_RBUS data_03;
		profile_ich1_hist_profile_data_04_RBUS data_04;
		profile_ich1_hist_profile_data_05_RBUS data_05;
		profile_ich1_hist_profile_data_06_RBUS data_06;
		profile_ich1_hist_profile_data_07_RBUS data_07;
		profile_ich1_hist_profile_data_08_RBUS data_08;
		profile_ich1_hist_profile_data_09_RBUS data_09;
		profile_ich1_hist_profile_data_10_RBUS data_10;
		profile_ich1_hist_profile_data_11_RBUS data_11;
		profile_ich1_hist_profile_data_12_RBUS data_12;
		profile_ich1_hist_profile_data_13_RBUS data_13;
		profile_ich1_hist_profile_data_14_RBUS data_14;
		profile_ich1_hist_profile_data_15_RBUS data_15;

		data_00.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_00_reg);
		data_01.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_01_reg);
		data_02.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_02_reg);
		data_03.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_03_reg);
		data_04.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_04_reg);
		data_05.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_05_reg);
		data_06.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_06_reg);
		data_07.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_07_reg);
		data_08.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_08_reg);
		data_09.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_09_reg);
		data_10.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_10_reg);
		data_11.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_11_reg);
		data_12.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_12_reg);
		data_13.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_13_reg);
		data_14.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_14_reg);
		data_15.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_15_reg);

		// Get Horizontal profile data for Y.
		H_Block_Y[0]   = data_00.ch1_pf_h00;
		H_Block_Y[1]   = data_00.ch1_pf_h01;
		H_Block_Y[2]   = data_01.ch1_pf_h02;
		H_Block_Y[3]   = data_01.ch1_pf_h03;
		H_Block_Y[4]   = data_02.ch1_pf_h04;
		H_Block_Y[5]   = data_02.ch1_pf_h05;
		H_Block_Y[6]   = data_03.ch1_pf_h06;
		H_Block_Y[7]   = data_03.ch1_pf_h07;
		H_Block_Y[8]   = data_04.ch1_pf_h08;
		H_Block_Y[9]   = data_04.ch1_pf_h09;
		H_Block_Y[10] = data_05.ch1_pf_h10;
		H_Block_Y[11] = data_05.ch1_pf_h11;
		H_Block_Y[12] = data_06.ch1_pf_h12;
		H_Block_Y[13] = data_06.ch1_pf_h13;
		H_Block_Y[14] = data_07.ch1_pf_h14;
		H_Block_Y[15] = data_07.ch1_pf_h15;

		// Get Vertical profile data for Y.
		V_Block_Y[0]   = data_08.ch1_pf_v00;
		V_Block_Y[1]   = data_08.ch1_pf_v01;
		V_Block_Y[2]   = data_09.ch1_pf_v02;
		V_Block_Y[3]   = data_09.ch1_pf_v03;
		V_Block_Y[4]   = data_10.ch1_pf_v04;
		V_Block_Y[5]   = data_10.ch1_pf_v05;
		V_Block_Y[6]   = data_11.ch1_pf_v06;
		V_Block_Y[7]   = data_11.ch1_pf_v07;
		V_Block_Y[8]   = data_12.ch1_pf_v08;
		V_Block_Y[9]   = data_12.ch1_pf_v09;
		V_Block_Y[10] = data_13.ch1_pf_v10;
		V_Block_Y[11] = data_13.ch1_pf_v11;
		V_Block_Y[12] = data_14.ch1_pf_v12;
		V_Block_Y[13] = data_14.ch1_pf_v13;
		V_Block_Y[14] = data_15.ch1_pf_v14;
		V_Block_Y[15] = data_15.ch1_pf_v15;


		//Enable profile
		hist_profile_ctrl_reg.ch1_pf_enable_profile = 0x1;
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_CTRL_reg, hist_profile_ctrl_reg.regValue);

		Profile1_Y_Sum = 0;
		for( ii=0; ii<16; ii++ ) {
			Profile1_Y_Sum += H_Block_Y[ii];
		}
		//Profile1_Y_Sum = Profile1_Y_Sum << profsum_shift;

		/* calc profile Y APL start*/
		Y_Sum = ProfSum;
		Y_Sum = Y_Sum << (profsum_shift-2);
		if (Profile1_H_Block_size)
		{
			Y_Sum=Y_Sum + (Profile1_H_Block_size>>1);
			do_div(Y_Sum, Profile1_H_Block_size);
		}

		SmartPic_clue->Profile_Cal_INFO.ProfileAPL_Y = Y_Sum & 0xffff;
		/* calc profile Y APL end*/

		#if Print_Profile_Data
		if (Profile_Print_Count%500 ==0)
		{
			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### H_Block_Y[%d] =%d ### \n", jj, H_Block_Y[jj]);
			}

			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### V_Block_Y[%d] =%d ### \n", jj, V_Block_Y[jj]);
			}

			ROSPrintf( "[ScalerVIP] ========Profile OK, Profile1_Y_Sum =%d========\n", Profile1_Y_Sum );

			Profile_Print_Count = 0;
		}

		Profile_Print_Count++;
		#endif

		return SUCCESS;
	}
	else
	{
		//ROSPrintf( "[ScalerVIP] ==========Profile NOT ready==========\n" );
		return FAILED;
	}

}

char scalerVIP_ReadProfile1_U(_clues* SmartPic_clue)
{
	profile_ich1_hist_profile_ctrl_RBUS hist_profile_ctrl_reg;

	unsigned char ii;
	#if Print_Profile_Data
	unsigned char jj;
	#endif

	unsigned short *H_Block_U = &(SmartPic_clue->Profile_Cal_INFO.H_Block_U[0]);
	unsigned short *V_Block_U = &(SmartPic_clue->Profile_Cal_INFO.V_Block_U[0]);

	hist_profile_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_CTRL_reg);

	// H & V Shift right hnsr bits each line.
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:
		case _MODE_576I:
		case _MODE_480P:
		case _MODE_576P:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x2;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x1;
			break;
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1080I25:
		case _MODE_1080I30:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x3;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x2;
			break;
		case _MODE_1080P50:
		case _MODE_1080P60:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x3;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x3;
			break;
	}

	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_CTRL_reg, hist_profile_ctrl_reg.regValue);

	if ( hist_profile_ctrl_reg.ch1_pf_enable_profile == 0 ) //wait for finish flag
	{

		profile_ich1_hist_profile_data_00_RBUS data_00;
		profile_ich1_hist_profile_data_01_RBUS data_01;
		profile_ich1_hist_profile_data_02_RBUS data_02;
		profile_ich1_hist_profile_data_03_RBUS data_03;
		profile_ich1_hist_profile_data_04_RBUS data_04;
		profile_ich1_hist_profile_data_05_RBUS data_05;
		profile_ich1_hist_profile_data_06_RBUS data_06;
		profile_ich1_hist_profile_data_07_RBUS data_07;
		profile_ich1_hist_profile_data_08_RBUS data_08;
		profile_ich1_hist_profile_data_09_RBUS data_09;
		profile_ich1_hist_profile_data_10_RBUS data_10;
		profile_ich1_hist_profile_data_11_RBUS data_11;
		profile_ich1_hist_profile_data_12_RBUS data_12;
		profile_ich1_hist_profile_data_13_RBUS data_13;
		profile_ich1_hist_profile_data_14_RBUS data_14;
		profile_ich1_hist_profile_data_15_RBUS data_15;

		data_00.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_00_reg);
		data_01.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_01_reg);
		data_02.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_02_reg);
		data_03.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_03_reg);
		data_04.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_04_reg);
		data_05.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_05_reg);
		data_06.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_06_reg);
		data_07.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_07_reg);
		data_08.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_08_reg);
		data_09.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_09_reg);
		data_10.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_10_reg);
		data_11.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_11_reg);
		data_12.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_12_reg);
		data_13.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_13_reg);
		data_14.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_14_reg);
		data_15.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_15_reg);

		// Get Horizontal profile data for U.
		H_Block_U[0]   = data_00.ch1_pf_h00;
		H_Block_U[1]   = data_00.ch1_pf_h01;
		H_Block_U[2]   = data_01.ch1_pf_h02;
		H_Block_U[3]   = data_01.ch1_pf_h03;
		H_Block_U[4]   = data_02.ch1_pf_h04;
		H_Block_U[5]   = data_02.ch1_pf_h05;
		H_Block_U[6]   = data_03.ch1_pf_h06;
		H_Block_U[7]   = data_03.ch1_pf_h07;
		H_Block_U[8]   = data_04.ch1_pf_h08;
		H_Block_U[9]   = data_04.ch1_pf_h09;
		H_Block_U[10] = data_05.ch1_pf_h10;
		H_Block_U[11] = data_05.ch1_pf_h11;
		H_Block_U[12] = data_06.ch1_pf_h12;
		H_Block_U[13] = data_06.ch1_pf_h13;
		H_Block_U[14] = data_07.ch1_pf_h14;
		H_Block_U[15] = data_07.ch1_pf_h15;

		// Get Vertical profile data for U.
		V_Block_U[0]    = data_08.ch1_pf_v00;
		V_Block_U[1]    = data_08.ch1_pf_v01;
		V_Block_U[2]    = data_09.ch1_pf_v02;
		V_Block_U[3]    = data_09.ch1_pf_v03;
		V_Block_U[4]    = data_10.ch1_pf_v04;
		V_Block_U[5]    = data_10.ch1_pf_v05;
		V_Block_U[6]    = data_11.ch1_pf_v06;
		V_Block_U[7]    = data_11.ch1_pf_v07;
		V_Block_U[8]    = data_12.ch1_pf_v08;
		V_Block_U[9]    = data_12.ch1_pf_v09;
		V_Block_U[10] = data_13.ch1_pf_v10;
		V_Block_U[11] = data_13.ch1_pf_v11;
		V_Block_U[12] = data_14.ch1_pf_v12;
		V_Block_U[13] = data_14.ch1_pf_v13;
		V_Block_U[14] = data_15.ch1_pf_v14;
		V_Block_U[15] = data_15.ch1_pf_v15;

		//Enable profile
		hist_profile_ctrl_reg.ch1_pf_enable_profile = 0x1;
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_CTRL_reg, hist_profile_ctrl_reg.regValue);

		Profile1_U_Sum = 0;
		for( ii=0; ii<16; ii++ ) {
			Profile1_U_Sum += H_Block_U[ii];
		}
		//Profile1_U_Sum = Profile1_U_Sum << profsum_shift;

		#if Print_Profile_Data
		if (Profile_Print_Count%500 ==0)
		{
			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### H_Block_U[%d] =%d ### \n", jj, H_Block_U[jj]);
			}

			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### V_Block_U[%d] =%d ### \n", jj, V_Block_U[jj]);
			}

			ROSPrintf( "[ScalerVIP] ========Profile OK, Profile1_U_Sum =%d========\n", Profile1_U_Sum);

			Profile_Print_Count = 0;
		}

		Profile_Print_Count++;
		#endif

		return SUCCESS;
	}
	else
	{
		//ROSPrintf( "[ScalerVIP] ==========Profile NOT ready==========\n" );
		return FAILED;
	}

}


char scalerVIP_ReadProfile1_V(_clues* SmartPic_clue)
{
	profile_ich1_hist_profile_ctrl_RBUS hist_profile_ctrl_reg;

	unsigned char ii;
	#if Print_Profile_Data
	unsigned char jj;
	#endif

	unsigned short *H_Block_V = &(SmartPic_clue->Profile_Cal_INFO.H_Block_V[0]);
	unsigned short *V_Block_V = &(SmartPic_clue->Profile_Cal_INFO.V_Block_V[0]);

	hist_profile_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile_CTRL_reg);

	// H & V Shift right hnsr bits each line.
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:
		case _MODE_576I:
		case _MODE_480P:
		case _MODE_576P:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x2;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x1;
			break;
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1080I25:
		case _MODE_1080I30:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x3;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x2;
			break;
		case _MODE_1080P50:
		case _MODE_1080P60:
			hist_profile_ctrl_reg.ch1_pf_hnsr = 0x3;
			hist_profile_ctrl_reg.ch1_pf_vnsr = 0x3;
			break;
	}

	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_CTRL_reg, hist_profile_ctrl_reg.regValue);

	if ( hist_profile_ctrl_reg.ch1_pf_enable_profile == 0 ) //wait for finish flag
	{

		profile_ich1_hist_profile_data_00_RBUS data_00;
		profile_ich1_hist_profile_data_01_RBUS data_01;
		profile_ich1_hist_profile_data_02_RBUS data_02;
		profile_ich1_hist_profile_data_03_RBUS data_03;
		profile_ich1_hist_profile_data_04_RBUS data_04;
		profile_ich1_hist_profile_data_05_RBUS data_05;
		profile_ich1_hist_profile_data_06_RBUS data_06;
		profile_ich1_hist_profile_data_07_RBUS data_07;
		profile_ich1_hist_profile_data_08_RBUS data_08;
		profile_ich1_hist_profile_data_09_RBUS data_09;
		profile_ich1_hist_profile_data_10_RBUS data_10;
		profile_ich1_hist_profile_data_11_RBUS data_11;
		profile_ich1_hist_profile_data_12_RBUS data_12;
		profile_ich1_hist_profile_data_13_RBUS data_13;
		profile_ich1_hist_profile_data_14_RBUS data_14;
		profile_ich1_hist_profile_data_15_RBUS data_15;

		data_00.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_00_reg);
		data_01.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_01_reg);
		data_02.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_02_reg);
		data_03.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_03_reg);
		data_04.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_04_reg);
		data_05.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_05_reg);
		data_06.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_06_reg);
		data_07.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_07_reg);
		data_08.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_08_reg);
		data_09.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_09_reg);
		data_10.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_10_reg);
		data_11.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_11_reg);
		data_12.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_12_reg);
		data_13.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_13_reg);
		data_14.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_14_reg);
		data_15.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_15_reg);

		// Get Horizontal profile data for V.
		H_Block_V[0]  = data_00.ch1_pf_h00;
		H_Block_V[1]  = data_00.ch1_pf_h01;
		H_Block_V[2]  = data_01.ch1_pf_h02;
		H_Block_V[3]  = data_01.ch1_pf_h03;
		H_Block_V[4]  = data_02.ch1_pf_h04;
		H_Block_V[5]  = data_02.ch1_pf_h05;
		H_Block_V[6]  = data_03.ch1_pf_h06;
		H_Block_V[7]  = data_03.ch1_pf_h07;
		H_Block_V[8]  = data_04.ch1_pf_h08;
		H_Block_V[9]  = data_04.ch1_pf_h09;
		H_Block_V[10] = data_05.ch1_pf_h10;
		H_Block_V[11] = data_05.ch1_pf_h11;
		H_Block_V[12] = data_06.ch1_pf_h12;
		H_Block_V[13] = data_06.ch1_pf_h13;
		H_Block_V[14] = data_07.ch1_pf_h14;
		H_Block_V[15] = data_07.ch1_pf_h15;

		// Get Vertical profile data for V.
		V_Block_V[0]    = data_08.ch1_pf_v00;
		V_Block_V[1]    = data_08.ch1_pf_v01;
		V_Block_V[2]    = data_09.ch1_pf_v02;
		V_Block_V[3]    = data_09.ch1_pf_v03;
		V_Block_V[4]    = data_10.ch1_pf_v04;
		V_Block_V[5]    = data_10.ch1_pf_v05;
		V_Block_V[6]    = data_11.ch1_pf_v06;
		V_Block_V[7]    = data_11.ch1_pf_v07;
		V_Block_V[8]    = data_12.ch1_pf_v08;
		V_Block_V[9]    = data_12.ch1_pf_v09;
		V_Block_V[10]  = data_13.ch1_pf_v10;
		V_Block_V[11]  = data_13.ch1_pf_v11;
		V_Block_V[12]  = data_14.ch1_pf_v12;
		V_Block_V[13]  = data_14.ch1_pf_v13;
		V_Block_V[14]  = data_15.ch1_pf_v14;
		V_Block_V[15]  = data_15.ch1_pf_v15;

		//Enable profile
		hist_profile_ctrl_reg.ch1_pf_enable_profile = 0x1;
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_CTRL_reg, hist_profile_ctrl_reg.regValue);

		Profile1_V_Sum = 0;
		for( ii=0; ii<16; ii++ ) {
			Profile1_V_Sum += H_Block_V[ii];
		}
		//Profile1_V_Sum = Profile1_V_Sum << profsum_shift;

		#if Print_Profile_Data
		if (Profile_Print_Count%500 ==0)
		{
			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### H_Block_V[%d] =%d ### \n", jj, H_Block_V[jj]);
			}

			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### V_Block_V[%d] =%d ### \n", jj, V_Block_V[jj]);
			}

			ROSPrintf( "[ScalerVIP] ========Profile OK, Profile1_V_Sum =%d========\n", Profile1_V_Sum);

			Profile_Print_Count = 0;
		}

		Profile_Print_Count++;
		#endif

		return SUCCESS;
	}
	else
	{
		//ROSPrintf( "[ScalerVIP] ==========Profile NOT ready==========\n" );
		return FAILED;
	}

}

char scalerVIP_ReadProfile2_Y(_clues* SmartPic_clue)
{
	profile_ich1_hist_profile2_ctrl_RBUS hist_profile2_ctrl_reg;

	unsigned char ii;
	#if Print_Profile_Data
	unsigned char jj;
	#endif

	unsigned short *Profile2_H_Block_Y = &(SmartPic_clue->Profile_Cal_INFO.Profile2_H_Block_Y[0]);
	unsigned short *Profile2_V_Block_Y = &(SmartPic_clue->Profile_Cal_INFO.Profile2_V_Block_Y[0]);

	hist_profile2_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile2_CTRL_reg);

	// H & V Shift right hnsr bits each line.
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:
		case _MODE_576I:
		case _MODE_480P:
		case _MODE_576P:
			hist_profile2_ctrl_reg.ch1_pf2_hnsr = 0x2;
			hist_profile2_ctrl_reg.ch1_pf2_vnsr = 0x1;
			break;
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1080I25:
		case _MODE_1080I30:
			hist_profile2_ctrl_reg.ch1_pf2_hnsr = 0x3;
			hist_profile2_ctrl_reg.ch1_pf2_vnsr = 0x2;
			break;
		case _MODE_1080P50:
		case _MODE_1080P60:
			hist_profile2_ctrl_reg.ch1_pf2_hnsr = 0x3;
			hist_profile2_ctrl_reg.ch1_pf2_vnsr = 0x3;
			break;
	}

	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_CTRL_reg, hist_profile2_ctrl_reg.regValue);

	if ( hist_profile2_ctrl_reg.ch1_pf2_enable_profile == 0 ) //wait for finish flag
	{

		profile_ich1_hist_profile_data_16_RBUS data_16;
		profile_ich1_hist_profile_data_17_RBUS data_17;
		profile_ich1_hist_profile_data_18_RBUS data_18;
		profile_ich1_hist_profile_data_19_RBUS data_19;
		profile_ich1_hist_profile_data_20_RBUS data_20;
		profile_ich1_hist_profile_data_21_RBUS data_21;
		profile_ich1_hist_profile_data_22_RBUS data_22;
		profile_ich1_hist_profile_data_23_RBUS data_23;
		profile_ich1_hist_profile_data_24_RBUS data_24;
		profile_ich1_hist_profile_data_25_RBUS data_25;
		profile_ich1_hist_profile_data_26_RBUS data_26;
		profile_ich1_hist_profile_data_27_RBUS data_27;
		profile_ich1_hist_profile_data_28_RBUS data_28;
		profile_ich1_hist_profile_data_29_RBUS data_29;
		profile_ich1_hist_profile_data_30_RBUS data_30;
		profile_ich1_hist_profile_data_31_RBUS data_31;

		data_16.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_16_reg);
		data_17.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_17_reg);
		data_18.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_18_reg);
		data_19.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_19_reg);
		data_20.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_20_reg);
		data_21.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_21_reg);
		data_22.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_22_reg);
		data_23.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_23_reg);
		data_24.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_24_reg);
		data_25.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_25_reg);
		data_26.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_26_reg);
		data_27.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_27_reg);
		data_28.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_28_reg);
		data_29.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_29_reg);
		data_30.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_30_reg);
		data_31.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_31_reg);

		Profile2_H_Block_Y[0]   = data_16.ch1_pf_h00_right;
		Profile2_H_Block_Y[1]   = data_16.ch1_pf_h01_right;
		Profile2_H_Block_Y[2]   = data_17.ch1_pf_h02_right;
		Profile2_H_Block_Y[3]   = data_17.ch1_pf_h03_right;
		Profile2_H_Block_Y[4]   = data_18.ch1_pf_h04_right;
		Profile2_H_Block_Y[5]   = data_18.ch1_pf_h05_right;
		Profile2_H_Block_Y[6]   = data_19.ch1_pf_h06_right;
		Profile2_H_Block_Y[7]   = data_19.ch1_pf_h07_right;
		Profile2_H_Block_Y[8]   = data_20.ch1_pf_h08_right;
		Profile2_H_Block_Y[9]   = data_20.ch1_pf_h09_right;
		Profile2_H_Block_Y[10] = data_21.ch1_pf_h10_right;
		Profile2_H_Block_Y[11] = data_21.ch1_pf_h11_right;
		Profile2_H_Block_Y[12] = data_22.ch1_pf_h12_right;
		Profile2_H_Block_Y[13] = data_22.ch1_pf_h13_right;
		Profile2_H_Block_Y[14] = data_23.ch1_pf_h14_right;
		Profile2_H_Block_Y[15] = data_23.ch1_pf_h15_right;

		Profile2_V_Block_Y[0]   = data_24.ch1_pf_v00_right;
		Profile2_V_Block_Y[1]   = data_24.ch1_pf_v01_right;
		Profile2_V_Block_Y[2]   = data_25.ch1_pf_v02_right;
		Profile2_V_Block_Y[3]   = data_25.ch1_pf_v03_right;
		Profile2_V_Block_Y[4]   = data_26.ch1_pf_v04_right;
		Profile2_V_Block_Y[5]   = data_26.ch1_pf_v05_right;
		Profile2_V_Block_Y[6]   = data_27.ch1_pf_v06_right;
		Profile2_V_Block_Y[7]   = data_27.ch1_pf_v07_right;
		Profile2_V_Block_Y[8]   = data_28.ch1_pf_v08_right;
		Profile2_V_Block_Y[9]   = data_28.ch1_pf_v09_right;
		Profile2_V_Block_Y[10] = data_29.ch1_pf_v10_right;
		Profile2_V_Block_Y[11] = data_29.ch1_pf_v11_right;
		Profile2_V_Block_Y[12] = data_30.ch1_pf_v12_right;
		Profile2_V_Block_Y[13] = data_30.ch1_pf_v13_right;
		Profile2_V_Block_Y[14] = data_31.ch1_pf_v14_right;
		Profile2_V_Block_Y[15] = data_31.ch1_pf_v15_right;

		//Enable profile
		hist_profile2_ctrl_reg.ch1_pf2_enable_profile = 0x1;
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile_CTRL_reg, hist_profile2_ctrl_reg.regValue);

		Profile2_Y_Sum = 0;
		for( ii=0; ii<16; ii++ ) {
			Profile2_Y_Sum += Profile2_H_Block_Y[ii];
		}
		//Profile2_Y_Sum = Profile2_Y_Sum << profsum_shift;

		#if Print_Profile_Data
		if (Profile_Print_Count%600 ==0)
		{
			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### Profile2_H_Block_Y[%d] =%d ### \n", jj, Profile2_H_Block_Y[jj]);
			}

			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### Profile2_V_Block_Y[%d] =%d ### \n", jj, Profile2_V_Block_Y[jj]);
			}

			ROSPrintf( "[ScalerVIP] ========Profile OK, Profile2_Y_Sum =%d========\n", Profile2_Y_Sum);

			Profile_Print_Count = 0;
		}

		Profile_Print_Count++;
		#endif

		return SUCCESS;
	}
	else
	{
		//ROSPrintf( "[ScalerVIP] ==========Profile NOT ready==========\n" );
		return FAILED;
	}

}

char scalerVIP_ReadProfile2_U(_clues* SmartPic_clue)
{
	profile_ich1_hist_profile2_ctrl_RBUS hist_profile2_ctrl_reg;

	unsigned char ii;
	#if Print_Profile_Data
	unsigned char jj;
	#endif

	unsigned short *Profile2_H_Block_U = &(SmartPic_clue->Profile_Cal_INFO.Profile2_H_Block_U[0]);
	unsigned short *Profile2_V_Block_U = &(SmartPic_clue->Profile_Cal_INFO.Profile2_V_Block_U[0]);

	hist_profile2_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile2_CTRL_reg);

	// H & V Shift right hnsr bits each line.
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:
		case _MODE_576I:
		case _MODE_480P:
		case _MODE_576P:
			hist_profile2_ctrl_reg.ch1_pf2_hnsr = 0x2;
			hist_profile2_ctrl_reg.ch1_pf2_vnsr = 0x1;
			break;
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1080I25:
		case _MODE_1080I30:
			hist_profile2_ctrl_reg.ch1_pf2_hnsr = 0x3;
			hist_profile2_ctrl_reg.ch1_pf2_vnsr = 0x2;
			break;
		case _MODE_1080P50:
		case _MODE_1080P60:
			hist_profile2_ctrl_reg.ch1_pf2_hnsr = 0x3;
			hist_profile2_ctrl_reg.ch1_pf2_vnsr = 0x3;
			break;
	}

	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile2_CTRL_reg, hist_profile2_ctrl_reg.regValue);

	if ( hist_profile2_ctrl_reg.ch1_pf2_enable_profile == 0 ) //wait for finish flag
	{

		profile_ich1_hist_profile_data_16_RBUS data_16;
		profile_ich1_hist_profile_data_17_RBUS data_17;
		profile_ich1_hist_profile_data_18_RBUS data_18;
		profile_ich1_hist_profile_data_19_RBUS data_19;
		profile_ich1_hist_profile_data_20_RBUS data_20;
		profile_ich1_hist_profile_data_21_RBUS data_21;
		profile_ich1_hist_profile_data_22_RBUS data_22;
		profile_ich1_hist_profile_data_23_RBUS data_23;
		profile_ich1_hist_profile_data_24_RBUS data_24;
		profile_ich1_hist_profile_data_25_RBUS data_25;
		profile_ich1_hist_profile_data_26_RBUS data_26;
		profile_ich1_hist_profile_data_27_RBUS data_27;
		profile_ich1_hist_profile_data_28_RBUS data_28;
		profile_ich1_hist_profile_data_29_RBUS data_29;
		profile_ich1_hist_profile_data_30_RBUS data_30;
		profile_ich1_hist_profile_data_31_RBUS data_31;

		data_16.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_16_reg);
		data_17.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_17_reg);
		data_18.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_18_reg);
		data_19.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_19_reg);
		data_20.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_20_reg);
		data_21.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_21_reg);
		data_22.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_22_reg);
		data_23.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_23_reg);
		data_24.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_24_reg);
		data_25.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_25_reg);
		data_26.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_26_reg);
		data_27.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_27_reg);
		data_28.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_28_reg);
		data_29.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_29_reg);
		data_30.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_30_reg);
		data_31.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_31_reg);

		// Get Horizontal profile data for U.
		Profile2_H_Block_U[0]   = data_16.ch1_pf_h00_right;
		Profile2_H_Block_U[1]   = data_16.ch1_pf_h01_right;
		Profile2_H_Block_U[2]   = data_17.ch1_pf_h02_right;
		Profile2_H_Block_U[3]   = data_17.ch1_pf_h03_right;
		Profile2_H_Block_U[4]   = data_18.ch1_pf_h04_right;
		Profile2_H_Block_U[5]   = data_18.ch1_pf_h05_right;
		Profile2_H_Block_U[6]   = data_19.ch1_pf_h06_right;
		Profile2_H_Block_U[7]   = data_19.ch1_pf_h07_right;
		Profile2_H_Block_U[8]   = data_20.ch1_pf_h08_right;
		Profile2_H_Block_U[9]   = data_20.ch1_pf_h09_right;
		Profile2_H_Block_U[10] = data_21.ch1_pf_h10_right;
		Profile2_H_Block_U[11] = data_21.ch1_pf_h11_right;
		Profile2_H_Block_U[12] = data_22.ch1_pf_h12_right;
		Profile2_H_Block_U[13] = data_22.ch1_pf_h13_right;
		Profile2_H_Block_U[14] = data_23.ch1_pf_h14_right;
		Profile2_H_Block_U[15] = data_23.ch1_pf_h15_right;

		// Get Vertical profile data for U.
		Profile2_V_Block_U[0]   = data_24.ch1_pf_v00_right;
		Profile2_V_Block_U[1]   = data_24.ch1_pf_v01_right;
		Profile2_V_Block_U[2]   = data_25.ch1_pf_v02_right;
		Profile2_V_Block_U[3]   = data_25.ch1_pf_v03_right;
		Profile2_V_Block_U[4]   = data_26.ch1_pf_v04_right;
		Profile2_V_Block_U[5]   = data_26.ch1_pf_v05_right;
		Profile2_V_Block_U[6]   = data_27.ch1_pf_v06_right;
		Profile2_V_Block_U[7]   = data_27.ch1_pf_v07_right;
		Profile2_V_Block_U[8]   = data_28.ch1_pf_v08_right;
		Profile2_V_Block_U[9]   = data_28.ch1_pf_v09_right;
		Profile2_V_Block_U[10] = data_29.ch1_pf_v10_right;
		Profile2_V_Block_U[11] = data_29.ch1_pf_v11_right;
		Profile2_V_Block_U[12] = data_30.ch1_pf_v12_right;
		Profile2_V_Block_U[13] = data_30.ch1_pf_v13_right;
		Profile2_V_Block_U[14] = data_31.ch1_pf_v14_right;
		Profile2_V_Block_U[15] = data_31.ch1_pf_v15_right;

		//Enable profile
		hist_profile2_ctrl_reg.ch1_pf2_enable_profile = 0x1;
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile2_CTRL_reg, hist_profile2_ctrl_reg.regValue);

		Profile2_U_Sum = 0;
		for( ii=0; ii<16; ii++ ) {
			Profile2_U_Sum += Profile2_H_Block_U[ii];
		}
		//Profile2_U_Sum = Profile2_U_Sum << profsum_shift;

		#if Print_Profile_Data
		if (Profile_Print_Count%600 ==0)
		{
			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### Profile2_H_Block_U[%d] =%d ### \n", jj, Profile2_H_Block_U[jj]);
			}

			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### Profile2_V_Block_U[%d] =%d ### \n", jj, Profile2_V_Block_U[jj]);
			}

			ROSPrintf( "[ScalerVIP] ========Profile OK, Profile2_U_Sum =%d========\n", Profile2_U_Sum);

			Profile_Print_Count = 0;
		}

		Profile_Print_Count++;
		#endif

		return SUCCESS;
	}
	else
	{
		//ROSPrintf( "[ScalerVIP] ==========Profile NOT ready==========\n" );
		return FAILED;
	}

}


char scalerVIP_ReadProfile2_V(_clues* SmartPic_clue)
{
	profile_ich1_hist_profile2_ctrl_RBUS hist_profile2_ctrl_reg;

	unsigned char ii;
	#if Print_Profile_Data
	unsigned char jj;
	#endif

	unsigned short *Profile2_H_Block_V = &(SmartPic_clue->Profile_Cal_INFO.Profile2_H_Block_V[0]);
	unsigned short *Profile2_V_Block_V = &(SmartPic_clue->Profile_Cal_INFO.Profile2_V_Block_V[0]);

	hist_profile2_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile2_CTRL_reg);

	// H & V Shift right hnsr bits each line.
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:
		case _MODE_576I:
		case _MODE_480P:
		case _MODE_576P:
			hist_profile2_ctrl_reg.ch1_pf2_hnsr = 0x2;
			hist_profile2_ctrl_reg.ch1_pf2_vnsr = 0x1;
			break;
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1080I25:
		case _MODE_1080I30:
			hist_profile2_ctrl_reg.ch1_pf2_hnsr = 0x3;
			hist_profile2_ctrl_reg.ch1_pf2_vnsr = 0x2;
			break;
		case _MODE_1080P50:
		case _MODE_1080P60:
			hist_profile2_ctrl_reg.ch1_pf2_hnsr = 0x3;
			hist_profile2_ctrl_reg.ch1_pf2_vnsr = 0x3;
			break;
	}

	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile2_CTRL_reg, hist_profile2_ctrl_reg.regValue);

	if ( hist_profile2_ctrl_reg.ch1_pf2_enable_profile == 0 ) //wait for finish flag
	{

		profile_ich1_hist_profile_data_16_RBUS data_16;
		profile_ich1_hist_profile_data_17_RBUS data_17;
		profile_ich1_hist_profile_data_18_RBUS data_18;
		profile_ich1_hist_profile_data_19_RBUS data_19;
		profile_ich1_hist_profile_data_20_RBUS data_20;
		profile_ich1_hist_profile_data_21_RBUS data_21;
		profile_ich1_hist_profile_data_22_RBUS data_22;
		profile_ich1_hist_profile_data_23_RBUS data_23;
		profile_ich1_hist_profile_data_24_RBUS data_24;
		profile_ich1_hist_profile_data_25_RBUS data_25;
		profile_ich1_hist_profile_data_26_RBUS data_26;
		profile_ich1_hist_profile_data_27_RBUS data_27;
		profile_ich1_hist_profile_data_28_RBUS data_28;
		profile_ich1_hist_profile_data_29_RBUS data_29;
		profile_ich1_hist_profile_data_30_RBUS data_30;
		profile_ich1_hist_profile_data_31_RBUS data_31;

		data_16.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_16_reg);
		data_17.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_17_reg);
		data_18.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_18_reg);
		data_19.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_19_reg);
		data_20.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_20_reg);
		data_21.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_21_reg);
		data_22.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_22_reg);
		data_23.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_23_reg);
		data_24.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_24_reg);
		data_25.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_25_reg);
		data_26.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_26_reg);
		data_27.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_27_reg);
		data_28.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_28_reg);
		data_29.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_29_reg);
		data_30.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_30_reg);
		data_31.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_Data_31_reg);

		// Get Horizontal profile data for U.
		Profile2_H_Block_V[0]   = data_16.ch1_pf_h00_right;
		Profile2_H_Block_V[1]   = data_16.ch1_pf_h01_right;
		Profile2_H_Block_V[2]   = data_17.ch1_pf_h02_right;
		Profile2_H_Block_V[3]   = data_17.ch1_pf_h03_right;
		Profile2_H_Block_V[4]   = data_18.ch1_pf_h04_right;
		Profile2_H_Block_V[5]   = data_18.ch1_pf_h05_right;
		Profile2_H_Block_V[6]   = data_19.ch1_pf_h06_right;
		Profile2_H_Block_V[7]   = data_19.ch1_pf_h07_right;
		Profile2_H_Block_V[8]   = data_20.ch1_pf_h08_right;
		Profile2_H_Block_V[9]   = data_20.ch1_pf_h09_right;
		Profile2_H_Block_V[10] = data_21.ch1_pf_h10_right;
		Profile2_H_Block_V[11] = data_21.ch1_pf_h11_right;
		Profile2_H_Block_V[12] = data_22.ch1_pf_h12_right;
		Profile2_H_Block_V[13] = data_22.ch1_pf_h13_right;
		Profile2_H_Block_V[14] = data_23.ch1_pf_h14_right;
		Profile2_H_Block_V[15] = data_23.ch1_pf_h15_right;

		// Get Vertical profile data for U.
		Profile2_V_Block_V[0]   = data_24.ch1_pf_v00_right;
		Profile2_V_Block_V[1]   = data_24.ch1_pf_v01_right;
		Profile2_V_Block_V[2]   = data_25.ch1_pf_v02_right;
		Profile2_V_Block_V[3]   = data_25.ch1_pf_v03_right;
		Profile2_V_Block_V[4]   = data_26.ch1_pf_v04_right;
		Profile2_V_Block_V[5]   = data_26.ch1_pf_v05_right;
		Profile2_V_Block_V[6]   = data_27.ch1_pf_v06_right;
		Profile2_V_Block_V[7]   = data_27.ch1_pf_v07_right;
		Profile2_V_Block_V[8]   = data_28.ch1_pf_v08_right;
		Profile2_V_Block_V[9]   = data_28.ch1_pf_v09_right;
		Profile2_V_Block_V[10] = data_29.ch1_pf_v10_right;
		Profile2_V_Block_V[11] = data_29.ch1_pf_v11_right;
		Profile2_V_Block_V[12] = data_30.ch1_pf_v12_right;
		Profile2_V_Block_V[13] = data_30.ch1_pf_v13_right;
		Profile2_V_Block_V[14] = data_31.ch1_pf_v14_right;
		Profile2_V_Block_V[15] = data_31.ch1_pf_v15_right;

		//Enable profile
		hist_profile2_ctrl_reg.ch1_pf2_enable_profile = 0x1;
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile2_CTRL_reg, hist_profile2_ctrl_reg.regValue);

		Profile2_V_Sum = 0;
		for( ii=0; ii<16; ii++ ) {
			Profile2_V_Sum += Profile2_H_Block_V[ii];
		}
		//Profile2_V_Sum = Profile2_V_Sum << profsum_shift;

		#if Print_Profile_Data
		if (Profile_Print_Count%600 ==0)
		{
			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### Profile2_H_Block_V[%d] =%d ### \n", jj, Profile2_H_Block_V[jj]);
			}

			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### Profile2_V_Block_V[%d] =%d ### \n", jj, Profile2_V_Block_V[jj]);
			}

			ROSPrintf( "[ScalerVIP] ========Profile OK, Profile2_V_Sum =%d========\n", Profile2_V_Sum );

			Profile_Print_Count = 0;
		}

		Profile_Print_Count++;
		#endif

		return SUCCESS;
	}
	else
	{
		//ROSPrintf( "[ScalerVIP] ==========Profile NOT ready==========\n" );
		return FAILED;
	}

}

char scalerVIP_ReadProfile3_Y(_clues* SmartPic_clue)
{
	profile_ich1_hist_profile3_ctrl_RBUS hist_profile3_ctrl_reg;

	unsigned char ii;
	#if Print_Profile_Data
	unsigned char jj;
	#endif

	unsigned short *Profile3_H_Block_Y = &(SmartPic_clue->Profile_Cal_INFO.Profile3_H_Block_Y[0]);
	unsigned short *Profile3_V_Block_Y = &(SmartPic_clue->Profile_Cal_INFO.Profile3_V_Block_Y[0]);

	hist_profile3_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile3_CTRL_reg);

	// H & V Shift right hnsr bits each line.
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:
		case _MODE_576I:
		case _MODE_480P:
		case _MODE_576P:
			hist_profile3_ctrl_reg.ch1_pf3_hnsr = 0x2;
			hist_profile3_ctrl_reg.ch1_pf3_vnsr = 0x1;
			break;
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1080I25:
		case _MODE_1080I30:
			hist_profile3_ctrl_reg.ch1_pf3_hnsr = 0x3;
			hist_profile3_ctrl_reg.ch1_pf3_vnsr = 0x2;
			break;
		case _MODE_1080P50:
		case _MODE_1080P60:
			hist_profile3_ctrl_reg.ch1_pf3_hnsr = 0x3;
			hist_profile3_ctrl_reg.ch1_pf3_vnsr = 0x3;
			break;
	}

	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile3_CTRL_reg, hist_profile3_ctrl_reg.regValue);

	if ( hist_profile3_ctrl_reg.ch1_pf3_enable_profile == 0 ) //wait for finish flag
	{

		profile_ich1_hist_profile3_data_00_RBUS data_00;
		profile_ich1_hist_profile3_data_01_RBUS data_01;
		profile_ich1_hist_profile3_data_02_RBUS data_02;
		profile_ich1_hist_profile3_data_03_RBUS data_03;
		profile_ich1_hist_profile3_data_04_RBUS data_04;
		profile_ich1_hist_profile3_data_05_RBUS data_05;
		profile_ich1_hist_profile3_data_06_RBUS data_06;
		profile_ich1_hist_profile3_data_07_RBUS data_07;
		profile_ich1_hist_profile3_data_08_RBUS data_08;
		profile_ich1_hist_profile3_data_09_RBUS data_09;
		profile_ich1_hist_profile3_data_10_RBUS data_10;
		profile_ich1_hist_profile3_data_11_RBUS data_11;
		profile_ich1_hist_profile3_data_12_RBUS data_12;
		profile_ich1_hist_profile3_data_13_RBUS data_13;
		profile_ich1_hist_profile3_data_14_RBUS data_14;
		profile_ich1_hist_profile3_data_15_RBUS data_15;

		data_00.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_00_reg);
		data_01.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_01_reg);
		data_02.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_02_reg);
		data_03.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_03_reg);
		data_04.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_04_reg);
		data_05.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_05_reg);
		data_06.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_06_reg);
		data_07.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_07_reg);
		data_08.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_08_reg);
		data_09.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_09_reg);
		data_10.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_10_reg);
		data_11.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_11_reg);
		data_12.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_12_reg);
		data_13.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_13_reg);
		data_14.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_14_reg);
		data_15.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_15_reg);

		// Get Horizontal profile data for V.
		Profile3_H_Block_Y[0]   = data_00.ch1_pf3_h00;
		Profile3_H_Block_Y[1]   = data_00.ch1_pf3_h01;
		Profile3_H_Block_Y[2]   = data_01.ch1_pf3_h02;
		Profile3_H_Block_Y[3]   = data_01.ch1_pf3_h03;
		Profile3_H_Block_Y[4]   = data_02.ch1_pf3_h04;
		Profile3_H_Block_Y[5]   = data_02.ch1_pf3_h05;
		Profile3_H_Block_Y[6]   = data_03.ch1_pf3_h06;
		Profile3_H_Block_Y[7]   = data_03.ch1_pf3_h07;
		Profile3_H_Block_Y[8]   = data_04.ch1_pf3_h08;
		Profile3_H_Block_Y[9]   = data_04.ch1_pf3_h09;
		Profile3_H_Block_Y[10] = data_05.ch1_pf3_h10;
		Profile3_H_Block_Y[11] = data_05.ch1_pf3_h11;
		Profile3_H_Block_Y[12] = data_06.ch1_pf3_h12;
		Profile3_H_Block_Y[13] = data_06.ch1_pf3_h13;
		Profile3_H_Block_Y[14] = data_07.ch1_pf3_h14;
		Profile3_H_Block_Y[15] = data_07.ch1_pf3_h15;

		// Get Vertical profile data for V.
		Profile3_V_Block_Y[0]   = data_08.ch1_pf3_v00;
		Profile3_V_Block_Y[1]   = data_08.ch1_pf3_v01;
		Profile3_V_Block_Y[2]   = data_09.ch1_pf3_v02;
		Profile3_V_Block_Y[3]   = data_09.ch1_pf3_v03;
		Profile3_V_Block_Y[4]   = data_10.ch1_pf3_v04;
		Profile3_V_Block_Y[5]   = data_10.ch1_pf3_v05;
		Profile3_V_Block_Y[6]   = data_11.ch1_pf3_v06;
		Profile3_V_Block_Y[7]   = data_11.ch1_pf3_v07;
		Profile3_V_Block_Y[8]   = data_12.ch1_pf3_v08;
		Profile3_V_Block_Y[9]   = data_12.ch1_pf3_v09;
		Profile3_V_Block_Y[10] = data_13.ch1_pf3_v10;
		Profile3_V_Block_Y[11] = data_13.ch1_pf3_v11;
		Profile3_V_Block_Y[12] = data_14.ch1_pf3_v12;
		Profile3_V_Block_Y[13] = data_14.ch1_pf3_v13;
		Profile3_V_Block_Y[14] = data_15.ch1_pf3_v14;
		Profile3_V_Block_Y[15] = data_15.ch1_pf3_v15;

		//Enable profile
		hist_profile3_ctrl_reg.ch1_pf3_enable_profile = 0x1;
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile3_CTRL_reg, hist_profile3_ctrl_reg.regValue);

		Profile3_Y_Sum = 0;
		for( ii=0; ii<16; ii++ ) {
			Profile3_Y_Sum += Profile3_H_Block_Y[ii];
		}
		//Profile3_Y_Sum = Profile3_Y_Sum << profsum_shift;

		#if Print_Profile_Data
		if (Profile_Print_Count%700 ==0)
		{
			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### Profile3_H_Block_Y[%d] =%d ### \n", jj, Profile3_H_Block_Y[jj]);
			}

			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### Profile3_V_Block_Y[%d] =%d ### \n", jj, Profile3_V_Block_Y[jj]);
			}

			ROSPrintf( "[ScalerVIP] ========Profile OK, Profile3_Y_Sum =%d========\n", Profile3_Y_Sum);

			Profile_Print_Count = 0;
		}

		Profile_Print_Count++;
		#endif

		return SUCCESS;
	}
	else
	{
		//ROSPrintf( "[ScalerVIP] ==========Profile NOT ready==========\n" );
		return FAILED;
	}

}

char scalerVIP_ReadProfile3_U(_clues* SmartPic_clue)
{
	profile_ich1_hist_profile3_ctrl_RBUS hist_profile3_ctrl_reg;

	unsigned char ii;
	#if Print_Profile_Data
	unsigned char jj;
	#endif

	unsigned short *Profile3_H_Block_U = &(SmartPic_clue->Profile_Cal_INFO.Profile3_H_Block_U[0]);
	unsigned short *Profile3_V_Block_U = &(SmartPic_clue->Profile_Cal_INFO.Profile3_V_Block_U[0]);

	hist_profile3_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile3_CTRL_reg);

	// H & V Shift right hnsr bits each line.
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:
		case _MODE_576I:
		case _MODE_480P:
		case _MODE_576P:
			hist_profile3_ctrl_reg.ch1_pf3_hnsr = 0x2;
			hist_profile3_ctrl_reg.ch1_pf3_vnsr = 0x1;
			break;
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1080I25:
		case _MODE_1080I30:
			hist_profile3_ctrl_reg.ch1_pf3_hnsr = 0x3;
			hist_profile3_ctrl_reg.ch1_pf3_vnsr = 0x2;
			break;
		case _MODE_1080P50:
		case _MODE_1080P60:
			hist_profile3_ctrl_reg.ch1_pf3_hnsr = 0x3;
			hist_profile3_ctrl_reg.ch1_pf3_vnsr = 0x3;
			break;
	}

	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile3_CTRL_reg, hist_profile3_ctrl_reg.regValue);

	if ( hist_profile3_ctrl_reg.ch1_pf3_enable_profile == 0 ) //wait for finish flag
	{

		profile_ich1_hist_profile3_data_00_RBUS data_00;
		profile_ich1_hist_profile3_data_01_RBUS data_01;
		profile_ich1_hist_profile3_data_02_RBUS data_02;
		profile_ich1_hist_profile3_data_03_RBUS data_03;
		profile_ich1_hist_profile3_data_04_RBUS data_04;
		profile_ich1_hist_profile3_data_05_RBUS data_05;
		profile_ich1_hist_profile3_data_06_RBUS data_06;
		profile_ich1_hist_profile3_data_07_RBUS data_07;
		profile_ich1_hist_profile3_data_08_RBUS data_08;
		profile_ich1_hist_profile3_data_09_RBUS data_09;
		profile_ich1_hist_profile3_data_10_RBUS data_10;
		profile_ich1_hist_profile3_data_11_RBUS data_11;
		profile_ich1_hist_profile3_data_12_RBUS data_12;
		profile_ich1_hist_profile3_data_13_RBUS data_13;
		profile_ich1_hist_profile3_data_14_RBUS data_14;
		profile_ich1_hist_profile3_data_15_RBUS data_15;

		data_00.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_00_reg);
		data_01.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_01_reg);
		data_02.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_02_reg);
		data_03.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_03_reg);
		data_04.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_04_reg);
		data_05.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_05_reg);
		data_06.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_06_reg);
		data_07.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_07_reg);
		data_08.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_08_reg);
		data_09.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_09_reg);
		data_10.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_10_reg);
		data_11.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_11_reg);
		data_12.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_12_reg);
		data_13.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_13_reg);
		data_14.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_14_reg);
		data_15.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_15_reg);

		// Get Horizontal profile data for U.
		Profile3_H_Block_U[0]   = data_00.ch1_pf3_h00;
		Profile3_H_Block_U[1]   = data_00.ch1_pf3_h01;
		Profile3_H_Block_U[2]   = data_01.ch1_pf3_h02;
		Profile3_H_Block_U[3]   = data_01.ch1_pf3_h03;
		Profile3_H_Block_U[4]   = data_02.ch1_pf3_h04;
		Profile3_H_Block_U[5]   = data_02.ch1_pf3_h05;
		Profile3_H_Block_U[6]   = data_03.ch1_pf3_h06;
		Profile3_H_Block_U[7]   = data_03.ch1_pf3_h07;
		Profile3_H_Block_U[8]   = data_04.ch1_pf3_h08;
		Profile3_H_Block_U[9]   = data_04.ch1_pf3_h09;
		Profile3_H_Block_U[10] = data_05.ch1_pf3_h10;
		Profile3_H_Block_U[11] = data_05.ch1_pf3_h11;
		Profile3_H_Block_U[12] = data_06.ch1_pf3_h12;
		Profile3_H_Block_U[13] = data_06.ch1_pf3_h13;
		Profile3_H_Block_U[14] = data_07.ch1_pf3_h14;
		Profile3_H_Block_U[15] = data_07.ch1_pf3_h15;

		// Get Vertical profile data for U.
		Profile3_V_Block_U[0]   = data_08.ch1_pf3_v00;
		Profile3_V_Block_U[1]   = data_08.ch1_pf3_v01;
		Profile3_V_Block_U[2]   = data_09.ch1_pf3_v02;
		Profile3_V_Block_U[3]   = data_09.ch1_pf3_v03;
		Profile3_V_Block_U[4]   = data_10.ch1_pf3_v04;
		Profile3_V_Block_U[5]   = data_10.ch1_pf3_v05;
		Profile3_V_Block_U[6]   = data_11.ch1_pf3_v06;
		Profile3_V_Block_U[7]   = data_11.ch1_pf3_v07;
		Profile3_V_Block_U[8]   = data_12.ch1_pf3_v08;
		Profile3_V_Block_U[9]   = data_12.ch1_pf3_v09;
		Profile3_V_Block_U[10] = data_13.ch1_pf3_v10;
		Profile3_V_Block_U[11] = data_13.ch1_pf3_v11;
		Profile3_V_Block_U[12] = data_14.ch1_pf3_v12;
		Profile3_V_Block_U[13] = data_14.ch1_pf3_v13;
		Profile3_V_Block_U[14] = data_15.ch1_pf3_v14;
		Profile3_V_Block_U[15] = data_15.ch1_pf3_v15;

		//Enable profile
		hist_profile3_ctrl_reg.ch1_pf3_enable_profile = 0x1;
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile3_CTRL_reg, hist_profile3_ctrl_reg.regValue);

		Profile3_U_Sum = 0;
		for( ii=0; ii<16; ii++ ) {
			Profile3_U_Sum += Profile3_H_Block_U[ii];
		}
		//Profile3_U_Sum = Profile3_U_Sum << profsum_shift;

		#if Print_Profile_Data
		if (Profile_Print_Count%700 ==0)
		{
			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### Profile3_H_Block_U[%d] =%d ### \n", jj, Profile3_H_Block_U[jj]);
			}

			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### Profile3_V_Block_U[%d] =%d ### \n", jj, Profile3_V_Block_U[jj]);
			}

			ROSPrintf( "[ScalerVIP] ========Profile OK, Profile3_U_Sum =%d========\n", Profile3_U_Sum);

			Profile_Print_Count = 0;
		}

		Profile_Print_Count++;
		#endif

		return SUCCESS;
	}
	else
	{
		//ROSPrintf( "[ScalerVIP] ==========Profile NOT ready==========\n" );
		return FAILED;
	}

}


char scalerVIP_ReadProfile3_V(_clues* SmartPic_clue)
{
	profile_ich1_hist_profile3_ctrl_RBUS hist_profile3_ctrl_reg;

	unsigned char ii;
	#if Print_Profile_Data
	unsigned char jj;
	#endif

	unsigned short *Profile3_H_Block_V = &(SmartPic_clue->Profile_Cal_INFO.Profile3_H_Block_V[0]);
	unsigned short *Profile3_V_Block_V = &(SmartPic_clue->Profile_Cal_INFO.Profile3_V_Block_V[0]);

	hist_profile3_ctrl_reg.regValue = scaler_rtd_inl(PROFILE_ICH1_Hist_Profile3_CTRL_reg);

	// H & V Shift right hnsr bits each line.
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_480I:
		case _MODE_576I:
		case _MODE_480P:
		case _MODE_576P:
			hist_profile3_ctrl_reg.ch1_pf3_hnsr = 0x2;
			hist_profile3_ctrl_reg.ch1_pf3_vnsr = 0x1;
			break;
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1080I25:
		case _MODE_1080I30:
			hist_profile3_ctrl_reg.ch1_pf3_hnsr = 0x3;
			hist_profile3_ctrl_reg.ch1_pf3_vnsr = 0x2;
			break;
		case _MODE_1080P50:
		case _MODE_1080P60:
			hist_profile3_ctrl_reg.ch1_pf3_hnsr = 0x3;
			hist_profile3_ctrl_reg.ch1_pf3_vnsr = 0x3;
			break;
	}

	scaler_rtd_outl(PROFILE_ICH1_Hist_Profile3_CTRL_reg, hist_profile3_ctrl_reg.regValue);

	if ( hist_profile3_ctrl_reg.ch1_pf3_enable_profile == 0 ) //wait for finish flag
	{

		profile_ich1_hist_profile3_data_00_RBUS data_00;
		profile_ich1_hist_profile3_data_01_RBUS data_01;
		profile_ich1_hist_profile3_data_02_RBUS data_02;
		profile_ich1_hist_profile3_data_03_RBUS data_03;
		profile_ich1_hist_profile3_data_04_RBUS data_04;
		profile_ich1_hist_profile3_data_05_RBUS data_05;
		profile_ich1_hist_profile3_data_06_RBUS data_06;
		profile_ich1_hist_profile3_data_07_RBUS data_07;
		profile_ich1_hist_profile3_data_08_RBUS data_08;
		profile_ich1_hist_profile3_data_09_RBUS data_09;
		profile_ich1_hist_profile3_data_10_RBUS data_10;
		profile_ich1_hist_profile3_data_11_RBUS data_11;
		profile_ich1_hist_profile3_data_12_RBUS data_12;
		profile_ich1_hist_profile3_data_13_RBUS data_13;
		profile_ich1_hist_profile3_data_14_RBUS data_14;
		profile_ich1_hist_profile3_data_15_RBUS data_15;

		data_00.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_00_reg);
		data_01.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_01_reg);
		data_02.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_02_reg);
		data_03.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_03_reg);
		data_04.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_04_reg);
		data_05.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_05_reg);
		data_06.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_06_reg);
		data_07.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_07_reg);
		data_08.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_08_reg);
		data_09.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_09_reg);
		data_10.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_10_reg);
		data_11.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_11_reg);
		data_12.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_12_reg);
		data_13.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_13_reg);
		data_14.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_14_reg);
		data_15.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile3_Data_15_reg);

		// Get Horizontal profile data for V.
		Profile3_H_Block_V[0]   = data_00.ch1_pf3_h00;
		Profile3_H_Block_V[1]   = data_00.ch1_pf3_h01;
		Profile3_H_Block_V[2]   = data_01.ch1_pf3_h02;
		Profile3_H_Block_V[3]   = data_01.ch1_pf3_h03;
		Profile3_H_Block_V[4]   = data_02.ch1_pf3_h04;
		Profile3_H_Block_V[5]   = data_02.ch1_pf3_h05;
		Profile3_H_Block_V[6]   = data_03.ch1_pf3_h06;
		Profile3_H_Block_V[7]   = data_03.ch1_pf3_h07;
		Profile3_H_Block_V[8]   = data_04.ch1_pf3_h08;
		Profile3_H_Block_V[9]   = data_04.ch1_pf3_h09;
		Profile3_H_Block_V[10] = data_05.ch1_pf3_h10;
		Profile3_H_Block_V[11] = data_05.ch1_pf3_h11;
		Profile3_H_Block_V[12] = data_06.ch1_pf3_h12;
		Profile3_H_Block_V[13] = data_06.ch1_pf3_h13;
		Profile3_H_Block_V[14] = data_07.ch1_pf3_h14;
		Profile3_H_Block_V[15] = data_07.ch1_pf3_h15;

		// Get Vertical profile data for V.
		Profile3_V_Block_V[0]   = data_08.ch1_pf3_v00;
		Profile3_V_Block_V[1]   = data_08.ch1_pf3_v01;
		Profile3_V_Block_V[2]   = data_09.ch1_pf3_v02;
		Profile3_V_Block_V[3]   = data_09.ch1_pf3_v03;
		Profile3_V_Block_V[4]   = data_10.ch1_pf3_v04;
		Profile3_V_Block_V[5]   = data_10.ch1_pf3_v05;
		Profile3_V_Block_V[6]   = data_11.ch1_pf3_v06;
		Profile3_V_Block_V[7]   = data_11.ch1_pf3_v07;
		Profile3_V_Block_V[8]   = data_12.ch1_pf3_v08;
		Profile3_V_Block_V[9]   = data_12.ch1_pf3_v09;
		Profile3_V_Block_V[10] = data_13.ch1_pf3_v10;
		Profile3_V_Block_V[11] = data_13.ch1_pf3_v11;
		Profile3_V_Block_V[12] = data_14.ch1_pf3_v12;
		Profile3_V_Block_V[13] = data_14.ch1_pf3_v13;
		Profile3_V_Block_V[14] = data_15.ch1_pf3_v14;
		Profile3_V_Block_V[15] = data_15.ch1_pf3_v15;


		//Enable profile
		hist_profile3_ctrl_reg.ch1_pf3_enable_profile = 0x1;
		scaler_rtd_outl(PROFILE_ICH1_Hist_Profile3_CTRL_reg, hist_profile3_ctrl_reg.regValue);

		Profile3_V_Sum = 0;
		for( ii=0; ii<16; ii++ ) {
			Profile3_V_Sum += Profile3_H_Block_V[ii];
		}
		//Profile3_V_Sum = Profile3_V_Sum << profsum_shift;

		#if Print_Profile_Data
		if (Profile_Print_Count%700 ==0)
		{
			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### Profile3_H_Block_V[%d] =%d ### \n", jj, Profile3_H_Block_V[jj]);
			}

			for(jj=0; jj<16; jj++ )
			{
				ROSPrintf( "### Profile3_V_Block_V[%d] =%d ### \n", jj, Profile3_V_Block_V[jj]);
			}

			ROSPrintf( "[ScalerVIP] ========Profile OK, Profile3_V_Sum =%d========\n", Profile3_V_Sum);

			Profile_Print_Count = 0;
		}

		Profile_Print_Count++;
		#endif

		return SUCCESS;
	}
	else
	{
		//ROSPrintf( "[ScalerVIP] ==========Profile NOT ready==========\n" );
		return FAILED;
	}

}

void scalerVIP_Difference_H_V_Calculate()
{
	unsigned char ii;
	unsigned char Profile_Block_Num = 16;//set profile h&V block number


	unsigned short *H_Block = &(SmartPic_clue->Profile_Cal_INFO.H_Block_Y[0]);
	unsigned short *V_Block = &(SmartPic_clue->Profile_Cal_INFO.V_Block_Y[0]);

	static unsigned short pre_H_Block[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static unsigned short pre_V_Block[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//	unsigned short Difference_H[16], Difference_V[16];
	unsigned int sum_of_difference = 0;

	for (ii=0;ii<Profile_Block_Num;ii++)
	{
		//calculate difference
		Difference_H[ii] = abs_value(H_Block[ii] - pre_H_Block[ii]);
		Difference_V[ii] = abs_value(V_Block[ii] - pre_V_Block[ii]);

		//calculate sum of difference
		sum_of_difference = sum_of_difference + Difference_H[ii] + Difference_V[ii];
	}

	//save profile to buffer:pre_H_Block, pre_V_Block
	for (ii=0;ii<Profile_Block_Num;ii++)
	{
		pre_H_Block[ii] = H_Block[ii];
		pre_V_Block[ii] = V_Block[ii];
	}

}

//20130402 Elsie: Still Motion judgement
MOTION_LEVEL scalerVIP_StillMotion(_clues* SmartPic_clue)
{
	unsigned char ii;
	unsigned char Profile_Block_Num = 16;//set profile h&V block number

	static unsigned char exit_motion_cnt_still = 0;
	static unsigned char exit_motion_cnt_very_still = 0;
	static unsigned char very_motion_cnt2 =0;

	MOTION_LEVEL flag_motion_type = MOTION_LEVEL_MOTION;
	static MOTION_LEVEL flag_motion_type_pre = MOTION_LEVEL_MOTION;
	static unsigned char isEvenFrame = 0;

	unsigned short *H_Block = &(SmartPic_clue->Profile_Cal_INFO.H_Block_Y[0]);
	unsigned short *V_Block = &(SmartPic_clue->Profile_Cal_INFO.V_Block_Y[0]);

	static unsigned short pre_H_Block[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static unsigned short pre_V_Block[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//	unsigned short Difference_H[16], Difference_V[16];
	unsigned int sum_of_difference = 0;

	//unsigned short H_Block[16], V_Block[16];
	unsigned short threshold_still = 180;

	MOTION_LEVEL MAD_level =0;

	//scalerDrvPrint( "enter scalerVIP_colorProfileStillMotion\n" );

	//Set threshold_still
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_1080PM50:
		case _MODE_1080PM60:
		case _MODE_1080P50:
		case _MODE_1080P60:
			threshold_still = 640;
			break;
		case _MODE_1080I25:
		case _MODE_1080I30:
			threshold_still =300;
			break;
		default:
			threshold_still = 180;	//CVBS, YPbPr
			break;
	}

	MAD_level  =999 ;
	//motion detection
	for (ii=0;ii<Profile_Block_Num;ii++)
	{
		//calculate difference
		Difference_H[ii] = abs_value(H_Block[ii] - pre_H_Block[ii]);
		Difference_V[ii] = abs_value(V_Block[ii] - pre_V_Block[ii]);

		//calculate sum of difference
		sum_of_difference = sum_of_difference + Difference_H[ii] + Difference_V[ii];
	}

	if (sum_of_difference <= threshold_still)	//not "very motion"
	{

		MAD_level = scalerVIP_MADStillMotion();	//Using MAD value
		very_motion_cnt2=0;
		if (MAD_level == MOTION_LEVEL_VERY_STILL)	//very still
		{
			very_motion_cnt=0;
			very_still_cnt ++;
			exit_motion_cnt_very_still++;
			if ( exit_motion_cnt_very_still >= 2 )
				flag_motion_type = MOTION_LEVEL_VERY_STILL; //pattern is very still
			else
				flag_motion_type = flag_motion_type_pre;
		}
		else if (MAD_level == MOTION_LEVEL_STILL) //still
		{
			very_motion_cnt++;
			very_still_cnt=0;
			exit_motion_cnt_still++;
			if ( exit_motion_cnt_still >= 2 )
				flag_motion_type = MOTION_LEVEL_STILL; //pattern is still
			else
				flag_motion_type = flag_motion_type_pre;
		}
		else
		{
			flag_motion_type = MOTION_LEVEL_SLOW_MOTION;
			very_motion_cnt++;
			very_still_cnt=0;
			exit_motion_cnt_still = 0;
			exit_motion_cnt_very_still = 0;
		}
		if (very_motion_cnt>20) very_motion_cnt=20;

	}
	else
	{
		very_motion_cnt++;
		very_motion_cnt2 ++;
		very_still_cnt=0;

		if ( very_motion_cnt2 >= 2 )
			flag_motion_type = MOTION_LEVEL_MOTION; //pattern is motion
		else
			flag_motion_type = flag_motion_type_pre; //pattern is motion

		exit_motion_cnt_still = 0;
		exit_motion_cnt_very_still = 0;
	}
#if 0
	if(MA_print_count % 3 == 0)
	{
		ROSPrintf("sum_of_difference=%d,threshold_still=%d\n",sum_of_difference,threshold_still );
		ROSPrintf("flag_motion_type=%d,MAD_level=%d ====\n",flag_motion_type,MAD_level  );
	}
#endif


	//save profile to buffer:pre_H_Block, pre_V_Block
	for (ii=0;ii<Profile_Block_Num;ii++)
	{
		pre_H_Block[ii] = H_Block[ii];
		pre_V_Block[ii] = V_Block[ii];
	}

	isEvenFrame = (isEvenFrame+1) % 2;
	flag_motion_type_pre = flag_motion_type;

	return flag_motion_type;
}

//20130402 Elsie add for motion detection with MAD value.
MOTION_LEVEL scalerVIP_MADStillMotion(void)
{
	di_im_di_rtnr_mad_y_th_RBUS im_di_rtnr_mad_y_th_reg;
	di_im_di_rtnr_mad_c_th_RBUS im_di_rtnr_mad_c_th_reg;
	di_im_di_rtnr_mad_y_counter_RBUS mad_y;
	di_im_di_rtnr_mad_u_counter_RBUS mad_u;
	di_im_di_rtnr_mad_v_counter_RBUS mad_v;

	unsigned int MAD_counter[3]; //Y,U,V respectively
	unsigned short MAD_threshold_y = 56;
	unsigned short MAD_threshold_c = 56;
	unsigned int MAD_counter_sum = 0;
	unsigned int MAD_counter_sum_threshold;
	static unsigned int preMAD_csum=0;
	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;
	RPC_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();

	if(RPC_system_info_structure_table == NULL)
	{
		if(MA_print_count%360==0)
			ROSPrintf("Share Memory PQ data ERROR, system_info = %p, \n",RPC_system_info_structure_table);
		return MOTION_LEVEL_MOTION;
	}


	im_di_rtnr_mad_y_th_reg.regValue = scaler_rtd_inl(DI_IM_DI_RTNR_MAD_Y_TH_reg);
	im_di_rtnr_mad_c_th_reg.regValue = scaler_rtd_inl(DI_IM_DI_RTNR_MAD_C_TH_reg);
	im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = MAD_threshold_y;
	im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th1 = MAD_threshold_c;
	if(RPC_system_info_structure_table->Project_ID == VIP_Project_ID_TV006){
	scaler_rtd_outl(DI_IM_DI_RTNR_MAD_Y_TH_reg,im_di_rtnr_mad_y_th_reg.regValue);
	scaler_rtd_outl(DI_IM_DI_RTNR_MAD_C_TH_reg,im_di_rtnr_mad_c_th_reg.regValue);
	}

	mad_y.res1 = rtd_inl(DI_IM_DI_RTNR_MAD_Y_COUNTER_reg);
	mad_u.res1 = rtd_inl(DI_IM_DI_RTNR_MAD_U_COUNTER_reg);
	mad_v.res1 = rtd_inl(DI_IM_DI_RTNR_MAD_V_COUNTER_reg);

	MAD_counter[0] = mad_y.cp_temporal_mad_countery;
	MAD_counter[1] = mad_u.cp_temporal_mad_counteru;
	MAD_counter[2] = mad_v.cp_temporal_mad_counterv;

	MAD_counter_sum =  MAD_counter[0] + MAD_counter[1] + MAD_counter[2];

	//Set MAD_counter_sum_threshold
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_1080PM50:
		case _MODE_1080PM60:
		case  _MODE_1080P50:
		case _MODE_1080P60:
			MAD_counter_sum_threshold = 220;
			break;
		case  _MODE_1080I25:
		case  _MODE_1080I30:
			MAD_counter_sum_threshold = 300;	//HDMI
			  if (MAD_counter_sum <= 10 || ( preMAD_csum+MAD_counter_sum)<260 )
			  	{
		   		      preMAD_csum=MAD_counter_sum;
				      return MOTION_LEVEL_VERY_STILL;	//very still
			  	}
			  else if (MAD_counter_sum <= MAD_counter_sum_threshold)
			  	{
			  	  preMAD_csum=MAD_counter_sum;
			         return MOTION_LEVEL_STILL;	//almost still
			  	}
			  else
			  	{
			  	  preMAD_csum=MAD_counter_sum;
			    	  return MOTION_LEVEL_SLOW_MOTION;	//slow motion, suppose it is randomly
				}
			break;
		default:
			MAD_counter_sum_threshold = 72;	//CVBS, YPbPr
			break;
	}

	if(MA_print_count % 9 == 0)
	{
//		ROSPrintf("MAD_counter_sum=%d,threshold=%d\n",MAD_counter_sum,MAD_counter_sum_threshold );
	}


	if (MAD_counter_sum == 0)
		return MOTION_LEVEL_VERY_STILL;	//very still
	else if (MAD_counter_sum <= MAD_counter_sum_threshold)
		return MOTION_LEVEL_STILL;	//almost still
	else
		return MOTION_LEVEL_SLOW_MOTION;	//slow motion, suppose it is randomly
}

#ifdef ISR_FADE_DETECT_FUNCTIOM_ENABLE
//Elsie 20130416: big smooth area & fade in/fade out detection.
unsigned char scalerVIP_BigSmoothAreaDetection_luma(void)
{
#if 0 // rbus sync error!! CSFC 2015/03/04
	unsigned int ii;
	unsigned int max_hist[3],max_hist_ID[3],hist_sum_tmp;
	unsigned char flag_big_smooth_area_luma = 0;
	unsigned char flag_sharpness_on, flag_high_edge_density = 0;
	unsigned int *reg_HistCnt=(SmartPic_clue->Y_Main_Hist);

	scalerDrvPrint("enter scalerVIP_ReadHist\n");

	// Luma
	max_hist[0] = max_hist[1] = max_hist[2] = 0;
	max_hist_ID[0] = max_hist_ID[1] = max_hist_ID[2] = 0;
	hist_sum_tmp = 0;
	for(ii=0;ii<COLOR_HISTOGRAM_LEVEL;ii++)
	{
		if(max_hist[0] < reg_HistCnt[ii])
		{
			max_hist[2] = max_hist[1];
			max_hist_ID[2] = max_hist_ID[1];
			max_hist[1] = max_hist[0];
			max_hist_ID[1] = max_hist_ID[0];
			max_hist[0] = reg_HistCnt[ii];
			max_hist_ID[0] = ii;
		}
		else if(max_hist[1] < reg_HistCnt[ii])
		{
			max_hist[2] = max_hist[1];
			max_hist_ID[2] = max_hist_ID[1];
			max_hist[1] = reg_HistCnt[ii];
			max_hist_ID[1] = ii;
		}
		else if(max_hist[2] < reg_HistCnt[ii])
		{
			max_hist[2] = reg_HistCnt[ii];
			max_hist_ID[2] = ii;
		}
	}

	//"edge" density: if there is too many edges, force the return value to false.
	flag_sharpness_on = rtd_t_inl(color_sharp_dm_nr_shp_ctrl_RBUS, COLOR_SHARP_DM_NR_SHP_CTRL_reg).mkiii_en;
	if(flag_sharpness_on == 1)
	{
		color_sharp_dm_auto_shp_data_RBUS dm_auto_shp_data_reg;
		dm_auto_shp_data_reg.regValue = rtd_inl(COLOR_SHARP_DM_AUTO_SHP_DATA_reg);
		unsigned int Hist_HPF[16], Hist_HPF_total = 0;

		if(dm_auto_shp_data_reg.enable == 1)	//histogram after HPF
		{
			Hist_HPF[0] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_00_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_00_reg).lut0;
			Hist_HPF[1] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_01_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_01_reg).lut1;
			Hist_HPF[2] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_02_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_02_reg).lut2;
			Hist_HPF[3] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_03_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_03_reg).lut3;
			Hist_HPF[4] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_04_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_04_reg).lut4;
			Hist_HPF[5] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_05_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_05_reg).lut5;
			Hist_HPF[6] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_06_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_06_reg).lut6;
			Hist_HPF[7] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_07_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_07_reg).lut7;
			Hist_HPF[8] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_08_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_08_reg).lut8;
			Hist_HPF[9] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_09_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_09_reg).lut9;
			Hist_HPF[10] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_10_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_10_reg).lut10;
			Hist_HPF[11] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_11_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_11_reg).lut11;
			Hist_HPF[12] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_12_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_12_reg).lut12;
			Hist_HPF[13] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_13_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_13_reg).lut13;
			Hist_HPF[14] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_14_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_14_reg).lut14;
			Hist_HPF[15] = rtd_t_inl(color_sharp_dm_auto_shp_fir_count_15_RBUS, COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_15_reg).lut15;

			for(ii=0;ii<16;ii++)
			{
				Hist_HPF_total = Hist_HPF_total + Hist_HPF[ii];
			}
			if(Hist_HPF_total*4/5 > Hist_HPF[0]+Hist_HPF[1])
				flag_high_edge_density = 1;
		}
		dm_auto_shp_data_reg.enable = 1;
		dm_auto_shp_data_reg.shift_bit = 0;
		rtd_outl(COLOR_SHARP_DM_AUTO_SHP_DATA_reg, dm_auto_shp_data_reg.regValue);
	}

	if(flag_high_edge_density == 1)
	{
		flag_big_smooth_area_luma = 0;
	}
	else if((max_hist[0]>>2)>max_hist[1]) 	//max_hist[0]>(max_hist[1]*4)
	{
		flag_big_smooth_area_luma = 1;
	}
	else if(abs_value(max_hist_ID[0]-max_hist_ID[1])==1)	//nearby
	{
		hist_sum_tmp = max_hist[0] + max_hist[1];
		if((hist_sum_tmp>>2)>max_hist[2])
		{
			flag_big_smooth_area_luma = 1;
		}
		else
		{
			flag_big_smooth_area_luma = 0;
		}
	}
	else
	{
		flag_big_smooth_area_luma = 0;
	}
	return flag_big_smooth_area_luma;
#endif
   return 0 ;
}

unsigned char scalerVIP_BigSmoothAreaDetection_chroma(void)
{
	unsigned int ii,hue_max_hist[3],hue_max_hist_ID[3],hue_hist_01 = 0,hue_hist_sum = 0;
	unsigned char flag_big_smooth_area_chroma = 0;

	// Hue
	hue_max_hist[0] = hue_max_hist[1] = hue_max_hist[2] = 0;
	hue_max_hist_ID[0] = hue_max_hist_ID[1] = hue_max_hist_ID[2] = 0;
	for(ii=0;ii<24;ii++)
	{
		if(hue_max_hist[0] < hue_hist_2[ii])
		{
			hue_max_hist[2] = hue_max_hist[1];
			hue_max_hist_ID[2] = hue_max_hist_ID[1];
			hue_max_hist[1] = hue_max_hist[0];
			hue_max_hist_ID[1] = hue_max_hist_ID[0];
			hue_max_hist[0] = hue_hist_2[ii];
			hue_max_hist_ID[0] = ii;
		}
		else if(hue_max_hist[1] < hue_hist_2[ii])
		{
			hue_max_hist[2] = hue_max_hist[1];
			hue_max_hist_ID[2] = hue_max_hist_ID[1];
			hue_max_hist[1] = hue_hist_2[ii];
			hue_max_hist_ID[1] = ii;
		}
		else if(hue_max_hist[2] < hue_hist_2[ii])
		{
			hue_max_hist[2] = hue_hist_2[ii];
			hue_max_hist_ID[2] = ii;
		}
		hue_hist_sum = hue_hist_sum + hue_hist_2[ii];
	}

	if (hue_max_hist[0]==0)	// pure gray level image, force the return value to true.
	{
		flag_big_smooth_area_chroma = 1;
	}
	else if((hue_max_hist[0]>>2)>hue_max_hist[1])	//max_hist[0]>(max_hist[1]*4)
	{
		flag_big_smooth_area_chroma = 1;
	}
	else if(abs_value(hue_max_hist_ID[0]-hue_max_hist_ID[1])==1)	//nearby
	{
		hue_hist_01 = hue_max_hist[0] + hue_max_hist[1];
		if((hue_hist_01>>2)>hue_max_hist[2])
		{
			flag_big_smooth_area_chroma = 1;
		}
		else
		{
			flag_big_smooth_area_chroma = 0;
		}
	}
	else
	{
		flag_big_smooth_area_chroma = 0;
	}

	return flag_big_smooth_area_chroma;
}
#endif

unsigned char scalerVIP_ColorbarDetector(_clues* SmartPic_clue)
{
	unsigned char ii, ProfileCount = 0;
	unsigned short thl;
	//unsigned short *H_Block = &(SmartPic_clue->Profile_Cal_INFO.H_Block_Y[0]);
	unsigned short *V_Block = &(SmartPic_clue->Profile_Cal_INFO.V_Block_Y[0]);

	//Set threshold_still
	switch (SmartPic_clue->SCALER_DISP_INFO.ucMode_Curr)
	{
		case _MODE_1080PM50:
		case _MODE_1080PM60:
		case _MODE_1080P50:
		case _MODE_1080P60:
			thl = 20;
			break;
		case _MODE_1080I25:
		case _MODE_1080I30:
			thl =10;
			break;
		default:
			thl = 6;	//CVBS, YPbPr
			break;
	}

	for(ii=1;ii<9;ii++)
	{
		if(V_Block[ii]-V_Block[ii-1] < thl)
			ProfileCount++;
	}

	if(ProfileCount == 8)
		return TRUE;
	else
		return FALSE;

}

#if 1

void scalerVIP_luma_level_distribution_Remapping(unsigned int *luma_distribution, unsigned char size)
{
	unsigned char nTotalSize = COLOR_HISTOGRAM_LEVEL;
	unsigned char nSegment = nTotalSize / size;
	unsigned char nIndex = 0, nSegIndx = 0;
	_clues* SmartPic_clue=NULL;
	unsigned short nPoint = 1;
	unsigned int nSumPoint = 0;

	SmartPic_clue = scaler_GetShare_Memory_SmartPic_Clue_Struct();

	if (SmartPic_clue == NULL) {
		for (nIndex = 0; nIndex < size; nIndex++) {
			luma_distribution[nIndex] = 0;
		}
		return;
	}

	for (nIndex = 0; nIndex < size; nIndex++) {
		luma_distribution[nIndex] = 0;
	}
	for (nIndex = 0; nIndex < size; nIndex++) {
		nSumPoint = 0;
		for (nSegIndx = 0; nSegIndx < nSegment; nSegIndx++) {
			if (nIndex * nSegment + nSegIndx < nTotalSize) {
				nSumPoint +=  SmartPic_clue->Y_Main_Hist[nIndex * nSegment + nSegIndx];
			}
		}
		luma_distribution[nIndex]  = nSumPoint*nPoint;

	}
}


void scalerVIP_YHist_HueHist_Ratio_Calculate()
{
	unsigned char i;
	unsigned int hue_sum = 0;

	unsigned int printf_Flag = system_info_structure_table->ISR_Printf_Flag.Y_Hue_Sat_Hist_Flag0;
	unsigned int print_delay = (system_info_structure_table->ISR_Printf_Flag.Delay_Time == 0)?(1):(system_info_structure_table->ISR_Printf_Flag.Delay_Time);
	for(i=0;i<24;i++) {
		//hue_hist_total += hue_hist_2[i];
		hue_hist_2[i] = scaler_rtd_inl( COLOR_ICM_HUE_COUNT_00_reg + (i<<2));
		hue_sum += hue_hist_2[i];
	}

	/*for(i=0;i<32;i++)
		y_sum += reg_HistCnt[i];*/
	if((SmartPic_clue->SCALER_DISP_INFO.DispLen*SmartPic_clue->SCALER_DISP_INFO.DispWid)!=0) {
		SmartPic_clue->hue_sumcnt_ratio = hue_sum*1000/(SmartPic_clue->SCALER_DISP_INFO.DispLen*SmartPic_clue->SCALER_DISP_INFO.DispWid);
	}

	if(hue_sum <= 1000) {
		//calculate permillage for Hue
		for(i=0;i<24;i++)
			 hue_hist_ratio[i] = 0;
		colorless_flag =1;
	} else {
		//calculate permillage for Hue
		for(i=0;i<24;i++)
			 hue_hist_ratio[i] = (hue_hist_2[i]/(hue_sum/1000));
		colorless_flag =0;
	}

	SmartPic_clue->colorlessflag = colorless_flag;
	for(i=0;i<24;i++) {
		(SmartPic_clue->Hue_Main_His)[i]	= hue_hist_2[i];
		(SmartPic_clue->Hue_Main_His_Ratio)[i] = hue_hist_ratio[i];
	}

	if((SmartPic_clue->Hist_Y_Total_sum)==0) {
		//calculate permillage for Y
		for(i=0;i<TV006_VPQ_chrm_bin;i++)
			 Y_hist_ratio[i] = 0;
	}
	else
	{
		//calculate permillage for Y
               if( (TV006_VPQ_chrm_bin > COLOR_HISTOGRAM_LEVEL) || ((COLOR_HISTOGRAM_LEVEL%TV006_VPQ_chrm_bin) != 0)) {
			for(i=0;i<TV006_VPQ_chrm_bin;i++)
			 	Y_hist_ratio[i] = 0;
		}
		else
		{
			unsigned int Tmp_Y_Histogram[32] = {0};
			scalerVIP_luma_level_distribution_Remapping(Tmp_Y_Histogram, TV006_VPQ_chrm_bin);
			for(i=0;i<TV006_VPQ_chrm_bin;i++)
				Y_hist_ratio[i] = (Tmp_Y_Histogram[i]*1000/(SmartPic_clue->Hist_Y_Total_sum));
		}
	}

	for(i=0;i<TV006_VPQ_chrm_bin;i++) {
		(SmartPic_clue->Y_Main_Hist_Ratio)[i] = Y_hist_ratio[i];
	}

#if 0
	if(MA_print_count%150==0 )
	{
		ROSPrintf("{");
		for(i=0;i<24;i++)
			ROSPrintf("%d,%d,%d ",hue_hist_ratio[i],(SmartPic_clue->Hue_Main_His)[i],(SmartPic_clue->Hue_Main_His_Ratio)[i]);
		ROSPrintf("}\n");

		ROSPrintf("{");
		for(i=0;i<32;i++)
			ROSPrintf("%d,%d ",Y_hist_ratio[i],(SmartPic_clue->Y_Main_Hist_Ratio)[i]);
		ROSPrintf("}\n");

		ROSPrintf("clf=%d,%d\n",colorless_flag,hue_sum);
	}
#endif

#if 1
	//for debug.
	if(((printf_Flag&_BIT0)!=0))
	{
		if(MA_print_count%print_delay==0)
		{
			rtd_pr_vpq_isr_info("Y_Hue_Sat cal, Y={");
			for(i=0;i<TV006_VPQ_chrm_bin;i++)
				rtd_pr_vpq_isr_info("%d,", SmartPic_clue->Y_Main_Hist_Ratio[i]);
			rtd_pr_vpq_isr_info("}\n");
		}
	}
	if(((printf_Flag&_BIT1)!=0))
	{
		if(MA_print_count%print_delay==0)
		{
			rtd_pr_vpq_isr_info("Y_Hue_Sat cal, Y={");
			for(i=0;i<TV006_VPQ_chrm_bin;i++)
				rtd_pr_vpq_isr_info("%d,", SmartPic_clue->Y_Main_Hist_Ratio[i]);
			rtd_pr_vpq_isr_info("}\n");
		}
	}

	if(((printf_Flag&_BIT2)!=0))
	{
		if(MA_print_count%print_delay==0)
		{
			rtd_pr_vpq_isr_info("Y_Hue_Sat cal, Hue={");
			for(i=0;i<COLOR_HUE_HISTOGRAM_LEVEL;i++)
				rtd_pr_vpq_isr_info("%d,", SmartPic_clue->Hue_Main_His[i]);
			rtd_pr_vpq_isr_info("}\n");
		}
	}
	if(((printf_Flag&_BIT3)!=0))
	{
		if(MA_print_count%print_delay==0)
		{
			rtd_pr_vpq_isr_info("Y_Hue_Sat cal, Hue_ratio={");
			for(i=0;i<COLOR_HUE_HISTOGRAM_LEVEL;i++)
				rtd_pr_vpq_isr_info("%d,", SmartPic_clue->Hue_Main_His_Ratio[i]);
			rtd_pr_vpq_isr_info("}\n");
		}
	}
#endif

}

void scalerVIP_Y_U_V_Mad_Calculate(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table/*, _RPC_system_setting_info* RPC_system_info_structure_table*/)
{
	unsigned short IphActWid_H=0,IpvActLen_V=0;
	unsigned int RTNR_TempMAD_count_Y=0,RTNR_TempMAD_count_U=0,RTNR_TempMAD_count_V=0;
	unsigned int RTNR_TempMAD_count_Y2=0,RTNR_TempMAD_count_U2=0,RTNR_TempMAD_count_V2=0;
	unsigned int RTNR_TempMAD_count_Y3=0,RTNR_TempMAD_count_U3=0,RTNR_TempMAD_count_V3=0;
	unsigned int RTNR_TempMAD_count_Y4=0,RTNR_TempMAD_count_U4=0,RTNR_TempMAD_count_V4=0;

	unsigned int total_pix; //rtnr_noise_new;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_MAD_HistTh_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_MAD_Hist_Th][0]);

//==============================================================
//========================Set MAD Threshold  =======================

	di_im_di_rtnr_mad_y_th_RBUS im_di_rtnr_mad_y_th_reg;
	di_im_di_rtnr_mad_y_th2_c_th2_RBUS im_di_rtnr_mad_y_th2_c_th2_reg;
	di_im_di_rtnr_mad_c_th_RBUS im_di_rtnr_mad_c_th_reg;
	//di_im_di_rtnr_level_bound_RBUS im_di_rtnr_level_bound_reg;
	//di_im_new_mcnr_control3_RBUS di_im_new_mcnr_control3; //mac6 use old mcnr
	di_im_di_rtnr_control_RBUS im_di_rtnr_control_reg;

	di_im_di_rtnr_mad_y_counter_RBUS im_di_rtnr_mad_y_counter_reg;
	di_im_di_rtnr_mad_u_counter_RBUS im_di_rtnr_mad_u_counter_reg;
	di_im_di_rtnr_mad_v_counter_RBUS im_di_rtnr_mad_v_counter_reg;

	di_im_di_rtnr_mad_y_counter_2_RBUS im_di_rtnr_mad_y_counter_2_reg;
	di_im_di_rtnr_mad_u_counter_2_RBUS im_di_rtnr_mad_u_counter_2_reg;
	di_im_di_rtnr_mad_v_counter_2_RBUS im_di_rtnr_mad_v_counter_2_reg;

	di_im_di_rtnr_mad_y_counter_3_RBUS im_di_rtnr_mad_y_counter_3_reg;
	di_im_di_rtnr_mad_u_counter_3_RBUS im_di_rtnr_mad_u_counter_3_reg;
	di_im_di_rtnr_mad_v_counter_3_RBUS im_di_rtnr_mad_v_counter_3_reg;

	unsigned int printf_Flag = system_info_structure_table->ISR_Printf_Flag.MAD_Hist_Flag0;
	unsigned int print_delay = (system_info_structure_table->ISR_Printf_Flag.Delay_Time == 0)?(1):(system_info_structure_table->ISR_Printf_Flag.Delay_Time);

	im_di_rtnr_mad_y_th_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_TH_reg);

	im_di_rtnr_mad_c_th_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_C_TH_reg);
	im_di_rtnr_mad_y_th2_c_th2_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_TH2_C_TH2_reg);

	//di_im_new_mcnr_control3.regValue = rtd_inl(DI_IM_NEW_MCNR_CONTROL3_reg); //mac6 use old mcnr

	/* get size info from scaler*/
	/*scalerVIP_Get_DI_Width_Height(&IphActWid_H, &IpvActLen_V);*/
	IpvActLen_V = system_info_structure_table->I_Height;
	IphActWid_H = system_info_structure_table->I_Width;

	im_di_rtnr_control_reg.regValue = rtd_inl(DI_IM_DI_RTNR_CONTROL_reg);

	if (pPQA_MAD_HistTh_TBL[PQA_I_L00] == 1) {	/* controlled by PQA input table*/
			im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = pPQA_MAD_HistTh_TBL[PQA_I_L01];
			im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2 = pPQA_MAD_HistTh_TBL[PQA_I_L02];
			im_di_rtnr_mad_y_th2_c_th2_reg.cp_temporal_mad_y_th3=pPQA_MAD_HistTh_TBL[PQA_I_L03];

			im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th1 =pPQA_MAD_HistTh_TBL[PQA_I_L04];
			im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th2 =pPQA_MAD_HistTh_TBL[PQA_I_L05];
			im_di_rtnr_mad_y_th2_c_th2_reg.cp_temporal_mad_c_th3=pPQA_MAD_HistTh_TBL[PQA_I_L06]; //big motion

			/* //mac6 use old mcnr
			if(IphActWid_H <=1920)
				di_im_new_mcnr_control3.n_mcnr_mad_hist_shift_en = 0;
			else
				di_im_new_mcnr_control3.n_mcnr_mad_hist_shift_en = 1;
			*/

	} else {
		if(IphActWid_H <=960){

			if(im_di_rtnr_control_reg.cp_rtnr_progressive != 1)
				im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = 4;
			/*else //coverity CID:27435, same code
				im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = 4;*/

			im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2 = 0x20;
			im_di_rtnr_mad_y_th2_c_th2_reg.cp_temporal_mad_y_th3=128;


			im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th1 =2;
			im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th2 =12;

			im_di_rtnr_mad_y_th2_c_th2_reg.cp_temporal_mad_c_th3=1023;

			//di_im_new_mcnr_control3.n_mcnr_mad_hist_shift_en=0; //mac6 use old mcnr


		}else if(IphActWid_H <=1920){
			im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = 4;
			im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2 = 0x20;
			im_di_rtnr_mad_y_th2_c_th2_reg.cp_temporal_mad_y_th3=128;

			im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th1 =0;
			im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th2 =2;
			im_di_rtnr_mad_y_th2_c_th2_reg.cp_temporal_mad_c_th3=2047; //big motion

			//di_im_new_mcnr_control3.n_mcnr_mad_hist_shift_en=0; //mac6 use old mcnr

		}
		else
		{
			im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = 4;
			im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2 = 0x20;
			im_di_rtnr_mad_y_th2_c_th2_reg.cp_temporal_mad_y_th3=2047;

			im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th1 =0;
			im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th2 =2;
			im_di_rtnr_mad_y_th2_c_th2_reg.cp_temporal_mad_c_th3=2047; //big motion

			//di_im_new_mcnr_control3.n_mcnr_mad_hist_shift_en=1; //mac6 use old mcnr


		}
	}

	//rtd_outl(DI_IM_NEW_MCNR_CONTROL3_reg,di_im_new_mcnr_control3.regValue); //mac6 use old mcnr

	rtd_outl(DI_IM_DI_RTNR_MAD_Y_TH_reg, im_di_rtnr_mad_y_th_reg.regValue);
	rtd_outl(DI_IM_DI_RTNR_MAD_C_TH_reg, im_di_rtnr_mad_c_th_reg.regValue);
	rtd_outl(DI_IM_DI_RTNR_MAD_Y_TH2_C_TH2_reg, im_di_rtnr_mad_y_th2_c_th2_reg.regValue);

//=============================================================
//========================Motion MAD Check =======================


	im_di_rtnr_mad_y_counter_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_COUNTER_reg);
	im_di_rtnr_mad_u_counter_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_U_COUNTER_reg);
	im_di_rtnr_mad_v_counter_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_V_COUNTER_reg);

	im_di_rtnr_mad_y_counter_2_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_COUNTER_2_reg);
	im_di_rtnr_mad_u_counter_2_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_U_COUNTER_2_reg);
	im_di_rtnr_mad_v_counter_2_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_V_COUNTER_2_reg);

	im_di_rtnr_mad_y_counter_3_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_COUNTER_3_reg);
	im_di_rtnr_mad_u_counter_3_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_U_COUNTER_3_reg);
	im_di_rtnr_mad_v_counter_3_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_V_COUNTER_3_reg);


	RTNR_TempMAD_count_Y = im_di_rtnr_mad_y_counter_reg.cp_temporal_mad_countery;
	RTNR_TempMAD_count_U = im_di_rtnr_mad_u_counter_reg.cp_temporal_mad_counteru;
	RTNR_TempMAD_count_V = im_di_rtnr_mad_v_counter_reg.cp_temporal_mad_counterv;

	RTNR_TempMAD_count_Y2 = im_di_rtnr_mad_y_counter_2_reg.cp_temporal_mad_countery2;
	RTNR_TempMAD_count_U2 = im_di_rtnr_mad_u_counter_2_reg.cp_temporal_mad_counteru2;
	RTNR_TempMAD_count_V2 = im_di_rtnr_mad_v_counter_2_reg.cp_temporal_mad_counterv2;

	RTNR_TempMAD_count_Y3 = im_di_rtnr_mad_y_counter_3_reg.cp_temporal_mad_countery3;
	RTNR_TempMAD_count_U3 = im_di_rtnr_mad_u_counter_3_reg.cp_temporal_mad_counteru3;
	RTNR_TempMAD_count_V3 = im_di_rtnr_mad_v_counter_3_reg.cp_temporal_mad_counterv3;

	//IphActWid_H=(SmartPic_clue->SCALER_DISP_INFO.IphActWid_H);
	//IpvActLen_V=(SmartPic_clue->SCALER_DISP_INFO.IpvActLen_V);

//====================get condition===========================
	total_pix = IphActWid_H*IpvActLen_V;//Active_Region[1]*Active_Region[3];

	/* //mac6 use old mcnr
	if(di_im_new_mcnr_control3.n_mcnr_mad_hist_shift_en==1)
		total_pix =total_pix/4;
	*/
	if(total_pix==0)
		return;

	RTNR_TempMAD_count_Y4 = total_pix - (RTNR_TempMAD_count_Y + RTNR_TempMAD_count_Y2 + RTNR_TempMAD_count_Y3);
	RTNR_TempMAD_count_U4 = total_pix - (RTNR_TempMAD_count_U + RTNR_TempMAD_count_U2 + RTNR_TempMAD_count_U3);
	RTNR_TempMAD_count_V4 = total_pix - (RTNR_TempMAD_count_V + RTNR_TempMAD_count_V2 + RTNR_TempMAD_count_V3);

	RTNR_MAD_count_Y_avg = (RTNR_TempMAD_count_Y*1000)/total_pix;
	RTNR_MAD_count_U_avg = (RTNR_TempMAD_count_U*1000)/total_pix;
	RTNR_MAD_count_V_avg = (RTNR_TempMAD_count_V*1000)/total_pix;

	RTNR_MAD_count_Y2_avg = (RTNR_TempMAD_count_Y2*1000)/total_pix;
	RTNR_MAD_count_U2_avg = (RTNR_TempMAD_count_U2*1000)/total_pix;
	RTNR_MAD_count_V2_avg = (RTNR_TempMAD_count_V2*1000)/total_pix;

	RTNR_MAD_count_Y3_avg = (RTNR_TempMAD_count_Y3*1000)/total_pix;
	RTNR_MAD_count_U3_avg = (RTNR_TempMAD_count_U3*1000)/total_pix;
	RTNR_MAD_count_V3_avg = (RTNR_TempMAD_count_V3*1000)/total_pix;

	SmartPic_clue->RTNR_MAD_count_Y_avg_ratio = RTNR_MAD_count_Y_avg;
	SmartPic_clue->RTNR_MAD_count_U_avg_ratio = RTNR_MAD_count_U_avg;
	SmartPic_clue->RTNR_MAD_count_V_avg_ratio = RTNR_MAD_count_V_avg;
	SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio = RTNR_MAD_count_Y2_avg;
	SmartPic_clue->RTNR_MAD_count_U2_avg_ratio = RTNR_MAD_count_U2_avg;
	SmartPic_clue->RTNR_MAD_count_V2_avg_ratio = RTNR_MAD_count_V2_avg;
	SmartPic_clue->RTNR_MAD_count_Y3_avg_ratio = RTNR_MAD_count_Y3_avg;
	SmartPic_clue->RTNR_MAD_count_U3_avg_ratio = RTNR_MAD_count_U3_avg;
	SmartPic_clue->RTNR_MAD_count_V3_avg_ratio = RTNR_MAD_count_V3_avg;

	SmartPic_clue->RTNR_MAD_count_Y4_avg_ratio = (RTNR_TempMAD_count_Y4*1000)/total_pix;
	SmartPic_clue->RTNR_MAD_count_U4_avg_ratio = (RTNR_TempMAD_count_U4*1000)/total_pix;
	SmartPic_clue->RTNR_MAD_count_V4_avg_ratio = (RTNR_TempMAD_count_V4*1000)/total_pix;

	SmartPic_clue->noise_ratio = RTNR_MAD_count_Y2_avg;


	/*if(rtd_inl(0xb802e4f4) == 11){
		rtd_pr_vpq_isr_info("MAD_1=%d=%d,=%d\n",RTNR_MAD_count_Y_avg,RTNR_MAD_count_U_avg,RTNR_MAD_count_V_avg);
		rtd_pr_vpq_isr_info("MAD_2=%d=%d,=%d\n",RTNR_MAD_count_Y2_avg,RTNR_MAD_count_U2_avg,RTNR_MAD_count_V2_avg);
		rtd_pr_vpq_isr_info("MAD_3=%d=%d,=%d\n",RTNR_MAD_count_Y3_avg,RTNR_MAD_count_U3_avg,RTNR_MAD_count_V3_avg);
		rtd_pr_vpq_isr_info("\n H=%d V=%d,pix=%d\n",IphActWid_H,IpvActLen_V,total_pix);
	}*/

	/*if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_12==TRUE))
	{
		rtd_pr_vpq_isr_info("H=%d V=%d,pix=%d\n",IphActWid_H,IpvActLen_V,total_pix);
		rtd_pr_vpq_isr_info("MAD_1=%d=%d,=%d\n",RTNR_MAD_count_Y_avg,RTNR_MAD_count_U_avg,RTNR_MAD_count_V_avg);
		rtd_pr_vpq_isr_info("MAD_2=%d=%d,=%d\n",RTNR_MAD_count_Y2_avg,RTNR_MAD_count_U2_avg,RTNR_MAD_count_V2_avg);
		rtd_pr_vpq_isr_info("MAD_3=%d=%d,=%d\n",RTNR_MAD_count_Y3_avg,RTNR_MAD_count_U3_avg,RTNR_MAD_count_V3_avg);
	}*/
#if 1
	//for debug.
	if(((printf_Flag&_BIT0)!=0))
	{
		if(MA_print_count%print_delay==0)
		{
			rtd_pr_vpq_isr_info("MAD_1=%d=%d,=%d\n",RTNR_MAD_count_Y_avg,RTNR_MAD_count_U_avg,RTNR_MAD_count_V_avg);
			rtd_pr_vpq_isr_info("MAD_2=%d=%d,=%d\n",RTNR_MAD_count_Y2_avg,RTNR_MAD_count_U2_avg,RTNR_MAD_count_V2_avg);
			rtd_pr_vpq_isr_info("MAD_3=%d=%d,=%d\n",RTNR_MAD_count_Y3_avg,RTNR_MAD_count_U3_avg,RTNR_MAD_count_V3_avg);
			rtd_pr_vpq_isr_info("MAD_4=%d=%d,=%d\n",SmartPic_clue->RTNR_MAD_count_Y4_avg_ratio,SmartPic_clue->RTNR_MAD_count_U4_avg_ratio,SmartPic_clue->RTNR_MAD_count_V4_avg_ratio);
			rtd_pr_vpq_isr_info("H=%d V=%d,pix=%d\n",IphActWid_H,IpvActLen_V,total_pix);
		}
	}
#endif

}


void scalerVIP_HMCNR_statistic(void) // this fucntion is for progressive mode
{
	unsigned int total=0;
	unsigned char i=0;

	di_im_di_rtnr_hmcnr_RBUS di_im_di_rtnr_hmcnr_RBUS_reg;
	di_im_di_rtnr_hmcnr_statistic_1_RBUS di_im_di_rtnr_hmcnr_statistic_1_reg;
	di_im_di_rtnr_hmcnr_statistic_2_RBUS di_im_di_rtnr_hmcnr_statistic_2_reg;
	di_im_di_rtnr_hmcnr_statistic_3_RBUS di_im_di_rtnr_hmcnr_statistic_3_reg;
	di_im_di_rtnr_hmcnr_statistic_4_RBUS di_im_di_rtnr_hmcnr_statistic_4_reg;
	di_im_di_rtnr_hmcnr_statistic_5_RBUS di_im_di_rtnr_hmcnr_statistic_5_reg;
	di_im_di_rtnr_hmcnr_statistic_6_RBUS di_im_di_rtnr_hmcnr_statistic_6_reg;


	di_im_di_rtnr_hmcnr_RBUS_reg.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_reg);

#if 0	//20150706 roger mark for compiler error
	di_im_di_rtnr_hmcnr_RBUS_reg.cp_temporal_hmcnr_search_range=5;
#endif
	rtd_outl(DI_IM_DI_RTNR_HMCNR_reg,di_im_di_rtnr_hmcnr_RBUS_reg.regValue);
	di_im_di_rtnr_hmcnr_statistic_1_reg.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_1_reg);
	di_im_di_rtnr_hmcnr_statistic_2_reg.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_2_reg);
	di_im_di_rtnr_hmcnr_statistic_3_reg.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_3_reg);
	di_im_di_rtnr_hmcnr_statistic_4_reg.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_4_reg);
	di_im_di_rtnr_hmcnr_statistic_5_reg.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_5_reg);
	di_im_di_rtnr_hmcnr_statistic_6_reg.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_6_reg);


	hmcnr_sttistic[0]=di_im_di_rtnr_hmcnr_statistic_6_reg.hmcnr_l5_sum;
	hmcnr_sttistic[1]=di_im_di_rtnr_hmcnr_statistic_5_reg.hmcnr_l4_sum;
	hmcnr_sttistic[2]=di_im_di_rtnr_hmcnr_statistic_5_reg.hmcnr_l3_sum;
	hmcnr_sttistic[3]=di_im_di_rtnr_hmcnr_statistic_4_reg.hmcnr_l2_sum;
	hmcnr_sttistic[4]=di_im_di_rtnr_hmcnr_statistic_4_reg.hmcnr_l1_sum;
	hmcnr_sttistic[5]=di_im_di_rtnr_hmcnr_statistic_3_reg.hmcnr_c0_sum;
	hmcnr_sttistic[6]=di_im_di_rtnr_hmcnr_statistic_3_reg.hmcnr_r1_sum;
	hmcnr_sttistic[7]=di_im_di_rtnr_hmcnr_statistic_2_reg.hmcnr_r2_sum;
	hmcnr_sttistic[8]=di_im_di_rtnr_hmcnr_statistic_2_reg.hmcnr_r3_sum;
	hmcnr_sttistic[9]=di_im_di_rtnr_hmcnr_statistic_1_reg.hmcnr_r4_sum;
	hmcnr_sttistic[10]=di_im_di_rtnr_hmcnr_statistic_1_reg.hmcnr_r5_sum;

	for(i=0;i<11;i++)
	{

		total+=hmcnr_sttistic[i];
	}

	if(total==0)
		return;
	center = (hmcnr_sttistic[5]*1000)/total;
	L1 = (hmcnr_sttistic[4]*1000)/total;
	R1 = (hmcnr_sttistic[6]*1000)/total;
	others= ((hmcnr_sttistic[6]+hmcnr_sttistic[5]+hmcnr_sttistic[4])*1000)/total;

	SmartPic_clue->motion_ratio = center;

}



unsigned char scalerVIP_ET_Y_MOTION(void)
{
	static unsigned int et_y_sum_1, et_y_count_1, et_y_sum_2, et_y_count_2;
	di_im_di_rtnr_sad_offset_RBUS im_di_rtnr_sad_offset_reg;
	di_im_di_rtnr_ediff_sobel_th_RBUS im_di_rtnr_ediff_sobel_th_reg;
	di_im_di_rtnr_level_bound_RBUS im_di_rtnr_level_bound_reg;
	di_im_di_rtnr_et_count_y_RBUS im_di_et_count_y_reg;
	di_im_di_rtnr_et_sum_y_RBUS im_di_et_sum_y_reg;
	di_im_di_rtnr_ediff_control_RBUS di_im_di_rtnr_ediff_control_RBUS_reg;
	unsigned char ret_value;

	unsigned int total_pixel;
	//unsigned short Active_Region[4]={0};
	unsigned short IphActWid_H=0,IpvActLen_V=0;

	    // offset table
	unsigned char motion_table[10][10] =
	{
		    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
		    {1, 2, 3, 4, 5, 6, 7, 8, 9, 9},
		    {2, 3, 4, 5, 6, 7, 8, 9, 9, 9},
		    {3, 4, 5, 6, 7, 8, 9, 9, 9, 9},
		    {4, 5, 6, 7, 8, 9, 9, 9, 9, 9},
		    {5, 6, 7, 8, 9, 9, 9, 9, 9, 9},
		    {6, 7, 8, 9, 9, 9, 9, 9, 9, 9},
		    {7, 8, 9, 9, 9, 9, 9, 9, 9, 9},
		    {8, 9, 9, 9, 9, 9, 9, 9, 9, 9},
		    {9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	};

	unsigned char mation_x=0, mation_y=0;
	unsigned int motion_value_1=0, motion_value_2 = 0, motion_diff = 0;




	IphActWid_H=(SmartPic_clue->SCALER_DISP_INFO.IphActWid_H);
	IpvActLen_V=(SmartPic_clue->SCALER_DISP_INFO.IpvActLen_V);
	//drvif_color_activeregion_dragon(_CHANNEL1, &Active_Region[0]);
	total_pixel = IphActWid_H*IpvActLen_V;//Active_Region[1] * Active_Region[3];
	//unsigned short noise_ratio=0, motion_ratio=0, noise_th=0;

	im_di_et_count_y_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_COUNT_Y_reg);
	im_di_et_sum_y_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_SUM_Y_reg);

	im_di_rtnr_sad_offset_reg.regValue = rtd_inl(DI_IM_DI_RTNR_SAD_OFFSET_reg);
	im_di_rtnr_sad_offset_reg.cp_temporal_sad_offset_y = 0;
	rtd_outl(DI_IM_DI_RTNR_SAD_OFFSET_reg, im_di_rtnr_sad_offset_reg.regValue);

	im_di_rtnr_ediff_sobel_th_reg.regValue = rtd_inl(DI_IM_DI_RTNR_EDIFF_SOBEL_TH_reg);
	im_di_rtnr_ediff_sobel_th_reg.cp_temporal_sbl_thm_y = 10;
	im_di_rtnr_ediff_sobel_th_reg.cp_temporal_sbl_ths_y = 10;
	rtd_outl(DI_IM_DI_RTNR_EDIFF_SOBEL_TH_reg, im_di_rtnr_ediff_sobel_th_reg.regValue);

	im_di_rtnr_level_bound_reg.regValue = rtd_inl(DI_IM_DI_RTNR_LEVEL_BOUND_reg);
	im_di_rtnr_level_bound_reg.cp_temporal_nl_low_bnd_y = 0x00;
	im_di_rtnr_level_bound_reg.cp_temporal_nl_up_bnd_y = 0xff;
	rtd_outl(DI_IM_DI_RTNR_LEVEL_BOUND_reg, im_di_rtnr_level_bound_reg.regValue);

	di_im_di_rtnr_ediff_control_RBUS_reg.regValue = rtd_inl(DI_IM_DI_RTNR_EDIFF_CONTROL_reg);
	if(MA_print_count % 2 == 0)
	{
	    et_y_count_1 = im_di_et_count_y_reg.cp_temporal_same_et_cnt_y;
	    et_y_sum_1 = im_di_et_sum_y_reg.cp_temporal_same_et_sad_sum_y;
			total_motion_count = im_di_et_count_y_reg.cp_temporal_same_et_cnt_y;
	    di_im_di_rtnr_ediff_control_RBUS_reg.cp_temporal_num_match_l = 5;
	}
	else
	{
	    et_y_count_2 = im_di_et_count_y_reg.cp_temporal_same_et_cnt_y;
	    et_y_sum_2 = im_di_et_sum_y_reg.cp_temporal_same_et_sad_sum_y;
			noise_flat_motion_count = im_di_et_count_y_reg.cp_temporal_same_et_cnt_y;
	    di_im_di_rtnr_ediff_control_RBUS_reg.cp_temporal_num_match_l = 0;
	}
	rtd_outl(DI_IM_DI_RTNR_EDIFF_CONTROL_reg, di_im_di_rtnr_ediff_control_RBUS_reg.regValue);

#if 0
	noise_ratio = 1000-1000*et_y_count_1/(total_pixel+1);
	motion_ratio = 1000-1000*et_y_count_2/(total_pixel+1);
	edge_motion_count = motion_ratio;

	if(motion_ratio<10)
		noise_th=10;
	else
		noise_th = motion_ratio;

	noise_th = (noise_th*5);

	if(Which_Source==VIP_QUALITY_HDMI_720P||Which_Source==VIP_QUALITY_YPbPr_720P)
	{
		if(Sony_Radio_Caculator_flag==1)
		{
			Off_Correction1_Flag = 0;
		}
		else if(noise_ratio>=noise_th)
		{
			Off_Correction1_Flag = 1;
			Off_Correction1_Flag = 0;
		}
		else
		{
			Off_Correction1_Flag = 0;
		}
	}
	else
	{
		Off_Correction1_Flag = 0;
	}

	if(INR_log==1&&noise_meter_sony_log_flag)
	{
		ROSPrintf("et=%d,%d,%d,%d,%d,%d,%d\n",
			noise_ratio,motion_ratio,noise_th,Off_Correction1_Flag,edge_motion_count,Sony_Radio_Caculator_flag,Which_Source);
	}
	return 0;
#endif
#if 1
	motion_value_1 = (et_y_sum_1 / (et_y_count_1+1));
	total_motion_count_ave = motion_value_1;
	motion_value_2 = (et_y_sum_2 / (et_y_count_2+1));
	noise_flatl_motion_count_ave = motion_value_2;
	motion_diff = ABS(motion_value_1,motion_value_2);
	edge_motion_count_ave = motion_diff;
	edge_motion_count =  ABS(et_y_count_1,et_y_count_2);

	if(Scaler_InputSrcGetMainChType() == _SRC_CVBS)
	{
	    if(motion_value_1 <= 20)               //底noise 高
	            mation_x = 0;
	    else if(motion_value_1 <= 40)
	            mation_x = 1;
	    else if(motion_value_1 <= 60)
	            mation_x = 2;
	    else if(motion_value_1 <= 80)
	            mation_x = 3;
	    else if(motion_value_1 <= 100)
	            mation_x = 4;
	    else if(motion_value_1 <= 120)
	            mation_x = 5;
	    else if(motion_value_1 <= 140)
	            mation_x = 6;
	    else
	            mation_x = 7;

	    if(motion_diff <= 5)
	            mation_y = 0;
	    else if(motion_diff <= 10)
	            mation_y = 1;
	    else if(motion_diff <= 15)
	            mation_y = 2;
	    else if(motion_diff <= 20)
	            mation_y = 3;
	    else if(motion_diff <= 25)
	            mation_y = 4;
	    else if(motion_diff <= 30)
	            mation_y = 5;
	    else if(motion_diff <= 35)
	            mation_y = 6;
	    else
	            mation_y = 7;

	}
	else
	{
		if(motion_value_1 <= 35)			   //底noise 高
				mation_x = 0;
		else if(motion_value_1 <= 50)
				mation_x = 1;
		else if(motion_value_1 <= 65)
				mation_x = 2;
		else if(motion_value_1 <= 85)
				mation_x = 3;
		else if(motion_value_1 <= 105)
				mation_x = 4;
		else if(motion_value_1 <= 135)
				mation_x = 5;
		else if(motion_value_1 <= 175)
				mation_x = 6;
		else if(motion_value_1 <= 215)
				mation_x = 7;
		else if(motion_value_1 <= 260)
				mation_x = 8;
		else
				mation_x = 9;

		if(motion_diff <= 15)
				mation_y = 0;
		else if(motion_diff <= 25)
				mation_y = 1;
		else if(motion_diff <= 35)
				mation_y = 2;
		else if(motion_diff <= 50)
				mation_y = 3;
		else if(motion_diff <= 65)
				mation_y = 4;
		else if(motion_diff <= 80)
				mation_y = 5;
		else if(motion_diff <= 95)
				mation_y = 6;
		else if(motion_diff <= 115)
				mation_y = 7;
		else if(motion_diff <= 140)
				mation_y = 8;
		else
				mation_y = 9;

	}
	ret_value = motion_table[mation_y][mation_x];

#if 0
	if(INR_log==1&&noise_meter_sony_log_flag)
	{
	  ROSPrintf("et=%d,%d,%d,%d,\n",
		  total_motion_count_ave,noise_flatl_motion_count_ave,edge_motion_count_ave,edge_motion_count);
	}
#endif
	return motion_table[mation_y][mation_x];
#endif
}

unsigned short scalerVIP_ratio_inner_product_base(unsigned int *vector1, unsigned int *vector2, unsigned char length)
{

	unsigned int inner_product=0;
	unsigned char i;
	for(i=0;i<length;i++)
		inner_product += DCCsqrt_table[*(vector1+i)] * DCCsqrt_table[*(vector2+i)];
	return (inner_product/1000)>>10;// permillage = 1000;
}

unsigned short scalerVIP_ratio_inner_product_base_16(unsigned short *vector1, unsigned short *vector2, unsigned char length)
{

	unsigned int inner_product=0;
	unsigned char i;
	for(i=0;i<length;i++)
		inner_product += DCCsqrt_table[*(vector1+i)] * DCCsqrt_table[*(vector2+i)];
	return (inner_product/1000)>>10;// permillage = 1000;
}

unsigned short scalerVIP_ratio_inner_product(unsigned short *vector1, unsigned short *vector2, unsigned char length)
{
	unsigned int inner_product1=0, inner_product2=0;
	unsigned short even1, even2, odd1, odd2;
	unsigned char i;

	if ((!vector1)||(!vector2))
		return 0;

	if(length>32)
		length=32;

	length &= 0xfe; //length must be even

	for(i=0;i<length;i+=2) {
		//even
		even1=vector1[i]+vector1[i+1];
		even2=vector2[i]+vector2[i+1];
		inner_product1 += DCCsqrt_table[even1] * DCCsqrt_table[even2];

		//odd
		if (unlikely(i==0)) {
			odd1= vector1[0];
			odd2= vector2[0];
		} else {
			odd1= vector1[i-1]+ vector1[i];
			odd2= vector2[i-1]+ vector2[i];
		}
		inner_product2 += DCCsqrt_table[odd1] * DCCsqrt_table[odd2];
	}

	if(inner_product1 >inner_product2)
		return (inner_product1/1000)>>10;
	else
		return (inner_product2/1000)>>10;
}

unsigned short scalerVIP_ratio_inner_product2(unsigned short *vector1, unsigned short *vector2, unsigned char length)
{
	unsigned int inner_product1=0;
	unsigned int inner_product2=0;
	unsigned char i;
	unsigned int inner_product3=0;
	unsigned int inner_product4=0;

	unsigned short even1[16];
	unsigned short even2[16];

	unsigned short odd1[17];
	unsigned short odd2[17];

	for(i=0;i<length/2;i++)
	{
		even1[i]=( *(vector1+i*2)+ *(vector1+1+i*2));

		even2[i]=( *(vector2+i*2)+ *(vector2+1+i*2));

	//ROSPrintf("even2[%d]=%d,inner_product4=%d \n",inner_product3,inner_product4);


	}


	for(i=0;i<length/2;i++)
		inner_product1 += DCCsqrt_table[*(even1+i)] * DCCsqrt_table[*(even2+i)];

	//return (inner_product/1000)>>10;// permillage = 1000;


	for(i=0;i<length/2+1;i++)
	{
		if(i==0)
		{
			odd1[i]= *(vector1);
			odd2[i]=*(vector2);

		}
		else if(i==length/2)
		{
			odd1[i]= *(vector1+i*2-1);
			odd2[i]= *(vector2+i*2-1);

		}
		else
		{
			odd1[i]=( *(vector1+i*2-1)+ *(vector1+i*2));
			odd2[i]=( *(vector2+i*2-1)+ *(vector2+i*2));
		}
	}



/*
i=0,data1=0 ,data2=d
i=1,data1=135 ,data2=d
i=2,data1=2 ,data2=d
i=3,data1=134 ,data2=d
i=4,data1=135 ,data2=d
i=5,data1=1 ,data2=d
i=6,data1=134 ,data2=d
i=7,data1=135 ,data2=d
i=8,data1=1 ,data2=d
i=9,data1=133 ,data2=d
i=10,data1=1 ,data2=d
i=11,data1=1 ,data2=d
i=12,data1=0 ,data2=d
i=13,data1=1 ,data2=d
i=14,data1=92 ,data2=d
i=15,data1=2 ,data2=

//No.192 ColorBar
unsigned short DynamicOptimizeSystem_192_y[][32]=
{ //#0 cvbs ntsc , #1 cvbs pal  ,#2 hdmi_sd , #3 hdmi_hd
	{12, 44, 84, 13, 127, 0, 0, 1, 136, 0, 119, 0, 0, 1, 103, 0, 119, 0, 0, 1, 103, 0, 82, 0, 0, 0, 0, 0, 0, 44, 0, 0, },
	{0, 77, 5, 3, 132, 1, 1, 7, 127, 2, 133, 0, 1, 128, 6, 1, 134, 0, 1, 2, 131, 1, 0, 0, 1, 0, 0, 0, 1, 92, 0, 0, },
	//{0, 79, 4, 2, 133, 1, 1, 4, 129, 2, 133, 1, 1, 5, 130, 1, 133, 1, 0, 3, 132, 0, 0, 1, 0, 0, 0, 1, 0, 94, 0, 0, }


	{0, 0, 108, 2, 125, 1, 0, 8, 119, 2, 127, 1, 0, 127, 2, 2, 125, 1, 0, 127, 1, 0, 0, 0, 1, 0, 0, 1, 108, 0, 0, 0, }
};

*/

	for(i=0;i<length/2;i++)
	{
		inner_product2 += DCCsqrt_table[*(odd1+i)] * DCCsqrt_table[*(odd2+i)];

		ROSPrintf("i=%d,data1=%d ,data2=%d \n",i,*(odd1+i),*(odd2+i));


	}
	inner_product3= (inner_product1/1000)>>10;
	inner_product4= (inner_product2/1000)>>10;


	//if(MA_print_count%40==0)
	//ROSPrintf("inner_product3=%d,inner_product4=%d \n",inner_product3,inner_product4);


	return 1;

	//return (inner_product/1000)>>10;// permillage = 1000;












}







unsigned short scalerVIP_ratio_difference(unsigned short *vector1, unsigned short *vector2, unsigned char length)
{
	unsigned int difference=0;
	unsigned char i;
	for(i=0;i<length;i++)
	{
		if(i == 0)
			difference += MIN((ABS(*(vector1+i), *(vector2+i))), (ABS(*(vector1+i), *(vector2+i+1))));
		else if(i == (length-1))
			difference += MIN((ABS(*(vector1+i), *(vector2+i))), (ABS(*(vector1+i), *(vector2+i-1))));
		else
			difference += MIN(MIN((ABS(*(vector1+i), *(vector2+i))), (ABS(*(vector1+i), *(vector2+i+1)))), (ABS(*(vector1+i), *(vector2+i-1))));
	}
	return difference;
}
#endif

#ifdef ISR_AMBILIGHT_ENABLE
void scalerVIP_Clip_Into_Range(unsigned char nBit,int * nData1 ,int * nData2,int * nData3)
{
	int nRange=1;
	int nDiff=0,nMax=0;
	nRange = nRange<<nBit ;
	nRange = nRange -1 ;

	//find Max.
	nMax=*nData1;
	if(*nData2 > nMax)
		nMax=*nData2;
	if(*nData3 > nMax)
		nMax=*nData3;

	//offset
	if(nMax>nRange)
	{
		nDiff=nMax-nRange;
		*nData1 = *nData1-nDiff;
		*nData2 = *nData2-nDiff;
		*nData3 = *nData3-nDiff;
	}
	// clip into Range
	*nData1 = (*nData1>nRange)? nRange:(*nData1<0)? 0:*nData1;
	*nData2 = (*nData2>nRange)? nRange:(*nData2<0)? 0:*nData2;
	*nData3 = (*nData3>nRange)? nRange:(*nData3<0)? 0:*nData3;

}

void scalerVIP_yuv2rgb_convert(void)
{
	unsigned short k11 = 1024, k13 = 1432, k22 = 352, k23 = 728, k32 = 1802; // mutiply by 1024
	int R,G,B;
	unsigned char i = 0;
	#if AMBI_DEBUG_TPV
	static unsigned char yuv2rgb_count = 0;
	yuv2rgb_count++;
	#endif
	unsigned short UV_Value=512;

	for(i=0;i<16;i++)
	{
		R = (INT32)(k11*Profile_Y_V[i] + k13*(Profile_V_V[i]-UV_Value));
		G = (INT32)(k11*Profile_Y_V[i] - k22*(Profile_U_V[i]-UV_Value) - k23*(Profile_V_V[i]-UV_Value));
		B = (INT32)(k11*Profile_Y_V[i] + k32*(Profile_U_V[i]-UV_Value));
		#if AMBI_DEBUG_TPV
		if((yuv2rgb_count % 60) == 0)
		{
		    ROSPrintf("\n [ambi_start] Profile_Y_V[%d]=%d, Profile_U_V[%d]=%d,Profile_V_V[%d]=%d",i,Profile_Y_V[i],i,Profile_U_V[i],i,Profile_V_V[i]);
			ROSPrintf("\n [ambi] R=%d, G=%d,B=%d",R,G,B);
		}
		#endif
		R = R>>10;
		G = G>>10;
		B = B>>10;
		#if AMBI_DEBUG_TPV
		if((yuv2rgb_count % 60) == 0)
		{
			ROSPrintf("\n [ambi] shift R=%d, G=%d,B=%d",R,G,B);
		}
		#endif

		// clip into 10 bit
		scalerVIP_Clip_Into_Range(10,&R,&G,&B);
		#if AMBI_DEBUG_TPV
		if((yuv2rgb_count % 60) == 0)
		{
			ROSPrintf("\n [ambi] 10 bit R=%d, G=%d,B=%d",R,G,B);
		}
		#endif
		Profile_R_V[i] = (unsigned short)R;
		Profile_G_V[i] = (unsigned short)G;
		Profile_B_V[i] = (unsigned short)B;
		R = (INT32)(k11*Profile_Y_H[i] + k13*(Profile_V_H[i]-UV_Value));
		G = (INT32)(k11*Profile_Y_H[i] - k22*(Profile_U_H[i]-UV_Value) - k23*(Profile_V_H[i]-UV_Value));
		B = (INT32)(k11*Profile_Y_H[i] + k32*(Profile_U_H[i]-UV_Value));
		#if AMBI_DEBUG_TPV
		if((yuv2rgb_count % 60) == 0)
		{
		    ROSPrintf("\n [ambi] Profile_Y_H[%d]=%d, Profile_U_H[%d]=%d,Profile_V_H[%d]=%d",i,Profile_Y_H[i],i,Profile_U_H[i],i,Profile_V_H[i]);
			ROSPrintf("\n [ambi] R=%d, G=%d,B=%d",R,G,B);
		}
		#endif
		R = R>>10;
		G = G>>10;
		B = B>>10;
		#if AMBI_DEBUG_TPV
		if((yuv2rgb_count % 60) == 0)
		{
			ROSPrintf("\n [ambi] shift R=%d, G=%d,B=%d",R,G,B);
		}
		#endif

		// clip into 10 bit
		scalerVIP_Clip_Into_Range(10,&R,&G,&B);
		#if AMBI_DEBUG_TPV
		if((yuv2rgb_count % 60) == 0)
		{
			ROSPrintf("\n [ambi] 10 bit R=%d, G=%d,B=%d",R,G,B);
		}
		#endif
		Profile_R_H[i] = R;
		Profile_G_H[i] = G;
	        Profile_B_H[i] = B;
	}
}
#endif

#if CB_flag
void scalerVIP_Identify_Pattern_forCB(void)
{
	unsigned short hue_ratio=0;
	unsigned short y_ratio=0;
	unsigned short hue_ratio2=0;
	unsigned short y_ratio2=0;
	//unsigned short hue_ratio3=0;
	//unsigned short y_ratio3=0;
	//unsigned short hue_ratio4=0;
	//unsigned short y_ratio4=0;
	//unsigned short hue_ratio5=0;
	//unsigned short y_ratio5=0;
	//unsigned short hue_ratio6=0;
	//unsigned short y_ratio6=0;


	unsigned char i;
	//sat_hist_ratio
//	unsigned int noise_status = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL);
	//unsigned int noise_relaible = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL_RELIABLE);
	//unsigned int motion = rtd_inl(DI_IM_DI_SI_FILM_MOTION_reg);
	//unsigned char motionl_level = scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION);
	unsigned char cur_source=(RPC_system_info_structure_table->VIP_source);
	static unsigned char pre_source=255;

	for(i=0;i<20;i++)//for_CB
	{
		ID_CB_NO[i]=0;
	}

	pre_source = cur_source;

	switch(cur_source)
	{
		case VIP_QUALITY_CVBS_NTSC:
		{
			//=======      TSB3_CH3   =======
			//TSB3_CH3
			y_ratio = scalerVIP_ratio_inner_product(CB_TSB3_CH3_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio = scalerVIP_ratio_inner_product(CB_TSB3_CH3_hue1[0], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_2==TRUE))
				ROSPrintf("CB_TSB3_CH3_y1=%d,CB_TSB3_CH3_hue1=%d \n",y_ratio,hue_ratio);

			//TSB3_CH3 7.5 IRE
			y_ratio2 = scalerVIP_ratio_inner_product(CB_TSB3_CH3_y1[1], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio2 = scalerVIP_ratio_inner_product(CB_TSB3_CH3_hue1[1], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_2==TRUE))
				ROSPrintf("CB_TSB3_CH3_y1 2=%d,CB_TSB3_CH3_hue1 2=%d \n",y_ratio2,hue_ratio2);


			if((y_ratio >=990 && hue_ratio >=990)||(y_ratio2 >=990 && hue_ratio2 >=990))
				ID_CB_NO[0]=1;
			else
				ID_CB_NO[0]=0;
//==============================================================================================//
			//TSB3_CH3
			y_ratio = scalerVIP_ratio_inner_product(CB_TSB3_CH3_y2[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio = scalerVIP_ratio_inner_product(CB_TSB3_CH3_hue2[0], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_2==TRUE))
				ROSPrintf("CB_TSB3_CH3_y2=%d,CB_TSB3_CH3_hue2=%d \n",y_ratio,hue_ratio);

			//TSB3_CH3 7.5 IRE
			y_ratio2 = scalerVIP_ratio_inner_product(CB_TSB3_CH3_y2[1], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio2 = scalerVIP_ratio_inner_product(CB_TSB3_CH3_hue2[1], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_2==TRUE))
				ROSPrintf("CB_TSB3_CH3_y2 2=%d,CB_TSB3_CH3_hue2 2=%d \n",y_ratio2,hue_ratio2);


			if((y_ratio >=990 && hue_ratio >=990)||(y_ratio2 >=990 && hue_ratio2 >=990))
				ID_CB_NO[1]=1;
			else
				ID_CB_NO[1]=0;

			//TSB3_CH3
			y_ratio = scalerVIP_ratio_inner_product(CB_TSB3_CH3_y3[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio = scalerVIP_ratio_inner_product(CB_TSB3_CH3_hue3[0], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_2==TRUE))
				ROSPrintf("CB_TSB3_CH3_y3=%d,CB_TSB3_CH3_hue3=%d \n",y_ratio,hue_ratio);

			//TSB3_CH3 7.5 IRE
			y_ratio2 = scalerVIP_ratio_inner_product(CB_TSB3_CH3_y3[1], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio2 = scalerVIP_ratio_inner_product(CB_TSB3_CH3_hue3[1], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_2==TRUE))
				ROSPrintf("CB_TSB3_CH3_y3 2=%d,CB_TSB3_CH3_hue3 2=%d \n",y_ratio2,hue_ratio2);


			if((y_ratio >=990 && hue_ratio >=990)||(y_ratio2 >=990 && hue_ratio2 >=990))
				ID_CB_NO[2]=1;
			else
				ID_CB_NO[2]=0;
			//========================

			//=======      SONY_CH84   =======
			//SONY_CH84
			y_ratio = scalerVIP_ratio_inner_product(CB_SONY_CH84_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio = scalerVIP_ratio_inner_product(CB_SONY_CH84_hue1[0], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_4==TRUE))
				ROSPrintf("CB_SONY_CH84_y1=%d,CB_SONY_CH84_hue1=%d \n",y_ratio,hue_ratio);

			//SONY_CH84 7.5 IRE
			y_ratio2 = scalerVIP_ratio_inner_product(CB_SONY_CH84_y1[1], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio2 = scalerVIP_ratio_inner_product(CB_SONY_CH84_hue1[1], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_4==TRUE))
				ROSPrintf("CB_SONY_CH84_y1 2=%d,CB_SONY_CH84_hue1 2=%d \n",y_ratio2,hue_ratio2);


			if((y_ratio >=980 && hue_ratio >=980)||(y_ratio2 >=980 && hue_ratio2 >=980))
				ID_CB_NO[3]=1;
			else
				ID_CB_NO[3]=0;
//==============================================================================================//
			//SONY_CH84
			y_ratio = scalerVIP_ratio_inner_product(CB_SONY_CH84_y2[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio = scalerVIP_ratio_inner_product(CB_SONY_CH84_hue2[0], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_4==TRUE))
				ROSPrintf("CB_SONY_CH84_y2=%d,CB_SONY_CH84_hue2=%d \n",y_ratio,hue_ratio);

			//SONY_CH84 7.5 IRE
			y_ratio2 = scalerVIP_ratio_inner_product(CB_SONY_CH84_y2[1], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio2 = scalerVIP_ratio_inner_product(CB_SONY_CH84_hue2[1], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_4==TRUE))
				ROSPrintf("CB_SONY_CH84_y2 2=%d,CB_SONY_CH84_hue2 2=%d \n",y_ratio2,hue_ratio2);


			if((y_ratio >=980 && hue_ratio >=980)||(y_ratio2 >=980 && hue_ratio2 >=980))
				ID_CB_NO[4]=1;
			else
				ID_CB_NO[4]=0;

			//========================

			//=======     USB_Red_Flower_Bee   =======
			//USB_Red_Flower_Bee
			y_ratio = scalerVIP_ratio_inner_product(CB_USB_Red_Flower_Bee_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio = scalerVIP_ratio_inner_product(CB_USB_Red_Flower_Bee_hue1[0], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_5==TRUE))
				ROSPrintf("CB_USB_Red_Flower_Bee_y1=%d,CB_USB_Red_Flower_Bee_hue1=%d \n",y_ratio,hue_ratio);

			//USB_Red_Flower_Bee 7.5 IRE
			y_ratio2 = scalerVIP_ratio_inner_product(CB_USB_Red_Flower_Bee_y1[1], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio2 = scalerVIP_ratio_inner_product(CB_USB_Red_Flower_Bee_hue1[1], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_5==TRUE))
				ROSPrintf("CB_USB_Red_Flower_Bee_y1 2=%d,CB_USB_Red_Flower_Bee_hue1 2=%d \n",y_ratio2,hue_ratio2);


			if((y_ratio >=980 && hue_ratio >=980)||(y_ratio2 >=980 && hue_ratio2 >=980))
				ID_CB_NO[5]=1;
			else
				ID_CB_NO[5]=0;
//==============================================================================================//
			//USB_Red_Flower_Bee
			y_ratio = scalerVIP_ratio_inner_product(CB_USB_Red_Flower_Bee_y2[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio = scalerVIP_ratio_inner_product(CB_USB_Red_Flower_Bee_hue2[0], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_5==TRUE))
				ROSPrintf("CB_USB_Red_Flower_Bee_y2=%d,CB_USB_Red_Flower_Bee_hue2=%d \n",y_ratio,hue_ratio);

			//USB_Red_Flower_Bee 7.5 IRE
			y_ratio2 = scalerVIP_ratio_inner_product(CB_USB_Red_Flower_Bee_y2[1], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
			hue_ratio2 = scalerVIP_ratio_inner_product(CB_USB_Red_Flower_Bee_hue2[1], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_5==TRUE))
				ROSPrintf("CB_USB_Red_Flower_Bee_y2 2=%d,CB_USB_Red_Flower_Bee_hue2 2=%d \n",y_ratio2,hue_ratio2);


			if((y_ratio >=980 && hue_ratio >=980)||(y_ratio2 >=980 && hue_ratio2 >=980))
				ID_CB_NO[6]=1;
			else
				ID_CB_NO[6]=0;

			//========================

			break;

		}
		case VIP_QUALITY_CVBS_PAL:
		{

			break;
		}
		case VIP_QUALITY_HDMI_480I:
		case VIP_QUALITY_HDMI_576I:
		case VIP_QUALITY_HDMI_480P:
		case VIP_QUALITY_HDMI_576P:
		{


			break;
		}

		case VIP_QUALITY_HDMI_1080I:
		case VIP_QUALITY_HDMI_1080P:
		{


			break;
		}
		default:

			break;



	}

}

void scalerVIP_Set_Parameter_forCB(void)
{
	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;
	RPC_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();

	if((system_info_structure_table == NULL))
	{
		if(MA_print_count%360==0)
			ROSPrintf("Share Memory PQ data ERROR, RPC_system_info = %x, \n",RPC_system_info_structure_table);
		return;
	}

	unsigned char cur_source=(RPC_system_info_structure_table->VIP_source);
	static unsigned char pre_source =255;

    //=======================================   init rbus
	di_im_di_control_RBUS di_control_reg;
	color_sharp_dm_segpk_vpk2_RBUS dm_segpk_vpk2;

	//======================================      read register
	//di_control_reg.regValue = rtd_inl(DI_IM_DI_CONTROL_reg ); //rord.tsao mark don't change di seeting avoid 3a/5a setting error 2016/02/19
	dm_segpk_vpk2.regValue = rtd_inl(COLOR_SHARP_DM_SEGPK_VPK2_reg );

	//============================================   init old value
	static unsigned char ori_ma_controlmode=0;
	static unsigned char ori_vpk_lv=0;

	//========================================= record old value
	if(pre_source !=cur_source)
	{
		ori_ma_controlmode=di_control_reg.ma_controlmode;
		ori_vpk_lv=dm_segpk_vpk2.vpk_lv;
	}

	//==========================================
	pre_source = cur_source;

	switch(cur_source)
	{
		case VIP_QUALITY_CVBS_NTSC:
		if(!vd_pwr_ok)
			break;
       //============   TSB3_CH3  ===========
		if( (ID_CB_NO[0]==1) || (ID_CB_NO[1]==1) || (ID_CB_NO[2]==1) )
		{
			di_control_reg.ma_controlmode = 2;
			dm_segpk_vpk2.vpk_lv = 77;
		}
		//else
		else if( DynamicOptimizeSystem[139] ==0  )
		{
			di_control_reg.ma_controlmode =ori_ma_controlmode;
			dm_segpk_vpk2.vpk_lv =ori_vpk_lv;

		}
		rtd_outl(COLOR_SHARP_DM_SEGPK_VPK2_reg, dm_segpk_vpk2.regValue);
		//rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);//rord.tsao mark don't change di seeting avoid 3a/5a setting error 2016/02/19

		//==================================================

		//============   USB_Red_Flower_Bee  ===========
        if( (ID_CB_NO[5]==1) || (ID_CB_NO[6]==1) )
        scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_CB_5_6);

		//==================================

		break;
		default:

			break;
	}

}
#endif

void scalerVIP_Identify_Pattern_forLG_K2L(void)
{
	//unsigned short hue_ratio=0;
	//unsigned short y_ratio=0;

	unsigned char i;
	unsigned char cur_source=(RPC_system_info_structure_table->VIP_source);
	static unsigned char pre_source=255;

	for(i=0;i<200;i++)//for_LG_demo
	{
		if(i!=123)
		DynamicOptimizeSystem[i]=0;
	}

	pre_source = cur_source;

	switch(cur_source)
	{
		case VIP_QUALITY_TV_NTSC:
		{
//==============================================================================================//
		/*	//RF NTSC flower identify, roger RF color bar issue
			y_ratio = scalerVIP_ratio_inner_product(DynamicOptimizeSystem_179_y[0], &Y_hist_ratio[0], 32);
			hue_ratio = scalerVIP_ratio_inner_product(DynamicOptimizeSystem_179_hue[0], &hue_hist_ratio[0], 24);
			if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.RF_flower==TRUE))
				ROSPrintf("RF_flower_y=%d,RF_flower_hue=%d \n",y_ratio,hue_ratio);

			if(y_ratio >=990 && hue_ratio >=990)
				DynamicOptimizeSystem[179]=1;
			else
				DynamicOptimizeSystem[179]=0;
		*/ //rord mark 2015/0827
//==============================================================================================//



		}

		default:

			break;
	}

}


void scalerVIP_Set_Parameter_forLG_K2L(void)
{
	/*//mac7p pq compile fix
	vdtop_auto_pos_ctrl_RBUS auto_pos_ctrl_reg;
	static unsigned char ori_VD_Cr_delay = 0;
	static unsigned char pre_source =255;
	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;

	unsigned char cur_source=0;
	RPC_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();

	if((RPC_system_info_structure_table == NULL))
	{
		if(MA_print_count%360==0)
			ROSPrintf("Share Memory PQ data ERROR, RPC_system_info = %p, \n",RPC_system_info_structure_table);
		return;
	}

	cur_source=RPC_system_info_structure_table->VIP_source;

	if (cur_source <= VIP_QUALITY_TV_PAL) {
		if (vd_pwr_ok)
			auto_pos_ctrl_reg.regValue = VDPQ_rtd_inl(VDTOP_AUTO_POS_CTRL_reg);
	}

	if(pre_source !=cur_source)
	{
		ori_VD_Cr_delay = auto_pos_ctrl_reg.cr1delay;
	}
	*/
	#if 0	//20151019 roger mark, no need and make cr1delay error when channel change
	switch(cur_source) //20150811 roger, RF flower
	{
		case VIP_QUALITY_TV_NTSC:
		if (!vd_pwr_ok)
			return;

		auto_pos_ctrl_reg.regValue = rtd_inl(VDTOP_AUTO_POS_CTRL_reg);

		if(DynamicOptimizeSystem[179] ==1)
		{

			auto_pos_ctrl_reg.cr1delay = 0x8;
		}
		else
		{

			auto_pos_ctrl_reg.cr1delay=ori_VD_Cr_delay;

		}



		rtd_outl(VDTOP_AUTO_POS_CTRL_reg, auto_pos_ctrl_reg.regValue);
		break;

		default:

			break;

	}
	#endif

}



/*#ifdef LG_Demo_flag*/
void scalerVIP_Set_Parameter_forLGM2(unsigned char apply)
{

	di_di_smd_waterflag_ctr3_RBUS di_di_smd_waterflag_ctr3;
	//di_di_smd_resurrect_ctrl_RBUS di_di_smd_resurrect_ctrl; //mac5p removed

	static di_di_smd_waterflag_ctr3_RBUS ori_di_di_smd_waterflag_ctr3;
	//static di_di_smd_resurrect_ctrl_RBUS ori_di_di_smd_resurrect_ctrl; //mac5p removed
	di_di_smd_choosecandidate_RBUS 	smd_choosecandidate_reg;
	di_di_smd_refinemv3_RBUS smd_refinemv3_reg;
	di_di_smd_compensate_RBUS smd_compensate_reg;
	di_di_smd_candidatelimitsad_RBUS init_di_smd_candidatelimitsad_reg;
	di_di_smd_additionalcoeff2_RBUS init_di_smd_additionalcoeff2_reg;
	di_di_smd_dummy_RBUS di_smd_dummy_reg;
	di_di_smd_refinemv2_RBUS di_smd_refinemv2_reg;

	di_di_smd_pan1_RBUS init_di_smd_pan1_reg;
	di_di_smd_hardwarepan_modify1_RBUS di_di_smd_hardwarepan_modify1;
	di_di_smd_sampling_gmvc_RBUS di_di_smd_sampling_gmvc;
	di_di_gst_compensate_RBUS di_di_gst_compensate;

	di_di_smd_additionalcoeff1_RBUS init_di_smd_additionalcoeff1_reg;
	di_di_smd_slow_motion_handle1_RBUS di_di_smd_slow_motion_handle1;
	di_di_smd_slow_motion_handle2_RBUS di_di_smd_slow_motion_handle2;

	di_im_di_weave_window_control_RBUS im_di_weave_window_control_reg;
	di_im_di_weave_window_1_start_RBUS im_di_weave_window_1_start_reg;
	di_im_di_weave_window_1_end_RBUS im_di_weave_window_1_end_reg;

	static 	di_im_di_weave_window_control_RBUS ORI_im_di_weave_window_control_reg;
	static	di_im_di_weave_window_1_start_RBUS ORI_im_di_weave_window_1_start_reg;
	static	di_im_di_weave_window_1_end_RBUS ORI_im_di_weave_window_1_end_reg;

	di_im_di_nintra_lowpass_source_RBUS di_im_di_nintra_lowpass_source;
	di_im_di_nintra_search_dir_limit_RBUS di_im_di_nintra_search_dir_limit;
	di_di_gst_ctrl_RBUS init_di_gst_ctrl_reg;

#ifdef CONFIG_HW_SUPPORT_MEMC
	memc_mux_memc_mux_di_ctrl_RBUS color_temp_ctrl_reg;
#endif //CONFIG_HW_SUPPORT_MEMC

	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	nr_curve_mapping_w1_1_RBUS curve_mapping_w1_1_reg;
	nr_curve_mapping_w1_2_RBUS curve_mapping_w1_2_reg;
	nr_curve_mapping_w1_3_RBUS curve_mapping_w1_3_reg;


	nr_curve_mapping_w2_1_RBUS curve_mapping_w2_1_reg;
	nr_curve_mapping_step_2_RBUS curve_mapping_step2_reg;
	nr_curve_mapping_step_3_RBUS curve_mapping_step3_reg;
	nr_mosquito_nr_level_ctrl_RBUS nr_mosquito_nr_level_ctrl_RBUS_reg;
	nr_mosquito_nr_level_ctrl_1_RBUS nr_mosquito_nr_level_ctrl_1_RBUS_reg;
	scaleup_dm_uzu_ctrl_RBUS dm_uzu_Ctrl_reg;
	di_im_di_control_RBUS di_control_reg;
	di_im_di_control_RBUS di_control_reg_Prtoect_3AEnable;

	color_sharp_dm_segpk_edgpk3_RBUS color_sharp_dm_segpk_edgpk3;
	color_sharp_dm_peaking_bound_1_RBUS color_sharp_dm_peaking_bound_1_reg;
//	color_sharp_dm_peaking_bound_0_RBUS color_sharp_dm_peaking_bound_0;
//	color_sharp_dm_segpk_edgpk2_RBUS color_sharp_dm_segpk_edgpk2;

	color_sharp_dm_cds_cm0_u_bound_0_RBUS color_sharp_dm_cds_cm0_u_bound_0_reg;
	nr_dch1_spatial_weight_RBUS dch1_Spatial_Weight_Reg;
	//vdpq_yc_sep_control_RBUS YC_SEP_CONTROL_reg;
 	//vdpq_yc_bw_ctrl_RBUS YC_BW_CTRL_reg;
	di_rtnr_ma_snr_strength_control_RBUS	di_rtnr_ma_snr_strength_control;
	color_sharp_dm_peaking_gain_RBUS color_sharp_dm_peaking_gain_reg;

	di_im_di_hmc_adjustable_RBUS di_hmc_adjustable_reg;
	static di_im_di_hmc_adjustable_RBUS ORI_di_hmc_adjustable_reg;

	di_im_di_hmc_adjustable2_RBUS di_hmc_adjustable2_reg;
	static di_im_di_hmc_adjustable2_RBUS ORI_di_hmc_adjustable2_reg;

	di_im_di_hmc_adjustable3_RBUS di_hmc_adjustable3_reg;
	static di_im_di_hmc_adjustable3_RBUS ORI_di_hmc_adjustable3_reg;

	di_im_di_hmc_adjustable4_RBUS di_hmc_adjustable4_reg;
	static di_im_di_hmc_adjustable4_RBUS ORI_di_hmc_adjustable4_reg;

	di_im_di_hmc_pan_control_par_2_RBUS di_hmc_pan_control_par_2_reg;
	di_im_di_hmc_pan_control_par_1_RBUS di_hmc_pan_control_par_1_reg;
	di_im_di_hmc_statistic1_RBUS di_hmc_statistic1_reg;

	di_im_di_framesobel_statistic_RBUS FrameSobel_statistic_Reg;
	static di_im_di_framesobel_statistic_RBUS ORI_FrameSobel_statistic_Reg;


	nr_curve_mapping_w2_2_RBUS curve_mapping_w2_2_reg;

	di_im_di_intra_th_RBUS intra_th_Reg;

	scaleup_dm_mem_crc_ctrl_RBUS dm_mem_crc_ctrl_RBUS_reg;
	//di_im_di_ma_frame_motion_th_a_RBUS frame_motion_a_reg;// frame motion a
	color_sharp_dm_segpk_vpk2_RBUS dm_segpk_vpk2;

	di_im_di_rtnr_new_control_RBUS rtnr_new_Ctrl;

	idcti_i_dcti_ctrl_2_RBUS ich1_newdcti_reg_2;
	idcti_i_dcti_ctrl_1_RBUS ich1_newdcti_reg_1;
	color_sharp_shp_dcti_ctrl_1_RBUS dm_dcti_REG_1;
	color_sharp_shp_dcti_ctrl_2_RBUS dm_dcti_REG_2;

	//vdpq_adap_bpf_y_th3_RBUS vdpq_adap_bpf_y_th3_reg;

	scaleup_dm_uzu_ctrl_RBUS dm_uzu_Ctrl;

	scaleup_dm_uzumain_vcti_lpf_RBUS dm_uzumain_vcti_lpf_reg;

	//vdtop_auto_pos_ctrl_RBUS auto_pos_ctrl_reg;//mac7p pq compile fix

	di_im_new_mcnr_control_RBUS di_im_new_mcnr_control;

	mpegnr_ich1_mpegnr2_RBUS ich1_MPEGNR2_reg;
	//di_im_new_mcnr_control4_RBUS di_im_new_mcnr_control4; //mac6 used old mcnr
	di_im_new_mcnr_pan_condition_RBUS di_im_new_mcnr_pan_condition;

	di_im_di_ma_frame_motion_th_a_RBUS di_ma_frame_motion_th_a_reg;
	di_im_di_ma_frame_motion_th_b_RBUS di_ma_frame_motion_th_b_reg;
	di_im_di_ma_frame_motion_th_c_RBUS di_ma_frame_motion_th_c_reg;
	di_im_di_nintra_control_RBUS di_im_di_nintra_control;
	di_di_smd_control_candidatelist_RBUS di_smd_control_candiate_reg;

	//di_im_di_film_mask_detection_RBUS film_mask_detection_reg; //mac6 remove film_mask_detection
#if 0
	di_im_di_hmc_pan_dummy_2_RBUS di_pan_dummy2_reg;
	di_im_di_hmc_pan_dummy_3_RBUS di_pan_dummy3_reg;
#else // henry Merlin3 temp
	di_im_di_rtnr_refined_ctrl2_RBUS di_im_di_rtnr_refined_ctrl2_reg;
	di_di_smd_dummy_RBUS di_di_smd_dummy_reg;
#endif

	di_im_di_btr_control_RBUS di_im_di_btr_control_reg;
	static di_im_di_rtnr_c_th0_th3_RBUS di_im_di_rtnr_c_th0_th3_RBUS_reg_save;
	di_im_di_rtnr_c_th0_th3_RBUS di_im_di_rtnr_c_th0_th3_RBUS_reg;
	static di_im_di_rtnr_c_th4_th6_RBUS di_im_di_rtnr_c_th4_th6_RBUS_reg_save;
	di_im_di_rtnr_c_th4_th6_RBUS di_im_di_rtnr_c_th4_th6_RBUS_reg;


	vdpq_postp_secam_fix_ctrl_RBUS postp_secam_fix_ctrl_reg = {0};
	vdpq_postp_secam_fix_y_detect0_RBUS postp_secam_fix_y_detect0_reg = {0};
	vdpq_postp_secam_fix_y_detect1_RBUS postp_secam_fix_y_detect1_reg = {0};

	scaleup_dm_dir_uzu_conf_RBUS dm_dir_uzu_conf_Reg;

	di_im_di_hmc_me_refine_ctrl_2_RBUS di_hmc_me_refine_ctrl2_reg;
	static di_im_di_hmc_me_refine_ctrl_2_RBUS ORI_di_hmc_me_refine_ctrl2_reg;

	/*LG internal TV signal "E-52"*/
	di_tnrxc_mkii_ctrl_RBUS tnrxc_mkii_ctrl_reg;

	static di_im_di_ma_frame_motion_th_a_RBUS ORI_di_ma_frame_motion_th_a_reg;
	static di_im_di_ma_frame_motion_th_b_RBUS ORI_di_ma_frame_motion_th_b_reg;
	static di_im_di_ma_frame_motion_th_c_RBUS ORI_di_ma_frame_motion_th_c_reg;
	static di_di_smd_control_candidatelist_RBUS ORI_di_smd_control_candiate_reg;
	static di_im_di_nintra_control_RBUS ori_di_im_di_nintra_control;
#if 0
	static di_im_di_hmc_pan_dummy_2_RBUS ORI_di_pan_dummy2_reg;
	static di_im_di_hmc_pan_dummy_3_RBUS ORI_di_pan_dummy3_reg;
#else // henry Merlin3 temp
	static di_im_di_rtnr_refined_ctrl2_RBUS ORI_di_im_di_rtnr_refined_ctrl2_reg;
	static di_di_smd_dummy_RBUS ORI_di_di_smd_dummy_reg;
#endif

	static di_im_di_btr_control_RBUS ORI_di_im_di_btr_control_reg;

  	static vdpq_postp_secam_fix_ctrl_RBUS ORI_postp_secam_fix_ctrl_reg;
	static vdpq_postp_secam_fix_y_detect0_RBUS ORI_postp_secam_fix_y_detect0_reg;
	static vdpq_postp_secam_fix_y_detect1_RBUS ORI_postp_secam_fix_y_detect1_reg;

	static di_im_di_hmc_pan_control_par_1_RBUS ORI_di_hmc_pan_control_par_1_reg;
	static di_im_di_hmc_statistic1_RBUS ORI_di_hmc_statistic1_reg;

	static di_di_gst_ctrl_RBUS ORI_init_di_gst_ctrl_reg;

#ifdef CONFIG_HW_SUPPORT_MEMC
	static memc_mux_memc_mux_di_ctrl_RBUS ORI_color_temp_ctrl_reg;
#endif //CONFIG_HW_SUPPORT_MEMC

	/*LG internal TV signal "E-52"*/
	static di_tnrxc_mkii_ctrl_RBUS ORI_tnrxc_mkii_ctrl_reg;

	//static vdtop_auto_pos_ctrl_RBUS ORI_auto_pos_ctrl_reg;//mac7p pq compile fix

	static unsigned int ori_ULTRI_value=0;
	static unsigned short ori_smd_upperbound_sad_forjudgemv=0;
	static unsigned short ori_smd_rang_sad=0;
	static unsigned char  ori_smd_firmware_pan_sad_th=0;
	static unsigned char ori_smd_firmware_pan=0;
	static unsigned char ori_smd_hardware_pan_en=0;
	static unsigned char ori_smd_apply_result_mode=0;
	static unsigned char ori_smd_firmwarepan_sadmax=0;
	static unsigned char ori_smd_close2gmv_mvselect=0;
	//static unsigned char ori_smd_close2gmv_fw_en=0; //mac5p removed
	static unsigned char ori_gst_subpixel_compensate_en=0;
	static unsigned char ori_gst_subpixel_hardwarepan_en=0;
	static unsigned char ori_smd_en=0;
	static unsigned char ori_smd_result_en=0;
	static unsigned char ori_smd_acc_smd_en =0;
	static unsigned char ori_smd_spatialfreqnomatch_off =0;
	static unsigned char ori_smd_outofrange_content_new = 0 ;

	static unsigned short ori_cp_sresultweight=0;
	static unsigned short ori_mosquito_en=0;
	static unsigned short ori_curve_mapping_w1_3=0,ori_curve_mapping_w1_4=0,ori_curve_mapping_w1_5=0;//,ori_curve_mapping_w2_2_2=0;
	static unsigned short ori_curve_mapping_w1_2=0,ori_curve_mapping_step6=0,ori_curve_mapping_step7=0,ori_curve_mapping_step8=0;
	static unsigned short ori_curve_mapping_w1_6=0,ori_curve_mapping_w1_7=0;
	static unsigned short ori_curve_mapping_w1_8=0;

	static unsigned short ori_mosquito_nr_positive_shift=0;
	//static unsigned short ori_mosquito_nr_bi_nlevel_max=0;
	static unsigned short ori_mosquito_far_var_gain=0;
//	static unsigned char gReg123=0;
//	static unsigned char gRegW_MonScope=FALSE;
	static unsigned char ori_Intr_di_range=0;
	static unsigned char ori_gain_pos=0;
	static unsigned char ori_gain_neg=0;
	static unsigned char ori_gain_LV_Gain=0;
	static unsigned char ori_cp_fixedweight1y=0;
	static unsigned char ori_cp_fixedweight2y=0;
	//VD register setting moved to scalerVideo.c, willy 20150713
	//static unsigned char ori_chroma_bw_lo=0;
	static unsigned char ori_ma_snr_edge_curve_slope=0;
	static unsigned char ori_ma_snr_output_clamp=0;

	static unsigned char ori_text_gain_pos=0;
	static unsigned char ori_text_gain_neg=0;

	static unsigned char ori_hmc_sub_pixel_diff_thd=0;
	static unsigned char ori_curve_mapping_w2_2=0;
	static unsigned char ori_curve_mapping_w2_3=0;
	static unsigned char ori_curve_mapping_w2_4=0;
	static unsigned char ori_curve_mapping_w2_5=0;

	static unsigned char ori_vcti_select=0;
	static unsigned char ori_vcti_en=0;
	static scaleup_dm_mem_crc_ctrl_RBUS ori_dm_mem_crc_ctrl_RBUS_reg;
	static unsigned char ori_vcti_low_pass_filter_en =0;
	static unsigned char ori_spatialenablec = 0;
	static unsigned char ori_mosquito_edgethd=0;
	static unsigned char ori_cds_enable=0;

	static unsigned char ori_vpk_gain_neg=0;
	static unsigned char ori_vpk_gain_pos=0;

	static unsigned char ori_I_uvsync_en=0;
	static unsigned char ori_I_uvgain=0;
	static unsigned char ori_I_lp_mode=0;
	static unsigned char ori_I_vlp_mode=0;
	static unsigned char ori_I_blending_mode=0;
	static unsigned char ori_I_cur_sel=0;
	static unsigned char ori_I_data_sel=0;
	static unsigned char ori_I_maxlen =0;

	static unsigned char ori_D_uvgain =0;
	static unsigned char ori_D_vlp_mode =0;
	static unsigned char ori_D_data_sel =0;
	static unsigned char ori_D_engdiv =0;
	static unsigned char ori_D_offdiv =0;
	static unsigned char ori_D_maxlen =0;
	static unsigned char ori_vc_snr_lpf_sel=0;
	static unsigned char ori_sresultweight_adjust_shift=0;
	static unsigned char ori_gst_gst_static_disable=0;
	static unsigned char ori_gst_min_sad_compare_diable=0;
	static unsigned char ori_gst_min_sad_nolimit_en=0;
	static unsigned char ori_smd_weave_dynamic=0;

	static unsigned char ori_gst_mv_limit=0;

    	static unsigned char ori_film_enable=0;

	unsigned char Motion_Th_c[7]={12,26,37,47,56,63,70};
	unsigned char Motion_Th_c_can[7] = {6,10,14,18,22,26,30};
	unsigned char Motion_Th[15]={8,10,13,16,17,28,36,45,53,57,64,67,68,69,72};

	static unsigned char ori_nintra_lp_src_yclamp_th=0;
	static di_im_di_nintra_search_dir_limit_RBUS ori_di_im_di_nintra_search_dir_limit;
	static di_di_smd_compensate_RBUS ori_smd_compensate_reg;
	static di_di_smd_refinemv2_RBUS ori_di_smd_refinemv2_reg;

	static unsigned char ori_dm_uzu_hor_mode_sel = 0;
	static unsigned char ori_dm_uzu_y_consttab_sel =0;

// chen 170522
//	static unsigned char ori_n_mcnr_offset = 0;
// end chen 170522
	static unsigned char ori_conf_lowbnd=0;
	static unsigned int kernel_init_count=0;

	static unsigned int start_count = 0;
	static unsigned char set_p2i_value = 0;

	di_im_di_hmc_pan_control_RBUS di_im_di_hmc_pan_control_reg;

	di_im_di_film_sawtooth_filmframe_th_RBUS im_di_film_sawtooth_filmframe_th_reg;
	static di_im_di_film_sawtooth_filmframe_th_RBUS ORI_im_di_film_sawtooth_filmframe_th_reg;
	di_im_di_film_static_sum_th_RBUS film_static_sum_th_reg;
	static di_im_di_film_static_sum_th_RBUS ORI_film_static_sum_th_reg;
	di_im_di_film_motion_sum_th_RBUS film_motion_sum_th_reg;
	static di_im_di_film_motion_sum_th_RBUS ORI_film_motion_sum_th_reg;

	_system_setting_info* system_info_structure_table = NULL;
	_clues* SmartPic_clue=NULL;
	_RPC_clues* RPC_SmartPic_clue=NULL;
	static unsigned char pre_source =255;
	unsigned char i=0;
	unsigned char panDeb_str = 0;
	unsigned char panConti_str = 0;


	unsigned char cur_source;
	RPC_ICM_Global_Ctrl 	*g_ICM_Global_Ctrl_Struct;

	system_info_structure_table = scaler_GetShare_Memory_system_setting_info_Struct();
	SmartPic_clue = scaler_GetShare_Memory_SmartPic_Clue_Struct();
	RPC_SmartPic_clue = scaler_GetShare_Memory_RPC_SmartPic_Clue_Struct();
	if((system_info_structure_table == NULL) || (SmartPic_clue == NULL)|| (RPC_SmartPic_clue == NULL))
	{
		if(MA_print_count%360==0)
			ROSPrintf("Share Memory PQ data ERROR, system_info = %p, SmartPic_clue=%p, RPC_SmartPic_clue=%p \n",system_info_structure_table, SmartPic_clue, RPC_SmartPic_clue );
		return;
	}





	//ROSPrintf("\n rord vip current %d \n ",cur_source);

	if(apply==0) // reset init value;
	{
		for(i=0;i<200;i++)
		{
			DynamicOptimizeSystem[i]=0;
		}
		if(Stop_Polling.Stop_Reset_Pattern==1)
			return;
		//ROSPrintf("\n cur_source %d , pre_source %d \n",cur_source,pre_source);
	}

	cur_source=(RPC_system_info_structure_table->VIP_source);
	g_ICM_Global_Ctrl_Struct= &(system_info_structure_table->PQ_Setting_Info.S_RPC_ICM_Global_Ctrl);


	init_di_smd_additionalcoeff1_reg.regValue=rtd_inl(DI_DI_SMD_AdditionalCoeff1_reg);

	smd_choosecandidate_reg.regValue = rtd_inl(DI_DI_SMD_ChooseCandidate_reg);

	smd_refinemv3_reg.regValue =  rtd_inl(DI_DI_SMD_RefineMV3_reg);
	smd_compensate_reg.regValue =  rtd_inl(DI_DI_SMD_Compensate_reg);

	init_di_smd_candidatelimitsad_reg.regValue =  rtd_inl(DI_DI_SMD_CandidateLimitSAD_reg);
	init_di_smd_additionalcoeff2_reg.regValue =  rtd_inl(DI_DI_SMD_AdditionalCoeff2_reg);
	di_smd_dummy_reg.regValue =  rtd_inl(DI_DI_SMD_dummy_reg);

	di_smd_refinemv2_reg.regValue = rtd_inl(DI_DI_SMD_RefineMV2_reg);

	init_di_smd_pan1_reg.regValue =rtd_inl(DI_DI_SMD_Pan1_reg);
	di_di_smd_hardwarepan_modify1.regValue=rtd_inl(DI_DI_SMD_HardwarePan_Modify1_reg);
	di_di_smd_sampling_gmvc.regValue = rtd_inl(DI_DI_SMD_Sampling_GMVc_reg);
	di_di_gst_compensate.regValue = rtd_inl(DI_DI_GST_Compensate_reg);

	di_di_smd_slow_motion_handle1.regValue= rtd_inl(DI_DI_SMD_Slow_Motion_handle1_reg);
	di_di_smd_slow_motion_handle2.regValue= rtd_inl(DI_DI_SMD_Slow_Motion_handle2_reg);

	di_im_di_nintra_lowpass_source.regValue	= rtd_inl(DI_IM_DI_NINTRA_LOWPASS_SOURCE_reg);

	di_im_di_nintra_search_dir_limit.regValue= rtd_inl(DI_IM_DI_NINTRA_SEARCH_DIR_LIMIT_reg);

	init_di_gst_ctrl_reg.regValue = rtd_inl(DI_DI_GST_Ctrl_reg);
#ifdef CONFIG_HW_SUPPORT_MEMC
	color_temp_ctrl_reg.regValue = rtd_inl(MEMC_MUX_MEMC_MUX_DI_CTRL_reg);
#endif //CONFIG_HW_SUPPORT_MEMC
	dch1_cp_ctrl_reg.regValue = rtd_inl(NR_DCH1_CP_Ctrl_reg);
	dm_uzumain_vcti_lpf_reg.regValue = rtd_inl(SCALEUP_DM_UZUMAIN_VCTI_LPF_reg);
	nr_mosquito_ctrl_RBUS_reg.regValue = rtd_inl(NR_MOSQUITO_CTRL_reg);
	curve_mapping_w1_1_reg.regValue = rtd_inl(NR_CURVE_MAPPING_W1_1_reg);
	curve_mapping_w1_2_reg.regValue = rtd_inl(NR_CURVE_MAPPING_W1_2_reg);
	curve_mapping_w1_3_reg.regValue = rtd_inl(NR_CURVE_MAPPING_W1_3_reg);

	curve_mapping_w2_1_reg.regValue = rtd_inl(NR_CURVE_MAPPING_W2_1_reg);
	curve_mapping_step2_reg.regValue = rtd_inl(NR_CURVE_MAPPING_STEP_2_reg);
	curve_mapping_step3_reg.regValue = rtd_inl(NR_CURVE_MAPPING_STEP_3_reg);
	di_control_reg.regValue = rtd_inl(DI_IM_DI_CONTROL_reg);

	nr_mosquito_nr_level_ctrl_RBUS_reg.regValue = rtd_inl(NR_MOSQUITO_NR_LEVEL_CTRL_reg);
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue= rtd_inl(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg);
	dm_uzu_Ctrl_reg.regValue = rtd_inl(SCALEUP_DM_UZU_Ctrl_reg);

	dch1_Spatial_Weight_Reg.regValue = rtd_inl(NR_DCH1_Spatial_Weight_reg);

	// No.191 monoscope
	color_sharp_dm_segpk_edgpk3.regValue = rtd_inl(COLOR_SHARP_DM_SEGPK_EDGPK3_reg);
	color_sharp_dm_peaking_bound_1_reg.regValue= rtd_inl(COLOR_SHARP_DM_PEAKING_BOUND_1_reg);


	di_rtnr_ma_snr_strength_control.regValue=	rtd_inl(DI_RTNR_MA_SNR_STRENGTH_CONTROL_reg);


	color_sharp_dm_peaking_gain_reg.regValue = rtd_inl(COLOR_SHARP_DM_PEAKING_GAIN_reg);

	di_hmc_adjustable_reg.regValue = rtd_inl(DI_IM_DI_HMC_ADJUSTABLE_reg);
	di_hmc_pan_control_par_2_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_CONTROL_PAR_2_reg);
	di_hmc_pan_control_par_1_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_CONTROL_PAR_1_reg);
	di_hmc_statistic1_reg.regValue = rtd_inl(DI_IM_DI_HMC_STATISTIC1_reg);

	di_hmc_adjustable3_reg.regValue = rtd_inl(DI_IM_DI_HMC_ADJUSTABLE3_reg);
	di_hmc_adjustable4_reg.regValue = rtd_inl(DI_IM_DI_HMC_ADJUSTABLE4_reg);


	dm_segpk_vpk2.regValue = rtd_inl(COLOR_SHARP_DM_SEGPK_VPK2_reg );



	im_di_weave_window_control_reg.regValue = rtd_inl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);
	im_di_weave_window_1_start_reg.regValue = rtd_inl(DI_IM_DI_WEAVE_WINDOW_1_START_reg);
	im_di_weave_window_1_end_reg.regValue = rtd_inl(DI_IM_DI_WEAVE_WINDOW_1_END_reg);

	di_ma_frame_motion_th_a_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
	di_ma_frame_motion_th_b_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg);
	di_ma_frame_motion_th_c_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg);


    /*
	if(system_info_structure_table->Input_src_Form == _SRC_FROM_VDC && vd_pwr_ok) //only VDC format can read it.avoid vd ip had close
	{
		//YC_BW_CTRL_reg.regValue = rtd_inl(VDPQ_YC_BW_CTRL_reg);
		// outer if function currently useless, willy 20150605
		//vdpq_adap_bpf_y_th3_reg.regValue = rtd_inl(VDPQ_ADAP_BPF_Y_TH3_reg);
	}
	// set as the IC default value, willy 20150605
	//vdpq_adap_bpf_y_th3_reg.regValue = 0;
    */

	curve_mapping_w2_1_reg.regValue 	=	rtd_inl(NR_CURVE_MAPPING_W2_1_reg);
	curve_mapping_w2_2_reg.regValue 	=	rtd_inl(NR_CURVE_MAPPING_W2_2_reg);

	di_control_reg.regValue = rtd_inl(DI_IM_DI_CONTROL_reg);
	intra_th_Reg.regValue = rtd_inl(DI_IM_DI_INTRA_TH_reg);
	dm_mem_crc_ctrl_RBUS_reg.regValue= rtd_inl(SCALEUP_DM_MEM_CRC_CTRL_reg);

	//frame_motion_a_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
	color_sharp_dm_cds_cm0_u_bound_0_reg.regValue = rtd_inl(COLOR_SHARP_DM_CDS_CM0_U_BOUND_0_reg);

	rtnr_new_Ctrl.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_CONTROL_reg);

	ich1_newdcti_reg_2.regValue = rtd_inl(IDCTI_I_DCTI_CTRL_2_reg);
	ich1_newdcti_reg_1.regValue = rtd_inl(IDCTI_I_DCTI_CTRL_1_reg);
	dm_dcti_REG_1.regValue = rtd_inl(COLOR_SHARP_SHP_DCTI_CTRL_1_reg);
	dm_dcti_REG_2.regValue = rtd_inl(COLOR_SHARP_SHP_DCTI_CTRL_2_reg);

	dm_uzu_Ctrl.regValue = rtd_inl(SCALEUP_DM_UZU_Ctrl_reg);

	/*//mac7p pq compile fix
	if (cur_source <= VIP_QUALITY_TV_PAL) {
		if (vd_pwr_ok)
			auto_pos_ctrl_reg.regValue = VDPQ_rtd_inl(VDTOP_AUTO_POS_CTRL_reg);
	}
	*/

	// special setting for LG139, willy 20151005
	di_ma_frame_motion_th_a_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
	di_ma_frame_motion_th_b_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg);
	di_ma_frame_motion_th_c_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg);
	di_smd_control_candiate_reg.regValue = rtd_inl(DI_DI_SMD_Control_CandidateList_reg);
	di_im_di_nintra_control.regValue = rtd_inl(DI_IM_DI_NINTRA_CONTROL_reg);
#if 0
	di_pan_dummy2_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_DUMMY_2_reg);
	di_pan_dummy3_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_DUMMY_3_reg);
#else // henry Merlin3 temp
	di_im_di_rtnr_refined_ctrl2_reg.regValue = rtd_inl(DI_IM_DI_RTNR_REFINED_CTRL2_reg);
	di_di_smd_dummy_reg.regValue = rtd_inl(DI_DI_SMD_dummy_reg);
#endif

	di_im_di_btr_control_reg.regValue = rtd_inl(DI_IM_DI_BTR_CONTROL_reg);

	di_hmc_adjustable2_reg.regValue = rtd_inl(DI_IM_DI_HMC_ADJUSTABLE2_reg);

	if(cur_source == VIP_QUALITY_TV_SECAN){
	    	postp_secam_fix_ctrl_reg.regValue =VDPQ_rtd_inl(VDPQ_POSTP_SECAM_FIX_CTRL_reg);
		postp_secam_fix_y_detect0_reg.regValue = VDPQ_rtd_inl(VDPQ_POSTP_SECAM_FIX_Y_DETECT0_reg);
		postp_secam_fix_y_detect1_reg.regValue = VDPQ_rtd_inl(VDPQ_POSTP_SECAM_FIX_Y_DETECT1_reg);
	}
	di_im_new_mcnr_control.regValue = rtd_inl(DI_IM_NEW_MCNR_CONTROL_reg);

	dm_dir_uzu_conf_Reg.regValue = rtd_inl(SCALEUP_DM_DIR_UZU_Conf_reg);
	di_im_di_hmc_pan_control_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_CONTROL_reg);
	di_hmc_me_refine_ctrl2_reg.regValue = rtd_inl(DI_IM_DI_HMC_ME_REFINE_CTRL_2_reg);
	FrameSobel_statistic_Reg.regValue = rtd_inl(DI_IM_DI_FRAMESOBEL_STATISTIC_reg);



	di_di_smd_waterflag_ctr3.regValue = rtd_inl(DI_DI_SMD_WaterFlag_Ctr3_reg);
	//di_di_smd_resurrect_ctrl.regValue = rtd_inl(DI_DI_SMD_Resurrect_Ctrl_reg); //mac5p removed




	im_di_film_sawtooth_filmframe_th_reg.regValue = rtd_inl(DI_IM_DI_FILM_SAWTOOTH_FILMFRAME_TH_reg);
	film_static_sum_th_reg.regValue = rtd_inl(DI_IM_DI_FILM_STATIC_SUM_TH_reg);
	film_motion_sum_th_reg.regValue = rtd_inl(DI_IM_DI_FILM_MOTION_SUM_TH_reg);


	/*LG internal TV signal "E-52"*/
	tnrxc_mkii_ctrl_reg.regValue = rtd_inl(DI_TNRXC_MkII_CTRL_reg);

	//when timing change ,need kernel quality handle load table finiished ,wait 120 times if still not flag form kernel , start work
	if((pre_source !=cur_source )&&(kernel_quality_init==0)&&(kernel_init_count<120))
	{
		kernel_init_count++;
		return;
	}

	if(pre_source !=cur_source||ScalerVIP_Get_pq_reset()==1)
	{
		scalerVIP_Set_pq_reset(0);
		//ROSPrintf(" \n rord ini =%d \n",kernel_quality_init);
		//ROSPrintf(" \n rord count =%d \n",kernel_init_count);

		ori_smd_upperbound_sad_forjudgemv=di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv;
		ori_smd_rang_sad = smd_choosecandidate_reg.smd_reliablerange_sad_th;

		ori_smd_firmware_pan_sad_th = init_di_smd_pan1_reg.smd_firmware_pan_sad_th;

		ori_smd_firmware_pan=init_di_smd_pan1_reg.smd_firmware_pan;
		ori_smd_hardware_pan_en=init_di_smd_pan1_reg.smd_hardware_pan_en;
		ori_smd_apply_result_mode=init_di_smd_pan1_reg.smd_apply_result_mode;
		ori_smd_firmwarepan_sadmax=di_di_smd_hardwarepan_modify1.smd_firmwarepan_sadmax;
		ori_smd_close2gmv_mvselect=di_di_smd_sampling_gmvc.smd_close2gmv_mvselect;
		//ori_smd_close2gmv_fw_en =di_di_smd_sampling_gmvc.smd_close2gmv_fw_en; //mac5 removed
		ori_gst_subpixel_compensate_en=di_di_gst_compensate.gst_subpixel_compensate_en;
		ori_gst_subpixel_hardwarepan_en = di_di_gst_compensate.gst_subpixel_hardwarepan_en;
		ori_smd_en = di_smd_control_candiate_reg.smd_en;
		ori_smd_result_en = init_di_smd_additionalcoeff1_reg.smd_result_en;


		ori_di_di_smd_waterflag_ctr3.regValue = di_di_smd_waterflag_ctr3.regValue;
		//ori_di_di_smd_resurrect_ctrl.regValue = di_di_smd_resurrect_ctrl.regValue; //mac5p removed



		ori_smd_acc_smd_en =di_di_smd_slow_motion_handle1.smd_acc_smd_en;
		ori_smd_spatialfreqnomatch_off =di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off;
		ori_smd_outofrange_content_new = di_di_smd_slow_motion_handle1.smd_outofrange_content_new;

		ori_gst_gst_static_disable = di_di_smd_slow_motion_handle1.gst_gst_static_disable;
		ori_gst_min_sad_compare_diable = di_di_smd_slow_motion_handle1.gst_min_sad_compare_diable;
		ori_gst_min_sad_nolimit_en = di_di_smd_slow_motion_handle1.gst_min_sad_nolimit_en;
		ori_smd_weave_dynamic = di_di_smd_slow_motion_handle1.smd_weave_dynamic;

		ori_gst_mv_limit = di_di_smd_slow_motion_handle2.gst_mv_limit;

		ori_nintra_lp_src_yclamp_th = di_im_di_nintra_lowpass_source.nintra_lp_src_yclamp_th;
		ori_di_im_di_nintra_search_dir_limit.regValue= di_im_di_nintra_search_dir_limit.regValue;


		ori_cp_sresultweight=dch1_cp_ctrl_reg.cp_sresultweight;
		ori_spatialenablec = dch1_cp_ctrl_reg.cp_spatialenablec;
		ori_vc_snr_lpf_sel = dch1_cp_ctrl_reg.vc_snr_lpf_sel;
		ori_sresultweight_adjust_shift = dch1_cp_ctrl_reg.sresultweight_adjust_shift;

		ori_mosquito_en=nr_mosquito_ctrl_RBUS_reg.mosquito_detect_en;
		ori_curve_mapping_w1_3 =curve_mapping_w1_1_reg.curvemapping_w1_3;
		ori_curve_mapping_w1_4=curve_mapping_w1_2_reg.curvemapping_w1_4;
		ori_curve_mapping_w1_5=curve_mapping_w1_2_reg.curvemapping_w1_5;
		ori_curve_mapping_w1_2=curve_mapping_w1_1_reg.curvemapping_w1_2;
		ori_curve_mapping_w1_6=curve_mapping_w1_2_reg.curvemapping_w1_6;
		ori_curve_mapping_w1_7=curve_mapping_w1_2_reg.curvemapping_w1_7;
		ori_curve_mapping_w1_8=curve_mapping_w1_3_reg.curvemapping_w1_8;

		ori_curve_mapping_step6=curve_mapping_step2_reg.curvemapping_step6;
		ori_curve_mapping_step7=curve_mapping_step3_reg.curvemapping_step7;
		ori_curve_mapping_step8=curve_mapping_step3_reg.curvemapping_step8;

		ori_ma_snr_edge_curve_slope=di_rtnr_ma_snr_strength_control.rtnr_ma_snr_edge_curve_slope;
		ori_ma_snr_output_clamp=di_rtnr_ma_snr_strength_control.rtnr_ma_snr_output_clamp;

		ori_mosquito_nr_positive_shift=nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_shift;
		//ori_mosquito_nr_bi_nlevel_max=nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_bi_nlevel_max;
		ori_mosquito_far_var_gain=nr_mosquito_nr_level_ctrl_1_RBUS_reg.far_var_gain;
		ori_ULTRI_value = dm_uzu_Ctrl_reg.regValue; // what is this????? assign a register value to a specific bit?
		ori_Intr_di_range = di_control_reg.regValue; // what is this????? assign a register value to a specific bit?

		ori_gain_pos=color_sharp_dm_segpk_edgpk3.gain_pos;
		ori_gain_neg=color_sharp_dm_segpk_edgpk3.gain_neg;
		ori_gain_LV_Gain=color_sharp_dm_peaking_bound_1_reg.lv;

		ori_cp_fixedweight1y = dch1_Spatial_Weight_Reg.cp_fixedweight1y;
		ori_cp_fixedweight2y = dch1_Spatial_Weight_Reg.cp_fixedweight2y;

		ori_text_gain_pos =color_sharp_dm_peaking_gain_reg.gain_pos;
		ori_text_gain_neg = color_sharp_dm_peaking_gain_reg.gain_neg;

		ORI_di_hmc_adjustable_reg.regValue= di_hmc_adjustable_reg.regValue;
		ori_hmc_sub_pixel_diff_thd = di_hmc_pan_control_par_2_reg.hmc_sub_pixel_diff_thd;
		ORI_di_hmc_adjustable3_reg.regValue= di_hmc_adjustable3_reg.regValue;
		ORI_di_hmc_adjustable4_reg.regValue= di_hmc_adjustable4_reg.regValue;

		ori_curve_mapping_w2_2 = curve_mapping_w2_1_reg.curvemapping_w2_2;
		ori_curve_mapping_w2_3 = curve_mapping_w2_1_reg.curvemapping_w2_3;
		ori_curve_mapping_w2_4 = curve_mapping_w2_2_reg.curvemapping_w2_4;
		ori_curve_mapping_w2_5 = curve_mapping_w2_2_reg.curvemapping_w2_5;

		ori_vcti_select = dm_mem_crc_ctrl_RBUS_reg.vcti_select;
		ori_vcti_en = dm_mem_crc_ctrl_RBUS_reg.vcti_en;
		ori_dm_mem_crc_ctrl_RBUS_reg.regValue = dm_mem_crc_ctrl_RBUS_reg.regValue;
		ori_vcti_low_pass_filter_en = dm_uzumain_vcti_lpf_reg.vcti_low_pass_filter_en;
		ori_mosquito_edgethd = nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd;
		//ori_frame_motion_a_reg = frame_motion_a_reg.ma_framestha;
		ori_cp_sresultweight = dch1_cp_ctrl_reg.cp_sresultweight;

		ori_cds_enable=color_sharp_dm_cds_cm0_u_bound_0_reg.cds_enable;


		ori_vpk_gain_neg=dm_segpk_vpk2.vpk_gain_neg;
		ori_vpk_gain_pos=dm_segpk_vpk2.vpk_gain_pos;


		ori_I_uvsync_en=ich1_newdcti_reg_2.uvsync_en;
		ori_I_uvgain=ich1_newdcti_reg_1.uvgain;
		ori_I_lp_mode=ich1_newdcti_reg_1.lp_mode;
		ori_I_vlp_mode=ich1_newdcti_reg_2.vlp_mode;
		ori_I_blending_mode=ich1_newdcti_reg_2.blending_mode;
		ori_I_cur_sel=ich1_newdcti_reg_1.cur_sel;
		ori_I_data_sel=ich1_newdcti_reg_1.data_sel;
		ori_I_maxlen =ich1_newdcti_reg_1.maxlen;

		ori_D_uvgain=dm_dcti_REG_1.uvgain;
		ori_D_vlp_mode=dm_dcti_REG_2.vlp_mode;
		ori_D_data_sel=dm_dcti_REG_1.data_sel;
		ori_D_engdiv=dm_dcti_REG_1.engdiv;
		ori_D_offdiv=dm_dcti_REG_1.offdiv;
		ori_D_maxlen=dm_dcti_REG_1.maxlen;

		ori_film_enable = di_control_reg.film_enable;

		ori_dm_uzu_hor_mode_sel = dm_uzu_Ctrl.hor_mode_sel;
		ori_dm_uzu_y_consttab_sel = dm_uzu_Ctrl.h_y_consttab_sel;
		//ORI_auto_pos_ctrl_reg.regValue= auto_pos_ctrl_reg.regValue;//mac7p pq compile fix

		ORI_di_ma_frame_motion_th_a_reg.regValue = di_ma_frame_motion_th_a_reg.regValue;
		ORI_di_ma_frame_motion_th_b_reg.regValue = di_ma_frame_motion_th_b_reg.regValue;
		ORI_di_ma_frame_motion_th_c_reg.regValue = di_ma_frame_motion_th_c_reg.regValue;
		ORI_di_smd_control_candiate_reg.regValue = di_smd_control_candiate_reg.regValue;
		ori_di_im_di_nintra_control.regValue = di_im_di_nintra_control.regValue;
#if 0
		ORI_di_pan_dummy2_reg.regValue = di_pan_dummy2_reg.regValue;
		ORI_di_pan_dummy3_reg.regValue = di_pan_dummy3_reg.regValue;
#else // henry Merlin3 temp
		ORI_di_im_di_rtnr_refined_ctrl2_reg.regValue = di_im_di_rtnr_refined_ctrl2_reg.regValue;
		ORI_di_di_smd_dummy_reg.regValue = di_di_smd_dummy_reg.regValue;
		panDeb_str = ORI_di_di_smd_dummy_reg.dummy18024540_31_2 & 0xFF; // 8 bits [9:2]
		panConti_str = ORI_di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 & 0xFF; // 8 buts [24:17]
#endif

		ORI_di_im_di_btr_control_reg.regValue = di_im_di_btr_control_reg.regValue;

		ORI_di_hmc_adjustable2_reg.regValue = di_hmc_adjustable2_reg.regValue;

		ORI_postp_secam_fix_ctrl_reg.regValue = postp_secam_fix_ctrl_reg.regValue;
		ORI_postp_secam_fix_y_detect0_reg.regValue = postp_secam_fix_y_detect0_reg.regValue;
		ORI_postp_secam_fix_y_detect1_reg.regValue = postp_secam_fix_y_detect1_reg.regValue;

// chen 170522
	//	ori_n_mcnr_offset = di_im_new_mcnr_control.n_mcnr_offset;
// end chen 170522

		ORI_im_di_weave_window_control_reg.regValue = im_di_weave_window_control_reg.regValue;
		ORI_im_di_weave_window_1_start_reg.regValue = im_di_weave_window_1_start_reg.regValue;
		ORI_im_di_weave_window_1_end_reg.regValue = im_di_weave_window_1_end_reg.regValue;

		ori_conf_lowbnd =dm_dir_uzu_conf_Reg.conf_lowbnd;

		ORI_di_hmc_pan_control_par_1_reg.regValue = di_hmc_pan_control_par_1_reg.regValue;
		ORI_di_hmc_statistic1_reg.regValue = di_hmc_statistic1_reg.regValue;
		ORI_di_hmc_me_refine_ctrl2_reg.regValue = di_hmc_me_refine_ctrl2_reg.regValue;

		ORI_FrameSobel_statistic_Reg.regValue = FrameSobel_statistic_Reg.regValue;


		ori_smd_compensate_reg.regValue = smd_compensate_reg.regValue;

		ori_di_smd_refinemv2_reg.regValue= di_smd_refinemv2_reg.regValue;

		ORI_tnrxc_mkii_ctrl_reg.regValue = tnrxc_mkii_ctrl_reg.regValue;
		ORI_init_di_gst_ctrl_reg.regValue= init_di_gst_ctrl_reg.regValue;
#ifdef CONFIG_HW_SUPPORT_MEMC
		ORI_color_temp_ctrl_reg.regValue=color_temp_ctrl_reg.regValue;
#endif //CONFIG_HW_SUPPORT_MEMC

		ORI_im_di_film_sawtooth_filmframe_th_reg.regValue = im_di_film_sawtooth_filmframe_th_reg.regValue;
		ORI_film_static_sum_th_reg.regValue = film_static_sum_th_reg.regValue;
		ORI_film_motion_sum_th_reg.regValue = film_motion_sum_th_reg.regValue;



		set_p2i_value = 0;

	}

	kernel_quality_init = 0 ;
	kernel_init_count =0;
	pre_source = cur_source;


	if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
	{
		di_double_buff_apply =1;
 	}
	init_di_smd_additionalcoeff1_reg.smd_result_en=ori_smd_result_en;
	rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg,init_di_smd_additionalcoeff1_reg.regValue);

	switch(cur_source)
	{
		case VIP_QUALITY_CVBS_NTSC:
		{
			if(!vd_pwr_ok)
				break;

			//************** INI VALE START    ***************************************************


			//#1
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			di_smd_control_candiate_reg.smd_en=ori_smd_en;
			smd_compensate_reg.smd_compensate_teeth_th =ori_smd_compensate_reg.smd_compensate_teeth_th;

			//#2
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;

			//#7
#if 0
			di_pan_dummy2_reg.panConti_str = ORI_di_pan_dummy2_reg.panConti_str;
			di_pan_dummy3_reg.panDeb_str = ORI_di_pan_dummy3_reg.panDeb_str;
#else // henry Merlin3 temp
			di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | panConti_str;
			di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | panDeb_str;
#endif
			//#12
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=ori_smd_upperbound_sad_forjudgemv;
			smd_compensate_reg.smd_compensate_teeth_th =ori_smd_compensate_reg.smd_compensate_teeth_th;

			//#23 #24
			if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
			{
				di_double_buff_apply =1;
			}

			init_di_smd_additionalcoeff1_reg.smd_result_en=ori_smd_result_en;

			//#28
			di_im_di_nintra_lowpass_source.nintra_lp_src_yclamp_th = ori_nintra_lp_src_yclamp_th;
			di_im_di_nintra_search_dir_limit.regValue=ori_di_im_di_nintra_search_dir_limit.regValue;

			//#63
			di_di_smd_slow_motion_handle1.smd_acc_smd_en =ori_smd_acc_smd_en;
			di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=ori_smd_spatialfreqnomatch_off;
			di_di_smd_slow_motion_handle1.smd_weave_dynamic = ori_smd_weave_dynamic;
			// di_im_new_mcnr_control.n_mcnr_offset ; don't set value in here, it control by mcnr

			//#75
#if 0
			di_pan_dummy2_reg.panConti_str = ORI_di_pan_dummy2_reg.panConti_str;
			di_pan_dummy3_reg.panDeb_str = ORI_di_pan_dummy3_reg.panDeb_str;
#else // henry Merlin3 temp
			di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | panConti_str;
			di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | panDeb_str;
#endif
			di_hmc_adjustable2_reg.hmc_mv_refine_mor_noise_th = ORI_di_hmc_adjustable2_reg.hmc_mv_refine_mor_noise_th;

			//#107
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=ori_smd_upperbound_sad_forjudgemv;
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			smd_refinemv3_reg.smd_sfreq_diff_gain =4;
			smd_refinemv3_reg.smd_sfreq_diff_th =25;
			smd_refinemv3_reg.smd_sfreq_drop_th =80;
			smd_compensate_reg.smd_compensate_teeth_th =ori_smd_compensate_reg.smd_compensate_teeth_th;

			init_di_smd_candidatelimitsad_reg.smd_gmv_sad_max=0xd2;
			init_di_smd_candidatelimitsad_reg.smd_gmv_sad_th=0x40;

			init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain=gSmd_gain;
			init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset=gSmd_offset;
			di_smd_dummy_reg.smd_partialweave =0;

			di_hmc_pan_control_par_2_reg.hmc_sub_pixel_diff_thd = ori_hmc_sub_pixel_diff_thd;

			di_di_smd_slow_motion_handle1.gst_min_sad_nolimit_en =ori_gst_min_sad_nolimit_en;
			di_di_smd_slow_motion_handle1.gst_gst_static_disable =ori_gst_gst_static_disable;
			di_di_smd_slow_motion_handle1.smd_weave_dynamic = ori_smd_weave_dynamic;
			di_di_smd_slow_motion_handle1.smd_outofrange_content_new = ori_smd_outofrange_content_new;
			di_di_smd_slow_motion_handle2.gst_mv_limit = ori_gst_mv_limit;
			//di_di_smd_slow_motion_handle1.smd_acc_smd_en =ori_smd_acc_smd_en;
			//di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=ori_smd_spatialfreqnomatch_off;

			im_di_weave_window_control_reg.fixedwindow1 = ORI_im_di_weave_window_control_reg.fixedwindow1;
			im_di_weave_window_1_start_reg.regValue = ORI_im_di_weave_window_1_start_reg.regValue;
			im_di_weave_window_1_end_reg.regValue = ORI_im_di_weave_window_1_end_reg.regValue;

			di_hmc_statistic1_reg.regValue = ORI_di_hmc_statistic1_reg.regValue;
			di_hmc_pan_control_par_1_reg.hme_sum_max_ratio = ORI_di_hmc_pan_control_par_1_reg.hme_sum_max_ratio;
			FrameSobel_statistic_Reg.regValue = ORI_FrameSobel_statistic_Reg.regValue;
			di_hmc_adjustable_reg.regValue = ORI_di_hmc_adjustable_reg.regValue;
			di_hmc_adjustable2_reg.regValue= ORI_di_hmc_adjustable2_reg.regValue;
			di_hmc_adjustable3_reg.regValue= ORI_di_hmc_adjustable3_reg.regValue;
			di_hmc_adjustable4_reg.regValue= ORI_di_hmc_adjustable4_reg.regValue;


			//#139
			di_ma_frame_motion_th_a_reg.ma_framestha = ORI_di_ma_frame_motion_th_a_reg.ma_framestha;
			//di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;
			di_ma_frame_motion_th_a_reg.ma_framerlvtha = ORI_di_ma_frame_motion_th_a_reg.ma_framerlvtha;
			di_ma_frame_motion_th_a_reg.ma_framemtha = ORI_di_ma_frame_motion_th_a_reg.ma_framemtha;
			di_ma_frame_motion_th_b_reg.ma_framesthb = ORI_di_ma_frame_motion_th_b_reg.ma_framesthb;
			di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;
			di_ma_frame_motion_th_c_reg.ma_framesthc = ORI_di_ma_frame_motion_th_c_reg.ma_framesthc;
			di_ma_frame_motion_th_c_reg.ma_framehthc = ORI_di_ma_frame_motion_th_c_reg.ma_framehthc;
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = ORI_di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv;

			dch1_cp_ctrl_reg.sresultweight_adjust_shift = ori_sresultweight_adjust_shift;
			dch1_cp_ctrl_reg.vc_snr_lpf_sel = ori_vc_snr_lpf_sel;
			curve_mapping_w1_2_reg.curvemapping_w1_6 = ori_curve_mapping_w1_6;
			curve_mapping_w1_2_reg.curvemapping_w1_7 = ori_curve_mapping_w1_7;
			curve_mapping_step3_reg.curvemapping_step8=ori_curve_mapping_step8;

			//#147,148,149,150
			dch1_Spatial_Weight_Reg.cp_fixedweight1y = ori_cp_fixedweight1y;
			dch1_Spatial_Weight_Reg.cp_fixedweight2y = ori_cp_fixedweight2y;

			//#151

			di_ma_frame_motion_th_a_reg.ma_framestha = ORI_di_ma_frame_motion_th_a_reg.ma_framestha;
			di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;
			di_ma_frame_motion_th_a_reg.ma_framerlvtha = ORI_di_ma_frame_motion_th_a_reg.ma_framerlvtha;
			di_ma_frame_motion_th_a_reg.ma_framemtha = ORI_di_ma_frame_motion_th_a_reg.ma_framemtha;
			di_ma_frame_motion_th_b_reg.ma_framesthb = ORI_di_ma_frame_motion_th_b_reg.ma_framesthb;
			di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;
			di_ma_frame_motion_th_b_reg.ma_framerlvthb = ORI_di_ma_frame_motion_th_b_reg.ma_framerlvthb;
			di_ma_frame_motion_th_b_reg.ma_framemthb = ORI_di_ma_frame_motion_th_b_reg.ma_framemthb;

			dm_dir_uzu_conf_Reg.conf_lowbnd = ori_conf_lowbnd;
			di_im_di_nintra_control.nintra_intersection_check_en =ori_di_im_di_nintra_control.nintra_intersection_check_en;
			di_im_di_nintra_control.nintra_preline_dir_check2_en = ori_di_im_di_nintra_control.nintra_preline_dir_check2_en;

			//#188
			di_im_di_btr_control_reg.btr_blendtemporalrecoveryen = ORI_di_im_di_btr_control_reg.btr_blendtemporalrecoveryen;

			//#189
			di_hmc_statistic1_reg.ma_hmc_lineratio = ORI_di_hmc_statistic1_reg.ma_hmc_lineratio;
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = ORI_di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass;
			di_hmc_adjustable_reg.hmc_mv_refine_en = ORI_di_hmc_adjustable_reg.hmc_mv_refine_en;
			di_hmc_adjustable2_reg.hme_x_continued_counter = ORI_di_hmc_adjustable2_reg.hme_x_continued_counter;
			if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
			{
				di_double_buff_apply =1;
			}

			init_di_smd_additionalcoeff1_reg.smd_result_en = ori_smd_result_en;
			di_hmc_pan_control_par_1_reg.hme_sum_max_ratio = ORI_di_hmc_pan_control_par_1_reg.hme_sum_max_ratio;

			//#192
			dch1_cp_ctrl_reg.cp_sresultweight =ori_cp_sresultweight;
			curve_mapping_w1_1_reg.curvemapping_w1_3 =ori_curve_mapping_w1_3;
			curve_mapping_w1_2_reg.curvemapping_w1_4 =ori_curve_mapping_w1_4;
			curve_mapping_w1_2_reg.curvemapping_w1_5 =ori_curve_mapping_w1_5;

			//VPAN case
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = ORI_di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv;

			//************** INI VALE END    ***************************************************

			if(DynamicOptimizeSystem[1] ==1)
			{
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 200;
				smd_compensate_reg.smd_compensate_teeth_th =200;
			}
			else if(DynamicOptimizeSystem[2] ==1)
			{
				drvif_color_rtnr_c_th_isr(_CHANNEL1,&Motion_Th_c[0]);
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
			}
			else if(DynamicOptimizeSystem[7] ==1)
			{
#if 0
				di_pan_dummy2_reg.panConti_str = 12;
				di_pan_dummy3_reg.panDeb_str = 80;
#else // henry Merlin3 temp
				di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | 12;
				di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | 80;
#endif

			}
			else if(DynamicOptimizeSystem[12] ==1)
			{
				//drvif_color_newrtnr_sad_th(1,&Motion_Th[0]);//for sad edge th setting
				//drvif_color_newrtnr_sad_th(0,&Motion_Th[0]);//for sad nonedge th setting
				drvif_color_rtnr_y_th_isr(_CHANNEL1,&Motion_Th[0]);

				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 200;
				smd_compensate_reg.smd_compensate_teeth_th =200;

			}
			else if((DynamicOptimizeSystem[23]==1)||(DynamicOptimizeSystem[24]==1))
			{

				if(init_di_smd_additionalcoeff1_reg.smd_result_en!=0)
				{
					di_double_buff_apply =1;
				}
				init_di_smd_additionalcoeff1_reg.smd_result_en=0;
			}
			else if(DynamicOptimizeSystem[28] ==1)
			{
				di_im_di_nintra_lowpass_source.nintra_lp_src_yclamp_th = 255;
				di_im_di_nintra_search_dir_limit.nintra_hdiff_th1=40;
				di_im_di_nintra_search_dir_limit.nintra_hdiff_th2=40;
			}
			else if(DynamicOptimizeSystem[28] ==2)
			{
				dm_dir_uzu_conf_Reg.conf_lowbnd = 30;
			}
			else if(DynamicOptimizeSystem[63] ==1)
			{
				di_di_smd_slow_motion_handle1.smd_acc_smd_en =1;
				di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=1;
				di_di_smd_slow_motion_handle1.smd_weave_dynamic = 3;
// chen 170522
	//			di_im_new_mcnr_control.n_mcnr_offset =0;
// end chen 170522


			}
			else if(DynamicOptimizeSystem[75] ==1)
			{
				drvif_color_rtnr_c_th_isr(_CHANNEL1,&Motion_Th_c_can[0]);
#if 0
				di_pan_dummy2_reg.panConti_str = 12;
				di_pan_dummy3_reg.panDeb_str = 80;
#else // henry Merlin3 temp
				di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | 12;
				di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | 80;
#endif
				di_hmc_adjustable2_reg.hmc_mv_refine_mor_noise_th = 13; // special HMC setting for LG#75 and LG#107, jimmy 20151203
			}
			else if(DynamicOptimizeSystem[107]==1)
			{
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 170;//125; willy, 20151224
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				smd_refinemv3_reg.smd_sfreq_diff_gain =0;
				smd_refinemv3_reg.smd_sfreq_diff_th =0;
				smd_refinemv3_reg.smd_sfreq_drop_th =0;
				smd_compensate_reg.smd_compensate_teeth_th =5;

				init_di_smd_candidatelimitsad_reg.smd_gmv_sad_max=0xff;
				init_di_smd_candidatelimitsad_reg.smd_gmv_sad_th=0x7f;

				init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain = 6;//2; willy, 20151224
				init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset=0x1f;
				di_smd_dummy_reg.smd_partialweave =3;

				di_hmc_pan_control_par_2_reg.hmc_sub_pixel_diff_thd = 160;

				di_di_smd_slow_motion_handle1.gst_min_sad_nolimit_en =1;
				di_di_smd_slow_motion_handle1.gst_gst_static_disable =1;
				di_di_smd_slow_motion_handle1.smd_weave_dynamic = 3;
				di_di_smd_slow_motion_handle1.smd_outofrange_content_new = 0;
				di_di_smd_slow_motion_handle2.gst_mv_limit = 1;
				//di_di_smd_slow_motion_handle1.smd_acc_smd_en =1;
				//di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=1;


				di_hmc_statistic1_reg.ma_hmc_lineratio = 24;
				rtd_outl(DI_IM_DI_HMC_STATISTIC1_reg, di_hmc_statistic1_reg.regValue);
				di_hmc_pan_control_par_1_reg.hme_sum_max_ratio = 18;

				FrameSobel_statistic_Reg.regValue = 0xc51f4200;
				di_hmc_adjustable_reg.regValue= 0x8FFD0004;
				di_hmc_adjustable2_reg.regValue= 0x0732c014; // special HMC setting for LG#75 and LG#107, jimmy 20151203
				di_hmc_adjustable3_reg.regValue= 0x6819141e;
				di_hmc_adjustable4_reg.regValue= 0x850a0c0c;



				if(RTNR_MAD_count_Y2_avg==0)
					start_count = 30;
				//select windown fix weave
				if(start_count==0) {
					im_di_weave_window_control_reg.fixedwindow1 = 1;
					im_di_weave_window_1_start_reg.regValue = 0x00460030;
					im_di_weave_window_1_end_reg.regValue = 0x008e00ec;
				} else {
					start_count--;
				}

			}
			else if(DynamicOptimizeSystem[139] ==1)
			{
				di_ma_frame_motion_th_a_reg.ma_framestha = 25;
				di_ma_frame_motion_th_a_reg.ma_framehtha = 25;
				di_ma_frame_motion_th_a_reg.ma_framerlvtha = 20;
				di_ma_frame_motion_th_a_reg.ma_framemtha = 9;

				di_ma_frame_motion_th_b_reg.ma_framesthb = 25;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 25;

				di_ma_frame_motion_th_c_reg.ma_framesthc = 20;
				di_ma_frame_motion_th_c_reg.ma_framehthc = 20;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 0;

				dch1_cp_ctrl_reg.sresultweight_adjust_shift = 0;
				dch1_cp_ctrl_reg.vc_snr_lpf_sel = 0;
				curve_mapping_w1_2_reg.curvemapping_w1_6 = 3;
				curve_mapping_w1_2_reg.curvemapping_w1_7 = 3;
				curve_mapping_step3_reg.curvemapping_step8=42;

			}
			else if(DynamicOptimizeSystem[147])
			{
				dch1_Spatial_Weight_Reg.cp_fixedweight1y = 1;
				dch1_Spatial_Weight_Reg.cp_fixedweight2y = 1;
			}
			else if(DynamicOptimizeSystem[151])
			{
				di_ma_frame_motion_th_a_reg.ma_framestha = 50;
				di_ma_frame_motion_th_a_reg.ma_framehtha = 70;
				di_ma_frame_motion_th_a_reg.ma_framerlvtha = 50;
				di_ma_frame_motion_th_a_reg.ma_framemtha = 70;
				di_ma_frame_motion_th_b_reg.ma_framesthb = 50;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 70;
				di_ma_frame_motion_th_b_reg.ma_framerlvthb = 50;
				di_ma_frame_motion_th_b_reg.ma_framemthb = 70;



				dm_dir_uzu_conf_Reg.conf_lowbnd = 63;
				di_im_di_nintra_control.nintra_intersection_check_en =0;
				di_im_di_nintra_control.nintra_preline_dir_check2_en = 0;

			}
			else if(DynamicOptimizeSystem[188])
			{
				di_im_di_btr_control_reg.btr_blendtemporalrecoveryen = 0;
				if(init_di_smd_additionalcoeff1_reg.smd_result_en!=0)
				{
					di_double_buff_apply =1;
				}
				init_di_smd_additionalcoeff1_reg.smd_result_en = 0;

			}
			else if(DynamicOptimizeSystem[189])
			{
				di_hmc_statistic1_reg.ma_hmc_lineratio += 24;
				di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
				di_hmc_adjustable_reg.hmc_mv_refine_en = 0;
				di_hmc_adjustable2_reg.hme_x_continued_counter = 5;

				if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
				{
					di_double_buff_apply =1;
				}
				init_di_smd_additionalcoeff1_reg.smd_result_en = 0;

				di_hmc_pan_control_par_1_reg.hme_sum_max_ratio = 12;

				rtd_outl(DI_IM_DI_HMC_ME_REFINE_CTRL_2_reg, di_hmc_me_refine_ctrl2_reg.regValue);

			}
			else if(DynamicOptimizeSystem[192])
			{
				dch1_cp_ctrl_reg.cp_sresultweight =15;
				curve_mapping_w1_1_reg.curvemapping_w1_3 =8;
				curve_mapping_w1_2_reg.curvemapping_w1_4 =0;
				curve_mapping_w1_2_reg.curvemapping_w1_5 =0;
			}
			else if(scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN) >= 2)
			{
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 0;
			}


			//#1
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
			rtd_outl(DI_DI_SMD_Compensate_reg, smd_compensate_reg.regValue);

			//#2
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);

			//#7

			//#23 #24
			rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg, init_di_smd_additionalcoeff1_reg.regValue);

			//#28
			rtd_outl(DI_IM_DI_NINTRA_LOWPASS_SOURCE_reg,di_im_di_nintra_lowpass_source.regValue);
			rtd_outl(DI_IM_DI_NINTRA_SEARCH_DIR_LIMIT_reg,di_im_di_nintra_search_dir_limit.regValue);

			//#63

			// protect di 3Abit,reduce r/w time .
			di_control_reg_Prtoect_3AEnable.regValue= rtd_inl(DI_IM_DI_CONTROL_reg);
			di_control_reg.ma_3aenable = di_control_reg_Prtoect_3AEnable.ma_3aenable;
			di_control_reg.write_enable_1=1;

			//rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);//rord.tsao mark don't change di seeting avoid 3a/5a setting error 2016/02/19
			rtd_outl(DI_IM_DI_INTRA_TH_reg, intra_th_Reg.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle1_reg, di_di_smd_slow_motion_handle1.regValue);
			rtd_outl(DI_IM_NEW_MCNR_CONTROL_reg,di_im_new_mcnr_control.regValue);

			//#75
#if 0
			rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_2_reg, di_pan_dummy2_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_3_reg, di_pan_dummy3_reg.regValue);
#else // henry Merlin3 temp
			rtd_outl(DI_IM_DI_RTNR_REFINED_CTRL2_reg, di_im_di_rtnr_refined_ctrl2_reg.regValue);
			rtd_outl(DI_DI_SMD_dummy_reg, di_di_smd_dummy_reg.regValue);
#endif
			//#107

			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
			rtd_outl(DI_DI_SMD_RefineMV3_reg, smd_refinemv3_reg.regValue);
			rtd_outl(DI_DI_SMD_Compensate_reg, smd_compensate_reg.regValue);
			rtd_outl(DI_DI_SMD_CandidateLimitSAD_reg, init_di_smd_candidatelimitsad_reg.regValue);
			rtd_outl(DI_DI_SMD_AdditionalCoeff2_reg, init_di_smd_additionalcoeff2_reg.regValue);
			rtd_outl(DI_DI_SMD_dummy_reg, di_smd_dummy_reg.regValue);

			rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_2_reg, di_hmc_pan_control_par_2_reg.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle1_reg, di_di_smd_slow_motion_handle1.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle2_reg, di_di_smd_slow_motion_handle2.regValue);

			rtd_outl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, im_di_weave_window_control_reg.regValue);
			rtd_outl(DI_IM_DI_WEAVE_WINDOW_1_START_reg, im_di_weave_window_1_start_reg.regValue);
			rtd_outl(DI_IM_DI_WEAVE_WINDOW_1_END_reg, im_di_weave_window_1_end_reg.regValue);


			rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_1_reg,di_hmc_pan_control_par_1_reg.regValue);
			rtd_outl(DI_IM_DI_FRAMESOBEL_STATISTIC_reg,FrameSobel_statistic_Reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg, di_hmc_adjustable_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE2_reg, di_hmc_adjustable2_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE3_reg, di_hmc_adjustable3_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE4_reg, di_hmc_adjustable4_reg.regValue);


			//#139
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, di_ma_frame_motion_th_c_reg.regValue);
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
			//rtd_outl(NR_CURVE_MAPPING_W1_2_reg, curve_mapping_w1_2_reg.regValue);			//LGE control
			//rtd_outl(NR_CURVE_MAPPING_STEP_3_reg, curve_mapping_step3_reg.regValue);		//LGE control

			//#147,148,149,150
			rtd_outl(NR_DCH1_Spatial_Weight_reg, dch1_Spatial_Weight_Reg.regValue);

			//#151

			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);

			rtd_outl(SCALEUP_DM_DIR_UZU_Conf_reg, dm_dir_uzu_conf_Reg.regValue);
			rtd_outl(DI_IM_DI_NINTRA_CONTROL_reg,di_im_di_nintra_control.regValue);

			//#188
			rtd_outl(DI_IM_DI_BTR_CONTROL_reg, di_im_di_btr_control_reg.regValue);
			rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg, init_di_smd_additionalcoeff1_reg.regValue);

			//#189
			rtd_outl(DI_IM_DI_HMC_STATISTIC1_reg, di_hmc_statistic1_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg, di_hmc_adjustable_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE2_reg, di_hmc_adjustable2_reg.regValue);
			rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg, init_di_smd_additionalcoeff1_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_1_reg,di_hmc_pan_control_par_1_reg.regValue);

			//#192
			//rtd_outl(NR_CURVE_MAPPING_W1_1_reg, curve_mapping_w1_1_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_W1_2_reg, curve_mapping_w1_2_reg.regValue);		//LGE control
			rtd_outl(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);

			// VPAN
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);

			// VD Setting related register control, created by willy, 20151114
			if( DynamicOptimizeSystem[1] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_1);

			if( DynamicOptimizeSystem[2] == 1 )
			{
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_2);
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_DOS_2_TNR_XC);
			}

			if( DynamicOptimizeSystem[7] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_7);

			if( DynamicOptimizeSystem[11] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_11);

			if( DynamicOptimizeSystem[12] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_12);

			if( DynamicOptimizeSystem[28] != 0 )
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_28);

			if( DynamicOptimizeSystem[63] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_63);

			if( DynamicOptimizeSystem[107] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_107);

			if( DynamicOptimizeSystem[123] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_123);

			if( DynamicOptimizeSystem[134] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_134);

			if( DynamicOptimizeSystem[139] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_139);

			if( DynamicOptimizeSystem[189] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_NTSC_VIP_DOS_LEAF_72);

		}
		break;

		case VIP_QUALITY_CVBS_PAL:
		{
			if(!vd_pwr_ok)
				break;
			//************** INI VALE START    ***************************************************
			//#1
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			di_smd_control_candiate_reg.smd_en=ori_smd_en;
			smd_compensate_reg.smd_compensate_teeth_th =ori_smd_compensate_reg.smd_compensate_teeth_th;

			//#2
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
	            //di_ma_frame_motion_th_a_reg.ma_framerlvtha = ORI_di_ma_frame_motion_th_a_reg.ma_framerlvtha; // 20160202, willy, smooth the noisy line, removed by MissKim
	            //di_ma_frame_motion_th_b_reg.ma_framerlvthb = ORI_di_ma_frame_motion_th_b_reg.ma_framerlvthb;
	            //di_ma_frame_motion_th_c_reg.ma_framerlvthc = ORI_di_ma_frame_motion_th_c_reg.ma_framerlvthc;
	            //dm_dir_uzu_conf_Reg.conf_lowbnd = ori_conf_lowbnd;

			//#7
#if 0
			di_pan_dummy2_reg.panConti_str = ORI_di_pan_dummy2_reg.panConti_str;
			di_pan_dummy3_reg.panDeb_str = ORI_di_pan_dummy3_reg.panDeb_str;
#else // henry Merlin3 temp
			di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | panConti_str;
			di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | panDeb_str;
#endif

			//#12
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=ori_smd_upperbound_sad_forjudgemv;
			smd_compensate_reg.smd_compensate_teeth_th =ori_smd_compensate_reg.smd_compensate_teeth_th;

			//#23 #24

			if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
			{
				di_double_buff_apply =1;
			}
			init_di_smd_additionalcoeff1_reg.smd_result_en=ori_smd_result_en;

			//#28
			di_im_di_nintra_lowpass_source.nintra_lp_src_yclamp_th = ori_nintra_lp_src_yclamp_th;
			di_im_di_nintra_search_dir_limit.regValue=ori_di_im_di_nintra_search_dir_limit.regValue;
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=ori_smd_upperbound_sad_forjudgemv;

			//#39
			di_control_reg.film_enable = ori_film_enable;
			di_control_reg.write_enable_5 =1;

			//#63
			di_di_smd_slow_motion_handle1.smd_acc_smd_en =ori_smd_acc_smd_en;
			di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=ori_smd_spatialfreqnomatch_off;
			di_di_smd_slow_motion_handle1.smd_weave_dynamic = ori_smd_weave_dynamic;
			// di_im_new_mcnr_control.n_mcnr_offset ; don't set value in here, it control by mcnr

			//#75
#if 0
			di_pan_dummy2_reg.panConti_str = ORI_di_pan_dummy2_reg.panConti_str;
			di_pan_dummy3_reg.panDeb_str = ORI_di_pan_dummy3_reg.panDeb_str;
#else // henry Merlin3 temp
			di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | panConti_str;
			di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | panDeb_str;
#endif
			di_hmc_adjustable2_reg.hmc_mv_refine_mor_noise_th = ORI_di_hmc_adjustable2_reg.hmc_mv_refine_mor_noise_th;

			//#107
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=ori_smd_upperbound_sad_forjudgemv;
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			smd_refinemv3_reg.smd_sfreq_diff_gain =4;
			smd_refinemv3_reg.smd_sfreq_diff_th =25;
			smd_refinemv3_reg.smd_sfreq_drop_th =80;
			smd_compensate_reg.smd_compensate_teeth_th =ori_smd_compensate_reg.smd_compensate_teeth_th;
			smd_compensate_reg.smd_compensate_avg = ori_smd_compensate_reg.smd_compensate_avg;

			init_di_smd_candidatelimitsad_reg.smd_gmv_sad_max=0xd2;
			init_di_smd_candidatelimitsad_reg.smd_gmv_sad_th=0x40;

			init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain=gSmd_gain;
			init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset=gSmd_offset;
			di_smd_dummy_reg.smd_partialweave =0;

			di_hmc_pan_control_par_2_reg.hmc_sub_pixel_diff_thd = ori_hmc_sub_pixel_diff_thd;

			di_di_smd_slow_motion_handle1.gst_min_sad_nolimit_en =ori_gst_min_sad_nolimit_en;
			di_di_smd_slow_motion_handle1.gst_gst_static_disable =ori_gst_gst_static_disable;
			di_di_smd_slow_motion_handle1.smd_weave_dynamic = ori_smd_weave_dynamic;
			di_di_smd_slow_motion_handle1.smd_outofrange_content_new = ori_smd_outofrange_content_new;
			di_di_smd_slow_motion_handle2.gst_mv_limit = ori_gst_mv_limit;
			//di_di_smd_slow_motion_handle1.smd_acc_smd_en =ori_smd_acc_smd_en;
			//di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=ori_smd_spatialfreqnomatch_off;

			im_di_weave_window_control_reg.fixedwindow1 = ORI_im_di_weave_window_control_reg.fixedwindow1;
			im_di_weave_window_control_reg.fixedwindowmode = ORI_im_di_weave_window_control_reg.fixedwindowmode;
			im_di_weave_window_1_start_reg.regValue = ORI_im_di_weave_window_1_start_reg.regValue;
			im_di_weave_window_1_end_reg.regValue = ORI_im_di_weave_window_1_end_reg.regValue;

			di_hmc_statistic1_reg.regValue = ORI_di_hmc_statistic1_reg.regValue;
			di_hmc_pan_control_par_1_reg.hme_sum_max_ratio = ORI_di_hmc_pan_control_par_1_reg.hme_sum_max_ratio;
			FrameSobel_statistic_Reg.regValue = ORI_FrameSobel_statistic_Reg.regValue;
			di_hmc_adjustable_reg.regValue= ORI_di_hmc_adjustable_reg.regValue;
			di_hmc_adjustable2_reg.regValue= ORI_di_hmc_adjustable2_reg.regValue;
			di_hmc_adjustable3_reg.regValue= ORI_di_hmc_adjustable3_reg.regValue;
			di_hmc_adjustable4_reg.regValue= ORI_di_hmc_adjustable4_reg.regValue;




			//#139
			di_ma_frame_motion_th_a_reg.ma_framestha = ORI_di_ma_frame_motion_th_a_reg.ma_framestha;
			di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;
			di_ma_frame_motion_th_a_reg.ma_framerlvtha = ORI_di_ma_frame_motion_th_a_reg.ma_framerlvtha;
			di_ma_frame_motion_th_a_reg.ma_framemtha = ORI_di_ma_frame_motion_th_a_reg.ma_framemtha;
			di_ma_frame_motion_th_b_reg.ma_framesthb = ORI_di_ma_frame_motion_th_b_reg.ma_framesthb;
			di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;
			di_ma_frame_motion_th_c_reg.ma_framesthc = ORI_di_ma_frame_motion_th_c_reg.ma_framesthc;
			di_ma_frame_motion_th_c_reg.ma_framehthc = ORI_di_ma_frame_motion_th_c_reg.ma_framehthc;
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = ORI_di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv;

			dch1_cp_ctrl_reg.sresultweight_adjust_shift = ori_sresultweight_adjust_shift;
			dch1_cp_ctrl_reg.vc_snr_lpf_sel = ori_vc_snr_lpf_sel;
			curve_mapping_w1_2_reg.curvemapping_w1_6 = ori_curve_mapping_w1_6;
			curve_mapping_w1_2_reg.curvemapping_w1_7 = ori_curve_mapping_w1_7;
			curve_mapping_w1_3_reg.curvemapping_w1_8 = ori_curve_mapping_w1_8;
			curve_mapping_step3_reg.curvemapping_step8=ori_curve_mapping_step8;

			//#147,148,149,150
			dch1_Spatial_Weight_Reg.cp_fixedweight1y = ori_cp_fixedweight1y;
			dch1_Spatial_Weight_Reg.cp_fixedweight2y = ori_cp_fixedweight2y;

			//#151

			di_ma_frame_motion_th_a_reg.ma_framestha = ORI_di_ma_frame_motion_th_a_reg.ma_framestha;
			di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;
			di_ma_frame_motion_th_a_reg.ma_framerlvtha = ORI_di_ma_frame_motion_th_a_reg.ma_framerlvtha;
			di_ma_frame_motion_th_a_reg.ma_framemtha = ORI_di_ma_frame_motion_th_a_reg.ma_framemtha;
			di_ma_frame_motion_th_b_reg.ma_framesthb = ORI_di_ma_frame_motion_th_b_reg.ma_framesthb;
			di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;
			di_ma_frame_motion_th_b_reg.ma_framerlvthb = ORI_di_ma_frame_motion_th_b_reg.ma_framerlvthb;
			di_ma_frame_motion_th_b_reg.ma_framemthb = ORI_di_ma_frame_motion_th_b_reg.ma_framemthb;


			dm_dir_uzu_conf_Reg.conf_lowbnd = ori_conf_lowbnd;
			di_im_di_nintra_control.nintra_intersection_check_en =ori_di_im_di_nintra_control.nintra_intersection_check_en;
			di_im_di_nintra_control.nintra_preline_dir_check2_en = ori_di_im_di_nintra_control.nintra_preline_dir_check2_en;

			//#188
			di_im_di_btr_control_reg.btr_blendtemporalrecoveryen = ORI_di_im_di_btr_control_reg.btr_blendtemporalrecoveryen;

			//#189
			di_hmc_statistic1_reg.ma_hmc_lineratio = ORI_di_hmc_statistic1_reg.ma_hmc_lineratio;
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = ORI_di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass;
			di_hmc_adjustable_reg.hmc_mv_refine_en = ORI_di_hmc_adjustable_reg.hmc_mv_refine_en;
			di_hmc_adjustable2_reg.hme_x_continued_counter = ORI_di_hmc_adjustable2_reg.hme_x_continued_counter;
			init_di_smd_additionalcoeff1_reg.smd_result_en = ori_smd_result_en;
			di_hmc_pan_control_par_1_reg.hme_sum_max_ratio = ORI_di_hmc_pan_control_par_1_reg.hme_sum_max_ratio;

			//#192

			dch1_cp_ctrl_reg.cp_sresultweight =ori_cp_sresultweight;
			curve_mapping_w1_1_reg.curvemapping_w1_3 =ori_curve_mapping_w1_3;
			curve_mapping_w1_2_reg.curvemapping_w1_4 =ori_curve_mapping_w1_4;
			curve_mapping_w1_2_reg.curvemapping_w1_5 =ori_curve_mapping_w1_5;

			//VPAN case
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = ORI_di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv;

			//************** INI VALE END    ***************************************************

			if(DynamicOptimizeSystem[1] ==1)
			{
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 200;
				smd_compensate_reg.smd_compensate_teeth_th =200;

			}
			else if(DynamicOptimizeSystem[2] ==1)
			{
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
		                //di_ma_frame_motion_th_a_reg.ma_framerlvtha = 1; // 20160202, willy, smooth the noisy line, removed by MissKim
		                //di_ma_frame_motion_th_b_reg.ma_framerlvthb = 1;
		                //di_ma_frame_motion_th_c_reg.ma_framerlvthc = 1;
		                //dm_dir_uzu_conf_Reg.conf_lowbnd = 50;
			}
			else if(DynamicOptimizeSystem[7] ==1)
			{
#if 0
				di_pan_dummy2_reg.panConti_str = 12;
				di_pan_dummy3_reg.panDeb_str = 80;
#else // henry Merlin3 temp
				di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | 12;
				di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | 80;
#endif

			}
			else if(DynamicOptimizeSystem[12] ==1)
			{
				//drvif_color_newrtnr_sad_th(1,&Motion_Th[0]);//for sad edge th setting
				//drvif_color_newrtnr_sad_th(0,&Motion_Th[0]);//for sad nonedge th setting
				drvif_color_rtnr_y_th_isr(_CHANNEL1,&Motion_Th[0]);

				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 200;
				smd_compensate_reg.smd_compensate_teeth_th =200;

			}
			else if((DynamicOptimizeSystem[23]==1)||(DynamicOptimizeSystem[24]==1))
			{

				if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
				{
					di_double_buff_apply =1;
				}
				init_di_smd_additionalcoeff1_reg.smd_result_en=0;
			}
			else if(DynamicOptimizeSystem[28] ==1)
			{
				di_im_di_nintra_lowpass_source.nintra_lp_src_yclamp_th = 30;
				di_im_di_nintra_search_dir_limit.nintra_hdiff_th1=16;
				di_im_di_nintra_search_dir_limit.nintra_hdiff_th2=16;
			}
			else if(DynamicOptimizeSystem[28] == 2)
			{
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 255;
                		dm_dir_uzu_conf_Reg.conf_lowbnd = 30;
			}
			else if(DynamicOptimizeSystem[39] == 1)
			{
				di_control_reg.film_enable = 17;
				di_control_reg.write_enable_5=1;
			}
			else if(DynamicOptimizeSystem[63] ==1)
			{
				di_di_smd_slow_motion_handle1.smd_acc_smd_en =1;
				di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=1;
				di_di_smd_slow_motion_handle1.smd_weave_dynamic = 3;

// chen 170522
	//			di_im_new_mcnr_control.n_mcnr_offset =0;
// end chen 170522

				// 20160106 jimmy, turn up RTNR C for cloth cross color
				drvif_color_rtnr_c_th_isr(_CHANNEL1,&Motion_Th_c[0]);

			}
			else if(DynamicOptimizeSystem[75] ==1)
			{
				drvif_color_rtnr_c_th_isr(_CHANNEL1,&Motion_Th_c_can[0]);
#if 0
				di_pan_dummy2_reg.panConti_str = 12;
				di_pan_dummy3_reg.panDeb_str = 80;
#else // henry Merlin3 temp
				di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | 12;
				di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | 80;
#endif
				di_hmc_adjustable2_reg.hmc_mv_refine_mor_noise_th = 13; // special HMC setting for LG#75, jimmy 20151203
			}
			else if(DynamicOptimizeSystem[107]==1)
			{
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 170;//125; willy, 20151224
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 10;//0; willy, 20151224
				smd_refinemv3_reg.smd_sfreq_diff_gain =0;
				smd_refinemv3_reg.smd_sfreq_diff_th =0;
				smd_refinemv3_reg.smd_sfreq_drop_th =0;
				smd_compensate_reg.smd_compensate_teeth_th =5;
                		smd_compensate_reg.smd_compensate_avg = 148; // willy, 20151224

				init_di_smd_candidatelimitsad_reg.smd_gmv_sad_max=0xff;
				init_di_smd_candidatelimitsad_reg.smd_gmv_sad_th=0x7f;

				init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain = 6;//2; willy, 20151224
				init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset=0x1f;
				di_smd_dummy_reg.smd_partialweave =3;

				di_hmc_adjustable_reg.hmc_mv_refine_mor_en = 1;
				di_hmc_adjustable_reg.hmc_mv_refine_y_th = 160;
				di_hmc_pan_control_par_2_reg.hmc_sub_pixel_diff_thd = 160;


				di_di_smd_slow_motion_handle1.gst_min_sad_nolimit_en =1;
				di_di_smd_slow_motion_handle1.gst_gst_static_disable =1;
				di_di_smd_slow_motion_handle1.smd_weave_dynamic = 0;//3; willy, 20151224
				di_di_smd_slow_motion_handle1.smd_outofrange_content_new = 0;
				di_di_smd_slow_motion_handle2.gst_mv_limit = 1;
				//di_di_smd_slow_motion_handle1.smd_acc_smd_en =1;
				//di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=1;



				di_hmc_statistic1_reg.ma_hmc_lineratio = 24;
				rtd_outl(DI_IM_DI_HMC_STATISTIC1_reg, di_hmc_statistic1_reg.regValue);
				di_hmc_pan_control_par_1_reg.hme_sum_max_ratio = 18;

				FrameSobel_statistic_Reg.regValue = 0xc51f4200;
				di_hmc_adjustable_reg.regValue= 0x8FFD0004;
				di_hmc_adjustable2_reg.regValue= 0x0732c014; // special HMC setting for LG#75 and LG#107, jimmy 20151203
				di_hmc_adjustable3_reg.regValue= 0x6819141e;
				di_hmc_adjustable4_reg.regValue= 0x850a0c0c;



		                // weave window to do intra
		                im_di_weave_window_control_reg.fixedwindow1 = 1;
		                im_di_weave_window_control_reg.fixedwindowmode = 0;
		                im_di_weave_window_1_start_reg.regValue = 0x00500033;
		                im_di_weave_window_1_end_reg.regValue = 0x00a700d3;

			}
			else if(DynamicOptimizeSystem[139] ==1)
			{
				di_ma_frame_motion_th_a_reg.ma_framestha = 20;
				di_ma_frame_motion_th_a_reg.ma_framehtha = 20;
				di_ma_frame_motion_th_a_reg.ma_framerlvtha = 20;
				di_ma_frame_motion_th_a_reg.ma_framemtha = 50;
				di_ma_frame_motion_th_b_reg.ma_framesthb = 20;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 20;
				di_ma_frame_motion_th_c_reg.ma_framesthc = 20;
				di_ma_frame_motion_th_c_reg.ma_framehthc = 20;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 0;

				dch1_cp_ctrl_reg.sresultweight_adjust_shift = 0;
				dch1_cp_ctrl_reg.vc_snr_lpf_sel = 0;
				curve_mapping_w1_2_reg.curvemapping_w1_6 = 3;
				curve_mapping_w1_2_reg.curvemapping_w1_7 = 3;
				curve_mapping_w1_3_reg.curvemapping_w1_8 = 6;
				curve_mapping_step3_reg.curvemapping_step8=42;

			}
			else if(DynamicOptimizeSystem[147])
			{
				dch1_Spatial_Weight_Reg.cp_fixedweight1y = 1;
				dch1_Spatial_Weight_Reg.cp_fixedweight2y = 1;
			}
			else if(DynamicOptimizeSystem[151])
			{

				di_ma_frame_motion_th_a_reg.ma_framestha = 50;
				di_ma_frame_motion_th_a_reg.ma_framehtha = 50;
				di_ma_frame_motion_th_a_reg.ma_framerlvtha = 50;
				di_ma_frame_motion_th_a_reg.ma_framemtha = 50;
				di_ma_frame_motion_th_b_reg.ma_framesthb = 50;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 50;
				di_ma_frame_motion_th_b_reg.ma_framerlvthb = 50;
				di_ma_frame_motion_th_b_reg.ma_framemthb = 50;

				dm_dir_uzu_conf_Reg.conf_lowbnd = 63;
				di_im_di_nintra_control.nintra_intersection_check_en =0;
				di_im_di_nintra_control.nintra_preline_dir_check2_en = 0;
			}
			else if(DynamicOptimizeSystem[188])
			{
				di_im_di_btr_control_reg.btr_blendtemporalrecoveryen = 0;
			}
			else if(DynamicOptimizeSystem[189])
			{
				di_hmc_statistic1_reg.ma_hmc_lineratio += 24;
				di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
				di_hmc_adjustable_reg.hmc_mv_refine_en = 0;
				di_hmc_adjustable2_reg.hme_x_continued_counter = 0;

				if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
				{
					di_double_buff_apply =1;
				}
				init_di_smd_additionalcoeff1_reg.smd_result_en = 0;
				di_hmc_pan_control_par_1_reg.hme_sum_max_ratio = 12;

				rtd_outl(DI_IM_DI_HMC_ME_REFINE_CTRL_2_reg, di_hmc_me_refine_ctrl2_reg.regValue);

			}
			else if(DynamicOptimizeSystem[192])
			{
				dch1_cp_ctrl_reg.cp_sresultweight =15;
				curve_mapping_w1_1_reg.curvemapping_w1_3 =8;
				curve_mapping_w1_2_reg.curvemapping_w1_4 =0;
				curve_mapping_w1_2_reg.curvemapping_w1_5 =0;
			}
			else if(scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN) >= 2)
			{
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 0;
			}


			//#1
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
			rtd_outl(DI_DI_SMD_Compensate_reg, smd_compensate_reg.regValue);

			//#2
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
            //rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue); // 20160202, willy, smooth the noisy line, removed by MissKim
			//rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);
			//rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, di_ma_frame_motion_th_c_reg.regValue);

			//#7

			//#23 #24
			rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg, init_di_smd_additionalcoeff1_reg.regValue);

			//#28
			rtd_outl(DI_IM_DI_NINTRA_LOWPASS_SOURCE_reg,di_im_di_nintra_lowpass_source.regValue);
			rtd_outl(DI_IM_DI_NINTRA_SEARCH_DIR_LIMIT_reg,di_im_di_nintra_search_dir_limit.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);

            //#39
            //rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);//rord.tsao mark don't change di seeting avoid 3a/5a setting error 2016/02/19

			//#63

			// protect di 3Abit,reduce r/w time .
			di_control_reg_Prtoect_3AEnable.regValue= rtd_inl(DI_IM_DI_CONTROL_reg);
			di_control_reg.ma_3aenable = di_control_reg_Prtoect_3AEnable.ma_3aenable;
			di_control_reg.write_enable_1=1;

			//rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);//rord.tsao mark don't change di seeting avoid 3a/5a setting error 2016/02/19
			rtd_outl(DI_IM_DI_INTRA_TH_reg, intra_th_Reg.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle1_reg, di_di_smd_slow_motion_handle1.regValue);
			rtd_outl(DI_IM_NEW_MCNR_CONTROL_reg,di_im_new_mcnr_control.regValue);

			//#75
#if 0
			rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_2_reg, di_pan_dummy2_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_3_reg, di_pan_dummy3_reg.regValue);
#else // henry Merlin3 temp
			rtd_outl(DI_IM_DI_RTNR_REFINED_CTRL2_reg, di_im_di_rtnr_refined_ctrl2_reg.regValue);
			rtd_outl(DI_DI_SMD_dummy_reg, di_di_smd_dummy_reg.regValue);
#endif
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE2_reg, di_hmc_adjustable2_reg.regValue);

			//#107

			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
			rtd_outl(DI_DI_SMD_RefineMV3_reg, smd_refinemv3_reg.regValue);
			rtd_outl(DI_DI_SMD_Compensate_reg, smd_compensate_reg.regValue);
			rtd_outl(DI_DI_SMD_CandidateLimitSAD_reg, init_di_smd_candidatelimitsad_reg.regValue);
			rtd_outl(DI_DI_SMD_AdditionalCoeff2_reg, init_di_smd_additionalcoeff2_reg.regValue);
			rtd_outl(DI_DI_SMD_dummy_reg, di_smd_dummy_reg.regValue);

			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg, di_hmc_adjustable_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_2_reg, di_hmc_pan_control_par_2_reg.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle1_reg, di_di_smd_slow_motion_handle1.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle2_reg, di_di_smd_slow_motion_handle2.regValue);


			rtd_outl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, im_di_weave_window_control_reg.regValue);
			rtd_outl(DI_IM_DI_WEAVE_WINDOW_1_START_reg, im_di_weave_window_1_start_reg.regValue);
			rtd_outl(DI_IM_DI_WEAVE_WINDOW_1_END_reg, im_di_weave_window_1_end_reg.regValue);


			rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_1_reg,di_hmc_pan_control_par_1_reg.regValue);
			rtd_outl(DI_IM_DI_FRAMESOBEL_STATISTIC_reg,FrameSobel_statistic_Reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg, di_hmc_adjustable_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE2_reg, di_hmc_adjustable2_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE3_reg, di_hmc_adjustable3_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE4_reg, di_hmc_adjustable4_reg.regValue);


			//#139
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, di_ma_frame_motion_th_c_reg.regValue);
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
			//rtd_outl(NR_CURVE_MAPPING_W1_2_reg, curve_mapping_w1_2_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_W1_3_reg, curve_mapping_w1_3_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_STEP_3_reg, curve_mapping_step3_reg.regValue);	//LGE control

			//#147,148,149,150
			rtd_outl(NR_DCH1_Spatial_Weight_reg, dch1_Spatial_Weight_Reg.regValue);

			//#151

			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);

			rtd_outl(SCALEUP_DM_DIR_UZU_Conf_reg, dm_dir_uzu_conf_Reg.regValue);
			rtd_outl(DI_IM_DI_NINTRA_CONTROL_reg,di_im_di_nintra_control.regValue);

			//#188
			rtd_outl(DI_IM_DI_BTR_CONTROL_reg, di_im_di_btr_control_reg.regValue);

			//#189
			rtd_outl(DI_IM_DI_HMC_STATISTIC1_reg, di_hmc_statistic1_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg, di_hmc_adjustable_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE2_reg, di_hmc_adjustable2_reg.regValue);
			rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg, init_di_smd_additionalcoeff1_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_1_reg,di_hmc_pan_control_par_1_reg.regValue);

			//#192
			//rtd_outl(NR_CURVE_MAPPING_W1_1_reg, curve_mapping_w1_1_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_W1_2_reg, curve_mapping_w1_2_reg.regValue);		//LGE control
			rtd_outl(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);

			// VPAN
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);

			// VD Setting related register control, created by willy, 20151114
			if( DynamicOptimizeSystem[2] == 1 )
			{
				scalerVideo_ApplyVDSetting(VDPQ_PAL_VIP_DOS_2);
				scalerVideo_ApplyVDSetting(VDPQ_PAL_DOS_2_TNR_XC);
			}

			if( DynamicOptimizeSystem[7] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_PAL_VIP_DOS_7);

			if( DynamicOptimizeSystem[11] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_PAL_VIP_DOS_11);

			if( DynamicOptimizeSystem[12] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_PAL_VIP_DOS_12);

			if( DynamicOptimizeSystem[28] != 0 )
				scalerVideo_ApplyVDSetting(VDPQ_PAL_VIP_DOS_28);

			if( DynamicOptimizeSystem[63] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_PAL_VIP_DOS_63);

			if( DynamicOptimizeSystem[107] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_PAL_VIP_DOS_107);

			if( DynamicOptimizeSystem[123] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_PAL_VIP_DOS_123);

			if( DynamicOptimizeSystem[134] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_PAL_VIP_DOS_134);

			if( DynamicOptimizeSystem[139] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_PAL_VIP_DOS_139);

			if( DynamicOptimizeSystem[189] == 1 )
				scalerVideo_ApplyVDSetting(VDPQ_PAL_VIP_DOS_LEAF_74);

		}
		break;

		case VIP_QUALITY_HDMI_480I:
		case VIP_QUALITY_YPbPr_480I:
		{

			//************** INI VALE START    ***************************************************
			//#1
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			di_smd_control_candiate_reg.smd_en=ori_smd_en;
			smd_compensate_reg.smd_compensate_teeth_th =ori_smd_compensate_reg.smd_compensate_teeth_th;

			//#2
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			di_di_smd_slow_motion_handle1.gst_gst_static_disable = ori_gst_gst_static_disable;
			di_di_smd_slow_motion_handle1.gst_min_sad_compare_diable =ori_gst_min_sad_compare_diable;

			//#7
#if 0
			di_pan_dummy2_reg.panConti_str = ORI_di_pan_dummy2_reg.panConti_str;
			di_pan_dummy3_reg.panDeb_str = ORI_di_pan_dummy3_reg.panDeb_str;
#else // henry Merlin3 temp
			di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | panConti_str;
			di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | panDeb_str;
#endif

			//#12
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=ori_smd_upperbound_sad_forjudgemv;
			smd_compensate_reg.smd_compensate_teeth_th =ori_smd_compensate_reg.smd_compensate_teeth_th;

			//#23 #24

			if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
			{
				di_double_buff_apply =1;
			}
			init_di_smd_additionalcoeff1_reg.smd_result_en=ori_smd_result_en;

			//#28
			di_im_di_nintra_lowpass_source.nintra_lp_src_yclamp_th = ori_nintra_lp_src_yclamp_th;
			di_im_di_nintra_search_dir_limit.regValue=ori_di_im_di_nintra_search_dir_limit.regValue;

			//#63
			di_di_smd_slow_motion_handle1.smd_acc_smd_en =ori_smd_acc_smd_en;
			di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=ori_smd_spatialfreqnomatch_off;
			di_di_smd_slow_motion_handle1.smd_weave_dynamic = ori_smd_weave_dynamic;

			//#75
#if 0
			di_pan_dummy2_reg.panConti_str = ORI_di_pan_dummy2_reg.panConti_str;
			di_pan_dummy3_reg.panDeb_str = ORI_di_pan_dummy3_reg.panDeb_str;
#else // henry Merlin3 temp
			di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | panConti_str;
			di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | panDeb_str;
#endif
			//#107
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=ori_smd_upperbound_sad_forjudgemv;
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			smd_refinemv3_reg.smd_sfreq_diff_gain =4;
			smd_refinemv3_reg.smd_sfreq_diff_th =25;
			smd_refinemv3_reg.smd_sfreq_drop_th =80;
			smd_compensate_reg.smd_compensate_teeth_th =ori_smd_compensate_reg.smd_compensate_teeth_th;
			smd_compensate_reg.smd_compensate_avg = ori_smd_compensate_reg.smd_compensate_avg;

			init_di_smd_candidatelimitsad_reg.smd_gmv_sad_max=0xd2;
			init_di_smd_candidatelimitsad_reg.smd_gmv_sad_th=0x40;
			init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain=gSmd_gain;
			init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset=gSmd_offset;
			di_smd_dummy_reg.smd_partialweave =0;

			di_hmc_pan_control_par_2_reg.hmc_sub_pixel_diff_thd = ori_hmc_sub_pixel_diff_thd;

			di_di_smd_slow_motion_handle1.gst_min_sad_nolimit_en =ori_gst_min_sad_nolimit_en;
			di_di_smd_slow_motion_handle1.gst_gst_static_disable =ori_gst_gst_static_disable;
			di_di_smd_slow_motion_handle1.smd_weave_dynamic = ori_smd_weave_dynamic;
			di_di_smd_slow_motion_handle1.smd_outofrange_content_new = ori_smd_outofrange_content_new;
			di_di_smd_slow_motion_handle2.gst_mv_limit = ori_gst_mv_limit;
			//di_di_smd_slow_motion_handle1.smd_acc_smd_en =ori_smd_acc_smd_en;
			//di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=ori_smd_spatialfreqnomatch_off;

			di_hmc_statistic1_reg.regValue = ORI_di_hmc_statistic1_reg.regValue;
			di_hmc_pan_control_par_1_reg.hme_sum_max_ratio = ORI_di_hmc_pan_control_par_1_reg.hme_sum_max_ratio;
			FrameSobel_statistic_Reg.regValue = ORI_FrameSobel_statistic_Reg.regValue;
			di_hmc_adjustable_reg.regValue = ORI_di_hmc_adjustable_reg.regValue;
			di_hmc_adjustable2_reg.regValue= ORI_di_hmc_adjustable2_reg.regValue;
			di_hmc_adjustable3_reg.regValue= ORI_di_hmc_adjustable3_reg.regValue;
			di_hmc_adjustable4_reg.regValue= ORI_di_hmc_adjustable4_reg.regValue;




			//#139
			di_ma_frame_motion_th_a_reg.ma_framestha = ORI_di_ma_frame_motion_th_a_reg.ma_framestha;
			di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;
			di_ma_frame_motion_th_a_reg.ma_framerlvtha = ORI_di_ma_frame_motion_th_a_reg.ma_framerlvtha;
			di_ma_frame_motion_th_a_reg.ma_framemtha = ORI_di_ma_frame_motion_th_a_reg.ma_framemtha;
			di_ma_frame_motion_th_b_reg.ma_framesthb = ORI_di_ma_frame_motion_th_b_reg.ma_framesthb;
			di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;
			di_ma_frame_motion_th_c_reg.ma_framesthc = ORI_di_ma_frame_motion_th_c_reg.ma_framesthc;
			di_ma_frame_motion_th_c_reg.ma_framehthc = ORI_di_ma_frame_motion_th_c_reg.ma_framehthc;
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = ORI_di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv;

			dch1_cp_ctrl_reg.sresultweight_adjust_shift = ori_sresultweight_adjust_shift;
			dch1_cp_ctrl_reg.vc_snr_lpf_sel = ori_vc_snr_lpf_sel;
			curve_mapping_w1_2_reg.curvemapping_w1_6 = ori_curve_mapping_w1_6;
			curve_mapping_w1_2_reg.curvemapping_w1_7 = ori_curve_mapping_w1_7;
			//curve_mapping_w1_3_reg.curvemapping_w1_8 = ori_curve_mapping_w1_8;

			curve_mapping_step3_reg.curvemapping_step8=ori_curve_mapping_step8;

			//#147,148,149,150
			dch1_Spatial_Weight_Reg.cp_fixedweight1y = ori_cp_fixedweight1y;
			dch1_Spatial_Weight_Reg.cp_fixedweight2y = ori_cp_fixedweight2y;

			//#151

			di_ma_frame_motion_th_a_reg.ma_framestha = ORI_di_ma_frame_motion_th_a_reg.ma_framestha;
			di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;
			di_ma_frame_motion_th_a_reg.ma_framerlvtha = ORI_di_ma_frame_motion_th_a_reg.ma_framerlvtha;
			di_ma_frame_motion_th_a_reg.ma_framemtha = ORI_di_ma_frame_motion_th_a_reg.ma_framemtha;
			di_ma_frame_motion_th_b_reg.ma_framesthb = ORI_di_ma_frame_motion_th_b_reg.ma_framesthb;
			di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;
			di_ma_frame_motion_th_b_reg.ma_framerlvthb = ORI_di_ma_frame_motion_th_b_reg.ma_framerlvthb;
			di_ma_frame_motion_th_b_reg.ma_framemthb = ORI_di_ma_frame_motion_th_b_reg.ma_framemthb;

			dm_dir_uzu_conf_Reg.conf_lowbnd = ori_conf_lowbnd;
			di_im_di_nintra_control.nintra_intersection_check_en =ori_di_im_di_nintra_control.nintra_intersection_check_en;
			di_im_di_nintra_control.nintra_preline_dir_check2_en = ori_di_im_di_nintra_control.nintra_preline_dir_check2_en;

			//#152
			di_hmc_adjustable_reg.hmc_mv_refine_en =ORI_di_hmc_adjustable_reg.hmc_mv_refine_en;
			di_hmc_adjustable_reg.mc_dynamic_threshold_clip_en = ORI_di_hmc_adjustable_reg.mc_dynamic_threshold_clip_en;

			//#192
			dch1_cp_ctrl_reg.cp_sresultweight = ori_cp_sresultweight;
			curve_mapping_w1_1_reg.curvemapping_w1_2=ori_curve_mapping_w1_2;
			curve_mapping_w1_1_reg.curvemapping_w1_3=ori_curve_mapping_w1_3;
			curve_mapping_w1_2_reg.curvemapping_w1_4=ori_curve_mapping_w1_4;
			curve_mapping_w1_2_reg.curvemapping_w1_5=ori_curve_mapping_w1_5;
			curve_mapping_step2_reg.curvemapping_step6=ori_curve_mapping_step6;
			curve_mapping_step3_reg.curvemapping_step7=ori_curve_mapping_step7;
			curve_mapping_step3_reg.curvemapping_step8=ori_curve_mapping_step8;

			//************** INI VALE END    ***************************************************

			if(DynamicOptimizeSystem[1] ==1)
			{
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 200;
				smd_compensate_reg.smd_compensate_teeth_th =200;
			}
			else if(DynamicOptimizeSystem[2] ==1)
			{
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				di_di_smd_slow_motion_handle1.gst_gst_static_disable = 1;
				di_di_smd_slow_motion_handle1.gst_min_sad_compare_diable =1;
			}
			else if(DynamicOptimizeSystem[7] ==1)
			{
#if 0
				di_pan_dummy2_reg.panConti_str = 12;
				di_pan_dummy3_reg.panDeb_str = 80;
#else // henry Merlin3 temp
				di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | 12;
				di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | 80;
#endif

			}
			else if(DynamicOptimizeSystem[12] ==1)
			{
				//drvif_color_newrtnr_sad_th(1,&Motion_Th[0]);//for sad edge th setting
				//drvif_color_newrtnr_sad_th(0,&Motion_Th[0]);//for sad nonedge th setting
				drvif_color_rtnr_y_th_isr(_CHANNEL1,&Motion_Th[0]);



				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 200;
				smd_compensate_reg.smd_compensate_teeth_th =200;

			}
			else if((DynamicOptimizeSystem[23]==1)||(DynamicOptimizeSystem[24]==1))
			{

				if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
				{
					di_double_buff_apply =1;
				}
				init_di_smd_additionalcoeff1_reg.smd_result_en=0;

			}
			else if(DynamicOptimizeSystem[28] ==1)
			{
				di_im_di_nintra_lowpass_source.nintra_lp_src_yclamp_th = 255;
				di_im_di_nintra_search_dir_limit.nintra_hdiff_th1=40;
				di_im_di_nintra_search_dir_limit.nintra_hdiff_th2=40;
			}
			else if(DynamicOptimizeSystem[63] ==1)
			{
				di_di_smd_slow_motion_handle1.smd_acc_smd_en =1;
				di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=1;
        			di_di_smd_slow_motion_handle1.smd_weave_dynamic = 3;

			}
			else if(DynamicOptimizeSystem[75] ==1)
			{
#if 0
				di_pan_dummy2_reg.panConti_str = 12;
				di_pan_dummy3_reg.panDeb_str = 80;
#else // henry Merlin3 temp
				di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | 12;
				di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | 80;
#endif

			}
			else if(DynamicOptimizeSystem[107]==1)
			{
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=225;
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				smd_refinemv3_reg.smd_sfreq_diff_gain =0;
				smd_refinemv3_reg.smd_sfreq_diff_th =0;
				smd_refinemv3_reg.smd_sfreq_drop_th =0;
				smd_compensate_reg.smd_compensate_teeth_th =5;
				smd_compensate_reg.smd_compensate_avg = 152;

				init_di_smd_candidatelimitsad_reg.smd_gmv_sad_max=0xff;
				init_di_smd_candidatelimitsad_reg.smd_gmv_sad_th=0x7f;

				init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain=6;
				init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset=0x1f;
				di_smd_dummy_reg.smd_partialweave =3;

				di_hmc_adjustable_reg.hmc_mv_refine_mor_en = 1;
				di_hmc_adjustable_reg.hmc_mv_refine_y_th = 120;
				di_hmc_pan_control_par_2_reg.hmc_sub_pixel_diff_thd = 180;


				di_di_smd_slow_motion_handle1.gst_min_sad_nolimit_en =1;
				di_di_smd_slow_motion_handle1.gst_gst_static_disable =1;
				di_di_smd_slow_motion_handle1.smd_weave_dynamic = 3;
				di_di_smd_slow_motion_handle1.smd_outofrange_content_new = 0;
				di_di_smd_slow_motion_handle2.gst_mv_limit = 1;
				//di_di_smd_slow_motion_handle1.smd_acc_smd_en =1;
				//di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=1;

				di_hmc_statistic1_reg.ma_hmc_lineratio = 24;
				rtd_outl(DI_IM_DI_HMC_STATISTIC1_reg, di_hmc_statistic1_reg.regValue);
				di_hmc_pan_control_par_1_reg.hme_sum_max_ratio = 18;

				FrameSobel_statistic_Reg.regValue = 0xc51f4200;
				di_hmc_adjustable_reg.regValue= 0x8FFD0004;
				di_hmc_adjustable2_reg.regValue= 0x0732c014; // special HMC setting for LG#75 and LG#107, jimmy 20151203
				di_hmc_adjustable3_reg.regValue= 0x6819141e;
				di_hmc_adjustable4_reg.regValue= 0x850a0c0c;

				 //weave window --> fix intra on LG logo!
				if(scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION)<=4)
				{
					im_di_weave_window_control_reg.fixedwindow1 = 0;
					im_di_weave_window_control_reg.fixedwindowmode = 0;
				}
				else
				{
					im_di_weave_window_control_reg.fixedwindow1 = 1;
					im_di_weave_window_control_reg.fixedwindowmode = 3;
				}
				im_di_weave_window_1_start_reg.xstart_25_13 =74 ;
				im_di_weave_window_1_start_reg.ystart_25_13=194 ;
				im_di_weave_window_1_end_reg.xend_25_13 = 81;
				im_di_weave_window_1_end_reg.yend_25_13 =201 ;


			}
			else if(DynamicOptimizeSystem[139] ==1)
			{
				di_ma_frame_motion_th_a_reg.ma_framestha = 20;
				di_ma_frame_motion_th_a_reg.ma_framehtha = 20;
				di_ma_frame_motion_th_a_reg.ma_framerlvtha = 7;
				di_ma_frame_motion_th_a_reg.ma_framemtha = 50;
				di_ma_frame_motion_th_b_reg.ma_framesthb = 20;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 20;
				di_ma_frame_motion_th_c_reg.ma_framesthc = 20;
				di_ma_frame_motion_th_c_reg.ma_framehthc = 20;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 0;

				dch1_cp_ctrl_reg.sresultweight_adjust_shift = 0;
				dch1_cp_ctrl_reg.vc_snr_lpf_sel = 0;
				curve_mapping_w1_2_reg.curvemapping_w1_6 = 3;
				curve_mapping_w1_2_reg.curvemapping_w1_7 = 3;
				//curve_mapping_w1_3_reg.curvemapping_w1_8 = 12;
				curve_mapping_step3_reg.curvemapping_step8=42;

			}
			else if(DynamicOptimizeSystem[147])
			{
				dch1_Spatial_Weight_Reg.cp_fixedweight1y = 1;
				dch1_Spatial_Weight_Reg.cp_fixedweight2y = 1;
			}
			else if(DynamicOptimizeSystem[151])
			{

				di_ma_frame_motion_th_a_reg.ma_framestha = 50;
				di_ma_frame_motion_th_a_reg.ma_framehtha = 70;
				di_ma_frame_motion_th_a_reg.ma_framerlvtha = 50;
				di_ma_frame_motion_th_a_reg.ma_framemtha = 70;
				di_ma_frame_motion_th_b_reg.ma_framesthb = 50;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 70;
				di_ma_frame_motion_th_b_reg.ma_framerlvthb = 50;
				di_ma_frame_motion_th_b_reg.ma_framemthb = 70;

				dm_dir_uzu_conf_Reg.conf_lowbnd = 63;
				di_im_di_nintra_control.nintra_intersection_check_en =0;
				di_im_di_nintra_control.nintra_preline_dir_check2_en = 0;
			}
			else if(DynamicOptimizeSystem[152]) //don't need reset value
			{
				di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
				di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector_det = 0;
				di_hmc_statistic1_reg.ma_hmc_lineratio =128;

				di_hmc_pan_control_par_1_reg.hme_sum_max_ratio =12;

				di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass =0;

				di_hmc_adjustable_reg.hmc_mv_refine_en =0;
				di_hmc_adjustable_reg.mc_dynamic_threshold_clip_en = 0;

				// protect di 3Abit,reduce r/w time .
				di_control_reg_Prtoect_3AEnable.regValue= rtd_inl(DI_IM_DI_CONTROL_reg);
				di_control_reg.ma_3aenable = di_control_reg_Prtoect_3AEnable.ma_3aenable;
				di_control_reg.write_enable_1=1;
				//rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);//rord.tsao mark don't change di seeting avoid 3a/5a setting error 2016/02/19
				rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_reg, di_im_di_hmc_pan_control_reg.regValue);
				rtd_outl(DI_IM_DI_HMC_STATISTIC1_reg, di_hmc_statistic1_reg.regValue);

				rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_1_reg,di_hmc_pan_control_par_1_reg.regValue);
				rtd_outl(DI_IM_DI_HMC_ME_REFINE_CTRL_2_reg,di_hmc_me_refine_ctrl2_reg.regValue);
				rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg,di_hmc_adjustable_reg.regValue);

			}
			else if(DynamicOptimizeSystem[192])
			{
				dch1_cp_ctrl_reg.cp_sresultweight =15;
				curve_mapping_w1_1_reg.curvemapping_w1_2=8;
				curve_mapping_w1_1_reg.curvemapping_w1_3=0;
				curve_mapping_w1_2_reg.curvemapping_w1_4=0;
				curve_mapping_w1_2_reg.curvemapping_w1_5=0;
				curve_mapping_step2_reg.curvemapping_step6=14;
				curve_mapping_step3_reg.curvemapping_step7=30;
				curve_mapping_step3_reg.curvemapping_step8=62;
			}


			//#1

			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
			rtd_outl(DI_DI_SMD_Compensate_reg, smd_compensate_reg.regValue);

			//#2
			rtd_outl(DI_DI_SMD_Slow_Motion_handle1_reg,di_di_smd_slow_motion_handle1.regValue);

			//#7
#if 0
			rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_2_reg, di_pan_dummy2_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_3_reg, di_pan_dummy3_reg.regValue);
#else // henry Merlin3 temp
			rtd_outl(DI_IM_DI_RTNR_REFINED_CTRL2_reg, di_im_di_rtnr_refined_ctrl2_reg.regValue);
			rtd_outl(DI_DI_SMD_dummy_reg, di_di_smd_dummy_reg.regValue);
#endif
			//#23 #24

			rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg, init_di_smd_additionalcoeff1_reg.regValue);


			//#28
			rtd_outl(DI_IM_DI_NINTRA_LOWPASS_SOURCE_reg,di_im_di_nintra_lowpass_source.regValue);
			rtd_outl(DI_IM_DI_NINTRA_SEARCH_DIR_LIMIT_reg,di_im_di_nintra_search_dir_limit.regValue);

			//#63
			// protect di 3Abit,reduce r/w time .
			di_control_reg_Prtoect_3AEnable.regValue= rtd_inl(DI_IM_DI_CONTROL_reg);
			di_control_reg.ma_3aenable = di_control_reg_Prtoect_3AEnable.ma_3aenable;
			di_control_reg.write_enable_1=1;
			//rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);//rord.tsao mark don't change di seeting avoid 3a/5a setting error 2016/02/19
			rtd_outl(DI_IM_DI_INTRA_TH_reg, intra_th_Reg.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle1_reg, di_di_smd_slow_motion_handle1.regValue);

			//#75
#if 0
			rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_2_reg, di_pan_dummy2_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_3_reg, di_pan_dummy3_reg.regValue);
#else // henry Merlin3 temp
			rtd_outl(DI_IM_DI_RTNR_REFINED_CTRL2_reg, di_im_di_rtnr_refined_ctrl2_reg.regValue);
			rtd_outl(DI_DI_SMD_dummy_reg, di_di_smd_dummy_reg.regValue);
#endif
			//#107

			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
			rtd_outl(DI_DI_SMD_RefineMV3_reg, smd_refinemv3_reg.regValue);
			rtd_outl(DI_DI_SMD_Compensate_reg, smd_compensate_reg.regValue);

			rtd_outl(DI_DI_SMD_CandidateLimitSAD_reg, init_di_smd_candidatelimitsad_reg.regValue);
			rtd_outl(DI_DI_SMD_AdditionalCoeff2_reg, init_di_smd_additionalcoeff2_reg.regValue);
			rtd_outl(DI_DI_SMD_dummy_reg, di_smd_dummy_reg.regValue);

			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg, di_hmc_adjustable_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_2_reg, di_hmc_pan_control_par_2_reg.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle1_reg, di_di_smd_slow_motion_handle1.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle2_reg, di_di_smd_slow_motion_handle2.regValue);

			rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_1_reg,di_hmc_pan_control_par_1_reg.regValue);
			rtd_outl(DI_IM_DI_FRAMESOBEL_STATISTIC_reg,FrameSobel_statistic_Reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg, di_hmc_adjustable_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE2_reg, di_hmc_adjustable2_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE3_reg, di_hmc_adjustable3_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE4_reg, di_hmc_adjustable4_reg.regValue);

			rtd_outl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, im_di_weave_window_control_reg.regValue);
			rtd_outl(DI_IM_DI_WEAVE_WINDOW_1_START_reg, im_di_weave_window_1_start_reg.regValue);
			rtd_outl(DI_IM_DI_WEAVE_WINDOW_1_END_reg, im_di_weave_window_1_end_reg.regValue);

			//#139
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, di_ma_frame_motion_th_c_reg.regValue);
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
			//rtd_outl(NR_CURVE_MAPPING_W1_2_reg, curve_mapping_w1_2_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_W1_3_reg, curve_mapping_w1_3_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_STEP_3_reg, curve_mapping_step3_reg.regValue);	//LGE control


			//#147,148,149,150
			rtd_outl(NR_DCH1_Spatial_Weight_reg, dch1_Spatial_Weight_Reg.regValue);

			//#151
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);

			rtd_outl(SCALEUP_DM_DIR_UZU_Conf_reg, dm_dir_uzu_conf_Reg.regValue);
			rtd_outl(DI_IM_DI_NINTRA_CONTROL_reg,di_im_di_nintra_control.regValue);

			//#152
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg, di_hmc_adjustable_reg.regValue);

			//#192
			//rtd_outl(NR_CURVE_MAPPING_W1_1_reg, curve_mapping_w1_1_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_W1_2_reg, curve_mapping_w1_2_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_STEP_2_reg, curve_mapping_step2_reg.regValue);	//LGE control
			//rtd_outl(NR_CURVE_MAPPING_STEP_3_reg, curve_mapping_step3_reg.regValue);	//LGE control
			rtd_outl(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);

		}
		break;
		case VIP_QUALITY_HDMI_576I:
		case VIP_QUALITY_YPbPr_576I:
		{
			//************** INI VALE START    ***************************************************

			//#1
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			di_smd_control_candiate_reg.smd_en=ori_smd_en;
			smd_compensate_reg.smd_compensate_teeth_th =ori_smd_compensate_reg.smd_compensate_teeth_th;

			//#2
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			di_di_smd_slow_motion_handle1.gst_gst_static_disable = ori_gst_gst_static_disable;
			di_di_smd_slow_motion_handle1.gst_min_sad_compare_diable =ori_gst_min_sad_compare_diable;

			//#7
#if 0
			di_pan_dummy2_reg.panConti_str = ORI_di_pan_dummy2_reg.panConti_str;
			di_pan_dummy3_reg.panDeb_str = ORI_di_pan_dummy3_reg.panDeb_str;
#else // henry Merlin3 temp
			di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | panConti_str;
			di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | panDeb_str;
#endif

			//#12
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=ori_smd_upperbound_sad_forjudgemv;
			smd_compensate_reg.smd_compensate_teeth_th =ori_smd_compensate_reg.smd_compensate_teeth_th;

			//#23 #24

			if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
			{
				di_double_buff_apply =1;
			}
			init_di_smd_additionalcoeff1_reg.smd_result_en=ori_smd_result_en;

			//#28
			di_im_di_nintra_lowpass_source.nintra_lp_src_yclamp_th = ori_nintra_lp_src_yclamp_th;
			di_im_di_nintra_search_dir_limit.regValue=ori_di_im_di_nintra_search_dir_limit.regValue;

			//#63
			di_di_smd_slow_motion_handle1.smd_acc_smd_en =ori_smd_acc_smd_en;
			di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=ori_smd_spatialfreqnomatch_off;
			di_di_smd_slow_motion_handle1.smd_weave_dynamic = ori_smd_weave_dynamic;

			//#75
#if 0
			di_pan_dummy2_reg.panConti_str = ORI_di_pan_dummy2_reg.panConti_str;
			di_pan_dummy3_reg.panDeb_str = ORI_di_pan_dummy3_reg.panDeb_str;
#else // henry Merlin3 temp
			di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | panConti_str;
			di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | panDeb_str;
#endif

			//#107
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=ori_smd_upperbound_sad_forjudgemv;
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;
			smd_refinemv3_reg.smd_sfreq_diff_gain =4;
			smd_refinemv3_reg.smd_sfreq_diff_th =25;
			smd_refinemv3_reg.smd_sfreq_drop_th =80;
			smd_compensate_reg.smd_compensate_teeth_th =ori_smd_compensate_reg.smd_compensate_teeth_th;
			smd_compensate_reg.smd_compensate_avg = ori_smd_compensate_reg.smd_compensate_avg;

			init_di_smd_candidatelimitsad_reg.smd_gmv_sad_max=0xd2;
			init_di_smd_candidatelimitsad_reg.smd_gmv_sad_th=0x40;

			init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain=gSmd_gain;
			init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset=gSmd_offset;
			di_smd_dummy_reg.smd_partialweave =0;

			di_hmc_pan_control_par_2_reg.hmc_sub_pixel_diff_thd = ori_hmc_sub_pixel_diff_thd;

			di_di_smd_slow_motion_handle1.gst_min_sad_nolimit_en =ori_gst_min_sad_nolimit_en;
			di_di_smd_slow_motion_handle1.gst_gst_static_disable =ori_gst_gst_static_disable;
			di_di_smd_slow_motion_handle1.smd_weave_dynamic = ori_smd_weave_dynamic;
			di_di_smd_slow_motion_handle1.smd_outofrange_content_new = ori_smd_outofrange_content_new;
			di_di_smd_slow_motion_handle2.gst_mv_limit = ori_gst_mv_limit;
			//di_di_smd_slow_motion_handle1.smd_acc_smd_en =ori_smd_acc_smd_en;
			//di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=ori_smd_spatialfreqnomatch_off;

			di_hmc_statistic1_reg.regValue = ORI_di_hmc_statistic1_reg.regValue;
			di_hmc_pan_control_par_1_reg.hme_sum_max_ratio = ORI_di_hmc_pan_control_par_1_reg.hme_sum_max_ratio;
			FrameSobel_statistic_Reg.regValue = ORI_FrameSobel_statistic_Reg.regValue;
			di_hmc_adjustable_reg.regValue = ORI_di_hmc_adjustable_reg.regValue;
			di_hmc_adjustable2_reg.regValue= ORI_di_hmc_adjustable2_reg.regValue;
			di_hmc_adjustable3_reg.regValue= ORI_di_hmc_adjustable3_reg.regValue;
			di_hmc_adjustable4_reg.regValue= ORI_di_hmc_adjustable4_reg.regValue;



			//#139
			di_ma_frame_motion_th_a_reg.ma_framestha = ORI_di_ma_frame_motion_th_a_reg.ma_framestha;
			di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;
			di_ma_frame_motion_th_a_reg.ma_framerlvtha = ORI_di_ma_frame_motion_th_a_reg.ma_framerlvtha;
			di_ma_frame_motion_th_a_reg.ma_framemtha = ORI_di_ma_frame_motion_th_a_reg.ma_framemtha;
			di_ma_frame_motion_th_b_reg.ma_framesthb = ORI_di_ma_frame_motion_th_b_reg.ma_framesthb;
			di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;
			di_ma_frame_motion_th_c_reg.ma_framesthc = ORI_di_ma_frame_motion_th_c_reg.ma_framesthc;
			di_ma_frame_motion_th_c_reg.ma_framehthc = ORI_di_ma_frame_motion_th_c_reg.ma_framehthc;
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = ORI_di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv;

			dch1_cp_ctrl_reg.sresultweight_adjust_shift = ori_sresultweight_adjust_shift;
			dch1_cp_ctrl_reg.vc_snr_lpf_sel = ori_vc_snr_lpf_sel;
			curve_mapping_w1_2_reg.curvemapping_w1_6 = ori_curve_mapping_w1_6;
			curve_mapping_w1_2_reg.curvemapping_w1_7 = ori_curve_mapping_w1_7;
			curve_mapping_w1_3_reg.curvemapping_w1_8 = ori_curve_mapping_w1_8;
			curve_mapping_step3_reg.curvemapping_step8=ori_curve_mapping_step8;


			//#147,148,149,150
			dch1_Spatial_Weight_Reg.cp_fixedweight1y = ori_cp_fixedweight1y;
			dch1_Spatial_Weight_Reg.cp_fixedweight2y = ori_cp_fixedweight2y;

			//#151
			di_ma_frame_motion_th_a_reg.ma_framestha = ORI_di_ma_frame_motion_th_a_reg.ma_framestha;
			di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;
			di_ma_frame_motion_th_a_reg.ma_framerlvtha = ORI_di_ma_frame_motion_th_a_reg.ma_framerlvtha;
			di_ma_frame_motion_th_a_reg.ma_framemtha = ORI_di_ma_frame_motion_th_a_reg.ma_framemtha;
			di_ma_frame_motion_th_b_reg.ma_framesthb = ORI_di_ma_frame_motion_th_b_reg.ma_framesthb;
			di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;
			di_ma_frame_motion_th_b_reg.ma_framerlvthb = ORI_di_ma_frame_motion_th_b_reg.ma_framerlvthb;
			di_ma_frame_motion_th_b_reg.ma_framemthb = ORI_di_ma_frame_motion_th_b_reg.ma_framemthb;

			dm_dir_uzu_conf_Reg.conf_lowbnd = ori_conf_lowbnd;
			di_im_di_nintra_control.nintra_intersection_check_en =ori_di_im_di_nintra_control.nintra_intersection_check_en;
			di_im_di_nintra_control.nintra_preline_dir_check2_en = ori_di_im_di_nintra_control.nintra_preline_dir_check2_en;

			//#152
			di_hmc_adjustable_reg.hmc_mv_refine_en =ORI_di_hmc_adjustable_reg.hmc_mv_refine_en;
			di_hmc_adjustable_reg.mc_dynamic_threshold_clip_en = ORI_di_hmc_adjustable_reg.mc_dynamic_threshold_clip_en;

			//#192
			dch1_cp_ctrl_reg.cp_sresultweight = ori_cp_sresultweight;
			curve_mapping_w1_1_reg.curvemapping_w1_2=ori_curve_mapping_w1_2;
			curve_mapping_w1_1_reg.curvemapping_w1_3=ori_curve_mapping_w1_3;
			curve_mapping_w1_2_reg.curvemapping_w1_4=ori_curve_mapping_w1_4;
			curve_mapping_w1_2_reg.curvemapping_w1_5=ori_curve_mapping_w1_5;
			curve_mapping_step2_reg.curvemapping_step6=ori_curve_mapping_step6;
			curve_mapping_step3_reg.curvemapping_step7=ori_curve_mapping_step7;
			curve_mapping_step3_reg.curvemapping_step8=ori_curve_mapping_step8;

			//************** INI VALE END    ***************************************************

			if(DynamicOptimizeSystem[1] ==1)
			{
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 200;
				smd_compensate_reg.smd_compensate_teeth_th =200;
			}
			else if(DynamicOptimizeSystem[2] ==1)
			{
				di_di_smd_slow_motion_handle1.gst_gst_static_disable = 1;
				di_di_smd_slow_motion_handle1.gst_min_sad_compare_diable =1;
				di_im_di_nintra_lowpass_source.nintra_lp_src_yclamp_th = 30;
				di_im_di_nintra_search_dir_limit.nintra_hdiff_th1=16;
				di_im_di_nintra_search_dir_limit.nintra_hdiff_th2=16;
			}
			else if(DynamicOptimizeSystem[7] ==1)
			{
#if 0
				di_pan_dummy2_reg.panConti_str = 12;
				di_pan_dummy3_reg.panDeb_str = 80;
#else // henry Merlin3 temp
				di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | 12;
				di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | 80;
#endif
			}
			else if(DynamicOptimizeSystem[12] ==1)
			{
				//drvif_color_newrtnr_sad_th(1,&Motion_Th[0]);//for sad edge th setting
				//drvif_color_newrtnr_sad_th(0,&Motion_Th[0]);//for sad nonedge th setting
				drvif_color_rtnr_y_th_isr(_CHANNEL1,&Motion_Th[0]);

				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 200;
				smd_compensate_reg.smd_compensate_teeth_th =200;

			}
			else if((DynamicOptimizeSystem[23]==1)||(DynamicOptimizeSystem[24]==1))
			{

				if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
				{
					di_double_buff_apply =1;
				}
				init_di_smd_additionalcoeff1_reg.smd_result_en=0;

			}
			else if(DynamicOptimizeSystem[28] ==1)
			{
				di_im_di_nintra_lowpass_source.nintra_lp_src_yclamp_th = 30;
				di_im_di_nintra_search_dir_limit.nintra_hdiff_th1=16;
				di_im_di_nintra_search_dir_limit.nintra_hdiff_th2=16;
			}
			else if(DynamicOptimizeSystem[63] ==1)
			{
				di_di_smd_slow_motion_handle1.smd_acc_smd_en =1;
				di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=1;
        			di_di_smd_slow_motion_handle1.smd_weave_dynamic = 3;

			}
			else if(DynamicOptimizeSystem[75] ==1)
			{
#if 0
				di_pan_dummy2_reg.panConti_str = 12;
				di_pan_dummy3_reg.panDeb_str = 80;
#else // henry Merlin3 temp
				di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 | 12;
				di_di_smd_dummy_reg.dummy18024540_31_2 = di_di_smd_dummy_reg.dummy18024540_31_2 | 80;
#endif
			}
			else if(DynamicOptimizeSystem[107]==1)
			{
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=125;
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 0;
				smd_refinemv3_reg.smd_sfreq_diff_gain =0;
				smd_refinemv3_reg.smd_sfreq_diff_th =0;
				smd_refinemv3_reg.smd_sfreq_drop_th =0;
				smd_compensate_reg.smd_compensate_teeth_th =5;
				smd_compensate_reg.smd_compensate_avg = 152;

				init_di_smd_candidatelimitsad_reg.smd_gmv_sad_max=0xff;
				init_di_smd_candidatelimitsad_reg.smd_gmv_sad_th=0x7f;

				init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain=2;
				init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset=0x1f;
				di_smd_dummy_reg.smd_partialweave =3;

				di_hmc_adjustable_reg.hmc_mv_refine_mor_en = 1;
				di_hmc_adjustable_reg.hmc_mv_refine_y_th = 120;
				di_hmc_pan_control_par_2_reg.hmc_sub_pixel_diff_thd = 180;


				di_di_smd_slow_motion_handle1.gst_min_sad_nolimit_en =1;
				di_di_smd_slow_motion_handle1.gst_gst_static_disable =1;
				di_di_smd_slow_motion_handle1.smd_weave_dynamic = 0;
				di_di_smd_slow_motion_handle1.smd_outofrange_content_new=0;
				di_di_smd_slow_motion_handle2.gst_mv_limit = 1;
				//di_di_smd_slow_motion_handle1.smd_acc_smd_en =1;
				//di_di_smd_slow_motion_handle1.smd_spatialfreqnomatch_off=1;

				di_hmc_statistic1_reg.ma_hmc_lineratio = 24;
				rtd_outl(DI_IM_DI_HMC_STATISTIC1_reg, di_hmc_statistic1_reg.regValue);
				di_hmc_pan_control_par_1_reg.hme_sum_max_ratio = 18;

				FrameSobel_statistic_Reg.regValue = 0xc51f4200;
				di_hmc_adjustable_reg.regValue= 0x8FFD0004;
				di_hmc_adjustable2_reg.regValue= 0x0732c014; // special HMC setting for LG#75 and LG#107, jimmy 20151203
				di_hmc_adjustable3_reg.regValue= 0x6819141e;
				di_hmc_adjustable4_reg.regValue= 0x850a0c0c;

			}
			else if(DynamicOptimizeSystem[139] ==1)
			{
				di_ma_frame_motion_th_a_reg.ma_framestha = 20;
				di_ma_frame_motion_th_a_reg.ma_framehtha = 20;
				di_ma_frame_motion_th_a_reg.ma_framerlvtha = 20;
				di_ma_frame_motion_th_a_reg.ma_framemtha = 50;
				di_ma_frame_motion_th_b_reg.ma_framesthb = 20;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 20;
				di_ma_frame_motion_th_c_reg.ma_framesthc = 20;
				di_ma_frame_motion_th_c_reg.ma_framehthc = 20;
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv = 0;

				dch1_cp_ctrl_reg.sresultweight_adjust_shift = 0;
				dch1_cp_ctrl_reg.vc_snr_lpf_sel = 0;
				curve_mapping_w1_2_reg.curvemapping_w1_6 = 0;
				curve_mapping_w1_2_reg.curvemapping_w1_7 = 0;
				curve_mapping_w1_3_reg.curvemapping_w1_8 = 7;
				curve_mapping_step3_reg.curvemapping_step8=42;

			}
			else if(DynamicOptimizeSystem[147])
			{
				dch1_Spatial_Weight_Reg.cp_fixedweight1y = 1;
				dch1_Spatial_Weight_Reg.cp_fixedweight2y = 1;
			}
			else if(DynamicOptimizeSystem[151])
			{

				di_ma_frame_motion_th_a_reg.ma_framestha = 50;
				di_ma_frame_motion_th_a_reg.ma_framehtha = 50;
				di_ma_frame_motion_th_a_reg.ma_framerlvtha = 50;
				di_ma_frame_motion_th_a_reg.ma_framemtha = 50;
				di_ma_frame_motion_th_b_reg.ma_framesthb = 50;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 50;
				di_ma_frame_motion_th_b_reg.ma_framerlvthb = 50;
				di_ma_frame_motion_th_b_reg.ma_framemthb = 50;

				dm_dir_uzu_conf_Reg.conf_lowbnd = 63;
				di_im_di_nintra_control.nintra_intersection_check_en =0;
				di_im_di_nintra_control.nintra_preline_dir_check2_en = 0;
			}
			else if(DynamicOptimizeSystem[152]) //don't need reset value
			{
				di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass =1;
				di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector_det = 0;
				di_hmc_statistic1_reg.ma_hmc_lineratio =128;

				di_hmc_pan_control_par_1_reg.hme_sum_max_ratio =12;

				di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass =0;

				di_hmc_adjustable_reg.hmc_mv_refine_en =0;
				di_hmc_adjustable_reg.mc_dynamic_threshold_clip_en = 0;


				// protect di 3Abit,reduce r/w time .
				di_control_reg_Prtoect_3AEnable.regValue= rtd_inl(DI_IM_DI_CONTROL_reg);
				di_control_reg.ma_3aenable = di_control_reg_Prtoect_3AEnable.ma_3aenable;
				di_control_reg.write_enable_1=1;
				//rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);//rord.tsao mark don't change di seeting avoid 3a/5a setting error 2016/02/19
				rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_reg, di_im_di_hmc_pan_control_reg.regValue);
				rtd_outl(DI_IM_DI_HMC_STATISTIC1_reg, di_hmc_statistic1_reg.regValue);

				rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_1_reg,di_hmc_pan_control_par_1_reg.regValue);
				rtd_outl(DI_IM_DI_HMC_ME_REFINE_CTRL_2_reg,di_hmc_me_refine_ctrl2_reg.regValue);
				rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg,di_hmc_adjustable_reg.regValue);

			}
			else if(DynamicOptimizeSystem[192])
			{
				dch1_cp_ctrl_reg.cp_sresultweight =15;
				curve_mapping_w1_1_reg.curvemapping_w1_2=8;
				curve_mapping_w1_1_reg.curvemapping_w1_3=0;
				curve_mapping_w1_2_reg.curvemapping_w1_4=0;
				curve_mapping_w1_2_reg.curvemapping_w1_5=0;
				curve_mapping_step2_reg.curvemapping_step6=14;
				curve_mapping_step3_reg.curvemapping_step7=30;
				curve_mapping_step3_reg.curvemapping_step8=62;
			}


			//#1

			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
			rtd_outl(DI_DI_SMD_Compensate_reg, smd_compensate_reg.regValue);

			//#2
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle1_reg,di_di_smd_slow_motion_handle1.regValue);

			//#7
#if 0
			rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_2_reg, di_pan_dummy2_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_3_reg, di_pan_dummy3_reg.regValue);
#else // henry Merlin3 temp
			rtd_outl(DI_IM_DI_RTNR_REFINED_CTRL2_reg, di_im_di_rtnr_refined_ctrl2_reg.regValue);
			rtd_outl(DI_DI_SMD_dummy_reg, di_di_smd_dummy_reg.regValue);
#endif
			//#12
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_Compensate_reg, smd_compensate_reg.regValue);


			//#23 #24

			rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg, init_di_smd_additionalcoeff1_reg.regValue);


			//#28
			rtd_outl(DI_IM_DI_NINTRA_LOWPASS_SOURCE_reg,di_im_di_nintra_lowpass_source.regValue);
			rtd_outl(DI_IM_DI_NINTRA_SEARCH_DIR_LIMIT_reg,di_im_di_nintra_search_dir_limit.regValue);

			//#63
			// protect di 3Abit,reduce r/w time .
			di_control_reg_Prtoect_3AEnable.regValue= rtd_inl(DI_IM_DI_CONTROL_reg);
			di_control_reg.ma_3aenable = di_control_reg_Prtoect_3AEnable.ma_3aenable;
			di_control_reg.write_enable_1=1;
			//rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);//rord.tsao mark don't change di seeting avoid 3a/5a setting error 2016/02/19
			rtd_outl(DI_IM_DI_INTRA_TH_reg, intra_th_Reg.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle1_reg, di_di_smd_slow_motion_handle1.regValue);

			//#75
#if 0
			rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_2_reg, di_pan_dummy2_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_3_reg, di_pan_dummy3_reg.regValue);
#else // henry Merlin3 temp
			rtd_outl(DI_IM_DI_RTNR_REFINED_CTRL2_reg, di_im_di_rtnr_refined_ctrl2_reg.regValue);
			rtd_outl(DI_DI_SMD_dummy_reg, di_di_smd_dummy_reg.regValue);
#endif
			//#107

			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
			rtd_outl(DI_DI_SMD_RefineMV3_reg, smd_refinemv3_reg.regValue);
			rtd_outl(DI_DI_SMD_Compensate_reg, smd_compensate_reg.regValue);

			rtd_outl(DI_DI_SMD_CandidateLimitSAD_reg, init_di_smd_candidatelimitsad_reg.regValue);
			rtd_outl(DI_DI_SMD_AdditionalCoeff2_reg, init_di_smd_additionalcoeff2_reg.regValue);
			rtd_outl(DI_DI_SMD_dummy_reg, di_smd_dummy_reg.regValue);

			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg, di_hmc_adjustable_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_2_reg, di_hmc_pan_control_par_2_reg.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle1_reg, di_di_smd_slow_motion_handle1.regValue);
			rtd_outl(DI_DI_SMD_Slow_Motion_handle2_reg, di_di_smd_slow_motion_handle2.regValue);

			rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_1_reg,di_hmc_pan_control_par_1_reg.regValue);
			rtd_outl(DI_IM_DI_FRAMESOBEL_STATISTIC_reg,FrameSobel_statistic_Reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg, di_hmc_adjustable_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE2_reg, di_hmc_adjustable2_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE3_reg, di_hmc_adjustable3_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE4_reg, di_hmc_adjustable4_reg.regValue);

			//#139
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, di_ma_frame_motion_th_c_reg.regValue);
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
			//rtd_outl(NR_CURVE_MAPPING_W1_2_reg, curve_mapping_w1_2_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_W1_3_reg, curve_mapping_w1_3_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_STEP_3_reg, curve_mapping_step3_reg.regValue);	//LGE control

			//#147,148,149,150
			rtd_outl(NR_DCH1_Spatial_Weight_reg, dch1_Spatial_Weight_Reg.regValue);

			//#151

			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);

			rtd_outl(SCALEUP_DM_DIR_UZU_Conf_reg, dm_dir_uzu_conf_Reg.regValue);
			rtd_outl(DI_IM_DI_NINTRA_CONTROL_reg,di_im_di_nintra_control.regValue);
			//#152
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg, di_hmc_adjustable_reg.regValue);

			//#192
			rtd_outl(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
			//rtd_outl(NR_CURVE_MAPPING_W1_1_reg, curve_mapping_w1_1_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_W1_2_reg, curve_mapping_w1_2_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_W1_3_reg, curve_mapping_w1_3_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_STEP_2_reg, curve_mapping_step2_reg.regValue);	//LGE control
			//rtd_outl(NR_CURVE_MAPPING_STEP_3_reg, curve_mapping_step3_reg.regValue);	//LGE control

		}
		break;
		case VIP_QUALITY_HDMI_1080I:
		case VIP_QUALITY_YPbPr_1080I:
		{
			//************** INI VALE START    ***************************************************

			//#14
			di_smd_control_candiate_reg.smd_en=ori_smd_en;
			smd_choosecandidate_reg.smd_reliablerange_sad_th = gSmd_weak_move;

			//#23,#24,#39
			di_ma_frame_motion_th_a_reg.ma_framestha = ORI_di_ma_frame_motion_th_a_reg.ma_framestha;
			di_smd_control_candiate_reg.smd_en=ori_smd_en;;

			//#107

			di_hmc_adjustable_reg.hmc_mv_refine_y_th = ORI_di_hmc_adjustable_reg.hmc_mv_refine_y_th;
			di_hmc_pan_control_par_2_reg.hmc_sub_pixel_diff_thd = ori_hmc_sub_pixel_diff_thd;

			di_im_di_btr_control_reg.btr_blendtemporalrecoveryen = ORI_di_im_di_btr_control_reg.btr_blendtemporalrecoveryen;

			init_di_gst_ctrl_reg.regValue = ORI_init_di_gst_ctrl_reg.regValue;
			init_di_smd_candidatelimitsad_reg.smd_gmv_sad_th=0x40;
			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=125;
			smd_choosecandidate_reg.smd_reliablerange_sad_th = 50;
			di_smd_refinemv2_reg.regValue = ori_di_smd_refinemv2_reg.regValue;

			init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain=2;
			init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset=0x1f;

			di_di_smd_slow_motion_handle1.gst_min_sad_nolimit_en =ori_gst_min_sad_nolimit_en;
			di_di_smd_slow_motion_handle1.gst_gst_static_disable =ori_gst_gst_static_disable;
			di_di_smd_slow_motion_handle1.smd_weave_dynamic = ori_smd_weave_dynamic;
			if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
			{
				di_double_buff_apply =1;
			}

			init_di_smd_additionalcoeff1_reg.smd_result_en = ori_smd_result_en;

			im_di_weave_window_control_reg.fixedwindow1 = ORI_im_di_weave_window_control_reg.fixedwindow1;
			im_di_weave_window_1_start_reg.xstart_25_13 =ORI_im_di_weave_window_1_start_reg.xstart_25_13;
			im_di_weave_window_1_start_reg.ystart_25_13=ORI_im_di_weave_window_1_start_reg.ystart_25_13;
			im_di_weave_window_1_end_reg.xend_25_13 = ORI_im_di_weave_window_1_end_reg.xend_25_13;
			im_di_weave_window_1_end_reg.yend_25_13 =ORI_im_di_weave_window_1_end_reg.yend_25_13;

			im_di_weave_window_control_reg.fixedwindow1 = 0;
			im_di_weave_window_1_start_reg.xstart_25_13 =0;
			im_di_weave_window_1_start_reg.ystart_25_13=0;
			im_di_weave_window_1_end_reg.xend_25_13 = 0;
			im_di_weave_window_1_end_reg.yend_25_13 =0;

			di_ma_frame_motion_th_a_reg.regValue = ORI_di_ma_frame_motion_th_a_reg.regValue;
			di_ma_frame_motion_th_b_reg.regValue = ORI_di_ma_frame_motion_th_b_reg.regValue;
			di_ma_frame_motion_th_c_reg.regValue = ORI_di_ma_frame_motion_th_c_reg.regValue;


			//#176

			im_di_film_sawtooth_filmframe_th_reg.regValue =ORI_im_di_film_sawtooth_filmframe_th_reg.regValue;
			film_static_sum_th_reg.regValue= ORI_film_static_sum_th_reg.regValue;
			film_motion_sum_th_reg.regValue= ORI_film_motion_sum_th_reg.regValue;



			//#178

			di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=ORI_di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv;
			smd_choosecandidate_reg.smd_reliablerange_sad_th=gSmd_weak_move;
			//di_di_smd_resurrect_ctrl.smd_mv_not_gmv_resurrect_en=ori_di_di_smd_resurrect_ctrl.smd_mv_not_gmv_resurrect_en; //mac5p removed
			//di_di_smd_resurrect_ctrl.smd_resurrect_noacc_en=ori_di_di_smd_resurrect_ctrl.smd_resurrect_noacc_en; //mac5p removed
			//di_di_smd_sampling_gmvc.smd_close2gmv_fw_en=ori_smd_close2gmv_fw_en; //mac5p removed
			if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
			{
				di_double_buff_apply =1;
			}
			init_di_smd_additionalcoeff1_reg.smd_result_en = ori_smd_result_en;


			//#182

			//#187
			di_ma_frame_motion_th_a_reg.ma_framestha = ORI_di_ma_frame_motion_th_a_reg.ma_framestha;
			di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;
			di_ma_frame_motion_th_a_reg.ma_framerlvtha =ORI_di_ma_frame_motion_th_a_reg.ma_framerlvtha;
			di_ma_frame_motion_th_a_reg.ma_framemtha = ORI_di_ma_frame_motion_th_a_reg.ma_framemtha;

			di_ma_frame_motion_th_b_reg.ma_framesthb = ORI_di_ma_frame_motion_th_b_reg.ma_framesthb;
			di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;
			di_ma_frame_motion_th_b_reg.ma_framerlvthb = ORI_di_ma_frame_motion_th_b_reg.ma_framerlvthb;
			di_ma_frame_motion_th_b_reg.ma_framemthb= ORI_di_ma_frame_motion_th_b_reg.ma_framemthb;

			//#197
			di_ma_frame_motion_th_a_reg.ma_framestha = ORI_di_ma_frame_motion_th_a_reg.ma_framestha;
			di_ma_frame_motion_th_b_reg.ma_framesthb = ORI_di_ma_frame_motion_th_b_reg.ma_framesthb;

			di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;
			di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;

			//#198

			di_ma_frame_motion_th_a_reg.ma_framestha = ORI_di_ma_frame_motion_th_a_reg.ma_framestha;
			di_ma_frame_motion_th_a_reg.ma_framerlvtha = ORI_di_ma_frame_motion_th_a_reg.ma_framerlvtha;
			di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;

			di_ma_frame_motion_th_b_reg.ma_framesthb = ORI_di_ma_frame_motion_th_b_reg.ma_framesthb;
			di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;
			di_ma_frame_motion_th_b_reg.ma_framerlvthb = ORI_di_ma_frame_motion_th_b_reg.ma_framerlvthb;

			di_im_di_nintra_lowpass_source.nintra_lp_src_yclamp_th = ori_nintra_lp_src_yclamp_th;
			di_im_di_nintra_search_dir_limit.nintra_hdiff_th1=ori_di_im_di_nintra_search_dir_limit.nintra_hdiff_th1;
			di_im_di_nintra_control.regValue=ori_di_im_di_nintra_control.regValue;

#ifdef CONFIG_HW_SUPPORT_MEMC
			//#Spears & Munsil ADVANCED VUDEO - EVALUATION - Chroma Zone Plate
			color_temp_ctrl_reg.regValue = ORI_color_temp_ctrl_reg.regValue;
#endif //CONFIG_HW_SUPPORT_MEMC

			//************** INI VALE END    ***************************************************

			if(DynamicOptimizeSystem[14]==1)
			{
				di_smd_control_candiate_reg.smd_en=1;
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 200;
			}
			else if(DynamicOptimizeSystem[23]==1)
			{
				di_ma_frame_motion_th_a_reg.ma_framestha =0x02;
				di_smd_control_candiate_reg.smd_en=0;

			}
			else if(DynamicOptimizeSystem[24]==1)
			{
				di_ma_frame_motion_th_a_reg.ma_framestha =0x02;
				di_smd_control_candiate_reg.smd_en=0;


			}
			else if(DynamicOptimizeSystem[39]==1)
			{
				di_ma_frame_motion_th_a_reg.ma_framestha =0x02;
				di_smd_control_candiate_reg.smd_en=0;


			}
			else if(DynamicOptimizeSystem[107]==1)
			{
				init_di_gst_ctrl_reg.gst_fmv2weave_p =15;
				init_di_gst_ctrl_reg.gst_fmv2weave_n =15;
				init_di_smd_candidatelimitsad_reg.smd_gmv_sad_th=0x7f;

				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=40;
				smd_choosecandidate_reg.smd_reliablerange_sad_th = 120;

				init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain=7;
				init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset=0x1f;

				di_smd_refinemv2_reg.smd_mvs_hchanged_en =1;
				di_smd_refinemv2_reg.smd_mvs_vchanged_en =1;

				di_di_smd_slow_motion_handle1.gst_min_sad_nolimit_en =1;
				di_di_smd_slow_motion_handle1.gst_gst_static_disable =1;
				di_di_smd_slow_motion_handle1.smd_weave_dynamic = 3;
				if(init_di_smd_additionalcoeff1_reg.smd_result_en!=1)
				{
					di_double_buff_apply =1;
				}

				init_di_smd_additionalcoeff1_reg.smd_result_en =1;

				di_hmc_adjustable_reg.hmc_mv_refine_y_th = 150;
				di_hmc_pan_control_par_2_reg.hmc_sub_pixel_diff_thd = 150;
				di_im_di_btr_control_reg.btr_blendtemporalrecoveryen = 0;
				di_hmc_statistic1_reg.ma_hmc_lineratio =di_hmc_statistic1_reg.ma_hmc_lineratio+12;
				if(di_hmc_statistic1_reg.ma_hmc_lineratio>12)
					di_hmc_statistic1_reg.ma_hmc_lineratio=12;
				rtd_outl(DI_IM_DI_HMC_STATISTIC1_reg, di_hmc_statistic1_reg.regValue);

			 //weave window --> fix intra on LG logo!
				if(scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION)<=3)
				{
					im_di_weave_window_control_reg.fixedwindow1 = 0;
					im_di_weave_window_control_reg.fixedwindowmode = 0;
				}
				else
				{
				im_di_weave_window_control_reg.fixedwindow1 = 1;
					im_di_weave_window_control_reg.fixedwindowmode = 3;
				}
				im_di_weave_window_1_start_reg.xstart_25_13 =140 ;
				im_di_weave_window_1_start_reg.ystart_25_13=431 ;
				im_di_weave_window_1_end_reg.xend_25_13 = 250;
				im_di_weave_window_1_end_reg.yend_25_13 =493 ;

			}
			else if(DynamicOptimizeSystem[176] ==1) // hqv flower 22 film
			{
				im_di_film_sawtooth_filmframe_th_reg.film22_sawtooththl=10;
				im_di_film_sawtooth_filmframe_th_reg.film_sawtooththl =10;
				im_di_film_sawtooth_filmframe_th_reg.film_frmotionthl =10;
				film_static_sum_th_reg.film22_fistaticsthl =10;
				film_motion_sum_th_reg.film22_fieldmotionsumthl =10;
				film_motion_sum_th_reg.film22_debounce_fieldnum =1;

			}
			else if( (DynamicOptimizeSystem[182]==1) || (DynamicOptimizeSystem[183]==1) || (DynamicOptimizeSystem[184]==1) || (DynamicOptimizeSystem[185]==1) || (DynamicOptimizeSystem[186]==1) )
			{
	 			ich1_MPEGNR2_reg.regValue = rtd_inl(MPEGNR_ICH1_MPEGNR2_reg);
				ich1_MPEGNR2_reg.cp_mpegenable_x=0;
				rtd_setbits(MPEGNR_ICH1_MPEGNR_DB_CTRL_reg, MPEGNR_ICH1_MPEGNR_DB_CTRL_mpegnr_db_en_mask);
				scaler_rtd_outl(MPEGNR_ICH1_MPEGNR2_reg, ich1_MPEGNR2_reg.regValue);
				rtd_setbits(MPEGNR_ICH1_MPEGNR_DB_CTRL_reg, MPEGNR_ICH1_MPEGNR_DB_CTRL_mpegnr_db_apply_mask);
 			}
			else if(DynamicOptimizeSystem[187] ==1) // qtec Makilo file #59
			{
				di_ma_frame_motion_th_a_reg.ma_framestha = 30;
				di_ma_frame_motion_th_a_reg.ma_framehtha = 30;
				di_ma_frame_motion_th_a_reg.ma_framerlvtha =30;
				di_ma_frame_motion_th_a_reg.ma_framemtha=30;

				di_ma_frame_motion_th_b_reg.ma_framesthb = 30;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 30;
				di_ma_frame_motion_th_b_reg.ma_framerlvthb =30;
				di_ma_frame_motion_th_b_reg.ma_framemthb=30;

 			}
#if 0		// Qtec build vertical line dot noise issue. (pattern in/out dot noise)
			else if(DynamicOptimizeSystem[197] ==1) //qtec build (streets)
			{
				di_ma_frame_motion_th_a_reg.ma_framestha = 20;
				di_ma_frame_motion_th_b_reg.ma_framesthb = 20;

				di_ma_frame_motion_th_a_reg.ma_framehtha = 30;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 30;
 			}
#endif
			else if(DynamicOptimizeSystem[198] ==1) //qtec building
			{
				di_ma_frame_motion_th_a_reg.ma_framestha = 1;
				di_ma_frame_motion_th_a_reg.ma_framerlvtha =1;
				di_ma_frame_motion_th_a_reg.ma_framehtha = 1;

				di_ma_frame_motion_th_b_reg.ma_framesthb = 1;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 1;
				di_ma_frame_motion_th_b_reg.ma_framerlvthb =1;

				di_im_di_nintra_lowpass_source.nintra_lp_src_yclamp_th = 0;
				di_im_di_nintra_search_dir_limit.nintra_hdiff_th1=3;
				di_im_di_nintra_control.nintra_min_second_dir_check2_en=0;
				di_im_di_nintra_control.nintra_min_second_dir_check_en =0;

 			}
#ifdef CONFIG_HW_SUPPORT_MEMC
			else if(SandM_Chroma_Zone_Plate == 1)
			{
				color_temp_ctrl_reg.temporal_enable = 0;
			}
#endif //CONFIG_HW_SUPPORT_MEMC

			di_im_new_mcnr_pan_condition.regValue = rtd_inl(DI_IM_NEW_MCNR_PAN_CONDITION_reg);
			//di_im_new_mcnr_control4.regValue = rtd_inl(DI_IM_NEW_MCNR_CONTROL4_reg); //mac6 used old mcnr

#if 0 // mac6 no MCNR
			if(scalerVIP_DI_MiddleWare_MCNR_Get_V_Pan_Flag()==1&&sat_ratio_mean_value<50) //tokoy night street
			{
				di_smd_control_candiate_reg.smd_upperbound_sad_forjudgemv=170;
				smd_choosecandidate_reg.smd_reliablerange_sad_th=0;
				//di_di_smd_resurrect_ctrl.smd_mv_not_gmv_resurrect_en=1; //mac5p removed
				//di_di_smd_resurrect_ctrl.smd_resurrect_noacc_en=1; //mac5p removed
				//di_di_smd_sampling_gmvc.smd_close2gmv_fw_en=1; //mac5p removed
				if(init_di_smd_additionalcoeff1_reg.smd_result_en!=1)
				{
					di_double_buff_apply =1;
				}

				init_di_smd_additionalcoeff1_reg.smd_result_en=1;
				//di_di_smd_waterflag_ctr3.smd_fw_gmvy= 5;//-(di_im_new_mcnr_control4.n_mcnr_v_pan_mv-6)+3; //mac5p removed
				//di_di_smd_hardwarepan_modify1.smd_fw_gmvx=-(di_im_new_mcnr_pan_condition.h_pan_mv_value-15)+3; //mac5p removed
			}
			else
#endif //mac6 no MCNR
			{

				if(init_di_smd_additionalcoeff1_reg.smd_result_en!=ori_smd_result_en)
				{
					di_double_buff_apply =1;
				}
				init_di_smd_additionalcoeff1_reg.smd_result_en=ori_smd_result_en;
			}
			//ROSPrintf("\n sssss %d \n ",scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN_SPECIAL_CASE));
			//#14
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);

			//#23
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);


			//#24
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);

			//ROSPrintf("\n ddd %d \n",im_di_weave_window_control_reg.regValue);
			//#107
			rtd_outl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, im_di_weave_window_control_reg.regValue);
			rtd_outl(DI_IM_DI_WEAVE_WINDOW_1_START_reg, im_di_weave_window_1_start_reg.regValue);
			rtd_outl(DI_IM_DI_WEAVE_WINDOW_1_END_reg, im_di_weave_window_1_end_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_ADJUSTABLE_reg, di_hmc_adjustable_reg.regValue);
			rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_PAR_2_reg, di_hmc_pan_control_par_2_reg.regValue);

			rtd_outl(DI_DI_GST_Ctrl_reg,init_di_gst_ctrl_reg.regValue);
			rtd_outl(DI_DI_SMD_CandidateLimitSAD_reg, init_di_smd_candidatelimitsad_reg.regValue);
#ifdef CONFIG_HW_SUPPORT_MEMC
			rtd_outl(MEMC_MUX_MEMC_MUX_DI_CTRL_reg, color_temp_ctrl_reg.regValue);
#endif //CONFIG_HW_SUPPORT_MEMC
			rtd_outl(DI_DI_SMD_AdditionalCoeff2_reg, init_di_smd_additionalcoeff2_reg.regValue);

			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);

			rtd_outl(DI_DI_SMD_Slow_Motion_handle1_reg, di_di_smd_slow_motion_handle1.regValue);
			rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg, init_di_smd_additionalcoeff1_reg.regValue);
			//rtd_outl(DI_DI_SMD_SLOW_MOTION_HANDLE2_reg, di_di_smd_slow_motion_handle2.regValue);

			rtd_outl(DI_DI_SMD_RefineMV2_reg, di_smd_refinemv2_reg.regValue);

			rtd_outl(DI_IM_DI_BTR_CONTROL_reg, di_im_di_btr_control_reg.regValue);

			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, di_ma_frame_motion_th_c_reg.regValue);


			//#173
			/* //mac6 remove film_mask_detection
			film_mask_detection_reg.regValue = rtd_inl(DI_IM_DI_FILM_MASK_DETECTION_reg);

			if(DynamicOptimizeSystem[173]==1)
			{
				film_mask_detection_reg.film_mask_en = 0;
			}
			else
			{
				film_mask_detection_reg.film_mask_en = 1;
			}

			rtd_outl(DI_IM_DI_FILM_MASK_DETECTION_reg, film_mask_detection_reg.regValue);
			*/

			//#176
			rtd_outl(DI_IM_DI_FILM_SAWTOOTH_FILMFRAME_TH_reg, im_di_film_sawtooth_filmframe_th_reg.regValue);
			rtd_outl(DI_IM_DI_FILM_STATIC_SUM_TH_reg, film_static_sum_th_reg.regValue);
			rtd_outl(DI_IM_DI_FILM_MOTION_SUM_TH_reg, film_motion_sum_th_reg.regValue);

			//#178
			rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
			rtd_outl(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);
			//rtd_outl(DI_DI_SMD_Resurrect_Ctrl_reg, di_di_smd_resurrect_ctrl.regValue); //mac5p removed
			rtd_outl(DI_DI_SMD_Sampling_GMVc_reg, di_di_smd_sampling_gmvc.regValue);

			rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg, init_di_smd_additionalcoeff1_reg.regValue);

			rtd_outl(DI_DI_SMD_WaterFlag_Ctr3_reg, di_di_smd_waterflag_ctr3.regValue);

			rtd_outl(DI_DI_SMD_HardwarePan_Modify1_reg, di_di_smd_hardwarepan_modify1.regValue);


			//#187
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);

			//#197
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);

			//#198
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);
			rtd_outl(DI_IM_DI_NINTRA_LOWPASS_SOURCE_reg,di_im_di_nintra_lowpass_source.regValue);
			rtd_outl(DI_IM_DI_NINTRA_SEARCH_DIR_LIMIT_reg,di_im_di_nintra_search_dir_limit.regValue);
			rtd_outl(DI_IM_DI_NINTRA_CONTROL_reg,di_im_di_nintra_control.regValue);

			di_im_new_mcnr_pan_condition.regValue = rtd_inl(DI_IM_NEW_MCNR_PAN_CONDITION_reg);

			//Pan Case
			if(scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN_SPECIAL_CASE)< 15)
			{
				if(DynamicOptimizeSystem[107]==1)
				{
					di_ma_frame_motion_th_a_reg.regValue = 0x12270033;//0x2c272e3a;
					di_ma_frame_motion_th_b_reg.regValue = 0x60560a4b;//0x54565060;
					di_ma_frame_motion_th_c_reg.regValue = 0x76781462;//0x7678787d;

				}
				else if(DI_Video_Broken==1)
				{
					di_ma_frame_motion_th_a_reg.regValue = 0x05040a05;//0x2c272e3a;
					di_ma_frame_motion_th_b_reg.regValue = 0x180a0c14;//0x54565060;
					di_ma_frame_motion_th_c_reg.regValue = 0x32141018;//0x7678787d;
				}
				else if(DI_Res_Pat_pan_65ppf == 1)
				{
					di_ma_frame_motion_th_a_reg.ma_framerlvtha =0x0;
				}
				else
				{
					/*rock_rau 20150922 Q-tek monoscope pan flicker issue(original setting)*/
					di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;
					di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;
					di_ma_frame_motion_th_c_reg.ma_framehthc = ORI_di_ma_frame_motion_th_c_reg.ma_framehthc;

				}
			}
			else if((scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN_SPECIAL_CASE) < 30)&&
				(di_im_new_mcnr_pan_condition.h_pan_mv_value==0))
			{
				di_ma_frame_motion_th_a_reg.ma_framehtha = 0x0a;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 0x12;
				di_ma_frame_motion_th_c_reg.ma_framehthc = 0x16;
			}
			else if ((scalerVIP_DI_MiddleWare_GetStatus(STATUS_PAN_SPECIAL_CASE) > 30)&&
				(di_im_new_mcnr_pan_condition.h_pan_mv_value==0))
			{
				di_ma_frame_motion_th_a_reg.ma_framehtha = 0x14;
				di_ma_frame_motion_th_b_reg.ma_framehthb = 0x16;
				di_ma_frame_motion_th_c_reg.ma_framehthc = 0x18;
			}

			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, di_ma_frame_motion_th_c_reg.regValue);

		}
		break;
		case VIP_QUALITY_HDMI_1080P:
		case VIP_QUALITY_YPbPr_1080P:
		{
#ifdef CONFIG_HW_SUPPORT_MEMC
			//#Spears & Munsil ADVANCED VUDEO - EVALUATION - Chroma Zone Plate
			color_temp_ctrl_reg.regValue = ORI_color_temp_ctrl_reg.regValue;


			if(SandM_Chroma_Zone_Plate == 1)
			{
				color_temp_ctrl_reg.temporal_enable = 0;
			}

			rtd_outl(MEMC_MUX_MEMC_MUX_DI_CTRL_reg, color_temp_ctrl_reg.regValue);
#endif //CONFIG_HW_SUPPORT_MEMC
		}
		break;
		case VIP_QUALITY_TV_SECAN:
		{

			if(LGPTFatoryMultiC34 == 1)
			{
				postp_secam_fix_ctrl_reg.regValue = 0x4000ff0c;
				postp_secam_fix_y_detect0_reg.regValue = 0xbf01a4a4;
				postp_secam_fix_y_detect1_reg.regValue = 0x3f3f0800;

				/*For color bar H-line flick*/
				tnrxc_mkii_ctrl_reg.regValue = 0x14b40401;
			}
			else
			{
				postp_secam_fix_ctrl_reg.regValue = ORI_postp_secam_fix_ctrl_reg.regValue;
				postp_secam_fix_y_detect0_reg.regValue = ORI_postp_secam_fix_y_detect0_reg.regValue;
				postp_secam_fix_y_detect1_reg.regValue = ORI_postp_secam_fix_y_detect1_reg.regValue;

				/*For color bar H-line flick*/
				tnrxc_mkii_ctrl_reg.regValue = ORI_tnrxc_mkii_ctrl_reg.regValue;
			}
			rtd_outl(DI_TNRXC_MkII_CTRL_reg, tnrxc_mkii_ctrl_reg.regValue);
#if 0
			VDPQ_rtd_outl(VDPQ_POSTP_SECAM_FIX_CTRL_reg, postp_secam_fix_ctrl_reg.regValue);
			VDPQ_rtd_outl(VDPQ_POSTP_SECAM_FIX_Y_DETECT0_reg, postp_secam_fix_y_detect0_reg.regValue);
			VDPQ_rtd_outl(VDPQ_POSTP_SECAM_FIX_Y_DETECT1_reg, postp_secam_fix_y_detect1_reg.regValue);
#endif
		}
		break;
		case VIP_QUALITY_TV_PAL:
		{
			LGTVPALBGE52 = 1;//[KTASKWBS-3528] fix LGE channel E-52 Shake issue.
			// fix enable tnr XC (original only LGTVPALBGE52 pattern enable)

			if( LGTVPALBGE52 == 1 )
			{
				tnrxc_mkii_ctrl_reg.regValue = 0x14b40401;
			}
			else
			{
				tnrxc_mkii_ctrl_reg.regValue = ORI_tnrxc_mkii_ctrl_reg.regValue;
			}
			rtd_outl(DI_TNRXC_MkII_CTRL_reg, tnrxc_mkii_ctrl_reg.regValue);
		}
		break;
		case VIP_QUALITY_HDMI_480P:
		case VIP_QUALITY_HDMI_576P:
		{

			//************** INI VALE START    ***************************************************

			//#147,148,149,150
			dch1_Spatial_Weight_Reg.cp_fixedweight1y = ori_cp_fixedweight1y;
			dch1_Spatial_Weight_Reg.cp_fixedweight2y = ori_cp_fixedweight2y;

			//#192
			dch1_cp_ctrl_reg.cp_sresultweight = ori_cp_sresultweight;
			curve_mapping_w1_1_reg.curvemapping_w1_2=ori_curve_mapping_w1_2;
			curve_mapping_w1_1_reg.curvemapping_w1_3=ori_curve_mapping_w1_3;
			curve_mapping_w1_2_reg.curvemapping_w1_4=ori_curve_mapping_w1_4;
			curve_mapping_w1_2_reg.curvemapping_w1_5=ori_curve_mapping_w1_5;
			curve_mapping_step2_reg.curvemapping_step6=ori_curve_mapping_step6;
			curve_mapping_step3_reg.curvemapping_step7=ori_curve_mapping_step7;
			curve_mapping_step3_reg.curvemapping_step8=ori_curve_mapping_step8;

			//************** INI VALE END    ***************************************************

			if(DynamicOptimizeSystem[147] ==1)
			{
				dch1_Spatial_Weight_Reg.cp_fixedweight1y = 1;
				dch1_Spatial_Weight_Reg.cp_fixedweight2y = 1;
 			}
			else if(DynamicOptimizeSystem[192])
			{
				dch1_cp_ctrl_reg.cp_sresultweight =15;
				curve_mapping_w1_1_reg.curvemapping_w1_2=8;
				curve_mapping_w1_1_reg.curvemapping_w1_3=0;
				curve_mapping_w1_2_reg.curvemapping_w1_4=0;
				curve_mapping_w1_2_reg.curvemapping_w1_5=0;
				curve_mapping_step2_reg.curvemapping_step6=14;
				curve_mapping_step3_reg.curvemapping_step7=30;
				curve_mapping_step3_reg.curvemapping_step8=62;
			}

			//#147,148,149,150
			rtd_outl(NR_DCH1_Spatial_Weight_reg, dch1_Spatial_Weight_Reg.regValue);

			//#192
			rtd_outl(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
			//rtd_outl(NR_CURVE_MAPPING_W1_1_reg, curve_mapping_w1_1_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_W1_2_reg, curve_mapping_w1_2_reg.regValue);		//LGE control
			//rtd_outl(NR_CURVE_MAPPING_STEP_2_reg, curve_mapping_step2_reg.regValue);	//LGE control
			//rtd_outl(NR_CURVE_MAPPING_STEP_3_reg, curve_mapping_step3_reg.regValue);	//LGE control

		}
		break;
		case VIP_QUALITY_DTV_480I:
		case VIP_QUALITY_DTV_576I:
		case VIP_QUALITY_DTV_480P:
		case VIP_QUALITY_DTV_576P:
		case VIP_QUALITY_DTV_720P:
		{
			di_smd_control_candiate_reg.regValue = rtd_inl(DI_DI_SMD_Control_CandidateList_reg);
			if(CITestPatch[0] == 1)
			{
				di_ma_frame_motion_th_a_reg.regValue = 0;
				di_ma_frame_motion_th_b_reg.regValue = 0;
				di_ma_frame_motion_th_c_reg.regValue = 0;
				di_smd_control_candiate_reg.smd_en=0;
				rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
				rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);
				rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, di_ma_frame_motion_th_c_reg.regValue);
				rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);

			}
			else
			{
				di_ma_frame_motion_th_a_reg.regValue = ORI_di_ma_frame_motion_th_a_reg.regValue;
				di_ma_frame_motion_th_b_reg.regValue = ORI_di_ma_frame_motion_th_b_reg.regValue;
				di_ma_frame_motion_th_c_reg.regValue = ORI_di_ma_frame_motion_th_c_reg.regValue;
				di_smd_control_candiate_reg.smd_en=ori_smd_en;
				rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
				rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);
				rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, di_ma_frame_motion_th_c_reg.regValue);
				rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);

			}


			//static unsigned char ori_film_status = 0;
			//static unsigned char ori_film_sawtooth_choose = 0;
			//ROSPrintf("\n DTV_SD_p2i =%d \n ",DTV_SD_p2i);

#ifdef CONFIG_HW_SUPPORT_MEMC
			#if 0 //fixed me
			unsigned char DTV_SD_p2i = RPC_SmartPic_clue->VCPU_setting_info.VCPU_DTV_SD_p2i;
			di_im_di_film_sawtooth_filmframe_th_RBUS film_sawtooth_filmframe_th_reg;

			if(((rtd_inl(0xb809d620)>>13)&0x7f)!=0) //this is for dtv patch
			{
				di_control_reg.regValue = rtd_inl(DI_IM_DI_CONTROL_reg);
				film_sawtooth_filmframe_th_reg.regValue = rtd_inl(DI_IM_DI_FILM_SAWTOOTH_FILMFRAME_TH_reg);

				if((DTV_SD_p2i ==1)&&((set_p2i_value==0)||di_control_reg.film_enable!=28))
				{
					//ROSPrintf("\n DTV_SD_p2i 1 =%d \n ",DTV_SD_p2i);


					set_p2i_value =1;
					//ori_film_status = di_control_reg.film_enable;
					//ori_film_sawtooth_choose = di_control_reg.film_sawtooth_choose;

					di_control_reg.film_enable = 28;
					di_control_reg.film_sawtooth_choose = 3;
					film_sawtooth_filmframe_th_reg.film_frmotionthl = 2;
					di_control_reg.write_enable_5=1;

					rtd_outl(DI_IM_DI_CONTROL_reg,di_control_reg.regValue);
					rtd_outl(DI_IM_DI_FILM_SAWTOOTH_FILMFRAME_TH_reg,film_sawtooth_filmframe_th_reg.regValue);
				}
				else if ((DTV_SD_p2i ==0)&&(set_p2i_value==1))
				{

					//ROSPrintf("\n DTV_SD_p2i 2 =%d \n ",DTV_SD_p2i);
					//set_value =0;

					//di_control_reg.film_enable = ori_film_status ;
					//di_control_reg.film_sawtooth_choose= ori_film_sawtooth_choose;

					//rtd_outl(DI_IM_DI_CONTROL_reg,di_control_reg.regValue);

				}


				/*else if ((DTV_SD_p2i ==0)&&(set_value==1))
				{
					set_value =0;
					di_control_reg.regValue = rtd_inl(DI_IM_DI_CONTROL_reg);
					di_control_reg.film_enable = ori_film_status ;
					di_control_reg.film_sawtooth_choose= ori_film_sawtooth_choose;
					rtd_outl(DI_IM_DI_CONTROL_reg,di_control_reg.regValue);

				}*/

			}
			#endif
#endif //CONFIG_HW_SUPPORT_MEMC



		}
		break;
		case VIP_QUALITY_DTV_1080I:
		{




			if(DI_Video_Broken==1)
			{
				di_ma_frame_motion_th_a_reg.regValue = 0x05040a05;//0x2c272e3a;
				di_ma_frame_motion_th_b_reg.regValue = 0x180a0c14;//0x54565060;
				di_ma_frame_motion_th_c_reg.regValue = 0x32141018;//0x7678787d;

			}
			else
			{

				di_ma_frame_motion_th_a_reg.ma_framehtha = ORI_di_ma_frame_motion_th_a_reg.ma_framehtha;
				di_ma_frame_motion_th_b_reg.ma_framehthb = ORI_di_ma_frame_motion_th_b_reg.ma_framehthb;
				di_ma_frame_motion_th_c_reg.ma_framehthc = ORI_di_ma_frame_motion_th_c_reg.ma_framehthc;


			}

			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);
			rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, di_ma_frame_motion_th_c_reg.regValue);


		}


		break;
		case VIP_QUALITY_DTV_1080P:
		{
			di_im_di_rtnr_c_th0_th3_RBUS_reg.regValue = rtd_inl(DI_IM_DI_RTNR_C_TH0_TH3_reg);

			di_im_di_rtnr_c_th4_th6_RBUS_reg.regValue = rtd_inl(DI_IM_DI_RTNR_C_TH4_TH6_reg);

			if (LGBirdCRTNRPatchTrigger) {
				if (LGBirdCRTNRPatch) {
					di_im_di_rtnr_c_th0_th3_RBUS_reg_save.regValue = di_im_di_rtnr_c_th0_th3_RBUS_reg.regValue;
					di_im_di_rtnr_c_th4_th6_RBUS_reg_save.regValue = di_im_di_rtnr_c_th4_th6_RBUS_reg.regValue;
					di_im_di_rtnr_c_th0_th3_RBUS_reg.regValue = 0;
					di_im_di_rtnr_c_th4_th6_RBUS_reg.regValue = 0;
				} else {
					di_im_di_rtnr_c_th0_th3_RBUS_reg.regValue = di_im_di_rtnr_c_th0_th3_RBUS_reg_save.regValue;
					di_im_di_rtnr_c_th4_th6_RBUS_reg.regValue = di_im_di_rtnr_c_th4_th6_RBUS_reg_save.regValue;
				}
				rtd_outl(DI_IM_DI_RTNR_C_TH0_TH3_reg, di_im_di_rtnr_c_th0_th3_RBUS_reg.regValue);
				rtd_outl(DI_IM_DI_RTNR_C_TH4_TH6_reg, di_im_di_rtnr_c_th4_th6_RBUS_reg.regValue);
			} else if (!LGBirdCRTNRPatch) {
				di_im_di_rtnr_c_th0_th3_RBUS_reg_save.regValue = di_im_di_rtnr_c_th0_th3_RBUS_reg.regValue;
				di_im_di_rtnr_c_th4_th6_RBUS_reg_save.regValue = di_im_di_rtnr_c_th4_th6_RBUS_reg.regValue;
			}

		}
		break;


	}
	if(di_double_buff_apply ==1)
	{
		di_db_reg_ctl_RBUS  db_reg_ctl_reg;

		db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
		db_reg_ctl_reg.db_apply=1;
		rtd_outl(DI_db_reg_ctl_reg,db_reg_ctl_reg.regValue);
		di_double_buff_apply =0;
	}

// new structure end
}

/*#endif*/


int scalerVIP_2_Complement(unsigned int nVal,unsigned char nBitValid)
{
	return ((nVal >> nBitValid) ? (nVal | (short)(~((1 << (nBitValid+1)) - 1))) : (nVal));
}


void scalerVIP_Set_Stop_Polling(void)
{
	vip_color_dcc_d_dcc_fwused_1_RBUS d_dcc_fwused_1_reg;
	d_dcc_fwused_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_FWUSED_1_reg);
	Stop_Polling.Stop_All= d_dcc_fwused_1_reg.regValue &0x01;
	Stop_Polling.Stop_Di_Hmc=(d_dcc_fwused_1_reg.regValue>>1) &0x01;
	Stop_Polling.Stop_Dcc= (d_dcc_fwused_1_reg.regValue>>2) &0x01;
	Stop_Polling.Stop_Auto_Nr=(d_dcc_fwused_1_reg.regValue>>3) &0x01;
	Stop_Polling.Stop_Dcti_for_colorbar=(d_dcc_fwused_1_reg.regValue>>4) &0x01;
	Stop_Polling.Stop_Noise_Mpeg=(d_dcc_fwused_1_reg.regValue>>5) &0x01;
	Stop_Polling.Stop_Dcr=(d_dcc_fwused_1_reg.regValue>>6) &0x01;
	Stop_Polling.Stop_Film_Mode_Detector=(d_dcc_fwused_1_reg.regValue>>7) &0x01;
	Stop_Polling.Stop_VD_Noise=(d_dcc_fwused_1_reg.regValue>>8) &0x01;
	Stop_Polling.Stop_Di_Smd=(d_dcc_fwused_1_reg.regValue>>9) &0x01;

	Stop_Polling.Stop_pattern_Monscope=(d_dcc_fwused_1_reg.regValue>>10) &0x01;
	Stop_Polling.Stop_pattern_ColorBar=(d_dcc_fwused_1_reg.regValue>>11) &0x01;
	Stop_Polling.Stop_pattern_No2=(d_dcc_fwused_1_reg.regValue>>12) &0x01;
	Stop_Polling.Stop_pattern_No4=(d_dcc_fwused_1_reg.regValue>>13) &0x01;
	Stop_Polling.Stop_pattern_No23=(d_dcc_fwused_1_reg.regValue>>14) &0x01;
	Stop_Polling.Stop_pattern_No24=(d_dcc_fwused_1_reg.regValue>>15) &0x01;
	Stop_Polling.Stop_pattern_No47=(d_dcc_fwused_1_reg.regValue>>16) &0x01;
	Stop_Polling.Stop_pattern_No107=(d_dcc_fwused_1_reg.regValue>>17) &0x01;
	Stop_Polling.Stop_pattern_No154=(d_dcc_fwused_1_reg.regValue>>18) &0x01;
	Stop_Polling.Stop_Reset_Pattern=(d_dcc_fwused_1_reg.regValue>>19) &0x01;

}
void scalerVIP_Set_Start_Print_Info()
{
	vip_color_dcc_d_dcc_fwused_2_RBUS  d_dcc_fwused_2_reg;
	vip_color_dcc_d_dcc_fwused_3_RBUS  d_dcc_fwused_3_reg;
	static unsigned char init=0;
	unsigned char i=0;
	unsigned int DCC_Histogram_Mean_value = 0;
	DCC_Histogram_Mean_value = SmartPic_clue->Hist_Y_Mean_Value;


	d_dcc_fwused_2_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_FWUSED_2_reg);
	d_dcc_fwused_3_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_FWUSED_3_reg);

	if (init == TRUE && ((d_dcc_fwused_2_reg.regValue&0xff) == 0))
	{
		init = FALSE;
	}

	if(init==FALSE)
	{
		init =TRUE;
		d_dcc_fwused_2_reg.delay_time= 150; //default dealy time=150
		d_dcc_fwused_2_reg.fw_2 =0;
		rtd_outl(COLOR_DCC_D_DCC_FWUSED_2_reg, d_dcc_fwused_2_reg.regValue);
	}
	//else
	{
		Start_Print.Delay_Time= d_dcc_fwused_2_reg.regValue &0xff;
		Start_Print.Y_Hist=( d_dcc_fwused_2_reg.regValue>>8) &0x01;
		Start_Print.Hue_Hist= (d_dcc_fwused_2_reg.regValue>>9) &0x01;
		Start_Print.Sat_Mean=(d_dcc_fwused_2_reg.regValue>>10) &0x01;
		Start_Print.Dcc_1=(d_dcc_fwused_2_reg.regValue>>11) &0x01;
		Start_Print.Dcc_2=(d_dcc_fwused_2_reg.regValue>>12) &0x01;
		Start_Print.DCC_3=(d_dcc_fwused_2_reg.regValue>>13) &0x01;
		Start_Print.Rtnr_1=(d_dcc_fwused_2_reg.regValue>>14) &0x01;
		Start_Print.Rtnr_2=(d_dcc_fwused_2_reg.regValue>>15) &0x01;
		Start_Print.Mpeg_Nr=(d_dcc_fwused_2_reg.regValue>>16) &0x01;
		Start_Print.Snr=(d_dcc_fwused_2_reg.regValue>>17) &0x01;
		Start_Print.Sharpness_1=(d_dcc_fwused_2_reg.regValue>>18) &0x01;
		Start_Print.Sharpness_2=(d_dcc_fwused_2_reg.regValue>>19) &0x01;
		Start_Print.DI_1=(d_dcc_fwused_2_reg.regValue>>20) &0x01;
		Start_Print.DI_2=(d_dcc_fwused_2_reg.regValue>>21) &0x01;
		Start_Print.Film_Mode=(d_dcc_fwused_2_reg.regValue>>22) &0x01;
		Start_Print.HIST=(d_dcc_fwused_2_reg.regValue>>23) &0x01;
		Start_Print.Reverse_2=(d_dcc_fwused_2_reg.regValue>>24) &0x01;
		Start_Print.Reverse_3=(d_dcc_fwused_2_reg.regValue>>25) &0x01;

		Start_Print.ID_Pattern_ALL=(d_dcc_fwused_3_reg.regValue>>0) &0x01;
		Start_Print.ID_Pattern_2=(d_dcc_fwused_3_reg.regValue>>1) &0x01;
		Start_Print.ID_Pattern_4=(d_dcc_fwused_3_reg.regValue>>2) &0x01;
		Start_Print.ID_Pattern_5=(d_dcc_fwused_3_reg.regValue>>3) &0x01;
		Start_Print.ID_Pattern_7=(d_dcc_fwused_3_reg.regValue>>4) &0x01;
		Start_Print.ID_Pattern_12=(d_dcc_fwused_3_reg.regValue>>5) &0x01;
		Start_Print.ID_Pattern_23=(d_dcc_fwused_3_reg.regValue>>6) &0x01;
		Start_Print.ID_Pattern_24=(d_dcc_fwused_3_reg.regValue>>7) &0x01;
		Start_Print.ID_Pattern_35=(d_dcc_fwused_3_reg.regValue>>8) &0x01;
		Start_Print.ID_Pattern_47=(d_dcc_fwused_3_reg.regValue>>9) &0x01;
		Start_Print.ID_Pattern_107=(d_dcc_fwused_3_reg.regValue>>10) &0x01;
		Start_Print.ID_Pattern_123=(d_dcc_fwused_3_reg.regValue>>11) &0x01;
		Start_Print.ID_Pattern_132=(d_dcc_fwused_3_reg.regValue>>12) &0x01;
		Start_Print.ID_Pattern_133=(d_dcc_fwused_3_reg.regValue>>13) &0x01;
		Start_Print.ID_Pattern_154=(d_dcc_fwused_3_reg.regValue>>14) &0x01;
		Start_Print.ID_Pattern_191=(d_dcc_fwused_3_reg.regValue>>15) &0x01;
		Start_Print.ID_Pattern_192=(d_dcc_fwused_3_reg.regValue>>16) &0x01;
		Start_Print.ID_Pattern_193=(d_dcc_fwused_3_reg.regValue>>17) &0x01;
		Start_Print.ID_Pattern_194=(d_dcc_fwused_3_reg.regValue>>18) &0x01;
		Start_Print.ID_Pattern_195=(d_dcc_fwused_3_reg.regValue>>19) &0x01;
		Start_Print.ID_Pattern_139=(d_dcc_fwused_3_reg.regValue>>20) &0x01;
		Start_Print.ID_Pattern_196=(d_dcc_fwused_3_reg.regValue>>21) &0x01;
		Start_Print.ID_Pattern_14=(d_dcc_fwused_3_reg.regValue>>22) &0x01;
		Start_Print.someothers=(d_dcc_fwused_3_reg.regValue>>23) &0x01;
		Start_Print.ID_Pattern_32=(d_dcc_fwused_3_reg.regValue>>24) &0x01;
		Start_Print.ID_Pattern_147=(d_dcc_fwused_3_reg.regValue>>25) &0x01;
		Start_Print.Reverse_10=(d_dcc_fwused_3_reg.regValue>>26) &0x01;
		Start_Print.Reverse_11=(d_dcc_fwused_3_reg.regValue>>27) &0x01;
		Start_Print.VD_LOG_ENABLE=(d_dcc_fwused_3_reg.regValue>>28) &0x01;
		Start_Print.ColorBar=(d_dcc_fwused_3_reg.regValue>>29) &0x01;
		Start_Print.Slow_Motion1=(d_dcc_fwused_3_reg.regValue>>30) &0x01;
		Start_Print.Slow_Motion2=(d_dcc_fwused_3_reg.regValue>>31) &0x01;

	}
	if(Start_Print.Delay_Time ==0)
	{
		Start_Print.Delay_Time =1;
	}

	// print   Y_hist_ratio
	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Y_Hist==TRUE))
	{
		ROSPrintf("{");
		for(i=0;i<TV006_VPQ_chrm_bin;i++)
			ROSPrintf("%d, ",Y_hist_ratio[i]);
		ROSPrintf("}\n");

	}

	// print   hue_hist_ratio
	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Hue_Hist==TRUE))
	{

		ROSPrintf("{");
		for(i=0;i<24;i++)
			ROSPrintf("%d, ",hue_hist_ratio[i]);
		ROSPrintf("}\n");

	}

	// print   sat_mean_value
	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Sat_Mean==TRUE))
	{
		ROSPrintf("sat_mean_value=%d \n", sat_ratio_mean_value);
	}

	// print   DCC_Histogram_Mean_value
	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Dcc_1==TRUE))
	{
		ROSPrintf(" DCC_Histogram_Mean_value=%d\n", DCC_Histogram_Mean_value);
	}

	// print   RTNR_MAD_count_Y_sum_avg
	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Rtnr_2==TRUE))
	{
		ROSPrintf(" RTNR_MAD_count_Y_sum_avg=%d\n", RTNR_MAD_count_Y_sum_avg);
	}

	// print   Identiy ID Pattern
	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_ALL==TRUE))
	{
		for(i=0;i<200;i++)
		{
			if(DynamicOptimizeSystem[i]!=0)
			{
				ROSPrintf("source=%d,No[%d]=%d \n",(RPC_system_info_structure_table->VIP_source),i,DynamicOptimizeSystem[i]);
			}
		}
	}
}
void scalerVIP_Sat_Hist_Calculate(void)
{
	//change according to sirius by leonard_wu@20140224

	unsigned char i, j, hidx;
	unsigned int sum = 0;
	unsigned short mean_tmp = 0;
	//unsigned short tmptmp = 0;
	//unsigned short Y_mean = 0;

	unsigned char total_sat_hist = 1;
	int timeout = 10;

	color_icm_d_auto_sat_hist_read_ctrl_RBUS		d_auto_sat_hist_read_ctrl_reg;
	color_icm_d_auto_sat_hist_read_flag_RBUS		d_auto_sat_hist_read_flag_reg;
	color_icm_d_auto_sat_hist_cnt_00_or_16_RBUS	d_auto_sat_hist_cnt_00_or_16_reg;

	unsigned int printf_Flag = system_info_structure_table->ISR_Printf_Flag.Y_Hue_Sat_Hist_Flag0;
	unsigned int print_delay = (system_info_structure_table->ISR_Printf_Flag.Delay_Time == 0)?(1):(system_info_structure_table->ISR_Printf_Flag.Delay_Time);

	//write clear first
	d_auto_sat_hist_read_flag_reg.regValue = rtd_inl(COLOR_ICM_D_AUTO_SAT_HIST_READ_FLAG_reg);
	d_auto_sat_hist_read_flag_reg.sat_hist_sram_read_consistent_flag = 0;
	rtd_outl(COLOR_ICM_D_AUTO_SAT_HIST_READ_FLAG_reg, d_auto_sat_hist_read_flag_reg.regValue);
	d_auto_sat_hist_read_flag_reg.sat_hist_sram_read_consistent_flag = 1;
	rtd_outl(COLOR_ICM_D_AUTO_SAT_HIST_READ_FLAG_reg, d_auto_sat_hist_read_flag_reg.regValue);

	//read sat hist 0; if you want to read all 4 hists, set total_sat_hist = 4 and sat_hist should be change too.
	total_sat_hist = 1;
	for(hidx = 0; hidx < total_sat_hist; hidx++)
	{
		sum = 0;
		mean_tmp = 0;
		for(i = 0; i < 2; i++)
		{
			d_auto_sat_hist_read_ctrl_reg.regValue = rtd_inl(COLOR_ICM_D_AUTO_SAT_HIST_READ_CTRL_reg);
			d_auto_sat_hist_read_ctrl_reg.sram_read = 1;
			d_auto_sat_hist_read_ctrl_reg.hist_ctrl = hidx;
			d_auto_sat_hist_read_ctrl_reg.cnt_block = i;
			rtd_outl(COLOR_ICM_D_AUTO_SAT_HIST_READ_CTRL_reg, d_auto_sat_hist_read_ctrl_reg.regValue);

			//histogram has been update from SRAM to registers?
			timeout = 10;
			do{
				d_auto_sat_hist_read_ctrl_reg.regValue = rtd_inl(COLOR_ICM_D_AUTO_SAT_HIST_READ_CTRL_reg);
			}while(d_auto_sat_hist_read_ctrl_reg.sram_read && (timeout-- > 0));

			if(timeout < 0)
				goto FINISH; //read back data error

			//read histogram from registers
			for(j = 0; j < 16; j++)
			{
				d_auto_sat_hist_cnt_00_or_16_reg.regValue = rtd_inl(COLOR_ICM_D_AUTO_SAT_HIST_CNT_00_OR_16_reg + (j * 4));
				sat_hist[i * 16 + j] = d_auto_sat_hist_cnt_00_or_16_reg.sat_hist_cnt_00_or_16;
        sum = sum + (sat_hist[i * 16 + j] >> 1); // shift 1 bit to avoid overflow when calculating ratio
			}
			//Y_mean = tmptmp>>5;

			//check consistent
			d_auto_sat_hist_read_flag_reg.regValue = rtd_inl(COLOR_ICM_D_AUTO_SAT_HIST_READ_FLAG_reg);

			if(d_auto_sat_hist_read_flag_reg.sat_hist_sram_read_consistent_flag)
				goto FINISH; //read back data error

		}
		//
		if(sum != 0)
		{
			for(i = 0; i < 32; i++)
			{
        sat_hist_ratio[i] = (sat_hist[i]>>1) * 1000 / sum; // shift 1 bit to match the sum
				mean_tmp += sat_hist_ratio[i] * (i + 1);//sat_hist_ratio[i]*(i+1)/COLOR_AutoSat_HISTOGRAM_LEVEL;
				//tmptmp += Y_hist_ratio[i]*(i+1);

				(SmartPic_clue->Sat_Main_His)[i] = sat_hist[i];
				(SmartPic_clue->Sat_Main_His_Ratio)[i]= sat_hist_ratio[i];

			}


		}
		if((SmartPic_clue->SCALER_DISP_INFO.DispLen*SmartPic_clue->SCALER_DISP_INFO.DispWid)!=0)
		{
      SmartPic_clue->sat_sumcnt_ratio = sum*1000/((SmartPic_clue->SCALER_DISP_INFO.DispLen*SmartPic_clue->SCALER_DISP_INFO.DispWid)>>1); // shift 1 bit to match the sum
		}
		sat_ratio_mean_value = mean_tmp >> 5;	//COLOR_AutoSat_HISTOGRAM_LEVEL = 32 ;

		SmartPic_clue->sat_ratio_mean_value = sat_ratio_mean_value;

    }



FINISH:
#if 1
	if(MA_print_count%100 == 0 && meanSat_meanValue_printf_log_flag)
		ROSPrintf("sat_mean_value=%d, Histogram_Mean_value=%d\n", sat_ratio_mean_value,(SmartPic_clue->Hist_Y_Mean_Value));
#endif
#if 0
	if(MA_print_count%300==0)
	{
		//ROSPrintf("sat_mean_value=%d,mean_tmp=%d,Histogram_Mean_value=%d,tmptmp=%d,Y_mean=%d\n",
			//sat_mean_value,mean_tmp,(SmartPic_clue->Hist_Y_Mean_Value),tmptmp,Y_mean);
		for(i=0;i<32;i++)
			ROSPrintf("%d,",sat_hist_ratio[i]);
		ROSPrintf("\n");
		for(i=0;i<TV006_VPQ_chrm_bin;i++)
			ROSPrintf("%d,",Y_hist_ratio[i]);
		ROSPrintf("\n");
	}
#endif

#if 1
	//for debug.
	if(((printf_Flag&_BIT4)!=0))
	{
		if(MA_print_count%print_delay==0)
		{
			rtd_pr_vpq_isr_info("Y_Hue_Sat cal, Sat={");
			for(i=0;i<COLOR_AutoSat_HISTOGRAM_LEVEL;i++)
				rtd_pr_vpq_isr_info("%d,", SmartPic_clue->Sat_Main_His[i]);
			rtd_pr_vpq_isr_info("}\n");
		}
	}
	if(((printf_Flag&_BIT5)!=0))
	{
		if(MA_print_count%print_delay==0)
		{
			rtd_pr_vpq_isr_info("Y_Hue_Sat cal, sat_ratio={");
			for(i=0;i<COLOR_AutoSat_HISTOGRAM_LEVEL;i++)
				rtd_pr_vpq_isr_info("%d,", SmartPic_clue->Sat_Main_His_Ratio[i]);
			rtd_pr_vpq_isr_info("}\n");
		}
	}

#endif


}

void scalerVIP_FMV_HMCbin_hist_ratio_Cal(SLR_VIP_TABLE* Share_Memory_VIP_TABLE_Struct, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table)
{
	unsigned int *FMV_hist, *HMC_bin_hist;
	unsigned int *FMV_hist_ratio, *HMC_bin_hist_ratio;
	unsigned char i, GMV, MV_Sel;
	unsigned int sum;
	unsigned int printf_Flag = system_info_structure_table->ISR_Printf_Flag.FMC_HMC_Flag0;
	unsigned int print_delay = (system_info_structure_table->ISR_Printf_Flag.Delay_Time == 0)?(1):(system_info_structure_table->ISR_Printf_Flag.Delay_Time);

	di_im_di_pan_detection_statistic_1_RBUS im_di_pan_detection_statistic_1_reg;
	di_im_di_pan_detection_statistic_2_RBUS im_di_pan_detection_statistic_2_reg;
	di_im_di_pan_detection_statistic_3_RBUS im_di_pan_detection_statistic_3_reg;
	di_im_di_pan_detection_statistic_4_RBUS im_di_pan_detection_statistic_4_reg;
	di_im_di_pan_detection_statistic_5_RBUS im_di_pan_detection_statistic_5_reg;
	di_im_di_pan_detection_statistic_6_RBUS im_di_pan_detection_statistic_6_reg;
	di_im_di_pan_detection_statistic_7_RBUS im_di_pan_detection_statistic_7_reg;
	di_im_di_pan_detection_statistic_8_RBUS im_di_pan_detection_statistic_8_reg;
	di_im_di_pan_detection_statistic_9_RBUS im_di_pan_detection_statistic_9_reg;
	di_im_di_pan_detection_statistic_10_RBUS im_di_pan_detection_statistic_10_reg;
	di_im_di_pan_detection_statistic_11_RBUS im_di_pan_detection_statistic_11_reg;
	di_im_di_pan_detection_statistic_12_RBUS im_di_pan_detection_statistic_12_reg;
	di_im_di_pan_detection_statistic_13_RBUS im_di_pan_detection_statistic_13_reg;
	di_im_di_pan_detection_statistic_14_RBUS im_di_pan_detection_statistic_14_reg;
	di_im_di_pan_detection_statistic_15_RBUS im_di_pan_detection_statistic_15_reg;
	di_im_di_pan_detection_statistic_16_RBUS im_di_pan_detection_statistic_16_reg;
	di_im_di_pan_detection_statistic_17_RBUS im_di_pan_detection_statistic_17_reg;
	di_di_gst_fmvhisa_RBUS di_gst_fmvhisa_reg;
	di_di_gst_fmvhisb_RBUS di_gst_fmvhisb_reg;
	di_di_gst_fmvhisc_RBUS di_gst_fmvhisc_reg;
	di_di_gst_fmvhisd_RBUS di_gst_fmvhisd_reg;
	di_di_gst_fmvhise_RBUS di_gst_fmvhise_reg;
	di_di_gst_fmvhisf_RBUS di_gst_fmvhisf_reg;
	di_di_gst_fmvhisg_RBUS di_gst_fmvhisg_reg;
	di_di_gst_fmvhish_RBUS di_gst_fmvhish_reg;
	di_di_gst_fmvhisi_RBUS di_gst_fmvhisi_reg;
	di_di_gst_fmvhisj_RBUS di_gst_fmvhisj_reg;
	di_di_gst_fmvhisk_RBUS di_gst_fmvhisk_reg;
	di_di_smd_sampling_gmvb_RBUS di_di_smd_sampling_gmvb_reg;
	di_di_smd_sampling_gmvc_RBUS di_di_smd_sampling_gmvc_reg;
	di_di_smd_gmvcoeff3_RBUS di_di_smd_gmvcoeff3_reg;

	im_di_pan_detection_statistic_1_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_1_reg);
	im_di_pan_detection_statistic_2_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_2_reg);
	im_di_pan_detection_statistic_3_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_3_reg);
	im_di_pan_detection_statistic_4_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_4_reg);
	im_di_pan_detection_statistic_5_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_5_reg);
	im_di_pan_detection_statistic_6_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_6_reg);
	im_di_pan_detection_statistic_7_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_7_reg);
	im_di_pan_detection_statistic_8_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_8_reg);
	im_di_pan_detection_statistic_9_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_9_reg);
	im_di_pan_detection_statistic_10_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_10_reg);
	im_di_pan_detection_statistic_11_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_11_reg);
	im_di_pan_detection_statistic_12_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_12_reg);
	im_di_pan_detection_statistic_13_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_13_reg);
	im_di_pan_detection_statistic_14_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_14_reg);
	im_di_pan_detection_statistic_15_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_15_reg);
	im_di_pan_detection_statistic_16_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_16_reg);
	im_di_pan_detection_statistic_17_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_17_reg);
	di_gst_fmvhisa_reg.regValue = rtd_inl(DI_DI_GST_FMVHisA_reg);
	di_gst_fmvhisb_reg.regValue = rtd_inl(DI_DI_GST_FMVHisB_reg);
	di_gst_fmvhisc_reg.regValue = rtd_inl(DI_DI_GST_FMVHisC_reg);
	di_gst_fmvhisd_reg.regValue = rtd_inl(DI_DI_GST_FMVHisD_reg);
	di_gst_fmvhise_reg.regValue = rtd_inl(DI_DI_GST_FMVHisE_reg);
	di_gst_fmvhisf_reg.regValue = rtd_inl(DI_DI_GST_FMVHisF_reg);
	di_gst_fmvhisg_reg.regValue = rtd_inl(DI_DI_GST_FMVHisG_reg);
	di_gst_fmvhish_reg.regValue = rtd_inl(DI_DI_GST_FMVHisH_reg);
	di_gst_fmvhisi_reg.regValue = rtd_inl(DI_DI_GST_FMVHisI_reg);
	di_gst_fmvhisj_reg.regValue = rtd_inl(DI_DI_GST_FMVHisJ_reg);
	di_gst_fmvhisk_reg.regValue = rtd_inl(DI_DI_GST_FMVHisK_reg);

	FMV_hist = SmartPic_clue->FMV_hist;
	FMV_hist_ratio = SmartPic_clue->FMV_hist_Ratio;
	HMC_bin_hist = SmartPic_clue->HMC_bin_hist;
	HMC_bin_hist_ratio = SmartPic_clue->HMC_bin_hist_Ratio;

	// get Global FMV
	di_di_smd_sampling_gmvb_reg.regValue = rtd_inl(DI_DI_SMD_Sampling_GMVb_reg);
	di_di_smd_sampling_gmvc_reg.regValue = rtd_inl(DI_DI_SMD_Sampling_GMVc_reg);
	di_di_smd_gmvcoeff3_reg.regValue = rtd_inl(DI_DI_SMD_GMVCoeff3_reg);
	MV_Sel = di_di_smd_sampling_gmvc_reg.smd_close2gmv_mvselect;
	if(MV_Sel==0)
		GMV = di_di_smd_sampling_gmvb_reg.smd_gmvb_y;
	else if(MV_Sel==1)
		GMV = di_di_smd_sampling_gmvc_reg.smd_gmvc_y;
	else
		GMV = di_di_smd_gmvcoeff3_reg.smd_gmv1_y;

	SmartPic_clue->GMV = GMV;

	HMC_bin_hist[0] = im_di_pan_detection_statistic_9_reg.hmc_r16_pixelsum;
	HMC_bin_hist[1] = im_di_pan_detection_statistic_9_reg.hmc_r15_pixelsum;
	HMC_bin_hist[2] = im_di_pan_detection_statistic_10_reg.hmc_r14_pixelsum;
	HMC_bin_hist[3] = im_di_pan_detection_statistic_10_reg.hmc_r13_pixelsum;
	HMC_bin_hist[4] = im_di_pan_detection_statistic_11_reg.hmc_r12_pixelsum;
	HMC_bin_hist[5] = im_di_pan_detection_statistic_11_reg.hmc_r11_pixelsum;
	HMC_bin_hist[6] = im_di_pan_detection_statistic_12_reg.hmc_r10_pixelsum;
	HMC_bin_hist[7] = im_di_pan_detection_statistic_12_reg.hmc_r9_pixelsum;
	HMC_bin_hist[8] = im_di_pan_detection_statistic_13_reg.hmc_r8_pixelsum;
	HMC_bin_hist[9] = im_di_pan_detection_statistic_1_reg.hmc_r7_pixelsum;
	HMC_bin_hist[10] = im_di_pan_detection_statistic_1_reg.hmc_r6_pixelsum;
	HMC_bin_hist[11] = im_di_pan_detection_statistic_2_reg.hmc_r5_pixelsum;
	HMC_bin_hist[12] = im_di_pan_detection_statistic_2_reg.hmc_r4_pixelsum;
	HMC_bin_hist[13] = im_di_pan_detection_statistic_3_reg.hmc_r3_pixelsum;
	HMC_bin_hist[14] = im_di_pan_detection_statistic_3_reg.hmc_r2_pixelsum;
	HMC_bin_hist[15] = im_di_pan_detection_statistic_4_reg.hmc_r1_pixelsum;
	HMC_bin_hist[16] = im_di_pan_detection_statistic_4_reg.hmc_c0_pixelsum;
	HMC_bin_hist[17] = im_di_pan_detection_statistic_5_reg.hmc_l1_pixelsum;
	HMC_bin_hist[18] = im_di_pan_detection_statistic_5_reg.hmc_l2_pixelsum;
	HMC_bin_hist[19] = im_di_pan_detection_statistic_6_reg.hmc_l3_pixelsum;
	HMC_bin_hist[20] = im_di_pan_detection_statistic_6_reg.hmc_l4_pixelsum;
	HMC_bin_hist[21] = im_di_pan_detection_statistic_7_reg.hmc_l5_pixelsum;
	HMC_bin_hist[22] = im_di_pan_detection_statistic_7_reg.hmc_l6_pixelsum;
	HMC_bin_hist[23] = im_di_pan_detection_statistic_8_reg.hmc_l7_pixelsum;
	HMC_bin_hist[24] = im_di_pan_detection_statistic_13_reg.hmc_l8_pixelsum;
	HMC_bin_hist[25] = im_di_pan_detection_statistic_14_reg.hmc_l9_pixelsum;
	HMC_bin_hist[26] = im_di_pan_detection_statistic_14_reg.hmc_l10_pixelsum;
	HMC_bin_hist[27] = im_di_pan_detection_statistic_15_reg.hmc_l11_pixelsum;
	HMC_bin_hist[28] = im_di_pan_detection_statistic_15_reg.hmc_l12_pixelsum;
	HMC_bin_hist[29] = im_di_pan_detection_statistic_16_reg.hmc_l13_pixelsum;
	HMC_bin_hist[30] = im_di_pan_detection_statistic_16_reg.hmc_l14_pixelsum;
	HMC_bin_hist[31] = im_di_pan_detection_statistic_17_reg.hmc_l15_pixelsum;
	HMC_bin_hist[32] = im_di_pan_detection_statistic_17_reg.hmc_l16_pixelsum;
	FMV_hist[0] =  di_gst_fmvhisa_reg.gst_fmv_his00;
	FMV_hist[1] =  di_gst_fmvhisa_reg.gst_fmv_his01;
	FMV_hist[2] =  di_gst_fmvhisb_reg.gst_fmv_his02;
	FMV_hist[3] =  di_gst_fmvhisb_reg.gst_fmv_his03;
	FMV_hist[4] =  di_gst_fmvhisc_reg.gst_fmv_his04;
	FMV_hist[5] =  di_gst_fmvhisc_reg.gst_fmv_his05;
	FMV_hist[6] =  di_gst_fmvhisd_reg.gst_fmv_his06;
	FMV_hist[7] =  di_gst_fmvhisd_reg.gst_fmv_his07;
	FMV_hist[8] =  di_gst_fmvhise_reg.gst_fmv_his08;
	FMV_hist[9] =  di_gst_fmvhise_reg.gst_fmv_his09;
	FMV_hist[10] = di_gst_fmvhisf_reg.gst_fmv_his10;
	FMV_hist[11] = di_gst_fmvhisg_reg.gst_fmv_his11;
	FMV_hist[12] = di_gst_fmvhisg_reg.gst_fmv_his12;
	FMV_hist[13] = di_gst_fmvhish_reg.gst_fmv_his13;
	FMV_hist[14] = di_gst_fmvhish_reg.gst_fmv_his14;
	FMV_hist[15] = di_gst_fmvhisi_reg.gst_fmv_his15;
	FMV_hist[16] = di_gst_fmvhisi_reg.gst_fmv_his16;
	FMV_hist[17] = di_gst_fmvhisj_reg.gst_fmv_his17;
	FMV_hist[18] = di_gst_fmvhisj_reg.gst_fmv_his18;
	FMV_hist[19] = di_gst_fmvhisk_reg.gst_fmv_his19;
	FMV_hist[20] = di_gst_fmvhisk_reg.gst_fmv_his20;

	sum = 1;
	for(i=0;i<FMV_Hist_Bin_Num;i++)
		sum = sum + FMV_hist[i];
	for(i=0;i<FMV_Hist_Bin_Num;i++)
		FMV_hist_ratio[i] = FMV_hist[i]*1000/sum;
	sum = 1;
	for(i=0;i<HMC_Hist_Bin_Num;i++)
		sum = sum + HMC_bin_hist[i];
	for(i=0;i<HMC_Hist_Bin_Num;i++)
		HMC_bin_hist_ratio[i] = HMC_bin_hist[i]*1000/sum;

#if 1
	//for debug.
	if(((printf_Flag&_BIT0)!=0))
	{
		if(MA_print_count%print_delay==0)
		{
			rtd_pr_vpq_isr_info("FMV HMC cal, FMV={");
			for(i=0;i<FMV_Hist_Bin_Num;i++)
				rtd_pr_vpq_isr_info("%d,", SmartPic_clue->FMV_hist_Ratio[i]);
			rtd_pr_vpq_isr_info("}\n");
		}
	}
	if(((printf_Flag&_BIT1)!=0))
	{
		if(MA_print_count%print_delay==0)
		{
			rtd_pr_vpq_isr_info("FMV HMC cal, HMC={");
			for(i=0;i<FMV_Hist_Bin_Num;i++)
				rtd_pr_vpq_isr_info("%d,", SmartPic_clue->HMC_bin_hist_Ratio[i]);
			rtd_pr_vpq_isr_info("}\n");
		}
	}

#endif

}

//Main function END

#if 0
unsigned int Noise_Statistics( unsigned int width, unsigned int height)
{
#if 0
        static unsigned char fcounter = 0;
        unsigned int MAD_Y_th1, MAD_Y_th2;
        unsigned int MAD_C_th1, MAD_C_th2;
        unsigned int MAD_Y_Count, MAD_U_Count, MAD_V_Count;
        static unsigned int MAD_Y_tmp_sum = 0;
        static unsigned int MAD_U_tmp_sum = 0;
        static unsigned int MAD_V_tmp_sum= 0;
        static unsigned int MAD_Y_min = 0xFFFFFFFF;
        static unsigned int MAD_Y_max = 0;
        static unsigned int MAD_Y_2ndmax = 0;
        static unsigned int MAD_U_min = 0xFFFFFFFF;
        static unsigned int MAD_U_max = 0;
        static unsigned int MAD_U_2ndmax = 0;
        static unsigned int MAD_V_min = 0xFFFFFFFF;
        static unsigned int MAD_V_max = 0;
        static unsigned int MAD_V_2ndmax = 0;
        unsigned int et_y_count;
        //unsigned int et_y_sum, et_u_sum, et_v_sum;
        static unsigned int et_y_count_tmp_sum = 0;
        unsigned int MAD_Y_avg, MAD_U_avg, MAD_V_avg, et_y_count_avg;
		static unsigned int ret_value = 0;
//     static int RTNR_offset;

        di_im_di_rtnr_mad_y_th_RBUS im_di_rtnr_mad_y_th_reg;
        di_im_di_rtnr_mad_c_th_RBUS im_di_rtnr_mad_c_th_reg;
        di_im_di_rtnr_mad_y_counter_RBUS im_di_rtnr_mad_y_counter_reg;
        di_im_di_rtnr_mad_u_counter_RBUS im_di_rtnr_mad_u_counter_reg;
        di_im_di_rtnr_mad_v_counter_RBUS im_di_rtnr_mad_v_counter_reg;
        di_im_di_rtnr_et_count_y_RBUS im_di_et_count_y_reg;
        //di_im_di_rtnr_et_sum_y_RBUS im_di_et_sum_y_reg;
        //di_im_di_rtnr_et_sum_u_RBUS im_di_et_sum_u_reg;
        //di_im_di_rtnr_et_sum_v_RBUS im_di_et_sum_v_reg;

        MAD_Y_th1 = 0x020;
        MAD_Y_th2 = 0x7FF;
        //MAD_Y_th1 = 0x0;
        //MAD_Y_th2 = 0xA;
        MAD_C_th1 = 0x010;
        MAD_C_th2 = 0x7FF;
        /*if( fcounter < 8 ) {
                MAD_Y_th1 = 0x100 * fcounter;
                MAD_Y_th2 = 0x100 * (fcounter+1) -1;
        }*/

        im_di_rtnr_mad_y_th_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_TH_reg);
        im_di_rtnr_mad_c_th_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_C_TH_reg);
        //MAD_Y_th1 = im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1;
        //MAD_Y_th2 = im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2;
        //MAD_C_th1 = im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th1;
        //MAD_C_th2 = im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th2;
        im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = MAD_Y_th1;
        im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2 = MAD_Y_th2;
        im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th1 = MAD_C_th1;
        im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th2 = MAD_C_th2;
        rtd_outl(DI_IM_DI_RTNR_MAD_Y_TH_reg, im_di_rtnr_mad_y_th_reg.regValue);
        rtd_outl(DI_IM_DI_RTNR_MAD_C_TH_reg, im_di_rtnr_mad_c_th_reg.regValue);

        im_di_rtnr_mad_y_counter_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_COUNTER_reg);
        im_di_rtnr_mad_u_counter_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_U_COUNTER_reg);
        im_di_rtnr_mad_v_counter_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_V_COUNTER_reg);
        MAD_Y_Count = im_di_rtnr_mad_y_counter_reg.cp_temporal_mad_countery;
        MAD_U_Count = im_di_rtnr_mad_u_counter_reg.cp_temporal_mad_counteru;
        MAD_V_Count = im_di_rtnr_mad_v_counter_reg.cp_temporal_mad_counterv;

        im_di_et_count_y_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_COUNT_Y_reg);
        //im_di_et_sum_y_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_SUM_Y_reg);
        //im_di_et_sum_u_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_SUM_U_reg);
        //im_di_et_sum_v_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_SUM_V_reg);
        et_y_count = im_di_et_count_y_reg.cp_temporal_same_et_cnt_y;
        //et_y_sum = im_di_et_sum_y_reg.cp_temporal_same_et_sad_sum_y;
        //et_u_sum = im_di_et_sum_u_reg.cp_temporal_same_et_sad_sum_u;
        //et_v_sum = im_di_et_sum_v_reg.cp_temporal_same_et_sad_sum_v;

        MAD_Y_tmp_sum += MAD_Y_Count;
        MAD_U_tmp_sum += MAD_U_Count;
        MAD_V_tmp_sum += MAD_V_Count;
        et_y_count_tmp_sum += et_y_count;
        if(et_y_count == 0 ) et_y_count += 1;

        if( MAD_Y_Count < MAD_Y_min ) MAD_Y_min = MAD_Y_Count;
        if( MAD_Y_Count > MAD_Y_2ndmax ) {
                if( MAD_Y_Count > MAD_Y_max ) {
                        MAD_Y_2ndmax = MAD_Y_max; // drop max to avoid scene change
                        MAD_Y_max= MAD_Y_Count;
                }
                else MAD_Y_2ndmax = MAD_Y_Count;
        }
        if( MAD_U_Count < MAD_U_min ) MAD_U_min = MAD_U_Count;
        if( MAD_U_Count > MAD_U_2ndmax ) {
                if( MAD_U_Count > MAD_U_max ) {
                        MAD_U_2ndmax = MAD_U_max; // drop max to avoid scene change
                        MAD_U_max= MAD_U_Count;
                }
                else MAD_U_2ndmax = MAD_U_Count;
        }
        if( MAD_V_Count < MAD_V_min ) MAD_V_min = MAD_V_Count;
        if( MAD_V_Count > MAD_V_2ndmax ) {
                if( MAD_V_Count > MAD_V_max ) {
                        MAD_V_2ndmax = MAD_V_max; // drop max to avoid scene change
                        MAD_V_max= MAD_V_Count;
                }
                else MAD_V_2ndmax = MAD_V_Count;
        }

        /*if( fcounter <= 8 ) {
                ROSPrintf("ooo--- Noise Statistics: ---ooo\n");
                ROSPrintf("ooo--- Count of Y MAD between %d and %d is : %d ---ooo\n", MAD_Y_th1, MAD_Y_th2, MAD_Y_Count);
                ROSPrintf("ooo--- Avg count of U&V MAD between %d and %d is : %d&%d ---ooo\n", MAD_C_th1, MAD_C_th2, MAD_U_Count, MAD_V_Count);
                ROSPrintf("ooo--- Avg count of same edge type: %d ---ooo\n", et_y_count);
                ROSPrintf("ooo--- Avg same edge type sum/count for Y = %d, U = %d, V = %d ---ooo\n", et_y_sum/et_y_count, et_u_sum/et_y_count, et_v_sum/et_y_count);
        }*/
    fcounter++;
    if(fcounter == 5) {
            MAD_Y_avg = MAD_Y_tmp_sum/5;
            MAD_U_avg = MAD_U_tmp_sum/5;
            MAD_V_avg = MAD_V_tmp_sum/5;
            et_y_count_avg = et_y_count_tmp_sum/5;
            /*
            RTNR_offset = 0;
            if( MAD_Y_avg < 1000 )
                    RTNR_offset -= 2;
            else if( MAD_Y_avg < 5000 )
                    RTNR_offset -= 1;
            if( MAD_Y_avg > 500000 )
                    RTNR_offset += 3;
            else if( MAD_Y_avg > 100000 )
                    RTNR_offset += 2;
            else if( MAD_Y_avg > 30000 )
                    RTNR_offset += 1;

            //if( MAD_U_avg > 300000 ) // judged as motion
            //     RTNR_offset -= 2;
            //else if( MAD_U_avg > 150000 )
            //     RTNR_offset -= 1;
            //else if( MAD_U_avg > 10000 && MAD_U_avg < 80000 ) // judged as noise
            if( MAD_U_avg > 10000 ) // judged as noise
                    RTNR_offset += 1;

            //if( MAD_V_avg > 300000 ) // judged as motion
            //     RTNR_offset -= 2;
            //else if( MAD_V_avg > 150000 )
            //     RTNR_offset -= 1;
            //else if( MAD_V_avg > 10000 && MAD_V_avg < 80000 ) // judged as noise
            if( MAD_V_avg > 10000 ) // judged as noise
                    RTNR_offset += 1;

            if( et_y_count_avg < 2000 )
                    RTNR_offset += 4;
            else if( et_y_count_avg < 3000 )
                    RTNR_offset += 3;
            else if( et_y_count_avg < 5000 )
                    RTNR_offset += 2;
            else if( et_y_count_avg < 10000 )
                    RTNR_offset += 1;
            if( et_y_count_avg > 800000 )
                    RTNR_offset -= 3;
            else if( et_y_count_avg > 500000 )
                    RTNR_offset -= 2;
            else if( et_y_count_avg > 300000 )
                    RTNR_offset -= 1;

            RTNR_offset /= 2;
            */
            ///*
            //ROSPrintf("ooo--- Noise Statistics: ---ooo\n");
            //ROSPrintf("ooo--- Range of Y MAD between %d and %d is : %d~%d, avg = %d ---ooo\n", MAD_Y_th1, MAD_Y_th2, MAD_Y_min, MAD_Y_2ndmax, MAD_Y_avg);
            //ROSPrintf("ooo--- Range of U&V MAD between %d and %d is : %d~%d & %d~%d ---ooo\n", MAD_C_th1, MAD_C_th2, MAD_U_min, MAD_U_2ndmax, MAD_V_min, MAD_V_2ndmax);
            //ROSPrintf("ooo--- Avg count of same edge type: %d ---ooo\n", et_y_count_avg);
            //ROSPrintf("ooo--- Avg same edge type sum/count for Y = %d, U = %d, V = %d ---ooo\n", et_y_ratio_tmp_sum/50, et_u_ratio_tmp_sum/50, et_v_ratio_tmp_sum/50);
            //ROSPrintf("ooo--- RTNR offset = %d, RTNR level = %d, SPNR level = %d ---ooo\n", RTNR_offset, RTNR_LEVEL_SDHD, SPNR_LEVEL_SDHD);
            //ROSPrintf("ooo--- RTNR offset = %d ---ooo\n", RTNR_offset);
            //*/
        #if 0
           	static unsigned int counter=0;
			if(counter%5==0)
				ROSPrintf("=%d,\n", MAD_Y_avg);
		#endif
            fcounter = 0;
            MAD_Y_tmp_sum = MAD_U_tmp_sum = MAD_V_tmp_sum = 0;
            et_y_count_tmp_sum =0;
            MAD_Y_min = MAD_U_min = MAD_V_min = 0xFFFFFFFF;
            MAD_Y_max = MAD_U_max = MAD_V_max = 0;
            MAD_Y_2ndmax = MAD_U_2ndmax = MAD_V_2ndmax = 0;
			ret_value = MAD_Y_avg;
    }
	return ret_value;
#else
	static unsigned int ret_value = 0;
	static unsigned char fcounter = 0;

	// MAD counter threshold setting
	unsigned int MAD_Y_Count;
	static unsigned int MAD_Y_tmp_sum = 0;
	unsigned int MAD_Y_avg;
	di_im_di_rtnr_mad_y_th_RBUS im_di_rtnr_mad_y_th_reg;
	di_im_di_rtnr_mad_y_counter_RBUS im_di_rtnr_mad_y_counter_reg;
	im_di_rtnr_mad_y_th_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_TH_reg);
	im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = 0x020;
	im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2 = 0x7FF;
#ifndef LG_Demo_flag
	rtd_outl(DI_IM_DI_RTNR_MAD_Y_TH_reg, im_di_rtnr_mad_y_th_reg.regValue);
#endif

	//same edge conter setting
	unsigned int et_y_count;
	unsigned int et_y_sum, et_y_ave;
	di_im_di_rtnr_et_count_y_RBUS im_di_et_count_y_reg;
	di_im_di_rtnr_et_sum_y_RBUS im_di_et_sum_y_reg;
	di_im_di_rtnr_hmcnr_RBUS di_im_di_rtnr_hmcnr_RBUS_reg;
	di_im_di_rtnr_hmcnr_RBUS_reg.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_reg);
	di_im_di_rtnr_hmcnr_RBUS_reg.cp_temporal_hmcnr_mad_window = 2;
	di_im_di_rtnr_hmcnr_RBUS_reg.cp_temporal_hmcnr_search_range = 0;
	rtd_outl(DI_IM_DI_RTNR_HMCNR_reg, di_im_di_rtnr_hmcnr_RBUS_reg.regValue);

	//MAD counter calculate
	im_di_rtnr_mad_y_counter_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_COUNTER_reg);
	MAD_Y_Count = im_di_rtnr_mad_y_counter_reg.cp_temporal_mad_countery;
	MAD_Y_tmp_sum += MAD_Y_Count;
	fcounter++;
	if(fcounter == 5)
	{
		MAD_Y_avg = MAD_Y_tmp_sum/5;
		fcounter = 0;
		MAD_Y_tmp_sum = 0;
		ret_value = MAD_Y_avg;
	}

	//same edge calculate
	im_di_et_count_y_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_COUNT_Y_reg);
	im_di_et_sum_y_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_SUM_Y_reg);
	et_y_count = im_di_et_count_y_reg.cp_temporal_same_et_cnt_y;
	et_y_sum = im_di_et_sum_y_reg.cp_temporal_same_et_sad_sum_y;
	if(et_y_count==0)
		et_y_count=1;
	et_y_ave = (et_y_sum/et_y_count);

#if 0
	if(MA_print_count%50==0)
		ROSPrintf("n=%d,e=%d,\n",ret_value, et_y_ave);
#endif
	//reuturn
	return ret_value;

#endif
}
#endif

//Elsie 20131212, original by jimmy.lin
unsigned char scalerVIP_Fade_Detection (void)
{
	//static unsigned int ProfSum_pre_field = 0;
	static unsigned int ProfSum_pre_frame = 0;
	static unsigned char isEvenField=0;
	static char fadeDirection = 0;
	static short fadeCount = 0;
	static short stillCount = 0;

	unsigned char isProgressive = 0;
	unsigned int ProfSum_curr_frame = 0;
	int frameDiff = 0;
	unsigned char isValidFrame=0;
	unsigned short diff_still_th = 0;
	unsigned short diff_Th1 = 0, diff_Th2 = 0;
	unsigned short still_Th = 10;

	unsigned short IWid;
	unsigned short ILen;
	unsigned char profsum_shift = 0;

	vgip_vgip_chn1_act_hsta_width_RBUS vgip_chn1_act_hsta_width_reg;
	vgip_vgip_chn1_act_vsta_length_RBUS vgip_chn1_act_vsta_length_reg;
	profile_ich1_hist_profile_ctrl_RBUS ich1_hist_profile_ctrl_reg;

	vgip_chn1_act_hsta_width_reg.regValue  = scaler_rtd_inl(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);
	vgip_chn1_act_vsta_length_reg.regValue = scaler_rtd_inl(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);

	isProgressive = ( SmartPic_clue->SCALER_DISP_INFO.disp_status & _BIT8 ) == 0;
/*
        if( MA_print_count% 180==0 )
	{
		ROSPrintf("[ScalerVIP] isProgressive = %d\n", isProgressive);
	}
*/
	//IWid = scaler_disp_info[_CHANNEL1].DispWid;
	//ILen = scaler_disp_info[_CHANNEL1].DispLen;
	IWid = vgip_chn1_act_hsta_width_reg.ch1_ih_act_wid;
	ILen = vgip_chn1_act_vsta_length_reg.ch1_iv_act_len;

	ich1_hist_profile_ctrl_reg.regValue = rtd_inl(PROFILE_ICH1_Hist_Profile_CTRL_reg);

	switch( ich1_hist_profile_ctrl_reg.ch1_pf_hnsr )
	{
		case 1:
			profsum_shift = 8;
			break;
		case 2:
			profsum_shift = 9;
			break;
		case 3:
			profsum_shift = 10;
			break;
		default: // shouldn't come here normally
			profsum_shift = 0;
			break;
	}

	// Progressive: current frame vs. previous frame
	if( isProgressive )
	{
		ProfSum_curr_frame = ProfSum;
		frameDiff = ProfSum_curr_frame - ProfSum_pre_frame;
		isValidFrame = 1;
	}

	// Interlaced: current even field vs. previous even field
	else if(isEvenField == 1 )
	{
		ProfSum_curr_frame = ProfSum;
		frameDiff = ProfSum_curr_frame - ProfSum_pre_frame;
		isValidFrame = 1;
		ILen *= 2;
	}
	else
	{
		isValidFrame = 0;
	}

	diff_still_th =  ( ( IWid * ILen ) >> profsum_shift ) >> 4; // avg. Y change 1/16
	diff_Th1 = ( ( IWid * ILen ) >> profsum_shift ) >> 2; // avg. Y change 1/4
	//diff_Th1 = 0;
	diff_Th2 = ( ( IWid * ILen ) >> profsum_shift ) << 3; // avg. Y change 8

	if( isValidFrame ) {
		if( abs_value(frameDiff) > diff_still_th ) {
			if( frameDiff * fadeDirection < 0 ) { // direction changed
				fadeCount = 0;
				fadeDirection = (frameDiff > 0) ? 1 : -1;
			}
			else if ( abs_value(frameDiff) >= diff_Th1 && abs_value(frameDiff) <= diff_Th2 ) {
				fadeCount++;
				fadeDirection = (frameDiff > 0) ? 1 : -1;
			}
			stillCount = 0;
		}
		else { // change 0~very small, judged as still
			stillCount++;
			if( stillCount >= still_Th ) { // still for more than set number of frames
				fadeCount = 0;
				fadeDirection = 0;
				stillCount = 0;
			}
		}

		ProfSum_pre_frame = ProfSum_curr_frame;
	}

	isEvenField = (isEvenField==0) ? 1 : 0;
#if 0
	if( MA_print_count% 180==0 )
	{
		ROSPrintf("[ScalerVIP] ProfSum = %d, ProfSum_pre = %d\n", ProfSum, ProfSum_pre_frame);
		ROSPrintf("[ScalerVIP] Frame diff: %d, Fade direction: %d, stillCount: %d\n", frameDiff, fadeDirection, stillCount);
	}
#endif
	if( fadeCount >= 4 ) return true;
	else return false;

}

// 20130823 jimmy.lin for noise statistics
MOTION_NOISE_TYPE scalerVIP_noise_estimation_MAD_et( unsigned int width, unsigned int height )
{
	static unsigned char fcounter = 0;
	unsigned int MAD_Y_th1, MAD_Y_th2;
	unsigned int MAD_C_th1, MAD_C_th2;
	unsigned int MAD_Y_Count, MAD_U_Count, MAD_V_Count;
	static unsigned int MAD_Y_tmp_sum = 0;
	static unsigned int MAD_U_tmp_sum = 0;
	static unsigned int MAD_V_tmp_sum= 0;
	static unsigned int MAD_Y_min = 0xFFFFFFFF;
	static unsigned int MAD_Y_max = 0;
	static unsigned int MAD_Y_2ndmax = 0;
	static unsigned int MAD_U_min = 0xFFFFFFFF;
	static unsigned int MAD_U_max = 0;
	static unsigned int MAD_U_2ndmax = 0;
	static unsigned int MAD_V_min = 0xFFFFFFFF;
	static unsigned int MAD_V_max = 0;
	static unsigned int MAD_V_2ndmax = 0;
	unsigned int et_y_count;
	//unsigned int et_y_sum, et_u_sum, et_v_sum;
	static unsigned int et_y_count_tmp_sum = 0;
	unsigned int MAD_Y_avg, MAD_U_avg, MAD_V_avg, et_y_count_avg;
	static MOTION_NOISE_TYPE isMotion = 0; // 0: still, 1: motion, 2: noise

	di_im_di_rtnr_mad_y_th_RBUS im_di_rtnr_mad_y_th_reg;
	di_im_di_rtnr_mad_c_th_RBUS im_di_rtnr_mad_c_th_reg;
	di_im_di_rtnr_mad_y_counter_RBUS im_di_rtnr_mad_y_counter_reg;
	di_im_di_rtnr_mad_u_counter_RBUS im_di_rtnr_mad_u_counter_reg;
	di_im_di_rtnr_mad_v_counter_RBUS im_di_rtnr_mad_v_counter_reg;
	di_im_di_rtnr_et_count_y_RBUS im_di_et_count_y_reg;
	di_im_di_rtnr_level_bound_RBUS im_di_rtnr_level_bound_reg;
	//di_im_di_rtnr_et_sum_y_RBUS im_di_et_sum_y_reg;
	//di_im_di_rtnr_et_sum_u_RBUS im_di_et_sum_u_reg;
	//di_im_di_rtnr_et_sum_v_RBUS im_di_et_sum_v_reg;

	MAD_Y_th1 = 0x017; //23
	MAD_Y_th2 = 0x7FF;
	//MAD_Y_th1 = 0x0;
	//MAD_Y_th2 = 0xA;
	MAD_C_th1 = 0x010;
	MAD_C_th2 = 0x7FF;
	/*if( fcounter < 8 ) {
		MAD_Y_th1 = 0x100 * fcounter;
		MAD_Y_th2 = 0x100 * (fcounter+1) -1;
	}*/

	im_di_rtnr_mad_y_th_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_TH_reg);
	im_di_rtnr_mad_c_th_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_C_TH_reg);
	im_di_rtnr_level_bound_reg.regValue = rtd_inl(DI_IM_DI_RTNR_LEVEL_BOUND_reg);
	//MAD_Y_th1 = im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1;
	//MAD_Y_th2 = im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2;
	//MAD_C_th1 = im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th1;
	//MAD_C_th2 = im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th2;
	im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = MAD_Y_th1;
	im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2 = MAD_Y_th2;
	im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th1 = MAD_C_th1;
	im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th2 = MAD_C_th2;
	im_di_rtnr_level_bound_reg.cp_temporal_nl_low_bnd_y = 0;
	im_di_rtnr_level_bound_reg.cp_temporal_nl_up_bnd_y = 0xFF;
	rtd_outl(DI_IM_DI_RTNR_MAD_Y_TH_reg, im_di_rtnr_mad_y_th_reg.regValue);
	rtd_outl(DI_IM_DI_RTNR_MAD_C_TH_reg, im_di_rtnr_mad_c_th_reg.regValue);
	rtd_outl(DI_IM_DI_RTNR_LEVEL_BOUND_reg, im_di_rtnr_level_bound_reg.regValue);

	im_di_rtnr_mad_y_counter_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_COUNTER_reg);
	im_di_rtnr_mad_u_counter_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_U_COUNTER_reg);
	im_di_rtnr_mad_v_counter_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_V_COUNTER_reg);
	MAD_Y_Count = im_di_rtnr_mad_y_counter_reg.cp_temporal_mad_countery;
	MAD_U_Count = im_di_rtnr_mad_u_counter_reg.cp_temporal_mad_counteru;
	MAD_V_Count = im_di_rtnr_mad_v_counter_reg.cp_temporal_mad_counterv;

	im_di_et_count_y_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_COUNT_Y_reg);
	//im_di_et_sum_y_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_SUM_Y_reg);
	//im_di_et_sum_u_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_SUM_U_reg);
	//im_di_et_sum_v_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_SUM_V_reg);
	et_y_count = im_di_et_count_y_reg.cp_temporal_same_et_cnt_y;
	//et_y_sum = im_di_et_sum_y_reg.cp_temporal_same_et_sad_sum_y;
	//et_u_sum = im_di_et_sum_u_reg.cp_temporal_same_et_sad_sum_u;
	//et_v_sum = im_di_et_sum_v_reg.cp_temporal_same_et_sad_sum_v;

	MAD_Y_tmp_sum += MAD_Y_Count;
	MAD_U_tmp_sum += MAD_U_Count;
	MAD_V_tmp_sum += MAD_V_Count;
	et_y_count_tmp_sum += et_y_count;
	if(et_y_count == 0 ) et_y_count += 1;

	if( MAD_Y_Count < MAD_Y_min ) MAD_Y_min = MAD_Y_Count;
	if( MAD_Y_Count > MAD_Y_2ndmax ) {
		if( MAD_Y_Count > MAD_Y_max ) {
			MAD_Y_2ndmax = MAD_Y_max; // drop max to avoid scene change
			MAD_Y_max= MAD_Y_Count;
		}
		else MAD_Y_2ndmax = MAD_Y_Count;
	}
	if( MAD_U_Count < MAD_U_min ) MAD_U_min = MAD_U_Count;
	if( MAD_U_Count > MAD_U_2ndmax ) {
		if( MAD_U_Count > MAD_U_max ) {
			MAD_U_2ndmax = MAD_U_max; // drop max to avoid scene change
			MAD_U_max= MAD_U_Count;
		}
		else MAD_U_2ndmax = MAD_U_Count;
	}
	if( MAD_V_Count < MAD_V_min ) MAD_V_min = MAD_V_Count;
	if( MAD_V_Count > MAD_V_2ndmax ) {
		if( MAD_V_Count > MAD_V_max ) {
			MAD_V_2ndmax = MAD_V_max; // drop max to avoid scene change
			MAD_V_max= MAD_V_Count;
		}
		else MAD_V_2ndmax = MAD_V_Count;
	}

	fcounter++;
	if(fcounter % 5 == 0) {
		MAD_Y_avg = MAD_Y_tmp_sum/5;
		MAD_U_avg = MAD_U_tmp_sum/5;
		MAD_V_avg = MAD_V_tmp_sum/5;
		et_y_count_avg = et_y_count_tmp_sum/5;

		isMotion = MOTION_NOISE_TYPE_STILL;
		if( MAD_Y_avg > ((width*height) >> 6) ) // total area/64
		{
			isMotion = MOTION_NOISE_TYPE_MOTION; // judge as motion
			if( MAD_Y_avg > ((width*height) >> 4) ) // total area / 16
				isMotion = MOTION_NOISE_TYPE_LARGE_MOTION;
			else if( et_y_count_avg < ((width*height) >> 4) ) // total area/16
				isMotion = MOTION_NOISE_TYPE_NOISE; // judge as noise
		}
#if 1
		if( (fcounter%debug_print_delay_time == 0) &&(Start_Print.Rtnr_1==TRUE) )
		{

			ROSPrintf("ooo--- Noise Statistics: ---ooo\n");
			ROSPrintf("ooo--- Range of Y MAD between %d and %d is : %d~%d, avg = %d ---ooo \n", MAD_Y_th1, MAD_Y_th2, MAD_Y_min, MAD_Y_2ndmax, MAD_Y_avg);
			ROSPrintf("ooo--- Range of U&V MAD between %d and %d is : %d~%d & %d~%d ---ooo \n", MAD_C_th1, MAD_C_th2, MAD_U_min, MAD_U_2ndmax, MAD_V_min, MAD_V_2ndmax);
			ROSPrintf("ooo--- Avg count of same edge type: %d ---ooo\n", et_y_count_avg);

			switch( isMotion )
			{
				case MOTION_NOISE_TYPE_STILL:
					ROSPrintf("ooo--- MOTION_NOISE_TYPE_STILL ---ooo\n");
					break;
				case MOTION_NOISE_TYPE_MOTION:
					ROSPrintf("ooo--- MOTION_NOISE_TYPE_MOTION ---ooo\n");
					break;
				case MOTION_NOISE_TYPE_LARGE_MOTION:
					ROSPrintf("ooo--- MOTION_NOISE_TYPE_LARGE_MOTION ---ooo\n");
					break;
				case MOTION_NOISE_TYPE_NOISE:
					ROSPrintf("ooo--- MOTION_NOISE_TYPE_NOISE ---ooo\n");
					break;
			}
			fcounter = 0;
		}
#endif
		MAD_Y_tmp_sum = MAD_U_tmp_sum = MAD_V_tmp_sum = 0;
		et_y_count_tmp_sum =0;
		MAD_Y_min = MAD_U_min = MAD_V_min = 0xFFFFFFFF;
		MAD_Y_max = MAD_U_max = MAD_V_max = 0;
		MAD_Y_2ndmax = MAD_U_2ndmax = MAD_V_2ndmax = 0;
	}

	return isMotion;

}


void scalerVIP_RTNR_correctionbit_period(void)
{
	static unsigned short framecnt = 0;
	static unsigned short pseudo_ran = 0;
	unsigned int pseudo_ran_var = 566927;
	unsigned char nRan_Range=5;
	unsigned char correctionbit_period = 0;

	if((SmartPic_clue->MAD_motion_status) == MOTION_NOISE_TYPE_MOTION) // motion
		correctionbit_period = 1;
	else if((SmartPic_clue->MAD_motion_status) == MOTION_NOISE_TYPE_NOISE) // noise or very large motion
		correctionbit_period = 1;
	else	// still
	{
		//correctionbit_period = pseudo_ran;
		correctionbit_period = 1;
	}

	if( (correctionbit_period != 0) && (framecnt % correctionbit_period == 0) ) {
		IoReg_SetBits(DI_IM_DI_RTNR_CONTROL_reg, _BIT3);
		if( pseudo_ran_var != 0 )
			pseudo_ran = (pseudo_ran*3612+5701) % pseudo_ran_var;
		else
			pseudo_ran = 0;

		pseudo_ran = pseudo_ran % nRan_Range;
		framecnt = 0;
	}

	/*
	if( MA_print_count% 180==0 ) {
		ROSPrintf("[AutoRTNR] MAD_motion_status = %d, di_motion_status = %d, correctionbit_period = %d\n", (SmartPic_clue->MAD_motion_status), (SmartPic_clue->di_motion_status), correctionbit_period );
	}
	*/
	framecnt++;
}


// 20131209 jimmy.lin
MOTION_NOISE_TYPE scalerVIP_Profile_Block_Motion(_clues* SmartPic_clue)
{
	unsigned char ii,jj;
	unsigned char Profile_Block_Num = 16;//set profile h&V block number
	unsigned short *H_Block = &(SmartPic_clue->Profile_Cal_INFO.H_Block_Y[0]);
	unsigned short *V_Block = &(SmartPic_clue->Profile_Cal_INFO.V_Block_Y[0]);

	static unsigned char isEvenField = 0;

	static unsigned short pre_H_Block[16] = {0};
	static unsigned short pre_V_Block[16] = {0};
	static int diff_H[10][16] = {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};
	static int diff_V[10][16] = {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};

	unsigned int sum_of_difference[10];
	//unsigned short motion_trend = 0;
	unsigned short motion_th = 1;
	unsigned char maxDirCnt = 0;
	unsigned char sameDirCnt = 0;
	unsigned char diffDirCnt = 0;
	char currDir;
	char sign; // 1: +, -1: -, 0: zero or under threshold
	unsigned short maxDiff, totalDiff;

	MOTION_NOISE_TYPE H_Block_Motion[16];
	MOTION_NOISE_TYPE V_Block_Motion[16];

	static MOTION_NOISE_TYPE isMotion = 0;

	for( jj=0; jj<10; jj++ ) {
		sum_of_difference[jj] = 0;
	}

	for( ii=0; ii<Profile_Block_Num; ii++ )
	{
		H_Block_Motion[ii] = MOTION_NOISE_TYPE_STILL;
		V_Block_Motion[ii] = MOTION_NOISE_TYPE_STILL;
	}

	if( isEvenField != 0 ) {

		block_motion_count_H = 0;
		block_motion_count_V = 0;
		block_noise_count_H = 0;
		block_noise_count_V = 0;

		//motion detection
		for (ii=0;ii<Profile_Block_Num;ii++)
		{
			for(jj=9; jj>0; jj--) {
				diff_H[jj][ii] = diff_H[jj-1][ii];
				diff_V[jj][ii] = diff_V[jj-1][ii];
				sum_of_difference[jj] += abs_value(diff_H[jj][ii]) + abs_value(diff_V[jj][ii]);
			}
			diff_H[0][ii] = H_Block[ii] - pre_H_Block[ii];
			diff_V[0][ii] = V_Block[ii] - pre_V_Block[ii];
			sum_of_difference[0] += abs_value(diff_H[0][ii]) + abs_value(diff_V[0][ii]);

		}

		//save profile to buffer:pre_H_Block, pre_V_Block
		for (ii=0;ii<Profile_Block_Num;ii++)
		{
			pre_H_Block[ii] = H_Block[ii];
			pre_V_Block[ii] = V_Block[ii];
		}

		for( ii=0; ii<Profile_Block_Num; ii++ ) {

			// H_Block motion determination
			maxDirCnt = 0;
			sameDirCnt = 0;
			diffDirCnt = 0;
			maxDiff = 0;
			totalDiff = 0;
			if( abs_value(diff_H[0][ii]) < motion_th )
				currDir = 0;
			else
				currDir = (diff_H[0][ii]>=0 ? 1 : -1);

			for( jj=1; jj<5; jj++ ) {
				if( abs_value(diff_H[jj][ii]) < motion_th )
					sign = 0;
				else
					sign = (diff_H[jj][ii]>=0 ? 1 : -1);

				if( sign == currDir && currDir!= 0 ) {
					sameDirCnt++;
					if( sameDirCnt > maxDirCnt )
						maxDirCnt = sameDirCnt;
					totalDiff += abs_value(diff_H[jj][ii]);
					if( totalDiff > maxDiff )
						maxDiff = totalDiff;
				}
				else if( sign != 0 && currDir != 0 ) { // different direction
					sameDirCnt = 0;
					diffDirCnt++;
					currDir = sign;
					totalDiff = 0;
				}
				else {
					totalDiff = abs_value(totalDiff + diff_H[jj][ii]);
					if( totalDiff > maxDiff )
						maxDiff = totalDiff;
				}
			}
			if( diffDirCnt >= 2  ) {
				H_Block_Motion[ii] = MOTION_NOISE_TYPE_NOISE;
				block_noise_count_H++;
			}
			else if( maxDirCnt >= 3 || (maxDirCnt>1 && maxDiff >= motion_th*maxDirCnt ) ) {
				H_Block_Motion[ii] = MOTION_NOISE_TYPE_MOTION;
				block_motion_count_H++;
			}
			else {
				H_Block_Motion[ii] = MOTION_NOISE_TYPE_STILL;
			}

			// V_Block motion determination
			maxDirCnt = 0;
			sameDirCnt = 0;
			diffDirCnt = 0;
			maxDiff = 0;
			totalDiff = 0;
			if( abs_value(diff_V[0][ii]) < motion_th )
				currDir = 0;
			else
				currDir = (diff_V[0][ii]>=0 ? 1 : -1);

			for( jj=1; jj<5; jj++ ) {
				if( abs_value(diff_V[jj][ii]) < motion_th )
					sign = 0;
				else
					sign = (diff_V[jj][ii]>=0 ? 1 : -1);

				if( sign == currDir && currDir!= 0 ) {
					sameDirCnt++;
					if( sameDirCnt > maxDirCnt )
						maxDirCnt = sameDirCnt;
					totalDiff += abs_value(diff_V[jj][ii]);
					if( totalDiff > maxDiff )
						maxDiff = totalDiff;
				}
				else if( sign != 0 && currDir != 0 ) { // different direction
					sameDirCnt = 0;
					diffDirCnt++;
					currDir = sign;
					totalDiff = 0;
				}
				else {
					totalDiff = abs_value(totalDiff + diff_V[jj][ii]);
					if( totalDiff > maxDiff )
						maxDiff = totalDiff;
				}
			}
			if( diffDirCnt > 2  ) {
				V_Block_Motion[ii] = MOTION_NOISE_TYPE_NOISE;
				block_noise_count_V++;
			}
			else if( maxDirCnt >= 3 || (maxDirCnt>1 && maxDiff >= motion_th*maxDirCnt ) ) {
				V_Block_Motion[ii] = MOTION_NOISE_TYPE_MOTION;
				block_motion_count_V++;
			}
			else {
				V_Block_Motion[ii] = MOTION_NOISE_TYPE_STILL;
			}

		}

	}

	isEvenField = (isEvenField+1) % 2;
	//flag_motion_type_pre = flag_motion_type;
	if( (block_motion_count_H >= 8) || (block_motion_count_V >= 8) )
		isMotion = MOTION_NOISE_TYPE_MOTION;
	else {
		isMotion = MOTION_NOISE_TYPE_STILL;
	}
#if 0
	if( MA_print_count% 180==0 ) {
		///*
		for( ii=0; ii<Profile_Block_Num; ii++ ) {
			ROSPrintf("diff_H of block %d: ", ii );
			for( jj=0; jj<5; jj++ ) {
				ROSPrintf("%d ", diff_H[jj][ii] );
			}
			ROSPrintf("\n");
		}


		for( ii=0; ii<Profile_Block_Num; ii++ ) {
			ROSPrintf("diff_V of block %d: ", ii );
			for( jj=0; jj<5; jj++ ) {
				ROSPrintf("%d ", diff_V[jj][ii] );
			}
			ROSPrintf("\n");
		}

		ROSPrintf("H motion level: ");
		for( ii=0; ii<Profile_Block_Num; ii++ ) {
			ROSPrintf("%d ", H_Block_Motion[ii]);
		}
		ROSPrintf("\n");
		ROSPrintf("V motion level: ");
		for( ii=0; ii<Profile_Block_Num; ii++ ) {
			ROSPrintf("%d ", V_Block_Motion[ii]);
		}
		ROSPrintf("\n");
		//*/
		//ROSPrintf("block_motion_count_H = %d, block_motion_count_V = %d\n", block_motion_count_H, block_motion_count_V );
	}
#endif
	return isMotion;
}

unsigned char scalerVIP_RTNR_level_compute(MOTION_NOISE_TYPE MADMotion, MOTION_NOISE_TYPE blockMotion, unsigned short *PQA_Input_Level, unsigned short *PQA_Input_rate)
{
	static char RTNR_level_SdHd = 1;
	static char SPNR_level_SdHd;
	static unsigned char counter_file = 0;
	unsigned char motion_blk_cnt;
	char target_level=0,target_level_T=0,target_level_S=0;
	static unsigned short motion_total = 0; // for avg..
	static unsigned char motion_cnt= 0;
	char content_spatial_noise_offset = 0;

	// content spatial noise meter offset
	if( scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL_RELIABLE) )
	{
		unsigned char spatial_noise_level = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL);

		if( (spatial_noise_level >= 1) && (spatial_noise_level <= 2) )
		{
			content_spatial_noise_offset = 1;
		}
		else if( (spatial_noise_level >= 3) && (spatial_noise_level <= 4))
		{
			content_spatial_noise_offset = 2;
		}
		else
		{
			content_spatial_noise_offset = 0;
		}
	}


	// motion_level: 0 <- still      motion -> 9
	// target_level = 9-motion_level
	if( MADMotion == MOTION_NOISE_TYPE_LARGE_MOTION )
	{
		motion_total += 9;
		motion_cnt++;
	}
	else
	{
		motion_blk_cnt = (block_motion_count_H>=block_motion_count_V) ? block_motion_count_H : block_motion_count_V;
		if( motion_blk_cnt >= 12 )
		{
			motion_total += 9;
			motion_cnt++;
		}
		else if( scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN) > 0 )  // V-PAN condition
		{
			motion_total += 9;
			motion_cnt++;
		}
		else if( motion_blk_cnt >= 7 ) // half motion <- 5~9 -> total motion
		{
			//motion_total += (12 - motion_blk_cnt);
			motion_total += motion_blk_cnt - 2;
			motion_cnt++;
		}
		else if( MADMotion == MOTION_NOISE_TYPE_MOTION )
		{
			motion_total += 7;
			motion_cnt++;
		}
	}

	counter_file++;

	if( counter_file % 21 == 0 )
	{
		if( motion_cnt > 10 )
		{
			//target_level = motion_total / motion_cnt - 1;
			target_level = 9 - (motion_total / motion_cnt);
		}
		else
		{
			//target_level = 9;
			target_level = 9 - (motion_total / 21);
		}

		// content special noise offset
		//target_level += content_spatial_noise_offset;
		//if( target_level > 9 )
			//target_level = 9;

		target_level_T=target_level;
		target_level_S=target_level;

		// special pattern identify
		if( (DynamicOptimizeSystem[35] == 1) || (DynamicOptimizeSystem[47] == 1) ||  (DynamicOptimizeSystem[133] == 1) || (DynamicOptimizeSystem[134] == 1) || (DynamicOptimizeSystem[135] == 1) )
		{
			target_level_T = 0;
		}


		if( RTNR_level_SdHd > target_level_T )
		{
			RTNR_level_SdHd--;
			// switch on correction1
			IoReg_SetBits(DI_IM_DI_RTNR_CONTROL_reg, _BIT3);
		}
		else if( RTNR_level_SdHd < target_level_T )
		{
			RTNR_level_SdHd++;
			// switch on correction1
			IoReg_SetBits(DI_IM_DI_RTNR_CONTROL_reg, _BIT3);
		}

		counter_file = 0;
		motion_total = 0;
		motion_cnt = 0;

		//ROSPrintf( "Current RTNR level = %d, target level = %d\n", RTNR_level_SdHd, target_level );
		//ROSPrintf( "Block motion H = %d, block motion V = %d\n", block_motion_count_H, block_motion_count_V );
		    SPNR_level_SdHd = RTNR_level_SdHd;

			// content special noise offset
		    if(target_level>5) // enhance TNR
		        target_level_T+=content_spatial_noise_offset;
		    else // enhance SNR
		      	target_level_S -= content_spatial_noise_offset;


		    if( RTNR_level_SdHd > target_level_T )
		        RTNR_level_SdHd--;
		    else if( RTNR_level_SdHd < target_level_T )
		        RTNR_level_SdHd++;

		    if( SPNR_level_SdHd > target_level_S )
		        SPNR_level_SdHd--;
		    else if( SPNR_level_SdHd < target_level_S )
		        SPNR_level_SdHd++;

		    if (RTNR_level_SdHd>9)
		        RTNR_level_SdHd=9;
		    if(RTNR_level_SdHd<0)
		        RTNR_level_SdHd=0;

		    if (SPNR_level_SdHd>9)
		        SPNR_level_SdHd=9;
		    if(SPNR_level_SdHd<0)
		        SPNR_level_SdHd=0;

	}



	if( SmartPic_clue->SceneChange_NR )
	{
		if(Start_Print.Rtnr_1==TRUE)
		{
			ROSPrintf("*************** [RTNR] Scene Change!!!!! ***************\n" );
		}
		RTNR_level_SdHd = 0;
	}

	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Snr==TRUE))
	{
		ROSPrintf( "** target level = %d, target level T = %d, target level S = %d\n", target_level, target_level_T, target_level_S );
		ROSPrintf( "** RTNR level = %d, SPNR level = %d\n",  RTNR_level_SdHd, SPNR_level_SdHd );
		ROSPrintf( "** content_spatial_noise_offset = %d\n", content_spatial_noise_offset );
	}

	PQA_Input_Level[I_MOTION_RTNR] = RTNR_level_SdHd;
	PQA_Input_rate[I_MOTION_RTNR] =0;
	PQA_Input_Level[I_MOTION_SPNR] = SPNR_level_SdHd;
	PQA_Input_rate[I_MOTION_SPNR] =0;
	return RTNR_level_SdHd;
}

//#ifdef ISR_READ_SRC_DET_FUNCTIOM_ENABLE
char scalerVIP_ReadSrcDet(unsigned char step_thl, unsigned char step_max, unsigned char step_min)
{
	profile_srcdet_control_RBUS srcdet_control_reg;
	profile_srcdet_tsh_step_thl_RBUS srcdet_tsh_step_thl_reg;
	profile_srcdet_tsh_step_max_RBUS srcdet_tsh_step_max_reg;
	profile_srcdet_tsh_step_min_RBUS srcdet_tsh_step_min_reg;
	UINT32 sum=0;


	unsigned int average;
	unsigned char i=0;
	srcdet_control_reg.regValue = rtd_inl(PROFILE_SRCDET_CONTROL_reg);
	srcdet_tsh_step_thl_reg.regValue = rtd_inl(PROFILE_SRCDET_TSH_STEP_THL_reg);
	srcdet_tsh_step_max_reg.regValue = rtd_inl(PROFILE_SRCDET_TSH_STEP_MAX_reg);
	srcdet_tsh_step_min_reg.regValue = rtd_inl(PROFILE_SRCDET_TSH_STEP_MIN_reg);

	srcdet_tsh_step_thl_reg.srcdet_tsh_step_thl = step_thl;
	srcdet_tsh_step_max_reg.srcdet_tsh_step_max = step_max;
	srcdet_tsh_step_min_reg.srcdet_tsh_step_min = step_min;


	rtd_outl(PROFILE_SRCDET_CONTROL_reg, srcdet_control_reg.regValue);
	if(ini_ReadSrcDet==0)
	{

		srcdet_tsh_step_thl_reg.srcdet_tsh_step_thl = 31;
		srcdet_tsh_step_max_reg.srcdet_tsh_step_max = step_max;
		srcdet_tsh_step_min_reg.srcdet_tsh_step_min = step_min;
	rtd_outl(PROFILE_SRCDET_TSH_STEP_THL_reg, srcdet_tsh_step_thl_reg.regValue);
	rtd_outl(PROFILE_SRCDET_TSH_STEP_MAX_reg, srcdet_tsh_step_max_reg.regValue);
	rtd_outl(PROFILE_SRCDET_TSH_STEP_MIN_reg, srcdet_tsh_step_min_reg.regValue);
		ini_ReadSrcDet=1;
	}
	if ( srcdet_control_reg.srcdet_start == 0 ) //wait for finish flag
	{

		profile_srcdet_tsh_hist01_RBUS tsh_hist01;
		profile_srcdet_tsh_hist02_RBUS tsh_hist02;
		profile_srcdet_tsh_hist03_RBUS tsh_hist03;
		profile_srcdet_tsh_hist04_RBUS tsh_hist04;
		profile_srcdet_tsh_hist05_RBUS tsh_hist05;
		profile_srcdet_tsh_hist06_RBUS tsh_hist06;
		profile_srcdet_tsh_hist07_RBUS tsh_hist07;
		profile_srcdet_tsh_hist08_RBUS tsh_hist08;
		profile_srcdet_tsh_hist09_RBUS tsh_hist09;
		profile_srcdet_tsh_hist10_RBUS tsh_hist10;
		profile_srcdet_tsh_hist11_RBUS tsh_hist11;
		profile_srcdet_tsh_hist12_RBUS tsh_hist12;
		profile_srcdet_tsh_hist13_RBUS tsh_hist13;
		profile_srcdet_tsh_hist14_RBUS tsh_hist14;
		profile_srcdet_tsh_hist15_RBUS tsh_hist15;
		profile_srcdet_tsh_hist16_RBUS tsh_hist16;
		profile_srcdet_tsh_hist17_RBUS tsh_hist17;
		profile_srcdet_tsh_hist18_RBUS tsh_hist18;
		profile_srcdet_tsh_hist19_RBUS tsh_hist19;
		profile_srcdet_tsh_hist20_RBUS tsh_hist20;

		profile_srcdet_pmh_hist0_RBUS pmh_hist0;
		profile_srcdet_pmh_hist1_RBUS pmh_hist1;
		profile_srcdet_pmh_hist2_RBUS pmh_hist2;
		profile_srcdet_pmh_hist3_RBUS pmh_hist3;
		profile_srcdet_pmh_hist4_RBUS pmh_hist4;
		profile_srcdet_pmh_hist5_RBUS pmh_hist5;

		tsh_hist01.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST01_reg);
		tsh_hist02.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST02_reg);
		tsh_hist03.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST03_reg);
		tsh_hist04.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST04_reg);
		tsh_hist05.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST05_reg);
		tsh_hist06.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST06_reg);
		tsh_hist07.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST07_reg);
		tsh_hist08.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST08_reg);
		tsh_hist09.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST09_reg);
		tsh_hist10.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST10_reg);
		tsh_hist11.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST11_reg);
		tsh_hist12.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST12_reg);
		tsh_hist13.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST13_reg);
		tsh_hist14.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST14_reg);
		tsh_hist15.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST15_reg);
		tsh_hist16.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST16_reg);
		tsh_hist17.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST17_reg);
		tsh_hist18.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST18_reg);
		tsh_hist19.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST19_reg);
		tsh_hist20.regValue = rtd_inl(PROFILE_SRCDET_TSH_HIST20_reg);

		pmh_hist0.regValue = rtd_inl(PROFILE_SRCDET_PMH_HIST0_reg);
		pmh_hist1.regValue = rtd_inl(PROFILE_SRCDET_PMH_HIST1_reg);
		pmh_hist2.regValue = rtd_inl(PROFILE_SRCDET_PMH_HIST2_reg);
		pmh_hist3.regValue = rtd_inl(PROFILE_SRCDET_PMH_HIST3_reg);
		pmh_hist4.regValue = rtd_inl(PROFILE_SRCDET_PMH_HIST4_reg);
		pmh_hist5.regValue = rtd_inl(PROFILE_SRCDET_PMH_HIST5_reg);

		Step_Hist[0] = tsh_hist01.srcdet_tsh_hist01;
		Step_Hist[1] = tsh_hist02.srcdet_tsh_hist02;
		Step_Hist[2] = tsh_hist03.srcdet_tsh_hist03;
		Step_Hist[3] = tsh_hist04.srcdet_tsh_hist04;
		Step_Hist[4] = tsh_hist05.srcdet_tsh_hist05;
		Step_Hist[5] = tsh_hist06.srcdet_tsh_hist06;
		Step_Hist[6] = tsh_hist07.srcdet_tsh_hist07;
		Step_Hist[7] = tsh_hist08.srcdet_tsh_hist08;
		Step_Hist[8] = tsh_hist09.srcdet_tsh_hist09;
		Step_Hist[9] = tsh_hist10.srcdet_tsh_hist10;
		Step_Hist[10] = tsh_hist11.srcdet_tsh_hist11;
		Step_Hist[11] = tsh_hist12.srcdet_tsh_hist12;
		Step_Hist[12] = tsh_hist13.srcdet_tsh_hist13;
		Step_Hist[13] = tsh_hist14.srcdet_tsh_hist14;
		Step_Hist[14] = tsh_hist15.srcdet_tsh_hist15;
		Step_Hist[15] = tsh_hist16.srcdet_tsh_hist16;
		Step_Hist[16] = tsh_hist17.srcdet_tsh_hist17;
		Step_Hist[17] = tsh_hist18.srcdet_tsh_hist18;
		Step_Hist[18] = tsh_hist19.srcdet_tsh_hist19;
		Step_Hist[19] = tsh_hist20.srcdet_tsh_hist20;

		PMH_Hist[0] = pmh_hist0.srcdet_pmh_hist0;
		PMH_Hist[1] = pmh_hist1.srcdet_pmh_hist1;
		PMH_Hist[2] = pmh_hist2.srcdet_pmh_hist2;
		PMH_Hist[3] = pmh_hist3.srcdet_pmh_hist3;
		PMH_Hist[4] = pmh_hist4.srcdet_pmh_hist4;
		PMH_Hist[5] = pmh_hist5.srcdet_pmh_hist5;


		//enable profile
		srcdet_control_reg.srcdet_pmh_depend_tsh = 0; // always do PMH
		srcdet_control_reg.srcdet_start = 1;
		rtd_outl(PROFILE_SRCDET_CONTROL_reg, srcdet_control_reg.regValue);

		step_count = Step_Hist[0]+Step_Hist[1]+Step_Hist[2];

		for( i=0; i<20; i++ )
		{
			sum += Step_Hist[i];
		}
		if( sum>0 )
		{
			average = step_count*1000/sum;
		}
		else
		{
			average = 0;
		}

		return SUCCESS;
	}
	else
	{
		//scalerDrvPrint( "[ScalerVIP] ==========Profile NOT ready==========\n" );
		return FAILED;
	}
}

unsigned char scalerVIP_Input_Source_Detection(unsigned int HD_SD_Thl)
{
	unsigned char i;
	unsigned char isScaled = FALSE;
	unsigned int sum=0;
	unsigned int weight_sum=0; // scale to 100x for average precision
	unsigned int squaresum = 0; // scale to 10000x for variance precision
	unsigned int max=0; // TSH max value
	unsigned char maxIdx=0; // TSH peak index
	unsigned int average;
	unsigned int variance;

	for( i=0; i<20; i++ )
	{
		if( Step_Hist[i]>max )
		{
			max = Step_Hist[i];
			maxIdx = i;
		}
		sum += Step_Hist[i];
		weight_sum += Step_Hist[i]*(i+1);
		squaresum += Step_Hist[i]*(i+1)*(i+1);
	}

	if( sum>0 )
	{
		average = weight_sum*100/sum;
		variance = squaresum*100/sum - average*average/100;
	}
	else
	{
		average = 0;
		variance = 0;
	}

	//if( MA_print_count % 180 == 0 )
	//	ROSPrintf( "[InputSourceDetection] TSH Average 100x = %d, variance 100x = %d\n", average, variance );

	if( average >= HD_SD_Thl )
		isScaled = TRUE;
	else
		isScaled = FALSE;

	return isScaled;
}
//#endif


unsigned char scalerVIP_DCC_Black_White_Detect(void)
{
	unsigned char i;
	unsigned int hist_max=0, hist_max2=0;
	unsigned char hist_max_idx=0, hist_max2_idx=0;
	unsigned int peak1_sum, peak2_sum;
	unsigned int *reg_HistCnt=(SmartPic_clue->Y_Main_Hist);

	for(i=0; i<32; i++ )
	{
		if( reg_HistCnt[i] > hist_max )
		{
			hist_max2 = hist_max;
			hist_max2_idx = hist_max_idx;
			hist_max = reg_HistCnt[i];
			hist_max_idx = i;
		}
		else if( reg_HistCnt[i] > hist_max2 )
		{
			hist_max2 = reg_HistCnt[i];
			hist_max2_idx = i;
		}
	}

	hist_max_idx = (hist_max_idx == 0) ? 1 : ((hist_max_idx==31) ? 30 : hist_max_idx);
	hist_max2_idx = (hist_max2_idx == 0) ? 1 : ((hist_max2_idx==31) ? 30 : hist_max2_idx);

	if( (hist_max_idx <= 2 )|| (hist_max_idx >= 28) )
	{
		peak1_sum = reg_HistCnt[ hist_max_idx-1 ] + reg_HistCnt[ hist_max_idx ]+ reg_HistCnt[ hist_max_idx+1 ];
		peak2_sum = reg_HistCnt[ hist_max2_idx-1 ] + reg_HistCnt[ hist_max2_idx ]+ reg_HistCnt[ hist_max2_idx+1 ];
		if( peak1_sum > (SmartPic_clue->Hist_Y_Total_sum)*3/4)
			return TRUE; // peak white or black

		else if( (abs_value(hist_max_idx-hist_max2_idx) >= 26) && (peak1_sum+peak2_sum > (SmartPic_clue->Hist_Y_Total_sum)*3/4) && ((SmartPic_clue->saturationflag)==0))
			return TRUE; // 2 peaks one white and black

		else
			return FALSE;
	}
	else
		return FALSE;
}

//=====================================================================================================================
//vvvvvvvvvvvvvvvvvvvvvvvTV_006_stylevvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvTV_006_stylevvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvTV_006_stylevvv
unsigned int scalerVIP_Cal_Idx_Ave_I_006_noise_mode3(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_006 32
	#define windowSizeBit_I_006 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_006]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_006;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_006;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_006;

		idx++;
		if(idx==windowSize_I_006)
			idx=0;
	}
	return ret;
}
unsigned int scalerVIP_Cal_Idx_Ave_I_006_mv_mode3(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_006 32
	#define windowSizeBit_I_006 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_006]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_006;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_006;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_006;

		idx++;
		if(idx==windowSize_I_006)
			idx=0;
	}
	return ret;
}

unsigned int scalerVIP_Cal_Idx_Ave_I_006_noise_mode4(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_006 32
	#define windowSizeBit_I_006 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_006]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_006;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_006;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_006;

		idx++;
		if(idx==windowSize_I_006)
			idx=0;
	}
	return ret;
}
unsigned int scalerVIP_Cal_Idx_Ave_I_006_mv_mode4(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_006 32
	#define windowSizeBit_I_006 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_006]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_006;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_006;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_006;

		idx++;
		if(idx==windowSize_I_006)
			idx=0;
	}
	return ret;
}

unsigned int scalerVIP_Cal_Idx_Ave_I_006(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_006 32
	#define windowSizeBit_I_006 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_006]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_006;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_006;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_006;

		idx++;
		if(idx==windowSize_I_006)
			idx=0;
	}
	return ret;
}

unsigned int scalerVIP_Cal_Idx_Ave_I_006_mv(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_006 32
	#define windowSizeBit_I_006 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_006]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_006;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_006;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_006;

		idx++;
		if(idx==windowSize_I_006)
			idx=0;
	}
	return ret;
}

unsigned int scalerVIP_Cal_Idx_Ave_I_006_noise(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_006 32
	#define windowSizeBit_I_006 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_006]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_006;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_006;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_006;

		idx++;
		if(idx==windowSize_I_006)
			idx=0;
	}
	return ret;
}
unsigned int scalerVIP_Cal_Idx_Ave_I_006_mcnr(unsigned int lv_index, unsigned char isAll_Flag)
{

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_006]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;
	unsigned char PQA_table_select= 0;
	unsigned int windowSize =0;

	SLR_VIP_TABLE* Share_Memory_VIP_TABLE_Struct = NULL;
	_clues* SmartPic_clue=NULL;
	_system_setting_info* system_info_structure_table = NULL;

	Share_Memory_VIP_TABLE_Struct = scaler_GetShare_Memory_VIP_TABLE_Struct();
	SmartPic_clue = scaler_GetShare_Memory_SmartPic_Clue_Struct();
	system_info_structure_table = scaler_GetShare_Memory_system_setting_info_Struct();


	if((Share_Memory_VIP_TABLE_Struct == NULL)||(SmartPic_clue == NULL)||(system_info_structure_table == NULL)) {
		if(MA_print_count%360==0)
			ROSPrintf("Share Memory PQ data ERROR, Share_Memory_VIP_TABLE_Struct=%p, SmartPic_clue = %p, system_info_structure_table=%p\n",Share_Memory_VIP_TABLE_Struct,SmartPic_clue,system_info_structure_table);
		return 0;
	}



	PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;

	windowSize = Share_Memory_VIP_TABLE_Struct->PQA_Input_Table[PQA_table_select][I_VD_noise_status][0];

	if(windowSize>windowSize_I_006)
		windowSize=windowSize_I_006;
	if(windowSize==0)
		windowSize=32;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize;i++)
			sum = sum + levelIdx_arr[i];
		//ret = sum>>windowSizeBit_I_006;
		ret = sum/windowSize;
		idx++;
		if(idx>=windowSize)
			idx=0;
	}
	return ret;
}


#define windowSize_I_mcnr 8
#define windowSizeBit_I_mcnr 3
unsigned int scalerVIP_compute_I_hmcnrlevel_Index_Ave(unsigned int lv_index, unsigned char isAll_Flag)
{
	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_mcnr]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_mcnr;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_mcnr;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_mcnr;

		idx++;
		if(idx==windowSize_I_mcnr)
			idx=0;
	}
	return ret;
}

unsigned int scalerVIP_Cal_Idx_Ave_I_006_chroma(unsigned int lv_index, unsigned char isAll_Flag)
{

#define windowSize_I_006 32
#define windowSizeBit_I_006 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_006]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_006;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_006;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_006;

		idx++;
		if(idx==windowSize_I_006)
			idx=0;
	}
	return ret;
}
#define windowSize_I_mcnr 8
#define windowSizeBit_I_mcnr 3
unsigned int scalerVIP_compute_I_chromanrlevel_Index_Ave(unsigned int lv_index, unsigned char isAll_Flag)
{
	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_mcnr]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_mcnr;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_mcnr;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_mcnr;

		idx++;
		if(idx==windowSize_I_mcnr)
			idx=0;
	}
	return ret;
}
#define windowSize_I_teeth 8
#define windowSizeBit_I_teeth 3
unsigned int scalerVIP_compute_I_teeth_Index_Ave(unsigned int lv_index, unsigned char isAll_Flag)
{
	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_teeth]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_teeth;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_teeth;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_teeth;

		idx++;
		if(idx==windowSize_I_teeth)
			idx=0;
	}
	return ret;
}

#define windowSize_I_Gmv 16
#define windowSizeBit_I_Gmv 4
unsigned int scalerVIP_compute_I_Gmv_Index_Ave(unsigned int lv_index, unsigned char isAll_Flag)
{
	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_Gmv]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_Gmv;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_Gmv;i++)
		{
			sum = sum + levelIdx_arr[i];
		}
		ret = sum>>windowSizeBit_I_Gmv;

		idx++;
		if(idx==windowSize_I_Gmv)
			idx=0;
	}
	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_107==TRUE))
	{
		ROSPrintf("\n sum  %d , lv_index %d\n", sum, lv_index);
	}
	return ret;
}


int scalerVIP_compute_Still_Idx_FMV_HMC_I_006(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table)
{
	//new still meter
	unsigned int printf_Flag=0, print_delay=200;
	unsigned char i;
	unsigned int *FMV_hist_ratio, *HMC_bin_hist_ratio;
	//unsigned char FMV_weight[FMV_Hist_Bin_Num] = {};
	//unsigned char HMC_bin_hist_ratio[HMC_Hist_Bin_Num] = {};
	unsigned int still_idx, still_idx_FMV, still_idx_HMC_Bin, FMV_w_total, HMC_w_total;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_FMV_w_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_FMV_Hist_Weighting][0]);	// for FMV weighting
	unsigned int *pPQA_HMC_MV_w_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_HMC_MV_Hist_Weighting][0]);
	unsigned char GMV;
	int ret_still_idx;

	FMV_hist_ratio = SmartPic_clue->FMV_hist_Ratio;
	HMC_bin_hist_ratio = SmartPic_clue->HMC_bin_hist_Ratio;
	GMV = SmartPic_clue->GMV;

	still_idx_FMV = 0;
	FMV_w_total =0;
	for(i=0;i<PQA_I_LEVEL_MAX;i++)
	{
		// 10 is center
		still_idx_FMV = still_idx_FMV + (FMV_hist_ratio[10+i]*pPQA_FMV_w_TBL[i]);
		if(i!=0)
			still_idx_FMV = still_idx_FMV + (FMV_hist_ratio[10-i]*pPQA_FMV_w_TBL[i]);
		FMV_w_total = FMV_w_total + pPQA_FMV_w_TBL[i];
		//ROSPrintf("%d,%d,%d,%d,%d\n",still_idx_FMV,FMV_hist_ratio[10+i],FMV_hist_ratio[10-i],pPQA_VD_status_TBL[i],FMV_w_total);
	}
	//if(FMV_w_total != 0)
		//still_idx_FMV = still_idx_FMV / FMV_w_total;

	still_idx_HMC_Bin = 0;
	HMC_w_total = 0;
	for(i=0;i<PQA_I_LEVEL_MAX;i++)
	{
		// 16 is center
		still_idx_HMC_Bin = still_idx_HMC_Bin + (HMC_bin_hist_ratio[16+i]*pPQA_HMC_MV_w_TBL[i]);
		if(i!=0)
			still_idx_HMC_Bin = still_idx_HMC_Bin + (HMC_bin_hist_ratio[16-i]*pPQA_HMC_MV_w_TBL[i]);
		HMC_w_total = HMC_w_total + pPQA_HMC_MV_w_TBL[i];
	}
	//if(HMC_w_total != 0)
		//still_idx_HMC_Bin = still_idx_HMC_Bin / HMC_w_total;

	still_idx = (still_idx_FMV+still_idx_HMC_Bin)>>1;

	//for debug.
	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	if((printf_Flag==4) && (MA_print_count%print_delay==0))
		ROSPrintf("FMV=%d,HMC=%d,still=%d,GMV=%d\n", still_idx_FMV, still_idx_HMC_Bin, still_idx,GMV);
	if((printf_Flag==8) && (MA_print_count%print_delay==0))
	{
		ROSPrintf("FMV_hist=");
		for(i=0;i<FMV_Hist_Bin_Num;i++)
			ROSPrintf("%d,",FMV_hist_ratio[i]);
		ROSPrintf("\n");

		ROSPrintf("HMC_hist=");
		for(i=0;i<HMC_Hist_Bin_Num;i++)
			ROSPrintf("%d,",HMC_bin_hist_ratio[i]);
		ROSPrintf("\n");

	}

	if(GMV==2 || GMV==4)
		ret_still_idx=-1;
	else if(GMV==1 || GMV==5)
		ret_still_idx=-2;
	else if(GMV==0 || GMV==6)
		ret_still_idx=-3;
	else
		ret_still_idx = still_idx_FMV;

	return ret_still_idx;
}

//=====================================================================================================================
//TV_005_style
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

void scalerVIP_compute_level_I_005_RFCVBS_DNR(unsigned char DNR_TABLE_SECET, unsigned int* g_PQA_Input_Table, unsigned short *PQA_Input_Level, unsigned short *PQA_Input_rate)
{
#if 0
	unsigned int current_Signal_Status = 0;
//==================================================================
//==================compute VD_noise_level of ==============================
//=======================TV_CVBS ====================================
//==================================================================
	//current_Signal_Status = drvif_module_vdc_NoiseStatus_isr();
	current_Signal_Status = (SmartPic_clue->VD_Signal_Status_value_avg);  //yuan1024 for rtnr n files avg

	unsigned char PQA_table_select = scaler_GetShare_Memory_system_setting_info_Struct()->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_table_select;
	unsigned int *pPQA_VD_noise_status = &(scaler_GetShare_Memory_VIP_TABLE_Struct()->PQA_Input_Table[PQA_table_select][I_VD_noise_status][0]);
//	unsigned int *pPQA_VD_noise_status_offset = &(scaler_GetShare_Memory_VIP_TABLE_Struct()->PQA_Input_Table[PQA_table_select][I_VD_noise_status_offset][0]);

	if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L00])
		PQA_Input_Level[I_005_RFCVBS] = 0;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L01])
		PQA_Input_Level[I_005_RFCVBS] = 1;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L02])
		PQA_Input_Level[I_005_RFCVBS] = 2;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L03])
		PQA_Input_Level[I_005_RFCVBS] = 3;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L04])
		PQA_Input_Level[I_005_RFCVBS] = 4;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L05])
		PQA_Input_Level[I_005_RFCVBS] = 5;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L06])
		PQA_Input_Level[I_005_RFCVBS] = 6;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L07])
		PQA_Input_Level[I_005_RFCVBS] = 7;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L08])
		PQA_Input_Level[I_005_RFCVBS] = 8;
	else
		PQA_Input_Level[I_005_RFCVBS] = 9;

	if(PQA_Input_Level[I_005_RFCVBS] >= 1) {
		if(pPQA_VD_noise_status[PQA_Input_Level[I_005_RFCVBS]] > pPQA_VD_noise_status[PQA_Input_Level[I_005_RFCVBS]-1])
			PQA_Input_rate[I_005_RFCVBS] = 255*(current_Signal_Status-pPQA_VD_noise_status[PQA_Input_Level[I_005_RFCVBS]-1])/(pPQA_VD_noise_status[PQA_Input_Level[I_005_RFCVBS]] - pPQA_VD_noise_status[PQA_Input_Level[I_005_RFCVBS]-1]);
		else
			PQA_Input_rate[I_005_RFCVBS] = 0;
	}
	else
		PQA_Input_rate[I_005_RFCVBS] = 0;
#else

	unsigned int current_Signal_Status = 0;
//==================================================================
//==================compute VD_noise_level of ==============================
//=======================TV_CVBS ====================================
//==================================================================




	unsigned int *pPQA_HMC_MV_w_TBL = NULL;

	int level_Idx=0, printf_Flag=0, print_delay=200,mv_index=0;
	unsigned char Lv=0,mv_lv=0,ori_lv=0;

		unsigned char PQA_table_select = scaler_GetShare_Memory_system_setting_info_Struct()->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_table_select;
		unsigned int *pPQA_VD_noise_status = &(scaler_GetShare_Memory_VIP_TABLE_Struct()->PQA_Input_Table[PQA_table_select][I_VD_noise_status][0]);
	//	unsigned int *pPQA_VD_noise_status_offset = &(scaler_GetShare_Memory_VIP_TABLE_Struct()->PQA_Input_Table[PQA_table_select][I_VD_noise_status_offset][0]);

	pPQA_HMC_MV_w_TBL = &(scaler_GetShare_Memory_VIP_TABLE_Struct()->PQA_Input_Table[PQA_table_select][I_HMC_MV_Hist_Weighting][0]);

	mv_index =scalerVIP_compute_I_hmcnrlevel_Index_Ave(SmartPic_clue->HMC_bin_hist_Ratio[16], 0);

	//current_Signal_Status = drvif_module_vdc_NoiseStatus_isr();
	current_Signal_Status = (SmartPic_clue->VD_Signal_Status_value_avg);  //yuan1024 for rtnr n files avg

	if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L00])
	{
		Lv = 0;
	}
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L01])
	{
		Lv = 1;
	}
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L02])
	{
		Lv = 2;
	}
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L03])
	{
		Lv = 3;

	}
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L04])
	{
		Lv = 4;
	}
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L05])
	{
		Lv = 5;
	}
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L06])
	{
		Lv = 6;
	}
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L07])
	{
		Lv = 7;
	}
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L08])
	{
		Lv = 8;
	}
	else
	{
		Lv = 9;
	}

	ori_lv = Lv;

	if(mv_index <pPQA_HMC_MV_w_TBL[0])
	{
		Lv = (Lv>9)?(Lv-9):0;
		mv_lv=9;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[1])
	{
		Lv = (Lv>8)?(Lv-8):0;
		mv_lv=8;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[2])
	{
		Lv = (Lv>7)?(Lv-7):0;
		mv_lv=7;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[3])
	{
		Lv = (Lv>6)?(Lv-6):0;
		mv_lv=6;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[4])
	{
		Lv = (Lv>5)?(Lv-5):0;
		mv_lv=5;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[5])
	{
		Lv = (Lv>4)?(Lv-4):0;
		mv_lv=4;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[6])
	{
		Lv = (Lv>3)?(Lv-3):0;
		mv_lv=3;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[7])
	{
		Lv = (Lv>2)?(Lv-2):0;
		mv_lv=2;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[8])
	{
		Lv = (Lv>1)?(Lv-1):0;
		mv_lv=1;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[9])
	{
		mv_lv=0;

	}
	else
	{
		mv_lv=0;
	}


	if(ori_lv<=3)
		Lv=ori_lv;


	if(Lv<=0)
	{
		Lv=0;

	}
	/*	 for OSD setting ,shift level Start	*/
	if(system_info_structure_table->OSD_Info.OSD_NR_Mode==1) //osd level =low
	{
		Lv = (Lv>1)?(Lv-1):0;
	}
	else if (system_info_structure_table->OSD_Info.OSD_NR_Mode==3) // osd level =high
	{
		Lv = (Lv>=9)?9:(Lv+1);
	}
	/*	 for OSD setting ,shift level END	*/


	PQA_Input_Level[I_005_RFCVBS] =Lv;

	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;

	if((printf_Flag&_BIT10)!=0 && print_delay && (MA_print_count%print_delay==0))
	{
		//ROSPrintf("Y1=%d ,Y2=%d ,Y3=%d , mv_index=%d ,Lv= %d ,C=%d ,mv_lv=%d \n",RTNR_MAD_count_Y_avg,RTNR_MAD_count_Y2_avg,RTNR_MAD_count_Y3_avg,mv_index,Lv,center,mv_lv);
		ROSPrintf("Y2=%d , mv_index=%d ,level_Idx= %d ,Lv=%d,C=%d ,mv_lv=%d \n",RTNR_MAD_count_Y2_avg,mv_index,level_Idx,Lv,center,mv_lv);

	}

#if 0
	if(PQA_Input_Level[I_005_RFCVBS] >= 1) {
		if(pPQA_VD_noise_status[PQA_Input_Level[I_005_RFCVBS]] > pPQA_VD_noise_status[PQA_Input_Level[I_005_RFCVBS]-1])
			PQA_Input_rate[I_005_RFCVBS] = 255*(current_Signal_Status-pPQA_VD_noise_status[PQA_Input_Level[I_005_RFCVBS]-1])/(pPQA_VD_noise_status[PQA_Input_Level[I_005_RFCVBS]] - pPQA_VD_noise_status[PQA_Input_Level[I_005_RFCVBS]-1]);
		else
			PQA_Input_rate[I_005_RFCVBS] = 0;
	}
	else
#endif
		PQA_Input_rate[I_005_RFCVBS] = 0;




#endif
}

void scalerVIP_compute_level_I_005_SDHD_DNR(unsigned char DNR_TABLE_SECET, unsigned int* g_PQA_Input_Table, unsigned short *PQA_Input_Level, unsigned short *PQA_Input_rate)
{
	#define DNR_Auto_Count_Num 30
	//=====================================================
	//static unsigned int counter_file = 0;
	static unsigned int RTNR_MAD_count_Y_avg_sum = 0;

	unsigned char PQA_Input_RTNR_MAD;
	unsigned char PQA_Input_DCC_Histogram_mean;

	unsigned int DCC_Histogram_Mean_value = 0;
	DCC_Histogram_Mean_value = SmartPic_clue->Hist_Y_Mean_Value;

	PQA_Input_RTNR_MAD = DNR_TABLE_SECET*PQA_I_ITEM_MAX*PQA_I_LEVEL_MAX + I_RTNR_MAD * PQA_I_LEVEL_MAX;
	PQA_Input_DCC_Histogram_mean = DNR_TABLE_SECET*PQA_I_ITEM_MAX*PQA_I_LEVEL_MAX + I_DCC_Histogram_mean * PQA_I_LEVEL_MAX;

	RTNR_MAD_count_Y_avg_sum += RTNR_MAD_count_Y2_avg;
	if(((MA_print_count % DNR_Auto_Count_Num) == 0)&&(MA_print_count != 0 ))
	{

		RTNR_MAD_count_Y_sum_avg = RTNR_MAD_count_Y_avg_sum/DNR_Auto_Count_Num;
		RTNR_MAD_count_Y_avg_sum = 0;

		if((RTNR_MAD_count_Y_sum_avg >= *(g_PQA_Input_Table + PQA_Input_RTNR_MAD + 0)) && (RTNR_MAD_count_Y_sum_avg <= *(g_PQA_Input_Table + PQA_Input_RTNR_MAD + 1)))
		{
			if(DCC_Histogram_Mean_value <= *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 0))
				PQA_Input_Level[I_005_SDHD] = 1;
			else if((DCC_Histogram_Mean_value > *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 0)) && (DCC_Histogram_Mean_value <= *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 1)))
				PQA_Input_Level[I_005_SDHD] = 2;
			else if((DCC_Histogram_Mean_value > *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 1)) && (DCC_Histogram_Mean_value <= *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 2)))
				PQA_Input_Level[I_005_SDHD] = 3;
			else if(DCC_Histogram_Mean_value > *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 2))
				PQA_Input_Level[I_005_SDHD] = 4;
		}
		else if((RTNR_MAD_count_Y_sum_avg > *(g_PQA_Input_Table + PQA_Input_RTNR_MAD + 1)) && (RTNR_MAD_count_Y_sum_avg <= *(g_PQA_Input_Table + PQA_Input_RTNR_MAD + 2)))
		{
			if(DCC_Histogram_Mean_value <= *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 0))
				PQA_Input_Level[I_005_SDHD] = 2;
			else if((DCC_Histogram_Mean_value > *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 0)) && (DCC_Histogram_Mean_value <= *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 1)))
				PQA_Input_Level[I_005_SDHD] = 3;
			else if((DCC_Histogram_Mean_value > *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 1)) && (DCC_Histogram_Mean_value <= *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 2)))
				PQA_Input_Level[I_005_SDHD] = 4;
			else if(DCC_Histogram_Mean_value > *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 2))
				PQA_Input_Level[I_005_SDHD] = 5;
		}
		else if(RTNR_MAD_count_Y_sum_avg > *(g_PQA_Input_Table + PQA_Input_RTNR_MAD + 2))
		{
			if(DCC_Histogram_Mean_value <= *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 0))
				PQA_Input_Level[I_005_SDHD] = 4;
			else if((DCC_Histogram_Mean_value > *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 0)) && (DCC_Histogram_Mean_value <= *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 1)))
				PQA_Input_Level[I_005_SDHD] = 5;
			else if((DCC_Histogram_Mean_value > *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 1)) && (DCC_Histogram_Mean_value <= *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 2)))
				PQA_Input_Level[I_005_SDHD] = 6;
			else if(DCC_Histogram_Mean_value > *(g_PQA_Input_Table + PQA_Input_DCC_Histogram_mean + 2))
				PQA_Input_Level[I_005_SDHD] = 7;
		}
		else
		{
			PQA_Input_Level[I_005_SDHD] = 7;
		}

	}
	PQA_Input_rate[I_005_SDHD] = 0;
/*
	if( MA_print_count % 60 == 0 )
	{
		ROSPrintf( "count_Y_avg_ratio = %d\n", SmartPic_clue->RTNR_MAD_count_Y_avg_ratio );
		ROSPrintf( "count_Y_avg_sum = %d, DNR_Auto_Count_Num = %d\n", RTNR_MAD_count_Y_avg_sum, DNR_Auto_Count_Num );
		ROSPrintf( "count_Y_sum_avg = %d, PQA_Input_Level = %d\n", RTNR_MAD_count_Y_sum_avg, PQA_Input_Level[I_005_SDHD] );
	}
*/
	//=====================================================

}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//TV_005_style
//=====================================================================================================================


void scalerVIP_compute_level_I_006_RTNR_Mode0(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table) // TV006 mode1 for RF CVBS RTNR
{
	unsigned int level_Idx=0, printf_Flag=0, print_delay=200;
	unsigned char Lv=0;

	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_InputMAD_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_RTNR_MAD][0]);
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];

	// get spatial content noise meter status
	unsigned int noise_status = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL);
	unsigned int noise_relaible = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL_RELIABLE);
	//get mean histgram Y
	unsigned char meanHistY = SmartPic_clue->Hist_Y_Mean_Value;

	unsigned int motion_h_t = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_T_reg);
	unsigned int motion_h_m = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_M_reg);
	unsigned int motion_h_b = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_B_reg);
	unsigned int motion_v_l = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_L_reg);
	unsigned int motion_v_m = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_M_reg);
	unsigned int motion_v_r = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_R_reg);
	unsigned int motion = rtd_inl(DI_IM_DI_SI_FILM_MOTION_reg);
	motion += pPQA_InputMAD_TBL[0]+1;

	// for id pattern
	if(DynamicOptimizeSystem[35]==1)	// lg radio,
		motion = pPQA_InputMAD_TBL[8];

	if(motion>pPQA_InputMAD_TBL[9])
		motion = pPQA_InputMAD_TBL[9];
	level_Idx = scalerVIP_Cal_Idx_Ave_I_006(motion, 0);

	if(level_Idx <= pPQA_InputMAD_TBL[0])
		Lv = 9;
	else if(level_Idx <= pPQA_InputMAD_TBL[1])
		Lv = 8;
	else if(level_Idx <= pPQA_InputMAD_TBL[2])
		Lv = 7;
	else if(level_Idx <= pPQA_InputMAD_TBL[3])
		Lv = 6;
	else if(level_Idx <= pPQA_InputMAD_TBL[4])
		Lv = 5;
	else if(level_Idx <= pPQA_InputMAD_TBL[5])
		Lv = 4;
	else if(level_Idx <= pPQA_InputMAD_TBL[6])
		Lv = 3;
	else if(level_Idx <= pPQA_InputMAD_TBL[7])
		Lv = 2;
	else if(level_Idx <= pPQA_InputMAD_TBL[8])
		Lv = 1;
	else
		Lv=0;
	/*	 for OSD setting ,shift level Start Rord 2015/11/07	*/
	if(system_info_structure_table->OSD_Info.OSD_NR_Mode==1) //osd level =low
	{
		Lv = (Lv>1)?(Lv-1):0;
	}
	else if (system_info_structure_table->OSD_Info.OSD_NR_Mode==3) // osd level =high
	{
		Lv = (Lv>=9)?9:(Lv+1);
	}
	/*	 for OSD setting ,shift level END	*/

	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_1] = motion;

	PQA_Input_Level[I_006_RTNR_Mode0] = Lv;
	PQA_Input_rate[I_006_RTNR_Mode0] = 0;
	scalerVIP_Set_Noise_Level(Lv);

	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//for debug.
	if(((printf_Flag&_BIT0)!=0) && (MA_print_count%print_delay==0))
		ROSPrintf("rm0~%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
		SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0], SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_1],
		SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_2], SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[I_006_RTNR_Mode2],
		noise_status, noise_relaible, meanHistY,motion_h_t,motion_h_m,motion_h_b,motion_v_l,motion_v_m,motion_v_r,motion);

}


void scalerVIP_compute_level_I_006_RTNR_Mode1(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table) // TV006 mode2 for SD RTNR
{
// it is for chroma NR information //2015/11/10 rord
	int level_Idx=0, printf_Flag=0, print_delay=200,mv_index=0;
	char Lv=0,mv_lv=0,ori_lv=0,mad_c_lv=0;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_InputMAD_Chroma_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_HMC_MV_Hist_stillIdx_th][0]); //for mad chroma
	//unsigned char meanHistY = SmartPic_clue->Hist_Y_Mean_Value;
	unsigned short Lv_intp=0;//, sr_d;
	unsigned int tmp;
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned int *pPQA_HMC_MV_C_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_HMC_MV_Hist_motionIdx_offset][0]);//for hmv
//	static unsigned char mcnr_pan_nr_level = 255;
	unsigned int uv_max_value1=0,uv_max_value2=0,uv_max_value3=0;
	unsigned char cur_source=0,sd_input=0;
	di_im_di_rtnr_control_RBUS im_di_rtnr_control_reg;

	uv_max_value1 = max(RTNR_MAD_count_U_avg,RTNR_MAD_count_V_avg);
	uv_max_value2 = max(RTNR_MAD_count_U2_avg,RTNR_MAD_count_V2_avg);
	uv_max_value3 = max(RTNR_MAD_count_U3_avg,RTNR_MAD_count_V3_avg);



	level_Idx = scalerVIP_Cal_Idx_Ave_I_006_chroma(uv_max_value2, 0);
	im_di_rtnr_control_reg.regValue = rtd_inl(DI_IM_DI_RTNR_CONTROL_reg);
	if(im_di_rtnr_control_reg.cp_rtnr_progressive==0)
		mv_index =scalerVIP_compute_I_chromanrlevel_Index_Ave(SmartPic_clue->HMC_bin_hist_Ratio[16]/10, 0);//hmc info is 1000% transfer to 100%
	else
	mv_index =scalerVIP_compute_I_chromanrlevel_Index_Ave(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(), 0);


	cur_source=(RPC_system_info_structure_table->VIP_source);

	if((cur_source >= VIP_QUALITY_CVBS_NTSC && cur_source <=VIP_QUALITY_YPbPr_576P)||
		(cur_source >= VIP_QUALITY_HDMI_480I && cur_source <=VIP_QUALITY_HDMI_576P)||
		(cur_source >= VIP_QUALITY_DTV_480I && cur_source <=VIP_QUALITY_DTV_576P)||
		(cur_source >= VIP_QUALITY_PVR_480I && cur_source <=VIP_QUALITY_PVR_576P)||
		(cur_source >= VIP_QUALITY_CVBS_SECAN && cur_source <=VIP_QUALITY_TV_NTSC443))
	{
		sd_input = 1;
	}
	else
	{
		sd_input = 0;
	}



	if(level_Idx < pPQA_InputMAD_Chroma_TBL[0])
	{
		Lv = 0;
		mad_c_lv =0;
	}
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[1])
	{
		Lv = 1;
		mad_c_lv =1;
	}
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[2])
	{
		Lv = 2;
		mad_c_lv =2;
	}
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[3])
	{
		Lv = 3;
		mad_c_lv =3;
	}
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[4])
	{
		Lv = 4;
		mad_c_lv =4;
	}
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[5])
	{
		Lv = 5;
		mad_c_lv =5;
	}
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[6])
	{
		Lv = 6;
		mad_c_lv =6;

	}
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[7])
	{
		Lv = 7;
		mad_c_lv =7;
	}
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[8])
	{
		Lv = 8;
		mad_c_lv =8;

	}
	else
	{
		Lv = 9;
		mad_c_lv =9;
	}

	ori_lv = Lv;


	if(mv_index <pPQA_HMC_MV_C_TBL[0])
	{
		Lv = (Lv>9)?(Lv-9):0;
		mv_lv=9;
	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[1])
	{
		Lv = (Lv>8)?(Lv-8):0;
		mv_lv=8;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[2])
	{
		Lv = (Lv>7)?(Lv-7):0;
		mv_lv=7;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[3])
	{
		Lv = (Lv>6)?(Lv-6):0;
		mv_lv=6;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[4])
	{
		Lv = (Lv>5)?(Lv-5):0;
		mv_lv=5;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[5])
	{
		Lv = (Lv>4)?(Lv-4):0;
		mv_lv=4;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[6])
	{
		Lv = (Lv>3)?(Lv-3):0;
		mv_lv=3;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[7])
	{
		Lv = (Lv>2)?(Lv-2):0;
		mv_lv=2;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[8])
	{
		Lv = (Lv>1)?(Lv-1):0;
		mv_lv=1;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[9])
	{
		mv_lv=0;

	}
	else
	{
		mv_lv=0;
	}


	if(ori_lv<=3)
		Lv=ori_lv;


	if(Lv<=0)
	{
		Lv=0;

	}


/////////////////// mcnr pan , use the same nr level start////////////////////////////////////////




/////////////////// mcnr pan , use the same nr level  end ////////////////////////////////////////
	/*	 for OSD setting ,shift level Start	*/
	if(system_info_structure_table->OSD_Info.OSD_NR_Mode==1) //osd level =low
	{
		Lv--;
		if(Lv<=0)
			Lv=0;
	}
	else if (system_info_structure_table->OSD_Info.OSD_NR_Mode==3) // osd level =high
	{
		Lv++;
		if(Lv>9)
			Lv=9;
	}
	/*	 for OSD setting ,shift level END	*/


	if(DynamicOptimizeSystem[7]==1)	// lg radio,
		Lv = 4;

	if(Lv==9)
		Lv_intp =0;
	else
	{
		if(level_Idx>pPQA_InputMAD_Chroma_TBL[PQA_I_L09])
			level_Idx = pPQA_InputMAD_Chroma_TBL[PQA_I_L09];
		tmp = ((pPQA_InputMAD_Chroma_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_Chroma_TBL[PQA_I_L09-Lv-1])==0)?1:(pPQA_InputMAD_Chroma_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_Chroma_TBL[PQA_I_L09-Lv-1]);
		Lv_intp = ((pPQA_InputMAD_Chroma_TBL[PQA_I_L09-Lv]-level_Idx)<<8) / tmp;
	}

	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;

	PQA_Input_Level[I_006_RTNR_Mode1] = Lv;
	PQA_Input_rate[I_006_RTNR_Mode1] = 0;

	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//for debug.

	if((printf_Flag&_BIT8)!=0 && (MA_print_count%print_delay==0))
	{
		rtd_pr_vpq_isr_info("\n RTNR_Mode1 LV=%d(%d-%d),level_Idx=%d,mv_index=%d",Lv,mad_c_lv,mv_lv,level_Idx,mv_index );
	}


	if((printf_Flag&_BIT7)!=0 && (MA_print_count%print_delay==0))
	{

	}
}



void scalerVIP_compute_level_I_006_RTNR_Mode2(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table) // TV006 mode3 for HD RTNR
{
#if 0
	unsigned int level_Idx=0, printf_Flag=0, print_delay=200;
	unsigned char Lv=0;
	static unsigned char Pre_Lv=0, Pre_rate=0;
	unsigned char i;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_InputMAD_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_RTNR_MAD][0]);
	unsigned int *pPQA_Input_HistMeanY_Dark_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean_dark][0]);
	unsigned int *pPQA_Input_HistMeanY_Dark_Gain_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean_dark_gain][0]);
	unsigned int tmp;

	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned int motioni[10]={0};

	unsigned short Lv_intp=0;//, sr_d;
	Lv_intp=0;
	int still_idx;
	still_idx=0;

	// get spatial content noise meter status
	unsigned int noise_status = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL);
	unsigned int noise_relaible = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL_RELIABLE);
	//get mean histgram Y
	unsigned char meanHistY = SmartPic_clue->Hist_Y_Mean_Value;

	//unsigned int motion_h_t = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_T_reg);
	//unsigned int motion_h_m = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_M_reg);
	//unsigned int motion_h_b = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_B_reg);
	//unsigned int motion_v_l = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_L_reg);
	//unsigned int motion_v_m = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_M_reg);
	//unsigned int motion_v_r = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_R_reg);
	unsigned int motion = rtd_inl(DI_IM_DI_SI_FILM_MOTION_reg);

	still_idx = scalerVIP_compute_Still_Idx_FMV_HMC_I_006(vipTable_shareMem, SmartPic_clue, system_info_structure_table);

	motioni[0] = motion;

	// reduce rtnr for dark sence,
	if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L00])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L00];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L00];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L01])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L01];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L01];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L02])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L02];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L02];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L03])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L03];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L03];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L04])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L04];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L04];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L05])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L05];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L05];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L06])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L06];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L06];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L07])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L07];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L07];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L08])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L08];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L08];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L09])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L09];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L09];
	else
		motion += 0;
	//dark_meanY_Gain = 1024;
	//motion = (motion*dark_meanY_Gain)>>10;
	motioni[1] = motion;

	//	for change smoothly
	if(motion>pPQA_InputMAD_TBL[PQA_I_L09])
		motion = pPQA_InputMAD_TBL[PQA_I_L09];

	// for moving bamboo women, and moving building
	if( (DynamicOptimizeSystem[198]==1) || (DynamicOptimizeSystem[199]==1) )
		motion = pPQA_InputMAD_TBL[PQA_I_L01];

	// for analog noise, little boy........., using old mode

	if( (DynamicOptimizeSystem[182]==1) || (DynamicOptimizeSystem[183]==1) || (DynamicOptimizeSystem[184]==1) || (DynamicOptimizeSystem[185]==1) || (DynamicOptimizeSystem[186]==1) || (scalerVIP_DI_MiddleWare_GetStatus(STATUS_HMCNR)==1) )
		rtd_clearbits(DI_IM_DI_RTNR_NEW_CONTROL_reg, _BIT0);
	else
		rtd_setbits(DI_IM_DI_RTNR_NEW_CONTROL_reg, _BIT0);

	level_Idx = scalerVIP_Cal_Idx_Ave_I_006(motion, 0);

	if(level_Idx <= pPQA_InputMAD_TBL[0])
		Lv = 9;
	else if(level_Idx <= pPQA_InputMAD_TBL[1])
		Lv = 8;
	else if(level_Idx <= pPQA_InputMAD_TBL[2])
		Lv = 7;
	else if(level_Idx <= pPQA_InputMAD_TBL[3])
		Lv = 6;
	else if(level_Idx <= pPQA_InputMAD_TBL[4])
		Lv = 5;
	else if(level_Idx <= pPQA_InputMAD_TBL[5])
		Lv = 4;
	else if(level_Idx <= pPQA_InputMAD_TBL[6])
		Lv = 3;
	else if(level_Idx <= pPQA_InputMAD_TBL[7])
		Lv = 2;
	else if(level_Idx <= pPQA_InputMAD_TBL[8])
		Lv = 1;
	else
		Lv=0;

	if(Lv==9)
		Lv_intp =0;
	else
	{
		if(level_Idx>pPQA_InputMAD_TBL[PQA_I_L09])
			level_Idx = pPQA_InputMAD_TBL[PQA_I_L09];
		tmp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1])==0)?1:(pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1]);
		Lv_intp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-level_Idx)<<8) / tmp;
	}

	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_1] = motion;

	if(Pre_Lv<Lv)
	{
		if(Pre_rate>=235)
		{
			Pre_rate = 0;
			Pre_Lv=Pre_Lv+1;
		}
		else
			Pre_rate=Pre_rate+20;
	}
	else if(Pre_Lv>Lv)
	{
		if(Pre_rate<=20)
		{
			Pre_rate = 255;
			Pre_Lv=Pre_Lv-1;
		}
		else
			Pre_rate=Pre_rate-20;
	}


	PQA_Input_Level[I_006_RTNR_Mode2] = Pre_Lv;
	PQA_Input_rate[I_006_RTNR_Mode2] = Pre_rate;

//	PQA_Input_Level[I_006_RTNR_Mode2] = Lv;
//	PQA_Input_rate[I_006_RTNR_Mode2] = 0;

	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//for debug.
	if((printf_Flag==1) && (MA_print_count%print_delay==0))
	{
		ROSPrintf("rm2~lv=%d,st=%d,LvI=%d,mo=%d,y=%d,int=%d,nmL=%d,nmR=%d\n",
			Lv, still_idx, level_Idx, motion, meanHistY, Lv_intp,noise_status,noise_status,noise_relaible);
		ROSPrintf("mo_i=");
		for(i=0;i<10;i++)
			ROSPrintf("%d,",motioni[i]);
		ROSPrintf("\n");
		/*
		ROSPrintf("rm2~%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
		SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0], SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_1],
		SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_2], SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[I_006_RTNR_Mode2],
		noise_status, noise_relaible, meanHistY,motion_h_t,motion_h_m,motion_h_b,motion_v_l,motion_v_m,motion_v_r,motion);
		*/
		//ROSPrintf("rm0~hpt=%d,mv=%d,\n",s_hpan_pass_total, s_MV);
	}
#else

	int level_Idx=0, printf_Flag=0, print_delay=200,mv_index=0;
	unsigned char Lv=0,mv_lv=0;
	unsigned char i;
	unsigned int HMC_count=0;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_InputMAD_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_RTNR_MAD][0]);
	unsigned char meanHistY = SmartPic_clue->Hist_Y_Mean_Value;
	unsigned short Lv_intp=0;//, sr_d;
	unsigned int tmp,tmp2;
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned int *pPQA_HMC_MV_w_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_HMC_MV_Hist_Weighting][0]);

	level_Idx = scalerVIP_Cal_Idx_Ave_I_006_noise(RTNR_MAD_count_Y2_avg, 0);

	for(i=0;i<((HMC_Hist_Bin_Num-1)>>1);i++)
	{
		HMC_count+=(SmartPic_clue->HMC_bin_hist_Ratio[32-i]*(16-i));
		HMC_count+=(SmartPic_clue->HMC_bin_hist_Ratio[i]*(16-i));
	}
	mv_index =scalerVIP_Cal_Idx_Ave_I_006_mv(HMC_count, 0);
	//mv_index =scalerVIP_Cal_Idx_Ave_I_006_mv(SmartPic_clue->HMC_bin_hist_Ratio[16], 0);

	//ROSPrintf("first_level_Idx=%d ,F_mv_index =%d\n ",level_Idx,mv_index);

	if(level_Idx < pPQA_InputMAD_TBL[0])
		Lv = 0;
	else if(level_Idx < pPQA_InputMAD_TBL[1])
		Lv = 1;
	else if(level_Idx < pPQA_InputMAD_TBL[2])
		Lv = 2;
	else if(level_Idx < pPQA_InputMAD_TBL[3])
		Lv = 3;
	else if(level_Idx < pPQA_InputMAD_TBL[4])
		Lv = 4;
	else if(level_Idx < pPQA_InputMAD_TBL[5])
		Lv = 5;
	else if(level_Idx < pPQA_InputMAD_TBL[6])
		Lv = 6;
	else if(level_Idx < pPQA_InputMAD_TBL[7])
		Lv = 7;
	else if(level_Idx < pPQA_InputMAD_TBL[8])
		Lv = 8;
	else
		Lv = 9;

	if(mv_index >pPQA_HMC_MV_w_TBL[0])
	{
		level_Idx -=700;
		mv_lv=9;
	}
	else if(mv_index >pPQA_HMC_MV_w_TBL[1])
	{
		level_Idx -=600;
		mv_lv=9;

	}
	else if(mv_index >pPQA_HMC_MV_w_TBL[2])
	{
		level_Idx -=500;
		mv_lv=8;

	}
	else if(mv_index >pPQA_HMC_MV_w_TBL[3])
	{
		level_Idx -=400;
		mv_lv=7;

	}
	else if(mv_index >pPQA_HMC_MV_w_TBL[4])
	{
		level_Idx -=300;
		mv_lv=6;

	}
	else if(mv_index >pPQA_HMC_MV_w_TBL[5])
	{
		level_Idx -=250;
		mv_lv=5;

	}
	else if(mv_index >pPQA_HMC_MV_w_TBL[6])
	{
		level_Idx -=200;
		mv_lv=4;

	}
	else if(mv_index >pPQA_HMC_MV_w_TBL[7])
	{
		level_Idx -=150;
		mv_lv=3;

	}
	else if(mv_index >pPQA_HMC_MV_w_TBL[8])
	{
		level_Idx -=100;
		mv_lv=2;

	}
	else if(mv_index >pPQA_HMC_MV_w_TBL[9])
	{
		level_Idx -=50;
		mv_lv=1;

	}
	else
	{
		mv_lv=0;
	}

	if(level_Idx<=0)
	{
		level_Idx=0;

	}

	//ROSPrintf("mv_index=%d ,pPQA_HMC_MV_w_TBL[0]=%d ,y=%d, Lv_intp=%d \n ",mv_index,pPQA_HMC_MV_w_TBL[0],mv_lv,Lv_intp);

	if(level_Idx < pPQA_InputMAD_TBL[0])
		Lv = 0;
	else if(level_Idx < pPQA_InputMAD_TBL[1])
		Lv = 1;
	else if(level_Idx < pPQA_InputMAD_TBL[2])
		Lv = 2;
	else if(level_Idx < pPQA_InputMAD_TBL[3])
		Lv = 3;
	else if(level_Idx < pPQA_InputMAD_TBL[4])
		Lv = 4;
	else if(level_Idx < pPQA_InputMAD_TBL[5])
		Lv = 5;
	else if(level_Idx < pPQA_InputMAD_TBL[6])
		Lv = 6;
	else if(level_Idx < pPQA_InputMAD_TBL[7])
		Lv = 7;
	else if(level_Idx < pPQA_InputMAD_TBL[8])
		Lv = 8;
	else
		Lv = 9;

	/*	 for OSD setting ,shift level Start Rord 2015/11/07	*/
	if(system_info_structure_table->OSD_Info.OSD_NR_Mode==1) //osd level =low
	{
		Lv = (Lv>1)?(Lv-1):0;
	}
	else if (system_info_structure_table->OSD_Info.OSD_NR_Mode==3) // osd level =high
	{
		Lv = (Lv>=9)?9:(Lv+1);
	}
	/*	 for OSD setting ,shift level END	*/


	if(Lv==9)
		Lv_intp =0;
	else
	{
		if(level_Idx>pPQA_InputMAD_TBL[PQA_I_L09])
			level_Idx = pPQA_InputMAD_TBL[PQA_I_L09];
		tmp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1])==0)?1:(pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1]);
		Lv_intp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-level_Idx)<<8) / tmp;
	}

	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;

	PQA_Input_Level[I_006_RTNR_Mode2] = Lv;
	PQA_Input_rate[I_006_RTNR_Mode2] = 0;
	scalerVIP_Set_Noise_Level(Lv);

	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//for debug.

	if((printf_Flag&_BIT9)!=0 && (MA_print_count%print_delay==0))
	{
		ROSPrintf("LV=%d ,motion=%d ,y=%d, Lv_intp=%d \n ",Lv,level_Idx,meanHistY,Lv_intp);

	}

	tmp2=1000-RTNR_MAD_count_Y_avg-RTNR_MAD_count_Y2_avg-RTNR_MAD_count_Y3_avg;
	if((printf_Flag&_BIT10)!=0 && (MA_print_count%print_delay==0))
	{
		//ROSPrintf("Y1=%d ,Y2=%d ,Y3=%d , mv_index=%d ,Lv= %d ,C=%d ,mv_lv=%d \n",RTNR_MAD_count_Y_avg,RTNR_MAD_count_Y2_avg,RTNR_MAD_count_Y3_avg,mv_index,Lv,center,mv_lv);
		ROSPrintf("Y2=%d , mv_index=%d ,level_Idx= %d ,Lv=%d,C=%d ,mv_lv=%d \n",RTNR_MAD_count_Y2_avg,mv_index,level_Idx,Lv,center,mv_lv);

	}

	if((printf_Flag&_BIT11)!=0 && (MA_print_count%print_delay==0))
	{
		//ROSPrintf("\n R5=%d ,R4=%d, R3=%d, R2=%d, R1=%d C=%d,L1=%d ,L2=%d ,L3=%d ,L4=%d ,L5=%d \n",hmcnr_sttistic[0],hmcnr_sttistic[1],hmcnr_sttistic[2],
		//hmcnr_sttistic[3],hmcnr_sttistic[4],hmcnr_sttistic[5],hmcnr_sttistic[6],hmcnr_sttistic[7],hmcnr_sttistic[8],hmcnr_sttistic[9],hmcnr_sttistic[10]);

		ROSPrintf("Y1=%d ,Y2=%d ,Y3=%d , mv_index=%d ,Lv= %d ,C=%d ,mv_lv=%d \n",RTNR_MAD_count_Y_avg,RTNR_MAD_count_Y2_avg,RTNR_MAD_count_Y3_avg,mv_index,Lv,center,mv_lv);

		//ROSPrintf("hmc_C=%d ,r1=%d,L2 =%d \n",SmartPic_clue->HMC_bin_hist_Ratio[16],SmartPic_clue->HMC_bin_hist_Ratio[15],SmartPic_clue->HMC_bin_hist_Ratio[17]);

	}

	if((printf_Flag&_BIT12)!=0 && (MA_print_count%print_delay==0))
	{
		ROSPrintf("C=%d , L1=%d ,R1= %d ,others =%d \n",center,L1,R1,others);

	}

#endif


}
void scalerVIP_compute_level_I_006_RTNR_Mode3(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table) // TV006 mode3 for HD RTNR
{
	int level_Idx=0, printf_Flag=0, print_delay=200,mv_index=0;
	signed char Lv=0,mad_lv=0,mv_lv=0,apl_lv=0,Lv_snr_mode3=0,snr_apl_lv;//,ori_lv=0;
	char Level=0;
	unsigned int tmp;

	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned char meanHistY = SmartPic_clue->Hist_Y_Mean_Value;
	unsigned short Lv_intp=0;//, sr_d;
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned int *pPQA_InputMAD_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_RTNR_MAD][0]);
	unsigned int *pPQA_HMC_MV_w_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_HMC_MV_Hist_Weighting][0]);
	unsigned int *pPQA_Input_HistMeanY_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean][0]);
	unsigned int *pPQA_Input_HistMeanY_Dark_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean_dark][0]);


	level_Idx = 0; //mac6 , Mad on 4k2k is not correct.so don't used mad information . rord.tsao 2017/12/05

	mv_index = scalerVIP_Cal_Idx_Ave_I_006_mv_mode3(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(),0);

	Lv = 9;
	mad_lv =9;

	if(mv_index <pPQA_HMC_MV_w_TBL[0])
	{
		Lv = (Lv>9)?(Lv-9):0;
		mv_lv=9;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[1])
	{
		Lv = (Lv>8)?(Lv-8):0;
		mv_lv=8;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[2])
	{
		Lv = (Lv>7)?(Lv-7):0;
		mv_lv=7;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[3])
	{
		Lv = (Lv>6)?(Lv-6):0;
		mv_lv=6;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[4])
	{
		Lv = (Lv>5)?(Lv-5):0;
		mv_lv=5;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[5])
	{
		Lv = (Lv>4)?(Lv-4):0;
		mv_lv=4;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[6])
	{
		Lv = (Lv>3)?(Lv-3):0;
		mv_lv=3;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[7])
	{
		Lv = (Lv>2)?(Lv-2):0;
		mv_lv=2;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[8])
	{
		Lv = (Lv>1)?(Lv-1):0;
		mv_lv=1;

	}
	else
	{
		mv_lv=0;
	}


	if(meanHistY<pPQA_Input_HistMeanY_TBL[0])
	{
		Lv = (Lv>9)?(Lv-9):0;
		apl_lv=9;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[1])
	{
		Lv = (Lv>8)?(Lv-8):0;
		apl_lv=8;

	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[2])
	{
		Lv = (Lv>7)?(Lv-7):0;
		apl_lv=7;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[3])
	{
		Lv = (Lv>6)?(Lv-6):0;
		apl_lv=6;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[4])
	{
		Lv = (Lv>5)?(Lv-5):0;
		apl_lv=5;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[5])
	{
		Lv = (Lv>4)?(Lv-4):0;
		apl_lv=4;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[6])
	{
		Lv = (Lv>3)?(Lv-3):0;
		apl_lv=3;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[7])
	{
		Lv = (Lv>2)?(Lv-2):0;
		apl_lv=2;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[8])
	{
		Lv = (Lv>1)?(Lv-1):0;
		apl_lv=1;
	}
	else
	{
		apl_lv=0;

	}

	if(Lv<=0)
	{
		Lv=0;

	}


	Level = Lv;

	if(system_info_structure_table->OSD_Info.OSD_NR_Mode==1) //osd level =low
	{
		Lv = (Lv>1)?(Lv-1):0;
	}
	else if (system_info_structure_table->OSD_Info.OSD_NR_Mode==3) // osd level =high
	{
		Lv = (Lv>=9)?9:(Lv+1);
	}
	/*	 for OSD setting ,shift level END	*/

	if(Lv==9)
		Lv_intp =0;
	else
	{
		if(level_Idx>pPQA_InputMAD_TBL[PQA_I_L09])
			level_Idx = pPQA_InputMAD_TBL[PQA_I_L09];
		tmp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1])==0)?1:(pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1]);
		Lv_intp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-level_Idx)<<8) / tmp;
	}


	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;

	PQA_Input_Level[I_006_RTNR_Mode3] = Lv;
	PQA_Input_rate[I_006_RTNR_Mode3] = 0;


//===========================Calculate level of SNR_Mode4 Start =========================================//


	if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[0])
		snr_apl_lv = 9;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[1])
		snr_apl_lv = 8;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[2])
		snr_apl_lv = 7;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[3])
		snr_apl_lv = 6;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[4])
		snr_apl_lv = 5;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[5])
		snr_apl_lv = 4;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[6])
		snr_apl_lv = 3;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[7])
		snr_apl_lv = 2;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[8])
		snr_apl_lv = 1;
	else
		snr_apl_lv = 0;

	Lv_snr_mode3 = 7 + mv_lv+snr_apl_lv;
	if(Lv_snr_mode3<0)
		Lv_snr_mode3 =0;
	if(Lv_snr_mode3>9)
		Lv_snr_mode3=9;

	PQA_Input_Level[I_006_SNR_Mode3] = Lv_snr_mode3;
	PQA_Input_rate[I_006_SNR_Mode3] = 0;


//===========================Calculate level of SNR_Mode4 End =========================================//


	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//for debug.

	if((printf_Flag&_BIT4)!=0 && (MA_print_count%print_delay==0))
	{
		rtd_pr_vpq_isr_info("\n SNR_Mode3 LV=%d(%d-%d+%d), snr_apl_Idx=%d,",Lv_snr_mode3,mad_lv,mv_lv,snr_apl_lv,meanHistY);
	}

	scalerVIP_Set_Noise_Level(Lv);

	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;


	if((printf_Flag&_BIT9)!=0 && (MA_print_count%print_delay==0))
	{
		rtd_pr_vpq_isr_info("\n I_006_RTNR_Mode3 Info Lv=%d",Lv);
		rtd_pr_vpq_isr_info("Calculate method : MAD-Motion-APL");
		rtd_pr_vpq_isr_info("1.MAD= idx 2:I_RTNR_MAD");
		rtd_pr_vpq_isr_info("2.Motion= idx 9:I_HMC_MV_Hist_Weighting");
		rtd_pr_vpq_isr_info("3.APL= idx3:I_DCC_Histogram_mean ");

	}
	if((printf_Flag&_BIT10)!=0 && (MA_print_count%print_delay==0))
	{
		rtd_pr_vpq_isr_info("\n RTNR_Mode3	LV=%d(%d-%d-%d),level_Idx =%d,mv_index=%d,meanHistY=%d	",Lv,mad_lv,mv_lv,apl_lv,level_Idx,mv_index,meanHistY );
	}
}

void scalerVIP_compute_level_I_006_RTNR_Mode4(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table) // TV006 mode3 for HD RTNR
{
	int level_Idx=0, printf_Flag=0, print_delay=200,mv_index=0;
	signed char Lv=0,mad_lv=0,mv_lv=0,apl_lv=0,Lv_snr_mode4=0,snr_apl_lv;//,ori_lv=0;
	unsigned char cur_source=0,sd_input=0;
	char Level=0;
	unsigned int tmp;

	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned char meanHistY = SmartPic_clue->Hist_Y_Mean_Value;
	unsigned short Lv_intp=0;//, sr_d;
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned int *pPQA_InputMAD_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_RTNR_MAD][0]);
	unsigned int *pPQA_HMC_MV_w_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_HMC_MV_Hist_Weighting][0]);
	unsigned int *pPQA_Input_HistMeanY_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean][0]);
	unsigned int *pPQA_Input_HistMeanY_Dark_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean_dark][0]);

	level_Idx = scalerVIP_Cal_Idx_Ave_I_006_noise_mode4(RTNR_MAD_count_Y2_avg, 0);
	//mv_index =scalerVIP_compute_I_hmcnrlevel_Index_Ave(SmartPic_clue->HMC_bin_hist_Ratio[16]/10, 0);//hmc info is 1000% transfer to 100%
	mv_index = scalerVIP_Cal_Idx_Ave_I_006_mv_mode4(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(),0);


	cur_source=(RPC_system_info_structure_table->VIP_source);

	if((cur_source == VIP_QUALITY_CVBS_NTSC && cur_source <=VIP_QUALITY_YPbPr_576P)||
		(cur_source >= VIP_QUALITY_HDMI_480I && cur_source <=VIP_QUALITY_HDMI_576P)||
		(cur_source >= VIP_QUALITY_DTV_480I && cur_source <=VIP_QUALITY_DTV_576P)||
		(cur_source >= VIP_QUALITY_PVR_480I && cur_source <=VIP_QUALITY_PVR_576P)||
		(cur_source >= VIP_QUALITY_CVBS_SECAN && cur_source <=VIP_QUALITY_TV_NTSC443))
	{
		sd_input = 1;
	}
	else
	{
		sd_input = 0;
	}



	if(level_Idx < pPQA_InputMAD_TBL[0])
	{
		Lv = 0;
		mad_lv =0;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[1])
	{
		Lv = 1;
		mad_lv =1;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[2])
	{
		Lv = 2;
		mad_lv =2;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[3])
	{
		Lv = 3;
		mad_lv =3;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[4])
	{
		Lv = 4;
		mad_lv =4;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[5])
	{
		Lv = 5;
		mad_lv =5;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[6])
	{
		Lv = 6;
		mad_lv =6;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[7])
	{
		Lv = 7;
		mad_lv =7;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[8])
	{
		Lv = 8;
		mad_lv =8;
	}
	else
	{
		Lv = 9;
		mad_lv =9;

	}

	if(mv_index <pPQA_HMC_MV_w_TBL[0])
	{
		Lv = (Lv>9)?(Lv-9):0;
		mv_lv=9;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[1])
	{
		Lv = (Lv>8)?(Lv-8):0;
		mv_lv=8;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[2])
	{
		Lv = (Lv>7)?(Lv-7):0;
		mv_lv=7;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[3])
	{
		Lv = (Lv>6)?(Lv-6):0;
		mv_lv=6;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[4])
	{
		Lv = (Lv>5)?(Lv-5):0;
		mv_lv=5;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[5])
	{
		Lv = (Lv>4)?(Lv-4):0;
		mv_lv=4;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[6])
	{
		Lv = (Lv>3)?(Lv-3):0;
		mv_lv=3;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[7])
	{
		Lv = (Lv>2)?(Lv-2):0;
		mv_lv=2;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[8])
	{
		Lv = (Lv>1)?(Lv-1):0;
		mv_lv=1;

	}
	else
	{
		mv_lv=0;
	}


	if(meanHistY<pPQA_Input_HistMeanY_TBL[0])
	{
		Lv = (Lv>9)?(Lv-9):0;
		apl_lv=9;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[1])
	{
		Lv = (Lv>8)?(Lv-8):0;
		apl_lv=8;

	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[2])
	{
		Lv = (Lv>7)?(Lv-7):0;
		apl_lv=7;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[3])
	{
		Lv = (Lv>6)?(Lv-6):0;
		apl_lv=6;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[4])
	{
		Lv = (Lv>5)?(Lv-5):0;
		apl_lv=5;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[5])
	{
		Lv = (Lv>4)?(Lv-4):0;
		apl_lv=4;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[6])
	{
		Lv = (Lv>3)?(Lv-3):0;
		apl_lv=3;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[7])
	{
		Lv = (Lv>2)?(Lv-2):0;
		apl_lv=2;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[8])
	{
		Lv = (Lv>1)?(Lv-1):0;
		apl_lv=1;
	}
	else
	{
		apl_lv=0;

	}

	if(Lv<=0)
	{
		Lv=0;
	}


	Level = Lv;

	if(system_info_structure_table->OSD_Info.OSD_NR_Mode==1) //osd level =low
	{
		Lv = (Lv>1)?(Lv-1):0;
	}
	else if (system_info_structure_table->OSD_Info.OSD_NR_Mode==3) // osd level =high
	{
		Lv = (Lv>=9)?9:(Lv+1);
	}
	/*	 for OSD setting ,shift level END	*/


	if(DynamicOptimizeSystem[35]==1)	// lg radio,
		Lv = 0;

	if(DynamicOptimizeSystem[7]==1) // lg radio,
		Lv = 4;

	if(Lv==9)
		Lv_intp =0;
	else
	{
		if(level_Idx>pPQA_InputMAD_TBL[PQA_I_L09])
			level_Idx = pPQA_InputMAD_TBL[PQA_I_L09];
		tmp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1])==0)?1:(pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1]);
		Lv_intp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-level_Idx)<<8) / tmp;
	}


	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;

	PQA_Input_Level[I_006_RTNR_Mode4] = Lv;
	PQA_Input_rate[I_006_RTNR_Mode4] = 0;


//===========================Calculate level of SNR_Mode4 Start =========================================//


	if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[0])
		snr_apl_lv = 9;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[1])
		snr_apl_lv = 8;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[2])
		snr_apl_lv = 7;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[3])
		snr_apl_lv = 6;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[4])
		snr_apl_lv = 5;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[5])
		snr_apl_lv = 4;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[6])
		snr_apl_lv = 3;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[7])
		snr_apl_lv = 2;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[8])
		snr_apl_lv = 1;
	else
		snr_apl_lv = 0;

	Lv_snr_mode4 = mad_lv - mv_lv+snr_apl_lv;
	if(Lv_snr_mode4<0)
		Lv_snr_mode4 =0;
	if(Lv_snr_mode4>9)
		Lv_snr_mode4 =9;

	PQA_Input_Level[I_006_SNR_Mode4] = Lv_snr_mode4;
	PQA_Input_rate[I_006_SNR_Mode4] = 0;


//===========================Calculate level of SNR_Mode4 End =========================================//


	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//for debug.


	scalerVIP_Set_Noise_Level(Lv);

	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;

	if((printf_Flag&_BIT4)!=0 && (MA_print_count%print_delay==0))
	{
		rtd_pr_vpq_isr_info("\n SNR_Mode4 LV=%d(%d-%d+%d), snr_apl_Idx=%d,",Lv_snr_mode4,mad_lv,mv_lv,snr_apl_lv,meanHistY);

	}
	if((printf_Flag&_BIT9)!=0 && (MA_print_count%print_delay==0))
	{
		rtd_pr_vpq_isr_info("\n I_006_RTNR_Mode4 Info Lv=%d",Lv);
		rtd_pr_vpq_isr_info("Calculate method : MAD-Motion-APL");
		rtd_pr_vpq_isr_info("1.MAD= idx 2:I_RTNR_MAD");
		rtd_pr_vpq_isr_info("2.Motion= idx 9:I_HMC_MV_Hist_Weighting");
		rtd_pr_vpq_isr_info("3.APL= idx3:I_DCC_Histogram_mean ");

	}
	if((printf_Flag&_BIT10)!=0 && (MA_print_count%print_delay==0))
	{
		rtd_pr_vpq_isr_info("\n Mode4  LV=%d(%d-%d-%d),level_Idx =%d,mv_index=%d,meanHistY=%d  ",Lv,mad_lv,mv_lv,apl_lv,level_Idx,mv_index,meanHistY );
	}
}



void scalerVIP_compute_level_I_006_RTNR_Mode2_HMCNR(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table) // TV006 mode2 for SD RTNR
{
	int level_Idx=0, printf_Flag=0, print_delay=200,mv_index=0;
	signed char Lv=0,mad_lv=0,mv_lv=0,apl_lv=0,Lv_snr_mode4=0,snr_apl_lv;//,ori_lv=0;
	unsigned char cur_source=0,sd_input=0;
	char Level=0;
	unsigned int tmp;

	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned char meanHistY = SmartPic_clue->Hist_Y_Mean_Value;
	unsigned short Lv_intp=0;//, sr_d;
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned int *pPQA_InputMAD_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_RTNR_MAD][0]);
	unsigned int *pPQA_HMC_MV_w_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_HMC_MV_Hist_Weighting][0]);
	unsigned int *pPQA_Input_HistMeanY_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean][0]);
	unsigned int *pPQA_Input_HistMeanY_Dark_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean_dark][0]);

	di_im_di_rtnr_control_RBUS im_di_rtnr_control_reg;



	level_Idx = scalerVIP_Cal_Idx_Ave_I_006_mcnr(RTNR_MAD_count_Y2_avg, 0);


	im_di_rtnr_control_reg.regValue = rtd_inl(DI_IM_DI_RTNR_CONTROL_reg);
	if(im_di_rtnr_control_reg.cp_rtnr_progressive==0)
		mv_index =scalerVIP_compute_I_hmcnrlevel_Index_Ave(SmartPic_clue->HMC_bin_hist_Ratio[16]/10, 0);//hmc info is 1000% transfer to 100%
	else
		mv_index = scalerVIP_compute_I_hmcnrlevel_Index_Ave(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(),0);


	cur_source=(RPC_system_info_structure_table->VIP_source);

	if((cur_source >= VIP_QUALITY_CVBS_NTSC && cur_source <=VIP_QUALITY_YPbPr_576P)||
		(cur_source >= VIP_QUALITY_HDMI_480I && cur_source <=VIP_QUALITY_HDMI_576P)||
		(cur_source >= VIP_QUALITY_DTV_480I && cur_source <=VIP_QUALITY_DTV_576P)||
		(cur_source >= VIP_QUALITY_PVR_480I && cur_source <=VIP_QUALITY_PVR_576P)||
		(cur_source >= VIP_QUALITY_CVBS_SECAN && cur_source <=VIP_QUALITY_TV_NTSC443))
	{
		sd_input = 1;
	}
	else
	{
		sd_input = 0;
	}



	if(level_Idx < pPQA_InputMAD_TBL[0])
	{
		Lv = 0;
		mad_lv =0;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[1])
	{
		Lv = 1;
		mad_lv =1;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[2])
	{
		Lv = 2;
		mad_lv =2;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[3])
	{
		Lv = 3;
		mad_lv =3;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[4])
	{
		Lv = 4;
		mad_lv =4;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[5])
	{
		Lv = 5;
		mad_lv =5;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[6])
	{
		Lv = 6;
		mad_lv =6;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[7])
	{
		Lv = 7;
		mad_lv =7;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[8])
	{
		Lv = 8;
		mad_lv =8;
	}
	else
	{
		Lv = 9;
		mad_lv =9;

	}
	//ori_lv = Lv;


	if(mv_index <pPQA_HMC_MV_w_TBL[0])
	{
		Lv = (Lv>9)?(Lv-9):0;
		mv_lv=9;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[1])
	{
		Lv = (Lv>8)?(Lv-8):0;
		mv_lv=8;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[2])
	{
		Lv = (Lv>7)?(Lv-7):0;
		mv_lv=7;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[3])
	{
		Lv = (Lv>6)?(Lv-6):0;
		mv_lv=6;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[4])
	{
		Lv = (Lv>5)?(Lv-5):0;
		mv_lv=5;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[5])
	{
		Lv = (Lv>4)?(Lv-4):0;
		mv_lv=4;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[6])
	{
		Lv = (Lv>3)?(Lv-3):0;
		mv_lv=3;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[7])
	{
		Lv = (Lv>2)?(Lv-2):0;
		mv_lv=2;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[8])
	{
		Lv = (Lv>1)?(Lv-1):0;
		mv_lv=1;

	}
	else
	{
		mv_lv=0;
	}


	if(meanHistY<pPQA_Input_HistMeanY_TBL[0])
	{
		Lv = (Lv>9)?(Lv-9):0;
		apl_lv=9;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[1])
	{
		Lv = (Lv>8)?(Lv-8):0;
		apl_lv=8;

	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[2])
	{
		Lv = (Lv>7)?(Lv-7):0;
		apl_lv=7;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[3])
	{
		Lv = (Lv>6)?(Lv-6):0;
		apl_lv=6;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[4])
	{
		Lv = (Lv>5)?(Lv-5):0;
		apl_lv=5;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[5])
	{
		Lv = (Lv>4)?(Lv-4):0;
		apl_lv=4;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[6])
	{
		Lv = (Lv>3)?(Lv-3):0;
		apl_lv=3;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[7])
	{
		Lv = (Lv>2)?(Lv-2):0;
		apl_lv=2;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[8])
	{
		Lv = (Lv>1)?(Lv-1):0;
		apl_lv=1;
	}
	else
	{
		apl_lv=0;

	}

	if(Lv<=0)
	{
		Lv=0;

	}


	Level = Lv;

	if(system_info_structure_table->OSD_Info.OSD_NR_Mode==1) //osd level =low
	{
		Lv = (Lv>1)?(Lv-1):0;
	}
	else if (system_info_structure_table->OSD_Info.OSD_NR_Mode==3) // osd level =high
	{
		Lv = (Lv>=9)?9:(Lv+1);
	}
	/*	 for OSD setting ,shift level END	*/


	if(DynamicOptimizeSystem[35]==1)	// lg radio,
		Lv = 0;

	if(DynamicOptimizeSystem[7]==1)	// lg radio,
		Lv = 4;

	if(Lv==9)
		Lv_intp =0;
	else
	{
		if(level_Idx>pPQA_InputMAD_TBL[PQA_I_L09])
			level_Idx = pPQA_InputMAD_TBL[PQA_I_L09];
		tmp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1])==0)?1:(pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1]);
		Lv_intp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-level_Idx)<<8) / tmp;
	}


	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;

	PQA_Input_Level[I_006_RTNR_Mode2_HMCNR] = Lv;
	PQA_Input_rate[I_006_RTNR_Mode2_HMCNR] = 0;


//===========================Calculate level of SNR_Mode4 Start =========================================//


	if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[0])
		snr_apl_lv = 9;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[1])
		snr_apl_lv = 8;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[2])
		snr_apl_lv = 7;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[3])
		snr_apl_lv = 6;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[4])
		snr_apl_lv = 5;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[5])
		snr_apl_lv = 4;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[6])
		snr_apl_lv = 3;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[7])
		snr_apl_lv = 2;
	else if(meanHistY < pPQA_Input_HistMeanY_Dark_TBL[8])
		snr_apl_lv = 1;
	else
		snr_apl_lv = 0;

	Lv_snr_mode4 = mad_lv - mv_lv+snr_apl_lv;
	if(Lv_snr_mode4<0)
		Lv_snr_mode4 =0;
	if(Lv_snr_mode4>9)
		Lv_snr_mode4 =9;

	PQA_Input_Level[I_006_SNR_Mode4] = Lv_snr_mode4;
	PQA_Input_rate[I_006_SNR_Mode4] = 0;


//===========================Calculate level of SNR_Mode4 End =========================================//


	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//for debug.

	if((printf_Flag&_BIT4)!=0 && (MA_print_count%print_delay==0))
	{
		rtd_pr_vpq_isr_info("\n SNR_Mode4 LV=%d(%d-%d+%d), snr_apl_Idx=%d,",Lv_snr_mode4,mad_lv,mv_lv,snr_apl_lv,meanHistY);

	}


	scalerVIP_Set_Noise_Level(Lv);

	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;




	if((printf_Flag&_BIT9)!=0 && (MA_print_count%print_delay==0))
	{
		rtd_pr_vpq_isr_info("\n I_006_RTNR_Mode2_HMCNR Info Lv=%d",Lv);
		rtd_pr_vpq_isr_info("Calculate method : MAD-Motion-APL");
		rtd_pr_vpq_isr_info("1.MAD= idx 2:I_RTNR_MAD");
		rtd_pr_vpq_isr_info("2.Motion= idx 9:I_HMC_MV_Hist_Weighting");
		rtd_pr_vpq_isr_info("3.APL= idx3:I_DCC_Histogram_mean ");

	}
	if((printf_Flag&_BIT10)!=0 && (MA_print_count%print_delay==0))
	{
		rtd_pr_vpq_isr_info("\n Mode2_HMCNR  LV=%d(%d-%d-%d),level_Idx =%d,mv_index=%d,meanHistY=%d  ",Lv,mad_lv,mv_lv,apl_lv,level_Idx,mv_index,meanHistY );
	}
}


void scalerVIP_compute_level_I_006_SNR_Mode0(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table) // TV006 mode1 for RF CVBS SNR
{
	int level_Idx=0;
	unsigned char Lv=0;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_InputMAD_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_FMV_Hist_motionIdx_offset][0]);
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];


	//NOTICE it is only for interlace mode

	//unsigned int noise_relaible = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL_RELIABLE);
	//unsigned int motion = rtd_inl(DI_IM_DI_SI_FILM_MOTION_reg);
	unsigned char motionl_level = scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION);

	//ROSPrintf("\n motionl_level =%d ,relaible = %d \n",motionl_level,noise_relaible);
	level_Idx = motionl_level;

	if(level_Idx < pPQA_InputMAD_TBL[0])
		Lv = 0;
	else if(level_Idx < pPQA_InputMAD_TBL[1])
		Lv = 1;
	else if(level_Idx < pPQA_InputMAD_TBL[2])
		Lv = 2;
	else if(level_Idx < pPQA_InputMAD_TBL[3])
		Lv = 3;
	else if(level_Idx < pPQA_InputMAD_TBL[4])
		Lv = 4;
	else if(level_Idx < pPQA_InputMAD_TBL[5])
		Lv = 5;
	else if(level_Idx < pPQA_InputMAD_TBL[6])
		Lv = 6;
	else if(level_Idx < pPQA_InputMAD_TBL[7])
		Lv = 7;
	else if(level_Idx < pPQA_InputMAD_TBL[8])
		Lv = 8;
	else
		Lv = 9;


	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;

	PQA_Input_Level[I_006_SNR_Mode0] = Lv;
	PQA_Input_rate[I_006_SNR_Mode0] = 0;

}


void scalerVIP_compute_level_I_006_SNR_Mode1(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table) // TV006 mode2 for SD SNR
	{

	#if 0
		unsigned int apl_Idx=0;
		signed char Lv=0,Apl_Lv=0;
		unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
		unsigned int *pPQA_InputAPL_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_LC_1][0]);
		unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
		unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
		static LEVEL_INFO Level_Info;
		unsigned int printf_Flag=0, print_delay=200;

		apl_Idx = SmartPic_clue->Hist_Y_Mean_Value;

		scalerVIP_Get_NR_Info(&Level_Info);


		if(apl_Idx < pPQA_InputAPL_TBL[0])
			Apl_Lv = 9;
		else if(apl_Idx < pPQA_InputAPL_TBL[1])
			Apl_Lv = 8;
		else if(apl_Idx < pPQA_InputAPL_TBL[2])
			Apl_Lv = 7;
		else if(apl_Idx < pPQA_InputAPL_TBL[3])
			Apl_Lv = 6;
		else if(apl_Idx < pPQA_InputAPL_TBL[4])
			apl_Idx = 5;
		else if(apl_Idx < pPQA_InputAPL_TBL[5])
			Apl_Lv = 4;
		else if(apl_Idx < pPQA_InputAPL_TBL[6])
			Apl_Lv = 3;
		else if(apl_Idx < pPQA_InputAPL_TBL[7])
			Apl_Lv = 2;
		else if(apl_Idx < pPQA_InputAPL_TBL[8])
			Apl_Lv = 1;
		else
			Apl_Lv = 0;

		Lv = Level_Info.Mad_lv -Level_Info.Motion_lv+Apl_Lv;
		if(Lv<0)
			Lv =0;
		if(Lv>9)
			Lv =9;


		PQA_Input_Level[I_006_SNR_Mode1] = Lv;
		PQA_Input_rate[I_006_SNR_Mode1] = 0;


		printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
		print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
		//for debug.

		if((printf_Flag&_BIT4)!=0 && (MA_print_count%print_delay==0))
		{
			rtd_pr_vpq_isr_info("\n LV=%d(%d-%d+%d), apl_Idx=%d,",Lv,Level_Info.Mad_lv,Level_Info.Motion_lv,Apl_Lv,apl_Idx);
		}


#endif

	}


void scalerVIP_compute_level_I_006_SNR_Mode2(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table) // TV006 mode3 for HD SNR
{

	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned int temp = 0;
	static unsigned int pre_temp = 0;
	static unsigned int pre_RTNR_MAD_count_Y_avg = 0;
	static unsigned int pre_RTNR_MAD_count_Y2_avg = 0;
	static unsigned int pre_RTNR_MAD_count_Y3_avg = 0;

	pre_RTNR_MAD_count_Y_avg = (RTNR_MAD_count_Y_avg+pre_RTNR_MAD_count_Y_avg*3)>>2;
	pre_RTNR_MAD_count_Y2_avg = (RTNR_MAD_count_Y2_avg+pre_RTNR_MAD_count_Y2_avg*3)>>2;
	pre_RTNR_MAD_count_Y3_avg = (RTNR_MAD_count_Y3_avg+pre_RTNR_MAD_count_Y3_avg*3)>>2;

	temp = ((pre_RTNR_MAD_count_Y_avg)-((pre_RTNR_MAD_count_Y2_avg+pre_RTNR_MAD_count_Y3_avg)>>1));
	temp = temp>255?255:temp;

	PQA_Input_rate[I_006_SNR_Mode2] = temp;
	pre_temp = PQA_Input_rate[I_006_SNR_Mode2];

	PQA_Input_Level[I_006_SNR_Mode2] = 0;
}
void scalerVIP_compute_level_I_006_SHP_Mode0(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table) // TV006 mode1 for RF CVBS RTNR
{
	//unsigned int level_Idx=0, printf_Flag=0, print_delay=200;
	unsigned char Lv=0;

	//unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	//unsigned int *pPQA_InputMAD_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_RTNR_MAD][0]);
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];

	// get spatial content noise meter status
	//unsigned int noise_status = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL);
	//unsigned int noise_relaible = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL_RELIABLE);
	//get mean histgram Y
	//unsigned char meanHistY = SmartPic_clue->Hist_Y_Mean_Value;


	unsigned char temporal_noise_level = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_TEMPORAL_NOISE_LEVEL);



	if(temporal_noise_level <=0)
		Lv = 9;
	else if(temporal_noise_level <= 1)
		Lv = 8;
	else if(temporal_noise_level <= 2)
		Lv = 7;
	else if(temporal_noise_level <= 3)
		Lv = 6;
	else if(temporal_noise_level <= 5)
		Lv = 5;
	else if(temporal_noise_level <=7)
		Lv = 4;
	else if(temporal_noise_level <= 9)
		Lv = 3;
	else if(temporal_noise_level <=11)
		Lv = 2;
	else if(temporal_noise_level <= 13)
		Lv = 1;
	else
		Lv=0;

	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = Lv;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_1] = temporal_noise_level;

	PQA_Input_Level[I_006_SHP_Mode0] = Lv;
	PQA_Input_rate[I_006_SHP_Mode0] = 0;



	//printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	//print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//for debug.
	//if(((printf_Flag&_BIT0)!=0) && (MA_print_count%print_delay==0))
	//	ROSPrintf("rm0~%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
	//	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0], SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_1],
	//	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_2], SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[I_006_RTNR_Mode2],
	//	noise_status, noise_relaible, meanHistY,motion_h_t,motion_h_m,motion_h_b,motion_v_l,motion_v_m,motion_v_r,motion);

}

unsigned int scalerVIP_Cal_Idx_Ave_I_006_Dynamic_SHP(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize1 64
	#define windowSizeBit1 6

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize1]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize1;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize1;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit1;

		idx++;
		if(idx==windowSize1)
			idx=0;
	}
	return ret;
}

void scalerVIP_compute_level_I_006_SHP_Mode2(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table) // TV006 mode2 for HD SNR
{
	unsigned int level_Idx=0, printf_Flag=0, print_delay=200;
	unsigned char Lv=0;
	unsigned char i;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_InputMAD_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_SHP_MAD][0]);
	//unsigned int *pPQA_Input_HistMeanY_Dark_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean_dark][0]);
	//unsigned int *pPQA_Input_HistMeanY_Dark_Gain_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean_dark_gain][0]);
	unsigned int tmp;

	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned int motioni[10]={0};

	unsigned short Lv_intp=0;//, sr_d;
	unsigned int motion = rtd_inl(DI_IM_DI_SI_FILM_MOTION_reg);

	Lv_intp=0;


	motioni[0] = motion;

	// for moving bamboo women, and moving building, more coring effect.
	if( (DynamicOptimizeSystem[198]==1) || (DynamicOptimizeSystem[199]==1) )
		motion = pPQA_InputMAD_TBL[PQA_I_L09];

	//	for change smoothly
	if(motion>pPQA_InputMAD_TBL[PQA_I_L09])
		motion = pPQA_InputMAD_TBL[PQA_I_L09];
	level_Idx = scalerVIP_Cal_Idx_Ave_I_006_Dynamic_SHP(motion, 0);

	if(level_Idx <= pPQA_InputMAD_TBL[0])
		Lv = 9;
	else if(level_Idx <= pPQA_InputMAD_TBL[1])
		Lv = 8;
	else if(level_Idx <= pPQA_InputMAD_TBL[2])
		Lv = 7;
	else if(level_Idx <= pPQA_InputMAD_TBL[3])
		Lv = 6;
	else if(level_Idx <= pPQA_InputMAD_TBL[4])
		Lv = 5;
	else if(level_Idx <= pPQA_InputMAD_TBL[5])
		Lv = 4;
	else if(level_Idx <= pPQA_InputMAD_TBL[6])
		Lv = 3;
	else if(level_Idx <= pPQA_InputMAD_TBL[7])
		Lv = 2;
	else if(level_Idx <= pPQA_InputMAD_TBL[8])
		Lv = 1;
	else
		Lv=0;

	if(Lv==9)
		Lv_intp =0;
	else
	{
		if(level_Idx>pPQA_InputMAD_TBL[PQA_I_L09])
			level_Idx = pPQA_InputMAD_TBL[PQA_I_L09];
		tmp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1])==0)?1:(pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1]);
		Lv_intp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-level_Idx)<<8) / tmp;
	}

	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_8] = level_Idx;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_9] = motion;

	PQA_Input_Level[I_006_SHP_Mode2] = Lv;
	PQA_Input_rate[I_006_SHP_Mode2] = Lv_intp;

	printf_Flag = system_info_structure_table->ISR_Printf_Flag.Flag0;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//for debug.
	if((printf_Flag==1) && (MA_print_count%print_delay==0))
	{
		ROSPrintf("sh2~lv=%d,LvI=%d,mo=%d,int=%d\n",
			Lv, level_Idx, motion, Lv_intp);
		ROSPrintf("mo_i=");
		for(i=0;i<10;i++)
			ROSPrintf("%d,",motioni[i]);
		ROSPrintf("\n");
	}
}

//vvvvvvvvvvvvvvvvvvvvvvvTV_006_stylevvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvTV_006_stylevvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvTV_006_stylevvv

//vvvvvvvvvvvvvvvvvvvvvvv simple mode vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv simple mode vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv simple mode vvv
void scalerVIP_compute_level_I_SPM_LV_CTRL0(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table) // TV006 mode2 for HD SNR
{
	unsigned int printf_Flag=0, print_delay=200;

	unsigned int current_Signal_Status, MAD_still_idx, FMV_still_idx, MAD_noise_index, wei_sum;
	unsigned char meanHistY, FMV_C, VD_Lv, MAD_Noise_Lv, MAD_Still_Lv, FMV_Still_Lv, Hist_Mean_Y_Lv, i, Lv = 0;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_Input_SPM_VD_status_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_SPM_VD_status][0]);
	unsigned int *pPQA_Input_SPM_MAD_Noise_Index_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_SPM_MAD_Noise_Index][0]);
	unsigned int *pPQA_Input_SPM_MAD_Still_Index_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_SPM_MAD_Still_Index][0]);
	unsigned int *pPQA_Input_SPM_FMV_Still_Index_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_SPM_FMV_Still_Index][0]);
	unsigned int *pPQA_Input_Hist_Mean_Y_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_SPM_Hist_Mean_Y][0]);

	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	//unsigned char *PQA_RunFlag = &(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[0]);

	unsigned int  VD_status_w = pPQA_Input_SPM_VD_status_TBL[PQA_I_L09];
	unsigned int  MAD_Noise_w = pPQA_Input_SPM_MAD_Noise_Index_TBL[PQA_I_L09];
	unsigned int  MAD_Still_w = pPQA_Input_SPM_MAD_Still_Index_TBL[PQA_I_L09];
	unsigned int  FMV_Still_w = pPQA_Input_SPM_FMV_Still_Index_TBL[PQA_I_L09];
	unsigned int  Hist_Mean_Y_w = pPQA_Input_Hist_Mean_Y_TBL[PQA_I_L09];

	/* I_SPM_VD_status */
	current_Signal_Status = SmartPic_clue->VD_Signal_Status_value_avg;
	VD_Lv = PQA_I_L09;
	if (VD_status_w != 0) {
		for (i=0;i<PQA_I_L09;i++)
			if (current_Signal_Status < pPQA_Input_SPM_VD_status_TBL[i]) {
				VD_Lv = i;
				break;
			}
	}

	/* I_SPM_MAD_Noise_Index */
	MAD_noise_index = SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio;
	MAD_Noise_Lv = PQA_I_L09;
	if (MAD_Noise_w != 0) {
		for (i=0;i<PQA_I_L09;i++)
			if (MAD_noise_index < pPQA_Input_SPM_MAD_Noise_Index_TBL[i]) {
				MAD_Noise_Lv = i;
				break;
			}
	}

	/* I_SPM_MAD_Still_Index */
	MAD_still_idx = (SmartPic_clue->RTNR_MAD_count_Y4_avg_ratio>1000)?(1000):(SmartPic_clue->RTNR_MAD_count_Y4_avg_ratio);
	MAD_still_idx = 1000 - MAD_still_idx;
	MAD_Still_Lv = PQA_I_L09;
	if (MAD_Still_w != 0) {
		for (i=0;i<PQA_I_L09;i++)
			if (MAD_still_idx < pPQA_Input_SPM_MAD_Still_Index_TBL[i]) {
				MAD_Still_Lv = i;
				break;
			}
	}

	/* I_SPM_FMV_Still_Index */
	if (SmartPic_clue->GMV == 3) {
		FMV_C = FMV_Hist_Bin_Num >> 1;
		FMV_still_idx = (SmartPic_clue->FMV_hist_Ratio[FMV_C] * 10) + (SmartPic_clue->FMV_hist_Ratio[FMV_C-1] * 3) + (SmartPic_clue->FMV_hist_Ratio[FMV_C+1] * 3);
		FMV_still_idx = FMV_still_idx >>4;
	} else {
		FMV_still_idx = 0;
	}
	FMV_Still_Lv = PQA_I_L09;
	if (FMV_Still_w != 0) {
		for (i=0;i<PQA_I_L09;i++)
			if (FMV_still_idx < pPQA_Input_SPM_FMV_Still_Index_TBL[i]) {
				FMV_Still_Lv = i;
				break;
			}
	}

	/* I_SPM_Hist_Mean_Y */
	meanHistY = SmartPic_clue->Hist_Y_Mean_Value;
	Hist_Mean_Y_Lv = PQA_I_L09;
	if (Hist_Mean_Y_w != 0) {
		for (i=0;i<PQA_I_L09;i++)
			if (meanHistY < pPQA_Input_Hist_Mean_Y_TBL[i]) {
				Hist_Mean_Y_Lv = i;
				break;
			}
	}

	/* cal final lv by weighting */
	wei_sum = VD_status_w + MAD_Noise_w + MAD_Still_w + FMV_Still_w + Hist_Mean_Y_w;
	if(wei_sum!=0){
		Lv = ((VD_status_w * VD_Lv) + (MAD_Noise_w * MAD_Noise_Lv) + (MAD_Still_w * MAD_Still_Lv) + (FMV_Still_w * FMV_Still_Lv) + (Hist_Mean_Y_w * Hist_Mean_Y_Lv)) / wei_sum;
		PQA_Input_Level[I_SPM_LV_CTRL0] = Lv;
		PQA_Input_rate[I_SPM_LV_CTRL0] = 0;
	}

	/* info recording*/
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_SPM_LvIdx_VD] = current_Signal_Status;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_SPM_LvIdx_MAD_Noise] = MAD_noise_index;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_SPM_LvIdx_MAD_Still] = MAD_still_idx;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_SPM_LvIdx_FMV_Still] = FMV_still_idx;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_SPM_LvIdx_Y] = meanHistY;

#if 1
	//for debug.
	if(((printf_Flag&_BIT0)!=0))
	{
		if(MA_print_count%print_delay==0)
		{
			rtd_pr_vpq_isr_info("Cal_SPM, current_Signal_Status=%d, MAD_noise_index=%d, MAD_still_idx=%d, FMV_still_idx=%d, meanHistY=%d\n",
				current_Signal_Status, MAD_noise_index, MAD_still_idx, FMV_still_idx, meanHistY);
			rtd_pr_vpq_isr_info("Cal_SPM, VD_status_w=%d, MAD_Noise_w=%d, MAD_Still_w=%d, FMV_Still_w=%d, Hist_Mean_Y_w=%d\n",
				VD_status_w, MAD_Noise_w, MAD_Still_w, FMV_Still_w, Hist_Mean_Y_w);
			rtd_pr_vpq_isr_info("Cal_SPM, VD_Lv=%d, MAD_Noise_Lv=%d, MAD_Still_Lv=%d, FMV_Still_Lv=%d, Hist_Mean_Y_Lv=%d, Lv=%d\n",
				VD_Lv, MAD_Noise_Lv, MAD_Still_Lv, FMV_Still_Lv, Hist_Mean_Y_Lv, Lv);
		}
	}
#endif

}
//vvvvvvvvvvvvvvvvvvvvvvv simple mode vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv simple mode vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv simple mode vvv

//=====================================================================================================================

void scalerVIP_PQA_level_Ctrl(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table)
{
	unsigned char i;

	for(i=0;i<PQA_INPUT_TYPE_MAX;i++)
	{
		if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type_setFlag[i]==1)
			system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[i] = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[i];
	}

	for(i=0;i<PQA_INPUT_ITEM_MAX;i++)
	{
		if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item_setFlag[i]==1)
		{
			SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[i] = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item[i];
			SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[i] = 0;
		}
	}

#if 0
	if(MA_print_count%20==0)
	{
		for(i=0;i<PQA_INPUT_TYPE_MAX;i++)
			ROSPrintf("%d,", system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[i]);
		ROSPrintf("\n");
		for(i=0;i<PQA_INPUT_TYPE_MAX;i++)
			ROSPrintf("%d,", system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[i]);
		ROSPrintf("\n");
	}
#endif
}

void scalerVIP_compute_PQA_level(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table)
{
	unsigned int *pPQA_InputTable = &(vipTable_shareMem->PQA_Input_Table[0][0][0]);
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;

//vvvvvvvvvvvvvvvvvvvvvvvTV_005_stylevvvvvvvvvvvvvvvvvvvvvvv
	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_005_RFCVBS]==1)
	scalerVIP_compute_level_I_005_RFCVBS_DNR(PQA_table_select, pPQA_InputTable, PQA_Input_Level, PQA_Input_rate);

	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_005_SDHD]==1)
	scalerVIP_compute_level_I_005_SDHD_DNR(PQA_table_select, pPQA_InputTable, PQA_Input_Level, PQA_Input_rate);
//^^^^^^^^^^^^^^^^^^TV_005_style^^^^^^^^^^^^^^^^^^
	if((system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_MOTION_RTNR]==1)||
	(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_MOTION_SPNR]==1))
	scalerVIP_RTNR_level_compute((SmartPic_clue->MAD_motion_status), (SmartPic_clue->block_motion_status), PQA_Input_Level, PQA_Input_rate);

//vvvvvvvvvvvvvvvvvvvvvvvTV_002_stylevvvvvvvvvvvvvvvvvvvvvvv
	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_002_RTNR_2]==1)
	{
		//scalerVIP_compute_level_I_002_RTNR2(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV002 mode2 for HD RTNR
		//scalerVIP_compute_level_I_002_CorrectionBit(vipTable_shareMem, SmartPic_clue, system_info_structure_table);
		Scaler_scalerVIP_compute_level_I_002_RTNR2_tv002(vipTable_shareMem, SmartPic_clue, system_info_structure_table, MA_print_count);
	}
	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_002_RTNR_0]==1)
		Scaler_scalerVIP_compute_level_I_002_RTNR0_tv002(vipTable_shareMem, SmartPic_clue, system_info_structure_table, MA_print_count);
//^^^^^^^^^^^^^^^^^^TV_002_style^^^^^^^^^^^^^^^^^^

//vvvvvvvvvvvvvvvvvvvvvvvTV_003_stylevvvvvvvvvvvvvvvvvvvvvvv
	if((system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_003_RTNR_TH1]==1)||
	(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_003_RTNR_TH2]==1)||
	(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_003_RTNR_TH3]==1)||
	(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_003_RTNR_TH4]==1)||
	(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_003_RTNR_TH5]==1)||
	(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_003_RTNR_TH6]==1)||
	(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_003_RTNR_TH7]==1))
	{
	scalerVIP_TV003_MAD_Y(PQA_Input_Level, PQA_Input_rate);
	}

//^^^^^^^^^^^^^^^^^^TV_003_style^^^^^^^^^^^^^^^^^^

//=====================================================================================================================
//vvvvvvvvvvvvvvvvvvvvvvvTV_006_stylevvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvTV_006_stylevvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvTV_006_stylevvv
	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_006_RTNR_Mode0]==1)
		scalerVIP_compute_level_I_006_RTNR_Mode0(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV006 mode0 for RF CVBS RTNR
	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_006_RTNR_Mode1]==1)
		scalerVIP_compute_level_I_006_RTNR_Mode1(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV006 mode1 for SD RTNR
	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_006_RTNR_Mode2]==1)
		scalerVIP_compute_level_I_006_RTNR_Mode2(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV006 mode2 for HD RTNR
	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_006_RTNR_Mode3]==1)
		scalerVIP_compute_level_I_006_RTNR_Mode3(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV006 mode4  for Progressive mode RTNR
	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_006_RTNR_Mode4]==1)
		scalerVIP_compute_level_I_006_RTNR_Mode4(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV006 mode4  for Progressive mode RTNR

	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_006_RTNR_Mode2_HMCNR]==1)
		scalerVIP_compute_level_I_006_RTNR_Mode2_HMCNR(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV006 mode2 for HD hmcnr



	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_006_SNR_Mode0]==1)
		scalerVIP_compute_level_I_006_SNR_Mode0(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV006 mode0 for RF CVBS SNR
	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_006_SNR_Mode1]==1)
		scalerVIP_compute_level_I_006_SNR_Mode1(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV006 mode1 for SD SNR
	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_006_SNR_Mode2]==1)
		scalerVIP_compute_level_I_006_SNR_Mode2(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV006 mode2 for HD SNR
	//dynamic shp
	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_006_SHP_Mode0]==1)
		scalerVIP_compute_level_I_006_SHP_Mode0(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV006 mode0 for NTSC SHP
	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_006_SHP_Mode2]==1)
		scalerVIP_compute_level_I_006_SHP_Mode2(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV006 mode2 for HD SNR


//vvvvvvvvvvvvvvvvvvvvvvvTV_006_stylevvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvTV_006_stylevvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvTV_006_stylevvv

//vvvvvvvvvvvvvvvvvvvvvvv simple mode vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv simple mode vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv simple mode vvv

	if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[I_SPM_LV_CTRL0]==1)
		scalerVIP_compute_level_I_SPM_LV_CTRL0(vipTable_shareMem, SmartPic_clue, system_info_structure_table); // TV006 mode2 for HD SNR

//vvvvvvvvvvvvvvvvvvvvvvv simple mode vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv simple mode vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv simple mode vvv

//=====================================================================================================================
}

void scalerVIP_PQ_Adaptive_Table(unsigned char mode, SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table)
{
	unsigned short PQA_ITEM_count, PQA_ITEM_count_offset;
	unsigned int PQA_row, PQA_row_offset;

	unsigned int PQA_Reg, PQA_Reg_offset, and_reg, offset_value;
	unsigned char PQA_Bitup, PQA_Bitup_offset, PQA_Bitlow, PQA_Bitlow_offset, PQA_Input_type, PQA_Input_type_offset, PQA_Input_item, PQA_Input_item_offset;
	unsigned int Level_low, Level_high, value = 0;

	unsigned int *g_PQA_Table = &(vipTable_shareMem->PQA_Table[0][0][0][0]);
	unsigned short *PQA_Input_Level = &(SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0]);
	unsigned short *PQA_Input_rate = &(SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0]);
	unsigned int *PQA_TABLE_OFFSET_TEMP = &(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.OFFSET_TEMP[0]);

	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int NR_Table = ((PQA_table_select*PQA_MODE_MAX*PQA_ITEM_MAX*PQA_LEVEL_MAX) + (mode*PQA_ITEM_MAX*PQA_LEVEL_MAX));

	unsigned char i=0;
	unsigned char idx_tmp=0;

	if(g_PQA_Table == NULL ||
		PQA_TABLE_OFFSET_TEMP == NULL ||
		PQA_Input_Level == NULL ||
		PQA_Input_rate == NULL)
	{
		if(MA_print_count%360==0)
			ROSPrintf("PQ data ERROR, PQA_TABLE is NULL!!\n");
		return;
	}

	// tool ctrl for input item and input type
	scalerVIP_PQA_level_Ctrl(vipTable_shareMem,  SmartPic_clue,  system_info_structure_table);

	// sort tmp ini
	for(i=0;i<PQA_ITEM_MAX;i++)
		PQA_Sort_Tmp[0][i] = 0xffffffff;

	for(PQA_ITEM_count = 0; PQA_ITEM_count < PQA_ITEM_MAX; PQA_ITEM_count++)
	{
		//=======================================================================
		PQA_row = NR_Table + (PQA_ITEM_count*PQA_LEVEL_MAX);

		//=======================================================================
		PQA_Reg = *(g_PQA_Table+PQA_row+PQA_reg);
		PQA_Bitup = *(g_PQA_Table+PQA_row+PQA_bitup);
		PQA_Bitlow = *(g_PQA_Table+PQA_row+PQA_bitlow);
		PQA_Input_type = *(g_PQA_Table+PQA_row+PQA_input_type);
		PQA_Input_item = *(g_PQA_Table+PQA_row+PQA_input_item);

		//=======================================================================
		if(PQA_Reg == 0xffffffff || PQA_ITEM_count >= PQA_ITEM_MAX)						//Data Finish
			break;

//		if(PQA_RUN[PQA_Input_type] == 0)														//continue this data
		if(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[PQA_Input_type] == 0 || !scalerVIP_CheckRegisterAccessProtect(PQA_Reg))														//continue this data
			continue;

		//=======================================================================

		//=======================================================================
		for(i=0;i<PQA_ITEM_MAX;i++)
		{
			if(PQA_Reg == PQA_Sort_Tmp [0][i])
			{
				idx_tmp = i;
				break;
			}
			else if(PQA_Sort_Tmp [0][i] == 0xffffffff && ( PQA_Reg >= 0xb8000000 && PQA_Reg <= 0xb8ffffff ))
			{
				PQA_Sort_Tmp[0][i]=PQA_Reg ;
                if(( PQA_Sort_Tmp[0][i] >= 0xb8019000 ) && ( PQA_Sort_Tmp[0][i] <= 0xb8019ffc) )
                {
                    PQA_Sort_Tmp[1][i]=VDPQ_rtd_inl(PQA_Reg) ;
                }
                else
                {
				PQA_Sort_Tmp[1][i]=rtd_inl(PQA_Reg) ;
                }
				idx_tmp = i;
				break;
			}
		}

		//=======================================================================

		// set rtnrC running flag for scalerVideo.c "void LG_XCRemove_by_RTNR_C(unsigned char format, unsigned char MA_motion)", elieli
		scalerVIP_PQA_TBL_ContentCheck(PQA_Reg, &(SmartPic_clue->PQA_ISR_Cal_Info.ContentCheck_Flag[0]));

		if(PQA_Input_Level[PQA_Input_item] < 9)
		{
			Level_low = *(g_PQA_Table+PQA_row+PQA_L00+PQA_Input_Level[PQA_Input_item]);
			Level_high = *(g_PQA_Table+PQA_row+PQA_L00+PQA_Input_Level[PQA_Input_item]+1);
			value = (((Level_high - Level_low)*PQA_Input_rate[PQA_Input_item])>>8) + Level_low;
		}
		else
			value = *(g_PQA_Table+PQA_row+PQA_L09);

		if(mode == PQA_MODE_WRITE)		//WRITE + OFFSET
		{
			for(PQA_ITEM_count_offset = 0; PQA_ITEM_count_offset < PQA_ITEM_MAX; PQA_ITEM_count_offset++)
			{
				//=======================================================================
				PQA_row_offset = ((PQA_table_select*PQA_MODE_MAX*PQA_ITEM_MAX*PQA_LEVEL_MAX) + (PQA_MODE_OFFSET*PQA_ITEM_MAX*PQA_LEVEL_MAX)) + (PQA_ITEM_count_offset*PQA_LEVEL_MAX);

				//=======================================================================
				PQA_Reg_offset = *(g_PQA_Table+PQA_row_offset+PQA_reg);
				PQA_Bitup_offset = *(g_PQA_Table+PQA_row_offset+PQA_bitup);
				PQA_Bitlow_offset = *(g_PQA_Table+PQA_row_offset+PQA_bitlow);
				PQA_Input_type_offset = *(g_PQA_Table+PQA_row_offset+PQA_input_type);
				PQA_Input_item_offset = *(g_PQA_Table+PQA_row_offset+PQA_input_item);
				//=======================================================================
				if(PQA_Reg_offset == 0xffffffff || PQA_ITEM_count_offset >= PQA_ITEM_MAX)						//Data Finish
					break;

				if((PQA_Reg == PQA_Reg_offset)&&(PQA_Bitup == PQA_Bitup_offset)&&(PQA_Bitlow == PQA_Bitlow_offset)&&(PQA_Input_type==PQA_Input_type_offset))
				{
					PQA_TABLE_OFFSET_TEMP[PQA_ITEM_count_offset] = value;
					break;
				}
			}
		}

		if(mode == PQA_MODE_OFFSET)															//offset function
		{
			value = (value + PQA_TABLE_OFFSET_TEMP[PQA_ITEM_count]);
			if(value >  andMASK_Table2[31])								//if sub < 0
				value = 0;
			else if(value >  andMASK_Table2[(PQA_Bitup-PQA_Bitlow+1)])		//if > bit num
				value = andMASK_Table2[(PQA_Bitup-PQA_Bitlow+1)];
		}

		if(value > (1<<(PQA_Bitup-PQA_Bitlow)))												//max value
			value = value & andMASK_Table2[(PQA_Bitup-PQA_Bitlow+1)];

		//=======================================================================
		offset_value = value<<PQA_Bitlow;													//Read Level
		and_reg = ((andMASK_Table1[(PQA_Bitup-PQA_Bitlow)]<<PQA_Bitlow)+andMASK_Table2[PQA_Bitlow]);

		//=======================================================================

		// write to tmp buffer
		PQA_Sort_Tmp[1][idx_tmp] = ((PQA_Sort_Tmp[1][idx_tmp] & (and_reg)) | (offset_value));

		//rtdf_maskl(PQA_Reg, and_reg, offset_value);								//Write Data
		//if (scalerVIP_CheckRegisterAccessProtect(PQA_Reg))
			//rtd_outl(PQA_Reg, ((rtd_inl(PQA_Reg) & (and_reg)) | (offset_value)));

	}

	for(i=0;i<PQA_ITEM_MAX;i++)
	{
		if(PQA_Sort_Tmp[0][i]==0xffffffff)
			break;
		if( ( PQA_Sort_Tmp[0][i] >= 0xb8019000 ) && ( PQA_Sort_Tmp[0][i] <= 0xb8019ffc ) ) { /*AVD flag change and protection*/
			VDPQ_rtd_outl(PQA_Sort_Tmp[0][i], PQA_Sort_Tmp[1][i]);
		} else {
			rtd_outl(PQA_Sort_Tmp[0][i], PQA_Sort_Tmp[1][i]);
		}
	}
}

void scalerVIP_Reset_PQA_TBL_ContentCheck_Flag(unsigned char* ContentCheck_Flag)
{
	unsigned char i;
	for(i=0;i<PQA_CC_MAX;i++)
		ContentCheck_Flag[i] = 0;
}

void scalerVIP_PQA_TBL_ContentCheck(unsigned int PQA_Reg, unsigned char* ContentCheck_Flag)
{
	switch (PQA_Reg)
	{
		case DI_IM_DI_RTNR_C_TH0_TH3_reg:
			ContentCheck_Flag[PQA_CC_RTNR_C_TH]=1;
			break;
		case DI_IM_DI_RTNR_C_TH4_TH6_reg:
			ContentCheck_Flag[PQA_CC_RTNR_C_TH]=1;
			break;
		case DI_IM_DI_RTNR_C_K_reg:
			ContentCheck_Flag[PQA_CC_RTNR_C_K]=1;
			break;
		default:
			break;
	}
}

// Sirius new function: Histogram section for high precision
/****** NOTE: Data is not valid until next frame sync!!! ******/
void scalerVIP_Histogram_Section( unsigned char select1, unsigned char select2 )
{
	histogram_ich1_c0_sectionhist_in_cselect_RBUS histogram_ich1_c0_sectionhist_in_cselect_RBUS_reg;

	histogram_ich1_c0_sectionhist_in_cselect_RBUS_reg.regValue = rtd_inl(HISTOGRAM_ICH1_C0_SectionHist_IN_CSELECT_reg);

	histogram_ich1_c0_sectionhist_in_cselect_RBUS_reg.ch1_c0_section0_bin_sel = select1;
	histogram_ich1_c0_sectionhist_in_cselect_RBUS_reg.ch1_c0_section1_bin_sel = select2;

	rtd_outl(HISTOGRAM_ICH1_C0_SectionHist_IN_CSELECT_reg, histogram_ich1_c0_sectionhist_in_cselect_RBUS_reg.regValue);
}

SLR_VIP_TABLE* scaler_GetShare_Memory_VIP_TABLE_Struct(void)
{
	if(g_Share_Memory_VIP_TABLE_Struct_isr==NULL)
	{
		if(MA_print_count%360==0)
		{
			ROSPrintf("g_Share_Memory_VIP_TABLE_Struct_isr NULL \n");
		}
		return NULL;
	}
	return g_Share_Memory_VIP_TABLE_Struct_isr;
}

_clues* scaler_GetShare_Memory_SmartPic_Clue_Struct(void)
{
	if(SmartPic_clue==NULL)
	{
		if(MA_print_count%360==0)
		{
			ROSPrintf("Share_Memory_SmartPic_clue_Struct NULL \n");
		}
		return NULL;
	}
	return SmartPic_clue;
}

_RPC_clues* scaler_GetShare_Memory_RPC_SmartPic_Clue_Struct(void)
{
	if(RPC_SmartPic_clue==NULL)
	{
		if(MA_print_count%360==0)
		{
			ROSPrintf("Share_Memory_SmartPic_clue_Struct NULL \n");
		}
		return NULL;
	}
	return RPC_SmartPic_clue;
}


_system_setting_info* scaler_GetShare_Memory_system_setting_info_Struct(void)
{
	if(system_info_structure_table==NULL)
	{
		if(MA_print_count%360 == 0)
		{
		//	ROSPrintf("system_info_structure_table NULL \n");
		}
		return NULL;
	}
	return system_info_structure_table;
}

_RPC_system_setting_info* scaler_GetShare_Memory_RPC_system_setting_info_Struct(void)
{
	if(RPC_system_info_structure_table==NULL)
	{
		if(MA_print_count%360 == 0)
		{
		//	ROSPrintf("system_info_structure_table NULL \n");
		}
		return NULL;
	}
	return RPC_system_info_structure_table;
}

void fwif_color_PrintArray_UINT32(unsigned int * pwTemp ,int nSize,int nID)
{
	int i;
	if (pwTemp) {
		for (i = 0; i < (nSize); i++)
			ROSPrintf("Share_Memory_Video_VIP_TABLE, Array ID%d [%d]=%d\n ",nID,i,pwTemp[i]);
	}
}
void fwif_color_PrintArray_INT32(int * pwTemp ,int nSize,int nID)
{
	int i;
	if (pwTemp) {
	for (i = 0; i < (nSize); i++)
		ROSPrintf("Share_Memory_Video_VIP_TABLE, Array ID%d [%d]=%d\n ",nID,i,pwTemp[i]);
	}
}
void fwif_color_PrintArray_UINT16(unsigned short * pwTemp ,int nSize,int nID)
{
	int i;
	if (pwTemp) {
	for (i = 0; i < (nSize); i++)
		ROSPrintf("Share_Memory_Video_VIP_TABLE, Array ID%d [%d]=%x\n ",nID,i,pwTemp[i]);
	}
}

void fwif_color_PrintArray_INT16(short * pwTemp ,int nSize,int nID)
{
	int i;
	if (pwTemp) {
	for (i = 0; i < (nSize); i++)
		ROSPrintf("Share_Memory_Video_VIP_TABLE, Array ID%d [%d]=%d\n ",nID,i,pwTemp[i]);
	}
}
void fwif_color_PrintArray_UINT8(unsigned char * pwTemp ,int nSize,int nID)
{
	int i;
	if (pwTemp) {
	for (i = 0; i < (nSize); i++)
		ROSPrintf("Share_Memory_Video_VIP_TABLE, Array ID%d [%d]=%d\n ",nID,i,pwTemp[i]);
	}
}

void scalerVIP_Share_Memory_Access_VIP_TABLE_Custom_Struct(unsigned long arg)
{
	g_Share_Memory_VIP_TABLE_Custom_Struct_isr= ((SLR_VIP_TABLE_CUSTOM_TV001 *)arg);
	//ROSPrintf("==  scalerVIP_Share_Memory_Access_VIP_TABLE_Custom_Struct==\n ");
}

void* scalerVIP_Get_Share_Memory_VIP_TABLE_Custom_Struct(void)
{
	if(g_Share_Memory_VIP_TABLE_Custom_Struct_isr==NULL)
	{
		if(MA_print_count%360 == 0)
		{
			ROSPrintf("VIP_TABLE_Custom_Struct NULL \n");
		}
		return NULL;
	}

	return (void*)g_Share_Memory_VIP_TABLE_Custom_Struct_isr;
}

// move to share memory access struct, elieli
/*
void scalerVIP_PQA_TABLE_OFFSET_TEMP(unsigned long arg)
{
	//ROSPrintf("enter PQA_TABLE_OFFSET_TEMP\n");
	PQA_TABLE_OFFSET_TEMP = (unsigned int*)arg;
}
*/

void scalerVIP_setDcrTable(void)
{
	SLR_VIP_TABLE* Share_Memory_VIP_TABLE_Struct = NULL;
	Share_Memory_VIP_TABLE_Struct = scaler_GetShare_Memory_VIP_TABLE_Struct();

	if(Share_Memory_VIP_TABLE_Struct == NULL) {
		if(MA_print_count%360==0) {
			ROSPrintf("\n====Share Memory PQ data ERROR====\n\n");
			ROSPrintf(" Mem0 = %p\n",Share_Memory_VIP_TABLE_Struct);
			ROSPrintf("\n===============================\n\n");
		}
		return;
	}

	DCR_TABLE = (unsigned char *)&Share_Memory_VIP_TABLE_Struct->DCR_TABLE[0][0];
#ifdef DCR_debug_en
	if(MA_print_count%250==0){
		rtd_pr_vpq_isr_notice("scalerVIP DCR %d	%d	%d	%d	%d\n", DCR_TABLE[0],DCR_TABLE[1],DCR_TABLE[2],DCR_TABLE[3],DCR_TABLE[4] );
		rtd_pr_vpq_isr_notice("scalerVIP DCR %d	%d	%d	%d	%d\n", DCR_TABLE[20],DCR_TABLE[21],DCR_TABLE[22],DCR_TABLE[23],DCR_TABLE[24] );
	}
#endif
}

void scalerVIP_Set_DCR_BackLightMappingRange(void)
{
	_system_setting_info* system_info_structure_table = NULL;

	system_info_structure_table = scaler_GetShare_Memory_system_setting_info_Struct();

	if(system_info_structure_table == NULL) {
		if(MA_print_count%360==0) {
			ROSPrintf("\n====Share Memory PQ data ERROR====\n\n");
			ROSPrintf(" Mem2 = %p\n",system_info_structure_table);
			ROSPrintf("\n===============================\n\n");
		}
		return;
	}

	scalerVIP_SetIsIncreseMode(system_info_structure_table->OSD_Info.isIncreaseMode_Flag);
	scalerVIP_SetDcrUIMaxValue(system_info_structure_table->OSD_Info.backlightLevel_uiMax);
	scalerVIP_SetDcrUIMinValue(system_info_structure_table->OSD_Info.backlightLevel_uiMin);
	scalerVIP_SetBlLvActMax(system_info_structure_table->OSD_Info.backlightLevel_actMax);
	scalerVIP_SetBlLvActMin(system_info_structure_table->OSD_Info.backlightLevel_uiMin);
	scalerVIP_SetDcrOnOff(system_info_structure_table->OSD_Info.OSD_DCR_Mode);
	scalerVIP_GetBlLvFromUser(system_info_structure_table->OSD_Info.OSD_Backlight);

}

void scalerVIP_SetIsIncreseMode(unsigned char arg)//Leo Chen 20101001
{
	m_isIncreaseMode= (unsigned int)arg;
}
void scalerVIP_SetDcrUIMaxValue(unsigned int arg)//Leo Chen 20101001
{
	m_DCR_UI_Max_Value= (unsigned int)arg;
}

void scalerVIP_SetDcrUIMinValue(unsigned int arg)//Leo Chen 20101001
{
	m_DCR_UI_Min_Value= (unsigned int)arg;
}

void scalerVIP_SetDcrOnOff(unsigned char arg)//Leo Chen 20101111
{
	m_DCR_On_Off= (unsigned char)arg;
	m_DCR_On_Off = (m_DCR_On_Off == 0) ?(0) : (1);
	/*ROSPrintf("m_DCR_On_Off=%d\n",m_DCR_On_Off);*/
}

void scalerVIP_GetBlLvFromUser(unsigned char arg)//Leo Chen 20101111
{
	m_BL_LV_From_User= (unsigned int)arg;

	m_DCR_refresh = TRUE; //Leo Chen 20101209
}

void scalerVIP_SetBlLvActMax(unsigned int arg)//Leo Chen 20101111
{
	m_BL_LV_Act_Max= (unsigned int)arg;
}

void scalerVIP_SetBlLvActMin(unsigned int arg)//Leo Chen 20101111
{
	m_BL_LV_Act_Min= (unsigned int)arg;
}

void scaler_Set_Dcr_Powersave_Enable(unsigned long  arg)
{

	//ROSPrintf("[Young]## scaler_Set_Dcr_Powersave_Enabley arg=%d@@ \n",arg);
	VIP_DCR_powersave_enable=*(unsigned char*)arg;

	//ROSPrintf("[Young]## scaler_Set_Dcr_Powersave_Enabley VIP_DCR_powersave_enable=%d@@ \n",VIP_DCR_powersave_enable);

}

//Profile RGB for ambilight sensor
#ifdef ISR_AMBILIGHT_ENABLE
void scalerVIP_Set_Profile_RGB_Data_Flag_V(unsigned long arg)
{
	scalerDrvPrint("enter scalerVIP_Set_Profile_RGB_Data_Flag_V\n");

	Profile_RGB_data_flag_V = (unsigned char) arg;

}
void scalerVIP_Get_Profile_RGB_Data_Flag_V(unsigned long arg)
{
	scalerDrvPrint("enter scalerVIP_Get_Profile_RGB_Data_Flag_V\n");

	*(unsigned char*)arg = Profile_RGB_data_flag_V;

}
void scalerVIP_Get_Profile_R_Data_V(unsigned long arg)
{
	scalerDrvPrint("enter scalerVIP_Get_Profile_R_Data_V\n");

	memcpy((unsigned short*)arg, &Profile_R_V[0], sizeof(unsigned short)*16);

}
void scalerVIP_Get_Profile_G_Data_V(unsigned long arg)
{
	scalerDrvPrint("enter scalerVIP_Get_Profile_G_Data_V\n");

	memcpy((unsigned short*)arg, &Profile_G_V[0], sizeof(unsigned short)*16);

}

void scalerVIP_Get_Profile_B_Data_V(unsigned long arg)
{
	scalerDrvPrint("enter scalerVIP_Get_Profile_B_Data_V\n");

	memcpy((unsigned short*)arg, &Profile_B_V[0], sizeof(unsigned short)*16);

}

void scalerVIP_Set_Profile_RGB_Data_Flag_H(unsigned long arg)
{
	scalerDrvPrint("enter scalerVIP_Set_Profile_RGB_Data_Flag_H\n");

	Profile_RGB_data_flag_H = (unsigned char) arg;

}
void scalerVIP_Get_Profile_RGB_Data_Flag_H(unsigned long arg)
{
	scalerDrvPrint("enter scalerVIP_Get_Profile_RGB_Data_Flag_H\n");

	*(unsigned char*)arg = Profile_RGB_data_flag_H;

}
void scalerVIP_Get_Profile_R_Data_H(unsigned long arg)
{
	scalerDrvPrint("enter scalerVIP_Get_Profile_R_Data_H\n");

	memcpy((unsigned short*)arg, &Profile_R_H[0], sizeof(unsigned short)*16);

}
void scalerVIP_Get_Profile_G_Data_H(unsigned long arg)
{
	scalerDrvPrint("enter scalerVIP_Get_Profile_G_Data_H\n");

	memcpy((unsigned short*)arg, &Profile_G_H[0], sizeof(unsigned short)*16);

}

void scalerVIP_Get_Profile_B_Data_H(unsigned long arg)
{
	scalerDrvPrint("enter scalerVIP_Get_Profile_B_Data_H\n");

	memcpy((unsigned short*)arg, &Profile_B_H[0], sizeof(unsigned short)*16);

}
#endif

unsigned char scalerVIP_Get_hpan_vfir()
{
	return h_pan_vfir_value;
}
void scalerVIP_Set_hpan_vfir(unsigned char value)
{
	if (value > 32)
		value = 32; //for clip
	h_pan_vfir_value = value;
}
void scalerVIP_Set_vfir_ctrl(unsigned char enable)
{
	iedge_smooth_vfilter_0_RBUS vfilter_reg;
	vfilter_reg.regValue = rtd_inl(IEDGE_SMOOTH_VFILTER_0_reg);

	if (enable)
		vfilter_reg.avlpf_en = 1;
	else
		vfilter_reg.avlpf_en = 0;

	rtd_outl(IEDGE_SMOOTH_VFILTER_0_reg, vfilter_reg.regValue);
}

void scalerVIP_Dynamic_Vertical_NR(void)
{
	static unsigned char pre_source =255;
	iedge_smooth_vfilter_0_RBUS iedge_smooth_vfilter_reg;
	unsigned char cur_source=(RPC_system_info_structure_table->VIP_source);
	static unsigned char ori_vnr_weight=0;
	static unsigned char ori_text_coring=0;
	color_sharp_dm_peaking_bound_1_RBUS color_sharp_dm_peaking_bound_1_reg;
	//unsigned char hpan_vfir=0,tnr_vfir=0,vfir_value;
	//unsigned char sat_th=60;
	static int vfir_weight = 0;
	vdtop_noise_status_RBUS vdtop_noise_status_reg;
	char temp=0;
	int TargetWeight=0;
	unsigned char spatial_noise_level = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL);
	//unsigned char temporal_noise_level = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_TEMPORAL_NOISE_LEVEL);

	iedge_smooth_vfilter_reg.regValue = rtd_inl(IEDGE_SMOOTH_VFILTER_0_reg);
	color_sharp_dm_peaking_bound_1_reg.regValue= rtd_inl(COLOR_SHARP_DM_PEAKING_BOUND_1_reg);

	if(pre_source !=cur_source)
	{
		ori_vnr_weight=iedge_smooth_vfilter_reg.vfir_weight;
		ori_text_coring = color_sharp_dm_peaking_bound_1_reg.lv;
	}
	pre_source = cur_source;

	temp=iedge_smooth_vfilter_reg.vfir_weight;


	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Reverse_3==TRUE))
	{
	//	ROSPrintf("SNR=%d,TNR=%d,tmp=%d\n", spatial_noise_level,temporal_noise_level,temp);
	}
	switch(cur_source)
	{
		case VIP_QUALITY_HDMI_480P:
		case VIP_QUALITY_HDMI_576P:
		case VIP_QUALITY_HDMI_720P:
		case VIP_QUALITY_HDMI_480I:
		case VIP_QUALITY_HDMI_576I:

			break;


		case VIP_QUALITY_DTV_480I:
		case VIP_QUALITY_DTV_576I:
		case VIP_QUALITY_DTV_480P:
		case VIP_QUALITY_DTV_576P:


		break;
		case VIP_QUALITY_HDMI_1080I:
		case VIP_QUALITY_HDMI_1080P:

			//rtd_outl(IEDGE_SMOOTH_VFILTER_0_reg, iedge_smooth_vfilter_reg.regValue);


		break;
		case VIP_QUALITY_CVBS_NTSC :
		{
	            static unsigned char pre_vif_weight = 0;
	            unsigned char TargetWeight = iedge_smooth_vfilter_reg.vfir_weight;

//	            if((scalerVIP_DI_MiddleWare_MCNR_Get_Pan_Flag()==1)) //mac6 no MCNR
//	                TargetWeight = 5;

	            if( scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN) >= 2 )
	                TargetWeight = 20;

	            if( pre_vif_weight < TargetWeight )
	                pre_vif_weight++;
	            else if( pre_vif_weight > TargetWeight )
	                pre_vif_weight--;
	            //else
	                //pre_vif_weight = pre_vif_weight;

	            if( pre_vif_weight > 31 )
	                pre_vif_weight = 31;
	            /*else if( pre_vif_weight < 0 ) //unsigned value is never < 0
	                pre_vif_weight = 0;*/

	            iedge_smooth_vfilter_reg.vfir_weight = pre_vif_weight;
	            rtd_outl(IEDGE_SMOOTH_VFILTER_0_reg, iedge_smooth_vfilter_reg.regValue);

	            if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Reverse_3==TRUE))
	                ROSPrintf("(NTSC) lpf target value = %d, write %d\n", TargetWeight, iedge_smooth_vfilter_reg.vfir_weight);

	      		  break;
	        }
		case VIP_QUALITY_CVBS_PAL:
		{
	            static unsigned char pre_vif_weight = 0;
	            unsigned char TargetWeight = iedge_smooth_vfilter_reg.vfir_weight;

//	            if((scalerVIP_DI_MiddleWare_MCNR_Get_Pan_Flag()==1)) //mac6 no MCNR
//	                TargetWeight = 5;

	            if( scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN) >= 2 )
	                TargetWeight = 20;

	            if( DynamicOptimizeSystem[189] == 1 )
	                TargetWeight = 10;

	            if( pre_vif_weight < TargetWeight )
	                pre_vif_weight++;
	            else if( pre_vif_weight > TargetWeight )
	                pre_vif_weight--;
	            //else
	                //pre_vif_weight = pre_vif_weight;

	            if( pre_vif_weight > 31 )
	                pre_vif_weight = 31;
	            /*else if( pre_vif_weight < 0 ) //unsigned value is never < 0
	                pre_vif_weight = 0;*/

	            iedge_smooth_vfilter_reg.vfir_weight = pre_vif_weight;
	            rtd_outl(IEDGE_SMOOTH_VFILTER_0_reg, iedge_smooth_vfilter_reg.regValue);

	            if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Reverse_3==TRUE))
	                ROSPrintf("(PAL) lpf target value = %d, write %d\n", TargetWeight, iedge_smooth_vfilter_reg.vfir_weight);

	            break;
	        }
	        case VIP_QUALITY_TV_NTSC:
	        case VIP_QUALITY_TV_PAL:
	        case VIP_QUALITY_TV_PAL_M:
	        case VIP_QUALITY_TV_SECAN:
	        case VIP_QUALITY_TV_NTSC443:
	        {

	            vdtop_noise_status_reg.regValue = VDPQ_rtd_inl(VDTOP_NOISE_STATUS_reg);
	            TargetWeight = 20 + vdtop_noise_status_reg.status_noise + spatial_noise_level;

	            if( TargetWeight > vfir_weight )
	                vfir_weight++;
	            else if( TargetWeight < vfir_weight )
	                vfir_weight--;
	            //else
	                //vfir_weight = vfir_weight;

	            if( vfir_weight > 26 )
	                vfir_weight = 26;
	            else if( vfir_weight < 0 )
	                vfir_weight = 0;

	            iedge_smooth_vfilter_reg.vfir_weight = vfir_weight;
	            rtd_outl(IEDGE_SMOOTH_VFILTER_0_reg, iedge_smooth_vfilter_reg.regValue);
	            break;
	        }
		default:
			break;
	}


}
void scalerVIP_BTR_Detecte(void)
{
	static unsigned char pre_source =255;
	static unsigned char ori_btr=0;
	unsigned char cur_source=0;
	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;
	di_im_di_btr_control_RBUS BTR_Control_Reg;
	unsigned char motionl_level = scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION);
	BTR_Control_Reg.regValue=rtd_inl(DI_IM_DI_BTR_CONTROL_reg);

	RPC_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();

	if(RPC_system_info_structure_table == NULL || SmartPic_clue ==NULL){
		if(MA_print_count%360==0)
			ROSPrintf("scalerVIP_BTR_Detecte Share Memory PQ data ERROR, system_info = %p,SmartPic_clue=%p \n",RPC_system_info_structure_table, SmartPic_clue);
		return;
	}
	cur_source=(RPC_system_info_structure_table->VIP_source);

	if(pre_source !=cur_source)
	{
		ori_btr=BTR_Control_Reg.btr_blendtemporalrecoveryen;
	}
	pre_source = cur_source;

	//ROSPrintf("motionl_level=%d \n", motionl_level);

	switch(cur_source)
	{
		case VIP_QUALITY_CVBS_NTSC:
		case VIP_QUALITY_CVBS_PAL:
		case VIP_QUALITY_CVBS_S_NTSC:
		case VIP_QUALITY_CVBS_S_PAL:
		case VIP_QUALITY_TV_NTSC:
		case VIP_QUALITY_TV_PAL:
		case VIP_QUALITY_YPbPr_480I:
		case VIP_QUALITY_YPbPr_576I:
		case VIP_QUALITY_HDMI_480I:
		case VIP_QUALITY_HDMI_576I:
		case VIP_QUALITY_DTV_480I:
		case VIP_QUALITY_DTV_576I:
		case VIP_QUALITY_CVBS_SECAN:
		case VIP_QUALITY_CVBS_S_SECAN:
		case VIP_QUALITY_TV_SECAN:
		case VIP_QUALITY_SCART_NTSC:
		case VIP_QUALITY_SCART_PAL:
		case VIP_QUALITY_SCART_SECAN:
		case VIP_QUALITY_SCART_RGB_NTSC:
		case VIP_QUALITY_SCART_RGB_PAL:
		case VIP_QUALITY_SCART_RGB_SECAN:
		case VIP_QUALITY_CVBS_PAL_M:
		case VIP_QUALITY_CVBS_S_PAL_M:
		case VIP_QUALITY_TV_PAL_M:
		case VIP_QUALITY_TV_NTSC443:
			if(((motionl_level<4)||(SmartPic_clue->SceneChange_NR))&&ori_btr==1)
			{
				BTR_Control_Reg.btr_blendtemporalrecoveryen=0;
				rtd_outl(DI_IM_DI_BTR_CONTROL_reg, BTR_Control_Reg.regValue);
			}
			else
			{
				BTR_Control_Reg.btr_blendtemporalrecoveryen=1;
				rtd_outl(DI_IM_DI_BTR_CONTROL_reg, BTR_Control_Reg.regValue);
			}

		break;
	default:

		break;
	}

	#if 0	//20150724 roger mark, we have new method to avoid fall into film22
	if(system_info_structure_table->Project_ID == VIP_Project_ID_TV006){
		if (cur_source == VIP_QUALITY_HDMI_1080I){
		di_im_di_control_RBUS di_control_reg;
		di_control_reg.regValue = rtd_inl(DI_IM_DI_CONTROL_reg);
			if (motionl_level <= 15){ // global motion less then 15 disable 22 film mode
			di_control_reg.film_enable = di_control_reg.film_enable & 0x1B; //11011
			}else {
			di_control_reg.film_enable = 0x1C;// 11100
		}
		rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);
	}
	}
	#endif
}

#ifdef CONFIG_HW_SUPPORT_I_DE_XC
void scalerVIP_Dynamic_DeXCXL_CTRL(_RPC_system_setting_info *VIP_RPC_system_info_structure_table, SLR_VIP_TABLE* Share_VIP_TABLE_Struct, _RPC_clues* RPC_SmartPic_clue)
{
	static unsigned char pre_force_disable = 255;
	unsigned char force_disable = 0;
	unsigned char de_xc_tbl_sel = 0;
	unsigned char vip_src;
	int mSpeed;
	int frmrate;

	de_xcxl_de_xcxl_ctrl_RBUS de_xcxl_de_xcxl_ctrl_reg;
	de_xcxl_de_xcxl_db_reg_ctl_RBUS de_xcxl_de_xcxl_db_reg_ctl_reg;

	if((RPC_SmartPic_clue == NULL) ||(VIP_RPC_system_info_structure_table == NULL) || (Share_VIP_TABLE_Struct == NULL)) {
		ROS_XCprintf_Info("scalerVIP_Dynamic_DeXCXL_CTRL DISABLED! VIP_RPC_system_info_structure_table = %p, Share_VIP_TABLE_Struct = %p, RPC_SmartPic_clue = %p,\n",
				VIP_RPC_system_info_structure_table, Share_VIP_TABLE_Struct, RPC_SmartPic_clue);
		return;
	}

	vip_src = VIP_RPC_system_info_structure_table->VIP_source;
	mSpeed = Scaler_ChangeUINT32Endian( RPC_SmartPic_clue->MEMC_VCPU_setting_info.vo_channel_speed );
	frmrate = Scaler_VOInfoPointer(0)->v_freq;
	de_xcxl_de_xcxl_ctrl_reg.regValue = rtd_inl(DE_XCXL_De_XCXL_CTRL_reg);

	// disable DeXCXL if playback speed is not 1x, MEMC film and pan
	if((mSpeed != 256) || (sat_ratio_mean_value <=35)  || (pattern_gen_pan_flag == 1))
		force_disable = 1;
	else
		force_disable = 0;

	if (pre_force_disable != force_disable) {
		pre_force_disable = force_disable;
		de_xcxl_de_xcxl_ctrl_reg.dexc_en = 0;
		if (force_disable == 0 && drv_memory_Get_I_De_XC_Mem_Ready_Flag(0) == 1) {
			de_xc_tbl_sel = Share_VIP_TABLE_Struct->VIP_QUALITY_Extend_Coef[vip_src][VIP_QUALITY_FUNCTION_I_De_XC];
			if(de_xc_tbl_sel >= I_De_XC_TBL_Max)
				de_xc_tbl_sel =0;
			de_xcxl_de_xcxl_ctrl_reg.dexc_en = I_De_XC_TBL[de_xc_tbl_sel].dexc_en;
		}
	}

	// dexc mode auto control, only enable PAL mode @25 or 50Hz
	if((frmrate >= 248 && frmrate <= 252) ||( (frmrate >= 498) && (frmrate <= 502)))
	{
		de_xcxl_de_xcxl_ctrl_reg.dexc_detect_type = 2; // Both NTSC & PAL
	}
	else
	{
		de_xcxl_de_xcxl_ctrl_reg.dexc_detect_type = 0; // NTSC only
	}

	ROS_XCprintf_Info("[scalerVIP_Dynamic_DeXCXL_CTRL] Info: VO channel speed = %d, Frame rate = %d\n", mSpeed, frmrate );
	ROS_XCprintf_Info("[scalerVIP_Dynamic_DeXCXL_CTRL]       sat_ratio_mean_value = %d, pan_flag = %d\n", sat_ratio_mean_value, pattern_gen_pan_flag );
	ROS_XCprintf_Info("[scalerVIP_Dynamic_DeXCXL_CTRL] Apply: DeXC enable = %d, DeXC force disable = %d, Detect type = %d\n", de_xcxl_de_xcxl_ctrl_reg.dexc_en, force_disable, de_xcxl_de_xcxl_ctrl_reg.dexc_detect_type );

	rtd_outl( DE_XCXL_De_XCXL_CTRL_reg, de_xcxl_de_xcxl_ctrl_reg.regValue );
	// db apply
	de_xcxl_de_xcxl_db_reg_ctl_reg.regValue = rtd_inl(DE_XCXL_DE_XCXL_db_reg_ctl_reg);
	de_xcxl_de_xcxl_db_reg_ctl_reg.db_apply = 1;
	rtd_outl( DE_XCXL_DE_XCXL_db_reg_ctl_reg, de_xcxl_de_xcxl_db_reg_ctl_reg.regValue );

}
#endif //CONFIG_HW_SUPPORT_I_DE_XC

#ifdef CONFIG_HW_SUPPORT_MEMC
void scalerVIP_IP_OnOff_By_MEMC_Film(_RPC_system_setting_info* system_RPC_info_structure_table) //rord.tsao 2016/0325
{
	// when content is film need disable smd.
	// some time film detected not correct in pq block,used memc film flag to disable.

	di_di_smd_additionalcoeff1_RBUS init_di_smd_additionalcoeff1_reg;
	di_color_recovery_option_RBUS Color_Recovery_Option_Reg;
	di_im_di_pan_detection_control_1_RBUS di_pan_detection_control_1_reg;
	di_im_di_hfd_ma_error_recovery_RBUS di_im_di_hfd_ma_error_recovery_reg;
	//de_xcxl_de_xcxl_blend_RBUS de_xcxl_de_xcxl_blend_reg;
	//de_xcxl_de_xcxl_ctrl_RBUS de_xcxl_de_xcxl_ctrl_reg;

	//unsigned char de_xc_tbl_sel = 0;
	static unsigned char isFilm = 40;

	// -----------------------------------------------------------------------------
	// Determin is Film or not

	// Film information from MEMC
	unsigned char *FilmInfo = Scaler_MEMC_GetCadence();
	if( FilmInfo == NULL )
	{
		if( MA_print_count%120 == 0)
			ROSPrintf("[%s] Film information ptr from MEMC error\n", __func__);
		return;
	}

	// Debounce
	if( FilmInfo[_FILM_MEMC_OUT_ALL] == 0 && isFilm > 0 )
		isFilm--;
	else if( FilmInfo[_FILM_MEMC_OUT_ALL] != 0 )
		isFilm = 40;

	// -----------------------------------------------------------------------------

	init_di_smd_additionalcoeff1_reg.regValue= rtd_inl(DI_DI_SMD_AdditionalCoeff1_reg);
	Color_Recovery_Option_Reg.regValue = rtd_inl(DI_Color_Recovery_Option_reg);
	if((DynamicOptimizeSystem[174]==1)&&(Color_Recovery_Option_Reg.game_mode_3a!=1))
	{

		if(init_di_smd_additionalcoeff1_reg.smd_result_en!=1)
		{
			di_double_buff_apply =1;
	 	}
		init_di_smd_additionalcoeff1_reg.smd_result_en =1;
		rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg, init_di_smd_additionalcoeff1_reg.regValue);

	}
	else if( (isFilm!=0) || (Color_Recovery_Option_Reg.game_mode_3a==1) )
	{
		init_di_smd_additionalcoeff1_reg.smd_result_en =0;

		if(init_di_smd_additionalcoeff1_reg.smd_result_en!=0)
		{
			di_double_buff_apply =1;
		}
		rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg, init_di_smd_additionalcoeff1_reg.regValue);

		if(system_RPC_info_structure_table->VIP_source ==VIP_QUALITY_HDMI_576I||RPC_system_info_structure_table->VIP_source ==VIP_QUALITY_HDMI_480I)
		{
			di_pan_detection_control_1_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_CONTROL_1_reg);
			di_pan_detection_control_1_reg.pan_en=0;
			rtd_outl(DI_IM_DI_PAN_DETECTION_CONTROL_1_reg,di_pan_detection_control_1_reg.regValue);
		}
	}
	else if((static_move==1)&&(Color_Recovery_Option_Reg.game_mode_3a!=1)&&(isFilm==0))
	{

		if(init_di_smd_additionalcoeff1_reg.smd_result_en!=1)
		{
			di_double_buff_apply =1;
	 	}
		init_di_smd_additionalcoeff1_reg.smd_result_en =1;
		rtd_outl(DI_DI_SMD_AdditionalCoeff1_reg, init_di_smd_additionalcoeff1_reg.regValue);

	}

	/* HMC off while film*/
	di_im_di_hfd_ma_error_recovery_reg.regValue = rtd_inl(DI_IM_DI_HFD_MA_ERROR_RECOVERY_reg);
	//if (FilmInfo[_FILM_MEMC_OUT_ALL] != 0)
	if( isFilm != 0 )
	{

		di_im_di_hfd_ma_error_recovery_reg.hmc_window_en = 1;
		di_im_di_hfd_ma_error_recovery_reg.hmc_window_top = 0;
		di_im_di_hfd_ma_error_recovery_reg.hmc_window_bot = 1023;
	}
	else
	{
		di_im_di_hfd_ma_error_recovery_reg.hmc_window_en = 0;
	}
	rtd_outl(DI_IM_DI_HFD_MA_ERROR_RECOVERY_reg, di_im_di_hfd_ma_error_recovery_reg.regValue);
#if 0	/* move to scalerVIP_Dynamic_DeXCXL_CTRL*/
	de_xcxl_de_xcxl_ctrl_reg.regValue = rtd_inl(DE_XCXL_De_XCXL_CTRL_reg);
	/* turn off de-xc while film*/
	if ((sat_ratio_mean_value <=35) && (de_xcxl_de_xcxl_ctrl_reg.dexc_debug_en != 1))
	{
		de_xcxl_de_xcxl_blend_reg.regValue = 0;
	}
	else
	{
		de_xc_tbl_sel = system_RPC_info_structure_table->I_De_XC_CTRL.table_select;
		de_xcxl_de_xcxl_blend_reg.dexc_blending_table_0 = system_RPC_info_structure_table->I_De_XC_CTRL.De_XC_TBL[de_xc_tbl_sel].dexc_blending_table_0;
		de_xcxl_de_xcxl_blend_reg.dexc_blending_table_1 = system_RPC_info_structure_table->I_De_XC_CTRL.De_XC_TBL[de_xc_tbl_sel].dexc_blending_table_1;
		de_xcxl_de_xcxl_blend_reg.dexc_blending_table_2 = system_RPC_info_structure_table->I_De_XC_CTRL.De_XC_TBL[de_xc_tbl_sel].dexc_blending_table_2;
		de_xcxl_de_xcxl_blend_reg.dexc_blending_table_3 = system_RPC_info_structure_table->I_De_XC_CTRL.De_XC_TBL[de_xc_tbl_sel].dexc_blending_table_3;
		de_xcxl_de_xcxl_blend_reg.dexc_blending_table_4 = system_RPC_info_structure_table->I_De_XC_CTRL.De_XC_TBL[de_xc_tbl_sel].dexc_blending_table_4;
		de_xcxl_de_xcxl_blend_reg.dexc_blending_table_5 = system_RPC_info_structure_table->I_De_XC_CTRL.De_XC_TBL[de_xc_tbl_sel].dexc_blending_table_5;
		de_xcxl_de_xcxl_blend_reg.dexc_blending_table_6 = system_RPC_info_structure_table->I_De_XC_CTRL.De_XC_TBL[de_xc_tbl_sel].dexc_blending_table_6;
		de_xcxl_de_xcxl_blend_reg.dexc_blending_table_7 = system_RPC_info_structure_table->I_De_XC_CTRL.De_XC_TBL[de_xc_tbl_sel].dexc_blending_table_7;
	}
	rtd_outl(DE_XCXL_De_XCXL_BLEND_reg, de_xcxl_de_xcxl_blend_reg.regValue);	/* is not db*/
#endif
	if(di_double_buff_apply ==1)
	{
		di_db_reg_ctl_RBUS  db_reg_ctl_reg;

		db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
		db_reg_ctl_reg.db_apply=1;
		rtd_outl(DI_db_reg_ctl_reg,db_reg_ctl_reg.regValue);
		di_double_buff_apply =0;
	}


}
#endif //CONFIG_HW_SUPPORT_MEMC

void scalerVIP_SMD_weake_move_Detecte(void)
{

	di_di_gst_fmv_pan_coeff2_RBUS di_di_gst_fmv_pan_coeff2;
	di_di_smd_sampling_gmvc_RBUS di_di_smd_sampling_gmvc;
	di_di_smd_gmvcoeff3_RBUS init_di_smd_gmvcoeff3_reg;
	di_di_smd_gmvcoeff1_RBUS smd_gmvcoeff1_reg;
	di_di_smd_gmvcoeff2_RBUS smd_gmvcoeff2_reg;
	di_di_smd_choosecandidate_RBUS smd_choosecandidate_reg;
	// chen 170522
//	di_im_di_zoommotion_detector_RBUS zoommotion_detector_reg;
	// end chen 170522
	di_di_smd_additionalcoeff2_RBUS init_di_smd_additionalcoeff2_reg;

	unsigned char cur_source=(RPC_system_info_structure_table->VIP_source);

	unsigned char hgmv=0,vgmv=0;
	unsigned short match_percent1=0,match_percent2=0;
	char which_source=(RPC_system_info_structure_table->VIP_source);
	unsigned char motionl_level = scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION);

	if(which_source == VIP_QUALITY_HDMI_1080I)
		return;
	di_di_gst_fmv_pan_coeff2.regValue=rtd_inl(DI_DI_GST_FMV_Pan_Coeff2_reg);
	di_di_gst_fmv_pan_coeff2.gst_fmvhist_sad_diff=1;
	rtd_outl(DI_DI_GST_FMV_Pan_Coeff2_reg, di_di_gst_fmv_pan_coeff2.regValue);

	di_di_smd_sampling_gmvc.regValue=rtd_inl(DI_DI_SMD_Sampling_GMVc_reg);
	if((DynamicOptimizeSystem[107]==1)&&cur_source==VIP_QUALITY_HDMI_1080I)
	{
	//	di_di_smd_sampling_gmvc.smd_close2gmv_mvselect =3;
	}
	else
	{
	//	di_di_smd_sampling_gmvc.smd_close2gmv_mvselect =3;
	}
	rtd_outl(DI_DI_SMD_Sampling_GMVc_reg, di_di_smd_sampling_gmvc.regValue);

	init_di_smd_gmvcoeff3_reg.regValue=rtd_inl(DI_DI_SMD_GMVCoeff3_reg);
	hgmv =init_di_smd_gmvcoeff3_reg.smd_gmv1_x;
	vgmv =init_di_smd_gmvcoeff3_reg.smd_gmv1_y;


	if(((hgmv==3)&&(vgmv==3))||DynamicOptimizeSystem[180]==1) //hGMV , VGMA=0
	{
		smd_gmvcoeff1_reg.regValue= rtd_inl(DI_DI_SMD_GMVCoeff1_reg);
		smd_gmvcoeff2_reg.regValue= rtd_inl(DI_DI_SMD_GMVCoeff2_reg);
		smd_choosecandidate_reg.regValue =  rtd_inl(DI_DI_SMD_ChooseCandidate_reg);
		// chen 170522
//		zoommotion_detector_reg.regValue= rtd_inl(DI_IM_DI_ZoomMotion_Detector_reg);
// end chen 170522
		init_di_smd_additionalcoeff2_reg.regValue =  rtd_inl(DI_DI_SMD_AdditionalCoeff2_reg);

		if(smd_gmvcoeff1_reg.smd_gmv_ctr==0)
			smd_gmvcoeff1_reg.smd_gmv_ctr=1;


		match_percent1 = smd_gmvcoeff1_reg.smd_gmv1_max_ctr*100/smd_gmvcoeff1_reg.smd_gmv_ctr;
		match_percent2=smd_gmvcoeff2_reg.smd_gmv2_max_ctr*100/smd_gmvcoeff1_reg.smd_gmv_ctr;

//		if((match_percent1>95&&match_percent2<10&&motionl_level<3)||zoommotion_detector_reg.smd_zoommotion_flag !=0||DynamicOptimizeSystem[180]==1)
// chen 170522
		if((match_percent1>95&&match_percent2<10&&motionl_level<3)||DynamicOptimizeSystem[180]==1)
// end chen 170522
		{
			smd_choosecandidate_reg.smd_reliablerange_sad_th=50;
			init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset=0x1f;
			init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain=0x07;

		}
		else
		{
			smd_choosecandidate_reg.smd_reliablerange_sad_th=125;
			init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset=0x0f;
			init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain=0x02;

		}
		gSmd_weak_move =smd_choosecandidate_reg.smd_reliablerange_sad_th; // for lg pattern used
		gSmd_offset = init_di_smd_additionalcoeff2_reg.smd_search_gmv_offset;
		gSmd_gain=init_di_smd_additionalcoeff2_reg.smd_search_gmv_gain;

		//rtd_outl(DI_DI_SMD_CHOOSECANDIDATE_reg, smd_choosecandidate_reg.regValue);
		rtd_outl(DI_DI_SMD_AdditionalCoeff2_reg,init_di_smd_additionalcoeff2_reg.regValue);

		if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Slow_Motion1==TRUE))
		{
			ROSPrintf("total_gmv=%d, gmv1=%d ,gmv2=%d \n ",smd_gmvcoeff1_reg.smd_gmv_ctr,smd_gmvcoeff1_reg.smd_gmv1_max_ctr, smd_gmvcoeff2_reg.smd_gmv2_max_ctr);
			ROSPrintf("match_percent1=%d, match_percent2=%d \n ", match_percent1,match_percent2);
	// chen 170522
	//		ROSPrintf("motionl_level=%d, zoom=%d \n", motionl_level,zoommotion_detector_reg.smd_zoommotion_flag);
// end chen 170522
		}
		if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.Slow_Motion2==TRUE))
		{
			ROSPrintf("DynamicOptimizeSystem[180]=%d\n", DynamicOptimizeSystem[180]);
			ROSPrintf("choosecandidate=%d\n",smd_choosecandidate_reg.regValue);
			ROSPrintf("\n \n");

		}

	}

}
void scalerVIP_quality_init(unsigned long arg)
{
	//========== clear mpegnr data ==========
	mpegnr_ich1_det_hist_hor_step_RBUS ich1_det_hist_hor_step_reg;
	kernel_quality_init=1;

	ich1_det_hist_hor_step_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_det_Hist_Hor_step_reg);
	ich1_det_hist_hor_step_reg.cp_histhor_det_xstep=array_xstep[0];
	scaler_rtd_outl(MPEGNR_ICH1_det_Hist_Hor_step_reg, ich1_det_hist_hor_step_reg.regValue);
}

#ifdef ISR_IS_3DLR_FUNCTIOM_ENABLE
void scalerVIP_is3DLR(void)
{
	if((scalerInfo_getPanelInfo_3DPanelType() == 1) && (scalerInfo_check_is_3dMode() == TRUE) &&
		(VODMA_3D_Check_Is_Idma3dMode() == FALSE))
	{
		if(scalerInfo_getDisp3dInfo_3dType() == HDMI3D_SIDE_BY_SIDE_HALF)
		{
			is3DLRmode = TRUE;
		}
		else if(scalerInfo_getDisp3dInfo_3dType() == HDMI3D_TOP_AND_BOTTOM)
		{
			is3DLRmode = TRUE;
		}
		else if(scalerInfo_getDisp3dInfo_3dType() == HDMI3D_FRAME_PACKING)
		{
			is3DLRmode = FALSE;
		}
		else if(scalerInfo_getDisp3dInfo_3dType() ==HDMI3D_2D_CVT_3D)
		{
			is3DLRmode = FALSE;
		}
		else
		{
			is3DLRmode = FALSE;
		}
	}
	else
	{
		is3DLRmode = FALSE;
	}
}
#endif

void scalerVIP_TV003_MAD_Y(unsigned short *PQA_Input_Level, unsigned short *PQA_Input_rate)
{
	unsigned char count_Y = 8;
	di_im_di_rtnr_mad_y_th_RBUS im_di_rtnr_mad_y_th_reg;
	di_im_di_rtnr_mad_y_counter_RBUS im_di_rtnr_mad_y_counter_reg;

	unsigned int mad_y_th1_1[8] = 	{0x004,	0x008,	0x00c,	0x010,	0x014,	0x018,	0x01c,	0x001};
	unsigned int mad_y_th2_1[8] = 	{0x008,	0x00c,	0x010,	0x014,	0x018,	0x01c,	0x7ff,	0x004};
	unsigned int mad_y_th1_2[8] = 	{0x004,	0x008,	0x00c,	0x010,	0x014,	0x018,	0x020,	0x001};
	unsigned int mad_y_th2_2[8] = 	{0x00c,	0x010,	0x014,	0x018,	0x01c,	0x020,	0x7ff,	0x008};

	static unsigned int mad_y_count[8] = {0};
	static unsigned int mad_y_std[8] = {0};

	static unsigned char RTNR_level[8] = {0};

	static unsigned int MAD_count = 0;

	if(RPC_SmartPic_clue->SceneChange)
		return;

//	di_im_di_rtnr_mad_c_th_RBUS im_di_rtnr_mad_c_th_reg;
	im_di_rtnr_mad_y_th_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_TH_reg);
//	im_di_rtnr_mad_c_th_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_C_TH_reg);

#if 1
	if(SmartPic_clue->Hist_Y_Mean_Value<15)
	{
		im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = mad_y_th1_1[MAD_count%count_Y];
		im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2 = mad_y_th2_1[MAD_count%count_Y];
	}
	else if(SmartPic_clue->Hist_Y_Mean_Value<25)
	{
		im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = ((mad_y_th1_2[MAD_count%count_Y]-mad_y_th1_1[MAD_count%count_Y])*(SmartPic_clue->Hist_Y_Mean_Value-15)/(20-15))+mad_y_th1_1[MAD_count%count_Y];
		im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2 = ((mad_y_th2_2[MAD_count%count_Y]-mad_y_th2_1[MAD_count%count_Y])*(SmartPic_clue->Hist_Y_Mean_Value-15)/(20-15))+mad_y_th2_1[MAD_count%count_Y];
	}
	else
#endif
	{
		im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = mad_y_th1_2[MAD_count%count_Y];
		im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2 = mad_y_th2_2[MAD_count%count_Y];
	}

	rtd_outl(DI_IM_DI_RTNR_MAD_Y_TH_reg, im_di_rtnr_mad_y_th_reg.regValue);
//	rtd_outl(DI_IM_DI_RTNR_MAD_C_TH_reg, im_di_rtnr_mad_c_th_reg.regValue);

	im_di_rtnr_mad_y_counter_reg.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_COUNTER_reg);

//	mad_y_count[MA_print_count%10] = (unsigned int)im_di_rtnr_mad_y_counter_reg.cp_temporal_mad_countery;

//	mad_y_std[MAD_count%count_Y] = (ABS(im_di_rtnr_mad_y_counter_reg.cp_temporal_mad_countery, mad_y_count[MAD_count%count_Y])>>7)*(ABS(im_di_rtnr_mad_y_counter_reg.cp_temporal_mad_countery, mad_y_count[MAD_count%count_Y])>>7);
	mad_y_std[MAD_count%count_Y] = (ABS(im_di_rtnr_mad_y_counter_reg.cp_temporal_mad_countery, mad_y_count[MAD_count%count_Y]))<<1;
	mad_y_count[MAD_count%count_Y] = (7 * (unsigned int)im_di_rtnr_mad_y_counter_reg.cp_temporal_mad_countery + 1*mad_y_count[MAD_count%count_Y])>>3;
//	mad_y_std[MA_print_count%count_Y] = (3*((ABS(im_di_rtnr_mad_y_counter_reg.cp_temporal_mad_countery, mad_y_count[MA_print_count%count_Y])>>4)) + 1*mad_y_std[MA_print_count%count_Y])>>2;


	if(MAD_count%count_Y == 0)
	{

		unsigned char Max_level = 9;

		RTNR_level[0] = ((unsigned short)((mad_y_count[0]*1)/(mad_y_count[7]+1))>Max_level)?Max_level:((unsigned short)((mad_y_count[0]*1)/(mad_y_count[7]+1)));
		RTNR_level[1] = ((unsigned short)((mad_y_count[1]*2)/(mad_y_count[7]+1))>Max_level)?Max_level:((unsigned short)((mad_y_count[1]*2)/(mad_y_count[7]+1)));
		RTNR_level[2] = ((unsigned short)((mad_y_count[2]*3)/(mad_y_count[7]+1))>Max_level)?Max_level:((unsigned short)((mad_y_count[2]*3)/(mad_y_count[7]+1)));
		RTNR_level[3] = ((unsigned short)((mad_y_count[3]*4)/(mad_y_count[7]+1))>Max_level)?Max_level:((unsigned short)((mad_y_count[3]*4)/(mad_y_count[7]+1)));
		RTNR_level[4] = ((unsigned short)((mad_y_count[4]*5)/(mad_y_count[7]+1))>Max_level)?Max_level:((unsigned short)((mad_y_count[4]*5)/(mad_y_count[7]+1)));
		RTNR_level[5] = ((unsigned short)((mad_y_count[5]*6)/(mad_y_count[7]+1))>Max_level)?Max_level:((unsigned short)((mad_y_count[5]*6)/(mad_y_count[7]+1)));
		RTNR_level[6] = ((unsigned short)((mad_y_count[6]*7)/(mad_y_count[7]+1))>Max_level)?Max_level:((unsigned short)((mad_y_count[6]*7)/(mad_y_count[7]+1)));

#if 0
#if 1
		ROSPrintf("a = 	");
		for(i = 0; i< count_Y; i++)
		{
			ROSPrintf("%d,	",mad_y_count[i]);
		}
		ROSPrintf("\n");
		ROSPrintf("b = 	");
		for(i = 0; i< count_Y; i++)
		{
			ROSPrintf("%d,	",mad_y_std[i]);
		}
		ROSPrintf("\n");
#endif
		ROSPrintf("c = 	");
		for(i = 0; i< count_Y; i++)
		{
			//ROSPrintf("%d,	",((unsigned short)(mad_y_count[i]/(mad_y_std[i]+1))));
			ROSPrintf("%d,	",RTNR_level[i]);
		}
		ROSPrintf("\n");
#endif
//		if((mad_y_count_sun!=0) && (mad_y_std_sun!=0))
//			ROSPrintf("b=	%d,		%d,		%d,		%d\n", mad_y_count_sun, mad_y_std_sun, ((mad_y_count_ave)/mad_y_count_sun), ((mad_y_std_ave)/mad_y_std_sun));
	}
	MAD_count++;
	PQA_Input_Level[I_003_RTNR_TH1] = ((unsigned char)PQA_Input_Level[I_003_RTNR_TH1]>RTNR_level[0])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH1]-1):(((unsigned char)PQA_Input_Level[I_003_RTNR_TH1]<RTNR_level[0])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH1]+1):((unsigned char)PQA_Input_Level[I_003_RTNR_TH1]));
	PQA_Input_Level[I_003_RTNR_TH2] = ((unsigned char)PQA_Input_Level[I_003_RTNR_TH2]>RTNR_level[1])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH2]-1):(((unsigned char)PQA_Input_Level[I_003_RTNR_TH2]<RTNR_level[1])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH2]+1):((unsigned char)PQA_Input_Level[I_003_RTNR_TH2]));
	PQA_Input_Level[I_003_RTNR_TH3] = ((unsigned char)PQA_Input_Level[I_003_RTNR_TH3]>RTNR_level[2])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH3]-1):(((unsigned char)PQA_Input_Level[I_003_RTNR_TH3]<RTNR_level[2])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH3]+1):((unsigned char)PQA_Input_Level[I_003_RTNR_TH3]));
	PQA_Input_Level[I_003_RTNR_TH4] = ((unsigned char)PQA_Input_Level[I_003_RTNR_TH4]>RTNR_level[3])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH4]-1):(((unsigned char)PQA_Input_Level[I_003_RTNR_TH4]<RTNR_level[3])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH4]+1):((unsigned char)PQA_Input_Level[I_003_RTNR_TH4]));
	PQA_Input_Level[I_003_RTNR_TH5] = ((unsigned char)PQA_Input_Level[I_003_RTNR_TH5]>RTNR_level[4])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH5]-1):(((unsigned char)PQA_Input_Level[I_003_RTNR_TH5]<RTNR_level[4])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH5]+1):((unsigned char)PQA_Input_Level[I_003_RTNR_TH5]));
	PQA_Input_Level[I_003_RTNR_TH6] = ((unsigned char)PQA_Input_Level[I_003_RTNR_TH6]>RTNR_level[5])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH6]-1):(((unsigned char)PQA_Input_Level[I_003_RTNR_TH6]<RTNR_level[5])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH6]+1):((unsigned char)PQA_Input_Level[I_003_RTNR_TH6]));
	PQA_Input_Level[I_003_RTNR_TH7] = ((unsigned char)PQA_Input_Level[I_003_RTNR_TH7]>RTNR_level[6])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH7]-1):(((unsigned char)PQA_Input_Level[I_003_RTNR_TH7]<RTNR_level[6])?((unsigned char)PQA_Input_Level[I_003_RTNR_TH7]+1):((unsigned char)PQA_Input_Level[I_003_RTNR_TH7]));

	PQA_Input_rate[I_003_RTNR_TH1] = 0;
	PQA_Input_rate[I_003_RTNR_TH2] = 0;
	PQA_Input_rate[I_003_RTNR_TH4] = 0;
	PQA_Input_rate[I_003_RTNR_TH5] = 0;
	PQA_Input_rate[I_003_RTNR_TH6] = 0;
	PQA_Input_rate[I_003_RTNR_TH7] = 0;
}

//#######################################################
//--------- NetFlix Smooth toggle Program -----------------------------------
void scalerVIP_NetFlix_smoothtoggle_enable(_RPC_clues* SmartPic_clue,_RPC_system_setting_info* RPC_system_info_structure_table)
{
	//_clues* SmartPic_clue=NULL;
	//_system_setting_info* system_info_structure_table = NULL;
	//SmartPic_clue = scaler_GetShare_Memory_SmartPic_Clue_Struct();
	//system_info_structure_table = scaler_GetShare_Memory_system_setting_info_Struct();

 if((RPC_system_info_structure_table->VIP_source >= VIP_QUALITY_PVR_480I) && (RPC_system_info_structure_table->VIP_source <=VIP_QUALITY_PVR_4k2kP_30))	//Netflix
 	RPC_SmartPic_clue->S_NetFlix_info.Netflix_Enable =1;
 else
 	RPC_SmartPic_clue->S_NetFlix_info.Netflix_Enable =0;
}

unsigned char scalerVIP_CheckRegisterAccessProtect(unsigned int regAddr)
{
	#define REG_BLOCK_VD 0xb8019000
	#define REG_BLOCK_MASK 0xfffff000
	//VD register
	if (((Scaler_InputSrcGetMainChFrom() != _SRC_FROM_VDC) || !vd_pwr_ok) && ((regAddr&REG_BLOCK_MASK)==REG_BLOCK_VD))
		return false;
	else
		return true;
}

//***************************************************************//
//****************Arrange no use function(From Sirius CSFC) ***********//
//***************************************************************//


/**
 * Profile stiall video or motion video
 *
 * @param	void
 * @return	void
 */

#if 0
static unsigned short H_Block_Pre[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static unsigned short V_Block_Pre[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
void scalerVIP_colorProfileStillMotion(void)
{

	unsigned char ii;
	unsigned short H_Block_Sum = 0;
	unsigned short V_Block_Sum = 0;
	unsigned char H_Ratio[16];//H_Block_Pre*100/H_Block
	unsigned char V_Ratio[16];
	unsigned char H_cnt = 0;
	unsigned char V_cnt = 0;
	unsigned char Profile_Block_Num = 16;//set profile h&V block number
	//unsigned char VD_Noise_Status = 0; //removed in columbus

	unsigned short Profile_transition_sum = 0;

	static unsigned char exit_motion_cnt = 0;
	static unsigned char flag_profile_still;
	static unsigned char flag_profile_still_pre;
	static unsigned char still_frame_cnt = 0;
	static unsigned short H_Block_Sum_Pre;
	static unsigned short V_Block_Sum_Pre;

	scalerDrvPrint( "enter scalerVIP_colorProfileStillMotion\n" );

	if ( scalerVIP_colorProfileDectector( _CHANNEL1, Profile_Block_Num,  &H_Block[0], &V_Block[0], &Profile_transition_sum ) == 1 )
	{
		for ( ii=0 ; ii<Profile_Block_Num ; ii++ )
		{
			H_Block_Sum += H_Block[ii];
			V_Block_Sum += V_Block[ii];

			H_Ratio[ii] = H_Block_Pre[ii] * 100 / (H_Block[ii]+1);
			V_Ratio[ii] = V_Block_Pre[ii] * 100 / (V_Block[ii]+1);

			if ( H_Ratio[ii]<98 || H_Ratio[ii]>101 ) //check if a large motion
				H_cnt++;
			if ( V_Ratio[ii]<98 || V_Ratio[ii]>101 )
				V_cnt++;
		}
		//scalerDrvPrint( "H_Block_Sum%d, V_Block_Sum=%d\n", H_Block_Sum, V_Block_Sum );
#if 0 // print debug messages
		for ( ii=0 ; ii<Profile_Block_Num ; ii=ii+2 )
		{
			scalerDrvPrint( "H_Ratio[%d]=%d, V_Ratio[%d]=%d\n", ii,H_Ratio[ii], ii, V_Ratio[ii] );
		}
		scalerDrvPrint( "H_cnt=%d, V_cnt=%d\n", H_cnt, V_cnt );
#endif
		if ( H_cnt<3 && V_cnt<3 )
		{
			exit_motion_cnt++;
			if ( exit_motion_cnt >= 3 )
				flag_profile_still = 1;//pattern is still
		}
		else
		{
			flag_profile_still = 0;//pattern is motion
			still_frame_cnt = 0;
			exit_motion_cnt = 0;
		}

		for ( ii=0 ; ii<Profile_Block_Num ; ii++ )
		{
			H_Block_Pre[ii] = H_Block[ii];
			V_Block_Pre[ii] = V_Block[ii];
		}
		H_Block_Sum_Pre = H_Block_Sum;
		V_Block_Sum_Pre = V_Block_Sum;
	}
	else
		flag_profile_still = 2;//didn't dectection

	flag_profile_still_pre = flag_profile_still;
	if ( (flag_profile_still_pre==1) && (flag_profile_still_pre==flag_profile_still) ) {
		still_frame_cnt++;
		if ( still_frame_cnt == 120 )
			still_frame_cnt = 5;
	}

}
#endif

//####################################################
void scalerVIP_pattern_detected(void)
{
	static unsigned char cnt = 0;

	mpegnr_ich1_accessdata_ctrl_pk_RBUS accessdata_ctrl_pk_reg;
	mpegnr_ich1_accessdata_posctrl_pk_RBUS accessdata_posctrl_pk_reg;
	mpegnr_ich1_readdata_data_y1_pk_RBUS readdata_data_y1_pk_reg;
	mpegnr_ich1_readdata_data_c1_pk_RBUS readdata_data_c1_pk_RBUS;
	accessdata_ctrl_pk_reg.regValue = rtd_inl(MPEGNR_ICH1_AccessData_CTRL_PK_reg);
	accessdata_posctrl_pk_reg.regValue = rtd_inl(MPEGNR_ICH1_AccessData_PosCtrl_PK_reg);
	readdata_data_y1_pk_reg.regValue = rtd_inl(MPEGNR_ICH1_ReadData_DATA_Y1_PK_reg);
	readdata_data_c1_pk_RBUS.regValue = rtd_inl(MPEGNR_ICH1_ReadData_DATA_C1_PK_reg);

	accessdata_ctrl_pk_reg.write_en = 1;
	accessdata_posctrl_pk_reg.startx = 100;
	accessdata_posctrl_pk_reg.starty = 100;
	readdata_data_y1_pk_reg.y00 = 255;

	if(cnt == 0)
	{
		readdata_data_c1_pk_RBUS.u00 = 255;
		readdata_data_c1_pk_RBUS.v00 = 0;
		cnt = 1;
	}
	else
	{
		readdata_data_c1_pk_RBUS.u00 = 0;
		readdata_data_c1_pk_RBUS.v00 = 255;
		cnt = 0;
	}

	rtd_outl(MPEGNR_ICH1_AccessData_CTRL_PK_reg, accessdata_ctrl_pk_reg.regValue);
	rtd_outl(MPEGNR_ICH1_AccessData_PosCtrl_PK_reg, accessdata_posctrl_pk_reg.regValue);
	rtd_outl(MPEGNR_ICH1_ReadData_DATA_Y1_PK_reg, readdata_data_y1_pk_reg.regValue);
	rtd_outl(MPEGNR_ICH1_ReadData_DATA_C1_PK_reg, readdata_data_c1_pk_RBUS.regValue);
}

void Scaler_color_print_scalerinfo_isr(void)
{
	vip_color_dcc_d_dcc_fwused_1_RBUS dcc_fwused_1_reg;
	dcc_fwused_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_FWUSED_1_reg);

	if((system_info_structure_table) && (dcc_fwused_1_reg.regValue>>31== 1) && (RPC_system_info_structure_table)) {
		ROSPrintf("===== VIP_scaler_info =====\n");
		/*============================================*/
		ROSPrintf("Input_src_Type = %d\n", system_info_structure_table->Input_src_Type);
		ROSPrintf("Input_src_Form = %d\n", system_info_structure_table->Input_src_Form);
		ROSPrintf("Input_VO_Form = %d\n", system_info_structure_table->Input_VO_Form);
		ROSPrintf("input_TVD_Form = %d\n", system_info_structure_table->Input_TVD_Form);
		ROSPrintf("vdc_color_standard = %d\n", system_info_structure_table->vdc_color_standard);
		ROSPrintf("Timing = %d\n", system_info_structure_table->Timing);
		ROSPrintf("HDMI_video_format = %d\n", system_info_structure_table->HDMI_video_format);
		ROSPrintf("VIP_source = %d\n", RPC_system_info_structure_table->VIP_source);
		ROSPrintf("Display_RATIO_TYPE = %d\n", system_info_structure_table->Display_RATIO_TYPE);
		ROSPrintf("HDMI_data_range = %d\n", system_info_structure_table->HDMI_data_range);
		ROSPrintf("HDR_flag = %d\n", system_info_structure_table->HDR_flag);
		ROSPrintf("DolbyHDR_flag = %d\n", system_info_structure_table->DolbyHDR_flag);

		ROSPrintf("input_display = %d\n", system_info_structure_table->input_display);
		ROSPrintf("HDMI_color_space = %d\n", system_info_structure_table->HDMI_color_space);
		ROSPrintf("IV_Start = %d\n", system_info_structure_table->IV_Start);
		ROSPrintf("IH_Start = %d\n", system_info_structure_table->IH_Start);
		ROSPrintf("I_Height = %d\n", system_info_structure_table->I_Height);
		ROSPrintf("I_Width = %d\n", system_info_structure_table->I_Width);
		ROSPrintf("Mem_Height = %d\n", system_info_structure_table->Mem_Height);
		ROSPrintf("Mem_Width = %d\n", system_info_structure_table->Mem_Width);
		ROSPrintf("Cap_Height = %d\n", system_info_structure_table->Cap_Height);
		ROSPrintf("Cap_Width = %d\n", system_info_structure_table->Cap_Width);
		ROSPrintf("DI_Width = %d\n", system_info_structure_table->DI_Width);
		ROSPrintf("D_Height = %d\n", system_info_structure_table->D_Height);
		ROSPrintf("D_Width = %d\n", system_info_structure_table->D_Width);
		ROSPrintf("color_fac_src_idx = %d\n", system_info_structure_table->color_fac_src_idx);
		ROSPrintf("Project_ID = %d\n", RPC_system_info_structure_table->Project_ID);
		/*============================================*/
		ROSPrintf("===== VIP_scaler_info end =====\n");
	}
}

unsigned char scalerVIP_check_vdc_hw_pwr()
{
	//crt_sys_pll_27x3_RBUS sys_pll_27x3_reg;
	sys_reg_sys_clken1_RBUS sys_reg_sys_clken1;

	//sys_pll_27x3_reg.regValue = rtdf_inl(CRT_SYS_PLL_27X3_reg);
	sys_reg_sys_clken1.regValue = rtd_inl(SYS_REG_SYS_CLKEN1_reg);

	if (sys_reg_sys_clken1.clken_vdec == 1 &&
		system_info_structure_table->Input_src_Form == _SRC_FROM_VDC) /*&&
		sys_pll_27x3_reg.pll27x_pow == 1 &&
		sys_pll_27x3_reg.pll27x_rstb == 1 &&
		sys_pll_27x3_reg.pll27x_vcorstb == 1 &&
		sys_pll_27x3_reg.pll27x_ps_en == 1)
		*/
		return 1;
	else
		return 0;
}

void scalerVIP_Set_Noise_Level(unsigned char Level)
{
	Noise_Level = Level;
}
unsigned char scalerVIP_Get_Noise_Level(void)
{
	return Noise_Level;
}
unsigned char scalerVIP_Get_PT_NO(void)
{
	unsigned char i=0;

	for(i=0;i<200;i++)
	{
		if(DynamicOptimizeSystem[i]==1)
		{
			return i;
		}
	}
	return 0;

}

//============= Set Info to MEMC  jzl 20151231====================
#ifdef CONFIG_HW_SUPPORT_MEMC
void scalerVIP_Set_MEMC_Info_toVCPU(void)
{
	_RPC_system_setting_info* system_RPC_info_structure_table = NULL;
	unsigned char *FilmInfo = Scaler_MEMC_GetCadence();

	system_RPC_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();


	if((system_RPC_info_structure_table == NULL)) {
		if(MA_print_count%360==0) {
			ROSPrintf(" === scalerVIP_Set_MEMC_Info_toVCPU === \n\n");
			ROSPrintf(" Mem = %p\n\n",system_RPC_info_structure_table);
		}
		return;
	}

	if( FilmInfo == NULL )
	{
		if( MA_print_count%120 == 0)
			ROSPrintf("[%s] Film information ptr from MEMC error\n", __func__);
		return;
	}


	system_RPC_info_structure_table->MEMC_ISRIINFO_TO_VCPU.memc_cadence = FilmInfo[_FILM_MEMC_OUT_ALL];
	system_RPC_info_structure_table->MEMC_ISRIINFO_TO_VCPU.memc_avdelay = Scaler_MEMC_GetAVSyncDelay();


}
#endif

extern int PictureMode_flg;
UINT32 Forbes_flg = 0;

void scalerVIP_Set_Info_toMEMC(void)
{
	unsigned int osd_bri_sub = 0;// (system_info_structure_table->OSD_Info.Brightness_Gain_Sub);
	unsigned int osd_con_sub = 0;
	static unsigned int su_hor_factor_pre, su_ver_factor_pre, sd_hor_factor_pre, sd_ver_factor_pre;
	unsigned int su_hor_factor_cur, su_ver_factor_cur, sd_hor_factor_cur, sd_ver_factor_cur;
	scaleup_dm_uzu_scale_ver_factor_RBUS	scaleup_dm_uzu_scale_ver_factor_reg;
	scaleup_dm_uzu_scale_hor_factor_RBUS	scaleup_dm_uzu_scale_hor_factor_reg;
	scaledown_ich1_uzd_scale_hor_factor_RBUS	scaledown_ich1_uzd_scale_hor_factor_reg;
	scaledown_ich1_uzd_scale_ver_factor_RBUS	scaledown_ich1_uzd_scale_ver_factor_reg;
	static int hor_front_pre, hor_back_pre, ver_front_pre, ver_back_pre;
	int hor_front_cur, hor_back_cur, ver_front_cur, ver_back_cur;

	scaledown_ich1_sdnr_cutout_range_hor_RBUS ich1_sdnr_cutout_range_hor_reg;
	scaledown_ich1_sdnr_cutout_range_ver0_RBUS ich1_sdnr_cutout_range_ver0_reg;
	unsigned char i=0;
	UINT32 Hist_diff = 0;

	setScalerVIPInfo.bSRC_VO = (system_info_structure_table->Input_src_Type == _SRC_VO)? 1:0;
	setScalerVIPInfo.source_type = RPC_system_info_structure_table->VIP_source;
	setScalerVIPInfo.bHDR_Chg = (RPC_system_info_structure_table->HDR_info.HDR_Setting_Status == 3)? 1:0;
	setScalerVIPInfo.PictureMode_Chg = PictureMode_flg;
	if(PictureMode_flg == 1)
		PictureMode_flg = 0;

	setScalerVIPInfo.mBrightness = ((osd_bri_sub<<8) + (system_info_structure_table->OSD_Info.Brightness_Gain)) & 0xffff;	// uint16: msb: bri_sub; lsb: bri_main
	setScalerVIPInfo.mContrast= ((osd_con_sub<<8) + (system_info_structure_table->OSD_Info.Contrast_Gain)) & 0xffff;	// uint16: msb: bri_sub; lsb: bri_main

	scaleup_dm_uzu_scale_hor_factor_reg.regValue = rtd_inl(SCALEUP_DM_UZU_Scale_Hor_Factor_reg);
	su_hor_factor_cur = scaleup_dm_uzu_scale_hor_factor_reg.hor_factor;

	scaleup_dm_uzu_scale_ver_factor_reg.regValue = rtd_inl(SCALEUP_DM_UZU_Scale_Ver_Factor_reg);
	su_ver_factor_cur = scaleup_dm_uzu_scale_ver_factor_reg.ver_factor;

	scaledown_ich1_uzd_scale_hor_factor_reg.regValue = rtd_inl(SCALEDOWN_ICH1_UZD_Scale_Hor_Factor_reg);
	sd_hor_factor_cur = scaledown_ich1_uzd_scale_hor_factor_reg.hor_fac;

	scaledown_ich1_uzd_scale_ver_factor_reg.regValue = rtd_inl(SCALEDOWN_ICH1_UZD_Scale_Ver_Factor_reg);
	sd_ver_factor_cur = scaledown_ich1_uzd_scale_ver_factor_reg.ver_fac;

	if((su_hor_factor_pre != su_hor_factor_cur) || (su_ver_factor_pre != su_ver_factor_cur)
		|| (sd_hor_factor_pre != sd_hor_factor_cur) || (sd_ver_factor_pre != sd_ver_factor_cur))
	{
		setScalerVIPInfo.bZoomAction = 1;
	}
	else
	{
		setScalerVIPInfo.bZoomAction = 0;
	}

	su_hor_factor_pre = su_hor_factor_cur;
	su_ver_factor_pre = su_ver_factor_cur;
	sd_hor_factor_pre = sd_hor_factor_cur;
	sd_ver_factor_pre = sd_ver_factor_cur;

	//xy shift
	ich1_sdnr_cutout_range_hor_reg.regValue = rtd_inl(SCALEDOWN_ich1_sdnr_cutout_range_hor_reg);
	hor_front_cur = ich1_sdnr_cutout_range_hor_reg.hor_front;
	hor_back_cur = ich1_sdnr_cutout_range_hor_reg.hor_back;

	ich1_sdnr_cutout_range_ver0_reg.regValue = rtd_inl(SCALEDOWN_ich1_sdnr_cutout_range_ver0_reg);
	ver_front_cur = ich1_sdnr_cutout_range_ver0_reg.ver_front;
	ver_back_cur = ich1_sdnr_cutout_range_ver0_reg.ver_back;

	if((hor_front_pre != hor_front_cur) || (hor_back_pre != hor_back_cur)
		|| (ver_front_pre != ver_front_cur) || (ver_back_pre != ver_back_cur))
	{
		setScalerVIPInfo.bShiftAction = 1;
	}
	else
	{
		setScalerVIPInfo.bShiftAction = 0;
	}

	hor_front_pre = hor_front_cur;
	hor_back_pre = hor_back_cur;
	ver_front_pre = ver_front_cur;
	ver_back_pre = ver_back_cur;


	for (i=0; i<COLOR_HISTOGRAM_LEVEL; i++)
	{
		if (SmartPic_clue->Y_Main_Hist[i] > 10000)
		{
			setScalerVIPInfo.mDarkLevel = (i*2);
			break;
		}
	}

	for (i=0; i<COLOR_HISTOGRAM_LEVEL; i++)
	{
		if(Y_Main_Hist_pre[i] > SmartPic_clue->Y_Main_Hist[i])
			Hist_diff = Hist_diff + (Y_Main_Hist_pre[i] - SmartPic_clue->Y_Main_Hist[i]);
		else
			Hist_diff = Hist_diff + (SmartPic_clue->Y_Main_Hist[i] - Y_Main_Hist_pre[i]);
	}

	setScalerVIPInfo.Hist_SC_flg = ((Hist_diff > 5000) ? 1 : 0);

//	if(Hist_SC_flg = 1)
//		rtd_pr_vpq_isr_info("$$$   Hist_diff = (%x)   $$$\r\n$$$   Hist_SC_flg = (%d)   $$$\r\n", Hist_diff, Hist_SC_flg);

	for(i =  0; i < COLOR_HISTOGRAM_LEVEL; i++){
		Y_Main_Hist_pre[i] = SmartPic_clue->Y_Main_Hist[i];
	}

	// soccer patch
	{
		unsigned short  h_soccer_grass[24]={10, 10, 4, 7, 7, 16, 117, 703, 11, 1, 0, 0, 0, 0, 1, 1, 5, 37, 24, 8, 7, 6, 6, 8};
		unsigned int  s_soccer_grass[32]={28, 98, 814, 34, 10, 3, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		unsigned short  y_soccer_grass[32]={0, 0, 0, 0, 1, 1, 2, 4, 8, 11, 15, 77, 301, 129, 284, 58, 29, 26, 13, 7, 3, 2, 2, 2, 1, 1, 1, 2, 1, 1, 0, 0};

		unsigned short h_ratio, s_ratio, y_ratio;
		unsigned short h_th = 950, s_th = 950, y_th = 920;

		h_ratio = scalerVIP_ratio_inner_product_base_16(&h_soccer_grass[0], &hue_hist_ratio[0], 24);
		s_ratio = scalerVIP_ratio_inner_product_base(&s_soccer_grass[0], &sat_hist_ratio[0], 32);
		y_ratio = scalerVIP_ratio_inner_product_base_16(&y_soccer_grass[0], &Y_hist_ratio[0], 32);

		if((h_ratio>h_th) && (s_ratio>s_th) && (y_ratio>y_th))
		{
			setScalerVIPInfo.mSoccer_holdfrm = 8;
		}
		else if(setScalerVIPInfo.mSoccer_holdfrm>0)
		{
			setScalerVIPInfo.mSoccer_holdfrm--;
		}
		else
		{
			setScalerVIPInfo.mSoccer_holdfrm = 0;
		}
	}

	//high-freq and player OSD change
	{
		if(system_info_structure_table->Input_src_Type == _SRC_HDMI)
		{
			if(RTNR_MAD_count_Y_avg>800 && RTNR_MAD_count_Y3_avg<20)
				setScalerVIPInfo.bVIP_still = 1;
			else
				setScalerVIPInfo.bVIP_still = 0;
		}
	}

	//Forbes ball patch
	{
		static UINT8 Forbes_cnt = 0;
		UINT32 black_sum = 0;
		UINT32 white_sum = 0;
		UINT32 total_sum = 0;
		UINT8 it;

		for(it = 0; it < 20; it++){
			black_sum += SmartPic_clue->Y_Main_Hist[it];
			white_sum += SmartPic_clue->Y_Main_Hist[it+107];
		}

		for(it = 0; it < 128; it++){
			total_sum += SmartPic_clue->Y_Main_Hist[it];
		}

		if(total_sum != 0){
			if((black_sum*100)/total_sum>=95 && (white_sum*100)/total_sum<=5){
				Forbes_cnt++;
				if(Forbes_cnt >= 80)
					Forbes_cnt = 80;
			}else{
				if(Forbes_cnt -1 < 1)
					Forbes_cnt = 1;
				else
					Forbes_cnt--;
			}
		}

		if(Forbes_cnt >= 10)
			Forbes_flg = 1;
		else
			Forbes_flg = 0;

		setScalerVIPInfo.Forbes_flg = Forbes_flg;

	}
}

SET_MEMC_SCALER_VIP_INFO scalerVIP_Get_Info_fromScaler(void)
{
	return setScalerVIPInfo;
}

//============= Set Info to MEMC  =============================

void scalerVIP_color_DRV_SNR_Veritcal_NR(DRV_SNR_Veritcal_NR *ptr)
{
	iedge_smooth_vfilter_0_RBUS vfilter_0_reg;
	iedge_smooth_vfilter_1_RBUS vfilter_1_reg;
	iedge_smooth_vfilter_2_RBUS vfilter_2_reg;
	iedge_smooth_vfilter_3_RBUS vfilter_3_reg;
#if 0 //merlin2
	iedge_smooth_vfilter_4_RBUS vfilter_4_reg;
	iedge_smooth_vfilter_5_RBUS vfilter_5_reg;
	iedge_smooth_vfilter_6_RBUS vfilter_6_reg;
#endif

	vfilter_0_reg.regValue = rtd_inl(IEDGE_SMOOTH_VFILTER_0_reg);
	vfilter_1_reg.regValue = rtd_inl(IEDGE_SMOOTH_VFILTER_1_reg);
	vfilter_2_reg.regValue = rtd_inl(IEDGE_SMOOTH_VFILTER_2_reg);
	vfilter_3_reg.regValue = rtd_inl(IEDGE_SMOOTH_VFILTER_3_reg);
#if 0 //merlin2
	vfilter_4_reg.regValue = rtd_inl(IEDGE_SMOOTH_VFILTER_4_reg);
	vfilter_5_reg.regValue = rtd_inl(IEDGE_SMOOTH_VFILTER_5_reg);
	vfilter_6_reg.regValue = rtd_inl(IEDGE_SMOOTH_VFILTER_6_reg);
#endif

	vfilter_0_reg.avlpf_en = ptr->SNR_Veritcal_avlpf_En;
	vfilter_2_reg.avlpf_c_en = ptr->SNR_Veritcal_C_En;
	vfilter_0_reg.avlpf_correct_en = ptr->SNR_Veritcal_correct_En;
	vfilter_0_reg.avlpf_bw_pattern_en = ptr->SNR_Veritcal_BW_En;
	vfilter_0_reg.vfir_weight = ptr->SNR_Veritcal_Y_Weight;
	vfilter_2_reg.vfir_c_weight = ptr->SNR_Veritcal_C_Weight;
	vfilter_0_reg.avlpf_mindiff_th_start = ptr->SNR_Veritcal_Th_start;
	vfilter_0_reg.avlpf_mindiff_curve_rate_start = ptr->SNR_Veritcal_Rate_start;
	vfilter_0_reg.avlpf_mindiff_th_finish = ptr->SNR_Veritcal_Th_finish;
	vfilter_0_reg.avlpf_mindiff_curve_rate_finish = ptr->SNR_Veritcal_Rate_finish;
	//vfilter_0_reg.avlpf_tap_sel = ptr->SNR_Veritcal_avlpf_Tap_Sel; /*Merlin3*/
	vfilter_1_reg.avlpf_max_weight = ptr->SNR_Veritcal_Max_MinDiff_Weight;
	vfilter_1_reg.avlpf_min_weight = ptr->SNR_Veritcal_Min_MinDiff_Weight;
	vfilter_1_reg.avlpf_maxmindiff_th = ptr->SNR_Veritcal_MaxMinDiff_Th;
	vfilter_1_reg.avlpf_maxmindiff_curve_rate = ptr->SNR_Veritcal_MaxMinDiff_Rate;
	vfilter_2_reg.avlpf_bw_th = ptr->SNR_Veritcal_BWPattern_Th;
	vfilter_2_reg.avlpf_bw_margin = ptr->SNR_Veritcal_BWPattern_Margin;
	vfilter_2_reg.avlpf_bw_curve_rate = ptr->SNR_Veritcal_BWPattern_Rate;

#if 0 //merlin2
	vfilter_4_reg.esmex_iedge_vfir_enable = ptr->SNR_Vertical_fir_En;
	vfilter_4_reg.egsm_avlpf_vmode = ptr->SNR_Veritcal_Vmode;
	vfilter_4_reg.egsm_avlpf_clamp_en = ptr->SNR_Vertical_Clamp_En;
	vfilter_4_reg.egsm_avlpf_clampth = ptr->SNR_Vertical_Clamp_Th;
	vfilter_4_reg.egsm_avlpf_mindiff_th2_start = ptr->SNR_Vertical_Th2_start;
	vfilter_6_reg.egsm_avlpf2d_en = ptr->SNR_Veritcal_avlpf2D_En;
	vfilter_6_reg.egsm_avlpf2d_tolirant_en = ptr->SNR_Veritcal_avlpf2D_Tolirant_En;
	vfilter_5_reg.egsm_avlpf2d_tolirantmode = ptr->SNR_Veritcal_avlpf2D_Tolirant_Mode;
	vfilter_6_reg.egsm_avlpf2d_tolirantoffset = ptr->SNR_Veritcal_avlpf2D_Tolirant_Offset;
	// :Vfir_En, :Vmode, :Clamp_En, :Clamp_th, :Th2_sta, :avlpf2D_En, :avlpf2D_Tol_En, :avlpf2D_Tol_Mode, :avlpf2D_Tol_Offset
	// 0, 1, 0, 6, 0, 0, 0, 0, 4
#else //mac6
	vfilter_3_reg.egsm_avlpf_clamp_en = ptr->SNR_Vertical_Clamp_En;
	vfilter_3_reg.egsm_avlpf_clampth = ptr->SNR_Vertical_Clamp_Th;
	vfilter_3_reg.egsm_avlpf_clampth_tth = ptr->SNR_Vertical_Clamp_Th_TTH;
	vfilter_3_reg.egsm_avlpf_ignorehlh = ptr->SNR_Vertical_IgnoreHLH;
#endif

	rtd_outl(IEDGE_SMOOTH_VFILTER_0_reg, vfilter_0_reg.regValue);
	rtd_outl(IEDGE_SMOOTH_VFILTER_1_reg, vfilter_1_reg.regValue);
	rtd_outl(IEDGE_SMOOTH_VFILTER_2_reg, vfilter_2_reg.regValue);
	rtd_outl(IEDGE_SMOOTH_VFILTER_3_reg, vfilter_3_reg.regValue);
#if 0 //merlin2
	rtd_outl(IEDGE_SMOOTH_VFILTER_4_reg, vfilter_4_reg.regValue);
	rtd_outl(IEDGE_SMOOTH_VFILTER_5_reg, vfilter_5_reg.regValue);
	rtd_outl(IEDGE_SMOOTH_VFILTER_6_reg, vfilter_6_reg.regValue);
#endif
}



void scalerVIP_Teeth_detcted(void)
{
#if 0 //mac6 no MCNR, this api almost can't work
	di_di_smd_waterflow_teeth_ctr_RBUS di_di_smd_waterflow_teeth_ctr;
	unsigned int teeth_value=0,gmv_avg=0;
	static unsigned int teeth_avg=0,bad_cnt=0 ,Step_Hist_th=0xc80;
	static unsigned char gmv_avg_th=50;

	di_im_di_ma_field_teeth_th_a_RBUS teeth_detect_th_a_reg;

	di_di_smd_waterflow_teeth_ctr.regValue= rtd_inl(DI_DI_SMD_WaterFlow_Teeth_Ctr_reg);
	di_di_smd_waterflow_teeth_ctr.smd_waterflow_protect_en =1;
	//di_di_smd_waterflow_teeth_ctr.smd_compensate_teeth_th_wf = 5;

	rtd_outl(DI_DI_SMD_WaterFlow_Teeth_Ctr_reg,di_di_smd_waterflow_teeth_ctr.regValue);

	teeth_value = rtd_inl(DI_DI_SMD_WaterFlow_Teeth_Count_reg)&0xffff;


	if((teeth_avg >=1023))
	{
		if((teeth_value<1023)&&(bad_cnt<63))
		{
			teeth_value = 1023;
			bad_cnt++;
			//ROSPrintf("\n 1 teeth_value %d ,teeth_avg %d ,bad_cnt %d \n ",teeth_value,teeth_avg,bad_cnt);
		}
		else
		{
			bad_cnt=0;
		}

	}
	//if(rtd_inl(0xb802e4f4)==12)
	//ROSPrintf("\n 2 teeth_value %d ,teeth_avg %d ,bad_cnt %d \n ",teeth_value,teeth_avg,bad_cnt);

	teeth_avg = scalerVIP_compute_I_teeth_Index_Ave(teeth_value,0);



	if(RPC_SmartPic_clue->SceneChange == 1){
		gmv_avg = scalerVIP_compute_I_Gmv_Index_Ave(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(),1);
	} else {
		gmv_avg = scalerVIP_compute_I_Gmv_Index_Ave(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(),0);
	}

	teeth_detect_th_a_reg.regValue= rtd_inl(DI_IM_DI_MA_FIELD_TEETH_TH_A_reg);

	DI_Video_Broken=0;
	DI_Res_Pat_pan_65ppf = 0;

	//if(rtd_inl(0xb802e4f4)==15)
	//	rtd_pr_vpq_isr_info("\n gmv_avg %d ,Step_Hist[0] %X ,teeth_avg %d \n ",gmv_avg,Step_Hist[0] ,teeth_avg);

	if((gmv_avg>=gmv_avg_th)&&Step_Hist[0] <Step_Hist_th) // more still and high frequency less pattern
	{
		gmv_avg_th =50;
		Step_Hist_th = 0xc80;

#if 0
		if(teeth_avg>=100)
		{
			teeth_detect_th_a_reg.ma_fieldteethstha =40;
			DI_Video_Broken =1;
		}
		else if(teeth_avg<=70)
		{
			teeth_detect_th_a_reg.ma_fieldteethstha =0;
			DI_Video_Broken=0;

		}
		else
		{
			teeth_detect_th_a_reg.ma_fieldteethstha = (2*teeth_avg)/5;
			DI_Video_Broken=1;
		}
#else
		if(teeth_avg>=1023)
		{
			teeth_detect_th_a_reg.ma_fieldteethstha =40;
			DI_Video_Broken =1;
		}
		else
		{
			teeth_detect_th_a_reg.ma_fieldteethstha = 0;
			DI_Video_Broken=0;
		}


#endif

	}
	else
	{
		//rtd_pr_vpq_isr_info("\n rord fail  %d , %d  \n",gmv_avg,Step_Hist[0]);
		gmv_avg_th =55;
		Step_Hist_th = 0xc80;
		teeth_detect_th_a_reg.ma_fieldteethstha = 0;
	}


	if (gmv_avg <= 38 ){
			DI_Res_Pat_pan_65ppf = 1;
	}

	rtd_outl(DI_IM_DI_MA_FIELD_TEETH_TH_A_reg,teeth_detect_th_a_reg.regValue);

	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_107==TRUE))
	{
		ROSPrintf("\n gmv_avg %d , teeth_avg %d  \n ",gmv_avg,teeth_avg);
		ROSPrintf("\n DI_Video_Broken %d , Step_Hist[0] %d  \n ",DI_Video_Broken,Step_Hist[0]);

	}
#endif //mac6 no MCNR
}

void scalerVIP_DI_GMV_AVG_for_film()//for DTV576i
{
#if 0 //mac6 no MCNR, this api almost can't work
	#if 1
	unsigned int gmv_avg=0;
	if(RPC_SmartPic_clue->SceneChange == 1){
		gmv_avg = scalerVIP_compute_I_Gmv_Index_Ave(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(),1);
	} else	{
		gmv_avg = scalerVIP_compute_I_Gmv_Index_Ave(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(),0);
	}

	if( gmv_avg >= 90 )
		GMV_status_for_Filim = 1;
	else
		GMV_status_for_Filim = 0;

	#else
	short i = 0;
	unsigned char vpan_up_pass_total = 0, vpan_down_pass_total = 0, pass = 0;
	di_di_smd_gmvcoeff3_RBUS smd_gmvcoeff3_reg;
	static unsigned char Pass_Result_Sequence[VPAN_PASS_COUNT] = {0,0,0,0,0,0,0,0,0,0,
							      0,0,0,0,0,0,0,0,0,0,
							      0,0,0,0,0,0,0,0,0,0};

	smd_gmvcoeff3_reg.regValue = rtd_inl(DI_DI_SMD_GMVCOEFF3_reg);

	if (smd_gmvcoeff3_reg.smd_gmv2_x == 3 && (smd_gmvcoeff3_reg.smd_gmv2_y < 3 || smd_gmvcoeff3_reg.smd_gmv1_y <3))
		pass = 2; // move up
	else if (smd_gmvcoeff3_reg.smd_gmv2_x == 3 && (smd_gmvcoeff3_reg.smd_gmv2_y > 3 || smd_gmvcoeff3_reg.smd_gmv1_y > 3))
		pass = 1; // move down
	else
		pass = 0;

	// assign pass sequence
	for (i=VPAN_PASS_COUNT-2; i>=0; i--){
		Pass_Result_Sequence[i+1] = Pass_Result_Sequence[i];
	}
	Pass_Result_Sequence[0] = pass;

	for (i=0; i<VPAN_PASS_COUNT; i++)
	{
		if (Pass_Result_Sequence[i] == 2)
			vpan_up_pass_total++;
		else if (Pass_Result_Sequence[i] == 1)
			vpan_down_pass_total++;
	}

	if (vpan_up_pass_total > 12)
		VPan_status_for_Filim = 2;
	else if (vpan_down_pass_total > 12)
		VPan_status_for_Filim = 1;
	else
		VPan_status_for_Filim = 0;

	if((MA_print_count%debug_print_delay_time==0)&&(Start_Print.ID_Pattern_107==TRUE))
	{
		ROSPrintf("StatusUpdate : VPan_status_for_Filim = %d \n",VPan_status_for_Filim);
		ROSPrintf("StatusUpdate : gmv_y = %d %d \n",smd_gmvcoeff3_reg.smd_gmv1_y,smd_gmvcoeff3_reg.smd_gmv2_y);
		ROSPrintf("StatusUpdate : gmv_x = %d %d %d \n",smd_gmvcoeff3_reg.smd_gmv1_x,smd_gmvcoeff3_reg.smd_gmv2_x,smd_gmvcoeff3_reg.smd_gmv3_x);
		ROSPrintf("StatusUpdate : vpan_up_pass_total = %d \n",vpan_up_pass_total);
		ROSPrintf("StatusUpdate : vpan_down_pass_total = %d \n",vpan_down_pass_total);
	}




	#endif
#endif //mac6 no MCNR
}

char scalerVIP_PF_BlackDetection(void)
{
	_clues* SmartPic_clue=NULL;

	unsigned int IWid, ILen, ISTA_H, ISTA_V;
	static unsigned int IWid_LastFrame, ILen_LastFrame, ISTA_H_LastFrame, ISTA_V_LastFrame;
	static unsigned int BBD_Act_X_LastFrame, BBD_Act_Y_LastFrame, BBD_Act_W_LastFrame, BBD_Act_H_LastFrame;
	static unsigned char ISTA_SMF_MEASURED = 0;
	static unsigned int ISTA_H_SMF, ISTA_V_SMF;
	unsigned char SmallShift_TH = 2;
	smartfit_auto_field_RBUS smartfit_auto_field; // 2000
	smartfit_auto_h_boundary_RBUS smartfit_auto_h_boundary; //2004 hin
	smartfit_auto_v_boundary_RBUS smartfit_auto_v_boundary; //2008 vin
	smartfit_auto_r_g_b_margin_RBUS smartfit_auto_r_g_b_margin; //200c TH
	smartfit_auto_adj_RBUS smartfit_auto_adj; // 2010 start
	smartfit_auto_result_vsta_end_RBUS smartfit_auto_result_vsta_end; //2014 result v
	smartfit_auto_result_hsta_end_RBUS smartfit_auto_result_hsta_end; //2018 result h

	static unsigned int cnt, protect_cnt = 0;
	//unsigned char Detected = 0;

	SmartPic_clue = scaler_GetShare_Memory_SmartPic_Clue_Struct();

	if(SmartPic_clue==NULL){
		return 0;
	}

	//IWid = SmartPic_clue->SCALER_DISP_INFO.IphActWid_H;
	//ILen = SmartPic_clue->SCALER_DISP_INFO.IpvActLen_V;
	//ISTA_H = SmartPic_clue->SCALER_DISP_INFO.IpvActSTA_H;
	//ISTA_V = SmartPic_clue->SCALER_DISP_INFO.IpvActSTA_V;

	IWid = SmartPic_clue->SCALER_DISP_INFO.IphActWid_Pre;
	ILen = SmartPic_clue->SCALER_DISP_INFO.IpvActLen_Pre;
	ISTA_H =SmartPic_clue->SCALER_DISP_INFO.IphActSTA_Pre;
	ISTA_V =SmartPic_clue->SCALER_DISP_INFO.IpvActSTA_Pre;

	smartfit_auto_field.regValue = scaler_rtd_inl(SMARTFIT_AUTO_FIELD_reg);
	smartfit_auto_h_boundary.regValue = scaler_rtd_inl(SMARTFIT_AUTO_H_BOUNDARY_reg);
	smartfit_auto_v_boundary.regValue = scaler_rtd_inl(SMARTFIT_AUTO_V_BOUNDARY_reg);
	smartfit_auto_r_g_b_margin.regValue = scaler_rtd_inl(SMARTFIT_AUTO_R_G_B_MARGIN_reg);


	SmartPic_clue->BlackDetectionInfo.DetectFlag = 0;

	smartfit_auto_adj.regValue = scaler_rtd_inl(SMARTFIT_AUTO_ADJ_reg);
	if (IWid == IWid_LastFrame && ILen == ILen_LastFrame && ISTA_H == ISTA_H_LastFrame && ISTA_V == ISTA_V_LastFrame) {


		if (smartfit_auto_adj.now_af ==0 ) {
			smartfit_auto_result_vsta_end.regValue = scaler_rtd_inl(SMARTFIT_AUTO_RESULT_VSTA_END_reg);
			smartfit_auto_result_hsta_end.regValue = scaler_rtd_inl(SMARTFIT_AUTO_RESULT_HSTA_END_reg);

			#if 0 /* Debug Log*/
			if (cnt %300 ==0) {
				rtd_pr_vpq_isr_info("cnt=%d, IWid_Pre = %d, ILen_Pre = %d\n",cnt, IWid,ILen);
				rtd_pr_vpq_isr_info("ISTA_H_Pre = %d, ISTA_V_Pre = %d\n",ISTA_H,ISTA_V);
				rtd_pr_vpq_isr_info("x1 = %d, x2 = %d\n",smartfit_auto_result_vsta_end.regValue,smartfit_auto_result_hsta_end.regValue);
				rtd_pr_vpq_isr_info("ISTA_H = %d, ISTA_V = %d, IWid = %d, ILen = %d\n", SmartPic_clue->SCALER_DISP_INFO.IpvActSTA_H, SmartPic_clue->SCALER_DISP_INFO.IpvActSTA_V, SmartPic_clue->SCALER_DISP_INFO.IphActWid_H, SmartPic_clue->SCALER_DISP_INFO.IpvActLen_V);
				rtd_pr_vpq_isr_info("HTotal = %d, VTotal=%d\n", Scaler_DispGetInputInfo(SLR_INPUT_H_LEN), Scaler_DispGetInputInfo(SLR_INPUT_V_LEN));
				rtd_pr_vpq_isr_info("ISTA_H_SMF = %d, ISTA_V_SMF = %d\n",ISTA_H_SMF,ISTA_V_SMF);
			}
			#endif

			/* protection*/
			if (smartfit_auto_result_vsta_end.vx_sta== 0 && smartfit_auto_result_vsta_end.vx_end ==0 && smartfit_auto_result_hsta_end.hx_sta == 0x3fff && smartfit_auto_result_hsta_end.hx_end == 0) {
				SmartPic_clue->BlackDetectionInfo.DetectFlag = 0;
				smartfit_auto_adj.now_af = 1;
				scaler_rtd_outl(SMARTFIT_AUTO_ADJ_reg, smartfit_auto_adj.regValue); // en
			} else {
				if (ISTA_SMF_MEASURED == 0) {
					ISTA_SMF_MEASURED = 1;

					ISTA_H_SMF = smartfit_auto_result_hsta_end.hx_sta;
					ISTA_V_SMF = smartfit_auto_result_vsta_end.vx_sta;

					smartfit_auto_h_boundary.hb_sta = ISTA_H_SMF;
					smartfit_auto_h_boundary.hb_end = (ISTA_H_SMF + IWid-1);
					smartfit_auto_v_boundary.vb_sta = ISTA_V_SMF;
					smartfit_auto_v_boundary.vb_end= (ISTA_V_SMF+ILen-1);
					smartfit_auto_r_g_b_margin.red_nm = 63;
					smartfit_auto_r_g_b_margin.grn_nm = 5;
					smartfit_auto_r_g_b_margin.blu_nm = 63;


				} else {
					/*update BBD result*/
					if (abs_value(((int)smartfit_auto_result_hsta_end.hx_sta-(int)ISTA_H_SMF)-(int)BBD_Act_X_LastFrame)>SmallShift_TH ||
				            abs_value(((int)smartfit_auto_result_vsta_end.vx_sta-(int)ISTA_V_SMF)-(int)BBD_Act_Y_LastFrame)>SmallShift_TH ||
				            abs_value(((int)smartfit_auto_result_hsta_end.hx_end-(int)smartfit_auto_result_hsta_end.hx_sta+1)-(int)BBD_Act_W_LastFrame)>SmallShift_TH ||
				            abs_value(((int)smartfit_auto_result_vsta_end.vx_end-(int)smartfit_auto_result_vsta_end.vx_sta+1)-(int)BBD_Act_H_LastFrame)>SmallShift_TH) {

						SmartPic_clue->BlackDetectionInfo.DetectFlag = 1;
						SmartPic_clue->BlackDetectionInfo.OriRegion.x = 0;
						SmartPic_clue->BlackDetectionInfo.OriRegion.y = 0;
						SmartPic_clue->BlackDetectionInfo.OriRegion.w = IWid;
						SmartPic_clue->BlackDetectionInfo.OriRegion.h = ILen;

						SmartPic_clue->BlackDetectionInfo.ActRegion.x = smartfit_auto_result_hsta_end.hx_sta-ISTA_H_SMF;
						SmartPic_clue->BlackDetectionInfo.ActRegion.y = smartfit_auto_result_vsta_end.vx_sta-ISTA_V_SMF;
						SmartPic_clue->BlackDetectionInfo.ActRegion.w = (smartfit_auto_result_hsta_end.hx_end-smartfit_auto_result_hsta_end.hx_sta+1);
						SmartPic_clue->BlackDetectionInfo.ActRegion.h = (smartfit_auto_result_vsta_end.vx_end-smartfit_auto_result_vsta_end.vx_sta+1);
						#if 0 /* Debug Log*/
						rtd_pr_vpq_isr_info("Difference : xdiff = %d ydiff =%d, wdiff=%d, hdiff=%d\n", abs_value((int)(smartfit_auto_result_hsta_end.hx_sta-(int)ISTA_H_SMF)-(int)BBD_Act_X_LastFrame),
							abs_value(((int)smartfit_auto_result_vsta_end.vx_sta-(int)ISTA_V_SMF)-(int)BBD_Act_Y_LastFrame),
							abs_value(((int)smartfit_auto_result_hsta_end.hx_end-(int)smartfit_auto_result_hsta_end.hx_sta+1)-(int)BBD_Act_W_LastFrame),
							abs_value(((int)smartfit_auto_result_vsta_end.vx_end-(int)smartfit_auto_result_vsta_end.vx_sta+1)-(int)BBD_Act_H_LastFrame));
						#endif
					}
					/*Set next time detection*/
					smartfit_auto_h_boundary.hb_sta = ISTA_H_SMF;
					smartfit_auto_h_boundary.hb_end = (ISTA_H_SMF + IWid-1);
					smartfit_auto_v_boundary.vb_sta = ISTA_V_SMF;
					smartfit_auto_v_boundary.vb_end= (ISTA_V_SMF+ILen-1);
					smartfit_auto_r_g_b_margin.red_nm = 63;
					smartfit_auto_r_g_b_margin.grn_nm = 5;
					smartfit_auto_r_g_b_margin.blu_nm = 63;

					/*update detection info*/
					BBD_Act_X_LastFrame = smartfit_auto_result_hsta_end.hx_sta-ISTA_H_SMF;
					BBD_Act_Y_LastFrame = smartfit_auto_result_vsta_end.vx_sta-ISTA_V_SMF;
					BBD_Act_W_LastFrame = smartfit_auto_result_hsta_end.hx_end-smartfit_auto_result_hsta_end.hx_sta+1;
					BBD_Act_H_LastFrame = smartfit_auto_result_vsta_end.vx_end-smartfit_auto_result_vsta_end.vx_sta+1;


				}

				smartfit_auto_field.smartfit_src_sel = 1;
				smartfit_auto_field.adj_source = 0;
				scaler_rtd_outl(SMARTFIT_AUTO_FIELD_reg, smartfit_auto_field.regValue);
				scaler_rtd_outl(SMARTFIT_AUTO_H_BOUNDARY_reg, smartfit_auto_h_boundary.regValue);
				scaler_rtd_outl(SMARTFIT_AUTO_V_BOUNDARY_reg, smartfit_auto_v_boundary.regValue);
				scaler_rtd_outl(SMARTFIT_AUTO_R_G_B_MARGIN_reg, smartfit_auto_r_g_b_margin.regValue);

				smartfit_auto_adj.m_vgen_en = 0;
				smartfit_auto_adj.now_af = 1;
				scaler_rtd_outl(SMARTFIT_AUTO_ADJ_reg, smartfit_auto_adj.regValue); // en

			}


		} else {
			protect_cnt++;
			if (protect_cnt%10==0) {
				smartfit_auto_adj.now_af = 0;
				scaler_rtd_outl(SMARTFIT_AUTO_ADJ_reg, smartfit_auto_adj.regValue); // en
				protect_cnt = 0;

				smartfit_auto_adj.now_af = 1;
				scaler_rtd_outl(SMARTFIT_AUTO_ADJ_reg, smartfit_auto_adj.regValue); // en
				//rtd_pr_vpq_isr_info("[SMF] AF Time out\n");

			}


		}
	}else {
		smartfit_auto_adj.now_af = 0; // stop
		scaler_rtd_outl(SMARTFIT_AUTO_ADJ_reg, smartfit_auto_adj.regValue); // en

		smartfit_auto_field.smartfit_src_sel = 1;
		smartfit_auto_field.adj_source = 0;
		smartfit_auto_h_boundary.hb_sta = 0;
		smartfit_auto_h_boundary.hb_end = Scaler_DispGetInputInfo(SLR_INPUT_H_LEN)-1;
		smartfit_auto_v_boundary.vb_sta = 0;
		smartfit_auto_v_boundary.vb_end= Scaler_DispGetInputInfo(SLR_INPUT_V_LEN)-1;
		smartfit_auto_r_g_b_margin.red_nm = 4;
		smartfit_auto_r_g_b_margin.grn_nm = 4;
		smartfit_auto_r_g_b_margin.blu_nm = 4;

		scaler_rtd_outl(SMARTFIT_AUTO_FIELD_reg, smartfit_auto_field.regValue);
		scaler_rtd_outl(SMARTFIT_AUTO_H_BOUNDARY_reg, smartfit_auto_h_boundary.regValue);
		scaler_rtd_outl(SMARTFIT_AUTO_V_BOUNDARY_reg, smartfit_auto_v_boundary.regValue);
		scaler_rtd_outl(SMARTFIT_AUTO_R_G_B_MARGIN_reg, smartfit_auto_r_g_b_margin.regValue);

		smartfit_auto_adj.m_vgen_en = 0;
		smartfit_auto_adj.now_af = 1;
		scaler_rtd_outl(SMARTFIT_AUTO_ADJ_reg, smartfit_auto_adj.regValue); // en

		ISTA_SMF_MEASURED = 0;

	}

	IWid_LastFrame = IWid;
	ILen_LastFrame = ILen;
	ISTA_H_LastFrame = ISTA_H;
	ISTA_V_LastFrame = ISTA_V;
	cnt ++;
	return 0;

}


void scalerVIP_Set_BlackDetection_EN(unsigned char bEnable)
{
	BlackDetection_en = bEnable;


}
void scalerVIP_Set_pq_reset(unsigned char bEnable)
{
	pq_reset = bEnable;


}
unsigned char ScalerVIP_Get_pq_reset(void)
{
	return pq_reset;
}

unsigned char scalerVIP_access_tv002_style_demo_flag(unsigned char access_mode, unsigned char *pFlag)
{
	static unsigned char flag_tv002_style_demo = 0;
	if (access_mode == 0)
		return flag_tv002_style_demo;
	else if (access_mode == 1) {
		if (pFlag != NULL)
			flag_tv002_style_demo = *pFlag;
		else
			return 0;
	}
	return 1;
}

void* scalerVIP_get_tv002_style_demo_struct(void)
{
	extern  SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;

	return (void*)&m_customVipTable_demo_tv002;
}

void scalerVIP_VO_resolution_change_hack(void)
{
	static unsigned char mode_pre = 0xff;
	unsigned char mode = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);

	if (system_info_structure_table == NULL)
		return;

	if (system_info_structure_table->Input_src_Form != _SRC_FROM_VO)
		return;

	if (mode_pre != mode)
		Scaler_recheck_sharp_nr_table();

	mode_pre = mode;
}

#if 0	/* move to "scalerVIP_Dynamic_DeXCXL_CTRL()" */
void scalerVIP_Set_DeXC_OnOff_by_PlaySpeed(_RPC_system_setting_info* system_RPC_info_structure_table)
{
#ifdef CONFIG_HW_SUPPORT_I_DE_XC
	static int pre_speed = 0;
	int mSpeed;
	static int pre_frmrate = -1;
	int frmrate;
	unsigned char de_xc_tbl_sel = 0;
	unsigned int de_xc_phyaddr = 0;

	static int print_count = 0;

	de_xcxl_de_xcxl_ctrl_RBUS de_xcxl_de_xcxl_ctrl_reg;
	de_xcxl_de_xcxl_db_reg_ctl_RBUS de_xcxl_de_xcxl_db_reg_ctl_reg;

	de_xcxl_de_xcxl_ctrl_reg.regValue = rtd_inl(DE_XCXL_De_XCXL_CTRL_reg);

	mSpeed = Scaler_ChangeUINT32Endian( RPC_SmartPic_clue->MEMC_VCPU_setting_info.vo_channel_speed );
	frmrate = Scaler_VOInfoPointer(0)->v_freq;

	print_count++;
	if( print_count % 600 == 0 )
	{
		rtd_pr_vpq_isr_debug("[scalerVIP_Set_DeXC_OnOff_by_PlaySpeed] play speed = %d\n", mSpeed );
		rtd_pr_vpq_isr_debug("[scalerVIP_Set_DeXC_OnOff_by_PlaySpeed] cur frame rate = %d\n", frmrate );
		print_count = 0;
	}

	if( mSpeed != pre_speed )
	{
		// disable DeXCXL if playback speed is not 1x
		if( mSpeed != 256 )
		{
			de_xcxl_de_xcxl_ctrl_reg.dexc_en = 0;
		}
		else
		{
			de_xc_tbl_sel = system_RPC_info_structure_table->I_De_XC_CTRL.table_select;
			de_xc_phyaddr = system_RPC_info_structure_table->DE_XCXL_Memory_CTRL[0].phyaddr;
			de_xcxl_de_xcxl_ctrl_reg.dexc_en = system_RPC_info_structure_table->I_De_XC_CTRL.De_XC_TBL[de_xc_tbl_sel].dexc_en;

			if( de_xc_phyaddr == 0 )
				de_xcxl_de_xcxl_ctrl_reg.dexc_en = 0;
		}

		rtd_outl( DE_XCXL_De_XCXL_CTRL_reg, de_xcxl_de_xcxl_ctrl_reg.regValue );

		// db apply
		de_xcxl_de_xcxl_db_reg_ctl_reg.regValue = rtd_inl(DE_XCXL_DE_XCXL_db_reg_ctl_reg);
		de_xcxl_de_xcxl_db_reg_ctl_reg.db_apply = 1;
		rtd_outl( DE_XCXL_DE_XCXL_db_reg_ctl_reg, de_xcxl_de_xcxl_db_reg_ctl_reg.regValue );
	}

	// dexc mode auto control, only enable PAL mode @25 or 50Hz
	if( (frmrate >= 248 && frmrate <= 252) ||( (frmrate >= 498) && (frmrate <= 502)) )
	{
		de_xcxl_de_xcxl_ctrl_reg.dexc_detect_type = 2; // Both NTSC & PAL
	}
	else
	{
		de_xcxl_de_xcxl_ctrl_reg.dexc_detect_type = 0; // NTSC only
	}
	rtd_outl( DE_XCXL_De_XCXL_CTRL_reg, de_xcxl_de_xcxl_ctrl_reg.regValue );

	pre_speed = mSpeed;
	pre_frmrate = frmrate;
#endif //endif
}
#endif


#ifdef BRING_UP_VERIFY_PQ//for bring up PQ default setting
void PQ_verify_initial(void)
{//for bring up PQ default setting
}
#endif

#if defined(CONFIG_RTK_AI_DRV)
int ScalerVIP_SE_Proc(void)
{
#ifdef CONFIG_RTK_KDRV_SE
	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;
	unsigned char SE_rdPtr = 0;
	unsigned char SE_status = 0;
	unsigned char SE_rdPtr_new = 0;
	unsigned char SE_reset_flag_new = 0;
	unsigned char SE_flush_flag_new = 0;
	UINT8  u8_reset_flag;
	UINT8  u8_flush_flag;

	RPC_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();

	if((RPC_system_info_structure_table==NULL))
	{
		ROSPrintf(" Mem4 = %p\n",RPC_system_info_structure_table);
		return -1;
	}

	preempt_disable();
	_rtd_hwsem_lock(SB2_HD_SEM_NEW_3_reg, SEMA_HW_SEM_3_SCPU_2);
	SE_rdPtr = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.rdPtr;
	SE_status = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.pic[SE_rdPtr].status;
	u8_reset_flag = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.reset_flag;
	u8_flush_flag = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.flush_flag;
	_rtd_hwsem_unlock(SB2_HD_SEM_NEW_3_reg, SEMA_HW_SEM_3_SCPU_2);
	preempt_enable();

	if(SE_status == 1){

		scalerAI_preprocessing();

		//update data
		preempt_disable();
		_rtd_hwsem_lock(SB2_HD_SEM_NEW_3_reg, SEMA_HW_SEM_3_SCPU_2);
		SE_rdPtr_new = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.rdPtr;
		SE_reset_flag_new = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.reset_flag;
		SE_flush_flag_new = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.flush_flag;

		if((SE_rdPtr == SE_rdPtr_new) && (u8_reset_flag == SE_reset_flag_new) && (u8_flush_flag == SE_flush_flag_new)){

			RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.pic[SE_rdPtr].status = 2;//OUTPUT;
			RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.rdPtr = (SE_rdPtr + 1) % MAX_FILM_BUFF;
			//rtd_pr_vpq_isr_emerg("lsy check end\n");
		}

		if(u8_reset_flag)
			RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.reset_flag = 0;

		if(u8_flush_flag)
			RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.flush_flag = 0;
		_rtd_hwsem_unlock(SB2_HD_SEM_NEW_3_reg, SEMA_HW_SEM_3_SCPU_2);
		preempt_enable();

	}
	else{
		//rtd_pr_vpq_isr_notice("%s(%d) vdec buffer status %d\n", __func__, __LINE__, SE_status);
	}

#endif
	return 0;

}
#endif
