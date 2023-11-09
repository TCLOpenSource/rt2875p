/***************************************************************************
                          scalerVIP.h  -  description
                             -------------------
    begin                : Mon Dec 22 2008
    copyright           : (C) 2008 by hsliao
    email                : hsliao@realtek.com
 ***************************************************************************/

#ifndef __ISR_SCALERVIP_H__
#define __ISR_SCALERVIP_H__

//#include "rtd_types.h"
//#include "kernel/common/gsys_share.h"

// driver header
//#include <kernel/scaler/scalerInfo.h>
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

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
	#include <scaler/vdcCommon.h>
#else
	#include <scalercommon/scalerCommon.h>
	#include <scalercommon/vdcCommon.h>
#endif

//#include "mach/io.h"
#include "tvscalercontrol/io/ioregdrv.h"

#include "tvscalercontrol/scaler/vipinclude.h"
#include "tvscalercontrol/vip/icm.h"
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

//#define ROSPrintf(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VGIP_VIP_DEBUG,0,fmt,##args)
//#define ROSPrintf(fmt, args...)
#define scalerDrvPrint ROSPrintf

//#define ISR_RTNR_SWITCH_CORRECTION_BIT_FUNCTION_ENABLE 1
//#define ISR_AMBILIGHT_ENABLE 1
#define ISR_LG_DEMO_FLAG 1
#define ISR_NEW_RTNR_ENABLE 1
//#define ISR_OLD_RTNR_ENABLE 1
//#define ISR_SHARPNESS_BY_MEAN_ENABLE 1
//#define ISR_NR_IMPULSE_FUNCTIOM_ENABLE 1

#define VIP_TG45_FLAG_NUM 6

#define debug_print_delay_time ((Start_Print.Delay_Time == 0)?(1):(Start_Print.Delay_Time))

extern unsigned char SceneChange;
extern unsigned char ucDCREnable;  // Erin 100311 from pacific
extern unsigned char TSB_style_ON;
extern unsigned char TG45_Flag;

typedef struct
{
	int H; // or H
	int S; // or S'
	int I; // or Y
}COLORELEM_ISR;

typedef enum _THREAD_TYPE
{
	VGIP_DS_ISR_THREAD,
	VGIP_DE_ISR_THREAD,
	DDOMAIN_DS_ISR_THREAD,
	DDOMAIN_DE_ISR_THREAD,
} THREAD_TYPE;

#define CLIP(min, max, val) ((val < min) ? min : (val > max ? max : val))

//extern unsigned char AutoMA_Enable=0;
unsigned char Scaler_get_AutoMA_Enable(void);

//USER:LewisLee DATE:2010/07/26
//for check histogram value to modify backlight
// oliver-, move to scalerCommon.h
/*
typedef enum
{
	_OPC_Daylight = 0,
	_OPC_Normal,
	_OPC_Night,
	_OPC_User,
	_OPC_Dynamic,
	_OPC_State_Max,
}OPC_PROC_STATE;
*/

/*typedef enum _SCALERVIP_MPEGNR {	//	Erin '100308
	SCALERVIP_MPEGNR_OFF = 0,
	SCALERVIP_MPEGNR_LOW,
	SCALERVIP_MPEGNR_MIDDLE,
	SCALERVIP_MPEGNR_HIGH,
} SCALERVIP_MPEGNR;*/

typedef struct
{
 unsigned int ENL_Y;
 unsigned int ENL_U;
 unsigned int ENL_V;
}_rtnr_enl;

typedef struct _RTNR_PARAS
{
	unsigned char y_th3;
	unsigned char y_th2;
	unsigned char y_th1;
	unsigned char c_th3;
	unsigned char c_th2;
	unsigned char c_th1;
	unsigned char y_k3;
	unsigned char y_k2;
	unsigned char y_k1;
	unsigned char y_k0;
	unsigned char c_k3;
	unsigned char c_k2;
	unsigned char c_k1;
	unsigned char c_k0;
}_rtnr_paras;


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
	// fwused_2
	/*[0:7]*/ unsigned char Delay_Time;
	/*[8 ] */ unsigned char Y_Hist;
	/*[9 ] */ unsigned char Hue_Hist;
	/*[10] */ unsigned char Sat_Mean;
	/*[11] */ unsigned char Dcc_1;
	/*[12] */ unsigned char Dcc_2;
	/*[13] */ unsigned char DCC_3;
	/*[14] */ unsigned char Rtnr_1;
	/*[15] */ unsigned char Rtnr_2;
	/*[16] */ unsigned char Mpeg_Nr;
	/*[17] */ unsigned char Snr;
	/*[18] */ unsigned char Sharpness_1;
	/*[19] */ unsigned char Sharpness_2;
	/*[20] */ unsigned char DI_1;
	/*[21] */ unsigned char DI_2;
	/*[22] */ unsigned char Film_Mode;
	/*[23] */ unsigned char HIST;
	/*[24] */ unsigned char Reverse_2;
	/*[25] */ unsigned char Reverse_3;
	/*[26] */ unsigned char FormattedLog;
	// fwused_3
	/*[0 ]*/ unsigned char ID_Pattern_ALL;
	/*[1 ]*/ unsigned char ID_Pattern_2;
	/*[2 ]*/ unsigned char ID_Pattern_4;
	/*[3 ]*/ unsigned char ID_Pattern_5;
	/*[4 ]*/ unsigned char ID_Pattern_7;
	/*[5 ]*/ unsigned char ID_Pattern_12;
	/*[6 ]*/ unsigned char ID_Pattern_23;
	/*[7 ]*/ unsigned char ID_Pattern_24;
	/*[8 ]*/ unsigned char ID_Pattern_35;
	/*[9 ]*/ unsigned char ID_Pattern_47;
	/*[10]*/ unsigned char ID_Pattern_107;
	/*[11]*/ unsigned char ID_Pattern_123;
	/*[12]*/ unsigned char ID_Pattern_132;
	/*[13]*/ unsigned char ID_Pattern_133;
	/*[14]*/ unsigned char ID_Pattern_154;
	/*[15]*/ unsigned char ID_Pattern_191;
	/*[16]*/ unsigned char ID_Pattern_192;
	/*[17]*/ unsigned char ID_Pattern_193;
	/*[18]*/ unsigned char ID_Pattern_194;
	/*[19]*/ unsigned char ID_Pattern_195;
	/*[20]*/ unsigned char ID_Pattern_139;
	/*[21]*/ unsigned char ID_Pattern_196;
	/*[22]*/ unsigned char ID_Pattern_14;
	/*[23]*/ unsigned char someothers;
	/*[24]*/ unsigned char ID_Pattern_32;
	/*[25]*/ unsigned char ID_Pattern_147;
	/*[26]*/ unsigned char Reverse_10;
	/*[27]*/ unsigned char Reverse_11;
	/*[28]*/ unsigned char VD_LOG_ENABLE;
	/*[29]*/ unsigned char ColorBar;
	/*[30]*/ unsigned char Slow_Motion1;
	/*[31]*/ unsigned char Slow_Motion2;
	/*[32]*/ unsigned char RF_flower;
}Start_Print_info_t;

typedef struct
{
 unsigned char Mad_lv;
 unsigned char Motion_lv;
 unsigned char Film_lv;
}LEVEL_INFO;

//===========planck add for TV AutoNR============//
typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  amount:8;
        RBus_UInt32  low_thd:8;
        RBus_UInt32  var_thd:8;
        RBus_UInt32  usm_en:1;
        RBus_UInt32  ed_detect_en:1;
        RBus_UInt32  gain_flex_en:1;
        RBus_UInt32  res1:1;
        RBus_UInt32  db_mode:4;
    };
}dm_usm_Un_Sharp_Mask_FILTER_RBUS_LINUX;

typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  res1:16;
        RBus_UInt32  min_shift:8;
        RBus_UInt32  max_shift:8;
    };
}dm_usm_upper_lower_bound_shift_RBUS_LINUX;

typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  diff_thd4:8;
        RBus_UInt32  diff_thd3:8;
        RBus_UInt32  diff_thd2:8;
        RBus_UInt32  diff_thd1:8;
    };
}dm_usm_Blending_region_thd_RBUS_LINUX;

typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  res1:2;
        RBus_UInt32  pk_mask_c0:5;
        RBus_UInt32  pk_mask_c1:4;
        RBus_UInt32  pk_mask_c2:4;
        RBus_UInt32  pk_en:1;
        RBus_UInt32  pk_coring:8;
        RBus_UInt32  pk_x1:8;
    };
}dm_uzu_Peaking_Coring_RBUS_LINUX;

typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  res1:2;
        RBus_UInt32  pk_mask_c3:4;
        RBus_UInt32  pk_shiftbit:2;
        RBus_UInt32  pk_offset:8;
        RBus_UInt32  pk_lmtp:4;
        RBus_UInt32  pk_lmtn:4;
        RBus_UInt32  pk_g1:4;
        RBus_UInt32  pk_g2:4;
    };
}dm_uzu_Peaking_Gain_RBUS_LINUX;
#if 0
typedef enum _DNR_ITEM_TYPE{         //20100921 LeoChen added for DNR LEVEL
       DNR_ITEM_SPATIAL_Y = 0,
       DNR_ITEM_SPATIAL_C,
       DNR_ITEM_TEMPORAL,
       DNR_ITEM_RTNR_Y,
       DNR_ITEM_RTNR_C,
       DNR_ITEM_MPEG_H,
       DNR_ITEM_MPEG_V,
       DNR_ITEM_VD_NR,
       DNR_ITEM_EDGE,
       DNR_ITEM_Dark_K,
       DNR_ITEM_MCNR,
       DNR_ITEM_peaking,//DNR_ITEM_Reserved1, // DNR_ITEM_peaking is sync from L3300
       DNR_ITEM_Reserved2,
       DNR_ITEM_MAX,
}DNR_ITEM_TYPE;

typedef enum _DNR_SELECT{
	DNR_SELECT_OFF = 0,
	DNR_SELECT_LOW,
	DNR_SELECT_MIDDLE,
	DNR_SELECT_HIGH,
	DNR_SELECT_NR_TABLE,
       DNR_SELECT_MAX,
}DNR_SELECT;
#endif
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
typedef enum _VIP_GAMMA_CHANNEL_RGB_ISR {
	GAMMA_CHANNEL_R_ISR = 0,
	GAMMA_CHANNEL_G_ISR,
	GAMMA_CHANNEL_B_ISR,
	VIP_GAMMA_CHANNEL_RGB_MAX_ISR,
	GAMMA_CHANNEL_W_ISR,
} VIP_GAMMA_CHANNEL_RGB_ISR;
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

//==========================================

#define _RGB2YUV_m11					0x00
#define _RGB2YUV_m12					0x01
#define _RGB2YUV_m13					0x02
#define _RGB2YUV_m21					0x03
#define _RGB2YUV_m22					0x04
#define _RGB2YUV_m23					0x05
#define _RGB2YUV_m31					0x06
#define _RGB2YUV_m32					0x07
#define _RGB2YUV_m33					0x08
#define _RGB2YUV_Yo_even				0x09
#define _RGB2YUV_Yo_odd					0x0a
#define _RGB2YUV_Y_gain					0x0b
#define _RGB2YUV_sel_RGB				0x0c
#define _RGB2YUV_set_Yin_offset				0x0d
#define _RGB2YUV_set_UV_out_offset			0x0e
#define _RGB2YUV_sel_UV_off				0x0f
#define _RGB2YUV_Matrix_bypass				0x10
#define _RGB2YUV_Enable_Y_gain				0x11


//==========================================

typedef struct {
	unsigned int VIP_QUALITY_Coef;
	unsigned int VIP_QUALITY_Extend_Coef;
	unsigned int VIP_QUALITY_Extend2_Coef;
	unsigned int VIP_QUALITY_Extend3_Coef;
	unsigned int VIP_QUALITY_3Dmode_Coef;
	unsigned int VIP_QUALITY_Extend4_Coef;

	unsigned int SD_H_table;
	unsigned int SD_V_table;

	unsigned int ODtable_Coeff;
	unsigned int DCR_TABLE;
	unsigned int ICM_by_timing_picmode;

	unsigned int CDS_ini;
	unsigned int D_EDGE_Smooth_Coef;
	unsigned int I_EDGE_Smooth_Coef;

	unsigned int YUV2RGB_CSMatrix_Table;
	unsigned int Tone_Mapping_LUT_R;
	unsigned int VD_ConBriHueSat;
	unsigned int tICM_ini;
	unsigned int tICM_H_7axis;
	unsigned int tGAMMA;
	unsigned int DCC_Control_Structure;
	unsigned int Ddomain_SHPTable;
	unsigned int Idomain_MBPKTable;
	unsigned int Manual_NR_Table;
	unsigned int PQA_Table;
	unsigned int PQA_Input_Table;

	unsigned int Auto_Function_Array1;
	unsigned int Auto_Function_Array2;
	unsigned int Auto_Function_Array3;
	unsigned int Auto_Function_Array4;
	unsigned int Auto_Function_Array5;
	unsigned int DrvSetting_Skip_Flag;
	unsigned int checkSum;
} VIP_table_crc_value_ISR;

/* ==========for MEMC infor jzl 20151231============================ */

typedef struct
{
	unsigned char 	bSRC_VO;	///< ScalerVIP status
	unsigned char 	bHDR_Chg;	///< ScalerVIP status
	unsigned short	mBrightness;  ///< ScalerVIP status
	unsigned short	mContrast;    ///< ScalerVIP status
    signed short	mSpeed;	 ///< ScalerVIP status
	BOOL	bZoomAction;		///< ScalerVIP status
	BOOL      bShiftAction;	///< ScalerVIP status
	unsigned short	source_type;
	unsigned char mVOSeek;
	unsigned short	MEMCMode;		///< MEMC mode
	unsigned char   mDarkLevel; ///< Hist dark level
	BOOL	Hist_SC_flg;
	unsigned char	mSoccer_holdfrm; ///< soccer patch
	BOOL		PictureMode_Chg;
	BOOL		bVIP_still;
	unsigned char	m_MiraCast;	///< m_MiraCast Status
	unsigned short	Forbes_flg;
	unsigned int DTL_Step_Hist_sum_ratio[5];
	unsigned int VIP_Project_ID;
	unsigned char	mTriBall_holdfrm; ///< patch_tri_ball_Issue_detect patch
}SET_MEMC_SCALER_VIP_INFO;

void scalerVIP_Set_Info_toMEMC(void);
void scalerVIP_Set_MEMC_Info_toVCPU(void);

SET_MEMC_SCALER_VIP_INFO scalerVIP_Get_Info_fromScaler(void);
UINT16 scalerVIP_Get_PatternFlag_Identification(UINT16 nNumber);
UINT8 scalerVIP_Get_MEMCPatternFlag_Identification(UINT16 prj,UINT16 nNumber);
//void scalerVIP_PQ_Bypass_Monitor(void);
void scalerVIP_color_MA_ISNR_MotionUpdate(unsigned char status);

/* ==========for MEMC infor jzl 20151231============================ */
/* ====================================== */
void scalerVIP_colorMaAutoISR(SCALER_CHANNEL_NUM channel);
#if 0
void drvif_color_TSB_Edge_Peaking_AutoMode(unsigned char noise_level_idx,unsigned char PictureMode);// Hawaii add for Edge Peaking 20131022
unsigned int drvif_color_2parts_linear_interpolation(unsigned int min, unsigned int max, unsigned int middle ,unsigned int OSD_sharp_level);// Hawaii add for Edge Peaking 20131022
int check_range(int data, int min, int max);// Hawaii add for Edge Peaking 20131022
#endif

