#ifndef __SCALERCOLOR_LIB_H__
#define __SCALERCOLOR_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Header include
******************************************************************************/


#include <tvscalercontrol/vip/icm.h>
#include <tvscalercontrol/scaler/source.h>
#include <tvscalercontrol/scaler/vipinclude.h>
#include <tvscalercontrol/vip/color.h>


/*******************************************************************************
* Macro
******************************************************************************/
/*#define MID_EXAMPLE(1)*/ /* Debug Message switch */
#define NOT_BY_DISPLAY 							0
#define BY_DISPLAY 								1


/*******************************************************************************
* Constant
******************************************************************************/
/*#define example  100000*/ /* 100Khz */

#define SCALERLIB_PatternColor_OFF		0
#define SCALERLIB_PatternColor_RED		1
#define SCALERLIB_PatternColor_GREEN	2
#define SCALERLIB_PatternColor_BLUE		3
#define SCALERLIB_PatternColor_WHITE	4
#define SCALERLIB_PatternColor_BLACK	5
#define SCALERLIB_PatternColor_COLORBAR	6
#define SCALERLIB_PatternColor_GRAYBAR	7
/*yanger20130114 modify for factory string command>*/
#ifdef CONFIG_ENABLE_COMPAL_LA_FACTORY_STRING_COMMAND
#define SCALERLIB_PatternColor_GRAY_25P	8
#define SCALERLIB_PatternColor_GRAY_50P	9
#endif
/*yanger20130114 modify for factory string command<*/

#define SCALERLIB_PatternColor_DCC_ON	254
#define SCALERLIB_PatternColor_DCC_OFF	255
#ifdef ENABLE_FACTORY_FUNCTIONS_TCL /*hardy20121212*/
#define SCALERLIB_PatternColor_YELLOW	8
#define SCALERLIB_PatternColor_GRAY	9
#endif


/*******************************************************************************
 * Structure
 ******************************************************************************/
/*typedef struct*/
/*{*/
/*} MID_example_Param_t;*/


/*******************************************************************************
* enumeration
******************************************************************************/
/*typedef enum*/
/*{*/
/*    MID_Example_SLEEPING = 0,*/
/*    MID_Example_RUNNING,*/
/*} MID_Example_Status_t;*/

enum _source_TV05_index {
	_SOURCE_INDEX_TV05_VGA,
	_SOURCE_INDEX_TV_NTSC,
	_SOURCE_INDEX_TV_PAL,
	_SOURCE_INDEX_TV_SECAM,
	_SOURCE_INDEX_AVSV_NTSC,
	_SOURCE_INDEX_AVSV_PAL,
	_SOURCE_INDEX_AVSV_SECAM,
	_SOURCE_INDEX_YPBPR_SD,
	_SOURCE_INDEX_YPBPR_HD,
	_SOURCE_INDEX_HDMI_SD,
	_SOURCE_INDEX_HDMI_HD,
	_PCB_SOURCE_TV05_TOTAL_NUM
};

typedef enum _VIP_isPC_RGB444_Check_Mode {
	VIP_Disable_PC_RGB444 = 0,
	VIP_HDMI_PC_RGB444,
	VIP_HDMI_PC_YUV444,
	VIP_DP_PC_RGB444,
	VIP_DP_PC_YUV444,
	
	VIP_RGB444_MAX,
} VIP_isPC_RGB444_Check_Mode;

typedef enum _COLOR_CALLBACK_TYPE {

	COLOR_CALLBACK_GET_QUALITY_COEF = 0,
	COLOR_CALLBACK_GET_QUALITY_EXTEND,
	COLOR_CALLBACK_GET_QUALITY_EXTEND2,
	COLOR_CALLBACK_GET_QUALITY_EXTEND3,
	COLOR_CALLBACK_GET_QUALITY_EXTEND4,
	COLOR_CALLBACK_GET_QUALITY_3Dmode,
	COLOR_CALLBACK_GET_COLOR_DATA,
	COLOR_CALLBACK_GET_COLOR_FAC_MODE,
	COLOR_CALLBACK_GET_COLOR_TEMP_DATA,
	COLOR_CALLBACK_GET_COLOR_TEMP_DATA_TSB,
	COLOR_CALLBACK_GET_COLOR_TEMP_LEVEL,
	COLOR_CALLBACK_GET_PICTURE_MODE_DATA,
	COLOR_CALLBACK_GET_DNR_MODE,

	COLOR_CALLBACK_GET_CURPICMODE,
	COLOR_CALLBACK_GET_ICM_BY_TIMING_PICMODE,	/*Elsie 20131224*/

} COLOR_CALLBACK_TYPE;

typedef enum _MAGIC_FUNCTION_TYPE {
	MAGIC_IN_FUNCTION = 0,
	MAGIC_EXIT_FUNCTION,
} MAGIC_FUNCTION_TYPE;

typedef enum _SLR_MAGIC_TYPE {
	SLR_MAGIC_OFF,
	SLR_MAGIC_STILLDEMO,
	SLR_MAGIC_STILLDEMO_INVERSE,
	SLR_MAGIC_FULLSCREEN,
	SLR_MAGIC_DYNAMICDEMO,
	SLR_MAGIC_MOVE,
	SLR_MAGIC_MOVE_INVERSE,
	SLR_MAGIC_ZOOM,
	SLR_MAGIC_OPTIMIZE,
	SLR_MAGIC_ENHANCE,
	SLR_MAGIC_FULLGAMUT,					//full gamut
	SLR_MAGIC_LOCALDIMMING_MARQUEE,	//local dimming marquee
	SLR_MAGIC_LOCALDIMMING_TRACK,		//local dimming track
	SLR_MAGIC_MEMC_STILLDEMO_INVERSE,   // MEMC DEMO
	SLR_MAGIC_WCG_STILLDEMO_INVERSE
} SLR_MAGIC_TYPE;

typedef struct StructMagicPicInfo {
	unsigned char	magic_picture;
	unsigned char	magic_direction;
	unsigned char	magic_step;
	unsigned char	magic_parameter;
	short	magic_XStart;
	short	magic_YStart;
	short	magic_XEnd;
	short	magic_YEnd;
	short	magic_XStartForDynamic;
	short	magic_YStartForDynamic;
	short	magic_XEndForDynamic;
	short	magic_YEndForDynamic;
	int	magic_DISP_WID_OSD;
	unsigned char	pre_magic_pic;
} StructMagicPicInfo;

typedef enum {
	HDMI_CSC_DATA_RANGE_MODE_AUTO = 0x0,
	HDMI_CSC_DATA_RANGE_MODE_FULL,
	HDMI_CSC_DATA_RANGE_MODE_LIMITED,
} HDMI_CSC_DATA_RANGE_MODE_T;

typedef enum _VIP_CSC_DATA_RANGE_MODE_T{
	CSC_DATA_RANGE_MODE_AUTO = 0x0,
	CSC_DATA_RANGE_MODE_FULL,
	CSC_DATA_RANGE_MODE_LIMITED,

	VIP_CSC_DATA_RANGE_MODE_MAX,
} VIP_CSC_DATA_RANGE_MODE_T;

typedef enum{
	PQ_Data_Access_Peaking = 0,
	PQ_Data_Access_UZD,
	PQ_Data_Access_UZU,
	PQ_Data_Access_ICM,
	PQ_Data_Access_YUV2RGB,
	PQ_Data_Access_PanelCompensation,
} PQ_Data_Access_STATUS;

typedef enum {
	VPQ_ModuleTestMode_PQByPass = 0x0,
	VPQ_ModuleTestMode_HDRPQByPass = 0x1,
	VPQ_ModuleTestMode_MAX,
} VPQ_ModuleTestMode;

typedef struct S_VPQ_ModuleTestMode {
	unsigned char ModuleTestMode_en;
	VPQ_ModuleTestMode ModuleTestMode_Flag;
} S_VPQ_ModuleTestMode;

typedef enum _ModuleTest_VIP_QUALITY_Coef_ENUM {
	ModuleTest_VIP_QUALITY_SU_H = 0,
	ModuleTest_VIP_QUALITY_SU_V,
	ModuleTest_VIP_QUALITY_TwoStep_SU_Table,
	ModuleTest_VIP_QUALITY_SD_H_Table,
	ModuleTest_VIP_QUALITY_SD_V_Table,
	ModuleTest_VIP_QUALITY_SD_444To422,
	ModuleTest_VIP_QUALITY_SU_H_8TAP,
	ModuleTest_VIP_QUALITY_SU_V_6TAP,
	ModuleTest_VIP_QUALITY_SU_DIR,
	ModuleTest_VIP_QUALITY_SU_DIR_Weighting,
	ModuleTest_VIP_QUALITY_SU_C_H,
	ModuleTest_VIP_QUALITY_SU_C_V,
} ModuleTest_VIP_QUALITY_Coef_ENUM;


typedef struct _KADP_VPQ_DOBYBAKLIGHT_INI{
    int enable_global_dimming;
    int ott_delay_frame;
    int hdmi_delay_frame;
    int point0_p;
    int point0_a;
    int point25_p;
    int point25_a;
    int point50_p;
    int point50_a;
    int point75_p;
    int point75_a;
    int point100_p;
    int point100_a;
    int point_turn_p;
    int point_turn_a;
    int point_turn_ui;
    int ui_volue;
	int picmode_offset;
}KADP_VPQ_DOBYBAKLIGHT_INI;

