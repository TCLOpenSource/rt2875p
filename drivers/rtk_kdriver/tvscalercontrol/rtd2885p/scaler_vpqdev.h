#ifndef __SCALER_VPQDEV_H
#define  __SCALER_VPQDEV_H


/**
 * Video Picture Quality Information.
 *
 * @see DDI Implementation Guide
*/
#include <scaler/vipCommon.h>

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
	#include <scalercommon/scalerDrvCommon.h>
#endif

#include <tvscalercontrol/vip/film.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/icm.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>


// include V4L2
#ifndef BUILD_QUICK_SHOW
#include <linux/videodev2.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <linux/version.h>
#include <asm/unaligned.h>
#endif
//#include <linux/v4l2-ext/videodev2-ext.h>
//#include <linux/v4l2-ext/v4l2-controls-ext.h>

//TVScaler Header file
//#include <tvscalercontrol/vpq_v4l2/vpq_v4l2_structure.h>
//#include <tvscalercontrol/vpq_v4l2/vpq_v4l2_api.h>




#define VPQ_RunTime_PM_Enable

#ifndef VIDEO_WID_T
	typedef enum{
		VIDEO_WID_MIN	= 0,				/**< WID min */
		VIDEO_WID_0 	= VIDEO_WID_MIN,	/**< WID main window */
		VIDEO_WID_1,						/**< WID sub window */
		VIDEO_WID_MAX	= VIDEO_WID_1,		/**< WID max */
		VIDEO_WID_MAXN, 					/**< WID max number */
	} __VIDEO_WID_T;
#define VIDEO_WID_T __VIDEO_WID_T
#endif

#ifndef SINT32
typedef signed int                __SINT32;
#define SINT32 __SINT32
#endif

typedef enum{
	UI_LEVEL_OFF = 0,
	UI_LEVEL_LOW,
	UI_LEVEL_MIDDLE,
	UI_LEVEL_HIGH,
	UI_LEVEL_AUTO,
	UI_LEVEL_SELECT_MAX,
} UI_LEVEL_SELECT;

typedef enum{
	Blk_Lv_UI_LEVEL_LOW = 0,
	Blk_Lv_UI_LEVEL_HIGH,
	Blk_Lv_UI_LEVEL_AUTO,
	Blk_Lv_UI_LEVEL_SELECT_MAX,
} BLACK_LEVEL_UI_ELECT;

typedef struct{
	UINT16 Red_Gain;
	UINT16 Green_Gain;
	UINT16 Blue_Gain;
	UINT16 Red_Offset;
	UINT16 Green_Offset;
	UINT16 Blue_Offset;
} COLORTEMP_ELEM_T;

typedef struct{
	UINT16 Gamme_Red[1024];
	UINT16 Gamme_Green[1024];
	UINT16 Gamme_Blue[1024];
} GAMMA_RGB_T;

typedef enum{
	PQ_DEV_NOTHING = 0,
	PQ_DEV_INIT_DONE,
	PQ_DEV_OPEN_DONE,
	PQ_DEV_UNINIT,
	PQ_DEV_CLOSE,
} PQ_DEV_STATUS;

typedef struct{
	UINT8 wId;
	UINT16 uDcVal;
	UINT32 pstData;
} FreshContrast_T;

typedef struct{
	signed int FreshContrastLUT[DCC_Curve_Node_MAX];
} FreshContrastLUT_T;

typedef struct{
	UINT32 bin[32];
} FreshContrastBin_T;

typedef struct{
	UINT32 chrm_bin[32];
} chrm_bin_T;

typedef struct{
	UINT32 hue_bin[24];
} hue_bin_T;

typedef struct{
	UINT8 wId;
	signed int uCgVal;
} ColorGain_T;


/* clear white gain control*/
typedef struct _HAL_VPQ_CLEAR_WHITE{
	VIDEO_WID_T wId;
	unsigned char enable;
	unsigned char mode;
	unsigned char UV_offset_index[VIP_YUV2RGB_Y_Seg_Max - 1];
	char Uoffset[VIP_YUV2RGB_Y_Seg_Max];
	char Voffset[VIP_YUV2RGB_Y_Seg_Max];
	UINT16 gain_by_UV_index[VIP_YUV2RGB_Y_Seg_Max - 1];
	UINT16 gain_by_UV[VIP_YUV2RGB_Y_Seg_Max];
} HAL_VPQ_CLEAR_WHITE;

