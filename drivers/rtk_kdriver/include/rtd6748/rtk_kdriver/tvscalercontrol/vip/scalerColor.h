#ifndef __SCALER_COLOR_H__
#define __SCALER_COLOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_VPQ_TABLE_GENERATE
#define CONFIG_VPQ_TABLE_GENERATE 0
#endif

#if !CONFIG_VPQ_TABLE_GENERATE
#include <tvscalercontrol/vip/icm.h>
#include <tvscalercontrol/scaler/source.h>
#include <tvscalercontrol/scaler/vipinclude.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/localdimming.h>
#include <tvscalercontrol/vip/localdimmingInterface.h>
#include <tvscalercontrol/vip/localcontrast.h>
#include <scaler_vbedev.h>
#include <scaler/vipRPCCommon.h>
/*#include <tvscalercontrol/vip/scalerColor_tv006.h>*/
#endif

#define _MAIN_DISPLAY								SLR_MAIN_DISPLAY
#define _SUB_DISPLAY								SLR_SUB_DISPLAY


#ifdef CONFIG_CUSTOMER_TV002
#define CONFIG_TV002_DE_SHARPNESS 1
#endif

#define GSR_move_GSR_to_LD_Compesation
#define OSD_Contrast_Compensation 1
#define complement2(arg) (((~arg)&0x7FF) + 1)
#define VIP_YUV2RGB_K_Y_NEG_BND 0x3FFF
#define VIP_YUV2RGB_K_C_NEG_BND 0x3FF
#define VIP_YUV2RGB_OFFSET_NEG_BND 0x3FFF
#define VIP_YUV2RGB_K_Y_NEG_CHANGE(arg) (((~arg)&(VIP_YUV2RGB_K_Y_NEG_BND*2+1)) + 1)
#define VIP_YUV2RGB_K_C_NEG_CHANGE(arg) (((~arg)&(VIP_YUV2RGB_K_C_NEG_BND*2+1)) + 1)
#define VIP_YUV2RGB_OFFSET_NEG_CHANGE(arg) (arg - ((VIP_YUV2RGB_OFFSET_NEG_BND+1)*2))

#define Seamless_Freeze_Frames_Max 15

/* define for DM_HDR_3D_LUT*/
#define VIP_DM_HDR_3D_LUT_SIZE 4913 /* 4913 = 17*17*17 */
#define VIP_DM_HDR_3D_LUT_DRV_TBL_SIZE 9826 /* 17*17*17*2 */
#define VIP_DM_HDR_3D_LUT_UI_TBL_SIZE 14739 /* 17*17*17*3 */

/* define for D_3D_LUT*/
#define VIP_D_3D_LUT_INDEX 17
#define VIP_D_3D_LUT_SIZE 4913 /* 4913 = 17*17*17 */
#define VIP_D_3D_LUT_DRV_TBL_SIZE 9826 /* 17*17*17*2 */
#define VIP_D_3D_LUT_UI_TBL_SIZE 14739 /* 17*17*17*3 */
#define VIP_D_3DLUT_999_SIZE 729 /* 9*9*9 */
#define VIP_D_3DLUT_NUM 3 /* 0:DEFAULT ; 1:CUSTOM ; 2:EXTEND*/

/* define DMA BUFFER*/ // total = 800k
#define VIP_DMAto3DTable_HDR_3D_LUT_SIZE (100 * 1024)	/* 200k*/
#define VIP_DMAto3DTable_HDR_EOTF_SIZE (40 * 1024)
#define VIP_DMAto3DTable_HDR_OETF_SIZE (20 * 1024)
#define VIP_DMAto3DTable_D_3D_LUT_SIZE (100 * 1024)
#define VIP_DMAto3DTable_ICM_SIZE (100 * 1024)
// SIP_1 = TC_LUT and SIP_2 = HDR_TM
#define VIP_DMAto3DTable_TC_LUT_and_SIP1_SIZE (100 * 1024)	//  SIP1 : table0_bitwidth * table0_num_x = 0x6000 * 0x20 = 786432, "coef1.tbl" = 0x8190000~0x81A7FFC
/*#define VIP_DMAto3DTable_TC_C_LUT_SIZE (20 * 1024)
#define VIP_DMAto3DTable_TC_I_LUT_SIZE (20 * 1024)
#define VIP_DMAto3DTable_TC_S_LUT_SIZE (20 * 1024)
#define VIP_DMAto3DTable_TC_P_LUT_SIZE (20 * 1024)
#define VIP_DMAto3DTable_TC_D_LUT_SIZE (20 * 1024)*/
#define VIP_DMAto3DTable_HDR_TM_and_SIP2_SIZE (40 * 1024) // SIP2  : 227328bit = 28416 byte
//#define VIP_DMAto3DTable_SIP_1_BUFFER_SIZE  (100 * 1024)
//#define VIP_DMAto3DTable_SIP_2_BUFFER_SIZE  (40 * 1024)
#define VIP_DMAto3DTABLE_SLD_APL_DMA_SIZE (40 * 1024)//29232*2
#define VIP_DMAto3DTABLE_SRNN_MODE2_SIZE (20 * 1024)//7380*2  
#define VIP_DMAto3DTABLE_SRNN_MODE3_SIZE (40 * 1024)//14364*2
#define VIP_DMAto3DTABLE_SRNN_MODE4_SIZE (40 * 1024)//14364*2
#define VIP_DMAto3DTABLE_SRNN_MODE5_SIZE (40 * 1024)//14729*2
#define VIP_DMAto3DTABLE_SRNN_MODE6_SIZE (60 * 1024)//28356*2
#define VIP_DMAto3DTABLE_SRNN_MODE7_SIZE (60 * 1024)//29232*2

#if 1
/*========================= for INNX demura =================================*/
//#define INNX_Demura_addr_DemuraFunctionDefinition 6
#define INNX_Demura_LUT_address_find 0x11
#define INNX_Demura_addr_Identification	0x000000
#define INNX_Demura_addr_ASIC	0x000001
#define INNX_Demura_addr_Version	0x000006
#define INNX_Demura_addr_DeMuraAlgorithm	0x000007
#define INNX_Demura_addr_Date	0x000008
#define INNX_Demura_addr_ChipStructure	0x00000C
#define INNX_Demura_addr_DemuraFunctionDefinition	0x00000D
#define INNX_Demura_addr_BlockStructure	0x00000E
#define INNX_Demura_addr_HeaderExtensionBlock	0x00000F
#define INNX_Demura_addr_Type	0x000010
#define INNX_Demura_addr_DataChecksum	0x000012
#define INNX_Demura_addr_StartAddress	0x000014
#define INNX_Demura_addr_DataLength	0x000018
// #define INNX_Demura_addr_Type	0x00001C
// #define INNX_Demura_addr_DataChecksum	0x00001E
// #define INNX_Demura_addr_StartAddress	0x000020
// #define INNX_Demura_addr_DataLength	0x000024
// #define INNX_Demura_addr_Type	0x000028
// #define INNX_Demura_addr_DataChecksum	0x00002A
// #define INNX_Demura_addr_StartAddress	0x00002C
// #define INNX_Demura_addr_DataLength	0x000030
// #define INNX_Demura_addr_Type	0x000034
// #define INNX_Demura_addr_DataChecksum	0x000036
// #define INNX_Demura_addr_StartAddress	0x000038
// #define INNX_Demura_addr_DataLength	0x00003C
// #define INNX_Demura_addr_Type	0x000040
// #define INNX_Demura_addr_DataChecksum	0x000042
// #define INNX_Demura_addr_StartAddress	0x000044
// #define INNX_Demura_addr_DataLength	0x000048
// #define INNX_Demura_addr_Type	0x00004C
// #define INNX_Demura_addr_DataChecksum	0x00004E
// #define INNX_Demura_addr_StartAddress	0x000050
// #define INNX_Demura_addr_DataLength	0x000054
// #define INNX_Demura_addr_Type	0x000058
// #define INNX_Demura_addr_DataChecksum	0x00005A
// #define INNX_Demura_addr_StartAddress	0x00005C
// #define INNX_Demura_addr_DataLength	0x000060
// #define INNX_Demura_addr_Type	0x000064
// #define INNX_Demura_addr_DataChecksum	0x000066
// #define INNX_Demura_addr_StartAddress	0x000068
// #define INNX_Demura_addr_DataLength	0x00006C
#define INNX_Demura_addr_BotLimit	0x000070
#define INNX_Demura_addr_TopLimit	0x000071
#define INNX_Demura_addr_Gray1	0x000072
#define INNX_Demura_addr_Gray2	0x000073
#define INNX_Demura_addr_Gray3	0x000074
#define INNX_Demura_addr_Gray4	0x000075
#define INNX_Demura_addr_Gray5	0x000076
#define INNX_Demura_addr_HeaderInfoBlock	0x000077
#define INNX_Demura_addr_LUTCheckSum	0x00007c
#define INNX_Demura_addr_HeaderCheckSum	0x00007e

#define INNX_Demura_TBL_Size 0x80000
#define INNX_Demura_W16 496	/* change to 481byte 1 line */
#define INNX_Demura_W 481
#define INNX_Demura_num_level 5
#define INNX_Demura_num_22 22
#define INNX_Demura_num_31 31
#define INNX_Demura_num_110 110
#define INNX_Demura_rowByte 1760
#define INNX_Demura_h_271 271
#define INNX_Demura_h_136 136
#define INNX_Demura_size_header_infor 1024
#define INNX_Demura_DecodeLUT_Size (INNX_Demura_h_271 * INNX_Demura_W16 * 5)
#define INNX_Demura_DecodeLUT_Low_sta_addr (INNX_Demura_h_271 * INNX_Demura_W16 * 0)
#define INNX_Demura_DecodeLUT_Mid_sta_addr (INNX_Demura_h_271 * INNX_Demura_W16 * 1)
#define INNX_Demura_DecodeLUT_Mid2_sta_addr (INNX_Demura_h_271 * INNX_Demura_W16 * 2)
#define INNX_Demura_DecodeLUT_Mid3_sta_addr (INNX_Demura_h_271 * INNX_Demura_W16 * 3)
#define INNX_Demura_DecodeLUT_High_sta_addr (INNX_Demura_h_271 * INNX_Demura_W16 * 4)
/*========================= for INNX demura =================================*/
#endif

#if CONFIG_VPQ_TABLE_GENERATE
typedef char bool;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
#endif

typedef enum _VPQ_ModuleTest_Item {
	Disable_init = 0,
	Disable_HDR,
	Disable_VPQ_ByPass,
	Disable_MEMC,
	Disable_MAX,
} VPQ_ModuleTest_Item;

// SIP_1 = TC_LUT and SIP_2 = HDR_TM
typedef enum _VIP_DMAto3DTABLE_INDEX {
	DMAto3DTABLE_HDR_3D_LUT = 0,
	DMAto3DTABLE_HDR_EOTF,
	DMAto3DTABLE_HDR_OETF,
	DMAto3DTABLE_HDR_D_3D_LUT,
	DMAto3DTABLE_ICM,
	DMAto3DTABLE_TC_LUT,
	DMAto3DTABLE_SIP_1 = DMAto3DTABLE_TC_LUT,
	/*DMAto3DTABLE_TC_C_LUT,
	DMAto3DTABLE_TC_I_LUT,
	DMAto3DTABLE_TC_S_LUT,
	DMAto3DTABLE_TC_P_LUT,
	DMAto3DTABLE_TC_D_LUT,*/
	DMAto3DTABLE_HDR_TM,
	DMAto3DTABLE_SIP_2 = DMAto3DTABLE_HDR_TM,
	DMAto3DTABLE_SLD_APL_DMA,
	DMAto3DTABLE_SRNN_MODE2,
	DMAto3DTABLE_SRNN_MODE3,
	DMAto3DTABLE_SRNN_MODE4,
	DMAto3DTABLE_SRNN_MODE5,
	DMAto3DTABLE_SRNN_MODE6,
	DMAto3DTABLE_SRNN_MODE7,

	VIP_DMAto3DTABLE_INDEX_MAX

} VIP_DMAto3DTABLE_INDEX;