typedef enum _VIP_APDEM_OD_Channel {
	APDEM_OD_Ch_R = 0,
	APDEM_OD_Ch_G,
	APDEM_OD_Ch_B,
	
	VIP_APDEM_OD_Ch_Max,
} VIP_APDEM_OD_Channel;

#define APDEM_Panal_ini_scale (100000)
typedef struct
{
	unsigned short un16_CIE_x;
	unsigned short	un16_CIE_y;
} VIP_PQ_COLOUR_GAMUT_POINT_T;

typedef struct
{
	VIP_PQ_COLOUR_GAMUT_POINT_T 	primary_r;
	VIP_PQ_COLOUR_GAMUT_POINT_T 	primary_g;
	VIP_PQ_COLOUR_GAMUT_POINT_T 	primary_b;
	VIP_PQ_COLOUR_GAMUT_POINT_T 	primary_wihte;
	unsigned short un16_max_luminance;
	unsigned short un16_min_luminance;
} VIP_PANEL_DATA_T; 	// *100000

#define VIP_DEM_10p_Gamma_Offset_idx_102_to_1024 1
#define VIP_DEM_10p_Gamma_Offset_Num 10
#define VIP_DEM_3D_LUT_Offset_Num 18	// 6 axis, 3 channel

// define panel ini address
#define VIP_DEM_PANEL_INI_Gamma_TBL_NUM 10
#define VIP_DEM_PANEL_INI_Gamma_IDX_NUM 1024
typedef struct _VIP_DEM_PANEL_INI_Gamma_TBL {
	unsigned int gamma_r[VIP_DEM_PANEL_INI_Gamma_IDX_NUM];
	unsigned int gamma_g[VIP_DEM_PANEL_INI_Gamma_IDX_NUM];
	unsigned int gamma_b[VIP_DEM_PANEL_INI_Gamma_IDX_NUM];
	
} VIP_DEM_PANEL_INI_Gamma_TBL;

typedef struct _VIP_DEM_PANEL_INI_Color_Chromaticity {
	unsigned int Panel_red_x;
	unsigned int Panel_red_y;
	unsigned int Panel_green_x;
	unsigned int Panel_green_y;
	unsigned int Panel_blue_x;
	unsigned int Panel_blue_y;
	unsigned int Panel_white_x;
	unsigned int Panel_white_y;
	unsigned int Panel_luma_max;
	unsigned int Panel_luma_min;	
} VIP_DEM_PANEL_INI_Color_Chromaticity;

typedef struct _VIP_DEM_PANEL_INI_TBL {	
	unsigned int gamma_bit_num;
	VIP_DEM_PANEL_INI_Gamma_TBL DEM_PANEL_INI_Gamma_TBL[VIP_DEM_PANEL_INI_Gamma_TBL_NUM];
	VIP_DEM_PANEL_INI_Color_Chromaticity DEM_PANEL_INI_Color_Chromaticity;
} VIP_DEM_PANEL_INI_TBL;

typedef enum _VIP_DEM_ARG_CTRL_ITEMS {
	DEM_command_Sent_NUM = 0,
	DEM_ARG_ICM_Idx,
	DEM_ARG_DCC_Idx,
	DEM_ARG_LC_Idx,
	DEM_ARG_LC_level,
	DEM_ARG_After_Filter_En,
	DEM_ARG_After_Filter_Val,	
	DEM_ARG_Blue_Stretch_level,
	DEM_ARG_Black_Extension_level,
	DEM_ARG_White_Extension_level,
	DEM_ARG_Data_Range,
	DEM_ARG_Gamma_exp_byOffset,
	DEM_ARG_10p_Gamma_Offset,
	DEM_ARG_Gamma_idx_set,
	DEM_ARG_Color_Space,
	DEM_ARG_D_3DLUT_Offset,
	DEM_ARG_Gamma_CURVE_TBL_PANEL_Chromaticity,
	DEM_ARG_LPF_Gain_ColorDataFac,
	DEM_ARG_OD_Gain,
	DEM_ARG_Dynamic_Black_Equalize_Gain,
	DEM_ARG_Shadow_detail_Gain,
	DEM_ARG_LowB_Gain,
	
	DEM_ARG_CTRL_ITEMS_Max,
} VIP_DEM_ARG_CTRL_ITEMS;

typedef enum _VIP_Color_Space_Target {
	Color_Space_Target_OFF = 0,
	Color_Space_Target_Auto,
	Color_Space_Target_sRGB,
	Color_Space_Target_DCIP3,
	Color_Space_Target_AdobeRGB,
	Color_Space_Target_BT2020,
	
	VIP_DEM_Color_Space_Apply_MAX,
} VIP_Color_Space_Target;

typedef enum _VIP_Gamma_Curve_exp {
	Gamma_exp_2p0 = 0,
	Gamma_exp_2p1,
	Gamma_exp_2p2,
	Gamma_exp_2p3,
	Gamma_exp_2p4,
	
	VIP_Gamma_exp_p_max,
} VIP_Gamma_Curve_exp;

typedef enum _VIP_QS_PQ_curMode{
	VIP_QS_PQ_cur_UserMode = 0,
	VIP_QS_PQ_cur_PcMode,
	VIP_QS_PQ_cur_GameMode,
	VIP_QS_PQ_cur__max,
} VIP_QS_PQ_curMode;

typedef struct _VIP_DEM_ColorDataFac {	
	unsigned int LPF_Gain_0;
	unsigned int LPF_Gain_25;
	unsigned int LPF_Gain_50;
	unsigned int LPF_Gain_75;
	unsigned int LPF_Gain_100;
} VIP_DEM_ColorDataFac;

#define VIP_DEM_Dynamic_Black_Equalize_TBL_NUM 11
#define VIP_DEM_Shadow_Detail_TBL_NUM 11
#define VIP_DEM_LowB_Gain_NUM 21

typedef struct _VIP_DEM_TBL {
	signed char LC_Level_Offset_Val[DCC_SELECT_MAX];
    unsigned char DEM_Black_Extent_Gain[DCC_SELECT_MAX];
	unsigned char DEM_White_Extent_Gain[DCC_SELECT_MAX];
	int DEM_10p_offset_TBL_R[VIP_DEM_10p_Gamma_Offset_Num+2];
	int DEM_10p_offset_TBL_G[VIP_DEM_10p_Gamma_Offset_Num+2];
	int DEM_10p_offset_TBL_B[VIP_DEM_10p_Gamma_Offset_Num+2];
	VIP_DEM_PANEL_INI_TBL PANEL_DATA_;
	unsigned int VPQ_COLORSPACE_INDEX[VIP_DEM_3D_LUT_Offset_Num];
	VIP_DEM_ColorDataFac ColorDataFac;
	short Color_Space_Target_Apply[VIP_DEM_Color_Space_Apply_MAX][3][3];
	unsigned char OverDriver_Gain[VIP_APDEM_OD_Ch_Max];
	unsigned int Dynamic_Black_Equalize_TBL[VIP_DEM_Dynamic_Black_Equalize_TBL_NUM];
	unsigned int Shadow_Detail_TBL[VIP_DEM_Shadow_Detail_TBL_NUM];
	unsigned int LowB_Gain_TBL[VIP_DEM_LowB_Gain_NUM];
	
} VIP_DEM_TBL;

typedef enum _VIP_APDEM_PTG_Pos_Mode {
	APDEM_PTG_Pos_byXY = 0,
	APDEM_PTG_Pos_Center,		
	APDEM_PTG_Pos_UpLeft,
	APDEM_PTG_Pos_UpRight,
	APDEM_PTG_Pos_BottomLeft,
	APDEM_PTG_Pos_BottomRight,

	VIP_APDEM_PTG_Pos_Info_MAX,
} VIP_APDEM_PTG_Pos_Mode;

typedef struct _VIP_APDEM_PTG_CTRL {
	int PTG_OnOff;
	int PTG_R_val;
	int PTG_G_val;
	int PTG_B_val;
	int PTG_Pos_Mode;
	int PTG_Size;	// 1-100%
	int x_sta;
	int y_sta;
	int x_end;
	int y_end;
	int mute_flag;
	int dis_flag;
} VIP_APDEM_PTG_CTRL;

#ifdef BUILD_QUICK_SHOW /*for quick_show mode */


typedef struct _PANEL_FILE_HEADER {
	// table offset of each table
	int	iSize;
	int	iYPPADCTableOffset;
	int	iVGAADCTableOffset;
	int	iColorDataOffset;
	int	iColorFacOffset;
	int	iPicModeOffset;
	int	iColorTempOffset;
	int	iVipTableOffset;
	int	iVipTableCustomOffset;
	int	iVipVerMiscOffset;
	int	iVersion;
	unsigned int iCRCValue;		// check value
} PANEL_FILE_HEADER;



typedef struct _PANEL_TABLE_STRUCT_HEADER {
	int				iSize;
	int				iTableArraySize;  // memory size
	int				iIndexTableSize;
	unsigned char	iTableType;		//PANEL_TABLE_TYPE
	unsigned char	iTableArrayLen; // array length
	unsigned char	iIndexTableLen;
	unsigned char	reserved;	// reserved to keep picmode tableLen
} PANEL_TABLE_STRUCT_HEADER;


