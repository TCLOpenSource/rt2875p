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
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/vip/localdimming.h>
#include <tvscalercontrol/vip/localdimmingInterface.h>
#include <scaler_vbedev.h>
#include <scaler/vipRPCCommon.h>
/*#include <tvscalercontrol/vip/scalerColor_tv006.h>*/
#endif
#include <tvscalercontrol/vip/pcid.h>/*sync TV030*/

//#define VIP_AP_DIAS_ENGINEER_MENU_CONFIG 1

#define _MAIN_DISPLAY								SLR_MAIN_DISPLAY
#define _SUB_DISPLAY								SLR_SUB_DISPLAY

#define OSD_Contrast_Compensation 1
#define complement2(arg) (((~arg)&0x7FF) + 1)
#define VIP_YUV2RGB_K_Y_NEG_BND 0x3FFF
#define VIP_YUV2RGB_K_C_NEG_BND 0x3FF
#define VIP_YUV2RGB_OFFSET_NEG_BND 0x3FFF
#define VIP_YUV2RGB_K_Y_NEG_CHANGE(arg) (((~arg)&(VIP_YUV2RGB_K_Y_NEG_BND*2+1)) + 1)
#define VIP_YUV2RGB_K_C_NEG_CHANGE(arg) (((~arg)&(VIP_YUV2RGB_K_C_NEG_BND*2+1)) + 1)
#define VIP_YUV2RGB_OFFSET_NEG_CHANGE(arg) (arg - ((VIP_YUV2RGB_OFFSET_NEG_BND+1)*2))

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

/* define DMA BUFFER*/
#define VIP_DMAto3DTable_NUM 3
#define VIP_DMAto3DTable_HDR_3D_LUT_SIZE (320 * 1024)
#define VIP_DMAto3DTable_HDR_EOTF_SIZE (150 * 1024)
#define VIP_DMAto3DTable_HDR_OETF_SIZE (100 * 1024)
#define VIP_DMAto3DTable_D_3D_LUT_SIZE (120 * 1024)
#define VIP_DMAto3DTable_ICM_SIZE (200 * 1024)
#define VIP_DMAto3DTable_TC_SIZE (100 * 1024)

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
#define INNX_Demura_W16 496
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
#define INNX_Demura_DecodeLUT_High_sta_addr (INNX_Demura_h_271 * INNX_Demura_W16 * 2)
#define INNX_Demura_DecodeLUT_4_sta_addr (INNX_Demura_h_271 * INNX_Demura_W16 * 3)
#define INNX_Demura_DecodeLUT_5_sta_addr (INNX_Demura_h_271 * INNX_Demura_W16 * 4)
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

