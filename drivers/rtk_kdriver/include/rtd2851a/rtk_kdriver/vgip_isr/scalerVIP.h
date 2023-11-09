/***************************************************************************
                          scalerVIP.h  -  description
                             -------------------
    begin                : Mon Dec 22 2008
    copyright           : (C) 2008 by hsliao
    email                : hsliao@realtek.com
 ***************************************************************************/

#ifndef __ISR_SCALERVIP_H__
#define __ISR_SCALERVIP_H__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/uaccess.h>

#include <scaler/vipCommon.h>
#include <scaler/vipRPCCommon.h>
#include <scaler/scalerCommon.h>
#include <scaler/vdcCommon.h>
//#include "mach/io.h"
#include "tvscalercontrol/io/ioregdrv.h"

#include "tvscalercontrol/scaler/vipinclude.h"
#include "tvscalercontrol/vip/color.h"
#include "tvscalercontrol/vip/scalerColor.h"
#include "rtk_vip_logger.h"

/*===================================  Types ================================*/
//#define LG_Demo_flag 0
#define LG_Demo_flag 1
#define CB_flag 0

#ifndef SUCCESS
  #define SUCCESS                   0
#endif

#ifndef FAILED
  #define FAILED                    (-1)
#endif

#ifndef _ON
#define _ON	1
#endif

#ifndef _OFF
#define _OFF 0
#endif



#define rtdf_clearBits(offset, Mask) 		rtdf_outl(offset, ((rtdf_inl(offset) & ~(Mask))))

#define ROSPrintf(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VGIP_VIP_DEBUG,0,fmt,##args)
//#define ROSPrintf(fmt, args...)
#define scalerDrvPrint ROSPrintf

//#define ISR_RTNR_SWITCH_CORRECTION_BIT_FUNCTION_ENABLE 1
//#define ISR_AMBILIGHT_ENABLE 1
//#define ISR_ICM_ADJUST_FUNCTIOM_ENABLE 1
#define ISR_LG_DEMO_FLAG 1
#define ISR_NEW_RTNR_ENABLE 1
//#define ISR_OLD_RTNR_ENABLE 1
//#define ISR_SHARPNESS_BY_MEAN_ENABLE 1
//#define ISR_NR_IMPULSE_FUNCTIOM_ENABLE 1

#define VIP_TG45_FLAG_NUM 6

#define debug_print_delay_time ((Start_Print.Delay_Time == 0)?(1):(Start_Print.Delay_Time))

extern unsigned char SceneChange;
extern unsigned char ucDCREnable;  // Erin 100311 from pacific
extern unsigned char TG45_Flag;

#define CLIP(min, max, val) (((val) < (min)) ? (min) : ((val) > (max) ? (max) : (val)))

typedef struct
{
 unsigned int ENL_Y;
 unsigned int ENL_U;
 unsigned int ENL_V;
}_rtnr_enl;

typedef struct
{
	unsigned char Stop_All;
	unsigned char Stop_Di_Hmc;
	unsigned char Stop_Dcc;
	unsigned char Stop_Auto_Nr;
	unsigned char Stop_Dcti_for_colorbar;
	unsigned char Stop_Noise_Mpeg;
	unsigned char Stop_Dcr;
	unsigned char Stop_Film_Mode_Detector;
	unsigned char Stop_VD_Noise;
	unsigned char Stop_Di_Smd;
	unsigned char Stop_pattern_Monscope;
	unsigned char Stop_pattern_ColorBar;
	unsigned char Stop_pattern_No2;
	unsigned char Stop_pattern_No4;
	unsigned char Stop_pattern_No23;
	unsigned char Stop_pattern_No24;
	unsigned char Stop_pattern_No47;
	unsigned char Stop_pattern_No107;
	unsigned char Stop_pattern_No154;
	unsigned char Stop_Reset_Pattern;

}Stop_Polling_t;