typedef struct _PANEL_TABLE_STRUCT {
	PANEL_TABLE_STRUCT_HEADER	header;
	void*	pTableArray;
	void*	pIndexTable;	//VIP_TABLE_DATA_STRUCT or VIP_TABLE_DATA_STRUCT_EX (pic mode table)
} PANEL_TABLE_STRUCT;

typedef struct _VIP_VER_MISC_INFO //512 bytes
{
	char vipver[64];
	char CustomerName[64];
	char ProjectName[64];
	char ModelName[64];
	char PCBVersion[64];
	char reserved1[64];
	char reserved2[64];
	char Security[64];
} VIP_VER_MISC_INFO;

typedef struct _PANEL_FILE_HEADER_ICM {
	// table offset of each table
	int	iSize;
	int	iVipTableICMOffset;
	int	iVipICMVerMiscOffset;
	int	iVersion;
	unsigned int iCRCValue;		// check value
} PANEL_FILE_HEADER_ICM;

typedef struct _VIP_ICM_VER_MISC_INFO //512 bytes
{
	char vipver[64];
	char reserved[448];
} VIP_ICM_VER_MISC_INFO;


typedef struct _PANEL_TABLE_STRUCT_HEADER_ICM {
	int				iSize;
	int				iTableArraySize;  // memory size
	int				iIndexTableSize;
	unsigned char	iTableType;		//PANEL_TABLE_TYPE
	unsigned char	iTableArrayLen; // array length
	unsigned char	iIndexTableLen;
	unsigned char	reserved;	// reserved to keep picmode tableLen
} PANEL_TABLE_STRUCT_HEADER_ICM;


typedef struct _PANEL_TABLE_STRUCT_ICM {
	PANEL_TABLE_STRUCT_HEADER_ICM	header;
	void*	pTableArray;
	void*	pIndexTable;	//VIP_TABLE_DATA_STRUCT or VIP_TABLE_DATA_STRUCT_EX (pic mode table)
} PANEL_TABLE_STRUCT_ICM;


typedef struct _PANEL_FILE_HEADER_HDR{
    // table offset of each table
    int    iSize;
    int    iSLR_HDR_TABLEOffset;
    int    SLR_HDR_TABLE_length;
    int    iVipVerMiscOffset;
    int    VipVerMisc_length;
    int    iVersion;
    unsigned int iCRCValue;        // check value
} PANEL_FILE_HEADER_HDR;


typedef struct _PANEL_TABLE_STRUCT_HEADER_HDR {
    int              iSize;
    int              iTableArraySize;  // memory size
    int              iIndexTableSize;
    unsigned char    iTableType;        //PANEL_TABLE_TYPE
    unsigned char    iTableArrayLen; // array length
    unsigned char    iIndexTableLen;
    unsigned char    reserved;    // reserved to keep picmode tableLen
} PANEL_TABLE_STRUCT_HEADER_HDR;


typedef struct _PANEL_TABLE_STRUCT_HDR {
    PANEL_TABLE_STRUCT_HEADER_HDR    header;
    void*    pTableArray;
} PANEL_TABLE_STRUCT_HDR;


typedef struct _VIP_VER_MISC_INFO_HDR{ //512 bytes
	char vipver[64];
	char reserved[448];
} VIP_HDR_VER_MISC_INFO;


#endif

/*******************************************************************************
* Variable
******************************************************************************/
/*static unsigned char gExample = 100000;*/ /* 100K */



/*******************************************************************************
* Program
******************************************************************************/


/*o---------------------------------------------------------------------------o*/
/*o-------------Scalercolor.cpp------------------------------------o*/
/*o---------------------------------------------------------------------------o*/


/*==================== Definitions ================= */


typedef void *(*COLOR_CALLBACK_FUNC1)(VIP_SOURCE_TIMING source_timing);
typedef void *(*COLOR_CALLBACK_FUNC2)(unsigned int pcbSourceIndex, unsigned int timing, unsigned int param);
typedef int (*COLOR_CALLBACK_FUNC3)(void);
typedef unsigned char (*COLOR_CALLBACK_FUNC4)(unsigned char picMode);	/*Elsie 20131224*/
typedef void *(*COLOR_CALLBACK_FUNC5)(VIP_3DSOURCE_TIMING source_timing);

/*USER: Vincent_Lee  DATE: 2011/10/26  TODO: calcu sat by factory data*/
StructColorDataType *fwif_color_get_color_data(unsigned char src_idx, unsigned char timing);
StructColorDataFacModeType *fwif_color_get_color_fac_mode(unsigned char src_idx, unsigned char timing);
StructColorDataFacModeType *fwif_color_get_color_fac_mode_by_picture(unsigned char src_idx, unsigned char timing, unsigned char pic);

void *fwif_color_get_picture_mode_data(unsigned char src_idx, unsigned char value);
unsigned char fwif_color_get_picture_mode(unsigned char src_idx);


UINT8 fwif_vip_Get_Input_DataArrayIndex(UINT8 display);

unsigned char fwif_color_get_backlight(unsigned char src_idx);
void fwif_color_set_backlight(unsigned char src_idx, unsigned char value);
void fwif_color_set_backlight_TPV(unsigned char src_idx, unsigned char value);
unsigned char Scaler_Set_Gamma_level(unsigned char value);
void scaler_set_color_temp_level(SLR_COLORTEMP_LEVEL level);

unsigned char scaler_get_color_temp_level_type(unsigned char src_idx);
SLR_COLORTEMP_DATA *scaler_get_color_tempset(unsigned char src_idx, unsigned char timing);

unsigned char fwif_color_get_color_temp_level_type_using_colortmp(unsigned char colortmp);
void fwif_color_set_color_temp_level_type_using_colortmp(unsigned char colortmp, unsigned char value);

unsigned short fwif_color_get_color_temp_r_type_using_colortmp(unsigned char colortmp);
void fwif_color_set_color_temp_r_type_using_colortmp(unsigned char colortmp, unsigned short value);

unsigned short fwif_color_get_color_temp_g_type_using_colortmp(unsigned char colortmp);
void fwif_color_set_color_temp_g_type_using_colortmp(unsigned char colortmp, unsigned short value);

unsigned short fwif_color_get_color_temp_b_type_using_colortmp(unsigned char colortmp);
void fwif_color_set_color_temp_b_type_using_colortmp(unsigned char colortmp, unsigned short value);

unsigned short fwif_color_get_color_temp_r_offset_using_colortmp(unsigned char colortmp);
void fwif_color_set_color_temp_r_offset_using_colortmp(unsigned char colortmp, unsigned short value);
unsigned short fwif_color_get_color_temp_g_offset_using_colortmp(unsigned char colortmp);
void fwif_color_set_color_temp_g_offset_using_colortmp(unsigned char colortmp, unsigned short value);
unsigned short fwif_color_get_color_temp_b_offset_using_colortmp(unsigned char colortmp);
void fwif_color_set_color_temp_b_offset_using_colortmp(unsigned char colortmp, unsigned short value);

unsigned char Scaler_get_i_dcti(void);
void Scaler_set_i_dcti(unsigned char src_idx, unsigned char value);


unsigned char fwif_color_get_MpegNR_Level(unsigned char  src_idx);
void fwif_color_set_MpegNR_Level(unsigned char  src_idx, unsigned char  value);

unsigned char fwif_color_get_MpegNR_H(unsigned char  src_idx);
void fwif_color_set_MpegNR_H(unsigned char  src_idx, unsigned char  value);

unsigned char fwif_color_get_MpegNR_V(unsigned char  src_idx);
void fwif_color_set_MpegNR_V(unsigned char  src_idx, unsigned char  value);

unsigned char fwif_color_get_nrspatial(unsigned char src_idx);
void fwif_color_set_nrspatial(unsigned char src_idx, unsigned char value);

void fwif_color_set_Vertical_NR_driver(unsigned char src_idx, unsigned char value);
void fwif_color_set_HMCNR_driver(unsigned char src_idx, unsigned char value);

void fwif_color_set_HMCNR_blending_driver(unsigned char src_idx, unsigned char value);

unsigned char fwif_color_get_dcc_level(unsigned char src_idx);
void fwif_color_set_dcc_level(unsigned char src_idx, unsigned char value, unsigned char SceneChange);

void fwif_color_set_i_ccoring_driver(unsigned char src_idx, unsigned char value);

void fwif_color_set_d_ccoring_driver(unsigned char src_idx, unsigned char value);

unsigned char fwif_color_get_grid_remove(unsigned char src_idx);
void fwif_color_set_grid_remove(unsigned char src_idx, unsigned char value);

unsigned char fwif_color_get_conti_still_line(unsigned char src_idx);
void fwif_color_set_conti_still_line(unsigned char src_idx, unsigned char value);



unsigned char fwif_color_get_senselight(unsigned char src_idx);
void fwif_color_set_senselight(unsigned char src_idx, unsigned char value);

unsigned char fwif_color_get_spread_spectrum(unsigned char src_idx);
void fwif_color_set_spread_spectrum(unsigned char src_idx, unsigned char value);
unsigned char fwif_color_get_vip_debug_mode(unsigned char src_idx);
void fwif_color_set_vip_debug_mode(unsigned char src_idx, unsigned char value);

