/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for color related functions.
 *
 * @author 	$Author$
 * @date 	$Date$
 * @version $Revision$
 */

/**
 * @addtogroup color
 * @{
 */

#ifndef _VIP_COLOR_H
#define _VIP_COLOR_H

/*================================ Definitions ==============================*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_VPQ_TABLE_GENERATE
#define CONFIG_VPQ_TABLE_GENERATE 0
#endif

#define ENABLE_HDR_DM_22b_PATH 1

/*******************************************************************************
 * Header include
 ******************************************************************************/
#include <scaler/vipCommon.h>
#include <scaler/vipRPCCommon.h>
#include <scaler/scalerCommon.h>
#include <tvscalercontrol/vip/tc_hdr.h>
#include <tvscalercontrol/vip/st2094.h>
#include <rbus/rbus_types.h>
#ifdef BUILD_QUICK_SHOW
#include <sysdefs.h>
#endif
/*******************************************************************************
 * Marco
 ******************************************************************************/


/*******************************************************************************
 * Variable
 ******************************************************************************/


/*******************************************************************************
* Constant
******************************************************************************/

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
#define _RGB2YUV_set_Yin_offset			0x0d
#define _RGB2YUV_set_UV_out_offset		0x0e
#define _RGB2YUV_sel_UV_off				0x0f
#define _RGB2YUV_Matrix_bypass			0x10
#define _RGB2YUV_Enable_Y_gain			0x11


#define _YUV2RGB_k11					0x00
#define _YUV2RGB_k12					0x01
#define _YUV2RGB_k13					0x02
#define _YUV2RGB_k22					0x03
#define _YUV2RGB_k23		 			0x04
#define _YUV2RGB_k32					0x05
#define _YUV2RGB_k33					0x06
#define _YUV2RGB_Roffset	 			0x07
#define _YUV2RGB_Goffset				0x08
#define _YUV2RGB_Boffset				0x09
#define _YUV2RGB_Y_Clamp				0x0a
#define _YUV2RGB_UV_Clamp				0x0b


#define _Gamma_Full						0x01
#define _Gamma_Compact					0x00

#define _ICM_ALL_DATA_HUE 				0
#define _ICM_ALL_DATA_SAT 				2
#define _ICM_ALL_DATA_ITN 				4
#define _ICM_ALL_DATA_SOFTCLAMP_CTRL 	6
#define _ICM_ALL_DATA_SOFTCLAMP_SLOPE 	7
#define _SOFTCLAMP_SAT_SLOP_EN			2
#define _SOFTCLAMP_ITN_SLOP_EN			1

#define _SRGB_TABLE_BYPASS				0
#define _SRGB_TABLE_COEFF1				1
#define _SRGB_TABLE_COEFF2				2
#define _SRGB_TABLE_RESERVED			3

#define _SRGB_MAIN_TABLE_SEL			_SRGB_TABLE_COEFF1
#define _SRGB_SUB_TABLE_SEL				_SRGB_TABLE_COEFF1

/*hsinyi 20090617*/
/* Definitions of Digital Filter===============*/
#define _DISABLE_PORT                   0
#define _PHASE_ACCESS_PORT              1
#define _NEG_SMEAR_ACCESS_PORT     		2
#define _POS_SMEAR_ACCESS_PORT			3
#define _NEG_RING_ACCESS_PORT        	4
#define _POS_RING_ACCESS_PORT			5
#define _MISMATCH_ACCESS_PORT			6
#define _YPBPR_ACCESS_PORT              7
#define _NOISE_REDUCTION_PORT        	8

#define _YPBPR_DISABLE                  0
#define _YPBPR_ENABLE                   7

#define _PHASE_THD_0                    0
#define _PHASE_THD_1                    1
#define _PHASE_THD_2                    2
#define _PHASE_THD_3                    3
#define _PHASE_THD_4                    4
#define _PHASE_THD_5                    5
#define _PHASE_THD_6                    6
#define _PHASE_THD_7                    7

#define _SMEAR_RING_THD_0               0
#define _SMEAR_RING_THD_1               1
#define _SMEAR_RING_THD_2               2
#define _SMEAR_RING_THD_3               3
#define _SMEAR_RING_THD_4               4
#define _SMEAR_RING_THD_5               5
#define _SMEAR_RING_THD_6               6
#define _SMEAR_RING_THD_7               7

#define _MISMATCH_THD_0                 0
#define _MISMATCH_THD_1                 1

#define _NOISE_REDUCTION_THD_0          0
#define _NOISE_REDUCTION_THD_1          1
#define _NOISE_REDUCTION_THD_2          2
#define _NOISE_REDUCTION_THD_3          3
#define _NOISE_REDUCTION_THD_4          4
#define _NOISE_REDUCTION_THD_5          5
#define _NOISE_REDUCTION_THD_6          6
#define _NOISE_REDUCTION_THD_7          7

#define _DIV_VALUE_0                    0
#define _DIV_VALUE_1                    1
#define _DIV_VALUE_2                    2
#define _DIV_VALUE_3                    3

#define VIP_DITHERLUT_ROW_MAX			3
#define VIP_DITHERLUT_COL_MAX			16
#define VIP_DITHERLUT_DEP_MAX			4

#define VIP_HDMI_DITHERLUT_COL_MAX		4
#define VIP_HDMI_DITHERLUT_DEP_MAX		4

#define _YUV2RGB_UVOFFSET_BY_Y_UOFFSET	0x00
#define _YUV2RGB_UVOFFSET_BY_Y_VOFFSET	0x01
#define _YUV2RGB_UVOFFSET_BY_Y_USTEP	0x02
#define _YUV2RGB_UVOFFSET_BY_Y_VSTEP	0x03

#define _YUV2RGB_COEF_BY_Y_M0			0x00
#define _YUV2RGB_COEF_BY_Y_M1			0x01
#define _YUV2RGB_COEF_BY_Y_M2			0x02
#define _YUV2RGB_COEF_BY_Y_M3			0x03
#define _YUV2RGB_COEF_BY_Y_M4			0x04
#define _YUV2RGB_COEF_BY_Y_M5			0x05
#define _YUV2RGB_COEF_BY_Y_M6			0x06
#define _YUV2RGB_COEF_BY_Y_M7			0x07
#define _YUV2RGB_COEF_BY_Y_M8			0x08
#define _YUV2RGB_COEF_BY_Y_M9			0x09
#define _YUV2RGB_COEF_BY_Y_M10			0x0a
#define _YUV2RGB_COEF_BY_Y_M11			0x0b
#define _YUV2RGB_COEF_BY_Y_M12			0x0c
#define _YUV2RGB_COEF_BY_Y_M13			0x0d
#define _YUV2RGB_COEF_BY_Y_M14			0x0e
#define _YUV2RGB_COEF_BY_Y_M15			0x0f
#define _YUV2RGB_COEF_BY_Y_M16			0x10

