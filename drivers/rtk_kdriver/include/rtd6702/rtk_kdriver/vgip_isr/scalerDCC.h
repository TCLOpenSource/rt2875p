/***************************************************************************
                          scalerVIP.h  -  description
                             -------------------
    begin                : Mon Dec 22 2008
    copyright           : (C) 2008 by hsliao
    email                : hsliao@realtek.com
 ***************************************************************************/

#ifndef __SCALERDCC_H__
#define __SCALERDCC_H__

#include "rtd_types.h"
//#include "kernel/common/gsys_share.h"

// driver header
//#include <kernel/scaler/scalerInfo.h>
#include <scaler/vipCommon.h>

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
	#include <scaler/vdcCommon.h>
#else
	#include <scalercommon/scalerCommon.h>
	#include <scalercommon/vdcCommon.h>
#endif


/*================COLOR_DCC FW register structure define =====*/
#ifndef LITTLE_ENDIAN //apply BIG_ENDIAN
typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  DCC_Level:4;// 28 ~ 31
	RBus_UInt32  res1:3; // 25 ~ 27
	RBus_UInt32  H_LPF_Tap:2; // 23 ~ 24
	RBus_UInt32  Panel_compensation:1;//22
	RBus_UInt32  Boundary_Type:4; // 18:21
	RBus_UInt32  C_LPF_Tap:3; // 15~17
	RBus_UInt32  Curve_LPF_Enable:1; // 14
	RBus_UInt32  Hist_LPF_Enable:1; // 13
	RBus_UInt32  Output_LimitRange_Enable:1; // 12
	RBus_UInt32  Limit_range_Hist_Cut_Enable:1;// 11
	RBus_UInt32  Bin_boundary_Enable:1;// 10

	RBus_UInt32  Database_Enable:1; // 9
	RBus_UInt32  UserCurve_Enable:1; // 8
	RBus_UInt32  Expand_Enable:1; // 7
	RBus_UInt32  Scurve_Enable:1; // 6
	RBus_UInt32  Hist_Adjust_Enable:1;// 5
	RBus_UInt32  DCL_Enable:1; // 4

	RBus_UInt32  SkinTone:1; // 3
	RBus_UInt32  SceneChange:1; // 2
	RBus_UInt32  PureColor:1; // 1

	RBus_UInt32  Dummy_debug_enable:1;//0
    };
}fw_color_dcc_d_dcc_fwused_0;

typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  VipInfo:1;
        RBus_UInt32  fw_1:7;
        RBus_UInt32  ThreeSEGXIndex1:8;
        RBus_UInt32  ThreeSEGXIndex0:8;
        RBus_UInt32  Res1:3;
        RBus_UInt32  HistogramScanningEnable:1;
        RBus_UInt32  CurveCoefInterpolationEnable:1;
        RBus_UInt32  Res0:1;
        RBus_UInt32  ThreeSegCurveEnable:1;
        RBus_UInt32  HighResCurveEnable:1;
    };
}fw_color_dcc_d_dcc_fwused_1;

typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  fw_2:24;
        RBus_UInt32  delay_time:8;
    };
}fw_color_dcc_d_dcc_fwused_2;

typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  ThreeSEGYNegOffset1:8;
	RBus_UInt32  ThreeSEGYNegOffset0:8;
        RBus_UInt32  ThreeSEGYPosOffset1:8;
        RBus_UInt32  ThreeSEGYPosOffset0:8;
    };
}fw_color_dcc_d_dcc_fwused_3;

#else

typedef union
{
    RBus_UInt32 regValue;
    struct{
	RBus_UInt32  Dummy_debug_enable:1;//0
	RBus_UInt32  PureColor:1; // 1
	RBus_UInt32  SceneChange:1; // 2
	RBus_UInt32  SkinTone:1; // 3
	RBus_UInt32  DCL_Enable:1; // 4
	RBus_UInt32  Hist_Adjust_Enable:1;// 5
	RBus_UInt32  Scurve_Enable:1; // 6
	RBus_UInt32  Expand_Enable:1; // 7
	RBus_UInt32  UserCurve_Enable:1; // 8
	RBus_UInt32  Database_Enable:1; // 9
	RBus_UInt32  Bin_boundary_Enable:1;// 10
	RBus_UInt32  Limit_range_Hist_Cut_Enable:1;// 11
	RBus_UInt32  Output_LimitRange_Enable:1; // 12
	RBus_UInt32  Hist_LPF_Enable:1; // 13
	RBus_UInt32  Curve_LPF_Enable:1; // 14
	RBus_UInt32  C_LPF_Tap:3; // 15~17
	RBus_UInt32  Boundary_Type:4; // 18:21
	RBus_UInt32  Panel_compensation:1;//22
	RBus_UInt32  H_LPF_Tap:2; // 23 ~ 24
	RBus_UInt32  res1:3; // 25 ~ 27
	RBus_UInt32  DCC_Level:4;// 28 ~ 31
    };
}fw_color_dcc_d_dcc_fwused_0;

typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  HighResCurveEnable:1;
        RBus_UInt32  ThreeSegCurveEnable:1;
        RBus_UInt32  Res0:1;
        RBus_UInt32  CurveCoefInterpolationEnable:1;
        RBus_UInt32  HistogramScanningEnable:1;
        RBus_UInt32  Res1:3;
        RBus_UInt32  ThreeSEGXIndex0:8;
        RBus_UInt32  ThreeSEGXIndex1:8;
        RBus_UInt32  fw_1:7;
        RBus_UInt32  VipInfo:1;
    };
}fw_color_dcc_d_dcc_fwused_1;

typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  delay_time:8;
        RBus_UInt32  fw_2:24;
    };
}fw_color_dcc_d_dcc_fwused_2;

typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  ThreeSEGYPosOffset0:8;
        RBus_UInt32  ThreeSEGYPosOffset1:8;
	RBus_UInt32  ThreeSEGYNegOffset0:8;
        RBus_UInt32  ThreeSEGYNegOffset1:8;
    };
}fw_color_dcc_d_dcc_fwused_3;


#endif
/*===================================  Types ================================*/

void scalerVIP_colorHistISR(unsigned char channel);
void histogram_new_dcc_write_table(unsigned char display, unsigned char Total_Curve_segment, unsigned char double_buffer_mode_on);
void drvif_adapt_stream_init_smt_flag(void);

void drvif_color_DCC_Curve_Blending(unsigned char channel,unsigned char bin,unsigned int *Adavance_Control_Curve_Blending_info,unsigned int *Adavance_Control_info);
void histogram_new_III_dcc(unsigned char display, unsigned char bit_num ,unsigned char Curve_bit_num);
int drvif_DCC_SC_smoothing_for_curve_delay_applied_condition_check(unsigned char MainDCC_SC, int* Main_preH, int* Gz, unsigned long Curve_diff_thershold, int Scene_change_DCC_apply_delay_flag);

void drvif_color_Get_Section_histogram(unsigned char channel, unsigned char C_Channel_select, unsigned char Section_Select, unsigned char Section_bin_Select, unsigned int *Section_Hist);
#if 1 //from Merlin2 20160629
void drvif_color_Get_Idomain_Y_histogram(unsigned char channel, unsigned char C_Channel_select, unsigned int *reg_HistCnt);
void drvif_color_Get_Ddomain_Y_histogram(unsigned char channel, unsigned char C_Channel_select, unsigned int *reg_HistCnt);
#else
void drvif_color_Get_Y_histogram(unsigned char channel, unsigned char C_Channel_select, unsigned int *reg_HistCnt);
#endif

void scalerVIP_SetDCC_Advance_control_table(unsigned long arg);

unsigned int scalerDCC_get_Adapt_Mean_info(void);
unsigned int scalerDCC_get_Adapt_Dev_info(void);

#if 1
void scalerVIP_ddomain_colorHistISR(unsigned char channel);
void drvif_color_DHIST_SC(unsigned char channel);
void drvif_color_IHIST_SC(unsigned char channel);
void scalerDCC_get_D_hist_window(unsigned short *pWidth, unsigned short *pHeight);
unsigned char scalerVIP_color_D_Hist_Get_APL(unsigned char channel, unsigned int *pHist_Y_APL);
#else
void drvif_color_DCC_SC(unsigned char channel);
#endif
void drvif_color_DCC_SC_TV002(unsigned char channel);
void drvif_color_DCC_SC_TV010(unsigned char channel);
void scalerDCC_tv006_apply_dcc(unsigned char display, unsigned char Curve_bit_num);
void driver_DCC_Chroma_enhance_gain_cal(unsigned char display,unsigned char Table_sel, int *DCC_Apply_Curve,VIP_DCC_Chroma_Compensation_TABLE *table);
void driver_DCC_Chroma_enhance_gain_apply(unsigned char display,unsigned char Table_sel, int *chroma_gain);
unsigned char scalerVIP_colorHist_Get_APL(unsigned char channel, unsigned int *pHist_Y_APL);
void scalerDCC_get_hist_window(unsigned short *pWidth, unsigned short *pHeight);

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
void fwif_color_dcc_Curve_interp_tv006(unsigned short *curve32, unsigned short *curve129);
void fwif_color_apply_dcc_tv006(UINT8 display, UINT8 Curve_bit_num, unsigned short *Apply_Curve, int *to_SRAM_Curve);
void dcc_user_curve_write_table_tv006(UINT8 display, UINT8 Total_Curve_segment, unsigned short *Apply_Curve, int *to_SRAM_Curve, UINT8 *wrtie_flag, UINT8 *apply_flag);
#endif
#endif