void *fwif_color_get_cur_picture_mode_data(unsigned char src_idx);

void fwif_set_ColorMatrix(bool value);


/*=========================================================*/


/*==================== Functions ================== */
/*void fwif_color_set_bg_color(unsigned char disp, unsigned char r, unsigned char g, unsigned char b);*/
/*void fwif_color_seticmhuesat(unsigned char display);*/

void fwif_color_setcontrastbrightness(unsigned char display);

/*rock_rau	20130227 mark not use function*/ /**/
/*void fwif_color_noisereduction_spatial(unsigned char channel, unsigned char level, unsigned char bNoiseReductionMode);*/
void fwif_color_video_quality_handler(void);
void fwif_color_video_3Dmode_quality_handler(void);/*young 3Dtable*/

void fwif_color_sync_rtnr(void);
void fwif_color_sync_dnr(void);
unsigned char fwif_color_reg_callback1(COLOR_CALLBACK_TYPE type, COLOR_CALLBACK_FUNC1 cbFunc);
unsigned char fwif_color_reg_callback2(COLOR_CALLBACK_TYPE type, COLOR_CALLBACK_FUNC2 cbFunc);
unsigned char fwif_color_reg_callback3(COLOR_CALLBACK_TYPE type, COLOR_CALLBACK_FUNC3 cbFunc);
unsigned char fwif_color_reg_callback4(COLOR_CALLBACK_TYPE type, COLOR_CALLBACK_FUNC4 cbFunc);	/*Elsie 20131224*/
unsigned char fwif_color_reg_callback5(COLOR_CALLBACK_TYPE type, COLOR_CALLBACK_FUNC5 cbFunc);/*young 3Dtable*/
/*char fwif_color_isUSBSource(void);*/

void fwif_ByPass_IDomainToMem(void); /* adc dump*/

int fwif_color_get_cur_picMode(void); /* return PICTURE_MODE*/
unsigned char fwif_vip_HDR10_check(unsigned char display, unsigned char input_src_type);
//unsigned char fwif_vip_DolbyHDR_check(unsigned char display, unsigned char input_src_type);
unsigned char fwif_vip_source_check(unsigned char display, unsigned char callmode);
unsigned char fwif_vip_3Dsource_check(unsigned char display, unsigned char callmode);/*young 3Dtable*/
VIP_QUALITY_Extend2_Coef *fwif_color_get_VIP_Extend2_CoefArray(unsigned char src_idx);
VIP_QUALITY_Extend3_Coef *fwif_color_get_VIP_Extend3_CoefArray(unsigned char src_idx);
VIP_QUALITY_3Dmode_Coef *fwif_color_get_VIP_3Dmode_CoefArray(unsigned char src_idx);/*young 3Dtable*/
VIP_QUALITY_Extend4_Coef *fwif_color_get_VIP_Extend4_CoefArray(unsigned char src_idx);

void  Scaler_icm_enhancement(unsigned char value);
void  Scaler_ChannelColourSet(signed char value);
void fwif_color_3DPQ_enhance(unsigned char enable);

/*===================================================================================== */ /*DCC CSFC 20140127*/
/*              DCC*/

/*o--- source table----o*/
unsigned char Scaler_GetDCC_Table(void);
void Scaler_SetDCC_Table(unsigned char value);

/*o--- picture mode----o*/
unsigned char Scaler_GetDCC_Mode(void);
unsigned char  Scaler_SetDCC_Mode(unsigned char value);
unsigned char Scaler_SetDCC_HAL_FreshContrast(unsigned char display , unsigned char uDCMode, UINT32 pstData);


/*o--- table cync. to video ---oo*/
unsigned char Scaler_Get_DCC_Boundary_Check_Table(void);
void Scaler_Set_DCC_Boundary_Check_Table(unsigned char value);
unsigned char Scaler_Get_DCC_Level_and_Blend_Coef_Table(void);
void Scaler_Set_DCC_Level_and_Blend_Coef_Table(unsigned char value);
void Scaler_Set_DCC_hist_adjust_table(unsigned char value);
unsigned char Scaler_Get_DCC_hist_adjust_table(void);
unsigned char Scaler_Get_DCC_database_Table(void);
void Scaler_Set_DCC_database_Table(unsigned char value);
unsigned char Scaler_Get_DCC_UserCurve_Table(void);
void Scaler_Set_DCC_UserCurve_Table(unsigned char value);
unsigned char Scaler_Get_DCC_AdaptCtrl_Level_Table(void);
void Scaler_Set_DCC_AdaptCtrl_Level_Table(unsigned char value);
unsigned char Scaler_Get_DCC_Picture_Mode_Weight(void);
void Scaler_Set_DCC_Picture_Mode_Weight(int value);

/*o---sync. level and coef base  to video ---o*/
unsigned char Scaler_GetDCL_W(void);
void Scaler_SetDCL_W(int value);
unsigned char Scaler_GetDCL_B(void);
void Scaler_SetDCC_B_extend(int value);
unsigned char Scaler_GetDCC_W_extend(void);
void Scaler_SetDCL_B(int value);
unsigned char Scaler_GetDCC_B_extend(void);
void Scaler_SetDCC_W_extend(int value);
unsigned char Scaler_Get_DCC_DelayTime(void);
void Scaler_Set_DCC_DelayTime(unsigned char flag);
unsigned char Scaler_Get_DCC_Step(void);
void Scaler_Set_DCC_Step(unsigned char flag);

/*o--- sync. table coef. that is non-adaptive table base API to video ---o*/
unsigned char Scaler_Get_Dcc_histogram_gain(unsigned char bin_num);
void Scaler_Set_Dcc_histogram_gain(unsigned char bin_num, unsigned char value, unsigned char SceneChange);
unsigned char Scaler_Get_Dcc_histogram_offset(unsigned char bin_num);
void Scaler_Set_Dcc_histogram_offset(unsigned char bin_num, unsigned char value, unsigned char SceneChange);
unsigned char Scaler_Get_Dcc_histogram_limit(unsigned char bin_num);
void Scaler_Set_Dcc_histogram_limit(unsigned char bin_num, unsigned char value, unsigned char SceneChange);

/*o--- non-adaptive table coef sync. when table change---o*/
void Scaler_set_dcc_sync_histogram_adjust_table(unsigned char src_idx, unsigned char Tabel_sel);/*for histogram adjus table*/

void Scaler_set_dcc_Sync_DCC_AP_Level(unsigned char src_idx, unsigned char Tabel_sel, unsigned char Mode_sel);/* for  DCC_Table change*/
void Scaler_set_dcc_Sync_DCC_Video_Driver_Level(unsigned char src_idx);/*for DCC DCC_Table change*/

UINT32 Scaler_get_DCC_AdapCtrl_Current_Mean(void);
UINT32 Scaler_get_DCC_AdapCtrl_Current_Dev(void);
VIP_DCC_AdaptCtrl_Level_Table *Scaler_get_DCC_AdapCtrl_Table(void);
unsigned char Scaler_get_DCC_AdapCtrl_DCC_Level(void);
BOOL Scaler_set_DCC_AdapCtrl_DCC_Level(UINT8 value);
unsigned char Scaler_get_DCC_AdapCtrl_Cubic_index(void);
BOOL Scaler_set_DCC_AdapCtrl_Cubic_index(UINT8 value);
unsigned char Scaler_get_DCC_AdapCtrl_Cubic_low(void);
BOOL Scaler_set_DCC_AdapCtrl_Cubic_low(UINT8 value);
unsigned char Scaler_get_DCC_AdapCtrl_Cubic_high(void);
BOOL Scaler_set_DCC_AdapCtrl_Cubic_high(UINT8 value);


/*o--- sync. table coef. that is adaptive by vide fw ---o*/

#if 0
/* mark those API, because control method change to tool control*/
unsigned char Scaler_GetDccLevel(void);
void Scaler_SetDccLevel(unsigned char value, unsigned char SceneChange);
unsigned char Scaler_GetDCC_Shigh(void);
void Scaler_SetDCC_Shigh(int value);
unsigned char Scaler_GetDCC_Slow(void);
void Scaler_SetDCC_Slow(int value);
unsigned char Scaler_GetDCC_Sindex(void);
void Scaler_SetDCC_Sindex(int value);
#endif

/*===================================================================================== */ /*DCC CSFC 20140128*/

/*===================================================================================== */ /*ConBri CSFC 20140128*/
unsigned char Scaler_GetFacBrightness(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100,
	unsigned char *retval_25, unsigned char *retval_75);
unsigned char Scaler_SetFacBrightness(unsigned char val_0, unsigned char val_50, unsigned char val_100,
	unsigned char val_25, unsigned char val_75);
unsigned char Scaler_GetFacContrast(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100,
	unsigned char *retval_25, unsigned char *retval_75);
unsigned char Scaler_SetFacContrast(unsigned char val_0, unsigned char val_50, unsigned char val_100,
	unsigned char val_25, unsigned char val_75);

/*===================================================================================== */ /*ConBri CSFC 20140128*/

/*======================================================== hue /sat*/
unsigned char Scaler_GetIntensity(void);
void Scaler_SetIntensity(unsigned char value);

