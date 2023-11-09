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

/*******************************************************************************
 * Header include
 ******************************************************************************/
#include <scaler/vipCommon.h>
#include <scaler/vipRPCCommon.h>
#include <scaler/scalerCommon.h>

/*******************************************************************************
 * Marco
 ******************************************************************************/

#define ENABLE_HDR_DM_22b_PATH 1

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
#define Bin_Num_Gamma_k24 				512

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

#define IRE_PATTERN_USE_3D_PTG	1
typedef enum _HDR_SUB_YUV2RGB_TABLE {
	HDR_SUB_YUV2RGB_BT709_255_to_255=0,
	HDR_SUB_YUV2RGB_BT709_Limted_235_240_to_255 ,
	HDR_SUB_YUV2RGB_BT709_235_240_to_235,
	HDR_SUB_YUV2RGB_BT2020_255_to_255,
	HDR_SUB_YUV2RGB_BT2020_Limted_235_240_to_255 ,
	HDR_SUB_YUV2RGB_BT2020_235_240_to_235,
	HDR_SUB_YUV2RGB_601_255_to_255,
	HDR_SUB_YUV2RGB_601_Limted_235_240_to_255 ,
	HDR_SUB_YUV2RGB_601_235_240_to_235,

	HDR_SUB_YUV2RGB_ByPass_255_to_255,
	HDR_SUB_YUV2RGB_ByPass_Limted_235_to_255,

	HDR_SUB_YUV2RGB_TABLE_Max,

} HDR_SUB_YUV2RGB_TABLE;


typedef enum _HDR_SUB_COLOR_MAP_TABLE{
	HDR_SUB_disable_colorMap = 0,
	HDR_SUB_bt2020_2_bt709,
	HDR_SUB_bt709_2_bt2020,
	HDR_SUB_dcip3_2_bt709  ,
	HDR_SUB_bt709_2_dcip3  ,
	HDR_SUB_dcip3_2_bt2020 ,
	HDR_SUB_bt2020_2_dcip3 ,

	HDR_SUB_TABLE_NUM,
} HDR_SUB_COLOR_MAP_TABLE;


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

	HDR_DM_CSC1_YUV709toRGB709_FtoF,
	HDR_DM_CSC1_YUV2020toRGB2020_FtoF,
	HDR_DM_CSC1_YUV709toRGB709_LtoF,
	HDR_DM_CSC1_YUV2020toRGB2020_LtoF,
	HDR_DM_CSC1_Inv2020_RGB709toRGB709_FtoF,
	HDR_DM_CSC1_Inv2020_RGB709toRGB709_LtoF,
	HDR_DM_CSC1_YUV601toRGB601_FtoF,
	HDR_DM_CSC1_YUV601toRGB601_LtoF,

	HDR_DM_CSC1_YUV2RGB_HDR10_VIVID_235_to_255,

	VIP_HDR_DM_CSC1_YUV2RGB_TABLE_Max,

} VIP_HDR_DM_CSC1_YUV2RGB_TABLE;
#if 0	// move to vipcommon
#define HDR_TABLE_NUM 5
/*#define HDR_RGB2YUV_TABLE_NUM 5*/

typedef enum {
	HDR_RGB2OPT_709_TO_2020 = 0,
	HDR_RGB2OPT_2020_TO_709,
	HDR_RGB2OPT_DCI_TO_709,
	HDR_RGB2OPT_DCI_TO_2020,
	HDR_RGB2OPT_2020_TO_DCI,
	HDR_RGB2OPT_RGB_TO_Y,
	HDR_RGB2OPT_RGB2020_TO_RGB709_F2F,
	HDR_RGB2OPT_TABLE_NUM,
} HDR_RGB2OPT_TABLE;
#endif

typedef union
{
	unsigned char IPCtrlBits;	// IP control bits
	struct{
		unsigned char reserve_bit:4;
		unsigned char ctrl_by_AP_not_finish:1;
		unsigned char i2run_run:1;	/* in i2run flow, IP set or not */
		unsigned char seamless_run:1;	/* in seamless flow, IP set or not	*/
		unsigned char control_by_PQ:1;	/* 1:control by PQ flow, 0: conctrol by AP/scaler/...	*/
	};
} HDR_IP_ctrl_bits;


#define HFC_control_by_PQ 0x01
#define HFC_seamless_run 0x02
#define HFC_i2run_run 0x04
#define HFC_ctrl_ReLoad 0x08
#define HFC_OETF_CTRL_BY_MAXCLL 250
#define HFC_EOTF_CTRL_BY_MAXCLL 250
#define HFC_RGB2OPT_OFF 255
#define HFC_EOTF_OFF 255
#define HFC_OETF_OFF 255
#define HFC_CSC3_OFF 255
#define HFC_CSC3_ReLoad 254
#define HFC_CSC3_CTRL_BY_MAXCLL 250
#define HFC_TONE_MAPPING_OFF 255
#define HFC_OETF_ReLoad 254

typedef struct{
/*0*/	HDR_IP_ctrl_bits V_TOP_ctrl;
/*1*/	unsigned int V_TOP_reg_table;
/*2*/	HDR_IP_ctrl_bits swap_ctrl;
/*3*/	unsigned char swap_value;
/*4*/	HDR_IP_ctrl_bits HDR_420_to_422_ctrl;
/*5*/	unsigned char HDR_420_to_422_value;
/*6*/	HDR_IP_ctrl_bits HDR_422_to_444_ctrl;
/*7*/	unsigned char HDR_422_to_444_value;
/*8*/	HDR_IP_ctrl_bits CSC1_ctrl;
/*9*/	unsigned char CSC1_table;
/*10*/	HDR_IP_ctrl_bits CSC3_ctrl;
/*11*/	unsigned char CSC3_table;
/*12*/	HDR_IP_ctrl_bits HDR_33_1_ctrl;
/*13*/	unsigned char HDR_33_1_SPtable;		/*define special index: HFC_RGB2OPT_OFF*/
/*14*/	HDR_IP_ctrl_bits HDR_33_2_ctrl;
/*15*/	unsigned char HDR_33_2_table;
/*16*/	HDR_IP_ctrl_bits EOTF_ctrl;
/*17*/	unsigned char EOTF_SPtable;			/*define special index: HFC_EOTF_OFF*/
/*18*/	HDR_IP_ctrl_bits OETF_ctrl;
/*19*/	unsigned char OETF_SPtable;			/*define special index: HFC_OETF_CTRL_BY_MAXCLL, HFC_OETF_OFF*/
/*20*/	HDR_IP_ctrl_bits tone_mapping_ctrl;
/*21*/	unsigned char tone_mapping_table;
/*22*/	HDR_IP_ctrl_bits I3DDMA_33_ctrl;
/*23*/	unsigned char I3DDMA_33_table;
/*24*/	HDR_IP_ctrl_bits I3DDMA_422_to_444_ctrl;
/*25*/	unsigned char I3DDMA_422_to_444_value;
/*26*/	HDR_IP_ctrl_bits I_domain_R2Y_ctrl;
/*27*/	unsigned char I_domain_R2Y_table;
/*28*/	HDR_IP_ctrl_bits I_domain_422_to_444_ctrl;
/*29*/	unsigned char I_domain_422_to_444_value;
/*30*/	HDR_IP_ctrl_bits D_domain_Y2R_ctrl;
/*31*/	unsigned char D_domain_Y2R_table;
/*32*/	HDR_IP_ctrl_bits ST2094_ctrl;
/*33*/	unsigned char ST2094_enbale;
/*34*/	HDR_IP_ctrl_bits TC_ctrl;
/*35*/	unsigned char TC_enbale;
/*36*/	HDR_IP_ctrl_bits DM2_gamma_ctrl;
/*37*/	unsigned char DM2_gamma_enable;
/*38*/	HDR_IP_ctrl_bits CSC3_INDEX_ctrl;
/*39*/	unsigned char CSC3_INDEX_table;
} HDR_flow_control;


