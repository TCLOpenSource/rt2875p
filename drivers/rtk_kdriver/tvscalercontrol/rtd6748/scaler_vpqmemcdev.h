#ifndef VPQ_MEMC_H
#define VPQ_MEMC_H

#include <linux/types.h>
#include <rtd_types.h>
#include "memc_isr/include/memc_lib.h"

//#include <base_types.h>
#ifndef BOOLEAN
#ifndef _EMUL_WIN
typedef	unsigned int			__BOOLEAN;
#define BOOLEAN __BOOLEAN
#else
typedef	unsigned char		__BOOLEAN;
#define BOOLEAN __BOOLEAN
#endif
#endif



/*-----------------------------------------------------------------------------
	Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Macro Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Extern Variables & Function Prototype Declarations
------------------------------------------------------------------------------*/
#define MEMC_DYNAMIC_ONOFF 0
#define MEMC_SUSPEND_RESUME_TEST 1
/*-----------------------------------------------------------------------------
	Local Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Local Type Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Global Type Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Static Variables & Function Prototypes Declarations
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Global Variables & Function Prototypes Declarations
------------------------------------------------------------------------------*/
/**
 * FRC MC type
 *
 */
typedef enum {
	VPQ_MEMC_TYPE_OFF = 0,      //24->60
	VPQ_MEMC_TYPE_LOW,
	VPQ_MEMC_TYPE_HIGH,
	VPQ_MEMC_TYPE_NORMAL,		// Clear Plus
	VPQ_MEMC_TYPE_USER,
	VPQ_MEMC_TYPE_55_PULLDOWN,  //24->48
	VPQ_MEMC_TYPE_MEDIUM,	
	VPQ_MEMC_TYPE_MAX,
} VPQ_MEMC_TYPE_T;

typedef enum
{
    VPQ_MEMC_LEVEL_OFF = 0,
    VPQ_MEMC_LEVEL_LOW,
    VPQ_MEMC_LEVEL_MID,
    VPQ_MEMC_LEVEL_HIGH,
    VPQ_MEMC_LEVEL_MAX
}VPQ_HAL_MEMC_LEVEL_E;

/**
*FRC bypass Region
*
*/
typedef enum {
	VPQ_BYPASS_REGION_0	= 0,
	VPQ_BYPASS_REGION_1,
	VPQ_BYPASS_REGION_2,
	VPQ_BYPASS_REGION_3,
	VPQ_BYPASS_REGION_4,
	VPQ_BYPASS_REGION_MAX,
} VPQ_MEMC_BYPASS_REGION_T;

/**
*FRC Rect
*
*/
typedef struct {
	unsigned int		w;
	unsigned int		h;
	unsigned int		x;
	unsigned int		y;
} VPQ_MEMC_BYPASS_REGION_RECT_T;


/**
 *FRC video mute type
 *
 */
typedef enum {
	VPQ_MEMC_MUTE_TYPE_INPUT = 0,
	VPQ_MEMC_MUTE_TYPE_OUTPUT,
	VPQ_MEMC_MUTE_TYPE_LVDS,
	VPQ_MEMC_MUTE_TYPE_MAX
} VPQ_MEMC_MUTE_TYPE_T;

typedef struct{
	unsigned int blurLevel;
	unsigned int judderLevel;
	unsigned int motion;
}VPQ_MEMC_SETMOTIONCOMP_T;

typedef struct{
	unsigned int bMotionPro;
}VPQ_MEMC_SETMOTIONPRO_T;

typedef struct{
	unsigned int type;
	unsigned int bOnOff;
}VPQ_MEMC_SETVIDEOBLOCK_T;

typedef struct{
	unsigned int bOnOff;
	unsigned int region;
	unsigned int x;
	unsigned int y;
	unsigned int w;
	unsigned int h;
}VPQ_MEMC_SETBYPASSREGION_T;

typedef struct{
	unsigned int bOnOff;
	unsigned int r;
	unsigned int g;
	unsigned int b;
}VPQ_MEMC_SETDEMOBAR_T;

/*! MEMC Input signal resolution enum. */
typedef enum
{
	VPQ_INPUT_1920X1080,					///< 1920X1080 resolution for FPGA Validation
	VPQ_INPUT_1920X540,						///< 1920X540  resolution for FPGA Validation
	VPQ_INPUT_3840X2160,					///< 3840x2160 resolution
	VPQ_INPUT_3840X1080,					///< 3840x2160 resolution, it is 3D input
	VPQ_INPUT_3840X540,						///< 3840X540 resolution
	VPQ_INPUT_1920X2160,					///< 1920X2160 resolution
	VPQ_INPUT_768X480,						///< 768X480 resolution for FPGA Validation
	VPQ_INPUT_1280X720,						///< 1280X720 resolution for FPGA Validation
	VPQ_INPUT_7680X4320,					///< 7680X4320 resolution 
	VPQ_INPUT_2560X1440,					///< 2560X1440 resolution 
	VPQ_INPUT_RESOLUTION_MAX,
}VPQ_MEMC_INPUT_RESOLUTION;

/*! MEMC Output signal resolution enum. */
typedef enum
{
	VPQ_OUTPUT_1920X1080,					///< 1920x1080 resolution for FPGA Validation
	VPQ_OUTPUT_1920X540,					///< 1920x540  resolution for FPGA Validation
	VPQ_OUTPUT_3840X2160,					///< 3840x2160 resolution
	VPQ_OUTPUT_3840X1080,					///< 3840x1080 resolution
	VPQ_OUTPUT_3840X540, 					///< 3840X540 resolution
	VPQ_OUTPUT_768X480,						///< 768X480 resolution for FPGA Validation
	VPQ_OUTPUT_1280X720,						///< 1280X720 resolution for FPGA Validation
	VPQ_OUTPUT_7680X4320,					///< 7680X4320 resolution 
	VPQ_OUTPUT_2560X1440,					///< 2560X1440 resolution 
	VPQ_OUTPUT_RESOLUTION_MAX,
}VPQ_MEMC_OUTPUT_RESOLUTION;

/*! MEMC Input signal frame rate enum. */
typedef enum
{
	VPQ_INPUT_24HZ,							///< input 24Hz
	VPQ_INPUT_25HZ,							///< input 25Hz
	VPQ_INPUT_30HZ,							///< input 30Hz
	VPQ_INPUT_48HZ,							///< input 48Hz
	VPQ_INPUT_50HZ,							///< input 50Hz
	VPQ_INPUT_60HZ,							///< input 60Hz
	VPQ_INPUT_96HZ,							///< input 96Hz
	VPQ_INPUT_100HZ,						///< input 100Hz
	VPQ_INPUT_120HZ,						///< input 120Hz
	VPQ_INPUT_FRAME_MAX,
}VPQ_MEMC_INPUT_FRAMERATE;

/*! MEMC Output signal frame rate enum. */
typedef enum
{
	VPQ_OUTPUT_48HZ,						///< output 48Hz
	VPQ_OUTPUT_50HZ,						///< output 50Hz
	VPQ_OUTPUT_60HZ,						///< output 60Hz
	VPQ_OUTPUT_96HZ,						///< output 96Hz
	VPQ_OUTPUT_100HZ,						///< output 100Hz
	VPQ_OUTPUT_120HZ,						///< output 120Hz
	VPQ_OUTPUT_200HZ,						///< output 200Hz
	VPQ_OUTPUT_240HZ,						///< output 240Hz
	VPQ_OUTPUT_FRAME_MAX,
	VPQ_OUTPUT_FRAME_INVALID = 255,
}VPQ_MEMC_OUTPUT_FRAMERATE;

/*! MEMC Input format enum. */
typedef enum
{
	VPQ_INPUT_2D,							///< memc input is 2D
	VPQ_INPUT_3D_SBS,						///< memc input is 3D side by side
	VPQ_INPUT_3D_TB,						///< memc input is 3D top and bottom
	VPQ_INPUT_3D_LR,						///< memc input is 3D frame sequential
	VPQ_INPUT_3D_LBL,						///< memc input is 3D line by line
	VPQ_INPUT_FORMAT_MAX,
}VPQ_MEMC_INPUT_FORMAT;

/*! MEMC Output format enum. */
typedef enum
{
	VPQ_OUTPUT_2D,							///< memc output is 2D
	VPQ_OUTPUT_3D_SG,						///< memc output is shutter glass 3D
	VPQ_OUTPUT_3D_PR,						///< memc output is Polarized 3D
	VPQ_OUTPUT_3D_SG_LLRR,					///< memc output is shutter glass 3D LLRR
	VPQ_OUTPUT_FORMAT_MAX,
}VPQ_MEMC_OUTPUT_FORMAT;

/*! MEMC In/Out mode enum. */
typedef enum
{
	VPQ_MEMC_RGB_IN_VIDEO_OUT,				///< memc RGB in and video output
	VPQ_MEMC_RGB_IN_PC_OUT,					///< memc RGB in and pc output
	VPQ_MEMC_YUV_IN_VIDEO_OUT, 				///< memc YUV444 in and video output
	VPQ_MEMC_YUV_IN_PC_OUT,					///< memc YUV444 in and pc output
	VPQ_MEMC_INOUT_MODE_MAX,
}VPQ_MEMC_INOUT_MODE;

/*! MEMC mode enum. */
typedef enum
{
	VPQ_MEMC_SMOOTH,						///< Smooth mode
	VPQ_MEMC_CLEAR,							///< Clear mode
	VPQ_MEMC_NORMAL,						///< Normal mode
	VPQ_MEMC_OFF,							///< Off mode
	VPQ_MEMC_USER,							///< User mode, can manually set dejudder and deblur level
	VPQ_MEMC_MODE_MAX,
}VPQ_MEMC_MODE;

/*! MEMC RIM mode. */
typedef enum
{
	VPQ_RIM_MODE_0,							///< memc rim mode 0
	VPQ_RIM_MODE_1,							///< memc rim mode 1
}VPQ_MEMC_RIM_MODE;

/*! module ID enum */
typedef enum
{
	VPQ_MODULE_ID_APP,
	VPQ_MODULE_ID_FRC,
}VPQ_MODULE_ID;

/*! MEMC demo mode enum. */
typedef enum
{
	VPQ_DEMO_MODE_OFF,						///< Demo mode off
	VPQ_DEMO_MODE_LEFT,						///< Demo mode left memc off, right memc on
	VPQ_DEMO_MODE_RIGHT,					///< Demo mode left memc on, right memc off
}VPQ_MEMC_DEMO_MODE;

/**
 * MEMC CLK EN
 * Reset flow : CLKEN = "0" 'RSTN = "0" 'RSTN = "1" ' CLKEN = "1"
 * RSTN_MEMC_RBUS clock in MEMC , should be set beforce  RSTN_MEMC/ RSTN_MEME_ME
 * CLKEN_MEMC_RBUS clock in MEMC , should be set beforce  CLKEN_MEMC/CLKEN_MEME_ME
 */
typedef enum {
	VPQ_MEMC_SYS_CLKEN_RBUS = 0,
	VPQ_MEMC_SYS_CLKEN,
	VPQ_MEMC_SYS_RSTN_RBUS,
	VPQ_MEMC_SYS_RSTN,
} VPQ_MEMC_SYS_CLKEN_T;