unsigned int scalerVIP_getColorHistValue(unsigned char idx);

void scalerVIP_setRTNR_Y_Table(unsigned long arg);
unsigned char scalerVIP_updateRTNR_Y_Table(unsigned long arg);
unsigned char scalerVIP_getRTNR_Y(unsigned char src_idx); // Because of synchronization problem, rtnr value in kernel space is read only.
void scalerVIP_setRTNR_C_Table(unsigned long arg);
unsigned char scalerVIP_updateRTNR_C_Table(unsigned long arg);
unsigned char scalerVIP_getRTNR_C(unsigned char src_idx); // Because of synchronization problem, rtnr value in kernel space is read only.
void scalerVIP_SetMpegNR(unsigned long arg);
void scalerVIP_GetMpegNR(unsigned long arg);
void scalerVIP_SetMpegNR_H(unsigned long arg);
void scalerVIP_SetMpegNR_V(unsigned long arg);
void scalerVIP_SetCorrectionBIT1_Duty_Debug(unsigned long arg);

void scaler_Set_Dcr_Panasonic_Enable(unsigned long arg);
void scaler_Set_Dcr_Powersave_Enable(unsigned long arg);

unsigned char scalerVIP_GetBrightness(void);
void scalerVIP_SetBrightness(unsigned char brightness);
unsigned char scalerVIP_GetContrast(void);
void scalerVIP_SetContrast(unsigned char contrast);
unsigned short scalerVIP_GetGreenHue(void);
void scalerVIP_GetAutoNR(unsigned long arg);
void scalerVIP_SetAutoNR(unsigned long arg);
void scalerVIP_GetAutoNR_by_Low_Mid_High(unsigned long arg);
void scalerVIP_SetAutoNR_by_Low_Mid_High(unsigned long arg);
void scalerVIP_setDNRTable(unsigned long arg);
unsigned char scalerVIP_updateDNRTable(unsigned long arg);
unsigned char scalerVIP_getDNR(unsigned char src_idx);


void scalerVIP_color_mpeg_data_reset(unsigned long arg);

void scalerVIP_setTableNRSpatial(unsigned long arg);
void scalerVIP_getTableNRSpatial(unsigned long arg);

void scalerVIP_setPictureMode(unsigned long arg);
void scalerVIP_SetMainTotalPixelInv(unsigned long arg);//Leo Chen 20100810

//=================== DCL ========================
//=================== Skin Tone Detection ========================
void skin_tone_detect(unsigned char channel, unsigned int width, unsigned int height);
void driver_rgb2yuv_constartbrightness(short contrast , short brightness);
void skin_tone_Level_detect(unsigned char channel, unsigned int width, unsigned int height);
void video_drvif_color_set_dcc_skin_tone_offset_apply(unsigned char Region);



//=================== New, copy from Pacific ========================
unsigned char drvif_color_chessboard_detection(unsigned char PF_Blk_Num, unsigned char th_profile_bin_cnt_diff, unsigned char th_hist_cnt_max2bin_percent, unsigned char th_hist_bincnt_diff_percent, unsigned char th_hist_binIdx_diff);
void p_film_detection(void);


unsigned char drvif_color_compute_noise_level(unsigned char channel);

void drvif_color_MpegNR_AutoMode_Off(void);
unsigned char drvif_color_DNR_AUTOMode_Mac(unsigned char display, unsigned char TVAutoNR_Mode, unsigned char VD_noise_level, unsigned char DNR_Auto_lvl_SdHd);
unsigned char drvif_color_DNR_AUTOMode_New(unsigned char display);

void drvif_color_colorsetcdcti(unsigned char display, unsigned char Prevent_PE,  unsigned char F_Gain, unsigned char F_th,unsigned char S_DCTI_Enable, unsigned char S_Gain, unsigned char S_Th);

//void drvif_color_setcdcti(unsigned char display, unsigned char Level);
void drvif_color_dnewdcti_table_isr(DRV_VipNewDDcti_Table* ptr, unsigned char offset_gain);
//================================
void drvif_color_DNRAUTO(unsigned char display,unsigned char AUTO_NR_style, unsigned char VD_noise_level, unsigned char DNR_Auto_lvl_SdHd);
unsigned char drvif_color_DNR_AUTOMode(unsigned char display, unsigned char AUTO_NR_style, unsigned char DNR_Auto_lvl_SdHd);
void drvif_color_TVAutoNR(unsigned char display, unsigned char AUTO_NR_style, unsigned char VD_noise_level, unsigned char DNR_Auto_lvl_SdHd);
void drvif_color_RTNR_correctionbit_Control_for_residual_image(SCALER_CHANNEL_NUM channel);

void drvif_color_newRTNRsetting_for_SonyRadio(SCALER_CHANNEL_NUM channel);
unsigned char drvif_color_check_source_remmping(unsigned char display);






//===========================================
// Erin 010310
int abs_dcc( int input );
unsigned int Lerp(unsigned int X_current, unsigned int X_1, unsigned int X_0, unsigned int Y_1, unsigned int Y_0, unsigned int x_offset_1, unsigned int x_offset_0);

unsigned int di_pow(unsigned int x, unsigned int p);
void drif_color_Edge_Noise_level(unsigned char ch, _rtnr_enl *ENL);
void scalerVIP_SetTableSNR_Y(unsigned char arg);
void scalerVIP_SetTableSNR_C(unsigned char arg);

void drvif_color_HDMI_AutoColorBarNR_ON(unsigned char channel);
void drvif_color_HDMI_AutoColorBarNR_OFF(unsigned char channel);

void drvif_color_AutoColorBarNR_ON(unsigned char channel);
void drvif_color_AutoColorBarNR_OFF(unsigned char channel);

int drvif_color_colorbar_dectector(_clues* SmartPic_clue);
int drvif_color_colorbar_dectector_by_SatHueProfile(_clues* SmartPic_clue);
//==========================================
//===========planck add for TV AutoNR===============
//==========================================
void drvif_color_TV_AutoNR_Mac(unsigned char display, unsigned char TVAutoNR_Mode, unsigned char VD_noise_level, unsigned char DNR_Auto_lvl_SdHd);

void drvif_color_TV_AutoNR(void);
void drvif_color_noisereduction_spatial(unsigned char channel, unsigned char level, unsigned char bNoiseReductionMode);
void drvif_color_noisereduction_impulse(unsigned char channel, unsigned char level, unsigned char bNoiseReductionMode);
void drvif_color_noisereduction_spatial_for_TV(unsigned char channel, unsigned char level, unsigned char bNoiseReductionMode);
void drvif_color_noisereduction_spatial_auto_mode(unsigned char channel, unsigned char level, unsigned char bNoiseReductionMode);
void scalerVIP_SetTableNRTemporal(unsigned char arg) ;
void scalerVIP_SetTableRTNR_Y(unsigned char arg);
void scalerVIP_SetTableRTNR_C(unsigned char arg);
void scalerVIP_SetTableUnShpMask(unsigned char arg);
void scalerVIP_SetTableSCALEPK(unsigned char arg);
void scalerVIP_SetTableSHP1D2D_EGSM(unsigned char arg);
//==========================================
//==========================================
void drvif_color_switch_rtnr_rounding_correction_bit(unsigned char switch_on); //add for  TPV 0,3,5 IRE Pattern
//==========================================

void drvif_color_ClueGather( unsigned char ch, _clues* clueA);
void drvif_color_CluePrinter(_clues* clueA);
unsigned char drvif_color_get_flag_chessboard(void);
void drvif_color_ma_dcc_chroma(unsigned char level);
unsigned char drvif_color_dcc_debug_mode(unsigned long arg);
void drvif_color_ma_dcc_on(unsigned char enable);
void drvif_color_ma_dcc_setlevel(unsigned char display, unsigned char level);
//=======DCR FUNCTION=====
void scalerVIP_DCR_TPV_TV(void);
void scalerVIP_DCR_TPV(void);
void scalerVIP_DCR_OLD(void);
void scalerVIP_DCR_TPV_TV_2694A(void);
void scalerVIP_DCR_TPV_2694A(void);
void scalerVIP_DCR_TPV_Multi_Mode(void);