#if 0 //k5l hw remove
typedef struct _RTK_BLUE_STRETCH_T {
	unsigned char ctrl[4];
	DRV_Gamma_BS gamma_bs;
} RTK_BLUE_STRETCH_T;
#endif
/* set black level*/
typedef enum{
	SET_BLACK_LEVEL_ITEM_Level = 0,
	SET_BLACK_LEVEL_ITEM_Input_Info,
	SET_BLACK_LEVEL_ITEM_MAX,
} HAL_VPQ_SET_BLACK_LEVEL_ITEM;

typedef struct _HAL_VPQ_SET_BLACK_LEVEL{
	VIDEO_WID_T wId;
	UINT8 uBlVal[SET_BLACK_LEVEL_ITEM_MAX];
} HAL_VPQ_SET_BLACK_LEVEL;


typedef struct {
	unsigned int Gamut33Matrix[9];
} HDR_SetGamut33Matrix_T;

typedef struct {
	UINT16 hdr_3dlut[HDR_24x24x24_size*3];
} HDR_Set3DLUT_16_T;

typedef struct {
    UINT32 version;//Version = 0 wild card(default data
    UINT32 length;//pData Length
    UINT8  wId;//0 : main
    UINT8* pData;
} HAL_VPQ_DATA_T;

typedef struct{
	UINT8 extType; // index for type (eg. 0: POD, 1:PCID, 2:reserved)
	UINT8 *pExtData; // LUT table data
	UINT32 extLength;  // LUT data size // variable size
}HAL_VPQ_OD_Extention_T;

/* for demura hal function */
#define DeMura_tv006FMT_to_tv001FMT(arg) ((arg>127)?((128-arg)):(arg))
#define Demura_LMMH_R_SD_check_BIT  0x1
#define Demura_LMMH_G_SD_check_BIT  0x2
#define Demura_LMMH_B_SD_check_BIT  0x4
#define Demura_LMMH_W_SD_check_BIT  0x8
#define Demura_LMMH_W_Size (481)
#define Demura_LMMH_H_Size (271)
#define Demura_LMMH_1Lv_Size (Demura_LMMH_W_Size * Demura_LMMH_H_Size)
#define Demura_LMMH_1ch_Size (Demura_LMMH_1Lv_Size * 4)
#define Demura_LMMH_4ch_Size (Demura_LMMH_1ch_Size *4)
#define Demura_LMMH_TBL_Size (Demura_LMMH_4ch_Size + 8)
#define Demura_LMMH_4ch_R_sta  (4+0*Demura_LMMH_1ch_Size)
#define Demura_LMMH_4ch_G_sta  (4+1*Demura_LMMH_1ch_Size)
#define Demura_LMMH_4ch_B_sta  (4+2*Demura_LMMH_1ch_Size)
#define Demura_LMMH_4ch_W_sta  (4+3*Demura_LMMH_1ch_Size)
#define Demura_LMMH_4ch_CRC_sta  (4+4*Demura_LMMH_1ch_Size)
#define Demura_LMMH_4ch_WL_sta  Demura_LMMH_4ch_W_sta
#define Demura_LMMH_4ch_WM1_sta  (Demura_LMMH_4ch_W_sta + Demura_LMMH_1Lv_Size)
#define Demura_LMMH_4ch_WM2_sta  (Demura_LMMH_4ch_W_sta + Demura_LMMH_1Lv_Size*2)
#define Demura_LMMH_4ch_WH_sta  (Demura_LMMH_4ch_W_sta + Demura_LMMH_1Lv_Size*3)

/**
*	3d mode type enum
*/
typedef enum{
	HAL_VPQ_3D_MODE_OFF = 0,		/*< 3d off, 3d to 2d*/
	HAL_VPQ_3D_MODE_3D_TB,			/*< 3d top and botom*/
	HAL_VPQ_3D_MODE_3D_SS,			/*< 3d side by side*/
	HAL_VPQ_3D_MODE_3D_CB,			/*< 3d checker board*/
	HAL_VPQ_3D_MODE_3D_FS,			/*< 3d frame seq*/
	HAL_VPQ_3D_MODE_3D_CI,			/*< 3d column interleave*/
	HAL_VPQ_3D_MODE_3D_LIH,			/*< 3d line interleave half*/
	HAL_VPQ_3D_MODE_3D_FP,			/*< 3d frame packing*/
	HAL_VPQ_3D_MODE_3D_FA,			/*< 3d field alternative*/
	HAL_VPQ_3D_MODE_3D_LA,			/*< 3d line alternative*/
	HAL_VPQ_3D_MODE_3D_SSF,			/*< 3d side by side full*/
	HAL_VPQ_3D_MODE_3D_DS,			/*< 3d dual stream*/
	HAL_VPQ_3D_MODE_2DTO3D,			/*< 2d to 3d*/
	HAL_VPQ_3D_MODE_MAX				/*< max num*/
}
HAL_VPQ_3D_MODE_T;