unsigned char Scaler_GetFacSaturation(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100,
	unsigned char *retval_25, unsigned char *retval_75);
unsigned char Scaler_SetFacSaturation(unsigned char val_0, unsigned char val_50, unsigned char val_100,
	unsigned char val_25, unsigned char val_75);
unsigned char Scaler_GetFacHue(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100,
	unsigned char *retval_25, unsigned char *retval_75);
unsigned char Scaler_SetFacHue(unsigned char val_0, unsigned char val_50, unsigned char val_100,
	unsigned char val_25, unsigned char val_75);

/*===================================================================================== */ /*PictureMode CSFC 20140128*/
/*void Scaler_SetPictureInfo(SCALER_DISP_CHANNEL display, SLR_PICTURE_INFO infoList, unsigned char value);*/
/*unsigned char Scaler_GetPictureInfo(SCALER_DISP_CHANNEL display, SLR_PICTURE_INFO  infoList);*/

void Scaler_ResetPictureQuality(SCALER_DISP_CHANNEL display);
/*void *Scaler_get_cur_picture_mode_data(unsigned char src_idx);*/
/*void *Scaler_get_picture_mode_data(unsigned char src_idx, unsigned char value);*/

void Scaler_RefreshPictureMode(void);
unsigned char Scaler_VPQ_check_PC_RGB444(void);
/*===================================================================================== */ /*ColorTemp CSFC 20140128*/
unsigned short Scaler_get_color_temp_r_type_using_colortmp(unsigned char colortmp);
void Scaler_set_color_temp_r_type_using_colortmp(unsigned char colortmp, unsigned short value);

unsigned short Scaler_get_color_temp_g_type_using_colortmp(unsigned char colortmp);
void Scaler_set_color_temp_g_type_using_colortmp(unsigned char colortmp, unsigned short value);

unsigned short Scaler_get_color_temp_b_type_using_colortmp(unsigned char colortmp);
void Scaler_set_color_temp_b_type_using_colortmp(unsigned char colortmp, unsigned short value);

unsigned short Scaler_get_color_temp_r_offset_using_colortmp(unsigned char colortmp);
void Scaler_set_color_temp_r_offset_using_colortmp(unsigned char colortmp, unsigned short value);
unsigned short Scaler_get_color_temp_g_offset_using_colortmp(unsigned char colortmp);
void Scaler_set_color_temp_g_offset_using_colortmp(unsigned char colortmp, unsigned short value);
unsigned short Scaler_get_color_temp_b_offset_using_colortmp(unsigned char colortmp);
void Scaler_set_color_temp_b_offset_using_colortmp(unsigned char colortmp, unsigned short value);

unsigned short Scaler_get_color_temp_r_type(unsigned char src_idx);
void Scaler_set_color_temp_r_type(unsigned char src_idx, unsigned short value);

unsigned short Scaler_get_color_temp_g_type(unsigned char src_idx);
void Scaler_set_color_temp_g_type(unsigned char src_idx, unsigned short value);

unsigned short Scaler_get_color_temp_b_type(unsigned char src_idx);
void Scaler_set_color_temp_b_type(unsigned char src_idx, unsigned short value);

unsigned short Scaler_get_color_temp_r_offset(unsigned char src_idx);
void Scaler_set_color_temp_r_offset(unsigned char src_idx, unsigned short value);
unsigned short Scaler_get_color_temp_g_offset(unsigned char src_idx);
void Scaler_set_color_temp_g_offset(unsigned char src_idx, unsigned short value);
unsigned short Scaler_get_color_temp_b_offset(unsigned char src_idx);
void Scaler_set_color_temp_b_offset(unsigned char src_idx, unsigned short value);
void Scaler_Set_WCG_mode(unsigned int wcgmode);

/*===================================================================================== */ /*ColorTemp CSFC 20140128*/


/*O================================================*/
/*O================================================*/
/*O================================================*/
unsigned short Scaler_GetICMTable(void);/* csfanchiang, 20101101*/
void Scaler_SetICMTable(unsigned char value);/* csfanchiang, 20101101*/
void Scaler_set_ICM_table_by_VipTable(void);

void scaler_set_PQA_Input_table(unsigned char value);
void scaler_set_PQA_table(unsigned char value);
unsigned char scaler_get_PQA_Input_table(void);
unsigned char scaler_get_PQA_table(void);

unsigned char  Scaler_GetDNR_table(void);
void Scaler_SetDNR(unsigned char value);
unsigned char Scaler_GetDNR(void);
void Scaler_SetMPEGNR(unsigned char value, unsigned char calledByOSD);
unsigned char Scaler_GetMPEGNR(void);
DRV_RTNR_General_ctrl *Scaler_GetRTNR_General(void);
void Scaler_SetRTNR_Y(unsigned char value);
unsigned char Scaler_GetRTNR_Y(void);
void Scaler_SetRTNR_C(unsigned char value);
unsigned char Scaler_GetRTNR_C(void);
void Scaler_SetNRSpatialY(unsigned char value);
unsigned char Scaler_GetNRSpatialY(void);
void Scaler_SetNRSpatialC(unsigned char value);
unsigned char Scaler_GetNRSpatialC(void);
void Scaler_SetMpegNR_H(unsigned char value);
unsigned char Scaler_GetMpegNR_H(void);
void Scaler_SetMpegNR_V(unsigned char value);
unsigned char Scaler_GetMpegNR_V(void);
void Scaler_SetNRTemporal(unsigned char value);
unsigned char Scaler_GetNRTemporal(void);

void Scaler_OPC_Auto_Backlight_Adjust(void);

void Scaler_SetFacPatternColor(unsigned char val);

unsigned char Scaler_GetFacPatternColor(void);

void Scaler_SetAutoMAFlag(unsigned char flag);


/*o===========================================================o*/
/*o=================== Enginner Menu Start ===================o*/
/*o===========================================================o*/

unsigned char Scaler_GetScaleDOWNH(void);
void Scaler_SetScaleDOWNH(unsigned char value);					/*range 0~17*/
unsigned char Scaler_GetScaleDOWNV(void);
void Scaler_SetScaleDOWNV(unsigned char value);					/*range 0~17*/

unsigned char Scaler_GetScaleDOWNVTable(void);
void Scaler_SetScaleDOWNVTable(unsigned char value);				/*range 0~7*/
unsigned short Scaler_GetH8Scaling(void);
void Scaler_SetH8Scaling(unsigned char value);
unsigned char Scaler_GetPacificNewIntra(void);
void Scaler_SetPacificNewIntra(unsigned char value);
unsigned short Scaler_GetICcoring(void);
void Scaler_SetICcoring(unsigned char value);
unsigned short Scaler_GetDCcoring(void);
void Scaler_SetDCcoring(unsigned char value);
unsigned char Scaler_GetVertical_NR(void);
void Scaler_SetVertical_NR(unsigned char value);
unsigned char Scaler_GetHMCNR(void);
void Scaler_SetHMCNR(unsigned char value);
unsigned char Scaler_GetHMCNR_blending(void);
void Scaler_SetHMCNR_blending(unsigned char value);
unsigned char Scaler_GetMBPeaking(void);
void Scaler_SetMBPeaking(unsigned char value);					/*range 0~40*/
unsigned char Scaler_GetMBSUPeaking(void);
void Scaler_SetMBSUPeaking(unsigned char value);					/*range 0~40*/
unsigned short Scaler_GetSharpnessVerticalGain(void);
void Scaler_SetSharpnessVerticalGain(unsigned char value);
unsigned short  Scaler_Getharpness2Dpk_Seg(void);
void Scaler_SetSharpness2Dpk_Seg(unsigned char value);
unsigned short  Scaler_GetSharpness2Dpk_Tex_Detect(void);
void Scaler_SetSharpness2Dpk_Tex_Detect(unsigned char value);
unsigned char Scaler_GetMADI_BTR(void);
void Scaler_SetMADI_BTR(unsigned char value);
unsigned char Scaler_GetMADI_BER(void);
void Scaler_SetMADI_BER(unsigned char value);
unsigned char Scaler_GetGridRemove(void);
void Scaler_SetGridRemove(unsigned char value);
unsigned char Scaler_GetContiStillLine(void);
void Scaler_SetContiStillLine(unsigned char value);

unsigned char Scaler_GetAutoMA1Flag(void);
void Scaler_SetAutoMA1Flag(unsigned char flag);
unsigned char Scaler_GetAutoMA2Flag(void);
void Scaler_SetAutoMA2Flag(unsigned char flag);
unsigned char Scaler_GetAutoMA3Flag(void);
void Scaler_SetAutoMA3Flag(unsigned char flag);
unsigned char Scaler_GetAutoMA4Flag(void);
void Scaler_SetAutoMA4Flag(unsigned char flag);
unsigned char Scaler_GetAutoMA5Flag(void);
void Scaler_SetAutoMA5Flag(unsigned char flag);
unsigned char Scaler_GetAutoMA6Flag(void);
void Scaler_SetAutoMA6Flag(unsigned char flag);
unsigned char Scaler_GetAutoMAFlag(void);
void Scaler_SetAutoMA_Flag(void);

/*o===========================================================o*/
/*o=================== Enginner Menu End =====================o*/
/*o===========================================================o*/