typedef enum {
	APP_VIDEO_OSD_ITEM_BACKLIGHT,
	APP_VIDEO_OSD_ITEM_BRIGHTNESS,
	APP_VIDEO_OSD_ITEM_CONTRAST,
	APP_VIDEO_OSD_ITEM_SATURTUION,
	APP_VIDEO_OSD_ITEM_SHARPNESS,
	APP_VIDEO_OSD_ITEM_HUE,
	APP_VIDEO_OSD_ITEM_COLORTEMP,
	APP_VIDEO_OSD_ITEM_NR,
	APP_VIDEO_OSD_ITEM_MAX,
} APP_Video_OsdItemType_t;

typedef enum _ICM_select_range_axis {
	h_sta = 0,
	s_sta,
	i_sta,
	h_end,
	s_end,
	i_end,
} ICM_select_range_axis;

typedef enum _VIP_HDR10_CSC3_TABLE_Mode{
	OLD_17x17x17 = 17,
	NEW_24x24x24 = 24,

} VIP_HDR10_CSC3_TABLE_Mode;

typedef struct {
	unsigned char newRTNR_style;
	unsigned char Reserved_01;
	unsigned char BTR;
	unsigned char Reserved_04;
	unsigned char Reserved_05;
} Scaler_DI_Coef_t;

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
} VIP_table_crc_value;

typedef enum _VIP_YUV2RGB_TBL_SELECT_ITEM {
	YUV2RGB_TBL_SELECT_601_Limted_NO_Gain = 0,
	YUV2RGB_TBL_SELECT_601_Full_NO_Gain,
	YUV2RGB_TBL_SELECT_709_Limted_NO_Gain,
	YUV2RGB_TBL_SELECT_709_Full_NO_Gain,
	YUV2RGB_TBL_SELECT_2020_NonConstantY,
	YUV2RGB_TBL_SELECT_2020_ConstantY,
	YUV2RGB_TBL_SELECT_601_Limted_235_240_to_255,
	YUV2RGB_TBL_SELECT_709_Limted_235_240_to_255,

	YUV2RGB_INPUT_TBL_UNKNOW,

	YUV2RGB_TBL_SELECT_ITEM_MAX,

} VIP_YUV2RGB_TBL_SELECT_ITEM;

#ifdef CONFIG_HDR_SDR_SEAMLESS
typedef enum _HDR_SDR_SEAMLESS_PQ_ITEM {
	HDR_SDR_SEAMLESS_PQ_HAL_PQModeInfo = 0,
	HDR_SDR_SEAMLESS_PQ_HAL_BLACK_LEVEL,
	HDR_SDR_SEAMLESS_PQ_RGB2YUV_Matrix_bypass_main,
	HDR_SDR_SEAMLESS_PQ_RGB2YUV_Matrix_bypass_sub,
	HDR_SDR_SEAMLESS_PQ_MAX,
}HDR_SDR_SEAMLESS_PQ_ITEM;

typedef struct _HDR_SDR_SEAMLESS_PQ_STRUCT{
	unsigned int HDR_SDR_SEAMLESS_PQ_SET[HDR_SDR_SEAMLESS_PQ_MAX];
	unsigned short *HDR_SDR_SEAMLESS_PQ_RGB2YUV_Matrix_index;
}HDR_SDR_SEAMLESS_PQ_STRUCT;

#endif

typedef enum _LC_BLOCK_NUM_ITEM {
	LC_BLOCK_16x16 = 0,
	LC_BLOCK_32x16,
	LC_BLOCK_48x27,
	LC_BLOCK_60x34,/*merlin5, add*/
}LC_BLOCK_NUM_ITEM;

typedef struct _FILM_FW_ShareMem
{
	unsigned char version;
	unsigned char flag_Trigger;
	unsigned char st_DI_STATUS_VPAN;
	unsigned char smooth_toggle_film;
	unsigned char film_en_ap_status;
	unsigned char reserved5;
	unsigned char reserved6;
	unsigned char reserved7;
} FILM_FW_ShareMem;

typedef struct{
       unsigned char c3dLUT_16_enable;
       unsigned short *a3dLUT_16_pArray;
} PQ_HDR_3dLUT_16_PARAM;

typedef enum _De_Jaggy_LEVEL {
	De_Jaggy_OFF = 0,
	De_Jaggy_LOW,
	De_Jaggy_MID,
	De_Jaggy_HIGH,
	De_Jaggy_LEVEL_MAX,

}De_Jaggy_LEVEL;

typedef struct _De_Jaggy_TBL {
	unsigned char IESM_En;
	unsigned char IESM_3tap_gain;
	unsigned char IESM_5tap_gain;

} De_Jaggy_TBL;

typedef enum _COLORSPACE_ITEM
{
	isBT2020=0,
	is601,
	is709,
	isRGB,
    isDCTIP3,
    isBT601_525,
    isBT601_625,
	COLORSPACE_MAX,
}COLORSPACE_ITEM;

typedef enum
{
	COLOR_MATRIX_TYPE_BT709=0,
	COLOR_MATRIX_TYPE_BT2020,
	COLOR_MATRIX_TYPE_DCTIP3,
	COLORSPACE_VIDEO_BT601_525, ///< BT601_525 SMPET-C
	COLORSPACE_VIDEO_BT601_625,///< BT601_625 EBU	
	COLOR_MATRIX_TYPE_ADOBE_RGB,
	COLOR_MATRIX_TYPE_MAX,
}COLOR_MATRIX_TYPE;

typedef enum _GAMMA_LEVEL {
	GAMMA_CURVE_0 = 0,
	GAMMA_CURVE_1,
	GAMMA_CURVE_2,
	GAMMA_CURVE_3,
	GAMMA_CURVE_4,
	GAMMA_CURVE_5,
	GAMMA_CURVE_6,
	GAMMA_CURVE_7,
	GAMMA_CURVE_8,
	GAMMA_CURVE_9,
	GAMMA_CURVE_identity,
	GAMMA_CURVE_RELATE_TO_GAMMA_MODE,
	GAMMA_CURVE_MAX_NUM,
} GAMMA_LEVEL;

#define RADCR_BL_LEVEL_NUM 101
typedef struct _RADCR_BL_MAP_ST {
	unsigned char bl_mapping[RADCR_BL_LEVEL_NUM];
} RADCR_BL_MAP_ST;

typedef struct _RADCR_TBL_ST {
	short RADCR_LD_Hist_th; // LD_Hist_permillage, 1~1000
	short RADCR_LD_Hist_BL_adj; // Backlight_Offset, -100~100
	short RADCR_LD_Hist_APL_th; // Minimum_APL, 0~100
	short RADCR_IIR_rate; //IIR Smooth, 0~64, 64 = pre, 0 = cur
	short RADCR_Reserved4;
	short RADCR_Reserved5;
	short RADCR_Reserved6;
	short RADCR_Reserved7;
	short RADCR_Reserved8;
	short RADCR_Reserved9;
	short RADCR_Reserved10;
	short RADCR_APL_th[11]; // APL, 0~100
	short RADCR_BL_adj[11]; // Backlight_Offset_percentage, -100~100
	short RADCR_BL_th[11]; // Backlight_mapping, 0~100
	short RADCR_Gain_adj[11]; // ColorTemp_Gain, 0~1023
	short RADCR_Offset_adj[11]; // ColorTemp_Offset, -512~511
} RADCR_TBL_ST;

typedef struct _RADCR_STATUS_ST {
	short status[5][11];
} RADCR_STATUS_ST;

typedef struct _RADCR_RTICE_ST {
	RADCR_TBL_ST table;
	RADCR_STATUS_ST status;
} RADCR_RTICE_ST;

/* VPQ_SetSaturationLUT */
#define VPQ_SaturationLUT_Seg_Max VIP_YUV2RGB_Y_Seg_Max
#define VPQ_SaturationLUT_Gain_ShiftBit 7

typedef struct {
	unsigned short uSaturationY[VPQ_SaturationLUT_Seg_Max];
	unsigned short uSaturationX[16];
} VPQ_SAT_LUT_T;

typedef struct _VIP_OPS_TPC_Item {
	unsigned short TPC_enable;
	unsigned short TPC_wait_time;
	unsigned short TPC_drop_time;
	unsigned short TPC_ascend_time;
	unsigned short TPC_gain_limit;

	unsigned short TPC_hist_diff_count_th;
	unsigned short TPC_GMV_th;
	unsigned short TPC_MEMC_th;
	unsigned short TPC_hist_high_th;
	unsigned short TPC_high_ratio_th;

	unsigned short TPC_sat_hist_high_th;
	unsigned short TPC_sat_high_ratio_th;
	unsigned short TPC_Y_Mean_th;
	unsigned short TPC_Y_Mean_high_cnt_th;
	unsigned short TPC_Y_Mean_high_ratio_th;

	unsigned short TPC_Y_Mean_hist_diff_count_th;
	unsigned short TPC_Y_Mean_GMV_th;
	unsigned short TPC_Y_Mean_MEMC_th;
	unsigned short TPC_counter_ro;
	unsigned short TPC_gain_ro;

	unsigned short still_cond_hist;
	unsigned short still_cond_gmv;
	unsigned short still_cond_memc;
	unsigned short peak_cond;
	unsigned short ID_cond;

	unsigned short GMV_ratio;
	unsigned short hist_diff_ratio;
	unsigned short hist_high_cnt_ratio;
	unsigned short hist_sat_high_cnt_ratio;
	unsigned short  memc_mvx_abs;

	unsigned short memc_mvy_abs;
	unsigned short TPC_Y_Mean;
	unsigned short TPC_Y_Mean_high_ratio;

} VIP_OPS_TPC_Item;

typedef struct _VIP_OPS_LBC_Item {
	unsigned short LBC_enable;
	unsigned short LBC_wait_time;
	unsigned short LBC_drop_time;
	unsigned short LBC_ascend_time;
	unsigned short LBC_temp_score_th;

	unsigned short LBC_block_cnt_th;
	unsigned short LBC_total_gain_limit;
	unsigned short LBC_ld_gain_limit;
	unsigned short LBC_ld_2d_cmp_tbl_gain;
	unsigned short LBC_block_apl_th_h;

	unsigned short LBC_block_apl_th_m2;
	unsigned short LBC_block_apl_th_m1;
	unsigned short LBC_block_apl_th_l;
	unsigned short LBC_counter_ro;
	unsigned short LBC_gain_ro;

	unsigned short LD_gain;
	unsigned short LBC_block_cnt;
	unsigned short LBC_block_cnt_target;
} VIP_OPS_LBC_Item;

typedef struct _VIP_OPS_CPC_Item {
	unsigned short CPC_enable;
	unsigned short CPC_wait_time;
	unsigned short CPC_drop_time;
	unsigned short CPC_ascend_time;
	unsigned short CPC_drop_min;

	unsigned short CPC_drop_max;
	unsigned short CPC_Ymean_hi_th;
	unsigned short CPC_hist_high_th;
	unsigned short CPC_high_ratio_th;
	unsigned short CPC_sat_hist_high_th;

	unsigned short CPC_sat_high_ratio_th;
	unsigned short CPC_counter_ro;
	unsigned short CPC_gain_ro;
	unsigned short peak_cond;
	unsigned short Y_mean_cond;

	unsigned short Y_mean;
	unsigned short hist_high_cnt_ratio;
	unsigned short hist_sat_high_cnt_ratio;
	unsigned short natural_score;

} VIP_OPS_CPC_Item;

typedef struct _VIP_OPS_Debug_Item {
	unsigned short Debug_Delay;
	unsigned short Debug_Log_TPC_En;
	unsigned short Debug_Log_LBC_En;
	unsigned short Debug_Log_CPC_En;
	unsigned short Debug_HWSW_SLD_En;
	unsigned short Debug_MEX_En;
	
} VIP_OPS_Debug_Item;