typedef enum _HDR_flow_control_timing {
	HFC_SDR_YUV = 0,
	HFC_SDR_RGB,
	HFC_DOLBY,
	HFC_USB_HDR10_YUV444_HD,
	HFC_USB_HDR10_YUV444_UHD,
	HFC_USB_HDR10_YUV422_HD,		/*5*/
	HFC_USB_HDR10_YUV422_UHD,
	HFC_USB_HDR10_YUV420_HD,
	HFC_USB_HDR10_YUV420_UHD,
	HFC_USB_HDR10_RGB_HD,
	HFC_USB_HDR10_RGB_UHD,			/*10*/
	HFC_USB_HLG_YUV444_HD,
	HFC_USB_HLG_YUV444_UHD,
	HFC_USB_HLG_YUV422_HD,
	HFC_USB_HLG_YUV422_UHD,
	HFC_USB_HLG_YUV420_HD,			/*15*/
	HFC_USB_HLG_YUV420_UHD,
	HFC_USB_HLG_RGB_HD,
	HFC_USB_HLG_RGB_UHD,
	HFC_DTV_HDR10_YUV444_HD,
	HFC_DTV_HDR10_YUV444_UHD,		/*20*/
	HFC_DTV_HDR10_YUV422_HD,
	HFC_DTV_HDR10_YUV422_UHD,
	HFC_DTV_HDR10_YUV420_HD,
	HFC_DTV_HDR10_YUV420_UHD,
	HFC_DTV_HDR10_RGB_HD,			/*25*/
	HFC_DTV_HDR10_RGB_UHD,
	HFC_DTV_HLG_YUV444_HD,
	HFC_DTV_HLG_YUV444_UHD,
	HFC_DTV_HLG_YUV422_HD,
	HFC_DTV_HLG_YUV422_UHD,			/*30*/
	HFC_DTV_HLG_YUV420_HD,
	HFC_DTV_HLG_YUV420_UHD,
	HFC_DTV_HLG_RGB_HD,
	HFC_DTV_HLG_RGB_UHD,
	HFC_NET_HDR10_YUV444,			/*35*/
	HFC_NET_HDR10_YUV422,
	HFC_NET_HDR10_YUV420,
	HFC_NET_HDR10_RGB,
	HFC_NET_HLG_YUV444,
	HFC_NET_HLG_YUV422,				/*40*/
	HFC_NET_HLG_YUV420,
	HFC_NET_HLG_RGB,
	HFC_HDMI_HDR10_YUV444_HD,
	HFC_HDMI_HDR10_YUV444_UHD,
	HFC_HDMI_HDR10_YUV422_HD,		/*45*/
	HFC_HDMI_HDR10_YUV422_UHD,
	HFC_HDMI_HDR10_YUV420_HD,
	HFC_HDMI_HDR10_YUV420_UHD,
	HFC_HDMI_HDR10_RGB_HD,
	HFC_HDMI_HDR10_RGB_UHD,			/*50*/
	HFC_HDMI_HLG_YUV444_HD,
	HFC_HDMI_HLG_YUV444_UHD,
	HFC_HDMI_HLG_YUV422_HD,
	HFC_HDMI_HLG_YUV422_UHD,
	HFC_HDMI_HLG_YUV420_HD,			/*55*/
	HFC_HDMI_HLG_YUV420_UHD,
	HFC_HDMI_HLG_RGB_HD,
	HFC_HDMI_HLG_RGB_UHD,

	HFC_TC,
	HFC_ST2094,						/*60*/

	HFC_HDR10_YUV444_DEBUG,
	HFC_HDR10_YUV422_DEBUG,
	HFC_HDR10_YUV420_DEBUG,
	HFC_HDR10_RGB_DEBUG,
	HFC_HLG_YUV444_DEBUG,			/*65*/
	HFC_HLG_YUV422_DEBUG,
	HFC_HLG_YUV420_DEBUG,
	HFC_HLG_RGB_DEBUG,
	HFC_SDR_YUV_DEBUG,
	HFC_SDR_RGB_DEBUG,				/*70*/
	HFC_BOOT_INIT,
	HFC_SOURCE_INIT,

	HFC_SDR_MAX_RGB,
	HFC_HDR_VIVI,
	
	HFC_SDR_TIMING_NUM,

} HDR_flow_control_timing;

typedef enum _HFC_VTop_reg {
	HFC_VTOP_REG_NORMAL = 0,
	HFC_VTOP_REG_COMPRESS,
	HFC_VTOP_REG_MAX,
} HFC_VTop_reg;
#if 0 // MOVE TO VIPCOMMON
/* DM2 */
#define EOTF_size 1025
#define OETF_size 1025
#endif
#define OETF_prog_idx_tbl_num 2
#define ToneMapping_size 129
#define HDR_Histogram_size 128
#if 0 // MOVE TO VIPCOMMON
#define HDR_24x24x24_size (24*24*24)
#endif
#define HDR_3DLUT_Index_Size 17
#define HDR_DM2_B05_1D_LUT_Size 33
#define HDR_Sat_Histogram_size 32
#define HDR_Hue_Histogram_size 32
typedef enum _VIP_DM2_Hist_Mode {
	DM2_Hist_Y_Mode = 0,
	DM2_Hist_RGB_MAX_Mode,
	DM2_Hist_MODE_MAX,
} VIP_DM2_Hist_Mode;
#if 0 // MOVE TO VIPCOMMON
typedef struct _VIP_HDR10_OETF_ByLuminace_TBL {
	unsigned short panel_Luminace;

	unsigned short  OETF_100N[OETF_size];
	unsigned short  OETF_150N[OETF_size];
	unsigned short  OETF_200N[OETF_size];
	unsigned short  OETF_250B[OETF_size];
	unsigned short  OETF_300B[OETF_size];
	unsigned short  OETF_350B[OETF_size];
	unsigned short  OETF_400B[OETF_size];
	unsigned short  OETF_400N6[OETF_size];
	unsigned short  OETF_500N6[OETF_size];
	unsigned short  OETF_600N6[OETF_size];
	unsigned short  OETF_660N6[OETF_size];
	unsigned short  OETF_700N6[OETF_size];
	unsigned short  OETF_800N6[OETF_size];
	unsigned short  OETF_1000N6[OETF_size];
	unsigned short  OETF_1000Over[OETF_size];

} VIP_HDR10_OETF_ByLuminace_TBL;
#endif

typedef enum _VIP_DataAccess_Debug_Sub_En {
	DAD_I3DDMA_En = 0x1,
	DAD_HDR_1_En = 0x2,
	DAD_HDR_2_En = 0x4,
	DAD_Peaking_En = 0x8,
	DAD_UZD_En = 0x10,
	DAD_UZU_En = 0x20,
	DAD_2_Step_En = 0x40,
	DAD_SHP_En = 0x80,
	DAD_ICM_En = 0x100,
	DAD_Y2R_En = 0x200,
	DAD_OD_En = 0x400,
	DAD_PCID_En = 0x800,
	
} VIP_DataAccess_Debug_Sub_En;

typedef struct _VIP_DRV_DataAccess_Debug {
	unsigned int debug_en;
	unsigned int input_x;
	unsigned int input_y;
	unsigned int uzu_x;
	unsigned int uzu_y;
	unsigned int output_x;
	unsigned int output_y;
	unsigned int i_cross_bar_en;
	unsigned int uzd_cross_bar_en;
	unsigned int d_cross_bar_en;
	unsigned int Log_en;
	unsigned int Log_Delay;
} VIP_DRV_DataAccess_Debug;

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
	//unsigned char type_sel; merlin3 remove
	//unsigned char mode_3d; merlin4 remove
	//unsigned char test_mode; merlin4 remove
	unsigned char mode_sel;


} DRV_Mplus_ModeChange;

typedef enum _VIP_RGBW_MODE {
	VIP_RGBW_MODE_DISABLE = 0,
	VIP_RGBW_MODE_LGDMPLUS,
	VIP_RGBW_MODE_RTKRGBW,
	VIP_RGBW_MODE_SIW,
	VIP_RGBW_MODE_YYG,
	VIP_RGBW_MODE_MAX,
} VIP_RGBW_MODE;

typedef struct _yuvdata
{
	unsigned short y;
	unsigned short u;
	unsigned short v;
} yuvdata;


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
	VIP_PANEL_BIT8,
	VIP_PANEL_BIT10,
	VIP_PANEL_BIT_OTHERS,
} VIP_PANEL_BIT;