typedef enum{
	HAL_VPQ_INPUT_ATV = 0,			/*< analog rf*/
	HAL_VPQ_INPUT_AV,				/*< cvbs,svideo*/
	HAL_VPQ_INPUT_SCARTRGB,			/*< scart-rgb*/
	HAL_VPQ_INPUT_COMP,				/*< component*/
	HAL_VPQ_INPUT_RGB_PC,			/*< rgb-pc*/
	HAL_VPQ_INPUT_HDMI_TV,			/*< hdmi non-pc(hdmi-av,dvi-dtv,mcam)*/
	HAL_VPQ_INPUT_HDMI_PC,			/*< hdmi pc(dvi-pc)*/
	HAL_VPQ_INPUT_DTV,				/*< digital rf*/
	HAL_VPQ_INPUT_PICWIZ,			/*< dtv pic wiz*/
	HAL_VPQ_INPUT_PICTEST,			/*< dtv pic test*/
	HAL_VPQ_INPUT_MEDIA_MOVIE,		/*< qwer, cp-qwer*/
	HAL_VPQ_INPUT_MEDIA_PHOTO,		/*< photo, cp-photo*/
	HAL_VPQ_INPUT_CAMERA,			/*< vcs*/
	HAL_VPQ_INPUT_PVR_DTV,			/*< pvr dtv*/
	HAL_VPQ_INPUT_PVR_ATV,			/*< pvr atv*/
	HAL_VPQ_INPUT_PVR_AV,			/*< pvr av,scart-rgb*/
	HAL_VPQ_INPUT_MAX				/*< max num*/
}
HAL_VPQ_INPUT_T;

/**
*	HDR CSC1 enum
*/
typedef enum
{
	HAL_VPQ_HDR_OFF = 0,			///  the special control here, it is HDR ALL OFF
	HAL_VPQ_HDR_CSC1_BT709,
	HAL_VPQ_HDR_BT2020,

	HAL_VPQ_HDR_CSC1_MAX				///< max num
}
HAL_VPQ_HDR_CSC1_T;

typedef struct{
	short m[3][3];
} PANEL_MATRIX_T;

typedef struct{
	unsigned char uColorGamut;
	unsigned char uIsSD;
	unsigned char uIsPAL;
	CHIP_COLOR_GAMUT_MAPPING_IO_T gamutMapping;
} COLOR_GAMUT_T;

typedef struct {
    /*Clk*/
	UINT32 CRT_PLL_SSC0_S_R;
    	UINT32 CRT_PLL_SSC3_S_R;
	UINT32 CRT_PLL_SSC4_S_R;
	UINT32 CRT_SYS_DCLKSS_S_R;
	UINT32 CRT_SYS_PLL_DISP1_S_R;
	UINT32 CRT_SYS_PLL_DISP2_S_R;
	UINT32 CRT_SYS_PLL_DISP3_S_R;
	UINT32 CRT_SYS_DISPCLKSEL_S_R;
} VPQ_SUSPEND_RESUME_T;

typedef struct {
    UINT32 version;//Version = 0 wild card(default data
    UINT32 length;//pData Length
    UINT8  wId;//0 : main
    UINT8* pData;
} _HAL_VPQ_DATA_T;



#ifndef BUILD_QUICK_SHOW
//v4l2 ioctrl callback api
int vpq_v4l2_main_ioctl_s_ext_ctrls(struct file *file, void *fh, struct v4l2_ext_controls *ctrls);
int vpq_v4l2_main_ioctl_g_ext_ctrls(struct file *file, void *fh, struct v4l2_ext_controls *ctrls);
int vpq_v4l2_main_ioctl_s_ctrl(struct file *file, void *fh, struct v4l2_control *ctrl);
int vpq_v4l2_main_ioctl_g_ctrl(struct file *file, void *fh, struct v4l2_control *ctrl);
#endif
unsigned char V4L2_VPQ_Calc_Step_Power(unsigned short* lut_in_17, unsigned short* lut_out_16, unsigned char bCheckPow2);