void scalerVIP_TVSource_Separation(void);
short drvif_color_BL_PWM__Interpolation(short nDiff,short nOffset,short nBLDiff,short nHisDiff);
void drvif_color_ma_DCR_TPV(unsigned char *TPV_DCR_TABLE, unsigned int DCC_HISTOGRAM_MEAN_VALUE, unsigned char BL_OSD);
void drvif_color_Plan_DCR_Curve(void);
void RecordAPL(unsigned char nAPL);
char GetMaxValue(unsigned char * Array ,char nLen);
char GetMinValue(unsigned char * Array ,char nLen);

//==========
void scalerVIP_Profile_Motion_Detect(unsigned char display, _clues* SmartPic_clue);
unsigned char scalerVIP_Get_Profile_Motion(void);
void drvif_color_ma_DCR_TV057(unsigned char Set_PWMValue);
unsigned char scalerVIP_get_Mean_and_Peak_Value_in_SmartFit(void);
unsigned char  scalerVIP_get_Mean_and_Peak_Value_in_SmartFit_003(unsigned char H_rate, unsigned char V_rate);
void drvif_color_ma_DCR_new(unsigned short Frame_need_SC, unsigned short Frame_need_NML, unsigned short stable_buf_thl, unsigned short DZ_break_thl);

void drvif_color_ma_hfd_problem(void);
void drvif_color_ma_TeethCheck(unsigned char Level);
int drvif_Get_Purecolor_Weighting(void);
unsigned char error_dist_detect(unsigned char channel, unsigned int width, unsigned int height);
unsigned char icm_hue_cnt_match(unsigned char * huehist );
int Long_Division( unsigned long numerator, unsigned long denominator, int depth );
unsigned char scalerVIP_colorActiveRegion( unsigned char display, unsigned short *Region );
void scalerVIP_colorHistDetect(unsigned int en_test);

void scalerVIP_colorMAChromaBW(void);

//void scalerVIP_colorProfileStillMotion(void);
void scalerVIP_colorSolveXcForVd(unsigned int en_test);
void scalerVIP_color_noise_mpeg(void);

int SC_detector(int *T, int N);
unsigned char pure_color_detect(unsigned char channel, unsigned int width, unsigned int height);
unsigned char pure_color_detect_Sony(unsigned char channel, unsigned int width, unsigned int height);
void drvif_color_rtnr_y_th_isr(unsigned char channel, unsigned char *Th_Y);

void scalerVIP_Share_Memory_Access_VIP_TABLE_CRC_Struct(unsigned long arg);
void fwif_color_PrintArray_UINT32(unsigned int * pwTemp ,int nSize,int nID);
void fwif_color_PrintArray_INT32(int * pwTemp ,int nSize,int nID);
void fwif_color_PrintArray_UINT16(unsigned short * pwTemp ,int nSize,int nID);
void fwif_color_PrintArray_INT16(short * pwTemp ,int nSize,int nID);
void fwif_color_PrintArray_UINT8(unsigned char * pwTemp ,int nSize,int nID);
void scalerVIP_Share_Memory_Access_DCC_Init(void);
void scalerVIP_Share_Memory_Access_NR_Init(void);
void scalerVIP_Share_Memory_Access_Sharpness_Init(void);
void* scalerVIP_Get_Share_Memory_VIP_TABLE_Custom_Struct(void);

void scalerVIP_SetNrAvgcount(unsigned long arg);//yuan1024
void scalerVIP_GetNrAvgcount(unsigned long arg);

void scalerVIP_setAutoMADebugMsg(unsigned long arg);  // Erin 100624
void scalerVIP_getAutoMADebugMsg(unsigned long arg);

void drvif_color_dark_area_AutoNR(void);    /* Leo Chen, 20100809, added for dark_area_AutoNR */
unsigned char black_white_pattern_detect(void);

void scalerVIP_SET_DNR_LEVEL_TABLE(unsigned long arg);

void scalerVIP_COEFF_BY_Y_REG(unsigned long arg);
void scalerVIP_NR_TABLE32(unsigned long arg);
void scalerVIP_NR_TABLE8(unsigned long arg);
//void scalerVIP_PQA_TABLE_OFFSET_TEMP(unsigned long arg);	// move to share memory access struct, elieli

// Hawaii add for Edge Peaking 20131022
void scalerVIP_Edge_Peaking_Table(unsigned long arg);

void scalerVIP_Mem_Edge_AutoPk_data(unsigned long arg);//RenChen sync from L3300
void scalerVIP_PWM_TABLE(unsigned long arg);

//for T2 2644
void scalerVIP_SET_INTELLIGENT_PICTURE_2D_PEAKING_TABLE(unsigned long arg);
void scalerVIP_GET_VD_NOISE_LEVEL(unsigned long arg);

void scalerVIP_SET_Custom_Style(unsigned char id);
/* DCR */
void scalerVIP_Set_DCR_BackLightMappingRange(void);
void scalerVIP_GetPinBlAdj(unsigned long arg);//Leo Chen 20101001
void scalerVIP_GetPinBlOnOff(unsigned long arg);//Leo Chen 20101001
void scalerVIP_GetBacklightOn(unsigned long arg);//Leo Chen 20101001
void scalerVIP_SetIsIncreseMode(unsigned char arg);//Leo Chen 20101001
void scalerVIP_GetDcrMaxValue(unsigned long arg);//Leo Chen 20101001
void scalerVIP_IoGpio_SetPwmDuty(unsigned int gpioInfo, unsigned int duty) ;//Leo Chen 20101001
unsigned char scalerVIP_IoGpio_GetPwmDuty(unsigned int pwmInfo);

void scalerVIP_IoGpio_SetPinLevel(unsigned int selectedGpio, unsigned char level);//Leo Chen 20101001
void scalerVIP_SetDcrUIMaxValue(unsigned int arg);//Leo Chen 20101111
void scalerVIP_SetDcrUIMinValue(unsigned int arg);//Leo Chen 20101111
void scalerVIP_SetDcrOnOff(unsigned char arg);//Leo Chen 20101111
void scalerVIP_GetBlLvFromUser(unsigned char arg);//Leo Chen 20101001
void scalerVIP_SetBlLvActMax(unsigned int arg);//Leo Chen 20101111
void scalerVIP_SetBlLvActMin(unsigned int arg);//Leo Chen 20101111
void scalerVIP_SetDefaultDCRMode(unsigned long arg);
unsigned char drvif_module_vdc_NoiseStatus_isr(void);
unsigned char drvif_color_VDStatus_AUTONR(unsigned char display );

//Sony radio detector
unsigned char Sony_radio_detect(unsigned char channel, unsigned int width, unsigned int height);

unsigned char DI_detect_Champagne(unsigned int width, unsigned int height);//sony demo

void DI_Champagne_problem(void);//sony demo

unsigned char motion_purecolor_concentric_circles_detect(unsigned int width, unsigned int height);//sony demo

unsigned char motion_move_concentric_circles_detect(unsigned int width, unsigned int height);//sony demo

unsigned char motion_concentric_circles_detect(unsigned int width, unsigned int height);

void scalerVIP_SetTableDCTI(unsigned char arg); //Leo Chen 20101217

void scalerVIP_SetICM_Global_Sat(unsigned short arg);
void scalerVIP_SetScaler_FIR_H(unsigned char arg);
void scalerVIP_SetScaler_FIR_V(unsigned char arg);
void scalerVIP_SetScaler_FIR_H_8tap(unsigned char arg);
void scalerVIP_SetScaler_FIR_V_6tap(unsigned char arg);