#define _YUV2RGB_COEF_BY_Y_k11			0x00
#define _YUV2RGB_COEF_BY_Y_k12			0x01
#define _YUV2RGB_COEF_BY_Y_k13			0x02
#define _YUV2RGB_COEF_BY_Y_k22			0x03
#define _YUV2RGB_COEF_BY_Y_k23			0x04
#define _YUV2RGB_COEF_BY_Y_k32			0x05
#define _YUV2RGB_COEF_BY_Y_k33			0x06

#define VIP_DYNAMIC_DITHERING_TABLE_MAX	2

#define Bin_Num_sRGB 					128	/* Mac2=32, Sirius=128 (jyyang_2013/12/28)*/

#define IRE_PATTERN_USE_3D_PTG	0

typedef enum _VC_MS_MODE{
	VC_MS_ALL_DISABLE = 0,
	VC_MS_MAIN_ONLY = 1,
	VC_MS_SUB_ONLY = 2,
	VC_MS_RESERVED3 = 3,
	VC_MS_RESERVED4 = 4,
	VC_MS_MAIN_AND_OVERLAY = 5,
	VC_MS_SUB_AND_OVERLAY = 6,
	VC_MS_ALL_ENABLE = 7,
} VC_MS_MODE;

typedef enum _VIP_HDR_DM_CSC1_YUV2RGB_TABLE {
	HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255 = 0,
	HDR_DM_CSC1_YUV2RGB_BT709_255_to_255,
	HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255,
	HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255,
	HDR_DM_CSC1_YUV2RGB_ByPass_255_to_255,
	HDR_DM_CSC1_YUV2RGB_ByPass_Limted_235_to_255,

	HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255,
	HDR_DM_CSC1_YUV2RGB_BT601_255_to_255,

	HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_255_to_255,
	HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_Limted_235_to_255,

	HDR_DM_CSC1_YUV2RGB_BT709_255_to_235_240,
	HDR_DM_CSC1_YUV2RGB_BT2020_255_to_235_240,
	HDR_DM_CSC1_YUV2RGB_BT601_255_to_235_240,
	HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_255_to_235,

	HDR_DM_CSC1_YUV2RGB_HDR10_VIVID_235_to_255,
	VIP_HDR_DM_CSC1_YUV2RGB_TABLE_Max,

} VIP_HDR_DM_CSC1_YUV2RGB_TABLE;

#define HDR_TABLE_NUM 5
/*#define HDR_RGB2YUV_TABLE_NUM 5*/

typedef enum {
	HDR_RGB2OPT_709_TO_2020 = 0,
	HDR_RGB2OPT_2020_TO_709,
	HDR_RGB2OPT_DCI_TO_709,
	HDR_RGB2OPT_DCI_TO_2020,
	HDR_RGB2OPT_2020_TO_DCI,
	HDR_RGB2OPT_RGB_TO_Y,
	HDR_RGB2OPT_TABLE_NUM,
} VIP_HDR_RGB2OPT_TABLE;

/* DM2 */
#define EOTF_size 1025
#define OETF_size 1025
#define OETF_size_64p 67
#define maxcll_num 6
#define ToneMapping_size 129
#define HDR_Histogram_size 128
#define HDR_24x24x24_size (24*24*24)

typedef enum _VIP_DM2_Hist_Mode {
	DM2_Hist_Y_Mode = 0,
	DM2_Hist_RGB_MAX_Mode,
	DM2_Hist_MODE_MAX,
} VIP_DM2_Hist_Mode;

typedef struct _VIP_HDR10_EOTF_ByLuminace_TBL {
	unsigned short EOTF_setting[EOTF_size];
} VIP_HDR10_EOTF_ByLuminace_TBL;

typedef struct _VIP_HDR10_OETF_ByLuminace_TBL {
	unsigned int OETF_setting[OETF_size_64p];

	unsigned int OETF_calman[maxcll_num][OETF_size_64p];
	unsigned int OETF_normal[maxcll_num][OETF_size_64p];	
} VIP_HDR10_OETF_ByLuminace_TBL;


/*******************************************************************************
 * Structure
 ******************************************************************************/
typedef struct {
	unsigned char Data_sel;
	unsigned char maxlen;
	unsigned char psmth;
	unsigned char lpmode;
	unsigned char EngDiv;
	unsigned char offDiv;
	unsigned char uvGain;
	unsigned char Dcti_mode;
	unsigned char cur_sel;
	unsigned char Dcti_en;
} DRV_VipSHDIDcti_t;

typedef struct {
	unsigned char u_delay;
	unsigned char v_delay;

} DRV_VipUV_Delay;


typedef struct {
	unsigned char yc_delay;
	unsigned char cb_delay;
	unsigned char cr_delay;
	unsigned char new_crldelay_en;
	unsigned char new_cbldelay_en;
	unsigned char new_yldelay_en;
	unsigned char crldelay_en;
	unsigned char cbldelay_en;
	unsigned char yldelay_en;
	unsigned char crl_delay;
	unsigned char cbl_delay;
	unsigned char yl_delay;

} DRV_VipUV_Delay_TOP;

typedef struct {
	DRV_VipUV_Delay_TOP VipUV_Delay_TOP;
} DRV_VipUV_Delay_TOP_Table;


typedef struct {
	unsigned int BSum;
	unsigned int GSum;
	unsigned int RSum;
	unsigned char BMax;
	unsigned char BMin;
	unsigned char GMax;
	unsigned char GMin;
	unsigned char RMax;
	unsigned char RMin;
} RGB_pixel_info;

typedef struct  {
	unsigned char HDR10_EN;
	unsigned char HDR10_420_2_422;
	unsigned char HDR10_422_2_444;
	unsigned char HDR10_YUV2RGB;
	unsigned char HDR10_EOTF;
	unsigned char HDR10_CMT;
	unsigned char HDR10_OETF;
	unsigned char HDR10_RGB2OPT;
	unsigned char HDR10_DITHER;
} DRV_HDR10_submodule_CTL;

typedef struct  {
	unsigned int dolby_mode;
	unsigned int hdmi_in_mux;
	unsigned int dolby_hsize;
	unsigned int dolby_vsize;
	unsigned int dolby_h_total;
	unsigned int dolby_h_den_sta;
	unsigned int dolby_v_den_sta;
} DRV_HDR10_top_init_CTL;

typedef struct  {

	unsigned char wpr_on;
	unsigned char opc_on;
	unsigned char mce_on;
	unsigned char type_sel;
	unsigned char mode_3d;
	unsigned char test_mode;
	unsigned char mode_sel;


} DRV_Mplus_ModeChange;

typedef enum _VIP_RGBW_MODE {
	VIP_RGBW_MODE_DISABLE = 0,
	VIP_RGBW_MODE_LGDMPLUS,
	VIP_RGBW_MODE_RTKRGBW,
	VIP_RGBW_MODE_HIMAXBOE,
	VIP_RGBW_MODE_MAX,
} VIP_RGBW_MODE;

typedef struct
{
	unsigned int dma_EOTF_startaddr;
	unsigned int dma_OETF_startaddr;
	unsigned int dma_HDR_3Dlut_startaddr;
	unsigned int dma_D_3DLut_startaddr;
	unsigned int dma_ICM_startaddr;

} VIP_DMA_Memory_Addr;