/*o===========================================================o*/
/*o==================== OSD MENU Start =======================o*/
/*o===========================================================o*/

unsigned char Scaler_GetFacSharpness(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100,
	unsigned char *retval_25, unsigned char *retval_75);
unsigned char Scaler_SetFacSharpness(unsigned char val_0, unsigned char val_50, unsigned char val_100,
	unsigned char val_25, unsigned char val_75);
unsigned char Scaler_GetFacBacklight(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100,
	unsigned char *retval_25, unsigned char *retval_75);
unsigned char Scaler_SetFacBacklight(unsigned char val_0, unsigned char val_50, unsigned char val_100,
	unsigned char val_25, unsigned char val_75);

unsigned char Scaler_APDEM_En_Access(unsigned char en, unsigned char isSetFlag);
unsigned char Scaler_APDEM_Arg_Access(VIP_DEM_ARG_CTRL_ITEMS ID, unsigned char val, unsigned char isSetFlag);
int Scaler_set_APDEM(VIP_DEM_ARG_CTRL_ITEMS ID, void *DEM_arg);
unsigned char Scaler_APDEM_OsdMapToRegValue(VIP_DEM_ARG_CTRL_ITEMS ID, unsigned char value);
char Scaler_APDEM_PTG_set(VIP_APDEM_PTG_CTRL *PTG_CTRL);

/*o===========================================================o*/
/*o==================== OSD MENU End = =======================o*/
/*o===========================================================o*/

/* for load quality handle*/

void Scaler_Get_Display_info(unsigned char *display, unsigned char *src_idx);

void Scaler_VIP_IP_init(unsigned char Vip_source);

void Scaler_VIP_PQ_QS_init(void);

void Scaler_VIP_MEMC_init(unsigned char Vip_source);

void Scaler_VIP_set_Project_ID(VIP_Customer_Project_ID_ENUM Project_ID);

unsigned char Scaler_VIP_get_Project_ID(void);

unsigned char Scaler_GetMA_Chroma_Error(void);

void Scaler_SetMA_Chroma_Error(unsigned char value);

unsigned char Scaler_GetMA_Chroma_Error_En_From_vipTable(void);

unsigned char Scaler_GetMADI_HMC(void);

void Scaler_SetMADI_HMC(unsigned char value);

unsigned char Scaler_GetMADI_HME(void);

void Scaler_SetMADI_HME(unsigned char value);

unsigned char Scaler_GetMADI_PAN(void);

void Scaler_SetMADI_PAN(unsigned char value);

unsigned char Scaler_GetNew_UVC(void);

void Scaler_SetNew_UVC(unsigned char value);

unsigned char Scaler_GetDLti(void);

void Scaler_SetDLti(unsigned char value);						/*range 0~10*/

unsigned char Scaler_GetDCti(void);

void Scaler_SetDCti(unsigned char value);						/*range 0~10*/

unsigned char Scaler_GetIDLti(void);

void Scaler_SetIDLti(unsigned char value);

unsigned char Scaler_GetIDCti(void);

void Scaler_SetIDCti(unsigned char value);

unsigned char Scaler_GetVDCti(void);

void Scaler_SetVDCti(unsigned char value);

unsigned char Scaler_GetVDCti_LPF(void);
void Scaler_SetVDCti_LPF(unsigned char value);

unsigned char Scaler_GetColor_UV_Delay_Enable(void);

void Scaler_SetColor_UV_Delay_Enable(unsigned char on_off);

unsigned char Scaler_GetColor_UV_Delay(void);

void Scaler_SetColor_UV_Delay(unsigned char value);
void Scaler_SetColor_UV_Delay_TOP(unsigned char value);



unsigned char Scaler_GetFilm_Mode(void);

void Scaler_SetFilm_Mode(unsigned char value);

void Scaler_SetFilmonoff(unsigned char value);

void Scaler_SetFilm_Setting(void);


unsigned char Scaler_GetIntra(void);

void Scaler_SetIntra(unsigned char value);	/*range 0~11*/

unsigned char Scaler_GetMADI(void);

void Scaler_SetMADI(unsigned char value);
void Scaler_SetMADI_INIT(void);

unsigned char Scaler_GetTNRXC_Ctrl(void);
void Scaler_SetTNRXC_Ctrl(unsigned char value);

unsigned char Scaler_GetTNRXC_MK2(void);
void Scaler_SetTNRXC_MK2(unsigned char value);

void Scaler_SetTNRXC(unsigned char value);

void Scaler_SetDataFormatHandler(void);

unsigned char Scaler_get_YUV2RGB(void);

void Scaler_set_YUV2RGB(unsigned char which_table, unsigned char drvSkip_Flag_coefByY, unsigned char drvSkip_Flag_uvOffset);

void Scaler_module_vdc_SetConBriHueSat_Table(unsigned char value);
unsigned char Scaler_module_vdc_GetConBriHueSat_Table(void);



unsigned char Scaler_get_ICM_table(void);

void Scaler_set_ICM_table(unsigned char value, unsigned char drvSkip_Flag);

unsigned char Scaler_GetGamma(void);

void Scaler_SetGamma(unsigned char value);/*range 0~3*/

unsigned char Scaler_GetSindex(void);

unsigned char Scaler_SetSindex(unsigned short value);/*range 0~32*/

unsigned char Scaler_GetSlow(void);

unsigned char Scaler_SetSlow(unsigned short value);	/*range 0~20*/

unsigned char Scaler_GetShigh(void);

unsigned char Scaler_SetShigh(unsigned short value);/*range 0~20*/

void Scaler_SetGamma_for_Handler(unsigned char gamma,
															unsigned char s_gamma_index,
															unsigned char s_gamma_low,
															unsigned char s_gamma_high, unsigned char drvSkip_Flag);
void Scaler_SetGamma_for_ddomainISR_Handler(unsigned char gamma,
															unsigned char s_gamma_index,
															unsigned char s_gamma_low,
															unsigned char s_gamma_high, unsigned char drvSkip_Flag);

unsigned char Scaler_Get_blue_stretch(void);

void Scaler_set_blue_stretch(unsigned char table);

unsigned short Scaler_GetSharpnessTable(void);

void Scaler_SetSharpnessTable(unsigned char value);

unsigned short Scaler_GetCDSTable(void);

void Scaler_SetCDSTable(unsigned char value);

unsigned short Scaler_GetEGSM_postshp_level(void);

void Scaler_SetEGSM_postshp_level(unsigned char value);

unsigned char Scaler_GetEMF_Mk2(void);

void Scaler_SetEMF_Mk2(unsigned char value);

unsigned char Scaler_Get_DCC_Color_Independent_Table(void);

void Scaler_Set_DCC_Color_Independent_Table(unsigned char value);

unsigned char Scaler_Get_DCC_chroma_compensation_Table(void);

void Scaler_Set_DCC_chroma_compensation_Table(unsigned char value);

unsigned char Scaler_get_Adaptive_Gamma(void);

//void Scaler_set_Adaptive_Gamma(unsigned char value);

/*unsigned char Scaler_Get_DCC_Style(void);*/

/*void Scaler_Set_DCC_Style(unsigned char value, unsigned char calledByOSD);*/

unsigned char Scaler_GetRTNR_Noise_Measure(void);

void Scaler_SetRTNR_Noise_Measure(unsigned char value);

unsigned char Scaler_GetScaleUPH(void);

void Scaler_SetScaleUPH(unsigned char value);

unsigned char Scaler_GetScaleUPH_12tap(void);

void Scaler_SetScaleUPH_12tap(unsigned char value);

unsigned char Scaler_GetScaleUPV(void);

void Scaler_SetScaleUPV(unsigned char value);
void Scaler_SetScaleUP_YCHV(unsigned char HY, unsigned char VY, unsigned char HC, unsigned char VC);
void Scaler_GetScaleUP_YCHV(unsigned char *pHY,unsigned char *pVY,unsigned char *pHC,unsigned char *pVC);

unsigned char Scaler_GetScaleUPDir(void);

void Scaler_SetScaleUPDir(unsigned char value);/*range 0~19*/

unsigned char Scaler_GetScaleUPDir_Weighting(void);

void Scaler_SetScaleUPDir_Weighting(unsigned char value);/*range 0~9*/

unsigned char Scaler_GetScalePK(void);

void Scaler_SetScalePK(unsigned char value);					/*range 0~10*/

unsigned short Scaler_GetSUPKType(void);

void Scaler_SetSUPKType(unsigned char value);

unsigned short Scaler_GetUnsharpMask(void);

void Scaler_SetUnsharpMask(unsigned char value);

unsigned char Scaler_getiESM(void);

void Scaler_setiESM(unsigned char value);

unsigned char Scaler_GetScaleDOWNHTable(void);

void Scaler_SetScaleDOWNHTable(unsigned char value);/*range 0~7*/

unsigned char Scaler_GetScaleDOWN444To422(void);

void Scaler_SetScaleDOWN444To422(unsigned char value);/*range 0~17*/

unsigned char Scaler_GetScaleUPV_8tap(void);
#ifdef CONFIG_HW_SUPPORT_TWOSTEP_SCALEUP
void Scaler_SetScaleUP_TwoStep(unsigned char value);
#endif //CONFIG_HW_SUPPORT_TWOSTEP_SCALEUP
void Scaler_SetScaleUPV_8tap(unsigned char value);