void scalerVIP_SetPeaking_POS_RANGE_MAX(unsigned int arg);
void scalerVIP_SetPeaking_NEG_RANGE_MAX(unsigned int arg);
void scalerVIP_SetPeaking_Coring(unsigned char arg);
void scalerVIP_SetPeaking_Level(unsigned char arg);

unsigned char drvif_color_profile_dectector(unsigned char display, unsigned short *H_Block, unsigned short *V_Block);
void scaler_Set_VIP_info_structure(unsigned char display,_clues* SmartPic_clue);
unsigned char drvif_color_activeregion_dragon(unsigned char display, unsigned short *Region);
void drvif_color_rtnr_y_k(unsigned char channel, unsigned int Y_K0_K7, unsigned int Y_K8_K15);
void drvif_color_rtnr_c_k(unsigned char channel, unsigned int K_C);
void drvif_color_rtnr_c_th_isr(unsigned char channel, unsigned char *Th_C);
void drvif_color_noisereduction_spatialy_for_TV(unsigned char channel, unsigned char level, unsigned char bNoiseReductionMode);
void drvif_color_noisereduction_spatialc_for_TV(unsigned char channel, unsigned char level, unsigned char bNoiseReductionMode);
unsigned char drvif_module_vdc_ReadMode(GET_VIDEO_MODE_METHOD result);


void fwif_color_rtnr_y_adjust(unsigned char channel, unsigned char osd_level);  //20080731 jason modify interface: which_source is not used.
void fwif_color_rtnr_c_adjust(unsigned char channel, unsigned char osd_level);  //20080731 jason modify interface: which_source is not used.

void drvif_color_newrtnr_rtnr_k(unsigned char row_select, unsigned char *th);
void drvif_color_newrtnr_sad_th(unsigned char edge, unsigned char *th);
void drvif_color_newrtnr_sta_th(unsigned char *th);
void drvif_color_newrtnr_edgetypediff_th(unsigned char *th);
//void drvif_color_newrtnr_snr_edge_th(unsigned int *th);
void fwif_color_new_rtnr_adjust(unsigned char channel, unsigned char RTNR_level);
void drvif_weak_signal_RTNR_ON_mode1(unsigned char channel, unsigned char signal_status);
void drvif_weak_signal_RTNR_ON_mode2(unsigned char channel, unsigned char signal_status);
void drvif_weak_signal_RTNR_ON_mode3(unsigned char channel, unsigned char signal_status);
void drvif_weak_signal_RTNR_ON_mode4(unsigned char channel, unsigned char signal_status);
void drvif_weak_signal_RTNR_ON_mode5(unsigned char channel, unsigned char signal_status);
void drvif_Strong_signal_RTNR_ON_forTV(unsigned char channel, unsigned char signal_status);
void drvif_color_TV_Auto_RTNR_MAD_Window(char level_index, unsigned int current_Signal_Status);
void drvif_color_TV_Auto_VD_NR(char level_index, unsigned int current_Signal_Status);
void drvif_color_TV_Auto_Special_NR(char level_index, unsigned int current_Signal_Status);
void fwif_color_rtnr_y_k_adjust(unsigned char channel, unsigned char osd_level);
void fwif_color_rtnr_c_k_adjust(unsigned char channel, unsigned char osd_level);
void fwif_color_rtnr_y_th_adjust(unsigned char channel, unsigned char osd_level);
void fwif_color_rtnr_c_th_adjust(unsigned char channel, unsigned char osd_level);
void scalerVIP_GetTVAutoNR_Mode(unsigned long arg);
void scalerVIP_SetTVAutoNR_Mode(unsigned long arg);
void scalerVIP_SetNoisereduction_Vfilter(unsigned long arg);
void scalerVIP_GetNoisereduction_Vfilter_ON(unsigned long arg);
void drvif_color_Mixer_ctrl_Vfilter(unsigned char channel, int vfilter_weight);

unsigned char Scaler_GetAutoMA(unsigned char flag);

void scalerVIP_SetTable_HMCNR(unsigned char arg);
void scalerVIP_SetTable_VerticalNR(unsigned char arg);

void scalerVIP_GetDNR_log(unsigned long arg);
void scalerVIP_SetDNR_log(unsigned long arg);
void scalerVIP_GetRTNR_log(unsigned long arg);
void scalerVIP_SetRTNR_log(unsigned long arg);
void scalerVIP_GetSNR_log(unsigned long arg);
void scalerVIP_SetSNR_log(unsigned long arg);
void scalerVIP_GetINR_log(unsigned long arg);
void scalerVIP_SetINR_log(unsigned long arg);

void scalerVIP_Get_DEBUG_AUTO_NR(unsigned long arg);
void scalerVIP_Set_DEBUG_AUTO_NR(unsigned long arg);
void scalerVIP_Get_DEBUG_VD_NOISE(unsigned long arg);
void scalerVIP_Set_DEBUG_VD_NOISE(unsigned long arg);
void scalerVIP_Get_DEBUG_RTNR_MAD(unsigned long arg);
void scalerVIP_Set_DEBUG_RTNR_MAD(unsigned long arg);
void scalerVIP_Get_DEBUG_HISTOGRAM_MEAN(unsigned long arg);
void scalerVIP_Set_DEBUG_HISTOGRAM_MEAN(unsigned long arg);

void drvif_color_TV_Auto_peaking_setting(char mode ,char level_index, unsigned int current_Signal_Status);
void scalerVIP_GetDCC_DelayTime(unsigned long arg);
void scalerVIP_SetDCC_DelayTime(unsigned long arg);
void scalerVIP_GetDCC_Step(unsigned long arg);
void scalerVIP_SetDCC_Step(unsigned long arg);
void scalerVIP_SET_AutoView_flag(unsigned long arg); // Hawaii add for new AutoView 20131001

void scalerVIP_Set_TV_05_style_ON(unsigned long arg);
void scalerVIP_Set_AUTO_NR_style(unsigned long arg);
void scalerVIP_Get_AUTO_NR_style(unsigned long arg);
void scalerVIP_Set_DccScurveLevel(unsigned long arg);
//void scalerVIP_Get_DNRTABLE(unsigned long arg);		// move "DNR table select info" to system_info_struct and replace by PQA table, elieli
//void scalerVIP_Set_DNRTABLE(unsigned long arg);			// move "DNR table select info" to system_info_struct and replace by PQA table, elieli
unsigned char scalerVIP_Update_TSB_Active_Backlight_Table(unsigned long arg);



//yuan1024 add for rtnr n fiels avg
extern unsigned char VD_noise_status_Num;
extern unsigned int VD_noise_count_Status_sum;
extern unsigned char VD_noise_flag;
extern unsigned int counter_noise;
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
void scalerVIP_color_noise_dejag_detect(void);// K4Lp
void scalerVIP_color_noise_mpeg_apply(void);
void scalerVIP_color_noise_mpeg_v_apply(void);// S4Ap K4Lp
void scalerVIP_color_noise_dejag_apply(void);// K4Lp
void scalerVIP_newRTNR_Adaptive_detect(SCALER_CHANNEL_NUM channel, unsigned char * plevel_flag, unsigned char * pPan_flag, unsigned char * pBigSmoothArea_flag);

void drvif_color_inewdcti_set(unsigned char display, unsigned char data_sel,  unsigned char maxlen, unsigned char psmth, unsigned char lpmode, unsigned char engdiv, unsigned char offdiv, unsigned char uvgain, unsigned char dcti_mode, unsigned char cur_sel, unsigned char dcti_en);
void drvif_color_inewdcti_table_isr(DRV_VipNewIDcti_Table* ptr, unsigned char offset_gain);
void scalerVIP_DCR(void);
void scalerVIP_setDcrTable(void);
void scalerVIP_DCR_TV057(void);
void scalerVIP_DCR_TV003(void);
void scalerVIP_DCR_TV015(void);


