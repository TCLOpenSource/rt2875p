	/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for scaler related functions.
 *
 * @author 	$Author$
 * @date		$Date$
 * @version	$Revision$
 * @ingroup	scaler
 */

/**
 * @addtogroup scaler
 * @{
 */

/*============================ Module dependency  ===========================*/
//Kernel Header file
//#include <unistd.h>			// 'close()'
//#include <fcntl.h>				// 'open()'
//#include <stdio.h>
//#include <string.h>
//#include <sys/ioctl.h>
//#include <time.h>
//#include <sys/mman.h>		// mmap
#ifndef BUILD_QUICK_SHOW
#include <linux/delay.h>
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <RPCDriver.h>
#include <mach/system.h>
#include <linux/pageremap.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
    
#endif /* !BUILD_QUICK_SHOW */
    
#include <mach/platform.h>
    
#include <mach/timex.h>
#ifdef BUILD_QUICK_SHOW
#include "vby1/panel_api.h"
#endif


//RBUS Header file
#include <rbus/ppoverlay_reg.h>
//#include <rbus/rbusScaledownReg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/vgip_reg.h>
#include <rbus/sub_vgip_reg.h>
#include <rbus/dma_vgip_reg.h>
#include <rbus/mdomain_cap_reg.h>
#include <rbus/mdomain_disp_reg.h>
#include <rbus/mdomain_vi_main_reg.h>
#ifdef CONFIG_SUPPORT_SRC_ADC
#endif
#include <rbus/onms_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/mdomain_vi_main_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/dm_reg.h>
#include <rbus/ldspi_reg.h>
#include <rbus/timer_reg.h>
#include <rbus/ptol_reg.h>
#include <rbus/hdr_sub_reg.h>
#include <rbus/stb_reg.h>
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
	#include <rtk_kdriver/scalercommon/scalerCommon.h>
#endif
#include <rbus/pst_i2rnd_reg.h>
#include <rbus/vodma_reg.h>
#include <rbus/hdr_all_top_reg.h>
#include <rbus/dm_reg.h>
#include <rbus/scaledown_reg.h>
#include <rbus/od_reg.h>
#include <rbus/valc_reg.h>
#include <rbus/color_temp_reg.h>
#include <rbus/h3ddma_hsd_reg.h>//data access
#include <rbus/mpegnr_reg.h>//data access
#include <rbus/two_step_uzu_reg.h>//data access
#include <rbus/color_icm_reg.h>//data access
#include <rbus/yuv2rgb_reg.h>//data access
#include <rbus/scaleup_reg.h>//data access
#include <rbus/sub_dither_reg.h>//data access

//TVScaler Header file
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/scalerdrv/scalerclock.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/scalerdrv/fraction.h>
#include <tvscalercontrol/scalerdrv/adjust.h>
#include <tvscalercontrol/scalerdrv/syncproc.h>
#include <tvscalercontrol/scalerdrv/mode.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scalerdrv/pipmp.h>
#include <tvscalercontrol/scalerdrv/framesync.h>
#include <tvscalercontrol/adcsource/ypbpr.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/scalerdrv/framesync.h> // for framesync_get_enterlastline_at_frc_mode_flg()
#include <tvscalercontrol/vip/di_ma.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/scalerdrv/scaler_2dcvt3d_ctrl.h>
#include <tvscalercontrol/scaler/modeState.h>
#include <tvscalercontrol/scaler/state.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <tvscalercontrol/i3ddma/i3ddma.h>
#include <scaler_vpqmemcdev.h>
#include <tvscalercontrol/vip/memc_mux.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/vip/ST2094.h>
#include <scaler_vfedev.h>
#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
#include <tvscalercontrol/scaler/scalervideo.h>
#endif
#include <scaler_vscdev.h>
#include <tvscalercontrol/adcsource/vga.h>

#include <tvscalercontrol/scalerdrv/scaler_i2rnd.h>
#include "memc_isr/scalerMEMC.h"

#ifndef BUILD_QUICK_SHOW
#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>
#include <tvscalercontrol/hdmirx/hdmi_vfe.h>

#ifndef CONFIG_MEMC_NOTSUPPORT
#include "memc_reg_def.h"
#endif


#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <VideoRPC_System.h>
	#include <VideoRPC_System_data.h>
#else
#include <rpc/VideoRPC_System.h>
#include <rpc/VideoRPC_System_data.h>
#endif

#include <rtd_log/rtd_module_log.h>
#endif


#include <rtd_log/rtd_module_log.h>
#include <mach/rtk_platform.h>
#include <tvscalercontrol/adcsource/adcctrl.h>
#include <scaler_vtdev.h>
#if IS_ENABLED(CONFIG_RTK_DPRX)
#include <rtk_kdriver/dprx/dprxfun.h>
#endif

#define UNCAC_BASE		_AC(0xa0000000, UL)
#define MEM_OFFSET  0  //0xa0000000  vcpu access over 512MB ,kernel not send VA

#ifdef BUILD_QUICK_SHOW
#include <sysdefs.h>
#include <no_os/printk.h>
#include <no_os/semaphore.h>
#include <no_os/spinlock.h>
#include <no_os/spinlock_types.h>
#include <no_os/wait.h>
#include <tvscalercontrol/scaler/scalervideo.h>
#include <rtd_log/rtd_module_log.h>
#include <timer.h>
#include <malloc.h>
#include <string.h>
#include <no_os/pageremap.h>
#include <no_os/slab.h>
#endif


// M-capture L/R signal toggle by HW (support on magellan Ver.D)
//#define ENABLE_M_CAPTURE_LR_CTRL_BY_HW
#ifdef ENABLE_M_CAPTURE_LR_CTRL_BY_HW
#define CRT_SC_VERID_VADDR        (STB_SC_VerID_reg)
#define CRT_VERCODE_A		0x62270000
#define CRT_VERCODE_B_C	0x62270001
#endif

#ifndef UINT8
typedef unsigned char  __UINT8;
#define UINT8 __UINT8
#endif
#ifndef UINT16
typedef unsigned short __UINT16;
#define UINT16 __UINT16
#endif
#ifndef UINT32
typedef unsigned int   __UINT32;
#define UINT32 __UINT32
#endif

#define HDMI_TO_VO_FRC_60_MAX_WIDTH_120_PANEL 4096
#define HDMI_TO_VO_FRC_60_MAX_LENTH_120_PANEL 2160
#define HDMI_TO_VO_FRC_60_MAX_WIDTH_60_PANEL 4096
#define HDMI_TO_VO_FRC_60_MAX_LENTH_60_PANEL 2160

#define VGIP_VS_DELAY (3)  //vsync 4~6 -> 3, 2~6 -> 5

#define DVS_HD_LONG_LINE_THRESHOLD (Get_DISPLAY_REFRESH_RATE() * Get_DISP_VERTICAL_TOTAL() / NON_VRR_LONG_DVS_THR)//20Hz
#define DVS_HD_SHORT_LINE_THRESHOLD 0x30B	//768+11
#define DVS_2K_LONG_LINE_THRESHOLD (Get_DISPLAY_REFRESH_RATE() * Get_DISP_VERTICAL_TOTAL() / NON_VRR_LONG_DVS_THR)  //NON_VRR_LONG_DVS_THR is the threshould
#define DVS_2K_SHORT_LINE_THRESHOLD 0x452
#define DVS_4K_LONG_LINE_THRESHOLD (Get_DISPLAY_REFRESH_RATE() * Get_DISP_VERTICAL_TOTAL() / NON_VRR_LONG_DVS_THR)//NON_VRR_LONG_DVS_THR is the threshould
#define DVS_4K_SHORT_LINE_THRESHOLD 0x8a5
#define DVS_4k_OLED_VRR_LONG_LINE_THRESHOLD (Get_DISPLAY_REFRESH_RATE() * Get_DISP_VERTICAL_TOTAL() / OLED_VRR_LONG_DVS_THR)//OLED_VRR_LONG_DVS_THR is the threshould
#define DVS_4k_LED_VRR_LONG_LINE_THRESHOLD (Get_DISPLAY_REFRESH_RATE() * Get_DISP_VERTICAL_TOTAL() / LED_VRR_LONG_DVS_THR)//LED_VRR_LONG_DVS_THR is the threshould
#define DVS_4k_VRR_LONG_LINE_THRESHOLD (Get_DISPLAY_REFRESH_RATE() * Get_DISP_VERTICAL_TOTAL() / VRR_LONG_DVS_THR)//LED_VRR_LONG_DVS_THR is the threshould

#define DVS_WQHD_LONG_LINE_THRESHOLD 0x17D9	//40Hz
#define DVS_WQHD_SHORT_LINE_THRESHOLD 0x5BE	//1460+10
#define DVS_4K_VRR_LONG_LINE_THRESHOLD 0x1FFF


extern unsigned char get_MEMC_bypass_status_refer_platform_model(void);
#define DVTOTAL_60Hz_UPPERBOUND 2137	//2250*0.95=2137=63.17Hz
#define DVTOTAL_60Hz_LOWERBOUND 2362	//2250*1.05=2362=57.15Hz
extern enum PANEL_RES_TYPE get_panel_res(void);

//method 0 is for adjust position and protect H/V porch region by IHD_Delay/IVS_Delay
#define VGIP_CAPTURE_SHIFT_METHOD_0	1

//kernel RPC reteun value
#define KERNEL_RPC_OK	0x10000000

/*===================================  Types ================================*/

// the parameter all data to zero
#define _ALL_ZERO (1 << 26)

// the paramter for increment / non-increment address
#define _AUTO_INC	0
#define _NON_INC	(1 << 31)
#define SRC_CHECK()   (Scaler_InputSrcGetMainChType() == _SRC_CVBS  || Scaler_InputSrcGetMainChType() == _SRC_TV || Scaler_InputSrcGetMainChType() == _SRC_SCART)

/*================================== Variables ==============================*/
#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
//USER:LewisLee DATE:2013/08/29
//this variable need set as VD_27M_OFF in here
//don't set it as you happy!!
//else it will let picture abnormal
VD_27M_OVERSAMPLING_MODE VD_27M_mode = VD_27M_OFF;
#endif

//static unsigned int __fixed_data_start; // last memory address
//static unsigned char AutoMA_Enable = 0;
static GET_INPUT_WID_F gGetInputWid = NULL;

static unsigned char LflagTYPE = 0;
#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE
static unsigned char voDropLine_for4k2k_3d=FALSE;
#endif
unsigned char bCapIdx = 0;

//static pthread_mutex_t		__drvif_scaler_Mutex=PTHREAD_MUTEX_INITIALIZER;


//#define REDUCE_DI_MEMORY_LIMIT_THRESHOLD 920
static unsigned char hfr_mode = FALSE;//support HFR mode or not
static SCALER_RPC_STRUCT *tRPCStruct;
static unsigned int  RPCStruc_phy_addr;
static unsigned char RPCVCPUInitFail;

static HDR_MODE hdmi_hdr_Mode = HDR_MODE_DISABLE;
static HDR_MODE ott_hdr_Mode = HDR_MODE_DISABLE;
static HDR_MODE hdmi_hdr_Mode_during_scaler = HDR_MODE_DISABLE;//Record hdmi hdr mode at scaler beginning
static DOLBY_HDMI_VSIF_T dolby_hdmi_vsif_mode=DOLBY_HDMI_VSIF_DISABLE;
static DOLBY_HDMI_VSIF_T dolby_h14b_vsif_mode=DOLBY_HDMI_VSIF_DISABLE;
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
//not good, need modify
extern DOLBY_HDMI_VSIF_T pre_dolby_hdmi_vsif_mode;
#endif
static unsigned char is_DTV_flag = 0;

/*================================== extern ==============================*/

//extern HDMI_ACTIVE_SPACE_TABLE_T hdmi_active_space_table[];
#ifdef CONFIG_I2RND_ENABLE
extern unsigned char vsc_i2rnd_sub_stage;
extern unsigned char vsc_i2rnd_dualvo_sub_stage;
extern unsigned char check_Sub_I2rnd_done(void);
#endif
extern FILM_SHAREMEM_PARAM *film_mode_param;
unsigned char pre_film_type = 0;
extern unsigned int disp_smooth_variable_setting_flag;
extern struct semaphore* get_scaler_fll_running_semaphore(void);
unsigned int *dynamic_framerate_change = 0;
extern unsigned char vbe_disp_oled_orbit_enable;
extern unsigned char vbe_disp_oled_orbit_mode;

extern HDMI_COLOR_SPACE_T drvif_Hdmi_GetRawColorSpace(void);
extern unsigned char drvif_Hdmi_GetFrlMode(void);
unsigned char rtk_hal_vsc_Getdualdecoder(void);
//extern webos_info_t  webos_tooloption;
extern UINT8 get_nn_force_i3ddma_enable(void);

extern unsigned char vsc_force_rerun_main_scaler;
extern struct semaphore *get_hdmi_detectsemaphore(void);
extern struct semaphore *get_adc_detectsemaphore(void);
extern void Set_Reply_Zero_Timing_Flag(unsigned char src, unsigned char flag);
extern void HDMI_set_detect_flag(unsigned char enable);
extern void ADC_set_detect_flag(unsigned char enable);
extern PANEL_PIXEL_MODE get_panel_pixel_mode(void);
extern unsigned int Get_DISP_VERTICAL_TOTAL(void);
extern unsigned int Get_DISPLAY_CLOCK_TYPICAL(void);
extern unsigned int Get_DISPLAY_REFRESH_RATE(void);
extern unsigned int Get_DISP_HORIZONTAL_TOTAL(void);

extern unsigned char get_vo_camera_flow_flag(unsigned char ch);
#define CAMERA_FLOW_UZU_VTOTAL 1195
#define CAMERA_FLOW_UZU_HTOTAL 2210

unsigned char datafs_gatting_enable = 0;
static unsigned char	Scaler_FreeRun_To_FrameSync_Reset_VODMA_DisableEnable_Cancel_Flag = _DISABLE;
void Scaler_Set_Reset_VODMA_DisableEnable_Cancel_Flag(unsigned int ucEnable)
{
	Scaler_FreeRun_To_FrameSync_Reset_VODMA_DisableEnable_Cancel_Flag = ucEnable;
}

unsigned int Scaler_Get_Reset_VODMA_DisableEnable_Cancel_Flag(void)
{
	return Scaler_FreeRun_To_FrameSync_Reset_VODMA_DisableEnable_Cancel_Flag;
}

#define VGIP_VS_DELAY (3)  //vsync 4~6 -> 3, 2~6 -> 5

#define OLED_VRR_LONG_DVS_THR 38//vrr 40~120hz can no send panel v sync under the hz
#define LED_VRR_LONG_DVS_THR 45//vrr 48~120hz can no send panel v sync under the hz
#define NON_VRR_LONG_DVS_THR 40//can no send panel v sync under the hz
#define VRR_LONG_DVS_THR 24//vrr 24~120hz can no send panel v sync under the hz

const HDMI_ACTIVE_SPACE_TABLE_T hdmi_active_space_table[] = {
    { "1080P FramePacking",  1, 1920,  1080 * 2 + 45, 1080, 45, 0 },
    { "720P FramePacking",  1, 1280,  720 * 2 + 30, 720, 30, 0 },
    { "480P FramePacking",  1, 720,  480 * 2 + 45, 480, 45, 0 },
    { "576P FramePacking",  1, 720,  576 * 2 + 49, 576, 49, 0 },
    { "1080I FramePacking",  0, 1920,  540 * 4 + 22 + 23 * 2, 540, 23, 22 },
    { "576I FramePacking",  0, 720,  288 * 4 + 32 + 21 * 2, 288, 21, 32 },
    { "480I FramePacking",  0, 720,  240 * 4 + 22 + 23 * 2, 240, 23, 22 },
    { NULL,  1, 1920,  520, 40, 0 },
};

extern void framesync_set_iv2dv_delay(unsigned int dispTimingMode);
unsigned char decide_dtg_m_mode_set_cinema_mode_flag;

/*================================== Function  ==============================*/

void rpc_send_source_2p_and_vrr_info(unsigned char two_p_flag_new, unsigned char vrr_flag_new, unsigned char freesync_flag_new)
{//send source 2p and freesync vrr to video fw
	static unsigned char two_p_flag = 0;
	static unsigned char vrr_flag = 0;
	static unsigned char freesync_flag = 0;

	VSC_SOURCE_2P_AND_VRR_INFO *vsc_source_2p_and_vrr_info;
	int ret =0;

	if((two_p_flag_new != two_p_flag) || (vrr_flag_new != vrr_flag) || (freesync_flag_new != freesync_flag))
	{
		vsc_source_2p_and_vrr_info = (VSC_SOURCE_2P_AND_VRR_INFO *)Scaler_GetShareMemVirAddr(SCALERIOC_SOURCE_2P_AND_VRR_INFO);

		vsc_source_2p_and_vrr_info->two_p_flag = two_p_flag_new;
		vsc_source_2p_and_vrr_info->vrr_flag = vrr_flag_new;
		vsc_source_2p_and_vrr_info->freesync_flag = freesync_flag_new;

		if (0 != (ret = Scaler_SendRPC(SCALERIOC_SOURCE_2P_AND_VRR_INFO, 0, 0)))
		{
			rtd_pr_vsc_err("ret=%d, SCALERIOC_SOURCE_2P_AND_VRR_INFO RPC fail !!! two_p_flag:%d, vrr_flag:%d, freesync_flag:%d\n", ret, two_p_flag_new, vrr_flag_new, freesync_flag_new);
			return;
		}
		else
		{
			rtd_pr_vsc_info("SCALERIOC_SOURCE_2P_AND_VRR_INFO RPC two_p_flag:%d, vrr_flag:%d, freesync_flag:%d\n", two_p_flag_new, vrr_flag_new, freesync_flag_new);
			two_p_flag = two_p_flag_new;
			vrr_flag = vrr_flag_new;
			freesync_flag = freesync_flag_new;
		}
	}
}

OLED_RESET_TARGET_CASE Get_OledResetTargetCase(void)
{
    unsigned char display = SLR_MAIN_DISPLAY;
    if(Get_DISPLAY_PANEL_OLED_TYPE() == 1)
    {
        if(Get_DisplayMode_Src(display) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(display) == VSC_INPUTSRC_CAMERA)
        {
            if(get_vsc_run_adaptive_stream(display))
            {
#if 0
                if(Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 300)
                {
                    if(Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_LEN) <= 1440)
                    {
                        return OLED_RESET_TARGET_CASE_CP_RPL;
                    }
                    else
                    {
                        return OLED_RESET_TARGET_CASE_CP;
                    }
                }
                else
                {
                    return OLED_RESET_TARGET_CASE_CP;
                }
#endif
                return OLED_RESET_TARGET_CASE_CP;
            }
            else
            {
                return OLED_RESET_TARGET_CASE_NONCP;
            }
        }
        else if(Get_DisplayMode_Src(display) == VSC_INPUTSRC_HDMI)
        {
            return OLED_RESET_TARGET_CASE_HDMI;
        }
    }
    else
    {
        if(Get_DisplayMode_Src(display) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(display) == VSC_INPUTSRC_CAMERA)
        {

            if(get_vsc_run_adaptive_stream(display))
            {
#if 0
                if(Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 300)
                {
                    if(Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_LEN) <= 1440)
                    {
                        return NON_OLED_RESET_TARGET_CASE_CP_RPL;
                    }
                    else
                    {
                        return NON_OLED_RESET_TARGET_CASE_CP;
                    }
                }
                else
                {
                    return NON_OLED_RESET_TARGET_CASE_CP;
                }
#endif
                return NON_OLED_RESET_TARGET_CASE_CP;
            }
            else
            {
                return OLED_RESET_TARGET_CASE_NONCP;
            }
            
        }
        else if(Get_DisplayMode_Src(display) == VSC_INPUTSRC_HDMI)
        {
            return NON_OLED_RESET_TARGET_CASE_HDMI;
        }
    }
    return OLED_RESET_TARGET_CASE_NONE;
}


unsigned char check_vsc_connect_source(void)
{//return TRUE: vsc conenct any source
	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_MAXN) || (Get_DisplayMode_Src(SLR_SUB_DISPLAY) != VSC_INPUTSRC_MAXN))
		return TRUE;
	return FALSE;
}

void set_HFR_mode(unsigned char enable)
{//enable or disable hfr mode
	hfr_mode = enable;
}

unsigned char get_HFR_mode(void)
{//get current hfr mode
	return hfr_mode;
}


void is_DTV_flag_set(unsigned char enable)
{
	is_DTV_flag = enable;
}

unsigned char is_DTV_flag_get(void)
{
	return is_DTV_flag;
}

HDR_MODE get_ori_HDMI_HDR_mode(void)
{
	return hdmi_hdr_Mode_during_scaler;
}

void set_ori_HDMI_HDR_mode(HDR_MODE mode)
{
	hdmi_hdr_Mode_during_scaler = mode;
}

void set_HDMI_Dolby_H14B_VSIF_mode(DOLBY_HDMI_VSIF_T vsif_mode)
{
	dolby_h14b_vsif_mode = vsif_mode;

#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
	set_sharememory_hdmi_dolby_visf(vsif_mode);
#endif
}

void set_HDMI_Dolby_VSIF_mode(DOLBY_HDMI_VSIF_T vsif_mode)
{
	dolby_hdmi_vsif_mode = vsif_mode;

#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
	set_sharememory_hdmi_dolby_visf(vsif_mode);
#endif
}

DOLBY_HDMI_VSIF_T get_HDMI_Dolby_VSIF_mode(void)
{
	extern bool is_available_dolby_vsif(void);
	extern bool is_available_dolby_h14b_vsif(void);

	DOLBY_HDMI_VSIF_T flag_dolby_vsif = DOLBY_HDMI_VSIF_DISABLE;
	DOLBY_HDMI_VSIF_T flag_dolby_h14b_vsif = DOLBY_HDMI_VSIF_DISABLE;
	DOLBY_HDMI_VSIF_T result;

	if (is_available_dolby_vsif())
		flag_dolby_vsif = dolby_hdmi_vsif_mode;

	if (is_available_dolby_h14b_vsif())
		flag_dolby_h14b_vsif = dolby_h14b_vsif_mode;

	result = (flag_dolby_vsif != DOLBY_HDMI_VSIF_DISABLE) ? flag_dolby_vsif : flag_dolby_h14b_vsif;

#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
	{
		extern bool dolby_adapter_is_force_dolby(void);

		if (dolby_adapter_is_force_dolby())
			result = DOLBY_HDMI_VSIF_STD;
	}
#endif

#if 0
	{
		static DOLBY_HDMI_VSIF_T pre_flag_dolby_vsif = DOLBY_HDMI_VSIF_DISABLE;
		static DOLBY_HDMI_VSIF_T pre_flag_dolby_h14b_vsif = DOLBY_HDMI_VSIF_DISABLE;

		if (pre_flag_dolby_vsif != flag_dolby_vsif || pre_flag_dolby_h14b_vsif != flag_dolby_h14b_vsif) {
			rtd_pr_vbe_info("[Dolby][%s:%d] (%d -> %d) (%d -> %d)\n", __func__, __LINE__
					, pre_flag_dolby_vsif
					, flag_dolby_vsif
				        , pre_flag_dolby_h14b_vsif
					, flag_dolby_h14b_vsif
				);
		}
		pre_flag_dolby_vsif = flag_dolby_vsif;
		pre_flag_dolby_h14b_vsif = flag_dolby_h14b_vsif;
	}
#endif

	return result;
}

unsigned char is_dolby_vision_tunnel_mode(void)
{
	bool ret = false;
#if 0	
	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) {
		if ((get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_h14B_VSIF)
				|| (get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_VSIF_STD))
			ret = true;
	} else if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP) {
		/* FIXME : need add this condition after DP driver provide vsif mode api */
	}
#endif
	return ret;
}

void set_PreHDMI_Dolby_VSIF_mode(DOLBY_HDMI_VSIF_T Vsif_mode)
{
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
    pre_dolby_hdmi_vsif_mode = Vsif_mode;
#endif //#ifdef CONFIG_SUPPORT_DOLBY_VSIF
}

HDR_MODE get_HDMI_HDR_mode(void)
{
	return hdmi_hdr_Mode;
}

void set_HDMI_HDR_mode(HDR_MODE mode)
{
	hdmi_hdr_Mode = mode;
}


HDR_MODE get_OTT_HDR_mode(void)
{
	return ott_hdr_Mode;
}

void set_OTT_HDR_mode(HDR_MODE mode)
{
	ott_hdr_Mode = mode;
}

unsigned char get_scaling_down_status(void)
{//for two pixel mode using
	return (Scaler_DispGetScaleStatus(SLR_MAIN_DISPLAY,SLR_SCALE_V_DOWN) || Scaler_DispGetScaleStatus(SLR_MAIN_DISPLAY,SLR_SCALE_H_DOWN))? TRUE : FALSE;
}

unsigned char get_MEMC_bypass_status_refer_platform_model(void)
{/*TRUE: memc bypass; FALSE: memc on */
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
	return TRUE;
#else

	//modify condition by DIA2DEV-35
	unsigned char flg = FALSE;
#ifdef CONFIG_MEMC_BYPASS
		flg = TRUE;
#else
	unsigned long memc_addr = 0, memc_addr_aligned = 0;

	if(Scaler_MEMC_RbusAccess_Check() == 0) {
		flg = TRUE; //OTP burn memc bypass
	}
	else if(Scaler_MEMC_Get_Size_Addr(&memc_addr_aligned, &memc_addr) == 0){
		flg = TRUE;
	}
	else if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI)||(Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI)){
		flg = TRUE; //convertor board default memc bypass
	}
	else if(get_product_type() == PRODUCT_TYPE_DIAS){
		flg = FALSE;
	}
	// If MEMC = OFF, (add API to get memc info from model.ini)  return true
#endif //CONFIG_MEMC_BYPASS

	return flg;
#endif
}

unsigned char get_MEMC_UI_status_refer_platform_model (void)
{/*TRUE: UI show memc; FALSE: UI don't show memc */
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
	return FALSE;
#else

	//add condition by DIA2DEV-35
	unsigned char flg = TRUE;
#ifdef CONFIG_MEMC_BYPASS
		flg = FALSE;
#else
	unsigned long memc_addr = 0, memc_addr_aligned = 0;

	if(Scaler_MEMC_RbusAccess_Check() == 0) {
		flg = FALSE; //OTP burn memc bypass
	}
	else if(Scaler_MEMC_Get_Size_Addr(&memc_addr_aligned, &memc_addr) == 0){
		flg = FALSE;
	}
	else if(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI){
		flg = FALSE; //convertor board default memc bypass
	}
	else if(get_product_type() == PRODUCT_TYPE_DIAS){
		flg = FALSE;
	}
	// If MEMC = OFF, (add API to get memc info from model.ini)	 return false
#endif
	return flg;
#endif
}

unsigned char decide_2step_uzu_merge_sub(void)
{//return TRUE: merge sub . return FALSE: no merge
	unsigned char hdmi_4k_hfr_mode = get_hdmi_4k_hfr_mode();
	unsigned char vuzd_flag=0;

    if (((get_rotate_mode(SLR_MAIN_DISPLAY) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_270)
        || (get_rotate_mode(SLR_MAIN_DISPLAY) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_90))
        && (is_4k_hfr_mode(SLR_MAIN_DISPLAY)))
    {
        return FALSE;
    }

	if(get_panel_pixel_mode() <= PANEL_1_PIXEL)
		return FALSE;//converted board does not go main sub merge
	if (Scaler_DispGetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_V_DOWN) == 0) {
		if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_SDNRV_ACT_LEN) > Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DISP_LEN)) {
			vuzd_flag=1;
		} else {
			vuzd_flag=0;
		}
	} else {
		vuzd_flag=1;
	}

	if(is_4k_hfr_mode(SLR_MAIN_DISPLAY))
		return vuzd_flag ? FALSE : TRUE;
	if(hdmi_4k_hfr_mode == HDMI_4K120_2_0)
		return vuzd_flag ? FALSE : TRUE;
    else if (get_hdmi_vrr_4k60_mode()) {
        return TRUE;
    }
    return FALSE;
}
#ifndef BUILD_QUICK_SHOW

unsigned char get_HD_FHD_platform_model(void)
{/*TRUE: hd or fhd model; FALSE: uhd model */
	unsigned char flg = FALSE;
	if(get_panel_res() == PANEL_RES_FHD || get_panel_res() == PANEL_RES_HD)
	{
		flg = TRUE;
	}else{
		flg = FALSE;
	}
	return flg;
}
#endif
unsigned char scaler_hmdi_hfr_mode = HDMI_NON_4K120;

void decide_scaler_hdmi_hfr_mode(void)
{//save scaler need to run 4k120 or not. decide at scaler start
	unsigned char hdmi_mode = HDMI_NON_4K120;
    unsigned int pixel_clock = 0;
#ifdef CONFIG_RTK_DPRX
    DPRX_bool ret = 0;
    DPRX_TIMING_INFO_T timing_info = {};
#endif
	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) {
		pixel_clock = (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHLEN)/10) * (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVLEN)/10) * (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ)/10);

		if (pixel_clock > 6000000)
		{
				hdmi_mode = HDMI_4K120_2_1;//pixel clock > 600M
		}
		/*		
		else if(Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHWID) >= 3840 && Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVHEIGHT) >= 2160 && Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ) >= 990)
		{
			hdmi_mode = HDMI_4K120_2_1;//HDMI_4K120_2_0; Let hdmi 2.0 also use hdmi2.1 for merlin6
		}
		*/
		else if(Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHWID) > 1920 && Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVHEIGHT) > 1080 && Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ) >= 990)
		{
			//hdmi_mode = HDMI_3K120;//HDMI_43k120  does not return 3k120 fix original path
		}

	}
    else if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP)
    {
#ifdef CONFIG_RTK_DPRX
        ret = drvif_Dprx_GetRawTimingInfo(&timing_info);
        if(ret == 1)
        {
            if(timing_info.pixel_mode == DPRX_OUT_PIXEL_MODE_1P)
            {
                hdmi_mode = HDMI_NON_4K120;
            }
            else if(timing_info.pixel_mode == DPRX_OUT_PIXEL_MODE_2P)
            {
                hdmi_mode = HDMI_4K120_2_1;
            }
            else if(timing_info.pixel_mode == DPRX_OUT_PIXEL_MODE_4P)
            {
                hdmi_mode = HDMI_4K120_2_1;
                rtd_pr_vbe_err("[%s %d][ERROR]scaler not support 4pixel mode\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            rtd_pr_vbe_err("[%s %d][ERROR]get dp timing error\n", __FUNCTION__, __LINE__);
        }
#endif
    }

	scaler_hmdi_hfr_mode = hdmi_mode;
}

unsigned char get_hdmi_4k_hfr_mode(void)
{
    if(get_vsc_src_is_hdmi_or_dp())
    {
        return scaler_hmdi_hfr_mode;
    }

	return HDMI_NON_4K120;// will decide at scaler start
}

unsigned char get_hdmi_vrr_4k60_mode(void){
	if((get_panel_pixel_mode() > PANEL_1_PIXEL)
		&& (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY)
		&& ((get_hdmi_4k_hfr_mode() == HDMI_NON_4K120) || (get_hdmi_4k_hfr_mode() == HDMI_3K120))
		&& (get_vsc_src_is_hdmi_or_dp())
		&& (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag()))
		return TRUE;
	else
		return FALSE;
}
unsigned char get_vdec_4k_hdr_mode(unsigned char display)
{
    VSC_INPUT_TYPE_T src_type = Get_DisplayMode_Src(display);
    unsigned char ret = false;
    if (src_type == VSC_INPUTSRC_VDEC || src_type == VSC_INPUTSRC_CAMERA) {
        SLR_VOINFO *pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
		uint64_t h_total = pVOInfo->h_total;
		uint64_t v_total = pVOInfo->v_total;
		uint64_t v_freq  = pVOInfo->v_freq;

		ret = (h_total * v_total * v_freq / 10) > 600000000;
	}
    return ret;
}

unsigned char is_4k_hfr_mode(unsigned char display)
{
	VSC_INPUT_TYPE_T src_type = Get_DisplayMode_Src(display);
	unsigned char ret = false;

	if (display != SLR_MAIN_DISPLAY)
		goto is_4k_hfr_mode_not_main_display;

	if (get_vsc_src_is_hdmi_or_dp()) {
		ret = (get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1);
	} else if (src_type == VSC_INPUTSRC_VDEC || src_type == VSC_INPUTSRC_CAMERA) {
        ret = get_vdec_4k_hdr_mode(display);
    }

is_4k_hfr_mode_not_main_display:
	return ret;
}

static unsigned char i3ddma_2p_mode_en = FALSE;
void set_i3ddma_2p_mode_flag(unsigned char bOnoff)
{
	i3ddma_2p_mode_en = bOnoff;
	if(bOnoff)
		rtd_printk(KERN_NOTICE, TAG_NAME_VSC, "4k qms-vrr or 4k120 pass i3ddma enable 2p mode=%d!!! \n", i3ddma_2p_mode_en);
}

unsigned char get_i3ddma_2p_mode_flag(void)
{
	return i3ddma_2p_mode_en;
}

void decide_i3ddma_2p_mode_on_off(void)
{
	unsigned char val = FALSE;
	//4k120 pass i3ddma or 4k qms
	if(((get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1) && (get_force_i3ddma_enable(SLR_MAIN_DISPLAY))) || 
		(get_scaler_qms_mode_flag() && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) >= 3840) &&
		(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) >= 2160)))
		val = TRUE;
	else
		val = FALSE;

	set_i3ddma_2p_mode_flag(val);
}


void drvif_memory_set_jpeg_disp_mem(StructJPEGDisplayInfo *jpeg_info);
#ifndef BUILD_QUICK_SHOW
void Set_3d_type(unsigned char type)
{
	LflagTYPE = type;
}

unsigned char Get_3d_type(void)
{
	return LflagTYPE;
}
#endif

void drvif_Dolby_HDR_disable_DM_LUT(void){
	// disable B02 & B05
	dm_dm_b02_lut_ctrl0_RBUS dm_b02_lut_ctrl0_reg;
	dm_b02_lut_ctrl0_reg.regValue = rtd_inl(DM_DM_B02_LUT_CTRL0_reg);
	//fixme：k4l bringup
	//dm_b02_lut_ctrl0_reg.lut_enable = 0;
	rtd_outl(DM_DM_B02_LUT_CTRL0_reg, dm_b02_lut_ctrl0_reg.regValue);
}

void drvif_scaler_vactive_sta_irq(unsigned char enable, unsigned char channel)
{
	//if (Scaler_IsUnitTest())
		//return;

	/*
	unsigned int VGIP_reg_value = 0;
	main_2_vgip_vgip_chn1_ctrl_RBUS main_2_vgip_chn1_ctrl_reg;
	main_2_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_MAIN_2_VGIP_CHN1_CTRL_VADDR);

	if(main_2_vgip_chn1_ctrl_reg.ch1_in_sel) //SENSIO path
		VGIP_reg_value = VGIP_MAIN_2_VGIP_CHN1_CTRL_VADDR;
	else

		VGIP_reg_value = VGIP_VGIP_CHN1_CTRL_reg;


	if(channel == SLR_MAIN_DISPLAY){
		if(VGIP_reg_value == VGIP_MAIN_2_VGIP_CHN1_CTRL_VADDR){
			IoReg_Mask32(VGIP_VGIP_CHN1_CTRL_reg, ~_BIT24, 0);
		}else{
			IoReg_Mask32(VGIP_MAIN_2_VGIP_CHN1_CTRL_VADDR, ~_BIT24, 0);
		}
	}
	*/

	// Don't enable vgip V-Start isr
#if 0
	if (TRUE == enable && TRUE == Scaler_GetDisableVIP())
	{
		rtd_pr_vbe_info("Disable VIP, don't enable VGIP V-start ISR\n");
		return;
	}
#endif


//#ifdef SCALER_INTERRUP_READY
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_Mask32(channel ? SUB_VGIP_VGIP_CHN2_CTRL_reg : VGIP_VGIP_CHN1_CTRL_reg, ~_BIT24, enable?_BIT24:0);	//enable VSync start interrupt
#else
	IoReg_Mask32(/*channel ? SUB_VGIP_VGIP_CHN2_CTRL_reg : */VGIP_VGIP_CHN1_CTRL_reg, ~_BIT24, enable?_BIT24:0);	//enable VSync start interrupt
#endif
}

void drvif_scaler_dma_vgip_sta_irq_ctrl(unsigned char sta_enable)
{//control dma start vgip isr
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_vact_start_ie = sta_enable ? 1: 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);
}

void drvif_scaler_dma_vgip_end_irq_ctrl(unsigned char end_enable)
{//control dma end vgip isr
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_ctrl_reg.dma_vact_end_ie = end_enable ? 1: 0;
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);
}


void drvif_scaler_vactive_end_irq(unsigned char enable, unsigned char channel)
{
	//if (Scaler_IsUnitTest())
		//return;
	/*
	unsigned int VGIP_reg_value = 0;
	main_2_vgip_vgip_chn1_ctrl_RBUS main_2_vgip_chn1_ctrl_reg;
	main_2_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_MAIN_2_VGIP_CHN1_CTRL_VADDR);
	if(main_2_vgip_chn1_ctrl_reg.ch1_in_sel) //SENSIO path
		VGIP_reg_value = VGIP_MAIN_2_VGIP_CHN1_CTRL_VADDR;
	else
		VGIP_reg_value = VGIP_VGIP_CHN1_CTRL_reg;

	if(channel == SLR_MAIN_DISPLAY){
		if(VGIP_reg_value == VGIP_MAIN_2_VGIP_CHN1_CTRL_VADDR){
			IoReg_Mask32(VGIP_VGIP_CHN1_CTRL_reg, ~_BIT25, 0);
		}else{
			IoReg_Mask32(VGIP_MAIN_2_VGIP_CHN1_CTRL_VADDR, ~_BIT25, 0);
		}
	}
	*/

	// Don't enable vgip V-End isr
#if 0
	if (TRUE == enable && TRUE == Scaler_GetDisableVIP())
	{
		rtd_pr_vbe_info("Disable VIP, don't enable VGIP V-end ISR\n");
		return;
	}
#endif

//#ifdef SCALER_INTERRUP_READY
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_Mask32(channel ? SUB_VGIP_VGIP_CHN2_CTRL_reg : VGIP_VGIP_CHN1_CTRL_reg, ~_BIT25, enable?_BIT25:0);	//enable VSync end interrupt
#else
	IoReg_Mask32(/*channel ? SUB_VGIP_VGIP_CHN2_CTRL_reg : */VGIP_VGIP_CHN1_CTRL_reg, ~_BIT25, enable?_BIT25:0);	//enable VSync end interrupt
#endif
}

//new function SDNR interrupt @Crixus 20170518
void drvif_scaler_sdnr_vactive_end_irq(unsigned char enable, unsigned char channel)
{
	vgip_vgip_chn1_wde_RBUS vgip_vgip_chn1_wde_reg;
	sub_vgip_vgip_chn2_wde_RBUS sub_vgip_vgip_chn2_wde_reg;
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	if(channel == SLR_SUB_DISPLAY){
		sub_vgip_vgip_chn2_wde_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_WDE_reg);
		sub_vgip_vgip_chn2_wde_reg.ch2_dispi_vact_end_ie = enable;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_WDE_reg, sub_vgip_vgip_chn2_wde_reg.regValue);
	}
	else{
		vgip_vgip_chn1_wde_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_WDE_reg);
		vgip_vgip_chn1_wde_reg.ch1_dispi_vact_end_ie = enable;
		IoReg_Write32(VGIP_VGIP_CHN1_WDE_reg, vgip_vgip_chn1_wde_reg.regValue);
	}
#else
	vgip_vgip_chn1_wde_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_WDE_reg);
	vgip_vgip_chn1_wde_reg.ch1_dispi_vact_end_ie = enable;
	IoReg_Write32(VGIP_VGIP_CHN1_WDE_reg, vgip_vgip_chn1_wde_reg.regValue);
#endif
}

/*============================================================================*/


/**
 * drvif_scaler_ddomain_switch_irq
 * switch (enable/disable) D-Domain IRQ
 *
 * @param <enable>	{ enable or disable D-Domain IRQ }
 * @return 			{ void }
 *
 */
void drvif_scaler_ddomain_switch_irq(unsigned char enable)	// add by hsliao 20081015
{
	ppoverlay_dtg_ie_RBUS	dtg_ie_reg;
	dtg_ie_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_ie_reg);
	dtg_ie_reg.mv_den_sta_event_ie = enable;
	IoReg_Write32(PPOVERLAY_DTG_ie_reg, dtg_ie_reg.regValue);
}
/*api which have protect by forcebg_semaphore*/
void drvif_scaler_ddomain_DTG_line_compare_irq(unsigned char enable, unsigned char vlc_mode, unsigned int vlc_src_sel, unsigned int dtg_vln)
{
	ppoverlay_dtg_lc_RBUS dtg_lc_reg;
	ppoverlay_dtg_ie_RBUS dtg_ie_reg;
	dtg_lc_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_LC_reg);
	dtg_ie_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_ie_reg);

	dtg_lc_reg.dtg_vlc_mode= vlc_mode;	// mode
	dtg_lc_reg.dtg_vln = dtg_vln;	// v length
	dtg_ie_reg.dtg_vlc_ie = enable;	// function en
	dtg_lc_reg.dtg_vlcen= enable;	// Interrupt en
	dtg_lc_reg.dtg_vlc_src_sel = vlc_src_sel;

	IoReg_Write32(PPOVERLAY_DTG_LC_reg, dtg_lc_reg.regValue);
	IoReg_Write32(PPOVERLAY_DTG_ie_reg, dtg_ie_reg.regValue);
	rtd_pr_vbe_notice("[%s]%d,%d,%d,%d\n", __FUNCTION__, enable, vlc_mode, vlc_src_sel, dtg_vln);
}


//lewis, for delete 4 lines (Up 2 line and down 2 line)
//to prevent image have garbage, in VGA or DVI, 100% overscan
//don't use it
//return true -> cut 4 line
unsigned char  fwif_scaler_decide_display_cut4line(unsigned char disp)
{
	return FALSE;/*k2h no need*/
#if 0
#ifdef RUN_ON_TVBOX
	return FALSE;
#else
#if 1
	if(fwif_vip_source_check(3,0)==VIP_QUALITY_HDMI_1080I)
	{
		return FALSE;
	}
#endif
	if(FALSE == Scaler_DispGetStatus((SCALER_DISP_CHANNEL)disp, SLR_DISP_THRIP))
		return FALSE;

#ifdef CONFIG_ENABLE_NATIVE_DISPLAY_RATIO_MODE
	if(_TRUE == Scaler_Get_AspectRatio_Enter_Native_Mode())
		return FALSE;
#endif //#ifdef CONFIG_ENABLE_NATIVE_DISPLAY_RATIO_MODE

//	if(IS_RATIO_NO_OVERSCAN())
//		return FALSE;

	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)disp, SLR_DISP_3D_3DTYPE) <= SLR_3D_SENSIO_CVT_2D))
		return FALSE;

	if(_SRC_VGA == Scaler_InputSrcGetMainChType())
		return FALSE;

	/*if(_SRC_FROM_TMDS == Scaler_InputSrcGetMainChFrom())

	{
		if( (drvif_Hdmi_GetColorSpace() == COLOR_RGB)||(drvif_Hdmi_GetColorSpace() == COLOR_YUV444))
			return FALSE;
	}*/
	if(_SRC_HDMI == Scaler_InputSrcGetMainChType())
	{
		if ( !drvif_IsHDMI() ) // DVI
	   		return FALSE;
		else if((drvif_HDMI_GetScreenMode()==HDMI_OSD_MODE_DVI)||((drvif_HDMI_GetScreenMode()==HDMI_OSD_MODE_AUTO)&&(!drvif_HDMI_IsVideoTiming())))
			return FALSE;
		// HDMI source but user set as DVI (by customer)
		//if ( drvif_HDMI_IsScreenModeDVI() )
		//	return FALSE;
	}

	if(Scaler_InputSrcGetMainChType() == _SRC_CVBS || Scaler_InputSrcGetMainChType() == _SRC_YPBPR)
		return FALSE;

	return TRUE;
#endif
#endif
}


//#define ENABLE_IDMA_VGIP_SYNC_TO_MAIN_VGIP
unsigned char drvif_scaler_decide_idmaVgip_syncTo_mainVgip(void)
{
#if defined(ENABLE_DRIVER_I3DDMA) && defined(ENABLE_IDMA_VGIP_SYNC_TO_MAIN_VGIP)
	// don't update DMA VGIP setting when enter IDMA control flow (VGIP setting in modestate_I3DDMA_config())
	if(Scaler_DispGetInputInfo(SLR_INPUT_THROUGH_I3DDMA) == FALSE)
		return TRUE;
#endif
	return FALSE;
}


/*============================================================================*/
/**
 * drvif_scaler_poweron_init
 * Initial settings for scaler from power saving mode or power-off mode.
 *
 * @param <void>		{ void }
 * @return 			{ void }
 *
 */
//void __init drvif_scaler_poweron_init(void)
void drvif_scaler_poweron_init(void)
{
	// Enable TV ctrl I/D/M domain

//	IoReg_SetBits(SYSTEM_GROUP1_CK_EN_VADDR,  _BIT4 | _BIT3);
}

/*============================================================================*/
/**
 * drvif_scaler_disable_display_output
 * Reset the scaler.
 *
 * @param <void>		{ void }
 * @return 			{ void }
 *
 */
void drvif_scaler_disable_display_output(void)
{
	IoReg_ClearBits(PPOVERLAY_Display_Timing_CTRL1_reg, _BIT24);
}

void drv_scaler_adjust_ivs_delay(unsigned char display, unsigned short usIVSDelay)
{
	vgip_vgip_chn1_delay_RBUS vgip_chn1_delay_reg;
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
//	sub_vgip_vgip_chn2_delay_RBUS vgip_chn2_delay_reg;
#endif


#if 0
 	IoReg_Mask32(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CHANNEL) ? SUB_VGIP_VGIP_CHN2_DELAY_reg : VGIP_VGIP_CHN1_DELAY_reg,  ~0x00000fff, ( usIVSDelay /*+_PROGRAM_VDELAY */) & 0x00000fff);
#else //#if 0
	if(SLR_MAIN_DISPLAY == display)
	{
		vgip_chn1_delay_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_DELAY_reg);
		vgip_chn1_delay_reg.ch1_ivs_dly = usIVSDelay;
		IoReg_Write32(VGIP_VGIP_CHN1_DELAY_reg, vgip_chn1_delay_reg.regValue);
		if(drvif_scaler_decide_idmaVgip_syncTo_mainVgip())
			IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, vgip_chn1_delay_reg.regValue);
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else// if(SLR_SUB_DISPLAY == display)
	{
		//crixus@20140722 - Ma3 no sub path
		//vgip_chn2_delay_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_DELAY_reg);
		//vgip_chn2_delay_reg.ch2_ivs_dly = usIVSDelay /*+_PROGRAM_VDELAY */;
		//IoReg_Write32(SUB_VGIP_VGIP_CHN2_DELAY_reg, vgip_chn2_delay_reg.regValue);
	}
#endif
#endif //#if 0

	rtd_pr_vbe_debug("IVSDelay=%x\n", usIVSDelay);
}

void drv_scaler_adjust_ihs_delay(unsigned char display, unsigned short usIHSDelay)
{
	vgip_vgip_chn1_delay_RBUS vgip_chn1_delay_reg;
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	sub_vgip_vgip_chn2_delay_RBUS vgip_chn2_delay_reg;
#endif

	if(SLR_MAIN_DISPLAY == display)
	{
		vgip_chn1_delay_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_DELAY_reg);
		vgip_chn1_delay_reg.ch1_ihs_dly = _PROGRAM_HDELAY + usIHSDelay;
		IoReg_Write32(VGIP_VGIP_CHN1_DELAY_reg, vgip_chn1_delay_reg.regValue);
		if(drvif_scaler_decide_idmaVgip_syncTo_mainVgip())
			IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, vgip_chn1_delay_reg.regValue);
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else// if(SLR_SUB_DISPLAY == display)
	{
		vgip_chn2_delay_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_DELAY_reg);
		vgip_chn2_delay_reg.ch2_ihs_dly = _PROGRAM_HDELAY + usIHSDelay;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_DELAY_reg, vgip_chn2_delay_reg.regValue);
	}
#endif

	rtd_pr_vbe_debug("IHSDelay in drv_scaler_adjust_ihs_delay=%x\n", usIHSDelay);
}

//#define SRC_CHECK()   (Scaler_InputSrcGetMainChType() == _SRC_VGA || Scaler_InputSrcGetMainChType() == _SRC_CVBS  || Scaler_InputSrcGetMainChType() == _SRC_TV || Scaler_InputSrcGetMainChType() == _SRC_SCART)
#define SRC_CHECK()   (Scaler_InputSrcGetMainChType() == _SRC_CVBS  || Scaler_InputSrcGetMainChType() == _SRC_TV || Scaler_InputSrcGetMainChType() == _SRC_SCART)

/*============================================================================*/
/**
 * fw_scaler_set_vgip_capture
 * Set capture window of input source.
 *
 * @param <void>		{ void }
 * @return 			{ void }
 *
 */


void fw_scaler_set_vgip_capture(unsigned char display)
{
	unsigned short usV_Porch;
	unsigned char temp=0;

	unsigned char ucIVS2DVSDelay = 0;
	unsigned short ucIHSDelay= 0;
	unsigned int new_v_start = 0;
	vgip_vgip_chn1_act_hsta_width_RBUS vgip_chn1_act_hsta_width_reg;
	vgip_vgip_chn1_act_vsta_length_RBUS vgip_chn1_act_vsta_length_reg;
	vgip_vgip_chn1_ctrl_RBUS vgip_chn1_ctrl_reg;

	vgip_vgip_chn1_delay_RBUS vgip_chn1_delay_reg;
//#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	sub_vgip_vgip_chn2_act_hsta_width_RBUS vgip_chn2_act_hsta_width_reg;
	sub_vgip_vgip_chn2_act_vsta_length_RBUS vgip_chn2_act_vsta_length_reg;
	sub_vgip_vgip_chn2_delay_RBUS sub_vgip_vgip_chn2_delay_reg;
	sub_vgip_vgip_chn2_ctrl_RBUS vgip_chn2_ctrl_reg;
//#endif

	//main_2_vgip_vgip_chn1_3d_right_act_hsta_width_RBUS main_2_vgip_chn1_3d_right_act_hsta_width_reg;
	//main_2_vgip_vgip_chn1_3d_right_act_vsta_length_RBUS main_2_vgip_chn1_3d_right_act_vsta_length_reg;
//	vgip_chn1_3d_right_act_hsta_width_RBUS vgip_chn1_3d_right_act_hsta_width_reg;
//	vgip_chn1_3d_right_act_vsta_length_RBUS vgip_chn1_3d_right_act_vsta_length_reg;

	unsigned short IPH_ACT_STA = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_STA);
	unsigned short IPV_ACT_STA = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPV_ACT_STA);
	unsigned short IPV_ACT_LEN = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_IPV_ACT_LEN);
	unsigned short V_LEN = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_V_LEN);

	//rtd_pr_vbe_emerg("fw_scaler_set_vgip_capture display=%d\n", display);
	rtd_pr_vbe_debug("fw_scaler_set_vgip_capture display=%d\n", display);

	if((IPV_ACT_STA + IPV_ACT_LEN) > V_LEN) {
		// fix for temporary image shift at booting while color system is set to PALN (Columbia)
		// ref : zoom_check_size_error()
			rtd_pr_vbe_emerg("IPV_ACT_STA(%d) + IPV_ACT_LEN(%d) > V_LEN(%d) \n", IPV_ACT_STA, IPV_ACT_LEN, V_LEN);
		IPV_ACT_STA = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_IPV_ACT_STA_PRE);
	}

	//rtd_pr_vbe_emerg("IPH_ACT_STA=%d;;;IPV_ACT_STA = %d\n", IPH_ACT_STA, IPV_ACT_STA);
	//rtd_pr_vbe_emerg("Scaler_DispGetInputInfo(SLR_INPUT_V_LEN)=%d in capture window\n", Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_V_LEN));
	rtd_pr_vbe_debug("IPH_ACT_STA=%d;;;IPV_ACT_STA = %d\n", IPH_ACT_STA, IPV_ACT_STA);
	rtd_pr_vbe_debug("Scaler_DispGetInputInfo(SLR_INPUT_V_LEN)=%d in capture window\n", Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_V_LEN));
	Scaler_DispSetIvs2DvsDelay(0);
	Scaler_DispSetIhsDelay(0);
	//rtd_pr_vbe_emerg("Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)=%x in capture window\n", Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISPLAY));
	rtd_pr_vbe_debug("Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)=%x in capture window\n", Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISPLAY));

#if !TEST_PATTERN_GEN
       if(SRC_CHECK()) { 	// weihao 960308
	{
		    ucIVS2DVSDelay = _PROGRAM_VDELAY; //it is short time value
			ucIHSDelay= _H_POSITION_MOVE_TOLERANCE;
#ifndef BUILD_QUICK_SHOW
#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
		  // if(VD_27M_OFF != fw_video_get_27mhz_mode((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)))
		   if(VD_27M_OFF != fw_video_get_27mhz_mode(display))
		   {
				ucIHSDelay *= 2;
		   }
#endif
#endif
		    //IPH_ACT_STA -= _H_POSITION_MOVE_TOLERANCE;
		    IPH_ACT_STA -= ucIHSDelay;

		    //rtd_pr_vbe_emerg("orgianl IPH_ACT_STA=%d, ucIHSDelay=%d\n", IPH_ACT_STA, ucIHSDelay);
		    rtd_pr_vbe_debug("orgianl IPH_ACT_STA=%d, ucIHSDelay=%d\n", IPH_ACT_STA, ucIHSDelay);

		    Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_STA , IPH_ACT_STA);
		    	//rtd_pr_vbe_emerg("\n SetCapture IHStart = %x in Scaler_SetCaptureWindow\n", IPH_ACT_STA);
	    		rtd_pr_vbe_debug("\n SetCapture IHStart = %x in Scaler_SetCaptureWindow\n", IPH_ACT_STA);

			//CSW 0970121 Add protection o avoid front porch exceed 255
			if (Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_V_LEN) - Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPV_ACT_LEN) - 4 > 255)
				usV_Porch = 255;
			else
				usV_Porch = (unsigned char)(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_V_LEN) - Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPV_ACT_LEN)  - 4);//Get the porch

           		//rtd_pr_vbe_emerg(" usV_Porch = %x\n",usV_Porch);
           		rtd_pr_vbe_debug(" usV_Porch = %x\n",usV_Porch);

			Scaler_DispSetIpMargin(SLR_IPMARGIN_H_MAX , 0x80 + ucIHSDelay);
			Scaler_DispSetIpMargin(SLR_IPMARGIN_H_MIN , 0x80 - ucIHSDelay);
			Scaler_DispSetIpMargin(SLR_IPMARGIN_V_MAX , 0x80 + ((usV_Porch > _V_POSITION_MOVE_TOLERANCE) ? _V_POSITION_MOVE_TOLERANCE : usV_Porch));

			if (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))
			{//coverity issue :ucIVS2DVSDelay is equal to 10, The condition "ucIVS2DVSDelay > 50" cannot be true.
			    temp = 0x80 -ucIVS2DVSDelay;//((ucIVS2DVSDelay > _V_POSITION_MOVE_TOLERANCE) ? _V_POSITION_MOVE_TOLERANCE :  ucIVS2DVSDelay);

				//temp = 0x80 -(((Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA) - _PROGRAM_VDELAY) > _V_POSITION_MOVE_TOLERANCE) ? _V_POSITION_MOVE_TOLERANCE : (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA) - _PROGRAM_VDELAY) );
				Scaler_DispSetIpMargin(SLR_IPMARGIN_V_MIN ,temp );
				//rtd_pr_vbe_emerg("Frame sync mode in SetCaptureWindow%c\n",'!');
				rtd_pr_vbe_debug("Frame sync mode in SetCaptureWindow%c\n",'!');
			}
			else
			{

			#if 0
				temp = 0x80 -(((IPV_ACT_STA - _PROGRAM_VDELAY) > _V_POSITION_MOVE_TOLERANCE) ? _V_POSITION_MOVE_TOLERANCE : (IPV_ACT_STA - _PROGRAM_VDELAY) );
				Scaler_DispSetIpMargin(SLR_IPMARGIN_V_MIN ,temp );

				if (IPV_ACT_STA - _PROGRAM_VDELAY > 255)
					ucIVS2DVSDelay = 255;
				else
					ucIVS2DVSDelay = IPV_ACT_STA - _PROGRAM_VDELAY;
			#else
				if(IPV_ACT_STA > (_V_POSITION_MOVE_TOLERANCE + _PROGRAM_VDELAY))
					Scaler_DispSetIpMargin(SLR_IPMARGIN_V_MIN ,0x80 - _V_POSITION_MOVE_TOLERANCE );
				else
					Scaler_DispSetIpMargin(SLR_IPMARGIN_V_MIN, 0x80 - (IPV_ACT_STA - _PROGRAM_VDELAY));

				if (IPV_ACT_STA - _PROGRAM_VDELAY > 255)
				{
					ucIVS2DVSDelay = 255;
					IPV_ACT_STA = _PROGRAM_VDELAY;
				}
				else
				{
					if((IPV_ACT_STA - _PROGRAM_VDELAY)<=_V_POSITION_MOVE_TOLERANCE)
					{
						ucIVS2DVSDelay = IPV_ACT_STA - _PROGRAM_VDELAY;
						IPV_ACT_STA = _PROGRAM_VDELAY;
					}
					else
					{
						ucIVS2DVSDelay = _V_POSITION_MOVE_TOLERANCE;
						IPV_ACT_STA = IPV_ACT_STA -_V_POSITION_MOVE_TOLERANCE;
					}
				}

			#endif

				Scaler_DispSetIvs2DvsDelay(ucIVS2DVSDelay);
				Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPV_ACT_STA , IPV_ACT_STA);
				//rtd_pr_vbe_emerg("Free Run mode in SetCaptureWindow%c\n",'!');
				rtd_pr_vbe_debug("Free Run mode in SetCaptureWindow%c\n",'!');
			}
		}

	}
#else
	Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_STA,0x07f);
	Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPV_ACT_STA,0x00f+4);
#endif

#ifdef CONFIG_SUPPORT_SRC_VGA
	if (_SRC_VGA == Scaler_InputSrcGetMainChType()){
		Scaler_SetHPosition(Scaler_GetHPosition());
		Scaler_SetVPosition(Scaler_GetVPosition());
	}
#endif//#ifdef CONFIG_SUPPORT_SRC_VGA

	if((SCALER_DISP_CHANNEL)_CHANNEL1 == (SCALER_DISP_CHANNEL)display)
	{
		vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);

#ifdef ENABLE_DRIVER_I3DDMA
		if(modestate_I3DDMA_get_In3dMode()){
			//do nothing
		}else{
			vgip_chn1_act_hsta_width_reg.ch1_ih_act_sta = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_STA);
			vgip_chn1_act_hsta_width_reg.ch1_ih_act_wid =Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID);
		}
#else
		vgip_chn1_act_hsta_width_reg.ch1_ih_act_sta = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_STA);
		vgip_chn1_act_hsta_width_reg.ch1_ih_act_wid =Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID);
#endif

		IoReg_Write32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg, vgip_chn1_act_hsta_width_reg.regValue);

		if (_SRC_CVBS == Scaler_InputSrcGetType(display) || _SRC_TV == Scaler_InputSrcGetType(display) || _SRC_SCART == Scaler_InputSrcGetType(display))
		{
			rtd_pr_vbe_info("SLR_INPUT_IPH_ACT_STA_PRE=%d\n", Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_STA_PRE));
			rtd_pr_vbe_info("VD source, fix ucIHSDelay\n");

			vgip_chn1_delay_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_DELAY_reg);
			vgip_chn1_delay_reg.ch1_ihs_dly = ucIHSDelay;
			vgip_chn1_delay_reg.ch1_ivs_dly = ucIVS2DVSDelay;

			IoReg_Write32(VGIP_VGIP_CHN1_DELAY_reg, vgip_chn1_delay_reg.regValue);


		}

		vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);

		//for hdmi smooth toggle portch adjustment
		if ((Scaler_InputSrcGetType(SLR_MAIN_DISPLAY)==_SRC_HDMI)
			&& ((Get_Val_DP_Vsync_Regen_Pos() == DP_REGEN_VSYNC_AT_MAIN_VGIP) || (Get_Val_DP_Vsync_Regen_Pos() == DP_REGEN_VSYNC_AT_HDR1))) {
			new_v_start = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_LEN)
				- Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) - FAKE_PORCH_THR_LINE;
			Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_STA, new_v_start);
		} else {
			vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPV_ACT_STA);
		}
		vgip_chn1_act_vsta_length_reg.ch1_iv_act_len = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPV_ACT_LEN) ;


		IoReg_Write32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg, vgip_chn1_act_vsta_length_reg.regValue);
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else// if(_CHANNEL2 == GET_INPUT_CHANNEL(info->display))
	{
		sub_vgip_vgip_chn2_delay_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_DELAY_reg);
		sub_vgip_vgip_chn2_delay_reg.ch2_ivs_dly = 0;//reset 0
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_DELAY_reg, sub_vgip_vgip_chn2_delay_reg.regValue);

		vgip_chn2_act_hsta_width_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg);
		vgip_chn2_act_hsta_width_reg.ch2_ih_act_sta = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_STA);
		vgip_chn2_act_hsta_width_reg.ch2_ih_act_wid = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID);
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg, vgip_chn2_act_hsta_width_reg.regValue);

		vgip_chn2_act_vsta_length_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg);
		vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPV_ACT_STA);
		vgip_chn2_act_vsta_length_reg.ch2_iv_act_len = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPV_ACT_LEN);
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg, vgip_chn2_act_vsta_length_reg.regValue);

		if (_SRC_CVBS == Scaler_InputSrcGetMainChType() || _SRC_TV == Scaler_InputSrcGetMainChType())
		{
			sub_vgip_vgip_chn2_delay_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_DELAY_reg);
			sub_vgip_vgip_chn2_delay_reg.ch2_ihs_dly = ucIHSDelay;
			sub_vgip_vgip_chn2_delay_reg.ch2_ivs_dly = ucIVS2DVSDelay;
			IoReg_Write32(SUB_VGIP_VGIP_CHN2_DELAY_reg, sub_vgip_vgip_chn2_delay_reg.regValue);
		}
	}
#endif

	// CSW+ 0961011 to set VGIP ctrl bits
#ifdef CONFIG_VGIP_DIGITAL_MODE
		if(_CHANNEL1 == Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL))
		{
			vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
			if(vgip_chn1_ctrl_reg.ch1_digital_mode == 1)
			{
				vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);
				vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta =0;
				IoReg_Write32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg, vgip_chn1_act_vsta_length_reg.regValue);
				vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);
				vgip_chn1_act_hsta_width_reg.ch1_ih_act_sta =0;
				IoReg_Write32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg, vgip_chn1_act_hsta_width_reg.regValue);
			}
		}
#endif
	if ( Scaler_InputSrcGetMainChType() == _SRC_DISPLAYPORT) {

		if(_CHANNEL1 == Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL))
		{
			if(1152 == Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID_PRE))
			{
				vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
				vgip_chn1_ctrl_reg.ch1_safe_mode = _DISABLE;
				//vgip_chn1_ctrl_reg.ch1_hs_syncedge = 0;
				vgip_chn1_ctrl_reg.ch1_vs_syncedge = 1;
				vgip_chn1_ctrl_reg.ch1_vs_by_hs_en_n = 0;
				IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
			}
			else if(800 == Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID_PRE))
			{
				vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
				vgip_chn1_ctrl_reg.ch1_safe_mode = _DISABLE;
				//vgip_chn1_ctrl_reg.ch1_hs_syncedge = 0;
				vgip_chn1_ctrl_reg.ch1_vs_syncedge = 1;
				vgip_chn1_ctrl_reg.ch1_vs_by_hs_en_n = 0;
				IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
			}
			else if(1024 == Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID_PRE))
			{
				vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
				vgip_chn1_ctrl_reg.ch1_safe_mode = _DISABLE;
				//vgip_chn1_ctrl_reg.ch1_hs_syncedge = 0;
				vgip_chn1_ctrl_reg.ch1_vs_syncedge = 0;
				vgip_chn1_ctrl_reg.ch1_vs_by_hs_en_n = 0;
				IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
			}
			else if(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID_PRE) <= 1360)
			{
				vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
				vgip_chn1_ctrl_reg.ch1_safe_mode = _DISABLE;
				//vgip_chn1_ctrl_reg.ch1_hs_syncedge = 0;
				vgip_chn1_ctrl_reg.ch1_vs_syncedge = 0;
				vgip_chn1_ctrl_reg.ch1_vs_by_hs_en_n = 0;
				IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
			}
			else if(1600 == Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID_PRE))
			{
				vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
				vgip_chn1_ctrl_reg.ch1_safe_mode = _DISABLE;
				//vgip_chn1_ctrl_reg.ch1_hs_syncedge = 0;
				vgip_chn1_ctrl_reg.ch1_vs_syncedge = 0;
				vgip_chn1_ctrl_reg.ch1_vs_by_hs_en_n = 0;
				IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
			}
			else
			{
				vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
				vgip_chn1_ctrl_reg.ch1_safe_mode = _DISABLE;
				//vgip_chn1_ctrl_reg.ch1_hs_syncedge = 0;
				vgip_chn1_ctrl_reg.ch1_vs_syncedge = 1;
				vgip_chn1_ctrl_reg.ch1_vs_by_hs_en_n = 0;
				IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
			}

			if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) && (Get_Val_DP_Vsync_Regen_Pos() == DP_NO_REGEN_VSYNC)) {
				unsigned int polarity=Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_SYNC_HIGH_PERIOD);
				vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
				if ((polarity>>1)&0x01)
					vgip_chn1_ctrl_reg.ch1_vs_inv = 0;
				else
					vgip_chn1_ctrl_reg.ch1_vs_inv = 1;

				if (polarity&0x01)
					vgip_chn1_ctrl_reg.ch1_hs_inv = 0;
				else
					vgip_chn1_ctrl_reg.ch1_hs_inv = 1;
				IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
			}
			// Let VS negtive polarity to positive polarity
			else if ((Scaler_DispGetInputInfo(SLR_INPUT_POLARITY)== _SYNC_HP_VN) || (Scaler_DispGetInputInfo(SLR_INPUT_POLARITY) == _SYNC_HN_VN))	//VS Negative
			{
				vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
				vgip_chn1_ctrl_reg.ch1_vs_inv = _ENABLE;
				IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
			}
		}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		else// if(_CHANNEL2 == GET_INPUT_CHANNEL(info->display))
		{
			if(1152 == Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID_PRE))
			{
				vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
				vgip_chn2_ctrl_reg.ch2_safe_mode = _DISABLE;
				//vgip_chn2_ctrl_reg.ch2_hs_syncedge = 0;
				vgip_chn2_ctrl_reg.ch2_vs_syncedge = 1;
				vgip_chn2_ctrl_reg.ch2_vs_by_hs_en_n = 0;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
			}
			else if(800 == Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID_PRE))
			{
				vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
				vgip_chn2_ctrl_reg.ch2_safe_mode = _DISABLE;
				//vgip_chn2_ctrl_reg.ch2_hs_syncedge = 0;
				vgip_chn2_ctrl_reg.ch2_vs_syncedge = 1;
				vgip_chn2_ctrl_reg.ch2_vs_by_hs_en_n = 0;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
			}
			else if(1024 == Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID_PRE))
			{
				vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
				vgip_chn2_ctrl_reg.ch2_safe_mode = _DISABLE;
				//vgip_chn2_ctrl_reg.ch2_hs_syncedge = 0;
				vgip_chn2_ctrl_reg.ch2_vs_syncedge = 0;
				vgip_chn2_ctrl_reg.ch2_vs_by_hs_en_n = 0;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
			}
			else if(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID_PRE) <= 1360)
			{
				vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
				vgip_chn2_ctrl_reg.ch2_safe_mode = _DISABLE;
				//vgip_chn2_ctrl_reg.ch2_hs_syncedge = 0;
				vgip_chn2_ctrl_reg.ch2_vs_syncedge = 0;
				vgip_chn2_ctrl_reg.ch2_vs_by_hs_en_n = 0;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
			}
			else if(1600 == Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID_PRE))
			{
				vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
				vgip_chn2_ctrl_reg.ch2_safe_mode = _DISABLE;
				//vgip_chn2_ctrl_reg.ch2_hs_syncedge = 0;
				vgip_chn2_ctrl_reg.ch2_vs_syncedge = 0;
				vgip_chn2_ctrl_reg.ch2_vs_by_hs_en_n = 0;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
			}
			else
			{
				vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
				vgip_chn2_ctrl_reg.ch2_safe_mode = _DISABLE;
				//vgip_chn2_ctrl_reg.ch2_hs_syncedge = 0;
				vgip_chn2_ctrl_reg.ch2_vs_syncedge = 1;
				vgip_chn2_ctrl_reg.ch2_vs_by_hs_en_n = 0;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
			}

			if(Get_DisplayMode_Src(SLR_SUB_DISPLAY)  == VSC_INPUTSRC_HDMI)
			{
				unsigned int polarity=Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_V_SYNC_HIGH_PERIOD);
				vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
				if ((polarity>>1)&0x01)
					vgip_chn2_ctrl_reg.ch2_vs_inv = 0;
				else
					vgip_chn2_ctrl_reg.ch2_vs_inv = 1;

				if (polarity&0x01)
					vgip_chn2_ctrl_reg.ch2_hs_inv = 0;
				else
					vgip_chn2_ctrl_reg.ch2_hs_inv = 1;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
			}
			// Let VS negtive polarity to positive polarity
			else if ((Scaler_DispGetInputInfo(SLR_INPUT_POLARITY)== _SYNC_HP_VN) || (Scaler_DispGetInputInfo(SLR_INPUT_POLARITY) == _SYNC_HN_VN))	//VS Negative
			{
				vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
				vgip_chn2_ctrl_reg.ch2_vs_inv = _ENABLE;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
			}

		}
#endif
	  }else if(Scaler_InputSrcGetMainChType() == _SRC_YPBPR) {
		if((SCALER_DISP_CHANNEL)_CHANNEL1 == (SCALER_DISP_CHANNEL)display)
		{
			vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
			vgip_chn1_ctrl_reg.ch1_field_inv = _ENABLE;
			vgip_chn1_ctrl_reg.ch1_field_det_en = _ENABLE;
			vgip_chn1_ctrl_reg.ch1_vs_inv = _DISABLE;
			IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
		}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		else// if(_CHANNEL2 == GET_INPUT_CHANNEL(info->display))
		{
			vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
			vgip_chn2_ctrl_reg.ch2_field_inv = _ENABLE;
			vgip_chn2_ctrl_reg.ch2_field_det_en = _ENABLE;
			vgip_chn2_ctrl_reg.ch2_vs_inv = _DISABLE;
			IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
		}
#endif

	}
#ifdef CONFIG_INPUT_SOURCE_IPG
	else if(Scaler_InputSrcGetMainChType() == _SRC_IPG) {
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		IoReg_Mask32((SCALER_DISP_CHANNEL)display ? SUB_VGIP_VGIP_CHN2_CTRL_reg : VGIP_VGIP_CHN1_CTRL_reg, ~_BIT3,_BIT16 | _BIT18);
#else
		IoReg_Mask32(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? SUB_VGIP_VGIP_CHN2_CTRL_reg : */VGIP_VGIP_CHN1_CTRL_reg, ~_BIT3,_BIT16 | _BIT18);
#endif
	}
#endif
	 else if(Scaler_InputSrcGetMainChType() == _SRC_VGA){  // VGA

		if((SCALER_DISP_CHANNEL)_CHANNEL1 == (SCALER_DISP_CHANNEL)display)
		{
			vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
			vgip_chn1_ctrl_reg.ch1_field_inv = _ENABLE;
			vgip_chn1_ctrl_reg.ch1_field_det_en = _ENABLE;
			vgip_chn1_ctrl_reg.ch1_vs_inv = _DISABLE;
			IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
		}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		else// if(_CHANNEL2 == GET_INPUT_CHANNEL(info->display))
		{
			vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
			vgip_chn2_ctrl_reg.ch2_field_inv = _ENABLE;
			vgip_chn2_ctrl_reg.ch2_field_det_en = _ENABLE;
			vgip_chn2_ctrl_reg.ch2_vs_inv = _DISABLE;
			IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
		}
#endif
	}
	else if((Scaler_InputSrcGetMainChType() == _SRC_VO) && (get_force_i3ddma_enable(display) == true)
	 	/*&& Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display, SLR_DISP_INTERLACE)*/ && (display == SLR_MAIN_DISPLAY))
	 	{
	 		rtd_pr_vbe_debug("idma interlace always set field\n");
			vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
			vgip_chn1_ctrl_reg.ch1_field_det_en = _DISABLE;		//bit16
			vgip_chn1_ctrl_reg.ch1_field_sync_edge = _ENABLE;	//bit17
			vgip_chn1_ctrl_reg.ch1_field_inv = _DISABLE;		//bit18
			//vgip_chn1_ctrl_reg.ch1_vs_inv = _DISABLE;			//3
			IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);

			ucIVS2DVSDelay = 0;
			vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);
			vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);

			if(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display, SLR_DISP_INTERLACE) == 0) //progressive
			{
				if(vgip_chn1_ctrl_reg.ch1_digital_mode == 1)
				{
#ifdef CONFIG_VGIP_DIGITAL_MODE
					vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta = 0;
					vgip_chn1_act_hsta_width_reg.ch1_ih_act_sta = 0;
					IoReg_Write32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg, vgip_chn1_act_vsta_length_reg.regValue);
					IoReg_Write32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg, vgip_chn1_act_hsta_width_reg.regValue);
#endif
				}
			}
			else
			{
				ucIVS2DVSDelay =VGIP_VS_DELAY;
				if(vgip_chn1_ctrl_reg.ch1_digital_mode == 0)
				{
					vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);
					if(vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta > VGIP_VS_DELAY)
					{
						vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta = vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta - ucIVS2DVSDelay;
						IoReg_Write32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg, vgip_chn1_act_vsta_length_reg.regValue);
					}
					else
						rtd_pr_vsc_notice("Warning: main active start(%d) is smaller than %d\n", vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta, ucIVS2DVSDelay);
				}
				else
				{
#ifdef CONFIG_VGIP_DIGITAL_MODE
					vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta =0;
					IoReg_Write32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg, vgip_chn1_act_vsta_length_reg.regValue);
					vgip_chn1_act_hsta_width_reg.ch1_ih_act_sta =0;
					IoReg_Write32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg, vgip_chn1_act_hsta_width_reg.regValue);
#endif
				}
				vgip_chn1_delay_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_DELAY_reg);
				vgip_chn1_delay_reg.ch1_ivs_dly = ucIVS2DVSDelay;	// according liyu chen suggestion
				IoReg_Write32(VGIP_VGIP_CHN1_DELAY_reg, vgip_chn1_delay_reg.regValue);
			}
			if(get_hdmi_4k_hfr_mode() == HDMI_4K120_2_0)
			{
				vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
				vgip_chn2_ctrl_reg.ch2_field_det_en = _DISABLE;		//bit16
				vgip_chn2_ctrl_reg.ch2_field_sync_edge = _ENABLE;	//bit17
				vgip_chn2_ctrl_reg.ch2_field_inv = _DISABLE;	//bit18
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);

				vgip_chn2_act_vsta_length_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg);
				vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = 0;
				vgip_chn2_act_vsta_length_reg.ch2_iv_act_len = Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_IPV_ACT_LEN);

				IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg, vgip_chn2_act_vsta_length_reg.regValue);
				vgip_chn2_act_hsta_width_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg);
				vgip_chn2_act_hsta_width_reg.ch2_ih_act_sta = 0;
				vgip_chn2_act_hsta_width_reg.ch2_ih_act_wid = Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_IPH_ACT_WID);
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg, vgip_chn2_act_hsta_width_reg.regValue);


				sub_vgip_vgip_chn2_delay_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_DELAY_reg);
				sub_vgip_vgip_chn2_delay_reg.ch2_ivs_dly = 0;	// according liyu chen suggestion
				sub_vgip_vgip_chn2_delay_reg.ch2_ihs_dly = 0;	// according liyu chen suggestion
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_DELAY_reg, sub_vgip_vgip_chn2_delay_reg.regValue);

            }

			rtd_pr_vbe_debug("[%s][%d]vgip_chn1_ctrl_reg.regValue = %x\n", __FUNCTION__, __LINE__, vgip_chn1_ctrl_reg.regValue);
			rtd_pr_vbe_debug("[%s][%d]VGIP_VGIP_CHN1_CTRL_reg = %x\n", __FUNCTION__, __LINE__, IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg));
	}
	else if((Scaler_InputSrcGetMainChType() == _SRC_VO) && (display == SLR_MAIN_DISPLAY))
	{
#ifdef CONFIG_VGIP_DIGITAL_MODE
		vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
		if(vgip_chn1_ctrl_reg.ch1_digital_mode)
		{
			vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);
			vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta = 0;
			IoReg_Write32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg, vgip_chn1_act_vsta_length_reg.regValue);
			vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);
			vgip_chn1_act_hsta_width_reg.ch1_ih_act_sta = 0;
			IoReg_Write32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg, vgip_chn1_act_hsta_width_reg.regValue);
		}
#endif
	}
	
#ifdef CONFIG_DUAL_CHANNEL
	if( display == SLR_SUB_DISPLAY)
	{
		if(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display, SLR_DISP_INTERLACE))
		{
			//sub
			ucIVS2DVSDelay = 0;
			vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
			vgip_chn2_ctrl_reg.ch2_field_det_en = _DISABLE;		//bit16
			vgip_chn2_ctrl_reg.ch2_field_sync_edge = _ENABLE;	//bit17
			#ifdef CONFIG_I2RND_ENABLE
				if(Scaler_I2rnd_get_timing_enable() == _ENABLE)
					vgip_chn2_ctrl_reg.ch2_field_inv = _DISABLE;	//bit18
				else
					vgip_chn2_ctrl_reg.ch2_field_inv = _ENABLE;		//bit18
			#else
			//	vgip_chn2_ctrl_reg.ch2_field_inv = _ENABLE;		//bit18
			#endif
			IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);

			vgip_chn2_act_vsta_length_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg);
			if(vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta >= VGIP_VS_DELAY)
			{
				ucIVS2DVSDelay = VGIP_VS_DELAY;
				if(vgip_chn2_ctrl_reg.ch2_digital_mode)
					vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = 0;
				else
					vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPV_ACT_STA_PRE) - ucIVS2DVSDelay;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg, vgip_chn2_act_vsta_length_reg.regValue);
			}
			else {
				rtd_pr_vsc_notice("Warning: sub active start(%d) is smaller than %d\n", vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta, ucIVS2DVSDelay);
				if(vgip_chn2_ctrl_reg.ch2_digital_mode) {
					vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = 0;
					IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg, vgip_chn2_act_vsta_length_reg.regValue);
				}
			}
			if(vgip_chn2_ctrl_reg.ch2_digital_mode)
			{
				vgip_chn2_act_hsta_width_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg);
				vgip_chn2_act_hsta_width_reg.ch2_ih_act_sta = 0;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg, vgip_chn2_act_hsta_width_reg.regValue);
			}

			sub_vgip_vgip_chn2_delay_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_DELAY_reg);
			sub_vgip_vgip_chn2_delay_reg.ch2_ivs_dly = ucIVS2DVSDelay;	// according liyu chen suggestion
			IoReg_Write32(SUB_VGIP_VGIP_CHN2_DELAY_reg, sub_vgip_vgip_chn2_delay_reg.regValue);
		}
		else
		{
			vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
			if(vgip_chn2_ctrl_reg.ch2_digital_mode) {
				vgip_chn2_act_vsta_length_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg);
				vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = 0;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg, vgip_chn2_act_vsta_length_reg.regValue);
				vgip_chn2_act_hsta_width_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg);
				vgip_chn2_act_hsta_width_reg.ch2_ih_act_sta = 0;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg, vgip_chn2_act_hsta_width_reg.regValue);

			}
		}

	}
#endif
}
#ifndef BUILD_QUICK_SHOW
void fw_scaler_set_3D_depth(void)
{
//	unsigned int pixelShiftCount;
//	unsigned int hactSta_Frame;
	if (Get_PANEL_SCALER_2D_3D_CVT_HWSHIFT_ENABLE())
	{
		rtd_pr_vbe_debug("[TODO][3D]new Function@%s\n", __FUNCTION__);

	}
}
#endif

void fw_scaler_set_vgip(unsigned char channel, unsigned char src, unsigned char mode)
{
	vgip_vgip_chn1_ctrl_RBUS vgip_chn1_ctrl_reg;
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	sub_vgip_vgip_chn2_ctrl_RBUS vgip_chn2_ctrl_reg;
#endif
#if 0//def CONFIG_VGIP_DIGITAL_MODE
		if(src == VGIP_SRC_VODMA1 || src == VGIP_SRC_VODMA2 || src == VGIP_SRC_TMDS)
			mode = VGIP_MODE_DIGITAL;
		else
			mode = VGIP_MODE_ANALOG;
#endif

	if ( _CHANNEL1 == channel )
	{
		vgip_vgip_chn1_misc_RBUS vgip_chn1_misc_reg;
		vgip_ich1_vgip_htotal_RBUS vgip_ich1_vgip_htotal_reg;
		// Disable SRC_Clock_Enbale bit
#ifdef CONFIG_I2RND_ENABLE
		if((channel == SLR_MAIN_DISPLAY)&&(check_Sub_I2rnd_done() == FALSE)&&(Scaler_I2rnd_get_timing_enable() == _ENABLE))
#endif
		{
			vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
			vgip_chn1_ctrl_reg.ch1_in_clk_en = 0;
			IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
		}
		// 2nd determine the input source
		vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
		vgip_chn1_ctrl_reg.ch1_in_sel = src;
		vgip_chn1_ctrl_reg.ch1_digital_mode = mode;

		IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
		// 3rd enable SRC_Clock_Enbale bit
		vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
		vgip_chn1_ctrl_reg.ch1_in_clk_en = 1;
		IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
		if(drvif_scaler_decide_idmaVgip_syncTo_mainVgip()){
			dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
			dma_vgip_chn1_ctrl_reg.dma_in_clk_en = 1;
			IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);
			rtd_pr_vbe_debug("[IDMA] Src, VgipM/DMA=%d, %d/%d\n", src, vgip_chn1_ctrl_reg.ch1_in_sel, dma_vgip_chn1_ctrl_reg.dma_in_sel );
		}

		vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
		vgip_chn1_misc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_MISC_reg);
		vgip_ich1_vgip_htotal_reg.regValue = IoReg_Read32(VGIP_ICH1_VGIP_HTOTAL_reg);
		if(vgip_chn1_ctrl_reg.ch1_digital_mode)
		{//digital mode
			vgip_chn1_misc_reg.ch1_hs_mode = 0;
			//vgip_chn1_ctrl_reg.ch1_vs_by_hs_en_n = 1;no nned
			vgip_ich1_vgip_htotal_reg.ch1_htotal_num = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_H_LEN) - 1;//set h toal
		}
		else
		{
			vgip_chn1_misc_reg.ch1_hs_mode = 1;
			//vgip_chn1_ctrl_reg.ch1_vs_by_hs_en_n = 0;
			vgip_ich1_vgip_htotal_reg.ch1_htotal_num = 0;//set h toal
		}
		IoReg_Write32(VGIP_VGIP_CHN1_MISC_reg, vgip_chn1_misc_reg.regValue);
		IoReg_Write32(VGIP_ICH1_VGIP_HTOTAL_reg, vgip_ich1_vgip_htotal_reg.regValue);

		rtd_pr_vbe_debug("[IDMA] Src, VgipM=%d, %d\n", src, vgip_chn1_ctrl_reg.ch1_in_sel);
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else	// if ( _CHANNEL2 == channel )
	{
		sub_vgip_vgip_chn2_misc_RBUS sub_vgip_vgip_chn2_misc_reg;
		sub_vgip_ich2_vgip_htotal_RBUS sub_vgip_ich2_vgip_htotal_reg;
		// Disable SRC_Clock_Enbale bit
		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		vgip_chn2_ctrl_reg.ch2_in_clk_en = 0;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);

		// 2nd determine the input source
		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		vgip_chn2_ctrl_reg.ch2_in_sel = src;
		vgip_chn2_ctrl_reg.ch2_digital_mode = mode;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);

		// 3rd enable SRC_Clock_Enbale bit
		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		vgip_chn2_ctrl_reg.ch2_in_clk_en = 1;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);

		sub_vgip_vgip_chn2_misc_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_MISC_reg);
		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		sub_vgip_ich2_vgip_htotal_reg.regValue = IoReg_Read32(SUB_VGIP_ICH2_VGIP_HTOTAL_reg);
		if(vgip_chn2_ctrl_reg.ch2_digital_mode)
		{//digital mode
			sub_vgip_vgip_chn2_misc_reg.ch2_hs_mode = 0;
			//vgip_chn2_ctrl_reg.ch2_vs_by_hs_en_n = 1;//no need
			sub_vgip_ich2_vgip_htotal_reg.ch2_htotal_num = Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_H_LEN) - 1;//set h toal
		}
		else
		{
			sub_vgip_vgip_chn2_misc_reg.ch2_hs_mode = 1;
			//vgip_chn2_ctrl_reg.ch2_vs_by_hs_en_n = 0;
			sub_vgip_ich2_vgip_htotal_reg.ch2_htotal_num = 0;//set h toal
		}
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_MISC_reg, sub_vgip_vgip_chn2_misc_reg.regValue);
		//IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
		IoReg_Write32(SUB_VGIP_ICH2_VGIP_HTOTAL_reg, sub_vgip_ich2_vgip_htotal_reg.regValue);
	}
#endif
}

void fw_scaler_set_sample(unsigned char channel, unsigned char enable)
{
	vgip_vgip_chn1_ctrl_RBUS vgip_chn1_ctrl_reg;
//	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	sub_vgip_vgip_chn2_ctrl_RBUS vgip_chn2_ctrl_reg;
#endif

	if(_CHANNEL1 == channel)
	{
		vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
		vgip_chn1_ctrl_reg.ch1_ivrun = enable;
		IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
#ifdef ENABLE_DRIVER_I3DDMA
		if(!enable || (Scaler_DispGetInputInfo(SLR_INPUT_THROUGH_I3DDMA) == TRUE)){
			dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
			//dma_vgip_chn1_ctrl_reg.dma_ivrun = enable;
			IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);
		}
#endif
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else// if(_CHANNEL2 == channel)
	{
		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		vgip_chn2_ctrl_reg.ch2_ivrun = enable;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
	}
#endif
}

void drvif_scaler_check_vgip_data_dclk(unsigned char channel)
{
	if(_CHANNEL1 == channel)
	{
		vgip_vgip_chn1_ctrl_RBUS vgip_chn1_ctrl_reg;
		vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
		if((_ENABLE != vgip_chn1_ctrl_reg.ch1_ivrun) ||(_ENABLE != vgip_chn1_ctrl_reg.ch1_in_clk_en))
		{
			vgip_chn1_ctrl_reg.ch1_ivrun = 1;
			vgip_chn1_ctrl_reg.ch1_in_clk_en= 1;
			IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
			mdelay(2);
			rtd_pr_vbe_debug("1EnVGIP Data.Dclk\n");
		}
	}
	else //sub
	{
		sub_vgip_vgip_chn2_ctrl_RBUS vgip_chn2_ctrl_reg;
		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		if((_ENABLE!= vgip_chn2_ctrl_reg.ch2_ivrun)||(_ENABLE != vgip_chn2_ctrl_reg.ch2_in_clk_en))
		{
			vgip_chn2_ctrl_reg.ch2_ivrun = 1;
			vgip_chn2_ctrl_reg.ch2_in_clk_en= 1;
			IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
			mdelay(2);
			rtd_pr_vbe_debug("2EnVGIP Data.Dclk\n");
		}
	}

}

static unsigned char dp_source_regen_vsync_pos = DP_NO_REGEN_VSYNC;//decide the regen v sync position
void Set_Val_DP_Vsync_Regen_Pos(unsigned char pos)
{//pos: _DP_VSYNC_REGEN_POS
	rtd_printk(KERN_INFO, TAG_NAME_VSC, "###[%s] (%d)\n", __FUNCTION__, pos);
	dp_source_regen_vsync_pos = pos;
}

unsigned char Get_Val_DP_Vsync_Regen_Pos(void)
{//Get the dp regen position
	return dp_source_regen_vsync_pos;
}

unsigned char decide_dp_source_regen_vsync(void)
{//return true: need to regen vsync. Check porch size
#if IS_ENABLED(CONFIG_RTK_DPRX)  // if CONFIG_RTK_DPRX is built-in =m (CONFIG_RTK_DPRX_MODULE) & module =y (CONFIG_RTK_DPRX)
	DPRX_TIMING_INFO_T dp_timing_info = {0};

	if(drvif_Dprx_GetRawTimingInfo(&dp_timing_info) != TRUE)
		return FALSE;

	if(DP_PTG_REGEN_NO_VSYNC_MODE == dp_timing_info.ptg_mode) {
		return TRUE;
	} else
		return FALSE;
#else  // if CONFIG_RTK_DPRX=n, end this function.
	return FALSE;
#endif
}

#ifndef BUILD_QUICK_SHOW
void fw_scaler_set_vgip_3d_mode(unsigned char channel, unsigned char enable)
{
	vgip_vgip_chn1_ctrl_RBUS vgip_chn1_ctrl_reg;
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	sub_vgip_vgip_chn2_ctrl_RBUS vgip_chn2_ctrl_reg;
#endif

	if(_CHANNEL1 == channel)
	{
		vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
		vgip_chn1_ctrl_reg.ch1_3d_mode_en = enable;
		IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else
	{
		vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		vgip_chn2_ctrl_reg.ch2_3d_mode_en = enable;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, vgip_chn2_ctrl_reg.regValue);
	}
#endif
}

void fw_scaler_ctrl_vgip_regen_vsync(unsigned char channel, unsigned char enable)
{
	vgip_vgip_chn1_regen_vs_ctrl1_RBUS vgip_vgip_chn1_regen_vs_ctrl1_reg;
	vgip_vgip_chn1_regen_vs_ctrl2_RBUS vgip_vgip_chn1_regen_vs_ctrl2_reg;
#ifdef CONFIG_DUAL_CHANNEL
	sub_vgip_vgip_chn2_regen_vs_ctrl1_RBUS sub_vgip_vgip_chn2_regen_vs_ctrl1_reg;
	sub_vgip_vgip_chn2_regen_vs_ctrl2_RBUS sub_vgip_vgip_chn2_regen_vs_ctrl2_reg;
#endif

	unsigned int input_src_htotal = Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_H_LEN);
	unsigned int input_src_vtotal = Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_V_LEN);
	unsigned int input_src_v_act  = Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_IPV_ACT_LEN_PRE);

	if(_CHANNEL1 == channel) {
		if(enable) {
			if((input_src_vtotal - input_src_v_act) < FAKE_PORCH_MIN_V_BLANK) {
				rtd_printk(KERN_INFO, TAG_NAME_VSC, "#####[%s(%d)]Abnormal timing can not enable fake vsync!\n", __FUNCTION__, __LINE__);
				return;
			}
			Set_Val_DP_Vsync_Regen_Pos(DP_REGEN_VSYNC_AT_MAIN_VGIP);
			vgip_vgip_chn1_regen_vs_ctrl1_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_REGEN_VS_CTRL1_reg);
			vgip_vgip_chn1_regen_vs_ctrl1_reg.ch1_fake_porch_thr_line = FAKE_PORCH_THR_LINE;
			IoReg_Write32(VGIP_VGIP_CHN1_REGEN_VS_CTRL1_reg, vgip_vgip_chn1_regen_vs_ctrl1_reg.regValue);

			vgip_vgip_chn1_regen_vs_ctrl2_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_REGEN_VS_CTRL2_reg);
			vgip_vgip_chn1_regen_vs_ctrl2_reg.ch1_fake_porch_en = enable;
			vgip_vgip_chn1_regen_vs_ctrl2_reg.ch1_fake_porch_sel = 1;
			vgip_vgip_chn1_regen_vs_ctrl2_reg.ch1_fake_vs_width = 2;
			vgip_vgip_chn1_regen_vs_ctrl2_reg.ch1_fake_porch_line_total = input_src_htotal;
			IoReg_Write32(VGIP_VGIP_CHN1_REGEN_VS_CTRL2_reg, vgip_vgip_chn1_regen_vs_ctrl2_reg.regValue);
		} else {
			vgip_vgip_chn1_regen_vs_ctrl2_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_REGEN_VS_CTRL2_reg);
			vgip_vgip_chn1_regen_vs_ctrl2_reg.ch1_fake_porch_en = 0;
			vgip_vgip_chn1_regen_vs_ctrl2_reg.ch1_fake_porch_sel = 0;
			IoReg_Write32(VGIP_VGIP_CHN1_REGEN_VS_CTRL2_reg, vgip_vgip_chn1_regen_vs_ctrl2_reg.regValue);
		}
	}
#ifdef CONFIG_DUAL_CHANNEL
	else {
		if(enable) {
			if((input_src_vtotal - input_src_v_act) < FAKE_PORCH_MIN_V_BLANK) {
				rtd_printk(KERN_INFO, TAG_NAME_VSC, "#####[%s(%d)]Abnormal timing can not enable fake vsync!\n", __FUNCTION__, __LINE__);
				return;
			}

			sub_vgip_vgip_chn2_regen_vs_ctrl1_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_REGEN_VS_CTRL1_reg);
			sub_vgip_vgip_chn2_regen_vs_ctrl1_reg.ch2_fake_porch_thr_line = FAKE_PORCH_THR_LINE;
			IoReg_Write32(SUB_VGIP_VGIP_CHN2_REGEN_VS_CTRL1_reg, sub_vgip_vgip_chn2_regen_vs_ctrl1_reg.regValue);

			sub_vgip_vgip_chn2_regen_vs_ctrl2_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_REGEN_VS_CTRL2_reg);
			sub_vgip_vgip_chn2_regen_vs_ctrl2_reg.ch2_fake_porch_en = enable;
			sub_vgip_vgip_chn2_regen_vs_ctrl2_reg.ch2_fake_porch_sel = 1;
			sub_vgip_vgip_chn2_regen_vs_ctrl2_reg.ch2_fake_vs_width = 2;
			sub_vgip_vgip_chn2_regen_vs_ctrl2_reg.ch2_fake_porch_line_total = input_src_htotal;
			IoReg_Write32(SUB_VGIP_VGIP_CHN2_REGEN_VS_CTRL2_reg, sub_vgip_vgip_chn2_regen_vs_ctrl2_reg.regValue);
		} else {
			sub_vgip_vgip_chn2_regen_vs_ctrl2_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_REGEN_VS_CTRL2_reg);
			sub_vgip_vgip_chn2_regen_vs_ctrl2_reg.ch2_fake_porch_en = 0;
			sub_vgip_vgip_chn2_regen_vs_ctrl2_reg.ch2_fake_porch_sel = 0;
			IoReg_Write32(SUB_VGIP_VGIP_CHN2_REGEN_VS_CTRL2_reg, sub_vgip_vgip_chn2_regen_vs_ctrl2_reg.regValue);
		}
	}
#endif
}

unsigned char fw_scaler_get_M_capture_LR_ctrl_by_HW(void)
{
	unsigned char result=FALSE;

#ifdef ENABLE_M_CAPTURE_LR_CTRL_BY_HW
	sc_verid_RBUS sc_verid_RBUS_reg;
	VCLK_CTRL_RBUS VCLK_CTRL_RBUS_reg;

	sc_verid_RBUS_reg.regValue = IoReg_Read32(CRT_SC_VERID_VADDR);

	// Ver.D support this function
	if((sc_verid_RBUS_reg.vercode == CRT_VERCODE_A) || (sc_verid_RBUS_reg.vercode == CRT_VERCODE_B_C))
		return FALSE;

	VCLK_CTRL_RBUS_reg.regValue = IoReg_Read32(VGIP_VCLK_CTRL_VADDR);
	result = VCLK_CTRL_RBUS_reg.v8_dummy & _BIT0;
#endif

	return result;
}


void fw_scaler_set_Main2_vgip_3d_mode(unsigned char enable)
{
	return;

#if 0
	main_2_vgip_vgip_chn1_ctrl_RBUS main_2_vgip_chn1_ctrl_reg;
	unsigned char leftSyncEdge=0, lflag_inv=0;
	sc_verid_RBUS sc_verid_RBUS_reg;

	main_2_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_MAIN_2_VGIP_CHN1_CTRL_VADDR);

#ifdef ENABLE_M_CAPTURE_LR_CTRL_BY_HW // [#0051720][ECO.D] fix M/D domain L-flag not active in Sensio input source issue in Magellan Ver.A/B/C
	sc_verid_RBUS_reg.regValue = IoReg_Read32(CRT_SC_VERID_VADDR);
	if((sc_verid_RBUS_reg.vercode != CRT_VERCODE_A) && (sc_verid_RBUS_reg.vercode != CRT_VERCODE_B_C))
	{
		// enable M/D domain L-flag refer to Main 2nd VGIP L_Flag
		VCLK_CTRL_RBUS VCLK_CTRL_RBUS_reg;
		VCLK_CTRL_RBUS_reg.regValue = IoReg_Read32(VGIP_VCLK_CTRL_VADDR);
		VCLK_CTRL_RBUS_reg.v8_dummy = enable; // enable/disable L-flag HW auto toggle
		IoReg_Write32(VGIP_VCLK_CTRL_VADDR, VCLK_CTRL_RBUS_reg.regValue);

		// [??] 3D Left sync need set to VS negedge when M/D-domain L-flag refer to Main 2nd VGIP L-flag status
		if(enable && (Scaler_DispGetInputInfo(SLR_INPUT_THROUGH_I3DDMA) == TRUE)){
			leftSyncEdge = 1;
			lflag_inv = 1;
		}
	}else
#endif
	// [??] 3D Left sync by VS negedge in interlaced video, why?
	if(enable && (Scaler_DispGetInputInfo(SLR_INPUT_THROUGH_I3DDMA) == TRUE) && Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_INTERLACE))
		leftSyncEdge = 1;

	main_2_vgip_chn1_ctrl_reg.ch1_3dleft_sync_edge = leftSyncEdge;
	main_2_vgip_chn1_ctrl_reg.ch1_3dleft_inv = lflag_inv;
	main_2_vgip_chn1_ctrl_reg.ch1_3d_mode_en = enable;
	IoReg_Write32(VGIP_MAIN_2_VGIP_CHN1_CTRL_VADDR, main_2_vgip_chn1_ctrl_reg.regValue);

	return;

#endif
}


void fw_scaler_set_vgip_frame_packing_3d_mode(unsigned char enable)
{
	vgip_framepacking_ctrl1_RBUS framepacking_ctrl1_reg;
	vgip_framepacking_ctrl2_RBUS framepacking_ctrl2_reg;
	vgip_framepacking_interlace_ctrl1_RBUS framepacking_interlace_ctrl1_reg;
	vgip_framepacking_interlace_ctrl2_RBUS framepacking_interlace_ctrl2_reg;
	vgip_framepacking_interlace_ctrl3_RBUS framepacking_interlace_ctrl3_reg;
	vgip_framepacking_interlace_ctrl4_RBUS framepacking_interlace_ctrl4_reg;
	vgip_vgip_chn1_act_hsta_width_RBUS vgip_chn1_act_hsta_width_reg;
	vgip_vgip_chn1_act_vsta_length_RBUS vgip_chn1_act_vsta_length_reg;
	vgip_vgip_chn1_ctrl_RBUS vgip_chn1_ctrl_reg;
	//main_2_vgip_vgip_chn1_ctrl_RBUS main_2_vgip_chn1_ctrl_reg;
	//crt_sc_verid_RBUS sc_verid_RBUS_reg;
	//unsigned char leftSyncEdge=0, lflag_inv=0;

	framepacking_ctrl1_reg.regValue = IoReg_Read32(VGIP_FRAMEPACKING_CTRL1_reg);
	vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
	//main_2_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_MAIN_2_VGIP_CHN1_CTRL_VADDR);

	framepacking_ctrl1_reg.frame_packing_en = enable;
	vgip_chn1_ctrl_reg.ch1_3d_mode_en= enable;
	//main_2_vgip_chn1_ctrl_reg.ch1_3d_mode_en= enable;

        framepacking_interlace_ctrl2_reg.regValue = IoReg_Read32(VGIP_FRAMEPACKING_INTERLACE_CTRL2_reg);
        framepacking_interlace_ctrl3_reg.regValue = IoReg_Read32(VGIP_FRAMEPACKING_INTERLACE_CTRL3_reg);
        framepacking_interlace_ctrl4_reg.regValue = IoReg_Read32(VGIP_FRAMEPACKING_INTERLACE_CTRL4_reg);

	if(enable)
	{
		if(Scaler_InputSrcGetMainChType() == _SRC_VO){
			// VGIP frame packing enable only for HDMI source
			framepacking_ctrl1_reg.frame_packing_en = 0;

			// [#0061252] fix MVC 3D VGIP L-flag not sync to VO VGIP L-flag issue
			vgip_chn1_ctrl_reg.ch1_3dleft_sync_edge = 1;
			vgip_chn1_ctrl_reg.ch1_3dleft_inv = 1;
		}else{
			unsigned int i;
			for(i=0; i< 8; i++){
				if(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) == (hdmi_active_space_table[i].v_act_len)){
					rtd_pr_vbe_debug("[HDMI-FP] Idx/P/vlen[%d/%d/%d], wid/len/space1/2[%d/%d/%d/%d]\n",
						i, hdmi_active_space_table[i].progressive, hdmi_active_space_table[i].v_act_len,
						hdmi_active_space_table[i].h_act_len, hdmi_active_space_table[i].lr_v_act_len,
						hdmi_active_space_table[i].v_active_space1, hdmi_active_space_table[i].v_active_space2);
					break;
				}
			}

			if(i >= 8){
				rtd_pr_vbe_debug("[ERROR] NO FOUND MATCHED HDMI FP TIMING, len=%d@%s.%d\n", Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE), __FUNCTION__, __LINE__);
				return;
			}

			framepacking_ctrl2_reg.regValue = IoReg_Read32(VGIP_FRAMEPACKING_CTRL2_reg);
			vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);
			vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);

			if(drvif_scaler3d_decide_HDMI_framePacking_interlaced_status() == FALSE || get_field_alternative_3d_mode_enable() == TRUE){
				// HDMI progressive FP 3D timing: 1080p/720p/576p/480p
				framepacking_ctrl2_reg.den_mask_start=hdmi_active_space_table[i].lr_v_act_len;
				framepacking_ctrl2_reg.den_mask_end=hdmi_active_space_table[i].lr_v_act_len + hdmi_active_space_table[i].v_active_space1;

				framepacking_ctrl1_reg.vs_width = 2;
				// vact start between each frame should be equal in VGIP analog mode
				framepacking_ctrl1_reg.vs_position = framepacking_ctrl2_reg.den_mask_end - Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA)+1;

				// frame packing field mode = 0
				framepacking_interlace_ctrl1_reg.frame_packing_field_en = 0;

				vgip_chn1_act_hsta_width_reg.ch1_ih_act_wid = hdmi_active_space_table[i].h_act_len;
				vgip_chn1_act_vsta_length_reg.ch1_iv_act_len = hdmi_active_space_table[i].lr_v_act_len;
			}
			else{
				// HDMI interlaced FP 3D timing: 1080i/576i/480i
				// HDMI interlaced frame packing format
				// ---------
				// vact start
				// ---------
				// L-odd
				// ----------
				// vact space1 (VactSpace-A)
				// ----------
				// R-odd
				// ------------------------
				// vact space2 (VactSpace-B)
				// ------------------------
				// L-even
				// ----------
				// vact space1 (VactSpace-C)
				// ----------
				// R-even
				// ---------

				// vsync between R-odd/L-even field (VactSpace-B)
				framepacking_ctrl2_reg.den_mask_start = (hdmi_active_space_table[i].lr_v_act_len * 2) + hdmi_active_space_table[i].v_active_space1;
				framepacking_ctrl2_reg.den_mask_end = framepacking_ctrl2_reg.den_mask_start + hdmi_active_space_table[i].v_active_space2;

				framepacking_ctrl1_reg.vs_width =2;
				framepacking_ctrl1_reg.vs_position = framepacking_ctrl2_reg.den_mask_end - Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA)+1;

				// vsync between L-odd/R-odd field (VactSpace-A)
				framepacking_interlace_ctrl2_reg.den_mask_start_r_odd = hdmi_active_space_table[i].lr_v_act_len;
				framepacking_interlace_ctrl2_reg.den_mask_end_r_odd = framepacking_interlace_ctrl2_reg.den_mask_start_r_odd + hdmi_active_space_table[i].v_active_space1;

				framepacking_interlace_ctrl1_reg.frame_packing_field_en = 1;
				framepacking_interlace_ctrl1_reg.vs_width_r_odd = 2;
				framepacking_interlace_ctrl1_reg.vs_position_r_odd = framepacking_interlace_ctrl2_reg.den_mask_end_r_odd - Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA)+1;

				// vsync between L-even/R-even field (VactSpace-C)
				framepacking_interlace_ctrl4_reg.den_mask_start_r_even = framepacking_ctrl2_reg.den_mask_end + hdmi_active_space_table[i].lr_v_act_len;
				framepacking_interlace_ctrl4_reg.den_mask_end_r_even = framepacking_interlace_ctrl4_reg.den_mask_start_r_even + hdmi_active_space_table[i].v_active_space1;

				framepacking_interlace_ctrl3_reg.vs_width_r_even = 2;
				framepacking_interlace_ctrl3_reg.vs_position_r_even = framepacking_interlace_ctrl4_reg.den_mask_end_r_even - Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA)+1;

				vgip_chn1_act_hsta_width_reg.ch1_ih_act_wid = hdmi_active_space_table[i].h_act_len;
				vgip_chn1_act_vsta_length_reg.ch1_iv_act_len = hdmi_active_space_table[i].lr_v_act_len;

			}

			IoReg_Write32(VGIP_FRAMEPACKING_CTRL2_reg, framepacking_ctrl2_reg.regValue);
			//FixMe
			//IoReg_Write32(VGIP_MAIN_2_FRAMEPACKING_CTRL2_VADDR, framepacking_ctrl2_reg.regValue);
			IoReg_Write32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg, vgip_chn1_act_hsta_width_reg.regValue);
			IoReg_Write32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg, vgip_chn1_act_vsta_length_reg.regValue);

			IoReg_Write32(VGIP_FRAMEPACKING_INTERLACE_CTRL1_reg, framepacking_interlace_ctrl1_reg.regValue);
			//IoReg_Write32(VGIP_MAIN_2_FRAMEPACKING_INTERLACE_CTRL1_VADDR, framepacking_interlace_ctrl1_reg.regValue);
			IoReg_Write32(VGIP_FRAMEPACKING_INTERLACE_CTRL2_reg, framepacking_interlace_ctrl2_reg.regValue);
			//IoReg_Write32(VGIP_MAIN_2_FRAMEPACKING_INTERLACE_CTRL2_VADDR, framepacking_interlace_ctrl2_reg.regValue);
			IoReg_Write32(VGIP_FRAMEPACKING_INTERLACE_CTRL3_reg, framepacking_interlace_ctrl3_reg.regValue);
			//IoReg_Write32(VGIP_MAIN_2_FRAMEPACKING_INTERLACE_CTRL3_VADDR, framepacking_interlace_ctrl3_reg.regValue);
			IoReg_Write32(VGIP_FRAMEPACKING_INTERLACE_CTRL4_reg, framepacking_interlace_ctrl4_reg.regValue);
			//IoReg_Write32(VGIP_MAIN_2_FRAMEPACKING_INTERLACE_CTRL4_VADDR, framepacking_interlace_ctrl4_reg.regValue);

			//IoReg_Write32(VGIP_MAIN_2_VGIP_CHN1_ACT_HSTA_WIDTH_VADDR, vgip_chn1_act_hsta_width_reg.regValue);
			//IoReg_Write32(VGIP_MAIN_2_VGIP_CHN1_ACT_VSTA_LENGTH_VADDR, vgip_chn1_act_vsta_length_reg.regValue);

#if defined(IS_MAGELLAN_ONLY)
			// [Magellan] vertical line delay between main VGIP & main 2nd VGIP need be counted (2 lines)
			if(framepacking_ctrl1_reg.vs_position - 2 > Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA)){
				framepacking_ctrl1_reg.vs_position -= 2;
				rtd_pr_vbe_debug("[SG][HDMI-FP] Main/Main2 VGIPs line buffer delay=2\n");
			}
#endif
		}
	}

	IoReg_Write32(VGIP_FRAMEPACKING_CTRL1_reg, framepacking_ctrl1_reg.regValue);
	//FixMe
	//IoReg_Write32(VGIP_MAIN_2_FRAMEPACKING_CTRL1_VADDR, framepacking_ctrl1_reg.regValue);
	IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_chn1_ctrl_reg.regValue);

	//IoReg_Write32(VGIP_MAIN_2_VGIP_CHN1_CTRL_VADDR, main_2_vgip_chn1_ctrl_reg.regValue);
	rtd_pr_vbe_debug("[3D][%d] FP Main2Delay=%x\n", enable, IoReg_Read32(VGIP_VGIP_CHN1_DELAY_reg));

	//USB VO Playback 3D mode switch 137ms Vsync lost,lost 7 vsync cause TCON abnormal

	/*no 3d case. if 3d case the delay need to tune
	if((Scaler_InputSrcGetMainChType() == _SRC_VO)&&(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()))
		msleep(5);
	*/
}
#endif

extern unsigned int ddomain_isr_get_mv_den_sta_count(void);
extern unsigned int ddomain_isr_get_mv_den_end_count(void);

/*============================================================================*/
/**
 * Hold until the specified event occurs
 *
 * @param domain: [input] I-domain or D-domain to wait.
 * @param ucEvent: [input] Specified event.
 * @return Wait result.
 * @retval TRUE while event happened.
 * @retval FALSE  for timeout.
 */
unsigned char fwif_scaler_wait_for_event(unsigned int addr, unsigned int event)
{
	unsigned int timeoutcnt = 0x032500;
	unsigned int start_count = 0;
	unsigned int end_count = 0;

	if(addr == PPOVERLAY_DTG_pending_status_2_reg){
       #ifndef BUILD_QUICK_SHOW 
		if (event == _BIT0)
			end_count = ddomain_isr_get_mv_den_end_count();
		else if (event == _BIT1)
			start_count = ddomain_isr_get_mv_den_sta_count();
        
      #endif
	}else if((addr == VGIP_VGIP_CHN1_STATUS_reg)&&((event&(_BIT24|_BIT25))>0)){
		;
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else if((addr == SUB_VGIP_VGIP_CHN2_STATUS_reg)&&((event&(_BIT24|_BIT25))>0)){
		;
	}
#endif
	else{
		if((addr == VGIP_VGIP_CHN1_STATUS_reg)
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
			|| (addr == SUB_VGIP_VGIP_CHN2_STATUS_reg)
#endif
			){
			//IoReg_SetBits(addr, value);
			IoReg_Write32(addr, ((IoReg_Read32(addr)&~(_BIT24|_BIT25)) | event));
			IoReg_ClearBits(addr, event|_BIT24|_BIT25);
		}else{
			IoReg_SetBits(addr, event);
			IoReg_ClearBits(addr, event);
		}
	}


	do {
		if (addr == PPOVERLAY_DTG_pending_status_2_reg) {
            #ifndef BUILD_QUICK_SHOW
			if (end_count != ddomain_isr_get_mv_den_end_count())
				return TRUE;
			else if (start_count != ddomain_isr_get_mv_den_sta_count())
				return TRUE;
            #endif
		} else if(IoReg_Read32(addr) & event) {
			return TRUE;
		}
		timeoutcnt--;
	} while(timeoutcnt);


	if(timeoutcnt == 0){
		rtd_pr_vbe_debug("\n Wait the correct line from %s timeout(%x %x) \n", __FUNCTION__, addr, event);
	}

	return FALSE;
}

#ifndef BUILD_QUICK_SHOW

//for full frame time for zoom IEN
/*============================================================================*/
/**
 * Hold until the specified event occurs, not immediate, no busy loop.
 *
 * @param domain: [input] I-domain or D-domain to wait.
 * @param ucEvent: [input] Specified event.
 * @return Wait result.
 * @retval TRUE while event happened.
 * @retval FALSE  for timeout.
 */

unsigned char fwif_scaler_wait_for_event_done(unsigned int addr, unsigned int event)
{
	unsigned int timeoutcnt = 10;
	unsigned int start_count, end_count;
	/*frank@0516 Change below code to avoid system cpu clear interrupt status*/
	if (addr == PPOVERLAY_DTG_pending_status_2_reg) {
		if (event == _BIT0)
			end_count = ddomain_isr_get_mv_den_end_count();
		else if (event == _BIT1)
			start_count = ddomain_isr_get_mv_den_sta_count();
	} else if ((addr == VGIP_VGIP_CHN1_STATUS_reg)
		   && ((event & (_BIT24 | _BIT25)) > 0)) {
		;
	}
#ifdef CONFIG_DUAL_CHANNEL	/* There are two channels*/
	else if ((addr == SUB_VGIP_VGIP_CHN2_STATUS_reg)
		 && ((event & (_BIT24 | _BIT25)) > 0)) {
		;
	}
#endif
	else {
		if ((addr == VGIP_VGIP_CHN1_STATUS_reg)
#ifdef CONFIG_DUAL_CHANNEL	/* There are two channels*/
		    || (addr == SUB_VGIP_VGIP_CHN2_STATUS_reg)
#endif
		    ) {
			/*IoReg_SetBits(addr, value);*/
			IoReg_Write32(addr,
				      ((IoReg_Read32(addr) & ~(_BIT24 | _BIT25))
				       | event));
			IoReg_ClearBits(addr, event | _BIT24 | _BIT25);
		} else {
			IoReg_SetBits(addr, event);
			IoReg_ClearBits(addr, event);
		}
	}

	do {
		if (addr == PPOVERLAY_DTG_pending_status_2_reg) {
			if (event == _BIT0 && end_count != ddomain_isr_get_mv_den_end_count())
				return TRUE;
			else if (event == _BIT1 && start_count != ddomain_isr_get_mv_den_sta_count())
				return TRUE;
		} else if (IoReg_Read32(addr) & event) {
			return TRUE;
		}
		msleep(10);
		timeoutcnt--;
	} while (timeoutcnt);

	if (timeoutcnt == 0) {
		rtd_pr_vbe_debug("\n Wait the correct line from %s timeout(%x %x) \n", __FUNCTION__, addr, event);
	}

	return FALSE;

}
unsigned char fwif_scaler_wait_for_event1(unsigned int addr, unsigned int event)
{
	unsigned int timeoutcnt = 0x032500;
	unsigned int timeoutcnt1 = 0x032500;
	//frank@0516 Change below code to avoid system cpu clear interrupt status
	if((addr == VGIP_VGIP_CHN1_STATUS_reg)&&((event&(_BIT24|_BIT25))>0)){
		;
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else if((addr == SUB_VGIP_VGIP_CHN2_STATUS_reg)&&((event&(_BIT24|_BIT25))>0)){
		;
	}
#endif
	else{
		if((addr == VGIP_VGIP_CHN1_STATUS_reg)
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
			|| (addr == SUB_VGIP_VGIP_CHN2_STATUS_reg)
#endif
			){
			IoReg_Write32(addr, ((IoReg_Read32(addr)&~(_BIT24|_BIT25)) | event));
			IoReg_ClearBits(addr, event|_BIT24|_BIT25);
		}else{
			IoReg_SetBits(addr, event);
			IoReg_ClearBits(addr, event);
		}
	}

	do {
		if(IoReg_Read32(addr) & event) {
			do
			{
			 	if((IoReg_Read32(addr) & event) == 0)
					return TRUE;
				timeoutcnt1--;
			}while(timeoutcnt1);
			return FALSE;
		}
		timeoutcnt--;
	} while(timeoutcnt);

	return FALSE;

}
#endif


void fw_scaler_enable_fs_wdg(unsigned char display, unsigned char enable)
{

#if 0//Remove by Will. We use drvif_mode_onlinemeasure_setting to set watch dog
	scaledown_ich1_ibuff_status_ctrl_RBUS ich1_ibuff_status_ctrl_reg;
	scaledown_ich1_ibuff_status_RBUS ich1_ibuff_status_reg;
	ppoverlay_main_display_control_rsv_RBUS main_display_control_rsv_reg;
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
//	ich2_ibuff_STATUS_RBUS ich2_ibuff_STATUS_reg;
	mp_layout_force_to_background_RBUS mp_layout_force_to_background_reg;
#endif
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;

#ifdef CONFIG_DUAL_CHANNEL
	if(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CHANNEL) == _CHANNEL2)
		return;
#endif //#ifdef CONFIG_DUAL_CHANNEL
	if(enable)
	{

		if(_CHANNEL1 == Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CHANNEL) )
		{
			//write clear error status
			ich1_ibuff_status_reg.regValue = IoReg_Read32(SCALEDOWN_ICH1_IBUFF_STATUS_VADDR);

			ich1_ibuff_status_reg.ibuff_ovf = 1;
			ich1_ibuff_status_reg.fsync_ovf = 1;
			ich1_ibuff_status_reg.fsync_udf = 1;
			IoReg_Write32(SCALEDOWN_ICH1_IBUFF_STATUS_VADDR, ich1_ibuff_status_reg.regValue);

			//set the watch dog for frame sync underflow and overflow
			if(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_FRAMESYNC))
			{
				ich1_ibuff_status_ctrl_reg.regValue = IoReg_Read32(SCALEDOWN_ICH1_IBUFF_STATUS_CTRL_VADDR);
				//frank@1129 add below code to open framesync watch dog ++
				ich1_ibuff_status_ctrl_reg.wd_fsync_ovf_en = 1;
				ich1_ibuff_status_ctrl_reg.wd_fsync_udf_en = 1;
				ich1_ibuff_status_ctrl_reg.wd_ibuff_ovf_en = 1;
				ich1_ibuff_status_ctrl_reg.wd_ibuff_to_main = 1;
				//frank@1129 add below code to open framesync watch dog --
				IoReg_Write32(SCALEDOWN_ICH1_IBUFF_STATUS_CTRL_VADDR, ich1_ibuff_status_ctrl_reg.regValue);
			}
			//set Main Watch Dog Enable(force to background)
			main_display_control_rsv_reg.regValue = IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
			main_display_control_rsv_reg.main_wd_to_background = 1;
			IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, main_display_control_rsv_reg.regValue);
		}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		else// if(_CHANNEL2 == GET_INPUT_CHANNEL(display))
		{
#if 0
			//write clear error status
			ich2_ibuff_STATUS_reg.regValue = IoReg_Read32(SCALEDOWN_ICH2_IBUFF_STATUS_VADDR);
			ich2_ibuff_STATUS_reg.ibuff_ovf = 1;
			ich2_ibuff_STATUS_reg.fsync_ovf = 1;
			ich2_ibuff_STATUS_reg.fsync_udf = 1;
			IoReg_Write32(SCALEDOWN_ICH2_IBUFF_STATUS_VADDR, ich2_ibuff_STATUS_reg.regValue);

			//set the watch dog for frame sync underflow and overflow
			if(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_FRAMESYNC))
			{
				ich2_ibuff_STATUS_reg.regValue = IoReg_Read32(SCALEDOWN_ICH2_IBUFF_STATUS_VADDR);
				ich2_ibuff_STATUS_reg.wd_fsync_ovf_en = 1;
				ich2_ibuff_STATUS_reg.wd_fsync_udf_en = 1;
				IoReg_Write32(SCALEDOWN_ICH2_IBUFF_STATUS_VADDR, ich1_ibuff_status_reg.regValue);
			}

#endif
			//set Sub Watch Dog Enable(force to background)
			mp_layout_force_to_background_reg.regValue = IoReg_Read32(PPOVERLAY_MP_Layout_Force_to_Background_reg);
			mp_layout_force_to_background_reg.sub_wd_to_background = 1;
			IoReg_Write32(PPOVERLAY_MP_Layout_Force_to_Background_reg, mp_layout_force_to_background_reg.regValue);
		}
#endif

		// set frame sync watch dog Enable(force to background)
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.wde_to_free_run = 1;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);

	}
	else
	{
		// set frame sync watch dog disable(force to background)
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.wde_to_free_run = 0;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);

		if(_CHANNEL1 == Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CHANNEL))
		{
			//set Main Watch Dog Disable(force to background)
			main_display_control_rsv_reg.regValue = IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
			main_display_control_rsv_reg.main_wd_to_background = 0;
			IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, main_display_control_rsv_reg.regValue);

			//disable watch dog for frame sync underflow and overflow
			ich1_ibuff_status_ctrl_reg.regValue = IoReg_Read32(SCALEDOWN_ICH1_IBUFF_STATUS_CTRL_VADDR);
			ich1_ibuff_status_ctrl_reg.wd_fsync_ovf_en = 0;
			ich1_ibuff_status_ctrl_reg.wd_fsync_udf_en = 0;
			ich1_ibuff_status_ctrl_reg.wd_ibuff_ovf_en = 0;
			ich1_ibuff_status_ctrl_reg.wd_ibuff_to_main = 0;
			IoReg_Write32(SCALEDOWN_ICH1_IBUFF_STATUS_CTRL_VADDR, ich1_ibuff_status_ctrl_reg.regValue);
		}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		else// if(_CHANNEL2 == GET_INPUT_CHANNEL(display))
		{
			//set Sub Watch Dog Disable(force to background)
			mp_layout_force_to_background_reg.regValue = IoReg_Read32(PPOVERLAY_MP_Layout_Force_to_Background_reg);
			mp_layout_force_to_background_reg.sub_wd_to_background = 0;
			IoReg_Write32(PPOVERLAY_MP_Layout_Force_to_Background_reg, mp_layout_force_to_background_reg.regValue);
#if 0
			//disable watch dog for frame sync underflow and overflow
			ich2_ibuff_STATUS_reg.regValue = IoReg_Read32(SCALEDOWN_ICH2_IBUFF_STATUS_VADDR);
			ich2_ibuff_STATUS_reg.wd_fsync_ovf_en = 0;
			ich2_ibuff_STATUS_reg.wd_fsync_udf_en = 0;
			IoReg_Write32(SCALEDOWN_ICH2_IBUFF_STATUS_VADDR, ich1_ibuff_status_reg.regValue);
#endif
		}
#endif
	}
#endif
}


//must use if value not apply with double buffer
void fw_scaler_double_buffer_maskl(unsigned char display,unsigned int addr,  unsigned int andmask, unsigned int ormask)
{

		//rtdf_clearBits(BUS_SIC_GIE1_VADDR, _BIT26 | _BIT23); //i domain & timer7 for hdmi
//		IoReg_ClearBits(BUS_SIC_M_GIE1_VADDR, _BIT26);
		IoReg_Mask32(addr,andmask,ormask);
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		display ? WaitFor_IVS2():WaitFor_IVS1();
		display ? WaitFor_IVS2():WaitFor_IVS1();
#else
		WaitFor_IVS1();
		WaitFor_IVS1();
#endif
//		IoReg_SetBits(BUS_SIC_M_GIE1_VADDR, _BIT26);
		//rtdf_setBits(BUS_SIC_GIE1_VADDR, _BIT26 | _BIT23);

}

void fw_scaler_dtg_double_buffer_enable(unsigned char bEnable)
{
	unsigned long flags;//for spin_lock_irqsave
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl2_reg;
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

	//double buffer D0
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.dreg_dbuf_en = bEnable;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock

	//double buffer D4
	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = bEnable;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
}

void fw_scaler_dtg_double_buffer_apply(void)
{
	unsigned long flags;//for spin_lock_irqsave
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl2_reg;
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

	//unsigned char timeout = 10;
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.dreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock

	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

/*frank@0903 mark below wait code
	do{
	   msleep(10);
	   double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	   double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
   }while((double_buffer_ctrl_reg.dreg_dbuf_set||double_buffer_ctrl2_reg.uzudtgreg_dbuf_set) && (--timeout));

	if(timeout==0){
		rtd_pr_vbe_info("[%s]WARNING! wait double buf timeout!\n", __FUNCTION__);
	}
*/

}
void fw_scaler_double_buffer_outl(unsigned char display,unsigned int addr,  unsigned int val)
{
	//	rtdf_clearBits(BUS_SIC_GIE1_VADDR, _BIT26);
		IoReg_Write32(addr,val);
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		display ? WaitFor_IVS2():WaitFor_IVS1();
		display ? WaitFor_IVS2():WaitFor_IVS1();
#else
		WaitFor_IVS1();
		WaitFor_IVS1();
#endif
	//	rtdf_setBits(BUS_SIC_GIE1_VADDR, _BIT26);


}
#ifndef BUILD_QUICK_SHOW

#if 0
void fw_scaler_enable_measure_wdg(unsigned char channel, unsigned char enable)
{
	SP_MS1IE_RBUS onms_wd_reg;

	// rtd_pr_vbe_info("WD:%d\n",enable);

#if 0
	if(enable)
		return;
#endif
	if ( Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) ) //sub
		return;

       if(enable) {


	    	if(Scaler_InputSrcGetMainChFrom() == _SRC_FROM_ADC  || (Scaler_InputSrcGetMainChType()==_SRC_DISPLAYPORT) || (Scaler_InputSrcGetMainChType()==_SRC_HDMI)
		|| (Scaler_InputSrcGetMainChFrom() == _SRC_FROM_VDC) || ((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 0)) )
		{

			rtd_pr_vbe_info("enable measure wdg\n");
			onms_wd_reg.regValue =  IoReg_Read32(channel ? SYNCPROCESSOR_SP_MS2IE_VADDR : SYNCPROCESSOR_SP_MS1IE_VADDR);
			if(channel)
				onms_wd_reg.wd_on1_to_sub		= 1;//set watch dog ch2 to sub
			else
				onms_wd_reg.wd_on1_to_main 		= 1;//set watch dog ch1 to main


			onms_wd_reg.wde_on1_vs_per_to 	= 1;//VS period timeout
			onms_wd_reg.wde_on1_vs_per_of	= 1;//VS period overflow

			onms_wd_reg.wde_on1_vs_over_range= 1;//VS period over range
			onms_wd_reg.wde_on1_vs_pol_chg	= 1;//VS polarity change
			onms_wd_reg.wde_on1_hs_per_of	= 1;//HS period overflow
			onms_wd_reg.wde_on1_hs_over_range= 0;//HS period over range
			onms_wd_reg.wde_on1_hs_pol_chg	= 1;//HS polarity change
			IoReg_Write32(channel ? SYNCPROCESSOR_SP_MS2IE_VADDR : SYNCPROCESSOR_SP_MS1IE_VADDR, onms_wd_reg.regValue);
			IoReg_SetBits(PPOVERLAY_Display_Timing_CTRL1_reg, _BIT20);// set frame sync(Free Run) watch dog Enable(force to background)
			rtdf_setBits(PPOVERLAY_Main_Display_Control_RSV_reg,_BIT14);
			IoReg_SetBits(channel? PPOVERLAY_MP_Layout_Force_to_Background_reg : PPOVERLAY_Main_Display_Control_RSV_reg, _BIT15);//set Main Watch Dog Enable(force to background)
		}
	}
	else{
		onms_wd_reg.regValue =  IoReg_Read32(channel ? SYNCPROCESSOR_SP_MS2IE_VADDR : SYNCPROCESSOR_SP_MS1IE_VADDR);
		if(channel)
			onms_wd_reg.wd_on1_to_sub		= 0;//set watch dog ch2 to sub
		else
			onms_wd_reg.wd_on1_to_main 		= 0;//set watch dog ch1 to main
		onms_wd_reg.wde_on1_vs_per_to 	= 0;//VS period timeout
		onms_wd_reg.wde_on1_vs_per_of	= 0;//VS period overflow
		onms_wd_reg.wde_on1_vs_over_range= 0;//VS period over range
		onms_wd_reg.wde_on1_vs_pol_chg	= 0;//VS polarity change
		onms_wd_reg.wde_on1_hs_per_of	= 0;//HS period overflow
		onms_wd_reg.wde_on1_hs_over_range= 0;//HS period over range
		onms_wd_reg.wde_on1_hs_pol_chg	= 0;//HS polarity change
		IoReg_Write32(channel ? SYNCPROCESSOR_SP_MS2IE_VADDR : SYNCPROCESSOR_SP_MS1IE_VADDR, onms_wd_reg.regValue);
		//frank@0817 mark below code to solve mantis29037 ATV always force background
		//IoReg_SetBits(channel? PPOVERLAY_MP_Layout_Force_to_Background_reg: PPOVERLAY_Main_Display_Control_RSV_reg, _BIT15);//set Main Watch Dog Enable(force to background)
		IoReg_ClearBits(channel? PPOVERLAY_MP_Layout_Force_to_Background_reg: PPOVERLAY_Main_Display_Control_RSV_reg, _BIT15);
	}

}
#else //#if 0

void fw_scaler_enable_measure_wdg(unsigned char channel, unsigned char enable)
{

#if 0//Remove by Will. We use drvif_mode_onlinemeasure_setting to set watch dog
	onms1_watchdog_en_RBUS onms_onms1_watchdog_en_reg;
	ppoverlay_main_display_control_rsv_RBUS main_display_control_rsv_reg;
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	onms1_status_RBUS onms_onms1_status_reg;
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	SP_MS2IE_RBUS SP_MS2IE_register;
	mp_layout_force_to_background_RBUS mp_layout_force_to_background_reg;
	SP_MS2Stus_RBUS SP_MS2Stus_reg;
#endif

	//you can choice sub turn on watch-dog or not
	if(SLR_SUB_DISPLAY == Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY))
		return;

	if(_ENABLE == enable)
	{
		//frank@1015 add remove VDC source code to disable WD to solve mantis30272
		//frank@0818 mark HDMI src on-line measure to solve WD not stable problem.
		{
                        if((Scaler_InputSrcGetMainChFrom() == _SRC_FROM_ADC)  || (Scaler_InputSrcGetMainChType()==_SRC_DISPLAYPORT)
				/*|| (Scaler_InputSrcGetMainChType()==_SRC_HDMI) */|| (Scaler_InputSrcGetMainChType() == _SRC_CVBS)//|| (Scaler_InputSrcGetMainChFrom() == _SRC_FROM_VDC)
				|| ((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 0)) )
			{
				//pass
			}
			else
			{
#ifdef CONFIG_ENABLE_OTHER_SRC_MEASURE_WD
				//USER:LewisLee DATE:2012/07/10
				//some DVD player, change timing have garbage
				if(_SRC_HDMI == Scaler_InputSrcGetMainChType())
				{
					if(_DISABLE == Scaler_Get_HDMI_EnableMeasure_WD_Flag())
						return;
				}
				else
				{
					return;
				}
#else //#ifdef CONFIG_ENABLE_OTHER_SRC_MEASURE_WD
				return;
#endif //#ifdef CONFIG_ENABLE_OTHER_SRC_MEASURE_WD
			}
		}

		if(_CHANNEL1 == channel)
		{
			onms_onms1_watchdog_en_reg.regValue = IoReg_Read32(ONMS_onms1_watchdog_en_reg);
			onms_onms1_watchdog_en_reg.on1_wd_to_main = _TRUE;		//Watch Dog select to Main Display
//			SP_MS1IE_register.wd_on1_to_sub			= _FALSE;	//Watch Dog select to Sub Display
			onms_onms1_watchdog_en_reg.on1_wd_vs_per_timeout = _ENABLE;	//No VSYNC occurred
			onms_onms1_watchdog_en_reg.on1_wd_vs_high_timeout= _DISABLE;	//Input VSYNC High Pulse Measurement Time-Out Watch-Dog Enable
			onms_onms1_watchdog_en_reg.on1_wd_vs_per_overflow = _ENABLE;	//Input HSYNC Period Measurement Over-flow Watch-Dog Enable

#if 1
			// Change color std in AV source will cause this bit always pending and can't clear it.
			// ex: input PAL, user sets NTSC
			if (Scaler_InputSrcGetMainChType() != _SRC_CVBS)
				onms_onms1_watchdog_en_reg.on1_wd_vs_per_over_range= _ENABLE;	//VSYNC_OVER_RANGE Watch-Dog Enable
#else //#if 1
			SP_MS1IE_register.wde_on1_vs_over_range	= _ENABLE;	//VSYNC_OVER_RANGE Watch-Dog Enable
#endif //#if 1
			onms_onms1_watchdog_en_reg.on1_wd_vs_pol_chg= _ENABLE;	//Input VSYNC Polarity change Watch-Dog Enable
			onms_onms1_watchdog_en_reg.on1_wd_hs_per_overflow= _ENABLE;	//Input HSYNC Period Measurement Over-flow Watch-Dog Enable
			onms_onms1_watchdog_en_reg.on1_wd_hs_per_over_range= _DISABLE;	//HSYNC_OVER_RANGE Watch-Dog Enable
			onms_onms1_watchdog_en_reg.on1_wd_hs_pol_chg= _ENABLE;	//Input HSYNC Polarity change Watch-Dog Enable
			IoReg_Write32(ONMS_onms1_watchdog_en_reg, onms_onms1_watchdog_en_reg.regValue);

			main_display_control_rsv_reg.regValue = IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
			main_display_control_rsv_reg.main_wd_to_background	= _ENABLE;	//Main Watch Dog Enable (Force-to-Background)
			main_display_control_rsv_reg.main_wd_to_free_run		= _ENABLE;	//FrameSync Main Watch Dog Enable (FreeRun)
			IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, main_display_control_rsv_reg.regValue);
		}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		else// if(_CHANNEL2 == channel)
		{
			SP_MS2IE_register.regValue = IoReg_Read32(SYNC_PROCESSOR_SP_MS2IE_VADDR);
			SP_MS2IE_register.wd_on2_to_sub			= _TRUE;		//Watch Dog select to Sub Display
			SP_MS2IE_register.wde_on2_vs_per_to		= _ENABLE;	//No VSYNC occurred
			SP_MS2IE_register.wde_on2_vs_high_to		= _DISABLE;	//Input VSYNC High Pulse Measurement Time-Out Watch-Dog Enable
			SP_MS2IE_register.wde_on2_vs_per_of		= _ENABLE;	//Input HSYNC Period Measurement Over-flow Watch-Dog Enable
			//frank@0929 add below code to disable WD to solve mantis30272
			if(Scaler_InputSrcGetMainChFrom() == _SRC_FROM_VDC)
				onms_onms1_watchdog_en_reg.on1_wd_vs_per_over_range= _DISABLE;	//VSYNC_OVER_RANGE Watch-Dog Enable
			else
				SP_MS2IE_register.wde_on2_vs_over_range	= _ENABLE;	//VSYNC_OVER_RANGE Watch-Dog Enable
			SP_MS2IE_register.wde_on2_vs_pol_chg		= _ENABLE;	//Input VSYNC Polarity change Watch-Dog Enable
			SP_MS2IE_register.wde_on2_hs_per_of		= _ENABLE;	//Input HSYNC Period Measurement Over-flow Watch-Dog Enable
			SP_MS2IE_register.wde_on2_hs_over_range	= _DISABLE;	//HSYNC_OVER_RANGE Watch-Dog Enable
			SP_MS2IE_register.wde_on2_hs_pol_chg		= _ENABLE;	//Input HSYNC Polarity change Watch-Dog Enable
			IoReg_Write32(SYNC_PROCESSOR_SP_MS2IE_VADDR, SP_MS2IE_register.regValue);

			mp_layout_force_to_background_reg.regValue =IoReg_Read32(PPOVERLAY_MP_Layout_Force_to_Background_reg);
			mp_layout_force_to_background_reg.sub_wd_to_background	= _ENABLE;	//Sub Watch Dog Enable (Force-to-Background)
			mp_layout_force_to_background_reg.sub_wd_to_free_run		= _DISABLE;	//FrameSync Sub Watch Dog Enable (FreeRun), sub can't enable free run
			IoReg_Write32(PPOVERLAY_MP_Layout_Force_to_Background_reg, mp_layout_force_to_background_reg.regValue);
		}
#endif

		//FrameSync Watch Dog Enable (FreeRun)
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.wde_to_free_run = _ENABLE;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
	}
	else// if(_DISABLE == enable)
	{
		if(_CHANNEL1 == channel)
		{
			onms_onms1_watchdog_en_reg.regValue = IoReg_Read32(ONMS_onms1_watchdog_en_reg);
			onms_onms1_watchdog_en_reg.on1_wd_to_main			= _FALSE;		//Watch Dog select to Main Display
//			SP_MS1IE_register.wd_on1_to_sub			= _FALSE;	//Watch Dog select to Sub Display
			onms_onms1_watchdog_en_reg.on1_wd_vs_per_timeout= _DISABLE;	//No VSYNC occurred
			onms_onms1_watchdog_en_reg.on1_wd_vs_high_timeout= _DISABLE;	//Input VSYNC High Pulse Measurement Time-Out Watch-Dog Enable
			onms_onms1_watchdog_en_reg.on1_wd_vs_per_overflow= _DISABLE;	//Input HSYNC Period Measurement Over-flow Watch-Dog Enable
			onms_onms1_watchdog_en_reg.on1_wd_vs_per_over_range= _DISABLE;	//VSYNC_OVER_RANGE Watch-Dog Enable
			onms_onms1_watchdog_en_reg.on1_wd_vs_pol_chg= _DISABLE;	//Input VSYNC Polarity change Watch-Dog Enable
			onms_onms1_watchdog_en_reg.on1_wd_hs_per_overflow= _DISABLE;	//Input HSYNC Period Measurement Over-flow Watch-Dog Enable
			onms_onms1_watchdog_en_reg.on1_wd_hs_per_over_range= _DISABLE;	//HSYNC_OVER_RANGE Watch-Dog Enable
			onms_onms1_watchdog_en_reg.on1_wd_hs_pol_chg		= _DISABLE;	//Input HSYNC Polarity change Watch-Dog Enable
			IoReg_Write32(ONMS_onms1_watchdog_en_reg, onms_onms1_watchdog_en_reg.regValue);

			main_display_control_rsv_reg.regValue = IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
			main_display_control_rsv_reg.main_wd_to_background	= _DISABLE;	//Main Watch Dog Enable (Force-to-Background)
			main_display_control_rsv_reg.main_wd_to_free_run		= _DISABLE;	//FrameSync Main Watch Dog Enable (FreeRun)
			IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, main_display_control_rsv_reg.regValue);

			//write clear status, if can not clear, need add delay
			onms_onms1_status_reg.regValue = IoReg_Read32(ONMS_onms1_status_reg);
			onms_onms1_status_reg.on1_vs_per_to_long= 1;	//Time-Out (76ms) bit of Input VSYNC Period Measurement (No VSYNC occurred)
			onms_onms1_status_reg.on1_vs_per_timeout= 1; //Time-Out (40ms) bit of Input VSYNC Period Measurement (No VSYNC occurred)
			onms_onms1_status_reg.on1_vs_high_timeout= 1;	//Time-Out bit of Input VSYNC High Pulse Measurement
			onms_onms1_status_reg.on1_vs_per_overflow= 1;	//Over-flow bit of Input HSYNC Period Measurement
			onms_onms1_status_reg.on1_vs_per_over_range= 1; //VSYNC_OVER_RANGE
			onms_onms1_status_reg.on1_vs_pol_chg= 1; //Input VSYNC Polarity change
			onms_onms1_status_reg.on1_hs_per_overflow= 1; //Over-flow bit of Input VSYNC Period Measurement
			onms_onms1_status_reg.on1_hs_per_over_range= 1; //HSYNC_OVER_RANGE
			onms_onms1_status_reg.on1_hs_pol_chg= 1; //Input HSYNC Polarity change
			IoReg_Write32(ONMS_onms1_status_reg, onms_onms1_status_reg.regValue);
		}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		else// if(_CHANNEL2 == channel)
		{
			SP_MS2IE_register.regValue = IoReg_Read32(SYNC_PROCESSOR_SP_MS2IE_VADDR);
			SP_MS2IE_register.wd_on2_to_sub			= _FALSE;		//Watch Dog select to Sub Display
			SP_MS2IE_register.wd_on2_to_sub			= _FALSE;	//Watch Dog select to Sub Display
			SP_MS2IE_register.wde_on2_vs_per_to		= _DISABLE;	//No VSYNC occurred
			SP_MS2IE_register.wde_on2_vs_high_to		= _DISABLE;	//Input VSYNC High Pulse Measurement Time-Out Watch-Dog Enable
			SP_MS2IE_register.wde_on2_vs_per_of		= _DISABLE;	//Input HSYNC Period Measurement Over-flow Watch-Dog Enable
			SP_MS2IE_register.wde_on2_vs_over_range	= _DISABLE;	//VSYNC_OVER_RANGE Watch-Dog Enable
			SP_MS2IE_register.wde_on2_vs_pol_chg		= _DISABLE;	//Input VSYNC Polarity change Watch-Dog Enable
			SP_MS2IE_register.wde_on2_hs_per_of		= _DISABLE;	//Input HSYNC Period Measurement Over-flow Watch-Dog Enable
			SP_MS2IE_register.wde_on2_hs_over_range	= _DISABLE;	//HSYNC_OVER_RANGE Watch-Dog Enable
			SP_MS2IE_register.wde_on2_hs_pol_chg		= _DISABLE;	//Input HSYNC Polarity change Watch-Dog Enable
			IoReg_Write32(SYNC_PROCESSOR_SP_MS2IE_VADDR, SP_MS2IE_register.regValue);

			mp_layout_force_to_background_reg.regValue =IoReg_Read32(PPOVERLAY_MP_Layout_Force_to_Background_reg);
			mp_layout_force_to_background_reg.sub_wd_to_background	= _DISABLE;	//Sub Watch Dog Enable (Force-to-Background)
			mp_layout_force_to_background_reg.sub_wd_to_free_run		= _DISABLE;	//FrameSync Sub Watch Dog Enable (FreeRun), sub can't enable free run
			IoReg_Write32(PPOVERLAY_MP_Layout_Force_to_Background_reg, mp_layout_force_to_background_reg.regValue);

			SP_MS2Stus_reg.regValue = IoReg_Read32(SYNC_PROCESSOR_SP_MS2STUS_VADDR);
	//		RTD_Log(LOGGER_ERROR,"\nSP_MS2Stus_reg = %x\n", SP_MS2Stus_reg.regValue);
			//SP_MS2Stus_reg.on2_vs_per_to_long	= 1;	//Time-Out (76ms) bit of Input VSYNC Period Measurement (No VSYNC occurred)
			SP_MS2Stus_reg.on2_vs_per_to		= 1; //Time-Out (40ms) bit of Input VSYNC Period Measurement (No VSYNC occurred)
			SP_MS2Stus_reg.on2_vs_high_to		= 1;	//Time-Out bit of Input VSYNC High Pulse Measurement
			SP_MS2Stus_reg.on2_vs_per_of		= 1;	//Over-flow bit of Input HSYNC Period Measurement
			SP_MS2Stus_reg.on2_vs_over_range	= 1; //VSYNC_OVER_RANGE
			SP_MS2Stus_reg.on2_vs_pol_chg		= 1; //Input VSYNC Polarity change
			SP_MS2Stus_reg.on2_hs_per_of		= 1; //Over-flow bit of Input VSYNC Period Measurement
			SP_MS2Stus_reg.on2_hs_over_range	= 1; //HSYNC_OVER_RANGE
			SP_MS2Stus_reg.on2_hs_pol_chg		= 1; //Input HSYNC Polarity change
			IoReg_Write32(SYNC_PROCESSOR_SP_MS2STUS_VADDR, SP_MS2Stus_reg.regValue);
		}
#endif

	}
#endif
}


#endif //#if 0

//USER:LewisLee DATE:2012/12/21
//add ADC noLock Watch Dog
//to Prevent measure watch dog response too slow
#ifdef CONFIG_SUPPORT_SRC_ADC
void fw_scaler_adc_nolock_wdg_ctrl(unsigned char display, unsigned char enable)
{

	dds_pll_dds_status_RBUS dds_status_reg;

	//Disable At first
	if (enable == FALSE) {
		dds_status_reg.regValue = IoReg_Read32(DDS_PLL_DDS_status_reg);
		dds_status_reg.wd_to_main = _DISABLE;
		dds_status_reg.wd_to_sub = _DISABLE;
		dds_status_reg.adc_nolock_wd_en = _DISABLE;
		dds_status_reg.adc_nolock_int_en = _DISABLE;
		dds_status_reg.adc_no_lock_st = 1;		// write clear error status
		IoReg_Write32(DDS_PLL_DDS_status_reg, dds_status_reg.regValue);
	} else {
		dds_status_reg.regValue = IoReg_Read32(DDS_PLL_DDS_status_reg);
		dds_status_reg.adc_no_lock_st = 1;		// write clear error status
		IoReg_Write32(DDS_PLL_DDS_status_reg, dds_status_reg.regValue);
		dds_status_reg.regValue = IoReg_Read32(DDS_PLL_DDS_status_reg);
		if (display == SLR_MAIN_DISPLAY) {
			dds_status_reg.wd_to_main = _ENABLE;
		} else {
			dds_status_reg.wd_to_sub = _ENABLE;
		}
		dds_status_reg.adc_nolock_wd_en = _ENABLE;
		IoReg_Write32(DDS_PLL_DDS_status_reg, dds_status_reg.regValue);
	}
}
#endif
//magic pic will force2background if signal is not stable
void fw_scaler_enable_measure_subwdg_magicpic(void)
{
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
//->?(weihao)	IoReg_SetBits(SYNC_PROCESSOR_SP_MS1IE_VADDR,_BIT30);
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_SetBits(PPOVERLAY_MP_Layout_Force_to_Background_reg, _BIT15);//set sub Watch Dog
#endif
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
}

void fw_scaler_disable_measure_subwdg_magicpic(void)
{
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
//->?(weihao)	IoReg_ClearBits(SYNC_PROCESSOR_SP_MS1IE_VADDR,_BIT30);
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_ClearBits(PPOVERLAY_MP_Layout_Force_to_Background_reg, _BIT15);//set sub Watch Dog
#endif
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
}

void fw_scaler_update_mode_para(void)
{
#if 0//Fix error by Will
	rtd_pr_vbe_info("Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)=%x in update mode para\n", Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY));

	//[Code Sync][AlanLi][0980617][1]
	drvif_clock_select_VCO_div(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY));
	//[Code Sync][AlanLi][0980617][1][end]

	Adjust_ADC_Clock(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), FALSE);
#endif
}
#endif

static void _dbg_dumpDispTimingInfo(unsigned char timeMode, unsigned int _90k_counter)
{
	if(drvif_scaler_get_display_mode() != timeMode){
		rtd_pr_vbe_debug("[TM]=%d@%d\n", timeMode, _90k_counter);
	}
	return;
}

/**
 * drvif_scaler_get_display_mode
 * Get current display timing mode.
 * 0:DISPLAY_MODE_FRC, 1: DISPLAY_MODE_FRAME_SYNC
 *
 * @param { void }
 * @return { current display timing mode }
 */
unsigned char drvif_scaler_get_display_mode(void)
{
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;

	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

	if(display_timing_ctrl1_reg.disp_fsync_en)
		return	DISPLAY_MODE_FRAME_SYNC;
	else{
		if(display_timing_ctrl1_reg.disp_fix_last_line_new == 1)
			return DISPLAY_MODE_NEW_FLL_MODE;
		else
			return DISPLAY_MODE_FRC;
	}

}

//USER:LewisLee DATE:2012/08/24
//for some bad Tcon IC, can not accept too long DV total
//so enable all mode at frc2framesync function
void drvif_scaler_set_display_mode_to_framesync_by_hw(void)
{
	//unsigned int delay = 5;
	if(_ENABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_Enable())
	{
		drvif_scaler_set_display_mode_frc2fsync_by_hw(_FALSE);
	}
	else// if(_DISABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_Enable())
	{
		//USER:LewisLee DATE:2012/07/11
		//In Line buffer mode, we don't use HW switch framesync
		//so don't need to wait 5ms
		if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
		{
			drvif_scaler_set_display_mode_frc2fsync_by_hw(_TRUE);
			return;
		}
		else// if(_FALSE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
		{
			drvif_scaler_set_display_mode_frc2fsync_by_hw(_FALSE);
		}
	}

	//delay = Scaler_Get_FreeRun_To_FrameSync_By_HW_Delay();
	//msleep(delay);
/*//Remove by Will. Ben Chenck no need
	if(_ENABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_FastMode())
  	{
		if(_TRUE == framesync_get_enterlastline_at_frc_mode_flg())
			WaitFor_DEN_STOP_Done();
	}
*/
 }
#ifndef BUILD_QUICK_SHOW

//USER:LewisLee DATE:2013/10/08
//for panel short time, keep last line pixel as even
void drvif_scaler_set_Disp_line_4x(UINT8 ucEnable)
{
//	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
//	UINT32 delay = 0;

	if(_ENABLE == ucEnable)
	{
	#if 0
		if(_TRUE == Scaler_Get_Line4x_Mode_At_Framesync_Flag())
		{
			//In Not Fix Last Line Mode, need set 4x, to make sure BOE work normal
//			if(_TRUE == Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))
//			if(_FALSE == framesync_get_enterlastline_at_frc_mode_flg())
			{
				display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

				if(_DISABLE == display_timing_ctrl1_reg.disp_line_4x)
				{
//					WaitFor_DEN_STOP();

//					display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
					display_timing_ctrl1_reg.disp_line_4x = _ENABLE;
					IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);

					delay = Scaler_Get_Line4x_Mode_At_Framesync_Delay_Ms();
					msleep(delay);
					rtd_pr_vbe_info("drvif_scaler_set_Disp_line_4x, Enable\n");
				}
			}
		}
	#endif
	}
	else// if(_DISABLE == ucEnable)
	{
#if 0
		if((_TRUE == Scaler_Get_Line4x_Mode_At_Framesync_Flag()) || (_ENABLE == Scaler_Get_DoubleDVS_Line4x_Mode_At_FRC_To_Framesync_Flag()))
		{
			display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

			if(_ENABLE == display_timing_ctrl1_reg.disp_line_4x)
			{
				delay = Scaler_Get_Line4x_Mode_At_Framesync_Delay_Ms();
				msleep(delay);

//				WaitFor_DEN_STOP();
				display_timing_ctrl1_reg.disp_line_4x = _DISABLE;
				IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
				rtd_pr_vbe_info("drvif_scaler_set_Disp_line_4x, Disable\n");
			}
		}
#endif
	}
}


//#define SLR_DRV_PRINT_ENABLE
#ifdef SLR_DRV_PRINT_ENABLE
  static inline void _dump_timingMode(unsigned char timing_mode, unsigned int line)
  {
  	if((Scaler_InputSrcGetType(SLR_MAIN_DISPLAY) != _SRC_OSD) && (drvif_scaler_get_display_mode() != timing_mode))
  		rtd_pr_vbe_debug("[SLR] DT=%d@%d\n", timing_mode, line);
  }

#else
  #define _dump_timingMode(x,y...)

#endif
#endif

void vbe_disp_set_VRR_set_display_vtotal_60Hz(void){
    ppoverlay_dv_total_RBUS dv_total_reg;
    ppoverlay_uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
    ppoverlay_memcdtg_dv_total_RBUS memcdtg_dv_total_reg;
    ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
    unsigned int vertical_total = Get_DISP_VERTICAL_TOTAL();

#if 0
	if(Get_DISPLAY_REFRESH_RATE() == 144)
		vertical_total = 2186;
	else if(Get_DISPLAY_REFRESH_RATE() == 240) {
		vertical_total = Get_DISP_VERTICAL_TOTAL();
	}
	else if(Get_DISPLAY_REFRESH_RATE() == 165) {
		vertical_total = 1480;
	}
#endif

    /*setup dtg master.*/
    dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
    dv_total_reg.dv_total= vertical_total-1;
    IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);

    uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
    uzudtg_dv_total_reg.uzudtg_dv_total = vertical_total-1;
    IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);

    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
    ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

    memcdtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
    memcdtg_dv_total_reg.memcdtg_dv_total = vertical_total-1;
    IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.regValue);

    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
    ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set = 1;
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
    rtd_pr_vbe_notice("[VRR] reset vtotal(%d) after timing sync",vertical_total);
}

#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
/**
 * drvif_scaler_set_display_mode
 * Set current display timing mode.
 * 0:DISPLAY_MODE_FRC, 1: DISPLAY_MODE_FRAME_SYNC
 *
 * @param { timing_mode: given timing mode }
 * @return { void }
 */
extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr;
#ifndef BUILD_QUICK_SHOW
/*Note: 20230307
This API not be used for QS,please use void drvif_scaler_set_display_mode_QS(UINT8 timing_mode)
*/
void drvif_scaler_set_display_mode(UINT8 timing_mode)
{
		ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
		ppoverlay_dh_total_last_line_length_RBUS dh_total_last_line_length_reg;
		pll27x_reg_sys_dclkss_RBUS sys_dclkss_reg;
		ppoverlay_fix_last_line_mode_lock_status_RBUS ppoverlay_fix_last_line_mode_lock_status_reg;
		//UINT8 ucfix_last_line_flag = _FALSE, uc_vo_frc_on_fsync_flag = _FALSE;
		pll27x_reg_dpll_in_fsync_tracking_ctrl_RBUS dpll_in_fsync_tracking_ctrl_reg;
		vgip_data_path_select_RBUS data_path_select_reg;
		unsigned int before_time;
		unsigned int aftertime;
		//USER:LewisLee DATE:2010/09/06
		//I think it need modify again
		if(drvif_scaler_get_display_mode() == timing_mode)
			return;
#ifndef BUILD_QUICK_SHOW
		// [SG] Display timing force free run in SG 3D data FRC mode
		if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && !Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()){
			if(drvif_scaelr3d_decide_is_3D_display_mode() && (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == FALSE))
				timing_mode = DISPLAY_MODE_FRC;
		}
#endif
#if 0  //mark by qing_liu
		if(_TRUE == Scaler_Keep_FRC_Mode())
		{
			//rtd_pr_vbe_info("%s\n", __FUNCTION__);
			timing_mode = DISPLAY_MODE_FRC;
		}
#endif
		// [PR to SG] for PR -> SG 3D TCON, 3D need timing free run
		if(Get_PANEL_3D_PR_TO_SG_OUTPUT_FRC_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode())
			timing_mode = DISPLAY_MODE_FRC;

		_dbg_dumpDispTimingInfo(timing_mode, IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg));

                display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
                sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);

		if(DISPLAY_MODE_FRAME_SYNC == timing_mode)
		{
			// [RL6856-1737][ECN] abnormal long/short enable before doing timing sync
            if(VODMA_VODMA_PVS0_Gen_get_iv_src_sel(IoReg_Read32(VODMA_VODMA_PVS0_Gen_reg)) != IV_SRC_SEL_MANUALVS)
                drvif_scaler_enable_abnormal_dvs_protect(1, 1);

			drvif_scaler_wclr_abnormal_dvs_long_short();
            if(_ENABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_Enable())
			{
				if(_DISABLE == display_timing_ctrl1_reg.disp_fsync_en)
				{

					IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
					dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
					dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 0;
					IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
					sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
					sys_dclkss_reg.dclk_ss_en = 1;
					sys_dclkss_reg.dclk_ss_load= 1;
					IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);

					data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);
					//[ECN TEST]Sub path set timing fsync, need set fsync channel to sub
					if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE) && (Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == SLR_SUB_DISPLAY)){
						data_path_select_reg.fsync_ch_sel = 1;	//sync sub i-domain path
					}
					else{
						data_path_select_reg.fsync_ch_sel = 0;	//sync main i-domain path
					}
					IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);

					drvif_scaler_set_display_mode_to_framesync_by_hw();

		                    if(Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2() ||
								Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3() ||
								Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL4() ||
								Scaler_Get_FreeRun_To_FrameSync_By_I2D_CTRL() ||
								Scaler_Get_FreeRun_To_FrameSync_By_Reset_VODMA()){
		                    }else{
								display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
								fw_scaler_dtg_double_buffer_enable(0);
								IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
		                    }
					before_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;
					drvif_scaler_set_display_mode_frc2fsync_check_lock();
					aftertime = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;
					rtd_pr_vbe_notice("\ndrvif_scaler_set_display_mode_frc2fsync_check_lock cost time=%d ms\n",(aftertime -before_time));

					//VRR set dvtotal 60Hz after timing sync to protect panel broken when wdg trigger freerun timing
					if((get_vsc_src_is_hdmi_or_dp()) && (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())){
						vbe_disp_set_VRR_set_display_vtotal_60Hz();
					}
				}
				else// if(_ENABLE == display_timing_ctrl1_reg.disp_fsync_en)
				{
					display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
					fw_scaler_dtg_double_buffer_enable(1);
					IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
					fw_scaler_dtg_double_buffer_apply();
					//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
				}
			}
			else// if(_DISABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_Enable())
			{
				display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
				IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
			}
#ifdef CONFIG_ENABLE_3D_FRAME_PACKING_ID_DOMAIN_NEW_RELATION
			if(_ENABLE == Scaler_Get_3D_FramePacking_Mode_Run_Framesync_Enable())
			{
				Scaler_3D_FramePacking_Mode_Run_Framesync_Action(_TRUE);
			}
#endif //#ifdef CONFIG_ENABLE_3D_FRAME_PACKING_ID_DOMAIN_NEW_RELATION
			//Set BFI flag on
			if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 495) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 605))
				fwif_color_set_BFI_En_by_InputFrameRate(_ENABLE);
		}
		else if(DISPLAY_MODE_NEW_FLL_MODE == timing_mode){
			//IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT2);
			display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
			display_timing_ctrl1_reg.disp_fix_last_line_new = 1;
			display_timing_ctrl1_reg.disp_fsync_en = _DISABLE;
			fw_scaler_dtg_double_buffer_enable(1);
			IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
			fw_scaler_dtg_double_buffer_apply();
			//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
			IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
			dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
			dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 1;
			IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
			sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
			sys_dclkss_reg.dclk_ss_en = 1;
			sys_dclkss_reg.dclk_ss_load= 1;
			IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);

			//write clear fll status @Crixus 20160909
			ppoverlay_fix_last_line_mode_lock_status_reg.regValue = IoReg_Read32(PPOVERLAY_fix_last_line_mode_lock_status_reg);
			//ppoverlay_fix_last_line_mode_lock_status_reg.lock_th = 0xafc;//modify the fll threshold, ((xclk / 12) *n%)<<4
			ppoverlay_fix_last_line_mode_lock_status_reg.t_s = 1;//reset fll lock status
			IoReg_Write32(PPOVERLAY_fix_last_line_mode_lock_status_reg, ppoverlay_fix_last_line_mode_lock_status_reg.regValue);

			//Set BFI flag on
			if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 495) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 605))
				fwif_color_set_BFI_En_by_InputFrameRate(_ENABLE);
		}
		else// if(DISPLAY_MODE_FRC== timing_mode)
		{
			drivf_scaler_reset_freerun();
		}

		// Free run mode, last line should set same as dhtotal
		if(timing_mode == DISPLAY_MODE_FRC){
			dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
			if(display_timing_ctrl1_reg.disp_fsync_en == 0){
				dh_total_last_line_length_reg.dh_total_last_line = dh_total_last_line_length_reg.dh_total;
				fw_scaler_dtg_double_buffer_enable(1);
				IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, dh_total_last_line_length_reg.regValue);
				fw_scaler_dtg_double_buffer_apply();
				//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
			}
		}
}
#endif
void drvif_scaler_set_display_modeByDisp(unsigned char timing_mode,unsigned char _channel)
{
	SCALER_DISP_CHANNEL channel = (SCALER_DISP_CHANNEL)_channel;
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	ppoverlay_dh_total_last_line_length_RBUS dh_total_last_line_length_reg;
	pll27x_reg_sys_dclkss_RBUS sys_dclkss_reg;
	//UINT8 ucfix_last_line_flag = _FALSE, uc_vo_frc_on_fsync_flag = _FALSE;
	pll27x_reg_dpll_in_fsync_tracking_ctrl_RBUS dpll_in_fsync_tracking_ctrl_reg;
	unsigned int before_time;
	unsigned int aftertime;
	//USER:LewisLee DATE:2010/09/06
	//I think it need modify again
	if(drvif_scaler_get_display_mode() == timing_mode)
		return;

	_dbg_dumpDispTimingInfo(timing_mode, IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg));
#ifndef BUILD_QUICK_SHOW
	// [SG] Display timing force free run in SG 3D data FRC mode
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && !Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()){
		if(drvif_scaelr3d_decide_is_3D_display_mode() && (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAMESYNC) == FALSE))
			timing_mode = DISPLAY_MODE_FRC;
	}
#endif
#if 0  //mark by qing_liu
	if(_TRUE == Scaler_Keep_FRC_Mode())
	{
		//rtd_pr_vbe_info("%s\n", __FUNCTION__);
		timing_mode = DISPLAY_MODE_FRC;
	}
#endif
#ifndef BUILD_QUICK_SHOW
	// [PR to SG] for PR -> SG 3D TCON, 3D need timing free run
	if(Get_PANEL_3D_PR_TO_SG_OUTPUT_FRC_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode())
		timing_mode = DISPLAY_MODE_FRC;
#endif
	_dbg_dumpDispTimingInfo(timing_mode, __LINE__);
	/*WaitFor_DEN_STOP();*/

        display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
        sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);

	if(DISPLAY_MODE_FRAME_SYNC == timing_mode)
	{
		if(_ENABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_Enable())
		{
			if(_DISABLE == display_timing_ctrl1_reg.disp_fsync_en)
			{

				IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
				dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
				dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 0;
				IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
				sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
				sys_dclkss_reg.dclk_ss_en = 1;
				sys_dclkss_reg.dclk_ss_load= 1;
				IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);

				drvif_scaler_set_display_mode_to_framesync_by_hw();
				if(Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2() ||
					Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3() ||
					Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL4()){
					before_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;
					drvif_scaler_set_display_mode_frc2fsync_check_lock();
					aftertime = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;
					rtd_pr_vbe_notice("\ndrvif_scaler_set_display_mode_frc2fsync_check_lock cost time=%d ms\n",(aftertime -before_time));
                		}else{
					display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
					fw_scaler_dtg_double_buffer_enable(0);
					IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
                		}
			}
		}
		else// if(_DISABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_Enable())
		{
			display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
			IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
		}
#ifdef CONFIG_ENABLE_3D_FRAME_PACKING_ID_DOMAIN_NEW_RELATION
		if(_ENABLE == Scaler_Get_3D_FramePacking_Mode_Run_Framesync_Enable())
		{
			Scaler_3D_FramePacking_Mode_Run_Framesync_Action(_TRUE);
		}
#endif //#ifdef CONFIG_ENABLE_3D_FRAME_PACKING_ID_DOMAIN_NEW_RELATION
	}
	else if(DISPLAY_MODE_NEW_FLL_MODE == timing_mode){
		//IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT2);
		display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
		display_timing_ctrl1_reg.disp_fix_last_line_new = 1;
		display_timing_ctrl1_reg.disp_fsync_en = _DISABLE;
		fw_scaler_dtg_double_buffer_enable(1);
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
		fw_scaler_dtg_double_buffer_apply();
		//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
		dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
		dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 1;
		IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
		sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
		sys_dclkss_reg.dclk_ss_en = 1;
		sys_dclkss_reg.dclk_ss_load= 1;
		IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
	}
	else// if(DISPLAY_MODE_FRC== timing_mode)
	{
#if 0
		IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
		dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
		dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 0;
		IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
		sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
		sys_dclkss_reg.dclk_ss_en = 1;
		sys_dclkss_reg.dclk_ss_load= 1;
		IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
		IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT2);
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
		display_timing_ctrl1_reg.disp_fix_last_line = _DISABLE;
		display_timing_ctrl1_reg.disp_fsync_en = _DISABLE;
		fw_scaler_dtg_double_buffer_enable(1);
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
		fw_scaler_dtg_double_buffer_apply();
#endif
		drivf_scaler_reset_freerun();
		//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		/*IoReg_SetBits(PPOVERLAY_DOUBLE_BUFFER_CTRL_VADDR, _BIT0);*/
		//USER:LewisLee DATE:2011/06/16 mask
		//only framesync or free run need to reset DV total
		//		drvif_framesync_reset_dv_total();
	}

	// Free run mode, last line should set same as dhtotal
	if(timing_mode == DISPLAY_MODE_FRC){
		dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
		if(display_timing_ctrl1_reg.disp_fsync_en == 0){
			dh_total_last_line_length_reg.dh_total_last_line = dh_total_last_line_length_reg.dh_total;
			fw_scaler_dtg_double_buffer_enable(1);
			IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, dh_total_last_line_length_reg.regValue);
			fw_scaler_dtg_double_buffer_apply();
			//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		}
	}
}

#else //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
/**
 * drvif_scaler_set_display_mode
 * Set current display timing mode.
 * 0:DISPLAY_MODE_FRC, 1: DISPLAY_MODE_FRAME_SYNC
 *
 * @param { timing_mode: given timing mode }
 * @return { void }
 */
void drvif_scaler_set_display_mode(unsigned char timing_mode)
{
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	ppoverlay_dh_total_last_line_length_RBUS dh_total_last_line_length_reg;
	pll27x_reg_dpll_in_fsync_tracking_ctrl_RBUS dpll_in_fsync_tracking_ctrl_reg;
	pll27x_reg_sys_dclkss_RBUS sys_dclkss_reg;
	unsigned long flags;//for spin_lock_irqsave

	//USER:LewisLee DATE:2010/09/06
	//I think it need modify again
	//if(drvif_scaler_get_display_mode() == timing_mode)
		//return;

	//if(Scaler_Get_Keep_In_BootUp_Mode_flag()==TRUE)
		//return;

	_dbg_dumpDispTimingInfo(timing_mode, __LINE__);

	// [SG] Display timing force free run in SG 3D data FRC mode
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && !Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()){
		if(drvif_scaelr3d_decide_is_3D_display_mode() && (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == FALSE))
			timing_mode = DISPLAY_MODE_FRC;
	}
#if 0
	if((Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI) && !Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)){
		timing_mode = DISPLAY_MODE_FRC;
	}
#endif
	else if(Get_DISPLAY_PANEL_TYPE() == P_DISP_TO_TVOX){
		timing_mode = DISPLAY_MODE_FRC;
	}
	_dbg_dumpDispTimingInfo(timing_mode, __LINE__);
#if 0
	if(_TRUE == Scaler_Keep_FRC_Mode())
	{
		//rtd_pr_vbe_info("%s\n", __FUNCTION__);
		timing_mode = DISPLAY_MODE_FRC;
	}
#endif
	/*WaitFor_DEN_STOP();*/

	_dbg_dumpDispTimingInfo(timing_mode, __LINE__);

	if(DISPLAY_MODE_FRAME_SYNC == timing_mode)
	{
		if(_TRUE == framesync_get_enterlastline_at_frc_mode_flg())
		{
			display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
#if 0
			if(_TRUE == Scaler_Get_Keep_Frc_Tracking_Framesync())
			{
				if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY,SLR_INPUT_FRAMESYNC))
					display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
				else
					display_timing_ctrl1_reg.disp_frc_on_fsync = _TRUE;
			}
			else
#endif
			{
				display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
			}

			        display_timing_ctrl1_reg.disp_fix_last_line_new = _ENABLE;
				display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;

				if (Get_PANEL_TYPE_IS_MILLET3_4K2K_SERIES() ||
					(Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI) ||
					(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) ||
					(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)) {
					display_timing_ctrl1_reg.disp_line_4x = 1;
					//display_timing_ctrl1_reg.disp_fix_last_line =1;
				}
			fw_scaler_dtg_double_buffer_enable(1);
				IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
			fw_scaler_dtg_double_buffer_apply();
			//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		}
		else// if(_TRUE == framesync_get_enterlastline_at_frc_mode_flg())
		{
			display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
#if 0
			if(_TRUE == Scaler_Get_Keep_Frc_Tracking_Framesync())
			{
				if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY,SLR_INPUT_FRAMESYNC))
					display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
				else
					display_timing_ctrl1_reg.disp_frc_on_fsync = _TRUE;
			}
			else
#endif
			IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
			dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
			dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 0;
			IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
			sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
			sys_dclkss_reg.dclk_ss_en = 1;
			sys_dclkss_reg.dclk_ss_load= 1;
			IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);

			{
				display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
			}
			display_timing_ctrl1_reg.disp_fix_last_line_new = _DISABLE;
			display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;

			if (Get_PANEL_TYPE_IS_MILLET3_4K2K_SERIES() ||
				(Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI) ||
				(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) ||
				(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)){
					display_timing_ctrl1_reg.disp_line_4x = 1;
//					display_timing_ctrl1_reg.disp_fix_last_line =0;

			}
			fw_scaler_dtg_double_buffer_enable(1);
			IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
			fw_scaler_dtg_double_buffer_apply();
			//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		}
	}
	else if(DISPLAY_MODE_NEW_FLL_MODE == timing_mode){
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT2);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
		display_timing_ctrl1_reg.disp_fix_last_line_new = 2;
		display_timing_ctrl1_reg.disp_fsync_en = _DISABLE;
		fw_scaler_dtg_double_buffer_enable(1);
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
		fw_scaler_dtg_double_buffer_apply();
		//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
		dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
		dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 1;
		IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
		sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
		sys_dclkss_reg.dclk_ss_en = 1;
		sys_dclkss_reg.dclk_ss_load= 1;
		IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
	}
	else// if(DISPLAY_MODE_FRC== timing_mode)
	{
		IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
		dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
		dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 0;
		IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
		if((Get_DISPLAY_PANEL_TYPE() != P_VBY1_TO_HDMI) && (Get_DISPLAY_PANEL_TYPE() != P_VBY1_TO_LVDS_TO_HDMI)){
			sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
			sys_dclkss_reg.dclk_ss_en = 1;
			sys_dclkss_reg.dclk_ss_load= 1;
			IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
		}
		else{
			sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
			sys_dclkss_reg.dclk_ss_en = 0;
			sys_dclkss_reg.dclk_ss_load= 0;
			IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
		}
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT2);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
		display_timing_ctrl1_reg.disp_fix_last_line_new = _DISABLE;
		display_timing_ctrl1_reg.disp_fsync_en = _DISABLE;
		fw_scaler_dtg_double_buffer_enable(1);
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
		fw_scaler_dtg_double_buffer_apply();
		//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		/*IoReg_SetBits(PPOVERLAY_DOUBLE_BUFFER_CTRL_VADDR, _BIT0);*/
//USER:LewisLee DATE:2011/06/16 mask
//only framesync or free run need to reset DV total
//		drvif_framesync_reset_dv_total();
	}

	// Free run mode, last line should set same as dhtotal
	if(timing_mode == DISPLAY_MODE_FRC){
		dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
		if(display_timing_ctrl1_reg.disp_fsync_en == 0){
			dh_total_last_line_length_reg.dh_total_last_line = dh_total_last_line_length_reg.dh_total;
			fw_scaler_dtg_double_buffer_enable(1);
			IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, dh_total_last_line_length_reg.regValue);
			fw_scaler_dtg_double_buffer_apply();
			//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		}
	}
}

void drvif_scaler_set_display_modeByDisp(unsigned char timing_mode,unsigned char _channel)
{
	SCALER_DISP_CHANNEL channel = (SCALER_DISP_CHANNEL)_channel;
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	ppoverlay_dh_total_last_line_length_RBUS dh_total_last_line_length_reg;
	pll27x_reg_dpll_in_fsync_tracking_ctrl_RBUS dpll_in_fsync_tracking_ctrl_reg;
	pll27x_reg_sys_dclkss_RBUS sys_dclkss_reg;
	unsigned long flags;//for spin_lock_irqsave

	//USER:LewisLee DATE:2010/09/06
	//I think it need modify again
	//if(drvif_scaler_get_display_mode() == timing_mode)
	//return;

	//if(Scaler_Get_Keep_In_BootUp_Mode_flag()==TRUE)
	//return;

	_dbg_dumpDispTimingInfo(timing_mode, __LINE__);

	// [SG] Display timing force free run in SG 3D data FRC mode
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && !Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()){
		if(drvif_scaelr3d_decide_is_3D_display_mode() && (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAMESYNC) == FALSE))
			timing_mode = DISPLAY_MODE_FRC;
	}
#if 0
	if((Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI) && !Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAMESYNC)){
		timing_mode = DISPLAY_MODE_FRC;
	}
#endif
	else if(Get_DISPLAY_PANEL_TYPE() == P_DISP_TO_TVOX){
		timing_mode = DISPLAY_MODE_FRC;
	}
	_dbg_dumpDispTimingInfo(timing_mode, __LINE__);
#if 0
	if(_TRUE == Scaler_Keep_FRC_Mode())
	{
		//rtd_pr_vbe_info("%s\n", __FUNCTION__);
		timing_mode = DISPLAY_MODE_FRC;
	}
#endif
	/*WaitFor_DEN_STOP();*/

	_dbg_dumpDispTimingInfo(timing_mode, __LINE__);

	if(DISPLAY_MODE_FRAME_SYNC == timing_mode)
	{
		if(_TRUE == framesync_get_enterlastline_at_frc_mode_flg())
		{
			display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
#if 0
			if(_TRUE == Scaler_Get_Keep_Frc_Tracking_Framesync())
			{
				if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY,SLR_INPUT_FRAMESYNC))
					display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
				else
					display_timing_ctrl1_reg.disp_frc_on_fsync = _TRUE;
			}
			else
#endif
			{
				display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
			}

			display_timing_ctrl1_reg.disp_fix_last_line_new = _ENABLE;
			display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;

			if (Get_PANEL_TYPE_IS_MILLET3_4K2K_SERIES() ||
				(Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI) ||
				(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) ||
				(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)){
				display_timing_ctrl1_reg.disp_line_4x = 1;
				//display_timing_ctrl1_reg.disp_fix_last_line =1;
			}
			fw_scaler_dtg_double_buffer_enable(1);
			IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
			fw_scaler_dtg_double_buffer_apply();
			//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		}
		else// if(_TRUE == framesync_get_enterlastline_at_frc_mode_flg())
		{
			display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
#if 0
			if(_TRUE == Scaler_Get_Keep_Frc_Tracking_Framesync())
			{
				if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY,SLR_INPUT_FRAMESYNC))
					display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
				else
					display_timing_ctrl1_reg.disp_frc_on_fsync = _TRUE;
			}
			else
#endif
				IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
			dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
			dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 0;
			IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
			sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
			sys_dclkss_reg.dclk_ss_en = 1;
			sys_dclkss_reg.dclk_ss_load= 1;
			IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);

			{
				display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
			}
			display_timing_ctrl1_reg.disp_fix_last_line_new = _DISABLE;
			display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;

			if (Get_PANEL_TYPE_IS_MILLET3_4K2K_SERIES() ||
				(Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI) ||
				(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) ||
				(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)){
				display_timing_ctrl1_reg.disp_line_4x = 1;
//					display_timing_ctrl1_reg.disp_fix_last_line =0;

			}
			fw_scaler_dtg_double_buffer_enable(1);
			IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
			fw_scaler_dtg_double_buffer_apply();
			//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		}
	}
	else if(DISPLAY_MODE_NEW_FLL_MODE == timing_mode){
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT2);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
		display_timing_ctrl1_reg.disp_fix_last_line_new = 2;
		display_timing_ctrl1_reg.disp_fsync_en = _DISABLE;
		fw_scaler_dtg_double_buffer_enable(1);
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
		fw_scaler_dtg_double_buffer_apply();
		//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
		dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
		dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 1;
		IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
		sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
		sys_dclkss_reg.dclk_ss_en = 1;
		sys_dclkss_reg.dclk_ss_load= 1;
		IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
	}
	else// if(DISPLAY_MODE_FRC== timing_mode)
	{
		IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
		dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
		dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 0;
		IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
		if((Get_DISPLAY_PANEL_TYPE() != P_VBY1_TO_HDMI) && (Get_DISPLAY_PANEL_TYPE() != P_VBY1_TO_LVDS_TO_HDMI)){
			sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
			sys_dclkss_reg.dclk_ss_en = 1;
			sys_dclkss_reg.dclk_ss_load= 1;
			IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
		}
		else{
			sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
			sys_dclkss_reg.dclk_ss_en = 0;
			sys_dclkss_reg.dclk_ss_load= 0;
			IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
		}
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT2);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
		display_timing_ctrl1_reg.disp_fix_last_line_new = _DISABLE;
		display_timing_ctrl1_reg.disp_fsync_en = _DISABLE;
		fw_scaler_dtg_double_buffer_enable(1);
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
		fw_scaler_dtg_double_buffer_apply();
		//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		/*IoReg_SetBits(PPOVERLAY_DOUBLE_BUFFER_CTRL_VADDR, _BIT0);*/
//USER:LewisLee DATE:2011/06/16 mask
//only framesync or free run need to reset DV total
//		drvif_framesync_reset_dv_total();
	}

	// Free run mode, last line should set same as dhtotal
	if(timing_mode == DISPLAY_MODE_FRC){
		dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
		if(display_timing_ctrl1_reg.disp_fsync_en == 0){
			dh_total_last_line_length_reg.dh_total_last_line = dh_total_last_line_length_reg.dh_total;
			fw_scaler_dtg_double_buffer_enable(1);
			IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, dh_total_last_line_length_reg.regValue);
			fw_scaler_dtg_double_buffer_apply();
			//frank@0903 reduce time fw_scaler_dtg_double_buffer_enable(0);
		}
	}
}
#endif //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW

/**
 * drvif_scaler_set_display_mode_frc2fsync_by_hw
 * Enable / Disable Display Timing from Free run to Frame sync by hw
 * 0:Disable, 1: Enable
 *
 * @param { void }
 * @return { void }
 */
void drvif_scaler_set_display_mode_frc2fsync_by_hw(UINT8 ucReset)
{
#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;

	if(_TRUE == ucReset)
	{
		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
		display_timing_ctrl2_reg.frc2fsync_en = _DISABLE;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
		return;
	}

	if(_ENABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_Enable())
	{
		//Mantis bug number = [common] 0020726
		//How : It flashes, when a "photo" is reproduced.
		//Why : when use line buffer, it can not be wait HW set framesync
		//What : we need to force set framesync

		//Mantis bug number = [common] 0020710
		//How : Screen flash all the time when play a HD movie in Media Player
		//Why : when use line buffer, it can not be wait HW set framesync
		//What : we need to force set framesync

		//Mantis bug number = [common] 0020703
		//How : HD movie file is unstable after playback
		//Why : when use line buffer, it can not be wait HW set framesync
		//What : we need to force set framesync

		if(_ENABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_Enable())
		{
			display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
			display_timing_ctrl2_reg.frc2fsync_en = _ENABLE;
			IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
		}
		else// if(_DISABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_Enable())
		{
			//USER:LewisLee DATE:2012/07/09
			//In framesync mode, don't apply HW set framesync
			if(_TRUE == Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))
			{
				display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
				display_timing_ctrl2_reg.frc2fsync_en = _DISABLE;
				IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
			}
			else// if(_FALSE == Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))
			{
				display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
				display_timing_ctrl2_reg.frc2fsync_en = _ENABLE;
				IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
			}
		}
	}
	else// if(_DISABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_Enable())
				{
		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
		display_timing_ctrl2_reg.frc2fsync_en = _DISABLE;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
	}
#endif //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
}

void drvif_scaler_set_display_mode_frc2fsync_check_lock(void)
{
#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
	if(_ENABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp())
	{
		//fwif_color_safe_od_enable(0);
		drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup();
		//fwif_color_safe_od_enable(1);
	}
	else// if(_DISABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp())
	{
		drvif_scaler_set_display_mode_frc2fsync_by_hw_wait_lock();
	}
#endif //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
}

//USER:LewisLee DATE:2012/11/27
//if we don't enable speed up function
//we remain need to wait lock
void drvif_scaler_set_display_mode_frc2fsync_by_hw_wait_lock(void)
{
#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	//UINT32 OriginalMcodeValue = 0;
	UINT32 TimeOutCounterThreshold = 0;
	UINT32 TimeOutCounter = 0;

	if(_ENABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp())
		return;

    /*
	if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
		TimeOutCounterThreshold = 50; //Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_WaitLock_TimeOut();
	else// if(_FALSE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
		TimeOutCounterThreshold = 50; //Scaler_Get_FreeRun_To_FrameSync_By_HW_WaitLock_TimeOut();
    */
    TimeOutCounterThreshold = 50;
	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

	while(_FALSE == display_timing_ctrl1_reg.disp_frc_fsync)
	{
		msleep(1);
		TimeOutCounter++;

		if(TimeOutCounter > TimeOutCounterThreshold)
		{
			rtd_pr_vbe_debug("WL wait timeout......\n");
			break;
		}

		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	}

	rtd_pr_vbe_debug("WL time count = %d\n",TimeOutCounter);

	/*WaitFor_DEN_STOP();*/

	if(TimeOutCounter > TimeOutCounterThreshold)
	{
		if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
		{
			drvif_scaler_set_display_mode_frc2fsync_by_hw(_TRUE);
			rtd_pr_vbe_debug("WL LineBufferMode HW set frc2framesync fail, disable it\n");
		}
	}

	return;
#endif //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
}

void drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_by_DVtotal(void)
{
	extern unsigned judge_scaler_break_case(unsigned char display);
	unsigned long flags;//for spin_lock_irqsave
	unsigned display;
	//ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	//pll27x_reg_dclk_frc2fsync_speedup_RBUS dclk_frc2fsync_speedup_reg;
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	char db_en_ori = 0;
	UINT32 TimeOutCounterThreshold = Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut();
	UINT32 TimeOutCounter = 0;

	ppoverlay_dv_total_RBUS dv_total_reg;
	UINT32 OriginalDVtotalValue = 0;
	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	OriginalDVtotalValue = dv_total_reg.dv_total;

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	db_en_ori = double_buffer_ctrl_reg.dreg_dbuf_en;
	double_buffer_ctrl_reg.dreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	//increase porch, vaule by test
	dv_total_reg.dv_total = 0xb76;
	rtd_pr_vbe_debug("[%s] speed up by vtotal : %x \n", __FUNCTION__, dv_total_reg.dv_total);
	WaitFor_DEN_START();
	IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue-1);

	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);

	while(_FALSE == display_timing_ctrl1_reg.disp_frc_fsync)
	{
		if(judge_scaler_break_case(display))
		{
			rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
			break;
		}
		mdelay(1);
		TimeOutCounter++;

		if(TimeOutCounter > TimeOutCounterThreshold)
		{
			rtd_pr_vbe_notice("SU wait timeout......\n");
			if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
			{
				drvif_scaler_set_display_mode_frc2fsync_by_hw(_TRUE);
				rtd_pr_vbe_notice("SU LineBufferMode HW set frc2framesync fail, disable it\n");
			}
			break;
		}

		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	}
	rtd_pr_vbe_debug("SU time count2= %d\n",TimeOutCounter);


	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	dv_total_reg.dv_total = OriginalDVtotalValue;
	WaitFor_DEN_START();
	IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
	rtd_pr_vbe_debug("[%s] recovery vtotal : %x \n", __FUNCTION__, dv_total_reg.dv_total);
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	db_en_ori = double_buffer_ctrl_reg.dreg_dbuf_en;
	double_buffer_ctrl_reg.dreg_dbuf_en = db_en_ori;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	return;
}

extern unsigned int scaler_disp_smooth_get_memc_protect_vtotal(void);

#define INCREASE_VTOTAL_60HZ 0xa8c
#define INCREASE_VTOTAL_50HZ 0xafc
#define INCREASE_VTOTAL_24HZ 0xB38
#define INCREASE_FLL_VTOTAL_60HZ 0x917
#define INCREASE_FLL_VTOTAL_50HZ 0xAFC
#define INCREASE_FLL_VTOTAL_48HZ 0x917
#define INCREASE_FLL_VTOTAL_48HZ_2 0xB76
//dynamic calculate
#define INCREASE_FLL_VFREQ_60HZ 58
#define INCREASE_FLL_VFREQ_50HZ 48
#define INCREASE_FLL_VFREQ_48HZ 58
#define INCREASE_FLL_VFREQ_48HZ_2 46
#define DEN_POSITION_PROTECT_PORCH 36
#define OLED_PANEL_DEN_PROTECT_PORCH 39
#define PANEL_DEN_PROTECT_PORCH 20
#define FRC2FS_MEMCDEN_PROTECT_PORCH 0x86
#define FRC2FS_I2D_TARGET 150
#define FRC2FSYNC_VRR_SPECIAL_FREQ 480
#define FRC2FSYNC_VRR_MIN_SUPPORT_FREQ 400
#define FRC2FSYNC_VRR_SPEEDUP_OFFSET 20
static unsigned int frc2fsync_original_vtotal = 0;

unsigned int drvif_scaler_get_speedUp_vtotal(unsigned int inVFreq)
{
	unsigned int speedUp_Vtotal = 2700;
	unsigned int speedUp_VFreq  = 50;


    if((inVFreq>245)&&(inVFreq<=255)){ // 25hz
		speedUp_VFreq = 48;
    }else if((inVFreq>495)&&(inVFreq<=505)){
		speedUp_VFreq = 48;
	}else if((inVFreq>295)&&(inVFreq<=305)){
		speedUp_VFreq = 48;
    }else{
		speedUp_VFreq = 50;
    }
	speedUp_Vtotal = Get_DISPLAY_CLOCK_TYPICAL() / (Get_DISP_HORIZONTAL_TOTAL()*speedUp_VFreq);

	return speedUp_Vtotal;
}
void drvif_scaler_set_frc2fsync_recovery_vtotal(unsigned int u32_vtotal)
{
	frc2fsync_original_vtotal = u32_vtotal;
}

unsigned int drvif_scaler_get_frc2fsync_recovery_vtotal(void)
{
	return frc2fsync_original_vtotal;
}

void drvif_scaler_set_frc2fsync_vtotal_protect_on(void)
{
	unsigned long flags;//for spin_lock_irqsave
    unsigned int count=20;
	unsigned int timeout = 0;
	ppoverlay_dv_total_RBUS dv_total_reg;
	ppoverlay_uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
    ppoverlay_memcdtg_dv_total_RBUS memcdtg_dv_total_reg;
	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	unsigned int speedUp_vtotal = 0;
	unsigned int vFreq=0;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS   dispd_stage1_sm_ctrl_reg;
    ppoverlay_dispd_smooth_toggle1_RBUS   dispd_smooth_toggle1_reg;

	ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;

	vFreq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
/*
    if((vFreq>245)&&(vFreq<=255)){ // 25hz
        speedUp_vtotal = INCREASE_VTOTAL_50HZ;
    }else if((vFreq>495)&&(vFreq<=505)){
        speedUp_vtotal = INCREASE_VTOTAL_50HZ;
    }else{
        speedUp_vtotal = INCREASE_VTOTAL_60HZ;
    }
*/
	speedUp_vtotal = drvif_scaler_get_speedUp_vtotal(vFreq);

    timeout = 0xa0000; //100ms, 5 frame.

    dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
    dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
    IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);

    dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
    dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
    IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
    do{
        dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
    }while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply==1) && (--timeout));
	if(timeout==0)
		rtd_pr_vbe_notice("[%s] wait timeout, line=%d]n", __FUNCTION__, __LINE__);

    down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);

    ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 1;      //D7
    ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_en = 1;  //D0
    ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 1; //D5
    ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;  //D4
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
    dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
    dv_total_reg.dv_total=speedUp_vtotal;
    IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
    uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
	if(speedUp_vtotal == INCREASE_VTOTAL_60HZ)
    	uzudtg_dv_total_reg.uzudtg_dv_total = 0xa26; // (0xa8c*4+0x8c4)/5 = 0xa30, -10line for buffer
    else
    	uzudtg_dv_total_reg.uzudtg_dv_total = speedUp_vtotal; //speedUp_vtotal;
    IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);
    memcdtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
    memcdtg_dv_total_reg.memcdtg_dv_total = speedUp_vtotal;
    IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.regValue);


    dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
    dv_den_start_end_reg.dv_den_sta =  Get_DISP_DEN_STA_VPOS()+DEN_POSITION_PROTECT_PORCH; //0x55;
    dv_den_start_end_reg.dv_den_end =  Get_DISP_DEN_END_VPOS()+DEN_POSITION_PROTECT_PORCH; //0x8c5;
    IoReg_Write32(PPOVERLAY_DV_DEN_Start_End_reg, dv_den_start_end_reg.regValue);
    down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set = 1;
    ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_set = 1;
    ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set = 1; //D5
    ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;  //D4
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
    ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);

    //while((IoReg_Read32(0xb8028028)&_BIT0)&&(--timeout))
    while(((ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set)||
		  (ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_set)||
		  (ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set)||
		  (ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set))&&(--timeout)){
        msleep(1);
		ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	}

	if(count==0)
		rtd_pr_vbe_notice("[%s] wait timeout, line=%d]n", __FUNCTION__, __LINE__);

    dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
    rtd_pr_vbe_debug("speedUpV=%d, den_sta=%d, den_end=%d\n", dv_total_reg.dv_total,dv_den_start_end_reg.dv_den_sta,
        dv_den_start_end_reg.dv_den_end);
}
extern unsigned char get_scaler_stop_flag(unsigned char display);

void drvif_scaler_waitFor_remove_multiple_CorrectTiming(void)
{
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	unsigned int count = 0;
	UINT32 mul_dbuf_cnt=0;
	unsigned long flags;//for spin_lock_irqsave

    down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.dreg_dbuf_en = 0;
	double_buffer_ctrl_reg.drm_multireg_dbuf_en = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_read_sel = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_vsync_sel = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

	count = 0xa0000; //100ms, 5 frame.
	while((count == 0xa0000)&&(mul_dbuf_cnt<10000)){
		double_buffer_ctrl_reg.drm_multireg_dbuf_set = 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
		count = 0xa0000;
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		while((double_buffer_ctrl_reg.drm_multireg_dbuf_set)&& --count){
			double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		}
		if(count==0){
			rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_set 1 wait timeout !!!\n");
		}
		mul_dbuf_cnt++;
	}
}

void drvif_scaler_set_fixlastline_vtotal_speedup_protect_on(void)
{
//    unsigned int count=20;
//	unsigned long flags;//for spin_lock_irqsave
	unsigned int timeout = 0;
	ppoverlay_dv_total_RBUS dv_total_reg;
	ppoverlay_uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
    ppoverlay_memcdtg_dv_total_RBUS memcdtg_dv_total_reg;
	unsigned int speedUp_vtotal = 0;
	unsigned int vFreq=0;
//    ppoverlay_dispd_stage1_sm_ctrl_RBUS   dispd_stage1_sm_ctrl_reg;
//	ppoverlay_dispd_smooth_toggle1_RBUS   dispd_smooth_toggle1_reg;
    ppoverlay_new_meas0_linecnt_real_RBUS ppoverlay_new_meas0_linecnt_real_reg;
//	ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
//	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	unsigned int speedUP_vFreq=INCREASE_FLL_VFREQ_60HZ;
	vFreq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);

    dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);

	if((vFreq>235)&&(vFreq<=245)){ // 24hz
		//if(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 48){ //24 to 48
		if(dv_total_reg.dv_total > INCREASE_FLL_VTOTAL_60HZ){ //24 to 48
			speedUP_vFreq = INCREASE_FLL_VFREQ_48HZ_2;
		}else{ // 24 to 60
			speedUP_vFreq = INCREASE_FLL_VFREQ_48HZ;
	}
	}else if((vFreq>245)&&(vFreq<=255)){ // 25hz
		speedUP_vFreq = INCREASE_FLL_VFREQ_50HZ;
	}else if((vFreq>495)&&(vFreq<=505)){
		speedUP_vFreq = INCREASE_FLL_VFREQ_50HZ;
	}else if((vFreq>295)&&(vFreq<=305)){
		speedUP_vFreq = INCREASE_FLL_VFREQ_60HZ;
	}else{
		speedUP_vFreq = INCREASE_FLL_VFREQ_60HZ;
	}
	speedUp_vtotal = Get_DISPLAY_CLOCK_TYPICAL()/(Get_DISP_HORIZONTAL_TOTAL()*speedUP_vFreq);
#if 0
    timeout = 0xa0000; //100ms, 5 frame.

    dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
    dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
    IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
    dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
    IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
    do{
        dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
    }while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply==1) && (--timeout));

    down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
    ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 1;      //D7
    ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_en = 1;  //D0
    ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 1; //D5
    ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;  //D4
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

    dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
    dv_total_reg.dv_total=speedUp_vtotal;
    IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
    uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
    uzudtg_dv_total_reg.uzudtg_dv_total = speedUp_vtotal;
    IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);
    memcdtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
    memcdtg_dv_total_reg.memcdtg_dv_total = speedUp_vtotal;
    IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.regValue);

    down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set = 1;
    ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_set = 1;
    ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set = 1; //D5
    ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;  //D4
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
    ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);

    timeout = 100; //100ms, 5 frame.

    //while((IoReg_Read32(0xb8028028)&_BIT0)&&(--timeout))
    while(((ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set)||
		  (ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_set)||
		  (ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set)||
		  (ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set))&&(--timeout)){

		if(get_scaler_stop_flag(SLR_MAIN_DISPLAY) == TRUE){
			rtd_pr_vbe_notice("FUNC:%s, break due to scaler stop\n", __FUNCTION__);
			break;
		}
        mdelay(1);
		ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	}
#else
    timeout = 0xa0000; //100ms, 5 frame.
    drvif_scaler_waitFor_remove_multiple_CorrectTiming();
    ppoverlay_new_meas0_linecnt_real_reg.regValue = rtd_inl(PPOVERLAY_new_meas0_linecnt_real_reg);
	while((ppoverlay_new_meas0_linecnt_real_reg.line_cnt_rt >= speedUp_vtotal) && --timeout){
		ppoverlay_new_meas0_linecnt_real_reg.regValue = rtd_inl(PPOVERLAY_new_meas0_linecnt_real_reg);
	}
    dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
    dv_total_reg.dv_total=speedUp_vtotal;
    IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
    uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
    uzudtg_dv_total_reg.uzudtg_dv_total = speedUp_vtotal;
    IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);
    memcdtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
    memcdtg_dv_total_reg.memcdtg_dv_total = speedUp_vtotal;
    IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.regValue);
#endif

	vbe_disp_pwm_frequency_update(FALSE);

	if(Get_DISPLAY_REFRESH_RATE() == 60){
		IoReg_Mask32(LDSPI_ld_spi_v_total_reg, 0x7ffff, ((speedUp_vtotal/2)-1)<<19); // repeat local dimming vsync
	}
    rtd_pr_vbe_notice("[FLL_SpeedUp] vfreq=%d, speedUpV=%d\n", vFreq, speedUp_vtotal);
}

void drvif_scaler_set_frc2fsync_vtotal_protect_on_without_den_shift(void)
{
//    unsigned int count=20;
	unsigned int timeout = 0;
	ppoverlay_dv_total_RBUS dv_total_reg;
	ppoverlay_uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
    ppoverlay_memcdtg_dv_total_RBUS memcdtg_dv_total_reg;
	unsigned int speedUp_vtotal = 0;
	unsigned int vFreq=0;
    ppoverlay_dispd_stage1_sm_ctrl_RBUS   dispd_stage1_sm_ctrl_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS   dispd_smooth_toggle1_reg;

	ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	unsigned long flags;//for spin_lock_irqsave

	vFreq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
/*
	if((vFreq>245)&&(vFreq<=255)){ // 25hz
		speedUp_vtotal = INCREASE_VTOTAL_50HZ;
	}else if((vFreq>495)&&(vFreq<=505)){
		speedUp_vtotal = INCREASE_VTOTAL_50HZ;
	}else if((vFreq>295)&&(vFreq<=305)){
		speedUp_vtotal = INCREASE_VTOTAL_50HZ;
	}else{
		speedUp_vtotal = INCREASE_VTOTAL_60HZ;
	}

	if(Get_DISP_HORIZONTAL_TOTAL() <= 2450){ // 2k panel
		speedUp_vtotal /= 2;
	}
*/
	speedUp_vtotal = drvif_scaler_get_speedUp_vtotal(vFreq);

    timeout = 0xa0000; //100ms, 5 frame.

    dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
    dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
    IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
    dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
    dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
    IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
    do{
        dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
    }while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply==1) && (--timeout));

    down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
    ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 1;      //D7
    ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_en = 1;  //D0
    ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 1; //D5
    ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;  //D4
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

    dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
    dv_total_reg.dv_total=speedUp_vtotal;
    IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
    uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
    uzudtg_dv_total_reg.uzudtg_dv_total = speedUp_vtotal;
    IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);
    memcdtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
    memcdtg_dv_total_reg.memcdtg_dv_total = speedUp_vtotal;
    IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.regValue);

	vbe_disp_pwm_frequency_update(TRUE);

    down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set = 1;
    ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_set = 1;
    ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set = 1; //D5
    ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;  //D4
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
    ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);

    timeout = 100; //100ms, 5 frame.

    //while((IoReg_Read32(0xb8028028)&_BIT0)&&(--timeout))
    while(((ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set)||
		  (ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_set)||
		  (ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set)||
		  (ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set))&&(--timeout)){

		if(get_scaler_stop_flag(SLR_MAIN_DISPLAY) == TRUE){
			rtd_pr_vbe_notice("FUNC:%s, break due to scaler stop\n", __FUNCTION__);
			break;
		}
        mdelay(1);
		ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	}

	if(Get_DISPLAY_REFRESH_RATE() == 60){
		IoReg_Mask32(LDSPI_ld_spi_v_total_reg, 0x7ffff, ((speedUp_vtotal/2)-1)<<19); // repeat local dimming vsync
	}

    rtd_pr_vbe_notice("vfreq=%d, speedUpV=%d\n", vFreq, speedUp_vtotal);
}
void drvif_scaler_set_frc2fsync_vtotal_protect_off(void)
{
//    unsigned int count=20;
//	unsigned int timeout = 0;
    unsigned int DVtotal=0;
	ppoverlay_dv_total_RBUS dv_total_reg;
	ppoverlay_uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
    ppoverlay_memcdtg_dv_total_RBUS memcdtg_dv_total_reg;
	UINT32 vFreq=0;
	ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	ppoverlay_new_meas0_linecnt_real_RBUS ppoverlay_new_meas0_linecnt_real_reg;
	UINT32 cnt = 0x3fffff;
	unsigned long flags;//for spin_lock_irqsave

	if((vbe_disp_get_adaptivestream_fs_mode() == 1)
#ifdef CONFIG_I2RND_ENABLE	//Eric@20171222 I2rnd re-run main case need to follow main vo framerate setting, so do not use recovery v-total
		|| (Scaler_I2rnd_get_timing_enable() && ((vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC) ||(vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1)))
#endif
	){
    		DVtotal = scaler_disp_smooth_get_memc_protect_vtotal();
	}else{
		DVtotal = drvif_scaler_get_frc2fsync_recovery_vtotal();
	}
    if(DVtotal==0){
		vFreq = Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ);
        DVtotal = Get_DISPLAY_CLOCK_TYPICAL()/(Get_DISP_HORIZONTAL_TOTAL()
			*vFreq);
    }

    rtd_pr_vbe_notice("[%s] DVtotal=%d\n", __FUNCTION__, DVtotal);
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	//down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
   	ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
   	ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 0;      //D7
	//ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_en = 0;  //D0
	ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0; //D5
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;  //D4
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	//up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock

	if(drvif_scaler_get_display_mode() != DISPLAY_MODE_FRAME_SYNC){
		ppoverlay_new_meas0_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
		while((ppoverlay_new_meas0_linecnt_real_reg.line_cnt_rt >= (DVtotal-1)) && --cnt){
			ppoverlay_new_meas0_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
		}
	}

	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
    dv_total_reg.dv_total=DVtotal-1;
    IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
    uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
    uzudtg_dv_total_reg.uzudtg_dv_total = DVtotal-1;
    IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);
    memcdtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
    memcdtg_dv_total_reg.memcdtg_dv_total = DVtotal-1;
    IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.regValue);
	vbe_disp_pwm_frequency_update(FALSE);

	if(Get_DISPLAY_REFRESH_RATE() == 60){
		IoReg_Mask32(LDSPI_ld_spi_v_total_reg, 0x7ffff, ((DVtotal/2)-1)<<19); // repeat local dimming vsync
		rtd_pr_vbe_notice("vertical_total=%x, 0xb802c96c = %x\n", DVtotal, IoReg_Read32(LDSPI_ld_spi_v_total_reg));
	}
}
void drvif_scaler_set_frc2fsync_den_protect_off(void)
{
	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	unsigned int timeout = 0;
    ppoverlay_dispd_smooth_toggle1_RBUS   dispd_smooth_toggle1_reg;
    ppoverlay_dispd_stage1_sm_ctrl_RBUS   dispd_stage1_sm_ctrl_reg;

    IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg,_BIT2);
    timeout = 0xa0000; //100ms, 5 frame.
    dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
    dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
    IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);

    dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
    dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
    IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
    do{
        dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
    }while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply==1) && (--timeout));
    dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
    dv_den_start_end_reg.dv_den_sta = Get_DISP_DEN_STA_VPOS();
    dv_den_start_end_reg.dv_den_end = Get_DISP_DEN_END_VPOS();
    IoReg_Write32(PPOVERLAY_DV_DEN_Start_End_reg, dv_den_start_end_reg.regValue);
    IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg,_BIT0);
    //rtd_pr_vbe_notice("v0=%x\n",IoReg_Read32(0xb809d0f0));
    timeout = 0xa0000; //100ms, 5 frame.
    while((IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg)&_BIT0)&&(--timeout)){};
}

void drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(UINT8 bEnable)
{
	ppoverlay_frc2fsync_speedup_ctrl_RBUS ppoverlay_frc2fsync_speedup_ctrl_reg;
	ldspi_ld_spi_v_total_RBUS ldspi_ld_spi_v_total_reg;
	ppoverlay_display_timing_ctrl1_RBUS ppoverlay_display_timing_ctrl1_reg;
	unsigned int speedUp_vtotal = 0;
	unsigned int vFreq=0;

	ppoverlay_frc2fsync_speedup_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_FRC2fsync_speedup_ctrl_reg);
	ldspi_ld_spi_v_total_reg.regValue = IoReg_Read32(LDSPI_ld_spi_v_total_reg);

	if(bEnable) {
		if(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag()){
			wait_ONMS_ready(SLR_MAIN_DISPLAY, FRC2FSYNC_VRR_MIN_SUPPORT_FREQ);
	    	if((get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1) || (get_hdmi_4k_hfr_mode() == HDMI_3K120))
			{
				vFreq = get_hdmi_online_freq(SLR_MAIN_DISPLAY, _MAIN_VGIP_PATH);
			}
			else
			{
				vFreq = get_hdmi_online_freq(SLR_MAIN_DISPLAY, _DAM_VGIP_PATH);
			}
			rtd_pr_vbe_notice("[%s] vrr get freq=%d.\n", __FUNCTION__, vFreq);
		}else{
			vFreq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
		}

		if((vFreq>245)&&(vFreq<=255)){ // 25hz
			speedUp_vtotal = INCREASE_VTOTAL_50HZ;
		}else if((vFreq>495)&&(vFreq<=505)){ //50Hz
			speedUp_vtotal = INCREASE_VTOTAL_50HZ;
		}else if((vFreq>995)&&(vFreq<=1005)){ // 100hz
			speedUp_vtotal = INCREASE_VTOTAL_50HZ;
		}else if((vFreq>295)&&(vFreq<=305)){ //30Hz
			speedUp_vtotal = INCREASE_VTOTAL_50HZ;
		}else if((vFreq>595)&&(vFreq<=605)){ //60Hz
			speedUp_vtotal = INCREASE_VTOTAL_50HZ;
		}else if((vFreq>1195)&&(vFreq<=1205)){ //120Hz
			speedUp_vtotal = INCREASE_VTOTAL_50HZ;
		}else if((vFreq>235)&&(vFreq<=245)){ // 24hz
			speedUp_vtotal = INCREASE_VTOTAL_24HZ;
		}else{
			if(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag()){
				// VRR case, freq < FRC2FSYNC_VRR_SPECIAL_FREQ, speed up vtotal need calculate by formula
				if(vFreq<=FRC2FSYNC_VRR_SPECIAL_FREQ){
					if(vFreq >= FRC2FSYNC_VRR_MIN_SUPPORT_FREQ){
						speedUp_vtotal = Get_DISPLAY_CLOCK_TYPICAL()/(Get_DISP_HORIZONTAL_TOTAL() * (vFreq-FRC2FSYNC_VRR_SPEEDUP_OFFSET)/10);
					}else{
						speedUp_vtotal = Get_DISPLAY_CLOCK_TYPICAL()/(Get_DISP_HORIZONTAL_TOTAL() * (FRC2FSYNC_VRR_MIN_SUPPORT_FREQ+FRC2FSYNC_VRR_SPEEDUP_OFFSET)/10);
					}
				}else{
					speedUp_vtotal = INCREASE_VTOTAL_60HZ;
				}
			}else{
				speedUp_vtotal = INCREASE_VTOTAL_60HZ;
			}
		}
			if((get_panel_res() == PANEL_RES_FHD) || (get_panel_res() == PANEL_RES_HD))
			speedUp_vtotal /= 2;
	    //add new feature, need set this bit for vtotal update.
		ppoverlay_display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		ppoverlay_display_timing_ctrl1_reg.mdtg_line_cnt_sync = 1;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, ppoverlay_display_timing_ctrl1_reg.regValue);
		rtd_pr_vbe_notice("[%s]speedUp_vtotal=%x \n", __FUNCTION__, speedUp_vtotal);
		ppoverlay_frc2fsync_speedup_ctrl_reg.speedup_dv_total = speedUp_vtotal;
		ldspi_ld_spi_v_total_reg.dv_total_sync_dtg = speedUp_vtotal/2;	//reproduce_mode = 2
		ppoverlay_frc2fsync_speedup_ctrl_reg.frc2fsync_speedup_by_vtotal_en = 1;
		ldspi_ld_spi_v_total_reg.dv_total_src_sel = 1;
	}else{
		ppoverlay_frc2fsync_speedup_ctrl_reg.frc2fsync_speedup_by_vtotal_en = 0;
		ldspi_ld_spi_v_total_reg.dv_total_src_sel = 0;
	}
	IoReg_Write32(LDSPI_ld_spi_v_total_reg, ldspi_ld_spi_v_total_reg.regValue);
	IoReg_Write32(PPOVERLAY_FRC2fsync_speedup_ctrl_reg, ppoverlay_frc2fsync_speedup_ctrl_reg.regValue);
	rtd_pr_vbe_notice("[%s]c96c=%x,8018=%x \n", __FUNCTION__, IoReg_Read32(LDSPI_ld_spi_v_total_reg),IoReg_Read32(PPOVERLAY_FRC2fsync_speedup_ctrl_reg));

}

void drvif_scaler_enable_frc2fsync_HW_speedup_vtotal_by_I2D(UINT8 bEnable)
{
        ppoverlay_frc2fsync_speedup_ctrl_RBUS ppoverlay_frc2fsync_speedup_ctrl_reg;
        ppoverlay_dv_total_RBUS ppoverlay_dv_total_reg;
        ldspi_ld_spi_v_total_RBUS ldspi_ld_spi_v_total_reg;
	ppoverlay_display_timing_ctrl1_RBUS ppoverlay_display_timing_ctrl1_reg;
        unsigned int speedUp_vtotal = 0;
        //unsigned int vFreq=0;

        //vFreq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
        ppoverlay_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
        speedUp_vtotal = ppoverlay_dv_total_reg.dv_total + FRC2FS_I2D_TARGET;

        //add new feature, need set this bit for vtotal update.
        ppoverlay_display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
        ppoverlay_display_timing_ctrl1_reg.mdtg_line_cnt_sync = 1;
        IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, ppoverlay_display_timing_ctrl1_reg.regValue);

        ppoverlay_frc2fsync_speedup_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_FRC2fsync_speedup_ctrl_reg);
        ppoverlay_frc2fsync_speedup_ctrl_reg.speedup_dv_total = speedUp_vtotal;
        ldspi_ld_spi_v_total_reg.regValue = IoReg_Read32(LDSPI_ld_spi_v_total_reg);
        ldspi_ld_spi_v_total_reg.dv_total_sync_dtg = speedUp_vtotal/2;  //reproduce_mode = 2
        if(bEnable){
                ppoverlay_frc2fsync_speedup_ctrl_reg.frc2fsync_speedup_by_vtotal_en = 1;
                ldspi_ld_spi_v_total_reg.dv_total_src_sel = 1;
        }else{
                ppoverlay_frc2fsync_speedup_ctrl_reg.frc2fsync_speedup_by_vtotal_en = 0;
                ldspi_ld_spi_v_total_reg.dv_total_src_sel = 0;
        }
        IoReg_Write32(LDSPI_ld_spi_v_total_reg, ldspi_ld_spi_v_total_reg.regValue);
        IoReg_Write32(PPOVERLAY_FRC2fsync_speedup_ctrl_reg, ppoverlay_frc2fsync_speedup_ctrl_reg.regValue);
        rtd_pr_vbe_notice("[%s]c96c=%x,8018=%x \n", __FUNCTION__, IoReg_Read32(LDSPI_ld_spi_v_total_reg),IoReg_Read32(PPOVERLAY_FRC2fsync_speedup_ctrl_reg));

}

void drvif_scaler_enable_frc2fsync_interrupt(UINT8 bEnable)
{
#if 0
	ppoverlay_dtg_ie_RBUS ppoverlay_dtg_ie_reg;
	ppoverlay_dtg_pending_status_RBUS ppoverlay_dtg_pending_status_reg;

	//clear frc2fsync pending status
	ppoverlay_dtg_pending_status_reg.regValue = 0;
	ppoverlay_dtg_pending_status_reg.disp_frc2fsync_event = 1;
	IoReg_Write32(PPOVERLAY_DTG_pending_status_reg, ppoverlay_dtg_pending_status_reg.regValue);
	ppoverlay_dtg_ie_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_ie_reg);

	if(bEnable){
		ppoverlay_dtg_ie_reg.disp_frc2fsync_event_ie = 1;
	}else{
		ppoverlay_dtg_ie_reg.disp_frc2fsync_event_ie = 0;
	}
	IoReg_Write32(PPOVERLAY_DTG_ie_reg, ppoverlay_dtg_ie_reg.regValue);
	rtd_pr_vbe_notice("[DTG][%s]  en=%d\n", __FUNCTION__,  bEnable);
#else
	ppoverlay_dtg_ie_2_RBUS ppoverlay_dtg_ie_2_reg;
	ppoverlay_dtg_pending_status_2_RBUS ppoverlay_dtg_pending_2_status_reg;

	//clear frc2fsync pending status
	ppoverlay_dtg_pending_2_status_reg.regValue = 0;
	ppoverlay_dtg_pending_2_status_reg.disp_frc2fsync_event_2 = 1;
	IoReg_Write32(PPOVERLAY_DTG_pending_status_2_reg, ppoverlay_dtg_pending_2_status_reg.regValue);
	ppoverlay_dtg_ie_2_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_ie_2_reg);

	if(bEnable){
		ppoverlay_dtg_ie_2_reg.disp_frc2fsync_event_ie_2= 1;
	}else{
		ppoverlay_dtg_ie_2_reg.disp_frc2fsync_event_ie_2 = 0;
	}
	IoReg_Write32(PPOVERLAY_DTG_ie_2_reg, ppoverlay_dtg_ie_2_reg.regValue);
	rtd_pr_vbe_notice("[DTG][%s]  en=%d\n", __FUNCTION__,  bEnable);

#endif
}

void drvif_scaler_memc_enable_frc2fsync_HW_speedup_vtotal(unsigned char bEnable)
{
    ppoverlay_frc_to_frame_sync_den_end_ctrl2_RBUS ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg;
    unsigned int speedUp_vtotal = 0;
    unsigned int vFreq=0;

    ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl2_reg);

    if(bEnable)
    {
#if 0
        if(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())
        {
            wait_ONMS_ready(SLR_MAIN_DISPLAY, FRC2FSYNC_VRR_MIN_SUPPORT_FREQ);
            if((get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1) || (get_hdmi_4k_hfr_mode() == HDMI_3K120))
            {
                vFreq = get_hdmi_online_freq(SLR_MAIN_DISPLAY, _MAIN_VGIP_PATH);
            }
            else
            {
                vFreq = get_hdmi_online_freq(SLR_MAIN_DISPLAY, _DAM_VGIP_PATH);
            }

            rtd_pr_vbe_notice("[%s] vrr get freq=%d.\n", __FUNCTION__, vFreq);
        }
        else
#endif
        {
            vFreq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
        }

        if((vFreq>245)&&(vFreq<=255))
        { // 25hz
            speedUp_vtotal = INCREASE_VTOTAL_50HZ;
        }
        else if((vFreq>495)&&(vFreq<=505))
        { //50Hz
            speedUp_vtotal = INCREASE_VTOTAL_50HZ;
        }
        else if((vFreq>995)&&(vFreq<=1005))
        { // 100hz
            speedUp_vtotal = INCREASE_VTOTAL_50HZ;
        }
        else if((vFreq>295)&&(vFreq<=305))
        { //30Hz
            speedUp_vtotal = INCREASE_VTOTAL_50HZ;
        }
        else if((vFreq>595)&&(vFreq<=605))
        { //60Hz
            speedUp_vtotal = INCREASE_VTOTAL_50HZ;
        }
        else if((vFreq>1195)&&(vFreq<=1205))
        { //120Hz
            speedUp_vtotal = INCREASE_VTOTAL_50HZ;
        }
        else if((vFreq>235)&&(vFreq<=245))
        { // 24hz
            speedUp_vtotal = INCREASE_VTOTAL_24HZ;
        }
        else
        {
#if 0
            if(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())
            {
                // VRR case, freq < FRC2FSYNC_VRR_SPECIAL_FREQ, speed up vtotal need calculate by formula
                if(vFreq<=FRC2FSYNC_VRR_SPECIAL_FREQ)
                {
                    if(vFreq >= FRC2FSYNC_VRR_MIN_SUPPORT_FREQ)
                    {
                    speedUp_vtotal = Get_DISPLAY_CLOCK_TYPICAL()/(Get_DISP_HORIZONTAL_TOTAL() * (vFreq-FRC2FSYNC_VRR_SPEEDUP_OFFSET)/10);
                    }
                    else
                    {
                        speedUp_vtotal = Get_DISPLAY_CLOCK_TYPICAL()/(Get_DISP_HORIZONTAL_TOTAL() * (FRC2FSYNC_VRR_MIN_SUPPORT_FREQ+FRC2FSYNC_VRR_SPEEDUP_OFFSET)/10);
                    }
                }
                else
                {
                    speedUp_vtotal = INCREASE_VTOTAL_60HZ;
                }
            }
            else
#endif
            {
                speedUp_vtotal = INCREASE_VTOTAL_60HZ;
            }
        }

        if((get_panel_res() == PANEL_RES_FHD) || (get_panel_res() == PANEL_RES_HD))
        {
            speedUp_vtotal /= 2;
        }

        ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg.memcdtg_speedup_dv_total = speedUp_vtotal;

        ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg.memcdtg_speedup_en = 1;

    }
    else
    {
        ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg.memcdtg_speedup_en = 0;
    }

    IoReg_Write32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl2_reg, ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg.regValue);

    ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl2_reg);

    rtd_pr_vbe_notice("[%s %d] speedup vtotal:%d\n", __FUNCTION__, __LINE__, ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg.memcdtg_speedup_dv_total);
}

void drvif_scaler_memc_enable_frc2fsync_interrupt(unsigned char enable)
{
    ppoverlay_dtg_ie_2_RBUS ppoverlay_dtg_ie_2_reg;
    ppoverlay_dtg_pending_status_2_RBUS ppoverlay_dtg_pending_2_status_reg;

    //clear frc2fsync pending status
    ppoverlay_dtg_pending_2_status_reg.regValue = 0;
    ppoverlay_dtg_pending_2_status_reg.memcdtg_frc2fsync_event_2 = 1;
    IoReg_Write32(PPOVERLAY_DTG_pending_status_2_reg, ppoverlay_dtg_pending_2_status_reg.regValue);

    ppoverlay_dtg_ie_2_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_ie_2_reg);

    if(enable)
    {
        ppoverlay_dtg_ie_2_reg.memcdtg_frc2fsync_event_ie_2= 1;
    }
    else
    {
        ppoverlay_dtg_ie_2_reg.memcdtg_frc2fsync_event_ie_2 = 0;
    }

    IoReg_Write32(PPOVERLAY_DTG_ie_2_reg, ppoverlay_dtg_ie_2_reg.regValue);

    rtd_pr_vbe_notice("[DTG][%s %d] enable = %d\n", __FUNCTION__,  __LINE__, enable);
}


void drvif_scaler_memc_set_display_mode_frc2fsync_by_hw_speedup(unsigned char frc2fsync)
{
    ppoverlay_frc_to_frame_sync_den_end_ctrl2_RBUS ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg;
    ppoverlay_memcdtg_control_RBUS memcdtg_control_reg;
    ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
    ppoverlay_memcdtg_free_dv_total_RBUS ppoverlay_memcdtg_free_dv_total_reg;
    ppoverlay_memcdtg_free_dh_total_RBUS ppoverlay_memcdtg_free_dh_total_reg;

    UINT32 TimeOutCounterThreshold = Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut();
    UINT32 TimeOutCounter = 0;
    unsigned char display;
    extern unsigned judge_scaler_break_case(unsigned char display);

    //frc
    if(!frc2fsync)
    {
        drvif_scaler_memc_enable_frc2fsync_HW_speedup_vtotal(0);
        drvif_scaler_memc_enable_frc2fsync_interrupt(0);

        ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
        ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
        IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

        ppoverlay_memcdtg_free_dv_total_reg.regValue = rtd_inl(PPOVERLAY_memcdtg_FREE_DV_TOTAL_reg);
        ppoverlay_memcdtg_free_dv_total_reg.memcdtg_free_dv_total = Get_DISP_VERTICAL_TOTAL() - 1;
        rtd_outl(PPOVERLAY_memcdtg_FREE_DV_TOTAL_reg, ppoverlay_memcdtg_free_dv_total_reg.regValue);

        ppoverlay_memcdtg_free_dh_total_reg.regValue = rtd_inl(PPOVERLAY_memcdtg_FREE_DH_TOTAL_reg);
        ppoverlay_memcdtg_free_dh_total_reg.memcdtg_free_dh_total = Get_DISP_HORIZONTAL_TOTAL() - 1;
        ppoverlay_memcdtg_free_dh_total_reg.memcdtg_free_dh_total_last_line = Get_DISP_HORIZONTAL_TOTAL() - 1;
        rtd_outl(PPOVERLAY_memcdtg_FREE_DH_TOTAL_reg, ppoverlay_memcdtg_free_dh_total_reg.regValue);

        memcdtg_control_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_reg);
        memcdtg_control_reg.memcdtg_en = 1;

        memcdtg_control_reg.memcdtg_fsync_en = 0;
        memcdtg_control_reg.memcdtg_line_cnt_sync = 0;
        // if this bit set 1 use  memc free run b802860c, b8028610(bootcode set this value)
        memcdtg_control_reg.memcdtg_free_vh_total_en = 1; 

        IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL_reg, memcdtg_control_reg.regValue);

        return;
    }

    memcdtg_control_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_reg);
    if(memcdtg_control_reg.memcdtg_frc_fsync_status  == 1)
    {
        rtd_pr_vbe_notice("[%s %d]memc is fsync, not run frc2fs flow\n", __FUNCTION__, __LINE__);
        drvif_scaler_memc_enable_frc2fsync_HW_speedup_vtotal(0);
        drvif_scaler_memc_enable_frc2fsync_interrupt(0);
        return;
    }


    //frc2fs
    if((Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_SUB_DISPLAY) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE)){
        display = SLR_SUB_DISPLAY;
    }
    else
    {
        display = SLR_MAIN_DISPLAY;
    }

    ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl2_reg);

    if(Get_DISPLAY_PANEL_OLED_TYPE()==1)
    {
        ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg.memcdtg_dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + OLED_PANEL_DEN_PROTECT_PORCH;
    }
    else
    {
        if((get_panel_res() == PANEL_RES_FHD) || (get_panel_res() == PANEL_RES_HD))
        {
            ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg.memcdtg_dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH/2;
        }
        else
        {
            ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg.memcdtg_dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH;
        }
    }

    IoReg_Write32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl2_reg, ppoverlay_frc_to_frame_sync_den_end_ctrl2_reg.regValue);

    //memcdtg frc2fs hw protect
    memcdtg_control_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_reg);
    memcdtg_control_reg.memcdtg_frc2fsync_by_hw = 1;
    IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL_reg, memcdtg_control_reg.regValue);

    drvif_scaler_memc_enable_frc2fsync_HW_speedup_vtotal(TRUE);
    drvif_scaler_memc_enable_frc2fsync_interrupt(TRUE);

    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
    ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

    memcdtg_control_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_reg);
    memcdtg_control_reg.memcdtg_en = 1;
    memcdtg_control_reg.memcdtg_fsync_en = 1;
    memcdtg_control_reg.memcdtg_line_cnt_sync = 1;
    memcdtg_control_reg.memcdtg_free_vh_total_en = 0;

    IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL_reg, memcdtg_control_reg.regValue);

    memcdtg_control_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_reg);
    while(memcdtg_control_reg.memcdtg_frc_fsync_status  == 0)
    {

        if(judge_scaler_break_case(display))
        {
            rtd_pr_vbe_notice("###[%s %d]memc frc2fsync need to break###\n", __FUNCTION__, __LINE__);
            break;
        }

        TimeOutCounter++;

        if(TimeOutCounter > TimeOutCounterThreshold)
        {
            rtd_pr_vbe_notice("[%s %d]memc frc2fs timeout\n", __FUNCTION__, __LINE__);
            break;
        }

        usleep_range(10 * 1000, 10 * 1000);

        memcdtg_control_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_reg);
    }

    drvif_scaler_memc_enable_frc2fsync_HW_speedup_vtotal(FALSE);
    drvif_scaler_memc_enable_frc2fsync_interrupt(FALSE);
}


UINT8 watingLock_flag=0;

void drvif_scaler_frc2fsync_set_waiting_lock(UINT8 b_watingLock)
{
    watingLock_flag = b_watingLock;
}

UINT8 drvif_scaler_frc2fsync_get_waiting_lock(void)
{
    return watingLock_flag;
}


unsigned char drvif_scaler_frc2fsync_check_fsync_lock(void)
{
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;

    display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

    if(display_timing_ctrl1_reg.disp_frc_fsync==1){
        drvif_scaler_set_frc2fsync_vtotal_protect_off();
        drvif_scaler_frc2fsync_set_waiting_lock(FALSE);
        return TRUE;
    }
    return FALSE;
}

extern bool is_veb_isr_registered(void);
void drvif_scaler_frc2fsync_check_timer_start(void)
{

	if (unlikely(!is_veb_isr_registered())) {
		rtd_pr_vbe_crit("%s: vbe isr isn't registered, won't start timer..\n", __func__);
		return;
	}

    rtd_pr_vbe_notice("#HWT_START#\n");
	// disable timer7
	rtd_outl(TIMER_TC7CR_reg, 0);
	// write 1 clear
	rtd_outl(TIMER_ISR_reg, TIMER_ISR_tc7_int_mask);
	// enable timer7 interrupt
	rtd_outl(TIMER_TCICR_reg, TIMER_TCICR_tc7ie_mask|1);

#ifndef BUILD_QUICK_SHOW
        rtk_timer_set_target(7, TIMER_CLOCK/4000); // 2ms
#else
        mdelay(2);// 2ms
#endif
	// enable timer7
	rtd_outl(TIMER_TC7CR_reg, TIMER_TC7CR_tc7en_mask);

}

void drvif_scaler_frc2fsync_check_timer_stop(void)
{
    rtd_pr_vbe_notice("#HWT_STOP#\n");

	// disable timer7
	rtd_outl(TIMER_TC7CR_reg, 0);
	// disable timer7 interrupt
	rtd_outl(TIMER_TCICR_reg, TIMER_TCICR_tc7ie_mask|0);
	// write 1 clear
	rtd_outl(TIMER_ISR_reg, TIMER_ISR_tc7_int_mask);
}

void drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Mcode_HW_CTRL(void)
{
	extern unsigned judge_scaler_break_case(unsigned char display);
	unsigned display;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	pll27x_reg_dclk_frc2fsync_speedup_RBUS dclk_frc2fsync_speedup_reg;
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	UINT32 TimeOutCounterThreshold = Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut();
	UINT32 TimeOutCounter = 0;
	ppoverlay_dv_total_RBUS dv_total_reg;
	UINT32 OriginalDVtotalValue = 0;

	rtd_pr_vbe_debug("TimeOutCounterThreshold:%d\n", TimeOutCounterThreshold);

	if(_DISABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp())
		return;
	display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);

	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	display_timing_ctrl2_reg.frc2fsync_speedup_en = 1;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
	// set speed up M code offset
	dclk_frc2fsync_speedup_reg.regValue = IoReg_Read32(PLL27X_REG_DCLK_FRC2FSYNC_SPEEDUP_reg);
	dclk_frc2fsync_speedup_reg.frc2fsync_speed_dir = 1;
	dclk_frc2fsync_speedup_reg.frc2fsync_speedup_offset = 0x80; // 2048, should be m_code = 1
	IoReg_Write32(PLL27X_REG_DCLK_FRC2FSYNC_SPEEDUP_reg, dclk_frc2fsync_speedup_reg.regValue);

	rtd_pr_vbe_debug("SYSTEM_DCLK_FRC2FSYNC_SPEEDUP_reg:%x\n", IoReg_Read32(PLL27X_REG_DCLK_FRC2FSYNC_SPEEDUP_reg));

	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	OriginalDVtotalValue = dv_total_reg.dv_total;

	//increase porch, vaule by test
	dv_total_reg.dv_total = dv_total_reg.dv_total+ Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_Porch();
	rtd_pr_vbe_debug("[%s] speed up by vtotal : %x \n", __FUNCTION__, dv_total_reg.dv_total);
	IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue-1);

	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

	while(_FALSE == display_timing_ctrl1_reg.disp_frc_fsync)
	{
		if(judge_scaler_break_case(display))
		{
			rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
			break;
		}
		if((Get_DISPLAY_PANEL_OLED_TYPE()==TRUE) &&
		    /*(drv_memory_get_game_mode_dynamic()!= drv_memory_get_game_mode()) &&*/
			(drv_memory_get_game_mode()==TRUE)){
			rtd_pr_vbe_notice("\r\n###game mode need to break###\r\n");
			break;
		}
		msleep(1);
		TimeOutCounter++;

		if(TimeOutCounter > TimeOutCounterThreshold)
		{
			rtd_pr_vbe_notice("SU wait timeout......\n");
			if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
			{
				drvif_scaler_set_display_mode_frc2fsync_by_hw(_TRUE);
				rtd_pr_vbe_notice("SU LineBufferMode HW set frc2framesync fail, disable it\n");
			}
			break;
		}

		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	}
	rtd_pr_vbe_debug("SU time count = %d\n",TimeOutCounter);


	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
    if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))
        dv_total_reg.dv_total = OriginalDVtotalValue;
    else if(scaler_disp_smooth_get_memc_protect_vtotal())
		dv_total_reg.dv_total =  scaler_disp_smooth_get_memc_protect_vtotal()-1;
	else
	    dv_total_reg.dv_total = OriginalDVtotalValue;
	IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
	rtd_pr_vbe_debug("[%s] recovery vtotal : %x \n", __FUNCTION__, dv_total_reg.dv_total);

	// release lock state
	//pthread_mutex_unlock(&__drvif_scaler_Mutex);
	return;
}

void drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Mcode_HW_CTRL_2(void)
{
	extern unsigned judge_scaler_break_case(unsigned char display);
	unsigned display;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	pll27x_reg_dclk_frc2fsync_speedup_RBUS dclk_frc2fsync_speedup_reg;
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	UINT32 TimeOutCounterThreshold = Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut();
	UINT32 TimeOutCounter = 0;
    ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	ppoverlay_dv_total_RBUS dv_total_reg;
//	ppoverlay_dv_total_RBUS dv_total_reg;
//	UINT32 OriginalDVtotalValue = 0;

	rtd_pr_vbe_debug("TimeOutCounterThreshold:%d\n", TimeOutCounterThreshold);

	if(_DISABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp())
		return;
	display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	drvif_scaler_set_frc2fsync_recovery_vtotal(dv_total_reg.dv_total);

	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	display_timing_ctrl2_reg.frc2fsync_speedup_en = 0;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
	// set speed up M code offset
	dclk_frc2fsync_speedup_reg.regValue = IoReg_Read32(PLL27X_REG_DCLK_FRC2FSYNC_SPEEDUP_reg);
	dclk_frc2fsync_speedup_reg.frc2fsync_speed_dir = 1;
	dclk_frc2fsync_speedup_reg.frc2fsync_speedup_offset = 0x80; // 2048, should be m_code = 1
	IoReg_Write32(PLL27X_REG_DCLK_FRC2FSYNC_SPEEDUP_reg, dclk_frc2fsync_speedup_reg.regValue);

	rtd_pr_vbe_debug("SYSTEM_DCLK_FRC2FSYNC_SPEEDUP_reg:%x\n", IoReg_Read32(PLL27X_REG_DCLK_FRC2FSYNC_SPEEDUP_reg));

    uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
    uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
    IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);

    drvif_scaler_set_frc2fsync_vtotal_protect_on();
    drvif_scaler_frc2fsync_check_timer_start();
    drvif_scaler_frc2fsync_set_waiting_lock(_TRUE);

    IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg,_BIT2); // clear double buffer
    display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
    display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
    IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
    display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
    if(display_timing_ctrl1_reg.disp_frc_fsync==1){
        drvif_scaler_set_frc2fsync_vtotal_protect_off();
        drvif_scaler_frc2fsync_set_waiting_lock(FALSE);
    }

	while(_TRUE == drvif_scaler_frc2fsync_get_waiting_lock())
	{
		if(judge_scaler_break_case(display))
		{
			rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
            drvif_scaler_set_frc2fsync_vtotal_protect_off();
            //drvif_scaler_frc2fsync_check_timer_stop();
            //drvif_scaler_set_frc2fsync_den_protect_off();
			break;
		}
		/*
		if((Get_DISPLAY_PANEL_OLED_TYPE()==TRUE) &&
		    (drv_memory_get_game_mode_dynamic()!= drv_memory_get_game_mode()) &&
			(drv_memory_get_game_mode()==TRUE)){
			rtd_pr_vbe_notice("\r\n###game mode need to break###\r\n");
			break;
		}*/
		msleep(1);
		TimeOutCounter++;

		if(TimeOutCounter > TimeOutCounterThreshold)
		{
            drvif_scaler_set_frc2fsync_vtotal_protect_off();
            //drvif_scaler_frc2fsync_check_timer_stop();
            //drvif_scaler_set_frc2fsync_den_protect_off();

			rtd_pr_vbe_emerg("SU wait timeout......\n");
			if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
			{
				drvif_scaler_set_display_mode_frc2fsync_by_hw(_TRUE);
				rtd_pr_vbe_notice("SU LineBufferMode HW set frc2framesync fail, disable it\n");
			}
			break;
		}
	}
    drvif_scaler_frc2fsync_check_timer_stop();
    drvif_scaler_set_frc2fsync_den_protect_off();
    rtd_pr_vbe_notice("SU timeout = %d\n", TimeOutCounter);
	return;
}
void drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Mcode_HW_CTRL_3(void)
{
		extern unsigned judge_scaler_break_case(unsigned char display);
		ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
		unsigned char display;
		ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
		pll27x_reg_dclk_frc2fsync_speedup_RBUS dclk_frc2fsync_speedup_reg;
		ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
		UINT32 TimeOutCounterThreshold = Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut();
		UINT32 TimeOutCounter = 0;
		ppoverlay_frc_to_frame_sync_den_end_ctrl_RBUS ppoverlay_frc_to_frame_sync_den_end_ctrl_reg;
		ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
		ppoverlay_dv_total_RBUS dv_total_reg;
		//unsigned int vFreq = 0;
		ppoverlay_fs_iv_dv_fine_tuning5_RBUS fs_iv_dv_fine_tuning5_reg;
		ppoverlay_iv2dv_double_buffer_ctrl_RBUS ppoverlay_iv2dv_double_buffer_ctrl_reg;
		unsigned long flags;//for spin_lock_irqsave
		ppoverlay_dh_total_last_line_length_RBUS ppoverlay_dh_total_last_line_length_reg;
		rtd_pr_vbe_debug("TimeOutCounterThreshold:%d\n", TimeOutCounterThreshold);

		if(_DISABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp())
			return;

		display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
		if(judge_scaler_break_case(display) == TRUE){
			rtd_pr_vbe_notice("\r\n###func:%s need to break (%d)###\r\n", __FUNCTION__, __LINE__);
			drivf_scaler_reset_freerun();
			return;
		}
		drivf_scaler_reset_freerun();
		drvif_scaler_wclr_abnormal_dvs_long_short();

		//reset iv2dv delay
		ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg);
		ppoverlay_iv2dv_double_buffer_ctrl_reg.iv2dv_db_en = 0;
		IoReg_Write32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg, ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue);
		fs_iv_dv_fine_tuning5_reg.regValue = IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg);
#ifdef CONFIG_ALL_SOURCE_DATA_FS
		fs_iv_dv_fine_tuning5_reg.iv2dv_line = 8;
		fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = 8;
#else
		fs_iv_dv_fine_tuning5_reg.iv2dv_line = 8;
		fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = 8;
#endif
		rtd_outl(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg, fs_iv_dv_fine_tuning5_reg.regValue);


		dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
		drvif_scaler_set_frc2fsync_recovery_vtotal(dv_total_reg.dv_total+1);

		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
		display_timing_ctrl2_reg.frc2fsync_speedup_en = 0;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
		// set speed up M code offset
		dclk_frc2fsync_speedup_reg.regValue = IoReg_Read32(PLL27X_REG_DCLK_FRC2FSYNC_SPEEDUP_reg);
		dclk_frc2fsync_speedup_reg.frc2fsync_speed_dir = 1;
		dclk_frc2fsync_speedup_reg.frc2fsync_speedup_offset = 0x80; // 2048, should be m_code = 1
		IoReg_Write32(PLL27X_REG_DCLK_FRC2FSYNC_SPEEDUP_reg, dclk_frc2fsync_speedup_reg.regValue);

		rtd_pr_vbe_debug("SYSTEM_DCLK_FRC2FSYNC_SPEEDUP_reg:%x\n", IoReg_Read32(PLL27X_REG_DCLK_FRC2FSYNC_SPEEDUP_reg));

		uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
		IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);

		ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg);
		if(Get_DISPLAY_PANEL_OLED_TYPE()==1)
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS()+DEN_POSITION_PROTECT_PORCH;
		else
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + 20;
		if( ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target < Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN() )
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN();
		IoReg_Write32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg, ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue);
		rtd_pr_vbe_notice("frame_sync_den_end ctrl:%x\n", IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg));
		//vFreq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
		drvif_scaler_set_frc2fsync_vtotal_protect_on_without_den_shift();
		drvif_scaler_frc2fsync_check_timer_start();
		drvif_scaler_frc2fsync_set_waiting_lock(_TRUE);

		ppoverlay_dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
		ppoverlay_dh_total_last_line_length_reg.dh_total = Get_DISP_HORIZONTAL_TOTAL()-1;
		ppoverlay_dh_total_last_line_length_reg.dh_total_last_line = Get_DISP_HORIZONTAL_TOTAL()-5;
		IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, ppoverlay_dh_total_last_line_length_reg.regValue);
		// clear double buffer
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 0;		//D7
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);

#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
		display_timing_ctrl2_reg.frc2fsync_en = 1;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
#endif

		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
		display_timing_ctrl1_reg.disp_fix_last_line_new= 0;
		display_timing_ctrl1_reg.fsync_fll_mode= _ENABLE;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		if(display_timing_ctrl1_reg.disp_frc_fsync==1){
			drvif_scaler_set_frc2fsync_vtotal_protect_off();
			drvif_scaler_frc2fsync_set_waiting_lock(FALSE);
		}

		while(_TRUE == drvif_scaler_frc2fsync_get_waiting_lock())
		{
			if(judge_scaler_break_case(display))
			{
				rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
				drivf_scaler_reset_freerun();
				//drvif_scaler_set_frc2fsync_vtotal_protect_off();
				//drvif_scaler_frc2fsync_check_timer_stop();
				//drvif_scaler_set_frc2fsync_den_protect_off();
				break;
			}
			/*
			if((Get_DISPLAY_PANEL_OLED_TYPE()==TRUE) &&
				(drv_memory_get_game_mode_dynamic()!= drv_memory_get_game_mode()) &&
				(drv_memory_get_game_mode()==TRUE)){
				rtd_pr_vbe_notice("\r\n###game mode need to break###\r\n");
				break;
			}*/
			/*
			if(vFreq != Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)){
				drvif_scaler_set_frc2fsync_vtotal_protect_on_without_den_shift();
				vFreq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
				rtd_pr_vbe_emerg("vchange \n");
			}
			*/
			msleep(1);
			TimeOutCounter++;

			if(TimeOutCounter > TimeOutCounterThreshold)
			{
				drvif_scaler_set_frc2fsync_vtotal_protect_off();
				drvif_scaler_frc2fsync_check_timer_stop();
				//drvif_scaler_set_frc2fsync_den_protect_off();

				rtd_pr_vbe_emerg("SU wait timeout......\n");
				if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
				{
					drvif_scaler_set_display_mode_frc2fsync_by_hw(_TRUE);
					rtd_pr_vbe_notice("SU LineBufferMode HW set frc2framesync fail, disable it\n");
				}
				break;
			}
		}
		drvif_scaler_frc2fsync_check_timer_stop();
		//drvif_scaler_set_frc2fsync_den_protect_off();
		rtd_pr_vbe_notice("SU timeout = %d\n", TimeOutCounter);

		return;
}

void drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_timeout_handler(void){
#ifdef CONFIG_SUPPORT_SRC_ADC
	StructDisplayInfo  *p_timing = NULL;
#endif

	drivf_scaler_reset_freerun();
	Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE, _MODE_STATE_SEARCH);//Let main path to search state. In order to request VO
	DbgSclrFlgTkr.Main_Run_Scaler_flag = TRUE;//trigger run scaler
	DbgSclrFlgTkr.Main_Scaler_Stop_flag = FALSE;//Let scaler can be run
	vsc_force_rerun_main_scaler = TRUE;
	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI){
#ifndef BUILD_QUICK_SHOW       
		reset_hdmi_timing_ready();
		drvif_Hdmi_set_infoframe_thread_stop(FALSE);
		down(get_hdmi_detectsemaphore());
		if (get_HDMI_Global_Status() == SRC_CONNECT_DONE) {
			vfe_hdmi_drv_handle_on_line_measure_error(0);
			Set_Reply_Zero_Timing_Flag(VSC_INPUTSRC_HDMI, REPORT_ZERO_TIMING);/*Let Webos get zero timing*/
			HDMI_set_detect_flag(TRUE);
		}
		up(get_hdmi_detectsemaphore());
#endif        
	}
    #ifndef BUILD_QUICK_SHOW
	else if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_ADC)
	{
		down(get_adc_detectsemaphore());
#ifdef CONFIG_SUPPORT_SRC_ADC
		reset_adc_timing_ready();
		p_timing = Get_ADC_Dispinfo();
		p_timing->IPH_ACT_WID_PRE = 0;
		p_timing->IPV_ACT_LEN_PRE = 0;
		p_timing->IVFreq = 0;
		p_timing->IHFreq = 0;
		p_timing->IHTotal = 0;
		p_timing->IVTotal = 0;
		p_timing->ucMode_Table_Index = Mode_Table_Init_Index;
		if (get_ADC_Global_Status() == SRC_CONNECT_DONE) {
			ADC_OnlineMeasureError_Handler(get_ADC_Input_Source());
			Set_Reply_Zero_Timing_Flag(VSC_INPUTSRC_ADC, REPORT_ZERO_TIMING);/*Let Webos get zero timing*/
			//YPbPr_Power_ADCRGB(FALSE);//Turn off RGB for saving power on detecting.  brandon add
			ADC_set_detect_flag(TRUE);
		}
#endif
		up(get_adc_detectsemaphore());
	}
    #endif
}

#define _VRR_MAX_SUPPORT_FRAMERATE 1440
#define _VRR_MIN_SUPPORT_FRAMERATE 400
extern unsigned int drv_GameMode_decided_iv2dv_delay_old_mode(void);
extern unsigned int drv_GameMode_decided_iv2dv_delay_new_mode(void);
extern unsigned int drv_GameMode_iv2dv_delay_compress_margin(unsigned int iv2dv_ori);

void drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Mcode_HW_CTRL_4(void)
{
	extern unsigned judge_scaler_break_case(unsigned char display);
	unsigned char display;
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
//		pll27x_reg_dclk_frc2fsync_speedup_RBUS dclk_frc2fsync_speedup_reg;
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	UINT32 TimeOutCounterThreshold = Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut();
	UINT32 TimeOutCounter = 0;
	ppoverlay_frc_to_frame_sync_den_end_ctrl_RBUS ppoverlay_frc_to_frame_sync_den_end_ctrl_reg;
	ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
//		ppoverlay_dv_total_RBUS dv_total_reg;
//		unsigned int vFreq = 0;
	ppoverlay_dv_total_RBUS ppoverlay_dv_total_reg;
	ppoverlay_dh_total_last_line_length_RBUS ppoverlay_dh_total_last_line_length_reg;
	unsigned long flags;//for spin_lock_irqsave
//#ifndef CONFIG_MEMC_BYPASS
	unsigned char frame_idicator=0;
	ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
	unsigned int count = 0x3fffff;
//#endif

	unsigned char SpeedUp_Timeout = 0;

	rtd_pr_vbe_debug("TimeOutCounterThreshold:%d\n", TimeOutCounterThreshold);

	if(_DISABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp())
		return;

	//display = SLR_MAIN_DISPLAY;//Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	if((Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_SUB_DISPLAY) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE)){
		display = SLR_SUB_DISPLAY;	//sub path only case
	}
	else{
		display = SLR_MAIN_DISPLAY;
	}
	if(judge_scaler_break_case(display) == TRUE){
		rtd_pr_vbe_notice("\r\n###func:%s need to break (%d)###\r\n", __FUNCTION__, __LINE__);
		drivf_scaler_reset_freerun();
		return;
	}

	drivf_scaler_reset_freerun();
	drvif_scaler_wclr_abnormal_dvs_long_short();

	framesync_set_iv2dv_delay(DISPLAY_MODE_FRAME_SYNC);

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
	IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);
	ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg);
	if(Get_DISPLAY_PANEL_OLED_TYPE()==1)
		ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + DEN_POSITION_PROTECT_PORCH + FRC2FS_MEMCDEN_PROTECT_PORCH;
	else{
		ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + 20;
	}

	if(vbe_disp_decide_uzuclock_mapping_vo()){
		if((Scaler_DispGetInputInfoByDisp(0,SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfoByDisp(0,SLR_INPUT_V_FREQ) < 245))
		{
			//DTG_M 594M / uzudtg 240M, frc2fsync is refer to DTG_M, so we need adjust den end position.
			// ((2160+45+10)/2250)*42ms = 21ms + (x/2812)*21ms
			// x = 2700
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + 500;
		}
		else if((Scaler_DispGetInputInfoByDisp(0,SLR_INPUT_V_FREQ) > 245) && (Scaler_DispGetInputInfoByDisp(0,SLR_INPUT_V_FREQ) < 255))
		{
			//DTG_M 594M / uzudtg 250M, frc2fsync is refer to DTG_M, so we need adjust den end position.
			// ((2160+45+10)/2250)*40ms = 20ms + (x/2700)*20ms
			// x = 2616
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + 405;
		}
	}

	if( ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target < Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN() )
		ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN();

	IoReg_Write32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg, ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue);
	rtd_pr_vbe_notice("frame_sync_den_end ctrl:%x\n", IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg));
#if 0
	//if ((webos_tooloption.eModelModuleType == module_LGD) && (webos_tooloption.eBackLight == direct ) && (webos_tooloption.eLEDBarType == local_dim_block_32) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)){
	if(Get_TIMING_PROTECT_PANEL_TYPE() && !(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())){

		drvif_scaler_set_display_mode_frc2fsync_I2D_disable();
		drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(FALSE);
#if 0		// unused code
		ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg);

		if(Get_DISPLAY_PANEL_OLED_TYPE()==1)
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS()+DEN_POSITION_PROTECT_PORCH;
		else
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + 10;

		IoReg_Write32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg, ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue);
#endif
		//drvif_scaler_enable_frc2fsync_interrupt(TRUE);

		// clear double buffer
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 0;		//D7
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

		drvif_scaler_set_display_mode_frc2fsync_by_Reset_VODMA();

		//force toggle, trigger local dimming vtotal update
		ppoverlay_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
		ppoverlay_dv_total_reg.dv_total = ppoverlay_dv_total_reg.dv_total+1;
		IoReg_Write32(PPOVERLAY_DV_total_reg, ppoverlay_dv_total_reg.regValue);
		ppoverlay_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
		ppoverlay_dv_total_reg.dv_total = ppoverlay_dv_total_reg.dv_total-1;
		IoReg_Write32(PPOVERLAY_DV_total_reg, ppoverlay_dv_total_reg.regValue);

		ppoverlay_dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
		ppoverlay_dh_total_last_line_length_reg.dh_total = Get_DISP_HORIZONTAL_TOTAL()-1;
		ppoverlay_dh_total_last_line_length_reg.dh_total_last_line = Get_DISP_HORIZONTAL_TOTAL()-5;
		IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, ppoverlay_dh_total_last_line_length_reg.regValue);

		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
		display_timing_ctrl2_reg.frc2fsync_en = 1;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);

		//set timing framesync
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
		display_timing_ctrl1_reg.disp_fix_last_line_new= 0;
		if(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())
			display_timing_ctrl1_reg.fsync_fll_mode= _DISABLE;
		else
			display_timing_ctrl1_reg.fsync_fll_mode= _ENABLE;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);

		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

		//check framesync status
		while(display_timing_ctrl1_reg.disp_frc_fsync==0)
		{
			//Eric@20180905 If other task change framerate, need break.
			if(judge_scaler_break_case(display) || (disp_smooth_variable_setting_flag == _ENABLE))
			{
				rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
				drivf_scaler_reset_freerun();
				break;
			}
			TimeOutCounter++;

			if(TimeOutCounter > TimeOutCounterThreshold)
			{
				IoReg_SetBits(SCALEDOWN_Measure_CTRL_reg, SCALEDOWN_Measure_CTRL_m_active_line_pixel_en_mask);
				//wait 1 frame to get the UZD timing measure result.
				Scaler_wait_for_input_one_frame(display);
				rtd_pr_vbe_emerg("SU wait timeout, run SpeedUP vtotal process again......\n");
				rtd_pr_vbe_emerg("UZD measure timing : (%x), UZD measure ctrl : (%x)\n", IoReg_Read32(SCALEDOWN_Measure_ActLine_Result_Main_Path_reg), IoReg_Read32(SCALEDOWN_Measure_CTRL_reg));
				rtd_pr_vbe_emerg("ONMS1 status : (%x), ONMS3 status : (%x)\n", IoReg_Read32(ONMS_onms1_status_reg), IoReg_Read32(ONMS_onms3_status_reg));
				//non full screen height timing fs fail case need disable frc2fsync by hw too
				//if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
				{
#if 0
					drvif_scaler_set_display_mode_frc2fsync_by_hw(_TRUE);
					rtd_pr_vbe_notice("SU LineBufferMode HW set frc2framesync fail, disable it\n");
#else
					SpeedUp_Timeout = 1;
#endif

				}
				break;
			}
			msleep(0);
			display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		}
		if(SpeedUp_Timeout == 1){
			TimeOutCounter = 0;
			//Do speedup vtotal process again
			drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(TRUE);
			drvif_scaler_enable_frc2fsync_interrupt(TRUE);
			while(display_timing_ctrl1_reg.disp_frc_fsync==0)
			{
				//Eric@20180905 If other task change framerate, need break.
				if(judge_scaler_break_case(display) || (disp_smooth_variable_setting_flag == _ENABLE))
				{
					rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
					drivf_scaler_reset_freerun();
					break;
				}
				TimeOutCounter++;

				if(TimeOutCounter > TimeOutCounterThreshold)
				{
					rtd_pr_vbe_emerg("SU wait timeout again, re-run scaler process......\n");
					drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(FALSE);
					drvif_scaler_enable_frc2fsync_interrupt(FALSE);
					drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_timeout_handler();
					break;
				}
				msleep(0);
				display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
			}
		}
		if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI){
			if(drvif_i3ddma_triplebuf_flag())
			{
				rtd_pr_vbe_notice("[%s] hdmi does not enable vodma tracking i3 because frame rate frc case\n", __FUNCTION__);
			}
			else
			{
				rtd_pr_vbe_notice("[%s] enable vodma tracking i3.\n", __FUNCTION__);
				Scaler_Set_VODMA_Tracking_I3DDMA(TRUE);
			}
		}
		//drvif_scaler_enable_frc2fsync_interrupt(FALSE);
	}else
#endif
	{
		ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg);
		if(Get_DISPLAY_PANEL_OLED_TYPE()==1){
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + OLED_PANEL_DEN_PROTECT_PORCH;
		}
		else{
			if((get_panel_res() == PANEL_RES_FHD) || (get_panel_res() == PANEL_RES_HD))// PANEL_DEN_PROTECT_PORCH*Get_DISP_VERTICAL_TOTAL()/2250, KTASKWBS-22532
				ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH/2;
			else
				ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH;
		}
		if( ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target < Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN() )
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN();
		IoReg_Write32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg, ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue);
		//Do speedup vtotal process again
		drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(TRUE);
		drvif_scaler_enable_frc2fsync_interrupt(TRUE);
		// clear double buffer
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 0;		//D7
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

//#ifndef CONFIG_MEMC_BYPASS
	if(get_MEMC_bypass_status_refer_platform_model() == FALSE){
		//check remove/multiple vsync frame idx
		if(Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO) > 1){
			frame_idicator = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO)-1;
			dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
			while((dtg_frame_cnt1_reg.uzudtg_input_frame_cnt!= (frame_idicator-1))&& --count){
				dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
			}
			count = 0x3fffff;
			//make sure that we can have a full porch apply timing change settings
			while((dtg_frame_cnt1_reg.uzudtg_input_frame_cnt!= frame_idicator)&& --count){
				dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
			}
			if(count == 0)
				rtd_pr_vbe_notice("count timeout !!!\n");
			rtd_pr_vbe_notice("++frame_idicator:%d\n", (IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)&0x3f000000)>>24);
		}
	}
//#endif
		//force toggle, trigger local dimming vtotal update
		ppoverlay_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
		ppoverlay_dv_total_reg.dv_total = ppoverlay_dv_total_reg.dv_total+1;
		IoReg_Write32(PPOVERLAY_DV_total_reg, ppoverlay_dv_total_reg.regValue);
		ppoverlay_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
		ppoverlay_dv_total_reg.dv_total = ppoverlay_dv_total_reg.dv_total-1;
		IoReg_Write32(PPOVERLAY_DV_total_reg, ppoverlay_dv_total_reg.regValue);

		ppoverlay_dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
		ppoverlay_dh_total_last_line_length_reg.dh_total = Get_DISP_HORIZONTAL_TOTAL()-1;
		ppoverlay_dh_total_last_line_length_reg.dh_total_last_line = Get_DISP_HORIZONTAL_TOTAL()-DTG_HTOTAL_LASTLINE_DELAY;
		IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, ppoverlay_dh_total_last_line_length_reg.regValue);

#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
		display_timing_ctrl2_reg.frc2fsync_en = 1;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
#endif


		//set timing framesync
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
		display_timing_ctrl1_reg.disp_fix_last_line_new= 0;
		if(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())
			display_timing_ctrl1_reg.fsync_fll_mode= _DISABLE;
		else
			display_timing_ctrl1_reg.fsync_fll_mode= _ENABLE;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);

		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		if(display_timing_ctrl1_reg.disp_frc_fsync==1){
			drvif_scaler_enable_frc2fsync_interrupt(FALSE);
			drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(FALSE);
		}
		//check framesync status
		while(display_timing_ctrl1_reg.disp_frc_fsync==0)
		{
			//Eric@20180905 If other task change framerate, need break.
			if(judge_scaler_break_case(display) || (disp_smooth_variable_setting_flag == _ENABLE))
			{
				rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
				drivf_scaler_reset_freerun();
				break;
			}
			TimeOutCounter++;

			if(TimeOutCounter > TimeOutCounterThreshold)
			{
				IoReg_SetBits(SCALEDOWN_Measure_CTRL_reg, SCALEDOWN_Measure_CTRL_m_active_line_pixel_en_mask);
				//wait 1 frame to get the UZD timing measure result.
				Scaler_wait_for_input_one_frame(display);
				rtd_pr_vbe_emerg("SU wait timeout, run SpeedUP vtotal process again......\n");
				rtd_pr_vbe_emerg("UZD measure timing : (%x), UZD measure ctrl : (%x)\n", IoReg_Read32(SCALEDOWN_Measure_ActLine_Result_Main_Path_reg), IoReg_Read32(SCALEDOWN_Measure_CTRL_reg));
				rtd_pr_vbe_emerg("ONMS1 status : (%x), ONMS3 status : (%x)\n", IoReg_Read32(ONMS_onms1_status_reg), IoReg_Read32(ONMS_onms3_status_reg));
				rtd_pr_vbe_emerg("ONMS1 wdg en : (%x), ONMS3 wdg en : (%x)\n", IoReg_Read32(ONMS_onms1_watchdog_en_reg), IoReg_Read32(ONMS_onms3_watchdog_en_reg));
				//non full screen height timing fs fail case need disable frc2fsync by hw too
				//if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
				{
#if 0
					drvif_scaler_set_display_mode_frc2fsync_by_hw(_TRUE);
					rtd_pr_vbe_notice("SU LineBufferMode HW set frc2framesync fail, disable it\n");
#else
					SpeedUp_Timeout = 1;
#endif

				}
				break;
			}
			msleep(0);
			display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		}
		if(SpeedUp_Timeout == 1){
			TimeOutCounter = 0;
			//Do speedup vtotal process again
			drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(TRUE);
			drvif_scaler_enable_frc2fsync_interrupt(TRUE);
			while(display_timing_ctrl1_reg.disp_frc_fsync==0)
			{
				//Eric@20180905 If other task change framerate, need break.
				if(judge_scaler_break_case(display) || (disp_smooth_variable_setting_flag == _ENABLE))
				{
					rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
					drivf_scaler_reset_freerun();
					break;
				}
				TimeOutCounter++;

				if(TimeOutCounter > TimeOutCounterThreshold)
				{
					rtd_pr_vbe_emerg("SU wait timeout again, re-run scaler process......\n");
					drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(FALSE);
					drvif_scaler_enable_frc2fsync_interrupt(FALSE);
					drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_timeout_handler();
					break;
				}
				msleep(0);
				display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
			}
		}
		drvif_scaler_enable_frc2fsync_interrupt(FALSE);
		drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(FALSE);
	}
#if 0
	//[WOSQRTK-14508] calculate iv2dv delay after timing sync
#ifdef CONFIG_DATA_FRAMESYNC_FORCE_422_CAP
	//VO 4k timing use non-gatting flow, do not calculate iv2dv delay here
	if(!vbe_get_VDEC4K_run_datafs_without_gatting_condition()){
#endif
		//reset iv2dv delay
		ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg);
		ppoverlay_iv2dv_double_buffer_ctrl_reg.iv2dv_db_en = 0;
		IoReg_Write32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg, ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue);
		fs_iv_dv_fine_tuning5_reg.regValue = IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg);
#if 1   /*Verify ECN-032 frc2fsync reference iv2dv_done*/
		if(drv_memory_get_game_mode()){//if game mode case, need to use calculation result @Crixus 20170712

			if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC)){
				fs_iv_dv_fine_tuning5_reg.iv2dv_line = 2;
				fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = 2;
			}else{
				if((Get_DISPLAY_REFRESH_RATE() == 120) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) && (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())&& ( Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DISP_LEN)== _DISP_LEN)){
					if(vbe_disp_get_VRR_device_max_framerate()> 1190){ // 120hz
						id2dv_delay = drv_GameMode_decided_iv2dv_delay_new_mode();
					}
#if 0	//ECN:RL6672-2036 verify after ic back
					else if((vbe_disp_get_VRR_device_max_framerate()< 595) && (vbe_disp_get_VRR_device_max_framerate()> 605)){ // 60hz
						id2dv_delay = drv_GameMode_decided_iv2dv_delay_new_mode();
					}
#endif
					else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= _VRR_MIN_SUPPORT_FRAMERATE) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= (_VRR_MAX_SUPPORT_FRAMERATE-10))){
						id2dv_delay = (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_LEN)*(_VRR_MAX_SUPPORT_FRAMERATE-Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)))/_VRR_MAX_SUPPORT_FRAMERATE;
					}
					else{
						id2dv_delay = (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_LEN)*(_VRR_MAX_SUPPORT_FRAMERATE-_VRR_MIN_SUPPORT_FRAMERATE))/_VRR_MAX_SUPPORT_FRAMERATE;
					}
					fs_iv_dv_fine_tuning5_reg.iv2dv_line = id2dv_delay;
					fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = id2dv_delay;
					rtd_pr_vbe_emerg("[VRR condition][line:%d]iv2dv delay=%x \n",__LINE__, fs_iv_dv_fine_tuning5_reg.regValue);
				}else{
				if(IoReg_Read32(VGIP_VGIP_VBISLI_reg)&_BIT19){ //for debug only
					id2dv_delay = drv_GameMode_decided_iv2dv_delay_old_mode();
				}else{
					id2dv_delay = drv_GameMode_decided_iv2dv_delay_new_mode();
				}

				//if compression enable, need to add margin. @Crixus 20170605
				if(dvrif_memory_compression_get_enable(SLR_MAIN_DISPLAY))
					id2dv_delay = drv_GameMode_iv2dv_delay_compress_margin(id2dv_delay);

				if (id2dv_delay > Scaler_DispGetInputInfo(SLR_INPUT_V_LEN))
					id2dv_delay = Scaler_DispGetInputInfo(SLR_INPUT_V_LEN) - Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);

				if(id2dv_delay<8){
					id2dv_delay = 8;
					id2dv_delay = 8;
				}
				fs_iv_dv_fine_tuning5_reg.iv2dv_line = id2dv_delay;
				fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = id2dv_delay;
				}
			}
		}
		else{
#ifdef CONFIG_DATA_FRAMESYNC_FORCE_422_CAP
			//VO 4k timing use non-gatting flow, do not calculate iv2dv delay here
			if(!vbe_get_VDEC4K_run_datafs_without_gatting_condition())
#endif
			{
				if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC)){
					fs_iv_dv_fine_tuning5_reg.iv2dv_line = 2;
					fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = 2;
				}
				else{
					if(drv_memory_get_vdec_direct_low_latency_mode()){
						if(IoReg_Read32(VGIP_VGIP_VBISLI_reg)&_BIT19){ //for debug only
							id2dv_delay = drv_GameMode_decided_iv2dv_delay_old_mode();
						}else{
							id2dv_delay = drv_GameMode_decided_iv2dv_delay_new_mode();
						}

						if(dvrif_memory_compression_get_enable(SLR_MAIN_DISPLAY))
							id2dv_delay = drv_GameMode_iv2dv_delay_compress_margin(id2dv_delay);

						if (id2dv_delay > Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_LEN))
							id2dv_delay = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_LEN) - Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_STA_PRE);

						if(id2dv_delay<8){
							id2dv_delay = 8;
							id2dv_delay = 8;
						}
						fs_iv_dv_fine_tuning5_reg.iv2dv_line = id2dv_delay;
						fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = id2dv_delay;
					}
					else if((Get_DISPLAY_REFRESH_RATE() == 120) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) && (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())&& ( Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DISP_LEN)== _DISP_LEN)){
						if(vbe_disp_get_VRR_device_max_framerate()> 1190){ // 120hz
							id2dv_delay = drv_GameMode_decided_iv2dv_delay_new_mode();
						}
#if 0	//ECN:RL6672-2036 verify after ic back
						else if((vbe_disp_get_VRR_device_max_framerate()< 595) && (vbe_disp_get_VRR_device_max_framerate()> 605)){ // 60hz
							id2dv_delay = drv_GameMode_decided_iv2dv_delay_new_mode();
						}
#endif
						else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= _VRR_MIN_SUPPORT_FRAMERATE) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= (_VRR_MAX_SUPPORT_FRAMERATE-10))){
							id2dv_delay = (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_LEN)*(_VRR_MAX_SUPPORT_FRAMERATE-Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)))/_VRR_MAX_SUPPORT_FRAMERATE;
						}
						else{
							id2dv_delay = (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_LEN)*(_VRR_MAX_SUPPORT_FRAMERATE-_VRR_MIN_SUPPORT_FRAMERATE))/_VRR_MAX_SUPPORT_FRAMERATE;
						}
						fs_iv_dv_fine_tuning5_reg.iv2dv_line = id2dv_delay;
						fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = id2dv_delay;
						rtd_pr_vbe_notice("[VRR condition][line:%d]iv2dv delay=%x \n",__LINE__, id2dv_delay);
					}
					else if((Get_DISPLAY_REFRESH_RATE() == 60) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) && (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())){
						id2dv_delay = drv_GameMode_decided_iv2dv_delay_new_mode();
						fs_iv_dv_fine_tuning5_reg.iv2dv_line = id2dv_delay;
						fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = id2dv_delay;
					}
					else{
						fs_iv_dv_fine_tuning5_reg.iv2dv_line = 2;
						fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = 2;
					}
				}
			}
		}
#else
		fs_iv_dv_fine_tuning5_reg.iv2dv_line = 1;
		fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = 1;
#endif
		rtd_outl(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg, fs_iv_dv_fine_tuning5_reg.regValue);
		rtd_pr_vbe_notice("[%s]iv2dv_delay:%d\n", __FUNCTION__, PPOVERLAY_FS_IV_DV_Fine_Tuning5_get_iv2dv_line(IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg)));
#ifdef CONFIG_DATA_FRAMESYNC_FORCE_422_CAP
	}
#endif

#endif //no need to change iv2dv delay after set timing sync KTASKWBS-19599
	rtd_pr_vbe_notice("SU timeout = %d\n", TimeOutCounter);
	return;
}

#define VODMA_1_PIXEL_OFFSET 5
#define VODMA_1_PIXEL_30HZ_OFFSET 1
#define VODMA_1_PIXEL_24HZ_OFFSET 2
#define VODMA_2_PIXEL_OFFSET 0
#define VODMA_2_PIXEL_30HZ_OFFSET 4
#define VODMA_2_PIXEL_24HZ_OFFSET 5
#define VODMA_2_PIXEL_NEW_OFFSET 6
#define VODMA_2_PIXEL_30HZ_NEW_OFFSET 5
#define VODMA_OLED_RESET_TARGET_OFFSET 4
#define VODMA_HD_RESET_TARGET_OFFSET 2
#define VODMA_IV2DV_0_RESET_TARGET_OFFSET 8

#define VODMA_IV2DV_0_RESET_TARGET_PQ_DELAY 2
#define VODMA_IV2DV_0_RESET_TARGET_CP_OFFSET 4
#define VODMA_IV2DV_0_RESET_TARGET_CP_RPL_OFFSET 7

unsigned int VSYNCGEN_get_vodma_clk_0_max(void)
{
    unsigned int vodmapll;
    unsigned int nMCode, nNCode, nOCode, regValue, busclk = 0;
    unsigned int divider_num = 0;

    vodma_vodma_clkgen_RBUS vodma_clkgen_reg;
    vodma_clkgen_reg.regValue=rtd_inl(VODMA_VODMA_CLKGEN_reg);


    regValue = rtd_inl(PLL_REG_SYS_PLL_VODMA1_reg);

    nMCode = ((regValue & 0x000ff000)>>12);
    nNCode = ((regValue & 0x00300000)>>20);
    nOCode = ((regValue & 0x00000180)>>7);

    busclk = 27000000*(nMCode+2)/(nNCode+1)/(nOCode+1);

    divider_num = vodma_clkgen_reg.vodma_clk_div_n;

    vodmapll = busclk / (divider_num+1);

    if(vodma_clkgen_reg.vodma_clk_div2_en){//interlace should div2
        vodmapll = vodmapll / 2;
        rtd_printk(KERN_NOTICE, TAG_NAME_VBE,"vo clk div2 for interlace timing\n");
    }
    rtd_printk(KERN_NOTICE, TAG_NAME_VBE,"[vo] vsyncgen busclk: %d/%d/%d, divider_num=%d\n", nMCode, nNCode, busclk, divider_num);

    return vodmapll;
}

void drvif_scaler_set_Vodma_Delay_Target(unsigned char bEnable)
{
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	ppoverlay_uzudtg_control1_RBUS ppoverlay_uzudtg_control1_reg;
    ppoverlay_srnn_control_RBUS srnn_control_reg;
	unsigned int u32_delay_target = 0;
	unsigned int b_2pixel_mode = (Get_DISPLAY_REFRESH_RATE()>=120)?2:1;

	ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	ppoverlay_uzudtg_control1_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control1_reg);
    srnn_control_reg.regValue = IoReg_Read32(PPOVERLAY_srnn_control_reg);

	if(bEnable){
		unsigned int dclk_frac_a = SYS_REG_SYS_DISPCLKSEL_get_dclk_fract_a(IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg))+1;
		unsigned int dclk_frac_b = SYS_REG_SYS_DISPCLKSEL_get_dclk_fract_b(IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg))+1;
		unsigned int frame_sync_den_target = PPOVERLAY_frc_to_frame_sync_den_end_ctrl_get_dv_den_cnt_target(IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg));

		u32_delay_target = (frame_sync_den_target*dclk_frac_a)/(dclk_frac_b*b_2pixel_mode);

		//adjust offset to make the input vsync drop to 2237 line ((2225 + 2250) / 2 = 2237)
		if(b_2pixel_mode == 1){
			if(((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245)))
			{	//24Hz
				u32_delay_target -= VODMA_1_PIXEL_24HZ_OFFSET;
			}
			else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305))
			{	//30Hz
				u32_delay_target -= VODMA_1_PIXEL_30HZ_OFFSET;
			}
			else
			{
				u32_delay_target += VODMA_1_PIXEL_OFFSET;
			}
		}
		else{
			if(((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245)))
			{	//24Hz
				u32_delay_target -= VODMA_2_PIXEL_24HZ_OFFSET;
			}
			else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305))
			{	//30Hz
				u32_delay_target -= VODMA_2_PIXEL_30HZ_OFFSET;
			}
			else
			{
				u32_delay_target -= VODMA_2_PIXEL_OFFSET;
			}
		}
		rtd_pr_vbe_notice("u32_delay_target:%d, frame_sync_den_target:%d,frac_a:%d,frac_b:%d\n",
			u32_delay_target, frame_sync_den_target, dclk_frac_a,dclk_frac_b);
	}else{
		u32_delay_target = 0;
	}

	ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line = u32_delay_target;
	IoReg_Write32(PPOVERLAY_uzudtg_control1_reg, ppoverlay_uzudtg_control1_reg.regValue);

    srnn_control_reg.dtgm2mdom_pre_vs_line = ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line;
    IoReg_Write32(PPOVERLAY_srnn_control_reg, srnn_control_reg.regValue);

	if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245)){
		rtd_pr_vbe_notice("[%s]wait for uzudelay apply.\n",__FUNCTION__);
		//Scaler_wait_for_frc2fs_hw_lock_done(SLR_MAIN_DISPLAY);
		Scaler_wait_for_LCM_done();
	}
	rtd_pr_vbe_notice("bendbg: uzuline:0x%x reg:0x%x\n",
		ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line, IoReg_Read32(PPOVERLAY_uzudtg_control1_reg));
}

#ifdef CONFIG_RESET_VODMA_BY_NEW_MODE
#ifdef CONFIG_RESET_VODMA_BY_MEASURE
void drvif_scaler_set_Vodma_reset_target_line_count(void)
{
    ppoverlay_dtg_vo_lc_RBUS ppoverlay_dtg_vo_lc_reg;
    vodma_vodma_ivs_distance_RBUS vodma_vodma_ivs_distance_reg;
    vgip_data_path_select_RBUS vgip_data_path_select_reg;
    vgip_data_path_select_RBUS reserved_vgip_data_path_select_reg;
    unsigned int ivs_distance_measure_result = 0;

    unsigned int dtg_vo_vln = 0;
    unsigned int offset = 0;
    unsigned int reset_target = 0;
    unsigned int b_2pixel_mode = (Get_DISPLAY_REFRESH_RATE()>=120)?2:1;
    unsigned int dclk_frac_a = SYS_REG_SYS_DISPCLKSEL_get_dclk_fract_a(IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg))+1;
    unsigned int dclk_frac_b = SYS_REG_SYS_DISPCLKSEL_get_dclk_fract_b(IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg))+1;
    unsigned int frame_sync_den_target = PPOVERLAY_frc_to_frame_sync_den_end_ctrl_get_dv_den_cnt_target(IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg));
    unsigned int mDtg_rem = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO);
    unsigned int mDtg_mul = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO);
    unsigned int pnlVTotal = Get_DISP_VERTICAL_TOTAL();
    unsigned int dHTotal = PPOVERLAY_DH_Total_Last_Line_Length_get_dh_total(IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg))+1;
    unsigned int dVtotal = PPOVERLAY_DV_total_get_dv_total(IoReg_Read32(PPOVERLAY_DV_total_reg))+1;
    unsigned int uzuHTotal = PPOVERLAY_uzudtg_DH_TOTAL_get_uzudtg_dh_total(IoReg_Read32(PPOVERLAY_uzudtg_DH_TOTAL_reg))+1;
    unsigned int uzuVTotal = PPOVERLAY_uzudtg_DV_TOTAL_get_uzudtg_dv_total(IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg))+1;
    unsigned int uzuClk = (Get_DISPLAY_CLOCK_TYPICAL() / dclk_frac_b) * dclk_frac_a;
    int timeout = 100;

    if(vbe_disp_decide_uzuclock_mapping_vo()){
        if((Scaler_DispGetInputInfoByDisp(0,SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfoByDisp(0,SLR_INPUT_V_FREQ) < 245))
        {
            //DTG_M 594M / uzudtg 240M, frc2fsync is refer to DTG_M, so we need  adjust den end position.
            // ((2160+45+10)/2250)*42ms = 21ms + (x/2812)*21ms
            // x = 2700
            frame_sync_den_target = Get_DISP_DEN_END_VPOS() + 500;
        }
        else if((Scaler_DispGetInputInfoByDisp(0,SLR_INPUT_V_FREQ) > 245) && (Scaler_DispGetInputInfoByDisp(0,SLR_INPUT_V_FREQ) < 255))
        {
            //DTG_M 594M / uzudtg 250M, frc2fsync is refer to DTG_M, so we need adjust den end position.
            // ((2160+45+10)/2250)*40ms = 20ms + (x/2700)*20ms
            // x = 2616
            frame_sync_den_target = Get_DISP_DEN_END_VPOS() + 405;
        }
        pr_notice("[voreset]vbe_disp_decide_uzuclock_mapping_vo, frame_sync_den_target = %d", frame_sync_den_target);
    }
    reset_target = (((dVtotal * (mDtg_mul-1) + frame_sync_den_target)*dHTotal*dclk_frac_a)/(dclk_frac_b*b_2pixel_mode*uzuHTotal)) - ((mDtg_rem-1)*uzuVTotal);

    reserved_vgip_data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg); // reserve vgip_data_path_select_reg

    vgip_data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);
    vgip_data_path_select_reg.ch1tovo1_vsync_sel = 0; // main path measure to sdnr out
    IoReg_Write32(VGIP_Data_Path_Select_reg, vgip_data_path_select_reg.regValue);

    vodma_vodma_ivs_distance_reg.regValue = IoReg_Read32(VODMA_vodma_ivs_distance_reg);
    vodma_vodma_ivs_distance_reg.ivs_distance_meas_en = 0;
    IoReg_Write32(VODMA_vodma_ivs_distance_reg, vodma_vodma_ivs_distance_reg.regValue); // clear ivs_distance_result
    rtd_pr_ddomain_isr_emerg("[voreset]Clear ivs_distance_result (%x)", IoReg_Read32(VODMA_vodma_ivs_distance_reg));

    vodma_vodma_ivs_distance_reg.ivs_distance_meas_en = 1;
    vodma_vodma_ivs_distance_reg.ivs_distance_meas_src_sel = 0; // main ivs
    IoReg_Write32(VODMA_vodma_ivs_distance_reg, vodma_vodma_ivs_distance_reg.regValue); // enable to measure ivs_distance
    rtd_pr_ddomain_isr_emerg("[voreset]Enable ivs_distance measure (%x)", IoReg_Read32(VODMA_vodma_ivs_distance_reg));

    vodma_vodma_ivs_distance_reg.regValue = IoReg_Read32(VODMA_vodma_ivs_distance_reg);
    ivs_distance_measure_result = vodma_vodma_ivs_distance_reg.ivs_distance_xclk_result;
    while(!ivs_distance_measure_result && (timeout--)) // wait measure result appear
    {
        usleep_range(1000, 1000);// msleep(1);
        vodma_vodma_ivs_distance_reg.regValue = IoReg_Read32(VODMA_vodma_ivs_distance_reg);
        ivs_distance_measure_result = vodma_vodma_ivs_distance_reg.ivs_distance_xclk_result;
    }
    if(timeout == 0)
    {
        rtd_pr_ddomain_isr_emerg("[voreset][Error]ivs_distance measure timeout !!");
    }

	/*
	**** offset = Convert to from vo free clk to uzu line ****
		Assume dclk=594M, frac_a=1, frac_b=1 (uzuclk=594M), vo freerun clk=27M, ivs_distance_measure_result = 1000
		Convert to uzuclk: 1000 * (594/27) = 22000	(uzuclk)
		Assume uzudtg htotal = 4400
		22000/4400 = 5 (line)
	*/
	offset = ((ivs_distance_measure_result * (uzuClk / 27000000)) / uzuHTotal); // convert vo period to uzu vline

    dtg_vo_vln = reset_target - offset;

    ppoverlay_dtg_vo_lc_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_VO_LC_reg);
    ppoverlay_dtg_vo_lc_reg.dtg_vo_src_sel = 1; //uzu_line_cnt
    ppoverlay_dtg_vo_lc_reg.dtg_vo_vln = dtg_vo_vln;
    IoReg_Write32(PPOVERLAY_DTG_VO_LC_reg, ppoverlay_dtg_vo_lc_reg.regValue);
    rtd_pr_vsc_notice("[voreset]dtg_vo_vln=%d,reset_target=%d,offset=%d,ivs_distance_measure_result=%d", dtg_vo_vln, reset_target, offset, ivs_distance_measure_result);
    rtd_pr_vsc_notice("[voreset]frame_sync_den_target=%d,frac_a=%d,frac_b=%d,2px=%d,mDtg_rem=%d,mDtg_mul=%d,pnlVTotal=%d,dVtotal=%d,dHTotal=%d,uzuVTotal=%d,uzuHTotal=%d,uzuClk=%d\n",
        frame_sync_den_target, dclk_frac_a, dclk_frac_b, b_2pixel_mode, mDtg_rem, mDtg_mul, pnlVTotal, dVtotal, dHTotal, uzuVTotal, uzuHTotal, uzuClk);

    vgip_data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);
    vgip_data_path_select_reg.ch1tovo1_vsync_sel = reserved_vgip_data_path_select_reg.ch1tovo1_vsync_sel; // restore vgip_data_path_select_reg.ch1tovo1_vsync_sel setting
    IoReg_Write32(VGIP_Data_Path_Select_reg, vgip_data_path_select_reg.regValue);
}
#else
void drvif_scaler_set_Vodma_reset_target_line_count(OLED_RESET_TARGET_CASE resetCase)
{
    ppoverlay_dtg_vo_lc_RBUS ppoverlay_dtg_vo_lc_reg;
    unsigned int dvtotal = PPOVERLAY_DV_total_get_dv_total(IoReg_Read32(PPOVERLAY_DV_total_reg));
    unsigned int frame_sync_den_target = PPOVERLAY_frc_to_frame_sync_den_end_ctrl_get_dv_den_cnt_target(IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg));
    unsigned int frac_a = SYS_REG_SYS_DISPCLKSEL_get_dclk_fract_a(IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg))+1;
    unsigned int frac_b = SYS_REG_SYS_DISPCLKSEL_get_dclk_fract_b(IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg))+1;
    unsigned int b_2pixel_mode = (Get_DISPLAY_REFRESH_RATE()>=120)?2:1;
    unsigned int offset = 0;
    unsigned int reset_target = 0;
    unsigned int reset_target_offset = 0;
    unsigned int vodmapll = VSYNCGEN_get_vodma_clk_0_max();
    unsigned int pixelClk = 0;
    SLR_VOINFO* pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
    int reset_target_clk_ratio = 0;
    int reset_target_clk_offset = 0;
    if(pVOInfo && pVOInfo->pixel_clk)
    {
        pixelClk = pVOInfo->pixel_clk;
        reset_target_clk_ratio = vodmapll / (pixelClk / 1000); // 0.001
        reset_target_clk_offset = ((VODMA_IV2DV_0_RESET_TARGET_PQ_DELAY * reset_target_clk_ratio) / 1000);
    }
    else
    {
        rtd_pr_vsc_notice("[voreset][Error] pVOInfo or pixel_clk null");
    }

    offset = ((dvtotal-frame_sync_den_target)*frac_a)/(frac_b*b_2pixel_mode);
    reset_target = dvtotal-offset;
    if(vbe_disp_decide_uzuclock_mapping_vo()){
        reset_target = frame_sync_den_target;
    }

    if(resetCase != OLED_RESET_TARGET_CASE_NONE)
    {
        switch (resetCase)
        {
            case OLED_RESET_TARGET_CASE_CP:
            case OLED_RESET_TARGET_CASE_NONCP:
            case OLED_RESET_TARGET_CASE_HDMI:
            case NON_OLED_RESET_TARGET_CASE_HDMI:
            case NON_OLED_RESET_TARGET_CASE_CP:
            {
                reset_target_offset = VODMA_IV2DV_0_RESET_TARGET_CP_OFFSET + reset_target_clk_offset;
            }
            break;
            case OLED_RESET_TARGET_CASE_CP_RPL:
            case NON_OLED_RESET_TARGET_CASE_CP_RPL:
            {
                reset_target_offset = VODMA_IV2DV_0_RESET_TARGET_CP_RPL_OFFSET + reset_target_clk_offset;
            }
            break;
            default:
            break;
        }
        rtd_pr_vsc_notice("[voreset]resetCase=%d org_reset_target=%d, reset_target_offset=%d, clk_offset=%d, vodmapll=%d, pixel_clk=%d\n", resetCase, reset_target, reset_target_offset, reset_target_clk_offset, vodmapll, pixelClk);
        reset_target -= reset_target_offset;
    }
    else
    {
        if(b_2pixel_mode == 2){
            if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305))
            {
                reset_target -= VODMA_2_PIXEL_30HZ_NEW_OFFSET;
            }else{
                reset_target -= VODMA_2_PIXEL_NEW_OFFSET;
            }
        }
        if((Get_DISPLAY_PANEL_OLED_TYPE()==1) && (Get_DISPLAY_REFRESH_RATE()==60))
            reset_target -= VODMA_OLED_RESET_TARGET_OFFSET;// since oled use larger dv_den_cnt_target, need to reduce reset target to avoid time out
        else if(get_panel_res() == PANEL_RES_HD){
            reset_target += VODMA_HD_RESET_TARGET_OFFSET;
        }
    }
    ppoverlay_dtg_vo_lc_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_VO_LC_reg);
    ppoverlay_dtg_vo_lc_reg.dtg_vo_src_sel = 1; //uzu_line_cnt
    ppoverlay_dtg_vo_lc_reg.dtg_vo_vln = reset_target;
    IoReg_Write32(PPOVERLAY_DTG_VO_LC_reg, ppoverlay_dtg_vo_lc_reg.regValue);
    rtd_pr_vsc_notice("[voreset] frac_a:%d, frac_b:%d, b_2pixel_mode:%d, frame_sync_den_target:%d \n", frac_a, frac_b,b_2pixel_mode,frame_sync_den_target);
    rtd_pr_vsc_notice("[voreset] offset:%d, reset target:%d \n", offset, reset_target);
}
#endif
#endif

void drvif_scaler_run_Reset_VODMA(void)
{
	UINT32 TimeOutCounterThreshold = 0;
	UINT32 TimeOutCounter = 0;
	int ret =0;
	rtd_pr_vsc_notice("drvif_scaler_run_Reset_VODMA \n");

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_SET_VO_FREERUN,0,1)))//set vo free run
	{
		rtd_pr_vsc_err("ret=%d, SCALERIOC_SET_VO_FREERUN RPC fail !!!\n", ret);
	}

	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI){
		drv_I3ddmaVodma_GameMode_iv2pv_delay(FALSE);
		framesync_set_vtotal_by_vodma_freerun_period();
	}
   	TimeOutCounterThreshold = 200;
	Scaler_Set_Reset_VODMA_Flag(_ENABLE);
	while(Scaler_Get_Reset_VODMA_Flag())
	{
	    usleep_range(1000, 1000);// msleep(1);
	    TimeOutCounter++;

	    if(TimeOutCounter > TimeOutCounterThreshold)
	    {
            rtd_pr_vbe_emerg("VODMA reset flow wait timeout......(%d/%d,break=%d)\n", TimeOutCounter , TimeOutCounterThreshold , judge_scaler_break_case(SLR_MAIN_DISPLAY));
            Scaler_Set_Reset_VODMA_DisableEnable_Cancel_Flag(_ENABLE);
	        break;
	    }
	}
	Scaler_Set_Reset_VODMA_Flag(_DISABLE);
}
//due to VO clk affects delay for DTG
//shorten DTG LastLine to make UZU period smaller than VO period,
//let VO to catch up in front porch
#define DTG_HTOTAL_LASTLINE_RATIO 64
#define DTG_HTOTAL_LASTLINE_RATIO_CP 5
#define DTG_HTOTAL_LASTLINE_RATIO_CP_MUL 3


//unsigned int speedup_rerun_resetvo_by_90K = 0;
void drvif_scaler_set_display_mode_frc2fsync_by_Reset_VODMA(void)
{
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
//	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	unsigned long flags;//for spin_lock_irqsave
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	ppoverlay_frc_to_frame_sync_den_end_ctrl_RBUS ppoverlay_frc_to_frame_sync_den_end_ctrl_reg;
	ppoverlay_uzudtg_control3_RBUS ppoverlay_uzudtg_control3_reg;

#ifndef CONFIG_RESET_VODMA_BY_MEASURE
    ppoverlay_dh_total_last_line_length_RBUS ppoverlay_dh_total_last_line_length_reg;
    unsigned int dh_total_last_line_ratio = 1;
    unsigned int dh_total_last_line_ratio_mul = 1;
    OLED_RESET_TARGET_CASE resetCase;
    bool bUseIv2dv0 = FALSE;
#endif

	unsigned char display;
	UINT32 TimeOutCounterThreshold = Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut();
	UINT32 TimeOutCounter = 0;
//	UINT32 count = 20;
	extern unsigned judge_scaler_break_case(unsigned char display);
	UINT32 temp_align_mode =0;
	unsigned char SpeedUp_Timeout = 0;
	unsigned int db_timeout_count = 0xff;

	display = SLR_MAIN_DISPLAY;//Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
    rtd_pr_vbe_emerg("drvif_scaler_set_display_mode_frc2fsync_by_Reset_VODMA\n");

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
	IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);
	ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg);
	if(Get_DISPLAY_PANEL_OLED_TYPE()==1)
            ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS()+OLED_PANEL_DEN_PROTECT_PORCH;
	else if(get_panel_res() == PANEL_RES_FHD)// PANEL_DEN_PROTECT_PORCH*Get_DISP_VERTICAL_TOTAL()/2250, KTASKWBS-22532
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH/2;
	else if(get_panel_res() == PANEL_RES_HD)// PANEL_DEN_PROTECT_PORCH*Get_DISP_VERTICAL_TOTAL()/2250, KTASKWBS-22532
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH/3;
    else {
        ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH;
        if(Get_DISP_VERTICAL_TOTAL() - Get_DISP_DEN_END_VPOS() < 45)
        {
           ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH / 2;
        }
	}
	if( ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target < Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN() )
		ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN();
	IoReg_Write32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg, ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue);

#ifdef CONFIG_RESET_VODMA_BY_MEASURE

#else
	ppoverlay_dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
	ppoverlay_dh_total_last_line_length_reg.dh_total = Get_DISP_HORIZONTAL_TOTAL()-1;
    resetCase = Get_OledResetTargetCase();
    if(resetCase != OLED_RESET_TARGET_CASE_NONE)
    {
        bUseIv2dv0 = TRUE;
        switch (resetCase) // Set last_line_ratio
        {
            case OLED_RESET_TARGET_CASE_CP:
            case OLED_RESET_TARGET_CASE_CP_RPL:
            case OLED_RESET_TARGET_CASE_NONCP:
            case OLED_RESET_TARGET_CASE_HDMI:
            case NON_OLED_RESET_TARGET_CASE_HDMI:
            case NON_OLED_RESET_TARGET_CASE_CP:
            case NON_OLED_RESET_TARGET_CASE_CP_RPL:
            {
                dh_total_last_line_ratio =  DTG_HTOTAL_LASTLINE_RATIO_CP;
                dh_total_last_line_ratio_mul = DTG_HTOTAL_LASTLINE_RATIO_CP_MUL;
            }
            break;
            default:
            {
                bUseIv2dv0 = FALSE;
            }
            break;
        }

        if(bUseIv2dv0)
        {
            ppoverlay_dh_total_last_line_length_reg.dh_total_last_line = ((Get_DISP_HORIZONTAL_TOTAL() / dh_total_last_line_ratio) * dh_total_last_line_ratio_mul) - DTG_HTOTAL_LASTLINE_DELAY;
        }
        else
        {
            ppoverlay_dh_total_last_line_length_reg.dh_total_last_line = Get_DISP_HORIZONTAL_TOTAL() - DTG_HTOTAL_LASTLINE_DELAY;
        }

        rtd_pr_vsc_notice("[voreset]Set dh_total_last_line %d, bUseIv2dv0=%d, dh_total_last_line_ratio=%d mul=%d resetCase=%d\n", ppoverlay_dh_total_last_line_length_reg.dh_total_last_line, bUseIv2dv0, dh_total_last_line_ratio, dh_total_last_line_ratio_mul, resetCase);
    }
    else
    {
        ppoverlay_dh_total_last_line_length_reg.dh_total_last_line = (Get_DISP_HORIZONTAL_TOTAL() / DTG_HTOTAL_LASTLINE_RATIO) - DTG_HTOTAL_LASTLINE_DELAY;
    }
	IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, ppoverlay_dh_total_last_line_length_reg.regValue);
#endif

	drvif_scaler_set_display_mode_frc2fsync_I2D_disable();
	drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(FALSE);
	//drvif_scaler_enable_frc2fsync_interrupt(1);

#ifdef CONFIG_RESET_VODMA_BY_MEASURE
    framesync_set_iv2dv_delay(DISPLAY_MODE_RESET_VODMA);
#else
	//iv2dv need set 2 before reset vodma
    if(bUseIv2dv0)
    {
        //iv2dv need set 0 before reset vodma
        framesync_set_iv2dv_delay(DISPLAY_MODE_RESET_VODMA);
    }
    else
    {
        //iv2dv need set 2 before reset vodma
        framesync_set_iv2dv_delay(DISPLAY_MODE_FRC);
    }
#endif

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 1;		//D7
	ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	//wait for 1 dvs to make iv2dv delay change, avoid dvs short in 50Hz
	while(ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set && --db_timeout_count){
		usleep_range(1000, 1000); // msleep(0);
		ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	}
	if(db_timeout_count == 0)
		rtd_pr_vbe_notice("[%s] wait for D7 timeout!!\n", __FUNCTION__);

	// clear double buffer
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 0;		//D7
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/


	//frc2fsync protect
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	display_timing_ctrl2_reg.frc2fsync_en = 1;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
#if 0
	//enable timing fsync
	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
	display_timing_ctrl1_reg.disp_fix_last_line_new= 0;
	display_timing_ctrl1_reg.fsync_fll_mode= _ENABLE;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
#endif

#ifdef CONFIG_RESET_VODMA_BY_NEW_MODE
#ifdef CONFIG_RESET_VODMA_BY_MEASURE
    drvif_scaler_set_Vodma_reset_target_line_count();
#else
	drvif_scaler_set_Vodma_reset_target_line_count(resetCase);
#endif
#else
	Scaler_MEMC_output_force_bg_enable(TRUE,__func__,__LINE__); //use memc force bg for change uzu delay garbage

	drvif_scaler_set_Vodma_Delay_Target(TRUE);

	if(Get_DISPLAY_PANEL_OLED_TYPE()==1){
		// oled frc2fsync target is less than normal panel, so need increase front porch
		drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(TRUE);
		drvif_scaler_enable_frc2fsync_interrupt(TRUE);
	}
#endif
//	speedup_rerun_resetvo_by_90K = (90000*10/Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ));//*3/4; //3/4 frame time
	ppoverlay_uzudtg_control3_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control3_reg);
	//record align mode
	temp_align_mode = ppoverlay_uzudtg_control3_reg.align_output_timing;
	//set align mode input for mv_end line count
	ppoverlay_uzudtg_control3_reg.align_output_timing = 0;
	IoReg_Write32(PPOVERLAY_uzudtg_control3_reg,ppoverlay_uzudtg_control3_reg.regValue);
	drvif_scaler_run_Reset_VODMA();

	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

	//check framesync status
	while(display_timing_ctrl1_reg.disp_frc_fsync==0)
	{
		//Eric@20180905 If other task change framerate, need break.
		if(judge_scaler_break_case(display) || (disp_smooth_variable_setting_flag == _ENABLE))
		{
			rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
			drivf_scaler_reset_freerun();
			break;
		}
		TimeOutCounter++;

		if(TimeOutCounter > TimeOutCounterThreshold)
		{
			rtd_pr_vbe_emerg("[frc2fsync_by_reset_vodma]error wait timeout!!!\n");

			IoReg_SetBits(SCALEDOWN_Measure_CTRL_reg, SCALEDOWN_Measure_CTRL_m_active_line_pixel_en_mask);
			//wait 1 frame to get the UZD timing measure result.
			Scaler_wait_for_input_one_frame(display);
			rtd_pr_vbe_emerg("SU wait timeout, run SpeedUP vtotal process again......\n");
			rtd_pr_vbe_emerg("UZD measure timing : (%x), UZD measure ctrl : (%x)\n", IoReg_Read32(SCALEDOWN_Measure_ActLine_Result_Main_Path_reg), IoReg_Read32(SCALEDOWN_Measure_CTRL_reg));
			rtd_pr_vbe_emerg("ONMS1 status : (%x), ONMS3 status : (%x)\n", IoReg_Read32(ONMS_onms1_status_reg), IoReg_Read32(ONMS_onms3_status_reg));
			rtd_pr_vbe_emerg("ONMS1 wdg en : (%x), ONMS3 wdg en : (%x)\n", IoReg_Read32(ONMS_onms1_watchdog_en_reg), IoReg_Read32(ONMS_onms3_watchdog_en_reg));
			//non full screen height timing fs fail case need disable frc2fsync by hw too
			//if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
			{
#if 0
				drvif_scaler_set_display_mode_frc2fsync_by_hw(_TRUE);
				rtd_pr_vbe_notice("SU LineBufferMode HW set frc2framesync fail, disable it\n");
#else
				SpeedUp_Timeout = 1;
#endif
			}
			break;
		}
		usleep_range(1000, 1000); // msleep(0);
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	}
	//recovery align mode
	ppoverlay_uzudtg_control3_reg.align_output_timing = temp_align_mode;
	IoReg_Write32(PPOVERLAY_uzudtg_control3_reg,ppoverlay_uzudtg_control3_reg.regValue);
	rtd_pr_vbe_notice("[frc2fsync_by_reset_vodma]recovery align mode = %d\n", temp_align_mode);

#ifdef CONFIG_RESET_VODMA_BY_NEW_MODE

#else
	drvif_scaler_set_Vodma_Delay_Target(FALSE);
#endif

	//Scaler_MEMC_output_force_bg_enable(FALSE,__func__,__LINE__); //disable memc forcebg on scaler forcebg = 0;

	if(SpeedUp_Timeout == 1){
		//enable timing fsync
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
		display_timing_ctrl1_reg.disp_fix_last_line_new= 0;
		display_timing_ctrl1_reg.fsync_fll_mode= _ENABLE;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
		TimeOutCounter = 0;
		drvif_scaler_enable_frc2fsync_HW_speedup_vtotal_by_I2D(FALSE);
		ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg);
        if(Get_DISPLAY_PANEL_OLED_TYPE()==1)
            ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS()+OLED_PANEL_DEN_PROTECT_PORCH;
        else if((get_panel_res() == PANEL_RES_FHD) || (get_panel_res() == PANEL_RES_HD))// PANEL_DEN_PROTECT_PORCH*Get_DISP_VERTICAL_TOTAL()/2250, KTASKWBS-22532
            ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH/2;
        else
            ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH;
		if( ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target < Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN() )
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN();
		IoReg_Write32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg, ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue);
		//Do speedup vtotal process again
		drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(TRUE);
		drvif_scaler_enable_frc2fsync_interrupt(TRUE);
		while(display_timing_ctrl1_reg.disp_frc_fsync==0)
		{
			//Eric@20180905 If other task change framerate, need break.
			if(judge_scaler_break_case(display) || (disp_smooth_variable_setting_flag == _ENABLE))
			{
				rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
				drivf_scaler_reset_freerun();
				break;
			}
			TimeOutCounter++;

			if(TimeOutCounter > TimeOutCounterThreshold)
			{
				rtd_pr_vbe_emerg("SU wait timeout again, re-run scaler process......\n");
				drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(FALSE);
				drvif_scaler_enable_frc2fsync_interrupt(FALSE);
				drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_timeout_handler();
				break;
			}
			msleep(0);
			display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		}
	}

#ifdef CONFIG_RESET_VODMA_BY_MEASURE

#else
    if(bUseIv2dv0)
    {
        ppoverlay_dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
        ppoverlay_dh_total_last_line_length_reg.dh_total_last_line = Get_DISP_HORIZONTAL_TOTAL() - DTG_HTOTAL_LASTLINE_DELAY;
        IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, ppoverlay_dh_total_last_line_length_reg.regValue);
        rtd_pr_vbe_emerg("[voreset]Restore dh_total_last_line %d\n", ppoverlay_dh_total_last_line_length_reg.dh_total_last_line);
    }
#endif
	// iv2dv need set after reset vodma
	framesync_set_iv2dv_delay(DISPLAY_MODE_FRAME_SYNC);

	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI){
		if(drvif_i3ddma_triplebuf_flag())
		{
			rtd_pr_vbe_notice("[%s] hdmi does not enable vodma tracking i3 because frame rate frc case\n", __FUNCTION__);
		}
		else
		{
			rtd_pr_vbe_notice("[%s] enable vodma tracking i3.\n", __FUNCTION__);
             #ifndef BUILD_QUICK_SHOW
			Scaler_Set_VODMA_Tracking_I3DDMA(TRUE);
		        scaler_set_VoTrackingI3DDMA_frequency_update_in_isr(TRUE);
                #endif
		}
	}

	//rtd_pr_vbe_notice("[FRC2FS] Line_cnt0 = %d,%d,%d,%d\n", vo_lineCnt0, mtg_lineCnt0,uzu_lineCnt0,memc_lineCnt0);
//rtd_pr_vbe_notice("[FRC2FS] Line_cnt1 = %d,%d,%d,%d\n", vo_lineCnt1, mtg_lineCnt1,uzu_lineCnt1,memc_lineCnt1);
	rtd_pr_vbe_notice("SU timeout = %d\n", TimeOutCounter);
}


void drvif_scaler_set_display_mode_frc2fsync_I2D_disable(void)
{
	ppoverlay_i2d_ctrl_0_RBUS ppoverlay_i2d_ctrl_0_reg;
	//disable I2D speed up by hw
	rtd_pr_vsc_notice("drvif_scaler_set_display_mode_frc2fsync_I2D_disable\n");
	ppoverlay_i2d_ctrl_0_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_CTRL_0_reg);
	ppoverlay_i2d_ctrl_0_reg.i2d_special_dly_en = 0;
	IoReg_Write32(PPOVERLAY_I2D_CTRL_0_reg, ppoverlay_i2d_ctrl_0_reg.regValue);

}

void drvif_scaler_set_display_mode_frc2fsync_by_I2D_CTRL(void)
{
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
//	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	ppoverlay_dh_total_last_line_length_RBUS ppoverlay_dh_total_last_line_length_reg;
	unsigned long flags;//for spin_lock_irqsave
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	ppoverlay_frc_to_frame_sync_den_end_ctrl_RBUS ppoverlay_frc_to_frame_sync_den_end_ctrl_reg;
	ppoverlay_i2d_meas_phase_RBUS ppoverlay_i2d_meas_phase_reg;
	ppoverlay_dtg_pending_status_RBUS ppoverlay_dtg_pending_status_reg;

	unsigned char display;
	UINT32 TimeOutCounterThreshold = Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut();
	UINT32 TimeOutCounter = 0;
//	UINT32 count = 20;
	extern unsigned judge_scaler_break_case(unsigned char display);
	ppoverlay_i2d_ctrl_0_RBUS ppoverlay_i2d_ctrl_0_reg;
	ppoverlay_i2d_ctrl_1_RBUS ppoverlay_i2d_ctrl_1_reg;

	ppoverlay_uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
	ppoverlay_dv_total_RBUS dv_total_reg;
	unsigned int i2d_timeout = 0x100;

	UINT32 vo_lineCnt0=0;
	UINT32 mtg_lineCnt0=0;
	UINT32 uzu_lineCnt0=0;
	UINT32 memc_lineCnt0=0;
	UINT32 vo_lineCnt1=0;
	UINT32 mtg_lineCnt1=0;
	UINT32 uzu_lineCnt1=0;
	UINT32 memc_lineCnt1=0;

	unsigned char SpeedUp_Timeout = 0;

	display = SLR_MAIN_DISPLAY;//Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
	IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);
	ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg);

	if(Get_DISPLAY_PANEL_OLED_TYPE()==1){
		ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + OLED_PANEL_DEN_PROTECT_PORCH;
	}
	else{
		if((get_panel_res() == PANEL_RES_FHD) || (get_panel_res() == PANEL_RES_HD))// PANEL_DEN_PROTECT_PORCH*Get_DISP_VERTICAL_TOTAL()/2250, KTASKWBS-22532
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH/2;
		else
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH;
	}

	if( ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target < Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN() )
		ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN();

	IoReg_Write32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg, ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue);

	framesync_set_iv2dv_delay(DISPLAY_MODE_FRAME_SYNC);

	ppoverlay_dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
	ppoverlay_dh_total_last_line_length_reg.dh_total = Get_DISP_HORIZONTAL_TOTAL()-1;
	ppoverlay_dh_total_last_line_length_reg.dh_total_last_line = Get_DISP_HORIZONTAL_TOTAL()-DTG_HTOTAL_LASTLINE_DELAY;
	IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, ppoverlay_dh_total_last_line_length_reg.regValue);
#if 0
	//if ((webos_tooloption.eModelModuleType == module_LGD) && (webos_tooloption.eBackLight == direct ) && (webos_tooloption.eLEDBarType == local_dim_block_32)&& (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)){
	if(Get_TIMING_PROTECT_PANEL_TYPE() && !(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())){
		rtd_pr_vbe_emerg("drvif_scaler_set_display_mode_frc2fsync_by_Reset_VODMA\n");

        drvif_scaler_set_display_mode_frc2fsync_I2D_disable();
		drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(FALSE);
		//drvif_scaler_enable_frc2fsync_interrupt(1);
		drvif_scaler_set_display_mode_frc2fsync_by_Reset_VODMA();

		// clear double buffer
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 0;		//D7
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

		//frc2fsync protect
		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	//	display_timing_ctrl2_reg.frc2fsync_speedup_en = 1;
		display_timing_ctrl2_reg.frc2fsync_en = 1;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);

		//enable timing fsync
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
		display_timing_ctrl1_reg.disp_fix_last_line_new= 0;
		display_timing_ctrl1_reg.fsync_fll_mode= _ENABLE;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);

		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

		//check framesync status
		while(display_timing_ctrl1_reg.disp_frc_fsync==0)
		{
			//Eric@20180905 If other task change framerate, need break.
			if(judge_scaler_break_case(display) || (disp_smooth_variable_setting_flag == _ENABLE))
			{
				rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
				drivf_scaler_reset_freerun();
				break;
			}
			TimeOutCounter++;

			ppoverlay_i2d_meas_phase_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_MEAS_phase_reg);
			//rtd_pr_vbe_emerg("[speedup_v_total] i2d_phase_err = %d, 82dc = %d\n", ppoverlay_i2d_meas_phase_reg.i2d_phase_err, IoReg_Read32(0xb80282dc));
			if(TimeOutCounter > TimeOutCounterThreshold)
			{
				IoReg_SetBits(SCALEDOWN_Measure_CTRL_reg, SCALEDOWN_Measure_CTRL_m_active_line_pixel_en_mask);
				//wait 1 frame to get the UZD timing measure result.
				Scaler_wait_for_input_one_frame(display);
				rtd_pr_vbe_emerg("SU wait timeout, run SpeedUP vtotal process again......\n");
				rtd_pr_vbe_emerg("UZD measure timing : (%x), UZD measure ctrl : (%x)\n", IoReg_Read32(SCALEDOWN_Measure_ActLine_Result_Main_Path_reg), IoReg_Read32(SCALEDOWN_Measure_CTRL_reg));
				rtd_pr_vbe_emerg("ONMS1 status : (%x), ONMS3 status : (%x)\n", IoReg_Read32(ONMS_onms1_status_reg), IoReg_Read32(ONMS_onms3_status_reg));
				//non full screen height timing fs fail case need disable frc2fsync by hw too
				//if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
				{
#if 0
					drvif_scaler_set_display_mode_frc2fsync_by_hw(_TRUE);
					rtd_pr_vbe_notice("SU LineBufferMode HW set frc2framesync fail, disable it\n");
#else
					SpeedUp_Timeout = 1;
#endif
				}
				break;
			}
			msleep(0);
			display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		}
		if(SpeedUp_Timeout == 1){
			TimeOutCounter = 0;
			drvif_scaler_enable_frc2fsync_HW_speedup_vtotal_by_I2D(FALSE);
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg);
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH;
			IoReg_Write32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg, ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue);
			//Do speedup vtotal process again
			drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(TRUE);
			drvif_scaler_enable_frc2fsync_interrupt(TRUE);
			while(display_timing_ctrl1_reg.disp_frc_fsync==0)
			{
				//Eric@20180905 If other task change framerate, need break.
				if(judge_scaler_break_case(display) || (disp_smooth_variable_setting_flag == _ENABLE))
				{
					rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
					drivf_scaler_reset_freerun();
					break;
				}
				TimeOutCounter++;

				if(TimeOutCounter > TimeOutCounterThreshold)
				{
					rtd_pr_vbe_emerg("SU wait timeout again, re-run scaler process......\n");
					drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(FALSE);
					drvif_scaler_enable_frc2fsync_interrupt(FALSE);
					drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_timeout_handler();
					break;
				}
				msleep(0);
				display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
			}
		}
		if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI){
			if(drvif_i3ddma_triplebuf_flag())
			{
				rtd_pr_vbe_notice("[%s] hdmi does not enable vodma tracking i3 because frame rate frc case\n", __FUNCTION__);
			}
			else
			{
				rtd_pr_vbe_notice("[%s] enable vodma tracking i3.\n", __FUNCTION__);
				Scaler_Set_VODMA_Tracking_I3DDMA(TRUE);
			}
		}
//		drvif_scaler_enable_frc2fsync_interrupt(0);
	}else
#endif
	{
		drvif_scaler_set_display_mode_frc2fsync_I2D_disable();
		drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(FALSE);

		//check the vo info change, to avoid the scaler info be changed and make the i2d delay line sets wrong.
		if(judge_scaler_break_case(display))
		{
			rtd_pr_vbe_notice("\r\n###func:%s need to skip frc2fsync process###\r\n", __FUNCTION__);
			return;
		}

		//I2D setting
		//enable I2D new mode
		ppoverlay_i2d_ctrl_0_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_CTRL_0_reg);
#ifdef CONFIG_I2D_NEW_MODE
		ppoverlay_i2d_ctrl_0_reg.i2d_dly_new_mode_en = 1;
#else
		ppoverlay_i2d_ctrl_0_reg.i2d_dly_new_mode_en = 0;
#endif
		IoReg_Write32(PPOVERLAY_I2D_CTRL_0_reg, ppoverlay_i2d_ctrl_0_reg.regValue);

		//clear one run done pending status
		ppoverlay_dtg_pending_status_reg.regValue = 0;
		ppoverlay_dtg_pending_status_reg.i2d_dly_onerun_done = 1;
		IoReg_Write32(PPOVERLAY_DTG_pending_status_reg, ppoverlay_dtg_pending_status_reg.regValue);

		ppoverlay_i2d_ctrl_1_reg.regValue = 0;
#ifdef CONFIG_I2D_NEW_MODE
#ifdef CONGIG_I2D_NEW_SW_MODE
		ppoverlay_i2d_ctrl_1_reg.i2d_sw_mode_en = 1;
#else
		ppoverlay_i2d_ctrl_1_reg.i2d_sw_mode_en = 0;
#endif
#else
		ppoverlay_i2d_ctrl_1_reg.i2d_sw_mode_en = 0;
#endif
		ppoverlay_i2d_ctrl_1_reg.i2d_measure_done = 1;//clear measure done status
		if ((get_panel_res() == PANEL_RES_FHD) || (get_panel_res() == PANEL_RES_HD)){
			if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245)){	//24 ,for 120hz panel
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 225;
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 0;
			}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 245) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 255)){	//25
				dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
				if((dv_total_reg.dv_total > DVTOTAL_60Hz_UPPERBOUND) && (dv_total_reg.dv_total < DVTOTAL_60Hz_LOWERBOUND))	//current 60/120Hz
				{
					ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 225;
					ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 0;
				}
				else{
					if(get_panel_res() == PANEL_RES_HD){
						ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 0;
						ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 150;
					}else{
						ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 0;
						ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 225;
					}
				}
			}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305)){	//30
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 225;
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 0;
			}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 505)){ //50
				dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
				if((dv_total_reg.dv_total > DVTOTAL_60Hz_UPPERBOUND) && (dv_total_reg.dv_total < DVTOTAL_60Hz_LOWERBOUND))	//current 60/120Hz
				{
					ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 225;
					ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 0;
				}
				else{
					ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 0;
					ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 225;
				}
			}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 595) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 605)){ //60
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 225;
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 0;
			}else{
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 225;
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 0;
			}
		}
		else{
		if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) &&
				(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245)){	//24 ,for 120hz panel
			ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 450;
			ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 0;
		}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 245) &&
				(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 255)){	//25
			uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
			if((uzudtg_dv_total_reg.uzudtg_dv_total > DVTOTAL_60Hz_UPPERBOUND) && (uzudtg_dv_total_reg.uzudtg_dv_total < DVTOTAL_60Hz_LOWERBOUND))	//current 60/120Hz
			{
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 450;
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 0;
			}
			else{
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 0;
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 450;
			}
		}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) &&
				(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305)){	//30
			ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 450;
			ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 0;
		}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495) &&
				(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 505)){ //50
			uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
			if((uzudtg_dv_total_reg.uzudtg_dv_total > DVTOTAL_60Hz_UPPERBOUND) && (uzudtg_dv_total_reg.uzudtg_dv_total < DVTOTAL_60Hz_LOWERBOUND))	//current 60/120Hz
			{
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 450;
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 0;
			}
			else{
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 0;
				ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 450;
			}
		}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 595) &&
				(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 605)){ //60
			ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 450;
			ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 0;
		}else{
			ppoverlay_i2d_ctrl_1_reg.i2d_dly_add_line_max = 450;
			ppoverlay_i2d_ctrl_1_reg.i2d_dly_sub_line_max = 0;
			}
		}
		IoReg_Write32(PPOVERLAY_I2D_CTRL_1_reg, ppoverlay_i2d_ctrl_1_reg.regValue);
		rtd_pr_vsc_notice("[I2D] PPOVERLAY_I2D_CTRL_1_reg = %x \n", IoReg_Read32(PPOVERLAY_I2D_CTRL_1_reg));
		// Frc2Fsync  sync point parameters.
#if 0
		//set i2d dly target larger than dvs, when add dvtotal, ivs can jump to porch
		//IVS   __|_____|_____|_____|_____|
		//DVS  _|_____|______|______|_____
#endif
		//use i2d to align ivs and dvs directly
		uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
		ppoverlay_i2d_ctrl_0_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_CTRL_0_reg);
		ppoverlay_i2d_ctrl_0_reg.i2d_special_dly_mode = 0; //one time mode
		ppoverlay_i2d_ctrl_0_reg.i2d_dly_lower = uzudtg_dv_total_reg.uzudtg_dv_total - 10;//FRC2FS_I2D_TARGET - 35;
		ppoverlay_i2d_ctrl_0_reg.i2d_dly_upper = uzudtg_dv_total_reg.uzudtg_dv_total - 8;//FRC2FS_I2D_TARGET - 5;
		if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245)){
			ppoverlay_i2d_ctrl_0_reg.i2d_dly_lower = uzudtg_dv_total_reg.uzudtg_dv_total - 14;//FRC2FS_I2D_TARGET - 35;
			ppoverlay_i2d_ctrl_0_reg.i2d_dly_upper = uzudtg_dv_total_reg.uzudtg_dv_total - 12;//FRC2FS_I2D_TARGET - 5;
		}
		IoReg_Write32(PPOVERLAY_I2D_CTRL_0_reg, ppoverlay_i2d_ctrl_0_reg.regValue);
		rtd_pr_vsc_notice("[I2D] PPOVERLAY_I2D_CTRL_0_reg = %x \n", IoReg_Read32(PPOVERLAY_I2D_CTRL_0_reg));
		Scaler_wait_for_input_one_frame(display);	//after adjusting i2d upper lower, need wait 1 frame to make phase error change
		ppoverlay_i2d_meas_phase_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_MEAS_phase_reg);
		//rtd_pr_vsc_notice("[I2D] after wait 1 frame, i2d_phase_err = %d\n", ppoverlay_i2d_meas_phase_reg.i2d_phase_err);
		//enable I2D speed up by hw
		ppoverlay_i2d_ctrl_0_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_CTRL_0_reg);
		ppoverlay_i2d_ctrl_0_reg.i2d_special_dly_en = 1;
		IoReg_Write32(PPOVERLAY_I2D_CTRL_0_reg, ppoverlay_i2d_ctrl_0_reg.regValue);
#ifdef CONFIG_I2D_NEW_MODE
#ifdef CONGIG_I2D_NEW_SW_MODE
		ppoverlay_i2d_ctrl_1_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_CTRL_1_reg);
		//rtd_pr_vbe_notice("pre I2D measure_done timeout=%x, i2d_measure_done=%d\n", i2d_timeout, ppoverlay_i2d_ctrl_1_reg.i2d_measure_done);
		while(ppoverlay_i2d_ctrl_1_reg.i2d_measure_done == 0){
			msleep(0);
			i2d_timeout --;
			if(i2d_timeout == 0)
			{
				rtd_pr_vbe_emerg("I2D measure_done wait timeout......\n");
				break;
			}
			ppoverlay_i2d_ctrl_1_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_CTRL_1_reg);
		}
		//rtd_pr_vbe_notice("after I2D measure_done timeout=%x, i2d_measure_done=%d\n", i2d_timeout, ppoverlay_i2d_ctrl_1_reg.i2d_measure_done);

		//i2d sw go
		ppoverlay_i2d_ctrl_1_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_CTRL_1_reg);
		ppoverlay_i2d_ctrl_1_reg.i2d_sw_go = 0;
		IoReg_Write32(PPOVERLAY_I2D_CTRL_1_reg, ppoverlay_i2d_ctrl_1_reg.regValue);
		ppoverlay_i2d_ctrl_1_reg.i2d_sw_go = 1;
		IoReg_Write32(PPOVERLAY_I2D_CTRL_1_reg, ppoverlay_i2d_ctrl_1_reg.regValue);
#endif
#endif
		i2d_timeout = 0x100;
		while(ppoverlay_i2d_ctrl_0_reg.i2d_special_dly_en == 1){
			msleep(0);
			ppoverlay_i2d_meas_phase_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_MEAS_phase_reg);
			//rtd_pr_vbe_emerg("[I2D] i2d_phase_err = %d, 82dc = %d\n", ppoverlay_i2d_meas_phase_reg.i2d_phase_err, IoReg_Read32(0xb80282dc));
			i2d_timeout --;
			if(i2d_timeout == 0)
			{
				rtd_pr_vbe_emerg("I2D wait timeout......\n");
				//Disable I2D
				ppoverlay_i2d_ctrl_0_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_CTRL_0_reg);
				ppoverlay_i2d_ctrl_0_reg.i2d_special_dly_en = 0;
				IoReg_Write32(PPOVERLAY_I2D_CTRL_0_reg, ppoverlay_i2d_ctrl_0_reg.regValue);
				break;
			}
			ppoverlay_i2d_ctrl_0_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_CTRL_0_reg);
		}
		ppoverlay_i2d_meas_phase_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_MEAS_phase_reg);
		rtd_pr_vsc_notice("i2d_timeout = %x , i2d_phase_err = %d\n", i2d_timeout, ppoverlay_i2d_meas_phase_reg.i2d_phase_err);
		//drvif_scaler_enable_frc2fsync_HW_speedup_vtotal_by_I2D(TRUE);

		//frc2fsync protect
		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	//	display_timing_ctrl2_reg.frc2fsync_speedup_en = 1;
		display_timing_ctrl2_reg.frc2fsync_en = 1;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);

		// clear double buffer
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 0;		//D7
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

		//enable timing fsync
		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
		display_timing_ctrl1_reg.disp_fix_last_line_new= 0;
		display_timing_ctrl1_reg.fsync_fll_mode= _ENABLE;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);

		drvif_scaler_enable_frc2fsync_interrupt(1);

		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

		//check framesync status
		while(display_timing_ctrl1_reg.disp_frc_fsync==0)
		{
			//Eric@20180905 If other task change framerate, need break.
			if(judge_scaler_break_case(display) || (disp_smooth_variable_setting_flag == _ENABLE))
			{
				rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
				drivf_scaler_reset_freerun();
				break;
			}
			TimeOutCounter++;

			ppoverlay_i2d_meas_phase_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_MEAS_phase_reg);
			//rtd_pr_vbe_emerg("[speedup_v_total] i2d_phase_err = %d, 82dc = %d\n", ppoverlay_i2d_meas_phase_reg.i2d_phase_err, IoReg_Read32(0xb80282dc));
			if(TimeOutCounter > TimeOutCounterThreshold)
			{
				IoReg_SetBits(SCALEDOWN_Measure_CTRL_reg, SCALEDOWN_Measure_CTRL_m_active_line_pixel_en_mask);
				//wait 1 frame to get the UZD timing measure result.
				Scaler_wait_for_input_one_frame(display);
				rtd_pr_vbe_emerg("SU wait timeout, run SpeedUP vtotal process again......\n");
				rtd_pr_vbe_emerg("UZD measure timing : (%x), UZD measure ctrl : (%x)\n", IoReg_Read32(SCALEDOWN_Measure_ActLine_Result_Main_Path_reg), IoReg_Read32(SCALEDOWN_Measure_CTRL_reg));
				rtd_pr_vbe_emerg("ONMS1 status : (%x), ONMS3 status : (%x)\n", IoReg_Read32(ONMS_onms1_status_reg), IoReg_Read32(ONMS_onms3_status_reg));
				rtd_pr_vbe_emerg("ONMS1 wdg en : (%x), ONMS3 wdg en : (%x)\n", IoReg_Read32(ONMS_onms1_watchdog_en_reg), IoReg_Read32(ONMS_onms3_watchdog_en_reg));
				//non full screen height timing fs fail case need disable frc2fsync by hw too
				//non full screen height timing fs fail case need disable frc2fsync by hw too
				//if(_TRUE == Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
				{
#if 0
					drvif_scaler_set_display_mode_frc2fsync_by_hw(_TRUE);
					rtd_pr_vbe_notice("SU LineBufferMode HW set frc2framesync fail, disable it\n");
#else
					SpeedUp_Timeout = 1;
#endif

				}
				break;
			}
			msleep(0);
			display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		}
		if(SpeedUp_Timeout == 1){
			TimeOutCounter = 0;
			drvif_scaler_enable_frc2fsync_HW_speedup_vtotal_by_I2D(FALSE);
			ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg);
			if((get_panel_res() == PANEL_RES_FHD) || (get_panel_res() == PANEL_RES_HD))// PANEL_DEN_PROTECT_PORCH*Get_DISP_VERTICAL_TOTAL()/2250, KTASKWBS-22532
				ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH/2;
			else
				ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + PANEL_DEN_PROTECT_PORCH;
			if( ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target < Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN() )
				ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN();
			IoReg_Write32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg, ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue);
			//Do speedup vtotal process again
			drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(TRUE);
			drvif_scaler_enable_frc2fsync_interrupt(TRUE);
			while(display_timing_ctrl1_reg.disp_frc_fsync==0)
			{
				//Eric@20180905 If other task change framerate, need break.
				if(judge_scaler_break_case(display) || (disp_smooth_variable_setting_flag == _ENABLE))
				{
					rtd_pr_vbe_notice("\r\n###func:%s need to break###\r\n", __FUNCTION__);
					drivf_scaler_reset_freerun();
					break;
				}
				TimeOutCounter++;

				if(TimeOutCounter > TimeOutCounterThreshold)
				{
					rtd_pr_vbe_emerg("SU wait timeout again, re-run scaler process......\n");
					drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(FALSE);
					drvif_scaler_enable_frc2fsync_interrupt(FALSE);
					drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_timeout_handler();
					break;
				}
				msleep(0);
				display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
			}
		}

	vo_lineCnt1 = IoReg_Read32(VODMA_VODMA_LINE_ST_reg);
	mtg_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	uzu_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg) & 0x1fff;
	memc_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg) & 0x1fff;
#if 0
	//recover vtotal to protect memc den
	ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg);
	if(Get_DISPLAY_PANEL_OLED_TYPE()==1)
		ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + DEN_POSITION_PROTECT_PORCH;
	else
		ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.dv_den_cnt_target = Get_DISP_DEN_END_VPOS() + 20;
	IoReg_Write32(PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg, ppoverlay_frc_to_frame_sync_den_end_ctrl_reg.regValue);
#endif


		//disable frc2fs
		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
		display_timing_ctrl2_reg.frc2fsync_speedup_en = 0;
		display_timing_ctrl2_reg.frc2fsync_en = 1;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
	}

	rtd_pr_vbe_notice("[FRC2FS] Line_cnt0 = %d,%d,%d,%d\n", vo_lineCnt0, mtg_lineCnt0,uzu_lineCnt0,memc_lineCnt0);
	rtd_pr_vbe_notice("[FRC2FS] Line_cnt1 = %d,%d,%d,%d\n", vo_lineCnt1, mtg_lineCnt1,uzu_lineCnt1,memc_lineCnt1);
	rtd_pr_vbe_notice("SU timeout = %d\n", TimeOutCounter);

}


void drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Mcode(void)
{
	return;
}

void drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Porch(void)
{
	return;
#if 0
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	UINT32 TimeOutCounter = 0;
	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;

	if(_DISABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp())
		return;

	if(0 == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_Porch())
	{
		//USER:LewisLee DATE:2013/09/09
		//to prevent porch is 0, action error
		// End = Start + (End - Start) * (19/20)
		//       = (Start + 19 * End)/20
		dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
		dv_den_start_end_reg.dv_den_sta = Get_DISP_DEN_STA_VPOS();
		dv_den_start_end_reg.dv_den_end = (Get_DISP_DEN_STA_VPOS() + Get_DISP_DEN_END_VPOS()*19)/20;
		IoReg_Write32(PPOVERLAY_DV_DEN_Start_End_reg, dv_den_start_end_reg.regValue);
	}
	else// if(0 != Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_Porch())
	{
		dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
		dv_den_start_end_reg.dv_den_sta = Get_DISP_DEN_STA_VPOS();
		dv_den_start_end_reg.dv_den_end = Get_DISP_DEN_STA_VPOS() + Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_Porch();
		IoReg_Write32(PPOVERLAY_DV_DEN_Start_End_reg, dv_den_start_end_reg.regValue);
	}

	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

	while(_FALSE == display_timing_ctrl1_reg.disp_frc_fsync)
	{
		msleep(1);
		TimeOutCounter++;

		if(TimeOutCounter > Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_TimeOut())
		{
			rtd_pr_vbe_info("wait timeout......\n");
			break;
		}

		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	}

	rtd_pr_vbe_info("time count = %d\n",TimeOutCounter);

	WaitFor_DEN_STOP();
	dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	dv_den_start_end_reg.dv_den_sta = Get_DISP_DEN_STA_VPOS();
	dv_den_start_end_reg.dv_den_end = Get_DISP_DEN_END_VPOS();
	IoReg_Write32(PPOVERLAY_DV_DEN_Start_End_reg, dv_den_start_end_reg.regValue);
#endif
}

/**
 * drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup
 * Display Timing from Free run to Frame sync by hw, set to framesync speed
 *
 * @param { void }
 * @return { void }
 */
void drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup(void)
{
/*
	if(Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2()){ //for oled panel
		rtd_pr_vbe_debug("Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2\n");
        drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Mcode_HW_CTRL_2();
	}
    else */
    if(Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2()||
		Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3()){	//for vby1 panel and oled panel
		rtd_pr_vbe_notice("Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3\n");
		drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Mcode_HW_CTRL_3();
    }else if(Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL4()){
		rtd_pr_vbe_notice("Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL4\n");
		drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Mcode_HW_CTRL_4();
	}else if(Scaler_Get_FreeRun_To_FrameSync_By_I2D_CTRL()){
		rtd_pr_vbe_notice("Scaler_Get_FreeRun_To_FrameSync_By_I2D_CTRL\n");
		drvif_scaler_set_display_mode_frc2fsync_by_I2D_CTRL();
	}else if(Scaler_Get_FreeRun_To_FrameSync_By_Reset_VODMA()){
		rtd_pr_vbe_notice("Scaler_Get_FreeRun_To_FrameSync_By_Reset_VODMA\n");
		drvif_scaler_set_display_mode_frc2fsync_by_Reset_VODMA();
	}else{
		rtd_pr_vbe_debug("Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL\n");
		drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Mcode_HW_CTRL();
	}
	//drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_by_DVtotal();
	return;
#if 0
	if(_DISABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp())
		return;

	rtd_pr_vbe_info("drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup\n");

	if(_ENABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByDVtotal())
	{
		rtd_pr_vbe_info("Speed up by vtotal\n");
		drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_by_DVtotal();
	}
	else if(_ENABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode())
	{
		rtd_pr_vbe_info("Speed up by Mcode\n");
		drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Mcode();
	}
	else if(_ENABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByPorch())
	{
		rtd_pr_vbe_info("Speed up by porch\n");
		drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Porch();
	}
	else if(_ENABLE == Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL())
	{
		rtd_pr_vbe_info("Speed up by Mcode HW Control\n");
		drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup_Mcode_HW_CTRL();
	}
#endif
}

#ifndef BUILD_QUICK_SHOW
static unsigned int _dvs_abnormal_event_flag = 0;

void drvif_scaler_set_abnormal_dvs_event_flag(unsigned int flag)
{
	_dvs_abnormal_event_flag = flag;

	if(flag)
		rtd_pr_vbe_emerg("drvif_scaler_set_abnormal_dvs_event_flag = 1 \n");
}

unsigned int drvif_scaler_get_abnormal_dvs_event_flag(void)
{
	return _dvs_abnormal_event_flag;
}
#endif

void drvif_scaler_enable_abnormal_dvs_protect(unsigned char bEnable_long, unsigned char bEnable_short)
{//bEnable_long: for dvs_abnormal_long_en    bEnable_short: for dvs_abnormal_short_en
	ppoverlay_dvs_abnormal_control_RBUS ppoverlay_dvs_abnormal_control_reg;

	ppoverlay_dvs_abnormal_control_reg.regValue = IoReg_Read32(PPOVERLAY_DVS_Abnormal_Control_reg);
	if (get_panel_res() == PANEL_RES_FHD)
	{
		ppoverlay_dvs_abnormal_control_reg.dvs_long_line_th = DVS_2K_LONG_LINE_THRESHOLD;
		ppoverlay_dvs_abnormal_control_reg.dvs_short_line_th = DVS_2K_SHORT_LINE_THRESHOLD;
	}
	else if (get_panel_res() == PANEL_RES_HD)
	{
		ppoverlay_dvs_abnormal_control_reg.dvs_long_line_th = DVS_HD_LONG_LINE_THRESHOLD;
		ppoverlay_dvs_abnormal_control_reg.dvs_short_line_th = DVS_HD_SHORT_LINE_THRESHOLD;
	}
	else if (get_panel_res() == PANEL_RES_WQHD)
	{
		ppoverlay_dvs_abnormal_control_reg.dvs_long_line_th = DVS_WQHD_LONG_LINE_THRESHOLD;
		ppoverlay_dvs_abnormal_control_reg.dvs_short_line_th = DVS_WQHD_SHORT_LINE_THRESHOLD;
	}
	else
	{
		if(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())
        {
            if(Get_DISPLAY_PANEL_OLED_TYPE())//oled case  Todo DVS_4k_VRR_LONG_LINE_THRESHOLD
                ppoverlay_dvs_abnormal_control_reg.dvs_long_line_th = DVS_4k_OLED_VRR_LONG_LINE_THRESHOLD;//use oled threashould 40~120
            else
                ppoverlay_dvs_abnormal_control_reg.dvs_long_line_th = DVS_4k_LED_VRR_LONG_LINE_THRESHOLD;//use led threashould 48~120
        }
        else
            ppoverlay_dvs_abnormal_control_reg.dvs_long_line_th = DVS_4K_LONG_LINE_THRESHOLD;
		ppoverlay_dvs_abnormal_control_reg.dvs_short_line_th = Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN();
	}
	IoReg_Write32(PPOVERLAY_DVS_Abnormal_Control_reg, ppoverlay_dvs_abnormal_control_reg.regValue);

	if(bEnable_long || bEnable_short){
		ppoverlay_dvs_abnormal_control_reg.dvs_abnormal_long_en = bEnable_long;
		ppoverlay_dvs_abnormal_control_reg.dvs_abnormal_short_en = bEnable_short;
		ppoverlay_dvs_abnormal_control_reg.dvs_abnormal_measure_en = 1;
	}else{
		ppoverlay_dvs_abnormal_control_reg.dvs_abnormal_long_en = 0;
		ppoverlay_dvs_abnormal_control_reg.dvs_abnormal_short_en = 0;
		ppoverlay_dvs_abnormal_control_reg.dvs_abnormal_measure_en = 0;
	}
	IoReg_Write32(PPOVERLAY_DVS_Abnormal_Control_reg, ppoverlay_dvs_abnormal_control_reg.regValue);
	//if(bEnable)
		//rtd_pr_vbe_notice("[%s] enable = %d \n", __FUNCTION__, bEnable);
}
void drvif_scaler_wclr_abnormal_dvs_long_short(void)
{
	ppoverlay_dtg_pending_status_RBUS	ppoverlay_dtg_pending_status_reg;

	ppoverlay_dtg_pending_status_reg.regValue = 0;//IoReg_Read32(PPOVERLAY_DTG_pending_status_reg);
	ppoverlay_dtg_pending_status_reg.dvs_long = 1;
	ppoverlay_dtg_pending_status_reg.dvs_short = 1;
	IoReg_Write32(PPOVERLAY_DTG_pending_status_reg, ppoverlay_dtg_pending_status_reg.regValue);
	rtd_pr_vbe_notice("[DTG][%s]\n", __FUNCTION__);
}

#ifndef BUILD_QUICK_SHOW
void drvif_scaler_handle_abnormal_dvs_event(void)
{
	ppoverlay_main_display_control_rsv_RBUS main_display_control_rsv_reg;
	
	extern void request_vo_info(void);
	
	rtd_pr_vbe_notice("[%s] happen abnormal dvs, trigger scaler re-run to recover timing\n", __FUNCTION__);
	mute_control(SLR_MAIN_DISPLAY, TRUE);/*Auto BG*/
	
	//reset timing as freerun
	drivf_scaler_reset_freerun();

	//clear watchdog
	main_display_control_rsv_reg.regValue = IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
	main_display_control_rsv_reg.main_wd_to_background = 0;//disable watch dog free run
	main_display_control_rsv_reg.main_wd_to_free_run = 0;//disalbe watch dog free run
	IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, main_display_control_rsv_reg.regValue);

	//clear status.
	drvif_scaler_wclr_abnormal_dvs_long_short();
	
	//trigger scaler re-run
	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI){
		scaler_HDMI_signal_error_handler(SLR_MAIN_DISPLAY);
	}
	else if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) || (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG) ||
        (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA)){
		request_vo_info();
		scaler_VDEC_signal_error_handler(SLR_MAIN_DISPLAY);
	}
	
	DbgSclrFlgTkr.Main_Run_Scaler_flag = 1;
	//drvif_scaler_set_abnormal_dvs_event_flag(1);

}


unsigned char drvif_scaler_get_abnormal_dvs_long_flag(void)
{
	ppoverlay_dtg_pending_status_RBUS	ppoverlay_dtg_pending_status_reg;

	ppoverlay_dtg_pending_status_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_pending_status_reg);
	if(ppoverlay_dtg_pending_status_reg.dvs_long == 1)
		return 1;
	else
		return 0;
}

unsigned char drvif_scaler_get_abnormal_dvs_short_flag(void)
{
	ppoverlay_dtg_pending_status_RBUS	ppoverlay_dtg_pending_status_reg;

	ppoverlay_dtg_pending_status_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_pending_status_reg);
	if(ppoverlay_dtg_pending_status_reg.dvs_short == 1)
		return 1;
	else
		return 0;
}

void drvif_scaler_check_abnormal_dvs_flag(void){
	
	if((drvif_scaler_get_abnormal_dvs_long_flag()==TRUE)||(drvif_scaler_get_abnormal_dvs_short_flag()==TRUE)){
		if(drvif_scaler_get_abnormal_dvs_long_flag()==TRUE){
			rtd_pr_vbe_emerg("[WARNING]abnormal dvs long flag = 1 \n");
		}
		if(drvif_scaler_get_abnormal_dvs_short_flag()==TRUE){
			rtd_pr_vbe_emerg("[WARNING]abnormal dvs short flag = 1 \n");
		}
		drvif_scaler_handle_abnormal_dvs_event();
	}
}

void drvif_scaler_check_ivs_over_threshold(unsigned int enable)
{
	static int start_flag = 1;
	ppoverlay_measure_ivs_ctrl0_RBUS measure_ivs_ctrl0_reg;

	measure_ivs_ctrl0_reg.regValue = IoReg_Read32(PPOVERLAY_measure_ivs_ctrl0_reg);

	if ((start_flag == 1) && (enable == TRUE)) {
		measure_ivs_ctrl0_reg.ivs_threshold = 27000000 / 10;	// for 10Hz
		IoReg_Write32(PPOVERLAY_measure_ivs_ctrl0_reg, measure_ivs_ctrl0_reg.regValue);
		start_flag = 0;
	}

	if (enable == FALSE) {
		measure_ivs_ctrl0_reg.ivs_threshold = 0;		// disable IVS over threshold test
		IoReg_Write32(PPOVERLAY_measure_ivs_ctrl0_reg, measure_ivs_ctrl0_reg.regValue);
		start_flag = 1;
	}

	if (measure_ivs_ctrl0_reg.ivs_over_threshold_err == 1) {
		rtd_pr_vbe_notice("[ERROR] IVS over threshold\n");

	}

}

void drvif_scaler_check_main_onlinemeasure_flag(void)
{
	static unsigned char dbg_prt_cnt=0;
	onms_onms1_status_RBUS onms_onms1_status_reg;
	onms_onms1_watchdog_en_RBUS onms_onms1_watchdog_en_reg;
	unsigned char clearONMS1 = 0;

	onms_onms1_watchdog_en_reg.regValue = IoReg_Read32(ONMS_onms1_watchdog_en_reg);
	onms_onms1_status_reg.regValue = IoReg_Read32(ONMS_onms1_status_reg);

	if(onms_onms1_watchdog_en_reg.regValue & onms_onms1_status_reg.regValue){
		clearONMS1 = 1;
	}

	if(clearONMS1==1){
		if(dbg_prt_cnt > 10){
			rtd_pr_vbe_notice("[WARNING] clearONMS1 (%x)\n", onms_onms1_status_reg.regValue);
			IoReg_Write32(ONMS_onms1_status_reg, onms_onms1_status_reg.regValue);//Clear online main status
			dbg_prt_cnt=0;
		}else{
			dbg_prt_cnt++;
		}
	}else{
		dbg_prt_cnt=0;
	}
}


void WaitFor_3DTG_Period(UINT32 StartOffset, UINT32 EndOffset)
{
	UINT32 timeoutcnt = 0x032500;
	ppoverlay_new_meas0_linecnt_real_RBUS new_meas0_linecnt_real_reg;

	do {
			new_meas0_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
			if((new_meas0_linecnt_real_reg.line_cnt_rt > StartOffset) && (new_meas0_linecnt_real_reg.line_cnt_rt < EndOffset)) {
				return;
			}
	} while(timeoutcnt-- != 0);

	if(timeoutcnt == 0){
		rtd_pr_vbe_debug("\n ERROR not wait the correct line \n");
	}
}

void WaitFor_DEN_STOP(void)
{
	#if 1
	UINT32 timeoutcnt = 0x032500;
	UINT32 denEnd;
	ppoverlay_new_meas0_linecnt_real_RBUS new_meas0_linecnt_real_reg;
	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	denEnd = dv_den_start_end_reg.dv_den_end;
	do{
			new_meas0_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
			if(new_meas0_linecnt_real_reg.line_cnt_rt > denEnd) {
				return;
			}
	} while((--timeoutcnt) != 0);

	if(timeoutcnt == 0){
		rtd_pr_vbe_debug("\n ERROR not wait the correct line \n");
		fwif_scaler_wait_for_event(PPOVERLAY_DTG_pending_status_2_reg, _BIT0);
	}

	#else
		fwif_scaler_wait_for_event(PPOVERLAY_DTG_pending_status_2_reg, _BIT0);
	#endif
}

//USER:LewisLee DATE:2012/07/09
//Wait Den Stop at some region
void WaitFor_DEN_STOP_Period(UINT32 PorchStartOffset, UINT32 PorchEndOffset)
{
	UINT32 timeoutcnt = 0x032500;
	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	ppoverlay_dv_total_RBUS dv_total_reg;
	ppoverlay_new_meas0_linecnt_real_RBUS new_meas0_linecnt_real_reg;
	UINT32 PorchStart = 0, PorchEnd = 0;

	dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	PorchStart = dv_den_start_end_reg.dv_den_end + PorchStartOffset;
	PorchEnd = dv_den_start_end_reg.dv_den_end + PorchEndOffset;

	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);

	//to prevent over range
	if(PorchEnd >= dv_total_reg.dv_total)
{
		PorchEnd = dv_total_reg.dv_total - 1;

		if(PorchStart >= PorchEnd)
			PorchStart = PorchEnd - 1;
	}

	do
	{
		new_meas0_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);

		if(new_meas0_linecnt_real_reg.line_cnt_rt > PorchStart)
		{
			if(new_meas0_linecnt_real_reg.line_cnt_rt < PorchEnd)
			return;
		}

		timeoutcnt--;
	} while(0 != timeoutcnt);

	if(0 == timeoutcnt)
	{
		rtd_pr_vbe_debug("\n ERROR not wait the correct line :%x, %x\n", PorchStart, PorchEnd);
		fwif_scaler_wait_for_event(PPOVERLAY_DTG_pending_status_2_reg, _BIT0);
	}
}


//USER:LewisLee DATE:2012/07/09
//Wait Den Stop at some region
void WaitFor_DEN_STOP_Period2(UINT32 PorchStartOffset, UINT32 PorchEndOffset)
{
	UINT32 timeoutcnt = 0xFFFFFF;//0x032500;
//	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	ppoverlay_dv_total_RBUS dv_total_reg;
	ppoverlay_new_meas0_linecnt_real_RBUS new_meas0_linecnt_real_reg;
	UINT32 PorchStart = 0, PorchEnd = 0;

//	dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	PorchStart = PorchStartOffset;
	PorchEnd = PorchEndOffset;

	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);

	//to prevent over range
	if(PorchEnd >= dv_total_reg.dv_total)
{
		PorchEnd = dv_total_reg.dv_total - 1;

		if(PorchStart >= PorchEnd)
			PorchStart = PorchEnd - 1;
	}

	do
	{
		new_meas0_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);

		if(new_meas0_linecnt_real_reg.line_cnt_rt > PorchStart)
		{
			if(new_meas0_linecnt_real_reg.line_cnt_rt < PorchEnd)
			return;
		}

		timeoutcnt--;
	} while(0 != timeoutcnt);

	if(0 == timeoutcnt)
	{
		rtd_pr_vbe_debug("\n ERROR not wait the correct line :%x, %x\n", PorchStart, PorchEnd);
		fwif_scaler_wait_for_event(PPOVERLAY_DTG_pending_status_2_reg, _BIT0);
	}
}

void WaitFor_DEN_STOP_UZUDTG(void)
{
	UINT32 timeoutcnt = 0xFFFFFF;
	UINT32 denEnd;
	ppoverlay_new_meas2_linecnt_real_RBUS new_meas2_linecnt_real_reg;
	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	denEnd = dv_den_start_end_reg.dv_den_end;

	do {
		new_meas2_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
		if(new_meas2_linecnt_real_reg.uzudtg_dly_line_cnt_rt > denEnd) {
			return;
		}
	} while(timeoutcnt-- != 0);

	if(timeoutcnt == 0){
		rtd_pr_vbe_debug("\n%s ERROR not wait the correct line \n", __FUNCTION__);
	}
}


void WaitFor_Frame_timer(void) //wait a frame by real time counter
{
	UINT32 timeoutcnt = 0xFFFFFF;
	unsigned int before_time = 0;
	unsigned int after_time = 0;
	unsigned int target_time = 0;
	if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245)){
		//24Hz
		target_time = 41;
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 245) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 255)){
		//25Hz
		target_time = 40;
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305)){
		//30Hz
		target_time = 33;
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 505)){
		//50Hz
		target_time = 20;
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 595) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 605)){
		//60Hz
		target_time = 16;
	}else{ // others
		target_time = 16;
	}

	before_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;

	do {
		after_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90;
		if((after_time - before_time) >= target_time) {
			break;
		}
	} while(--timeoutcnt != 0);

	if(timeoutcnt == 0){
		rtd_pr_vbe_debug("\n%s Frame timer wait timeout!!! \n", __FUNCTION__);
	}
}
#endif

void scaler_config_zoom_para(void)
{
//	unsigned short capWid = Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID);
	unsigned short capLen = Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN);
	unsigned short iphActWid = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
//	unsigned short memActWid = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID);

	if (!Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)){
			Scaler_DispSetInputInfo(SLR_INPUT_DI_WID,iphActWid);
			Scaler_DispSetInputInfo(SLR_INPUT_CAP_WID,iphActWid);
			Scaler_DispSetInputInfo(SLR_INPUT_DISP_WID,iphActWid);
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_WID,iphActWid);
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,capLen);
			Scaler_DispSetInputInfo(SLR_INPUT_DISP_LEN,capLen);
	}
}

#ifndef BUILD_QUICK_SHOW


#ifdef CONFIG_3D_OVERSCAN_FRAMESEQ

extern void (*pOverscan_func)(stDISPLAY_OVERSCAN_DATA *pt_scan, SLR_RATIO_TYPE ratio_type);
extern void (*pGet_Src_Overscan_ratio)(UINT8 *h_ratio, UINT8 *v_ratio);
extern void (*pVGIP_Adjustment_func) (stVGIP_ADJUSTMENT_INFO *pt_data, SLR_RATIO_TYPE ratio_type);

static void FS_3d_overscan(stDISPLAY_OVERSCAN_DATA *pt_scan)
{

	unsigned short ulResult;
	unsigned char h_ratio,v_ratio;

	if (pGet_Src_Overscan_ratio)
		pGet_Src_Overscan_ratio(&h_ratio,&v_ratio);

#ifndef CONFIG_3D_OVERSCAN_ENABLE
	h_ratio = v_ratio = 100;  // default not to overscan
#endif

	rtd_pr_vbe_debug("3d overscan ratio %d %d\n",h_ratio,v_ratio);

	if (h_ratio == 100 && v_ratio == 100)
		return;

	ulResult = pt_scan->VHeight*(100-v_ratio)/400;

	pt_scan->VStart = pt_scan->VStart + ulResult;
	pt_scan->VHeight = pt_scan->VHeight -(4*ulResult);

	ulResult = pt_scan->HWidth*(100-h_ratio)/200;

	if (ulResult%2)//for YUV422
		ulResult++;

	pt_scan->HStart = pt_scan->HStart + ulResult;
	pt_scan->HWidth = pt_scan->HWidth -(2*ulResult);
}
#endif


#ifdef CONFIG_3D_ASPECT_RATIO_FRAMESEQ

void FS_frame_packet_size_config(void)
{
	SLR_RATIO_TYPE ratio_type = Scaler_DispGetRatioMode2();

	unsigned short caplen_noactive = 1080;
	unsigned short temp;
#ifdef CONFIG_3D_OVERSCAN_FRAMESEQ
		stDISPLAY_OVERSCAN_DATA scan_data;
#endif

	rtd_pr_vbe_debug("ratio_type=%d\n",ratio_type);

	Scaler_DispSetInputInfo(SLR_INPUT_CAP_LEN, 1103);


#ifdef CONFIG_3D_OVERSCAN_FRAMESEQ
	scan_data.HStart = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA);
	scan_data.HWidth = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID);
	scan_data.VStart = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA);
	scan_data.VHeight = caplen_noactive ; //Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN);
	rtd_pr_vbe_debug("before m-domain frame sequence v overscan info %d %d %d %d\n",scan_data.HStart,scan_data.HWidth,scan_data.VHeight,scan_data.VStart);
	FS_3d_overscan(&scan_data);
	rtd_pr_vbe_debug("m-domain frame sequence v overscan info %d %d %d %d\n",scan_data.HStart,scan_data.HWidth,scan_data.VHeight,scan_data.VStart);
#endif

	switch (ratio_type)
	{
		case SLR_ASPECT_RATIO_CAPTION:
			rtd_pr_vbe_debug("packet caption...\n");
			temp = (caplen_noactive >> 5) & ~_BIT0 ;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA,temp);
			temp = ((caplen_noactive *15)>>4) & ~_BIT0 ;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,temp / 2);
			//Scaler_DispSetInputInfo(SLR_INPUT_DISP_LEN,temp);
			break;


		case SLR_ASPECT_RATIO_MOVIE:

			temp = (caplen_noactive >> 5) & ~_BIT0;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA,temp);
			temp = ((caplen_noactive>>3)*7) & ~_BIT0 ;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,temp / 2);
		//	Scaler_DispSetInputInfo(SLR_INPUT_DISP_LEN,temp);
			break;
		case SLR_ASPECT_RATIO_PERSON:

			temp = caplen_noactive >>6 & ~_BIT0;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA,temp);
			temp = ((caplen_noactive>>4)*15) & ~_BIT0 ;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,temp / 2);
		//	Scaler_DispSetInputInfo(SLR_INPUT_DISP_LEN,temp);
			break;

		default:
			// 540+540+23(active space)
			#ifdef CONFIG_3D_OVERSCAN_FRAMESEQ
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN, scan_data.VHeight / 2);
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA, scan_data.VStart);
			#else
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN, 540);
			#endif

	}


	rtd_pr_vbe_debug("mem start=%x %x\n",Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
}



void FS_top_bot_size_config(unsigned int usAfterDILen)
{
	unsigned short temp;
	SLR_RATIO_TYPE ratio_type = Scaler_DispGetRatioMode2();
#ifdef CONFIG_3D_OVERSCAN_FRAMESEQ
	stDISPLAY_OVERSCAN_DATA scan_data;
#endif
	Scaler_DispSetInputInfo(SLR_INPUT_CAP_LEN, usAfterDILen);

#ifdef CONFIG_3D_OVERSCAN_FRAMESEQ
	scan_data.HStart = 0 ;//Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA);
	scan_data.HWidth = 0 ;//Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID);
	scan_data.VStart = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA);
	scan_data.VHeight =  Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN) ; //Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN);
	rtd_pr_vbe_debug("before m-domain frame sequence v overscan info %d %d %d %d\n",scan_data.HStart,scan_data.HWidth,scan_data.VHeight,scan_data.VStart);
	FS_3d_overscan(&scan_data);
	rtd_pr_vbe_debug("m-domain frame sequence v overscan info %d %d %d %d\n",scan_data.HStart,scan_data.HWidth,scan_data.VHeight,scan_data.VStart);
#endif


		if(ratio_type==SLR_ASPECT_RATIO_CAPTION )
		{
			temp = Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN) >> 5;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA,temp);
			temp = ((Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN)*15)>>4) & ~_BIT0 ;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,temp/2);

			//Scaler_DispSetInputInfo(SLR_INPUT_DISP_LEN,temp);


		}
		else if(ratio_type==SLR_ASPECT_RATIO_MOVIE )
		{
			temp = (Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN) >> 5);
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA,temp);
			temp = ((Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN)>>3)*7) & ~_BIT0 ;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,temp/2);
			//Scaler_DispSetInputInfo(SLR_INPUT_DISP_LEN,temp);
		}
		else if(ratio_type==SLR_ASPECT_RATIO_PERSON )
		{


			temp = Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN) >>6;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA,temp);
			temp = ((Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN)>>4)*15) & ~_BIT0 ;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,temp/2);
			//Scaler_DispSetInputInfo(SLR_INPUT_DISP_LEN,temp);
		}
	 	else
	 	{
			#ifdef CONFIG_3D_OVERSCAN_FRAMESEQ
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN, scan_data.VHeight /2);
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA, scan_data.VStart);
			#else
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN, Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN)/2);
			#endif
		}
}


//frame sequence side by side size config
void FS_SBS_size_config(unsigned int usAfterDILen)
{
	UINT32 temp;
#ifdef CONFIG_3D_OVERSCAN_FRAMESEQ
	stDISPLAY_OVERSCAN_DATA scan_data;
#endif
	temp = min((unsigned int)(Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN)/2),(unsigned int)usAfterDILen);
	Scaler_DispSetInputInfo(SLR_INPUT_CAP_LEN, temp);

	#ifdef CONFIG_3D_OVERSCAN_FRAMESEQ
	scan_data.HStart = 0 ;//Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA);
	scan_data.HWidth = 0 ;//Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID);
	scan_data.VStart = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA);
	scan_data.VHeight =  temp ; //Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN);
	rtd_pr_vbe_debug("before m-domain frame sequence v overscan info %d %d %d %d\n",scan_data.HStart,scan_data.HWidth,scan_data.VHeight,scan_data.VStart);
	FS_3d_overscan(&scan_data);
	rtd_pr_vbe_debug("m-domain frame sequence v overscan info %d %d %d %d\n",scan_data.HStart,scan_data.HWidth,scan_data.VHeight,scan_data.VStart);
	#endif


	#ifdef CONFIG_3D_OVERSCAN_FRAMESEQ
	Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN, scan_data.VHeight);
	Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA, scan_data.VStart);
	#else
	Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN, usAfterDILen);
	#endif
}



#endif


#ifdef CONFIG_3D_ASPECT_RATIO
void frame_packet_size_config(unsigned int usAfterDILen)
{
	unsigned short temp;
	unsigned short caplen_noactive;
	unsigned short wid;

	int m_ActiveSpace = 0;
	SLR_RATIO_TYPE ratio_type = Scaler_DispGetRatioMode2();

	m_ActiveSpace = (usAfterDILen - (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID)*9/16)*2);

	if (m_ActiveSpace < 0) // cklai@temp add this patch that switch to 2D mode will let scaler malloc failed.
	{
		rtd_pr_vbe_debug("[3D][%s][%d]=%d=%d=%d=%d=\n", __FUNCTION__, __LINE__, m_ActiveSpace, usAfterDILen, Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN), Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID));
		m_ActiveSpace = 0;
	}


	//Scaler_DispSetInputInfo(SLR_INPUT_CAP_LEN, Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN)+16);//display 1366x768
	//Scaler_DispSetInputInfo(SLR_INPUT_CAP_LEN, Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN)+23);//display 1920x1080
	if (drvif_scaler3d_decide_3d_PR_PQ_improve() == TRUE)
	{
		// 1080p frame packing
		Scaler_DispSetInputInfo(SLR_INPUT_CAP_LEN, usAfterDILen);
	}
#if 0 // forGumpDemo
	else if (Scaler_Get3D_for_GumpDemo_isScaleDown() == 0)
	{
		// 1080p frame packing
		Scaler_DispSetInputInfo(SLR_INPUT_CAP_LEN, usAfterDILen);
	}
#endif
	else
	{
		wid = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
		if (0 != wid)
			Scaler_DispSetInputInfo(SLR_INPUT_CAP_LEN, Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN)+(m_ActiveSpace*(Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN)/2))/(wid*9/16));
//		if (m_ActiveSpace % 2 != 0) // prevent inaccuracy after LR reverse
		{
			Scaler_DispSetInputInfo(SLR_INPUT_CAP_LEN, Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN)+1);
		}
	}

	caplen_noactive = 1080;

	switch (ratio_type)
	{

		case SLR_ASPECT_RATIO_CAPTION:

			temp = (caplen_noactive >> 5) & ~_BIT0 ;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA,temp);
			temp = ((caplen_noactive *15)>>4) & ~_BIT0 ;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,temp);
			Scaler_DispSetInputInfo(SLR_INPUT_DISP_LEN,temp);

			break;


		case SLR_ASPECT_RATIO_MOVIE:

			temp = (caplen_noactive >> 5) & ~_BIT0;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA,temp);
			temp = ((caplen_noactive>>3)*7) & ~_BIT0 ;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,temp);
			Scaler_DispSetInputInfo(SLR_INPUT_DISP_LEN,temp);
			break;
		case SLR_ASPECT_RATIO_PERSON:

			temp = caplen_noactive >>6 & ~_BIT0;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA,temp);
			temp = ((caplen_noactive>>4)*15) & ~_BIT0 ;
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,temp);
			Scaler_DispSetInputInfo(SLR_INPUT_DISP_LEN,temp);
			break;

		default:

			//rtd_pr_vbe_info("[3D][%s][%d]=%d=%d=\n", __FUNCTION__, __LINE__, usAfterDILen, Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN));
			//rtd_pr_vbe_info("[3D][%s][%d]=%d=%d=%d=\n", __FUNCTION__, __LINE__, m_ActiveSpace, Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN), Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID));
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN));

	}
}
#endif

UINT32 drvif_scaler_3D_scale_down_width(UINT32 width)
{
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF)){
		if(Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()|| (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_SIDE_BY_SIDE_HALF) == TRUE))
		{
			if(((width>>1)%2)!=0)
				width=width-2;
		}
	}
	return width;
}
#endif

unsigned int memory_get_capture_size(unsigned char display, eMemCapAccessType AccessType);
unsigned int memory_get_display_size(unsigned char display);

#ifdef CONFIG_DUAL_CHANNEL
extern unsigned short Sub_InputRegion_x;
extern unsigned short Sub_InputRegion_y;
extern unsigned short Sub_InputRegion_w;
extern unsigned short Sub_InputRegion_h;
#endif
extern unsigned short Main_InputRegion_x;
extern unsigned short Main_InputRegion_y;
extern unsigned short Main_InputRegion_w;
extern unsigned short Main_InputRegion_h;

#ifdef CONFIG_SDNR_CROP
extern unsigned char rtk_hal_vsc_GetInputRegion(VIDEO_WID_T wid, VIDEO_RECT_T * pinregion);
extern unsigned char rtk_hal_vsc_GetOutputRegion(VIDEO_WID_T wid, VIDEO_RECT_T * poutregion);
#endif

extern StructSrcRect Scaler_intputRegion[MAX_DISP_CHANNEL_NUM];
extern StructSrcRect Scaler_intputTiming[MAX_DISP_CHANNEL_NUM];
extern StructSrcRect Scaler_intputRegion_base_scalerTiming[MAX_DISP_CHANNEL_NUM];

/*qiangzhou added this api for lower align example:15 align 4 will be 12*/
unsigned int drvif_memory_get_data_littlealign(unsigned int Value, unsigned int unit)
{
	unsigned int AlignVal;

	AlignVal = Value;
	unit -= 1;
	if (Value & unit)
		AlignVal = (Value & (~unit));
	return AlignVal;
}

bool is_caveout_memory_buffer_source(SCALER_DISP_CHANNEL display)
{//if true: main m domain use 2k size(k8 carveout size). using decide scaling down or not
    extern bool is_game_mode_set_line_buffer(void);
    bool ret = true;
    VSC_INPUT_TYPE_T srctype;

    srctype = Get_DisplayMode_Src(display);

    if (display == SLR_MAIN_DISPLAY) {
        if(data_frc_need_borrow_cma_buffer(display))
        {
        	ret = false;
        }
        else{
        // 1. 4khfr,borrow from memc;
        // 2. no i3,borrow from vbm;
        // 3. run i3:
            if (is_4k_hfr_mode(display)) {
                ret = false;
           } else if (get_vsc_src_is_hdmi_or_dp()) {
                if (0==get_force_i3ddma_enable(SLR_MAIN_DISPLAY)) {
                    ret =false;
                } else if ((((scaler_vsc_get_gamemode_go_datafrc_mode() == TRUE) && (is_game_mode_set_line_buffer() == false)&& (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) > M_DOMAIN_WIDTH_CARVEOUT_2K)
				&& (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > M_DOMAIN_LEN_CARVEOUT_2K))
				|| (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())
				|| ((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) > M_DOMAIN_WIDTH_CARVEOUT_2K)
				&& (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > M_DOMAIN_LEN_CARVEOUT_2K)
				&& (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 990))//input size > 2K & HFR
				)) {
    			ret = false;
                }
            }
    	}
    }

    return ret;

}

static unsigned int align(unsigned int value, int align_bit)
{
	unsigned int align_value = (align_bit > 0) ? 1 << align_bit : 0;
	unsigned int mask = (align_bit > 0) ? align_value - 1 : 0;
	return (value & ~mask) + (((value & mask) == 0) ? 0 : align_value);
}


unsigned int max_width,max_len ;

void calculate_carveout_size(unsigned char display)
{
    unsigned int carveout_width,carveout_len;
#ifdef CONFIG_RTK_BUSINESS_DISPLAY_MODEL
    if (Get_rotate_function(display)){
        carveout_width = M_DOMAIN_WIDTH_BUSINESS_DISPLAY;
        carveout_len = M_DOMAIN_LEN_BUSINESS_DISPLAY;
    } else {
        carveout_width = M_DOMAIN_WIDTH_CARVEOUT;
        carveout_len = M_DOMAIN_LEN_CARVEOUT;
    }

#else
    carveout_width = M_DOMAIN_WIDTH_CARVEOUT;
    carveout_len = M_DOMAIN_LEN_CARVEOUT;
#endif

    max_width = (display == SLR_MAIN_DISPLAY) ? carveout_width : M_DOMAIN_WIDTH_2K;
    max_len   = (display == SLR_MAIN_DISPLAY) ? carveout_len   : M_DOMAIN_LEN_2K;

}

static bool is_m_domain_need_scale_down(unsigned char display)
{
    const int width = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_WID);
    const int len   = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_LEN);

    calculate_carveout_size(display);
 //   if ((display == SLR_MAIN_DISPLAY) && is_panel_size_bigger_than_m_size(display)) {
        return (width * len) > (max_width*max_len);
        //(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID) * Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_LEN));
//    }
}


/*============================================================================*/
/**
 * fw_scaler_cal_scale_down_size
 * Decide
 *
 * @param <info->IPH_ACT_STA>	{  }
 * @param <info->IPV_ACT_STA>	{  }
 * @param <info->IPH_ACT_WID>	{  }
 * @param <info->IPV_ACT_LEN>	{  }
 * @param <info->CapWid>			{  }
 * @param <info->CapLen>			{  }
 * @param <info->MEM_ACT_HSTA>	{  }
 * @param <info->MEM_ACT_VSTA>	{  }
 * @param <info->MEM_ACT_WID>	{  }
 * @param <info->MEM_ACT_LEN>	{  }
 * @param <info->DispWid>		{  }
 * @param <info->CapLen>			{  }
 * @param <info->DispLen>		{  }
 * @return 						{ void }
 *
 */
void fw_scaler_cal_scale_down_size(void)
{
	unsigned short usAfterDILen;
	unsigned short temp=0;
//	unsigned short usTempHor = 0; //, usTempVer = 0;
//	unsigned char ucTemp = 0;
	unsigned short carveout_width;
	unsigned short carveout_len;
	SCALER_DISP_CHANNEL display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
    calculate_carveout_size(display);
	// Decide whether enable delete 4 lines (Up 2 line and down 2 line) and calculate line
	// When enter DI, we should cut 	(Up 2 line and down 2 line)
        carveout_width = max_width;
        carveout_len = max_len;

#ifndef BUILD_QUICK_SHOW //def ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
	if((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
		&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING) )|| (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
	{
		if(Scaler_InputSrcGetMainChType() == _SRC_VO)
		{
			//Scaler_DispSetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE, Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) *2);
			//usAfterDILen = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
			rtd_pr_vbe_debug("[mvc]: pr3d fp mode vo source............\n");
			if(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_THRIP))
				usAfterDILen = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) << 1;
			else
				usAfterDILen = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
		}
		else{
			unsigned int i;
			for(i=0; i< 8; i++){
				if(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) == (hdmi_active_space_table[i].v_act_len)){
					rtd_pr_vbe_debug("[HDMI-FP] Idx/P/v_len[%d/%d/%d]\n", i, hdmi_active_space_table[i].progressive, hdmi_active_space_table[i].v_act_len);
					break;
				}
			}
			if(i >= 8){
				rtd_pr_vbe_debug("[ERROR] NO FOUND MATCHED HDMI FP TIMING, len=%d@%s.%d\n", Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE), __FUNCTION__, __LINE__);
				return;
			}

			if(drvif_scaler3d_decide_HDMI_framePacking_interlaced_status()){
				Scaler_DispSetInputInfo(SLR_INPUT_IPV_ACT_LEN, hdmi_active_space_table[i].lr_v_act_len);
				usAfterDILen = (hdmi_active_space_table[i].lr_v_act_len << 1) * 2;
			}else
				usAfterDILen = hdmi_active_space_table[i].lr_v_act_len * 2;

			rtd_pr_vbe_debug("[SG][FP] V.len=%d -> %d\n", Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN), usAfterDILen);
		}
	}else
#endif
	if(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_THRIP)) {
		// if the input is DVI, dont' cut 4 lines
		//if (((Scaler_InputSrcGetMainChFrom() == _SRC_FROM_TMDS) && (!drvif_IsHDMI())))
		if(TRUE == fwif_scaler_decide_display_cut4line(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)))
			usAfterDILen  = (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) << 1) - 4;
		else
			usAfterDILen =  (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) << 1);
	} else  {
		usAfterDILen = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
	}
#ifndef BUILD_QUICK_SHOW
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)==SLR_3D_SIDE_BY_SIDE_HALF))
	{
			temp = min((unsigned int)(Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN)/2),(unsigned int)usAfterDILen);
			Scaler_DispSetInputInfo(SLR_INPUT_CAP_LEN,temp);
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,temp<<1);
			Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MEM_LEN_PRE,Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN)*Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_V_RATIO)/100));
			if(get_3D_overscan_enable() == TRUE)
			{
				temp = min((unsigned int)((Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN) - get_3D_overscan_input_vsta())/2),(unsigned int)get_3D_overscan_input_len());
				Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN, temp <<1);
				if(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_THRIP))
				{
					temp = min((unsigned int)((Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN) - get_3D_overscan_input_vsta())/2),(unsigned int)get_3D_overscan_input_len()<<1);
					Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN, temp << 1);
				}
			}
	}
	else
#endif        
	{
			// VO source frame packing 3D mode -- ex: MVC, IDMA 3D mode
			//unsigned char bVoFp3d=0;
#if 0
			if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE())
				bVoFp3d = ((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)? 1: 0);
#endif
#ifndef BUILD_QUICK_SHOW
	#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
			if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == FALSE)
				&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_TOP_AND_BOTTOM) && drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_TOP_AND_BOTTOM)
				&& (Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN) <= 1080))
			{ // 4k2k Video TaB 3D mode, capture vertical length can be full size
				temp = min(Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN) << 1, usAfterDILen);
			}else
	#endif
#endif    
			{

#ifdef CONFIG_SDNR_CROP
				// VO frame packing video length will become double after remove R-frame v-sync
				if(is_4k_hfr_mode(display))
					temp = min((unsigned int)Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN),(unsigned int)((Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_LEN))/2));	//4k1k
				else
					temp = min((unsigned int)Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN),(unsigned int)(Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_LEN)));
				//rtd_pr_vbe_emerg("file=%s, function=%s, line=%d\n", __FILE__, __FUNCTION__, __LINE__);
				//rtd_pr_vbe_emerg("temp=%d, usHVal=%d, bVoFp3d=%d\n", temp, usHVal, bVoFp3d);
#else
			// VO frame packing video length will become double after remove R-frame v-sync
			if(is_4k_hfr_mode(display))
				temp = min((unsigned int)Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN),(unsigned int)(( usAfterDILen << bVoFp3d)/2));
			else
				temp = min((unsigned int)Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN),(unsigned int)( usAfterDILen << bVoFp3d));
#endif
			}
#ifndef BUILD_QUICK_SHOW

			// 4k2k video vertical length need scaling down to 1080 in SBS 3D mode (3840x2160 -> 3840x1080)
			if((temp > 1080)
				&& Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF)
				&& (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()|| (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_SIDE_BY_SIDE_HALF) == TRUE)))
			{
				temp = 1080;

                rtd_pr_vbe_debug("[vsg][orbit_dbg] scale down\n");
			}
 #endif               
			#ifdef CONFIG_SDNR_CROP
			//sunray add for WOSQRTK-8296 and WOSQRTK-8469
			 if(VD_27M_OFF == fw_video_get_27mhz_mode(display) &&(Get_DisplayMode_Src(display) == VSC_INPUTSRC_AVD) ){
			if(temp>=Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_SDNRV_ACT_LEN))
				temp=Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_SDNRV_ACT_LEN);
			 }
			 #endif

			Scaler_DispSetInputInfo(SLR_INPUT_CAP_LEN,temp);
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,temp);
#ifdef CONFIG_DUAL_CHANNEL
			   
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_VSTA, 0);
			
#endif
#ifndef BUILD_QUICK_SHOW

			if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()
				&& ((Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)==SLR_3D_TOP_AND_BOTTOM) || (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)))
			{
				Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MEM_LEN_PRE, Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
				Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN,(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN)*Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_V_RATIO)/100));
				if(get_3D_overscan_enable() == TRUE)
				{
					temp = min((unsigned int)Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN) -get_3D_overscan_input_vsta(),(unsigned int)get_3D_overscan_input_len());
					Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN, temp);
					if(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_THRIP))
					{
						temp = min((unsigned int)Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN) -get_3D_overscan_input_vsta(),(unsigned int)get_3D_overscan_input_len()<<1);
						Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN, get_3D_overscan_input_len() << 1);
					}
				}
			} // SG 3D disable IDMA 3D mode
			else if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == FALSE)
				&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_TOP_AND_BOTTOM) && (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_TOP_AND_BOTTOM) == TRUE))
			{
				Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN, temp/2);
				Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MEM_LEN_PRE, Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
			}
			else if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == FALSE)
				&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING) && (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
			{
				temp = min((unsigned int)(Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN) << 1),(unsigned int)usAfterDILen);
				Scaler_DispSetInputInfo(SLR_INPUT_CAP_LEN, temp);
				Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_LEN, temp >> 1);
				Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MEM_LEN_PRE, Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
				rtd_pr_vbe_debug("[SG][FP] Cap/Disp=%d/%d\n", Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN), Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
			}
#endif                
	}


	if (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)==SLR_MAIN_DISPLAY) {
		// due to bandwidth issue, we need to ask ap system for real input width
		//frank 3D test ++
			if(gGetInputWid)
				temp = (*gGetInputWid)(min(Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID),Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID)));
#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
			else if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == FALSE)
				&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF) && drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_SIDE_BY_SIDE_HALF)
				&& (Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID) <= 1920))
			{ // 4k2k Video SBS 3D mode, capture horizontal width can be full size
				temp = min(Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID) << 1, Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID));
			}else
#endif
#ifdef CONFIG_SDNR_CROP
  			{
				temp = min(Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID),Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID));
			//rtd_pr_vbe_emerg("file=%s, function=%s, line=%d\n", __FILE__, __FUNCTION__, __LINE__);
			//rtd_pr_vbe_emerg("temp=%d, SLR_INPUT_DISP_WID=%d, SLR_INPUT_SDNRH_ACT_WID=%d\n", temp, Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID), Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID));

  			}
#else
			temp = min(Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID),Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID));
#endif
#ifndef BUILD_QUICK_SHOW
			// 4k2k video horizontal width need scaling down to 1920 in TaB 3D mode (3840x2160 -> 1920x2160)
			if((temp > 1920)
				&& Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_TOP_AND_BOTTOM)
				&& (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_TOP_AND_BOTTOM) == TRUE))
			{
				temp = 1920;

                rtd_pr_vbe_debug("[vsg][orbit_dbg] scale down\n");
			}

			if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF))
				temp=drvif_scaler_3D_scale_down_width(temp);
#endif
#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
			// VD 27M (1440 Scaling down to 960 to use DI-5A)
			if(VD_27M_OFF == fw_video_get_27mhz_mode(SLR_MAIN_DISPLAY))
			{
				//do nothing
			}
			else// if(VD_27M_OFF != fw_video_get_27mhz_mode(SLR_MAIN_DISPLAY))
			{
				if(temp > 960)
				{
					rtd_pr_vbe_info("VDC 27M, set SLR_INPUT_CAP_WID, SLR_INPUT_MEM_ACT_WID from %d to 960\n", temp);
					temp = 960;
				}
			}
#endif //#ifdef CONFIG_ENABLE_VD_27MHZ_MODE

			//frank@1129 add below code to solve CUSTOM ratio mode memory not enough problem ++
             //        if((Scaler_InputSrcGetMainChType()==_SRC_VO) && (Scaler_DispGetRatioMode() == SLR_RATIO_CUSTOM) && (temp>REDUCE_DI_MEMORY_LIMIT_THRESHOLD))
             //                temp = REDUCE_DI_MEMORY_LIMIT_THRESHOLD;
                     //frank@1129 add below code to solve CUSTOM ratio mode memory not enough problem --

			Scaler_DispSetInputInfo(SLR_INPUT_CAP_WID,temp);

//			Scaler_DispSetInputInfo(SLR_INPUT_DI_WID,Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID));
#ifndef BUILD_QUICK_SHOW
#ifdef CONFIG_ENABLE_SE_ROTATE_M_DOMAIN_BLOCK_MODE
			//2k1k panel: 2k1k uzd to 1k1k, 4K2K  panel: 4k2k uzd to 2k1k
			if(_DISP_WID <= 1920 && _DISP_LEN <= 1080) {
				if(Get_rotate_function(SLR_MAIN_DISPLAY)&&(get_rotate_curState(SLR_MAIN_DISPLAY)==MAIN_ROTATE_INIT_UZD)){
					unsigned short tmpwid,tmplen,rotatetmp;
					tmpwid = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID);
					tmplen = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_LEN);
					if(((tmpwid >  _DISP_LEN ) || (tmplen> _DISP_LEN )))
					{
						if((get_rotate_mode(SLR_MAIN_DISPLAY)==(DIRECT_VO_FRAME_ORIENTATION)ROTATE_MODE_90)||(get_rotate_mode(SLR_MAIN_DISPLAY)==(DIRECT_VO_FRAME_ORIENTATION)ROTATE_MODE_270))
						{
							tmpwid = min(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_LEN),Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_SDNRH_ACT_WID));
							tmplen = min(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_WID),Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_SDNRV_ACT_LEN));
							Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID,tmplen);
							Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_LEN,tmpwid);

							if (tmpwid >_DISP_LEN) {
								Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID,_DISP_LEN);
							}
							if (tmplen >_DISP_LEN) {
								Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_LEN,_DISP_LEN);
							}
						}
						else{

							if(tmpwid >= _DISP_LEN)
							{
								rotatetmp=_DISP_LEN;
								Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID,rotatetmp);
							}

							if(tmplen >= _DISP_LEN)
							{
								rotatetmp=_DISP_LEN;
								Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_LEN,rotatetmp);
							}

						}
					}
					else
					{
						if((get_rotate_mode(SLR_MAIN_DISPLAY)==(DIRECT_VO_FRAME_ORIENTATION)ROTATE_MODE_90)||(get_rotate_mode(SLR_MAIN_DISPLAY)==(DIRECT_VO_FRAME_ORIENTATION)ROTATE_MODE_270))
						{
							tmpwid = min(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_LEN),Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_SDNRH_ACT_WID));
							tmplen = min(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_WID),Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_SDNRV_ACT_LEN));

							Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID,tmpwid);
							Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_LEN,tmplen);

							if (tmpwid >_DISP_LEN) {
								Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID,_DISP_LEN);
							}
							if (tmplen >_DISP_LEN) {
								Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_LEN,_DISP_LEN);
							}
						}
					}

					set_rotate_curState(SLR_MAIN_DISPLAY, MAIN_ROTATE_INIT_VI);//ready to initial VI via RPC

				}

			} else {
				if(Get_rotate_function(SLR_MAIN_DISPLAY)&&(get_rotate_curState(SLR_MAIN_DISPLAY)==MAIN_ROTATE_INIT_UZD)){
                    unsigned short tmpwid,tmplen;

                    tmpwid = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_SDNRH_ACT_WID);
                    if (is_4k_hfr_mode(SLR_MAIN_DISPLAY) == TRUE)
                    {
                        tmplen = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_SDNRV_ACT_LEN) / 2;
                    } else {
                        tmplen = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_SDNRV_ACT_LEN);
                    }

                    if((get_rotate_mode(SLR_MAIN_DISPLAY)==(DIRECT_VO_FRAME_ORIENTATION)ROTATE_MODE_90)||(get_rotate_mode(SLR_MAIN_DISPLAY)==(DIRECT_VO_FRAME_ORIENTATION)ROTATE_MODE_270))
                    {
                        tmpwid = min(tmpwid, Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_LEN));
                        tmplen = min(tmplen, Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_WID));
                    }
                    else{
                        tmpwid = min(tmpwid, Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_WID));
                        tmplen = min(tmplen, Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_LEN));
                    }

                    if (tmpwid < tmplen ) {
                        tmpwid = min(tmpwid, carveout_len);
                        tmplen = min(tmplen, carveout_width);
                    } else {
                        tmpwid = min(tmpwid, carveout_width);
                        tmplen = min(tmplen, carveout_len);
                    }

                    Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID, tmpwid);
                    Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_LEN, tmplen);

					set_rotate_curState(SLR_MAIN_DISPLAY, MAIN_ROTATE_INIT_VI);//ready to initial VI via RPC

				}
			}

#endif
#endif
#ifndef BUILD_QUICK_SHOW
			if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF)
				&& (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()|| (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_SIDE_BY_SIDE_HALF) == TRUE)))
			{
				Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_WID,min((unsigned int)Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID),(unsigned int)(Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID)>>1)));
				Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MEM_WID_PRE, Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID));
				if(Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE())
				Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_WID,(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID)*Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_H_RATIO)/100));
				Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_WID,(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID)&  ~_BIT0));
				if(get_3D_overscan_enable() == TRUE)
				{
					temp = min(Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID)-get_3D_overscan_input_hsta(),get_3D_overscan_input_wid()>>1);
					Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_WID,temp);
					Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_WID,(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID)&  ~_BIT0));
				}
				rtd_pr_vbe_debug("[SBS] WID of MEM_ACT/CAP/DISP=%x/%x/%x, H_Rat=%d\n",
					Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID), Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID), Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID),
					Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_H_RATIO));
			}
			else
 #endif               
			{
				Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_WID,temp);
                #ifndef BUILD_QUICK_SHOW
				if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()
					&& ((Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_TOP_AND_BOTTOM) || (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING)))
			        {
					Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MEM_WID_PRE, Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID));
					Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_WID,(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID)*Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_H_RATIO)/100));
					Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_WID,(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID)&  ~_BIT0));
					if(get_3D_overscan_enable() == TRUE)
					{
						temp = min(Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID)-get_3D_overscan_input_hsta(),get_3D_overscan_input_wid());
						Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_WID, temp);
						Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_WID,(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID)&  ~_BIT0));
					}
				}
               #endif
			}



			
#ifdef CONFIG_DUAL_CHANNEL
			
			Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_HSTA, 0);
			
#endif

	#ifdef CONFIG_ENABLE_DOT_BY_DOT_DISPLAY_RATIO_MODE
			if(_TRUE == Scaler_Get_AspectRatio_Enter_DotByDot_Mode())
			{
				rtd_pr_vbe_debug("fw_scaler_cal_scale_down_size, DotByDot Mode, not execute scaler_config_zoom_para\n");
			}
			else// if(_FALSE == Scaler_Get_AspectRatio_Enter_DotByDot_Mode())
			{
				if (Scaler_DispCheckRatio(SLR_RATIO_POINTTOPOINT)) {  // if display ratio is point2point, need to change memory setting
					scaler_config_zoom_para();
				}
			}
	#else //#ifdef CONFIG_ENABLE_DOT_BY_DOT_DISPLAY_RATIO_MODE
			if (Scaler_DispCheckRatio(SLR_RATIO_POINTTOPOINT)) {  // if display ratio is point2point, need to change memory setting
				scaler_config_zoom_para();
			}
	#endif //#ifdef CONFIG_ENABLE_DOT_BY_DOT_DISPLAY_RATIO_MODE


	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else{  // for sub
//frank@02012013 Change below code to solve set sub display window BUG ++

		{
			if(gGetInputWid)
				temp = (*gGetInputWid)(min(Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID),Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID)));
			else
#ifdef CONFIG_SDNR_CROP
  			{

					temp = min(Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID),Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID));

				//rtd_pr_vbe_emerg("file=%s, function=%s, line=%d\n", __FILE__, __FUNCTION__, __LINE__);
				//rtd_pr_vbe_emerg("temp=%d, SLR_INPUT_DISP_WID=%d, SLR_INPUT_SDNRH_ACT_WID=%d\n", temp, Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID), Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID));
  			}
#else
			temp = min(Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID),Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID));
#endif
		}


		if (temp > 1920) {/*reduce sub 4k2k input to 2k2k*/
			temp = 1920;
		}

		Scaler_DispSetInputInfo(SLR_INPUT_CAP_WID,temp);
		Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_WID,temp);
		 
		Scaler_DispSetInputInfo(SLR_INPUT_MEM_ACT_HSTA, 0);
		
		rtd_pr_vbe_emerg("fw_scaler_cal_scale_down_size:SLR_INPUT_CAP_WID=%d;;SLR_INPUT_MEM_ACT_WID=%d\n", Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID));

//frank@02012013 Change below code to solve set sub display window BUG --

	}
#endif
#ifndef BUILD_QUICK_SHOW
#ifdef CONFIG_ENABLE_SE_ROTATE_M_DOMAIN_BLOCK_MODE
	if((display == SLR_MAIN_DISPLAY) && Get_rotate_function(SLR_MAIN_DISPLAY) && (get_rotate_curState(SLR_MAIN_DISPLAY)==MAIN_ROTATE_INIT_VI)) {
		unsigned int align_width = 0,align_height = 0;
		if ((get_rotate_mode(SLR_MAIN_DISPLAY)==(DIRECT_VO_FRAME_ORIENTATION)ROTATE_MODE_90)||(get_rotate_mode(SLR_MAIN_DISPLAY)==(DIRECT_VO_FRAME_ORIENTATION)ROTATE_MODE_270)) {
			Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_WID,Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_INPUT_CAP_LEN));
			Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_LEN,Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_INPUT_CAP_WID));
			align_width=Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MEM_ACT_WID)%16;
			if (align_width) {
				unsigned int temp_width=  Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MEM_ACT_WID)-align_width;
				Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_LEN,temp_width);
				Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_WID, temp_width);
			}
			align_height=Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MEM_ACT_LEN)%16;
			if (align_height) {
				unsigned int temp_height=  Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MEM_ACT_LEN)-align_height;
				Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID,temp_height);
				Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_LEN, temp_height);
			}
		} else {
			Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_WID,Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_INPUT_CAP_WID));
			Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_LEN,Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_INPUT_CAP_LEN));
			align_width=Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MEM_ACT_WID)%16;
			if (align_width) {
				unsigned int temp_width=  Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MEM_ACT_WID)-align_width;
				Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID,temp_width);
				Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_WID, temp_width);
			}
			align_height=Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MEM_ACT_LEN)%16;
			if (align_height) {
				unsigned int temp_height=  Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MEM_ACT_LEN)-align_height;
				Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_LEN,temp_height);
				Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_LEN, temp_height);
			}
		}
	} else
#endif
#endif
	{
		if (get_hdmi_vrr_4k60_mode()) {
			Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_LEN,Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_LEN)/2);
		}
		else if ((((display == SLR_MAIN_DISPLAY)
            #ifndef BUILD_QUICK_SHOW
            && (Scaler_get_data_framesync(display) == FALSE)
            #endif
            && ((is_caveout_memory_buffer_source(display)) ||((Scaler_InputSrcGetType(display)==_SRC_VO) && is_4k_hfr_mode(display)))) ||(display == SLR_SUB_DISPLAY))
            && is_m_domain_need_scale_down(display))
        {//decide mdoamin size to caveout size
#ifndef CONFIG_RTK_KDRV_DV_IDK_DUMP

            if (Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_FRAMESYNC) == FALSE) {
                if ((Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_WID)> max_width )
                        &&(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_SDNRH_ACT_WID)>max_width )
                        && !((get_vsc_src_is_hdmi_or_dp())
                            && (Get_DISPLAY_REFRESH_RATE() == 120) && (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag()))) {
                    Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID,max_width );
                    Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_WID, max_width );
                }
                if ((Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_LEN)> max_len)
                    &&(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_SDNRV_ACT_LEN)> align(max_len, 4))
                    && !((get_vsc_src_is_hdmi_or_dp())
                            && (Get_DISPLAY_REFRESH_RATE() == 120) && (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag()))
                            ) {
                    Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_LEN,max_len);
                    Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_LEN, max_len);
                }
            }
#endif
        }
    }

  #ifdef CONFIG_SDNR_CROP
	if((Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID) > Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID)))
		  Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_H_DOWN,TRUE);
  	else
		  Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_H_DOWN,FALSE);

  #else
	if((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) > Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID)))
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_H_DOWN,TRUE);
	else
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_H_DOWN,FALSE);
  #endif
#ifndef BUILD_QUICK_SHOW  
	// ------------------------------------------------------------------------------------------------------------------------
	// force to enter vertical scaling down mode in 3D PR PQ Improvement if support that timing
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()
		&& ((Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)==SLR_3D_SIDE_BY_SIDE_HALF)
			|| (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING)
			|| (drvif_scaler3d_decide_3d_PR_enable_IDMA() == TRUE))
		&& (usAfterDILen > (Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN)>>1)))
	{
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_DOWN,TRUE);
	}
#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
	else if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == FALSE)
		&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING) && (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
	{
		if((Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN) << 1) < usAfterDILen)
			Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_DOWN,TRUE);
		else
			Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_DOWN,FALSE);
	}
#endif
#endif


  #ifdef CONFIG_SDNR_CROP
#ifndef BUILD_QUICK_SHOW
        else
#endif
        if ((display==SLR_MAIN_DISPLAY) && (is_4k_hfr_mode(display))) {
		if(((Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_LEN)>>1) > Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN)))
			Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_DOWN,TRUE);
		else
			Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_DOWN,FALSE);
  	}
	else if(Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_LEN) > Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN))
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_DOWN,TRUE);
  	else
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_DOWN,FALSE);
  #else
	else if(usAfterDILen > Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN))
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_DOWN,TRUE);
	else
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_DOWN,FALSE);
  #endif

    if(vbe_disp_oled_orbit_mode ==_VBE_PANEL_ORBIT_AUTO_MODE && vbe_disp_oled_orbit_enable &&  Scaler_get_data_framesync(display)){
            Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_H_UP,TRUE);
    }
#ifndef BUILD_QUICK_SHOW    
	else if((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE()) && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF)
		&& (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_SIDE_BY_SIDE_HALF) == TRUE)))
	{
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_H_UP,TRUE);
	} else if ((display==SLR_MAIN_DISPLAY) && (is_4k_hfr_mode(SLR_MAIN_DISPLAY))) {
		if(((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID)<<1) < Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID)))
			Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_H_UP,TRUE);
		else
			Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_H_UP,FALSE);
	}
#endif        
    else if(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) < Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID)){
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_H_UP,TRUE);
	}
#ifndef BUILD_QUICK_SHOW     
    else if (((Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE /*|| Get_DISPLAY_PANEL_BOW_RGBW() == TRUE*/) && get_vsc_run_pc_mode() == 1
		&& Get_DisplayMode_Src(SLR_MAIN_DISPLAY)  == VSC_INPUTSRC_JPEG && drvif_M_Plus_Status_Check() == 0) &&
		(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) == Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID))){	/* v_com pattern, turn off uzu, elieli*/
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_H_UP,FALSE);
	} 
#endif        
    else if((!get_vsc_run_adaptive_stream(display)) && (Scaler_DispGetStatus(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), SLR_DISP_422CAP)) && (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) == Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID))){
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_H_UP,TRUE);	 //PQ Request
	}
    else{
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_H_UP,FALSE);
	}
    if(vbe_disp_oled_orbit_mode ==_VBE_PANEL_ORBIT_AUTO_MODE && vbe_disp_oled_orbit_enable &&  Scaler_get_data_framesync(display))
        Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_UP,TRUE);
	else if(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN) < Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN))
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_UP,TRUE);
#ifndef BUILD_QUICK_SHOW 
    else if (((Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE /*|| Get_DISPLAY_PANEL_BOW_RGBW() == TRUE*/) && get_vsc_run_pc_mode() == 1
		&& Get_DisplayMode_Src(SLR_MAIN_DISPLAY)  == VSC_INPUTSRC_JPEG && drvif_M_Plus_Status_Check() == 0) &&
		(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) == Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID)))		/* v_com pattern, turn off uzu, elieli*/
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_UP,FALSE);
	else if ((display==SLR_MAIN_DISPLAY) && (is_4k_hfr_mode(SLR_MAIN_DISPLAY))) {
		if(((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN)<<1) < Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN)))
			Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_UP,TRUE);
		else
			Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_UP,FALSE);
	}
#endif
    else if((!get_vsc_run_adaptive_stream(display)) && (Scaler_DispGetStatus(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), SLR_DISP_422CAP)) && (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN) == Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN)))
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_UP,TRUE);		//PQ Request
	else
		Scaler_DispSetScaleStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_SCALE_V_UP,FALSE);

    if(Get_DisplayMode_Src(display) == VSC_INPUTSRC_CAMERA)
    {
        Scaler_DispSetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_V_UP, FALSE);
        Scaler_DispSetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_H_UP, FALSE);
        Scaler_DispSetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_V_DOWN, FALSE);
        Scaler_DispSetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_H_DOWN, FALSE);
    }

	//if (Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
	{
#ifdef BUILD_QUICK_SHOW
        printf2("InputWid=%d,InputLen=%d,InputFreq=%d\n", Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID),Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN),Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ));
        printf2("DiWid=%d,DiLen=%d,SdnrWid=%d,SdnrLen=%d\n",Scaler_DispGetInputInfo(SLR_INPUT_DI_WID),Scaler_DispGetInputInfo(SLR_INPUT_DI_LEN),Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID),Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_LEN));
        printf2("CapWid=%d,Caplen=%d,DispWid=%d,DispLen=%d\n", Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID),Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN),Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID),Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN));
        printf2("V_DOWN=%d,H_DOWN=%d,V_UP=%d,H_UP=%d\n", Scaler_DispGetScaleStatus(display,SLR_SCALE_V_DOWN),Scaler_DispGetScaleStatus(display,SLR_SCALE_H_DOWN),Scaler_DispGetScaleStatus(display,SLR_SCALE_V_UP),Scaler_DispGetScaleStatus(display,SLR_SCALE_H_UP));
#else
		//rtd_pr_vbe_emerg("IPV_ACT_LEN = 0x%x, usAfterDILen = 0x%x, IPH_ACT_WID = 0x%x\n", Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN), usAfterDILen, Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID));
		//rtd_pr_vbe_emerg("DiWid = 0x%x, DiLen = 0x%x \n",Scaler_DispGetInputInfo(SLR_INPUT_DI_WID),Scaler_DispGetInputInfo(SLR_INPUT_DI_LEN));
		rtd_pr_vbe_emerg("CapLen = %d, CapWid = %d\n", Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN), Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID));
		rtd_pr_vbe_emerg("MEM_ACT_LEN = %d, MEM_ACT_WID = %d\n", Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN), Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID));
		rtd_pr_vbe_emerg("SLR_INPUT_MEM_ACT_VSTA = %d, SLR_INPUT_MEM_ACT_HSTA = %d\n", Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA), Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA));
		rtd_pr_vbe_emerg("DispLen = %d, DispWid = %d\n", Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN), Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID));
		rtd_pr_vbe_emerg("IV_FREQ = %d\n", Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ));
		rtd_pr_vbe_emerg("SLR_SCALE_V_DOWN = %d, SLR_SCALE_H_DOWN = %d \n", Scaler_DispGetScaleStatus(display, SLR_SCALE_V_DOWN), Scaler_DispGetScaleStatus(display, SLR_SCALE_H_DOWN));
		rtd_pr_vbe_emerg("SLR_SCALE_V_UP = %d, SLR_SCALE_H_UP = %d \n", Scaler_DispGetScaleStatus(display, SLR_SCALE_V_UP), Scaler_DispGetScaleStatus(display, SLR_SCALE_H_UP));
		//rtd_pr_vbe_emerg("sdnrwinstart= 0x%x, sdnrlenstart = 0x%x\n", Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_STA), Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_STA));
		//rtd_pr_vbe_emerg("sdnrwin = 0x%x, sdnrlen = 0x%x\n", Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID), Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_LEN));

#endif

	if((display == SLR_MAIN_DISPLAY) && ((is_4k_hfr_mode(display)) || (get_hdmi_4k_hfr_mode() == HDMI_4K120_2_0) ||(get_hdmi_vrr_4k60_mode())))
	{//4k2k120 case copy m domain information to sub
		Scaler_DispSetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_CAP_WID, Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID));
		Scaler_DispSetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_CAP_LEN, Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN));
		Scaler_DispSetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_MEM_ACT_WID, Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID));
		Scaler_DispSetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_MEM_ACT_LEN, Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
		Scaler_DispSetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_MEM_ACT_HSTA, 0);
		Scaler_DispSetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_MEM_ACT_VSTA, 0);
		Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_V_DOWN, Scaler_DispGetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_V_DOWN));
		Scaler_DispSetScaleStatus(SLR_SUB_DISPLAY, SLR_SCALE_H_DOWN, Scaler_DispGetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_H_DOWN));
	}
}

}
#ifndef BUILD_QUICK_SHOW

void drvif_scaler_WriteTable(unsigned int* array)
{
	unsigned int	len,index;
	unsigned int	inc,all_zero;

	do {
		len = *array & (~(_AUTO_INC | _ALL_ZERO));
		if (len==0)
			break;

		inc = 0;//(*array & _AUTO_INC ? 1: 0);
		all_zero =  *array & _ALL_ZERO;
		array++;
		index = *array;
		array++;
		if(all_zero) {
			while(len--) {
				IoReg_Write32(index, 0);
				if(!inc)
					index+=4;
			}
		} else {
			while(len--) {
				IoReg_Write32(index, *array);
				if(!inc)
					index+=4;
				array++;
			}
		}
	} while (1);

}

unsigned char drvif_scaler_regGetInputWidCallBck(GET_INPUT_WID_F function)
{
	if( function == NULL )
		return FALSE;

	gGetInputWid = function ;

	return TRUE;
}


/*
 * 3D PR type enter 3DDMA IP
 */
// for darwin 1xDDR package, this flag should be set as 0
#define ENABLE_3D_PR_ENTER_3DDMA 0
unsigned char drvif_scaler3d_decide_3d_PR_enter_3DDMA(void)
{
	unsigned char ret = FALSE;
	return ret;
}

/*
 * 2Dcvt3D PR type enter 3DDMA IP
 */
unsigned char drvif_scaler3d_decide_2d_cvt_3d_PR_enter_3DDMA(void)
{
	unsigned char ret = FALSE;

#if ENABLE_3D_PR_ENTER_3DDMA
		ret = TRUE;
#endif

	return ret;
}


/*
 * Enabel IDMA for some PR 3D timing
 */
unsigned char drvif_scaler3d_decide_3d_PR_enable_IDMA(void)
{
	unsigned char result=FALSE;

#if 1//def ENABLE_DRIVER_I3DDMA
	if((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() == FALSE) || (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == FALSE))
		return FALSE;

	// support 3D timing: HDMI 1080iFP
	if((Scaler_DispGetInputInfo(SLR_INPUT_THROUGH_I3DDMA) == TRUE) && (get_HDMI_HDR_mode() == HDR_MODE_DISABLE))
		result = TRUE;
#endif
	return result;
}

extern VO_3D_MODE_TYPE get_vo_3d_mode(void);
unsigned char drvif_scaler3d_decide_HDMI_framePacking_interlaced_status(void)
{
	unsigned int usLength;
	usLength = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);

	// HDMI FP IPV 1080i=2228, 576i=1226, 486i=1028
	if((usLength == 2228) || (usLength == 1226) || (usLength == 1028))
		return TRUE;
	//else if(get_vo_3d_mode() == VO_3D_FRAME_PACKING && usLength == 540 )
	//	return TRUE;
	else if(get_field_alternative_3d_mode_enable())
		 return TRUE;
	else
		return FALSE;

}

unsigned char drvif_scaler3d_decide_HDMI_framePacking_auto_status(SCALER_DISP_CHANNEL channel)
{
	unsigned int usLength = 0;
	usLength = Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_IPV_ACT_LEN_PRE);
	// HDMI FP IPV 1080i=2228, 576i=1226, 486i=1028
	if((usLength == 2228) || (usLength == 1226) || (usLength == 1028))
		return TRUE;
	else if((usLength == 2205) || (usLength == 1470) || (usLength == 1005) || (usLength == 1201))
		return TRUE;
	else
		return FALSE;

}

/*
unsigned char drvif_scaler3d_decide_HDMI_framePacking_auto_status(void)
{
	unsigned int usLength;
	usLength = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);

	// HDMI FP IPV 1080i=2228, 576i=1226, 486i=1028
	if((usLength == 2228) || (usLength == 1226) || (usLength == 1028))
		return TRUE;
	else if((usLength == 2205) || (usLength == 1470) || (usLength == 1005) || (usLength == 1201))
		return TRUE;
	else
		return FALSE;

}
*/

unsigned char drvif_scaler3d_decide_3d_SG_Disable_IDMA_suport_FP_interlace(void)
{
	// [NOTE] Magellan HW issue, if don't want enable IDMA 3D in HDMI interlaced FP 3D video then need enable 3DTG in this condition
	return FALSE;
}


// check if current source/3D type need use IDMA
unsigned char drvif_scaler3d_decide_3d_SG_Disable_IDMA(unsigned char uc3dType)
{
	unsigned char result=FALSE;
#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
	unsigned char srcType = 0;

	// for SG 3D only
	if((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() == FALSE) || (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE))
		return FALSE;

	// SBS/TaB 3D mode don't need IDMA
	if((uc3dType == SLR_3D_SIDE_BY_SIDE_HALF)|| (uc3dType == SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D)
		|| (uc3dType == SLR_3D_TOP_AND_BOTTOM)|| (uc3dType == SLR_3D_TOP_AND_BOTTOM_CVT_2D)
		|| ((Scaler_InputSrcGetMainChType() == _SRC_HDMI) && ((uc3dType == SLR_3D_FRAME_PACKING)|| (uc3dType == SLR_3D_FRAME_PACKING_CVT_2D))
			&& ((drvif_scaler3d_decide_HDMI_framePacking_interlaced_status() == FALSE) || drvif_scaler3d_decide_3d_SG_Disable_IDMA_suport_FP_interlace())))
	{
		return TRUE;
	}

  #ifdef ENABLE_DRIVER_I3DDMA
	if(Scaler_Get_forceEnable_VO_UZU() == FALSE){
		UINT16 usWidth, usLength;
		// VO/HDMI/YPbPr source 3D mode use IDMA in current default
		srcType = modestate_I3DDMA_get_2dSrcType();
		usWidth = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
		usLength = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
		if((usWidth > 1920) && (usLength > 1080)){
			// 4k2k video support only SBS/TaB 3D mode in IDMA bypass mode
			//rtd_pr_vbe_info("[4k2k] bypassIDMA\n");
			return TRUE;
		}else if((srcType == _SRC_VO) || (srcType == _SRC_HDMI) || (srcType == _SRC_YPBPR)){
			//rtd_pr_vbe_info("[IDMA] IDMA=enabled!\n");
			return FALSE;
		}
	}
  #endif
#endif

	return result;
}




/*
 * Force do SG 3D FRC in M-domain for 2Dcvt3D mode
 */
unsigned char drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain(void)
{
	unsigned char ret = FALSE;
	unsigned cur3dType = Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE);

	// SG 2Dcvt3D repeat frame in M-display (3D sequence out enable)
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == FALSE)
		&& ((cur3dType == SLR_3D_2D_CVT_3D) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(cur3dType) == TRUE))  )
	{
		ret = TRUE;
	}
	return ret;
}




/*
 * VO output LR frame sequence
 */
#define ENABLE_VO_3D_OUTPUT_FRAME_SEQUENCE 1
unsigned char drvif_scaler3d_decide_vo_3d_output_frame_seq(void)
{
	unsigned char ret = FALSE;

#if ENABLE_VO_3D_OUTPUT_FRAME_SEQUENCE
	if (Get_PANEL_3D_SG_OUTPUT_120HZ_ENABLE()){
			ret = FALSE;
	}else if (drvif_scaler3d_decide_3d_PR_enter_3DDMA() == FALSE)
		ret = FALSE;
	else
		ret = TRUE;
#endif
	return ret;
}

/*
 * M-domain output LR frame sequence
 */
#define ENABLE_DISP_M_OUTPUT_FRAME_SEQUENCE 1 // 1
unsigned char drvif_scaler3d_decide_dispM_3d_output_frame_seq(void)
{
	unsigned char ret = FALSE;
#if ENABLE_DISP_M_OUTPUT_FRAME_SEQUENCE
	if (Get_PANEL_3D_SG_OUTPUT_120HZ_ENABLE())
		ret = TRUE;
	else if (drvif_scaler3d_decide_3d_PR_enter_3DDMA() == FALSE)
		ret = FALSE;
	else
		ret = TRUE;
#endif
	return ret;
}

/*
 * D-domain output 120Hz for PR type panel
 */
unsigned char drvif_scaler3d_decide_PR_output_120Hz(void)
{
	unsigned char ret = FALSE;
	if (Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() &&
		Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() &&
		Get_DISPLAY_PORT() == 2)
	{
		ret = TRUE;
	}
	return ret;
}

/*
* Consider bandwidth issue, we can let 3D PR Top and Bottom 1080i enter 2DDI to reduce.
*/
#define ENABLE_3D_PR_TOP_AND_BOTTOM_1080I_ENTER_2DDI 0
unsigned char drvif_scaelr3d_decide_1080i_TB_enter_2ddi(void)
{
	unsigned char ret = FALSE;

#if ENABLE_3D_PR_TOP_AND_BOTTOM_1080I_ENTER_2DDI
	ret = TRUE;
#endif

	return ret;
}

unsigned char drvif_scaelr3d_decide_is_3Dto2D_display_mode(void)
{
	if((Scaler_Get3DMode((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)) >= SLR_3DMODE_3D_AUTO_CVT_2D) && (Scaler_Get3DMode((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)) <= SLR_3DMODE_3D_SENSIO_CVT_2D))
		return TRUE;
	else
		return FALSE;

}

unsigned char drvif_scaelr3d_decide_is_3D_display_mode(void)
{
	unsigned char result = 0;
	if((Scaler_Get3DMode((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)) == SLR_3DMODE_2D) ||((Scaler_Get3DMode((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)) >= SLR_3DMODE_3D_AUTO_CVT_2D) && (Scaler_Get3DMode((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)) <= SLR_3DMODE_3D_SENSIO_CVT_2D)))
		return FALSE;
	else
		return TRUE;

	return result;
}


SLR_2DCVT3D_MODE drvif_scaler3d_decide_2d_cvt_3d_mode(void)
{
	return SLR_2DCVT3D_MODE_DISABLE;
}


unsigned char drvif_scaler_decide_freerun_60hz(void)
{
#ifdef CONFIG_PANEL_auo_55_T550HVD02
	return 1;
#else
  // Don't enable double DVS in 2D mode of ICS paltform (double DVS ON/OFF may cause panel un-stable in ICS main page)
  #if 1//#ifdef TV_ICS_PLATFORM
	unsigned char freeRunEn=0;
	unsigned int frameRate = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
	unsigned char bIn3D = drvif_scaelr3d_decide_is_3D_display_mode();

	// [MEMC] Keep video timing frame sync in 2D MEMC enable mode (via double DVS)
	freeRunEn = (bIn3D == FALSE) && (frameRate < 310)
				&& ((scalerdisplay_get_2D_MEMC_support() == FALSE)|| (Scaler_DispGetRatioMode() == SLR_RATIO_CUSTOM)|| (frameRate < 230));
	rtd_pr_vbe_debug("[ICS][2D] FreeRun=%d, 3D/FR=%d/%d\n", freeRunEn, bIn3D, frameRate);
	return freeRunEn;
  #else
	return 0;
  #endif
#endif

	return 0;

}


unsigned char drvif_scaler3d_decide_isPR3dMode(void)
{
	UINT8 result = 0;
	// Magellan PR 3D display mode condition
	result = Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode();

	return result;
}


// 0: IDMA not active, 1: In IDMA 3D mode, 2: In IDMA 3Dto2D mode
SLR_IDMA_MODE drvif_scaler3d_get_IDMA3dMode(void)
{
	SLR_IDMA_MODE result = SLR_IDMA_MODE_DISABLE;

#ifdef ENABLE_DRIVER_I3DDMA
	if(modestate_I3DDMA_get_In3dMode() == FALSE)
		result = SLR_IDMA_MODE_DISABLE; // Not in IDMA 3D mode
	else if(drvif_scaelr3d_decide_is_3D_display_mode())
		result = SLR_IDMA_MODE_3D; // In IDMA 3D mode
	else
		result = SLR_IDMA_MODE_3Dto2D; // In IDMA 3D to 2D mode
#endif
	return result;
}

// check if current SG 3D in data FRC mode
unsigned char drvif_scaler3d_decide_3d_SG_data_FRC(void)
{
#ifndef ENABLE_FORCE_MVC_3D_IN_DATA_FRC // for 3D mode in data frame sync path condition
		SLR_VOINFO* pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
#endif

	// for SG 3D only
	if((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() == FALSE) || (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE))
		return FALSE;

#ifndef ENABLE_FORCE_MVC_3D_IN_DATA_FRC // for 3D mode in data frame sync path condition
	if((Scaler_InputSrcGetMainChType() == _SRC_VO) && (pVOInfo && (pVOInfo->mode_3d == VO_3D_FRAME_PACKING)) && (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == TRUE))
		return FALSE;
#endif

	// IDMA 3D/3Dto2D or MVC 3D data FRC mode force enter M-domain SG data FRC 3D mode
	return (modestate_I3DDMA_check_idma_mode_enable(Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE))
			|| modestate_get_vo3d_in_framePacking_FRC_mode());

return 0;
}




#define ENABLE_3D_PR_VIDEO_PQ_IMPROVEMENT 0
unsigned char drvif_scaler3d_decide_3d_PR_PQ_improve(void)
{
	unsigned char ret = FALSE;

	return ret;
}


void drvif_scaler3d_set_voDropLine_for_4k2k_3d(unsigned char enable)
{
#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // [SG ] VO drop control for 4k2k 3D video
	rtd_pr_vbe_debug("[VO] 4k2k dropLine = %d\n", enable);
	voDropLine_for4k2k_3d = enable;
#endif

	return;
}


unsigned char drvif_scaler3d_get_voDropLine_for_4k2k_3d(void)
{
#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // [SG ] VO drop control for 4k2k 3D video
	return voDropLine_for4k2k_3d;
#endif
	return FALSE;
}


unsigned int drvif_scaler3d_get_vo_3dDropLineMode(void)
{
	unsigned int dropLineMode=0;
	// Drop Line for PR 3D data FRC bandwidth issue
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE())
		dropLineMode |= MODE_3D_MVC;
#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // [SG ] VO drop control for 4k2k 3D video
	else if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
		dropLineMode = drvif_scaler3d_get_voDropLine_for_4k2k_3d();
#endif

	return dropLineMode;
}



int drvif_scaler3d_set_panel_info(void)
{
	int ret=0;
	unsigned int *pulTemp = NULL;
	unsigned int ulCount = 0, i = 0;
	PANEL_INFO tPanelInfo;

	// Set VTotal info to video cpu
	tPanelInfo.ulVTotal = Get_DISP_VERTICAL_TOTAL();
	tPanelInfo.ulDEN_STA_VPOS = Get_DISP_DEN_STA_VPOS();
	tPanelInfo.ulDEN_END_VPOS = Get_DISP_DEN_END_VPOS();
	tPanelInfo.ul3DPanelType = Get_PANEL_3D_PANEL_TYPE();
	tPanelInfo.ulPanelVflip =Get_PANEL_VFLIP_ENABLE();
        tPanelInfo.memc_bypass_info = get_MEMC_bypass_status_refer_platform_model();

	pulTemp = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_SET_PANEL_INFO);
	ulCount = sizeof(PANEL_INFO) / sizeof(unsigned int);
	// copy to RPC share memory
	memcpy(pulTemp, &tPanelInfo, sizeof(PANEL_INFO));

	for (i = 0; i < ulCount; i++)
		pulTemp[i] = htonl(pulTemp[i]);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_SET_PANEL_INFO,0,0)))
			rtd_pr_vbe_debug("ret=%d, send SCALERIOC_SET_PANEL_INFO to driver fail !!!\n", ret );

	return ret;
}


UINT8 drv_scaler_vo_isEnterDVS_Condition(void)
{
	if(Get_PANEL_TYPE_IS_MILLET3_4K2K_SERIES() == 1)
			return FALSE;
	else{
		if(scalerdisplay_get_2D_MEMC_support() == 1)
			return FALSE;

        if(get_MEMC_bypass_status_refer_platform_model() == TRUE)
        {
            return TRUE;
        }
	}

	return DEFAULT_ENABLE_DOUBLE_DVS;
}

#if 0
//added by rika 20140515 to force framerate 60 for netflix
#define DEFAULT_ENABLE_FORCE_FRAMERATE TRUE
#define DEFAULT_FORCE_FRAMERATE 59940
#endif
static VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT tVoutFRCtrlStruct;

void drvif_scaler_voFrameRateControl_init(void)
{
	//CLNT_STRUCT clnt;
	int i;
	VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT * tVoutFRCtrlStructlocal;
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long ret = 0;
#endif


	// 2D /3D mode -- data frame sync path
	// Dobuling the frame rate in playback source by VO for frame sync and panel compatibility issues
	tVoutFRCtrlStruct.curDispMode = _DISP_2D;
	for(i=0; i < _DISP_MODE_NUM; i++){
//#ifdef CONFIG_MEMC_BYPASS
#if 0
                tVoutFRCtrlStruct.enableForceFrameRate[i]       = TRUE;
                tVoutFRCtrlStruct.ForceFrameRate[i]             = 60000; // 59940
#else
	  	tVoutFRCtrlStruct.enableForceFrameRate[i] 	= DEFAULT_ENABLE_FORCE_FRAMERATE; // FALSE
		tVoutFRCtrlStruct.ForceFrameRate[i] 		= DEFAULT_FORCE_FRAMERATE; // 59940
#endif
		tVoutFRCtrlStruct.enableDoubleDvs[i]		= drv_scaler_vo_isEnterDVS_Condition();
        //tVoutFRCtrlStruct.enableDoubleDvs[i]		= DEFAULT_ENABLE_DOUBLE_DVS;
		tVoutFRCtrlStruct.DoubleDvsThreshold[i] 		= DEFAULT_DOUBLE_DVS_THRESHOLD; // 31000
		tVoutFRCtrlStruct.enableHighBound[i] 		= DEFAULT_ENABLE_HIGH_BOUND; // TRUE
#if 0
		if(Get_DISPLAY_PANEL_TYPE() == P_DISP_TO_TVOX){
			tVoutFRCtrlStruct.HighBound[i] 			= 31000;
		}
		else
#endif
		{
			tVoutFRCtrlStruct.HighBound[i] 			= DEFAULT_HIGH_BOUND; // 60000
		}
		tVoutFRCtrlStruct.HighAdjust[i] 				= DEFAULT_HIGH_ADJUST; // 60000
		tVoutFRCtrlStruct.enableLowBound[i] 		= DEFAULT_ENABLE_LOW_BOUND;
#ifndef CONFIG_MEMC_BYPASS
		tVoutFRCtrlStruct.LowBound[i]				= DEFAULT_LOW_BOUND_FOR_MEMC;//(scalerdisplay_get_2D_MEMC_support()? DEFAULT_LOW_BOUND_FOR_MEMC: DEFAULT_LOW_BOUND); // 50000
#else
        tVoutFRCtrlStruct.LowBound[i]				= DEFAULT_LOW_BOUND;
#endif
		tVoutFRCtrlStruct.LowAdjust[i] 				= DEFAULT_LOW_ADJUST; // 59940
	}

	// if the default value is be changed, MUST sync the table with Video FW
	tVoutFRCtrlStruct.opCode = _OPCODE_TABLE_CHANGE;
#if 0
	clnt = prepareCLNT(BLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);
	HRESULT* hr = VIDEO_RPC_VOUT_ToAgent_VodmaFrameRateControl_0(&tVoutFRCtrlStruct, &clnt);

	if (-1 == (long)hr) {
		rtd_pr_vbe_info("Send RPC fail@%s\n", __FUNCTION__);
		ret = (long)hr;
	}
	// NONBLOCK_MODE don't need to free memory
	else {
		if (NULL != hr) {
			ret = *hr;
			free(hr);
		}
	}
#else

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
	if (vir_addr) {
		phy_addr = (unsigned int)dvr_to_phys((void *)vir_addr);
		tVoutFRCtrlStructlocal = (VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT *)vir_addr_noncache;

		tVoutFRCtrlStructlocal->opCode = tVoutFRCtrlStruct.opCode;
		tVoutFRCtrlStructlocal->curDispMode = tVoutFRCtrlStruct.curDispMode;
		for(i=0; i < _DISP_MODE_NUM; i++){
			tVoutFRCtrlStructlocal->enableForceFrameRate[i] = tVoutFRCtrlStruct.enableForceFrameRate[i];
			tVoutFRCtrlStructlocal->ForceFrameRate[i] = htonl(tVoutFRCtrlStruct.ForceFrameRate[i]);
			tVoutFRCtrlStructlocal->enableDoubleDvs[i] = tVoutFRCtrlStruct.enableDoubleDvs[i];
			tVoutFRCtrlStructlocal->DoubleDvsThreshold[i] = htonl(tVoutFRCtrlStruct.DoubleDvsThreshold[i]);
			tVoutFRCtrlStructlocal->enableHighBound[i] = tVoutFRCtrlStruct.enableHighBound[i];
			tVoutFRCtrlStructlocal->HighBound[i] = htonl(tVoutFRCtrlStruct.HighBound[i]);
			tVoutFRCtrlStructlocal->HighAdjust[i] = htonl(tVoutFRCtrlStruct.HighAdjust[i]);
			tVoutFRCtrlStructlocal->enableLowBound[i] = tVoutFRCtrlStruct.enableLowBound[i];
			tVoutFRCtrlStructlocal->LowBound[i] = htonl(tVoutFRCtrlStruct.LowBound[i]);
			tVoutFRCtrlStructlocal->LowAdjust[i] = htonl(tVoutFRCtrlStruct.LowAdjust[i]);
		}

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
		if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_VodmaFrameRateControl, phy_addr, 0, &ret))
			rtd_pr_vbe_debug("RPC fail!!\n");
#endif
		dvr_free((void *)vir_addr);
	}
#endif
	return;
}

void drvif_scaler_voFrameRateControl_force(unsigned char iEnable,unsigned int theframerate)
{
#if 1
	//CLNT_STRUCT clnt;
	int i;
	VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT * tVoutFRCtrlStructlocal;
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long ret = 0;
#endif
	//theframerate=DEFAULT_FORCE_FRAMERATE;
	// 2D /3D mode -- data frame sync path
	// Dobuling the frame rate in playback source by VO for frame sync and panel compatibility issues
	tVoutFRCtrlStruct.curDispMode = _DISP_2D;
	#if 0
	if(iEnable)
		rtd_pr_vbe_info("_rk force vo framerate enable\n");
	else
		rtd_pr_vbe_info("_rk force vo framerate disable\n");
	#endif

	for(i=0; i < _DISP_MODE_NUM; i++){
		tVoutFRCtrlStruct.enableForceFrameRate[i]	= iEnable; // FALSE
		tVoutFRCtrlStruct.ForceFrameRate[i] 		= theframerate; // 59940
		tVoutFRCtrlStruct.enableDoubleDvs[i]		= drv_scaler_vo_isEnterDVS_Condition();
		tVoutFRCtrlStruct.DoubleDvsThreshold[i] 		= DEFAULT_DOUBLE_DVS_THRESHOLD; // 31000
		tVoutFRCtrlStruct.enableHighBound[i]		= DEFAULT_ENABLE_HIGH_BOUND; // TRUE
		#if 0
		if(Get_DISPLAY_PANEL_TYPE() == P_DISP_TO_TVOX){
			tVoutFRCtrlStruct.HighBound[i]			= 31000;
		}
		else
		#endif
		{
			tVoutFRCtrlStruct.HighBound[i]			= DEFAULT_HIGH_BOUND; // 60000
		}
		tVoutFRCtrlStruct.HighAdjust[i] 				= DEFAULT_HIGH_ADJUST; // 60000
		tVoutFRCtrlStruct.enableLowBound[i] 		= DEFAULT_ENABLE_LOW_BOUND;
		tVoutFRCtrlStruct.LowBound[i]				= DEFAULT_LOW_BOUND_FOR_MEMC;//(scalerdisplay_get_2D_MEMC_support()? DEFAULT_LOW_BOUND_FOR_MEMC: DEFAULT_LOW_BOUND); // 50000
		tVoutFRCtrlStruct.LowAdjust[i]				= DEFAULT_LOW_ADJUST; // 59940
	}

	// if the default value is be changed, MUST sync the table with Video FW
	tVoutFRCtrlStruct.opCode = _OPCODE_TABLE_CHANGE;
#if 0
	clnt = prepareCLNT(BLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);
	HRESULT* hr = VIDEO_RPC_VOUT_ToAgent_VodmaFrameRateControl_0(&tVoutFRCtrlStruct, &clnt);

	if (-1 == (long)hr) {
		rtd_pr_vbe_info("Send RPC fail@%s\n", __FUNCTION__);
		ret = (long)hr;
	}
	// NONBLOCK_MODE don't need to free memory
	else {
		if (NULL != hr) {
			ret = *hr;
			free(hr);
		}
	}
#else

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
	if (vir_addr) {
		phy_addr = (unsigned int)dvr_to_phys((void *)vir_addr);
		tVoutFRCtrlStructlocal = (VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT *)vir_addr_noncache;

		tVoutFRCtrlStructlocal->opCode = tVoutFRCtrlStruct.opCode;
		tVoutFRCtrlStructlocal->curDispMode = tVoutFRCtrlStruct.curDispMode;
		for(i=0; i < _DISP_MODE_NUM; i++){
			tVoutFRCtrlStructlocal->enableForceFrameRate[i] = tVoutFRCtrlStruct.enableForceFrameRate[i];
			tVoutFRCtrlStructlocal->ForceFrameRate[i] = htonl(tVoutFRCtrlStruct.ForceFrameRate[i]);
			tVoutFRCtrlStructlocal->enableDoubleDvs[i] = tVoutFRCtrlStruct.enableDoubleDvs[i];
			tVoutFRCtrlStructlocal->DoubleDvsThreshold[i] = htonl(tVoutFRCtrlStruct.DoubleDvsThreshold[i]);
			tVoutFRCtrlStructlocal->enableHighBound[i] = tVoutFRCtrlStruct.enableHighBound[i];
			tVoutFRCtrlStructlocal->HighBound[i] = htonl(tVoutFRCtrlStruct.HighBound[i]);
			tVoutFRCtrlStructlocal->HighAdjust[i] = htonl(tVoutFRCtrlStruct.HighAdjust[i]);
			tVoutFRCtrlStructlocal->enableLowBound[i] = tVoutFRCtrlStruct.enableLowBound[i];
			tVoutFRCtrlStructlocal->LowBound[i] = htonl(tVoutFRCtrlStruct.LowBound[i]);
			tVoutFRCtrlStructlocal->LowAdjust[i] = htonl(tVoutFRCtrlStruct.LowAdjust[i]);
		}

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
		if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_VodmaFrameRateControl, phy_addr, 0, &ret))
			rtd_pr_vbe_debug("RPC fail!!\n");
#endif
		dvr_free((void *)vir_addr);
	}


#endif
	return;
#endif
}

unsigned int g_voFrameRateControl_lowBound_value[2] = {0,0};

unsigned int drvif_scaler_voFrameRateControl_get_lowBound(DISP_MODE flag_3d)
{
	return g_voFrameRateControl_lowBound_value[flag_3d];
}

// Set video output frame rate parameter
int drvif_scaler_voFrameRateControl_set_bwSavingMode(DISP_MODE flag_3d, unsigned char bwSavingEnable)
{
	VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT * tVoutFRCtrlStructlocal;
	int i;
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;
    SLR_VOINFO *pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long ret = 0;
#endif
	rtd_pr_vbe_debug("[SLR] Mode[%d], BW Saving=%d\n", flag_3d, bwSavingEnable);
    if (scaler_vsc_get_keep_vo_framerate_control() == 1)
    {
       rtd_pr_vbe_notice("[SLR] keep_vo_framerate_control @ %s\n", __FUNCTION__);
       return FALSE;
    }

	// VO frame rate control -- ONLY FOR VO SOURCE
	if(bwSavingEnable){
		// 3D mode -- data FRC path@120
		// Don't rising VO frame rate in playback source when display timing is FRC@120Hz (saving bandwidth)
	  	tVoutFRCtrlStruct.enableForceFrameRate[flag_3d]	= FALSE;
		tVoutFRCtrlStruct.enableDoubleDvs[flag_3d] 		= FALSE;
		tVoutFRCtrlStruct.enableLowBound[flag_3d] 		= TRUE;
        tVoutFRCtrlStruct.LowBound[flag_3d] 		    = DEFAULT_LOW_BOUND_FOR_MEMC;
        tVoutFRCtrlStruct.LowAdjust[flag_3d]            = DEFAULT_LOW_ADJUST;
    }
	else{
	  	tVoutFRCtrlStruct.enableForceFrameRate[flag_3d]	= FALSE;
		tVoutFRCtrlStruct.enableDoubleDvs[flag_3d] 		= TRUE;
		tVoutFRCtrlStruct.enableLowBound[flag_3d] 		= TRUE;
		if(scaler_vsc_get_adaptive_pst_lowdelay_mode() == 1){
			if((Get_DISPLAY_REFRESH_RATE() < 120)&&(Scaler_MEMC_GetMotionType() == VPQ_MEMC_TYPE_55_PULLDOWN) &&(fwif_color_get_cinema_mode_en() == 1)){ //tru-motion off + real cinema
				// 24hz to 48hz
				tVoutFRCtrlStruct.LowBound[flag_3d] = DEFAULT_LOW_BOUND;
                tVoutFRCtrlStruct.LowAdjust[flag_3d]= DEFAULT_LOW_ADJUST;
			}else{
				// 24hz to 60hz
				if ((get_vt_EnableFRCMode() != FALSE) &&(Get_DISPLAY_REFRESH_RATE() >= 120) && (pVOInfo->is2KCP == 1)) {
					// 24hz to 120hz
					tVoutFRCtrlStruct.LowBound[flag_3d] = 99500;
					tVoutFRCtrlStruct.LowAdjust[flag_3d]=120000;
				} else {
					// 24hz to 60hz
					tVoutFRCtrlStruct.LowBound[flag_3d] = 49000;
					tVoutFRCtrlStruct.LowAdjust[flag_3d]= DEFAULT_LOW_ADJUST;
				}
			}

		}else{
        	tVoutFRCtrlStruct.LowBound[flag_3d] = DEFAULT_LOW_BOUND;
            tVoutFRCtrlStruct.LowAdjust[flag_3d]= DEFAULT_LOW_ADJUST;
		}
	}
	g_voFrameRateControl_lowBound_value[flag_3d] = tVoutFRCtrlStruct.LowBound[flag_3d];
	rtd_pr_vbe_notice("[SLR] BW Saving=%d, LowBound = %d \n", bwSavingEnable, tVoutFRCtrlStruct.LowBound[flag_3d]);

	tVoutFRCtrlStruct.opCode = _OPCODE_TABLE_CHANGE;
#if 0
	clnt = prepareCLNT(BLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);
	HRESULT* hr = VIDEO_RPC_VOUT_ToAgent_VodmaFrameRateControl_0(&tVoutFRCtrlStruct, &clnt);

	if (-1 == (long)hr) {
		rtd_pr_vbe_info("Send RPC fail@%s\n", __FUNCTION__);
		ret = (long)hr;
	}
	// NONBLOCK_MODE don't need to free memory
	else {
		if (NULL != hr) {
			ret = *hr;
			free(hr);
		}
	}
#else

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
	if (vir_addr) {
		phy_addr = (unsigned int)dvr_to_phys((void *)vir_addr);
		tVoutFRCtrlStructlocal = (VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT *)vir_addr_noncache;

		tVoutFRCtrlStructlocal->opCode = tVoutFRCtrlStruct.opCode;
		tVoutFRCtrlStructlocal->curDispMode = tVoutFRCtrlStruct.curDispMode;
		for(i=0; i < _DISP_MODE_NUM; i++){
			tVoutFRCtrlStructlocal->enableForceFrameRate[i] = tVoutFRCtrlStruct.enableForceFrameRate[i];
			tVoutFRCtrlStructlocal->ForceFrameRate[i] = htonl(tVoutFRCtrlStruct.ForceFrameRate[i]);
			tVoutFRCtrlStructlocal->enableDoubleDvs[i] = tVoutFRCtrlStruct.enableDoubleDvs[i];
			tVoutFRCtrlStructlocal->DoubleDvsThreshold[i] = htonl(tVoutFRCtrlStruct.DoubleDvsThreshold[i]);
			tVoutFRCtrlStructlocal->enableHighBound[i] = tVoutFRCtrlStruct.enableHighBound[i];
			tVoutFRCtrlStructlocal->HighBound[i] = htonl(tVoutFRCtrlStruct.HighBound[i]);
			tVoutFRCtrlStructlocal->HighAdjust[i] = htonl(tVoutFRCtrlStruct.HighAdjust[i]);
			tVoutFRCtrlStructlocal->enableLowBound[i] = tVoutFRCtrlStruct.enableLowBound[i];
			tVoutFRCtrlStructlocal->LowBound[i] = htonl(tVoutFRCtrlStruct.LowBound[i]);
			tVoutFRCtrlStructlocal->LowAdjust[i] = htonl(tVoutFRCtrlStruct.LowAdjust[i]);
		}

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
		if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_VodmaFrameRateControl, phy_addr, 0, &ret)) {
			rtd_pr_vbe_debug("RPC fail!!\n");
			dvr_free((void *)vir_addr);
			return -1;
		}
#endif
		dvr_free((void *)vir_addr);
		return FALSE;
	}
#endif
	return TRUE;
}

#ifdef RUN_ON_TVBOX

int drvif_scaler_voForceFrameRateControl_by_TvBox(UINT32 forceFrameRate, unsigned char bEnable)
{
	CLNT_STRUCT clnt;
	int ret=0;

	rtd_pr_vbe_debug("[%s] Force FrameRate = %d, enable=%d\n",  __FUNCTION__, forceFrameRate, bEnable);

	tVoutFRCtrlStruct.curDispMode = _DISP_2D;

	if(bEnable == _TRUE){
	  	tVoutFRCtrlStruct.enableForceFrameRate[_DISP_2D] 	= 1;
		tVoutFRCtrlStruct.ForceFrameRate[_DISP_2D] 		= forceFrameRate;
	}
	else{
	  	tVoutFRCtrlStruct.enableForceFrameRate[_DISP_2D] 	= 0;
	}

	tVoutFRCtrlStruct.opCode = _OPCODE_TABLE_CHANGE;
	clnt = prepareCLNT(BLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);
	HRESULT* hr = VIDEO_RPC_VOUT_ToAgent_VodmaFrameRateControl_0(&tVoutFRCtrlStruct, &clnt);

	if (-1 == (long)hr) {
		rtd_pr_vbe_debug("Send RPC fail@%s\n", __FUNCTION__);
		ret = (long)hr;
	}
	// NONBLOCK_MODE don't need to free memory
	else {
		if (NULL != hr) {
			ret = *hr;
			free(hr);
		}
	}

	return ret;

}

#endif

// [4k2k] force all case frame rate limit under 60Hz -- For CMI test
// LVDS output frame rate:
//	- 2D mode             	: 50/60Hz
//	- MVC 3D              	: 60Hz
//	- 4k2k photo/video	: 60Hz
//	- other 3D mode     	: 120Hz

//#define SLR_4K2K_FORCE_DISPLAY_OUTPUT_ON_60HZ
unsigned char drvif_scaler4k2k_get_forceDisplayOutput_60Hz(void)
{
	unsigned char result = FALSE;

	// [MEMC] In 2D MEMC display mode, keep frame rate under 60Hz
	if(scalerdisplay_get_2D_MEMC_support() && (drvif_scaelr3d_decide_is_3D_display_mode() == FALSE))
		result = TRUE;

	return result;
}

#if 0
void drvif_scalerConfig4K2KInput(bool iEnable)
{
    int fd = open("/dev/gdma0", O_RDONLY);

    if (!fd)
    {
        rtd_pr_vbe_info("open /dev/gdma0 failed\n");
        return;
    }

    ioctl(fd, GDMA_CONFIG_INPUTSRC_4K2K, &iEnable) ;
    close(fd);
}

void drvif_scalerConfig4K2KOsd(bool iEnable)
{
    int fd = open("/dev/gdma0", O_RDONLY);

    if (!fd)
    {
        rtd_pr_vbe_info("open /dev/gdma0 failed\n");
        return;
    }

    ioctl(fd, GDMA_CONFIG_OSD_ENABLE, &iEnable) ;
    close(fd);
}

void drvif_scalerConfig4K2K3DUI(bool iEnable)
{
    int fd = open("/dev/gdma0", O_RDONLY);

    if (!fd)
    {
        rtd_pr_vbe_info("open /dev/gdma0 failed\n");
        return;
    }

    ioctl(fd, GDMA_CONFIG_3D_UI, &iEnable) ;
    close(fd);
}
#endif

#ifdef CONFIG_4K2K_VIDEO_EMEMC
void drvif_scalerEMEMC(bool iEnable)
{
    int fd = open("/dev/gdma0", O_RDONLY);

    if (!fd)
    {
        rtd_pr_vbe_info("open /dev/gdma0 failed\n");
        return;
    }

    ioctl(fd, GDMA_CONFIG_EMEMC, &iEnable) ;
    close(fd);
}
#endif



// Set video output frame rate parameter
int drvif_scaler3d_voFrameRateControl_set_forceMaxFrameRate(DISP_MODE flag_3d, unsigned char flag_forceEnable, unsigned char boolUpdate)
{
	unsigned long ret=0;
	//CLNT_STRUCT clnt;
	int i;
	VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT * tVoutFRCtrlStructlocal;
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;

	// [NOTE] Enable VO force max frame rate display need enable force data frame sync control too (bandwidth issue)
	rtd_pr_vbe_debug("[SLR] Mode[%d], ForceMaxFrameRate=%d\n", flag_3d, flag_forceEnable);

	// VO frame rate control -- ONLY FOR VO SOURCE
	if(flag_forceEnable){
		unsigned int forceFrameRate = VO_FR_CTRL_FORCE_FRAMERATE_MAX;

		// 3D mode -- data FRC path@120
		// Don't rising VO frame rate in playback source when display timing is FRC@120Hz (saving bandwidth)
	  	tVoutFRCtrlStruct.enableForceFrameRate[flag_3d] 	= TRUE;
		tVoutFRCtrlStruct.ForceFrameRate[flag_3d] 		= forceFrameRate;
	}
	else{
	  	tVoutFRCtrlStruct.enableForceFrameRate[flag_3d]	= FALSE;
		tVoutFRCtrlStruct.ForceFrameRate[flag_3d] 			= DEFAULT_FORCE_FRAMERATE;
	}

	if(boolUpdate){
		tVoutFRCtrlStruct.opCode = _OPCODE_TABLE_CHANGE;
#if 0
		clnt = prepareCLNT(BLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);
		HRESULT* hr = VIDEO_RPC_VOUT_ToAgent_VodmaFrameRateControl_0(&tVoutFRCtrlStruct, &clnt);

		rtd_pr_vbe_info("[SLR] Mode[%d], ForceMaxFrameRate=%d, FR=%d\n", flag_3d, flag_forceEnable, tVoutFRCtrlStruct.ForceFrameRate[flag_3d]);

		if (-1 == (long)hr) {
			rtd_pr_vbe_info("Send RPC fail@%s\n", __FUNCTION__);
			ret = (long)hr;
		}
		// NONBLOCK_MODE don't need to free memory
		else {
			if (NULL != hr) {
				ret = *hr;
				free(hr);
			}
		}
#else

		vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
		if (vir_addr) {
			phy_addr = (unsigned int)dvr_to_phys((void *)vir_addr);
			tVoutFRCtrlStructlocal = (VIDEO_RPC_VOUT_FRAMERATE_CONTROL_STRUCT *)vir_addr_noncache;

			tVoutFRCtrlStructlocal->opCode = tVoutFRCtrlStruct.opCode;
			tVoutFRCtrlStructlocal->curDispMode = tVoutFRCtrlStruct.curDispMode;
			for(i=0; i < _DISP_MODE_NUM; i++){
				tVoutFRCtrlStructlocal->enableForceFrameRate[i] = tVoutFRCtrlStruct.enableForceFrameRate[i];
				tVoutFRCtrlStructlocal->ForceFrameRate[i] = htonl(tVoutFRCtrlStruct.ForceFrameRate[i]);
				tVoutFRCtrlStructlocal->enableDoubleDvs[i] = tVoutFRCtrlStruct.enableDoubleDvs[i];
				tVoutFRCtrlStructlocal->DoubleDvsThreshold[i] = htonl(tVoutFRCtrlStruct.DoubleDvsThreshold[i]);
				tVoutFRCtrlStructlocal->enableHighBound[i] = tVoutFRCtrlStruct.enableHighBound[i];
				tVoutFRCtrlStructlocal->HighBound[i] = htonl(tVoutFRCtrlStruct.HighBound[i]);
				tVoutFRCtrlStructlocal->HighAdjust[i] = htonl(tVoutFRCtrlStruct.HighAdjust[i]);
				tVoutFRCtrlStructlocal->enableLowBound[i] = tVoutFRCtrlStruct.enableLowBound[i];
				tVoutFRCtrlStructlocal->LowBound[i] = htonl(tVoutFRCtrlStruct.LowBound[i]);
				tVoutFRCtrlStructlocal->LowAdjust[i] = htonl(tVoutFRCtrlStruct.LowAdjust[i]);
			}

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
			if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_VodmaFrameRateControl, phy_addr, 0, &ret)) {
				rtd_pr_vbe_debug("RPC fail!!\n");
				dvr_free((void *)vir_addr);
				return -1;
			}
#endif
			dvr_free((void *)vir_addr);
		}
#endif
	}

	return ret;
}

unsigned char drvif_rpc_ConfigVodmaZoom(unsigned char ratio)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result = 0;
#endif
	VIDEO_RPC_VOUT_VODMA_ZOOM_RATE *tVodmaZoomStruct;

	//struct page *page = 0;
	//page = alloc_page(GFP_KERNEL);
	//tVodmaZoomStructtmp = (VIDEO_RPC_VOUT_VODMA_ZOOM_RATE *)page_address(page);
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_VODMA_ZOOM_RATE), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
	if (vir_addr) {
		phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);

		tVodmaZoomStruct = (VIDEO_RPC_VOUT_VODMA_ZOOM_RATE *)vir_addr_noncache;
		tVodmaZoomStruct->videoPlane =VO_VIDEO_PLANE_V1;// Scaler_Get_2D_VoPlane();
		tVodmaZoomStruct->zoomRate = ratio ;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)

		if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_ConfigVodmaZoom, phy_addr, 0, &result))
		{
			rtd_pr_vbe_debug("RPC fail!!\n");
			dvr_free((void *)vir_addr);
			return FALSE;
		}
#endif
		dvr_free((void *)vir_addr);
	}
	return TRUE;
}

unsigned char drvif_rpc_set3dmode( unsigned char vo_3d_mode)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result = 0;
#endif
	VIDEO_RPC_VOUT_SET_3D_MODE *config_data;

	//struct page *page = 0;
	//page = alloc_page(GFP_KERNEL);
	//config_datatmp = (VIDEO_RPC_VOUT_SET_3D_MODE *)page_address(page);
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_SET_3D_MODE), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
	//phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);

	config_data = (VIDEO_RPC_VOUT_SET_3D_MODE *)vir_addr_noncache;
	config_data->videoPlane = VO_VIDEO_PLANE_V1;//Scaler_Get_2D_VoPlane();
	config_data->mode_3d = vo_3d_mode;
	config_data->videoPlane = htonl(config_data->videoPlane);
	config_data->mode_3d= htonl(config_data->mode_3d);
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_Set_3D_Mode,phy_addr, 0, &result))
	{
		rtd_pr_vbe_debug("RPC fail!!\n");
		dvr_free((void *)vir_addr);
		return FALSE;
	}
#endif
	dvr_free((void *)vir_addr);
	return TRUE;
}


//extern void (*pOverscan_func)(stDISPLAY_OVERSCAN_DATA *pt_scan, SLR_RATIO_TYPE ratio_type);
int drvif_scaler3d_init_vo_3d_mode(unsigned char uc3dMode)
{
	VO_3D_MODE_TYPE vo_3d_mode;
	//CLNT_STRUCT clnt;
	//HRESULT* hr;


	//unsigned long ret=0;
	unsigned char ratio = 100;
	unsigned int calc_ratio = 100;
	unsigned short VHeight =0;


	//if(drvif_scaler3d_decide_vo_3d_output_frame_seq() == FALSE)
	//	return;
	if((uc3dMode == SLR_3DMODE_3D_SBS) || (uc3dMode == SLR_3DMODE_3D_TB)){
		//frank@0823 add below code to calculate the correct overscan
		stDISPLAY_OVERSCAN_DATA scan_data;
		SLR_RATIO_TYPE ratio_type = Scaler_DispGetRatioMode2();
		scan_data.HStart = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE);
		scan_data.HWidth = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
		scan_data.VStart = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);
		scan_data.VHeight = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
		if(pOverscan_func)
			pOverscan_func(&scan_data, ratio_type);
		VHeight = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
#if 1 // cklai@patch
		if (VHeight == 0)
			calc_ratio = 100; // patch for v length = 0
		else
			calc_ratio = VHeight*100/VHeight;
#endif

		if(calc_ratio >= 100)
			calc_ratio = 100;
		else if(calc_ratio <= 95)
			calc_ratio = 95;
		ratio = (unsigned char)calc_ratio;

#if 0
		memcpy((unsigned char*)Scaler_GetShareMemVirAddr(SCALERIOC_SENDZOOM), &ratio, sizeof(unsigned char));
		if (0 != (ret = Scaler_SendRPC(SCALERIOC_SENDZOOM)))
		{
			rtd_pr_vbe_info("ret=%d, set SCALERIOC_SENDZOOM to driver fail !!!\n", ret);
		}
#else



#if 0
		CLNT_STRUCT clnt = prepareCLNT(BLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);
		HRESULT* hr = VIDEO_RPC_VOUT_ToAgent_ConfigVodmaZoom_0(&tVodmaZoomStruct, &clnt);
		if (-1 == (long)hr) {
			rtd_pr_vbe_info("VIDEO_RPC_VOUT_ToAgent_ConfigVodmaZoom_0  fail\n");
			ret = (long)hr;
		}
		// NONBLOCK_MODE don't need to free memory
		else {
			if (NULL != hr) {
				ret = *hr;
				free(hr);
			}
		}
#else
		if(FALSE == drvif_rpc_ConfigVodmaZoom(ratio))
		{
			rtd_pr_vbe_debug("VIDEO_RPC_VOUT_ToAgent_ConfigVodmaZoom_0  fail\n");
			return FALSE;
		}
#endif
#endif
	}

	switch(uc3dMode){
		case SLR_3DMODE_2D:
			vo_3d_mode = VO_2D_MODE;
			break;

		// VO 3D auto detect mode active
		case SLR_3DMODE_3D_AUTO:
			vo_3d_mode = VO_3D_AUTO;
			break;

		case SLR_3DMODE_3D_SBS:
			vo_3d_mode = VO_3D_SIDE_BY_SIDE_HALF;
			break;

		case SLR_3DMODE_3D_TB:
			vo_3d_mode = VO_3D_TOP_AND_BOTTOM;
			break;

		case SLR_3DMODE_3D_FP: // MVC??
			rtd_pr_vbe_debug("Warning:%s(%d) means FP or MVC?\n", __FUNCTION__, __LINE__);
			vo_3d_mode = VO_3D_FRAME_PACKING;
			break;

		default:
			vo_3d_mode = VO_2D_MODE;
			break;
	}

	//clnt = prepareCLNT(BLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);


#if 0
	rtd_pr_vbe_info("[RPC]Send to VCPU Set 3D Mode(%d)\n", vo_3d_mode);

	// Send RPC
	hr = VIDEO_RPC_VOUT_ToAgent_Set_3D_Mode_0(&config_data, &clnt);

	if (*hr < 0){
		rtd_pr_vbe_info("ret = %d, Set_3D_Mode to VODMA driver fail !!!\n", (int)*hr);
		ret = -1;
	}

	if (hr)
		free(hr);
#else

	if(FALSE == drvif_rpc_set3dmode(vo_3d_mode))
	{
		rtd_pr_vbe_debug("VIDEO_RPC_VOUT_ToAgent_Set_3D_Mode  fail\n");
		return FALSE;
	}

#endif
	return TRUE;
}
#endif

void drivf_scaler_reset_freerun()
{
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	//ppoverlay_dh_total_last_line_length_RBUS dh_total_last_line_length_reg;
	pll27x_reg_dpll_in_fsync_tracking_ctrl_RBUS dpll_in_fsync_tracking_ctrl_reg;
	pll27x_reg_sys_dclkss_RBUS sys_dclkss_reg;
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_new_meas0_linecnt_real_RBUS ppoverlay_new_meas0_linecnt_real_reg;
	ppoverlay_i2d_ctrl_0_RBUS ppoverlay_i2d_ctrl_0_reg;
	ppoverlay_dv_total_RBUS dv_total_reg;
	UINT32 cnt = 0x3fffff;
	unsigned long flags;//for spin_lock_irqsave
	rtd_pr_vbe_notice("######### [%s]line:%d\n", __FUNCTION__, __LINE__);
	//IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
	dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
	dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 0;
	IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
	if((Get_DISPLAY_PANEL_TYPE() != P_VBY1_TO_HDMI) &&	(Get_DISPLAY_PANEL_TYPE() != P_VBY1_TO_LVDS_TO_HDMI)){
		sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
		sys_dclkss_reg.dclk_ss_en = 1;
		sys_dclkss_reg.dclk_ss_load= 1;
		IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
	}
	else{
		sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
		sys_dclkss_reg.dclk_ss_en = 0;
		sys_dclkss_reg.dclk_ss_load= 0;
		IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
	}
	ppoverlay_i2d_ctrl_0_reg.regValue = IoReg_Read32(PPOVERLAY_I2D_CTRL_0_reg);
	ppoverlay_i2d_ctrl_0_reg.i2d_special_dly_en = 0;
	IoReg_Write32(PPOVERLAY_I2D_CTRL_0_reg, ppoverlay_i2d_ctrl_0_reg.regValue);
	//IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT1);
	down(get_forcebg_semaphore());
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.dreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());
	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
	display_timing_ctrl1_reg.disp_fix_last_line_new = _DISABLE;
	display_timing_ctrl1_reg.disp_fsync_en = _DISABLE;

	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	ppoverlay_new_meas0_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
	rtd_pr_vbe_notice("### vtotal:%d, dtg lineCnt:%d, cnt:%x\n", dv_total_reg.dv_total, ppoverlay_new_meas0_linecnt_real_reg.line_cnt_rt, cnt);

	drvif_scaler_enable_abnormal_dvs_protect(0, 0);
#ifndef BUILD_QUICK_SHOW
	if((Get_TIMING_PROTECT_PANEL_TYPE()||Get_VO_Tracking_D_PANEL_TYPE())&&
		(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)){
			if(Scaler_Get_VODMA_Tracking_I3DDMA()==TRUE){
			    rtd_pr_vbe_notice("[%s] disble vodma tracking i3.\n", __FUNCTION__);
			    Scaler_Set_VODMA_Tracking_I3DDMA(FALSE);
			}
			if((scaler_get_VoTrackingI3DDMA_frequency_update_in_isr()==TRUE)){
				unsigned int count=100;
				extern void framesync_set_vtotal_by_vodma_freerun_period_isr(void);
				extern unsigned int Scaler_Get_VODMA_Tracking_I3DDMA_M_STAGE(void);
				rtd_pr_vbe_notice("[%s] recovery vtotal by vodma freerun period.\n", __FUNCTION__);
				while((Scaler_Get_VODMA_Tracking_I3DDMA_M_STAGE()!=0)&&--count)
					usleep_range(10000, 10000); // msleep(10);
				if(count==0){
					rtd_pr_vbe_notice("[%s] wait m_stage timeout (m_stage=%d)\n", __FUNCTION__,Scaler_Get_VODMA_Tracking_I3DDMA_M_STAGE());
				}
				//recovery i3 tracking vo, vtotal
				framesync_set_vtotal_by_vodma_freerun_period_isr();
				scaler_set_VoTrackingI3DDMA_frequency_update_in_isr(FALSE);
			}
	}
#endif        
	if(VODMA_VODMA_CLKGEN_get_en_fifo_full_gate(IoReg_Read32(VODMA_VODMA_CLKGEN_reg)) == 1){
            rtd_pr_vbe_notice("[%s]disable vo gating.\n",__FUNCTION__);
            #ifndef BUILD_QUICK_SHOW
            scaler_set_full_gatting_rpc(false);
            #endif
     }
}
#ifndef BUILD_QUICK_SHOW
void drivf_scaler_reset_freerun_60Hz(void)
{
//	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
//	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
//	pll27x_reg_dpll_in_fsync_tracking_ctrl_RBUS dpll_in_fsync_tracking_ctrl_reg;
//	pll27x_reg_sys_dclkss_RBUS sys_dclkss_reg;
	ppoverlay_dv_total_RBUS dv_total_reg;
	unsigned int vertical_total;
	ppoverlay_uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
	ppoverlay_memcdtg_dv_total_RBUS memcdtg_dv_total_reg;
	sys_reg_sys_dispclksel_RBUS sys_dispclksel_reg;
	//UINT32 dclk_fract_a, dclk_fract_b;
	UINT32 dclk_fract_a;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS dispd_stage1_sm_ctrl_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS	 dispd_smooth_toggle1_reg;
	unsigned int timeout=0x3fffff;
	ppoverlay_dtg_m_remove_input_vsync_RBUS dtg_m_remove_input_vsync_reg;
	ppoverlay_dtg_m_multiple_vsync_RBUS dtg_m_multiple_vsync_reg;
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	unsigned long flags;//for spin_lock_irqsave

#if 0
	fwif_color_safe_od_enable(0);
	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
	IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
	rtd_pr_vbe_debug("######### [%s]line:%d\n", __FUNCTION__, __LINE__);
	IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
	dpll_in_fsync_tracking_ctrl_reg.regValue = IoReg_Read32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg);
	dpll_in_fsync_tracking_ctrl_reg.fll_tracking_en = 0;
	IoReg_Write32(PLL27X_REG_DPLL_IN_FSYNC_TRACKING_CTRL_reg, dpll_in_fsync_tracking_ctrl_reg.regValue);
	sys_dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
	sys_dclkss_reg.dclk_ss_en = 1;
	sys_dclkss_reg.dclk_ss_load= 1;
	IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, sys_dclkss_reg.regValue);
	IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT2);
	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	display_timing_ctrl1_reg.disp_frc_on_fsync = _FALSE;
	display_timing_ctrl1_reg.disp_fix_last_line = _DISABLE;
	display_timing_ctrl1_reg.disp_fsync_en = _DISABLE;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);


	vertical_total = scalerdisplay_get_vtotal_by_vfreq(60);

	fw_scaler_dtg_double_buffer_enable(1);

	/*setup dtg master.*/
	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	dv_total_reg.dv_total= vertical_total-1;
	IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);

	/*setup uzu dtg.*/
	uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
	uzudtg_dv_total_reg.uzudtg_dv_total = vertical_total-1;
	IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);

	/*setup memc dtg.*/
	memcdtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
	memcdtg_dv_total_reg.memcdtg_dv_total = vertical_total-1;
	IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.memcdtg_dv_total);

	dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
	dtg_m_remove_input_vsync_reg.remove_half_ivs = 0; //mode2
	dtg_m_remove_input_vsync_reg.remove_half_ivs_mode2 = 0;
	IoReg_Write32(PPOVERLAY_DTG_M_Remove_input_vsync_reg, dtg_m_remove_input_vsync_reg.regValue);

	dtg_m_multiple_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_multiple_vsync_reg);
	if(display_timing_ctrl2_reg.dout_pixel_mode == PIXEL_MODE_2_PIXEL)
		dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync = 1;
	else
	dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync = 0;
	IoReg_Write32(PPOVERLAY_DTG_M_multiple_vsync_reg, dtg_m_multiple_vsync_reg.regValue);

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs = 1; // enable;
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
	if(display_timing_ctrl2_reg.dout_pixel_mode == PIXEL_MODE_2_PIXEL){
		uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = 1;
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = 1;
	}
	else{
		uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = 0;
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = 0;
	}
	IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);

	if(display_timing_ctrl2_reg.dout_pixel_mode == PIXEL_MODE_2_PIXEL)
		dclk_fract_a = 1;
	else
		dclk_fract_a = 0;
	sys_dispclksel_reg.regValue = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	sys_dispclksel_reg.dclk_fract_en = 1;
	sys_dispclksel_reg.dclk_fract_a = dclk_fract_a;
	sys_dispclksel_reg.dclk_fract_b = 0;
	//sys_dispclksel_reg.dclk_fract_b_eco = 0; //K3L error Will mark 0603
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, sys_dispclksel_reg.regValue);

	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);

	do{
		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	}while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply==1) && (--timeout));

	if(timeout==0){
		rtd_pr_vbe_notice("[%s][%d]WARNING! wait dispd_stage1_smooth_toggle_apply timeout!\n", __FUNCTION__, __LINE__);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 0;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
		dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
		dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
		IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	}else{
		dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
		dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
		IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
	}
	fw_scaler_dtg_double_buffer_enable(0);
	fwif_color_safe_od_enable(1);
#else // k3l timing change flow

	fwif_color_safe_od_enable(0);
	drivf_scaler_reset_freerun();
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.dreg_dbuf_en = 0;
	double_buffer_ctrl_reg.drm_multireg_dbuf_en = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_read_sel = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_vsync_sel = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

	dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
	dtg_m_remove_input_vsync_reg.remove_half_ivs = 0; //mode2
	dtg_m_remove_input_vsync_reg.remove_half_ivs_mode2 = 0;
	IoReg_Write32(PPOVERLAY_DTG_M_Remove_input_vsync_reg, dtg_m_remove_input_vsync_reg.regValue);

	dtg_m_multiple_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_multiple_vsync_reg);
	/*if(display_timing_ctrl2_reg.dout_pixel_mode == PIXEL_MODE_2_PIXEL)
		dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync = 1;
	else*/
	if(Get_DISPLAY_REFRESH_RATE() == 60)
		dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync = 0;
	else
		dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync = 1;
	IoReg_Write32(PPOVERLAY_DTG_M_multiple_vsync_reg, dtg_m_multiple_vsync_reg.regValue);

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs = 1; // enable;
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
	/*if(display_timing_ctrl2_reg.dout_pixel_mode == PIXEL_MODE_2_PIXEL){
		uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = 1;
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = 1;
	}
	else*/
	if(Get_DISPLAY_REFRESH_RATE() == 60)
	{
		uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = 0;
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = 0;
	}
	else
	{
		uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = 0;
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = 1;
	}
	IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);

	double_buffer_ctrl_reg.drm_multireg_dbuf_set = 1;
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	timeout = 0x3fffff;
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	while((double_buffer_ctrl_reg.drm_multireg_dbuf_set)&& --timeout){
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	}
	if(timeout==0){
		rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_set 2 wait timeout !!!\n");
	}

	/*if(display_timing_ctrl2_reg.dout_pixel_mode == PIXEL_MODE_2_PIXEL)
		dclk_fract_a = 1;
	else*/
		dclk_fract_a = 0;
	sys_dispclksel_reg.regValue = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	sys_dispclksel_reg.dclk_fract_en = 1;
	sys_dispclksel_reg.dclk_fract_a = dclk_fract_a;
	sys_dispclksel_reg.dclk_fract_b = 0;
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, sys_dispclksel_reg.regValue);

	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);

	do{
		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	}while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply==1) && (--timeout));

	if(timeout==0){
		rtd_pr_vbe_notice("[%s][%d]WARNING! wait dispd_stage1_smooth_toggle_apply timeout!\n", __FUNCTION__, __LINE__);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 0;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
		dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
		dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
		IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	}else{
		dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
		dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
		IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
	}
	vertical_total = scalerdisplay_get_vtotal_by_vfreq(60);
	/*setup dtg master.*/
	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	dv_total_reg.dv_total= vertical_total-1;
	IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);

	/*setup uzu dtg.*/
	uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
	uzudtg_dv_total_reg.uzudtg_dv_total = vertical_total-1;
	IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);

	/*setup memc dtg.*/
	memcdtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
	memcdtg_dv_total_reg.memcdtg_dv_total = vertical_total-1;
	IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.regValue);
	fwif_color_safe_od_enable(1);
	rtd_pr_vbe_notice("[%s][%d]\n", __FUNCTION__, __LINE__);

#endif
}


void drivf_scaler_reset_fraction_ratio(void)
{
	ppoverlay_dv_total_RBUS dv_total_reg;
	unsigned int vertical_total;
	ppoverlay_uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
	ppoverlay_memcdtg_dv_total_RBUS memcdtg_dv_total_reg;
	sys_reg_sys_dispclksel_RBUS sys_dispclksel_reg;
	//UINT32 dclk_fract_a;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS dispd_stage1_sm_ctrl_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS	 dispd_smooth_toggle1_reg;
	unsigned int timeout=0x3fffff;
	ppoverlay_dtg_m_remove_input_vsync_RBUS dtg_m_remove_input_vsync_reg;
	ppoverlay_dtg_m_multiple_vsync_RBUS dtg_m_multiple_vsync_reg;
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	unsigned long flags;//for spin_lock_irqsave

	// due to vo force mode change need align to 12Hz of 24in and 60out
	//it may garbage due to not align; in order to avoid this case
	// I will reset 24in as 48hz out, then it can do mode change safely.
	if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) &&
		(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245)){	//24Hz

		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		double_buffer_ctrl_reg.dreg_dbuf_en = 0;
		double_buffer_ctrl_reg.drm_multireg_dbuf_en = 1;
		double_buffer_ctrl_reg.drm_multireg_dbuf_read_sel = 1;
		double_buffer_ctrl_reg.drm_multireg_dbuf_vsync_sel = 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
		ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

		dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
		dtg_m_remove_input_vsync_reg.remove_half_ivs = 0; //mode2
		dtg_m_remove_input_vsync_reg.remove_half_ivs_mode2 = 0;
		IoReg_Write32(PPOVERLAY_DTG_M_Remove_input_vsync_reg, dtg_m_remove_input_vsync_reg.regValue);

		dtg_m_multiple_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_multiple_vsync_reg);
		dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync = 1;
		IoReg_Write32(PPOVERLAY_DTG_M_multiple_vsync_reg, dtg_m_multiple_vsync_reg.regValue);

		uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs = 1; // enable;
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
		uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = 0;
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = 1;
		IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);

		double_buffer_ctrl_reg.drm_multireg_dbuf_set = 1;
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		timeout = 0x3fffff;
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		while((double_buffer_ctrl_reg.drm_multireg_dbuf_set)&& --timeout){
			double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		}
		if(timeout==0){
			rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_set 2 wait timeout !!!\n");
		}

		sys_dispclksel_reg.regValue = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
		sys_dispclksel_reg.dclk_fract_en = 1;
		sys_dispclksel_reg.dclk_fract_a = 0;
		sys_dispclksel_reg.dclk_fract_b = 1;
		IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, sys_dispclksel_reg.regValue);

		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);

		timeout = 0x3fffff;
		do{
			dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
		}while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply==1) && (--timeout));

		if(timeout==0){
			rtd_pr_vbe_notice("[%s][%d]WARNING! wait dispd_stage1_smooth_toggle_apply timeout!\n", __FUNCTION__, __LINE__);
			dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 0;
			IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
			dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
			dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
			IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
			dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
			IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
		}else{
			dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
			dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
			IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
		}
		vertical_total = scalerdisplay_get_vtotal_by_vfreq(48);
		/*setup dtg master.*/
		dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
		dv_total_reg.dv_total= vertical_total-1;
		IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);

		/*setup uzu dtg.*/
		uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
		uzudtg_dv_total_reg.uzudtg_dv_total = vertical_total-1;
		IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);

		/*setup memc dtg.*/
		memcdtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
		memcdtg_dv_total_reg.memcdtg_dv_total = vertical_total-1;
		IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.regValue);
		rtd_pr_vbe_notice("[%s][%d]\n", __FUNCTION__, __LINE__);

	}

}

//#ifdef UZU_60_24_MEMC_BYPASS_ENABLE
int drivf_scaler_dtg_memc_bypass_switch = 0;
int drivf_scaler_memc_24_to_60_flag = 0;

int vbe_dtg_memec_bypass_switch(void)
{
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
		return 0;
#else

	if((rtd_inl(SOFTWARE1_SOFTWARE1_00_reg)>>10)&0x1){//FRC_TOP__software1__reg_software_00_ADDR
		drivf_scaler_dtg_memc_bypass_switch = 0;
		return 0;
	}
	return drivf_scaler_dtg_memc_bypass_switch;
#endif
}

void vbe_dtg_memec_bypass_switch_set(int mode)
{
	drivf_scaler_dtg_memc_bypass_switch = mode;
}

int vbe_get_dtg_memec_24_to_60_flag(void)
{
	return drivf_scaler_memc_24_to_60_flag;
}

void vbe_set_dtg_memec_24_to_60_flag(int mode)
{
	drivf_scaler_memc_24_to_60_flag = mode;
}

/*****	     drivf_scaler_set_memc_bypass	    *****/
/*======================================================*/
/* UZU 24 -> 60, MEMC enable -> bypass			*/
/* Control flow:					*/
/* 1. set double buffer					*/
/* 2. set MEMC UZU remove/multi				*/
/* 3. set UZU clock					*/
/* 4. Scaler_MEMC_SetInOutFrameRate			*/
/* 5. delay 1 frame					*/
/* 6. set MEMC disable					*/
/* 7. vbe_set_dtg_memec_24_to_60_flag			*/
/*======================================================*/
/* UZU 60 -> 24, MEMC bypass -> enable			*/
/* Control flow:					*/
/* 1. set freeze					*/
/* 2. enable vbe_dtg_memec_bypass_switch_set to MEMC	*/
/* 3. delay 4 frame					*/
/* 4. Scaler_MEMC_SetInOutFrameRate			*/
/* 5. set double buffer					*/
/* 6. set MEMC UZU remove/multi				*/
/* 7. set UZU clock					*/
/* 9. set MEMC enable					*/
/* 10. release frame					*/
/********************************************************/

void drivf_scaler_set_memc_bypass(void)
{
	extern void drvif_scalerdisplay_set_freeze(unsigned char channel, unsigned char mode);
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl2_reg;
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS dispd_stage1_sm_ctrl_reg;
	//ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
	unsigned long flags;//for spin_lock_irqsave
	unsigned char multiple_ratio=0, remove_ratio=0;
	UINT32 dclk_fract_a, dclk_fract_b;
	sys_reg_sys_dispclksel_RBUS sys_dispclksel_reg;
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS dispd_smooth_toggle1_reg;
	int input_v_freq;
	unsigned int count=0x3fffff;
	UINT32 mul_dbuf_cnt=0;
	int delay_frame;
	int memc_bypass;
	ppoverlay_new_meas1_linecnt_real_RBUS new_meas1_linecnt_real_reg;

	if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 235) || (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 245)) {
		rtd_pr_vbe_warn("UZU 60/24, MEMC bypass/enable support 24fps input only\n");
		return;
	}

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	if (uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode) // UZU = 24 memc enable mode
		memc_bypass = TRUE;	// set memc bypass
	else
		memc_bypass = FALSE;
	// for UZU 60 -> 24, MEMC bypass -> enable
	if (TRUE != memc_bypass) {
		drvif_scalerdisplay_set_freeze(0, TRUE);
		vbe_dtg_memec_bypass_switch_set(1);
		//Scaler_MEMC_DTV_ME_CLK_OnOff(0, 0);
		Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 0);

		for (delay_frame = 5; delay_frame > 0; delay_frame--) {
			double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
			double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;  // D4
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
			double_buffer_ctrl2_reg.uzudtg_dbuf_vsync_sel = 2;
			double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
			count = 0x3fffff;
			while((double_buffer_ctrl2_reg.uzudtgreg_dbuf_set)&& --count){
				double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
			}
		}
		Scaler_MEMC_SetInOutFrameRate();
	}

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.dreg_dbuf_en = 0;	// D7 DTG master
	double_buffer_ctrl_reg.drm_multireg_dbuf_en = 1;	// D8
	double_buffer_ctrl_reg.drm_multireg_dbuf_read_sel = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_vsync_sel = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());

	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;	// D4
	double_buffer_ctrl2_reg.uzudtg_dbuf_vsync_sel = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

//======= Set MEMC UZU remove/multi
	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs = 1; // D8
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 0;
	multiple_ratio = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO);
	remove_ratio = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO);

	if (TRUE == memc_bypass) {
		uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = 0;
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = 0;
	} else {
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = (multiple_ratio-1);
		uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = (remove_ratio-1);
	}
	IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);

	count = 0x3fffff;
	while((count == 0x3fffff)&&(mul_dbuf_cnt<10000)){
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		double_buffer_ctrl_reg.drm_multireg_dbuf_set = 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		count = 0x3fffff;
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		while((double_buffer_ctrl_reg.drm_multireg_dbuf_set)&& --count){
			double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		}
		if(count==0){
			rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_set 2 wait timeout !!!\n");
		}
		mul_dbuf_cnt++;
	}

//====== Set UZU clock
	sys_dispclksel_reg.regValue = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	dclk_fract_a = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO)-1;
	dclk_fract_b = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO)-1;

	sys_dispclksel_reg.dclk_fract_en = 1;
	if  (TRUE == memc_bypass) {
		sys_dispclksel_reg.dclk_fract_a = 0;
		sys_dispclksel_reg.dclk_fract_b = 0;
	} else {
		sys_dispclksel_reg.dclk_fract_a = dclk_fract_a;
		sys_dispclksel_reg.dclk_fract_b = (dclk_fract_b&0x1f);
	}
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, sys_dispclksel_reg.regValue);

	count = 0x3fffff;
	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=0;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
	IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply)&& --count){
		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);

		new_meas1_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);
		if (new_meas1_linecnt_real_reg.memcdtg_line_cnt_rt > 0x8bf) {
			dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 0;
			IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
			dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
			dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
			IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
			dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
			IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
			break;
		}
	}
	if(count==0){
		rtd_pr_vbe_emerg("dispd_smooth_toggle1 timeout !!!\n");
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 0;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
		dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
		dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
		IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	}
	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

	if (TRUE == memc_bypass) {
		input_v_freq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
		Scaler_DispSetInputInfo(SLR_INPUT_V_FREQ, 600);
		Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);
		Scaler_DispSetInputInfo(SLR_INPUT_V_FREQ, input_v_freq);

		double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;	// D4
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

		memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
		memc_mux_ctrl_reg.memc_outmux_sel = 0;	//D4 MEMC bypass

		IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);
		count = 0x3fffff;
		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=0;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
		dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
		dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
		IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);

		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
		while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply)&& --count){
			dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
		}
		if(count==0){
			rtd_pr_vbe_emerg("dispd_smooth_toggle1 timeout !!!\n");
			dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 0;
			IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
			dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
			dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
			IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
			dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
			IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
		}
		vbe_set_dtg_memec_24_to_60_flag(1);
		//Scaler_MEMC_DTV_ME_CLK_OnOff(1, 0);
		Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 0);
	} else {
		double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

		memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
		memc_mux_ctrl_reg.memc_outmux_sel = 1;
		IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);
		drvif_scalerdisplay_set_freeze(0, FALSE);
	}

	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
}
//#endif

void Scaler_set_film_mode_change_type(unsigned int enable)
{
	film_mode_param = (FILM_SHAREMEM_PARAM*)Scaler_GetShareMemVirAddr(FILM_MODE_CHANGE_TYPE);
	film_mode_param->film_mode_type_change = htonl(enable);
}

unsigned int Scaler_get_film_mode_change_type(void)
{
	film_mode_param = (FILM_SHAREMEM_PARAM*)Scaler_GetShareMemVirAddr(FILM_MODE_CHANGE_TYPE);
	return (film_mode_param? htonl(film_mode_param->film_mode_type_change): 0);
}

unsigned int Scaler_get_memc_me_clock_off(void)
{
	film_mode_param = (FILM_SHAREMEM_PARAM*)Scaler_GetShareMemVirAddr(FILM_MODE_CHANGE_TYPE);
	return (film_mode_param? htonl(film_mode_param->memc_me_clock_off): 0);
}

void Scaler_set_dynamic_framerate_change_flag(unsigned int enable)
{
//	rtd_pr_vbe_notice("[Eric] Scaler_set_dynamic_framerate_change_flag = %d\n",enable);
	dynamic_framerate_change = (unsigned int*)Scaler_GetShareMemVirAddr(DYNAMIC_FRAMERATE_CHANGE_INFO);
	*dynamic_framerate_change = htonl(enable);
}

unsigned int Scaler_get_dynamic_framerate_change_flag(void)
{
	dynamic_framerate_change = (unsigned int*)Scaler_GetShareMemVirAddr(DYNAMIC_FRAMERATE_CHANGE_INFO);
	return (dynamic_framerate_change? htonl(*dynamic_framerate_change): 0);
}

// FixMe, 20190925
//extern VOID Scaler_MEMC_Set_BypassToEnable_Flag(unsigned char u1_enable);
//extern VOID Scaler_MEMC_Set_EnableToBypass_Flag(unsigned char u1_enable);
extern unsigned char get_vt_EnableFRCMode(void);
void drivf_scaler_change_film_mode_type(int enable, int type, int v_freq)
{
	extern void drvif_scalerdisplay_set_freeze(unsigned char channel, unsigned char mode);
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl2_reg;
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS dispd_stage1_sm_ctrl_reg;
	ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
	ppoverlay_dtg_m_remove_input_vsync_RBUS dtg_m_remove_input_vsync_reg;
	ppoverlay_dtg_m_multiple_vsync_RBUS dtg_m_multiple_vsync_reg;
	unsigned int vertical_total;
	ppoverlay_dv_total_RBUS dv_total_reg;
	ppoverlay_uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
	ppoverlay_memcdtg_dv_total_RBUS memcdtg_dv_total_reg;
	mdomain_disp_ddr_mainctrl_RBUS ddr_mainctrl;
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;

	unsigned long flags;//for spin_lock_irqsave
	unsigned char multiple_ratio=0, remove_ratio=0;
	UINT32 dclk_fract_a, dclk_fract_b;
	sys_reg_sys_dispclksel_RBUS sys_dispclksel_reg;
//	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS dispd_smooth_toggle1_reg;
	int input_freq = v_freq, output_freq = v_freq;
	unsigned int count=0x3fffff;
//	int delay_frame;
	int me_bypass;
//	ppoverlay_new_meas1_linecnt_real_RBUS new_meas1_linecnt_real_reg;
	//static int film_mode_type = VO_FILM_TYPE_NONE;
	unsigned char uzu_valid_framecnt = 0;
	//UINT32 meas0_lineCnt1=0;
	//UINT32 meas0_lineCnt2=0;
	//UINT32 meas0_lineCnt3=0;
	//UINT32 meas0_lineCnt4=0;
	//UINT32 meas0_lineCnt5=0;
	//UINT32 meas1_lineCnt1=0;
	//UINT32 meas1_lineCnt2=0;
	//UINT32 meas1_lineCnt3=0;
	//UINT32 meas1_lineCnt4=0;
	//UINT32 meas1_lineCnt5=0;
	UINT32 timestamp[5] = {0,0,0,0,0};

	char *film_type_str[] = {
		"CAD_NONE",
		"CAD_VIDEO",
		"CAD_22",
		"CAD_32",
		"CAD_32322",
		"CAD_334",
		"CAD_22224",
		"CAD_2224",
		"CAD_3223",
		"CAD_55",
		"CAD_66",
		"CAD_44",
		"CAD_1112",
		"CAD_11112",
		"CAD_122",
		"CAD_11123",
		"CAD_BAD_EDIT",
		"CAD_VIDEO"
	};
//	UINT32 test_val = 0;
//	test_val = (rtd_inl(0xb809d7f4)>>16)&0xff;

	rtd_pr_vbe_info("FILM_MODE, %s, set enable = %d, mode = %d\n", __func__, enable, type);

//	if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 235) || (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 245)) {
//		rtd_pr_vbe_warn("UZU 60/24, MEMC bypass/enable support 24fps input only\n");
//		return;
//	}
	if (enable == 0) {
		//film_mode_type = VO_FILM_TYPE_NONE;
		return;
	}
	rtd_pr_vbe_notice("[FILM] film switch from %d(%s) to %d(%s) ...\n", pre_film_type, film_type_str[pre_film_type], type, film_type_str[type]);
	Scaler_set_film_mode_change_type(1);

	down(get_scaler_fll_running_semaphore());

	//reset timing to freerun
	drivf_scaler_reset_freerun();
	//if fll is running, need to disable fll task
	if(modestate_get_fll_running_flag()){
		modestate_set_fll_running_flag(_DISABLE);
	}

	switch (type) {
	case _VO_CAD_22:
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
		me_bypass = FALSE;
		if (v_freq == 250) {
			input_freq = 250;	// for 50 to 25 film mode
			output_freq = 500;
		} else {
			input_freq = 300;	// for 60 to 30 film mode
			output_freq = 600;
		}
	break;

	case _VO_CAD_32:
	case _VO_CAD_2224:
	case _VO_CAD_3223:
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 2);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
		me_bypass = FALSE;
		input_freq = 240;	// for 60 to 24 film mode
		output_freq = 600;
	break;

	case _VO_CAD_32322:		// 60 to 25
	case _VO_CAD_22224:
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
		me_bypass = FALSE;
		input_freq = 250;
		output_freq = 500;
	break;

	case _VO_CAD_122:			// 50 to 30
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
		me_bypass = FALSE;
		input_freq = 300;
		output_freq = 600;
	break;

	case _VO_CAD_11112:
		if (v_freq == 300) {	// 30 to 25
			me_bypass = FALSE;
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			input_freq = 250;
			output_freq = 500;
		} else {		// 60 to 50
			me_bypass = TRUE;
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
			input_freq = 500;
			output_freq = 500;
		}
	break;

	case _VO_CAD_1112:
		me_bypass = FALSE;
		if (v_freq == 300) {	// 30 to 24
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 2);
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
			input_freq = 240;
			output_freq = 600;
		} else {		// 60 to 48
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			input_freq = 480;
			output_freq = 480;
		}
	break;

	case _VO_CAD_11123:		// 50 to 24
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 2);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
		me_bypass = FALSE;
		input_freq = 240;
		output_freq = 600;
	break;

	//Eric@20180814 bad edit type when memc mux disable case
	case _VO_CAD_BAD_EDIT:
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
		me_bypass = FALSE;
		input_freq = v_freq;
		output_freq = v_freq;
	break;

	default:			// VO_FILM_TYPE_NONE
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
		me_bypass = TRUE;     // set memc bypass
		input_freq = v_freq;
		output_freq = v_freq;
	break;
	}
	Scaler_DispSetInputInfo(SLR_INPUT_V_FREQ, input_freq);
	Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, output_freq / 10);

	//120Hz panel do not set MC only mode
	if(Get_DISPLAY_REFRESH_RATE() >= 120){
		me_bypass = FALSE;
	}

	ddr_mainctrl.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);

	rtd_pr_vbe_info("FILM_MODE %s, REMOVE_RATIO = %d, MULTIPLE_RATIO = %d, input_freq = %d, output_freq = %d\n", __func__,
		Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO), Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO),
		input_freq, output_freq);
	//meas0_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[0] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);

#if 0	//Eric@20180917 move to DTG change done
	if (TRUE != me_bypass) {
		memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
//		if(memc_mux_ctrl_reg.memc_outmux_sel == 0){
//			vbe_dtg_memec_bypass_switch_set(0);
			Scaler_MEMC_Set_BypassToEnable_Flag(1);//add by jerry wang;only for memc use
			rtd_pr_vbe_notice("[jerry_Scaler_MEMC_Bypass_CLK_OnOff_EEE][%d]\n", ((rtd_inl(0xb8028100)>>13)&0x1));
			rtd_pr_vbe_notice("[jerry_MEMC_bypass_switch_test_AAA][%d][%d]\n", rtd_inl(0xB801B6B8), ((rtd_inl(0xb8028100)>>13)&0x1));
			//Scaler_MEMC_DTV_ME_CLK_OnOff(1, 0);
			Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 0);
//		}
		rtd_pr_vbe_notice("[jerry_MEMC_bypass_switch_test_BBB][%d][%d]\n", rtd_inl(0xB801B6B8), ((rtd_inl(0xb8028100)>>13)&0x1));
		Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);
	}
#endif

	//memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	if ((TRUE == me_bypass) && (get_vt_EnableFRCMode() == FALSE)){
		//VR360 case, set memc mux off
		rtd_pr_vbe_notice("[VR360] VR360 case, set memc mux off\n");
		Scaler_MEMC_outMux(_ENABLE,_DISABLE);	//db on, mux off
	}else if((get_new_game_mode_vdec_memc_bypass_condition()==TRUE) &&
		(drv_memory_get_low_delay_game_mode_dynamic()==TRUE)){
		//if memc bypass, change non 1:1 case, timing will error without memc, so enable memc mux first
		Scaler_MEMC_outMux(_ENABLE,_ENABLE);	//db on, mux on
	}

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.dreg_dbuf_en = 0;	// D7 DTG master
	double_buffer_ctrl_reg.drm_multireg_dbuf_en = 1;	// D8
	double_buffer_ctrl_reg.drm_multireg_dbuf_read_sel = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_vsync_sel = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzu_valid_framecnt = uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync;	//if 2:5, valid_framecnt = 1, if 1:2, valid_framecnt = 0

	//wait to front porch
	count = 0x3fffff;
	dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
	while(!((dtg_frame_cnt1_reg.uzudtg_output_frame_cnt == uzu_valid_framecnt) && (IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg) > 100) && (IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg) < 2000)) && --count){
		dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
	}
	if(count == 0)
		rtd_pr_vbe_emerg("[DTG] Wait to uzu_valid_framecnt timeout!!, uzu_valid_framecnt = %d, uzudtg_output_frame_cnt = %d\n",uzu_valid_framecnt, dtg_frame_cnt1_reg.uzudtg_output_frame_cnt);

	//meas0_lineCnt2 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt2 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[1] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);

//======= set Master
	dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
	dtg_m_remove_input_vsync_reg.remove_half_ivs = 0; //mode2
	dtg_m_remove_input_vsync_reg.remove_half_ivs_mode2 = (Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO)-1);
	IoReg_Write32(PPOVERLAY_DTG_M_Remove_input_vsync_reg, dtg_m_remove_input_vsync_reg.regValue);

	dtg_m_multiple_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_multiple_vsync_reg);
	dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync = (Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO)-1);
	IoReg_Write32(PPOVERLAY_DTG_M_multiple_vsync_reg, dtg_m_multiple_vsync_reg.regValue);

//======= Set MEMC UZU remove/multi
	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs = 1; // D8
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 0;
	multiple_ratio = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO);
	remove_ratio = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO);

	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = (multiple_ratio-1);
	uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = (remove_ratio-1);
	IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);

	//meas0_lineCnt3 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt3 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[2] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);

//====== Set UZU clock
	sys_dispclksel_reg.regValue = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	dclk_fract_a = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO)-1;
	dclk_fract_b = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO)-1;

	sys_dispclksel_reg.dclk_fract_en = 1;
	sys_dispclksel_reg.dclk_fract_a = dclk_fract_a;
	sys_dispclksel_reg.dclk_fract_b = (dclk_fract_b&0x1f);
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, sys_dispclksel_reg.regValue);

	//meas0_lineCnt4 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt4 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[3] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);

	count = 0x3fffff;
	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=0;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
	IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply)&& --count){
		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	}
	if(count==0){
		rtd_pr_vbe_emerg("dispd_smooth_toggle1 timeout !!!\n");
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 0;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
		dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
		dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
		IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	}

	//meas0_lineCnt5 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt5 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[4] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);

	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

//====== Set V_total
	vertical_total = scalerdisplay_get_vtotal_by_vfreq(output_freq / 10);
	/*setup dtg master.*/
	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	dv_total_reg.dv_total= vertical_total-1;
	IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);

	/*setup uzu dtg.*/
	uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
	uzudtg_dv_total_reg.uzudtg_dv_total = vertical_total-1;
	IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);

	/*setup memc dtg.*/
	memcdtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
	memcdtg_dv_total_reg.memcdtg_dv_total = vertical_total-1;
	IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.regValue);

	//Eric@20180821 if in m-domain input fast mode, need disable input fast after dtg done.
	if(get_frc_style_input_fast_than_display()){
		set_frc_style_input_fast_than_display(FALSE);
		WaitFor_DEN_STOP_Done();
		rtd_pr_vbe_notice("[FILM] Disable frc input fast!!!\n");
	}

#if 0	//Debug use
	rtd_pr_vbe_notice("[Eric] uzu_valid_framecnt = %d, uzudtg_output_frame_cnt = %d\n",uzu_valid_framecnt,dtg_frame_cnt1_reg.uzudtg_output_frame_cnt);
	rtd_pr_vbe_notice("meas0_lineCnt1 (%d)\n", meas0_lineCnt1);
	rtd_pr_vbe_notice("meas1_lineCnt1 (%d)\n", meas1_lineCnt1);
	rtd_pr_vbe_notice("meas0_lineCnt2 (%d)\n", meas0_lineCnt2);
	rtd_pr_vbe_notice("meas1_lineCnt2 (%d)\n", meas1_lineCnt2);
	rtd_pr_vbe_notice("meas0_lineCnt3 (%d)\n", meas0_lineCnt3);
	rtd_pr_vbe_notice("meas1_lineCnt3 (%d)\n", meas1_lineCnt3);
	rtd_pr_vbe_notice("meas0_lineCnt4 (%d)\n", meas0_lineCnt4);
	rtd_pr_vbe_notice("meas1_lineCnt4 (%d)\n", meas1_lineCnt4);
	rtd_pr_vbe_notice("meas0_lineCnt5 (%d)\n", meas0_lineCnt5);
	rtd_pr_vbe_notice("meas1_lineCnt5 (%d)\n", meas1_lineCnt5);
	rtd_pr_vbe_notice("time = (%x,%x,%x,%x,%x)\n", timestamp[0],timestamp[1],timestamp[2],timestamp[3],timestamp[4]);
#endif

//====== Set MEMC Mux mode
	if (TRUE == me_bypass) {
		Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);

#if 0	//Eric@20180917 do not change memc mux settings
		double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;	// D4
		double_buffer_ctrl2_reg.uzudtg_dbuf_vsync_sel= 3;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

		memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
		memc_mux_ctrl_reg.memc_outmux_sel = 1;	//D4 MEMC bypass

		IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);
		count = 0x3fffff;
		double_buffer_ctrl2_reg.uzudtgreg_dbuf_set= 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

		while((double_buffer_ctrl2_reg.uzudtgreg_dbuf_set) && --count){
			double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		}
		if(count==0){
			rtd_pr_vbe_emerg("PPOVERLAY_Double_Buffer_CTRL2_reg timeout Line=%d!!!\n", __LINE__);
			double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;	// D4
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
		}
#endif

#if 0	//Eric@20180917 do not change memc mux settings
		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=0;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
		dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
		dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
		IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);

		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
		while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply)&& --count){
			dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
		}
		if(count==0){
			rtd_pr_vbe_emerg("dispd_smooth_toggle1 timeout !!!\n");
			dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 0;
			IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
			dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
			dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
			IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
			dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
			IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
		}
#endif
		rtd_pr_vbe_notice("[jerry_Scaler_MEMC_Bypass_CLK_OnOff_FFF]\n");
		// FixMe, 20190925
		//Scaler_MEMC_Set_EnableToBypass_Flag(1);
		//Scaler_MEMC_DTV_ME_CLK_OnOff(0, 0);
		if(get_vt_EnableFRCMode() == TRUE){
			rtd_pr_vbe_notice("[VR360] not VR360 case, go MC on flow\n");
			Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 0);	//not VR360, go MC on setting
		}
		else{
			rtd_pr_vbe_notice("[VR360] VR360 case, bypass memc\n");
			Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 0);	//VR360, go MEMC bypass setting
		}

	} else {
		// FixMe, 20190925
		//Scaler_MEMC_Set_BypassToEnable_Flag(1);//add by jerry wang;only for memc use
		rtd_pr_vbe_notice("[jerry_MEMC_bypass_switch_test_AAA][%d][%d]\n", rtd_inl(TIMER_SCPU_CLK90K_LO_reg), ((rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg)>>13)&0x1));
		//Scaler_MEMC_DTV_ME_CLK_OnOff(1, 0);
		Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 0);
		rtd_pr_vbe_notice("[jerry_MEMC_bypass_switch_test_BBB][%d][%d]\n", rtd_inl(TIMER_SCPU_CLK90K_LO_reg), ((rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg)>>13)&0x1));
		memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
		if(memc_mux_ctrl_reg.memc_outmux_sel == 0){	//if memc mux off, need turn on
			rtd_pr_vbe_notice("[MEMC]memc mux off, need turn on!!\n");
			Scaler_MEMC_outMux(_ENABLE,_ENABLE);	//db on, mux on
		}
		Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);

#if 0	//Eric@20180917 do not change memc mux settings
		double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

		memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
		memc_mux_ctrl_reg.memc_outmux_sel = 1;
		IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);
//		drvif_scalerdisplay_set_freeze(0, FALSE);
		rtd_pr_vbe_notice("[jerry_Scaler_MEMC_Bypass_CLK_OnOff_KKK]\n");
#endif

	}

	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
	Scaler_set_film_mode_change_type(0);
    	rtd_pr_vbe_notice("[FILM] film switch from %d(%s) to %d(%s) Done\n", pre_film_type, film_type_str[pre_film_type], type, film_type_str[type]);
	pre_film_type = type;
	vbe_disp_pwm_frequency_update(FALSE);

	//MEMC bypass Flow
	// Check is DTV condition
	if(get_new_game_mode_vdec_memc_bypass_condition() == TRUE){
			// Check Picture mode is game mode
			if(drv_memory_get_low_delay_game_mode_dynamic()==TRUE){
				rtd_pr_new_game_mode_notice("[GameMode] FILM condition: MEMC_BYPASS 1\n");
				vbe_disp_game_mode_memc_bypass(1);
				fw_set_vsc_GameMode(1);
			}else{
				rtd_pr_new_game_mode_notice("[GameMode] FILM condition: MEMC_BYPASS 0\n");
				vbe_disp_game_mode_memc_bypass(0);
				fw_set_vsc_GameMode(0);
			}
	}
	//after framerate change, trigger fll again.
	modestate_set_fll_running_flag(_ENABLE);

	up(get_scaler_fll_running_semaphore());
}

#endif  



//#define ENABLE_SCALER_RPC //FIX ME@frank@0303 mark to fix complier error
static bool scalerRPC_isInit = false;
static RPC_SHARE_MEM_INFO g_atShareMemInfo[SCALERIOC_LATESTITEM];

static unsigned long g_ulNonCachedAddr;
static unsigned long  g_ulCachedAddr = 0;
static unsigned long g_ulPhyAddr = 0;

static unsigned int g_ulShareMemOffset = 0;
#ifdef BUILD_QUICK_SHOW
static unsigned long g_quick_showAddr; //for quick_show flow use
#endif

void SetRPCShareMem(unsigned int a_ulCmd, unsigned int a_ulSize)
{
	// Change to 4 alignment
	//rtd_pr_vbe_info("D. Scaler_SetAutoMA1Flag\n");
#ifdef CONFIG_ARM64
	if (0 != (g_ulShareMemOffset % 8))
		g_ulShareMemOffset = ((g_ulShareMemOffset / 8) + 1) * 8;
#else
	if (0 != (g_ulShareMemOffset % 4))
		g_ulShareMemOffset = ((g_ulShareMemOffset / 4) + 1) * 4;
#endif

	g_atShareMemInfo[a_ulCmd].ulOffset = g_ulShareMemOffset;
	g_atShareMemInfo[a_ulCmd].ulSize = a_ulSize;

	g_ulShareMemOffset += a_ulSize;

}

static unsigned char SetRPCTheSameShareMem(unsigned int a_ulNewCmd, unsigned int a_ulOrgCmd)
{
	unsigned char ucErrCode = 0;
	unsigned int ulOffset = g_atShareMemInfo[a_ulOrgCmd].ulOffset;
	unsigned int ulSize = g_atShareMemInfo[a_ulOrgCmd].ulSize;

	//rtd_pr_vbe_info("C. Scaler_SetAutoMA1Flag\n");
	if (ulOffset != 0 && ulSize != 0)
	{
		g_atShareMemInfo[a_ulNewCmd].ulOffset = g_atShareMemInfo[a_ulOrgCmd].ulOffset;
		g_atShareMemInfo[a_ulNewCmd].ulSize = g_atShareMemInfo[a_ulOrgCmd].ulSize;
	}
	else
	{
		ucErrCode = 1;
		rtd_pr_vbe_debug("oliver+, SetRPCTheSameShareMem\n");
	}

	return ucErrCode;

}
extern struct semaphore *get_scalerrpc_semaphore(void);
long Scaler_SendRPC(unsigned int a_ulCmd, unsigned long a_ulValue, unsigned char a_ucValueOnly)
{
#ifndef BUILD_QUICK_SHOW
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long ret_value = 0;
	int ret_rpc = 0;
#endif
	//if (Scaler_IsUnitTest())
		//return ret;

	if (!scalerRPC_isInit) {
		rtd_pr_vbe_debug("%s %s:%d, scalerRPC_isInit = false\n", __FUNCTION__, __FILE__, __LINE__);
		//assert(0);
		return -1;
	}
#ifndef BUILD_QUICK_SHOW
	down(get_scalerrpc_semaphore());
#endif
	  //clnt = prepareCLNT(NONBLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);
	  //clnt = prepareCLNT(BLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);

	tRPCStruct->ulCmd = a_ulCmd;

	// Sned value to video cpu directly and don't need to change endian because RPC will handle this.
	if (1 == a_ucValueOnly)
	{
		tRPCStruct->ulArgAddr = a_ulValue;
		tRPCStruct->ulArgSize = 0;
	}
	else
	{
		//protect for memory trash
		if ( (a_ulCmd >= SCALERIOC_LATESTITEM) || (g_atShareMemInfo[a_ulCmd].ulSize == 0) )
		{
			rtd_pr_vbe_err("\033[1;31m %s %s:%d, no ShareMem a_ulCmd = %d \033[m\n", __FUNCTION__, __FILE__, __LINE__, a_ulCmd);
			up(get_scalerrpc_semaphore());
			return -1;
		}
		tRPCStruct->ulArgAddr = g_atShareMemInfo[a_ulCmd].ulOffset + g_ulPhyAddr + MEM_OFFSET;
		tRPCStruct->ulArgSize = g_atShareMemInfo[a_ulCmd].ulSize;
	}

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	//Send RPC @ Crixus 20141009
	//change endian
	tRPCStruct->ulCmd = htonl(tRPCStruct->ulCmd);
	tRPCStruct->ulArgAddr = htonl(tRPCStruct->ulArgAddr);
	tRPCStruct->ulArgSize = htonl(tRPCStruct->ulArgSize);

	ret_rpc = send_rpc_command(RPC_VIDEO, VIDEO_RPC_ToAgent_ScalerDrv, RPCStruc_phy_addr, 0, &ret_value);
	//send RPC to video pass && scalerDrv ioctl return ok
	if((ret_rpc == RPC_FAIL) || (ret_value != KERNEL_RPC_OK)){
		if(ret_rpc == RPC_FAIL)
			rtd_pr_vbe_err("\033[1;31m Scaler_SendRPC ::::::RPC fail!! Cmd == %#x  \033[m \n", a_ulCmd);
		up(get_scalerrpc_semaphore());
		return -1;
	}
#endif
	up(get_scalerrpc_semaphore());
#endif
	return 0;
}
#ifndef BUILD_QUICK_SHOW 
#ifdef PLI_FAIL
static unsigned char g_aucRPCShareMem[60000000];
#endif
#endif
#ifdef CONFIG_ARM64
unsigned long Scaler_GetShareMemVirAddr(unsigned int a_ulCmd)
{
#ifndef BUILD_QUICK_SHOW
	if ( (a_ulCmd >= SCALERIOC_LATESTITEM) || (g_atShareMemInfo[a_ulCmd].ulSize == 0) || (0 == g_ulNonCachedAddr)) {//protect for memory trash
		rtd_pr_vbe_debug("\033[1;31m %s %s:%d, no ShareMem a_ulCmd = %d or memory is not allocated g_ulNonCachedAddr=%lx \033[m\n", __FUNCTION__, __FILE__, __LINE__, a_ulCmd, g_ulNonCachedAddr);
		//assert(0);
		return 0;
	}

	#ifdef PLI_FAIL
		return (unsigned long)&g_aucRPCShareMem;
	#else
		return (g_atShareMemInfo[a_ulCmd].ulOffset + g_ulNonCachedAddr);
	#endif
#else
	if ( (a_ulCmd >= SCALERIOC_LATESTITEM) || (g_atShareMemInfo[a_ulCmd].ulSize == 0) || (0 == g_quick_showAddr)) {//protect for memory trash
		printf("\033[1;31m %s %s:%d, no ShareMem a_ulCmd = %d or memory is not allocated g_quick_showAddr=%lx \033[m\n", __FUNCTION__, __FILE__, __LINE__, a_ulCmd, g_quick_showAddr);
		//assert(0);
		return 0;
	}

	//printf2(" Scaler_GetShareMemVirAddr =0x%x \n",g_atShareMemInfo[a_ulCmd].ulOffset+ g_ulNonCachedAddr);
	return (g_atShareMemInfo[a_ulCmd].ulOffset+ (unsigned int)g_quick_showAddr);
#endif	
}
unsigned long (*p_Scaler_GetShareMemVirAddr)(unsigned int) = Scaler_GetShareMemVirAddr;
#else
unsigned int Scaler_GetShareMemVirAddr(unsigned int a_ulCmd)
{
#ifndef BUILD_QUICK_SHOW
	if ( (a_ulCmd >= SCALERIOC_LATESTITEM) || (g_atShareMemInfo[a_ulCmd].ulSize == 0) || (0 == g_ulNonCachedAddr)) {//protect for memory trash
		rtd_pr_vbe_debug("\033[1;31m %s %s:%d, no ShareMem a_ulCmd = %d or memory is not allocated g_ulNonCachedAddr=%lx \033[m\n", __FUNCTION__, __FILE__, __LINE__, a_ulCmd, g_ulNonCachedAddr);
		//assert(0);
		return 0;
	}

	#ifdef PLI_FAIL
		return (unsigned int)&g_aucRPCShareMem;
	#else
		return (g_atShareMemInfo[a_ulCmd].ulOffset + g_ulNonCachedAddr);
	#endif
#else
	if ( (a_ulCmd >= SCALERIOC_LATESTITEM) || (g_atShareMemInfo[a_ulCmd].ulSize == 0) || (0 == g_quick_showAddr)) {//protect for memory trash
		printf("\033[1;31m %s %s:%d, no ShareMem a_ulCmd = %d or memory is not allocated g_quick_showAddr=%lx \033[m\n", __FUNCTION__, __FILE__, __LINE__, a_ulCmd, g_quick_showAddr);
		//assert(0);
		return 0;
	}

	//printf2(" Scaler_GetShareMemVirAddr =0x%x \n",g_atShareMemInfo[a_ulCmd].ulOffset+ g_ulNonCachedAddr);
	return (g_atShareMemInfo[a_ulCmd].ulOffset+ (unsigned int)g_quick_showAddr);
#endif		
}
unsigned int (*p_Scaler_GetShareMemVirAddr)(unsigned int) = Scaler_GetShareMemVirAddr;
#endif

void Scaler_SetRPCInitByVCPU(unsigned char flag){
	/*False:VCPU RPC initilize OK.*/
	/*True:VCPU RPC initilize fail.*/
	RPCVCPUInitFail = flag;
}
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
unsigned char Scaler_GetRPCInitByVCPU(void){
	return RPCVCPUInitFail;
}
#endif

static long SendShareMemAddrByRPC(void)
{
#ifndef BUILD_QUICK_SHOW
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long ret = 0;
	int ret_val = 0;
	UINT8 count = 0;
#endif
	SCALER_RPC_SHARE_MEM *tRPCShareMem;
	unsigned long  vir_addr = 0, vir_addr_noncache;
	unsigned int  phy_addr;

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(SCALER_RPC_SHARE_MEM), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
	//get physical address
	//phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);
	tRPCShareMem = (SCALER_RPC_SHARE_MEM *)vir_addr_noncache;

#if 1
	//USER:LewisLee DATE:2014/10/06
	//to prevent shut down
	if(/*(0 == g_atShareMemInfo) || */(0 == g_ulPhyAddr) || (NULL == tRPCShareMem))
	{
		rtd_pr_vbe_debug("g_atShareMemInfo : %lx, g_ulPhyAddr : %lx, tRPCShareMem : %lx\n", (unsigned long)g_atShareMemInfo, (unsigned long)g_ulPhyAddr, (unsigned long)tRPCShareMem);
		return -1;
	}
#ifdef CONFIG_ARM64
	memset_io(tRPCShareMem, 0, sizeof(SCALER_RPC_SHARE_MEM));
#else
	memset(tRPCShareMem, 0, sizeof(SCALER_RPC_SHARE_MEM));
#endif
#else //#if 1
	memset(&tRPCShareMem, 0, sizeof(SCALER_RPC_SHARE_MEM));
#endif //#if 1

	//clnt = prepareCLNT(NONBLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);
	//clnt = prepareCLNT(BLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);
    // === scalerVIP.c ===
	// tRPCShareMem->ulAddr[HIST_CNT_MEM_IDX] = g_atShareMemInfo[SCALERIOC_GETCOLORHISTOGRAM].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[RPC_SMARTPIC_CLUS_IDX] = g_atShareMemInfo[SCALERIOC_VIP_RPC_SMARTPIC_CLUS].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	// tRPCShareMem->ulAddr[HIST_CNT_SUB_MEM_IDX] = g_atShareMemInfo[SCALERIOC_GETCOLORHISTOGRAM_SUB].ulOffset + g_ulPhyAddr + MEM_OFFSET;
//	tRPCShareMem->ulAddr[EMF_COEF_MEM_IDX] = g_atShareMemInfo[SCALERIOC_SETEMF].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[DCR_TABLE_MEM_IDX] = g_atShareMemInfo[SCALERIOC_DCR_TABLE].ulOffset + g_ulPhyAddr + MEM_OFFSET;
    // === scalerMemory.c ===
	tRPCShareMem->ulAddr[MEM_TAG_MEM_IDX] = g_atShareMemInfo[SCALERIOC_SET_DOUBLEBUF].ulOffset + g_ulPhyAddr + MEM_OFFSET;
    // === scalerVO.c ===
	tRPCShareMem->ulAddr[VO_INFO_MEM_IDX] = g_atShareMemInfo[SCALERIOC_GETVOINFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[MVD_INFO_MEM_IDX] = g_atShareMemInfo[SCALERIOC_GETMVDINFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[INFO_3DDETECT_MEM_IDX] = g_atShareMemInfo[SCALERIOC_GET3DDETECTINFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
    // === scalerIRQ.c ===
    	// oliver+, fix me
	//tRPCShareMem.ulAddr[JPEG_CTRL_MEM_IDX] = g_atShareMemInfo[].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[KB_INFO_MEM_IDX] = g_atShareMemInfo[SCALERIOC_SETKBINFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	// HDR -- Dolby Vision
	tRPCShareMem->ulAddr[HDR_DV_HDMI_INFO_MEM_IDX] = g_atShareMemInfo[SCALERIOC_GET_HDMI_HDR_FRAMEINFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[HDR_DV_OTT_INFO_MEM_IDX] = g_atShareMemInfo[SCALERIOC_GET_OTT_HDR_FRAMEINFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[VO_SMOOTH_TOGGLE_IDX] = g_atShareMemInfo[SCALERIOC_SMOOTHTOGGLE_SHARE_INFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[VO_SMOOTH_TOGGLE_SHAREMEM_SYNC_IDX] = g_atShareMemInfo[SCALERIOC_SMOOTHTOGGLE_SHAREMEMORY_SYNC_FALG].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[VO_SMOOTH_TOGGLE_SHIFTXY_SHAREMEM_SYNC_IDX] = g_atShareMemInfo[SCALERIOC_SMOOTHTOGGLE_SHIFTXY_SHAREMEMORY_SYNC_FALG].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[VO_RUN_SCALER_IDX] = g_atShareMemInfo[VO_RUN_SCALER_FLAG].ulOffset + g_ulPhyAddr + MEM_OFFSET;
    tRPCShareMem->ulAddr[DM_CONNECT_FLAG_IDX] = g_atShareMemInfo[SCALERIOC_DM_CONNECT_FLAG].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_IDX] = g_atShareMemInfo[SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_FLAG].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[VBI_SHAREMEM_DATA_IDX] = g_atShareMemInfo[SCALERIOC_VBI_SHAREMEMORY_DATA].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	//vo photo
	tRPCShareMem->ulAddr[VO_PHOTO_UPDATE_SHAREMEM_IDX] = g_atShareMemInfo[SCALERIOC_PHOTO_UPDATE_SHAREMEM_FALG].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[VO_PHOTO_SHIFT_SHAREMEM_IDX] = g_atShareMemInfo[SCALERIOC_PHOTO_SHIFT_SHAREMEM_INFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[SCALERIOC_DUMP_REGISTER_COUNTER_IDX] = g_atShareMemInfo[SCALERIOC_DUMP_REGISTER_COUNTER].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	//tRPCShareMem->ulAddr[SCALERIOC_DUMP_REGISTER_INFO_IDX] = g_atShareMemInfo[SCALERIOC_DUMP_REGISTER_INFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[SCALERIOC_AV_SYNC_IDX] = g_atShareMemInfo[SCALERIOC_AV_SYNC_READY_FLAG].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[SCALERIOC_PANORAMA_IDX] = g_atShareMemInfo[SCALERIOC_SET_PANORAMA].ulOffset + g_ulPhyAddr + MEM_OFFSET;
        //#ifdef CONFIG_SUPPORT_DOLBY_VSIF
	tRPCShareMem->ulAddr[HDMI_DOLBY_VSIF_IDX] = g_atShareMemInfo[HDMI_DOLBY_VSIF_FLAG].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	//#endif

#ifdef CONFIG_I2RND_ENABLE
	//i2r sub hdr info
	tRPCShareMem->ulAddr[SCALERIOC_SUB_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_IDX] = g_atShareMemInfo[SCALERIOC_SUB_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_FLAG].ulOffset + g_ulPhyAddr + MEM_OFFSET;
#endif

#ifdef NEW_INPUT_OUTPUT_BUFFER_FLOW_CONFIG
	tRPCShareMem->ulAddr[SCALER_MAIN_INPUT_OUTPUT_BUFFER_IDX] = g_atShareMemInfo[SCALERIOC_MAIN_INPUT_OUTPUT_BUFFER_INFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
#ifdef NEW_INPUT_OUTPUT_BUFFER_FLOW_SUPPORT_SUB
	tRPCShareMem->ulAddr[SCALER_SUB_INPUT_OUTPUT_BUFFER_IDX] = g_atShareMemInfo[SCALERIOC_SUB_INPUT_OUTPUT_BUFFER_INFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
#endif
#endif
	tRPCShareMem->ulAddr[FILM_MODE_CHANGE_TYPE_IDX] = g_atShareMemInfo[FILM_MODE_CHANGE_TYPE].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[SCALER_WIN_CALLBACK_DELAY_IDX] = g_atShareMemInfo[SCALERIOC_WIN_CALLBACK_DELAY_INFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[DYNAMIC_FRAMERATE_CHANGE_INFO_IDX] = g_atShareMemInfo[DYNAMIC_FRAMERATE_CHANGE_INFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[VO_FORCE_V_TOP_IDX] = g_atShareMemInfo[SCALERIOC_VO_FORCE_V_TOP].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[SCALERIOC_SETUP_DI_DONE_IDX] = g_atShareMemInfo[SCALERIOC_SETUP_DI_DONE].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[PC_MODE_SYNC_INFO_IDX] = g_atShareMemInfo[SCALERIOC_PC_MODE_INFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[ORBIT_SHIFT_INFO_IDX] = g_atShareMemInfo[SCALERIOC_ORBIT_SHIFT_INFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[SCALER_BSP_ERROR_REPORT_INFO_BUFFER_IDX] = g_atShareMemInfo[SCALERIOC_BSP_ERROR_REPORT_INFO_BUFFER].ulOffset + g_ulPhyAddr + MEM_OFFSET;////bsp error share memory
	tRPCShareMem->ulAddr[CINEMA_MODE_ENABLE_IDX] = g_atShareMemInfo[SCALERIOC_CINEMA_MODE_ENABLE].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[VO_TRACK_ENABLE_IDX] = g_atShareMemInfo[SCALERIOC_VO_TRACK_ENABLE].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[VO_TRACK_I3_ENABLE_IDX] = g_atShareMemInfo[SCALERIOC_VO_TRACK_I3_ENABLE].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[I3_NN_SHARE_BUF_STATUS_IDX] = g_atShareMemInfo[SCALERIOC_I3_NN_SHARE_BUF_STATUS].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[I3DDMA_VODMA_COMP_ATTR_IDX] = g_atShareMemInfo[SCALERIOC_SET_I3DDMA_VODMA_COMP_ATTR].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[SCALERIOC_GAMEMODE_DATA_IDX] = g_atShareMemInfo[SCALERIOC_GAMEMODE_DATA].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[VO_TRACK_I3_M_STAGE_IDX] = g_atShareMemInfo[SCALERIOC_VO_TRACK_I3_M_STAGE].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[VO_IVS_SRC_INFO_IDX] = g_atShareMemInfo[SCALERIOC_VO_IVS_SRC_INFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[VO_FORCE_60_FRAMERATE_IDX] = g_atShareMemInfo[SCALERIOC_VO_FORCE_60_FRAMERATE_STATE].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[I3_CAPTURE_INDEX_IDX] = g_atShareMemInfo[SCALERIOC_I3_CAPTURE_INDEX].ulOffset + g_ulPhyAddr + MEM_OFFSET;
	tRPCShareMem->ulAddr[DOLBY_BUFFER_IDX] = g_atShareMemInfo[SCALERIOC_DOLBY_BUFFER].ulOffset + g_ulPhyAddr + MEM_OFFSET;
   	tRPCShareMem->ulAddr[HDMI_QMS_MODE_STATUS_IDX] = g_atShareMemInfo[SCALERIOC_MDMI_QMS_INFO].ulOffset + g_ulPhyAddr + MEM_OFFSET;//qms share memory

	//change endian @ Crixus 20141009
	// tRPCShareMem->ulAddr[HIST_CNT_MEM_IDX] = htonl(tRPCShareMem->ulAddr[HIST_CNT_MEM_IDX]);
	//tRPCShareMem->ulAddr[SMARTPIC_CLUS_IDX] = htonl(tRPCShareMem->ulAddr[SMARTPIC_CLUS_IDX]);
	tRPCShareMem->ulAddr[RPC_SMARTPIC_CLUS_IDX] = htonl(tRPCShareMem->ulAddr[RPC_SMARTPIC_CLUS_IDX]);
	// tRPCShareMem->ulAddr[HIST_CNT_SUB_MEM_IDX] = htonl(tRPCShareMem->ulAddr[HIST_CNT_SUB_MEM_IDX]);
	tRPCShareMem->ulAddr[DCR_TABLE_MEM_IDX] = htonl(tRPCShareMem->ulAddr[DCR_TABLE_MEM_IDX]);
	tRPCShareMem->ulAddr[MEM_TAG_MEM_IDX] = htonl(tRPCShareMem->ulAddr[MEM_TAG_MEM_IDX]);
	tRPCShareMem->ulAddr[VO_INFO_MEM_IDX] = htonl(tRPCShareMem->ulAddr[VO_INFO_MEM_IDX]);
	tRPCShareMem->ulAddr[MVD_INFO_MEM_IDX] = htonl(tRPCShareMem->ulAddr[MVD_INFO_MEM_IDX]);
	tRPCShareMem->ulAddr[INFO_3DDETECT_MEM_IDX] = htonl(tRPCShareMem->ulAddr[INFO_3DDETECT_MEM_IDX]);
	tRPCShareMem->ulAddr[KB_INFO_MEM_IDX] = htonl(tRPCShareMem->ulAddr[KB_INFO_MEM_IDX]);
	tRPCShareMem->ulAddr[HDR_DV_HDMI_INFO_MEM_IDX] = htonl(tRPCShareMem->ulAddr[HDR_DV_HDMI_INFO_MEM_IDX]);
	tRPCShareMem->ulAddr[HDR_DV_OTT_INFO_MEM_IDX] = htonl(tRPCShareMem->ulAddr[HDR_DV_OTT_INFO_MEM_IDX]);
	tRPCShareMem->ulAddr[VO_SMOOTH_TOGGLE_IDX] = htonl(tRPCShareMem->ulAddr[VO_SMOOTH_TOGGLE_IDX]);
	tRPCShareMem->ulAddr[VO_SMOOTH_TOGGLE_SHAREMEM_SYNC_IDX] = htonl(tRPCShareMem->ulAddr[VO_SMOOTH_TOGGLE_SHAREMEM_SYNC_IDX]);
	tRPCShareMem->ulAddr[VO_SMOOTH_TOGGLE_SHIFTXY_SHAREMEM_SYNC_IDX] = htonl(tRPCShareMem->ulAddr[VO_SMOOTH_TOGGLE_SHIFTXY_SHAREMEM_SYNC_IDX]);
	tRPCShareMem->ulAddr[VO_RUN_SCALER_IDX] = htonl(tRPCShareMem->ulAddr[VO_RUN_SCALER_IDX]);
	tRPCShareMem->ulAddr[DM_CONNECT_FLAG_IDX] = htonl(tRPCShareMem->ulAddr[DM_CONNECT_FLAG_IDX]);
	tRPCShareMem->ulAddr[SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_IDX] = htonl(tRPCShareMem->ulAddr[SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_IDX]);
	tRPCShareMem->ulAddr[VBI_SHAREMEM_DATA_IDX] = htonl(tRPCShareMem->ulAddr[VBI_SHAREMEM_DATA_IDX]);
	//vo photo
	tRPCShareMem->ulAddr[VO_PHOTO_UPDATE_SHAREMEM_IDX] = htonl(tRPCShareMem->ulAddr[VO_PHOTO_UPDATE_SHAREMEM_IDX]);
	tRPCShareMem->ulAddr[VO_PHOTO_SHIFT_SHAREMEM_IDX] = htonl(tRPCShareMem->ulAddr[VO_PHOTO_SHIFT_SHAREMEM_IDX]);
	tRPCShareMem->ulAddr[SCALERIOC_DUMP_REGISTER_COUNTER_IDX] = htonl(tRPCShareMem->ulAddr[SCALERIOC_DUMP_REGISTER_COUNTER_IDX]);
	tRPCShareMem->ulAddr[SCALERIOC_AV_SYNC_IDX] = htonl(tRPCShareMem->ulAddr[SCALERIOC_AV_SYNC_IDX]);
	tRPCShareMem->ulAddr[SCALERIOC_PANORAMA_IDX] = htonl(tRPCShareMem->ulAddr[SCALERIOC_PANORAMA_IDX]);
	tRPCShareMem->ulAddr[PC_MODE_SYNC_INFO_IDX] = htonl(tRPCShareMem->ulAddr[PC_MODE_SYNC_INFO_IDX]);
        //#ifdef CONFIG_SUPPORT_DOLBY_VSIF
	tRPCShareMem->ulAddr[HDMI_DOLBY_VSIF_IDX] = htonl(tRPCShareMem->ulAddr[HDMI_DOLBY_VSIF_IDX]);
	//#endif

#ifdef CONFIG_I2RND_ENABLE
	//i2r sub hdr info
	tRPCShareMem->ulAddr[SCALERIOC_SUB_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_IDX] = htonl(tRPCShareMem->ulAddr[SCALERIOC_SUB_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_IDX]);
#endif

#ifdef NEW_INPUT_OUTPUT_BUFFER_FLOW_CONFIG
	tRPCShareMem->ulAddr[SCALER_MAIN_INPUT_OUTPUT_BUFFER_IDX] = htonl(tRPCShareMem->ulAddr[SCALER_MAIN_INPUT_OUTPUT_BUFFER_IDX]);
#ifdef NEW_INPUT_OUTPUT_BUFFER_FLOW_SUPPORT_SUB
	tRPCShareMem->ulAddr[SCALER_SUB_INPUT_OUTPUT_BUFFER_IDX] = htonl(tRPCShareMem->ulAddr[SCALER_SUB_INPUT_OUTPUT_BUFFER_IDX]);
#endif
#endif
	tRPCShareMem->ulAddr[FILM_MODE_CHANGE_TYPE_IDX] = htonl(tRPCShareMem->ulAddr[FILM_MODE_CHANGE_TYPE_IDX]);
	tRPCShareMem->ulAddr[SCALER_WIN_CALLBACK_DELAY_IDX] = htonl(tRPCShareMem->ulAddr[SCALER_WIN_CALLBACK_DELAY_IDX]);
	tRPCShareMem->ulAddr[DYNAMIC_FRAMERATE_CHANGE_INFO_IDX] = htonl(tRPCShareMem->ulAddr[DYNAMIC_FRAMERATE_CHANGE_INFO_IDX]);
	tRPCShareMem->ulAddr[VO_FORCE_V_TOP_IDX] = htonl(tRPCShareMem->ulAddr[VO_FORCE_V_TOP_IDX]);
	tRPCShareMem->ulAddr[SCALERIOC_SETUP_DI_DONE_IDX] = htonl(tRPCShareMem->ulAddr[SCALERIOC_SETUP_DI_DONE_IDX]);
	tRPCShareMem->ulAddr[ORBIT_SHIFT_INFO_IDX] = htonl(tRPCShareMem->ulAddr[ORBIT_SHIFT_INFO_IDX]);
	tRPCShareMem->ulAddr[SCALER_BSP_ERROR_REPORT_INFO_BUFFER_IDX] = htonl(tRPCShareMem->ulAddr[SCALER_BSP_ERROR_REPORT_INFO_BUFFER_IDX]);//bsp error share memory
	tRPCShareMem->ulAddr[CINEMA_MODE_ENABLE_IDX] = htonl(tRPCShareMem->ulAddr[CINEMA_MODE_ENABLE_IDX]);
	tRPCShareMem->ulAddr[VO_TRACK_ENABLE_IDX] = htonl(tRPCShareMem->ulAddr[VO_TRACK_ENABLE_IDX]);
	tRPCShareMem->ulAddr[VO_TRACK_I3_ENABLE_IDX] = htonl(tRPCShareMem->ulAddr[VO_TRACK_I3_ENABLE_IDX]);
	tRPCShareMem->ulAddr[I3_NN_SHARE_BUF_STATUS_IDX] = htonl(tRPCShareMem->ulAddr[I3_NN_SHARE_BUF_STATUS_IDX]);
	tRPCShareMem->ulAddr[I3DDMA_VODMA_COMP_ATTR_IDX] = htonl(tRPCShareMem->ulAddr[I3DDMA_VODMA_COMP_ATTR_IDX]);
	tRPCShareMem->ulAddr[SCALERIOC_GAMEMODE_DATA_IDX] = htonl(tRPCShareMem->ulAddr[SCALERIOC_GAMEMODE_DATA_IDX]);
	tRPCShareMem->ulAddr[VO_TRACK_I3_M_STAGE_IDX] = htonl(tRPCShareMem->ulAddr[VO_TRACK_I3_M_STAGE_IDX]);
	tRPCShareMem->ulAddr[VO_IVS_SRC_INFO_IDX] = htonl(tRPCShareMem->ulAddr[VO_IVS_SRC_INFO_IDX]);
        tRPCShareMem->ulAddr[VO_FORCE_60_FRAMERATE_IDX] = htonl(tRPCShareMem->ulAddr[VO_FORCE_60_FRAMERATE_IDX]);
        tRPCShareMem->ulAddr[I3_CAPTURE_INDEX_IDX] = htonl(tRPCShareMem->ulAddr[I3_CAPTURE_INDEX_IDX]);
	tRPCShareMem->ulAddr[DOLBY_BUFFER_IDX] = htonl(tRPCShareMem->ulAddr[DOLBY_BUFFER_IDX]);
        tRPCShareMem->ulAddr[HDMI_QMS_MODE_STATUS_IDX] = htonl(tRPCShareMem->ulAddr[HDMI_QMS_MODE_STATUS_IDX]);//qms share memory
        
	rtd_pr_vbe_debug("System SendShareMemAddrByRPC\n");

	// Send RPC
	//VIDEO_RPC_ToAgent_SetShareMemAddr_0(&tRPCShareMem, &clnt);

	//if (Scaler_IsUnitTest())
		//return ret;

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	do{

		ret_val = send_rpc_command(RPC_VIDEO, VIDEO_RPC_ToAgent_SetShareMemAddr, phy_addr, 0, &ret);
		if (RPC_FAIL == ret_val){
			rtd_pr_vbe_err("RPC fail!!\n");
			dvr_free((void*)vir_addr);
			return -1;
		}

		if (RPC_OK == ret_val){
			dvr_free((void*)vir_addr);
			return RPC_OK;
		}

		msleep(20);
		count++;
	}while((ret_val == RPC_VCPU_NOT_READY) && (count < 100));

	if(count == 100)
	{
		rtd_pr_vbe_err("SendShareMemAddrByRPC RPC timeout!!\n");
	}
	rtd_pr_vbe_debug("RPC ret_val=%d\n", ret_val);

	if (ret_val == RPC_VCPU_NOT_READY) {
		Scaler_SetRPCInitByVCPU(TRUE);
		rtd_pr_vbe_debug("RPC_VCPU_NOT_READY fail!!\n");
	}
	else{
		Scaler_SetRPCInitByVCPU(FALSE);
	}
#endif

	dvr_free((void*)vir_addr);
	return RPC_OK;
#else
        return 0;
#endif

}



unsigned int Scaler_InitRPCShareMem(void)
{
#ifndef BUILD_QUICK_SHOW
		long lRPCError = 0;
		unsigned long  vir_addr = 0, vir_addr_noncache;

		rtd_pr_vbe_debug("Scaler_InitRPCShareMem xxxxx\n");

		if (scalerRPC_isInit)
			return 0;

		// Init Offset
		g_ulShareMemOffset = 0;

		g_ulNonCachedAddr = 0;

		//initial scaler RPC struct @ Crixus 20141009
		vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(SCALER_RPC_STRUCT), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
		//get physical address
		//RPCStruc_phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
		RPCStruc_phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);
		tRPCStruct = (SCALER_RPC_STRUCT *)vir_addr_noncache;


		memset(g_atShareMemInfo, 0, sizeof(RPC_SHARE_MEM_INFO)*SCALERIOC_LATESTITEM);

        	SetRPCShareMem(SCALERIOC_SEND_DISP_READY_IRC, sizeof(SCALERIOC_SEND_DISP_READY_IRC));

		//========scalerDrv.c====================
		//SCALERIOC_SETBRIGHTNESS
		SetRPCShareMem(SCALERIOC_SETBRIGHTNESS, sizeof(unsigned char));

		//SCALERIOC_SETCONTRAST
		SetRPCShareMem(SCALERIOC_SETCONTRAST, sizeof(unsigned char));

		// SCALERIOC_VBICCPLAY
		SetRPCShareMem(SCALERIOC_VBICCPLAY, sizeof(unsigned char));

		// SCALERIOC_SET_TV_SCAN_STATUS
		SetRPCShareMem(SCALERIOC_SET_TV_SCAN_STATUS, sizeof(INT8));

		// SCALERIOC_SET_MMAP_MODE
		SetRPCShareMem(SCALERIOC_SET_MMAP_MODE, sizeof(SCALER_MMAP_MODE));

		// SCALERIOC_VBITT_FUNCTION
		SetRPCShareMem(SCALERIOC_VBITT_FUNCTION, sizeof(TTX_VBI_IOCTL_PARAM));

		// SCALERIOC_VBI_ENABLE
		SetRPCShareMem(SCALERIOC_VBI_ENABLE, sizeof(VBI_IOCTL_PARAM));

		//SCALERIOC_VO_ROTATE_ENABLE
		SetRPCShareMem(SCALERIOC_VO_ROTATE_ENABLE, sizeof(ROTATE_IOCTL_PARAM));

		//SCALERIOC_VIDEO_DELAY_ENABLE
		SetRPCShareMem(SCALERIOC_VIDEO_DELAY_ENABLE, sizeof(FRAME_DELAY_IOCTL_PARAM));

		//SCALERIOC_VIDEO_FILMMODE_ENABLE
		SetRPCShareMem(SCALERIOC_VIDEO_FILMMODE_ENABLE, sizeof(FILM_MODE_IOCTL_PARAM));

		// SCALERIOC_GET3DDETECTFMT
		SetRPCShareMem(SCALERIOC_GET3DDETECTFMT, sizeof(unsigned int));

		//========scalerInfo.c====================
		//SCALERIOC_SETDISPINFO
		SetRPCShareMem(SCALERIOC_SETDISPINFO, sizeof(SCALER_DISP_INFO) * MAX_DISP_CHANNEL_NUM);

		// SCALERIOC_SETDISP3DINFO
		SetRPCShareMem(SCALERIOC_SETDISP3DINFO, sizeof(SCALER_3D_DISP_INFO));

		// SCALERIOC_SETSRCINFO
		SetRPCShareMem(SCALERIOC_SETSRCINFO, Scaler_GetRPCSrcInfoSize());

		//=======scalerIRQ.c=====================
		//SCALERIOC_SETADAPTIVE3D
		SetRPCShareMem(SCALERIOC_SETADAPTIVE3D, sizeof(INT8));

		//SCALERIOC_SETTVSCANMODE
		SetRPCShareMem(SCALERIOC_SETTVSCANMODE, sizeof(INT8));

		// SCALERIOC_GETHISTFLAG
		SetRPCShareMem(SCALERIOC_GETHISTFLAG, sizeof(INT8));

		// CALERIOC_SETHISTFLAG
		SetRPCTheSameShareMem(SCALERIOC_SETHISTFLAG, SCALERIOC_GETHISTFLAG);

		// SCALERIOC_GETAUTOMAFLAG
		SetRPCShareMem(SCALERIOC_GETAUTOMAFLAG, sizeof(INT8));

		// SCALERIOC_SETAUTOMAFLAG
		SetRPCTheSameShareMem(SCALERIOC_SETAUTOMAFLAG, SCALERIOC_GETAUTOMAFLAG);

		// SCALERIOC_SETKBFLAG
		SetRPCShareMem(SCALERIOC_SETKBFLAG, sizeof(INT8));

		// SCALERIOC_SETKBINFO
		SetRPCShareMem(SCALERIOC_SETKBINFO, sizeof(KB_Param));


		//=======scalerVideo.c=====================

		// SCALERIOC_SET_TABLE_NTSC_3D
		SetRPCShareMem(SCALERIOC_SET_TABLE_NTSC_3D, sizeof(unsigned char)*30*8*2);

		// SCALERIOC_SET_TABLE_PAL_3D
		SetRPCShareMem(SCALERIOC_SET_TABLE_PAL_3D, sizeof(unsigned char)*30*8*2);

		// SCALERIOC_GETYCSEP
		SetRPCShareMem(SCALERIOC_GETYCSEP, sizeof(unsigned char));

		// SCALERIOC_SETYCSEP
		SetRPCTheSameShareMem(SCALERIOC_SETYCSEP, SCALERIOC_GETYCSEP);

		// SCALERIOC_GETFH1DFLAG
		SetRPCShareMem(SCALERIOC_GETFH1DFLAG, sizeof(INT8));

		// SCALERIOC_GETTVCHECK3DFLAG
		SetRPCShareMem(SCALERIOC_GETTVCHECK3DFLAG, sizeof(INT8));

		// SCALERIOC_SETTVCHECK3DFLAG
		SetRPCTheSameShareMem(SCALERIOC_SETTVCHECK3DFLAG, SCALERIOC_GETTVCHECK3DFLAG);

		// SCALERIOC_GET3DTABLEINDEX
		SetRPCShareMem(SCALERIOC_GET3DTABLEINDEX, sizeof(unsigned char));

		//SCALERIOC_SET3DTABLEINDEX
		SetRPCTheSameShareMem(SCALERIOC_SET3DTABLEINDEX, SCALERIOC_GET3DTABLEINDEX);

		//========scalerVip.c=========================

		// SCALERIOC_GETCOLORHISTOGRAM
		SetRPCShareMem(SCALERIOC_GETCOLORHISTOGRAM, sizeof(unsigned int)*COLOR_HISTOGRAM_LEVEL);

		// SCALERIOC_GETCOLORHISTOGRAM
		SetRPCShareMem(SCALERIOC_GETCOLORHISTOGRAM_SUB, sizeof(unsigned int)*COLOR_HISTOGRAM_LEVEL);

		// SCALERIOC_SETRTNR_Y
		SetRPCShareMem(SCALERIOC_SETRTNR_Y, (sizeof(unsigned char)*Scaler_GetSourceListLen()+1));

		// SCALERIOC_SETRTNR_C
		SetRPCShareMem(SCALERIOC_SETRTNR_C, (sizeof(unsigned char)*Scaler_GetSourceListLen()+1));

		// SCALERIOC_UPDATERTNR_Y
		SetRPCShareMem(SCALERIOC_UPDATERTNR_Y, sizeof(unsigned char)*2);

		// SCALERIOC_UPDATERTNR_C
		SetRPCShareMem(SCALERIOC_UPDATERTNR_C, sizeof(unsigned char)*2);

		// SCALERIOC_GETAUTONR
		SetRPCShareMem(SCALERIOC_GETAUTONR, sizeof(unsigned char));

		// SCALERIOC_SETAUTONR
		SetRPCTheSameShareMem(SCALERIOC_SETAUTONR, SCALERIOC_GETAUTONR);

		// SCALERIOC_SETDNR
		SetRPCShareMem(SCALERIOC_SETDNR, (sizeof(unsigned char)*Scaler_GetSourceListLen()+1));

		// SCALERIOC_UPDATEDNR
		SetRPCShareMem(SCALERIOC_UPDATEDNR, sizeof(unsigned char)*2);

		// SCALERIOC_UPDATE_ACTIVE_BACKLIGHT_TABLE
		SetRPCShareMem(SCALERIOC_UPDATE_ACTIVE_BACKLIGHT_TABLE, sizeof(unsigned char)*18);

		// SCALERIOC_GET_MPEGNR
		SetRPCShareMem(SCALERIOC_GET_MPEGNR, sizeof(SCALERVIP_MPEGNR));

		// SCALERIOC_SET_TABLENRSPATIAL
		SetRPCShareMem(SCALERIOC_SET_TABLENRSPATIAL, sizeof(unsigned char));

		// SCALERIOC_GET_TABLENRSPATIAL
		SetRPCTheSameShareMem(SCALERIOC_GET_TABLENRSPATIAL, SCALERIOC_SET_TABLENRSPATIAL);

		// SCALERIOC_SET_PICTURE_MODE
		//SetRPCShareMem(SCALERIOC_SET_PICTURE_MODE, sizeof(unsigned char));

		// SCALERIOC_DCR_TABLE
		SetRPCShareMem(SCALERIOC_DCR_TABLE, sizeof(UINT16)*DCR_MAX*DCR_NODE_NUM);

		// SCALERIOC_VIP_system_info_structure
		SetRPCShareMem(SCALERIOC_VIP_system_info_structure, sizeof(_system_setting_info));


		// SCALERIOC_GET_NRAVGCOUNT
		SetRPCShareMem(SCALERIOC_GET_NRAVGCOUNT, sizeof(unsigned char));

		// SCALERIOC_SET_NRAVGCOUNT
		SetRPCTheSameShareMem(SCALERIOC_SET_NRAVGCOUNT, SCALERIOC_GET_NRAVGCOUNT);

		//NR_log
		// SCALERIOC_GETDNR_log
		SetRPCShareMem(SCALERIOC_GETDNR_log, sizeof(unsigned char));

		// SCALERIOC_SETDNR_log
		SetRPCTheSameShareMem(SCALERIOC_SETDNR_log, SCALERIOC_GETDNR_log);

		// SCALERIOC_GETRTNR_log
		SetRPCShareMem(SCALERIOC_GETRTNR_log, sizeof(unsigned char));

		// SCALERIOC_SETRTNR_log
		SetRPCTheSameShareMem(SCALERIOC_SETRTNR_log, SCALERIOC_GETRTNR_log);

		// SCALERIOC_GETSNR_log
		SetRPCShareMem(SCALERIOC_GETSNR_log, sizeof(unsigned char));

		// SCALERIOC_SETSNR_log
		SetRPCTheSameShareMem(SCALERIOC_SETSNR_log, SCALERIOC_GETSNR_log);

		// SCALERIOC_GETINR_log
		SetRPCShareMem(SCALERIOC_GETINR_log, sizeof(unsigned char));

		// SCALERIOC_SETINR_log
		SetRPCTheSameShareMem(SCALERIOC_SETINR_log, SCALERIOC_GETINR_log);

		// SCALERIOC_GET_DEBUG_AUTO_NR
		SetRPCShareMem(SCALERIOC_GET_DEBUG_AUTO_NR, sizeof(unsigned char));

		// SCALERIOC_SET_DEBUG_AUTO_NR
		SetRPCTheSameShareMem(SCALERIOC_SET_DEBUG_AUTO_NR, SCALERIOC_GET_DEBUG_AUTO_NR);

		// SCALERIOC_GET_DEBUG_VD_NOISE
		SetRPCShareMem(SCALERIOC_GET_DEBUG_VD_NOISE, sizeof(unsigned char));

		// SCALERIOC_SET_DEBUG_VD_NOISE
		SetRPCTheSameShareMem(SCALERIOC_SET_DEBUG_VD_NOISE, SCALERIOC_GET_DEBUG_VD_NOISE);

		// SCALERIOC_GET_DEBUG_RTNR_MAD
		SetRPCShareMem(SCALERIOC_GET_DEBUG_RTNR_MAD, sizeof(unsigned char));

		// SCALERIOC_SET_DEBUG_RTNR_MAD
		SetRPCTheSameShareMem(SCALERIOC_SET_DEBUG_RTNR_MAD, SCALERIOC_GET_DEBUG_RTNR_MAD);

		// SCALERIOC_GET_DEBUG_HISTOGRAM_MEAN
		SetRPCShareMem(SCALERIOC_GET_DEBUG_HISTOGRAM_MEAN, sizeof(unsigned char));

		// SCALERIOC_SET_DEBUG_HISTOGRAM_MEAN
		SetRPCTheSameShareMem(SCALERIOC_SET_DEBUG_HISTOGRAM_MEAN, SCALERIOC_GET_DEBUG_HISTOGRAM_MEAN);

		//SCALERIOC_SET_TABLE_TV_AutoNR_Signal_Status
		//SetRPCShareMem(SCALERIOC_SET_TABLE_TV_AutoNR_Signal_Status, sizeof(unsigned int)*3);

		// SCALERIOC_SET_TABLE_DNR_LEVEL
		SetRPCShareMem(SCALERIOC_SET_DNR_LEVEL_TABLE, sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*DNR_ITEM_MAX*DNR_SELECT_MAX);

		//SCALERIOC_SET_PQA_TABLE_OFFSET_TEMP
		//SetRPCShareMem(SCALERIOC_SET_PQA_TABLE_OFFSET_TEMP, sizeof(unsigned int)*PQA_ITEM_MAX);		// move to share memory access struct, elieli

		// move "DNR table select info" to system_info_struct and replace by PQA table, elieli
		// SCALERIOC_GETDNRTABLE
		//SetRPCShareMem(SCALERIOC_GETDNRTABLE, sizeof(unsigned char));

		// move "DNR table select info" to system_info_struct and replace by PQA table, elieli
		// SCALERIOC_SETDNRTABLE
		//SetRPCTheSameShareMem(SCALERIOC_SETDNRTABLE, SCALERIOC_GETDNRTABLE);

/* === checksum ========== */
		// SCALERIOC_VIP_TABLE_CRC_STRUCT
		SetRPCShareMem(SCALERIOC_VIP_TABLE_CRC_STRUCT,  sizeof(VIP_table_crc_value));
/*==========================*/
		// SCALERIOC_VIP_TABLE_STRUCT
		SetRPCShareMem(SCALERIOC_VIP_TABLE_STRUCT,  sizeof(SLR_VIP_TABLE));

		SetRPCShareMem(SCALERIOC_VIP_FW_FILM,  sizeof(FILM_FW_ShareMem));

		// SCALERIOC_VIP_TABLE_CUSTOM_STRUCT
		#if 1//def SLR_VIP_TABLE_CUSTOM
		SetRPCShareMem(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT,  sizeof(SLR_VIP_TABLE_CUSTOM_TV001));
		#endif

		// new add
		 //SCALERIOC_PIN_BL_ADJ
		SetRPCShareMem(SCALERIOC_PIN_BL_ADJ, sizeof(unsigned int));

		// SCALERIOC_PIN_BL_ON_OFF
		SetRPCShareMem(SCALERIOC_PIN_BL_ON_OFF, sizeof(unsigned int));

		// SCALERIOC_BACKLIGHT_ON
		SetRPCShareMem(SCALERIOC_BACKLIGHT_ON, sizeof(unsigned int));

		// SCALERIOC_IS_INCREASE_MODE
		SetRPCShareMem(SCALERIOC_IS_INCREASE_MODE, sizeof(unsigned int));

		// SCALERIOC_BL_LV_FROM_USER
		SetRPCShareMem(SCALERIOC_BL_LV_FROM_USER, sizeof(unsigned char));

		// SCALERIOC_BL_LV_ACT_MAX
		SetRPCShareMem(SCALERIOC_BL_LV_ACT_MAX, sizeof(unsigned int));

		// SCALERIOC_BL_LV_ACT_MIN
		SetRPCShareMem(SCALERIOC_BL_LV_ACT_MIN, sizeof(unsigned int));

		// SCALERIOC_DCR_MAX_VALUE
		SetRPCShareMem(SCALERIOC_DCR_MAX_VALUE, sizeof(unsigned int));

		SetRPCShareMem(SCALERIOC_SETAUTOMADBFLAG, sizeof(unsigned char));

		SetRPCTheSameShareMem(SCALERIOC_GETAUTOMADBFLAG, SCALERIOC_SETAUTOMADBFLAG);

		//Elsie 20131210
		SetRPCShareMem(SCALERIOC_SET_COEFF_BY_Y, sizeof(unsigned int)*YUV2RGB_TABLE_SIZE);

		//Elsie 20140127 sync from Mac2
		SetRPCShareMem(SCALERIOC_SET_SRGB_HUE_SAT, sizeof(int)*9*(Bin_Num_sRGB+1));

		SetRPCShareMem(SCALERIOC_SET_GAMMA, sizeof(UINT32)*1536);

		SetRPCShareMem(SCALERIOC_GAMMA_ENABLE_PATCH, sizeof(unsigned char));

		SetRPCShareMem(SCALERIOC_VIP_SET_DCC_FORCE_UPDATE, sizeof(unsigned int));

		//========scalerVO.c=========================
		SetRPCShareMem(SCALERIOC_SETVOINFO, sizeof(unsigned char));

		SetRPCShareMem(SCALERIOC_SENDASPECTRATIO, sizeof(unsigned char));

		SetRPCShareMem(SCALERIOC_SENDVFLIP, sizeof(unsigned char));

		SetRPCShareMem(SCALERIOC_SENDZOOM, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_SENDPOSFINETUNE, sizeof(SLR_VO_Shift_Window));


		SetRPCShareMem(SCALERIOC_SENDRATIOSMOOTHTOGGLEINFO, sizeof(VO_RATIO_SMOOTH_TOGGLE_INFO));
		SetRPCShareMem(SCALERIOC_SENDSMOOTHTOGGLESTATE, sizeof(VO_Scaler_State));
		SetRPCShareMem(SCALERIOC_SENDSCALERDATAMODE, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_SENDSMOOTHTOGGLEUPDATEFLAG, sizeof(SMOOTH_TOGGLE_UPDATE_PLANE_INFO));
		SetRPCShareMem(SCALERIOC_SENDVGIPRATIO, sizeof(VGIP_Overscan_INFO));
		SetRPCShareMem(SCALERIOC_SENDPANELPARAMTERINFO, sizeof(Panel_Parameter_INFO));
		SetRPCShareMem(SCALERIOC_SENDSCALERMEMORYINFO, sizeof(SCALER_MEMORY_INFO));
		SetRPCShareMem(SCALERIOC_SENDZOOMINFO, sizeof(SCALER_ZOOM_INFO));//rika 20140630 added for fpp tcl api


		//SCALERIOC_GETVOINFO
		SetRPCShareMem(SCALERIOC_GETVOINFO, sizeof(SLR_VOINFO));

		//SCALERIOC_GETMVDINFO
		SetRPCShareMem(SCALERIOC_GETMVDINFO, sizeof(SLR_MVDINFO));

		SetRPCShareMem(SCALERIOC_GET3DDETECTINFO, sizeof(SLR_3DDETECTINFO));

		//========scalerVBI.c=========================
		SetRPCShareMem(SCALERIOC_VBICCSETBUF, sizeof(unsigned int));

		SetRPCShareMem(SCALERIOC_VBITTXSETBUF, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_VBIVPSSETBUF, sizeof(unsigned int));

		SetRPCShareMem(SCALERIOC_VBI_GETWSS, sizeof(UINT16));
		SetRPCShareMem(SCALERIOC_VBICGMSENABLE, sizeof(VBI_CGMS_PARAM));

		//========scalerMemory.c=========================
#ifndef CONFIG_DARWIN_SCALER // for MacArthur
		//SCALERIOC_ALLOC_BUFFER
		SetRPCShareMem(SCALERIOC_ALLOBUFFER, sizeof(SCALERDRV_ALLOCBUFFER_INFO));
#endif

//rotate info to vi & m-domain block read
//#ifdef ENABLE_SE_ROTATE_M_DOMAIN_BLOCK_MODE
		//SCALERIOC_SEND_ROTATE_INFO_VI_BLOCK_READ
		SetRPCShareMem(SCALERIOC_SEND_ROTATE_INFO_VI_BLOCK_READ,sizeof(SCALER_ROTATE_INFO));
//#endif
//#ifdef DTV_SCART_OUT_ENABLE
		//SCALERIOC_VO_SE_SCALING_ENABLE
		SetRPCShareMem(SCALERIOC_VO_SE_SCALING_ENABLE,sizeof(SCALER_DISPLAY_WINDOW));
//#endif
		//SCALERIOC_SET_DOUBLEBUF
		SetRPCShareMem(SCALERIOC_SET_DOUBLEBUF, sizeof(SCALERDRV_DOUBLEBUF_INFO));

		// Panel info
		SetRPCShareMem(SCALERIOC_SET_PANEL_INFO, sizeof(PANEL_INFO));


		// SCALERIOC_GET_TVAUTONR_Mode
		SetRPCShareMem(SCALERIOC_GET_TVAUTONR_Mode, sizeof(unsigned char));

		// SCALERIOC_SET_TVAUTONR_Mode
		SetRPCTheSameShareMem(SCALERIOC_SET_TVAUTONR_Mode, SCALERIOC_GET_TVAUTONR_Mode);

		// SCALERIOC_SET_TSB_style_ON
		SetRPCShareMem(SCALERIOC_SET_TSB_style_ON, sizeof(unsigned char));

		// SCALERIOC_GET_NR_style
		SetRPCShareMem(SCALERIOC_GET_NR_style, sizeof(unsigned char));

		// SCALERIOC_SET_NR_style
		SetRPCTheSameShareMem(SCALERIOC_SET_NR_style, SCALERIOC_GET_NR_style);

		//SCALERIOC_SET_OPEN_CELL_PANEL_OD_ON
		SetRPCShareMem(SCALERIOC_SET_OPEN_CELL_PANEL_OD_ON, sizeof(unsigned char));

		//SCALERIOC_Set_New_DCCmode
		SetRPCShareMem(SCALERIOC_Set_New_DCCmode, sizeof(unsigned char));

		//SCALERIOC_Set_Dcr_Panasonic
		SetRPCShareMem(SCALERIOC_Set_Dcr_Panasonic, sizeof(unsigned char));

		//SCALERIOC_SET_VIP_QUALITY_INIT
		SetRPCShareMem(SCALERIOC_SET_VIP_QUALITY_INIT,sizeof(unsigned int)*QUALITY_INIT_MAX);

		//SCALERIOC_SET_VIP_PICMODE_INIT
		SetRPCShareMem(SCALERIOC_SET_VIP_PICMODE_INIT,sizeof(unsigned int)*PICMODE_INIT_MAX);

		//SCALERIOC_SET_YC2D3D	 young YC3D_comb 20120830
		SetRPCShareMem(SCALERIOC_SET_YC2D3D, sizeof(unsigned char));
#if defined(IS_TV003_STYLE_PICTUREMODE)
		//SCALERIOC_SET_VIP_PICMODE_INIT
		SetRPCShareMem(SCALERIOC_VIP_ICM_TABLE,sizeof(unsigned short)*VIP_ICM_TBL_X*VIP_ICM_TBL_Y*VIP_ICM_TBL_Z);
#endif

		// SCALERIOC_VIP_TV003_share_memory_table
		SetRPCShareMem(SCALERIOC_VIP_TV003_share_memory_table, sizeof(RPC_TV003_Table));

		//SCALERIOC_SET_MDOMAIN_COMPRESSION_ENABLE
		SetRPCShareMem(SCALERIOC_SET_MDOMAIN_COMPRESSION_ENABLE, sizeof(M_DOMAIN_COMPRESSION_T));

		//SCALERIOC_SET_DC2H_SWMODE_ENABLE
		SetRPCShareMem(SCALERIOC_SET_DC2H_SWMODE_ENABLE, sizeof(DC2H_SWMODE_STRUCT_T));

		SetRPCShareMem(SCALERIOC_SET_VT_SE_CAPTURE_ENABLE, sizeof(VT_CUR_CAPTURE_INFO));

		SetRPCShareMem(SCALERIOC_SET_PANORAMA, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_AV_SYNC_READY_FLAG, sizeof(SCALER_AV_SYNC_INFO));

//#if RUN_MERLIN_MEMC_ENABLE
		//MEMC RPC shared memory initialization
		SetRPCShareMem(SCALERIOC_MEMC_INITIALIZATION, sizeof(SCALER_MEMC_DMASTARTADDRESS_T));
		SetRPCShareMem(SCALERIOC_MEMC_SETMOTIONCOMP, sizeof(VPQ_MEMC_SETMOTIONCOMP_T));
		SetRPCShareMem(SCALERIOC_MEMC_SETBLURLEVEL, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_MEMC_SETJUDDERLEVEL, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_MEMC_MOTIONCOMPONOFF, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_MEMC_LOWDELAYMODE, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_MEMC_SETRGBYUVMode, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_MEMC_GETFRAMEDELAY, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_MEMC_SETVIDEOBLOCK, sizeof(VPQ_MEMC_SETVIDEOBLOCK_T));
		SetRPCShareMem(SCALERIOC_MEMC_SETTRUEMOTIONDEMO, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_MEMC_GETFIRMWAREVERSION, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_MEMC_SETBYPASSREGION, sizeof(VPQ_MEMC_SETBYPASSREGION_T));
		SetRPCShareMem(SCALERIOC_MEMC_SETREVERSECONTROL, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_MEMC_FREEZE, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_MEMC_SETDEMOBAR, sizeof(VPQ_MEMC_SETDEMOBAR_T));
		SetRPCShareMem(SCALERIOC_MEMC_SETINOUTUSECASE, sizeof(SCALER_MEMC_SETINOUTUSECASE_T));
		SetRPCShareMem(SCALERIOC_MEMC_SETINPUTOUTPUTFORMAT, sizeof(SCALER_MEMC_SETINOUTPUTFORMAT_T));
		SetRPCShareMem(SCALERIOC_MEMC_SETINPUTOUTPUTRESOLUTION, sizeof(SCALER_MEMC_SETINOUTPUTRESOLUTION_T));
		//SetRPCShareMem(SCALERIOC_MEMC_SETINPUTFRAMERATE, sizeof(unsigned char));
		//SetRPCShareMem(SCALERIOC_MEMC_SETOUTPUTFRAMERATE, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_MEMC_SETINOUTPUTFRAMERATE, sizeof(SCALER_MEMC_INOUTFRAMERATE_T));
		SetRPCShareMem(SCALERIOC_MEMC_SETMEMCFRAMEREPEATENABLE, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_MEMC_INSTANBOOT_INITPHASETABLE, sizeof(unsigned char));
		// HDR Dolby Vision
		SetRPCShareMem(SCALERIOC_GET_HDMI_HDR_FRAMEINFO, sizeof(VIDEO_RPC_DOBLBY_VISION_SHM));
		SetRPCShareMem(SCALERIOC_GET_OTT_HDR_FRAMEINFO, sizeof(VIDEO_RPC_DOBLBY_VISION_SHM));
		SetRPCShareMem(SCALERIOC_SEND_VO_OVERSCAN_INFO, sizeof(VIDEO_RPC_VO_OVERSCAN_INFO));
		SetRPCShareMem(SCALERIOC_SMOOTHTOGGLE_SHARE_INFO, sizeof(SMOOTH_TOGGLE_UPDATE_FWINFO_PKG));
		SetRPCShareMem(SCALERIOC_I2RND_SET_BUFFER, sizeof(I2RND_START_ADDR));
		SetRPCShareMem(SCALERIOC_I2RND_SET_APVR_INFO, sizeof(I2RND_APVR_INFO));
		SetRPCShareMem(SCALERIOC_PST_SET_BUFFER, sizeof(PST_START_ADDR));
		SetRPCShareMem(SCALERIOC_I2RND_SEND_TABLE_IDX, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_SMOOTHTOGGLE_SHAREMEMORY_SYNC_FALG, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_SMOOTHTOGGLE_SHIFTXY_SHAREMEMORY_SYNC_FALG, sizeof(unsigned int));
		SetRPCShareMem(VO_RUN_SCALER_FLAG, sizeof(unsigned int));
        SetRPCShareMem(SCALERIOC_DM_CONNECT_FLAG, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_FLAG, sizeof(seamless_change_sync_info));
		SetRPCShareMem(SCALERIOC_VBI_SHAREMEMORY_DATA, sizeof(VBI_UPDATE_DATA_INFO));
		SetRPCShareMem(SCALERIOC_PHOTO_UPDATE_SHAREMEM_FALG, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_PHOTO_SHIFT_SHAREMEM_INFO, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_DUMP_REGISTER_COUNTER, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_DUMP_REGISTER_INFO, sizeof(VIDEO_RPC_DUMP_REGISTER));
		SetRPCShareMem(SCALERIOC_VIP_RPC_system_info_structure, sizeof(_RPC_system_setting_info));
		SetRPCShareMem(SCALERIOC_VIP_RPC_SMARTPIC_CLUS, sizeof(_RPC_clues));
		SetRPCShareMem(SCALERIOC_VIP_RPC_TABLE_STRUCT, sizeof(RPC_SLR_VIP_TABLE));
		SetRPCShareMem(SCALERIOC_DRIVER_CONFIG_INFO, sizeof(DRIVER_CONFIG_DATA_INFO));
		SetRPCShareMem(SCALERIOC_SUB_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_FLAG, sizeof(seamless_change_sync_info));
                //#ifdef CONFIG_SUPPORT_DOLBY_VSIF
		SetRPCShareMem(HDMI_DOLBY_VSIF_FLAG, sizeof(DOLBY_HDMI_VSIF_T));

#ifdef NEW_INPUT_OUTPUT_BUFFER_FLOW_CONFIG
		SetRPCShareMem(SCALERIOC_MAIN_INPUT_OUTPUT_BUFFER_INFO, sizeof(INPUT_OUTPUT_RINGBUFFER_INFO_T));
#ifdef NEW_INPUT_OUTPUT_BUFFER_FLOW_SUPPORT_SUB
		SetRPCShareMem(SCALERIOC_SUB_INPUT_OUTPUT_BUFFER_INFO, sizeof(INPUT_OUTPUT_RINGBUFFER_INFO_T));
#endif
#endif
		//#endif

//#endif
		SetRPCShareMem(SCALERIOC_MULTIBUFFER_NUMBER, sizeof(unsigned int));
		SetRPCShareMem(FILM_MODE_CHANGE_TYPE, sizeof(FILM_SHAREMEM_PARAM));
		SetRPCShareMem(SCALERIOC_WIN_CALLBACK_DELAY_INFO, sizeof(SCALER_WIN_CALLBACK_DELAY_INFO));
		SetRPCShareMem(DYNAMIC_FRAMERATE_CHANGE_INFO, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_VO_FORCE_V_TOP, sizeof(VO_FORCE_V_TOP_DATA));
		SetRPCShareMem(SCALERIOC_SETUP_DI_DONE, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_PC_MODE_INFO, sizeof(pc_mode_sync_info));
		SetRPCShareMem(SCALERIOC_ORBIT_SHIFT_INFO, sizeof(SCALER_ORBIT_SHIFT_INFO));
		SetRPCShareMem(SCALERIOC_BSP_ERROR_REPORT_INFO_BUFFER, sizeof(BSP_ERROR_REPORT_RING_BUFFER_INFO_T));//bsp error ring buffer
		SetRPCShareMem(SCALERIOC_DUAL_CHANNEL_FLAG, sizeof(unsigned char));
		SetRPCShareMem(SCALERIOC_DTATA_FS_ALGO_RESULT, sizeof(SCALER_DATA_FS_ALGO_RESULT));
		SetRPCShareMem(SCALER_SEND_DISP_SIZE_RERUN_VO, sizeof(SEND_DISP_SIZE_INFO));
		SetRPCShareMem(SCALER_AIRMODE_SEND_DISP_SIZE, sizeof(AIRMODE_DISP_SIZE_INFO));
		SetRPCShareMem(SCALERIOC_CINEMA_MODE_ENABLE, sizeof(CINEMA_MODE_INFO));
		SetRPCShareMem(SCALERIOC_VO_TRACK_ENABLE, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_VO_TRACK_I3_ENABLE, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_I3_NN_SEND_CAP_INFO, sizeof(I3DDMA_NN_CAP_INFO_T));
		SetRPCShareMem(SCALERIOC_I3_NN_SHARE_BUF_STATUS, sizeof(I3DDMA_NN_BUFFER_ATTR_T) * NN_CAP_BUFFER_NUM);
		SetRPCShareMem(SCALERIOC_SET_I3DDMA_VODMA_COMP_ATTR, sizeof(I3DDMA_VODMA_COMP_ATTR_T));
		SetRPCShareMem(SCALERIOC_GAMEMODE_DATA, sizeof(GAME_MODE_DATA));
		SetRPCShareMem(SCALERIOC_VO_TRACK_I3_M_STAGE, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_SET_VO_I3_BUF_SW_MODE, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_SET_VO_FORCE_FRAMERATE, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_VO_IVS_SRC_INFO, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_VO_FORCE_60_FRAMERATE_STATE, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_I3_CAPTURE_INDEX, sizeof(unsigned int));
		SetRPCShareMem(SCALERIOC_DOLBY_BUFFER, sizeof(DOLBY_VIDEO_CORRESPOND_T));
		SetRPCShareMem(SCALERIOC_MDMI_QMS_INFO, sizeof(HDMI_QMS_MODE_STATUS));//initial qms share memory
		SetRPCShareMem(SCALERIOC_LOW_POWER_CONTROL_VIDEOFW, sizeof(unsigned int));//for low power
		SetRPCShareMem(SCALERIOC_SOURCE_2P_AND_VRR_INFO, sizeof(VSC_SOURCE_2P_AND_VRR_INFO));

		// SCALERIOC_VIP_PQ_DEVICE
//		SetRPCShareMem(SCALERIOC_VIP_PQ_DEVICE, sizeof(PQ_device_struct));
		// Reset address
		g_ulCachedAddr = 0;


		//allocate virtual memory address@ Crixus 20141009
		g_ulCachedAddr = (unsigned long)dvr_malloc_uncached_specific(g_ulShareMemOffset, GFP_DCU1_LIMIT, (void **)&g_ulNonCachedAddr);
		//get physical address
		//g_ulPhyAddr = (unsigned int)virt_to_phys((void*)g_ulCachedAddr);
		g_ulPhyAddr = (unsigned int)dvr_to_phys((void*)g_ulCachedAddr);

		// allocate mem success
		if(g_ulCachedAddr != 0)
		{
			rtd_pr_vbe_notice("ShareMem alloc success CachedAddr:%lx size:%d\n", g_ulCachedAddr, g_ulShareMemOffset);

			memset((void*)g_ulCachedAddr, 0, g_ulShareMemOffset);

			scalerRPC_isInit = true;


			// Send RPC fail
			if (0 != (lRPCError = SendShareMemAddrByRPC()) )
				return 1;
			else
				return 0;

		}
		// fail
		else
		{
			rtd_pr_vbe_debug("oliver+, ############dvr_malloci alloc memory Fail\n");
			//assert(0);
			return 1;
		}
#else            
			if (scalerRPC_isInit)
				return 0;
		
			scalerRPC_isInit = true;
			g_ulShareMemOffset = 0;
			g_ulNonCachedAddr = 0;
		
			memset(g_atShareMemInfo, 0, sizeof(RPC_SHARE_MEM_INFO)*SCALERIOC_LATESTITEM);
		
			SetRPCShareMem(SCALERIOC_VIP_RPC_system_info_structure, sizeof(_RPC_system_setting_info));
			SetRPCShareMem(SCALERIOC_VIP_system_info_structure, sizeof(_system_setting_info));
			SetRPCShareMem(SCALERIOC_VIP_RPC_SMARTPIC_CLUS, sizeof(_RPC_clues));
			SetRPCShareMem(SCALERIOC_VIP_TABLE_STRUCT,	sizeof(SLR_VIP_TABLE));
			SetRPCShareMem(SCALERIOC_VIP_RPC_TABLE_STRUCT, sizeof(RPC_SLR_VIP_TABLE));
			//SetRPCShareMem(SCALERIOC_VIP_SMARTPIC_CLUS, sizeof(_clues));
			SetRPCShareMem(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT,  sizeof(SLR_VIP_TABLE_CUSTOM_TV001));
			SetRPCShareMem(SCALERIOC_MEMC_INITIALIZATION, sizeof(SCALER_MEMC_DMASTARTADDRESS_T));
			//SetRPCShareMem(SCALERIOC_DCR_MAX_VALUE, sizeof(unsigned int));
			//SetRPCShareMem(SCALERIOC_DCR_TABLE, sizeof(UINT16)*DCR_MAX*DCR_NODE_NUM);
			SetRPCShareMem(SCALERIOC_PC_MODE_INFO, sizeof(pc_mode_sync_info));
			g_quick_showAddr = (unsigned long )dvr_malloc(g_ulShareMemOffset);
			if(g_quick_showAddr ==0){
				printf2(" quick show Scaler_InitRPCShareMem memory alloc fail \n");
			}
				
#endif

}
#ifndef BUILD_QUICK_SHOW

void ResetShareMem(void)
{//This api is used by std reset sharememory
	if(g_ulCachedAddr != 0)
	{
		memset((void*)g_ulCachedAddr, 0, g_ulShareMemOffset);
		dmac_flush_range((const void *)g_ulCachedAddr, (const void *)(g_ulCachedAddr + g_ulShareMemOffset));//flush. Let cache to memory
		rtd_pr_vbe_notice("\r\n ResetShareMem success startaddr:%lx size:%d \r\n", g_ulCachedAddr, g_ulShareMemOffset);
	}
}

void Scaler_FreeRPCShareMem(void)
{
	if (g_ulCachedAddr != 0)
	{
		rtd_pr_vbe_debug("[STD]=%s=%d=\n", __FUNCTION__, __LINE__);
		dvr_free((void *)g_ulCachedAddr);
		g_ulCachedAddr = 0;

		g_ulShareMemOffset = 0;
	}
}

#endif

unsigned int Scaler_ChangeUINT32Endian(unsigned int a_ulLittleIndian)
{
    unsigned int ulBigIndian = 0;
    unsigned char *pucBigIndian = (unsigned char *)(&ulBigIndian);
    unsigned char *pucLittleIndian = (unsigned char *)(&a_ulLittleIndian);

    pucBigIndian[0] = pucLittleIndian[3];
    pucBigIndian[1] = pucLittleIndian[2];
    pucBigIndian[2] = pucLittleIndian[1];
    pucBigIndian[3] = pucLittleIndian[0];

    return ulBigIndian;

}


UINT16 Scaler_ChangeUINT16Endian(UINT16 a_usLittleEndian)
{
    volatile UINT16 usBigEndian = 0;
    unsigned char *pucBigEndian = (unsigned char *)(&usBigEndian);
    unsigned char *pucLittleEndian = (unsigned char *)(&a_usLittleEndian);

    pucBigEndian[0] = pucLittleEndian[1];
    pucBigEndian[1] = pucLittleEndian[0];

   //rtd_pr_vbe_info("usBigEndian=%x, pucBigEndian[0]=%x, pucBigEndian[1]=%x, pucLittleEndian[0]=%x, pucLittleEndian[1]=%x\n", usBigEndian, pucBigEndian[0], pucBigEndian[1], pucLittleEndian[0], pucLittleEndian[1]);

    return usBigEndian;

}
unsigned long long Scaler_ChangeUINT64Endian(unsigned long long a_ulLittleIndian)
{
	unsigned long long ulBigIndian = 0;
	unsigned char *pucBigIndian = (unsigned char *)(&ulBigIndian);
	unsigned char *pucLittleIndian = (unsigned char *)(&a_ulLittleIndian);

	pucBigIndian[0] = pucLittleIndian[7];
	pucBigIndian[1] = pucLittleIndian[6];
	pucBigIndian[2] = pucLittleIndian[5];
	pucBigIndian[3] = pucLittleIndian[4];
	pucBigIndian[4] = pucLittleIndian[3];
	pucBigIndian[5] = pucLittleIndian[2];
	pucBigIndian[6] = pucLittleIndian[1];
	pucBigIndian[7] = pucLittleIndian[0];

	return ulBigIndian;

}
#ifndef BUILD_QUICK_SHOW

SCALER_WIN_CALLBACK_DELAY_INFO *pWinCallbackDelayShareMemInfo = NULL;
SCALER_WIN_CALLBACK_DELAY_INFO winCallbackDelayInfo;

SCALER_WIN_CALLBACK_DELAY_INFO* Scaler_Get_Pst_Window_Delay_Setting(void)
{
	unsigned int *pulTemp = NULL;
	unsigned int i = 0, ulItemCount = 0;

	pWinCallbackDelayShareMemInfo = (SCALER_WIN_CALLBACK_DELAY_INFO *)Scaler_GetShareMemVirAddr(SCALERIOC_WIN_CALLBACK_DELAY_INFO);

	if(pWinCallbackDelayShareMemInfo){

		pWinCallbackDelayShareMemInfo = &winCallbackDelayInfo;

		memcpy((void*) pWinCallbackDelayShareMemInfo, (void*) Scaler_GetShareMemVirAddr(SCALERIOC_WIN_CALLBACK_DELAY_INFO), sizeof(SCALER_WIN_CALLBACK_DELAY_INFO));

		//rtd_pr_vbe_emerg("[Share_Get_Pst_Window_Delay_Setting] (%x.%x.%x.%x.%x)\n", winCallbackDelayInfo.uzulineCount,
			//winCallbackDelayInfo.region_x,winCallbackDelayInfo.region_y, winCallbackDelayInfo.region_w, winCallbackDelayInfo.region_h);

		ulItemCount = sizeof(SCALER_WIN_CALLBACK_DELAY_INFO) /  sizeof(UINT32);
		pulTemp = (unsigned int *)pWinCallbackDelayShareMemInfo;
		for (i = 0; i < ulItemCount; i++){
			pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);
		}

		//rtd_pr_vbe_emerg("[Share_Get_Pst_Window_Delay_Setting] (%x.%x.%x.%x.%x)\n", winCallbackDelayInfo.uzulineCount,
			//winCallbackDelayInfo.region_x,winCallbackDelayInfo.region_y, winCallbackDelayInfo.region_w, winCallbackDelayInfo.region_h);

	}
	return pWinCallbackDelayShareMemInfo;
}

void Scaler_Reset_Pst_Window_Delay_Setting(void)
{
	SCALER_WIN_CALLBACK_DELAY_INFO *pWinCallbackDelayShareMemInfo = NULL;

	pWinCallbackDelayShareMemInfo = (SCALER_WIN_CALLBACK_DELAY_INFO *)Scaler_GetShareMemVirAddr(SCALERIOC_WIN_CALLBACK_DELAY_INFO);

	if(pWinCallbackDelayShareMemInfo){
		memset((void*) pWinCallbackDelayShareMemInfo, 0, sizeof(SCALER_WIN_CALLBACK_DELAY_INFO));

		//rtd_pr_vbe_emerg("[Scaler_Reset_Pst_Window_Delay_Setting] (%x.%x.%x.%x.%x)\n", pWinCallbackDelayShareMemInfo->validinfo,
			//	pWinCallbackDelayShareMemInfo->region_x,pWinCallbackDelayShareMemInfo->region_y,
			//	pWinCallbackDelayShareMemInfo->region_w, pWinCallbackDelayShareMemInfo->region_h);
	}

}

UINT32 Scaler_Pst_Callback_Check_Vaildinfo(void)
{
	SCALER_WIN_CALLBACK_DELAY_INFO* pPstDelaySetting = NULL;

	pPstDelaySetting = Scaler_Get_Pst_Window_Delay_Setting();

	if((pPstDelaySetting)&&(pPstDelaySetting->validinfo)){
		return 1;
	}else
		return 0;
}
#endif


unsigned char modestate_I3DDMA_get_In3dMode(void)
{
	unsigned char result=FALSE;

	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() == FALSE)
		return FALSE;

#ifdef ENABLE_DRIVER_I3DDMA
	result = (Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_DispGetInputInfo(SLR_INPUT_THROUGH_I3DDMA) == TRUE)
			&& (Scaler_Get_CurVoInfo_plane() == VO_VIDEO_PLANE_V1);
#endif

	return result;
}

unsigned char modestate_decide_double_dvs_enable(void)
{
	unsigned char result = _DISABLE;

//#ifdef VBY_ONE_PANEL
//	return result;
//#endif
#if 0
	if(Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI)
		return result;
#endif
	if(Get_DISPLAY_PANEL_TYPE() == P_DISP_TO_TVOX)
		return result;

//#ifdef VBY_ONE_PANEL
//	if((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) > 1920) && (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > 1080) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 160))
//		return result;
//#else
	// [4k2k] no double DVS in 4k2k output timing
	//if((_DISP_WID > 1920) && (_DISP_LEN > 1080) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 160))
		//return result;
//#endif

	if(modestate_check_input_special_framerate()){
		return result;
	}
#ifndef BUILD_QUICK_SHOW    
	// 2D MEMC enable mode, force to timing frame sync (via double DVS enable)
	if((scalerdisplay_get_2D_MEMC_support() == TRUE) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 310) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 230)){
		if((drvif_scaelr3d_decide_is_3D_display_mode() == FALSE) && (Scaler_DispGetRatioMode() != SLR_RATIO_CUSTOM))
			return TRUE;
	}

	// [SG] VO source data frame sync when VO output frame rate is 120Hz
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_SG_OUTPUT_120HZ_ENABLE()){
		if((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) == (VO_FR_CTRL_FORCE_FRAMERATE_MAX/100))){
			return FALSE;
		}
	}

	// [SG][3D] 3D display force in free run@120Hz
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && !Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode())
	{
		return FALSE;
	}

	// [PR to SG] for PR -> SG 3D TCON, 3D need timing free run
	if(Get_PANEL_3D_PR_TO_SG_OUTPUT_FRC_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode()){
		//rtd_pr_vbe_info("[DBG] 3D IGNORE 2x DVS@%d\n", __LINE__);
		return FALSE;
	}
#endif
#if 0
	if(Get_PANEL_3D_SG_OUTPUT_120HZ_ENABLE()){
		if(Get_PANEL_3D_SG_OUTPUT_120HZ_ENABLE() && (scalerdisplay_get_2D_MEMC_support() == FALSE)/*drvif_scaler_decide_2D_freerun_120hz()*/
			&& (Scaler_InputSrcGetMainChType() != _SRC_VO) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 310)){
			rtd_pr_vbe_info("[SG][2D] DoubleDVS=1@%d\n", __LINE__);
			return _ENABLE;
		}/*else if(drvif_scaler3D_get_mdomain_force_frameSync_status()){
			rtd_pr_vbe_info("[SG][2D] DoubleDVS=0@%d\n", __LINE__);
			return _DISABLE;
		}*/
	}
#endif

	if (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)) // sub display
		return _DISABLE;
#ifndef BUILD_QUICK_SHOW

	if(Get_PANEL_TYPE_IS_MILLET3_4K2K_SERIES() == 1){
		if((Get_DISPLAY_2K1K_windows_mode() ==0) && (Scaler_DispGetInputInfo(SLR_INPUT_THROUGH_I3DDMA) == FALSE)){
			if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 610) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 1000) && (Scaler_InputSrcGetMainChType()!=_SRC_TV)){
				return _DISABLE;
			}else{
				rtd_pr_vbe_info("enable double dvs! \n");
				return _ENABLE;
			}
		}
	}
	result = _DISABLE;
#endif
	switch(Scaler_InputSrcGetType((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)))
	{
		case _SRC_YPBPR:
			if((Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR) >= _MODE_1080P23)
				&& (Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR) <= _MODE_1080P30))
				result = _ENABLE;
		break;

		case _SRC_HDMI:
			if(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 310)
			{
				//MODESTATE_PRINTF("HDMI Enter GIBI\n");

				//if((_DISP_WID > 1920) && (_DISP_LEN > 1080))
					//result = _DISABLE;
				//else
					result = _ENABLE;

//#ifdef VBY_ONE_PANEL
//				result = _ENABLE;
//#endif
			}
		break;

		case _SRC_VO:
			//frank@0412 add below to do the double DVS flow
			if(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 310)
				result = _ENABLE;
		break;

		default:
			result = _DISABLE;
		break;
	}

	return result;
}

UINT8 modestate_check_input_special_framerate(void)
{
	unsigned char result = TRUE;
/*
	if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 235) || ((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 255)
		&& (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 290))){
		return TRUE;
*/
	if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 255)){
		result = FALSE;
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305)){
		result = FALSE;
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 475) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 485)){
		result = FALSE;
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 505)){
		result = FALSE;
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 595) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 605)){
		result = FALSE;
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 994) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 1005)){
		if(Get_DISPLAY_REFRESH_RATE() >= 120)
			result = FALSE;
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 1191) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 1206)){
		if(Get_DISPLAY_REFRESH_RATE() >= 120)
			result = FALSE;
	}
    else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 1435) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 1445)){
        if(Get_DISPLAY_REFRESH_RATE() >= 144)
            result = FALSE;
    }
    else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 1640) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 1655)){
        if(Get_DISPLAY_REFRESH_RATE() >= 120)
            result = FALSE;
	}
	else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 2390) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 2410)){
		if(Get_DISPLAY_REFRESH_RATE() >= 120)
			result = FALSE;
	}
    else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 2878) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 2898)){
        if(Get_DISPLAY_REFRESH_RATE() >= 120)
            result = FALSE;
	}
	/*
	if(result == TRUE)
		rtd_pr_vbe_notice("!!!!!!!!!! input_special_framerate (%d)!!!!!!!\n", Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) );
	*/
	return result;
}
#ifndef BUILD_QUICK_SHOW

unsigned char modestate_I3DDMA_check_idma_mode_enable(unsigned char cur3dType)
{
	unsigned char result=FALSE;

#ifdef ENABLE_DRIVER_I3DDMA
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() == FALSE)
	{
		return FALSE;
	}

	// PR 3D enable IDMA only for new 3D format
	if(Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()){
  #ifdef ENABLE_USE_VO_UZU // [VO UZU] VO in VO1 for UZU funciton (bypass IDMA)
		if((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_Get_forceEnable_VO_UZU())){
			result = FALSE;
		}else
  #endif
		if(((cur3dType >= SLR_3D_VERTICAL_STRIP) && (cur3dType <= SLR_3D_SENSIO))|| ((cur3dType >= SLR_3D_VERTICAL_STRIP_CVT_2D) && (cur3dType <= SLR_3D_SENSIO_CVT_2D))){
			result = TRUE; // new 3D mode
		}else if((Scaler_InputSrcGetMainChType() == _SRC_HDMI) && (cur3dType == SLR_3D_FRAME_PACKING) /*&& Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_INTERLACE)*/
			&& (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) == 2228) ){
			result = TRUE; // HDMI 1080i FP 3D mode
		}
	}else{ // SG 3D
		if((cur3dType != SLR_3D_2D_ONLY) && (cur3dType != SLR_3D_2D_CVT_3D) && (cur3dType != SLR_3D_FRAMESEQUENCE) && (cur3dType != SLR_3D_L_DEPTH ) && (cur3dType != SLR_3D_L_DEPTH_GPX )){
	#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
			unsigned char srcType = 0;
			srcType = modestate_I3DDMA_get_2dSrcType();
			UINT16 usWidth, usLength;
			usWidth = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
			usLength = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
	  #ifdef ENABLE_USE_VO_UZU // [VO UZU] VO in VO1 for UZU funciton (bypass IDMA)
			if((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_Get_forceEnable_VO_UZU())){
				result = FALSE;
			}else
	  #endif
			if((cur3dType == SLR_3D_SIDE_BY_SIDE_HALF)|| (cur3dType == SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D)
				|| (cur3dType == SLR_3D_TOP_AND_BOTTOM)|| (cur3dType == SLR_3D_TOP_AND_BOTTOM_CVT_2D)
				|| ((Scaler_InputSrcGetMainChType() == _SRC_HDMI) && ((cur3dType == SLR_3D_FRAME_PACKING)|| (cur3dType == SLR_3D_FRAME_PACKING_CVT_2D))
					&& ((drvif_scaler3d_decide_HDMI_framePacking_interlaced_status() == FALSE) || drvif_scaler3d_decide_3d_SG_Disable_IDMA_suport_FP_interlace())))
			{
				// SBS/TaB 3D mode don't need enable IDMA
				result = FALSE;
			}else if((srcType != _SRC_VO) && (srcType != _SRC_HDMI) && (srcType != _SRC_YPBPR)){
				// IDMA 3D mode support for VO/HDMI/YPbPr source in current status
				result = FALSE;
			}else if((usWidth > 1920) && (usLength > 1080)){
				// 4k2k video bypass IDMA
				result = FALSE;
			}else
	#endif
			result = TRUE;
	}
	}
#endif

	return result;
}

unsigned char modestate_get_vo3d_in_framePacking_FRC_mode(void)
{
	unsigned char result=FALSE;

	unsigned int src_org;
	SLR_VOINFO* pVOInfo;


#ifdef ENABLE_DRIVER_I3DDMA
	if(modestate_I3DDMA_get_In3dMode())
		src_org = drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE);
	else
#endif
		src_org = Scaler_InputSrcGetMainChType();

	if(src_org != _SRC_VO)
		return FALSE;

	if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == TRUE)
		return FALSE;

	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	if(pVOInfo)
		result = ((pVOInfo->mode_3d == VO_3D_FRAME_PACKING) && (pVOInfo->force2d == 0)? TRUE: FALSE);

	return result;


}


unsigned char modestate_get_pr3d_in_fp_3d_mode(void)
{
	unsigned char result=FALSE;

	// [PR] FP 3D IVS is 2x input frame rate when enable VGIP frame packing 3D mode
	if(Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode()
		/*&& (Scaler_InputSrcGetMainChType() == _SRC_HDMI)*/ && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING))
	{
		result = TRUE;
	}

	return result;
}

#if 1//qiang_zhou moved from scalerlib.c
//static SLR_3D_MODE cur3DMode = SLR_3DMODE_2D;
#ifndef CONFIG_DUAL_CHANNEL
static SLR_3D_MODE cur3DMode[MAX_DISP_CHANNEL_NUM] = {SLR_3DMODE_2D};
#else
static SLR_3D_MODE cur3DMode[MAX_DISP_CHANNEL_NUM] = {SLR_3DMODE_2D, SLR_3DMODE_2D};
#endif

unsigned char Scaler_Get3DMode(SCALER_DISP_CHANNEL display)
{
#ifdef CONFIG_DUAL_CHANNEL
	if ( display > SLR_SUB_DISPLAY)
	{
		rtd_pr_vbe_debug("[disp_info] Invalid channel\n");
		return 0;
	}
#else //#ifdef CONFIG_DUAL_CHANNEL
	display = SLR_MAIN_DISPLAY;
#endif //#ifdef CONFIG_DUAL_CHANNEL

	return cur3DMode[display];
}

void Scaler_Set3DMode_Attr(SCALER_DISP_CHANNEL display, unsigned char message)
{
#ifdef CONFIG_DUAL_CHANNEL
	if ( display > SLR_SUB_DISPLAY)
	{
		rtd_pr_vbe_debug("[disp_info] Invalid channel\n");
		return;
	}
#else //#ifdef CONFIG_DUAL_CHANNEL
	display = SLR_MAIN_DISPLAY;
#endif //#ifdef CONFIG_DUAL_CHANNEL

	if (cur3DMode[display] != (SLR_3D_MODE)message) {
#ifdef ENABLE_SPEED_UP_SLR_DISPLAY_ON_FLOW
		pre3DMode = cur3DMode[display];
#endif //#ifdef ENABLE_SPEED_UP_SLR_DISPLAY_ON_FLOW
		cur3DMode[display] = (SLR_3D_MODE)message;
	}
}

/*
unsigned char Scaler_Get3DMode(void)
{
	return cur3DMode;
}

void Scaler_Set3DMode_Attr(unsigned char message)
{
	if (cur3DMode != (SLR_3D_MODE)message) {
#ifdef ENABLE_SPEED_UP_SLR_DISPLAY_ON_FLOW
		pre3DMode = cur3DMode;
#endif
	cur3DMode = (SLR_3D_MODE)message;
}
}
*/

static void WaitDisplayReady(void)
{

}

static UINT8 ForceSupport_3D = 0;
void Scaler_ForceSupport3D(unsigned char value)
{
	ForceSupport_3D = value;
}

static unsigned char Scaler_GetForceSupport3D(void)
{
	return ForceSupport_3D;
}

INT8 Scaler_Get3D_IsSupport3D(unsigned char is2Dcvt3D, unsigned char uc3DType)
{
	unsigned char bSupport3D = FALSE;
	//unsigned int curMode = 0;
	unsigned char uc3DFmt = 0;
	unsigned char srcType = 0;
	unsigned char bInterlaced = FALSE;
	UINT16 usWidth = 0;
	UINT16 usLength = 0;

	rtd_pr_vbe_info("\n [%s %d] uc3DType = %d\n",__FUNCTION__,__LINE__,uc3DType);
	if (Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
	{
		if(Scaler_DispGetInputInfo(SLR_INPUT_THROUGH_I3DDMA) == TRUE && uc3DType == HDMI3D_FRAME_PACKING)
			return TRUE;

		if(get_field_alternative_3d_mode_enable())
			return TRUE;

		if((is2Dcvt3D == FALSE) && (uc3DType > HDMI3D_SENSIO)){
			// 2D or 3Dto2D always support
			return TRUE;
		}

		if (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE())
			uc3DFmt = 2;
		else
			uc3DFmt = 1;

		usWidth = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
		usLength = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);

		// calculate the real picture size
		if(modestate_I3DDMA_get_In3dMode() && (Scaler_DispGetInputInfoByDisp(_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)){
			unsigned char cur3dType;
			cur3dType = Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE);
			if((cur3dType == SLR_3D_SIDE_BY_SIDE_HALF)|| (cur3dType == SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D))
				usWidth <<= 1; // SBS 3D: width *= 2
			else if((cur3dType == SLR_3D_TOP_AND_BOTTOM)|| (cur3dType == SLR_3D_TOP_AND_BOTTOM_CVT_2D))
				usLength <<= 1; // TaB 3D: Len *=2

			rtd_pr_vbe_info("[DBG] curType/wid/len=%d/%d/%d, nextType=%d\n", cur3dType, usWidth, usLength, uc3DType);
		}

		// 4k2k video
		if((usWidth > 1920) && (usLength > 1080)){
#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE
			if((uc3DType == HDMI3D_SIDE_BY_SIDE_HALF)|| (uc3DType == HDMI3D_TOP_AND_BOTTOM)){
				bSupport3D = TRUE;
			}else
#endif
		{
			rtd_pr_vbe_info("[%s:%d] 4k2k (h/v=%d/%d), not support 3D\n", __FILE__, __LINE__, usWidth, usLength);
			bSupport3D = FALSE;
		}
		} // full SBS 3D mode
		else if((usWidth > 1920) && ( is2Dcvt3D || ((uc3DType != HDMI3D_SIDE_BY_SIDE_HALF) && (uc3DType != HDMI3D_SIDE_BY_SIDE_FULL)))){
			// 4k2k video support SBS/TaB 3D mode only
			if(uc3DType == HDMI3D_TOP_AND_BOTTOM){
				bSupport3D = TRUE;
			}else{
				rtd_pr_vbe_info("[%s:%d] 4k2k SBS full (h/v=%d/%d), not support 3D[%d, %d]\n", __FILE__, __LINE__, usWidth, usLength, is2Dcvt3D, uc3DType);
				bSupport3D = TRUE;
			}
		} // full TaB 3D mode
		else if((usLength > 1080) && (is2Dcvt3D || ((uc3DType != HDMI3D_TOP_AND_BOTTOM) && (uc3DType != HDMI3D_FRAME_PACKING)))){
			// 4k2k video support SBS/TaB 3D mode only
			if((uc3DType == HDMI3D_SIDE_BY_SIDE_HALF) || (uc3DType == HDMI3D_SIDE_BY_SIDE_FULL)){
				bSupport3D = TRUE;
			}else{
				rtd_pr_vbe_info("[%s:%d] 4k2k TaB full (h/v=%d/%d), not support 3D[%d, %d]\n", __FILE__, __LINE__, usWidth, usLength, is2Dcvt3D, uc3DType);
				bSupport3D = TRUE;
			}
		}
		else if (is2Dcvt3D == FALSE){ // 3D
				//curMode = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_MODE_CURR);
				bInterlaced = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE);

			if (uc3DFmt == 1){ // SG 3D
				srcType = Scaler_InputSrcGetMainChType();
	#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
				bSupport3D = TRUE;
	#else
				// 3D mode support for VO/HDMI/YPbPr source in current status
				if((srcType == _SRC_VO)|| (srcType == _SRC_HDMI)|| (srcType == _SRC_YPBPR))
						bSupport3D = TRUE;
	#endif
			}
			else if (uc3DFmt == 2){ // 3D PR type cannot support vertical active line is less than 540
				if(Scaler_GetForceSupport3D())//force support 3D
					return TRUE;

				if (uc3DType == HDMI3D_SIDE_BY_SIDE_HALF){
						bSupport3D = TRUE;
				}
				else if (uc3DType == HDMI3D_TOP_AND_BOTTOM){
						bSupport3D = TRUE;
					}
				else if (uc3DType == HDMI3D_FRAME_PACKING)
				{
						srcType = Scaler_InputSrcGetMainChType();
						usWidth = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
						usLength = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);

					 // Frame Packing From HDMI 1080p or 720p
					if (srcType == _SRC_HDMI){
						if ((bInterlaced == FALSE) && ((usWidth == 1920 && usLength == 2205) || (usWidth == 1280 && usLength == 1470))) // 1080p or 720p frame packing
							bSupport3D = TRUE;
						else if (usWidth == 1920 && usLength == 2228) // 1080i frame packing (Input format is progressive)
							bSupport3D = TRUE;
						else{
							bSupport3D = FALSE;
							rtd_pr_vbe_info("[FP] I/H/V=%d/%d/%d\n", bInterlaced, usWidth, usLength);
					}
					}
					else if(modestate_I3DDMA_get_In3dMode()){
						bSupport3D = TRUE;
					}else{
						bSupport3D = FALSE;
					}
				}
				else if(modestate_I3DDMA_check_idma_mode_enable(uc3DType) == TRUE){
					bSupport3D = TRUE;
				}
			}
		}
		else{ // 2Dcvt3D
			if (uc3DFmt == 1){ // SG 3D 2Dcvt3D
					bSupport3D = TRUE;
			}
			else if (uc3DFmt == 2){
							bSupport3D = TRUE;
			}
		}
	}

	// print debug message
	rtd_pr_vbe_info("EEE[3D]IsSupport[src/2d3d/3dtype][%d/%d/%d]=%d\n", srcType, is2Dcvt3D, uc3DType, bSupport3D);

	return bSupport3D;
}

unsigned char Scaler_Set3DMode(SCALER_DISP_CHANNEL display, unsigned char message, bool muteAudio)
{
	unsigned char isHandled = FALSE;
	//if (!(Scaler_InputSrcGetMainChType()== _SRC_HDMI && (!drvif_IsHDMI())))
	//	pthread_mutex_lock(&scalerMutex);


	rtd_pr_vbe_info("[3D]%s(%d)\n", __FUNCTION__, message);

	if (FALSE == Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
	{
		//if (!(Scaler_InputSrcGetMainChType()== _SRC_HDMI && (!drvif_IsHDMI())))
		//	pthread_mutex_unlock(&scalerMutex);

		//SetAudioMuteCtrl(AUDIO_MUTE_ID_SYSTEM,false);

		return isHandled; // 0:Not support 1:Support
	}

#ifdef USE_3D_AUTO_DETECT//JunnyModify20130726
	if((message == SLR_3DMODE_3D_AUTO) && (Scaler_Get3DMode(display) != SLR_3DMODE_3D_AUTO_CVT_2D)){
		; // don't forceBG if going to enable VO auto 3D detect mode
	}else
#endif
#if 0
	{
		if(message != Scaler_Get3DMode()){
			// enable forceBG when change 3D mode
			Scaler_ForceDisableBg(FALSE);
		}
		Scaler_ForceBg(SLR_MAIN_DISPLAY, SLR_FORCE_BG_SYSTEM, true);
	}

#endif

	//fw_flow_Set3DModeReadyFlag(FALSE);

	// AP set this flag as false, then scaler will set it as true after change to specific 3D (or 2D) mode.
	//Scaler_Set3DMode_ScalerHasProcessMsgFlag(FALSE);

	// mute audio before change 3D mode
	if( (Scaler_InputSrcGetType(display) != _SRC_VO) && (Scaler_Get3DMode(display) != message) && (muteAudio == true)) {
#ifdef USE_3D_AUTO_DETECT//JunnyModify20130726
		if((message == SLR_3DMODE_3D_AUTO) && (Scaler_Get3DMode(display) != SLR_3DMODE_3D_AUTO_CVT_2D)){
			; // don't mute audio if going to enable VO auto 3D detect mode
		}else
#endif
		//SetAudioMuteCtrl(AUDIO_MUTE_ID_SYSTEM,true);
	} // mute audio before exit 3D mode in external input source

	switch(message)
	{
		// 3D->2D
		case SLR_3DMODE_2D:
			WaitDisplayReady();

			//fw_flow_SetAPDispReadyFlag(FALSE);
			// Disable 3D->2D finishflag
			//Scaler_Set3D2DFinishFlag(FALSE);

#ifdef USE_3D_AUTO_DETECT//JunnyModify20130726
			if(Scaler_Get3D_VoAutoDetectMode())
				Scaler_Set3D_VoAutoDetect_Enable(FALSE);
#endif
			//flow_message_push(_UI2FL_3D_FORCE_2D_MODE_EVENT);
			Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_2D_ONLY);
			Scaler_Set3DMode_Attr(display, SLR_3DMODE_2D);
			Scaler_ForceSupport3D(FALSE);
			isHandled = TRUE;
		break;

		case SLR_3DMODE_3D_AUTO:
#ifdef USE_3D_AUTO_DETECT//JunnyModify20130726
			if(Scaler_Get3DMode(display) != SLR_3DMODE_3D_AUTO_CVT_2D){
				Scaler_Set3D_VoAutoDetect_Enable(TRUE);
				isHandled = TRUE;
			}else
#endif
			if(Scaler_Get3DMode(display) == SLR_3DMODE_3D_AUTO_CVT_2D){
				//flow_message_push(_UI2FL_3D_INPUT_AUTO_FROM_INFO_FRAME_FLAG_CHANGE);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_AUTO);
				isHandled = TRUE;
			}
		break;

		case SLR_3DMODE_3D_SBS:
			if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_SIDE_BY_SIDE_HALF) == TRUE)
			{
				//flow_message_push(_UI2FL_3D_INPUT_SIDE_BY_SIDE_HALF_FLAG_CHANGE);
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_SIDE_BY_SIDE_HALF);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_SBS);
				isHandled = TRUE;
			}
		break;

		case SLR_3DMODE_3D_TB:
			if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_TOP_AND_BOTTOM) == TRUE)
			{
				//flow_message_push(_UI2FL_3D_INPUT_TOP_AND_BOTTOM_FLAG_CHANGE);
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_TB);
				isHandled = TRUE;
			}
		break;

		case SLR_3DMODE_3D_FP:
			if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_FRAME_PACKING) == TRUE)
			{
				if(Scaler_InputSrcGetType(display) == _SRC_VO){
					int result;
					// Frame sequence 3D
					result = drvif_scaler3d_init_vo_3d_mode(SLR_3DMODE_3D_FP);
					if(result != 0)
						rtd_pr_vbe_info("[SLR] ERR: Set init_vo_3d_mode[FP_3D] FAIL!!\n");
					else{
						rtd_pr_vbe_info("[AUTO_3D] SetVoAuto3DDetect DONE!!\n");
					isHandled = TRUE;
					}
				}else{
					//flow_message_push(_UI2FL_3D_INPUT_FRAME_PACKING_FLAG_CHANGE);
					Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_FRAME_PACKING);
					Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_FP);
					isHandled = TRUE;
				}
			}
		break;

		case SLR_3DMODE_3D_LBL:
			if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_LINE_ALTERNATIVE) == TRUE)
			{
				//flow_message_push(_UI2FL_3D_INPUT_LINE_BY_LINE_FLAG_CHANGE);
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_LINE_ALTERNATIVE);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_LBL);
				isHandled = TRUE;
			}
		break;

		case SLR_3DMODE_3D_VSTRIP:
			if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_VERTICAL_STRIPE) == TRUE)
			{
				//flow_message_push(_UI2FL_3D_INPUT_VERTICAL_STRIPE_FLAG_CHANGE);
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_VERTICAL_STRIP);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_VSTRIP);
				isHandled = TRUE;
			}
		break;

		case SLR_3DMODE_3D_CKB:
			if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_CHECKER_BOARD) == TRUE)
			{
				//flow_message_push(_UI2FL_3D_INPUT_CHECKER_BOARD_FLAG_CHANGE);
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_CHECKER_BOARD);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_CKB);
				isHandled = TRUE;
			}
		break;

		case SLR_3DMODE_3D_REALID:
			if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_REALID) == TRUE)
			{
				//flow_message_push(_UI2FL_3D_INPUT_REALID_FLAG_CHANGE);
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_REALID);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_REALID);
				isHandled = TRUE;
			}
		break;

		case SLR_3DMODE_3D_SENSIO:
			if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_SENSIO) == TRUE)
			{
				//flow_message_push(_UI2FL_3D_INPUT_SENSIO_FLAG_CHANGE);
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_SENSIO);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_SENSIO);
				isHandled = TRUE;
			}
		break;

		case SLR_3DMODE_2D_CVT_3D:
			// 2D->3D
			if (Scaler_Get3D_IsSupport3D(TRUE, 0) == TRUE)
			{
				WaitDisplayReady();
				//fw_flow_SetAPDispReadyFlag(FALSE);
				//flow_message_push(_UI2FL_3D_INPUT_2D_CVT_3D);
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_2D_CVT_3D);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_2D_CVT_3D);
				isHandled = TRUE;
			}
		break;

		case SLR_3DMODE_3D_AUTO_CVT_2D:
			switch(Scaler_Get3DMode(display))
			{
				case SLR_3DMODE_3D_AUTO: // when AP's 3D type setting is auto mode
//					if (GET_IS3DFMT() == 0) // if scaler is still in 2D mode, we use auto detect 3d format to decide
//					{
#if defined(USE_3D_AUTO_DETECT) && defined(ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE) // VO auto 3D to 3Dto2D mode format change
					UINT8 cur3DType = Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE);
					if((Scaler_InputSrcGetType(display) == _SRC_VO) && ((drvif_scaler3d_decide_3d_SG_Disable_IDMA(display, cur3DType) == TRUE))){
						// VO auto 3D change to 3Dto2D mode (disable VO auto detect)
						if (cur3DType == SLR_3D_SIDE_BY_SIDE_HALF){
							//flow_message_push(_UI2FL_3D_INPUT_SIDE_BY_SIDE_HALF_CVT_2D);
							Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D);
							Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_SBS_CVT_2D);
						}else if (cur3DType == SLR_3D_TOP_AND_BOTTOM){
							//flow_message_push(_UI2FL_3D_INPUT_TOP_AND_BOTTOM_CVT_2D);
							Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_TOP_AND_BOTTOM_CVT_2D);
							Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_TB_CVT_2D);
						}else{
							//flow_message_push(_UI2FL_3D_INPUT_AUTO_FROM_INFO_FRAME_CVT_2D);
							Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_AUTO_CVT_2D);
						}
						//drvif_scaler3d_init_vo_3d_mode(display, SLR_3DMODE_2D);
						Scaler_Set3D_VoAutoDetect_UserNewMode((SLR_3D_MODE)Scaler_Get3DMode(display));
						rtd_pr_vbe_info("[AUTO_3D] change to 3Dto2D mode[%d]\n", Scaler_Get3DMode(display));
					}else
#endif
					{
						//flow_message_push(_UI2FL_3D_INPUT_AUTO_FROM_INFO_FRAME_CVT_2D);
						Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_AUTO_CVT_2D);
					}
//					}
				break;

				case SLR_3DMODE_3D_SBS: // when AP's 3D type setting is SBS, we use SBS to transfer
					//flow_message_push(_UI2FL_3D_INPUT_SIDE_BY_SIDE_HALF_CVT_2D);
					Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D);
					Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_SBS_CVT_2D);
				break;

				case SLR_3DMODE_3D_TB: // when AP's 3D type setting is TB, we use TB to transfer
					//flow_message_push(_UI2FL_3D_INPUT_TOP_AND_BOTTOM_CVT_2D);
					Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM_CVT_2D);
					Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_TB_CVT_2D);
				break;

				case SLR_3DMODE_3D_FP: // when AP's 3D type setting is FP, we use FP to transfer
					//flow_message_push(_UI2FL_3D_INPUT_FRAME_PACKING_CVT_2D);
					Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_FRAME_PACKING_CVT_2D);
					Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_FP_CVT_2D);
				break;

				default:
					//flow_message_push(_UI2FL_3D_INPUT_AUTO_FROM_INFO_FRAME_CVT_2D);
					Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_AUTO_CVT_2D);
				break;
			}

			isHandled = TRUE;
		break;

		case SLR_3DMODE_3D_SBS_CVT_2D:
			//flow_message_push(_UI2FL_3D_INPUT_SIDE_BY_SIDE_HALF_CVT_2D);
			Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D);
			Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_SBS_CVT_2D);
			isHandled = TRUE;
		break;

		case SLR_3DMODE_3D_TB_CVT_2D:
			//flow_message_push(_UI2FL_3D_INPUT_TOP_AND_BOTTOM_CVT_2D);
			Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_TOP_AND_BOTTOM_CVT_2D);
			Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_TB_CVT_2D);
			isHandled = TRUE;
		break;

		case SLR_3DMODE_3D_FP_CVT_2D:
			//flow_message_push(_UI2FL_3D_INPUT_FRAME_PACKING_CVT_2D);
			Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_FRAME_PACKING_CVT_2D);
			Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_FP_CVT_2D);
			isHandled = TRUE;
		break;

		case SLR_3DMODE_3D_LBL_CVT_2D:
			//flow_message_push(_UI2FL_3D_INPUT_LINE_BY_LINE_CVT_2D);
			Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_LINE_ALTERNATIVE_CVT_2D);
			Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_LBL_CVT_2D);
			isHandled = TRUE;
		break;

		case SLR_3DMODE_3D_VSTRIP_CVT_2D:
			//flow_message_push(_UI2FL_3D_INPUT_VERTICAL_STRIPE_CVT_2D);
			Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_VERTICAL_STRIP_CVT_2D);
			Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_VSTRIP_CVT_2D);
			isHandled = TRUE;
		break;

		case SLR_3DMODE_3D_CKB_CVT_2D:
			//flow_message_push(_UI2FL_3D_INPUT_CHECKER_BOARD_CVT_2D);
			Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_VERTICAL_STRIP_CVT_2D);
			Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_CKB_CVT_2D);
			isHandled = TRUE;
		break;

		case SLR_3DMODE_3D_REALID_CVT_2D:
			//flow_message_push(_UI2FL_3D_INPUT_REALID_CVT_2D);
			Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_REALID_CVT_2D);
			Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_REALID_CVT_2D);
			isHandled = TRUE;
		break;

		case SLR_3DMODE_3D_SENSIO_CVT_2D:
			//flow_message_push(_UI2FL_3D_INPUT_SENSIO_CVT_2D);
			Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_SENSIO_CVT_2D);
			Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_SENSIO_CVT_2D);
			isHandled = TRUE;
		break;

		default:
			rtd_pr_vbe_info("[%s][%d]unknown message.\n", __FUNCTION__, __LINE__);
		break;
	}

#if 0
	if(!isHandled) {
		Scaler_ForceBg(SLR_MAIN_DISPLAY, SLR_FORCE_BG_SYSTEM, false);
		fw_flow_Set3DModeReadyFlag(TRUE); // restore 3d mode flag
	}
#endif


	//if (!(Scaler_InputSrcGetMainChType()== _SRC_HDMI && (!drvif_IsHDMI())))
	//	pthread_mutex_unlock(&scalerMutex);

	//SetAudioMuteCtrl(AUDIO_MUTE_ID_SYSTEM,false);

	return isHandled; // 0:Not support 1:Support
}

/*
unsigned char Scaler_Set3DMode(unsigned char message, bool muteAudio)
{
	//if (!(Scaler_InputSrcGetMainChType()== _SRC_HDMI && (!drvif_IsHDMI())))
	//	pthread_mutex_lock(&scalerMutex);

	unsigned char isHandled = FALSE;

	rtd_pr_vbe_info("[3D]%s(%d)\n", __FUNCTION__, message);

	if (Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
	{
#ifdef USE_3D_AUTO_DETECT//JunnyModify20130726
		if((message == SLR_3DMODE_3D_AUTO) && (Scaler_Get3DMode() != SLR_3DMODE_3D_AUTO_CVT_2D)){
			; // don't forceBG if going to enable VO auto 3D detect mode
		}else
#endif
#if 0
		{
			if(message != Scaler_Get3DMode()){
				// enable forceBG when change 3D mode
				Scaler_ForceDisableBg(FALSE);
			}
			Scaler_ForceBg(SLR_MAIN_DISPLAY, SLR_FORCE_BG_SYSTEM, true);
		}

#endif

		//fw_flow_Set3DModeReadyFlag(FALSE);

		// AP set this flag as false, then scaler will set it as true after change to specific 3D (or 2D) mode.
		//Scaler_Set3DMode_ScalerHasProcessMsgFlag(FALSE);

		// mute audio before change 3D mode
		if( (Scaler_InputSrcGetMainChType() != _SRC_VO) && (Scaler_Get3DMode() != message) && (muteAudio == true)) {
#ifdef USE_3D_AUTO_DETECT//JunnyModify20130726
		if((message == SLR_3DMODE_3D_AUTO) && (Scaler_Get3DMode() != SLR_3DMODE_3D_AUTO_CVT_2D)){
			; // don't mute audio if going to enable VO auto 3D detect mode
		}else
#endif
			//SetAudioMuteCtrl(AUDIO_MUTE_ID_SYSTEM,true);
		} // mute audio before exit 3D mode in external input source

		switch(message){
			// 3D->2D
			case SLR_3DMODE_2D:
				WaitDisplayReady();

				//fw_flow_SetAPDispReadyFlag(FALSE);
				// Disable 3D->2D finishflag
				//Scaler_Set3D2DFinishFlag(FALSE);

#ifdef USE_3D_AUTO_DETECT//JunnyModify20130726
				if(Scaler_Get3D_VoAutoDetectMode())
					Scaler_Set3D_VoAutoDetect_Enable(FALSE);
#endif
				//flow_message_push(_UI2FL_3D_FORCE_2D_MODE_EVENT);
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_2D_ONLY);
				Scaler_Set3DMode_Attr(SLR_3DMODE_2D);
				Scaler_ForceSupport3D(FALSE);
				isHandled = TRUE;
				break;

			case SLR_3DMODE_3D_AUTO:
#ifdef USE_3D_AUTO_DETECT//JunnyModify20130726
				if(Scaler_Get3DMode() != SLR_3DMODE_3D_AUTO_CVT_2D){
					Scaler_Set3D_VoAutoDetect_Enable(TRUE);
					isHandled = TRUE;
				}else
#endif
				if(Scaler_Get3DMode() == SLR_3DMODE_3D_AUTO_CVT_2D){
					//flow_message_push(_UI2FL_3D_INPUT_AUTO_FROM_INFO_FRAME_FLAG_CHANGE);
					Scaler_Set3DMode_Attr(SLR_3DMODE_3D_AUTO);
					isHandled = TRUE;
				}
				break;

			case SLR_3DMODE_3D_SBS:
				if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_SIDE_BY_SIDE_HALF) == TRUE)
				{
					//flow_message_push(_UI2FL_3D_INPUT_SIDE_BY_SIDE_HALF_FLAG_CHANGE);
					Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_SIDE_BY_SIDE_HALF);
					Scaler_Set3DMode_Attr(SLR_3DMODE_3D_SBS);
					isHandled = TRUE;
				}
				break;

			case SLR_3DMODE_3D_TB:
				if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_TOP_AND_BOTTOM) == TRUE)
				{
					//flow_message_push(_UI2FL_3D_INPUT_TOP_AND_BOTTOM_FLAG_CHANGE);
					Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM);
					Scaler_Set3DMode_Attr(SLR_3DMODE_3D_TB);
					isHandled = TRUE;
				}
				break;

			case SLR_3DMODE_3D_FP:
				if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_FRAME_PACKING) == TRUE)
				{
					if(Scaler_InputSrcGetMainChType() == _SRC_VO){
						int result;
						// Frame sequence 3D
						result = drvif_scaler3d_init_vo_3d_mode(SLR_3DMODE_3D_FP);
						if(result != 0)
							rtd_pr_vbe_info("[SLR] ERR: Set init_vo_3d_mode[FP_3D] FAIL!!\n");
						else{
							rtd_pr_vbe_info("[AUTO_3D] SetVoAuto3DDetect DONE!!\n");
							isHandled = TRUE;
					    }
					}else{
					//flow_message_push(_UI2FL_3D_INPUT_FRAME_PACKING_FLAG_CHANGE);
					Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_FRAME_PACKING);
					Scaler_Set3DMode_Attr(SLR_3DMODE_3D_FP);
					isHandled = TRUE;
				}
				}
				break;

			case SLR_3DMODE_3D_LBL:
				if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_LINE_ALTERNATIVE) == TRUE)
				{
					//flow_message_push(_UI2FL_3D_INPUT_LINE_BY_LINE_FLAG_CHANGE);
					Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_LINE_ALTERNATIVE);
					Scaler_Set3DMode_Attr(SLR_3DMODE_3D_LBL);
					isHandled = TRUE;
				}
				break;

			case SLR_3DMODE_3D_VSTRIP:
				if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_VERTICAL_STRIPE) == TRUE)
				{
					//flow_message_push(_UI2FL_3D_INPUT_VERTICAL_STRIPE_FLAG_CHANGE);
					Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_VERTICAL_STRIP);
					Scaler_Set3DMode_Attr(SLR_3DMODE_3D_VSTRIP);
					isHandled = TRUE;
				}
				break;

			case SLR_3DMODE_3D_CKB:
				if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_CHECKER_BOARD) == TRUE)
				{
					//flow_message_push(_UI2FL_3D_INPUT_CHECKER_BOARD_FLAG_CHANGE);
					Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_CHECKER_BOARD);
					Scaler_Set3DMode_Attr(SLR_3DMODE_3D_CKB);
					isHandled = TRUE;
				}
				break;

			case SLR_3DMODE_3D_REALID:
				if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_REALID) == TRUE)
				{
					//flow_message_push(_UI2FL_3D_INPUT_REALID_FLAG_CHANGE);
					Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_REALID);
					Scaler_Set3DMode_Attr(SLR_3DMODE_3D_REALID);
					isHandled = TRUE;
				}
				break;

			case SLR_3DMODE_3D_SENSIO:
				if (Scaler_Get3D_IsSupport3D(FALSE, HDMI3D_SENSIO) == TRUE)
				{
					//flow_message_push(_UI2FL_3D_INPUT_SENSIO_FLAG_CHANGE);
					Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_SENSIO);
					Scaler_Set3DMode_Attr(SLR_3DMODE_3D_SENSIO);
					isHandled = TRUE;
				}
				break;

			case SLR_3DMODE_2D_CVT_3D:
				// 2D->3D
				if (Scaler_Get3D_IsSupport3D(TRUE, 0) == TRUE)
				{
					WaitDisplayReady();
					//fw_flow_SetAPDispReadyFlag(FALSE);
					//flow_message_push(_UI2FL_3D_INPUT_2D_CVT_3D);
					Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_2D_CVT_3D);
					Scaler_Set3DMode_Attr(SLR_3DMODE_2D_CVT_3D);
					isHandled = TRUE;
				}
				break;
				case SLR_3DMODE_3D_AUTO_CVT_2D:
				switch(cur3DMode){
					case SLR_3DMODE_3D_AUTO: // when AP's 3D type setting is auto mode
//						if (GET_IS3DFMT() == 0) // if scaler is still in 2D mode, we use auto detect 3d format to decide
//						{
#if defined(USE_3D_AUTO_DETECT) && defined(ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE) // VO auto 3D to 3Dto2D mode format change
							UINT8 cur3DType = Scaler_Disp3dGetInfo(SLR_DISP_3D_3DTYPE);
							if((Scaler_InputSrcGetMainChType() == _SRC_VO) && ((drvif_scaler3d_decide_3d_SG_Disable_IDMA(cur3DType) == TRUE))){
								// VO auto 3D change to 3Dto2D mode (disable VO auto detect)
								if (cur3DType == SLR_3D_SIDE_BY_SIDE_HALF){
									//flow_message_push(_UI2FL_3D_INPUT_SIDE_BY_SIDE_HALF_CVT_2D);
									Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D);
									Scaler_Set3DMode_Attr(SLR_3DMODE_3D_SBS_CVT_2D);
								}else if (cur3DType == SLR_3D_TOP_AND_BOTTOM){
									//flow_message_push(_UI2FL_3D_INPUT_TOP_AND_BOTTOM_CVT_2D);
									Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM_CVT_2D);
									Scaler_Set3DMode_Attr(SLR_3DMODE_3D_TB_CVT_2D);
								}else{
									//flow_message_push(_UI2FL_3D_INPUT_AUTO_FROM_INFO_FRAME_CVT_2D);
									Scaler_Set3DMode_Attr(SLR_3DMODE_3D_AUTO_CVT_2D);
								}
								//drvif_scaler3d_init_vo_3d_mode(SLR_3DMODE_2D);
								Scaler_Set3D_VoAutoDetect_UserNewMode((SLR_3D_MODE)Scaler_Get3DMode());
								rtd_pr_vbe_info("[AUTO_3D] change to 3Dto2D mode[%d]\n", Scaler_Get3DMode());
							}else
#endif
							{
								//flow_message_push(_UI2FL_3D_INPUT_AUTO_FROM_INFO_FRAME_CVT_2D);
								Scaler_Set3DMode_Attr(SLR_3DMODE_3D_AUTO_CVT_2D);
							}
//						}
						break;

					case SLR_3DMODE_3D_SBS: // when AP's 3D type setting is SBS, we use SBS to transfer
						//flow_message_push(_UI2FL_3D_INPUT_SIDE_BY_SIDE_HALF_CVT_2D);
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D);
						Scaler_Set3DMode_Attr(SLR_3DMODE_3D_SBS_CVT_2D);
						break;

					case SLR_3DMODE_3D_TB: // when AP's 3D type setting is TB, we use TB to transfer
						//flow_message_push(_UI2FL_3D_INPUT_TOP_AND_BOTTOM_CVT_2D);
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM_CVT_2D);
						Scaler_Set3DMode_Attr(SLR_3DMODE_3D_TB_CVT_2D);
						break;

					case SLR_3DMODE_3D_FP: // when AP's 3D type setting is FP, we use FP to transfer
						//flow_message_push(_UI2FL_3D_INPUT_FRAME_PACKING_CVT_2D);
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_FRAME_PACKING_CVT_2D);
						Scaler_Set3DMode_Attr(SLR_3DMODE_3D_FP_CVT_2D);
						break;

					default:
						//flow_message_push(_UI2FL_3D_INPUT_AUTO_FROM_INFO_FRAME_CVT_2D);
						Scaler_Set3DMode_Attr(SLR_3DMODE_3D_AUTO_CVT_2D);
						break;
				}

				isHandled = TRUE;
				break;
			case SLR_3DMODE_3D_SBS_CVT_2D:
				//flow_message_push(_UI2FL_3D_INPUT_SIDE_BY_SIDE_HALF_CVT_2D);
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D);
				Scaler_Set3DMode_Attr(SLR_3DMODE_3D_SBS_CVT_2D);
				isHandled = TRUE;
				break;

			case SLR_3DMODE_3D_TB_CVT_2D:
				//flow_message_push(_UI2FL_3D_INPUT_TOP_AND_BOTTOM_CVT_2D);
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM_CVT_2D);
				Scaler_Set3DMode_Attr(SLR_3DMODE_3D_TB_CVT_2D);
				isHandled = TRUE;
				break;

			case SLR_3DMODE_3D_FP_CVT_2D:
				//flow_message_push(_UI2FL_3D_INPUT_FRAME_PACKING_CVT_2D);
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_FRAME_PACKING_CVT_2D);
				Scaler_Set3DMode_Attr(SLR_3DMODE_3D_FP_CVT_2D);
				isHandled = TRUE;
				break;

			case SLR_3DMODE_3D_LBL_CVT_2D:
				//flow_message_push(_UI2FL_3D_INPUT_LINE_BY_LINE_CVT_2D);
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_LINE_ALTERNATIVE_CVT_2D);
				Scaler_Set3DMode_Attr(SLR_3DMODE_3D_LBL_CVT_2D);
				isHandled = TRUE;
				break;

			case SLR_3DMODE_3D_VSTRIP_CVT_2D:
				//flow_message_push(_UI2FL_3D_INPUT_VERTICAL_STRIPE_CVT_2D);
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_VERTICAL_STRIP_CVT_2D);
				Scaler_Set3DMode_Attr(SLR_3DMODE_3D_VSTRIP_CVT_2D);
				isHandled = TRUE;
				break;

			case SLR_3DMODE_3D_CKB_CVT_2D:
				//flow_message_push(_UI2FL_3D_INPUT_CHECKER_BOARD_CVT_2D);
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_VERTICAL_STRIP_CVT_2D);
				Scaler_Set3DMode_Attr(SLR_3DMODE_3D_CKB_CVT_2D);
				isHandled = TRUE;
				break;

			case SLR_3DMODE_3D_REALID_CVT_2D:
				//flow_message_push(_UI2FL_3D_INPUT_REALID_CVT_2D);
				Scaler_Set3DMode_Attr(SLR_3DMODE_3D_REALID_CVT_2D);
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_REALID_CVT_2D);
				isHandled = TRUE;
				break;

			case SLR_3DMODE_3D_SENSIO_CVT_2D:
				//flow_message_push(_UI2FL_3D_INPUT_SENSIO_CVT_2D);
				Scaler_Set3DMode_Attr(SLR_3DMODE_3D_SENSIO_CVT_2D);
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_SENSIO_CVT_2D);
				isHandled = TRUE;
				break;

			default:
				rtd_pr_vbe_info("[%s][%d]unknown message.\n", __FUNCTION__, __LINE__);
				break;
		}
#if 0
		if(!isHandled) {
		 	Scaler_ForceBg(SLR_MAIN_DISPLAY, SLR_FORCE_BG_SYSTEM, false);
			fw_flow_Set3DModeReadyFlag(TRUE); // restore 3d mode flag
		}
#endif
	}

	//if (!(Scaler_InputSrcGetMainChType()== _SRC_HDMI && (!drvif_IsHDMI())))
	//	pthread_mutex_unlock(&scalerMutex);

	//SetAudioMuteCtrl(AUDIO_MUTE_ID_SYSTEM,false);

	return isHandled; // 0:Not support 1:Support
}
*/

unsigned char Scaler_Set3DLRSwap(unsigned char message)
{
	unsigned char isHandled = FALSE;
	if (Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
	{
		Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_LR_SEQUENCE,message);
		if(Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()){ // PR 3D
			if(modestate_I3DDMA_get_In3dMode()){
				//flow_message_push(_UI2FL_3D_FORCE_LR_CHANGE);
				isHandled = TRUE;
			}else{
				drvif_memory_set_3D_LR_swap(message,Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE));
				return isHandled;
			}
		}
#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
		else if(drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() && drvif_scaelr3d_decide_is_3D_display_mode() && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE) != SLR_3D_2D_CVT_3D)){
			//[SG] non-IDMA SG 3D mode
			drvif_memory_set_3D_LR_swap_for_SG_frc_in_mDomain(FALSE);
			rtd_pr_vbe_info("[SG] FRC_IN_M L/R SEQ=%d\n", Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_LR_SEQUENCE));
 		 }
#endif
		else { // SG 3D
			// SG MVC 3D in data FRC mode LR swap control in M-capture
			if(modestate_get_vo3d_in_framePacking_FRC_mode()){
#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
				drvif_memory_set_3D_LR_swap_for_SG_frc_in_mDomain(Scaler_Get_CurVoInfo_plane() == VO_VIDEO_PLANE_V1? FALSE: TRUE);
#else
				drvif_memory_set_MVC_FRC_LR_swap(TRUE);
#endif
			} // SG 3D L/R toggle by HW
			else if(drvif_scaelr3d_decide_is_3D_display_mode() && fw_scaler_get_M_capture_LR_ctrl_by_HW()){
				drvif_memory_set_MVC_FRC_LR_swap(FALSE);
			}else{
			;//flow_message_push(_UI2FL_3D_FORCE_LR_CHANGE);
			}
			isHandled = TRUE;
		}
	}

	return isHandled;
}

unsigned char Scaler_Get3D_IsFramePacking(void)
{
	// FIXME: need to add new mode to modeTable
	UINT16 width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
	UINT16 length = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
	UINT16 width_tolerance = 10;
	UINT16 length_tolerance = 10;

	if ((ABS(width, 1920) < width_tolerance) && (ABS(length, 2205) < length_tolerance))
	{
		return 2; // 1080p FP
	}
	else if ((ABS(width, 1280) < width_tolerance) && (ABS(length, 1470) < length_tolerance))
	{
		return 1; // 720p FP
	}
	else
	{
		return 0;
	}
}

// 3D depth shift range control, 3/5 of OSD value (if OSD range is +/- 15)
#define _3D_SHIFT_MAX_MULTIPLE					3
#define _3D_SHIFT_MAX_DIV						5
static INT8 cur3D_Depth = 0;

void Scaler_Set3D_Depth(INT8 value)
{
	INT8 orig_value = 0;
	UINT16 borderwidth = 0;
	unsigned int act_width = 0;

	orig_value = cur3D_Depth;
	cur3D_Depth = value;
	act_width = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID);

	if (Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
	{
		//frank@0808 add below code to support MVC depth function.
		{
			INT8 newDepth=value;
			INT32 actWid, dispWid;

			// Reduce 3D depth shift range in 3D mode, assume 3D depth shift range between +/- 15
			// new depth = (3/5 value) / scalingUpRate
			actWid = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
			dispWid = Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
			if(actWid && (_3D_SHIFT_MAX_DIV*dispWid > actWid))
				newDepth = (value*_3D_SHIFT_MAX_MULTIPLE*100)/(_3D_SHIFT_MAX_DIV*dispWid*100 / actWid);
			rtd_pr_vbe_debug("[DBG] depth/value=%d/%d, act/disp=%d/%d\n", newDepth, value, actWid, dispWid);

			if ((orig_value >= 0 && cur3D_Depth > orig_value) || (orig_value <= 0 && cur3D_Depth < orig_value)) // set more bolder
			{
				if(act_width > 0)
					borderwidth = ABS(newDepth,0) * 2 * _DISP_WID/ act_width;
				else
					rtd_pr_vbe_notice("[DBG] act_width is %d !!!\n", act_width);
				if (drvif_scaler3d_decide_3d_PR_enter_3DDMA() == FALSE)
				{
					borderwidth *= 2;
				}
				drvif_scalerdisplay_set_bg_color(SLR_MAIN_DISPLAY, _DISPLAY_EB, 0x0, 0x0, 0x0); // set external border color
				scalerdisplay_set_main_display_window_H(0, _DISP_WID, ABS(borderwidth, 0));

				// Prevent the adjust speed is not the same with [the else case]
				WaitFor_IVS1_Done();
				WaitFor_DEN_STOP_Done();
				WaitFor_IVS1_Done();
				WaitFor_DEN_STOP_Done();

				Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DDEPTH, newDepth);
				state_update_disp3d_info();
			}
			else
			{
				Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DDEPTH, newDepth);
				state_update_disp3d_info();

				// Make sure the shift function is ready to prevent dirty image
				WaitFor_IVS1_Done();
				WaitFor_DEN_STOP_Done();
				WaitFor_IVS1_Done();
				WaitFor_DEN_STOP_Done();

				if(act_width > 0)
					borderwidth = ABS(newDepth,0) * 2 * _DISP_WID/ act_width;
				else
					rtd_pr_vbe_notice("[DBG] act_width is %d !!!\n", act_width);
				if (drvif_scaler3d_decide_3d_PR_enter_3DDMA() == FALSE)
				{
					borderwidth *= 2;
				}
				drvif_scalerdisplay_set_bg_color(SLR_MAIN_DISPLAY, _DISPLAY_EB, 0x0, 0x0, 0x0); // set external border color
				scalerdisplay_set_main_display_window_H(0, _DISP_WID, ABS(borderwidth, 0));
			}
		}
	}

	rtd_pr_vbe_info("[3D]AP call %s(%d)\n", __FUNCTION__, cur3D_Depth);

}

INT8 Scaler_Get3D_Depth(void)
{
	return cur3D_Depth;
}

void state_update_disp3d_info(void)
{
	SCALER_DISP3DINFO_PKG info3d_pkg;
	int ret;
	unsigned int i = 0, ulIPKGItemCount = 0;
	unsigned int *pulTemp;

	info3d_pkg.disp3d_info.bIs3dFmt = 0;
	info3d_pkg.disp3d_info.uc3dType = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE);
	info3d_pkg.disp3d_info.uc3dFlow = 0;
	info3d_pkg.disp3d_info.bIs2Dcvt3D = 0;
	info3d_pkg.disp3d_info.bIs2Dcvt3D_user = 0;
	info3d_pkg.disp3d_info.bIs2Dcvt3D_ddMode = 0;
	info3d_pkg.disp3d_info.bIsVflip = Get_PANEL_VFLIP_ENABLE() ;
	info3d_pkg.disp3d_info.bEnableDoubleDVS = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_ENABLE_DOUBLE_DVS);
	info3d_pkg.disp3d_info.capWid = Scaler_DispGetInputInfoByDisp(0, SLR_INPUT_CAP_WID);
	info3d_pkg.disp3d_info.capLen = Scaler_DispGetInputInfoByDisp(0, SLR_INPUT_CAP_LEN);
	info3d_pkg.disp3d_info.bInterlaced = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE);
	info3d_pkg.disp3d_info.display_size = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_DISPLAY_SIZE);
	info3d_pkg.disp3d_info.main_addr = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MAIN_ADDR);
	info3d_pkg.disp3d_info.main_sec_addr = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MAIN_SEC_ADDR);
	info3d_pkg.disp3d_info.main_third_addr = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MAIN_THIRD_ADDR);
	info3d_pkg.disp3d_info.sub_addr = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_SUB_ADDR);
	info3d_pkg.disp3d_info.sub_sec_addr = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_SUB_SEC_ADDR);
	info3d_pkg.disp3d_info.sub_third_addr = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_SUB_THIRD_ADDR);
	info3d_pkg.disp3d_info.ucLRsequence = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_LR_SEQUENCE);
	info3d_pkg.disp3d_info.c3dDepth = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DDEPTH);
	info3d_pkg.disp3d_info.c2Dcvt3dDepth = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_2DCVT3DDEPTH);
	info3d_pkg.disp3d_info.ui3dBlkStep = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_BLKSTEP);
	info3d_pkg.disp3d_info.ui3dDropLineMode = drvif_scaler3d_get_vo_3dDropLineMode();;
	info3d_pkg.disp3d_info.vgip2_vfreq = 0;
	info3d_pkg.disp3d_info.uc3dMcapFlow = (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)? 0: 1);// data frame sync(0), FRC(1)

	info3d_pkg.disp3d_info.vgip2_src = 0;
	info3d_pkg.disp3d_info.mainSub_freeRun = 0;

	pulTemp = (unsigned int *)(&info3d_pkg);

	ulIPKGItemCount = sizeof(SCALER_DISP3DINFO_PKG) / sizeof(unsigned int);
	// change endian
	for (i = 0; i < ulIPKGItemCount; i++)
		pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);

	//frank@0629 compare Disp_info is the same of video firmware,when the result is the same then return to speed up the code
	if (memcmp((unsigned char*)Scaler_GetShareMemVirAddr(SCALERIOC_SETDISP3DINFO), pulTemp, sizeof(SCALER_DISP3DINFO_PKG)) == 0) //compare equal or not
		return;


	// copy from RPC share memory
	memcpy((unsigned char*)Scaler_GetShareMemVirAddr(SCALERIOC_SETDISP3DINFO), pulTemp, sizeof(SCALER_DISP3DINFO_PKG));

	if ((ret = Scaler_SendRPC(SCALERIOC_SETDISP3DINFO, 0, 0)) < 0)
	{
		rtd_pr_vbe_info("ret = %d, set disp3d_info to driver fail !!!\n", ret );
		return;
	}

}

/*
void state_update_disp3d_info(void)
{
	SCALER_DISP3DINFO_PKG info3d_pkg;
	int ret;
	unsigned long i = 0, ulIPKGItemCount = 0;
	unsigned long *pulTemp;

	info3d_pkg.disp3d_info.bIs3dFmt = 0;
	info3d_pkg.disp3d_info.uc3dType = Scaler_Disp3dGetInfo(SLR_DISP_3D_3DTYPE);
	info3d_pkg.disp3d_info.uc3dFlow = 0;
	info3d_pkg.disp3d_info.bIs2Dcvt3D = 0;
	info3d_pkg.disp3d_info.bIs2Dcvt3D_user = 0;
	info3d_pkg.disp3d_info.bIs2Dcvt3D_ddMode = 0;
	info3d_pkg.disp3d_info.bIsVflip = Get_PANEL_VFLIP_ENABLE() ;
    info3d_pkg.disp3d_info.bEnableDoubleDVS = Scaler_Disp3dGetInfo(SLR_DISP_3D_ENABLE_DOUBLE_DVS);
	info3d_pkg.disp3d_info.capWid = Scaler_DispGetInputInfoByDisp(0, SLR_INPUT_CAP_WID);
	info3d_pkg.disp3d_info.capLen = Scaler_DispGetInputInfoByDisp(0, SLR_INPUT_CAP_LEN);
	info3d_pkg.disp3d_info.bInterlaced = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE);
	info3d_pkg.disp3d_info.display_size = Scaler_Disp3dGetInfo(SLR_DISP_3D_DISPLAY_SIZE);
	info3d_pkg.disp3d_info.main_addr = Scaler_Disp3dGetInfo(SLR_DISP_3D_MAIN_ADDR);
	info3d_pkg.disp3d_info.main_sec_addr = Scaler_Disp3dGetInfo(SLR_DISP_3D_MAIN_SEC_ADDR);
	info3d_pkg.disp3d_info.main_third_addr = Scaler_Disp3dGetInfo(SLR_DISP_3D_MAIN_THIRD_ADDR);
	info3d_pkg.disp3d_info.sub_addr = Scaler_Disp3dGetInfo(SLR_DISP_3D_SUB_ADDR);
	info3d_pkg.disp3d_info.sub_sec_addr = Scaler_Disp3dGetInfo(SLR_DISP_3D_SUB_SEC_ADDR);
	info3d_pkg.disp3d_info.sub_third_addr = Scaler_Disp3dGetInfo(SLR_DISP_3D_SUB_THIRD_ADDR);
	info3d_pkg.disp3d_info.ucLRsequence = Scaler_Disp3dGetInfo(SLR_DISP_3D_LR_SEQUENCE);
	info3d_pkg.disp3d_info.c3dDepth = Scaler_Disp3dGetInfo(SLR_DISP_3D_3DDEPTH);
	info3d_pkg.disp3d_info.c2Dcvt3dDepth = Scaler_Disp3dGetInfo(SLR_DISP_3D_2DCVT3DDEPTH);
	info3d_pkg.disp3d_info.ui3dBlkStep = Scaler_Disp3dGetInfo(SLR_DISP_3D_BLKSTEP);
	info3d_pkg.disp3d_info.ui3dDropLineMode = drvif_scaler3d_get_vo_3dDropLineMode();;
	info3d_pkg.disp3d_info.vgip2_vfreq = 0;
	info3d_pkg.disp3d_info.uc3dMcapFlow = (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)? 0: 1);// data frame sync(0), FRC(1)

	info3d_pkg.disp3d_info.vgip2_src = 0;
	info3d_pkg.disp3d_info.mainSub_freeRun = 0;

	pulTemp = (unsigned long *)(&info3d_pkg);

	ulIPKGItemCount = sizeof(SCALER_DISP3DINFO_PKG) / sizeof(unsigned long);
	// change endian
	for (i = 0; i < ulIPKGItemCount; i++)
		pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);

	//frank@0629 compare Disp_info is the same of video firmware,when the result is the same then return to speed up the code
	if (memcmp((unsigned char*)Scaler_GetShareMemVirAddr(SCALERIOC_SETDISP3DINFO), pulTemp, sizeof(SCALER_DISP3DINFO_PKG)) == 0) //compare equal or not
		return;


	// copy from RPC share memory
	memcpy((unsigned char*)Scaler_GetShareMemVirAddr(SCALERIOC_SETDISP3DINFO), pulTemp, sizeof(SCALER_DISP3DINFO_PKG));

	if ((ret = Scaler_SendRPC(SCALERIOC_SETDISP3DINFO, 0, 0)) < 0)
	{
		rtd_pr_vbe_info("ret = %d, set disp3d_info to driver fail !!!\n", ret );
		return;
	}

}
*/

static unsigned char g_b3dVoAutoDetectEnableFlag = FALSE;
// VO 3D auto detect enable/disable control
int Scaler_Set3D_VoAutoDetect_Enable(unsigned char enable)
{
	int result=0;

	rtd_pr_vbe_debug("[AUTO_3D] SetVoAuto3DDetect=%d\n", enable);
	result = drvif_scaler3d_init_vo_3d_mode(enable? SLR_3DMODE_3D_AUTO: SLR_3DMODE_2D);

	if(result != TRUE)
		rtd_pr_vbe_debug("[SLR] ERR: Set init_vo_3d_mode[%d] FAIL!!\n", enable);
	else{
		g_b3dVoAutoDetectEnableFlag = enable;
		rtd_pr_vbe_debug("[AUTO_3D] SetVoAuto3DDetect DONE!!\n");
    }

	return result;
}


unsigned char Scaler_Get3D_VoAutoDetectMode(void)
{
	unsigned char result=FALSE;
	if (!Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
		return FALSE;

	if(!Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE())
		return FALSE;

	result = g_b3dVoAutoDetectEnableFlag;

	return result;
}

static VO_3D_MODE_TYPE g_curVoDetect3dResult=VO_2D_MODE;
void check_vo3d_detect_result(SCALER_DISP_CHANNEL display)
{
	g_curVoDetect3dResult = VO_2D_MODE;
	if(Scaler_Get3D_VoAutoDetectMode() == TRUE){
		unsigned int src_org;

		src_org =  Scaler_InputSrcGetFrom(display);

		// not enable VO auto detect in VO source or ATV source
		if((src_org != _SRC_VO) && (src_org != _SRC_TV)  && (Scaler_Get3DMode(display) != SLR_3DMODE_3D_AUTO))
		{
			//static int count = 0;
			//if (count++ > 10)
			{
				// copy from RPC share memory
				SLR_3DDETECTINFO Info3D ;
				unsigned int i = 0, ulVOInfoItemCount = 0;
				unsigned int *pulTemp;
				unsigned char cur3dMode = Scaler_Get3DMode(display);

				memcpy(&Info3D, (unsigned char*)Scaler_GetShareMemVirAddr(SCALERIOC_GET3DDETECTINFO), sizeof(SLR_3DDETECTINFO));
				ulVOInfoItemCount = sizeof(SLR_3DDETECTINFO) /  sizeof(UINT32);
				pulTemp = (unsigned int *)&Info3D;
				// change endian
				for (i = 0; i < ulVOInfoItemCount; i++)
					pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);
				rtd_pr_vbe_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~cur/Info3D.mode_3d = %d/%d\n", cur3dMode, Info3D.mode_3d);

				if ((Info3D.mode_3d == VO_3D_SIDE_BY_SIDE_HALF) && (cur3dMode != SLR_3DMODE_3D_SBS)){
					Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_SBS);
				}else if ((Info3D.mode_3d == VO_3D_TOP_AND_BOTTOM) && (cur3dMode != SLR_3DMODE_3D_TB)){
					Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_TB);
				}else if ((Info3D.mode_3d == VO_2D_MODE) && (cur3dMode != SLR_3DMODE_2D)){
					Scaler_Set3DMode_Attr(display, SLR_3DMODE_2D);
				}
				g_curVoDetect3dResult = Info3D.mode_3d;
				//count = 0;
			}
		}
	}

	return;
}

/*
void check_vo3d_detect_result(void)
{
	g_curVoDetect3dResult = VO_2D_MODE;

	if(Scaler_Get3D_VoAutoDetectMode() == TRUE){
		unsigned int src_org;

			src_org =  Scaler_InputSrcGetMainChType();

		// not enable VO auto detect in VO source or ATV source
		if((src_org != _SRC_VO) && (src_org != _SRC_TV)  && (Scaler_Get3DMode() != SLR_3DMODE_3D_AUTO))
		{
			static int count = 0;
			//if (count++ > 10)
			{
				// copy from RPC share memory
				SLR_3DDETECTINFO Info3D ;
				unsigned int i = 0, ulVOInfoItemCount = 0;
				unsigned int *pulTemp;
				unsigned char cur3dMode = Scaler_Get3DMode();

				memcpy(&Info3D, (unsigned char*)Scaler_GetShareMemVirAddr(SCALERIOC_GET3DDETECTINFO), sizeof(SLR_3DDETECTINFO));
				ulVOInfoItemCount = sizeof(SLR_3DDETECTINFO) /  sizeof(UINT32);
				pulTemp = (unsigned int *)&Info3D;
				// change endian
				for (i = 0; i < ulVOInfoItemCount; i++)
					pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);
				rtd_pr_vbe_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~cur/Info3D.mode_3d = %d/%d\n", cur3dMode, Info3D.mode_3d);

				if ((Info3D.mode_3d == VO_3D_SIDE_BY_SIDE_HALF) && (cur3dMode != SLR_3DMODE_3D_SBS)){
					Scaler_Set3DMode_Attr(SLR_3DMODE_3D_SBS);
				}else if ((Info3D.mode_3d == VO_3D_TOP_AND_BOTTOM) && (cur3dMode != SLR_3DMODE_3D_TB)){
					Scaler_Set3DMode_Attr(SLR_3DMODE_3D_TB);
				}else if ((Info3D.mode_3d == VO_2D_MODE) && (cur3dMode != SLR_3DMODE_2D)){
					Scaler_Set3DMode_Attr(SLR_3DMODE_2D);
				}
				g_curVoDetect3dResult = Info3D.mode_3d;
				count = 0;
			}
		}
	}

	return;
}
*/

VO_3D_MODE_TYPE modestate_get_vo3d_detect_result(void)
{
	return g_curVoDetect3dResult;
}

void modestate_HDMI_3D_format_auto_detection(SCALER_DISP_CHANNEL display)
{
//20120818 fix vo gatting[379]
	unsigned char ucHdmi_Real3Dfmt = drvif_Hdmi_GetReal3DFomat(); // we get real 3D format without using fw 3D format
	UINT8 ucGet3DMode = Scaler_Get3DMode(display);

	rtd_pr_vbe_info("** Real3Dfmt =%d,src/3DMode/type =%d/%d/%d \n",ucHdmi_Real3Dfmt,Scaler_InputSrcGetFrom(display), Scaler_Get3DMode(display), Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE));

	switch(ucHdmi_Real3Dfmt)
	{
		case HDMI3D_FRAME_PACKING:
			rtd_pr_vbe_info("FP 3D, Interlaced=%d\n", Scaler_DispGetStatus(display, SLR_DISP_INTERLACE));

			if(Scaler_Get3DMode(display)==SLR_3DMODE_3D_FP_CVT_2D || Scaler_Get3DMode(display)==SLR_3DMODE_3D_AUTO_CVT_2D)
			{
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_FRAME_PACKING_CVT_2D);
			}
			else
			{
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_FRAME_PACKING);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_AUTO);
			}

#if 1//def ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
			if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING)
				&& ((Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE)))
			{
				if(drvif_scaler3d_decide_HDMI_framePacking_interlaced_status()){
					Scaler_DispSetStatus(display, SLR_DISP_INTERLACE, TRUE);
					rtd_pr_vbe_info("[HDMI][FP] change interlaced mode = 1\n");
				}
			}
#endif

			break;
		case HDMI3D_TOP_AND_BOTTOM:
			if(Scaler_Get3DMode(display)==SLR_3DMODE_3D_TB_CVT_2D || Scaler_Get3DMode(display)==SLR_3DMODE_3D_AUTO_CVT_2D)
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM_CVT_2D);
			else
			{
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_AUTO);
			}
			break;
		case HDMI3D_SIDE_BY_SIDE_FULL:
		case HDMI3D_SIDE_BY_SIDE_HALF:
			if(Scaler_Get3DMode(display)==SLR_3DMODE_3D_SBS_CVT_2D || Scaler_Get3DMode(display)==SLR_3DMODE_3D_AUTO_CVT_2D)
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D);
			else
			{
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_SIDE_BY_SIDE_HALF);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_3D_AUTO);
			}
			break;
		case HDMI3D_2D_ONLY:
			 if(ucGet3DMode == SLR_3DMODE_2D_CVT_3D)
			{
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_2D_CVT_3D);
			}
			else if(ucGet3DMode >= SLR_3DMODE_3D_SBS_CVT_2D  && ucGet3DMode <=SLR_3DMODE_3D_SENSIO_CVT_2D)
			{

				switch(ucGet3DMode)
				{
					case SLR_3DMODE_3D_SBS_CVT_2D:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE, SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D);
					break;
				      case SLR_3DMODE_3D_TB_CVT_2D:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM_CVT_2D);
					break;
					case SLR_3DMODE_3D_FP_CVT_2D:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_FRAME_PACKING_CVT_2D);
					break;
					// -------------- new 3D format
					case SLR_3DMODE_3D_LBL_CVT_2D:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_LINE_ALTERNATIVE_CVT_2D);
					break;
					case SLR_3DMODE_3D_VSTRIP_CVT_2D:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_VERTICAL_STRIP_CVT_2D);
					break;
					case SLR_3DMODE_3D_CKB_CVT_2D:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_CHECKER_BOARD_CVT_2D);
					break;
					case SLR_3DMODE_3D_REALID_CVT_2D:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_REALID_CVT_2D);
					break;
					case SLR_3DMODE_3D_SENSIO_CVT_2D:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_SENSIO_CVT_2D);
					break;
					// -------------- --------------
					default:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_2D_ONLY);
						Scaler_Set3DMode_Attr(display, SLR_3DMODE_2D);//xpli reset 3D
					break;
				}

			}
			else if(ucGet3DMode >= SLR_3DMODE_3D_SBS  && ucGet3DMode <=SLR_3DMODE_3D_SENSIO)
			{
				switch(ucGet3DMode)
				{
					case SLR_3DMODE_3D_SBS:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_SIDE_BY_SIDE_HALF);
					break;
				      case SLR_3DMODE_3D_TB:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM);
					break;
					case SLR_3DMODE_3D_FP:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_FRAME_PACKING);
					break;
					// -------------- new 3D format
					case SLR_3DMODE_3D_LBL:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_LINE_ALTERNATIVE);
					break;
					case SLR_3DMODE_3D_VSTRIP:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_VERTICAL_STRIP);
					break;
					case SLR_3DMODE_3D_CKB:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_CHECKER_BOARD);
					break;
					case SLR_3DMODE_3D_REALID:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_REALID);
					break;
					case SLR_3DMODE_3D_SENSIO:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_SENSIO);
					break;
					// ---------------------------
					#if 0
					default:
						Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_2D_ONLY);
						Scaler_Set3DMode_Attr(display, SLR_3DMODE_2D);//xpli reset 3D
					break;
					#endif
				}
			}
			else
			{
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_2D_ONLY);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_2D);//xpli reset 3D
			}
			break;

		default:
				Scaler_Disp3dSetInfo(display, SLR_DISP_3D_3DTYPE,SLR_3D_2D_ONLY);
				Scaler_Set3DMode_Attr(display, SLR_3DMODE_2D);//xpli reset 3D
			break;
	}
	if((Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) == SLR_3D_2D_ONLY)
		|| ((Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) >= SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D) && (Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) <= SLR_3D_SENSIO_CVT_2D)))
	{
		Set_PANEL_3D_PIN(0);
	}else
		Set_PANEL_3D_PIN(1);
//~20120818 fix vo gatting[379]
	rtd_pr_vbe_info("** Update 3D mode/type=%d/%d \n",Scaler_Get3DMode(display), Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE));

}

/*
void modestate_HDMI_3D_format_auto_detection(void)
{
//20120818 fix vo gatting[379]
	unsigned char ucHdmi_Real3Dfmt = drvif_Hdmi_GetReal3DFomat(); // we get real 3D format without using fw 3D format
	UINT8 ucGet3DMode = Scaler_Get3DMode();
	rtd_pr_vbe_info("** Real3Dfmt =%d,src/3DMode/type =%d/%d/%d \n",ucHdmi_Real3Dfmt,Scaler_InputSrcGetMainChType(), Scaler_Get3DMode(), Scaler_Disp3dGetInfo(SLR_DISP_3D_3DTYPE));
	switch(ucHdmi_Real3Dfmt)
	{
		case HDMI3D_FRAME_PACKING:
			rtd_pr_vbe_info("FP 3D, Interlaced=%d\n", Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_INTERLACE));

			if(Scaler_Get3DMode()==SLR_3DMODE_3D_FP_CVT_2D || Scaler_Get3DMode()==SLR_3DMODE_3D_AUTO_CVT_2D)
			{
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_FRAME_PACKING_CVT_2D);
			}
			else
			{
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_FRAME_PACKING);
				Scaler_Set3DMode_Attr(SLR_3DMODE_3D_AUTO);
			}

#if 1//def ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
			if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo(SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING)
				&& ((Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE)))
			{
				if(drvif_scaler3d_decide_HDMI_framePacking_interlaced_status()){
					Scaler_DispSetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_INTERLACE, TRUE);
					rtd_pr_vbe_info("[HDMI][FP] change interlaced mode = 1\n");
				}
			}
#endif

			break;
		case HDMI3D_TOP_AND_BOTTOM:
			if(Scaler_Get3DMode()==SLR_3DMODE_3D_TB_CVT_2D || Scaler_Get3DMode()==SLR_3DMODE_3D_AUTO_CVT_2D)
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM_CVT_2D);
			else
			{
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM);
				Scaler_Set3DMode_Attr(SLR_3DMODE_3D_AUTO);
			}
			break;
		case HDMI3D_SIDE_BY_SIDE_FULL:
		case HDMI3D_SIDE_BY_SIDE_HALF:
			if(Scaler_Get3DMode()==SLR_3DMODE_3D_SBS_CVT_2D || Scaler_Get3DMode()==SLR_3DMODE_3D_AUTO_CVT_2D)
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D);
			else
			{
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_SIDE_BY_SIDE_HALF);
				Scaler_Set3DMode_Attr(SLR_3DMODE_3D_AUTO);
			}
			break;
		case HDMI3D_2D_ONLY:
			 if(ucGet3DMode == SLR_3DMODE_2D_CVT_3D)
			{
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_2D_CVT_3D);
			}
			else if(ucGet3DMode >= SLR_3DMODE_3D_SBS_CVT_2D  && ucGet3DMode <=SLR_3DMODE_3D_SENSIO_CVT_2D)
			{

				switch(ucGet3DMode)
				{
					case SLR_3DMODE_3D_SBS_CVT_2D:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D);
					break;
				      case SLR_3DMODE_3D_TB_CVT_2D:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM_CVT_2D);
					break;
					case SLR_3DMODE_3D_FP_CVT_2D:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_FRAME_PACKING_CVT_2D);
					break;
					// -------------- new 3D format
					case SLR_3DMODE_3D_LBL_CVT_2D:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_LINE_ALTERNATIVE_CVT_2D);
					break;
					case SLR_3DMODE_3D_VSTRIP_CVT_2D:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_VERTICAL_STRIP_CVT_2D);
					break;
					case SLR_3DMODE_3D_CKB_CVT_2D:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_CHECKER_BOARD_CVT_2D);
					break;
					case SLR_3DMODE_3D_REALID_CVT_2D:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_REALID_CVT_2D);
					break;
					case SLR_3DMODE_3D_SENSIO_CVT_2D:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_SENSIO_CVT_2D);
					break;
					// -------------- --------------
					default:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_2D_ONLY);
						Scaler_Set3DMode_Attr(SLR_3DMODE_2D);//xpli reset 3D
					break;
				}

			}
			else if(ucGet3DMode >= SLR_3DMODE_3D_SBS  && ucGet3DMode <=SLR_3DMODE_3D_SENSIO)
			{
				switch(ucGet3DMode)
				{
					case SLR_3DMODE_3D_SBS:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_SIDE_BY_SIDE_HALF);
					break;
				      case SLR_3DMODE_3D_TB:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_TOP_AND_BOTTOM);
					break;
					case SLR_3DMODE_3D_FP:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_FRAME_PACKING);
					break;
					// -------------- new 3D format
					case SLR_3DMODE_3D_LBL:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_LINE_ALTERNATIVE);
					break;
					case SLR_3DMODE_3D_VSTRIP:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_VERTICAL_STRIP);
					break;
					case SLR_3DMODE_3D_CKB:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_CHECKER_BOARD);
					break;
					case SLR_3DMODE_3D_REALID:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_REALID);
					break;
					case SLR_3DMODE_3D_SENSIO:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_SENSIO);
					break;
					// ---------------------------
					default:
						Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_2D_ONLY);
						Scaler_Set3DMode_Attr(SLR_3DMODE_2D);//xpli reset 3D
					break;
				}
			}
			else
			{
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_2D_ONLY);
				Scaler_Set3DMode_Attr(SLR_3DMODE_2D);//xpli reset 3D
			}
			break;

		default:
				Scaler_Disp3dSetInfo(SLR_DISP_3D_3DTYPE,SLR_3D_2D_ONLY);
				Scaler_Set3DMode_Attr(SLR_3DMODE_2D);//xpli reset 3D
			break;
	}
	if((Scaler_Disp3dGetInfo(SLR_DISP_3D_3DTYPE) == SLR_3D_2D_ONLY)
		|| ((Scaler_Disp3dGetInfo(SLR_DISP_3D_3DTYPE) >= SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D) && (Scaler_Disp3dGetInfo(SLR_DISP_3D_3DTYPE) <= SLR_3D_SENSIO_CVT_2D)))
	{
		Set_PANEL_3D_PIN(0);
	}else
		Set_PANEL_3D_PIN(1);
//~20120818 fix vo gatting[379]
	rtd_pr_vbe_info("** Update 3D mode/type=%d/%d \n",Scaler_Get3DMode(), Scaler_Disp3dGetInfo(SLR_DISP_3D_3DTYPE));
}
*/

static unsigned char enable_3d_mode = FALSE;
static unsigned char enable_3d_overscan = FALSE;
static unsigned int    hstart_input_3d_overscan = 0;
static unsigned int    vstart_input_3d_overscan = 0;
static unsigned int    width_input_3d_overscan = 1280;
static unsigned int    length_input_3d_overscan = 720;
static unsigned int    hstart_output_3d_overscan = 0;
static unsigned int    vstart_output_3d_overscan = 0;
static unsigned int    width_output_3d_overscan = 1920;
static unsigned int    length_output_3d_overscan = 1080;
static unsigned int   enable_field_alternative_3d_mode= FALSE;
static unsigned int   enable_line_alternative_3d_mode= FALSE;



unsigned char get_3D_mode_enable(void)
{
	return enable_3d_mode;
}

void set_3D_mode_enable(unsigned char enable)
{
	enable_3d_mode = enable;
}

unsigned char get_3D_overscan_enable(void)
{
	return enable_3d_overscan;
}

void set_3D_overscan_enable(unsigned char enable)
{
	enable_3d_overscan = enable;
}

unsigned int get_3D_overscan_input_hsta(void)
{
	return hstart_input_3d_overscan;
}

void set_3D_overscan_input_hsta(unsigned int hsta)
{
	hstart_input_3d_overscan = hsta;
}

unsigned int get_3D_overscan_input_vsta(void)
{
	return vstart_input_3d_overscan;
}

void set_3D_overscan_input_vsta(unsigned int vsta)
{
	vstart_input_3d_overscan = vsta;
}

unsigned int get_3D_overscan_input_wid(void)
{
	return width_input_3d_overscan;
}

void set_3D_overscan_input_wid(unsigned int wid)
{
	width_input_3d_overscan = wid;
}

unsigned int get_3D_overscan_input_len(void)
{
	return length_input_3d_overscan;
}

void set_3D_overscan_input_len(unsigned int len)
{
	length_input_3d_overscan = len;
}

unsigned int get_3D_overscan_output_hsta(void)
{
	return hstart_output_3d_overscan;
}

void set_3D_overscan_output_hsta(unsigned int hsta)
{
	hstart_output_3d_overscan = hsta;
}

unsigned int get_3D_overscan_output_vsta(void)
{
	return vstart_output_3d_overscan;
}

void set_3D_overscan_output_vsta(unsigned int vsta)
{
	vstart_output_3d_overscan = vsta;
}

unsigned int get_3D_overscan_output_wid(void)
{
	return width_output_3d_overscan;
}

void set_3D_overscan_output_wid(unsigned int wid)
{
	width_output_3d_overscan = wid;
}

unsigned int get_3D_overscan_output_len(void)
{
	return length_output_3d_overscan;
}

void set_3D_overscan_output_len(unsigned int len)
{
	length_output_3d_overscan = len;
}

unsigned int get_field_alternative_3d_mode_enable(void)
{
	return enable_field_alternative_3d_mode;
}

void set_field_alternative_3d_mode_enable(unsigned int enable)
{
	enable_field_alternative_3d_mode = enable;
}

unsigned int get_line_alternative_3d_mode_enable(void)
{
	return enable_line_alternative_3d_mode;
}

void set_line_alternative_3d_mode_enable(unsigned int enable)
{
	enable_line_alternative_3d_mode = enable;
}


void check_3D_overscan_size_error(void)
{
	unsigned int input_wid = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);//Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
	unsigned int input_len = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);//Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN);
	unsigned int disp_len = Get_DISP_ACT_END_VPOS() - Get_DISP_ACT_STA_VPOS();
	unsigned int disp_wid = Get_DISP_ACT_END_HPOS() - Get_DISP_ACT_STA_HPOS();

	if(get_3D_overscan_input_wid() + get_3D_overscan_input_hsta() > input_wid)
	{
		set_3D_overscan_input_wid(input_wid - get_3D_overscan_input_hsta());
		rtd_pr_vbe_debug("3d overscan input_hsize can't be >INPUT_WID\n");
	}
	if(get_3D_overscan_input_len() + get_3D_overscan_input_vsta() > input_len)
	{
		set_3D_overscan_input_len(input_len - get_3D_overscan_input_vsta());
		rtd_pr_vbe_debug("3d overscan input_vsize can't be >INPUT_LEN\n");
	}

	if(get_3D_overscan_output_wid() > disp_wid)
	{
		rtd_pr_vbe_debug("3d overscan disp_hsize>DISP_WID \n");
		set_3D_overscan_output_wid(disp_wid);

	}

	if(get_3D_overscan_output_len() > disp_len)
	{
		rtd_pr_vbe_debug("3d overscan disp_vsize>DISP_LEN \n");
		set_3D_overscan_output_wid(disp_len);
	}
}
#endif

/*
unsigned char Rt_Sleep(unsigned int  milliSec)
{
#ifdef WIN32
	Sleep(milliSec);
	return TRUE;
#else
    int retVal;

    // nanosleep() appears to be thread friendly and doesn't block the entire process.
    struct timespec Required, Remain;

    // fill out timespec structure.
    Required.tv_sec = (long) (milliSec/1000);

    // range from 0 ~ 999,999,999
    Required.tv_nsec = (milliSec - Required.tv_sec*1000) *1000*1000;

    retVal = nanosleep(&Required, &Remain);

    if (retVal != 0)
        return FALSE;
    else
        return TRUE;
#endif
}
*/


long Scaler_Kernel_SendRPC_to_Video(unsigned int rpc_cmd, unsigned int param_1)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long ret;

	if(send_rpc_command(RPC_VIDEO, VIDEO_RPC_ToAgent_ScalerDrv, param_1, rpc_cmd, &ret)){
		rtd_pr_vbe_debug("RPC fail!!\n");
		return _FALSE;
	}
#endif
	return _TRUE;
}


#ifdef CONFIG_ENABLE_3D_SETTING
void modestate_decide3D_for_vdc(void)
{
	//if ((Scaler_InputSrcGetMainChFrom() != _SRC_FROM_VDC) /*|| (GET_DRAGON_IC_TYPE() ==  _DRAGON_IC_DRAGON_P)*/)
	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_AVD)
		return;

	if (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY) {
		Scaler_Set3DEnable(TRUE);
		//MODESTATE_PRINTF( "1.display = %x, Scaler_Get3DEnable() = %x in modestate_decide3D_for_vdc\n", Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), Scaler_Get3DEnable());
	} else {
		if ((Scaler_PipGetInfo(SLR_PIP_TYPE) >= SUBTYPE_PIPSMALL) && (Scaler_PipGetInfo(SLR_PIP_TYPE) <= SUBTYPE_PIPLARGE))
        {
			//Scaler_Set3DEnable(FALSE);
			Scaler_Set3DEnable(TRUE);
			//MODESTATE_PRINTF( "2.display = %x, Scaler_Get3DEnable() = %x in modestate_decide3D_for_vdc\n", Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), Scaler_Get3DEnable());
		}
        else
        {
			Scaler_Set3DEnable(TRUE);
		//MODESTATE_PRINTF( "3.display = %x, Scaler_Get3DEnable() = %x in modestate_decide3D_for_vdc\n", Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), Scaler_Get3DEnable());
	}
	}
}
#endif //#ifdef CONFIG_ENABLE_3D_SETTING

static unsigned char Scaler_FRC_Open_Fix_Lastline_2_Flag = _TRUE;

void Scaler_Set_FRC_Open_Fix_Lastline_2(unsigned char ucEnable)
{
	Scaler_FRC_Open_Fix_Lastline_2_Flag = ucEnable;
}

unsigned char Scaler_Get_FRC_Open_Fix_Lastline_2(void)
{
	return Scaler_FRC_Open_Fix_Lastline_2_Flag;
}
unsigned char Scaler_EnterFRC_LastLine_Condition(void)
{
	if(_FALSE == Scaler_FRC_Open_Fix_Lastline_2_Flag)
		return _FALSE;
	return _TRUE;
}
#endif

static unsigned char	Scaler_FreeRun_To_FrameSync_By_HW_Enable = _DISABLE;
static unsigned char	Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp = _DISABLE;
static unsigned int	Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_TimeOut = 100;
static unsigned char	Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_Porch = 40;
static unsigned char	Scaler_FreeRun_To_FrameSync_By_HW_LineBufferMode_Enable = _ENABLE;
static unsigned int	Scaler_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut = 100;
static unsigned int	Scaler_FreeRun_To_FrameSync_By_HW_Delay = 5;
static unsigned char	Scaler_FreeRun_To_FrameSync_By_HW_FastMode = _DISABLE;
static unsigned char	Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2 = _DISABLE;
static unsigned char	Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3 = _DISABLE;
static unsigned char	Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL4 = _ENABLE;
static unsigned char	Scaler_FreeRun_To_FrameSync_By_I2D_CTRL = _DISABLE;
static unsigned char	Scaler_FreeRun_To_FrameSync_By_Reset_VODMA = _DISABLE;
static unsigned char	Scaler_FreeRun_To_FrameSync_Reset_VODMA_Flag = _DISABLE;


//USER:LewisLee DATE:2012/07/06
//let FRC -> Framesync switch by HW
//to reduce panel jitter
void Scaler_Set_FreeRun_To_FrameSync_By_HW_Enable(unsigned char ucEnable)
{
	Scaler_FreeRun_To_FrameSync_By_HW_Enable = ucEnable;
}

unsigned char Scaler_Get_FreeRun_To_FrameSync_By_HW_Enable(void)
{
#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
	//return Scaler_FreeRun_To_FrameSync_By_HW_Enable;
	return _TRUE;
#else //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
	return _DISABLE;
#endif //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
}
//USER : LewusLee DATE:2012/08/30
//for novatek Tcon IC, too easy to enter burn in mode
//sw we need to set FRC -> Framesync moer smooth
void Scaler_Set_FreeRun_To_FrameSync_By_HW_LineBufferMode_Enable(unsigned char ucEnable)
{
	Scaler_FreeRun_To_FrameSync_By_HW_LineBufferMode_Enable = ucEnable;
}

unsigned char Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_Enable(void)
{
#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
	if(_DISABLE == Scaler_FreeRun_To_FrameSync_By_HW_Enable)
		return _DISABLE;

	return Scaler_FreeRun_To_FrameSync_By_HW_LineBufferMode_Enable;
#else //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
	return _DISABLE;
#endif //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
}

void Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2(unsigned char ucEnable)
{
	Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2 = ucEnable;
}

unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2(void)
{
	return Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2;
}

void Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3(unsigned char ucEnable)
{
	Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3 = ucEnable;
}

unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3(void)
{
	return Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3;
}

void Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL4(unsigned char ucEnable)
{
	Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL4 = ucEnable;
}

unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL4(void)
{
	return Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL4;
}

void Scaler_Set_FreeRun_To_FrameSync_By_Reset_VODMA(unsigned int ucEnable)
{   
    
	Scaler_FreeRun_To_FrameSync_By_Reset_VODMA= ucEnable;
}

unsigned int Scaler_Get_FreeRun_To_FrameSync_By_Reset_VODMA(void)
{
	return Scaler_FreeRun_To_FrameSync_By_Reset_VODMA;
}

void Scaler_Set_Reset_VODMA_Flag(unsigned int ucEnable)
{
	Scaler_FreeRun_To_FrameSync_Reset_VODMA_Flag= ucEnable;
}

unsigned int Scaler_Get_Reset_VODMA_Flag(void)
{
	return Scaler_FreeRun_To_FrameSync_Reset_VODMA_Flag;
}

void Scaler_Set_FreeRun_To_FrameSync_By_I2D_CTRL(unsigned char ucEnable)
{
	Scaler_FreeRun_To_FrameSync_By_I2D_CTRL = ucEnable;
}

unsigned int Scaler_Get_FreeRun_To_FrameSync_By_I2D_CTRL(void)
{
	return Scaler_FreeRun_To_FrameSync_By_I2D_CTRL;
}

void Scaler_Set_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut(unsigned int Value)
{
	Scaler_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut = Value;
}

unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut(void)
{
	return Scaler_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut;
}
void Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp(unsigned char ucEnable)
{
	Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp = ucEnable;
}

unsigned char Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp(void)
{
#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
	return TRUE;//Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp;
#else //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
	return _DISABLE;
#endif //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
}

void Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_TimeOut(unsigned int Value)
{
	Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_TimeOut = Value;
}

unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_TimeOut(void)
{
	return Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_TimeOut;
}

void Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_Porch(unsigned char Value)
{
	Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_Porch = Value;
}

unsigned char Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_Porch(void)
{
	return Scaler_FreeRun_To_FrameSync_By_HW_SpeedUp_Porch;
}
//USER:LewisLee DATE:2012/12/04
//start FRC2Framesync by HW, need wait some time, and start framesync bit
void Scaler_Set_FreeRun_To_FrameSync_By_HW_Delay(unsigned int delay)
{
	Scaler_FreeRun_To_FrameSync_By_HW_Delay = delay;
}

unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_Delay(void)
{
	return Scaler_FreeRun_To_FrameSync_By_HW_Delay;
}
//USER:LewisLee DATE:2012/08/24
//if Enable it => FS -> FLL, lock will more fast
//if Disable it => FLL, lock will more smooth
void Scaler_Set_FreeRun_To_FrameSync_By_HW_FastMode(unsigned int Value)
{
	Scaler_FreeRun_To_FrameSync_By_HW_FastMode = Value;
}

unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_FastMode(void)
{
	return Scaler_FreeRun_To_FrameSync_By_HW_FastMode;
}

DIRECT_VO_FRAME_ORIENTATION main_VoRotateMode = DIRECT_VO_FRAME_ORIENTATION_DEFAULT; //this is the current rotate mode for scaler control
DIRECT_VO_FRAME_ORIENTATION main_oriRotateMode = DIRECT_VO_FRAME_ORIENTATION_DEFAULT; //this is the last rotate mode for scaler control

void set_rotate_mode(unsigned char display, DIRECT_VO_FRAME_ORIENTATION value)
{
	if(display == SLR_MAIN_DISPLAY)
		main_VoRotateMode=value;
	return;
}

DIRECT_VO_FRAME_ORIENTATION get_rotate_mode(unsigned char display)
{
	if(display == SLR_MAIN_DISPLAY)
		return main_VoRotateMode;
	else
		return 0;
}

void set_ori_rotate_mode(unsigned char display, DIRECT_VO_FRAME_ORIENTATION value)
{
	if(display == SLR_MAIN_DISPLAY)
		main_oriRotateMode=value;
	return;
}

DIRECT_VO_FRAME_ORIENTATION get_ori_rotate_mode(unsigned char display)
{
	if(display == SLR_MAIN_DISPLAY)
		return main_oriRotateMode;
	else
		return 0;
}

#ifndef BUILD_QUICK_SHOW 

void Scaler_Set_VODMA_Tracking_I3DDMA(unsigned int ucEnable)
{
	unsigned int *vo_track_i3_enable_addr = NULL;
	vo_track_i3_enable_addr = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_VO_TRACK_I3_ENABLE);
	if(vo_track_i3_enable_addr)
		*vo_track_i3_enable_addr = htonl(ucEnable);
	rtd_pr_vbe_notice("[%s] ucEnable = %d \n", __FUNCTION__, ucEnable);
}

unsigned int Scaler_Get_VODMA_Tracking_I3DDMA(void)
{
	unsigned int *vo_track_i3_enable_addr = NULL;
	vo_track_i3_enable_addr = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_VO_TRACK_I3_ENABLE);
	return (vo_track_i3_enable_addr? htonl(*vo_track_i3_enable_addr): 0);
}

// This api is to get the vo tracking i3 m_stage
unsigned int Scaler_Get_VODMA_Tracking_I3DDMA_M_STAGE(void)
{
	unsigned int *vo_track_i3_m_stage_addr = NULL;
	vo_track_i3_m_stage_addr = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_VO_TRACK_I3_M_STAGE);
	return (vo_track_i3_m_stage_addr? htonl(*vo_track_i3_m_stage_addr): 0);
}

unsigned int Scaler_Get_VO_Ivs_SrcInfo(void)
{
	unsigned int *vo_ivs_src_info_addr = NULL;
	vo_ivs_src_info_addr = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_VO_IVS_SRC_INFO);
	return (vo_ivs_src_info_addr? htonl(*vo_ivs_src_info_addr): 0);
}

void Scaler_Set_Record_I3DDMA_Capture_Index(unsigned int capture_index)
{
	unsigned int *i3_capture_index_addr = NULL;
	i3_capture_index_addr = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_I3_CAPTURE_INDEX);

	if(i3_capture_index_addr)
		*i3_capture_index_addr = htonl(capture_index);
	//rtd_pr_vbe_notice("[%s] ucEnable = %d \n", __FUNCTION__, ucEnable);
}

#ifdef CONFIG_ENABLE_SE_ROTATE_M_DOMAIN_BLOCK_MODE
//static ROTATE_MODE_LIST HDMIRotateMode = ROTATE_MODE_0;
unsigned int rotatedisconnecten = 0;

#include <vo/rtk_vo.h>

void scaler_Rotate_VI_init_send_rpc(void)
{
	SCALER_ROTATE_INFO *rotate_data;
	//unsigned int ulCount = 0;//, i = 0;
	int ret;

	rtd_pr_vbe_emerg("[VI RPC] Init ...\n");
	rtd_pr_vbe_emerg("[crixus]###############%s~~1 \n",__FUNCTION__);

	rotate_data = (SCALER_ROTATE_INFO *)Scaler_GetShareMemVirAddr(SCALERIOC_SEND_ROTATE_INFO_VI_BLOCK_READ);
	//ulCount = sizeof(SCALER_ROTATE_INFO) / sizeof(unsigned int);

	//rotate_data->width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);//scaler_MaincvtSub_get_vgip2IhWidth();
	//rotate_data->height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);//scaler_MaincvtSub_get_vgip2IvHeight();
	rotate_data->width = Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID);
	rotate_data->height = Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN);
	rotate_data->rotatemode =  get_SE_rotate_mode(SLR_MAIN_DISPLAY);
	rotate_data->disconnecten = rotatedisconnecten;
	rotate_data->disp_wid = _DISP_WID;
	rotate_data->disp_len = _DISP_LEN;
    rotate_data->vi_vflip_en = get_vi_vflip_flag(SLR_MAIN_DISPLAY);

	//if(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)_CHANNEL1, SLR_DISP_INTERLACE))//scaler_MaincvtSub_get_vgip2SrcInterlaced())
		//rotate_data->height *= 2;

	rtd_pr_vbe_emerg("[crixus]rotate_data->width = %d\n", rotate_data->width);
	rtd_pr_vbe_emerg("[crixus]rotate_data->height = %d\n", rotate_data->height);
	rtd_pr_vbe_emerg("[crixus]rotate_data->rotatemode = %d\n", rotate_data->rotatemode);
	rtd_pr_vbe_emerg("[crixus]rotate_data->disconnecten = %d\n", rotate_data->disconnecten);
	rtd_pr_vbe_emerg("[crixus]rotate_data->disp_wid = %d\n", rotate_data->disp_wid);
	rtd_pr_vbe_emerg("[crixus]rotate_data->disp_len = %d\n", rotate_data->disp_len);
    rtd_pr_vbe_emerg("[crixus]rotate_data->vi_vflip_en = %d\n", rotate_data->vi_vflip_en);

	//change endian
	rotate_data->width = htonl(rotate_data->width);
	rotate_data->height  = htonl(rotate_data->height);
	rotate_data->rotatemode = htonl(rotate_data->rotatemode);
	rotate_data->disconnecten =htonl(rotate_data->disconnecten);
	rotate_data->disp_wid = htonl(rotate_data->disp_wid);
	rotate_data->disp_len =htonl(rotate_data->disp_len);
    rotate_data->vi_vflip_en =htonl(rotate_data->vi_vflip_en);
	if(rotate_data->width != 0 && rotate_data->height != 0){
		if (0 != (ret = Scaler_SendRPC(SCALERIOC_SEND_ROTATE_INFO_VI_BLOCK_READ,0,0))){
			rtd_pr_vbe_emerg("ret=%d, Deinit VI ROTATE fail !!!\n", ret);
		}
	
		rtd_pr_vbe_emerg("[Rotate][VI][RPC] Init done!!\n");
	} else {
		rtd_pr_vbe_emerg("[Rotate][VI][RPC] Init cancel!!\n");
	}
 	return;
}
#if 0
void Set_Rotate_Mode(unsigned char display, ROTATE_MODE_LIST value)
{
	HDMIRotateMode = value;
}

ROTATE_MODE_LIST Get_Rotate_Mode(unsigned char display)
{
	return HDMIRotateMode;
}
#endif

unsigned char main_rotate_stretch_function = 0;
MAINSUB_ROTATE_STATUS main_rotateStatus;
void Set_rotate_function(unsigned char display, unsigned char enable)
{
	if(display == SLR_MAIN_DISPLAY)
		main_rotate_stretch_function=enable;
}
unsigned char Get_rotate_function(unsigned char display)
{
	if(display == SLR_MAIN_DISPLAY)
		return main_rotate_stretch_function;
	else
		return 0;
}
void set_rotate_curState(unsigned char display, MAINSUB_ROTATE_STATUS value)
{
	if(display == SLR_MAIN_DISPLAY)
		main_rotateStatus = value;
}

MAINSUB_ROTATE_STATUS get_rotate_curState(unsigned char display)
{
	if(display == SLR_MAIN_DISPLAY)
		return main_rotateStatus;
	else
		return 0;
}

void VI_close(void)
{
	//close vi quickly for isr happen
	mdomain_vi_main_inten_RBUS vi_inten_reg; //MDOM_VI_INTEN_VADDR
	mdomain_vi_main_intst_RBUS vi_intst_reg; //MDOM_VI_INTST_VADDR
	mdomain_vi_main_gctl_RBUS vi_gctl_reg;//vsce1 //MDOM_VI_GCTL_VADDR
	mdomain_vi_main_dmactl_RBUS vi_dmactl_reg;//dmaen  //MDOM_VI_DMACTL_VADDR
	vi_dmactl_reg.regValue = rtd_inl(MDOMAIN_VI_MAIN_DMACTL_reg);
	vi_dmactl_reg.dmaen1 = 0;
	rtd_outl(MDOMAIN_VI_MAIN_DMACTL_reg, vi_dmactl_reg.regValue);

	vi_gctl_reg.regValue = rtd_inl(MDOMAIN_VI_MAIN_GCTL_reg);
	vi_gctl_reg.vsce1 = 0;
	rtd_outl(MDOMAIN_VI_MAIN_GCTL_reg, vi_gctl_reg.regValue);

	vi_inten_reg.regValue = rtd_inl(MDOMAIN_VI_MAIN_INTEN_reg);
	#if 0//m6 spec removed
	vi_inten_reg.dmaerrinte = 0;
	#endif
	vi_inten_reg.video_in_last_wr_y_flag_ie = 0;
	vi_inten_reg.video_in_last_wr_c_flag_ie = 0;
	vi_inten_reg.vsinte1 = 0;
	rtd_outl(MDOMAIN_VI_MAIN_INTEN_reg, vi_inten_reg.regValue);

	vi_intst_reg.regValue = rtd_inl(MDOMAIN_VI_MAIN_INTST_reg);
	#if 0//m6 spec removed
	vi_intst_reg.dmaerrints = 1;
	#endif
	vi_intst_reg.video_in_last_wr_y_flag = 1;
	vi_intst_reg.video_in_last_wr_c_flag = 1;
	vi_intst_reg.vsints1 = 1;
	rtd_outl(MDOMAIN_VI_MAIN_INTST_reg, vi_intst_reg.regValue);
}

void Scaler_HDMI_Rotate(char rotateon, ROTATE_MODE_LIST mode, unsigned char srcIdx)
{
	//int srcIdx;
	if(mode < ROTATE_MODE_0||mode > ROTATE_MODE_270)
		return;

	if(rotateon==TRUE){
		rtd_pr_vbe_emerg("[rotate debug]rotateon:%d;;mode:%d\n", rotateon, mode);
		//pthread_mutex_lock(&scalerMutex);

		//disablse M-domain VI & disable block read
		//VI_close();
		//drvif_memory_block_mode_enable(FALSE);

		//drvif_scaler_vactive_end_irq(TRUE, _CHANNEL1); // enable main path VGIP ISR
		//drvif_scaler_vactive_sta_irq(TRUE, _CHANNEL1);
		//drvif_scaler_ddomain_switch_irq(_ENABLE);//Enable D-domain ISR

		//Set_rotate_function(SLR_MAIN_DISPLAY, TRUE);
		set_rotate_curState(SLR_MAIN_DISPLAY, MAIN_ROTATE_INIT_UZD);//ready to initial & set UZD
		//Set_Rotate_Mode(SLR_MAIN_DISPLAY, (ROTATE_MODE_LIST)mode);

		//Scaler_DispSetInputInfo(SLR_INPUT_STATE, _MODE_STATE_SEARCH);

		//pthread_mutex_unlock(&scalerMutex);
	}
#if 0
	else{
		rtd_pr_vbe_emerg("[rotate debug]rotateon:%d;;mode:%d\n", rotateon, mode);
		if(Get_rotate_function(SLR_MAIN_DISPLAY)==TRUE){
			//disablse VI & disable block read
			VI_close();
			drvif_memory_block_mode_enable(FALSE);
			//Scaler_VoStop();
		}
		//Set_rotate_function(SLR_MAIN_DISPLAY, FALSE);
		set_rotate_curState(SLR_MAIN_DISPLAY, MAIN_ROTATE_NON_ACTIVE);
		Set_Rotate_Mode(SLR_MAIN_DISPLAY, ROTATE_MODE_0);

		Scaler_DispSetInputInfo(SLR_INPUT_STATE, _MODE_STATE_SEARCH);
	}
#endif
}

unsigned int scaler_send_rotate_vflip_rpc(unsigned enable)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result =0;
#endif
	VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE *config_data;
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE), GFP_DCU1_LIMIT, (void**)&vir_addr_noncache);
	//phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);
	config_data = (VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE *)vir_addr_noncache;
	config_data->videoPlane = VO_VIDEO_PLANE_V1;
	config_data->enable =enable;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_ConfigVodmaVFlip,phy_addr, 0, &result))
	{
		rtd_pr_vbe_debug("RPC fail!!\n");
		dvr_free((void *)vir_addr);
		return FALSE;
	}
#endif
	dvr_free((void *)vir_addr);
	//drvif_scaler3d_set_panel_info(); //send panel info to video fw
	return TRUE;
}

DIRECT_VO_FRAME_ORIENTATION get_SE_rotate_mode(unsigned char display)
{
    DIRECT_VO_FRAME_ORIENTATION se_mode = get_rotate_mode(display);

    if (Get_PANEL_VFLIP_ENABLE())
    {
        if ((get_rotate_mode(display) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_90) && (Scaler_InputSrcGetMainChType() == _SRC_VO))
        {
            // VO vflip + SE rotate 270 + SFG hflip + VflipPanel = rotate 90
            se_mode =  DIRECT_VO_FRAME_ORIENTATION_ROTATE_270;
        } else if ((get_rotate_mode(display) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_270) && (Scaler_InputSrcGetMainChType() == _SRC_VO))
        {
            // VO vflip + SE rotate 90 + SFG hflip + VflipPanel = rotate 270
            se_mode =  DIRECT_VO_FRAME_ORIENTATION_ROTATE_90;
        }
    }

    return se_mode;
}

#endif
unsigned char get_vo_vflip_flag(unsigned char display)
{
    if (Scaler_InputSrcGetMainChType() != _SRC_VO)
    {
        return FALSE;
    }

    if (display == SLR_MAIN_DISPLAY)
    {
        if(Get_PANEL_VFLIP_ENABLE())
        {
            if ((get_rotate_mode(display) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_0)
                || (get_rotate_mode(display) == DIRECT_VO_FRAME_ORIENTATION_DEFAULT)
                || (get_rotate_mode(display) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_90)
                || (get_rotate_mode(display) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_270)
                )
            {
                //VO vflip + SE rotate 0/90/270 + SFG hflip + VflipPanel = rotate 0/90/270
                return TRUE;
            }
            
        } else {
            if (get_rotate_mode(display) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_180)
            {
                if (get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1)
                {
                    //VO vflip + 2Pto1L hflip = rotate 180
                    return TRUE;
                } else if (!Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display, SLR_DISP_INTERLACE))
                {
                    //VO vflip + RTNR hflip = rotate 180
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

unsigned char get_vi_vflip_flag(unsigned char display)
{
    //If VO can be used for vflip effect, do not use VI
    if (Scaler_InputSrcGetMainChType() == _SRC_VO)
    {
        return FALSE;
    }

    if(Get_PANEL_VFLIP_ENABLE())
    {
        if((get_rotate_mode(display) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_90) || (get_rotate_mode(display) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_270))
        {
            //SE rotate 90/270 + VI vflip + SFG hflip + VflipPanel = rotate 90/270
            return TRUE;
        }
    }

    return FALSE;
}

unsigned char get_mdomain_vflip_flag(unsigned char display)
{
    // If VO can be used for vflip effect, do not use Mdomain
    if (Scaler_InputSrcGetMainChType() == _SRC_VO)
    {
        return FALSE;
    }

    if (display == SLR_MAIN_DISPLAY)
    {
        if(Get_PANEL_VFLIP_ENABLE())
        {
            if ((get_rotate_mode(display) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_0)
                || (get_rotate_mode(display) == DIRECT_VO_FRAME_ORIENTATION_DEFAULT))
            {
                // Mdomain vflip + SFG hflip + VflipPanel = rotate 0
                return TRUE;
            }
        } else {
            if (get_rotate_mode(display) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_180)
            {
                 if (get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1)
                {
                    // 2Pto1L hflip + Mdomain vflip = rotate 180
                    return TRUE;
                } 
            }
        }
    }

    return FALSE;
}

unsigned char get_rtnr_hflip_flag(void)
{
    // 4k120 and interlace case, bypass rtnr
    if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE) || (is_4k_hfr_mode(SLR_MAIN_DISPLAY)))
    {
        return FALSE;
    }

    if (get_rotate_mode(SLR_MAIN_DISPLAY) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_180)
    {
        if(Get_PANEL_VFLIP_ENABLE())
        {
            // RTNR hflip + SFG hflip + VflipPanel = rotate 180
            return TRUE;
        } else if (Scaler_InputSrcGetMainChType() == _SRC_VO)
        {
            // VO vflip + RTNR hflip = rotate 180
            return TRUE;
        }
    }

    return FALSE;
}

unsigned char get_2Pto1L_hflip_flag(void)
{
    if (is_4k_hfr_mode(SLR_MAIN_DISPLAY) == FALSE)
    {
        return FALSE;
    }

    if (get_rotate_mode(SLR_MAIN_DISPLAY) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_180)
    {
        return TRUE;
    }

    return FALSE;
}

#endif 

unsigned char Scaler_get_realcinema_mode_condition(void)
{
	if((Scaler_MEMC_GetMotionType() == VPQ_MEMC_TYPE_55_PULLDOWN) &&
		(fwif_color_get_cinema_mode_en() == 1)){ //tru-motion off + real cinema
		return TRUE;
	}
	else
		return FALSE;
}

void Scaler_Set_Cinema_Mode(unsigned int cinema_mode_enable, unsigned int run_vo_cinema)
{
#ifndef BUILD_QUICK_SHOW
	unsigned int *pulTemp = NULL;
	unsigned int i=0, ulItemCount = 0;
	CINEMA_MODE_INFO cinema_info;
	rtd_pr_vbe_emerg("Scaler_Set_Cinema_Mode enable=%d runVo=%d\n", cinema_mode_enable, run_vo_cinema);

	pulTemp = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_CINEMA_MODE_ENABLE);
	ulItemCount = sizeof(CINEMA_MODE_INFO) /  sizeof(unsigned int);

	cinema_info.cinema_mode_enable = cinema_mode_enable;
	cinema_info.run_vo_cinema = run_vo_cinema;
	if (pulTemp) {
		memcpy(pulTemp, &cinema_info, sizeof(CINEMA_MODE_INFO));
		for (i = 0; i < ulItemCount; i++)
		{
			pulTemp[i] = htonl(pulTemp[i]);

		}
		if (0 != Scaler_SendRPC(SCALERIOC_CINEMA_MODE_ENABLE,0,0))
		{
			rtd_pr_vbe_debug("send driver config info fail !!!\n");
			return;

		}
	}
#endif
	return;
}


void modestate_decide_dtg_m_mode(void)
{
	rtd_pr_vbe_emerg("modestate_decide_dtg_m_mode => ");
	rtd_pr_vbe_emerg("pixel=%d, input=%d, ", Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE),
		Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ));

    rtd_pr_vbe_emerg("Get_DISPLAY_PANEL_TYPE => %d", Get_DISPLAY_PANEL_TYPE());

	if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)){
		Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		rtd_pr_vbe_notice("[VBY1_TO_HDMI]output=%d \n", Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ));
		return;
	}


	if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) &&
		(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245))	//24Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){//120 panel
			if(Get_DISPLAY_REFRESH_RATE() == 144){
                Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 144);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 144);
			}
			else{
				Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
			}
		}else{//60 panel
                        if(drv_memory_get_game_mode()==1){
                                Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
                                rtd_pr_vbe_notice("GameMode output = 60Hz\n");
                        }else{
                                if((Scaler_MEMC_GetMotionType() == VPQ_MEMC_TYPE_55_PULLDOWN) &&
                                   (fwif_color_get_cinema_mode_en() == 1)){ //tru-motion off + real cinema
                                        Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 48);
                                }else{
                                        if((vbe_disp_get_adaptivestream_fs_mode() == 1) && (0 == vbe_disp_get_adaptivestream_force_framerate()))
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 48);
				else
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
                                }
			}
		}
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 245) &&
		(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 255))	//25Hz
	{
	    /*
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 50);
		}else{
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 50);
		}*/
		if(Get_DISPLAY_REFRESH_RATE() == 144){
            Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 125);
		}
		else if(Get_DISPLAY_REFRESH_RATE() == 165){
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 150);
		}
		else{
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 50);
		}
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) &&
		(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305))	//30Hz
	{
	    /*
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}else{
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}
		*/
		if(Get_DISPLAY_REFRESH_RATE() == 144){
            Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 120);
		}
		else if(Get_DISPLAY_REFRESH_RATE() == 165){
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 150);
		}
		else{
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 475) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 485))	//48Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() == 144){
            Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 144);
		}
		else if(Get_DISPLAY_REFRESH_RATE() == 165){
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 144);
		}
		else{
        	Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 48);
		}
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 505))	//50Hz
	{
	    /*
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 50);
		}else{
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 50);
		}
		*/
		if(Get_DISPLAY_REFRESH_RATE() == 144){
            Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 100);
		}
		else if(Get_DISPLAY_REFRESH_RATE() == 165){
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 150);
		}
		else{
        	Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 50);
		}
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 595) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 605))	//60hz
	{
	    /*
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}else{
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}*/
		if(Get_DISPLAY_REFRESH_RATE() == 144){
            Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 120);
		}
		else if(Get_DISPLAY_REFRESH_RATE() == 165){
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 120);
		}
		else{
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}
	}else{ // others
	    /*
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}else{
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}*/
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 994) &&
				(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 1005)){	//100Hz
				if(Get_DISPLAY_REFRESH_RATE() == 144){
                    Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 100);
				}
				else if(Get_DISPLAY_REFRESH_RATE() == 165){
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 100);
				}
				else{
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 50);
				}
			}
			else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 1191) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 1206)){	//120Hz
				if(Get_DISPLAY_REFRESH_RATE() == 144){
                    Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 120);
				}
				else if(Get_DISPLAY_REFRESH_RATE() == 165){
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 120);
				}
				else{
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
				}
			}
			else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 1435) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 1445)){	//144Hz
                    Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 144);
			}
			else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 2395) &&
                (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 2405)){	//240Hz
                if(Get_DISPLAY_REFRESH_RATE() == 240)
                {
                    Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 240);
                }
                else
                {
                    Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
                }
            }
            else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 2878) &&
                (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 2898))
            {
                if(Get_DISPLAY_REFRESH_RATE() == 288)
                {
                    Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 288);
                }
                else
                {
                    Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
                }
            }
		}
		else
			Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
	}
	Set_decide_dtg_m_mode_set_cinema_mode_flag(true); // Move to vsc tsk due to sending rpc in another rpc(rpcSetMEMCBypass)
	rtd_pr_vbe_emerg("output=%d \n", Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ));

}

void Set_decide_dtg_m_mode_set_cinema_mode_flag(unsigned char enable)
{
    decide_dtg_m_mode_set_cinema_mode_flag = enable;
}

unsigned char Get_decide_dtg_m_mode_set_cinema_mode_flag()
{
    return decide_dtg_m_mode_set_cinema_mode_flag;
}

void modestate_decide_dtg_m_mode_scaler_set_cinema_mode()
{
    rtd_pr_vbe_notice("[%s] SLR_INPUT_DTG_MASTER_V_FREQ=%d", __FUNCTION__, Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ));
    if(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 48){
		Scaler_Set_Cinema_Mode(_ENABLE, _DISABLE);
	}
	else{
		Scaler_Set_Cinema_Mode(_DISABLE, _DISABLE);
	}
}

void modestate_decide_dtg_m_modeByDisp(unsigned char _channel)
{
	SCALER_DISP_CHANNEL channel = _channel;

	rtd_pr_vbe_debug("modestate_decide_dtg_m_mode ByDisp => ");
	rtd_pr_vbe_debug("pixel=%d, input=%d, ", Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE),
			  Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ));

	if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)){
		Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		rtd_pr_vbe_notice("[VBY1_TO_HDMI]output=%d \n", Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ));
		return;
	}

	if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 235) &&
			(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 245))	//24Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){//120 panel
			if(Get_DISPLAY_REFRESH_RATE() == 144){
                Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 144);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 144);
			}
			else{
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
			}
		}else{//60 panel
			if(drv_memory_get_game_mode()==1){
                                Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
                                rtd_pr_vbe_notice("GameMode output = 60Hz\n");
			}else{
				if((Scaler_MEMC_GetMotionType() == VPQ_MEMC_TYPE_55_PULLDOWN) &&
                                   (fwif_color_get_cinema_mode_en() == 1)){ //tru-motion off + real cinema
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 48);
				}else{
					if((vbe_disp_get_adaptivestream_fs_mode() == 1) && (0 == vbe_disp_get_adaptivestream_force_framerate()))
						Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 48);
					else
						Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
				}
			}
		}
	}else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 245) &&
			  (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 255))	//25Hz
	{
	    /*
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 50);
		}else{
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 50);
		}*/
		if(Get_DISPLAY_REFRESH_RATE() == 144){
            Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 125);
		}
		else if(Get_DISPLAY_REFRESH_RATE() == 165){
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 150);
		}
		else{
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 50);
		}
	}else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 295) &&
			  (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 305))	//30Hz
	{
	    /*
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}else{
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}*/
		if(Get_DISPLAY_REFRESH_RATE() == 144){
            Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 120);
		}
		else if(Get_DISPLAY_REFRESH_RATE() == 165){
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 150);
		}
		else{
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 475) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 485))	//48Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() == 144){
            Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 144);
		}
		else if(Get_DISPLAY_REFRESH_RATE() == 165){
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 144);
		}
		else{
        	Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 48);
		}
	}else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 495) &&
			  (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 505))	//50Hz
	{
	    /*
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 50);
		}else{
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 50);
		}*/
		if(Get_DISPLAY_REFRESH_RATE() == 144){
            Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 100);
		}
		else if(Get_DISPLAY_REFRESH_RATE() == 165){
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 150);
		}
		else{
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 50);
		}
	}else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 595) &&
			  (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) <= 605))	//60hz
	{
	    /*
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}else{
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}*/
		if(Get_DISPLAY_REFRESH_RATE() == 144){
            Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 120);
		}
		else if(Get_DISPLAY_REFRESH_RATE() == 165){
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 120);
		}
		else{
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}
	}else{ // others
	    /*
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL){
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}else{
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
		}*/
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) >= 994) &&
				(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) <= 1005)){	//100Hz
				if(Get_DISPLAY_REFRESH_RATE() == 144){
                    Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 100);
				}
				else if(Get_DISPLAY_REFRESH_RATE() == 165){
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 100);
				}
				else{
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 50);
				}
			}
			else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) >= 1191) &&
					(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) <= 1206)){	//120Hz
				if(Get_DISPLAY_REFRESH_RATE() == 144){
                    Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 120);
				}
				else if(Get_DISPLAY_REFRESH_RATE() == 165){
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 120);
				}
				else{
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
				}
			}
			else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) >= 1435) &&
					(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) <= 1445)){	//144Hz
                if(Get_DISPLAY_REFRESH_RATE() == 144){
                    Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 144);
				}
				else if(Get_DISPLAY_REFRESH_RATE() == 165){
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 144);
				}
				else{
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
				}
			}
            else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) >= 1645) &&
					(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) <= 1655)){	//144Hz
                if(Get_DISPLAY_REFRESH_RATE() == 165){
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 165);
				}
				else{
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
				}
			}
			else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) >= 2395) &&
				(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) <= 2405))
            {	//240Hz
                if(Get_DISPLAY_REFRESH_RATE() == 240)
                {
                    Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 240);
                }
                else
                {
                    Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
                }
            }
            else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) >= 2878) &&
                (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) <= 2898))
            { //240Hz
                if(Get_DISPLAY_REFRESH_RATE() == 288)
                {
                    Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 288);
                }
                else
                {
                    Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
                }
            }
		}
		else
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ, 60);
	}
    Set_decide_dtg_m_mode_set_cinema_mode_flag(true); // Move to vsc tsk due to sending rpc in another rpc(rpcSetMEMCBypass)
	rtd_pr_vbe_debug("output=%d \n", Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ));

}

void modestate_decide_pixel_mode(void)
{
	if(Get_DISPLAY_SR_PIXEL_MODE() == 0){
		Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE, PIXEL_MODE_1_PIXEL);
		rtd_pr_vbe_debug("[%s] 1 pixel mode. \n", __FUNCTION__);
	}else if(Get_DISPLAY_SR_PIXEL_MODE() == 1){
		Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE, PIXEL_MODE_2_PIXEL);
		rtd_pr_vbe_debug("[%s] 2 pixel mode. \n", __FUNCTION__);
	}else{
		Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE, PIXEL_MODE_1_PIXEL);
		rtd_pr_vbe_debug("[%s] 1 pixel mode. \n", __FUNCTION__);
	}
}



unsigned char g_ucMdomainFrc[2] = {FALSE, FALSE};
unsigned int g_ucMDomainFrcVFreq[2] = {60, 60};

unsigned char modestate_get_mdomain_frc_flag(unsigned char _channel)
{
	SCALER_DISP_CHANNEL channel = (SCALER_DISP_CHANNEL)_channel;


	pr_emerg("modestate_get_mdomain_frc_flag game mode=%x, game mode flag=%x, game_mode_dynamic=%x\n", drv_memory_get_game_mode(), drv_memory_get_game_mode_flag(), drv_memory_get_low_delay_game_mode_dynamic());

	if ((Get_DisplayMode_Src(_channel) != VSC_INPUTSRC_HDMI) || (TRUE == drv_memory_get_game_mode()) || TRUE == drv_memory_get_low_delay_game_mode_dynamic()
		|| TRUE == drv_memory_get_game_mode_flag())
	{
		pr_emerg("modestate_get_mdomain_frc_flag return false, source=%x, game mode=%x\n", Get_DisplayMode_Src(_channel), drv_memory_get_game_mode());

		g_ucMdomainFrc[_channel] = FALSE;

		return g_ucMdomainFrc[_channel] ;

	}

#if 0
	if ((rtd_inl(0xb80277f0) & _BIT0) != 0)
	{
		g_ucMdomainFrc[_channel] = FALSE;

		return g_ucMdomainFrc[_channel] ;
	}
#endif
	// input 24hz, uzu 48hz
	if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 235) &&
			(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 245))	//24Hz
	{
		g_ucMdomainFrc[_channel] = TRUE;
		g_ucMDomainFrcVFreq[_channel] = 480;

		pr_emerg("modestate_get_mdomain_frc_flag input 24hz, uzu 48hz\n");

	}
	// input 25hz, uzu 50hz
	else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 245) &&
			  (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 255))	//25Hz
	{
		g_ucMdomainFrc[_channel] = TRUE;
		g_ucMDomainFrcVFreq[_channel] = 500;

		pr_emerg("modestate_get_mdomain_frc_flag input 25hz, uzu 50hz\n");
	}
	// input 30hz, uzu 60hz
	else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 295) &&
			  (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 305))	//30Hz
	{
		g_ucMdomainFrc[_channel] = TRUE;
		g_ucMDomainFrcVFreq[_channel] = 600;

		pr_emerg("modestate_get_mdomain_frc_flag input 30hz, uzu 60hz\n");

	}
	else
	{
		g_ucMdomainFrc[_channel] = FALSE;
	}


	pr_emerg("modestate_get_mdomain_frc_flag=%x, channel=%d\n", g_ucMdomainFrc[_channel], _channel);
	return g_ucMdomainFrc[_channel];
}


unsigned char modestate_check_mdomain_frc_scaler_rerun(unsigned char _channel)
{
	unsigned char ucPreFlag = g_ucMdomainFrc[_channel];
	unsigned char ucCurFlag = modestate_get_mdomain_frc_flag(_channel);


	pr_emerg("ucPreFlag=%x, ucCurFlag=%x\n", ucPreFlag, ucCurFlag);

	if (ucPreFlag != ucCurFlag)
	{
		pr_emerg("modestate_check_mdomain_frc_scaler_rerun=TRUE\n");
		return TRUE;
	}
	else
	{
		pr_emerg("modestate_check_mdomain_frc_scaler_rerun=FALSE\n");
		return FALSE;
	}
}



unsigned int modestate_get_mdomain_frc_vfreq(unsigned char _channel)
{
	pr_emerg("modestate_get_mdomain_frc_vfreq=%x, channel=%d\n", g_ucMDomainFrcVFreq[_channel], _channel);
	return g_ucMDomainFrcVFreq[_channel];
}




// input 24hz,25hz, 30hz, repeat m-domain
unsigned char modestate_decide_mdomain_frc_fractional(unsigned char _channel)
{
	SCALER_DISP_CHANNEL channel = (SCALER_DISP_CHANNEL)_channel;


	pr_emerg("modestate_decide_mdomain_frc_fractional game mode=%x, game mode flag=%x, game_mode_dynamic=%x\n", drv_memory_get_game_mode(), drv_memory_get_game_mode_flag(), drv_memory_get_low_delay_game_mode_dynamic());

	if ((Get_DisplayMode_Src(_channel) != VSC_INPUTSRC_HDMI) || (TRUE == drv_memory_get_game_mode()) || TRUE == drv_memory_get_game_mode_flag() 
		|| TRUE == drv_memory_get_low_delay_game_mode_dynamic())
	{
		pr_emerg("Don't do modestate_decide_mdomain_frc_fractional, source=%x\n", Get_DisplayMode_Src(_channel));

		g_ucMdomainFrc[_channel] = FALSE;

		return g_ucMdomainFrc[_channel] ;

	}


#if 0
	if ((rtd_inl(0xb80277f0) & _BIT0) != 0)
	{
		g_ucMdomainFrc[_channel] = FALSE;

		return g_ucMdomainFrc[_channel] ;
	}
#endif

	// input 24hz, uzu 48hz
	if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 235) &&
			(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 245))	//24Hz
	{
		g_ucMdomainFrc[_channel] = TRUE;
		g_ucMDomainFrcVFreq[_channel] = 480;

		pr_emerg("modestate_decide_mdomain_frc_fractional input 24hz, uzu 48hz\n");

		// UZU 48hz
		if(Get_DISPLAY_REFRESH_RATE() == 120){
			pr_emerg( "2/1, 1pixel \n");
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
		}else{
			if(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
				pr_emerg( "4/5, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 4);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
			}else{
				pr_emerg( "1/1, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
			}
		}

	}
	// input 25hz, uzu 50hz
	else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 245) &&
			  (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 255))	//25Hz
	{
		g_ucMdomainFrc[_channel] = TRUE;
		g_ucMDomainFrcVFreq[_channel] = 500;

		pr_emerg("modestate_decide_mdomain_frc_fractional input 25hz, uzu 50hz\n");

		if(Get_DISPLAY_REFRESH_RATE() == 120){
			if(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
				pr_emerg( "12/5, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 5);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 12);
			}else{
				pr_emerg( "2/1, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
		}else{
			if(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
				pr_emerg( "6/5, 1pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 5);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 6);
			}else{
				pr_emerg( "1/1, 1pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
			}
		}

	}
	// input 30hz, uzu 60hz
	else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 295) &&
			  (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 305))	//30Hz
	{
		g_ucMdomainFrc[_channel] = TRUE;
		g_ucMDomainFrcVFreq[_channel] = 600;

		pr_emerg("modestate_decide_mdomain_frc_fractional input 30hz, uzu 60hz\n");

		if(Get_DISPLAY_REFRESH_RATE() == 120){
			pr_emerg( "2/1, 2pixel \n");
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
		}else{
			pr_emerg( "1/1, 1pixel  \n");
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
		}

	}
	else
	{
		g_ucMdomainFrc[_channel] = FALSE;
	}

	pr_emerg("modestate_decide_mdomain_frc_fractional g_ucMdomainFrc[_channel]=%x\n", g_ucMdomainFrc[_channel]);

	return g_ucMdomainFrc[_channel];
}




void modestate_decide_fractional_framesync(void)
{
    //rtd_pr_vbe_debug("modestate_decide_fractional_framesync=> ");

#ifdef CONFIG_MDOMAIN_FORCE_FRC
	pr_emerg("modestate_decide_fractional_framesync=> ");

	if (FALSE == drv_memory_get_game_mode())
	{
		if (TRUE == modestate_decide_mdomain_frc_fractional(SLR_MAIN_DISPLAY))
		{
			pr_emerg("modestate_decide_mdomain_frc_fractional=TRUE\n");
			return;
		}
	}
#endif

    unsigned int inputFrameRate = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
    unsigned int dtg_master_v_freq = Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ);
    unsigned int remove = 0;
    unsigned int multiple = 0;

    get_scaler_remove_mutiple(inputFrameRate, dtg_master_v_freq, Get_DISPLAY_REFRESH_RATE(), &remove, &multiple);

    Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, remove);
    Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, multiple);
#if 0
	if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) &&
		(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245))	//24Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(Get_DISPLAY_REFRESH_RATE() == 144){
				rtd_pr_vbe_debug("6/1, 2pixel \n");
                Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
                Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 6);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				rtd_pr_vbe_debug("6/1, 2pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 6);
			}
			else{
				rtd_pr_vbe_debug("10/2, 2pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
			}
		}else{
			if(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
				rtd_pr_vbe_debug("5/2, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 2);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
			}else{ // dtg_m 48Hz
				rtd_pr_vbe_debug("2/1, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
		}
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 245) &&
		(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 255))	//25Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(Get_DISPLAY_REFRESH_RATE() == 144){
                Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
                Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				rtd_pr_vbe_debug("6/1, 2pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 6);
			}
			else{
				if(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
					rtd_pr_vbe_debug("24/5, 2pixel \n");
					Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 5);
					Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 24);
				}else{ // dtg_m 50Hz
					rtd_pr_vbe_debug("4/1, 2pixel \n");
					Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
					Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 4);
				}
			}
		}else{
			if(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
				rtd_pr_vbe_debug("12/5, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 5);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 12);
			}else{ // dtg_m 50Hz
				rtd_pr_vbe_debug("2/1, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
		}
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) &&
		(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305))	//30Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(Get_DISPLAY_REFRESH_RATE() == 144){
				rtd_pr_vbe_debug("4/1, 2pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 4);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				rtd_pr_vbe_debug("5/1, 2pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
			}
			else{
				rtd_pr_vbe_debug("4/1, 2pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 4);
			}
		}else{
			rtd_pr_vbe_debug("2/1, 1pixel \n");
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
		}
	}
	else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 475) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 485))	//48Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(Get_DISPLAY_REFRESH_RATE() == 144){
				rtd_pr_vbe_debug("3/1, 2pixel \n");
                Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
                Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 3);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				rtd_pr_vbe_debug("3/1, 2pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 3);
			}
			else{
				rtd_pr_vbe_debug("2/1, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
		}else{
			if(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
				rtd_pr_vbe_debug("4/5, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 4);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
			}else{
				rtd_pr_vbe_debug("1/1, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
			}
		}
	}
	else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 505))	//50Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(Get_DISPLAY_REFRESH_RATE() == 144){
				rtd_pr_vbe_debug("2/1, 2pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				rtd_pr_vbe_debug("3/1, 2pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 3);
			}
			else{
				if(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
					rtd_pr_vbe_debug("12/5, 2pixel \n");
					Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 5);
					Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 12);
				}else{
					rtd_pr_vbe_debug("2/1, 2pixel \n");
					Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
					Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
				}
			}
		}else{
			if(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
				rtd_pr_vbe_debug("6/5, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 5);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 6);
			}else{
				rtd_pr_vbe_debug("1/1, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
			}
		}
	}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 595) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 605))	//60hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(Get_DISPLAY_REFRESH_RATE() == 144){
				rtd_pr_vbe_debug("2/1, 2pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				rtd_pr_vbe_debug("2/1, 2pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
			else{
				rtd_pr_vbe_debug("2/1, 2pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
		}else{
			rtd_pr_vbe_debug("1/1, 1pixel  \n");
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
		}
	}else{ // others
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 994) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 1005))
				|| ((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 1191) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 1206))
				|| ((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 1435) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 1445))
				|| ((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 2395) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 2405))
                || ((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 2875) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 2885))){
				rtd_pr_vbe_debug("1/1, 1pixel (input 100Hz/120Hz/240Hz/288Hz) \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
			}
			else{
				rtd_pr_vbe_debug("2/1, 2pixel (other timing) \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
		}else{
			rtd_pr_vbe_debug("1/1, 1pixel (other timing) \n");
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
		}
	}
	rtd_pr_vbe_emerg("modestate_decide_fractional_framesync=> %d : %d\n", Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO), Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO));
#endif
}

void modestate_decide_fractional_framesyncByDisp(unsigned char _channel)
{
    SCALER_DISP_CHANNEL channel = (SCALER_DISP_CHANNEL)_channel;
    unsigned int inputFrameRate = Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_V_FREQ);
    unsigned int dtg_master_v_freq = Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ);
    unsigned int remove = 0;
    unsigned int multiple = 0;

    get_scaler_remove_mutiple(inputFrameRate, dtg_master_v_freq, Get_DISPLAY_REFRESH_RATE(), &remove, &multiple);

    Scaler_DispSetInputInfoByDisp(channel, SLR_INPUT_FRAC_FS_REMOVE_RATIO, remove);
    Scaler_DispSetInputInfoByDisp(channel, SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, multiple);

    //rtd_pr_vbe_debug("modestate_decide_fractional_framesyncByDisp=> ");

#ifdef CONFIG_MDOMAIN_FORCE_FRC
	pr_emerg("modestate_decide_fractional_framesyncByDisp=> ");

	if (FALSE == drv_memory_get_game_mode())
	{
		if (TRUE == modestate_decide_mdomain_frc_fractional(_channel))
		{
			pr_emerg("modestate_decide_mdomain_frc_fractional=TRUE\n");
			return;
		}
	}
#endif

#if 0
	if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 235) &&
			(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 245))	//24Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(Get_DISPLAY_REFRESH_RATE() == 144){
				rtd_pr_vbe_debug("6/1, 2pixel \n");
                Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
                Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 6);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				rtd_pr_vbe_debug("6/1, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 6);
			}
			else{
				rtd_pr_vbe_debug("10/2, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
			}
		}else{
			if(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
				rtd_pr_vbe_debug("5/2, 1pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 2);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
			}else{ // dtg_m 48Hz
				rtd_pr_vbe_debug("2/1, 1pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
		}
	}else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 245) &&
			  (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 255))	//25Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(Get_DISPLAY_REFRESH_RATE() == 144){
				rtd_pr_vbe_debug("5/1, 2pixel \n");
                Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
                Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				rtd_pr_vbe_debug("6/1, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 6);
			}
			else{
				if(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
					rtd_pr_vbe_debug("24/5, 2pixel \n");
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 5);
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 24);
				}else{ // dtg_m 50Hz
					rtd_pr_vbe_debug("4/1, 2pixel \n");
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 4);
				}
			}
		}else{
			if(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
				rtd_pr_vbe_debug("12/5, 1pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 5);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 12);
			}else{ // dtg_m 50Hz
				rtd_pr_vbe_debug("2/1, 1pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
		}
	}else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 295) &&
			  (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 305))	//30Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(Get_DISPLAY_REFRESH_RATE() == 144){
				rtd_pr_vbe_debug("4/1, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 4);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				rtd_pr_vbe_debug("5/1, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
			}
			else{
				rtd_pr_vbe_debug("4/1, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 4);
			}
		}else{
			rtd_pr_vbe_debug("2/1, 1pixel \n");
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
		}
	}
	else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 475) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 485))	//48Hz
	{

		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(Get_DISPLAY_REFRESH_RATE() == 144){
				rtd_pr_vbe_debug("3/1, 2pixel \n");
                Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
                Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 3);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				rtd_pr_vbe_debug("3/1, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 3);
			}
			else{
				rtd_pr_vbe_debug("2/1, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
		}else{
			if(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
				rtd_pr_vbe_debug("4/5, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 4);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
			}else{
				rtd_pr_vbe_debug("1/1, 1pixel \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
			}
		}
	}
	else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 495) &&
			(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) < 505))	//50Hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(Get_DISPLAY_REFRESH_RATE() == 144){
				rtd_pr_vbe_debug("2/1, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				rtd_pr_vbe_debug("3/1, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 3);
			}
			else{
				if(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
					rtd_pr_vbe_debug("12/5, 2pixel \n");
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 5);
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 12);
				}else{
					rtd_pr_vbe_debug("2/1, 2pixel \n");
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
					Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
				}
			}
		}else{
			if(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ) == 60){
				rtd_pr_vbe_debug("6/5, 1pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 5);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 6);
			}else{
				rtd_pr_vbe_debug("1/1, 1pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
			}
		}
	}else if((Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) > 595) &&
			  (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_V_FREQ) <= 605))	//60hz
	{
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(Get_DISPLAY_REFRESH_RATE() == 144){
				rtd_pr_vbe_debug("2/1, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
			else if(Get_DISPLAY_REFRESH_RATE() == 165){
				rtd_pr_vbe_debug("2/1, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
			else{
				rtd_pr_vbe_debug("2/1, 2pixel \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
		}else{
			rtd_pr_vbe_debug("1/1, 1pixel  \n");
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
		}
	}else{ // others
		if(Get_DISPLAY_REFRESH_RATE() >= 120){
			if(((Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_V_FREQ) >= 994) && (Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_V_FREQ) <= 1005))
				|| ((Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_V_FREQ) >= 1191) && (Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_V_FREQ) <= 1206))
				|| ((Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_V_FREQ) >= 1435) && (Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_V_FREQ) <= 1445))
				|| ((Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_V_FREQ) >= 2395) && (Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_V_FREQ) <= 2405))
                || ((Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_V_FREQ) >= 2875) && (Scaler_DispGetInputInfoByDisp(channel, SLR_INPUT_V_FREQ) <= 2885))){
				rtd_pr_vbe_debug("1/1, 1pixel (input 100Hz/120Hz/240Hz/288Hz) \n");
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
			}
			else{
				rtd_pr_vbe_debug("2/1, 2pixel (other timing) \n");
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
				Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
			}
		}else{
			rtd_pr_vbe_debug("1/1, 1pixel (other timing) \n");
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
			Scaler_DispSetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
		}
	}
#endif
}

#define HTOTAL_MAPPING_MEMC_CLOCK_INCREASE 5500
void modestate_set_display_timing_gen(void)
{
	ppoverlay_dv_total_RBUS dv_total_reg;
	ppoverlay_uzudtg_countrol_RBUS uzudtg_countrol_reg;
	ppoverlay_uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
	ppoverlay_uzudtg_dh_total_RBUS uzudtg_dh_total_reg;
	ppoverlay_dh_den_start_end_RBUS dh_den_start_end_reg;
	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	ppoverlay_memcdtg_control_RBUS memcdtg_control_reg;
	ppoverlay_memcdtg_dv_total_RBUS memcdtg_dv_total_reg;
	ppoverlay_memcdtg_dh_total_RBUS memcdtg_dh_total_reg;
	ppoverlay_memcdtg_dv_den_start_end_RBUS memcdtg_dv_den_start_end_reg;
	ppoverlay_memcdtg_dh_den_start_end_RBUS memcdtg_dh_den_start_end_reg;
	ppoverlay_dctl_iv2dv_ihs_ctrl_RBUS ppoverlay_dctl_iv2dv_ihs_ctrl_reg;
	ppoverlay_sram_ok_sub_result_even_RBUS ppoverlay_sram_ok_sub_result_even_reg;
//	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	//memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
	unsigned int vertical_total;
	ppoverlay_new_meas0_linecnt_real_RBUS ppoverlay_new_meas0_linecnt_real_reg;
	UINT32 cnt = 0xa0000;
	if((vbe_disp_get_adaptivestream_fs_mode()) || (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == TRUE)
		|| ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA)
		&& (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) > DATAFS_VDEC_RUN_DATAFS_WIDTH)
		&& (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) > DATAFS_VDEC_RUN_DATAFS_HEIGHT))
	) {
		if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) && (Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_COMMERCIAL_4K2K_600M_1S_8L_DUPLICATE))
			vertical_total = scalerdisplay_get_vtotal_by_vfreq(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ));
		else
			vertical_total = scaler_disp_smooth_get_memc_protect_vtotal();
	}
	else
		vertical_total = scalerdisplay_get_vtotal_by_vfreq(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ));

	ppoverlay_new_meas0_linecnt_real_reg.regValue = rtd_inl(PPOVERLAY_new_meas0_linecnt_real_reg);
	while((ppoverlay_new_meas0_linecnt_real_reg.line_cnt_rt >= vertical_total) && --cnt){
		ppoverlay_new_meas0_linecnt_real_reg.regValue = rtd_inl(PPOVERLAY_new_meas0_linecnt_real_reg);
	}

	/*setup dtg master.*/
	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	dv_total_reg.dv_total= vertical_total-1;
	IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
	vbe_disp_pwm_frequency_update(FALSE);

	if(Get_DISPLAY_REFRESH_RATE() == 60){
		IoReg_Mask32(LDSPI_ld_spi_v_total_reg, 0x7ffff, ((vertical_total/2)-1)<<19); // repeat local dimming vsync
		rtd_pr_vbe_notice("vertical_total=%x, 0xb802c96c = %x\n", vertical_total, IoReg_Read32(LDSPI_ld_spi_v_total_reg));
	}

	/*setup uzu dtg.*/
	uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
	uzudtg_dv_total_reg.uzudtg_dv_total = vertical_total-1;
	IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);
	uzudtg_dh_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DH_TOTAL_reg);
//#ifdef CONFIG_MEMC_BYPASS
	if(get_MEMC_bypass_status_refer_platform_model() == TRUE){
        uzudtg_dh_total_reg.uzudtg_dh_total = (Get_DISP_HORIZONTAL_TOTAL()-1)/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
        uzudtg_dh_total_reg.uzudtg_dh_total_last_line = (Get_DISP_HORIZONTAL_TOTAL()-1)/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
    }else{
//#else
#ifndef BUILD_QUICK_SHOW 

		if(Get_MEMC_Enable_Dynamic() == 0){
          uzudtg_dh_total_reg.uzudtg_dh_total = (Get_DISP_HORIZONTAL_TOTAL()-1)/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
          uzudtg_dh_total_reg.uzudtg_dh_total_last_line = (Get_DISP_HORIZONTAL_TOTAL()-1)/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
		}else
#endif            
            {
          uzudtg_dh_total_reg.uzudtg_dh_total = (Get_DISP_HORIZONTAL_TOTAL()-1);
          uzudtg_dh_total_reg.uzudtg_dh_total_last_line = (Get_DISP_HORIZONTAL_TOTAL()-1);
        }

	if((Get_DISPLAY_REFRESH_RATE()>=120)&&(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)>235)&&(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)<245)){
		//In order to keep framerate not change, we will make each cost time of vertical line is the same.
		// (1/240M) * 4400 = (1/300M) * 5500
		rtd_pr_vbe_notice("120HZ panel, input 24HZ, Hack htotal as %d.\n", HTOTAL_MAPPING_MEMC_CLOCK_INCREASE);
		uzudtg_dh_total_reg.uzudtg_dh_total = (HTOTAL_MAPPING_MEMC_CLOCK_INCREASE-1);
		uzudtg_dh_total_reg.uzudtg_dh_total_last_line = (HTOTAL_MAPPING_MEMC_CLOCK_INCREASE-1);
	}
//#endif
    }

	IoReg_Write32(PPOVERLAY_uzudtg_DH_TOTAL_reg, uzudtg_dh_total_reg.regValue);
	dh_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DH_DEN_Start_End_reg);
	if((get_MEMC_bypass_status_refer_platform_model() == TRUE) || (Get_DISPLAY_REFRESH_RATE() > 120)){
//#ifdef CONFIG_MEMC_BYPASS
      dh_den_start_end_reg.dh_den_sta = Get_DISP_DEN_STA_HPOS()/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
      dh_den_start_end_reg.dh_den_end = Get_DISP_DEN_END_HPOS()/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
	}else{
//#else
#ifndef BUILD_QUICK_SHOW 

      if(Get_MEMC_Enable_Dynamic() == 0){
        dh_den_start_end_reg.dh_den_sta = Get_DISP_DEN_STA_HPOS()/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
        dh_den_start_end_reg.dh_den_end = Get_DISP_DEN_END_HPOS()/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
      }else
#endif        
        {
        dh_den_start_end_reg.dh_den_sta = Get_DISP_DEN_STA_HPOS();
        dh_den_start_end_reg.dh_den_end = Get_DISP_DEN_END_HPOS();
      }
//#endif
	}
	IoReg_Write32(PPOVERLAY_DH_DEN_Start_End_reg, dh_den_start_end_reg.regValue);
	dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	dv_den_start_end_reg.dv_den_sta = Get_DISP_DEN_STA_VPOS();
	dv_den_start_end_reg.dv_den_end = Get_DISP_DEN_END_VPOS();
    if((Get_DISP_DEN_END_VPOS() - Get_DISP_DEN_STA_VPOS()) == 1080)
    {
    //    dv_den_start_end_reg.dv_den_end = Get_DISP_DEN_END_VPOS() + 8;
    }
	IoReg_Write32(PPOVERLAY_DV_DEN_Start_End_reg, dv_den_start_end_reg.regValue);
	uzudtg_countrol_reg.regValue = IoReg_Read32(PPOVERLAY_UZUDTG_COUNTROL_reg);
	uzudtg_countrol_reg.uzudtg_en = 1;
	uzudtg_countrol_reg.uzudtg_fsync_en = 1;
	IoReg_Write32(PPOVERLAY_UZUDTG_COUNTROL_reg, uzudtg_countrol_reg.regValue);

	/*setup memc dtg.*/
	memcdtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
	memcdtg_dv_total_reg.memcdtg_dv_total = vertical_total-1;
	IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.regValue);
	memcdtg_dh_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DH_TOTAL_reg);
	memcdtg_dh_total_reg.memcdtg_dh_total = Get_DISP_HORIZONTAL_TOTAL()-1;
	memcdtg_dh_total_reg.memcdtg_dh_total_last_line = Get_DISP_HORIZONTAL_TOTAL()-1;
	IoReg_Write32(PPOVERLAY_memcdtg_DH_TOTAL_reg, memcdtg_dh_total_reg.regValue);
	memcdtg_dh_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg);
#if 0 // ECN RL6748-551 for memc convert board.
	if(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI_MEMC_FREERUN){
		memcdtg_dh_den_start_end_reg.memcdtg_dh_den_sta = Get_DISP_DEN_STA_HPOS()+DTG_MEMC_FREERUN_DEN_OFFSET;
		memcdtg_dh_den_start_end_reg.memcdtg_dh_den_end = Get_DISP_DEN_END_HPOS()+DTG_MEMC_FREERUN_DEN_OFFSET;
	}else{
		memcdtg_dh_den_start_end_reg.memcdtg_dh_den_sta = Get_DISP_DEN_STA_HPOS();
		memcdtg_dh_den_start_end_reg.memcdtg_dh_den_end = Get_DISP_DEN_END_HPOS();
	}
#endif
	memcdtg_dh_den_start_end_reg.memcdtg_dh_den_sta = Get_DISP_DEN_STA_HPOS();
	memcdtg_dh_den_start_end_reg.memcdtg_dh_den_end = Get_DISP_DEN_END_HPOS();
	IoReg_Write32(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg, memcdtg_dh_den_start_end_reg.regValue);
	memcdtg_dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg);
//#ifdef CONFIG_MEMC_BYPASS

    memcdtg_dv_den_start_end_reg.memcdtg_dv_den_sta = Get_DISP_DEN_STA_VPOS();
    memcdtg_dv_den_start_end_reg.memcdtg_dv_den_end = Get_DISP_DEN_END_VPOS();

	IoReg_Write32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg, memcdtg_dv_den_start_end_reg.regValue);

	memcdtg_control_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_reg);
	memcdtg_control_reg.memcdtg_en = 1;
    IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL_reg, memcdtg_control_reg.regValue);

    if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI_MEMC_FREERUN) 
            || (get_vo_camera_flow_flag(Get_DisplayMode_Port(SLR_MAIN_DISPLAY))==1) || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA)
    {
        //frc
        drvif_scaler_memc_set_display_mode_frc2fsync_by_hw_speedup(0);
    }
    else
    {
        //frc2fs
        drvif_scaler_memc_set_display_mode_frc2fsync_by_hw_speedup(1);
    }

	ppoverlay_dctl_iv2dv_ihs_ctrl_reg.iv2dv_ih_slf_gen_en = 1;
	if(VGIP_Data_Path_Select_get_main_2pxl_en(IoReg_Read32(VGIP_Data_Path_Select_reg)))
		ppoverlay_dctl_iv2dv_ihs_ctrl_reg.iv2dv_ih_slf_width = Scaler_DispGetInputInfo(SLR_INPUT_H_LEN) / 2;
	else
		ppoverlay_dctl_iv2dv_ihs_ctrl_reg.iv2dv_ih_slf_width = Scaler_DispGetInputInfo(SLR_INPUT_H_LEN);
	IoReg_Write32(PPOVERLAY_dctl_iv2dv_ihs_ctrl_reg, ppoverlay_dctl_iv2dv_ihs_ctrl_reg.regValue);

	if(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI_MEMC_FREERUN){
		ppoverlay_sram_ok_sub_result_even_reg.regValue = IoReg_Read32(PPOVERLAY_SRAM_OK_sub_Result_even_reg);
		ppoverlay_sram_ok_sub_result_even_reg.s2_align_memc4p_en = 1;
		IoReg_Write32(PPOVERLAY_SRAM_OK_sub_Result_even_reg, ppoverlay_sram_ok_sub_result_even_reg.regValue);
	}

	// update sfg for vrr
	vbe_disp_set_vrr_average_den_mode(0);
	vbe_disp_set_vrr_average_den_update(1);

	//Update MEMC Htotal
	Scaler_MEMC_Set_DISP_DHTOTAL(Get_DISP_HORIZONTAL_TOTAL());
	Scaler_set_MEMC_change_size();

#if 0//def CONFIG_MEMC_BYPASS => change to control by MEMC
	memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	memc_mux_ctrl_reg.memc_outmux_sel = 0; //from MEMC
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);
//#else
	if(Get_MEMC_Enable_Dynamic() == 0){
		memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
		memc_mux_ctrl_reg.memc_outmux_sel = 0; //from MEMC
		IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);
	}
#endif
#if 0//fixme k4l bringup no dout_pixel_mode item
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	if(Get_DISPLAY_REFRESH_RATE()>=120)
		display_timing_ctrl2_reg.dout_pixel_mode=1;
	else
		display_timing_ctrl2_reg.dout_pixel_mode=0;

	rtd_pr_vbe_notice("display_timing_ctrl2_reg:%x\n", display_timing_ctrl2_reg.regValue);
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
#endif

	/*set memc den settings==>*/

}

#define ADJUST_VTOTAL_VALUE 25
void modestate_set_display_timing_genByDisp(unsigned char _channel)
{
	ppoverlay_dv_total_RBUS dv_total_reg;
	ppoverlay_uzudtg_countrol_RBUS uzudtg_countrol_reg;
	ppoverlay_uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
	ppoverlay_uzudtg_dh_total_RBUS uzudtg_dh_total_reg;
	ppoverlay_dh_den_start_end_RBUS dh_den_start_end_reg;
	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	ppoverlay_memcdtg_control_RBUS memcdtg_control_reg;
	ppoverlay_memcdtg_dv_total_RBUS memcdtg_dv_total_reg;
	ppoverlay_memcdtg_dh_total_RBUS memcdtg_dh_total_reg;
	ppoverlay_memcdtg_dv_den_start_end_RBUS memcdtg_dv_den_start_end_reg;
	ppoverlay_memcdtg_dh_den_start_end_RBUS memcdtg_dh_den_start_end_reg;
//	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	ppoverlay_dh_total_last_line_length_RBUS ppoverlay_dh_total_last_line_length_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
    	ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
    	ppoverlay_dctl_iv2dv_ihs_ctrl_RBUS ppoverlay_dctl_iv2dv_ihs_ctrl_reg;
	ppoverlay_sram_ok_sub_result_even_RBUS ppoverlay_sram_ok_sub_result_even_reg;
	//memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
	unsigned int vertical_total;
	ppoverlay_new_meas0_linecnt_real_RBUS ppoverlay_new_meas0_linecnt_real_reg;
	UINT32 cnt = 0xa0000;
	unsigned int m_adjust_vtotal = 0;
	unsigned int m_loop_timeout = 0;

	if((get_vsc_src_is_hdmi_or_dp()) && (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())){
		vertical_total = vbe_disp_get_VRR_timing_real_vtotal();
		if(vertical_total==0)
			return;
	}
	else if((vbe_disp_get_adaptivestream_fs_mode()) || (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == TRUE)
		|| ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA)
		&& (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) > DATAFS_VDEC_RUN_DATAFS_WIDTH)
		&& (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) > DATAFS_VDEC_RUN_DATAFS_HEIGHT))
	) {
		if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) && (Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_COMMERCIAL_4K2K_600M_1S_8L_DUPLICATE))
			vertical_total = scalerdisplay_get_vtotal_by_vfreq(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ));
		else
			vertical_total = scaler_disp_smooth_get_memc_protect_vtotal();
	}
	else
		vertical_total = scalerdisplay_get_vtotal_by_vfreq(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ));

	ppoverlay_new_meas0_linecnt_real_reg.regValue = rtd_inl(PPOVERLAY_new_meas0_linecnt_real_reg);
	while((ppoverlay_new_meas0_linecnt_real_reg.line_cnt_rt >= vertical_total) && --cnt){
		ppoverlay_new_meas0_linecnt_real_reg.regValue = rtd_inl(PPOVERLAY_new_meas0_linecnt_real_reg);
	}

	/*setup dtg master.*/
	if (Get_TIMING_PROTECT_PANEL_TYPE() &&
		!(vbe_disp_decide_uzuclock_mapping_vo())
	  )
	{
		dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
		m_adjust_vtotal = dv_total_reg.dv_total;
		if((m_adjust_vtotal > (vertical_total-1)) && ((m_adjust_vtotal-vertical_total+1)>ADJUST_VTOTAL_VALUE)){//2700 to 2250
			ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
			ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 1;//enable double buffer D7
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
			ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
			ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

			m_loop_timeout = 100;
			while(((m_adjust_vtotal > (vertical_total-1)) &&((m_adjust_vtotal-vertical_total+1)>ADJUST_VTOTAL_VALUE)) && (--m_loop_timeout)){
				m_adjust_vtotal = m_adjust_vtotal - ADJUST_VTOTAL_VALUE;
				dv_total_reg.dv_total= m_adjust_vtotal;
				IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
				uzudtg_dv_total_reg.uzudtg_dv_total = m_adjust_vtotal;
				IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);

				ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set = 1;//enable double buffer D7
				IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
				ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;
				IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
				cnt = 10;
				while(PPOVERLAY_Double_Buffer_CTRL_get_dreg_dbuf_set(IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg)) && (--cnt)){
					usleep_range(10 * 1000, 10* 1000);
				}
				if(cnt == 0)
					rtd_pr_vbe_notice("wait d7 timeout %x\n", IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg));
				//rtd_pr_vbe_notice("-m_adjust_vtotal=%d, b8028014=%x,b802850c=%x (loop %d\n", m_adjust_vtotal, IoReg_Read32(0xb8028014),IoReg_Read32(0xb8028504), m_loop_timeout);
			}
			if(m_loop_timeout == 0)
				rtd_pr_vbe_notice("adjust vtotal loop timeout %d:%d\n", m_adjust_vtotal, (vertical_total-1));
			ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set = 1;//enable double buffer D7
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
			ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
			//wait last frame vtotal is applied
			while(PPOVERLAY_Double_Buffer_CTRL_get_dreg_dbuf_set(IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg)) && (--cnt)){
				usleep_range(10 * 1000, 10 * 1000);
			}
			ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 0;//enable double buffer D7
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
			ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
		}else if((m_adjust_vtotal < (vertical_total-1)) && ((vertical_total-1-m_adjust_vtotal)>ADJUST_VTOTAL_VALUE)){//2250 to 2700
			ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
			ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 1;//enable double buffer D7
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
			ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
			ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
			m_loop_timeout = 100;
			while(((m_adjust_vtotal < (vertical_total-1)) &&((vertical_total-1-m_adjust_vtotal)>ADJUST_VTOTAL_VALUE)) && (--m_loop_timeout)){
				m_adjust_vtotal = m_adjust_vtotal + ADJUST_VTOTAL_VALUE;
				dv_total_reg.dv_total= m_adjust_vtotal;
				IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
				uzudtg_dv_total_reg.uzudtg_dv_total = m_adjust_vtotal;
				IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);

				ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set = 1;//enable double buffer D7
				IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
				ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;
				IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
				cnt = 10;
				while(PPOVERLAY_Double_Buffer_CTRL_get_dreg_dbuf_set(IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg)) && (--cnt)){
					usleep_range(10000, 10000);
				}
				if(cnt == 0)
					rtd_pr_vbe_notice("wait d7 timeout %x\n", IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg));

				//rtd_pr_vbe_notice("+m_adjust_vtotal=%d, b8028014=%x,b802850c=%x (loop %d\n", m_adjust_vtotal, IoReg_Read32(0xb8028014),IoReg_Read32(0xb8028504), m_loop_timeout);
			}
			if(m_loop_timeout == 0)
				rtd_pr_vbe_notice("adjust vtotal loop timeout %d:%d\n", m_adjust_vtotal, (vertical_total-1));
			ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set = 1;//enable double buffer D7
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
			ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
			//wait last frame vtotal is applied
			while(PPOVERLAY_Double_Buffer_CTRL_get_dreg_dbuf_set(IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg)) && (--cnt)){
				usleep_range(10000, 10000);
			}
			ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 0;//enable double buffer D7
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
			ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
		}
		dv_total_reg.dv_total= vertical_total-1;
		IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
		uzudtg_dv_total_reg.uzudtg_dv_total = vertical_total-1;
		IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);
	}
    else
    {
		dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
		dv_total_reg.dv_total= vertical_total-1;
		IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);

		/*setup uzu dtg.*/

		ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

		uzudtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
		uzudtg_dv_total_reg.uzudtg_dv_total = vertical_total-1;
		/* camera 120hz
		if(get_vo_camera_flow_flag(Get_DisplayMode_Port(SLR_MAIN_DISPLAY))==1){
		uzudtg_dv_total_reg.uzudtg_dv_total = CAMERA_FLOW_UZU_VTOTAL-1;
		printk("[camera]PPOVERLAY_uzudtg_DV_TOTAL_reg (%d)@%s/%d\n", uzudtg_dv_total_reg.regValue,__FUNCTION__,__LINE__);
		}*/
		IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);
	}
	vbe_disp_pwm_frequency_update(FALSE);
	if(Get_DISPLAY_REFRESH_RATE() == 60){
		IoReg_Mask32(LDSPI_ld_spi_v_total_reg, 0x7ffff, ((vertical_total/2)-1)<<19); // repeat local dimming vsync
		rtd_pr_vbe_notice("vertical_total=%x, 0xb802c96c = %x\n", vertical_total, IoReg_Read32(LDSPI_ld_spi_v_total_reg));
	}
	ppoverlay_dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
	ppoverlay_dh_total_last_line_length_reg.dh_total = (Get_DISP_HORIZONTAL_TOTAL() - 1);
	ppoverlay_dh_total_last_line_length_reg.dh_total_last_line = (Get_DISP_HORIZONTAL_TOTAL() - 1);
	IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, ppoverlay_dh_total_last_line_length_reg.regValue);

	uzudtg_dh_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DH_TOTAL_reg);
//#ifdef CONFIG_MEMC_BYPASS
	if(get_MEMC_bypass_status_refer_platform_model() == TRUE){
		uzudtg_dh_total_reg.uzudtg_dh_total = (Get_DISP_HORIZONTAL_TOTAL()-1)/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
		uzudtg_dh_total_reg.uzudtg_dh_total_last_line = (Get_DISP_HORIZONTAL_TOTAL()-1)/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
//#else
	}else{
	#ifndef BUILD_QUICK_SHOW 
		if(Get_MEMC_Enable_Dynamic() == 0){
			uzudtg_dh_total_reg.uzudtg_dh_total = (Get_DISP_HORIZONTAL_TOTAL()-1)/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
			uzudtg_dh_total_reg.uzudtg_dh_total_last_line = (Get_DISP_HORIZONTAL_TOTAL()-1)/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
		}else
    #endif        
            {
			uzudtg_dh_total_reg.uzudtg_dh_total = (Get_DISP_HORIZONTAL_TOTAL()-1);
			uzudtg_dh_total_reg.uzudtg_dh_total_last_line = (Get_DISP_HORIZONTAL_TOTAL()-1);
		}
	}
//#endif
/*camera 120hz
	if(get_vo_camera_flow_flag(Get_DisplayMode_Port(SLR_MAIN_DISPLAY))==1){
		printk("[camera]uzudtg_dh_total (%d)@%s/%d\n", CAMERA_FLOW_UZU_HTOTAL-1,__FUNCTION__,__LINE__);
		uzudtg_dh_total_reg.uzudtg_dh_total = CAMERA_FLOW_UZU_HTOTAL-1;
		uzudtg_dh_total_reg.uzudtg_dh_total_last_line = CAMERA_FLOW_UZU_HTOTAL-1;
	}*/
	IoReg_Write32(PPOVERLAY_uzudtg_DH_TOTAL_reg, uzudtg_dh_total_reg.regValue);
	dh_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DH_DEN_Start_End_reg);
//#ifdef CONFIG_MEMC_BYPASS
	if(get_MEMC_bypass_status_refer_platform_model() == TRUE){
		dh_den_start_end_reg.dh_den_sta = Get_DISP_DEN_STA_HPOS()/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
		dh_den_start_end_reg.dh_den_end = Get_DISP_DEN_END_HPOS()/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
//#else
	}else{
	#ifndef BUILD_QUICK_SHOW 
	if(Get_MEMC_Enable_Dynamic() == 0){
			dh_den_start_end_reg.dh_den_sta = Get_DISP_DEN_STA_HPOS()/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
			dh_den_start_end_reg.dh_den_end = Get_DISP_DEN_END_HPOS()/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
		}else
    #endif        
            {
			dh_den_start_end_reg.dh_den_sta = Get_DISP_DEN_STA_HPOS();
			dh_den_start_end_reg.dh_den_end = Get_DISP_DEN_END_HPOS();
		}
	}
//#endif

	IoReg_Write32(PPOVERLAY_DH_DEN_Start_End_reg, dh_den_start_end_reg.regValue);
	dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	dv_den_start_end_reg.dv_den_sta = Get_DISP_DEN_STA_VPOS();
	dv_den_start_end_reg.dv_den_end = Get_DISP_DEN_END_VPOS();
    if((Get_DISP_DEN_END_VPOS() - Get_DISP_DEN_STA_VPOS()) == 1080)
    {
    //    dv_den_start_end_reg.dv_den_end = Get_DISP_DEN_END_VPOS() + 8;
    }
	IoReg_Write32(PPOVERLAY_DV_DEN_Start_End_reg, dv_den_start_end_reg.regValue);
	uzudtg_countrol_reg.regValue = IoReg_Read32(PPOVERLAY_UZUDTG_COUNTROL_reg);
	uzudtg_countrol_reg.uzudtg_en = 1;
	uzudtg_countrol_reg.uzudtg_fsync_en = 1;
	IoReg_Write32(PPOVERLAY_UZUDTG_COUNTROL_reg, uzudtg_countrol_reg.regValue);
	ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

	/*setup memc dtg.*/
	memcdtg_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
	memcdtg_dv_total_reg.memcdtg_dv_total = vertical_total-1;
	IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.regValue);
	memcdtg_dh_total_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DH_TOTAL_reg);
	memcdtg_dh_total_reg.memcdtg_dh_total = Get_DISP_HORIZONTAL_TOTAL()-1;
	memcdtg_dh_total_reg.memcdtg_dh_total_last_line = Get_DISP_HORIZONTAL_TOTAL()-1;
	IoReg_Write32(PPOVERLAY_memcdtg_DH_TOTAL_reg, memcdtg_dh_total_reg.regValue);
	memcdtg_dh_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg);
	memcdtg_dh_den_start_end_reg.memcdtg_dh_den_sta = Get_DISP_DEN_STA_HPOS();
	memcdtg_dh_den_start_end_reg.memcdtg_dh_den_end = Get_DISP_DEN_END_HPOS();
	IoReg_Write32(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg, memcdtg_dh_den_start_end_reg.regValue);
	memcdtg_dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg);

    memcdtg_dv_den_start_end_reg.memcdtg_dv_den_sta = Get_DISP_DEN_STA_VPOS();
    memcdtg_dv_den_start_end_reg.memcdtg_dv_den_end = Get_DISP_DEN_END_VPOS();

	IoReg_Write32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg, memcdtg_dv_den_start_end_reg.regValue);

    memcdtg_control_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_reg);
    memcdtg_control_reg.memcdtg_en = 1;
    IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL_reg, memcdtg_control_reg.regValue);

    if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI_MEMC_FREERUN) 
            || (get_vo_camera_flow_flag(Get_DisplayMode_Port(SLR_MAIN_DISPLAY))==1) || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA)
    {
        //frc
        drvif_scaler_memc_set_display_mode_frc2fsync_by_hw_speedup(0);
    }
    else
    {
        //frc2fs
        drvif_scaler_memc_set_display_mode_frc2fsync_by_hw_speedup(1);
    }
    
    ppoverlay_dctl_iv2dv_ihs_ctrl_reg.iv2dv_ih_slf_gen_en = 1;
	if(VGIP_Data_Path_Select_get_main_2pxl_en(IoReg_Read32(VGIP_Data_Path_Select_reg)))
		ppoverlay_dctl_iv2dv_ihs_ctrl_reg.iv2dv_ih_slf_width = Scaler_DispGetInputInfo(SLR_INPUT_H_LEN) / 2;
	else
		ppoverlay_dctl_iv2dv_ihs_ctrl_reg.iv2dv_ih_slf_width = Scaler_DispGetInputInfo(SLR_INPUT_H_LEN);
	IoReg_Write32(PPOVERLAY_dctl_iv2dv_ihs_ctrl_reg, ppoverlay_dctl_iv2dv_ihs_ctrl_reg.regValue);
	if(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI_MEMC_FREERUN){
		ppoverlay_sram_ok_sub_result_even_reg.regValue = IoReg_Read32(PPOVERLAY_SRAM_OK_sub_Result_even_reg);
		ppoverlay_sram_ok_sub_result_even_reg.s2_align_memc4p_en = 1;
		IoReg_Write32(PPOVERLAY_SRAM_OK_sub_Result_even_reg, ppoverlay_sram_ok_sub_result_even_reg.regValue);
	}
	// update sfg for vrr
	vbe_disp_set_vrr_average_den_mode(0);
	vbe_disp_set_vrr_average_den_update(1);
	/*set memc den settings==>*/

	//Update MEMC Htotal
	Scaler_MEMC_Set_DISP_DHTOTAL(Get_DISP_HORIZONTAL_TOTAL());
	Scaler_set_MEMC_change_size();
}

void modestate_set_display_timing_gen_htotal_ByDisp(unsigned char _channel)
{
	ppoverlay_uzudtg_dh_total_RBUS uzudtg_dh_total_reg;
	ppoverlay_dh_total_last_line_length_RBUS ppoverlay_dh_total_last_line_length_reg;

	ppoverlay_dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
	ppoverlay_dh_total_last_line_length_reg.dh_total = (Get_DISP_HORIZONTAL_TOTAL() - 1);
	ppoverlay_dh_total_last_line_length_reg.dh_total_last_line = (Get_DISP_HORIZONTAL_TOTAL() - 1);
	IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, ppoverlay_dh_total_last_line_length_reg.regValue);

	uzudtg_dh_total_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_DH_TOTAL_reg);
//#ifdef CONFIG_MEMC_BYPASS
	if(get_MEMC_bypass_status_refer_platform_model() == TRUE){
      uzudtg_dh_total_reg.uzudtg_dh_total = (Get_DISP_HORIZONTAL_TOTAL()-1)/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
      uzudtg_dh_total_reg.uzudtg_dh_total_last_line = (Get_DISP_HORIZONTAL_TOTAL()-1)/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
	}else{
//#else
	if(Get_MEMC_Enable_Dynamic() == 0){
		uzudtg_dh_total_reg.uzudtg_dh_total = (Get_DISP_HORIZONTAL_TOTAL()-1)/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
		uzudtg_dh_total_reg.uzudtg_dh_total_last_line = (Get_DISP_HORIZONTAL_TOTAL()-1)/(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) +1);
	}else{
		uzudtg_dh_total_reg.uzudtg_dh_total = (Get_DISP_HORIZONTAL_TOTAL()-1);
		uzudtg_dh_total_reg.uzudtg_dh_total_last_line = (Get_DISP_HORIZONTAL_TOTAL()-1);
	}
#if 0 //remove hack uzu 300MHz code
	if((Get_DISPLAY_REFRESH_RATE()==120)&&(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)>235)&&(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)<245)){
		//In order to keep framerate not change, we will make each cost time of vertical line is the same.
		// (1/240M) * 4400 = (1/300M) * 5500
		rtd_pr_vbe_notice("120HZ panel, input 24HZ, Hack htotal as %d.\n", HTOTAL_MAPPING_MEMC_CLOCK_INCREASE);
		uzudtg_dh_total_reg.uzudtg_dh_total = (HTOTAL_MAPPING_MEMC_CLOCK_INCREASE-1);
		uzudtg_dh_total_reg.uzudtg_dh_total_last_line = (HTOTAL_MAPPING_MEMC_CLOCK_INCREASE-1);
	}
#endif
	}
//#endif
	IoReg_Write32(PPOVERLAY_uzudtg_DH_TOTAL_reg, uzudtg_dh_total_reg.regValue);

}
#ifndef BUILD_QUICK_SHOW 

void modestate_set_display_timing_gen_clock_div(void)
{
	drvif_clock_set_dtg_uzu_div();
	drvif_clock_set_dtg_memc_div();
}
#endif
void modestate_set_display_timing_gen_clock_divByDisp(unsigned char _channel)
{
	drvif_clock_set_dtg_uzu_divByDisp(_channel);
	drvif_clock_set_dtg_memc_div();
}
#ifndef BUILD_QUICK_SHOW 

void modestate_set_lowdelay_display_timing_gen_clock(unsigned char _channel, unsigned char lowdelay_en)
{
    drvif_clock_set_lowdelay_dtg_uzu(_channel, lowdelay_en);
    drvif_clock_set_lowdelay_dtg_memc_div(lowdelay_en);
}

void modestate_set_fractional_framesync(void)
{
	ppoverlay_dtg_m_remove_input_vsync_RBUS dtg_m_remove_input_vsync_reg;
	ppoverlay_dtg_m_multiple_vsync_RBUS dtg_m_multiple_vsync_reg;
//#ifndef CONFIG_MEMC_BYPASS
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	unsigned char multiple_ratio=0, remove_ratio=0;
//#endif

	dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
	dtg_m_remove_input_vsync_reg.remove_half_ivs = 0; //mode2
	dtg_m_remove_input_vsync_reg.remove_half_ivs_mode2 = (Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO)-1);
	IoReg_Write32(PPOVERLAY_DTG_M_Remove_input_vsync_reg, dtg_m_remove_input_vsync_reg.regValue);
	rtd_pr_vbe_debug("dtg_m_remove_input_vsync_reg : %x\n", dtg_m_remove_input_vsync_reg.regValue);

	dtg_m_multiple_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_multiple_vsync_reg);
	dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync = (Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO)-1);
	IoReg_Write32(PPOVERLAY_DTG_M_multiple_vsync_reg, dtg_m_multiple_vsync_reg.regValue);
	rtd_pr_vbe_debug("dtg_m_multiple_vsync_reg : %x\n", dtg_m_multiple_vsync_reg.regValue);

//#ifdef CONFIG_MEMC_BYPASS
	if(get_MEMC_bypass_status_refer_platform_model() == TRUE){
	// don't set uzudtg fractional fs.
	rtd_pr_vbe_debug("MEMC bypass !! Don't set uzudtg fractional fs. \n");
	}else{
//#else

	multiple_ratio = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO);
	remove_ratio = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO);

	if (((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)) &&
		(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495)) {
		multiple_ratio = 1;
		remove_ratio = 1;
	}

	if(!get_scaler_qms_mode_flag() && ((multiple_ratio==5) && (remove_ratio==1))){ //qms can not use 2:10, becasue can not delay apply
		multiple_ratio = 10;
		remove_ratio = 2;
	}

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs = 1; // enable;
#ifdef SMOOTH_TOGGLE_DATAFRCFS_TEST
	//adaptive stream run timing frame sync so it should align IVS @Crixus 20160202
	if((vbe_disp_get_adaptivestream_fs_mode() == 1) || (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))){
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
	}else{
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 0; // align to ivs
	}
#else
	if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
	else
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 0; // align to ivs
#endif
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = (multiple_ratio-1);
	uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = (remove_ratio-1);
	IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);
	rtd_pr_vbe_debug("uzudtg_fractional_fsync_reg : %x\n", uzudtg_fractional_fsync_reg.regValue);
	}
//#endif
}
void modestate_set_lowdelay_fractional_framesync(unsigned char _channel, unsigned int lowdelay_en)
{
    SCALER_DISP_CHANNEL channel = (SCALER_DISP_CHANNEL)_channel;
    ppoverlay_dtg_m_remove_input_vsync_RBUS dtg_m_remove_input_vsync_reg;
    ppoverlay_dtg_m_multiple_vsync_RBUS dtg_m_multiple_vsync_reg;
//#ifndef CONFIG_MEMC_BYPASS
    ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
    unsigned char multiple_ratio=0, remove_ratio=0;
//#endif
    dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
    dtg_m_remove_input_vsync_reg.remove_half_ivs = 0; //mode2
    if(lowdelay_en == 1)
        dtg_m_remove_input_vsync_reg.remove_half_ivs_mode2 = 0;
    else
        dtg_m_remove_input_vsync_reg.remove_half_ivs_mode2 = (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO)-1);
    IoReg_Write32(PPOVERLAY_DTG_M_Remove_input_vsync_reg, dtg_m_remove_input_vsync_reg.regValue);
    rtd_pr_vbe_debug("dtg_m_remove_input_vsync_reg : %x\n", dtg_m_remove_input_vsync_reg.regValue);

    dtg_m_multiple_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_multiple_vsync_reg);
    if(lowdelay_en == 1)
        dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync = 0;
    else
        dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync = (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO)-1);
    IoReg_Write32(PPOVERLAY_DTG_M_multiple_vsync_reg, dtg_m_multiple_vsync_reg.regValue);
    rtd_pr_vbe_debug("dtg_m_multiple_vsync_reg : %x\n", dtg_m_multiple_vsync_reg.regValue);

//#ifdef CONFIG_MEMC_BYPASS
	if(get_MEMC_bypass_status_refer_platform_model() == TRUE){
    // don't set uzudtg fractional fs.
    rtd_pr_vbe_debug("MEMC bypass !! Don't set uzudtg fractional fs. \n");
//#else
	}else{
    if(lowdelay_en == 1){
        multiple_ratio = 1;
        remove_ratio = 1;
    }
    else{
        multiple_ratio = Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO);
        remove_ratio = Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO);
    }

    if (((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)) &&
        (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495)) {
        multiple_ratio = 1;
        remove_ratio = 1;
    }

    if(!get_scaler_qms_mode_flag() && ((multiple_ratio==5) && (remove_ratio==1))){//qms can not use 2:10, becasue can not delay apply
        multiple_ratio = 10;
        remove_ratio = 2;
    }

    uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
    uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs = 1; // enable;

    if(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAMESYNC))
        uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
    else
        uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 0; // align to ivs

    uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = (multiple_ratio-1);
    uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = (remove_ratio-1);
    IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);
    rtd_pr_vbe_debug("uzudtg_fractional_fsync_reg : %x\n", uzudtg_fractional_fsync_reg.regValue);
//#endif
		}
}
#endif

void modestate_set_fractional_framesyncByDisp(unsigned char _channel)
{
	SCALER_DISP_CHANNEL channel = (SCALER_DISP_CHANNEL)_channel;
	ppoverlay_dtg_m_remove_input_vsync_RBUS dtg_m_remove_input_vsync_reg;
	ppoverlay_dtg_m_multiple_vsync_RBUS dtg_m_multiple_vsync_reg;
//#ifndef CONFIG_MEMC_BYPASS
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	unsigned char multiple_ratio=0, remove_ratio=0;
//#endif
	dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
	dtg_m_remove_input_vsync_reg.remove_half_ivs = 0; //mode2
	dtg_m_remove_input_vsync_reg.remove_half_ivs_mode2 = (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO)-1);
	IoReg_Write32(PPOVERLAY_DTG_M_Remove_input_vsync_reg, dtg_m_remove_input_vsync_reg.regValue);
	rtd_pr_vbe_debug("dtg_m_remove_input_vsync_reg : %x\n", dtg_m_remove_input_vsync_reg.regValue);

	dtg_m_multiple_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_multiple_vsync_reg);
	dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync = (Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO)-1);
	IoReg_Write32(PPOVERLAY_DTG_M_multiple_vsync_reg, dtg_m_multiple_vsync_reg.regValue);
	rtd_pr_vbe_debug("dtg_m_multiple_vsync_reg : %x\n", dtg_m_multiple_vsync_reg.regValue);

//#ifdef CONFIG_MEMC_BYPASS
	if(get_MEMC_bypass_status_refer_platform_model() == TRUE){
	// don't set uzudtg fractional fs.
	rtd_pr_vbe_debug("MEMC bypass !! Don't set uzudtg fractional fs. \n");
//#else
	}else{
	multiple_ratio = Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO);
	remove_ratio = Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_REMOVE_RATIO);

	if (((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)) &&
		(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495)) {
		multiple_ratio = 1;
		remove_ratio = 1;
	}

	if(!get_scaler_qms_mode_flag() && ((multiple_ratio==5) && (remove_ratio==1))){//qms can not use 2:10, becasue can not delay apply
		multiple_ratio = 10;
		remove_ratio = 2;
	}

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs = 1; // enable;

	if(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAMESYNC))
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
	else
		uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 0; // align to ivs

	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = (multiple_ratio-1);
	uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = (remove_ratio-1);
/* camera 120hz
	if(get_vo_camera_flow_flag(Get_DisplayMode_Port(SLR_MAIN_DISPLAY))==1)
		uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = 1;
	printk("uzudtg_multiple_vsyn=%d@%s/%d\n",uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync,__FUNCTION__,__LINE__);
*/
	IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);
	rtd_pr_vbe_debug("uzudtg_fractional_fsync_reg : %x\n", uzudtg_fractional_fsync_reg.regValue);
	}
//#endif
}

void modestate_set_memc_pixel_mode(void)
{
#if 0//fixme k4l bringup no dout_pixel_mode item

	ppoverlay_display_timing_ctrl2_RBUS 	display_timing_ctrl2_reg;
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	display_timing_ctrl2_reg.dout_pixel_mode = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE);
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
#endif
}

void modestate_disp_flow_double_buf_enable(unsigned char enable)
{
	fw_scaler_dtg_double_buffer_enable(enable);
}

void modestate_disp_flow_double_buf_apply(void)
{
	ppoverlay_dispd_smooth_toggle1_RBUS	dispd_smooth_toggle1_reg;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS 	dispd_stage1_sm_ctrl_reg;
	unsigned int timeout = 0x3fffff;


	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
	IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);

	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	do{
		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	}while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply==1) && (--timeout));

	if(timeout==0){
		rtd_pr_vbe_notice("[%s][%d]WARNING! wait dispd_stage1_smooth_toggle_apply timeout!\n", __FUNCTION__, __LINE__);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 0;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
		dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
		dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
		IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	}else{
		dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
		dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
		IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
	}
	fw_scaler_dtg_double_buffer_apply();
}

//========================================================================
//						VDC 27MHz Function Start Function Start
//========================================================================
static UINT8 ucVDC_27MHz_Enable = _DISABLE;
static UINT32 VDC_27MHz_SD_Width = 960;
//static UINT32 VDC_27MHz_SD_Width = 0; //short time, need modify late

//USER:LewisLee DATE:2013/03/18
//modify VDC 27MHz mode function
void Scaler_Set_VDC_27MHz_Mode_Enable(UINT8 ucEnable)
{
	ucVDC_27MHz_Enable = ucEnable;
}

UINT8 Scaler_Get_VDC_27MHz_Mode_Enable(void)
{
#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
	if(_DISABLE == ucVDC_27MHz_Enable)
		return _DISABLE;

	return _ENABLE;
#else //#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
	return _DISABLE;
#endif //#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
}

#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
VD_27M_OVERSAMPLING_MODE Scaler_Enter_VDC_27MHz_Mode_Condition(SCALER_DISP_CHANNEL Display)
{
	VD_27M_OVERSAMPLING_MODE VDC_27MHz_Mode_Enable = VD_27M_OFF;

	if(_DISABLE == Scaler_Get_VDC_27MHz_Mode_Enable())
		return VD_27M_OFF;

	switch(Scaler_InputSrcGetType(Display))
	{
		case _SRC_CVBS:
		case _SRC_SV:
		case _SRC_TV:
			VDC_27MHz_Mode_Enable = VD_27M_HSD960_DI5A;
		break;

		default:
			VDC_27MHz_Mode_Enable = VD_27M_OFF;
		break;
	}

/*
	if(VD_27M_OFF == VDC_27MHz_Mode_Enable)
	{
		//not need to check
	}
	else// if(VD_27M_OFF != VDC_27MHz_Mode_Enable)
	{
		if(SLR_RATIO_FIT == Scaler_DispGetRatioMode2())
			VDC_27MHz_Mode_Enable = VD_27M_OFF;
	}
*/
	return VDC_27MHz_Mode_Enable;

}

//USER:LewisLee DATE:2013/10/09
//set target scaling down size
void Scaler_Set_VDC_27MHz_Mode_SD_Width(UINT32 ScalingDown_Width)
{
	VDC_27MHz_SD_Width = ScalingDown_Width;
}

UINT32 Scaler_Get_VDC_27MHz_Mode_SD_Width(void)
{
//#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
	if(_DISABLE == ucVDC_27MHz_Enable)
		return 0;

	return VDC_27MHz_SD_Width;
//#endif //#ifdef CONFIG_ENABLE_VD_27MHZ_MODE

	//return 0;
}

void fw_video_set_27mhz_mode(VD_27M_OVERSAMPLING_MODE mode)
{
//#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
	VD_27M_mode = mode;
//#endif //#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
}

VD_27M_OVERSAMPLING_MODE fw_video_get_27mhz_mode(SCALER_DISP_CHANNEL Display)
{
//#ifdef CONFIG_ENABLE_VD_27MHZ_MODE

	rtd_pr_vbe_debug("fw_video_get_27mhz_mode  src=%d\n", Scaler_InputSrcGetType(Display));
	rtd_pr_vbe_debug("VD_27M_mode=%d\n", VD_27M_mode);
	rtd_pr_vbe_debug("Display=%d\n", Display);
#ifdef CONFIG_I2RND_ENABLE
	//[ANDROIDTV-3562]i2run case, if main is AVD source, need to check sub source @Crixus 20171025
	if((Display == SLR_MAIN_DISPLAY)
		&& ((Scaler_I2rnd_get_timing_enable() && (vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1))
		|| ((Scaler_I2rnd_get_timing_enable() == _ENABLE) && (vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC)))
		&& (Get_DisplayMode_Src(SLR_SUB_DISPLAY) == VSC_INPUTSRC_VDEC)){

		rtd_pr_vbe_emerg("[crixus]i2r PIP case, if main is AVD, need to check sub really source for re-run main.\n");
		return VD_27M_OFF;
	}
#endif
	if((Scaler_InputSrcGetType(Display)==_SRC_CVBS)||(Scaler_InputSrcGetType(Display)==_SRC_TV)||(Scaler_InputSrcGetType(Display)==_SRC_SV) ){
		return VD_27M_mode;
	}
#ifdef CONFIG_FORCE_RUN_I3DDMA
	else if ((get_force_i3ddma_enable(Display) == true) && (DMA_VGIP_DMA_VGIP_CTRL_get_dma_in_sel(rtd_inl(DMA_VGIP_DMA_VGIP_CTRL_reg)) == VGIP_SRC_ATV)) {
		return VD_27M_mode;
	}
#endif
	else
	{
		return VD_27M_OFF;
	}
}

VD_27M_OVERSAMPLING_MODE fw_video_check_enter_27mhz_mode_condition(SCALER_DISP_CHANNEL Display)
{
//#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
	VD_27M_mode = Scaler_Enter_VDC_27MHz_Mode_Condition(Display);

	return fw_video_get_27mhz_mode(SLR_MAIN_DISPLAY);
//#else //#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
//	return VD_27M_OFF;
//#endif //#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
}
#endif //#ifdef CONFIG_ENABLE_VD_27MHZ_MODE

//========================================================================
//						VDC 27MHz Function Start Function End
//========================================================================

// *i3dForce2d return 0: disable, 1: L, 2: R
#if defined(CONFIG_ENABLE_HDR10_HDMI_AUTO_DETECT)|| defined(CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT)
 static void drivf_I3DDMA_Get_3D_Format(I3DDMA_3D_T *i3dFmt, VO_FORCE2D_MODE *i3dForce2d)
 {
	 I3DDMA_3D_T i3ddma_3dMode=I3DDMA_2D_ONLY;//I3DDMA_CHECKER_BOARD;
	 VO_FORCE2D_MODE force2dFlag = VO_FORCE2D_DISABLE;
	 SLR_3D_TYPE cur3dType;

	 if(!i3dFmt || !i3dForce2d){
		 rtd_pr_vbe_debug("ERR: NULL PTR@%s, %ld/%ld\n", __FUNCTION__, (unsigned long)i3dFmt, (unsigned long)i3dForce2d);
		 return;
	 }


#if 1//def ENABLE_DRIVER_I3DDMA
	 cur3dType =  (SLR_3D_TYPE)Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE);

	 //SLR_3D_TYPE dbg3dType = DBG_get_dbg3dType();

	// if(dbg3dType != SLR_3D_2D_ONLY)
	//	 cur3dType = dbg3dType;

	 // assign I3DDMA 3D format
	 switch(cur3dType){
	   case SLR_3D_FRAME_PACKING:
	   case SLR_3D_FRAME_PACKING_CVT_2D:
		 i3ddma_3dMode = I3DDMA_FRAME_PACKING;
		 break;
	   case SLR_3D_FIELD_ALTERNATIVE:
		 i3ddma_3dMode = I3DDMA_FIELD_ALTERNATIVE;
		 break;
	   case SLR_3D_LINE_ALTERNATIVE:
	   case SLR_3D_LINE_ALTERNATIVE_CVT_2D:
		 i3ddma_3dMode = I3DDMA_LINE_ALTERNATIVE;
		 break;
	   case SLR_3D_TOP_AND_BOTTOM:
	   case SLR_3D_TOP_AND_BOTTOM_CVT_2D:
		 i3ddma_3dMode = I3DDMA_TOP_AND_BOTTOM;
		 break;
	   case SLR_3D_SIDE_BY_SIDE_HALF:
	   case SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D:
		 i3ddma_3dMode = I3DDMA_SIDE_BY_SIDE_HALF;
		 break;
	   case SLR_3D_VERTICAL_STRIP:
	   case SLR_3D_VERTICAL_STRIP_CVT_2D:
		 i3ddma_3dMode = I3DDMA_VERTICAL_STRIPE;
		 break;
	   case SLR_3D_CHECKER_BOARD:
	   case SLR_3D_CHECKER_BOARD_CVT_2D:
		 i3ddma_3dMode = I3DDMA_CHECKER_BOARD;
		 break;
	   case SLR_3D_REALID:
	   case SLR_3D_REALID_CVT_2D:
		 i3ddma_3dMode = I3DDMA_REALID;
		 break;
	   case SLR_3D_SENSIO:
	   case SLR_3D_SENSIO_CVT_2D:
		 i3ddma_3dMode = I3DDMA_SENSIO;
		 break;
	   case SLR_3D_2D_CVT_3D:
		 i3ddma_3dMode = I3DDMA_FRAMESEQUENCE;
		 break;
	   default:
		 break;
	 }

#if 0
	 // 3Dto2D L/R frame select
	 if((cur3dType >= SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D) && (cur3dType <= SLR_3D_SENSIO_CVT_2D))
		 force2dFlag = ((Scaler_Get3DCvt2D_LR() ==0)? VO_FORCE2D_RIGHT: VO_FORCE2D_LEFT);
#endif
	 rtd_pr_vbe_debug("[IDMA] 3D Mode/type=%d/%d->%d, 3Dto2D=%d\n", Scaler_Get3DMode((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)), cur3dType, i3ddma_3dMode, force2dFlag);
#endif

	 *i3dFmt = i3ddma_3dMode;
	 *i3dForce2d = force2dFlag;

	 return;
 }
#endif

 void I3DDMA_Set_HDMI3D_Vgip(void);


 int drivf_I3DDMA_3D_config(void)
{
	rtd_pr_vbe_debug("[IDMA] I3DDMA_config\n");
#if defined(CONFIG_ENABLE_HDR10_HDMI_AUTO_DETECT)|| defined(CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT)
	if(Scaler_DispGetInputInfo(SLR_INPUT_THROUGH_I3DDMA) == TRUE){
		// I3DDMA init and let VO run
		if((Scaler_InputSrcGetMainChType() != _SRC_VO)|| (Scaler_Get_CurVoInfo_plane() == VO_VIDEO_PLANE_V2) || get_line_alternative_3d_mode_enable() || (Scaler_Hdmi_DM_Get_Status() == SLR_HDMI_DM_CONNECT)){
			I3DDMA_TIMING_T i3ddma_timing;
//			UINT32 i = 0, j=0, tmp=0;
			//UINT32 addr = 0x36d3d000;
			I3DDMA_3D_T i3dFmt;
			VO_FORCE2D_MODE i3dForce2d;

			drivf_I3DDMA_Get_3D_Format(&i3dFmt, &i3dForce2d);

			memset(&i3ddma_timing, 0 , sizeof(I3DDMA_TIMING_T));
			// VO source Sensio 3D mode force to chroma YUV422
			if((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_Get_CurVoInfo_plane() == VO_VIDEO_PLANE_V2)){
				i3ddma_timing.color = (I3DDMA_COLOR_SPACE_T)(i3dFmt == I3DDMA_SENSIO? COLOR_YUV422: COLOR_YUV444);
				//rtd_pr_vbe_info("[IDMA] VO Chroma=%d@1\n", i3ddma_timing.color);
			}else{
				i3ddma_timing.color = (I3DDMA_COLOR_SPACE_T) ((Scaler_InputSrcGetMainChType() == _SRC_YPBPR)? COLOR_YUV444: Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE));//hdmi.tx_timing.color;
			}

			i3ddma_timing.depth = (I3DDMA_COLOR_DEPTH_T) Scaler_DispGetInputInfo(SLR_INPUT_COLOR_DEPTH);//hdmi.tx_timing.depth;
			i3ddma_timing.colorimetry = (I3DDMA_COLORIMETRY_T) Scaler_DispGetInputInfo(SLR_INPUT_COLOR_IMETRY);//hdmi.tx_timing.colorimetry;

			i3ddma_timing.h_act_len = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE); //hdmi.tx_timing.h_act_len;
			i3ddma_timing.h_act_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE); //hdmi.tx_timing.h_act_sta;
			i3ddma_timing.v_act_len = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);//hdmi.tx_timing.v_act_len;
			i3ddma_timing.v_act_sta =  Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE); //Scaler_ModeGetModeInfo(SLR_MODE_IVSTARTPOS); //hdmi.tx_timing.v_act_sta;

                        if( Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI )
                          i3ddma_timing.h_freq = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHFREQ_NEW);  // hdmi.tx_timing.h_freq ( 0.001Hz )
                        else
			  i3ddma_timing.h_freq = Scaler_DispGetInputInfo(SLR_INPUT_H_FREQ) * 100;

			i3ddma_timing.h_total = Scaler_DispGetInputInfo(SLR_INPUT_H_LEN); //hdmi.tx_timing.h_total;
			i3ddma_timing.i3ddma_3dformat = i3dFmt; //I3DDMA_CHECKER_BOARD; //(I3DDMA_3D_T) drvif_Hdmi_GetReal3DFomat();//hdmi.tx_timing.hdmi_3dformat;
			i3ddma_timing.i3ddma_force2d = i3dForce2d; // 0: disable, 1: L-frame, 2: R-frame
			i3ddma_timing.polarity = Scaler_DispGetInputInfo(SLR_INPUT_POLARITY); //hdmi.tx_timing.polarity;
			i3ddma_timing.progressive = Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_INTERLACE)?0:1; //hdmi.tx_timing.progressive;
			i3ddma_timing.v_active_space1 = 0; //hdmi.tx_timing.v_active_space1; //get from I3DDMA_MeasureActiveSpace
			i3ddma_timing.v_active_space2 = 0; //hdmi.tx_timing.v_active_space2; //get from I3DDMA_MeasureActiveSpace

                        if( Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI )
                          i3ddma_timing.v_freq = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW);  // hdmi.tx_timing.v_freq ( 0.001Hz )
                        else
                          i3ddma_timing.v_freq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) * 100;

			i3ddma_timing.v_total = Scaler_DispGetInputInfo(SLR_INPUT_V_LEN); //hdmi.tx_timing.v_total;

			// [FIX-ME][#0055901] IDMA 3D mode picture width align to 8
			if((i3ddma_timing.h_act_len % 8) != 0){
			  unsigned int diff = i3ddma_timing.h_act_len - (i3ddma_timing.h_act_len & ~0x7);
			  i3ddma_timing.h_act_len -= diff;
			  i3ddma_timing.h_act_sta  += (diff/2);
			  rtd_pr_vbe_debug("\n[IDMA][SBS] @@ align 8 new h_adj wid/sta=%d/%d\n",i3ddma_timing.h_act_len, i3ddma_timing.h_act_sta);
			}

#if 1 // TEST
			rtd_pr_vbe_debug("[IDMA] C/D/CM/= %d/%d/%d\n", i3ddma_timing.color, i3ddma_timing.depth, i3ddma_timing.colorimetry);
			rtd_pr_vbe_debug("[IDMA] H/HST/HFre/HT=%d/%d/%d/%d\n", 	i3ddma_timing.h_act_len, i3ddma_timing.h_act_sta, i3ddma_timing.h_freq, i3ddma_timing.h_total);
			rtd_pr_vbe_debug("[IDMA] 3D/PO/Prog=%d/%d/%d\n", i3ddma_timing.i3ddma_3dformat , i3ddma_timing.polarity, i3ddma_timing.progressive);
			rtd_pr_vbe_debug("[IDMA] V/VST/VFRE/VT=%d/%d/%d/%d\n", i3ddma_timing.v_act_len, i3ddma_timing.v_act_sta , i3ddma_timing.v_freq, i3ddma_timing.v_total);
#endif
			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_I3DDMA_DMA, I3DDMA_3D_OPMODE_HW);
			rtd_pr_vbe_debug("################# 111 i3ddma_timing.v_freq : %d\n", i3ddma_timing.v_freq);
			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_TIMING, (unsigned long) &i3ddma_timing);
			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_IDX, Scaler_DispGetInputInfo(SLR_INPUT_INPUT_SRC));
			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE, Scaler_InputSrcGetMainChType());
			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_FROM, Scaler_InputSrcGetMainChFrom());
			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR, Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR));
			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_H_COUNT, Scaler_DispGetInputInfo(SLR_INPUT_H_COUNT));


			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_HSD_ENABLE, _DISABLE);
			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_H_SIZE, i3ddma_timing.h_act_len);
			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_OUTPUT_H_SIZE, i3ddma_timing.h_act_len);
			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_DITHER12X10_ENABLE, _DISABLE);

            // [WOSQRTK-10007][BW] Force enable HDMI 444(420) to 422!!
            if( ( i3ddma_timing.h_act_len >= 3800 ) && ( i3ddma_timing.v_freq > 48000 )
             && ( i3ddma_timing.color == I3DDMA_COLOR_YUV444 ) && ( i3ddma_timing.depth > 0 ) )
            {
                drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_4XXTO4XX_ENABLE, _ENABLE);
                rtd_pr_vbe_info("[IDMA.3D] Force enable HDMI 444(420) to 422!!\n");
            }else{
    			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_4XXTO4XX_ENABLE, _DISABLE);
            }

			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_DEPTH, I3DDMA_COLOR_DEPTH_10B);

			// In Vertical strip & Checker board 3D, I3DDMA only support YUV444 capture
			if((i3ddma_timing.i3ddma_3dformat == I3DDMA_VERTICAL_STRIPE)|| (i3ddma_timing.i3ddma_3dformat == I3DDMA_CHECKER_BOARD))
			{
				drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV444);
			} // [Dolby Vision] IDMA capture in RGB format
			else if((get_HDMI_HDR_mode() == HDR_DOLBY_HDMI) && (i3ddma_timing.color == I3DDMA_COLOR_RGB)){
				drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_RGB);
			}
			else if((get_HDMI_HDR_mode() == HDR_HDR10_HDMI) && (i3ddma_timing.color != I3DDMA_COLOR_RGB)){
				//drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV444);
				drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
			} else {
				drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
			}


			//drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_FRAMERATE_X2, true);

			I3DDMA_Set_HDMI3D_Vgip();
			drv_I3DDMA_ApplySetting(SLR_MAIN_DISPLAY);
/*
			rtd_maskl(0xb8025870,  ~_BIT0, 0);

			for(i=0; i<= (1920*1080*3) ; i+= (54*1920*3)){		// 10bit , 422

				if(j%2){
					tmp = 0x55555555;
					j=0;
				}
				else{
					tmp = 0xaaaaaaaa;
					j=1;
				}
				memset((void*) (addr+i),  tmp , 54*1920*3);
			}
			rtd_pr_vbe_info("ddr final addr = %x\n", addr+i);
*/
		}else {
  #if 1 // [FIX-ME]
  			// re-config VGIP
			dma_vgip_dma_vgip_act_hsta_width_RBUS dma_vgip_chn1_act_hsta_width_reg;
			dma_vgip_dma_vgip_act_vsta_length_RBUS dma_vgip_chn1_act_vsta_length_reg;
//			dma_vgip_dma_vgip_delay_RBUS dma_vgip_chn1_delay_reg;
			I3DDMA_TIMING_T *i3ddma_timing = (I3DDMA_TIMING_T*)drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_TIMING);

			// [FIX-ME] recover Main VGIP setting
			if(i3ddma_timing){
				dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
				dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);

				dma_vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
				dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_sta = i3ddma_timing->h_act_sta;
				dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid = i3ddma_timing->h_act_len;
				IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, dma_vgip_chn1_act_hsta_width_reg.regValue);

				dma_vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);
				dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_sta = i3ddma_timing->v_act_sta;
				dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len = i3ddma_timing->v_act_len;
				IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg, dma_vgip_chn1_act_vsta_length_reg.regValue);
#if 0
				if(drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE) == _SRC_YPBPR){
					unsigned int srcMode = drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR);
					dma_vgip_chn1_delay_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
					// [FIX-ME] YPbPr phase detect need to be fixed in 3D mode
					dma_vgip_chn1_delay_reg.dma_ivs_dly = 0 + Mode_Get_ypbpr_3D_adjustment_vStart(srcMode);
					dma_vgip_chn1_delay_reg.dma_ihs_dly = 0x35 + Mode_Get_ypbpr_3D_adjustment_hStart(srcMode);
					IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg, dma_vgip_chn1_delay_reg.regValue);
					rtd_pr_vbe_info("[IDMA] Force IH/IV_DLY=%x\n", IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg));
				}
#endif
				rtd_pr_vbe_info("[IDMA] TX.P=%d, HSta/Wid, VSta/Len=%d/%d, %d/%d, polarity=%x(%d)\n",
					i3ddma_timing->progressive,
					i3ddma_timing->h_act_sta, i3ddma_timing->h_act_len,
					i3ddma_timing->v_act_sta, i3ddma_timing->v_act_len,
					IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg) & 0xf, i3ddma_timing->polarity);

				// [IDMA] HDMI 1080i FP 3D Input is progressive format, however the output is interlaced format
				if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_INTERLACE) == TRUE){
#if 1 // [IDMA] field invert, why?
                 			if(1){//if(drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE) == _SRC_HDMI){
						//vgip_vgip_chn1_ctrl_RBUS vgip_chn1_ctrl_reg;
						// refer to fw_scaler_set_vgip_capture() setting, YPbPr source need enable ch1_odd_inv & ch1_odd_det_en
						//dma_vgip_chn1_ctrl_reg.dma_odd_inv = (drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE) == _SRC_YPBPR? _ENABLE: 0);
						//dma_vgip_chn1_ctrl_reg.dma_odd_det_en = (drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE) == _SRC_YPBPR? _ENABLE: 0);

						//dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CHN1_CTRL_VADDR);
						//dma_vgip_chn1_ctrl_reg.ch3_odd_sync_edge = 1; // [IDMA] from designer's recommendation
						//if(i3ddma_timing->progressive == FALSE) // HDMI 1080i FP 3D don't need do field invert
						//	dma_vgip_chn1_ctrl_reg.ch3_odd_inv = 1;
						//IoReg_Write32(DMA_VGIP_DMA_VGIP_CHN1_CTRL_VADDR, dma_vgip_chn1_ctrl_reg.regValue);
                 			}
#if 0
				   else if(drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE) == _SRC_VO){
					   dma_vgip_chn1_ctrl_RBUS dma_vgip_chn1_ctrl_reg;

					   dma_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CHN1_CTRL_VADDR);
					   //dma_vgip_chn1_ctrl_reg.ch3_odd_inv = 1;
					   dma_vgip_chn1_ctrl_reg.ch3_odd_sync_edge = 1;
					   IoReg_Write32(DMA_VGIP_DMA_VGIP_CHN1_CTRL_VADDR, dma_vgip_chn1_ctrl_reg.regValue);
				   }
 #endif
#endif
				}

				IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_chn1_ctrl_reg.regValue);
				drvif_I3DDMA_Enable_Vgip();
			}else
				rtd_pr_vbe_info("[IDMA] TX=NULL\n");

  #endif
			Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DISP_WID, _DISP_WID);
  #if 1 // TEST
  			if(i3ddma_timing){
	  			unsigned int dbgCnt=10;
				do{
					dma_vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
					dma_vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);

					rtd_pr_vbe_info("[IDMA] VGIP wid[%d]/len[%d]=%d/%d\n",
						i3ddma_timing->h_act_len, i3ddma_timing->v_act_len,
						dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid, dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len);

					if(dma_vgip_chn1_act_hsta_width_reg.dma_ih_act_wid != i3ddma_timing->h_act_len){
						rtd_pr_vbe_info("[IDMA] VGIP1 IH WRITE FAIL\n");
						//Rt_Sleep(1000);
						continue;
					}else if(dma_vgip_chn1_act_vsta_length_reg.dma_iv_act_len != i3ddma_timing->v_act_len){
						rtd_pr_vbe_info("[IDMA] VGIP1 IV WRITE FAIL\n");
						//Rt_Sleep(1000);
						continue;
					}else
						break;

				}while(dbgCnt--);
  			}
  #endif

			if(i3ddma_timing){
				I3DDMA_3D_T i3dFmt;
				VO_FORCE2D_MODE i3dForce2d;
				drivf_I3DDMA_Get_3D_Format(&i3dFmt, &i3dForce2d);

				// VO source Sensio 3D mode force to chroma YUV422
				if(drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE) == _SRC_VO)
				{
					i3ddma_timing->color = (I3DDMA_COLOR_SPACE_T)(i3dFmt == I3DDMA_SENSIO? COLOR_YUV422: COLOR_YUV444);
					//rtd_pr_vbe_info("[IDMA] VO Chroma=%d@2\n", i3ddma_timing->color);
				}

				// update 3D type
				i3ddma_timing->i3ddma_3dformat = i3dFmt;
				i3ddma_timing->i3ddma_force2d = i3dForce2d; // 0: disable, 1: L-frame, 2: R-frame
				drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_TIMING, (unsigned long)i3ddma_timing);

				// re-config color format to fix change 3D mode from other 3D mode without reset to 2D mode
				// In Vertical strip & Checker board 3D, I3DDMA only support YUV444 capture
				if((i3ddma_timing->i3ddma_3dformat == I3DDMA_VERTICAL_STRIPE)|| (i3ddma_timing->i3ddma_3dformat == I3DDMA_CHECKER_BOARD))
				{
					drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV444);
				}else{
					drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
				}
				// re-init VODMA
				drv_I3DDMA_ApplySetting(SLR_MAIN_DISPLAY);
			}
		}
	}
#endif
	return 0;
}

extern void drvif_color_Set_DRV_SNR_Clock(SCALER_DISP_CHANNEL display, unsigned char En_flag);
void force_enable_datapath_clock(void)
{//enable b8022284 related clock. current bit3 bit12 bit13
	unsigned char isAdaptivestream_flag = 0;
	unsigned int height = 0;
	unsigned int width = 0;
	unsigned char input_src_type = 0;

	unsigned char need_update = FALSE;
	vgip_data_path_select_RBUS data_path_select_reg;

	SCALER_DISP_CHANNEL display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);

	data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);

	if (display == SLR_MAIN_DISPLAY) {
		height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
		width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);

		input_src_type = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);

		if (input_src_type == VSC_INPUTSRC_VDEC || input_src_type == VSC_INPUTSRC_CAMERA)
			isAdaptivestream_flag = get_vsc_run_adaptive_stream(display);

		if(data_path_select_reg.xcdi_clk_en == 0)
		{
			/* ====20180610 for power issue, USB 4K60 disable RTNR CLK==== */
			if (isAdaptivestream_flag == 0)
				need_update = FALSE;
			else
				need_update = TRUE;
			/* ====20180610 for power issue, USB 4K60 disable RTNR CLK==== */

			data_path_select_reg.xcdi_clk_en = 1;
		}
		if(need_update)
			IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);

	}
#if 0	// control by "drvif_color_Set_DRV_SNR_Clock"
	if(data_path_select_reg.s2_clk_en == 0)
	{
		need_update = TRUE;
		data_path_select_reg.s2_clk_en = 1;
	}


	if(data_path_select_reg.s1_clk_en == 0)
	{
		//need_update = TRUE;
		//data_path_select_reg.s1_clk_en = 1;
		drvif_color_Set_DRV_SNR_Clock(display, 1);
	}
#endif

	drvif_color_Set_DRV_SNR_Clock(display, 1);

	rtd_pr_vbe_info("force en ,clk=%d,, input_src_type=%d, isAdaptivestream_flag=%d,, height=%d, width=%d,display=%d,\n",
		data_path_select_reg.xcdi_clk_en, input_src_type, isAdaptivestream_flag, height, width, display);

}


void fw_i3ddma_set_vgip(unsigned char src, unsigned char mode);
//extern void rtk_hal_vsc_i2rnd_enable(unsigned char enable);
#ifdef CONFIG_RTK_KDRV_DV
extern bool check_hdmi_dolby_vision_rgb_and_full_hd(void);
#endif
void set_i3ddma_compression_mode(void)
{
#ifdef BUILD_QUICK_SHOW
	if(drv_memory_get_game_mode() == _ENABLE) {
		dvrif_i3ddma_set_compression_mode(I3DDMA_COMP_LINE_MODE);
		rtd_pr_vbe_notice("[crixus]compression line mode\n");
	}
	else
#endif
	if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) <= 1920) &&
		(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) <= 1080) &&
		(get_current_driver_pattern(DRIVER_FREERUN_PATTERN) == 0) && (Get_PANEL_VFLIP_ENABLE() == 0))
	{
		if((get_rotate_mode(SLR_MAIN_DISPLAY) == DIRECT_VO_FRAME_ORIENTATION_ROTATE_180) && (Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE) == FALSE))
		{
			dvrif_i3ddma_set_compression_mode(I3DDMA_COMP_LINE_MODE);
		} else{
			dvrif_i3ddma_set_compression_mode(I3DDMA_COMP_FRAME_MODE);
		}
	}
	else{
		dvrif_i3ddma_set_compression_mode(I3DDMA_COMP_LINE_MODE);
	}
}
unsigned char drivf_i3ddma_config(unsigned char display, unsigned char src)
{
	I3DDMA_TIMING_T i3ddma_timing;
	unsigned char mode;
	h3ddma_h3ddma_pq_cmp_double_buffer_ctrl_RBUS h3ddma_h3ddma_pq_cmp_double_buffer_ctrl_reg;

	if((src != VGIP_SRC_TMDS
#ifdef CONFIG_SUPPORT_SRC_ADC
		&& src != VGIP_SRC_ADC
#endif//#ifdef CONFIG_SUPPORT_SRC_ADC
		&& src != VGIP_SRC_ATV && src != VGIP_SRC_HDR1)
		|| (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_MAXN)) {
		rtd_pr_vbe_emerg("drivf_i3ddma_config src error\n");
		return FALSE;
	}

	decide_i3ddma_2p_mode_on_off();
		
	//sync_pc_mode_info();//this position must need: vodma compress need check pc mode yuv444
	//disable i3dmma db
	h3ddma_h3ddma_pq_cmp_double_buffer_ctrl_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_DOUBLE_BUFFER_CTRL_reg);
	h3ddma_h3ddma_pq_cmp_double_buffer_ctrl_reg.db_en = 0;
	IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_DOUBLE_BUFFER_CTRL_reg, h3ddma_h3ddma_pq_cmp_double_buffer_ctrl_reg.regValue);

// timing dectect done
    scaler_vsc_set_external_src_vfreq(display, Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ));
    scaler_vsc_set_external_src_backporch(display, Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_STA_PRE));

    if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
	{
		vbe_disp_set_freesync_mode_flag(drvif_Hdmi_GetAMDFreeSyncEnable());//save current freesync mode
		vbe_disp_set_VRR_timingMode_flag(drvif_Hdmi_GetVRREnable());//save current vrr mode
	}
#if IS_ENABLED(CONFIG_RTK_DPRX)
	else if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP)
	{
		vbe_disp_set_freesync_mode_flag(drvif_Dprx_GetDrrMode() == DP_DRR_MODE_AMD_FREE_SYNC);//save current freesync mode
		vbe_disp_set_VRR_timingMode_flag(drvif_Dprx_GetDrrMode() == DP_DRR_MODE_ADAPTIVE_SYNC);//save current vrr mode
	}
#endif
	else
	{
		vbe_disp_set_freesync_mode_flag(0);//save current freesync mode
		vbe_disp_set_VRR_timingMode_flag(0);//save current vrr mode
	}

#ifndef BUILD_QUICK_SHOW
    if(get_vo_vflip_flag(SLR_MAIN_DISPLAY)) {
        rtd_pr_vsc_info("send vo vflip=true to vcpu\n");
        scaler_send_rotate_vflip_rpc(TRUE);
    } else {
        rtd_pr_vsc_info("send vo vflip=false to vcpu\n");
        scaler_send_rotate_vflip_rpc(FALSE);
    }
#endif

#ifndef CONFIG_RTK_KDRV_DV_IDK_DUMP
	if((get_i3ddma_4k120_flag() == TRUE)
		|| get_nn_force_i3ddma_enable()
#ifdef CONFIG_RTK_KDRV_DV
		|| check_hdmi_dolby_vision_rgb_and_full_hd()
#endif
		)

#else
	if (1)
#endif
	{
		rtd_pr_vbe_emerg("enable 4k120 2.0 or force i3ddma nn flag\n");

		dvrif_i3ddma_compression_set_enable(false);
		//Re-order control for HDMI Dolby @Crixus 20180322
		set_i3ddma_dolby_reorder_mode(_DISABLE);
	}
	else
	{
		dvrif_i3ddma_compression_set_enable(true);
		if(get_vsc_run_pc_mode() && ((src == VGIP_SRC_TMDS) || (src == VGIP_SRC_ADC))){
			//K5LG-1492:modify for i3ddma compression to 20bit
			if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) &&
				(drvif_i3ddma_triplebuf_flag() || drvif_i3ddma_triplebuf_by_timing_protect_panel()) &&
			(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) >=3840) &&
			(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) >= 2160))
			{
				if(drvif_i3ddma_triplebuf_by_timing_protect_panel() && (I3DDMA_Get_UHD_PCmode_Mem_Alloc_Fail_flag() != TRUE)){
					dvrif_i3ddma_set_compression_bits(COMPRESSION_20_BITS);//pc mode go 20bit only YPP and hdmi
					rtd_pr_vbe_notice("[%s][%d] i3ddma pc mode COMPRESSION_20_BITS. \n",__FUNCTION__,__LINE__);
				}else{
					dvrif_i3ddma_set_compression_bits(COMPRESSION_12_BITS);//default 12 bit
					rtd_pr_vbe_notice("[%s][%d] i3ddma pc mode 3 buffer 4k COMPRESSION_12_BITS. \n",__FUNCTION__,__LINE__);
				}
			}
			else if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) <= 1920) &&
				(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) <= 1080))
			{
				dvrif_i3ddma_set_compression_bits(COMPRESSION_24_BITS);//pc mode go 20bit only YPP and hdmi
				rtd_pr_vbe_notice("[%s][%d] i3ddma pc mode under 2K COMPRESSION_24_BITS. \n",__FUNCTION__,__LINE__);
			}
			else
			{
				dvrif_i3ddma_set_compression_bits(COMPRESSION_20_BITS);//pc mode go 20bit only YPP and hdmi
				rtd_pr_vbe_notice("[%s][%d] i3ddma pc mode COMPRESSION_20_BITS. \n",__FUNCTION__,__LINE__);
			}
		}
		else if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) <= 1920) &&
				(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) <= 1080))
		{
			dvrif_i3ddma_set_compression_bits(COMPRESSION_18_BITS);//pc mode go 20bit only YPP and hdmi
			rtd_pr_vbe_notice("[%s][%d] i3ddma under 2K COMPRESSION_18_BITS. \n",__FUNCTION__,__LINE__);
		}
		else
		{
		    rtd_pr_vbe_notice("[%s][%d] i3ddma COMPRESSION_12_BITS. \n",__FUNCTION__,__LINE__);
			dvrif_i3ddma_set_compression_bits(COMPRESSION_12_BITS);//default 12 bit
		}
		set_i3ddma_compression_mode();

		//Re-order control for HDMI Dolby @Crixus 20180322
		if((get_HDMI_HDR_mode() == HDR_DOLBY_HDMI)
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
			&& ((get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_h14B_VSIF) || (get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_VSIF_STD))
#endif
		)
		{//this is standard mode
			set_i3ddma_dolby_reorder_mode(_ENABLE);
			i3ddma_dolby_reorder_update_color_format();//update color format to 422 12bits
		}
		else{
			set_i3ddma_dolby_reorder_mode(_DISABLE);
		}
	}
	if(!get_nn_force_i3ddma_enable())
	{
	    if(I3DDMA_DolbyVision_HDMI_Init() == 1)
		{
			rtd_pr_vbe_emerg("drivf_i3ddma_config alloc buf error\n");
			return FALSE;
		}
	}

#ifdef CONFIG_I2RND_ENABLE
	if(Scaler_I2rnd_get_timing_enable()){
		//Scaler_I2rnd_set_display(I2RND_MAIN_S0_TABLE);
		//drvif_scaler_i2rnd_vodma_setting(SLR_MAIN_DISPLAY, Get_DisplayMode_Src(SLR_MAIN_DISPLAY));
		//drvif_scaler_i3ddma_vodma_fifo_input_fast(SLR_MAIN_DISPLAY, Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ));
		rtd_pr_vbe_emerg("[crixus]I2rnd path@@@\n");
	}
	else{
#endif
		//drvif_scaler_i3ddma_vodma_fifo_setting(_ENABLE);         // move to video fw
		//drvif_scaler_i3ddma_vodma_fifo_input_fast(SLR_MAIN_DISPLAY, Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ), Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE));    // move to video fw
		rtd_pr_vbe_emerg("[crixus]Non-I2rnd path@@@\n");
#ifdef CONFIG_I2RND_ENABLE
	}
#endif

	memset(&i3ddma_timing, 0 , sizeof(I3DDMA_TIMING_T));
	//rtd_pr_vbe_emerg("%d===%d\n",Scaler_DispGetInputInfoByDisp(display,SLR_DISP_INTERLACE), Scaler_DispGetStatus(display, SLR_DISP_INTERLACE));
	if((Get_DisplayMode_Src(display) == VSC_INPUTSRC_VDEC) || (Get_DisplayMode_Src(display) == VSC_INPUTSRC_JPEG) || (Get_DisplayMode_Src(display) == VSC_INPUTSRC_CAMERA))
		i3ddma_timing.color = (I3DDMA_COLOR_SPACE_T) I3DDMA_COLOR_RGB;//hdr output is rgb
	else
		i3ddma_timing.color = (I3DDMA_COLOR_SPACE_T) Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_COLOR_SPACE);
#ifdef ENABLE_DE_CONTOUR_I2R
	// force capture to 10bit
	if((rtd_inl(VGIP_VGIP_VBISLI_reg) & _BIT20)
	    && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPH_ACT_WID_PRE) <= 1920)
	    && ((I3DDMA_COLOR_DEPTH_T) Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_COLOR_DEPTH) == I3DDMA_COLOR_DEPTH_8B))
	{
	    i3ddma_timing.depth = I3DDMA_COLOR_DEPTH_10B;
	}
	else
	i3ddma_timing.depth = (I3DDMA_COLOR_DEPTH_T) Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_COLOR_DEPTH);
#else
	i3ddma_timing.depth = (I3DDMA_COLOR_DEPTH_T) Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_COLOR_DEPTH);
#endif

	i3ddma_timing.colorimetry = (I3DDMA_COLORIMETRY_T) Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_COLOR_IMETRY);

	if(get_nn_force_i3ddma_enable())
	{
		if(get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1)
			i3ddma_timing.h_act_len = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPH_ACT_WID_PRE)/2;
		else
			i3ddma_timing.h_act_len = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPH_ACT_WID_PRE);
	}
	else
		i3ddma_timing.h_act_len = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPH_ACT_WID_PRE);
	i3ddma_timing.h_act_sta = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPH_ACT_STA_PRE);
	i3ddma_timing.v_act_len = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_LEN_PRE);
	i3ddma_timing.v_act_sta =  Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_STA_PRE);

    if( Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI )
      i3ddma_timing.h_freq = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHFREQ_NEW);  // hdmi.tx_timing.h_freq ( 0.001 Hz )
    else
      i3ddma_timing.h_freq = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_H_FREQ) * 100;

	i3ddma_timing.h_total = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_H_LEN);
	i3ddma_timing.i3ddma_3dformat = I3DDMA_2D_ONLY;
	i3ddma_timing.i3ddma_force2d = VO_FORCE2D_DISABLE;
	i3ddma_timing.polarity = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_POLARITY);
	i3ddma_timing.progressive = Scaler_DispGetStatus(display,SLR_DISP_INTERLACE)?0:1;
	i3ddma_timing.v_active_space1 = 0;
	i3ddma_timing.v_active_space2 = 0;

    if( Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI )
      i3ddma_timing.v_freq = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW);  // hdmi.tx_timing.v_freq ( 0.001 Hz )
    else
      i3ddma_timing.v_freq = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) * 100;

	i3ddma_timing.v_total = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_LEN);


#if 1 // dump source info
	rtd_pr_vbe_notice("[IDMA] C/D/CM/= %d/%d/%d\n", i3ddma_timing.color, i3ddma_timing.depth, i3ddma_timing.colorimetry);
	rtd_pr_vbe_notice("[IDMA] H/HST/HFre/HT=%d/%d/%d/%d\n", 	i3ddma_timing.h_act_len, i3ddma_timing.h_act_sta, i3ddma_timing.h_freq, i3ddma_timing.h_total);
	rtd_pr_vbe_notice("[IDMA] 3D/PO/Prog=%d/%d/%d\n", i3ddma_timing.i3ddma_3dformat , i3ddma_timing.polarity, i3ddma_timing.progressive);
	rtd_pr_vbe_notice("[IDMA] V/VST/VFRE/VT=%d/%d/%d/%d\n", i3ddma_timing.v_act_len, i3ddma_timing.v_act_sta , i3ddma_timing.v_freq, i3ddma_timing.v_total);
#endif

	drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_I3DDMA_DMA, I3DDMA_3D_OPMODE_HW);
	rtd_pr_vbe_notice("################# i3ddma_timing.v_freq : %d\n", i3ddma_timing.v_freq);

	drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_TIMING, (unsigned long) &i3ddma_timing);
	drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_IDX, Scaler_DispGetInputInfo(SLR_INPUT_INPUT_SRC));
	drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE, Scaler_InputSrcGetMainChType());
	drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_FROM, Scaler_InputSrcGetMainChFrom());
	drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR, Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR));
	drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_H_COUNT, Scaler_DispGetInputInfo(SLR_INPUT_H_COUNT));

	drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_HSD_ENABLE, _DISABLE);
	drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_H_SIZE, i3ddma_timing.h_act_len);
	drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_OUTPUT_H_SIZE, i3ddma_timing.h_act_len);
	drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_DITHER12X10_ENABLE, _DISABLE);

    // [WOSQRTK-10007][BW] Force enable HDMI 444(420) to 422!!
    if( ( i3ddma_timing.h_act_len >= 3800 ) && ( i3ddma_timing.v_freq > 48000 )
     && ( i3ddma_timing.color == I3DDMA_COLOR_YUV444 ) && ( i3ddma_timing.depth > 0 ) )
    {
        drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_4XXTO4XX_ENABLE, _ENABLE);
        rtd_pr_vbe_info("[IDMA] Force enable HDMI 444(420) to 422!!\n");
    }else{
	    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_4XXTO4XX_ENABLE, _DISABLE);
    }

	drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_DEPTH, I3DDMA_COLOR_DEPTH_10B);

	if(dvrif_i3ddma_compression_get_enable()){
		drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV444);
		rtd_pr_vbe_notice("\r\n#### func:%s compress(yuv444) comp_bit(%d)#######\r\n", __FUNCTION__, dvrif_i3ddma_get_compression_bits());
	}
	else if((get_i3ddma_4k120_flag() == TRUE) || (get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1))
	{
		drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_DEPTH, I3DDMA_COLOR_DEPTH_8B);
		drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
		rtd_pr_vbe_notice("\r\n#### func:%s 4k120 yuv420(yuv422)#######\r\n", __FUNCTION__);
	}
	else if(get_hdmi_4k_hfr_mode() == HDMI_3K120)
	{//need check
		drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_DEPTH, I3DDMA_COLOR_DEPTH_8B);
		drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
		rtd_pr_vbe_notice("\r\n#### func:%s 3k120 yuv420(yuv422)#######\r\n", __FUNCTION__);
	}

	else if(src == VGIP_SRC_HDR1)
	{
		drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_DEPTH, I3DDMA_COLOR_DEPTH_8B);
		drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
		rtd_pr_vbe_notice("\r\n#### func:%s i3ddma nn force (yuv422)#######\r\n", __FUNCTION__);
	}

	else if((i3ddma_timing.i3ddma_3dformat == I3DDMA_VERTICAL_STRIPE)|| (i3ddma_timing.i3ddma_3dformat == I3DDMA_CHECKER_BOARD))
	{
		// In Vertical strip & Checker board 3D, I3DDMA only support YUV444 capture
		drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV444);
	} // [Dolby Vision] IDMA capture in RGB format
	else if((get_HDMI_HDR_mode() == HDR_DOLBY_HDMI) && (i3ddma_timing.color == I3DDMA_COLOR_RGB)){
		drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_RGB);
	}
	else if((get_HDMI_HDR_mode() == HDR_HDR10_HDMI) || (i3ddma_timing.h_act_len > 3840)/*&& (i3ddma_timing.color != I3DDMA_COLOR_RGB)*/){
		//drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV444);
		rtd_pr_vbe_notice("\r\n####func:%s hdmi10 or width=4096 go YUV422#######\r\n",__FUNCTION__);
		drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
	}
    #ifndef BUILD_QUICK_SHOW
    else if((src == VGIP_SRC_ATV) && (get_AVD_Input_Source() == _SRC_TV)&&(dvrif_i3ddma_compression_get_enable()==FALSE)){
		rtd_pr_vbe_notice("\r\n####func:%s ATV go YUV422#######\r\n",__FUNCTION__);
		drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
	}
    #endif
    else if(i3ddma_timing.color == I3DDMA_COLOR_RGB) {//sync i3ddma uzd color issue from S4AP @Crixus 20170620
		rtd_pr_vbe_notice("\r\n####func:%s RGB go YUV444#######\r\n",__FUNCTION__);
		drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV444);
	}else {
        // [WOSQRTK-10007][BW] Force enable HDMI 444(420) to 422!!
        if( ( i3ddma_timing.h_act_len >= 3800 ) && ( i3ddma_timing.v_freq > 48000 )
            && ( i3ddma_timing.color == I3DDMA_COLOR_YUV444 ) && ( i3ddma_timing.depth > 0 ) )
        {
            drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
            rtd_pr_vbe_info("[IDMA] Force color downsample to 422, wid/freq=%d/%d\n", i3ddma_timing.h_act_len, i3ddma_timing.v_freq);
        }else {
			rtd_pr_vbe_notice("\r\n####func:%s go original format(%d)#######\r\n",__FUNCTION__, i3ddma_timing.color);
			drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, i3ddma_timing.color);
    	}
	}

#ifdef CONFIG_HDR_SDR_SEAMLESS
	if(Get_DisplayMode_Src(display) == VSC_INPUTSRC_HDMI)
	{
		backup_hdmi_src_info(&i3ddma_timing);
		set_vtop_input_color_format(drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR));
	}
#endif

	//drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_FRAMERATE_X2, true);
	if((src == VGIP_SRC_TMDS) || (src == VGIP_SRC_HDR1))
		mode = VGIP_MODE_DIGITAL;
	else
		mode = VGIP_MODE_ANALOG;
	fw_i3ddma_set_vgip(src, mode);
	drv_I3DDMA_ApplySetting(display);

        if(((Get_TIMING_PROTECT_PANEL_TYPE()==TRUE)||(Get_VO_Tracking_D_PANEL_TYPE()==TRUE)) &&  (src == VGIP_SRC_TMDS)){
		drvif_i3ddma_set_triplebuf_flag(drvif_i3ddma_triplebuf_by_timing_protect_panel());
	}
	return TRUE;
}

//New adding for I3DDMA and VO fifo_mode
void drvif_scaler_i3ddma_vodma_fifo_setting(unsigned char enable)
{
	vodma_vodma_i2rnd_RBUS    vo_i2rnd;
    	vodma_vodma_i2rnd_m_flag_RBUS vo_i2rnd_m_flag;
	vodma_vodma_i2rnd_dma_info_RBUS vodma_vodma_i2rnd_dma_info;

	/*
	OK Condition:
		dma_info_wptr_i3ddma - dma_info_rptr_i3ddma >= 2
	*/
	if(enable == _ENABLE){
		//I2rnd fifo mode & timing mode
	    	vo_i2rnd.regValue = rtd_inl(VODMA_vodma_i2rnd_reg);
	    	vo_i2rnd.i2rnd_flag_fifo_en = 1;
	    	vo_i2rnd.i2rnd_src_flag_inv = 0;
	    	vo_i2rnd.i2rnd_timing_mode = 0;
	    	vo_i2rnd.i2rnd_timing_en = 0;
	    	IoReg_Write32(VODMA_vodma_i2rnd_reg, vo_i2rnd.regValue);

		//I2rnd M_flag selection
	    	vo_i2rnd_m_flag.regValue = rtd_inl(VODMA_vodma_i2rnd_m_flag_reg);
	    	vo_i2rnd_m_flag.i2rnd_m_field_fw = 0;
	    	vo_i2rnd_m_flag.i2rnd_m_block_free_run = 0;
	    	vo_i2rnd_m_flag.i2rnd_m_block_sel = 1;
	    	vo_i2rnd_m_flag.i2rnd_m_field_fw = 0;
	    	vo_i2rnd_m_flag.i2rnd_m_field_free_run = 0;
	    	vo_i2rnd_m_flag.i2rnd_m_field_sel = (Get_PANEL_VFLIP_ENABLE()? 2: 1);
	    	vo_i2rnd_m_flag.i2rnd_m_lflag_fw = 0;
	    	vo_i2rnd_m_flag.i2rnd_m_lflag_free_run = 0;
	    	vo_i2rnd_m_flag.i2rnd_m_lflag_sel = 1;
	    	IoReg_Write32(VODMA_vodma_i2rnd_m_flag_reg, vo_i2rnd_m_flag.regValue);

		//reset read/write point
		vodma_vodma_i2rnd_dma_info.regValue = IoReg_Read32(VODMA_vodma_i2rnd_dma_info_reg);
		vodma_vodma_i2rnd_dma_info.dma_info_rptr_fw_set_i3ddma = 1;
		vodma_vodma_i2rnd_dma_info.dma_info_wptr_fw_set_i3ddma = 1;
		IoReg_Write32(VODMA_vodma_i2rnd_dma_info_reg, vodma_vodma_i2rnd_dma_info.regValue);
	}
	else{
		//disable I2rnd fifo mode & timing mode
	    	vo_i2rnd.regValue = rtd_inl(VODMA_vodma_i2rnd_reg);
	    	//vo_i2rnd.i2rnd_flag_fifo_en = 0; //do not disable flag_fifo mode, sync from merlin2 and DIC suggestion. @Crixus 20180226
	    	vo_i2rnd.i2rnd_timing_en = 0;
	    	IoReg_Write32(VODMA_vodma_i2rnd_reg, vo_i2rnd.regValue);
	}
}


unsigned char scaler_i3ddma_save_triplebuf_flag = 0;

unsigned int drvif_i3ddma_triplebuf_by_timing_protect_panel(void)
{
	if((Get_TIMING_PROTECT_PANEL_TYPE()==FALSE)&& (Get_VO_Tracking_D_PANEL_TYPE()==FALSE))
		return FALSE;

	//rtd_pr_vbe_notice("[%s] %d.%d\n",__FUNCTION__,(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI), get_current_driver_pattern(DRIVER_FREERUN_PATTERN));
#if 0
	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) && (get_current_driver_pattern(DRIVER_FREERUN_PATTERN))){
		if (scaler_vsc_get_gamemode_go_datafrc_mode() || (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag()))//vrr always go datafrc
			return FALSE;
		else if (Scaler_DispWindowGet().src_height <  _DISP_LEN)
			return FALSE;
		else
			return TRUE;
	}
#else
	//small window will go hw_ctrl4, no use triple buffer.
	if(Scaler_Get_FreeRun_To_FrameSync_By_Reset_VODMA() 
		&& vbe_get_HDMI_run_timing_framesync_condition()
#ifndef BUILD_QUICK_SHOW		
		&& !get_new_game_mode_small_window_condition() 
#endif		
		&& !(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())) { // !VRR
		return TRUE;
	}
#endif
	return FALSE;
}

void drvif_i3ddma_set_triplebuf_flag(unsigned char bEnable)
{
	rtd_pr_vbe_notice("[%s] enable = %d\n", __FUNCTION__, bEnable);
	scaler_i3ddma_save_triplebuf_flag = bEnable;
}

unsigned char drvif_i3ddma_get_triplebuf_flag(void)
{
	return scaler_i3ddma_save_triplebuf_flag;
}

int drvif_i3ddma_triplebuf_flag(void)
{
#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP_RGB
	return TRUE;
#endif
	if(Get_PANEL_VFLIP_ENABLE())
		return TRUE;

        if( ( Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI ) && ( Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_COMMERCIAL_4K2K_600M_1S_8L_DUPLICATE ) )
          return TRUE;

        if( Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI )
        {
                if(get_scaler_qms_mode_flag())
		{//qms use 3 buffer
			rtd_pr_vsc_info("QMS force i3ddma 3 buffer \n");
			return TRUE;
		}
                if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_INTERLACE))
                        return FALSE;// interlace i domain is real frame rate

                if( Get_DISPLAY_REFRESH_RATE() >= 120 )
                {//120 panel
                        if( ( Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) <= HDMI_TO_VO_FRC_60_MAX_WIDTH_120_PANEL )
                         && ( Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) <= HDMI_TO_VO_FRC_60_MAX_LENTH_120_PANEL ) )
                        {
                          // HDMI CTS framerate range : -0.6 ~ +0.6 %
                          if( ( ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) >= 99400 ) && ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) <= 100600 ) )
                           || ( ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) >= 119100 ) && ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) <= 120720 ) )
                           || ( ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) >= 143500 ) && ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) <= 144864 ) ) 
                           || ( ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) >= 238560 ) && ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) <= 241440 ) ) )
                          {
                            return FALSE;  // go 100 or 120 Hz
                          }
                          else if( ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) > 60100 ) && ( !( ( Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI ) && ( vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag() ) ) ) )  // 60.101 ~ 99.399 Hz  // to be compatible with OLED panel
                          {
                            if( vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag() )
                            {
                              rtd_pr_vsc_info("vrr input frame big than 60, dont use triple buffer.\n");
                              return FALSE;  // i domain go 60
                            }
                            else
                            {
                              rtd_pr_vsc_info("input frame big than 60 and VO force to 60hz\n");
                              return TRUE;  // i domain go 60
                            }
                          }
                          else if( ( Get_DISPLAY_PANEL_OLED_TYPE() == TRUE ) && ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) > 24005 ) && ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) <= 24500 ) )  // to be compatible with OLED panel, 24.016 case
                          {
                            rtd_pr_vsc_info("[OLED] input frame bigger than 24, VO force to 24hz\n");
                            return TRUE;  // i domain go 24
                          }
                          else if( ( Get_DISPLAY_PANEL_OLED_TYPE() == TRUE ) && ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) > 30010 ) && ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) <= 30500 ) )  // to be compatible with OLED panel
                          {
                            rtd_pr_vsc_info("[OLED] input frame bigger than 30, VO force to 30hz\n");
                            return TRUE;  // i domain go 30
                          }
                          else
                          {
                            return FALSE;
                          }
                        }
                        else
                        {
                          if( get_MEMC_bypass_status_refer_platform_model() == TRUE )
                            if( ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ) <= 310 ) && get_current_driver_pattern(DRIVER_FREERUN_PATTERN) )
                              return TRUE;

                          return FALSE;
                        }
                }
                else
                {
                        if( ( Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) <= HDMI_TO_VO_FRC_60_MAX_WIDTH_60_PANEL )
                         && ( Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) <= HDMI_TO_VO_FRC_60_MAX_LENTH_60_PANEL ) )
                        {
                          if( ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) > 60100 ) && ( !( ( Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI ) && ( vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag() ) ) ) )  // > 60.101 Hz  // to be compatible with OLED panel
                          {
                            if( vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag() )
                            {
                              rtd_pr_vsc_info("vrr input frame big than 60, dont use triple buffer.\n");
                              return FALSE;
                            }
                            else
                            {
                              rtd_pr_vsc_info("input frame big than 60 and VO force to 60hz\n");
                              return TRUE;  // i domain go 60
                            }
                          }
                          else if( ( Get_DISPLAY_PANEL_OLED_TYPE() == TRUE ) && ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) > 24010 ) && ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) <= 24500 ) )  // to be compatible with OLED panel
                          {
                            rtd_pr_vsc_info("[OLED] input frame bigger than 24, VO force to 24hz\n");
                            return TRUE;  // i domain go 24
                          }
                          else if( ( Get_DISPLAY_PANEL_OLED_TYPE() == TRUE ) && ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) > 30010 ) && ( Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ_NEW) <= 30500 ) )  // to be compatible with OLED panel
                          {
                            rtd_pr_vsc_info("[OLED] input frame bigger than 30, VO force to 30hz\n");
                            return TRUE;  // i domain go 30
                          }
                          else
                          {
                            // 4k 24hz  30 hz memc bypass vo need run 50-60Hz triple buffer
                            if( get_MEMC_bypass_status_refer_platform_model() == TRUE )
                              if( ( Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) <= 310 ) && get_current_driver_pattern(DRIVER_FREERUN_PATTERN) )
                                  return TRUE;

                            return FALSE;
                          }
                        }

//#ifdef CONFIG_MEMC_BYPASS
                        if( get_MEMC_bypass_status_refer_platform_model() == TRUE )
                          if( ( Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) <= 310 ) && get_current_driver_pattern(DRIVER_FREERUN_PATTERN) )
                            return TRUE;
//#endif

                        return FALSE;
                }
        }
        else
        {
          return FALSE;
        }
}
#ifndef BUILD_QUICK_SHOW 
#ifdef CONFIG_I2RND_ENABLE
#define SUB_FMT_422 1
#define SUB_FMT_444 2
char drivf_Sub_I2rnd_SetupVODMA(void) {

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result = 0;
#endif
	VIDEO_RPC_VOUT_CONFIG_HDMI_3D *config_data;

	//struct page *page = 0;
	//page = alloc_page(GFP_KERNEL);
	//config_datatmp = (VIDEO_RPC_VOUT_SET_3D_MODE *)page_address(page);
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_CONFIG_HDMI_3D), GFP_DCU1_LIMIT, (void **)&vir_addr_noncache);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);

	config_data = (VIDEO_RPC_VOUT_CONFIG_HDMI_3D *)vir_addr_noncache;

	//rtd_pr_vbe_emerg("[crixus]DbgSclrFlgTkr.multiview_sub_interlace_flag = %d\n", DbgSclrFlgTkr.multiview_sub_interlace_flag);

	memset(config_data, 0, sizeof(VIDEO_RPC_VOUT_CONFIG_HDMI_3D));
	config_data->width = Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_MEM_ACT_WID);//timing->h_act_len;
	config_data->height = Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_MEM_ACT_LEN);//timing->v_act_len;
	config_data->chroma_fmt = Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_DISP_422CAP) ? SUB_FMT_422 : SUB_FMT_444;
	config_data->data_bit_width = (Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_10BIT) == 0)? 8: 10;//(timing->depth == I3DDMA_COLOR_DEPTH_8B)? 8: 10;
	config_data->framerate =  Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_V_FREQ) * 100;//timing->v_freq * 100;
	config_data->progressive = (DbgSclrFlgTkr.multiview_sub_interlace_flag == 0)? 1: 0;//timing->progressive;
	config_data->l1_st_adr = rtd_inl(MDOMAIN_CAP_DDR_In2Addr_reg);//rtd_inl(H3DDMA_Cap_L1_Start_reg);
	config_data->r1_st_adr = 0;//rtd_inl(H3DDMA_Cap_R1_Start_reg);
	config_data->l2_st_adr = rtd_inl(MDOMAIN_CAP_DDR_In2_2ndAddr_reg);;//rtd_inl(H3DDMA_Cap_L2_Start_reg);
	config_data->r2_st_adr = 0;//rtd_inl(H3DDMA_Cap_R2_Start_reg);
	config_data->l3_st_adr = rtd_inl(MDOMAIN_CAP_DDR_In2_3rdAddr_reg);//rtd_inl(H3DDMA_Cap_L1_Start_reg);
	config_data->r3_st_adr = 0;//rtd_inl(H3DDMA_Cap_R1_Start_reg);
	//config_data->hw_auto_mode = 1;
	// Dolby Vision force in FW mode
	config_data->hw_auto_mode = 1;
	config_data->quincunx_en = 0;
	config_data->videoPlane = VO_VIDEO_PLANE_V1;
	config_data->i2rnd_sub_src = _ENABLE;
	config_data->i2rnd_display_table = I2RND_SUB_S1_TABLE;
#if 0
	rtd_pr_vbe_emerg("[crixus]config_data->width = %d\n", config_data->width);
	rtd_pr_vbe_emerg("[crixus]config_data->height = %d\n", config_data->height);
	rtd_pr_vbe_emerg("[crixus]config_data->chroma_fmt = %d\n", config_data->chroma_fmt);
	rtd_pr_vbe_emerg("[crixus]config_data->data_bit_width = %d\n", config_data->data_bit_width);
	rtd_pr_vbe_emerg("[crixus]config_data->framerate = %d\n", config_data->framerate);
	rtd_pr_vbe_emerg("[crixus]config_data->progressive = %d\n", config_data->progressive);
	rtd_pr_vbe_emerg("[crixus]config_data->l1_st_adr = %x\n", config_data->l1_st_adr);
	rtd_pr_vbe_emerg("[crixus]config_data->r1_st_adr = %x\n", config_data->r1_st_adr);
	rtd_pr_vbe_emerg("[crixus]config_data->l2_st_adr = %x\n", config_data->l2_st_adr);
	rtd_pr_vbe_emerg("[crixus]config_data->r2_st_adr = %x\n", config_data->r2_st_adr);
	rtd_pr_vbe_emerg("[crixus]config_data->hw_auto_mode = %d\n", config_data->hw_auto_mode);
	rtd_pr_vbe_emerg("[crixus]config_data->i2rnd_sub_src = %d\n", config_data->i2rnd_sub_src);
	rtd_pr_vbe_emerg("[crixus]config_data->i2rnd_display_table = %d\n", config_data->i2rnd_display_table);
#endif
	config_data->width = htonl(config_data->width);
	config_data->height = htonl(config_data->height);
	config_data->chroma_fmt = htonl(config_data->chroma_fmt);
	config_data->data_bit_width = htonl(config_data->data_bit_width);
	config_data->framerate =  htonl(config_data->framerate);
	config_data->progressive = htonl(config_data->progressive);
	config_data->l1_st_adr = htonl(config_data->l1_st_adr);
	config_data->r1_st_adr = htonl(config_data->r1_st_adr);
	config_data->l2_st_adr = htonl(config_data->l2_st_adr);
	config_data->r2_st_adr = htonl(config_data->r2_st_adr);
	config_data->l3_st_adr = htonl(config_data->l3_st_adr);
	config_data->r3_st_adr = htonl(config_data->r3_st_adr);
	config_data->hw_auto_mode = htonl(config_data->hw_auto_mode);
	config_data->quincunx_en = htonl(config_data->quincunx_en);
	config_data->videoPlane = htonl(config_data->videoPlane);
	config_data->i2rnd_sub_src = htonl(config_data->i2rnd_sub_src);
	config_data->i2rnd_display_table = htonl(config_data->i2rnd_display_table);

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_ConfigHDMI_3D,phy_addr, 0, &result))
	{
		rtd_pr_vbe_info("RPC fail!!\n");
		dvr_free((void *)vir_addr);
		return FALSE;
	}
#endif
	dvr_free((void *)vir_addr);

	return TRUE;

}
#endif

//VGIP CTS power saving
//vgip_source: main:0, sub:1, dma vgip:2
void drvif_vgip_CTS_power_saving(unsigned char vgip_source, unsigned char enable){
	vgip_cts_fifo_ctl_RBUS vgip_cts_fifo_ctl_reg;
	sub_vgip_sub_cts_fifo_ctl_RBUS sub_vgip_sub_cts_fifo_ctl_reg;
	dma_vgip_dma_vgip_cts_fifo_ctl_RBUS dma_vgip_dma_vgip_cts_fifo_ctl_reg;

	switch(vgip_source){
		//main vgip
		case 0:
			vgip_cts_fifo_ctl_reg.regValue = IoReg_Read32(VGIP_CTS_FIFO_CTL_reg);
			vgip_cts_fifo_ctl_reg.en_ctsfifo_gate_vgip = enable;
			IoReg_Write32(VGIP_CTS_FIFO_CTL_reg, vgip_cts_fifo_ctl_reg.regValue);
			break;

		//sub vgip
		case 1:
			sub_vgip_sub_cts_fifo_ctl_reg.regValue = IoReg_Read32(SUB_VGIP_SUB_CTS_FIFO_CTL_reg);
			sub_vgip_sub_cts_fifo_ctl_reg.ch2_ctsfifo_gate_vgip = enable;
			IoReg_Write32(SUB_VGIP_SUB_CTS_FIFO_CTL_reg, sub_vgip_sub_cts_fifo_ctl_reg.regValue);
			break;

		//dma vgip
		case 2:
			dma_vgip_dma_vgip_cts_fifo_ctl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg);
			dma_vgip_dma_vgip_cts_fifo_ctl_reg.dma_ctsfifo_gate_vgip = enable;
			IoReg_Write32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg, dma_vgip_dma_vgip_cts_fifo_ctl_reg.regValue);
			break;

		default:
			break;
	}
}


unsigned char main_vsc_connect_ready = FALSE;
#ifdef CONFIG_DUAL_CHANNEL
unsigned char sub_vsc_connect_ready = FALSE;
#endif
void set_vsc_connect_ready(unsigned char display, unsigned char ready)
{//when vsc conenct finish. we set main_vsc_connect_ready is true
	if(display == SLR_MAIN_DISPLAY)
	{
		main_vsc_connect_ready = ready;
	}
#ifdef CONFIG_DUAL_CHANNEL
	else
	{
		sub_vsc_connect_ready = ready;
	}
#endif
}

#ifdef SPEEDUP_NEW_SCALER_FLOW_FOR_ATV
unsigned char check_atv_mode_id_change(void)
{//return TRUE: change.  return FALSE: no change
	extern StructDisplayInfo *Get_AVD_ScalerDispinfo(void);
	extern unsigned char avd_drv_timing_ready(void);
	StructDisplayInfo *avd_info = NULL;
	if(!avd_drv_timing_ready())
		return FALSE;
	avd_info = Get_AVD_ScalerDispinfo();
	if(avd_info->ucMode_Curr != Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_MODE_CURR))
	{
		set_display_forcebg_mask(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_SIG_UNSTABLE, TRUE);//Enable SLR_FORCE_BG_TYPE_SIG_UNSTABLE mask
		Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE, _MODE_STATE_SEARCH);
		return TRUE;
	}
	else
		return FALSE;
}
#endif

unsigned char decide_auto_scaler_run(unsigned char display)
{//return true means auto run. return false: do nothing
#ifdef CONFIG_SCALER_BRING_UP
	return false;
#else
#ifdef SPEEDUP_NEW_SCALER_FLOW
	extern unsigned char get_vo_change_flag(unsigned char ch);
	extern unsigned char avd_drv_timing_ready(void);
	unsigned char port;

	if(display == SLR_MAIN_DISPLAY)
	{
		if(main_vsc_connect_ready)
		{
			if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_SEARCH))
			{
				switch(Get_DisplayMode_Src(SLR_MAIN_DISPLAY))
				{
					case VSC_INPUTSRC_ADC:
#ifdef CONFIG_SUPPORT_SRC_ADC
						return adc_drv_timing_ready();
#endif
					break;
					case VSC_INPUTSRC_AVD:
#ifdef SPEEDUP_NEW_SCALER_FLOW_FOR_ATV
						return avd_drv_timing_ready();
#else
						if(get_AVD_Input_Source() != _SRC_TV) //means AV
							return avd_drv_timing_ready();
#endif
					break;

				case VSC_INPUTSRC_HDMI:
				case VSC_INPUTSRC_DP:
					return FALSE;//vfe_hdmi_drv_timing_ready(); //use run scaler flag to speed up
				break;

				case VSC_INPUTSRC_VDEC:
                case VSC_INPUTSRC_CAMERA:
					port = Get_DisplayMode_Port(SLR_MAIN_DISPLAY);
					if(get_vo_change_flag(port))
						return TRUE;

				break;

				case VSC_INPUTSRC_JPEG:
					port = Get_DisplayMode_Port(SLR_MAIN_DISPLAY);
					if(get_vo_change_flag(port))
						return TRUE;
				break;

					default:
					break;
				}
			}
#ifdef SPEEDUP_NEW_SCALER_FLOW_FOR_ATV
			else
			{
				if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_AVD) && (get_AVD_Input_Source() == _SRC_TV))
				{
					return (check_atv_mode_id_change() && avd_drv_timing_ready());
				}
			}
#endif
		}
	}
#endif
	return FALSE;
#endif //#endof #ifdef CONFIG_SCALER_BRING_UP ... #else ....
}

void drvif_scaler_sub_mdomain_switch_irq(unsigned char enable)
{
	mdomain_cap_cap_ie_wd_ctrl_RBUS mdomain_cap_ie_wd_ctrl_reg;
	mdomain_cap_ie_wd_ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_IE_WD_CTRL_reg);
	mdomain_cap_ie_wd_ctrl_reg.in2_cap_last_wr_ie = enable;
	IoReg_Write32(MDOMAIN_CAP_Cap_IE_WD_CTRL_reg, mdomain_cap_ie_wd_ctrl_reg.regValue);
}
#endif

int wait_vtop_ctrl_double_buffer_apply(void)
{
#ifndef BUILD_QUICK_SHOW

	int count = 0x100;
	hdr_all_top_top_d_buf_RBUS top_d_buf_reg;

	top_d_buf_reg.regValue = rtd_inl(HDR_ALL_TOP_TOP_D_BUF_reg);
	while (top_d_buf_reg.dolby_double_apply == 1 && count > 0) {
		count--;
		usleep_range(1000, 1100);
		top_d_buf_reg.regValue = rtd_inl(HDR_ALL_TOP_TOP_D_BUF_reg);
	}

	if (count <= 0) {
		rtd_pr_vbe_info("timeout for waiting dolby double buffer apply\n");
		return -1;
	}
#endif
	return 0;
}

static void clear_dm_aoi(void)
{
    /* clear dm aoi setting
     * it would cause other format show region error
     */
    rtd_outl(DM_AOI_row_reg, 0);
    rtd_outl(DM_AOI_col_reg, 0);
}


long get_hdr1_in_sel(void)
{
	extern bool get_support_vo_force_v_top(unsigned int func_flag);
	long sel = 0;

	if ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
			|| (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP)) {
		if (get_force_i3ddma_enable(SLR_MAIN_DISPLAY))
			sel = 0;
		else
			sel = 1;
	} else {
		sel = 0; //vodma1 input
	}
	return sel;
}

void scaler_vtop_dolby_mode_setting(unsigned char hdrtype)
{//only for vtop mode setting for scaler
	hdr_all_top_top_ctl_RBUS hdr_all_top_top_ctl_reg;
	hdr_all_top_top_d_buf_RBUS top_d_buf_reg;
	dm_dm_b05_lut_ctrl2_RBUS dm_b05_lut_ctrl2_reg;

	dm_b05_lut_ctrl2_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_D_BUF_reg);

    clear_dm_aoi();

	switch(hdrtype)
	{
		case HDR_DM_MODE_NONE:
			return;

		case HDR_DM_HDR10:
		case HDR_DM_SDR_MAX_RGB:
			/*20190213 roger*/
			top_d_buf_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_D_BUF_reg);
			top_d_buf_reg.dolby_double_en = 1;
			rtd_outl(HDR_ALL_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);//enable double buffer

			hdr_all_top_top_ctl_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_CTL_reg);
			hdr_all_top_top_ctl_reg.dolby_mode = 1;
			hdr_all_top_top_ctl_reg.hdr1_2p = is_4k_hfr_mode(SLR_MAIN_DISPLAY) ? 1 : 0;
			hdr_all_top_top_ctl_reg.hdmi_in_mux = 1;
			hdr_all_top_top_ctl_reg.en_422to444_1 = 0;
			hdr_all_top_top_ctl_reg.en_422to444_tc = 0;
			hdr_all_top_top_ctl_reg.hdr1_in_sel = get_hdr1_in_sel();;//hdmi input

			IoReg_Write32(HDR_ALL_TOP_TOP_CTL_reg, hdr_all_top_top_ctl_reg.regValue); //HDMI dolby mode or HDR10 mode

			top_d_buf_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_D_BUF_reg);
			top_d_buf_reg.dolby_double_apply = 1;
			rtd_outl(HDR_ALL_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);

			wait_vtop_ctrl_double_buffer_apply();
			/*20190213 roger*/

			break;

		case HDR_DM_HLG:
			/*20190213 roger*/
			top_d_buf_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_D_BUF_reg);
			top_d_buf_reg.dolby_double_en = 1;
			hdr_all_top_top_ctl_reg.hdr1_in_sel = get_hdr1_in_sel();//hdmi input

			rtd_outl(HDR_ALL_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);//enable double buffer

			hdr_all_top_top_ctl_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_CTL_reg);
			hdr_all_top_top_ctl_reg.dolby_mode = 1;
			hdr_all_top_top_ctl_reg.hdr1_2p = is_4k_hfr_mode(SLR_MAIN_DISPLAY) ? 1 : 0;
			hdr_all_top_top_ctl_reg.hdmi_in_mux = 1;
			hdr_all_top_top_ctl_reg.en_422to444_1 = 0;
			hdr_all_top_top_ctl_reg.en_422to444_tc = 0;
			IoReg_Write32(HDR_ALL_TOP_TOP_CTL_reg, hdr_all_top_top_ctl_reg.regValue); //HDMI dolby mode or HDR10 mode

			top_d_buf_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_D_BUF_reg);
			top_d_buf_reg.dolby_double_apply = 1;
			rtd_outl(HDR_ALL_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);

			wait_vtop_ctrl_double_buffer_apply();
			/*20190213 roger*/

			break;

		case HDR_DM_TECHNICOLOR:
			/*20190213 roger*/
			top_d_buf_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_D_BUF_reg);
			top_d_buf_reg.dolby_double_en = 1;
			rtd_outl(HDR_ALL_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);//enable double buffer

			hdr_all_top_top_ctl_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_CTL_reg);
			hdr_all_top_top_ctl_reg.dolby_mode = 1;
			hdr_all_top_top_ctl_reg.hdr1_2p = is_4k_hfr_mode(SLR_MAIN_DISPLAY) ? 1 : 0;
			hdr_all_top_top_ctl_reg.hdmi_in_mux = 1;
			hdr_all_top_top_ctl_reg.en_422to444_1 = 0;
			hdr_all_top_top_ctl_reg.en_422to444_tc = 1;
			hdr_all_top_top_ctl_reg.hdr1_in_sel =  get_hdr1_in_sel();

			IoReg_Write32(HDR_ALL_TOP_TOP_CTL_reg, hdr_all_top_top_ctl_reg.regValue); //HDMI dolby mode or HDR10 mode

			top_d_buf_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_D_BUF_reg);
			top_d_buf_reg.dolby_double_apply = 1;
			rtd_outl(HDR_ALL_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);

			wait_vtop_ctrl_double_buffer_apply();
			/*20190213 roger*/

			break;

		case HDR_DM_ST2094:
			/*20190213 roger*/
			top_d_buf_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_D_BUF_reg);
			top_d_buf_reg.dolby_double_en = 1;
			hdr_all_top_top_ctl_reg.hdr1_in_sel =  get_hdr1_in_sel();

			rtd_outl(HDR_ALL_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);//enable double buffer

			hdr_all_top_top_ctl_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_CTL_reg);
			hdr_all_top_top_ctl_reg.dolby_mode = 1;
			hdr_all_top_top_ctl_reg.hdr1_2p = is_4k_hfr_mode(SLR_MAIN_DISPLAY) ? 1 : 0;
			hdr_all_top_top_ctl_reg.hdmi_in_mux = 1;
			hdr_all_top_top_ctl_reg.en_422to444_1 = 0;
			hdr_all_top_top_ctl_reg.en_422to444_tc = 0;
			IoReg_Write32(HDR_ALL_TOP_TOP_CTL_reg, hdr_all_top_top_ctl_reg.regValue); //HDMI dolby mode or HDR10 mode

			top_d_buf_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_D_BUF_reg);
			top_d_buf_reg.dolby_double_apply = 1;
			rtd_outl(HDR_ALL_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);

			wait_vtop_ctrl_double_buffer_apply();
			/*20190213 roger*/

			break;
#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
		case HDR_DM_DOLBY:
			top_d_buf_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_D_BUF_reg);
			top_d_buf_reg.dolby_double_en = 1;
			rtd_outl(HDR_ALL_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);//enable double buffer

			hdr_all_top_top_ctl_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_CTL_reg);
			if(get_vsc_src_is_hdmi_or_dp())
			{
				//top_ctl_reg.regValue = IoReg_Read32(DOLBY_V_TOP_TOP_CTL_reg);
				hdr_all_top_top_ctl_reg.hdr1_in_sel =  get_hdr1_in_sel();
				hdr_all_top_top_ctl_reg.dolby_mode = 1;
				hdr_all_top_top_ctl_reg.hdr1_2p = is_4k_hfr_mode(SLR_MAIN_DISPLAY) ? 1 : 0;
				hdr_all_top_top_ctl_reg.hdmi_in_mux = 1;
				//top_ctl_reg.dolby_ratio = 0; //k5l hw remove it
				hdr_all_top_top_ctl_reg.en_422to444_1 = 1;

				//Re-order control for HDMI Dolby @Crixus 20180322
#if 0
				if(get_i3ddma_dolby_reorder_mode()
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
					  || (drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR) == I3DDMA_COLOR_YUV444)
#endif
				  )
					hdr_all_top_top_ctl_reg.hdr_yuv444_en = 1;
				else
					hdr_all_top_top_ctl_reg.hdr_yuv444_en = 0;
#endif


				IoReg_Write32(HDR_ALL_TOP_TOP_CTL_reg, hdr_all_top_top_ctl_reg.regValue); //HDMI dolby mode or HDR10 mode


			}
			else if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA)
			{//dolby OTT mode
				//top_ctl_reg.regValue = IoReg_Read32(DOLBY_V_TOP_TOP_CTL_reg);
				hdr_all_top_top_ctl_reg.dolby_mode = 2;
				hdr_all_top_top_ctl_reg.hdr1_2p = is_4k_hfr_mode(SLR_MAIN_DISPLAY) ? 1 : 0;
				//top_ctl_reg.dolby_ratio = 1;//k5l hw remove it
				hdr_all_top_top_ctl_reg.hdmi_in_mux = 0;
				IoReg_Write32(HDR_ALL_TOP_TOP_CTL_reg, hdr_all_top_top_ctl_reg.regValue); //Playback dolbymode and compress
			}

			top_d_buf_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_D_BUF_reg);
			top_d_buf_reg.dolby_double_apply = 1;
			rtd_outl(HDR_ALL_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);

			wait_vtop_ctrl_double_buffer_apply();
			break;
#endif//#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT

		default:
			rtd_pr_vbe_notice("scaler_vtop_dolby_mode_setting, get unknow type = %d\n", hdrtype);
			return;
	}

}

void scaler_hdr2_src_sel(void)
{
	hdr_sub_hdr2_top_ctl_RBUS hdr_sub_hdr2_top_ctl_reg;
	VSC_INPUT_TYPE_T src_type = Get_DisplayMode_Src(SLR_SUB_DISPLAY);

	hdr_sub_hdr2_top_ctl_reg.regValue = IoReg_Read32(HDR_SUB_HDR2_TOP_CTL_reg);

	if (src_type == VSC_INPUTSRC_HDMI || src_type == VSC_INPUTSRC_DP)
		hdr_sub_hdr2_top_ctl_reg.hdr2_in_sel = 1;
	else
		hdr_sub_hdr2_top_ctl_reg.hdr2_in_sel = 0;

	IoReg_Write32(HDR_SUB_HDR2_TOP_CTL_reg, hdr_sub_hdr2_top_ctl_reg.regValue);

	 /* apply doublue buffer */
	IoReg_SetBits(HDR_SUB_HDR2_TOP_D_BUF_reg, _BIT1);
}

extern struct semaphore VPQ_DM_DMA_TBL_Semaphore;

void set_hdr_fake_proch_en(bool enable, int width, int htotal)
{
	hdr_all_top_fake_porch_en_RBUS hdr_all_top_fake_porch_en_reg;

	hdr_all_top_fake_porch_en_reg.fake_porch_en = enable;
        hdr_all_top_fake_porch_en_reg.fake_vs_width =  width;
        hdr_all_top_fake_porch_en_reg.fake_porch_htotal = htotal;

	IoReg_Write32(HDR_ALL_TOP_fake_porch_en_reg, hdr_all_top_fake_porch_en_reg.regValue);
}

void set_hdr_fake_porch_line(int line, int pixel)
{
	hdr_all_top_fake_porch_line_px_RBUS hdr_all_top_fake_porch_line_px_reg;


        hdr_all_top_fake_porch_line_px_reg.fake_porch_thr_line  = line;
        hdr_all_top_fake_porch_line_px_reg.fake_porch_thr_pixel = pixel;

	IoReg_Write32(HDR_ALL_TOP_fake_porch_line_px_reg, hdr_all_top_fake_porch_line_px_reg.regValue);
}

void set_hdr_fake_porch(int enable)
{
#define FAKE_VS_WIDTH   2
#define FAKE_PORCH_PIXEL 0

	uint32_t dolby_htotal = HDR_ALL_TOP_TOP_PIC_TOTAL_dolby_h_total(IoReg_Read32(HDR_ALL_TOP_TOP_PIC_TOTAL_reg));
	uint32_t input_src_vtotal = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_LEN);
	uint32_t input_src_v_act  = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE);

	if((input_src_vtotal - input_src_v_act) < FAKE_PORCH_MIN_V_BLANK) {
		rtd_printk(KERN_INFO, TAG_NAME_VSC, "#####[%s(%d)]Abnormal timing can not enable fake vsync!\n", __FUNCTION__, __LINE__);
		return;
	}

	set_hdr_fake_proch_en(enable, FAKE_VS_WIDTH, dolby_htotal);
	set_hdr_fake_porch_line(FAKE_PORCH_THR_LINE, FAKE_PORCH_PIXEL);
}

void hdmi_vtop_hfr_control(void)
{
	dm_input_format_RBUS dm_input_format_reg;
	dm_input_format_reg.regValue = IoReg_Read32(DM_Input_Format_reg);
	if(get_vsc_src_is_hdmi_or_dp())
	{
		if(get_hdr1_in_sel() != 0/*(is_scaler_run_i3ddma_vo_path()) && (get_force_hdmi_hdr_flow_enable(SLR_MAIN_DISPLAY) == true)*/)
		{//4k2k input
			hdr_all_top_top_ctl_RBUS hdr_all_top_top_ctl_reg;
			hdr_all_top_top_pic_size_RBUS hdr_all_top_top_pic_size_reg;
			hdr_all_top_top_pic_total_RBUS hdr_all_top_top_pic_total_reg;
			hdr_all_top_top_pic_sta_RBUS hdr_all_top_top_pic_sta_reg;

			hdr_all_top_top_ctl_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_CTL_reg);
			hdr_all_top_top_ctl_reg.hdr1_in_sel = get_hdr1_in_sel();//hdmi input
			hdr_all_top_top_ctl_reg.dolby_mode = 1;//hdmi dolby mode
			IoReg_Write32(HDR_ALL_TOP_TOP_CTL_reg, hdr_all_top_top_ctl_reg.regValue); //HDMI dolby mode or HDR10 mode

			hdr_all_top_top_pic_size_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_PIC_SIZE_reg);
			hdr_all_top_top_pic_size_reg.dolby_hsize = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
			hdr_all_top_top_pic_size_reg.dolby_vsize = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
			IoReg_Write32(HDR_ALL_TOP_TOP_PIC_SIZE_reg, hdr_all_top_top_pic_size_reg.regValue);

			hdr_all_top_top_pic_total_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_PIC_TOTAL_reg);
			hdr_all_top_top_pic_total_reg.dolby_h_total = Scaler_DispGetInputInfo(SLR_INPUT_H_LEN);
			IoReg_Write32(HDR_ALL_TOP_TOP_PIC_TOTAL_reg, hdr_all_top_top_pic_total_reg.regValue);

			hdr_all_top_top_pic_sta_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_PIC_STA_reg);
			hdr_all_top_top_pic_sta_reg.dolby_v_den_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);
			hdr_all_top_top_pic_sta_reg.dolby_h_den_sta = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE);
			IoReg_Write32(HDR_ALL_TOP_TOP_PIC_STA_reg, hdr_all_top_top_pic_sta_reg.regValue);

			IoReg_SetBits(HDR_ALL_TOP_TOP_D_BUF_reg, _BIT1);//Enable doublue buffer

			if((drvif_Hdmi_GetColorSpace() == COLOR_YUV444) || (drvif_Hdmi_GetColorSpace() == COLOR_RGB))
			{
				if(get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1)
					dm_input_format_reg.dm_in_sel = 2;//2pixel 444
				else
					dm_input_format_reg.dm_in_sel = 2;//1pixel 444 3k120
			}
			else
			{
				if(get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1)
					dm_input_format_reg.dm_in_sel = 1;//2pixel 422
				else
					dm_input_format_reg.dm_in_sel = 1;//1pixel 422 3k120
			}
			IoReg_Write32(DM_Input_Format_reg, dm_input_format_reg.regValue); //set dm in select
			IoReg_SetBits(DM_HDR_Double_Buffer_CTRL_reg, _BIT0);//apply doublue buffer
		}
	}
}

void scaler_hdr_mode_setting_IMD(unsigned char display, unsigned char hdrtype)
{
	if(display == SLR_MAIN_DISPLAY) {
		Scaler_color_set_HDR_Type(hdrtype);
		if ((hdrtype == HDR_DM_DOLBY) ||(hdrtype == HDR_DM_MODE_NONE))
			drvif_color_set_HDR_RGB_swap(0);
		else
			drvif_color_set_HDR_RGB_swap(1);
	}
}

void scaler_hdr_mode_setting(unsigned char display, unsigned char hdrtype)
{
	//rtd_pr_vbe_info("scaler_hdr_mode_setting display:%d type = %d\n", display, hdrtype); setup IMD will print
	dm_dm_b05_lut_ctrl2_RBUS dm_dm_b05_lut_ctrl2_Reg;
#ifndef BUILD_QUICK_SHOW     
	down(&VPQ_DM_DMA_TBL_Semaphore);
#endif
	if(display == SLR_MAIN_DISPLAY)
	{
		switch(hdrtype)
		{
			case HDR_DM_MODE_NONE:
				drvif_TV006_SET_HDR_mode_off();
				drvif_color_HDR10_DM_setting();
				fwif_color_set_DM2_Hist_AutoMode_Enable(0);
				if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) || (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA) || (get_hdmi_4k_hfr_mode() == HDMI_4K120_2_0))
				{//k9155 patch. decide 422 to 444 at sdr mode.
					hdr_all_top_top_ctl_RBUS hdr_all_top_top_ctl_reg;
					hdr_all_top_top_ctl_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_CTL_reg);
					hdr_all_top_top_ctl_reg.en_422to444_1 = 1;
					IoReg_Write32(HDR_ALL_TOP_TOP_CTL_reg, hdr_all_top_top_ctl_reg.regValue); //HDMI dolby mode or HDR10 mode
					IoReg_SetBits(HDR_ALL_TOP_TOP_D_BUF_reg, _BIT1);//apply doublue buffer
				}
				fwif_color_TC_Flow(0);
#ifndef BUILD_QUICK_SHOW				
				fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
#endif
				Scaler_color_HFC_set_TC_FrameSync_Enable(0);
				fwif_color_set_ST2094_FrameSync_Enable(0);				
				break;

			case HDR_DM_SDR_MAX_RGB:
				hdmi_vtop_hfr_control();//for two pixel mode setting
				Scaler_LGE_HDR10_Enable_newflow(2/*BT2020*/);
				drvif_DM2_OETF_EOTF_Path_Sel(1);
				drvif_DM2_B0203_noSram_Enable(1, 0);
				fwif_color_set_HDR_Dolby_mode_Enable(hdrtype);
				drvif_color_set_BBC_Enable(0);
				fwif_color_set_HDR_TV006();
				Scaler_Set_HDR_YUV2RGB(HAL_VPQ_HDR_MODE_SDR_MAX_RGB, HAL_VPQ_COLORIMETRY_MAX);
				drvif_color_HDR10_DM_setting();
	            dm_dm_b05_lut_ctrl2_Reg.regValue = IoReg_Read32(DM_DM_B05_LUT_CTRL2_reg);
	            dm_dm_b05_lut_ctrl2_Reg.dm_b05_input_ctrl = 1;
				IoReg_Write32(DM_DM_B05_LUT_CTRL2_reg, dm_dm_b05_lut_ctrl2_Reg.regValue);
				fwif_color_set_HDR10_runmode();
				drvif_color_set_HDR_RGB_swap(1);
				fwif_color_TC_Flow(0);
				fwif_color_set_DM2_Hist_AutoMode_Enable(1);
#ifndef BUILD_QUICK_SHOW
				fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
#endif
				Scaler_color_HFC_set_TC_FrameSync_Enable(0);
				fwif_color_set_ST2094_FrameSync_Enable(0);				
				
				break;

			case HDR_DM_HDR10:
				hdmi_vtop_hfr_control();//for two pixel mode setting
				Scaler_LGE_HDR10_Enable_newflow(2/*BT2020*/);
				drvif_DM2_OETF_EOTF_Path_Sel(1);
				drvif_DM2_B0203_noSram_Enable(1, 0);
				fwif_color_set_HDR_Dolby_mode_Enable(hdrtype);
				fwif_color_set_HDR_TV006();
				Scaler_Set_HDR_YUV2RGB(HAL_VPQ_HDR_MODE_HDR10, HAL_VPQ_COLORIMETRY_BT2020);
				drvif_color_HDR10_DM_setting();
				fwif_color_set_HDR10_runmode();
				drvif_color_set_HDR_RGB_swap(1);
				fwif_color_TC_Flow(0);
				fwif_color_set_DM2_Hist_AutoMode_Enable(1);
#ifndef BUILD_QUICK_SHOW
				fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
#endif
				Scaler_color_HFC_set_TC_FrameSync_Enable(0);
				fwif_color_set_ST2094_FrameSync_Enable(0);				
								
				#ifdef CONFIG_SUPPORT_IPQ
				drvif_color_set_BBC_Enable(1);
				drvif_color_set_BBC_shift_bit(4);
				drvif_color_Set_ST2094_3Dlut_CInv(1024<<(9-4), 1, 10);
			       dm_dm_b05_lut_ctrl2_Reg.regValue = rtd_inl(DM_DM_B05_LUT_CTRL2_reg);
			        dm_dm_b05_lut_ctrl2_Reg.dm_b05_input_ctrl = 1;
				IoReg_Write32(DM_DM_B05_LUT_CTRL2_reg, dm_dm_b05_lut_ctrl2_Reg.regValue);
				#else
				drvif_color_set_BBC_Enable(1);
				#endif
				break;

			case HDR_DM_HLG:
				hdmi_vtop_hfr_control();//for two pixel mode setting
				Scaler_LGE_HDR10_Enable_newflow(2/*BT2020*/);
				drvif_DM2_OETF_EOTF_Path_Sel(1);
				drvif_DM2_B0203_noSram_Enable(1, 0);
				fwif_color_set_HDR_Dolby_mode_Enable(hdrtype);
				drvif_color_set_BBC_Enable(1);
				#ifndef CONFIG_SUPPORT_IPQ
				drvif_color_set_BBC_shift_bit(0);
				#endif
				fwif_color_set_HDR_TV006();
				Scaler_Set_HDR_YUV2RGB(HAL_VPQ_HDR_MODE_HLG, HAL_VPQ_COLORIMETRY_BT2020);
				drvif_color_HDR10_DM_setting();
				fwif_color_set_HDR10_runmode();
				drvif_color_set_HDR_RGB_swap(1);
				fwif_color_TC_Flow(0);
				fwif_color_set_DM2_Hist_AutoMode_Enable(1);
#ifndef BUILD_QUICK_SHOW
				fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
#endif
				Scaler_color_HFC_set_TC_FrameSync_Enable(0);
				fwif_color_set_ST2094_FrameSync_Enable(0);				
				
				break;

			case HDR_DM_TECHNICOLOR:
				/* enable HDR10*/
				hdmi_vtop_hfr_control();//for two pixel mode setting
				Scaler_LGE_HDR10_Enable_newflow(1);
				drvif_DM2_OETF_EOTF_Path_Sel(1);
				drvif_DM2_B0203_noSram_Enable(1, 0);
				fwif_color_set_HDR_Dolby_mode_Enable(hdrtype);
				drvif_color_set_BBC_Enable(0);
				fwif_color_set_HDR_TV006();
				//set_HDR10_DM_setting();
				fwif_color_set_HDR10_runmode();
				drvif_color_set_HDR_RGB_swap(1);
				/* Enable TC*/
				fwif_color_TC_Flow(1);
				drvif_color_HDR10_DM_setting();
				fwif_color_set_DM2_Hist_AutoMode_Enable(0);
#ifndef BUILD_QUICK_SHOW
				fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
#endif
				Scaler_color_HFC_set_TC_FrameSync_Enable(1);
				fwif_color_set_ST2094_FrameSync_Enable(0);				
				
				break;

			case HDR_DM_ST2094:
				hdmi_vtop_hfr_control();//for two pixel mode setting
				Scaler_LGE_HDR10_Enable_newflow(2/*BT2020*/);
				drvif_DM2_OETF_EOTF_Path_Sel(1);
				drvif_DM2_B0203_noSram_Enable(1, 0);
				fwif_color_set_HDR_Dolby_mode_Enable(hdrtype);
				drvif_color_set_BBC_Enable(2);
				fwif_color_set_HDR_TV006();
				Scaler_Set_HDR_YUV2RGB(HAL_VPQ_HDR_MODE_ST2094, HAL_VPQ_COLORIMETRY_BT2020);
				drvif_color_HDR10_DM_setting();
				fwif_color_set_HDR10_runmode();
				drvif_color_set_HDR_RGB_swap(1);
				fwif_color_TC_Flow(0);
				fwif_color_set_DM2_Hist_AutoMode_Enable(0);
#ifndef BUILD_QUICK_SHOW
				fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
#endif
				Scaler_color_HFC_set_TC_FrameSync_Enable(0);
				fwif_color_set_ST2094_FrameSync_Enable(1);				
				
				break;
#ifdef support_hdr10_vivi_mode
			case HDR_DM_HDR10_VIVID:
				hdmi_vtop_hfr_control();//for two pixel mode setting
				Scaler_LGE_HDR10_Enable_newflow(2/*BT2020*/);
				drvif_DM2_OETF_EOTF_Path_Sel(1);
				drvif_DM2_B0203_noSram_Enable(1, 0);
				fwif_color_set_HDR_Dolby_mode_Enable(hdrtype);
				drvif_color_set_BBC_Enable(1);
	#ifndef CONFIG_SUPPORT_IPQ
				drvif_color_set_BBC_shift_bit(0);
	#endif
				fwif_color_set_HDR_TV006();
				Scaler_Set_HDR_YUV2RGB(HAL_VPQ_HDR_MODE_HLG, HAL_VPQ_COLORIMETRY_BT2020);
				drvif_color_HDR10_DM_setting();
				fwif_color_set_HDR10_runmode();
				drvif_color_set_HDR_RGB_swap(1);
				fwif_color_TC_Flow(0);
				fwif_color_set_DM2_Hist_AutoMode_Enable(1);
#ifndef BUILD_QUICK_SHOW
				fwif_color_set_HDR_VIVID_FrameSync_Enable(1);
#endif
				Scaler_color_HFC_set_TC_FrameSync_Enable(0);
				fwif_color_set_ST2094_FrameSync_Enable(0);				
				
				break;
#endif
			case HDR_DM_DOLBY:
				{
#ifdef CONFIG_RTK_KDRV_DV
#ifdef  CONFIG_DOLBY_IDK_1_6_1_1
                  extern void dolby_adapter_dolby_init(void);
#endif
#endif
	                        /*20170908, will add dolby hdr case*/
				#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
				dm_input_format_RBUS dm_input_format_reg;
				//extern void drvif_hdr_idma_color_colorspacergb2yuvtransfer(void);
				hdr_all_top_top_ctl_RBUS hdr_all_top_top_ctl_reg;
				dm_dm_b05_lut_ctrl2_RBUS dm_dm_b05_lut_ctrl2_Reg;
				vgip_vgip_chn1_misc_RBUS vgip_chn1_misc_reg;
				dm_hdr_path_sel_RBUS dm_hdr_path_sel_reg;
				dm_hdr_double_buffer_ctrl_RBUS dm_double_buffer_ctrl_reg;

				/* disable HDR10 Basic setting and clear HDR10 Flag first*/
				drvif_TV006_SET_HDR_mode_off();

				dm_dm_b05_lut_ctrl2_Reg.regValue = rtd_inl(DM_DM_B05_LUT_CTRL2_reg);
				#if 0  //m6 spec removed
				dm_dm_b05_lut_ctrl2_Reg.dm_b05_dimension = 0;//reset to 0(table 17*17*17*3);
				#endif
				dm_dm_b05_lut_ctrl2_Reg.dm_b05_input_ctrl = 0;
				dm_dm_b05_lut_ctrl2_Reg.b05_dolby_mode = 1;
				IoReg_Write32(DM_DM_B05_LUT_CTRL2_reg, dm_dm_b05_lut_ctrl2_Reg.regValue);

				/*to clean EO and OE data(0xb806b404) when dolby case*/
				drvif_DM2_EOTF_Enable(0, 0);
				drvif_DM2_OETF_Enable(0, 0);
				drvif_DM2_OETF_EOTF_Path_Sel(0);
				drvif_DM2_B0203_noSram_Enable(0, 0);
				hdr_all_top_top_ctl_reg.regValue = IoReg_Read32(HDR_ALL_TOP_TOP_CTL_reg);
				rtd_pr_vbe_notice("scaler_hdr_mode_setting for dolby\n");
				//K3LG-1046 noise screen
	//dolby_v_top_top_d_buf_RBUS top_d_buf_reg;


				//reset color space for dolby OTT in case webos not set //WOSQRTK-7184
				vgip_chn1_misc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_MISC_reg);
				vgip_chn1_misc_reg.ch1_v1_v3_swap = 0;
				vgip_chn1_misc_reg.ch1_v2_v3_swap = 0;
				vgip_chn1_misc_reg.ch1_v1_v2_swap = 0;
				IoReg_Write32(VGIP_VGIP_CHN1_MISC_reg, vgip_chn1_misc_reg.regValue);

				dm_hdr_path_sel_reg.regValue = IoReg_Read32(DM_HDR_PATH_SEL_reg);
				dm_hdr_path_sel_reg.b05_position = 0; //00:B04->B05->dither, 01:Tone mapping->B05->gamma,
				dm_hdr_path_sel_reg.dm_b05_22bit_path = 0;
				IoReg_Write32(DM_HDR_PATH_SEL_reg, dm_hdr_path_sel_reg.regValue);
				dm_double_buffer_ctrl_reg.regValue = IoReg_Read32(DM_HDR_Double_Buffer_CTRL_reg);
				dm_double_buffer_ctrl_reg.dm_db_read_sel=1;
				dm_double_buffer_ctrl_reg.dm_db_apply = 1;
				IoReg_Write32(DM_HDR_Double_Buffer_CTRL_reg, dm_double_buffer_ctrl_reg.regValue);
				dm_input_format_reg.regValue = IoReg_Read32(DM_Input_Format_reg);
				if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
				{
				       //top_ctl_reg.regValue = IoReg_Read32(DOLBY_V_TOP_TOP_CTL_reg);
					hdr_all_top_top_ctl_reg.dolby_mode = 1;
					hdr_all_top_top_ctl_reg.hdmi_in_mux = 1;
					//top_ctl_reg.dolby_ratio = 0; //k5l hw remove it
					hdr_all_top_top_ctl_reg.en_422to444_1 = 1;

					//Re-order control for HDMI Dolby @Crixus 20180322
#if 0//fix later by will
					if(get_i3ddma_dolby_reorder_mode()
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
						|| ((get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_VSIF_LL) && ((drvif_Hdmi_GetColorSpace() == COLOR_YUV444) || (drvif_Hdmi_GetColorSpace() == COLOR_RGB)))
#endif
					)
						hdr_all_top_top_ctl_reg.hdr_yuv444_en = 1;
					else
						hdr_all_top_top_ctl_reg.hdr_yuv444_en = 0;

#endif

					if(drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR) == I3DDMA_COLOR_YUV444)
					{
						dm_input_format_reg.dm_in_sel = 2;//444
					}
					else
					{
						dm_input_format_reg.dm_in_sel = 1;//422
					}
					IoReg_Write32(DM_Input_Format_reg, dm_input_format_reg.regValue); //set dm in select
					IoReg_SetBits(DM_HDR_Double_Buffer_CTRL_reg, _BIT0);//apply doublue buffer

					IoReg_Write32(HDR_ALL_TOP_TOP_CTL_reg, hdr_all_top_top_ctl_reg.regValue); //HDMI dolby mode or HDR10 mode
					IoReg_SetBits(HDR_ALL_TOP_TOP_D_BUF_reg, _BIT1);//Enable doublue buffer
					drvif_hdr_idma_color_colorspacergb2yuvtransfer();


				}
				else if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA)
				{//dolby OTT mode
				       //top_ctl_reg.regValue = IoReg_Read32(DOLBY_V_TOP_TOP_CTL_reg);
					hdr_all_top_top_ctl_reg.dolby_mode = 2;
					//top_ctl_reg.dolby_ratio = 1;//k5l hw remove it
					hdr_all_top_top_ctl_reg.hdmi_in_mux = 0;
					IoReg_Write32(HDR_ALL_TOP_TOP_CTL_reg, hdr_all_top_top_ctl_reg.regValue); //Playback dolbymode and compress
					IoReg_SetBits(HDR_ALL_TOP_TOP_D_BUF_reg, _BIT1);//Enable doublue buffer

					dm_input_format_reg.dm_in_sel = 0;//420
					IoReg_Write32(DM_Input_Format_reg, dm_input_format_reg.regValue); //set dm in select
					IoReg_SetBits(DM_HDR_Double_Buffer_CTRL_reg, _BIT0);//apply doublue buffer
				}
				fwif_color_set_DM2_Hist_AutoMode_Enable(0);
				fwif_color_TC_Flow(0);
#ifndef BUILD_QUICK_SHOW				
				fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
#endif
				Scaler_color_HFC_set_TC_FrameSync_Enable(0);
				fwif_color_set_ST2094_FrameSync_Enable(0);				
				
			#endif
#ifdef CONFIG_RTK_KDRV_DV
#ifdef  CONFIG_DOLBY_IDK_1_6_1_1
                dolby_adapter_dolby_init();
#endif
#endif
				}
				break;

			default:
				rtd_pr_vbe_notice("scaler_hdr_mode_setting, get unknow type = %d\n", hdrtype);
				break;
		}
	}
	else
	{
		if (hdrtype != HDR_DM_MODE_NONE)
			scaler_hdr2_src_sel();

		switch(hdrtype)
		{
			case HDR_DM_MODE_NONE:
			{
				hdr_sub_hdr2_top_ctl_RBUS hdr_sub_hdr2_top_ctl_reg;
				hdr_sub_hdr2_top_ctl_reg.regValue = IoReg_Read32(HDR_SUB_HDR2_TOP_CTL_reg);
				hdr_sub_hdr2_top_ctl_reg.hdr2_en_422to444_1 = 1;
				hdr_sub_hdr2_top_ctl_reg.hdr2_en = 0;
				IoReg_Write32(HDR_SUB_HDR2_TOP_CTL_reg, hdr_sub_hdr2_top_ctl_reg.regValue); //HDMI dolby mode or HDR10 mode
				IoReg_SetBits(HDR_SUB_HDR2_TOP_D_BUF_reg, _BIT1);//apply doublue buffer
			}
				break;

			case HDR_DM_HDR10:

				break;

			case HDR_DM_HLG:

				break;

			case HDR_DM_TECHNICOLOR:

				break;

			case HDR_DM_ST2094:

				break;

			case HDR_DM_DOLBY:
				break;

			default:
				break;
		}
	}
#ifndef BUILD_QUICK_SHOW     
	up(&VPQ_DM_DMA_TBL_Semaphore);
#endif
}

#ifndef BUILD_QUICK_SHOW 

unsigned char hdr_semaless_active = FALSE;

void set_hdr_semaless_active(unsigned char enable)
{
	seamless_change_sync_info *seamless_info = NULL;
	seamless_info = (seamless_change_sync_info *)Scaler_GetShareMemVirAddr(SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_FLAG);
	hdr_semaless_active = enable;
	if(seamless_info)
	{
		if(enable)
			seamless_info->hdr_active_state = 1;
		else
			seamless_info->hdr_active_state = 0;
	}
}


unsigned char get_hdr_semaless_active(void)
{//means that we can run seamless state //decide we can run seamless flow or not
	return hdr_semaless_active;
}
#endif 

/*
Input Source Time			RTNR		    OD	             DexC	data-path	       VE-Comps     DIpqc-bit      MA-snr
				 (OFF/422/420/400)   (OFF/3bit/4bit)	  (OFF/ON)   (FRC/Frame-sync)	   (OFF/ON)	(6bit/10bit)  (OFF/ON)
HDMI / 2K1K 30 fps	 422	 		   OFF			ON		FRC                          OFF               10bit           ON
HDMI / 2K1K 50/60 fps	 422			   OFF		       ON		FRC				    OFF               10bit           ON
HDMI / 4K2K 30 fps	 OFF		          OFF		       OFF		FRC				    OFF               10bit           ON
HDMI / 4K2K 50/60 fps	 OFF	           OFF		       OFF		FRC				    OFF               10bit           ON
VO / 2K1K 30 fps			422		          OFF	   	ON		FRC				    ON                10bit           ON
VO / 2K1K 50/60 fps		420		          OFF		       ON		FRC				    ON                10bit           ON
VO / 4K2K 30 fps			OFF		          OFF		       OFF		FRC				    ON                6bit             ON
VO / 4K2K 50/60 fps		OFF		          OFF		       OFF		FRC	     		    ON                 6bit            OFF
HDMI HDR/4k 50/60            OFF                   OFF                  OFF         FRC              		    OFF                10bit           ON
*/


VSC_DRIVER_PATTERN vsc_driver_pattern_table[] ={
#ifdef CONFIG_RTK_BUSINESS_DISPLAY_MODEL
  	{HDMI_2K1K_30,		0, 2, 1, 0, 0, 1, 1},
	{HDMI_2K1K_60, 		0, 2, 1, 0, 0, 1, 1},
	{HDMI_2K1K_120,	 	0, 2, 1, 0, 0, 1, 1},
	{HDMI_4K2K_30, 		0, 2, 0, 1, 0, 1, 0},
	{HDMI_4K2K_60, 		0, 2, 0, 1, 0, 1, 0},
	{HDMI_4K2K_120, 	0, 2, 0, 1, 0, 1, 0},
	{HDMI_8K4K_30, 		0, 2, 0, 1, 0, 1, 0},
	{HDMI_8K4K_60, 		0, 2, 0, 1, 0, 1, 0},
	{VO_2K1K_30, 		0, 2, 1, 0, 1, 1, 1},
	{VO_2K1K_60, 		0, 2, 1, 0, 1, 1, 1},
	{VO_2K1K_120, 		0, 2, 1, 0, 1, 1, 1},
	{VO_4K1K_120, 		0, 2, 0, 1, 1, 0, 0},
	{VO_4K2K_30, 		0, 2, 0, 1, 1, 1, 0},
	{VO_4K2K_60, 		0, 2, 0, 1, 1, 0, 0},
	{VO_4K2K_120, 		0, 2, 0, 1, 1, 0, 0},
	{VO_8K4K_30, 		0, 2, 0, 1, 1, 0, 0},
	{VO_8K4K_60, 		0, 2, 0, 1, 1, 0, 0},
	{CP_2K1K_30, 		0, 2, 1, 1, 1, 1, 1},
	{CP_2K1K_60, 		0, 2, 1, 1, 1, 1, 1},
	{CP_2K1K_120, 		0, 2, 1, 0, 1, 1, 1},
	{CP_4K1K_120, 		0, 2, 0, 1, 1, 0, 0},
	{CP_4K2K_30, 		0, 2, 0, 1, 1, 1, 0},
	{CP_4K2K_60, 		0, 2, 0, 1, 1, 0, 0},
	{CP_4K2K_120, 		0, 2, 0, 1, 1, 0, 0},
	{CP_8K4K_30, 		0, 2, 0, 1, 1, 0, 0},
	{CP_8K4K_60, 		0, 2, 0, 1, 1, 0, 0},
	{HDMI_HDR_4K2K_60, 	0, 2, 0, 1, 0, 1, 0},
	{HDMI_HDR_8K4K_60, 	0, 2, 0, 1, 0, 1, 0},
	{JPEG_2K1K_30, 		0, 2, 1, 0, 1, 0, 0},
	{JPEG_4K2K_30, 		0, 2, 0, 1, 1, 0, 0},
	{JPEG_8K4K_30, 		0, 2, 0, 1, 1, 0, 0},
	{TIMING_INVAILD,	0, 1, 0, 0, 0, 0, 0},
#else
	{HDMI_2K1K_30,		1, 2, 1, 0, 0, 1, 1},
	{HDMI_2K1K_60, 		1, 2, 1, 0, 0, 1, 1},
	{HDMI_2K1K_120,	 	1, 2, 1, 0, 0, 1, 1},
	{HDMI_4K2K_30, 		1, 2, 0, 1, 0, 1, 0},
	{HDMI_4K2K_60, 		1, 2, 0, 1, 0, 1, 0},
	{HDMI_4K2K_120, 	0, 2, 0, 1, 0, 1, 0},
	{HDMI_8K4K_30, 		0, 2, 0, 1, 0, 1, 0},
	{HDMI_8K4K_60, 		0, 2, 0, 1, 0, 1, 0},
	{VO_2K1K_30, 		1, 2, 1, 0, 1, 1, 1},
	{VO_2K1K_60, 		1, 2, 1, 0, 1, 1, 1},
	{VO_2K1K_120, 		1, 2, 1, 0, 1, 1, 1},
	{VO_4K1K_120, 		0, 2, 0, 1, 1, 0, 0},
	{VO_4K2K_30, 		1, 2, 0, 1, 1, 1, 0},
	{VO_4K2K_60, 		0, 2, 0, 1, 1, 0, 0},
	{VO_4K2K_120, 		0, 2, 0, 1, 1, 0, 0},
	{VO_8K4K_30, 		0, 2, 0, 1, 1, 0, 0},
	{VO_8K4K_60, 		0, 2, 0, 1, 1, 0, 0},
	{CP_2K1K_30, 		1, 2, 1, 1, 1, 1, 1},
	{CP_2K1K_60, 		1, 2, 1, 1, 1, 1, 1},
	{CP_2K1K_120, 		1, 2, 1, 1, 1, 1, 1},
	{CP_4K1K_120, 		0, 2, 0, 1, 1, 0, 0},
	{CP_4K2K_30, 		1, 2, 0, 1, 1, 1, 0},
	{CP_4K2K_60, 		0, 2, 0, 1, 1, 0, 0},
	{CP_4K2K_120, 		0, 2, 0, 1, 1, 0, 0},
	{CP_8K4K_30, 		0, 2, 0, 1, 1, 0, 0},
	{CP_8K4K_60, 		0, 2, 0, 1, 1, 0, 0},
	{HDMI_HDR_4K2K_60, 	1, 2, 0, 1, 0, 1, 0},
	{HDMI_HDR_8K4K_60, 	0, 2, 0, 1, 0, 1, 0},
	{JPEG_2K1K_30, 		0, 2, 1, 1, 1, 0, 0},
	{JPEG_4K2K_30, 		0, 2, 0, 1, 1, 0, 0},
	{JPEG_8K4K_30, 		0, 2, 0, 1, 1, 0, 0},
	{TIMING_INVAILD,	0, 1, 0, 0, 0, 0, 0},
#endif
 };

#ifndef BUILD_QUICK_SHOW

int parse_xml_driver_string(VSC_DRIVER_STRING *driver_str)
{
	char *timing[9] = {"HDMI_2K1K_30", "HDMI_2K1K_60", "HDMI_4K2K_30", "HDMI_4K2K_60", "VO_2K1K_30", "VO_2K1K_60", "VO_4K2K_30", "VO_4K2K_60","HDMI_HDR_4K2K_60"};
	char *rtnr_mode_str[4] = {"off", "422", "420", "400"};
	char *od_str[3] = {"off", "3bit", "4bit"};
	char *dexc_str[2] = {"off", "on"};
	char *free_run_str[2] = {"frc", "frame_sync"};
	char *ve_comp_str[2] = {"off", "on"};
	char *dipqc_bit_str[2] = {"6bit", "10bit"};
	char *ma_snr_str[2] = {"off", "on"};

	unsigned int rtnr_mode[4] = {0,1,2,3};
	unsigned int od[3] = {0,1,2};
	unsigned int dexc[2] = {0,1};
	unsigned int free_run[2] = {0,1};
	unsigned int ve_comp[2] = {0,1};
	unsigned int dipqc_bit[2] = {0,1};
	unsigned int ma_snr[2] = {0,1};

	int i = 0;
	int timing_id = HDMI_2K1K_30;
	VSC_DRIVER_PATTERN driver_config[TIMING_INVAILD];

	while(timing_id != TIMING_INVAILD)
	{
		//parse timing info
		if(strcmp(driver_str[timing_id].timing_index_str, timing[timing_id]) != 0)
		{
			rtd_pr_vbe_emerg("parse xml timing fail,ap:%s,expect:%s\n",driver_str[timing_id].timing_index_str,timing[timing_id]);
			return -1;
		}
		driver_config[timing_id].timing_index = timing_id;

		//parse rtnr mode
		for(i = 0; i < sizeof(rtnr_mode_str)/sizeof(char *); i++)
		{
			if(strcmp(driver_str[timing_id].rtnr_mode_str, rtnr_mode_str[i]) == 0)
			{
				driver_config[timing_id].rtnr_mode = rtnr_mode[i];
				break;
			}

		}
		if(i == sizeof(rtnr_mode_str)/sizeof(char *))
		{
			rtd_pr_vbe_emerg("parse xml rtnr_mode fail,ap:%s,%s\n",driver_str[timing_id].timing_index_str,driver_str[timing_id].rtnr_mode_str);
			return -1;
		}

		//parse od
		for(i = 0; i < sizeof(od_str)/sizeof(char *); i++)
		{
			if(strcmp(driver_str[timing_id].od_en_str, od_str[i]) == 0)
			{
				driver_config[timing_id].od_en= od[i];
				break;
			}
		}
		if(i == sizeof(od_str)/sizeof(char *))
		{
			rtd_pr_vbe_emerg("parse xml od fail,ap:[%d]%s,%s\n",i,driver_str[timing_id].timing_index_str,driver_str[timing_id].od_en_str);
			return -1;
		}

		//parse dexc
		for(i = 0; i < sizeof(dexc_str)/sizeof(char *); i++)
		{
			if(strcmp(driver_str[timing_id].dexc_en_str, dexc_str[i]) == 0)
			{
				driver_config[timing_id].dexc_en= dexc[i];
				break;
			}

		}
		if(i == sizeof(dexc_str)/sizeof(char *))
		{
			rtd_pr_vbe_emerg("parse xml dexc fail,ap:[%d]%s,%s\n",i,driver_str[timing_id].timing_index_str,driver_str[timing_id].dexc_en_str);
			return -1;
		}

		//parse free run
		for(i = 0; i < sizeof(free_run_str)/sizeof(char *); i++)
		{
			if(strcmp(driver_str[timing_id].free_run_str, free_run_str[i]) == 0)
			{
				driver_config[timing_id].free_run= free_run[i];
				break;
			}
		}
		if(i == sizeof(free_run_str)/sizeof(char *))
		{
			rtd_pr_vbe_emerg("parse xml free_run fail,ap:[%d]%s,%s\n",i,driver_str[timing_id].timing_index_str,driver_str[timing_id].free_run_str);
			return -1;
		}

		//parse ve compressive
		for(i = 0; i < sizeof(ve_comp_str)/sizeof(char *); i++)
		{
			if(strcmp(driver_str[timing_id].ve_comp_str, ve_comp_str[i]) == 0)
			{
				driver_config[timing_id].ve_comp= ve_comp[i];
				break;
			}
		}
		if(i == sizeof(ve_comp_str)/sizeof(char *))
		{
			rtd_pr_vbe_emerg("parse xml ve_comp fail,ap:[%d]%s,%s\n",i,driver_str[timing_id].timing_index_str,driver_str[timing_id].ve_comp_str);
			return -1;
		}

		//parse di pqc bit
		for(i = 0; i < sizeof(dipqc_bit_str)/sizeof(char *); i++)
		{
			if(strcmp(driver_str[timing_id].dipqc_bit_str, dipqc_bit_str[i]) == 0)
			{
				driver_config[timing_id].dipqc_bit= dipqc_bit[i];
				break;
			}
		}
		if(i == sizeof(dipqc_bit_str)/sizeof(char *))
		{
			rtd_pr_vbe_emerg("parse xml dipqc_bit fail,ap:[%d]%s,%s\n",i,driver_str[timing_id].timing_index_str,driver_str[timing_id].dipqc_bit_str);
			return -1;
		}

		//parse ma-snr
		for(i = 0; i < sizeof(ma_snr_str)/sizeof(char *); i++)
		{
			if(strcmp(driver_str[timing_id].ma_snr_str, ma_snr_str[i]) == 0)
			{
				driver_config[timing_id].ma_snr= ma_snr[i];
				break;
			}
		}
		if(i == sizeof(ma_snr_str)/sizeof(char *))
		{
			rtd_pr_vbe_emerg("parse xml ma-snr fail,ap:[%d]%s,%s\n",i,driver_str[timing_id].timing_index_str,driver_str[timing_id].ma_snr_str);
			return -1;
		}

		timing_id++;

	}
	copy_current_driver_table(driver_config);
	return 0;
}


void copy_current_driver_pattern(VSC_DRIVER_PATTERN driver_pattern)
{
	INPUT_TIMING_INDEX i = HDMI_2K1K_30;
	for(i = HDMI_2K1K_30; i < TIMING_INVAILD; i++)
	{
		if(driver_pattern.timing_index == i)
		{
			memcpy(&(vsc_driver_pattern_table[i]), &driver_pattern, sizeof(VSC_DRIVER_PATTERN));
			rtd_pr_vbe_emerg("match driver pattern[%d] OK, copy the setting from AP\n",i);
		}
	}
	if(i == TIMING_INVAILD)
	{
		rtd_pr_vbe_emerg("match the driver pattern fail\n");
	}
	//driver_config_data_to_video();
}

void copy_current_driver_table(VSC_DRIVER_PATTERN *driver_table)
{

	INPUT_TIMING_INDEX i = HDMI_2K1K_30;
	if(driver_table == NULL)
		return;
	for(i = HDMI_2K1K_30; i < TIMING_INVAILD; i++)
	{
		memcpy(&(vsc_driver_pattern_table[i]), driver_table + i, sizeof(VSC_DRIVER_PATTERN));

	}
	//driver_config_data_to_video();
}
#endif 

unsigned char driver_timinginfo_check(unsigned short input_width,unsigned short input_length,unsigned short h_freq,
										unsigned short v_freq,unsigned short h_total,unsigned short v_total)
{
	if(input_width == 0)
	{
		return FALSE;
	}
	else if(input_length == 0)
	{
		return FALSE;
	}
	else if(h_freq == 0)
	{
		return FALSE;
	}
	else if(v_freq == 0)
	{
		return FALSE;
	}
	else if(h_total == 0)
	{
		return FALSE;
	}
	else if(v_total == 0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

extern StructDisplayInfo * Get_ADC_Dispinfo(void);
extern StructDisplayInfo * Get_AVD_ScalerDispinfo(void);
INPUT_TIMING_INDEX get_current_pattern_index(void)
{
	INPUT_TIMING_INDEX index = TIMING_INVAILD;
	unsigned int cur_freq = 0;
	unsigned int cur_act_w = 0;
	unsigned int cur_act_h = 0;
#ifndef BUILD_QUICK_SHOW

	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_AVD)
	{
		StructDisplayInfo* VFEDispInfo = Get_AVD_ScalerDispinfo();
		if(VFEDispInfo == NULL)
		{
			rtd_pr_vbe_emerg("adc avd timinginfo error\n");
			return TIMING_INVAILD;
		}
		if(driver_timinginfo_check(VFEDispInfo->IPH_ACT_WID_PRE,VFEDispInfo->IPV_ACT_LEN_PRE,VFEDispInfo->IHFreq,
			VFEDispInfo->IVFreq,VFEDispInfo->IHTotal,VFEDispInfo->IVTotal) == FALSE)
		{
			rtd_pr_vbe_emerg("avd timinginfo check error\n");
			return TIMING_INVAILD;
		}

		cur_freq = VFEDispInfo->IVFreq;
		if(cur_freq < 310)
			index = HDMI_2K1K_30;
		else if(cur_freq >= 310 && cur_freq <= 600)
			index = HDMI_2K1K_60;
		else
			index = TIMING_INVAILD;
		return index;
	}
#ifdef CONFIG_SUPPORT_SRC_ADC
	else if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_ADC)
	{
		StructDisplayInfo* VFEDispInfo = Get_ADC_Dispinfo();
		if(VFEDispInfo == NULL)
		{
			rtd_pr_vbe_emerg("adc get timinginfo error\n");
			return TIMING_INVAILD;
		}
		if(driver_timinginfo_check(VFEDispInfo->IPH_ACT_WID_PRE,VFEDispInfo->IPV_ACT_LEN_PRE,VFEDispInfo->IHFreq,
			VFEDispInfo->IVFreq,VFEDispInfo->IHTotal,VFEDispInfo->IVTotal) == FALSE)
		{
			rtd_pr_vbe_emerg("adc timinginfo check error\n");
			return TIMING_INVAILD;
		}

		cur_freq = VFEDispInfo->IVFreq;
		if(cur_freq < 310)
			index = HDMI_2K1K_30;
		else if(cur_freq >= 310 && cur_freq <= 600)
			index = HDMI_2K1K_60;
		else
			index = TIMING_INVAILD;
		return index;
	}
#endif//#ifdef CONFIG_SUPPORT_SRC_ADC
	else
#endif        
        if ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) || (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP))
	{
		if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) {
#ifdef USE_NEW_HDMI_TIMING_INFO_STRUCTURE
#ifdef CONFIG_RTK_HDMI_RX
			MEASURE_TIMING_T vfehdmitiminginfo;
		if(0 == drvif_Hdmi_GetRawTimingInfo(&vfehdmitiminginfo)){
			rtd_pr_vbe_emerg("hdmi get timinginfo error\n");
			return TIMING_INVAILD;
		}
		if(driver_timinginfo_check(vfehdmitiminginfo.h_act_len,vfehdmitiminginfo.v_act_len,vfehdmitiminginfo.h_freq,
			vfehdmitiminginfo.v_freq,vfehdmitiminginfo.h_total,vfehdmitiminginfo.v_total) == FALSE)
		{
			rtd_pr_vbe_emerg("hdmi timinginfo check error\n");
			return TIMING_INVAILD;
		}

		cur_freq = Scaler_Get_HDMI_VFREQ(vfehdmitiminginfo.v_freq);
		cur_act_w = vfehdmitiminginfo.h_act_len;
		cur_act_h = vfehdmitiminginfo.v_act_len;
		if(vfehdmitiminginfo.is_interlace)
			cur_act_h *= 2;
		rtd_pr_vbe_emerg("[%s] %d HDMI cur_freq=%d, cur_act_w=%d, cur_act_h=%d, is_interlace=%d\n", __FUNCTION__, __LINE__, cur_freq, cur_act_w, cur_act_h, vfehdmitiminginfo.is_interlace);
#endif        
#else
		vfe_hdmi_timing_info_t vfehdmitiminginfo;

		if(0 != vfe_hdmi_drv_get_display_timing_info(&vfehdmitiminginfo, SLR_MAIN_DISPLAY))
		{
			rtd_pr_vbe_emerg("hdmi get timinginfo error\n");
			return TIMING_INVAILD;
		}
		if(driver_timinginfo_check(vfehdmitiminginfo.active.w,vfehdmitiminginfo.active.h,vfehdmitiminginfo.h_freq,
			vfehdmitiminginfo.v_freq,vfehdmitiminginfo.h_total,vfehdmitiminginfo.v_total) == FALSE)
		{
			rtd_pr_vbe_emerg("hdmi timinginfo check error\n");
			return TIMING_INVAILD;
		}

		cur_freq = vfehdmitiminginfo.v_freq;
		cur_act_w = vfehdmitiminginfo.active.w;
		cur_act_h = vfehdmitiminginfo.active.h;
		if(vfehdmitiminginfo.scan_type == 0)
			cur_act_h *= 2;
		rtd_pr_vbe_emerg("[%s] %d HDMI cur_freq=%d, cur_act_w=%d, cur_act_h=%d, scan_type=%d\n", __FUNCTION__, __LINE__, cur_freq, cur_act_w, cur_act_h, vfehdmitiminginfo.scan_type);
#endif
		} else {
#ifndef UT_flag  // Add UT compile flag, because UT can NOT recognize the definition of "IS_ENABLED". (UT will build fail)
#if IS_ENABLED(CONFIG_RTK_DPRX)  // if CONFIG_RTK_DPRX is built-in =m (CONFIG_RTK_DPRX_MODULE) & module =y (CONFIG_RTK_DPRX)
			DPRX_TIMING_INFO_T vfedptiminginfo;

			if(0 == drvif_Dprx_GetRawTimingInfo(&vfedptiminginfo))
			{
				rtd_pr_vbe_emerg("dp get timinginfo error\n");
				return TIMING_INVAILD;
			}
			if(driver_timinginfo_check(vfedptiminginfo.hact,vfedptiminginfo.vact,vfedptiminginfo.hfreq_hz,
				vfedptiminginfo.vfreq_hz_x100,vfedptiminginfo.htotal,vfedptiminginfo.vtotal) == FALSE)
			{
				rtd_pr_vbe_emerg("dp timinginfo check error\n");
				return TIMING_INVAILD;
			}

			cur_freq = vfedptiminginfo.vfreq_hz_x100 / 10;
			cur_act_w = vfedptiminginfo.hact;
			cur_act_h = vfedptiminginfo.vact;
			if(vfedptiminginfo.is_interlace)
				cur_act_h *= 2;
			rtd_pr_vbe_emerg("[%s] %d DP cur_freq=%d, cur_act_w=%d, cur_act_h=%d, is_interlace=%d\n", __FUNCTION__, __LINE__, cur_freq, cur_act_w, cur_act_h, vfedptiminginfo.is_interlace);
#endif
#endif
		}

		if((cur_act_w <= 1920)||(cur_act_h <= 1080))
		{
			if(cur_freq < 310)
				index = HDMI_2K1K_30;
			else if(cur_freq >= 310 && cur_freq <= 610)
				index = HDMI_2K1K_60;
			else if(cur_freq > 610)
			{
				if(Get_DISPLAY_REFRESH_RATE() >= 120)
				{//use 120 hz panel
					if(((cur_freq > 990) && (cur_freq < 1010)) || ((cur_freq > 1190) && (cur_freq < 1210)))
					{
						index = HDMI_2K1K_120;//change to 2k 120
					}
					else
					{//for 75 85 hz
						index = HDMI_2K1K_60;
					}
				}
				else
					index = HDMI_2K1K_60;
			}
			//else
			//	index = TIMING_INVAILD;

		}
		 else if((cur_act_w <= 4096)&&(cur_act_h <= 2160))
		{
			if(cur_freq < 310)
				index = HDMI_4K2K_30;
			else if(cur_freq >= 310 && cur_freq <= 610)
			{
				if(drvif_Hdmi_IsDRMInfoReady())
					index = HDMI_HDR_4K2K_60;
				else
					index = HDMI_4K2K_60;
			}
			else if(cur_freq > 610)
			{
				//if(Get_DISPLAY_REFRESH_RATE() >= 120)
				{//use 120 hz panel
					if(((cur_freq > 990) && (cur_freq < 1010)) || ((cur_freq > 1190) && (cur_freq < 1210)))
					{//100 hz
						index = HDMI_4K2K_120;//change to 4k 120
					}
					else
					{//for 75 85 hz
						index = HDMI_4K2K_60;
					}
				}
				//else
					//index = HDMI_4K2K_60;
			}
			//else
			//	index = TIMING_INVAILD;
		}
		else
		{
			if(cur_freq < 310)
				index = HDMI_8K4K_30;
			else if(cur_freq >= 310 && cur_freq <= 610)
			{
				if(drvif_Hdmi_IsDRMInfoReady())
					index = HDMI_HDR_8K4K_60;
				else
					index = HDMI_8K4K_60;
			}
			else
				index = TIMING_INVAILD;
		}
		rtd_pr_vbe_emerg("[%s] %d HDMI/DP current index=%d\n", __FUNCTION__, __LINE__, index);
		return index;
	}
	else if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA)
	{
		SLR_VOINFO *pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
		if(pVOInfo == NULL)
		{
			rtd_pr_vbe_emerg("vo get timinginfo error\n");
			return TIMING_INVAILD;
		}
		if(driver_timinginfo_check(pVOInfo->h_width,pVOInfo->v_length,pVOInfo->h_freq,
			pVOInfo->v_freq,pVOInfo->h_total,pVOInfo->v_total) == FALSE)
		{
			rtd_pr_vbe_emerg("vo timinginfo check error\n");
			return TIMING_INVAILD;
		}

		cur_freq = pVOInfo->v_freq;
		cur_act_w = pVOInfo->h_width;
		cur_act_h = pVOInfo->v_length;
		if(pVOInfo->progressive == 0)
			cur_act_h *= 2;
        if((cur_act_w <= 1920)||(cur_act_h <= 1088))
		{
			if(cur_freq < 310)
				index = VO_2K1K_30;
			else if(cur_freq >= 310 && cur_freq <= 600)
				index = VO_2K1K_60;
			else if(cur_freq > 600)
			{
			//	if(Get_DISPLAY_REFRESH_RATE() >= 120)
				{//use 120 hz panel
					if(((cur_freq > 990) && (cur_freq < 1010)) || ((cur_freq > 1190) && (cur_freq < 1210)))
					{
						index = VO_2K1K_120;//change to 2k 120
					}
					else
					{//for 75 85 hz
						index = VO_2K1K_60;
					}
				}
				//else
					//index = VO_2K1K_60;

                if((cur_act_w > 3800) && (cur_act_w <= 4096) && (cur_freq > 1190) && (cur_freq < 1210))
                {
                    index = VO_4K1K_120;
                }
			}
			//else
			//	index = TIMING_INVAILD;

		}
		else if ((cur_act_w <= 4096)&&(cur_act_h <= 2160))
		{
			if(pVOInfo->is8K) {//really source is 8k
				if(cur_freq < 310)
					index = VO_8K4K_30;
				else if(cur_freq >= 310 && cur_freq <= 600)
					index = VO_8K4K_60;
				else
					index = TIMING_INVAILD;
			} else if(cur_freq < 310)
				index = VO_4K2K_30;
			else if(cur_freq >= 310 && cur_freq <= 600)
				index = VO_4K2K_60;
			else if(((cur_freq > 990) && (cur_freq < 1010)) || ((cur_freq > 1190) && (cur_freq < 1210)))
					index = VO_4K2K_120;
			else
				index = TIMING_INVAILD;
		}
		else{
			if(cur_freq < 310)
				index = VO_8K4K_30;
			else if(cur_freq >= 310 && cur_freq <= 600)
				index = VO_8K4K_60;
			else
				index = TIMING_INVAILD;
		}
		if ((index!=TIMING_INVAILD)&&((Scaler_VOFromVDEC(Scaler_Get_CurVoInfo_plane()) == SOURCE_DIRECTVO) &&
				(get_vsc_run_adaptive_stream(SLR_MAIN_DISPLAY)==ADAPTIVE_STREAM_ON) && (0==rtk_hal_vsc_Getdualdecoder()&& (pVOInfo->is2KCP == 0)))) {
			index=index+CP_2K1K_30-VO_2K1K_30;//cp case need to get cp index
		}
		return index;
	} else if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG) {
		SLR_VOINFO *pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
		if(pVOInfo == NULL)
		{
			rtd_pr_vbe_emerg("vo get timinginfo error\n");
			return TIMING_INVAILD;
		}
		if(driver_timinginfo_check(pVOInfo->h_width,pVOInfo->v_length,pVOInfo->h_freq,
			pVOInfo->v_freq,pVOInfo->h_total,pVOInfo->v_total) == FALSE)
		{
			rtd_pr_vbe_emerg("vo timinginfo check error\n");
			return TIMING_INVAILD;
		}

		cur_act_w = pVOInfo->h_width;
		cur_act_h = pVOInfo->v_length;
		if(pVOInfo->progressive == 0)
			cur_act_h *= 2;

		if((cur_act_w <= 1920)&&(cur_act_h <= 1088))
		{
			index = JPEG_2K1K_30;
		} else if((cur_act_w > 1920)&&(cur_act_h <= 1088)) {
			index = JPEG_2K1K_30;
		}else if((cur_act_w <= 4096)&&(cur_act_h <= 2160))
		{
			index = JPEG_4K2K_30;
		}
		else
		{
			index = JPEG_8K4K_30;
		}
		return index;
	}
	else
	{
		rtd_pr_vbe_emerg("get input source type error\n");
		return TIMING_INVAILD;
	}
}

unsigned int get_current_driver_pattern(DRIVER_LIST_INFO info)
{
	INPUT_TIMING_INDEX index = get_current_pattern_index();

	if(index == TIMING_INVAILD)
	{
		rtd_pr_vbe_err("get_current_driver_pattern get input timing index is TIMING_INVAILD\n");
	}
	switch(info)
	{
		case DRIVER_RTNR_PATTERN:
			return vsc_driver_pattern_table[index].rtnr_mode;
		case DRIVER_OD_PATTERN:
			return vsc_driver_pattern_table[index].od_en;
		case DRIVER_DEXC_PATTERN:
			return vsc_driver_pattern_table[index].dexc_en;
		case DRIVER_FREERUN_PATTERN:
#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
			return 0;
#endif
			if (((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) && (Get_DISPLAY_PANEL_CUSTOM_INDEX() != VBY_ONE_PANEL_COMMERCIAL_4K2K_600M_1S_8L_DUPLICATE)) ||
				(Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI) ||
				(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)) {//P_VBY1_TO_HDMI
				return 0;
			} else
			{
			#ifndef BUILD_QUICK_SHOW
				if ((get_platform_model() == PLATFORM_MODEL_8K)&& (get_display_resolution()==DISPLAY_RESOLUTION_8K)) { // 4k timing go data frc for 8k resolution 
					if ((index==HDMI_4K2K_30)||(index==HDMI_4K2K_60)
						||(index==VO_4K2K_30)||(index==VO_4K2K_60)
						||(index==HDMI_HDR_4K2K_60)||(index==JPEG_4K2K_30)) {
						rtd_pr_vbe_emerg("data frc for 8k solution\n");
						return 0;
					}
				}
            #endif
				return vsc_driver_pattern_table[index].free_run;
			}
		case DRIVER_VECOMP_PATTERN:
			return vsc_driver_pattern_table[index].ve_comp;
		case DRIVER_MASNR_PATTERN:
			return vsc_driver_pattern_table[index].ma_snr;
		case DRIVER_DIPQC_BIT_PATTERN:
			return vsc_driver_pattern_table[index].dipqc_bit;
		default:
			rtd_pr_vbe_emerg("get input timing index error\n");
			return 0;
	}
}
#ifndef BUILD_QUICK_SHOW

void print_current_driver_pattern(void)
{
	INPUT_TIMING_INDEX i = HDMI_2K1K_30;
	char *timing_string[] = {
		"HDMI_2K1K_30",
		"HDMI_2K1K_60",
		"HDMI_2K1K_120",
		"HDMI_4K2K_30",
		"HDMI_4K2K_60",
		"HDMI_4K2K_120",
		"HDMI_8K4K_30",
		"HDMI_8K4K_60",
		"VO_2K1K_30",
		"VO_2K1K_60",
		"VO_2K1K_120",
		"VO_4K1K_120",
		"VO_4K2K_30",
		"VO_4K2K_60",
		"VO_4K2K_120",
		"VO_8K4K_30",
		"VO_8K4K_60",
		"CP_2K1K_30",
		"CP_2K1K_60",
		"CP_2K1K_120",
		"CP_4K1K_120",
		"CP_4K2K_30",
		"CP_4K2K_60",
		"CP_4K2K_120",
		"CP_8K4K_30",
		"CP_8K4K_60",
		"HDMI_HDR_4K2K_60",
		"HDMI_HDR_8K4K_60",
		"JPEG_2K1K_30",
		"JPEG_4K2K_30",
		"JPEG_8K4K_30",
		"timing invaild"
	};
	INPUT_TIMING_INDEX index = get_current_pattern_index();
	rtd_pr_vbe_emerg("****************Driver Pattern table********************\n");

	for(i = HDMI_2K1K_30; i < TIMING_INVAILD; i++)
	{
		rtd_pr_vbe_emerg("%s,%d,%d,%d,%d,%d,%d,%d\n",timing_string[i],vsc_driver_pattern_table[i].rtnr_mode,vsc_driver_pattern_table[i].od_en,
			vsc_driver_pattern_table[i].dexc_en,vsc_driver_pattern_table[i].free_run,vsc_driver_pattern_table[i].ve_comp,
			vsc_driver_pattern_table[i].dipqc_bit,vsc_driver_pattern_table[i].ma_snr);
	}
	rtd_pr_vbe_emerg("current timing info:\n");
	rtd_pr_vbe_emerg("%s,%d,%d,%d,%d,%d,%d,%d\n",timing_string[index],get_current_driver_pattern(DRIVER_RTNR_PATTERN),get_current_driver_pattern(DRIVER_OD_PATTERN),
			get_current_driver_pattern(DRIVER_DEXC_PATTERN),get_current_driver_pattern(DRIVER_FREERUN_PATTERN),get_current_driver_pattern(DRIVER_VECOMP_PATTERN),
			get_current_driver_pattern(DRIVER_DIPQC_BIT_PATTERN),get_current_driver_pattern(DRIVER_MASNR_PATTERN));
	rtd_pr_vbe_emerg("**************************************************\n");
	//driver_config_data_to_video();

}

void driver_config_data_to_video(void)
{
	unsigned int *pulTemp = NULL;
	unsigned int i=0, ulItemCount = 0;
	//DRIVER_CONFIG_DATA_INFO driver_config_info;

	pulTemp = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_DRIVER_CONFIG_INFO);
	ulItemCount = sizeof(DRIVER_CONFIG_DATA_INFO) /  sizeof(unsigned int);
	if (pulTemp) {
		memcpy(pulTemp, &vsc_driver_pattern_table, sizeof(DRIVER_CONFIG_DATA_INFO));
		for (i = 0; i < ulItemCount; i++)
		{
			pulTemp[i] = htonl(pulTemp[i]);

		}
#if 1
		if (0 != Scaler_SendRPC(SCALERIOC_DRIVER_CONFIG_INFO,0,0))
		{
			rtd_pr_vbe_debug("send driver config info fail !!!\n");
			return;

		}
#endif
	}

	return;
}


void enable_clock_mux_new_mode(void)
{//this is ECN for dispi clock non-glitch mux for MAC6-566
	vgip_v8_ctrl_RBUS vgip_v8_ctrl_reg;
	vgip_v8_ctrl_reg.regValue = IoReg_Read32(VGIP_V8_CTRL_reg);
	vgip_v8_ctrl_reg.state_period = 4;//DIS suggestion value
	IoReg_Write32(VGIP_V8_CTRL_reg, vgip_v8_ctrl_reg.regValue);
	vgip_v8_ctrl_reg.regValue = IoReg_Read32(VGIP_V8_CTRL_reg);
	vgip_v8_ctrl_reg.clk_mux_old_mode_i3 = 0;// 0: new mode  1: old mode. dma vgip
	vgip_v8_ctrl_reg.clk_mux_old_mode_i1= 0;// 0: new mode  1: old mode. main vgip
	vgip_v8_ctrl_reg.clk_mux_old_mode_s1= 1;// 0: new mode  1: old mode. main uzd
	IoReg_Write32(VGIP_V8_CTRL_reg, vgip_v8_ctrl_reg.regValue);
}

extern void * __init get_platform_resource(void);

int copy_driver_table_from_bootload(void)
{
#if 0

	void *driver_resource = NULL;
	driver_resource = (void *)get_platform_resource();
	if(driver_resource == NULL)
		return -1;

	if(parse_xml_driver_string((VSC_DRIVER_STRING *)driver_resource) == 0)
		return 0;
	else
		return -1;
#endif
return -1;
}


#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
static struct semaphore dolby_vision_dm_path_sem;//for dm table patch control semaphore

void init_dolby_vision_dm_path_sem(void)
{
	sema_init(&dolby_vision_dm_path_sem, 1);
}

struct semaphore *get_dolby_vision_dm_path_sem(void)
{
	return &dolby_vision_dm_path_sem;
}

void dolby_ott_dm_init(void)
{//for dolby dm table source select. chnage to dolby dm atble
	dm_hdr_path_sel_RBUS dm_hdr_path_sel_reg;
	dm_dmato3dtable_ctrl_RBUS dm_dmato3dtable_ctrl_reg;
	dm_dm_b05_lut_ctrl2_RBUS dm_dm_b05_lut_ctrl2_Reg;
	dm_hdr_double_buffer_ctrl_RBUS dm_double_buffer_ctrl_reg;

	down(&dolby_vision_dm_path_sem);
	dm_hdr_path_sel_reg.regValue = 0;
	IoReg_Write32(DM_HDR_PATH_SEL_reg, dm_hdr_path_sel_reg.regValue);

	dm_dmato3dtable_ctrl_reg.regValue = 0;
	IoReg_Write32(DM_DMAto3Dtable_CTRL_reg,dm_dmato3dtable_ctrl_reg.regValue);


	dm_dm_b05_lut_ctrl2_Reg.regValue = rtd_inl(DM_DM_B05_LUT_CTRL2_reg);
	#if 0  //m6 spec removed
	dm_dm_b05_lut_ctrl2_Reg.dm_b05_dimension = 0;//reset to 0(table 17*17*17*3);
	#endif
	IoReg_Write32(DM_DM_B05_LUT_CTRL2_reg, dm_dm_b05_lut_ctrl2_Reg.regValue);

		//apply
	dm_double_buffer_ctrl_reg.regValue = IoReg_Read32(DM_HDR_Double_Buffer_CTRL_reg);
	dm_double_buffer_ctrl_reg.dm_db_apply = 1;
	IoReg_Write32(DM_HDR_Double_Buffer_CTRL_reg, dm_double_buffer_ctrl_reg.regValue);
	up(&dolby_vision_dm_path_sem);

}
#endif
#endif

/*merlin4 new function*/
//Re-order control for HDMI Dolby
static unsigned char i3ddma_dolby_reorder_flag = _DISABLE;
void set_i3ddma_dolby_reorder_mode(unsigned char enable){
	i3ddma_dolby_reorder_flag = enable;
}

unsigned char get_i3ddma_dolby_reorder_mode(void){
	return i3ddma_dolby_reorder_flag;
}

void i3ddma_dolby_reorder_update_color_format(void)
{
	//reorder output format is 422 12bits
	Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_COLOR_SPACE, COLOR_YUV422);
	Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_COLOR_DEPTH, I3DDMA_COLOR_DEPTH_12B);
	Scaler_DispSetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422, 1);
}
#ifndef BUILD_QUICK_SHOW 

#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
void set_sharememory_hdmi_dolby_visf(DOLBY_HDMI_VSIF_T input_dolby_format)
{
	unsigned int *dolby_vsif = NULL;
	dolby_vsif = (DOLBY_HDMI_VSIF_T *)Scaler_GetShareMemVirAddr(HDMI_DOLBY_VSIF_FLAG);
	*dolby_vsif = ntohl(input_dolby_format);

}
#endif
#endif 

unsigned char dp_input_source[2] = {_SRC_MINI_DP, _SRC_MINI_DP};
void Set_Val_dp_input_source(unsigned char display, unsigned char type)
{
    if(display == SLR_MAIN_DISPLAY)
        dp_input_source[0] = type;
    else
        dp_input_source[1] = type;
}

unsigned short get_scaler_connect_source(unsigned char display)
{//get vsc connect source form AP

#ifndef CONFIG_DUAL_CHANNEL
	if(display != SLR_MAIN_DISPLAY)
		return _SRC_MAX;//unknown display error
#endif
	switch(Get_DisplayMode_Src(display))
	{
#ifndef BUILD_QUICK_SHOW	
		case VSC_INPUTSRC_AVD:
			if(get_AVD_Input_Source() == _SRC_TV)
				return _SRC_TV;//atv source
			else
				return _SRC_CVBS;//av source
		break;

		case VSC_INPUTSRC_ADC:
			if(get_ADC_Input_Source() == _SRC_VGA)
				return _SRC_VGA;//vga source. LG not use
			else
				return _SRC_YPBPR;//YPP source
		break;
#endif
		case VSC_INPUTSRC_HDMI:
			return _SRC_HDMI;//HDMI source
		break;

		case VSC_INPUTSRC_VDEC:
        case VSC_INPUTSRC_CAMERA:
			return _SRC_VO;//vo source
		break;

		case VSC_INPUTSRC_DP:
			if(display == SLR_MAIN_DISPLAY)
				return dp_input_source[0];//DP source from vsc parameter
			else
				return dp_input_source[1];//DP source from vsc parameter
		break;

		case VSC_INPUTSRC_JPEG:
			return _SRC_VO;//vo source
		break;

		default:
			return _SRC_MAX;//unknown
		break;
	}
	return _SRC_MAX;//unknown
}



unsigned short get_scaler_ivs_framerate(void)
{//get scale main path ivs frame rate. current is for HDMI VRR
	unsigned int framerate = 0x1017DF80;//27000000 * 10
	ppoverlay_ivs_cnt_3_RBUS ppoverlay_ivs_cnt_3_reg;
	ppoverlay_ivs_cnt_3_reg.regValue = IoReg_Read32(PPOVERLAY_IVS_cnt_3_reg);
	if(ppoverlay_ivs_cnt_3_reg.ivs_cnt_ori)
	{
		framerate = (unsigned int)framerate / (unsigned int)ppoverlay_ivs_cnt_3_reg.ivs_cnt_ori;
		return (unsigned short)framerate;
	}
	else
	{
		return Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ);
	}
}

unsigned char check_hdmi_uhd_condition(void)
{
	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) && (get_force_i3ddma_enable(SLR_MAIN_DISPLAY)))
	{
		unsigned int hdmi_wid = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHWID);
		unsigned int hdmi_len = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVHEIGHT);
		unsigned int hdmi_vfreq = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ);
		if((hdmi_wid > 2560) && (hdmi_len > 1440) && (hdmi_vfreq > 490))//4k2k 50 60
			return TRUE;
	}
	return FALSE;
}


void two_pixel_mode_ctrl(unsigned char enable)
{//for 4k 120/ 2 pixel mode
	vgip_data_path_select_RBUS vgip_data_path_select_reg;
	sub_vgip_vgip_chn2_double_buffer_ctrl_RBUS sub_vgip_vgip_chn2_double_buffer_ctrl_reg;
	sub_vgip_vgip_chn2_ctrl_RBUS sub_vgip_vgip_chn2_ctrl_reg;

	if(enable)
	{
		sub_vgip_vgip_chn2_double_buffer_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_DOUBLE_BUFFER_CTRL_reg);
		sub_vgip_vgip_chn2_double_buffer_ctrl_reg.ch2_db_en = 0;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_DOUBLE_BUFFER_CTRL_reg, sub_vgip_vgip_chn2_double_buffer_ctrl_reg.regValue);

		sub_vgip_vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		sub_vgip_vgip_chn2_ctrl_reg.ch2_in_clk_en = 1;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);

		vgip_data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);
		vgip_data_path_select_reg.main_2pxl_en = 1;
		vgip_data_path_select_reg.uzd2_in_sel = 3;//MERLIN5-2773 DIC suggesting
		vgip_data_path_select_reg.s2_clk_en = 1;
		IoReg_Write32(VGIP_Data_Path_Select_reg, vgip_data_path_select_reg.regValue);

		/*
		sub_vgip_vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		sub_vgip_vgip_chn2_ctrl_reg.ch2_in_clk_en = 0;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);
		*/
	}
	else
	{
		vgip_data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);
		vgip_data_path_select_reg.main_2pxl_en = 0;
		if(get_hdmi_4k_hfr_mode() == HDMI_4K120_2_0)
		{
			vgip_data_path_select_reg.uzd2_in_sel = 0;//Recover from hdmi2.1
		}
		IoReg_Write32(VGIP_Data_Path_Select_reg, vgip_data_path_select_reg.regValue);
	}
}

void twopixel_disable_sub_vgip_clock(void)
{//disable vgip clock for 4k120 hdmi2.1
	sub_vgip_vgip_chn2_ctrl_RBUS sub_vgip_vgip_chn2_ctrl_reg;
	sub_vgip_vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
	sub_vgip_vgip_chn2_ctrl_reg.ch2_in_clk_en = 0;
	IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);
}

void twopixel_2_oneline_ctrl(unsigned char enable)
{//enable or disable 2pixel mode for ecn MERLIN5-945
 	ptol_ptol_ctrl2_RBUS ptol_ptol_ctrl2_reg;
	ptol_ptol_ctrl_RBUS ptol_ptol_ctrl_reg;
	ptol_ptol_db_ctrl_RBUS  ptol_ptol_db_ctrl_reg;
	ptol_ptol_db_ctrl_reg.regValue = IoReg_Read32(PTOL_PtoL_DB_CTRL_reg);
	ptol_ptol_db_ctrl_reg.double_buf_en = 1;//enable double buffer
	ptol_ptol_db_ctrl_reg.double_buf_read_level = 1;//change rbus level
	IoReg_Write32(PTOL_PtoL_DB_CTRL_reg, ptol_ptol_db_ctrl_reg.regValue);

	if(enable)
	{
		ptol_ptol_ctrl_reg.regValue = IoReg_Read32(PTOL_PtoL_ctrl_reg);
		ptol_ptol_ctrl_reg.den_width = Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID);
		ptol_ptol_ctrl_reg.den_height = Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_LEN);
		IoReg_Write32(PTOL_PtoL_ctrl_reg, ptol_ptol_ctrl_reg.regValue);

		ptol_ptol_ctrl2_reg.regValue = IoReg_Read32(PTOL_PtoL_ctrl2_reg);
		ptol_ptol_ctrl2_reg.ptol_en = 1;
		if(Get_PANEL_VFLIP_ENABLE())
			ptol_ptol_ctrl2_reg.line_swap_en = 1;//for vflip
		else
			ptol_ptol_ctrl2_reg.line_swap_en = 0;//for vflip
#ifndef BUILD_QUICK_SHOW 
        ptol_ptol_ctrl2_reg.hflip_en = get_2Pto1L_hflip_flag();
#endif
		IoReg_Write32(PTOL_PtoL_ctrl2_reg, ptol_ptol_ctrl2_reg.regValue);
	}
	else
	{
		ptol_ptol_ctrl_reg.regValue = IoReg_Read32(PTOL_PtoL_ctrl_reg);
		ptol_ptol_ctrl_reg.den_width = 0;
		ptol_ptol_ctrl_reg.den_height = 0;
		IoReg_Write32(PTOL_PtoL_ctrl_reg, ptol_ptol_ctrl_reg.regValue);

		ptol_ptol_ctrl2_reg.regValue = IoReg_Read32(PTOL_PtoL_ctrl2_reg);
		ptol_ptol_ctrl2_reg.ptol_en = 0;
		ptol_ptol_ctrl2_reg.line_swap_en = 0;//for vflip
		ptol_ptol_ctrl2_reg.hflip_en = 0;
		IoReg_Write32(PTOL_PtoL_ctrl2_reg, ptol_ptol_ctrl2_reg.regValue);
	}


	ptol_ptol_db_ctrl_reg.regValue = IoReg_Read32(PTOL_PtoL_DB_CTRL_reg);
	ptol_ptol_db_ctrl_reg.double_buf_set = 1;//apply double bfufe
	IoReg_Write32(PTOL_PtoL_DB_CTRL_reg, ptol_ptol_db_ctrl_reg.regValue);
}

void get_vtop_input_info(unsigned char *twopixel_mode, unsigned char *hdmi_input, unsigned char *color_format)
{
	VSC_INPUT_TYPE_T src = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
	*twopixel_mode = (is_4k_hfr_mode(SLR_MAIN_DISPLAY)) ? TRUE: FALSE;
	*hdmi_input = get_hdr1_in_sel() ? TRUE: FALSE;

	if(src == VSC_INPUTSRC_HDMI)
	{
		if(*hdmi_input)
		{
			*color_format = drvif_Hdmi_GetColorSpace();
		}
		else
		{
			switch(drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR))
			{
				case I3DDMA_COLOR_YUV444:
				default:
				*color_format = COLOR_YUV444;
				break;

				case I3DDMA_COLOR_YUV422:
				*color_format = COLOR_YUV422;
				break;

				case I3DDMA_COLOR_RGB:
				*color_format = I3DDMA_COLOR_RGB;
				break;
			}
		}
	}
	else if(src == VSC_INPUTSRC_DP) {
#if IS_ENABLED(CONFIG_RTK_DPRX)  // if CONFIG_RTK_DPRX is built-in =m (CONFIG_RTK_DPRX_MODULE) & module =y (CONFIG_RTK_DPRX)
		if(*hdmi_input) {
			DPRX_TIMING_INFO_T dp_timing_info = {0};

			if(0 == drvif_Dprx_GetRawTimingInfo(&dp_timing_info)) {
				rtd_pr_vbe_emerg("dp get timinginfo error\n");
				return;
			}

			switch (dp_timing_info.color_space) {
				case DP_COLOR_SPACE_YUV422:
					*color_format = COLOR_YUV422;
				break;
				case DP_COLOR_SPACE_RGB:
					*color_format = COLOR_RGB;
				break;
				case DP_COLOR_SPACE_YUV444:
				default:
					*color_format = COLOR_YUV444;
			}
		} else {
			switch(drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR)) {
				case I3DDMA_COLOR_YUV444:
				default:
					*color_format = COLOR_YUV444;
				break;

				case I3DDMA_COLOR_YUV422:
					*color_format = COLOR_YUV422;
				break;

				case I3DDMA_COLOR_RGB:
					*color_format = COLOR_RGB;
				break;
			}
		}
#endif
	}
	else if(src == VSC_INPUTSRC_VDEC || src == VSC_INPUTSRC_CAMERA)
	{//vdec
		*color_format = COLOR_YUV422;
	}
	else if(src == VSC_INPUTSRC_JPEG)
	{//vdec
		*color_format = COLOR_YUV444;
	}
	else
	{//avd adc
		switch(drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR))
		{
			case I3DDMA_COLOR_YUV444:
			default:
			*color_format = COLOR_YUV444;
			break;

			case I3DDMA_COLOR_YUV422:
			*color_format = COLOR_YUV422;
			break;

			case I3DDMA_COLOR_RGB:
			*color_format = I3DDMA_COLOR_RGB;
			break;
		}
	}

}

unsigned char check_hdr_block_use(unsigned char display)
{//checj hdr block use or not return 1: use  return 0: no use
    if(display == SLR_MAIN_DISPLAY)
    {
        if(VGIP_VGIP_CHN1_CTRL_get_ch1_in_sel(IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg)) == VGIP_SRC_HDR1)//hdr1
            return TRUE;
    }
    else
    {
        if(SUB_VGIP_VGIP_CHN2_CTRL_get_ch2_in_sel(IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg)) == VGIP_SRC_HDR2)//hdr2
            return TRUE;
    }
    return FALSE;
}

unsigned char force_enable_two_step_uzu(void)
{//TRUE: use two step UZU not normal uzu   2k 120 case
	if(get_MEMC_bypass_status_refer_platform_model()) {
		if(get_panel_pixel_mode() > PANEL_1_PIXEL)
			return TRUE;
	}
    if((get_panel_pixel_mode() > PANEL_1_PIXEL) &&
        ((((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) >= 994) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) <= 1005)) 
        || ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) >= 1191) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) <= 1206)))
        || ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) >= 1435) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) <= 1445))
        || ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) >= 2395) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) <= 2405))
        || ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) >= 2875) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) <= 2885))
        || ((get_vsc_src_is_hdmi_or_dp()) && (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag()))))
		return TRUE;
    else if((get_panel_pixel_mode() > PANEL_1_PIXEL) && (scaler_vsc_get_gamemode_force_framerate_lowdelay_mode()==1))
        return TRUE;
    else
        return FALSE;
}

#ifndef BUILD_QUICK_SHOW 
void set_bbd_vgip_driver(unsigned char display, StructDisplayInfo* VFEDispInfo)
{
	if(VFEDispInfo == NULL)
	{
		rtd_pr_vbe_err("\r### error func:%s VFEDispInfo is null ###\r\n", __FUNCTION__);
		return;	
	}
	
	if(display == SLR_MAIN_DISPLAY)	
	{
		vgip_vgip_chn1_ctrl_RBUS vgip_vgip_chn1_ctrl_reg;
		vgip_vgip_chn1_act_hsta_width_RBUS vgip_vgip_chn1_act_hsta_width_reg;
		vgip_vgip_chn1_act_vsta_length_RBUS vgip_vgip_chn1_act_vsta_length_reg;
		vgip_vgip_chn1_delay_RBUS vgip_vgip_chn1_delay_reg;
		vgip_vgip_chn1_misc_RBUS vgip_vgip_chn1_misc_reg;
		vgip_ich1_vgip_htotal_RBUS vgip_ich1_vgip_htotal_reg;

		//set size
		vgip_vgip_chn1_act_hsta_width_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);
		vgip_vgip_chn1_act_hsta_width_reg.ch1_ih_act_sta = 0;
		vgip_vgip_chn1_act_hsta_width_reg.ch1_ih_act_wid = VFEDispInfo->IPH_ACT_WID_PRE;
		IoReg_Write32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg, vgip_vgip_chn1_act_hsta_width_reg.regValue);

		vgip_vgip_chn1_act_vsta_length_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);
		vgip_vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta = 0;
		vgip_vgip_chn1_act_vsta_length_reg.ch1_iv_act_len = VFEDispInfo->IPV_ACT_LEN_PRE;
		IoReg_Write32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg, vgip_vgip_chn1_act_vsta_length_reg.regValue);

		//set delay
		vgip_vgip_chn1_delay_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_DELAY_reg);
		vgip_vgip_chn1_delay_reg.ch1_ihs_dly = 0;
		vgip_vgip_chn1_delay_reg.ch1_ivs_dly = 0;
		IoReg_Write32(VGIP_VGIP_CHN1_DELAY_reg, vgip_vgip_chn1_delay_reg.regValue);


		//set no-hsync
		vgip_vgip_chn1_misc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_MISC_reg);
		vgip_vgip_chn1_misc_reg.ch1_hs_mode = 0;
		IoReg_Write32(VGIP_VGIP_CHN1_MISC_reg, vgip_vgip_chn1_misc_reg.regValue);

		//set h total
		vgip_ich1_vgip_htotal_reg.regValue = IoReg_Read32(VGIP_ICH1_VGIP_HTOTAL_reg);
		vgip_ich1_vgip_htotal_reg.ch1_htotal_num = VFEDispInfo->IHTotal - 1;//set h toal
		IoReg_Write32(VGIP_ICH1_VGIP_HTOTAL_reg, vgip_ich1_vgip_htotal_reg.regValue);

		//set vgip clock
		vgip_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
		vgip_vgip_chn1_ctrl_reg.ch1_ivrun = 1;
		vgip_vgip_chn1_ctrl_reg.ch1_in_clk_en = 1;
		vgip_vgip_chn1_ctrl_reg.ch1_digital_mode = 1;
		vgip_vgip_chn1_ctrl_reg.ch1_vact_start_ie = 1;
		vgip_vgip_chn1_ctrl_reg.ch1_vact_end_ie = 1;
		vgip_vgip_chn1_ctrl_reg.ch1_in_sel = 4;
		IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);
		
	}
	else
	{
		sub_vgip_vgip_chn2_ctrl_RBUS sub_vgip_vgip_chn2_ctrl_reg;
		sub_vgip_vgip_chn2_act_hsta_width_RBUS sub_vgip_vgip_chn2_act_hsta_width_reg;
		sub_vgip_vgip_chn2_act_vsta_length_RBUS sub_vgip_vgip_chn2_act_vsta_length_reg;
		sub_vgip_vgip_chn2_delay_RBUS sub_vgip_vgip_chn2_delay_reg;
		sub_vgip_vgip_chn2_misc_RBUS sub_vgip_vgip_chn2_misc_reg;
		sub_vgip_ich2_vgip_htotal_RBUS sub_vgip_ich2_vgip_htotal_reg;



		//set size
		sub_vgip_vgip_chn2_act_hsta_width_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg);
		sub_vgip_vgip_chn2_act_hsta_width_reg.ch2_ih_act_sta = 0;
		sub_vgip_vgip_chn2_act_hsta_width_reg.ch2_ih_act_wid = VFEDispInfo->IPH_ACT_WID_PRE;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg, sub_vgip_vgip_chn2_act_hsta_width_reg.regValue);

		sub_vgip_vgip_chn2_act_vsta_length_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg);
		sub_vgip_vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = 0;
		sub_vgip_vgip_chn2_act_vsta_length_reg.ch2_iv_act_len = VFEDispInfo->IPV_ACT_LEN_PRE;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg, sub_vgip_vgip_chn2_act_vsta_length_reg.regValue);

		//set delay
		sub_vgip_vgip_chn2_delay_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_DELAY_reg);
		sub_vgip_vgip_chn2_delay_reg.ch2_ihs_dly = 0;
		sub_vgip_vgip_chn2_delay_reg.ch2_ivs_dly = 0;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_DELAY_reg, sub_vgip_vgip_chn2_delay_reg.regValue);


		//set no-hsync
		sub_vgip_vgip_chn2_misc_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_MISC_reg);
		sub_vgip_vgip_chn2_misc_reg.ch2_hs_mode = 0;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_MISC_reg, sub_vgip_vgip_chn2_misc_reg.regValue);

		//set h total
		sub_vgip_ich2_vgip_htotal_reg.regValue = IoReg_Read32(SUB_VGIP_ICH2_VGIP_HTOTAL_reg);
		sub_vgip_ich2_vgip_htotal_reg.ch2_htotal_num = VFEDispInfo->IHTotal - 1;//set h toal
		IoReg_Write32(SUB_VGIP_ICH2_VGIP_HTOTAL_reg, sub_vgip_ich2_vgip_htotal_reg.regValue);

		//set vgip clock
		sub_vgip_vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
		sub_vgip_vgip_chn2_ctrl_reg.ch2_ivrun = 1;
		sub_vgip_vgip_chn2_ctrl_reg.ch2_in_clk_en = 1;
		sub_vgip_vgip_chn2_ctrl_reg.ch2_digital_mode = 1;
		sub_vgip_vgip_chn2_ctrl_reg.ch2_vact_start_ie = 1;
		sub_vgip_vgip_chn2_ctrl_reg.ch2_vact_end_ie = 1;
		sub_vgip_vgip_chn2_ctrl_reg.ch2_in_sel = 5;
		IoReg_Write32(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);
		
	}
}
#endif

void drv_set_datafs_gatting_enable_done(unsigned char enable){
	datafs_gatting_enable = enable;
}

unsigned char drv_get_datafs_gatting_enable_done(void){
	return datafs_gatting_enable;
}
extern unsigned char get_vo_change_flag(unsigned char ch);

void scaler_set_full_gatting_rpc(unsigned int enable)
{
    unsigned char ret;
	UINT32 wait_timeoutcnt = 10; /* timeout 100 ms*/
	UINT32 reset_timeoutcnt = 5;
	if (0 != (ret = Scaler_SendRPC(SCALERIOC_FULL_GATTING_ENABLE, enable, 1))){	
		rtd_pr_vbe_err("ret=%d, SCALERIOC_FULL_GATTING_ENABLE RPC fail !!!\n", ret);
	}
	
	if(1 == enable) { /* wait video set gatting bit done */
		do{
			wait_timeoutcnt = 1000;
			do{
				if(IoReg_Read32(VODMA_VODMA_CLKGEN_reg) & VODMA_VODMA_CLKGEN_en_fifo_full_gate_mask)
					break;
				else
					usleep_range(1000, 1000); /* delay 1 ms*/
			}while(--wait_timeoutcnt);

			if(0 == wait_timeoutcnt && !get_vo_change_flag(Get_DisplayMode_Port(SLR_MAIN_DISPLAY))) {
				rtd_pr_vbe_err("wait video set gatting bit done timeout,reset gatting rpc again!\n");

				if (0 != (ret = Scaler_SendRPC(SCALERIOC_FULL_GATTING_ENABLE, enable, 1))){	
					rtd_pr_vbe_err("ret=%d, SCALERIOC_FULL_GATTING_ENABLE RPC fail !!!\n", ret);
				}
				
			} else {
				drv_set_datafs_gatting_enable_done(TRUE);
				return;
			}
		}while(--reset_timeoutcnt);

		if(0 == reset_timeoutcnt) {
			rtd_pr_vbe_err("reset gatting timeout,the input timing is nostable\n\n");
			IoReg_SetBits(VODMA_VODMA_CLKGEN_reg,VODMA_VODMA_CLKGEN_en_fifo_full_gate_mask);
			drv_set_datafs_gatting_enable_done(TRUE);
		}

	} else {
		drv_set_datafs_gatting_enable_done(FALSE);
	}
}

unsigned char get_vsc_src_is_hdmi_or_dp(void)
{
	// reduce complexity for if condition
	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
		|| (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP))
		return TRUE;
	return FALSE;
}

static unsigned char vsc_go_pc_mode_flag = FALSE;

unsigned char get_vsc_run_pc_mode(void)
{//return run pc mode. return false: run no pc mode setting
    return vsc_go_pc_mode_flag;
}


void set_vsc_run_pc_mode(unsigned char flag)
{
    vsc_go_pc_mode_flag = flag;
}

unsigned char Get_Val_vsc_run_pc_mode(void)
{//return run pc mode. return false: run no pc mode setting
	return vsc_go_pc_mode_flag;
}


void Set_Val_vsc_run_pc_mode(unsigned char flag)
{
	vsc_go_pc_mode_flag = flag;
}

static unsigned char vsc_main_go_adaptive_stream_flag = FALSE;
static unsigned char vsc_sub_go_adaptive_stream_flag = FALSE;
static unsigned char vsc_gamemode_datafrc_mode_flag = FALSE;
unsigned char get_vsc_run_adaptive_stream(unsigned char display)
{//return run adaptive stream.
	if(display == SLR_MAIN_DISPLAY)
		return vsc_main_go_adaptive_stream_flag;
	else
		return vsc_sub_go_adaptive_stream_flag;
}

void set_vsc_run_adaptive_stream(unsigned char display, unsigned char flag)
{
	if(display == SLR_MAIN_DISPLAY)
		vsc_main_go_adaptive_stream_flag = flag;
	else
		vsc_sub_go_adaptive_stream_flag = flag;
}

void set_vsc_gamemode_datafrc_mode_flag(unsigned char flag)
{
	//rtd_pr_vsc_notice("function=%s, flag=%d \n", __FUNCTION__, flag);
	vsc_gamemode_datafrc_mode_flag = flag;
}

unsigned char get_vsc_gamemode_datafrc_mode_flag(void)
{
	return vsc_gamemode_datafrc_mode_flag;
}

#ifndef BUILD_QUICK_SHOW

unsigned char decide_hdmi_hfr_rerun(StructSrcRect input_region, StructSrcRect ori_region, StructSrcRect output_region)
{//return TRUE: rerun scaler //return: FALSE: run smooth toggle
	unsigned char hdmi_4k_hfr_mode = get_hdmi_4k_hfr_mode();
	unsigned char original_uzd_status = FALSE;
	unsigned char target_uzd_status = FALSE;
	unsigned int sdnr_h;

	if(hdmi_4k_hfr_mode == HDMI_4K120_2_1)
	{
#if 0
		original_uzd_status = (Scaler_DispGetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_V_DOWN) || Scaler_DispGetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_H_DOWN)) ? TRUE : FALSE;
		if(ori_region.src_wid)
			sdnr_w = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHWID)*input_region.src_wid /ori_region.src_wid;
		else
			sdnr_w = input_region.src_wid;
		if(ori_region.src_height)
			sdnr_h = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVHEIGHT)*input_region.src_height /ori_region.src_height;
		else
			sdnr_h = input_region.src_height;
		target_uzd_status = ((output_region.src_wid < sdnr_w) || (output_region.src_height < sdnr_h))? TRUE : FALSE;

		if(original_uzd_status == target_uzd_status)
			return FALSE;
		else
		{
			Scaler_DispSetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_H_DOWN, (output_region.src_wid < sdnr_w)? TRUE : FALSE);
			Scaler_DispSetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_V_DOWN, (output_region.src_height < sdnr_h)? TRUE : FALSE);
			return TRUE;
		}
#endif
	}
	else if(hdmi_4k_hfr_mode == HDMI_4K120_2_0)
	{
		original_uzd_status = (Scaler_DispGetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_V_DOWN)) ? TRUE : FALSE;

		if(ori_region.src_height)
			sdnr_h = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVHEIGHT) *input_region.src_height /ori_region.src_height;
		else
			sdnr_h = input_region.src_height;

		target_uzd_status = (output_region.src_height < sdnr_h)? TRUE : FALSE;

		if(original_uzd_status == target_uzd_status)
		{
			return FALSE;
		}
		else
		{
			Scaler_DispSetScaleStatus(SLR_MAIN_DISPLAY, SLR_SCALE_V_DOWN, (output_region.src_height < sdnr_h)? TRUE : FALSE);
			return TRUE;
		}
	}


	return FALSE;

}


/********************************BSP Error Report start********************************************/
static DEFINE_SPINLOCK(BSP_ERROR_REPORT_SPINLOCK);/* This spinlock is for bsp error report buffer */
unsigned char error_run_scaler = FALSE;//some error need to run scaler recover
unsigned char main_mute_time_abnormal = FALSE;//ap alreadu mute off, but driver long time mute
unsigned char sub_mute_time_abnormal = FALSE;//ap alreadu mute off, but driver long time mute

static unsigned int main_check_abnormal_mute_start_time = 0;//recored all mute off start time
static unsigned int sub_check_abnormal_mute_start_time = 0;//recored all mute off start time
#define MUTE_TIME_ABNORMAL_THR 6000//6 second

void check_bsp_error_event(unsigned int err_type, unsigned char video_runtime_suspend_status)
{//check whether set error event flag. video run runtime suspend case or not
    static unsigned char main_mute_mask_status = 1;//record last time check mask status. Originally is true
    static unsigned char sub_mute_mask_status = 1;	//record last time check mask status. Originally is true
    unsigned char main_mute_mask_cur_status, sub_mute_mask_cur_status;//current related mute mask status
    unsigned char scaler_mute0ff;

    switch(err_type)
    {
        case Abnormal_Run_Scaler:
            if(error_run_scaler)
                return;
            error_run_scaler = TRUE;//need to report
            break;

        case Main_Mute_Time_Abnormal:
            if(main_mute_time_abnormal)//wait report
            {
                return;
            }
            main_mute_mask_cur_status = video_runtime_suspend_status || Get_display_forcebg_mask(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_VSC)
            || Get_display_forcebg_mask(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_ZERO_NULL_ARC_MUTE) || Get_display_forcebg_mask(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_HDMI_AVMUTE_MASK);

            if(!main_mute_mask_cur_status)
            {//all mask mute off
                if(main_mute_mask_status)
                {
                    main_mute_mask_status = 0;
                    main_check_abnormal_mute_start_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);//record time
                }
                else
                {
                    scaler_mute0ff = (!PPOVERLAY_Main_Display_Control_RSV_get_m_force_bg(IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg)) && PPOVERLAY_Main_Display_Control_RSV_get_m_disp_en(IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg)));
                    if(!scaler_mute0ff)
                    {//not yet mute to check the time
                        if(drvif_wait_timeout_check_by_counter(main_check_abnormal_mute_start_time, MUTE_TIME_ABNORMAL_THR))
                        {
                            main_mute_time_abnormal = 1;//need report error
                        }
                    }
                    else
                    {//record time again
                        main_check_abnormal_mute_start_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);//record time
                    }
                }
            }
            else
            {
                main_mute_mask_status = 1;
            }

            break;

        case Sub_Mute_Time_Abnormal:
            if(sub_mute_time_abnormal)//wait report
            {
                return;
            }
            sub_mute_mask_cur_status = video_runtime_suspend_status || Get_display_forcebg_mask(SLR_SUB_DISPLAY, SLR_FORCE_BG_TYPE_VSC)
            || Get_display_forcebg_mask(SLR_SUB_DISPLAY, SLR_FORCE_BG_TYPE_ZERO_NULL_ARC_MUTE) || Get_display_forcebg_mask(SLR_SUB_DISPLAY, SLR_FORCE_BG_TYPE_HDMI_AVMUTE_MASK);

            if(!sub_mute_mask_cur_status)
            {//all mask mute off
                if(sub_mute_mask_status)
                {
                    sub_mute_mask_status = 0;
                    sub_check_abnormal_mute_start_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);//record time
                }
                else
                {
                    scaler_mute0ff = (!PPOVERLAY_MP_Layout_Force_to_Background_get_s_force_bg(IoReg_Read32(PPOVERLAY_MP_Layout_Force_to_Background_reg)) && PPOVERLAY_MP_Layout_Force_to_Background_get_s_disp_en(IoReg_Read32(PPOVERLAY_MP_Layout_Force_to_Background_reg)));
                    if(!scaler_mute0ff)
                    {//not yet mute to check the time
                        if(drvif_wait_timeout_check_by_counter(sub_check_abnormal_mute_start_time, MUTE_TIME_ABNORMAL_THR))
                        {
                            sub_mute_time_abnormal = 1;//need report error
                        }
                    }
                    else
                    {//record time again
                        sub_check_abnormal_mute_start_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);//record time
                    }
                }
            }
            else
            {
                sub_mute_mask_status = 1;
            }
            break;

        default:
        break;

    }
}


void decide_report_kernel_bsp_error(unsigned char *outputstring)
{
	unsigned int string_size = 0;
	unsigned char error_message[ERROR_MESSAGE_MAX_STRING_SIZE] = {0};
	if(error_run_scaler)
	{
		string_size = sizeof(ABNORMAL_RUN_SCALER_MESSAGE);
		memcpy(error_message, ABNORMAL_RUN_SCALER_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
		error_run_scaler = 0;
	}
	else if(main_mute_time_abnormal)
	{
		string_size = sizeof(MAIN_MUTE_TIME_ABNORMAL_MESSAGE);
		memcpy(error_message, MAIN_MUTE_TIME_ABNORMAL_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
		main_check_abnormal_mute_start_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);//reset to record time
		main_mute_time_abnormal = 0;
	}
	else if(sub_mute_time_abnormal)
	{
		string_size = sizeof(SUB_MUTE_TIME_ABNORMAL_MESSAGE);
		memcpy(error_message, SUB_MUTE_TIME_ABNORMAL_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
		sub_check_abnormal_mute_start_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);//reset to record time
		sub_mute_time_abnormal = 0;
	}
	if(string_size)
	{
		error_message[ERROR_MESSAGE_MAX_STRING_SIZE - 1] = 0;
		rtd_pr_vbe_info("### [BSP ERROR] report:%s ###\r\n", error_message);
	}
    if(outputstring)
        memcpy(outputstring, error_message, ERROR_MESSAGE_MAX_STRING_SIZE);
}

void BSP_ERROR_BUFFER_RESET(void)
{//set search state
	unsigned long flags = 0;//for spin_lock_irqsave
	BSP_ERROR_REPORT_RING_BUFFER_INFO_T *p_bsp_err_ring_buffer_addr = NULL;
	p_bsp_err_ring_buffer_addr = (BSP_ERROR_REPORT_RING_BUFFER_INFO_T *)p_Scaler_GetShareMemVirAddr(SCALERIOC_BSP_ERROR_REPORT_INFO_BUFFER);
	if(p_bsp_err_ring_buffer_addr)
	{
		spin_lock_irqsave(&BSP_ERROR_REPORT_SPINLOCK, flags);
		p_bsp_err_ring_buffer_addr->reset_read_idx = 1;
		p_bsp_err_ring_buffer_addr->reset_write_idx = 1;
		spin_unlock_irqrestore(&BSP_ERROR_REPORT_SPINLOCK, flags);
	}
}

void BSP_ERROR_CHECK_REPORT(unsigned char *outputstring)
{//call by isr   outputstring: output the error string
    unsigned long flags = 0;//for spin_lock_irqsave
    BSP_ERROR_REPORT_RING_BUFFER_INFO_T *p_bsp_err_ring_buffer_addr = NULL;
    unsigned int read_idx = 0;
    unsigned int string_size = 0;
    unsigned char error_message[ERROR_MESSAGE_MAX_STRING_SIZE] = {0};
    p_bsp_err_ring_buffer_addr = (BSP_ERROR_REPORT_RING_BUFFER_INFO_T *)p_Scaler_GetShareMemVirAddr(SCALERIOC_BSP_ERROR_REPORT_INFO_BUFFER);
    if(p_bsp_err_ring_buffer_addr)
    {//not null
        if(p_bsp_err_ring_buffer_addr->reset_read_idx)
        {//reset case
            spin_lock_irqsave(&BSP_ERROR_REPORT_SPINLOCK, flags);
            p_bsp_err_ring_buffer_addr->read_idx = 0;
            p_bsp_err_ring_buffer_addr->reset_read_idx = 0;
            spin_unlock_irqrestore(&BSP_ERROR_REPORT_SPINLOCK, flags);
        }
        else if(p_bsp_err_ring_buffer_addr->reset_write_idx == 0)
        {
            if(p_bsp_err_ring_buffer_addr->read_idx != p_bsp_err_ring_buffer_addr->write_idx)
            {//has new data
                spin_lock_irqsave(&BSP_ERROR_REPORT_SPINLOCK, flags);
                if(p_bsp_err_ring_buffer_addr->reset_write_idx == 0)
                {
                    read_idx = (Scaler_ChangeUINT32Endian(p_bsp_err_ring_buffer_addr->read_idx) + 1) % BSP_ERROR_REPORT_RING_BUFFER_SIZE;
                    switch (Scaler_ChangeUINT32Endian(p_bsp_err_ring_buffer_addr->error_type_buffer[read_idx]))
                    {
                        case Main_Smooth_Toggle_Fail:
                        string_size = sizeof(MAIN_SMOOTH_TOGGLE_ERROR_MESSAGE);
                        memcpy(error_message, MAIN_SMOOTH_TOGGLE_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case Sub_Smooth_Toggle_Fail:
                        string_size = sizeof(SUB_SMOOTH_TOGGLE_ERROR_MESSAGE);
                        memcpy(error_message, SUB_SMOOTH_TOGGLE_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case Main_M_domain_PQC_error:
                        string_size = sizeof(MAIN_M_DOMAIN_PQC_ERROR_MESSAGE);
                        memcpy(error_message, MAIN_M_DOMAIN_PQC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case Main_M_domain_PQDC_error:
                        string_size = sizeof(MAIN_M_DOMAIN_PQDC_ERROR_MESSAGE);
                        memcpy(error_message, MAIN_M_DOMAIN_PQDC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case Sub_domain_PQC_error:
                        string_size = sizeof(SUB_M_DOMAIN_PQC_ERROR_MESSAGE);
                        memcpy(error_message, SUB_M_DOMAIN_PQC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case Sub_domain_PQDC_error:
                        string_size = sizeof(SUB_M_DOMAIN_PQDC_ERROR_MESSAGE);
                        memcpy(error_message, SUB_M_DOMAIN_PQDC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case Main_VI_error:
                        string_size = sizeof(MAIN_VI_ERROR_MESSAGE);
                        memcpy(error_message, MAIN_VI_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case Sub_VI_error:
                        string_size = sizeof(SUB_VI_ERROR_MESSAGE);
                        memcpy(error_message, SUB_VI_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case Line_Buffer_Sync_Error:
                        string_size = sizeof(LINE_BUFFER_SYNC_ERROR_MESSAGE);
                        memcpy(error_message, LINE_BUFFER_SYNC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case MEMC_PQC_Error:
                        string_size = sizeof(MEMC_PQC_ERROR_MESSAGE);
                        memcpy(error_message, MEMC_PQC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case MEMC_PQDC_Error:
                        string_size = sizeof(MEMC_PQDC_ERROR_MESSAGE);
                        memcpy(error_message, MEMC_PQDC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case MEMC_DMA_Error:
                        string_size = sizeof(MEMC_DMA_ERROR_MESSAGE);
                        memcpy(error_message, MEMC_DMA_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case I3DDMA_PQC_Error:
                        string_size = sizeof(I3DDMA_PQC_ERROR_MESSAGE);
                        memcpy(error_message, I3DDMA_PQC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case VO_I3ddma_PQDC_Error:
                        string_size = sizeof(VO_I3DDMA_PQDC_ERROR_MESSAGE);
                        memcpy(error_message, VO_I3DDMA_PQDC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case VO1_Vdec_DMA_error:
                        string_size = sizeof(VO1_VDEC_DMA_ERROR_MESSAGE);
                        memcpy(error_message, VO1_VDEC_DMA_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case VO1_Vdec_Decompress_error:
                        string_size = sizeof(VO1_VDEC_DECOMPRESS_ERROR_MESSAGE);
                        memcpy(error_message, VO1_VDEC_DECOMPRESS_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case VO2_Vdec_DMA_error:
                        string_size = sizeof(VO2_VDEC_DMA_ERROR_MESSAGE);
                        memcpy(error_message, VO2_VDEC_DMA_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case VO2_Vdec_Decompress_error:
                        string_size = sizeof(VO2_VDEC_DECOMPRESS_ERROR_MESSAGE);
                        memcpy(error_message, VO2_VDEC_DECOMPRESS_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case DI_PQC_Error:
                        string_size = sizeof(DI_PQC_ERROR_MESSAGE);
                        memcpy(error_message, DI_PQC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case DI_PQDC_Error:
                        string_size = sizeof(DI_PQDC_ERROR_MESSAGE);
                        memcpy(error_message, DI_PQDC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case OD_DMA_Error:
                        string_size = sizeof(OD_DMA_ERROR_MESSAGE);
                        memcpy(error_message, OD_DMA_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case OD_PQC_Error:
                        string_size = sizeof(OD_PQC_ERROR_MESSAGE);
                        memcpy(error_message, OD_PQC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        case OD_PQDC_Error:
                        string_size = sizeof(OD_PQDC_ERROR_MESSAGE);
                        memcpy(error_message, OD_PQDC_ERROR_MESSAGE, string_size > ERROR_MESSAGE_MAX_STRING_SIZE ? ERROR_MESSAGE_MAX_STRING_SIZE : string_size);
                        break;

                        default:
                        break;
                    }
                }
                p_bsp_err_ring_buffer_addr->read_idx = Scaler_ChangeUINT32Endian(read_idx);
                spin_unlock_irqrestore(&BSP_ERROR_REPORT_SPINLOCK, flags);
                if(string_size)
                {
                    error_message[ERROR_MESSAGE_MAX_STRING_SIZE - 1] = 0;
                    if(outputstring)
                        memcpy(outputstring, error_message, ERROR_MESSAGE_MAX_STRING_SIZE);
                    rtd_pr_vbe_info("### [BSP ERROR] report:%s 90k:0x%x###\r\n", error_message, IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg));
                    return;
                }
            }
        }
    }
    decide_report_kernel_bsp_error(outputstring);//report kernel error
}

/********************************BSP Error Report end********************************************/
#endif

//dada access
#define BACKGROUND_DATA_ACCESS_LONG_TIMEOUT 6000 //6 second //backgrond after 5 sec 6 sec output 1 time
#define BACKGROUND_DATA_ACCESS_SHORT_TIMEOUT 500 //500ms  when mute off start 500ms output 1 time
#define BACKGROUND_DATA_ACCESS_USE_SHORT_TIMEOUT_TH 15 //OUTPUT_DATA_ACCESS_COUNT bigger than this will use BACKGROUND_DATA_ACCESS_SHORT_TIMEOUT
#define BACKGROUND_DATA_ACCESS_START_CONTER 25 //10 times 0.5 sec + 6 second 15 times//1.5 min
#define OUTPUT_DATA_ACCESS_COUNT 5 //wait 5 loop
unsigned char data_access_enable = FALSE;
unsigned char data_access_print_count = OUTPUT_DATA_ACCESS_COUNT;//around 100 ms
struct semaphore DATA_ACCESS_DEBUG_Semaphore;/*For data access Semaphore*/
unsigned int data_access_basetime = 0;//if 0 no enable
unsigned background_data_access_start_conter = BACKGROUND_DATA_ACCESS_START_CONTER;//1.5 min

unsigned char sub_data_access_enable = FALSE;
unsigned char sub_data_access_print_count = OUTPUT_DATA_ACCESS_COUNT;//around 100 ms
struct semaphore SUB_DATA_ACCESS_DEBUG_Semaphore;/*For data access Semaphore*/
unsigned int sub_data_access_basetime = 0;//if 0 no enable
unsigned sub_background_data_access_start_conter = BACKGROUND_DATA_ACCESS_START_CONTER;//1.5 min
#ifndef BUILD_QUICK_SHOW

void check_enable_data_access_background(void)
{//enable data access
	unsigned int timeout = 0;
	VSC_INPUT_TYPE_T webos_src;
	{//main
		webos_src = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
		if(PPOVERLAY_Main_Display_Control_RSV_get_m_force_bg(IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg)) ||
			(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE) || (webos_src == VSC_INPUTSRC_MAXN) || (data_access_basetime == 0))
		{//alreay disconenct or timing chnage mute on
			if(data_access_basetime)
				data_access_basetime = 0;//disable enable data access
		}
		else
		{
			if(background_data_access_start_conter)//0.5  or 6 second
			{
				if(background_data_access_start_conter >= BACKGROUND_DATA_ACCESS_USE_SHORT_TIMEOUT_TH)
					timeout = BACKGROUND_DATA_ACCESS_SHORT_TIMEOUT;//0.5 seccond
				else
					timeout = BACKGROUND_DATA_ACCESS_LONG_TIMEOUT;//6 seccond
				if(drvif_wait_timeout_check_by_counter(data_access_basetime, timeout))
				{
					enable_data_access(SLR_MAIN_DISPLAY, FALSE);//enable data access
					data_access_basetime = 0;//disbale
				}
			}
		}
	}

	{//sub
		webos_src = Get_DisplayMode_Src(SLR_SUB_DISPLAY);
		if(PPOVERLAY_MP_Layout_Force_to_Background_get_s_force_bg(IoReg_Read32(PPOVERLAY_MP_Layout_Force_to_Background_reg)) || (PPOVERLAY_MP_Layout_Force_to_Background_get_s_disp_en(IoReg_Read32(PPOVERLAY_MP_Layout_Force_to_Background_reg)) == 0) ||
			(Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE) || (webos_src == VSC_INPUTSRC_MAXN) || (sub_data_access_basetime == 0))
		{//alreay disconenct or timing chnage mute on
			if(sub_data_access_basetime)
				sub_data_access_basetime = 0;//disable enable data access
		}
		else
		{
			if(sub_background_data_access_start_conter)//0.5  or 6 second
			{
				if(sub_background_data_access_start_conter >= BACKGROUND_DATA_ACCESS_USE_SHORT_TIMEOUT_TH)
					timeout = BACKGROUND_DATA_ACCESS_SHORT_TIMEOUT;//0.5  seccond
				else
					timeout = BACKGROUND_DATA_ACCESS_LONG_TIMEOUT;//6 seccond
				if(drvif_wait_timeout_check_by_counter(sub_data_access_basetime, timeout))
				{
					enable_data_access(SLR_SUB_DISPLAY, FALSE);//enable data access
					sub_data_access_basetime = 0;//disbale
				}
			}
		}
	}
}

#endif

extern VIP_DRV_DataAccess_Debug vip_data_access_debug;
void enable_data_access(unsigned char display, unsigned char reset)
{
	unsigned int i_x, i_y, d_x, d_y, two_uzu_x,two_uzu_y, uzd_x, uzd_y;//i domain x,y and d domain x, y
	if(display == SLR_MAIN_DISPLAY)
	{//main
		h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_RBUS h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_reg;
		h3ddma_hsd_i3ddma_accessdata_posctrl_uzd_RBUS h3ddma_hsd_i3ddma_accessdata_posctrl_uzd_reg;

		mpegnr_ich1_accessdata_posctrl_pk_RBUS mpegnr_ich1_accessdata_posctrl_pk_reg;
		mpegnr_ich1_accessdata_ctrl_pk_RBUS mpegnr_ich1_accessdata_ctrl_pk_reg;

		scaledown_accessdata_ctrl_uzd_RBUS  scaledown_accessdata_ctrl_uzd_reg;
		scaledown_accessdata_posctrl_uzd_RBUS scaledown_accessdata_posctrl_uzd_reg;

		scaleup_accessdata_ctrl_uzu_RBUS scaleup_accessdata_ctrl_uzu_reg;
		scaleup_accessdata_posctrl_uzu_RBUS scaleup_accessdata_posctrl_uzu_reg;

		two_step_uzu_accessdata_posctrl_sr_RBUS two_step_uzu_accessdata_posctrl_sr_reg;
		two_step_uzu_accessdata_ctrl_sr_RBUS two_step_uzu_accessdata_ctrl_sr_reg;

		color_sharp_access_data_0_RBUS color_sharp_access_data_0_reg;
		color_sharp_access_data_1_RBUS color_sharp_access_data_1_reg;


		color_icm_dm_icm_accessdata_posctrl_RBUS color_icm_dm_icm_accessdata_posctrl_reg;
		color_icm_dm_icm_accessdata_ctrl_RBUS color_icm_dm_icm_accessdata_ctrl_reg;

		yuv2rgb_accessdata_ctrl_RBUS yuv2rgb_accessdata_ctrl_reg;
		yuv2rgb_accessdata_posctrl_RBUS yuv2rgb_accessdata_posctrl_reg;

		dm_hdr_access_data_ctrl0_RBUS dm_hdr_access_data_ctrl0_reg;
		dm_hdr_access_data_ctrl1_RBUS dm_hdr_access_data_ctrl1_reg;

		od_accessdata_ctrl_pc_RBUS od_accessdata_ctrl_pc_reg;
		od_accessdata_posctrl_pc_RBUS od_accessdata_posctrl_pc_reg;

		/*
		valc_valc_accessdata_ctrl_RBUS valc_valc_accessdata_ctrl_reg;
		valc_valc_accessdata_posctrl_RBUS valc_valc_accessdata_posctrl_reg;
		*/

		//color temp
		color_temp_coloraccessdata_ctrl_RBUS color_temp_coloraccessdata_ctrl_reg;
		color_temp_coloraccessdata_posctrl_RBUS color_temp_coloraccessdata_posctrl_reg;

		if (vip_data_access_debug.debug_en != 0) {	// vip_data_access_debug.debug_en is only enable by tool
			data_access_enable = FALSE;
			return;
		}

		down(&DATA_ACCESS_DEBUG_Semaphore);
		i_x = VGIP_VGIP_CHN1_ACT_HSTA_Width_get_ch1_ih_act_wid(IoReg_Read32(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg))/2;
		i_y = VGIP_VGIP_CHN1_ACT_VSTA_Length_get_ch1_iv_act_len(IoReg_Read32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg))/2;

		d_x = ((unsigned int)PPOVERLAY_MAIN_Active_H_Start_End_get_mh_act_end(IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg)) + (unsigned int)PPOVERLAY_MAIN_Active_H_Start_End_get_mh_act_sta(IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg)))/2;
		d_y = ((unsigned int)PPOVERLAY_MAIN_Active_V_Start_End_get_mv_act_end(IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg)) + (unsigned int)PPOVERLAY_MAIN_Active_V_Start_End_get_mv_act_sta(IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg)))/2;

		if(
			((unsigned int)PPOVERLAY_MAIN_Active_H_Start_End_get_mh_act_end(IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg)) >= (unsigned int)PPOVERLAY_MAIN_Active_H_Start_End_get_mh_act_sta(IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg))) &&
			((unsigned int)PPOVERLAY_MAIN_Active_V_Start_End_get_mv_act_end(IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg)) >= (unsigned int)PPOVERLAY_MAIN_Active_V_Start_End_get_mv_act_sta(IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg)))
		)
		{
			two_uzu_x = ((unsigned int)PPOVERLAY_MAIN_Active_H_Start_End_get_mh_act_end(IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg)) - (unsigned int)PPOVERLAY_MAIN_Active_H_Start_End_get_mh_act_sta(IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg)))/2;
			two_uzu_y = ((unsigned int)PPOVERLAY_MAIN_Active_V_Start_End_get_mv_act_end(IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg)) - (unsigned int)PPOVERLAY_MAIN_Active_V_Start_End_get_mv_act_sta(IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg)))/2;
		}
		else
		{
			two_uzu_x = i_x;
			two_uzu_y = i_y;
		}

		uzd_x = SCALEUP_DM_UZU_Input_Size_get_hor_input_size(IoReg_Read32(SCALEUP_DM_UZU_Input_Size_reg))/2;
		uzd_y = SCALEUP_DM_UZU_Input_Size_get_ver_input_size(IoReg_Read32(SCALEUP_DM_UZU_Input_Size_reg))/2;

		//disable data access
		//i3ddma
		h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_AccessData_CTRL_UZD_reg);
		h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_reg.read_en = 0;
		IoReg_Write32(H3DDMA_HSD_I3DDMA_AccessData_CTRL_UZD_reg, h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_reg.regValue);

		//peaking
		mpegnr_ich1_accessdata_ctrl_pk_reg.regValue = IoReg_Read32(MPEGNR_ICH1_AccessData_CTRL_PK_reg);
		mpegnr_ich1_accessdata_ctrl_pk_reg.read_en = 0;
		IoReg_Write32(MPEGNR_ICH1_AccessData_CTRL_PK_reg, mpegnr_ich1_accessdata_ctrl_pk_reg.regValue);

		//uzd
		scaledown_accessdata_ctrl_uzd_reg.regValue = IoReg_Read32(SCALEDOWN_AccessData_CTRL_UZD_reg);
		scaledown_accessdata_ctrl_uzd_reg.read_en = 0;
		IoReg_Write32(SCALEDOWN_AccessData_CTRL_UZD_reg, scaledown_accessdata_ctrl_uzd_reg.regValue);

		//uzu
		scaleup_accessdata_ctrl_uzu_reg.regValue = IoReg_Read32(SCALEUP_AccessData_CTRL_UZU_reg);
		scaleup_accessdata_ctrl_uzu_reg.read_en = 0;
		IoReg_Write32(SCALEUP_AccessData_CTRL_UZU_reg, scaleup_accessdata_ctrl_uzu_reg.regValue);

		//2step uzu
		two_step_uzu_accessdata_ctrl_sr_reg.regValue = IoReg_Read32(TWO_STEP_UZU_AccessData_CTRL_SR_reg);
		two_step_uzu_accessdata_ctrl_sr_reg.read_en = 0;
		IoReg_Write32(TWO_STEP_UZU_AccessData_CTRL_SR_reg, two_step_uzu_accessdata_ctrl_sr_reg.regValue);


		//color_sharp
		color_sharp_access_data_0_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_0_reg);
		color_sharp_access_data_0_reg.read_en = 0;
		IoReg_Write32(COLOR_SHARP_Access_Data_0_reg, color_sharp_access_data_0_reg.regValue);


		//ICM
		color_icm_dm_icm_accessdata_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_AccessData_CTRL_reg);
		color_icm_dm_icm_accessdata_ctrl_reg.accessdata_read_en = 0;
		IoReg_Write32(COLOR_ICM_DM_ICM_AccessData_CTRL_reg, color_icm_dm_icm_accessdata_ctrl_reg.regValue);


		//Y2R
		yuv2rgb_accessdata_ctrl_reg.regValue = IoReg_Read32(YUV2RGB_AccessData_CTRL_reg);
		yuv2rgb_accessdata_ctrl_reg.read_en = 0;
		IoReg_Write32(YUV2RGB_AccessData_CTRL_reg, yuv2rgb_accessdata_ctrl_reg.regValue);

		//HDR
		dm_hdr_access_data_ctrl0_reg.regValue = IoReg_Read32(DM_HDR_ACCESS_DATA_CTRL0_reg);
		dm_hdr_access_data_ctrl0_reg.accessdata_rd_en = 0;
		IoReg_Write32(DM_HDR_ACCESS_DATA_CTRL0_reg, dm_hdr_access_data_ctrl0_reg.regValue);

		//color temp
		color_temp_coloraccessdata_ctrl_reg.regValue = IoReg_Read32(COLOR_TEMP_ColorAccessData_CTRL_reg);
		color_temp_coloraccessdata_ctrl_reg.read_en = 0;
		IoReg_Write32(COLOR_TEMP_ColorAccessData_CTRL_reg, color_temp_coloraccessdata_ctrl_reg.regValue);
		
		//OD
		od_accessdata_ctrl_pc_reg.regValue = IoReg_Read32(OD_AccessData_CTRL_PC_reg);
		od_accessdata_ctrl_pc_reg.access_read_en = 0;
		IoReg_Write32(OD_AccessData_CTRL_PC_reg, od_accessdata_ctrl_pc_reg.regValue);

		/*
		//VALC
		valc_valc_accessdata_ctrl_reg.regValue = IoReg_Read32(VALC_VALC_AccessData_CTRL_reg);
		valc_valc_accessdata_ctrl_reg.access_read_en = 0;
		IoReg_Write32(VALC_VALC_AccessData_CTRL_reg, valc_valc_accessdata_ctrl_reg.regValue);
		*/

		//i3ddma
		h3ddma_hsd_i3ddma_accessdata_posctrl_uzd_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_AccessData_PosCtrl_UZD_reg);
		h3ddma_hsd_i3ddma_accessdata_posctrl_uzd_reg.startx = i_x;//read 250, 250 position
		h3ddma_hsd_i3ddma_accessdata_posctrl_uzd_reg.starty = i_y;
		IoReg_Write32(H3DDMA_HSD_I3DDMA_AccessData_PosCtrl_UZD_reg, h3ddma_hsd_i3ddma_accessdata_posctrl_uzd_reg.regValue);

		h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_AccessData_CTRL_UZD_reg);
		h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_reg.read_en = 1;//enabel i3ddma data access
		IoReg_Write32(H3DDMA_HSD_I3DDMA_AccessData_CTRL_UZD_reg, h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_reg.regValue);

		//HDR
		dm_hdr_access_data_ctrl1_reg.regValue = IoReg_Read32(DM_HDR_ACCESS_DATA_CTRL1_reg);
		dm_hdr_access_data_ctrl1_reg.accessdata_startx = i_x;//read 250, 250 position
		dm_hdr_access_data_ctrl1_reg.accessdata_starty = i_y;
		IoReg_Write32(DM_HDR_ACCESS_DATA_CTRL1_reg, dm_hdr_access_data_ctrl1_reg.regValue);

		dm_hdr_access_data_ctrl0_reg.regValue = IoReg_Read32(DM_HDR_ACCESS_DATA_CTRL0_reg);
		dm_hdr_access_data_ctrl0_reg.accessdata_rd_en = 1;
		dm_hdr_access_data_ctrl0_reg.access_data_sel = 2;
		IoReg_Write32(DM_HDR_ACCESS_DATA_CTRL0_reg, dm_hdr_access_data_ctrl0_reg.regValue);

		//peaking
		mpegnr_ich1_accessdata_posctrl_pk_reg.regValue = IoReg_Read32(MPEGNR_ICH1_AccessData_PosCtrl_PK_reg);
		mpegnr_ich1_accessdata_posctrl_pk_reg.startx = i_x;//read 250, 250 position
		mpegnr_ich1_accessdata_posctrl_pk_reg.starty = i_y;
		IoReg_Write32(MPEGNR_ICH1_AccessData_PosCtrl_PK_reg, mpegnr_ich1_accessdata_posctrl_pk_reg.regValue);

		mpegnr_ich1_accessdata_ctrl_pk_reg.regValue = IoReg_Read32(MPEGNR_ICH1_AccessData_CTRL_PK_reg);
		mpegnr_ich1_accessdata_ctrl_pk_reg.read_en = 1;
		IoReg_Write32(MPEGNR_ICH1_AccessData_CTRL_PK_reg, mpegnr_ich1_accessdata_ctrl_pk_reg.regValue);

		//uzd
		scaledown_accessdata_posctrl_uzd_reg.regValue = IoReg_Read32(SCALEDOWN_AccessData_PosCtrl_UZD_reg);
		scaledown_accessdata_posctrl_uzd_reg.startx = uzd_x;//read 250, 250 position
		scaledown_accessdata_posctrl_uzd_reg.starty = uzd_y;
		IoReg_Write32(SCALEDOWN_AccessData_PosCtrl_UZD_reg, scaledown_accessdata_posctrl_uzd_reg.regValue);

		scaledown_accessdata_ctrl_uzd_reg.regValue = IoReg_Read32(SCALEDOWN_AccessData_CTRL_UZD_reg);
		scaledown_accessdata_ctrl_uzd_reg.read_en = 1;
		IoReg_Write32(SCALEDOWN_AccessData_CTRL_UZD_reg, scaledown_accessdata_ctrl_uzd_reg.regValue);

		//uzu
		scaleup_accessdata_posctrl_uzu_reg.regValue = IoReg_Read32(SCALEUP_AccessData_PosCtrl_UZU_reg);
		scaleup_accessdata_posctrl_uzu_reg.startx = uzd_x;//read 250, 250 position
		scaleup_accessdata_posctrl_uzu_reg.starty = uzd_y;
		IoReg_Write32(SCALEUP_AccessData_PosCtrl_UZU_reg, scaleup_accessdata_posctrl_uzu_reg.regValue);

		scaleup_accessdata_ctrl_uzu_reg.regValue = IoReg_Read32(SCALEUP_AccessData_CTRL_UZU_reg);
		scaleup_accessdata_ctrl_uzu_reg.read_en = 1;
		IoReg_Write32(SCALEUP_AccessData_CTRL_UZU_reg, scaleup_accessdata_ctrl_uzu_reg.regValue);

		//2step uzu
		two_step_uzu_accessdata_posctrl_sr_reg.regValue = IoReg_Read32(TWO_STEP_UZU_AccessData_PosCtrl_SR_reg);
		two_step_uzu_accessdata_posctrl_sr_reg.startx = two_uzu_x;//read 250, 250 position
		two_step_uzu_accessdata_posctrl_sr_reg.starty = two_uzu_y;
		IoReg_Write32(TWO_STEP_UZU_AccessData_PosCtrl_SR_reg, two_step_uzu_accessdata_posctrl_sr_reg.regValue);

		two_step_uzu_accessdata_ctrl_sr_reg.regValue = IoReg_Read32(TWO_STEP_UZU_AccessData_CTRL_SR_reg);
		two_step_uzu_accessdata_ctrl_sr_reg.inout_sel = 1;//output
		two_step_uzu_accessdata_ctrl_sr_reg.read_en = 1;
		IoReg_Write32(TWO_STEP_UZU_AccessData_CTRL_SR_reg, two_step_uzu_accessdata_ctrl_sr_reg.regValue);

		//color_sharp
		color_sharp_access_data_1_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_1_reg);
		color_sharp_access_data_1_reg.startx = two_uzu_x;//read 250, 250 position
		color_sharp_access_data_1_reg.starty = two_uzu_y;
		IoReg_Write32(COLOR_SHARP_Access_Data_1_reg, color_sharp_access_data_1_reg.regValue);

		color_sharp_access_data_0_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_0_reg);
		color_sharp_access_data_0_reg.read_en = 1;
		IoReg_Write32(COLOR_SHARP_Access_Data_0_reg, color_sharp_access_data_0_reg.regValue);

		//ICM
		color_icm_dm_icm_accessdata_posctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_AccessData_PosCtrl_reg);
		color_icm_dm_icm_accessdata_posctrl_reg.accessdata_startx = two_uzu_x;//read 250, 250 position
		color_icm_dm_icm_accessdata_posctrl_reg.accessdata_starty= two_uzu_y;
		IoReg_Write32(COLOR_ICM_DM_ICM_AccessData_PosCtrl_reg, color_icm_dm_icm_accessdata_posctrl_reg.regValue);

		color_icm_dm_icm_accessdata_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_AccessData_CTRL_reg);
		color_icm_dm_icm_accessdata_ctrl_reg.accessdata_read_en = 1;
		IoReg_Write32(COLOR_ICM_DM_ICM_AccessData_CTRL_reg, color_icm_dm_icm_accessdata_ctrl_reg.regValue);

		//Y2R
		yuv2rgb_accessdata_posctrl_reg.regValue = IoReg_Read32(YUV2RGB_AccessData_PosCtrl_reg);
		yuv2rgb_accessdata_posctrl_reg.startx = d_x;//read 250, 250 position
		yuv2rgb_accessdata_posctrl_reg.starty = d_y;
		IoReg_Write32(YUV2RGB_AccessData_PosCtrl_reg, yuv2rgb_accessdata_posctrl_reg.regValue);

		yuv2rgb_accessdata_ctrl_reg.regValue = IoReg_Read32(YUV2RGB_AccessData_CTRL_reg);
		yuv2rgb_accessdata_ctrl_reg.read_en = 1;
		IoReg_Write32(YUV2RGB_AccessData_CTRL_reg, yuv2rgb_accessdata_ctrl_reg.regValue);


		//color temp
		color_temp_coloraccessdata_posctrl_reg.regValue = IoReg_Read32(COLOR_TEMP_ColorAccessData_PosCtrl_reg);
		color_temp_coloraccessdata_posctrl_reg.startx = d_x;
		color_temp_coloraccessdata_posctrl_reg.starty = d_y;
		IoReg_Write32(COLOR_TEMP_ColorAccessData_PosCtrl_reg, color_temp_coloraccessdata_posctrl_reg.regValue);
		
		color_temp_coloraccessdata_ctrl_reg.regValue = IoReg_Read32(COLOR_TEMP_ColorAccessData_CTRL_reg);
		color_temp_coloraccessdata_ctrl_reg.read_en = 1;
		IoReg_Write32(COLOR_TEMP_ColorAccessData_CTRL_reg, color_temp_coloraccessdata_ctrl_reg.regValue);
		

		//OD
		od_accessdata_posctrl_pc_reg.regValue = IoReg_Read32(OD_AccessData_PosCtrl_PC_reg);
		od_accessdata_posctrl_pc_reg.access_startx = d_x;
		od_accessdata_posctrl_pc_reg.access_starty = d_y;
		IoReg_Write32(OD_AccessData_PosCtrl_PC_reg, od_accessdata_posctrl_pc_reg.regValue);
		
		od_accessdata_ctrl_pc_reg.regValue = IoReg_Read32(OD_AccessData_CTRL_PC_reg);
		od_accessdata_ctrl_pc_reg.access_read_en = 1;
		IoReg_Write32(OD_AccessData_CTRL_PC_reg, od_accessdata_ctrl_pc_reg.regValue);

		/*
		//VALC
		valc_valc_accessdata_posctrl_reg.regValue = IoReg_Read32(VALC_VALC_AccessData_PosCtrl_reg);
		valc_valc_accessdata_posctrl_reg.access_startx = d_x;
		valc_valc_accessdata_posctrl_reg.access_starty = d_y;
		IoReg_Write32(VALC_VALC_AccessData_PosCtrl_reg, valc_valc_accessdata_posctrl_reg.regValue);
		
		valc_valc_accessdata_ctrl_reg.regValue = IoReg_Read32(VALC_VALC_AccessData_CTRL_reg);
		valc_valc_accessdata_ctrl_reg.access_read_en = 1;
		IoReg_Write32(VALC_VALC_AccessData_CTRL_reg, valc_valc_accessdata_ctrl_reg.regValue);
		*/

		data_access_enable = TRUE;
		data_access_print_count = OUTPUT_DATA_ACCESS_COUNT;//around 50 ms
		if(reset)
			background_data_access_start_conter = BACKGROUND_DATA_ACCESS_START_CONTER;//reset after mute off. 
		else if(background_data_access_start_conter)
			background_data_access_start_conter--;//count down 15 times
		up(&DATA_ACCESS_DEBUG_Semaphore);
	}
#ifdef CONFIG_DUAL_CHANNEL     
	else
	{//sub
		//sub dither
		sub_dither_i_sub_dither_accessdata_ctrl_RBUS sub_dither_i_sub_dither_accessdata_ctrl_reg;
		sub_dither_i_sub_dither_accessdata_posctrl_RBUS sub_dither_i_sub_dither_accessdata_posctrl_reg;		
		
		if (vip_data_access_debug.debug_en != 0) {	// vip_data_access_debug.debug_en is only enable by tool
			sub_data_access_enable = FALSE;
			return;
		}

		down(&SUB_DATA_ACCESS_DEBUG_Semaphore);
		i_x = SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_get_ch2_ih_act_wid(IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg))/2;
		i_y = SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_get_ch2_iv_act_len(IoReg_Read32(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg))/2;
		
		d_x = ((unsigned int)PPOVERLAY_SUB_Active_H_Start_End_get_sh_act_end(IoReg_Read32(PPOVERLAY_SUB_Active_H_Start_End_reg)) + (unsigned int)PPOVERLAY_SUB_Active_H_Start_End_get_sh_act_sta(IoReg_Read32(PPOVERLAY_SUB_Active_H_Start_End_reg)))/2;
		d_y = ((unsigned int)PPOVERLAY_SUB_Active_V_Start_End_get_sv_act_end(IoReg_Read32(PPOVERLAY_SUB_Active_V_Start_End_reg)) + (unsigned int)PPOVERLAY_SUB_Active_V_Start_End_get_sv_act_sta(IoReg_Read32(PPOVERLAY_SUB_Active_V_Start_End_reg)))/2;

		//disable data access
		//sub dither
		sub_dither_i_sub_dither_accessdata_ctrl_reg.regValue = IoReg_Read32(SUB_DITHER_I_Sub_DITHER_AccessData_CTRL_reg);
		sub_dither_i_sub_dither_accessdata_ctrl_reg.read_en = 0;
		IoReg_Write32(SUB_DITHER_I_Sub_DITHER_AccessData_CTRL_reg, sub_dither_i_sub_dither_accessdata_ctrl_reg.regValue);

		

		//sub dither
		sub_dither_i_sub_dither_accessdata_posctrl_reg.regValue = IoReg_Read32(SUB_DITHER_I_Sub_DITHER_AccessData_PosCtrl_reg);
		sub_dither_i_sub_dither_accessdata_posctrl_reg.startx = i_x;
		sub_dither_i_sub_dither_accessdata_posctrl_reg.starty = i_y;
		IoReg_Write32(SUB_DITHER_I_Sub_DITHER_AccessData_PosCtrl_reg, sub_dither_i_sub_dither_accessdata_posctrl_reg.regValue);

		sub_dither_i_sub_dither_accessdata_ctrl_reg.regValue = IoReg_Read32(SUB_DITHER_I_Sub_DITHER_AccessData_CTRL_reg);
		sub_dither_i_sub_dither_accessdata_ctrl_reg.read_en = 1;
		IoReg_Write32(SUB_DITHER_I_Sub_DITHER_AccessData_CTRL_reg, sub_dither_i_sub_dither_accessdata_ctrl_reg.regValue);
		
		sub_data_access_enable = TRUE;
		sub_data_access_print_count = OUTPUT_DATA_ACCESS_COUNT;//around 50 ms
		if(reset)
			sub_background_data_access_start_conter = BACKGROUND_DATA_ACCESS_START_CONTER;//reset after mute off. 
		else if(sub_background_data_access_start_conter)
			sub_background_data_access_start_conter--;//count down 15 times
		up(&SUB_DATA_ACCESS_DEBUG_Semaphore);
	}
#endif   
}
int qms_driver_enable = 1;

#ifndef BUILD_QUICK_SHOW 

void output_data_access_data(void)
{
	VSC_INPUT_TYPE_T webos_src;

	{//main
		//i3ddma
		h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_RBUS h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_reg;
		h3ddma_hsd_i3ddma_readdata_data_y1_uzd_RBUS h3ddma_hsd_i3ddma_readdata_data_y1_uzd_reg;
		h3ddma_hsd_i3ddma_readdata_data_c1_uzd_RBUS h3ddma_hsd_i3ddma_readdata_data_c1_uzd_reg;
		h3ddma_hsd_i3ddma_accessdata_posctrl_uzd_RBUS h3ddma_hsd_i3ddma_accessdata_posctrl_uzd_reg;

		//hdr
		dm_hdr_access_data_ctrl0_RBUS dm_hdr_access_data_ctrl0_reg;
		dm_hdr_access_data_status0_RBUS dm_hdr_access_data_status0_reg;
		dm_hdr_access_data_status4_RBUS dm_hdr_access_data_status4_reg;
		dm_hdr_access_data_status8_RBUS dm_hdr_access_data_status8_reg;
		dm_hdr_access_data_ctrl1_RBUS dm_hdr_access_data_ctrl1_reg;

		//peaking
		mpegnr_ich1_accessdata_ctrl_pk_RBUS mpegnr_ich1_accessdata_ctrl_pk_reg;
		mpegnr_ich1_readdata_data_y1_pk_RBUS mpegnr_ich1_readdata_data_y1_pk_reg;
		mpegnr_ich1_readdata_data_c1_pk_RBUS mpegnr_ich1_readdata_data_c1_pk_reg;
		mpegnr_ich1_accessdata_posctrl_pk_RBUS mpegnr_ich1_accessdata_posctrl_pk_reg;

		//uzd
		scaledown_accessdata_ctrl_uzd_RBUS  scaledown_accessdata_ctrl_uzd_reg;
		scaledown_readdata_data_y1_uzd_RBUS scaledown_readdata_data_y1_uzd_reg;
		scaledown_readdata_data_c1_uzd_RBUS scaledown_readdata_data_c1_uzd_reg;
		scaledown_accessdata_posctrl_uzd_RBUS scaledown_accessdata_posctrl_uzd_reg;

		//uzu
		scaleup_accessdata_ctrl_uzu_RBUS scaleup_accessdata_ctrl_uzu_reg;
		scaleup_readdata_data_y1_uzu_RBUS scaleup_readdata_data_y1_uzu_reg;
		scaleup_readdata_data_c1_uzu_RBUS scaleup_readdata_data_c1_uzu_reg;
		scaleup_accessdata_posctrl_uzu_RBUS scaleup_accessdata_posctrl_uzu_reg;

		//2_step uzu
		two_step_uzu_accessdata_ctrl_sr_RBUS two_step_uzu_accessdata_ctrl_sr_reg;
		two_step_uzu_readdata_data_y1_sr_RBUS two_step_uzu_readdata_data_y1_sr_reg;
		two_step_uzu_readdata_data_c1_sr_RBUS two_step_uzu_readdata_data_c1_sr_reg;
		two_step_uzu_accessdata_posctrl_sr_RBUS two_step_uzu_accessdata_posctrl_sr_reg;

		//sharp
		color_sharp_access_data_0_RBUS color_sharp_access_data_0_reg;
		color_sharp_access_data_2_RBUS color_sharp_access_data_2_reg;
		color_sharp_access_data_4_RBUS color_sharp_access_data_4_reg;
		color_sharp_access_data_1_RBUS color_sharp_access_data_1_reg;

		//icm
		color_icm_dm_icm_accessdata_ctrl_RBUS color_icm_dm_icm_accessdata_ctrl_reg;
		color_icm_dm_icm_readdata_data_h1_RBUS color_icm_dm_icm_readdata_data_h1_reg;
		color_icm_dm_icm_readdata_data_s1_RBUS color_icm_dm_icm_readdata_data_s1_reg;
		color_icm_dm_icm_readdata_data_i1_RBUS color_icm_dm_icm_readdata_data_i1_reg;
		color_icm_dm_icm_accessdata_posctrl_RBUS color_icm_dm_icm_accessdata_posctrl_reg;

		//yuv2rgb
		yuv2rgb_accessdata_ctrl_RBUS yuv2rgb_accessdata_ctrl_reg;
		yuv2rgb_readdata_data_y1_RBUS yuv2rgb_readdata_data_y1_reg;
		yuv2rgb_readdata_data_c1_RBUS yuv2rgb_readdata_data_c1_reg;
		yuv2rgb_accessdata_posctrl_RBUS yuv2rgb_accessdata_posctrl_reg;


		//color temp
		color_temp_coloraccessdata_ctrl_RBUS color_temp_coloraccessdata_ctrl_reg;
		color_temp_coloraccessdata_posctrl_RBUS color_temp_coloraccessdata_posctrl_reg;
		color_temp_colorreaddata_data_y1_RBUS color_temp_colorreaddata_data_y1_reg;
		color_temp_colorreaddata_data_c1_RBUS color_temp_colorreaddata_data_c1_reg;
		
		//od
		od_accessdata_ctrl_pc_RBUS od_accessdata_ctrl_pc_reg;
		od_accessdata_posctrl_pc_RBUS od_accessdata_posctrl_pc_reg;
		od_readdata_data_channel1_1_pc_RBUS od_readdata_data_channel1_1_pc_reg;
		od_readdata_data_channel2_1_pc_RBUS od_readdata_data_channel2_1_pc_reg;
		od_readdata_data_channel3_1_pc_RBUS od_readdata_data_channel3_1_pc_reg;

		/*
		//valc
		valc_valc_accessdata_ctrl_RBUS valc_valc_accessdata_ctrl_reg;
		valc_valc_accessdata_posctrl_RBUS valc_valc_accessdata_posctrl_reg;
		valc_valc_accessdata_r_RBUS valc_valc_accessdata_r_reg;
		valc_valc_accessdata_g_RBUS valc_valc_accessdata_g_reg;
		valc_valc_accessdata_b_RBUS valc_valc_accessdata_b_reg;
		*/
		
		

		if(data_access_enable)
		{
			down(&DATA_ACCESS_DEBUG_Semaphore);
			if(data_access_enable)
			{
				webos_src = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
				if(PPOVERLAY_Main_Display_Control_RSV_get_m_force_bg(IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg)) ||
					(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE) || (webos_src == VSC_INPUTSRC_MAXN))
				{
					data_access_enable = FALSE;
					data_access_print_count = 0;
					data_access_basetime = 0;//disbale backdround data access
				}
				else if(data_access_print_count == 0)//count down
				{//print
					data_access_enable = FALSE;
					data_access_basetime = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
					if(data_access_basetime == 0)
						data_access_basetime = 1;//because 0 is disable
					if((webos_src == VSC_INPUTSRC_HDMI) || (webos_src == VSC_INPUTSRC_ADC) || (webos_src == VSC_INPUTSRC_AVD))
					{
						//i3ddma
						h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_AccessData_CTRL_UZD_reg);
						if(h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_reg.read_en)
						{
							h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_reg.read_en = 0;
							IoReg_Write32(H3DDMA_HSD_I3DDMA_AccessData_CTRL_UZD_reg, h3ddma_hsd_i3ddma_accessdata_ctrl_uzd_reg.regValue);
							rtd_pr_vsc_info("i3ddma access data not ready \n");
						}
						else
						{//output i3ddma data
							h3ddma_hsd_i3ddma_accessdata_posctrl_uzd_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_AccessData_PosCtrl_UZD_reg);
							h3ddma_hsd_i3ddma_readdata_data_y1_uzd_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_ReadData_DATA_Y1_UZD_reg);
							h3ddma_hsd_i3ddma_readdata_data_c1_uzd_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_ReadData_DATA_C1_UZD_reg);
							rtd_pr_vsc_info("i3ddma access data XY(%d %d) YUV(%d %d %d) \n", h3ddma_hsd_i3ddma_accessdata_posctrl_uzd_reg.startx,
								h3ddma_hsd_i3ddma_accessdata_posctrl_uzd_reg.starty, h3ddma_hsd_i3ddma_readdata_data_y1_uzd_reg.y00,
								h3ddma_hsd_i3ddma_readdata_data_c1_uzd_reg.u00, h3ddma_hsd_i3ddma_readdata_data_c1_uzd_reg.v00);
						}
					}

					//HDR
					dm_hdr_access_data_ctrl0_reg.regValue = IoReg_Read32(DM_HDR_ACCESS_DATA_CTRL0_reg);
					if(dm_hdr_access_data_ctrl0_reg.accessdata_rd_en)
					{
						dm_hdr_access_data_ctrl0_reg.accessdata_rd_en = 0;
						IoReg_Write32(DM_HDR_ACCESS_DATA_CTRL0_reg, dm_hdr_access_data_ctrl0_reg.regValue);
						rtd_pr_vsc_info("HDR access data not ready \n");
					}
					else
					{//output HDR data
						dm_hdr_access_data_ctrl1_reg.regValue = IoReg_Read32(DM_HDR_ACCESS_DATA_CTRL1_reg);
						dm_hdr_access_data_status0_reg.regValue = IoReg_Read32(DM_HDR_ACCESS_DATA_STATUS0_reg);
						dm_hdr_access_data_status4_reg.regValue = IoReg_Read32(DM_HDR_ACCESS_DATA_STATUS4_reg);
						dm_hdr_access_data_status8_reg.regValue = IoReg_Read32(DM_HDR_ACCESS_DATA_STATUS8_reg);
						rtd_pr_vsc_info("HDR sel=2 access data XY(%d %d) RGB(%d %d %d) \n", dm_hdr_access_data_ctrl1_reg.accessdata_startx, dm_hdr_access_data_ctrl1_reg.accessdata_starty,
						(dm_hdr_access_data_status0_reg.access_rdata_r00 & 0xFFF)>>2,
						(dm_hdr_access_data_status4_reg.access_rdata_g00 & 0xFFF)>>2, (dm_hdr_access_data_status8_reg.access_rdata_b00 & 0xFFF)>>2);
					}

					//peaking
					mpegnr_ich1_accessdata_ctrl_pk_reg.regValue = IoReg_Read32(MPEGNR_ICH1_AccessData_CTRL_PK_reg);
					if(mpegnr_ich1_accessdata_ctrl_pk_reg.read_en)
					{
						mpegnr_ich1_accessdata_ctrl_pk_reg.read_en = 0;
						IoReg_Write32(MPEGNR_ICH1_AccessData_CTRL_PK_reg, mpegnr_ich1_accessdata_ctrl_pk_reg.regValue);
						rtd_pr_vsc_info("peaking access data not ready \n");
					}
					else
					{//output peaking data
						mpegnr_ich1_accessdata_posctrl_pk_reg.regValue = IoReg_Read32(MPEGNR_ICH1_AccessData_PosCtrl_PK_reg);
						mpegnr_ich1_readdata_data_y1_pk_reg.regValue = IoReg_Read32(MPEGNR_ICH1_ReadData_DATA_Y1_PK_reg);
		                mpegnr_ich1_readdata_data_c1_pk_reg.regValue = IoReg_Read32(MPEGNR_ICH1_ReadData_DATA_C1_PK_reg);
						rtd_pr_vsc_info("peaking access data XY(%d %d) YUV(%d %d %d) \n", mpegnr_ich1_accessdata_posctrl_pk_reg.startx, mpegnr_ich1_accessdata_posctrl_pk_reg.starty, mpegnr_ich1_readdata_data_y1_pk_reg.y00,
							mpegnr_ich1_readdata_data_c1_pk_reg.u00, mpegnr_ich1_readdata_data_c1_pk_reg.v00);
					}

					//uzd
					scaledown_accessdata_ctrl_uzd_reg.regValue = IoReg_Read32(SCALEDOWN_AccessData_CTRL_UZD_reg);
					if(scaledown_accessdata_ctrl_uzd_reg.read_en)
					{
						scaledown_accessdata_ctrl_uzd_reg.read_en = 0;
						IoReg_Write32(SCALEDOWN_AccessData_CTRL_UZD_reg, scaledown_accessdata_ctrl_uzd_reg.regValue);
						rtd_pr_vsc_info("uzd access data not ready \n");
					}
					else
					{//output uzd data
						scaledown_accessdata_posctrl_uzd_reg.regValue = IoReg_Read32(SCALEDOWN_AccessData_PosCtrl_UZD_reg);
						scaledown_readdata_data_y1_uzd_reg.regValue = IoReg_Read32(SCALEDOWN_ReadData_DATA_Y1_UZD_reg);
						scaledown_readdata_data_c1_uzd_reg.regValue = IoReg_Read32(SCALEDOWN_ReadData_DATA_C1_UZD_reg);
						rtd_pr_vsc_info("uzd access data XY(%d %d) YUV(%d %d %d) \n", scaledown_accessdata_posctrl_uzd_reg.startx, scaledown_accessdata_posctrl_uzd_reg.starty, scaledown_readdata_data_y1_uzd_reg.y00,
							scaledown_readdata_data_c1_uzd_reg.u00, scaledown_readdata_data_c1_uzd_reg.v00);
					}

					//uzu
					scaleup_accessdata_ctrl_uzu_reg.regValue = IoReg_Read32(SCALEUP_AccessData_CTRL_UZU_reg);
					if(scaleup_accessdata_ctrl_uzu_reg.read_en)
					{
						scaleup_accessdata_ctrl_uzu_reg.read_en = 0;
						IoReg_Write32(SCALEUP_AccessData_CTRL_UZU_reg, scaleup_accessdata_ctrl_uzu_reg.regValue);
						rtd_pr_vsc_info("uzu access data not ready \n");
					}
					else
					{//output uzu data
						scaleup_accessdata_posctrl_uzu_reg.regValue = IoReg_Read32(SCALEUP_AccessData_PosCtrl_UZU_reg);
						scaleup_readdata_data_y1_uzu_reg.regValue = IoReg_Read32(SCALEUP_ReadData_DATA_Y1_UZU_reg);
						scaleup_readdata_data_c1_uzu_reg.regValue = IoReg_Read32(SCALEUP_ReadData_DATA_C1_UZU_reg);

						rtd_pr_vsc_info("uzu access data XY(%d %d) YUV(%d %d %d) \n", scaleup_accessdata_posctrl_uzu_reg.startx, scaleup_accessdata_posctrl_uzu_reg.starty, scaleup_readdata_data_y1_uzu_reg.y00,
							scaleup_readdata_data_c1_uzu_reg.u00, scaleup_readdata_data_c1_uzu_reg.v00);
					}


					//2step uzu
					two_step_uzu_accessdata_ctrl_sr_reg.regValue = IoReg_Read32(TWO_STEP_UZU_AccessData_CTRL_SR_reg);
					if(two_step_uzu_accessdata_ctrl_sr_reg.read_en)
					{
						two_step_uzu_accessdata_ctrl_sr_reg.read_en = 0;
						IoReg_Write32(TWO_STEP_UZU_AccessData_CTRL_SR_reg, two_step_uzu_accessdata_ctrl_sr_reg.regValue);
						rtd_pr_vsc_info("2step uzu access data not ready \n");
					}
					else
					{//output 2step uzu data
						two_step_uzu_accessdata_posctrl_sr_reg.regValue = IoReg_Read32(TWO_STEP_UZU_AccessData_PosCtrl_SR_reg);
						two_step_uzu_readdata_data_y1_sr_reg.regValue = IoReg_Read32(TWO_STEP_UZU_ReadData_DATA_Y1_SR_reg);
		                two_step_uzu_readdata_data_c1_sr_reg.regValue = IoReg_Read32(TWO_STEP_UZU_ReadData_DATA_C1_SR_reg);
						rtd_pr_vsc_info("2step uzu access data XY(%d %d) YUV(%d %d %d) \n", two_step_uzu_accessdata_posctrl_sr_reg.startx, two_step_uzu_accessdata_posctrl_sr_reg.starty, two_step_uzu_readdata_data_y1_sr_reg.y00,
						two_step_uzu_readdata_data_c1_sr_reg.u00, two_step_uzu_readdata_data_c1_sr_reg.v00);

					}

					//color_sharp
					color_sharp_access_data_0_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_0_reg);
					if(color_sharp_access_data_0_reg.read_en)
					{
						color_sharp_access_data_0_reg.read_en = 0;
						IoReg_Write32(COLOR_SHARP_Access_Data_0_reg, color_sharp_access_data_0_reg.regValue);
						rtd_pr_vsc_info("sharp access data not ready \n");
					}
					else
					{//output color_sharp data
						color_sharp_access_data_1_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_1_reg);
						color_sharp_access_data_2_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_2_reg);
						color_sharp_access_data_4_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_4_reg);
						rtd_pr_vsc_info("sharp access data XY(%d %d) YUV(%d %d %d) \n", color_sharp_access_data_1_reg.startx, color_sharp_access_data_1_reg.starty, color_sharp_access_data_2_reg.y00,
						color_sharp_access_data_4_reg.u00, color_sharp_access_data_4_reg.v00);
					}


					//ICM
					color_icm_dm_icm_accessdata_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_AccessData_CTRL_reg);
					if(color_icm_dm_icm_accessdata_ctrl_reg.accessdata_read_en)
					{
						color_icm_dm_icm_accessdata_ctrl_reg.accessdata_read_en = 0;
						IoReg_Write32(COLOR_ICM_DM_ICM_AccessData_CTRL_reg, color_icm_dm_icm_accessdata_ctrl_reg.regValue);
						rtd_pr_vsc_info("ICM access data not ready \n");
					}
					else
					{//output ICM data
						color_icm_dm_icm_accessdata_posctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_AccessData_PosCtrl_reg);
						color_icm_dm_icm_readdata_data_h1_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_ReadData_DATA_H1_reg);
						color_icm_dm_icm_readdata_data_s1_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_ReadData_DATA_S1_reg);
						color_icm_dm_icm_readdata_data_i1_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_ReadData_DATA_I1_reg);
						rtd_pr_vsc_info("ICM access data XY(%d %d) HSI(%d %d %d) \n", color_icm_dm_icm_accessdata_posctrl_reg.accessdata_startx, color_icm_dm_icm_accessdata_posctrl_reg.accessdata_starty, 
						color_icm_dm_icm_readdata_data_h1_reg.readdata_h00,
						color_icm_dm_icm_readdata_data_s1_reg.readdata_s00, color_icm_dm_icm_readdata_data_i1_reg.readdata_i00);
					}

					//Y2R
					yuv2rgb_accessdata_ctrl_reg.regValue = IoReg_Read32(YUV2RGB_AccessData_CTRL_reg);
					if(yuv2rgb_accessdata_ctrl_reg.read_en)
					{
						yuv2rgb_accessdata_ctrl_reg.read_en = 0;
						IoReg_Write32(YUV2RGB_AccessData_CTRL_reg, yuv2rgb_accessdata_ctrl_reg.regValue);
						rtd_pr_vsc_info("Y2R access data not ready \n");
					}
					else
					{//output Y2R data
						yuv2rgb_accessdata_posctrl_reg.regValue = IoReg_Read32(YUV2RGB_AccessData_PosCtrl_reg);
						yuv2rgb_readdata_data_y1_reg.regValue = IoReg_Read32(YUV2RGB_ReadData_DATA_Y1_reg);
						yuv2rgb_readdata_data_c1_reg.regValue = IoReg_Read32(YUV2RGB_ReadData_DATA_C1_reg);
						rtd_pr_vsc_info("Y2R access data XY(%d %d) RGB(%d %d %d) \n", yuv2rgb_accessdata_posctrl_reg.startx, yuv2rgb_accessdata_posctrl_reg.starty, yuv2rgb_readdata_data_y1_reg.y00>>4,
						yuv2rgb_readdata_data_c1_reg.u00>>4, yuv2rgb_readdata_data_c1_reg.v00>>4);
					}

					//color temp
					color_temp_coloraccessdata_ctrl_reg.regValue = IoReg_Read32(COLOR_TEMP_ColorAccessData_CTRL_reg);
					if(color_temp_coloraccessdata_ctrl_reg.read_en)
					{
						color_temp_coloraccessdata_ctrl_reg.read_en = 0;
						IoReg_Write32(COLOR_TEMP_ColorAccessData_CTRL_reg, color_temp_coloraccessdata_ctrl_reg.regValue);
						rtd_pr_vsc_info("color temp access data not ready \n");
					}
					else
					{//output color temp data
						color_temp_coloraccessdata_posctrl_reg.regValue = IoReg_Read32(COLOR_TEMP_ColorAccessData_PosCtrl_reg);
						color_temp_colorreaddata_data_y1_reg.regValue = IoReg_Read32(COLOR_TEMP_ColorReadData_DATA_Y1_reg);
						color_temp_colorreaddata_data_c1_reg.regValue = IoReg_Read32(COLOR_TEMP_ColorReadData_DATA_C1_reg);
						rtd_pr_vsc_info("color temp access data XY(%d %d) RGB(%d %d %d) \n", color_temp_coloraccessdata_posctrl_reg.startx, color_temp_coloraccessdata_posctrl_reg.starty, color_temp_colorreaddata_data_y1_reg.r00>>2,
						color_temp_colorreaddata_data_c1_reg.g00 >>2, color_temp_colorreaddata_data_c1_reg.b00 >>2);
					}

					//od
					od_accessdata_ctrl_pc_reg.regValue = IoReg_Read32(OD_AccessData_CTRL_PC_reg);
					if(od_accessdata_ctrl_pc_reg.access_read_en)
					{
						od_accessdata_ctrl_pc_reg.access_read_en = 0;
						IoReg_Write32(OD_AccessData_CTRL_PC_reg, od_accessdata_ctrl_pc_reg.regValue);
						rtd_pr_vsc_info("OD access data not ready \n");
					}
					else
					{//output OD data
						od_accessdata_posctrl_pc_reg.regValue = IoReg_Read32(OD_AccessData_PosCtrl_PC_reg);
						od_readdata_data_channel1_1_pc_reg.regValue = IoReg_Read32(OD_ReadData_DATA_Channel1_1_PC_reg);
						od_readdata_data_channel2_1_pc_reg.regValue = IoReg_Read32(OD_ReadData_DATA_Channel2_1_PC_reg);
						od_readdata_data_channel3_1_pc_reg.regValue = IoReg_Read32(OD_ReadData_DATA_Channel3_1_PC_reg);
						rtd_pr_vsc_info("OD access data XY(%d %d) RGB(%d %d %d) \n", od_accessdata_posctrl_pc_reg.access_startx, od_accessdata_posctrl_pc_reg.access_starty, 
							od_readdata_data_channel1_1_pc_reg.access_y00 >> 2, od_readdata_data_channel2_1_pc_reg.access_u00 >> 2, od_readdata_data_channel3_1_pc_reg.access_v00 >> 2);
					}

					/*
					//valc
					valc_valc_accessdata_ctrl_reg.regValue = IoReg_Read32(VALC_VALC_AccessData_CTRL_reg);
					if(od_accessdata_ctrl_pc_reg.access_read_en)
					{
						od_accessdata_ctrl_pc_reg.access_read_en = 0;
						IoReg_Write32(VALC_VALC_AccessData_CTRL_reg, valc_valc_accessdata_ctrl_reg.regValue);
						rtd_pr_vsc_info("VALC access data not ready \n");
					}
					else
					{//output valc data
						valc_valc_accessdata_posctrl_reg.regValue = IoReg_Read32(VALC_VALC_AccessData_PosCtrl_reg);
						valc_valc_accessdata_r_reg.regValue = IoReg_Read32(VALC_VALC_AccessData_R_reg);
						valc_valc_accessdata_g_reg.regValue = IoReg_Read32(VALC_VALC_AccessData_G_reg);
						valc_valc_accessdata_b_reg.regValue = IoReg_Read32(VALC_VALC_AccessData_B_reg);

						rtd_pr_vsc_info("VALC access data XY(%d %d) RGB(%d %d %d) \n", valc_valc_accessdata_posctrl_reg.access_startx, valc_valc_accessdata_posctrl_reg.access_starty, 
							valc_valc_accessdata_r_reg.access_read_r >> 2, valc_valc_accessdata_g_reg.access_read_g >> 2, valc_valc_accessdata_b_reg.access_read_b >> 2);
					}
					*/
				}
				else
				{
					data_access_print_count --;
				}
			}
			up(&DATA_ACCESS_DEBUG_Semaphore);
		}
	}

	{//sub
		//Sub dither
		sub_dither_i_sub_dither_accessdata_ctrl_RBUS sub_dither_i_sub_dither_accessdata_ctrl_reg;
		sub_dither_i_sub_dither_accessdata_posctrl_RBUS sub_dither_i_sub_dither_accessdata_posctrl_reg;		
		sub_dither_i_sub_dither_readdata_data_y1_RBUS sub_dither_i_sub_dither_readdata_data_y1_reg;
		sub_dither_i_sub_dither_readdata_data_c1_RBUS sub_dither_i_sub_dither_readdata_data_c1_reg;
			
		if(sub_data_access_enable)
		{
			down(&SUB_DATA_ACCESS_DEBUG_Semaphore);
			if(sub_data_access_enable)
			{
				webos_src = Get_DisplayMode_Src(SLR_SUB_DISPLAY);

				if(PPOVERLAY_MP_Layout_Force_to_Background_get_s_force_bg(IoReg_Read32(PPOVERLAY_MP_Layout_Force_to_Background_reg)) || (PPOVERLAY_MP_Layout_Force_to_Background_get_s_disp_en(IoReg_Read32(PPOVERLAY_MP_Layout_Force_to_Background_reg)) == 0) ||
					(Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE) || (webos_src == VSC_INPUTSRC_MAXN))
				{//already change soruce or mute 
					sub_data_access_enable = FALSE;
					sub_data_access_print_count = 0;
					sub_data_access_basetime = 0;//disbale backdround data access
				}
				else if(sub_data_access_print_count == 0)//count down
				{//print
					sub_data_access_enable = FALSE;
					sub_data_access_basetime = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
					if(sub_data_access_basetime == 0)
						sub_data_access_basetime = 1;//because 0 is disable

					//Sub dither
					sub_dither_i_sub_dither_accessdata_ctrl_reg.regValue = IoReg_Read32(SUB_DITHER_I_Sub_DITHER_AccessData_CTRL_reg);
					if(sub_dither_i_sub_dither_accessdata_ctrl_reg.read_en)
					{
						sub_dither_i_sub_dither_accessdata_ctrl_reg.read_en = 0;
						IoReg_Write32(SUB_DITHER_I_Sub_DITHER_AccessData_CTRL_reg, sub_dither_i_sub_dither_accessdata_ctrl_reg.regValue);
						rtd_pr_vsc_info("[SUB]Sub dither access data not ready \n");
					}
					else
					{//output Sub dither data
						sub_dither_i_sub_dither_accessdata_posctrl_reg.regValue = IoReg_Read32(SUB_DITHER_I_Sub_DITHER_AccessData_PosCtrl_reg);
	
						sub_dither_i_sub_dither_readdata_data_y1_reg.regValue = IoReg_Read32(SUB_DITHER_I_Sub_DITHER_ReadData_DATA_Y1_reg);
						sub_dither_i_sub_dither_readdata_data_c1_reg.regValue = IoReg_Read32(SUB_DITHER_I_Sub_DITHER_ReadData_DATA_C1_reg);

						if(HDR_SUB_HDR2_TOP_CTL_get_hdr2_en(IoReg_Read32(HDR_SUB_HDR2_TOP_CTL_reg)))
						{//RGB output because hdr
							rtd_pr_vsc_info("[SUB]Sub dither access data XY(%d %d) RGB(%d %d %d) \n", sub_dither_i_sub_dither_accessdata_posctrl_reg.startx, sub_dither_i_sub_dither_accessdata_posctrl_reg.starty, 
							sub_dither_i_sub_dither_readdata_data_y1_reg.y00 >> 2,
							sub_dither_i_sub_dither_readdata_data_c1_reg.u00 >>2, sub_dither_i_sub_dither_readdata_data_c1_reg.v00 >> 2);
						}
						else
						{//yuv output
							rtd_pr_vsc_info("[SUB]Sub dither access data XY(%d %d) YUV(%d %d %d) \n", sub_dither_i_sub_dither_accessdata_posctrl_reg.startx, sub_dither_i_sub_dither_accessdata_posctrl_reg.starty, 
							sub_dither_i_sub_dither_readdata_data_y1_reg.y00 >> 2,
							sub_dither_i_sub_dither_readdata_data_c1_reg.u00 >>2, sub_dither_i_sub_dither_readdata_data_c1_reg.v00 >> 2);
						}
					}			
				}
				else
				{
					sub_data_access_print_count --;
				}
			}
			up(&SUB_DATA_ACCESS_DEBUG_Semaphore);
		}
	}
}

void Scaler_Set_Vo_Force_60_Framerate_State(unsigned int state)
{
    unsigned int *vo_force_60_framerate_state_addr = NULL;
    vo_force_60_framerate_state_addr = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_VO_FORCE_60_FRAMERATE_STATE);
    if(vo_force_60_framerate_state_addr)
        *vo_force_60_framerate_state_addr = htonl(state);

}

unsigned int Scaler_Get_Vo_Force_60_Framerate_State(void)
{
    unsigned int *vo_force_60_framerate_state_addr = NULL;
    vo_force_60_framerate_state_addr = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_VO_FORCE_60_FRAMERATE_STATE);
    return (vo_force_60_framerate_state_addr? htonl(*vo_force_60_framerate_state_addr): 0);
}


/* qms_driver_enable* is only for demo QMS and QMS_suppressed effection.
 * (not a normal case. can be ignored or removed)
 */
void on3_vtotal_delta_set(int delta, int ie)
{
        // set On3_demode_vtotal_delta to trigger watchdog interrupt, then trigger setup IMD again.
        IoReg_Mask32(ONMS_Onms3_Demode_ctrl_reg, ~(0x0fff << 4), (delta<<4));
        if (ie) {
                IoReg_SetBits(ONMS_onms3_watchdog_en_reg, _BIT15);
                IoReg_SetBits(ONMS_onms3_interrupt_en_reg, _BIT15);
        } else {
                IoReg_ClearBits(ONMS_onms3_watchdog_en_reg, _BIT15);
                IoReg_ClearBits(ONMS_onms3_interrupt_en_reg, _BIT15);
        }
}

int qms_driver_enable_set(int enable)
{
        rtd_pr_vsc_alert("input_event_filter: qms_driver_enable: %d to %d\n", qms_driver_enable, enable);

        qms_driver_enable = enable;
        
        if ( get_scaler_qms_mode_flag()) {
                // to supress QMS flow for demo the FBG=1 across timing change
                if (qms_driver_enable == 0) { 
                        on3_vtotal_delta_set(500, 1);
                }

                if (enable != 0) { 
                        on3_vtotal_delta_set(0, 0);
                }
        }

        return 0;
}

#endif


//QMS driver start
unsigned char scaler_hdmi_qms_mode_enable = 0;//record scaler is use qms mode

unsigned char get_scaler_qms_mode_flag(void)
{//value from drvif_Hdmi_GetQmsStatus
	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
	{
		return scaler_hdmi_qms_mode_enable;
	}
	else
		return 0;
}

void set_scaler_qms_mode_flag (unsigned char enable)
{//decidescaler use qms mode
	scaler_hdmi_qms_mode_enable = enable;
}

unsigned char current_is_hdmi_qms_mode(void)
{//get hdmi qms mode from hdmi
	unsigned short current_vfreq;//dummy
	HDMI_QMS_STATUS_T *qms_status = NULL;
	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
	{
		qms_status = drvif_Hdmi_GetQmsStatus(&current_vfreq);
		if(qms_status)
		{
			if(qms_status->qms_fsm != HDMI_QMS_FSM_NOT_SUPPORT)
				return 1;
		}
	}
	return 0;
}


unsigned char check_hdmi_current_qms_status(unsigned short* stable_vfreq)
{//get hdmi qms status from hdmi
	unsigned short current_vfreq;//dummy
	HDMI_QMS_STATUS_T *qms_status = NULL;
	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
	{
		qms_status = drvif_Hdmi_GetQmsStatus(&current_vfreq);
		if(qms_status)
		{//no null
			if(stable_vfreq)
			{
				*stable_vfreq = qms_status->next_tfr_vfreq;//get next frame rate at the same frame
			}
			return qms_status->qms_fsm;
		}
	}
	return HDMI_QMS_FSM_NOT_SUPPORT;
}

void update_qms_block_info(I3DDMA_CAPTURE_BUFFER_T *capture_info)
{//update block addr and size info
	int i;
	HDMI_QMS_MODE_STATUS* hdmi_qms_status = NULL;
	hdmi_qms_status = (HDMI_QMS_MODE_STATUS *)Scaler_GetShareMemVirAddr(SCALERIOC_MDMI_QMS_INFO);
	if(hdmi_qms_status)
	{
		if(capture_info)
		{
			unsigned int addr = 0;
			hdmi_qms_status->QMS_I3CAP_WRITE_IDX = 0;
			//hdmi_qms_status->QMS_VODMA_READ_IDX = 0;
			hdmi_qms_status->DMA_BUFFER_SIZE = Scaler_ChangeUINT32Endian(capture_info[0].size);
			for(i = 0; i < QMS_BUFFER_NUM; i++)
			{
				addr = (unsigned int)capture_info[i].phyaddr;
				hdmi_qms_status->DMA_BUFFER_ADDR[i] = Scaler_ChangeUINT32Endian(addr);
			}
		}
		else
		{
			hdmi_qms_status->QMS_I3CAP_WRITE_IDX = 0;
			hdmi_qms_status->QMS_VODMA_READ_IDX = 0;
		}
	}
}

enum qms_message {
	QMS_WRITE_INDEX_HIT_READ_INDEX,
	QMS_STABLE_TO_UNSTABLE,
	QMS_UNSTABLE_TO_STABE,
	QMS_NO_MESSAGE
};

#define NUM_SKIP_UPDATE_WRITE_INDEX 0//no drop any frame for sync mode
void update_qms_i3ddma_block_write_idx(void)
{//update write index at capture done
	h3ddma_cap0_cap_ctl0_RBUS i3ddma_cap_ctl0_reg;
	unsigned int new_write_idx = 0;
	HDMI_QMS_MODE_STATUS* hdmi_qms_status = NULL;
	hdmi_qms_status = (HDMI_QMS_MODE_STATUS *)Scaler_GetShareMemVirAddr(SCALERIOC_MDMI_QMS_INFO);
	if(hdmi_qms_status)
	{
		new_write_idx = (Scaler_ChangeUINT32Endian(hdmi_qms_status->QMS_I3CAP_WRITE_IDX) + 1) % QMS_BUFFER_NUM;
			i3ddma_cap_ctl0_reg.regValue = IoReg_Read32(H3DDMA_CAP0_Cap_CTL0_reg);
			i3ddma_cap_ctl0_reg.cap0_cap_block_fw = new_write_idx;//select block index
			IoReg_Write32(H3DDMA_CAP0_Cap_CTL0_reg, i3ddma_cap_ctl0_reg.regValue);
			hdmi_qms_status->QMS_I3CAP_WRITE_IDX = Scaler_ChangeUINT32Endian(new_write_idx);
		}
}

#ifdef QMS_DEBUG_PRINT
void hdmi_qms_debug_print(void)
{//confirm hdmi info
	unsigned int online_period = 0;
	static unsigned char enternum = 0;
 	static unsigned int last_time = 0;//90k
	unsigned int cur_time = 0;//90k
	HDMI_QMS_MODE_STATUS* hdmi_qms_status = NULL;
	hdmi_qms_status = (HDMI_QMS_MODE_STATUS *)Scaler_GetShareMemVirAddr(SCALERIOC_MDMI_QMS_INFO);
	cur_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
	if(IoReg_Read32(0xb8022710) & _BIT18)
	{
		online_period = ONMS_Onms3_Demode_Vs_period_get_on3_de_vs_period(IoReg_Read32(ONMS_Onms3_Demode_Vs_period_reg));
		rtd_pr_vsc_emerg("### [QMS] func:%s num:%d ready(%d) nfr:%d time_diff:%d onine:%d(ms) dmaline:%d 90k:%x###\r\n",__FUNCTION__, enternum, (hdmi_qms_status->QMS_READY != 0), Scaler_ChangeUINT32Endian(hdmi_qms_status->QMS_STABLE_FRAME_RATE)
			, drvif_report_cost_time_by_counter(last_time), online_period? 27000000/online_period : 0,
			DMA_VGIP_DMA_VGIP_LC_get_dma_line_cnt(IoReg_Read32(DMA_VGIP_DMA_VGIP_LC_reg)), cur_time);
	}
	last_time = cur_time;
	enternum ++;
}
#endif

unsigned char check_qms_unstable_period(void)
{//QMS scaler is active and mcost =0 retunr true
	HDMI_QMS_MODE_STATUS* hdmi_qms_status = NULL;
	if(get_scaler_qms_mode_flag() && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE))
	{
		hdmi_qms_status = (HDMI_QMS_MODE_STATUS *)Scaler_GetShareMemVirAddr(SCALERIOC_MDMI_QMS_INFO);
		if(hdmi_qms_status)
		{
			if(hdmi_qms_status->QMS_READY == 0)
				return TRUE;//unstable
		}
	}
	return FALSE;
}

static DEFINE_SPINLOCK(QMS_FRAME_RATE_UPDATE_LOCK);/*Spin lock no context switch. This is for qms_framerate_change*/
static unsigned char qms_framerate_change_flag = 0;//qms frame rate change

void set_qms_framerate_change_flag(unsigned char value)
{
	unsigned long flags;
    spin_lock_irqsave(&QMS_FRAME_RATE_UPDATE_LOCK, flags);
	qms_framerate_change_flag = value;
	spin_unlock_irqrestore(&QMS_FRAME_RATE_UPDATE_LOCK, flags);
}

void qms_notify_memc_input_output_frame_rate(void)
{//call void Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY)
	if(qms_framerate_change_flag)
	{
		set_qms_framerate_change_flag(0);//reset
		if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) && get_scaler_qms_mode_flag())
		{
			Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);
		}
	}
}

unsigned int get_qms_current_framerate(void)
{//get current frame rate for qms for vbe setting
	HDMI_QMS_MODE_STATUS* hdmi_qms_status = NULL;
	hdmi_qms_status = (HDMI_QMS_MODE_STATUS *)Scaler_GetShareMemVirAddr(SCALERIOC_MDMI_QMS_INFO);
	if(hdmi_qms_status)
	{
		return Scaler_ChangeUINT32Endian(hdmi_qms_status->QMS_STABLE_FRAME_RATE);
	}
	return 0;
}


void update_hdmi_qms_info_shamemory(void)
{//updatehdmi qms info to share memory
	unsigned int cur_time = 0;//90k
	unsigned int dma_line_cnt = 0, vo_line_cnt = 0;
	HDMI_QMS_MODE_STATUS* hdmi_qms_status = NULL;
	unsigned short stable_framerate = 0;
	static unsigned short last_framerate =0;
	hdmi_qms_status = (HDMI_QMS_MODE_STATUS *)Scaler_GetShareMemVirAddr(SCALERIOC_MDMI_QMS_INFO);
	if(hdmi_qms_status)
	{
		hdmi_qms_status->QMS_ENABLE = Scaler_ChangeUINT32Endian(get_scaler_qms_mode_flag());
		if(hdmi_qms_status->QMS_ENABLE)
		{//QMS mode
			cur_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			dma_line_cnt = DMA_VGIP_DMA_VGIP_LC_get_dma_line_cnt(IoReg_Read32(DMA_VGIP_DMA_VGIP_LC_reg));
			vo_line_cnt = VODMA_VODMA_LINE_ST_get_line_cnt(IoReg_Read32(VODMA_VODMA_LINE_ST_reg));

			hdmi_qms_status->QMS_PRE_READY = hdmi_qms_status->QMS_READY;//save previous frame ready status
			if(check_hdmi_current_qms_status(&stable_framerate) != HDMI_QMS_FSM_READY)
			{//M_CONST = 0
				hdmi_qms_status->QMS_READY = 0;
				hdmi_qms_status->QMS_STABLE_FRAME_RATE = Scaler_ChangeUINT32Endian((unsigned int)stable_framerate);
				if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
				{//avtive to report the memc input/output frame rate
					if(last_framerate != stable_framerate)
					{
						set_qms_framerate_change_flag(1);//reset
					}
				}
				else
				{
					if(qms_framerate_change_flag)
					{//reset
						set_qms_framerate_change_flag(0);//reset
					}
				}
				last_framerate = stable_framerate;
				rtd_pr_vsc_emerg("### [QMS] func:%s unstable case frameinfo:%d dmalt:%d volt:%d 90k:%x###\r\n",__FUNCTION__, stable_framerate, dma_line_cnt, vo_line_cnt, cur_time);
			}
			else
			{//M_CONST = 1
				hdmi_qms_status->QMS_STABLE_FRAME_RATE = Scaler_ChangeUINT32Endian((unsigned int)stable_framerate);
				hdmi_qms_status->QMS_READY = Scaler_ChangeUINT32Endian(1);
				if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
				{//avtive to report the memc input/output frame rate
					if(last_framerate != stable_framerate)
					{
						set_qms_framerate_change_flag(1);//reset
					}
				}
				else
				{
					if(qms_framerate_change_flag)
					{//reset
						set_qms_framerate_change_flag(0);//reset
					}
				}
				if(hdmi_qms_status->QMS_PRE_READY == 0)
				{
                                        rtd_pr_vsc_emerg("### [QMS] func:%s unstable->stable case frameinfo(%d -> %d) dmalt:%d volt:%d 90k:%x ###\r\n",__FUNCTION__, last_framerate, stable_framerate, dma_line_cnt, vo_line_cnt, cur_time);
					last_framerate = stable_framerate;
				}
				else if(last_framerate != stable_framerate)
				{
					rtd_pr_vsc_emerg("### [QMS] func:%s ERR stable case frameinfo change(%d -> %d) dmalt:%d volt:%d 90k:%x ###\r\n",__FUNCTION__, last_framerate, stable_framerate, dma_line_cnt, vo_line_cnt, cur_time);
					last_framerate = stable_framerate;
				}
			}
#ifdef QMS_DEBUG_PRINT
			hdmi_qms_debug_print();
#endif
		}
		else
		{//normal mode
			hdmi_qms_status->QMS_PRE_READY = 0;
			hdmi_qms_status->QMS_READY = 0;
			hdmi_qms_status->QMS_STABLE_FRAME_RATE = 0;
			if(qms_framerate_change_flag)
			{
				set_qms_framerate_change_flag(0);//reset
			}
		}
	}
}
//QMS driver end


/****************************Low Power start***********************************/

unsigned char drvif_wait_timeout_check_by_counter(unsigned int base_time, unsigned int timeout)
{//if timeout return True  timeout : unit is ms
    unsigned int cur_counter = 0;
    unsigned int time_diff = 0;

    cur_counter = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
    if(cur_counter < base_time) {
        time_diff = (cur_counter + (0xFFFFFFFF - base_time));
    } else {
        time_diff = (cur_counter - base_time);
    }

    if((time_diff/90) >= timeout)
        return TRUE;//already time out

    return FALSE;//not yet time out
}

unsigned char drvif_wait_time_by_counter(unsigned int base_time, unsigned int wait_time)
{//if return True: ok wait_time : unit is ms. use for at least time
    unsigned int cur_counter = 0;
    unsigned int time_diff = 0;

    cur_counter = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
    if(cur_counter < base_time) {
        time_diff = (cur_counter + (0xFFFFFFFF - base_time));
    } else {
        time_diff = (cur_counter - base_time);
    }

    if((time_diff/90) >= wait_time)
        return TRUE;//already ready

    return FALSE;//not yet wait finish
}

unsigned int drvif_report_cost_time_by_counter(unsigned int base_time)
{//report wait_time : unit is ms
    unsigned int cur_counter = 0;
    unsigned int time_diff = 0;

    cur_counter = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
    if(cur_counter < base_time) {
        time_diff = (cur_counter + (0xFFFFFFFF - base_time));
    } else {
        time_diff = (cur_counter - base_time);
    }

    return (time_diff/90);//not yet wait finish
}

static unsigned char scaler_low_power_mode = FALSE;//for low power  audio only case

unsigned char Get_Val_scaler_low_power_mode(void)
{//get to enter low power mode or not
    return scaler_low_power_mode;
}

void Set_Val_scaler_low_power_mode(unsigned char condition)
{//decide to enter low power mode or not
    scaler_low_power_mode = condition;
}

/****************************Low Power end***********************************/

/*======================== End of File =======================================*/
/**
*
* @}
*/