/*-----------------------------------------------------------------------------
	Local Variables & Function Prototypes Declarations
------------------------------------------------------------------------------*/
void HAL_VPQ_MEMC_Initialize(void);
void HAL_VPQ_MEMC_Uninitialize(void);
long HAL_VPQ_MEMC_SetMotionComp(unsigned char blurLevel, unsigned char judderLevel, VPQ_MEMC_TYPE_T motion);
void HAL_VPQ_MEMC_SetBlurLevel(unsigned char blurLevel);
void HAL_VPQ_MEMC_SetJudderLevel(unsigned char judderLevel);
long HAL_VPQ_MEMC_MotionCompOnOff(BOOLEAN bOnOff);
long HAL_VPQ_MEMC_LowDelayMode(unsigned char type);
long HAL_VPQ_MEMC_SetRGBYUVMode(unsigned char mode);
void HAL_VPQ_MEMC_GetFrameDelay (unsigned int *pFrameDelay);
void HAL_VPQ_MEMC_SetVideoBlock(VPQ_MEMC_MUTE_TYPE_T type, BOOLEAN bOnOff);
void HAL_VPQ_MEMC_SetTrueMotionDemo(BOOLEAN bOnOff);
void HAL_VPQ_MEMC_GetFirmwareVersion(unsigned int *pVersion);
void HAL_VPQ_MEMC_SetBypassRegion(BOOLEAN bOnOff, VPQ_MEMC_BYPASS_REGION_T region, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void HAL_VPQ_MEMC_SetReverseControl(unsigned char u8Mode);
void HAL_VPQ_MEMC_Freeze(unsigned char type);
void HAL_VPQ_MEMC_SetDemoBar(BOOLEAN bOnOff, unsigned char r, unsigned char g, unsigned char b);
void HAL_VPQ_MEMC_DEBUG(void);
long HAL_VPQ_MEMC_SetMotionPro(BOOLEAN bMotionPro);
bool get_reset_MEMC_flg(void);
void set_reset_MEMC_flg(bool enable);

//scaler driver
void MEMC_SYS_Delay(unsigned char nDelay);
void MEMC_LoadScriptDMA(void);
void MEMC_LoadScriptDMADisable(void);
void MEMC_ChangeMCBufferAddr(unsigned char u1_is_changed_to_6_buffer);
void MEMC_Set_malloc_address(unsigned char status);
int MEMC_Get_DMA_malloc_address(unsigned char status, unsigned int *DMA_release_start, unsigned int *DMA_release_size);

void Scaler_MEMC_Mux(void);
void Scaler_MEMC_outMux(unsigned char dbuf_en,unsigned char outmux_sel);
void Scaler_MEMC_CLK_Enable(void);
unsigned char Scaler_MEMC_CLK_Check(void);
unsigned char Scaler_ME_CLK_Check(void);
void Scaler_MEMC_CLK(void);
void Scaler_MEMC_Handler(void);
void Scaler_MEMC_initial_Case(void);
//void Scaler_MEMC_MC_Set_MEMBoundary(unsigned int  nDownlimit,unsigned int nUplimit);
void Scaler_MEMC_allocate_memory(void);
void Scaler_MEMC_ForceBg(unsigned char bEnable);
void Scaler_MEMC_output_force_bg_enable(unsigned char enable,char const* pfunc,unsigned int line);
void Scaler_MEMC_set_output_bg_color(unsigned int color_r, unsigned int color_g, unsigned int color_b);
void Scaler_MEMC_SetMotionType(VPQ_MEMC_TYPE_T motion);
VPQ_MEMC_TYPE_T Scaler_MEMC_GetMotionType(void);
int Scaler_MEMC_set_cinema_mode_en(unsigned char bEn);

void vpq_memc_disp_suspend_instanboot(void);
void vpq_memc_disp_resume_instanboot(void);
void vpq_memc_disp_suspend_snashop(void);
void vpq_memc_disp_resume_snashop(void);
void vpq_memc_runtime_suspend_flow(void);
void vpq_memc_runtime_resume_flow(void);

void Scaler_MEMC_fbg_control(char const* pfunc,unsigned int line);
void Scaler_MEMC_SetInOutFrameRate(void);
void Scaler_MEMC_SetInOutFrameRateByDisp(unsigned char _channel);
/**
 * @brief This function get the MEMC output size informtion base on the scaler Disp. information.
 * @retval unsigned char, return the information of the panel size 	
*/
unsigned char Scaler_MEMC_GetPanelSizeByDisp(void);
void Scaler_MEMC_Booting_NoSignal_Check_OutputBG(unsigned char enable);
void Scaler_MEMC_Set_CheckMEMC_Outbg(unsigned char mode);
unsigned char Scaler_MEMC_Get_CheckMEMC_Outbg(void);
unsigned char Scaler_MEMC_Get_First_Run_FBG_enable(void);
unsigned char Scaler_MEMC_Get_instanboot_resume_Done(void);
unsigned char Scaler_MEMC_Get_snashop_resume_Done(void);
unsigned int Scaler_MEMC_Get_rotate_mode(void);
void Scaler_MEMC_Bypass_On(unsigned char b_isBypass);
void Scaler_MEMC_Bypass_CLK_OnOff(unsigned char is_clk_on);
//void Scaler_MEMC_Bypass_CLK(unsigned char is_clk_on);
unsigned char Scaler_MEMC_get_Bypass_Status(void);
void Scaler_MEMC_Set_Clock_Enable(unsigned char enable, unsigned char bMute);
int Scaler_MEMC_Get_Clock_Enable(unsigned char *enable, unsigned char *bMute);
void Scaler_MEMC_Set_Clock_ForceExit(void);
void Scaler_MEMC_HDMI_ME_CLK_OnOff(unsigned char is_clk_on, unsigned char mode);
int Scaler_MEMC_DTV_ME_CLK_OnOff(unsigned char enable, unsigned char bMode);
int Scaler_MEMC_MEMC_CLK_OnOff(unsigned char ME_enable, unsigned char MC_enable, unsigned char bMute);
long vpq_memc_set_lowDelayMode(unsigned char type);
void Scaler_MEMC_Set_PowerSaving_Status(unsigned char mode);
unsigned char Scaler_MEMC_Get_PowerSaving_Status(void);
void Scaler_MEMC_Set_BlueScreen(unsigned char mode);
void Scaler_MEMC_Set_Performance_Checking_Database_index(unsigned char index);
unsigned char Scaler_MEMC_Get_Performance_Checking_Database_index(void);
unsigned char * Scaler_MEMC_GetCadence(void);
void Scaler_MEMC_Set_Lib_Freeze(unsigned char enable);
void Scaler_MEMC_Set_MEMC_FreezeStatus_Flag(unsigned char u1_enable);
unsigned char Scaler_MEMC_Get_MEMC_FreezeStatus_Flag(void);
BOOL Scaler_PQLContext_GetStillFrameFlag(void);
unsigned char Scaler_MEMC_Lib_UltraLowDelayMode_Judge(void);
BOOL Scaler_MEMC_LibSetMEMCFrameRepeatEnable(BOOL bEnable, BOOL bForce);
unsigned int Scaler_Vpqmemcdev_MEMC_GetAVSyncDelay(void);
void Scaler_Set_Info_from_Scaler(MEMC_INFO_FROM_SCALER *info_from_scaler);
MEMC_INFO_FROM_SCALER *Scaler_Get_Info_from_Scaler(VOID);

void Scaler_set_MEMC_gamemode_lowdelay(unsigned char enable);
void vpq_memc_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop);
void VPQMEMC_rlink_MEMC_ID_enable(void);
void VPQMEMC_rlink_MEMC_ID(void);
void Scaler_MEMC_Set_DISP_DHTOTAL(unsigned int DHTotal);
unsigned int Scaler_MEMC_Get_DISP_DHTOTAL(void);

/**
 * @brief This function set power saving mode on
 * @retval 1		power save on fail
 * @retval 0		power save on success
*/
BOOL ScalerMEMC_EnablePowerSave(void);
/**
 * @brief This function set power saving mode off
 * @retval 1		power save off fail
 * @retval 0		power save off success
*/
BOOL ScalerMEMC_DisablePowerSave(void);

/**
 * @brief This function get RbusAccess stataus of MEMC
 * @retval TRUE		MEMC Rbus access Ready!		
 * @retval FALSE	MEMC Rbus can't be accessed!  
*/
unsigned char Scaler_MEMC_RbusAccess_Check(void);


#ifdef MEMC_DYNAMIC_ONOFF
void Scaler_MEMC_Dynamic_OnOff(void);
void Scaler_MEMC_Mux_off(void);

#endif

#define MEMC_LATENCY01_ADDR 0xB80070B0
#define MEMC_LATENCY02_ADDR 0xB80030B0