typedef struct _VIP_OPS_CTRL_Item {
	unsigned short OPS_enable;
	VIP_OPS_TPC_Item OPS_TPC_Item;
	VIP_OPS_LBC_Item OPS_LBC_Item;
	VIP_OPS_CPC_Item OPS_CPC_Item;

	unsigned short PLC_curve_TPC[8];
	unsigned short PLC_curve_LBC[8];
	unsigned short PLC_curve_ori[8];
	unsigned short PLC_curve_read[8];

	VIP_OPS_Debug_Item OPS_Debug_Item;

	// for fw use
	unsigned char OPS_TPC_PLC_Apply;
	unsigned char OPS_LBC_PLC_Apply;
	unsigned char OPS_LBC_LD_Apply;
	unsigned char OPS_CPC_Demura_Apply;

} VIP_OPS_CTRL_Item;

typedef struct _VD_COLOR_ST {
	unsigned char Contrast;
	unsigned char Brightness;
	unsigned char Saturation;
	unsigned char Hue;
} VD_COLOR_ST;

typedef struct _VIP_Output_Gamma_CTRL_ITEM{
	unsigned int output_Gamma_En;
	unsigned int output_Location;
	unsigned int output_format;
	unsigned int reserved1;
} VIP_Output_Gamma_CTRL_ITEM;


//DLG & VRR
typedef struct _DLG_OD_TABLE_TYPE{
	unsigned char dlg_od_en;
	SLR_VIP_TABLE_OD dlg_od_normal_table;
	SLR_VIP_TABLE_OD dlg_od_dlg_table;
}DLG_OD_TABLE_TYPE;

typedef struct _ACC_TABLE_TYPE{
	unsigned int TBL_R[Bin_Num_Output_Gamma/2];
	unsigned int TBL_G[Bin_Num_Output_Gamma/2];
	unsigned int TBL_B[Bin_Num_Output_Gamma/2];
}ACC_TABLE_TYPE;

typedef struct _DLG_ACC_TABLE_TYPE{
	unsigned char dlg_acc_en;
	ACC_TABLE_TYPE dlg_acc_normal_table;
	ACC_TABLE_TYPE dlg_acc_dlg_table;
}DLG_ACC_TABLE_TYPE;

#define V_R_R_OD_TABLE_NUM 3
typedef struct _V_R_R_OD_TABLE_TYPE{
	unsigned char table_num;
	unsigned short framerate_div[V_R_R_OD_TABLE_NUM];
	SLR_VIP_TABLE_OD v_r_r_od_table[V_R_R_OD_TABLE_NUM];
}V_R_R_OD_TABLE_TYPE;

#define V_R_R_ACC_TABLE_NUM	10
typedef struct _V_R_R_ACC_TABLE_TYPE{
	unsigned char table_num;
	unsigned short framerate_div[V_R_R_ACC_TABLE_NUM];
	ACC_TABLE_TYPE v_acc_table[V_R_R_ACC_TABLE_NUM];
}V_R_R_ACC_TABLE_TYPE;

typedef struct _DRV_LDM_CUSTOME_INFO{
	unsigned char u8LD_Type;
	unsigned char u8LD_blk_Hnum;
	unsigned char u8LD_blk_Vnum;
	unsigned char u8Data_Mode;
	unsigned char u8SpiChannelNum;
	unsigned char u8LD_Comp_en;
	unsigned char u8LD_Valid;
	unsigned short u16Panel_Width;
	unsigned short u16Panel_Height;
	unsigned short u16Panel_Freq;

	unsigned char u8LD_Maxgain;
	unsigned char u8LD_Avegain;
	unsigned char u8LD_Histshiftbit;
	unsigned char u8LD_Hist0Gain;
	unsigned char u8LD_Hist1Gain;
	unsigned char u8LD_Hist2Gain;
	unsigned char u8LD_Hist3Gain;
	unsigned char u8LD_Hist4Gain;
	unsigned char u8LD_Hist5Gain;
	unsigned char u8LD_Hist6Gain;
	unsigned char u8LD_Hist7Gain;

	unsigned char u8LD_blu_spien;
	unsigned char u8LD_blu_wr_num;

	unsigned char u8LD_Spatialfilter_Low[39];
	unsigned char u8LD_Spatialfilter_Mid[39];
	unsigned char u8LD_Spatialfilter_High[39];

	unsigned char u8LD_Temporal_Filter_en;
	unsigned char u8LD_Tmp_Maxdiff;
	unsigned char u8LD_Tmp_SceneChangeGain;
	unsigned char u8LD_Pos0Thd;
	unsigned char u8LD_Pos1Thd;
	unsigned char u8LD_PostTMinGain;
	unsigned char u8LD_PostTMaxGain;
	unsigned char u8LD_Neg0Thd;
	unsigned char u8LD_Neg1Thd;
	unsigned char u8LD_NegTMinGain;
	unsigned char u8LD_NegTMaxGain;

	unsigned short u16LD_BLPF_H[40];
	unsigned short u16LD_BLPF_V[40];
	unsigned char u8LD_PF_TabHsize;
	unsigned char u8LD_PF_TabVsize;

	unsigned short u16LD_SpatialremapLow[65];
	unsigned short u16LD_SpatialremapMid[65];
	unsigned short u16LD_SpatialremapHigh[65];

	unsigned char u8LD_comp_mode;
	unsigned char u8LD_SoftCom_gain;
	unsigned short u16LD_comp_Low[289];
	unsigned short u16LD_comp_Mid[289];
	unsigned short u16LD_comp_High[289];

	unsigned char u8LD_Boost_en;
	unsigned short u16LD_boostCurve_Low[65];
	unsigned short u16LD_boostCurve_Mid[65];
	unsigned short u16LD_boostCurve_High[65];

	unsigned char u8cmdlength;
	unsigned char u8MspiHead[16];
	unsigned char u8ExtDataLength;
	unsigned char u8ExtData[60];
	unsigned short DecayWei[256];
}DRV_LDM_CUSTOME_INFO;

extern DRV_NR_Item      *Manual_NR_TABLE;

DRV_RPC_AutoMA_Flag *fwif_color_GetAutoMA_Struct(void);
RPC_DCC_LAYER_Array_Struct *fwif_color_GetDCC_LAYER_Array_Struct(void);
RPC_ICM_Global_Ctrl *fwif_color_GetICM_Global_Ctrl_Struct(void);
SLR_VIP_TABLE *fwif_color_GetShare_Memory_VIP_TABLE_Struct(void);

RPC_DCC_Advance_control_table  *fwif_color_GetShare_Memory_DCC_Advance_control_Struct(void);
_clues  *fwif_color_GetShare_Memory_SmartPic_clue(void);

/*== Share Memory init function*/
void fwif_color_set_VIP_system_info_structure(void);
void fwif_color_set_VIP_RPC_system_info_structure(void);

/*== picture mode*/
void fwif_color_set_set_picture_mode(unsigned char src_idx, unsigned char value);
void fwif_color_set_set_picture_mode_VIPinit(unsigned char src_idx, unsigned char value);

/*====================*/
/*===== Set DCC ========*/

/*o-----DCC init Process-------o*/
void fwif_color_hist_init(unsigned char display, unsigned short width, unsigned short height, Histogram_BIN_MODE BinMode);
void fwif_color_D_hist_init(unsigned char display, unsigned short width, unsigned short height);
void fwif_color_set_hist_3d_en(unsigned char bEnable);
void fwif_color_set_DCC_Init(unsigned char display);

/*picture mode*/
void fwif_color_set_dcc_mode(unsigned char src_idx, unsigned char value);