typedef struct
{
	unsigned char Delay_Time;
	unsigned char Y_Hist;
	unsigned char Hue_Hist;
	unsigned char Sat_Mean;
	unsigned char Dcc_1;
	unsigned char Dcc_2;
	unsigned char DCC_3;
	unsigned char Rtnr_1;
	unsigned char Rtnr_2;
	unsigned char Mpeg_Nr;
	unsigned char Snr;
	unsigned char Sharpness_1;
	unsigned char Sharpness_2;
	unsigned char DI_1;
	unsigned char DI_2;
	unsigned char Film_Mode;
	unsigned char HIST;
	unsigned char Reverse_2;
	unsigned char Reverse_3;
	unsigned char ID_Pattern_ALL;
	unsigned char ID_Pattern_2;
	unsigned char ID_Pattern_4;
	unsigned char ID_Pattern_5;
	unsigned char ID_Pattern_7;
	unsigned char ID_Pattern_12;
	unsigned char ID_Pattern_23;
	unsigned char ID_Pattern_24;
	unsigned char ID_Pattern_35;
	unsigned char ID_Pattern_47;
	unsigned char ID_Pattern_107;
	unsigned char ID_Pattern_123;
	unsigned char ID_Pattern_132;
	unsigned char ID_Pattern_133;
	unsigned char ID_Pattern_154;
	unsigned char ID_Pattern_191;
	unsigned char ID_Pattern_192;
	unsigned char ID_Pattern_193;
	unsigned char ID_Pattern_194;
	unsigned char ID_Pattern_195;
	unsigned char ID_Pattern_139;
	unsigned char ID_Pattern_196;
	unsigned char ID_Pattern_14;
	unsigned char someothers;
	unsigned char ID_Pattern_32;
	unsigned char ID_Pattern_147;
	unsigned char Reverse_10;
	unsigned char Reverse_11;
	unsigned char VD_LOG_ENABLE;
	unsigned char ColorBar;
	unsigned char Slow_Motion1;
	unsigned char Slow_Motion2;
	unsigned char RF_flower;

}Start_Print_info_t;

//DCR mode
typedef enum _DCR_MODE_ISR{
    DCR_MASTER_ISR,
    DCR_SLAVE_ISR,
    DCR_MODE_MAX_ISR,
}DCR_MODE_ISR;

typedef enum _MOTION_LEVEL{
    MOTION_LEVEL_MOTION,
    MOTION_LEVEL_VERY_STILL,
    MOTION_LEVEL_STILL,
    MOTION_LEVEL_SLOW_MOTION,
    MOTION_LEVEL_MAX,
}MOTION_LEVEL;

typedef enum _MOTION_NOISE_TYPE {
	MOTION_NOISE_TYPE_STILL,
	MOTION_NOISE_TYPE_MOTION,
	MOTION_NOISE_TYPE_LARGE_MOTION,
	MOTION_NOISE_TYPE_NOISE,
}MOTION_NOISE_TYPE;
/*
// oooo-------- for 3D mode --------oooo
// Copy from scalerVOCommon.h, for 3D detection use.
typedef enum {
	HDMI3D_FRAME_PACKING = 0,
	HDMI3D_FIELD_ALTERNATIVE=1,
	HDMI3D_LINE_ALTERNATIVE=2,
	HDMI3D_SIDE_BY_SIDE_FULL=3,
	HDMI3D_L_DEPTH=4,
	HDMI3D_L_DEPTH_GPX=5,
	HDMI3D_TOP_AND_BOTTOM=6,
	HDMI3D_FRAMESEQUENCE=7,
	HDMI3D_SIDE_BY_SIDE_HALF=8,
	// --- new 3D format ---
	HDMI3D_VERTICAL_STRIP,
	HDMI3D_CHECKER_BOARD,
	HDMI3D_REALID,
	HDMI3D_SENSIO,
	// -------------------
	HDMI3D_SIDE_BY_SIDE_HALF_CVT_2D,
	HDMI3D_TOP_AND_BOTTOM_CVT_2D,
	HDMI3D_FRAME_PACKING_CVT_2D,
	// --- new 3D format ---
	HDMI3D_LINE_ALTERNATIVE_CVT_2D,
	HDMI3D_VERTICAL_STRIP_CVT_2D,
	HDMI3D_CHECKER_BOARD_CVT_2D,
	HDMI3D_REALID_CVT_2D,
	HDMI3D_SENSIO_CVT_2D,
	// -------------------
	HDMI3D_2D_CVT_3D,
	HDMI3D_2D_ONLY,
	HDMI3D_UNKOWN = 0xFFFFFFFF
} HDMI3D_T;
*/
typedef enum _PATTERN_LEVEL{
	NTSC_IRE00_HIGH = 0,
	NTSC_IRE75_HIGH,
	PAL_AUTO,
	SD_HIGH,
	HD_HIGH,
	NTSC_IRE00_LOW,
	NTSC_IRE75_LOW,
	SD_LOW,
	HD_LOW,

}PATTERN_LEVEL;