void drvif_color_DCTI_for_ColorBar(unsigned char color_bar_flag,unsigned char colorbar_score);
unsigned char drvif_color_DCTI_auto_compute(DRV_VipNewDcti_auto_adjust* ptr,unsigned char colorbar_score);
void drvif_color_DELAY_for_ColorBar(unsigned char color_bar_flag,unsigned char colorbar_score);


//Elsie 20130521
unsigned char scalerVIP_ColorbarDetector(_clues* SmartPic_clue);
void scalerVIP_newdcti(unsigned char isColorBar);
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

void scalerVIP_MA_IEdgeSmooth(unsigned char flag);
void scalerVIP_IEedgeSmooth_Linemode_check(void);

unsigned char scalerVIP_Get_hpan_vfir(void);
void scalerVIP_Set_hpan_vfir(unsigned char value);
void scalerVIP_Dynamic_Vertical_NR(void);
void scalerVIP_Set_vfir_ctrl(unsigned char enable);
void scalerVIP_BTR_Detecte(void);
// chen 180308 remove
//void scalerVIP_SMD_weake_move_Detecte(void);
// end chen 180308 remove
void scalerVIP_Dynamic_DeXCXL_CTRL(_RPC_system_setting_info *VIP_RPC_system_info_structure_table, SLR_VIP_TABLE* Share_VIP_TABLE_Struct, _RPC_clues* RPC_SmartPic_clue);
void scalerVIP_IP_OnOff_By_MEMC_Film(_RPC_system_setting_info* system_RPC_info_structure_table); //rord.tsao 2016/0325

// identify pattern for LGE_Monitor demo, elieli
void scalerVIP_Identify_Pattern_forLGM(void);

// 20130823 jimmy.lin
/*unsigned int Noise_Statistics( unsigned int width, unsigned int height );*/
unsigned char scalerVIP_Fade_Detection ( void );

//20130521 added by Hawaii for Dynamic Peaking Display
void scalerVIP_Get_DCC_Histogram_Mean_value(unsigned long arg);
void scalerVIP_Get_RTNR_MAD_count_Y_sum_avg(unsigned long arg);

#define CONFIG_60_patch 1

#if CONFIG_60_patch
void scalerVIP_SetCONFIG60_VD(unsigned char Enable);
unsigned char scalerVIP_GetCONFIG60_VD(void);
unsigned char scaler_GetDisableVD(void);
void scaler_SetDisableVD(unsigned char Enable);
#endif

//Elsie 20130820, 20130930 sync from Magellan
void scalerVIP_Set_RTNR_K_force(unsigned long arg);
void scalerVIP_RTNR_K_force(void);

//Elsie 20131025: move from AP
void scalerVIP_sRGB_Hue_Sat_Table(unsigned long arg);
void scalerVIP_sRGB_HueSat_Write(void);
void wait_for_sRGB_fifo_empty_isr(void);

void scalerVIP_Gamma_Table_Write(void);
#ifdef CONFIG_ARM64 //ARM32 compatible
void scalerVIP_Set_Gamma_Table(unsigned long arg);
#else
void scalerVIP_Set_Gamma_Table(unsigned int arg);
#endif

void scalerVIP_Set_Inv_Gamma_En(unsigned char arg);
void scalerVIP_Inv_Gamma_En_Write(void);


unsigned char GetAutoMA2Flag(void);
unsigned char GetAutoMA_Flag(unsigned char flag);
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

unsigned int scalerVIP_Get_Step_CNT(void);
char scalerVIP_ReadSrcDet(unsigned char step_thl, unsigned char isd_tsh_hist_mode);

unsigned char scalerVIP_Input_Source_Detection(unsigned int HD_SD_Thl);
//cychen2 20130124
void scalerVIP_RTNR_set_sharpness(int level, int SC);
void scalerVIP_patten_sharpness(int level, int SC);
unsigned char scalerVIP_DCC_Black_White_Detect(void);
void scalerVIP_compute_PQA_level(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, _RPC_clues* RPC_SmartPic_clue);
//void drvif_color_compute_MA_level(void);
//void scalerVIP_PQ_Adaptive_offset_temp(unsigned char src, unsigned char index);
void scalerVIP_PQ_Adaptive_Table(unsigned char mode, SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table);
void scalerVIP_PQ_Adaptive_Table_TV002(unsigned char mode, SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table);
void scalerVIP_Reset_PQA_TBL_ContentCheck_Flag(unsigned char* ContentCheck_Flag);
void scalerVIP_PQA_TBL_ContentCheck(unsigned int PQA_Reg, unsigned char* ContentCheck_Flag);

void scalerVIP_Histogram_Section( unsigned char select1, unsigned char select2 );
void scalerVIP_Identify_Pattern_forLGM2(void);
void scalerVIP_Identify_Pattern_forMEMC(void);
void scalerVIP_Identify_Pattern_forSKY(void);
void scalerVIP_Set_Parameter_forLGM2(unsigned char apply);
void scalerVIP_Set_Parameter_forSKY(unsigned char apply);
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
void scalerVIP_picmode_init(unsigned long arg);

//  TV015 project
#ifdef ISR_TV015_DI_FUNCTION_ENABLE
void scalerVIP_TV015_Lead(void);
void scalerVIP_TV015_Adjust(void);
void scalerVIP_DI_Chroma_Error_de_DotCrawing(void);
#endif
void scalerVIP_AmbiLight(unsigned char display);
unsigned char drvif_color_dragon_profile_dectector_ambilight(unsigned char display, unsigned short *H_ambBlock, unsigned short *V_ambBlock, unsigned short *Profile_transition_sum, unsigned char hx_offset, unsigned char vy_offset, unsigned char Color_sel);

//=============


void scalerVIP_INT32_Copy(int * pwTemp ,int nSize,int * pwCopyTemp);

unsigned int checksum_vip(const void *buf, unsigned int size);

//************************************************************************//
//****************Arrange no use function(From Sirius CSFC, 20140424) ***********//
//************************************************************************//

#if 0 //Replase by PQA adaptive control Flow
void drvif_color_VDNR_AutoMode_RfCvbs(unsigned char channel, unsigned char VD_noise_level);
void drvif_color_VDNR_AutoMode_Off(void);
void drvif_color_RTNR_AutoMode_RfCvbs(unsigned char channel, unsigned char VD_noise_level);
void drvif_color_RTNR_AutoMode_Off(unsigned char channel);
void drvif_color_RTNR_AutoMode_SdHd(unsigned char channel, unsigned char DNR_Auto_lvl_SdHd);
void drvif_color_RTNR_OF_SNR_AutoMode_RfCvbs(unsigned char display, unsigned char DNR_Auto_level);
void drvif_color_RTNR_OF_SNR_AutoMode_SdHd(unsigned char display, unsigned char DNR_Auto_level);
void drvif_color_Ediff_AutoMode_RfCvbs(unsigned char channel, unsigned char VD_noise_level);
void drvif_color_Ediff_AutoMode_SdHd(unsigned char channel, unsigned char DNR_Auto_lvl_SdHd);
void drvif_color_Spatial_NR_AutoMode_RfCvbs(unsigned char channel, unsigned char VD_noise_level);
void drvif_color_Spatial_NR_AutoMode_SdHd(unsigned char channel, unsigned char DNR_Auto_lvl_SdHd);
void drvif_color_Spatial_NR_curve_mapping_mode_AutoMode_RfCvbs(unsigned char channel, unsigned char VD_noise_level);
void drvif_color_Intelligent_picture_Ctrl_RfCvbs(unsigned char channel, unsigned char VD_noise_level);//for T2 @ RF/CVBS, elieli 20130925
void scalerVIP_Adjusted_Shp_by_HistMeanValue_HDMI(void);//add from demo code for T2 @ HDMI, enhance NR effect, elieli 20131028
unsigned char scalerVIP_RTNR_Level_Step_Change(unsigned char RTNR_LEVEL);//add to reduce NR effect in motion , elieli 20131028
void drvif_color_Spatial_NR_curve_mapping_mode_AutoMode_SdHd(unsigned char channel, unsigned char DNR_Auto_lvl_SdHd);
void drvif_color_Dark_k_value_AutoMode_RfCvbs(unsigned char channel, unsigned char VD_noise_level);
void drvif_color_Dark_k_value_AutoMode_HdSd(unsigned char channel, unsigned char DNR_Auto_lvl_SdHd);
void drvif_color_MCNR_AutoMode_RfCvbs(unsigned char channel, unsigned char VD_noise_level);
void drvif_color_MCNR_AutoMode_HdSd(unsigned char channel, unsigned char DNR_Auto_lvl_SdHd);
void drvif_color_MpegNR_AutoMode_RfCvbs(unsigned char channel, unsigned char VD_noise_level);
void drvif_color_DVBT_MpegNR(void);
void drvif_color_MpegNR_AutoMode_SdHd(unsigned char channel, unsigned char DNR_Auto_lvl_SdHd);
void drvif_color_TV_Auto_Sat_Coring_control(char level_index, unsigned int current_Signal_Status);
unsigned int Lerp_NR_TABLE32(unsigned char X_current, unsigned char NR_LEVEL, unsigned short NRY, unsigned short NRX, unsigned char NRX_OFFSET);
unsigned int Lerp_NR_TABLE8(unsigned char X_current, unsigned char NR_LEVEL, unsigned short NRY, unsigned short NRX, unsigned char NRX_OFFSET);