unsigned char Scaler_GetDiSmd(void);

void Scaler_SetDiSmd(unsigned char value);

void Scaler_SetScaleOSD_Sharpness(unsigned char value);

unsigned char Scaler_Debug_Get_UserCurveDCC_Curve_Select(void);
unsigned char Scaler_Debug_Get_HIST_MEAN_VALUE(void);
unsigned char Scaler_Debug_Get_Pure_Color_Flag(void);
unsigned char Scaler_Debug_Get_Skin_Tone_Found_Flag(void);
unsigned char Scaler_Debug_Get_UserCurveDCC_Curve_Weighting(void);
unsigned char Scaler_Debug_Get_UserCurveDCC_Curve_Weighting_Skin(void);

void Scaler_Debug_Set_UserCurveDCC_Curve_Select(unsigned char value);
void Scaler_Debug_Set_UserCurveDCC_Curve_Apply(unsigned char value);
void Scaler_Debug_Set_UserCurveDCC_Curve_Weighting(unsigned char value);
void Scaler_Debug_Set_UserCurveDCC_Curve_Weighting_Skin(unsigned char value);

void Scaler_set_pq_dir_path(char *path, char *patch_path);
unsigned char Scaler_get_average_luma(unsigned char *val);
unsigned char Scaler_get_luma_level_distribution(unsigned short *luma_distribution, unsigned char size);
unsigned char Scaler_get_Y_Total_Hist_Cnt_distribution(UINT8 MODE, UINT8 size, unsigned int *Y_Hist_distr_cnt);

void Scaler_SetDcrMode(unsigned char value);
unsigned char Scaler_GetDcrMode(void);
unsigned char Scaler_SetOSDBacklight2VIP(unsigned char value);
void Scaler_set_DCR_Enable(UINT8 value);
UINT8 Scaler_get_DCR_Enable(void);
void Scaler_Get_color_access_TVStatus_DisplayInfo(unsigned int *VIP_Access_Memory_DisplayInfo_Table);

#ifdef LocalDimming
void Scaler_SetLocalDimmingEnable(unsigned char enable);
void Scaler_SetLocalDimmingTable(unsigned char TableIdx);
void Scaler_fwif_color_set_LocalDimming_table(unsigned char src_idx, unsigned char TableIdx);
#endif

void Scaler_SetLocalContrastEnable(unsigned char enable);
unsigned char Scaler_GetLocalContrastEnable(void);
void Scaler_SetLocalContrastTable(unsigned char TableIdx);
unsigned char Scaler_GetLocalContrastTable(void);

void Scaler_fwif_color_set_LocalContrast_table(unsigned char src_idx, unsigned char TableIdx);

int Scaler_GetRGBPixelInfo(RGB_pixel_info *data);

void Scaler_Init_ColorLib_customer_share_memory_struct(void);

void Scaler_color_handler(void);

void fwif_color_video_quality_handler_customization(void);
void Scaler_SetPictureMode(unsigned char level);

unsigned char Scaler_GetPictureMode(void);

int Scaler_GetHDR3DLUT_Offset(void);

void Scale_SetHDR3DLUT_Offset(int offset);

unsigned char Scaler_GetContrast(void);

void Scaler_SetContrast(unsigned char value);				/*range 0~100*/

unsigned char Scaler_GetBrightness(void);

void Scaler_SetBrightness(unsigned char value);				/*range 0~100*/

unsigned char Scaler_GetSaturation(void);

void Scaler_SetSaturation(unsigned char value);    /*range 0~100*/

unsigned char Scaler_GetHue(void);

void Scaler_SetHue(unsigned char value);        /*range 0~100*/

unsigned char Scaler_GetSharpness(void);
unsigned char Scaler_GetSharpness_fromAP(void);

void Scaler_SetSharpness(unsigned char value);

void Scaler_SetCDSTable_by_Sharpness(unsigned char value);

unsigned char Scaler_SetBackLight(unsigned char value);

unsigned char Scaler_GetBacklight(void);

SLR_COLORTEMP_LEVEL Scaler_GetColorTemp(void);

void Scaler_SetColorTemp(SLR_COLORTEMP_LEVEL level);

unsigned char Scaler_Get_ColorMapping_3x3_Table(void);

void Scaler_Set_ColorMapping_3x3_Table(unsigned char table_idx);

void Scaler_Set_ColorMapping_By_ColorFormat(unsigned char channel, unsigned char nSD_HD);

unsigned char Scaler_PipGetSubBrightness(void);

/**
 * Scaler_PipSetSubBrightness
 * set the brightness level of sub channel
 *
 * @param { value:0~100}
 * @return { }
 *
 */
void Scaler_PipSetSubBrightness(unsigned char value);

/**
 * Scaler_PipGetSubContrast
 * set the contrast level of sub channel
 *
 * @param { }
 * @return { TRUE/FALSE }
 *
 */
unsigned char Scaler_PipGetSubContrast(void);

/**
 * Scaler_PipSetSubContrast
 * set the contrast level of sub channel
 *
 * @param { value:0~100  }
 * @return { }
 *
 */
void Scaler_PipSetSubContrast(unsigned char value);
void Scaler_Load_VipTable_Custom(void *ptr);

UINT8 Scaler_Get_DI_Force_2D_Enable(void);
void Scaler_Set_DI_Force_2D_Enable(UINT8 ucEnable);
UINT8 Scaler_Enter_DI_Force_2D_Condition(UINT8 ucDisplay);

unsigned short Scaler_GetMAG_H_Region_Start(void);
unsigned short Scaler_GetMAG_H_Region_End(void);
unsigned short Scaler_GetMAG_H_Region_Width(void);
unsigned short Scaler_GetMAG_V_Region_Start(void);
unsigned short Scaler_GetMAG_V_Region_End(void);
unsigned short Scaler_GetMAG_V_Region_Len(void);

/*=======================for magic picture======================*/
void Scaler_SetMagicPicture(SLR_MAGIC_TYPE level);
void mScalerSetMAG(void);
void Scaler_MagicSaturation(unsigned char display, unsigned char MagicMode);
void mScalerMagicPictureCtrlForDynamic(void);
void Scaler_MoveMagicPictureForStill(unsigned int timerId, void *msg);
/*=======================for magic picture end======================*/

unsigned char Scaler_getDI_SMD_en(unsigned char source);

void Scaler_color_copy_scalerinfo_to_vip(unsigned char display);
void Scaler_color_print_scalerinfo(void);

void Scaler_SetFacPatternColor(unsigned char val);

void Scaler_SetMagicPicture_demo_reset(unsigned char level);
unsigned char Scaler_GetMagicPicture_demo_reset(void);

//for VIP table check
void Scaler_checkVIPTable(void);

void Scaler_Set3dLUT(UINT8 TableIdx);
unsigned char Scaler_Get3dLUT(void);

unsigned char fwif_vip_increase_DI_water_lv(void);
//void Scaler_LGE_HDR10_Enable(unsigned char enable);
void Scaler_Set_HDR_YUV2RGB(unsigned char HDR_mode, unsigned char color_format);
void Scaler_SetHDR_BlackLevel(unsigned char blk_lv);

void drvif_TV006_HDR10_init(void);
void Scaler_set_HDR10_Enable(unsigned char* enable);
void Scaler_LGE_ONEKEY_HDR10_Enable(unsigned char enable);
void Scaler_LGE_ONEKEY_BBC_Enable(unsigned char enable);
void drvif_TV006_SET_HDR_mode(UINT32* modeinfo);
#ifdef CONFIG_HDR_SDR_SEAMLESS
void drvif_TV006_SET_HDR_mode_Seamless(UINT32* modeinfo);
void drvif_TV006_SET_HDR_mode_Seamless_hdmi_position(UINT32* modeinfo);
void drvif_TV006_SET_HDR_mode_Seamless_hdmi_finish(UINT32* modeinfo);
void Scaler_LGE_HDR10_Enable_Seamless(unsigned char enable);
void Scaler_LGE_HDR10_Enable_Seamless_hdmi_position(unsigned char enable);
void Scaler_LGE_HDR10_Enable_Seamless_hdmi_finish(unsigned char enable);
void Scaler_SetDataFormatHandle_Seamless(void);
#endif
void Scaler_LGE_ONEKEY_SDR2HDR_Enable(unsigned char enable);


void Scaler_color_set_HDR_DEMO_Mode_ON(void);
void Scaler_color_set_HDR_DEMO_Mode_OFF(void);

void Scaler_color_set_m_nowSource(unsigned char value);
unsigned char Scaler_color_get_m_nowSource(void);

void vpq_PQ_setting_check(char *item_name, unsigned int *PQ_check_error);
void vpq_PQ_check_timming(char *item_name, unsigned int *PQ_check_error);

#ifdef CONFIG_HW_SUPPORT_I_DE_XC //mac6 removed de_xcxl
unsigned char  Scaler_Set_I_De_XC(unsigned char value);
#endif
unsigned char Scaler_Get_I_De_XC(void);

unsigned char  Scaler_Set_I_De_Contour(unsigned char value);
unsigned char Scaler_Get_I_De_Contour(void);