typedef struct _yuvdata
{
	unsigned short y;
	unsigned short u;
	unsigned short v;
} yuvdata;

typedef struct  {
	unsigned short h_sta;
	unsigned short h_end;
	unsigned short v_sta;
	unsigned short v_end;
} Drvif_SCALER_DISPLAY_DATA;

typedef enum _VIP_8VERTEX_RGB {
	VIP_8VERTEX_R = 0,
	VIP_8VERTEX_G,
	VIP_8VERTEX_B,

	VIP_8VERTEX_RGB_MAX
} VIP_8VERTEX_RGB;

typedef enum _VIP_8VERTEX {
	VIP_8VERTEX_WHITE = 0,
	VIP_8VERTEX_BLACK,
	VIP_8VERTEX_RED,
	VIP_8VERTEX_GREEN,
	VIP_8VERTEX_BLUE,
	VIP_8VERTEX_YELLOW,
	VIP_8VERTEX_CYAN,
	VIP_8VERTEX_MAGENTA,

	VIP_8VERTEX_MAX
} VIP_8VERTEX;

/*******************************************************************************
* enumeration
******************************************************************************/

typedef enum _VIP_GAMMA_TYPE {
	VIP_GAMMA_MODE1_R = 0,
	VIP_GAMMA_MODE1_G,
	VIP_GAMMA_MODE1_B,
	VIP_GAMMA_MODE2_R,
	VIP_GAMMA_MODE2_G,
	VIP_GAMMA_MODE2_B,
	VIP_GAMMA_MODE3_R,
	VIP_GAMMA_MODE3_G,
	VIP_GAMMA_MODE3_B,
	VIP_GAMMA_MODE4_R,
	VIP_GAMMA_MODE4_G,
	VIP_GAMMA_MODE4_B,
	VIP_GAMMA_MODE5_R,
	VIP_GAMMA_MODE5_G,
	VIP_GAMMA_MODE5_B,
	VIP_GAMMA_MODE6_R,
	VIP_GAMMA_MODE6_G,
	VIP_GAMMA_MODE6_B,
	VIP_GAMMA_MODE7_R,
	VIP_GAMMA_MODE7_G,
	VIP_GAMMA_MODE7_B,
	VIP_GAMMA_MODE8_R,
	VIP_GAMMA_MODE8_G,
	VIP_GAMMA_MODE8_B,
	VIP_GAMMA_MODE9_R,
	VIP_GAMMA_MODE9_G,
	VIP_GAMMA_MODE9_B,
	VIP_GAMMA_MODE10_R,
	VIP_GAMMA_MODE10_G,
	VIP_GAMMA_MODE10_B,
	VIP_GAMMA_MODE11_R,
	VIP_GAMMA_MODE11_G,
	VIP_GAMMA_MODE11_B,
	VIP_GAMMA_MODE12_R,
	VIP_GAMMA_MODE12_G,
	VIP_GAMMA_MODE12_B,

} VIP_GAMMA_TYPE;

typedef enum _VIP_PANEL_BIT {
	VIP_PANEL_BIT6 = 0,
	VIP_PANEL_BIT7,
	VIP_PANEL_BIT8,
	VIP_PANEL_BIT9,
	VIP_PANEL_BIT10,
	VIP_PANEL_BIT_OTHERS,
} VIP_PANEL_BIT;

typedef enum _VIP_DITHER_IP {
	VIP_DITHER_IP_MAIN = 0,
	VIP_DITHER_IP_HDMI,
	VIP_DITHER_IP_HSD,
	#ifdef CONFIG_HW_SUPPORT_MEMC
	VIP_DITHER_IP_MEMC_MUX,
	#endif //CONFIG_HW_SUPPORT_MEMC
	VIP_DITHER_IP_LGD,
	VIP_DITHER_IP_PCID,
	VIP_DITHER_IP_PANEL,
	VIP_DITHER_IP_DD,
	VIP_DITHER_IP_MAX,
} VIP_DITHER_IP;

typedef enum _VIP_DITHER_SEQUENCE {
	VIP_DITHER_SEQUENCE_MAIN = 0,
	VIP_DITHER_SEQUENCE_PANEL,
	VIP_DITHER_SEQUENCE_MAX,
} VIP_DITHER_SEQUENCE;

typedef enum _VIP_DITHER_MAINTABLE {
	/*        12to10       */
	VIP_DITHER_MAINTABLE_DEFAULT_12to10= 0,
	VIP_DITHER_MAINTABLE_LGD_12to10,
	VIP_DITHER_MAINTABLE_HDR10_12to10,
	/*        12to8       */
	VIP_DITHER_MAINTABLE_DEFAULT_12to8,
	VIP_DITHER_MAINTABLE_BLANCE_12to8,
	VIP_DITHER_MAINTABLE_CP_12to8,
	VIP_DITHER_MAINTABLE_MAX,
} VIP_DITHER_MAINTABLE;

typedef enum _VIP_DITHER_PANELTABLE {
	VIP_DITHER_PANELTABLE_DEFAULT = 0,
	VIP_DITHER_PANELTABLE_BLANCE,
	VIP_DITHER_PANELTABLE_BLANCE_ROLL_15,
	VIP_DITHER_PANELTABLE_MAX,
} VIP_DITHER_PANELTABLE;

typedef enum _VIP_DITHER_TEMPORAL {
	VIP_DITHER_TEMPORAL_MAIN_GENERAL = 0,
	VIP_DITHER_TEMPORAL_HSD,     //  HSD hw default
	VIP_DITHER_TEMPORAL_MEMC,  // HIGH FREQ.
	VIP_DITHER_TEMPORAL_MAX,
} VIP_DITHER_TEMPORAL;

typedef enum _VIP_UV_DELAY_ENABLE {
	VIP_UV_DELAY_OFF = 0,
	VIP_UV_DELAY_ON,
} VIP_UV_DELAY_ENABLE;

typedef enum _VIP_GAMMA_CHANNEL_RGB {
	GAMMA_CHANNEL_R = 0,
	GAMMA_CHANNEL_G,
	GAMMA_CHANNEL_B,
	VIP_GAMMA_CHANNEL_RGB_MAX,
	GAMMA_CHANNEL_W,
} VIP_GAMMA_CHANNEL_RGB;

typedef enum _VIP_RGB2YUV_OUT_DATA_Ctrl {
	RGB2YUV_OUT_DATA_Old_Mode = 0,
	RGB2YUV_OUT_DATA_16_235,
	RGB2YUV_OUT_DATA_0_255,
	RGB2YUV_OUT_DATA_newTable_newPath_Mode,
	RGB2YUV_OUT_DATA_0_255_HDR10_Mode,

	RGB2YUV_OUT_DATA__Max,
} VIP_RGB2YUV_OUT_DATA_Ctrl;