void scalerVIP_getColorHist(unsigned long arg);
void scaler_Set_New_DCCmode_Enabel(unsigned char  arg);//no use, all change to newDCC
//unsigned char DI_detect_Champagne(unsigned char channel, unsigned int width, unsigned int height);//sony demo
//void DI_Champagne_problem(unsigned char channel);//sony demo
//unsigned char motion_purecolor_concentric_circles_detect(unsigned char channel, unsigned int width, unsigned int height);//sony demo
//unsigned char motion_move_concentric_circles_detect(unsigned char channel, unsigned int width, unsigned int height);//sony demo
void scalerVIP_100_4_Bright_Plus(void);
//unsigned char motion_concentric_circles_detect(unsigned char channel, unsigned int width, unsigned int height);
//unsigned char skin_tone_detect(unsigned char channel, unsigned int width, unsigned int height);
unsigned char drvif_color_chessboard_detection(unsigned char PF_Blk_Num, unsigned char th_profile_bin_cnt_diff, unsigned char th_hist_cnt_max2bin_percent, unsigned char th_hist_bincnt_diff_percent, unsigned char th_hist_binIdx_diff);
void scalerVIP_SET_TABLE_TV_AutoNR_Signal_Status(unsigned long  arg); //csfanchiang,20101020

void scalerVIP_OD_CheckBandwidth(void);

void scalerVIP_CorrectionBit1_Ctrl(unsigned char AutoMA_En, unsigned char VD_noise_level, unsigned char DNR_Auto_lvl_SdHd, unsigned char Y_LEVEL, unsigned char C_LEVEL, unsigned char Fade_flag);

void scalerVIP_RTNR_SceneChange_Policy(unsigned char Enable_flag, unsigned char AutoMA_En, unsigned char VD_noise_level, unsigned char DNR_Auto_lvl_SdHd, unsigned char Y_LEVEL, unsigned char C_LEVEL);


#endif

//***************************************************************//


void scalerVIP_TV003_MAD_Y(unsigned short *PQA_Input_Level, unsigned short *PQA_Input_rate);
void scalerVIP_TV003_share_memory_table(unsigned long arg);
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

//unsigned char scalerVIP_Get_ChromaErrorEn_from_viptab(unsigned char isSmoothtoggle, unsigned char smoothtoggle_source);

void Scaler_color_print_scalerinfo_isr(void);

unsigned char scalerVIP_check_vdc_hw_pwr(void);
void scalerVIP_color_colorspacergb2yuvtransfer(unsigned char channel, unsigned short *table_index);
void scalerVIP_Write_RGB2YUV(unsigned long arg);
void scalerVIP_Set_FW_Film_Mode_En(unsigned char enable);
void scalerVIP_Set_Sharpness_DoubleBufferApply(void);

void scalerVIP_Set_Noise_Level(unsigned char Level);
unsigned char scalerVIP_Get_Noise_Level(void);
unsigned char scalerVIP_Get_PT_NO(void);
void scalerVIP_Set_DCC_ForceWrite_Flag(unsigned long arg);
void scalerVIP_color_DRV_SNR_Veritcal_NR(DRV_SNR_Veritcal_NR *ptr);
void scalerVIP_Teeth_detcted(void);

void scalerVIP_DI_GMV_AVG_for_film(void);
void drvif_color_colorwritegamma_isr(unsigned int *pArray , VIP_GAMMA_CHANNEL_RGB_ISR Channel);

unsigned char scalerVIP_Get_AutoMA_Running(void);
void scalerVIP_DCCCurve_ChangeINT32Endian(int *DCC_apply_curve, unsigned int size );
void scalerVIP_colorSCPUISR_INFO(SCALER_CHANNEL_NUM channel);

//char ScalerVIP_Set_PQ_ByPass_Lv_ISR(unsigned char table_idx);
//char ScalerVIP_Get_PQ_ByPass_Lv_ISR(void);
//char fwif_color_PQ_ByPass_Handler_ISR(unsigned char table_idx, unsigned char isSet_Flag, _RPC_system_setting_info *RPC_VIP_system_info_structure_table, unsigned char Force_byPass);
//char drvif_color_set_PQ_ByPass_ISR(unsigned char bypass_item);

unsigned char ScalerVIP_Get_pq_reset(void);
void scalerVIP_Set_pq_reset(unsigned char bEnable);

unsigned char scalerVIP_access_tv002_style_demo_flag(unsigned char access_mode, unsigned char *pFlag);
void* scalerVIP_get_tv002_style_demo_struct(void);
void scalerVIP_Set_Decontour(void);
void scalerVIP_Res_Det(void);
void scalerVIP_Bad_UpScaling(void);

unsigned short scalerVIP_get_motion_info(void);
void scalerVIP_VGIPISR_enable_status_check(int proc);
int scalerVIP_color_Get_noise_mpeg_detect_y(void);

char scalerVIP_SMF_BlackDetection(unsigned char enable);
char scalerVIP_SMF_BlackDetection_SUB(unsigned char enable);
void scalerVIP_Set_BlackDetection_EN(unsigned char bEnable_main, unsigned char bEnable_sub);
void scalerVIP_Get_BlackDetection_EN(unsigned char* bEnable_main, unsigned char* bEnable_sub);

//void scalerVIP_Set_DeXC_OnOff_by_PlaySpeed(_RPC_system_setting_info* system_RPC_info_structure_table);

#define Pro_tv043 0
#define Pro_tv030 0
#define Pro_tv002 0
#define Pro_tv010 0
#define Pro_tv011 0
#define IDENTIFICATION_EN 1
#define SE_MIX_MODE_REGION_17 0
#define _ABS_(x)	((x)<0? (-1*(x)) : (x))
#define _MIN_(x,y)	(((x)<=(y))? (x) : (y))
#define _MAX_(x,y)	(((x)>=(y))? (x) : (y))
#define _CLIP_(x,dmin, dmax)                  ((x)>(dmax)? (dmax):(x)<(dmin)?(dmin):(x))
#define _CLIP_UBOUND_(x,dmax)	((x)>(dmax)? (dmax):(x))
#define _2Val_RndBlending_(v0, v1, a0, p)	(((v0)*(a0)+(v1)*((1<<(p))-(a0)) + (1<<(p)>>1))>>(p))
#define TAG_NAME_SE_FILM "SE_FILM"
#define	KEEP_FRAME_NUM	60