void vpq_vbe_low_power_mode_suspend(void);
void vpq_vbe_low_power_mode_resume(void);
char vpq_pm_runtime_get(unsigned char isSyncFlag);
char vpq_pm_runtime_put(unsigned char isSyncFlag);
unsigned char vpq_get_VPQ_TSK_Stop(void);
unsigned char vpq_set_VPQ_TSK_Stop(unsigned char stop_en);

void vpq_v4l2_ai_nnsr_set_En(unsigned char bEn);
char vpq_v4l2_ai_nnsr_get_En(void);
void vpq_do_prepare_suspend(void);
void vpq_do_suspend(void);
void vpq_do_resume(void);
void vpq_do_resume_instanboot(void);
unsigned char vpq_get_handler_bypass(void);
void vpq_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop);
void vpq_extern_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop);
void vpq_boot_init(void);
void vpq_boot_init_QS(void);
unsigned char Get_Var_Demo_Flag(void);
void Set_Var_Demo_Flag(unsigned char value);
unsigned char Get_Var_PQ_Dev_Status(void);
void Set_Var_PQ_Dev_Status(unsigned char value);
unsigned char Get_Var_game_process(void);
void Set_Var_game_process(unsigned char value);
unsigned char Get_Var_g_bDIGameModeOnlyDebug(void);
void Set_Var_g_bDIGameModeOnlyDebug(unsigned char value);
unsigned char Get_Var_g_bAPLColorGainClr(void);
void Set_Var_g_bAPLColorGainClr(unsigned char value);
unsigned char Get_Var_ucColorFilterMode(void);
void Set_Var_ucColorFilterMode(unsigned char value);
unsigned char Get_Var_g_LGE_HDR_CSC_CTRL(void);
void Set_Var_g_LGE_HDR_CSC_CTRL(unsigned char value);
unsigned int *Get_Var_PQModeInfo_flag(void);
unsigned char Get_Var_g_srgbForceUpdate(void);
void Set_Var_g_srgbForceUpdate(unsigned char value);
unsigned char Get_Var_g_3DLUT_LastInternalSelect(void);
void Set_Var_g_3DLUT_LastInternalSelect(unsigned char value);
unsigned char Get_Var_g_3DLUT_LastEnableSatus(void);
void Set_Var_g_3DLUT_LastEnableSatus(unsigned char value);
unsigned char Get_Var_g_3DLUT_Resume(void);
void Set_Var_g_3DLUT_Resume(unsigned char value);
unsigned char Get_Var_g_InvGammaPowerMode(void);
void Set_Var_g_InvGammaPowerMode(unsigned char value);
GAMUT_3D_LUT_17x17x17_T *Get_Var_Ptr_g_buf3DLUT_LGDB(void);
unsigned char Get_Var_g_Color_Mode(void);
void Set_Var_g_Color_Mode(unsigned char value);
unsigned char Get_Var_g_flag_cm_adv_init_ok(void);
void Set_Var_g_flag_cm_adv_init_ok(unsigned char value);
unsigned char Get_Var_g_flag_cm_exp_init_ok(void);
void Set_Var_g_flag_cm_exp_init_ok(unsigned char value);
unsigned char Get_Var_g_cm_need_refresh(void);
void Set_Var_g_cm_need_refresh(unsigned char value);
CHIP_CM_REGION_T *Get_Var_Ptr_g_cm_rgn_adv(void);
CHIP_CM_REGION_T *Get_Var_Ptr_g_cm_rgn_exp(void);
CHIP_COLOR_CONTROL_T *Get_Var_Ptr_g_cm_ctrl_adv(void);
CHIP_COLOR_CONTROL_T *Get_Var_Ptr_g_cm_ctrl_exp(void);
CHIP_CM_REGION_EXT_T *Get_Var_Ptr_cmRegionExt(void);
unsigned char *Get_Var_Ptr_deMura_7Blk_Decode_TBL(void);
void Set_Var_vpq_project_id(unsigned int value);
#endif