typedef enum _VIP_RGB2YUV_COEF_MATRIX_MODE {
	RGB2YUV_COEF_BYPASS_NORMAL_GAIN = 0,
	RGB2YUV_COEF_BYPASS_0_255_TO_16_235,
	RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235,
	RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235,
	RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235,
	RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235,
	RGB2YUV_COEF_RGB_0_255_TO_YCC709_0_255,
	RGB2YUV_COEF_TV006_YPP_LOW,
	RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235,
	RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255,

	RGB2YUV_COEF_RGB601_0_255_TO_YCC709_16_235, //yvonne
	RGB2YUV_COEF_RGB601_16_235_TO_YCC709_16_235, //yvonne2
	RGB2YUV_COEF_RGB2020_0_255_TO_YCC709_16_235, //yvonne
	RGB2YUV_COEF_RGB2020_16_235_TO_YCC709_16_235, //yvonne2

	RGB2YUV_COEF_YCC601_0_255_TO_YCC709_0_255,
	RGB2YUV_COEF_YCC601_16_235_TO_YCC709_16_235, //yvonne
	RGB2YUV_COEF_YCC601_0_255_TO_YCC709_16_235, //yvonne
	RGB2YUV_COEF_YCC709_0_255_TO_YCC709_16_235, //yvonne
	RGB2YUV_COEF_YCC2020_0_255_TO_YCC2020_16_235, //yvonne
	RGB2YUV_COEF_YCC2020_16_235_TO_YCC709_16_235,
	RGB2YUV_COEF_YCC2020_0_255_TO_YCC709_16_235,

	RGB2YUV_COEF_MATRIX_MODE_Max,

} VIP_RGB2YUV_COEF_MATRIX_MODE;

typedef enum _VIP_RGB2YUV_COEF_MATRIX_ITEMS {
	RGB2YUV_COEF_M11 = 0,
	RGB2YUV_COEF_M12,
	RGB2YUV_COEF_M13,
	RGB2YUV_COEF_M21,
	RGB2YUV_COEF_M22,
	RGB2YUV_COEF_M23,
	RGB2YUV_COEF_M31,
	RGB2YUV_COEF_M32,
	RGB2YUV_COEF_M33,
	RGB2YUV_COEF_Yo_Even,
	RGB2YUV_COEF_Yo_Odd,
	RGB2YUV_COEF_Yo_Gain,
	RGB2YUV_COEF_Sel_RGB,
	RGB2YUV_COEF_Sel_Yin_Offset,
	RGB2YUV_COEF_Sel_UV_Out_Offset,
	RGB2YUV_COEF_Sel_UV_Off,
	RGB2YUV_COEF_Matrix_Bypass,
	RGB2YUV_COEF_Enable_Y_Gain,

	RGB2YUV_COEF_MATRIX_ITEMS_Max,

} VIP_RGB2YUV_COEF_MATRIX_ITEMS;

typedef enum _COLOR_FUN_BYPASS {
	BYPASS_VIVID_COLOR = 0,
	BYPASS_RGB_CON,
	BYPASS_RGB_BRI,
	BYPASS_GAMMA,
	BYPASS_D_DITHER,
	BYPASS_SRGB,
	BYPASS_RGB2YUV_MATRIX,
	BYPASS_YUV2RGB_MATRIX,
} COLOR_FUN_BYPASS;

typedef enum _BYPASS_FLAG {
	IP_BYPASS = 0,
	IP_NO_BYPASS
} BYPASS_FLAG;

typedef enum _HDR_table_item {
	HDR_EN = 0,
	HDR_420_2_422,
	HDR_422_2_444,
	HDR_DITHER,

	HDR_ITEM_MAX,
} HDR_table_item;

/*******************************************************************************
 * Structure
 ******************************************************************************/
typedef struct {
	unsigned char gamma_adapt_en;
	unsigned short sat_thl;
	unsigned short sat_thl_mul;
} DRV_Adaptive_Gamma_Ctrl;

typedef struct {
	COLOR_FUN_BYPASS idIP;
	unsigned char bypass_switch;
} Color_Fun_Bypass;
#define Bin_Num_Output_InvOutput_Gamma 256
enum {
	Output_InvOutput_Gamma_TBL_Table0,//from bootcode bin Tcon_Outputgamma.bin if there's it
	Output_InvOutput_Gamma_TBL_Table1,
	Output_InvOutput_Gamma_TBL_TV030,
	Output_InvOutput_Gamma_TBL_SCurve,
	Output_InvOutput_Gamma_TBL_Reserved04,
	Output_InvOutput_Gamma_TBL_MAX //5
};
//#define Output_InvOutput_Gamma_TBL_MAX 5

/* demura DRV*/
#define VIP_Demura_Adaptive_Scale_LV_NUM 10
#define VIP_Demura_Encode_TBL_ROW 271/*136*/
#define VIP_Demura_Encode_TBL_COL 3312/*1680*/
#define VIP_DemuraTBL_LineSize_RGB_481x271 3312
#define VIP_DemuraTBL_LineSize_Y_481x271 1104
#define VIP_DemuraTBL_LineSize_RGB_241x136 1680
#define VIP_DemuraTBL_LineSize_Y_241x136 560

typedef struct {
	unsigned int hdr_table_size;
	unsigned char hdr_table[HDR_TABLE_NUM][HDR_ITEM_MAX];
	unsigned int hdr_YUV2RGB[VIP_HDR_DM_CSC1_YUV2RGB_TABLE_Max][12];
	unsigned int hdr_RGB2OPT[HDR_RGB2OPT_TABLE_NUM][9];
	unsigned short HDR10_3DLUT_17x17x17[17*17*17][3];
	unsigned int HLG_3DLUT_17x17x17[17*17*17][3];
	unsigned short HDR10_3DLUT_24x24x24[HDR_24x24x24_size][3];
	unsigned short HLG_3DLUT_24x24x24[HDR_24x24x24_size][3];
	unsigned int EOTF_LUT_R[1025];
	unsigned short OETF_LUT_R[1025];
	unsigned short HLG_OETF_LUT_R[OETF_size];
	unsigned int HLG_EOTF_LUT_R[EOTF_size];
	unsigned int HDR10_EOTF_2084[EOTF_size];
	VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_250N;
	VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_300N;
	VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_400N;
	VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_500N;
	VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_600N;
	VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_700N;
	VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_800N;
	unsigned short panel_luminance;
	VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_250N;
	VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_300N;
	VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_400N;
	VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_500N;
	VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_600N;
	VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_700N;
	VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_800N;
	VIP_TCHDR_Mode_TBL TCHDR_Mode_TBL;
	DRV_TCHDR_Table TCHDR_Table[TC_Table_NUM];
	DRV_TCHDR_COEF_I_lut TCHDR_COEF_I_LUT_TBL;
	DRV_TCHDR_COEF_S_lut TCHDR_COEF_S_LUT_TBL;
	DRV_TCHDR_v130_D_Table_settings TCHDR_v130_D_Table_settings[NUM_D_LUT_DEFAULT];
} SLR_HDR_TABLE;