/* ==========for MEMC infor jzl 20151231============================ */

typedef struct
{
	unsigned char 	bSRC_VO;	///< ScalerVIP status
	unsigned char 	bHDR_Chg;	///< ScalerVIP status
	unsigned short	mBrightness;  ///< ScalerVIP status
	unsigned short	mContrast;    ///< ScalerVIP status
        short	mSpeed;	 ///< ScalerVIP status
	BOOL	bZoomAction;		///< ScalerVIP status
	BOOL      bShiftAction;	///< ScalerVIP status
	unsigned short	source_type;
	unsigned char mVOSeek;
	UINT16	MEMCMode;		///< MEMC mode
	unsigned char   mDarkLevel; ///< Hist dark level
	BOOL	Hist_SC_flg;
	unsigned char	mSoccer_holdfrm; ///< soccer patch
	BOOL		PictureMode_Chg;
	BOOL		bVIP_still;
	UINT16	Forbes_flg;
}SET_MEMC_SCALER_VIP_INFO;

void scalerVIP_Set_Info_toMEMC(void);
#ifdef CONFIG_HW_SUPPORT_MEMC
void scalerVIP_Set_MEMC_Info_toVCPU(void);
#endif

SET_MEMC_SCALER_VIP_INFO scalerVIP_Get_Info_fromScaler(void);

/* ==========for MEMC infor jzl 20151231============================ */
/* ====================================== */
void scalerVIP_colorMaAutoISR(SCALER_CHANNEL_NUM channel);

void scaler_Set_Dcr_Panasonic_Enable(unsigned long arg);
void scaler_Set_Dcr_Powersave_Enable(unsigned long arg);

//=================== DCL ========================
//=================== Skin Tone Detection ========================
void skin_tone_detect(unsigned char channel, unsigned int width, unsigned int height);
void skin_tone_Level_detect(unsigned char channel, unsigned int width, unsigned int height);

//=================== New, copy from Pacific ========================
void drvif_color_dnewdcti_table_isr(DRV_VipNewDDcti_Table* ptr, unsigned char offset_gain);
//================================

unsigned char drvif_color_check_source_remmping(unsigned char display);

//===========================================
// Erin 010310
int abs_dcc( int input );
unsigned int Lerp(unsigned int X_current, unsigned int X_1, unsigned int X_0, unsigned int Y_1, unsigned int Y_0, unsigned int x_offset_1, unsigned int x_offset_0);

unsigned int di_pow(unsigned int x, unsigned int p);
void drif_color_Edge_Noise_level(unsigned char ch, _rtnr_enl *ENL);

int drvif_color_colorbar_dectector(_clues* SmartPic_clue);
int drvif_color_colorbar_dectector_by_SatHueProfile(_clues* SmartPic_clue);
//==========================================
void drvif_color_switch_rtnr_rounding_correction_bit(unsigned char switch_on); //add for  TPV 0,3,5 IRE Pattern
//==========================================

void drvif_color_ClueGather( unsigned char ch, _clues* clueA);
void drvif_color_CluePrinter(_clues* clueA);
//=======DCR FUNCTION=====
void scalerVIP_DCR_OLD(void);

//==========
void scalerVIP_Profile_Motion_Detect(unsigned char display, _clues* SmartPic_clue);
unsigned char scalerVIP_Get_Profile_Motion(void);
void drvif_color_ma_DCR_new(unsigned short Frame_need_SC, unsigned short Frame_need_NML, unsigned short stable_buf_thl, unsigned short DZ_break_thl);

void drvif_color_ma_hfd_problem(void);
void drvif_color_ma_TeethCheck(unsigned char Level);
int drvif_Get_Purecolor_Weighting(void);
void scalerVIP_colorHistDetect(unsigned int en_test);