typedef struct{
	//kmc_top
	unsigned int	KMC_TOP__KMC_TOP_00;
	unsigned int	KMC_TOP__KMC_TOP_04;
	unsigned int	KMC_TOP__KMC_TOP_08;
	unsigned int	KMC_TOP__KMC_TOP_0C;
	unsigned int	KMC_TOP__KMC_TOP_10;
	unsigned int	KMC_TOP__KMC_TOP_14;
	unsigned int	KMC_TOP__KMC_TOP_18;
	unsigned int	KMC_TOP__KMC_TOP_1C;
	unsigned int	KMC_TOP__KMC_TOP_20;
	unsigned int	KMC_TOP__KMC_TOP_24;
	unsigned int	KMC_TOP__KMC_TOP_28;
	unsigned int	KMC_TOP__KMC_TOP_2C;
	unsigned int	KMC_TOP__KMC_TOP_30;
	unsigned int	KMC_TOP__KMC_TOP_34;
	unsigned int	KMC_TOP__KMC_TOP_38;
	unsigned int	KMC_TOP__KMC_TOP_3C;
	unsigned int	KMC_TOP__KMC_TOP_40;
	unsigned int	KMC_TOP__KMC_TOP_44;
	unsigned int	KMC_TOP__KMC_TOP_48;
	unsigned int	KMC_TOP__KMC_TOP_4C;
	unsigned int	KMC_TOP__KMC_TOP_50;
	unsigned int	KMC_TOP__KMC_TOP_54;
	unsigned int	KMC_TOP__KMC_TOP_58;
	unsigned int	KMC_TOP__KMC_TOP_5C;
	unsigned int	KMC_TOP__KMC_TOP_60;
	unsigned int	KMC_TOP__KMC_TOP_64;
	unsigned int	KMC_TOP__KMC_TOP_68;
	unsigned int	KMC_TOP__KMC_TOP_6C;
	//IPPRE
	unsigned int	IPPRE__IPPRE_00;
	unsigned int	IPPRE__IPPRE_04;
	unsigned int	IPPRE__IPPRE_08;
	unsigned int	IPPRE__IPPRE_0C;
	unsigned int	IPPRE__IPPRE_10;
	unsigned int	IPPRE__IPPRE_14;
	unsigned int	IPPRE__IPPRE_18;
	unsigned int	IPPRE__IPPRE_1C;
	unsigned int	IPPRE__IPPRE_20;
	unsigned int	IPPRE__IPPRE_24;
	unsigned int	IPPRE__IPPRE_28;
	unsigned int	IPPRE__IPPRE_2C;
	unsigned int	IPPRE__IPPRE_30;
	unsigned int	IPPRE__IPPRE_34;
	unsigned int	IPPRE__IPPRE_38;
	unsigned int	IPPRE__IPPRE_3C;
	unsigned int	IPPRE__IPPRE_40;
	unsigned int	IPPRE__IPPRE_44;
	unsigned int	IPPRE__IPPRE_48;
	unsigned int	IPPRE__IPPRE_4C;
	unsigned int	IPPRE__IPPRE_50;
	unsigned int	IPPRE__IPPRE_54;
	unsigned int	IPPRE__IPPRE_58;
	unsigned int	IPPRE__IPPRE_5C;
	unsigned int	IPPRE__IPPRE_60;
	unsigned int	IPPRE__IPPRE_64;
	unsigned int	IPPRE__IPPRE_68;
	unsigned int	IPPRE__IPPRE_6C;
	unsigned int	IPPRE__IPPRE_70;
	unsigned int	IPPRE__IPPRE_74;
	unsigned int	IPPRE__IPPRE_78;
	unsigned int	IPPRE__IPPRE_7C;
	unsigned int	IPPRE__IPPRE_80;
	unsigned int	IPPRE__IPPRE_84;
	unsigned int	IPPRE__IPPRE_88;
	unsigned int	IPPRE__IPPRE_8C;
	unsigned int	IPPRE__IPPRE_90;
	unsigned int	IPPRE__IPPRE_94;
	unsigned int	IPPRE__IPPRE_98;
	unsigned int	IPPRE__IPPRE_9C;
	unsigned int	IPPRE__IPPRE_A0;
	unsigned int	IPPRE__IPPRE_A4;
	unsigned int	IPPRE__IPPRE_A8;
	unsigned int	IPPRE__IPPRE_AC;
	unsigned int	IPPRE__IPPRE_B0;
	unsigned int	IPPRE__IPPRE_B4;
	unsigned int	IPPRE__IPPRE_B8;
	unsigned int	IPPRE__IPPRE_BC;
	unsigned int	IPPRE__IPPRE_C0;
	unsigned int	IPPRE__IPPRE_C4;
	unsigned int	IPPRE__IPPRE_C8;
	unsigned int	IPPRE__IPPRE_CC;
	unsigned int	IPPRE__IPPRE_D0;
	unsigned int	IPPRE__IPPRE_D4;
	unsigned int	IPPRE__IPPRE_D8;
	unsigned int	IPPRE__IPPRE_DC;
	unsigned int	IPPRE__IPPRE_E0;
	unsigned int	IPPRE__IPPRE_E4;
	//KMC_CPR_TOP0
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_00;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_04;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_08;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_0C;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_10;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_14;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_18;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_1C;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_20;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_24;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_28;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_2C;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_30;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_34;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_38;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_3C;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_40;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_44;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_48;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_4C;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_50;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_54;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_58;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_5C;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_60;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_64;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_68;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_6C;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_70;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_74;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_78;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_7C;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_80;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_84;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_88;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_8C;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_90;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_94;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_98;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_9C;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_A0;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_A4;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_A8;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_AC;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_B0;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_B4;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_B8;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_BC;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_C0;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_C4;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_C8;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_CC;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_D0;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_D4;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_D8;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_DC;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_E0;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_E4;
	unsigned int	KMC_CPR_TOP0__KMC_CPR_TOP0_E8;
	//KMC_CPR_TOP1
	unsigned int	KMC_CPR_TOP1__PATT_CTRL0;
	unsigned int	KMC_CPR_TOP1__PATT_LF_I_POSITION0;
	unsigned int	KMC_CPR_TOP1__PATT_LF_I_POSITION1;
	unsigned int	KMC_CPR_TOP1__PATT_HF_I_POSITION0;
	unsigned int	KMC_CPR_TOP1__PATT_HF_I_POSITION1;
	unsigned int	KMC_CPR_TOP1__CPR_PATT_LF_I_SETTING0;
	unsigned int	KMC_CPR_TOP1__CPR_PATT_LF_I_SETTING1;
	unsigned int	KMC_CPR_TOP1__CPR_PATT_HF_I_SETTING0;
	unsigned int	KMC_CPR_TOP1__CPR_PATT_HF_I_SETTING1;
	unsigned int	KMC_CPR_TOP1__PATT_LF_P_POSITION0;
	unsigned int	KMC_CPR_TOP1__PATT_LF_P_POSITION1;
	unsigned int	KMC_CPR_TOP1__PATT_HF_P_POSITION0;
	unsigned int	KMC_CPR_TOP1__PATT_HF_P_POSITION1;
	unsigned int	KMC_CPR_TOP1__CPR_PATT_LF_P_SETTING0;
	unsigned int	KMC_CPR_TOP1__CPR_PATT_LF_P_SETTING1;
	unsigned int	KMC_CPR_TOP1__CPR_PATT_HF_P_SETTING0;
	unsigned int	KMC_CPR_TOP1__CPR_PATT_HF_P_SETTING1;
	unsigned int	KMC_CPR_TOP1__PATT_LF_NR_POSITION0;
	unsigned int	KMC_CPR_TOP1__PATT_LF_NR_POSITION1;
	unsigned int	KMC_CPR_TOP1__PATT_HF_NR_POSITION;
	unsigned int	KMC_CPR_TOP1__PATT_HF_NR_POSITION1;
	unsigned int	KMC_CPR_TOP1__CPR_PATT_LF_NR_SETTING0;
	unsigned int	KMC_CPR_TOP1__CPR_PATT_LF_NR_SETTING1;
	unsigned int	KMC_CPR_TOP1__CPR_PATT_HF_NR_SETTING0;
	unsigned int	KMC_CPR_TOP1__CPR_PATT_HF_NR_SETTING1;
	unsigned int	KMC_CPR_TOP1__CPR_DEBUG_CFG0;
	//KMC_DM_TOP
	unsigned int	KMC_DM_TOP__KMC_00_START_ADDRESS0;
	unsigned int	KMC_DM_TOP__KMC_00_START_ADDRESS1;
	unsigned int	KMC_DM_TOP__KMC_00_START_ADDRESS2;
	unsigned int	KMC_DM_TOP__KMC_00_START_ADDRESS3;
	unsigned int	KMC_DM_TOP__KMC_00_START_ADDRESS4;
	unsigned int	KMC_DM_TOP__KMC_00_START_ADDRESS5;
	unsigned int	KMC_DM_TOP__KMC_00_END_ADDRESS0;
	unsigned int	KMC_DM_TOP__KMC_00_END_ADDRESS1;
	unsigned int	KMC_DM_TOP__KMC_00_END_ADDRESS2;
	unsigned int	KMC_DM_TOP__KMC_00_END_ADDRESS3;
	unsigned int	KMC_DM_TOP__KMC_00_END_ADDRESS4;
	unsigned int	KMC_DM_TOP__KMC_00_END_ADDRESS5;
	unsigned int	KMC_DM_TOP__KMC_00_LR_OFFSET_ADDR;
	unsigned int	KMC_DM_TOP__KMC_00_LINE_OFFSET_ADDR;
	unsigned int	KMC_DM_TOP__KMC_00_AGENT;
	unsigned int	KMC_DM_TOP__KMC_00_THRESHOLD;
	unsigned int	KMC_DM_TOP__KMC_01_START_ADDRESS0;
	unsigned int	KMC_DM_TOP__KMC_01_START_ADDRESS1;
	unsigned int	KMC_DM_TOP__KMC_01_START_ADDRESS2;
	unsigned int	KMC_DM_TOP__KMC_01_START_ADDRESS3;
	unsigned int	KMC_DM_TOP__KMC_01_START_ADDRESS4;
	unsigned int	KMC_DM_TOP__KMC_01_START_ADDRESS5;
	unsigned int	KMC_DM_TOP__KMC_01_END_ADDRESS0;
	unsigned int	KMC_DM_TOP__KMC_01_END_ADDRESS1;
	unsigned int	KMC_DM_TOP__KMC_01_END_ADDRESS2;
	unsigned int	KMC_DM_TOP__KMC_01_END_ADDRESS3;
	unsigned int	KMC_DM_TOP__KMC_01_END_ADDRESS4;
	unsigned int	KMC_DM_TOP__KMC_01_END_ADDRESS5;
	unsigned int	KMC_DM_TOP__KMC_01_LR_OFFSET_ADDR;
	unsigned int	KMC_DM_TOP__KMC_01_LINE_OFFSET_ADDR;
	unsigned int	KMC_DM_TOP__KMC_01_AGENT;
	unsigned int	KMC_DM_TOP__KMC_01_THRESHOLD;
	unsigned int	KMC_DM_TOP__KMC_02_AGENT;
	unsigned int	KMC_DM_TOP__KMC_02_THRESHOLD;
	unsigned int	KMC_DM_TOP__KMC_03_AGENT;
	unsigned int	KMC_DM_TOP__KMC_03_THRESHOLD;
	unsigned int	KMC_DM_TOP__KMC_04_AGENT;
	unsigned int	KMC_DM_TOP__KMC_04_THRESHOLD;
	unsigned int	KMC_DM_TOP__KMC_05_AGENT;
	unsigned int	KMC_DM_TOP__KMC_05_THRESHOLD;
	unsigned int	KMC_DM_TOP__KMC_06_AGENT;
	unsigned int	KMC_DM_TOP__KMC_06_THRESHOLD;
	unsigned int	KMC_DM_TOP__KMC_07_AGENT;
	unsigned int	KMC_DM_TOP__KMC_07_THRESHOLD;
	unsigned int	KMC_DM_TOP__KMC_VTOTAL_AND_TIMER;
	unsigned int	KMC_DM_TOP__KMC_HOLD_TIME_G0;
	unsigned int	KMC_DM_TOP__KMC_HOLD_TIME_G1;
	unsigned int	KMC_DM_TOP__KMC_SOURCE_MODE;
	unsigned int	KMC_DM_TOP__KMC_DEBUG_HEAD_G0;
	unsigned int	KMC_DM_TOP__KMC_DEBUG_HEAD_G1;
	unsigned int	KMC_DM_TOP__KMC_DEBUG_HEAD_G2;
	unsigned int	KMC_DM_TOP__KMC_DEBUG_HEAD_G3;
	unsigned int	KMC_DM_TOP__KMC_00_START_ADDRESS6;
	unsigned int	KMC_DM_TOP__KMC_00_END_ADDRESS6;
	unsigned int	KMC_DM_TOP__KMC_00_START_ADDRESS7;
	unsigned int	KMC_DM_TOP__KMC_00_END_ADDRESS7;
	unsigned int	KMC_DM_TOP__KMC_01_START_ADDRESS6;
	unsigned int	KMC_DM_TOP__KMC_01_END_ADDRESS6;
	unsigned int	KMC_DM_TOP__KMC_01_START_ADDRESS7;
	unsigned int	KMC_DM_TOP__KMC_01_END_ADDRESS7;
	//unsigned int	KMC_DM_TOP__KMC_DMA_MC_DMA_F0;
	//IPPRE1
	unsigned int	IPPRE1__IPPRE1_00;
	unsigned int	IPPRE1__IPPRE1_04;
	//MC
	unsigned int	MC__MC_0C;
	unsigned int	MC__MC_10;
	unsigned int	MC__MC_14;
	unsigned int	MC__MC_18;
	unsigned int	MC__MC_1C;
	unsigned int	MC__MC_20;
	unsigned int	MC__MC_24;
	unsigned int	MC__MC_28;
	unsigned int	MC__MC_2C;
	unsigned int	MC__MC_30;
	unsigned int	MC__MC_34;
	unsigned int	MC__MC_38;
	unsigned int	MC__MC_3C;
	unsigned int	MC__MC_40;
	unsigned int	MC__MC_44;
	unsigned int	MC__MC_48;
	unsigned int	MC__MC_4C;
	unsigned int	MC__MC_50;
	unsigned int	MC__MC_54;
	unsigned int	MC__MC_58;
	unsigned int	MC__MC_5C;
	unsigned int	MC__MC_60;
	unsigned int	MC__MC_64;
	unsigned int	MC__MC_68;
	unsigned int	MC__MC_6C;
	unsigned int	MC__MC_70;
	unsigned int	MC__MC_74;
	unsigned int	MC__MC_78;
	unsigned int	MC__MC_7C;
	unsigned int	MC__MC_80;
	unsigned int	MC__MC_84;
	unsigned int	MC__MC_88;
	unsigned int	MC__MC_8C;
	unsigned int	MC__MC_90;
	unsigned int	MC__MC_94;
	unsigned int	MC__MC_98;
	unsigned int	MC__MC_9C;
	unsigned int	MC__MC_A0;
	unsigned int	MC__MC_A4;
	unsigned int	MC__MC_A8;
	unsigned int	MC__MC_AC;
	unsigned int	MC__MC_B0;
	unsigned int	MC__MC_B4;
	unsigned int	MC__MC_B8;
	unsigned int	MC__MC_BC;
	unsigned int	MC__MC_C0;
	unsigned int	MC__MC_C4;
	unsigned int	MC__MC_C8;
	unsigned int	MC__MC_CC;
	unsigned int	MC__MC_D0;
	unsigned int	MC__MC_D4;
	unsigned int	MC__MC_D8;
	unsigned int	MC__MC_DC;
	unsigned int	MC__MC_E0;
	unsigned int	MC__MC_E4;
	unsigned int	MC__MC_E8;
	//MC2
	unsigned int	MC2__MC2_00;
	unsigned int	MC2__MC2_04;
	unsigned int	MC2__MC2_08;
	unsigned int	MC2__MC2_0C;
	unsigned int	MC2__MC2_10;
	unsigned int	MC2__MC2_14;
	unsigned int	MC2__MC2_18;
	unsigned int	MC2__MC2_1C;
	unsigned int	MC2__MC2_20;
	unsigned int	MC2__MC2_24;
	unsigned int	MC2__MC2_28;
	unsigned int	MC2__MC2_2C;
	unsigned int	MC2__MC2_30;
	unsigned int	MC2__MC2_50;
	unsigned int	MC2__MC2_54;
	unsigned int	MC2__MC2_58;
	unsigned int	MC2__MC2_5C;
	unsigned int	MC2__MC2_60;
	unsigned int	MC2__MC2_64;
	unsigned int	MC2__MC2_68;
	unsigned int	MC2__MC2_6C;
	unsigned int	MC2__MC2_70;
	unsigned int	MC2__MC2_74;
	unsigned int	MC2__MC2_7C;
	unsigned int	MC2__MC2_80;
	unsigned int	MC2__MC2_84;
	unsigned int	MC2__MC2_88;
	unsigned int	MC2__MC2_8C;
	unsigned int	MC2__MC2_90;
	unsigned int	MC2__MC2_94;
	unsigned int	MC2__MC2_98;
	unsigned int	MC2__MC2_9C;
	unsigned int	MC2__MC2_A0;
	unsigned int	MC2__MC2_A4;
	unsigned int	MC2__MC2_A8;
	//MC_PQ_DECMP
	unsigned int	PQDC_08_00;
	unsigned int	PQDC_08_04;
	unsigned int	PQDC_08_0C;
	unsigned int	PQDC_08_10;
	unsigned int	PQDC_08_14;
	unsigned int	PQDC_08_18;
	unsigned int	PQDC_08_1C;
	unsigned int	PQDC_08_20;
	unsigned int	PQDC_08_24;
	unsigned int	PQDC_08_28;
	unsigned int	PQDC_08_2C;
	unsigned int	PQDC_08_30;
	unsigned int	PQDC_08_34;
	unsigned int	PQDC_08_38;
	unsigned int	PQDC_08_3C;
	unsigned int	PQDC_08_40;
	unsigned int	PQDC_08_44;
	unsigned int	PQDC_08_48;
	unsigned int	PQDC_08_4C;
	unsigned int	PQDC_08_50;
	unsigned int	PQDC_08_54;
	unsigned int	PQDC_08_58;
	unsigned int	PQDC_08_5C;
	unsigned int	PQDC_08_60;
	unsigned int	PQDC_08_64;
	unsigned int	PQDC_08_68;
	unsigned int	PQDC_08_6C;
	unsigned int	PQDC_08_70;
	unsigned int	PQDC_08_74;
	unsigned int	PQDC_08_78;
	unsigned int	PQDC_08_7C;
	unsigned int	PQDC_08_80;
	unsigned int	PQDC_08_84;
	unsigned int	PQDC_08_88;
	unsigned int	PQDC_08_8C;
	unsigned int	PQDC_08_90;
	unsigned int	PQDC_08_94;
	unsigned int	PQDC_08_98;
	unsigned int	PQDC_08_9C;
	unsigned int	PQDC_08_A0;
	unsigned int	PQDC_08_A4;
	unsigned int	PQDC_08_A8;
	unsigned int	PQDC_08_AC;
	unsigned int	PQDC_08_B0;
	unsigned int	PQDC_08_B4;
	unsigned int	PQDC_08_B8;
	unsigned int	PQDC_08_BC;
	unsigned int	PQDC_08_C0;
	unsigned int	PQDC_08_C4;
	unsigned int	PQDC_08_C8;
	unsigned int	PQDC_08_CC;
	unsigned int	PQDC_08_D0;
	unsigned int	PQDC_08_D4;
	unsigned int	PQDC_08_D8;
	unsigned int	PQDC_08_DC;
	unsigned int	PQDC_08_E0;
	unsigned int	PQDC_08_E4;
	unsigned int	PQDC_08_E8;
	unsigned int	PQDC_08_EC;
	unsigned int	PQDC_08_F0;
	unsigned int	PQDC_08_F4;
	unsigned int	PQDC_08_F8;
	unsigned int	PQDC_08_FC;
	//LBMC
	unsigned int	LBMC__LBMC_00;
	unsigned int	LBMC__LBMC_04;
	unsigned int	LBMC__LBMC_08;
	unsigned int	LBMC__LBMC_0C;
	unsigned int	LBMC__LBMC_10;
	unsigned int	LBMC__LBMC_14;
	unsigned int	LBMC__LBMC_18;
	unsigned int	LBMC__LBMC_1C;
	unsigned int	LBMC__LBMC_20;
	unsigned int	LBMC__LBMC_24;
	unsigned int	LBMC__LBMC_28;
	unsigned int	LBMC__LBMC_2C;
	unsigned int	LBMC__LBMC_30;
	unsigned int	LBMC__LBMC_34;
	unsigned int	LBMC__LBMC_38;
	unsigned int	LBMC__LBMC_3C;
	unsigned int	LBMC__LBMC_40;
	unsigned int	LBMC__LBMC_44;
	unsigned int	LBMC__LBMC_48;
	unsigned int	LBMC__LBMC_4C;
	unsigned int	LBMC__LBMC_50;
	unsigned int	LBMC__LBMC_54;
	unsigned int	LBMC__LBMC_58;
	unsigned int	LBMC__LBMC_5C;
	unsigned int	LBMC__LBMC_60;
	unsigned int	LBMC__LBMC_80;
	unsigned int	LBMC__LBMC_84;
	unsigned int	LBMC__LBMC_88;
	//BBD
	unsigned int	BBD__BBD_00;
	unsigned int	BBD__BBD_04;
	unsigned int	BBD__BBD_08;
	unsigned int	BBD__BBD_0C;
	unsigned int	BBD__BBD_10;
	//KMC_METER_TOP
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_00;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_04;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_08;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_0C;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_10;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_14;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_18;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_1C;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_20;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_24;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_28;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_2C;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_30;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_34;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_38;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_3C;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_40;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_44;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_48;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_4C;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_50;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_54;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_58;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_5C;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_60;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_64;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_68;
	unsigned int	KMC_METER_TOP__KMC_METER_TOP_6C;
	//KMC_DM_METER_TOP
	unsigned int	KMC_DM_METER_TOP__KMC_METER_REG0;
	unsigned int	KMC_DM_METER_TOP__KMC_METER_NUM;
	unsigned int	KMC_DM_METER_TOP__KMC_METER_H_NUM0;
	unsigned int	KMC_DM_METER_TOP__KMC_METER_H_NUM1;
	unsigned int	KMC_DM_METER_TOP__METER_TH0;
	unsigned int	KMC_DM_METER_TOP__METER_TH1;
	unsigned int	KMC_DM_METER_TOP__METER_TH3;
	unsigned int	KMC_DM_METER_TOP__METER_TIMER0;
	//KME_TOP
	unsigned int	KME_TOP__KME_TOP_04;
	unsigned int	KME_TOP__KME_TOP_08;
	unsigned int	KME_TOP__KME_TOP_10;
	unsigned int	KME_TOP__KME_TOP_14;
	unsigned int	KME_TOP__KME_TOP_18;
	unsigned int	KME_TOP__KME_TOP_1C;
	unsigned int	KME_TOP__KME_TOP_20;
	unsigned int	KME_TOP__KME_TOP_24;
	unsigned int	KME_TOP__KME_TOP_28;
	unsigned int	KME_TOP__KME_TOP_2C;
	unsigned int	KME_TOP__KME_TOP_30;
	unsigned int	KME_TOP__KME_TOP_34;
	unsigned int	KME_TOP__KME_TOP_38;
	unsigned int	KME_TOP__KME_TOP_3C;
	//KME_VBUF_TOP
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_00;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_04;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_08;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_0C;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_10;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_14;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_18;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_1C;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_20;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_24;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_28;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_2C;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_30;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_34;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_38;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_3C;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_40;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_44;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_48;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_4C;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_50;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_54;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_58;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_5C;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_60;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_64;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_68;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_6C;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_70;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_74;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_78;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_7C;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_80;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_84;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_88;
	unsigned int	KME_VBUF_TOP__KME_VBUF_TOP_8C;
	//KME_ME1_TOP2
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_00;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_04;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_08;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_0C;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_10;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_14;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_18;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_1C;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_20;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_24;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_28;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_2C;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_30;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_34;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_38;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_3C;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_40;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_44;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_48;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_4C;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_50;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_54;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_58;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_5C;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_60;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_64;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_80;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_84;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_88;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_8C;
	unsigned int	KME_ME1_TOP2__KME_ME1_TOP2_90;
	//KME_ME1_TOP3
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_00;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_04;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_08;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_0C;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_10;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_14;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_18;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_1C;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_20;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_24;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_28;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_2C;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_30;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_34;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_38;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_3C;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_40;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_44;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_48;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_4C;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_50;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_54;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_58;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_5C;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_60;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_64;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_68;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_6C;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_70;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_74;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_78;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_7C;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_80;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_84;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_88;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_8C;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_90;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_94;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_98;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_9C;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_A0;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_A4;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_A8;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_AC;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_B0;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_B4;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_B8;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_BC;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_C0;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_C4;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_C8;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_CC;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_D0;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_D4;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_D8;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_DC;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_E0;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_E4;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_E8;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_EC;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_F0;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_F4;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_F8;
	unsigned int	KME_ME1_TOP3__KME_ME1_TOP3_FC;
	//KME_ME1_TOP4
	//KME_ME1_TOP5
	//ME_SHARE_DMA
	unsigned int KME_SHARE_DMA__ME_SHARE_DMA_94;
	unsigned int KME_SHARE_DMA__ME_SHARE_DMA_E0;
	//KME_LBME_TOP
	unsigned int	KME_LBME_TOP__KME_LBME_TOP_00;
	unsigned int	KME_LBME_TOP__KME_LBME_TOP_04;
	unsigned int	KME_LBME_TOP__KME_LBME_TOP_08;
	unsigned int	KME_LBME_TOP__KME_LBME_TOP_0C;
	unsigned int	KME_LBME_TOP__KME_LBME_TOP_10;
	unsigned int	KME_LBME_TOP__KME_LBME_TOP_14;
	unsigned int	KME_LBME_TOP__KME_LBME_TOP_18;
	unsigned int	KME_LBME_TOP__KME_LBME_TOP_1C;
	unsigned int	KME_LBME_TOP__KME_LBME_TOP_20;
	unsigned int	KME_LBME_TOP__KME_LBME_TOP_24;
	//KME_ME2_CALC1
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_00;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_04;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_08;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_0C;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_10;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_14;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_18;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_1C;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_20;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_24;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_28;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_2C;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_30;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_34;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_38;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_3C;
	unsigned int	KME_ME2_CALC1__KME_ME2_CALC1_40;
	//KME_LOGO2
	unsigned int	KME_LOGO2__KME_LOGO2_00;
	unsigned int	KME_LOGO2__KME_LOGO2_04;
	unsigned int	KME_LOGO2__KME_LOGO2_08;
	unsigned int	KME_LOGO2__KME_LOGO2_0C;
	unsigned int	KME_LOGO2__KME_LOGO2_10;
	unsigned int	KME_LOGO2__KME_LOGO2_14;
	unsigned int	KME_LOGO2__KME_LOGO2_18;
	unsigned int	KME_LOGO2__KME_LOGO2_1C;
	unsigned int	KME_LOGO2__KME_LOGO2_20;
	unsigned int	KME_LOGO2__KME_LOGO2_24;
	unsigned int	KME_LOGO2__KME_LOGO2_28;
	unsigned int	KME_LOGO2__KME_LOGO2_2C;
	unsigned int	KME_LOGO2__KME_LOGO2_30;
	unsigned int	KME_LOGO2__KME_LOGO2_34;
	//KME_IPME
	unsigned int	KME_IPME__KME_IPME_00;
	unsigned int	KME_IPME__KME_IPME_04;
	unsigned int	KME_IPME__KME_IPME_08;
	unsigned int	KME_IPME__KME_IPME_0C;
	unsigned int	KME_IPME__KME_IPME_10;
	unsigned int	KME_IPME__KME_IPME_14;
	unsigned int	KME_IPME__KME_IPME_18;
	unsigned int	KME_IPME__KME_IPME_1C;
	unsigned int	KME_IPME__KME_IPME_20;
	unsigned int	KME_IPME__KME_IPME_24;
	unsigned int	KME_IPME__KME_IPME_28;
	unsigned int	KME_IPME__KME_IPME_2C;
	unsigned int	KME_IPME__KME_IPME_30;
	unsigned int	KME_IPME__KME_IPME_34;
	unsigned int	KME_IPME__KME_IPME_38;
	unsigned int	KME_IPME__KME_IPME_3C;
	unsigned int	KME_IPME__KME_IPME_40;
	unsigned int	KME_IPME__KME_IPME_44;
	unsigned int	KME_IPME__KME_IPME_48;
	unsigned int	KME_IPME__KME_IPME_4C;
	unsigned int	KME_IPME__KME_IPME_50;
	unsigned int	KME_IPME__KME_IPME_54;
	unsigned int	KME_IPME__KME_IPME_58;
	unsigned int	KME_IPME__KME_IPME_5C;
	unsigned int	KME_IPME__KME_IPME_60;
	unsigned int	KME_IPME__KME_IPME_64;
	unsigned int	KME_IPME__KME_IPME_68;
	unsigned int	KME_IPME__KME_IPME_6C;
	unsigned int	KME_IPME__KME_IPME_70;
	unsigned int	KME_IPME__KME_IPME_74;
	unsigned int	KME_IPME__KME_IPME_78;
	unsigned int	KME_IPME__KME_IPME_7C;
	unsigned int	KME_IPME__KME_IPME_80;
	unsigned int	KME_IPME__KME_IPME_84;
	unsigned int	KME_IPME__KME_IPME_88;
	unsigned int	KME_IPME__KME_IPME_8C;
	unsigned int	KME_IPME__KME_IPME_90;
	unsigned int	KME_IPME__KME_IPME_94;
	unsigned int	KME_IPME__KME_IPME_98;
	unsigned int	KME_IPME__KME_IPME_9C;
	unsigned int	KME_IPME__KME_IPME_A0;
	//KME_IPME1
	unsigned int	KME_IPME1__KME_IPME1_00;
	unsigned int	KME_IPME1__KME_IPME1_04;
	unsigned int	KME_IPME1__KME_IPME1_08;
	unsigned int	KME_IPME1__KME_IPME1_0C;
	//KME_LOGO0
	unsigned int	KME_LOGO0__KME_LOGO0_00;
	unsigned int	KME_LOGO0__KME_LOGO0_04;
	unsigned int	KME_LOGO0__KME_LOGO0_08;
	unsigned int	KME_LOGO0__KME_LOGO0_0C;
	unsigned int	KME_LOGO0__KME_LOGO0_10;
	unsigned int	KME_LOGO0__KME_LOGO0_14;
	unsigned int	KME_LOGO0__KME_LOGO0_18;
	unsigned int	KME_LOGO0__KME_LOGO0_1C;
	unsigned int	KME_LOGO0__KME_LOGO0_20;
	unsigned int	KME_LOGO0__KME_LOGO0_24;
	unsigned int	KME_LOGO0__KME_LOGO0_28;
	unsigned int	KME_LOGO0__KME_LOGO0_2C;
	unsigned int	KME_LOGO0__KME_LOGO0_30;
	unsigned int	KME_LOGO0__KME_LOGO0_34;
	unsigned int	KME_LOGO0__KME_LOGO0_38;
	unsigned int	KME_LOGO0__KME_LOGO0_3C;
	unsigned int	KME_LOGO0__KME_LOGO0_40;
	unsigned int	KME_LOGO0__KME_LOGO0_44;
	unsigned int	KME_LOGO0__KME_LOGO0_48;
	unsigned int	KME_LOGO0__KME_LOGO0_4C;
	unsigned int	KME_LOGO0__KME_LOGO0_50;
	unsigned int	KME_LOGO0__KME_LOGO0_54;
	unsigned int	KME_LOGO0__KME_LOGO0_58;
	unsigned int	KME_LOGO0__KME_LOGO0_5C;
	unsigned int	KME_LOGO0__KME_LOGO0_60;
	unsigned int	KME_LOGO0__KME_LOGO0_64;
	unsigned int	KME_LOGO0__KME_LOGO0_68;
	unsigned int	KME_LOGO0__KME_LOGO0_6C;
	unsigned int	KME_LOGO0__KME_LOGO0_70;
	unsigned int	KME_LOGO0__KME_LOGO0_74;
	unsigned int	KME_LOGO0__KME_LOGO0_78;
	unsigned int	KME_LOGO0__KME_LOGO0_7C;
	unsigned int	KME_LOGO0__KME_LOGO0_80;
	unsigned int	KME_LOGO0__KME_LOGO0_84;
	unsigned int	KME_LOGO0__KME_LOGO0_88;
	unsigned int	KME_LOGO0__KME_LOGO0_8C;
	unsigned int	KME_LOGO0__KME_LOGO0_90;
	unsigned int	KME_LOGO0__KME_LOGO0_94;
	unsigned int	KME_LOGO0__KME_LOGO0_98;
	unsigned int	KME_LOGO0__KME_LOGO0_9C;
	unsigned int	KME_LOGO0__KME_LOGO0_A0;
	unsigned int	KME_LOGO0__KME_LOGO0_A4;
	unsigned int	KME_LOGO0__KME_LOGO0_A8;
	unsigned int	KME_LOGO0__KME_LOGO0_AC;
	unsigned int	KME_LOGO0__KME_LOGO0_B0;
	unsigned int	KME_LOGO0__KME_LOGO0_B4;
	unsigned int	KME_LOGO0__KME_LOGO0_B8;
	unsigned int	KME_LOGO0__KME_LOGO0_BC;
	unsigned int	KME_LOGO0__KME_LOGO0_C0;
	unsigned int	KME_LOGO0__KME_LOGO0_C4;
	unsigned int	KME_LOGO0__KME_LOGO0_C8;
	unsigned int	KME_LOGO0__KME_LOGO0_CC;
	unsigned int	KME_LOGO0__KME_LOGO0_D0;
	unsigned int	KME_LOGO0__KME_LOGO0_D4;
	unsigned int	KME_LOGO0__KME_LOGO0_D8;
	unsigned int	KME_LOGO0__KME_LOGO0_DC;
	unsigned int	KME_LOGO0__KME_LOGO0_E0;
	unsigned int	KME_LOGO0__KME_LOGO0_E4;
	unsigned int	KME_LOGO0__KME_LOGO0_E8;
	unsigned int	KME_LOGO0__KME_LOGO0_EC;
	unsigned int	KME_LOGO0__KME_LOGO0_F0;
	unsigned int	KME_LOGO0__KME_LOGO0_F4;
	unsigned int	KME_LOGO0__KME_LOGO0_F8;
	unsigned int	KME_LOGO0__KME_LOGO0_FC;
	//KME_LOGO1
	unsigned int	KME_LOGO1__KME_LOGO1_00;
	unsigned int	KME_LOGO1__KME_LOGO1_04;
	unsigned int	KME_LOGO1__KME_LOGO1_08;
	unsigned int	KME_LOGO1__KME_LOGO1_0C;
	unsigned int	KME_LOGO1__KME_LOGO1_10;
	unsigned int	KME_LOGO1__KME_LOGO1_14;
	unsigned int	KME_LOGO1__KME_LOGO1_18;
	unsigned int	KME_LOGO1__KME_LOGO1_1C;
	unsigned int	KME_LOGO1__KME_LOGO1_20;
	unsigned int	KME_LOGO1__KME_LOGO1_2C;
	unsigned int	KME_LOGO1__KME_LOGO1_30;
	unsigned int	KME_LOGO1__KME_LOGO1_34;
	unsigned int	KME_LOGO1__KME_LOGO1_38;
	unsigned int	KME_LOGO1__KME_LOGO1_3C;
	unsigned int	KME_LOGO1__KME_LOGO1_40;
	unsigned int	KME_LOGO1__KME_LOGO1_44;
	unsigned int	KME_LOGO1__KME_LOGO1_48;
	unsigned int	KME_LOGO1__KME_LOGO1_4C;
	unsigned int	KME_LOGO1__KME_LOGO1_50;
	unsigned int	KME_LOGO1__KME_LOGO1_54;
	unsigned int	KME_LOGO1__KME_LOGO1_58;
	unsigned int	KME_LOGO1__KME_LOGO1_5C;
	unsigned int	KME_LOGO1__KME_LOGO1_60;
	unsigned int	KME_LOGO1__KME_LOGO1_64;
	unsigned int	KME_LOGO1__KME_LOGO1_68;
	unsigned int	KME_LOGO1__KME_LOGO1_6C;
	unsigned int	KME_LOGO1__KME_LOGO1_70;
	unsigned int	KME_LOGO1__KME_LOGO1_74;
	unsigned int	KME_LOGO1__KME_LOGO1_78;
	unsigned int	KME_LOGO1__KME_LOGO1_7C;
	unsigned int	KME_LOGO1__KME_LOGO1_80;
	unsigned int	KME_LOGO1__KME_LOGO1_84;
	unsigned int	KME_LOGO1__KME_LOGO1_88;
	unsigned int	KME_LOGO1__KME_LOGO1_8C;
	unsigned int	KME_LOGO1__KME_LOGO1_90;
	unsigned int	KME_LOGO1__KME_LOGO1_94;
	unsigned int	KME_LOGO1__KME_LOGO1_98;
	unsigned int	KME_LOGO1__KME_LOGO1_9C;
	unsigned int	KME_LOGO1__KME_LOGO1_A0;
	unsigned int	KME_LOGO1__KME_LOGO1_A4;
	unsigned int	KME_LOGO1__KME_LOGO1_A8;
	unsigned int	KME_LOGO1__KME_LOGO1_AC;
	unsigned int	KME_LOGO1__KME_LOGO1_B0;
	unsigned int	KME_LOGO1__KME_LOGO1_B4;
	unsigned int	KME_LOGO1__KME_LOGO1_B8;
	unsigned int	KME_LOGO1__KME_LOGO1_BC;
	unsigned int	KME_LOGO1__KME_LOGO1_C0;
	unsigned int	KME_LOGO1__KME_LOGO1_C4;
	unsigned int	KME_LOGO1__KME_LOGO1_C8;
	unsigned int	KME_LOGO1__KME_LOGO1_CC;
	unsigned int	KME_LOGO1__KME_LOGO1_D0;
	unsigned int	KME_LOGO1__KME_LOGO1_D4;
	unsigned int	KME_LOGO1__KME_LOGO1_D8;
	unsigned int	KME_LOGO1__KME_LOGO1_DC;
	unsigned int	KME_LOGO1__KME_LOGO1_E0;
	unsigned int	KME_LOGO1__KME_LOGO1_E4;
	unsigned int	KME_LOGO1__KME_LOGO1_E8;
	unsigned int	KME_LOGO1__KME_LOGO1_EC;
	unsigned int	KME_LOGO1__KME_LOGO1_F0;
	unsigned int	KME_LOGO1__KME_LOGO1_F4;
	unsigned int	KME_LOGO1__KME_LOGO1_F8;
	unsigned int	KME_LOGO1__KME_LOGO1_FC;
	//KME_DM_TOP0
	unsigned int	KME_DM_TOP0__KME_00_START_ADDRESS0;
	unsigned int	KME_DM_TOP0__KME_00_START_ADDRESS1;
	unsigned int	KME_DM_TOP0__KME_00_START_ADDRESS2;
	unsigned int	KME_DM_TOP0__KME_00_START_ADDRESS3;
	unsigned int	KME_DM_TOP0__KME_00_START_ADDRESS4;
	unsigned int	KME_DM_TOP0__KME_00_START_ADDRESS5;
	unsigned int	KME_DM_TOP0__KME_00_END_ADDRESS0;
	unsigned int	KME_DM_TOP0__KME_00_END_ADDRESS1;
	unsigned int	KME_DM_TOP0__KME_00_END_ADDRESS2;
	unsigned int	KME_DM_TOP0__KME_00_END_ADDRESS3;
	unsigned int	KME_DM_TOP0__KME_00_END_ADDRESS4;
	unsigned int	KME_DM_TOP0__KME_00_END_ADDRESS5;
	unsigned int	KME_DM_TOP0__KME_00_LR_OFFSET_ADDR;
	unsigned int	KME_DM_TOP0__KME_00_LINE_OFFSET_ADDR;
	unsigned int	KME_DM_TOP0__KME_00_AGENT;
	unsigned int	KME_DM_TOP0__KME_00_THRESHOLD;
	unsigned int	KME_DM_TOP0__KME_01_AGENT;
	unsigned int	KME_DM_TOP0__KME_01_THRESHOLD;
	unsigned int	KME_DM_TOP0__KME_02_AGENT;
	unsigned int	KME_DM_TOP0__KME_02_THRESHOLD;
	unsigned int	KME_DM_TOP0__KME_03_AGENT;
	unsigned int	KME_DM_TOP0__KME_03_THRESHOLD;
	unsigned int	KME_DM_TOP0__KME_04_START_ADDRESS0;
	unsigned int	KME_DM_TOP0__KME_04_START_ADDRESS1;
	unsigned int	KME_DM_TOP0__KME_04_END_ADDRESS0;
	unsigned int	KME_DM_TOP0__KME_04_END_ADDRESS1;
	unsigned int	KME_DM_TOP0__KME_04_LR_OFFSET_ADDR;
	unsigned int	KME_DM_TOP0__KME_04_LINE_OFFSET_ADDR;
	unsigned int	KME_DM_TOP0__KME_04_AGENT;
	unsigned int	KME_DM_TOP0__KME_04_THRESHOLD;
	unsigned int	KME_DM_TOP0__KME_05_START_ADDRESS0;
	unsigned int	KME_DM_TOP0__KME_05_START_ADDRESS1;
	unsigned int	KME_DM_TOP0__KME_05_END_ADDRESS0;
	unsigned int	KME_DM_TOP0__KME_05_END_ADDRESS1;
	unsigned int	KME_DM_TOP0__KME_05_LR_OFFSET_ADDR;
	unsigned int	KME_DM_TOP0__KME_05_LINE_OFFSET_ADDR;
	unsigned int	KME_DM_TOP0__KME_05_AGENT;
	unsigned int	KME_DM_TOP0__KME_05_THRESHOLD;
	unsigned int	KME_DM_TOP0__KME06AGENT;
	unsigned int	KME_DM_TOP0__KME_06_THRESHOLD;
	unsigned int	KME_DM_TOP0__KME_07_AGENT;
	unsigned int	KME_DM_TOP0__KME_07_THRESHOLD;
	unsigned int	KME_DM_TOP0__KME_ME_RESOLUTION;
	unsigned int	KME_DM_TOP0__KME_MV_RESOLUTION;
	unsigned int	KME_DM_TOP0__KME_SOURCE_MODE;
	unsigned int	KME_DM_TOP0__KME_ME_HNUM;
	unsigned int	KME_DM_TOP0__KME_TIMER_ENABLE;
	unsigned int	KME_DM_TOP0__KME_HOLD_TIME0;
	unsigned int	KME_DM_TOP0__KME_HOLD_TIME1;
	unsigned int	KME_DM_TOP0__KME_HOLD_TIME2;
	unsigned int	KME_DM_TOP0__KME_HOLD_TIME3;
	unsigned int	KME_DM_TOP0__KME_DM_TOP0_CC;
	unsigned int	KME_DM_TOP0__WAIT_TIME1;
	unsigned int	KME_DM_TOP0__WAIT_TIME2;
	unsigned int	KME_DM_TOP0__MV04_START_ADDRESS2;
	unsigned int	KME_DM_TOP0__MV04_END_ADDRESS2;
	//KME_DM_TOP1
	unsigned int	KME_DM_TOP1__KME_08_START_ADDRESS0;
	unsigned int	KME_DM_TOP1__KME_08_START_ADDRESS1;
	unsigned int	KME_DM_TOP1__KME_08_END_ADDRESS0;
	unsigned int	KME_DM_TOP1__KME_08_END_ADDRESS1;
	unsigned int	KME_DM_TOP1__KME_08_LINE_OFFSET_ADDR;
	unsigned int	KME_DM_TOP1__KME_08_AGENT;
	unsigned int	KME_DM_TOP1__KME_08_THRESHOLD;
	unsigned int	KME_DM_TOP1__KME_09_AGENT;
	unsigned int	KME_DM_TOP1__KME_09_THRESHOLD;
	unsigned int	KME_DM_TOP1__KME_10_START_ADDRESS0;
	unsigned int	KME_DM_TOP1__KME_10_START_ADDRESS1;
	unsigned int	KME_DM_TOP1__KME_10_END_ADDRESS0;
	unsigned int	KME_DM_TOP1__KME_10_END_ADDRESS1;
	unsigned int	KME_DM_TOP1__KME_10_LINE_OFFSET_ADDR;
	unsigned int	KME_DM_TOP1__KME_10_AGENT;
	unsigned int	KME_DM_TOP1__KME_10_THRESHOLD;
	unsigned int	KME_DM_TOP1__KME_11_AGENT;
	unsigned int	KME_DM_TOP1__KME_11_THRESHOLD;
	unsigned int	KME_DM_TOP1__KME_12_START_ADDRESS0;
	unsigned int	KME_DM_TOP1__KME_12_START_ADDRESS1;
	unsigned int	KME_DM_TOP1__KME_12_END_ADDRESS0;
	unsigned int	KME_DM_TOP1__KME_12_END_ADDRESS1;
	unsigned int	KME_DM_TOP1__KME_12_LINE_OFFSET_ADDR;
	unsigned int	KME_DM_TOP1__KME_12_AGENT;
	unsigned int	KME_DM_TOP1__KME_12_THRESHOLD;
	unsigned int	KME_DM_TOP1__KME_13_AGENT;
	unsigned int	KME_DM_TOP1__KME_13_THRESHOLD;
	unsigned int	KME_DM_TOP1__KME_14_START_ADDRESS0;
	unsigned int	KME_DM_TOP1__KME_14_START_ADDRESS1;
	unsigned int	KME_DM_TOP1__KME_14_END_ADDRESS0;
	unsigned int	KME_DM_TOP1__KME_14_END_ADDRESS1;
	unsigned int	KME_DM_TOP1__KME_14_LINE_OFFSET_ADDR;
	unsigned int	KME_DM_TOP1__KME_14_AGENT;
	unsigned int	KME_DM_TOP1__KME_14_THRESHOLD;
	unsigned int	KME_DM_TOP1__KME_15_AGENT;
	unsigned int	KME_DM_TOP1__KME_15_THRESHOLD;
	unsigned int	KME_DM_TOP1__KME_IPLOGO_RESOLUTION;
	unsigned int	KME_DM_TOP1__KME_MLOGO_RESOLUTION;
	unsigned int	KME_DM_TOP1__KME_PLOGO_RESOLUTION;
	unsigned int	KME_DM_TOP1__KME_HLOGO_RESOLUTION;
	unsigned int	KME_DM_TOP1__KME_LOGO_HNUM;
	unsigned int	KME_DM_TOP1__KME_CLR;
	unsigned int	KME_DM_TOP1__TH0;
	unsigned int	KME_DM_TOP1__TH1;
	unsigned int	KME_DM_TOP1__TH2;
	unsigned int	KME_DM_TOP1__TH3;
	unsigned int	KME_DM_TOP1__TH4;
	unsigned int	KME_DM_TOP1__METER_TIMER1;
	//KME_DM_TOP2
	unsigned int	KME_DM_TOP2__MV01_START_ADDRESS0;
	unsigned int	KME_DM_TOP2__MV01_END_ADDRESS0;
	unsigned int	KME_DM_TOP2__MV_01_LINE_OFFSET_ADDR;
	unsigned int	KME_DM_TOP2__MV01_LR_OFFSET_ADDR;
	unsigned int	KME_DM_TOP2__MV01_AGENT;
	unsigned int	KME_DM_TOP2__MV02_START_ADDRESS11;
	unsigned int	KME_DM_TOP2__MV02_END_ADDRESS0;
	unsigned int	KME_DM_TOP2__MV_02_LINE_OFFSET_ADDR;
	unsigned int	KME_DM_TOP2__MV02_LR_OFFSET_ADDR;
	unsigned int	KME_DM_TOP2__MV02_AGENT;
	unsigned int	KME_DM_TOP2__MV03_AGENT;
	unsigned int	KME_DM_TOP2__MV04_START_ADDRESS0;
	unsigned int	KME_DM_TOP2__MV04_START_ADDRESS1;
	unsigned int	KME_DM_TOP2__MV04_END_ADDRESS0;
	unsigned int	KME_DM_TOP2__MV04_END_ADDRESS1;
	unsigned int	KME_DM_TOP2__MV_04_LINE_OFFSET_ADDR;
	unsigned int	KME_DM_TOP2__MV04_LR_OFFSET_ADDR;
	unsigned int	KME_DM_TOP2__MV04_AGENT;
	unsigned int	KME_DM_TOP2__MV05_START_ADDRESS0;
	unsigned int	KME_DM_TOP2__MV05_START_ADDRESS1;
	unsigned int	KME_DM_TOP2__MV05_END_ADDRESS0;
	unsigned int	KME_DM_TOP2__MV05_END_ADDRESS1;
	unsigned int	KME_DM_TOP2__MV_05_LINE_OFFSET_ADDR;
	unsigned int	KME_DM_TOP2__MV05_LR_OFFSET_ADDR;
	unsigned int	KME_DM_TOP2__MV05_AGENT;
	unsigned int	KME_DM_TOP2__MV06_AGENT;
	unsigned int	KME_DM_TOP2__MV07_AGENT;
	unsigned int	KME_DM_TOP2__MV08_AGENT;
	unsigned int	KME_DM_TOP2__MV09_AGENT;
	unsigned int	KME_DM_TOP2__MV10_AGENT;
	unsigned int	KME_DM_TOP2__MV11_AGENT;
	unsigned int	KME_DM_TOP2__KME_00_START_ADDRESS6;
	unsigned int	KME_DM_TOP2__KME_00_START_ADDRESS7;
	unsigned int	KME_DM_TOP2__KME_00_END_ADDRESS6;
	unsigned int	KME_DM_TOP2__KME_00_END_ADDRESS7;
	unsigned int	KME_DM_TOP2__MV_SOURCE_MODE;
	unsigned int	KME_DM_TOP2__MV01_RESOLUTION;
	unsigned int	KME_DM_TOP2__MV02_RESOLUTION;
	unsigned int	KME_DM_TOP2__MV04_RESOLUTION;
	unsigned int	KME_DM_TOP2__MV05_RESOLUTION;
	unsigned int	KME_DM_TOP2__MV_TIME_ENABLE;
	unsigned int	KME_DM_TOP2__MV_HOLD_TIME0;
	unsigned int	KME_DM_TOP2__MV_HOLD_TIME1;
	unsigned int	KME_DM_TOP2__MV_HOLD_TIME2;
	unsigned int	KME_DM_TOP2__MV_WAIT_TIME0;
	unsigned int	KME_DM_TOP2__MV_WAIT_TIME1;
	//KME_DEHALO3
	unsigned int	KME_DEHALO3__KME_DEHALO3_0C;
	//KME_ME1_TOP0
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_00;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_04;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_08;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_0C;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_10;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_14;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_18;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_1C;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_20;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_24;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_28;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_2C;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_30;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_34;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_38;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_40;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_44;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_48;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_4C;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_50;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_54;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_58;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_5C;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_60;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_64;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_68;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_6C;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_70;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_74;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_78;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_7C;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_80;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_84;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_88;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_8C;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_90;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_94;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_98;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_9C;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_A0;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_A4;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_B0;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_B4;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_B8;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_BC;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_C0;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_C4;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_EC;
	unsigned int	KME_ME1_TOP0__KME_ME1_TOP0_F0;
	//KME_ME1_TOP1
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_00;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_01;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_02;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_03;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_04;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_05;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_06;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_07;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_08;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_09;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_10;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_11;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_12;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_13;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_14;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_15;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_16;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_17;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_18;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_19;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_20;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_21;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_22;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_23;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_24;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_25;
	unsigned int	KME_ME1_TOP1__KME_ME1_TOP1_68;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_27;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_28;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_29;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_30;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_31;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_32;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_33;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_34;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_35;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_36;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_37;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_38;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_39;
	unsigned int	KME_ME1_TOP1__ME1_COMMON1_40;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_00;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_01;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_02;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_03;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_04;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_05;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_06;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_07;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_08;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_09;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_10;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_11;
	unsigned int	KME_ME1_TOP1__ME1_SCENE1_12;
	unsigned int	KME_ME1_TOP1__KME_ME1_TOP1_F4;
	//KME_LBME2_TOP
	unsigned int	KME_LBME2_TOP__KME_LBME2_TOP_00;
	unsigned int	KME_LBME2_TOP__KME_LBME2_TOP_04;
	unsigned int	KME_LBME2_TOP__KME_LBME2_TOP_08;
	unsigned int	KME_LBME2_TOP__KME_LBME2_TOP_0C;
	unsigned int	KME_LBME2_TOP__KME_LBME2_TOP_10;
	unsigned int	KME_LBME2_TOP__KME_LBME2_TOP_14;
	unsigned int	KME_LBME2_TOP__KME_LBME2_TOP_18;
	//KME_ME2_VBUF_TOP
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_00;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_04;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_08;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_0C;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_10;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_14;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_18;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_1C;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_20;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_24;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_28;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_2C;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_30;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_34;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_38;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_3C;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_40;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_44;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_48;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_4C;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_50;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_54;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_58;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_5C;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_60;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_64;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_68;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_6C;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_70;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_74;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_78;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_7C;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_80;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_84;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_88;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_8C;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_90;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_94;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_98;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_9C;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A0;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A4;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_A8;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_AC;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B0;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B4;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_B8;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_BC;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C0;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C4;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_C8;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_CC;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D0;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D4;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_D8;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_DC;
	unsigned int	KME_ME2_VBUF_TOP__KME_ME2_VBUF_TOP_E0;
	//KME_ME2_CALC0
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_00;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_04;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_08;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_0C;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_10;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_14;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_18;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_1C;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_20;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_24;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_28;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_2C;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_30;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_34;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_38;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_3C;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_40;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_44;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_48;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_4C;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_50;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_54;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_60;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_64;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_68;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_6C;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_70;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_74;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_78;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_7C;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_80;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_84;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_88;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_8C;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_90;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_94;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_98;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_9C;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_A0;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_A4;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_A8;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_AC;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_B0;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_B4;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_B8;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_BC;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_C0;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_C4;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_C8;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_CC;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_D0;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_D4;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_D8;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_DC;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_E0;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_E4;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_E8;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_EC;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_F0;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_F4;
	unsigned int	KME_ME2_CALC0__KME_ME2_CALC0_F8;
	//KME_ME1_TOP6
	unsigned int	KME_ME1_TOP6__KME_ME1_TOP6_00;
	unsigned int	KME_ME1_TOP6__KME_ME1_TOP6_04;
	unsigned int	KME_ME1_TOP6__KME_ME1_TOP6_08;
	unsigned int	KME_ME1_TOP6__KME_ME1_TOP6_0C;
	unsigned int	KME_ME1_TOP6__KME_ME1_TOP6_10;
	unsigned int	KME_ME1_TOP6__KME_ME1_TOP6_14;
	unsigned int	KME_ME1_TOP6__KME_ME1_TOP6_18;
	unsigned int	KME_ME1_TOP6__KME_ME1_TOP6_1C;
	unsigned int	KME_ME1_TOP6__KME_ME1_TOP6_20;
	//KME_ME1_TOP7
	//KME_ME1_TOP8
	//KME_ME1_TOP9
	//KME_DEHALO
	unsigned int	KME_DEHALO__KME_DEHALO_10;
	unsigned int	KME_DEHALO__KME_DEHALO_14;
	unsigned int	KME_DEHALO__KME_DEHALO_18;
	unsigned int	KME_DEHALO__KME_DEHALO_1C;
	unsigned int	KME_DEHALO__KME_DEHALO_20;
	unsigned int	KME_DEHALO__KME_DEHALO_24;
	unsigned int	KME_DEHALO__KME_DEHALO_28;
	unsigned int	KME_DEHALO__KME_DEHALO_2C;
	unsigned int	KME_DEHALO__KME_DEHALO_30;
	unsigned int	KME_DEHALO__KME_DEHALO_34;
	unsigned int	KME_DEHALO__KME_DEHALO_38;
	unsigned int	KME_DEHALO__KME_DEHALO_3C;
	unsigned int	KME_DEHALO__KME_DEHALO_40;
	unsigned int	KME_DEHALO__KME_DEHALO_44;
	unsigned int	KME_DEHALO__KME_DEHALO_48;
	unsigned int	KME_DEHALO__KME_DEHALO_4C;
	unsigned int	KME_DEHALO__KME_DEHALO_50;
	unsigned int	KME_DEHALO__KME_DEHALO_54;
	unsigned int	KME_DEHALO__KME_DEHALO_58;
	unsigned int	KME_DEHALO__KME_DEHALO_5C;
	unsigned int	KME_DEHALO__KME_DEHALO_60;
	unsigned int	KME_DEHALO__KME_DEHALO_64;
	unsigned int	KME_DEHALO__KME_DEHALO_68;
	unsigned int	KME_DEHALO__KME_DEHALO_6C;
	unsigned int	KME_DEHALO__KME_DEHALO_70;
	unsigned int	KME_DEHALO__KME_DEHALO_74;
	unsigned int	KME_DEHALO__KME_DEHALO_78;
	unsigned int	KME_DEHALO__KME_DEHALO_7C;
	unsigned int	KME_DEHALO__KME_DEHALO_80;
	unsigned int	KME_DEHALO__KME_DEHALO_84;
	unsigned int	KME_DEHALO__KME_DEHALO_88;
	unsigned int	KME_DEHALO__KME_DEHALO_8C;
	unsigned int	KME_DEHALO__KME_DEHALO_90;
	unsigned int	KME_DEHALO__KME_DEHALO_94;
	unsigned int	KME_DEHALO__KME_DEHALO_A4;
	unsigned int	KME_DEHALO__KME_DEHALO_A8;
	unsigned int	KME_DEHALO__KME_DEHALO_AC;
	unsigned int	KME_DEHALO__KME_DEHALO_B0;
	unsigned int	KME_DEHALO__KME_DEHALO_B4;
	unsigned int	KME_DEHALO__KME_DEHALO_B8;
	unsigned int	KME_DEHALO__KME_DEHALO_BC;
	unsigned int	KME_DEHALO__KME_DEHALO_C0;
	unsigned int	KME_DEHALO__KME_DEHALO_C4;
	unsigned int	KME_DEHALO__KME_DEHALO_C8;
	unsigned int	KME_DEHALO__KME_DEHALO_CC;
	unsigned int	KME_DEHALO__KME_DEHALO_D0;
	unsigned int	KME_DEHALO__KME_DEHALO_D4;
	unsigned int	KME_DEHALO__KME_DEHALO_D8;
	unsigned int	KME_DEHALO__KME_DEHALO_DC;
	unsigned int	KME_DEHALO__KME_DEHALO_E0;
	unsigned int	KME_DEHALO__KME_DEHALO_E4;
	unsigned int	KME_DEHALO__KME_DEHALO_E8;
	unsigned int	KME_DEHALO__KME_DEHALO_EC;
	unsigned int	KME_DEHALO__KME_DEHALO_F0;
	unsigned int	KME_DEHALO__KME_DEHALO_F4;
	unsigned int	KME_DEHALO__KME_DEHALO_F8;
	unsigned int	KME_DEHALO__KME_DEHALO_FC;
	//KME_DEHALO2
	unsigned int	KME_DEHALO2__KME_DEHALO2_08;
	unsigned int	KME_DEHALO2__KME_DEHALO2_10;
	unsigned int	KME_DEHALO2__KME_DEHALO2_14;
	unsigned int	KME_DEHALO2__KME_DEHALO2_18;
	unsigned int	KME_DEHALO2__KME_DEHALO2_1C;
	unsigned int	KME_DEHALO2__KME_DEHALO2_20;
	unsigned int	KME_DEHALO2__KME_DEHALO2_24;
	unsigned int	KME_DEHALO2__KME_DEHALO2_28;
	unsigned int	KME_DEHALO2__KME_DEHALO2_2C;
	unsigned int	KME_DEHALO2__KME_DEHALO2_30;
	unsigned int	KME_DEHALO2__KME_DEHALO2_34;
	unsigned int	KME_DEHALO2__KME_DEHALO2_38;
	unsigned int	KME_DEHALO2__KME_DEHALO2_3C;
	unsigned int	KME_DEHALO2__KME_DEHALO2_40;
	unsigned int	KME_DEHALO2__KME_DEHALO2_44;
	unsigned int	KME_DEHALO2__KME_DEHALO2_48;
	unsigned int	KME_DEHALO2__KME_DEHALO2_4C;
	unsigned int	KME_DEHALO2__KME_DEHALO2_50;
	unsigned int	KME_DEHALO2__KME_DEHALO2_54;
	unsigned int	KME_DEHALO2__KME_DEHALO2_58;
	unsigned int	KME_DEHALO2__KME_DEHALO2_74;
	unsigned int	KME_DEHALO2__KME_DEHALO2_78;
	unsigned int	KME_DEHALO2__KME_DEHALO2_7C;
	unsigned int	KME_DEHALO2__KME_DEHALO2_80;
	unsigned int	KME_DEHALO2__KME_DEHALO2_84;
	unsigned int	KME_DEHALO2__KME_DEHALO2_88;
	unsigned int	KME_DEHALO2__KME_DEHALO2_8C;
	unsigned int	KME_DEHALO2__KME_DEHALO2_90;
	unsigned int	KME_DEHALO2__KME_DEHALO2_94;
	unsigned int	KME_DEHALO2__KME_DEHALO2_98;
	unsigned int	KME_DEHALO2__KME_DEHALO2_9C;
	unsigned int	KME_DEHALO2__KME_DEHALO2_A0;
	unsigned int	KME_DEHALO2__KME_DEHALO2_A4;
	unsigned int	KME_DEHALO2__KME_DEHALO2_A8;
	unsigned int	KME_DEHALO2__KME_DEHALO2_AC;
	unsigned int	KME_DEHALO2__KME_DEHALO2_B0;
	unsigned int	KME_DEHALO2__KME_DEHALO2_B4;
	unsigned int	KME_DEHALO2__KME_DEHALO2_B8;
	unsigned int	KME_DEHALO2__KME_DEHALO2_BC;
	unsigned int	KME_DEHALO2__KME_DEHALO2_C0;
	unsigned int	KME_DEHALO2__KME_DEHALO2_C4;
	unsigned int	KME_DEHALO2__KME_DEHALO2_C8;
	unsigned int	KME_DEHALO2__KME_DEHALO2_CC;
	unsigned int	KME_DEHALO2__KME_DEHALO2_D0;
	unsigned int	KME_DEHALO2__KME_DEHALO2_D4;
	unsigned int	KME_DEHALO2__KME_DEHALO2_D8;
	unsigned int	KME_DEHALO2__KME_DEHALO2_DC;
	unsigned int	KME_DEHALO2__KME_DEHALO2_E0;
	unsigned int	KME_DEHALO2__KME_DEHALO2_E4;
	unsigned int	KME_DEHALO2__KME_DEHALO2_E8;
	//KPOST_TOP
	unsigned int	KPOST_TOP__KPOST_TOP_00;
	unsigned int	KPOST_TOP__KPOST_TOP_04;
	unsigned int	KPOST_TOP__KPOST_TOP_08;
	unsigned int	KPOST_TOP__KPOST_TOP_0C;
	unsigned int	KPOST_TOP__KPOST_TOP_10;
	unsigned int	KPOST_TOP__KPOST_TOP_14;
	unsigned int	KPOST_TOP__KPOST_TOP_18;
	unsigned int	KPOST_TOP__KPOST_TOP_1C;
	unsigned int	KPOST_TOP__KPOST_TOP_20;
	unsigned int	KPOST_TOP__KPOST_TOP_24;
	unsigned int	KPOST_TOP__KPOST_TOP_28;
	unsigned int	KPOST_TOP__KPOST_TOP_30;
	unsigned int	KPOST_TOP__KPOST_TOP_34;
	unsigned int	KPOST_TOP__KPOST_TOP_38;
	unsigned int	KPOST_TOP__KPOST_TOP_3C;
	unsigned int	KPOST_TOP__KPOST_TOP_40;
	unsigned int	KPOST_TOP__KPOST_TOP_44;
	unsigned int	KPOST_TOP__KPOST_TOP_48;
	unsigned int	KPOST_TOP__KPOST_TOP_4C;
	unsigned int	KPOST_TOP__KPOST_TOP_50;
	unsigned int	KPOST_TOP__KPOST_TOP_60;
	unsigned int	KPOST_TOP__KPOST_TOP_64;
	unsigned int	KPOST_TOP__KPOST_TOP_68;
	unsigned int	KPOST_TOP__KPOST_TOP_6C;
	unsigned int	KPOST_TOP__KPOST_TOP_80;
	unsigned int	KPOST_TOP__KPOST_TOP_84;
	unsigned int	KPOST_TOP__KPOST_TOP_88;
	unsigned int	KPOST_TOP__KPOST_TOP_8C;
	unsigned int	KPOST_TOP__KPOST_TOP_90;
	unsigned int	KPOST_TOP__KPOST_TOP_94;
	unsigned int	KPOST_TOP__KPOST_TOP_A0;
	unsigned int	KPOST_TOP__KPOST_TOP_A4;
	unsigned int	KPOST_TOP__KPOST_TOP_A8;
	unsigned int	KPOST_TOP__KPOST_TOP_AC;
	unsigned int	KPOST_TOP__KPOST_TOP_B0;
	unsigned int	KPOST_TOP__KPOST_TOP_B4;
	unsigned int	KPOST_TOP__KPOST_TOP_B8;
	unsigned int	KPOST_TOP__KPOST_TOP_BC;
	//CRTC1
	unsigned int	CRTC1__CRTC1_00;
	unsigned int	CRTC1__CRTC1_04;
	unsigned int	CRTC1__CRTC1_08;
	unsigned int	CRTC1__CRTC1_0C;
	unsigned int	CRTC1__CRTC1_10;
	unsigned int	CRTC1__CRTC1_14;
	unsigned int	CRTC1__CRTC1_18;
	unsigned int	CRTC1__CRTC1_1C;
	unsigned int	CRTC1__CRTC1_20;
	unsigned int	CRTC1__CRTC1_24;
	unsigned int	CRTC1__CRTC1_28;
	unsigned int	CRTC1__CRTC1_2C;
	unsigned int	CRTC1__CRTC1_30;
	unsigned int	CRTC1__CRTC1_34;
	unsigned int	CRTC1__CRTC1_38;
	unsigned int	CRTC1__CRTC1_3C;
	unsigned int	CRTC1__CRTC1_40;
	unsigned int	CRTC1__CRTC1_44;
	unsigned int	CRTC1__CRTC1_48;
	unsigned int	CRTC1__CRTC1_60;
	unsigned int	CRTC1__CRTC1_64;
	unsigned int	CRTC1__CRTC1_68;
	unsigned int	CRTC1__CRTC1_6C;
	unsigned int	CRTC1__CRTC1_70;
	unsigned int	CRTC1__CRTC1_74;
	unsigned int	CRTC1__CRTC1_78;
	unsigned int	CRTC1__CRTC1_7C;
	unsigned int	CRTC1__CRTC1_80;
	unsigned int	CRTC1__CRTC1_84;
	unsigned int	CRTC1__CRTC1_88;
	unsigned int	CRTC1__CRTC1_8C;
	unsigned int	CRTC1__CRTC1_90;
	unsigned int	CRTC1__CRTC1_94;
	//kphase
	unsigned int	KPHASE__KPHASE_00;
	unsigned int	KPHASE__KPHASE_04;
	unsigned int	KPHASE__KPHASE_08;
	unsigned int	KPHASE__KPHASE_0C;
	unsigned int	KPHASE__KPHASE_10;
	unsigned int	KPHASE__KPHASE_14;
	unsigned int	KPHASE__KPHASE_18;
	unsigned int	KPHASE__KPHASE_1C;
	unsigned int	KPHASE__KPHASE_20;
	unsigned int	KPHASE__KPHASE_24;
	unsigned int	KPHASE__KPHASE_28;
	unsigned int	KPHASE__KPHASE_2C;
	unsigned int	KPHASE__KPHASE_30;
	unsigned int	KPHASE__KPHASE_34;
	unsigned int	KPHASE__KPHASE_38;
	unsigned int	KPHASE__KPHASE_3C;
	unsigned int	KPHASE__KPHASE_40;
	unsigned int	KPHASE__KPHASE_44;
	unsigned int	KPHASE__KPHASE_48;
	unsigned int	KPHASE__KPHASE_4C;
	unsigned int	KPHASE__KPHASE_50;
	unsigned int	KPHASE__KPHASE_54;
	unsigned int	KPHASE__KPHASE_58;
	unsigned int	KPHASE__KPHASE_5C;
	unsigned int	KPHASE__KPHASE_60;
	unsigned int	KPHASE__KPHASE_64;
	unsigned int	KPHASE__KPHASE_68;
	unsigned int	KPHASE__KPHASE_6C;
	unsigned int	KPHASE__KPHASE_70;
	unsigned int	KPHASE__KPHASE_74;
	unsigned int	KPHASE__KPHASE_78;
	//Hardware
	unsigned int	HARDWARE__HARDWARE_00;
	unsigned int	HARDWARE__HARDWARE_01;
	unsigned int	HARDWARE__HARDWARE_02;
	unsigned int	HARDWARE__HARDWARE_03;
	unsigned int	HARDWARE__HARDWARE_04;
	unsigned int	HARDWARE__HARDWARE_05;
	unsigned int	HARDWARE__HARDWARE_06;
	unsigned int	HARDWARE__HARDWARE_07;
	unsigned int	HARDWARE__HARDWARE_08;
	unsigned int	HARDWARE__HARDWARE_09;
	unsigned int	HARDWARE__HARDWARE_10;
	unsigned int	HARDWARE__HARDWARE_11;
	unsigned int	HARDWARE__HARDWARE_12;
	unsigned int	HARDWARE__HARDWARE_13;
	unsigned int	HARDWARE__HARDWARE_14;
	unsigned int	HARDWARE__HARDWARE_15;
	unsigned int	HARDWARE__HARDWARE_16;
	unsigned int	HARDWARE__HARDWARE_17;
	unsigned int	HARDWARE__HARDWARE_18;
	unsigned int	HARDWARE__HARDWARE_19;
	unsigned int	HARDWARE__HARDWARE_20;
	unsigned int	HARDWARE__HARDWARE_21;
	unsigned int	HARDWARE__HARDWARE_22;
	unsigned int	HARDWARE__HARDWARE_23;
	unsigned int	HARDWARE__HARDWARE_24;
	unsigned int	HARDWARE__HARDWARE_25;
	unsigned int	HARDWARE__HARDWARE_26;
	unsigned int	HARDWARE__HARDWARE_27;
	unsigned int	HARDWARE__HARDWARE_28;
	unsigned int	HARDWARE__HARDWARE_29;
	unsigned int	HARDWARE__HARDWARE_30;
	unsigned int	HARDWARE__HARDWARE_31;
	unsigned int	HARDWARE__HARDWARE_32;
	unsigned int	HARDWARE__HARDWARE_33;
	unsigned int	HARDWARE__HARDWARE_34;
	unsigned int	HARDWARE__HARDWARE_35;
	unsigned int	HARDWARE__HARDWARE_36;
	unsigned int	HARDWARE__HARDWARE_37;
	unsigned int	HARDWARE__HARDWARE_38;
	unsigned int	HARDWARE__HARDWARE_39;
	unsigned int	HARDWARE__HARDWARE_40;
	unsigned int	HARDWARE__HARDWARE_41;
	unsigned int	HARDWARE__HARDWARE_42;
	unsigned int	HARDWARE__HARDWARE_43;
	unsigned int	HARDWARE__HARDWARE_44;
	unsigned int	HARDWARE__HARDWARE_45;
	unsigned int	HARDWARE__HARDWARE_46;
	unsigned int	HARDWARE__HARDWARE_47;
	unsigned int	HARDWARE__HARDWARE_48;
	unsigned int	HARDWARE__HARDWARE_49;
	unsigned int	HARDWARE__HARDWARE_50;
	unsigned int	HARDWARE__HARDWARE_51;
	unsigned int	HARDWARE__HARDWARE_52;
	unsigned int	HARDWARE__HARDWARE_53;
	unsigned int	HARDWARE__HARDWARE_54;
	unsigned int	HARDWARE__HARDWARE_55;
	unsigned int	HARDWARE__HARDWARE_56;
	unsigned int	HARDWARE__HARDWARE_57;
	unsigned int	HARDWARE__HARDWARE_58;
	unsigned int	HARDWARE__HARDWARE_59;
	unsigned int	HARDWARE__HARDWARE_60;
	unsigned int	HARDWARE__HARDWARE_61;
	unsigned int	HARDWARE__HARDWARE_62;
	unsigned int	HARDWARE__HARDWARE_63;
	//Software
	unsigned int	SOFTWARE__SOFTWARE_00;
	unsigned int	SOFTWARE__SOFTWARE_01;
	unsigned int	SOFTWARE__SOFTWARE_02;
	unsigned int	SOFTWARE__SOFTWARE_03;
	unsigned int	SOFTWARE__SOFTWARE_04;
	unsigned int	SOFTWARE__SOFTWARE_05;
	unsigned int	SOFTWARE__SOFTWARE_06;
	unsigned int	SOFTWARE__SOFTWARE_07;
	unsigned int	SOFTWARE__SOFTWARE_08;
	unsigned int	SOFTWARE__SOFTWARE_09;
	unsigned int	SOFTWARE__SOFTWARE_10;
	unsigned int	SOFTWARE__SOFTWARE_11;
	unsigned int	SOFTWARE__SOFTWARE_12;
	unsigned int	SOFTWARE__SOFTWARE_13;
	unsigned int	SOFTWARE__SOFTWARE_14;
	unsigned int	SOFTWARE__SOFTWARE_15;
	unsigned int	SOFTWARE__SOFTWARE_16;
	unsigned int	SOFTWARE__SOFTWARE_17;
	unsigned int	SOFTWARE__SOFTWARE_18;
	unsigned int	SOFTWARE__SOFTWARE_19;
	unsigned int	SOFTWARE__SOFTWARE_20;
	unsigned int	SOFTWARE__SOFTWARE_21;
	unsigned int	SOFTWARE__SOFTWARE_22;
	unsigned int	SOFTWARE__SOFTWARE_23;
	unsigned int	SOFTWARE__SOFTWARE_24;
	unsigned int	SOFTWARE__SOFTWARE_25;
	unsigned int	SOFTWARE__SOFTWARE_26;
	unsigned int	SOFTWARE__SOFTWARE_27;
	unsigned int	SOFTWARE__SOFTWARE_28;
	unsigned int	SOFTWARE__SOFTWARE_29;
	unsigned int	SOFTWARE__SOFTWARE_30;
	unsigned int	SOFTWARE__SOFTWARE_31;
	unsigned int	SOFTWARE__SOFTWARE_32;
	unsigned int	SOFTWARE__SOFTWARE_33;
	unsigned int	SOFTWARE__SOFTWARE_34;
	unsigned int	SOFTWARE__SOFTWARE_35;
	unsigned int	SOFTWARE__SOFTWARE_36;
	unsigned int	SOFTWARE__SOFTWARE_37;
	unsigned int	SOFTWARE__SOFTWARE_38;
	unsigned int	SOFTWARE__SOFTWARE_39;
	unsigned int	SOFTWARE__SOFTWARE_40;
	unsigned int	SOFTWARE__SOFTWARE_41;
	unsigned int	SOFTWARE__SOFTWARE_42;
	unsigned int	SOFTWARE__SOFTWARE_43;
	unsigned int	SOFTWARE__SOFTWARE_44;
	unsigned int	SOFTWARE__SOFTWARE_45;
	unsigned int	SOFTWARE__SOFTWARE_46;
	unsigned int	SOFTWARE__SOFTWARE_47;
	unsigned int	SOFTWARE__SOFTWARE_48;
	unsigned int	SOFTWARE__SOFTWARE_49;
	unsigned int	SOFTWARE__SOFTWARE_50;
	unsigned int	SOFTWARE__SOFTWARE_51;
	unsigned int	SOFTWARE__SOFTWARE_52;
	unsigned int	SOFTWARE__SOFTWARE_53;
	unsigned int	SOFTWARE__SOFTWARE_54;
	unsigned int	SOFTWARE__SOFTWARE_55;
	unsigned int	SOFTWARE__SOFTWARE_56;
	unsigned int	SOFTWARE__SOFTWARE_57;
	unsigned int	SOFTWARE__SOFTWARE_58;
	unsigned int	SOFTWARE__SOFTWARE_59;
	unsigned int	SOFTWARE__SOFTWARE_60;
	unsigned int	SOFTWARE__SOFTWARE_61;
	unsigned int	SOFTWARE__SOFTWARE_62;
	unsigned int	SOFTWARE__SOFTWARE_63;
	//Software1
	unsigned int	SOFTWARE1__SOFTWARE1_00;
	unsigned int	SOFTWARE1__SOFTWARE1_01;
	unsigned int	SOFTWARE1__SOFTWARE1_02;
	unsigned int	SOFTWARE1__SOFTWARE1_03;
	unsigned int	SOFTWARE1__SOFTWARE1_04;
	unsigned int	SOFTWARE1__SOFTWARE1_05;
	unsigned int	SOFTWARE1__SOFTWARE1_06;
	unsigned int	SOFTWARE1__SOFTWARE1_07;
	unsigned int	SOFTWARE1__SOFTWARE1_08;
	unsigned int	SOFTWARE1__SOFTWARE1_09;
	unsigned int	SOFTWARE1__SOFTWARE1_10;
	unsigned int	SOFTWARE1__SOFTWARE1_11;
	unsigned int	SOFTWARE1__SOFTWARE1_12;
	unsigned int	SOFTWARE1__SOFTWARE1_13;
	unsigned int	SOFTWARE1__SOFTWARE1_14;
	unsigned int	SOFTWARE1__SOFTWARE1_15;
	unsigned int	SOFTWARE1__SOFTWARE1_16;
	unsigned int	SOFTWARE1__SOFTWARE1_17;
	unsigned int	SOFTWARE1__SOFTWARE1_18;
	unsigned int	SOFTWARE1__SOFTWARE1_19;
	unsigned int	SOFTWARE1__SOFTWARE1_20;
	unsigned int	SOFTWARE1__SOFTWARE1_21;
	unsigned int	SOFTWARE1__SOFTWARE1_22;
	unsigned int	SOFTWARE1__SOFTWARE1_23;
	unsigned int	SOFTWARE1__SOFTWARE1_24;
	unsigned int	SOFTWARE1__SOFTWARE1_25;
	unsigned int	SOFTWARE1__SOFTWARE1_26;
	unsigned int	SOFTWARE1__SOFTWARE1_27;
	unsigned int	SOFTWARE1__SOFTWARE1_28;
	unsigned int	SOFTWARE1__SOFTWARE1_29;
	unsigned int	SOFTWARE1__SOFTWARE1_30;
	unsigned int	SOFTWARE1__SOFTWARE1_31;
	unsigned int	SOFTWARE1__SOFTWARE1_32;
	unsigned int	SOFTWARE1__SOFTWARE1_33;
	unsigned int	SOFTWARE1__SOFTWARE1_34;
	unsigned int	SOFTWARE1__SOFTWARE1_35;
	unsigned int	SOFTWARE1__SOFTWARE1_36;
	unsigned int	SOFTWARE1__SOFTWARE1_37;
	unsigned int	SOFTWARE1__SOFTWARE1_38;
	unsigned int	SOFTWARE1__SOFTWARE1_39;
	unsigned int	SOFTWARE1__SOFTWARE1_40;
	unsigned int	SOFTWARE1__SOFTWARE1_41;
	unsigned int	SOFTWARE1__SOFTWARE1_42;
	unsigned int	SOFTWARE1__SOFTWARE1_43;
	unsigned int	SOFTWARE1__SOFTWARE1_44;
	unsigned int	SOFTWARE1__SOFTWARE1_45;
	unsigned int	SOFTWARE1__SOFTWARE1_46;
	unsigned int	SOFTWARE1__SOFTWARE1_47;
	unsigned int	SOFTWARE1__SOFTWARE1_48;
	unsigned int	SOFTWARE1__SOFTWARE1_49;
	unsigned int	SOFTWARE1__SOFTWARE1_50;
	unsigned int	SOFTWARE1__SOFTWARE1_51;
	unsigned int	SOFTWARE1__SOFTWARE1_52;
	unsigned int	SOFTWARE1__SOFTWARE1_53;
	unsigned int	SOFTWARE1__SOFTWARE1_54;
	unsigned int	SOFTWARE1__SOFTWARE1_55;
	unsigned int	SOFTWARE1__SOFTWARE1_56;
	unsigned int	SOFTWARE1__SOFTWARE1_57;
	unsigned int	SOFTWARE1__SOFTWARE1_58;
	unsigned int	SOFTWARE1__SOFTWARE1_59;
	unsigned int	SOFTWARE1__SOFTWARE1_60;
	unsigned int	SOFTWARE1__SOFTWARE1_61;
	unsigned int	SOFTWARE1__SOFTWARE1_62;
	unsigned int	SOFTWARE1__SOFTWARE1_63;


	unsigned int	MESHARE__KME_WR_client_en;
	unsigned int	MESHARE__ME1_WDMA0_CTRL;
	unsigned int	MESHARE__ME1_WDMA0_NUM_BL;
	unsigned int	MESHARE__ME1_WDMA0_LSTEP;
	unsigned int	MESHARE__ME1_WDMA0_MSTART0;
	unsigned int	MESHARE__ME1_WDMA0_MSTART1;
	unsigned int	MESHARE__ME1_WDMA0_MEND0;
	unsigned int	MESHARE__ME1_WDMA0_MEND1;
	unsigned int	MESHARE__ME1_WDMA1_LSTEP;
	unsigned int	MESHARE__ME1_WDMA1_MSTART0;
	unsigned int	MESHARE__ME1_WDMA1_MSTART1;
	unsigned int	MESHARE__ME1_WDMA1_MSTART2;
	unsigned int	MESHARE__ME1_WDMA1_MEND0;
	unsigned int	MESHARE__ME1_WDMA1_MEND1;
	unsigned int	MESHARE__ME1_WDMA1_MEND2;
	unsigned int	MESHARE__IPME_WDMA_CTRL;
	unsigned int	MESHARE__IPME_WDMA_NUM_BL;
	unsigned int	MESHARE__IPME_WDMA_LSTEP;
	unsigned int	MESHARE__IPME_WDMA_MSTART0;
	unsigned int	MESHARE__IPME_WDMA_MSTART1;
	unsigned int	MESHARE__IPME_WDMA_MSTART2;
	unsigned int	MESHARE__IPME_WDMA_MSTART3;
	unsigned int	MESHARE__IPME_WDMA_MSTART4;
	unsigned int	MESHARE__IPME_WDMA_MSTART5;
	unsigned int	MESHARE__IPME_WDMA_MSTART6;
	unsigned int	MESHARE__IPME_WDMA_MSTART7;
	unsigned int	MESHARE__IPME_WDMA_MEND0;
	unsigned int	MESHARE__IPME_WDMA_MEND1;
	unsigned int	MESHARE__IPME_WDMA_MEND2;
	unsigned int	MESHARE__IPME_WDMA_MEND3;
	unsigned int	MESHARE__IPME_WDMA_MEND4;
	unsigned int	MESHARE__IPME_WDMA_MEND5;
	unsigned int	MESHARE__IPME_WDMA_MEND6;
	unsigned int	MESHARE__IPME_WDMA_MEND7;
	unsigned int	MESHARE__ME_DMA_WR_Rule_check_up;
	unsigned int	MESHARE__ME_DMA_WR_Rule_check_low;
	unsigned int	MESHARE__ME_DMA_WR_Ctrl;
	unsigned int	MESHARE__KME_RD_client_en;
	unsigned int	MESHARE__DH_RDMA0_CTRL;
	unsigned int	MESHARE__DH_RDMA0_NUM_BL;
	unsigned int	MESHARE__ME1_RDMA0_CTRL;
	unsigned int	MESHARE__ME1_RDMA0_NUM_BL;
	unsigned int	MESHARE__ME2_RDMA0_CTRL;
	unsigned int	MESHARE__ME2_RDMA0_NUM_BL;
	unsigned int	MESHARE__ME_DMA_RD_Rule_check_up;
	unsigned int	MESHARE__ME_DMA_RD_Rule_check_low;
	unsigned int	MESHARE__ME_DMA_RD_Ctrl;

	unsigned int	MCDMA__MC_WDMA_CTRL0;
	unsigned int	MCDMA__MC_WDMA_CTRL1;
	unsigned int	MCDMA__MC_LF_WDMA_NUM_BL;
	unsigned int	MCDMA__MC_LF_WDMA_MSTART0;
	unsigned int	MCDMA__MC_LF_WDMA_MSTART1;
	unsigned int	MCDMA__MC_LF_WDMA_MSTART2;
	unsigned int	MCDMA__MC_LF_WDMA_MSTART3;
	unsigned int	MCDMA__MC_LF_WDMA_MSTART4;
	unsigned int	MCDMA__MC_LF_WDMA_MSTART5;
	unsigned int	MCDMA__MC_LF_WDMA_MSTART6;
	unsigned int	MCDMA__MC_LF_WDMA_MSTART7;
	unsigned int	MCDMA__MC_LF_WDMA_MEND0;
	unsigned int	MCDMA__MC_LF_WDMA_MEND1;
	unsigned int	MCDMA__MC_LF_WDMA_MEND2;
	unsigned int	MCDMA__MC_LF_WDMA_MEND3;
	unsigned int	MCDMA__MC_LF_WDMA_MEND4;
	unsigned int	MCDMA__MC_LF_WDMA_MEND5;
	unsigned int	MCDMA__MC_LF_WDMA_MEND6;
	unsigned int	MCDMA__MC_LF_WDMA_MEND7;
	unsigned int	MCDMA__MC_LF_WDMA_LSTEP;
	unsigned int	MCDMA__MC_LF_DMA_WR_Rule_check_up;
	unsigned int	MCDMA__MC_LF_DMA_WR_Rule_check_low;
	unsigned int	MCDMA__MC_LF_DMA_WR_Ctrl;
	unsigned int	MCDMA__MC_HF_WDMA_NUM_BL;
	unsigned int	MCDMA__MC_HF_WDMA_MSTART0;
	unsigned int	MCDMA__MC_HF_WDMA_MSTART1;
	unsigned int	MCDMA__MC_HF_WDMA_MSTART2;
	unsigned int	MCDMA__MC_HF_WDMA_MSTART3;
	unsigned int	MCDMA__MC_HF_WDMA_MSTART4;
	unsigned int	MCDMA__MC_HF_WDMA_MSTART5;
	unsigned int	MCDMA__MC_HF_WDMA_MSTART6;
	unsigned int	MCDMA__MC_HF_WDMA_MSTART7;
	unsigned int	MCDMA__MC_HF_WDMA_MEND0;
	unsigned int	MCDMA__MC_HF_WDMA_MEND1;
	unsigned int	MCDMA__MC_HF_WDMA_MEND2;
	unsigned int	MCDMA__MC_HF_WDMA_MEND3;
	unsigned int	MCDMA__MC_HF_WDMA_MEND4;
	unsigned int	MCDMA__MC_HF_WDMA_MEND5;
	unsigned int	MCDMA__MC_HF_WDMA_MEND6;
	unsigned int	MCDMA__MC_HF_WDMA_MEND7;
	unsigned int	MCDMA__MC_HF_WDMA_LSTEP;
	unsigned int	MCDMA__MC_HF_DMA_WR_Rule_check_up;
	unsigned int	MCDMA__MC_HF_DMA_WR_Rule_check_low;
	unsigned int	MCDMA__MC_HF_DMA_WR_Ctrl;
	unsigned int	MCDMA__MC_WDMA_DB_CTRL;
	unsigned int	MCDMA__MC_RDMA_CTRL;
	unsigned int	MCDMA__MC_RDMA_CTRL1;
	unsigned int	MCDMA__MC_LF_I_RDMA_NUM_BL;
	unsigned int	MCDMA__MC_LF_I_DMA_RD_Rule_check_up;
	unsigned int	MCDMA__MC_LF_I_DMA_RD_Rule_check_low;
	unsigned int	MCDMA__MC_LF_I_DMA_RD_Ctrl;
	unsigned int	MCDMA__MC_HF_I_RDMA_NUM_BL;
	unsigned int	MCDMA__MC_HF_I_DMA_RD_Rule_check_up;
	unsigned int	MCDMA__MC_HF_I_DMA_RD_Rule_check_low;
	unsigned int	MCDMA__MC_HF_I_DMA_RD_Ctrl;
	unsigned int	MCDMA__MC_LF_P_RDMA_NUM_BL;
	unsigned int	MCDMA__MC_LF_P_DMA_RD_Rule_check_up;
	unsigned int	MCDMA__MC_LF_P_DMA_RD_Rule_check_low;
	unsigned int	MCDMA__MC_LF_P_DMA_RD_Ctrl;
	unsigned int	MCDMA__MC_HF_P_RDMA_NUM_BL;
	unsigned int	MCDMA__MC_HF_P_DMA_RD_Rule_check_up;
	unsigned int	MCDMA__MC_HF_P_DMA_RD_Rule_check_low;
	unsigned int	MCDMA__MC_HF_P_DMA_RD_Ctrl;
	unsigned int	MCDMA__MC_RDMA_DB_CTRL;

	//MEMC mux
	unsigned int MEMC_MUX_MEMC_MUX_FUNC_CTRL_SUSPEND;
	//FBG
	unsigned int PPOVERLAY_MEMC_MUX_CTRL_SUSPEND;
	//clock
	unsigned int SYS_REG_SYS_DISPCLKSEL_SUSPEND;
	unsigned int MEMC_LATENCY01;
	unsigned int MEMC_LATENCY02;
	//ME memory boundary
	unsigned int MEMC_DBUS_WRAP_ME_DOWNLIMIT;
	unsigned int MEMC_DBUS_WRAP_ME_UPLIMIT;
	//ME_SHARE boundary
	unsigned int ME_SHARE_WR_low_limit;
	unsigned int ME_SHARE_WR_up_limit;
	unsigned int ME_SHARE_RD_low_limit;
	unsigned int ME_SHARE_RD_up_limit;
	//MV_INFO boundary
	unsigned int MV_INFO_WR_low_limit;
	unsigned int MV_INFO_WR_up_limit;
	unsigned int MV_INFO_RD_low_limit;
	unsigned int MV_INFO_RD_up_limit;
	//MC memory boundary
	unsigned int MC_dma_low_limit;
	unsigned int MC_dma_up_limit;
	unsigned int MEMC_reg[99][64];

}VPQ_MEMC_SUSPEND_RESUME_T;
#endif