/*for VIP table DCC info sync. to video fw*/
void fwif_color_set_dcc_OnOff(unsigned char src_idx, VIP_DCC_Disable_Mode Mode);
void fwif_color_set_DCC_Boundary_Check_Table(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_Level_and_Blend_Coef_Table(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_hist_adjust_table(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_AdaptCtrl_Level_Table(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_User_Curve_Table(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_database_Table(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_Picture_Mode_Weight(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCL_B(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCL_W(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_B_expand(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_W_expand(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_Blending_Step(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_Blending_DelayTime(unsigned char src_idx, unsigned char value);

/*for AP eng. menu level sync.*/
void fwif_color_set_dcc_table_select(unsigned char src_idx, unsigned char value);
/*
void fwif_color_set_dcc_level(unsigned char src_idx, unsigned char value, unsigned char SceneChange);
void fwif_color_set_DCC_Sindex(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_Slow(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_Shigh(unsigned char src_idx, unsigned char value);
*/
void fwif_color_set_DCC_SC_for_Coef_Change(unsigned char src_idx, unsigned char value);
void fwif_color_set_DCC_histogram_gain(unsigned char src_idx, unsigned char value, unsigned char bin_num);
void fwif_color_set_DCC_histogram_offset(unsigned char src_idx, unsigned char value, unsigned char bin_num);
void fwif_color_set_DCC_histogram_limit(unsigned char src_idx, unsigned char value, unsigned char bin_num);


/*Set System fw Driver*/
void fwif_color_set_dcc_Color_Independent_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_dcc_chroma_compensation_driver(unsigned char src_idx, unsigned char value);

/*Information API*/
unsigned char fwif_get_Y_Total_Hist_Cnt_distribution(unsigned char MODE, unsigned char size, unsigned int *Y_Hist_distr_cnt);


/*============================*/

/*===============================================================================================	*/ /*PictureMode CSFC 20140128*/
/*=== Set PictureMode =====*/
void fwif_color_set_set_picture_mode(unsigned char src_idx, unsigned char value);
/*===============================================================================================	*/ /*PictureMode CSFC 20140128*/
int fwif_color_setCon_Bri_Color_Tint(unsigned char src_idx, unsigned char display, int Con, int Bri, int Color, int Tint);
int fwif_color_setCon_Bri_Color_Tint_from_System_Info(unsigned char src_idx, unsigned char display);
int fwif_color_get_ConBriMapping_Offset_Value(unsigned char HAL_Src_Idx, unsigned char UI_Level, unsigned char *Contrast, unsigned char *Brightness);
int fwif_color_setConBri_FacMappingValue(unsigned char Contrast, unsigned char Brightness);
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
void fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain_AI(unsigned char src_idx, unsigned char display,  _system_setting_info *system_info_struct/*, int Con, int Bri, int Color*/);
#endif
/*===============================================================================================	*/ /*ConBri CSFC 20140128*/
/*== set con/bri ===*/
void fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(unsigned char src_idx, unsigned char display,  _system_setting_info *system_info_struct);
void fwif_color_setContrast(unsigned char src_idx, unsigned char display, unsigned char value);
void fwif_color_setBrightness(unsigned char src_idx, unsigned char display, unsigned char value);
void fwif_color_CalContrast_Gain(unsigned char OSD_Contrast, unsigned char *Contrast_Gain, unsigned char *Y_Clamp);
void fwif_color_CalContrast_Compensation(unsigned char display, unsigned char Contrast_Gain, _system_setting_info *system_info_struct);
void fwif_color_SetDataFormatHandler(SCALER_DISP_CHANNEL display, unsigned short mode, unsigned short channel, unsigned char en_422to444,
	unsigned char InputSrcGetType);

#if defined(CONFIG_TTX_TOSHIBA_PATCH)
void fwif_color_setcontrastbrightness_half(unsigned char display, BOOL sRGB_User_Sat_En, BOOL sRGB_User_Sat_Mapping);
#endif
/*===============================================================================================	*/ /*ConBri CSFC 20140128*/

/*== set hue/sat ===*/
void fwif_color_setSaturation(unsigned char src_idx, unsigned char display, unsigned char value);
void fwif_color_setHue(unsigned char src_idx, unsigned char display, unsigned char value);
/*======set hue/sat ===*/

char fwif_color_set_DSE(unsigned char display, VPQ_SAT_LUT_T *satLUT);

/*============== ICM  =====*/
void fwif_color_regIcmTable(unsigned short *table);
void fwif_color_set_ICM_table(unsigned char src_idx, unsigned char value);
void fwif_color_set_ICM_table_driver(unsigned char src_idx, unsigned char value, unsigned char use_DMA);
void fwif_color_set_ICM_Global_Adjust(int dhue_cur, int dsatbysat_cur[SATSEGMAX], int ditnbyitn_cur[ITNSEGMAX], bool bSubtractPrevious);
bool fwif_color_icm_SramBlockAccessSpeedup(unsigned int *buf, bool if_write_ic, int h_sta, int h_end, int s_sta, int s_end, int i_sta, int i_end);
void  fwif_color_icm_enhancement(unsigned char display, unsigned char which_table, unsigned char value);
void  fwif_color_ChannelColourSet(unsigned char display, unsigned char which_table , signed char level);
void fwif_color_icm_user_function(unsigned char display, unsigned char which_table, unsigned char skin_tone, unsigned char color_wheel, unsigned char detail_enhance, unsigned dynamic_tint);
void fwif_color_get_icm_table_Adjust_Block(unsigned char display, unsigned short *tICM_ini, unsigned char which_table, int *SR, unsigned int *buf);
void fwif_color_modify_icm_table_Adjust_Block(unsigned int *buf, unsigned char color, ICM_OSD_table *ICM_OSD_Table, int *SR, unsigned char *OSD);
void fwif_color_icm_OSD_color(unsigned char display, unsigned char which_table, ICM_OSD_table *ICM_OSD_Table, unsigned char *ICM_OSD_value, unsigned char color);

/*============== ICM  =====*/

/*=============================================================================*/ /*VDC ConBri CSFC 20140210*/
/*===  VDC Contrast / Brightness ================*/


void fwif_color_module_vdc_SetConBriHueSat(unsigned char src_idx, unsigned char value);
int fwif_color_module_vdc_SetConBriSatHue_byAP(VD_COLOR_ST *pVD_Color);
int fwif_color_module_vdc_GetConBriSatHue_byAP(VD_COLOR_ST *pVD_Color);

/*===  VDC Contrast / Brightness ================*/
/*=============================================================================*/ /*VDC ConBri CSFC 20140210*/

/*======== Set color temp=============*/
void fwif_color_setrgbcontrast(unsigned char display, unsigned short Red, unsigned short Green, unsigned short Blue, unsigned char  BlendRatio, unsigned char Overlay);
void fwif_color_setrgbbrightness(unsigned char display, unsigned short Red, unsigned short Green, unsigned short Blue);
void fwif_color_Gamma_Curve_Write(unsigned int *Gamma_Encode, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void fwif_color_setgamma(unsigned char display, unsigned char Mode);
void fwif_color_setrgbcontrast_Identity(UINT8 isOverlay);
void fwif_color_setrgbbrightness_Identity(void);
void fwif_color_setrgbcontrast_By_Table(unsigned short R, unsigned short G, unsigned short B, UINT8 isOverlay);
void fwif_color_setrgbbrightness_By_Table(unsigned short R, unsigned short G, unsigned short B);
void fwif_color_gamma_control_front(unsigned char display);
void fwif_color_gamma_control_back(unsigned char display, unsigned char ucWriteEnable);
void fwif_color_set_gamma_Identity(void);
void fwif_color_get_gamma_default(unsigned char Gamma_Mode, unsigned int **In_R, unsigned int **In_G, unsigned int **In_B);
void fwif_color_encode_gamma_debug(unsigned char type);
void fwif_color_gamma_decode(UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B, unsigned int *In_R, unsigned int *In_G, unsigned int *In_B);
void fwif_color_decode_gamma_debug(unsigned char type);
void fwif_color_gamma_blending_S_curve(unsigned short index, unsigned short C_gain1, unsigned short C_gain0);
void fwif_color_gamma_curve_data_protect(UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B);
void fwif_color_osd_gamma_encode(unsigned int *final_R, unsigned int *final_G, unsigned int *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B);
void fwif_color_gamma_encode(unsigned int *final_R, unsigned int *final_G, unsigned int *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B);
/*void fwif_color_set_gamma_encode(void);*/
void fwif_color_set_gamma_encode(unsigned int *final_GAMMA_R, unsigned int *final_GAMMA_G, unsigned int *final_GAMMA_B);
void fwif_color_set_ddomainISR_gamma_encode(unsigned int *final_GAMMA_R, unsigned int *final_GAMMA_G, unsigned int *final_GAMMA_B);
void fwif_color_Set_Inv_Gamma_Encode(unsigned int *final_Inv_GAMMA_R, unsigned int *final_Inv_GAMMA_G, unsigned int *final_Inv_GAMMA_B);
/*======== Set color temp end=============*/
void fwif_set_gamma(unsigned char Gamma_Mode, unsigned char Gamma_Sindex, unsigned char Gamma_Shigh, unsigned char Gamma_Slow);
void fwif_set_ddomainISR_gamma(unsigned char Gamma_Mode, unsigned char Gamma_Sindex, unsigned char Gamma_Shigh, unsigned char Gamma_Slow);
void fwif_set_software_gamma(unsigned char Gamma_Mode, unsigned char Gamma_Sindex, unsigned char Gamma_Shigh, unsigned char Gamma_Slow);
void fwif_color_gamma_enable_patch(unsigned char ucEbable);
void fwif_color_inv_gamma_enable_patch(unsigned char ucEbable);

unsigned char fwif_color_GetAutoMAFlag(void);
void fwif_color_SetAutoMAFlag(unsigned char flag);

/*===============================================================================================	*/ /*NR rock 20140127*/
/*unsigned int fwif_color_regPQATableOffsetTemp(unsigned int *ptr);	*/ /*move to share memory access struct, elieli*/
unsigned int  fwif_color_regNRTable(DRV_NR_Item *ptr);
unsigned int  fwif_color_regIEdgeSmooth_Coef(DRV_IEdgeSmooth_Coef *ptr);
unsigned int  fwif_color_regDEdgeSmooth_Coef(DRV_DEdgeSmooth_Coef *ptr);
void fwif_color_set_MPEG_init(void);
void fwif_color_RTNR_flow(DRV_NR_Item *ptr);
void fwif_color_SNR_flow(DRV_NR_Item *ptr);
void fwif_color_SetDNR(unsigned char display, unsigned char level);
void fwif_color_SetMPEGNR(unsigned char display, unsigned char level, unsigned char calledByOSD);
void fwif_color_RHAL_SetDNR(unsigned char level);
void fwif_color_RHAL_SetMPEGNR(unsigned char level);

unsigned char fwif_color_access_DeJaggy_Level(unsigned char Level, unsigned char isSetFlag);

DRV_RTNR_General_ctrl *fwif_color_GetRTNR_General(unsigned char source, unsigned char level);
void fwif_color_set_iESM(unsigned char display, unsigned char value);
void fwif_color_set_dESM(unsigned char display, unsigned char value);
/*void fwif_color_SetPQA(unsigned char input, unsigned char mode);*/
void fwif_color_set_PQA_Input_table(unsigned char value);
void fwif_color_set_PQA_table(unsigned char value);
unsigned char fwif_color_get_PQA_Input_table(void);
unsigned char fwif_color_get_PQA_table(void);
void fwif_color_PQA_Input_Item_Check(_system_setting_info *VIP_system_info_structure_table, unsigned int *PQA_TABLE, unsigned char table_select);

/*void fwif_color_set_DNR_table(unsigned char value, unsigned char calledByOSD);	*/ /*move "DNR table select info" to system_info_struct and replace by PQA table, elieli*/


void fwif_color_set_TNRXC_Ctrl(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_TNRXC_MK2(unsigned char src_idx, unsigned char TableIdx);


void fwif_color_Set_NRtemporal(unsigned char display, unsigned char value);

/* chen 0716*/
void fwif_color_set_rtnr_masnr_driver(unsigned char src_idx, unsigned char value);
/* end chen 0716*/


/*===============================================================================================	*/ /*NR rock 20140127*/

void fwif_Save_PQSource(VIP_SOURCE_TIMING src);
void fwif_Save_SWSource(unsigned char main_source, unsigned char sub_source);


/*==============================rord.tsao start===================================================*/ /**/
void fwif_color_set_shp_driver(unsigned char src_idx, unsigned char value);
void fwif_color_sharpness_osd_adjust(unsigned char src_idx, unsigned char value);

void fwif_color_set_sharpness_level(unsigned char table, unsigned char value);
void fwif_color_set_DLTI_level(unsigned char value);

void fwif_color_set_sharpness_table(unsigned char src_idx, unsigned char value);
void fwif_color_set_MBPK_table(unsigned char src_idx, unsigned char value);
void fwif_color_set_MBSU_table(unsigned char src_idx, unsigned char value);
void fwif_color_VIPShpTable_to_DRVShpTable(VIP_Sharpness_Table *VIPShpTable, DRV_Sharpness_Table *DRVShpTable);
void fwif_color_set_shp_vgain(unsigned char src_idx, unsigned char value);
void fwif_color_set_cds_table(unsigned char src_idx, unsigned char TableIdx);

void fwif_color_set_unsharp_mask_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_egsm_postshp_level_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_SuperResolution_init_table(unsigned char value);
void fwif_color_set_dismd_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_emf_mk2_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_shp_2Dpk_Seg(unsigned char src_idx, unsigned char value);
void fwif_color_set_shp_2Dpk_Tex_Detect(unsigned char src_idx, unsigned char value);
void fwif_color_set_TwoD_peaking_AdaptiveCtrl(unsigned char src_idx, unsigned char value);
void fwif_color_set_ma_BTR(unsigned char src_idx, unsigned char value);
void fwif_color_set_ma_BER(unsigned char src_idx, unsigned char value);
/*void fwif_color_set_di_ma_hmc(unsigned char src_idx, unsigned char value);*/
void fwif_set_color_MADI_HMC(unsigned char src_idx, unsigned char level);
void fwif_set_color_MADI_HME(unsigned char src_idx, unsigned char level);
void fwif_set_color_MADI_PAN(unsigned char src_idx, unsigned char level);
void fwif_color_ma_set_DI_MA_Adjust_Table(unsigned char src_idx, unsigned char level);
void fwif_color_ma_set_DI_MA_Init_Table(void);
void fwif_color_ma_init(void);

void fwif_color_set_crosscolor_driver(unsigned char src_idx, unsigned char value);
/*void fwif_color_set_SCurve_driver(unsigned char src_idx, unsigned char value);*/
void fwif_module_set_film_mode(unsigned char src_idx, unsigned char value);

void fwif_module_set_film_setting(void);
void fwif_module_set_film_table_t(unsigned char Table_Idx);


void fwif_color_set_film22_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_film32_driver(unsigned char src_idx, unsigned char value);
//void fwif_color_set_MA_Chroma_Error(unsigned char src_idx, unsigned char value);
void fwif_color_set_dlti_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_dcti(unsigned char src_idx, unsigned char value);
void fwif_color_set_Intra_Ver2_driver(unsigned char src_idx, unsigned char value);

unsigned char fwif_color_get_MA_Chroma_Error_en_from_vipTable(unsigned char src_idx, unsigned char value);

void fwif_color_set_scaleup_hcoef(unsigned char src_idx, unsigned char display, unsigned char TableIdx);
void fwif_color_set_scaleup_vcoef(unsigned char src_idx, unsigned char display, unsigned char TableIdx);
void fwif_color_set_scaleup_dircoef(unsigned char src_idx, unsigned char display, unsigned char TableIdx);

void fwif_color_set_twostep_scaleup_coef(unsigned char src_idx, unsigned char display, unsigned char TableIdx);

void fwif_color_set_scaleuph_8tap_driver(unsigned char src_idx, unsigned char display, unsigned char TableIdx);
void fwif_color_set_scaleupv_6tap_driver(unsigned char src_idx, unsigned char display, unsigned char TableIdx);

void fwif_color_set_scaleup_dir_weighting_driver(unsigned char src_idx, unsigned char value);

void fwif_color_set_scaledownh_table(unsigned char src_idx, unsigned char value);
void fwif_color_set_scaledownv_table(unsigned char src_idx, unsigned char value);
unsigned char fwif_color_get_scaledownh(unsigned char src_idx);
void fwif_color_set_scaledownh(unsigned char src_idx, unsigned char value);
/*void fwif_color_set_scaledownh_driver(unsigned char src_idx, unsigned char value);*/
unsigned char fwif_color_get_scaledownv(unsigned char src_idx);
void fwif_color_set_scaledownv(unsigned char src_idx, unsigned char value);
/*void fwif_color_set_scaledownv_driver(unsigned char src_idx, unsigned char value);*/
unsigned char fwif_color_get_scaledown444To422(unsigned char src_idx);
void fwif_color_set_scaledown444To422(unsigned char src_idx, unsigned char value);
void fwif_color_set_scaledown444To422_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_scalepk_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_supk_type_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_intra_all_pacific_new_switch(unsigned char src_idx, unsigned char value);
void fwif_color_set_inewdcti(unsigned char src_idx, unsigned char value);

void fwif_color_set_v_dcti_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_v_dcti_lpf_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_UV_Delay(unsigned char src_idx, unsigned char value);
void fwif_color_set_UV_Delay_TOP(unsigned char src_idx, unsigned char value);

unsigned char fwif_color_get_UV_Delay(void);
void fwif_color_UV_Delay_Enable(unsigned char src_idx, unsigned char value);

//void fwif_color_set_Adaptive_Gamma(unsigned char src_idx, unsigned char TableIdx);

void fwif_color_colorspacergb2yuvtransfer(unsigned char display, unsigned char nSrcType, unsigned char nSD_HD, unsigned char VO_Jpeg_Back_YUV, unsigned char useSendRPC);
#ifdef CONFIG_HDR_SDR_SEAMLESS
void fwif_color_colorspacergb2yuvtransfer_Seamless(unsigned char HDR_Mode, unsigned char nSrcType, unsigned char useSendRPC);
#endif
void fwif_color_set_DCC_YUV2RGB_CTRL(unsigned char display, unsigned char nSrcType, unsigned char nSD_HD, unsigned char VO_Jpeg_Back_YUV,  _system_setting_info *VIP_system_info_structure_table);
void fwif_color_set_ICM_YUV2RGB_CTRL(unsigned char display, unsigned char nSrcType, unsigned char nSD_HD, unsigned char VO_Jpeg_Back_YUV,  _system_setting_info *VIP_system_info_structure_table);

void fwif_color_setycontrastbrightness(unsigned char display, unsigned char Contrast, unsigned char Brightness);
/*void fwif_color_set_color_temp_level_type(unsigned char src_idx, unsigned char value);*/
unsigned char fwif_OsdMapToRegValue(APP_Video_OsdItemType_t bOsdItem, unsigned char value);

void fwif_colo_load_vip_table(SLR_VIP_TABLE *ptr);
SLR_VIP_TABLE *fwif_colo_get_AP_vip_table_gVIP_Table(void);

_clues* fwif_color_Get_SmartPic_clue(void);
unsigned char fwif_color_Send_VIP_Table(SLR_VIP_TABLE *pVIPtable);
unsigned char fwif_color_Send_RPC_VIP_Table(SLR_VIP_TABLE *pVIPtable);
void fwif_color_set_RTNR_Noise_Measure_flow(unsigned char display, unsigned char value);
void fwif_color_ChangeUINT32Endian(unsigned int  *pwTemp , int nSize, unsigned char convert_endian_flag);
void fwif_color_ChangeINT32Endian(int  *pwTemp , int nSize, unsigned char convert_endian_flag);
void fwif_color_ChangeUINT16Endian(unsigned short  *pwTemp , int nSize, unsigned char convert_endian_flag);
void fwif_color_ChangeINT16Endian(short  *pwTemp , int nSize, unsigned char convert_endian_flag);
void fwif_color_ChangeUINT32Endian_Copy(unsigned int  *pwTemp , int nSize, unsigned int  *pwCopyTemp, unsigned char convert_endian_flag);
void fwif_color_ChangeUINT32Endian_Copy(unsigned int  *pwTemp , int nSize, unsigned int  *pwCopyTemp, unsigned char convert_endian_flag);
void fwif_color_ChangeINT32Endian_Copy(int  *pwTemp , int nSize, int  *pwCopyTemp, unsigned char convert_endian_flag);
void fwif_color_ChangeUINT16Endian_Copy(unsigned short  *pwTemp , int nSize, unsigned short  *pwCopyTemp, unsigned char convert_endian_flag);
void fwif_color_ChangeINT16Endian_Copy(short  *pwTemp , int nSize, short  *pwCopyTemp, unsigned char convert_endian_flag);
unsigned int fwif_color_ChangeOneUINT32Endian(unsigned int pwTemp, unsigned char convert_endian_flag);
unsigned short fwif_color_ChangeOneUINT16Endian(unsigned short pwTemp, unsigned char convert_endian_flag);
void fwif_colo_vip_ShareMemory_init(SLR_VIP_TABLE *pVIPtable);


/*==============================rord.tsao end=====================================================*/
#if 0
void fwif_color_set_YUV2RGB_COEF_BY_Y_driver(unsigned char src_idx, unsigned char which_table);
void fwif_color_set_YUV2RGB_UV_OFFSET_BY_Y_driver(unsigned char src_idx, unsigned char which_table);
#endif
void fwif_color_set_YUV2RGB_COEF_BY_Y(unsigned char src_idx, unsigned char display, unsigned char which_table);
void fwif_color_set_YUV2RGB_UV_OFFSET_BY_Y(unsigned char src_idx, unsigned char display, unsigned char which_table);
/*initialization in quality handler, mark thiss function. elieli*/
/*void fwif_color_colorspaceyuv2rgbtransfer(unsigned char display, unsigned char nSrcType, unsigned char VO_Jpeg_Back_YUV);*/
unsigned char fwif_color_get_Decide_YUV2RGB_TBL_Index(unsigned char src_idx, unsigned char display, unsigned char Input_Data_Mode);
void fwif_color_set_YUV2RGB(unsigned char src_idx, unsigned char display, unsigned char which_table, unsigned char drvSkip_Flag_coefByY, unsigned char drvSkip_Flag_uvOffset);
/*int fwif_color_set_YUV2RGB_UV_Offset(unsigned char src_idx, unsigned char display, unsigned char level);*//*for LG DB, chane to fwif_color_set_YUV2RGB_UV_Offset_tv006, no more use this*/
void fwif_color_set_CS_Matrix_byTBL_Level(unsigned char src_idx, unsigned char display, unsigned char which_table, VIP_YUV2RGB_LEVEL_SELECT which_Lv, unsigned char drvSkip_Flag_coefByY, unsigned char drvSkip_Flag_uvOffset);
void fwif_color_set_UV_Offset_byTBL_Level(unsigned char src_idx, unsigned char display, unsigned char which_table, VIP_YUV2RGB_LEVEL_SELECT which_Lv, unsigned char drvSkip_Flag_coefByY, unsigned char drvSkip_Flag_uvOffset);

void fwif_color_set_quality_init_value(unsigned char item_index, unsigned short value);
int fwif_color_set_quality_init_sendRPC(void);

unsigned char fwif_color_Debug_Get_UserCurveDCC_Curve_Select(void);
unsigned char fwif_color_Debug_Get_HIST_MEAN_VALUE(void);
unsigned char fwif_color_Debug_Get_Pure_Color_Flag(void);
unsigned char fwif_color_Debug_Get_Skin_Tone_Found_Flag(void);
unsigned char fwif_color_Debug_Get_UserCurveDCC_Curve_Weighting(unsigned char table_select);
unsigned char fwif_color_Debug_Get_UserCurveDCC_Curve_Weighting_Skin(unsigned char table_select);

void fwif_color_Debug_Set_UserCurveDCC_Curve_Apply(unsigned char table_select, unsigned char value);
void fwif_color_Debug_Set_UserCurveDCC_Curve_Select(unsigned char value);
void fwif_color_Debug_Set_UserCurveDCC_Curve_Weighting(unsigned char table_select, unsigned char value);
void fwif_color_Debug_Set_UserCurveDCC_Curve_Weighting_Skin(unsigned char table_select, unsigned char value);

/*#ifdef 1*/
void fwif_color_set_LD_Enable(unsigned char src_idx, bool enable);
void fwif_color_set_LD_Global_Ctrl(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_Backlight_Decision(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_Spatial_Filter(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_Spatial_Remap(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_Boost(unsigned char src_idx, unsigned char TableIdx);

void fwif_color_set_LD_Temporal_Filter(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_Backlight_Final_Decision(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_Data_Compensation(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_Data_Compensation_NewMode_2DTable(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_Backlight_Profile_Interpolation(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_Backlight_Profile_Interpolation_Table(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_BL_Profile_Interpolation_Table(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_BL_Profile_Interpolation_Table_HV(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_Demo_Window(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LD_SPIDataSRAM_Duty(unsigned short duty); /*12bits input*/
unsigned char fwif_color_Get_LD_Init_Done(void);
void fwif_color_Set_LD_Init_Done(unsigned char status);
void fwif_set_LdInterface_Table(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LDInterface_CtrlSPI_init(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LDSPI_TXDone_ISR_En(unsigned char enable);
/*#endif*/
void fwif_color_OPS_Debug_init(void);
void fwif_color_OPS_init(void);
void fwif_color_OPS_set_PLC_Curve(VIP_OPS_CTRL_Item *pOPS_CTRL_Item);
void fwif_color_OPS_Apply_inVPQ_task(void);
void fwif_color_OPS_GSR_LD_CMPS_Apply_inISR(void);

void fwif_color_set_LC_Enable(unsigned char src_idx, bool enable);
void fwif_color_set_LC_Global_Ctrl(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_Temporal_Filter(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_Backlight_Profile_Interpolation(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_Demo_Window(unsigned char src_idx, unsigned char TableIdx);

void fwif_color_set_LC_ToneMapping_SetGrid0(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_ToneMapping_SetGrid2(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_ToneMapping_CurveSelect(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_ToneMapping_Blend(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_Shpnr_Gain1st(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_Shpnr_Gain2nd(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_Diff_Ctrl0(unsigned char src_idx, unsigned char TableIdx);
//juwen, add LC : tone mapping curve
void fwif_color_set_LC_ToneMappingSlopePoint(unsigned char src_idx,unsigned char TableIdx);
void fwif_color_set_LC_DebugMode(unsigned char src_idx, unsigned char DebugMode);
char fwif_color_get_LC_Enable(void);
char fwif_color_get_LC_DebugMode(void);
void fwif_color_set_LC_Global_Region_Num_Size(unsigned char TableIdx, unsigned char TotalBlkNum);
unsigned char fwif_color_Cal_LC_Global_Region_Num_Size(VIP_DRV_Local_Contrast_Region_Num_Size *LC_Region_Num_Size,
	unsigned short  nImageHsize,unsigned short  nImageVsize,unsigned char tab_hsize,unsigned char tab_vsize, unsigned char max_vBlk, unsigned char max_hBlk);
//juwen, k5lp
void fwif_color_set_LC_saturation(unsigned char src_idx,unsigned char TableIdx);
void fwif_color_set_LC_flicker(unsigned char src_idx,unsigned char TableIdx);
//jimmy, LC decontour, k5lp
void fwif_color_set_LC_decontour(unsigned char src_idx,unsigned char TableIdx ,unsigned char level);
void fwif_color_set_LC_CDLC_script_for_demo(void);
// willy, SLC related, k6lp
void fwif_color_Init_SLC_Curve(unsigned char TblIdx);
void fwif_color_Set_SLC_Curve_Region_Num_Size(unsigned char MaxBlkHNum, unsigned char MaxBlkVNum);
unsigned char fwif_color_Cal_SLC_Curve_Region_Num_Size(
	DRV_SLC_Curve_Size *ptr,
	unsigned int ImgH, unsigned int ImgV,
	unsigned char MaxBlkHNum, unsigned char MaxBlkVNum
);
void fwif_color_Set_SLC_Histogram(unsigned char TblIdx);
void fwif_color_Set_SLC_Histogram_Region_Num_Size(unsigned char MaxBlkHNum, unsigned char MaxBlkVNum);
unsigned char fwif_color_Cal_SLC_Histogram_Region_Num_Size(
	DRV_SLC_Histogram_Size *ptr,
	unsigned int ImgH, unsigned int ImgV,
	unsigned char MaxBlkHNum, unsigned char MaxBlkVNum
);
void fwif_color_dynamic_SLC_Curve_Ctrl(void);
void fwif_color_Set_CDLC(unsigned char TblIdx);
void fwif_color_Set_LC_overlay_APL(unsigned char TblIdx);
void fwif_color_Set_LC_Tonemapping_Yavg_Yin_Blend(unsigned char TblIdx);
void fwif_color_Set_LC_Tonemapping_PQmask(unsigned char TblIdx);
void fwif_color_Set_LC_Sharpness_PQmask(unsigned char TblIdx);
unsigned int fwif_color_di_pow_Cal(unsigned int x, unsigned int p);

void fwif_set_pq_dir_path(char *path, char *patch_path);
void fwif_get_average_luma(unsigned char *val);
void fwif_get_luma_level_distribution(unsigned int *luma_distribution, unsigned char size);

int fwif_color_get_rgb_pixel_info(RGB_pixel_info *data);

void drvif_color_reg_UV_Gains_Table(void *table);
void fwif_sRGB_SetHueSat(unsigned char display, unsigned short Hue, unsigned short Sat,
	unsigned char User_Sat_en, unsigned char User_Sat_Mapping);
void fwif_sRGB_SetMatrix(unsigned char display, short (*sRGB_Matrix)[3], unsigned char forceUpdate);
/*for xvYcc*/
void fwif_color_inv_gamma_init(void);
unsigned char fwif_color_set_xvYCC(unsigned char src_idx, unsigned char display, unsigned char Mode);
void fwif_color_inv_gamma_control_front(unsigned char display);
void fwif_color_inv_gamma_control_back(unsigned char display, unsigned char ucWriteEnable);
void drvif_color_inv_gamma_control_back_TV006_demo(void);
void fwif_color_set_Inv_gamma_encode(void);
unsigned char fwif_color_set_xvYCC_sRGB_Curve(unsigned char src_idx, unsigned char display, unsigned char Mode);
void fwif_color_set_xvYCC_flag(void);
void fwif_color_xvYcc_colorspaceyuv2rgbtransfer(unsigned char display, unsigned char nSrcTyp, unsigned char xvYcc_en_flag);
void fwif_color_gamma_remmping_for_xvYcc(unsigned char src_idx, UINT16 *GOut_R, UINT16 *GOut_G, UINT16 *GOut_B);

void fwif_color_set_od(unsigned char bOD);
void fwif_color_safe_od_enable(unsigned char bEnable);
bool fwif_color_od_init(unsigned char mode);
unsigned char fwif_color_set_od_bits(unsigned char bits);
unsigned char fwif_color_od_test(unsigned char en);
void fwif_color_set_od_lut_indices(unsigned char recursive);
void fwif_color_set_od_liner_indices(bool recursive);
unsigned char fwif_color_set_od_liner_table(bool recursive);
unsigned char fwif_color_set_od_default_table(void);
#if 0
void fwif_color_od_table_tv006_convert(unsigned char *pTableIn, unsigned int *pTableOut);
void fwif_color_od_table_tv006_transform(bool transType, unsigned int *pTableIn, unsigned int *pTableOut);
#endif
void fwif_color_od_table_interpolation(unsigned char *pTableIn, unsigned char *pTableOut);
void fwif_color_od_table_transform(bool transType, bool recursive, unsigned char *pTableIn, unsigned char *pTableOut);
void fwif_color_od_table_restore(void);
void fwif_color_od_table_dynamic_for_VRR(void);
void fwif_color_handler(void);

void fwif_color_set_new_uvc(unsigned char src_idx, unsigned char level);

//void fwif_color_set_blue_stretch(unsigned char src_idx, unsigned char table);//k5l hw remove
void fwif_color_set_LGD_POD(unsigned char* pInputLUT);

void fwif_color_set_pcid2(unsigned char bpcid2);
void fwif_color_set_pcid2_data_setting(unsigned char DataSet);
void fwif_color_set_pcid2_pixel_reference_setting(unsigned char PxlRefSet);
void fwif_color_set_pcid_RgnTable_Setting_TV006(unsigned char RgnSettingSet, unsigned int RowBnd[17], unsigned int ColBnd[17]);
void fwif_color_set_pcid_RgnTable_Setting(unsigned char RgnSettingSet);

void fwif_color_refresh_pcid_RgnTable(void);
void fwif_color_pcid_Transpose_TV006(unsigned short* p_iTbl, unsigned int tblsz, unsigned int *p_oTbl);
void fwif_color_set_pcid_RgnTableValue(unsigned int* pTblValue, unsigned int ucTblIdx, unsigned char ucChannel);
void fwif_color_set_pcid_RgnWtTableValue(unsigned int* pTblValue);
/* pcid for rtice only */
void fwif_color_get_pcid_RgnTableValue(unsigned int* pTblValue, unsigned int ucTblIdx, unsigned char ucChannel);
void fwif_color_get_pcid_RgnWtTableValue(unsigned int* pTblValue);

void fwif_color_set_valc(unsigned char bValc);
void fwif_color_set_valc_table(unsigned int* pTbl, unsigned char ucTblIdx, unsigned char ucChannel);
void fwif_color_get_valc_table(unsigned int* pTbl, unsigned char ucTblIdx, unsigned char ucChannel);

short CAdjustCosine(short fDegree);
short CAdjustSine(short fDegree);

void fwif_color_set_WB_Pattern_IRE(unsigned char ucType, unsigned char ucIREx2);
void fwif_color_setrgbcolortemp_contrast(unsigned short Red, unsigned short Green, unsigned short Blue);
void fwif_color_setrgbcolortemp_bright(unsigned short Red, unsigned short Green, unsigned short Blue);
void fwif_color_setrgbcolortemp_en(unsigned char enable);

unsigned char fwif_color_icm_ini2elemtable(unsigned short *tICM_ini, unsigned char which_table, COLORELEM_TAB_T *st_icm_tab);
unsigned char fwif_color_icm_global_adjust_on_elem_table(COLORELEM_TAB_T *st_icm_tab_in, int g_dhue_cur, int *g_dsatbysat_cur, int *g_ditnbyitn_cur, COLORELEM_TAB_T *st_icm_tab_out);
unsigned char fwif_color_icm_block_adjust_on_elem_table(COLORELEM_TAB_T *st_icm_tab_in, COLORELEM_BLOCK_ADJ_PARAM *adj_param, COLORELEM_TAB_T *st_icm_tab_out, bool doCLIP);
void fwif_color_Icm_table_select(unsigned char display, unsigned short *tICM_ini, unsigned char which_table);
void fwif_color_Icm_table_select_By_DMA(unsigned char display, unsigned short *tICM_ini, unsigned char which_table);
void fwif_color_icm_set_pillar_by_ISR(void);
void fwif_color_ICM_RGB_Offset_Table_Select(unsigned char display, unsigned char which_table);
void fwif_color_D_LUT_By_DMA(unsigned char display, unsigned char enable, unsigned int *pArray);
void fwif_color_D_LUT_By_DMA_16(unsigned char display, unsigned char enable, unsigned short *pArray);
unsigned char fwif_color_get_HistoData_Countbins(UINT8 nBinNum, UINT32 *uCountBins);
unsigned char fwif_color_get_HistoData_chrmbins(UINT8 nBinNum, UINT32 *uchrmbins);
unsigned char fwif_color_get_HistoData_huebins(UINT8 nBinNum, UINT32 *uhuebins);
unsigned char fwif_color_get_HistoData_APL(UINT32 *uAPL);
unsigned char fwif_color_get_HistoData_Min(int *uMin);
unsigned char fwif_color_get_HistoData_Max(int *uMax);
unsigned char fwif_color_get_HistoData_PeakLow(int *uPeakLow);
unsigned char fwif_color_get_HistoData_PeakHigh(int *uPeakHigh);
unsigned char fwif_color_get_HistoData_skinCount(UINT32 *uskinCount);
unsigned char fwif_color_get_HistoData_SatStatus(UINT32 *uSatStatus);
unsigned char fwif_color_get_HistoData_diffSum(UINT32 *udiffSum);
unsigned char fwif_color_get_HistoData_motion(UINT32 *umotion);
unsigned char fwif_color_get_HistoData_texture(UINT32 *utexture);
unsigned char fwif_color_set_dcc_Curve_Write(unsigned char display, UINT8 CurveSegNum, signed int *sAccLUT);
unsigned char fwif_color_get_FreshContrastBypassLUT(unsigned char Curve_Seg, signed int *pDCBypassLUT);

#if !CONFIG_VPQ_TABLE_GENERATE
void fwif_color_set_LDSetLUT(UINT8 lutTableIndex);
bool fwif_color_set_LED_Initialize(HAL_LED_PANEL_INFO_T panelInfo);
void fwif_color_set_LD_CtrlSPI_init(HAL_LED_PANEL_INFO_T panelInfo);
#endif
int fwif_color_get_VDBlackLevel_MappingValue(unsigned char HAL_Src_Idx, unsigned char UI_Level,
unsigned char *Contrast, unsigned char *Brightness, unsigned char *Hue, unsigned char *Sat);
int fwif_color_set_VDBlackLevel(unsigned char Contrast, unsigned char Brightness, unsigned char Hue, unsigned char Sat);
void fwif_color_vpq_pic_init(void);

void fwif_VIP_set_Project_ID(VIP_Customer_Project_ID_ENUM Project_ID);
unsigned char fwif_VIP_get_Project_ID(void);

/*Elsie 20150310*/
void fwif_color_set_de_contour(unsigned char src_idx, unsigned char level);
unsigned char fwif_color_Set_Block_DeContour(unsigned char value);
unsigned char fwif_color_Set_Block_DeContour_Size(void);
//void fwif_color_set_color_mapping(unsigned char src_idx, unsigned char table);/*juwen, Merlin3, remove color map*/
unsigned char  fwif_color_Set_AI_Ctrl(unsigned char value_icm, unsigned char value_dcc, unsigned char value_cds, unsigned char value_ctrl);

void fwif_color_set_3dLUT(UINT8 TableIdx);
/*20150701 for VIP table CRC checking*/
unsigned int crc32(const void *buf, unsigned int size);
/* === checksum ========== */
void fwif_color_check_VIPTable_crc(VIP_table_crc_value* crc_value, SLR_VIP_TABLE * vip_table);
/*==========================*/

unsigned char fwif_color_set_pq_demo_flag_rpc(unsigned char flag);

typedef void *(*DEMO_CALLBACK_FUNC)(void *);
typedef enum _DEMO_CALLBACK_ID {
	DEMO_CALLBACKID_ON_OFF_SWITCH,
	DEMO_CALLBACKID_OVERSCAN,
	DEMO_CALLBACKID_MAX,
} DEMO_CALLBACK_ID;
void fwif_color_reg_demo_callback(DEMO_CALLBACK_ID id, DEMO_CALLBACK_FUNC cbFunc);
DEMO_CALLBACK_FUNC fwif_color_get_demo_callback(DEMO_CALLBACK_ID id);

void fwif_color_disable_VIP(unsigned char flag);

char fwif_color_set_bt2020_Handler(unsigned char display, unsigned char Enable_Flag, unsigned char bt2020_Mode);
char fwif_color_set_InvGamma(unsigned char display, unsigned char tblSelect, unsigned short *invGamma_R, unsigned short *invGamma_G,  unsigned short *invGamma_B);

char fwif_color_set_DM2_OETF_Set_Prog_Index(unsigned char tbl_sel);
void fwif_color_set_DM_HDR_3dLUT(unsigned char enable, unsigned int *pArray);/*flora@20150617, modify by juwen*/
char fwif_color_set_DM_HDR_3dLUT_16(void *p);

void fwif_OneKey_HDR_HLG_Decide_HDR(unsigned char mode);

UINT8 fwif_HDR_compare_3DLUT(void *p, VIP_HDR10_CSC3_TABLE_Mode mode);
void fwif_HDR_RGB2OPT(unsigned char enable, unsigned int *pArray);
void fwif_DM2_GAMMA_Enable(unsigned char En);

void fwif_color_get_Read_DM_HDR_3dLUT(unsigned char *enable, unsigned int *pArray);

char fwif_color_DM_HDR_3dLUT_Encode(unsigned int* p_inArray, unsigned int *p_outArray);
char fwif_color_DM_HDR_3dLUT_Decode(unsigned int* p_inArray, unsigned int *p_outArray);
void fwif_color_set_Inv_gamma_DEMO_TV006(void);
void fwif_color_reset_invGamma_tv006(unsigned char value);

void fwif_color_set_D_3dLUT(unsigned char LUT_Ctrl, unsigned int *pArray);
void fwif_color_get_Read_D_3dLUT(unsigned char *LUT_Ctrl, unsigned int *pArray);

char fwif_color_D_3dLUT_Encode(unsigned int* p_inArray, unsigned int *p_outArray);
char fwif_color_D_3dLUT_Decode(unsigned int* p_inArray, unsigned int *p_outArray);
char fwif_color_D_3dLUT_Encode_16(unsigned short* p_inArray, unsigned int *p_outArray);

char fwif_color_PQ_ByPass_Handler(unsigned char table_idx, unsigned char isSet_Flag, _RPC_system_setting_info *VIP_RPC_system_info_structure_table, unsigned char Force_byPass);
char fwif_color_set_PQ_ByPass_DRV_Active(unsigned char active_enable);
char fwif_color_get_PQ_ByPass_DRV_Active(void);
char fwif_color_set_PQ_ByPass_Handler_VPQ_TSK(void);
void fwif_color_set_PQ_ByPass_2P_mode_misc(void);
//unsigned char fwif_color_Get_PQ_ByPass_ISR_Flag(void);
//unsigned char fwif_color_Set_PQ_ByPass_ISR_Flag(unsigned char Flag);
void fwif_color_set_osd_Outer(unsigned char enable);
char fwif_color_get_osd_Outer(void);
void fwif_color_set_scaleup_hv4tap_coef(unsigned char src_idx, unsigned char display, char CoefIdx_HY, char CoefIdx_VY, char CoefIdx_HC, char CoefIdx_VC);
void fwif_color_set_BOE_RGBW(int w_rate);
/*========================== ST2094==========================================*/
#if 1
char fwif_color_set_ST2094_FrameSync_Enable(unsigned char Enable);
char fwif_color_ScalerVIP_ST2094_frameSync(void);
char fwif_color_ST2094_Ctrl_Structure_ini(void);
#endif
/*========================== ST2094==========================================*/
void fwif_color_set_SLD_init(void);
char fwif_color_SLD_Block_APL_Read_by_DMA_ISR(void/*unsigned short *pout_tbl*/);
char fwif_color_fwif_color_Get_SLD_APL(unsigned short *pTBL, unsigned int num);
#if 0 /*SLD, hack, elieli*/ //k5l remove
void fwif_color_set_SLD_Enable(unsigned char src_idx,bool enable);
void fwif_color_set_SLD_Ctrl(unsigned char src_idx, DRV_Still_Logo_Detection_Table* ptr);
void fwif_color_set_SLD_input_size(unsigned short sld_width, unsigned short sld_height);
void fwif_color_set_SLD_curve_map(unsigned char src_idx, DRV_Still_Logo_Detection_Table* ptr);
void fwif_color_set_SLD_drop(unsigned char src_idx, DRV_Still_Logo_Detection_Table* ptr);
#endif
void fwif_color_set_RGBW_histogram(int *hist);
void fwif_color_set_RGBW_adaptive_white(int hist[9][8]);
unsigned char fwif_color_set_FILM_FW_ShareMemory(void);
void fwif_color_WaitFor_DEN_STOP_UZUDTG(void);
void fwif_color_WaitFor_DEN_STOP_MEMCDTG(void);
void fwif_color_WaitFor_SYNC_START_UZUDTG(void);
void fwif_color_WaitFor_SYNC_START_MEMCDTG(void);

void fwif_color_pattern_mute(unsigned char  bflag);

void fwif_color_out_gamma_control_front(void);
void fwif_color_out_gamma_control_back(void);
void fwif_color_out_gamma_control_enable(unsigned char enable);
void fwif_color_Out_Gamma_Curve_Write(unsigned int *Gamma_Encode, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void fwif_color_out_gamma_encode(unsigned int *final_R, unsigned int *final_G, unsigned int *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B);
void fwif_color_out_gamma_encode_1ch(unsigned int * final, UINT16 * Out);

unsigned char  fwif_color_Set_I_De_XC(unsigned char value);
unsigned char  fwif_color_set_force_I_De_XC_Disable(unsigned char value);
unsigned char  fwif_color_get_force_I_De_XC_En(void);

unsigned char  fwif_color_Set_I_De_Contour(unsigned char value);

void fwif_color_set_MA_SNR_IESM_TBL(unsigned char which_TBL);

void fwif_color_colorwrite_Output_gamma(unsigned char value);
void fwif_color_colorwrite_InvOutput_gamma(unsigned char value);

void fwif_color_rtice_DM2_EOTF_Set(unsigned int *EOTF_table32_R, unsigned int *EOTF_table32_G, unsigned int *EOTF_table32_B);
void fwif_color_rtice_DM2_EOTF_Get(unsigned int *EOTF_table32_R, unsigned int *EOTF_table32_G, unsigned int *EOTF_table32_B);
void fwif_color_rtice_DM2_OETF_Set(unsigned short *OETF_table16_R, unsigned short *OETF_table16_G, unsigned short *OETF_table16_B);
void fwif_color_rtice_DM2_OETF_Get(unsigned short *OETF_table16_R, unsigned short *OETF_table16_G, unsigned short *OETF_table16_B);
char fwif_color_rtice_DM2_ToneMapping_Set(short *TM_table12_R, short *TM_table12_G, short *TM_table12_B);
char fwif_color_rtice_DM2_ToneMapping_Get(short *TM_table12_R, short *TM_table12_G, short *TM_table12_B);
char fwif_color_rtice_DM2_3D_24x24x24_LUT_Set(unsigned char enable, unsigned short *pArray);
char fwif_color_rtice_DM2_3D_24x24x24_LUT_Get(unsigned short *pArray, unsigned char getFromReg);
char fwif_color_rtice_DM2_3D_17x17x17_LUT_Set(unsigned char enable, unsigned short *pArray);
char fwif_color_rtice_DM2_3D_17x17x17_LUT_Get(unsigned short *pArray, unsigned char getFromReg);
char fwif_color_DM2_3D_LUT_BIT_Change(unsigned short *pArray, unsigned short *outArray, VIP_HDR10_CSC3_TABLE_Mode mode);

char fwif_color_DM2_EOTF_Set_By_DMA(unsigned int *EOTF_table32_R, unsigned int *EOTF_table32_G, unsigned int *EOTF_table32_B);
char fwif_color_DM2_OETF_Set_By_DMA(unsigned short *OETF_table16_R, unsigned short *OETF_table16_G, unsigned short *OETF_table16_B);
char fwif_color_DM2_ToneMapping_Set_By_DMA(short *TM_table12_R, short *TM_table12_G, short *TM_table12_B);
char fwif_color_DM2_3D_24x24x24_LUT_Set_By_DMA(unsigned char enable, unsigned short *pArray);
char fwif_color_DM2_3D_17x17x17_LUT_Set_By_DMA(unsigned char enable, unsigned short *pArray);

unsigned char fwif_color_init_PQ_device_struct_ShareMemory(void);

void fwif_color_Set_RPC_system_info_structure_table(unsigned long arg);
void fwif_color_Set_system_info_structure_table(unsigned long arg);
void fwif_color_Share_Memory_Access_VIP_TABLE_Struct(unsigned long arg);
void fwif_color_Share_Memory_Access_VIP_TABLE_CRC_Struct(unsigned long arg);
#if 0	/* juwen, move to "tc_hdr", elieli */
void fwif_color_set_TC_Enable(unsigned char src_idx, bool enable);
void fwif_color_set_TC_certification_Enable(unsigned char src_idx, bool certification_enable, bool certification_bitout);
void fwif_color_set_TC_Range_Adaptation(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_TC_L_Compute(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_TC_T_Compute(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_TC_M_Compute(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_TC_Write_C_LUT(unsigned char src_idx,unsigned char C_lut_idx);
void fwif_color_set_TC_Read_C_LUT(unsigned short *c_lut_coef_read_ptr);
void fwif_color_set_TC_Write_I_LUT(unsigned char src_idx,unsigned char I_lut_idx);
void fwif_color_set_TC_Read_I_LUT(unsigned short *c_lut_coef_read_ptr);
void fwif_color_set_TC_Write_S_LUT(unsigned char src_idx,unsigned char I_lut_idx);
void fwif_color_set_TC_Read_S_LUT(unsigned short *c_lut_coef_read_ptr);
void fwif_color_set_TC_Write_P_LUT(unsigned char src_idx,unsigned char I_lut_idx);
void fwif_color_set_TC_Read_P_LUT(unsigned short *c_lut_coef_read_ptr);
void fwif_color_set_TC_D_LUT_Step_Threshold(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_TC_Write_D_LUT(unsigned char src_idx,unsigned char D_lut_Red_idx,unsigned char D_lut_Geen_idx,unsigned char D_lut_Blue_idx);
void fwif_color_set_TC_Read_D_LUT(unsigned short *d_lut_red_coef_read_ptr,unsigned short *d_lut_green_coef_read_ptr,unsigned short *d_lut_blue_coef_read_ptr);
#else
// --------------------------------TC---------------------------------------------------------
char fwif_color_rtice_set_TC_Write_P_LUT(unsigned char src_idx, unsigned short *p_lut_coef_ptr);
char fwif_color_rtice_set_TC_Read_P_LUT(unsigned short *p_lut_coef_read_ptr);
void fwif_color_ScalerVIP_set_TC_v140_metadata2reg_frameSync(int display_OETF, int display_Brightness, int proc_mode, int width, int height, int yuv_range, _RPC_system_setting_info *RPC_sys_info);
void fwif_color_set_TC_v140_metadata2reg_init(int display_OETF, int display_Brightness, int proc_mode,int yuv_range);
void* fwif_color_Get_TC_CTRL(void);
char fwif_color_ScalerVIP_TC_metadata_frameSync(void);
void fwif_color_TC_Flow(unsigned char TC_En);
char fwif_color_set_TC_FrameSync_Enable(unsigned char Enable_flag);
char fwif_color_Reg_TC_DMA_addr(void);
// --------------------------------TC---------------------------------------------------------
#endif
unsigned char fwif_color_get_force_run_i3ddma_enable(unsigned char dispaly);
void fwif_color_VIP_get_VO_Info(unsigned char input_src_type);
void fwif_color_VIP_get_DRM_Info(unsigned char input_src_type);

void fwif_color_set_gamma_from_MagicGamma_MiddleWare(void);
char fwif_color_Cal_gammaCurve_from_MagicGamma_MiddleWare(VIP_MAGIC_GAMMA_Curve_Driver_Data *pData);
void fwif_color_set_clarity_resolution(void);
unsigned char fwif_color_icm_global_gain_access(unsigned int access_mode, unsigned char hsi_sel, short* pValue);
void fwif_color_icm_global_gain_calc(int *g_dhue_cur, int *g_dsatbysat_cur, int *g_ditnbyitn_cur);
void fwif_color_set_live_colour(unsigned char level);
void fwif_color_set_mastered_4k(void);
void fwif_color_DEXCXL_IP_ini(void);
void fwif_color_DI_IP_ini(void);
void fwif_color_DI_RTNR_Timing_Measure_Check_ISR(void);
void fwif_color_DI_IEGSM_ini(void);
unsigned int get_query_VIP_DMAto3DTABLE_start_address(unsigned char idx);
unsigned char fwif_color_set_DeMura_TBL_Mode(unsigned char mode);
unsigned char fwif_color_get_DeMura_TBL_Mode(void);
char fwif_color_DeMura_init(unsigned char bLogEn, unsigned char unpdateTBL_only);
char fwif_color_set_SIW_DeMura(unsigned char* data, unsigned int size);
char fwif_color_Get_DeMura_CTRL(DRV_DeMura_CTRL_TBL *ptr);
char fwif_color_set_DeMura_En(unsigned char enable);
unsigned char fwif_color_get_DeMura_En(void);
void fwif_color_DeMura_encode( short* tbl_ori, unsigned char table_mode, unsigned char table_separate, unsigned char plane_num, VIP_DeMura_TBL * Demura_TBL);
void fwif_color_DeMura_encode_all_para( short* tbl_ori, unsigned char table_mode, unsigned char table_separate, unsigned char plane_num, VIP_DeMura_TBL * Demura_TBL, unsigned short line_size);
void fwif_color_set_INX_Demura_ctrl(VIP_DeMura_TBL * Demura_TBL);
void fwif_color_DeMura_encode_old( short* tbl_ori_lo, short* tbl_ori_md, short* tbl_ori_hi, unsigned char table_mode, unsigned char table_separate, VIP_DeMura_TBL * Demura_TBL);
char fwif_color_INNX_Demura_TBL(unsigned char * pINNX_DemuraTBL, unsigned int TBL_Size, short *pDeLut);
void fwif_color_DeMura_encode_diff(short* tbl_ori_lo, short* tbl_ori_md, short* tbl_ori_md2, short* tbl_ori_md3, short* tbl_ori_hi, unsigned char table_mode, unsigned char table_separate, VIP_DeMura_TBL * Demura_TBL);
void fwif_color_DeMura_encode_diff_input_line_size(short* tbl_ori_lo, short* tbl_ori_md, short* tbl_ori_md2, short* tbl_ori_md3, short* tbl_ori_hi, unsigned char table_mode, unsigned char table_separate, VIP_DeMura_TBL * Demura_TBL, unsigned short in_tbl_width);
void fwif_color_set_OSD_enhance_init(void);
void fwif_color_set_OSD_enhance_enable(unsigned char Enable);
char check_at_uzudtg_front_porch(unsigned char waitFixedLine, unsigned int FixedLine);
unsigned int rand32(void);
void fwif_color_LD_HDR_OverCurren_Demo(unsigned char Enable);

unsigned char fwif_Set_VIP_Disable_PQ(VPQ_ModuleTest_Item ModuleTest_Item,unsigned char Enable);

char fwif_color_set_PQ_SOURCE_TYPE_From_AP(unsigned char);
unsigned char fwif_color_Get_PQ_SOURCE_TYPE_From_AP(void);


/* ====  Dither ======*/
void fwif_color_dither_mode(KADP_DISP_TCON_DITHER_T mode);
void fwif_color_set_Main_Dither(unsigned char enable,unsigned char nTableType);
void fwif_color_set_HDMI_Dither(unsigned char enable,unsigned char nTableType);
void fwif_color_set_HSD_Dither(unsigned char enable);
void fwif_color_set_MEMC_dither(unsigned char enable,VIP_PANEL_BIT default_vip_panel_bit,unsigned char nTableType);
void fwif_color_set_Panel_Dither(unsigned char enable,VIP_PANEL_BIT default_vip_panel_bit,unsigned char nTableType);
void fwif_color_set_Dynamic_Dither(unsigned char enable, unsigned char nTableType);
 void fwif_color_set_PCID_Dither(unsigned char enable,unsigned char nTableType);
 void fwif_color_set_LGD_Dither(unsigned char enable,unsigned char nTableType);
 /* =========================*/
void fwif_color_MEX_MODE_init(void);
unsigned char fwif_color_set_PIP_overlap_area_color_selection(unsigned char sel);

void fwif_color_set_HDR_sub_en(unsigned char bEnable);
void fwif_color_HDR_sub_set422to444(unsigned char bEnable);
void fwif_color_HDR_Sub_gamma_encode(unsigned int *final_R, unsigned int *final_G, unsigned int *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B);
void fwif_color_set_HDR_Sub_Gamma1(unsigned char en, unsigned short *Gamma_R, unsigned short *Gamma_G,  unsigned short *Gamma_B);
void fwif_color_set_HDR_Sub_Gamma2(unsigned char en, unsigned short *Gamma_R, unsigned short *Gamma_G,  unsigned short *Gamma_B);

void fwif_color_HDR_sub_set_yuv2rgb(DRV_subHDR_YUV2RGB_CSMatrix *ptr);
void fwif_color_HDR_sub_set_colorMap(DRV_Color_Mapping *ptr);

void fwif_color_Set_BLPF_type(DRV_BLPF_TYPE BLPF_Idx);

DRV_BLPF_TYPE fwif_color_Get_BLPF_type(void);

//TV030
void fwif_color_set_sRGBMatrix(void);
unsigned char fwif_get_colorspaceInfo(void);
void fwif_color_ColorMap_SetMatrix(unsigned char display, short (*ColorMapMatrix)[3], unsigned char scale, unsigned char forceUpdate, unsigned char bNoWaitPorch);
void fwif_color_set_color_mapping_enable(unsigned char display, unsigned char enable);
void fwif_color_cpy_D_3DLUT_TBL(void);
void fwif_color_set_gamma_Magic(void);
void fwif_color_set_gamma_curve_index(unsigned char gamma_level, unsigned int **In_R, unsigned int **In_G, unsigned int **In_B);
void fwif_set_gamma_multiply_remapOETF(UINT16 *final_R, UINT16 *final_G, UINT16 *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B);
void fwif_set_gamma_multiply_remap(UINT16 *final_R, UINT16 *final_G, UINT16 *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B);
void fwif_color_set_GainOffsetGamma(UINT16 *GOut_R, UINT16 *GOut_G, UINT16 *GOut_B);
void fwif_color_get_LC_Hist_dat(unsigned int *plc_out);

void fwif_color_out_gamma_curve_data_protect(UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B);
void fwif_color_set_color_temp(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con, short Red_Bri, short Green_Bri, short Blue_Bri, unsigned char waitSync);
void fwif_color_set_iog_color_temp(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con, short Red_Bri, short Green_Bri, short Blue_Bri, unsigned char waitSync);
void fwif_color_set_frequency_detection(unsigned char idx);
void fwif_color_PQ_SR_MODE_init(void);
unsigned char fwif_color_Get_PQ_SR_MODE(void);
void fwif_color_Set_PQ_SR_MODE(unsigned char sr_mode); //for rtice used
void fwif_color_Set_NN_SRCtrl(unsigned char srnn_ctrl_idx);
char fwif_color_Set_NN_SR_Weights_Blend(unsigned char nnsr_mode, unsigned char weight_blend_idx)  ;
void fwif_color_Set_NNSR_model(unsigned char ai_sr_mode);
void fwif_color_Set_NNSR_model_By_DMA(unsigned char nnsr_mode, unsigned short *pArray);
void fwif_color_srnn_weight_blend_4_axis(unsigned char ai_sr_mode,DRV_SRNN_WEIGHT_Table SRNN_WEIGHT_Table,int blend_tbl_num, int scale, unsigned short *pin_array, unsigned short *pout_array );
void fwif_color_srnn_m_disp_en_ctrl(unsigned char en);
void fwif_color_Set_NN_SR_Weights_Blend_for_rtice(unsigned char nnsr_mode, unsigned char *blending_ctrl) ;
void fwif_color_srnn_weight_blend_4_axis_for_rtice(unsigned char ai_sr_mode,unsigned char *blending_ctrl, int para_num, int scale, unsigned short *pin_array, unsigned short *pout_array );
char fwif_color_set_NNSR_model_TBL(unsigned char nnsr_mode, unsigned short *pArray);
char fwif_color_NNSR_model_TBL_update_VPQTSK(void);
unsigned char fwif_color_get_TV006_LineOD_gate_table_index(void);
void fwif_color_PQ_DataAccess_debug_check_ISR(void);
void fwif_color_PQ_DataAccess_debug_check_Init(void);
void fwif_color_PQ_setting_check(void);

void fwif_color_Set_GSR2(unsigned char TblIdx);

void fwif_WBBL_TV002(void);
void fwif_Blue_Stretch_TV002(void);
void fwif_RGB_gain_offset_TV002(void);
void fwif_update_TV002_ConBri_Offset(short Con_Offset, short Bri_Offset);

void fwif_color_set_cds_level(unsigned char table, unsigned char sharp, unsigned char sharp_mid);
void fwif_color_set_DeSharpness(unsigned char table, unsigned char osd_value, unsigned char sharp);
void fwif_color_set_InvOutputGamma_System(unsigned int gamma_curve_index);
void fwif_color_set_ColorMap_3x3_Table(unsigned char idx);

void fwif_color_PQ_DataAccess_debug_check_ISR(void);
void fwif_color_PQ_DataAccess_debug_check_Init(void);
void fwif_color_PQ_setting_check(void);

unsigned char Get_tv006_wb_pattern(void);
void Set_tv006_wb_pattern(unsigned char value);

//void fwif_color_Set_GSR2(unsigned char TblIdx);

void fwif_color_set_linear_3dLUT(void);
void fwif_color_cpy_D_3DLUT_17_TBL(void);
void fwif_color_set_ColorMatrix(unsigned char enable,unsigned int index);
unsigned int fwif_color_get_colorspaceInfo(void);

void fwif_color_Set_LD_Init_Done(unsigned char status);
unsigned char fwif_color_Get_LD_Init_Done(void);
void fwif_color_LD_Boost_1DLUT_SWProcess(unsigned short* backlight_input, unsigned short* backlight_boost);
//void drvif_color_set_LDSPI_DataSRAM_Data_Continuous(unsigned short SRAM_Position, unsigned short length, unsigned short* SRAM_Data);
void fwif_color_ld_power_protect(unsigned short *BLValue,unsigned short ldblocknum);
void fwif_color_LocalDimming_Process(void);
void fwif_color_Set_LD_SPI_Header(unsigned char head_length,unsigned char *phead,unsigned char ext_length,unsigned char *pextdata);
//void fwif_color_od_ctrl_update_setting(DRV_od_ctrl *od_ctrl);
unsigned char fwif_color_set_od_table(void);
void fwif_color_set_3dlut_table_run(void);
unsigned int fwif_color_get_colorspaceInfo(void);
void fwif_color_set_ColorMatrix(unsigned char enable,unsigned int index);
void fwif_color_LD_SPI_init(unsigned char init_status);
void fwif_color_LD_Table_init(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_update_frameRates(unsigned int frameRates);
void fwif_color_dlg_trigger(void);
void fwif_color_dlg_do_suspend(void);
void fwif_color_framerate_detect(void);
void fwif_color_od_ctrl_get_setting(void);
void fwif_color_od_get_table(void);
unsigned char fwif_color_get_valc(void);
void fwif_color_set_pcid(void);
char fwif_color_get_pcid(void);
void fwif_color_icm_Set_RGB_Offset_Table_Smooth(int temp_tar_icmRGB[3][HUESEGMAX],unsigned char temp_tar_s_gain[SATSEGMAX], unsigned char temp_tar_i_gain[ITNSEGMAX],unsigned char which_table);
void fwif_color_set_demura_dlg_mode(unsigned char dlg_mode);
void fwif_color_set_PCLRC_Enable(bool enable);
void fwif_color_set_LD_Ctrl_SPI_init(void);
unsigned char fwif_color_get_vpan_status(void);
void fwif_color_set_LD_hvsize_change(unsigned short udispWidth,unsigned short udispHeight);
void fwif_color_LD_dynamic_proccess(void);
void drvif_color_get_LD_Block_Max_Info(unsigned short *LD_BlockMax, unsigned short start_addr, unsigned short length, unsigned short *global_max_ave, unsigned short *max_high_low_score);
void drvif_color_get_LD_Block_Ave_Info(unsigned short *LD_BlockAve, unsigned short start_addr, unsigned short length, unsigned short *global_ave);
unsigned char fwif_color_LD_SW_Temporal_Filter(unsigned char* Current, unsigned char *target, unsigned char step, unsigned char cnt);



#if 0 //H Rim
unsigned short* fwif_color_Get_APL(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SCALER_COLOR_H__*/