unsigned char pure_color_detect(unsigned char channel, unsigned int width, unsigned int height);
unsigned char pure_color_detect_Sony(unsigned char channel, unsigned int width, unsigned int height);
void drvif_color_rtnr_y_th_isr(unsigned char channel, unsigned char *Th_Y);

void scalerVIP_Share_Memory_Access_VIP_TABLE_CRC_Struct(unsigned long arg);
void fwif_color_PrintArray_UINT32(unsigned int * pwTemp ,int nSize,int nID);
void fwif_color_PrintArray_INT32(int * pwTemp ,int nSize,int nID);
void fwif_color_PrintArray_UINT16(unsigned short * pwTemp ,int nSize,int nID);
void fwif_color_PrintArray_INT16(short * pwTemp ,int nSize,int nID);
void fwif_color_PrintArray_UINT8(unsigned char * pwTemp ,int nSize,int nID);
void* scalerVIP_Get_Share_Memory_VIP_TABLE_Custom_Struct(void);

unsigned char black_white_pattern_detect(void);

/* DCR */
void scalerVIP_Set_DCR_BackLightMappingRange(void);
void scalerVIP_SetIsIncreseMode(unsigned char arg);//Leo Chen 20101001

void scalerVIP_SetDcrUIMaxValue(unsigned int arg);//Leo Chen 20101111
void scalerVIP_SetDcrUIMinValue(unsigned int arg);//Leo Chen 20101111
void scalerVIP_SetDcrOnOff(unsigned char arg);//Leo Chen 20101111
void scalerVIP_GetBlLvFromUser(unsigned char arg);//Leo Chen 20101001
void scalerVIP_SetBlLvActMax(unsigned int arg);//Leo Chen 20101111
void scalerVIP_SetBlLvActMin(unsigned int arg);//Leo Chen 20101111

//Sony radio detector
unsigned char Sony_radio_detect(unsigned char channel, unsigned int width, unsigned int height);

unsigned char DI_detect_Champagne(unsigned int width, unsigned int height);//sony demo
void DI_Champagne_problem(void);//sony demo

unsigned char motion_purecolor_concentric_circles_detect(unsigned int width, unsigned int height);//sony demo
unsigned char motion_move_concentric_circles_detect(unsigned int width, unsigned int height);//sony demo
unsigned char motion_concentric_circles_detect(unsigned int width, unsigned int height);

void scaler_Set_VIP_info_structure(unsigned char display,_clues* SmartPic_clue);
unsigned char drvif_color_activeregion_dragon(unsigned char display, unsigned short *Region);
void drvif_color_rtnr_y_k(unsigned char channel, unsigned int Y_K0_K7, unsigned int Y_K8_K15);
void drvif_color_rtnr_c_k(unsigned char channel, unsigned int K_C);
void drvif_color_rtnr_c_th_isr(unsigned char channel, unsigned char *Th_C);
unsigned char drvif_module_vdc_ReadMode(GET_VIDEO_MODE_METHOD result);

void drvif_color_newrtnr_rtnr_k(unsigned char row_select, unsigned char *th);
void drvif_color_newrtnr_sad_th(unsigned char edge, unsigned char *th);
void drvif_color_newrtnr_sta_th(unsigned char *th);
void drvif_color_newrtnr_edgetypediff_th(unsigned char *th);
void fwif_color_new_rtnr_adjust(unsigned char channel, unsigned char RTNR_level);
void drvif_color_Mixer_ctrl_Vfilter(unsigned char channel, int vfilter_weight);

unsigned char Scaler_GetAutoMA(unsigned char flag);

//yuan1024 add for rtnr n fiels avg
unsigned char VD_noise_Status_sum_avg(void);

void scalerVIP_Difference_H_V_Calculate(void);
//20130402 Elsie
MOTION_LEVEL scalerVIP_StillMotion(_clues* SmartPic_clue);
MOTION_LEVEL scalerVIP_MADStillMotion(void);

//20130416 Elsie
unsigned char scalerVIP_BigSmoothAreaDetection_luma(void);
unsigned char scalerVIP_BigSmoothAreaDetection_chroma(void);