void  Scaler_Set_BOE_RGBW(int w_rate);
unsigned char  Scaler_Set_SLD(unsigned char value);
unsigned char Scaler_Get_SLD(void);

void Scaler_color_colorwrite_Output_InvOutput_gamma(unsigned char value);
void set_HDR10_DM_setting(void);


void Scaler_Share_Memory_Access_VIP_TABLE_Custom_Struct(unsigned long arg);

unsigned char Scaler_color_LC_DemoStep(unsigned char mode);
void Scaler_color_set_LC_DebugMode(unsigned char DebugMode);
char Scaler_color_get_LC_DebugMode(void);
unsigned char Scaler_color_LC_DemoSplit_Screen(unsigned char mode);

#ifdef CONFIG_HDR_SDR_SEAMLESS
void Scaler_hdr_setting_SEAMLESS(unsigned char hdr_enable);
void Scaler_hdr_setting_SEAMLESS_hdmi_position(unsigned char hdr_enable, unsigned char hdr_type);
void Scaler_hdr_setting_SEAMLESS_hdmi_finish(unsigned char hdr_enable, unsigned char hdr_type);
#endif
unsigned char*  Scaler_color_Access_HDR_AutoRun_TBL_Flag(unsigned char isSetFlag, unsigned char *HDR_AutoRun_TBL_Flag);
void Scaler_color_set_HDR_AutoRun_TBL(void);
void fwif_color_vsc_to_scaler_src_mapping(unsigned char vsc_src, unsigned char *p_scaler_src_type, unsigned char *p_scaler_src_form);
unsigned char fwif_color_get_cur_input_timing_mode(unsigned char display);

char Scaler_color_Set_PQ_ByPass_Lv(unsigned char table_idx);
char Scaler_color_Get_PQ_ByPass_Lv(void);
void drvif_TV006_SET_HDR_mode_off(void);
void Scaler_LGE_HDR10_Enable_newflow(unsigned char enable);

void Scaler_color_Set_EOTF_OETF_Table(void);
void EOTF_1024point(unsigned short maxlevel, VIP_HDR10_EOTF_ByLuminace_TBL* EOTF_table, VIP_HDR10_OETF_ByLuminace_TBL* OETF_table, unsigned int* EOTFyi_curve);
char Scaler_color_Get_HDR10_EOTF_ByMaxCLL_ByPanelLuminance(unsigned short forceIndex);
void OETF_inner_64point(int row, int col, unsigned short maxlevel, unsigned int OETFyi_curve[6][67], unsigned int* OETFyi_inner, unsigned int* shift_bit, unsigned short* _3dlut);
char Scaler_color_Get_HDR10_OETF_ByMaxCLL_ByPanelLuminance(unsigned short forceIndex);
void Scaler_color_Get_HDR10_EOTF_OETF_ByMaxCLL_ByPanelLuminance_for_task(void);
void Scaler_color_set_HDR_Type(unsigned char HDR_Type);

void Scaler_color_Set_HDR_AutoRun(unsigned char nEnable);
unsigned char Scaler_color_Get_HDR_AutoRun(void);
void Scaler_color_HDRIP_AutoRun(unsigned char HDR_Type);
void Scaler_color_HDRInfo_Update(UINT32* modeinfo);
void Scaler_color_copy_HDR_table_from_AP(SLR_HDR_TABLE* ptr_HDR_bin);
void Scaler_color_copy_HDR_table_panel_luminance_from_AP(void);
void Scaler_color_copy_HDR_table_oetf_setting_ctrl_from_AP(unsigned short oetf_setting_ctrl_from_ini);
void Scaler_color_copy_HDR_table_oetf_curve_ctrl_from_AP(unsigned short oetf_curve_index_from_ini);
void Scaler_color_Set_DCCProcess(unsigned char nEnable);
unsigned char Scaler_color_Get_DCCProcess(void);

void Scaler_color_set_pq_src_idx(unsigned char src_idx);
unsigned char Scaler_color_get_pq_src_idx(void);

HDMI_CSC_DATA_RANGE_MODE_T Scaler_GetHDMI_CSC_DataRange_Mode(void);
void Scaler_SetHDMI_CSC_DataRange_Mode(HDMI_CSC_DATA_RANGE_MODE_T value);
void Scaler_recheck_sharp_nr_table(void);
unsigned char Scaler_Get_CinemaMode_PQ(void);
void Scaler_Set_CinemaMode_PQ(unsigned char mode);
bool Scaler_Init_OD(void);
unsigned char Scaler_Get_OD_Bits(void);
unsigned char Scaler_Set_OD_Bits(unsigned char bits);
unsigned char Scaler_Get_OD_Enable(void);
unsigned char Scaler_OD_Test(unsigned char en);
char Scaler_Set_DeMura_En(unsigned char en);
unsigned char Scaler_Get_DeMura_En(void);
char Scaler_Set_DeMura_Table(DRV_DeMura_CTRL_TBL * pCtrl_tbl, unsigned char *pDeMura_tbl);
void Scaler_Set_PicMode_VIP_src(unsigned char src);
void Scaler_Check_VIP_src_resync_flag(void);
unsigned char Scaler_Get_VIP_src_resync_flag(void);
unsigned char Scaler_SetBackLightMappingRange(unsigned int uiMax,unsigned int uiMin,unsigned int actMax,unsigned int actMin,unsigned char increaseMode);
unsigned char Scaler_GetBackLightMappingRange(unsigned int *uiMax,unsigned int *uiMin,unsigned int *actMax,unsigned int *actMin, unsigned char *increaseMode);
unsigned char Scaler_Get_DCR_Backlight(void);
unsigned char  Scaler_Set_VIP_HANDLER_RESET(void);

unsigned char Scaler_Set_OD_Bit_By_XML(void);
unsigned char Scaler_Set_OutGamma_SCurve(unsigned char enable);
void Scaler_Set_PQ_table_bin_En(unsigned char en);
unsigned char Scaler_Get_PQ_table_bin_En(void);
char Scaler_Get_Data_Access(unsigned int type, unsigned int *index, unsigned int *YUV);
void Scaler_color_Dither_Table_Ctrl(void);

unsigned char Scaler_VPQ_Set_ModuleTestMode(unsigned char);
unsigned char  Scaler_Set_VIP_Disable_PQ(VPQ_ModuleTestMode ModuleTestMode, unsigned char enable);

char Scaler_Set_DI_SNR_Borrow_Sram_Mode(VIP_SNR_DI_Sram_Borrow_Mode mode);
char Scaler_Get_DI_SNR_Borrow_Sram_Mode(void);

void Scaler_color_ModuleTest_HDR_FixTable(void);
unsigned char Scaler_color_access_RTNR_H_Rotate_Flag(unsigned char Val, unsigned char isSetFlag);
void Scaler_color_set_RTNR_H_Rotate(unsigned char display, unsigned char En);
void Scaler_color_decide_RTNR_H_Rotate(unsigned char display);
unsigned char  Scaler_Set_PQ_ModuleTest(VPQ_ModuleTestMode ModuleTestMode);
S_VPQ_ModuleTestMode *Scaler_Get_PQ_ModuleTest(void);

void Scaler_color_get_LC_Blight_dat(unsigned int *ptr_out, unsigned int bufferSz);
void Scaler_color_get_LC_His_dat(unsigned int *plc_out);
void Scaler_color_set_DDomain3DLUT_gamma_by_colorspace(UINT8 colorspace_index);
void Scaler_color_vpq_set_dolbylight_init(KADP_VPQ_DOBYBAKLIGHT_INI dolbybacklight_ini);
UINT8 Scaler_color_get_colorspace_index(void);
unsigned char Scaler_get_dcc_table_by_source(unsigned char table_mode);
unsigned char Scaler_Get_ADV_API_LEVEL_TBL_Idx(VIP_ADV_API_LEVEL API_Level);
char Scaler_get_sGamma(void);
void Scaler_set_sGamma(char level_index);
unsigned char Scaler_get_BLE(void);
void Scaler_set_BLE(unsigned char level_index);
void Scaler_set_ICM_7Axis_table(unsigned char which_table);
unsigned char Scaler_get_ICM_7Axis_table(void);
void Scaler_set_ICM_7_Axis_from_user(ICM_H_7axis_table* data);
void Scaler_get_ICM_7_Axis_from_user(ICM_H_7axis_table* data);
void Scaler_set_ICM_7Axis_Adjust(unsigned char HSI, unsigned short value, unsigned char axis_num);
unsigned short Scaler_get_ICM_7Axis_Adjust(unsigned char HSI, unsigned char axis_num);
void Scaler_set_ICM_7Axis_Adjust_enable(unsigned short enable);
unsigned short  Scaler_get_ICM_7Axis_Adjust_enable(void);
unsigned char Scaler_color_HLG_support_HLG14(void);//decide HLG 14 to HLG or SDR
unsigned char Scaler_apply_PQ_Linear(unsigned char Enable);
void Scaler_Set_PQ_RGB444MODE(unsigned char path);

#if defined(CONFIG_RTK_AI_DRV)
void Scaler_VIP_LC_reset(void);
void Scaler_VIP_scene_sharpness_reset(void);
void Scaler_VIP_DCC_Color_Independent_reset(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SCALER_LIB_H__*/