// VIP status check, VIP self test
typedef struct  _VIP_IP_Status_Check_List {
	unsigned char name[28];	// char
	unsigned int reg_addr;
	unsigned int reg_mask;
	unsigned int reg_addr_clk;
	unsigned int reg_mask_clk;
	
} VIP_IP_Status_Check_List;

typedef struct  _VIP_Self_Check_Reg_Get {
	unsigned char dolby_mode;
	unsigned char eotf_lut_enable;
	unsigned char dm_b0104_enable;
	unsigned char oetf_lut_enable;
	unsigned char dm_b0501_enable;

} VIP_Self_Check_Reg_Get;


#if 0//ndef LITTLE_ENDIAN //apply BIG_ENDIAN

//======INVOUTPUTGAMMA register structure define==========

typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  res1:2;
        RBus_UInt32  out_gamma_tab_index:14;
        RBus_UInt32  out_gamma_tab_d0:8;
        RBus_UInt32  out_gamma_tab_d1:8;
    };
}out_gamma_encode;
#else

typedef union
{
    RBus_UInt32 regValue;
    struct{
        RBus_UInt32  out_gamma_tab_d1:8;
        RBus_UInt32  out_gamma_tab_d0:8;
        RBus_UInt32  out_gamma_tab_index:14;
        RBus_UInt32  res1:2;
    };
}out_gamma_encode;

#endif

/*******************************************************************************
 * Program
 ******************************************************************************/
VIP_IP_Status_Check_List* drvif_Get_IP_Status_Check_List_TBL(unsigned char *tbl_item_num);
unsigned int drvif_color_get_reg_Val(unsigned int reg_addr, unsigned char reg_shift, unsigned int reg_mask);
char drvif_Self_Check_Reg_Get(unsigned char display, VIP_Self_Check_Reg_Get *pReg_set);

 /* ====  Dither ======*/
void drvif_color_Setditherbit(VIP_PANEL_BIT default_vip_panel_bit);  // for panel dither
void drvif_color_set_Main_Dither_Enable(unsigned char enable);
void drvif_color_set_ColorTemp_Dither_Enable(unsigned char enable);
void drvif_color_set_HDMI_Dither_Enable(unsigned char enable);
void drvif_color_set_HSD_Dither_Enable(unsigned char enable);
void drvif_color_Panel_Dither_mode(unsigned char mode);
void drvif_color_set_Panel_Dither_Enable(unsigned char enable);
void drvif_color_set_Panel_Dither_Temporal_Enable(unsigned char enable);
void drvif_color_set_Panel_Dither_OLED_Enable(unsigned char enable);
void drvif_color_set_Panel_Dither_OLED_Th(unsigned int th);
void drvif_color_set_Dynamic_Dither(unsigned char enable,unsigned char * tDDither_Coef);
void drvif_color_set_PCID_Dither_Enable(unsigned char enable);
void drvif_color_set_LGD_Dither_Enable(unsigned char enable);

void drvif_color_set_MainType_Sequence(unsigned char DitherIP,unsigned char  (*pDITHER_SEQUENCE)[16]);
void drvif_color_set_PanelType_Sequence(unsigned char DitherIP,unsigned char (*pDITHER_SEQUENCE)[16]);
void drvif_color_set_MainType_Dither_Table(unsigned char DitherIP,unsigned char (*pDITHER_TABLE)[4][4]);
void drvif_color_set_PanelType_Dither_Table(unsigned char DitherIP,unsigned char  (*pDITHER_TABLE)[16][4]);
void drvif_color_set_Dither_Sequence(unsigned char DitherIP,unsigned char (*pDITHER_SEQUENCE)[16]);
void drvif_color_set_Dither_Temporal(unsigned char DitherIP,unsigned char * pDITHER_TEMPORAL);
void drvif_color_set_sub_Dither_Enable(unsigned char enable);
/* ===============================*/
#if !CONFIG_VPQ_TABLE_GENERATE
void drvif_color_set_BOE_RGBW(unsigned short *ptr_tab, int w_rate);
void drvif_color_set_Panel_Bit(VIP_PANEL_BIT bit);
void drvif_color_setSD_dithering(unsigned char enable);
void drvif_color_setydlti(unsigned char display, unsigned char Level);
void drvif_color_setgamma(unsigned char display, unsigned char Mode, VIP_Gamma *ptr);
void drvif_color_setycontrastbrightness_new(unsigned char display, unsigned char Contrast, unsigned char Brightness, unsigned char csp_control);
void drvif_color_setYUV2RGB_CSMatrix(DRV_VIP_YUV2RGB_CSMatrix *ptr, VIP_CSMatrix_WriteType WriteMode, VIP_YUV2RGB_Y_SEG idx, unsigned char display, VIP_Table_Select tbl);
void drvif_color_setrgbcontrast(unsigned char display, unsigned short Red, unsigned short Green, unsigned short Blue, unsigned char  BlendRatio, unsigned char Overlay);
void drvif_color_setrgbbrightness(unsigned char display, unsigned short Red, unsigned short Green, unsigned short Blue);
void drvif_color_gamma_control_front(unsigned char display);
void drvif_color_gamma_control_back(unsigned char display, unsigned char ucWriteEnable);