typedef enum _VIP_DITHER_IP {
	VIP_DITHER_IP_MAIN = 0,
	VIP_DITHER_IP_HDMI,
	VIP_DITHER_IP_MEMC_MUX,
	VIP_DITHER_IP_LGD,
	VIP_DITHER_IP_PANEL,
	VIP_DITHER_IP_COLORTEMP,
	VIP_DITHER_IP_HDMI_R2Y,
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

#define Bin_Num_Gamma 1024 
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
	RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255_style2,

	RGB2YUV_COEF_YCC601_0_255_TO_YCC709_0_255,

	RGB2YUV_COEF_YCC2020_16_235_TO_YCC709_16_235,
	RGB2YUV_COEF_YCC2020_0_255_TO_YCC709_16_235,

	RGB2YUV_COEF_YCC709_16_235_TO_YCC601_16_235,
	RGB2YUV_COEF_YCC709_0_255_TO_YCC601_16_235,
	RGB2YUV_COEF_YCC2020_16_235_TO_YCC601_16_235,
	RGB2YUV_COEF_YCC2020_0_255_TO_YCC601_16_235,
	RGB2YUV_COEF_RGB709_16_235_TO_YCC601_16_235,
	RGB2YUV_COEF_RGB709_0_255_TO_YCC601_16_235,
	RGB2YUV_COEF_RGB2020_16_235_TO_YCC601_16_235,
	RGB2YUV_COEF_RGB2020_0_255_TO_YCC601_16_235,

	RGB2YUV_COEF_Inv2020_RGB601toYUV601_FtoL,
	RGB2YUV_COEF_Inv2020_RGB709toYUV601_FtoL,
	RGB2YUV_COEF_Inv2020_RGB601toYUV601_FtoF,
	RGB2YUV_COEF_Inv2020_RGB709toYUV601_FtoF,

	TV002_RGB2YUV_COEF_YCC601_16_235_TO_YCC601_16_235,
	TV002_RGB2YUV_COEF_YCC601_0_255_TO_YCC601_16_235,
	TV002_RGB2YUV_COEF_YCC709_16_235_TO_YCC601_16_235,
	TV002_RGB2YUV_COEF_YCC709_0_255_TO_YCC601_16_235,
	TV002_RGB2YUV_COEF_YCC2020_16_235_TO_YCC601_16_235,
	TV002_RGB2YUV_COEF_YCC2020_0_255_TO_YCC601_16_235,
	TV002_RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235,
	TV002_RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235,
	TV002_RGB2YUV_COEF_RGB_16_235_TO_YCC2020_16_235,
	TV002_RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255,

	RGB2YUV_COEF_RGB601_0_255_TO_YCC709_16_235,
	RGB2YUV_COEF_RGB601_16_235_TO_YCC709_16_235,
	RGB2YUV_COEF_RGB2020_0_255_TO_YCC709_16_235,
	RGB2YUV_COEF_RGB2020_16_235_TO_YCC709_16_235,

	RGB2YUV_COEF_YCC601_16_235_TO_YCC709_16_235,
	RGB2YUV_COEF_YCC601_0_255_TO_YCC709_16_235,
	RGB2YUV_COEF_YCC709_0_255_TO_YCC709_16_235,
	RGB2YUV_COEF_YCC2020_0_255_TO_YCC2020_16_235,

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
#if 0 // MOVE TO VIPCOMMON
typedef enum _HDR_table_item {
	HDR_EN = 0,
	HDR_420_2_422,
	HDR_422_2_444,
	HDR_DITHER,

	HDR_ITEM_MAX,
} HDR_table_item;
#endif
typedef enum _VIP_8VERTEX_RGB {
	VIP_8VERTEX_R = 0,
	VIP_8VERTEX_G,
	VIP_8VERTEX_B,

	VIP_8VERTEX_RGB_MAX
} VIP_8VERTEX_RGB;


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
#if 0
#define VIP_PQ_ByPass_TBL_Max 10
typedef enum _VIP_PQ_ByPass_ITEMS {
	/*===============I Domain===========*/
	/*Main*/
	PQ_ByPass_Iedge_Smooth = 0,		/*0*/
	PQ_ByPass_MPEG_NR,
	PQ_ByPass_HSD_DITHER,
	PQ_ByPass_HSD_DITHER_Temporal,
	PQ_ByPass_RTNR_Y,
	PQ_ByPass_RTNR_C,				/*5*/
	PQ_ByPass_MCNR,
	PQ_ByPass_I_DCTI,
	PQ_ByPass_ImpulseNR,
	PQ_ByPass_SNR_C,
	PQ_ByPass_SNR_Y,				/*10*/
	PQ_ByPass_MosquiutoNR,
	PQ_ByPass_I_Peaking,
	PQ_ByPass_MB_Peaking,

	/*Sub*/
	PQ_ByPass_HSD_DITHER_Sub,
	PQ_ByPass_HSD_DITHER_Temporal_Sub,		/*15*/
	PQ_ByPass_I_Peaking_Sub,

	/*===============D Domain===========*/
	/*Main*/
	PQ_ByPass_USM,
	PQ_ByPass_SU_Peaking,
	PQ_ByPass_SHP_DLTI,
	PQ_ByPass_DCC,			/*20*/
	PQ_ByPass_DCTI,
	PQ_ByPass_ICM,
	PQ_ByPass_SHP,
	PQ_ByPass_Dedge_Smooth,
	PQ_ByPass_CDS,			/*25*/
	PQ_ByPass_UV_Offset,
	PQ_ByPass_V_CTI,
	PQ_ByPass_sRGB,
	PQ_ByPass_Gamma,
	PQ_ByPass_InvGamma,	/*30*/
	PQ_ByPass_ColorTemp,
	PQ_ByPass_DITHER,
	PQ_ByPass_DITHER_Temporal,

	/*Sub*/
	PQ_ByPass_DCTI_Sub,
	PQ_ByPass_ICM_Sub,		/*35*/
	PQ_ByPass_SHP_Sub,
	PQ_ByPass_sRGB_Sub,
	PQ_ByPass_Gamma_Gamma,
	PQ_ByPass_InvGamma_Sub,

	/* new */
	PQ_ByPass_I_DI_IP_Enable,	/*40*/

	VIP_PQ_ByPass_ITEMS_Max,

} VIP_PQ_ByPass_ITEMS;

typedef struct _VIP_PQ_ByPass_Struct{
	unsigned char VIP_PQ_ByPass_TBL[VIP_PQ_ByPass_ITEMS_Max];

} VIP_PQ_ByPass_Struct;
#endif
#define Bin_Num_Output_Gamma 1024
#define Bin_Num_InvOutput_Gamma 1024
#define Output_Gamma_TBL_MAX 5
#define InvOutput_Gamma_TBL_MAX 5

#define Bin_Num_Mixer_Gamma 256

typedef struct _VIP_Output_InvOutput_Gamma_CTRL_ITEM{
	unsigned char output_Gamma_En;
	unsigned char output_Location;

} VIP_Output_InvOutput_Gamma_CTRL_ITEM;

typedef struct _VIP_Output_Gamma_TBL{
	unsigned int TBL_R[Bin_Num_Output_Gamma/2];
	unsigned int TBL_G[Bin_Num_Output_Gamma/2];
	unsigned int TBL_B[Bin_Num_Output_Gamma/2];
	unsigned int TBL_W[Bin_Num_Output_Gamma/2];

} VIP_Output_Gamma_TBL;

typedef struct _VIP_InvOutput_Gamma_TBL{
	unsigned int TBL_R[Bin_Num_InvOutput_Gamma/2];
	unsigned int TBL_G[Bin_Num_InvOutput_Gamma/2];
	unsigned int TBL_B[Bin_Num_InvOutput_Gamma/2];

} VIP_InvOutput_Gamma_TBL;

typedef struct _VIP_Output_Gamma{
	VIP_Output_InvOutput_Gamma_CTRL_ITEM Output_InvOutput_Gamma_CTRL_ITEM;
	VIP_Output_Gamma_TBL Output_Gamma_TBL;
}VIP_Output_Gamma;

typedef struct _VIP_InvOutput_Gamma{
	unsigned char InvOutput_Gamma_En;
	VIP_InvOutput_Gamma_TBL InvOutput_Gamma_TBL;
}VIP_InvOutput_Gamma;

#define VIP_DeMura_241x136_H 241
#define VIP_DeMura_241x136_V 136
#define VIP_DeMura_481x271_H 481
#define VIP_DeMura_481x271_V 271

/* demura DRV*/
#define VIP_Demura_Adaptive_Scale_LV_NUM 10
#define VIP_Demura_Encode_TBL_ROW 271/*136*/
#define VIP_Demura_Encode_TBL_COL 5808/*2928*/


// Merlin6 demura update: 3 plane -> max 5 planes
// 3 plane: 18bit each unit, 128 bit = 18*7 + 2 (7 units + 2bit dummy)
// 241/7 = 34.4 -> 35, 35 * 128bit / 8 = 560 bytes, RGB -> 560 *3 = 1680 bytes
// 481/7 = 68.7 -> 69, 69 * 128bit / 8 = 1104bytes, RGB -> 1104*3 = 3312 bytes
// 4 plane: 24bit each unit, 128 bit = 24*5 + 8 (5 units + 8bit dummy)
// 241/5 = 48.2 -> 49, 49 * 128bit / 8 = 784 bytes, RGB -> 784 *3 = 2352 bytes
// 481/5 = 96.2 -> 97, 97 * 128bit / 8 = 1552bytes, RGB -> 1552*3 = 4656 bytes
// 5 plane: 30bit each unit, 128 bit = 30*4 + 8 (4 units + 8bit dummy)
// 241/4 = 60.25 -> 61, 61 * 128bit / 8 = 976 bytes, RGB -> 976 *3 = 2928 bytes
// 481/4 = 120.25 -> 121, 121 * 128bit / 8 = 1936 bytes, RGB -> 1936*3 = 5808 bytes
#define VIP_DemuraTBL_LineSize_Y_481x271_3Plane 1104
#define VIP_DemuraTBL_LineSize_Y_481x271_4Plane 1552
#define VIP_DemuraTBL_LineSize_Y_481x271_5Plane 1936
#define VIP_DemuraTBL_LineSize_Y_241x136_3Plane 560
#define VIP_DemuraTBL_LineSize_Y_241x136_4Plane 784
#define VIP_DemuraTBL_LineSize_Y_241x136_5Plane 976

#define VIP_DemuraTBL_LineSize_RGB_481x271_3Plane 3312
#define VIP_DemuraTBL_LineSize_RGB_481x271_4Plane 4656
#define VIP_DemuraTBL_LineSize_RGB_481x271_5Plane 5808
#define VIP_DemuraTBL_LineSize_RGB_241x136_3Plane 1680
#define VIP_DemuraTBL_LineSize_RGB_241x136_4Plane 2352
#define VIP_DemuraTBL_LineSize_RGB_241x136_5Plane 2928

/* merlin7 add 8-bit mode */
// 3 plane: 24bit each unit, 128 bit = 24*5 + 8 (5 units + 8bit dummy)
// Y: (241+1)*1/5 = 48.4 ->49, 49*16 = 784 bytes; RGB: (241+1)*3/5 = 145.2 -> 146, 146*16 = 2336 bytes
// Y: (481+1)*1/5 = 96.4 ->97, 97*16 = 1552 bytes; RGB: (481+1)*3/5 = 289.2 -> 290, 290*16 = 4640 bytes
// 4 plane: 32bit each unit, 128 bit = 32*4 (4 units)
// Y: (241+1)*1/4 = 60.5 ->61, 61*16 = 976 bytes; RGB: (241+1)*3/4 = 181.5 -> 182, 182*16 = 2912 bytes
// Y: (481+1)*1/4 = 120.5 ->121, 121*16 = 1936 bytes; RGB: (481+1)*3/4 = 361.5 -> 362, 362*16 = 5792 bytes
// 5 plane: 40bit each unit, 128 bit = 40*3 + 8 (3 units + 8bit dummy)
// Y: (241+1)*1/3 = 80.6 ->81, 81*16 = 1296 bytes; RGB: (241+1)*3/3 = 242, 242*16 = 3872 bytes
// Y: (481+1)*1/3 = 160.6 ->161, 161*16 = 2576 bytes; RGB: (481+1)*3/3 = 482, 482*16 = 7712 bytes
#define VIP_DemuraTBL_LineSize_8bit_Y_481x271_3Plane 1552
#define VIP_DemuraTBL_LineSize_8bit_Y_481x271_4Plane 1936
#define VIP_DemuraTBL_LineSize_8bit_Y_481x271_5Plane 2576
#define VIP_DemuraTBL_LineSize_8bit_Y_241x136_3Plane 784
#define VIP_DemuraTBL_LineSize_8bit_Y_241x136_4Plane 976
#define VIP_DemuraTBL_LineSize_8bit_Y_241x136_5Plane 1296

#define VIP_DemuraTBL_LineSize_8bit_RGB_481x271_3Plane 4640
#define VIP_DemuraTBL_LineSize_8bit_RGB_481x271_4Plane 5792
#define VIP_DemuraTBL_LineSize_8bit_RGB_481x271_5Plane 7712
#define VIP_DemuraTBL_LineSize_8bit_RGB_241x136_3Plane 2336
#define VIP_DemuraTBL_LineSize_8bit_RGB_241x136_4Plane 2912
#define VIP_DemuraTBL_LineSize_8bit_RGB_241x136_5Plane 3872

typedef enum _VIP_DeMura_TBL_Mode {
	DeMura_TBL_241x136 = 0,
	DeMura_TBL_481x271,

	VIP_DeMura_TBL_Mode_MAX,
} VIP_DeMura_TBL_Mode;

typedef enum _VIP_DeMura_Channel_Mode {
	DeMura_Ch_Y_Mode = 0,
	DeMura_Ch_RGB_Separate,

	VIP_DeMura_Channel_Mode_MAX,
} VIP_DeMura_Channel_Mode;

typedef struct  _DRV_DeMura_Adaptive_TBL {
	unsigned char adaptiveScale_En;
	unsigned char adaptiveScale[VIP_Demura_Adaptive_Scale_LV_NUM];
	unsigned char adaptiveScale_APL_th[VIP_Demura_Adaptive_Scale_LV_NUM];

} DRV_DeMura_Adaptive_TBL;

typedef struct  _DRV_DeMura_CTRL_TBL {
	// Merlin6 update: rearrange table order
	
	unsigned char demura_en;					// demura function enable, 0: disable, 1: enable
	unsigned char demura_table_level;			// # of planes, 0: 3 planes, 1: 4 planes, 2: 5 planes
	unsigned char demura_rgb_table_seperate;	// RGB separate mode, 0: RGB use same table (use R settings), 1: RGB use separate tables
	unsigned char demura_block_size; 			// demura block size, 0: 8x8, 1: 16x16
	unsigned char demura_table_mode; 			// 0: 241x136, 1: 481x271

	/* merlin7 add */
	unsigned char demura_sample_mode;		//0:edge position, 1:center position
	unsigned char demura_gain_shiftbit;			//0:shift 4bit, 1:shift 5bit, 2:shift 6bit, 3:shift 7bit
	unsigned char demura_8bitmode_en;		//0:6bit diff table mode, 1:8bit diff table mode
	unsigned char demura_block_size_v;			//DeMura V block size(only DeMura_block_size=3) 0:8, 1:16, 2:4
	unsigned char demura_block_size_h;			//DeMura H block size(only DeMura_block_size=3) 0:8, 1:16
	// table settings on each level
	// for 3 planes: use L/M/H, M2 & M3 don't care
	// for 4 planes: use L/M/M2/H, M3 don't care
	// for 5 planes: use L/M/M2/M3/H

	/* merlin7 remove */ 
	/*
	// demura table scale
	// rule: if demura_table_scale != 3, all planes use global scale
	//       else each plane use individual scale setting
	unsigned char demura_table_scale;			// global table scale, 0: 1x, 1: 2x, 2: 4x, 3: use separate scale settings
	unsigned char demura_table_scale_L;		// scale on plane L, 0: 1x, 1: 2x, 2: 4x, 3: 8x
	unsigned char demura_table_scale_M;		// scale on plane M, 0: 1x, 1: 2x, 2: 4x, 3: 8x
	unsigned char demura_table_scale_M2;		// scale on plane M2, 0: 1x, 1: 2x, 2: 4x, 3: 8x, Merlin6 add
	unsigned char demura_table_scale_M3;		// scale on plane M3, 0: 1x, 1: 2x, 2: 4x, 3: 8x, Merlin6 add
	unsigned char demura_table_scale_H;		// scale on plane H, 0: 1x, 1: 2x, 2: 4x, 3: 8x
	*/
	
	// level settings, 10 bits each (0~1023)
	unsigned short demura_r_lower_cutoff;		// Level position of lower cutoff (black limit) on R
	unsigned short demura_r_l_level;			// Level position of plane L on R
	unsigned short demura_r_m_level;			// Level position of plane M on R
	unsigned short demura_r_m2_level;		// Level position of plane M2 on R, Merlin6 add
	unsigned short demura_r_m3_level;		// Level position of plane M3 on R, Merlin6 add
	unsigned short demura_r_h_level;			// Level position of plane H on R
	unsigned short demura_r_upper_cutoff;		// Level position of upper cutoff (white limit) on R
	
	// div. factor: slope settings, calculate from levels to 16-bits each
	// div_factor_l = 2^16 / ((l_level - lower_cutoff)*4)
	// div_factor_l_m = 2^16 / ((m_level - l_level)*4)
	// for 3 planes: div_factor_m_h = 2^16 / ((h_level - m_level)*4), div_factor_m1_m2 & div_factor_m2_m3 don't care
	// for 4 planes: div_factor_m1_m2 = 2^16 / ((m2_level - m_level)*4),
	//				 div_factor_m_h = 2^16 / ((h_level - m2_level)*4), div_factor_m2_m3 don't care
	// for 5 planes: div_factor_m1_m2 = 2^16 / ((m2_level - m_level)*4),
	//				 div_factor_m2_m3 = 2^16 / ((m3_level - m2_level)*4),
	//				 div_factor_m_h = 2^16 / ((h_level - m3_level)*4)
	// div_factor_h = 2^16 / ((upper_cutoff - h_level)*4)
	unsigned short r_div_factor_l;
	unsigned short r_div_factor_l_m;
	unsigned short r_div_factor_m1_m2; // Merlin6 add
	unsigned short r_div_factor_m2_m3; // Merlin6 add
	unsigned short r_div_factor_m_h;
	unsigned short r_div_factor_h;

	// settings on G & B, same rule as R
	// only used when demura_rgb_table_seperate==1, else all channels use R settings
	unsigned short demura_g_lower_cutoff;
	unsigned short demura_g_l_level;
	unsigned short demura_g_m_level;
	unsigned short demura_g_m2_level; // Merlin6 add
	unsigned short demura_g_m3_level; // Merlin6 add
	unsigned short demura_g_h_level;
	unsigned short demura_g_upper_cutoff;
	unsigned short g_div_factor_l;
	unsigned short g_div_factor_l_m;
	unsigned short g_div_factor_m1_m2; // Merlin6 add
	unsigned short g_div_factor_m2_m3; // Merlin6 add
	unsigned short g_div_factor_m_h;
	unsigned short g_div_factor_h;

	unsigned short demura_b_lower_cutoff;
	unsigned short demura_b_l_level;
	unsigned short demura_b_m_level;
	unsigned short demura_b_m2_level; // Merlin6 add
	unsigned short demura_b_m3_level; // Merlin6 add
	unsigned short demura_b_h_level;
	unsigned short demura_b_upper_cutoff;
	unsigned short b_div_factor_l;
	unsigned short b_div_factor_l_m;
	unsigned short b_div_factor_m1_m2; // Merlin6 add
	unsigned short b_div_factor_m2_m3; // Merlin6 add
	unsigned short b_div_factor_m_h;
	unsigned short b_div_factor_h;

	/* Melrin7 add, demura table gain and offset */
	// R 
	unsigned short demura_gain_l_r;
	unsigned short demura_gain_m_r;
	unsigned short demura_gain_m2_r;
	unsigned short demura_gain_m3_r;
	unsigned short demura_gain_h_r;
	unsigned short demura_offset_l_r;
	unsigned short demura_offset_m_r;
	unsigned short demura_offset_m2_r;
	unsigned short demura_offset_m3_r;
	unsigned short demura_offset_h_r;
	// G
	unsigned short demura_gain_l_g;
	unsigned short demura_gain_m_g;
	unsigned short demura_gain_m2_g;
	unsigned short demura_gain_m3_g;
	unsigned short demura_gain_h_g;
	unsigned short demura_offset_l_g;
	unsigned short demura_offset_m_g;
	unsigned short demura_offset_m2_g;
	unsigned short demura_offset_m3_g;
	unsigned short demura_offset_h_g;
	// B
	unsigned short demura_gain_l_b;
	unsigned short demura_gain_m_b;
	unsigned short demura_gain_m2_b;
	unsigned short demura_gain_m3_b;
	unsigned short demura_gain_h_b;
	unsigned short demura_offset_l_b;
	unsigned short demura_offset_m_b;
	unsigned short demura_offset_m2_b;
	unsigned short demura_offset_m3_b;
	unsigned short demura_offset_h_b;

} DRV_DeMura_CTRL_TBL;

typedef struct  _VIP_DeMura_TBL {
	DRV_DeMura_CTRL_TBL DeMura_CTRL_TBL;
	unsigned char TBL[VIP_Demura_Encode_TBL_ROW*VIP_Demura_Encode_TBL_COL];
	unsigned char table_mode;
	DRV_DeMura_Adaptive_TBL DeMura_Adaptive_TBL;

} VIP_DeMura_TBL;

typedef struct  _DRV_DeMura_DMA_TBL {
	unsigned int burst_len;
	unsigned int line_128_num;
	unsigned int line_step;
	unsigned int table_height;
	unsigned int table_width;

} DRV_DeMura_DMA_TBL;

typedef struct  {
	unsigned short h_sta;
	unsigned short h_end;
	unsigned short v_sta;
	unsigned short v_end;
} Drvif_SCALER_DISPLAY_DATA;

typedef enum _VIP_MEX_IP_ITEM {
	MEX_RTK_DEMURA = 0,
	MEX_SIW_DEMURA,
	MEX_SRNN,
	MEX_SRS,
	MEX_HCIC,
	MEX_SLD,
	MEX_LINE_OD,
	MEX_PCLRC,
	MEX_VALC,
	MEX_POD,
	MEX_A10,
	MEX_OD,

	MEX_IP_ITEM_MAX,
} VIP_MEX_IP_ITEM;

typedef enum _VIP_MEX_MODE_ITEM {
	MEX_UNKNOW = 0,
	MEX_SlAVE_IC,
	MEX_LGD,
	MEX_LGD_120,
	MEX_LGD_OLED,
	MEX_BOE,
	MEX_BOE_120,
	MEX_CSOT,
	MEX_INX,
	MEX_S_AUO,
	MEX_HKC,
	MEX_SHARP,

	MEX_Force_En_All,
	MEX_MODE_ITEM_MAX,
} VIP_MEX_MODE_ITEM;

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

/*******************************************************************************
 * Program
 ******************************************************************************/
VIP_IP_Status_Check_List* drvif_Get_IP_Status_Check_List_TBL(unsigned char *tbl_item_num);
unsigned int drvif_color_get_reg_Val(unsigned int reg_addr, unsigned char reg_shift, unsigned int reg_mask);
char drvif_Self_Check_Reg_Get(unsigned char display, VIP_Self_Check_Reg_Get *pReg_set);

 /* ====  Dither ======*/
void drvif_color_Setditherbit(VIP_PANEL_BIT default_vip_panel_bit);  // for panel dither
void drvif_color_set_Main_Dither_Enable(unsigned char enable);
void drvif_color_set_HDMI_Dither_Enable(unsigned char enable);
void drvif_color_set_HDMI_444to422_Enable(unsigned char enable,unsigned char drop_enable,unsigned char delay,unsigned char sel);
void drvif_color_Panel_Dither_mode(unsigned char mode);
void drvif_color_set_Panel_Dither_Enable(unsigned char enable);
void drvif_color_set_LGD_Dither_Enable(unsigned char enable);
void drvif_color_set_r2y_Dither_Enable(unsigned char enable);
void drvif_color_set_CT_Dither_Enable(unsigned char enable);
void drvif_color_set_Panel_Dither_OLED_mode( unsigned short th2,unsigned short th3,unsigned short shift,unsigned short gain);
void drvif_color_set_Panel_Dither_table_shift_curve( unsigned short *yshift,unsigned short *yth);
void drvif_color_get_Panel_Dither_table_shift_curve( unsigned short *yshift,unsigned short *yth);
void drvif_color_set_MainType_Sequence(unsigned char DitherIP,unsigned char  (*pDITHER_SEQUENCE)[16]);
void drvif_color_set_PanelType_Sequence(unsigned char DitherIP,unsigned char (*pDITHER_SEQUENCE)[16]);
void drvif_color_set_MainType_Dither_Table(unsigned char DitherIP,unsigned char (*pDITHER_TABLE)[4][4]);
void drvif_color_set_PanelType_Dither_Table(unsigned char DitherIP,unsigned char  (*pDITHER_TABLE)[16][4]);
void drvif_color_set_Dither_Sequence(unsigned char DitherIP,unsigned char (*pDITHER_SEQUENCE)[16]);
void drvif_color_set_Dither_Temporal(unsigned char DitherIP,unsigned char * pDITHER_TEMPORAL);
/* ===============================*/
void drvif_color_set_BOE_RGBW(unsigned short *ptr_tab, int w_rate);

void drvif_color_set_Panel_Bit(VIP_PANEL_BIT bit);


void drvif_color_setydlti(unsigned char display, unsigned char Level);
void drvif_color_setgamma(unsigned char display, unsigned char Mode, VIP_Gamma *ptr);
void drvif_color_setycontrastbrightness_new(unsigned char display, unsigned char Contrast, unsigned char Brightness, unsigned char csp_control);
/* driver for YUV2RGB, sub channel is same as main now. fix this driver for sub channel if necessary.*/
void drvif_color_setYUV2RGB_CSMatrix(DRV_VIP_YUV2RGB_CSMatrix *ptr, VIP_CSMatrix_WriteType WriteMode, VIP_YUV2RGB_Y_SEG idx, unsigned char display, VIP_Table_Select tbl);


/*void drvif_color_handler(void);	*/ /* move to scalerColor*/
/*void drvif_color_setdataformathandler(void);	*/ /* move to scalerLib*/

void drvif_color_setrgbcontrast(unsigned char display, unsigned short Red, unsigned short Green, unsigned short Blue, unsigned char  BlendRatio, unsigned char Overlay);
void drvif_color_setrgbbrightness(unsigned char display, unsigned short Red, unsigned short Green, unsigned short Blue);
void drvif_color_gamma_control_front(unsigned char display);
void drvif_color_gamma_control_back(unsigned char display, unsigned char ucWriteEnable);

void drvif_color_colorwrite_output_gamma(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorwrite_inv_output_gamma(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorwrite_inv_output_gamma_Enable(unsigned char Enable);
void drvif_color_colorread_inv_output_gamma(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
unsigned char drvif_color_colorread_inv_output_gamma_Enable(void);
void drvif_color_colorwrite_mixer_gamma1(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorwrite_mixer_gamma1_Enable(unsigned char Enable);
void drvif_color_colorwrite_mixer_gamma2(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorwrite_mixer_gamma2_Enable(unsigned char Enable);
void drvif_color_set_output_gamma_format(unsigned char Enable, unsigned char fmt, unsigned char loc, unsigned char ch_order);
unsigned char drvif_color_Get_RGBW_Mode(void);
void drvif_color_set_RGBW(void);
void drvif_color_set_RGBW_cpc_be(unsigned char level1, unsigned char level2);
void drvif_color_set_RGBW_deoversat(unsigned char level);
void drvif_color_set_RGBW_hfreq(unsigned char level);
void drvif_color_set_RGBW_newCM_sprOffset(unsigned char level);
void drvif_color_set_RGBW_spr_coef(unsigned char mode, unsigned char ch_order);
void drvif_color_colorread_gamma_enhance(unsigned int *pArray, unsigned int *pArray_index, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorwrite_gamma_enhance(unsigned int *pArray_d0d1,unsigned int *pArray_index, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
unsigned char drvif_color_get_gamma_enable_enhance(unsigned char display);
void drvif_color_set_gamma_enable_enhance(unsigned char display, unsigned char enable);
void drvif_color_colorRead_outGamma_enhance(unsigned int *pArray, unsigned int *pArray_index, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorwrite_outGamma_enhance(unsigned int *pArray, unsigned int *pArray_index, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_gamma_control_front_enhance(unsigned char display);
void drvif_color_gamma_control_back_enhance(unsigned char display, unsigned char ucWriteEnable);
void drvif_color_gamma_enable_enhance(unsigned char display, unsigned char enable);
unsigned char drvif_color_get_output_gamma_loc(void);
unsigned char drvif_color_get_output_gamma_enable(void);

void drvif_color_TCON_outGammaTblSel(unsigned char tcon_en,unsigned char sharInvTblEn,unsigned int tblSel);
void drvif_color_VRR_outGammaTblCtrl_En(unsigned char en);
void drvif_color_VRR_outGammaTblCtrl_weightByFW(unsigned char w_sel,unsigned int w_FW);
void drvif_color_VRR_outGammaTblCtrl_lineCntByFW(unsigned char LC_sel,unsigned int LC_FW);
void drvif_color_VRR_outGammaTblCtrl_lineCntCurve(unsigned int vrrLC1,unsigned int rate);
void drvif_color_outGamma_DoubleBufferEnable(unsigned char en);
void drvif_color_outGamma_DoubleBufferApply(void);
void drvif_color_VRR_getStatus( unsigned int *lineCnt,  unsigned int *finalWeight);

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
void drvif_sRGB_SetGrid(unsigned char display, signed int *sRGB_grid);
void scalerVIP_sRGB_Write_FullfSize_rgbChange(int color_change);
void scalerVIP_sRGB_Write_HalfSize_rgbChange(int table_sel, int color_change);

void scalerVIP_sRGB_Read_FullSize(void);

void drvif_color_chroma_coring(unsigned char display, unsigned char domain, unsigned char Level);
void drvif_color_profile_init(unsigned char display);


void wait_for_sRGB_fifo_empty(void);

void drvif_color_UV_Delay_Enable(VIP_UV_DELAY_ENABLE on_off);
void drvif_color_Set_UV_Delay(DRV_VipUV_Delay *ptr);
void drvif_color_Set_UV_Delay_TOP(DRV_VipUV_Delay_TOP_Table *ptr);

void drvif_color_Get_UV_Delay(DRV_VipUV_Delay *ptr);


void drvif_color_Gamma_Curve_Write(unsigned int *Gamma_Encode, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_Gamma_Curve_Write_enhance(unsigned int *pArray_d0d1,unsigned int *pArray_index, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);

unsigned char drvif_color_Gamma_Read_Write_Check(void);

void drvif_color_dnewdcti_table(DRV_VipNewDDcti_Table *ptr);
void drvif_color_get_dnewdcti_table(DRV_VipNewDDcti_Table *ptr);

void drvif_color_inewdcti_table(DRV_VipNewIDcti_Table *ptr);
void drvif_color_get_inewdcti_table(DRV_VipNewIDcti_Table *ptr);

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
/*unsigned char drvif_color_regDitherLUT(unsigned char *table);*/



/*for xvYvv*/
unsigned char drvif_color_get_inv_gamma_Enable(unsigned char display);
void drvif_color_set_inv_gamma_Enable(unsigned char display, unsigned char Enable);
void drvif_color_inv_gamma_control_front(unsigned char display);
void drvif_color_inv_gamma_control_back(unsigned char display, unsigned char ucWriteEnable);
void drvif_color_Inv_Gamma_Curve_Write(unsigned int *Gamma_Encode, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorRead_invGamma(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_sRGB_ByPass(void);
void drvif_color_xvYcc_colorspaceyuv2rgbtransfer(unsigned char display, unsigned char xvYcc_en_flag);

void fwif_color_set_TSB_rgbbrightness(unsigned char display, unsigned short Red, unsigned short Green, unsigned short Blue);

//void drvif_color_set_Adaptive_Gamma(DRV_Adaptive_Gamma_Ctrl *ptr);

/*using drvif_color_setYUV2RGB_CSMatrix instead, elieli*/
#if 0
void drvif_color_colorspaceyuv2rgbtransfer_coef_by_y(unsigned char display, unsigned char which_table, unsigned int tYUV2RGB_COEF_BY_Y_table[][YUV2RGB_COEF_BY_Y_Seg_Num]);
void drvif_color_colorspaceyuv2rgbtransfer_uv_offset_by_y(unsigned char display, unsigned char which_table, unsigned int tYUV2RGB_UV_OFFSET_BY_Y_table[][YUV2RGB_UV_OFFSET_BY_Y_Seg_Num]);
void drvif_color_colorspaceyuv2rgbtransfer(unsigned char display, unsigned short *table_index);

void drvif_color_reg_YUV2RGB_COEF_BY_Y_Table(unsigned int *table);
void drvif_color_reg_YUV2RGB_UV_OFFSET_BY_Y_Table(unsigned int *table);
#endif

//int drvif_color_get_rgb_pixel_info(RGB_pixel_info *data);
void drvif_color_bypass(Color_Fun_Bypass *ptr, unsigned char ipNum);
char drvif_color_Set_Sharpness_PowerSaving_Mute(unsigned char Mute_En);
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

void drvif_color_set_Color_Mapping(unsigned char display ,DRV_Color_Mapping *ptr);
void drvif_color_set_Color_Mapping_en(unsigned char display, unsigned char en);
void drvif_color_inv_gamma_init(void);

char drvif_color_D_3dLUT(unsigned char LUT_Ctrl, unsigned int *pArray);//flora@20150617
char drvif_set_color_D_3dLUT_CRC_Enable(unsigned char En);
unsigned long long drvif_get_color_D_3dLUT_CRC(void);
short drvif_DD_DMA_Err_Check(unsigned char showMSG);
void drvif_DD_Err_Reset(void);
char drvif_DD_Wait_for_DMA_Apply_Done(void);

void drvif_color_D_3dLUT_By_DMA(unsigned char LUT_Ctrl, unsigned int *pArray, unsigned int addr, unsigned int* vir_addr);
void drvif_color_Read_D_3dLUT(unsigned char *LUT_Ctrl, unsigned int *pArray);

void drvif_color_set_Vivid_Color_Enable(unsigned char ucEnable);
void drvif_color_set_sub_Vivid_Color_Enable(unsigned char ucEnable);

void drvif_color_disable_VIP(void);

void drvif_color_set_color_temp(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con,
	short Red_Bri, short Green_Bri, short Blue_Bri);
void drvif_color_inv_gamma_enable(unsigned char display, unsigned char enable, unsigned char useRPC);
void drvif_color_gamma_enable(unsigned char display, unsigned char enable, unsigned char useRPC);
void drvif_DM_B05_Set(unsigned short *lutMap);
void drvif_DM_HDR_3dLUT(unsigned char enable, unsigned int *pArray);/*flora@20150617, modify by juwen*/
#ifdef CONFIG_I2RND_B05_ENABLE
void drvif_DM_HDR_3dLUT_16_i2run(unsigned char enable, unsigned short *pArray);
#endif
char drvif_DM_HDR_3dLUT_16(unsigned char enable, unsigned short *pArray);/*flora@20150617, modify by juwen*/
unsigned char drvif_GET_VGIP_Main_2Pixel(void);
unsigned int drvif_Get_90k_Lo_clk(void);
char drvif_Wait_VO_start(short delay0, short delay1, unsigned int timeout);
char drvif_Get_VO_Active_Info(unsigned short *v_sta, unsigned short *v_end, unsigned short *line_cnt);
char drvif_Wait_VGIP_start(short delay0, short delay1, unsigned int timeout);
char drvif_Get_VGIP_Active_Info(unsigned short *v_sta, unsigned short *v_len, unsigned short *line_cnt);
char drvif_Wait_UZUDTG_start(short delay_st, short delay_end);
char drvif_Wait_UZUDTG_END(short delay_st, short delay_end);
char drvif_Get_UZUDTG_Info(unsigned short *v_sta, unsigned short *v_end, unsigned short *line_cnt);
void drvif_Read_DM_HDR_3dLUT(unsigned char *enable, unsigned int *pArray);
char drvif_HDR_RGB2OPT_Get(unsigned char *enable, unsigned int *coef);
void drvif_HDR_YUV2RGB(unsigned char enable, unsigned int *pArray);
void drvif_HDR_RGB2OPT(unsigned char enable, unsigned int *pArray);
void drvif_HDMI_HDR_I3DDMA_RGB2YUV422(unsigned char enable, unsigned short *pArray, unsigned char enable_444_to_422);
void drvif_HDMI_HDR_I3DDMA_RGB2YUV_only_Matrix(unsigned short *pArray);
char drvif_Set_DM_HDR_3dLUT_24x24x24_16(unsigned char enable, unsigned short *pArray);
char drvif_Set_DM_HDR_3dLUT_24x24x24_16_By_DMA(unsigned char enable, unsigned short *pArray, unsigned int addr, unsigned int* vir_addr, unsigned int flush_size);
char drvif_Set_DM_HDR_3dLUT_17x17x17_16_By_DMA(unsigned char enable, unsigned short *pArray, unsigned int addr, unsigned int* vir_addr, unsigned int flush_size);
char drvif_DM_HDR_3dLUT_max_min_inv_get(unsigned char *enable, unsigned int *pMax, unsigned int *pMin, unsigned int *pInv);
char drvif_Set_DM_HDR_3dLUT_Dimension(unsigned char dimension);
char drvif_Get_DM_HDR_3dLUT_24x24x24_16(unsigned short *pArray);
char drvif_Set_DM_HDR_3dLUT_Index(unsigned int* r_position, unsigned int* g_position, unsigned int* b_position);
char drvif_Get_DM_HDR_3dLUT_Index(unsigned int* r_position, unsigned int* g_position, unsigned int* b_position);
char drvif_set_color_DM_HDR_3dLUT_CRC_Enable(unsigned char En);
unsigned long long drvif_get_color_DM_HDR_3dLUT_CRC(void);
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

char drvif_M_Plus_Status_Check(void);

char drvif_color_set_PQ_ByPass(unsigned char bypass_item, unsigned char isInterlaceMode);
//char drvif_color_set_PQ_ByPass_Resort(unsigned char Resort_item);
void drvif_color_set_PQ_ByPass_2P_mode_misc(void);
void drvif_color_set_video_fw_irq(void);


char WaitDOLBY_DB_Apply_Clean(void);
char WaitDM_DB_Apply_Clean(void);
//char drvif_color_SET_DM_DB_En(unsigned char en);

void drvif_color_HDR10_DM_setting(void);
void drvif_color_D_3dLUT_Enable(unsigned char LUT_Ctrl);
void drvif_color_get_rgb2yuvtransfer(unsigned char channel, unsigned short *table_index);

void drvif_color_out_gamma_control_front(void);
void drvif_color_out_gamma_control_back(void);
void drvif_color_out_gamma_control_front_enhance(void);
void drvif_color_out_gamma_control_back_enhance(void);

void drvif_color_out_gamma_control_enable(unsigned char enable);
void drvif_color_out_gamma_Control_Enhance(unsigned char enable, unsigned char loc);
void drvif_color_colorwrite_outGamma(unsigned int *pArray);
void drvif_color_Out_Gamma_Curve_Write(unsigned int *Gamma_Encode, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void drvif_color_colorRead_outGamma(unsigned int *pArray, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
short drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(unsigned char showMSG);
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
void drvif_DM2_B0203_noSram_Enable(unsigned char En, unsigned char noDB_flag);
void drvif_DM2_OETF_EOTF_Path_Sel(unsigned char sel);
unsigned char drvif_check_VO_CLK(unsigned char apiID);
void drvif_DM2_set_B05_1D_LUT_Enable(unsigned char En);
unsigned char drvif_DM2_get_B05_1D_LUT_Enable(void);
char drvif_DM2_set_B05_1D_LUT_Index(unsigned int *pLut_Idx);
char drvif_DM2_get_B05_1D_LUT_Index(unsigned int *pLut_Idx);
char drvif_DM2_set_B05_1D_LUT(unsigned int *pLut);
char drvif_DM2_get_B05_1D_LUT(unsigned int *pLut);
void drvif_DM2_dtm_Enable(unsigned char enable,unsigned char dlut_en);
void drvif_DM2_dtm_set_weight(unsigned short *h_weight,unsigned short *s_weight);
char drvif_Get_DM2_DB_Status(unsigned char *db_en, unsigned char *db_lv, unsigned char *db_apply);
char drvif_Get_DM2_OETF_Enable_Status(unsigned char *oetf_lut_enable, unsigned char *b0105_or_oetf, unsigned char *b02b03_no_sram_mode);
char drvif_Get_DM2_EOTF_Enable_Status(unsigned char *eotf_lut_enable, unsigned char *b0103_or_eotf, unsigned char *b02b03_no_sram_mode);
char drvif_Get_DM2_B05_Enable_Status(unsigned char *b05_En, unsigned char *dimension);
unsigned char drvif_Get_DM2_B05_Dimension_clk_flip_flop(void);
unsigned char drvif_Get_DM2_B0203_noSram_clk_flip_flop(void);

char drvif_DM2_ToneMapping_Set(short *TM_table12_R, short *TM_table12_G, short *TM_table12_B);
char drvif_DM2_ToneMapping_Set_By_DMA(short *TM12_R, short *TM12_G, short *TM12_B, unsigned int addr, unsigned int* vir_addr);
char drvif_DM2_ToneMapping_Get(short *TM_table12_R, short *TM_table12_G, short *TM_table12_B);
char drvif_DM2_ToneMapping_Enable(unsigned char En, unsigned char sel);
char drvif_DM2_ToneMapping_Get_Enable(void);
void drvif_DM2_GAMMA_Enable(unsigned char En);
void drvif_DM2_b0501_enable_Enable(unsigned char En);
void drvif_DM2_GAMMA_out_shift_Enable(unsigned char En);

char drvif_DM2_Set_HDR_histogram_region_Enable(unsigned char En);
char drvif_DM2_Set_HDR_histogram_region_XY(unsigned short x_sta, unsigned short x_end, unsigned short y_sta, unsigned short y_end);
unsigned char drvif_DM2_HDR2_histogram_Get(unsigned int *histogram_table24);
char drvif_DM2_Set_HDR2_histogram_region_Enable(unsigned char En);
char drvif_DM2_Set_HDR2_histogram_region_XY(unsigned short x_sta, unsigned short x_end, unsigned short y_sta, unsigned short y_end);
void drvif_DM2_HDR_histogram_Auto_Mode_Init(unsigned char autoMode_en, unsigned char hist_mode);
unsigned char drvif_DM2_HDR_histogram_IRQ_Status_Get_ISR(void);
void drvif_DM2_HDR_histogram_IRQ_Status_Set_ISR(unsigned char hist_irq_staus);
void drvif_Set_dm_hdr_path_sel_reg_ISR(unsigned int dm_hdr_path_sel_reg);
unsigned int drvif_Get_dm_hdr_path_sel_reg_ISR(void);
unsigned char drvif_DM2_HDR_histogram_Mode_Get_ISR(void);
char drvif_DM2_HDR_histogram_Get_ISR(unsigned int *histogram_table24);
void drvif_DM2_HDR_sat_hue_histo_init(void);
char drvif_DM2_HDR_sat_histo_get_ISR(unsigned int *pSatHist);
char drvif_DM2_HDR_Hue_histo_get_ISR(unsigned int *pHueHist);
unsigned char drvif_DM2_HDR_histogram_Get(unsigned int *histogram_table24);
void drvif_DM2_HDR_RGB_max_Get(unsigned int *RGB_max);
void drvif_color_MplusSet(unsigned short *pRegisterSet, unsigned short regSize, unsigned char regStep);
void drvif_color_MplusGet(unsigned short *pRegisterSet, unsigned short regSize, unsigned char regStep);
void drvif_color_BOE_MplusSet(unsigned short *pRegisterSet, unsigned short regSize, unsigned char regStep);
void drvif_color_set_SiW_ModeChange(unsigned char  mode);
void drvif_color_set_Mplus_ModeChange(DRV_Mplus_ModeChange *ptr);
void drvif_color_set_HLW(unsigned char en, unsigned char mode,
	unsigned short h_sta, unsigned short v_sta, unsigned short h_wid, unsigned short v_len);
void drvif_color_SetFrameGainLimit(unsigned short nFrameGainLimit);
void drvif_color_GetFrameGainLimit (unsigned short *nFrameGainLimit);
void drvif_color_SetPixelGainLimit(unsigned short nPixelGainLimit);
void drvif_color_GetPixelGainLimit(unsigned short *nPixelGainLimit);

void drvif_I_Main_DITHER_Y_HISTOGRAM_init(void);
unsigned long long drvif_I_Main_DITHER_Y_HISTOGRAM_Get_ISR(unsigned int *pHist_32, unsigned short *pMAX, unsigned short *pMIN);

void drvif_DM_HDR_Vtop_set(unsigned char dolbyMode, unsigned char en_422to444_1, unsigned char en_TC_422to444_1);
void drvif_color_set_RGBW_Curcuit(VIP_RGBW_MODE mode);
unsigned char drvif_DM_HDR10_enable_Get(void);
unsigned char drvif_DM_HDR10_porch_check(void);
unsigned int drvif_DM_HDR10_I_V_sta_check(void);
void drvif_color_get_YUV2RGB_nonliner_index(unsigned short *pYidx);

void drvif_color_set_dm_in_sel(unsigned char in_sel, unsigned char inbits_sel);
void drvif_color_set_HDR_RGB_swap(unsigned char swap);

void drvif_color_set_mixer_osd3x3(unsigned int *array);
void drvif_color_set_mixer_osd3x3_Enable(unsigned char Enable);
char drvif_color_Get_main_den_h_v_start_end(unsigned short *H_STA, unsigned short *H_END, unsigned short *V_STA, unsigned short *V_END);
char drvif_color_set_DeMura(DRV_DeMura_DMA_TBL* pDMA_TBL, DRV_DeMura_CTRL_TBL *ptr, unsigned char *pArray, unsigned int addr, unsigned int *vir_addr, unsigned int TBL_Line_size, unsigned char unpdateTBL_only);
char drvif_color_Get_DeMura_CTRL(DRV_DeMura_CTRL_TBL *ptr);
void drvif_color_set_DeMura_En(unsigned char enable);
unsigned char drvif_color_get_DeMura_En(void);
void drvif_color_set_SIW_DeMura_config(unsigned char en, unsigned char* config, unsigned char size);
void drvif_color_set_SIW_DeMura_120_config(unsigned char en, unsigned char* config, unsigned char size);//Merlin8 add
char drvif_color_Get_DTG_S1_Timing(unsigned short *h_sta, unsigned short *v_sta, unsigned short *h_end, unsigned short *v_end);

unsigned char drvif_color_VPQ_Disable_init(unsigned char enable);
unsigned char drvif_color_Disable_HDR(unsigned char enable);
unsigned char drvif_color_VPQ_ByPass(unsigned char enable);
unsigned char drvif_color_Disable_MEMC(unsigned char enable);
void drvif_color_Stop_VGIP_isr(unsigned char bStop);
unsigned char drvif_color_set_PIP_overlap_area_color_selection(unsigned char sel);
char drvif_color_set_HW_MEX_MODE(unsigned char MEX_MODE);
char drvif_color_get_HW_MEX_STATUS(VIP_MEX_IP_ITEM MEX_IP);
void drvif_color_set_HDR_sub_en(unsigned char bEnable);
void drvif_color_HDR_sub_set422to444(unsigned char bEnable);
void drvif_color_subHDR_set_Color_Mapping(DRV_Color_Mapping *ptr);
void drvif_color_subHDR_setYUV2RGB(DRV_subHDR_YUV2RGB_CSMatrix *ptr, subHDR_CSMatrix_WriteType WriteMode);
void drvif_color_HDR_Sub_Gamma1_Enable( unsigned char en );
void drvif_color_HDR_Sub_Gamma2_Enable( unsigned char en );
void drvif_color_HDR_Sub_write_Gamma1(unsigned int *pArray , unsigned Channel);
void drvif_color_HDR_Sub_write_Gamma2(unsigned int *pArray , unsigned Channel);
void drvif_color_HDR_Sub_double_buffer_apply(void);
unsigned char drvif_color_get_HDMI_dither_table_index(void);
void drvif_color_VPQ_check(void);
void drvif_color_data_access_debug_check(VIP_DRV_DataAccess_Debug data);
void drvif_color_data_access_debug_Set(VIP_DRV_DataAccess_Debug data);
void set_inx_panel_para(void);


void drvif_color_set_Partten4AutoGamma(unsigned char  Enable, unsigned short  r_Val,unsigned short  g_Val,unsigned short  b_Val);
void drvif_color_set_Color_Mapping_Enable(unsigned char display, unsigned char enable);
void drvif_DM2_HDR_Hist_Position(unsigned char hdr_hist_position);
void drvif_color_set_Partten4AutoGamma_mute(unsigned char mute_flag, unsigned char display_flag,Drvif_SCALER_DISPLAY_DATA* sdp_info);
int drvif_color_get_access_data_point(ACCESS_DATA_PT_T *pPT);
void drvif_color_set_INX_panel_para(void);
unsigned char drvif_color_wait_timeout_check_by_counter(unsigned int base_time, unsigned int timeout);
unsigned int drvif_color_report_cost_time_by_counter(unsigned int base_time);
unsigned int drvif_color_get_cur_counter(void);

#ifdef __cplusplus
}
#endif


#endif
/* 100312 Erin*/