typedef enum _VIP_DMAto3DTABLE_INDEX {
	DMAto3DTABLE_HDR_3D_LUT = 0,
	DMAto3DTABLE_HDR_EOTF,
	DMAto3DTABLE_HDR_OETF,
	DMAto3DTABLE_HDR_D_3D_LUT,
	DMAto3DTABLE_ICM,
	DMAto3DTABLE_TC,

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

typedef enum {
	qs_sharp_idx =0,
	qs_MBPK_idx,
	qs_DLTI_idx,
	qs_iDcti_idx,
	qs_dDcti_idx,
	qs_Gamma_idx,
	qs_Osd,
} qs_idx;




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
	YUV2RGB_INPUT_TBL_Disable,
	
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
	LC_BLOCK_60x34
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

extern DRV_NR_Item      *Manual_NR_TABLE;

//TV030
typedef enum _COLORSPACE_ITEM
{
	isBT2020=0,
	is601,
	is709,
	isDCI,
	isAdobeRGB,
	COLORSPACE_MAX,
}COLORSPACE_ITEM;

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

typedef struct _RTK_TV030_MagicGamma {
	unsigned short enable;
	unsigned short MagicGammaR[20];
	unsigned short MagicGammaG[20];
	unsigned short MagicGammaB[20];

} RTK_TV030_MagicGamma;

typedef struct _VD_COLOR_ST {
	unsigned char Contrast;
	unsigned char Brightness;
	unsigned char Saturation;
	unsigned char Hue;
} VD_COLOR_ST;


typedef struct {
	unsigned char OSD_Contrast;
	unsigned char OSD_Brightness;
	unsigned char OSD_Saturation;
	unsigned char OSD_Hue;
	unsigned char OSD_Sharpness;
	unsigned char OSD_colorTemp;
	unsigned short Contrast_Gain;
	unsigned short Brightness_Gain;
	unsigned short Saturation_Gain;
	unsigned short Hue_Gain;/* unit:degree*/
	short Brightness_Cmps;
	unsigned short Saturation_Cmps;
} QS_value_Store;


typedef struct{
	VIP_Sharpness_Table	qs_sharp;
	VIP_MBPK_Table		qs_MBPK;
	DRV_D_DLTI_Table	qs_DLTI;
	DRV_VipNewIDcti_Table	qs_iDcti;
	DRV_VipNewDDcti_Table	qs_dDcti;
	VIP_Gamma		qs_Gamma;
	QS_value_Store          qs_Store;
} DRV_QS_Table;

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


#if !CONFIG_VPQ_TABLE_GENERATE

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
#ifdef CONFIG_HW_SUPPORT_D_HISTOGRAM
void fwif_color_D_hist_init(unsigned char display, unsigned short width, unsigned short height);
#endif //CONFIG_HW_SUPPORT_D_HISTOGRAM
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
void fwif_color_read_output_gamma(unsigned char value);
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
void fwif_set_od_setting_table(DRV_od_table_mode_t from_bin[]);
void fwif_set_od_table(DRV_od_table_t* from_bin);
void fwif_set_od_table_test(unsigned char from_bin[]);
void fwif_color_set_od_gain_RGB(unsigned char rGain, unsigned char gGain, unsigned char bGain);
void fwif_set_DeMura(VIP_DeMura_TBL* from_bin);
void fwif_set_VALC(unsigned char from_bin[6][17][17]);
void fwif_set_VALCprotectSetting(DRV_pcid_valc_t from_bin[]);
void fwif_set_LEDOutGamma(unsigned short from_bin[]);
void fwif_set_Output_InvOutput_Gamma(VIP_Output_InvOutput_Gamma from_bin[]);


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

/*===============================================================================================	*/ /*ConBri CSFC 20140128*/
/*== set con/bri ===*/
#if defined(CONFIG_RTK_AI_DRV)
void fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain_AI(unsigned char src_idx, unsigned char display,  _system_setting_info *system_info_struct/*, int Con, int Bri, int Color*/);
#endif
void fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(unsigned char src_idx, unsigned char display,  _system_setting_info *system_info_struct);
void fwif_color_setContrast(unsigned char src_idx, unsigned char display, unsigned char value);
void fwif_color_setBrightness(unsigned char src_idx, unsigned char display, unsigned char value);
void fwif_color_CalContrast_Gain(unsigned char OSD_Contrast, unsigned char *Contrast_Gain, unsigned char *Y_Clamp);
void fwif_color_CalContrast_Compensation(unsigned char display, unsigned char Contrast_Gain, _system_setting_info *system_info_struct);
void fwif_color_SetDataFormatHandler(SCALER_DISP_CHANNEL display, unsigned short mode, unsigned short channel, unsigned char en_422to444, unsigned char InputSrcGetType);

/*===============================================================================================	*/ /*ConBri CSFC 20140128*/

/*== set hue/sat ===*/
void fwif_color_setSaturation(unsigned char src_idx, unsigned char display, unsigned char value);
void fwif_color_setHue(unsigned char src_idx, unsigned char display, unsigned char value);
/*======set hue/sat ===*/

/*============== ICM  =====*/
void fwif_color_regIcmTable(unsigned short *table);
void fwif_color_set_ICM_table(unsigned char src_idx, unsigned char value);
void fwif_color_set_ICM_table_driver(unsigned char src_idx, unsigned char value);
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
void fwif_color_set_color_temp(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con, short Red_Bri, short Green_Bri, short Blue_Bri, unsigned char waitSync);
void fwif_color_set_iog_color_temp(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con, short Red_Bri, short Green_Bri, short Blue_Bri, unsigned char waitSync);
void fwif_color_gamma_control_front(unsigned char display);
void fwif_color_gamma_control_back(unsigned char display, unsigned char ucWriteEnable);
void fwif_color_set_gamma_Identity(void);
void fwif_color_get_gamma_default(unsigned char Gamma_Mode, unsigned int **In_R, unsigned int **In_G, unsigned int **In_B);
void fwif_color_encode_gamma_debug(unsigned char type);
void fwif_color_gamma_decode(UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B, unsigned int *In_R, unsigned int *In_G, unsigned int *In_B);
void fwif_color_decode_gamma_debug(unsigned char type);
void fwif_color_gamma_blending_S_curve(unsigned short index, unsigned short C_gain1, unsigned short C_gain0);
void fwif_color_gamma_curve_data_protect(UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B);
void fwif_color_gamma_curve_bitdata_protect(UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B,UINT16 Bin_Num ,UINT16 Vmax);
void fwif_color_gamma_encode(unsigned int *final_R, unsigned int *final_G, unsigned int *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B);
/*void fwif_color_set_gamma_encode(void);*/
void fwif_color_set_gamma_encode(unsigned int *final_GAMMA_R, unsigned int *final_GAMMA_G, unsigned int *final_GAMMA_B);
bool fwif_color_set_ddomainISR_gamma_encode(unsigned int *final_GAMMA_R, unsigned int *final_GAMMA_G, unsigned int *final_GAMMA_B);
void fwif_color_Set_Inv_Gamma_Encode(unsigned int *final_Inv_GAMMA_R, unsigned int *final_Inv_GAMMA_G, unsigned int *final_Inv_GAMMA_B);
/*======== Set color temp end=============*/
void fwif_set_gamma(unsigned char Gamma_Mode, unsigned char Gamma_Sindex, unsigned char Gamma_Shigh, unsigned char Gamma_Slow);
void fwif_set_ddomainISR_gamma(unsigned char Gamma_Mode, unsigned char Gamma_Sindex, unsigned char Gamma_Shigh, unsigned char Gamma_Slow);
void fwif_set_software_gamma(unsigned char Gamma_Mode, unsigned char Gamma_Sindex, unsigned char Gamma_Shigh, unsigned char Gamma_Slow);
void fwif_color_set_InvOutputGamma_System(unsigned int gamma_curve_index);
void fwif_color_write_InvOutputGamma_curve(void);
void fwif_color_gamma_encode_enhance(unsigned int *pArray_d0d1, unsigned int *pArray_index, unsigned short *pDecode);
void fwif_color_gamma_control_front_enhance(unsigned char display);
void fwif_color_gamma_control_back_enhance(unsigned char display, unsigned char ucWriteEnable);
void fwif_color_set_gamma_enhance(unsigned int *pArray_d0d1,unsigned int *pArray_index, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx);
void fwif_set_gamma_system_enhance(unsigned short *pDecode_TBL_R, unsigned short *pDecode_TBL_G, unsigned short *pDecode_TBL_B);

unsigned char fwif_color_GetAutoMAFlag(void);
void fwif_color_SetAutoMAFlag(unsigned char flag);

//char fwif_color_DI_RTNR_FRZ_ISR(void);
//char fwif_color_DI_RTNR_FRZ_ReCheck_TSK(void);

/*===============================================================================================	*/ /*NR rock 20140127*/
void fwif_color_RTNR_flow(DRV_NR_Item *ptr);
void fwif_color_SNR_flow(DRV_NR_Item *ptr);
void fwif_color_SetDNR(unsigned char display, unsigned char level);
void fwif_color_SetMPEGNR(unsigned char display, unsigned char level, unsigned char calledByOSD);
void fwif_color_RHAL_SetDNR(unsigned char level);
void fwif_color_RHAL_SetMPEGNR(unsigned char level);


DRV_RTNR_General_ctrl *fwif_color_GetRTNR_General(unsigned char source, unsigned char level);
void fwif_color_set_iESM(unsigned char display, unsigned char value);
/*void fwif_color_SetPQA(unsigned char input, unsigned char mode);*/
void fwif_color_set_PQA_Input_table(unsigned char value);
void fwif_color_set_PQA_table(unsigned char value);
unsigned char fwif_color_get_PQA_Input_table(void);
unsigned char fwif_color_get_PQA_table(void);
void fwif_color_PQA_Input_Item_Check(_system_setting_info *VIP_system_info_structure_table, unsigned int *PQA_TABLE, unsigned char table_select);

void fwif_color_set_TNRXC_Ctrl(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_TNRXC_MK2(unsigned char src_idx, unsigned char TableIdx);


void fwif_color_Set_NRtemporal(unsigned char display, unsigned char value);

/* chen 0716*/
void fwif_color_set_rtnr_masnr_driver(unsigned char src_idx, unsigned char value);
/* end chen 0716*/

/*==============================rord.tsao start===================================================*/ /**/
void fwif_color_set_shp_driver(unsigned char src_idx, unsigned char value);
void fwif_color_sharpness_osd_adjust(unsigned char src_idx, unsigned char value);

void fwif_color_set_CDSTable_level(unsigned char table, unsigned char value);
void fwif_color_set_sharpness_level(unsigned char table, unsigned char value);
void fwif_color_set_sharpness_level_Minumun_0(unsigned char table, unsigned char value);
void fwif_color_set_sharpness_table(unsigned char src_idx, unsigned char value);
void fwif_color_set_MBPK_table_byLevel(unsigned char TBL_idx, unsigned char SHP_LV);
void fwif_color_set_MBPK_table(unsigned char src_idx, unsigned char value);
void fwif_color_set_MBSU_table(unsigned char src_idx, unsigned char value);
void fwif_color_VIPShpTable_to_DRVShpTable(VIP_Sharpness_Table *VIPShpTable, DRV_Sharpness_Table *DRVShpTable);
void fwif_color_set_shp_vgain(unsigned char src_idx, unsigned char value);
void fwif_color_set_cds_table(unsigned char src_idx, unsigned char TableIdx);

void fwif_color_set_unsharp_mask_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_egsm_postshp_level_driver(unsigned char src_idx, unsigned char value);
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
void fwif_color_set_MA_Chroma_Error(unsigned char src_idx, unsigned char value);
void fwif_color_set_dlti_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_dlti_bySHPLevel(unsigned char TBL_idx, unsigned char SHP_LV);

void fwif_color_set_dcti(unsigned char src_idx, unsigned char value);
void fwif_color_set_dcti_bySHPLevel(unsigned char TBL_idx, unsigned char SHP_LV);
void fwif_color_set_Intra_Ver2_driver(unsigned char src_idx, unsigned char value);

unsigned char fwif_color_get_MA_Chroma_Error_en_from_vipTable(unsigned char src_idx, unsigned char value);

void fwif_color_set_scaleup_hcoef(unsigned char src_idx, unsigned char display, unsigned char TableIdx);
void fwif_color_set_scaleup_vcoef(unsigned char src_idx, unsigned char display, unsigned char TableIdx);
void fwif_color_set_scaleup_dircoef(unsigned char src_idx, unsigned char display, unsigned char TableIdx);
#ifdef CONFIG_HW_SUPPORT_TWOSTEP_SCALEUP
void fwif_color_set_twostep_scaleup_coef(unsigned char src_idx, unsigned char display, unsigned char TableIdx);
#endif //CONFIG_HW_SUPPORT_TWOSTEP_SCALEUP
void fwif_color_set_scaleuph_12tap_driver(unsigned char src_idx, unsigned char display, unsigned char TableIdx);
void fwif_color_set_scaleupv_8tap_driver(unsigned char src_idx, unsigned char display, unsigned char TableIdx);

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
void fwif_color_set_inewdcti_bySHPLevel(unsigned char TBL_idx, unsigned char SHP_LV);

void fwif_color_set_v_dcti_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_v_dcti_lpf_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_UV_Delay(unsigned char src_idx, unsigned char value);
void fwif_color_set_UV_Delay_TOP(unsigned char src_idx, unsigned char value);

unsigned char fwif_color_get_UV_Delay(void);
void fwif_color_UV_Delay_Enable(unsigned char src_idx, unsigned char value);

unsigned char fwif_color_get_I3DDMA_colorspacergb2yuvtransfer_table(unsigned char nSrcType, unsigned char nSD_HD);

//void fwif_color_set_Adaptive_Gamma(unsigned char src_idx, unsigned char TableIdx);

void fwif_color_colorspacergb2yuvtransfer(unsigned char display, unsigned char nSrcType, unsigned char nSD_HD, unsigned char VO_Jpeg_Back_YUV);
#if 0
void fwif_color_colorspacergb2yuvtransfer_Seamless(unsigned char HDR_Mode, unsigned char nSrcType);
#endif
void fwif_color_set_DCC_YUV2RGB_CTRL(unsigned char display, unsigned char nSrcType, unsigned char nSD_HD, unsigned char VO_Jpeg_Back_YUV,  _system_setting_info *VIP_system_info_structure_table);
void fwif_color_set_ICM_YUV2RGB_CTRL(unsigned char display, unsigned char nSrcType, unsigned char nSD_HD, unsigned char VO_Jpeg_Back_YUV,  _system_setting_info *VIP_system_info_structure_table);

void fwif_color_setycontrastbrightness(unsigned char display, unsigned char Contrast, unsigned char Brightness);
/*void fwif_color_set_color_temp_level_type(unsigned char src_idx, unsigned char value);*/
unsigned char fwif_OsdMapToRegValue(APP_Video_OsdItemType_t bOsdItem, unsigned char value);
void fwif_color_load_QS_vip_memory_from_carveout(void);
void fwif_colo_load_vip_table(SLR_VIP_TABLE *ptr);
SLR_VIP_TABLE *fwif_colo_get_AP_vip_table_gVIP_Table(void);

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

/*LD Related Middleware start*/
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
void fwif_color_set_LD_hw_blight_end_ie_en(unsigned char enable);
void fwif_color_set_LD_hw_blight_end_ie2_en(unsigned char enable);
void fwif_color_set_LD_hist_done_ie_en(unsigned char enable);
void fwif_color_set_LD_hist_done_ie2_en(unsigned char enable);
void fwif_color_set_LDSPI_TXDone_ISR_En(unsigned char enable);
void fwif_color_Set_LD_Init_Done(unsigned char status);
unsigned char fwif_color_Get_LD_Init_Done(void);
void fwif_color_set_LD_SPIDataSRAM_Duty(unsigned short duty); /*12bits input*/
void fwif_set_LdInterface_Table(unsigned char src_idx, unsigned char TableIdx);

/*LD Related Middleware end */

void fwif_color_set_LC_Enable(unsigned char src_idx, bool enable);
void fwif_color_set_LC_DB(unsigned char en, unsigned char sel, unsigned char apply);
void fwif_color_set_LC_Global_Ctrl(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_Backlight_Decision(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_Spatial_Filter(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_Temporal_Filter(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_Backlight_Profile_Interpolation(unsigned char src_idx, unsigned char TableIdx);
void fwif_color_set_LC_Backlight_Profile_Interpolation_only_table(unsigned char src_idx,unsigned char TableIdx, unsigned int (*pTable)[125]);
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
unsigned char fwif_color_Cal_LC_Global_Region_Num_Size(
	VIP_DRV_Local_Contrast_Region_Num_Size *LC_Region_Num_Size,
	unsigned short nImageHsize, unsigned short nImageVsize,
	unsigned char tab_hsize, unsigned char tab_vsize,
	unsigned char nBlkHNum, unsigned char nBlkVNum
);
unsigned int fwif_color_di_pow_Cal(unsigned int x, unsigned int p);

void fwif_set_pq_dir_path(char *path, char *patch_path);
void fwif_get_average_luma(unsigned char *val);
void fwif_get_luma_level_distribution(unsigned int *luma_distribution, unsigned char size);

int fwif_color_get_rgb_pixel_info(RGB_pixel_info *data);

/*for xvYcc*/
void fwif_color_inv_gamma_init(void);
void fwif_color_inv_gamma_control_front(unsigned char display);
void fwif_color_inv_gamma_control_back(unsigned char display, unsigned char ucWriteEnable);
void drvif_color_inv_gamma_control_back_TV006_demo(void);
void fwif_color_set_Inv_gamma_encode(void);
void fwif_color_set_xvYCC_flag(void);
void fwif_color_xvYcc_colorspaceyuv2rgbtransfer(unsigned char display, unsigned char nSrcTyp, unsigned char xvYcc_en_flag);
void fwif_color_gamma_remmping_for_xvYcc(unsigned char src_idx, UINT16 *GOut_R, UINT16 *GOut_G, UINT16 *GOut_B);

/*Elsie 20140825 OD middleware*/
void fwif_color_set_od(unsigned char bOD);
void fwif_color_set_od_table(char tablemode, unsigned char level);
bool fwif_color_od_init(unsigned char color_depth_bits, unsigned char dma_mode, unsigned char freq);

void fwif_color_handler(void);
void fwif_color_od_handler(void);


void fwif_color_set_new_uvc(unsigned char src_idx, unsigned char level);
void fwif_color_set_blue_stretch(unsigned char src_idx, unsigned char table);

//void fwif_color_pcid_init(UINT8 bits, UINT8 mode, UINT8 freq);
//void fwif_color_pcid_handler(void);
void fwif_color_set_pcid(unsigned char bpcid);
void fwif_color_set_pcid_valuetable(unsigned char* InputLUT);
void fwif_color_set_pcid_pixel_setting(void);

void fwif_color_set_pcid2(unsigned char bpcid2);
void fwif_color_set_pcid2_valuetable(unsigned char* InputLUT);
void fwif_color_set_pcid2_valuetable_channel(unsigned char* InputLUT, unsigned char channel);
void fwif_color_set_pcid2_poltable(void);
void fwif_color_set_pcid2_pixel_setting(void);
void fwif_color_set_PCID_BypassSetting(void);
void fwif_color_set_PCID_RgnWeightSetting(DRV_pcid_valc_t *pSetting);
void fwif_color_get_PCID_RgnWeightSetting(DRV_pcid_valc_t *pSetting);
void fwif_color_set_PCID_VALCSetting(void);
void fwif_color_set_pcid_valc_hv(void);
void fwif_color_set_valc_valuetable(signed int* pInputLUT,unsigned char TableIndex, unsigned char channel);
void fwif_color_set_PCID_LineOD_DualGateSetting(void);
void fwif_color_set_pcid_RgnWeight(void);
void fwif_color_get_pcid_valuetable(char* pRetTbl, unsigned char ucTblIdx, unsigned char ucChannel);
void fwif_color_set_pcid_single_valuetable(char* pInputLUT, unsigned char ucTblIdx, unsigned char ucChannel);
short CAdjustCosine(short fDegree);
short CAdjustSine(short fDegree);
int fwif_color_icm_set_ICM_H_7axis_adjust_HSI(unsigned char display, unsigned char which_table, int prvValue, int value, unsigned char Tuning_Axis, unsigned char HSI);

void fwif_color_set_WB_Pattern_IRE(unsigned char ucType, unsigned char ucIRE);
void fwif_color_setrgbcolortemp_contrast(unsigned short Red, unsigned short Green, unsigned short Blue);
void fwif_color_setrgbcolortemp_bright(unsigned short Red, unsigned short Green, unsigned short Blue);
void fwif_color_setrgbcolortemp_en(unsigned char enable);

unsigned char fwif_color_icm_ini2elemtable(unsigned short *tICM_ini, unsigned char which_table, COLORELEM_TAB_T *st_icm_tab);
unsigned char fwif_color_icm_global_adjust_on_elem_table(COLORELEM_TAB_T *st_icm_tab_in, int g_dhue_cur, int *g_dsatbysat_cur, int *g_ditnbyitn_cur, COLORELEM_TAB_T *st_icm_tab_out);
unsigned char fwif_color_icm_block_adjust_on_elem_table(COLORELEM_TAB_T *st_icm_tab_in, COLORELEM_BLOCK_ADJ_PARAM *adj_param, COLORELEM_TAB_T *st_icm_tab_out, bool doCLIP);
void fwif_color_Icm_table_select(unsigned char display, unsigned short *tICM_ini, unsigned char which_table);
void fwif_color_Icm_table_select_By_DMA(unsigned char display, unsigned short *tICM_ini, unsigned char which_table);
void fwif_color_icm_set_pillar_by_ISR(void);
unsigned char fwif_color_icm_Write3DTable_By_DMA(COLORELEM ICM_TAB_ACCESS[ITNSEGMAX][SATSEGMAX][HUESEGMAX]);

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

void fwif_color_set_LDSetLUT(UINT8 lutTableIndex);
bool fwif_color_set_LED_Initialize(HAL_LED_PANEL_INFO_T panelInfo);
void fwif_color_dither_mode(KADP_DISP_TCON_DITHER_T mode);
void fwif_color_set_LD_CtrlSPI_init(HAL_LED_PANEL_INFO_T panelInfo);
void fwif_color_set_LDInterface_CtrlSPI_init(unsigned char src_idx, unsigned char TableIdx);
int fwif_color_get_VDBlackLevel_MappingValue(unsigned char HAL_Src_Idx, unsigned char UI_Level,
unsigned char *Contrast, unsigned char *Brightness, unsigned char *Hue, unsigned char *Sat);
int fwif_color_set_VDBlackLevel(unsigned char Contrast, unsigned char Brightness, unsigned char Hue, unsigned char Sat);
void fwif_color_vpq_pic_init(void);

void fwif_VIP_set_Project_ID(VIP_Customer_Project_ID_ENUM Project_ID);
unsigned char fwif_VIP_get_Project_ID(void);

/*Elsie 20150310*/
void fwif_color_set_de_contour(unsigned char src_idx, unsigned char level);
void fwif_color_set_color_mapping(unsigned char src_idx, unsigned char table);
void fwif_color_set_color_mapping_enable(unsigned char display, unsigned char enable);

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

void fwif_color_set_DM_HDR_3dLUT(unsigned char enable, unsigned int *pArray);/*flora@20150617, modify by juwen*/
char fwif_color_set_DM_HDR_3dLUT_16(void *p);

void fwif_OneKey_HDR_HLG_Decide_HDR(unsigned char mode);

UINT8 fwif_HDR_compare_3DLUT(void *p, VIP_HDR10_CSC3_TABLE_Mode mode);
void fwif_HDR_RGB2OPT(unsigned char enable, unsigned int *pArray);
void fwif_DM2_GAMMA_Enable(unsigned char En);

void fwif_color_get_Read_DM_HDR_3dLUT(unsigned char *enable, unsigned int *pArray, unsigned char getFrom_reg);

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
/*========================== hdr vivid ==========================================*/
#if 1
char fwif_color_set_HDR_VIVID_FrameSync_Enable(unsigned char Enable);
char fwif_color_ScalerVIP_HDR_VIVID_frameSync(void);
char fwif_color_HDR_VIVID_frameSync_DMA_Apply_TASK(void);
char fwif_color_HDR_VIVID_CTRL_Structure_ini(void);
#endif
/*========================== hdr vivid ==========================================*/

#ifdef CONFIG_HW_SUPPORT_SLD /*SLD, hack, elieli*/
void fwif_color_set_SLD_Enable(unsigned char src_idx,bool enable);
void fwif_color_set_SLD_Ctrl(unsigned char src_idx, DRV_Still_Logo_Detection_Table* ptr);
void fwif_color_set_SLD_input_size(unsigned short sld_width, unsigned short sld_height);
void fwif_color_set_SLD_curve_map(unsigned char src_idx, DRV_Still_Logo_Detection_Table* ptr);
void fwif_color_set_SLD_drop(unsigned char src_idx, DRV_Still_Logo_Detection_Table* ptr);
#endif //CONFIG_HW_SUPPORT_SLD
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
void fwif_color_bit_gamma_encode(unsigned int *final_R, unsigned int *final_G, unsigned int *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B,UINT16 Bin_Num , UINT16 Vmax);
void fwif_color_out_gamma_encode_1ch(unsigned int * final, UINT16 * Out);

#ifdef CONFIG_HW_SUPPORT_I_DE_XC //mac6 removed de_xcxl
unsigned char  fwif_color_Set_I_De_XC(unsigned char value);
#endif //CONFIG_HW_SUPPORT_I_DE_XC
unsigned char  fwif_color_Set_I_De_Contour(unsigned char value);
void fwif_color_I_De_Contour_line_buffer_check(void);

void fwif_color_set_MA_SNR_IESM_TBL(unsigned char which_TBL);

void fwif_color_colorwrite_Output_gamma(unsigned char value);
void fwif_color_colorwrite_InvOutput_gamma(unsigned char value);
unsigned int fwif_color_set_APDEM_10p_Offset_gainVal(unsigned int gainVal, unsigned char accMode);
void fwif_color_set_GammaByExponent_10p_Offset(unsigned short *pdecode_GammaTableR, unsigned short *pdecode_GammaTableG, unsigned short *pdecode_GammaTableB);

void fwif_color_rtice_DM2_EOTF_Set(unsigned int *EOTF_table32_R, unsigned int *EOTF_table32_G, unsigned int *EOTF_table32_B);
void fwif_color_rtice_DM2_EOTF_Get(unsigned int *EOTF_table32_R, unsigned int *EOTF_table32_G, unsigned int *EOTF_table32_B);
void fwif_color_rtice_DM2_OETF_Set(unsigned short *OETF_table16_R, unsigned short *OETF_table16_G, unsigned short *OETF_table16_B);
void fwif_color_rtice_DM2_OETF_Get(unsigned short *OETF_table16_R, unsigned short *OETF_table16_G, unsigned short *OETF_table16_B);
int fwif_color_icm_get_ICM_H_7axis_adjust_HSI(int index, unsigned char Tuning_Axis);

#ifdef CONFIG_HW_SUPPORT_HDR_TONE
char fwif_color_rtice_DM2_ToneMapping_Set(short *TM_table12_R, short *TM_table12_G, short *TM_table12_B);
char fwif_color_rtice_DM2_ToneMapping_Get(short *TM_table12_R, short *TM_table12_G, short *TM_table12_B);
#endif //CONFIG_HW_SUPPORT_HDR_TONE
#ifdef CONFIG_HW_SUPPORT_HDR_3DLUT_24
char fwif_color_rtice_DM2_3D_24x24x24_LUT_Set(unsigned char enable, unsigned short *pArray);
char fwif_color_rtice_DM2_3D_24x24x24_LUT_Get(unsigned short *pArray, unsigned char getFromReg);
#endif //CONFIG_HW_SUPPORT_HDR_3DLUT_24
char fwif_color_DM2_3D_LUT_BIT_Change(unsigned short *pArray, unsigned short *outArray, VIP_HDR10_CSC3_TABLE_Mode mode);

char fwif_color_DM2_EOTF_Set_By_DMA(unsigned int *EOTF_table32_R, unsigned int *EOTF_table32_G, unsigned int *EOTF_table32_B);
char fwif_color_DM2_OETF_Set_By_DMA(unsigned short *OETF_table16_R, unsigned short *OETF_table16_G, unsigned short *OETF_table16_B);
#ifdef CONFIG_HW_SUPPORT_HDR_3DLUT_24
char fwif_color_DM2_3D_24x24x24_LUT_Set_By_DMA(unsigned char enable, unsigned short *pArray);
#endif //CONFIG_HW_SUPPORT_HDR_3DLUT_24
char fwif_color_DM_3D_17x17x17_LUT_Set_By_DMA(unsigned char enable, unsigned short *pArray);

void fwif_color_Set_RPC_system_info_structure_table(unsigned long arg);
void fwif_color_Set_system_info_structure_table(unsigned long arg);
void fwif_color_Share_Memory_Access_VIP_TABLE_Struct(unsigned long arg);
void fwif_color_Share_Memory_Access_VIP_TABLE_CRC_Struct(unsigned long arg);

// --------------------------------TC---------------------------------------------------------
char fwif_color_rtice_set_TC_Write_P_LUT(unsigned char src_idx, unsigned short *p_lut_coef_ptr);
char fwif_color_rtice_set_TC_Read_P_LUT(unsigned short *p_lut_coef_read_ptr);
void fwif_color_ScalerVIP_set_TC_v130_metadata2reg_frameSync(int display_OETF, int display_Brightness, int proc_mode, int width, int height, int yuv_range, _RPC_system_setting_info *RPC_sys_info);
void fwif_color_set_TC_v130_metadata2reg_init(int display_OETF, int display_Brightness, int proc_mode, int width, int height, int yuv_range);
void* fwif_color_Get_TC_CTRL(void);
char fwif_color_ScalerVIP_TC_metadata_frameSync(void);
void fwif_color_TC_Flow(unsigned char TC_En);
char fwif_color_set_TC_FrameSync_Enable(unsigned char Enable_flag);
// --------------------------------TC---------------------------------------------------------

unsigned char fwif_color_get_force_run_i3ddma_enable(unsigned char dispaly);
void fwif_color_safe_od_enable(unsigned char bEnable);


void fwif_color_VIP_get_VO_Info(unsigned char input_src_type);
void fwif_color_VIP_get_DRM_Info(unsigned char input_src_type);

void fwif_color_set_gamma_from_MagicGamma_MiddleWare(void);
char fwif_color_Cal_gammaCurve_from_MagicGamma_MiddleWare(VIP_MAGIC_GAMMA_Curve_Driver_Data *pData);
void fwif_color_set_clarity_resolution(void);
unsigned char fwif_color_icm_global_gain_access(unsigned int access_mode, unsigned char hsi_sel, short* pValue);
void fwif_color_icm_global_gain_calc(int *g_dhue_cur, int *g_dsatbysat_cur, int *g_ditnbyitn_cur);
void fwif_color_set_live_colour(unsigned char level);
void fwif_color_set_mastered_4k(void);
void fwif_color_DI_RTNR_Timing_Measure_Check_ISR(void);
void fwif_color_DI_IP_ini(void);
void fwif_color_DI_IEGSM_ini(void);
unsigned int get_query_VIP_DMAto3DTABLE_start_address(unsigned char idx);
unsigned char fwif_color_set_DeMura_TBL_Mode(unsigned char mode);
unsigned char fwif_color_get_DeMura_TBL_Mode(void);
char fwif_color_DeMura_init(void);
char fwif_color_Get_DeMura_CTRL(DRV_DeMura_CTRL_TBL *ptr);
char fwif_color_set_DeMura_En(unsigned char enable);
unsigned char fwif_color_get_DeMura_En(void);
char fwif_color_INNX_Demura_TBL(unsigned char * pINNX_DemuraTBL, unsigned short TBL_Size, char *pDeLut);
#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
void fwif_color_set_OSD_enhance_init(void);
void fwif_color_set_OSD_enhance_enable(unsigned char Enable);
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
unsigned char fwif_color_set_od_bits(unsigned char bits);
unsigned char fwif_color_set_od_dma_enable(unsigned char enable);
unsigned char fwif_color_get_od_bits(void);
unsigned char fwif_color_get_od_en(void);
unsigned char fwif_color_od_test(unsigned char en);
unsigned char fwif_color_set_liner_od_table(void);
char check_at_uzudtg_front_porch(unsigned char waitFixedLine, unsigned int FixedLine);
unsigned int rand32(void);

 /* ====  Dither ======*/
void fwif_color_dither_mode(KADP_DISP_TCON_DITHER_T mode);
void fwif_color_set_Main_Dither(unsigned char enable,unsigned char nTableType);
void fwif_color_set_HDMI_Dither(unsigned char enable,unsigned char nTableType);
void fwif_color_set_HSD_Dither(unsigned char enable);
#ifdef CONFIG_HW_SUPPORT_MEMC
void fwif_color_set_MEMC_dither(unsigned char enable,VIP_PANEL_BIT default_vip_panel_bit,unsigned char nTableType);
#endif //CONFIG_HW_SUPPORT_MEMC
void fwif_color_set_Panel_Dither(unsigned char enable,VIP_PANEL_BIT default_vip_panel_bit,unsigned char nTableType);
void fwif_color_set_Dynamic_Dither(unsigned char enable, unsigned char nTableType);
 void fwif_color_set_PCID_Dither(unsigned char enable,unsigned char nTableType);
 void fwif_color_set_LGD_Dither(unsigned char enable,unsigned char nTableType);
 /* =========================*/

/*sync TV030*/
void fwif_color_set_pcid2_valuetable_bycolor(unsigned char* InputLUT, DRV_pcid2_channel_t Channel);
/*sync TV030 end*/

void fwif_color_set_POD_DATA_table(DRV_POD_DATA_table_t* POD_DATA_table);

char fwif_color_get_Peaking_Data_Access(unsigned int *index, unsigned int *YUV);
char fwif_color_get_UZD_Data_Access(unsigned int *index, unsigned int *YUV);
char fwif_color_get_UZU_Data_Access(unsigned int *index, unsigned int *YUV);
char fwif_color_get_ICM_Data_Access(unsigned int *index, unsigned int *YUV);
char fwif_color_get_YUV2RGB_Data_Access(unsigned int *index, unsigned int *YUV);
char fwif_color_get_Panel_Compensation_Data_Access(unsigned int *index, unsigned int *YUV);

void fwif_color_IESM_format_convert(void);
unsigned char fwif_color_get_LC_blk_hnum(void);
unsigned char fwif_color_get_LC_blk_vnum(void);
unsigned char fwif_color_get_LD_Enable(void);
unsigned int  fwif_color_get_get_LC_APL(unsigned int *ptr_out, unsigned int bufferSz);
unsigned int  fwif_color_get_get_LC_Histogram(unsigned int *plc_out);

//TV030
void drvif_color_get_D_3DLUT_by_8vertex(UINT16 *vip_8vertex);
void fwif_color_get_LC_Blight_dat(unsigned int *ptr_out, unsigned int bufferSz);
unsigned char fwif_Slope_Interpolation(UINT16* Array, unsigned short inLow, unsigned short inHigh, UINT16 outLow, UINT16 outHigh);
void fwif_color_get_LC_Hist_dat(unsigned int *plc_out);
void fwif_color_set_gamma_curve_index(unsigned char gamma_level, unsigned int **In_R, unsigned int **In_G, unsigned int **In_B);
void fwif_color_set_gamma_from_MagicGamma_10points(RTK_TV030_MagicGamma *pData);
void fwif_color_set_gamma_Magic(void);
void fwif_color_set_20pointsData(RTK_TV030_MagicGamma *pData);
void fwif_color_set_20pointsGamma(UINT16 *GOut_R, UINT16 *GOut_G, UINT16 *GOut_B);
void fwif_color_set_GainOffsetGamma(UINT16 *GOut_R, UINT16 *GOut_G, UINT16 *GOut_B);
void fwif_set_gamma_multiply_remap(UINT16 *final_R, UINT16 *final_G, UINT16 *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B);
void fwif_set_gamma_multiply_remapOETF(UINT16 *final_R, UINT16 *final_G, UINT16 *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B);
void fwif_color_set_sRGBMatrix(void);
unsigned char fwif_get_colorspaceInfo(void);
unsigned char fwif_color_get_APDEM_CM_IDX(void);
void fwif_color_set_ColorMap_3x3_Table(unsigned char idx);
void fwif_color_D_LUT_By_DMA(unsigned char display, unsigned char enable, unsigned int *pArray);
char fwif_color_D_LUT_By_DMA_16(unsigned char display, unsigned char enable, unsigned short *pArray);
void fwif_color_ColorMap_SetMatrix(unsigned char display, short (*ColorMapMatrix)[3], unsigned char scale, unsigned char forceUpdate, unsigned char bNoWaitPorch);
void fwif_color_ColorMap_SetMatrix_offset(unsigned char display, short (*ColorMapMatrix)[3], unsigned char scale, int* In_Offset, int* Out_Offset);
void fwif_color_set_color_mapping_enable(unsigned char display, unsigned char enable);
void fwif_color_cpy_D_3DLUT_TBL(void);
unsigned char fwif_write_qs_pq_table(qs_idx idx , unsigned char *ptr);
#endif

#ifdef BUILD_QUICK_SHOW /*for quick_show mode */
void fwif_color_hdr_init_quick_show(void);
void fwif_color_pq_init_quick_show(void);
void fwif_color_colorspacergb2yuvtransfer_quick_show(void);
void fwif_color_pq_ap_ctrl_quick_show(void);
void fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain_quick_show(void);
void fwif_color_set_YUV2RGB_quick_show(void);
void fwif_color_SetDataFormatHandler_quick_show(unsigned char en_422to444);

//void fwif_color_SET_StructColorDataFacModeType_quick_show(void);

#endif /* end of quick_show */

unsigned char Get_tv006_wb_pattern(void);
void Set_tv006_wb_pattern(unsigned char value);
void fwif_color_set_OutputGamma_System(unsigned int gamma_curve_index);
unsigned char  fwif_color_Set_AI_Ctrl(unsigned char value_icm, unsigned char value_dcc, unsigned char value_ctrl);
unsigned char fwif_Set_VIP_Disable_PQ(VPQ_ModuleTest_Item ModuleTest_Item,unsigned char Enable);
void fwif_color_set_Panel_Dither_By_ini(void);

#ifdef __cplusplus
}
#endif

#endif /* __SCALER_COLOR_H__*/