//20130425 Elsie
void scalerVIP_color_noise_mpeg_detect(void);
void scalerVIP_color_noise_mpeg_v_detect(void);// S4Ap K4Lp
void scalerVIP_color_noise_mpeg_apply(void);
void scalerVIP_color_noise_mpeg_v_apply(void);// S4Ap K4Lp

void drvif_color_inewdcti_set(unsigned char display, unsigned char data_sel,  unsigned char maxlen, unsigned char psmth, unsigned char lpmode, unsigned char engdiv, unsigned char offdiv, unsigned char uvgain, unsigned char dcti_mode, unsigned char cur_sel, unsigned char dcti_en);
void drvif_color_inewdcti_table_isr(DRV_VipNewIDcti_Table* ptr, unsigned char offset_gain);
void scalerVIP_DCR(void);
void scalerVIP_setDcrTable(void);

void drvif_color_DCTI_for_ColorBar(unsigned char color_bar_flag,unsigned char colorbar_score);
unsigned char drvif_color_DCTI_auto_compute(DRV_VipNewDcti_auto_adjust* ptr,unsigned char colorbar_score);
void drvif_color_DELAY_for_ColorBar(unsigned char color_bar_flag,unsigned char colorbar_score);


//Elsie 20130521
unsigned char scalerVIP_ColorbarDetector(_clues* SmartPic_clue);
char scalerVIP_ReadProfile(_clues* SmartPic_clue);

void scalerVIP_scalerColor_profile(void);
void scalerVIP_fwif_color_set_profile(void);
void scalerVIP_drvif_color_profile(DRV_Vip_Profile *ptr);
char scalerVIP_ReadProfile1_Y(_clues* SmartPic_clue);
char scalerVIP_ReadProfile1_U(_clues* SmartPic_clue);
char scalerVIP_ReadProfile1_V(_clues* SmartPic_clue);
char scalerVIP_ReadProfile2_Y(_clues* SmartPic_clue);
char scalerVIP_ReadProfile2_U(_clues* SmartPic_clue);
char scalerVIP_ReadProfile2_V(_clues* SmartPic_clue);
char scalerVIP_ReadProfile3_Y(_clues* SmartPic_clue);
char scalerVIP_ReadProfile3_U(_clues* SmartPic_clue);
char scalerVIP_ReadProfile3_V(_clues* SmartPic_clue);

//Y and H hist ratio calculate, Y U V motion calculate,elieli
void scalerVIP_YHist_HueHist_Ratio_Calculate(void);
void scalerVIP_Y_U_V_Mad_Calculate(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table/*, _RPC_system_setting_info* RPC_system_info_structure_table*/);
void scalerVIP_HMCNR_statistic(void);

unsigned char scalerVIP_ET_Y_MOTION(void) ;
void scalerVIP_Sat_Hist_Calculate(void);
void scalerVIP_FMV_HMCbin_hist_ratio_Cal(SLR_VIP_TABLE* Share_Memory_VIP_TABLE_Struct, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table);

unsigned short scalerVIP_ratio_inner_product_base(unsigned int *vector1, unsigned int *vector2, unsigned char length);
unsigned short scalerVIP_ratio_inner_product_base_16(unsigned short *vector1, unsigned short *vector2, unsigned char length);
unsigned short scalerVIP_ratio_inner_product(unsigned short *vector1, unsigned short *vector2, unsigned char length);
unsigned short scalerVIP_ratio_inner_product2(unsigned short *vector1, unsigned short *vector2, unsigned char length);

//Profile RGB for ambilight sensor
void scalerVIP_Set_Profile_RGB_Data_Flag_V(unsigned long arg);
void scalerVIP_Get_Profile_RGB_Data_Flag_V(unsigned long arg);
void scalerVIP_Get_Profile_R_Data_V(unsigned long arg);
void scalerVIP_Get_Profile_G_Data_V(unsigned long arg);
void scalerVIP_Get_Profile_B_Data_V(unsigned long arg);
void scalerVIP_Set_Profile_RGB_Data_Flag_H(unsigned long arg);
void scalerVIP_Get_Profile_RGB_Data_Flag_H(unsigned long arg);
void scalerVIP_Get_Profile_R_Data_H(unsigned long arg);
void scalerVIP_Get_Profile_G_Data_H(unsigned long arg);
void scalerVIP_Get_Profile_B_Data_H(unsigned long arg);
void scalerVIP_yuv2rgb_convert(void);
void scalerVIP_Clip_Into_Range(unsigned char nBit,int * nData1 ,int * nData2,int * nData3);
//=====================
unsigned short scalerVIP_ratio_difference(unsigned short *vector1, unsigned short *vector2, unsigned char length);