typedef enum{
	SE_CAD_N = 0,	// no consider this cadence+timing
	SE_CAD_T,		// support
	SE_CAD_F,		// no support
	SE_CAD_TYPE_NUM,
}SE_CAD_SUPPORT_TYPE;

typedef enum{
	_CAD_VIDEO,        // 0
	_CAD_22,           // 1
	_CAD_32,           // 2
	_CAD_32322,        // 3
	_CAD_334,          // 4
	_CAD_22224,        // 5
	_CAD_2224,         // 6
	_CAD_3223,         // 7
	_CAD_55,           // 8
	_CAD_66,           // 9
	_CAD_44,           // 0xA
	_CAD_1112,         // 0xB
	_CAD_11112,        // 0xC
	_CAD_122,          // 0xD
	_CAD_11i23,        // 0xE
	_CAD_BAD_EDIT,        // 0xF
	_CAD_1225_T1,         // 0x10	
	_CAD_1225_T2,	   // 0x11
	_CAD_12214,		   // 0x12
	_FRC_CADENCE_NUM_,
}SE_FRC_CADENCE_ID;

typedef enum
{
	_FILM_ALL,
	_FILM_5RGN_TOP,
	_FILM_5RGN_BOT,
	_FILM_5RGN_LFT,
	_FILM_5RGN_RHT,
	_FILM_5RGN_CEN,
	_FILM_12RGN_01,
	_FILM_12RGN_02,
	_FILM_12RGN_03,
	_FILM_12RGN_04,
	_FILM_12RGN_05,
	_FILM_12RGN_06,
	_FILM_12RGN_07,
	_FILM_12RGN_08,
	_FILM_12RGN_09,
	_FILM_12RGN_10,
	_FILM_12RGN_11,
	_FILM_12RGN_12,

	_FILM_MOT_NUM,

}_SE_FILM_MOT_TYPE;

//////////////////////////////////////////////////////////////////////////
typedef struct
{
	UINT8   u1_filmDet_bypass;
	UINT32  u32_filmDet_cadence_en;

	UINT8   u4_Mot_rgn_diffWgt;
	UINT8   u4_Mot_all_diffWgt;
	UINT32  u27_Mot_all_min;

	UINT8   u4_enter_22_cntGain;
	UINT8   u4_enter_32_cntGain;
	UINT8   u4_enter_else_cntGain;

	UINT8  u1_quit_motWgt_auto_en;
	UINT8  u4_quit_bigMot_wgt;
	UINT8  u4_quit_smlMot_wgt;
	UINT8  u8_quit_cnt_th;
	UINT8  u8_quit_prd_th;

	UINT8  u1_mixMode_en;
	UINT8  u1_mixMode_det_en;
	UINT8  u1_mixMode_rgnEn[_FILM_MOT_NUM];
	UINT8  u8_mixMode_enter_cntTh;
	UINT8  u8_mixMode_cnt_max;
	UINT32 u27_mixMode_rgnMove_minMot;
	UINT8  u4_mixMode_rgnMove_motWgt; // u1.3

	UINT8 u1_FrcCadSupport_En;
	UINT8 u1_QuickSwitch_En;
	UINT8 u1_DbgPrt_FRChg_En;
	UINT8 u1_StillDetect_En;
	UINT8 u3_DbgPrt_Rgn;
	UINT8  u1_BadEditSlowOut_En;


	UINT8  u5_dbg_param1;
	UINT32 u32_dbg_param2;
	UINT32 u32_dbg_cnt;

}_SE_PARAM_FilmDetectCtrl;

typedef struct
{
	UINT32 u27_ipme_motionPool[_FILM_MOT_NUM][8]; // pool length = 8.
	UINT32 u32_Mot_sequence[_FILM_MOT_NUM];
	UINT8  u1_Mot_selBig;

	/////
	UINT32 u27_quit_bigMot[_FILM_MOT_NUM];
	UINT32 u27_quit_smlMot[_FILM_MOT_NUM];
	UINT32 u27_quit_motTh[_FILM_MOT_NUM];
	UINT8  u8_quit_cnt[_FILM_MOT_NUM];
	UINT8  u8_quit_prd[_FILM_MOT_NUM];

	/////
	UINT8  u8_det_cadence_Id[_FILM_MOT_NUM]; // detected cadence according to motion sequence.
	UINT8  u8_pre_cadence_Id[_FILM_MOT_NUM]; // previous output cadence.
	UINT8  u8_cur_cadence_Id[_FILM_MOT_NUM]; // current output cadence.
	UINT8  u8_phase_Idx[_FILM_MOT_NUM];

	/////
	UINT8  u1_cad_resync_true[_FILM_MOT_NUM];
	UINT8  u1_cad_fastOut_true;
	UINT8  u8_mixMode_cnt;
	UINT8  u1_mixMode;

	///
	UINT32 u27_FrmMotion_S[_FILM_MOT_NUM];

	/////
	UINT32 u5_dbg_param1_pre;
	UINT32 u32_dbg_param2_pre;
	UINT32 u32_dbg_cnt_hold;
	UINT8 u8_dbg_motion_t;
	/////
	UINT8  u8_phT_phase_Idx[_FILM_MOT_NUM];

	/////
	UINT32 u27_FrmMotionSort[26];
	UINT32 u27_FrmMotionSortTick[26];

	/////
	UINT32 u27_g_FrmMot_Th[_FILM_MOT_NUM];

	//--output to other module--
	UINT8  u8_cur_cadence_out[_FILM_MOT_NUM]; // current output cadence.
	UINT8  u8_phT_phase_Idx_out[_FILM_MOT_NUM];
	UINT8  u8_flbk_lvl_cad_cnt;
	UINT8  u8_pre_cadence_out; // current output cadence.
	//--end of "output to other module"

	//
	UINT8  u8_cadence_sequence[KEEP_FRAME_NUM];
	UINT8  u8_BadEdit_flag;

	UINT8  u8_reset_flag;
	UINT8  u8_flush_flag;
	//
	UINT32  u32_input_frame_rate;
}_SE_OUTPUT_FilmDetectCtrl;

typedef struct
{
	UINT8  u3_ipme_filmMode_rb; // FRC_TOP__KME_IPME__rdbk_ipme_fdet_film_mode_ADDR
	UINT32  u27_ipme_AllMot_rb;
	UINT32  u27_ipme_CenterMot_rb;
	UINT32  u27_ipme_5Mot_rb[5];
	UINT32  u27_ipme_12Mot_rb[12];
	UINT8  u8_sys_N_rb; // FRC_TOP__CRTC1__reg_crtc1_n_ADDR[15:8]
	UINT8  u8_sys_M_rb; // FRC_TOP__CRTC1__reg_crtc1_n_ADDR[23:16]
	UINT8  u1_SW_sc_true;
}_SE_ReadBack_Regs;

typedef struct
{
	UINT32 cad_seq;
	UINT32 cad_usableFlag;
	UINT8  cad_inLen;
	UINT8  cad_outLen;
	UINT8  cad_isLRin;

	UINT32 cad_check;
	UINT8  cad_len;
	UINT8  cad_film_resync_phId;

	UINT8  cad_id;
	UINT8  u1_isLowFrameRate;

} SE_FRC_CADENCE;

void SE_FilmDetectCtrl_Proc(_SE_PARAM_FilmDetectCtrl *pParam, _SE_OUTPUT_FilmDetectCtrl *pOutput, _SE_ReadBack_Regs *pRBRegs);
//int SE_FilmDetect_SECmd(SE_FMT_ST *pInfo);
int ScalerVIP_SE_Proc(void);

void scalerVIP_reduce_flicker(unsigned char vip_source,unsigned char level);
#endif