void drvif_color_output_gamma_control_front(unsigned char display);
void drvif_color_output_gamma_control_back(unsigned char display);
void drvif_color_inv_output_gamma_control_front(unsigned char display);
void drvif_color_inv_output_gamma_control_back(unsigned char display);
void drvif_color_inv_out_gamma_control_back(void);
void drvif_color_colorread_inv_output_gamma(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorwrite_inv_output_gamma(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorwrite_inv_output_gamma_Enable(unsigned char Enable);
unsigned char drvif_color_colorread_inv_output_gamma_Enable(void);
#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
void drvif_color_colorwrite_mixer_gamma1(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorwrite_mixer_gamma1_Enable(unsigned char Enable);
void drvif_color_colorwrite_mixer_gamma2(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorwrite_mixer_gamma2_Enable(unsigned char Enable);
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
void drvif_color_set_output_gamma_format(unsigned char Enable, unsigned char loc, unsigned char ch_order);
unsigned char drvif_color_Get_RGBW_Mode(void);
void drvif_color_set_RGBW(void);
void drvif_color_set_RGBW_deoversat(unsigned char level);
void drvif_color_set_RGBW_hfreq(unsigned char level);
void drvif_color_set_RGBW_newCM_sprOffset(unsigned char level);
void drvif_color_set_RGBW_spr_coef(unsigned char mode, unsigned char ch_order);
void drvif_color_sub_huesat(unsigned char display, short Hue, short Sat);
void drvif_color_set422to444(unsigned char channel, unsigned char bEnable);
void drvif_color_sethighlightwindowstep(unsigned short hstep, unsigned short wstep, unsigned short vstep, unsigned short lstep);
void drvif_color_set_sethighlightwindowmin(unsigned short minwidth, unsigned short minlen);
void drvif_color_sethighlightwindowborder(unsigned char Border, unsigned char BorderType);
unsigned char drvif_color_gethighlightwindow_enable(void);
void drvif_color_sethighlightwindow(unsigned char Ena_bit, unsigned short HSTA, unsigned short VSTA, unsigned short HEND, unsigned short VEND, unsigned short Border);
void drvif_color_colorspacergb2yuvtransfer(unsigned char channel, unsigned short *table_index);
void drvif_color_setpiprgbcontrastbrightness(unsigned char BlendRatio);
void drvif_color_setBrightnessDualplayOverlay(unsigned int mainBlendFactor,unsigned int subBlendFactor);
void drvif_color_setuv_sat(unsigned char display, unsigned char Sat);
void drvif_color_chroma_coring(unsigned char display, unsigned char domain, unsigned char Level);
void drvif_color_profile_init(unsigned char display);
void drvif_color_UV_Delay_Enable(VIP_UV_DELAY_ENABLE on_off);
void drvif_color_Set_UV_Delay(DRV_VipUV_Delay *ptr);
void drvif_color_Set_UV_Delay_TOP(DRV_VipUV_Delay_TOP_Table *ptr);
void drvif_color_Get_UV_Delay(DRV_VipUV_Delay *ptr);
void drvif_color_Gamma_Curve_Write(unsigned int *Gamma_Encode, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_Gamma_Curve_Write_enhance(unsigned int *pArray_d0d1,unsigned int *pArray_index, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_dnewdcti_table(DRV_VipNewDDcti_Table *ptr);
void drvif_color_inewdcti_table(DRV_VipNewIDcti_Table *ptr);

/* 2013.07.07 Diane add for Gamma Log*/
void drvif_Gamma_Encode_Log_Setting(unsigned char value);
void drvif_Gamma_Decode_Log_Setting(unsigned char value);
void drvif_Gamma_Calculated_Log_Setting(unsigned char value);
void drvif_Gamma_Final_Table_Log_Setting(unsigned char value);
void drvif_Output_Gamma_Encode_Log_Setting(unsigned char value);
void drvif_Output_Gamma_Decode_Log_Setting(unsigned char value);
void drvif_Output_Gamma_Calculated_Log_Setting(unsigned char value);
void drvif_Output_Gamma_Final_Table_Log_Setting(unsigned char value);
unsigned char drvif_Gamma_Encode_Log_Getting(void);
unsigned char drvif_Gamma_Decode_Log_Getting(void);
unsigned char drvif_Gamma_Calculated_Log_Getting(void);
unsigned char drvif_Gamma_Final_Table_Log_Getting(void);
unsigned char drvif_Output_Gamma_Encode_Log_Getting(void);
unsigned char drvif_Output_Gamma_Decode_Log_Getting(void);
unsigned char drvif_Output_Gamma_Calculated_Log_Getting(void);
unsigned char drvif_Output_Gamma_Final_Table_Log_Getting(void);
unsigned char drvif_color_Get_Color_UV_Delay(void);
unsigned char drvif_color_Get_Color_UV_Delay_Enable(void);

/*for xvYvv*/
unsigned char drvif_color_get_inv_gamma_Enable(unsigned char display);
void drvif_color_set_inv_gamma_Enable(unsigned char display, unsigned char Enable);
void drvif_color_inv_gamma_control_front(unsigned char display);
void drvif_color_inv_gamma_control_back(unsigned char display, unsigned char ucWriteEnable);
void drvif_color_Inv_Gamma_Curve_Write(unsigned int *Gamma_Encode, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorRead_invGamma(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_xvYcc_colorspaceyuv2rgbtransfer(unsigned char display, unsigned char xvYcc_en_flag);
void fwif_color_set_TSB_rgbbrightness(unsigned char display, unsigned short Red, unsigned short Green, unsigned short Blue);
void drvif_color_bypass(Color_Fun_Bypass *ptr, unsigned char ipNum);
void drvif_color_set_WB_pattern_setup(void);
void drvif_color_set_WB_pattern_off(unsigned char bForceBG, unsigned char b422Format);
void drvif_color_set_WB_pattern_RGB(unsigned char R, unsigned char G, unsigned char B);
unsigned char drvif_color_get_WB_pattern_on(void);

void drvif_color_setrgbcolortemp_contrast(unsigned short Red, unsigned short Green, unsigned short Blue);
void drvif_color_setrgbcolortemp_bright(unsigned short Red, unsigned short Green, unsigned short Blue);
void drvif_color_setrgbcolortemp_en(unsigned char enable);
void drvif_color_setrgbcolortemp_IOG_contrast(unsigned short Red, unsigned short Green, unsigned short Blue);
void drvif_color_setrgbcolortemp_IOG_bright(unsigned short Red, unsigned short Green, unsigned short Blue);
void drvif_color_setrgbcolortemp_IOG_en(unsigned char enable);
void drvif_color_set_new_uvc(DRV_NEW_UVC *ptr);
void drvif_color_set_de_contour(DRV_De_CT_1D *ptr);
void drvif_color_set_Blue_Stretch(DRV_Gamma_BS *ptr);

void drvif_color_set_Color_Mapping_out_offset(unsigned char display, int offset_R, int offset_G, int offset_B);
void drvif_color_set_Color_Mapping(unsigned char display, DRV_Color_Mapping *ptr);
void drvif_color_set_Color_Mapping_Enable(unsigned char display, unsigned char enable);
void drvif_color_inv_gamma_init(void);
char drvif_color_D_3dLUT(unsigned char LUT_Ctrl, unsigned int *pArray);//flora@20150617
unsigned char drvif_color_D_3dLUT_By_DMA(unsigned char LUT_Ctrl, unsigned int *pArray, unsigned int addr, unsigned int* vir_addr);
short drvif_DD_DMA_Err_Check(unsigned char showMSG);
void drvif_DD_Err_Reset(void);
char drvif_DD_Wait_for_DMA_Apply_Done(void);
void drvif_color_Read_D_3dLUT(unsigned char *LUT_Ctrl, unsigned int *pArray);
void drvif_color_set_Vivid_Color_Enable(unsigned char ucEnable);
void drvif_color_disable_VIP(void);

void drvif_color_set_color_temp(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con,
	short Red_Bri, short Green_Bri, short Blue_Bri);
bool drvif_color_inv_gamma_enable(unsigned char display, unsigned char enable, unsigned char useRPC);
void drvif_color_gamma_enable(unsigned char display, unsigned char enable);
void drvif_DM_B05_Set(unsigned short *lutMap);
void drvif_DM_HDR_3dLUT(unsigned char enable, unsigned int *pArray);/*flora@20150617, modify by juwen*/
char drvif_Set_DM_HDR_3dLUT_17x17x17_16_By_DMA(unsigned char enable, unsigned short *pArray, unsigned int addr, unsigned int* vir_addr);
char drvif_DM_HDR_3dLUT_max_min_inv_get(unsigned char *enable, unsigned int *pMax, unsigned int *pMin, unsigned int *pInv);
#ifdef CONFIG_I2RND_B05_ENABLE
void drvif_DM_HDR_3dLUT_16_i2run(unsigned char enable, unsigned short *pArray);
#endif
char drvif_DM_HDR_3dLUT_16(unsigned char enable, unsigned short *pArray);/*flora@20150617, modify by juwen*/

unsigned int drvif_Get_90k_Lo_clk(void);
char drvif_Wait_VO_start(short delay0, short delay1, unsigned int timeout);
char drvif_Get_VO_Active_Info(unsigned short *v_sta, unsigned short *v_end, unsigned short *line_cnt);
char drvif_Wait_VGIP_start(short delay0, short delay1, unsigned short timeout);
char drvif_Get_VGIP_Active_Info(unsigned short *v_sta, unsigned short *v_len, unsigned short *line_cnt);
char drvif_Wait_UZUDTG_start(short delay_st, short delay_end);
void drvif_Read_DM_HDR_3dLUT(unsigned char *enable, unsigned int *pArray);
void drvif_DM_set_HDR_3dLUT_byoffset(void);
void drvif_HDR_YUV2RGB(unsigned char enable, unsigned int *pArray);
char drvif_HDR_RGB2OPT_Get(unsigned char *enable, unsigned int *coef);
void drvif_HDR_RGB2OPT(unsigned char enable, unsigned int *pArray);
void drvif_HDMI_HDR_I3DDMA_RGB2YUV422(unsigned char enable, unsigned short *pArray, unsigned char enable_444_to_422);
void drvif_HDMI_HDR_I3DDMA_RGB2YUV_only_Matrix(unsigned short *pArray);
#ifdef CONFIG_HW_SUPPORT_HDR_3DLUT_24
char drvif_Set_DM_HDR_3dLUT_24x24x24_16(unsigned char enable, unsigned short *pArray);
char drvif_Set_DM_HDR_3dLUT_24x24x24_16_By_DMA(unsigned char enable, unsigned short *pArray, unsigned int addr, unsigned int* vir_addr);
char drvif_Get_DM_HDR_3dLUT_24x24x24_16(unsigned short *pArray);
#endif
void drvif_Set_DM_HDR_CLK(void);

void drvif_color_disable_VIP(void);

unsigned char drvif_color_set_BBC_Enable(unsigned char enable);
void drvif_color_set_BBC_shift_bit( unsigned char shiftBit );
unsigned char drvif_color_set_HDR10_Enable(unsigned char* enable);
#ifdef CONFIG_HDR_SDR_SEAMLESS
unsigned char drvif_color_set_HDR10_Enable_Seamless(unsigned char* enable);
unsigned char drvif_color_set_HDR10_Enable_Seamless_hdmi_position(unsigned char* enable);
unsigned char drvif_color_set_HDR10_Enable_Seamless_hdmi_finish(unsigned char* enable);
#endif
void drvif_HDR_EOTF_OETF(unsigned char Mode);

unsigned char drvif_HDR_get_VO_Decompress_Status(unsigned char ch);
char drvif_color_set_PQ_ByPass(unsigned char bypass_item);
char drvif_color_set_PQ_Module(unsigned char bypass_item,unsigned char bEnable);
unsigned int drvif_color_get_PQ_Module(unsigned char bypass_item);
void drvif_color_set_video_fw_irq(void);

char WaitDOLBY_DB_Apply_Clean(void);
char WaitDM_DB_Apply_Clean(void);

void drvif_color_D_3dLUT_Enable(unsigned char LUT_Ctrl);

void drvif_color_out_gamma_control_front(void);
void drvif_color_out_gamma_control_back(void);
void drvif_color_out_gamma_control_enable(unsigned char enable);
void drvif_color_colorwrite_outGamma(unsigned int *pArray);
void drvif_color_colorwrite_outGamma_enhance(unsigned int *pArray, unsigned int *pArray_index);
void drvif_color_gamma_control_front_enhance(unsigned char display);
void drvif_color_gamma_control_back_enhance(unsigned char display, unsigned char ucWriteEnable);
void drvif_color_colorwrite_gamma_enhance(unsigned int *pArray_d0d1,unsigned int *pArray_index);
unsigned char drvif_color_get_gamma_enable_enhance(unsigned char display);
void drvif_color_set_gamma_enable_enhance(unsigned char display, unsigned char enable);
void drvif_color_colorread_gamma_enhance(unsigned int *pArray, unsigned int *pArray_index, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_Out_Gamma_Curve_Write_Enhance(unsigned int *pArray, unsigned int *pArray_index, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_Out_Gamma_Curve_Write(unsigned int *Gamma_Encode, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorRead_outGamma(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorRead_outGamma_enhance(unsigned int *pArray, unsigned int *pArray_index, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
unsigned char drvif_color_get_output_gamma_enable(void);
unsigned char drvif_color_get_output_gamma_loc(void);
short drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(void);
void drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Reset(void);
char drvif_DM2_Wait_for_DMA_Apply_Done(void);
char drvif_DM2_EOTF_Set(unsigned int *EOTF_table32_R, unsigned int *EOTF_table32_G, unsigned int *EOTF_table32_B);
char drvif_DM2_EOTF_Set_By_DMA(unsigned int *EOTF_table32_R, unsigned int *EOTF_table32_G, unsigned int *EOTF_table32_B, unsigned int addr, unsigned int* vir_addr);
void drvif_DM2_EOTF_Get(unsigned int *EOTF_table32_R, unsigned int *EOTF_table32_G, unsigned int *EOTF_table32_B);
void drvif_DM2_EOTF_Enable(unsigned char En, unsigned char sel);
char drvif_DM2_OETF_Set(unsigned short *OETF_table16_R, unsigned short *OETF_table16_G, unsigned short *OETF_table16_B);
void drvif_DM2_OETF_Set_Prog_Index( VIP_HDR_OETF_Prog_Idx_TBL* OETF_prog_idx_tbl );
char drvif_DM2_OETF_Set_Prog_Index_th(unsigned int *OETF_Prog_Idx_th_TBL);
char drvif_DM2_OETF_Set_By_DMA(unsigned short *OETF_table16_R, unsigned short *OETF_table16_G, unsigned short *OETF_table16_B, unsigned int addr, unsigned int* vir_addr);
void drvif_DM2_OETF_Get(unsigned short *OETF_table16_R, unsigned short *OETF_table16_G, unsigned short *OETF_table16_B);
void drvif_DM2_OETF_Enable(unsigned char En, unsigned char sel);
void drvif_DM2_B0203_noSram_Enable(unsigned char En);
void drvif_DM2_OETF_EOTF_Path_Sel(unsigned char sel);
char drvif_Get_DM2_DB_Status(unsigned char *db_en, unsigned char *db_lv, unsigned char *db_apply);
char drvif_Get_DM2_OETF_Enable_Status(unsigned char *oetf_lut_enable, unsigned char *b0105_or_oetf, unsigned char *b02b03_no_sram_mode);
char drvif_Get_DM2_EOTF_Enable_Status(unsigned char *eotf_lut_enable, unsigned char *b0103_or_eotf, unsigned char *b02b03_no_sram_mode);
char drvif_DM2_ToneMapping_Set(short *TM_table12_R, short *TM_table12_G, short *TM_table12_B);
char drvif_DM2_ToneMapping_Get(short *TM_table12_R, short *TM_table12_G, short *TM_table12_B);
char drvif_DM2_ToneMapping_Enable(unsigned char En, unsigned char sel);
void drvif_DM2_GAMMA_Enable(unsigned char En);
void drvif_DM2_GAMMA_out_shift_Enable(unsigned char En);
unsigned char drvif_DM2_HDR_histogram_Get(unsigned int *histogram_table24);
void drvif_DM2_HDR_RGB_max_Get(unsigned int *RGB_max);
void drvif_DM2_HDR_histogram_Auto_Mode_Init(unsigned char autoMode_en);
unsigned char drvif_DM2_HDR_histogram_IRQ_Status_Get_ISR(void);
void drvif_Set_dm_sel_hdr_hist_position_ISR(unsigned char mode);
unsigned char drvif_DM2_HDR_histogram_Mode_Get_ISR(void);
char drvif_DM2_HDR_histogram_Get_ISR(unsigned int *histogram_table24);
unsigned char drvif_DM2_HDR2_histogram_Get(unsigned int *histogram_table24);
char drvif_DM2_Set_HDR2_histogram_region_Enable(unsigned char En);
char drvif_DM2_Set_HDR2_histogram_region_XY(unsigned short x_sta, unsigned short x_end, unsigned short y_sta, unsigned short y_end);
void drvif_color_set_Mplus_ModeChange(DRV_Mplus_ModeChange *ptr);
void drvif_color_SetFrameGainLimit(unsigned short nFrameGainLimit);
void drvif_color_GetFrameGainLimit (unsigned short *nFrameGainLimit);
void drvif_color_SetPixelGainLimit(unsigned short nPixelGainLimit);
void drvif_color_GetPixelGainLimit(unsigned short *nPixelGainLimit);

void drvif_DM_HDR_Vtop_set(unsigned char display, unsigned char dolbyMode, unsigned char en_422to444_1, unsigned char en_TC_422to444_1);
void drvif_color_set_RGBW_Curcuit(VIP_RGBW_MODE mode);
unsigned char drvif_DM_HDR10_enable_Get(void);
unsigned char drvif_DM_HDR10_porch_check(void);
unsigned int drvif_DM_HDR10_I_V_sta_check(void);
void drvif_color_get_YUV2RGB_nonliner_index(unsigned short *pYidx);
void drvif_color_set_dma_memory(VIP_DMA_Memory_Addr mem_addr);
void drvif_color_set_HDR10_runmode(void);
void drvif_color_set_HDR_RGB_swap(unsigned char swap);
#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
void drvif_color_set_mixer_osd3x3(unsigned int *array);
void drvif_color_set_mixer_osd3x3_Enable(unsigned char Enable);
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
char drvif_color_Get_main_den_h_v_start_end(unsigned short *H_STA, unsigned short *H_END, unsigned short *V_STA, unsigned short *V_END);
char drvif_color_set_DeMura(DRV_DeMura_DMA_TBL* pDMA_TBL, DRV_DeMura_CTRL_TBL *ptr, unsigned char *pArray, unsigned int addr, unsigned int *vir_addr, unsigned int TBL_Line_size);
char drvif_color_Get_DeMura_CTRL(DRV_DeMura_CTRL_TBL *ptr);
void drvif_color_set_DeMura_En(unsigned char enable);
unsigned char drvif_color_get_DeMura_En(void);

char drvif_color_get_Peaking_Data_Access(unsigned int *index, unsigned int *YUV);
char drvif_color_get_UZD_Data_Access(unsigned int *index, unsigned int *YUV);
char drvif_color_get_UZU_Data_Access(unsigned int *index, unsigned int *YUV);
char drvif_color_get_ICM_Data_Access(unsigned int *index, unsigned int *YUV);
char drvif_color_get_YUV2RGB_Data_Access(unsigned int *index, unsigned int *YUV);
char drvif_color_get_Panel_Compensation_Data_Access(unsigned int *index, unsigned int *YUV);
void drvif_color_Stop_VGIP_isr(unsigned char bStop);
void  IoReg_D_domain_SetBits(unsigned int addr, unsigned int Mask);
void  IoReg_D_domain_ClearBits(unsigned int addr, unsigned int Mask);
void  IoReg_D_domain_VC_Set_MainSub_Modes(unsigned int addr, unsigned int mask, VC_MS_MODE mode);
unsigned char drvif_color_wait_timeout_check_by_counter(unsigned int base_time, unsigned int timeout);
unsigned int drvif_color_report_cost_time_by_counter(unsigned int base_time);
unsigned int drvif_color_get_cur_counter(void);

//TV030
void drvif_color_set_Partten4AutoGamma(unsigned char  Enable, unsigned short  r_Val,unsigned short  g_Val,unsigned short  b_Val);
void drvif_color_set_Partten4AutoGamma_mute(unsigned char mute_flag, unsigned char display_flag,Drvif_SCALER_DISPLAY_DATA* sdp_info);
#endif

unsigned char drvif_color_VPQ_Disable_init(unsigned char enable);
unsigned char drvif_color_Disable_HDR(unsigned char enable);
unsigned char drvif_color_VPQ_ByPass(unsigned char enable);
unsigned char drvif_color_Disable_MEMC(unsigned char enable);
void drvif_DM2_HDR_Hist_Position(unsigned char hdr_hist_position);
void drvif_color_HDR10_DM_setting(void);
void drvif_color_set_dm_in_sel(unsigned char in_sel, unsigned char inbits_sel);

void drvif_color_set_HDR_sub_en(unsigned char bEnable);
void drvif_color_HDR_sub_set422to444(unsigned char bEnable);
void drvif_color_subHDR_set_Color_Mapping(DRV_Color_Mapping *ptr);
void drvif_color_subHDR_setYUV2RGB(DRV_subHDR_YUV2RGB_CSMatrix *ptr, subHDR_CSMatrix_WriteType WriteMode);
void drvif_color_HDR_Sub_Gamma1_Enable( unsigned char en );
void drvif_color_HDR_Sub_Gamma2_Enable( unsigned char en );
void drvif_color_HDR_Sub_write_Gamma1(unsigned int *pArray , unsigned Channel);
void drvif_color_HDR_Sub_write_Gamma2(unsigned int *pArray , unsigned Channel);
void drvif_color_HDR_Sub_double_buffer_apply(void);

#ifdef BUILD_QUICK_SHOW
void drvif_color_setYUV2RGB_CSMatrix_quick_show(DRV_VIP_YUV2RGB_CSMatrix *ptr, VIP_CSMatrix_WriteType WriteMode, VIP_YUV2RGB_Y_SEG seg,
unsigned char display, VIP_Table_Select tbl);
void drvif_scalerdisplay_set_sharpness_line_mode_quick_show(unsigned char is_game_en);
#endif

#ifdef __cplusplus
}
#endif


#endif
/* 100312 Erin*/