unsigned char scalerVIP_Get_hpan_vfir(void);
void scalerVIP_Set_hpan_vfir(unsigned char value);
void scalerVIP_Dynamic_Vertical_NR(void);
void scalerVIP_Set_vfir_ctrl(unsigned char enable);
void scalerVIP_BTR_Detecte(void);
void scalerVIP_SMD_weake_move_Detecte(void);

#ifdef CONFIG_HW_SUPPORT_I_DE_XC
void scalerVIP_Dynamic_DeXCXL_CTRL(_RPC_system_setting_info *VIP_RPC_system_info_structure_table, SLR_VIP_TABLE* Share_VIP_TABLE_Struct, _RPC_clues* RPC_SmartPic_clue);
#endif //CONFIG_HW_SUPPORT_I_DE_XC
#ifdef CONFIG_HW_SUPPORT_MEMC
void scalerVIP_IP_OnOff_By_MEMC_Film(_RPC_system_setting_info* system_RPC_info_structure_table); //rord.tsao 2016/0325
#endif //CONFIG_HW_SUPPORT_MEMC

// identify pattern for LGE_Monitor demo, elieli
void scalerVIP_Identify_Pattern_forLGM(void);

// 20130823 jimmy.lin
/*unsigned int Noise_Statistics( unsigned int width, unsigned int height );*/
unsigned char scalerVIP_Fade_Detection ( void );

SLR_VIP_TABLE* scaler_GetShare_Memory_VIP_TABLE_Struct(void);
_clues* scaler_GetShare_Memory_SmartPic_Clue_Struct(void);
_RPC_clues* scaler_GetShare_Memory_RPC_SmartPic_Clue_Struct(void);
_system_setting_info* scaler_GetShare_Memory_system_setting_info_Struct(void);
_RPC_system_setting_info* scaler_GetShare_Memory_RPC_system_setting_info_Struct(void);

//Elsie 20131212, original by jimmy.lin
unsigned char scalerVIP_Fade_Detection (void);
MOTION_NOISE_TYPE scalerVIP_noise_estimation_MAD_et( unsigned int width, unsigned int height );
void scalerVIP_RTNR_correctionbit_period(void);
MOTION_NOISE_TYPE scalerVIP_Profile_Block_Motion(_clues* SmartPic_clue);
unsigned char scalerVIP_RTNR_level_compute(MOTION_NOISE_TYPE MADMotion, MOTION_NOISE_TYPE blockMotion, unsigned short *PQA_Input_Level, unsigned short *PQA_Input_rate);

char scalerVIP_ReadSrcDet(unsigned char step_thl, unsigned char step_max, unsigned char step_min);
unsigned char scalerVIP_Input_Source_Detection(unsigned int HD_SD_Thl);
unsigned char scalerVIP_DCC_Black_White_Detect(void);
void scalerVIP_compute_PQA_level(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table);
void scalerVIP_PQ_Adaptive_Table(unsigned char mode, SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table);
void scalerVIP_Reset_PQA_TBL_ContentCheck_Flag(unsigned char* ContentCheck_Flag);
void scalerVIP_PQA_TBL_ContentCheck(unsigned int PQA_Reg, unsigned char* ContentCheck_Flag);

void scalerVIP_Histogram_Section( unsigned char select1, unsigned char select2 );
void scalerVIP_Identify_Pattern_forLGM2(void);
void scalerVIP_Set_Parameter_forLGM2(unsigned char apply);
void scalerVIP_Identify_Pattern_forLG_K2L(void);
void scalerVIP_Set_Parameter_forLG_K2L(void);

#if CB_flag
void scalerVIP_Identify_Pattern_forCB(void);
void scalerVIP_Set_Parameter_forCB(void);
#endif

void scalerVIP_Set_Stop_Polling(void);
void scalerVIP_Set_Start_Print_Info(void);
int scalerVIP_2_Complement(unsigned int nVal,unsigned char nBitValid);

void scalerVIP_quality_init(unsigned long arg);

//  TV015 project
#ifdef ISR_TV015_DI_FUNCTION_ENABLE
void scalerVIP_TV015_Adjust(void);
void scalerVIP_DI_Chroma_Error_de_DotCrawing(void);
#endif
#ifdef ISR_AMBILIGHT_ENABLE
void scalerVIP_TV015_Lead(void);
void scalerVIP_AmbiLight(unsigned char display);
unsigned char drvif_color_dragon_profile_dectector_ambilight(unsigned char display, unsigned short *H_ambBlock, unsigned short *V_ambBlock, unsigned short *Profile_transition_sum, unsigned char hx_offset, unsigned char vy_offset, unsigned char Color_sel);
#endif
//=============

//***************************************************************//
void scalerVIP_TV003_MAD_Y(unsigned short *PQA_Input_Level, unsigned short *PQA_Input_rate);
void scalerVIP_hist_init(unsigned char display, unsigned short width, unsigned short height);
void scalerVIP_NetFlix_Smooth_TimingChange_for_RTNR(void);
void scalerVIP_NetFlix_smoothtoggle_enable(_RPC_clues* SmartPic_clue,_RPC_system_setting_info* RPC_system_info_structure_table);
unsigned char scalerVIP_CheckRegisterAccessProtect(unsigned int regAddr);

unsigned short scalerVIP_Brightness_Plus_Cal_OverSat_Protect_Gain(unsigned short BP_Gain, unsigned char Max_Bright_bin, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _clues* SmartPic_clue);
unsigned short scalerVIP_Brightness_Plus_Smooth_Gain_Cal(unsigned short BP_Gain, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem);
short scalerVIP_Brightness_Plus_Smooth_Offset_Cal(short RGB_Offset, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem);
short scalerVIP_Brightness_Plus_Compensation_RGB_Offset_Cal(unsigned short BP_Gain, unsigned short APL, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* sysInfo_TBL);
void scalerVIP_Brightness_Plus_Function(SLR_VIP_TABLE* vipTBL_shareMem, _clues* SmartPic_clue, _system_setting_info* sysInfo_TBL, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem);
void drvif_color_set_BrightnessPlus_YUV2RGB_CSMatrix(DRV_VIP_YUV2RGB_CSMatrix *ptr);

unsigned short scalerVIP_DBC_get_Mean_and_Peak_Value_in_SmartFit_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table);

unsigned char scalerVIP_DBC_BASIC_BLIGHT_Cal_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table);
unsigned char scalerVIP_DBC_DBC_Cal_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table);
unsigned char scalerVIP_DBC_BackLight_Cal_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table, _clues *SmartPic_clue);
unsigned char scalerVIP_DBC_Y_Max_Cal_TV002(_clues *SmartPic_clue);
void scalerVIP_DBC_CTRL_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table, _clues *SmartPic_clue);

void Scaler_color_print_scalerinfo_isr(void);

unsigned char scalerVIP_check_vdc_hw_pwr(void);

void scalerVIP_Set_Noise_Level(unsigned char Level);
unsigned char scalerVIP_Get_Noise_Level(void);
unsigned char scalerVIP_Get_PT_NO(void);
void scalerVIP_color_DRV_SNR_Veritcal_NR(DRV_SNR_Veritcal_NR *ptr);
void scalerVIP_Teeth_detcted(void);

void scalerVIP_DI_GMV_AVG_for_film(void);

unsigned char ScalerVIP_Get_pq_reset(void);
void scalerVIP_Set_pq_reset(unsigned char bEnable);

char scalerVIP_PF_BlackDetection(void);
unsigned char scalerVIP_access_tv002_style_demo_flag(unsigned char access_mode, unsigned char *pFlag);
void* scalerVIP_get_tv002_style_demo_struct(void);

void scalerVIP_VO_resolution_change_hack(void);

//void scalerVIP_Set_DeXC_OnOff_by_PlaySpeed(_RPC_system_setting_info* system_RPC_info_structure_table);

#if defined(CONFIG_RTK_AI_DRV)
int ScalerVIP_SE_Proc(void);
#endif

#endif
