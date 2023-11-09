//Kernel Header file
#ifndef BUILD_QUICK_SHOW
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <generated/autoconf.h>
#include <asm/cacheflush.h>
#include <linux/pageremap.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
#include <linux/spinlock_types.h>/*For spinlock*/
//#include <rtk_kdriver/common.h>
//#include <rtk_kdriver/io.h>
#include <rtk_kdriver/RPCDriver.h>
#include <rtk_kdriver/rtk_pwm.h>
#include <mach/platform.h>
#include <mach/rtk_platform.h>
#include <mach/timex.h>
#include <scaler_vtdev.h>
#else
#include <no_os/printk.h>
#include <no_os/semaphore.h>
#include <no_os/spinlock.h>
#include <no_os/spinlock_types.h>
#include <rtd_log/rtd_module_log.h>
#include <timer.h>
#include <div64.h>
#include "vby1/panel_api.h"
#endif
#include <tvscalercontrol/hdmirx/hdmi_vfe.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>



#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
#include <scalercommon/scalerCommon.h>
#endif

#include "scaler_vbedev.h"
#include "scaler_vfedev.h"
#include "scaler_vscdev.h"

#include "tvscalercontrol/io/ioregdrv.h"
#include "tvscalercontrol/scalerdrv/scalerdrv.h"
#include "tvscalercontrol/scalerdrv/scalerdisplay.h"
#include "tvscalercontrol/scalerdrv/adjust.h"
#include "tvscalercontrol/scalerdrv/scalerclock.h"
#include "tvscalercontrol/scalerdrv/framesync.h"
#include "tvscalercontrol/scalerdrv/conversion_2d_3d.h"
#include <tvscalercontrol/scalerdrv/scaler_2dcvt3d_ctrl.h>
#include <rbus/scaledown_reg.h>

//Fixed me later due to rtk_vo.h compiler error
//#include <vo/rtk_vo.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <tvscalercontrol/vdac/vdac.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/viptable.h>
#ifndef BUILD_QUICK_SHOW
#include <scaler_vpqdev.h>
#include <scaler_vpqleddev.h>
#include <scaler_vpqmemcdev.h>
#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/avd/avdctrl.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#endif
// FixMe, 20190919
//#include <hdmirx/hdmi_emp.h>
#include "memc_isr/HAL/hal_mc.h"
#include "memc_isr/HAL/hal_crtc.h"

#include <rbus/timer_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/sfg_reg.h>
//#include <rbus/pif_reg.h>
#include <rbus/mdomain_cap_reg.h>
#include <rbus/mdomain_disp_reg.h>
#include <rbus/scaledown_reg.h>

//#include <rbus/rbus_DesignSpec_MISC_GPIOReg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <rbus/dma_vgip_reg.h>
#include <rbus/h3ddma_rgb2yuv_dither_4xxto4xx_reg.h>
#include <rbus/h3ddma_reg.h>
#include <rbus/h3ddma_hsd_reg.h>
//#include <rbus/tve_dma_reg.h>
//#include <rbus/tve_tv_reg.h>
#include <rbus/vodma_reg.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/vby1_reg.h>
#include <rbus/vby1_osd_reg.h>
#include <rbus/tcon_reg.h>
#include <rbus/epi_reg.h>
#include <rbus/pinmux_lvdsphy_reg.h>
#include <rbus/hdr_all_top_reg.h>
//#include <linux/i2c.h>
//#include <rbus/d_2d_to_3d_reg.h>
//#include <rbus/dispd_boe_rgbw_reg.h>
#include <rbus/scaleup_reg.h>
//#include <rbus/lg_m_plus_reg.h>
#include <rbus/vgip_reg.h>
#include <rbus/od_dma_reg.h>
#include <rbus/ldspi_reg.h>
//#include <rbus/tve_reg.h>
#include <rbus/vodma_reg.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/hdr_all_top_reg.h>
#include <rbus/dhdr_v_composer_reg.h>
#include <rbus/dm_reg.h>
#include <tvscalercontrol/panel/panel_dlg.h>
#include <tvscalercontrol/scalerdrv/mode.h>
#if 0// to sync tv006 patch it will let compiler error
#include <rpc/VideoRPC_System.h>
#include <rpc/VideoRPC_System_data.h>
#endif
//extern webos_info_t  webos_tooloption; //to sync tv006 patch
#ifndef BUILD_QUICK_SHOW
#ifndef CONFIG_MEMC_NOTSUPPORT
#include "memc_reg_def.h"
#endif
#include "memc_isr/MID/mid_mode.h"
#include "memc_isr/include/memc_lib.h"
#endif

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#define to_user_ptr(x)          compat_ptr((unsigned int)x)
#else
#define to_user_ptr(x)          ((void* __user)(x)) // convert 32 bit value to user pointer
#endif

#ifdef VBE_DISP_TCON_SELF_CHECK
#include <rbus/mdomain_disp_reg.h>
extern unsigned int monitor_rgb_rolling;
#endif

// for register dump
#include <tvscalercontrol/io/ioregdrv.h>
#undef rtd_outl
#define rtd_outl(x, y)     								IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)     									IoReg_Read32(x)
#undef rtd_maskl
#define rtd_maskl(x, y, z)     							IoReg_Mask32(x,y,z)
#undef rtd_setbits
#define rtd_setbits(offset, Mask) rtd_outl(offset, (rtd_inl(offset) | Mask))
#undef rtd_clearbits
#define rtd_clearbits(offset, Mask) rtd_outl(offset, ((rtd_inl(offset) & ~(Mask))))

#define OLED_DISP_FRAMERATE_LIMIT_120HZ 224400 // 27000000/224400=120.32Hz
#define OLED_DISP_FRAMERATE_LIMIT_60HZ 448600 // 27000000/448600=60.18Hz

#if IS_ENABLED(CONFIG_RTK_KDRV_GDMA) && !defined(CONFIG_CUSTOMER_TV006)
extern int gdma_resume_by_vbe(void);
extern int gdma_suspend_by_vbe(void);
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_GDMA)
//call GAL osd resume
extern int GAL_Runtime_Resume_light(void);
//call GAL osd suspend
extern int GAL_Runtime_Suspend_light(void);
#endif

#ifndef BUILD_QUICK_SHOW
static struct cdev vbe_cdev;
#endif

int vbe_major   = VBE_MAJOR;
int vbe_minor   = 0 ;
int vbe_nr_devs = VBE_NR_DEVS;

struct semaphore VBE_Semaphore;
struct semaphore VBE_LowDelay_Semaphore;
static struct semaphore VBE_RGBW_Resume_Semaphore;
#ifndef BUILD_QUICK_SHOW
module_param(vbe_major, int, S_IRUGO);
module_param(vbe_minor, int, S_IRUGO);
module_param(vbe_nr_devs, int, S_IRUGO);
#endif
static struct class *vbe_class;
void vbe_disp_early_resume(void);
extern struct semaphore Orbit_Semaphore;
extern unsigned char get_MEMC_bypass_status_refer_platform_model(void);
extern PANEL_PIXEL_MODE get_panel_pixel_mode(void);
extern unsigned int Get_DISP_VERTICAL_TOTAL(void);
extern unsigned int Get_DISPLAY_CLOCK_TYPICAL(void);
extern unsigned int Get_DISPLAY_REFRESH_RATE(void);
extern unsigned int Get_DISP_HORIZONTAL_TOTAL(void);
extern unsigned int Get_DISPLAY_PANEL_CUSTOM_INDEX(void);
#ifdef CONFIG_PM
//static VBE_AVE_SUSPEND_RESUME_T vbe_ave_suspend_resume;
static VBE_DISP_TCON_SUSPEND_RESUME_T vbe_disp_tcon_suspend_resume;
void vbe_disp_panel_resume(void);
extern void vpq_memc_disp_suspend(void);
extern void vpq_memc_disp_resume(void);
VBE_DISP_SUSPEND_RESUME_T vbe_disp_suspend_resume;
VBE_DISP_PANEL_EPI_SUSPEND_RESUME_T vbe_disp_panel_epi_suspend_resume;
VBE_DISP_PANEL_TCON_SUSPEND_RESUME_T vbe_disp_panel_tcon_suspend_resume;
static int vbe_suspend_instant_mode = 0; //use this flag to distinguish STR or STD suspend
#endif
extern StructSrcRect main_input_size;
extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr; // debug scaler flag tracker
extern void Check_smooth_toggle_update_flag(unsigned char display);
extern unsigned char g_MEMC_powersaving_on;
extern unsigned char get_vo_camera_flow_flag(unsigned char ch);
extern void drvif_framesync_gatting(unsigned char enable);
extern void Scaler_MEMC_SetInOutFrameRateByDisp(unsigned char _channel);
extern long vpq_memc_set_lowDelayMode(unsigned char type);
extern void Scaler_MEMC_Bypass_On(unsigned char b_isBypass);
extern unsigned char Scaler_MEMC_Lib_UltraLowDelayMode_Judge(void);
extern void Scaler_MEMC_ForceBg(unsigned char bEnable);
extern unsigned int Hdmi_Search_Mode(unsigned int *p_mid);

/*******************************************************************************
****************************VBE DISP DRIVER*************************************
*******************************************************************************/
static unsigned char VBE_DISP_Global_Status = VBE_DISP_NOTHING;
unsigned char panel_bl_type;
unsigned char led_bar_type;
unsigned char epi_panel_type;
unsigned char panel_inch;
unsigned char panel_maker;
unsigned int disp_osd_width;
unsigned int disp_osd_height;
BOOLEAN disp_ForceFreeRun=DISP_TIMING_DEFAULT;
unsigned short disp_framerate;
unsigned int vtotal_measure;
unsigned int mask_3d_auto_detect = 0;
static UINT8 first_boot_run_main = FALSE;
static UINT8 first_disp_resume = FALSE;
extern struct semaphore Iv2dv_Slow_Tuning_Semaphore;
static UINT8 oled_resume_eit_flag = FALSE; //this flag control sfg forcebg when enable OLED panel EIT function
unsigned char vbe_dynamic_memc_bypass_flag = 0;
unsigned int disp_data_frame_sync_flag = FALSE;
unsigned int adaptivestream_force_framerate = 60;
static unsigned int stBacklihgtStart_Tick = 0;
unsigned int disp_smooth_variable_setting_flag = 0;
unsigned int vbe_dynamicPolarityControl_initialized_status = 0;
unsigned int vbe_dynamicPolarityControl_detect_frames = 10;
unsigned int vbe_dynamicPolarityControl_detect_threadhold = 0;
unsigned int vbe_dynamicPolarityControl_check_frame = 0;
unsigned int vbe_dynamicPolarityControl_enable = 0;
unsigned int vbe_dynamicPolarityControl_debug_mode = 0;
unsigned int vbe_print_debug_line_count = 0;
unsigned int vbe_bringup_test_case = 0;
unsigned int vbe_vo_tracking_d_panel_type = 1;
//#define DPC_ONE_RUN_FRAME 10
unsigned int vbe_tcon_28s_toggle_enable = 0;
unsigned int vbe_tcon_28s_timer_counter = 0;
unsigned char vbe_tcon_28s_toggle_state = 0;
unsigned char vbe_stop_memc_cmd_flag = 0;

SCALER_DATA_FS_ALGO_RESULT g_tDataFsAlgoResult;

static ORBIT_PIXEL_SHIFT_STRUCT isr_orbit_shift;
int pre_den_end_line_count =0;
#ifdef CONFIG_ORBIT_PIXEL_SHIFT_ALGO
/*
typedef struct{
	int  x;
	int  y;
}ORBIT_PIXEL_SHIFT_STRUCT;

enum vbe_panel_orbit_mode {
    _VBE_PANEL_ORBIT_JUSTSCAN_MODE = 0,
    _VBE_PANEL_ORBIT_AUTO_MODE,
    _VBE_PANEL_ORBIT_STORE_MODE,
    _VBE_PANEL_OREBIT_MODE_MAX
};
*/
#define ORBIT_JUSTSCAN_4K120_DELAY 114

#define ORBIT_JUSTSCAN_SHIFT_X      (16)
#define ORBIT_JUSTSCAN_SHIFT_Y      (8)
#define ORBIT_OVERSCAN_SHIFT_X      (16)
#define ORBIT_OVERSCAN_SHIFT_Y      (8)
#define ORBIT_STORE_SHIFT_X         (32)
#define ORBIT_STORE_SHIFT_Y         (16)

unsigned char vbe_disp_oled_orbit_enable=FALSE;
unsigned char vbe_disp_oled_orbit_mode=0; //0 justscan
extern unsigned int ORBIT_TIME;//default 3min
extern void ScalerForceUpdateOrbit(unsigned char force_update);


ORBIT_PIXEL_SHIFT_STRUCT orbit_move_range[_VBE_PANEL_OREBIT_MODE_MAX] =
{
	{ORBIT_JUSTSCAN_SHIFT_X, ORBIT_JUSTSCAN_SHIFT_Y},	//justscan   20 * 10(10, 5)   32*16 (16, 8)
	{ORBIT_OVERSCAN_SHIFT_X, ORBIT_OVERSCAN_SHIFT_Y},	//overscan   20 * 10(10, 5)   32*16 (16, 8)
	{ORBIT_STORE_SHIFT_X, ORBIT_STORE_SHIFT_Y},	//store   64 * 32(32, 16)
};
#endif
#ifndef BUILD_QUICK_SHOW

ORBIT_PIXEL_SHIFT_STRUCT Get_Orbit_Shift_Data(void)
{
    return isr_orbit_shift;
}
void Set_Orbit_Shift_Data(ORBIT_PIXEL_SHIFT_STRUCT orbit_shift_data)
{
    isr_orbit_shift = orbit_shift_data;
}

unsigned char Get_PANEL_BL_type(void)
{
	return panel_bl_type;
}
void Set_PANEL_BL_type(unsigned char type)
{
	panel_bl_type=type;
}
unsigned char Get_PANEL_LED_Bar_type(void)
{
	return led_bar_type;
}
void Set_PANEL_LED_Bar_type(unsigned char type)
{
	led_bar_type=type;
}
unsigned char Get_PANEL_EPI_TYPE(void)
{
	return epi_panel_type;
}
void Set_PANEL_EPI_TYPE(unsigned char type)
{
	epi_panel_type=type;
}
unsigned char Get_PANEL_INCH(void)
{
	return panel_inch;
}
void Set_PANEL_INCH(unsigned char inch)
{
	panel_inch=inch;
}

unsigned char Get_PANEL_MAKER(void)
{
	return panel_maker;
}
void Set_PANEL_MAKER(unsigned char maker)
{
	panel_maker=maker;
}

unsigned int Get_DISP_OSD_Width(void)
{
	return disp_osd_width;
}
void Set_DISP_OSD_Width(unsigned int width)
{
	disp_osd_width=width;
}
unsigned int Get_DISP_OSD_Height(void)
{
	return disp_osd_height;
}
void Set_DISP_OSD_Height(unsigned int height)
{
	disp_osd_height=height;
}
void Disp_Enable_isForceFreeRun(BOOLEAN isForceFreeRun)
{
	if(isForceFreeRun)
	{
		disp_ForceFreeRun=DISP_FREE_RUN;
	}
	else
	{
		disp_ForceFreeRun=DISP_FRAMESYNC;
	}
}
#endif

BOOLEAN Disp_Get_isForceFreeRun(void)
{
	 return disp_ForceFreeRun;
}

void Disp_Set_framerate(unsigned short framerate)
{
	disp_framerate=framerate;
}
unsigned short Disp_Get_framerate(void)
{
	return disp_framerate;
}

unsigned char vflip_enable=0;
unsigned char mirror_flag=0;

void Set_Mirror_VFLIP_ENABLE(unsigned char enable)
{
	vflip_enable=enable;
	mirror_flag=1;
}
unsigned char Get_Mirror_VFLIP_ENABLE(void)
{
	mirror_flag=0;
	return vflip_enable;
}
unsigned char Get_vbe_mirror_vflip(void)
{
	if(mirror_flag)
	{
		return vflip_enable;
	}
	else
	{
		return 2; // 2 is bypass vbe mirror
	}
}

unsigned char fll_running_flag=0;

unsigned char vbe_disp_gamemode_use_fixlastline(void)
{
	if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC) || get_vsc_run_adaptive_stream(SLR_MAIN_DISPLAY))
		return FALSE;

	//for debug purpose
	if(IoReg_Read32(VGIP_VGIP_VBISLI_reg)&_BIT23)
		return FALSE;
	else
		return TRUE;
}

unsigned char vbe_disp_dataFramesync_use_fixlastline(void)
{
	//for debug purpose
	if(IoReg_Read32(VGIP_VGIP_VBISLI_reg)&_BIT24)
		return TRUE;
	else
		return FALSE;
}

void modestate_set_fll_running_flag(unsigned char bEnable)
{
	//rtd_pr_vbe_notice("### modestate_set_fll_running_flag (%d) ###\n", bEnable);
	fll_running_flag = bEnable;
}

unsigned char modestate_get_fll_running_flag(void)
{
	return fll_running_flag;
}
#ifndef BUILD_QUICK_SHOW

unsigned char vbe_disp_timing_change_stable_flag = 0;

void vbe_set_disp_timing_change_status(unsigned char stable)
{
	vbe_disp_timing_change_stable_flag = stable;
	rtd_pr_vbe_emerg("[%s] Set timing change status (%d)\n", __FUNCTION__, vbe_disp_timing_change_stable_flag);
}

unsigned char vbe_get_disp_timing_change_status(void)
{
	//rtd_pr_vbe_emerg("[%s] Timing change stable = %d\n", __FUNCTION__, vbe_disp_timing_change_stable_flag);
	return vbe_disp_timing_change_stable_flag;
}


unsigned char modestate_decide_display_timing(void)
{
	if(Disp_Get_isForceFreeRun()==DISP_FREE_RUN){
		return DISPLAY_MODE_FRC;
	}else if(_TRUE == Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)){
		return DISPLAY_MODE_FRAME_SYNC;
	}
#ifdef ENABLE_VR360_DATA_FS_FLOW
	else if(get_vt_EnableFRCMode() == FALSE){
		return DISPLAY_MODE_FRAME_SYNC;
	}
#endif
	else if(modestate_check_input_special_framerate()){
		return DISPLAY_MODE_FRC;
	}else if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)){
		return DISPLAY_MODE_FRC;
	}else
		return DISPLAY_MODE_NEW_FLL_MODE;
}
#endif

unsigned char vbe_disp_get_adaptivestream_fs_mode(void)
{
	char keepAdaptiveStreamFsMode=0;
//	vp9_ctrl0_RBUS vodma_vp9_ctrl0_reg;

	keepAdaptiveStreamFsMode = (vsc_get_adaptivestream_flag(SLR_MAIN_DISPLAY) != 0) ? TRUE : FALSE;
/*
	vodma_vp9_ctrl0_reg.regValue = IoReg_Read32(VODMA_VP9_CTRL0_reg);
	if((vodma_vp9_ctrl0_reg.vp9_en == 1)&&(vsc_get_adaptivestream_flag()==1)){
		keepAdaptiveStreamFsMode = 1;
	}else{
		keepAdaptiveStreamFsMode = 0;
	}
*/
	if(Get_DisplayMode_Src(0) != VSC_INPUTSRC_VDEC && Get_DisplayMode_Src(0) != VSC_INPUTSRC_CAMERA){
		keepAdaptiveStreamFsMode = 0;
	}

	//rtd_pr_vbe_notice("file=%s, function=%s, keepAdaptiveStreamFsMode=%d\n", __FILE__, __FUNCTION__,  keepAdaptiveStreamFsMode);

	return keepAdaptiveStreamFsMode;
}

void vbe_disp_set_adaptivestream_force_framerate(unsigned int framerate)
{
	if (vbe_disp_get_adaptivestream_fs_mode())
		adaptivestream_force_framerate = framerate;
	else
		adaptivestream_force_framerate = 0;
}

unsigned int vbe_disp_get_adaptivestream_force_framerate(void)
{
	if (0 == vbe_disp_get_adaptivestream_fs_mode()) {
		return 0;
	}
	return adaptivestream_force_framerate;
}
void modestate_set_display_timing(unsigned char dclk_is_max)
{
	ppoverlay_dv_total_RBUS dv_total_reg;
	if(Disp_Get_isForceFreeRun()==DISP_FREE_RUN){
		rtd_pr_vbe_debug("[modestate_set_display_timing] Disp_Get_isForceFreeRun! \n");
		drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
		return;
	}
//	unsigned int vtotal;
	//framesync mode will set framesync bit at other place
	//if vby1 to hdmi board, used new fll mode
#ifdef ENABLE_VBY1_TO_HDMI_4K2K_DATA_FS
	if ((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)) {
		if(_TRUE == Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))
			return;
	}
#else
	if ((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)) {
			drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
			return;
	}
#endif

#ifdef CONFIG_DUAL_CHANNEL
	//sub display not apply
	if(SLR_SUB_DISPLAY == Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)){
		rtd_pr_vbe_emerg("[modestate_set_display_timing] SLR_SUB_DISPLAY Force return! \n");
		return;
	}
#endif

	//timing
	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	vtotal_measure=dv_total_reg.dv_total;
#if 0
	if(Disp_Get_isForceFreeRun()==DISP_FREE_RUN)
	{
		drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
		vtotal=(Get_DISPLAY_CLOCK_TYPICAL()/Get_DISP_HORIZONTAL_TOTAL()/Disp_Get_framerate());
		dv_total_reg.dv_total = vtotal;
		IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
		rtd_pr_vbe_info("DVTotal:%x, 8014:%x\n", vtotal, IoReg_Read32(PPOVERLAY_DV_total_reg));
		vtotal_measure=Get_DISP_VERTICAL_TOTAL();
		return;
	}
	else if(Disp_Get_isForceFreeRun()==DISP_FRAMESYNC)
	{
		drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
		return;
	}
#endif
#ifndef BUILD_QUICK_SHOW

	// [SG] 3D free run condition -- SG 3D FRC@120Hz for 3D or 2Dcvt3D mode
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE()){
		if((drvif_scaler3d_get_IDMA3dMode() != SLR_IDMA_MODE_DISABLE)
			|| ((drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() == TRUE) && drvif_scaelr3d_decide_is_3D_display_mode()))
		{
			// don't change DTG timing to frame sync mode in IDMA data FRC mode
			if(drvif_scaler_get_display_mode() == 0){
				rtd_pr_vbe_debug("[IDMA] IGNORE FS_EN\n");
				return;
			}
		}
		else if(drvif_scaelr3d_decide_is_3D_display_mode()
			&& (Get_PANEL_3D_SG_OUTPUT_120HZ_ENABLE() || Get_PANEL_3D_PR_TO_SG_OUTPUT_FRC_ENABLE()))
		{
			// 3D display timing free run for SG or PR to SG 3D panel
			drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
			return;
		}
	}
#endif
#if 0 // marked unused code in merlin by ben_wang
	if(Get_PANEL_TYPE_IS_MILLET3_4K2K_SERIES() == 1){

/*		if(Get_DISPLAY_2K1K_windows_mode() == 1){
				rtd_pr_vbe_info("+++++++ Custom window mode : go FRC timing +++++++++ \n");
 				drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
		}
		else{
*/
		{
 			if(Get_PANEL_3D_SG_OUTPUT_120HZ_ENABLE()){
 				rtd_pr_vbe_info("+++++++ 2k1k lvds t120Hz iming +++++++++ \n");
				// 3D data FRC path
				if((modestate_I3DDMA_get_In3dMode()|| modestate_get_vo3d_in_framePacking_FRC_mode()
					|| ((drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() == TRUE) && drvif_scaelr3d_decide_is_3D_display_mode()))
					&& (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == FALSE))
				{
					// 3D free run 120Hz
					drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
	 				rtd_pr_vbe_info("[IDMA] 3D=%d, data/timing FRC\n", Scaler_Get3DMode());
				}
				else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 610) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 1000) && (Scaler_InputSrcGetMainChType()!=_SRC_TV))
				{
					drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
	 				rtd_pr_vbe_info("go FRC timing\n");
				}else{
	 				rtd_pr_vbe_info("go Framesync timing\n");
					drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
				}
 			}
 			else{
 				rtd_pr_vbe_info("+++++++ 4k2k lvds 30Hz timing +++++++++ \n");
				rtd_pr_vbe_info("go Framesync timing\n");
				drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
 			}
		}
		return;
	}
#endif

	if(modestate_check_input_special_framerate()){
		rtd_pr_vbe_debug("[modestate_set_display_timing] Special input framerate, set timing free run! \n");
		drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
		return;
	}
/*
	if((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) < 2048) && (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) < 1200)
	  &&((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 235) || ((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 255) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 290)))){
		rtd_pr_vbe_info("[modestate_set_display_timing] Special input framerate, set timing free run! \n");
		drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
*/
	//enable new fix last line
	{
		drvif_scaler_set_display_mode(DISPLAY_MODE_NEW_FLL_MODE);
		return;
	}


	//USER:LewisLee DATE:2010/12/7
	//to prevent Vflip state, it have frame tear
	//we set framesync at framesync_lastlinefinetune_at_frc_mode()
	//let IVS2DVS delay have good relationship
//	if(_ENABLE == Scaler_Get_Vflip_Enable()) //frank@0517 sync Mac project let timing go framesync to solve mantis#38804 movie delay problem
//	{
//#ifndef VBY_ONE_PANEL //frank@0521 add VBY1 define to solve VBY1 panel shift problem
#if 0 // marked unused code in merlin by ben_wang
		if(_TRUE == Scaler_EnterFRC_LastLine_Condition())
		{
			drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
			drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC); //frank@05232014 enable FS timing when fll flow to solve mantis#75280
			return;
		}
#endif
//#endif
//	}
#if 0

	if (Scaler_InputSrcGetMainChType() == _SRC_TV)
	{

#ifdef CONFIG_ATV_FRC
		drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
		return;
#endif //#ifdef CONFIG_ATV_FRC

#if 1 //#ifdef CONFIG_FV_CHECK
		if (!CTVCheckFVBias() && !Scaler_TVGetScanMode()) {
 			drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
			return;
		}
#endif //#ifdef CONFIG_FV_CHECK

		//USER:LewisLee DATE:2012/06/27
		//for customer request, weak signal keep in framesync
		if(_TRUE == drvif_video_status_reg(VDC_hv_lock))
		{
			if(_TRUE == framesync_get_enterlastline_at_frc_mode_flg())
			{
				//In Fix last line mode, can keep in framesync mode
				drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
				rtd_pr_vbe_info("Change to FrameSync7-1\n");
			}
			else// if(_FALSE == framesync_get_enterlastline_at_frc_mode_flg())
			{
				if(_TRUE == Scaler_Get_ATV_WeakSignal_ChangeToFRC_Flag())
				{
					if(drvif_module_vdc_NoiseStatus() < Scaler_Get_ATV_VDC_NoiseLevel_Threshold())
					{
						drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
						rtd_pr_vbe_info("Change to FrameSync7-2\n");
					}
					else
					{
					        drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
					}
				}
				else// if(_FALSE == Scaler_Get_ATV_WeakSignal_ChangeToFRC_Flag())
				{
					drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
					rtd_pr_vbe_info("Change to FrameSync7-3\n");
				}
			}
		}
		else// if(_FALSE == drvif_video_status_reg(VDC_hv_lock))
		{
			drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
		}
 			//frank@0428, when source is VGA or DVI source,go FRC timing
	}
	else if ((Scaler_InputSrcGetMainChType() == _SRC_VGA) || // VGA
		((Scaler_InputSrcGetMainChFrom() == _SRC_FROM_TMDS) && (!drvif_IsHDMI())) || // DVI & DisplayPort
		((Scaler_InputSrcGetMainChFrom() == _SRC_FROM_TMDS) && drvif_IsHDMI() && (drvif_Hdmi_GetColorSpace()==0) && (Scaler_ModeGetModeInfo(SLR_MODE_IVFREQ)>600)) ) // HDMI via VGA card (HDMI && RGB && IVFreq>60Hz)
	{
		//USER:LewisLee DATE:2011/02/22
		//to prevent VGA have frame tear
		if(_TRUE == dclk_is_max)
		{
			drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
		}
		else// if(_FALSE == dclk_is_max)
		{
			if(_TRUE == Scaler_EnterFRC_LastLine_Condition())
			{
				drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
			}
			else// if(_FALSE == Scaler_EnterFRC_LastLine_Condition())
			{
				//if Vfreq large than 65Hz, need set it as FRC
				if(_TRUE == Scaler_Get_Customer_Force_FRC_Mode(_SRC_VGA))
				        drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
				else
					drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
			}
		}
	}
	else
#endif
#if 0 // marked unused code in merlin by ben_wang
	{
		if (dclk_is_max) // d-clk is the panel maximun,set frc timing
			drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
		else
			drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
	}
#endif
}


// decide it set stop memc command or not
void vbe_disp_set_stop_memc_push_flag(unsigned char bOnOff)
{
	rtd_pr_vbe_emerg("vbe_disp_set_stop_memc_push_flag = %d \n", bOnOff);
	vbe_stop_memc_cmd_flag = bOnOff;
}

unsigned char vbe_disp_get_stop_memc_push_flag(void)
{
	return vbe_stop_memc_cmd_flag;
}

// decide it's dynamic bypass memc flow, or not
void vbe_disp_set_dynamic_memc_bypass_flag(unsigned char bOnOff)
{
	vbe_dynamic_memc_bypass_flag = bOnOff;
}

unsigned char vbe_disp_get_dynamic_memc_bypass_flag(void)
{
	//rtd_pr_vbe_emerg("vbe_disp_get_dynamic_memc_bypass_flag = %d \n", vbe_dynamic_memc_bypass_flag);
	return vbe_dynamic_memc_bypass_flag;
}
#ifndef BUILD_QUICK_SHOW

unsigned char vbe_disp_decide_PQ_power_saveing(void)
{
	if(vbe_disp_get_adaptivestream_fs_mode()==TRUE)
		return FALSE;


	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)||(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA)||
		(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG)){

		//rtd_pr_vbe_debug("SLR_INPUT_IPH_ACT_WID_PRE = %d \n", Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE));
		//rtd_pr_vbe_debug("SLR_INPUT_IPV_ACT_LEN_PRE = %d \n", Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE));
		//rtd_pr_vbe_debug("SLR_INPUT_V_FREQ = %d \n", Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ));

		if((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) == 3840) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) == 2160) &&
			(((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)>495)&&(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)<505))||
			((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)>595)&&(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)<605)))){
			rtd_pr_vbe_notice("Enter PQ power saving mode\n");
			return TRUE;
		}
	}
	rtd_pr_vbe_notice("Disable PQ power saving mode\n");
	return FALSE;
}


unsigned char vbe_disp_decide_memc_mux_bypass(void)
{
#if 0
	if(vbe_disp_get_dynamic_memc_bypass_flag() == FALSE){
		// no memc dynamic bypass flow, return it.
		return FALSE;
	}

	if(vbe_disp_get_adaptivestream_fs_mode()==TRUE)
		return FALSE;

	if(Get_DISPLAY_REFRESH_RATE()==60){
		if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)||
			(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG)){

			//rtd_pr_vbe_debug("SLR_INPUT_IPH_ACT_WID_PRE = %d \n", Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE));
			//rtd_pr_vbe_debug("SLR_INPUT_IPV_ACT_LEN_PRE = %d \n", Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE));
			//rtd_pr_vbe_debug("SLR_INPUT_V_FREQ = %d \n", Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ));

			if((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) == 3840) &&
				(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) == 2160) &&
				((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)>595)&&(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)<605))){
				//rtd_pr_vbe_notice("Switch to MEMC BYPASS\n");
				return TRUE;
			}
		}
		//rtd_pr_vbe_notice("Switch to MEMC\n");
	}
#endif
	return FALSE;
}

unsigned char vbe_disp_decide_frc2fsync_using_I2D(void)
{
	if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245)){ //24
			return FALSE;
	}
	else if((get_vsc_src_is_hdmi_or_dp()) && (vbe_disp_get_VRR_timingMode_flag()|| vbe_disp_get_freesync_mode_flag() || get_scaler_qms_mode_flag())){//VRR
		return FALSE;
	}
	return TRUE;
}
#endif

unsigned char vbe_disp_decide_uzuclock_mapping_vo(void)
{
	unsigned char result = FALSE;

#if 0
	if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)){
		if( (Get_DISPLAY_REFRESH_RATE() == 60) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 48) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)>235)&&(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)<245))	// real cinema mode, uzuclock using 240M not 300M
		{
			result = TRUE;
		}else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)>245)&&(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)<255)) // 25hz uzuclock using 250M not 300M
		{
			result = TRUE;
		}
	}
#endif
	return result;
}
extern unsigned char drvif_color_Get_Two_Step_UZU_Mode(void);
static unsigned char st_dtg_lowdelay_mode;
#define SLOW_TUNNING_OFFSET 25
#define SLOW_TUNNING_OLED_OFFSET 2
#define SLOW_TUNING_DEFAULT_OFFSET 4

#ifndef BUILD_QUICK_SHOW


extern unsigned char Scaler_ME_CLK_Check(void);
extern UINT8 TV006_WB_Pattern_En;
void vbe_disp_game_mode_memc_bypass(unsigned char b_gameMode_onOff)
{
	unsigned char set_memc_bypass = 0;
//	unsigned char memc_forcebg_enable = 0;
	unsigned char force_cnt ;
	if(vbe_disp_get_dynamic_memc_bypass_flag() == FALSE){
		// no memc dynamic bypass flow, return it.
		return;
	}

	if(vbe_disp_decide_memc_mux_bypass())
		set_memc_bypass = 1;

	if(b_gameMode_onOff==1){
		if ((Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO) == 1) &&
		(Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO) == 1)){
			set_memc_bypass = 1;
		}
	}
#if 0	//Do not force MEMC on for Cloud game
	//sync from K3LP for CP case @Crixus 20170720
	if(get_vsc_run_adaptive_stream(SLR_MAIN_DISPLAY))
		set_memc_bypass = 0;
#endif
	//rtd_pr_vbe_emerg("(game=%d), set_memc_bypass=%d\n", b_gameMode_onOff, set_memc_bypass);

	if (TV006_WB_Pattern_En) set_memc_bypass = 1;	/*20181102 roger, when WB adjust, MEMC bypass for low-delay*/

	if(set_memc_bypass != Scaler_MEMC_get_Bypass_Status()){
#if 0//remove in k4lp @Crixus 20170530
		//only active mode can control MEMC force bg. Avoid DC on noise @Crixus 20161019
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE){
			Scaler_MEMC_output_force_bg_enable(_ENABLE);
			memc_forcebg_enable = 1;
		}
#endif
		if(set_memc_bypass){
			//need do memc bypass
			Scaler_MEMC_Bypass_On(1);
		}else{
//			if(Scaler_ME_CLK_Check() == TRUE)
                if(get_vt_EnableFRCMode() != FALSE)
				    Scaler_MEMC_Bypass_On(0);
		}

		//need to wait MEMC detect candence. @Crixus 20161007
		force_cnt = 15;
		while(force_cnt != 0){
			WaitFor_Frame_timer();
			force_cnt--;
		}
#if 0//remove in k4lp @Crixus 20170530
		//only active mode can control MEMC force bg. Avoid DC on noise @Crixus 20161019
		if(memc_forcebg_enable == 1){
			Scaler_MEMC_output_force_bg_enable(_DISABLE);
			memc_forcebg_enable = 0;
		}
#endif
	}
}
/*
mode 0 :
dtgm2goldenpostvs_line = DTGM_2_GOLDEN_VSYNC_DELAY_LINE
dtgm2uzuvs_line = DTGM_2_GOLDEN_VSYNC_DELAY_LINE
mode 1 :
dtgm2goldenpostvs_line = DTGM_2_GOLDEN_VSYNC_DELAY_LINE
dtgm2uzuvs_line = 0
mode 2:
dtgm2goldenpostvs_line = 0
dtgm2uzuvs_line = 0
*/


unsigned char vbe_disp_get_dtgM2uzu_lowdelay_mode(void)
{
	return st_dtg_lowdelay_mode;
}
#endif

extern VOID MEMC_Lib_Set_UltraLowDelayMode(unsigned char u1_enable);
void vbe_disp_set_dtgM2uzu_lowdelay_mode(unsigned char mode)
{
//#ifndef CONFIG_MEMC_BYPASS
	ppoverlay_uzudtg_control1_RBUS ppoverlay_uzudtg_control1_reg;
    ppoverlay_srnn_control_RBUS srnn_control_reg;
	ppoverlay_memcdtg_control3_RBUS ppoverlay_memcdtg_control3_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
    ppoverlay_dispd_smooth_toggle3_RBUS ppoverlay_dispd_smooth_toggle3_reg;
	ppoverlay_dv_total_RBUS ppoverlay_dv_total_reg;
	unsigned int timeout = 10;
	unsigned int recovery_vtotal = 0;

	unsigned int u32_dtgm2goldenpostvs_line = 0;
	unsigned int u32_dtgm2goldenpostvs_pixel = 0;
	unsigned int u32_dtgm2uzuvs_line = 0;
	unsigned int u32_dtgm2uzuvs_pixel = 0;
	unsigned char needUpdate = 0;
	unsigned int memc_goldenvs_delay_line = 0;
	unsigned int memc_goldenvs_delay_pixel = 0;
	unsigned char needSlow_tunning = 0;

	if((get_MEMC_bypass_status_refer_platform_model() == TRUE) && (mode != 2)) {
		return;
	}

	if(Get_DISPLAY_REFRESH_RATE() == 144){
		if( Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_innolux_4K2K_144HZ_2S_16L )
		{
			memc_goldenvs_delay_line = DTGM_2_GOLDEN_VSYNC_DELAY_LINE_INX_144PANEL;
			memc_goldenvs_delay_pixel = DTGM_2_GOLDEN_VSYNC_DELAY_PIXEL_INX_144PANEL;
		}
		else
		{
			memc_goldenvs_delay_line = DTGM_2_GOLDEN_VSYNC_DELAY_LINE_144PANEL;
			memc_goldenvs_delay_pixel = DTGM_2_GOLDEN_VSYNC_DELAY_PIXEL_144PANEL;
		}
	}
	else if(Get_DISPLAY_REFRESH_RATE() == 120){
        if(get_panel_pixel_mode() > PANEL_1_PIXEL)
        {
            memc_goldenvs_delay_line = DTGM_2_GOLDEN_VSYNC_DELAY_LINE_120PANEL;
            memc_goldenvs_delay_pixel = DTGM_2_GOLDEN_VSYNC_DELAY_PIXEL;
        }
        else
        {
            memc_goldenvs_delay_line = DTGM_2_GOLDEN_VSYNC_DELAY_4k1k_LINE;
            memc_goldenvs_delay_pixel = DTGM_2_GOLDEN_VSYNC_DELAY_PIXEL;
        }
	}
	else{
		memc_goldenvs_delay_line = DTGM_2_GOLDEN_VSYNC_DELAY_LINE;
		memc_goldenvs_delay_pixel = DTGM_2_GOLDEN_VSYNC_DELAY_PIXEL;
	}
	st_dtg_lowdelay_mode = mode;
	if(Get_TIMING_PROTECT_PANEL_TYPE()){
		if(mode == 2){
			rtd_pr_vbe_notice("LGD LD32 panel, cannot set mode2, due to timing change make panel flicker.\n");
			mode=0;
		}
	}
	if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)){
		if(mode == 0){
			rtd_pr_vbe_notice("[%s]Warning.Data Fsync cannot set mode 0.\n",__FUNCTION__);
		}
	}

	if(mode == 0){
		//set goldenvsync
		u32_dtgm2goldenpostvs_line = memc_goldenvs_delay_line;
		u32_dtgm2goldenpostvs_pixel = memc_goldenvs_delay_pixel;
		//set uzu1 delay
		u32_dtgm2uzuvs_line = memc_goldenvs_delay_line;
		u32_dtgm2uzuvs_pixel = memc_goldenvs_delay_pixel-STAGE1_STAGE2_FIFO_MARGIN;
	}
	else if(mode == 1){
		//set goldenvsync
		u32_dtgm2goldenpostvs_line = memc_goldenvs_delay_line;
		u32_dtgm2goldenpostvs_pixel = memc_goldenvs_delay_pixel;
		//set uzu1 delay
		if(drvif_color_Get_Two_Step_UZU_Mode()){
			u32_dtgm2uzuvs_line = TWO_STEP_UZU_ON_OFF_DELAY_LINE;
			u32_dtgm2uzuvs_pixel = memc_goldenvs_delay_pixel-STAGE1_STAGE2_FIFO_MARGIN;
		}else{
			u32_dtgm2uzuvs_line = 0;
			u32_dtgm2uzuvs_pixel = memc_goldenvs_delay_pixel-STAGE1_STAGE2_FIFO_MARGIN;
		}
	}
	else if(mode == 2){
		//set goldenvsync
		u32_dtgm2goldenpostvs_line = 0;
		u32_dtgm2goldenpostvs_pixel = memc_goldenvs_delay_pixel;
		//set uzu1 delay
		if(drvif_color_Get_Two_Step_UZU_Mode()){
			u32_dtgm2goldenpostvs_line = TWO_STEP_UZU_ON_OFF_DELAY_LINE;
			u32_dtgm2uzuvs_line = TWO_STEP_UZU_ON_OFF_DELAY_LINE;
			u32_dtgm2uzuvs_pixel = memc_goldenvs_delay_pixel-STAGE1_STAGE2_FIFO_MARGIN;
		}else{
			u32_dtgm2uzuvs_line = 0;
			u32_dtgm2uzuvs_pixel = memc_goldenvs_delay_pixel-STAGE1_STAGE2_FIFO_MARGIN;
		}
	}
	else if(mode == 3){
		//set goldenvsync
		if((Get_DISPLAY_PANEL_OLED_TYPE()==TRUE)/*&&(Scaler_check_orbit_store_mode() == TRUE)*/)
			u32_dtgm2goldenpostvs_line = DTGM_2_GOLDEN_VSYNC_DELAY_LINE_GAMEMODE_OLED;
		else
        {
			u32_dtgm2goldenpostvs_line = DTGM_2_GOLDEN_VSYNC_DELAY_LINE_GAMEMODE;
            if((Get_DISPLAY_REFRESH_RATE() >= 120) && (get_panel_pixel_mode() < PANEL_1_PIXEL))
            {
                u32_dtgm2goldenpostvs_line = DTGM_2_GOLDEN_VSYNC_DELAY_LINE_4k1k_GAMEMODE;
            }
        }
		u32_dtgm2goldenpostvs_pixel = memc_goldenvs_delay_pixel;
		//set uzu1 delay
		u32_dtgm2uzuvs_line = 0;
		u32_dtgm2uzuvs_pixel = memc_goldenvs_delay_pixel-STAGE1_STAGE2_FIFO_MARGIN;
	}
	else{
		//set goldenvsync
		u32_dtgm2goldenpostvs_line = memc_goldenvs_delay_line;
		u32_dtgm2goldenpostvs_pixel = memc_goldenvs_delay_pixel;
		//set uzu1 delay
		u32_dtgm2uzuvs_line = memc_goldenvs_delay_line;
		u32_dtgm2uzuvs_pixel = memc_goldenvs_delay_pixel-STAGE1_STAGE2_FIFO_MARGIN;
	}

	//check if need update or not.
	ppoverlay_memcdtg_control3_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL3_reg);
	ppoverlay_uzudtg_control1_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control1_reg);

	rtd_pr_vbe_notice("compare 1.[(%x.%x),(%x.%x)] 3.[(%x.%x),(%x.%x)]\n",
		ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line, u32_dtgm2goldenpostvs_line,
		ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_pixel,u32_dtgm2goldenpostvs_pixel,
		ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line,     u32_dtgm2uzuvs_line,
		ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_pixel,    u32_dtgm2uzuvs_pixel);

	if(
	   ((ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line  != u32_dtgm2goldenpostvs_line)||
	    (ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_pixel != u32_dtgm2goldenpostvs_pixel)) ||
	   ((ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line != u32_dtgm2uzuvs_line)||
	    (ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_pixel != u32_dtgm2uzuvs_pixel))
	  )
	{
		needUpdate = 1;
	}


	if(needUpdate){
		ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 1;
		ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

        //RL6856-1055 bring up need set this bit = 1
        ppoverlay_dispd_smooth_toggle3_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle3_reg);
        ppoverlay_dispd_smooth_toggle3_reg.dispd_memc_db5_smt_en = 1;
        IoReg_Write32(PPOVERLAY_dispd_smooth_toggle3_reg, ppoverlay_dispd_smooth_toggle3_reg.regValue);

        MEMC_Lib_Set_UltraLowDelayMode(TRUE);
		//if ((webos_tooloption.eModelModuleType == module_LGD) && (webos_tooloption.eBackLight == direct ) && (webos_tooloption.eLEDBarType == local_dim_block_32) ){
#if 0
				Scaler_MEMC_ForceBg(1); //use memc force bg for change uzu delay garbage
				rtd_pr_vbe_notice("Use memc forcebg for uzudelay garbage.\n");
				//double buffer apply
				ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
				ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set= 1;
				IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
				while((--timeout)  &&
				(ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set == 1)){
					msleep(0);
					ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
				}
				timeout = 10;
#endif
/*
		ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line = u32_dtgm2goldenpostvs_line;
		ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_pixel = u32_dtgm2goldenpostvs_pixel;
		ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line = u32_dtgm2uzuvs_line;
		ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_pixel = u32_dtgm2uzuvs_pixel;
		IoReg_Write32(PPOVERLAY_uzudtg_control1_reg, ppoverlay_uzudtg_control1_reg.regValue);
		IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL3_reg, ppoverlay_memcdtg_control3_reg.regValue);
*/
		//if ((webos_tooloption.eModelModuleType == module_LGD) && (webos_tooloption.eBackLight == direct ) && (webos_tooloption.eLEDBarType == local_dim_block_32)){
		if(0){
				; //specific panel can not change large Vtotal
		}else{
			if((mode == 1)||(mode == 2)||(mode == 3)){
				ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
				ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 0;
				IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
				ppoverlay_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
				recovery_vtotal = ppoverlay_dv_total_reg.dv_total;
				if(!PPOVERLAY_Display_Timing_CTRL1_get_disp_fsync_en(IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg))){
					ppoverlay_dv_total_reg.dv_total += memc_goldenvs_delay_line;
					IoReg_Write32(PPOVERLAY_DV_total_reg, ppoverlay_dv_total_reg.regValue);
				}else if (!((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA) && vsc_get_adaptivestream_flag(SLR_MAIN_DISPLAY))){
					needSlow_tunning = 1;
				}
			}
		}

		if(needSlow_tunning){
			unsigned int loop_count = 0;
			unsigned char slow_tunning_offset = 0;
			unsigned int i;
			unsigned char b_goldenvs_tunning_done = 0;
			unsigned char b_uzudly_tunning_done = 0;			
            if(Get_DISPLAY_PANEL_OLED_TYPE())
            {
                slow_tunning_offset = SLOW_TUNNING_OLED_OFFSET;
            }
            else
            {
                slow_tunning_offset = SLOW_TUNNING_OFFSET;
            }

			//for coverity check, (DTGM_2_GOLDEN_VSYNC_DELAY_LINE%slow_tunning_offset) always be true.
			//loop_count = (DTGM_2_GOLDEN_VSYNC_DELAY_LINE%slow_tunning_offset)?(DTGM_2_GOLDEN_VSYNC_DELAY_LINE/slow_tunning_offset+1):(DTGM_2_GOLDEN_VSYNC_DELAY_LINE/slow_tunning_offset);
			loop_count = (DTGM_2_GOLDEN_VSYNC_DELAY_LINE/slow_tunning_offset+1);

			ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
			ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 1;
			ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;
			ppoverlay_double_buffer_ctrl2_reg.uzudtg_dbuf_vsync_sel = 1;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

			for(i=0; i<loop_count; i++){
				extern unsigned judge_scaler_break_case(unsigned char display);
				unsigned int b_judge_break = judge_scaler_break_case(0);

				ppoverlay_memcdtg_control3_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL3_reg);
				ppoverlay_uzudtg_control1_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control1_reg);
                srnn_control_reg.regValue = IoReg_Read32(PPOVERLAY_srnn_control_reg);

				if(ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line > u32_dtgm2goldenpostvs_line){
                    if(ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line < slow_tunning_offset)
                    {
                        update_memc_vtrig_delay(0, ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line);
                        ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line = 0;
                    }
                    else
                    {
                        ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line -= slow_tunning_offset;
                        update_memc_vtrig_delay(0, slow_tunning_offset);
                    }
                    IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL3_reg, ppoverlay_memcdtg_control3_reg.regValue);
				}else{
					b_goldenvs_tunning_done = 1;
				}

				if(ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line > u32_dtgm2uzuvs_line){
					if(ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line < slow_tunning_offset)
						ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line = 0;
					else
						ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line -= slow_tunning_offset;
					IoReg_Write32(PPOVERLAY_uzudtg_control1_reg, ppoverlay_uzudtg_control1_reg.regValue);

                    srnn_control_reg.dtgm2mdom_pre_vs_line = ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line;
                    IoReg_Write32(PPOVERLAY_srnn_control_reg, srnn_control_reg.regValue);
				}else{
					b_uzudly_tunning_done = 1;
				}

				if(b_goldenvs_tunning_done && b_uzudly_tunning_done){
					break;
				}

				ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
				ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set= 1;
				ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set= 1;
				IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
				timeout = 10;
				while(!b_judge_break && (--timeout)&&((ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set == 1)||(ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set == 1))){
					msleep(0);
					ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
				}

				if(b_judge_break){
					rtd_pr_vbe_notice("[%s]_slow tunning break.\n", __FUNCTION__);
					return;
				}

				rtd_pr_vbe_notice("[%s]_slow tunning (Loop:%d, i:%d),(UZUDTG_CONTROL1=%x)(MEMCDTG_CONTROL3=%x)\n", __FUNCTION__, loop_count, i, IoReg_Read32(PPOVERLAY_uzudtg_control1_reg),IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL3_reg));
			}

            if(ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line >= u32_dtgm2goldenpostvs_line)
            {
                update_memc_vtrig_delay(0, (ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line - u32_dtgm2goldenpostvs_line));
            }
            else
            {
                update_memc_vtrig_delay(1, (u32_dtgm2goldenpostvs_line - ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line));
            }

			ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line = u32_dtgm2goldenpostvs_line;
			ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_pixel = u32_dtgm2goldenpostvs_pixel;
			ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line = u32_dtgm2uzuvs_line;
			ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_pixel = u32_dtgm2uzuvs_pixel;
			IoReg_Write32(PPOVERLAY_uzudtg_control1_reg, ppoverlay_uzudtg_control1_reg.regValue);
			IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL3_reg, ppoverlay_memcdtg_control3_reg.regValue);

            srnn_control_reg.dtgm2mdom_pre_vs_line = ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line;
            srnn_control_reg.dtgm2mdom_pre_vs_pixel = ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_pixel;
            IoReg_Write32(PPOVERLAY_srnn_control_reg, srnn_control_reg.regValue);

			//double buffer apply
			ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
			ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set= 1;
			ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set= 1;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
			while((--timeout)  &&
			((ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set == 1)||
			 (ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set == 1))){
			 	msleep(0);
				ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
			}
			if(Get_TIMING_PROTECT_PANEL_TYPE()){
					extern unsigned char Scaler_MEMC_Get_First_Run_FBG_enable(void);
					extern unsigned char Scaler_MEMC_Get_instanboot_resume_Done(void);
					extern unsigned char Scaler_MEMC_Get_snashop_resume_Done(void);
					; //specific panel can not change large Vtotal
					if(!(Scaler_MEMC_Get_First_Run_FBG_enable() || Scaler_MEMC_Get_instanboot_resume_Done() || Scaler_MEMC_Get_snashop_resume_Done() /*|| get_WB_MEMC_forcebg_flag()*/))
							Scaler_MEMC_ForceBg(0);//boot case will be control memc mute off after memc ready
			}
		}
		else{
            if(ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line >= u32_dtgm2goldenpostvs_line)
            {
                update_memc_vtrig_delay(0, (ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line - u32_dtgm2goldenpostvs_line));
            }
            else
            {
                update_memc_vtrig_delay(1, (u32_dtgm2goldenpostvs_line - ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line));
            }
			ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line = u32_dtgm2goldenpostvs_line;
			ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_pixel = u32_dtgm2goldenpostvs_pixel;
			ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line = u32_dtgm2uzuvs_line;
			ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_pixel = u32_dtgm2uzuvs_pixel;
			IoReg_Write32(PPOVERLAY_uzudtg_control1_reg, ppoverlay_uzudtg_control1_reg.regValue);
			IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL3_reg, ppoverlay_memcdtg_control3_reg.regValue);

            srnn_control_reg.dtgm2mdom_pre_vs_line = ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line;
            srnn_control_reg.dtgm2mdom_pre_vs_pixel = ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_pixel;
            IoReg_Write32(PPOVERLAY_srnn_control_reg, srnn_control_reg.regValue);

			//double buffer apply
			ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
			ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set= 1;
			ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set= 1;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
			while((--timeout)  &&
			((ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set == 1)||
			 (ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set == 1))){
			 	msleep(0);
				ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
			}
			Scaler_wait_for_LCM_done();

			//if ((webos_tooloption.eModelModuleType == module_LGD) && (webos_tooloption.eBackLight == direct ) && (webos_tooloption.eLEDBarType == local_dim_block_32)){
			if(Get_TIMING_PROTECT_PANEL_TYPE()){
					extern unsigned char Scaler_MEMC_Get_First_Run_FBG_enable(void);
					extern unsigned char Scaler_MEMC_Get_instanboot_resume_Done(void);
					extern unsigned char Scaler_MEMC_Get_snashop_resume_Done(void);
					; //specific panel can not change large Vtotal
					if(!(Scaler_MEMC_Get_First_Run_FBG_enable() || Scaler_MEMC_Get_instanboot_resume_Done() || Scaler_MEMC_Get_snashop_resume_Done() /*|| get_WB_MEMC_forcebg_flag()*/))
							Scaler_MEMC_ForceBg(0);//boot case will be control memc mute off after memc ready
			}else{
				unsigned int pre_dreg_db = 0;
				if(((mode == 1)||(mode == 2)||(mode == 3))
					&& !PPOVERLAY_Display_Timing_CTRL1_get_disp_fsync_en(IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg))){
					//disable db to make dvtotal apply directly
					ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
					pre_dreg_db = ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en;
					ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 0;
					IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
					ppoverlay_dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
					ppoverlay_dv_total_reg.dv_total = recovery_vtotal;
					IoReg_Write32(PPOVERLAY_DV_total_reg, ppoverlay_dv_total_reg.regValue);
					//recover db after dvtotal
					ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
					ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = pre_dreg_db;
					IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
				}
			}
		}
		rtd_pr_vbe_notice("[%s] mode=%d. (goldenv: %x, uzu_ctrl1:%x)\n", __FUNCTION__, mode,
			IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL3_reg), IoReg_Read32(PPOVERLAY_uzudtg_control1_reg));

        //RL6856-1055
        ppoverlay_dispd_smooth_toggle3_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle3_reg);
        ppoverlay_dispd_smooth_toggle3_reg.dispd_memc_db5_smt_en = 0;
        IoReg_Write32(PPOVERLAY_dispd_smooth_toggle3_reg, ppoverlay_dispd_smooth_toggle3_reg.regValue);

        MEMC_Lib_Set_UltraLowDelayMode(FALSE);
	}
	else{
		rtd_pr_vbe_notice("[%s] no need update.\n", __FUNCTION__);
	}
//#endif //CONFIG_MEMC_BYPASS
}

/**
adjust memc delay from b8028644
*/
void update_memc_vtrig_delay(unsigned char inCrease, unsigned int step)
{
    unsigned int u32_FRC_Vtrig_dly = 0;
    unsigned int u32MC_Vtrig_dly_long = 0;
    ppoverlay_dispd_smooth_toggle3_RBUS ppoverlay_dispd_smooth_toggle3_reg;

    ppoverlay_dispd_smooth_toggle3_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle3_reg);

    if(!ppoverlay_dispd_smooth_toggle3_reg.dispd_memc_db5_smt_en)
    {
        return;
    }

    if(inCrease)
    {
        u32_FRC_Vtrig_dly = HAL_CRTC_GetFRCVtrigDelay();
        u32_FRC_Vtrig_dly += step;

        u32MC_Vtrig_dly_long = HAL_CRTC_GetMCVtrigDelayLong();
        u32MC_Vtrig_dly_long += step;
    }
    else
    {
        u32_FRC_Vtrig_dly = HAL_CRTC_GetFRCVtrigDelay();
        u32MC_Vtrig_dly_long = HAL_CRTC_GetMCVtrigDelayLong();

        if(u32_FRC_Vtrig_dly >= step)
        {
            u32_FRC_Vtrig_dly -= step;
        }
        else
        {
            u32_FRC_Vtrig_dly = 0;
        }

        if(u32MC_Vtrig_dly_long >= step)
        {
            u32MC_Vtrig_dly_long -= step;
        }
        else
        {
            u32MC_Vtrig_dly_long = 0;
        }
    }

    rtd_pr_vbe_notice("[%s %d]u32_FRC_Vtrig_dly:%d, u32MC_Vtrig_dly_long:%d\n", __FUNCTION__, __LINE__, u32_FRC_Vtrig_dly, u32MC_Vtrig_dly_long);

    HAL_CRTC_SetFRCVtrigDelay(u32_FRC_Vtrig_dly);
    HAL_CRTC_SetMCVtrigDelayLong(u32MC_Vtrig_dly_long);

}

static unsigned int vbe_HDMI_dynamic_film_mode_flag;

void vbe_set_HDMI_dynamic_film_mode_flag(unsigned int status)
{
	vbe_HDMI_dynamic_film_mode_flag = status;
}

unsigned int vbe_get_HDMI_dynamic_film_mode_flag(void)
{
	return vbe_HDMI_dynamic_film_mode_flag;
}

#ifndef BUILD_QUICK_SHOW

void UpdateDataFsAlgoRsult(void)
{
	int ret;
	unsigned int i = 0, ulItemCount = 0;
	unsigned int *pulTemp;

	pulTemp = (unsigned int *)(&g_tDataFsAlgoResult);

	ulItemCount = sizeof(SCALER_DATA_FS_ALGO_RESULT) / sizeof(unsigned int);
	// change endian
	for (i = 0; i < ulItemCount; i++)
		pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);

	// copy from RPC share memory
	memcpy((unsigned char*)Scaler_GetShareMemVirAddr(SCALERIOC_DTATA_FS_ALGO_RESULT), pulTemp, sizeof(SCALER_DATA_FS_ALGO_RESULT));

	if ((ret = Scaler_SendRPC(SCALERIOC_DTATA_FS_ALGO_RESULT, 0, 0)) < 0)
	{
		rtd_pr_vbe_emerg("ret = %d, set SCALERIOC_DTATA_FS_ALGO_RESULT to video fw fail !!!\n", ret );
		return;
	}

}

void ClearDataFsAlgoRsult(void)
{
	memset(&g_tDataFsAlgoResult, 0, sizeof(SCALER_DATA_FS_ALGO_RESULT));
	UpdateDataFsAlgoRsult();
}
#endif

unsigned int VSYNCGEN_get_vodma_clk_0(void)
{
	unsigned int vodmapll;
	unsigned int nMCode, nNCode, nOCode, regValue, busclk = 0;
	unsigned int divider_num = 0, ratio_n_off = 0;

	vodma_vodma_clkgen_RBUS vodma_clkgen_reg;
	vodma_clkgen_reg.regValue=rtd_inl(VODMA_VODMA_CLKGEN_reg);


	regValue = rtd_inl(PLL_REG_SYS_PLL_VODMA1_reg);

	nMCode = ((regValue & 0x000ff000)>>12);
	nNCode = ((regValue & 0x00300000)>>20);
	nOCode = ((regValue & 0x00000180)>>7);

	busclk = 27000000*(nMCode+2)/(nNCode+1)/(nOCode+1);

	divider_num = vodma_clkgen_reg.vodma_clk_div_n;
	ratio_n_off = vodma_clkgen_reg.vodma_clk_ratio_n_off;

#if 1//def CONFIG_REALTEK_2K_MODEL_ENABLED
	vodmapll = busclk / (divider_num+1);

	vodmapll = (vodmapll/64) * (64 - ratio_n_off);
#else
	vodmapll = busclk;
#endif
	if(vodma_clkgen_reg.vodma_clk_div2_en){//interlace should div2
		vodmapll = vodmapll / 2;
		rtd_pr_vbe_emerg("vo clk div2 for interlace timing\n");
	}
	rtd_pr_vbe_emerg("[vo] vsyncgen busclk: %d/%d/%d, divider_num=%d, ratio_n_off=%d\n", nMCode, nNCode, busclk, divider_num, ratio_n_off);

	return vodmapll;
}
#ifndef BUILD_QUICK_SHOW

void vbe_disp_set_print_debug_line_count(unsigned char b_paneltype)
{
  vbe_print_debug_line_count = b_paneltype;
}

unsigned char vbe_disp_get_print_debug_line_count(void)
{
	return vbe_print_debug_line_count;
}

void vbe_disp_bringup_vbe_test(unsigned char timingCase)
{
	extern unsigned int idomain_vfreq_pre;
	if(timingCase==60){
		vbe_bringup_test_case = 1;
		Scaler_DispSetInputInfo(SLR_INPUT_V_FREQ, 600);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
	}else if(timingCase==24){
		vbe_bringup_test_case = 1;
		Scaler_DispSetInputInfo(SLR_INPUT_V_FREQ, 240);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 2);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 5);
	}else if(timingCase==30){
		vbe_bringup_test_case = 1;
		Scaler_DispSetInputInfo(SLR_INPUT_V_FREQ, 300);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
	}else if(timingCase==25){
		vbe_bringup_test_case = 1;
		Scaler_DispSetInputInfo(SLR_INPUT_V_FREQ, 250);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 2);
	}else if(timingCase==50){
		vbe_bringup_test_case = 1;
		Scaler_DispSetInputInfo(SLR_INPUT_V_FREQ, 500);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);
		Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO, 1);
	}else{
		vbe_bringup_test_case = 0;
	}

	if(vbe_bringup_test_case){
		idomain_vfreq_pre = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
		modestate_decide_pixel_mode();
		modestate_decide_dtg_m_mode();
		modestate_decide_fractional_framesync();
		scaler_disp_dtg_pre_i_domain_setting(0);
		Scaler_disp_setting(0);
	}
}
#endif

unsigned char vbe_disp_get_bringup_vbe_test(void)
{
	return vbe_bringup_test_case;
}

unsigned int vbe_get_VDEC4K_run_datafs_without_gatting_condition(void){
	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA)
		&& (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) > DATAFS_DISABLE_GATTING_WIDTH)
		&& (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) > DATAFS_DISABLE_GATTING_HEIGHT))
		return 1;
	else
		return 0;
}

unsigned int vbe_get_VDEC4K_run_datafs_condition(void){
#ifndef BUILD_QUICK_SHOW
	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA)
		&& ((get_panel_res() != PANEL_RES_FHD)&&(get_panel_res() != PANEL_RES_HD))
		&& (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) > DATAFS_VDEC_RUN_DATAFS_WIDTH)
		&& (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) > DATAFS_VDEC_RUN_DATAFS_HEIGHT)
		&&((get_panel_pixel_mode() > PANEL_1_PIXEL)||Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ)<990))
		return 1;
	else
		return 0;
#else
    return 0;
#endif
}
/* This function is for decide run timing sync  at scaler initialize
 * if we need to line buffer mode in the future, it need to return true.
 * */
unsigned int vbe_get_HDMI_run_timing_framesync_condition(void){
	if(((get_vsc_src_is_hdmi_or_dp())
        &&(get_force_i3ddma_enable(SLR_MAIN_DISPLAY))
		&& (get_current_driver_pattern(DRIVER_FREERUN_PATTERN) == 1)
		&& (is_game_mode_set_line_buffer() || !drv_memory_get_game_mode_flag())//game mode and can use line buffer mode for full panel
		&& (get_hdmi_4k_hfr_mode() == HDMI_NON_4K120)
		&& (scaler_vsc_get_gamemode_go_datafrc_mode()==FALSE)) //game mode go data frc case cannot increase hdmi clock
		|| (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())
	)
		return 1;
	else
		return 0;
}
#ifndef BUILD_QUICK_SHOW

/* This function is for judge for run data fs in HDMI */
unsigned int vbe_get_HDMI_run_datafs_condition(void){
	if((get_vsc_src_is_hdmi_or_dp())
		&& (get_force_i3ddma_enable(SLR_MAIN_DISPLAY))
		&& (get_current_driver_pattern(DRIVER_FREERUN_PATTERN) == 1)
		&& (get_hdmi_4k_hfr_mode() == HDMI_NON_4K120)
		&& !(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag()))
		return 1;
	else
		return 0;
}
#endif
void scaler_disp_dtg_set_align_output_timing(unsigned char bEnable)
{
	//1: align output, 0: align input
	ppoverlay_uzudtg_control3_RBUS ppoverlay_uzudtg_control3_reg;
	ppoverlay_uzudtg_orbit_debug_ctrl0_RBUS ppoverlay_uzudtg_orbit_debug_ctrl0_reg;

    ppoverlay_uzudtg_control3_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control3_reg);
	ppoverlay_uzudtg_control3_reg.align_output_timing = bEnable;
	IoReg_Write32(PPOVERLAY_uzudtg_control3_reg, ppoverlay_uzudtg_control3_reg.regValue);
	if(Get_DISPLAY_PANEL_OLED_TYPE() == TRUE)
	{//orbit line delay sw mode will affect by align output timing
	 //when align input => sw mode =1 , align output => sw mode =0
		ppoverlay_uzudtg_orbit_debug_ctrl0_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_orbit_debug_ctrl0_reg);
		ppoverlay_uzudtg_orbit_debug_ctrl0_reg.orbit_line_delay_sw_mode = !bEnable;
		IoReg_Write32(PPOVERLAY_uzudtg_orbit_debug_ctrl0_reg,ppoverlay_uzudtg_orbit_debug_ctrl0_reg.regValue);
	}
	rtd_pr_vbe_notice("align output timing=%d\n", bEnable);
}

static unsigned char judge_set_s1ip_run_slow_tuning(void)
{
    if((Get_DISP_VERTICAL_TOTAL() - Get_DISP_DEN_END_VPOS()) < DTG_DEFAULT_S1IP_LINE_DELAY)
    {
        return TRUE;
    }

    return FALSE;
}

void wait_for_uzu_dtg_db_apply_success(void)
{
    unsigned int timeout = 10;
    ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;

    timeout = 10;

    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
    ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set =1;
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

    while(ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set && --timeout)
    {
        usleep_range(10  * 1000, 10* 1000);

        ppoverlay_double_buffer_ctrl2_reg.regValue=IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
    }
}

void wait_for_uzu_memc_dtg_db_apply_success(void)
{
    unsigned int timeout = 10;
    unsigned int count = 0xffff;
    unsigned start_line_cnt = Get_DISP_DEN_END_VPOS() + 5;
    unsigned end_line_cnt = Get_DISP_VERTICAL_TOTAL() - 5;
    ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;

    count = 0xfffff;
    timeout = 10;

    //wait for front porch
    while(!((PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)) > start_line_cnt) &&
                (PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)) < end_line_cnt)) && --count){
    }

    if(count == 0)
    {
        rtd_pr_vbe_notice("[%s %d]waitin for front porch timeout\n", __FUNCTION__, __LINE__);
    }

    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
    ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set =1;
    ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set = 1;
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

    while((ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set || ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set) && --timeout)
    {
        usleep_range(10  * 1000, 10* 1000);
        ppoverlay_double_buffer_ctrl2_reg.regValue=IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
    }

    if(timeout == 0)
    {
        rtd_pr_vbe_notice("[%s %d]uzudtg db, memcdtg db apply timeout\n", __FUNCTION__, __LINE__);
    }

}

/*
	// game mode+memc bypass / vrr case
		time borrow 0, s1ip delay 0 , hw auto calculate
	// game mode+ultra lowdelay case
		time borrow 0, s1ip delay 6
	// normal game mode
		time borrow 0x15, s1ip delay 0x2b
*/
void scaler_disp_dtg_set_s1ip_dleay(unsigned char bEnable_DB)
{
    ppoverlay_uzudtg_control3_RBUS ppoverlay_uzudtg_control3_reg;
    ppoverlay_uzudtg_stage1_time_borrow_RBUS ppoverlay_uzudtg_stage1_time_borrow_reg;
    ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
    ppoverlay_memcdtg_control3_RBUS ppoverlay_memcdtg_control3_reg;
    ppoverlay_uzudtg_control4_RBUS ppoverlay_uzudtg_control4_reg;
    unsigned int s1ip_delay = 0;
    unsigned int slow_tuning_step = 0;
    unsigned int time_borrow = 0;
    unsigned int time_borrow_modify = 0;
    unsigned int s1ip_delay_modify = 0;
    unsigned int dvstart = Get_DISP_DEN_STA_VPOS();

    unsigned int b_isMEMCypass = (PPOVERLAY_MEMC_MUX_CTRL_get_memc_outmux_sel(IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg))==0)?1:0;
    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);

    if(bEnable_DB)
    {
        ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;
        ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 1;
        ppoverlay_double_buffer_ctrl2_reg.uzudtg_dbuf_vsync_sel = 2;
        ppoverlay_double_buffer_ctrl2_reg.memcdtg_dbuf_vsync_sel = 1;

        IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
    }

    rtd_pr_vbe_notice("game=%d, vrr=%d, bypass=%d, ultra judge=%d\n", drv_memory_get_game_mode(),(vbe_disp_get_VRR_timingMode_flag()||vbe_disp_get_freesync_mode_flag()),
        b_isMEMCypass, Scaler_MEMC_Lib_UltraLowDelayMode_Judge());

    time_borrow = scaler_disp_dtg_get_time_borrow_value();
    s1ip_delay = scaler_disp_dtg_get_s1ip_delay_value();

    ppoverlay_uzudtg_stage1_time_borrow_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_stage1_time_borrow_reg);
    ppoverlay_uzudtg_control3_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control3_reg);
    ppoverlay_uzudtg_control4_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control4_reg);

    rtd_pr_vbe_notice("orign stage1_time_borrow=%d, adjust time_borrow:%d, orign stage1_s1ip_delay:%d, adjust s1ip_delay:%d\n", ppoverlay_uzudtg_stage1_time_borrow_reg.stage1_time_borrow_v, time_borrow, ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay, s1ip_delay);

    ppoverlay_uzudtg_stage1_time_borrow_reg.stage1_time_borrow_sw_mode = 0;
    ppoverlay_uzudtg_control3_reg.s1ip_line_delay_sw_mode = 0;
    ppoverlay_uzudtg_control4_reg.db_latch_line = dvstart - ppoverlay_uzudtg_stage1_time_borrow_reg.stage1_time_borrow_v;

    time_borrow_modify = ABS(ppoverlay_uzudtg_stage1_time_borrow_reg.stage1_time_borrow_v, time_borrow);
    s1ip_delay_modify = ABS(ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay, s1ip_delay);

    if(s1ip_delay_modify > time_borrow_modify)
    {
        //decrease
        if(time_borrow < ppoverlay_uzudtg_stage1_time_borrow_reg.stage1_time_borrow_v)
        {
            ppoverlay_uzudtg_stage1_time_borrow_reg.stage1_time_borrow_v -= time_borrow_modify;
            ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay -= time_borrow_modify;
        }
        else
        {
            ppoverlay_uzudtg_stage1_time_borrow_reg.stage1_time_borrow_v += time_borrow_modify;
            ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay += time_borrow_modify;
        }
    }
    else
    {
        //decrease
        if(s1ip_delay < ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay)
        {
            ppoverlay_uzudtg_stage1_time_borrow_reg.stage1_time_borrow_v -= s1ip_delay_modify;
            ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay -= s1ip_delay_modify;
        }
        else
        {
            ppoverlay_uzudtg_stage1_time_borrow_reg.stage1_time_borrow_v += s1ip_delay_modify;
            ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay += s1ip_delay_modify;
        }
    }

    //need update db latch line
    IoReg_Write32(PPOVERLAY_uzudtg_control4_reg, ppoverlay_uzudtg_control4_reg.regValue);
    IoReg_Write32(PPOVERLAY_uzudtg_stage1_time_borrow_reg, ppoverlay_uzudtg_stage1_time_borrow_reg.regValue);
    IoReg_Write32(PPOVERLAY_uzudtg_control3_reg, ppoverlay_uzudtg_control3_reg.regValue);

    if(bEnable_DB)
    {
        wait_for_uzu_dtg_db_apply_success();
    }

    ppoverlay_uzudtg_control3_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control3_reg);
    ppoverlay_memcdtg_control3_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL3_reg);

    if((Get_DISPLAY_PANEL_OLED_TYPE() == TRUE) || (judge_set_s1ip_run_slow_tuning() == TRUE))
    {
        if(s1ip_delay < ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay)
        {
            while(s1ip_delay != ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay)
            {
                if(Get_DISPLAY_PANEL_OLED_TYPE() == TRUE)
                {
                    slow_tuning_step = SLOW_TUNNING_OLED_OFFSET;
                }
                else
                {
                    slow_tuning_step = SLOW_TUNING_DEFAULT_OFFSET;
                }

                if((ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay-s1ip_delay) > slow_tuning_step)
                {
                    ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay -= slow_tuning_step;

                    ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line += slow_tuning_step;
                }
                else
                {
                    ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line += (ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay - s1ip_delay);
                    ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay = s1ip_delay;
                }

                rtd_pr_vbe_notice("[%s]slow tuning s1ip_max_line_delay=%d, ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line:%d\n",__FUNCTION__, ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay, ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line);

                IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL3_reg, ppoverlay_memcdtg_control3_reg.regValue);
                IoReg_Write32(PPOVERLAY_uzudtg_control3_reg, ppoverlay_uzudtg_control3_reg.regValue);

                if(bEnable_DB)
                {
                    wait_for_uzu_memc_dtg_db_apply_success();
                }

                ppoverlay_uzudtg_control3_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control3_reg);
                ppoverlay_memcdtg_control3_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL3_reg);
            }
        }
        else
        {
            ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line -= (s1ip_delay - ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay);
            ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay = s1ip_delay;

            rtd_pr_vbe_notice("s1ip_delay_line=%d, ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line:%d\n", ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay, ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line);
        }
    }
    else
    {
        if(ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay > s1ip_delay)
        {
            ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line += (ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay - s1ip_delay);
        }
        else
        {
            ppoverlay_memcdtg_control3_reg.dtgm2goldenpostvs_line -= (s1ip_delay - ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay);
        }

        ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay = s1ip_delay;
    }

    IoReg_Write32(PPOVERLAY_uzudtg_control3_reg, ppoverlay_uzudtg_control3_reg.regValue);
    IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL3_reg, ppoverlay_memcdtg_control3_reg.regValue);

    if(bEnable_DB)
    {
        wait_for_uzu_memc_dtg_db_apply_success();
    }

    rtd_pr_vbe_notice("s1ip_delay_line=%d\n", ppoverlay_uzudtg_control3_reg.s1ip_max_line_delay);
}


unsigned int scaler_disp_dtg_get_time_borrow_value(void)
{
	unsigned int time_borrow =0;
	unsigned int b_isMEMCypass = (PPOVERLAY_MEMC_MUX_CTRL_get_memc_outmux_sel(IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg))==0)?1:0;
    
    if(get_MEMC_bypass_status_refer_platform_model() == TRUE)
    {
        return 0;
    }	
#ifndef BUILD_QUICK_SHOW
    if((drv_memory_get_game_mode() == _ENABLE) || vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag()){
		if((get_vsc_src_is_hdmi_or_dp()) &&	Scaler_MEMC_Lib_UltraLowDelayMode_Judge())
			time_borrow = 0;	//game mode ultra lowdelay set time borrow = 0
		else if(b_isMEMCypass==0)
        {
            time_borrow = DTG_DEFAULT_TIME_BORROW_V; //normal game mode case
            if(Get_DISP_DEN_STA_VPOS() <= DTG_DEFAULT_TIME_BORROW_V)
            {
                time_borrow = 0;
            }
        }
		else
			time_borrow = 0;	//game mode bypass memc or VRR set time borrow to 0, use HW auto mode
	}
    else if(Get_DISPLAY_PANEL_OLED_TYPE() == TRUE){
        if(Scaler_check_orbit_store_mode() == TRUE){
            //time borrow = NNSR worse case: 37 + overscan store mode : 16 - front porch 45
            time_borrow = DTG_DEFAULT_TIME_BORROW_V - DTG_ORBIT_MAX_EXTEND_V + DTG_ORBIT_STORE_MODE_OFFSET;
        }else{
            time_borrow = DTG_DEFAULT_TIME_BORROW_V - DTG_ORBIT_MAX_EXTEND_V;
        }
	}else
#endif
	   if((get_panel_res() == PANEL_RES_FHD) || (get_panel_res() == PANEL_RES_HD) || (Get_DISP_DEN_STA_VPOS() < (DTG_DEFAULT_TIME_BORROW_V+_M_PRE_READ_MARGIN)))
		time_borrow = 0; // FHD/HD model don't need time borrow
	else        
		time_borrow = DTG_DEFAULT_TIME_BORROW_V;

	return time_borrow;
}
unsigned int scaler_disp_dtg_get_s1ip_delay_value(void)
{

	unsigned int s1ip_delay_line =DTG_DEFAULT_S1IP_LINE_DELAY;
	unsigned int time_borrow =scaler_disp_dtg_get_time_borrow_value();
	unsigned int b_isMEMCypass = (PPOVERLAY_MEMC_MUX_CTRL_get_memc_outmux_sel(IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg))==0)?1:0;
	
    if(get_MEMC_bypass_status_refer_platform_model() == TRUE)
    {
        return 0;
    }
    
    if((Get_DISP_VERTICAL_TOTAL() - Get_DISP_DEN_END_VPOS()) < DTG_DEFAULT_S1IP_LINE_DELAY)
    {
        s1ip_delay_line = Get_DISP_VERTICAL_TOTAL() - Get_DISP_DEN_END_VPOS() - 2;
    }

    if((drv_memory_get_game_mode() == _ENABLE) || vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())
	{
		if((get_vsc_src_is_hdmi_or_dp()) && Scaler_MEMC_Lib_UltraLowDelayMode_Judge()){
			//game mode ultra lowdelay to adjust s1ip delay for memc internal delay
			s1ip_delay_line = DTG_ULTRA_LOWDELAY_S1IP_LINE_DELAY;
		}else if ((drv_memory_get_game_mode() == _ENABLE) && (b_isMEMCypass == 0)){
			//normal game mode
			s1ip_delay_line = DTG_DEFAULT_S1IP_LINE_DELAY;
		}else
			//game mode + memc bypass case or VRR set s1ip delay to 0, use HW auto mode
			s1ip_delay_line = 0;	//game mode or VRR set s1ip delay to 0, use HW auto mode
	}

	if(Get_DISPLAY_PANEL_OLED_TYPE() == TRUE)
	{
		if((drv_memory_get_game_mode()) && (get_vsc_src_is_hdmi_or_dp()) && Scaler_MEMC_Lib_UltraLowDelayMode_Judge()){
			// ultra-lowdelay game mode, no need to change.
		}else{
			//oled panel need to keep fixed value 0x16 between s1ip delay and timeborrow
			s1ip_delay_line = DTG_DEFAULT_S1IP_LINE_DELAY - DTG_DEFAULT_TIME_BORROW_V + time_borrow;
		}
	}
	return s1ip_delay_line;
}
#ifndef BUILD_QUICK_SHOW

void wait_dispd_stage1_smooth_toggle_apply_done(void)
{
    ppoverlay_dispd_smooth_toggle1_RBUS dispd_smooth_toggle1_reg;
    unsigned int count = 0xfffff;
    
    dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
    while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply)&& --count){
        dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
    }
    if(count==0){
        rtd_pr_vbe_emerg("[%s]timeout !!!\n",__func__);
    }

}

unsigned char scaler_disp_wait_uzudtg_align_framecount(char const* pfunc,UINT32 line)
{
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS	dispd_smooth_toggle1_reg;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS dispd_stage1_sm_ctrl_reg;
	unsigned int count = 0xfffff;
	unsigned char uzu_valid_framecnt = 0;

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzu_valid_framecnt = uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync;	//if 2:5, valid_framecnt = 1, if 1:2, valid_framecnt = 0
	//wait to front porch
	count = 0xfffff;
    dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
	while(!((dtg_frame_cnt1_reg.uzudtg_output_frame_cnt == uzu_valid_framecnt) &&
		(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)) > 100) &&
		(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)) < 1900)) && --count){
		dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
	}
	if(count == 0){
		rtd_pr_vbe_emerg("[DTG][%s][%d] Wait to uzu_valid_framecnt timeout!!, uzu_valid_framecnt = %d, uzudtg_output_frame_cnt = %d\n"
			,pfunc,line,uzu_valid_framecnt, dtg_frame_cnt1_reg.uzudtg_output_frame_cnt);
		return 0;
	}
	count = 0xfffff;

    if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE){
		Check_smooth_toggle_update_flag(SLR_MAIN_DISPLAY);// wait main smooth toggle apply done
	}
	if(Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE){
		Check_smooth_toggle_update_flag(SLR_SUB_DISPLAY);// wait sub smooth toggle apply done
	}
    
	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	//stage1 clk mode revised default set to 1
	dispd_smooth_toggle1_reg.dispd_stage1_clk_mode_revised = 1;
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_src_sel = 1;
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);

	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply)&& --count){
		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	}
	if(count==0){
		rtd_pr_vbe_emerg("[%s][%d]dispd_smooth_toggle1 timeout !!!\n", pfunc,line);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 0;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
		dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
		dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
		IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
		dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
		return 0;
	}else{
		rtd_pr_vbe_emerg("[DTG][%s][%d] uzu valid_framecnt is ok. uzudtg_output_frame_cnt = %d, uzu_lcnt=%d \n"
			,pfunc,line,PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_output_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)), PPOVERLAY_new_meas2_linecnt_real_get_osddtg_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)));
		return 1;
	}
	return 1;
}
#endif

void scaler_disp_dtg_set_db_latch_line(unsigned int db_latch_line)
{
	ppoverlay_uzudtg_control4_RBUS ppoverlay_uzudtg_control4_reg;
	ppoverlay_uzudtg_control4_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control4_reg);
	ppoverlay_uzudtg_control4_reg.db_latch_line = db_latch_line;
	IoReg_Write32(PPOVERLAY_uzudtg_control4_reg,ppoverlay_uzudtg_control4_reg.regValue);
	rtd_pr_vbe_notice("[%s] db_latch_line=%d\n", __FUNCTION__, ppoverlay_uzudtg_control4_reg.db_latch_line);
}

extern void vbe_dtg_memec_bypass_switch_set(int mode);
extern int vbe_dtg_memec_bypass_switch(void);
extern int Scaler_MEMC_DTV_ME_CLK_OnOff(unsigned char enable, unsigned char bMode);
extern int Scaler_MEMC_MEMC_CLK_OnOff(unsigned char ME_enable, unsigned char MC_enable, unsigned char bMute);
extern unsigned char rtk_hal_vsc_GetOutputRegion(VIDEO_WID_T wid, VIDEO_RECT_T * poutregion);
extern void Scaler_MEMC_HDMI_ME_CLK_OnOff(unsigned char is_clk_on, unsigned char mode);

void scaler_disp_dtg_pre_i_domain_setting(unsigned char display)
{
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS	dispd_smooth_toggle1_reg;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS 	dispd_stage1_sm_ctrl_reg;
//	ppoverlay_dtg_m_remove_input_vsync_RBUS dtg_m_remove_input_vsync_reg;
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	ppoverlay_uzudtg_countrol_RBUS ppoverlay_uzudtg_countrol_reg;

//	ppoverlay_uzudtg_control1_RBUS ppoverlay_uzudtg_control1_reg;
	unsigned char uzu_valid_framecnt = 0;
	ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
	SCALER_DISP_CHANNEL _channel = 0;
	unsigned long flags;//for spin_lock_irqsave
	unsigned int count = 0xfffff;
	UINT32 mul_dbuf_cnt=0;
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

	if(display > SLR_MAIN_DISPLAY){
		rtd_pr_vbe_notice("[%s]sub path not config timing\n", __FUNCTION__);
		return;
	}

	rtd_pr_vbe_notice("+++scaler_disp_dtg_pre_i_domain_setting!!\n");

	/* KTASKWBS-12187 timing change need to turn off mode revised, otherwise dtg ip will block due to
	     frame index cannot count.
	     For case : 24hz in/ 120hz out , uzu remove/multiple = 10:2, change timing to
	                    30hz in / 120hz out, uzu remove/multiple = 4:1, frame count cannot read index 1
	                    and issue happened.
	     DIC recommaned turn off this mode, duriing timine change procedure.
	 */
	ppoverlay_uzudtg_countrol_reg.regValue = IoReg_Read32(PPOVERLAY_UZUDTG_COUNTROL_reg);
	ppoverlay_uzudtg_countrol_reg.uzudtg_mode_revised = 0;
	IoReg_Write32(PPOVERLAY_UZUDTG_COUNTROL_reg, ppoverlay_uzudtg_countrol_reg.regValue);
	rtd_pr_vbe_notice("uzudtg_mode_revised == 0\n");

	drivf_scaler_reset_freerun();
#ifndef BUILD_QUICK_SHOW
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
#endif
    //double buffer D0
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.dreg_dbuf_en = 0;
	double_buffer_ctrl_reg.drm_multireg_dbuf_en = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_read_sel = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_vsync_sel = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
#ifndef BUILD_QUICK_SHOW
    up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
#endif
    ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

    down(&VBE_LowDelay_Semaphore);
	//pre-flow. patch, let D8 double buffer works well
	count = 0xfffff;
	while((count == 0xfffff)&&(mul_dbuf_cnt<10000)){
		//msleep(10);
		
#ifndef BUILD_QUICK_SHOW
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
#endif
        double_buffer_ctrl_reg.drm_multireg_dbuf_set = 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
#ifndef BUILD_QUICK_SHOW
        up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
#endif
        count = 0xfffff;
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		while((double_buffer_ctrl_reg.drm_multireg_dbuf_set)&& --count){
			double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		}
		if(count==0){
			rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_set 1 wait timeout !!!\n");
		}
		if(double_buffer_ctrl_reg.drm_multireg_dbuf_en==0)
			rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_en=0 1!!\n");

		mul_dbuf_cnt++;
	}

	//meas0_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[0] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);

	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=0;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
	IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzu_valid_framecnt = uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync;	//if 2:5, valid_framecnt = 1, if 1:2, valid_framecnt = 0
	//wait to front porch
	count = 0xfffff;
    dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
	while(!((dtg_frame_cnt1_reg.uzudtg_output_frame_cnt == uzu_valid_framecnt) &&
		(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)) > 100) &&
		(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)) < 1900)) && --count){
		dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
	}
	if(count == 0)
		rtd_pr_vbe_emerg("[DTG] Wait to uzu_valid_framecnt timeout!!, uzu_valid_framecnt = %d, uzudtg_output_frame_cnt = %d\n",uzu_valid_framecnt, dtg_frame_cnt1_reg.uzudtg_output_frame_cnt);

	//meas0_lineCnt2 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt2 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[1] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);

	if(scaler_get_idomain_vfreq_pre() != Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)){
		rtd_pr_vbe_emerg("input vfreq has changed !!!\n");
		up(&VBE_LowDelay_Semaphore);
		return;
	}
	modestate_set_fractional_framesyncByDisp(_channel);
	//meas0_lineCnt3 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt3 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[2] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
	if(scaler_get_idomain_vfreq_pre() != Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)){
		rtd_pr_vbe_emerg("input vfreq has changed !!!\n");
		up(&VBE_LowDelay_Semaphore);
		return;
	}
	modestate_set_display_timing_gen_clock_divByDisp(_channel);
	modestate_set_display_timing_gen_htotal_ByDisp(_channel);
	//meas0_lineCnt4 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt4 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[3] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
	count = 0xfffff;

	//set memc in/out framerate after memc handle done, if set this function after force bg,
	//it may happen memc buffer index jump back. (ex: 60hz->24hz, unmute, set in/out framerate)
	//move in front of set display timing gen to prevent memc phase table error
	Scaler_MEMC_SetInOutFrameRateByDisp(_channel);


	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	//stage1 clk mode revised default set to 1
	dispd_smooth_toggle1_reg.dispd_stage1_clk_mode_revised = 1;
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_src_sel = 1;
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

	ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	if(scaler_get_idomain_vfreq_pre() != Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)){
		rtd_pr_vbe_emerg("input vfreq has changed !!!\n");
		up(&VBE_LowDelay_Semaphore);
		return;
	}
#ifndef USE_NEW_HDMI_TIMING_INFO_STRUCTURE
    if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI){
		if(((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245)) ||
			((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305)) /*||
			((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 595) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 605))*/)
			usleep_range(40 * 1000, 40 * 1000);	//wait for ivs cnt update, to calculate real vtotal by ivs cnt
	}
#endif
	modestate_set_display_timing_genByDisp(_channel);//set dvtotal
    up(&VBE_LowDelay_Semaphore);
/*
	rtd_pr_vbe_emerg("[Eric] uzu_valid_framecnt = %d, uzudtg_output_frame_cnt = %d\n",uzu_valid_framecnt,dtg_frame_cnt1_reg.uzudtg_output_frame_cnt);
	rtd_pr_vbe_emerg("meas0_lineCnt1 (%d)\n", PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(meas0_lineCnt1));
	rtd_pr_vbe_emerg("meas1_lineCnt1 (%d)\n", PPOVERLAY_new_meas2_linecnt_real_uzudtg_dly_line_cnt_rt(meas1_lineCnt1));
	rtd_pr_vbe_emerg("meas0_lineCnt2 (%d)\n", PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(meas0_lineCnt2));
	rtd_pr_vbe_emerg("meas1_lineCnt2 (%d)\n", PPOVERLAY_new_meas2_linecnt_real_uzudtg_dly_line_cnt_rt(meas1_lineCnt2));
	rtd_pr_vbe_emerg("meas0_lineCnt3 (%d)\n", PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(meas0_lineCnt3));
	rtd_pr_vbe_emerg("meas1_lineCnt3 (%d)\n", PPOVERLAY_new_meas2_linecnt_real_uzudtg_dly_line_cnt_rt(meas1_lineCnt3));
	rtd_pr_vbe_emerg("meas0_lineCnt4 (%d)\n", PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(meas0_lineCnt4));
	rtd_pr_vbe_emerg("meas1_lineCnt4 (%d)\n", PPOVERLAY_new_meas2_linecnt_real_uzudtg_dly_line_cnt_rt(meas1_lineCnt4));
	rtd_pr_vbe_emerg("meas0_lineCnt5 (%d)\n", PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(meas0_lineCnt5));
	rtd_pr_vbe_emerg("meas1_lineCnt5 (%d)\n", PPOVERLAY_new_meas2_linecnt_real_uzudtg_dly_line_cnt_rt(meas1_lineCnt5));
	rtd_pr_vbe_emerg("time = (%x,%x,%x,%x,%x)\n", timestamp[0],timestamp[1],timestamp[2],timestamp[3],timestamp[4]);
*/
	rtd_pr_vbe_notice("---scaler_disp_dtg_pre_i_domain_setting!!\n");

}

void drvif_scaler_set_Pattern_mute(unsigned char mute_flag, unsigned char display_flag,StructSrcRect* sdp_info)
{
    ppoverlay_double_buffer_ctrl3_RBUS ppoverlay_double_buffer_ctrl3_reg;
    ppoverlay_main_display_control_rsv_RBUS main_display_control_rsv_reg;
    ppoverlay_memc_mux_ctrl_bg_RBUS memc_mux_ctrl_reg;
    
    unsigned long flags = 0;//for spin_lock_irqsave

    unsigned short VRatioOffset = 0, HRatioOffset = 0;
    unsigned short HStartOffset = 0;
    unsigned short VStartOffset = 0;
 
    ppoverlay_double_buffer_ctrl3_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL3_reg); 
    ppoverlay_double_buffer_ctrl3_reg.d2bg_dtgreg_dbuf_en = 0; //0xb802847c  bit 18 = 0 ,enable memc double bugffer
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL3_reg, ppoverlay_double_buffer_ctrl3_reg.regValue);
    
    memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_BG_reg); 
    memc_mux_ctrl_reg.memc_out_bg_en = 0; //0xb8028100  bit 12 = 0 ,memc bg
    IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_BG_reg, memc_mux_ctrl_reg.regValue);
    
	if(display_flag == 1)
	{
		StructSrcRect m_dispwin_set;

		m_dispwin_set.srcx = sdp_info->srcx;
		m_dispwin_set.srcy = sdp_info->srcy;
		m_dispwin_set.src_height = sdp_info->src_height;
		m_dispwin_set.src_wid = sdp_info->src_wid;

		rtd_pr_vbe_emerg("drvif_scaler_set_Pattern_mute m_dispwin_set.srcx=%d,srcy=%d,src_height=%d,src_wid=%x", m_dispwin_set.srcx, m_dispwin_set.srcy,m_dispwin_set.src_height, m_dispwin_set.src_wid); 

		Scaler_DispWindowSet(m_dispwin_set);
		{
		StructSrcRect m_dispwin = Scaler_DispWindowGet();
			   HRatioOffset =  m_dispwin.srcx;
			   VRatioOffset = m_dispwin.srcy;
			   scalerdisplay_set_main_act_window(
							   (unsigned short)Get_DISP_ACT_STA_HPOS() + HRatioOffset + HStartOffset,
							   (unsigned short)Get_DISP_ACT_STA_HPOS() + Scaler_DispWindowGet().src_wid + HRatioOffset + HStartOffset ,
							   (unsigned short)Get_DISP_ACT_STA_VPOS() + VRatioOffset + VStartOffset,
							   (unsigned short)Get_DISP_ACT_STA_VPOS() + Scaler_DispWindowGet().src_height+ VRatioOffset + VStartOffset );
		}
	}

	if(mute_flag == 0)
	{
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock

		ppoverlay_double_buffer_ctrl3_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL3_reg);
		ppoverlay_double_buffer_ctrl3_reg.dmainreg_other_dbuf_en = 0; //enable force bg double buffer
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL3_reg, ppoverlay_double_buffer_ctrl3_reg.regValue);

		main_display_control_rsv_reg.regValue = IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
		main_display_control_rsv_reg.m_force_bg = 0;
		//main_display_control_rsv_reg.main_wd_to_background = 0;
		IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, main_display_control_rsv_reg.regValue);

		ppoverlay_double_buffer_ctrl3_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL3_reg);
		ppoverlay_double_buffer_ctrl3_reg.dmainreg_other_dbuf_en = 1; //enable force bg double buffer
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL3_reg, ppoverlay_double_buffer_ctrl3_reg.regValue);

		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	}
	else if(mute_flag == 1)
        {

		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock

		ppoverlay_double_buffer_ctrl3_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL3_reg);
		ppoverlay_double_buffer_ctrl3_reg.dmainreg_other_dbuf_en = 0; //enable force bg double buffer
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL3_reg, ppoverlay_double_buffer_ctrl3_reg.regValue);

		main_display_control_rsv_reg.regValue = IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
		main_display_control_rsv_reg.m_force_bg = 1;//check it later
		//main_display_control_rsv_reg.main_wd_to_background = 1;
		IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, main_display_control_rsv_reg.regValue);

		ppoverlay_double_buffer_ctrl3_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL3_reg);
		ppoverlay_double_buffer_ctrl3_reg.dmainreg_other_dbuf_en = 1; //enable force bg double buffer
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL3_reg, ppoverlay_double_buffer_ctrl3_reg.regValue);

		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

	}
}

void Scaler_wait_for_LCM_done(void){
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	unsigned int start_time = 0, end_time = 0;
	unsigned int count = 0x3fffff;

	start_time = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.drm_multireg_dbuf_en = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_vsync_sel = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);

	double_buffer_ctrl_reg.drm_multireg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);

	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	while(double_buffer_ctrl_reg.drm_multireg_dbuf_set && count){
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		count--;
	}
	if(count==0){
		rtd_pr_vbe_notice("[%s] wait timeout.\n", __FUNCTION__);
	}
	end_time = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
	rtd_pr_vbe_notice("[%s] wait time = %d ms\n", __FUNCTION__,(end_time - start_time)/90);
}

void Scaler_wait_for_frc2fs_hw_lock_done(unsigned char display)
{
	//when disp_frc_fsync raised, position is at den_end, front porch may not at LCM framecnt.
	//need to wait a LCM period to make sure vsync aligned done.
	unsigned int start_time = 0, end_time = 0;
	start_time = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
	if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) < 245))
		msleep(90);	//24Hz to 60Hz LCM=12Hz
	else if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 245) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) < 255))
		msleep(40);	//25 to 50Hz LCM=25Hz
	else if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) < 305))
		msleep(40);	//30 to 60Hz LCM=30Hz
	else if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 495) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) < 505))
		msleep(20);	//50 to 50Hz LCM=50Hz
	else if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 595) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) < 605))
		msleep(20);	//60 to 60Hz LCM=60Hz
	else
		msleep(90);
	end_time = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
	rtd_pr_vbe_debug("[%s] wait time = %d ms\n", __FUNCTION__,(end_time - start_time)/90);
}

void Scaler_wait_for_input_one_frame(unsigned char display)
{
	//when disp_frc_fsync raised, position is at den_end, front porch may not at LCM framecnt.
	//need to wait a LCM period to make sure vsync aligned done.
	//unsigned int start_time = 0, end_time = 0;
	//start_time = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
	if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) < 245))
		msleep(50);	//24Hz 1frame = 41.6ms
	else if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 245) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) < 255))
		msleep(40);	//25Hz 1frame = 40ms
	else if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) < 305))
		msleep(40);	//30Hz 1frame=33.2ms
	else if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 495) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) < 505))
		msleep(20);	//50Hz 1frame=20ms
	else if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 595) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) < 605))
		msleep(20);	//60Hz 1frame=16.6ms
	else if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) >= 995) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) < 1205))
		msleep(10);	//100Hz = 10ms
	else
		msleep(50);
	//end_time = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
//	rtd_pr_vbe_debug("[%s] wait time = %d ms\n", __FUNCTION__,(end_time - start_time)/90);
}


static void set_ivs_source(int src)
{
	ppoverlay_fs_iv_dv_fine_tuning1_RBUS ppoverlay_fs_iv_dv_fine_tuning1_reg;

	ppoverlay_fs_iv_dv_fine_tuning1_reg.regValue = IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning1_reg);
	ppoverlay_fs_iv_dv_fine_tuning1_reg.ivs_sel = src;
	IoReg_Write32(PPOVERLAY_FS_IV_DV_Fine_Tuning1_reg, ppoverlay_fs_iv_dv_fine_tuning1_reg.regValue);
}

void rest_dtg_ivs_source(void)
{
	set_ivs_source(0);
}

void set_dtg_ivs_source(void)
{
	if ((get_vsc_src_is_hdmi_or_dp())
		// && (vbe_disp_get_freesync_mode_flag() || vbe_disp_get_VRR_timingMode_flag())
		&& ((Get_Val_DP_Vsync_Regen_Pos() == DP_REGEN_VSYNC_AT_HDR1)
			|| (Get_Val_DP_Vsync_Regen_Pos() == DP_REGEN_VSYNC_AT_MAIN_VGIP)))
		set_ivs_source(1);
	else
		set_ivs_source(0);
}
/*Note: 20230307
This API not be used for QS,please use void Scaler_disp_setting_QS(unsigned char display)
*/
void Scaler_disp_setting(unsigned char display)
{
#ifndef BUILD_QUICK_SHOW
//	unsigned char ret=0;
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS	dispd_smooth_toggle1_reg;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS 	dispd_stage1_sm_ctrl_reg;
	ppoverlay_uzudtg_countrol_RBUS ppoverlay_uzudtg_countrol_reg;
//	ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
//	ppoverlay_dtg_m_remove_input_vsync_RBUS dtg_m_remove_input_vsync_reg;
//	unsigned char uzu_valid_framecnt = 0;

	unsigned int count = 0x3fffff;
	UINT32 mul_dbuf_cnt=0;
	UINT32 mul_dbuf_cnt2=0;
	VIDEO_RECT_T outregion;
	unsigned long flags;//for spin_lock_irqsave
	S_VPQ_ModuleTestMode S_ModuleTestMode = {0};

	if( Scaler_Get_PQ_ModuleTest() != NULL ){
		S_ModuleTestMode.ModuleTestMode_en = Scaler_Get_PQ_ModuleTest()->ModuleTestMode_en;
		S_ModuleTestMode.ModuleTestMode_Flag= Scaler_Get_PQ_ModuleTest()->ModuleTestMode_Flag;
	}

	down(&VBE_Semaphore);

	set_dtg_ivs_source();

	if ((SCALER_DISP_CHANNEL)display == SLR_MAIN_DISPLAY) {
#if 0	// move to scaler_disp_dtg_pre_i_domain_setting
		SCALER_DISP_CHANNEL _channel = 0;
		UINT32 meas0_lineCnt1=0;
		UINT32 meas0_lineCnt2=0;
		UINT32 meas0_lineCnt3=0;
		UINT32 meas0_lineCnt4=0;
		UINT32 meas0_lineCnt5=0;
		UINT32 meas1_lineCnt1=0;
		UINT32 meas1_lineCnt2=0;
		UINT32 meas1_lineCnt3=0;
		UINT32 meas1_lineCnt4=0;
		UINT32 meas1_lineCnt5=0;
		UINT32 timestamp[5] = {0,0,0,0,0};
		unsigned long flags;//for spin_lock_irqsave

		drivf_scaler_reset_freerun();

		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		//double buffer D0
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		double_buffer_ctrl_reg.dreg_dbuf_en = 0;
		double_buffer_ctrl_reg.drm_multireg_dbuf_en = 1;
		double_buffer_ctrl_reg.drm_multireg_dbuf_read_sel = 1;
		double_buffer_ctrl_reg.drm_multireg_dbuf_vsync_sel = 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
		ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

        down(&VBE_LowDelay_Semaphore);
		//pre-flow. patch, let D8 double buffer works well
		count = 0x3fffff;
		while((count == 0x3fffff)&&(mul_dbuf_cnt<10000)){
			//msleep(10);
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
				rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_set 1 wait timeout !!!\n");
			}
			if(double_buffer_ctrl_reg.drm_multireg_dbuf_en==0)
				rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_en=0 1!!\n");

			mul_dbuf_cnt++;
		}

		meas0_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
		meas1_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
		timestamp[0] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
		dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
		uzu_valid_framecnt = dtg_m_remove_input_vsync_reg.remove_half_ivs_mode2;	//if 2:5, valid_framecnt = 1, if 1:2, valid_framecnt = 0
		//wait to front porch
		count = 0x3fffff;
        dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
		while(!((dtg_frame_cnt1_reg.uzudtg_output_frame_cnt == uzu_valid_framecnt) && (IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg) > 100) && (IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg) < 2000)) && --count){
			dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
		}
		if(count == 0)
			rtd_pr_vbe_emerg("[DTG] Wait to uzu_valid_framecnt timeout!!, uzu_valid_framecnt = %d, uzudtg_output_frame_cnt = %d\n",uzu_valid_framecnt, dtg_frame_cnt1_reg.uzudtg_output_frame_cnt);

		meas0_lineCnt2 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
		meas1_lineCnt2 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
		timestamp[1] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
		modestate_set_fractional_framesyncByDisp(_channel);
		meas0_lineCnt3 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
		meas1_lineCnt3 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
		timestamp[2] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
		modestate_set_display_timing_gen_clock_divByDisp(_channel);
		meas0_lineCnt4 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
		meas1_lineCnt4 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
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
		meas0_lineCnt5 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
		meas1_lineCnt5 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
		timestamp[4] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);

		ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
		ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
		modestate_set_display_timing_genByDisp(_channel);//set dvtotal
        up(&VBE_LowDelay_Semaphore);
#endif
/*
		rtd_pr_vbe_emerg("[Eric] uzu_valid_framecnt = %d, uzudtg_output_frame_cnt = %d\n",uzu_valid_framecnt,dtg_frame_cnt1_reg.uzudtg_output_frame_cnt);
		rtd_pr_vbe_emerg("meas0_lineCnt1 (%d)\n", meas0_lineCnt1);
		rtd_pr_vbe_emerg("meas1_lineCnt1 (%d)\n", meas1_lineCnt1);
		rtd_pr_vbe_emerg("meas0_lineCnt2 (%d)\n", meas0_lineCnt2);
		rtd_pr_vbe_emerg("meas1_lineCnt2 (%d)\n", meas1_lineCnt2);
		rtd_pr_vbe_emerg("meas0_lineCnt3 (%d)\n", meas0_lineCnt3);
		rtd_pr_vbe_emerg("meas1_lineCnt3 (%d)\n", meas1_lineCnt3);
		rtd_pr_vbe_emerg("meas0_lineCnt4 (%d)\n", meas0_lineCnt4);
		rtd_pr_vbe_emerg("meas1_lineCnt4 (%d)\n", meas1_lineCnt4);
		rtd_pr_vbe_emerg("meas0_lineCnt5 (%d)\n", meas0_lineCnt5);
		rtd_pr_vbe_emerg("meas1_lineCnt5 (%d)\n", meas1_lineCnt5);
		rtd_pr_vbe_emerg("time = (%x,%x,%x,%x,%x)\n", timestamp[0],timestamp[1],timestamp[2],timestamp[3],timestamp[4]);
*/
    extern unsigned int Get_DISPLAY_REFRESH_RATE(void);

	//K6HP/K6LP  both can set 60Hz panel, so only check panel refresh rate to go MC only mode
	if(Get_DISPLAY_REFRESH_RATE() >= 120){
		if(((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) >= 994) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) <= 1005)) ||
			((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) >= 1191) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) <= 1206)) ||
            ((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) >= 1435) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) <= 1445)) ||
            ((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) >= 2395) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) <= 2405)) ||
            ((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) >= 2875) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) <= 2885))
			||((get_vsc_src_is_hdmi_or_dp()) && (get_hdmi_4k_hfr_mode() != HDMI_NON_4K120))){
			//set MEMC mux disable
			Scaler_MEMC_Bypass_On(1);
			//MEMC bypass
			Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);
	//		vbe_dtg_memec_bypass_switch_set(0);
			rtd_pr_vbe_notice("[Scaler_disp_setting] HFR bypass MEMC\n");
		}
		else if (get_vsc_src_is_hdmi_or_dp())
		{
			//Freesync need go low delay mode, bypass memc
			if(vbe_disp_get_VRR_timingMode_flag() ||vbe_disp_get_freesync_mode_flag()){
				//disable MEMC clock
				Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);
				Scaler_MEMC_Bypass_On(1);
				rtd_pr_vbe_notice("[Scaler_disp_setting] VRR/Freesync mode set MEMC bypass\n");
			}
			else
			{
				//enable MEMC clock
				Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);
				//set MEMC mux enable
				Scaler_MEMC_Bypass_On(0);
		//		vbe_dtg_memec_bypass_switch_set(0);
				rtd_pr_vbe_notice("[Scaler_disp_setting]120Hz panel set hdmi uhd MEMC on\n");
			}
		}
		else{
            #ifndef BUILD_QUICK_SHOW
           if(get_vt_EnableFRCMode() != FALSE)
           {
    			//enable MEMC clock
    			Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);
    			//set MEMC mux enable
    			Scaler_MEMC_Bypass_On(0);
    	//		vbe_dtg_memec_bypass_switch_set(0);
    			rtd_pr_vbe_notice("[Scaler_disp_setting]120Hz panel set MEMC on\n");
            }
           #endif
			//Eric@20190926 Vdec Direct Low Latency mode flow
			if(drv_memory_get_vdec_direct_low_latency_mode()){
				rtd_pr_vbe_notice("[%s][%d][LowDelayMode]", __FUNCTION__, __LINE__);
				vpq_memc_set_lowDelayMode(_ENABLE);
				rtd_pr_vbe_notice("[Scaler_disp_setting]Vdec Direct set MEMC Low Latency mode\n");
			}
		}
	}
	else
	{
        if(is_4k_hfr_mode(display))
        {
            Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);//disable clock
            Scaler_MEMC_Bypass_On(1); //-> MEMC Mux bypass
            rtd_pr_vbe_notice("[Scaler_disp_setting]4k hfr bypass memc\n");
        }
		else if ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA) &&    //DTV/USB playback 4K 50/60 default ME off, MC on
			(!get_vsc_run_adaptive_stream(SLR_MAIN_DISPLAY)) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > 2560) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) > 1440) &&
			(((Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO) == 1) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO) == 1)) ||
			((Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO) == 5) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO) == 6)))) {
		#if 0 //MEMC off
			Scaler_MEMC_Bypass_On(1);
			vbe_dtg_memec_bypass_switch_set(1);
			rtd_pr_vbe_notice("[jerry_Scaler_MEMC_DTV_ME_CLK_OnOff_AAA][%d]\n", ((rtd_inl(0xb8028100)>>13)&0x1));
			//Scaler_MEMC_Bypass_CLK_OnOff(0);
			Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);
		#else //MC on, ME off, setting clock then mux avoid see garbage
			//disable ME clock
	//		Scaler_MEMC_DTV_ME_CLK_OnOff(0, 1);
			if(TV006_WB_Pattern_En){
				Scaler_MEMC_Bypass_On(1);
				Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);
			}
			else{
             #ifndef BUILD_QUICK_SHOW   
              if(get_vt_EnableFRCMode() != FALSE)
              {
				Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 1);
				//set MEMC mux enable
				Scaler_MEMC_Bypass_On(0);
              }
              #endif
			}
			rtd_pr_vbe_notice("[Scaler_disp_setting][AA][%d,%d]\n", ((rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg)>>13)&0x1), TV006_WB_Pattern_En);
		#endif
		}else if ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA) &&     //CP 4k2k 50/60Hz default ME off, MC on
			(get_vsc_run_adaptive_stream(SLR_MAIN_DISPLAY)) &&
			(((Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO) == 1) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO) == 1)) ||
			((Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO) == 5) &&
			(Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO) == 6))) &&
			((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) > 3000) && (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > 2000))) {
			//Eric@20181016 Vdec Direct Low Latency mode flow
			if(drv_memory_get_vdec_direct_low_latency_mode()){
				//set MEMC mux disable
				Scaler_MEMC_Bypass_On(1);
				//disable MEMC clock
				Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);
				rtd_pr_vbe_notice("[vo low delay][%d]\n", ((rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg)>>13)&0x1));
			}
			else if(TV006_WB_Pattern_En){
				Scaler_MEMC_Bypass_On(1);
				Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);
			}
			else{
                #ifndef BUILD_QUICK_SHOW
                if(get_vt_EnableFRCMode() != FALSE)
                {
    				//disable MEMC clock
    				Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 1);
    				//set MEMC mux disable
    				Scaler_MEMC_Bypass_On(0);
                }
                #endif
			}
	//		vbe_dtg_memec_bypass_switch_set(1);
			rtd_pr_vbe_notice("[Scaler_disp_setting][BB][%d,%d,%d]\n", ((rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg)>>13)&0x1), TV006_WB_Pattern_En, drv_memory_get_vdec_direct_low_latency_mode());
			//Scaler_MEMC_Bypass_CLK_OnOff(0);
		} else if ((get_vsc_src_is_hdmi_or_dp()) && (check_hdmi_uhd_condition() == TRUE)) {
			//Freesync need go low delay mode, bypass memc
			if(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag()){
				//disable MEMC clock
				Scaler_MEMC_Bypass_On(1);
				Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);
			}
			else{
				if((Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO) == 1) &&
				(Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO) == 1)){
					// 4k60Hz default ME off, MC on
					Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 1);
					vbe_set_HDMI_dynamic_film_mode_flag(TRUE);
				}
				else{
					//enable MEMC clock
					Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);
					vbe_set_HDMI_dynamic_film_mode_flag(FALSE);
				}
				//set MEMC mux enable
				Scaler_MEMC_Bypass_On(0);
			}
			rtd_pr_vbe_notice("[Scaler_disp_setting][CC][%d]\n", ((rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg)>>13)&0x1));
		}  else /*if (vbe_dtg_memec_bypass_switch())*/ { //othersource need MEMC on, if previous close must on again
			//CP Vdec Direct need force MEMC gamemode
			if(((drv_memory_get_vdec_direct_low_latency_mode() &&
                               ((Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO) == 1) &&
                               (Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO) == 1)) &&
                               get_vsc_run_adaptive_stream(SLR_MAIN_DISPLAY)) ||
				TV006_WB_Pattern_En)){
				//set MEMC mux disable
				Scaler_MEMC_Bypass_On(1);
				//disable MEMC clock
				Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);
				rtd_pr_vbe_notice("[vo low delay][%d]\n", ((rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg)>>13)&0x1));
			}
			else if(vbe_disp_get_VRR_timingMode_flag() ||vbe_disp_get_freesync_mode_flag()){	//Freesync need go low delay mode, bypass memc
				//disable MEMC clock
				Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);
				Scaler_MEMC_Bypass_On(1);
				rtd_pr_vbe_notice("[Freesync mode][%d]\n", ((rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg)>>13)&0x1));
			}
			else{
                #ifndef BUILD_QUICK_SHOW
                  if(get_vt_EnableFRCMode() != FALSE)
                  {
    				//enable MEMC clock
    				Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);
    				//set MEMC mux enable
    				Scaler_MEMC_Bypass_On(0);
    		//		vbe_dtg_memec_bypass_switch_set(0);
    				rtd_pr_vbe_notice("[Scaler_disp_setting][DD][%d]\n", ((rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg)>>13)&0x1));

				//Scaler_MEMC_Bypass_CLK_OnOff(1);
				}
                #endif
			}
		}
	}

	//default clear fll running flag
		modestate_set_fll_running_flag(FALSE);
#ifdef ENABLE_VBY1_TO_HDMI_4K2K_DATA_FS
		if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)){
			//drvif_framesync_gatting(_DISABLE);
			//drvif_framesync_gatting(_ENABLE);
			drvif_framesync_gatting_do();
			//Eric@20190919 wait a lowest-common-multiple time to make sure HW process frc2fs done
			Scaler_wait_for_frc2fs_hw_lock_done(display);
#ifdef CONFIG_DATA_FRAMESYNC_FORCE_422_CAP
			if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA){
				//VDEC 2k1k use original flow, framesync with gatting
				if(!((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > DATAFS_DISABLE_GATTING_WIDTH) && (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) > DATAFS_DISABLE_GATTING_HEIGHT))){
					//IoReg_SetBits(VODMA_VODMA_CLKGEN_reg,VODMA_VODMA_CLKGEN_en_fifo_full_gate_mask);
					scaler_set_full_gatting_rpc(true);
					rtd_pr_vbe_emerg("Enable Gating!\n");
				}
				else
					rtd_pr_vbe_emerg("Force Don't enable Gating\n");
			}
			else if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_HDMI) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_JPEG)){
				//IoReg_SetBits(VODMA_VODMA_CLKGEN_reg,VODMA_VODMA_CLKGEN_en_fifo_full_gate_mask);
                scaler_set_full_gatting_rpc(true);
                rtd_pr_vbe_emerg("Enable Gating!\n");
			}
#endif
			if ((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)) {
				modestate_set_fll_running_flag(_DISABLE);
			}
		}else{
			modestate_set_fll_running_flag(_ENABLE);
#ifndef BUILD_QUICK_SHOW
			if(Scaler_InputSrcGetType(display) == _SRC_TV){
				if(drvif_module_vdc_read_Clamping_path() && drvif_module_vdc_read_scan_flag()){
					rtd_pr_vbe_notice("### ATV Channel Scan !!! ###\n");
					modestate_set_fll_running_flag(FALSE);
				}
			}
#endif
		}
#else
	if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)){
		if((Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_COMMERCIAL_4K2K_600M_1S_8L_DUPLICATE) && (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))){
			rtd_pr_vbe_notice("### Business Display Vby1 to HDMI go data fs !!! ###\n");
			//drvif_framesync_gatting(_DISABLE);
			//drvif_framesync_gatting(_ENABLE);
			drvif_framesync_gatting_do();
		}
		modestate_set_fll_running_flag(_DISABLE);
		modestate_set_display_timing(0);
		Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_FRC);
	}
	else{
		if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)){
			//gatting new flow : raise vo clock first -> set timing sync -> enable gatting after timing stable
			//drvif_framesync_gatting(_DISABLE);
			//drvif_framesync_gatting(_ENABLE);
			drvif_framesync_gatting_do();
			//Eric@20190919 wait a lowest-common-multiple time to make sure HW process frc2fs done
#ifdef CONFIG_DATA_FRAMESYNC_FORCE_422_CAP
			Scaler_wait_for_frc2fs_hw_lock_done(display);
			//enable gatting
			if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA){
				//VDEC 2k1k use original flow, framesync with gatting
				if(!((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > DATAFS_DISABLE_GATTING_WIDTH) && (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) > DATAFS_DISABLE_GATTING_HEIGHT))){
					//IoReg_SetBits(VODMA_VODMA_CLKGEN_reg,VODMA_VODMA_CLKGEN_en_fifo_full_gate_mask);
                    scaler_set_full_gatting_rpc(true);
                    rtd_pr_vbe_emerg("Enable Gating!\n");
				}
				else
					rtd_pr_vbe_emerg("Force Don't enable Gating\n");
			}
			else if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_HDMI) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_JPEG)){
                //IoReg_SetBits(VODMA_VODMA_CLKGEN_reg,VODMA_VODMA_CLKGEN_en_fifo_full_gate_mask);
                scaler_set_full_gatting_rpc(true);
                rtd_pr_vbe_emerg("Enable Gating!\n");
			}
#endif
		}else{

#ifdef SMOOTH_TOGGLE_DATAFRCFS_TEST
			if ((vbe_disp_get_adaptivestream_fs_mode() == 1) &&
				(Get_DISPLAY_PANEL_TYPE() != P_VBY1_TO_HDMI) &&
				(Get_DISPLAY_PANEL_TYPE() != P_LVDS_TO_HDMI) &&
				(Get_DISPLAY_PANEL_TYPE() != P_VBY1_TO_LVDS_TO_HDMI)) {
				if(get_vo_camera_flow_flag(Get_DisplayMode_Port(SLR_MAIN_DISPLAY))!=1){
					//drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
    				if(drvif_scaler_get_display_mode() != DISPLAY_MODE_FRAME_SYNC){
    					rtd_pr_vbe_notice("[%s][line:%d] adaptive stream timing fs.\n", __FUNCTION__, __LINE__);
    					fwif_color_safe_od_enable(0);
    					drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
						Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_FRAME_SYNC);
    					fwif_color_safe_od_enable(1);
    				}
    				modestate_set_fll_running_flag(_DISABLE);
				}
			} else if(vbe_get_VDEC4K_run_datafs_condition()){
				rtd_pr_vbe_notice("[%s][line:%d] vdec 4k default run timing fs.\n", __FUNCTION__, __LINE__);
				fwif_color_safe_od_enable(0);
				drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
				Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_FRAME_SYNC);
				fwif_color_safe_od_enable(1);
			} else {
				if((get_vsc_src_is_hdmi_or_dp()) && (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag())){
					//modestate_set_fll_running_flag(_DISABLE);
					vbe_disp_set_VRR_fractional_framerate_mode();
					fwif_color_safe_od_enable(0);
					drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
					Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_FRAME_SYNC);
					fwif_color_safe_od_enable(1);
					scaler_set_VRR_pwm_frequency_update_in_isr(TRUE);
				}
				else if(vbe_get_HDMI_run_timing_framesync_condition()){
					rtd_pr_vbe_notice("[%s][line:%d] HDMI 4k default run timing fs.\n", __FUNCTION__, __LINE__);
					drvif_UHD_HDMI_increase_clk();
					fwif_color_safe_od_enable(0);

#ifdef CONFIG_MDOMAIN_FORCE_FRC
					if (TRUE == modestate_get_mdomain_frc_flag(SLR_MAIN_DISPLAY))
					{
						rtd_pr_vbe_notice("[%s][line:%d] HDMI 4k , modestate_get_mdomain_frc_flag=TRUE, run timing frc.\n", __FUNCTION__, __LINE__);
						drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
						Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_FRC);
					}
					else
#endif
					{
						drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
						Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_FRAME_SYNC);
					}
					fwif_color_safe_od_enable(1);
				}
				else
				{
#ifdef CONFIG_MDOMAIN_FORCE_FRC
					if (TRUE == modestate_get_mdomain_frc_flag(SLR_MAIN_DISPLAY))
					{
						rtd_pr_vbe_notice("[%s][line:%d] HDMI non 4k,modestate_get_mdomain_frc_flag=TRUE, run timing frc.\n", __FUNCTION__, __LINE__);
						drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
						Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_FRC);
					}
					else
#endif
					{
						modestate_set_fll_running_flag(_ENABLE);
						Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_NEW_FLL_MODE);
					}
				}
			}
			#else
				modestate_set_fll_running_flag(_ENABLE);
				Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_NEW_FLL_MODE);
			#endif
#ifndef BUILD_QUICK_SHOW 

			if(Scaler_InputSrcGetType(display) == _SRC_TV){
				if(drvif_module_vdc_read_Clamping_path() && drvif_module_vdc_read_scan_flag()){
					rtd_pr_vbe_notice("### ATV Channel Scan !!! ###\n");
					modestate_set_fll_running_flag(FALSE);
				}
			}
#endif
		}
	}

#endif
		ppoverlay_uzudtg_countrol_reg.regValue = IoReg_Read32(PPOVERLAY_UZUDTG_COUNTROL_reg);
		ppoverlay_uzudtg_countrol_reg.uzudtg_mode_revised = 1;
		IoReg_Write32(PPOVERLAY_UZUDTG_COUNTROL_reg, ppoverlay_uzudtg_countrol_reg.regValue);
		rtd_pr_vbe_notice("uzudtg_mode_revised == 1\n");


		if(drv_memory_get_low_delay_game_mode_dynamic())	//WOSQRTK-10203 need to do this early	(patch for QA stage)
			Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);
#ifndef BUILD_QUICK_SHOW 
		if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
			state_update_disp3d_info();
#endif
		if(_FALSE == Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))
		{
		//	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
		//		state_update_disp3d_info();
			drvif_scaler_ddomain_switch_irq(_ENABLE);
		}
#ifdef CONFIG_DCLK_CHECK
		framesync_syncDoubleDVS_Frc_Tracking_Framesync(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), _DISABLE);
#endif //#ifdef CONFIG_DCLK_CHECK
		drvif_scalerdisplay_set_sharpness_line_mode();
		if ((Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI))
		{
			drv_adjust_spreadspectrumrange(0);
		}
#ifndef BUILD_QUICK_SHOW	
		// Add for PQ part by Hawaii 20150413
		if (!vpq_get_handler_bypass()) {
			fwif_color_handler();
			fwif_color_video_quality_handler();
			fwif_color_vpq_pic_init();
			fwif_color_set_PIP_overlap_area_color_selection(1);
			fwif_color_SetAutoMAFlag(1);
			Scaler_Set_OD_Bit_By_XML();

			Scaler_Set_VIP_Disable_PQ(S_ModuleTestMode.ModuleTestMode_Flag, S_ModuleTestMode.ModuleTestMode_en);
		}
/*#else
		unsigned int before_time=0,after_time=0;
		before_time = get_boot_time_us();
		fwif_color_video_quality_handler();
		after_time = get_boot_time_us();
		printf("\n PQ total time =%d , before time =%d, after time =d  \n",after_time-before_time,before_time, after_time);*/ 
#endif

		/*
		if((Scaler_InputSrcGetType(display) == _SRC_VO) && ((get_HDMI_HDR_mode() != HDR_MODE_DISABLE) || (get_OTT_HDR_mode() != HDR_MODE_DISABLE) || ((IoReg_Read32(DOLBY_V_TOP_TOP_CTL_reg) & 7) != 0))){
			rtd_pr_vbe_notice("[DolbyVision] IGNORE VIP FOR DOLBY VISION\n");
			fwif_color_disable_VIP(_ENABLE);
		}*/
		// ======= Add for PQ part End ========
#ifndef BUILD_QUICK_SHOW 

		if (Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
		{
			//USER : LewisLee DATE : 2012/11/07
			//only 3D mode need to apply all
			//to pevent 2D -> 3D -> 2D, set aspect ratio, the picture abnormal
			//so need to set scaler_2Dcvt3D_set_userEnableStatus(_FALSE)
			if(Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)==SLR_3D_2D_CVT_3D)
			{
				scaler_2Dcvt3D_set_userEnableStatus(_ENABLE);
				scaler_2Dcvt3D_set_shifter(Scaler_Get2DCvt3D_Strength(), Scaler_Get2DCvt3D_Depth());
				scaler_2Dcvt3D_set_tilt_depth(scaler_2Dcvt3D_get_tilt_enable());

			}
			else
				scaler_2Dcvt3D_set_userEnableStatus(_DISABLE);
		}
#endif
		rtk_hal_vsc_GetOutputRegion(VIDEO_WID_0, &outregion);
		if (!((outregion.x == 0)&&(outregion.y == 0)&&(outregion.w == 0)&&(outregion.h == 0)) ){
#if 0//Move after memc
			if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == FALSE){
				unsigned int count = 20;
				cap_ddr_fifostatus_RBUS cap_ddr_fifostatus_reg;
				/*check memory cap down*/
				cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
				while(!(cap_ddr_fifostatus_reg.in1_cap_last_wr_flag) && --count){
					cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
					mdelay(10);
				}
				rtd_pr_vbe_debug("M domain done\n");
				if(count==0)
					rtd_pr_vbe_err("M domain timeout !\n");
			}
#endif
			down(get_forcebg_semaphore());
			drvif_scalerdisplay_enable_display(SLR_MAIN_DISPLAY, _ENABLE);
			up(get_forcebg_semaphore());

			//first boot run Main done
			if(first_boot_run_main == FALSE){
				first_boot_run_main = TRUE;
			}
		}

	} else if ((SCALER_DISP_CHANNEL)display == SLR_SUB_DISPLAY) {
#if 0//Move after memc
		if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == FALSE){
			unsigned int count = 10;
			cap_ddr_fifostatus_RBUS cap_ddr_fifostatus_reg;
			/*check memory cap down*/
			cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
			while(!(cap_ddr_fifostatus_reg.in2_cap_last_wr_flag) && count--){
				cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
				mdelay(10);
			}
			rtd_pr_vbe_debug("Sub M domain done\n");
			if(count==0)
				rtd_pr_vbe_err("Sub M domain timeout !\n");
		}
#endif
		//if first run sub path, it should set disp timing. @Crixus 20151230
		//sync benwang:for k3lp project no mutilview case, remove (drvif_scalerdisplay_get_force_bg_status()) condition for livezoom sub no need set timing(qiangzhou20161104)
		if((FALSE == first_boot_run_main)/*||(drvif_scalerdisplay_get_force_bg_status())*/){
			rtd_pr_vbe_debug("First boot run Sub!!!!\n");
#if 0
			modestate_disp_flow_double_buf_enable(_ENABLE);
			modestate_set_display_timing_genByDisp(display);//set dvtotal
			modestate_set_display_timing_gen_clock_div();
			modestate_set_fractional_framesync();
#ifndef CONFIG_MEMC_BYPASS	//bypass memc rzhen@2016-06-29
			//add ME protection when change frame rate @Crixus 20151221
			if((IoReg_Read32(KME_DM_TOP0_KME_05_AGENT_reg) & _BIT20) || (IoReg_Read32(me_05_write_enable_ADDR) & _BIT20)){
				me_protection_flag = 1;//if ME si disable, not to protect.
			}
			if(me_protection_flag == 1){
				IoReg_ClearBits(KME_DM_TOP0_KME_04_AGENT_reg, _BIT20);
				IoReg_ClearBits(KME_DM_TOP0_KME_05_AGENT_reg, _BIT20);
				IoReg_SetBits(KME_ME1_TOP1_ME1_SCENE1_00_reg, _BIT0);
				WaitFor_DEN_STOP();
				WaitFor_DEN_STOP();
				rtd_pr_vbe_debug("turn off me4.5 ok in disp_setting\n");
			}
			modestate_disp_flow_double_buf_apply();
			modestate_disp_flow_double_buf_enable(_DISABLE);
			if(me_protection_flag == 1){
				IoReg_ClearBits(KME_ME1_TOP1_ME1_SCENE1_00_reg, _BIT0);
				IoReg_SetBits(KME_DM_TOP0_KME_04_AGENT_reg, _BIT20);
				IoReg_SetBits(KME_DM_TOP0_KME_05_AGENT_reg, _BIT20);
				WaitFor_DEN_STOP();
				me_protection_flag = 0;
				rtd_pr_vbe_debug("turn on me4.5 ok in disp_setting\n");
			}
#endif
#else
            if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE){
				if((PPOVERLAY_uzudtg_fractional_fsync_get_uzudtg_remove_half_ivs_mode(IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg))!=0) ||
						(PPOVERLAY_uzudtg_fractional_fsync_get_uzudtg_multiple_vsync(IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg))!=0)){
					//enable MEMC clock
					Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);
					//set MEMC mux enable
					Scaler_MEMC_Bypass_On(0);
			//		vbe_dtg_memec_bypass_switch_set(0);
					rtd_pr_vbe_notice("[Scaler_disp_setting]120Hz panel sub path set MEMC on\n");
				}
			}
			down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
			//double buffer D0
			spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
			double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
			double_buffer_ctrl_reg.dreg_dbuf_en = 0;
			double_buffer_ctrl_reg.drm_multireg_dbuf_en = 1;
			double_buffer_ctrl_reg.drm_multireg_dbuf_read_sel = 1;
			double_buffer_ctrl_reg.drm_multireg_dbuf_vsync_sel = 1;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
			spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
			up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
			ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
			ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
			ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

			drivf_scaler_reset_freerun();

			//pre-flow. patch, let D8 double buffer works well
			count = 0x3fffff;
			while((count == 0x3fffff)&&(mul_dbuf_cnt<10000)){
				//msleep(10);
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
					rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_set 1 wait timeout !!!\n");
				}
				mul_dbuf_cnt++;
			}
			modestate_set_fractional_framesyncByDisp(display);

			count = 0x3fffff;
			while((count == 0x3fffff)&&(mul_dbuf_cnt2<10000)){
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
				mul_dbuf_cnt2++;
			}
			modestate_set_display_timing_gen_clock_divByDisp(display);
			count = 0x3fffff;
			dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
			dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=0;
			IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
			dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
			//stage1 clk mode revised default set to 1
			dispd_smooth_toggle1_reg.dispd_stage1_clk_mode_revised = 1;
			dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_src_sel = 1;
			dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
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
			ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
			ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
			ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
			IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
			modestate_set_display_timing_genByDisp(display);//set dvtotal

#endif

#ifdef ENABLE_VBY1_TO_HDMI_4K2K_DATA_FS
			if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)){
				//drvif_framesync_gatting(_DISABLE);
				drvif_framesync_gatting_do();
				msleep(50);
				//drvif_framesync_gatting(_ENABLE);
				if ((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)) {
					modestate_set_fll_running_flag(_DISABLE);
				}
			}else{
				modestate_set_fll_running_flag(_ENABLE);
				if(Scaler_InputSrcGetType(display) == _SRC_TV){
					if(drvif_module_vdc_read_Clamping_path() && drvif_module_vdc_read_scan_flag()){
						rtd_pr_vbe_notice("### ATV Channel Scan !!! ###\n");
						modestate_set_fll_running_flag(FALSE);
					}
				}
			}
#else
			if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)){
				//gating clock apply first, then enable gating
				//drvif_framesync_gatting(_DISABLE);
				drvif_framesync_gatting_do();
				msleep(50);
				//drvif_framesync_gatting(_ENABLE);
			}else{
			#ifdef SMOOTH_TOGGLE_DATAFRCFS_TEST
				if(vbe_disp_get_adaptivestream_fs_mode()){
					modestate_set_fll_running_flag(_DISABLE);
                    		fwif_color_safe_od_enable(0);
					drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
					Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_FRAME_SYNC);
					fwif_color_safe_od_enable(1);
				}else{
					modestate_set_fll_running_flag(_ENABLE);
					Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_NEW_FLL_MODE);
				}
			#else
				modestate_set_fll_running_flag(_ENABLE);
				Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_NEW_FLL_MODE);
			#endif
				if(Scaler_InputSrcGetType(display) == _SRC_TV){
					if(drvif_module_vdc_read_Clamping_path() && drvif_module_vdc_read_scan_flag()){
						rtd_pr_vbe_notice("### ATV Channel Scan !!! ###\n");
						modestate_set_fll_running_flag(FALSE);
						Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_FRC);
					}
				}
			}
#endif

	if(drv_memory_get_low_delay_game_mode_dynamic())	//WOSQRTK-10203 need to do this early	(patch for QA stage)
		Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);

			//if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
				//state_update_disp3d_info();

			if(_FALSE == Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))
			{
				drvif_scaler_ddomain_switch_irq(_ENABLE);
			}
#ifdef CONFIG_DCLK_CHECK
			framesync_syncDoubleDVS_Frc_Tracking_Framesync(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), _DISABLE);
#endif //#ifdef CONFIG_DCLK_CHECK
			drvif_scalerdisplay_set_sharpness_line_mode();
			if ((Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI) ||
				(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) ||
				(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)) {
				drv_adjust_spreadspectrumrange(0);
			}

		}
		down(get_forcebg_semaphore());
		drvif_scalerdisplay_enable_display(SLR_SUB_DISPLAY, _ENABLE);
		up(get_forcebg_semaphore());
		drvif_color_setpiprgbcontrastbrightness(0);
		fwif_color_set_PIP_overlap_area_color_selection(1);
	}
	up(&VBE_Semaphore);
#endif
}

unsigned int Get_Input_vfreq_by_vsync_cnt(void)
{
	ppoverlay_ivs_cnt_3_RBUS uzudtg_ivs_cnt_3_reg;
	unsigned int vfreq = 0;
	unsigned int ivs_cnt = 0;
	unsigned int XTAL_CNT= 27000000;
	unsigned char count = 6;

	while(count){
		uzudtg_ivs_cnt_3_reg.regValue = IoReg_Read32(PPOVERLAY_IVS_cnt_3_reg);
		ivs_cnt = uzudtg_ivs_cnt_3_reg.ivs_cnt_ori;
		rtd_pr_vbe_info("[WARN] IVS N.\n");
		if(ivs_cnt > 0){
			vfreq = (XTAL_CNT*100)/ivs_cnt;
		}
		if(ABS(vfreq/10, Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_INPUT_V_FREQ))>10){
			rtd_pr_vbe_err("[WARN] IVS Freq not ready.(%d.%d)\n", vfreq, Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_V_FREQ));
			count--;
		}else{
			// ivs correct
			break;
		}
		msleep(10);
	}


	if(vfreq > 0){
		if((vfreq >= 5999) && (vfreq <= 6001))
			vfreq = 6000;
		else if((vfreq >= 5992) && (vfreq <= 5995))
			vfreq = 5994;
		else if((vfreq >= 2999) && (vfreq <= 3001))
			vfreq = 3000;
		else if((vfreq >= 2996) && (vfreq <= 2998))
			vfreq = 2997;
		else if((vfreq >= 2399) && (vfreq <= 2401))
			vfreq = 2400;
		else if((vfreq >= 2396) && (vfreq <= 2398))
			vfreq = 2397;
		else if((vfreq >= 4790) && (vfreq <= 4810))
			vfreq = 4800;
		else if((vfreq >= 4990) && (vfreq <= 5010))
			vfreq = 5000;
		else{
			rtd_pr_vbe_err("[WARN] Not normalized ivs freq.(%d)\n", vfreq);
		}
	}

	if(count == 0){
		// if ivs count is diff offline measure more than 1 hz, use offline measure result.
		vfreq = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_INPUT_V_FREQ)*10;
	}

	if(vfreq == 0){
		//it should be never happen
		rtd_pr_vbe_err("[WARN] IVS Freq is zero.\n");
		vfreq= 6000;
	}
	return vfreq;
}

extern HDMI_bool drvif_Hdmi_GetRawTimingInfo(MEASURE_TIMING_T* timing_info);
unsigned int Get_Input_vfreq_by_HDMI_ori_vfreq(void)
{
#ifdef CONFIG_ENABLE_HDMI_PATTERN
	extern unsigned char Hdmi_GetRawTimingInfo_for_hdmi_ptg(MEASURE_TIMING_T* timing_info);
#endif
	unsigned int vfreq = 0;
#ifdef CONFIG_RTK_HDMI_RX
	MEASURE_TIMING_T vfehdmitiminginfo;
#ifdef CONFIG_ENABLE_HDMI_PATTERN
	Hdmi_GetRawTimingInfo_for_hdmi_ptg(&vfehdmitiminginfo);
#else
	drvif_Hdmi_GetRawTimingInfo(&vfehdmitiminginfo);
#endif
#endif

	if(get_scaler_qms_mode_flag())
	{//qms case
		vfreq = get_qms_current_framerate() / 10;
		if(vfreq == 0)
		{
			vfreq = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_INPUT_V_FREQ)*10;
			rtd_pr_vbe_err("[%s][%d] Errr QMS get frame rate is 0 \n", __FUNCTION__, __LINE__);
		}
	}
	else
	{
#ifdef CONFIG_RTK_HDMI_RX	
	vfreq = vfehdmitiminginfo.v_freq / 10;
#endif
	if((vfreq > 0) && !drvif_i3ddma_triplebuf_flag()){
		if((vfreq >= 5999) && (vfreq <= 6001))
			vfreq = 6000;
		else if((vfreq >= 5992) && (vfreq <= 5995))
			vfreq = 5994;
		else if((vfreq >= 2999) && (vfreq <= 3001))
			vfreq = 3000;
		else if((vfreq >= 2996) && (vfreq <= 2998))
			vfreq = 2997;
		else if((vfreq >= 2399) && (vfreq <= 2401))
			vfreq = 2400;
		else if((vfreq >= 2396) && (vfreq <= 2398))
			vfreq = 2397;
		else if((vfreq >= 4790) && (vfreq <= 4810))
			vfreq = 4800;
		else if((vfreq >= 4990) && (vfreq <= 5010))
			vfreq = 5000;
		else if((vfreq >= 9990) && (vfreq <= 10010))
			vfreq = 10000;
		else if((vfreq >= 11982) && (vfreq <= 11990))
			vfreq = 11988;
		else if((vfreq > 11990) && (vfreq <= 12010))
			vfreq = 12000;
		else if((vfreq > 14380) && (vfreq <= 14388))
			vfreq = 14385;
		else if((vfreq > 14389) && (vfreq <= 14410))
			vfreq = 14400;
		else if((vfreq > 23900) && (vfreq <= 24100))
			vfreq = 24000;
		else{
			rtd_pr_vbe_err("[WARN] Not normalized ivs freq.(%d)\n", vfreq);
			vfreq = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_INPUT_V_FREQ)*10;
		}
	}
	else{
		vfreq = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_INPUT_V_FREQ)*10;
	}
        }
	rtd_pr_vbe_notice("[%s][%d] vFreq =%d \n", __FUNCTION__, __LINE__,vfreq);

	if(vfreq == 0){
		//it should be never happen
		rtd_pr_vbe_err("[WARN] IVS Freq is zero.\n");
		vfreq= 6000;
	}
	return vfreq;
}

unsigned int scaler_disp_smooth_get_memc_protect_vtotal(void)
	{
		//unsigned char pixelMode=0;
		ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
//		dv_total_RBUS dv_total_reg;
//		uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
//		memcdtg_dv_total_RBUS memcdtg_dv_total_reg;
		UINT32 vFreq=0;
		unsigned int vertical_total=0;
            unsigned long long temp;
//#ifdef CONFIG_MEMC_BYPASS
		ppoverlay_dtg_m_remove_input_vsync_RBUS ppoverlay_dtg_m_remove_input_vsync_reg;
		ppoverlay_dtg_m_multiple_vsync_RBUS ppoverlay_dtg_m_multiple_vsync_reg;

//#else
		ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
//#endif
		unsigned int removeRatio=0;
		unsigned int multipleRatio=0;
        unsigned int outputFrameRate=0;
		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
		//pixelMode = 0; //display_timing_ctrl2_reg.dout_pixel_mode;
		vFreq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);

		// avoid truncate problem.
		if(vFreq==239)
			vFreq = 2397;
		else if(vFreq==299){
			unsigned int ivs_cnt = PPOVERLAY_IVS_cnt_3_get_ivs_cnt_ori(IoReg_Read32(PPOVERLAY_IVS_cnt_3_reg));
			rtd_pr_vbe_notice("[%s] ivs_cnt=%x\n", __FUNCTION__,ivs_cnt);
			if((ivs_cnt < (2700000000UL/2999))&& (ivs_cnt > (2700000000UL/3001)))
				vFreq = 3000;
			else
				vFreq = 2997;
		}
		else if(vFreq==599)
			vFreq = 5994;
		else
			vFreq = vFreq*10;

		//check scaler no break case, otherwise it may get next source timing info. (KTASKWBS-19909)
		if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)&&(!judge_scaler_break_case(SLR_MAIN_DISPLAY))){
#ifndef USE_NEW_HDMI_TIMING_INFO_STRUCTURE
			if(((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245)) ||
			((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305)) ||
			((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 595) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 605)))
				vFreq = Get_Input_vfreq_by_vsync_cnt();
#else
			vFreq = Get_Input_vfreq_by_HDMI_ori_vfreq();
#endif
		}else if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC || Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_CAMERA){
			if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305))
				vFreq = Get_Input_vfreq_by_vsync_cnt();
		}
//#ifdef CONFIG_MEMC_BYPASS
		if(get_MEMC_bypass_status_refer_platform_model()){
            ppoverlay_dtg_m_multiple_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_multiple_vsync_reg);
            ppoverlay_dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
            removeRatio = ppoverlay_dtg_m_remove_input_vsync_reg.remove_half_ivs_mode2+1;
            multipleRatio = ppoverlay_dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync+1;
		}
//#else
		else{
			uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
			removeRatio = uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync+1;
			multipleRatio = uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode+1;
		}
//#endif
#if 0
		if(get_panel_pixel_mode() > PANEL_1_PIXEL){
			if((removeRatio == 1) && (multipleRatio == 5)){
				//24Hz, remove=2, multiple=5
				removeRatio = 2;
			}
			else{
				if(multipleRatio >= 2)
					multipleRatio/= 2;
			}
		}
#endif
#if 0
		if(((vFreq>2350)&&(vFreq<=2450)) || //24hz
		   ((vFreq>2450)&&(vFreq<=2550)) || //25hz
		   ((vFreq>2950)&&(vFreq<=3050)) || //30hz
		   ((vFreq>4750)&&(vFreq<=4850)) || //48hz
		   ((vFreq>4950)&&(vFreq<=5050)) || //50hz
		   ((vFreq>5950)&&(vFreq<=6050)))   //60hz
		{
            if(get_panel_pixel_mode() > PANEL_1_PIXEL)
            {
                temp = div64_u64((unsigned long long)Get_DISPLAY_CLOCK_TYPICAL()*200,(Get_DISP_HORIZONTAL_TOTAL()));
                vertical_total = div64_u64(div64_u64(temp,vFreq)*removeRatio,multipleRatio);
            }
            else
            {
                temp = div64_u64((unsigned long long)Get_DISPLAY_CLOCK_TYPICAL()*100,(Get_DISP_HORIZONTAL_TOTAL()));
                vertical_total = div64_u64(div64_u64(temp,vFreq)*removeRatio,multipleRatio);
            }
		}
        else
		{
				if((get_panel_pixel_mode() > PANEL_1_PIXEL) &&
				(((vFreq>9950)&&(vFreq<=10050)) ||	//100hz
				((vFreq>11950)&&(vFreq<=12050)) ||	//120hz
				((vFreq>14350)&&(vFreq<=14450))))	//144hz
			{
                temp = div64_u64((unsigned long long)Get_DISPLAY_CLOCK_TYPICAL()*200,(Get_DISP_HORIZONTAL_TOTAL()));
                vertical_total = div64_u64(div64_u64(temp,vFreq)*removeRatio,multipleRatio);
			}
			else{
				// unsupport input frame rate, do error handle.
				vertical_total = Get_DISP_VERTICAL_TOTAL();
				rtd_pr_vbe_notice("[%s][ERR] invalid vfreq(%d), protect vtotal=%d\n", __FUNCTION__, vFreq, vertical_total);
			}
		}
#endif
        outputFrameRate = get_outputFramerate(vFreq, removeRatio, multipleRatio);//(intputFrameRate*multipleRatio)/removeRatio;
        if(get_panel_pixel_mode() > PANEL_1_PIXEL)
        {
            //verticalTotal = ((Get_DISPLAY_CLOCK_TYPICAL()/Get_DISP_HORIZONTAL_TOTAL())*200)/outputFrameRate; // multiple 100 and shift 1 for rounding
            //verticalTotal = ((verticalTotal + 1) >> 1) & 0xFFFFFFFE;
            temp = div64_u64((unsigned long long)Get_DISPLAY_CLOCK_TYPICAL()*200, (Get_DISP_HORIZONTAL_TOTAL()));
            vertical_total = div64_u64(temp, outputFrameRate);
        }
        else
        {
            temp = div64_u64((unsigned long long)Get_DISPLAY_CLOCK_TYPICAL()*100, (Get_DISP_HORIZONTAL_TOTAL()));
            vertical_total = div64_u64(temp, outputFrameRate);
        }
    
        rtd_pr_vbe_notice("[%s %d] vertical_total  = %d\n", __FUNCTION__, __LINE__, vertical_total);

		//OLED protect dvtotal due to panel spec
		if(Get_DISPLAY_PANEL_OLED_TYPE() == TRUE){
			if(vertical_total < OLED_PANEL_DVTOTAL_MIN){
				rtd_pr_vbe_notice(" verticalTotal(%d) out of OLED panel spec (min=%d)\n", vertical_total, OLED_PANEL_DVTOTAL_MIN);
				vertical_total = OLED_PANEL_DVTOTAL_MIN + 1;
			}
		}

		//check panel boundary
		if(vertical_total <= Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN())
		{
			rtd_pr_vbe_notice(" verticalTotal(%d) out of boundary (min=%d)\n", vertical_total, Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN());
			vertical_total = Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN();
		}
		return vertical_total;

	}

unsigned int scaler_disp_smooth_get_memc_protect_vtotal_By_100(void)
{
	//unsigned char pixelMode=0;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	UINT32 vFreq=0;
	unsigned int vertical_total=0;
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	unsigned int removeRatio=0;
	unsigned int multipleRatio=0;
    unsigned int outputFrameRate=0;
	unsigned long long temp;
	ppoverlay_dtg_m_remove_input_vsync_RBUS ppoverlay_dtg_m_remove_input_vsync_reg;
	ppoverlay_dtg_m_multiple_vsync_RBUS ppoverlay_dtg_m_multiple_vsync_reg;

	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	//pixelMode = 0; //display_timing_ctrl2_reg.dout_pixel_mode;
	vFreq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
    if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI){
#ifndef USE_NEW_HDMI_TIMING_INFO_STRUCTURE
		if(((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245)) ||
		((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 295) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 305)) ||
		((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 595) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 605)))
			vFreq = Get_Input_vfreq_by_vsync_cnt();
#else
		vFreq = Get_Input_vfreq_by_HDMI_ori_vfreq();
#endif

     }

	// avoid truncate problem.
	if(vFreq==239)
		vFreq = 2397;
	else if(vFreq==299)
		vFreq = 2997;
	else if(vFreq==599)
		vFreq = 5994;
	else
		vFreq = vFreq*10;

	if(get_MEMC_bypass_status_refer_platform_model()){
		ppoverlay_dtg_m_multiple_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_multiple_vsync_reg);
		ppoverlay_dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
		removeRatio = ppoverlay_dtg_m_remove_input_vsync_reg.remove_half_ivs_mode2+1;
		multipleRatio = ppoverlay_dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync+1;
	}
	else{
		uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	 
		removeRatio = uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync+1;
		multipleRatio = uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode+1;
	}

#if 0 
	if(Get_DISPLAY_REFRESH_RATE() >= 120){
		if((removeRatio == 1) && (multipleRatio == 5)){
			//24Hz, remove=2, multiple=5
			removeRatio = 2;
		}
		else
		{
			if(multipleRatio >= 2)
				multipleRatio/= 2;
		}
	}
#endif
#if 0
    if(((vFreq>2350)&&(vFreq<=2450)) || //24hz
    ((vFreq>2450)&&(vFreq<=2550)) || //25hz
    ((vFreq>2950)&&(vFreq<=3050)) || //30hz
    ((vFreq>4750)&&(vFreq<=4850)) || //48hz
    ((vFreq>4950)&&(vFreq<=5050)) || //50hz
    ((vFreq>5950)&&(vFreq<=6050)))   //60hz
    {
        if(get_panel_pixel_mode() > PANEL_1_PIXEL)
        {
            temp = div64_u64((unsigned long long)Get_DISPLAY_CLOCK_TYPICAL()*200,(Get_DISP_HORIZONTAL_TOTAL()));
            temp = temp * 100;
            vertical_total = div64_u64(div64_u64(temp,vFreq)*removeRatio,multipleRatio);
        }
        else
        {
            temp = div64_u64((unsigned long long)Get_DISPLAY_CLOCK_TYPICAL()*100,(Get_DISP_HORIZONTAL_TOTAL()));
            temp = temp*100;
            if((vFreq == 2397)&&(removeRatio == 2)&&(multipleRatio == 5))
                vertical_total = div64_u64(temp,5994);
            else
                vertical_total = div64_u64(div64_u64(temp,vFreq)*removeRatio,multipleRatio);
        }
    }
    else
    {
        vertical_total =(PPOVERLAY_DV_total_get_dv_total(IoReg_Read32(PPOVERLAY_DV_total_reg)))*100;
    }
    
    rtd_pr_vbe_notice("[%s] vFreq =%d, mul/remove=%d.%d. vertical_total=%d \n", __FUNCTION__, vFreq, multipleRatio, removeRatio, vertical_total);
    #endif
    outputFrameRate = get_outputFramerate(vFreq, removeRatio, multipleRatio);//(intputFrameRate*multipleRatio)/removeRatio;
    if(get_panel_pixel_mode() > PANEL_1_PIXEL)
    {
        //verticalTotal = ((Get_DISPLAY_CLOCK_TYPICAL()/Get_DISP_HORIZONTAL_TOTAL())*200)/outputFrameRate; // multiple 100 and shift 1 for rounding
        //verticalTotal = ((verticalTotal + 1) >> 1) & 0xFFFFFFFE;
        temp = div64_u64((unsigned long long)Get_DISPLAY_CLOCK_TYPICAL()*200,(Get_DISP_HORIZONTAL_TOTAL()));
        temp = temp * 100;
        vertical_total = div64_u64(temp, outputFrameRate);
    }
    else
    {
        temp = div64_u64((unsigned long long)Get_DISPLAY_CLOCK_TYPICAL()*100,(Get_DISP_HORIZONTAL_TOTAL()));
        temp = temp * 100;
        vertical_total = div64_u64(temp, outputFrameRate);
    }
    
    rtd_pr_vbe_notice("[%s %d] vertical_total  = %d\n", __FUNCTION__, __LINE__, vertical_total);

    //check panel boundary
    if(vertical_total <= Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN())
    {
        rtd_pr_vbe_notice(" verticalTotal(%d) out of boundary (min=%d)\n", vertical_total, Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN());
        vertical_total = Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN();
    }
    return vertical_total;
}
unsigned char vbe_disp_get_orbit_mode(void)
{
	return vbe_disp_oled_orbit_mode;
}

/*api which have protect by forcebg_semaphore*/
unsigned char vbe_disp_get_orbit(void)
{
	return vbe_disp_oled_orbit_enable;
}

extern void Set_GamemodeSaveI3ddmaGetcurMode(int value);
void Scaler_hdmi_updata_scaler_info(void)
{

	MEASURE_TIMING_T timing;
	drvif_Hdmi_GetRawTimingInfo(&timing);
	Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IVSTARTPOS,timing.v_act_sta);
	Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IHSTARTPOS,timing.h_act_sta);
	Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IVHEIGHT,timing.v_act_len);
	Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IHWID,timing.h_act_len);
	Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IVLEN,timing.v_total);
	Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IHLEN,timing.h_total);
	Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IVCOUNT,timing.v_total);
	Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IHCOUNT,timing.h_total);
	Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IVFREQ,lib_measure_vfreq_prescion_3_to_1(timing.v_freq));
	Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IHFREQ,timing.h_freq/100);
	Hdmi_Search_Mode(&timing.mode_id);
	Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_MODE_CURR, timing.mode_id);
	//Set_GamemodeSaveI3ddmaGetcurMode(timing.mode_id);
}

#ifndef BUILD_QUICK_SHOW
void scaler_disp_smooth_variable_setting(void)
{
	unsigned char frame_idicator=0;
	unsigned char b_run_frame_check=0;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS dispd_stage1_sm_ctrl_reg;
	ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
	unsigned int count = 0x3fffff;
//#ifndef CONFIG_MEMC_BYPASS
	//unsigned char me_protection_flag = 0;
//#endif
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS	 dispd_smooth_toggle1_reg;

	if(vbe_disp_get_adaptivestream_fs_mode() == 1){
//#ifdef CONFIG_MEMC_BYPASS
			// do nothing.
//#else
		if(get_MEMC_bypass_status_refer_platform_model() == FALSE){
			Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);
		}
//#endif
		return;
	}

	rtd_pr_vbe_debug("scaler_disp_smooth_variable_setting\n");
	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
	IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);

	if(Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO) > 1){
		b_run_frame_check = 1;
		frame_idicator = Scaler_DispGetInputInfo(SLR_INPUT_FRAC_FS_MULTIPLE_RATIO)-1;
	}

	modestate_decide_pixel_mode();
	modestate_decide_dtg_m_mode();
	modestate_decide_fractional_framesync();
	#ifdef SMOOTH_TOGGLE_DATAFRCFS_TEST
	if(vbe_disp_get_adaptivestream_fs_mode() == 0){
		drivf_scaler_reset_freerun();
	}
	#else
	drivf_scaler_reset_freerun();
	#endif
	modestate_disp_flow_double_buf_enable(_ENABLE);
	//modestate_set_display_timing_gen();
	modestate_set_display_timing_gen_clock_div();
	modestate_set_fractional_framesync();

	if(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 60)
		rtd_pr_vbe_debug("######## 48Hz to 60HZ #############\n");
	else if(Scaler_DispGetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ) == 48)
		rtd_pr_vbe_debug("######## 60Hz to 48HZ #############\n");
	rtd_pr_vbe_debug("decide frame_idicator:%d\n", frame_idicator);
//#ifndef CONFIG_MEMC_BYPASS
#if 0
	if(get_MEMC_bypass_status_refer_platform_model() == FALSE) {

		//add ME protection when change frame rate @Crixus 20151221
		if((IoReg_Read32(KME_DM_TOP0_KME_04_AGENT_reg) & _BIT20) || (IoReg_Read32(KME_DM_TOP0_KME_05_AGENT_reg) & _BIT20)){
			me_protection_flag = 1;//if ME is disable, not to protect.
		}
		if(me_protection_flag == 1){
			//disable ME DMA
			IoReg_ClearBits(KME_DM_TOP0_KME_04_AGENT_reg, _BIT20);
			IoReg_ClearBits(KME_DM_TOP0_KME_05_AGENT_reg, _BIT20);
			IoReg_SetBits(KME_ME1_TOP1_ME1_SCENE1_00_reg, _BIT0);
			WaitFor_DEN_STOP();
			WaitFor_DEN_STOP();
			rtd_pr_vbe_debug("turn off me4.5 ok in smooth setting\n");
		}
	}
#endif
//#endif
	if(b_run_frame_check == 1){
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
		//rtd_pr_vbe_debug("++frame_idicator:%d\n", (IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)&0x3f000000)>>24);
	}else{
		ppoverlay_new_meas1_linecnt_real_RBUS new_meas1_linecnt_real_reg;
		new_meas1_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);
		while((new_meas1_linecnt_real_reg.memcdtg_line_cnt_rt > 200)&& --count){
			new_meas1_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);
		}
	}
	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	modestate_set_fractional_framesync();
	fw_scaler_dtg_double_buffer_apply();
	rtd_pr_vbe_debug("--- 1frame_idicator:%d\n",(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)&0x3f000000)>>24);

    	count = 0x3fffff;
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	while((double_buffer_ctrl_reg.dreg_dbuf_set)&& --count){
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	}
	if(count==0){
		rtd_pr_vbe_notice("double buffer timeout !!!\n");
	}
	rtd_pr_vbe_debug("--- 2frame_idicator:%d\n",(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)&0x3f000000)>>24);

	count = 0x3fffff;
	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply)&& --count){
		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	}
	if(count==0){
		rtd_pr_vbe_notice("dispd_smooth_toggle1 timeout !!!\n");
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
	modestate_disp_flow_double_buf_enable(_DISABLE);
	modestate_set_display_timing_gen();//set dvtotal
//#ifndef CONFIG_MEMC_BYPASS
#if 0
	if(get_MEMC_bypass_status_refer_platform_model() == FALSE){
		if(me_protection_flag == 1){
			IoReg_ClearBits(KME_ME1_TOP1_ME1_SCENE1_00_reg, _BIT0);
			IoReg_SetBits(KME_DM_TOP0_KME_04_AGENT_reg, _BIT20);
			IoReg_SetBits(KME_DM_TOP0_KME_05_AGENT_reg, _BIT20);
			WaitFor_DEN_STOP();
			me_protection_flag = 0;
			rtd_pr_vbe_debug("turn on me4.5 ok in smooth setting\n");
		}
	}
#endif
//#endif

	rtd_pr_vbe_debug("--- 3frame_idicator:%d\n",(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)&0x3f000000)>>24);

//#ifdef CONFIG_MEMC_BYPASS
		// do nothing.
//#else
	if(get_MEMC_bypass_status_refer_platform_model() == FALSE) {
		Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);
	}
//#endif

	#ifdef SMOOTH_TOGGLE_DATAFRCFS_TEST
	if(vbe_disp_get_adaptivestream_fs_mode() == 0){
		modestate_set_fll_running_flag(_ENABLE);
		Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_NEW_FLL_MODE);
	}else{
		fwif_color_safe_od_enable(0);
		drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
		Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_FRAME_SYNC);
		fwif_color_safe_od_enable(1);
	}

	if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)){
		rtd_pr_vbe_notice("[%s][%d]P_VBY1_TO_HDMI=FRC\n",__FUNCTION__,__LINE__);
		modestate_set_fll_running_flag(_DISABLE);
		Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_FRC);
		drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
	}
	#else
	modestate_set_fll_running_flag(_ENABLE);
	Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_NEW_FLL_MODE);
	#endif

}


void scaler_disp_smooth_variable_setting_slow(void)
{
	SCALER_DISP_CHANNEL _channel = 0;
	unsigned int count = 0x3fffff;
//	unsigned int count2 = 0x3fffff;
//	ppoverlay_new_meas1_linecnt_real_RBUS ppoverlay_new_meas1_linecnt_real_reg;
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS	 dispd_smooth_toggle1_reg;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS dispd_stage1_sm_ctrl_reg;
	ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
//	ppoverlay_dtg_m_remove_input_vsync_RBUS dtg_m_remove_input_vsync_reg;
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
	ppoverlay_iv2dv_double_buffer_ctrl_RBUS iv2dv_double_buffer_reg;
	ppoverlay_uzudtg_countrol_RBUS ppoverlay_uzudtg_countrol_reg;
//	ppoverlay_uzudtg_control1_RBUS ppoverlay_uzudtg_control1_reg;
	unsigned char uzu_valid_framecnt = 0;
	UINT32 mul_dbuf_cnt=0;
//	UINT32 mul_dbuf_cnt2=0;
	//UINT32 meas0_lineCnt1=0;
	//UINT32 meas0_lineCnt2=0;
	//UINT32 meas0_lineCnt3=0;
	//UINT32 meas0_lineCnt4=0;
	//UINT32 meas0_lineCnt5=0;
	//UINT32 meas0_lineCnt6=0;
	//UINT32 meas1_lineCnt1=0;
	//UINT32 meas1_lineCnt2=0;
	//UINT32 meas1_lineCnt3=0;
	//UINT32 meas1_lineCnt4=0;
	//UINT32 meas1_lineCnt5=0;
	//UINT32 meas1_lineCnt6=0;
	UINT32 timestamp[6] = {0,0,0,0,0,0};
	unsigned long flags;//for spin_lock_irqsave

	rtd_pr_vbe_notice("scaler_disp_smooth_variable_setting_slow\n");

	//disable uzudtg mode revised to make avoid DTG settings applied fail
	ppoverlay_uzudtg_countrol_reg.regValue = IoReg_Read32(PPOVERLAY_UZUDTG_COUNTROL_reg);
	ppoverlay_uzudtg_countrol_reg.uzudtg_mode_revised = 0;
	IoReg_Write32(PPOVERLAY_UZUDTG_COUNTROL_reg, ppoverlay_uzudtg_countrol_reg.regValue);
	rtd_pr_vbe_notice("uzudtg_mode_revised == 0\n");

	if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == FALSE)
		drivf_scaler_reset_freerun();

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	//double buffer D0
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.dreg_dbuf_en = 0;
	double_buffer_ctrl_reg.drm_multireg_dbuf_en = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_read_sel = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_vsync_sel = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	//pre-flow. patch, let D8 double buffer works well
	iv2dv_double_buffer_reg.regValue = IoReg_Read32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg);
	iv2dv_double_buffer_reg.iv2dv_db_en = 0;
	iv2dv_double_buffer_reg.iv2dv_db_apply= 0;
	IoReg_Write32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg, iv2dv_double_buffer_reg.regValue);
	//pre-flow.patch,let D2 double buffer works well
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
			rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_set 1 wait timeout !!!\n");
		}
		if(double_buffer_ctrl_reg.drm_multireg_dbuf_en==0)
			rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_en=0 1!!\n");

		mul_dbuf_cnt++;
	}
	//meas0_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[0] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);

	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=0;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	//stage1 clk mode revised default set to 1
	dispd_smooth_toggle1_reg.dispd_stage1_clk_mode_revised = 1;
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_src_sel = 1;
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
	IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzu_valid_framecnt = uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync;	//if 2:5, valid_framecnt = 1, if 1:2, valid_framecnt = 0
	//wait to front porch
	count = 0x3fffff;
	dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
	while(!((dtg_frame_cnt1_reg.uzudtg_output_frame_cnt == uzu_valid_framecnt) &&
		(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)) > 100) &&
		(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)) < 1500)) && --count){
		dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
	}
	if(count == 0)
		rtd_pr_vbe_emerg("[DTG] Wait to uzu_valid_framecnt timeout!!, uzu_valid_framecnt = %d, uzudtg_output_frame_cnt = %d\n",uzu_valid_framecnt, dtg_frame_cnt1_reg.uzudtg_output_frame_cnt);

	//meas0_lineCnt2 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt2 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[1] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
	modestate_set_fractional_framesyncByDisp(_channel);
	modestate_set_display_timing_gen_clock_divByDisp(_channel);
	count = 0x3fffff;

	//meas0_lineCnt3 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt3 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[2] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_src_sel = 1;
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

	//meas0_lineCnt4 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt4 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[3] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);

	ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);

	//meas0_lineCnt5 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt5 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[4] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);
	modestate_set_display_timing_genByDisp(_channel);//set dvtotal

	//meas0_lineCnt6 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	//meas1_lineCnt6 = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	timestamp[5] = IoReg_Read32(TIMER_VCPU_CLK90K_LO_reg);

/*
	rtd_pr_vbe_emerg("[Eric] uzu_valid_framecnt = %d, uzudtg_output_frame_cnt = %d, uzudtg_input_frame_cnt = %d\n",uzu_valid_framecnt,dtg_frame_cnt1_reg.uzudtg_output_frame_cnt,dtg_frame_cnt1_reg.uzudtg_input_frame_cnt);
	rtd_pr_vbe_emerg("meas0_lineCnt1 (%d)\n", PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(meas0_lineCnt1));
	rtd_pr_vbe_emerg("meas1_lineCnt1 (%d)\n", PPOVERLAY_new_meas2_linecnt_real_uzudtg_dly_line_cnt_rt(meas1_lineCnt1));
	rtd_pr_vbe_emerg("meas0_lineCnt2 (%d)\n", PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(meas0_lineCnt2));
	rtd_pr_vbe_emerg("meas1_lineCnt2 (%d)\n", PPOVERLAY_new_meas2_linecnt_real_uzudtg_dly_line_cnt_rt(meas1_lineCnt2));
	rtd_pr_vbe_emerg("meas0_lineCnt3 (%d)\n", PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(meas0_lineCnt3));
	rtd_pr_vbe_emerg("meas1_lineCnt3 (%d)\n", PPOVERLAY_new_meas2_linecnt_real_uzudtg_dly_line_cnt_rt(meas1_lineCnt3));
	rtd_pr_vbe_emerg("meas0_lineCnt4 (%d)\n", PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(meas0_lineCnt4));
	rtd_pr_vbe_emerg("meas1_lineCnt4 (%d)\n", PPOVERLAY_new_meas2_linecnt_real_uzudtg_dly_line_cnt_rt(meas1_lineCnt4));
	rtd_pr_vbe_emerg("meas0_lineCnt5 (%d)\n", PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(meas0_lineCnt5));
	rtd_pr_vbe_emerg("meas1_lineCnt5 (%d)\n", PPOVERLAY_new_meas2_linecnt_real_uzudtg_dly_line_cnt_rt(meas1_lineCnt5));
	rtd_pr_vbe_emerg("meas0_lineCnt6 (%d)\n", PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(meas0_lineCnt6));
	rtd_pr_vbe_emerg("meas1_lineCnt6 (%d)\n", PPOVERLAY_new_meas2_linecnt_real_uzudtg_dly_line_cnt_rt(meas1_lineCnt6));
	rtd_pr_vbe_emerg("time = (%x,%x,%x,%x,%x,%x)\n", timestamp[0],timestamp[1],timestamp[2],timestamp[3],timestamp[4],timestamp[5]);
*/
	//after DTG settings, recover uzudtg mode revised.
	ppoverlay_uzudtg_countrol_reg.regValue = IoReg_Read32(PPOVERLAY_UZUDTG_COUNTROL_reg);
	ppoverlay_uzudtg_countrol_reg.uzudtg_mode_revised = 1;
	IoReg_Write32(PPOVERLAY_UZUDTG_COUNTROL_reg, ppoverlay_uzudtg_countrol_reg.regValue);
	rtd_pr_vbe_notice("uzudtg_mode_revised == 1\n");
}

void scaler_disp_smooth_variable_setting_fast(void)
{
	SCALER_DISP_CHANNEL channel = 0;
	unsigned char frame_idicator=0;
	unsigned char b_run_frame_check=0;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS dispd_stage1_sm_ctrl_reg;
	ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
	unsigned int count = 0x3fffff;
//	unsigned char me_protection_flag = 0;
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS	 dispd_smooth_toggle1_reg;
	unsigned long flags;//for spin_lock_irqsave
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;

	rtd_pr_vbe_notice("scaler_disp_smooth_variable_setting_fast\n");
	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
	IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	if(uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode>0){
		b_run_frame_check = 1;
		frame_idicator = uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode;
	}

	//modestate_decide_pixel_mode();
	//modestate_decide_dtg_m_modeByDisp(channel);
	//modestate_decide_fractional_framesyncByDisp(channel);
	drivf_scaler_reset_freerun();

	modestate_set_display_timing_gen_clock_divByDisp(channel);
	rtd_pr_vbe_notice("decide frame_idicator:%d\n", frame_idicator);

	if(b_run_frame_check == 1){
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
	}else{
		ppoverlay_new_meas2_linecnt_real_RBUS new_meas2_linecnt_real_reg;
		new_meas2_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
		while(((new_meas2_linecnt_real_reg.uzudtg_dly_line_cnt_rt>200)||(new_meas2_linecnt_real_reg.uzudtg_dly_line_cnt_rt<100))&& --count){
			new_meas2_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
		}
	}
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);

	//local_irq_disable();
	//preempt_disable();
	modestate_disp_flow_double_buf_enable(_ENABLE);
	fw_scaler_dtg_double_buffer_apply();
	count = 0x3fffff;
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	while((double_buffer_ctrl_reg.dreg_dbuf_set)&& --count){
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	}
	if(count==0){
		rtd_pr_vbe_notice("double buffer timeout !!!\n");
	}
	count = 0x3fffff;
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
	}else{
		dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
		dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
		IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
	}
	count = 0x3fffff;
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.dmainreg_dbuf_en = 0;
	double_buffer_ctrl_reg.dmainreg_dbuf_set = 0;
	double_buffer_ctrl_reg.dreg_dbuf_en = 0;
	double_buffer_ctrl_reg.dreg_dbuf_set = 0;
	double_buffer_ctrl_reg.drm_multireg_dbuf_en = 0;
	double_buffer_ctrl_reg.drm_multireg_dbuf_set = 0;
	double_buffer_ctrl_reg.drm_multireg_dbuf_vsync_sel = 0;
	double_buffer_ctrl_reg.dsubreg_dbuf_en = 0;
	double_buffer_ctrl_reg.dsubreg_dbuf_set = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	//ppoverlay_double_buffer_ctrl2_reg.osddtgreg_dbuf_en = 0;
	//ppoverlay_double_buffer_ctrl2_reg.osddtgreg_dbuf_set = 0;
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_set = 0;
	ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set = 0;
	//ppoverlay_double_buffer_ctrl2_reg.osddtg_dbuf_vsync_sel = 0;
	ppoverlay_double_buffer_ctrl2_reg.uzudtg_dbuf_vsync_sel = 0;
	ppoverlay_double_buffer_ctrl2_reg.memcdtg_dbuf_vsync_sel = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	modestate_set_fractional_framesyncByDisp(channel);
	modestate_set_display_timing_genByDisp(channel);//set dvtotal
	//local_irq_enable();
	//preempt_enable();
}

extern struct semaphore* get_double_buffer_semaphore(void);
extern struct semaphore* get_scaler_fll_running_semaphore(void);

void scaler_disp_smooth_variable_settingByDisp(unsigned char _channel)
{
	sys_reg_sys_dispclksel_RBUS sys_dispclksel_reg;
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
	UINT32 dclk_fract_a_new, dclk_fract_b_new, dclk_factor_new;
	UINT32 dclk_fract_a_current, dclk_fract_b_current, dclk_factor_current;


#if 0
	if((Scaler_DispGetInputInfoByDisp(_channel,SLR_INPUT_V_FREQ) > 475) &&
				(Scaler_DispGetInputInfoByDisp(_channel,SLR_INPUT_V_FREQ) < 485)){
		rtd_pr_vbe_notice("48Hz input is not support.\n");
		return;
	}
#endif

	if(vbe_disp_get_adaptivestream_fs_mode()){
		Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);
		return;
	}
	disp_smooth_variable_setting_flag = _ENABLE;

	modestate_decide_pixel_mode();
	modestate_decide_dtg_m_modeByDisp(_channel);
	modestate_decide_fractional_framesyncByDisp(_channel);

	dclk_fract_a_new = Scaler_DispGetInputInfoByDisp(0,SLR_INPUT_FRAC_FS_REMOVE_RATIO)-1;
	dclk_fract_b_new = Scaler_DispGetInputInfoByDisp(0,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO)-1;
	if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_2_PIXEL)
		dclk_fract_a_new = (Scaler_DispGetInputInfoByDisp(0,SLR_INPUT_FRAC_FS_REMOVE_RATIO)*2)-1;
	else
		dclk_fract_a_new = Scaler_DispGetInputInfoByDisp(0,SLR_INPUT_FRAC_FS_REMOVE_RATIO)-1;
	dclk_factor_new = ((dclk_fract_a_new+1)*1000 / (dclk_fract_b_new+1));

	sys_dispclksel_reg.regValue = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	dclk_fract_a_current = sys_dispclksel_reg.dclk_fract_a;
	dclk_fract_b_current = sys_dispclksel_reg.dclk_fract_b;
	dclk_factor_current = ((dclk_fract_a_current+1)*1000 / (dclk_fract_b_current+1));

	rtd_pr_vbe_debug("clk current (a:%d)(b:%d)(factor:%d)\n", dclk_fract_a_current, dclk_fract_b_current, dclk_factor_current);
	rtd_pr_vbe_debug("clk new (a:%d)(b:%d)(factor:%d)\n", dclk_fract_a_new, dclk_fract_b_new, dclk_factor_new);
#if 0
	//real cinema mode + true motion off
	if((Scaler_DispGetInputInfoByDisp(_channel,SLR_INPUT_V_FREQ) > 235) &&
			(Scaler_DispGetInputInfoByDisp(_channel,SLR_INPUT_V_FREQ) < 245)){
		scaler_disp_smooth_variable_setting_slow();
	}else{
		if(dclk_factor_current > dclk_factor_new) // change slow
			scaler_disp_smooth_variable_setting_slow();
		else
			scaler_disp_smooth_variable_setting_fast();
	}
#else

	if(0/*(dclk_factor_new/dclk_factor_current)>1*/){ // dclk change fast
		scaler_disp_smooth_variable_setting_fast();
	} else {
		if(Get_DISPLAY_REFRESH_RATE() < 120){
#ifndef BUILD_QUICK_SHOW
			if(get_vt_EnableFRCMode() == TRUE){	//not VR360 case
				if((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > 2560) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) > 1440) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 490)){
					Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 1);	//4k50/60Hz case, go MC on setting
				}
				else{
					Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);	//not 4k50/60Hz case, go MEMC on setting
				}
				memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
				if(memc_mux_ctrl_reg.memc_outmux_sel == 0){     //if memc mux off, need turn on
					rtd_pr_vbe_notice("[MEMC]memc mux off, need turn on!!\n");
					Scaler_MEMC_outMux(_ENABLE,_ENABLE);    //db on, mux on
				}
			}
			else
#endif                
                {   //VR360 case
				if(!Scaler_get_vdec_2k120hz()){
					if(((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > 2560) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) > 1440) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 490))){
						;										//4k50,60Hz case, do nothing
					}
					else if((((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > 2560) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) > 1440) &&
					(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) <= 490)))){
						Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 1);	//4k 24/25/30/48Hz case, go MC on setting
						memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
						if(memc_mux_ctrl_reg.memc_outmux_sel == 0){     //if memc mux off, need turn on
							rtd_pr_vbe_notice("[MEMC]memc mux off, need turn on!!\n");
							Scaler_MEMC_outMux(_ENABLE,_ENABLE);    //db on, mux on
						}
					}
					else{
						Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);	//not 4k case, go MEMC on setting
						memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
						if(memc_mux_ctrl_reg.memc_outmux_sel == 0){     //if memc mux off, need turn on
							rtd_pr_vbe_notice("[MEMC]memc mux off, need turn on!!\n");
							Scaler_MEMC_outMux(_ENABLE,_ENABLE);    //db on, mux on
						}
					}
				}
			}
		}
		else{
			//for 120Hz panel, input 100/120Hz bypass MEMC
			if(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 995){
				Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);
				memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
				if(memc_mux_ctrl_reg.memc_outmux_sel == 1){     //if memc mux on, need turn off
					rtd_pr_vbe_notice("[MEMC]memc mux off, need turn on!!\n");
					Scaler_MEMC_outMux(_ENABLE,_DISABLE);    //db on, mux off
				}
			}
			else{
				Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 1);
				memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
				if(memc_mux_ctrl_reg.memc_outmux_sel == 0){     //if memc mux off, need turn on
					rtd_pr_vbe_notice("[MEMC]memc mux off, need turn on!!\n");
					Scaler_MEMC_outMux(_ENABLE,_ENABLE);    //db on, mux on
				}
			}
		}
		down(get_scaler_fll_running_semaphore());
		down(get_double_buffer_semaphore());
		scaler_disp_smooth_variable_setting_slow();
		up(get_double_buffer_semaphore());
		up(get_scaler_fll_running_semaphore());
		if(Get_DISPLAY_REFRESH_RATE() < 120){
			if ((((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > 2560) &&
				(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE) > 1440) &&
				(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 490))|| Scaler_get_vdec_2k120hz()) && (get_vt_EnableFRCMode() == FALSE)){
				//VR360 4k 50/60Hz or 2k120Hz case, set memc mux off
				rtd_pr_vbe_notice("[VR360] 4k 50/60Hz VR360 case, bypass MEMC and set memc mux off\n");
				Scaler_MEMC_outMux(_ENABLE,_DISABLE);	//db on, mux off
				Scaler_MEMC_MEMC_CLK_OnOff(0, 0, 1);	//VR360, go MEMC bypass setting
			}
		}
	}
#endif
	Scaler_MEMC_SetInOutFrameRateByDisp(_channel);/*checked*/
	disp_smooth_variable_setting_flag = _DISABLE;
	//Eric@20181009 framerate change flow done, set dynamic_framerate_change_flag = 0
	Scaler_set_dynamic_framerate_change_flag(_DISABLE);
	if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)){
		modestate_set_fll_running_flag(_DISABLE);
		drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
	}
	else {
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_FRAMESYNC) == FALSE){
			modestate_set_fll_running_flag(_ENABLE);
			Scaler_Set_Display_Timing_Mode(DISPLAY_MODE_NEW_FLL_MODE);
		}
	}


#if 0
	SCALER_DISP_CHANNEL channel = _channel;
	unsigned char frame_idicator=0;
	unsigned char b_run_frame_check=0;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS dispd_stage1_sm_ctrl_reg;
	ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
	unsigned int count = 0x3fffff;
	unsigned char me_protection_flag = 0;
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS	 dispd_smooth_toggle1_reg;

	if(vbe_disp_get_adaptivestream_fs_mode() == 1){
		Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY);
		return;
	}
#if 0
	rtd_pr_vbe_debug("scaler_disp_smooth_variable_setting\n");
	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
	IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);

	if(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO) > 1){
		b_run_frame_check = 1;
		frame_idicator = Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_FRAC_FS_MULTIPLE_RATIO)-1;
	}

	modestate_decide_pixel_mode();
	modestate_decide_dtg_m_modeByDisp(_channel);
	modestate_decide_fractional_framesyncByDisp(_channel);
#ifdef SMOOTH_TOGGLE_DATAFRCFS_TEST
	if(vbe_disp_get_adaptivestream_fs_mode() == 0){
		drivf_scaler_reset_freerun();
	}
#else
	drivf_scaler_reset_freerun();
#endif
	modestate_disp_flow_double_buf_enable(_ENABLE);
	//modestate_set_display_timing_gen();
	modestate_set_display_timing_gen_clock_divByDisp(_channel);
	modestate_set_fractional_framesyncByDisp(_channel);

	if(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ) == 60)
		rtd_pr_vbe_debug("######## 48Hz to 60HZ #############\n");
	else if(Scaler_DispGetInputInfoByDisp(channel,SLR_INPUT_DTG_MASTER_V_FREQ) == 48)
		rtd_pr_vbe_debug("######## 60Hz to 48HZ #############\n");
	rtd_pr_vbe_debug("decide frame_idicator:%d\n", frame_idicator);

	//add ME protection when change frame rate @Crixus 20151221
	if((IoReg_Read32(KME_DM_TOP0_KME_04_AGENT_reg) & _BIT20) || (IoReg_Read32(KME_DM_TOP0_KME_05_AGENT_reg) & _BIT20)){
		me_protection_flag = 1;//if ME is disable, not to protect.
	}
	if(me_protection_flag == 1){
		//disable ME DMA
		IoReg_ClearBits(KME_DM_TOP0_KME_04_AGENT_reg, _BIT20);
		IoReg_ClearBits(KME_DM_TOP0_KME_05_AGENT_reg, _BIT20);
		IoReg_SetBits(KME_ME_TOP1__REG_METOP_SC_CLR_SAD_EN_ADDR, _BIT10);
		WaitFor_DEN_STOP();
		WaitFor_DEN_STOP();
		rtd_pr_vbe_debug("turn off me4.5 ok in smooth setting\n");
	}

	if(b_run_frame_check == 1){
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
		//rtd_pr_vbe_debug("++frame_idicator:%d\n", (IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)&0x3f000000)>>24);
	}else{
		ppoverlay_new_meas1_linecnt_real_RBUS new_meas1_linecnt_real_reg;
		new_meas1_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);
		while((new_meas1_linecnt_real_reg.memcdtg_line_cnt_rt > 200)&& --count){
			new_meas1_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);
		}
	}
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	modestate_set_fractional_framesyncByDisp(_channel);
	fw_scaler_dtg_double_buffer_apply();
	rtd_pr_vbe_debug("--- 1frame_idicator:%d\n",(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)&0x3f000000)>>24);

	count = 0x3fffff;
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	while((double_buffer_ctrl_reg.dreg_dbuf_set)&& --count){
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	}
	if(count==0){
		rtd_pr_vbe_notice("double buffer timeout !!!\n");
	}
	rtd_pr_vbe_debug("--- 2frame_idicator:%d\n",(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)&0x3f000000)>>24);

	count = 0x3fffff;
	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply)&& --count){
		dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	}
	if(count==0){
		rtd_pr_vbe_notice("dispd_smooth_toggle1 timeout !!!\n");
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
	modestate_disp_flow_double_buf_enable(_DISABLE);
	modestate_set_display_timing_genByDisp(_channel);//set dvtotal

	if(me_protection_flag == 1){
		IoReg_ClearBits(KME_ME_TOP1__REG_METOP_SC_CLR_SAD_EN_ADDR, _BIT10);
		IoReg_SetBits(KME_DM_TOP0_KME_04_AGENT_reg, _BIT20);
		IoReg_SetBits(KME_DM_TOP0_KME_05_AGENT_reg, _BIT20);
		WaitFor_DEN_STOP();
		me_protection_flag = 0;
		rtd_pr_vbe_debug("turn on me4.5 ok in smooth setting\n");
	}

	rtd_pr_vbe_debug("--- 3frame_idicator:%d\n",(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)&0x3f000000)>>24);
	Scaler_MEMC_SetInOutFrameRateByDisp(_channel);/*checked*/
#ifdef SMOOTH_TOGGLE_DATAFRCFS_TEST
	if(vbe_disp_get_adaptivestream_fs_mode() == 0){
		modestate_set_fll_running_flag(_ENABLE);
	}else{
		drvif_scaler_set_display_modeByDisp(DISPLAY_MODE_FRAME_SYNC,_channel);
	}

	if(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI){
		modestate_set_fll_running_flag(_DISABLE);
		drvif_scaler_set_display_mode(DISPLAY_MODE_FRC);
	}
#else
	modestate_set_fll_running_flag(_ENABLE);
#endif

#else
	UINT32 meas0_lineCnt1=0;
	UINT32 meas0_lineCnt2=0;
	UINT32 meas0_lineCnt3=0;
	UINT32 meas1_lineCnt1=0;
	UINT32 meas1_lineCnt2=0;
	UINT32 meas1_lineCnt3=0;
	UINT32 mul_dbuf_cnt=0;
	UINT32 mul_dbuf_cnt2=0;
	UINT32 db_val=0;
	ppoverlay_new_meas1_linecnt_real_RBUS ppoverlay_new_meas1_linecnt_real_reg;

	rtd_pr_vbe_emerg("scaler_disp_smooth_variable_setting\n");

	modestate_decide_pixel_mode();
	modestate_decide_dtg_m_modeByDisp(_channel);
	modestate_decide_fractional_framesyncByDisp(_channel);

	if(vbe_disp_decide_memc_mux_bypass() == FALSE)
		Scaler_MEMC_Bypass_On(0);

	//double buffer D0
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

	drivf_scaler_reset_freerun();

	//pre-flow. patch, let D8 double buffer works well
	count = 0x3fffff;
	while((count == 0x3fffff)&&(mul_dbuf_cnt<10000)){
		//msleep(10);
		double_buffer_ctrl_reg.drm_multireg_dbuf_set = 1;
		meas0_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
		meas1_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
		count = 0x3fffff;
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		while((double_buffer_ctrl_reg.drm_multireg_dbuf_set)&& --count){
			double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		}
		if(count==0){
			rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_set 1 wait timeout !!!\n");
		}
		meas0_lineCnt2 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
		meas1_lineCnt2 = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);
		mul_dbuf_cnt++;
	}
	modestate_set_fractional_framesyncByDisp(_channel);

	count = 0x3fffff;
	while((count == 0x3fffff)&&(mul_dbuf_cnt2<10000)){
		double_buffer_ctrl_reg.drm_multireg_dbuf_set = 1;
		meas0_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
		meas1_lineCnt1 = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
		count = 0x3fffff;
		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		while((double_buffer_ctrl_reg.drm_multireg_dbuf_set)&& --count){
			double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		}
		if(count==0){
			rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_set 2 wait timeout !!!\n");
		}
		meas0_lineCnt2 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
		meas1_lineCnt2 = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);
		mul_dbuf_cnt2++;
	}

	modestate_set_display_timing_gen_clock_divByDisp(_channel);
	count = 0x3fffff;

	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=0;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
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
	meas0_lineCnt3 = IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg);
	meas1_lineCnt3 = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);

	ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	ppoverlay_double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
 	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
	db_val = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	modestate_set_display_timing_genByDisp(_channel);//set dvtotal
	Scaler_MEMC_SetInOutFrameRateByDisp(_channel);/*checked*/
	modestate_set_fll_running_flag(_ENABLE);

	if(vbe_disp_decide_memc_mux_bypass() == TRUE)
		Scaler_MEMC_Bypass_On(1);


#if 0
	rtd_pr_vbe_emerg("meas0_lineCnt1 (%x)\n", meas0_lineCnt1);
	rtd_pr_vbe_emerg("meas1_lineCnt1 (%x)\n", meas1_lineCnt1);
	rtd_pr_vbe_emerg("meas0_lineCnt2 (%x)\n", meas0_lineCnt2);
	rtd_pr_vbe_emerg("meas1_lineCnt2 (%x)\n", meas1_lineCnt2);
	rtd_pr_vbe_emerg("meas0_lineCnt3 (%x)\n", meas0_lineCnt3);
	rtd_pr_vbe_emerg("meas1_lineCnt3 (%x)\n", meas1_lineCnt3);
	rtd_pr_vbe_emerg("mul_dbuf_cnt (%x)\n", mul_dbuf_cnt);
	rtd_pr_vbe_emerg("mul_dbuf_cnt2 (%x)\n", mul_dbuf_cnt2);
	rtd_pr_vbe_emerg("db_val (%x)\n", db_val);
#endif

#endif
#endif
}

extern platform_info_t platform_info;
unsigned char HAL_VBE_DISP_Initialize(KADP_DISP_PANEL_INFO_T panelInfo)
{
	unsigned char ret=1;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result =0;
#endif
	VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE *config_data;
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;
//	PANEL_CONFIG_PARAMETER parameter;
//	sfg_sfg_zz_shift_dbg_ctrl_RBUS sfg_zz_shift_dbg_ctrl_reg;
//	pif_lvds_ctrl3_RBUS  lvds_ctrl3;
//	pif_lvds_ctrl0_RBUS lvds_ctrl0;

	down(&VBE_Semaphore);
#if 0
 rtd_pr_vbe_emerg("####[KERN]HAL_VBE_DISP_Initialize###\n");
 rtd_pr_vbe_emerg("####[KERN]panelInfo.panel_inch = %x\n", panelInfo.panel_inch);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.panel_bl_type = %x\n", panelInfo.panel_bl_type);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.panel_maker = %x\n", panelInfo.panel_maker);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.led_bar_type = %x\n", panelInfo.led_bar_type);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.panelInterface = %x\n", panelInfo.panelInterface);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.panelResolution = %x\n", panelInfo.panelResolution);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.panel_version = %x\n", panelInfo.panel_version);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.frc_type = %x\n", panelInfo.frc_type);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.cell_type = %x\n", panelInfo.cell_type);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.disp_out_resolution = %x\n", panelInfo.disp_out_resolution);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.lvds_bit_mode = %x\n", panelInfo.lvds_bit_mode);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.lvds_type = %x\n", panelInfo.lvds_type);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.disp_out_lane_bw = %x\n", panelInfo.disp_out_lane_bw);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.panelFrameRate = %x\n", panelInfo.panelFrameRate);
 rtd_pr_vbe_emerg("####[KERN]panelInfo.user_specific_option = %x\n", panelInfo.user_specific_option);
#endif

	if(VBE_DISP_Global_Status != VBE_DISP_NOTHING){
		rtd_pr_vbe_debug("KERN_INFO \r\n VBE_DISP_Initialize has initialized.\n");
		up(&VBE_Semaphore);
		return ret;
	}
#if 0 // ben mark it@ restruct disp info not sync with webos
	memset(&parameter,0,sizeof(PANEL_CONFIG_PARAMETER));

	parameter.iCONFIG_DISP_ACT_STA_HPOS=0;
	parameter.iCONFIG_DISP_ACT_END_HPOS=panelInfo.disp_out_resolution.video_width;
	parameter.iCONFIG_DISP_ACT_STA_VPOS=0;
	parameter.iCONFIG_DISP_ACT_END_VPOS=panelInfo.disp_out_resolution.video_height;
	parameter.iCONFIG_DISPLAY_COLOR_BITS=(panelInfo.lvds_bit_mode)?0:1;  //panelInfo.lvds_bit_mode 0 is 8bit,1 is 10bit
	parameter.iCONFIG_DISPLAY_BITMAPPING_TABLE=(panelInfo.lvds_type)?0:1;  //panelInfo.lvds_type 0 is VESA,1 is JEIDA

	lvds_ctrl0.regValue = IoReg_Read32(PIF_LVDS_CTRL0_reg);
/*
	if(panelInfo.panelInterface==DISP_PANEL_HSLVDS)
	{
		parameter.iCONFIG_PANEL_TYPE=P_HSLVDS_TYPE;
		lvds_ctrl0.plt_lvds_mode=1;    // mLVDS-mode
	}
	else
*/
/*
	if(panelInfo.panelInterface==KADP_DISP_PANEL_EPI)
	{
		parameter.iCONFIG_PANEL_TYPE=P_EPI_TYPE;
		lvds_ctrl0.plt_lvds_mode=2;    //EPI-mode
	}
*/
/*
	else if(panelInfo.panelInterface==DISP_PANEL_Vx1)
	{
		parameter.iCONFIG_PANEL_TYPE=P_VBY1_4K2K;
		lvds_ctrl0.plt_lvds_mode=3;    //Vby1-mode
	}
*/
/*
	else
	{
		parameter.iCONFIG_PANEL_TYPE=P_LVDS_2K1K;
		lvds_ctrl0.plt_lvds_mode=0;    //LVDS-mode
	}
*/
	IoReg_Write32(PIF_LVDS_CTRL0_reg, lvds_ctrl0.regValue);

	Set_PANEL_BL_type(panelInfo.panel_bl_type);
	Set_PANEL_LED_Bar_type(panelInfo.led_bar_type);
	//Set_PANEL_EPI_TYPE(panelInfo.epi_panel_type);
	Set_PANEL_INCH(panelInfo.panel_inch);
	Set_PANEL_MAKER(panelInfo.panel_maker);
	Set_DISP_OSD_Width(panelInfo.disp_out_resolution.osd_width);
	Set_DISP_OSD_Width(panelInfo.disp_out_resolution.osd_height);

	//temp mark :  panelInfo.useInternalFrc;
	//sfg_zz_shift_dbg_ctrl_reg.regValue = IoReg_Read32(SFG_SFG_ZZ_shift_dbg_CTRL_reg);
	//sfg_zz_shift_dbg_ctrl_reg.dout_mode = parameter.iCONFIG_DISPLAY_COLOR_BITS;  //0 is 30 bit,1 is 24 bit
	//IoReg_Write32(SFG_SFG_ZZ_shift_dbg_CTRL_reg, sfg_zz_shift_dbg_ctrl_reg.regValue);

	lvds_ctrl3.regValue = IoReg_Read32(PIF_LVDS_CTRL3_reg);
	lvds_ctrl3.lvds_map = parameter.iCONFIG_DISPLAY_BITMAPPING_TABLE ? 1 : 0;  //0 is JEIDA,1is VESA
	IoReg_Write32(PIF_LVDS_CTRL3_reg, lvds_ctrl3.regValue);
#endif
	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE), GFP_DCU1_LIMIT, (void**)&vir_addr_noncache);
	//phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);
	config_data = (VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE *)vir_addr_noncache;
	config_data->videoPlane = VO_VIDEO_PLANE_V1;
	config_data->enable =Get_PANEL_VFLIP_ENABLE();
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_ConfigVodmaVFlip,phy_addr, 0, &result))
	{
		rtd_pr_vbe_debug("RPC fail!!\n");
		ret=0;
	}
#endif
	dvr_free((void *)vir_addr);
	drvif_scaler3d_set_panel_info(); //send panel info to video fw

	Scaler_Set_EnterFRC_LastLine_FixDclk_DVTotoal_Limit_Enable_Flag(_ENABLE);
	Scaler_Set_EnterFRC_LastLine_FixDclk_DVTotoal_Limit_Upper(Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MAX());
	Scaler_Set_EnterFRC_LastLine_FixDclk_DVTotoal_Limit_Lower(Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN());
	Scaler_Set_FixLastLine_DVTotoal_Limit_ByVfreq_Enable_Flag(_ENABLE);
	Scaler_Set_FixLastLine_DVTotal_Limit_60Hz_Typical(1125);
	Scaler_Set_FixLastLine_DVTotal_Limit_60Hz_Upper(Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MAX());
	Scaler_Set_FixLastLine_DVTotal_Limit_60Hz_Lower(Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN());
	Scaler_Set_FixLastLine_DVTotal_Limit_50Hz_Typical(1350);
	Scaler_Set_FixLastLine_DVTotal_Limit_50Hz_Upper(Get_PANEL_DISP_VERTICAL_TOTAL_50Hz_MAX());
	Scaler_Set_FixLastLine_DVTotal_Limit_50Hz_Lower(Get_PANEL_DISP_VERTICAL_TOTAL_50Hz_MIN());

#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
	Scaler_Set_FreeRun_To_FrameSync_By_HW_Enable(_ENABLE);
#else //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
	Scaler_Set_FreeRun_To_FrameSync_By_HW_Enable(_DISABLE);
#endif //#ifdef CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW
	Scaler_Set_FreeRun_To_FrameSync_By_HW_LineBufferMode_Enable(_ENABLE);
    Scaler_Set_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut(100);
	Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp(_ENABLE);
	Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_TimeOut(1000);
	Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_Porch(22);
	Scaler_Set_FreeRun_To_FrameSync_By_Reset_VODMA(TRUE);
/*
	if(Get_DISPLAY_PANEL_OLED_TYPE()== TRUE){
		Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2(_ENABLE);
		Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3(_DISABLE);
	}
	else{
		Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2(_DISABLE);
		Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3(_ENABLE);
	}
*/
	Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2(_DISABLE);
	Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3(_DISABLE);
	Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL4(_ENABLE);
	vbe_disp_dynamic_polarity_control_init();
	if((Get_DISPLAY_REFRESH_RATE() >= 120)&&(Get_DISPLAY_PANEL_TYPE()==P_EPI_TYPE)){
		vbe_disp_dynamic_polarity_control_set_detect_enable(1);
	}
	if(ret)
	{
		VBE_DISP_Global_Status =VBE_DISP_INIT_DONE;
	}

    Set_PANEL_MAKER(panelInfo.panel_maker);
	up(&VBE_Semaphore);
	return ret;
}
unsigned char HAL_VBE_DISP_Uninitialize(void)
{
	down(&VBE_Semaphore);
	if(VBE_DISP_Global_Status != VBE_DISP_INIT_DONE){
		rtd_pr_vbe_debug("\r\n HAL_VBE_DISP_Uninitialize fail. no initialize.\n");
		up(&VBE_Semaphore);
		return 0;
	}
	Panel_releaseParameter();
	//Scaler_ReleaseODMem();
	VBE_DISP_Global_Status =VBE_DISP_NOTHING;
	up(&VBE_Semaphore);
	return 1;
}

unsigned char HAL_VBE_DISP_Resume(KADP_DISP_PANEL_INFO_T panelInfo)
{
	unsigned char ret=1;
	//unsigned long result =0;
	//VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE *config_data;
	//unsigned int vir_addr, vir_addr_noncache;
	//unsigned int phy_addr;
	//PANEL_CONFIG_PARAMETER parameter;
	//sfg_zz_shift_dbg_ctrl_RBUS sfg_zz_shift_dbg_ctrl_reg;
	//lvds_ctrl3_RBUS  lvds_ctrl3;
	//lvds_ctrl0_RBUS lvds_ctrl0;


	down(&VBE_Semaphore);
/*
	if(VBE_DISP_Global_Status != VBE_DISP_INIT_DONE){
		rtd_pr_vbe_info("HAL_VBE_DISP_Resume no initialize.\n");
		up(&VBE_Semaphore);
		return 0;
	}
*/
	rtd_pr_vbe_emerg("[HAL_VBE_DISP_Resume] HAL_VBE_DISP_Resume !\n");
	vbe_disp_resume();

#ifdef VBE_DISP_TCON_SELF_CHECK
	vbe_disp_tcon_lock_self_check();
#if IS_ENABLED(CONFIG_RTK_KDRV_THERMAL_SENSOR)
	monitor_rgb_rolling=0; //for run tcon check
#endif
#endif

#if 0
	memset(&parameter,0,sizeof(PANEL_CONFIG_PARAMETER));

	parameter.iCONFIG_DISP_ACT_STA_HPOS=0;
	parameter.iCONFIG_DISP_ACT_END_HPOS=panelInfo.disp_out_resolution.video_width;
	parameter.iCONFIG_DISP_ACT_STA_VPOS=0;
	parameter.iCONFIG_DISP_ACT_END_VPOS=panelInfo.disp_out_resolution.video_height;
	parameter.iCONFIG_DISPLAY_COLOR_BITS=(panelInfo.lvds_bit_mode)?0:1;  //panelInfo.lvds_bit_mode 0 is 8bit,1 is 10bit
	parameter.iCONFIG_DISPLAY_BITMAPPING_TABLE=(panelInfo.lvds_type)?0:1;  //panelInfo.lvds_type 0 is VESA,1 is JEIDA
	lvds_ctrl0.regValue = IoReg_Read32(PIF_LVDS_CTRL0_reg);
	if(panelInfo.panelInterface==DISP_PANEL_HSLVDS)
	{
		parameter.iCONFIG_PANEL_TYPE=P_HSLVDS_TYPE;
		lvds_ctrl0.plt_lvds_mode=1;    // mLVDS-mode
	}
	else if(panelInfo.panelInterface==DISP_PANEL_EPI)
	{
		parameter.iCONFIG_PANEL_TYPE=P_EPI_TYPE;
		lvds_ctrl0.plt_lvds_mode=2;    //EPI-mode
	}
/*
	else if(panelInfo.panelInterface==DISP_PANEL_Vx1)
	{
		parameter.iCONFIG_PANEL_TYPE=P_VBY1_4K2K;
		lvds_ctrl0.plt_lvds_mode=3;    //Vby1-mode
	}
*/
	else
	{
		parameter.iCONFIG_PANEL_TYPE=P_LVDS_2K1K;
		lvds_ctrl0.plt_lvds_mode=0;    //LVDS-mode
	}
	IoReg_Write32(PIF_LVDS_CTRL0_reg, lvds_ctrl0.regValue);

	Set_PANEL_BL_type(panelInfo.panel_bl_type);
	Set_PANEL_LED_Bar_type(panelInfo.led_bar_type);
	Set_PANEL_EPI_TYPE(panelInfo.epi_panel_type);
	Set_PANEL_INCH(panelInfo.panel_inch);
	Set_PANEL_MAKER(panelInfo.panel_maker);
	Set_DISP_OSD_Width(panelInfo.disp_out_resolution.osd_width);
	Set_DISP_OSD_Width(panelInfo.disp_out_resolution.osd_height);

	//temp mark :  panelInfo.useInternalFrc;
	sfg_zz_shift_dbg_ctrl_reg.regValue = IoReg_Read32(SFG_SFG_ZZ_shift_dbg_CTRL_reg);
	sfg_zz_shift_dbg_ctrl_reg.dout_mode = parameter.iCONFIG_DISPLAY_COLOR_BITS;  //0 is 30 bit,1 is 24 bit
	IoReg_Write32(SFG_SFG_ZZ_shift_dbg_CTRL_reg, sfg_zz_shift_dbg_ctrl_reg.regValue);

	lvds_ctrl3.regValue = IoReg_Read32(PIF_LVDS_CTRL3_reg);
	lvds_ctrl3.lvds_map = parameter.iCONFIG_DISPLAY_BITMAPPING_TABLE ? 1 : 0;  //0 is JEIDA,1is VESA
	IoReg_Write32(PIF_LVDS_CTRL3_reg, lvds_ctrl3.regValue);

	vir_addr = (unsigned int)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE), GFP_DCU1_LIMIT, (void**)&vir_addr_noncache);
	//phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);
	config_data = (VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE *)vir_addr_noncache;
	config_data->videoPlane = VO_VIDEO_PLANE_V1;
	config_data->enable =Get_PANEL_VFLIP_ENABLE();
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_ConfigVodmaVFlip,phy_addr, 0, &result))
	{
		rtd_pr_vbe_info("RPC fail!!\n");
		ret=0;
	}
#endif
	dvr_free((void *)vir_addr);
	drvif_scaler3d_set_panel_info(); //send panel info to video fw
	if(ret)
	{
		VBE_DISP_Global_Status =VBE_DISP_INIT_DONE;
	}
#endif
    Set_PANEL_MAKER(panelInfo.panel_maker);

	up(&VBE_Semaphore);
	return ret;
}

void HAL_VBE_DISP_Connect(KADP_DISP_WINDOW_T wId)
{
	//do nothing:vsc would do
}
void HAL_VBE_DISP_Disconnect(KADP_DISP_WINDOW_T wId)
{
	//do nothing:vsc would do
}

//extern unsigned char HAL_VT_Finalize(void);
extern void vt_enable_dc2h(unsigned char state);
extern unsigned char get_vt_function(void);
extern BOOL Scaler_MEMC_LibSetMEMCFrameRepeatEnable(BOOL bEnable, BOOL bForce);
void HAL_VBE_DISP_SetDisplayOutput(BOOLEAN bOnOff)
{

	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	//main_display_control_rsv_RBUS main_display_control_rsv_reg;
	ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
	unsigned long flags;//for spin_lock_irqsave
	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	//main_display_control_rsv_reg.regValue = IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
	ppoverlay_double_buffer_ctrl_reg.regValue=IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);

	if(bOnOff)
	{
		rtd_pr_vbe_emerg("[HAL_VBE_DISP_SetDisplayOutput][%d]\n", bOnOff);

		display_timing_ctrl1_reg.disp_en=1;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg,display_timing_ctrl1_reg.regValue);
#if IS_ENABLED(CONFIG_RTK_KDRV_GDMA)
        //call GAL osd resume
		GAL_Runtime_Resume_light();
#endif
		//KTASKWBS-8895 : BOE panel power on sequence
		if((Get_DISPLAY_PANEL_TYPE() == P_CEDS)||(Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE)){
			msleep(25);
			rtd_pr_vbe_emerg("HAL_VBE_DISP_SetDisplayOutput. TCON++ (%x)\n", IoReg_Read32(TCON_TCON_IP_EN_reg));
			IoReg_SetBits(TCON_TCON_IP_EN_reg, _BIT0);
			rtd_pr_vbe_emerg("HAL_VBE_DISP_SetDisplayOutput. TCON-- (%x)\n", IoReg_Read32(TCON_TCON_IP_EN_reg));
		}

		//KTASKWBS-8895/KTASKWBS-9202 : lane power control by this api
		if((Get_DISPLAY_PANEL_TYPE() == P_CEDS)||(Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE)||(Get_DISPLAY_PANEL_TYPE() == P_VBY1_4K2K)){
			vbe_disp_lane_power_on();
		}
		msleep(25);
#ifndef CONFIG_MEMC_NOTSUPPORT
		if(get_MEMC_bypass_status_refer_platform_model() == FALSE){
			Mid_Mode_EnableMEDMA();
			Mid_Mode_EnableMCDMA();
			Scaler_MEMC_LibSetMEMCFrameRepeatEnable(0, 1);
			msleep(25);
			rtd_pr_vbe_notice("[HAL_VBE_DISP_SetDisplayOutput][%d][,%x,%x,%x,%x,%x,][,%x,%x,%x,%x,%x,][,%x,%x,%x,%x,%x,%x]\n\r", __LINE__,
				rtd_inl(KME_DM_TOP0_KME_00_AGENT_reg), rtd_inl(KME_DM_TOP0_KME_01_AGENT_reg), rtd_inl(KME_DM_TOP0_KME_02_AGENT_reg), rtd_inl(KME_DM_TOP0_KME_03_AGENT_reg), rtd_inl(KME_DM_TOP0_KME_04_AGENT_reg),
				rtd_inl(KME_DM_TOP0_KME_05_AGENT_reg), rtd_inl(KME_DM_TOP0_KME06AGENT_reg), rtd_inl(KME_DM_TOP0_KME_07_AGENT_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_14_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_1C_reg),
				rtd_inl(KME_DM_TOP1_KME_DM_TOP1_38_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_40_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_5C_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_64_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_80_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_88_reg));
			rtd_pr_vbe_notice("[HAL_VBE_DISP_SetDisplayOutput][%d][,%x,%x,%x,%x,%x,%x,][,%x,%x,%x,%x,%x,%x]\n\r", __LINE__,
				rtd_inl(KME_DM_TOP2_MV01_AGENT_reg), rtd_inl(KME_DM_TOP2_MV02_AGENT_reg), rtd_inl(KME_DM_TOP2_MV03_AGENT_reg), rtd_inl(KME_DM_TOP2_MV04_AGENT_reg), rtd_inl(KME_DM_TOP2_MV05_AGENT_reg), rtd_inl(KME_DM_TOP2_MV06_AGENT_reg),
				rtd_inl(KME_DM_TOP2_MV07_AGENT_reg), rtd_inl(KME_DM_TOP2_MV08_AGENT_reg), rtd_inl(KME_DM_TOP2_MV09_AGENT_reg), rtd_inl(KME_DM_TOP2_MV10_AGENT_reg), rtd_inl(KME_DM_TOP2_MV11_AGENT_reg), rtd_inl(KME_DM_TOP2_MV12_AGENT_reg));
		}
#endif
	}
	else
	{
		Scaler_MEMC_Set_PowerSaving_Status(0);
#if 0	//MEMC fbg no need to control here. //to sync tv006 patch
		//when disable d-domain clock, first enable MEMC output bg in factory mode @Crixus 20151231
		Scaler_MEMC_output_force_bg_enable(_ENABLE,__func__,__LINE__);
#endif
		rtd_pr_vbe_debug("Enable MEMC BG in SetDisplayOutput!!!\n");
		rtd_pr_vbe_emerg("[HAL_VBE_DISP_SetDisplayOutput][%d]\n", bOnOff);

#ifndef CONFIG_MEMC_NOTSUPPORT		
		if(get_MEMC_bypass_status_refer_platform_model() == FALSE){
			Mid_Mode_DisableMEDMA(0);
			Mid_Mode_DisableMCDMA();
			msleep(25);
			rtd_pr_vbe_notice("[HAL_VBE_DISP_SetDisplayOutput][%d][,%x,%x,%x,%x,%x,][,%x,%x,%x,%x,%x,][,%x,%x,%x,%x,%x,%x]\n\r", __LINE__,
				rtd_inl(KME_DM_TOP0_KME_00_AGENT_reg), rtd_inl(KME_DM_TOP0_KME_01_AGENT_reg), rtd_inl(KME_DM_TOP0_KME_02_AGENT_reg), rtd_inl(KME_DM_TOP0_KME_03_AGENT_reg), rtd_inl(KME_DM_TOP0_KME_04_AGENT_reg),
				rtd_inl(KME_DM_TOP0_KME_05_AGENT_reg), rtd_inl(KME_DM_TOP0_KME06AGENT_reg), rtd_inl(KME_DM_TOP0_KME_07_AGENT_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_14_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_1C_reg),
				rtd_inl(KME_DM_TOP1_KME_DM_TOP1_38_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_40_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_5C_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_64_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_80_reg), rtd_inl(KME_DM_TOP1_KME_DM_TOP1_88_reg));
			rtd_pr_vbe_notice("[HAL_VBE_DISP_SetDisplayOutput][%d][,%x,%x,%x,%x,%x,%x,][,%x,%x,%x,%x,%x,%x]\n\r", __LINE__,
				rtd_inl(KME_DM_TOP2_MV01_AGENT_reg), rtd_inl(KME_DM_TOP2_MV02_AGENT_reg), rtd_inl(KME_DM_TOP2_MV03_AGENT_reg), rtd_inl(KME_DM_TOP2_MV04_AGENT_reg), rtd_inl(KME_DM_TOP2_MV05_AGENT_reg), rtd_inl(KME_DM_TOP2_MV06_AGENT_reg),
				rtd_inl(KME_DM_TOP2_MV07_AGENT_reg), rtd_inl(KME_DM_TOP2_MV08_AGENT_reg), rtd_inl(KME_DM_TOP2_MV09_AGENT_reg), rtd_inl(KME_DM_TOP2_MV10_AGENT_reg), rtd_inl(KME_DM_TOP2_MV11_AGENT_reg), rtd_inl(KME_DM_TOP2_MV12_AGENT_reg));
		}
#endif
		//KTASKWBS-8895 : BOE panel power on sequence
		if((Get_DISPLAY_PANEL_TYPE() == P_CEDS)||(Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE)){
			rtd_pr_vbe_emerg("HAL_VBE_DISP_SetDisplayOutput. TCON++ (%x)\n", IoReg_Read32(TCON_TCON_IP_EN_reg));
			IoReg_ClearBits(TCON_TCON_IP_EN_reg, _BIT0);
			msleep(25);
			rtd_pr_vbe_emerg("HAL_VBE_DISP_SetDisplayOutput. TCON-- (%x)\n", IoReg_Read32(TCON_TCON_IP_EN_reg));
		}

		//KTASKWBS-8895/KTASKWBS-9202 : lane power control by this api
		if((Get_DISPLAY_PANEL_TYPE() == P_CEDS)||(Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE)||(Get_DISPLAY_PANEL_TYPE() == P_VBY1_4K2K)){
		vbe_disp_lane_power_off();
		msleep(25);
		}

		/*need checking dc2h capture status before close dclock, if is capturing do close, or do nothing*/
		if(get_vt_function() == TRUE) /* if VT function already work,stop dc2h when no clock */
			vt_enable_dc2h(FALSE); /*WOSQRTK-13830*/

#if IS_ENABLED(CONFIG_RTK_KDRV_GDMA)
		//call GAL osd suspend
		GAL_Runtime_Suspend_light();
#endif

		display_timing_ctrl1_reg.disp_en=0;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg,display_timing_ctrl1_reg.regValue);
	}
	//IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg,main_display_control_rsv_reg.regValue);
	if(ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en)
	{
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set=1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg,ppoverlay_double_buffer_ctrl_reg.regValue);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	}

#if 0	//MEMC disable fbg will control in scaler flow, no need to control here. //to sync tv006
	if(bOnOff)
	{
		//wait enable d-domain clock, then enable MEMC output bg in factory mode @Crixus 20151231
		//Because intanboot resume will call this function, if instanboot resume, it should be wait resume done.
		if(Scaler_MEMC_Get_instanboot_resume_Done() == 0){
			Scaler_MEMC_output_force_bg_enable(_DISABLE,__func__,__LINE__);
			rtd_pr_vbe_debug("Disable MEMC BG in SetDisplayOutput!!!\n");
		}
	}
#endif
}
void vbe_disp_suspend(void);
void vbe_disp_resume(void);

void HAL_VBE_DISP_SetMute(BOOLEAN bOnOff)
{
	sfg_sfg_force_bg_at_dif_RBUS sfg_force_bg_at_dif_reg;

	/*WaitFor_DEN_STOP();*/
	sfg_force_bg_at_dif_reg.regValue = IoReg_Read32(SFG_SFG_FORCE_BG_AT_DIF_reg);
	if(bOnOff){
		sfg_force_bg_at_dif_reg.en_force_bg_dif = 1;  //osd and video all mute
		if(Get_DISPLAY_PANEL_OLED_TYPE() == TRUE){
			oled_resume_eit_flag = TRUE;
		}
	}else{
		sfg_force_bg_at_dif_reg.en_force_bg_dif = 0;
		if(Get_DISPLAY_PANEL_OLED_TYPE() == TRUE){
			oled_resume_eit_flag = FALSE;
		}
	}
	IoReg_Write32(SFG_SFG_FORCE_BG_AT_DIF_reg, sfg_force_bg_at_dif_reg.regValue);
}

//[RTKREQ-819] u16Percent=0.1% , u16Period = 0.1k
//- If percent is 11, driver set SSC percent set 1.1 %.
//- If Period is 253, driver set SSC percent set 25.3 KHz.
void HAL_VBE_DISP_SetSpreadSpectrum(BOOLEAN bEnable, UINT16 u16Percent, UINT16 u16Period)
{
	rtd_pr_vbe_notice("[SSCG] bEnable=%d, u16Percent=%d, u16Period=%d\n", bEnable, u16Percent, u16Period);

	if((u16Period>=100)&&(u16Period<=1000))
		u16Period = u16Period*100;

	if(u16Percent % 10 == 0)
		u16Percent = u16Percent * 10;
	else if(u16Percent % 100 == 0)
		u16Percent = u16Percent * 1;
	else
		u16Percent = u16Percent * 100;

	if((Get_DISPLAY_PANEL_TYPE() == P_EDP_4K2K) ||
		(Get_DISPLAY_PANEL_TYPE() == P_EDP_4K2K_2PORT) ||
		(Get_DISPLAY_PANEL_TYPE() == P_EDP_3K1K) ||
		(Get_DISPLAY_PANEL_TYPE() == P_EDP_4K1K)){
          	//****Percent unit is x0.01 and divide 2, For example: if need sscg +/-3%, must set u16Percent=600****//
                //****Period unit is x0.001, For example: if modulation frequncy is 30khz, must set u16Period=30000****//
                unsigned int dclk_sscg_range = 0;
                unsigned int txpll_prediv_bypass=1, txpll_ckin_div_n=0, txpll_prescaler_div2_edp=1;
                unsigned int f_code, n_code, n_code_ssc, f_code_ssc;
                unsigned int dot_gran_edptx=4, gran_est;
                unsigned int lane_data_rate=5400;
                unsigned int ssc_clk, step;
                unsigned int ss_freq = u16Period;
                unsigned int temp=0, ncode_tmp=0, ncode_ssc_tmp=0, gran_est_tmp=0;

                if(bEnable == 0 || u16Percent == 0){
                        //disable edptx sscg
                        rtd_pr_vbe_debug("\n### Disable edptx sscg ###\n");
                        rtd_part_outl(0xb8000EAC, 0, 0, 0);
                        return;
                }

                dclk_sscg_range = u16Percent;

                //rtd_pr_vbe_debug("\n dclk_sscg_range = %d, sscg_frequency = %d \n", dclk_sscg_range, u16Period);

                temp = lane_data_rate*(10000+dclk_sscg_range) / 27;
                ncode_tmp = temp * (txpll_prediv_bypass+(1-txpll_prediv_bypass)*(txpll_ckin_div_n+2)) / (1<<txpll_prescaler_div2_edp);

                n_code = (ncode_tmp/10000)-3;
                f_code = ((ncode_tmp % 10000)*2048)/10000;

                ncode_ssc_tmp = lane_data_rate*(10000-dclk_sscg_range) / 27 * (txpll_prediv_bypass+(1-txpll_prediv_bypass)*(txpll_ckin_div_n+2)) / (1<<txpll_prescaler_div2_edp);
                n_code_ssc = (ncode_ssc_tmp/10000)-3;
                f_code_ssc = ((ncode_ssc_tmp % 10000)*2048)/10000;

                //Step1: 0x18000EA0[29]=0, oc_en disable
                rtd_part_outl(0xb8000EA0, 29, 29, 0);

                //Step2: apply SSC parameters
                rtd_part_outl(0xb8000EA0, 19, 12, n_code); //set ncode_t_edptx
                rtd_part_outl(0xb8000EA0, 10, 0, f_code); //set fcode_t_edptx
                rtd_part_outl(0xb8000EA8, 23, 16, n_code_ssc); //set ncode_ssc_edptx
                rtd_part_outl(0xb8000EA8, 10, 0, f_code_ssc); //set fcode_ssc_edptx

                //rtd_pr_vbe_debug("\n temp:%d, ncode_tmp:%d, ncode_ssc_tmp:%d\n", temp, ncode_tmp, ncode_ssc_tmp);
                rtd_pr_vbe_debug("\n n_code:%d, f_code:%d, n_code_ssc:%d, f_code_ssc:%d\n", n_code, f_code, n_code_ssc, f_code_ssc);

                temp = (10000*((n_code+n_code_ssc)*2048+f_code+f_code_ssc)/2/2048 + 3*10000);
                if(temp != 0)
                        ssc_clk = (lane_data_rate*10000) / (1<<txpll_prescaler_div2_edp) / temp;
                else {
                        if(temp == 0)
                                rtd_pr_vbe_err("\n [Error]temp should not be zero!!!! \n");
                        return;
                }

                if(ss_freq != 0)
                        step =	(ssc_clk*1000) / (ss_freq/1000);
                else {
                        rtd_pr_vbe_err("\n [Error]ss_freq should not be zero!!!! \n");
                        return;
                }
                rtd_pr_vbe_debug("\n temp:%d, ssc_clk:%d \n", temp, ssc_clk);

                gran_est_tmp = (((n_code-n_code_ssc)*2048+f_code-f_code_ssc)*2*(1<<(15-dot_gran_edptx))) /step;
                if(gran_est_tmp < (1 << 19))
                        gran_est = gran_est_tmp;
                else
                        gran_est = (((n_code-1-n_code_ssc)*2048+ (2048+f_code)-f_code_ssc)*2*(1<<(15-dot_gran_edptx))) /step;

                rtd_part_outl(0xb8000EAC, 22, 4, gran_est); //set reg_gran_est_edptx

                rtd_pr_vbe_debug( "\n step:%d, gran_est:%d\n", step , gran_est);

                rtd_part_outl(0xb8000C78, 5, 5, txpll_prediv_bypass); //set txpll_prediv_bypass
                rtd_part_outl(0xb8000C78, 13, 8, txpll_ckin_div_n); //set txpll_ckin_div_n
                rtd_part_outl(0xb8000C10, 4, 4, txpll_prescaler_div2_edp); //set txpll_prescaler_div2_edp
                rtd_part_outl(0xb8000EAC, 26, 24, dot_gran_edptx); //set reg_dot_gran_edptx

                //Step3: 0x18000EA0[29]=1, oc_en enable
                rtd_part_outl(0xb8000EA0, 29, 29, 1);

                //Step4: wait at least 10us or read oc_done_edptx=1 (0x18000EC0[29])
                mdelay(1);

                //Step5: 0x18000EA0[29]=0, oc_en disable
                rtd_part_outl(0xb8000EA0, 29, 29, 0);

                //Step6. enable en_ssc_edptx (0x8000EAC[0]=1) 
                rtd_part_outl(0xb8000EAC, 0, 0, 1);
        }
	else{
        	//****Percent unit is x0.01 and divide 2, For example: if need sscg +/-3%, must set u16Percent=600****//
                //****Period unit is x0.001, For example: if modulation frequncy is 30khz, must set u16Period=30000****//
                UINT16 dclk_sscg_range = 0;
                pll27x_reg_sys_dclkss_RBUS dclkss_reg;
                pll27x_reg_sys_pll_disp1_RBUS pll_disp1_reg;
          pll27x_reg_sys_pll_disp2_RBUS pll_disp2_reg;
                pll27x_reg_pll_ssc0_RBUS pll_ssc0_reg;
                pll27x_reg_pll_ssc3_RBUS pll_ssc3_reg;
                pll27x_reg_pll_ssc4_RBUS pll_ssc4_reg;
                pll27x_reg_pll_ssc5_RBUS pll_ssc5_reg;
                pll27x_reg_sys_pll_disp3_RBUS pll27x_reg_sys_pll_disp3_reg;
                UINT32 m_code, f_code, n_code, n_code_ssc, f_code_ssc;
                UINT32 temp, temp_ssc;
                UINT32 dot_gran=4, gran_est;
                UINT32 gran_est_temp, dpll_clk_ssc, ssc_clk, step;
                UINT32 cnt=0;
                UINT32 dclk_Temp, bpsin;
                UINT32 ulFreq=0, target_freq=0;
                UINT32 div=0;
          UINT32 ss_freq = u16Period;

                dclk_sscg_range = u16Percent;

                rtd_pr_vbe_debug("\n dclk_sscg_range = %d \n", dclk_sscg_range);

                ulFreq = Get_DISPLAY_CLOCK_TYPICAL();
                pll_disp2_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_PLL_DISP2_reg);

                if((ulFreq>=400*1000000) && (ulFreq <= 700*1000000)){
                        div = 1;
                }else if((ulFreq>=200*1000000) && (ulFreq <= 350*1000000)){
                        div = 2;
                        ulFreq = ulFreq*2;
                }else if((ulFreq>=100*1000000) && (ulFreq <= 175*1000000)){
                        div = 4;
                        ulFreq = ulFreq*4;
                }else if((ulFreq>=60*1000000) && (ulFreq <= 80*1000000)){
                        div = 8;
                        ulFreq = ulFreq*8;
                }else{
                        rtd_pr_vbe_err("\n Error Dclk Range!\n");
                }
                n_code = ((pll_disp2_reg.dpll_x_bpsin == 1) ? 0 : 1);
                bpsin = pll_disp2_reg.dpll_x_bpsin;
                dclk_Temp = (ulFreq*((pll_disp2_reg.dpll_x_bpsin == 1) ? (n_code + 1) : (n_code + 2))) / 2700;
                m_code = ((dclk_Temp/10000)-3);
                f_code = ((dclk_Temp % 10000)*2048) / 10000;	// offset
                pll_disp1_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_PLL_DISP1_reg);
                pll_ssc0_reg.regValue = IoReg_Read32(PLL27X_REG_PLL_SSC0_reg);
                pll_ssc3_reg.regValue = IoReg_Read32(PLL27X_REG_PLL_SSC3_reg);
                pll_ssc4_reg.regValue = IoReg_Read32(PLL27X_REG_PLL_SSC4_reg);
                rtd_pr_vbe_debug("\n n_code:%d, m_code:%d, f_code:%d \n", n_code, m_code, f_code);

                if((bEnable == 0) || (u16Percent ==0) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI) || (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_eDP)){
                        //set default dclk freq.
                        pll_disp1_reg.dpll_m = m_code;
                        pll_disp1_reg.dpll_n = n_code;
                        pll_ssc0_reg.fcode_t_ssc = f_code;
                        pll_ssc0_reg.oc_en = 0;
                        pll_ssc0_reg.oc_step_set = ((pll_disp2_reg.dpll_x_bpsin == 1) ? 0x10 : 0x200);;
                        IoReg_Write32(PLL27X_REG_SYS_PLL_DISP1_reg, pll_disp1_reg.regValue);
                        IoReg_Write32(PLL27X_REG_PLL_SSC0_reg, pll_ssc0_reg.regValue);
                        pll_ssc0_reg.oc_en = 1;
                        IoReg_Write32(PLL27X_REG_PLL_SSC0_reg, pll_ssc0_reg.regValue);
                        cnt=10;
                        do{
                                pll_ssc5_reg.regValue = IoReg_Read32(PLL27X_REG_PLL_SSC5_reg);
                                mdelay(1);
                        }while((pll_ssc5_reg.oc_done != 1) && (cnt--));
                        pll_ssc0_reg.oc_en = 0;
                        IoReg_Write32(PLL27X_REG_PLL_SSC0_reg, pll_ssc0_reg.regValue);
                        //disable sscg
                        pll_ssc4_reg.en_ssc = 0;
                        IoReg_Write32(PLL27X_REG_PLL_SSC4_reg, pll_ssc4_reg.regValue);

                        dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
                        dclkss_reg.dclk_ss_load = 0;
                        dclkss_reg.dclk_ss_en = 0;
                        IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, dclkss_reg.regValue);

                        return;
                }

                target_freq = ulFreq / 1000000 * (10000+u16Percent/2);
                temp = target_freq/27*(bpsin + (1-bpsin)*(n_code+2));
                m_code = ((temp/10000)-3);
                f_code = ((temp%10000)*2048)/10000;
                temp_ssc = ((target_freq/10000)*(10000-dclk_sscg_range)/27*(bpsin + (1-bpsin)*(n_code+2))) + ((target_freq%10000)*(10000-dclk_sscg_range)/27*(bpsin + (1-bpsin)*(n_code+2))/10000);
                n_code_ssc = ((temp_ssc/10000)-3);
                f_code_ssc = ((temp_ssc % 10000)*2048) / 10000;

                //rtd_pr_vbe_debug("\n m_code:%d, f_code:%d, n_code_ssc:%d, f_code_ssc:%d \n", m_code, f_code, n_code_ssc, f_code_ssc);

                pll_disp1_reg.dpll_m = m_code;
                pll_disp1_reg.dpll_n = n_code;
                pll_ssc0_reg.fcode_t_ssc = f_code;

                pll_ssc3_reg.ncode_ssc = n_code_ssc;
                pll_ssc3_reg.fcode_ssc = f_code_ssc;
                pll_ssc0_reg.oc_en = 0;
                pll_ssc0_reg.oc_step_set = ((pll_disp2_reg.dpll_x_bpsin == 1) ? 0x10 : 0x200);;
                IoReg_Write32(PLL27X_REG_SYS_PLL_DISP1_reg, pll_disp1_reg.regValue);
                IoReg_Write32(PLL27X_REG_PLL_SSC0_reg, pll_ssc0_reg.regValue);
                IoReg_Write32(PLL27X_REG_PLL_SSC3_reg, pll_ssc3_reg.regValue);

                pll27x_reg_sys_pll_disp3_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_PLL_DISP3_reg);
                if(div == 1)
                        pll27x_reg_sys_pll_disp3_reg.dpll_o = 0;
                else if(div == 2)
                        pll27x_reg_sys_pll_disp3_reg.dpll_o = 1;
                else if(div == 4)
                        pll27x_reg_sys_pll_disp3_reg.dpll_o = 2;
                else if(div == 8)
                        pll27x_reg_sys_pll_disp3_reg.dpll_o = 3;
                else
                        pll27x_reg_sys_pll_disp3_reg.dpll_o = 0;
                IoReg_Write32(PLL27X_REG_SYS_PLL_DISP3_reg, pll27x_reg_sys_pll_disp3_reg.regValue);

                temp= 0;
                dpll_clk_ssc = ((target_freq/10000)*(10000-dclk_sscg_range/2)) + ((target_freq%10000)*(10000-dclk_sscg_range/2))/10000;
                temp = (10000*((m_code-n_code_ssc)*2048+f_code-f_code_ssc)/2/2048 + (n_code_ssc+3)*10000);
                if(temp != 0)
                        ssc_clk = dpll_clk_ssc/temp;
                else
                {
                        if(temp == 0)
                                rtd_pr_vbe_err("\n [Error]temp should not be zero!!!! \n");
                        return;
                }
                //rtd_pr_vbe_debug("\n dpll_clk_ssc:%d, temp:%d, ssc_clk:%d \n", dpll_clk_ssc, temp, ssc_clk);

                if(ss_freq != 0)
                        step =	(ssc_clk*1000) / (ss_freq/1000);
                else {
                        rtd_pr_vbe_err("\n [Error]ss_freq should not be zero!!!! \n");
                        return;
                }

                gran_est_temp = (((m_code-n_code_ssc)*2048+(f_code-f_code_ssc))*2*(1<<(15-dot_gran))) /step;
                if(gran_est_temp < (1<<19))
                        gran_est = gran_est_temp;
                else
                        gran_est = (((m_code-1-n_code_ssc)*2048+ ((2048+f_code)-f_code_ssc))*2*(1<<(15-dot_gran))) /step;

                //rtd_pr_vbe_debug( "\n step:%d, gran_est:%d\n", step , gran_est);

                pll_ssc4_reg.dot_gran = dot_gran;
                pll_ssc4_reg.gran_auto_rst = 1;
                pll_ssc4_reg.gran_est = gran_est;
                IoReg_Write32(PLL27X_REG_PLL_SSC4_reg, pll_ssc4_reg.regValue);

                pll_ssc0_reg.oc_en = 1;
                IoReg_Write32(PLL27X_REG_PLL_SSC0_reg, pll_ssc0_reg.regValue);

                cnt=100;
                do{
                        mdelay(1);
                        pll_ssc5_reg.regValue = IoReg_Read32(PLL27X_REG_PLL_SSC5_reg);
                }while((pll_ssc5_reg.oc_done != 1) && (cnt--));

                pll_ssc0_reg.oc_en = 0;
                IoReg_Write32(PLL27X_REG_PLL_SSC0_reg, pll_ssc0_reg.regValue);
                pll_ssc4_reg.en_ssc = 1;
                IoReg_Write32(PLL27X_REG_PLL_SSC4_reg, pll_ssc4_reg.regValue);

                dclkss_reg.regValue = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
                dclkss_reg.dclk_ss_load = 1;
                dclkss_reg.dclk_ss_en = 1;
                IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, dclkss_reg.regValue);	
        }

}


unsigned char HAL_VBE_DISP_SetVideoMirror(BOOLEAN bIsH,BOOLEAN bIsV)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result =0;
#endif
	VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE *config_data;
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;
	sfg_sfg_ctrl_0_RBUS sfg_ctrl_0_reg;
	sfg_ctrl_0_reg.regValue = IoReg_Read32(SFG_SFG_CTRL_0_reg);
	 //hflip
	if(bIsH)
	{
		sfg_ctrl_0_reg.h_flip_en = 1;

	}
	else
	{
		sfg_ctrl_0_reg.h_flip_en = 0;
	}
	if((Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE)||(Get_DISPLAY_PANEL_TYPE() == P_CEDS)){
		//EPI V16DRD Horizontal need hflip, so dirty patch it.
		sfg_ctrl_0_reg.h_flip_en = !sfg_ctrl_0_reg.h_flip_en;
	}
	IoReg_Write32(SFG_SFG_CTRL_0_reg, sfg_ctrl_0_reg.regValue);

	//vflip
	Set_Mirror_VFLIP_ENABLE(bIsV);
	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE), GFP_DCU1_LIMIT, (void**)&vir_addr_noncache);
	//phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);
	config_data = (VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE *)vir_addr_noncache;
	config_data->videoPlane = VO_VIDEO_PLANE_V1;
	config_data->enable =bIsV;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_ConfigVodmaVFlip,phy_addr, 0, &result))
	{
		rtd_pr_vbe_debug("RPC fail!!\n");
		dvr_free((void *)vir_addr);
		return FALSE;
	}
#endif
	dvr_free((void *)vir_addr);
	drvif_scaler3d_set_panel_info(); //send panel info to video fw
	return TRUE;

}

extern unsigned char get_MEMC_bypass_status_refer_platform_model(void);
extern unsigned char get_MEMC_UI_status_refer_platform_model(void);

unsigned char HAL_VBE_DISP_Get_MEMC_Bypass_Status(void)
{
	if(get_MEMC_bypass_status_refer_platform_model() == FALSE)
		return FALSE;
	else
		return TRUE;
}

BOOLEAN HAL_VBE_DISP_Get_MEMC_UI_Status(void)
{
	if(get_MEMC_UI_status_refer_platform_model() == FALSE)
		return FALSE;
	else
		return TRUE;
}

void HAL_VBE_DISP_SetDisplayFormat(KADP_DISP_PANEL_ATTRIBUTE_T * pPanelAttr,UINT8 framerate)
{
//	sfg_sfg_zz_shift_dbg_ctrl_RBUS sfg_zz_shift_dbg_ctrl_reg;
//	pif_lvds_ctrl3_RBUS  lvds_ctrl3;
//	pif_lvds_ctrl0_RBUS lvds_ctrl0;
	unsigned int panel_width,panel_height;
	PANEL_CONFIG_PARAMETER parameter;
	STRUCT_PANEL_SPECIAL_SETTING panel_special_para;
	down(&VBE_Semaphore);
	panel_width=(pPanelAttr->panelResolution>>12)&0xffff;
	panel_height=pPanelAttr->panelResolution&0xfff;

	memset(&parameter,0,sizeof(PANEL_CONFIG_PARAMETER));
#if 0 // no used , because decide by bootcode
	lvds_ctrl0.regValue = IoReg_Read32(PIF_LVDS_CTRL0_reg);
/*
	if(pPanelAttr->ePanelInterface==DISP_PANEL_HSLVDS)
	{
		parameter.iCONFIG_PANEL_TYPE=P_HSLVDS_TYPE;
		lvds_ctrl0.plt_lvds_mode= 1;
	}
	else
*/
	if(pPanelAttr->ePanelInterface==DISP_PANEL_EPI)
	{
		parameter.iCONFIG_PANEL_TYPE=P_EPI_TYPE;
		lvds_ctrl0.plt_lvds_mode = 2;    /*EPI-mode*/
	}
	else if(pPanelAttr->ePanelInterface==DISP_PANEL_Vx1)
	{
		parameter.iCONFIG_PANEL_TYPE=P_VBY1_4K2K;
			lvds_ctrl0.plt_lvds_mode = 3;	 /*Vby1-mode*/
	}
	else
	{
		parameter.iCONFIG_PANEL_TYPE=P_LVDS_2K1K;
	}
	IoReg_Write32(PIF_LVDS_CTRL0_reg, lvds_ctrl0.regValue);
#endif
	parameter.iCONFIG_DISPLAY_PORT=pPanelAttr->eLVDSPixel;
	parameter.iCONFIG_DISPLAY_BITMAPPING_TABLE=(pPanelAttr->eLVDSSelect)?0:1;  //pPanelAttr.eLVDSSelect 0 is VESA,1 is JEIDA
	parameter.iCONFIG_DISPLAY_COLOR_BITS=(pPanelAttr->eColorDepth)?0:1;   //pPanelAttr.eColorDepth 0 is 8bit,1 is 10bit
	parameter.iCONFIG_DISPLAY_HORIZONTAL_SYNC_NORMAL=1;
	parameter.iCONFIG_DISPLAY_VERTICAL_SYNC_NORMAL=1;

	parameter.iCONFIG_DISP_ACT_STA_HPOS=0;
	parameter.iCONFIG_DISP_ACT_END_HPOS=panel_width;
	parameter.iCONFIG_DISP_ACT_STA_VPOS=0;
	parameter.iCONFIG_DISP_ACT_END_VPOS=panel_height;
	parameter.iCONFIG_DISP_HORIZONTAL_TOTAL=pPanelAttr->horizTypPeriod; //Dh total need as even

	parameter.iCONFIG_DISPLAY_REFRESH_RATE=framerate;
	if(parameter.iCONFIG_PANEL_TYPE==P_LVDS_2K1K)
	{
		if(parameter.iCONFIG_DISPLAY_PORT)
		{
			parameter.iCONFIG_DISPLAY_CLOCK_MIN=126;
			parameter.iCONFIG_DISPLAY_CLOCK_MAX=154;
		}
		else
		{
			parameter.iCONFIG_DISPLAY_CLOCK_MIN=50;
			parameter.iCONFIG_DISPLAY_CLOCK_MAX=85;
		}
	}
	else if((parameter.iCONFIG_PANEL_TYPE==P_EPI_TYPE)||(parameter.iCONFIG_PANEL_TYPE==P_VBY1_4K2K))
	{  //don't use ,fix 297MHz
		parameter.iCONFIG_DISPLAY_CLOCK_MIN=126;
		parameter.iCONFIG_DISPLAY_CLOCK_MAX=154;
	}
	else
	{
		parameter.iCONFIG_DISPLAY_CLOCK_MIN=126;
		parameter.iCONFIG_DISPLAY_CLOCK_MAX=154;
	}
	parameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MIN=pPanelAttr->vertBackPorch50Hz;
	parameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MAX=pPanelAttr->vertTypPeriod50Hz;
	parameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MIN=pPanelAttr->vertBackPorch60Hz;
	parameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MAX=pPanelAttr->vertTypPeriod60Hz;

	if(framerate==48)
	{
		parameter.iCONFIG_DISP_VERTICAL_TOTAL=pPanelAttr->vertTypPeriod48Hz;
		parameter.iCONFIG_DISPLAY_CLOCK_TYPICAL=framerate*pPanelAttr->horizTypPeriod*pPanelAttr->vertTypPeriod48Hz;
		parameter.iCONFIG_DISP_DEN_STA_VPOS=pPanelAttr->vertBackPorch48Hz;
		parameter.iCONFIG_DISP_DEN_END_VPOS=pPanelAttr->vertBackPorch48Hz+panel_height;
		parameter.iCONFIG_DISP_VSYNC_LENGTH=pPanelAttr->vertSyncWidth48Hz;
		parameter.iCONFIG_DISP_HSYNC_WIDTH =pPanelAttr->horizSyncWidth48Hz;
		parameter.iCONFIG_DISP_DEN_STA_HPOS=pPanelAttr->horizBackPorch48Hz;
		parameter.iCONFIG_DISP_DEN_END_HPOS=pPanelAttr->horizBackPorch48Hz+panel_width;
	}
	else if(framerate==50)
	{
		parameter.iCONFIG_DISP_VERTICAL_TOTAL=pPanelAttr->vertTypPeriod50Hz;
		parameter.iCONFIG_DISPLAY_CLOCK_TYPICAL=framerate*pPanelAttr->horizTypPeriod*pPanelAttr->vertTypPeriod50Hz;
		parameter.iCONFIG_DISP_DEN_STA_VPOS=pPanelAttr->vertBackPorch50Hz;
		parameter.iCONFIG_DISP_DEN_END_VPOS=pPanelAttr->vertBackPorch50Hz+panel_height;
		parameter.iCONFIG_DISP_VSYNC_LENGTH=pPanelAttr->vertSyncWidth50Hz;
		parameter.iCONFIG_DISP_HSYNC_WIDTH =pPanelAttr->horizSyncWidth50Hz;
		parameter.iCONFIG_DISP_DEN_STA_HPOS=pPanelAttr->horizBackPorch50Hz;
		parameter.iCONFIG_DISP_DEN_END_HPOS=pPanelAttr->horizBackPorch50Hz+panel_width;
	}
	else
	{
		parameter.iCONFIG_DISP_VERTICAL_TOTAL=pPanelAttr->vertTypPeriod60Hz;
		parameter.iCONFIG_DISPLAY_CLOCK_TYPICAL=framerate*pPanelAttr->horizTypPeriod*pPanelAttr->vertTypPeriod60Hz;
		parameter.iCONFIG_DISP_DEN_STA_VPOS=pPanelAttr->vertBackPorch60Hz;
		parameter.iCONFIG_DISP_DEN_END_VPOS=pPanelAttr->vertBackPorch60Hz+panel_height;
		parameter.iCONFIG_DISP_VSYNC_LENGTH=pPanelAttr->vertSyncWidth60Hz;
		parameter.iCONFIG_DISP_HSYNC_WIDTH =pPanelAttr->horizSyncWidth60Hz;
		parameter.iCONFIG_DISP_DEN_STA_HPOS=pPanelAttr->horizBackPorch60Hz;
		parameter.iCONFIG_DISP_DEN_END_HPOS=pPanelAttr->horizBackPorch60Hz+panel_width;
	}

	parameter.iCONFIG_DISP_HSYNC_LASTLINE=pPanelAttr->horizTypPeriod-1;

	parameter.iVFLIP=Get_Mirror_VFLIP_ENABLE();
//	Panel_InitParameter(&parameter);

	Set_PANEL_INCH(pPanelAttr->ePanelInch);
	Set_PANEL_MAKER(pPanelAttr->ePanelType);
	panel_special_para.HBackPorch48Hz=pPanelAttr->horizBackPorch48Hz;
	panel_special_para.HSyncWidth48Hz=pPanelAttr->horizSyncWidth48Hz;
	panel_special_para.VTypPeriod48Hz=pPanelAttr->vertTypPeriod48Hz;
	panel_special_para.VBackPorch48Hz=pPanelAttr->vertBackPorch48Hz;
	panel_special_para.VSyncWidth48Hz=pPanelAttr->vertSyncWidth48Hz;

	panel_special_para.HBackPorch50Hz=pPanelAttr->horizBackPorch50Hz;
	panel_special_para.HSyncWidth50Hz=pPanelAttr->horizSyncWidth50Hz;
	panel_special_para.VTypPeriod50Hz=pPanelAttr->vertTypPeriod50Hz;
	panel_special_para.VBackPorch50Hz=pPanelAttr->vertBackPorch50Hz;
	panel_special_para.VSyncWidth50Hz=pPanelAttr->vertSyncWidth50Hz;

	panel_special_para.HBackPorch60Hz=pPanelAttr->horizBackPorch60Hz;
	panel_special_para.HSyncWidth60Hz=pPanelAttr->horizSyncWidth60Hz;
	panel_special_para.VTypPeriod60Hz=pPanelAttr->vertTypPeriod60Hz;
	panel_special_para.VBackPorch60Hz=pPanelAttr->vertBackPorch60Hz;
	panel_special_para.VSyncWidth60Hz=pPanelAttr->vertSyncWidth60Hz;

#if 0
		sfg_zz_shift_dbg_ctrl_reg.regValue = IoReg_Read32(SFG_SFG_ZZ_shift_dbg_CTRL_reg);
		sfg_zz_shift_dbg_ctrl_reg.dout_mode = parameter.iCONFIG_DISPLAY_COLOR_BITS;
		IoReg_Write32(SFG_SFG_ZZ_shift_dbg_CTRL_reg, sfg_zz_shift_dbg_ctrl_reg.regValue);

		lvds_ctrl3.regValue = IoReg_Read32(PIF_LVDS_CTRL3_reg);
		lvds_ctrl3.lvds_map = parameter.iCONFIG_DISPLAY_BITMAPPING_TABLE;
		//lvds_ctrl3.lvds_map = Get_DISPLAY_BITMAPPING_TABLE()?1:0;  //0 is JEIDA,1is VESA
		IoReg_Write32(PIF_LVDS_CTRL3_reg, lvds_ctrl3.regValue);
#endif
#if 0

	drvif_scaler3d_set_panel_info(); //send panel info to video fw

	Scaler_Set_EnterFRC_LastLine_FixDclk_DVTotoal_Limit_Enable_Flag(_ENABLE);
	Scaler_Set_EnterFRC_LastLine_FixDclk_DVTotoal_Limit_Upper(Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MAX());
	Scaler_Set_EnterFRC_LastLine_FixDclk_DVTotoal_Limit_Lower(Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN());
	Scaler_Set_FixLastLine_DVTotoal_Limit_ByVfreq_Enable_Flag(_ENABLE);
	Scaler_Set_FixLastLine_DVTotal_Limit_60Hz_Typical(1125);
	Scaler_Set_FixLastLine_DVTotal_Limit_60Hz_Upper(Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MAX());
	Scaler_Set_FixLastLine_DVTotal_Limit_60Hz_Lower(Get_PANEL_DISP_VERTICAL_TOTAL_60Hz_MIN());
	Scaler_Set_FixLastLine_DVTotal_Limit_50Hz_Typical(1350);
	Scaler_Set_FixLastLine_DVTotal_Limit_50Hz_Upper(Get_PANEL_DISP_VERTICAL_TOTAL_50Hz_MAX());
	Scaler_Set_FixLastLine_DVTotal_Limit_50Hz_Lower(Get_PANEL_DISP_VERTICAL_TOTAL_50Hz_MIN());
#endif
	up(&VBE_Semaphore);
}

extern unsigned char rtk_rerun_scaler(SCALER_DISP_CHANNEL channel);

void HAL_VBE_DISP_SetFrameRate(BOOLEAN isForceFreeRun,UINT16 frameRates)
{
	unsigned int vtotal = 0;
	ppoverlay_dv_total_RBUS dv_total_reg;
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;

	ppoverlay_dispd_smooth_toggle1_RBUS	dispd_smooth_toggle1_reg;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS 	dispd_stage1_sm_ctrl_reg;
	ppoverlay_dtg_m_multiple_vsync_RBUS   dtg_m_multiple_vsync_reg;
	ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
	unsigned int timeout = 0;
	unsigned char frame_idicator=0;

	down(&VBE_Semaphore);

        display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

	if(isForceFreeRun)
	{
		Disp_Set_framerate(frameRates);
		if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))
		{
			rtd_pr_vbe_debug("data fs do free run need to restart\n");
			Disp_Enable_isForceFreeRun(isForceFreeRun);
			rtk_rerun_scaler(SLR_MAIN_DISPLAY);
			rtk_rerun_scaler(SLR_SUB_DISPLAY);
		}
		else
		{

			modestate_decide_pixel_mode();
			Scaler_DispSetInputInfo(SLR_INPUT_FRAC_FS_REMOVE_RATIO, 1);

			if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_PIXEL_MODE) == PIXEL_MODE_1_PIXEL){ //60hz
				if(frameRates == 48){
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 48);
				}else if(frameRates == 50){
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 50);
				}else if(frameRates == 60){
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
				}else{
					rtd_pr_vbe_err("[%s]Set Framerate (%d) error. Reset to 60Hz\n", __FUNCTION__, frameRates);
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
				}
			}else{
				if((frameRates == 50)||(frameRates == 100)){
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 50);
				}else if((frameRates == 60)||(frameRates == 120)){
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
				}else{
					rtd_pr_vbe_err("[%s]Set Framerate (%d) error. Reset to 120Hz\n", __FUNCTION__, frameRates);
					Scaler_DispSetInputInfo(SLR_INPUT_DTG_MASTER_V_FREQ, 60);
				}
			}

			drivf_scaler_reset_freerun();
			modestate_set_fll_running_flag(_DISABLE);
			modestate_disp_flow_double_buf_enable(_ENABLE);

			dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
			dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
			IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);

			dtg_m_multiple_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_multiple_vsync_reg);
			frame_idicator = dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync;
			rtd_pr_vbe_debug("*uzudtg_remove_half_ivs:%d\n", frame_idicator);

			if(frame_idicator > 0){ 	// run frame check
				timeout = 0xa0000; //100ms, 5 frame.
				dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
				while((dtg_frame_cnt1_reg.uzudtg_input_frame_cnt!= (frame_idicator-1))&& --timeout){
					dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
				}
				if(timeout == 0)
					rtd_pr_vbe_notice("[%s][%d]count timeout !!!\n", __FUNCTION__, __LINE__);
				//rtd_pr_vbe_notice("1. frame_idicator:%d\n", (IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)&0x3f000000)>>24);
				timeout = 0x25000; //20ms, 1 frame.
				//make sure that we can have a full porch apply timing change settings
				while((dtg_frame_cnt1_reg.uzudtg_input_frame_cnt!= frame_idicator)&& --timeout){
					dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
				}
				if(timeout == 0)
					rtd_pr_vbe_notice("[%s][%d]count timeout !!!\n", __FUNCTION__, __LINE__);
				//rtd_pr_vbe_notice("2. frame_idicator:%d\n", (IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)&0x3f000000)>>24);
			}
			modestate_set_display_timing_gen_clock_div();
			modestate_set_fractional_framesync();

			timeout = 0x25000;	//20ms, 1 frame.
			//rtd_pr_vbe_notice("+++uzudtg line_cnt:%d\n", (IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg)&0x0fff0000)>>16);
			dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
			dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
			IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
			do{
				dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
			}while((dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply==1) && (--timeout));
			//rtd_pr_vbe_notice("---uzudtg line_cnt:%d\n", (IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg)&0x0fff0000)>>16);

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
				rtd_pr_vbe_debug("[%s][%d] wait dispd_stage1_smooth_toggle_apply ok!\n", __FUNCTION__, __LINE__);
				dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
				dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
				IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
			}
			fw_scaler_dtg_double_buffer_apply();
			modestate_disp_flow_double_buf_enable(_DISABLE);
			modestate_set_display_timing_gen();

		}
	}
	else
	{
		if(display_timing_ctrl1_reg.disp_fsync_en==0) //timing free run
		{
			if(!vtotal_measure)
			{
				vtotal_measure=Get_DISP_VERTICAL_TOTAL()-1;
			}
			/*WaitFor_DEN_STOP();*/
			fw_scaler_dtg_double_buffer_enable(1);
			dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
			dv_total_reg.dv_total = vtotal_measure;
			IoReg_Write32(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);
			fw_scaler_dtg_double_buffer_apply();
		//	fw_scaler_dtg_double_buffer_enable(0);
			drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
			rtd_pr_vbe_debug("framesync:DVTotal:%x, 8014:%x\n", vtotal, IoReg_Read32(PPOVERLAY_DV_total_reg));
		}
	}
	up(&VBE_Semaphore);
}
#define XTAL_FREQ 27000000
#define IVS_XTAL_CNT 1

void HAL_VBE_DISP_GetOutputFrameRate(UINT16* pframeRates)
{
	ppoverlay_memcdtg_dvs_cnt_RBUS memcdtg_dvs_cnt_reg;
	unsigned int vSyncFreq = 0;

	if(Get_Val_scaler_low_power_mode())
	{//low power mode
		*pframeRates  = 60;
		return;
	}

	down(&VBE_Semaphore);

	if(Get_Val_scaler_low_power_mode())
	{//low power mode
		*pframeRates  = 60;
		up(&VBE_Semaphore);
		return;
	}

	memcdtg_dvs_cnt_reg.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DVS_cnt_reg);
       vSyncFreq = 270000000 / (memcdtg_dvs_cnt_reg.memcdtg_dvs_cnt);
	if((vSyncFreq%10) >5)
		*pframeRates = (vSyncFreq/10)+1;
	else
		*pframeRates = (vSyncFreq/10);

	//rtd_pr_vbe_debug("Get output frame rate : %d \n", *pframeRates);

	up(&VBE_Semaphore);

#if 0	//fix me later by ben.
	unsigned int vSyncFreq = 0;
	display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	dv_total_RBUS dv_total_reg;
	dh_total_last_line_length_RBUS dh_total_last_line_length_reg;
	unsigned int dclkVal=0;
	unsigned int ivs_freq=0;
	unsigned int ivs_cnt=0;
	unsigned char ivs_freq_1=0;

	down(&VBE_Semaphore);
	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

	if((display_timing_ctrl1_reg.disp_frc_fsync == 1)&&(display_timing_ctrl1_reg.disp_fsync_en == 1)){	//framesync timing
		framesync_wait_vdc_stable();
		ivs_cnt = ((UINT32) framesync_new_fixlastline_get_stable_xtal_cnt(IVS_XTAL_CNT));
		ivs_freq = (((UINT32) XTAL_FREQ * 100) / ivs_cnt);
	//	vSyncFreq = scalerdisplay_get_framerate(_TYPE_IVS);
		ivs_freq_1 =ivs_freq%100;
		if(ivs_freq_1>=90) //measure value would like 2390 or 2399
		{
			ivs_freq=ivs_freq+(100-ivs_freq_1);
		}
		vSyncFreq=ivs_freq/100;
		if(display_timing_ctrl1_reg.multiple_dvs == 1){
			vSyncFreq = vSyncFreq*2;
		}
	}
	else
	{	// free run timing
		dclkVal= scalerdisplay_get_dclk();
		dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
		dh_total_last_line_length_reg.regValue = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);

		vSyncFreq = ((dclkVal/dh_total_last_line_length_reg.dh_total)/(dv_total_reg.dv_total));
		if(display_timing_ctrl1_reg.multiple_dvs == 1){
			vSyncFreq = vSyncFreq*2;
		}
	}
	rtd_pr_vbe_info("Get output frame rate : %d \n", vSyncFreq);
	*pframeRates=vSyncFreq;
	up(&VBE_Semaphore);
#endif
}

/* framerate = dvs frame rate * 100 */
bool get_dvs_framerate(int *framerate)
{
	bool ret = false;
	uint32_t dvs_count, vsync_freq;

	dvs_count = PPOVERLAY_DVS_cnt_get_dvs_cnt(IoReg_Read32(PPOVERLAY_DVS_cnt_reg));

	if (dvs_count > 0)
		vsync_freq = 27000000ull * 100 / dvs_count;
	else
		vsync_freq = 0;

	*framerate = vsync_freq;

	if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE)
			!= _MODE_STATE_ACTIVE)
		ret = false;
	else
		ret = true;

	pr_debug("[ScalerDisplay] Get DVS Frame rate value : %d\n", vsync_freq);

	return ret;
}

void HAL_VBE_DISP_SetBackgroundColor(UINT8 red,UINT8 green,UINT8 blue)
{
	unsigned int color_r, color_b, color_g;

	color_r = red<<8;
	color_g = green<<8;
	color_b = blue<<8;

	drvif_scalerdisplay_set_bg_color(SLR_MAIN_DISPLAY, _DISPLAY_BG,  color_r, color_g, color_b);
	/*drvif_scalerDTG_set_bg_color(red,green,blue,1);*/
}

void HAL_VBE_DISP_SetDisplayOutput_5060HZ(BOOLEAN bOnOff)
{
	extern unsigned char vsc_force_rerun_main_scaler;
	extern struct semaphore *get_hdmi_detectsemaphore(void);
	extern void Set_Reply_Zero_Timing_Flag(unsigned char src, unsigned char flag);
	extern void HDMI_set_detect_flag(unsigned char enable);

	rtd_pr_vbe_notice("[%s] bOnOff : %d\n", __FUNCTION__, bOnOff);

	vbe_disp_set_force_60hz_panel_type(FALSE);

	if(bOnOff){
		if(Get_DISPLAY_REFRESH_RATE() == 60){
			// do nothing
		}
		else{	// 120hz panel force 60hz
			vbe_disp_lane_power_off();

			// 60hz panel parameter config
			vbe_disp_set_force_60hz_panel_type(TRUE);

			drivf_scaler_reset_freerun();

			// re-trigger scaler task.
			Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE, _MODE_STATE_SEARCH);//Let main path to search state. In order to request VO
			DbgSclrFlgTkr.Main_Run_Scaler_flag = TRUE;//trigger run scaler
			DbgSclrFlgTkr.Main_Scaler_Stop_flag = FALSE;//Let scaler can be run
			vsc_force_rerun_main_scaler = TRUE;
			if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI){
				reset_hdmi_timing_ready();
				down(get_hdmi_detectsemaphore());
				if (get_HDMI_Global_Status() == SRC_CONNECT_DONE) {
					vfe_hdmi_drv_handle_on_line_measure_error(0);
					Set_Reply_Zero_Timing_Flag(VSC_INPUTSRC_HDMI, REPORT_ZERO_TIMING);/*Let Webos get zero timing*/
					HDMI_set_detect_flag(TRUE);
				}
				up(get_hdmi_detectsemaphore());
			}
			// setup 120hz panel interface
			vbe_disp_config_60hz_panel_interface(TRUE);
		}
	}else{
		if(Get_DISPLAY_REFRESH_RATE() == 60){
			// do nothing
		}
		else{	// recover to 120hz
			vbe_disp_lane_power_off();

			// 120hz panel parameter config
			vbe_disp_set_force_60hz_panel_type(FALSE);

			drivf_scaler_reset_freerun();

			// re-trigger scaler task.
			Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE, _MODE_STATE_SEARCH);//Let main path to search state. In order to request VO
			DbgSclrFlgTkr.Main_Run_Scaler_flag = TRUE;//trigger run scaler
			DbgSclrFlgTkr.Main_Scaler_Stop_flag = FALSE;//Let scaler can be run
			vsc_force_rerun_main_scaler = TRUE;
			if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI){
				reset_hdmi_timing_ready();
				down(get_hdmi_detectsemaphore());
				if (get_HDMI_Global_Status() == SRC_CONNECT_DONE) {
					vfe_hdmi_drv_handle_on_line_measure_error(0);
					Set_Reply_Zero_Timing_Flag(VSC_INPUTSRC_HDMI, REPORT_ZERO_TIMING);/*Let Webos get zero timing*/
					HDMI_set_detect_flag(TRUE);
				}
				up(get_hdmi_detectsemaphore());
			}
			// setup 120hz panel interface
			vbe_disp_config_60hz_panel_interface(FALSE);
		}
	}
}

#ifdef CONFIG_ORBIT_PIXEL_SHIFT_ALGO


void HAL_VBE_DISP_OLED_SetOrbit(BOOLEAN bEnable, unsigned char mode)
{
	static BOOLEAN pre_bEnable=0;
	down(&VBE_Semaphore);

	rtd_pr_vbe_notice("HAL_VBE_DISP_OLED_SetOrbit : %d \n", bEnable);
	vbe_disp_set_orbit(bEnable, mode);
	if(pre_bEnable==0 && bEnable == 1)
	{
		ScalerForceUpdateOrbit(0);
	}
	pre_bEnable = bEnable;

	up(&VBE_Semaphore);
}

void vbe_disp_set_orbit(UINT8 bEnable, unsigned char mode)
{

	if(bEnable!=vbe_disp_oled_orbit_enable  ){
		vbe_disp_oled_orbit_enable = bEnable;
        Scaler_reset_orbit();
	}else{
		vbe_disp_oled_orbit_enable = bEnable;
	}
    if(mode != vbe_disp_oled_orbit_mode)
    {
        Scaler_reset_orbit();
        vbe_disp_oled_orbit_mode = mode;
        vbe_disp_oled_orbit_enable = bEnable;
		ScalerForceUpdateOrbit(0);
    }
	else
	{
         vbe_disp_oled_orbit_mode = mode;
	}
	if(bEnable){
//		if(drvif_scalerdisplay_get_force_bg_status()==FALSE) {
			Scaler_start_orbit_algo(TRUE);
//		}
    }else{
		SCALER_ORBIT_SHIFT_INFO scalerOrbitMemInfo = {0};
		scalerOrbitMemInfo.orbit_mode = _VBE_PANEL_OREBIT_MODE_MAX;
		vbe_disp_set_orbit_shift_info_sharememory(scalerOrbitMemInfo);
	}
}


#endif

void vbe_resume_instanboot_Panel_SetBackLight(int enable)
{
        return;
}

unsigned char vbe_disp_get_BoeVersion(UINT8 type, KADP_DISP_BOE_VERSION_T *pVer)
{
#if 0
	short i, cnt;
	dispd_boe_rgbw_mcu_control_RBUS dispd_boe_rgbw_mcu_control_REG;
	dispd_boe_rgbw_mcu_st_RBUS dispd_boe_rgbw_mcu_st_RBEG;
	dispd_boe_rgbw_apb_w_control_RBUS dispd_boe_rgbw_apb_w_control_REG;
	dispd_boe_rgbw_apb_w_data_RBUS dispd_boe_rgbw_apb_w_data_REG;
	dispd_boe_rgbw_apb_r_control_RBUS dispd_boe_rgbw_apb_r_control_reg;
	dispd_boe_rgbw_apb_r_data_RBUS dispd_boe_rgbw_apb_r_data_reg;

	dispd_boe_rgbw_apb_w_control_REG.regValue = IoReg_Read32(DISPD_BOE_RGBW_APB_w_control_reg);
	dispd_boe_rgbw_apb_w_data_REG.regValue = IoReg_Read32(DISPD_BOE_RGBW_APB_w_data_reg);

	dispd_boe_rgbw_apb_w_control_REG.apb_w_mode = 0; //	0:addr +1, 1:addr +2, 2:addr +4
	dispd_boe_rgbw_apb_w_control_REG.apb_w_addr= 0xc008;
	if(type == BOE_ROM_VERSION)
		dispd_boe_rgbw_apb_w_data_REG.apb_w_data0 = 0xee;
	else if(type == BOE_FW_VERSION)
		dispd_boe_rgbw_apb_w_data_REG.apb_w_data0 = 0xef;
	else{
		dispd_boe_rgbw_apb_w_data_REG.apb_w_data0 = 0xee;
		rtd_pr_vbe_err("[%s][%d] [ERROR] Unknow Mode\n", __FUNCTION__,__LINE__);
	}
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_control_reg, dispd_boe_rgbw_apb_w_control_REG.regValue);
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_data_reg, dispd_boe_rgbw_apb_w_data_REG.regValue);

	//wclr status
	dispd_boe_rgbw_mcu_st_RBEG.regValue = IoReg_Read32(DISPD_BOE_RGBW_mcu_st_reg);
	dispd_boe_rgbw_mcu_st_RBEG.mcu_int_st = 1;
	IoReg_Write32(DISPD_BOE_RGBW_mcu_st_reg, dispd_boe_rgbw_mcu_st_RBEG.regValue);
	rtd_pr_vbe_err("[%s][%d] DISPD_BOE_RGBW_mcu_st_reg=%d \n", __FUNCTION__,__LINE__, IoReg_Read32(DISPD_BOE_RGBW_mcu_st_reg)&_BIT24);

	// int_in=0 -> wait status=1 -> int_in=1
	dispd_boe_rgbw_mcu_control_REG.regValue = IoReg_Read32(DISPD_BOE_RGBW_mcu_control_reg);
	dispd_boe_rgbw_mcu_control_REG.mcu_int_in = 0;
	IoReg_Write32(DISPD_BOE_RGBW_mcu_control_reg, dispd_boe_rgbw_mcu_control_REG.regValue);
	cnt = 0x3ffff;
	do{ // wait b802EC20[24]=1
		dispd_boe_rgbw_mcu_st_RBEG.regValue = IoReg_Read32(DISPD_BOE_RGBW_mcu_st_reg);
	}while((cnt--) && dispd_boe_rgbw_mcu_st_RBEG.mcu_int_st == 0);
	if(cnt == 0){
		rtd_pr_vbe_err("[%s][%d] [ERROR] mcu_int_st timeout\n", __FUNCTION__,__LINE__);
	}

	dispd_boe_rgbw_apb_r_control_reg.regValue = IoReg_Read32(DISPD_BOE_RGBW_APB_r_control_reg);
	dispd_boe_rgbw_apb_r_control_reg.apb_r_addr = 0xc00c;
	IoReg_Write32(DISPD_BOE_RGBW_APB_r_control_reg, dispd_boe_rgbw_apb_r_control_reg.regValue);
	// read data.
	dispd_boe_rgbw_apb_r_data_reg.regValue = IoReg_Read32(DISPD_BOE_RGBW_APB_r_data_reg);
	pVer->b0 = dispd_boe_rgbw_apb_r_data_reg.apb_r_data;
	dispd_boe_rgbw_apb_r_data_reg.regValue = IoReg_Read32(DISPD_BOE_RGBW_APB_r_data_reg);
	pVer->b1 = dispd_boe_rgbw_apb_r_data_reg.apb_r_data;
	dispd_boe_rgbw_apb_r_data_reg.regValue = IoReg_Read32(DISPD_BOE_RGBW_APB_r_data_reg);
	pVer->b2 = dispd_boe_rgbw_apb_r_data_reg.apb_r_data;

	//end communication
	dispd_boe_rgbw_mcu_control_REG.mcu_int_in = 1;
	IoReg_Write32(DISPD_BOE_RGBW_mcu_control_reg, dispd_boe_rgbw_mcu_control_REG.regValue);
	dispd_boe_rgbw_apb_w_control_REG.regValue = IoReg_Read32(DISPD_BOE_RGBW_APB_w_control_reg);
	dispd_boe_rgbw_apb_w_data_REG.regValue = IoReg_Read32(DISPD_BOE_RGBW_APB_w_data_reg);
	dispd_boe_rgbw_apb_w_control_REG.apb_w_mode = 0; // 0:addr +1, 1:addr +2, 2:addr +4
	dispd_boe_rgbw_apb_w_control_REG.apb_w_addr= 0xc008;
	dispd_boe_rgbw_apb_w_data_REG.apb_w_data0 = 0x00;
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_control_reg, dispd_boe_rgbw_apb_w_control_REG.regValue);
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_data_reg, dispd_boe_rgbw_apb_w_data_REG.regValue);

	rtd_pr_vbe_notice("[vbe_disp_get_BoeVersion] pVer (%x.%x.%x)\n",
		pVer->b0, pVer->b1, pVer->b2);
#endif
	return TRUE;
}

unsigned char vbe_disp_set_BoeMode(UINT8 *pFwData, UINT32 size)
{
#if 0
	UINT32 i, cnt;
	dispd_boe_rgbw_apb_w_control_RBUS dispd_boe_rgbw_apb_w_control_REG;
	dispd_boe_rgbw_apb_w_data_RBUS dispd_boe_rgbw_apb_w_data_REG;
	dispd_boe_rgbw_mcu_control_RBUS dispd_boe_rgbw_mcu_control_REG;
	dispd_boe_rgbw_mcu_st_RBUS dispd_boe_rgbw_mcu_st_RBEG;
	UINT16* pWriteData = (UINT16*)(pFwData);

	// Himax suspend
	dispd_boe_rgbw_apb_w_control_REG.regValue = 0;
	dispd_boe_rgbw_apb_w_data_REG.regValue = 0;
	dispd_boe_rgbw_apb_w_control_REG.apb_w_mode = 0; // 0:addr +1, 1:addr +2, 2:addr +4
	dispd_boe_rgbw_apb_w_control_REG.apb_w_addr= 0xc008;
	dispd_boe_rgbw_apb_w_data_REG.apb_w_data0 = 0x10;
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_control_reg, dispd_boe_rgbw_apb_w_control_REG.regValue);
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_data_reg, dispd_boe_rgbw_apb_w_data_REG.regValue);

	//wclr status
	dispd_boe_rgbw_mcu_st_RBEG.regValue = IoReg_Read32(DISPD_BOE_RGBW_mcu_st_reg);
	dispd_boe_rgbw_mcu_st_RBEG.mcu_int_st = 1;
	IoReg_Write32(DISPD_BOE_RGBW_mcu_st_reg, dispd_boe_rgbw_mcu_st_RBEG.regValue);
	//rtd_pr_vbe_notice("[%s][%d] ++DISPD_BOE_RGBW_mcu_st_reg=%d \n", __FUNCTION__,__LINE__, IoReg_Read32(DISPD_BOE_RGBW_mcu_st_reg)&_BIT24);

	dispd_boe_rgbw_mcu_control_REG.regValue = IoReg_Read32(DISPD_BOE_RGBW_mcu_control_reg);
	dispd_boe_rgbw_mcu_control_REG.mcu_int_in = 0;
	IoReg_Write32(DISPD_BOE_RGBW_mcu_control_reg, dispd_boe_rgbw_mcu_control_REG.regValue);
	cnt = 0x3ffff;
	do{ // wait b802EC20[24]=1
		dispd_boe_rgbw_mcu_st_RBEG.regValue = IoReg_Read32(DISPD_BOE_RGBW_mcu_st_reg);
	}while((cnt--) && dispd_boe_rgbw_mcu_st_RBEG.mcu_int_st == 0);
	if(cnt == 0){
		rtd_pr_vbe_err("[%s][%d] [ERROR] mcu_int_st timeout\n", __FUNCTION__,__LINE__);
	}
	//rtd_pr_vbe_notice("[%s][%d] DISPD_BOE_RGBW_mcu_st_reg=%d \n", __FUNCTION__,__LINE__, IoReg_Read32(DISPD_BOE_RGBW_mcu_st_reg)&_BIT24);

	//end communication
	dispd_boe_rgbw_mcu_control_REG.mcu_int_in = 1;
	IoReg_Write32(DISPD_BOE_RGBW_mcu_control_reg, dispd_boe_rgbw_mcu_control_REG.regValue);
	dispd_boe_rgbw_apb_w_control_REG.regValue = 0;
	dispd_boe_rgbw_apb_w_data_REG.regValue = 0;
	dispd_boe_rgbw_apb_w_control_REG.apb_w_mode = 0; // 0:addr +1, 1:addr +2, 2:addr +4
	dispd_boe_rgbw_apb_w_control_REG.apb_w_addr= 0xc008;
	dispd_boe_rgbw_apb_w_data_REG.apb_w_data0 = 0x00;
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_control_reg, dispd_boe_rgbw_apb_w_control_REG.regValue);
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_data_reg, dispd_boe_rgbw_apb_w_data_REG.regValue);
	//rtd_pr_vbe_notice("[%s][%d]end communication\n", __FUNCTION__,__LINE__);

	//RESET RGBW-IP-MCU
	dispd_boe_rgbw_apb_w_control_REG.regValue = 0;
	dispd_boe_rgbw_apb_w_data_REG.regValue = 0;
	dispd_boe_rgbw_apb_w_control_REG.apb_w_mode = 0; // 0:addr +1, 1:addr +2, 2:addr +4
	dispd_boe_rgbw_apb_w_control_REG.apb_w_addr= 0xc000;
	dispd_boe_rgbw_apb_w_data_REG.apb_w_data0 = 0x00;
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_control_reg, dispd_boe_rgbw_apb_w_control_REG.regValue);
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_data_reg, dispd_boe_rgbw_apb_w_data_REG.regValue);
	//rtd_pr_vbe_notice("[%s][%d]RESET RGBW-IP-MCU\n", __FUNCTION__,__LINE__);

	//Access 32K SRAM via APB
	dispd_boe_rgbw_apb_w_control_REG.apb_w_mode = 0; // 0:addr +1, 1:addr +2, 2:addr +4
	dispd_boe_rgbw_apb_w_control_REG.apb_w_addr= 0xc001;
	dispd_boe_rgbw_apb_w_data_REG.apb_w_data0 = 0;
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_control_reg, dispd_boe_rgbw_apb_w_control_REG.regValue);
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_data_reg, dispd_boe_rgbw_apb_w_data_REG.regValue);
	//rtd_pr_vbe_notice("[%s][%d]Access 32K SRAM via APB\n", __FUNCTION__,__LINE__);

	// === write table 0~0x7dff ===
	dispd_boe_rgbw_apb_w_control_REG.apb_w_mode = 1; // 0:addr +1, 1:addr +2, 2:addr +4
	dispd_boe_rgbw_apb_w_control_REG.apb_w_addr= 0x0000;
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_control_reg, dispd_boe_rgbw_apb_w_control_REG.regValue);

	//rtd_pr_vbe_notice("pWriteData = %x.%x\n", pWriteData[0], pWriteData[1]);

	for(i = 0; i < (size/2); i++)
	{
		dispd_boe_rgbw_apb_w_data_REG.apb_w_data0 = pWriteData[i]&0xff;
		dispd_boe_rgbw_apb_w_data_REG.apb_w_data1 = (pWriteData[i]>>8)&0xff;
		IoReg_Write32(DISPD_BOE_RGBW_APB_w_data_reg, dispd_boe_rgbw_apb_w_data_REG.regValue);
	}
	rtd_pr_vbe_notice("pWriteData[%d] = %x\n", i-1, pWriteData[i-1]);
	rtd_pr_vbe_notice("[%s][%d]write table done.\n", __FUNCTION__,__LINE__);

	// === MCU <-> SoC ===
	// C001 = 7; (Control to MCU)
	dispd_boe_rgbw_apb_w_control_REG.apb_w_mode = 0; // 0:addr +1, 1:addr +2, 2:addr +4
	dispd_boe_rgbw_apb_w_control_REG.apb_w_addr= 0xc001;
	dispd_boe_rgbw_apb_w_data_REG.apb_w_data0 = 7;
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_control_reg, dispd_boe_rgbw_apb_w_control_REG.regValue);
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_data_reg, dispd_boe_rgbw_apb_w_data_REG.regValue);
	// wait V-blank
	// C000 = 1; (Reset MCU)
	dispd_boe_rgbw_apb_w_control_REG.apb_w_mode = 0; // 0:addr +1, 1:addr +2, 2:addr +4
	dispd_boe_rgbw_apb_w_control_REG.apb_w_addr= 0xc000;
	dispd_boe_rgbw_apb_w_data_REG.apb_w_data0 = 1;
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_control_reg, dispd_boe_rgbw_apb_w_control_REG.regValue);
	IoReg_Write32(DISPD_BOE_RGBW_APB_w_data_reg, dispd_boe_rgbw_apb_w_data_REG.regValue);
#endif
	return TRUE;
}

void vbe_disp_get_disp_panel_size(KADP_DISP_PANEL_SIZE_T *panel_size)
{
    panel_size->DISP_HEIGHT = Get_DEFAULT_DISP_ACT_END_VPOS() - Get_DEFAULT_DISP_ACT_STA_VPOS();
    panel_size->DISP_WIDTH = Get_DEFAULT_DISP_ACT_END_HPOS() - Get_DEFAULT_DISP_ACT_STA_HPOS();
}


void vbe_disp_mtg_inner_ptg(UINT8 bOnOff, VBE_DISP_INNER_PTG_TYPE_T type)
{
	if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_R)
		drvif_scalerdisplay_set_bg_color(SLR_MAIN_DISPLAY, _DISPLAY_BG,  0x3fff, 0, 0);
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_G)
		drvif_scalerdisplay_set_bg_color(SLR_MAIN_DISPLAY, _DISPLAY_BG,  0, 0x3fff, 0);
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_B)
		drvif_scalerdisplay_set_bg_color(SLR_MAIN_DISPLAY, _DISPLAY_BG,  0, 0, 0x3fff);
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_WHITE)
		drvif_scalerdisplay_set_bg_color(SLR_MAIN_DISPLAY, _DISPLAY_BG,  0x3fff, 0x3fff, 0x3fff);
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_BLACK)
		drvif_scalerdisplay_set_bg_color(SLR_MAIN_DISPLAY, _DISPLAY_BG,  0, 0, 0);
	else
		drvif_scalerdisplay_set_bg_color(SLR_MAIN_DISPLAY, _DISPLAY_BG,  0, 0, 0);

	mute_control(SLR_MAIN_DISPLAY, bOnOff);
}

void vbe_disp_memc_inputmux_inner_ptg(UINT8 bOnOff, VBE_DISP_INNER_PTG_TYPE_T type)
{
	ppoverlay_memc_mux_ctrl_bg_RBUS ppoverlay_memc_mux_ctrl_bg_reg;
	ppoverlay_memc_mux_in_color_value_gb_RBUS ppoverlay_memc_mux_in_color_value_gb_reg;
	ppoverlay_memc_mux_in_color_value_r_RBUS ppoverlay_memc_mux_in_color_value_r_reg;
	ppoverlay_double_buffer_ctrl3_RBUS ppoverlay_double_buffer_ctrl3_reg;

	ppoverlay_memc_mux_ctrl_bg_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_BG_reg);
	ppoverlay_memc_mux_in_color_value_gb_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_IN_COLOR_VALUE_GB_reg);
	ppoverlay_memc_mux_in_color_value_r_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_IN_COLOR_VALUE_R_reg);
	ppoverlay_double_buffer_ctrl3_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL3_reg);


	if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_R){
		ppoverlay_memc_mux_in_color_value_gb_reg.in_cb_b_value = 0;
		ppoverlay_memc_mux_in_color_value_gb_reg.in_y_g_value = 0;
		ppoverlay_memc_mux_in_color_value_r_reg.in_cr_r_value = 0xfff;
	}
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_G){
		ppoverlay_memc_mux_in_color_value_gb_reg.in_cb_b_value = 0;
		ppoverlay_memc_mux_in_color_value_gb_reg.in_y_g_value = 0xfff;
		ppoverlay_memc_mux_in_color_value_r_reg.in_cr_r_value = 0;
	}
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_B){
		ppoverlay_memc_mux_in_color_value_gb_reg.in_cb_b_value = 0xfff;
		ppoverlay_memc_mux_in_color_value_gb_reg.in_y_g_value = 0;
		ppoverlay_memc_mux_in_color_value_r_reg.in_cr_r_value = 0;
	}
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_WHITE){
		ppoverlay_memc_mux_in_color_value_gb_reg.in_cb_b_value = 0xfff;
		ppoverlay_memc_mux_in_color_value_gb_reg.in_y_g_value = 0xfff;
		ppoverlay_memc_mux_in_color_value_r_reg.in_cr_r_value = 0xfff;
	}
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_BLACK){
		ppoverlay_memc_mux_in_color_value_gb_reg.in_cb_b_value = 0;
		ppoverlay_memc_mux_in_color_value_gb_reg.in_y_g_value = 0;
		ppoverlay_memc_mux_in_color_value_r_reg.in_cr_r_value = 0;
	}
	else{
		ppoverlay_memc_mux_in_color_value_gb_reg.in_cb_b_value = 0;
		ppoverlay_memc_mux_in_color_value_gb_reg.in_y_g_value = 0;
		ppoverlay_memc_mux_in_color_value_r_reg.in_cr_r_value = 0;
	}
	if(bOnOff){
		ppoverlay_memc_mux_ctrl_bg_reg.memc_in_bg_en = 1;
	}else{
		ppoverlay_memc_mux_in_color_value_gb_reg.in_cb_b_value = 0;
		ppoverlay_memc_mux_in_color_value_gb_reg.in_y_g_value = 0;
		ppoverlay_memc_mux_in_color_value_r_reg.in_cr_r_value = 0;
		ppoverlay_memc_mux_ctrl_bg_reg.memc_in_bg_en = 0;
	}
	IoReg_Write32(PPOVERLAY_MEMC_MUX_IN_COLOR_VALUE_GB_reg, ppoverlay_memc_mux_in_color_value_gb_reg.regValue);
	IoReg_Write32(PPOVERLAY_MEMC_MUX_IN_COLOR_VALUE_R_reg, ppoverlay_memc_mux_in_color_value_r_reg.regValue);
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_BG_reg, ppoverlay_memc_mux_ctrl_bg_reg.regValue);

	ppoverlay_double_buffer_ctrl3_reg.d1bg_dtgreg_dbuf_set= 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL3_reg, ppoverlay_double_buffer_ctrl3_reg.regValue);
}

void vbe_disp_memc_outputmux_inner_ptg(UINT8 bOnOff, VBE_DISP_INNER_PTG_TYPE_T type)
{
	ppoverlay_memc_mux_ctrl_bg_RBUS ppoverlay_memc_mux_ctrl_bg_reg;
	ppoverlay_memc_mux_out1_color_value_gb_RBUS ppoverlay_memc_mux_out1_color_value_gb_reg;
	ppoverlay_memc_mux_out1_color_value_r_RBUS ppoverlay_memc_mux_out1_color_value_r_reg;
	ppoverlay_double_buffer_ctrl3_RBUS ppoverlay_double_buffer_ctrl3_reg;

	ppoverlay_memc_mux_ctrl_bg_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_BG_reg);
	ppoverlay_memc_mux_out1_color_value_gb_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_OUT1_COLOR_VALUE_GB_reg);
	ppoverlay_memc_mux_out1_color_value_r_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_OUT1_COLOR_VALUE_R_reg);
	ppoverlay_double_buffer_ctrl3_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL3_reg);


	if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_R){
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_cb_b_value= 0;
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_y_g_value = 0;
		ppoverlay_memc_mux_out1_color_value_r_reg.out_cr_r_value = 0xfff;
	}
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_G){
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_cb_b_value = 0;
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_y_g_value = 0xfff;
		ppoverlay_memc_mux_out1_color_value_r_reg.out_cr_r_value = 0;
	}
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_B){
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_cb_b_value = 0xfff;
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_y_g_value = 0;
		ppoverlay_memc_mux_out1_color_value_r_reg.out_cr_r_value = 0;
	}
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_WHITE){
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_cb_b_value = 0xfff;
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_y_g_value = 0xfff;
		ppoverlay_memc_mux_out1_color_value_r_reg.out_cr_r_value = 0xfff;
	}
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_BLACK){
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_cb_b_value = 0;
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_y_g_value = 0;
		ppoverlay_memc_mux_out1_color_value_r_reg.out_cr_r_value = 0;
	}
	else{
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_cb_b_value = 0;
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_y_g_value= 0;
		ppoverlay_memc_mux_out1_color_value_r_reg.out_cr_r_value = 0;
	}
	if(bOnOff){
		ppoverlay_memc_mux_ctrl_bg_reg.memc_out_bg_en = 1;
	}else{
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_cb_b_value = 0;
		ppoverlay_memc_mux_out1_color_value_gb_reg.out_y_g_value = 0;
		ppoverlay_memc_mux_out1_color_value_r_reg.out_cr_r_value = 0;
		ppoverlay_memc_mux_ctrl_bg_reg.memc_out_bg_en = 0;
	}
	IoReg_Write32(PPOVERLAY_MEMC_MUX_OUT1_COLOR_VALUE_GB_reg, ppoverlay_memc_mux_out1_color_value_gb_reg.regValue);
	IoReg_Write32(PPOVERLAY_MEMC_MUX_OUT1_COLOR_VALUE_R_reg, ppoverlay_memc_mux_out1_color_value_r_reg.regValue);
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_BG_reg, ppoverlay_memc_mux_ctrl_bg_reg.regValue);

	ppoverlay_double_buffer_ctrl3_reg.d2bg_dtgreg_dbuf_en= 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL3_reg, ppoverlay_double_buffer_ctrl3_reg.regValue);
}

void vbe_disp_sfg_inner_ptg(UINT8 bOnOff, VBE_DISP_INNER_PTG_TYPE_T type)
{
	sfg_sfg_force_bg_at_dif_RBUS sfg_sfg_force_bg_at_dif_reg;

	sfg_sfg_force_bg_at_dif_reg.regValue = IoReg_Read32(SFG_SFG_FORCE_BG_AT_DIF_reg);

	if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_R){
		sfg_sfg_force_bg_at_dif_reg.bg_dif_red = 0x3ff;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_grn = 0;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_blu = 0;
	}
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_G){
		sfg_sfg_force_bg_at_dif_reg.bg_dif_red = 0;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_grn = 0x3ff;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_blu = 0;
	}
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_B){
		sfg_sfg_force_bg_at_dif_reg.bg_dif_red = 0;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_grn = 0;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_blu = 0x3ff;
	}
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_WHITE){
		sfg_sfg_force_bg_at_dif_reg.bg_dif_red = 0x3ff;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_grn = 0x3ff;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_blu = 0x3ff;
	}
	else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_BLACK){
		sfg_sfg_force_bg_at_dif_reg.bg_dif_red = 0;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_grn = 0;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_blu = 0;
	}
	else{
		sfg_sfg_force_bg_at_dif_reg.bg_dif_red = 0;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_grn = 0;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_blu = 0;
	}
	if(bOnOff){
		sfg_sfg_force_bg_at_dif_reg.en_force_bg_dif = 1;
	}else{
		sfg_sfg_force_bg_at_dif_reg.en_force_bg_dif = 0;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_red = 0;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_grn = 0;
		sfg_sfg_force_bg_at_dif_reg.bg_dif_blu = 0;
	}
	IoReg_Write32(SFG_SFG_FORCE_BG_AT_DIF_reg, sfg_sfg_force_bg_at_dif_reg.regValue);

}

void vbe_disp_uzu_inner_ptg(UINT8 bOnOff, VBE_DISP_INNER_PTG_TYPE_T type)
{
	scaleup_d_uzu_globle_ctrl_RBUS scaleup_d_uzu_globle_ctrl_reg;
	scaleup_dm_uzu_input_size_RBUS scaleup_dm_uzu_input_size_reg;
	scaleup_d_uzu_patgen_frame_toggle_RBUS scaleup_d_uzu_patgen_frame_toggle_reg;
	unsigned int colorBar_width=0;

	scaleup_d_uzu_globle_ctrl_reg.regValue = IoReg_Read32(SCALEUP_D_UZU_Globle_Ctrl_reg);
	scaleup_dm_uzu_input_size_reg.regValue = IoReg_Read32(SCALEUP_DM_UZU_Input_Size_reg);
	scaleup_d_uzu_patgen_frame_toggle_reg.regValue = IoReg_Read32(SCALEUP_D_UZU_PATGEN_FRAME_TOGGLE_reg);

	colorBar_width = (scaleup_dm_uzu_input_size_reg.hor_input_size/8)-1;
	rtd_pr_vbe_notice("colorBar_width=%x\n", colorBar_width);

	if(type == VBE_DISP_INNER_PTG_TYPE_RANDOM_GEN){
		scaleup_d_uzu_globle_ctrl_reg.patgen_mode = 0;
	}else if(type == VBE_DISP_INNER_PTG_TYPE_COLOR_BAR){
		scaleup_d_uzu_globle_ctrl_reg.patgen_mode = 3; // color bar
		scaleup_d_uzu_patgen_frame_toggle_reg.patgen_width = colorBar_width;
	}else{
		scaleup_d_uzu_globle_ctrl_reg.patgen_mode = 0;
	}

	if(bOnOff){
		scaleup_d_uzu_globle_ctrl_reg.patgen_sel = 1;
	}else{
		scaleup_d_uzu_globle_ctrl_reg.patgen_sel = 0;
	}
	IoReg_Write32(SCALEUP_D_UZU_Globle_Ctrl_reg, scaleup_d_uzu_globle_ctrl_reg.regValue);
	IoReg_Write32(SCALEUP_D_UZU_PATGEN_FRAME_TOGGLE_reg, scaleup_d_uzu_patgen_frame_toggle_reg.regValue);
}

void vbe_disp_vgip_inner_ptg(UINT8 bOnOff, VBE_DISP_INNER_PTG_TYPE_T type)
{
	vgip_vgip_chn1_ctrl_RBUS vgip_vgip_chn1_ctrl_reg;
	vgip_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
	if(bOnOff)
		vgip_vgip_chn1_ctrl_reg.ch1_random_en = 1;
	else
		vgip_vgip_chn1_ctrl_reg.ch1_random_en = 0;
	IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);

}

void vbe_disp_set_inner_ptg(UINT8 bOnOff, VBE_DISP_INNER_PTG_BLOCK_T block, VBE_DISP_INNER_PTG_TYPE_T type)
{
	if(block == VBE_DISP_INNER_PTG_BLOCK_VGIP){
		vbe_disp_vgip_inner_ptg(bOnOff, type);
	}else if(block == VBE_DISP_INNER_PTG_BLOCK_MTG){
		vbe_disp_mtg_inner_ptg(bOnOff, type);
	}else if(block == VBE_DISP_INNER_PTG_BLOCK_UZU){
		vbe_disp_uzu_inner_ptg(bOnOff, type);
	}else if(block == VBE_DISP_INNER_PTG_BLOCK_MEMC_INPUTMUX){
		vbe_disp_memc_inputmux_inner_ptg(bOnOff, type);
	}else if(block == VBE_DISP_INNER_PTG_BLOCK_MEMC_OUTPUTMUX){
		vbe_disp_memc_outputmux_inner_ptg(bOnOff, type);
	}else if(block == VBE_DISP_INNER_PTG_BLOCK_SFG){
		vbe_disp_sfg_inner_ptg(bOnOff, type);
	}else{
		rtd_pr_vbe_err("[%s] Error block :%d \n", __FUNCTION__, block);
	}
}

#define VCOM_SIZE 96
#define VCOM_PATTENT_LINE_SIZE 24
void vbe_disp_vcomPatternDraw( UINT16 *vcomPattern,UINT16 nSize )
{
	UINT32 vComPatLineSetting[4];
	UINT16 grayLevel=0;
	int i=0;
	sfg_sfg_patgen_8_RBUS sfg_sfg_patgen_8_reg;
	sfg_sfg_patgen_9_RBUS sfg_sfg_patgen_9_reg;
	sfg_sfg_patgen_10_RBUS sfg_sfg_patgen_10_reg;
	sfg_sfg_patgen_11_RBUS sfg_sfg_patgen_11_reg;
	sfg_sfg_patgen_12_RBUS sfg_sfg_patgen_12_reg;

	memset(vComPatLineSetting, 0, sizeof(UINT32)*4);

	if(nSize != VCOM_SIZE){
		rtd_pr_vbe_emerg("[Warning] nSize %d define is not %d\n", nSize, VCOM_SIZE);
	}

	//convert to our SFG setting method.
	// 8x3x4line
	for(i=0; i<nSize; i++){
		//rtd_pr_vbe_emerg("vcom[%d] = %x\n", i, p_vcomPattern[i]);
		if(vcomPattern[i] != 0){
			vComPatLineSetting[(i/VCOM_PATTENT_LINE_SIZE)] |= (1<<(i%VCOM_PATTENT_LINE_SIZE));
			//vComPatLineSetting[(i/VCOM_PATTENT_LINE_SIZE)] |= (1<<(VCOM_PATTENT_LINE_SIZE-1-(i%VCOM_PATTENT_LINE_SIZE)));

			if(grayLevel==0)
				grayLevel = vcomPattern[i];
			else{
				//check only one setting value of vComPattern, else print warning msg
				if(grayLevel != vcomPattern[i])
					rtd_pr_vbe_emerg("[error] Vcom Pattern has different gary value (%d.%d)\n", grayLevel, vcomPattern[i]);
			}
		}
	}
	rtd_pr_vbe_notice("Vcom Pattern Gray Value = %d \n", grayLevel);
	rtd_pr_vbe_notice("vComLinePattern[0] = %x\n", vComPatLineSetting[0]);
	rtd_pr_vbe_notice("vComLinePattern[1] = %x\n", vComPatLineSetting[1]);
	rtd_pr_vbe_notice("vComLinePattern[2] = %x\n", vComPatLineSetting[2]);
	rtd_pr_vbe_notice("vComLinePattern[3] = %x\n", vComPatLineSetting[3]);

	//set SFG vcom pattern here.
	sfg_sfg_patgen_8_reg.regValue = IoReg_Read32(SFG_SFG_patgen_8_reg);
	sfg_sfg_patgen_8_reg.sfg_pat3_subpix_0 = 0;
	sfg_sfg_patgen_8_reg.sfg_pat3_subpix_1 = grayLevel;
	IoReg_Write32(SFG_SFG_patgen_8_reg, sfg_sfg_patgen_8_reg.regValue);

	//Vcom Line 1
	sfg_sfg_patgen_9_reg.regValue = IoReg_Read32(SFG_SFG_patgen_9_reg);
	sfg_sfg_patgen_9_reg.sfg_pat3_line_0 = vComPatLineSetting[0];
	IoReg_Write32(SFG_SFG_patgen_9_reg, sfg_sfg_patgen_9_reg.regValue);
	//Vcom Line 2
	sfg_sfg_patgen_10_reg.regValue = IoReg_Read32(SFG_SFG_patgen_10_reg);
	sfg_sfg_patgen_10_reg.sfg_pat3_line_1= vComPatLineSetting[1];
	IoReg_Write32(SFG_SFG_patgen_10_reg, sfg_sfg_patgen_10_reg.regValue);
	//Vcom Line 3
	sfg_sfg_patgen_11_reg.regValue = IoReg_Read32(SFG_SFG_patgen_11_reg);
	sfg_sfg_patgen_11_reg.sfg_pat3_line_2 = vComPatLineSetting[2];
	IoReg_Write32(SFG_SFG_patgen_11_reg, sfg_sfg_patgen_11_reg.regValue);
	//Vcom Line 4
	sfg_sfg_patgen_12_reg.regValue = IoReg_Read32(SFG_SFG_patgen_12_reg);
	sfg_sfg_patgen_12_reg.sfg_pat3_line_3 = vComPatLineSetting[3];
	IoReg_Write32(SFG_SFG_patgen_12_reg, sfg_sfg_patgen_12_reg.regValue);

}
void HAL_VBE_DISP_VCOMPatternDraw(UINT16 *vcomPattern,UINT16 nSize )
{
	down(&VBE_Semaphore);

	rtd_pr_vbe_notice("HAL_VBE_DISP_VCOMPatternDraw size:%d \n", nSize);

	vbe_disp_vcomPatternDraw(vcomPattern,nSize);

	up(&VBE_Semaphore);

}

void HAL_VBE_DISP_VCOMPatternCtrl(KADP_DISP_PANEL_VCOM_PAT_CTRL_T nCtrl)
{
	sfg_sfg_patgen_ctrl_RBUS sfg_sfg_patgen_ctrl_reg;

	down(&VBE_Semaphore);

	sfg_sfg_patgen_ctrl_reg.regValue = IoReg_Read32(SFG_SFG_patgen_ctrl_reg);
	sfg_sfg_patgen_ctrl_reg.sfg_pat_mode = 3;
	IoReg_Write32(SFG_SFG_patgen_ctrl_reg, sfg_sfg_patgen_ctrl_reg.regValue);

	rtd_pr_vbe_notice("HAL_VBE_DISP_VCOMPatternCtrl\n");

	if(nCtrl == KADP_DISP_PANEL_VCOM_PAT_CTRL_ON){
		rtd_pr_vbe_notice("KADP_DISP_PANEL_VCOM_PAT_CTRL_ON\n");
		sfg_sfg_patgen_ctrl_reg.sfg_pat_en = 1;
	}else if(nCtrl == KADP_DISP_PANEL_VCOM_PAT_CTRL_OFF){
		rtd_pr_vbe_notice("KADP_DISP_PANEL_VCOM_PAT_CTRL_OFF\n");
		sfg_sfg_patgen_ctrl_reg.sfg_pat_en = 0;
	}
	IoReg_Write32(SFG_SFG_patgen_ctrl_reg, sfg_sfg_patgen_ctrl_reg.regValue);

	up(&VBE_Semaphore);
}
void HAL_VBE_DISP_SetBOEMode(UINT8 *pFwData, UINT32 size)
{
	down(&VBE_Semaphore);

	rtd_pr_vbe_notice("HAL_VBE_DISP_SetBOEMode, size:%d \n", size);

	down(&VBE_RGBW_Resume_Semaphore);
	//vbe_disp_set_BoeMode(pFwData, size);
	fwif_color_set_SiW_ModeChange_TV006(pFwData, size);
	up(&VBE_RGBW_Resume_Semaphore);

	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_GetBOEVersion(KADP_DISP_BOE_VERSION_T *pstRomVer, KADP_DISP_BOE_VERSION_T *pstFwVer)
{
	down(&VBE_Semaphore);

	rtd_pr_vbe_notice("HAL_VBE_DISP_GetBOEVersion\n");

	down(&VBE_RGBW_Resume_Semaphore);
	vbe_disp_get_BoeVersion(BOE_ROM_VERSION, pstRomVer);
	vbe_disp_get_BoeVersion(BOE_FW_VERSION, pstFwVer);
	up(&VBE_RGBW_Resume_Semaphore);

	up(&VBE_Semaphore);
}

#ifdef CONFIG_SCALER_ENABLE_V4L2
void HAL_VBE_DISP_SetMLEMode(enum v4l2_ext_vbe_mplus_mode index)
{
	enum v4l2_ext_vbe_mplus_mode MLE_Idx;
	down(&VBE_Semaphore);

	MLE_Idx = 0;
	rtd_pr_vbe_notice("HAL_VBE_DISP_SetMLEMode : %d \n", index);

	memcpy((void*)&MLE_Idx, (void*)&index, sizeof(enum v4l2_ext_vbe_mplus_mode));

#ifdef CONFIG_SCALER_ENABLE_V4L2
	fwif_color_set_Mplus_ModeChange_TV006(MLE_Idx);
#endif

	up(&VBE_Semaphore);
}
#endif

void HAL_VBE_DISP_SetInnerPattern(UINT8 bOnOff, VBE_DISP_INNER_PTG_BLOCK_T block, VBE_DISP_INNER_PTG_TYPE_T type)
{
	down(&VBE_Semaphore);

	rtd_pr_vbe_notice("HAL_VBE_DISP_SetInnerPattern\n");
	rtd_pr_vbe_notice("bOnOff=%d, block%d, type=%d\n", bOnOff, block, type);

	vbe_disp_set_inner_ptg(bOnOff, block, type);

	up(&VBE_Semaphore);

}

void HAL_VBE_DISP_SetPanelTiMode(UINT8 tiMode)
{
#if 0 // not use in Merlin8
	pif_lvds_ctrl3_RBUS  lvds_ctrl3_reg;
    down(&VBE_Semaphore);

    if (Get_DISPLAY_PANEL_TYPE() == P_LVDS_2K1K)
    {
        lvds_ctrl3_reg.regValue = IoReg_Read32(PIF_LVDS_CTRL3_reg);
        lvds_ctrl3_reg.lvds_map = tiMode;
        IoReg_Write32(PIF_LVDS_CTRL3_reg, lvds_ctrl3_reg.regValue);
    }

    up(&VBE_Semaphore);
#endif
}

void HAL_VBE_DISP_GetPanelTiMode(UINT8* tiMode)
{
#if 0 // not use in Merlin8
	pif_lvds_ctrl3_RBUS  lvds_ctrl3_reg;
    down(&VBE_Semaphore);

    lvds_ctrl3_reg.regValue = IoReg_Read32(PIF_LVDS_CTRL3_reg);
    *tiMode = lvds_ctrl3_reg.lvds_map;

    up(&VBE_Semaphore);
#endif
}

void HAL_VBE_DISP_SetPanelBitMode(UINT8 bitMode)
{
    sfg_sfg_zz_shift_dbg_ctrl_RBUS sfg_zz_shift_dbg_ctrl_reg;
    down(&VBE_Semaphore);

    sfg_zz_shift_dbg_ctrl_reg.regValue = IoReg_Read32(SFG_SFG_ZZ_shift_dbg_CTRL_reg);
    sfg_zz_shift_dbg_ctrl_reg.dout_mode = bitMode;
    IoReg_Write32(SFG_SFG_ZZ_shift_dbg_CTRL_reg, sfg_zz_shift_dbg_ctrl_reg.regValue);

    up(&VBE_Semaphore);
}

void HAL_VBE_DISP_GetPanelBitMode(UINT8* bitMode)
{
    sfg_sfg_zz_shift_dbg_ctrl_RBUS sfg_zz_shift_dbg_ctrl_reg;
    down(&VBE_Semaphore);

    sfg_zz_shift_dbg_ctrl_reg.regValue = IoReg_Read32(SFG_SFG_ZZ_shift_dbg_CTRL_reg);
    *bitMode = sfg_zz_shift_dbg_ctrl_reg.dout_mode;

    up(&VBE_Semaphore);
}

void HAL_VBE_DISP_SetPanelSFGDualMode(UINT8 segNum)
{
    sfg_sfg_ctrl_0_RBUS sfg_ctrl_0_reg;
    sfg_sfg_ctrl_1_RBUS sfg_ctrl_1_reg;
    unsigned int segWidth = 0;
    int i = 0;
    down(&VBE_Semaphore);

    sfg_ctrl_0_reg.regValue = IoReg_Read32(SFG_SFG_CTRL_0_reg);
    switch (segNum)
    {
        case 1:
        case 2:
        case 4:
        {
            segWidth = (Get_DISP_DEN_END_HPOS() - Get_DISP_DEN_STA_HPOS()) / segNum;
            for (i=0; i<4; i++)
            {
                if (i < segNum/2)
                {
                    sfg_ctrl_1_reg.seg0_start = (i*2)*segWidth;
                    sfg_ctrl_1_reg.seg1_start = (i*2+1) * segWidth;
                    IoReg_Write32(SFG_SFG_CTRL_1_reg+i*4, sfg_ctrl_1_reg.regValue);
                }
                else
                {
                    IoReg_Write32(SFG_SFG_CTRL_1_reg+i*4, 0);
                }
            }
            sfg_ctrl_0_reg.seg_num = segNum-1;
            IoReg_Write32(SFG_SFG_CTRL_0_reg, sfg_ctrl_0_reg.regValue);
            break;
        }
        default:
            rtd_pr_vbe_err("segment param error...\n");
            break;
    }

    up(&VBE_Semaphore);
}

void HAL_VBE_DISP_GetPanelSFGDualMode(UINT8* segNum)
{
    sfg_sfg_ctrl_0_RBUS sfg_ctrl_0_reg;
    down(&VBE_Semaphore);

    sfg_ctrl_0_reg.regValue = IoReg_Read32(SFG_SFG_CTRL_0_reg);
    *segNum = sfg_ctrl_0_reg.seg_num + 1;

    up(&VBE_Semaphore);
}

extern void fwif_color_MplusSet(unsigned short *pRegisterSet, unsigned char panelMaker);
extern void fwif_color_MplusGet(unsigned short *pRegisterSet, unsigned char panelMaker);
extern void fwif_color_set_SetBOERGBWBypass_TV006(unsigned char BOE_EN);
extern void fwif_color_SetFrameGainLimit_TV006(unsigned short nFrameGainLimit);
extern void fwif_color_GetFrameGainLimit_TV006(unsigned short *nFrameGainLimit);
extern void fwif_color_SetPixelGainLimit_TV006(unsigned short nPixelGainLimit);
extern void fwif_color_GetPixelGainLimit_TV006(unsigned short *nPixelGainLimit);
extern void fwif_color_set_fcic_TV006(unsigned int *u32pTSCICTbl, unsigned int u32Tscicsize, unsigned char *u8pControlTbl, unsigned int u32Ctrlsize, unsigned char bCtrl);

void HAL_VBE_DISP_SetBOERGBWBypass(BOOLEAN bEnable)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_SetBOERGBWBypass : %d \n", bEnable);

	//if (Get_DISPLAY_PANEL_BOW_RGBW() == TRUE) /*protection for BOE*/
		fwif_color_set_SetBOERGBWBypass_TV006(bEnable);

	up(&VBE_Semaphore);
}


extern void fwif_color_set_output_gamma_4CHDGA_TV006 (UINT32 *pRedGammaTable, UINT32 * pGreenGammaTable, UINT32 * pBlueGammaTable, UINT32 * pWhiteGammaTable, UINT16 nTableSize);

void HAL_VBE_SetDGA4CH (UINT32 *pRedGammaTable, UINT32 *pGreenGammaTable, UINT32 *pBlueGammaTable, UINT32 *pWhiteGammaTable, UINT16 nTableSize)
{
	down(&VBE_Semaphore);

	rtd_pr_vbe_notice("HAL_VBE_SetDGA4CH\n");
	rtd_pr_vbe_notice("nTableSize = %d \n", nTableSize);
	fwif_color_set_output_gamma_4CHDGA_TV006(pRedGammaTable,pGreenGammaTable,pBlueGammaTable,pWhiteGammaTable,nTableSize);

	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_GetPanelSize(KADP_DISP_PANEL_SIZE_T *panel_size)
{
	down(&VBE_Semaphore);

	vbe_disp_get_disp_panel_size(panel_size);
	rtd_pr_vbe_notice("HAL_VBE_DISP_GetPanelSize : w = %d , h = %d \n", panel_size->DISP_WIDTH, panel_size->DISP_HEIGHT);

	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_MplusSet(UINT16 * pRegisterSet, UINT8 nPanelMaker)
{
	rtd_pr_vbe_notice("HAL_VBE_DISP_MplusSet \n");
	rtd_pr_vbe_notice("nPanelMaker = (%d) \n", nPanelMaker);

	down(&VBE_Semaphore);
	fwif_color_MplusSet(pRegisterSet, nPanelMaker);
	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_MplusGet(UINT16 * pRegisterGet, UINT8 nPanelMaker)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_MplusGet \n");
	rtd_pr_vbe_notice("nPanelMaker = (%d) \n", nPanelMaker);

	if(Get_DISPLAY_PANEL_MPLUS_RGBW() == 1) // LGD M+
		fwif_color_MplusGet(pRegisterGet, 0);
	if(Get_DISPLAY_PANEL_BOW_RGBW() == 1) // BOE SiW
		fwif_color_MplusGet(pRegisterGet, 1);

	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_SetFrameGainLimit(UINT16 nFrameGainLimit)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_SetFrameGainLimit : %d \n", nFrameGainLimit);

	fwif_color_SetFrameGainLimit_TV006(nFrameGainLimit);

	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_GetFrameGainLimit (UINT16 *nFrameGainLimit)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_GetFrameGainLimit : %ld \n", (unsigned long)nFrameGainLimit);

	fwif_color_GetFrameGainLimit_TV006(nFrameGainLimit);

	up(&VBE_Semaphore);
}


void HAL_VBE_DISP_SetPixelGainLimit (UINT16 nPixelGainLimit)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_SetPixelGainLimit : %d \n", nPixelGainLimit);

	fwif_color_SetPixelGainLimit_TV006(nPixelGainLimit);

	up(&VBE_Semaphore);
}


void HAL_VBE_DISP_GetPixelGainLimit (UINT16 *nPixelGainLimit)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_GetPixelGainLimit : %ld \n",(unsigned long) nPixelGainLimit);

	fwif_color_GetPixelGainLimit_TV006(nPixelGainLimit);

	up(&VBE_Semaphore);
}

void HAL_VBE_TSCIC_Load(UINT32 *u32pTSCICTbl, UINT32 u32Tscicsize, UINT8 *u8pControlTbl, UINT32 u32Ctrlsize)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("u32Tscicsize = %d,  u32Ctrlsize = %d\n", u32Tscicsize, u32Ctrlsize);
	fwif_color_set_fcic_TV006(u32pTSCICTbl, u32Tscicsize, u8pControlTbl, u32Ctrlsize, 1);
	up(&VBE_Semaphore);



}

void HAL_VBE_DISP_SetOLEDLSR (VBE_DISP_LSR_ADJUST_MODE_T nStep, UINT32 *LSRTbl)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_SetOLEDLSR \n");
	rtd_pr_vbe_notice("Value = (%d, %x) \n", nStep, *LSRTbl);
	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_SetOLEDLSR_Tunning (VBE_DISP_LSR_TUNNING_VAL_T *stLSRdata)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_SetOLEDLSR_Tunning \n");
	rtd_pr_vbe_notice("IIR = %d \n", stLSRdata->IIR);
	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_SetOLEDCPC_Tunning (VBE_DISP_CPC_TUNNING_VAL_T *stCPCdata)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_SetOLEDCPC_Tunning \n");
	rtd_pr_vbe_notice("STEP = (%d, %d) \n", stCPCdata->STEP[0], stCPCdata->STEP[1]);
	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_SetOLEDGSR (UINT32 *GSRTbl)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_SetOLEDGSR \n");
	rtd_pr_vbe_notice("GSRTbl = (%d) \n", *GSRTbl);
	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_GetOSDAlphaAPL(UINT32 *alphaTbl, UINT32 size)
{
	UINT32 dummyAlphaTbl = 0;

	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_GetOSDAlphaAPL \n");
	rtd_pr_vbe_notice("size = (%d) \n", size);
	alphaTbl = &dummyAlphaTbl;
	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_SetOSDRGBLevel(BOOLEAN bOnOff, UINT32 *lvlTbl, UINT32 size)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_SetOSDRGBLevel \n");
	rtd_pr_vbe_notice("size = (%d) \n", size);
	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_SetEPIDataScramble(BOOLEAN bOnOff)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_SetEPIDataScramble \n");
	rtd_pr_vbe_notice("bOnOff = (%d) \n", bOnOff);
	up(&VBE_Semaphore);
}

void HAL_VBE_DISP_SetAdvanced10bit(BOOLEAN bOnOff)
{
	down(&VBE_Semaphore);
	rtd_pr_vbe_notice("HAL_VBE_DISP_SetAdvanced10bit \n");
	rtd_pr_vbe_notice("bOnOff = (%d) \n", bOnOff);
	up(&VBE_Semaphore);
}

void Scaler_set_lvds_to_hdmi_parameter(KADP_DISP_PANEL_ATTRIBUTE_T * pPanelAttr,UINT8 framerate)
{
	unsigned int panel_width,panel_height;
	PANEL_CONFIG_PARAMETER parameter;
	panel_width=(pPanelAttr->panelResolution>>12)&0xffff;
	panel_height=pPanelAttr->panelResolution&0xfff;

	memset(&parameter,0,sizeof(PANEL_CONFIG_PARAMETER));

	parameter.iCONFIG_PANEL_TYPE=P_LVDS_TO_HDMI;

	parameter.iCONFIG_DISPLAY_PORT=pPanelAttr->eLVDSPixel;
	parameter.iCONFIG_DISPLAY_BITMAPPING_TABLE=(pPanelAttr->eLVDSSelect)?0:1;  //pPanelAttr.eLVDSSelect 0 is VESA,1 is JEIDA
	parameter.iCONFIG_DISPLAY_COLOR_BITS=(pPanelAttr->eColorDepth)?0:1;   //pPanelAttr.eColorDepth 0 is 8bit,1 is 10bit
	parameter.iCONFIG_DISPLAY_HORIZONTAL_SYNC_NORMAL=1;
	parameter.iCONFIG_DISPLAY_VERTICAL_SYNC_NORMAL=1;

	parameter.iCONFIG_DISP_ACT_STA_HPOS=0;
	parameter.iCONFIG_DISP_ACT_END_HPOS=panel_width;
	parameter.iCONFIG_DISP_ACT_STA_VPOS=0;
	parameter.iCONFIG_DISP_ACT_END_VPOS=panel_height;
	parameter.iCONFIG_DISP_HORIZONTAL_TOTAL=pPanelAttr->horizTypPeriod; //Dh total need as even

	parameter.iCONFIG_DISPLAY_REFRESH_RATE=framerate;
	parameter.iCONFIG_DISPLAY_CLOCK_MIN=126;
	parameter.iCONFIG_DISPLAY_CLOCK_MAX=154;
	parameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MIN=pPanelAttr->vertBackPorch50Hz;
	parameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MAX=pPanelAttr->vertTypPeriod50Hz;
	parameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MIN=pPanelAttr->vertBackPorch60Hz;
	parameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MAX=pPanelAttr->vertTypPeriod60Hz;

	parameter.iCONFIG_DISP_VERTICAL_TOTAL=pPanelAttr->vertTypPeriod60Hz;
	parameter.iCONFIG_DISPLAY_CLOCK_TYPICAL=framerate*pPanelAttr->horizTypPeriod*pPanelAttr->vertTypPeriod60Hz;
	parameter.iCONFIG_DISP_DEN_STA_VPOS=pPanelAttr->vertBackPorch60Hz;
	parameter.iCONFIG_DISP_DEN_END_VPOS=pPanelAttr->vertBackPorch60Hz+panel_height;
	parameter.iCONFIG_DISP_VSYNC_LENGTH=pPanelAttr->vertSyncWidth60Hz;
	parameter.iCONFIG_DISP_HSYNC_WIDTH =pPanelAttr->horizSyncWidth60Hz;
	parameter.iCONFIG_DISP_DEN_STA_HPOS=pPanelAttr->horizBackPorch60Hz;
	parameter.iCONFIG_DISP_DEN_END_HPOS=pPanelAttr->horizBackPorch60Hz+panel_width;

	parameter.iCONFIG_DISP_HSYNC_LASTLINE=pPanelAttr->horizTypPeriod-1;

	Panel_InitParameter(&parameter);
}
#ifdef CONFIG_PM
//VBE_DISP_SUSPEND_RESUME_T vbe_disp_suspend_resume;

void panel_epi_resume_flow(void)
{
#if 0   //Fixed me later by benwang
	rtd_pr_vbe_notice("###[%s][Line:%d]###\n", __FUNCTION__, __LINE__);
	msleep(1);
	rtd_setbits(VBY1_VIDEO_FORMAT_reg, _BIT23);	// 0xb802D700 lane 0~7 disable, lane 8~15 use EPI 8~15, lane 16~23 use EPI 16~23,
	rtd_outl(SFG_SFG_DUMMY_reg, 0x00000000);	// 0xb802D9B8 EPI Clock Disable, Data inverse disable,
	rtd_outl(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_27_reg, 0x00000000);	// 0xB8000E90 lane power disable
	rtd_setbits(EPI_EPI_CLOCK_CTRL0_reg, _BIT28);	// 0xb8026064, EPI reset enable, CST/C3/DST packet disable, EPI lock HW mode,
	msleep(1);
	rtd_setbits(SFG_SFG_DUMMY_reg, _BIT30);	//0xb802D9B8  EPI Clock Enable
	msleep(1);
	rtd_clearbits(EPI_EPI_CLOCK_CTRL0_reg, _BIT28);	// 0xb8026064, EPI reset disable
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_27_reg, vbe_disp_suspend_resume.PINMUX_LVDSPHY_VBY1EPI_90_S_R); // 0xB8000E90 enable lane power
	msleep(1);
	IoReg_Write32(PLL27X_REG_PLL_SSC4_reg, vbe_disp_suspend_resume.CRT_PLL_SSC4_S_R);
#endif
}

void vbe_disp_panel_mplus_resume_flow(void)
{
#ifdef _VIP_Mer7_Compile_Error__
	sys_reg_sys_srst3_RBUS sys_reg_sys_srst3_reg;
	sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_reg;
//	ppoverlay_display_timing_ctrl1_RBUS ppoverlay_display_timing_ctrl1_reg;
	ppoverlay_display_timing_ctrl2_RBUS ppoverlay_display_timing_ctrl2_reg;
	lg_m_plus_m_plus_ctl_RBUS lg_m_plus_m_plus_ctl_reg;

	//sys_reg_sys_srst3_reg.regValue = IoReg_Read32(SYS_REG_SYS_SRST3_reg);
	sys_reg_sys_srst3_reg.regValue = 0;
//	sys_reg_sys_srst3_reg.rstn_disp_lg_mplus = 1;
	sys_reg_sys_srst3_reg.write_data = 1;
	IoReg_Write32(SYS_REG_SYS_SRST3_reg,sys_reg_sys_srst3_reg.regValue);

	//sys_reg_sys_clken3_reg.regValue = IoReg_Read32(SYS_REG_SYS_CLKEN3_reg);
	sys_reg_sys_clken3_reg.regValue = 0;
//	sys_reg_sys_clken3_reg.clken_disp_lg_mplus = 1;
	sys_reg_sys_clken3_reg.write_data = 1;
	IoReg_Write32(SYS_REG_SYS_CLKEN3_reg,sys_reg_sys_clken3_reg.regValue);

	rtd_pr_vbe_notice("###[%s][Line:%d]###\n", __FUNCTION__, __LINE__);

	//MPLUS parameter
	rtd_outl(LG_M_PLUS_M_plus_reg000_reg, 0xffffffff);
	rtd_outl(LG_M_PLUS_M_plus_reg004_reg, 0xffffffff);
	rtd_outl(LG_M_PLUS_M_plus_reg008_reg, 0xff100010);
	rtd_outl(LG_M_PLUS_M_plus_reg00c_reg, 0x00100010);
	rtd_outl(LG_M_PLUS_M_plus_reg010_reg, 0x00100010);
	rtd_outl(LG_M_PLUS_M_plus_reg014_reg, 0x00100010);
	rtd_outl(LG_M_PLUS_M_plus_reg018_reg, 0x00000400);
	rtd_outl(LG_M_PLUS_M_plus_reg01c_reg, 0x00040000);
	rtd_outl(LG_M_PLUS_M_plus_reg020_reg, 0x04000004);
	rtd_outl(LG_M_PLUS_M_plus_reg024_reg, 0x00000400);
	rtd_outl(LG_M_PLUS_M_plus_reg028_reg, 0x00040000);
	rtd_outl(LG_M_PLUS_M_plus_reg02c_reg, 0x04000004);
	rtd_outl(LG_M_PLUS_M_plus_reg030_reg, 0x00000400);
	rtd_outl(LG_M_PLUS_M_plus_reg034_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg038_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg03c_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg040_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg044_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg048_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg04c_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg050_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg054_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg058_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg05c_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg060_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg064_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg068_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg06c_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg070_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg074_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg078_reg, 0x007e9000);
	rtd_outl(LG_M_PLUS_M_plus_reg07c_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg080_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg084_reg, 0x00200020);
	rtd_outl(LG_M_PLUS_M_plus_reg088_reg, 0x00200020);
	rtd_outl(LG_M_PLUS_M_plus_reg08c_reg, 0x00200020);
	rtd_outl(LG_M_PLUS_M_plus_reg090_reg, 0x00200020);
	rtd_outl(LG_M_PLUS_M_plus_reg094_reg, 0x00300010);
	rtd_outl(LG_M_PLUS_M_plus_reg098_reg, 0x10101010);
	rtd_outl(LG_M_PLUS_M_plus_reg09c_reg, 0x10101010);
	rtd_outl(LG_M_PLUS_M_plus_reg0a0_reg, 0x01000100);
	rtd_outl(LG_M_PLUS_M_plus_reg0a4_reg, 0x01000100);
	rtd_outl(LG_M_PLUS_M_plus_reg0a8_reg, 0x01000100);
	rtd_outl(LG_M_PLUS_M_plus_reg0ac_reg, 0x01000100);
	rtd_outl(LG_M_PLUS_M_plus_reg0b0_reg, 0x01a001a0);
	rtd_outl(LG_M_PLUS_M_plus_reg0b4_reg, 0x01a001a0);
	rtd_outl(LG_M_PLUS_M_plus_reg0b8_reg, 0x01a001a0);
	rtd_outl(LG_M_PLUS_M_plus_reg0bc_reg, 0x01a001a0);
	rtd_outl(LG_M_PLUS_M_plus_reg0c0_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg0c4_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg0c8_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg0cc_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg0d0_reg, 0x10101010);
	rtd_outl(LG_M_PLUS_M_plus_reg0d4_reg, 0x10101010);
	rtd_outl(LG_M_PLUS_M_plus_reg0d8_reg, 0x20001555);
	rtd_outl(LG_M_PLUS_M_plus_reg0dc_reg, 0x15551555);
	rtd_outl(LG_M_PLUS_M_plus_reg0e0_reg, 0x15551555);
	rtd_outl(LG_M_PLUS_M_plus_reg0e4_reg, 0x15551555);
	rtd_outl(LG_M_PLUS_M_plus_reg0e8_reg, 0x15551555);
	rtd_outl(LG_M_PLUS_M_plus_reg0ec_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg0f0_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg0f4_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg0f8_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg0fc_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg100_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg104_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg108_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg10c_reg, 0x1000ffff);
	rtd_outl(LG_M_PLUS_M_plus_reg110_reg, 0xffffffff);
	rtd_outl(LG_M_PLUS_M_plus_reg114_reg, 0xffffff01);
	rtd_outl(LG_M_PLUS_M_plus_reg118_reg, 0x00020003);
	rtd_outl(LG_M_PLUS_M_plus_reg11c_reg, 0x00040005);
	rtd_outl(LG_M_PLUS_M_plus_reg120_reg, 0x00060007);
	rtd_outl(LG_M_PLUS_M_plus_reg124_reg, 0x00080009);
	rtd_outl(LG_M_PLUS_M_plus_reg128_reg, 0x000a000b);
	rtd_outl(LG_M_PLUS_M_plus_reg12c_reg, 0x000c000d);
	rtd_outl(LG_M_PLUS_M_plus_reg130_reg, 0x000e000f);
	rtd_outl(LG_M_PLUS_M_plus_reg134_reg, 0x00040404);
	rtd_outl(LG_M_PLUS_M_plus_reg138_reg, 0x04913000);
	rtd_outl(LG_M_PLUS_M_plus_reg13c_reg, 0x80004040);
	rtd_outl(LG_M_PLUS_M_plus_reg140_reg, 0x20000880);
	rtd_outl(LG_M_PLUS_M_plus_reg144_reg, 0x80208004);
	rtd_outl(LG_M_PLUS_M_plus_reg148_reg, 0x10204020);
	rtd_outl(LG_M_PLUS_M_plus_reg14c_reg, 0xff010110);
	rtd_outl(LG_M_PLUS_M_plus_reg150_reg, 0x20304050);
	rtd_outl(LG_M_PLUS_M_plus_reg154_reg, 0x60708090);
	rtd_outl(LG_M_PLUS_M_plus_reg158_reg, 0xa0b0c0d0);
	rtd_outl(LG_M_PLUS_M_plus_reg15c_reg, 0xe0f01000);
	rtd_outl(LG_M_PLUS_M_plus_reg160_reg, 0x0aab071c);
	rtd_outl(LG_M_PLUS_M_plus_reg164_reg, 0x04be0329);
	rtd_outl(LG_M_PLUS_M_plus_reg168_reg, 0x021b0168);
	rtd_outl(LG_M_PLUS_M_plus_reg16c_reg, 0x00f000a0);
	rtd_outl(LG_M_PLUS_M_plus_reg170_reg, 0x006b0047);
	rtd_outl(LG_M_PLUS_M_plus_reg174_reg, 0x002f0020);
	rtd_outl(LG_M_PLUS_M_plus_reg178_reg, 0x0015000e);
	rtd_outl(LG_M_PLUS_M_plus_reg17c_reg, 0x00090006);
	rtd_outl(LG_M_PLUS_M_plus_reg180_reg, 0x00040003);
	rtd_outl(LG_M_PLUS_M_plus_reg184_reg, 0x00020001);
	rtd_outl(LG_M_PLUS_M_plus_reg188_reg, 0x00010001);
	rtd_outl(LG_M_PLUS_M_plus_reg18c_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg190_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg194_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg198_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg19c_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1a0_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1a4_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1a8_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1ac_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1b0_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1b4_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1b8_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1bc_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1c0_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1c4_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1c8_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1cc_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1d0_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1d4_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1d8_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1dc_reg, 0x00000000);
	rtd_outl(LG_M_PLUS_M_plus_reg1e0_reg, 0x22661489);
	rtd_outl(LG_M_PLUS_M_plus_reg1e4_reg, 0x0fe90e5f);
	rtd_outl(LG_M_PLUS_M_plus_reg1e8_reg, 0x0ddb0db0);
	rtd_outl(LG_M_PLUS_M_plus_reg1ec_reg, 0x0da10d9c);
	rtd_outl(LG_M_PLUS_M_plus_reg1f0_reg, 0x0d9a0d9a);
	rtd_outl(LG_M_PLUS_M_plus_reg1f4_reg, 0x0d9a0d9a);
	rtd_outl(LG_M_PLUS_M_plus_reg1f8_reg, 0x0d9c0da1);
	rtd_outl(LG_M_PLUS_M_plus_reg1fc_reg, 0x0db00ddb);
	rtd_outl(LG_M_PLUS_M_plus_reg200_reg, 0x0e5f0fe9);
	rtd_outl(LG_M_PLUS_M_plus_reg204_reg, 0x14892266);
	rtd_outl(LG_M_PLUS_M_plus_reg208_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg20c_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg210_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg214_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg218_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg21c_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg220_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg224_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg228_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg22c_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg230_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg234_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg238_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg23c_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg240_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg244_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg248_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg24c_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg250_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg254_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg258_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg25c_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg260_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg264_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg268_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg26c_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg270_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg274_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg278_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg27c_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg280_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg284_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg288_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg28c_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg290_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg294_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg298_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg29c_reg, 0x22662266);
	rtd_outl(LG_M_PLUS_M_plus_reg2a0_reg, 0x12660400);
	rtd_outl(LG_M_PLUS_M_plus_reg2a4_reg, 0x04000400);
	rtd_outl(LG_M_PLUS_M_plus_reg2a8_reg, 0x04000400);
	rtd_outl(LG_M_PLUS_M_plus_reg2ac_reg, 0x04000400);
	rtd_outl(LG_M_PLUS_M_plus_reg2b0_reg, 0x0400ffff);
	rtd_outl(LG_M_PLUS_M_plus_reg2b4_reg, 0xffffffff);
	rtd_outl(LG_M_PLUS_M_plus_reg2b8_reg, 0xffffffff);
	rtd_outl(LG_M_PLUS_M_plus_reg2bc_reg, 0xffffffff);
	rtd_outl(LG_M_PLUS_M_plus_reg2c0_reg, 0xffff0400);
	rtd_outl(LG_M_PLUS_M_plus_reg2c4_reg, 0x04000400);
	rtd_outl(LG_M_PLUS_M_plus_reg2c8_reg, 0x04000400);
	rtd_outl(LG_M_PLUS_M_plus_reg2cc_reg, 0x04000400);
	rtd_outl(LG_M_PLUS_M_plus_reg2d0_reg, 0x04001000);
	rtd_outl(LG_M_PLUS_M_plus_reg2d4_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg2d8_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg2dc_reg, 0x10001000);
	rtd_outl(LG_M_PLUS_M_plus_reg2e0_reg, 0x10004040);
	rtd_outl(LG_M_PLUS_M_plus_reg2e4_reg, 0x40404040);
	rtd_outl(LG_M_PLUS_M_plus_reg2e8_reg, 0x40400200);
	rtd_outl(LG_M_PLUS_M_plus_reg2ec_reg, 0x02000200);
	rtd_outl(LG_M_PLUS_M_plus_reg2f0_reg, 0x02000200);
	rtd_outl(LG_M_PLUS_M_plus_reg2f4_reg, 0x02000200);
	rtd_outl(LG_M_PLUS_M_plus_reg2f8_reg, 0x02000800);
	rtd_outl(LG_M_PLUS_M_plus_reg2fc_reg, 0x08000800);
	rtd_outl(LG_M_PLUS_M_plus_reg300_reg, 0x08000800);
	rtd_outl(LG_M_PLUS_M_plus_reg304_reg, 0x08000800);
	rtd_outl(LG_M_PLUS_M_plus_reg308_reg, 0x08000800);
	rtd_outl(LG_M_PLUS_M_plus_reg30c_reg, 0x04000400);
	rtd_outl(LG_M_PLUS_M_plus_reg310_reg, 0x04000400);
	rtd_outl(LG_M_PLUS_M_plus_reg314_reg, 0x04000400);
	rtd_outl(LG_M_PLUS_M_plus_reg318_reg, 0x04000400);
	rtd_outl(LG_M_PLUS_M_plus_reg31c_reg, 0x04004000);
	rtd_outl(LG_M_PLUS_M_plus_reg320_reg, 0x20000800);
	rtd_outl(LG_M_PLUS_M_plus_reg324_reg, 0x10300030);
	rtd_outl(LG_M_PLUS_M_plus_reg328_reg, 0x00300030);
	rtd_outl(LG_M_PLUS_M_plus_reg32c_reg, 0x00300030);
	rtd_outl(LG_M_PLUS_M_plus_reg330_reg, 0x00300030);
	rtd_outl(LG_M_PLUS_M_plus_reg334_reg, 0x003000ff);
	rtd_outl(LG_M_PLUS_M_plus_reg338_reg, 0xffffffff);
	rtd_outl(LG_M_PLUS_M_plus_reg33c_reg, 0xffffffff);
	rtd_outl(LG_M_PLUS_M_plus_reg340_reg, 0x08180808);
	rtd_outl(LG_M_PLUS_M_plus_reg344_reg, 0x18080818);
	rtd_outl(LG_M_PLUS_M_plus_reg348_reg, 0x08081808);
	rtd_outl(LG_M_PLUS_M_plus_reg34c_reg, 0x08180808);
	rtd_outl(LG_M_PLUS_M_plus_reg350_reg, 0x18080818);
	rtd_outl(LG_M_PLUS_M_plus_reg354_reg, 0x08081808);
	rtd_outl(LG_M_PLUS_M_plus_reg358_reg, 0x00600060);
	rtd_outl(LG_M_PLUS_M_plus_reg35c_reg, 0x00600060);
	rtd_outl(LG_M_PLUS_M_plus_reg360_reg, 0x00600060);
	rtd_outl(LG_M_PLUS_M_plus_reg364_reg, 0x00600060);
	rtd_outl(LG_M_PLUS_M_plus_reg368_reg, 0x10101010);
	rtd_outl(LG_M_PLUS_M_plus_reg36c_reg, 0x10101010);
	rtd_outl(LG_M_PLUS_M_plus_reg370_reg, 0x16000200);
	rtd_outl(LG_M_PLUS_M_plus_reg374_reg, 0x02000600);
	rtd_outl(LG_M_PLUS_M_plus_reg378_reg, 0x06000a00);
	rtd_outl(LG_M_PLUS_M_plus_reg37c_reg, 0x0a000e00);
	rtd_outl(LG_M_PLUS_M_plus_reg380_reg, 0x0e001200);
	rtd_outl(LG_M_PLUS_M_plus_reg384_reg, 0x12001600);
	rtd_outl(LG_M_PLUS_M_plus_reg388_reg, 0x08005460);
	rtd_outl(LG_M_PLUS_M_plus_reg38c_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg390_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg394_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg398_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg39c_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3a0_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3a4_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3a8_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3ac_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3b0_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3b4_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3b8_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3bc_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3c0_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3c4_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3c8_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3cc_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3d0_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3d4_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3d8_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3dc_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3e0_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3e4_reg, 0x54605460);
	rtd_outl(LG_M_PLUS_M_plus_reg3e8_reg, 0x54600666);
	rtd_outl(LG_M_PLUS_M_plus_reg3ec_reg, 0x06660666);
	rtd_outl(LG_M_PLUS_M_plus_reg3f0_reg, 0x06660666);
	rtd_outl(LG_M_PLUS_M_plus_reg3f4_reg, 0x06660666);
	rtd_outl(LG_M_PLUS_M_plus_reg3f8_reg, 0x06660606);
	rtd_outl(LG_M_PLUS_M_plus_reg3fc_reg, 0x06060606);
	rtd_outl(LG_M_PLUS_M_plus_reg400_reg, 0x0606147a);
	rtd_outl(LG_M_PLUS_M_plus_reg404_reg, 0xe1147ae1);
	rtd_outl(LG_M_PLUS_M_plus_reg408_reg, 0x147ae114);
	rtd_outl(LG_M_PLUS_M_plus_reg40c_reg, 0x7ae1147a);
	rtd_outl(LG_M_PLUS_M_plus_reg410_reg, 0xe1147ae1);
	rtd_outl(LG_M_PLUS_M_plus_reg414_reg, 0x147ae114);
	rtd_outl(LG_M_PLUS_M_plus_reg418_reg, 0x7ae1147a);
	rtd_outl(LG_M_PLUS_M_plus_reg41c_reg, 0xe1101010);
	rtd_outl(LG_M_PLUS_M_plus_reg420_reg, 0x10101010);
	rtd_outl(LG_M_PLUS_M_plus_reg424_reg, 0x10104040);
	rtd_outl(LG_M_PLUS_M_plus_reg428_reg, 0x40404040);
	rtd_outl(LG_M_PLUS_M_plus_reg42c_reg, 0x404040e0);
	rtd_outl(LG_M_PLUS_M_plus_reg430_reg, 0x6cff40a0);
	rtd_outl(LG_M_PLUS_M_plus_reg434_reg, 0xa040a0a0);
	rtd_outl(LG_M_PLUS_M_plus_reg438_reg, 0x46888088);
	rtd_outl(LG_M_PLUS_M_plus_reg43c_reg, 0x80888080);
	rtd_outl(LG_M_PLUS_M_plus_reg440_reg, 0x88808880);
	rtd_outl(LG_M_PLUS_M_plus_reg444_reg, 0x88888088);
	rtd_outl(LG_M_PLUS_M_plus_reg448_reg, 0x80888080);
	rtd_outl(LG_M_PLUS_M_plus_reg44c_reg, 0x88808880);
	rtd_outl(LG_M_PLUS_M_plus_reg450_reg, 0x88484000);
	rtd_outl(LG_M_PLUS_M_plus_reg454_reg, 0x48400048);
	rtd_outl(LG_M_PLUS_M_plus_reg458_reg, 0x40004840);
	rtd_outl(LG_M_PLUS_M_plus_reg45c_reg, 0x00000484);
	rtd_outl(LG_M_PLUS_M_plus_reg460_reg, 0x00048400);
	rtd_outl(LG_M_PLUS_M_plus_reg464_reg, 0x04840004);
	rtd_outl(LG_M_PLUS_M_plus_reg468_reg, 0x8460f504);
	rtd_outl(LG_M_PLUS_M_plus_reg46c_reg, 0x80402140);
	rtd_outl(LG_M_PLUS_M_plus_reg470_reg, 0x38302820);
	rtd_outl(LG_M_PLUS_M_plus_reg474_reg, 0x18101010);
	rtd_outl(LG_M_PLUS_M_plus_reg478_reg, 0x10101010);
	rtd_outl(LG_M_PLUS_M_plus_reg47c_reg, 0x10101010);


	lg_m_plus_m_plus_ctl_reg.regValue = IoReg_Read32(LG_M_PLUS_M_plus_ctl_reg);
	lg_m_plus_m_plus_ctl_reg.wpr_on = 1;
	IoReg_Write32(LG_M_PLUS_M_plus_ctl_reg,lg_m_plus_m_plus_ctl_reg.regValue);

	ppoverlay_display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	ppoverlay_display_timing_ctrl2_reg.mplus_rgbw_en = 1; //mplus enable, rgbw disable, BOE-rgbw disable
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg,ppoverlay_display_timing_ctrl2_reg.regValue);
#endif
}

extern UINT16 romcode_boe_rgbw_table_vivi_160815[16128];
void vbe_disp_panel_boe_resume_flow(void)
{
#ifdef _VIP_Mer7_Compile_Error__
    // === CRT reset off===
    sys_reg_sys_srst3_RBUS sys_reg_sys_srst3_REG;
    sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_REG;
    ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_REG;

    // flow:  rst_on -> clk_on
    //sys_reg_sys_srst3_REG.regValue = IoReg_Read32(SYS_REG_SYS_SRST3_reg);
    //sys_reg_sys_clken3_REG.regValue = IoReg_Read32(SYS_REG_SYS_CLKEN3_reg);
    sys_reg_sys_srst3_REG.regValue = 0;
//    sys_reg_sys_srst3_REG.rstn_siliconworks_rgbw = 1;
    sys_reg_sys_srst3_REG.write_data = 1;
    IoReg_Write32(SYS_REG_SYS_SRST3_reg, sys_reg_sys_srst3_REG.regValue);
    sys_reg_sys_clken3_REG.regValue = 0;
//    sys_reg_sys_clken3_REG.clken_disp_lg_mplus = 1;
    sys_reg_sys_clken3_REG.write_data = 1;
    IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, sys_reg_sys_clken3_REG.regValue);

    display_timing_ctrl2_REG.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
    display_timing_ctrl2_REG.mplus_rgbw_en = 4;
    IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_REG.regValue);

    if (Get_DISPLAY_PANEL_CUSTOM_INDEX()==P_CEDS_LG_49INCH_UHD60_12LANE){
        IoReg_Write32(LG_M_PLUS_M_plus_reg000_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg004_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg008_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg00c_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg010_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg014_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg018_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg01c_reg, 0x10001200);
        IoReg_Write32(LG_M_PLUS_M_plus_reg020_reg, 0x04000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg024_reg, 0x7e90007e);
        IoReg_Write32(LG_M_PLUS_M_plus_reg028_reg, 0x04000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg02c_reg, 0x7e90007e);
        IoReg_Write32(LG_M_PLUS_M_plus_reg030_reg, 0x00011400);
        IoReg_Write32(LG_M_PLUS_M_plus_reg034_reg, 0x10000006);
        IoReg_Write32(LG_M_PLUS_M_plus_reg038_reg, 0x00012000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg03c_reg, 0x10000006);
        IoReg_Write32(LG_M_PLUS_M_plus_reg040_reg, 0x01000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg044_reg, 0x01800800);
        IoReg_Write32(LG_M_PLUS_M_plus_reg048_reg, 0x00000010);
        IoReg_Write32(LG_M_PLUS_M_plus_reg04c_reg, 0x01000001);
        IoReg_Write32(LG_M_PLUS_M_plus_reg050_reg, 0x01801000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg054_reg, 0x00000010);
        IoReg_Write32(LG_M_PLUS_M_plus_reg058_reg, 0x20000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg05c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg060_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg064_reg, 0x20000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg068_reg, 0x00010000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg06c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg070_reg, 0x00010000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg074_reg, 0x00020000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg078_reg, 0x005d1482);
        IoReg_Write32(LG_M_PLUS_M_plus_reg07c_reg, 0x00040bb8);
        IoReg_Write32(LG_M_PLUS_M_plus_reg080_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg084_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg088_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg08c_reg, 0x04000008);
        IoReg_Write32(LG_M_PLUS_M_plus_reg090_reg, 0x00010040);
        IoReg_Write32(LG_M_PLUS_M_plus_reg094_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg098_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg09c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0a0_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0a4_reg, 0x0f000870);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0a8_reg, 0x00000808);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0ac_reg, 0x01000c00);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0b0_reg, 0x000800c0);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0b4_reg, 0x6c9f0100);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0b8_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0bc_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0c0_reg, 0x00800180);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0c4_reg, 0x00800018);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0c8_reg, 0x00ff0000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0cc_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0d0_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0d4_reg, 0x10000001);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0d8_reg, 0x00000002);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0dc_reg, 0x00010000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0e0_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0e4_reg, 0x00000001);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0e8_reg, 0x000e001a);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0ec_reg, 0x00220040);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0f0_reg, 0x00500060);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0f4_reg, 0x00700080);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0f8_reg, 0x009000a0);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0fc_reg, 0x00b000c0);
        IoReg_Write32(LG_M_PLUS_M_plus_reg100_reg, 0x00d000e0);
        IoReg_Write32(LG_M_PLUS_M_plus_reg104_reg, 0x00f00006);
        IoReg_Write32(LG_M_PLUS_M_plus_reg108_reg, 0x0000fa80);
        IoReg_Write32(LG_M_PLUS_M_plus_reg10c_reg, 0xfb000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg110_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg114_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg118_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg11c_reg, 0x02000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg120_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg124_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg128_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg12c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg130_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg134_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg138_reg, 0x000c0000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg13c_reg, 0x01000100);
        IoReg_Write32(LG_M_PLUS_M_plus_reg140_reg, 0x01000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg144_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg148_reg, 0x00000400);
        IoReg_Write32(LG_M_PLUS_M_plus_reg14c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg150_reg, 0x00000400);
        IoReg_Write32(LG_M_PLUS_M_plus_reg154_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg158_reg, 0x00000400);
        IoReg_Write32(LG_M_PLUS_M_plus_reg15c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg160_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg164_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg168_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg16c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg170_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg174_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg178_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg17c_reg, 0x028a03ff);
        IoReg_Write32(LG_M_PLUS_M_plus_reg180_reg, 0x028a0172);
        IoReg_Write32(LG_M_PLUS_M_plus_reg184_reg, 0x03ff0172);
        IoReg_Write32(LG_M_PLUS_M_plus_reg188_reg, 0x00ff00e6);
        IoReg_Write32(LG_M_PLUS_M_plus_reg18c_reg, 0x00ff0190);
        IoReg_Write32(LG_M_PLUS_M_plus_reg190_reg, 0x01ff0190);
        IoReg_Write32(LG_M_PLUS_M_plus_reg194_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg198_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg19c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1a0_reg, 0x03840384);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1a4_reg, 0x03840320);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1a8_reg, 0x02ee0258);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1ac_reg, 0x00640064);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1b0_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1b4_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1b8_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1bc_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1c0_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1c4_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1c8_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1cc_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1d0_reg, 0x0da12dc2);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1d4_reg, 0x049b0000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1d8_reg, 0x00000003);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1dc_reg, 0x46500000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1e0_reg, 0x30bc0000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1e4_reg, 0x4000303c);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1e8_reg, 0x3fff3800);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1ec_reg, 0x157c0000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1f0_reg, 0x000c03ac);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1f4_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1f8_reg, 0x00000002);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1fc_reg, 0x00000000);

    }else{
        IoReg_Write32(LG_M_PLUS_M_plus_reg000_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg004_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg008_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg00c_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg010_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg014_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg018_reg, 0x10001000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg01c_reg, 0x10001200);
        IoReg_Write32(LG_M_PLUS_M_plus_reg020_reg, 0x04000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg024_reg, 0x7e90007e);
        IoReg_Write32(LG_M_PLUS_M_plus_reg028_reg, 0x04000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg02c_reg, 0x7e90007e);
        IoReg_Write32(LG_M_PLUS_M_plus_reg030_reg, 0x00011400);
        IoReg_Write32(LG_M_PLUS_M_plus_reg034_reg, 0x10000006);
        IoReg_Write32(LG_M_PLUS_M_plus_reg038_reg, 0x00012000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg03c_reg, 0x10000006);
        IoReg_Write32(LG_M_PLUS_M_plus_reg040_reg, 0x01000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg044_reg, 0x01800800);
        IoReg_Write32(LG_M_PLUS_M_plus_reg048_reg, 0x00000010);
        IoReg_Write32(LG_M_PLUS_M_plus_reg04c_reg, 0x01000001);
        IoReg_Write32(LG_M_PLUS_M_plus_reg050_reg, 0x01801000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg054_reg, 0x00000010);
        IoReg_Write32(LG_M_PLUS_M_plus_reg058_reg, 0x20000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg05c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg060_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg064_reg, 0x20000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg068_reg, 0x00010000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg06c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg070_reg, 0x00010000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg074_reg, 0x00020000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg078_reg, 0x005d1482);
        IoReg_Write32(LG_M_PLUS_M_plus_reg07c_reg, 0x00040bb8);
        IoReg_Write32(LG_M_PLUS_M_plus_reg080_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg084_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg088_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg08c_reg, 0x04000008);
        IoReg_Write32(LG_M_PLUS_M_plus_reg090_reg, 0x00010040);
        IoReg_Write32(LG_M_PLUS_M_plus_reg094_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg098_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg09c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0a0_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0a4_reg, 0x0f000870);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0a8_reg, 0x00000808);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0ac_reg, 0x01000c00);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0b0_reg, 0x000800c0);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0b4_reg, 0x6c9f0100);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0b8_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0bc_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0c0_reg, 0x00800180);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0c4_reg, 0x00800018);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0c8_reg, 0x00ff0000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0cc_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0d0_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0d4_reg, 0x10000001);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0d8_reg, 0x00000002);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0dc_reg, 0x00010000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0e0_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0e4_reg, 0x00000001);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0e8_reg, 0x000e001a);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0ec_reg, 0x00220040);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0f0_reg, 0x00500060);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0f4_reg, 0x00700080);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0f8_reg, 0x009000a0);
        IoReg_Write32(LG_M_PLUS_M_plus_reg0fc_reg, 0x00b000c0);
        IoReg_Write32(LG_M_PLUS_M_plus_reg100_reg, 0x00d000e0);
        IoReg_Write32(LG_M_PLUS_M_plus_reg104_reg, 0x00f00006);
        IoReg_Write32(LG_M_PLUS_M_plus_reg108_reg, 0x0000fa80);
        IoReg_Write32(LG_M_PLUS_M_plus_reg10c_reg, 0xfb000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg110_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg114_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg118_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg11c_reg, 0x02000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg120_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg124_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg128_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg12c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg130_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg134_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg138_reg, 0x000c0000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg13c_reg, 0x01000100);
        IoReg_Write32(LG_M_PLUS_M_plus_reg140_reg, 0x01000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg144_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg148_reg, 0x00000400);
        IoReg_Write32(LG_M_PLUS_M_plus_reg14c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg150_reg, 0x00000400);
        IoReg_Write32(LG_M_PLUS_M_plus_reg154_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg158_reg, 0x00000400);
        IoReg_Write32(LG_M_PLUS_M_plus_reg15c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg160_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg164_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg168_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg16c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg170_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg174_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg178_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg17c_reg, 0x028a03ff);
        IoReg_Write32(LG_M_PLUS_M_plus_reg180_reg, 0x028a0172);
        IoReg_Write32(LG_M_PLUS_M_plus_reg184_reg, 0x03ff0172);
        IoReg_Write32(LG_M_PLUS_M_plus_reg188_reg, 0x00ff00e6);
        IoReg_Write32(LG_M_PLUS_M_plus_reg18c_reg, 0x00ff0190);
        IoReg_Write32(LG_M_PLUS_M_plus_reg190_reg, 0x01ff0190);
        IoReg_Write32(LG_M_PLUS_M_plus_reg194_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg198_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg19c_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1a0_reg, 0x03840384);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1a4_reg, 0x038402b0);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1a8_reg, 0x02a00258);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1ac_reg, 0x00640064);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1b0_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1b4_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1b8_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1bc_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1c0_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1c4_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1c8_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1cc_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1d0_reg, 0x0da12dc2);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1d4_reg, 0x049b0000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1d8_reg, 0x00000003);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1dc_reg, 0x46500000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1e0_reg, 0x30bc0000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1e4_reg, 0x4000303c);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1e8_reg, 0x3fff3800);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1ec_reg, 0x157c0000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1f0_reg, 0x000c03ac);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1f4_reg, 0x00000000);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1f8_reg, 0x00000002);
        IoReg_Write32(LG_M_PLUS_M_plus_reg1fc_reg, 0x00000000);
    }
    display_timing_ctrl2_REG.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
    display_timing_ctrl2_REG.dispd_lg_mplus_siw_rgbw_clk_en = 1;
    IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_REG.regValue);
#endif
}

void vbe_disp_panel_rgbw_resume(void)
{
	rtd_pr_vbe_notice("###[%s][Line:%d]###\n", __FUNCTION__, __LINE__);

	down(&VBE_RGBW_Resume_Semaphore);
	if(Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE){
		vbe_disp_panel_mplus_resume_flow();
	}else if(Get_DISPLAY_PANEL_BOW_RGBW() == TRUE){
		vbe_disp_panel_boe_resume_flow();
	}else{
		rtd_pr_vbe_notice("not rgbw tcon \n");
	}
	up(&VBE_RGBW_Resume_Semaphore);
}

void vbe_disp_panel_suspend(void)
{
#ifdef CONFIG_ANDROID
    // Till now, this happened at AN platform only.
    //
    // Enable force_bg to avoid garbage when resume
    //
    // Enter suspend when Youtube or Google Play is playing, the app might not stop play before driver suspend.
    // Which means the stop play will not occur until next resume.
    //
    // Under this situation, the code flow becomes
    // 1. Youtube or Google Play playing, user pressed DC off
    // 2. kernel asked driver to enter suspend
    //    a. vbe_disp_panel_suspend() w/o mute video
    //       i.PPOVERLAY_Main_Display_Control_RSV_reg.disp_en = 1
    //         PPOVERLAY_Main_Display_Control_RSV_reg.force_bg = 0
    // 3. suspend
    // 4. DC on
    // 5. kernel ask driver to execute resume
    //    a. disp_en still be 1, force_bg still be 0
    // 6. execute vbe_disp_lane_power_on()
    //    a. you'll see garbage video
    // 7. Youtube or Google Play stop playing
    //    a. mute video
    //
    // So we've to enable force_bg here to avoid such kind of garbage.
    //
    // For other use cases, it should be safe to enable force_bg here.
    // Because the whole playback flow will start again after resume.
    scalerdisplay_force_bg_enable(SLR_MAIN_DISPLAY, _ENABLE);

  #ifdef CONFIG_DUAL_CHANNEL
    scalerdisplay_force_bg_enable(SLR_SUB_DISPLAY, _ENABLE);
  #endif
#endif    // #if CONFIG_ANDROID

	//panel driver setting
	vbe_disp_suspend_resume.PPOVERLAY_Display_Background_Color_S_R = IoReg_Read32(PPOVERLAY_Display_Background_Color_reg);
	vbe_disp_suspend_resume.PPOVERLAY_Display_Background_Color_2_S_R = IoReg_Read32(PPOVERLAY_Display_Background_Color_2_reg);
	vbe_disp_suspend_resume.PPOVERLAY_Main_Display_Control_RSV_S_R = IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
	vbe_disp_suspend_resume.PPOVERLAY_Main_Border_Highlight_Border_Color1_S_R = IoReg_Read32(PPOVERLAY_Main_Border_Highlight_Border_Color1_reg);
	vbe_disp_suspend_resume.PPOVERLAY_Main_Border_Highlight_Border_Color2_S_R = IoReg_Read32(PPOVERLAY_Main_Border_Highlight_Border_Color2_reg);
	vbe_disp_suspend_resume.PPOVERLAY_Main_Border_Highlight_Border_Color3_S_R = IoReg_Read32(PPOVERLAY_Main_Border_Highlight_Border_Color3_reg);
	vbe_disp_suspend_resume.PPOVERLAY_MAIN_DEN_H_Start_End_S_R  = IoReg_Read32(PPOVERLAY_MAIN_DEN_H_Start_End_reg);
	vbe_disp_suspend_resume.PPOVERLAY_MAIN_DEN_V_Start_End_S_R = IoReg_Read32(PPOVERLAY_MAIN_DEN_V_Start_End_reg);
	vbe_disp_suspend_resume.PPOVERLAY_MAIN_Background_H_Start_End_S_R = IoReg_Read32(PPOVERLAY_MAIN_Background_H_Start_End_reg);
	vbe_disp_suspend_resume.PPOVERLAY_MAIN_Background_V_Start_End_S_R  = IoReg_Read32(PPOVERLAY_MAIN_Background_V_Start_End_reg);
	vbe_disp_suspend_resume.PPOVERLAY_MAIN_Active_H_Start_End_S_R = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);
	vbe_disp_suspend_resume.PPOVERLAY_MAIN_Active_V_Start_End_S_R = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);

	//sfg
	vbe_disp_suspend_resume.SFG_SFG_ZZ_shift_dbg_CTRL_S_R = 		 IoReg_Read32(SFG_SFG_ZZ_shift_dbg_CTRL_reg);
	vbe_disp_suspend_resume.SFG_SFG_SWAP_CTRL0_S_R =				 IoReg_Read32(SFG_SFG_SWAP_CTRL0_reg);
	vbe_disp_suspend_resume.SFG_SFG_CRC_CTRL_S_R =					 IoReg_Read32(SFG_SFG_CRC_CTRL_reg);
	vbe_disp_suspend_resume.SFG_SFG_CRC_RESULT_S_R =				 IoReg_Read32(SFG_SFG_CRC_RESULT_reg);
	vbe_disp_suspend_resume.SFG_SFG_CRC_R_RESULT_S_R =				 IoReg_Read32(SFG_SFG_CRC_R_RESULT_reg);
	vbe_disp_suspend_resume.SFG_SFG_L_DES_CRC_S_R = 				 IoReg_Read32(SFG_SFG_L_DES_CRC_reg);
	vbe_disp_suspend_resume.SFG_SFG_R_DES_CRC_S_R = 				 IoReg_Read32(SFG_SFG_R_DES_CRC_reg);
	vbe_disp_suspend_resume.SFG_SFG_CRC_ERR_CNT_S_R =				 IoReg_Read32(SFG_SFG_CRC_ERR_CNT_reg);
	vbe_disp_suspend_resume.SFG_SFG_FORCE_BG_AT_DIF_S_R =			 IoReg_Read32(SFG_SFG_FORCE_BG_AT_DIF_reg);
	vbe_disp_suspend_resume.SFG_ZZ_CTRL_S_R =						 IoReg_Read32(SFG_ZZ_CTRL_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_0A_S_R = 			 IoReg_Read32(SFG_SFG_DUMMY_CTRL_0A_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_0B_S_R = 			 IoReg_Read32(SFG_SFG_DUMMY_CTRL_0B_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_0B_S_R =					  IoReg_Read32(SFG_SFG_CTRL_0B_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_1A_S_R = 			 IoReg_Read32(SFG_SFG_DUMMY_CTRL_1A_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_1B_S_R = 			 IoReg_Read32(SFG_SFG_DUMMY_CTRL_1B_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_8_S_R =				 IoReg_Read32(SFG_SFG_DUMMY_CTRL_8_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_10_S_R = 			 IoReg_Read32(SFG_SFG_DUMMY_CTRL_10_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_93C_S_R = 			  IoReg_Read32(SFG_SFG_DUMMY_CTRL_93C_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B10_S_R = 			  IoReg_Read32(SFG_SFG_DUMMY_CTRL_B10_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B14_S_R = 			  IoReg_Read32(SFG_SFG_DUMMY_CTRL_B14_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B50_S_R = 			  IoReg_Read32(SFG_SFG_DUMMY_CTRL_B50_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B54_S_R = 			  IoReg_Read32(SFG_SFG_DUMMY_CTRL_B54_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B58_S_R = 			  IoReg_Read32(SFG_SFG_DUMMY_CTRL_B58_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B5C_S_R = 			  IoReg_Read32(SFG_SFG_DUMMY_CTRL_B5C_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B60_S_R = 			  IoReg_Read32(SFG_SFG_DUMMY_CTRL_B60_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B64_S_R = 			  IoReg_Read32(SFG_SFG_DUMMY_CTRL_B64_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_13_S_R =					 IoReg_Read32(SFG_SFG_CTRL_13_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_24_S_R =					 IoReg_Read32(SFG_SFG_CTRL_24_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_25_S_R =					 IoReg_Read32(SFG_SFG_CTRL_25_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_26_S_R =					 IoReg_Read32(SFG_SFG_CTRL_26_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_28_S_R =					  IoReg_Read32(SFG_SFG_CTRL_28_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_29_S_R =					  IoReg_Read32(SFG_SFG_CTRL_29_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_30_S_R =					  IoReg_Read32(SFG_SFG_CTRL_30_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_0_S_R =					 IoReg_Read32(SFG_SFG_CTRL_0_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_1_S_R =					 IoReg_Read32(SFG_SFG_CTRL_1_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_2_S_R =					 IoReg_Read32(SFG_SFG_CTRL_2_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_3_S_R =					 IoReg_Read32(SFG_SFG_CTRL_3_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_4_S_R =					 IoReg_Read32(SFG_SFG_CTRL_4_reg);
	vbe_disp_suspend_resume.SFG_SFG_START_CTRL_8_S_R =				 IoReg_Read32(SFG_SFG_START_CTRL_8_reg);
	vbe_disp_suspend_resume.SFG_SFG_START_CTRL_10_S_R = 			 IoReg_Read32(SFG_SFG_START_CTRL_10_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_11_S_R =					 IoReg_Read32(SFG_SFG_CTRL_11_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_12_S_R =					 IoReg_Read32(SFG_SFG_CTRL_12_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_6_S_R =					 IoReg_Read32(SFG_SFG_CTRL_6_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_7_S_R =					 IoReg_Read32(SFG_SFG_CTRL_7_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_8_S_R =					 IoReg_Read32(SFG_SFG_CTRL_8_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_9_S_R =					 IoReg_Read32(SFG_SFG_CTRL_9_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_5_S_R =					 IoReg_Read32(SFG_SFG_CTRL_5_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_A_S_R =					 IoReg_Read32(SFG_SFG_CTRL_A_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_B_S_R =					 IoReg_Read32(SFG_SFG_CTRL_B_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_C_S_R =					 IoReg_Read32(SFG_SFG_CTRL_C_reg);
	vbe_disp_suspend_resume.SFG_SFG_TG_AUTO_1_S_R = 				 IoReg_Read32(SFG_SFG_TG_AUTO_1_reg);
	vbe_disp_suspend_resume.SFG_SFG_TG_AUTO_2_S_R = 				 IoReg_Read32(SFG_SFG_TG_AUTO_2_reg);
	vbe_disp_suspend_resume.SFG_SFG_TG_AUTO_3_S_R = 				 IoReg_Read32(SFG_SFG_TG_AUTO_3_reg);
	vbe_disp_suspend_resume.SFG_SFG_TG_AUTO_4_S_R = 				 IoReg_Read32(SFG_SFG_TG_AUTO_4_reg);
	vbe_disp_suspend_resume.SFG_SFG_CLKEN_CTRL_S_R = 				 IoReg_Read32(SFG_SFG_CLKEN_CTRL_reg);
	vbe_disp_suspend_resume.SFG_SFG_BIST_MODE_S_R = 				 IoReg_Read32(SFG_SFG_BIST_MODE_reg);
	vbe_disp_suspend_resume.SFG_SFG_BIST_DONE_S_R = 				 IoReg_Read32(SFG_SFG_BIST_DONE_reg);
	vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_0_S_R = 		 IoReg_Read32(SFG_SFG_insert_subpixel_0_reg);
	vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_1_S_R = 		 IoReg_Read32(SFG_SFG_insert_subpixel_1_reg);
	vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_2_S_R = 		 IoReg_Read32(SFG_SFG_insert_subpixel_2_reg);
	vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_3_S_R = 		 IoReg_Read32(SFG_SFG_insert_subpixel_3_reg);
	vbe_disp_suspend_resume.SFG_SFG_ctrl_10_S_R =					 IoReg_Read32(SFG_SFG_ctrl_10_reg);
	vbe_disp_suspend_resume.SFG_SFG_Last_ext_pixel_last_flip_S_R =	 IoReg_Read32(SFG_SFG_Last_ext_pixel_last_flip_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_14_S_R =					 IoReg_Read32(SFG_SFG_CTRL_14_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_15_S_R =					 IoReg_Read32(SFG_SFG_CTRL_15_reg);
	vbe_disp_suspend_resume.SFG_SFG_debug_S_R = 					 IoReg_Read32(SFG_SFG_debug_reg);
	vbe_disp_suspend_resume.SFG_SFG_data_capture_vh_S_R =			 IoReg_Read32(SFG_SFG_data_capture_vh_reg);
	vbe_disp_suspend_resume.SFG_SFG_data_capture_src_S_R =			 IoReg_Read32(SFG_SFG_data_capture_src_reg);
	vbe_disp_suspend_resume.SFG_SFG_data_capture_read_data_S_R =	 IoReg_Read32(SFG_SFG_data_capture_read_data_reg);
	vbe_disp_suspend_resume.SFG_SFG_last_seg_0_S_R =				 IoReg_Read32(SFG_SFG_last_seg_0_reg);
	vbe_disp_suspend_resume.SFG_SFG_last_seg_1_S_R =				 IoReg_Read32(SFG_SFG_last_seg_1_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_0_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_0_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_1_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_1_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_2_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_2_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_6_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_6_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_7_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_7_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_8_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_8_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_12_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_12_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_13_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_13_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_14_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_14_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_18_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_18_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_19_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_19_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_20_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_20_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_3_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_3_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_4_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_4_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_5_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_5_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_9_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_9_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_10_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_10_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_11_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_11_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_15_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_15_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_16_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_16_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_17_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_17_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_21_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_21_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_22_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_22_reg);
	vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_23_S_R =				 IoReg_Read32(SFG_SFG_DRD_CTRL_23_reg);
	vbe_disp_suspend_resume.SFG_SFG_START_CTRL_27_S_R = 			 IoReg_Read32(SFG_SFG_START_CTRL_27_reg);
	vbe_disp_suspend_resume.SFG_SFG_START_CTRL_28_S_R = 			 IoReg_Read32(SFG_SFG_START_CTRL_28_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_29_S_R = 			 IoReg_Read32(SFG_SFG_DUMMY_CTRL_29_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_30_S_R = 			 IoReg_Read32(SFG_SFG_DUMMY_CTRL_30_reg);
	vbe_disp_suspend_resume.SFG_SFG1_fw_ext_SUBPIXEL_S_R =			 IoReg_Read32(SFG_SFG1_fw_ext_SUBPIXEL_reg);
	vbe_disp_suspend_resume.SFG_SFG1_fw_ext_SUBPIXEL2_S_R = 		 IoReg_Read32(SFG_SFG1_fw_ext_SUBPIXEL2_reg);
	vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_0_S_R =			 IoReg_Read32(SFG_SFG1_DUM_DATA_LINE_0_reg);
	vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_1_S_R =			 IoReg_Read32(SFG_SFG1_DUM_DATA_LINE_1_reg);
	vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_2_S_R =			 IoReg_Read32(SFG_SFG1_DUM_DATA_LINE_2_reg);
	vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_3_S_R =			 IoReg_Read32(SFG_SFG1_DUM_DATA_LINE_3_reg);
	vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_4_S_R =			 IoReg_Read32(SFG_SFG1_DUM_DATA_LINE_4_reg);
	vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_5_S_R =			 IoReg_Read32(SFG_SFG1_DUM_DATA_LINE_5_reg);
	vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_6_S_R =			 IoReg_Read32(SFG_SFG1_DUM_DATA_LINE_6_reg);
	vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_7_S_R =			 IoReg_Read32(SFG_SFG1_DUM_DATA_LINE_7_reg);
	vbe_disp_suspend_resume.SFG_ZZ_CTRL_2_S_R = 					 IoReg_Read32(SFG_ZZ_CTRL_2_reg);
	vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_4_S_R = 		 IoReg_Read32(SFG_SFG_insert_subpixel_4_reg);
	vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_5_S_R = 		 IoReg_Read32(SFG_SFG_insert_subpixel_5_reg);
	vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_6_S_R = 		 IoReg_Read32(SFG_SFG_insert_subpixel_6_reg);
	vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_7_S_R = 		 IoReg_Read32(SFG_SFG_insert_subpixel_7_reg);
	vbe_disp_suspend_resume.SFG_SFG_Last_ext_pixel_last_flip_2_S_R = IoReg_Read32(SFG_SFG_Last_ext_pixel_last_flip_2_reg);
	vbe_disp_suspend_resume.SFG_SFG_last_seg_2_S_R =				 IoReg_Read32(SFG_SFG_last_seg_2_reg);
	vbe_disp_suspend_resume.SFG_SFG_last_seg_3_S_R =				 IoReg_Read32(SFG_SFG_last_seg_3_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_ctrl_S_R =				 IoReg_Read32(SFG_SFG_patgen_ctrl_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_0_S_R =					 IoReg_Read32(SFG_SFG_patgen_0_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_1_S_R =					 IoReg_Read32(SFG_SFG_patgen_1_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_2_S_R =					 IoReg_Read32(SFG_SFG_patgen_2_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_3_S_R =					 IoReg_Read32(SFG_SFG_patgen_3_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_4_S_R =					 IoReg_Read32(SFG_SFG_patgen_4_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_5_S_R =					 IoReg_Read32(SFG_SFG_patgen_5_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_6_S_R =					 IoReg_Read32(SFG_SFG_patgen_6_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_7_S_R =					 IoReg_Read32(SFG_SFG_patgen_7_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_8_S_R =					 IoReg_Read32(SFG_SFG_patgen_8_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_9_S_R =					 IoReg_Read32(SFG_SFG_patgen_9_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_10_S_R = 				 IoReg_Read32(SFG_SFG_patgen_10_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_11_S_R = 				 IoReg_Read32(SFG_SFG_patgen_11_reg);
	vbe_disp_suspend_resume.SFG_SFG_patgen_12_S_R = 				 IoReg_Read32(SFG_SFG_patgen_12_reg);
	vbe_disp_suspend_resume.SFG_SFG_irq_ctrl_0_S_R =				 IoReg_Read32(SFG_SFG_irq_ctrl_0_reg);
	vbe_disp_suspend_resume.SFG_SFG_irq_ctrl_1_S_R =				 IoReg_Read32(SFG_SFG_irq_ctrl_1_reg);
	vbe_disp_suspend_resume.SFG_Pat_det_window_H_ini_sta_width_S_R = IoReg_Read32(SFG_Pat_det_window_H_ini_sta_width_reg);
	vbe_disp_suspend_resume.SFG_Pat_det_window_V_ini_sta_height_S_R= IoReg_Read32(SFG_Pat_det_window_V_ini_sta_height_reg);
	vbe_disp_suspend_resume.SFG_Pattern_detect_CTRL_S_R =			 IoReg_Read32(SFG_Pattern_detect_CTRL_reg);
	vbe_disp_suspend_resume.SFG_Pat_det_ctrl_1_S_R =				 IoReg_Read32(SFG_Pat_det_ctrl_1_reg);
	vbe_disp_suspend_resume.SFG_Pat_det_ctrl_2_S_R =				 IoReg_Read32(SFG_Pat_det_ctrl_2_reg);
	vbe_disp_suspend_resume.SFG_Pat_det_ctrl_3_S_R =				 IoReg_Read32(SFG_Pat_det_ctrl_3_reg);
	vbe_disp_suspend_resume.SFG_Pat_det_ctrl_4_S_R =				 IoReg_Read32(SFG_Pat_det_ctrl_4_reg);
	vbe_disp_suspend_resume.SFG_Pat_det_ctrl_5_S_R =				 IoReg_Read32(SFG_Pat_det_ctrl_5_reg);
	vbe_disp_suspend_resume.SFG_Pat_det_ctrl_7_S_R =				 IoReg_Read32(SFG_Pat_det_ctrl_7_reg);
	vbe_disp_suspend_resume.SFG_Pat_det_ctrl_6_S_R =				 IoReg_Read32(SFG_Pat_det_ctrl_6_reg);
	vbe_disp_suspend_resume.SFG_Pat_det_acc11_S_R = 				 IoReg_Read32(SFG_Pat_det_acc11_reg);
	vbe_disp_suspend_resume.SFG_Debug_pin_S_R = 					 IoReg_Read32(SFG_Debug_pin_reg);
	vbe_disp_suspend_resume.SFG_SFG2_CTRL_0_S_R =					 IoReg_Read32(SFG_SFG2_CTRL_0_reg);
	vbe_disp_suspend_resume.SFG_SFG2_FORCE_0_S_R =					 IoReg_Read32(SFG_SFG2_FORCE_0_reg);
	vbe_disp_suspend_resume.SFG_SFG2_FORCE_1_S_R =					 IoReg_Read32(SFG_SFG2_FORCE_1_reg);
	vbe_disp_suspend_resume.SFG_SFG2_CRC_CTRL_S_R = 				 IoReg_Read32(SFG_SFG2_CRC_CTRL_reg);
	vbe_disp_suspend_resume.SFG_SFG2_CRC_RESULT_S_R =				 IoReg_Read32(SFG_SFG2_CRC_RESULT_reg);
	vbe_disp_suspend_resume.SFG_SFG2_CRC_R_RESULT_S_R = 			 IoReg_Read32(SFG_SFG2_CRC_R_RESULT_reg);
	vbe_disp_suspend_resume.SFG_SFG2_CRC_L_DES_CRC_S_R =			 IoReg_Read32(SFG_SFG2_CRC_L_DES_CRC_reg);
	vbe_disp_suspend_resume.SFG_SFG2_CRC_R_DES_CRC_S_R =			 IoReg_Read32(SFG_SFG2_CRC_R_DES_CRC_reg);
	vbe_disp_suspend_resume.SFG_SFG2_CRC_ERR_CNT_S_R =				 IoReg_Read32(SFG_SFG2_CRC_ERR_CNT_reg);
	vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL0_S_R =				 IoReg_Read32(SFG_PIF_FIFO_CTRL0_reg);
	vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL1_S_R =				 IoReg_Read32(SFG_PIF_FIFO_CTRL1_reg);
	vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL2_S_R =				 IoReg_Read32(SFG_PIF_FIFO_CTRL2_reg);
	vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL3_S_R =				 IoReg_Read32(SFG_PIF_FIFO_CTRL3_reg);
	vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL4_S_R =				 IoReg_Read32(SFG_PIF_FIFO_CTRL4_reg);
	vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL5_S_R =				 IoReg_Read32(SFG_PIF_FIFO_CTRL5_reg);
	vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL6_S_R =				 IoReg_Read32(SFG_PIF_FIFO_CTRL6_reg);
	vbe_disp_suspend_resume.SFG_PIF_FIFO_MUX_CTRL0_S_R =			 IoReg_Read32(SFG_PIF_FIFO_MUX_CTRL0_reg);
	vbe_disp_suspend_resume.SFG_PIF_FIFO_MUX_CTRL1_S_R =			 IoReg_Read32(SFG_PIF_FIFO_MUX_CTRL1_reg);
	vbe_disp_suspend_resume.SFG_PIF_FIFO_MUX_CTRL2_S_R =			 IoReg_Read32(SFG_PIF_FIFO_MUX_CTRL2_reg);
	vbe_disp_suspend_resume.SFG_DE_HALF_CTL_S_R =					  IoReg_Read32(SFG_DE_HALF_CTL_reg);
	vbe_disp_suspend_resume.SFG_PIF_MISC_S_R =						 IoReg_Read32(SFG_PIF_MISC_reg);
	vbe_disp_suspend_resume.SFG_SFG_EXT5_S_R =						  IoReg_Read32(SFG_SFG_EXT5_reg);
	vbe_disp_suspend_resume.SFG_SFG_EXT6_S_R =						  IoReg_Read32(SFG_SFG_EXT6_reg);
	vbe_disp_suspend_resume.SFG_SFG_EXT7_S_R =						  IoReg_Read32(SFG_SFG_EXT7_reg);
	vbe_disp_suspend_resume.SFG_SFG_EXT8_S_R =						  IoReg_Read32(SFG_SFG_EXT8_reg);
	vbe_disp_suspend_resume.SFG_SFG_EXT9_S_R =						  IoReg_Read32(SFG_SFG_EXT9_reg);
	vbe_disp_suspend_resume.SFG_SFG_EXT10_S_R = 					  IoReg_Read32(SFG_SFG_EXT10_reg);
	vbe_disp_suspend_resume.SFG_SFG_EXT11_S_R = 					  IoReg_Read32(SFG_SFG_EXT11_reg);
	vbe_disp_suspend_resume.SFG_SFG_EXT12_S_R = 					  IoReg_Read32(SFG_SFG_EXT12_reg);
	vbe_disp_suspend_resume.SFG_SFG_EXT13_S_R = 					  IoReg_Read32(SFG_SFG_EXT13_reg);
	vbe_disp_suspend_resume.SFG_SFG_EXT14_S_R = 					  IoReg_Read32(SFG_SFG_EXT14_reg);
	vbe_disp_suspend_resume.SFG_SFG_EXT15_S_R = 					  IoReg_Read32(SFG_SFG_EXT15_reg);
	vbe_disp_suspend_resume.SFG_SFG_EXT16_S_R = 					  IoReg_Read32(SFG_SFG_EXT16_reg);
	vbe_disp_suspend_resume.SFG_SFG_LC_S_R =						  IoReg_Read32(SFG_SFG_LC_reg);
	vbe_disp_suspend_resume.SFG_SFG_DUMMY_REG_S_R = 				  IoReg_Read32(SFG_SFG_DUMMY_REG_reg);

	//panel interface common
#if 0 // not use in Merlin8
	vbe_disp_suspend_resume.PIF_LVDS_CTRL1_S_R =			  IoReg_Read32(PIF_LVDS_CTRL1_reg);
	vbe_disp_suspend_resume.PIF_LVDS_CTRL2_S_R =			  IoReg_Read32(PIF_LVDS_CTRL2_reg);
	vbe_disp_suspend_resume.PIF_LVDS_CTRL3_S_R =			  IoReg_Read32(PIF_LVDS_CTRL3_reg);
	vbe_disp_suspend_resume.PIF_LVDS_CTRL4_S_R =			  IoReg_Read32(PIF_LVDS_CTRL4_reg);
	vbe_disp_suspend_resume.PIF_LVDS_CTRL5_S_R =			  IoReg_Read32(PIF_LVDS_CTRL5_reg);
	vbe_disp_suspend_resume.PIF_MLVDS_CTRL0_S_R =			  IoReg_Read32(PIF_MLVDS_CTRL0_reg);
	vbe_disp_suspend_resume.PIF_MLVDS_Reset_Pul_CTRL_S_R =	  IoReg_Read32(PIF_MLVDS_Reset_Pul_CTRL_reg);
	vbe_disp_suspend_resume.PIF_MLVDS_CTRL1_S_R =			  IoReg_Read32(PIF_MLVDS_CTRL1_reg);
	vbe_disp_suspend_resume.PIF_MLVDS_CTRL2_S_R =			  IoReg_Read32(PIF_MLVDS_CTRL2_reg);
	vbe_disp_suspend_resume.PIF_MLVDS_CTRL3_S_R =			  IoReg_Read32(PIF_MLVDS_CTRL3_reg);
	vbe_disp_suspend_resume.PIF_MLVDS_CTRL4_S_R =			  IoReg_Read32(PIF_MLVDS_CTRL4_reg);
#endif

	//PIF APHY
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_PIF_3_S_R =					   IoReg_Read32(PINMUX_LVDSPHY_PIF_3_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_PIF_4_S_R =					   IoReg_Read32(PINMUX_LVDSPHY_PIF_4_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_PIF_5_S_R =					   IoReg_Read32(PINMUX_LVDSPHY_PIF_5_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_PIF_6_S_R =					   IoReg_Read32(PINMUX_LVDSPHY_PIF_6_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_Pin_Mux_LVDS_S_R =			   IoReg_Read32(PINMUX_LVDSPHY_Pin_Mux_LVDS_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_0_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_0_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_1_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_1_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_2_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_2_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_3_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_3_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_3_2_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_3_2_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_4_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_4_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_4_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_4_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_18_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_18_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_27_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_27_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_5_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_5_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_0_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_0_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_4_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_4_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_8_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_8_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_12_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_12_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_16_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_16_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_20_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_20_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_24_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_24_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_28_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_28_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_32_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_32_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_36_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_36_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_40_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_40_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_44_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_44_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_48_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_48_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_52_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_52_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_56_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_56_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_60_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_60_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_1_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_1_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_5_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_5_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_9_S_R =  IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_9_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_13_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_13_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_17_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_17_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_21_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_21_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_25_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_25_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_29_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_29_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_33_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_33_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_37_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_37_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_41_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_41_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_45_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_45_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_49_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_49_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_53_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_53_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_57_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_57_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_61_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_61_reg);
	if(IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_reg)!=0){
		vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_reg);
	}
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_34_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_34_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_38_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_38_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_42_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_42_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_46_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_46_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_50_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_50_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_54_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_54_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_58_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_58_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_62_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_62_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_66_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_66_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_70_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_70_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_74_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_74_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_78_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_78_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_82_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_82_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_86_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_86_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_90_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_90_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_94_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_94_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_95_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_95_reg);

	vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_00_S_R = IoReg_Read32(PINMUX_LVDSPHY_AUX_APHY_CTRL_00_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_01_S_R = IoReg_Read32(PINMUX_LVDSPHY_AUX_APHY_CTRL_01_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_02_S_R = IoReg_Read32(PINMUX_LVDSPHY_AUX_APHY_CTRL_02_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_03_S_R = IoReg_Read32(PINMUX_LVDSPHY_AUX_APHY_CTRL_03_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_04_S_R = IoReg_Read32(PINMUX_LVDSPHY_AUX_APHY_CTRL_04_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_05_S_R = IoReg_Read32(PINMUX_LVDSPHY_AUX_APHY_CTRL_05_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_06_S_R = IoReg_Read32(PINMUX_LVDSPHY_AUX_APHY_CTRL_06_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_07_S_R = IoReg_Read32(PINMUX_LVDSPHY_AUX_APHY_CTRL_07_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_08_S_R = IoReg_Read32(PINMUX_LVDSPHY_AUX_APHY_CTRL_08_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_09_S_R = IoReg_Read32(PINMUX_LVDSPHY_AUX_APHY_CTRL_09_reg);

	vbe_disp_suspend_resume.PINMUX_LVDSPHY_CMU_SSC_CTRL_00_S_R = IoReg_Read32(PINMUX_LVDSPHY_CMU_SSC_CTRL_00_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_CMU_SSC_CTRL_01_S_R = IoReg_Read32(PINMUX_LVDSPHY_CMU_SSC_CTRL_01_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_CMU_SSC_CTRL_02_S_R = IoReg_Read32(PINMUX_LVDSPHY_CMU_SSC_CTRL_02_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_CMU_SSC_CTRL_03_S_R = IoReg_Read32(PINMUX_LVDSPHY_CMU_SSC_CTRL_03_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_CMU_SSC_CTRL_04_S_R = IoReg_Read32(PINMUX_LVDSPHY_CMU_SSC_CTRL_04_reg);
	vbe_disp_suspend_resume.PINMUX_LVDSPHY_CMU_SSC_CTRL_08_S_R = IoReg_Read32(PINMUX_LVDSPHY_CMU_SSC_CTRL_08_reg);

	//local dimming
	vbe_disp_suspend_resume.LDSPI_LD_SPI_H_TOTAL_S_R = IoReg_Read32(LDSPI_ld_spi_h_total_reg);
	vbe_disp_suspend_resume.LDSPI_LD_SPI_V_TOTAL_S_R = IoReg_Read32(LDSPI_ld_spi_v_total_reg);
	vbe_disp_suspend_resume.LDSPI_LD_SPI_REPRODUCE_S_R = IoReg_Read32(LDSPI_ld_spi_reproduce_mode_reg);

	//frank@06222019 disable epi enable default
	//IoReg_ClearBits(EPI_EPI_PORT_OPTION_CTRL_reg, EPI_EPI_PORT_OPTION_CTRL_epi_enable_mask);


	if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_4K2K)  ||
		(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS) ||
		(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) ||
		(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI) ||
        (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_eDP))
	{
		//VBy1 Mac
		vbe_disp_suspend_resume.VBY1_VIDEO_FORMAT_S_R = 	   IoReg_Read32(VBY1_VIDEO_FORMAT_reg);
		vbe_disp_suspend_resume.VBY1_LANE_STATUS_S_R =		   IoReg_Read32(VBY1_LANE_STATUS_reg);
		vbe_disp_suspend_resume.VBY1_LANE_STATUS_1_S_R =	   IoReg_Read32(VBY1_LANE_STATUS_1_reg);
		vbe_disp_suspend_resume.VBY1_CTL_S_R =				   IoReg_Read32(VBY1_CTL_reg);
		vbe_disp_suspend_resume.VBY1_TRANSMIT_CONTROL_S_R =    IoReg_Read32(VBY1_TRANSMIT_CONTROL_reg);
		vbe_disp_suspend_resume.VBY1_TRANSMIT_STATUS_S_R =	   IoReg_Read32(VBY1_TRANSMIT_STATUS_reg);
		vbe_disp_suspend_resume.VBY1_THREE_FLAG_S_R =		   IoReg_Read32(VBY1_THREE_FLAG_reg);
		vbe_disp_suspend_resume.VBY1_CRC_CONTROL_S_R =		   IoReg_Read32(VBY1_CRC_CONTROL_reg);
		vbe_disp_suspend_resume.VBY1_CRC_RESULT_S_R =		   IoReg_Read32(VBY1_CRC_RESULT_reg);
		vbe_disp_suspend_resume.VBY1_CRC_DES_S_R =			   IoReg_Read32(VBY1_CRC_DES_reg);
		vbe_disp_suspend_resume.VBY1_CRC_ERROR_CNT_S_R =	   IoReg_Read32(VBY1_CRC_ERROR_CNT_reg);
		vbe_disp_suspend_resume.VBY1_ASYNC_FIFO_RST_S_R =	   IoReg_Read32(VBY1_ASYNC_FIFO_RST_reg);
		vbe_disp_suspend_resume.VBY1_ASYNC_FIFO_BIST_S_R =	   IoReg_Read32(VBY1_ASYNC_FIFO_BIST_reg);
		vbe_disp_suspend_resume.VBY1_VBY1_DEBUG_S_R =		   IoReg_Read32(VBY1_VBY1_DEBUG_reg);
		vbe_disp_suspend_resume.VBY1_VBY1_STATUS_FW_CTRL_S_R = IoReg_Read32(VBY1_VBY1_STATUS_FW_CTRL_reg);
		vbe_disp_suspend_resume.VBY1_DATA_CAPTURE_S_R = 	   IoReg_Read32(VBY1_DATA_CAPTURE_reg);
		vbe_disp_suspend_resume.VBY1_DATA_CAPTURE_1_S_R =	   IoReg_Read32(VBY1_DATA_CAPTURE_1_reg);
		vbe_disp_suspend_resume.VBY1_DATA_CAPTURE_2_S_R =	   IoReg_Read32(VBY1_DATA_CAPTURE_2_reg);
		vbe_disp_suspend_resume.VBY1_DATA_CAPTURE_3_S_R =	   IoReg_Read32(VBY1_DATA_CAPTURE_3_reg);
		vbe_disp_suspend_resume.VBY1_DBG_EN_S_R =			   IoReg_Read32(VBY1_DBG_EN_reg);
		vbe_disp_suspend_resume.VBY1_DBG_DATA_S_R = 		   IoReg_Read32(VBY1_DBG_DATA_reg);
		vbe_disp_suspend_resume.VBY1_DPHY_CTL_S_R = 		   IoReg_Read32(VBY1_DPHY_CTL_reg);
		vbe_disp_suspend_resume.VBY1_DPHY_RD_S_R =			   IoReg_Read32(VBY1_DPHY_RD_reg);
		vbe_disp_suspend_resume.VBY1_META_CTL_ACL_S_R =		   IoReg_Read32(VBY1_META_CTL_ACL_reg);
		vbe_disp_suspend_resume.VBY1_META_CTL_ACL1_S_R =	   IoReg_Read32(VBY1_META_CTL_ACL1_reg);
		vbe_disp_suspend_resume.VBY1_META_CTL_indicator_APL_S_R = IoReg_Read32(VBY1_META_CTL_indicator_APL_reg);
		vbe_disp_suspend_resume.VBY1_META_CTL_ACL_MP0_S_R =	   IoReg_Read32(VBY1_META_CTL_ACL_MP0_reg);
		vbe_disp_suspend_resume.VBY1_META_CTRL_0B_S_R =		   IoReg_Read32(VBY1_META_CTRL_0B_reg);
		vbe_disp_suspend_resume.VBY1_META_CTL_S_R = 		   IoReg_Read32(VBY1_META_CTL_reg);
		vbe_disp_suspend_resume.VBY1_META_CTL_1_S_R =		   IoReg_Read32(VBY1_META_CTL_1_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_0_S_R =		   IoReg_Read32(VBY1_META_DATA_0_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_1_S_R =		   IoReg_Read32(VBY1_META_DATA_1_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_2_S_R =		   IoReg_Read32(VBY1_META_DATA_2_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_3_S_R =		   IoReg_Read32(VBY1_META_DATA_3_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_4_S_R =		   IoReg_Read32(VBY1_META_DATA_4_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_5_S_R =		   IoReg_Read32(VBY1_META_DATA_5_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_6_S_R =		   IoReg_Read32(VBY1_META_DATA_6_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_7_S_R =		   IoReg_Read32(VBY1_META_DATA_7_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_8_S_R =		   IoReg_Read32(VBY1_META_DATA_8_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_9_S_R =		   IoReg_Read32(VBY1_META_DATA_9_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_10_S_R = 	   IoReg_Read32(VBY1_META_DATA_10_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_11_S_R = 	   IoReg_Read32(VBY1_META_DATA_11_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_12_S_R = 	   IoReg_Read32(VBY1_META_DATA_12_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_13_S_R = 	   IoReg_Read32(VBY1_META_DATA_13_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_14_S_R = 	   IoReg_Read32(VBY1_META_DATA_14_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_15_S_R = 	   IoReg_Read32(VBY1_META_DATA_15_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_16_S_R = 	   IoReg_Read32(VBY1_META_DATA_16_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_17_S_R = 	   IoReg_Read32(VBY1_META_DATA_17_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_18_S_R = 	   IoReg_Read32(VBY1_META_DATA_18_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_19_S_R = 	   IoReg_Read32(VBY1_META_DATA_19_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_20_S_R = 	   IoReg_Read32(VBY1_META_DATA_20_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_21_S_R = 	   IoReg_Read32(VBY1_META_DATA_21_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_22_S_R = 	   IoReg_Read32(VBY1_META_DATA_22_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_23_S_R = 	   IoReg_Read32(VBY1_META_DATA_23_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_24_S_R = 	   IoReg_Read32(VBY1_META_DATA_24_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_25_S_R = 	   IoReg_Read32(VBY1_META_DATA_25_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_26_S_R = 	   IoReg_Read32(VBY1_META_DATA_26_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_27_S_R = 	   IoReg_Read32(VBY1_META_DATA_27_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_28_S_R = 	   IoReg_Read32(VBY1_META_DATA_28_reg);
		vbe_disp_suspend_resume.VBY1_META_DATA_29_S_R = 	   IoReg_Read32(VBY1_META_DATA_29_reg);
		// for OSD split
		if((Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_4K2K_OSD2K1K_to_H5X_SPLIT) ||(Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_4K2K_OSD1K2K_to_H5X_SPLIT)){
			vbe_disp_suspend_resume.VBY1_OSD_OSD_VIDEO_FORMAT_S_R = IoReg_Read32(VBY1_OSD_OSD_VIDEO_FORMAT_reg);
			vbe_disp_suspend_resume.VBY1_OSD_OSD_LANE_STATUS_S_R = IoReg_Read32(VBY1_OSD_OSD_LANE_STATUS_reg);
			vbe_disp_suspend_resume.VBY1_OSD_OSD_CTL_S_R = IoReg_Read32(VBY1_OSD_OSD_CTL_reg);
			vbe_disp_suspend_resume.VBY1_OSD_OSD_TRANSMIT_CONTROL_S_R = IoReg_Read32(VBY1_OSD_OSD_TRANSMIT_CONTROL_reg);
			vbe_disp_suspend_resume.VBY1_OSD_OSD_THREE_FLAG_S_R = IoReg_Read32(VBY1_OSD_OSD_THREE_FLAG_reg);
			vbe_disp_suspend_resume.VBY1_OSD_OSD_CRC_CONTROL_S_R = IoReg_Read32(VBY1_OSD_OSD_CRC_CONTROL_reg);
			vbe_disp_suspend_resume.VBY1_OSD_OSD_VBY1_STATUS_FW_CTRL_S_R = IoReg_Read32(VBY1_OSD_OSD_VBY1_STATUS_FW_CTRL_reg);
			vbe_disp_suspend_resume.VBY1_OSD_OSD_DPHY_CTL_S_R = IoReg_Read32(VBY1_OSD_OSD_DPHY_CTL_reg);
		}
	}
	else if((Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE)||(Get_DISPLAY_PANEL_TYPE() == P_CEDS))
	{
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_PACKET_CTR1_0_0_S_R =			 IoReg_Read32(EPI_EPI_PACKET_CTR1_0_0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_PACKET_CTRL_S_R =				 IoReg_Read32(EPI_EPI_PACKET_CTRL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLOCK_CTRL0_S_R =				 IoReg_Read32(EPI_EPI_CLOCK_CTRL0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_DGB_READ_S_R =				 IoReg_Read32(EPI_EPI_DGB_READ_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_PORT_OPTION_CTRL_S_R =		 IoReg_Read32(EPI_EPI_PORT_OPTION_CTRL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_FRONT_S_R =				 IoReg_Read32(EPI_EPI_CMPI_FRONT_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_CMPI_DUMMY_COLOR_0_S_R =			 IoReg_Read32(EPI_CMPI_DUMMY_COLOR_0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_CMPI_DUMMY_COLOR_1_S_R =			 IoReg_Read32(EPI_CMPI_DUMMY_COLOR_1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_CMPI_DUMMY_COLOR_2_S_R =			 IoReg_Read32(EPI_CMPI_DUMMY_COLOR_2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_CMPI_DUMMY_COLOR_3_S_R =			 IoReg_Read32(EPI_CMPI_DUMMY_COLOR_3_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CRC_CTRL_0_S_R =				 IoReg_Read32(EPI_EPI_CRC_CTRL_0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CRC_CTRL_1_S_R =				 IoReg_Read32(EPI_EPI_CRC_CTRL_1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CRC_CTRL_2_S_R =				 IoReg_Read32(EPI_EPI_CRC_CTRL_2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_DATA_SRC_S_R =				 IoReg_Read32(EPI_EPI_DATA_SRC_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_PACKET_CTR1_INV_S_R =			 IoReg_Read32(EPI_EPI_PACKET_CTR1_INV_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_PACKET_CTR1_INV_TABLE_1_S_R =	 IoReg_Read32(EPI_EPI_PACKET_CTR1_INV_TABLE_1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_RLC_S_R =				 IoReg_Read32(EPI_EPI_CMPI_RLC_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_VBT_S_R =				 IoReg_Read32(EPI_EPI_CMPI_VBT_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_VBT_OPT_S_R =			 IoReg_Read32(EPI_EPI_CMPI_VBT_OPT_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR_TCON_RST_S_R =			 IoReg_Read32(EPI_EPI_SCR_TCON_RST_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR8_CTRL_S_R = 				 IoReg_Read32(EPI_EPI_SCR8_CTRL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR8_INI_VAL_S_R =			 IoReg_Read32(EPI_EPI_SCR8_INI_VAL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR10_CTRL_S_R =				 IoReg_Read32(EPI_EPI_SCR10_CTRL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR10_INI_VAL_S_R = 			 IoReg_Read32(EPI_EPI_SCR10_INI_VAL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR8CM_CTRL_S_R =				 IoReg_Read32(EPI_EPI_SCR8CM_CTRL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR8CM_INI_VAL_S_R =			 IoReg_Read32(EPI_EPI_SCR8CM_INI_VAL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR10CM_CTRL_S_R =			 IoReg_Read32(EPI_EPI_SCR10CM_CTRL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR10CM_INI_VAL_S_R =			 IoReg_Read32(EPI_EPI_SCR10CM_INI_VAL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CTRL_S_R =			 IoReg_Read32(EPI_EPI_CMPI_PG_CTRL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CMD_ST_S_R =			 IoReg_Read32(EPI_EPI_CMPI_PG_CMD_ST_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CMD_CMD1_S_R =		 IoReg_Read32(EPI_EPI_CMPI_PG_CMD_CMD1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CMD_CMD2_S_R =		 IoReg_Read32(EPI_EPI_CMPI_PG_CMD_CMD2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CMD_CMD3_S_R =		 IoReg_Read32(EPI_EPI_CMPI_PG_CMD_CMD3_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CMD_CMD4_S_R =		 IoReg_Read32(EPI_EPI_CMPI_PG_CMD_CMD4_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CMD_CMD5_S_R =		 IoReg_Read32(EPI_EPI_CMPI_PG_CMD_CMD5_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_START_S_R = 			 IoReg_Read32(EPI_EPI_CMPI_PG_START_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_END_S_R =				 IoReg_Read32(EPI_EPI_CMPI_PG_END_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LS_PORT_EN_S_R =				 IoReg_Read32(EPI_EPI_LS_PORT_EN_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_FSM_SEL_S_R =					 IoReg_Read32(EPI_EPI_FSM_SEL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_DATA_PORT_EN_S_R =			 IoReg_Read32(EPI_EPI_DATA_PORT_EN_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_DEN_FALL_LOCA_S_R = 			 IoReg_Read32(EPI_EPI_DEN_FALL_LOCA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LOCK_DEBOUNCE_CTL_S_R = 		 IoReg_Read32(EPI_EPI_LOCK_DEBOUNCE_CTL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_DBG_REG_FLAG_S_R =			 IoReg_Read32(EPI_EPI_DBG_REG_FLAG_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_DBG_REG_CTL_S_R =				 IoReg_Read32(EPI_EPI_DBG_REG_CTL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_CONTROL_S_R =				 IoReg_Read32(EPI_EPI_SOE_CONTROL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE0_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE0_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE0_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE0_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE1_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE1_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE1_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE1_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE2_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE2_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE2_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE2_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE3_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE3_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE3_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE3_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE3_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE3_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE3_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE3_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE4_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE4_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE4_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE4_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE4_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE4_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE4_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE4_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE5_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE5_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE5_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE5_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE5_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE5_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE5_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE5_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE6_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE6_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE6_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE6_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE6_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE6_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE6_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE6_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE7_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE7_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE7_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE7_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE7_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE7_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE7_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE7_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE8_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE8_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE8_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE8_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE8_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE8_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE8_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE8_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE9_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE9_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE9_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE9_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE9_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE9_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE9_S_R =			 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE9_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE10_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE10_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE10_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE10_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE10_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE10_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE10_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE10_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE11_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE11_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE11_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE11_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE11_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE11_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE11_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE11_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE12_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE12_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE12_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE12_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE12_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE12_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE12_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE12_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE13_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE13_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE13_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE13_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE13_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE13_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE13_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE13_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE14_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE14_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE14_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE14_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE14_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE14_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE14_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE14_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE15_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE0_LANE15_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE15_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE1_LANE15_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE15_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE2_LANE15_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE15_S_R =		 IoReg_Read32(EPI_EPI_SOE_LINE3_LANE15_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_MUX0_S_R = 			 IoReg_Read32(EPI_EPI_LANE_DST_MUX0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_MUX1_S_R = 			 IoReg_Read32(EPI_EPI_LANE_DST_MUX1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_MUX2_S_R = 			 IoReg_Read32(EPI_EPI_LANE_DST_MUX2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_MUX3_S_R = 			 IoReg_Read32(EPI_EPI_LANE_DST_MUX3_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_MUX4_S_R = 			 IoReg_Read32(EPI_EPI_LANE_DST_MUX4_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_MUX5_S_R = 			 IoReg_Read32(EPI_EPI_LANE_DST_MUX5_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_MUX0_S_R = 			 IoReg_Read32(EPI_EPI_LANE_CST_MUX0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_MUX1_S_R = 			 IoReg_Read32(EPI_EPI_LANE_CST_MUX1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_MUX2_S_R = 			 IoReg_Read32(EPI_EPI_LANE_CST_MUX2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_MUX3_S_R = 			 IoReg_Read32(EPI_EPI_LANE_CST_MUX3_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_MUX4_S_R = 			 IoReg_Read32(EPI_EPI_LANE_CST_MUX4_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_MUX5_S_R = 			 IoReg_Read32(EPI_EPI_LANE_CST_MUX5_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR1_MUX0_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR1_MUX0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR1_MUX1_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR1_MUX1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR1_MUX2_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR1_MUX2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR1_MUX3_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR1_MUX3_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR1_MUX4_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR1_MUX4_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR1_MUX5_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR1_MUX5_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR2_MUX0_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR2_MUX0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR2_MUX1_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR2_MUX1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR2_MUX2_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR2_MUX2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR2_MUX3_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR2_MUX3_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR2_MUX4_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR2_MUX4_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR2_MUX5_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR2_MUX5_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR3_MUX0_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR3_MUX0_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR3_MUX1_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR3_MUX1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR3_MUX2_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR3_MUX2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR3_MUX3_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR3_MUX3_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR3_MUX4_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR3_MUX4_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR3_MUX5_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR3_MUX5_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_TCON_MUX_S_R =				 IoReg_Read32(EPI_EPI_TCON_MUX_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_TCON_MUX1_S_R = 				 IoReg_Read32(EPI_EPI_TCON_MUX1_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_TCON_MUX2_S_R = 				 IoReg_Read32(EPI_EPI_TCON_MUX2_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_TCON_MUX3_S_R = 				 IoReg_Read32(EPI_EPI_TCON_MUX3_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_DUMMY_REG_S_R = 				 IoReg_Read32(EPI_EPI_DUMMY_REG_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_HW_DUMMY0_REG_S_R = 			 IoReg_Read32(EPI_EPI_HW_DUMMY0_REG_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_HW_DUMMY1_REG_S_R = 			 IoReg_Read32(EPI_EPI_HW_DUMMY1_REG_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_HW_DUMMY2_REG_S_R = 			 IoReg_Read32(EPI_EPI_HW_DUMMY2_REG_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_HW_DUMMY3_REG_S_R = 			 IoReg_Read32(EPI_EPI_HW_DUMMY3_REG_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CEDS_POL_REG_S_R =			 IoReg_Read32(EPI_EPI_CEDS_POL_REG_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_DATA_S_R = 			 IoReg_Read32(EPI_EPI_LANE_DST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_DATA_S_R = 			 IoReg_Read32(EPI_EPI_LANE_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE1_CST_DATA_S_R = 			 IoReg_Read32(EPI_EPI_LANE1_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE2_CST_DATA_S_R = 			 IoReg_Read32(EPI_EPI_LANE2_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE3_CST_DATA_S_R = 			 IoReg_Read32(EPI_EPI_LANE3_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE4_CST_DATA_S_R = 			 IoReg_Read32(EPI_EPI_LANE4_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE5_CST_DATA_S_R = 			 IoReg_Read32(EPI_EPI_LANE5_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE6_CST_DATA_S_R = 			 IoReg_Read32(EPI_EPI_LANE6_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE7_CST_DATA_S_R = 			 IoReg_Read32(EPI_EPI_LANE7_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE8_CST_DATA_S_R = 			 IoReg_Read32(EPI_EPI_LANE8_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE9_CST_DATA_S_R = 			 IoReg_Read32(EPI_EPI_LANE9_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE10_CST_DATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE10_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE11_CST_DATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE11_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE12_CST_DATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE12_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE13_CST_DATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE13_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE14_CST_DATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE14_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE15_CST_DATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE15_CST_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR4_DATA_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR4_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR5_DATA_S_R =			 IoReg_Read32(EPI_EPI_LANE_CTR5_DATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE0_CTR1_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE0_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE1_CTR1_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE1_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE2_CTR1_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE2_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE3_CTR1_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE3_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE4_CTR1_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE4_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE5_CTR1_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE5_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE6_CTR1_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE6_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE7_CTR1_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE7_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE8_CTR1_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE8_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE9_CTR1_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE9_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE10_CTR1_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE10_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE11_CTR1_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE11_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE12_CTR1_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE12_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE13_CTR1_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE13_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE14_CTR1_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE14_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE15_CTR1_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE15_CTR1_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE0_CTR2_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE0_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE1_CTR2_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE1_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE2_CTR2_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE2_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE3_CTR2_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE3_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE4_CTR2_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE4_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE5_CTR2_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE5_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE6_CTR2_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE6_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE7_CTR2_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE7_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE8_CTR2_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE8_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE9_CTR2_ODATA_S_R =		 IoReg_Read32(EPI_EPI_LANE9_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE10_CTR2_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE10_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE11_CTR2_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE11_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE12_CTR2_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE12_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE13_CTR2_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE13_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE14_CTR2_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE14_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE15_CTR2_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE15_CTR2_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE0_CTR3_ODATA_S_R =			 IoReg_Read32(EPI_EPI_LANE0_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE1_CTR3_ODATA_S_R =			 IoReg_Read32(EPI_EPI_LANE1_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE2_CTR3_ODATA_S_R =			 IoReg_Read32(EPI_EPI_LANE2_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE3_CTR3_ODATA_S_R =			 IoReg_Read32(EPI_EPI_LANE3_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE4_CTR3_ODATA_S_R =			 IoReg_Read32(EPI_EPI_LANE4_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE5_CTR3_ODATA_S_R =			 IoReg_Read32(EPI_EPI_LANE5_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE6_CTR3_ODATA_S_R =			 IoReg_Read32(EPI_EPI_LANE6_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE7_CTR3_ODATA_S_R =			 IoReg_Read32(EPI_EPI_LANE7_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE8_CTR3_ODATA_S_R =			 IoReg_Read32(EPI_EPI_LANE8_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE9_CTR3_ODATA_S_R =			 IoReg_Read32(EPI_EPI_LANE9_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE10_CTR3_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE10_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE11_CTR3_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE11_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE12_CTR3_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE12_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE13_CTR3_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE13_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE14_CTR3_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE14_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE15_CTR3_ODATA_S_R = 		 IoReg_Read32(EPI_EPI_LANE15_CTR3_ODATA_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_GPLUT_CONTROL_S_R = 			 IoReg_Read32(EPI_EPI_GPLUT_CONTROL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_GPLUT_LINE_01_S_R = 			 IoReg_Read32(EPI_EPI_GPLUT_LINE_01_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_GPLUT_LINE_23_S_R = 			 IoReg_Read32(EPI_EPI_GPLUT_LINE_23_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_GPLUT_LINE_45_S_R = 			 IoReg_Read32(EPI_EPI_GPLUT_LINE_45_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_GPLUT_LINE_67_S_R = 			 IoReg_Read32(EPI_EPI_GPLUT_LINE_67_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_00_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_00_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_01_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_01_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_02_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_02_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_03_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_03_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_04_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_04_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_05_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_05_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_06_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_06_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_07_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_07_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_08_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_08_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_09_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_09_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_10_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_10_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_11_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_11_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_12_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_12_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_13_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_13_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_14_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_14_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_15_S_R = 			 IoReg_Read32(EPI_EPI_CLK_PATTERN_15_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_DUMMY_CTRL_S_R = 				 IoReg_Read32(EPI_EPI_DUMMY_CTRL_reg);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_DUMMY_MP0_S_R = 				 IoReg_Read32(EPI_EPI_DUMMY_MP0_reg);

		//tcon
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON_CTRL_S_R = 					IoReg_Read32(TCON_TCON_CTRL_reg);

		//[KTASKWBS-8895] Tcon disable should be earlier than lane power OFF
		//add condition to protect the suspend value, prevent from call suspend twince let tcon setting as 0
		if(IoReg_Read32(TCON_TCON_IP_EN_reg)!=0){
			vbe_disp_panel_tcon_suspend_resume.TCON_TCON_IP_EN_S_R =					IoReg_Read32(TCON_TCON_IP_EN_reg);
		}
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_HSE_S_R = 					IoReg_Read32(TCON_TCON0_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_VSE_S_R = 					IoReg_Read32(TCON_TCON0_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_Ctrl_S_R =					IoReg_Read32(TCON_TCON0_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON0_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON0_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON0_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_HSE_S_R = 					IoReg_Read32(TCON_TCON1_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_VSE_S_R = 					IoReg_Read32(TCON_TCON1_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_Ctrl_S_R =					IoReg_Read32(TCON_TCON1_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON1_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON1_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON1_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_HSE_S_R = 					IoReg_Read32(TCON_TCON2_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_VSE_S_R = 					IoReg_Read32(TCON_TCON2_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_Ctrl_S_R =					IoReg_Read32(TCON_TCON2_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON2_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON2_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON2_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_HSE_S_R = 					IoReg_Read32(TCON_TCON3_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_VSE_S_R = 					IoReg_Read32(TCON_TCON3_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_Ctrl_S_R =					IoReg_Read32(TCON_TCON3_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON3_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON3_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON3_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_HSE_S_R = 					IoReg_Read32(TCON_TCON4_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_VSE_S_R = 					IoReg_Read32(TCON_TCON4_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_Ctrl_S_R =					IoReg_Read32(TCON_TCON4_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON4_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON4_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON4_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_HSE_S_R = 					IoReg_Read32(TCON_TCON5_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_VSE_S_R = 					IoReg_Read32(TCON_TCON5_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_Ctrl_S_R =					IoReg_Read32(TCON_TCON5_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON5_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON5_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON5_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_HSE_S_R = 					IoReg_Read32(TCON_TCON6_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_VSE_S_R = 					IoReg_Read32(TCON_TCON6_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_Ctrl_S_R =					IoReg_Read32(TCON_TCON6_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON6_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON6_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON6_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_HSE_S_R = 					IoReg_Read32(TCON_TCON7_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_VSE_S_R = 					IoReg_Read32(TCON_TCON7_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_Ctrl_S_R =					IoReg_Read32(TCON_TCON7_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON7_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON7_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON7_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_HSE_S_R = 					IoReg_Read32(TCON_TCON8_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_VSE_S_R = 					IoReg_Read32(TCON_TCON8_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_Ctrl_S_R =					IoReg_Read32(TCON_TCON8_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON8_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON8_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON8_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_HSE_S_R = 					IoReg_Read32(TCON_TCON9_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_VSE_S_R = 					IoReg_Read32(TCON_TCON9_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_Ctrl_S_R =					IoReg_Read32(TCON_TCON9_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON9_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON9_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON9_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_HSE_S_R =					IoReg_Read32(TCON_TCON10_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_VSE_S_R =					IoReg_Read32(TCON_TCON10_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_Ctrl_S_R =					IoReg_Read32(TCON_TCON10_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON10_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON10_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON10_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_HSE_S_R =					IoReg_Read32(TCON_TCON11_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_VSE_S_R =					IoReg_Read32(TCON_TCON11_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_Ctrl_S_R =					IoReg_Read32(TCON_TCON11_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON11_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON11_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON11_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON12_HSE_S_R =					IoReg_Read32(TCON_TCON12_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON12_VSE_S_R =					IoReg_Read32(TCON_TCON12_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON12_Ctrl_S_R =					IoReg_Read32(TCON_TCON12_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON12_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON12_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON12_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON12_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON12_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON12_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON13_HSE_S_R =					IoReg_Read32(TCON_TCON13_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON13_VSE_S_R =					IoReg_Read32(TCON_TCON13_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON13_Ctrl_S_R =					IoReg_Read32(TCON_TCON13_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON13_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON13_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON13_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON13_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON13_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON13_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON14_HSE_S_R =					IoReg_Read32(TCON_TCON14_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON14_VSE_S_R =					IoReg_Read32(TCON_TCON14_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON14_Ctrl_S_R =					IoReg_Read32(TCON_TCON14_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON14_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON14_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON14_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON14_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON14_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON14_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON15_HSE_S_R =					IoReg_Read32(TCON_TCON15_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON15_VSE_S_R =					IoReg_Read32(TCON_TCON15_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON15_Ctrl_S_R =					IoReg_Read32(TCON_TCON15_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON15_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON15_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON15_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON15_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON15_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON15_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON16_HSE_S_R =					IoReg_Read32(TCON_TCON16_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON16_VSE_S_R =					IoReg_Read32(TCON_TCON16_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON16_Ctrl_S_R =					IoReg_Read32(TCON_TCON16_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON16_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON16_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON16_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON16_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON16_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON16_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON17_HSE_S_R =					IoReg_Read32(TCON_TCON17_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON17_VSE_S_R =					IoReg_Read32(TCON_TCON17_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON17_Ctrl_S_R =					IoReg_Read32(TCON_TCON17_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON17_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON17_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON17_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON17_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON17_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON17_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON18_HSE_S_R =					IoReg_Read32(TCON_TCON18_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON18_VSE_S_R =					IoReg_Read32(TCON_TCON18_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON18_Ctrl_S_R =					IoReg_Read32(TCON_TCON18_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON18_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON18_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON18_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON18_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON18_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON18_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON19_HSE_S_R =					IoReg_Read32(TCON_TCON19_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON19_VSE_S_R =					IoReg_Read32(TCON_TCON19_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON19_Ctrl_S_R =					IoReg_Read32(TCON_TCON19_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON19_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON19_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON19_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON19_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON19_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON19_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON20_HSE_S_R =					IoReg_Read32(TCON_TCON20_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON20_VSE_S_R =					IoReg_Read32(TCON_TCON20_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON20_Ctrl_S_R =					IoReg_Read32(TCON_TCON20_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON20_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON20_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON20_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON20_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON20_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON20_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON21_HSE_S_R =					IoReg_Read32(TCON_TCON21_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON21_VSE_S_R =					IoReg_Read32(TCON_TCON21_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON21_Ctrl_S_R =					IoReg_Read32(TCON_TCON21_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON21_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON21_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON21_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON21_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON21_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON21_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON22_HSE_S_R =					IoReg_Read32(TCON_TCON22_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON22_VSE_S_R =					IoReg_Read32(TCON_TCON22_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON22_Ctrl_S_R =					IoReg_Read32(TCON_TCON22_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON22_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON22_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON22_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON22_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON22_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON22_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON23_HSE_S_R =					IoReg_Read32(TCON_TCON23_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON23_VSE_S_R =					IoReg_Read32(TCON_TCON23_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON23_Ctrl_S_R =					IoReg_Read32(TCON_TCON23_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON23_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON23_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON23_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON23_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON23_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON23_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON24_HSE_S_R =					IoReg_Read32(TCON_TCON24_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON24_VSE_S_R =					IoReg_Read32(TCON_TCON24_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON24_Ctrl_S_R =					IoReg_Read32(TCON_TCON24_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON24_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON24_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON24_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON24_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON24_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON24_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON25_HSE_S_R =					IoReg_Read32(TCON_TCON25_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON25_VSE_S_R =					IoReg_Read32(TCON_TCON25_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON25_Ctrl_S_R =					IoReg_Read32(TCON_TCON25_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON25_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON25_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON25_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON25_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON25_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON25_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON26_HSE_S_R =					IoReg_Read32(TCON_TCON26_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON26_VSE_S_R =					IoReg_Read32(TCON_TCON26_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON26_Ctrl_S_R =					IoReg_Read32(TCON_TCON26_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON26_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON26_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON26_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON26_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON26_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON26_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON27_HSE_S_R =					IoReg_Read32(TCON_TCON27_HSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON27_VSE_S_R =					IoReg_Read32(TCON_TCON27_VSE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON27_Ctrl_S_R =					IoReg_Read32(TCON_TCON27_Ctrl_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON27_ACROSS_LINE_CONTROL3_S_R =	IoReg_Read32(TCON_TCON27_ACROSS_LINE_CONTROL3_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON27_ACROSS_FRAME_CONTROL_S_R =	IoReg_Read32(TCON_TCON27_ACROSS_FRAME_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON27_FRAME_HL_CONTROL_S_R =		IoReg_Read32(TCON_TCON27_FRAME_HL_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_NO_SSCG_CONTROL_S_R =			IoReg_Read32(TCON_TCON0_NO_SSCG_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_no_SSCG_SE_S_R = 				IoReg_Read32(TCON_TCON0_no_SSCG_SE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_NO_SSCG_CONTROL_S_R =			IoReg_Read32(TCON_TCON1_NO_SSCG_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_no_SSCG_SE_S_R = 				IoReg_Read32(TCON_TCON1_no_SSCG_SE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_NO_SSCG_CONTROL_S_R =			IoReg_Read32(TCON_TCON2_NO_SSCG_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_no_SSCG_SE_S_R = 				IoReg_Read32(TCON_TCON2_no_SSCG_SE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_NO_SSCG_CONTROL_S_R =			IoReg_Read32(TCON_TCON3_NO_SSCG_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_no_SSCG_SE_S_R = 				IoReg_Read32(TCON_TCON3_no_SSCG_SE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_NO_SSCG_CONTROL_S_R =			IoReg_Read32(TCON_TCON4_NO_SSCG_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_no_SSCG_SE_S_R = 				IoReg_Read32(TCON_TCON4_no_SSCG_SE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_NO_SSCG_CONTROL_S_R =			IoReg_Read32(TCON_TCON5_NO_SSCG_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_no_SSCG_SE_S_R = 				IoReg_Read32(TCON_TCON5_no_SSCG_SE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_NO_SSCG_CONTROL_S_R =			IoReg_Read32(TCON_TCON6_NO_SSCG_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_no_SSCG_SE_S_R = 				IoReg_Read32(TCON_TCON6_no_SSCG_SE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_NO_SSCG_CONTROL_S_R =			IoReg_Read32(TCON_TCON7_NO_SSCG_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_no_SSCG_SE_S_R = 				IoReg_Read32(TCON_TCON7_no_SSCG_SE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_NO_SSCG_CONTROL_S_R =			IoReg_Read32(TCON_TCON8_NO_SSCG_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_no_SSCG_SE_S_R = 				IoReg_Read32(TCON_TCON8_no_SSCG_SE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_NO_SSCG_CONTROL_S_R =			IoReg_Read32(TCON_TCON9_NO_SSCG_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_no_SSCG_SE_S_R = 				IoReg_Read32(TCON_TCON9_no_SSCG_SE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_NO_SSCG_CONTROL_S_R =		IoReg_Read32(TCON_TCON10_NO_SSCG_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_no_SSCG_SE_S_R = 			IoReg_Read32(TCON_TCON10_no_SSCG_SE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_NO_SSCG_CONTROL_S_R =		IoReg_Read32(TCON_TCON11_NO_SSCG_CONTROL_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_no_SSCG_SE_S_R = 			IoReg_Read32(TCON_TCON11_no_SSCG_SE_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_ALIGN_N_S_R = 						IoReg_Read32(TCON_ALIGN_N_reg);
		vbe_disp_panel_tcon_suspend_resume.TCON_ALIGN_N_2_S_R = 					IoReg_Read32(TCON_ALIGN_N_2_reg);

		//[KTASKWBS-8895] Tcon disable should be earlier than lane power OFF
		if(vbe_suspend_instant_mode){	// Only STR need to control Tcon off
			IoReg_Write32(TCON_TCON_IP_EN_reg, 0);
			rtd_pr_vbe_emerg("[suspend]TCON_TCON_IP_EN_reg = 0!! \n");
		}
	}
    else if((Get_DISPLAY_PANEL_TYPE() == P_EDP_4K2K) ||
        (Get_DISPLAY_PANEL_TYPE() == P_EDP_4K2K_2PORT) ||
        (Get_DISPLAY_PANEL_TYPE() == P_EDP_3K1K) ||
        (Get_DISPLAY_PANEL_TYPE() == P_EDP_4K1K))
    {

    }
    
}

void vbe_disp_panel_resume(void)
{
	//panel driver setting
	IoReg_Write32(PPOVERLAY_Display_Background_Color_reg,vbe_disp_suspend_resume.PPOVERLAY_Display_Background_Color_S_R );
	IoReg_Write32(PPOVERLAY_Display_Background_Color_2_reg, vbe_disp_suspend_resume.PPOVERLAY_Display_Background_Color_2_S_R);
	IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, vbe_disp_suspend_resume.PPOVERLAY_Main_Display_Control_RSV_S_R);
	IoReg_Write32(PPOVERLAY_Main_Border_Highlight_Border_Color1_reg, vbe_disp_suspend_resume.PPOVERLAY_Main_Border_Highlight_Border_Color1_S_R);
	IoReg_Write32(PPOVERLAY_Main_Border_Highlight_Border_Color2_reg, vbe_disp_suspend_resume.PPOVERLAY_Main_Border_Highlight_Border_Color2_S_R);
	IoReg_Write32(PPOVERLAY_Main_Border_Highlight_Border_Color3_reg, vbe_disp_suspend_resume.PPOVERLAY_Main_Border_Highlight_Border_Color3_S_R);
	IoReg_Write32(PPOVERLAY_MAIN_DEN_H_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_MAIN_DEN_H_Start_End_S_R);
	IoReg_Write32(PPOVERLAY_MAIN_DEN_V_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_MAIN_DEN_V_Start_End_S_R);
	IoReg_Write32(PPOVERLAY_MAIN_Background_H_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_MAIN_Background_H_Start_End_S_R);
	IoReg_Write32(PPOVERLAY_MAIN_Background_V_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_MAIN_Background_V_Start_End_S_R);
	IoReg_Write32(PPOVERLAY_MAIN_Active_H_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_MAIN_Active_H_Start_End_S_R);
	IoReg_Write32(PPOVERLAY_MAIN_Active_V_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_MAIN_Active_V_Start_End_S_R);

	//sfg
	IoReg_Write32(SFG_SFG_ZZ_shift_dbg_CTRL_reg,				   vbe_disp_suspend_resume.SFG_SFG_ZZ_shift_dbg_CTRL_S_R);
	IoReg_Write32(SFG_SFG_SWAP_CTRL0_reg,						   vbe_disp_suspend_resume.SFG_SFG_SWAP_CTRL0_S_R);
	IoReg_Write32(SFG_SFG_CRC_CTRL_reg, 						   vbe_disp_suspend_resume.SFG_SFG_CRC_CTRL_S_R);
	IoReg_Write32(SFG_SFG_CRC_RESULT_reg,						   vbe_disp_suspend_resume.SFG_SFG_CRC_RESULT_S_R);
	IoReg_Write32(SFG_SFG_CRC_R_RESULT_reg, 					   vbe_disp_suspend_resume.SFG_SFG_CRC_R_RESULT_S_R);
	IoReg_Write32(SFG_SFG_L_DES_CRC_reg,						   vbe_disp_suspend_resume.SFG_SFG_L_DES_CRC_S_R);
	IoReg_Write32(SFG_SFG_R_DES_CRC_reg,						   vbe_disp_suspend_resume.SFG_SFG_R_DES_CRC_S_R);
	IoReg_Write32(SFG_SFG_CRC_ERR_CNT_reg,						   vbe_disp_suspend_resume.SFG_SFG_CRC_ERR_CNT_S_R);
	IoReg_Write32(SFG_SFG_FORCE_BG_AT_DIF_reg,					   vbe_disp_suspend_resume.SFG_SFG_FORCE_BG_AT_DIF_S_R);
	IoReg_Write32(SFG_ZZ_CTRL_reg,								   vbe_disp_suspend_resume.SFG_ZZ_CTRL_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_0A_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_0A_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_0B_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_0B_S_R);
	IoReg_Write32(SFG_SFG_CTRL_0B_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_0B_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_1A_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_1A_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_1B_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_1B_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_8_reg, 					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_8_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_10_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_10_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_93C_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_93C_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_B10_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B10_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_B14_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B14_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_B50_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B50_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_B54_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B54_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_B58_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B58_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_B5C_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B5C_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_B60_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B60_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_B64_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_B64_S_R);
	IoReg_Write32(SFG_SFG_CTRL_13_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_13_S_R);
	IoReg_Write32(SFG_SFG_CTRL_24_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_24_S_R);
	IoReg_Write32(SFG_SFG_CTRL_25_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_25_S_R);
	IoReg_Write32(SFG_SFG_CTRL_26_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_26_S_R);
	IoReg_Write32(SFG_SFG_CTRL_28_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_28_S_R);
	IoReg_Write32(SFG_SFG_CTRL_29_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_29_S_R);
	IoReg_Write32(SFG_SFG_CTRL_30_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_30_S_R);
	IoReg_Write32(SFG_SFG_CTRL_0_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_0_S_R);
	IoReg_Write32(SFG_SFG_CTRL_1_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_1_S_R);
	IoReg_Write32(SFG_SFG_CTRL_2_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_2_S_R);
	IoReg_Write32(SFG_SFG_CTRL_3_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_3_S_R);
	IoReg_Write32(SFG_SFG_CTRL_4_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_4_S_R);
	IoReg_Write32(SFG_SFG_START_CTRL_8_reg, 					   vbe_disp_suspend_resume.SFG_SFG_START_CTRL_8_S_R);
	IoReg_Write32(SFG_SFG_START_CTRL_10_reg,					   vbe_disp_suspend_resume.SFG_SFG_START_CTRL_10_S_R);
	IoReg_Write32(SFG_SFG_CTRL_11_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_11_S_R);
	IoReg_Write32(SFG_SFG_CTRL_12_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_12_S_R);
	IoReg_Write32(SFG_SFG_CTRL_6_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_6_S_R);
	IoReg_Write32(SFG_SFG_CTRL_7_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_7_S_R);
	IoReg_Write32(SFG_SFG_CTRL_8_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_8_S_R);
	IoReg_Write32(SFG_SFG_CTRL_9_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_9_S_R);
	IoReg_Write32(SFG_SFG_CTRL_5_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_5_S_R);
	IoReg_Write32(SFG_SFG_CTRL_A_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_A_S_R);
	IoReg_Write32(SFG_SFG_CTRL_B_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_B_S_R);
	IoReg_Write32(SFG_SFG_CTRL_C_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_C_S_R);
	IoReg_Write32(SFG_SFG_TG_AUTO_1_reg,						   vbe_disp_suspend_resume.SFG_SFG_TG_AUTO_1_S_R);
	IoReg_Write32(SFG_SFG_TG_AUTO_2_reg,						   vbe_disp_suspend_resume.SFG_SFG_TG_AUTO_2_S_R);
	IoReg_Write32(SFG_SFG_TG_AUTO_3_reg,						   vbe_disp_suspend_resume.SFG_SFG_TG_AUTO_3_S_R);
	IoReg_Write32(SFG_SFG_TG_AUTO_4_reg,						   vbe_disp_suspend_resume.SFG_SFG_TG_AUTO_4_S_R);
	IoReg_Write32(SFG_SFG_CLKEN_CTRL_reg,						   vbe_disp_suspend_resume.SFG_SFG_CLKEN_CTRL_S_R);
	IoReg_Write32(SFG_SFG_BIST_MODE_reg,						   vbe_disp_suspend_resume.SFG_SFG_BIST_MODE_S_R);
	IoReg_Write32(SFG_SFG_BIST_DONE_reg,						   vbe_disp_suspend_resume.SFG_SFG_BIST_DONE_S_R);
	IoReg_Write32(SFG_SFG_insert_subpixel_0_reg,				   vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_0_S_R);
	IoReg_Write32(SFG_SFG_insert_subpixel_1_reg,				   vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_1_S_R);
	IoReg_Write32(SFG_SFG_insert_subpixel_2_reg,				   vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_2_S_R);
	IoReg_Write32(SFG_SFG_insert_subpixel_3_reg,				   vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_3_S_R);
	IoReg_Write32(SFG_SFG_ctrl_10_reg,							   vbe_disp_suspend_resume.SFG_SFG_ctrl_10_S_R);
	IoReg_Write32(SFG_SFG_Last_ext_pixel_last_flip_reg, 		   vbe_disp_suspend_resume.SFG_SFG_Last_ext_pixel_last_flip_S_R);
	IoReg_Write32(SFG_SFG_CTRL_14_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_14_S_R);
	IoReg_Write32(SFG_SFG_CTRL_15_reg,							   vbe_disp_suspend_resume.SFG_SFG_CTRL_15_S_R);
	IoReg_Write32(SFG_SFG_debug_reg,							   vbe_disp_suspend_resume.SFG_SFG_debug_S_R);
	IoReg_Write32(SFG_SFG_data_capture_vh_reg,					   vbe_disp_suspend_resume.SFG_SFG_data_capture_vh_S_R);
	IoReg_Write32(SFG_SFG_data_capture_src_reg, 				   vbe_disp_suspend_resume.SFG_SFG_data_capture_src_S_R);
	IoReg_Write32(SFG_SFG_data_capture_read_data_reg,			   vbe_disp_suspend_resume.SFG_SFG_data_capture_read_data_S_R);
	IoReg_Write32(SFG_SFG_last_seg_0_reg,						   vbe_disp_suspend_resume.SFG_SFG_last_seg_0_S_R);
	IoReg_Write32(SFG_SFG_last_seg_1_reg,						   vbe_disp_suspend_resume.SFG_SFG_last_seg_1_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_0_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_0_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_1_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_1_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_2_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_2_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_6_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_6_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_7_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_7_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_8_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_8_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_12_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_12_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_13_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_13_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_14_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_14_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_18_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_18_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_19_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_19_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_20_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_20_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_3_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_3_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_4_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_4_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_5_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_5_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_9_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_9_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_10_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_10_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_11_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_11_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_15_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_15_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_16_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_16_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_17_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_17_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_21_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_21_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_22_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_22_S_R);
	IoReg_Write32(SFG_SFG_DRD_CTRL_23_reg,						   vbe_disp_suspend_resume.SFG_SFG_DRD_CTRL_23_S_R);
	IoReg_Write32(SFG_SFG_START_CTRL_27_reg,					   vbe_disp_suspend_resume.SFG_SFG_START_CTRL_27_S_R);
	IoReg_Write32(SFG_SFG_START_CTRL_28_reg,					   vbe_disp_suspend_resume.SFG_SFG_START_CTRL_28_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_29_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_29_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_CTRL_30_reg,					   vbe_disp_suspend_resume.SFG_SFG_DUMMY_CTRL_30_S_R);
	IoReg_Write32(SFG_SFG1_fw_ext_SUBPIXEL_reg, 				   vbe_disp_suspend_resume.SFG_SFG1_fw_ext_SUBPIXEL_S_R);
	IoReg_Write32(SFG_SFG1_fw_ext_SUBPIXEL2_reg,				   vbe_disp_suspend_resume.SFG_SFG1_fw_ext_SUBPIXEL2_S_R);
	IoReg_Write32(SFG_SFG1_DUM_DATA_LINE_0_reg, 				   vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_0_S_R);
	IoReg_Write32(SFG_SFG1_DUM_DATA_LINE_1_reg, 				   vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_1_S_R);
	IoReg_Write32(SFG_SFG1_DUM_DATA_LINE_2_reg, 				   vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_2_S_R);
	IoReg_Write32(SFG_SFG1_DUM_DATA_LINE_3_reg, 				   vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_3_S_R);
	IoReg_Write32(SFG_SFG1_DUM_DATA_LINE_4_reg, 				   vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_4_S_R);
	IoReg_Write32(SFG_SFG1_DUM_DATA_LINE_5_reg, 				   vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_5_S_R);
	IoReg_Write32(SFG_SFG1_DUM_DATA_LINE_6_reg, 				   vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_6_S_R);
	IoReg_Write32(SFG_SFG1_DUM_DATA_LINE_7_reg, 				   vbe_disp_suspend_resume.SFG_SFG1_DUM_DATA_LINE_7_S_R);
	IoReg_Write32(SFG_ZZ_CTRL_2_reg,							   vbe_disp_suspend_resume.SFG_ZZ_CTRL_2_S_R);
	IoReg_Write32(SFG_SFG_insert_subpixel_4_reg,				   vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_4_S_R);
	IoReg_Write32(SFG_SFG_insert_subpixel_5_reg,				   vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_5_S_R);
	IoReg_Write32(SFG_SFG_insert_subpixel_6_reg,				   vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_6_S_R);
	IoReg_Write32(SFG_SFG_insert_subpixel_7_reg,				   vbe_disp_suspend_resume.SFG_SFG_insert_subpixel_7_S_R);
	IoReg_Write32(SFG_SFG_Last_ext_pixel_last_flip_2_reg,		   vbe_disp_suspend_resume.SFG_SFG_Last_ext_pixel_last_flip_2_S_R);
	IoReg_Write32(SFG_SFG_last_seg_2_reg,						   vbe_disp_suspend_resume.SFG_SFG_last_seg_2_S_R);
	IoReg_Write32(SFG_SFG_last_seg_3_reg,						   vbe_disp_suspend_resume.SFG_SFG_last_seg_3_S_R);
	IoReg_Write32(SFG_SFG_patgen_ctrl_reg,						   vbe_disp_suspend_resume.SFG_SFG_patgen_ctrl_S_R);
	IoReg_Write32(SFG_SFG_patgen_0_reg, 						   vbe_disp_suspend_resume.SFG_SFG_patgen_0_S_R);
	IoReg_Write32(SFG_SFG_patgen_1_reg, 						   vbe_disp_suspend_resume.SFG_SFG_patgen_1_S_R);
	IoReg_Write32(SFG_SFG_patgen_2_reg, 						   vbe_disp_suspend_resume.SFG_SFG_patgen_2_S_R);
	IoReg_Write32(SFG_SFG_patgen_3_reg, 						   vbe_disp_suspend_resume.SFG_SFG_patgen_3_S_R);
	IoReg_Write32(SFG_SFG_patgen_4_reg, 						   vbe_disp_suspend_resume.SFG_SFG_patgen_4_S_R);
	IoReg_Write32(SFG_SFG_patgen_5_reg, 						   vbe_disp_suspend_resume.SFG_SFG_patgen_5_S_R);
	IoReg_Write32(SFG_SFG_patgen_6_reg, 						   vbe_disp_suspend_resume.SFG_SFG_patgen_6_S_R);
	IoReg_Write32(SFG_SFG_patgen_7_reg, 						   vbe_disp_suspend_resume.SFG_SFG_patgen_7_S_R);
	IoReg_Write32(SFG_SFG_patgen_8_reg, 						   vbe_disp_suspend_resume.SFG_SFG_patgen_8_S_R);
	IoReg_Write32(SFG_SFG_patgen_9_reg, 						   vbe_disp_suspend_resume.SFG_SFG_patgen_9_S_R);
	IoReg_Write32(SFG_SFG_patgen_10_reg,						   vbe_disp_suspend_resume.SFG_SFG_patgen_10_S_R);
	IoReg_Write32(SFG_SFG_patgen_11_reg,						   vbe_disp_suspend_resume.SFG_SFG_patgen_11_S_R);
	IoReg_Write32(SFG_SFG_patgen_12_reg,						   vbe_disp_suspend_resume.SFG_SFG_patgen_12_S_R);
	IoReg_Write32(SFG_SFG_irq_ctrl_0_reg,						   vbe_disp_suspend_resume.SFG_SFG_irq_ctrl_0_S_R);
	IoReg_Write32(SFG_SFG_irq_ctrl_1_reg,						   vbe_disp_suspend_resume.SFG_SFG_irq_ctrl_1_S_R);
	IoReg_Write32(SFG_Pat_det_window_H_ini_sta_width_reg,		   vbe_disp_suspend_resume.SFG_Pat_det_window_H_ini_sta_width_S_R);
	IoReg_Write32(SFG_Pat_det_window_V_ini_sta_height_reg,		   vbe_disp_suspend_resume.SFG_Pat_det_window_V_ini_sta_height_S_R);
	IoReg_Write32(SFG_Pattern_detect_CTRL_reg,					   vbe_disp_suspend_resume.SFG_Pattern_detect_CTRL_S_R);
	IoReg_Write32(SFG_Pat_det_ctrl_1_reg,						   vbe_disp_suspend_resume.SFG_Pat_det_ctrl_1_S_R);
	IoReg_Write32(SFG_Pat_det_ctrl_2_reg,						   vbe_disp_suspend_resume.SFG_Pat_det_ctrl_2_S_R);
	IoReg_Write32(SFG_Pat_det_ctrl_3_reg,						   vbe_disp_suspend_resume.SFG_Pat_det_ctrl_3_S_R);
	IoReg_Write32(SFG_Pat_det_ctrl_4_reg,						   vbe_disp_suspend_resume.SFG_Pat_det_ctrl_4_S_R);
	IoReg_Write32(SFG_Pat_det_ctrl_5_reg,						   vbe_disp_suspend_resume.SFG_Pat_det_ctrl_5_S_R);
	IoReg_Write32(SFG_Pat_det_ctrl_7_reg,						   vbe_disp_suspend_resume.SFG_Pat_det_ctrl_7_S_R);
	IoReg_Write32(SFG_Pat_det_ctrl_6_reg,						   vbe_disp_suspend_resume.SFG_Pat_det_ctrl_6_S_R);
	IoReg_Write32(SFG_Pat_det_acc11_reg,						   vbe_disp_suspend_resume.SFG_Pat_det_acc11_S_R);
	IoReg_Write32(SFG_Debug_pin_reg,							   vbe_disp_suspend_resume.SFG_Debug_pin_S_R);
	IoReg_Write32(SFG_SFG2_CTRL_0_reg,							   vbe_disp_suspend_resume.SFG_SFG2_CTRL_0_S_R);
	IoReg_Write32(SFG_SFG2_FORCE_0_reg, 						   vbe_disp_suspend_resume.SFG_SFG2_FORCE_0_S_R);
	IoReg_Write32(SFG_SFG2_FORCE_1_reg, 						   vbe_disp_suspend_resume.SFG_SFG2_FORCE_1_S_R);
	IoReg_Write32(SFG_SFG2_CRC_CTRL_reg,						   vbe_disp_suspend_resume.SFG_SFG2_CRC_CTRL_S_R);
	IoReg_Write32(SFG_SFG2_CRC_RESULT_reg,						   vbe_disp_suspend_resume.SFG_SFG2_CRC_RESULT_S_R);
	IoReg_Write32(SFG_SFG2_CRC_R_RESULT_reg,					   vbe_disp_suspend_resume.SFG_SFG2_CRC_R_RESULT_S_R);
	IoReg_Write32(SFG_SFG2_CRC_L_DES_CRC_reg,					   vbe_disp_suspend_resume.SFG_SFG2_CRC_L_DES_CRC_S_R);
	IoReg_Write32(SFG_SFG2_CRC_R_DES_CRC_reg,					   vbe_disp_suspend_resume.SFG_SFG2_CRC_R_DES_CRC_S_R);
	IoReg_Write32(SFG_SFG2_CRC_ERR_CNT_reg, 					   vbe_disp_suspend_resume.SFG_SFG2_CRC_ERR_CNT_S_R);
	IoReg_Write32(SFG_PIF_FIFO_CTRL0_reg,						   vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL0_S_R);
	IoReg_Write32(SFG_PIF_FIFO_CTRL1_reg,						   vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL1_S_R);
	IoReg_Write32(SFG_PIF_FIFO_CTRL2_reg,						   vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL2_S_R);
	IoReg_Write32(SFG_PIF_FIFO_CTRL3_reg,						   vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL3_S_R);
	IoReg_Write32(SFG_PIF_FIFO_CTRL4_reg,						   vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL4_S_R);
	IoReg_Write32(SFG_PIF_FIFO_CTRL5_reg,						   vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL5_S_R);
	IoReg_Write32(SFG_PIF_FIFO_CTRL6_reg,						   vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL6_S_R);
	IoReg_Write32(SFG_PIF_FIFO_MUX_CTRL0_reg,					   vbe_disp_suspend_resume.SFG_PIF_FIFO_MUX_CTRL0_S_R);
	IoReg_Write32(SFG_PIF_FIFO_MUX_CTRL1_reg,					   vbe_disp_suspend_resume.SFG_PIF_FIFO_MUX_CTRL1_S_R);
	IoReg_Write32(SFG_PIF_FIFO_MUX_CTRL2_reg,					   vbe_disp_suspend_resume.SFG_PIF_FIFO_MUX_CTRL2_S_R);
	IoReg_Write32(SFG_DE_HALF_CTL_reg,							   vbe_disp_suspend_resume.SFG_DE_HALF_CTL_S_R);
	IoReg_Write32(SFG_PIF_MISC_reg, 							   vbe_disp_suspend_resume.SFG_PIF_MISC_S_R);
	IoReg_Write32(SFG_SFG_EXT5_reg, 							   vbe_disp_suspend_resume.SFG_SFG_EXT5_S_R);
	IoReg_Write32(SFG_SFG_EXT6_reg, 							   vbe_disp_suspend_resume.SFG_SFG_EXT6_S_R);
	IoReg_Write32(SFG_SFG_EXT7_reg, 							   vbe_disp_suspend_resume.SFG_SFG_EXT7_S_R);
	IoReg_Write32(SFG_SFG_EXT8_reg, 							   vbe_disp_suspend_resume.SFG_SFG_EXT8_S_R);
	IoReg_Write32(SFG_SFG_EXT9_reg, 							   vbe_disp_suspend_resume.SFG_SFG_EXT9_S_R);
	IoReg_Write32(SFG_SFG_EXT10_reg,							   vbe_disp_suspend_resume.SFG_SFG_EXT10_S_R);
	IoReg_Write32(SFG_SFG_EXT11_reg,							   vbe_disp_suspend_resume.SFG_SFG_EXT11_S_R);
	IoReg_Write32(SFG_SFG_EXT12_reg,							   vbe_disp_suspend_resume.SFG_SFG_EXT12_S_R);
	IoReg_Write32(SFG_SFG_EXT13_reg,							   vbe_disp_suspend_resume.SFG_SFG_EXT13_S_R);
	IoReg_Write32(SFG_SFG_EXT14_reg,							   vbe_disp_suspend_resume.SFG_SFG_EXT14_S_R);
	IoReg_Write32(SFG_SFG_EXT15_reg,							   vbe_disp_suspend_resume.SFG_SFG_EXT15_S_R);
	IoReg_Write32(SFG_SFG_EXT16_reg,							   vbe_disp_suspend_resume.SFG_SFG_EXT16_S_R);
	IoReg_Write32(SFG_SFG_LC_reg,								   vbe_disp_suspend_resume.SFG_SFG_LC_S_R);
	IoReg_Write32(SFG_SFG_DUMMY_REG_reg,						   vbe_disp_suspend_resume.SFG_SFG_DUMMY_REG_S_R);

	//panel interface common
#if 0 // not use in Merlin8
	IoReg_Write32(PIF_LVDS_CTRL1_reg,			 vbe_disp_suspend_resume.PIF_LVDS_CTRL1_S_R);
	IoReg_Write32(PIF_LVDS_CTRL2_reg,			 vbe_disp_suspend_resume.PIF_LVDS_CTRL2_S_R);
	IoReg_Write32(PIF_LVDS_CTRL3_reg,			 vbe_disp_suspend_resume.PIF_LVDS_CTRL3_S_R);
	IoReg_Write32(PIF_LVDS_CTRL4_reg,			 vbe_disp_suspend_resume.PIF_LVDS_CTRL4_S_R);
	IoReg_Write32(PIF_LVDS_CTRL5_reg,			 vbe_disp_suspend_resume.PIF_LVDS_CTRL5_S_R);
	IoReg_Write32(PIF_MLVDS_CTRL0_reg,			 vbe_disp_suspend_resume.PIF_MLVDS_CTRL0_S_R);
	IoReg_Write32(PIF_MLVDS_Reset_Pul_CTRL_reg,  vbe_disp_suspend_resume.PIF_MLVDS_Reset_Pul_CTRL_S_R);
	IoReg_Write32(PIF_MLVDS_CTRL1_reg,			 vbe_disp_suspend_resume.PIF_MLVDS_CTRL1_S_R);
	IoReg_Write32(PIF_MLVDS_CTRL2_reg,			 vbe_disp_suspend_resume.PIF_MLVDS_CTRL2_S_R);
	IoReg_Write32(PIF_MLVDS_CTRL3_reg,			 vbe_disp_suspend_resume.PIF_MLVDS_CTRL3_S_R);
	IoReg_Write32(PIF_MLVDS_CTRL4_reg,			 vbe_disp_suspend_resume.PIF_MLVDS_CTRL4_S_R);
#endif

	//PIF APHY
	IoReg_Write32(PINMUX_LVDSPHY_PIF_3_reg , 					 vbe_disp_suspend_resume.PINMUX_LVDSPHY_PIF_3_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_PIF_4_reg , 					 vbe_disp_suspend_resume.PINMUX_LVDSPHY_PIF_4_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_PIF_5_reg , 					 vbe_disp_suspend_resume.PINMUX_LVDSPHY_PIF_5_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_PIF_6_reg , 					 vbe_disp_suspend_resume.PINMUX_LVDSPHY_PIF_6_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_Pin_Mux_LVDS_reg ,				 vbe_disp_suspend_resume.PINMUX_LVDSPHY_Pin_Mux_LVDS_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_0_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_0_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_1_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_1_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_2_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_2_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_3_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_3_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_3_2_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_3_2_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_4_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_4_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_4_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_4_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_18_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_18_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_27_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_27_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_5_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_5_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_0_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_0_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_4_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_4_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_8_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_8_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_12_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_12_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_16_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_16_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_20_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_20_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_24_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_24_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_28_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_28_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_32_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_32_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_36_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_36_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_40_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_40_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_44_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_44_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_48_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_48_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_52_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_52_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_56_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_56_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_60_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_60_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_1_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_1_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_5_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_5_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_9_reg ,	 vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_9_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_13_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_13_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_17_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_17_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_21_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_21_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_25_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_25_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_29_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_29_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_33_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_33_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_37_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_37_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_41_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_41_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_45_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_45_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_49_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_49_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_53_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_53_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_57_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_57_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_61_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_61_S_R);
#if 0 //don't control lane power here
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_S_R);
#endif
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_34_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_34_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_38_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_38_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_42_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_42_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_46_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_46_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_50_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_50_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_54_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_54_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_58_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_58_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_62_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_62_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_66_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_66_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_70_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_70_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_74_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_74_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_78_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_78_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_82_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_82_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_86_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_86_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_90_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_90_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_94_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_94_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_95_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_95_S_R);

	IoReg_Write32(PINMUX_LVDSPHY_AUX_APHY_CTRL_00_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_00_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_AUX_APHY_CTRL_01_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_01_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_AUX_APHY_CTRL_02_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_02_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_AUX_APHY_CTRL_03_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_03_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_AUX_APHY_CTRL_04_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_04_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_AUX_APHY_CTRL_05_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_05_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_AUX_APHY_CTRL_06_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_06_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_AUX_APHY_CTRL_07_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_07_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_AUX_APHY_CTRL_08_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_08_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_AUX_APHY_CTRL_09_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_AUX_APHY_CTRL_09_S_R);

	IoReg_Write32(PINMUX_LVDSPHY_CMU_SSC_CTRL_00_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_CMU_SSC_CTRL_00_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_CMU_SSC_CTRL_01_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_CMU_SSC_CTRL_01_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_CMU_SSC_CTRL_02_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_CMU_SSC_CTRL_02_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_CMU_SSC_CTRL_03_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_CMU_SSC_CTRL_03_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_CMU_SSC_CTRL_04_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_CMU_SSC_CTRL_04_S_R);
	IoReg_Write32(PINMUX_LVDSPHY_CMU_SSC_CTRL_08_reg ,  vbe_disp_suspend_resume.PINMUX_LVDSPHY_CMU_SSC_CTRL_08_S_R);


	if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_4K2K)  ||
		(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS) ||
		(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) ||
		(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI) ||
        (Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_eDP))
	{
		//VBy1 Mac
		IoReg_Write32(VBY1_VIDEO_FORMAT_reg ,	   vbe_disp_suspend_resume.VBY1_VIDEO_FORMAT_S_R);
		IoReg_Write32(VBY1_LANE_STATUS_reg , 	   vbe_disp_suspend_resume.VBY1_LANE_STATUS_S_R);
		IoReg_Write32(VBY1_LANE_STATUS_1_reg , 	   vbe_disp_suspend_resume.VBY1_LANE_STATUS_1_S_R);
		IoReg_Write32(VBY1_CTL_reg , 			   vbe_disp_suspend_resume.VBY1_CTL_S_R);
		IoReg_Write32(VBY1_TRANSMIT_CONTROL_reg ,   vbe_disp_suspend_resume.VBY1_TRANSMIT_CONTROL_S_R);
		IoReg_Write32(VBY1_TRANSMIT_STATUS_reg ,    vbe_disp_suspend_resume.VBY1_TRANSMIT_STATUS_S_R);
		IoReg_Write32(VBY1_THREE_FLAG_reg , 	   vbe_disp_suspend_resume.VBY1_THREE_FLAG_S_R);
		IoReg_Write32(VBY1_CRC_CONTROL_reg , 	   vbe_disp_suspend_resume.VBY1_CRC_CONTROL_S_R);
		IoReg_Write32(VBY1_CRC_RESULT_reg , 	   vbe_disp_suspend_resume.VBY1_CRC_RESULT_S_R);
		IoReg_Write32(VBY1_CRC_DES_reg , 		   vbe_disp_suspend_resume.VBY1_CRC_DES_S_R);
		IoReg_Write32(VBY1_CRC_ERROR_CNT_reg ,	   vbe_disp_suspend_resume.VBY1_CRC_ERROR_CNT_S_R);
		IoReg_Write32(VBY1_ASYNC_FIFO_RST_reg ,	   vbe_disp_suspend_resume.VBY1_ASYNC_FIFO_RST_S_R);
		IoReg_Write32(VBY1_ASYNC_FIFO_BIST_reg ,   vbe_disp_suspend_resume.VBY1_ASYNC_FIFO_BIST_S_R);
		IoReg_Write32(VBY1_VBY1_DEBUG_reg ,		   vbe_disp_suspend_resume.VBY1_VBY1_DEBUG_S_R);
		IoReg_Write32(VBY1_VBY1_STATUS_FW_CTRL_reg, vbe_disp_suspend_resume.VBY1_VBY1_STATUS_FW_CTRL_S_R);
		IoReg_Write32(VBY1_DATA_CAPTURE_reg ,	   vbe_disp_suspend_resume.VBY1_DATA_CAPTURE_S_R);
		IoReg_Write32(VBY1_DATA_CAPTURE_1_reg ,	   vbe_disp_suspend_resume.VBY1_DATA_CAPTURE_1_S_R);
		IoReg_Write32(VBY1_DATA_CAPTURE_2_reg ,	   vbe_disp_suspend_resume.VBY1_DATA_CAPTURE_2_S_R);
		IoReg_Write32(VBY1_DATA_CAPTURE_3_reg ,	   vbe_disp_suspend_resume.VBY1_DATA_CAPTURE_3_S_R);
		IoReg_Write32(VBY1_DBG_EN_reg ,			   vbe_disp_suspend_resume.VBY1_DBG_EN_S_R);
		IoReg_Write32(VBY1_DBG_DATA_reg ,		   vbe_disp_suspend_resume.VBY1_DBG_DATA_S_R);
		IoReg_Write32(VBY1_DPHY_CTL_reg ,		   vbe_disp_suspend_resume.VBY1_DPHY_CTL_S_R);
		IoReg_Write32(VBY1_DPHY_RD_reg , 		   vbe_disp_suspend_resume.VBY1_DPHY_RD_S_R);
		IoReg_Write32(VBY1_META_CTL_ACL_reg , 	   vbe_disp_suspend_resume.VBY1_META_CTL_ACL_S_R);
		IoReg_Write32(VBY1_META_CTL_ACL1_reg , 	   vbe_disp_suspend_resume.VBY1_META_CTL_ACL1_S_R);
		IoReg_Write32(VBY1_META_CTL_indicator_APL_reg , vbe_disp_suspend_resume.VBY1_META_CTL_indicator_APL_S_R);
		IoReg_Write32(VBY1_META_CTL_ACL_MP0_reg ,  vbe_disp_suspend_resume.VBY1_META_CTL_ACL_MP0_S_R);
		IoReg_Write32(VBY1_META_CTL_reg ,		   vbe_disp_suspend_resume.VBY1_META_CTL_S_R);
		IoReg_Write32(VBY1_META_CTL_1_reg ,		   vbe_disp_suspend_resume.VBY1_META_CTL_1_S_R);
		IoReg_Write32(VBY1_META_DATA_0_reg , 	   vbe_disp_suspend_resume.VBY1_META_DATA_0_S_R);
		IoReg_Write32(VBY1_META_DATA_1_reg , 	   vbe_disp_suspend_resume.VBY1_META_DATA_1_S_R);
		IoReg_Write32(VBY1_META_DATA_2_reg , 	   vbe_disp_suspend_resume.VBY1_META_DATA_2_S_R);
		IoReg_Write32(VBY1_META_DATA_3_reg , 	   vbe_disp_suspend_resume.VBY1_META_DATA_3_S_R);
		IoReg_Write32(VBY1_META_DATA_4_reg , 	   vbe_disp_suspend_resume.VBY1_META_DATA_4_S_R);
		IoReg_Write32(VBY1_META_DATA_5_reg , 	   vbe_disp_suspend_resume.VBY1_META_DATA_5_S_R);
		IoReg_Write32(VBY1_META_DATA_6_reg , 	   vbe_disp_suspend_resume.VBY1_META_DATA_6_S_R);
		IoReg_Write32(VBY1_META_DATA_7_reg , 	   vbe_disp_suspend_resume.VBY1_META_DATA_7_S_R);
		IoReg_Write32(VBY1_META_DATA_8_reg , 	   vbe_disp_suspend_resume.VBY1_META_DATA_8_S_R);
		IoReg_Write32(VBY1_META_DATA_9_reg , 	   vbe_disp_suspend_resume.VBY1_META_DATA_9_S_R);
		IoReg_Write32(VBY1_META_DATA_10_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_10_S_R);
		IoReg_Write32(VBY1_META_DATA_11_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_11_S_R);
		IoReg_Write32(VBY1_META_DATA_12_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_12_S_R);
		IoReg_Write32(VBY1_META_DATA_13_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_13_S_R);
		IoReg_Write32(VBY1_META_DATA_14_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_14_S_R);
		IoReg_Write32(VBY1_META_DATA_15_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_15_S_R);
		IoReg_Write32(VBY1_META_DATA_16_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_16_S_R);
		IoReg_Write32(VBY1_META_DATA_17_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_17_S_R);
		IoReg_Write32(VBY1_META_DATA_18_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_18_S_R);
		IoReg_Write32(VBY1_META_DATA_19_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_19_S_R);
		IoReg_Write32(VBY1_META_DATA_20_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_20_S_R);
		IoReg_Write32(VBY1_META_DATA_21_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_21_S_R);
		IoReg_Write32(VBY1_META_DATA_22_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_22_S_R);
		IoReg_Write32(VBY1_META_DATA_23_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_23_S_R);
		IoReg_Write32(VBY1_META_DATA_24_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_24_S_R);
		IoReg_Write32(VBY1_META_DATA_25_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_25_S_R);
		IoReg_Write32(VBY1_META_DATA_26_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_26_S_R);
		IoReg_Write32(VBY1_META_DATA_27_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_27_S_R);
		IoReg_Write32(VBY1_META_DATA_28_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_28_S_R);
		IoReg_Write32(VBY1_META_DATA_29_reg ,	   vbe_disp_suspend_resume.VBY1_META_DATA_29_S_R);
		// for OSD split
		if((Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_4K2K_OSD2K1K_to_H5X_SPLIT) ||(Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_4K2K_OSD1K2K_to_H5X_SPLIT)){
			IoReg_Write32(VBY1_OSD_OSD_VIDEO_FORMAT_reg, vbe_disp_suspend_resume.VBY1_OSD_OSD_VIDEO_FORMAT_S_R);
			IoReg_Write32(VBY1_OSD_OSD_LANE_STATUS_reg, vbe_disp_suspend_resume.VBY1_OSD_OSD_LANE_STATUS_S_R);
			IoReg_Write32(VBY1_OSD_OSD_CTL_reg, vbe_disp_suspend_resume.VBY1_OSD_OSD_CTL_S_R);
			IoReg_Write32(VBY1_OSD_OSD_TRANSMIT_CONTROL_reg, vbe_disp_suspend_resume.VBY1_OSD_OSD_TRANSMIT_CONTROL_S_R);
			IoReg_Write32(VBY1_OSD_OSD_THREE_FLAG_reg, vbe_disp_suspend_resume.VBY1_OSD_OSD_THREE_FLAG_S_R);
			IoReg_Write32(VBY1_OSD_OSD_CRC_CONTROL_reg, vbe_disp_suspend_resume.VBY1_OSD_OSD_CRC_CONTROL_S_R);
			IoReg_Write32(VBY1_OSD_OSD_VBY1_STATUS_FW_CTRL_reg, vbe_disp_suspend_resume.VBY1_OSD_OSD_VBY1_STATUS_FW_CTRL_S_R);
			IoReg_Write32(VBY1_OSD_OSD_DPHY_CTL_reg, vbe_disp_suspend_resume.VBY1_OSD_OSD_DPHY_CTL_S_R);
		}
	}
	else if((Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE)||(Get_DISPLAY_PANEL_TYPE() == P_CEDS))
	{
		IoReg_Write32(EPI_EPI_PACKET_CTR1_0_0_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_PACKET_CTR1_0_0_S_R);
		IoReg_Write32(EPI_EPI_PACKET_CTRL_reg , 			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_PACKET_CTRL_S_R);
		IoReg_Write32(EPI_EPI_CLOCK_CTRL0_reg , 			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLOCK_CTRL0_S_R);
		IoReg_Write32(EPI_EPI_DGB_READ_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_DGB_READ_S_R);
		IoReg_Write32(EPI_EPI_PORT_OPTION_CTRL_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_PORT_OPTION_CTRL_S_R);
		IoReg_Write32(EPI_EPI_CMPI_FRONT_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_FRONT_S_R);
		IoReg_Write32(EPI_CMPI_DUMMY_COLOR_0_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_CMPI_DUMMY_COLOR_0_S_R);
		IoReg_Write32(EPI_CMPI_DUMMY_COLOR_1_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_CMPI_DUMMY_COLOR_1_S_R);
		IoReg_Write32(EPI_CMPI_DUMMY_COLOR_2_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_CMPI_DUMMY_COLOR_2_S_R);
		IoReg_Write32(EPI_CMPI_DUMMY_COLOR_3_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_CMPI_DUMMY_COLOR_3_S_R);
		IoReg_Write32(EPI_EPI_CRC_CTRL_0_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CRC_CTRL_0_S_R);
		IoReg_Write32(EPI_EPI_CRC_CTRL_1_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CRC_CTRL_1_S_R);
		IoReg_Write32(EPI_EPI_CRC_CTRL_2_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CRC_CTRL_2_S_R);
		IoReg_Write32(EPI_EPI_DATA_SRC_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_DATA_SRC_S_R);
		IoReg_Write32(EPI_EPI_PACKET_CTR1_INV_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_PACKET_CTR1_INV_S_R);
		IoReg_Write32(EPI_EPI_PACKET_CTR1_INV_TABLE_1_reg ,  vbe_disp_panel_epi_suspend_resume.EPI_EPI_PACKET_CTR1_INV_TABLE_1_S_R);
		IoReg_Write32(EPI_EPI_CMPI_RLC_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_RLC_S_R);
		IoReg_Write32(EPI_EPI_CMPI_VBT_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_VBT_S_R);
		IoReg_Write32(EPI_EPI_CMPI_VBT_OPT_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_VBT_OPT_S_R);
		IoReg_Write32(EPI_EPI_SCR_TCON_RST_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR_TCON_RST_S_R);
		IoReg_Write32(EPI_EPI_SCR8_CTRL_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR8_CTRL_S_R);
		IoReg_Write32(EPI_EPI_SCR8_INI_VAL_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR8_INI_VAL_S_R);
		IoReg_Write32(EPI_EPI_SCR10_CTRL_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR10_CTRL_S_R);
		IoReg_Write32(EPI_EPI_SCR10_INI_VAL_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR10_INI_VAL_S_R);
		IoReg_Write32(EPI_EPI_SCR8CM_CTRL_reg , 			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR8CM_CTRL_S_R);
		IoReg_Write32(EPI_EPI_SCR8CM_INI_VAL_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR8CM_INI_VAL_S_R);
		IoReg_Write32(EPI_EPI_SCR10CM_CTRL_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR10CM_CTRL_S_R);
		IoReg_Write32(EPI_EPI_SCR10CM_INI_VAL_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SCR10CM_INI_VAL_S_R);
		IoReg_Write32(EPI_EPI_CMPI_PG_CTRL_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CTRL_S_R);
		IoReg_Write32(EPI_EPI_CMPI_PG_CMD_ST_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CMD_ST_S_R);
		IoReg_Write32(EPI_EPI_CMPI_PG_CMD_CMD1_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CMD_CMD1_S_R);
		IoReg_Write32(EPI_EPI_CMPI_PG_CMD_CMD2_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CMD_CMD2_S_R);
		IoReg_Write32(EPI_EPI_CMPI_PG_CMD_CMD3_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CMD_CMD3_S_R);
		IoReg_Write32(EPI_EPI_CMPI_PG_CMD_CMD4_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CMD_CMD4_S_R);
		IoReg_Write32(EPI_EPI_CMPI_PG_CMD_CMD5_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_CMD_CMD5_S_R);
		IoReg_Write32(EPI_EPI_CMPI_PG_START_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_START_S_R);
		IoReg_Write32(EPI_EPI_CMPI_PG_END_reg , 			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CMPI_PG_END_S_R);
		IoReg_Write32(EPI_EPI_LS_PORT_EN_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LS_PORT_EN_S_R);
		IoReg_Write32(EPI_EPI_FSM_SEL_reg , 				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_FSM_SEL_S_R);
		IoReg_Write32(EPI_EPI_DATA_PORT_EN_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_DATA_PORT_EN_S_R);
		IoReg_Write32(EPI_EPI_DEN_FALL_LOCA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_DEN_FALL_LOCA_S_R);
		IoReg_Write32(EPI_EPI_LOCK_DEBOUNCE_CTL_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LOCK_DEBOUNCE_CTL_S_R);
		IoReg_Write32(EPI_EPI_DBG_REG_FLAG_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_DBG_REG_FLAG_S_R);
		IoReg_Write32(EPI_EPI_DBG_REG_CTL_reg , 			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_DBG_REG_CTL_S_R);
		IoReg_Write32(EPI_EPI_SOE_CONTROL_reg , 			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_CONTROL_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE0_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE0_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE0_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE0_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE0_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE0_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE0_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE0_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE1_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE1_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE1_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE1_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE1_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE1_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE1_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE1_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE2_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE2_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE2_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE2_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE2_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE2_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE2_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE2_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE3_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE3_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE3_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE3_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE3_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE3_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE3_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE3_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE4_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE4_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE4_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE4_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE4_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE4_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE4_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE4_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE5_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE5_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE5_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE5_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE5_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE5_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE5_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE5_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE6_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE6_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE6_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE6_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE6_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE6_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE6_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE6_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE7_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE7_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE7_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE7_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE7_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE7_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE7_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE7_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE8_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE8_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE8_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE8_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE8_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE8_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE8_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE8_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE9_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE9_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE9_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE9_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE9_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE9_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE9_reg , 		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE9_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE10_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE10_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE10_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE10_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE10_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE10_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE10_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE10_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE11_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE11_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE11_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE11_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE11_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE11_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE11_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE11_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE12_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE12_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE12_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE12_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE12_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE12_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE12_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE12_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE13_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE13_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE13_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE13_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE13_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE13_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE13_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE13_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE14_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE14_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE14_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE14_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE14_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE14_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE14_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE14_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE0_LANE15_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE0_LANE15_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE1_LANE15_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE1_LANE15_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE2_LANE15_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE2_LANE15_S_R);
		IoReg_Write32(EPI_EPI_SOE_LINE3_LANE15_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_SOE_LINE3_LANE15_S_R);
		IoReg_Write32(EPI_EPI_LANE_DST_MUX0_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_MUX0_S_R);
		IoReg_Write32(EPI_EPI_LANE_DST_MUX1_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_MUX1_S_R);
		IoReg_Write32(EPI_EPI_LANE_DST_MUX2_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_MUX2_S_R);
		IoReg_Write32(EPI_EPI_LANE_DST_MUX3_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_MUX3_S_R);
		IoReg_Write32(EPI_EPI_LANE_DST_MUX4_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_MUX4_S_R);
		IoReg_Write32(EPI_EPI_LANE_DST_MUX5_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_MUX5_S_R);
		IoReg_Write32(EPI_EPI_LANE_CST_MUX0_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_MUX0_S_R);
		IoReg_Write32(EPI_EPI_LANE_CST_MUX1_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_MUX1_S_R);
		IoReg_Write32(EPI_EPI_LANE_CST_MUX2_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_MUX2_S_R);
		IoReg_Write32(EPI_EPI_LANE_CST_MUX3_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_MUX3_S_R);
		IoReg_Write32(EPI_EPI_LANE_CST_MUX4_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_MUX4_S_R);
		IoReg_Write32(EPI_EPI_LANE_CST_MUX5_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_MUX5_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR1_MUX0_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR1_MUX0_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR1_MUX1_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR1_MUX1_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR1_MUX2_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR1_MUX2_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR1_MUX3_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR1_MUX3_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR1_MUX4_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR1_MUX4_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR1_MUX5_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR1_MUX5_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR2_MUX0_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR2_MUX0_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR2_MUX1_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR2_MUX1_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR2_MUX2_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR2_MUX2_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR2_MUX3_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR2_MUX3_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR2_MUX4_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR2_MUX4_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR2_MUX5_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR2_MUX5_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR3_MUX0_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR3_MUX0_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR3_MUX1_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR3_MUX1_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR3_MUX2_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR3_MUX2_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR3_MUX3_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR3_MUX3_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR3_MUX4_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR3_MUX4_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR3_MUX5_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR3_MUX5_S_R);
		IoReg_Write32(EPI_EPI_TCON_MUX_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_TCON_MUX_S_R);
		IoReg_Write32(EPI_EPI_TCON_MUX1_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_TCON_MUX1_S_R);
		IoReg_Write32(EPI_EPI_TCON_MUX2_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_TCON_MUX2_S_R);
		IoReg_Write32(EPI_EPI_TCON_MUX3_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_TCON_MUX3_S_R);
		IoReg_Write32(EPI_EPI_DUMMY_REG_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_DUMMY_REG_S_R);
		IoReg_Write32(EPI_EPI_HW_DUMMY0_REG_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_HW_DUMMY0_REG_S_R);
		IoReg_Write32(EPI_EPI_HW_DUMMY1_REG_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_HW_DUMMY1_REG_S_R);
		IoReg_Write32(EPI_EPI_HW_DUMMY2_REG_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_HW_DUMMY2_REG_S_R);
		IoReg_Write32(EPI_EPI_HW_DUMMY3_REG_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_HW_DUMMY3_REG_S_R);
		IoReg_Write32(EPI_EPI_CEDS_POL_REG_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CEDS_POL_REG_S_R);
		IoReg_Write32(EPI_EPI_LANE_DST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_DST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE1_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE1_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE2_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE2_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE3_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE3_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE4_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE4_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE5_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE5_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE6_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE6_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE7_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE7_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE8_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE8_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE9_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE9_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE10_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE10_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE11_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE11_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE12_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE12_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE13_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE13_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE14_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE14_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE15_CST_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE15_CST_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR4_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR4_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE_CTR5_DATA_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE_CTR5_DATA_S_R);
		IoReg_Write32(EPI_EPI_LANE0_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE0_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE1_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE1_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE2_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE2_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE3_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE3_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE4_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE4_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE5_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE5_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE6_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE6_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE7_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE7_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE8_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE8_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE9_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE9_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE10_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE10_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE11_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE11_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE12_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE12_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE13_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE13_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE14_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE14_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE15_CTR1_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE15_CTR1_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE0_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE0_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE1_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE1_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE2_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE2_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE3_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE3_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE4_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE4_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE5_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE5_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE6_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE6_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE7_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE7_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE8_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE8_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE9_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE9_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE10_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE10_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE11_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE11_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE12_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE12_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE13_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE13_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE14_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE14_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE15_CTR2_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE15_CTR2_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE0_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE0_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE1_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE1_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE2_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE2_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE3_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE3_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE4_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE4_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE5_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE5_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE6_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE6_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE7_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE7_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE8_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE8_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE9_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE9_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE10_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE10_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE11_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE11_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE12_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE12_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE13_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE13_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE14_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE14_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_LANE15_CTR3_ODATA_reg ,		 vbe_disp_panel_epi_suspend_resume.EPI_EPI_LANE15_CTR3_ODATA_S_R);
		IoReg_Write32(EPI_EPI_GPLUT_CONTROL_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_GPLUT_CONTROL_S_R);
		IoReg_Write32(EPI_EPI_GPLUT_LINE_01_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_GPLUT_LINE_01_S_R);
		IoReg_Write32(EPI_EPI_GPLUT_LINE_23_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_GPLUT_LINE_23_S_R);
		IoReg_Write32(EPI_EPI_GPLUT_LINE_45_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_GPLUT_LINE_45_S_R);
		IoReg_Write32(EPI_EPI_GPLUT_LINE_67_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_GPLUT_LINE_67_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_00_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_00_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_01_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_01_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_02_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_02_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_03_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_03_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_04_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_04_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_05_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_05_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_06_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_06_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_07_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_07_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_08_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_08_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_09_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_09_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_10_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_10_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_11_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_11_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_12_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_12_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_13_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_13_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_14_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_14_S_R);
		IoReg_Write32(EPI_EPI_CLK_PATTERN_15_reg ,			 vbe_disp_panel_epi_suspend_resume.EPI_EPI_CLK_PATTERN_14_S_R);
		IoReg_Write32(EPI_EPI_DUMMY_CTRL_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_DUMMY_CTRL_S_R);
		IoReg_Write32(EPI_EPI_DUMMY_MP0_reg ,				 vbe_disp_panel_epi_suspend_resume.EPI_EPI_DUMMY_MP0_S_R);

		IoReg_Write32(TCON_TCON_CTRL_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON_CTRL_S_R);
		IoReg_Write32(TCON_TCON_IP_EN_reg,                  (vbe_disp_panel_tcon_suspend_resume.TCON_TCON_IP_EN_S_R)&(~_BIT0));
		IoReg_Write32(TCON_TCON0_HSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_HSE_S_R);
		IoReg_Write32(TCON_TCON0_VSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_VSE_S_R);
		IoReg_Write32(TCON_TCON0_Ctrl_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_Ctrl_S_R);
		IoReg_Write32(TCON_TCON0_ACROSS_LINE_CONTROL3_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON0_ACROSS_FRAME_CONTROL_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON0_FRAME_HL_CONTROL_reg,      vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON1_HSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_HSE_S_R);
		IoReg_Write32(TCON_TCON1_VSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_VSE_S_R);
		IoReg_Write32(TCON_TCON1_Ctrl_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_Ctrl_S_R);
		IoReg_Write32(TCON_TCON1_ACROSS_LINE_CONTROL3_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON1_ACROSS_FRAME_CONTROL_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON1_FRAME_HL_CONTROL_reg,      vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON2_HSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_HSE_S_R);
		IoReg_Write32(TCON_TCON2_VSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_VSE_S_R);
		IoReg_Write32(TCON_TCON2_Ctrl_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_Ctrl_S_R);
		IoReg_Write32(TCON_TCON2_ACROSS_LINE_CONTROL3_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON2_ACROSS_FRAME_CONTROL_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON2_FRAME_HL_CONTROL_reg,      vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON3_HSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_HSE_S_R);
		IoReg_Write32(TCON_TCON3_VSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_VSE_S_R);
		IoReg_Write32(TCON_TCON3_Ctrl_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_Ctrl_S_R);
		IoReg_Write32(TCON_TCON3_ACROSS_LINE_CONTROL3_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON3_ACROSS_FRAME_CONTROL_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON3_FRAME_HL_CONTROL_reg,      vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON4_HSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_HSE_S_R);
		IoReg_Write32(TCON_TCON4_VSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_VSE_S_R);
		IoReg_Write32(TCON_TCON4_Ctrl_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_Ctrl_S_R);
		IoReg_Write32(TCON_TCON4_ACROSS_LINE_CONTROL3_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON4_ACROSS_FRAME_CONTROL_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON4_FRAME_HL_CONTROL_reg,      vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON5_HSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_HSE_S_R);
		IoReg_Write32(TCON_TCON5_VSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_VSE_S_R);
		IoReg_Write32(TCON_TCON5_Ctrl_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_Ctrl_S_R);
		IoReg_Write32(TCON_TCON5_ACROSS_LINE_CONTROL3_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON5_ACROSS_FRAME_CONTROL_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON5_FRAME_HL_CONTROL_reg,      vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON6_HSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_HSE_S_R);
		IoReg_Write32(TCON_TCON6_VSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_VSE_S_R);
		IoReg_Write32(TCON_TCON6_Ctrl_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_Ctrl_S_R);
		IoReg_Write32(TCON_TCON6_ACROSS_LINE_CONTROL3_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON6_ACROSS_FRAME_CONTROL_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON6_FRAME_HL_CONTROL_reg,      vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON7_HSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_HSE_S_R);
		IoReg_Write32(TCON_TCON7_VSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_VSE_S_R);
		IoReg_Write32(TCON_TCON7_Ctrl_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_Ctrl_S_R);
		IoReg_Write32(TCON_TCON7_ACROSS_LINE_CONTROL3_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON7_ACROSS_FRAME_CONTROL_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON7_FRAME_HL_CONTROL_reg,      vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON8_HSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_HSE_S_R);
		IoReg_Write32(TCON_TCON8_VSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_VSE_S_R);
		IoReg_Write32(TCON_TCON8_Ctrl_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_Ctrl_S_R);
		IoReg_Write32(TCON_TCON8_ACROSS_LINE_CONTROL3_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON8_ACROSS_FRAME_CONTROL_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON8_FRAME_HL_CONTROL_reg,      vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON9_HSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_HSE_S_R);
		IoReg_Write32(TCON_TCON9_VSE_reg,                   vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_VSE_S_R);
		IoReg_Write32(TCON_TCON9_Ctrl_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_Ctrl_S_R);
		IoReg_Write32(TCON_TCON9_ACROSS_LINE_CONTROL3_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON9_ACROSS_FRAME_CONTROL_reg,  vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON9_FRAME_HL_CONTROL_reg,      vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON10_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_HSE_S_R);
		IoReg_Write32(TCON_TCON10_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_VSE_S_R);
		IoReg_Write32(TCON_TCON10_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_Ctrl_S_R);
		IoReg_Write32(TCON_TCON10_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON10_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON10_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON11_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_HSE_S_R);
		IoReg_Write32(TCON_TCON11_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_VSE_S_R);
		IoReg_Write32(TCON_TCON11_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_Ctrl_S_R);
		IoReg_Write32(TCON_TCON11_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON11_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON11_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON12_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON12_HSE_S_R);
		IoReg_Write32(TCON_TCON12_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON12_VSE_S_R);
		IoReg_Write32(TCON_TCON12_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON12_Ctrl_S_R);
		IoReg_Write32(TCON_TCON12_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON12_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON12_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON12_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON12_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON12_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON13_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON13_HSE_S_R);
		IoReg_Write32(TCON_TCON13_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON13_VSE_S_R);
		IoReg_Write32(TCON_TCON13_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON13_Ctrl_S_R);
		IoReg_Write32(TCON_TCON13_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON13_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON13_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON13_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON13_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON13_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON14_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON14_HSE_S_R);
		IoReg_Write32(TCON_TCON14_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON14_VSE_S_R);
		IoReg_Write32(TCON_TCON14_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON14_Ctrl_S_R);
		IoReg_Write32(TCON_TCON14_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON14_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON14_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON14_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON14_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON14_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON15_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON15_HSE_S_R);
		IoReg_Write32(TCON_TCON15_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON15_VSE_S_R);
		IoReg_Write32(TCON_TCON15_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON15_Ctrl_S_R);
		IoReg_Write32(TCON_TCON15_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON15_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON15_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON15_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON15_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON15_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON16_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON16_HSE_S_R);
		IoReg_Write32(TCON_TCON16_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON16_VSE_S_R);
		IoReg_Write32(TCON_TCON16_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON16_Ctrl_S_R);
		IoReg_Write32(TCON_TCON16_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON16_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON16_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON16_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON16_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON16_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON17_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON17_HSE_S_R);
		IoReg_Write32(TCON_TCON17_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON17_VSE_S_R);
		IoReg_Write32(TCON_TCON17_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON17_Ctrl_S_R);
		IoReg_Write32(TCON_TCON17_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON17_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON17_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON17_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON17_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON17_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON18_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON18_HSE_S_R);
		IoReg_Write32(TCON_TCON18_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON18_VSE_S_R);
		IoReg_Write32(TCON_TCON18_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON18_Ctrl_S_R);
		IoReg_Write32(TCON_TCON18_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON18_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON18_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON18_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON18_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON18_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON19_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON19_HSE_S_R);
		IoReg_Write32(TCON_TCON19_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON19_VSE_S_R);
		IoReg_Write32(TCON_TCON19_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON19_Ctrl_S_R);
		IoReg_Write32(TCON_TCON19_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON19_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON19_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON19_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON19_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON19_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON20_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON20_HSE_S_R);
		IoReg_Write32(TCON_TCON20_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON20_VSE_S_R);
		IoReg_Write32(TCON_TCON20_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON20_Ctrl_S_R);
		IoReg_Write32(TCON_TCON20_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON20_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON20_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON20_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON20_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON20_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON21_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON21_HSE_S_R);
		IoReg_Write32(TCON_TCON21_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON21_VSE_S_R);
		IoReg_Write32(TCON_TCON21_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON21_Ctrl_S_R);
		IoReg_Write32(TCON_TCON21_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON21_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON21_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON21_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON21_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON21_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON22_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON22_HSE_S_R);
		IoReg_Write32(TCON_TCON22_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON22_VSE_S_R);
		IoReg_Write32(TCON_TCON22_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON22_Ctrl_S_R);
		IoReg_Write32(TCON_TCON22_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON22_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON22_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON22_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON22_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON22_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON23_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON23_HSE_S_R);
		IoReg_Write32(TCON_TCON23_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON23_VSE_S_R);
		IoReg_Write32(TCON_TCON23_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON23_Ctrl_S_R);
		IoReg_Write32(TCON_TCON23_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON23_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON23_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON23_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON23_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON23_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON24_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON24_HSE_S_R);
		IoReg_Write32(TCON_TCON24_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON24_VSE_S_R);
		IoReg_Write32(TCON_TCON24_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON24_Ctrl_S_R);
		IoReg_Write32(TCON_TCON24_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON24_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON24_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON24_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON24_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON24_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON25_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON25_HSE_S_R);
		IoReg_Write32(TCON_TCON25_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON25_VSE_S_R);
		IoReg_Write32(TCON_TCON25_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON25_Ctrl_S_R);
		IoReg_Write32(TCON_TCON25_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON25_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON25_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON25_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON25_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON25_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON26_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON26_HSE_S_R);
		IoReg_Write32(TCON_TCON26_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON26_VSE_S_R);
		IoReg_Write32(TCON_TCON26_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON26_Ctrl_S_R);
		IoReg_Write32(TCON_TCON26_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON26_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON26_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON26_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON26_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON26_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON27_HSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON27_HSE_S_R);
		IoReg_Write32(TCON_TCON27_VSE_reg,                  vbe_disp_panel_tcon_suspend_resume.TCON_TCON27_VSE_S_R);
		IoReg_Write32(TCON_TCON27_Ctrl_reg,                 vbe_disp_panel_tcon_suspend_resume.TCON_TCON27_Ctrl_S_R);
		IoReg_Write32(TCON_TCON27_ACROSS_LINE_CONTROL3_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON27_ACROSS_LINE_CONTROL3_S_R);
		IoReg_Write32(TCON_TCON27_ACROSS_FRAME_CONTROL_reg, vbe_disp_panel_tcon_suspend_resume.TCON_TCON27_ACROSS_FRAME_CONTROL_S_R);
		IoReg_Write32(TCON_TCON27_FRAME_HL_CONTROL_reg,     vbe_disp_panel_tcon_suspend_resume.TCON_TCON27_FRAME_HL_CONTROL_S_R);
		IoReg_Write32(TCON_TCON0_NO_SSCG_CONTROL_reg,	    vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_NO_SSCG_CONTROL_S_R);
		IoReg_Write32(TCON_TCON0_no_SSCG_SE_reg,		    vbe_disp_panel_tcon_suspend_resume.TCON_TCON0_no_SSCG_SE_S_R);
		IoReg_Write32(TCON_TCON1_NO_SSCG_CONTROL_reg,	    vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_NO_SSCG_CONTROL_S_R);
		IoReg_Write32(TCON_TCON1_no_SSCG_SE_reg,		    vbe_disp_panel_tcon_suspend_resume.TCON_TCON1_no_SSCG_SE_S_R);
		IoReg_Write32(TCON_TCON2_NO_SSCG_CONTROL_reg,	    vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_NO_SSCG_CONTROL_S_R);
		IoReg_Write32(TCON_TCON2_no_SSCG_SE_reg,		    vbe_disp_panel_tcon_suspend_resume.TCON_TCON2_no_SSCG_SE_S_R);
		IoReg_Write32(TCON_TCON3_NO_SSCG_CONTROL_reg,	    vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_NO_SSCG_CONTROL_S_R);
		IoReg_Write32(TCON_TCON3_no_SSCG_SE_reg,		    vbe_disp_panel_tcon_suspend_resume.TCON_TCON3_no_SSCG_SE_S_R);
		IoReg_Write32(TCON_TCON4_NO_SSCG_CONTROL_reg,	    vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_NO_SSCG_CONTROL_S_R);
		IoReg_Write32(TCON_TCON4_no_SSCG_SE_reg,		    vbe_disp_panel_tcon_suspend_resume.TCON_TCON4_no_SSCG_SE_S_R);
		IoReg_Write32(TCON_TCON5_NO_SSCG_CONTROL_reg,	    vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_NO_SSCG_CONTROL_S_R);
		IoReg_Write32(TCON_TCON5_no_SSCG_SE_reg,		    vbe_disp_panel_tcon_suspend_resume.TCON_TCON5_no_SSCG_SE_S_R);
		IoReg_Write32(TCON_TCON6_NO_SSCG_CONTROL_reg,	    vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_NO_SSCG_CONTROL_S_R);
		IoReg_Write32(TCON_TCON6_no_SSCG_SE_reg,		    vbe_disp_panel_tcon_suspend_resume.TCON_TCON6_no_SSCG_SE_S_R);
		IoReg_Write32(TCON_TCON7_NO_SSCG_CONTROL_reg,	    vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_NO_SSCG_CONTROL_S_R);
		IoReg_Write32(TCON_TCON7_no_SSCG_SE_reg,		    vbe_disp_panel_tcon_suspend_resume.TCON_TCON7_no_SSCG_SE_S_R);
		IoReg_Write32(TCON_TCON8_NO_SSCG_CONTROL_reg,	    vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_NO_SSCG_CONTROL_S_R);
		IoReg_Write32(TCON_TCON8_no_SSCG_SE_reg,		    vbe_disp_panel_tcon_suspend_resume.TCON_TCON8_no_SSCG_SE_S_R);
		IoReg_Write32(TCON_TCON9_NO_SSCG_CONTROL_reg,	    vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_NO_SSCG_CONTROL_S_R);
		IoReg_Write32(TCON_TCON9_no_SSCG_SE_reg,		    vbe_disp_panel_tcon_suspend_resume.TCON_TCON9_no_SSCG_SE_S_R);
		IoReg_Write32(TCON_TCON10_NO_SSCG_CONTROL_reg,	    vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_NO_SSCG_CONTROL_S_R);
		IoReg_Write32(TCON_TCON10_no_SSCG_SE_reg,		    vbe_disp_panel_tcon_suspend_resume.TCON_TCON10_no_SSCG_SE_S_R);
		IoReg_Write32(TCON_TCON11_NO_SSCG_CONTROL_reg,	    vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_NO_SSCG_CONTROL_S_R);
		IoReg_Write32(TCON_TCON11_no_SSCG_SE_reg,		    vbe_disp_panel_tcon_suspend_resume.TCON_TCON11_no_SSCG_SE_S_R);
		IoReg_Write32(TCON_ALIGN_N_reg,					    vbe_disp_panel_tcon_suspend_resume.TCON_ALIGN_N_S_R);
		IoReg_Write32(TCON_ALIGN_N_2_reg,				    vbe_disp_panel_tcon_suspend_resume.TCON_ALIGN_N_2_S_R);

		vbe_disp_panel_rgbw_resume();
	}
    else if((Get_DISPLAY_PANEL_TYPE() == P_EDP_4K2K) ||
		(Get_DISPLAY_PANEL_TYPE() == P_EDP_4K2K_2PORT) ||
		(Get_DISPLAY_PANEL_TYPE() == P_EDP_3K1K) ||
		(Get_DISPLAY_PANEL_TYPE() == P_EDP_4K1K))
	{
		//------AUX0 Tx Ctrl-------
		IoReg_Write32(0xb81C616C, 0x00000003);  // Reset I2C/Native AUX FIFO[1:0]=0x3
		IoReg_Write32(0xb81C6180, 0x00000078);  // Disable Timeout Retry
		IoReg_Write32(0xb81C6188, 0x0000003F);  // Enable IRQ
		IoReg_Write32(0xb81C6184, 0x0000007F);  // CLR IRQ Flag
		IoReg_Write32(0xb81C6180, 0x000000F8);  // Enable Timeout Retry
		IoReg_Write32(0xb81C6150, 0x80010700);  // tx_len[3:0]=0xF, tx_addr[7:0]=0x00, tx_addr[15:8]=0x60, tx_addr[19:16]=0x00, tx_cmd[23:20]=0x8
		IoReg_Write32(0xb81C6154, 0x00000080);  // [7:0] Tx Data 
		IoReg_Write32(0xb81C614c, 0x00000021);  // bit 0 Set 1 to Transmit Aux Data And Then HW Auto, Clear When Finished
		mdelay(1);

		//------AUX1 Tx Ctrl-------
		IoReg_Write32(0xb81C716C, 0x00000003);  // Reset I2C/Native AUX FIFO[1:0]=0x3
		IoReg_Write32(0xb81C7180, 0x00000078);  // Disable Timeout Retry
		IoReg_Write32(0xb81C7188, 0x0000003F);  // Enable IRQ
		IoReg_Write32(0xb81C7184, 0x0000007F);  // CLR IRQ Flag
		IoReg_Write32(0xb81C7180, 0x000000F8);  // Enable Timeout Retry
		IoReg_Write32(0xb81C7150, 0x80010700);  // tx_len[3:0]=0xF, tx_addr[7:0]=0x00, tx_addr[15:8]=0x60, tx_addr[19:16]=0x00, tx_cmd[23:20]=0x8
		IoReg_Write32(0xb81C7154, 0x00000080);  // [7:0] Tx Data 
		IoReg_Write32(0xb81C714c, 0x00000021);  // bit 0 Set 1 to Transmit Aux Data And Then HW Auto, Clear When Finished
		mdelay(1);

		//------AUX2 Tx Ctrl-------
		IoReg_Write32(0xb81C656C, 0x00000003);  // Reset I2C/Native AUX FIFO[1:0]=0x3
		IoReg_Write32(0xb81C6580, 0x00000078);  // Disable Timeout Retry
		IoReg_Write32(0xb81C6588, 0x0000003F);  // Enable IRQ
		IoReg_Write32(0xb81C6584, 0x0000007F);  // CLR IRQ Flag
		IoReg_Write32(0xb81C6580, 0x000000F8);  // Enable Timeout Retry
		IoReg_Write32(0xb81C6550, 0x80010700);  // tx_len[3:0]=0xF, tx_addr[7:0]=0x00, tx_addr[15:8]=0x60, tx_addr[19:16]=0x00, tx_cmd[23:20]=0x8
		IoReg_Write32(0xb81C6554, 0x00000080);  // [7:0] Tx Data 
		IoReg_Write32(0xb81C654c, 0x00000021);  // bit 0 Set 1 to Transmit Aux Data And Then HW Auto, Clear When Finished
		mdelay(1);

		//------AUX3 Tx Ctrl-------
		IoReg_Write32(0xb81C756C, 0x00000003);  // Reset I2C/Native AUX FIFO[1:0]=0x3
		IoReg_Write32(0xb81C7580, 0x00000078);  // Disable Timeout Retry
		IoReg_Write32(0xb81C7588, 0x0000003F);  // Enable IRQ
		IoReg_Write32(0xb81C7584, 0x0000007F);  // CLR IRQ Flag
		IoReg_Write32(0xb81C7580, 0x000000F8);  // Enable Timeout Retry
		IoReg_Write32(0xb81C7550, 0x80010700);  // tx_len[3:0]=0xF, tx_addr[7:0]=0x00, tx_addr[15:8]=0x60, tx_addr[19:16]=0x00, tx_cmd[23:20]=0x8
		IoReg_Write32(0xb81C7554, 0x00000080);  // [7:0] Tx Data 
		IoReg_Write32(0xb81C754c, 0x00000021);  // bit 0 Set 1 to Transmit Aux Data And Then HW Auto, Clear When Finished
		mdelay(1);

		//---------------------------------------------------------------------------------------------------------------------------------
	    //HPTDN Pin share
	    //---------------------------------------------------------------------------------------------------------------------------------
	    IoReg_Mask32(0xb8000CBC, 0xFF00FFFF, 0x00000000);  // eDP_HTPD0= HTPD0_SRC0, eDP_HTPD1= HTPD1_SRC0, eDP_HTPD2= HTPD2_SRC0, eDP_HTPD3= HTPD3_SRC0
	    IoReg_Mask32(0xb800081C, 0xFFFFFF0F, 0x00000000);  // GPIO89= eDP_HTPD0_SRC0
	    IoReg_Mask32(0xb8000898, 0xFF0F0FFF, 0x00000000);  // GPIO96= eDP_HTPD1_SRC0, GPIO98= eDP_HTPD3_SRC0
	    IoReg_Mask32(0xb800088C, 0xFFFFFF0F, 0x00000000);  // GPIO65= eDP_HTPD2_SRC0
	    IoReg_Mask32(0xb8000830, 0x7FFFFFFF, 0x00000000);  // GPIO_89(eDP_HTPD0) use 3.3V GPI,
	    IoReg_Mask32(0xb80008A8, 0xBFFFFFFF, 0x00000000);  // GPIO_96(eDP_HTPD1) use 3.3V GPI,
	    
	    //HTPD0
	    IoReg_Write32(0xb81C63C0, 0x000000A8);   // 0.5m=0x34B,[7]DP_HTPD0=en,clk_div=/16,deb_lv=1
	    IoReg_Write32(0xb81C63CC, 0x00000036);   // MSB 0.5m=0x3,MSB 1=0x6
	    IoReg_Write32(0xb81C63D0, 0x0000004B);   // LSB 0.5m=0x4B,
	    IoReg_Write32(0xb81C63D4, 0x00000096);   // LSB   1m=0x96,
	    IoReg_Write32(0xb81C63D8, 0x0000000D);   // MSB   2m=0xD,
	    IoReg_Write32(0xb81C63DC, 0x0000002C);   // LSB   2m=0x2C,
	    IoReg_Write32(0xb81C63E0, 0x00000041);   // MSB   10m=0x41,
	    IoReg_Write32(0xb81C63E4, 0x000000DC);   // LSB   10m=0xDC,
	    
	    //HTPD1
	    IoReg_Write32(0xb81C73C0, 0x000000A8);   // 0.5m=0x34B,[7]DP_HTPD1=en,clk_div=/16,deb_lv=1
	    IoReg_Write32(0xb81C73CC, 0x00000036);   // MSB 0.5m=0x3,MSB 1=0x6
	    IoReg_Write32(0xb81C73D0, 0x0000004B);   // LSB 0.5m=0x4B,
	    IoReg_Write32(0xb81C73D4, 0x00000096);   // LSB   1m=0x96,
	    IoReg_Write32(0xb81C73D8, 0x0000000D);   // MSB   2m=0xD,
	    IoReg_Write32(0xb81C73DC, 0x0000002C);   // LSB   2m=0x2C,
	    IoReg_Write32(0xb81C73E0, 0x00000041);   // MSB   10m=0x41,
	    IoReg_Write32(0xb81C73E4, 0x000000DC);   // LSB   10m=0xDC,
	    
	    //HTPD2
	    IoReg_Write32(0xb81C67C0, 0x000000A8);   // 0.5m=0x34B,[7]DP_HTPD0=en,clk_div=/16,deb_lv=1
	    IoReg_Write32(0xb81C67CC, 0x00000036);   // MSB 0.5m=0x3,MSB 1=0x6
	    IoReg_Write32(0xb81C67D0, 0x0000004B);   // LSB 0.5m=0x4B,
	    IoReg_Write32(0xb81C67D4, 0x00000096);   // LSB   1m=0x96,
	    IoReg_Write32(0xb81C67D8, 0x0000000D);   // MSB   2m=0xD,
	    IoReg_Write32(0xb81C67DC, 0x0000002C);   // LSB   2m=0x2C,
	    IoReg_Write32(0xb81C67E0, 0x00000041);   // MSB   10m=0x41,
	    IoReg_Write32(0xb81C67E4, 0x000000DC);   // LSB   10m=0xDC,
	    
	    //HTPD3
	    IoReg_Write32(0xb81C77C0, 0x000000A8);   // 0.5m=0x34B,[7]DP_HTPD1=en,clk_div=/16,deb_lv=1
	    IoReg_Write32(0xb81C77CC, 0x00000036);   // MSB 0.5m=0x3,MSB 1=0x6
	    IoReg_Write32(0xb81C77D0, 0x0000004B);   // LSB 0.5m=0x4B,
	    IoReg_Write32(0xb81C77D4, 0x00000096);   // LSB   1m=0x96,
	    IoReg_Write32(0xb81C77D8, 0x0000000D);   // MSB   2m=0xD,
	    IoReg_Write32(0xb81C77DC, 0x0000002C);   // LSB   2m=0x2C,
	    IoReg_Write32(0xb81C77E0, 0x00000041);   // MSB   10m=0x41,
	    IoReg_Write32(0xb81C77E4, 0x000000DC);   // LSB   10m=0xDC,
	    
	    //---------------------------------------------------------------------------------------------------------------------------------
	    //AUX Pin share
	    //---------------------------------------------------------------------------------------------------------------------------------	    
	    IoReg_Mask32(0xb8000898, 0xFFFFFF0F, 0x000000C0);  // GPIO99= AUX_TX3P 
	    IoReg_Mask32(0xb800089C, 0x0F0F0F0F, 0xC0C0C0C0);  // GPIO100= AUX_TX3N, GPIO101= AUX_TX2P, GPIO102= AUX_TX2N, GPIO103= AUX_TX1P 
	    IoReg_Mask32(0xb80008A0, 0x0F0F0FFF, 0xC0C0C000);  // GPIO104= AUX_TX1N, GPIO105= AUX_TX0P, GPIO106= AUX_TX0N 

		//---------------------------------------------------------------------------------------------------------------------------------
    	//eDP Dphy Common
    	//---------------------------------------------------------------------------------------------------------------------------------
    	IoReg_Write32(0xb802DF00, 0xFFFF000F); // [31:16] phy_lane_en=0xFFFF, [3:0]=0xF mac0/1/2/3 20b to 10b enable 
    	IoReg_Write32(0xb802DF04, 0x0000FFFF); // [15:0] phy lane 15~0 pn_swap 
    	IoReg_Write32(0xb802DF10, 0x03020100); // phy lane 3/2/1/0 pair     select from mac0 3/2/1/0 pair 
    	IoReg_Write32(0xb802DF14, 0x03020100); // phy lane 7/6/5/4 pair     select from mac0 3/2/1/0 pair
    	IoReg_Write32(0xb802DF18, 0x03020100); // phy lane 11/10/9/8 pair   select from mac0 3/2/1/0 pair
    	IoReg_Write32(0xb802DF1C, 0x03020100); // phy lane 15/14/13/12 pair select from mac0 3/2/1/0 pair


		//---------------------------------------------------------------------------------------------------------------------------------
	    //TX0_Dphy
	    //---------------------------------------------------------------------------------------------------------------------------------
	    IoReg_Write32(0xb81C6200, 0x0000000C);  //[7:0]: 4 lane
	    IoReg_Write32(0xb81C6204, 0x00000042);  //[7:0]: Video pattern, DB enable
	                                       //[31:8]: Reserved
	    //IoReg_Write32(0xb81C6208, 0x830F3EF8);  //80b customer pat   
	    //IoReg_Write32(0xb81C620C, 0x0F3EF8E0);  //80b customer pat                                 
	    //IoReg_Write32(0xb81C6210, 0x0000E083);  //80b customer pat 
	    //IoReg_Write32(0xb81C6230, 0x00000000);  //compliance eye pat1 
	    IoReg_Write32(0xb81C6234, 0x00000015);  //insert_en=1, skew_en, normal scb, scb_en
	    IoReg_Write32(0xb81C6238, 0x0000001B);  //[7:0] lane swap, lane0=0,lane1=1,lane2=2,lane3=3  
	    IoReg_Write32(0xb81C623C, 0x00000010);  //[7:0] sr_insert_per_MSB(total=512), tst_mode=0;
	    IoReg_Write32(0xb81C6240, 0x000000FF);  //[7:0] sr_insert_per_LSB(total=512)
	    IoReg_Write32(0xb81C6244, 0x00000000);  //[7:0] lane1/lane0: skew no delay
	    IoReg_Write32(0xb81C6248, 0x00000000);  //[7:0] lane3/lane2: skew no delay
	    
	    //---------------------------------------------------------------------------------------------------------------------------------
	    //TX1_Dphy
	    //---------------------------------------------------------------------------------------------------------------------------------
	    IoReg_Write32(0xb81C7200, 0x0000000C);  //[7:0]: 4 lane
	    IoReg_Write32(0xb81C7204, 0x00000042);  //[7:0]: Video pat, DB enable
	                                       //[31:8]: Reserved
	    //IoReg_Write32(0xb81C7208, 0x830F3EF8);  //80b customer pat   
	    //IoReg_Write32(0xb81C720C, 0x0F3EF8E0);  //80b customer pat                                 
	    //IoReg_Write32(0xb81C7210, 0x0000E083);  //80b customer pat 
	    //IoReg_Write32(0xb81C7230, 0x00000000);  //compliance eye pat1 
	    IoReg_Write32(0xb81C7234, 0x00000015);  //insert_en=1, skew_en, normal scb, scb_en
	    IoReg_Write32(0xb81C7238, 0x0000001B);  //[7:0] lane swap, lane0=0,lane1=1,lane2=2,lane3=3  
	    IoReg_Write32(0xb81C723C, 0x00000010);  //[7:0] sr_insert_per_MSB(total=512), tst_mode=0;
	    IoReg_Write32(0xb81C7240, 0x000000FF);  //[7:0] sr_insert_per_LSB(total=512)
	    IoReg_Write32(0xb81C7244, 0x00000000);  //[7:0] lane1/lane0: skew no delay
	    IoReg_Write32(0xb81C7248, 0x00000000);  //[7:0] lane3/lane2: skew no delay
	    
	    //---------------------------------------------------------------------------------------------------------------------------------
	    //TX2_Dphy
	    //---------------------------------------------------------------------------------------------------------------------------------
	    IoReg_Write32(0xb81C6600, 0x0000000C);  //[7:0]: 4 lane
	    IoReg_Write32(0xb81C6604, 0x00000042);  //[7:0]: Video pattern, DB enable
	                                       //[31:8]: Reserved
	    //IoReg_Write32(0xb81C6608, 0x830F3EF8);  //80b customer pat   
	    //IoReg_Write32(0xb81C660C, 0x0F3EF8E0);  //80b customer pat                                 
	    //IoReg_Write32(0xb81C6610, 0x0000E083);  //80b customer pat 
	    //IoReg_Write32(0xb81C6630, 0x00000000);  //compliance eye pat1 
	    IoReg_Write32(0xb81C6634, 0x00000015);  //insert_en=1, skew_en, normal scb, scb_en
	    IoReg_Write32(0xb81C6638, 0x0000001B);  //[7:0] lane swap, lane0=0,lane1=1,lane2=2,lane3=3  
	    IoReg_Write32(0xb81C663C, 0x00000010);  //[7:0] sr_insert_per_MSB(total=512), tst_mode=0;
	    IoReg_Write32(0xb81C6640, 0x000000FF);  //[7:0] sr_insert_per_LSB(total=512)
	    IoReg_Write32(0xb81C6644, 0x00000000);  //[7:0] lane1/lane0: skew no delay
	    IoReg_Write32(0xb81C6648, 0x00000000);  //[7:0] lane3/lane2: skew no delay
	    
	    //---------------------------------------------------------------------------------------------------------------------------------
	    //TX3_Dphy
	    //---------------------------------------------------------------------------------------------------------------------------------
	    IoReg_Write32(0xb81C7600, 0x0000000C);  //[7:0]: 4 lane
	    IoReg_Write32(0xb81C7604, 0x00000042);  //[7:0]: Video pat, DB enable
	                                       //[31:8]: Reserved
	    //IoReg_Write32(0xb81C7608, 0x830F3EF8);  //80b customer pat   
	    //IoReg_Write32(0xb81C760C, 0x0F3EF8E0);  //80b customer pat                                 
	    //IoReg_Write32(0xb81C7610, 0x0000E083);  //80b customer pat 
	    //IoReg_Write32(0xb81C7630, 0x00000000);  //compliance eye pat1 
	    IoReg_Write32(0xb81C7634, 0x00000015);  //insert_en=1, skew_en, normal scb, scb_en
	    IoReg_Write32(0xb81C7638, 0x0000001B);  //[7:0] lane swap, lane0=0,lane1=1,lane2=2,lane3=3  
	    IoReg_Write32(0xb81C763C, 0x00000010);  //[7:0] sr_insert_per_MSB(total=512), tst_mode=0;
	    IoReg_Write32(0xb81C7640, 0x000000FF);  //[7:0] sr_insert_per_LSB(total=512)
	    IoReg_Write32(0xb81C7644, 0x00000000);  //[7:0] lane1/lane0: skew no delay
	    IoReg_Write32(0xb81C7648, 0x00000000);  //[7:0] lane3/lane2: skew no delay

		//---------------------------------------------------------------------------------------------------------------------------------
		//eDP Mac
		//---------------------------------------------------------------------------------------------------------------------------------
		if(Get_DISPLAY_PANEL_CUSTOM_INDEX() == P_EDP_P0_QHD_165_648M) {
			//---------------------------------------------------------------------------------------------------------------------------------
			//TX0_MAC
			//---------------------------------------------------------------------------------------------------------------------------------
			IoReg_Write32(0xb81C6300, 0x000000C7);		// [7:0]:MSO=4 lane, enhance mode=EN, 4-lane
			IoReg_Write32(0xb81C6304, 0x00000000);		// [7:0]:Pg_rst=0,msa_rst=0  
			IoReg_Write32(0xb81C6308, 0x00000000);		// [7:0]:debug sel
			IoReg_Write32(0xb81C630C, 0x00000000);		// [7:0]:irq disable
			IoReg_Write32(0xb81C631C, 0x00000000);		// IE disable
			IoReg_Write32(0xb81C6320, 0x00000000);		// [7] MN_vid auto=dis, [6]:db, [0]:popup  
			IoReg_Write32(0xb81C6324, 0x000025D7);		// [23:0]M_vid=0x0025D7
			IoReg_Write32(0xb81C6328, 0x00002000);		// [23:0]N_vid=0x002000 
			IoReg_Write32(0xb81C6330, 0x00000000);		// [7:0] N_mul=0;  
			IoReg_Write32(0xb81C6320, 0x00000040);		// [7] MN_vid auto=dis, [6]:db, [0]:popup C

			//// MSA								  
			IoReg_Write32(0xb81C6338, 0x00000040);		// [7] msa_db=0 [6]=1 just before MSA 
			IoReg_Write32(0xb81C633C, 0x00000020);		// [14]:color format follow misc1.6, [13]ycc_col=601, [12]VESA, [11]Async,[10:9] no stereo, [8]interlace odd num,
												   // [7:5]:color bit=8 bit [4:3]format=RGB, 
			IoReg_Write32(0xb81C6340, 0x00000A50);		// [15:0]:Htt =2640   
			IoReg_Write32(0xb81C6344, 0x00000030);		// [15:0]:Hsta =48
			IoReg_Write32(0xb81C6348, 0x00000A00);		// [15:0]:Hwidth =2560
			IoReg_Write32(0xb81C634C, 0x0000003F);		// [15]HSP=low, [14:0]:Hsw =64
			IoReg_Write32(0xb81C6350, 0x000005BA);		// [15:0]:Vtt =1466 	
			IoReg_Write32(0xb81C6354, 0x0000000D);		// [15:0]:Vsta =13 
			IoReg_Write32(0xb81C6358, 0x000005A0);		// [15:0]:Vhight =1440		  
			IoReg_Write32(0xb81C635C, 0x00000005);		// [15]VSP=low, [14:0]:Vsw =5	 

			//// VBID								  
			IoReg_Write32(0xb81C6360, 0x00000000);		// [2]audiomute_flag [1]novideo_flag
			IoReg_Write32(0xb81C6364, 0x00000000);		// [1]interlace=0 [0]vbid_fw_ctrl=HW mode

			//// ARBITER							  
			IoReg_Write32(0xb81C6368, 0x00000012);		// [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode [0]arbiter_en=0
			IoReg_Write32(0xb81C636C, 0x00000780);		// [15:0]vdata_per_line=0x0780 (1920)
												   //  vdata_per_line==> Hwidth * BPC (bit per component) * 3 / 8 (byte) = lane_num * v_data_per_line (byte)  
			IoReg_Write32(0xb81C6370, 0x00000040);		// [7]TU_size<1,[6:0]:tu_size=0x40 (64)    
			IoReg_Write32(0xb81C6374, 0x000001D6);		// [9:3]tu_data_size=0x34<<3=1A6, [2:0]=0.8=0x6
												   // tu_data_size =-> SCLK(freq) * BPC * 3 /8 (byte)= (TU_DATA_SIZE/TU) * LCLK(freq) * LANE_NUM (byte) 
			IoReg_Write32(0xb81C6378, 0x000000C8);		// [15:0]:hdelay								  
			IoReg_Write32(0xb81C637C, 0x00000064);		// [7:0]:sec_end_cnt MSB[15:8] LSB[7:0]
			IoReg_Write32(0xb81C63AC, 0x00000046);		// [7:0]:sec_idel_end_cnt  
			IoReg_Write32(0xb81C6380, 0x00000080);		// sec_end db apply

			////TX_TOP
			IoReg_Write32(0xb81C638C, 0x00000000);		// I2V_first_blank_filter[1], idel2Vdieo switch=cnt to 8191
			IoReg_Write32(0xb81C6390, 0x00000000);		// RSV1
			IoReg_Write32(0xb81C6394, 0x00000000);		// RSV2

			////Arbiter
			IoReg_Write32(0xb81C6398, 0x00008000);		// (Read_only) [15]wclr [14:0]min_h_blank_wid
			IoReg_Write32(0xb81C639C, 0x00000000);		// IE disable

			////Vesa
			IoReg_Write32(0xb81C63A0, 0x00000000);		// align Vsync and Hsync = disable
			IoReg_Write32(0xb81C63B0, 0x00000013);		// [3] FW M/N code [0] using source's field signal
			IoReg_Write32(0xb81C63B8, 0x000000FF);		// [0]=1 Ref BE to gen VBID[0]

			////DB apply
			IoReg_Write32(0xb81C6338, 0x000000C0);		// [7] msa_db=1 [6]=0 just before MSA 
			IoReg_Write32(0xb81C6368, 0x00000013);		// [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode 
												   // [0]arbiter_en=1 
			//---------------------------------------------------------------------------------------------------------------------------------
			//TX1_MAC
			//---------------------------------------------------------------------------------------------------------------------------------
			IoReg_Write32(0xb81C7300, 0x000000C7);		// [7:0]:MSO=4 lane, enhance mode=EN, 4-lane																									 
			IoReg_Write32(0xb81C7304, 0x00000000);		// [7:0]:Pg_rst=0,msa_rst=0 																													 
			IoReg_Write32(0xb81C7308, 0x00000000);		// [7:0]:debug sel																																 
			IoReg_Write32(0xb81C730C, 0x00000000);		// [7:0]:irq disable																															 
			IoReg_Write32(0xb81C731C, 0x00000000);		// IE disable																																	 
			IoReg_Write32(0xb81C7320, 0x00000000);		// [7] MN_vid auto=dis, [6]:db, [0]:popup																										 
			IoReg_Write32(0xb81C7324, 0x000025D7);		// [23:0]M_vid=0x0025D7 																														 
			IoReg_Write32(0xb81C7328, 0x00002000);		// [23:0]N_vid=0x002000 																														 
			IoReg_Write32(0xb81C7330, 0x00000000);		// [7:0] N_mul=0;																																 
			IoReg_Write32(0xb81C7320, 0x00000040);		// [7] MN_vid auto=dis, [6]:db, [0]:popup C																									  

			//// MSA																																												
			IoReg_Write32(0xb81C7338, 0x00000040);		// [7] msa_db=0 [6]=1 just before MSA																											 
			IoReg_Write32(0xb81C733C, 0x00000020);		// [14]:color format follow misc1.6, [13]ycc_col=601, [12]VESA, [11]Async,[10:9] no stereo, [8]interlace odd num,								 
												   // [7:5]:color bit=8 bit [4:3]format=RGB,																										
			IoReg_Write32(0xb81C7340, 0x00000A50);		// [15:0]:Htt =2640 																															 
			IoReg_Write32(0xb81C7344, 0x00000030);		// [15:0]:Hsta =48																																 
			IoReg_Write32(0xb81C7348, 0x00000A00);		// [15:0]:Hwidth =2560																															 
			IoReg_Write32(0xb81C734C, 0x0000003F);		// [15]HSP=low, [14:0]:Hsw =64																													 
			IoReg_Write32(0xb81C7350, 0x000005BA);		// [15:0]:Vtt =1466 																															 
			IoReg_Write32(0xb81C7354, 0x0000000D);		// [15:0]:Vsta =13																																 
			IoReg_Write32(0xb81C7358, 0x000005A0);		// [15:0]:Vhight =1440																															 
			IoReg_Write32(0xb81C735C, 0x00000005);		// [15]VSP=low, [14:0]:Vsw =5																													 

			//// VBID																																												
			IoReg_Write32(0xb81C7360, 0x00000000);		// [2]audiomute_flag [1]novideo_flag																											 
			IoReg_Write32(0xb81C7364, 0x00000000);		// [1]interlace=0 [0]vbid_fw_ctrl=HW mode																										 

			//// ARBITER																																											
			IoReg_Write32(0xb81C7368, 0x00000012);		// [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode [0]arbiter_en=0 			 biter_en=0
			IoReg_Write32(0xb81C736C, 0x00000780);		// [15:0]vdata_per_line=0x0780 (1920)																											 
												   //  vdata_per_line==> Hwidth * BPC (bit per component) * 3 / 8 (byte) = lane_num * v_data_per_line (byte)										
			IoReg_Write32(0xb81C7370, 0x00000040);		// [7]TU_size<1,[6:0]:tu_size=0x40 (64) 																										 
			IoReg_Write32(0xb81C7374, 0x000001D6);		// [9:3]tu_data_size=0x34<<3=1A6, [2:0]=0.8=0x6 																								 
												   // tu_data_size =-> SCLK(freq) * BPC * 3 /8 (byte)= (TU_DATA_SIZE/TU) * LCLK(freq) * LANE_NUM (byte) 											
			IoReg_Write32(0xb81C7378, 0x000000C8);		// [15:0]:hdelay																																 
			IoReg_Write32(0xb81C737C, 0x00000064);		// [7:0]:sec_end_cnt MSB[15:8] LSB[7:0] 																										 
			IoReg_Write32(0xb81C73AC, 0x00000046);		// [7:0]:sec_idel_end_cnt																														 
			IoReg_Write32(0xb81C7380, 0x00000080);		// sec_end db apply 																															 

			////TX_TOP																																												
			IoReg_Write32(0xb81C738C, 0x00000000);		// I2V_first_blank_filter[1], idel2Vdieo switch=cnt to 8191 																					 
			IoReg_Write32(0xb81C7390, 0x00000000);		// RSV1 																																		 
			IoReg_Write32(0xb81C7394, 0x00000000);		// RSV2 																																		 

			////Arbiter 																																											
			IoReg_Write32(0xb81C7398, 0x00008000);		// (Read_only) [15]wclr [14:0]min_h_blank_wid																									 
			IoReg_Write32(0xb81C739C, 0x00000000);		// IE disable																																	 

			////Vesa																																												
			IoReg_Write32(0xb81C73A0, 0x00000000);		// align Vsync and Hsync = disable																												 
			IoReg_Write32(0xb81C73B0, 0x00000013);		// [3] FW M/N code [0] using source's field signal																								 
			IoReg_Write32(0xb81C73B8, 0x000000FF);		// [0]=1 Ref BE to gen VBID[0]																													 

			////db apply																																											
			IoReg_Write32(0xb81C7338, 0x000000C0);		// [7] msa_db=1 [6]=0 just before MSA																											 
			IoReg_Write32(0xb81C7368, 0x00000013);		// [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode 							 
												   // [0]arbiter_en=1																																
			
			//---------------------------------------------------------------------------------------------------------------------------------
			//TX2_MAC
			//---------------------------------------------------------------------------------------------------------------------------------
			IoReg_Write32(0xb81C6700, 0x000000C7);		// [7:0]:MSO=4 lane, enhance mode=EN, 4-lane																									 
			IoReg_Write32(0xb81C6704, 0x00000000);		// [7:0]:Pg_rst=0,msa_rst=0 																													 
			IoReg_Write32(0xb81C6708, 0x00000000);		// [7:0]:debug sel																																 
			IoReg_Write32(0xb81C670C, 0x00000000);		// [7:0]:irq disable																															 
			IoReg_Write32(0xb81C671C, 0x00000000);		// IE disable																																	 
			IoReg_Write32(0xb81C6720, 0x00000000);		// [7] MN_vid auto=dis, [6]:db, [0]:popup																										 
			IoReg_Write32(0xb81C6724, 0x000025D7);		// [23:0]M_vid=0x0025D7 																														 
			IoReg_Write32(0xb81C6728, 0x00002000);		// [23:0]N_vid=0x002000 																														 
			IoReg_Write32(0xb81C6730, 0x00000000);		// [7:0] N_mul=0;																																 
			IoReg_Write32(0xb81C6720, 0x00000040);		// [7] MN_vid auto=dis, [6]:db, [0]:popup C																									  

			//// MSA																																												
			IoReg_Write32(0xb81C6738, 0x00000040);		// [7] msa_db=0 [6]=1 just before MSA																											 
			IoReg_Write32(0xb81C673C, 0x00000020);		// [14]:color format follow misc1.6, [13]ycc_col=601, [12]VESA, [11]Async,[10:9] no stereo, [8]interlace odd num,								 
												   // [7:5]:color bit=8 bit [4:3]format=RGB,																										
			IoReg_Write32(0xb81C6740, 0x00000A50);		// [15:0]:Htt =2640 																															 
			IoReg_Write32(0xb81C6744, 0x00000030);		// [15:0]:Hsta =48																																 
			IoReg_Write32(0xb81C6748, 0x00000A00);		// [15:0]:Hwidth =2560																															 
			IoReg_Write32(0xb81C674C, 0x0000003F);		// [15]HSP=low, [14:0]:Hsw =64																													 
			IoReg_Write32(0xb81C6750, 0x000005BA);		// [15:0]:Vtt =1466 																															 
			IoReg_Write32(0xb81C6754, 0x0000000D);		// [15:0]:Vsta =13																																 
			IoReg_Write32(0xb81C6758, 0x000005A0);		// [15:0]:Vhight =1440																															 
			IoReg_Write32(0xb81C675C, 0x00000005);		// [15]VSP=low, [14:0]:Vsw =5																													 

			//// VBID																																												
			IoReg_Write32(0xb81C6760, 0x00000000);		// [2]audiomute_flag [1]novideo_flag																											 
			IoReg_Write32(0xb81C6764, 0x00000000);		// [1]interlace=0 [0]vbid_fw_ctrl=HW mode																										 

			//// ARBITER																																											
			IoReg_Write32(0xb81C6768, 0x00000012);		// [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode [0]arbiter_en=0 			 
			IoReg_Write32(0xb81C676C, 0x00000780);		// [15:0]vdata_per_line=0x0780 (1920)																											 
												   //  vdata_per_line==> Hwidth * BPC (bit per component) * 3 / 8 (byte) = lane_num * v_data_per_line (byte)										
			IoReg_Write32(0xb81C6770, 0x00000040);		// [7]TU_size<1,[6:0]:tu_size=0x40 (64) 																										 
			IoReg_Write32(0xb81C6774, 0x000001D6);		// [9:3]tu_data_size=0x34<<3=1A6, [2:0]=0.8=0x6 																								 
												   // tu_data_size =-> SCLK(freq) * BPC * 3 /8 (byte)= (TU_DATA_SIZE/TU) * LCLK(freq) * LANE_NUM (byte) 											
			IoReg_Write32(0xb81C6778, 0x000000C8);		// [15:0]:hdelay																																 
			IoReg_Write32(0xb81C677C, 0x00000064);		// [7:0]:sec_end_cnt MSB[15:8] LSB[7:0] 																										 
			IoReg_Write32(0xb81C67AC, 0x00000046);		// [7:0]:sec_idel_end_cnt																														 
			IoReg_Write32(0xb81C6780, 0x00000080);		// sec_end db apply 																															 

			////TX_TOP																																												
			IoReg_Write32(0xb81C678C, 0x00000000);		// I2V_first_blank_filter[1], idel2Vdieo switch=cnt to 8191 																					 
			IoReg_Write32(0xb81C6790, 0x00000000);		// RSV1 																																		 
			IoReg_Write32(0xb81C6794, 0x00000000);		// RSV2 																																		 

			////Arbiter 																																											
			IoReg_Write32(0xb81C6798, 0x00008000);		// (Read_only) [15]wclr [14:0]min_h_blank_wid																									 
			IoReg_Write32(0xb81C679C, 0x00000000);		// IE disable																																	 

			////Vesa																																												
			IoReg_Write32(0xb81C67A0, 0x00000000);		// align Vsync and Hsync = disable																												 
			IoReg_Write32(0xb81C67B0, 0x00000013);		// [3] FW M/N code [0] using source's field signal																								 
			IoReg_Write32(0xb81C67B8, 0x000000FF);		// [0]=1 Ref BE to gen VBID[0]																													 

			////DB apply																																											
			IoReg_Write32(0xb81C6738, 0x000000C0);		// [7] msa_db=1 [6]=0 just before MSA																											 
			IoReg_Write32(0xb81C6768, 0x00000013);		// [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode 							 
												   // [0]arbiter_en=1																																
			
			//---------------------------------------------------------------------------------------------------------------------------------
			//TX3_MAC
			//---------------------------------------------------------------------------------------------------------------------------------
			IoReg_Write32(0xb81C7700, 0x000000C7);		// [7:0]:MSO=4 lane, enhance mode=EN, 4-lane																									 
			IoReg_Write32(0xb81C7704, 0x00000000);		// [7:0]:Pg_rst=0,msa_rst=0 																													 
			IoReg_Write32(0xb81C7708, 0x00000000);		// [7:0]:debug sel																																 
			IoReg_Write32(0xb81C770C, 0x00000000);		// [7:0]:irq disable																															 
			IoReg_Write32(0xb81C771C, 0x00000000);		// IE disable																																	 
			IoReg_Write32(0xb81C7720, 0x00000000);		// [7] MN_vid auto=dis, [6]:db, [0]:popup																										 
			IoReg_Write32(0xb81C7724, 0x000025D7);		// [23:0]M_vid=0x0025D7 																														 
			IoReg_Write32(0xb81C7728, 0x00002000);		// [23:0]N_vid=0x002000 																														 
			IoReg_Write32(0xb81C7730, 0x00000000);		// [7:0] N_mul=0;																																 
			IoReg_Write32(0xb81C7720, 0x00000040);		// [7] MN_vid auto=dis, [6]:db, [0]:popup C																									  

			//// MSA																																												
			IoReg_Write32(0xb81C7738, 0x00000040);		// [7] msa_db=0 [6]=1 just before MSA																											 
			IoReg_Write32(0xb81C773C, 0x00000020);		// [14]:color format follow misc1.6, [13]ycc_col=601, [12]VESA, [11]Async,[10:9] no stereo, [8]interlace odd num,								 
												   // [7:5]:color bit=8 bit [4:3]format=RGB,																										
			IoReg_Write32(0xb81C7740, 0x00000A50);		// [15:0]:Htt =2640 																															 
			IoReg_Write32(0xb81C7744, 0x00000030);		// [15:0]:Hsta =48																																 
			IoReg_Write32(0xb81C7748, 0x00000A00);		// [15:0]:Hwidth =2560																															 
			IoReg_Write32(0xb81C774C, 0x0000003F);		// [15]HSP=low, [14:0]:Hsw =64																													 
			IoReg_Write32(0xb81C7750, 0x000005BA);		// [15:0]:Vtt =1466 																															 
			IoReg_Write32(0xb81C7754, 0x0000000D);		// [15:0]:Vsta =13																																 
			IoReg_Write32(0xb81C7758, 0x000005A0);		// [15:0]:Vhight =1440																															 
			IoReg_Write32(0xb81C775C, 0x00000005);		// [15]VSP=low, [14:0]:Vsw =5																													 

			//// VBID																																												
			IoReg_Write32(0xb81C7760, 0x00000000);		// [2]audiomute_flag [1]novideo_flag																											 
			IoReg_Write32(0xb81C7764, 0x00000000);		// [1]interlace=0 [0]vbid_fw_ctrl=HW mode																										 

			//// ARBITER																																											
			IoReg_Write32(0xb81C7768, 0x00000012);		// [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode [0]arbiter_en=0 			 
			IoReg_Write32(0xb81C776C, 0x00000780);		// [15:0]vdata_per_line=0x0780 (1920)																											 
												   //  vdata_per_line==> Hwidth * BPC (bit per component) * 3 / 8 (byte) = lane_num * v_data_per_line (byte)										
			IoReg_Write32(0xb81C7770, 0x00000040);		// [7]TU_size<1,[6:0]:tu_size=0x40 (64) 																										 
			IoReg_Write32(0xb81C7774, 0x000001D6);		// [9:3]tu_data_size=0x34<<3=1A6, [2:0]=0.8=0x6 																								 
												   // tu_data_size =-> SCLK(freq) * BPC * 3 /8 (byte)= (TU_DATA_SIZE/TU) * LCLK(freq) * LANE_NUM (byte) 											
			IoReg_Write32(0xb81C7778, 0x000000C8);		// [15:0]:hdelay																																 
			IoReg_Write32(0xb81C777C, 0x00000064);		// [7:0]:sec_end_cnt MSB[15:8] LSB[7:0] 																										 
			IoReg_Write32(0xb81C77AC, 0x00000046);		// [7:0]:sec_idel_end_cnt																														 
			IoReg_Write32(0xb81C7780, 0x00000080);		// sec_end db apply 																															 

			////TX_TOP																																												
			IoReg_Write32(0xb81C778C, 0x00000000);		// I2V_first_blank_filter[1], idel2Vdieo switch=cnt to 8191 																					 
			IoReg_Write32(0xb81C7790, 0x00000000);		// RSV1 																																		 
			IoReg_Write32(0xb81C7794, 0x00000000);		// RSV2 																																		 

			////Arbiter 																																											
			IoReg_Write32(0xb81C7798, 0x00008000);		// (Read_only) [15]wclr [14:0]min_h_blank_wid																									 
			IoReg_Write32(0xb81C779C, 0x00000000);		// IE disable																																	 

			////Vesa																																												
			IoReg_Write32(0xb81C77A0, 0x00000000);		// align Vsync and Hsync = disable																												 
			IoReg_Write32(0xb81C77B0, 0x00000013);		// [3] FW M/N code [0] using source's field signal																								 
			IoReg_Write32(0xb81C77B8, 0x000000FF);		// [0]=1 Ref BE to gen VBID[0]																													 

			////db apply																																											
			IoReg_Write32(0xb81C7738, 0x000000C0);		// [7] msa_db=1 [6]=0 just before MSA																											 
			IoReg_Write32(0xb81C7768, 0x00000013);		// [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode 							 
												   // [0]arbiter_en=1
		}
		else if(Get_DISPLAY_PANEL_CUSTOM_INDEX() == P_EDP_P0_4K2K_144_652M) {
			//---------------------------------------------------------------------------------------------------------------------------------
		    //TX0_MAC
		    //---------------------------------------------------------------------------------------------------------------------------------
		    IoReg_Write32(0xb81C6300, 0x000000C7);      // [7:0]:MSO=4 lane, enhance mode=EN, 4-lane
		    IoReg_Write32(0xb81C6304, 0x00000000);      // [7:0]:Pg_rst=0,msa_rst=0  
		    IoReg_Write32(0xb81C6308, 0x00000000);      // [7:0]:debug sel
		    IoReg_Write32(0xb81C630C, 0x00000000);      // [7:0]:irq disable
		    IoReg_Write32(0xb81C631C, 0x00000000);      // IE disable
		    IoReg_Write32(0xb81C6320, 0x00000000);      // [7] MN_vid auto=dis, [6]:db, [0]:popup  
		    IoReg_Write32(0xb81C6324, 0x000026B1);      // [23:0]M_vid=0x002583  
		    IoReg_Write32(0xb81C6328, 0x00002000);      // [23:0]N_vid=0x002000 
		    IoReg_Write32(0xb81C6330, 0x00000000);      // [7:0] N_mul=0;  
		    IoReg_Write32(0xb81C6320, 0x00000040);      // [7] MN_vid auto=dis, [6]:db, [0]:popup C

			//// MSA                                  
		    IoReg_Write32(0xb81C6338, 0x00000040);      // [7] msa_db=0 [6]=1 just before MSA 
		    IoReg_Write32(0xb81C633C, 0x00000020);      // [14]:color format follow misc1.6, [13]ycc_col=601, [12]VESA, [11]Async,[10:9] no stereo, [8]interlace odd num,
		                                           // [7:5]:color bit=8 bit [4:3]format=RGB, 
		    IoReg_Write32(0xb81C6340, 0x00000820);      // [15:0]:Htt =2080
		    IoReg_Write32(0xb81C6344, 0x0000003C);      // [15:0]:Hsta =60
		    IoReg_Write32(0xb81C6348, 0x00000780);      // [15:0]:Hwidth =1920
		    IoReg_Write32(0xb81C634C, 0x00000032);      // [15]HSP=low, [14:0]:Hsw =50
		    IoReg_Write32(0xb81C6350, 0x00000884);      // [15:0]:Vtt =2180     
		    IoReg_Write32(0xb81C6354, 0x0000000C);      // [15:0]:Vsta =12 
		    IoReg_Write32(0xb81C6358, 0x00000870);      // [15:0]:Vhight =2160       
		    IoReg_Write32(0xb81C635C, 0x00000001);      // [15]VSP=low, [14:0]:Vsw =1    

			//// VBID                                 
		    IoReg_Write32(0xb81C6360, 0x00000000);      // [2]audiomute_flag [1]novideo_flag
		    IoReg_Write32(0xb81C6364, 0x00000000);      // [1]interlace=0 [0]vbid_fw_ctrl=HW mode

			//// ARBITER                              
		    IoReg_Write32(0xb81C6368, 0x00000012);      // [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode [0]arbiter_en=0
		    IoReg_Write32(0xb81C636C, 0x000005A0);      // [15:0]vdata_per_line=1440
		                                           //  vdata_per_line==> Hwidth * BPC (bit per component) * 3 / 8 (byte) = lane_num * v_data_per_line (byte)  
		    IoReg_Write32(0xb81C6370, 0x00000040);      // [7]TU_size<1,[6:0]:tu_size=0x40 (64)    
		    IoReg_Write32(0xb81C6374, 0x000001D0);      // [9:3]tu_data_size=0x38, [2:0]=0.3=0x1
		                                           // tu_data_size =-> SCLK(freq) * BPC * 3 /8 (byte)= (TU_DATA_SIZE/TU) * LCLK(freq) * LANE_NUM (byte) 
		    IoReg_Write32(0xb81C6378, 0x00000090);      // [15:0]:hdelay                                  
		    IoReg_Write32(0xb81C637C, 0x00000064);      // [7:0]:sec_end_cnt MSB[15:8] LSB[7:0]
		    IoReg_Write32(0xb81C63AC, 0x00000046);      // [7:0]:sec_idel_end_cnt  
		    IoReg_Write32(0xb81C6380, 0x00000080);      // sec_end db apply

			////TX_TOP
		    IoReg_Write32(0xb81C638C, 0x00000000);      // I2V_first_blank_filter[1], idel2Vdieo switch=cnt to 8191
		    IoReg_Write32(0xb81C6390, 0x00000000);      // RSV1
		    IoReg_Write32(0xb81C6394, 0x00000000);      // RSV2

			////Arbiter
		    IoReg_Write32(0xb81C6398, 0x00008000);      // (Read_only) [15]wclr [14:0]min_h_blank_wid
		    IoReg_Write32(0xb81C639C, 0x00000000);      // IE disable

			////Vesa
		    IoReg_Write32(0xb81C63A0, 0x00000000);      // align Vsync and Hsync = disable
		    IoReg_Write32(0xb81C63B0, 0x00000013);      // [3] FW M/N code [0] using source's field signal
		    IoReg_Write32(0xb81C63B8, 0x000000FF);      // [0]=1 Ref BE to gen VBID[0]

			////DB apply
		    IoReg_Write32(0xb81C6338, 0x000000C0);      // [7] msa_db=1 [6]=0 just before MSA 
		    IoReg_Write32(0xb81C6368, 0x00000013);      // [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode 
		                                           // [0]arbiter_en=1 
		    
		    //---------------------------------------------------------------------------------------------------------------------------------
		    //TX1_MAC
		    //---------------------------------------------------------------------------------------------------------------------------------
		    IoReg_Write32(0xb81C7300, 0x000000C7);      // [7:0]:MSO=4 lane, enhance mode=EN, 4-lane                                                                                                                             
		    IoReg_Write32(0xb81C7304, 0x00000000);      // [7:0]:Pg_rst=0,msa_rst=0                                                                                                                                              
		    IoReg_Write32(0xb81C7308, 0x00000000);      // [7:0]:debug sel                                                                                                                                                       
		    IoReg_Write32(0xb81C730C, 0x00000000);      // [7:0]:irq disable                                                                                                                                                     
		    IoReg_Write32(0xb81C731C, 0x00000000);      // IE disable                                                                                                                                                            
		    IoReg_Write32(0xb81C7320, 0x00000000);      // [7] MN_vid auto=dis, [6]:db, [0]:popup                                                                                                                                
		    IoReg_Write32(0xb81C7324, 0x000026B1);      // [23:0]M_vid=0x002583                                                                                                                                                  
		    IoReg_Write32(0xb81C7328, 0x00002000);      // [23:0]N_vid=0x002000                                                                                                                                                  
		    IoReg_Write32(0xb81C7330, 0x00000000);      // [7:0] N_mul=0;                                                                                                                                                        
		    IoReg_Write32(0xb81C7320, 0x00000040);      // [7] MN_vid auto=dis, [6]:db, [0]:popup C                                                                                                                            

			//// MSA                                                                                                                                                                                                        
		    IoReg_Write32(0xb81C7338, 0x00000040);      // [7] msa_db=0 [6]=1 just before MSA                                                                                                                                    
		    IoReg_Write32(0xb81C733C, 0x00000020);      // [14]:color format follow misc1.6, [13]ycc_col=601, [12]VESA, [11]Async,[10:9] no stereo, [8]interlace odd num,                                                        
		                                           // [7:5]:color bit=8 bit [4:3]format=RGB,                                                                                                                                
		    IoReg_Write32(0xb81C7340, 0x00000820);      // [15:0]:Htt =2080                                                                                                                                                      
		    IoReg_Write32(0xb81C7344, 0x0000003C);      // [15:0]:Hsta =60                                                                                                                                                       
		    IoReg_Write32(0xb81C7348, 0x00000780);      // [15:0]:Hwidth =1920                                                                                                                                                   
		    IoReg_Write32(0xb81C734C, 0x00000032);      // [15]HSP=low, [14:0]:Hsw =50                                                                                                                                           
		    IoReg_Write32(0xb81C7350, 0x00000884);      // [15:0]:Vtt =2180                                                                                                                                                      
		    IoReg_Write32(0xb81C7354, 0x0000000C);      // [15:0]:Vsta =12                                                                                                                                                       
		    IoReg_Write32(0xb81C7358, 0x00000870);      // [15:0]:Vhight =2160                                                                                                                                                   
		    IoReg_Write32(0xb81C735C, 0x00000001);      // [15]VSP=low, [14:0]:Vsw =1                                                                                                                                            

			//// VBID                                                                                                                                                                                                       
		    IoReg_Write32(0xb81C7360, 0x00000000);      // [2]audiomute_flag [1]novideo_flag                                                                                                                                     
		    IoReg_Write32(0xb81C7364, 0x00000000);      // [1]interlace=0 [0]vbid_fw_ctrl=HW mode                                                                                                                                

			//// ARBITER                                                                                                                                                                                                    
		    IoReg_Write32(0xb81C7368, 0x00000012);      // [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode [0]arbiter_en=0                                      
		    IoReg_Write32(0xb81C736C, 0x000005A0);      // [15:0]vdata_per_line=1440                                                                                                                                             
		                                           //  vdata_per_line==> Hwidth * BPC (bit per component) * 3 / 8 (byte) = lane_num * v_data_per_line (byte)                                                                
		    IoReg_Write32(0xb81C7370, 0x00000040);      // [7]TU_size<1,[6:0]:tu_size=0x40 (64)                                                                                                                                  
		    IoReg_Write32(0xb81C7374, 0x000001D0);      // [9:3]tu_data_size=0x38, [2:0]=0.3=0x1                                                                                                                                 
		                                           // tu_data_size =-> SCLK(freq) * BPC * 3 /8 (byte)= (TU_DATA_SIZE/TU) * LCLK(freq) * LANE_NUM (byte)                                                                     
		    IoReg_Write32(0xb81C7378, 0x00000090);      // [15:0]:hdelay                                                                                                                                                         
		    IoReg_Write32(0xb81C737C, 0x00000064);      // [7:0]:sec_end_cnt MSB[15:8] LSB[7:0]                                                                                                                                  
		    IoReg_Write32(0xb81C73AC, 0x00000046);      // [7:0]:sec_idel_end_cnt                                                                                                                                                
		    IoReg_Write32(0xb81C7380, 0x00000080);      // sec_end db apply                                                                                                                                                      

			////TX_TOP                                                                                                                                                                                                      
		    IoReg_Write32(0xb81C738C, 0x00000000);      // I2V_first_blank_filter[1], idel2Vdieo switch=cnt to 8191                                                                                                              
		    IoReg_Write32(0xb81C7390, 0x00000000);      // RSV1                                                                                                                                                                  
		    IoReg_Write32(0xb81C7394, 0x00000000);      // RSV2                                                                                                                                                                  

			////Arbiter                                                                                                                                                                                                     
		    IoReg_Write32(0xb81C7398, 0x00008000);      // (Read_only) [15]wclr [14:0]min_h_blank_wid                                                                                                                            
		    IoReg_Write32(0xb81C739C, 0x00000000);      // IE disable                                                                                                                                                            

			////Vesa                                                                                                                                                                                                        
		    IoReg_Write32(0xb81C73A0, 0x00000000);      // align Vsync and Hsync = disable                                                                                                                                       
		    IoReg_Write32(0xb81C73B0, 0x00000013);      // [3] FW M/N code [0] using source's field signal                                                                                                                       
		    IoReg_Write32(0xb81C73B8, 0x000000FF);      // [0]=1 Ref BE to gen VBID[0]                                                                                                                                           

			////db apply                                                                                                                                                                                                    
		    IoReg_Write32(0xb81C7338, 0x000000C0);      // [7] msa_db=1 [6]=0 just before MSA                                                                                                                                    
		    IoReg_Write32(0xb81C7368, 0x00000013);      // [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode                                                      
		                                           // [0]arbiter_en=1                                                                                                                                                       
		    
		    //---------------------------------------------------------------------------------------------------------------------------------
		    //TX2_MAC
		    //---------------------------------------------------------------------------------------------------------------------------------
		    IoReg_Write32(0xb81C6700, 0x000000C7);      // [7:0]:MSO=4 lane, enhance mode=EN, 4-lane                                                                                                                                     
		    IoReg_Write32(0xb81C6704, 0x00000000);      // [7:0]:Pg_rst=0,msa_rst=0                                                                                                                                                      
		    IoReg_Write32(0xb81C6708, 0x00000000);      // [7:0]:debug sel                                                                                                                                                               
		    IoReg_Write32(0xb81C670C, 0x00000000);      // [7:0]:irq disable                                                                                                                                                             
		    IoReg_Write32(0xb81C671C, 0x00000000);      // IE disable                                                                                                                                                                    
		    IoReg_Write32(0xb81C6720, 0x00000000);      // [7] MN_vid auto=dis, [6]:db, [0]:popup                                                                                                                                        
		    IoReg_Write32(0xb81C6724, 0x000026B1);      // [23:0]M_vid=0x002583                                                                                                                                                          
		    IoReg_Write32(0xb81C6728, 0x00002000);      // [23:0]N_vid=0x002000                                                                                                                                                          
		    IoReg_Write32(0xb81C6730, 0x00000000);      // [7:0] N_mul=0;                                                                                                                                                                
		    IoReg_Write32(0xb81C6720, 0x00000040);      // [7] MN_vid auto=dis, [6]:db, [0]:popup C                                                                                                                                    

			//// MSA                                                                                                                                                                                                                
		    IoReg_Write32(0xb81C6738, 0x00000040);      // [7] msa_db=0 [6]=1 just before MSA                                                                                                                                            
		    IoReg_Write32(0xb81C673C, 0x00000020);      // [14]:color format follow misc1.6, [13]ycc_col=601, [12]VESA, [11]Async,[10:9] no stereo, [8]interlace odd num,                                                                
		                                           // [7:5]:color bit=8 bit [4:3]format=RGB,                                                                                                                                        
		    IoReg_Write32(0xb81C6740, 0x00000820);      // [15:0]:Htt =2080                                                                                                                                                              
		    IoReg_Write32(0xb81C6744, 0x0000003C);      // [15:0]:Hsta =60                                                                                                                                                               
		    IoReg_Write32(0xb81C6748, 0x00000780);      // [15:0]:Hwidth =1920                                                                                                                                                           
		    IoReg_Write32(0xb81C674C, 0x00000032);      // [15]HSP=low, [14:0]:Hsw =50                                                                                                                                                   
		    IoReg_Write32(0xb81C6750, 0x00000884);      // [15:0]:Vtt =2180                                                                                                                                                              
		    IoReg_Write32(0xb81C6754, 0x0000000C);      // [15:0]:Vsta =12                                                                                                                                                               
		    IoReg_Write32(0xb81C6758, 0x00000870);      // [15:0]:Vhight =2160                                                                                                                                                           
		    IoReg_Write32(0xb81C675C, 0x00000001);      // [15]VSP=low, [14:0]:Vsw =1                                                                                                                                                    

			//// VBID                                                                                                                                                                                                               
		    IoReg_Write32(0xb81C6760, 0x00000000);      // [2]audiomute_flag [1]novideo_flag                                                                                                                                             
		    IoReg_Write32(0xb81C6764, 0x00000000);      // [1]interlace=0 [0]vbid_fw_ctrl=HW mode                                                                                                                                        

			//// ARBITER                                                                                                                                                                                                            
		    IoReg_Write32(0xb81C6768, 0x00000012);      // [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode [0]arbiter_en=0                                              
		    IoReg_Write32(0xb81C676C, 0x000005A0);      // [15:0]vdata_per_line=1440                                                                                                                                                     
		                                           //  vdata_per_line==> Hwidth * BPC (bit per component) * 3 / 8 (byte) = lane_num * v_data_per_line (byte)                                                                        
		    IoReg_Write32(0xb81C6770, 0x00000040);      // [7]TU_size<1,[6:0]:tu_size=0x40 (64)                                                                                                                                          
		    IoReg_Write32(0xb81C6774, 0x000001D0);      // [9:3]tu_data_size=0x38, [2:0]=0.3=0x1                                                                                                                                         
		                                           // tu_data_size =-> SCLK(freq) * BPC * 3 /8 (byte)= (TU_DATA_SIZE/TU) * LCLK(freq) * LANE_NUM (byte)                                                                             
		    IoReg_Write32(0xb81C6778, 0x00000090);      // [15:0]:hdelay                                                                                                                                                                 
		    IoReg_Write32(0xb81C677C, 0x00000064);      // [7:0]:sec_end_cnt MSB[15:8] LSB[7:0]                                                                                                                                          
		    IoReg_Write32(0xb81C67AC, 0x00000046);      // [7:0]:sec_idel_end_cnt                                                                                                                                                        
		    IoReg_Write32(0xb81C6780, 0x00000080);      // sec_end db apply                                                                                                                                                              

			////TX_TOP                                                                                                                                                                                                              
		    IoReg_Write32(0xb81C678C, 0x00000000);      // I2V_first_blank_filter[1], idel2Vdieo switch=cnt to 8191                                                                                                                      
		    IoReg_Write32(0xb81C6790, 0x00000000);      // RSV1                                                                                                                                                                          
		    IoReg_Write32(0xb81C6794, 0x00000000);      // RSV2                                                                                                                                                                          

			////Arbiter                                                                                                                                                                                                             
		    IoReg_Write32(0xb81C6798, 0x00008000);      // (Read_only) [15]wclr [14:0]min_h_blank_wid                                                                                                                                    
		    IoReg_Write32(0xb81C679C, 0x00000000);      // IE disable                                                                                                                                                                    

			////Vesa                                                                                                                                                                                                                
		    IoReg_Write32(0xb81C67A0, 0x00000000);      // align Vsync and Hsync = disable                                                                                                                                               
		    IoReg_Write32(0xb81C67B0, 0x00000013);      // [3] FW M/N code [0] using source's field signal                                                                                                                               
		    IoReg_Write32(0xb81C67B8, 0x000000FF);      // [0]=1 Ref BE to gen VBID[0]                                                                                                                                                   

			////DB apply                                                                                                                                                                                                            
		    IoReg_Write32(0xb81C6738, 0x000000C0);      // [7] msa_db=1 [6]=0 just before MSA                                                                                                                                            
		    IoReg_Write32(0xb81C6768, 0x00000013);      // [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode                                                              
		                                           // [0]arbiter_en=1                                                                                                                                                               
		    
		    //---------------------------------------------------------------------------------------------------------------------------------
		    //TX3_MAC
		    //---------------------------------------------------------------------------------------------------------------------------------
		    IoReg_Write32(0xb81C7700, 0x000000C7);      // [7:0]:MSO=4 lane, enhance mode=EN, 4-lane                                                                                                                                                 
		    IoReg_Write32(0xb81C7704, 0x00000000);      // [7:0]:Pg_rst=0,msa_rst=0                                                                                                                                                                  
		    IoReg_Write32(0xb81C7708, 0x00000000);      // [7:0]:debug sel                                                                                                                                                                           
		    IoReg_Write32(0xb81C770C, 0x00000000);      // [7:0]:irq disable                                                                                                                                                                         
		    IoReg_Write32(0xb81C771C, 0x00000000);      // IE disable                                                                                                                                                                                
		    IoReg_Write32(0xb81C7720, 0x00000000);      // [7] MN_vid auto=dis, [6]:db, [0]:popup                                                                                                                                                    
		    IoReg_Write32(0xb81C7724, 0x000026B1);      // [23:0]M_vid=0x002583                                                                                                                                                                      
		    IoReg_Write32(0xb81C7728, 0x00002000);      // [23:0]N_vid=0x002000                                                                                                                                                                      
		    IoReg_Write32(0xb81C7730, 0x00000000);      // [7:0] N_mul=0;                                                                                                                                                                            
		    IoReg_Write32(0xb81C7720, 0x00000040);      // [7] MN_vid auto=dis, [6]:db, [0]:popup C                                                                                                                                                

			//// MSA                                                                                                                                                                                                                            
		    IoReg_Write32(0xb81C7738, 0x00000040);      // [7] msa_db=0 [6]=1 just before MSA                                                                                                                                                        
		    IoReg_Write32(0xb81C773C, 0x00000020);      // [14]:color format follow misc1.6, [13]ycc_col=601, [12]VESA, [11]Async,[10:9] no stereo, [8]interlace odd num,                                                                            
		                                           // [7:5]:color bit=8 bit [4:3]format=RGB,                                                                                                                                                    
		    IoReg_Write32(0xb81C7740, 0x00000820);      // [15:0]:Htt =2080                                                                                                                                                                          
		    IoReg_Write32(0xb81C7744, 0x0000003C);      // [15:0]:Hsta =60                                                                                                                                                                           
		    IoReg_Write32(0xb81C7748, 0x00000780);      // [15:0]:Hwidth =1920                                                                                                                                                                       
		    IoReg_Write32(0xb81C774C, 0x00000032);      // [15]HSP=low, [14:0]:Hsw =50                                                                                                                                                               
		    IoReg_Write32(0xb81C7750, 0x00000884);      // [15:0]:Vtt =2180                                                                                                                                                                          
		    IoReg_Write32(0xb81C7754, 0x0000000C);      // [15:0]:Vsta =12                                                                                                                                                                           
		    IoReg_Write32(0xb81C7758, 0x00000870);      // [15:0]:Vhight =2160                                                                                                                                                                       
		    IoReg_Write32(0xb81C775C, 0x00000001);      // [15]VSP=low, [14:0]:Vsw =1                                                                                                                                                                

			//// VBID                                                                                                                                                                                                                           
		    IoReg_Write32(0xb81C7760, 0x00000000);      // [2]audiomute_flag [1]novideo_flag                                                                                                                                                         
		    IoReg_Write32(0xb81C7764, 0x00000000);      // [1]interlace=0 [0]vbid_fw_ctrl=HW mode                                                                                                                                                    

			//// ARBITER                                                                                                                                                                                                                        
		    IoReg_Write32(0xb81C7768, 0x00000012);      // [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode [0]arbiter_en=0                                                          
		    IoReg_Write32(0xb81C776C, 0x000005A0);      // [15:0]vdata_per_line=1440                                                                                                                                                                 
		                                           //  vdata_per_line==> Hwidth * BPC (bit per component) * 3 / 8 (byte) = lane_num * v_data_per_line (byte)                                                                                    
		    IoReg_Write32(0xb81C7770, 0x00000040);      // [7]TU_size<1,[6:0]:tu_size=0x40 (64)                                                                                                                                                      
		    IoReg_Write32(0xb81C7774, 0x000001D0);      // [9:3]tu_data_size=0x38, [2:0]=0.3=0x1                                                                                                                                                     
		                                           // tu_data_size =-> SCLK(freq) * BPC * 3 /8 (byte)= (TU_DATA_SIZE/TU) * LCLK(freq) * LANE_NUM (byte)                                                                                         
		    IoReg_Write32(0xb81C7778, 0x00000090);      // [15:0]:hdelay                                                                                                                                                                             
		    IoReg_Write32(0xb81C777C, 0x00000064);      // [7:0]:sec_end_cnt MSB[15:8] LSB[7:0]                                                                                                                                                      
		    IoReg_Write32(0xb81C77AC, 0x00000046);      // [7:0]:sec_idel_end_cnt                                                                                                                                                                    
		    IoReg_Write32(0xb81C7780, 0x00000080);      // sec_end db apply                                                                                                                                                                          

			////TX_TOP                                                                                                                                                                                                                          
		    IoReg_Write32(0xb81C778C, 0x00000000);      // I2V_first_blank_filter[1], idel2Vdieo switch=cnt to 8191                                                                                                                                  
		    IoReg_Write32(0xb81C7790, 0x00000000);      // RSV1                                                                                                                                                                                      
		    IoReg_Write32(0xb81C7794, 0x00000000);      // RSV2                                                                                                                                                                                      

			////Arbiter                                                                                                                                                                                                                         
		    IoReg_Write32(0xb81C7798, 0x00008000);      // (Read_only) [15]wclr [14:0]min_h_blank_wid                                                                                                                                                
		    IoReg_Write32(0xb81C779C, 0x00000000);      // IE disable                                                                                                                                                                                

			////Vesa                                                                                                                                                                                                                            
		    IoReg_Write32(0xb81C77A0, 0x00000000);      // align Vsync and Hsync = disable                                                                                                                                                           
		    IoReg_Write32(0xb81C77B0, 0x00000013);      // [3] FW M/N code [0] using source's field signal                                                                                                                                           
		    IoReg_Write32(0xb81C77B8, 0x000000FF);      // [0]=1 Ref BE to gen VBID[0]                                                                                                                                                               

			////db apply                                                                                                                                                                                                                        
		    IoReg_Write32(0xb81C7738, 0x000000C0);      // [7] msa_db=1 [6]=0 just before MSA                                                                                                                                                        
		    IoReg_Write32(0xb81C7768, 0x00000013);      // [6]idel switch by VS, [5]db=0, [4]video_idle_db=vact_edge[3]Video_idle=0, [2] Hwid_div2=0,[1]vactive_md=new mode                                                                          
		                                           // [0]arbiter_en=1   
		}

		//TX0 DPHY Video/MAC FIFO                                      
	    IoReg_Write32(0xb81C6204, 0x00000043);  //[7:0]: Video pattern, DB enable                                        
	    IoReg_Write32(0xb81C6310, 0x000000FF);  //[7:0]:FIFO status wclr 
	    IoReg_Write32(0xb81C6314, 0x0000007F);  // (Read_only wclr)[7:0]:max_vf_water_level  
	    IoReg_Write32(0xb81C6318, 0x0000007F);  // (Read_only wcle)[7:0]:vf_water_level
	    
	    //TX1 DPHY Video/MAC FIFO  
	    IoReg_Write32(0xb81C7204, 0x00000043);  //[7:0]: Video pattern, DB enable     
	    IoReg_Write32(0xb81C7310, 0x000000FF);  //[7:0]:FIFO status wclr
	    IoReg_Write32(0xb81C7314, 0x0000007F);  // (Read_only wclr)[7:0]:max_vf_water_level  
	    IoReg_Write32(0xb81C7318, 0x0000007F);  // (Read_only wcle)[7:0]:vf_water_level  
	    
	    //TX2 DPHY Video/MAC FIFO                                           
	    IoReg_Write32(0xb81C6604, 0x00000043);  //[7:0]: Video pattern, DB enable                                        
	    IoReg_Write32(0xb81C6710, 0x000000FF);  //[7:0]:FIFO status wclr 
	    IoReg_Write32(0xb81C6714, 0x0000007F);  // (Read_only wclr)[7:0]:max_vf_water_level  
	    IoReg_Write32(0xb81C6718, 0x0000007F);  // (Read_only wcle)[7:0]:vf_water_level
	    
	    //TX3 DPHY Video/MAC FIFO  
	    IoReg_Write32(0xb81C7604, 0x00000043);  //[7:0]: Video pattern, DB enable     
	    IoReg_Write32(0xb81C7710, 0x000000FF);  //[7:0]:FIFO status wclr
	    IoReg_Write32(0xb81C7714, 0x0000007F);  // (Read_only wclr)[7:0]:max_vf_water_level  
	    IoReg_Write32(0xb81C7718, 0x0000007F);  // (Read_only wcle)[7:0]:vf_water_level 
	}
#if 0
	if(vbe_disp_suspend_resume.SFG_DUMMY_S_R==0x20000000){
		rtd_pr_vbe_notice("[vbe_disp_panel_resume] vby1 panel enable vby clock after vby1 done\n");
		msleep(1);
		IoReg_Write32(SFG_SFG_DUMMY_reg, vbe_disp_suspend_resume.SFG_DUMMY_S_R);
		rtd_pr_vbe_notice("[vbe_disp_panel_resume] SFG_SFG_DUMMY_reg=%x\n", IoReg_Read32(SFG_SFG_DUMMY_reg));
	}
#endif
}

/****************************Low Power start***********************************/
unsigned char low_power_lane_power_resume(void)
{//only for runtime PM low power
#if 0
    if((Get_DISPLAY_PANEL_TYPE() == P_CEDS)||(Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE)){
        rtd_printk(KERN_EMERG, TAG_NAME_VBE,"low_power_lane_power_resume. TCON++ (%x)\n", IoReg_Read32(TCON_TCON_IP_EN_reg));
        if(Get_DISPLAY_PANEL_CUSTOM_INDEX() == P_CEDS_50_UHD60_INX){
            IoReg_SetBits(0xb802D258, _BIT7);
            IoReg_SetBits(0xb802D3F8, _BIT7);
            IoReg_SetBits(0xb802D398, _BIT7);
        }
        IoReg_SetBits(TCON_TCON_IP_EN_reg, _BIT0);
        rtd_printk(KERN_EMERG, TAG_NAME_VBE,"low_power_lane_power_resume. TCON-- (%x)\n", IoReg_Read32(TCON_TCON_IP_EN_reg));
    }
#endif
#ifdef CONFIG_PM
    IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_reg, vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_S_R);
#endif
    return 0;
}

unsigned char power_on_enable_dtg_ie2_interrupt(void)
{//for std and str
    //enable dtg_ie2 interrupt
    ppoverlay_dtg_ie_2_RBUS ppoverlay_dtg_ie_2_reg;
    ppoverlay_dtg_ie_2_reg.regValue = rtd_inl(PPOVERLAY_DTG_ie_2_reg);
    ppoverlay_dtg_ie_2_reg.dtg_ie_2 = 1;
    ppoverlay_dtg_ie_2_reg.ivs_rm_rising_ie_2 = 1;
    ppoverlay_dtg_ie_2_reg.ivs_dly_rm_rising_ie_2 = 1;
    ppoverlay_dtg_ie_2_reg.mv_den_sta_event_ie_2 = 1;
    ppoverlay_dtg_ie_2_reg.mv_den_end_event_ie_2 = 1;
    ppoverlay_dtg_ie_2_reg.memcdtgv_den_sta_event_ie_2 = 1;//for orbit
    ppoverlay_dtg_ie_2_reg.memcdtgv_den_end_event_ie_2 = 1;//for orbit
    rtd_outl(PPOVERLAY_DTG_ie_2_reg, ppoverlay_dtg_ie_2_reg.regValue);
    rtd_pr_vbe_info("### func:%s finish ###\r\n",__FUNCTION__);
    return 0;
}

unsigned char power_on_resume_dtg_ie_interrupt(void)
{//for str use
#ifdef CONFIG_PM
    IoReg_Write32(PPOVERLAY_DTG_ie_reg, vbe_disp_suspend_resume.PPOVERLAY_DTG_IE_S_R);
#endif
    rtd_pr_vbe_info("### func:%s finish ###\r\n",__FUNCTION__);
    return 0;
}

/****************************Low Power end***********************************/

void vbe_disp_first_resume(void)
{
	//frank@1114 add below flow to set up Ddomain related register
	sys_reg_sys_dispclksel_RBUS sys_dispclksel_reg;
	sys_dispclksel_reg.regValue = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
#if 0 //FIXME: remove CLKEN_DISP_SFG (0x18000208[4]).
	sys_dispclksel_reg.clken_disp_sfg = 1;
#endif
	sys_dispclksel_reg.clken_disp_stage1 = 1;
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, sys_dispclksel_reg.regValue);

	IoReg_SetBits(PPOVERLAY_Main_Display_Control_RSV_reg, _BIT1);//Let main default value enable force bg
	IoReg_SetBits(PPOVERLAY_MP_Layout_Force_to_Background_reg, _BIT1);//Let sub default value enable force bg

	if(IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg) & _BIT24) {
		return;
	}

	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_27_reg, 0x80000000);
#if 0  //FIXME: need to chedk lane power off setting @qing_liu
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPI_PHY_CTRL_1_27_reg, 0x80000000);
#endif

	/*DTG*/
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, (vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_TIMING_CTRL1_S_R&(~_BIT24)));
#if 0	//Eric@20190626 mark for k6hp resume video abnormal
	if(Get_DISPLAY_REFRESH_RATE() >= 120){
		vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_TIMING_CTRL2_S_R = vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_TIMING_CTRL2_S_R|_BIT29;
	}
#endif
	IoReg_Write32(SB2_SHADOW_0_reg, vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_TIMING_CTRL2_S_R); // dummy register for debug
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_TIMING_CTRL2_S_R);
	IoReg_Write32(PPOVERLAY_DV_Length_reg, vbe_disp_suspend_resume.PPOVERLAY_DV_LENGTH_S_R);
	IoReg_Write32(PPOVERLAY_DV_total_reg, vbe_disp_suspend_resume.PPOVERLAY_DV_TOTAL_S_R);
	//IoReg_Write32(PPOVERLAY_DTG_pending_status_reg, vbe_disp_suspend_resume.PPOVERLAY_DTG_PENDING_STATUS_S_R);
	IoReg_Write32(PPOVERLAY_Display_Background_Color_reg, vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_BACKGROUND_COLOR_S_R);
	IoReg_Write32(PPOVERLAY_DH_Width_reg, vbe_disp_suspend_resume.PPOVERLAY_DH_WIDTH_S_R);
	IoReg_Write32(PPOVERLAY_DTG_ie_reg, vbe_disp_suspend_resume.PPOVERLAY_DTG_IE_S_R);
	IoReg_Write32(PPOVERLAY_DTG_ie_2_reg, vbe_disp_suspend_resume.PPOVERLAY_DTG_IE_2_S_R);
	IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, vbe_disp_suspend_resume.PPOVERLAY_DH_TOTAL_LAST_LINE_LENGTH_S_R);
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, vbe_disp_suspend_resume.SYS_DISPCLKSEL_S_R);
	IoReg_Write32(PPOVERLAY_FS_IV_DV_Fine_Tuning2_reg, vbe_disp_suspend_resume.PPOVERLAY_FS_IV_DV_FINE_TUNING2_S_R);
	IoReg_Write32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg, vbe_disp_suspend_resume.PPOVERLAY_FS_IV_DV_FINE_TUNING5_S_R);
	IoReg_Write32(PPOVERLAY_FS_IV_DV_Fine_Tuning1_reg, vbe_disp_suspend_resume.PPOVERLAY_FS_IV_DV_FINE_TUNING1_S_R);
	IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg , vbe_disp_suspend_resume.PPOVERLAY_uzudtg_DV_TOTAL_S_R  );
	IoReg_Write32(PPOVERLAY_uzudtg_DH_TOTAL_reg , vbe_disp_suspend_resume.PPOVERLAY_uzudtg_DH_TOTAL_S_R  );
	IoReg_Write32(PPOVERLAY_DH_DEN_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_DH_DEN_Start_End_S_R );
	IoReg_Write32(PPOVERLAY_DV_DEN_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_DV_DEN_Start_End_S_R );
	IoReg_Write32(PPOVERLAY_UZUDTG_COUNTROL_reg , vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_COUNTROL_S_R  );
	IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, vbe_disp_suspend_resume.PPOVERLAY_memcdtg_DV_TOTAL_S_R );
	IoReg_Write32(PPOVERLAY_memcdtg_DH_TOTAL_reg, vbe_disp_suspend_resume.PPOVERLAY_memcdtg_DH_TOTAL_S_R );
	IoReg_Write32(PPOVERLAY_memcdtg_FREE_DV_TOTAL_reg, vbe_disp_suspend_resume.PPOVERLAY_memcdtg_FREE_DV_TOTAL_S_R );
	IoReg_Write32(PPOVERLAY_memcdtg_FREE_DH_TOTAL_reg, vbe_disp_suspend_resume.PPOVERLAY_memcdtg_FREE_DH_TOTAL_S_R );
	IoReg_Write32(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_memcdtg_DH_DEN_Start_End_S_R);
	IoReg_Write32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_memcdtg_DV_DEN_Start_End_S_R);
	IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL_reg, vbe_disp_suspend_resume.PPOVERLAY_MEMCDTG_CONTROL_S_R  );
	//for OSD split
	if((Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_4K2K_OSD2K1K_to_H5X_SPLIT) ||(Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_4K2K_OSD1K2K_to_H5X_SPLIT)){
		IoReg_Write32(PPOVERLAY_osddtg_DV_TOTAL_reg, vbe_disp_suspend_resume.PPOVERLAY_osddtg_DV_TOTAL_S_R);
		IoReg_Write32(PPOVERLAY_osddtg_DH_TOTAL_reg, vbe_disp_suspend_resume.PPOVERLAY_osddtg_DH_TOTAL_S_R);
		IoReg_Write32(PPOVERLAY_osddtg_DV_DEN_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_osddtg_DV_DEN_Start_End_S_R);
		IoReg_Write32(PPOVERLAY_osddtg_DH_DEN_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_osddtg_DH_DEN_Start_End_S_R);
		IoReg_Write32(PPOVERLAY_OSDDTG_CONTROL_reg, vbe_disp_suspend_resume.PPOVERLAY_OSDDTG_CONTROL_S_R);
		IoReg_Write32(PPOVERLAY_OSDDTG_CONTROL2_reg, vbe_disp_suspend_resume.PPOVERLAY_OSDDTG_CONTROL2_S_R);
		IoReg_Write32(PPOVERLAY_osddtg_fractional_fsync_reg, vbe_disp_suspend_resume.PPOVERLAY_osddtg_fractional_fsync_S_R);
		IoReg_Write32(PPOVERLAY_osd_dtg_dh_width_reg, vbe_disp_suspend_resume.PPOVERLAY_osd_dtg_dh_width_S_R);
		IoReg_Write32(PPOVERLAY_osd_dtg_dv_length_reg, vbe_disp_suspend_resume.PPOVERLAY_osd_dtg_dv_length_S_R);
	}

	if(get_MEMC_bypass_status_refer_platform_model() == TRUE){
		IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, vbe_disp_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_S_R);
	}
	else {
		IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg, vbe_disp_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_S_R|_BIT12);
	}
	IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg,vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_FRACTIONAL_FSYNC_S_R);
	IoReg_Write32(PPOVERLAY_DTG_M_Remove_input_vsync_reg,vbe_disp_suspend_resume.PPOVERLAY_DTG_M_REMOVE_INPUT_VSYNC_S_R);
    first_disp_resume = TRUE;
}
EXPORT_SYMBOL(vbe_disp_first_resume);

/*VBE disp suspend & resume*/
void vbe_disp_suspend(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_GDMA) && !defined(CONFIG_CUSTOMER_TV006)
	gdma_suspend_by_vbe();
#endif
	Panel_SetBackLightMode(0);

	/*DTG*/
	vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_TIMING_CTRL1_S_R = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
	vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_TIMING_CTRL2_S_R = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	vbe_disp_suspend_resume.PPOVERLAY_DV_LENGTH_S_R = IoReg_Read32(PPOVERLAY_DV_Length_reg);
	vbe_disp_suspend_resume.PPOVERLAY_DV_TOTAL_S_R = IoReg_Read32(PPOVERLAY_DV_total_reg);
	//vbe_disp_suspend_resume.PPOVERLAY_DTG_PENDING_STATUS_S_R = IoReg_Read32(PPOVERLAY_DTG_pending_status_reg);
	vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_BACKGROUND_COLOR_S_R = IoReg_Read32(PPOVERLAY_Display_Background_Color_reg);
	vbe_disp_suspend_resume.PPOVERLAY_DH_WIDTH_S_R = IoReg_Read32(PPOVERLAY_DH_Width_reg);
	vbe_disp_suspend_resume.PPOVERLAY_DTG_IE_S_R = IoReg_Read32(PPOVERLAY_DTG_ie_reg);
	vbe_disp_suspend_resume.PPOVERLAY_DTG_IE_2_S_R = IoReg_Read32(PPOVERLAY_DTG_ie_2_reg);
	vbe_disp_suspend_resume.PPOVERLAY_DH_TOTAL_LAST_LINE_LENGTH_S_R = IoReg_Read32(PPOVERLAY_DH_Total_Last_Line_Length_reg);
	vbe_disp_suspend_resume.SYS_DISPCLKSEL_S_R = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	vbe_disp_suspend_resume.PPOVERLAY_FS_IV_DV_FINE_TUNING2_S_R = IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning2_reg);
	vbe_disp_suspend_resume.PPOVERLAY_FS_IV_DV_FINE_TUNING5_S_R = IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg);
	vbe_disp_suspend_resume.PPOVERLAY_FS_IV_DV_FINE_TUNING1_S_R = IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning1_reg);
	vbe_disp_suspend_resume.PPOVERLAY_DOUBLE_BUFFER_CTRL_S_R = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	vbe_disp_suspend_resume.PPOVERLAY_uzudtg_DV_TOTAL_S_R  = IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg);
	vbe_disp_suspend_resume.PPOVERLAY_uzudtg_DH_TOTAL_S_R  = IoReg_Read32(PPOVERLAY_uzudtg_DH_TOTAL_reg);
	vbe_disp_suspend_resume.PPOVERLAY_DH_DEN_Start_End_S_R = IoReg_Read32(PPOVERLAY_DH_DEN_Start_End_reg);
	vbe_disp_suspend_resume.PPOVERLAY_DV_DEN_Start_End_S_R = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_COUNTROL_S_R  = IoReg_Read32(PPOVERLAY_UZUDTG_COUNTROL_reg);
	vbe_disp_suspend_resume.PPOVERLAY_memcdtg_DV_TOTAL_S_R = IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg);
	vbe_disp_suspend_resume.PPOVERLAY_memcdtg_DH_TOTAL_S_R = IoReg_Read32(PPOVERLAY_memcdtg_DH_TOTAL_reg);
	vbe_disp_suspend_resume.PPOVERLAY_memcdtg_FREE_DV_TOTAL_S_R = IoReg_Read32(PPOVERLAY_memcdtg_FREE_DV_TOTAL_reg);
	vbe_disp_suspend_resume.PPOVERLAY_memcdtg_FREE_DH_TOTAL_S_R = IoReg_Read32(PPOVERLAY_memcdtg_FREE_DH_TOTAL_reg);
	vbe_disp_suspend_resume.PPOVERLAY_memcdtg_DH_DEN_Start_End_S_R = IoReg_Read32(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg);
	vbe_disp_suspend_resume.PPOVERLAY_memcdtg_DV_DEN_Start_End_S_R = IoReg_Read32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg);
	vbe_disp_suspend_resume.PPOVERLAY_MEMCDTG_CONTROL_S_R  = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_reg);
	vbe_disp_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_S_R    = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_FRACTIONAL_FSYNC_S_R = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	vbe_disp_suspend_resume.PPOVERLAY_DTG_M_REMOVE_INPUT_VSYNC_S_R = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);

	/*Aisr mode & s1ip delay*/
	vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_CONTROL3_S_R = IoReg_Read32(PPOVERLAY_uzudtg_control3_reg);
	vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_CONTROL5_S_R = IoReg_Read32(PPOVERLAY_uzudtg_control5_reg);

	/*Time borrow*/
	vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_STAGE1_TIME_BORROW_S_R = IoReg_Read32(PPOVERLAY_uzudtg_stage1_time_borrow_reg);

	/*Orbit overscan*/
	vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_ORBIT_MAIN_CTRL0_S_R = IoReg_Read32(PPOVERLAY_uzudtg_orbit_main_ctrl0_reg);
	vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_ORBIT_MAIN_CTRL2_S_R = IoReg_Read32(PPOVERLAY_uzudtg_orbit_main_ctrl2_reg);
	vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_ORBIT_DEBUG_CTRL0_S_R = IoReg_Read32(PPOVERLAY_uzudtg_orbit_debug_ctrl0_reg);

	// for OSD split
	if((Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_4K2K_OSD2K1K_to_H5X_SPLIT) ||(Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_4K2K_OSD1K2K_to_H5X_SPLIT)){
		vbe_disp_suspend_resume.PPOVERLAY_osddtg_DV_TOTAL_S_R = IoReg_Read32(PPOVERLAY_osddtg_DV_TOTAL_reg);
		vbe_disp_suspend_resume.PPOVERLAY_osddtg_DH_TOTAL_S_R = IoReg_Read32(PPOVERLAY_osddtg_DH_TOTAL_reg);
		vbe_disp_suspend_resume.PPOVERLAY_osddtg_DV_DEN_Start_End_S_R = IoReg_Read32(PPOVERLAY_osddtg_DV_DEN_Start_End_reg);
		vbe_disp_suspend_resume.PPOVERLAY_osddtg_DH_DEN_Start_End_S_R = IoReg_Read32(PPOVERLAY_osddtg_DH_DEN_Start_End_reg);
		vbe_disp_suspend_resume.PPOVERLAY_OSDDTG_CONTROL_S_R = IoReg_Read32(PPOVERLAY_OSDDTG_CONTROL_reg);
		vbe_disp_suspend_resume.PPOVERLAY_OSDDTG_CONTROL2_S_R = IoReg_Read32(PPOVERLAY_OSDDTG_CONTROL2_reg);
		vbe_disp_suspend_resume.PPOVERLAY_osddtg_fractional_fsync_S_R = IoReg_Read32(PPOVERLAY_osddtg_fractional_fsync_reg);
		vbe_disp_suspend_resume.PPOVERLAY_osd_dtg_dh_width_S_R = IoReg_Read32(PPOVERLAY_osd_dtg_dh_width_reg);
		vbe_disp_suspend_resume.PPOVERLAY_osd_dtg_dv_length_S_R = IoReg_Read32(PPOVERLAY_osd_dtg_dv_length_reg);
	}

	/*GoldenVsyn & twoStepUzu delay*/
	vbe_disp_suspend_resume.PPOVERLAY_MEMCDTG_CONTROL2_S_R = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL2_reg);
	vbe_disp_suspend_resume.PPOVERLAY_MEMCDTG_CONTROL3_S_R = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL3_reg);
	vbe_disp_suspend_resume.PPOVERLAY_uzudtg_control1_S_R	= IoReg_Read32(PPOVERLAY_uzudtg_control1_reg);
	vbe_disp_suspend_resume.PPOVERLAY_uzudtg_control2_S_R	= IoReg_Read32(PPOVERLAY_uzudtg_control2_reg);
    vbe_disp_suspend_resume.PPOVERLAY_srnn_control_S_R	= IoReg_Read32(PPOVERLAY_srnn_control_reg);

	 /*SFG*/
	vbe_disp_suspend_resume.SFG_SFG_FORCE_BG_AT_DIF_S_R = IoReg_Read32(SFG_SFG_FORCE_BG_AT_DIF_reg);
	vbe_disp_suspend_resume.SFG_SFG_CTRL_0_S_R = IoReg_Read32(SFG_SFG_CTRL_0_reg);
 	 /*Clk*/
	vbe_disp_suspend_resume.CRT_DCLK_GATE_SEL0_S_R = IoReg_Read32(SYS_REG_SYS_DCLK_GATE_SEL0_reg);
	vbe_disp_suspend_resume.CRT_DCLK_GATE_SEL1_S_R = IoReg_Read32(SYS_REG_SYS_DCLK_GATE_SEL1_reg);
  	vbe_disp_suspend_resume.CRT_PLL_SSC0_S_R = IoReg_Read32(PLL27X_REG_PLL_SSC0_reg);
  	vbe_disp_suspend_resume.CRT_PLL_SSC3_S_R = IoReg_Read32(PLL27X_REG_PLL_SSC3_reg);
	vbe_disp_suspend_resume.CRT_PLL_SSC4_S_R = IoReg_Read32(PLL27X_REG_PLL_SSC4_reg);
	vbe_disp_suspend_resume.CRT_SYS_DCLKSS_S_R = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
	vbe_disp_suspend_resume.CRT_SYS_PLL_DISP1_S_R = IoReg_Read32(PLL27X_REG_SYS_PLL_DISP1_reg);
	vbe_disp_suspend_resume.CRT_SYS_PLL_DISP2_S_R = IoReg_Read32(PLL27X_REG_SYS_PLL_DISP2_reg);
	vbe_disp_suspend_resume.CRT_SYS_PLL_DISP3_S_R = IoReg_Read32(PLL27X_REG_SYS_PLL_DISP3_reg);
	vbe_disp_suspend_resume.CRT_SYS_DISPCLKSEL_S_R = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	vbe_disp_panel_suspend();
}

void vbe_disp_init_flow_pre_settings(void)
{
	sfg_sfg_clken_ctrl_RBUS sfg_sfg_clken_ctrl_reg;
	//disable lane power
	vbe_disp_lane_power_off();
	sfg_sfg_clken_ctrl_reg.regValue = IoReg_Read32(SFG_SFG_CLKEN_CTRL_reg);
	if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_4K2K)  ||
		(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS) ||
		(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) ||
		(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)){
		sfg_sfg_clken_ctrl_reg.clken_sfg = 1;
		sfg_sfg_clken_ctrl_reg.clken_sfg_osd = 1;
		sfg_sfg_clken_ctrl_reg.rbus_clken_vby1 = 1;
		sfg_sfg_clken_ctrl_reg.rbus_clken_vby1_osd = 1;//0x0000C003
		rtd_outl(SFG_SFG_CLKEN_CTRL_reg, sfg_sfg_clken_ctrl_reg.regValue);
		//TXPLL reset, rtd_part_outl(0xb8000C20, 10, 10, 0x0);
		vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_S_R &= ~_BIT10;
		//VBy1 MAC disable,  rtd_part_outl(0xb802D70C, 31, 31, 0x0);
		vbe_disp_suspend_resume.VBY1_TRANSMIT_CONTROL_S_R &= ~(_BIT31);
		//Disable Async FIFO,	rtd_outl(0xb802DC94, 0x00000000);
		vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL0_S_R &= ~_BIT0;
	}else if((Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE)||(Get_DISPLAY_PANEL_TYPE() == P_CEDS)){
		sfg_sfg_clken_ctrl_reg.clken_sfg = 1;
		sfg_sfg_clken_ctrl_reg.rbus_clken_epi = 1;
		sfg_sfg_clken_ctrl_reg.clken_tcon = 1;
		sfg_sfg_clken_ctrl_reg.rbus_clken_tcon = 1;//0x00110101
		rtd_outl(SFG_SFG_CLKEN_CTRL_reg, sfg_sfg_clken_ctrl_reg.regValue);
		//TXPLL reset, rtd_part_outl(0xb8000C20, 10, 10, 0x0);
		vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_S_R &= ~_BIT10;
		// EPI MAC Disable  rtd_part_outl(0xb802606C, 16, 16, 0x0);
		vbe_disp_panel_epi_suspend_resume.EPI_EPI_PORT_OPTION_CTRL_S_R &= ~_BIT16;
		//Disable Async FIFO,	rtd_outl(0xb802DC94, 0x00000000);
		vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL0_S_R &= ~_BIT0;
	}
	else if((Get_DISPLAY_PANEL_TYPE() == P_EDP_4K2K)||(Get_DISPLAY_PANEL_TYPE() == P_EDP_4K2K_2PORT)
		|| (Get_DISPLAY_PANEL_TYPE() == P_EDP_3K1K) ||(Get_DISPLAY_PANEL_TYPE() == P_EDP_4K1K)){
		sfg_sfg_clken_ctrl_reg.clken_sfg = 1;
		sfg_sfg_clken_ctrl_reg.clken_sfg_osd = 1;
		sfg_sfg_clken_ctrl_reg.clken_vby1 = 1;
		sfg_sfg_clken_ctrl_reg.rbus_clken_vby1 = 1;
		sfg_sfg_clken_ctrl_reg.rbus_clken_edptx = 1;//0x00404007
		rtd_outl(SFG_SFG_CLKEN_CTRL_reg, sfg_sfg_clken_ctrl_reg.regValue);
	}
#if 0 // not use in Merlin8
	}else if((Get_DISPLAY_PANEL_TYPE() == P_LVDS_2K1K)||(Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI)){
		//TXPLL reset, rtd_part_outl(0xb8000C20, 10, 10, 0x0);
		vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_S_R &= ~_BIT10;
		//EPI Clock disable , rtd_part_outl(0xb802D9B8, 31, 29, 0x0);
		vbe_disp_suspend_resume.SFG_SFG_CLKEN_CTRL_S_R &= ~(_BIT31|_BIT30|_BIT29);
		// LVDS MAC Enable (Below Setting), rtd_part_outl(0xb802D008, 7, 4, 0);
		vbe_disp_suspend_resume.PIF_LVDS_CTRL3_S_R &= ~(_BIT4|_BIT5|_BIT6|_BIT7);
		//Disable Async FIFO,	rtd_outl(0xb802DC94, 0x00000000);
		vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL0_S_R &= ~_BIT0;
	}
#endif

	rtd_pr_vbe_emerg("[vbe_disp_init_flow_pre_settings] PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_S_R:%x\n",vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_S_R);
	rtd_pr_vbe_emerg("[vbe_disp_init_flow_pre_settings] SFG_SFG_DUMMY_S_R:%x\n",vbe_disp_suspend_resume.SFG_SFG_CLKEN_CTRL_S_R);
	rtd_pr_vbe_emerg("[vbe_disp_init_flow_pre_settings] VBY1_TRANSMIT_CONTROL_S_R:%x\n",vbe_disp_suspend_resume.VBY1_TRANSMIT_CONTROL_S_R);
	rtd_pr_vbe_emerg("[vbe_disp_init_flow_pre_settings] EPI_EPI_PORT_OPTION_CTRL_S_R:%x\n",vbe_disp_panel_epi_suspend_resume.EPI_EPI_PORT_OPTION_CTRL_S_R);
	rtd_pr_vbe_emerg("[vbe_disp_init_flow_pre_settings] PIF_PIF_FIFO_EN_S_R:%x\n",vbe_disp_suspend_resume.SFG_PIF_FIFO_CTRL0_S_R);
}

void vbe_disp_init_flow_finish(void)
{

	if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_4K2K) ||
		(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS) ||
		(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI) ||
		(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_LVDS_TO_HDMI)){
		// (5) TXPLL reset release (Below Setting) , rtd_part_outl(0xb8000C20, 10, 10, 0x1);
		IoReg_SetBits(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_reg,PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_pif_h_cmu_rstb_mask);
		// (6) Wait TXPLL stable at least 150us (SW Control)
		mdelay(1);
		// ClkEn      for[10]:eDP, [8]:TCON,  [6]:P2P,  [5]:ISP,  [4]:EPI,  [3]:Vby1_osd,  [2]:Vby1,  [1]:sfg_osd, [0]:sfg 
		//rtd_part_outl(0xb802D9B8, 2, 2, 0x1);
		IoReg_SetBits(SFG_SFG_CLKEN_CTRL_reg,SFG_SFG_CLKEN_CTRL_clken_vby1_mask);
		// (8) VBy1 MAC Enable (Below Setting), rtd_part_outl(0xb802D70C, 31, 31, 0x1);
		IoReg_SetBits(VBY1_TRANSMIT_CONTROL_reg,VBY1_TRANSMIT_CONTROL_reg_en_mask);
		// (9) Enable Async FIFO,	rtd_outl(0xb802DC94, 0x80000000);
		IoReg_SetBits(SFG_PIF_FIFO_CTRL0_reg,SFG_PIF_FIFO_CTRL0_pif_fifo_en_mask);
	}else if((Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE)||(Get_DISPLAY_PANEL_TYPE() == P_CEDS)){
		// (5) TXPLL reset release (Below Setting) rtd_part_outl(0xb8000C20, 10, 10, 0x1);
		IoReg_SetBits(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_reg,PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_pif_h_cmu_rstb_mask);
		// (6) Wait TXPLL stable at least 150us (SW Control)
		mdelay(1);
		// (7) EPI Clock Enable (Below Setting) rtd_part_outl(0xb802D9B8, 4, 4, 0x1);
		IoReg_SetBits(SFG_SFG_CLKEN_CTRL_reg,SFG_SFG_CLKEN_CTRL_clken_epi_mask);
		// (8) EPI MAC Enable (Below Setting), rtd_part_outl(0xb802606C, 16, 16, 0x1);
		IoReg_SetBits(EPI_EPI_PORT_OPTION_CTRL_reg,EPI_EPI_PORT_OPTION_CTRL_epi_enable_mask);
		// (9) Enable Async FIFO,	rtd_outl(0xb802DC94, 0x80000000);
		IoReg_SetBits(SFG_PIF_FIFO_CTRL0_reg,SFG_PIF_FIFO_CTRL0_pif_fifo_en_mask);
	}
	else if((Get_DISPLAY_PANEL_TYPE() == P_EDP_4K2K)||(Get_DISPLAY_PANEL_TYPE() == P_EDP_4K2K_2PORT)
		|| (Get_DISPLAY_PANEL_TYPE() == P_EDP_3K1K) ||(Get_DISPLAY_PANEL_TYPE() == P_EDP_4K1K)){
		// (5) TXPLL reset release (Below Setting) rtd_part_outl(0xb8000C20, 10, 10, 0x1);
		IoReg_SetBits(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_reg,PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_pif_h_cmu_rstb_mask);
		// (6) Wait TXPLL stable at least 150us (SW Control) rtd_part_outl(0x18000EA0, 29, 29, 0x1);
		IoReg_SetBits(PINMUX_LVDSPHY_CMU_SSC_CTRL_00_reg,PINMUX_LVDSPHY_CMU_SSC_CTRL_00_oc_en_edptx_mask);
		//
		// Wait DPLL stable at least 100us (SW Control) rtd_part_outl(0x18000EA0, 31, 31, 0x1);
		IoReg_SetBits(PINMUX_LVDSPHY_CMU_SSC_CTRL_00_reg,PINMUX_LVDSPHY_CMU_SSC_CTRL_00_pll_edptx_lclk_en_mask);

		// (6) Wait TXPLL stable at least 150us (SW Control)
		mdelay(1);
		// (7) EDPClock Enable (Below Setting),rtd_part_outl(0xb802D9B8, 10, 10, 0x1);
		IoReg_SetBits(SFG_SFG_CLKEN_CTRL_reg,SFG_SFG_CLKEN_CTRL_clken_edptx_mask);
		// (9) Enable Async FIFO,	rtd_outl(0xb802DC94, 0x80000000);
		IoReg_SetBits(SFG_PIF_FIFO_CTRL0_reg,SFG_PIF_FIFO_CTRL0_pif_fifo_en_mask);
	}
#if 0 // not use in Merlin8
	}else if((Get_DISPLAY_PANEL_TYPE() == P_LVDS_2K1K)||(Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI)){
		//TXPLL reset, rtd_part_outl(0xb8000C20, 10, 10, 0x1);
		IoReg_SetBits(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_2_3_reg,_BIT10);
		//EPI Clock disable , rtd_part_outl(0xb802D9B8, 31, 29, _BIT31);
		IoReg_SetBits(SFG_SFG_CLKEN_CTRL_reg,_BIT31);
		// LVDS MAC Enable (Below Setting), rtd_part_outl(0xb802D008, 7, 4, 0xc);
		IoReg_SetBits(PIF_LVDS_CTRL3_reg, (_BIT6|_BIT7));
		// (9) Enable Async FIFO,	rtd_outl(0xb802DC94, 0x80000000);
		IoReg_SetBits(SFG_PIF_FIFO_CTRL0_reg,_BIT31);	//Merlin5 removed
	}
#endif 
	// (10) APHY Lane Power Enable (Below Setting)
	if((Get_DISPLAY_PANEL_TYPE() == P_CEDS)||(Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE)||(Get_DISPLAY_PANEL_TYPE() == P_VBY1_4K2K)){
        //control on HAL_VBE_DISP_SetDisplayOutput
    }else
	    vbe_disp_lane_power_on();
}


void vbe_disp_resume(void)
{
    ppoverlay_dtg_ie_2_RBUS ppoverlay_dtg_ie_2_reg;
	unsigned int tmp;
//	unsigned char timeout;
//	pll27x_reg_pll_ssc5_RBUS pll_ssc5_reg;
//	UINT32 cnt=0;
	//frank@1114 add below flow to set up Ddomain related register
	ppoverlay_dispd_smooth_toggle1_RBUS dispd_smooth_toggle1_reg;
	sys_reg_sys_dispclksel_RBUS sys_dispclksel_reg;
//	sys_reg_sys_dclk_gate_sel0_RBUS sys_reg_sys_dclk_gate_sel0_reg;
//	sys_reg_sys_dclk_gate_sel1_RBUS sys_reg_sys_dclk_gate_sel1_reg;
//	ppoverlay_dispd_smooth_toggle1_RBUS ppoverlay_dispd_smooth_toggle1_reg;
	scaleup_dm_uzu_input_size_RBUS scaleup_dm_uzu_input_size_reg;

	//dispd_rst for crt.
	sys_reg_sys_srst1_RBUS sys_reg_sys_srst1_reg;
	sys_reg_sys_srst1_reg.regValue = 0;
	sys_reg_sys_srst1_reg.rstn_dispd = 1;
	sys_reg_sys_srst1_reg.write_data = 1;
	IoReg_Write32(SYS_REG_SYS_SRST1_reg, sys_reg_sys_srst1_reg.regValue);

	sys_dispclksel_reg.regValue = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	sys_dispclksel_reg.clken_disp_stage1 = 1;
	sys_dispclksel_reg.clken_disp_memc = 1;
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, sys_dispclksel_reg.regValue);

        //enable dtg_ie2 interrupt
        ppoverlay_dtg_ie_2_reg.regValue = rtd_inl(PPOVERLAY_DTG_ie_2_reg);
        ppoverlay_dtg_ie_2_reg.dtg_ie_2 = 1;
        ppoverlay_dtg_ie_2_reg.ivs_rm_rising_ie_2 = 1;
        ppoverlay_dtg_ie_2_reg.ivs_dly_rm_rising_ie_2 = 1;
        ppoverlay_dtg_ie_2_reg.mv_den_sta_event_ie_2 = 1;
        ppoverlay_dtg_ie_2_reg.mv_den_end_event_ie_2 = 1;
        ppoverlay_dtg_ie_2_reg.memcdtgv_den_sta_event_ie_2 = 1;//for orbit
        ppoverlay_dtg_ie_2_reg.memcdtgv_den_end_event_ie_2 = 1;//for orbit
        rtd_outl(PPOVERLAY_DTG_ie_2_reg, ppoverlay_dtg_ie_2_reg.regValue);
#if 0
	//for ECN RL6748-1899, DIC recommand setting before disp_en = 1
	scaleup_dm_uzu_input_size_reg.regValue = 0;
        //constraint: vertical input size and horizontal input_siz need > 0 ;
        // 0x40 is DIC comment value, so we set this value here.
	scaleup_dm_uzu_input_size_reg.hor_input_size = 0x40;
	scaleup_dm_uzu_input_size_reg.ver_input_size = 0x40;
	IoReg_Write32(SCALEUP_DM_UZU_Input_Size_reg, scaleup_dm_uzu_input_size_reg.regValue);	// Input Size

	sys_reg_sys_dclk_gate_sel0_reg.regValue = rtd_inl(SYS_REG_SYS_DCLK_GATE_SEL0_reg);
	sys_reg_sys_dclk_gate_sel1_reg.regValue = rtd_inl(SYS_REG_SYS_DCLK_GATE_SEL1_reg);

	if(Get_DISPLAY_REFRESH_RATE() >= 120){
#ifdef CONFIG_MEMC_BYPASS
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f1p_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f2p_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f1p_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f2p_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_osd_f1p_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_osd_f2p_gate_sel = 2;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_memc_in_if_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_memc_out_if_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_ippre_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_ippre2x_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_s2_f2p_div2_gate_sel = 1;
#else
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f1p_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f2p_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f1p_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f2p_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_osd_f1p_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_osd_f2p_gate_sel = 2;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_memc_in_if_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_memc_out_if_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_ippre_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_ippre2x_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_s2_f2p_div2_gate_sel = 1;
#endif
	}else{
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f1p_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f2p_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f1p_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f2p_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_osd_f1p_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel0_reg.dclk_osd_f2p_gate_sel = 2;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_memc_in_if_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_memc_out_if_gate_sel = 2;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_ippre_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_ippre2x_gate_sel = 0;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_gate_sel = 1;
		sys_reg_sys_dclk_gate_sel1_reg.dclk_s2_f2p_div2_gate_sel = 1;
	}
	rtd_outl(SYS_REG_SYS_DCLK_GATE_SEL0_reg, sys_reg_sys_dclk_gate_sel0_reg.regValue);
	rtd_outl(SYS_REG_SYS_DCLK_GATE_SEL1_reg, sys_reg_sys_dclk_gate_sel1_reg.regValue);

	ppoverlay_dispd_smooth_toggle1_reg.regValue = 0;
	//stage1 clk mode revised default set to 1
	ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_clk_mode_revised = 1;
	ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_src_sel = 1;
	ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
	rtd_outl(PPOVERLAY_dispd_smooth_toggle1_reg, ppoverlay_dispd_smooth_toggle1_reg.regValue);
#endif

	if( Scaler_MEMC_CLK_Check() == FALSE ){
		rtd_pr_vbe_notice("%s %d\n",__func__,__LINE__);
		rtd_pr_vbe_notice("Scaler_MEMC_CLK_Check = FALSE! Do Scaler_MEMC_CLK_Enable()!\n");
		Scaler_MEMC_CLK_Enable();//Enable MEMC CLK
	}

	//remove pmic in str driver for KTASKWBS-7931
	//if(Get_PANEL_TYPE_SUPPORT_PMIC() == TRUE)
	//	panel_setup_PMIC();

	if(/*(IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg) & _BIT24) &&*/ !(first_disp_resume)) {
		rtd_pr_vbe_notice("[vbe_disp_resume] Already Set, so no need to set again\n");
#if !defined(CONFIG_CUSTOMER_TV006) && IS_ENABLED(CONFIG_RTK_KDRV_GDMA)
		gdma_resume_by_vbe();
#endif
		return;
	}
    
    //for ECN RL6748-1899, DIC recommand setting before disp_en = 1
    scaleup_dm_uzu_input_size_reg.regValue = IoReg_Read32(SCALEUP_DM_UZU_Input_Size_reg);
    if((scaleup_dm_uzu_input_size_reg.hor_input_size == 0) || (scaleup_dm_uzu_input_size_reg.ver_input_size == 0))
    {
        scaleup_dm_uzu_input_size_reg.regValue = 0;
            //constraint: vertical input size and horizontal input_siz need > 0 ;
            // 0x40 is DIC comment value, so we set this value here.
        scaleup_dm_uzu_input_size_reg.hor_input_size = 0x40;
        scaleup_dm_uzu_input_size_reg.ver_input_size = 0x40;
        IoReg_Write32(SCALEUP_DM_UZU_Input_Size_reg, scaleup_dm_uzu_input_size_reg.regValue);   // Input Size
    }

	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, 0 | _BIT1|_BIT9|_BIT13|_BIT17);	//clear double buffer first to make all settings can apply directly
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, 0 | _BIT21 | _BIT17| _BIT13);	//clear double buffer first to make all settings can apply directly

	IoReg_SetBits(PPOVERLAY_Main_Display_Control_RSV_reg, _BIT1);//Let main default value enable force bg
	IoReg_SetBits(PPOVERLAY_MP_Layout_Force_to_Background_reg, _BIT1);//Let sub default value enable force bg

	if(first_disp_resume)
		first_disp_resume = FALSE;

	rtd_pr_vbe_debug("[vbe_disp_resume] step 1, turn on VBy1 APHY Power\n");
	vbe_disp_init_flow_pre_settings();

	/*DTG*/
	//frank@02022016 IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, (vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_TIMING_CTRL1_S_R&(~_BIT24)));
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, (vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_TIMING_CTRL1_S_R&(~(_BIT24|_BIT26))));
	rtd_pr_vbe_debug("[vbe_disp_resume] PPOVERLAY_Display_Timing_CTRL1_reg:%x !\n", IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg));
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_TIMING_CTRL2_S_R);
	IoReg_Write32(PPOVERLAY_DV_Length_reg, vbe_disp_suspend_resume.PPOVERLAY_DV_LENGTH_S_R);
	IoReg_Write32(PPOVERLAY_DV_total_reg, vbe_disp_suspend_resume.PPOVERLAY_DV_TOTAL_S_R);
	//IoReg_Write32(PPOVERLAY_DTG_pending_status_reg, vbe_disp_suspend_resume.PPOVERLAY_DTG_PENDING_STATUS_S_R);
	IoReg_Write32(PPOVERLAY_Display_Background_Color_reg, vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_BACKGROUND_COLOR_S_R);
	IoReg_Write32(PPOVERLAY_DH_Width_reg, vbe_disp_suspend_resume.PPOVERLAY_DH_WIDTH_S_R);
	IoReg_Write32(PPOVERLAY_DTG_ie_reg, vbe_disp_suspend_resume.PPOVERLAY_DTG_IE_S_R);
	//IoReg_Write32(PPOVERLAY_DTG_ie_2_reg, vbe_disp_suspend_resume.PPOVERLAY_DTG_IE_2_S_R);
	IoReg_Write32(PPOVERLAY_DH_Total_Last_Line_Length_reg, vbe_disp_suspend_resume.PPOVERLAY_DH_TOTAL_LAST_LINE_LENGTH_S_R);
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, vbe_disp_suspend_resume.SYS_DISPCLKSEL_S_R);
	IoReg_Write32(PPOVERLAY_FS_IV_DV_Fine_Tuning2_reg, vbe_disp_suspend_resume.PPOVERLAY_FS_IV_DV_FINE_TUNING2_S_R);
	IoReg_Write32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg, vbe_disp_suspend_resume.PPOVERLAY_FS_IV_DV_FINE_TUNING5_S_R);
	IoReg_Write32(PPOVERLAY_FS_IV_DV_Fine_Tuning1_reg, vbe_disp_suspend_resume.PPOVERLAY_FS_IV_DV_FINE_TUNING1_S_R);
	IoReg_Write32(PPOVERLAY_uzudtg_DV_TOTAL_reg , vbe_disp_suspend_resume.PPOVERLAY_uzudtg_DV_TOTAL_S_R  );
	IoReg_Write32(PPOVERLAY_uzudtg_DH_TOTAL_reg , vbe_disp_suspend_resume.PPOVERLAY_uzudtg_DH_TOTAL_S_R  );
	IoReg_Write32(PPOVERLAY_DH_DEN_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_DH_DEN_Start_End_S_R );
	IoReg_Write32(PPOVERLAY_DV_DEN_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_DV_DEN_Start_End_S_R );
	IoReg_Write32(PPOVERLAY_UZUDTG_COUNTROL_reg , vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_COUNTROL_S_R  );
	IoReg_Write32(PPOVERLAY_memcdtg_DV_TOTAL_reg, vbe_disp_suspend_resume.PPOVERLAY_memcdtg_DV_TOTAL_S_R );
	IoReg_Write32(PPOVERLAY_memcdtg_DH_TOTAL_reg, vbe_disp_suspend_resume.PPOVERLAY_memcdtg_DH_TOTAL_S_R );
	IoReg_Write32(PPOVERLAY_memcdtg_FREE_DV_TOTAL_reg, vbe_disp_suspend_resume.PPOVERLAY_memcdtg_FREE_DV_TOTAL_S_R );
	IoReg_Write32(PPOVERLAY_memcdtg_FREE_DH_TOTAL_reg, vbe_disp_suspend_resume.PPOVERLAY_memcdtg_FREE_DH_TOTAL_S_R );
	IoReg_Write32(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_memcdtg_DH_DEN_Start_End_S_R);
	IoReg_Write32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_memcdtg_DV_DEN_Start_End_S_R);
	IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL_reg, vbe_disp_suspend_resume.PPOVERLAY_MEMCDTG_CONTROL_S_R  );
	//IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg  , vbe_disp_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_S_R|_BIT12); //@benwang marked for resume test
	IoReg_Write32(PPOVERLAY_MEMC_MUX_CTRL_reg  , vbe_disp_suspend_resume.PPOVERLAY_MEMC_MUX_CTRL_S_R);
	IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg,vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_FRACTIONAL_FSYNC_S_R);
	IoReg_Write32(PPOVERLAY_DTG_M_Remove_input_vsync_reg,vbe_disp_suspend_resume.PPOVERLAY_DTG_M_REMOVE_INPUT_VSYNC_S_R);

	/*Aisr mode & s1ip delay*/
	IoReg_Write32(PPOVERLAY_uzudtg_control3_reg,vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_CONTROL3_S_R);
	IoReg_Write32(PPOVERLAY_uzudtg_control5_reg,vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_CONTROL5_S_R);

	/*Time borrow*/
	IoReg_Write32(PPOVERLAY_uzudtg_stage1_time_borrow_reg,vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_STAGE1_TIME_BORROW_S_R);

	/*Orbit overscan*/
	IoReg_Write32(PPOVERLAY_uzudtg_orbit_main_ctrl0_reg,vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_ORBIT_MAIN_CTRL0_S_R);
	IoReg_Write32(PPOVERLAY_uzudtg_orbit_main_ctrl2_reg,vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_ORBIT_MAIN_CTRL2_S_R);
	IoReg_Write32(PPOVERLAY_uzudtg_orbit_debug_ctrl0_reg,vbe_disp_suspend_resume.PPOVERLAY_UZUDTG_ORBIT_DEBUG_CTRL0_S_R);

	/*Orbit justscan*/
	IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL_4_reg,vbe_disp_suspend_resume.PPOVERLAY_MEMCDTG_CONTROL_4_S_R);
	IoReg_Write32(PPOVERLAY_SRAM_OK_main_Result_even_reg,vbe_disp_suspend_resume.PPOVERLAY_SRAM_OK_main_Result_even_S_R);
	IoReg_Write32(PPOVERLAY_ORBIT_BG_COLOR2_reg,vbe_disp_suspend_resume.PPOVERLAY_ORBIT_BG_COLOR2_S_R);

	// for OSD split
	if((Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_4K2K_OSD2K1K_to_H5X_SPLIT) ||(Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_4K2K_OSD1K2K_to_H5X_SPLIT)){
		IoReg_Write32(PPOVERLAY_osddtg_DV_TOTAL_reg, vbe_disp_suspend_resume.PPOVERLAY_osddtg_DV_TOTAL_S_R);
		IoReg_Write32(PPOVERLAY_osddtg_DH_TOTAL_reg, vbe_disp_suspend_resume.PPOVERLAY_osddtg_DH_TOTAL_S_R);
		IoReg_Write32(PPOVERLAY_osddtg_DV_DEN_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_osddtg_DV_DEN_Start_End_S_R);
		IoReg_Write32(PPOVERLAY_osddtg_DH_DEN_Start_End_reg, vbe_disp_suspend_resume.PPOVERLAY_osddtg_DH_DEN_Start_End_S_R);
		IoReg_Write32(PPOVERLAY_OSDDTG_CONTROL_reg, vbe_disp_suspend_resume.PPOVERLAY_OSDDTG_CONTROL_S_R);
		IoReg_Write32(PPOVERLAY_OSDDTG_CONTROL2_reg, vbe_disp_suspend_resume.PPOVERLAY_OSDDTG_CONTROL2_S_R);
		IoReg_Write32(PPOVERLAY_osddtg_fractional_fsync_reg, vbe_disp_suspend_resume.PPOVERLAY_osddtg_fractional_fsync_S_R);
		IoReg_Write32(PPOVERLAY_osd_dtg_dh_width_reg, vbe_disp_suspend_resume.PPOVERLAY_osd_dtg_dh_width_S_R);
		IoReg_Write32(PPOVERLAY_osd_dtg_dv_length_reg, vbe_disp_suspend_resume.PPOVERLAY_osd_dtg_dv_length_S_R);
	}

	/*GoldenVsyn & twoStepUzu delay*/
	IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL2_reg,vbe_disp_suspend_resume.PPOVERLAY_MEMCDTG_CONTROL2_S_R);
	IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL3_reg,vbe_disp_suspend_resume.PPOVERLAY_MEMCDTG_CONTROL3_S_R);
	IoReg_Write32(PPOVERLAY_uzudtg_control1_reg,vbe_disp_suspend_resume.PPOVERLAY_uzudtg_control1_S_R);
	IoReg_Write32(PPOVERLAY_uzudtg_control2_reg,vbe_disp_suspend_resume.PPOVERLAY_uzudtg_control2_S_R);
    IoReg_Write32(PPOVERLAY_srnn_control_reg,vbe_disp_suspend_resume.PPOVERLAY_srnn_control_S_R);

	/*SFG*/
	//[QEVENT-7662]Fixed OLED panel EIT function sfg setting issue @Crixus 20160527
	if((Get_DISPLAY_PANEL_OLED_TYPE() == TRUE) && (oled_resume_eit_flag == TRUE)){
		IoReg_Write32(SFG_SFG_FORCE_BG_AT_DIF_reg, (vbe_disp_suspend_resume.SFG_SFG_FORCE_BG_AT_DIF_S_R | _BIT31));// In OLED EIT function, resume must enable SFG forceBG @Crixus 20160527
		oled_resume_eit_flag = FALSE;
	}
	else{
		IoReg_Write32(SFG_SFG_FORCE_BG_AT_DIF_reg, (vbe_disp_suspend_resume.SFG_SFG_FORCE_BG_AT_DIF_S_R & ~_BIT31));// resume must disable SFG forceBG @Crixus 20160124
	}
	//IoReg_Write32(SFG_SFG_CTRL_0_reg, vbe_disp_suspend_resume.SFG_SFG_CTRL_0_S_R);
	/*Clk*/
	IoReg_Write32(SYS_REG_SYS_DCLK_GATE_SEL0_reg, vbe_disp_suspend_resume.CRT_DCLK_GATE_SEL0_S_R);
	IoReg_Write32(SYS_REG_SYS_DCLK_GATE_SEL1_reg, vbe_disp_suspend_resume.CRT_DCLK_GATE_SEL1_S_R);
	IoReg_Write32(PLL27X_REG_PLL_SSC0_reg, vbe_disp_suspend_resume.CRT_PLL_SSC0_S_R&~_BIT0);
	IoReg_Write32(PLL27X_REG_PLL_SSC3_reg, vbe_disp_suspend_resume.CRT_PLL_SSC3_S_R);
	IoReg_Write32(PLL27X_REG_PLL_SSC4_reg, vbe_disp_suspend_resume.CRT_PLL_SSC4_S_R);
	IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, vbe_disp_suspend_resume.CRT_SYS_DCLKSS_S_R);
	IoReg_Write32(PLL27X_REG_SYS_PLL_DISP1_reg, vbe_disp_suspend_resume.CRT_SYS_PLL_DISP1_S_R);
	IoReg_Write32(PLL27X_REG_SYS_PLL_DISP2_reg, vbe_disp_suspend_resume.CRT_SYS_PLL_DISP2_S_R);
	IoReg_Write32(PLL27X_REG_SYS_PLL_DISP3_reg, vbe_disp_suspend_resume.CRT_SYS_PLL_DISP3_S_R);
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, vbe_disp_suspend_resume.CRT_SYS_DISPCLKSEL_S_R);
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ((vbe_disp_suspend_resume.PPOVERLAY_DOUBLE_BUFFER_CTRL_S_R)&(~(_BIT2|_BIT10|_BIT14|_BIT18)))|_BIT0);	//disable DTG double buffer
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, 0 | _BIT21 | _BIT17| _BIT13);	//disable DTG double buffer

	IoReg_Write32(PLL27X_REG_PLL_SSC0_reg, (vbe_disp_suspend_resume.CRT_PLL_SSC0_S_R|_BIT0));

	mdelay(1); //150us for dpll hw apply
	IoReg_ClearBits(PLL27X_REG_PLL_SSC0_reg,_BIT0);
	vbe_disp_panel_resume();

	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, ((vbe_disp_suspend_resume.PPOVERLAY_DISPLAY_TIMING_CTRL1_S_R&(~(_BIT26)))|_BIT24));
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ((vbe_disp_suspend_resume.PPOVERLAY_DOUBLE_BUFFER_CTRL_S_R)&(~(_BIT2|_BIT10|_BIT14|_BIT18)))|_BIT0);	//disable DTG double buffer

	tmp = 0;
	while(IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg)&_BIT0){
		mdelay(2);
		if(++tmp>30)
			break;
	}
	if(tmp > 30){
		rtd_pr_vbe_notice("[vbe_disp_resume]Wait Display double buffer reg timeout, 90k(%d)\n",IoReg_Read32(0xB801B6B8));
	}

	//apply uzudtg clk double buffer
	dispd_smooth_toggle1_reg.regValue = 0;
	//stage1 clk mode revised default set to 1
	dispd_smooth_toggle1_reg.dispd_stage1_clk_mode_revised = 1;
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_src_sel = 1;
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);

	rtd_pr_vbe_notice("[vbe_disp_resume] d domain ready\n");

	vbe_disp_init_flow_finish();
	stBacklihgtStart_Tick = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
	//clear first boot flag
	first_boot_run_main = FALSE;

	//power on sequence : panel to LVDS on
	if(Get_DISPLAY_PANEL_TYPE()==P_LVDS_2K1K){
		rtd_pr_vbe_emerg("[vbe_disp_resume] Get_PANEL_TO_LVDS_ON_ms = %d\n", Get_PANEL_TO_LVDS_ON_ms());
		msleep(Get_PANEL_TO_LVDS_ON_ms());
	}

#if IS_ENABLED(CONFIG_RTK_KDRV_GDMA) && !defined(CONFIG_CUSTOMER_TV006)
	gdma_resume_by_vbe();
#endif

}


void vbe_disp_early_resume(void)
{
	vbe_disp_resume();

}
EXPORT_SYMBOL(vbe_disp_early_resume);

#endif

bool vbe_get_data_frame_sync_flag(void)
{
	return disp_data_frame_sync_flag;
}

void HAL_VBE_DISP_DataFrameSync(bool enable)
{
	if (enable != disp_data_frame_sync_flag) {
		disp_data_frame_sync_flag = enable;
	}
}

/*******************************************************************************
****************************VBE DISP TCON DRIVER********************************
*******************************************************************************/
void HAL_VBE_DISP_TCON_Initialize(void){
	rtd_pr_vbe_debug("###############%s\n",__FUNCTION__);
}
void HAL_VBE_DISP_TCON_Uninitialize(void){
	//to do
	rtd_pr_vbe_debug("###############%s\n",__FUNCTION__);
}
void HAL_VBE_DISP_TCON_EnableColorTemp(bool bOnOff){
	//to do
	rtd_pr_vbe_debug("###############%s\n",__FUNCTION__);
}
void HAL_VBE_DISP_TCON_EnableDGA(bool bOnOff){
	//to do
	rtd_pr_vbe_debug("###############%s\n",__FUNCTION__);
}
void HAL_VBE_DISP_TCON_EnableDither(bool bOnOff){
	Color_Fun_Bypass bDither;
	rtd_pr_vbe_debug("###############%s, enable = %d\n",__FUNCTION__,bOnOff);
	bDither.idIP = BYPASS_D_DITHER;
	bDither.bypass_switch = bOnOff;

	drvif_color_bypass(&bDither, 1);
}
void HAL_VBE_DISP_TCON_SetDitherMode(KADP_DISP_TCON_DITHER_T mode){
	rtd_pr_vbe_debug("###############%s\n",__FUNCTION__);
	fwif_color_dither_mode(mode);
}
void HAL_VBE_DISP_TCON_SetGammaTable(UINT32 *pRedGammaTable, UINT32 *pGreenGammaTable, UINT32 *pBlueGammaTable){
	//to do
	rtd_pr_vbe_debug("###############%s\n",__FUNCTION__);
}
void HAL_VBE_DISP_TCON_EnableTCon(bool bOnOff){
	rtd_pr_vbe_debug("###############%s\n",__FUNCTION__);
	HAL_VBE_DISP_TCON_EnableColorTemp(bOnOff);
	HAL_VBE_DISP_TCON_EnableDGA(bOnOff);
	HAL_VBE_DISP_TCON_EnableDither(bOnOff);
}
void HAL_VBE_DISP_TCON_H_Reverse(UINT8 u8Mode){
	sfg_sfg_ctrl_0_RBUS sfg_ctrl_0_reg;
	sfg_ctrl_0_reg.regValue = IoReg_Read32(SFG_SFG_CTRL_0_reg);

	rtd_pr_vbe_debug("###############%s, u8Mode = %x\n",__FUNCTION__,u8Mode);
	 //hflip
	if(u8Mode){
		sfg_ctrl_0_reg.h_flip_en = 1;
	}
	else{
		sfg_ctrl_0_reg.h_flip_en = 0;
	}

	if((Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE)||(Get_DISPLAY_PANEL_TYPE() == P_CEDS)){
		//EPI V16DRD Horizontal need hflip, so dirty patch it.
		sfg_ctrl_0_reg.h_flip_en = !sfg_ctrl_0_reg.h_flip_en;
	}

	IoReg_Write32(SFG_SFG_CTRL_0_reg, sfg_ctrl_0_reg.regValue);
}
void HAL_VBE_DISP_TCON_SetClock(bool bOnOff){
	rtd_pr_vbe_debug("###############%s\n",__FUNCTION__);
#if 0
	//---------------------------------------------------------------------------------------------------------------------------------
	//SFG1:Video, DH_ST_END = 0x008C080C, DTG_HS_Width = 16, DTG_VS_Width = 6
	//---------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------
	//SFG, 1 Pixel Mode, DH_ST_END = 0x008C080C, DTG_HS_Width = 16, DTG_VS_Width = 6
	//---------------------------------------------------------------------------------------------------------------------------------
	IoReg_Write32(0xb802D900, 0x000018C0);	// 8-bit Mode, Dout Enable, delay chain inverse + 0ns
	IoReg_Write32(0xb802D904, 0x0000FF00);	// PIF_Clock = Dclk, Force to background enable
	IoReg_Write32(0xb802D908, 0xffff3210);	// Port A~H mapping
	IoReg_Write32(0xb802D90c, 0xffffffff);
	IoReg_Write32(0xb802D92C, 0x00005000);	// Line 4n + 0: shift 1 sub-Pixel
						// Line 4n + 1: shift 1 sub-Pixel
						// Line 4n + 2: no sub-Pixel shift
						// Line 4n + 3: no sub-Pixel shift
	IoReg_Write32(0xb802D930, 0x00000000);	// SEG 0/1, Front/End, 4n+0/4n+1 Line, no dummy pixel
	IoReg_Write32(0xb802D934, 0x00000000);	// SEG 2/3, Front/End, 4n+2/4n+3 Line, no dummy pixel
	IoReg_Write32(0xb802D938, 0x00000000);	// SEG 4/5, Front/End, 4n+0/4n+1 Line, no dummy pixel
	IoReg_Write32(0xb802D93c, 0x00000000);	// SEG 6/7, Front/End, 4n+2/4n+3 Line, no dummy pixel
	IoReg_Write32(0xb802D954, 0x25310400);	// DRD input line 0 mapping
	IoReg_Write32(0xb802D958, 0x25310400);	// DRD input line 1 mapping
	IoReg_Write32(0xb802D95c, 0x25310400);	// DRD input line 2 mapping
	IoReg_Write32(0xb802D960, 0x25310400);	// DRD input line 3 mapping
	IoReg_Write32(0xb802D964, 0x10000133);	// DRD Data Mapping, 4-SEG, 4-Port, H-Flip
	IoReg_Write32(0xb802D968, 0x000001E0);	// start address of SEG 0/1 , residue of SEG 0/1
	IoReg_Write32(0xb802D96C, 0x03C005A0);	// start address of SEG 2/3 , residue of SEG 2/3
	IoReg_Write32(0xb802D970, 0x00000000);	// start address of SEG 4/5 , residue of SEG 4/5
	IoReg_Write32(0xb802D974, 0x00000000);	// start address of SEG 6/7 , residue of SEG 6/7
	IoReg_Write32(0xb802D988, 0x40000000);	// Disable ByPass Mode, Auto config mode enable, Disanle SFG P2S Mode
	IoReg_Write32(0xb802D98c, 0x000F088B);	// HS_Width = 16 dclk; hsync_delay = DH_Total  - 13 = 2187;
	IoReg_Write32(0xb802D990, 0x00050000);	// VS_ST = 0; VS_END = 6;
	IoReg_Write32(0xb802D994, 0x008C0086);	// sfg_dh_den_sta = DH_DEN_ST = 140;
						// req_st = hs_back_porch - 6 = 134;
	IoReg_Write32(0xb802D990,0x044C0000);
	IoReg_Write32(0xb802D99C, 0x03C00000);	// final_line_length = 1920/2, hsync_threshold = 0
	IoReg_Write32(0xb802D9a0, 0x00000000);	// Residue_pix_div_12_split_mode (no USE)
						// Final_addr_split_mode for Tri-gate Mode (no USE)
	IoReg_Write32(0xb802D9a4, 0x008b0892);	// hs_den_io_dist = req_st + 6 = 140; (for EPI)
						// hs_io_dist = hs_delay + 6 = 2187 + 6 = 2193; (for EPI)
	IoReg_Write32(0xb802D9B8, 0x00000000);	// Line 4N/4N+1/4N+2/4N+3 => G_0/G_last select original data,
						// decided by zz_shift_lst_sel_0/1/2/3 & zz_shift_last_sel_0/1/2/3 first
						// data not inverse
	IoReg_Write32(0xb802D9E8, 0x00000000);	// SEG 0/1, no middle dummy pixel
	IoReg_Write32(0xb802D9EC, 0x00000000);	// SEG 0/1, no middle dummy pixel
	IoReg_Write32(0xb802D9F0, 0x00000000);	// SEG 0/1, no middle dummy pixel
	IoReg_Write32(0xb802D9F4, 0x00000000);	// SEG 0/1, no middle dummy pixel
	IoReg_Write32(0xb802Da08, 0x00000000);	// Active_pixel_RGB for Tri-Gate Mode & Last_ext_pixel for EPI
	//---------------------------------------------------------------------------------------------------------------------------------

	//---------------------------------------------------------------------------------------------------------------------------------
	//PIF
	//---------------------------------------------------------------------------------------------------------------------------------
	IoReg_Write32(0xb802D000, 0x0000000C);	// PIF EPI Mode, data is Video, clock is Video_clk,
	IoReg_Write32(0xb802D010, 0x000050F0);	// LVDS A/B/C/D Port Enable, LSB First, LVDS format use Table1,
	IoReg_Write32(0xb802D100, 0x00010203);	// LVDS ARGB de_sel, vs_sel, hs_sel, rev0_sel,
	IoReg_Write32(0xb802D104, 0x04050607);	// LVDS ARGB rev0_sel, a1_sel, a2_sel,
	IoReg_Write32(0xb802D108, 0x0008090A);	// LVDS ARGB a3_sel, a4_sel, a5_sel,

	//---------------------------------------------------------------------------------------------------------------------------------
	//LVDS Pin Share
	//---------------------------------------------------------------------------------------------------------------------------------
	IoReg_Write32(0xb800087C, 0x00000000);	// LVDS A-Port C/D/E/F-Pair Pinshare,
	IoReg_Write32(0xb8000880, 0x00000000);	// LVDS A-Port A/B-Pair Pinshare, LVDS B-Port E/F-Pair Pinshare,
	IoReg_Write32(0xb8000884, 0x00000000);	// LVDS B-Port A/B/C/D-Pair Pinshare,
	IoReg_Write32(0xb8000888, 0x00000000);	// LVDS C-Port C/D/E/F-Pair Pinshare,
	IoReg_Write32(0xb800088C, 0x00000000);	// LVDS C-Port A/B-Pair Pinshare, LVDS D-Port E/F-Pair Pinshare,
	IoReg_Write32(0xb8000890, 0x00000000);	// LVDS D-Port A/B/C/D-Pair Pinshare,


	//---------------------------------------------------------------------------------------------------------------------------------
	//EPI MAC
	//---------------------------------------------------------------------------------------------------------------------------------
	rtd_maskl(0xb8000204,0xFDFFFFFF,0x00000000); // [25]=0, EPI clk is normal path. (not debug mode) (CRT)
	rtd_maskl(0xb802D904,0x8FFFFFFF,0x10000000); // [30:28]=001=DIV1, Epi  4-lane mode (SFG spec)
	// EPI MAC-layer configuration
    IoReg_Write32(0xb802D500,0x00000015);  // CTR_Start=0x15,CTR_Start_dummy=0x0,[30]=0=PN_no_swap

  	// L0~L7, Embedded bit setting
	IoReg_Write32(0xb802D504,0x00000000);  // L0: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D508,0x00000000);  // L1: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D50C,0x00000000);  // L2: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D510,0x00000000);  // L3: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D514,0x00000000);  // L4: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D518,0x00000000);  // L5: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D51C,0x00000000);  // L6: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D520,0x00000000);  // L7: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D524,0x0000db09);  // L0: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D528,0x0000db09);  // L1: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D52C,0x0000db09);  // L2: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D530,0x0000db09);  // L3: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D534,0x0000db09);  // L4: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D538,0x0000db09);  // L5: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D53C,0x0000db09);  // L6: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D540,0x0000db09);  // L7: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D544,0x00001010);  // L0: CTR2: LTD2=1, GMAEN=0, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D548,0x00001010);  // L1: CTR2: LTD2=1, GMAEN=0, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D54C,0x00001810);  // L2: CTR2: LTD2=1, GMAEN=1, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D550,0x00001810);  // L3: CTR2: LTD2=1, GMAEN=1, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D554,0x00001810);  // L4: CTR2: LTD2=1, GMAEN=1, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D558,0x00001810);  // L5: CTR2: LTD2=1, GMAEN=1, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D55C,0x00001010);  // L6: CTR2: LTD2=1, GMAEN=0, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D560,0x00001010);  // L7: CTR2: LTD2=1, GMAEN=0, others:0 (LG: C14=1)

	// L8~L15, Embedded bit setting
	IoReg_Write32(0xb802D604,0x00000000);  // L0: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D608,0x00000000);  // L1: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D60C,0x00000000);  // L2: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D610,0x00000000);  // L3: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D614,0x00000000);  // L4: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D618,0x00000000);  // L5: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D61C,0x00000000);  // L6: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D620,0x00000000);  // L7: [31:16]=Phase_II_preamble_num=0, [15:0]=Phase_I_preamble_num=0
	IoReg_Write32(0xb802D624,0x0000db09);  // L0: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D628,0x0000db09);  // L1: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D62C,0x0000db09);  // L2: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D630,0x0000db09);  // L3: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D634,0x0000db09);  // L4: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D638,0x0000db09);  // L5: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D63C,0x0000db09);  // L6: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D640,0x0000db09);  // L7: [29:18]=New_CTRL_bit,[17:8]=SOE_rising=0xDB=(T2=222),[7:0]=SOE_falling=0x09=(T1=13)
	IoReg_Write32(0xb802D644,0x00001010);  // L0: CTR2: LTD2=1, GMAEN=0, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D648,0x00001010);  // L1: CTR2: LTD2=1, GMAEN=0, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D64C,0x00001810);  // L2: CTR2: LTD2=1, GMAEN=1, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D650,0x00001810);  // L3: CTR2: LTD2=1, GMAEN=1, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D654,0x00001810);  // L4: CTR2: LTD2=1, GMAEN=1, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D658,0x00001810);  // L5: CTR2: LTD2=1, GMAEN=1, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D65C,0x00001010);  // L6: CTR2: LTD2=1, GMAEN=0, others:0 (LG: C14=1)
	IoReg_Write32(0xb802D660,0x00001010);  // L7: CTR2: LTD2=1, GMAEN=0, others:0 (LG: C14=1)

  	// Tcon merge setting (for 16lane)
	IoReg_Write32(0xb802D564,0xFFFFFFE1);  // CTR2:Select CTR2(c0 ~c5 ) source. From Tcon or Register(all "1"). (c0,POL from tcon1)//POL
	IoReg_Write32(0xb802D568,0xFFF07FFF);  // CTR2:Select CTR2(c6 ~c11) source. From Tcon or Register(all "1"). (c9,GSP from tcon0)
	IoReg_Write32(0xb802D56C,0xFFFFFFFF);  // CTR2:Select CTR2(c12~c17) source. From Tcon or Register(all "1").
	IoReg_Write32(0xb802D570,0xFFFFFFFF);  // CTR2:Select CTR2(c18~c23) source. From Tcon or Register(all "1").
	IoReg_Write32(0xb802D574,0xFFFFFFFF);  // CTR2:Select CTR2(c24~c29) source. From Tcon or Register(all "1").

 	// EPI mode setting (for 16lane)
	IoReg_Write32(0xb802D578,0x00000000);  // CTR3: reserved for future. bit30=0 to disable.
	IoReg_Write32(0xb802D57C,0x0000002A);  // Data_Start=0x2A,Data_Start_dummy=0x0.
	IoReg_Write32(0xb802D580,0x08000000);  // Bit31=0=No_data_in.(Bit30=1 to reset), Bit29=0=PRBS7_OFF, 8bit
	IoReg_Write32(0xb802D584,0xC0000000);  // [1]=0=Lock_hw_mode
	IoReg_Write32(0xb802D588,0x00093100);  // [21:17]=Port_num=00100=4_port, [16]=1=normal(no_rst),[11:10]=map_table=0

  	// Setting for application
     rtd_maskl(0xb802d584,0xFFFFFFF9,0x00000006); // [1]=1=Lock_fw_mode, [2]=Lock_fw_value

	//---------------------------------------------------------------------------------------------------------------------------------
	//EPI PHY
	//---------------------------------------------------------------------------------------------------------------------------------
      	// EPI phy setting, 4Lane,
	// Data_rate = (594M/4)*14*2/(4lane) = 1.039Gbps
	// Pixel_clk = (594M/4)/(4lane) = 37.125Mbps
    IoReg_Write32(0xb802D000, 0x0000000C);	// PIF EPI Mode, data is Video, clock is Video_clk,

  	//EPI_X,Y, Panel power enable
	IoReg_Write32(0xb8000C74, 0x80000000);	// P0,P1, [31]=PLT_POW=1, All lane power = off
	IoReg_Write32(0xb8000D74, 0x80000000);	// P0,P1, [31]=PLT_POW=1, All lane power = off

  	//EPI_X,Y, PhyPLL
   	IoReg_Mask32(0xb8000C08,0xFFFF88FF,0x00002300); // [14:12]=DIVM=2+2=4, [10:8]=DIVN=0x3=/8 (8lane)
	IoReg_Mask32(0xb8000C0C,0xA008F1FF,0x04C20600); // [30]=0=CKPIXEL_NOTINV,[28:26]=1=DIV234=2, [25:20]=DIV_P=0xC(12+2=14) // [18:16]=Icp=0x2=3.75uA, [11:9]=Rs=0x3=12K
	IoReg_Mask32(0xb8000D08,0xFFFF88FF,0x00002300); // [14:12]=DIVM=2+2=4,[10:8]=DIVN=0x3=/8 (8lane)
	IoReg_Mask32(0xb8000D0C,0xA008F1FF,0x04C20600); // [30]=0=CKPIXEL_NOTINV,[28:26]=1=DIV234=2, [25:20]=DIV_P=0xC(12+2=14) // [18:16]=Icp=0x2=3.75uA, [11:9]=Rs=0x3=12K

  	//EPI_X (APHY)
	IoReg_Mask32(0xb8000C10,0x03F9FFC0,0x6C000000); // P0_A = Epi mode, [18:17]=00=300ohm, [5:0]=000000=EPI_LVDS_Mode (for A~F)
	IoReg_Mask32(0xb8000C14,0x03F9FFFF,0x6C000000); // P0_B = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000C18,0x03F9FFFF,0x6C000000); // P0_C = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000C1C,0x03F9FFFF,0x6C000000); // P0_D = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000C20,0x03F9FFFF,0x6C000000); // P0_E = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000C24,0x03F9FFFF,0x6C000000); // P0_F = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000C28,0xDC0FFFFF,0x20000000); // P0, [29]=CK7X_Inv=1, [25:20]=000000=1X_current
	IoReg_Mask32(0xb8000C30,0x03F9FFC0,0x6C000000); // P1_A = Epi mode, [18:17]=00=300ohm, [5:0]=000000=EPI_LVDS_Mode (for A~F)
	IoReg_Mask32(0xb8000C34,0x03F9FFFF,0x6C000000); // P1_B = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000C38,0x03F9FFFF,0x6C000000); // P1_C = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000C3C,0x03F9FFFF,0x6C000000); // P1_D = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000C40,0x03F9FFFF,0x6C000000); // P1_E = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000C44,0x03F9FFFF,0x6C000000); // P1_F = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000C48,0xDC0FFFFF,0x20000000); // P1, [29]=CK7X_Inv=1, [25:20]=000000=1X_current
	IoReg_Mask32(0xb8000C50,0x3FFF3FFF,0x0000C000); // P0,P1,[31]=1=IBX, [30]=0=Epi,[15]=1=1VLDO_On,[14]=1=2VLDO_On
	IoReg_Mask32(0xb8000C50,0xFC01FFFF,0x01D00000); // P0,P1,[25:22]=Swing=7=(2.25+7*0.25=4mA), [21:17]=Vcom=8=(0.7+8*0.025=0.9V)
	IoReg_Mask32(0xb8000C54,0xFFC0C0C0,0x003F003F); // P0,[21:16]=111111=2.4mA (>1G),[13:8]=000000=Emp_Off,[5:0]=111111=Zon
	IoReg_Mask32(0xb8000C58,0xC0FFFFFF,0x3F000000); // P0,[29:24]=111111=EPI_CML_mode
	IoReg_Mask32(0xb8000C5C,0xFFC0C0C0,0x003F003F); // P1,[21:16]=111111=2.4mA (>1G),[13:8]=000000=Emp_Off,[5:0]=111111=Zon
	IoReg_Mask32(0xb8000C60,0xC0FFFFFF,0x3F000000); // P1,[29:24]=111111=EPI_CML_mode

	//EPI_Y (APHY)
	IoReg_Mask32(0xb8000D10,0x03F9FFC0,0x6C000000); // P0_A = Epi mode, [18:17]=00=300ohm, [5:0]=000000=EPI_LVDS_Mode (for A~F)
	IoReg_Mask32(0xb8000D14,0x03F9FFFF,0x6C000000); // P0_B = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000D18,0x03F9FFFF,0x6C000000); // P0_C = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000D1C,0x03F9FFFF,0x6C000000); // P0_D = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000D20,0x03F9FFFF,0x6C000000); // P0_E = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000D24,0x03F9FFFF,0x6C000000); // P0_F = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000D28,0xDC0FFFFF,0x20000000); // P0, [29]=CK7X_Inv=1, [25:20]=000000=1X_current
	IoReg_Mask32(0xb8000D30,0x03F9FFC0,0x6C000000); // P1_A = Epi mode, [18:17]=00=300ohm, [5:0]=000000=EPI_LVDS_Mode (for A~F)
	IoReg_Mask32(0xb8000D34,0x03F9FFFF,0x6C000000); // P1_B = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000D38,0x03F9FFFF,0x6C000000); // P1_C = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000D3C,0x03F9FFFF,0x6C000000); // P1_D = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000D40,0x03F9FFFF,0x6C000000); // P1_E = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000D44,0x03F9FFFF,0x6C000000); // P1_F = Epi mode, [18:17]=00=300ohm
	IoReg_Mask32(0xb8000D48,0xDC0FFFFF,0x20000000); // P1, [29]=CK7X_Inv=1, [25:20]=000000=1X_current
	IoReg_Mask32(0xb8000D50,0x3FFF3FFF,0x0000C000); // P0,P1,[31]=1=IBX, [30]=0=Epi,[15]=1=1VLDO_On,[14]=1=2VLDO_On
	IoReg_Mask32(0xb8000D50,0xFC01FFFF,0x01D00000); // P0,P1,[25:22]=Swing=7=(2.25+7*0.25=4mA), [21:17]=Vcom=8=(0.7+8*0.025=0.9V)
	IoReg_Mask32(0xb8000D54,0xFFC0C0C0,0x003F003F); // P0,[21:16]=111111=2.4mA (>1G),[13:8]=000000=Emp_Off,[5:0]=111111=Zon
	IoReg_Mask32(0xb8000D58,0xC0FFFFFF,0x3F000000); // P0,[29:24]=111111=EPI_CML_mode
	IoReg_Mask32(0xb8000D5C,0xFFC0C0C0,0x003F003F); // P1,[21:16]=111111=2.4mA (>1G),[13:8]=000000=Emp_Off,[5:0]=111111=Zon
	IoReg_Mask32(0xb8000D60,0xC0FFFFFF,0x3F000000); // P1,[29:24]=111111=EPI_CML_mode

	//EPI_X,Y, Lane Power Enable
	IoReg_Mask32(0xb8000C74,0xC0FFFFFF,0x3F000000); // [29:24] =epi_lane_pow=111111=X_P0_on
	IoReg_Mask32(0xb8000C74,0xFFC0FFFF,0x003F0000); // [21:16] =epi_lane_pow=111111=X_P1_on
	IoReg_Mask32(0xb8000D74,0xC0FFFFFF,0x3F000000); // [29:24] =epi_lane_pow=111111=Y_P2_on
	IoReg_Mask32(0xb8000D74,0xFFC0FFFF,0x003F0000); // [21:16] =epi_lane_pow=111111=Y_P3_on

	//---------------------------------------------------------------------------------------------------------------------------------
	//TCON SETTING
	//---------------------------------------------------------------------------------------------------------------------------------

    	// Pinshare
   	IoReg_Mask32(0xb8000890,0xFFF0FF00,0x00040044); //pin share MCLK to become Tcon7 //pin share GCLK to become Tcon6 //pin share GST to become Tcon3
    IoReg_Mask32(0xb800088c,0xFFFFFFF0,0x00000004); //pin share E/O to become Tcon14
	IoReg_Write32(0xb802d42c,0x00000000);	// Tcon grobal disable
	IoReg_Write32(0xb802d42c,0x00000001);	// Tcon grobal enable
	IoReg_Write32(0xb802d200,0x40000000);	// Tcon across line3, across frame enable

	// GST
	IoReg_Write32(0xb802d23c,0x00260052);	// Tcon2 Hstart Hend
	IoReg_Write32(0xb802d240,0x00260028);	// Tcon2 Vstart Vend
	IoReg_Write32(0xb802d244,0x00000180);	// Tcon2 across line1 mode
	IoReg_Write32(0xb802d258,0x040C040C);	// Tcon3 Hstart Hend
	IoReg_Write32(0xb802d25c,0x08A608AE);	// Tcon3 Vstart and Vend
	IoReg_Write32(0xb802d260,0x00000183);	// Tcon3 across line1 mode, Xor with Tcon[n-1]
	///////////////////////////////////////////////////////////////////////////////////////
	//GCLK for magellen
	//rtd_part_outl(0xb8000880,3,0,0x4);//pin share GCLK to become Tcon5
	////////////////////////////////////////////////////////////////////////////////////////
	// GCLK for mag2
	IoReg_Write32(0xb802d274,0x01C00108);	// Tcon4 Hstart Hend
	IoReg_Write32(0xb802d278,0x0024089E);	// Tcon4 Vstart Vend
	IoReg_Write32(0xb802d27c,0x00000280); 	// Tcon4 across line2 mode

	IoReg_Write32(0xb802d290,0x02B80070);	// Tcon5 Hstart Hend
	IoReg_Write32(0xb802d294,0x00220028);	// Tcon5 Vstart Vend
	IoReg_Write32(0xb802d298,0x000001C1);	// Tcon5 across line1 mode, And with Tcon[n-1]

	IoReg_Write32(0xb802d2ac,0x040C040C);	// Tcon6 Hstart Hend
	IoReg_Write32(0xb802d2b0,0x089E08AE);	// Tcon6 Vstart Vend
	IoReg_Write32(0xb802d2b4,0x00000182);	// Tcon6 across line1 mode, Or with Tcon[n-1]
	///////////////////////////////////////////////////////////////////////////////////////////
	// MCLK
	IoReg_Write32(0xb802d2c8,0x009001A8);	// Tcon7 HStart and Hend
	IoReg_Write32(0xb802d2cc,0x002A08A4);	// Tcon7 VStart and Vend
	IoReg_Write32(0xb802d2d0,0x00000080);	// Tcon7 Normal Mode

	// EO
	IoReg_Write32(0xb802d37c,0x040C040C); 	// Tcon13 Hstart Hend
	IoReg_Write32(0xb802d380,0x08AE08AE);	// Tcon13 Vstart Vend
	IoReg_Write32(0xb802d384,0x00000480);	// Tcon13 across frame mode
	IoReg_Write32(0xb802d38c,0x0000007E);	// Tcon13 across frame period
	IoReg_Write32(0xb802d398,0x004C004C);	// Tcon14 Hstart Hend
	IoReg_Write32(0xb802d39c,0x08AF08AF);	// Tcon14 Vstart Vend
	IoReg_Write32(0xb802d3a0,0x00000483);	// Tcon14 across frame mode
	IoReg_Write32(0xb802d3a8,0x0000007E);	// Tcon14 across frame period
	IoReg_Write32(0xb802d42c,0x00000000);	// Tcon grobal disable
	IoReg_Write32(0xb802d42c,0x00000001);	// Tcon grobal enable

	// POL
	IoReg_Write32(0xb802d220,0x004C004C);	// Tcon1 Hstart Hend
	IoReg_Write32(0xb802d224,0x002C002C);	// Tcon1 Vstart Vend
	IoReg_Write32(0xb802d228,0x00000480);	// Tcon1 frame across mode

	// GSP
	IoReg_Write32(0xb802d204,0x005C03A8);	// Tcon0 Hstart Hend
	IoReg_Write32(0xb802d208,0x00280030);	// Tcon0 Vstart Vend
	IoReg_Write32(0xb802d20c,0x00000180);	// Tcon0 across line1 mode

	IoReg_Write32(0xb802d42c,0x00000001);	// Tcon grobal enable


#ifdef CONFIG_PM
	if(bOnOff == TRUE)
		vbe_disp_tcon_suspend_resume.ENALE_SUSPEND_RESUME= TRUE;
	else
		vbe_disp_tcon_suspend_resume.ENALE_SUSPEND_RESUME= FALSE;
#endif
#endif
}
void HAL_VBE_DISP_TCON_WriteRegister(UINT32 u32Address, UINT32 u32Data){
	rtd_pr_vbe_debug("###############%s\n",__FUNCTION__);
	IoReg_Write32(u32Address, u32Data);
}
void HAL_VBE_DISP_TCON_ReadRegister(UINT32 u32Address, UINT32 *pData){
	rtd_pr_vbe_debug("###############%s\n",__FUNCTION__);
	*pData = IoReg_Read32(u32Address);
}
void HAL_VBE_DISP_TCON_SetLvdsFormat(KADP_VBE_DISP_TCON_LVDS_TYPE_T type){
	rtd_pr_vbe_debug("###############%s, type = %x\n",__FUNCTION__,type);
#if 0 
	switch(type){
		case KADP_VBE_DISP_TCON_LVDS_VESA:
			rtd_maskl(PIF_LVDS_CTRL3_reg, 0xffffcfff, 0x00001000);
			rtd_pr_vbe_debug("KADP_VBE_DISP_TCON_LVDS_VESA!!\n");
			break;

		case KADP_VBE_DISP_TCON_LVDS_JEIDA:
			rtd_maskl(PIF_LVDS_CTRL3_reg, 0xffffcfff, 0x00000000);
			rtd_pr_vbe_debug("KADP_VBE_DISP_TCON_LVDS_JEIDA!!\n");
			break;

		default:
			break;
	}
#endif
}
void HAL_VBE_DISP_TCON_Debug(void){
	return ;
}
/*
Notice:The arg current_val is 32 bits, so the len is at least 4.
*/
unsigned char drvif_factory_set_lvds_current(KADP_VBE_DISP_LVDS_T *lvds_info)
{
	UINT32 index=0;
	UINT8 len=0;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_0_RBUS		phy_ctrl_1_0_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_4_RBUS		phy_ctrl_1_4_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_8_RBUS		phy_ctrl_1_8_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_12_RBUS		phy_ctrl_1_12_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_16_RBUS		phy_ctrl_1_16_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_20_RBUS		phy_ctrl_1_20_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_0_24_RBUS		phy_ctrl_0_24_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_28_RBUS		phy_ctrl_1_28_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_32_RBUS		phy_ctrl_1_32_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_36_RBUS		phy_ctrl_1_36_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_40_RBUS		phy_ctrl_1_40_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_44_RBUS		phy_ctrl_1_44_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_48_RBUS		phy_ctrl_1_48_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_52_RBUS		phy_ctrl_1_52_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_56_RBUS		phy_ctrl_1_56_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_60_RBUS		phy_ctrl_1_60_reg;

	if(lvds_info == NULL)
	{
		rtd_pr_vbe_err("%s recv error arguments!!\n",__FUNCTION__);
		return 0;
	}

	len = lvds_info->len / sizeof(UINT32);
	if((len > MAX_CURRENT_NUM) || (len == 0x00))
	{
		rtd_pr_vbe_err("%s len is too long or small!!\n",__FUNCTION__);
		return 0;
	}

	if(len > 0)
	{
		phy_ctrl_1_0_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_0_reg);
		phy_ctrl_1_0_reg.pif_tx0_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_0_reg, phy_ctrl_1_0_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_4_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_4_reg);
		phy_ctrl_1_4_reg.pif_tx1_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_4_reg, phy_ctrl_1_4_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_8_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_8_reg);
		phy_ctrl_1_8_reg.pif_tx2_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_8_reg, phy_ctrl_1_8_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_12_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_12_reg);
		phy_ctrl_1_12_reg.pif_tx3_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_12_reg, phy_ctrl_1_12_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_16_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_16_reg);
		phy_ctrl_1_16_reg.pif_tx4_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_16_reg, phy_ctrl_1_16_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_20_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_20_reg);
		phy_ctrl_1_20_reg.pif_tx5_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_20_reg, phy_ctrl_1_20_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_0_24_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_24_reg);
		phy_ctrl_0_24_reg.pif_tx6_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_24_reg, phy_ctrl_0_24_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_28_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_28_reg);
		phy_ctrl_1_28_reg.pif_tx7_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_28_reg, phy_ctrl_1_28_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_32_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_32_reg);
		phy_ctrl_1_32_reg.pif_tx8_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_32_reg, phy_ctrl_1_32_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_36_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_36_reg);
		phy_ctrl_1_36_reg.pif_tx9_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_36_reg, phy_ctrl_1_36_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_40_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_40_reg);
		phy_ctrl_1_40_reg.pif_tx10_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_40_reg, phy_ctrl_1_40_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_44_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_44_reg);
		phy_ctrl_1_44_reg.pif_tx11_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_44_reg, phy_ctrl_1_44_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_48_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_48_reg);
		phy_ctrl_1_48_reg.pif_tx12_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_48_reg, phy_ctrl_1_48_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_52_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_52_reg);
		phy_ctrl_1_52_reg.pif_tx13_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_52_reg, phy_ctrl_1_52_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_56_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_56_reg);
		phy_ctrl_1_56_reg.pif_tx14_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_56_reg, phy_ctrl_1_56_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_60_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_60_reg);
		phy_ctrl_1_60_reg.pif_tx15_idrv_sel = lvds_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_60_reg, phy_ctrl_1_60_reg.regValue);
		len--;
	}

	return 1;
}
unsigned char drvif_factory_get_lvds_current(KADP_VBE_DISP_LVDS_T *lvds_info)
{
	UINT32 regValue[MAX_CURRENT_NUM]={0x00};
	UINT32 len=0;
	UINT8 index=0;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_0_RBUS		phy_ctrl_1_0_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_4_RBUS		phy_ctrl_1_4_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_8_RBUS		phy_ctrl_1_8_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_12_RBUS		phy_ctrl_1_12_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_16_RBUS		phy_ctrl_1_16_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_20_RBUS		phy_ctrl_1_20_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_0_24_RBUS		phy_ctrl_0_24_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_28_RBUS		phy_ctrl_1_28_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_32_RBUS		phy_ctrl_1_32_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_36_RBUS		phy_ctrl_1_36_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_40_RBUS		phy_ctrl_1_40_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_44_RBUS		phy_ctrl_1_44_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_48_RBUS		phy_ctrl_1_48_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_52_RBUS		phy_ctrl_1_52_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_56_RBUS		phy_ctrl_1_56_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_60_RBUS		phy_ctrl_1_60_reg;

	if(lvds_info == NULL)
	{
		rtd_pr_vbe_err("%s recv error arguments!!\n",__FUNCTION__);
		return 0;
	}

	len = lvds_info->len / sizeof(UINT32);
	if((len > MAX_CURRENT_NUM) || (len == 0x00))
	{
		rtd_pr_vbe_err("%s len is too long or small!!\n",__FUNCTION__);
		return 0;
	}
	len = sizeof(UINT32) * len;

	phy_ctrl_1_0_reg.regValue =		IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_0_reg);
	phy_ctrl_1_4_reg.regValue =		IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_4_reg);
	phy_ctrl_1_8_reg.regValue =		IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_8_reg);
	phy_ctrl_1_12_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_12_reg);
	phy_ctrl_1_16_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_16_reg);
	phy_ctrl_1_20_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_20_reg);
	phy_ctrl_0_24_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_24_reg);
	phy_ctrl_1_28_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_28_reg);
	phy_ctrl_1_32_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_32_reg);
	phy_ctrl_1_36_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_36_reg);
	phy_ctrl_1_40_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_40_reg);
	phy_ctrl_1_44_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_44_reg);
	phy_ctrl_1_48_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_48_reg);
	phy_ctrl_1_52_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_52_reg);
	phy_ctrl_1_56_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_56_reg);
	phy_ctrl_1_60_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_60_reg);

	regValue[index++] = phy_ctrl_1_0_reg.pif_tx0_idrv_sel;
	regValue[index++] = phy_ctrl_1_4_reg.pif_tx1_idrv_sel;
	regValue[index++] = phy_ctrl_1_8_reg.pif_tx2_idrv_sel;
	regValue[index++] = phy_ctrl_1_12_reg.pif_tx3_idrv_sel;
	regValue[index++] = phy_ctrl_1_16_reg.pif_tx4_idrv_sel;
	regValue[index++] = phy_ctrl_1_20_reg.pif_tx5_idrv_sel;
	regValue[index++] = phy_ctrl_0_24_reg.pif_tx6_idrv_sel;
	regValue[index++] = phy_ctrl_1_28_reg.pif_tx7_idrv_sel;
	regValue[index++] = phy_ctrl_1_32_reg.pif_tx8_idrv_sel;
	regValue[index++] = phy_ctrl_1_36_reg.pif_tx9_idrv_sel;
	regValue[index++] = phy_ctrl_1_40_reg.pif_tx10_idrv_sel;
	regValue[index++] = phy_ctrl_1_44_reg.pif_tx11_idrv_sel;
	regValue[index++] = phy_ctrl_1_48_reg.pif_tx12_idrv_sel;
	regValue[index++] = phy_ctrl_1_52_reg.pif_tx13_idrv_sel;
	regValue[index++] = phy_ctrl_1_56_reg.pif_tx14_idrv_sel;
	regValue[index++] = phy_ctrl_1_60_reg.pif_tx15_idrv_sel;

	memcpy(lvds_info->current_val,regValue,len);

	return 1;
}

/*
Notice:The arg current_val is 32 bits, so the len is at least 4.
*/
unsigned char drvif_factory_set_pre_emphasis_current(KADP_VBE_DISP_Emphasis_T *emp_info)
{
	UINT32 index=0;
	UINT8 len=0;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_1_RBUS		phy_ctrl_1_1_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_5_RBUS		phy_ctrl_1_5_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_9_RBUS		phy_ctrl_1_9_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_13_RBUS		phy_ctrl_1_13_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_17_RBUS		phy_ctrl_1_17_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_21_RBUS		phy_ctrl_1_21_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_0_25_RBUS		phy_ctrl_0_25_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_29_RBUS		phy_ctrl_1_29_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_33_RBUS		phy_ctrl_1_33_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_37_RBUS		phy_ctrl_1_37_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_41_RBUS		phy_ctrl_1_41_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_45_RBUS		phy_ctrl_1_45_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_49_RBUS		phy_ctrl_1_49_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_53_RBUS		phy_ctrl_1_53_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_57_RBUS		phy_ctrl_1_57_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_61_RBUS		phy_ctrl_1_61_reg;

	if(emp_info == NULL)
	{
		rtd_pr_vbe_err("%s recv error arguments!!\n",__FUNCTION__);
		return 0;
	}

	len = emp_info->len / sizeof(UINT32);
	if((len > MAX_CURRENT_NUM) || (len == 0x00))
	{
		rtd_pr_vbe_err("%s len is too long or small!!\n",__FUNCTION__);
		return 0;
	}

	if(len > 0)
	{
		phy_ctrl_1_1_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_1_reg);
		phy_ctrl_1_1_reg.pif_tx0_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_1_reg, phy_ctrl_1_1_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_5_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_5_reg);
		phy_ctrl_1_5_reg.pif_tx1_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_5_reg, phy_ctrl_1_5_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_9_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_9_reg);
		phy_ctrl_1_9_reg.pif_tx2_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_9_reg, phy_ctrl_1_9_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_13_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_13_reg);
		phy_ctrl_1_13_reg.pif_tx3_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_13_reg, phy_ctrl_1_13_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_17_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_17_reg);
		phy_ctrl_1_17_reg.pif_tx4_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_17_reg, phy_ctrl_1_17_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_21_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_21_reg);
		phy_ctrl_1_21_reg.pif_tx5_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_21_reg, phy_ctrl_1_21_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_0_25_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_25_reg);
		phy_ctrl_0_25_reg.pif_tx6_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_25_reg, phy_ctrl_0_25_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_29_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_29_reg);
		phy_ctrl_1_29_reg.pif_tx7_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_29_reg, phy_ctrl_1_29_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_33_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_33_reg);
		phy_ctrl_1_33_reg.pif_tx8_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_33_reg, phy_ctrl_1_33_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_37_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_37_reg);
		phy_ctrl_1_37_reg.pif_tx9_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_37_reg, phy_ctrl_1_37_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_41_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_41_reg);
		phy_ctrl_1_41_reg.pif_tx10_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_41_reg, phy_ctrl_1_41_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_45_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_45_reg);
		phy_ctrl_1_45_reg.pif_tx11_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_45_reg, phy_ctrl_1_45_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_49_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_49_reg);
		phy_ctrl_1_49_reg.pif_tx12_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_49_reg, phy_ctrl_1_49_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_53_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_53_reg);
		phy_ctrl_1_53_reg.pif_tx13_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_53_reg, phy_ctrl_1_53_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_57_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_57_reg);
		phy_ctrl_1_57_reg.pif_tx14_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_57_reg, phy_ctrl_1_57_reg.regValue);
		len--;
	}

	if(len > 0)
	{
		phy_ctrl_1_61_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_61_reg);
		phy_ctrl_1_61_reg.pif_tx15_iem_sel = emp_info->current_val[index++];
		IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_61_reg, phy_ctrl_1_61_reg.regValue);
		len--;
	}

	return 1;
}
unsigned char drvif_factory_get_pre_emphasis_current(KADP_VBE_DISP_Emphasis_T *emp_info)
{
	UINT32 regValue[MAX_CURRENT_NUM]={0x00};
	UINT32 len=0;
	UINT8 index=0;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_1_RBUS		phy_ctrl_1_1_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_5_RBUS		phy_ctrl_1_5_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_9_RBUS		phy_ctrl_1_9_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_13_RBUS		phy_ctrl_1_13_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_17_RBUS		phy_ctrl_1_17_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_21_RBUS		phy_ctrl_1_21_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_0_25_RBUS		phy_ctrl_0_25_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_29_RBUS		phy_ctrl_1_29_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_33_RBUS		phy_ctrl_1_33_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_37_RBUS		phy_ctrl_1_37_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_41_RBUS		phy_ctrl_1_41_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_45_RBUS		phy_ctrl_1_45_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_49_RBUS		phy_ctrl_1_49_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_53_RBUS		phy_ctrl_1_53_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_57_RBUS		phy_ctrl_1_57_reg;
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_1_61_RBUS		phy_ctrl_1_61_reg;

	if(emp_info == NULL)
	{
		rtd_pr_vbe_err("%s recv error arguments!!\n",__FUNCTION__);
		return 0;
	}

	len = emp_info->len / sizeof(UINT32);
	if((len > MAX_CURRENT_NUM) || (len == 0x00))
	{
		rtd_pr_vbe_err("%s len is too long or small!!\n",__FUNCTION__);
		return 0;
	}
	len = sizeof(UINT32) * len;

	phy_ctrl_1_1_reg.regValue =		IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_1_reg);
	phy_ctrl_1_5_reg.regValue =		IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_5_reg);
	phy_ctrl_1_9_reg.regValue =		IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_9_reg);
	phy_ctrl_1_13_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_13_reg);
	phy_ctrl_1_17_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_17_reg);
	phy_ctrl_1_21_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_21_reg);
	phy_ctrl_0_25_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_25_reg);
	phy_ctrl_1_29_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_29_reg);
	phy_ctrl_1_33_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_33_reg);
	phy_ctrl_1_37_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_37_reg);
	phy_ctrl_1_41_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_41_reg);
	phy_ctrl_1_45_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_45_reg);
	phy_ctrl_1_49_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_49_reg);
	phy_ctrl_1_53_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_53_reg);
	phy_ctrl_1_57_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_57_reg);
	phy_ctrl_1_61_reg.regValue =	IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_1_61_reg);

	regValue[index++] = phy_ctrl_1_1_reg.pif_tx0_iem_sel;
	regValue[index++] = phy_ctrl_1_5_reg.pif_tx1_iem_sel;
	regValue[index++] = phy_ctrl_1_9_reg.pif_tx2_iem_sel;
	regValue[index++] = phy_ctrl_1_13_reg.pif_tx3_iem_sel;
	regValue[index++] = phy_ctrl_1_17_reg.pif_tx4_iem_sel;
	regValue[index++] = phy_ctrl_1_21_reg.pif_tx5_iem_sel;
	regValue[index++] = phy_ctrl_0_25_reg.pif_tx6_iem_sel;
	regValue[index++] = phy_ctrl_1_29_reg.pif_tx7_iem_sel;
	regValue[index++] = phy_ctrl_1_33_reg.pif_tx8_iem_sel;
	regValue[index++] = phy_ctrl_1_37_reg.pif_tx9_iem_sel;
	regValue[index++] = phy_ctrl_1_41_reg.pif_tx10_iem_sel;
	regValue[index++] = phy_ctrl_1_45_reg.pif_tx11_iem_sel;
	regValue[index++] = phy_ctrl_1_49_reg.pif_tx12_iem_sel;
	regValue[index++] = phy_ctrl_1_53_reg.pif_tx13_iem_sel;
	regValue[index++] = phy_ctrl_1_57_reg.pif_tx14_iem_sel;
	regValue[index++] = phy_ctrl_1_61_reg.pif_tx15_iem_sel;

	memcpy(emp_info->current_val,regValue,len);

	return 1;
}


/*******************************************************************************
****************************VBE AVE DRIVER**************************************
*******************************************************************************/
//static unsigned char vbe_ave_input_index = 5;//MAX value
//static unsigned char VBE_AVE_Global_Status = VBE_AVE_NOTHING;
int HAL_VBE_AVE_Initialize(void){
#if 0 //remove tve
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_AVE_Global_Status);
	if(VBE_AVE_Global_Status != VBE_AVE_NOTHING){
		rtd_pr_vbe_debug("\r\n VBE_AVE_Initialize fail. Source is not VBE_AVE_NOTHING.\n");
		return -1;
	}
	VBE_AVE_Global_Status = VBE_AVE_INIT_DONE;
#endif
	return 0;
}

int HAL_VBE_AVE_Uninitialize(void){
#if 0 //remove tve
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_AVE_Global_Status);
	if(VBE_AVE_Global_Status != VBE_AVE_INIT_DONE){
		rtd_pr_vbe_debug("\r\n VBE_AVE_Uninitialize fail. Source is not VBE_AVE_INIT_DONE.\n");
		return -1;
	}
	VBE_AVE_Global_Status = VBE_AVE_NOTHING;
#endif
	return 0;
}

int HAL_VBE_AVE_Open(void){
#if 0 //remove tve
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_AVE_Global_Status);
	if(VBE_AVE_Global_Status != VBE_AVE_INIT_DONE){
		rtd_pr_vbe_debug("\r\n VBE_AVE_Open fail. Source is not VBE_AVE_INIT_DONE.\n");
		return -1;
	}

	//enable TVE clock
	drvif_module_tve_init();
	VBE_AVE_Global_Status = VBE_AVE_OPEN_DONE;
#endif
	return 0;
}

int HAL_VBE_AVE_Close(void){
#if 0 //remove tve
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_AVE_Global_Status);
	if((VBE_AVE_Global_Status != VBE_AVE_OPEN_DONE) && (VBE_AVE_Global_Status != VBE_AVE_CONNECT_DONE)){
		rtd_pr_vbe_debug("\r\n VBE_AVE_Close fail. Source is not VBE_AVE_OPEN_DONE.\n");
		return -1;
	}

	//reset tve
	drvif_module_tve_reset();

	VBE_AVE_Global_Status = VBE_AVE_INIT_DONE;
#endif
	return 0;
}
//extern void scaler_dispDtoI3ddma_prog(TVE_VIDEO_MODE_SEL tve_mode);
//extern void scaler_vodmatosub_prog(TVE_VIDEO_MODE_SEL tve_mode, UINT8 tve_vflip_en);
int HAL_VBE_AVE_Connect(KADP_VBE_AVE_INPUT_INDEX AVEInput){
#if 0 //remove tve
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_AVE_Global_Status);
	rtd_pr_vbe_debug("[scart out]AVEInput = %x \n",AVEInput);
	if(VBE_AVE_Global_Status != VBE_AVE_OPEN_DONE){
		rtd_pr_vbe_debug("\r\n VBE_AVE_Connect fail. Source is not VBE_AVE_OPEN_DONE.\n");
		return -1;
	}

	VBE_AVE_Global_Status = VBE_AVE_CONNECT_DONE;
	vbe_ave_input_index = AVEInput;

	switch(AVEInput){
		case KADP_TUNER_OUTPUT://ATV scart output
			drvif_module_tve_AVout_MODE(VDAC_TV_SRC_SEL_VD);
#ifdef CONFIG_PM
			vbe_ave_suspend_resume.ENALE_SUSPEND_RESUME = VBE_AVE_SUSPEND_RESUME_TUNER_OUTPUT;
#endif
			break;

		case KADP_VSC_SCALER0://DTV scart output
		case KADP_VSC_SCALER1:
			#if 1
			//scaler_vodmatoI3ddma_prog(TVE_PAL_I, Get_PANEL_VFLIP_ENABLE());
			scaler_vodmatosub_prog(TVE_PAL_I, Get_PANEL_VFLIP_ENABLE());
			drvif_module_tve_AVout_MODE(VDAC_TV_SRC_SEL_TV_TVE);
			vo_se_scaling_ctrl(_TRUE);
			#else
			/*test only, disp-D=>i3ddma path*/
			scaler_dispDtoI3ddma_prog(TVE_PAL_I);
			drvif_module_tve_AVout_MODE(VDAC_TV_SRC_SEL_TV_TVE);
			#endif
#ifdef CONFIG_PM
			vbe_ave_suspend_resume.ENALE_SUSPEND_RESUME = VBE_AVE_SUSPEND_RESUME_VSC_SCALER;
#endif

			break;

		case KADP_VDEC0:
		case KADP_VDEC1:
			break;
	}



	//drvif_module_tve_AVout_Enable(TVE_AVOUT_ENABLE);
#endif
	return 0;
}

int HAL_VBE_AVE_Disconnect(void){
#if 0 //remove tve
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_AVE_Global_Status);
	if(VBE_AVE_Global_Status != VBE_AVE_CONNECT_DONE){
		rtd_pr_vbe_debug("\r\n VBE_AVE_Disconnect fail. Source is not VBE_AVE_CONNECT_DONE.\n");
		return -1;
	}

	//Send RPC to disable tveinterrupt checking
	scaler_scart_out_isr_set_enable(_FALSE);
	vo_se_scaling_ctrl(_FALSE);

	//DTV sould de-initial i3ddma path
	if((vbe_ave_input_index == KADP_VSC_SCALER0) || (vbe_ave_input_index == KADP_VSC_SCALER1)){
		//de-initial i3ddma path
		disable_I3DDMA_dispDtoSub();
	}

	//VDAC source change to no using case, scart-out force background
	drvif_module_tve_AVout_MODE(VDAC_TV_SRC_SEL_IFD_DEBUG);
	VBE_AVE_Global_Status = VBE_AVE_OPEN_DONE;
#endif
	return 0;
}

void HAL_VBE_AVE_IsATVWatching(void){
	//to do
	//skip this function
	//rtd_pr_vbe_debug("###############%s\n",__FUNCTION__);
}

void HAL_VBE_AVE_SetATVWatching(KADP_VBE_AVE_INPUT_INDEX AVEInput, BOOLEAN bIsATVWatching){
#if 0 //remove tve
	//rtd_pr_vbe_info("###############%s, status = %d\n",__FUNCTION__,VBE_AVE_Global_Status);
	if(VBE_AVE_Global_Status != VBE_AVE_CONNECT_DONE){
		rtd_pr_vbe_debug("\r\n HAL_VBE_AVE_SetATVWatching fail. Source is not VBE_AVE_CONNECT_DONE.\n");
		return;
	}

	if(AVEInput == KADP_VSC_SCALER1){
		// DTV watching
         		// do nothing
     	}
      	else if ((AVEInput == KADP_TUNER_OUTPUT) && (bIsATVWatching == TRUE)){
		//ATV Watching, Buf Clamp Off
      	}
      	else if ((AVEInput == KADP_TUNER_OUTPUT) && (bIsATVWatching == FALSE)){
		//ATV Not Watching, Buf Clamp On
      	}
#endif
}

void HAL_VBE_AVE_VBI_SetInsert(KADP_VBE_AVE_VBI_TYPE_T type, UINT8 OnOff){
#if 0 //remove tve
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_AVE_Global_Status);
	if(VBE_AVE_Global_Status != VBE_AVE_CONNECT_DONE){
		rtd_pr_vbe_debug("\r\n HAL_VBE_AVE_VBI_SetInsert fail. Source is not VBE_AVE_CONNECT_DONE.\n");
		return;
	}
	switch(type){
		case KADP_VBE_AVE_VBI_PAL_TELETEXT:
			drvif_module_tve_vbi_setting(OnOff);
			break;

		case KADP_VBE_AVE_VBI_PAL_WSS:
			drvif_module_tve_wss_setting(OnOff);
			break;

		case KADP_VBE_AVE_VBI_PAL_VPS:
			drvif_module_tve_vps_setting(OnOff);
			break;

		case KADP_VBE_AVE_VBI_NTSC_CC608:
			drvif_module_tve_cc_setting(OnOff);
			break;

		default:
			rtd_pr_vbe_debug("VBI type is error 1!!");
			break;
	}
#ifdef CONFIG_PM
	if(OnOff == TRUE){
		vbe_ave_suspend_resume.VBI_DATA_ENALE_SUSPEND_RESUME = TRUE;
		vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME = type;
	}
	else{
		vbe_ave_suspend_resume.VBI_DATA_ENALE_SUSPEND_RESUME = FALSE;
		vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME = KADP_VBE_AVE_VBI_MAX_SERVICE;
	}
#endif
#endif
}
#if 0 //remove tve
#define TTX_DATA_MAX_SIZE 16*42 //42 bytes per line, total 16 lines and use 2-buffer
#define TTX_DATA_BUFFER_0	1
#define TTX_DATA_BUFFER_1	0
static UINT8 *ttx_data_pointer;
static unsigned long  ttx_vir_addr, ttx_vir_addr_noncache;
static unsigned int  ttx_phy_addr;
static UINT8 ttx_data_first_run_done = false;
static UINT8 ttx_data_buffer_index = TTX_DATA_BUFFER_0;
#endif
void HAL_VBE_AVE_VBI_InsertData (KADP_VBE_AVE_VBI_TYPE_T type, UINT32 nrLine, UINT32 *pLines, UINT32 size, UINT8 *pData){
#if 0 //remove tve
	Struct_TVE_VPS_DATA *VPS_DATA;
//	unsigned int  /*vps_vir_addr, */vps_vir_addr_noncache;
//	unsigned int  vps_phy_addr;
	unsigned int insert_vbi_data = 0, i = 0;
	UINT8 pline_count = 0;
	unsigned int top_line_position = 0, bottom_line_position = 0;

	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_AVE_Global_Status);
	if(VBE_AVE_Global_Status != VBE_AVE_CONNECT_DONE){
		rtd_pr_vbe_debug("\r\n HAL_VBE_AVE_VBI_InsertData fail. Source is not VBE_AVE_CONNECT_DONE.\n");
		return;
	}
	switch(type){
		case KADP_VBE_AVE_VBI_PAL_TELETEXT:
			//memory allocate
			if(ttx_data_first_run_done == false){
				ttx_vir_addr = (unsigned long)dvr_malloc_uncached(sizeof(unsigned char)*TTX_DATA_MAX_SIZE*2, (void**)&ttx_vir_addr_noncache);//use 2-buffer
				//ttx_phy_addr = (unsigned int)virt_to_phys((void*)ttx_vir_addr);
				ttx_phy_addr = (unsigned int)dvr_to_phys((void*)ttx_vir_addr);
				ttx_data_pointer = (void*)ttx_vir_addr_noncache;
				ttx_data_buffer_index = TTX_DATA_BUFFER_0;
				ttx_data_first_run_done = true;
				//rtd_pr_vbe_info("[crixus]TTX data furst do OK!!!!!\n");
			}
			else{
				//set the data pointer
				if(ttx_data_buffer_index == TTX_DATA_BUFFER_0){
					ttx_data_buffer_index = TTX_DATA_BUFFER_1;//change the buffer index
					ttx_data_pointer = ttx_data_pointer + sizeof(unsigned char)*TTX_DATA_MAX_SIZE;//change the data pointer to buffer 1
					//rtd_pr_vbe_info("[crixus]TTX data chanhe to buffer 1~~~\n");
				}
				else if(ttx_data_buffer_index == TTX_DATA_BUFFER_1){
					ttx_data_buffer_index = TTX_DATA_BUFFER_0;//change the buffer index
					ttx_data_pointer = ttx_data_pointer - sizeof(unsigned char)*TTX_DATA_MAX_SIZE;//change the data pointer to buffer 0
					//rtd_pr_vbe_info("[crixus]TTX data chanhe to buffer 0~~~\n");
				}
			}

			memset(ttx_data_pointer,0x0,sizeof(unsigned char)*size);
			memcpy(ttx_data_pointer,pData,sizeof(unsigned char)*size);

			//for(i=0;i<size;i++)
				//rtd_pr_vbe_info("ttx_data_pointer[%d] = %x\n",i,ttx_data_pointer[i]);

			//check the line numbr
			for(i=0;i<32;i++){
				pline_count++;
				//rtd_pr_vbe_info("pLines[%d] = %d\n",i,pLines[i]);
				if(pLines[i] >= 288){
					top_line_position = pLines[0];
					bottom_line_position = pLines[i];
					pline_count = pline_count -1;
					break;
				}
			}

			//set tt vbi data total line number
			drvif_module_tve_vbi_tt_line_num(pline_count);
			//set top and bottom field lone position
			drvif_module_tve_vbi_pos(top_line_position,bottom_line_position);
			//set memory address
			drvif_module_tve_vbi_memory_setting(ttx_phy_addr);
			//edge trigger to change buffer
			drvif_module_tve_vbidma_trigger(ttx_data_buffer_index);
			break;

		case KADP_VBE_AVE_VBI_PAL_WSS:
			//WSS mode use 2 bytes for each line
			insert_vbi_data = ((unsigned int)pData[1] << 8) | pData[0];
			drvif_module_tve_wss_data(insert_vbi_data);
			drvif_module_tve_wss_pos((pLines[0]-1));//there is one line inaccuracy
			break;

		case KADP_VBE_AVE_VBI_PAL_VPS:
			//memory allocate
		//	vps_vir_addr = (unsigned int)dvr_malloc_uncached(sizeof(Struct_TVE_VPS_DATA), (void**)&vps_vir_addr_noncache);
			//phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
		//	VPS_DATA = (Struct_TVE_VPS_DATA *)vps_vir_addr_noncache;

			//VPS mode use 15 bytes for each line
			VPS_DATA = (Struct_TVE_VPS_DATA *)pData;
			drvif_module_tve_vps_data(VPS_DATA);
			drvif_module_tve_vps_pos(pLines[0]);
		//	dvr_free((void *)vps_vir_addr);
			break;

		case KADP_VBE_AVE_VBI_NTSC_CC608:
			//CC mode use 2 bytes for each line
			insert_vbi_data = ((unsigned int)pData[1] << 8) | pData[0];
			drvif_module_tve_cc_data(insert_vbi_data,insert_vbi_data);
			break;

		default:
			rtd_pr_vbe_debug("VBI type is error 2!!");
			break;
	}
#endif
}

void HAL_VBE_AVE_SetVCRWinBlank(bool bOnOff){
#if 0 //remove tve
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_AVE_Global_Status);
	if(VBE_AVE_Global_Status != VBE_AVE_CONNECT_DONE){
		rtd_pr_vbe_debug("\r\n HAL_VBE_AVE_SetVCRWinBlank fail. Source is not VBE_AVE_CONNECT_DONE.\n");
		return;
	}
	if(vbe_ave_input_index == KADP_TUNER_OUTPUT){//ATV mute=>disable DAC
		if(bOnOff == true)
			drvif_module_tve_AVout_MODE(VDAC_TV_SRC_SEL_IFD_DEBUG);
		else
			drvif_module_tve_AVout_MODE(VDAC_TV_SRC_SEL_TV_TVE);
	}
	else{//DTV mute
		drvif_module_tve_forcebg(bOnOff);
	}
#endif
}
#ifdef CONFIG_PM
//VBE suspend & resume
void vbe_ave_vdac_suspend(void){
	//VDAC
#if 0 //Fix error by Will Magellan2
	vbe_ave_suspend_resume.IFD_ANALOG_VDAC_VDBSO_S_R=IoReg_Read32(IFD_ANALOG_VDAC_VDBSO_VADDR);
	vbe_ave_suspend_resume.IFD_ANALOG_VDAC_S_R=IoReg_Read32(IFD_ANALOG_VDAC_VADDR);
	vbe_ave_suspend_resume.IFD_ANALOG_VADC_IDACO_CTRL1_S_R=IoReg_Read32(IFD_ANALOG_VADC_IDACO_CTRL1_VADDR);
	vbe_ave_suspend_resume.IFD_ANALOG_VDAC_IDACO_CTRL2_S_R=IoReg_Read32(IFD_ANALOG_VDAC_IDACO_CTRL2_VADDR);
	vbe_ave_suspend_resume.IFD_ANALOG_VDAC_SRC_SEL_S_R=IoReg_Read32(IFD_ANALOG_VDAC_SRC_SEL_VADDR);
#endif
}

//VBE suspend & resume
void vbe_ave_tve_dma_suspend(void){
#if 0 //remove tve
	//TVEDMA
	vbe_ave_suspend_resume.TVE_DMA_TVE_VDDMA_S_R=IoReg_Read32(TVE_DMA_TVE_VDDMA_reg);
	vbe_ave_suspend_resume.TVE_DMA_TVEVDDMA_SATRT0_S_R=IoReg_Read32(TVE_DMA_TVEVDDMA_SATRT0_reg);
	vbe_ave_suspend_resume.TVE_DMA_TVEVDDMA_SATRT1_S_R=IoReg_Read32(TVE_DMA_TVEVDDMA_SATRT1_reg);
	vbe_ave_suspend_resume.TVE_DMA_TVEVDDMA_SATRT2_S_R=IoReg_Read32(TVE_DMA_TVEVDDMA_SATRT2_reg);
	vbe_ave_suspend_resume.TVE_DMA_TVEVDDMA_LINESTEP_S_R=IoReg_Read32(TVE_DMA_TVEVDDMA_LINESTEP_reg);
#endif
}

void vbe_ave_vdac_resume(void){
	//VDAC
#if 0 //Fix error by Will Magellan2
	IoReg_Write32(IFD_ANALOG_VDAC_VDBSO_VADDR,vbe_ave_suspend_resume.IFD_ANALOG_VDAC_VDBSO_S_R);
	IoReg_Write32(IFD_ANALOG_VDAC_VADDR,vbe_ave_suspend_resume.IFD_ANALOG_VDAC_S_R);
	IoReg_Write32(IFD_ANALOG_VADC_IDACO_CTRL1_VADDR,vbe_ave_suspend_resume.IFD_ANALOG_VADC_IDACO_CTRL1_S_R);
	IoReg_Write32(IFD_ANALOG_VDAC_IDACO_CTRL2_VADDR,vbe_ave_suspend_resume.IFD_ANALOG_VDAC_IDACO_CTRL2_S_R);
	IoReg_Write32(IFD_ANALOG_VDAC_SRC_SEL_VADDR,vbe_ave_suspend_resume.IFD_ANALOG_VDAC_SRC_SEL_S_R);
#endif
}

void vbe_ave_tve_dma_resume(void){
#if 0 //remove tve
	//TVEDMA
	IoReg_Write32(TVE_DMA_TVE_VDDMA_reg,vbe_ave_suspend_resume.TVE_DMA_TVE_VDDMA_S_R);
	IoReg_Write32(TVE_DMA_TVEVDDMA_SATRT0_reg,vbe_ave_suspend_resume.TVE_DMA_TVEVDDMA_SATRT0_S_R);
	IoReg_Write32(TVE_DMA_TVEVDDMA_SATRT1_reg,vbe_ave_suspend_resume.TVE_DMA_TVEVDDMA_SATRT1_S_R);
	IoReg_Write32(TVE_DMA_TVEVDDMA_SATRT2_reg,vbe_ave_suspend_resume.TVE_DMA_TVEVDDMA_SATRT2_S_R);
	IoReg_Write32(TVE_DMA_TVEVDDMA_LINESTEP_reg,vbe_ave_suspend_resume.TVE_DMA_TVEVDDMA_LINESTEP_S_R);
#endif
}


void vbe_ave_dmavgip_i3ddma_suspend(void){
#if 0 //remove tve
	//DMAVGIP
	vbe_ave_suspend_resume.DMA_VGIP_DMA_VGIP_CTRL_S_R=IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
	vbe_ave_suspend_resume.DMA_VGIP_DMA_VGIP_ACT_HSTA_WIDTH_S_R=IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
	vbe_ave_suspend_resume.DMA_VGIP_DMA_VGIP_ACT_VSTA_LENGTH_S_R=IoReg_Read32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);
	vbe_ave_suspend_resume.DMA_VGIP_DMA_VGIP_DELAY_S_R=IoReg_Read32(DMA_VGIP_DMA_VGIP_DELAY_reg);
	vbe_ave_suspend_resume.DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_S_R=IoReg_Read32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg);

	//I3DDMAcolorspace
	vbe_ave_suspend_resume.I3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_S_R=IoReg_Read32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg);
	vbe_ave_suspend_resume.I3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB1_M1_S_R=IoReg_Read32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_M11_M12_reg);
	vbe_ave_suspend_resume.I3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB1_M2_S_R=IoReg_Read32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_M22_M23_reg);
	vbe_ave_suspend_resume.I3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB1_M3_S_R=IoReg_Read32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_M31_M32_reg);
	vbe_ave_suspend_resume.I3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB1_Y_S_R=IoReg_Read32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_Yo_reg);

	//I3DDMAUZD
	vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_HSD_CTRL0_S_R=IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Ctrl0_reg);
	vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_VSD_CTRL0_S_R=IoReg_Read32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg);
	vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_UZD_FIR_COEF_TAB1_C0_S_R=IoReg_Read32(H3DDMA_HSD_I3DDMA_UZD_FIR_Coef_Tab1_C0_reg);
	vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_UZD_FIR_COEF_TAB2_C0_S_R=IoReg_Read32(H3DDMA_HSD_I3DDMA_UZD_FIR_Coef_Tab2_C0_reg);
	vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_HSD_SCALE_HOR_FACTOR_S_R=IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Scale_Hor_Factor_reg);
	vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_VSD_SCALE_VER_FACTOR_S_R=IoReg_Read32(H3DDMA_HSD_I3DDMA_VSD_Scale_Ver_Factor_reg);
	vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_HSD_HOR_SEGMENT_S_R=IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Hor_Segment_reg);
	//vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_HSD_HOR_DELTA1_S_R=IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Hor_Delta1_reg);		//Merlin5 removed
	vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_VSD_INITIAL_VALUE_S_R=IoReg_Read32(H3DDMA_HSD_I3DDMA_VSD_Initial_Value_reg);
	vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_HSD_INITIAL_VALUE_S_R=IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Initial_Value_reg);

	//I3DDMACap
	//vbe_ave_suspend_resume.I3DDMA_CAP_BOUNDARYADDR2_S_R=IoReg_Read32(H3DDMA_Cap_BoundaryAddr2_reg);	//Merlin5 removed
	//vbe_ave_suspend_resume.I3DDMA_CAP_BOUNDARYADDR1_S_R=IoReg_Read32(H3DDMA_Cap_BoundaryAddr1_reg);	//Merlin5 removed
	vbe_ave_suspend_resume.I3DDMA_LR_SEPARATE_CTRL1_S_R=IoReg_Read32(H3DDMA_LR_Separate_CTRL1_reg);
	vbe_ave_suspend_resume.I3DDMA_LR_SEPARATE_CTRL2_S_R=IoReg_Read32(H3DDMA_LR_Separate_CTRL2_reg);
	//vbe_ave_suspend_resume.I3DDMA_CAP_CTL1_S_R=IoReg_Read32(H3DDMA_Cap_CTL1_reg);	//Merlin5 removed
	//vbe_ave_suspend_resume.I3DDMA_CAP_CTL0_S_R=IoReg_Read32(H3DDMA_Cap_CTL0_reg);	//Merlin5 removed
	vbe_ave_suspend_resume.I3DDMA_TG_V1_END_S_R=IoReg_Read32(H3DDMA_TG_v1_end_reg);
	vbe_ave_suspend_resume.I3DDMA_I3DDMA_ENABLE_S_R=IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
	vbe_ave_suspend_resume.I3DDMA_BYTE_CHANNEL_SWAP_S_R=IoReg_Read32(H3DDMA_I3DDMA_ctrl_0_reg);
	//vbe_ave_suspend_resume.I3DDMA_CAP_L1_START_S_R=IoReg_Read32(H3DDMA_Cap_L1_Start_reg);	//Merlin5 removed
	//vbe_ave_suspend_resume.I3DDMA_CAP_L2_START_S_R=IoReg_Read32(H3DDMA_Cap_L2_Start_reg);	//Merlin5 removed
	//vbe_ave_suspend_resume.I3DDMA_CAP_L3_START_S_R=IoReg_Read32(H3DDMA_Cap_L3_Start_reg);	//Merlin5 removed
#endif
}

void vbe_ave_dmavgip_i3ddma_resume(void){
#if 0 //remove tve
	//DMA VGIP
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTRL_reg,vbe_ave_suspend_resume.DMA_VGIP_DMA_VGIP_CTRL_S_R);
	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg,vbe_ave_suspend_resume.DMA_VGIP_DMA_VGIP_ACT_HSTA_WIDTH_S_R);
	IoReg_Write32(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg,vbe_ave_suspend_resume.DMA_VGIP_DMA_VGIP_ACT_VSTA_LENGTH_S_R);
	IoReg_Write32(DMA_VGIP_DMA_VGIP_DELAY_reg,vbe_ave_suspend_resume.DMA_VGIP_DMA_VGIP_DELAY_S_R);
	IoReg_Write32(DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_reg,vbe_ave_suspend_resume.DMA_VGIP_DMA_VGIP_CTS_FIFO_CTL_S_R);

	//I3DDMA color space
	IoReg_Write32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg,vbe_ave_suspend_resume.I3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_S_R);
	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_M11_M12_reg,vbe_ave_suspend_resume.I3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB1_M1_S_R);
	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_M13_M21_reg,vbe_ave_suspend_resume.I3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB1_M2_S_R);
	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_M31_M32_reg,vbe_ave_suspend_resume.I3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB1_M3_S_R);
	IoReg_Write32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB_Yo_reg,vbe_ave_suspend_resume.I3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_TAB1_Y_S_R);

	//I3DDMA UZD
	IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Ctrl0_reg,vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_HSD_CTRL0_S_R);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg,vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_VSD_CTRL0_S_R);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_UZD_FIR_Coef_Tab1_C0_reg,vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_UZD_FIR_COEF_TAB1_C0_S_R);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_UZD_FIR_Coef_Tab2_C0_reg,vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_UZD_FIR_COEF_TAB2_C0_S_R);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Scale_Hor_Factor_reg,vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_HSD_SCALE_HOR_FACTOR_S_R);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_VSD_Scale_Ver_Factor_reg,vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_VSD_SCALE_VER_FACTOR_S_R);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Hor_Segment_reg,vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_HSD_HOR_SEGMENT_S_R);
	//IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Hor_Delta1_reg,vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_HSD_HOR_DELTA1_S_R);			//Merlin5 removed
	IoReg_Write32(H3DDMA_HSD_I3DDMA_VSD_Initial_Value_reg,vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_VSD_INITIAL_VALUE_S_R);
	IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Initial_Value_reg,vbe_ave_suspend_resume.I3DDMA_SCALEDOWN_HSD_INITIAL_VALUE_S_R);

	//I3DDMA Cap
	//IoReg_Write32(H3DDMA_Cap_BoundaryAddr2_reg,vbe_ave_suspend_resume.I3DDMA_CAP_BOUNDARYADDR2_S_R);	//Merlin5 removed
	//IoReg_Write32(H3DDMA_Cap_BoundaryAddr1_reg,vbe_ave_suspend_resume.I3DDMA_CAP_BOUNDARYADDR1_S_R);	//Merlin5 removed
	IoReg_Write32(H3DDMA_LR_Separate_CTRL1_reg,vbe_ave_suspend_resume.I3DDMA_LR_SEPARATE_CTRL1_S_R);
       	IoReg_Write32(H3DDMA_LR_Separate_CTRL2_reg,vbe_ave_suspend_resume.I3DDMA_LR_SEPARATE_CTRL2_S_R);
	//IoReg_Write32(H3DDMA_Cap_CTL1_reg,vbe_ave_suspend_resume.I3DDMA_CAP_CTL1_S_R);						//Merlin5 removed
	//IoReg_Write32(H3DDMA_Cap_CTL0_reg,vbe_ave_suspend_resume.I3DDMA_CAP_CTL0_S_R);						//Merlin5 removed
 	IoReg_Write32(H3DDMA_TG_v1_end_reg,vbe_ave_suspend_resume.I3DDMA_TG_V1_END_S_R);
 	IoReg_Write32(H3DDMA_I3DDMA_enable_reg,vbe_ave_suspend_resume.I3DDMA_I3DDMA_ENABLE_S_R);
	IoReg_Write32(H3DDMA_I3DDMA_ctrl_0_reg,vbe_ave_suspend_resume.I3DDMA_BYTE_CHANNEL_SWAP_S_R);
	//IoReg_Write32(H3DDMA_Cap_L1_Start_reg,vbe_ave_suspend_resume.I3DDMA_CAP_L1_START_S_R);				//Merlin5 removed
	//IoReg_Write32(H3DDMA_Cap_L2_Start_reg,vbe_ave_suspend_resume.I3DDMA_CAP_L2_START_S_R);				//Merlin5 removed
	//IoReg_Write32(H3DDMA_Cap_L3_Start_reg,vbe_ave_suspend_resume.I3DDMA_CAP_L3_START_S_R);				//Merlin5 removed
#endif
}

void vbe_ave_vbidata_tt_suspend(void){
#if 0 //remove tve
	//VBI DATA - TT
	vbe_ave_suspend_resume.TT_CTRL_S_R=IoReg_Read32(TVE_tt_ctrl_reg);
	vbe_ave_suspend_resume.TT_VPOS_S_R=IoReg_Read32(TVE_tt_vpos_reg);
	vbe_ave_suspend_resume.TVE_DMA_TVEVBIDMA_CTRL_S_R=IoReg_Read32(TVE_DMA_TVEVBIDMA_CTRL_reg);
	vbe_ave_suspend_resume.TVE_DMA_TVEVBIDMA_START_S_R=IoReg_Read32(TVE_DMA_TVEVBIDMA_START_reg);
	vbe_ave_suspend_resume.TVE_DMA_TVEVBIDMA_READ_WINDOW_S_R=IoReg_Read32(TVE_DMA_TVEVBIDMA_READ_WINDOW_reg);
#endif
}

void vbe_ave_vbidata_wss_suspend(void){
#if 0 //remove tve
	//VBI DATA - WSS
	vbe_ave_suspend_resume.WSS_CTRL_S_R=IoReg_Read32(TVE_wss_ctrl_reg);
	vbe_ave_suspend_resume.WSS_POS_S_R=IoReg_Read32(TVE_wss_pos_reg);
#endif
}

void vbe_ave_vbidata_vps_suspend(void){
#if 0 //remove tve
	//VBI DATA - VPS
	vbe_ave_suspend_resume.VPS_CTRL_S_R=IoReg_Read32(TVE_vps_ctrl_reg);
	vbe_ave_suspend_resume.VPS_DATA_0_S_R=IoReg_Read32(TVE_vps_data_0_reg);
	vbe_ave_suspend_resume.VPS_DATA_1_S_R=IoReg_Read32(TVE_vps_data_1_reg);
	vbe_ave_suspend_resume.VPS_DATA_2_S_R=IoReg_Read32(TVE_vps_data_2_reg);
	vbe_ave_suspend_resume.VPS_DATA_3_S_R=IoReg_Read32(TVE_vps_data1_reg);
#endif
}

void vbe_ave_vbidata_cc_suspend(void){
#if 0 //remove tve
	//VBI DATA - CC
	vbe_ave_suspend_resume.IP_CCD_ENBL_S_R=IoReg_Read32(TVE_ip_ccd_enbl_reg);
	vbe_ave_suspend_resume.IP_CCD_ODD_0_S_R=IoReg_Read32(TVE_ip_ccd_odd_0_reg);
	vbe_ave_suspend_resume.IP_CCD_ODD_1_S_R=IoReg_Read32(TVE_ip_ccd_odd_1_reg);
	vbe_ave_suspend_resume.IP_CCD_EVEN_0_S_R=IoReg_Read32(TVE_ip_ccd_even_0_reg);
	vbe_ave_suspend_resume.IP_CCD_EVEN_1_S_R=IoReg_Read32(TVE_ip_ccd_even_1_reg);
#endif
}

void vbe_ave_vbidata_tt_resume(void){
#if 0 //remove tve
	//VBI DATA - TT
	IoReg_Write32(TVE_tt_ctrl_reg,vbe_ave_suspend_resume.TT_CTRL_S_R);
	IoReg_Write32(TVE_tt_vpos_reg,vbe_ave_suspend_resume.TT_VPOS_S_R);
	IoReg_Write32(TVE_DMA_TVEVBIDMA_CTRL_reg,vbe_ave_suspend_resume.TVE_DMA_TVEVBIDMA_CTRL_S_R);
	IoReg_Write32(TVE_DMA_TVEVBIDMA_START_reg,vbe_ave_suspend_resume.TVE_DMA_TVEVBIDMA_START_S_R);
	IoReg_Write32(TVE_DMA_TVEVBIDMA_READ_WINDOW_reg,vbe_ave_suspend_resume.TVE_DMA_TVEVBIDMA_READ_WINDOW_S_R);
#endif
}

void vbe_ave_vbidata_wss_resume(void){
#if 0 //remove tve
	//VBI DATA - WSS
	IoReg_Write32(TVE_wss_ctrl_reg,vbe_ave_suspend_resume.WSS_CTRL_S_R);
	IoReg_Write32(TVE_wss_pos_reg,vbe_ave_suspend_resume.WSS_POS_S_R);
#endif

}

void vbe_ave_vbidata_vps_resume(void){
#if 0 //remove tve
	//VBI DATA - VPS
	IoReg_Write32(TVE_vps_ctrl_reg,vbe_ave_suspend_resume.VPS_CTRL_S_R);
	IoReg_Write32(TVE_vps_data_0_reg,vbe_ave_suspend_resume.VPS_DATA_0_S_R);
	IoReg_Write32(TVE_vps_data_1_reg,vbe_ave_suspend_resume.VPS_DATA_1_S_R);
	IoReg_Write32(TVE_vps_data_2_reg,vbe_ave_suspend_resume.VPS_DATA_2_S_R);
	IoReg_Write32(TVE_vps_data1_reg,vbe_ave_suspend_resume.VPS_DATA_3_S_R);
#endif
}

void vbe_ave_vbidata_cc_resume(void){
#if 0 //remove tve
	//VBI DATA - CC
	IoReg_Write32(TVE_ip_ccd_enbl_reg,vbe_ave_suspend_resume.IP_CCD_ENBL_S_R);
	IoReg_Write32(TVE_ip_ccd_odd_0_reg,vbe_ave_suspend_resume.IP_CCD_ODD_0_S_R);
	IoReg_Write32(TVE_ip_ccd_odd_1_reg,vbe_ave_suspend_resume.IP_CCD_ODD_1_S_R);
	IoReg_Write32(TVE_ip_ccd_even_0_reg,vbe_ave_suspend_resume.IP_CCD_EVEN_0_S_R);
	IoReg_Write32(TVE_ip_ccd_even_1_reg,vbe_ave_suspend_resume.IP_CCD_EVEN_1_S_R);
#endif
}
#endif
/*******************************************************************************
****************************VBE_TRIDFMT DRIVER**********************************
*******************************************************************************/

static unsigned char  VBE_Tridfmt_Global_Status = VBE_TRIDFMT_NOTHING;
static KADP_TRIDTV_INPUT_CONFIG_INFO_T vbe_tridtv_cfg_info;
static KADP_TRIDTV_FMT_CTRL_T vbe_tridtv_fmt;
void HAL_VBE_TRIDFMT_Initialize(void){
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_Tridfmt_Global_Status);
	if(VBE_Tridfmt_Global_Status != VBE_TRIDFMT_NOTHING){
		rtd_pr_vbe_debug("\r\n VBE_TRIDFMT_Initialize fail. Source is not VBE_TRIDFMT_NOTHING.\n");
		return;
	}
	VBE_Tridfmt_Global_Status = VBE_TRIDFMT_INIT_DONE;
	return;
}

void HAL_VBE_TRIDFMT_Uninitialize(void){
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_Tridfmt_Global_Status);
	if(VBE_Tridfmt_Global_Status != VBE_TRIDFMT_INIT_DONE){
		rtd_pr_vbe_debug("\r\n VBE_TRIDFMT_Uninitialize fail. Source is not VBE_TRIDFMT_INIT_DONE.\n");
		return;
	}
	VBE_Tridfmt_Global_Status = VBE_TRIDFMT_NOTHING;

	return;
}

void HAL_VBE_TRIDFMT_Open(void){
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_Tridfmt_Global_Status);
	if(VBE_Tridfmt_Global_Status != VBE_TRIDFMT_INIT_DONE){
		rtd_pr_vbe_info("\r\n VBE_TRIDFMT_Open fail. Source is not VBE_TRIDFMT_INIT_DONE.\n");
		return;
	}

	vbe_tridtv_fmt.mode = KADP_VBE_TRIDTV_3DMODE_ON;
	vbe_tridtv_fmt.in_fmt = KADP_VBE_INPUT_TYPE_3DOFF;
	vbe_tridtv_fmt.out_fmt = KADP_VBE_OUT_SINGLE_FS;
	vbe_tridtv_fmt.lr = KADP_VBE_INPUT_LR_SEQ ;
	vbe_tridtv_fmt.depthCtrl = 10;
	vbe_tridtv_fmt.convCtrl = 0;
	vbe_tridtv_fmt.bAutoConvMode = 0;
	//VBE_set_tridfmt_connect(&vbe_tridtv_fmt);

	VBE_Tridfmt_Global_Status = VBE_TRIDFMT_OPEN_DONE;

	return;
}

void HAL_VBE_TRIDFMT_Close(void){
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_Tridfmt_Global_Status);
	if((VBE_Tridfmt_Global_Status != VBE_TRIDFMT_OPEN_DONE) && (VBE_Tridfmt_Global_Status != VBE_TRIDFMT_CONNECT_DONE)){
		rtd_pr_vbe_debug("\r\n VBE_TRIDFMT_Close fail. Source is not VBE_TRIDFMT_OPEN_DONE.\n");
		return;
	}

	//drvif_module_tve_reset();

	VBE_Tridfmt_Global_Status = VBE_TRIDFMT_INIT_DONE;

	return;
}

void HAL_VBE_TRIDFMT_SetScreenOnOff(BOOLEAN  bOnOff){
	set_display_forcebg_mask(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_3D, bOnOff);
	mute_control(SLR_MAIN_DISPLAY, bOnOff);
}

extern unsigned char rtk_output_connect(unsigned char display);
char VBE_set_tridfmt_connect(KADP_TRIDTV_FMT_CTRL_T *TriFRC){
	unsigned char curmode;
	unsigned char support3Dinput[INPUT_TYPE_MAX]= {
		SLR_3DMODE_2D, SLR_3DMODE_3D_TB, SLR_3DMODE_3D_SBS, SLR_3DMODE_2D,
		SLR_3DMODE_2D, SLR_3DMODE_2D, SLR_3DMODE_2D, SLR_3DMODE_3D_FP,
		SLR_3DMODE_2D, SLR_3DMODE_2D, SLR_3DMODE_3D_SBS, SLR_3DMODE_2D,
		SLR_3DMODE_2D_CVT_3D};
	if(TriFRC == NULL){
		rtd_pr_vbe_debug("VBE_TRIDFMT_SetFMTMode paramter NULL!!!\n");
		return FALSE;
	}
	if(TriFRC->mode >= KADP_VBE_TRIDTV_3DMODE_MAX || TriFRC->mode  < KADP_VBE_TRIDTV_3DMODE_OFF
		||TriFRC->in_fmt >= KADP_VBE_INPUT_TYPE_MAX || TriFRC->in_fmt < KADP_VBE_INPUT_TYPE_3DOFF
		||TriFRC->out_fmt >= KADP_VBE_OUT_MAX || TriFRC->out_fmt < KADP_VBE_OUT_SINGLE_2D
		||TriFRC->lr >= KADP_VBE_INPUT_LR_MAX || TriFRC->lr  < KADP_VBE_INPUT_LR_SEQ
		||TriFRC->depthCtrl > 20 // || TriFRC->depthCtrl < 0  Coverity CID 16456
		|| TriFRC->convCtrl > 10 || TriFRC->convCtrl < -10){
		rtd_pr_vbe_debug("VBE_TRIDFMT_SetFMTMode paramter error!!!\n");
		return FALSE;
	}

	if(TriFRC->mode == KADP_VBE_TRIDTV_3DMODE_OFF){
		rtd_pr_vbe_debug("VBE_TRIDFMT_SetFMTMode OFF!change to 2D!\n");
		if(get_3D_mode_enable() == TRUE){
			set_3D_mode_enable(FALSE);
			set_3D_overscan_enable(FALSE);
			Scaler_Set3DMode(SLR_MAIN_DISPLAY, SLR_3DMODE_2D, TRUE);
			rtk_rerun_scaler(SLR_MAIN_DISPLAY);
		}
		return TRUE;
	}

	if(TriFRC->out_fmt != KADP_VBE_OUT_SINGLE_FS)
		TriFRC->out_fmt = KADP_VBE_OUT_SINGLE_FS;

	if(TriFRC->out_fmt != KADP_VBE_OUT_SINGLE_FS){
		rtd_pr_vbe_debug("VBE_TRIDFMT_SetFMTMode out format no support!change to 2D!\n");
		if(get_3D_mode_enable() == TRUE){
			set_3D_mode_enable(FALSE);
			set_3D_overscan_enable(FALSE);
			Scaler_Set3DMode(SLR_MAIN_DISPLAY, SLR_3DMODE_2D, TRUE);
  			rtk_output_connect(SLR_MAIN_DISPLAY);
		}
		return TRUE;
	}
	else if(TriFRC->mode == KADP_VBE_TRIDTV_3DMODE_ON){
		curmode = TriFRC->in_fmt;
		curmode = support3Dinput[(curmode < KADP_VBE_INPUT_TYPE_MAX? curmode: KADP_VBE_INPUT_TYPE_3DOFF)];

		if(curmode == SLR_3DMODE_2D){
			if(get_3D_mode_enable() == TRUE){
				set_3D_mode_enable(FALSE);
				set_3D_overscan_enable(FALSE);
				Scaler_Set3DMode(SLR_MAIN_DISPLAY, SLR_3DMODE_2D, TRUE);
  				rtk_output_connect(SLR_MAIN_DISPLAY);
				return TRUE;
			}
			else{
				return TRUE;
			}
		}

		Scaler_Set3DMode(SLR_MAIN_DISPLAY, curmode, TRUE);
		 if (Get_PANEL_SCALER_2D_3D_CVT_HWSHIFT_ENABLE() && TriFRC->in_fmt == KADP_VBE_INPUT_TYPE_2DTO3D){
			// [2D3D] default mode init
			rtd_pr_vbe_debug("2d concvt 3d init\n");
			scaler_2Dcvt3D_init();
		}
		 if( drvif_scaler3d_decide_HDMI_framePacking_interlaced_status() && TriFRC->in_fmt == KADP_VBE_INPUT_TYPE_FRAMEPAC){
		 	rtd_pr_vbe_debug("Interlace Fp 3D, change progress to interlace!\n");
			Scaler_DispSetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_INTERLACE,true);
		 }

		set_3D_mode_enable(TRUE);
		set_3D_overscan_enable(FALSE);
  		rtk_output_connect(SLR_MAIN_DISPLAY);

		Scaler_Set3DLRSwap(TriFRC->lr);

		if(curmode == SLR_3DMODE_2D_CVT_3D)
			Scaler_Set2DCvt3D_Depth(TriFRC->depthCtrl);
		else if((curmode > SLR_3DMODE_2D) && (curmode <SLR_3DMODE_2D_CVT_3D))
		{
			/*change 2d cvt 3d to do depth control*/
			scaler_2Dcvt3D_init();
			scaler_2Dcvt3D_set_userEnableStatus(_ENABLE);
			scaler_2Dcvt3D_set_shifter(Scaler_Get2DCvt3D_Strength(), Scaler_Get2DCvt3D_Depth());
			scaler_2Dcvt3D_set_tilt_depth(scaler_2Dcvt3D_get_tilt_enable());
			Scaler_Set2DCvt3D_Depth(TriFRC->depthCtrl);
		}


		drv_2D_to_3D_shifter_offset(TriFRC->convCtrl);
	}
	return TRUE;
}

char HAL_VBE_TRIDFMT_SetFMTMode(VIDEO_WID_T wId,KADP_TRIDTV_FMT_CTRL_T *TriFRC){
#ifdef CONFIG_DUAL_CHANNEL
	if((wId < VIDEO_WID_MIN)||(wId > VIDEO_WID_MAX)){
		rtd_pr_vbe_debug("wId id is not ok\n");
		return FALSE;
	}
#else
	if(wId !=VIDEO_WID_0){
		rtd_pr_vbe_debug("wid id is not ok\n");
		return FALSE;
	}
#endif
	vbe_tridtv_fmt.mode = TriFRC->mode;
	vbe_tridtv_fmt.in_fmt = TriFRC->in_fmt;
	vbe_tridtv_fmt.lr = TriFRC->lr;
	vbe_tridtv_fmt.depthCtrl = TriFRC->depthCtrl;
	vbe_tridtv_fmt.convCtrl = TriFRC->convCtrl;
	vbe_tridtv_fmt.out_fmt = KADP_VBE_OUT_SINGLE_FS;
	vbe_tridtv_fmt.bAutoConvMode = TriFRC->bAutoConvMode;
	VBE_set_tridfmt_connect(&vbe_tridtv_fmt);

	return TRUE;
}

char HAL_VBE_TRIDFMT_SetFMTMode_TEMP(VIDEO_WID_T wId,HAL_TRIDTV_FMT_CTRL_TEMP_T *TriFRC){
#ifdef CONFIG_DUAL_CHANNEL
	if((wId < VIDEO_WID_MIN)||(wId > VIDEO_WID_MAX)){
		rtd_pr_vbe_debug("wId id is not ok\n");
		return FALSE;
	}
#else
	if(wId !=VIDEO_WID_0){
		rtd_pr_vbe_debug("wid id is not ok\n");
		return FALSE;
	}
#endif

	vbe_tridtv_fmt.mode = TriFRC->mode;
	vbe_tridtv_fmt.in_fmt = TriFRC->in_fmt;
	vbe_tridtv_fmt.lr = TriFRC->lr;
	vbe_tridtv_fmt.depthCtrl = TriFRC->depthCtrl;
	vbe_tridtv_fmt.convCtrl = TriFRC->convCtrl;
	vbe_tridtv_fmt.out_fmt = KADP_VBE_OUT_SINGLE_FS;
	vbe_tridtv_fmt.bAutoConvMode = TriFRC->bAutoConvMode;
	VBE_set_tridfmt_connect(&vbe_tridtv_fmt);

	return TRUE;
}

void HAL_VBE_TRIDFMT_Set3DModeOnOff(KADP_TRIDTV_INPUT_CONFIG_INFO_T *p3DModeInfo){
	vbe_tridtv_fmt.mode = p3DModeInfo->n3DInputMode;
	vbe_tridtv_fmt.in_fmt = p3DModeInfo->n3DInputType;
	vbe_tridtv_fmt.lr = p3DModeInfo->n3DInputLRSeq;
	vbe_tridtv_fmt.depthCtrl = p3DModeInfo->n3DInputDepthCtrl;
	vbe_tridtv_fmt.convCtrl = p3DModeInfo->n3DInputConvertCtrl;
	vbe_tridtv_fmt.out_fmt = KADP_VBE_OUT_SINGLE_FS;
	VBE_set_tridfmt_connect(&vbe_tridtv_fmt);
	return;
}

void HAL_VBE_TRIDFMT_ToggleInputLRFrame(KADP_TRIDTV_INPUT_LR_SEQ_T type){
	if(type >= KADP_INPUT_LR_MAX)
		return;
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() == true)
		Scaler_Set3DLRSwap(type);
}

void HAL_VBE_TRIDFMT_SelectInputFormat(KADP_TRIDTV_INPUT_TYPE_T type){
	vbe_tridtv_fmt.in_fmt = type;
	VBE_set_tridfmt_connect(&vbe_tridtv_fmt);
	return;
}

void HAL_VBE_TRIDFMT_SetInputFrameRate(KADP_TRIDTV_INPUT_FREQ_T type){
	//vbe 3D bypass
	return;
}

void HAL_VBE_TRIDFMT_ToggleOutputLRFrame(KADP_TRIDTV_OUTPUT_LR_SEQ_T type){
	if(type >= KADP_VBE_OUTPUT_LR_MAX)
		return;
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() == true)
		Scaler_Set3DLRSwap(type);

}

void HAL_VBE_TRIDFMT_SelectOutputFormat(KADP_TRIDTV_OUTPUT_TYPE_T type){
	vbe_tridtv_fmt.out_fmt = (KADP_VBE_3D_OUTPUT)type;
	VBE_set_tridfmt_connect(&vbe_tridtv_fmt);
	return;
}

void HAL_VBE_TRIDFMT_SetOutputFrameRate(KADP_TRIDTV_OUTPUT_FREQ_T type){
	//vbe 3D bypass
	return;
}

void HAL_VBE_TRIDFMT_Get3DCfgInfo(KADP_TRIDTV_CFG_ITEMS_T n3DCfgItems, UINT8 *nCurCfgInfo){
	if(nCurCfgInfo == NULL)
		return;
	if(n3DCfgItems >= KADP_VBE_TRIDTV_CFG_ITEMS_MAX)
		return;
	switch(n3DCfgItems){
		case KADP_VBE_TRIDTV_3DMODE_STATUS:
			*nCurCfgInfo = vbe_tridtv_cfg_info.n3DInputMode;
			break;
		case KADP_VBE_TRIDTV_3D_INPUT_TYPE:
			*nCurCfgInfo = vbe_tridtv_cfg_info.n3DInputType;
			break;
		case KADP_VBE_TRIDTV_3D_INPUT_LR_SEQ:
			*nCurCfgInfo = vbe_tridtv_cfg_info.n3DInputLRSeq;
			break;
		default:
			break;
	}
}

void HAL_VBE_TRIDFMT_Set3DCfgInfo(KADP_TRIDTV_INPUT_CONFIG_INFO_T *p3DModeInfo){
	if(p3DModeInfo == NULL)
		return;

	vbe_tridtv_cfg_info.n3DInputMode = p3DModeInfo->n3DInputMode;
	vbe_tridtv_cfg_info.n3DInputType = p3DModeInfo->n3DInputType;
	vbe_tridtv_cfg_info.n3DInputLRSeq = p3DModeInfo->n3DInputLRSeq;
	vbe_tridtv_cfg_info.n3DInputDepthCtrl = p3DModeInfo->n3DInputDepthCtrl;
	vbe_tridtv_cfg_info.n3DInputConvertCtrl = p3DModeInfo->n3DInputConvertCtrl;
	vbe_tridtv_cfg_info.bPositiveDepth = p3DModeInfo->bPositiveDepth;
	vbe_tridtv_cfg_info.bAutoConvMode = p3DModeInfo->bAutoConvMode;

}

void HAL_VBE_TRIDFMT_SetConvValue(char  s8ConvValue){
	if( Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE) == SLR_3D_2D_CVT_3D) {
		drv_2D_to_3D_shifter_offset(s8ConvValue);
	}
	else {
		scaler_2Dcvt3D_init();
		scaler_2Dcvt3D_set_userEnableStatus(_ENABLE);
		scaler_2Dcvt3D_set_shifter(Scaler_Get2DCvt3D_Strength(), Scaler_Get2DCvt3D_Depth());
		scaler_2Dcvt3D_set_tilt_depth(scaler_2Dcvt3D_get_tilt_enable());
		/*Scaler_Set2DCvt3D_Depth(s8ConvValue);*/
		drv_2D_to_3D_shifter_offset(s8ConvValue);
	}

}

void HAL_VBE_TRIDFMT_CtrlDepth(KADP_TRIDTV_3DMODE_FLAG_T n3Dmode, UINT8 u8DepthLevel){
	//unsigned char curmode;
	if(KADP_TRIDTV_3DMODE_ON == n3Dmode) {
		/*change 2d cvt 3d to do depth control*/
		scaler_2Dcvt3D_init();
		scaler_2Dcvt3D_set_userEnableStatus(_ENABLE);
		scaler_2Dcvt3D_set_shifter(Scaler_Get2DCvt3D_Strength(), Scaler_Get2DCvt3D_Depth());
		scaler_2Dcvt3D_set_tilt_depth(scaler_2Dcvt3D_get_tilt_enable());
		Scaler_Set2DCvt3D_Depth(u8DepthLevel);
	}
	else if (n3Dmode == KADP_TRIDTV_3DMODE_2DTO3D) {
		Scaler_Set2DCvt3D_Depth(u8DepthLevel);
	}
}

void HAL_VBE_TRIDFMT_AutoConvergenceEnable(BOOLEAN  bOnOff){
	vbe_tridtv_cfg_info.bAutoConvMode = bOnOff;
	//mac3 is not support
}

void HAL_VBE_TRIDFMT_Set3DVideoOutputMode(UINT32 bIsSGMode){
	//vbe 3D bypass
	return;
}

void HAL_VBE_TRIDFMT_Set3DDepthController(UINT32 bOnOff){
	vbe_tridtv_cfg_info.bPositiveDepth = bOnOff;
}

void HAL_VBE_TRIDFMT_Debug(void){
	//vbe 3D bypass
	return;
}

extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr; // debug scaler flag tracker //extern unsigned int main_opensourceID;
extern void Scaler_SET_VSCDispinfo_WithVFEHMDI(unsigned char display, vfe_hdmi_timing_info_t vfehdmitiminginfo);
extern int drivf_I3DDMA_3D_config(void);
extern void HDMI_set_detect_flag(unsigned char enable);
void HAL_V3D_Initialize (void)
{
	rtd_pr_vbe_debug("###############%s, status = %d\n",__FUNCTION__,VBE_Tridfmt_Global_Status);
	if(VBE_Tridfmt_Global_Status != VBE_TRIDFMT_NOTHING){
		rtd_pr_vbe_debug("\r\n VBE_TRIDFMT_Initialize fail. Source is not VBE_TRIDFMT_NOTHING.\n");
		return;
	}
	VBE_Tridfmt_Global_Status = VBE_TRIDFMT_INIT_DONE;
	return;
}
char HAL_V3D_Set3DFMT (VIDEO_WID_T wid, V3D_FMT_TYPE_T in_fmt, V3D_FMT_TYPE_T out_fmt)
{
#if 0  //Fixed me later due to rtk_vo.h compiler error
	vfe_hdmi_timing_info_t vfehdmitiminginfo;
	unsigned char curmode;
	unsigned char support3Dinput[INPUT_TYPE_MAX]= {
		SLR_3DMODE_2D, SLR_3DMODE_3D_TB, SLR_3DMODE_3D_SBS, SLR_3DMODE_2D,
		SLR_3DMODE_2D, SLR_3DMODE_2D, SLR_3DMODE_2D, SLR_3DMODE_3D_FP,
		SLR_3DMODE_2D, SLR_3DMODE_2D, SLR_3DMODE_3D_SBS, SLR_3DMODE_3D_FP,
		SLR_3DMODE_2D_CVT_3D};
	rtd_pr_vbe_debug("v3d:%d=%d\n",in_fmt, out_fmt);

	curmode = in_fmt;
	curmode = support3Dinput[(curmode < INPUT_TYPE_MAX? curmode: INPUT_TYPE_3DOFF)];

	if(in_fmt > V3D_FMT_MAX || in_fmt < V3D_FMT_2D
		||out_fmt > V3D_FMT_MAX ||out_fmt < V3D_FMT_2D)
	{
		rtd_pr_vbe_debug("scaler vsc Set3DForScaler paramter error!!!\n");
		return FALSE;
	}

	set_field_alternative_3d_mode_enable(FALSE);
#if 1
	if(get_line_alternative_3d_mode_enable() == TRUE)
	{
		Scaler_DispSetInputInfo(SLR_INPUT_THROUGH_I3DDMA, false);
		set_line_alternative_3d_mode_enable(FALSE);
		DbgSclrFlgTkr.main_opensourceID =_UNKNOWN_INPUT;
	}
#endif
	 if(in_fmt == V3D_FMT_MAX  ||out_fmt == V3D_FMT_MAX)
	{

		if(get_3D_mode_enable() == TRUE)
		{
			rtd_pr_vbe_debug("scaler vsc Set3DForScaler MAXNUM!change to 2D!\n");
			set_3D_mode_enable(FALSE);
			set_3D_overscan_enable(FALSE);
			Scaler_Set3DMode(SLR_MAIN_DISPLAY, SLR_3DMODE_2D, TRUE);
			down(&VBE_Semaphore);
			if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
				rtk_output_connect(SLR_MAIN_DISPLAY);
			up(&VBE_Semaphore);
		}
		return TRUE;
	}


	/*force set the out_fmt SC_OUT_SINGLE_FS*/
	if(out_fmt != V3D_FMT_FRAMESEQ)
		out_fmt = V3D_FMT_FRAMESEQ;
#if 0
	if(out_fmt != V3D_FMT_FRAMESEQ)
	{
		rtd_pr_vbe_debug("scaler vsc Set3DForScaler out format no support!change to 2D!\n");
		if(get_3D_mode_enable() == TRUE)
		{
			set_3D_mode_enable(FALSE);
			set_3D_overscan_enable(FALSE);
			Scaler_Set3DMode(SLR_MAIN_DISPLAY, SLR_3DMODE_2D, TRUE);
			down(&VBE_Semaphore);
			if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
				rtk_output_connect(SLR_MAIN_DISPLAY);
			up(&VBE_Semaphore);
		}
		return TRUE;
	}
#endif
	if(in_fmt == V3D_FMT_DUALSTREAM)
	{
		set_3D_mode_enable(TRUE);
		set_3D_overscan_enable(FALSE);
		Scaler_Set3DMode(SLR_MAIN_DISPLAY, SLR_3DMODE_3D_FP, TRUE);
		down(&VBE_Semaphore);
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
			rtk_output_connect(SLR_MAIN_DISPLAY);
		up(&VBE_Semaphore);

		return TRUE;
	}

	if(in_fmt == V3D_FMT_LINE_ALTERNATIVE/*&& DisplayModeInputInfo.type == VSC_INPUTSRC_HDMI*/)
	{
		rtd_pr_vbe_debug("input is line alternative\n");
		Scaler_Set3DMode(SLR_MAIN_DISPLAY, SLR_3DMODE_3D_LBL, TRUE);

		set_3D_mode_enable(TRUE);
		set_3D_overscan_enable(FALSE);

		 //DisplayModeInputInfo.type = VSC_INPUTSRC_VDEC;

		if(0!=vfe_hdmi_drv_get_display_timing_info(&vfehdmitiminginfo, ((wid==VIDEO_WID_0)?SLR_MAIN_DISPLAY : SLR_SUB_DISPLAY)))
			return FALSE;
		Scaler_SET_VSCDispinfo_WithVFEHMDI(SLR_MAIN_DISPLAY,vfehdmitiminginfo);
		drvif_I3DDMA_Init();
		HDMI_set_detect_flag(FALSE);
		Scaler_DispSetInputInfo(SLR_INPUT_THROUGH_I3DDMA, true);
		set_line_alternative_3d_mode_enable(TRUE);
		drivf_I3DDMA_3D_config();

		Scaler_Set3DMode(SLR_MAIN_DISPLAY, SLR_3DMODE_3D_FP, TRUE);
		Scaler_Disp3dSetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE,SLR_3D_FRAME_PACKING);
		Scaler_Set3DMode_Attr(SLR_MAIN_DISPLAY, SLR_3DMODE_3D_FP);
		set_vo_3d_mode(VO_2D_MODE);

		DbgSclrFlgTkr.main_opensourceID =_UNKNOWN_INPUT;
		down(&VBE_Semaphore);
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
			rtk_output_connect(SLR_MAIN_DISPLAY);
		up(&VBE_Semaphore);

		return TRUE;


	}
	else if(in_fmt == V3D_FMT_FIELD_ALTERNATIVE/*&& DisplayModeInputInfo.type == VSC_INPUTSRC_HDMI*/)
	{
		rtd_pr_vbe_debug("input is feild alternative\n");
		Scaler_DispSetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_INTERLACE,true);
		set_3D_mode_enable(TRUE);
		set_3D_overscan_enable(FALSE);
		set_field_alternative_3d_mode_enable(TRUE);
		Scaler_Set3DMode(SLR_MAIN_DISPLAY, SLR_3DMODE_3D_FP, TRUE);
		down(&VBE_Semaphore);
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
			rtk_output_connect(SLR_MAIN_DISPLAY);
		up(&VBE_Semaphore);

		 return TRUE;

	}
	if(wid == VIDEO_WID_0)
	{

		if(curmode == SLR_3DMODE_2D)
		{
			if(get_3D_mode_enable() == TRUE)
			{
				set_3D_mode_enable(FALSE);
				set_3D_overscan_enable(FALSE);
				Scaler_Set3DMode(SLR_MAIN_DISPLAY, SLR_3DMODE_2D, TRUE);
				down(&VBE_Semaphore);
				if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
					rtk_output_connect(SLR_MAIN_DISPLAY);
				up(&VBE_Semaphore);
				return TRUE;
			}
			else
			{
				return TRUE;
			}
		}

		Scaler_Set3DMode(SLR_MAIN_DISPLAY, curmode, TRUE);
	#if 0
		 if (Get_PANEL_SCALER_2D_3D_CVT_HWSHIFT_ENABLE() && in_fmt == INPUT_TYPE_2DTO3D)
		{
			// [2D3D] default mode init
			rtd_pr_vbe_debug("2d concvt 3d init\n");
			scaler_2Dcvt3D_init();
		}
	#endif
		 if( drvif_scaler3d_decide_HDMI_framePacking_interlaced_status() && in_fmt == V3D_FMT_FRAMEPAC)
		 {
			rtd_pr_vbe_debug("Interlace Fp 3D, change progress to interlace!\n");
			Scaler_DispSetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_INTERLACE,true);
		 }

		set_3D_mode_enable(TRUE);
		set_3D_overscan_enable(FALSE);
		down(&VBE_Semaphore);
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
			rtk_output_connect(SLR_MAIN_DISPLAY);
		up(&VBE_Semaphore);
	}
#endif
	return TRUE;
}


void HAL_V3D_SetLRSeq (V3D_LR_SEQ_T type)
{
	Scaler_Set3DLRSwap(type);
}
void HAL_V3D_CtrlConvergence (INT8 s8ConvValue)
{
	if( Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE) == SLR_3D_2D_CVT_3D) {
		drv_2D_to_3D_shifter_offset(s8ConvValue);
	}
	else {
		scaler_2Dcvt3D_init();
		scaler_2Dcvt3D_set_userEnableStatus(_ENABLE);
		scaler_2Dcvt3D_set_shifter(Scaler_Get2DCvt3D_Strength(), Scaler_Get2DCvt3D_Depth());
		scaler_2Dcvt3D_set_tilt_depth(scaler_2Dcvt3D_get_tilt_enable());
		/*Scaler_Set2DCvt3D_Depth(s8ConvValue);*/
		drv_2D_to_3D_shifter_offset(s8ConvValue);
	}
}
void HAL_V3D_CtrlDepth (UINT8 u8DepthLevel)
{
	if( Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE) == SLR_3D_2D_CVT_3D) {
		drv_2D_to_3D_shifter_offset(u8DepthLevel);
	}  else {
		/*change 2d cvt 3d to do depth control*/
		scaler_2Dcvt3D_init();
		scaler_2Dcvt3D_set_userEnableStatus(_ENABLE);
		scaler_2Dcvt3D_set_shifter(Scaler_Get2DCvt3D_Strength(), Scaler_Get2DCvt3D_Depth());
		scaler_2Dcvt3D_set_tilt_depth(scaler_2Dcvt3D_get_tilt_enable());
		Scaler_Set2DCvt3D_Depth(u8DepthLevel);
	}
}

void HAL_V3D_SetAutoConvEnable(BOOLEAN bOnOff)
{
	if(bOnOff == TRUE) {
		if( Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE) == SLR_3D_2D_CVT_3D) {
			drv_2D_to_3D_shifter_offset(0);
		}  else {
			/*change 2d cvt 3d to do depth control*/
			scaler_2Dcvt3D_init();
			scaler_2Dcvt3D_set_userEnableStatus(_ENABLE);
			scaler_2Dcvt3D_set_shifter(Scaler_Get2DCvt3D_Strength(), Scaler_Get2DCvt3D_Depth());
			scaler_2Dcvt3D_set_tilt_depth(scaler_2Dcvt3D_get_tilt_enable());
			Scaler_Set2DCvt3D_Depth(0);
		}
	}
}

void HAL_V3D_GetAutoDetectingResult (V3D_FMT_TYPE_T * pVideo3Dtype)
{
#if 0  //Fixed me later due to rtk_vo.h compiler error
	unsigned char  curmode;
	if (Scaler_InputSrcGetMainChType() == _SRC_VO)
	{
		if(get_vo_3d_mode() == VO_3D_SIDE_BY_SIDE_HALF)
			*pVideo3Dtype = V3D_FMT_SIDEBYSIDE;
		else if(get_vo_3d_mode() == VO_3D_TOP_AND_BOTTOM)
			*pVideo3Dtype = V3D_FMT_TOPNBOTTOM;
		else
			*pVideo3Dtype = V3D_FMT_2D;
		return;
	}

	Scaler_Set3D_VoAutoDetect_Enable(true);
	check_vo3d_detect_result((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY));
	curmode = modestate_get_vo3d_detect_result();
	switch(curmode)
	{
		case VO_2D_MODE:
			*pVideo3Dtype = V3D_FMT_2D;
			break;
		case VO_3D_SIDE_BY_SIDE_HALF:
			*pVideo3Dtype = V3D_FMT_SIDEBYSIDE;
			break;
		case VO_3D_TOP_AND_BOTTOM:
			*pVideo3Dtype = V3D_FMT_TOPNBOTTOM;
			break;
		default:
			rtd_pr_vbe_debug("Can't detect 3D auto mode!\n");
			break;
	}
#endif
	return;
}

void HAL_V3D_SetAutoDetectingType (V3D_FORMAT_TYPE_T mode)
{
	mask_3d_auto_detect |= mode;
}


void HAL_DISP_PANEL_GetPanelInfo(KADP_DISP_PANEL_INFO_T *panelInfo)
{
    if(panelInfo == NULL)
    {
        rtd_pr_vbe_err("[%s %d] ERROR panelInfo is NULL\n", __FUNCTION__, __LINE__);
        return;
    }

    switch(Get_DISPLAY_REFRESH_RATE())
    {
        case 60:
            panelInfo->panelFrameRate = KADP_DISP_PANEL_FRAMERATE_60Hz;
            break;

        case 120:
            panelInfo->panelFrameRate = KADP_DISP_PANEL_FRAMERATE_120Hz;
            break;

        case 144:
            panelInfo->panelFrameRate = KADP_DISP_PANEL_FRAMERATE_144Hz;
            break;

        case 165:
            panelInfo->panelFrameRate = KADP_DISP_PANEL_FRAMERATE_165Hz;
            break;

        case 240:
            panelInfo->panelFrameRate = KADP_DISP_PANEL_FRAMERATE_240Hz;
            break;

        case 288:
            panelInfo->panelFrameRate = KADP_DISP_PANEL_FRAMERATE_288Hz;
            break;

        default:
            panelInfo->panelFrameRate = KADP_DISP_PANEL_FRAMERATE_60Hz;
            break;
    }
}

/*******************************************************************************
****************************VBE DEVICE******************************************
*******************************************************************************/
#ifdef CONFIG_PM
#ifdef CONFIG_HIBERNATION
static int vbe_suspend (struct device *p_dev)
{
	// STD suspend mode
	vbe_suspend_instant_mode = 0;
#if 0
	//VBE_AVE suspend
	if(VBE_AVE_Global_Status == VBE_AVE_OPEN_DONE){
		vbe_ave_vdac_suspend();
	}
	else if(VBE_AVE_Global_Status == VBE_AVE_CONNECT_DONE){
		if(vbe_ave_suspend_resume.ENALE_SUSPEND_RESUME == VBE_AVE_SUSPEND_RESUME_VSC_SCALER){
			vbe_ave_vdac_suspend();
			vbe_ave_tve_dma_suspend();
			vbe_ave_dmavgip_i3ddma_suspend();
		}

	}

	//VBI DATA suspend
	if(vbe_ave_suspend_resume.VBI_DATA_ENALE_SUSPEND_RESUME == TRUE){
		if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_PAL_TELETEXT)
			vbe_ave_vbidata_tt_suspend();
		else if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_PAL_WSS)
			vbe_ave_vbidata_wss_suspend();
		else if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_PAL_VPS)
			vbe_ave_vbidata_vps_suspend();
		else if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_NTSC_CC608)
			vbe_ave_vbidata_cc_suspend();
	}
#endif
	vbe_disp_suspend();
	vpq_memc_disp_suspend_snashop();
	vpq_do_suspend();

	return 0;
}

static int vbe_resume (struct device *p_dev)
{

	//VBE_DISP_TCON resume
	if(vbe_disp_tcon_suspend_resume.ENALE_SUSPEND_RESUME == TRUE){
		HAL_VBE_DISP_TCON_SetClock(TRUE);
	}
#if 0	//remove tve
	//VBE_AVE resume
	if(VBE_AVE_Global_Status == VBE_AVE_OPEN_DONE){
		drvif_tve_resume_setting();
		vbe_ave_vdac_resume();
	}
	else if(VBE_AVE_Global_Status == VBE_AVE_CONNECT_DONE){
		//both ATV and DTV scart out should do this
		drvif_tve_resume_setting();
		vbe_ave_vdac_resume();

		//DTV-scart out
		if(vbe_ave_suspend_resume.ENALE_SUSPEND_RESUME == VBE_AVE_SUSPEND_RESUME_VSC_SCALER){
			drv_tve_set_video_pal_i();
			vbe_ave_tve_dma_resume();
			vbe_ave_dmavgip_i3ddma_resume();
		}
	}

	//VBI DATA resume
	if(vbe_ave_suspend_resume.VBI_DATA_ENALE_SUSPEND_RESUME == TRUE){
		if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_PAL_TELETEXT)
			vbe_ave_vbidata_tt_resume();
		else if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_PAL_WSS)
			vbe_ave_vbidata_wss_resume();
		else if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_PAL_VPS)
			vbe_ave_vbidata_vps_resume();
		else if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_NTSC_CC608)
			vbe_ave_vbidata_cc_resume();
	}
#endif
	vbe_disp_resume();
	vpq_memc_disp_resume_snashop();
	vpq_do_resume();
	vpqled_do_resume();

	return 0;
}
#endif
static int vbe_suspend_instanboot (struct device *p_dev)
{
	// STR suspend mode
	vbe_suspend_instant_mode = 1;
#if 0
	//VBE_AVE suspend
	if(VBE_AVE_Global_Status == VBE_AVE_OPEN_DONE){
		vbe_ave_vdac_suspend();
	}
	else if(VBE_AVE_Global_Status == VBE_AVE_CONNECT_DONE){
		if(vbe_ave_suspend_resume.ENALE_SUSPEND_RESUME == VBE_AVE_SUSPEND_RESUME_VSC_SCALER){
			vbe_ave_vdac_suspend();
			vbe_ave_tve_dma_suspend();
			vbe_ave_dmavgip_i3ddma_suspend();
		}

	}

	//VBI DATA suspend
	if(vbe_ave_suspend_resume.VBI_DATA_ENALE_SUSPEND_RESUME == TRUE){
		if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_PAL_TELETEXT)
			vbe_ave_vbidata_tt_suspend();
		else if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_PAL_WSS)
			vbe_ave_vbidata_wss_suspend();
		else if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_PAL_VPS)
			vbe_ave_vbidata_vps_suspend();
		else if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_NTSC_CC608)
			vbe_ave_vbidata_cc_suspend();
	}
#endif
	vbe_disp_suspend();
	vpq_memc_disp_suspend_instanboot();
	vpq_do_suspend();
#ifdef VBE_DISP_TCON_SELF_CHECK
#if IS_ENABLED(CONFIG_RTK_KDRV_THERMAL_SENSOR)
	monitor_rgb_rolling=1; //for stop tcon check
#endif
#endif
	return 0;
}

static int vbe_resume_instanboot (struct device *p_dev)
{
	//VBE_DISP_TCON resume
	if(vbe_disp_tcon_suspend_resume.ENALE_SUSPEND_RESUME == TRUE){
		HAL_VBE_DISP_TCON_SetClock(TRUE);
	}
#if 0	//remove tve
	//VBE_AVE resume
	if(VBE_AVE_Global_Status == VBE_AVE_OPEN_DONE){
		drvif_tve_resume_setting();
		vbe_ave_vdac_resume();
	}
	else if(VBE_AVE_Global_Status == VBE_AVE_CONNECT_DONE){
		//both ATV and DTV scart out should do this
		drvif_tve_resume_setting();
		vbe_ave_vdac_resume();

		//DTV-scart out
		if(vbe_ave_suspend_resume.ENALE_SUSPEND_RESUME == VBE_AVE_SUSPEND_RESUME_VSC_SCALER){
			drv_tve_set_video_pal_i();
			vbe_ave_tve_dma_resume();
			vbe_ave_dmavgip_i3ddma_resume();
		}
	}

	//VBI DATA resume
	if(vbe_ave_suspend_resume.VBI_DATA_ENALE_SUSPEND_RESUME == TRUE){
		if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_PAL_TELETEXT)
			vbe_ave_vbidata_tt_resume();
		else if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_PAL_WSS)
			vbe_ave_vbidata_wss_resume();
		else if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_PAL_VPS)
			vbe_ave_vbidata_vps_resume();
		else if(vbe_ave_suspend_resume.VBI_DATA_TYPE_SUSPEND_RESUME == KADP_VBE_AVE_VBI_NTSC_CC608)
			vbe_ave_vbidata_cc_resume();
	}
#endif
	/*KTASKWBS-4676, wait LG webos call, so we enable first*/
	vbe_disp_resume();
	rtd_pr_vbe_debug("[vbe_resume_instanboot] Remove vbe_disp_resume !\n");
	vpq_memc_disp_resume_instanboot();
	vpq_do_resume_instanboot();
#if 0
	//EPI panel need to fix last line 4-align in fs mode @Crixus 20160929
	if(Get_DISPLAY_PANEL_TYPE() == P_EPI_TYPE){
		ppoverlay_fs_fll_mode_control_RBUS ppoverlay_fs_fll_mode_control_reg;
		ppoverlay_fs_fll_mode_control_reg.regValue = rtd_inl(PPOVERLAY_FS_FLL_Mode_Control_reg);
		ppoverlay_fs_fll_mode_control_reg.win_sta = 0x1120;
		ppoverlay_fs_fll_mode_control_reg.win_end = 0x112c;
		ppoverlay_fs_fll_mode_control_reg.fs_fll_range_mode = 1;
		rtd_outl(PPOVERLAY_FS_FLL_Mode_Control_reg, ppoverlay_fs_fll_mode_control_reg.regValue);
	}
#endif


	//power on sequence : LVDS/VBY1 to Backlight on
	//rtd_pr_vbe_emerg("[vbe_disp_resume] Get_LVDS_TO_LIGHT_ON_ms = %d\n", Get_LVDS_TO_LIGHT_ON_ms());
	//msleep(Get_LVDS_TO_LIGHT_ON_ms());

	return 0;
}

#define WKSOR_REG   0xb8060574
#define WKSOR_PWR 3
#define WKSOR_WOL 6
#define WKSOR_RTC 8
void vbe_resume_Backlight_TurnOn(struct device *p_dev)
{
	unsigned int tick1=0, tick2=0;
	unsigned int pastTimeTick=0;

	rtd_pr_vbe_emerg("vbe_resume_Backlight_TurnOn !\n");

	tick1 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
	if(tick1>=stBacklihgtStart_Tick)
		pastTimeTick = tick1 - stBacklihgtStart_Tick;
	else //loop back
		pastTimeTick = (0xffffffff-stBacklihgtStart_Tick) + tick1;

	if(pastTimeTick >= (Get_LVDS_TO_LIGHT_ON_ms()*90)){
		//no need to delay
	}else if(Panel_Get_STR_BackLight_Ctrl_Mode() == _BACKLIGHT_STR_CTRL_MODE_BY_AP){
		//no need to delay
	}else{
		msleep(((Get_LVDS_TO_LIGHT_ON_ms()*90)-pastTimeTick)/90);
	}
	tick2 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);

	if(Panel_Get_STR_BackLight_Ctrl_Mode() == _BACKLIGHT_STR_CTRL_MODE_BY_KERNEL){
        //Panel_SetBackLightMode(1);
        unsigned int regValue = rtd_inl(WKSOR_REG);
        if(((regValue & 0xff) != 0x0) && ((regValue & 0xff) != WKSOR_WOL) && ((regValue & 0xff) != WKSOR_RTC))
        {
            Panel_SetBackLightMode(1);
            rtd_pr_vbe_notice("[%s] STR BACKLIGHT CTRL MODE_BY_KERNEL\n",__FUNCTION__);
        }
        else
            rtd_pr_vbe_notice("[%s] RTC/WOW wakeup,do not control backlight\n",__FUNCTION__);
	}
	else if(Panel_Get_STR_BackLight_Ctrl_Mode() == _BACKLIGHT_STR_CTRL_MODE_BY_AP){
		rtd_pr_vbe_notice("[%s] STR BACKLIGHT CTRL MODE_BY_AP, no need turn on back light\n",__FUNCTION__);
	}
	else{
		Panel_SetBackLightMode(1);
		rtd_pr_vbe_notice("[%s] BACKLIGHT ON\n",__FUNCTION__);
	}
	rtd_pr_vbe_notice("[%s] BL_delay=%d(ms), BL_startTick=%x, tick1=%x, tick2=%x, \n",__FUNCTION__,
		Get_LVDS_TO_LIGHT_ON_ms(), stBacklihgtStart_Tick, tick1, tick2);
}
#endif

#ifdef CONFIG_ORBIT_PIXEL_SHIFT_ALGO

#define ORBIT_TABLE_IDX_MAX 44

void set_justscan_bg(int32_t cb_b, int32_t y_g, int32_t cr_r)
{
	/* orbit justscan background */
	ppoverlay_orbit_bg_color1_RBUS ppoverlay_orbit_bg_color1_reg;
	ppoverlay_orbit_bg_color2_RBUS ppoverlay_orbit_bg_color2_reg;
	ppoverlay_orbit_bg_color1_reg.regValue = IoReg_Read32(PPOVERLAY_ORBIT_BG_COLOR1_reg);
	ppoverlay_orbit_bg_color2_reg.regValue = IoReg_Read32(PPOVERLAY_ORBIT_BG_COLOR2_reg);
	ppoverlay_orbit_bg_color1_reg.orbit_bg_cb_b_value = cb_b & 0x3FFF;
	ppoverlay_orbit_bg_color1_reg.orbit_bg_y_g_value  = y_g  & 0x3FFF;
	ppoverlay_orbit_bg_color2_reg.orbit_bg_cr_r_value = cr_r & 0x3FFF;
	IoReg_Write32(PPOVERLAY_ORBIT_BG_COLOR1_reg, ppoverlay_orbit_bg_color1_reg.regValue);
	IoReg_Write32(PPOVERLAY_ORBIT_BG_COLOR2_reg, ppoverlay_orbit_bg_color2_reg.regValue);
}

ORBIT_PIXEL_SHIFT_STRUCT vbe_disp_calculate_orbit_position(unsigned int MaxPixelShift, unsigned int MaxLineShift, unsigned int index)
{
	unsigned int _max_index = 0;
	ORBIT_PIXEL_SHIFT_STRUCT orbit_position={0};
	unsigned int group=0, temp=0;
	if(Panel_Get_STR_BackLight_Ctrl_Mode() == _BACKLIGHT_STR_CTRL_MODE_BY_KERNEL){
        //Panel_SetBackLightMode(1);
        unsigned int regValue = rtd_inl(WKSOR_REG);
        if(((regValue & 0xff) != 0x0) && ((regValue & 0xff) != WKSOR_WOL) && ((regValue & 0xff) != WKSOR_RTC))
        {
            Panel_SetBackLightMode(1);
            rtd_pr_vbe_notice("[%s] STR BACKLIGHT CTRL MODE_BY_KERNEL\n",__FUNCTION__);
        }
        else
            rtd_pr_vbe_notice("[%s] RTC/WOW wakeup,do not control backlight\n",__FUNCTION__);
	}
	else if(Panel_Get_STR_BackLight_Ctrl_Mode() == _BACKLIGHT_STR_CTRL_MODE_BY_AP){
		rtd_pr_vbe_notice("[%s] STR BACKLIGHT CTRL MODE_BY_AP, no need turn on back light\n",__FUNCTION__);
	}
	else{
		Panel_SetBackLightMode(1);
		rtd_pr_vbe_notice("[%s] BACKLIGHT ON\n",__FUNCTION__);
	}
	if(MaxLineShift != MaxPixelShift/2){
		rtd_pr_vbe_err("[%s] No Match Orbit Algo.\n",__FUNCTION__);
		return orbit_position;
	}
	_max_index = MaxLineShift*8;

	if(index >= _max_index){
		rtd_pr_vbe_err("[%s] Error update index.\n", __FUNCTION__);
		return orbit_position;
	}
	group = index/MaxLineShift;
	temp  = index%MaxLineShift;

	// x > 0: right shift, y >0 : up shift
	switch(group)
	{
		case 0:
		{
			orbit_position.x = temp;
			orbit_position.y = temp;
			break;
		}
		case 1:
		{
			orbit_position.x = MaxPixelShift/2+temp;
			orbit_position.y = MaxLineShift-temp;
			break;
		}
		case 2:
		{
			orbit_position.x = MaxPixelShift-temp;
			orbit_position.y = -1*temp;
			break;
		}
		case 3:
		{
			orbit_position.x = MaxPixelShift/2-temp;
			orbit_position.y = -1*MaxLineShift+temp;
			break;
		}
		case 4:
		{
			orbit_position.x = -1*temp;
			orbit_position.y = temp;
			break;
		}
		case 5:
		{
			orbit_position.x = -1*(temp+MaxPixelShift/2);
			orbit_position.y = MaxLineShift-temp;
			break;
		}
		case 6:
		{
			orbit_position.x = -1*MaxPixelShift+temp;
			orbit_position.y = -1*temp;
			break;
		}
		case 7:
		{
			orbit_position.x = temp-MaxPixelShift/2;
			orbit_position.y = temp-MaxLineShift;
			break;
		}
		default :
			break;
	}

	return orbit_position;


}

#define SHIFT_FACTOR 1

extern void drvif_color_ultrazoom_scalerup_setorbit(
	int x_in_width,// width before scale up
	int x_in_height,// height before scale up
	int x_out_width,// width after scale up
	int x_out_height,// height after scale up
	int x_out_left,// cropped width after scale up
	int x_out_up,// cropped height after scale up
	int display_width,//real display width
	int display_height,//real display height
	int mode
);


unsigned int vbe_disp_orbit_get_max_table_index(void)
{
	if(vbe_disp_oled_orbit_mode == _VBE_PANEL_ORBIT_JUSTSCAN_MODE){
		return (orbit_move_range[_VBE_PANEL_ORBIT_JUSTSCAN_MODE].x*4);
	}else if(vbe_disp_oled_orbit_mode == _VBE_PANEL_ORBIT_AUTO_MODE){
		return (orbit_move_range[_VBE_PANEL_ORBIT_AUTO_MODE].x*4);
	}else if(vbe_disp_oled_orbit_mode == _VBE_PANEL_ORBIT_STORE_MODE){
		return (orbit_move_range[_VBE_PANEL_ORBIT_STORE_MODE].x*4);
	}else{
		return (orbit_move_range[_VBE_PANEL_ORBIT_JUSTSCAN_MODE].x*4);
	}

}


unsigned char vbe_disp_orbit_set_position_justscan(int x, int y)
{
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl2_reg;
	ppoverlay_display_timing_ctrl2_RBUS ppoverlay_display_timing_ctrl2_reg;
	ppoverlay_memcdtg_control_4_RBUS 	ppoverlay_memcdtg_control_4_reg;
	//SCALER_ORBIT_SHIFT_INFO scalerOrbitMemInfo = {0};
    sfg_sfg_ctrl_1_RBUS sfg_sfg_ctrl_1_reg;
    sfg_sfg_dummy_ctrl_0a_RBUS sfg_sfg_dummy_ctrl_0a_reg;
    int current_line_count =0;
    unsigned char ret = -1;

	//set orbit timing
	ppoverlay_display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	ppoverlay_display_timing_ctrl2_reg.orbit_timing_en = 1;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, ppoverlay_display_timing_ctrl2_reg.regValue);

    //close double buffer
	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

    sfg_sfg_ctrl_1_reg.regValue = IoReg_Read32(SFG_SFG_CTRL_1_reg);
    sfg_sfg_dummy_ctrl_0a_reg.regValue = IoReg_Read32(SFG_SFG_DUMMY_CTRL_0A_reg);

	//set pixel & line shift
	ppoverlay_memcdtg_control_4_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_4_reg);
	if(!(x%2)){//2n
	    if(x ==0){
            sfg_sfg_ctrl_1_reg.seg0_start = 0;
            sfg_sfg_dummy_ctrl_0a_reg.regValue = 0;
            ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft = -1*x;
	    }
		if(x < 0){
            sfg_sfg_ctrl_1_reg.seg0_start = 0;
            ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft = -1*x;

		}else{
            sfg_sfg_dummy_ctrl_0a_reg.regValue = 0;
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft = x;
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft |= _BIT12; 	//sign bit
		}
	}
    else{
        if(x < 0){// 2n+1
            //vbe_orbit_shift_flag = 0;
            sfg_sfg_ctrl_1_reg.seg0_start = 1;
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft = -1*(x+1);
		}else{
            sfg_sfg_dummy_ctrl_0a_reg.regValue = 0x44440000;
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft = x -1;
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft |= _BIT12; 	//sign bit
		}
    }
	if(y >=0){
	 ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_line_sft = y;
	}else{
	 ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_line_sft = (-1*y);
	 ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_line_sft |= _BIT12;	 //sign bit
	}
    current_line_count = PPOVERLAY_new_meas1_linecnt_real_get_memcdtg_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg));
    //if((current_line_count - pre_den_end_line_count <10) && (current_line_count - pre_den_end_line_count >-15))
    {
    	IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL_4_reg, ppoverlay_memcdtg_control_4_reg.regValue);
        //rtd_pr_vbe_emerg("orbit_set: set orbit line count  =  %d\n", PPOVERLAY_new_meas1_linecnt_real_get_memcdtg_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg)) );
        IoReg_Write32(SFG_SFG_CTRL_1_reg, sfg_sfg_ctrl_1_reg.regValue);
        IoReg_Write32(SFG_SFG_DUMMY_CTRL_0A_reg, sfg_sfg_dummy_ctrl_0a_reg.regValue);
        //rtd_pr_vbe_emerg("orbit_set: set sfg line count =  %d\n", PPOVERLAY_new_meas1_linecnt_real_get_memcdtg_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg)) );
        ret = 0;
        pre_den_end_line_count = current_line_count;
    }/*
    else
    {
        //rtd_pr_vbe_emerg("orbit_set: pre_den_end_line_count =  %d,current_line_count=%d \n", pre_den_end_line_count,current_line_count );
        pre_den_end_line_count = current_line_count;
        return ret;
    }


    //enable double buffer
	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

	scalerOrbitMemInfo.orbit_enable = 1;
	scalerOrbitMemInfo.orbit_mode = _VBE_PANEL_ORBIT_JUSTSCAN_MODE;
	scalerOrbitMemInfo.shift_range_x = orbit_move_range[_VBE_PANEL_ORBIT_JUSTSCAN_MODE].x*2;
	scalerOrbitMemInfo.shift_range_y = orbit_move_range[_VBE_PANEL_ORBIT_JUSTSCAN_MODE].y*2;
	scalerOrbitMemInfo.shift_x = x;
	scalerOrbitMemInfo.shift_y = y;
	vbe_disp_set_orbit_shift_info_sharememory(scalerOrbitMemInfo);
    */
	rtd_pr_vbe_emerg("[vbe_disp_orbit_set_position_justscan] orbit_pixel_shift = %d, orbit_line_shift=%d, reg=(%x) \n", x, y, IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_4_reg));
    return ret;
}

extern int zoom_imd_smooth_toggle_config(unsigned char display, SCALER_VSC_INPUT_REGION_TYPE inputType, StructSrcRect inputRegion, StructSrcRect inputTiming, StructSrcRect outputRegion);
#if IS_ENABLED(CONFIG_RTK_KDRV_GDMA)
extern bool GDMA_OSDShiftUpscale_wrapper(int disPlane, int h_shift_pixels,
		 				  int v_shift_pixels, int h_move_range,
						  int v_move_range, int apply);
#endif

static unsigned char overscan_Vo=0;
static unsigned char overscan_Frc=0;
static unsigned char overscan_Sub=0;
static unsigned char overscan_OSD=0;
static unsigned char orbit_force_update=0;

StructSrcRect vbe_orbit_input_size={0};
StructSrcRect vbe_orbit_output_size={0};
StructSrcRect vbe_orbit_sub_input_size={0};
StructSrcRect vbe_orbit_sub_input_timing_size={0};
StructSrcRect vbe_orbit_sub_output_size={0};

static int vo_first_set =1;
static int frc_first_set =1;
//static int pig_mode_x_sta =0 ,pig_mode_x_end=0,pig_mode_y_sta=0,pig_mode_y_end=0;

void vbe_disp_orbit_set_vo_overscan_state(unsigned char bEnable)
{
   vo_first_set = bEnable;
}
unsigned char vbe_disp_orbit_get_sub_overscan(void)
{
	return overscan_Sub;
}

void vbe_disp_orbit_set_sub_overscan(unsigned char bEnable)
{
	overscan_Sub = bEnable;
}
void vbe_disp_orbit_set_vo_overscan(unsigned char bEnable)
{
	overscan_Vo = bEnable;
}

unsigned char vbe_disp_orbit_get_vo_overscan(void)
{
	return overscan_Vo;
}

void vbe_disp_orbit_set_forceupdate_state(unsigned char bEnable)
{
    orbit_force_update = bEnable;
}
unsigned char vbe_disp_orbit_get_forceupdate_state(void)
{
    return orbit_force_update;
}

void vbe_disp_orbit_set_frc_overscan(unsigned char bEnable)
{
	overscan_Frc = bEnable;
}

unsigned char vbe_disp_orbit_get_frc_overscan(void)
{
	return overscan_Frc;
}
void vbe_disp_orbit_set_osd_overscan(unsigned char bEnable)
{
	overscan_OSD = bEnable;
}

unsigned char vbe_disp_orbit_get_osd_overscan(void)
{
	return overscan_OSD;
}

int ori_input_width =0, ori_input_height=0;

void vbe_disp_orbit_vo_overscan()
{
    //static int change =0;
    //StructSrcRect input_size;
    //StructSrcRect input_timing;
    //StructSrcRect output_dispwin;
    scaledown_ich1_sdnr_cutout_range_hor_RBUS scaledown_ich1_sdnr_cutout_range_hor_reg;
    scaledown_ich1_sdnr_cutout_range_ver0_RBUS scaledown_ich1_sdnr_cutout_range_ver0_reg;
    ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;
    ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_reg;
    SCALER_ORBIT_SHIFT_INFO scalerOrbitMemInfo = {0};
    int shift_x, shift_y,range_x,range_y;
    //int vo_crop_front_y,vo_crop_back_y;
    int input_frame_rate =0;
    ORBIT_PIXEL_SHIFT_STRUCT orbit_shift;
    //int disp_width, disp_height;

    extern UINT8 TV006_WB_Pattern_En;

    down(&Orbit_Semaphore);

    if(TV006_WB_Pattern_En)
    {
        up(&Orbit_Semaphore);
        return;
    }

    main_active_h_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);
    main_active_v_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);
    //disp_width = Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
    //disp_height = Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN);
    //disp_width = main_active_h_start_end_reg.mh_act_end - main_active_h_start_end_reg.mh_act_sta ;// Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DISP_WID);
    //disp_height = main_active_v_start_end_reg.mv_act_end - main_active_v_start_end_reg.mv_act_sta;//Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DISP_LEN);
    orbit_shift = Get_Orbit_Shift_Data();
    shift_x = orbit_move_range[vbe_disp_oled_orbit_mode].x - orbit_shift.x;
    shift_y = orbit_move_range[vbe_disp_oled_orbit_mode].y + orbit_shift.y;
    range_x = orbit_move_range[vbe_disp_oled_orbit_mode].x*2;
    range_y = orbit_move_range[vbe_disp_oled_orbit_mode].y*2;

    scaledown_ich1_sdnr_cutout_range_hor_reg.regValue = IoReg_Read32(SCALEDOWN_ich1_sdnr_cutout_range_hor_reg);
    scaledown_ich1_sdnr_cutout_range_ver0_reg.regValue = IoReg_Read32(SCALEDOWN_ich1_sdnr_cutout_range_ver0_reg);
    //vbe_orbit_input_size = get_input_for_crop(SLR_MAIN_DISPLAY);
    //vbe_orbit_output_size = Scaler_DispWindowGet();
    /*
    if(vo_first_set ==1 && scaledown_ich1_sdnr_cutout_range_hor_reg.hor_width!=0 && scaledown_ich1_sdnr_cutout_range_ver0_reg.ver_height !=0)
    {
        vo_first_set = 0;
        ori_input_width = scaledown_ich1_sdnr_cutout_range_hor_reg.hor_width;
        ori_input_height = scaledown_ich1_sdnr_cutout_range_ver0_reg.ver_height;

        rtd_pr_vbe_notice("[%s] ori_input_width = %d, ori_input_height = %d \n", __FUNCTION__, ori_input_width,ori_input_height);
    }

    real_shift_y = (shift_y *ori_input_height)/ (vbe_orbit_output_size.src_height+range_y) ;
    //range_y = (vbe_orbit_output_size.src_height/ori_input_height)*(range_y *ori_input_height)/ (vbe_orbit_output_size.src_height+range_y) ;
    vo_crop_front_y = (real_shift_y /8) *8;

    if(ori_input_width > 2560 && ori_input_height >1440)
    {
        vo_crop_back_y = ( (range_y -real_shift_y)/8) *8;
    }
    else if(ori_input_width == 1920 && ori_input_height ==1080)
    {
        real_shift_y = ((range_y -shift_y) *ori_input_height)/ (vbe_orbit_output_size.src_height+range_y) ;
        vo_crop_back_y = ( (real_shift_y)/8) *8;
    }
    else
    {
        vo_crop_back_y = ori_input_height - (vbe_orbit_output_size.src_height*ori_input_height)/ (vbe_orbit_output_size.src_height+range_y) - real_shift_y;
        vo_crop_back_y = ( (vo_crop_back_y)/8) *8;
    }

    input_size.srcx = vbe_orbit_input_size.srcx;
    input_size.srcy = vbe_orbit_input_size.srcy + vo_crop_front_y;// real_shift_y - ((orbit_move_range[_VBE_PANEL_ORBIT_AUTO_MODE].y*2) - vo_crop_y)/2;
    input_size.src_wid = vbe_orbit_input_size.src_wid;//scaledown_ich1_sdnr_cutout_range_hor_reg.hor_width;
    //input_size.src_height = scaledown_ich1_sdnr_cutout_range_ver0_reg.ver_height-orbit_move_range[_VBE_PANEL_ORBIT_AUTO_MODE].y*2;
    input_size.src_height = ori_input_height-vo_crop_front_y -vo_crop_back_y;//scaledown_ich1_sdnr_cutout_range_ver0_reg.ver_height-8;
    rtd_pr_vbe_notice("[%s] real_shift_y = %d ,vo_crop_front_y = %d, vo_crop_back_y = %d \n", __FUNCTION__,real_shift_y, vo_crop_front_y,vo_crop_back_y);

    input_timing.srcx = 0;
    input_timing.srcy = 0;
    input_timing.src_wid = ori_input_width;//scaledown_ich1_sdnr_cutout_range_hor_reg.hor_width;
    input_timing.src_height = ori_input_height;//scaledown_ich1_sdnr_cutout_range_ver0_reg.ver_height;
    if(vbe_orbit_output_size.src_wid !=0 && vbe_orbit_output_size.src_height !=0)
    {
        if(disp_width/vbe_orbit_output_size.src_wid>2)
        {
            output_dispwin.srcx = 0;
            output_dispwin.srcy = 0;
            output_dispwin.src_wid = disp_width ;
            output_dispwin.src_height = disp_height;

        }
        else
        {
            output_dispwin.srcx = vbe_orbit_output_size.srcx;
            output_dispwin.srcy = vbe_orbit_output_size.srcy;
            output_dispwin.src_wid = vbe_orbit_output_size.src_wid ;
            output_dispwin.src_height = vbe_orbit_output_size.src_height;
        }
    }

    rtd_pr_vbe_notice("[%s] input_size.srcx = %d ,input_size.srcy = %d, input_size.src_wid = %d , input_size.src_height=%d\n", __FUNCTION__,input_size.srcx, input_size.srcy,input_size.src_wid,input_size.src_height);
    rtd_pr_vbe_notice("[%s] output_dispwin.srcx = %d ,output_dispwin.srcy = %d, output_dispwin.src_wid = %d ,output_dispwin.src_height=%d\n", __FUNCTION__,output_dispwin.srcx, output_dispwin.srcy,output_dispwin.src_wid,output_dispwin.src_height);
    */
    scalerOrbitMemInfo.orbit_enable = 1;
    scalerOrbitMemInfo.orbit_mode = _VBE_PANEL_ORBIT_AUTO_MODE;
    scalerOrbitMemInfo.shift_range_x = range_x;
    scalerOrbitMemInfo.shift_range_y = range_y;
    scalerOrbitMemInfo.shift_x = shift_x;
    scalerOrbitMemInfo.shift_y = shift_y;
    scalerOrbitMemInfo.ori_input_x = (ori_input_width > vbe_orbit_output_size.src_wid)? vbe_orbit_output_size.src_wid :ori_input_width ;
    scalerOrbitMemInfo.ori_input_y = ori_input_height;
    scalerOrbitMemInfo.force_update = vbe_disp_orbit_get_forceupdate_state();
    scalerOrbitMemInfo.execute_count = 1;
    scalerOrbitMemInfo.active_state = 1;
    vbe_disp_set_orbit_shift_info_sharememory(scalerOrbitMemInfo);

    input_frame_rate = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_V_FREQ);

    rtd_pr_vbe_notice("[%s] input_frame_rate = %d, disp_width = %d, disp_height=%d\n", __FUNCTION__, input_frame_rate,Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID),Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN));
/*
    if(zoom_imd_smooth_toggle_config(SLR_MAIN_DISPLAY, INPUT_BASE_ON_TIMING, input_size, input_timing, output_dispwin)==0)
    {
        rtd_pr_vbe_notice("[%s] zoom_imd_smooth_toggle done \n", __FUNCTION__);
    }*/
    //vbe_disp_orbit_set_osd_overscan(1);
    vbe_disp_orbit_set_forceupdate_state(0);
    vbe_disp_orbit_set_vo_overscan(0);

    up(&Orbit_Semaphore);
}

void vbe_disp_orbit_sub_overscan()
{

    extern StructSrcRect Scaler_SubDispWindowGet(void);
    StructSrcRect input_size;
    StructSrcRect input_timing;
    StructSrcRect output_dispwin;
    SCALER_ORBIT_SHIFT_INFO *orbit_shift_info = NULL;


    ORBIT_PIXEL_SHIFT_STRUCT orbit_shift;
    int shift_x, shift_y, range_x,range_y;
    orbit_shift_info = (SCALER_ORBIT_SHIFT_INFO*)Scaler_GetShareMemVirAddr(SCALERIOC_ORBIT_SHIFT_INFO);

    orbit_shift = Get_Orbit_Shift_Data();
    shift_x = orbit_move_range[vbe_disp_oled_orbit_mode].x - orbit_shift.x;
    shift_y = orbit_move_range[vbe_disp_oled_orbit_mode].y + orbit_shift.y;
    range_x = orbit_move_range[vbe_disp_oled_orbit_mode].x*2;
    range_y = orbit_move_range[vbe_disp_oled_orbit_mode].y*2;//+ orbit_move_range[vbe_disp_oled_orbit_mode].y/2;
    if(vbe_orbit_sub_input_size.src_wid==0 || vbe_orbit_sub_input_size.src_height==0)
    {
        vbe_orbit_sub_input_size = get_input_for_crop(SLR_SUB_DISPLAY);
    }
    if(vbe_orbit_sub_input_timing_size.src_wid==0 || vbe_orbit_sub_input_timing_size.src_height==0)
    {
        vbe_orbit_sub_input_timing_size = get_input_timing_for_crop(SLR_SUB_DISPLAY);
    }

    vbe_orbit_sub_input_size = get_input_for_crop(SLR_SUB_DISPLAY);
    vbe_orbit_sub_input_timing_size = get_input_timing_for_crop(SLR_SUB_DISPLAY);

    input_size.srcx = vbe_orbit_sub_input_size.srcx;
    input_size.srcy = vbe_orbit_sub_input_size.srcy ;
    input_size.src_wid = vbe_orbit_sub_input_size.src_wid;
    input_size.src_height = vbe_orbit_sub_input_size.src_height;

    input_timing.srcx = vbe_orbit_sub_input_timing_size.srcx ;
    input_timing.srcy = vbe_orbit_sub_input_timing_size.srcy ;
    input_timing.src_height = vbe_orbit_sub_input_timing_size.src_height ;
    input_timing.src_wid = vbe_orbit_sub_input_timing_size.src_wid ;
    vbe_orbit_sub_output_size = Scaler_SubDispWindowGet();

    if(Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE || vbe_disp_oled_orbit_mode == _VBE_PANEL_ORBIT_STORE_MODE)
    {
        vbe_disp_orbit_set_sub_overscan(0);
        vbe_orbit_sub_input_size.src_wid = 0;
        vbe_orbit_sub_input_size.src_height = 0;
        vbe_orbit_sub_input_timing_size.src_height =0;
        vbe_orbit_sub_input_timing_size.src_wid =0;

        rtd_pr_vbe_notice("[%s] no sub input size or store mode\n", __FUNCTION__);
        return;
    }
    /*
    if(vbe_orbit_sub_output_size.srcx >1910)//right side
    {
        output_dispwin.srcx = vbe_orbit_sub_output_size.srcx + orbit_move_range[vbe_disp_oled_orbit_mode].x + 5 - shift_x;
        output_dispwin.srcy = vbe_orbit_sub_output_size.srcy - shift_y;
        output_dispwin.src_height = vbe_orbit_sub_output_size.src_height + range_y;
        output_dispwin.src_wid = vbe_orbit_sub_output_size.src_wid  +range_x;
    }
    else//left side
    {
        if(vbe_orbit_sub_output_size.srcx < shift_x)
        {
            input_size.srcx = vbe_orbit_sub_input_size.srcx + shift_x;
            input_size.srcy = vbe_orbit_sub_input_size.srcy ;
            input_size.src_wid = vbe_orbit_sub_input_size.src_wid ;
            input_size.src_height = vbe_orbit_sub_input_size.src_height;

            output_dispwin.srcx = 0;
            output_dispwin.srcy = vbe_orbit_sub_output_size.srcy - shift_y;
            output_dispwin.src_height = vbe_orbit_sub_output_size.src_height + range_y;
            output_dispwin.src_wid = vbe_orbit_sub_output_size.src_wid  +range_x;
        }
        else
        {
            output_dispwin.srcx = vbe_orbit_sub_output_size.srcx - shift_x;
            output_dispwin.srcy = vbe_orbit_sub_output_size.srcy - shift_y;
            output_dispwin.src_height = vbe_orbit_sub_output_size.src_height + range_y;
            output_dispwin.src_wid = vbe_orbit_sub_output_size.src_wid  +range_x;
        }
    }*/
    //for main path might be no signal.
    orbit_shift_info->orbit_mode = htonl(_VBE_PANEL_ORBIT_AUTO_MODE);
    orbit_shift_info->shift_range_x = htonl(range_x);
    orbit_shift_info->shift_range_y = htonl(range_y);
    orbit_shift_info->shift_x = htonl(shift_x);
    orbit_shift_info->shift_y = htonl(shift_y);

    output_dispwin.srcx = vbe_orbit_sub_output_size.srcx ;
    output_dispwin.srcy = vbe_orbit_sub_output_size.srcy ;
    output_dispwin.src_height = vbe_orbit_sub_output_size.src_height ;
    output_dispwin.src_wid = vbe_orbit_sub_output_size.src_wid  ;
    rtd_pr_vbe_notice("[%s] input_size.srcx = %d ,input_size.srcy = %d, input_size.src_wid = %d , input_size.src_height=%d\n", __FUNCTION__,input_size.srcx, input_size.srcy,input_size.src_wid,input_size.src_height);
    rtd_pr_vbe_notice("[%s] output_dispwin.srcx = %d ,output_dispwin.srcy = %d, output_dispwin.src_wid = %d ,output_dispwin.src_height=%d\n", __FUNCTION__,output_dispwin.srcx, output_dispwin.srcy,output_dispwin.src_wid,output_dispwin.src_height);


    if(zoom_imd_smooth_toggle_config(SLR_SUB_DISPLAY, INPUT_BASE_ON_TIMING, input_size, input_timing, output_dispwin)==0)
    {
        rtd_pr_vbe_notice("[%s] sub zoom_imd_smooth_toggle done \n", __FUNCTION__);
    }
    vbe_disp_orbit_set_sub_overscan(0);
}


void vbe_disp_orbit_frc_overscan()
{

    static ORBIT_SIZE inSize;
	int shift_x, shift_y,range_x,range_y;
	scaledown_ich1_sdnr_cutout_range_hor_RBUS scaledown_ich1_sdnr_cutout_range_hor_reg;
	scaledown_ich1_sdnr_cutout_range_ver0_RBUS scaledown_ich1_sdnr_cutout_range_ver0_reg;
    ppoverlay_main_den_h_start_end_RBUS main_den_h_start_end_reg;
    ppoverlay_main_den_v_start_end_RBUS main_den_v_start_end_reg;
    ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;
    ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_reg;
	//SCALER_ORBIT_SHIFT_INFO scalerOrbitMemInfo = {0};
	//SCALER_DISP_CHANNEL display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
    //int ori_disp_width,ori_disp_height;
	ORBIT_PIXEL_SHIFT_STRUCT orbit_shift;
    extern UINT8 TV006_WB_Pattern_En;
	SCALER_ORBIT_SHIFT_INFO scalerOrbitMemInfo = {0};
    down(&Orbit_Semaphore);

    if(TV006_WB_Pattern_En)
    {
        up(&Orbit_Semaphore);
        return;
    }

    orbit_shift = Get_Orbit_Shift_Data();
	scaledown_ich1_sdnr_cutout_range_hor_reg.regValue = IoReg_Read32(SCALEDOWN_ich1_sdnr_cutout_range_hor_reg);
	scaledown_ich1_sdnr_cutout_range_ver0_reg.regValue = IoReg_Read32(SCALEDOWN_ich1_sdnr_cutout_range_ver0_reg);

    main_active_h_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);
	main_active_v_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);

	//ori_disp_width = main_active_h_start_end_reg.mh_act_end - main_active_h_start_end_reg.mh_act_sta ;// Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DISP_WID);
	//ori_disp_height = main_active_v_start_end_reg.mv_act_end - main_active_v_start_end_reg.mv_act_sta;//Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DISP_LEN);

	inSize.nLength = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_MEM_ACT_LEN);//scaledown_ich1_sdnr_cutout_range_ver0_reg.ver_height;// ;
	inSize.nWidth = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_MEM_ACT_WID);//scaledown_ich1_sdnr_cutout_range_hor_reg.hor_width;//

    rtd_pr_vbe_notice("[%s] inSize.nLength = %d, inSize.nWidth = %d \n", __FUNCTION__, inSize.nLength,inSize.nWidth);

    shift_x = orbit_move_range[vbe_disp_oled_orbit_mode].x - orbit_shift.x;
    shift_y = orbit_move_range[vbe_disp_oled_orbit_mode].y + orbit_shift.y;
    range_x = orbit_move_range[vbe_disp_oled_orbit_mode].x*2;
    range_y = orbit_move_range[vbe_disp_oled_orbit_mode].y*2;

    /*
    drvif_color_ultrazoom_scalerup_setorbit(
        inSize.nWidth,
        inSize.nLength,
        ori_disp_width+range_x,
        ori_disp_height+range_y,
        shift_x,
        shift_y,
        ori_disp_width,
        ori_disp_height,
        0);
*/
    //msleep(5);

    main_den_h_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_DEN_H_Start_End_reg);
    main_den_v_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_DEN_V_Start_End_reg);

    /*
    if(vbe_disp_oled_orbit_mode ==_VBE_PANEL_ORBIT_AUTO_MODE &&
        main_den_h_start_end_reg.mh_den_sta < main_active_h_start_end_reg.mh_act_sta &&
       main_den_v_start_end_reg.mv_den_sta <  main_active_v_start_end_reg.mv_act_sta)
    {
        if(frc_first_set)
        {
            frc_first_set =0;
            pig_mode_x_sta = main_active_h_start_end_reg.mh_act_sta;
            pig_mode_x_end = main_active_h_start_end_reg.mh_act_end;
            pig_mode_y_sta = main_active_v_start_end_reg.mv_act_sta;
            pig_mode_y_end = main_active_v_start_end_reg.mv_act_end;

            rtd_pr_vbe_notice("[%s] pig_mode_x_sta = %d, pig_mode_x_end =%d, pig_mode_y_sta =%d, pig_mode_y_end=%d\n", __FUNCTION__,pig_mode_x_sta,pig_mode_x_end,pig_mode_y_sta,pig_mode_y_end);
        }

        rtd_pr_vbe_notice("[%s] orbit_shift.x = %d, orbit_shift.y =%d\n", __FUNCTION__,orbit_shift.x,orbit_shift.y);
        main_active_h_start_end_reg.mh_act_sta = pig_mode_x_sta + orbit_shift.x ;
        main_active_h_start_end_reg.mh_act_end = pig_mode_x_end + orbit_shift.x ;
        IoReg_Write32(PPOVERLAY_MAIN_Active_H_Start_End_reg, main_active_h_start_end_reg.regValue);
        main_active_v_start_end_reg.mv_act_sta = pig_mode_y_sta - orbit_shift.y;
        main_active_v_start_end_reg.mv_act_end = pig_mode_y_end - orbit_shift.y;
        IoReg_Write32(PPOVERLAY_MAIN_Active_V_Start_End_reg, main_active_v_start_end_reg.regValue);
    }
    else
    {
        frc_first_set =1;
    }
    */
	scalerOrbitMemInfo.orbit_enable = 1;
	scalerOrbitMemInfo.orbit_mode = _VBE_PANEL_ORBIT_AUTO_MODE;
	scalerOrbitMemInfo.shift_range_x = range_x;
	scalerOrbitMemInfo.shift_range_y = range_y;
	scalerOrbitMemInfo.shift_x = shift_x;
	scalerOrbitMemInfo.shift_y = shift_y;
	scalerOrbitMemInfo.ori_input_x = inSize.nWidth ;
	scalerOrbitMemInfo.ori_input_y = inSize.nLength;
	scalerOrbitMemInfo.force_update = vbe_disp_orbit_get_forceupdate_state();
	scalerOrbitMemInfo.execute_count = 0;
    scalerOrbitMemInfo.active_state = 1;
	vbe_disp_set_orbit_shift_info_sharememory(scalerOrbitMemInfo);
    vbe_disp_orbit_set_forceupdate_state(0);
    //vbe_disp_orbit_set_osd_overscan(1);
    //vbe_disp_orbit_OSD_overscan();
    up(&Orbit_Semaphore);
}

void vbe_disp_orbit_OSD_overscan()
{

	int shift_x, shift_y;

	ORBIT_PIXEL_SHIFT_STRUCT orbit_shift;

    orbit_shift = Get_Orbit_Shift_Data();
    shift_x = orbit_move_range[_VBE_PANEL_ORBIT_AUTO_MODE].x - orbit_shift.x;
    shift_y = orbit_move_range[_VBE_PANEL_ORBIT_AUTO_MODE].y + orbit_shift.y;

    rtd_pr_vbe_notice("[%s] GDMA_OSDShiftUpscale \n", __FUNCTION__);

    if(vbe_disp_oled_orbit_mode ==_VBE_PANEL_ORBIT_AUTO_MODE)
    {
#if IS_ENABLED(CONFIG_RTK_KDRV_GDMA)
        GDMA_OSDShiftUpscale_wrapper(1,shift_x,shift_y,orbit_move_range[_VBE_PANEL_ORBIT_AUTO_MODE].x*2,
                        orbit_move_range[_VBE_PANEL_ORBIT_AUTO_MODE].y*2,0);
#endif
    }
}


void vbe_disp_orbit_set_position_overscan(int x, int y)
{
	if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_FRAMESYNC)){
		// data fsync need crop by vo, can not overscan here
		vbe_disp_orbit_set_vo_overscan(1);
		vbe_disp_orbit_set_sub_overscan(1);
        rtd_pr_vbe_notice("[%s] data frame sync mode \n", __FUNCTION__);
        frc_first_set =1;
	}else{
        vo_first_set =1 ;// reset for data fs vo crop
		vbe_disp_orbit_set_frc_overscan(1);
		vbe_disp_orbit_set_sub_overscan(1);
        rtd_pr_vbe_notice("[%s] FRC mode \n", __FUNCTION__);
	}
    vbe_disp_orbit_set_osd_overscan(0);

}

void vbe_disp_set_orbit_shift_info_sharememory(SCALER_ORBIT_SHIFT_INFO scalerOrbitInfo)
{
	SCALER_ORBIT_SHIFT_INFO *orbit_shift_info = NULL;

	orbit_shift_info = (SCALER_ORBIT_SHIFT_INFO*)Scaler_GetShareMemVirAddr(SCALERIOC_ORBIT_SHIFT_INFO);
	orbit_shift_info->shift_range_x = htonl(scalerOrbitInfo.shift_range_x);
	orbit_shift_info->shift_range_y = htonl(scalerOrbitInfo.shift_range_y);
	orbit_shift_info->shift_x = htonl(scalerOrbitInfo.shift_x);
	orbit_shift_info->shift_y = htonl(scalerOrbitInfo.shift_y);
	orbit_shift_info->ori_input_x = htonl(scalerOrbitInfo.ori_input_x);
	orbit_shift_info->ori_input_y = htonl(scalerOrbitInfo.ori_input_y);
	orbit_shift_info->force_update = htonl(scalerOrbitInfo.force_update);
	orbit_shift_info->orbit_mode = htonl(scalerOrbitInfo.orbit_mode);
	orbit_shift_info->orbit_enable = htonl(scalerOrbitInfo.orbit_enable);
	orbit_shift_info->execute_count = htonl(scalerOrbitInfo.execute_count);
    orbit_shift_info->active_state = htonl(scalerOrbitInfo.active_state);
	rtd_pr_vbe_notice("[%s] x=%d,y=%d, range(%d.%d)(enable:%d, mode:%d)\n", __FUNCTION__, scalerOrbitInfo.shift_x, scalerOrbitInfo.shift_y,
		scalerOrbitInfo.shift_range_x, scalerOrbitInfo.shift_range_y, scalerOrbitInfo.orbit_enable, scalerOrbitInfo.orbit_mode);
}


void vbe_disp_orbit_reset_position_justscan(void)
{
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl2_reg;
	ppoverlay_display_timing_ctrl2_RBUS ppoverlay_display_timing_ctrl2_reg;
	ppoverlay_memcdtg_control_4_RBUS 	ppoverlay_memcdtg_control_4_reg;
    sfg_sfg_ctrl_1_RBUS sfg_sfg_ctrl_1_reg;
    sfg_sfg_dummy_ctrl_0a_RBUS sfg_sfg_dummy_ctrl_0a_reg;
	//SCALER_ORBIT_SHIFT_INFO scalerOrbitMemInfo = {0};
	// reset orbit
	ppoverlay_display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	ppoverlay_display_timing_ctrl2_reg.orbit_timing_en = 0;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, ppoverlay_display_timing_ctrl2_reg.regValue);

	ppoverlay_memcdtg_control_4_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_4_reg);
	ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_line_sft = 0;
	ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft = 0;
	IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL_4_reg, ppoverlay_memcdtg_control_4_reg.regValue);

    sfg_sfg_ctrl_1_reg.regValue = IoReg_Read32(SFG_SFG_CTRL_1_reg);
    sfg_sfg_dummy_ctrl_0a_reg.regValue = IoReg_Read32(SFG_SFG_DUMMY_CTRL_0A_reg);
    sfg_sfg_ctrl_1_reg.seg0_start = 0;
    sfg_sfg_dummy_ctrl_0a_reg.regValue = 0;
    IoReg_Write32(SFG_SFG_CTRL_1_reg, sfg_sfg_ctrl_1_reg.regValue);
    IoReg_Write32(SFG_SFG_DUMMY_CTRL_0A_reg, sfg_sfg_dummy_ctrl_0a_reg.regValue);

	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
    /*
	scalerOrbitMemInfo.orbit_enable = 0;
	scalerOrbitMemInfo.orbit_mode = _VBE_PANEL_OREBIT_MODE_MAX;
	scalerOrbitMemInfo.shift_range_x = 0;
	scalerOrbitMemInfo.shift_range_y = 0;
	scalerOrbitMemInfo.shift_x = 0;
	scalerOrbitMemInfo.shift_y = 0;
	scalerOrbitMemInfo.ori_input_x = Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
	scalerOrbitMemInfo.ori_input_y = Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN);
	vbe_disp_set_orbit_shift_info_sharememory(scalerOrbitMemInfo);*/
}

void vbe_disp_orbit_reset_position_overscan(void)
{
    ORBIT_SIZE inSize;
    scaledown_ich1_sdnr_cutout_range_hor_RBUS scaledown_ich1_sdnr_cutout_range_hor_reg;
    scaledown_ich1_sdnr_cutout_range_ver0_RBUS scaledown_ich1_sdnr_cutout_range_ver0_reg;
    //ppoverlay_main_den_h_start_end_RBUS main_den_h_start_end_reg;
    //ppoverlay_main_den_v_start_end_RBUS main_den_v_start_end_reg;
    //ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;
    //ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_reg;

    SCALER_ORBIT_SHIFT_INFO scalerOrbitMemInfo = {0};
    //SCALER_ORBIT_SHIFT_INFO *p_orbitShiftInfo = NULL;
    StructSrcRect input_size;
    StructSrcRect input_timing;
    StructSrcRect output_dispwin = {0, 0, 3840, 2160};
    //SCALER_DISP_CHANNEL display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
    int ori_disp_wid,ori_disp_len;
    scaledown_ich1_sdnr_cutout_range_hor_reg.regValue = IoReg_Read32(SCALEDOWN_ich1_sdnr_cutout_range_hor_reg);
    scaledown_ich1_sdnr_cutout_range_ver0_reg.regValue = IoReg_Read32(SCALEDOWN_ich1_sdnr_cutout_range_ver0_reg);
    inSize.nLength = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_MEM_ACT_LEN);
    inSize.nWidth = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_MEM_ACT_WID);///will change after vo crop

    ori_disp_wid = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_DISP_WID);
    ori_disp_len = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_DISP_LEN);

    rtd_pr_vbe_notice("[%s] inSize.nWidth = %d, inSize.nLength = %d ,ori_disp_wid =%d ,ori_disp_len=%d\n", __FUNCTION__,inSize.nWidth,inSize.nLength,ori_disp_wid ,ori_disp_len);
    if(inSize.nLength !=0 && inSize.nWidth!=0)///make sure it has video source
        ;//p_orbitShiftInfo = (SCALER_ORBIT_SHIFT_INFO*)Scaler_GetShareMemVirAddr(SCALERIOC_ORBIT_SHIFT_INFO);//get original info from share memory

    if( vbe_orbit_input_size.src_height!=0 && vbe_orbit_input_size.src_wid !=0)
    {
        scalerOrbitMemInfo.orbit_enable = 1;
        scalerOrbitMemInfo.orbit_mode = _VBE_PANEL_OREBIT_MODE_MAX;
        scalerOrbitMemInfo.shift_range_x = 0;
        scalerOrbitMemInfo.shift_range_y = 0;
        scalerOrbitMemInfo.shift_x = 0;
        scalerOrbitMemInfo.shift_y = 0;
        scalerOrbitMemInfo.ori_input_x = vbe_orbit_input_size.src_wid;
        scalerOrbitMemInfo.ori_input_y = vbe_orbit_input_size.src_height;
        scalerOrbitMemInfo.force_update = 0;
        scalerOrbitMemInfo.execute_count = 1;
        scalerOrbitMemInfo.active_state = 1;
        vbe_disp_set_orbit_shift_info_sharememory(scalerOrbitMemInfo);//
    }
    vo_first_set =1;
    frc_first_set =1;
    /*
    main_den_h_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_DEN_H_Start_End_reg);
    main_den_v_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_DEN_V_Start_End_reg);
    main_active_h_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);
    main_active_v_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);
    if(main_den_h_start_end_reg.mh_den_sta < main_active_h_start_end_reg.mh_act_sta &&
        main_den_v_start_end_reg.mv_den_sta <  main_active_v_start_end_reg.mv_act_sta &&
        pig_mode_x_sta!=0 && pig_mode_x_end!=0 && pig_mode_y_sta!=0 && pig_mode_y_end!=0)
    {
        main_active_h_start_end_reg.mh_act_sta = pig_mode_x_sta;
        main_active_h_start_end_reg.mh_act_end = pig_mode_x_end;
        IoReg_Write32(PPOVERLAY_MAIN_Active_H_Start_End_reg, main_active_h_start_end_reg.regValue);
        main_active_v_start_end_reg.mv_act_sta = pig_mode_y_sta;
        main_active_v_start_end_reg.mv_act_end = pig_mode_y_end;
        IoReg_Write32(PPOVERLAY_MAIN_Active_V_Start_End_reg, main_active_v_start_end_reg.regValue);
        pig_mode_x_sta = 0;
        pig_mode_x_end = 0;
        pig_mode_y_sta = 0;
        pig_mode_y_end = 0;
    }
        */
    if(inSize.nLength !=0 && inSize.nWidth!=0)
    {
        unsigned int counter  = 5;
        if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_FRAMESYNC)){
            scalerOrbitMemInfo.orbit_enable = 1;
            scalerOrbitMemInfo.orbit_mode = _VBE_PANEL_ORBIT_AUTO_MODE;
            scalerOrbitMemInfo.shift_range_x = 0;
            scalerOrbitMemInfo.shift_range_y = 0;
            scalerOrbitMemInfo.shift_x = 0;
            scalerOrbitMemInfo.shift_y = 0;
            scalerOrbitMemInfo.ori_input_x = vbe_orbit_input_size.src_wid;
            scalerOrbitMemInfo.ori_input_y = vbe_orbit_input_size.src_height;
            scalerOrbitMemInfo.force_update = 0;
            scalerOrbitMemInfo.execute_count = 1;
            scalerOrbitMemInfo.active_state = 1;
            vbe_disp_set_orbit_shift_info_sharememory(scalerOrbitMemInfo);//

            vbe_disp_orbit_set_vo_overscan(0);
            /*
            // data fsync need crop by vo, can not overscan here
            if( vbe_orbit_input_size.src_height!=0 && vbe_orbit_input_size.src_wid !=0)
            {
                input_size.srcx = vbe_orbit_input_size.srcx ;
                input_size.srcy = vbe_orbit_input_size.srcy ;
                input_size.src_wid = vbe_orbit_input_size.src_wid;
                input_size.src_height = vbe_orbit_input_size.src_height;
                input_timing = get_input_timing_for_crop(SLR_MAIN_DISPLAY);

            }
            else
            {
                vbe_orbit_input_size = get_input_for_crop(SLR_MAIN_DISPLAY);
                input_size = get_input_for_crop(SLR_MAIN_DISPLAY);
                input_timing = get_input_timing_for_crop(SLR_MAIN_DISPLAY);
            }
            rtd_pr_vbe_notice("[%s] input_size.srcx =%d, input_size.srcy=%d, input_size.src_wid =%d, input_size.src_height=%d\n", __FUNCTION__,input_size.srcx,input_size.srcy,input_size.src_wid,input_size.src_height);
            if(vbe_orbit_output_size.src_wid !=0 && vbe_orbit_output_size.src_height !=0 )
            {
                if((ori_disp_wid/vbe_orbit_output_size.src_wid)>2)
                {
                    output_dispwin = Scaler_DispWindowGet();
                }
                else
                {   rtd_pr_vbe_notice("[%s] test src_wid =%d, ori_disp_wid=%d \n", __FUNCTION__,vbe_orbit_output_size.src_wid,ori_disp_wid);
                    output_dispwin.srcx = vbe_orbit_output_size.srcx;
                    output_dispwin.srcy = vbe_orbit_output_size.srcy;
                    output_dispwin.src_wid = vbe_orbit_output_size.src_wid ;
                    output_dispwin.src_height = vbe_orbit_output_size.src_height;
                }

                rtd_pr_vbe_notice("[%s] output_dispwin.srcx =%d, output_dispwin.srcy=%d, output_dispwin.src_wid =%d, output_dispwin.src_height=%d\n", __FUNCTION__,output_dispwin.srcx,output_dispwin.srcy,output_dispwin.src_wid,output_dispwin.src_height);
            }
            else
            {
                output_dispwin = Scaler_DispWindowGet();
                vbe_orbit_output_size = Scaler_DispWindowGet();
            }
            if(zoom_imd_smooth_toggle_config(SLR_MAIN_DISPLAY, INPUT_BASE_ON_TIMING, input_size, input_timing, output_dispwin)==0)
            {
                rtd_pr_vbe_notice("[%s] zoom_imd_smooth_toggle done \n", __FUNCTION__);
            }

            msleep(30);// avoid share memory change when Vcpu get rpc
            */
        }
        else
        {
            scalerOrbitMemInfo.orbit_enable = 1;
            scalerOrbitMemInfo.orbit_mode = _VBE_PANEL_ORBIT_AUTO_MODE;
            scalerOrbitMemInfo.shift_range_x = 0;
            scalerOrbitMemInfo.shift_range_y = 0;
            scalerOrbitMemInfo.shift_x = 0;
            scalerOrbitMemInfo.shift_y = 0;
            scalerOrbitMemInfo.ori_input_x = vbe_orbit_input_size.src_wid;
            scalerOrbitMemInfo.ori_input_y = vbe_orbit_input_size.src_height;
            scalerOrbitMemInfo.force_update = 0;
            scalerOrbitMemInfo.execute_count = 0;
            scalerOrbitMemInfo.active_state = 1;
            vbe_disp_set_orbit_shift_info_sharememory(scalerOrbitMemInfo);//

            vbe_disp_orbit_set_frc_overscan(0);

            rtd_pr_vbe_notice("[%s] FRC mode rest \n", __FUNCTION__);
            if(ori_disp_wid!=0 && ori_disp_len!=0)
            {    drvif_color_ultrazoom_scalerup_setorbit(
                inSize.nWidth,
                inSize.nLength,
                ori_disp_wid,
                ori_disp_len,
                0,
                0,
                ori_disp_wid,
                ori_disp_len,
                0);
            }

        }
        while((IoReg_Read32(SCALEUP_DM_UZUMAIN_H_PARTIAL_DROP_reg) && IoReg_Read32(SCALEUP_DM_UZUMAIN_V_PARTIAL_DROP_reg)) && counter)
        {
            msleep(10);
            counter--;
        }
    }
    scalerOrbitMemInfo.orbit_enable = 0;
    scalerOrbitMemInfo.orbit_mode = _VBE_PANEL_OREBIT_MODE_MAX;
    scalerOrbitMemInfo.shift_range_x = 0;
    scalerOrbitMemInfo.shift_range_y = 0;
    scalerOrbitMemInfo.shift_x = 0;
    scalerOrbitMemInfo.shift_y = 0;
    scalerOrbitMemInfo.ori_input_x = ori_disp_wid;
    scalerOrbitMemInfo.ori_input_y = ori_disp_len;
    scalerOrbitMemInfo.force_update = 0;
    scalerOrbitMemInfo.execute_count = 0;
    scalerOrbitMemInfo.active_state = 0;
    vbe_disp_set_orbit_shift_info_sharememory(scalerOrbitMemInfo);//

    if((vbe_orbit_sub_input_size.src_wid !=0 || vbe_orbit_sub_input_size.src_height!=0))
    {

        vbe_disp_orbit_set_sub_overscan(1);
        input_size.srcx = vbe_orbit_sub_input_size.srcx ;
        input_size.srcy = vbe_orbit_sub_input_size.srcy ;
        input_size.src_wid = vbe_orbit_sub_input_size.src_wid;
        input_size.src_height = vbe_orbit_sub_input_size.src_height;

        input_timing.srcx = vbe_orbit_sub_input_timing_size.srcx ;
        input_timing.srcy = vbe_orbit_sub_input_timing_size.srcy ;
        input_timing.src_wid = vbe_orbit_sub_input_timing_size.src_wid;
        input_timing.src_height = vbe_orbit_sub_input_timing_size.src_height;

        output_dispwin.srcx = vbe_orbit_sub_output_size.srcx;
        output_dispwin.srcy = vbe_orbit_sub_output_size.srcy;
        output_dispwin.src_wid = vbe_orbit_sub_output_size.src_wid ;
        output_dispwin.src_height = vbe_orbit_sub_output_size.src_height;

        if(zoom_imd_smooth_toggle_config(SLR_SUB_DISPLAY, INPUT_BASE_ON_TIMING, input_size, input_timing, output_dispwin)==0)
        {
            rtd_pr_vbe_notice("[%s] zoom_imd_smooth_toggle done \n", __FUNCTION__);
        }

        vbe_disp_orbit_set_sub_overscan(0);
    }

#if IS_ENABLED(CONFIG_RTK_KDRV_GDMA)
    GDMA_OSDShiftUpscale_wrapper(1,0,0,0,0,0);// OSD reset
#endif
}


/*api which have protect by forcebg_semaphore*/
void vbe_disp_orbit_position_update(unsigned char tableIdx,unsigned char force_update)
{
//	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl2_reg;
	static unsigned char previous_tableIdx=0xff;
//	ppoverlay_display_timing_ctrl2_RBUS ppoverlay_display_timing_ctrl2_reg;
//	ppoverlay_memcdtg_control_4_RBUS 	ppoverlay_memcdtg_control_4_reg;
	unsigned int orbit_table_index_max;
	ORBIT_PIXEL_SHIFT_STRUCT orbit_shift;

	if(tableIdx == 0xff){
		//vbe_disp_orbit_reset_position_justscan();
		//vbe_disp_orbit_reset_position_overscan();
		vbe_disp_orbit_justscan_HW_Ctrl(0,0,0,TRUE);
        orbit_shift.x =0;
        orbit_shift.y =0;
        Set_Orbit_Shift_Data(orbit_shift);
        previous_tableIdx=0xff;
		return;
	}

	if(!Scaler_get_orbit_algo_status())
		return;

	orbit_table_index_max = vbe_disp_orbit_get_max_table_index();
    vbe_disp_orbit_set_forceupdate_state(force_update);

	if(previous_tableIdx!=tableIdx || force_update){
		previous_tableIdx=tableIdx;
	}else{
		return;
	}

	if(tableIdx>=orbit_table_index_max){
		tableIdx = 0;
		previous_tableIdx=tableIdx;
	}

	orbit_shift = vbe_disp_calculate_orbit_position(orbit_move_range[vbe_disp_oled_orbit_mode].x, orbit_move_range[vbe_disp_oled_orbit_mode].y, tableIdx);
    Set_Orbit_Shift_Data(orbit_shift);
	if(vbe_disp_oled_orbit_mode == _VBE_PANEL_ORBIT_AUTO_MODE){
		//vbe_disp_orbit_set_position_overscan(orbit_shift.x, orbit_shift.y);
	}
	else if(vbe_disp_oled_orbit_mode == _VBE_PANEL_ORBIT_STORE_MODE){
		//vbe_disp_orbit_set_position_overscan(orbit_shift.x, orbit_shift.y);
	}
	else if(vbe_disp_oled_orbit_mode == _VBE_PANEL_ORBIT_JUSTSCAN_MODE){
		vbe_disp_orbit_justscan_HW_Ctrl(orbit_shift.x, orbit_shift.y,0,TRUE);
	}
	else{
		//not support type
		rtd_pr_vbe_notice("\n orbit do nothing");
	}
	 rtd_pr_vbe_notice("\n (tableIdx=%d)orbit_pixel_shift = %d, orbit_line_shift=%d, mode=(%x) \n",tableIdx, orbit_shift.x, orbit_shift.y, vbe_disp_oled_orbit_mode);
}


void vbe_disp_orbit_overscan_HW_Ctrl(int shitf_x, int shift_y, int range_x, int range_y, unsigned char bEnable_DB)
{
	//ppoverlay_uzudtg_control3_RBUS ppoverlay_uzudtg_control3_reg;
	ppoverlay_uzudtg_orbit_main_ctrl0_RBUS ppoverlay_uzudtg_orbit_main_ctrl0_reg;
	ppoverlay_uzudtg_orbit_main_ctrl2_RBUS ppoverlay_uzudtg_orbit_main_ctrl2_reg;
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_uzudtg_orbit_debug_ctrl0_RBUS ppoverlay_uzudtg_orbit_debug_ctrl0_reg;

	ppoverlay_uzudtg_orbit_debug_ctrl0_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_orbit_debug_ctrl0_reg);
	ppoverlay_uzudtg_orbit_debug_ctrl0_reg.orbit_ungated_dvs_mode = 3;//Bit[1],1��b1: full_screen / overscan, does not latch by dvs
	IoReg_Write32(PPOVERLAY_uzudtg_orbit_debug_ctrl0_reg,ppoverlay_uzudtg_orbit_debug_ctrl0_reg.regValue);

	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	if(bEnable_DB)
	{
		double_buffer_ctrl_reg.orbit_dbuf_en = 1;// enable orbit double buffer
		double_buffer_ctrl_reg.orbit_dbuf_link = 1;// link orbit double buffer to smt
	}
	else
	{
		double_buffer_ctrl_reg.orbit_dbuf_en = 0;//
		double_buffer_ctrl_reg.orbit_dbuf_link = 0;//
	}

	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg,double_buffer_ctrl_reg.regValue);
/*  //timing align should be set in scaler flow
	ppoverlay_uzudtg_control3_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control3_reg);
	ppoverlay_uzudtg_control3_reg.align_output_timing = 1;// memc bypass need to set 1
	IoReg_Write32(PPOVERLAY_uzudtg_control3_reg,ppoverlay_uzudtg_control3_reg.regValue);
*/
	ppoverlay_uzudtg_orbit_main_ctrl0_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_orbit_main_ctrl0_reg);
	scaler_disp_dtg_set_s1ip_dleay(bEnable_DB);

	if((range_x == 0) || (range_y == 0))
	{//overscan off

		ppoverlay_uzudtg_orbit_main_ctrl0_reg.orbit_m_overscan_en = 0;// disable overscan
		//ppoverlay_uzudtg_orbit_main_ctrl0_reg.orbit_mh_extend_max = 0;//
		//ppoverlay_uzudtg_orbit_main_ctrl0_reg.orbit_mv_extend_max = 0;//
		IoReg_Write32(PPOVERLAY_uzudtg_orbit_main_ctrl0_reg,ppoverlay_uzudtg_orbit_main_ctrl0_reg.regValue);

		ppoverlay_uzudtg_orbit_main_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_orbit_main_ctrl2_reg);
		ppoverlay_uzudtg_orbit_main_ctrl2_reg.orbit_mh_shift_dir = 0;
		ppoverlay_uzudtg_orbit_main_ctrl2_reg.orbit_mh_shift = 0;
		ppoverlay_uzudtg_orbit_main_ctrl2_reg.orbit_mv_shift_dir = 0;
		ppoverlay_uzudtg_orbit_main_ctrl2_reg.orbit_mv_shift = 0;
		IoReg_Write32(PPOVERLAY_uzudtg_orbit_main_ctrl2_reg,ppoverlay_uzudtg_orbit_main_ctrl2_reg.regValue);

		double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		double_buffer_ctrl_reg.orbit_dbuf_set = 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg,double_buffer_ctrl_reg.regValue);
		rtd_pr_vbe_notice("[vbe_disp_orbit_overscan_HW_Ctrl] zero range range_x=%d, range_y=%d\n",range_x,range_y);

		return;
	}

	ppoverlay_uzudtg_orbit_main_ctrl0_reg.orbit_m_overscan_en = 1;//
	ppoverlay_uzudtg_orbit_main_ctrl0_reg.orbit_mh_extend_max = range_x;//set as 2N,
	ppoverlay_uzudtg_orbit_main_ctrl0_reg.orbit_mv_extend_max = range_y;//
	IoReg_Write32(PPOVERLAY_uzudtg_orbit_main_ctrl0_reg,ppoverlay_uzudtg_orbit_main_ctrl0_reg.regValue);

	ppoverlay_uzudtg_orbit_main_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_orbit_main_ctrl2_reg);
	if(shitf_x<=0)
	{
		ppoverlay_uzudtg_orbit_main_ctrl2_reg.orbit_mh_shift_dir = 1;
		ppoverlay_uzudtg_orbit_main_ctrl2_reg.orbit_mh_shift = -1*shitf_x;
	}
	else
	{
		ppoverlay_uzudtg_orbit_main_ctrl2_reg.orbit_mh_shift_dir = 0;
		ppoverlay_uzudtg_orbit_main_ctrl2_reg.orbit_mh_shift = shitf_x;
	}

	if(shift_y<=0)
	{
		ppoverlay_uzudtg_orbit_main_ctrl2_reg.orbit_mv_shift_dir = 1;
		ppoverlay_uzudtg_orbit_main_ctrl2_reg.orbit_mv_shift = -1*shift_y;
	}
	else
	{
		ppoverlay_uzudtg_orbit_main_ctrl2_reg.orbit_mv_shift_dir = 0;
		ppoverlay_uzudtg_orbit_main_ctrl2_reg.orbit_mv_shift = shift_y;
	}

	IoReg_Write32(PPOVERLAY_uzudtg_orbit_main_ctrl2_reg,ppoverlay_uzudtg_orbit_main_ctrl2_reg.regValue);
	if(bEnable_DB)
	{
	    double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		double_buffer_ctrl_reg.orbit_dbuf_set = 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg,double_buffer_ctrl_reg.regValue);
	}

	rtd_pr_vbe_notice("[vbe_disp_orbit_overscan_HW_Ctrl] position shift_x=%d, shift_y=%d\n",shitf_x,shift_y);
	rtd_pr_vbe_notice("[vbe_disp_orbit_overscan_HW_Ctrl] overscan range_x=%d, range_y=%d\n",range_x,range_y);

	return;
}

void vbe_disp_orbit_justscan_HW_Ctrl(int shift_x, int shift_y, unsigned char justscan_position, unsigned char bEnable_DB)
{
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl2_reg;
	ppoverlay_display_timing_ctrl2_RBUS ppoverlay_display_timing_ctrl2_reg;
	ppoverlay_memcdtg_control_4_RBUS	ppoverlay_memcdtg_control_4_reg;
	ppoverlay_orbit_bg_color2_RBUS ppoverlay_orbit_bg_color2_reg;
	ppoverlay_sram_ok_main_result_even_RBUS ppoverlay_sram_ok_main_result_even_reg;
	ppoverlay_uzudtg_control1_RBUS ppoverlay_uzudtg_control1_reg;

	ppoverlay_uzudtg_control1_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control1_reg);

	//select justscan hw position
	ppoverlay_orbit_bg_color2_reg.regValue = IoReg_Read32(PPOVERLAY_ORBIT_BG_COLOR2_reg);
	ppoverlay_orbit_bg_color2_reg.orbit_before_osd = justscan_position;//check osd justscan to select
	IoReg_Write32(PPOVERLAY_ORBIT_BG_COLOR2_reg, ppoverlay_orbit_bg_color2_reg.regValue);


	//set orbit timing
	ppoverlay_display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	if(shift_x ==0 && shift_y ==0)
		ppoverlay_display_timing_ctrl2_reg.orbit_timing_en = 0;
	else
		ppoverlay_display_timing_ctrl2_reg.orbit_timing_en = 1;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, ppoverlay_display_timing_ctrl2_reg.regValue);

	//double buffer
	if(bEnable_DB)
	{
		double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
	}
	//set pixel & line shift
	ppoverlay_memcdtg_control_4_reg.regValue = IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_4_reg);
	ppoverlay_sram_ok_main_result_even_reg.regValue = IoReg_Read32(PPOVERLAY_SRAM_OK_main_Result_even_reg);
	if(((is_4k_hfr_mode(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)))||force_enable_two_step_uzu() || get_hdmi_vrr_4k60_mode() || (ppoverlay_uzudtg_control1_reg.uzu_4k_120_mode == 1)) && (Get_DISPLAY_REFRESH_RATE() == 120)/* && justscan_position ==1*/)
	{
		if(shift_x <= 0)
		{
			if((shift_x%2) ==0)
			{
				ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft = ORBIT_JUSTSCAN_4K120_DELAY - (-1*shift_x);
				ppoverlay_sram_ok_main_result_even_reg.dtg_justscan_manual_mode = 0;
				ppoverlay_sram_ok_main_result_even_reg.dtg_justscan_manual_phase = 0;
			}
			else
			{
				ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft = ORBIT_JUSTSCAN_4K120_DELAY - (-1*shift_x) + 2;//odd need to add 2
				ppoverlay_sram_ok_main_result_even_reg.dtg_justscan_manual_mode = 1;
				ppoverlay_sram_ok_main_result_even_reg.dtg_justscan_manual_phase = 0;
			}
		}
		else
		{

			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft = ORBIT_JUSTSCAN_4K120_DELAY + shift_x;
			if((shift_x%2) ==0)
			{
				ppoverlay_sram_ok_main_result_even_reg.dtg_justscan_manual_mode = 0;
				ppoverlay_sram_ok_main_result_even_reg.dtg_justscan_manual_phase = 0;
			}
			else
			{
				ppoverlay_sram_ok_main_result_even_reg.dtg_justscan_manual_mode = 1;
				ppoverlay_sram_ok_main_result_even_reg.dtg_justscan_manual_phase = 1;
			}
		}
		ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft |= _BIT13;	//sign bit
		if(shift_y >=0)
		{
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_line_sft = shift_y;
		}
		else
		{
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_line_sft = (-1*shift_y);
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_line_sft |= _BIT13;	 //sign bit
		}

		rtd_pr_vbe_emerg("[%s] 4k120 = %d, reg=(%x) \n",__FUNCTION__,ppoverlay_uzudtg_control1_reg.uzu_4k_120_mode,ppoverlay_sram_ok_main_result_even_reg.regValue);
	}
	else
	{
		if(ppoverlay_sram_ok_main_result_even_reg.dtg_justscan_manual_mode
			|| ppoverlay_sram_ok_main_result_even_reg.dtg_justscan_manual_phase)
		{
			ppoverlay_sram_ok_main_result_even_reg.dtg_justscan_manual_mode = 0;
			ppoverlay_sram_ok_main_result_even_reg.dtg_justscan_manual_phase = 0;
		}
		if(shift_x <= 0)
		{
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft = -1*shift_x;
		}
		else
		{
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft = shift_x;
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_pixel_sft |= _BIT13;	//sign bit
		}
		if(shift_y >=0)
		{
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_line_sft = shift_y;
		}
		else
		{
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_line_sft = (-1*shift_y);
			ppoverlay_memcdtg_control_4_reg.dtgm2orbitvs_line_sft |= _BIT13;	 //sign bit
		}
	}
	IoReg_Write32(PPOVERLAY_MEMCDTG_CONTROL_4_reg, ppoverlay_memcdtg_control_4_reg.regValue);
	//need to check hw setting can apply without double buffer
	IoReg_Write32(PPOVERLAY_SRAM_OK_main_Result_even_reg, ppoverlay_sram_ok_main_result_even_reg.regValue);
	if(bEnable_DB)
	{
		double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set = 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
	}

	rtd_pr_vbe_emerg("[%s] orbit_pixel_shift = %d, orbit_line_shift=%d, reg=(%x) \n",__FUNCTION__, shift_x, shift_y, IoReg_Read32(PPOVERLAY_MEMCDTG_CONTROL_4_reg));

	return;
}

#endif

extern unsigned char drvif_scaler_frc2fsync_check_fsync_lock(void);

static irqreturn_t vbe_timer_display_interrupt(int irq, void *dev_id)
{
	//static unsigned int before_time = 0;
	//unsigned int after_time = 0;
	if(TIMER_ISR_get_tc7_int(IoReg_Read32(TIMER_ISR_reg))) {
        //after_time = IoReg_Read32(SCPU_CLK90K_LO_reg)/90;

		if(drvif_scaler_frc2fsync_check_fsync_lock()==TRUE){
			rtd_pr_vbe_notice("fsync ok#\n");
			// disable timer7
			IoReg_Write32(TIMER_TC7CR_reg, 0);
			// write 1 clear
			IoReg_Write32(TIMER_ISR_reg, TIMER_ISR_tc7_int_mask);
			// disable timer7 interrupt
			IoReg_Write32(TIMER_TCICR, TIMER_TCICR_tc7ie_mask|0);
			return IRQ_HANDLED;
		}

        //if(IoReg_Read32(0xb8022288)&_BIT22)
            //rtd_pr_vbe_emerg("#v(%d)#\n", after_time-before_time);
        //before_time = after_time;

		// disable timer7
		IoReg_Write32(TIMER_TC7CR_reg, 0);
		// write 1 clear
		IoReg_Write32(TIMER_ISR_reg, TIMER_ISR_tc7_int_mask);

		rtk_timer_set_target(7, TIMER_CLOCK/4000); // 2ms

		// enable timer7
		IoReg_Write32(TIMER_TC7CR_reg, TIMER_TC7CR_tc7en_mask);

		return IRQ_HANDLED;
    }
    return IRQ_NONE;
}
#endif

extern int rtk_pwm_set_scaler_source( int h_total, int v_total, int ifold, unsigned char bEnableDB);
void vbe_disp_pwm_frequency_update(bool bEnableDB)
{
	ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
	ppoverlay_dv_total_RBUS dv_total_reg;
	unsigned int vtotal;
	unsigned long flags;//for spin_lock_irqsave

	if(vbe_disp_get_adaptivestream_fs_mode() == 1){
		vtotal = scaler_disp_smooth_get_memc_protect_vtotal_By_100();
	}else{
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
		ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_read_sel = 1;  //D7_read_sel
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);
		dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
		vtotal = dv_total_reg.dv_total;
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		vtotal *= 100;
	}


#if IS_ENABLED(CONFIG_RTK_KDRV_PWM)
	rtk_pwm_set_scaler_source(Get_DISP_HORIZONTAL_TOTAL(),vtotal, 2, bEnableDB);
#endif
	rtd_pr_vbe_notice("########vbe_disp_pwm_frequency_update vtotal:%d##########\r\n", vtotal);

}
unsigned int vbe_disp_orbit_get_mode_table_shift_x(void)
{
	return orbit_move_range[vbe_disp_oled_orbit_mode].x;
}

unsigned int vbe_disp_orbit_get_mode_table_shift_y(void)
{
	return orbit_move_range[vbe_disp_oled_orbit_mode].x;
}

#ifndef BUILD_QUICK_SHOW

unsigned int p0_lane_power_value;
unsigned int p1_lane_power_value;
unsigned int p2_lane_power_value;
unsigned int p3_lane_power_value;
void vbe_disp_lane_power_off(void)
{
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_0_28_RBUS pinmux_lvdsphy_lvdsepivby1_phy_ctrl_0_28_reg;

	if(IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_reg)!=0){
		vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_reg);
	}
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_0_28_reg.regValue = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_reg);
	pinmux_lvdsphy_lvdsepivby1_phy_ctrl_0_28_reg.regValue = 0;
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_reg, pinmux_lvdsphy_lvdsepivby1_phy_ctrl_0_28_reg.regValue);

	rtd_pr_vbe_notice("turn off disp lane Power\n");
}
void vbe_disp_lane_power_on(void)
{
	if(vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_S_R == 0){
		if(IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_reg)!=0){
			vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_S_R = IoReg_Read32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_reg);
		}else{
			//error recovery
			vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_S_R = 0xfff;
		}
	}
	IoReg_Write32(PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_reg , vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_S_R);
	rtd_pr_vbe_notice("[vbe_disp_lane_power_on] turn on disp lane Power (lane:%x)\n", vbe_disp_suspend_resume.PINMUX_LVDSPHY_LVDSEPIVBY1_PHY_CTRL_0_28_S_R);
}

void vbe_disp_monitor_timing_change_in_isr(void)
{
	{	//iv2dv check
		static unsigned int reg_old = 0;
		unsigned int reg_new = 0;
		unsigned int checkRegister = PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg;
		unsigned int fsync_status = PPOVERLAY_Display_Timing_CTRL1_get_disp_frc_fsync(IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg));

		if((IoReg_Read32(SYS_REG_SYS_CLKEN1_reg)&_BIT2) && (IoReg_Read32(SYS_REG_SYS_SRST1_reg)&_BIT2)
			&& (IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg)&PPOVERLAY_Display_Timing_CTRL1_disp_en_mask))
		{
			reg_new = IoReg_Read32(checkRegister);
		}
		if((reg_old != reg_new) && fsync_status){
			rtd_pr_vbe_notice("[IV2DV] Reg:%x (%x->%x), 90K:%x\n", checkRegister,reg_old,reg_new,drvif_Get_90k_Lo_clk());
		}
		reg_old = reg_new;
	}

	{
		static unsigned int reg_old = 0;
		unsigned int reg_new = 0;
		static unsigned int iv2pv_old = 0;
		unsigned int iv2pv_new = 0;
		unsigned int iv2pv_diff = 0;
		unsigned int iv2pv_th = 3;
		unsigned int checkRegister = VODMA_VODMA_PVS0_Gen_reg;
		unsigned int fsync_status = PPOVERLAY_Display_Timing_CTRL1_get_disp_frc_fsync(IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg));

		if((IoReg_Read32(SYS_REG_SYS_CLKEN1_reg)&_BIT2) && (IoReg_Read32(SYS_REG_SYS_SRST1_reg)&_BIT2)
			&& (IoReg_Read32(VODMA_VODMA_PVS0_Gen_reg)&VODMA_VODMA_PVS0_Gen_en_pvgen_mask) &&
			(IoReg_Read32(VODMA_VODMA_CLKGEN_reg)&VODMA_VODMA_CLKGEN_vodma_enclk_mask))
		{
			reg_new = IoReg_Read32(VODMA_VODMA_PVS0_Gen_reg);
			iv2pv_new = VODMA_VODMA_PVS0_Gen_get_iv2pv_dly(IoReg_Read32(VODMA_VODMA_PVS0_Gen_reg));
			if(Scaler_DispGetInputInfo(SLR_INPUT_H_LEN) != 0)
				iv2pv_diff = (ABS(iv2pv_new,iv2pv_old)/Scaler_DispGetInputInfo(SLR_INPUT_H_LEN));
		}
		if((iv2pv_diff > iv2pv_th) && fsync_status){
			rtd_pr_vbe_notice("[IV2PV] Reg:%x (%x->%x), iv2pv_diff:%d, 90K:%x\n", checkRegister,reg_old,reg_new,iv2pv_diff,drvif_Get_90k_Lo_clk());
		}
		reg_old = reg_new;
		iv2pv_old = iv2pv_new;
	}

}

void vbe_disp_debug_framerate_change_msg(void)
{

	UINT32 u32_iFrameRateHz_pre = 0;
	UINT32 u32_iFrameRateHz = 0;
	UINT32 u32_oFrameRateHz_pre = 0;
	UINT32 u32_oFrameRateHz = 0;
	UINT32 u32_iFrameRateTh = (27000000/50 - 27000000/60)/2;
	UINT32 u32_oFrameRateTh = (27000000/100 - 27000000/120)/2;
	UINT32 u32_iFrameRate = 0;
	UINT32 u32_oFrameRate = 0;
	UINT32 u32_timingMode = 0;
	static UINT32 u32_iFrameRate_Pre = 0;
	static UINT32 u32_oFrameRate_Pre = 0;
	static UINT32 u32_timingMode_Pre = 0;
	UINT8 u1_trig_iFrameRate_noMatch;
	UINT8 u1_trig_oFrameRate_noMatch;

    u32_iFrameRate = IoReg_Read32(PPOVERLAY_uzudtg_DVS_cnt_reg);
	u32_oFrameRate = IoReg_Read32(PPOVERLAY_memcdtg_DVS_cnt_reg);
	u32_timingMode = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

	u32_iFrameRateHz_pre    = (u32_iFrameRate_Pre==0) ? 0: (27000000*10/u32_iFrameRate_Pre);
	u32_iFrameRateHz        =  (u32_iFrameRate==0) ? 0: (27000000*10/u32_iFrameRate);
	u32_oFrameRateHz_pre    =  (u32_oFrameRate_Pre==0) ? 0: (27000000*10/u32_oFrameRate_Pre);
	u32_oFrameRateHz        =  (u32_oFrameRate==0) ? 0: (27000000*10/u32_oFrameRate);
	u32_timingMode			=  (u32_timingMode&~(0xE3FFFFFF))>>26;


	u1_trig_iFrameRate_noMatch	= (u32_iFrameRate>u32_iFrameRate_Pre)? ((u32_iFrameRate -u32_iFrameRate_Pre) > u32_iFrameRateTh) : ((u32_iFrameRate_Pre -u32_iFrameRate) > u32_iFrameRateTh);
	u1_trig_oFrameRate_noMatch    = (u32_oFrameRate>u32_oFrameRate_Pre)? ((u32_oFrameRate -u32_oFrameRate_Pre) > u32_oFrameRateTh) : ((u32_oFrameRate_Pre -u32_oFrameRate) > u32_oFrameRateTh);

	if(u1_trig_iFrameRate_noMatch )
		rtd_pr_vbe_notice("[FR] in FrameRate Change!!!(%dHz(%d) -> %dHz(%d)),(t:%d)\n\r", u32_iFrameRateHz_pre, u32_iFrameRate_Pre, u32_iFrameRateHz, u32_iFrameRate, rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
	//if(u1_trig_oFrameRate_noMatch  && u1_IOPhase_N2M_logEn)
	if(u1_trig_oFrameRate_noMatch )
		rtd_pr_vbe_notice("[FR] out FrameRate Change!!!(%dHz(%d) -> %dHz(%d)),(t:%d)\n\r", u32_oFrameRateHz_pre, u32_oFrameRate_Pre, u32_oFrameRateHz, u32_oFrameRate, rtd_inl(TIMER_SCPU_CLK90K_LO_reg));

	if(u32_timingMode_Pre != u32_timingMode){
		rtd_pr_vbe_notice("[FR] Time Mode changed!!!(%d -> %d),(t:%d)\n\r", u32_timingMode_Pre, u32_timingMode, rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
	}

	if((Get_DISPLAY_PANEL_OLED_TYPE() == TRUE) && (Get_DISPLAY_REFRESH_RATE() == 120) && (u32_oFrameRate < OLED_DISP_FRAMERATE_LIMIT_120HZ))
		rtd_pr_memc_notice("[FR] out FrameRate Over Range!!!(%dHz(%d) -> %dHz(%d))(90K = %d)\n\r", u32_oFrameRateHz_pre, u32_oFrameRate_Pre, u32_oFrameRateHz, u32_oFrameRate, rtd_inl(0xB801B6B8));

	u32_iFrameRate_Pre      = u32_iFrameRate;
	u32_oFrameRate_Pre      = u32_oFrameRate;
	u32_timingMode_Pre		= u32_timingMode;
	//**************************//

}

#define FLAG_IRQ_NUM 16

void vbe_disp_dynamic_polarity_control_init(void)
{
	sfg_pat_det_window_h_ini_sta_width_RBUS sfg_pat_det_window_h_ini_sta_width_reg;
	sfg_pat_det_window_v_ini_sta_height_RBUS sfg_pat_det_window_v_ini_sta_height_reg;
	sfg_pat_det_ctrl_1_RBUS sfg_pat_det_ctrl_1_reg;
	sfg_pat_det_ctrl_2_RBUS sfg_pat_det_ctrl_2_reg;
	sfg_pat_det_ctrl_3_RBUS sfg_pat_det_ctrl_3_reg;
	sfg_pat_det_ctrl_4_RBUS sfg_pat_det_ctrl_4_reg;
	sfg_pat_det_ctrl_6_RBUS sfg_pat_det_ctrl_6_reg;

	if(Get_DISPLAY_REFRESH_RATE()==60){
		sfg_pat_det_window_h_ini_sta_width_reg.pat_det_h_ini_sta = 0;
		sfg_pat_det_window_h_ini_sta_width_reg.pat_det_width = 0xF00;
		sfg_pat_det_window_v_ini_sta_height_reg.pat_det_v_ini_sta = 0;
		sfg_pat_det_window_v_ini_sta_height_reg.pat_det_height = 0x870;
		sfg_pat_det_ctrl_1_reg.pxl_diff_12 = 0x1ff;
		sfg_pat_det_ctrl_1_reg.pxl_diff_sub_12 = 0x3f;
		sfg_pat_det_ctrl_2_reg.pxl_diff_13 = 0x1ff;
		sfg_pat_det_ctrl_2_reg.pxl_diff_sub_13 = 0x3f;
		sfg_pat_det_ctrl_3_reg.vline_num_thresh_12 = 0x214;
		sfg_pat_det_ctrl_3_reg.h_num_thresh_12 = 0x3BC;
		sfg_pat_det_ctrl_4_reg.vline_num_thresh_13 = 0x214;
		sfg_pat_det_ctrl_4_reg.h_num_thresh_13 = 0x3BC;
		sfg_pat_det_ctrl_6_reg.vsign_num_thresh_12 = 0x85;
		sfg_pat_det_ctrl_6_reg.vsign_num_thresh_13 = 0x85;
	}else{
		sfg_pat_det_window_h_ini_sta_width_reg.pat_det_h_ini_sta = 0;
		sfg_pat_det_window_h_ini_sta_width_reg.pat_det_width = 0x780;
		sfg_pat_det_window_v_ini_sta_height_reg.pat_det_v_ini_sta = 0;
		sfg_pat_det_window_v_ini_sta_height_reg.pat_det_height = 0x870;
		sfg_pat_det_ctrl_1_reg.pxl_diff_12 = 0x1ff;
		sfg_pat_det_ctrl_1_reg.pxl_diff_sub_12 = 0x3f;
		sfg_pat_det_ctrl_2_reg.pxl_diff_13 = 0x1ff;
		sfg_pat_det_ctrl_2_reg.pxl_diff_sub_13 = 0x3f;
		sfg_pat_det_ctrl_3_reg.vline_num_thresh_12 = 0x4B0;
		sfg_pat_det_ctrl_3_reg.h_num_thresh_12 = 0xB40;
		sfg_pat_det_ctrl_4_reg.vline_num_thresh_13 = 0x4B0;
		sfg_pat_det_ctrl_4_reg.h_num_thresh_13 = 0xB40;
		sfg_pat_det_ctrl_6_reg.vsign_num_thresh_12 = 0x85;
		sfg_pat_det_ctrl_6_reg.vsign_num_thresh_13 = 0x85;
	}

	IoReg_Write32(SFG_Pat_det_window_H_ini_sta_width_reg,sfg_pat_det_window_h_ini_sta_width_reg.regValue);
	IoReg_Write32(SFG_Pat_det_window_V_ini_sta_height_reg,sfg_pat_det_window_v_ini_sta_height_reg.regValue);
	IoReg_Write32(SFG_Pat_det_ctrl_1_reg,sfg_pat_det_ctrl_1_reg.regValue);
	IoReg_Write32(SFG_Pat_det_ctrl_2_reg,sfg_pat_det_ctrl_2_reg.regValue);
	IoReg_Write32(SFG_Pat_det_ctrl_3_reg,sfg_pat_det_ctrl_3_reg.regValue);
	IoReg_Write32(SFG_Pat_det_ctrl_4_reg,sfg_pat_det_ctrl_4_reg.regValue);
	IoReg_Write32(SFG_Pat_det_ctrl_6_reg,sfg_pat_det_ctrl_6_reg.regValue);
	vbe_dynamicPolarityControl_initialized_status = TRUE;
}

void vbe_disp_dynamic_polarity_control_set_detect_region(KADP_VIDEO_RECT_T detectRegions)
{
	sfg_pat_det_ctrl_3_RBUS sfg_pat_det_ctrl_3_reg;
	sfg_pat_det_ctrl_4_RBUS sfg_pat_det_ctrl_4_reg;
	sfg_pat_det_ctrl_6_RBUS sfg_pat_det_ctrl_6_reg;
	unsigned int hact, vact;

	/*
	a.	60Hz Panel : LG return special size Hact x Vact (���tporch)
	(1) Hact -4 write in 0xb802DC44[13:0] and 0xb802DC48[13:0]
	(2) Vact -8 write in 0xb802DC44[27:16] and 0xb802DC48[27:16]
	(3) (Vact -8)/4 write in 0xb802DC54[9:0] and 0xb802DC54[25:16]
	b.	120Hz Panel : LG return special size Hact x Vact (���tporch)
	(1) (Hact/2) -4 write in 0xb802DC44[13:0] and 0xb802DC48[13:0]
	(2) Vact -8 write in 0xb802DC44[27:16] and 0xb802DC48[27:16]
	(3) (Vact -8)/4 write in 0xb802DC54[9:0] and 0xb802DC54[25:16]
	*/

	hact = detectRegions.w;
	vact = detectRegions.h;

	sfg_pat_det_ctrl_3_reg.regValue = IoReg_Read32(SFG_Pat_det_ctrl_3_reg);
	sfg_pat_det_ctrl_4_reg.regValue = IoReg_Read32(SFG_Pat_det_ctrl_4_reg);
	sfg_pat_det_ctrl_6_reg.regValue = IoReg_Read32(SFG_Pat_det_ctrl_6_reg);
	if(Get_DISPLAY_REFRESH_RATE()==60){
		sfg_pat_det_ctrl_3_reg.h_num_thresh_12 = sfg_pat_det_ctrl_4_reg.h_num_thresh_13 = hact-4;
		sfg_pat_det_ctrl_3_reg.vline_num_thresh_12 = sfg_pat_det_ctrl_4_reg.vline_num_thresh_13 = vact-8;
		sfg_pat_det_ctrl_6_reg.vsign_num_thresh_12 = sfg_pat_det_ctrl_6_reg.vsign_num_thresh_13 = (vact-8)/4;
	}else{
		sfg_pat_det_ctrl_3_reg.h_num_thresh_12 = sfg_pat_det_ctrl_4_reg.h_num_thresh_13 = hact/2-4;
		sfg_pat_det_ctrl_3_reg.vline_num_thresh_12 = sfg_pat_det_ctrl_4_reg.vline_num_thresh_13 = vact-8;
		sfg_pat_det_ctrl_6_reg.vsign_num_thresh_12 = sfg_pat_det_ctrl_6_reg.vsign_num_thresh_13 = (vact-8)/4;
	}

	IoReg_Write32(SFG_Pat_det_ctrl_3_reg,sfg_pat_det_ctrl_3_reg.regValue);
	IoReg_Write32(SFG_Pat_det_ctrl_4_reg,sfg_pat_det_ctrl_4_reg.regValue);
	IoReg_Write32(SFG_Pat_det_ctrl_6_reg,sfg_pat_det_ctrl_6_reg.regValue);

}

void vbe_disp_dynamic_polarity_control_set_detect_framecounts(unsigned int detectFrames)
{
	vbe_dynamicPolarityControl_detect_frames = detectFrames;
}

unsigned int vbe_disp_dynamic_polarity_control_get_detect_framecounts(void)
{
	if(vbe_dynamicPolarityControl_debug_mode){
		if((IoReg_Read32(VBY1_META_DATA_3_reg)&0xffff)!=0)
			return (IoReg_Read32(VBY1_META_DATA_3_reg)&0xffff);
	}
	return vbe_dynamicPolarityControl_detect_frames;
}

void vbe_disp_dynamic_polarity_control_set_detect_thresholds(unsigned int threshold)
{
	vbe_dynamicPolarityControl_detect_threadhold = threshold;
}

unsigned int vbe_disp_dynamic_polarity_control_get_detect_threshold(void)
{
	return vbe_dynamicPolarityControl_detect_threadhold;
}

void vbe_disp_dynamic_polarity_control_set_detect_enable(unsigned char bEnable)
{
	sfg_pattern_detect_ctrl_RBUS sfg_pattern_detect_ctrl_reg;
	vbe_dynamicPolarityControl_enable  = bEnable;
	if(!vbe_dynamicPolarityControl_initialized_status)
		vbe_disp_dynamic_polarity_control_init();
	sfg_pattern_detect_ctrl_reg.regValue = IoReg_Read32(SFG_Pattern_detect_CTRL_reg);
	sfg_pattern_detect_ctrl_reg.pat_det_en = bEnable;
	IoReg_Write32(SFG_Pattern_detect_CTRL_reg, sfg_pattern_detect_ctrl_reg.regValue);
	if(bEnable ==0)
		vbe_dynamicPolarityControl_check_frame = 0;
}

void vbe_disp_dynamic_polarity_control_enable_debug_mode(unsigned char bEnable)
{
	vbe_dynamicPolarityControl_debug_mode = bEnable;
}

void vbe_disp_dynamic_polarity_control_set_control_packet(unsigned char bEnable)
{
	epi_epi_lane0_ctr1_odata_RBUS  	epi_epi_lane0_ctr1_odata_reg;
	epi_epi_lane1_ctr1_odata_RBUS  	epi_epi_lane1_ctr1_odata_reg;
	epi_epi_lane2_ctr1_odata_RBUS  	epi_epi_lane2_ctr1_odata_reg;
	epi_epi_lane3_ctr1_odata_RBUS  	epi_epi_lane3_ctr1_odata_reg;
	epi_epi_lane4_ctr1_odata_RBUS  	epi_epi_lane4_ctr1_odata_reg;
	epi_epi_lane5_ctr1_odata_RBUS  	epi_epi_lane5_ctr1_odata_reg;
	epi_epi_lane6_ctr1_odata_RBUS  	epi_epi_lane6_ctr1_odata_reg;
	epi_epi_lane7_ctr1_odata_RBUS  	epi_epi_lane7_ctr1_odata_reg;
	epi_epi_lane8_ctr1_odata_RBUS  	epi_epi_lane8_ctr1_odata_reg;
	epi_epi_lane9_ctr1_odata_RBUS  	epi_epi_lane9_ctr1_odata_reg;
	epi_epi_lane10_ctr1_odata_RBUS 	epi_epi_lane10_ctr1_odata_reg;
	epi_epi_lane11_ctr1_odata_RBUS 	epi_epi_lane11_ctr1_odata_reg;

	epi_epi_lane0_ctr1_odata_reg.regValue  = IoReg_Read32(EPI_EPI_LANE0_CTR1_ODATA_reg );
	epi_epi_lane1_ctr1_odata_reg.regValue  = IoReg_Read32(EPI_EPI_LANE1_CTR1_ODATA_reg );
	epi_epi_lane2_ctr1_odata_reg.regValue  = IoReg_Read32(EPI_EPI_LANE2_CTR1_ODATA_reg );
	epi_epi_lane3_ctr1_odata_reg.regValue  = IoReg_Read32(EPI_EPI_LANE3_CTR1_ODATA_reg );
	epi_epi_lane4_ctr1_odata_reg.regValue  = IoReg_Read32(EPI_EPI_LANE4_CTR1_ODATA_reg );
	epi_epi_lane5_ctr1_odata_reg.regValue  = IoReg_Read32(EPI_EPI_LANE5_CTR1_ODATA_reg );
	epi_epi_lane6_ctr1_odata_reg.regValue  = IoReg_Read32(EPI_EPI_LANE6_CTR1_ODATA_reg );
	epi_epi_lane7_ctr1_odata_reg.regValue  = IoReg_Read32(EPI_EPI_LANE7_CTR1_ODATA_reg );
	epi_epi_lane8_ctr1_odata_reg.regValue  = IoReg_Read32(EPI_EPI_LANE8_CTR1_ODATA_reg );
	epi_epi_lane9_ctr1_odata_reg.regValue  = IoReg_Read32(EPI_EPI_LANE9_CTR1_ODATA_reg );
	epi_epi_lane10_ctr1_odata_reg.regValue = IoReg_Read32(EPI_EPI_LANE10_CTR1_ODATA_reg);
	epi_epi_lane11_ctr1_odata_reg.regValue = IoReg_Read32(EPI_EPI_LANE11_CTR1_ODATA_reg);

	if(bEnable){
		epi_epi_lane0_ctr1_odata_reg.regValue  |= _BIT11;
		epi_epi_lane1_ctr1_odata_reg.regValue  |= _BIT11;
		epi_epi_lane2_ctr1_odata_reg.regValue  |= _BIT11;
		epi_epi_lane3_ctr1_odata_reg.regValue  |= _BIT11;
		epi_epi_lane4_ctr1_odata_reg.regValue  |= _BIT11;
		epi_epi_lane5_ctr1_odata_reg.regValue  |= _BIT11;
		epi_epi_lane6_ctr1_odata_reg.regValue  |= _BIT11;
		epi_epi_lane7_ctr1_odata_reg.regValue  |= _BIT11;
		epi_epi_lane8_ctr1_odata_reg.regValue  |= _BIT11;
		epi_epi_lane9_ctr1_odata_reg.regValue  |= _BIT11;
		epi_epi_lane10_ctr1_odata_reg.regValue |= _BIT11;
		epi_epi_lane11_ctr1_odata_reg.regValue |= _BIT11;
	}else{
		epi_epi_lane0_ctr1_odata_reg.regValue  &= ~_BIT11;
		epi_epi_lane1_ctr1_odata_reg.regValue  &= ~_BIT11;
		epi_epi_lane2_ctr1_odata_reg.regValue  &= ~_BIT11;
		epi_epi_lane3_ctr1_odata_reg.regValue  &= ~_BIT11;
		epi_epi_lane4_ctr1_odata_reg.regValue  &= ~_BIT11;
		epi_epi_lane5_ctr1_odata_reg.regValue  &= ~_BIT11;
		epi_epi_lane6_ctr1_odata_reg.regValue  &= ~_BIT11;
		epi_epi_lane7_ctr1_odata_reg.regValue  &= ~_BIT11;
		epi_epi_lane8_ctr1_odata_reg.regValue  &= ~_BIT11;
		epi_epi_lane9_ctr1_odata_reg.regValue  &= ~_BIT11;
		epi_epi_lane10_ctr1_odata_reg.regValue &= ~_BIT11;
		epi_epi_lane11_ctr1_odata_reg.regValue &= ~_BIT11;
	}
	IoReg_Write32(EPI_EPI_LANE0_CTR1_ODATA_reg ,	epi_epi_lane0_ctr1_odata_reg.regValue );
	IoReg_Write32(EPI_EPI_LANE1_CTR1_ODATA_reg ,	epi_epi_lane1_ctr1_odata_reg.regValue );
	IoReg_Write32(EPI_EPI_LANE2_CTR1_ODATA_reg ,	epi_epi_lane2_ctr1_odata_reg.regValue );
	IoReg_Write32(EPI_EPI_LANE3_CTR1_ODATA_reg ,	epi_epi_lane3_ctr1_odata_reg.regValue );
	IoReg_Write32(EPI_EPI_LANE4_CTR1_ODATA_reg ,	epi_epi_lane4_ctr1_odata_reg.regValue );
	IoReg_Write32(EPI_EPI_LANE5_CTR1_ODATA_reg ,	epi_epi_lane5_ctr1_odata_reg.regValue );
	IoReg_Write32(EPI_EPI_LANE6_CTR1_ODATA_reg ,	epi_epi_lane6_ctr1_odata_reg.regValue );
	IoReg_Write32(EPI_EPI_LANE7_CTR1_ODATA_reg ,	epi_epi_lane7_ctr1_odata_reg.regValue );
	IoReg_Write32(EPI_EPI_LANE8_CTR1_ODATA_reg ,	epi_epi_lane8_ctr1_odata_reg.regValue );
	IoReg_Write32(EPI_EPI_LANE9_CTR1_ODATA_reg ,	epi_epi_lane9_ctr1_odata_reg.regValue );
	IoReg_Write32(EPI_EPI_LANE10_CTR1_ODATA_reg,	epi_epi_lane10_ctr1_odata_reg.regValue);
	IoReg_Write32(EPI_EPI_LANE11_CTR1_ODATA_reg,	epi_epi_lane11_ctr1_odata_reg.regValue);
}

void vbe_disp_dynamic_polarity_control_analyze_pattern(void)
{
	static unsigned char dpc_irq_flag[FLAG_IRQ_NUM];
//	unsigned int detect_max_frame = vbe_disp_dynamic_polarity_control_get_detect_framecounts()*DPC_ONE_RUN_FRAME;
	unsigned int detect_max_frame = vbe_disp_dynamic_polarity_control_get_detect_framecounts();
	unsigned int detect_threashold = vbe_disp_dynamic_polarity_control_get_detect_threshold();
//	unsigned char case_type=0;
	sfg_pat_det_ctrl_5_RBUS sfg_pat_det_ctrl_5_reg;

	if(vbe_dynamicPolarityControl_enable == 0){
		return;
	}

	if(vbe_dynamicPolarityControl_check_frame == 0){
		memset(&dpc_irq_flag, 0, FLAG_IRQ_NUM);
	}

	if(vbe_dynamicPolarityControl_check_frame < detect_max_frame){
		unsigned int i =0;
/******
Frame01 : 0xb802DC4C = 0x1FFFF clear flag  ->  0xb802DC38[0] = 1 enable DPC
 Frame02 : don't care
 �K
�K
�K
FrameN : read 0xb802DC4C flag -> 0xb802DC38[0] = 0 disable DPC

FrameN+1 : 0xb802DC38[0] = 1 enable DPC
FrameN+2 : don't care
 �K
�K
�K
Frame2N : read 0xb802DC4C flag -> 0xb802DC38[0] = 0 disable DPC

Frame2N+1 : 0xb802DC38[0] = 1 enable DPC
�K
�K
�K

******/
		//if((vbe_dynamicPolarityControl_check_frame%DPC_ONE_RUN_FRAME)==0){
		if(vbe_dynamicPolarityControl_check_frame ==0)
		{
			IoReg_SetBits(SFG_Pattern_detect_CTRL_reg, SFG_Pattern_detect_CTRL_pat_det_en_mask);
		}
		if(vbe_dynamicPolarityControl_check_frame == detect_max_frame-1){
			sfg_pat_det_ctrl_5_reg.regValue = IoReg_Read32(SFG_Pat_det_ctrl_5_reg);
			//rtd_pr_vbe_emerg("vbe_dynamicPolarityControl_check_frame=%d \n",vbe_dynamicPolarityControl_check_frame);
			//collect data
			for(i=0; i< FLAG_IRQ_NUM ; i++)
			{
				if((sfg_pat_det_ctrl_5_reg.irq_flag)&(1<<i)){
					dpc_irq_flag[i]++;
					//rtd_pr_vbe_emerg("irg[%d]=%d \n", i ,dpc_irq_flag[i]);
				}
			}
			//clear pendings
			sfg_pat_det_ctrl_5_reg.irq_flag = 0x1FFFF;
			IoReg_Write32(SFG_Pat_det_ctrl_5_reg, sfg_pat_det_ctrl_5_reg.regValue);
			IoReg_ClearBits(SFG_Pattern_detect_CTRL_reg, SFG_Pattern_detect_CTRL_pat_det_en_mask);
		//	IoReg_SetBits(SFG_Pattern_detect_CTRL_reg, SFG_Pattern_detect_CTRL_pat_det_en_mask);
		}
		vbe_dynamicPolarityControl_check_frame++;
		//rtd_pr_vbe_notice("###### frame_count =%d, SFG_Pat_det_ctrl_5_reg=%x \n",vbe_dynamicPolarityControl_check_frame,IoReg_Read32(SFG_Pat_det_ctrl_5_reg));
	}

	//data collecting done
	if(vbe_dynamicPolarityControl_check_frame == detect_max_frame){

		if(vbe_dynamicPolarityControl_debug_mode){
			unsigned int i=0, j=0;
			unsigned int temp =0, temp2=0;
			for(i=0; i< FLAG_IRQ_NUM ; i++)
			{
				//rtd_pr_vbe_emerg("######  irg[%d]=%d \n", i ,dpc_irq_flag[i]);
				//dummy register for debug
				if(i<8){
					temp |= dpc_irq_flag[i]<<(i*4);
					IoReg_Write32(VBY1_META_DATA_0_reg, temp);
				}else{
					j = i%8;
					temp2 |= dpc_irq_flag[i]<<(j*4);
					IoReg_Write32(VBY1_META_DATA_1_reg, temp2);
				}
			}
			rtd_pr_vbe_notice("###### detect_max_frame=%d, result[%x.%x] \n", detect_max_frame, IoReg_Read32(VBY1_META_DATA_0_reg), IoReg_Read32(VBY1_META_DATA_1_reg));
		}
#if 0
		if((dpc_irq_flag[1] >= detect_threashold) && (dpc_irq_flag[0] >= detect_threashold)){
			case_type = 1; //Case1   Flag : [1][0], need PQ
		}
		else if((dpc_irq_flag[3] >= detect_threashold) && (dpc_irq_flag[2] >= detect_threashold)){
			case_type = 3; //Case3   Flag : [3][2]  need PQ
		}
		else if((dpc_irq_flag[6] >= detect_threashold) && (dpc_irq_flag[4] >= detect_threashold)){
			case_type = 5; //Case5   Flag : [6][4] no need PQ
		}
		else if((dpc_irq_flag[7] >= detect_threashold) && (dpc_irq_flag[5] >= detect_threashold)){
			case_type = 6; //Case6   Flag : [7][5] no need PQ
		}
		else if((dpc_irq_flag[6] >= detect_threashold) && (dpc_irq_flag[5] >= detect_threashold)){
			case_type = 7; //Case7   Flag : [6][5] no need PQ
		}
		else if((dpc_irq_flag[7] >= detect_threashold) && (dpc_irq_flag[4] >= detect_threashold)){
			case_type = 8; //Case7   Flag : [7][4] no need PQ
		}
		else if((dpc_irq_flag[9] >= detect_threashold) && (dpc_irq_flag[8] >= detect_threashold)){
			case_type = 9; //Case9   Flag : [9][8] no need PQ
		}
		else if((dpc_irq_flag[11] >= detect_threashold) && (dpc_irq_flag[10] >= detect_threashold)){
			case_type = 11;  //Case9   Flag : 11][10]  need PQ
		}

		if((case_type == 1) || (case_type == 3) || (case_type == 5) || (case_type == 6) ||
			(case_type == 7) || (case_type == 8) || (case_type == 9) || (case_type == 11)){
				vbe_disp_dynamic_polarity_control_set_control_packet(1);
				//test code
				//dummy register for debug
				if(vbe_dynamicPolarityControl_debug_mode)
					IoReg_Write32(VBY1_META_DATA_2_reg, case_type);

		}else{
			vbe_disp_dynamic_polarity_control_set_control_packet(0);
			//test code
			if(vbe_dynamicPolarityControl_debug_mode)
				IoReg_Write32(VBY1_META_DATA_2_reg, 0xff);
		}
#else
		{	//check result
			int i = 0, result=0;

			for(i=0; i<FLAG_IRQ_NUM; i++){
				if(dpc_irq_flag[i]>detect_threashold){
					result = 1;
					break;
				}
			}

			if(result){
				vbe_disp_dynamic_polarity_control_set_control_packet(1);
			}else{
				vbe_disp_dynamic_polarity_control_set_control_packet(0);
			}
		}

#endif
		vbe_dynamicPolarityControl_check_frame = 0;
	}


}
#define _28S_COUNTER_TARGET 28*90000


void vbe_disp_tcon_28s_toggle_set_enable(unsigned char bEnable)
{
	vbe_tcon_28s_toggle_enable = bEnable;
}

unsigned char vbe_disp_tcon_28s_toggle_get_enable(void)
{
	return vbe_tcon_28s_toggle_enable;
}

void vbe_disp_tcon_28s_toggle_reset_timer(void)
{
	vbe_tcon_28s_timer_counter = 0;
}

void vbe_disp_tcon_28s_toggle_init(void)
{
	//TCON18: across line mode1
	IoReg_Write32(TCON_TCON18_HSE_reg,0x044C044C);
	IoReg_Write32(TCON_TCON18_VSE_reg,0x0001FFFF);
	IoReg_Mask32(TCON_TCON18_Ctrl_reg,0x00000080,0x00000120);
	//TCON24:
	IoReg_Write32(TCON_TCON24_HSE_reg,0x044C044C);
	IoReg_Write32(TCON_TCON24_VSE_reg,0x0001FFFF);
	IoReg_Write32(TCON_TCON24_Ctrl_reg,0x000001A0);
	vbe_disp_tcon_28s_toggle_set_enable(1);
}

unsigned char vbe_disp_tcon_28s_toggle_check_timeout(void)
{
	unsigned int diff_time = 0;
	static unsigned int prev_time = 0;
	unsigned int current_time = 0;
	unsigned char result = 0;

	if(vbe_tcon_28s_toggle_enable){
		if(prev_time==0){
			prev_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		}else{
			current_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			if(current_time>= prev_time)
				diff_time = current_time-prev_time;
			else
				diff_time = (0xFFFFFFFF-prev_time)+current_time;
			vbe_tcon_28s_timer_counter += diff_time;
			prev_time = current_time;
		}
		if(vbe_tcon_28s_timer_counter >= _28S_COUNTER_TARGET)
			result = 1;
	}
	return result;
}

void vbe_disp_tcon_28s_toggle_set_state(unsigned char bState)
{
	vbe_tcon_28s_toggle_state = bState;
}

unsigned char vbe_disp_tcon_28s_toggle_get_state(void)
{
	return vbe_tcon_28s_toggle_state;
}

void vbe_disp_tcon_28s_toggle_handle_state(unsigned char bState)
{
	if(bState == DISP_TCON_TOGGLE_STATE_ACTIVE1){
		tcon_tcon18_ctrl_RBUS tcon_tcon18_ctrl_reg;
		tcon_tcon24_ctrl_RBUS tcon_tcon24_ctrl_reg;

		tcon_tcon18_ctrl_reg.regValue = IoReg_Read32(TCON_TCON18_Ctrl_reg);
		tcon_tcon18_ctrl_reg.tcon18_comb_inv = ~tcon_tcon18_ctrl_reg.tcon18_comb_inv;
		IoReg_Write32(TCON_TCON18_Ctrl_reg, tcon_tcon18_ctrl_reg.regValue);

		tcon_tcon24_ctrl_reg.regValue = IoReg_Read32(TCON_TCON24_Ctrl_reg);
		tcon_tcon24_ctrl_reg.tcon24_comb_inv = 0;
		IoReg_Write32(TCON_TCON24_Ctrl_reg, tcon_tcon24_ctrl_reg.regValue);
		rtd_pr_vbe_notice("[TCON_28S] handle_active1\n");
	}
	else if(bState == DISP_TCON_TOGGLE_STATE_ACTIVE2){
		tcon_tcon24_ctrl_RBUS tcon_tcon24_ctrl_reg;
		tcon_tcon24_ctrl_reg.regValue = IoReg_Read32(TCON_TCON24_Ctrl_reg);
		tcon_tcon24_ctrl_reg.tcon24_comb_inv = 1;
		IoReg_Write32(TCON_TCON24_Ctrl_reg, tcon_tcon24_ctrl_reg.regValue);
		//rtd_pr_vbe_notice("[TCON_28S] handle_active2\n");
	}else if(bState == DISP_TCON_TOGGLE_STATE_INIT){
		vbe_disp_tcon_28s_toggle_reset_timer();
		//rtd_pr_vbe_notice("[TCON_28S] handle_init\n");
	}else if(bState == DISP_TCON_TOGGLE_STATE_NONE){
		vbe_disp_tcon_28s_toggle_init();
		vbe_disp_tcon_28s_toggle_reset_timer();
		//rtd_pr_vbe_notice("[TCON_28S] handle_none\n");
	}else{
		vbe_disp_tcon_28s_toggle_reset_timer();
	}
}
#endif 

void vbe_disp_set_vo_tracking_d_panel_type(unsigned char b_paneltype)
{
  vbe_vo_tracking_d_panel_type = b_paneltype;
}

unsigned char vbe_disp_get_vo_tracking_d_panel_type(void)
{
	return vbe_vo_tracking_d_panel_type;
}

static unsigned char b_force_60hz_panel_type = 0;

void vbe_disp_set_force_60hz_panel_type(unsigned char bEnable)
{
	b_force_60hz_panel_type = bEnable;
}

unsigned char vbe_disp_get_force_60hz_panel_type(void)
{
	return b_force_60hz_panel_type;
}

#ifndef BUILD_QUICK_SHOW
void vbe_disp_config_60hz_panel_interface(unsigned char bEnable)
{
	if(bEnable){

		rtd_pr_vbe_notice("=== panel_vby1_4k60_tx_phy ===");
		//---------------------------------------------------------------------------------------------------------------------------------
		//SFG: DH_ST_END = 0x01181018, DTG_HS_Width = 32, DTG_VS_Width = 12
		//---------------------------------------------------------------------------------------------------------------------------------
		IoReg_Write32(0xb802D900, 0x01140040);	// [27]=0:TCON_clk_div=1,[26]=SFG_clk_div=1, path enable, Common fifo use vby1_clk, 30-bit Mode, SFG out disable,
							                // Remove last too short dhs, any to any sub-pixel assign disable,
		IoReg_Write32(0xb802D904, 0x00FFF000);	// PIF_Clock = Dclk, Force to background enable,
		IoReg_Write32(0xb802D910, 0x00000003);	// CRC Enable,
		IoReg_Write32(0xb802D928, 0x00000000);	// PIF Forcre to BG disable,
		IoReg_Write32(0xb802D92C, 0x00000000);	// Line 8n+0/8n+1/8n+2/8n+3 not sub-pixel shift,
		IoReg_Write32(0xb802DBA8, 0x00000000);	// Line 8n+4/8n+5/8n+6/8n+7 not sub-pixel shift,
		IoReg_Write32(0xb802D930, 0x00000000);	// SEG0/SEG1 No front/back dummy pixel,
		IoReg_Write32(0xb802D950, 0x089D002D);	// DV_Den_end,DV_Den_sta, (follow DTG timing)
		IoReg_Write32(0xb802D954, 0x10180118);	// DH_Den_end,DH_Den_sta, (follow DTG timing)
		IoReg_Write32(0xb802D958, 0x057F112F);	// DV_width,Dh_width,Dh_total (follow DTG timing)
		IoReg_Write32(0xb802D964, 0x00000007);	// SFG Normal Mode, 1.5 packet mode disable,1 pixel mode, H-Flip disable, 1-SEG, 8-Port,
		IoReg_Write32(0xb802D968, 0x00000000);	// SEG0/SEG1 Start Address,
		IoReg_Write32(0xb802D988, 0x40000000);	// Bypass SFG disable, Auto config enable, downsample disable, dummy ine disable,
		IoReg_Write32(0xb802D990, 0x00000300);	// Hs_distance, Vsync Start/End,
		IoReg_Write32(0xb802D998, 0x00000000);	// Segement-flip Disable,
		IoReg_Write32(0xb802D99C, 0x0F000000);	// Final line length,
		IoReg_Write32(0xb802D9A0, 0x00000000);	// Residue_pix_split_mode, Final_addr_split_mode,
		IoReg_Write32(0xb802D9A4, 0x01181128);	// hs_den_io_dist, hs_io_dist,
		IoReg_Write32(0xb802D9B8, 0x00000000);	// All PIF Clock disable, Data inverse disable,[31]lvds, [30]epi/cmpi, [29]vby1, [28]vby1OSD, [15]p2p, [14]isp
		IoReg_Write32(0xb802D9E8, 0x00000000);	// SEG0/SEG1 No middle dummy sub-pixel,
		IoReg_Write32(0xb802DA08, 0x81000000);	// Active_pixel_RGB, ext_num use fw mode, unit is sub-pixel,
		IoReg_Write32(0xb802DA1C, 0x00000000);	// dummy_pixel_data is black,
		IoReg_Write32(0xb802DA94, 0x00000000);	// Last_seg_start, last_seg_start_res for line 4n+0/1,
		IoReg_Write32(0xb802DA98, 0x00000000);	// Last_seg_start, last_seg_start_res for line 4n+2/3,
		IoReg_Write32(0xb802DBE0, 0x00000000);	// Last_seg_start, last_seg_start_res for line 4n+4/5,
		IoReg_Write32(0xb802DBE4, 0x00000000);	// Last_seg_start, last_seg_start_res for line 4n+6/7,
		IoReg_Write32(0xb802DBF0, 0x1DF10E00);	// SFG Pattern Gen. Disable, RGB Mode, Width = 480, Hight = 270, pattern reset by hs,
		IoReg_Write32(0xb802DBF4, 0x00FFFFFF);	// SFG Pattern Gen. R/G/B/W Color for Bar0,
		IoReg_Write32(0xb802DBF8, 0x00FFFF00);	// SFG Pattern Gen. R/G/B/W Color for Bar1,
		IoReg_Write32(0xb802DBFC, 0x0000FFFF);	// SFG Pattern Gen. R/G/B/W Color for Bar2,
		IoReg_Write32(0xb802DC00, 0x0000FF00);	// SFG Pattern Gen. R/G/B/W Color for Bar3,
		IoReg_Write32(0xb802DC04, 0x00FF00FF);	// SFG Pattern Gen. R/G/B/W Color for Bar4,
		IoReg_Write32(0xb802DC08, 0x00FF0000);	// SFG Pattern Gen. R/G/B/W Color for Bar5,
		IoReg_Write32(0xb802DC0C, 0x000000FF);	// SFG Pattern Gen. R/G/B/W Color for Bar6,
		IoReg_Write32(0xb802DC10, 0x00000000);	// SFG Pattern Gen. R/G/B/W Color for Bar7,
		IoReg_Write32(0xb802DC94, 0x10000000);	// Aysnc FIFO disble, Async FIFO use Vby1 Mode,
		IoReg_Write32(0xb802DC98, 0x9C0000FF);	// 15~0 Lane_en
		IoReg_Write32(0xb802DC9C, 0x76543210);	// Lane 7~0 pair assign,  2K using lane2~lane14 for LSphy
		IoReg_Write32(0xb802DCA0, 0xFFFFFFFF);	// Lane 15~8 pair assign,  2K using lane2~lane14 for LSphy
		IoReg_Write32(0xb802DCC8, 0x000000FF);	// VIDEO_OSD_SEL 16lane video, phy lane7~lane0 enable
		IoReg_Write32(0xb802DCD0, 0x00000000);	// P/N Swap enable ,
		//rtd_part_outl(0xb802D900, 11, 11, 0x1); // SFG out enable,
		IoReg_SetBits(0xb802D900, _BIT11); // SFG out enable,

		//---------------------------------------------------------------------------------------------------------------------------------
		//LOCKN, HPTDN Pin share
		//---------------------------------------------------------------------------------------------------------------------------------
		IoReg_Mask32(0xb8000818, 0xFFFF0FFF, 0x00002000);	// GPIO_26 pinshare to vby1_htpd_src0,
		IoReg_Mask32(0xb800081C, 0x0FFFFFFF, 0x10000000);	// GPIO_28 pinshare to vby1_lock_src0,
		IoReg_Mask32(0xb8000CBC, 0xFFFFFFC3, 0x00000000);	// VBy1_HPTDN/VBy1_LOCKN use vby1_htpd_src0/vby1_lock_src0,
		IoReg_Mask32(0xb8000824, 0xE7FFFFFF, 0x00000000);	// GPIO_28/GPIO_26 use 3.3V GPI,

		//---------------------------------------------------------------------------------------------------------------------------------
		//VBy1 MAC
		//---------------------------------------------------------------------------------------------------------------------------------
		IoReg_Write32(0xb802D700, 0x00300000);	// VBy1_MAC_CLK = dclk/8,
		IoReg_Write32(0xb802D704, 0x00000004);	// VBy1 4-Byte Mode,
		IoReg_Write32(0xb802D708, 0x00000000);	// 24-bit ctrl_bit,
		IoReg_Write32(0xb802D70C, 0x6AC00000);	// VBy1 disable, PLL for lclk is stable, debounce count for HTPDN/LOCKN is 16-xtal,
							// Send BE before ALN, don't send BS after ALN finish,
		IoReg_Write32(0xb802D714, 0x00000000);	// 3D Flag Disable,
		IoReg_Write32(0xb802D718, 0x00FF0003);	// CRC Continuous Mode Enable for All lane,
		IoReg_Write32(0xb802D730, 0x00000000);	// HTPDN/LOCKN use HW Mode, DE Only Mode Disable,
		IoReg_Write32(0xb802D758, 0xC001FFFF);	  // 8B10B and Scrambler Enable, set 8B10/scrambler initial state,

		IoReg_Write32(0xb802D774, 0x0000005);  //disable,meta_sta;
		IoReg_Write32(0xb802D778, 0x1000009);  //start by HVsync, meta_end;

		//---------------------------------------------------------------------------------------------------------------------------------
		//PIF_APHY
		//---------------------------------------------------------------------------------------------------------------------------------
		IoReg_Write32(0xb8000CC0, 0x00000000);	// Lane Power disable,
		IoReg_Write32(0xb8000C74, 0x80000000);	// GLOBAL_POW_EN(1),
		IoReg_Write32(0xb8000C00, 0x30030020);	// PI_A1(Phase16), PI_A2(Phase16), VBG,
		IoReg_Write32(0xb8000C04, 0x30030020);	// PI_B1(Phase16), PI_B2(Phase16),
		IoReg_Write32(0xb8000C08, 0x03080100);	// PI_AB(Phase0), LDO_1V = 0.988V, DIVB(1),
		IoReg_Write32(0xb8000C0C, 0x8000E4B0);	// Big KVCO, Bypass PI, CKREF_INV(0), PLL_ICP<2:0>, PLL_SC1, PLL_SC2, PLL_SR,
		IoReg_Write32(0xb8000C10, 0x00000000);	// CK1X_INV(0), CK20X_INV(0), MACPLL_CKO_INV(0), PRESCALERDIV_HS(1),
		IoReg_Write32(0xb8000C14, 0x00000001);	// Normal VCO, PLL Test PAD select, PLL_ICP<3>,
		IoReg_Write32(0xb8000C18, 0x00000020);	// TXPLL use VBy1 feedback path, MACPLL_DIVM(4),
		IoReg_Write32(0xb8000C20, 0x00000200);	// TXPLL_EN(1), TXPLL_RSTN(0),
		IoReg_Write32(0xb8000C50, 0x01F48000);	// IBN Current, VCM Level, LDO1V_EN(1), VCM_SHIFT_DOWN(0),
		IoReg_Write32(0xb8000C78, 0x00000E00);	// CKIN_DIVN(16),
		//
		IoReg_Write32(0xb8000D00, 0x66002815);	// Lane0 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D10, 0x66002815);	// Lane1 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D20, 0x66002815);	// Lane2 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D30, 0x66002815);	// Lane3 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D40, 0x66002815);	// Lane4 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D50, 0x66002815);	// Lane5 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D60, 0x66002815);	// Lane6 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D70, 0x66002815);	// Lane7 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D80, 0x77002815);	// Lane8 : Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D90, 0x77002815);	// Lane9 : Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000DA0, 0x77002815);	// Lane10: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000DB0, 0x77002815);	// Lane11: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000DC0, 0x77002815);	// Lane12: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000DD0, 0x77002815);	// Lane13: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000DE0, 0x77002815);	// Lane14: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000DF0, 0x77002815);	// Lane15: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		//
		IoReg_Write32(0xb8000D04, 0x44003006);	// Lane0 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D14, 0x44003006);	// Lane1 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D24, 0x44003006);	// Lane2 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D34, 0x44003006);	// Lane3 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D44, 0x44003006);	// Lane4 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D54, 0x44003006);	// Lane5 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D64, 0x44003006);	// Lane6 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D74, 0x44003006);	// Lane7 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D84, 0x44003006);	// Lane8 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D94, 0x44003006);	// Lane9 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000DA4, 0x44003006);	// Lane10: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000DB4, 0x44003006);	// Lane11: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000DC4, 0x44003006);	// Lane12: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000DD4, 0x44003006);	// Lane13: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000DE4, 0x44003006);	// Lane14: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000DF4, 0x44003006);	// Lane15: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		//
		IoReg_Write32(0xb8000D08, 0x00000009);	// Lane0 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D18, 0x00000009);	// Lane1 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D28, 0x00000009);	// Lane2 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D38, 0x00000009);	// Lane3 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D48, 0x00000009);	// Lane4 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D58, 0x00000009);	// Lane5 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D68, 0x00000009);	// Lane6 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D78, 0x00000009);	// Lane7 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),

		//---------------------------------------------------------------------------------------------------------------------------------
		//Initial Flow
		//--------------------------------------------------------------------------------------------------------------------------------
		// (1) DPLL Power Enable  (As above Script)
		//
		// (2) Wait DPLL stable at least 150us (SW Control)
		//
		// (3) Release DCLK Gating (SW Control)
		//
		// (4) TXPLL power up TXPLL during reset state (As above Script)
		//
		// (5) TXPLL reset release (Below Setting)
		//rtd_part_outl(0xb8000C20, 10, 10, 0x1);
		IoReg_SetBits(0xb8000C20, _BIT10);
		//
		// (6) Wait TXPLL stable at least 150us (SW Control)
		//
		// (7) VBy1 Clock Enable (Below Setting)
		IoReg_Write32(0xb802D9B8, 0x20000000); //[31]lvds, [30]epi/cmpi, [29]vby1, [28]vby1OSD, [15]p2p, [14]isp
		//
		// (8) VBy1 MAC Enable (Below Setting)
		//rtd_part_outl(0xb802D70C, 31, 31, 0x1);
		IoReg_SetBits(0xb802D70C, _BIT31);
		//
		// (9) Enable Async FIFO,
		//rtd_part_outl(0xb802DC94, 31, 31, 0x1);
		IoReg_SetBits(0xb802DC94, _BIT31);
		//
		// (10) APHY Lane Power Enable (Below Setting)
		IoReg_Write32(0xb8000CC0, 0x000000FF);
		//
		// (11) Panel Power Enable (SW Control)

	}else{

		rtd_pr_vbe_notice("=== panel_vby1_4k120_tx_phy ===");
		//---------------------------------------------------------------------------------------------------------------------------------
		//SFG: DH_ST_END = 0x01181018, DTG_HS_Width = 32, DTG_VS_Width = 12
		//---------------------------------------------------------------------------------------------------------------------------------
		IoReg_Write32(0xb802D900, 0x01140040);	// [27]=0:TCON_clk_div=1,[26]=SFG_clk_div=1, path enable, Common fifo use vby1_clk, 30-bit Mode, SFG out disable,
							// Remove last too short dhs, any to any sub-pixel assign disable,
		IoReg_Write32(0xb802D904, 0x00FFFF00);	// PIF_Clock = Dclk, Force to background enable,
		IoReg_Write32(0xb802D910, 0x00000003);	// CRC Enable,
		IoReg_Write32(0xb802D928, 0x00000000);	// PIF Forcre to BG disable,
		IoReg_Write32(0xb802D92C, 0x00000000);	// Line 8n+0/8n+1/8n+2/8n+3 not sub-pixel shift,
		IoReg_Write32(0xb802DBA8, 0x00000000);	// Line 8n+4/8n+5/8n+6/8n+7 not sub-pixel shift,
		IoReg_Write32(0xb802D930, 0x00000000);	// SEG0/SEG1 No front/back dummy pixel,
		IoReg_Write32(0xb802D950, 0x089D002D);	// DV_Den_end,DV_Den_sta, (follow DTG timing)
		IoReg_Write32(0xb802D954, 0x080C008C);	// DH_Den_end,DH_Den_sta, (follow DTG timing)
		IoReg_Write32(0xb802D958, 0x053F0897);	// DV_width,Dh_width,Dh_total (follow DTG timing)
		IoReg_Write32(0xb802D964, 0x0001000F);	// SFG Normal Mode, 1.5 packet mode disable, 2 pixel mode, H-Flip disable, 1-SEG, 16-Port,
		IoReg_Write32(0xb802D968, 0x00000000);	// SEG0/SEG1 Start Address,
		IoReg_Write32(0xb802D988, 0x40000000);	// Bypass SFG disable, Auto config enable, downsample disable, dummy ine disable,
		IoReg_Write32(0xb802D990, 0x00000300);	// Hs_distance, Vsync Start/End,
		IoReg_Write32(0xb802D998, 0x00000000);	// Segement-flip Disable,
		IoReg_Write32(0xb802D99C, 0x07800000);	// Final line length,
		IoReg_Write32(0xb802D9A0, 0x00000000);	// Residue_pix_split_mode, Final_addr_split_mode,
		IoReg_Write32(0xb802D9A4, 0x008C0894);	// hs_den_io_dist, hs_io_dist,
		IoReg_Write32(0xb802D9B8, 0x00000000);	// All PIF Clock disable, Data inverse disable,[31]lvds, [30]epi/cmpi, [29]vby1, [28]vby1OSD, [15]p2p, [14]isp
		IoReg_Write32(0xb802D9E8, 0x00000000);	// SEG0/SEG1 No middle dummy sub-pixel,
		IoReg_Write32(0xb802DA08, 0x81000000);	// Active_pixel_RGB, ext_num use fw mode, unit is sub-pixel,
		IoReg_Write32(0xb802DA1C, 0x00000000);	// dummy_pixel_data is black,
		IoReg_Write32(0xb802DA94, 0x00000000);	// Last_seg_start, last_seg_start_res for line 4n+0/1,
		IoReg_Write32(0xb802DA98, 0x00000000);	// Last_seg_start, last_seg_start_res for line 4n+2/3,
		IoReg_Write32(0xb802DBE0, 0x00000000);	// Last_seg_start, last_seg_start_res for line 4n+4/5,
		IoReg_Write32(0xb802DBE4, 0x00000000);	// Last_seg_start, last_seg_start_res for line 4n+6/7,
		IoReg_Write32(0xb802DBF0, 0x0F010E00);	// SFG Pattern Gen. Disable, RGB Mode, Width = 240, Hight = 270, pattern reset by hs,
		IoReg_Write32(0xb802DBF4, 0x00FFFFFF);	// SFG Pattern Gen. R/G/B/W Color for Bar0,
		IoReg_Write32(0xb802DBF8, 0x00FFFF00);	// SFG Pattern Gen. R/G/B/W Color for Bar1,
		IoReg_Write32(0xb802DBFC, 0x0000FFFF);	// SFG Pattern Gen. R/G/B/W Color for Bar2,
		IoReg_Write32(0xb802DC00, 0x0000FF00);	// SFG Pattern Gen. R/G/B/W Color for Bar3,
		IoReg_Write32(0xb802DC04, 0x00FF00FF);	// SFG Pattern Gen. R/G/B/W Color for Bar4,
		IoReg_Write32(0xb802DC08, 0x00FF0000);	// SFG Pattern Gen. R/G/B/W Color for Bar5,
		IoReg_Write32(0xb802DC0C, 0x000000FF);	// SFG Pattern Gen. R/G/B/W Color for Bar6,
		IoReg_Write32(0xb802DC10, 0x00000000);	// SFG Pattern Gen. R/G/B/W Color for Bar7,
		IoReg_Write32(0xb802DC94, 0x10000000);	// Aysnc FIFO disble, Async FIFO use Vby1 Mode,
		IoReg_Write32(0xb802DC98, 0x9C00FFFF);	// 15~0 Lane_en
		IoReg_Write32(0xb802DC9C, 0x76543210);	// Lane 7~0 pair assign,  (2K using lane2~lane14 for LSphy)
		IoReg_Write32(0xb802DCA0, 0xFEDCBA98);	// Lane 15~8 pair assign, (2K using lane2~lane14 for LSphy)
		IoReg_Write32(0xb802DCC8, 0x0000FFFF);	// VIDEO_OSD_SEL 16lane video, phy lane15~lane0 enable
		IoReg_Write32(0xb802DCD0, 0x00000000);	// P/N Swap enable ,
		//rtd_part_outl(0xb802D900, 11, 11, 0x1); // SFG out enable,
		IoReg_SetBits(0xb802D900, _BIT11); // SFG out enable,

		//---------------------------------------------------------------------------------------------------------------------------------
		//LOCKN, HPTDN Pin share
		//---------------------------------------------------------------------------------------------------------------------------------
		IoReg_Mask32(0xb8000818, 0xFFFF0FFF, 0x00002000);	// GPIO_26 pinshare to vby1_htpd_src0,
		IoReg_Mask32(0xb800081C, 0x0FFFFFFF, 0x10000000);	// GPIO_28 pinshare to vby1_lock_src0,
		IoReg_Mask32(0xb8000CBC, 0xFFFFFFC3, 0x00000000);	// VBy1_HPTDN/VBy1_LOCKN use vby1_htpd_src0/vby1_lock_src0,
		IoReg_Mask32(0xb8000824, 0xE7FFFFFF, 0x00000000);	// GPIO_28/GPIO_26 use 3.3V GPI,

		//---------------------------------------------------------------------------------------------------------------------------------
		//VBy1 MAC
		//---------------------------------------------------------------------------------------------------------------------------------
		IoReg_Write32(0xb802D700, 0x00300000);	// VBy1_MAC_CLK = dclk/8,
		IoReg_Write32(0xb802D704, 0x00000004);	// VBy1 4-Byte Mode,
		IoReg_Write32(0xb802D708, 0x00000000);	// 24-bit ctrl_bit,
		IoReg_Write32(0xb802D70C, 0x6AC00000);	// VBy1 disable, PLL for lclk is stable, debounce count for HTPDN/LOCKN is 16-xtal,
							// Send BE before ALN, don't send BS after ALN finish,
		IoReg_Write32(0xb802D714, 0x00000000);	// 3D Flag Disable,
		IoReg_Write32(0xb802D718, 0x00FF0003);	// CRC Continuous Mode Enable for All lane,
		IoReg_Write32(0xb802D730, 0x00000000);	// HTPDN/LOCKN use HW Mode, DE Only Mode Disable,
		IoReg_Write32(0xb802D758, 0xC001FFFF);	  // 8B10B and Scrambler Enable, set 8B10/scrambler initial state,

		IoReg_Write32(0xb802D774, 0x0000005);  //disable,meta_sta;
		IoReg_Write32(0xb802D778, 0x1000009);  //start by HVsync, meta_end;
		//---------------------------------------------------------------------------------------------------------------------------------
		//PIF_APHY
		//---------------------------------------------------------------------------------------------------------------------------------
		IoReg_Write32(0xb8000CC0, 0x00000000);	// Lane Power disable,
		IoReg_Write32(0xb8000C74, 0x80000000);	// GLOBAL_POW_EN(1),
		IoReg_Write32(0xb8000C00, 0x30030020);	// PI_A1(Phase16), PI_A2(Phase16), VBG,
		IoReg_Write32(0xb8000C04, 0x30030020);	// PI_B1(Phase16), PI_B2(Phase16),
		IoReg_Write32(0xb8000C08, 0x03080100);	// PI_AB(Phase0), LDO_1V = 0.988V, DIVB(1),
		IoReg_Write32(0xb8000C0C, 0x8000E4B0);	// Big KVCO, Bypass PI, CKREF_INV(0), PLL_ICP<2:0>, PLL_SC1, PLL_SC2, PLL_SR,
		IoReg_Write32(0xb8000C10, 0x00000000);	// CK1X_INV(0), CK20X_INV(0), MACPLL_CKO_INV(0), PRESCALERDIV_HS(1),
		IoReg_Write32(0xb8000C14, 0x00000001);	// Normal VCO, PLL Test PAD select, PLL_ICP<3>,
		IoReg_Write32(0xb8000C18, 0x00000020);	// TXPLL use VBy1 feedback path, MACPLL_DIVM(4),
		IoReg_Write32(0xb8000C20, 0x00000200);	// TXPLL_EN(1), TXPLL_RSTN(0),
		IoReg_Write32(0xb8000C50, 0x01F48000);	// IBN Current, VCM Level, LDO1V_EN(1), VCM_SHIFT_DOWN(0),
		IoReg_Write32(0xb8000C78, 0x00000E00);	// CKIN_DIVN(16),
		//
		IoReg_Write32(0xb8000D00, 0x66002815);	// Lane0 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D10, 0x66002815);	// Lane1 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D20, 0x66002815);	// Lane2 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D30, 0x66002815);	// Lane3 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D40, 0x66002815);	// Lane4 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D50, 0x66002815);	// Lane5 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D60, 0x66002815);	// Lane6 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D70, 0x66002815);	// Lane7 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D80, 0x66002815);	// Lane8 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000D90, 0x66002815);	// Lane9 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000DA0, 0x66002815);	// Lane10: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000DB0, 0x66002815);	// Lane11: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000DC0, 0x66002815);	// Lane12: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000DD0, 0x66002815);	// Lane13: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000DE0, 0x66002815);	// Lane14: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		IoReg_Write32(0xb8000DF0, 0x66002815);	// Lane15: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
		//
		IoReg_Write32(0xb8000D04, 0x44003006);	// Lane0 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D14, 0x44003006);	// Lane1 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D24, 0x44003006);	// Lane2 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D34, 0x44003006);	// Lane3 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D44, 0x44003006);	// Lane4 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D54, 0x44003006);	// Lane5 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D64, 0x44003006);	// Lane6 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D74, 0x44003006);	// Lane7 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D84, 0x44003006);	// Lane8 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000D94, 0x44003006);	// Lane9 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000DA4, 0x44003006);	// Lane10: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000DB4, 0x44003006);	// Lane11: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000DC4, 0x44003006);	// Lane12: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000DD4, 0x44003006);	// Lane13: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000DE4, 0x44003006);	// Lane14: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		IoReg_Write32(0xb8000DF4, 0x44003006);	// Lane15: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
		//
		IoReg_Write32(0xb8000D08, 0x00000009);	// Lane0 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D18, 0x00000009);	// Lane1 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D28, 0x00000009);	// Lane2 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D38, 0x00000009);	// Lane3 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D48, 0x00000009);	// Lane4 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D58, 0x00000009);	// Lane5 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D68, 0x00000009);	// Lane6 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D78, 0x00000009);	// Lane7 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D88, 0x00000009);	// Lane8 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000D98, 0x00000009);	// Lane9 : PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000DA8, 0x00000009);	// Lane10: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000DB8, 0x00000009);	// Lane11: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000DC8, 0x00000009);	// Lane12: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000DD8, 0x00000009);	// Lane13: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000DE8, 0x00000009);	// Lane14: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
		IoReg_Write32(0xb8000DF8, 0x00000009);	// Lane15: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),

		//---------------------------------------------------------------------------------------------------------------------------------
		//Initial Flow
		//--------------------------------------------------------------------------------------------------------------------------------
		// (1) DPLL Power Enable  (As above Script)
		//
		// (2) Wait DPLL stable at least 150us (SW Control)
		//
		// (3) Release DCLK Gating (SW Control)
		//
		// (4) TXPLL power up TXPLL during reset state (As above Script)
		//
		// (5) TXPLL reset release (Below Setting)
		//rtd_part_outl(0xb8000C20, 10, 10, 0x1);
		IoReg_SetBits(0xb8000C20, _BIT10);
		//
		// (6) Wait TXPLL stable at least 150us (SW Control)
		//
		// (7) VBy1 Clock Enable (Below Setting)
		IoReg_Write32(0xb802D9B8, 0x20000000); //[31]lvds, [30]epi/cmpi, [29]vby1, [28]vby1OSD, [15]p2p, [14]isp
		//
		// (8) VBy1 MAC Enable (Below Setting)
		//rtd_part_outl(0xb802D70C, 31, 31, 0x1);
		IoReg_SetBits(0xb802D70C, _BIT31);
		//
		// (9) Enable Async FIFO,
		//rtd_part_outl(0xb802DC94, 31, 31, 0x1);
		IoReg_SetBits(0xb802DC94, _BIT31);

		//
		// (10) APHY Lane Power Enable (Below Setting)
		IoReg_Write32(0xb8000CC0, 0x0000FFFF);
		//
		// (11) Panel Power Enable (SW Control)
	}
}
#endif

#define VRR_FR_PROTECT_LOWER_BOUND_1 0xD4A37	// 30 Hz
#define VRR_FR_PROTECT_UPPER_BOUND_1 0x11E999   // 23 Hz
#define VRR_FR_PROTECT_LOWER_BOUND_2 0x6C0FE	// 61 Hz
#define VRR_FR_PROTECT_UPPER_BOUND_2 0x8F4CC    // 46 Hz

#define VRR_FR_FRAC_1_1_LOWER_BOUND 0x6C0FE	   // 61 Hz
#define VRR_FR_FRAC_1_1_UPPER_BOUND 0x8C404   // 47 Hz
#define VRR_FR_FRAC_1_2_LOWER_BOUND 0xD4A37	   // 31 Hz
#define VRR_FR_FRAC_1_2_UPPER_BOUND 0x11E999   // 23 Hz
//#define VRR_FR_FRAC_2_5_LOWER_BOUND 0x11E999   // 23 Hz
//#define VRR_FR_FRAC_2_5_UPPER_BOUND 0xD4A37	   // 30 Hz

unsigned char Freesync_mode_enable = 0;
unsigned char scaler_VRR_timingMode_flag = 0;//record scaler is use vrr or not

unsigned char vbe_disp_get_freesync_mode_flag(void)
{
	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
		||(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP)) {
		return (!get_scaler_qms_mode_flag() && Freesync_mode_enable);//drvif_Hdmi_GetAMDFreeSyncEnable();
	}
	else
		return 0;
}

void vbe_disp_set_freesync_mode_flag (unsigned char enable)
{
	Freesync_mode_enable = enable;

    if(!vbe_get_panel_vrr_freesync_timing_is_valid())
    {
        Freesync_mode_enable = 0;
    }
}

unsigned char vbe_disp_get_VRR_timingMode_flag(void)
{
	if(get_vsc_src_is_hdmi_or_dp()) {
		return (!get_scaler_qms_mode_flag() && scaler_VRR_timingMode_flag);//drvif_Hdmi_GetVRREnable();
	}
	else
		return 0;
}

void Set_Val_VRR_timingMode_flag(unsigned char flag)
{
	scaler_VRR_timingMode_flag = flag;
}

bool is_vrr_or_freesync_mode(void)
{
	return vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag();
}

void vbe_disp_set_VRR_timingMode_flag(unsigned char flag)
{
	scaler_VRR_timingMode_flag = flag;

    if(!vbe_get_panel_vrr_freesync_timing_is_valid())
    {
        scaler_VRR_timingMode_flag = 0;
    }
}

unsigned char pre_VRR_ALLM_flag = 0;

unsigned char vbe_disp_get_pre_VRR_ALLM_flag(void)
{
	return pre_VRR_ALLM_flag;
}

void vbe_disp_set_pre_VRR_ALLM_flag (unsigned char enable)
{
	pre_VRR_ALLM_flag = enable;
}

extern HDMI_bool drvif_Hdmi_GetALLMEnable(void);
unsigned char vbe_disp_get_VRR_ALLM_flag(void)
{
	return  (unsigned char)drvif_Hdmi_GetALLMEnable();
}


unsigned char vbe_disp_check_support_VRR_framerate_range(void)
{
	UINT32 u32_iFrameRate = 0;

	u32_iFrameRate = IoReg_Read32(PPOVERLAY_IVS_cnt_3_reg);

	if(((u32_iFrameRate<=VRR_FR_PROTECT_UPPER_BOUND_1)&&(u32_iFrameRate>=VRR_FR_PROTECT_LOWER_BOUND_1)) ||
		((u32_iFrameRate<=VRR_FR_PROTECT_UPPER_BOUND_2)&&(u32_iFrameRate>=VRR_FR_PROTECT_LOWER_BOUND_2))){
		return TRUE;
	}else{
		return FALSE;
	}
}

VBE_DISP_VRR_FRACTIONAL_TIMING_MODE_T vbe_disp_get_VRR_timing_fractional_mode(void)
{
	UINT32 u32_iFrameRate = 0;

	u32_iFrameRate = IoReg_Read32(PPOVERLAY_IVS_cnt_3_reg);

	if((u32_iFrameRate<=VRR_FR_FRAC_1_1_UPPER_BOUND)&&(u32_iFrameRate>=VRR_FR_FRAC_1_1_LOWER_BOUND)){
		return VBE_DISP_VRR_1_1;
	}else if((u32_iFrameRate<=VRR_FR_FRAC_1_2_UPPER_BOUND)&&(u32_iFrameRate>=VRR_FR_FRAC_1_2_LOWER_BOUND)){
		return VBE_DISP_VRR_1_2;
	}else
		return VBE_DISP_VRR_MAX;

	return VBE_DISP_VRR_MAX;
}

VBE_DISP_VRR_FRACTIONAL_TIMING_MODE_T vbe_disp_get_Freesync_timing_fractional_mode(void)
{
	//UINT32 u32_iFrameRate = 0;

	//u32_iFrameRate = IoReg_Read32(PPOVERLAY_IVS_cnt_3_reg);

#if 1
	return VBE_DISP_VRR_1_1;
#else
	if((u32_iFrameRate<=VRR_FR_FRAC_1_1_UPPER_BOUND)&&(u32_iFrameRate>=VRR_FR_FRAC_1_1_LOWER_BOUND)){
		return VBE_DISP_VRR_1_1;
	}else if((u32_iFrameRate<=VRR_FR_FRAC_1_2_UPPER_BOUND)&&(u32_iFrameRate>=VRR_FR_FRAC_1_2_LOWER_BOUND)){
		return VBE_DISP_VRR_1_2;
	}else
		return VBE_DISP_VRR_MAX;
#endif

	return VBE_DISP_VRR_MAX;
}


UINT32 vbe_disp_get_VRR_timing_real_vtotal(void)
{
	UINT32 vertical_total = 0;
	UINT32 vsyncFreq = 0;
    UINT32 IVScnt = 0;
	VBE_DISP_VRR_FRACTIONAL_TIMING_MODE_T vrr_timingMode=vbe_disp_get_VRR_timing_fractional_mode();

	if(vrr_timingMode != VBE_DISP_VRR_MAX){

		if(IoReg_Read32(PPOVERLAY_IVS_cnt_3_reg) > 0)
			IVScnt = IoReg_Read32(PPOVERLAY_IVS_cnt_3_reg);
		else{
			if(((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 505))
				|| ((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 995) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 1005)))
			{
				rtd_pr_vbe_notice("[%s] ivs is wrong(%d), set default 50Hz to protect vtotal!!!\r", __FUNCTION__, IoReg_Read32(PPOVERLAY_IVS_cnt_3_reg));
				IVScnt = 540000;	//default set to 50Hz
			}
			else
			{
				rtd_pr_vbe_notice("[%s] ivs is wrong(%d), set default 60Hz to protect vtotal!!!\r", __FUNCTION__, IoReg_Read32(PPOVERLAY_IVS_cnt_3_reg));
				IVScnt = 450000;	//default set to 60Hz
			}
		}
		vsyncFreq = 27000000/IVScnt;

		if(vrr_timingMode == VBE_DISP_VRR_1_1){
			vertical_total = Get_DISPLAY_CLOCK_TYPICAL()/(Get_DISP_HORIZONTAL_TOTAL() * vsyncFreq);
		}else if(vrr_timingMode == VBE_DISP_VRR_1_2){
			vertical_total = Get_DISPLAY_CLOCK_TYPICAL()/(Get_DISP_HORIZONTAL_TOTAL() * vsyncFreq * 2);
		}
	}

    if(vertical_total == 0)
    {
    	if(Get_DISPLAY_REFRESH_RATE() == 165){
			if(((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 505))
				|| ((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 995) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 1005)))
			{
				rtd_pr_vbe_notice("[%s] vertical_total is wrong(%d), set default 50Hz to protect vtotal!!!\r", __FUNCTION__, vertical_total);
				vertical_total = 2442;
			}
			else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 1435) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 1445))
			{
				rtd_pr_vbe_notice("[%s] vertical_total is wrong(%d), input is 144Hz, set 144Hz to protect vtotal!!!\r", __FUNCTION__, vertical_total);
				vertical_total = 1695;
			}
			else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 1640) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 1660))
			{
				rtd_pr_vbe_notice("[%s] vertical_total is wrong(%d), input is 165Hz, set 165Hz to protect vtotal!!!\r", __FUNCTION__, vertical_total);
				vertical_total = 1480;
			}
			else
			{
				rtd_pr_vbe_notice("[%s] vertical_total is wrong(%d), set default 60Hz to protect vtotal!!!\r", __FUNCTION__, vertical_total);
				vertical_total = 2035;
			}
    	}
		else{
	        if(((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 495) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 505))
				|| ((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 995) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 1005)))
			{
				rtd_pr_vbe_notice("[%s] vertical_total is wrong(%d), set default 50Hz to protect vtotal!!!\r", __FUNCTION__, vertical_total);
				vertical_total = 2700;
			}
			else if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 1435) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 1445))
			{
				rtd_pr_vbe_notice("[%s] vertical_total is wrong(%d), input is 144Hz, set 144Hz to protect vtotal!!!\r", __FUNCTION__, vertical_total);
				vertical_total = Get_DISP_VERTICAL_TOTAL();
			}
			else if(((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 2390) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 2410))
                || ((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 2878) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 2898)))
			{
				rtd_pr_vbe_notice("[%s] vertical_total is wrong(%d), input is 240Hz, set 240Hz to protect vtotal!!!\r", __FUNCTION__, vertical_total);
				vertical_total = Get_DISP_VERTICAL_TOTAL();
			}
			else
			{
				rtd_pr_vbe_notice("[%s] vertical_total is wrong(%d), set default 60Hz to protect vtotal!!!\r", __FUNCTION__, vertical_total);
				vertical_total = 2250;
			}
		}
    }

	return vertical_total;
}

UINT32 vbe_disp_get_VRR_device_max_framerate(void)
{
	UINT32 max_support_freq = 0;
#if 0
	UINT32 u32_htotal = Scaler_DispGetInputInfoByDisp(0, SLR_INPUT_H_LEN);
	//UINT32 u32_vtotal = Scaler_DispGetInputInfoByDisp(0, SLR_INPUT_V_LEN);
	UINT32 u32_vact = Scaler_DispGetInputInfoByDisp(0, SLR_INPUT_IPV_ACT_LEN);	//vtoal of ofms is not fix during vrr, so use act
	UINT32 u32_pixelclock = 0;
	if(drvif_Hdmi_GetPixelClockKHz(&u32_pixelclock)){
		rtd_pr_vbe_notice("[%s]clock:%d\n", __FUNCTION__,u32_pixelclock);
		if((u32_htotal!=0) && (u32_vact!=0)){
			max_support_freq = (u32_pixelclock*1000)/((u32_htotal/10)*u32_vact);
			rtd_pr_vbe_notice("[%s]max_support_freq:%d\n", __FUNCTION__,max_support_freq);
		}
	}else{
		rtd_pr_vbe_notice("[ERROR][%s]drvif_Hdmi_GetPixelClockKHz return false\n", __FUNCTION__);
	}
#else
	max_support_freq=Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ);
	rtd_pr_vbe_notice("[%s]max_support_freq:%d\n", __FUNCTION__,max_support_freq);
#endif
	return max_support_freq;
}

#ifndef BUILD_QUICK_SHOW
void vbe_disp_VRR_framerate_protect_check(void)
{
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	UINT8  b_enterProtect = 0;

	display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);

	if(display_timing_ctrl1_reg.disp_fsync_en == 0){
		//no need to check
		return;
	}

	if(vbe_disp_check_support_VRR_framerate_range()){
		b_enterProtect = 0;
	}else{
		rtd_pr_vbe_notice(" %dHz(%d)\n\r", (270000000/IoReg_Read32(PPOVERLAY_IVS_cnt_3_reg)), rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
		b_enterProtect = 1;
	}
	if(b_enterProtect == 1){
		drivf_scaler_reset_freerun();
		//HAL_MC_SetMCBlendMode(0);
	}else{
		//HAL_MC_SetMCBlendMode(2);
	}
}
#endif

unsigned char vbe_disp_set_VRR_fractional_framerate_mode(void)
{
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl2_reg;
//#ifndef CONFIG_MEMC_BYPASS
	ppoverlay_uzudtg_fractional_fsync_RBUS uzudtg_fractional_fsync_reg;
//#endif
	ppoverlay_dtg_m_remove_input_vsync_RBUS dtg_m_remove_input_vsync_reg;
	ppoverlay_dtg_m_multiple_vsync_RBUS dtg_m_multiple_vsync_reg;
	ppoverlay_dispd_stage1_sm_ctrl_RBUS dispd_stage1_sm_ctrl_reg;
	VBE_DISP_VRR_FRACTIONAL_TIMING_MODE_T vrr_frac_mode = VBE_DISP_VRR_1_1;
	unsigned long flags;//for spin_lock_irqsave
	unsigned char multiple_ratio=0, remove_ratio=0;
//#ifndef CONFIG_MEMC_BYPASS
	UINT32 dclk_fract_a, dclk_fract_b;
	sys_reg_sys_dispclksel_RBUS sys_dispclksel_reg;
//#endif
	//ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
	unsigned char uzu_valid_framecnt = 0;
	ppoverlay_dtg_frame_cnt1_RBUS dtg_frame_cnt1_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS	 dispd_smooth_toggle1_reg;
	unsigned int count=0x3fffff;
	UINT32 mul_dbuf_cnt=0;

#if 0	//Eric@20181228 VRR always go 1:1 mode
	if(vbe_disp_get_freesync_mode_flag() || vbe_disp_get_VRR_timingMode_flag())
		vrr_frac_mode = VBE_DISP_VRR_1_1;
	else
		vrr_frac_mode = vbe_disp_get_VRR_timing_fractional_mode();

	rtd_pr_vbe_emerg("%s, set vrr_frac_mode mode = %d", __func__, vrr_frac_mode);

	if (vrr_frac_mode>=VBE_DISP_VRR_MAX) {
		rtd_pr_vbe_info("vrr_frac_mode error (%d)\n", vrr_frac_mode);
		return FALSE;
	}

	if(vrr_frac_mode == VBE_DISP_VRR_1_1){
		multiple_ratio = 1;
		remove_ratio = 1;
	}else if(vrr_frac_mode == VBE_DISP_VRR_1_2){
		multiple_ratio = 2;
		remove_ratio = 1;
	}else if(vrr_frac_mode == VBE_DISP_VRR_2_5){
		multiple_ratio = 5;
		remove_ratio = 2;
	}
#else
    rtd_pr_vbe_info("%s, set vrr_frac_mode mode = %d", __func__, vrr_frac_mode);
    multiple_ratio = 1;
    remove_ratio = 1;
#endif

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	//double buffer D0
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.dreg_dbuf_en = 0;
	double_buffer_ctrl_reg.drm_multireg_dbuf_en = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_read_sel = 1;
	double_buffer_ctrl_reg.drm_multireg_dbuf_vsync_sel = 1;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);

	//pre-flow. patch, let D8 double buffer works well
	count = 0x3fffff;
	while((count == 0x3fffff)&&(mul_dbuf_cnt<10000)){
		//msleep(10);
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
			rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_set 1 wait timeout !!!\n");
		}
		if(double_buffer_ctrl_reg.drm_multireg_dbuf_en==0)
			rtd_pr_vbe_emerg("[double buffer] drm_multireg_dbuf_en=0 1!!\n");

		mul_dbuf_cnt++;
	}

	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=0;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
	IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);

	if(get_MEMC_bypass_status_refer_platform_model() == FALSE){
	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzu_valid_framecnt = uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync;	//if 2:5, valid_framecnt = 1, if 1:2, valid_framecnt = 0
	//wait to front porch
	count = 0x3fffff;
    dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
	while(!((dtg_frame_cnt1_reg.uzudtg_output_frame_cnt == uzu_valid_framecnt) &&
		(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)) > 100) &&
		(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)) < 1900)) && --count){
		dtg_frame_cnt1_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg);
	}
	if(count == 0)
		rtd_pr_vbe_emerg("[DTG] Wait to uzu_valid_framecnt timeout!!, uzu_valid_framecnt = %d, uzudtg_output_frame_cnt = %d\n",uzu_valid_framecnt, dtg_frame_cnt1_reg.uzudtg_output_frame_cnt);
	}
	dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
	dtg_m_remove_input_vsync_reg.remove_half_ivs = 0; //mode2
	dtg_m_remove_input_vsync_reg.remove_half_ivs_mode2 = (remove_ratio-1);
	IoReg_Write32(PPOVERLAY_DTG_M_Remove_input_vsync_reg, dtg_m_remove_input_vsync_reg.regValue);

	dtg_m_multiple_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_multiple_vsync_reg);
	dtg_m_multiple_vsync_reg.dtg_m_multiple_vsync = (multiple_ratio-1);
	IoReg_Write32(PPOVERLAY_DTG_M_multiple_vsync_reg, dtg_m_multiple_vsync_reg.regValue);

	//======= Set UZU remove/multi
//#ifdef CONFIG_MEMC_BYPASS
	if(get_MEMC_bypass_status_refer_platform_model() == TRUE){
	// don't set uzudtg fractional fs.
		rtd_pr_vbe_info("MEMC bypass !! Don't set uzudtg fractional fs. \n");
	}else{
//#else

	uzudtg_fractional_fsync_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg);
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs = 1; // enable;
#ifdef SMOOTH_TOGGLE_DATAFRCFS_TEST
	//run timing frame sync should align IVS @Crixus 20160202
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_alignmode = 1; // align to ivs
#endif
	uzudtg_fractional_fsync_reg.uzudtg_remove_half_ivs_mode = (multiple_ratio-1);
	uzudtg_fractional_fsync_reg.uzudtg_multiple_vsync = (remove_ratio-1);
	IoReg_Write32(PPOVERLAY_uzudtg_fractional_fsync_reg, uzudtg_fractional_fsync_reg.regValue);
	rtd_pr_vbe_debug("uzudtg_fractional_fsync_reg : %x\n", uzudtg_fractional_fsync_reg.regValue);
//#endif
	}
//#ifndef CONFIG_MEMC_BYPASS
	if(get_MEMC_bypass_status_refer_platform_model() == FALSE){
//====== Set UZU clock
	sys_dispclksel_reg.regValue = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	dclk_fract_a = remove_ratio-1;
	dclk_fract_b = multiple_ratio-1;

	sys_dispclksel_reg.dclk_fract_en = 1;
	sys_dispclksel_reg.dclk_fract_a = dclk_fract_a;
	sys_dispclksel_reg.dclk_fract_b = (dclk_fract_b&0x1f);
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, sys_dispclksel_reg.regValue);
//=======
	}
//#endif

	count = 0x3fffff;
	dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
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
	double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
	double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
	double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
	IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
	modestate_set_display_timing_genByDisp(SLR_MAIN_DISPLAY);//set dvtotal

	if((vbe_disp_get_VRR_device_max_framerate()< 595) && (vbe_disp_get_VRR_device_max_framerate()> 605)){ // 120hz
		// update sfg for vrr
		vbe_disp_set_vrr_average_den_mode(1);
		vbe_disp_set_vrr_average_den_update(1);
	}

	return TRUE;
}

static unsigned int u32_vbe_disp_sfg_htotal = 0;
static unsigned int u32_vbe_disp_sfg_pixel_mode = 0xff;
static unsigned int u32_vbe_disp_vrr_average_den_update = 0;
static unsigned int u32_vbe_disp_vrr_average_den_mode = 0;

void vbe_disp_set_sfg_dh_total(unsigned int u32_htotal)
{
	u32_vbe_disp_sfg_htotal = u32_htotal;
}

unsigned int vbe_disp_get_sfg_dh_total(void)
{
	if(u32_vbe_disp_sfg_htotal == 0){
		vbe_disp_set_sfg_dh_total(SFG_SFG_CTRL_26_get_dh_total(IoReg_Read32(SFG_SFG_CTRL_26_reg)));
		rtd_pr_vbe_notice("sfg_dhtotal=0, save current value.\n");
	}
	return u32_vbe_disp_sfg_htotal;
}

void vbe_disp_set_sfg_pixel_mode(unsigned int u32_pixelMode)
{
	u32_vbe_disp_sfg_pixel_mode = u32_pixelMode;
}

unsigned int vbe_disp_get_sfg_pixel_mode(void)
{
	if(u32_vbe_disp_sfg_pixel_mode == 0xff){
		vbe_disp_set_sfg_pixel_mode(SFG_SFG_CTRL_0_get_pif_pixel_mode(IoReg_Read32(SFG_SFG_CTRL_0_reg)));
		rtd_pr_vbe_notice("sfg_pixelMode=0xff, save current value.\n");
	}
	return u32_vbe_disp_sfg_pixel_mode;
}

void vbe_disp_set_vrr_average_den_update(unsigned int u32_update)
{
	u32_vbe_disp_vrr_average_den_update = u32_update;
}

unsigned int vbe_disp_get_vrr_average_den_update(void)
{
	return u32_vbe_disp_vrr_average_den_update;
}

void vbe_disp_set_vrr_average_den_mode(unsigned int u32_mode)
{
	u32_vbe_disp_vrr_average_den_mode = u32_mode;
}

unsigned int vbe_disp_get_vrr_average_den_mode(void)
{
	return u32_vbe_disp_vrr_average_den_mode;
}

#ifndef BUILD_QUICK_SHOW
void vbe_disp_set_VRR_60Hz_average_data_enable_on(unsigned char bEnable)
{
	sys_reg_sys_dclk_gate_sel0_RBUS sys_reg_sys_dclk_gate_sel0_reg;
	ppoverlay_dispd_smooth_toggle1_RBUS ppoverlay_dispd_smooth_toggle1_reg;
	sfg_sfg_ctrl_26_RBUS sfg_sfg_ctrl_26_reg;
	sfg_sfg_ctrl_0_RBUS sfg_sfg_ctrl_0_reg;

	if(Get_DISPLAY_REFRESH_RATE() != 120){
		return;
	}

	if(bEnable){	// VRR panel average den to 60hz
		// dtg clock s2 300M*2pixel
		sys_reg_sys_dclk_gate_sel0_reg.regValue = IoReg_Read32(SYS_REG_SYS_DCLK_GATE_SEL0_reg);
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f2p_gate_sel = 1;
		IoReg_Write32(SYS_REG_SYS_DCLK_GATE_SEL0_reg, sys_reg_sys_dclk_gate_sel0_reg.regValue);
		ppoverlay_dispd_smooth_toggle1_reg.regValue = 0;
		//stage1 clk mode revised default set to 1
		ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_clk_mode_revised = 1;
		ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_src_sel = 1;
		ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, ppoverlay_dispd_smooth_toggle1_reg.regValue);

		// double sfg htotal
		sfg_sfg_ctrl_26_reg.regValue = IoReg_Read32(SFG_SFG_CTRL_26_reg);
		sfg_sfg_ctrl_26_reg.dh_total = 2*vbe_disp_get_sfg_dh_total();
		IoReg_Write32(SFG_SFG_CTRL_26_reg, sfg_sfg_ctrl_26_reg.regValue);

		// den average sfg pixel mode
		sfg_sfg_ctrl_0_reg.regValue = IoReg_Read32(SFG_SFG_CTRL_0_reg);
		sfg_sfg_ctrl_0_reg.pif_pixel_mode = 3;
		IoReg_Write32(SFG_SFG_CTRL_0_reg, sfg_sfg_ctrl_0_reg.regValue);
	}else{
		// dtg clock s2 600M*2pixel
		sys_reg_sys_dclk_gate_sel0_reg.regValue = IoReg_Read32(SYS_REG_SYS_DCLK_GATE_SEL0_reg);
		sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f2p_gate_sel = 0;
		IoReg_Write32(SYS_REG_SYS_DCLK_GATE_SEL0_reg, sys_reg_sys_dclk_gate_sel0_reg.regValue);
		ppoverlay_dispd_smooth_toggle1_reg.regValue = 0;
		//stage1 clk mode revised default set to 1
		ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_clk_mode_revised = 1;
		ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_src_sel = 1;
		ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
		IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, ppoverlay_dispd_smooth_toggle1_reg.regValue);

		// recovery sfg htotal
		sfg_sfg_ctrl_26_reg.regValue = IoReg_Read32(SFG_SFG_CTRL_26_reg);
		sfg_sfg_ctrl_26_reg.dh_total = vbe_disp_get_sfg_dh_total();
		IoReg_Write32(SFG_SFG_CTRL_26_reg, sfg_sfg_ctrl_26_reg.regValue);

		// recovery sfg pixel mode
		sfg_sfg_ctrl_0_reg.regValue = IoReg_Read32(SFG_SFG_CTRL_0_reg);
		sfg_sfg_ctrl_0_reg.pif_pixel_mode = vbe_disp_get_sfg_pixel_mode();
		IoReg_Write32(SFG_SFG_CTRL_0_reg, sfg_sfg_ctrl_0_reg.regValue);
	}

	rtd_pr_vbe_notice("#### [%s] enable=%d\n", __FUNCTION__, bEnable);

}

int vbe_open(struct inode *inode, struct file *filp) {
	return 0;	/* success */
}

ssize_t  vbe_read(struct file *filep, char *buffer, size_t count, loff_t *offp)
{
	return 0;

}

ssize_t vbe_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{
	extern void force_change_orbit_timer_time(void);
	char cmd_buf[128] = {0};
	//long ret = count;
#if 1

	if (count >= 128)
		return -EFAULT;

	if (copy_from_user(cmd_buf, buffer, count)) {
		//ret = -EFAULT;
	}

	if(count > 0) {
	   cmd_buf[count] = '\0';
    }

	rtd_pr_vbe_emerg("vbedev cmd : %s (count=%ld)\n", cmd_buf, count);

	// command : echo test_dpc=1 > /dev/vbedev
	if(strcmp(cmd_buf, "test_dpc=1\n") == 0){
		vbe_disp_dynamic_polarity_control_set_detect_enable(1);
		vbe_disp_dynamic_polarity_control_enable_debug_mode(1);
		rtd_pr_vbe_emerg("enable dpc function \n");
	}
	else if(strcmp(cmd_buf, "test_dpc=0\n") == 0){
		vbe_disp_dynamic_polarity_control_set_detect_enable(0);
		vbe_disp_dynamic_polarity_control_enable_debug_mode(0);
		rtd_pr_vbe_emerg("disable dpc function \n");
	}
	else if(strcmp(cmd_buf, "print_line_cnt=2\n") == 0){
		vbe_disp_set_print_debug_line_count(2);
		rtd_pr_vbe_emerg("print debug line count = 2\n");
	}
	else if(strcmp(cmd_buf, "print_line_cnt=3\n") == 0){
		vbe_disp_set_print_debug_line_count(3);
		rtd_pr_vbe_emerg("print debug line count = 3\n");
	}
	else if(strcmp(cmd_buf, "print_line_cnt=4\n") == 0){
		vbe_disp_set_print_debug_line_count(4);
		rtd_pr_vbe_emerg("print debug line count = 4\n");
	}

    if(strcmp(cmd_buf, "oled_type=1\n") == 0){
		Panel_force_set_oled_type(1);
		rtd_pr_vbe_emerg("force oled panel type = 1\n");
	}
	else if(strcmp(cmd_buf, "oled_type=0\n") == 0){
		Panel_force_set_oled_type(0);
		rtd_pr_vbe_emerg("force oled panel type = 0\n");
	}

	// command : echo voreset_d=0 > /dev/vbedev
	if(strcmp(cmd_buf, "voreset_d=1\n") == 0){
		vbe_disp_set_vo_tracking_d_panel_type(1);
		rtd_pr_vbe_emerg("force voreset_d panel type = 1\n");
	}
	else if(strcmp(cmd_buf, "voreset_d=0\n") == 0){
		vbe_disp_set_vo_tracking_d_panel_type(0);
		rtd_pr_vbe_emerg("force voreset_d panel type = 0\n");
	}
	// command : echo bringup_vbe_test=24 > /dev/vbedev
	if(strcmp(cmd_buf, "bringup_vbe_test=24\n") == 0){
		vbe_disp_bringup_vbe_test(24);
		rtd_pr_vbe_emerg("bringup_vbe_test = 24hz\n");
	}else if(strcmp(cmd_buf, "bringup_vbe_test=25\n") == 0){
		vbe_disp_bringup_vbe_test(25);
		rtd_pr_vbe_emerg("bringup_vbe_test = 25hz\n");
	}else if(strcmp(cmd_buf, "bringup_vbe_test=30\n") == 0){
		vbe_disp_bringup_vbe_test(30);
		rtd_pr_vbe_emerg("bringup_vbe_test = 30hz\n");
	}else if(strcmp(cmd_buf, "bringup_vbe_test=50\n") == 0){
		vbe_disp_bringup_vbe_test(50);
		rtd_pr_vbe_emerg("bringup_vbe_test = 50hz\n");
	}else if(strcmp(cmd_buf, "bringup_vbe_test=60\n") == 0){
		vbe_disp_bringup_vbe_test(60);
		rtd_pr_vbe_emerg("bringup_vbe_test = 60hz\n");
	}
#ifdef CONFIG_ORBIT_PIXEL_SHIFT_ALGO
	// command : echo test_orbit_mode=1 > /dev/vbedev
	else if(strcmp(cmd_buf, "test_orbit_mode=0\n") == 0){
        ORBIT_TIME = 5;
		force_change_orbit_timer_time();
		HAL_VBE_DISP_OLED_SetOrbit(1, 0);
		rtd_pr_vbe_emerg("enable orbit function mode 0\n");
	}
    else if(strcmp(cmd_buf, "test_orbit_mode=01\n") == 0)
    {
        ORBIT_TIME = 1;
        force_change_orbit_timer_time();
        HAL_VBE_DISP_OLED_SetOrbit(1, 0);
        rtd_pr_vbe_emerg("enable orbit function mode 01\n");
    }
    else if(strcmp(cmd_buf, "test_orbit_mode=02\n") == 0){
        ORBIT_TIME = 2;
        force_change_orbit_timer_time();
        HAL_VBE_DISP_OLED_SetOrbit(1, 0);
        rtd_pr_vbe_emerg("enable orbit function mode 02\n");
    }
    else if(strcmp(cmd_buf, "test_orbit_mode=03\n") == 0){
        ORBIT_TIME = 3;
        force_change_orbit_timer_time();
        HAL_VBE_DISP_OLED_SetOrbit(1, 0);
        rtd_pr_vbe_emerg("enable orbit function mode 03\n");
    }
    else if(strcmp(cmd_buf, "test_orbit_mode=04\n") == 0){
        ORBIT_TIME = 4;
        force_change_orbit_timer_time();
        HAL_VBE_DISP_OLED_SetOrbit(1, 0);
        rtd_pr_vbe_emerg("enable orbit function mode 04\n");
    }
	else if(strcmp(cmd_buf, "test_orbit_mode=1\n") == 0){
        ORBIT_TIME = 5;
		force_change_orbit_timer_time();
		HAL_VBE_DISP_OLED_SetOrbit(1, 1);
		rtd_pr_vbe_emerg("enable orbit function mode 1 \n");
	}
	else if(strcmp(cmd_buf, "test_orbit_mode=2\n") == 0){
        ORBIT_TIME = 5;
		force_change_orbit_timer_time();
		HAL_VBE_DISP_OLED_SetOrbit(1, 2);
		rtd_pr_vbe_emerg("enable orbit function mode 1 \n");
	}
	else if(strcmp(cmd_buf, "test_orbit=0\n") == 0){
        ORBIT_TIME = 180;//reset to 3 min or LG previous setting
        	force_change_orbit_timer_time();
		HAL_VBE_DISP_OLED_SetOrbit(0, 0);
		rtd_pr_vbe_emerg("disable orbit function \n");
	}
#endif

	// command : echo test_displayoutput_60hz=1 > /dev/vbedev
	if(strcmp(cmd_buf, "test_displayoutput_60hz=1\n") == 0){
		HAL_VBE_DISP_SetDisplayOutput_5060HZ(1);
		rtd_pr_vbe_emerg("enable HAL_VBE_DISP_SetDisplayOutput_5060HZ = 1.\n");
	}
	else if(strcmp(cmd_buf, "test_displayoutput_60hz=0\n") == 0){
		HAL_VBE_DISP_SetDisplayOutput_5060HZ(0);
		rtd_pr_vbe_emerg("enable HAL_VBE_DISP_SetDisplayOutput_5060HZ = 0.\n");
	}

#ifdef CONFIG_DYNAMIC_PANEL_SELECT
    if(strcmp(cmd_buf, "dlgEnable=1\n") == 0){
        HAL_VBE_Panel_Set_DlgEnable(1);
        rtd_pr_vbe_emerg("enable dlgEnable = 1\n");
    }
    else if(strcmp(cmd_buf, "dlgEnable=0\n") == 0){
        HAL_VBE_Panel_Set_DlgEnable(0);
        rtd_pr_vbe_emerg("enable dlgEnable = 0\n");
    }
#endif

    if(strcmp(cmd_buf, "memc_frc2fsync=1\n") == 0)
    {
        drvif_scaler_memc_set_display_mode_frc2fsync_by_hw_speedup(1);
    }
    else if(strcmp(cmd_buf, "memc_frc2fsync=0\n") == 0)
    {
        drvif_scaler_memc_set_display_mode_frc2fsync_by_hw_speedup(0);
    }

    if(strcmp(cmd_buf, "panelinfo=1\n") == 0)
    {
        KADP_DISP_PANEL_INFO_T panelInfo;
        memset(&panelInfo, 0, sizeof(KADP_DISP_PANEL_INFO_T));
        HAL_DISP_PANEL_GetPanelInfo(&panelInfo);
        rtd_pr_vbe_emerg("panelInfo.panelFrameRate:%d\n", panelInfo.panelFrameRate);
    }

	return count;
#else
	return 0;
#endif
}


int vbe_release(struct inode *inode, struct file *filep)
{
	return 0;
}

UINT16 Mplus_Value[928] = {1};
long vbe_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	int retval = 0;

	//rtd_pr_vbe_debug("VBE: vbe_ioctl, %x;;agr:%lx\n", _IOC_NR(cmd), arg);
	if (_IOC_TYPE(cmd) != VBE_IOC_MAGIC || _IOC_NR(cmd) > VBE_IOC_MAXNR) return -ENOTTY ;

	switch (cmd)
	{
		case VBE_IOC_DISP_INITIALIZE:
		{
			KADP_DISP_PANEL_INFO_T panel_info;
			if(copy_from_user((void *)&panel_info, (const void __user *)arg, sizeof(KADP_DISP_PANEL_INFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_INITIALIZE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VBE_DISP_Initialize(panel_info)==0)
				{
					retval = -EFAULT;
				}
			}
			break;
		}
		case VBE_IOC_DISP_UNINITIALIZE:
		{
			if(HAL_VBE_DISP_Uninitialize()==0)
			{
				retval = -EFAULT;
			}
			break;
		}
		case VBE_IOC_DISP_RESUME:
		{
			KADP_DISP_PANEL_INFO_T panel_info;
			if(copy_from_user((void *)&panel_info, (const void __user *)arg, sizeof(KADP_DISP_PANEL_INFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_RESUME failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VBE_DISP_Resume(panel_info)==0)
				{
					retval = -EFAULT;
				}
			}
			break;
		}
		case VBE_IOC_DISP_CONNECT:
		{
			KADP_DISP_WINDOW_T disp_window;
			if(copy_from_user((void *)&disp_window, (const void __user *)arg, sizeof(KADP_DISP_WINDOW_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_CONNECT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_Connect(disp_window);
			}
			break;
		}
		case VBE_IOC_DISP_DISCONNECT:
		{
			KADP_DISP_WINDOW_T disp_window;
			if(copy_from_user((void *)&disp_window, (const void __user *)arg, sizeof(KADP_DISP_WINDOW_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_DISCONNECT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_Disconnect(disp_window);
			}
			break;
		}
		case VBE_IOC_DISP_WRITE_DIAPLAYOUTPUT:
		{
			BOOLEAN length;
			if(copy_from_user((void *)&length, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_WRITE_DIAPLAYOUTPUT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetDisplayOutput(length);
			}
			break;
		}
		case VBE_IOC_DISP_WRITE_MUTE:
		{
			BOOLEAN length;
			if(copy_from_user((void *)&length, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_WRITE_MUTE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetMute(length);
			}
			break;
		}
		case VBE_IOC_DISP_WRITE_SPREADSPECTRUM:
		{
			KADP_VBE_DISP_SpreadSpectrum_T spreadspectrum;
			if(copy_from_user((void *)&spreadspectrum, (const void __user *)arg, sizeof(KADP_VBE_DISP_SpreadSpectrum_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_WRITE_SPREADSPECTRUM failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetSpreadSpectrum(spreadspectrum.bEnable,spreadspectrum.Percenct,spreadspectrum.Period);
			}
			break;
		}
		case VBE_IOC_DISP_WRITE_VIDEO_MIRROR:
		{
			KADP_VBE_DISP_VideoMirror_T mirror_enable;
			if(copy_from_user((void *)&mirror_enable, (const void __user *)arg, sizeof(KADP_VBE_DISP_VideoMirror_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_WRITE_VIDEO_MIRROR failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(HAL_VBE_DISP_SetVideoMirror(mirror_enable.bIsH,mirror_enable.bIsV)==FALSE)
				{
					retval = -EFAULT;
				}
			}
			break;
		}
		case VBE_IOC_DISP_WRITE_DISPLAY_FORMAT:
		{
			KADP_VBE_DISP_FORMAT_T panel_info;
			if(copy_from_user((void *)&panel_info, (const void __user *)arg, sizeof(KADP_VBE_DISP_FORMAT_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_WRITE_DISPLAY_FORMAT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetDisplayFormat(&(panel_info.panel_attribute),panel_info.framerate);
			}
			break;
		}
		case VBE_IOC_DISP_WRITE_FRAMERATE:
		{
			KADP_VBE_DISP_Framerate_T framerate_info;
			if(copy_from_user((void *)&framerate_info, (const void __user *)arg, sizeof(KADP_VBE_DISP_Framerate_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_WRITE_FRAMERATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetFrameRate(framerate_info.isForceFreeRun,framerate_info.framerate);
			}
			break;
		}
		case VBE_IOC_DISP_READ_OUTPUTFRAMERATE:
		{
			UINT16 framerate=0;
			HAL_VBE_DISP_GetOutputFrameRate(&framerate);
			if(copy_to_user((void __user *)arg, (void *)&framerate, sizeof(UINT16)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_READ_OUTPUTFRAMERATE failed!!!!!!!!!!!!!!!\n");
			}
			break;
		}
		case VBE_IOC_DISP_WRITE_BGCOLOR:
		{
			KADP_VBE_DISP_BGColor_T color;
			if(copy_from_user((void *)&color, (const void __user *)arg, sizeof(KADP_VBE_DISP_BGColor_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_WRITE_BGCOLOR failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetBackgroundColor(color.red,color.green,color.blue);
			}
			break;
		}

		case VBE_IOC_DISP_OD_WRITE_TABLE:
		{
			extern unsigned char od_target_table_g[OD_table_length];

			enum PLAFTORM_TYPE platform = get_platform();
			if (PLATFORM_KXLP != platform)
				return -1;

			return -1;  //fixed me , force od enable by lg request,when  project going please remove it.rord 2018/01/16
			if (copy_from_user(od_target_table_g, (void __user *)arg, sizeof(unsigned char)*OD_table_length))
			{
		 		rtd_pr_vbe_debug("kernel VBE_IOC_DISP_OD_WRITE_TABLE fail\n");
				retval = -1;
			}
			else
			{
				extern unsigned char bODTableLoaded;
				extern char od_table_mode_store;
				extern unsigned char od_delta_table_g[OD_table_length];

				fwif_color_od_table_transform(0, 0, od_target_table_g, od_delta_table_g);
				drvif_color_od_table_17x17_set(od_delta_table_g, 0, 3);// 0:delta mode 1:target mode(output mode)
				rtd_pr_vbe_info("[OD]%s, od_delta_table_g updated\n", __func__);

				od_table_mode_store = 1;// 0:target mode 1:delta mode -1:inverse mode
				bODTableLoaded = TRUE;
				rtd_pr_vbe_debug("kernel VBE_IOC_DISP_OD_WRITE_TABLE success\n");
			 	retval = 0;
			}
			break;
		}
		case VBE_IOC_DISP_OD_ENABLE:
		{
			unsigned int tmp;

			return -1;  //fixed me , force od enable by lg request,when  project going please remove it.rord 2018/01/16

			if (copy_from_user(&tmp, (unsigned char __user *)arg, sizeof(tmp)))
			{
		 		rtd_pr_vbe_debug("kernel VBE_IOC_DISP_OD_ENABLE fail\n");
				retval = -1;
			}
			else
			{
				extern unsigned char bODInited;
				extern unsigned char bODTableLoaded;
				extern unsigned char bODPreEnable;

				enum PLAFTORM_TYPE platform = get_platform();
				if (PLATFORM_KXLP != platform)
					return -1;

				rtd_pr_vbe_debug("kernel VBE_IOC_DISP_OD_ENABLE = %d\n",tmp);
				bODPreEnable = tmp;
				if (!bODInited || !bODTableLoaded)
					return -1;
				fwif_color_set_od(tmp);
				retval = 0;
			}
			break;
		}

		case VBE_IOC_DISP_PCID_WRITE_TABLE:
		{
#ifdef SCALER_PCID_ENABLE
#if SCALER_PCID_ENABLE
			unsigned char PCIDtable[OD_table_length];
			if (copy_from_user(&PCIDtable[0], (unsigned char __user *)arg, sizeof(unsigned char)*OD_table_length))
			{
		 		rtd_pr_vbe_debug("kernel VBE_IOC_DISP_PCID_WRITE_TABLE fail\n");
				retval = -1;
			}
			else
			{
				drvif_color_pcid_table(PCIDtable);
				rtd_pr_vbe_debug("kernel VBE_IOC_DISP_PCID_WRITE_TABLE success\n");
			 	retval = 0;
			}
#endif
#endif
			break;

		}
		case VBE_IOC_DISP_PCID_ENABLE:
		{
#ifdef SCALER_PCID_ENABLE
#if SCALER_PCID_ENABLE
			unsigned int tmp;
			if (copy_from_user(&tmp, (unsigned char __user *)arg, sizeof(tmp)))
			{
		 		rtd_pr_vbe_debug("kernel VBE_IOC_DISP_PCID_ENABLE fail\n");
				retval = -1;
			}
			else
			{
				rtd_pr_vbe_debug("kernel VBE_IOC_DISP_PCID_ENABLE = %d\n",tmp);
				drvif_color_pcid(tmp);
				retval = 0;
			}
#endif
#endif
			break;
		}

		case VBE_IOC_DISP_SR_INITIALIZE:
		{
			break;
		}
		case VBE_IOC_DISP_SR_UNINITIALIZE:
		{
			break;
		}
		case VBE_IOC_DISP_SR_WRITE_CONTROL:
		{
			break;
		}
		case VBE_IOC_DISP_TCON_INITILIZE:
		{
			HAL_VBE_DISP_TCON_Initialize();
			break;
		}
		case VBE_IOC_DISP_TCON_UNINITILIZE:
		{
			HAL_VBE_DISP_TCON_Uninitialize();
			break;
		}
		case VBE_IOC_DISP_TCON_ENABLE_TCON:
		{
			BOOLEAN enable;
			if(copy_from_user((void *)&enable, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_TCON_ENABLE_TCON failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_TCON_EnableTCon(enable);
			}
			break;
		}
		case VBE_IOC_DISP_TCON_EABLE_COLORTEMP:
		{
			break;
		}
		case VBE_IOC_DISP_TCON_EABLE_DGA:
		{
			break;
		}
		case VBE_IOC_DISP_TCON_ENABLE_DITHER:
		{
			//unsigned int* tmp = (unsigned int*)arg;
			unsigned int tmp;
			if (copy_from_user(&tmp, (unsigned char __user *)arg, sizeof(tmp)))
			{
		 		rtd_pr_vbe_debug("kernel VBE_IOC_DISP_TCON_ENABLE_DITHER fail\n");
				retval = -1;
			}
			else
			{
	 			rtd_pr_vbe_debug("kernel VBE_IOC_DISP_TCON_ENABLE_DITHER = %d\n",tmp);
				HAL_VBE_DISP_TCON_EnableDither(tmp);
				rtd_pr_vbe_debug("kernel VBE_IOC_DISP_TCON_ENABLE_DITHER success\n");
				retval = 0;
			}

			break;
		}
		case VBE_IOC_DISP_TCON_SET_DITHER_MODE:
		{
			//unsigned int* tmp = (unsigned int*)arg;
			unsigned int tmp;
                        if (copy_from_user(&tmp, (unsigned char __user *)arg, sizeof(tmp)))
                        {
                                rtd_pr_vbe_debug("kernel VBE_IOC_DISP_TCON_SET_DITHER_MODE fail\n");
                                retval = -1;
                        }
                        else
                        {
	 			rtd_pr_vbe_debug("kernel VBE_IOC_DISP_TCON_SET_DITHER_MODE = %d\n",tmp);
	 			rtd_pr_vbe_debug("kernel VBE_IOC_DISP_TCON_SET_DITHER_MODE = %d\n",tmp);
				HAL_VBE_DISP_TCON_SetDitherMode((KADP_DISP_TCON_DITHER_T)tmp);
				retval = 0;
			}
			break;
		}
		case VBE_IOC_DISP_TCON_SET_GAMMA_TABLE:
		{
			break;
		}
		case VBE_IOC_DISP_TCON_H_REVERSE:
		{
			UINT32 h_rev_mode;
			if(copy_from_user((void *)&h_rev_mode, (const void __user *)arg, sizeof(UINT32)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_TCON_H_REVERSE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{

				HAL_VBE_DISP_TCON_H_Reverse(h_rev_mode);
			}
			break;
		}
		case VBE_IOC_DISP_TCON_SET_CLOCK:
		{
			UINT32 bOnOff;
			if(copy_from_user((void *)&bOnOff, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_TCON_SET_CLOCK failed!!!!!!!!!!!!!!!\n");
			}
			else
			{

				HAL_VBE_DISP_TCON_SetClock(bOnOff);
			}
			break;
		}
		case VBE_IOC_DISP_TCON_WRITE_REGISTER:
		{

			HAL_VBE_DISP_TCON_WRITE_REGISTERS disp_tcon_write_reg_info;
			if(copy_from_user((void *)&disp_tcon_write_reg_info, (const void __user *)arg, sizeof(HAL_VBE_DISP_TCON_WRITE_REGISTERS)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_TCON_WRITE_REGISTER failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_TCON_WriteRegister(disp_tcon_write_reg_info.reg_address, disp_tcon_write_reg_info.reg_data);
			}
			break;
		}
		case VBE_IOC_DISP_TCON_READ_REGISTER:
		{
			HAL_VBE_DISP_TCON_READ_REGISTERS disp_tcon_read_reg_info;
			if(copy_from_user((void *)&disp_tcon_read_reg_info, (const void __user *)arg, sizeof(HAL_VBE_DISP_TCON_READ_REGISTERS)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_TCON_READ_REGISTER failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_TCON_ReadRegister(disp_tcon_read_reg_info.reg_address, to_user_ptr(disp_tcon_read_reg_info.reg_data));
			}
			break;
		}
		case VBE_IOC_DISP_TCON_SET_LVDS_FORMAT:
		{
			KADP_VBE_DISP_TCON_LVDS_TYPE_T type;
			if(copy_from_user((void *)&type, (const void __user *)arg, sizeof(KADP_VBE_DISP_TCON_LVDS_TYPE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_WRITE_FRAMERATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_TCON_SetLvdsFormat(type);
			}
			break;
		}
		case VBE_IOC_DISP_TCON_DEBUG:
		{
			HAL_VBE_DISP_TCON_Debug();
			break;
		}

		case VBE_IOC_AVE_INITILIZE:
		{
			HAL_VBE_AVE_Initialize();
			break;
		}
		case VBE_IOC_AVE_UNINITILIZE:
		{
			HAL_VBE_AVE_Uninitialize();
			break;
		}
		case VBE_IOC_AVE_OPEN:
		{
			HAL_VBE_AVE_Open();
			break;
		}
		case VBE_IOC_AVE_CLOSE:
		{
			HAL_VBE_AVE_Close();
			break;
		}
		case VBE_IOC_AVE_CONNECT:
		{
			KADP_VBE_AVE_INPUT_INDEX AVEInput;
			if(copy_from_user((void *)&AVEInput, (const void __user *)arg, sizeof(KADP_VBE_AVE_INPUT_INDEX)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_WRITE_FRAMERATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_AVE_Connect(AVEInput);
			}
			break;
		}
		case VBE_IOC_AVE_DISCONNECT:
		{
			HAL_VBE_AVE_Disconnect();
			break;
		}
		case VBE_IOC_AVE_IS_ATV_WATCHING:
		{
			HAL_VBE_AVE_IsATVWatching();
			break;
		}
		case VBE_IOC_AVE_SET_ATV_WATCHING:
		{
			VBE_AVE_SET_ATVWATCHING_T set_atv_watching_info;

			if(copy_from_user((void *)&set_atv_watching_info, (const void __user *)arg, sizeof(VBE_AVE_SET_ATVWATCHING_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_AVE_SET_ATV_WATCHING failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_AVE_SetATVWatching(set_atv_watching_info.AVEInput, set_atv_watching_info.bIsATVWatching);
			}

			break;
		}
		case VBE_IOC_AVE_VBI_SET_INSERT:
		{
			VBE_AVE_VBI_SET_INSERT_T vbi_set_insert_info;
			if(copy_from_user((void *)&vbi_set_insert_info, (const void __user *)arg, sizeof(VBE_AVE_VBI_SET_INSERT_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_AVE_VBI_SET_INSERT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_AVE_VBI_SetInsert(vbi_set_insert_info.type, vbi_set_insert_info.OnOff);
			}

			break;
		}
		case VBE_IOC_AVE_VBI_INSERT_DATA:
		{
			VBE_AVE_VBI_INSERT_DATA_T vbi_insert_data_info;
			if(copy_from_user((void *)&vbi_insert_data_info, (const void __user *)arg, sizeof(VBE_AVE_VBI_INSERT_DATA_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_AVE_VBI_INSERT_DATA failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_AVE_VBI_InsertData(vbi_insert_data_info.type, vbi_insert_data_info.nrLine, to_user_ptr(vbi_insert_data_info.pLines), vbi_insert_data_info.size, to_user_ptr(vbi_insert_data_info.pData));
			}
			break;
		}
		case VBE_IOC_AVE_SET_VCR_WIN_BLANK:
		{
			BOOLEAN bOnOff;
			if(copy_from_user((void *)&bOnOff, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_AVE_SET_VCR_WIN_BLANK failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_AVE_SetVCRWinBlank(bOnOff);
			}
			break;
		}
		case VBI_IOC_DISP_LVDS_TO_HDMI:
		{
			KADP_VBE_DISP_FORMAT_T panel_info;
			if(copy_from_user((void *)&panel_info, (const void __user *)arg, sizeof(KADP_VBE_DISP_FORMAT_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBI_IOC_DISP_LVDS_TO_HDMI failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				Scaler_set_lvds_to_hdmi_parameter(&(panel_info.panel_attribute),panel_info.framerate);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_INITIALIZE:
		{
			HAL_VBE_TRIDFMT_Initialize();
			break;
		}
		case VBE_IOC_TRIDFMT_UNINITIALIZE:
		{
			HAL_VBE_TRIDFMT_Uninitialize();
			break;
		}
		case VBE_IOC_TRIDFMT_OPEN:
		{
			HAL_VBE_TRIDFMT_Open();
			break;
		}
		case VBE_IOC_TRIDFMT_CLOSE:
		{
			HAL_VBE_TRIDFMT_Close();
			break;
		}
		case VBE_IOC_TRIDFMT_SETSCREENONOFF:
		{
			BOOLEAN enable;
			if(copy_from_user((void *)&enable, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_SETSCREENONOFF failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_SetScreenOnOff(enable);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_SETFMTMODE:
		{
			VBE_TRIDFMT_FMT_MODE_T fmtMode;
			if(copy_from_user((void *)&fmtMode, (const void __user *)arg, sizeof(VBE_TRIDFMT_FMT_MODE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_SETFMTMODE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_SetFMTMode(fmtMode.wId, to_user_ptr(fmtMode.TriFRC));
			}
			break;
		}
		case VBE_IOC_TRIDFMT_SETFMTMODE_TEMP:
		{
			VBE_TRIDFMT_FMT_MODE_TEMP_T fmtMode;
			if(copy_from_user((void *)&fmtMode, (const void __user *)arg, sizeof(VBE_TRIDFMT_FMT_MODE_TEMP_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_SETFMTMODE_TEMP failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_SetFMTMode_TEMP(fmtMode.wId, to_user_ptr(fmtMode.TriFRC));
			}
			break;
		}
		case VBE_IOC_TRIDFMT_SET3DMODEONOFF:
		{
			KADP_TRIDTV_INPUT_CONFIG_INFO_T cfgInfo;
			if(copy_from_user((void *)&cfgInfo, (const void __user *)arg, sizeof(KADP_TRIDTV_INPUT_CONFIG_INFO_T )))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_SET3DMODEONOFF failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_Set3DModeOnOff(&cfgInfo);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_TOGGLEINPUTLRFRAME:
		{
			KADP_TRIDTV_INPUT_LR_SEQ_T type;
			if(copy_from_user((void *)&type, (const void __user *)arg, sizeof(KADP_TRIDTV_INPUT_LR_SEQ_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_TOGGLEINPUTLRFRAME failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_ToggleInputLRFrame(type);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_SELECTINPUTFORMAT:
		{
			KADP_TRIDTV_INPUT_TYPE_T type;
			if(copy_from_user((void *)&type, (const void __user *)arg, sizeof(KADP_TRIDTV_INPUT_TYPE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_SELECTINPUTFORMAT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_SelectInputFormat(type);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_SETINPUTFRAMERATE:
		{
			KADP_TRIDTV_INPUT_FREQ_T type;
			if(copy_from_user((void *)&type, (const void __user *)arg, sizeof(KADP_TRIDTV_INPUT_FREQ_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_SETINPUTFRAMERATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_SetInputFrameRate(type);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_TOGGLEOUTPUTLRFRAME:
		{
			KADP_TRIDTV_OUTPUT_LR_SEQ_T type;
			if(copy_from_user((void *)&type, (const void __user *)arg, sizeof(KADP_TRIDTV_OUTPUT_LR_SEQ_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_TOGGLEOUTPUTLRFRAME failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_ToggleOutputLRFrame(type);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_SELECTOUTPUTFORMAT:
		{
			KADP_TRIDTV_OUTPUT_TYPE_T type;
			if(copy_from_user((void *)&type, (const void __user *)arg, sizeof(KADP_TRIDTV_OUTPUT_TYPE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_SELECTOUTPUTFORMAT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_SelectOutputFormat(type);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_SETOUTPUTFRAMERATE:
		{
			KADP_TRIDTV_OUTPUT_FREQ_T type;
			if(copy_from_user((void *)&type, (const void __user *)arg, sizeof(KADP_TRIDTV_OUTPUT_FREQ_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_SETOUTPUTFRAMERATE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_SetOutputFrameRate(type);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_GET3DCFGINFO:
		{
			VBE_TRIDFMT_3D_CFG_INFO_T modeInfo;
			if(copy_from_user((void *)&modeInfo, (const void __user *)arg, sizeof(VBE_TRIDFMT_3D_CFG_INFO_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_GET3DCFGINFO failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_Get3DCfgInfo(modeInfo.n3DCfgItems, to_user_ptr(modeInfo.nCurCfgInfo));
			}
			break;
		}
		case VBE_IOC_TRIDFMT_SET3DCFGINFO:
		{
			KADP_TRIDTV_INPUT_CONFIG_INFO_T *p3DModeInfo;
			if(copy_from_user((void *)&p3DModeInfo, (const void __user *)arg, sizeof(KADP_TRIDTV_INPUT_CONFIG_INFO_T *)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_SET3DCFGINFO failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_Set3DCfgInfo(p3DModeInfo);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_SETCONVVALUE:
		{
			char convVal;
			if(copy_from_user((void *)&convVal, (const void __user *)arg, sizeof(char)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_SETCONVVALUE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_SetConvValue(convVal);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_CTRLDEPTH:
		{
			VBE_TRIDFMT_CTRL_DEPTH_T depthInfo;
			if(copy_from_user((void *)&depthInfo, (const void __user *)arg, sizeof(VBE_TRIDFMT_CTRL_DEPTH_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_CTRLDEPTH failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_CtrlDepth(depthInfo.n3Dmode, depthInfo.u8DepthLevel);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_AUTOCONVERGENCEENABLE:
		{
			BOOLEAN enable;
			if(copy_from_user((void *)&enable, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_AUTOCONVERGENCEENABLE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_AutoConvergenceEnable(enable);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_SET3DVIDEOOUTPUTMODE:
		{
			UINT32 bIsSGMode;
			if(copy_from_user((void *)&bIsSGMode, (const void __user *)arg, sizeof(UINT32)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_SET3DVIDEOOUTPUTMODE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_Set3DVideoOutputMode(bIsSGMode);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_SET3DDEPTHCONTROLLER:
		{
			UINT32 bOnOff;
			if(copy_from_user((void *)&bOnOff, (const void __user *)arg, sizeof(UINT32)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TRIDFMT_SET3DDEPTHCONTROLLER failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_TRIDFMT_Set3DDepthController(bOnOff);
			}
			break;
		}
		case VBE_IOC_TRIDFMT_DEBUG:
		{
			HAL_VBE_TRIDFMT_Debug();
			break;
		}
		case VBE_IOC_SETORBIT:
		{
			BOOLEAN enable;
			if(copy_from_user((void *)&enable, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETORBIT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
#ifdef CONFIG_ORBIT_PIXEL_SHIFT_ALGO
				HAL_VBE_DISP_OLED_SetOrbit(enable, 0);
#endif
			}
			break;
		}
		case VBE_IOC_SETBOEMODE:
		{
			UINT32 data_len;
			unsigned char	*m_pCacheStartAddr = NULL;

			rtd_pr_vbe_notice("VBE_IOC_SETBOEMODE\n");

			if(copy_from_user((void *)&data_len, (const void __user *)arg, sizeof(UINT32)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETORBIT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				rtd_pr_vbe_notice("data_len=%d\n", data_len);
				m_pCacheStartAddr = (unsigned char *)dvr_malloc(data_len);
				if(m_pCacheStartAddr == NULL){
					rtd_pr_vbe_err("[ERROR]VBE_IOC_SETBOEMODE Allocate data_len=%x fail\n",data_len);
					return FALSE;
				}

				if(copy_from_user((void *)m_pCacheStartAddr, (const void __user *)(arg+sizeof(UINT32)), data_len))
				{
					retval = -EFAULT;
					rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETORBIT failed!!!!!!!!!!!!!!!\n");
				}
				HAL_VBE_DISP_SetBOEMode(m_pCacheStartAddr, data_len);

				if(m_pCacheStartAddr){
					dvr_free((void *)m_pCacheStartAddr);
				}
			}

			break;
		}
		case VBE_IOC_GETBOEVERSION:
		{
			KADP_VBE_DISP_BOE_VERSION_T vbeDispBoeVer;
			KADP_DISP_BOE_VERSION_T stRomVer = { 0, 0, 0 };
			KADP_DISP_BOE_VERSION_T stFwVer = { 0, 0, 0 };
			rtd_pr_vbe_notice("VBE_IOC_GETBOEVERSION\n");

			HAL_VBE_DISP_GetBOEVersion(&stRomVer, &stFwVer);

			//rtd_pr_vbe_emerg("stRomVer(%d.%d.%d), stFwVer(%d.%d.%d)\n",
				//stRomVer.b0, stRomVer.b1, stRomVer.b2, stFwVer.b0, stFwVer.b1, stFwVer.b2);

			memcpy((UINT8*)&(vbeDispBoeVer.stRomVer), (UINT8*)&stRomVer, sizeof(KADP_DISP_BOE_VERSION_T));
			memcpy((UINT8*)&(vbeDispBoeVer.stFwVer), (UINT8*)&stFwVer, sizeof(KADP_DISP_BOE_VERSION_T));

			if(copy_to_user((void __user *)arg, (void *)&vbeDispBoeVer, sizeof(KADP_VBE_DISP_BOE_VERSION_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_GETBOEVERSION failed!!!!!!!!!!!!!!!\n");
			}

			break;
		}
		case VBE_IOC_SETMLEMODE:
		{
			VBE_DISP_MLE_MODE_T index;

			rtd_pr_vbe_notice("VBE_IOC_SETMLEMODE\n");

			if(copy_from_user((void *)&index, (const void __user *)arg, sizeof(VBE_DISP_MLE_MODE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code = VBE_IOC_SETMLEMODE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				#ifdef CONFIG_SCALER_ENABLE_V4L2
				HAL_VBE_DISP_SetMLEMode(index);
				#endif
			}
			break;
		}
		case VBE_IOC_SETINNERPATTERN:
		{
 			VBE_DISP_INNER_PATTERN_CTRL_T dispInnerPtgCtrl;

			rtd_pr_vbe_notice("VBE_IOC_SETINNERPATTERN\n");

			if(copy_from_user((void *)&dispInnerPtgCtrl, (const void __user *)arg, sizeof(VBE_DISP_INNER_PATTERN_CTRL_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code = VBE_IOC_SETINNERPATTERN failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetInnerPattern(dispInnerPtgCtrl.bOnOff, dispInnerPtgCtrl.block, dispInnerPtgCtrl.type);
			}
			break;

		}

		case VBE_IOC_SETBOERGBWBYPASS:
		{
			BOOLEAN enable;
			if(copy_from_user((void *)&enable, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETBOERGBWBYPASS failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetBOERGBWBypass(enable);
			}
			break;
		}

		case VBE_IOC_SETDGA4CH:
		{
			UINT16 table_size;
			unsigned char	*m_pCacheStartAddr = NULL;

			rtd_pr_vbe_notice("VBE_IOC_SETDGA4CH\n");

			if(copy_from_user((void *)&table_size, (const void __user *)arg, sizeof(UINT16)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETDGA4CH failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(table_size > 0){
					rtd_pr_vbe_notice("table_size=%d\n", table_size);
					m_pCacheStartAddr = (unsigned char *)dvr_malloc(4*table_size*sizeof(UINT32));
					if(m_pCacheStartAddr == NULL){
						rtd_pr_vbe_err("[ERROR]VBE_IOC_SETDGA4CH Allocate table_size=%x fail\n",table_size);
						return FALSE;
					}

					if(copy_from_user((void *)m_pCacheStartAddr, (const void __user *)(arg+sizeof(UINT16)), 4*table_size*sizeof(UINT32)))
					{
						retval = -EFAULT;
						rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETDGA4CH failed!!!!!!!!!!!!!!!\n");
					}
					HAL_VBE_SetDGA4CH((UINT32 *)m_pCacheStartAddr, (UINT32 *)(m_pCacheStartAddr+table_size*sizeof(UINT32)), (UINT32 *)(m_pCacheStartAddr+2*table_size*sizeof(UINT32)),
						(UINT32 *)(m_pCacheStartAddr+3*table_size*sizeof(UINT32)), table_size);

					if(m_pCacheStartAddr){
						dvr_free((void *)m_pCacheStartAddr);
					}
				}
			}

			break;
		}
		case VBE_IOC_SETFRAMEGAINLIMIT:
		{
			unsigned short nFrameGainLimit = 0;
			if(copy_from_user((void *)&nFrameGainLimit, (const void __user *)arg, sizeof(unsigned short)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETFRAMEGAINLIMIT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetFrameGainLimit(nFrameGainLimit);
			}
			break;
		}

		case VBE_IOC_GETFRAMEGAINLIMIT:
		{
			unsigned short nFrameGainLimit=0;
			HAL_VBE_DISP_GetFrameGainLimit(&nFrameGainLimit);
			if(copy_to_user((void __user *)arg, (void *)&nFrameGainLimit, sizeof(unsigned short)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_GETFRAMEGAINLIMIT failed!!!!!!!!!!!!!!!\n");
			}
			break;
		}
		case VBE_IOC_SETPIXELGAINLIMIT:
		{
			unsigned short nPixelGainLimit = 0;
			if(copy_from_user((void *)&nPixelGainLimit, (const void __user *)arg, sizeof(unsigned short)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETPIXELGAINLIMIT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetPixelGainLimit(nPixelGainLimit);
			}
			break;
		}
		case VBE_IOC_GETPIXELGAINLIMIT:
		{
			unsigned short nPixelGainLimit=0;
			HAL_VBE_DISP_GetPixelGainLimit(&nPixelGainLimit);
			if(copy_to_user((void __user *)arg, (void *)&nPixelGainLimit, sizeof(unsigned short)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_GETPIXELGAINLIMIT failed!!!!!!!!!!!!!!!\n");
			}
			break;
		}
		case VBE_IOC_GETPANELDISPSIZE:
		{
			KADP_DISP_PANEL_SIZE_T panel_size;

			//rtd_pr_vbe_info("VBE_IOC_GETPANELDISPSIZE \n");

			HAL_VBE_DISP_GetPanelSize(&panel_size);
			//rtd_pr_vbe_info("w= %d, h= %d\n",panel_size.DISP_WIDTH,panel_size.DISP_HEIGHT);

			if(copy_to_user((void __user *)arg, (void *)&panel_size, sizeof(KADP_DISP_PANEL_SIZE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_GETPANELDISPSIZE failed!!!!!!!!!!!!!!!\n");
			}
			break;
		}
		case VBE_IOC_TSCIC_LOAD:
		{

			VBE_DISP_PANEL_TSCIC_T tFCIC_T;
			unsigned char	*m_pCacheStartAddr = NULL;
			if(copy_from_user((void *)&tFCIC_T, (const void __user *)arg, sizeof(VBE_DISP_PANEL_TSCIC_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETPIXELGAINLIMIT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				rtd_pr_vbe_notice("FCIC_table_size=%d, FCIC_Ctrl_size=%d\n", tFCIC_T.u32Tscicsize, tFCIC_T.u32Ctrlsize);

				if ((tFCIC_T.u32Tscicsize==0) || (tFCIC_T.u32Tscicsize!=0x5C30C)) {
					fwif_color_set_fcic_TV006(NULL, tFCIC_T.u32Tscicsize, NULL, tFCIC_T.u32Ctrlsize, 0);

				} else {

					tFCIC_T.u32Tscicsize = tFCIC_T.u32Tscicsize/4; // our driver use 94403

					m_pCacheStartAddr = (unsigned char *)dvr_malloc(tFCIC_T.u32Tscicsize*sizeof(UINT32)+tFCIC_T.u32Ctrlsize*sizeof(UINT8));
					if(m_pCacheStartAddr == NULL){
						rtd_pr_vbe_err("[ERROR]VBE_IOC_TSCIC_LOAD Allocate table_size=%lx fail\n",(unsigned long)(tFCIC_T.u32Tscicsize*sizeof(UINT32)+tFCIC_T.u32Ctrlsize*sizeof(UINT8)));
						return FALSE;
					}
					/*FCIC Table*/
					if(copy_from_user((void *)m_pCacheStartAddr, (const void __user *)to_user_ptr(tFCIC_T.u32pTSCICTbl), tFCIC_T.u32Tscicsize*sizeof(UINT32)))
					{
						retval = -EFAULT;
						rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TSCIC_LOAD  Table failed!!!!!!!!!!!!!!!\n");
					}
					/*FCIC_Ctrl*/
					if(copy_from_user((void *)m_pCacheStartAddr+tFCIC_T.u32Tscicsize*sizeof(UINT32), (const void __user *)to_user_ptr(tFCIC_T.u8pControlTbl), tFCIC_T.u32Ctrlsize*sizeof(UINT8)))
					{
						retval = -EFAULT;
						rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_TSCIC_LOAD Ctrl failed!!!!!!!!!!!!!!!\n");
					}


					HAL_VBE_TSCIC_Load((UINT32 *)m_pCacheStartAddr, tFCIC_T.u32Tscicsize, m_pCacheStartAddr+tFCIC_T.u32Tscicsize*sizeof(UINT32), tFCIC_T.u32Ctrlsize);

					if(m_pCacheStartAddr){
						dvr_free((void *)m_pCacheStartAddr);
					}
					}
			}
			break;
		}
		case VBE_IOC_SETOLEDLSR:
		{
			VBE_LSR_ADJ_MODE_T tLSR_ADJ_Mode;
			UINT32 LSRTbl;

			if(copy_from_user((void *)&tLSR_ADJ_Mode, (const void __user *)arg, sizeof(VBE_LSR_ADJ_MODE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETOLEDLSR 1 failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(copy_from_user((void *)&LSRTbl, (const void __user *)to_user_ptr(tLSR_ADJ_Mode.pLSRTbl), sizeof(UINT32)))
				{
					retval = -EFAULT;
					rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETOLEDLSR 2 failed!!!!!!!!!!!!!!!\n");
				}

				HAL_VBE_DISP_SetOLEDLSR(tLSR_ADJ_Mode.nStep, &LSRTbl);
			}
			break;
		}
		case VBE_IOC_SETOLEDLSRTUNNING:
		{
			VBE_DISP_LSR_TUNNING_VAL_T tLSR_Tunning_Val;

			if(copy_from_user((void *)&tLSR_Tunning_Val, (const void __user *)arg, sizeof(VBE_DISP_LSR_TUNNING_VAL_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETOLEDLSR 1 failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetOLEDLSR_Tunning(&tLSR_Tunning_Val);
			}

			break;
		}
		case VBE_IOC_SETOLEDCPCTUNNING:
		{

			VBE_DISP_CPC_TUNNING_VAL_T tCPC_Tunning_Val;

			if(copy_from_user((void *)&tCPC_Tunning_Val, (const void __user *)arg, sizeof(VBE_DISP_CPC_TUNNING_VAL_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETOLEDCPCTUNNING 1 failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetOLEDCPC_Tunning(&tCPC_Tunning_Val);
			}
			break;
		}
		case VBE_IOC_SETOLEDGSR:
		{

			UINT32 GSRTbl;

			if(copy_from_user((void *)&GSRTbl, (const void __user *)arg, sizeof(UINT32)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETOLEDGSR 1 failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetOLEDGSR(&GSRTbl);
			}
			break;
		}
		case VBE_IOC_MPLUSSET:
		{

#if defined(CONFIG_COMPAT) && defined(CONFIG_ARM64)
			VBE_DISP_MPLUS_REGISTER_T mPlusRegStructSet = {0, 0};
#else
			VBE_DISP_MPLUS_REGISTER_T mPlusRegStructSet = {(void *)NULL, 0};
#endif

			if(copy_from_user((void *)&mPlusRegStructSet, (const void __user *)(arg), sizeof(VBE_DISP_MPLUS_REGISTER_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_MPLUSSET struct copy from user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				UINT16 regSize = 0;

				if(Get_DISPLAY_PANEL_MPLUS_RGBW() == 1) // M+
				{
					regSize = 928/2;
          rtd_pr_vbe_debug(" === [VBE_IOC_MPLUSSET] Panel is made by LGD\n");
				}
	      else if(Get_DISPLAY_PANEL_BOW_RGBW() == 1) // BOE SiW
	      {
					regSize = 256;
          rtd_pr_vbe_debug(" === [VBE_IOC_MPLUSSET] Panel is made by B-O-E\n");
	      }
				else
				{
					regSize = 256;
          rtd_pr_vbe_debug("=== [VBE_IOC_MPLUSSET] Doesn't support this panel\n");
				}

				if(copy_from_user(Mplus_Value, (const void __user *)((unsigned long)mPlusRegStructSet.pRegisterSet), regSize*sizeof(UINT16)))
				{
            rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_MPLUSSET pointer copy from user failed!!!!!!!!!!!!!!!\n");
            break;
				}
				else
  					HAL_VBE_DISP_MplusSet(Mplus_Value, mPlusRegStructSet.nPanelMaker);
			}
			break;
		}
		case VBE_IOC_MPLUSGET:
		{
#if defined(CONFIG_COMPAT) && defined(CONFIG_ARM64)
			VBE_DISP_MPLUS_REGISTER_T mPlusRegStructSet = {0, 0};
#else
			VBE_DISP_MPLUS_REGISTER_T mPlusRegStructSet = {(void *)NULL, 0};
#endif

			if(copy_from_user((void *)&mPlusRegStructSet, (const void __user *)(arg), sizeof(VBE_DISP_MPLUS_REGISTER_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_MPLUSGET copy from user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				UINT16 regSize = 0;
        int i;

        HAL_VBE_DISP_MplusGet(Mplus_Value, mPlusRegStructSet.nPanelMaker);

				if(Get_DISPLAY_PANEL_MPLUS_RGBW() == 1) // M+
				{
					regSize = 928>>1;

          // match LG format
          // Mplus_Value[0] = 0x1234;// --> print 0x34, 0x12
          for(i=0;i<regSize;i++)
          {
            Mplus_Value[i] = (Mplus_Value[i*2+1]<<8) + Mplus_Value[i*2];
          }

          rtd_pr_vbe_debug(" === [VBE_IOC_MPLUSGET] Panel is made by LGD\n");
				}
				else if(Get_DISPLAY_PANEL_BOW_RGBW() == 1) // BOE SiW
				{
					regSize = 256;

          // merge 8bits to 16 bits
          for(i=0;i<regSize/2;i++)
          {
            Mplus_Value[i] = (Mplus_Value[i*2]<<8) + Mplus_Value[i*2+1];
          }
          rtd_pr_vbe_debug(" === [VBE_IOC_MPLUSGET] Panel is made by B-O-E\n");
				}
				else // default
				{
					regSize = 256;
          rtd_pr_vbe_debug("=== [VBE_IOC_MPLUSGET] Doesn't support this panel\n");
				}

        //rtd_pr_vbe_info("[henry] VBE_IOC_MPLUSGET, nPanelMaker=%d\n", mPlusRegStructSet.nPanelMaker);
        //rtd_pr_vbe_info("[henry] VBE_IOC_MPLUSGET, %x, %x\n", Mplus_Value[0], Mplus_Value[1]);
        //rtd_pr_vbe_info("[henry] VBE_IOC_MPLUSGET, return16=%x\n", *( (UINT16 *)mPlusRegStructSet.pRegisterSet));
        //rtd_pr_vbe_info("[henry] VBE_IOC_MPLUSGET, return8=%x\n", *( (UINT8 *)mPlusRegStructSet.pRegisterSet));
        //rtd_pr_vbe_info("[henry] VBE_IOC_MPLUSGET, global array addr=%p\n", Mplus_Value);
        //rtd_pr_vbe_info("[henry] VBE_IOC_MPLUSGET, return addr=%p\n", mPlusRegStructSet.pRegisterSet);


        if(copy_to_user(to_user_ptr(mPlusRegStructSet.pRegisterSet), (void *)Mplus_Value, regSize*sizeof(UINT16)))
  			{
  				  retval = -EFAULT;
    				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_MPLUSGET copy to user failed!!!!!!!!!!!!!!!\n");
  			}
			}
			break;

		}

		case VBE_IOC_GET_OSD_ALPHA_APL:
		{
#if defined(CONFIG_COMPAT) && defined(CONFIG_ARM64)
			VBE_DISP_OSD_ALPHA_APL_T osdAlphaAPL = {0, 0};
#else
			VBE_DISP_OSD_ALPHA_APL_T osdAlphaAPL = {(UINT32 *)NULL, 0};
#endif
			HAL_VBE_DISP_GetOSDAlphaAPL(to_user_ptr(osdAlphaAPL.pAlphaTbl), osdAlphaAPL.size);

			if(copy_to_user(to_user_ptr(arg), (void *)&osdAlphaAPL, sizeof(VBE_DISP_OSD_ALPHA_APL_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_GET_OSD_ALPHA_APL failed!!!!!!!!!!!!!!!\n");
			}
			break;

		}
		case VBE_IOC_SET_OSD_RGB_LEVEL:
		{

			VBE_DISP_OSD_RGB_LEVEL_T osdRGBLevel;

			if(copy_from_user((void *)&osdRGBLevel, (const void __user *)arg, sizeof(VBE_DISP_OSD_RGB_LEVEL_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SET_OSD_RGB_LEVEL failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetOSDRGBLevel(osdRGBLevel.bOnOff, to_user_ptr(osdRGBLevel.pLvlTbl), osdRGBLevel.size);
			}
			break;
		}


		case VBE_IOC_SETEPIDATASCRAMBLE:
		{
			BOOLEAN bOnOFF;

			if(copy_from_user((void *)&bOnOFF, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETEPIDATASCRAMBLE 1 failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetEPIDataScramble(bOnOFF);
			}
			break;
		}
		case VBE_IOC_SETADVANCED10BIT:
		{
			BOOLEAN bOnOFF;

			if(copy_from_user((void *)&bOnOFF, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SETADVANCED10BIT 1 failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_VBE_DISP_SetAdvanced10bit(bOnOFF);
			}
			break;
		}

		case V3D_IOC_INITIALIZE:
		{
			HAL_V3D_Initialize();
			break;
		}
		case V3D_IOC_SET3DFMT:
		{
			V3D_Set3DFMT_T v3d_set3dfmt_param;
			if(copy_from_user((void *)&v3d_set3dfmt_param, (const void __user *)arg, sizeof(V3D_Set3DFMT_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=V3D_IOC_SET3DFMT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_V3D_Set3DFMT(v3d_set3dfmt_param.wid, v3d_set3dfmt_param.in_fmt, v3d_set3dfmt_param.out_fmt);
			}
			break;
		}
		case V3D_IOC_SETLRSEQ:
		{
			V3D_LR_SEQ_T type;
			if(copy_from_user((void *)&type, (const void __user *)arg, sizeof(V3D_LR_SEQ_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=V3D_IOC_SETLRSEQ failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_V3D_SetLRSeq(type);
			}
			break;
		}
		case V3D_IOC_CTRLCONVERGENCE:
		{
			INT8 s8ConvValue;
			if(copy_from_user((void *)&s8ConvValue, (const void __user *)arg, sizeof(INT8)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=V3D_IOC_CTRLCONVERGENCE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_V3D_CtrlConvergence(s8ConvValue);
			}
			break;
		}
		case V3D_IOC_CTRLDEPTH:
		{
			UINT8 u8DepthLevel;
			if(copy_from_user((void *)&u8DepthLevel, (const void __user *)arg, sizeof(UINT8)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=V3D_IOC_CTRLDEPTH failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_V3D_CtrlDepth(u8DepthLevel);
			}
			break;
		}
		case V3D_IOC_SETAUTOCONVENABLE:
		{
			BOOLEAN bOnOff;
			if(copy_from_user((void *)&bOnOff, (const void __user *)arg, sizeof(BOOLEAN)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=V3D_IOC_SETAUTOCONVENABLE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_V3D_SetAutoConvEnable(bOnOff);
			}
			break;
		}
		case V3D_IOC_GETAUTODETECTINGRESULT:
		{
			V3D_FMT_TYPE_T pVideo3Dtype;
			if(copy_from_user((void *)&pVideo3Dtype, (const void __user *)arg, sizeof(V3D_FMT_TYPE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=V3D_IOC_GETAUTODETECTINGRESULT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_V3D_GetAutoDetectingResult(&pVideo3Dtype);
			}
			break;
		}
		case V3D_IOC_SETAUTODETECTINGTYPE:
		{
			V3D_FORMAT_TYPE_T mode;
			if(copy_from_user((void *)&mode, (const void __user *)arg, sizeof(V3D_FORMAT_TYPE_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=V3D_IOC_SETAUTODETECTINGTYPE failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				HAL_V3D_SetAutoDetectingType(mode);
			}
			break;
		}
		case VBE_IOC_PANEL_MODE_SET:
		{
			KADP_PANEL_MODE_SET_T panel_set;

			if(copy_from_user((void *)&panel_set, (const void __user *)arg, sizeof(KADP_PANEL_MODE_SET_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_notice("scaler vbe ioctl code=VBE_IOC_PANEL_POWER_ONOFF failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				rtd_pr_vbe_notice("panel_set.ctrl_set = %d, enable = %d \n", panel_set.ctrl_set, panel_set.power_enable);
				if (panel_set.ctrl_set == KADP_DISP_PANEL_STR_BACKLIGHT_CTRL_BY_AP) {
					Panel_Set_STR_BackLight_Ctrl_Mode(panel_set.power_enable);
				}
			}
			break;
		}
		case VBE_IOC_DISP_DATAFRAMESYNC:
		{
			BOOLEAN enable;
			if(copy_from_user((void *)&enable, (const void __user *)arg, sizeof(BOOLEAN))) {
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_DATAFRAMESYNC failed!!!!!!!!!!!!!!!\n");
			} else {
				HAL_VBE_DISP_DataFrameSync(enable);
			}
			break;
		}
		case VBE_IOC_DISP_ADAPTIVESTREAM_FRAMERATE:
		{
			unsigned int framerate;
			if(copy_from_user((void *)&framerate, (const void __user *)arg, sizeof(unsigned int))) {
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_DISP_ADAPTIVESTREAM_FRAMERATE failed!!!!!!!!!!!!!!!\n");
			} else {
				if(vbe_disp_get_adaptivestream_fs_mode() == 1)
					vbe_disp_set_adaptivestream_force_framerate(framerate);
			}
			break;
		}

		case VBE_IOC_SET_FORCEBG_COLOR:
		{
			FORCEBG_COLOR_CTRL_TYPE bgcolortype;
			if(copy_from_user((void *)&bgcolortype, (const void __user *)arg, sizeof(FORCEBG_COLOR_CTRL_TYPE))) {
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SET_FORCEBG_COLOR failed!!!!!!!!!!!!!!!\n");
			} else {
				drvif_scalerdisplay_set_bg_color((unsigned char )bgcolortype.wid, _DISPLAY_BG, ((bgcolortype.COLORRGB >> 16) & 0xFF) << 6 , ((bgcolortype.COLORRGB >> 8) & 0xFF) << 6 , (bgcolortype.COLORRGB  & 0xFF) << 6 );

			}
			break;
		}

		case VBE_IOC_SET_PANEL_SFG:
		{
			unsigned char enable;
			if(copy_from_user((void *)&enable, (const void __user *)arg, sizeof(unsigned char))) {
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SET_PANEL_FG failed!!!!!!!!!!!!!!!\n");
			} else {
				Scaler_ForcePanelBg(enable);

			}
			break;
		}

		case VBE_IOC_SET_PANEL_FORCEBG_COLOR:
		{
			unsigned int panel_bg_color;
			if(copy_from_user((void *)&panel_bg_color, (const void __user *)arg, sizeof(unsigned int))) {
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SET_PANEL_FORCEBG_COLOR failed!!!!!!!!!!!!!!!\n");
			} else {
				drvif_scalerdisplay_set_panel_bg_color(((panel_bg_color >> 16) & 0xFF) << 2 , ((panel_bg_color >> 8) & 0xFF) << 2 , (panel_bg_color  & 0xFF) << 2 );

			}
			break;
		}
		case VBE_IOC_SET_SWING:
		{
			KADP_VBE_DISP_LVDS_T lvds_info;
			if(copy_from_user((void *)&lvds_info, (const void __user *)arg, sizeof(KADP_VBE_DISP_LVDS_T))) {
				retval = -EFAULT;
				rtd_pr_vbe_info("scaler vbe ioctl code=VBE_IOC_SET_SWING failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(drvif_factory_set_lvds_current(&lvds_info) == 0)
				{
					retval = -EFAULT;
					break;
				}
				rtd_pr_vbe_notice("scaler vbe ioctl code=VBE_IOC_SET_SWING success!!!!!!!!!!!!!!!\n");
			}
			break;
		}
		case VBE_IOC_GET_SWING:
		{
			KADP_VBE_DISP_LVDS_T lvds_info;
			if(copy_from_user((void *)&lvds_info, (const void __user *)arg, sizeof(KADP_VBE_DISP_LVDS_T))) {
				retval = -EFAULT;
				rtd_pr_vbe_info("scaler vbe ioctl code=VBE_IOC_GET_SWING failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(drvif_factory_get_lvds_current(&lvds_info) == 0)
				{
					retval = -EFAULT;
					break;
				}

				if(copy_to_user((void __user *)arg, (const void  *)&lvds_info, sizeof(KADP_VBE_DISP_LVDS_T))) {
					retval = -EFAULT;
					rtd_pr_vbe_notice("scaler vbe ioctl code=VBE_IOC_GET_SWING copy failed!!!!!!!!!!!!!!!\n");
				}
				else
				{
					rtd_pr_vbe_notice("scaler vbe ioctl code=VBE_IOC_GET_SWING success!!!!!!!!!!!!!!!\n");
				}
			}
			break;
		}
		case VBE_IOC_SET_EMPHASIS_CURRENT:
		{
			KADP_VBE_DISP_Emphasis_T emp_info;
			if(copy_from_user((void *)&emp_info, (const void __user *)arg, sizeof(KADP_VBE_DISP_Emphasis_T)))
			{
				retval = -EFAULT;
				rtd_pr_vbe_info("scaler vbe ioctl code=VBE_IOC_SET_EMPHASIS_CURRENT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(drvif_factory_set_pre_emphasis_current(&emp_info) == 0)
				{
					retval = -EFAULT;
					break;
				}
				rtd_pr_vbe_notice("scaler vbe ioctl code=VBE_IOC_SET_EMPHASIS_CURRENT success!!!!!!!!!!!!!!!\n");
			}
			break;
		}
		case VBE_IOC_GET_EMPHASIS_CURRENT:
		{
			KADP_VBE_DISP_Emphasis_T emp_info;
			if(copy_from_user((void *)&emp_info, (const void __user *)arg, sizeof(KADP_VBE_DISP_Emphasis_T))) {
				retval = -EFAULT;
				rtd_pr_vbe_info("scaler vbe ioctl code=VBE_IOC_GET_EMPHASIS_CURRENT failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(drvif_factory_get_pre_emphasis_current(&emp_info) == 0)
				{
					retval = -EFAULT;
					break;
				}

				if(copy_to_user((void __user *)arg, (const void  *)&emp_info, sizeof(KADP_VBE_DISP_Emphasis_T)))
				{
					retval = -EFAULT;
					rtd_pr_vbe_notice("scaler vbe ioctl code=VBE_IOC_GET_EMPHASIS_CURRENT copy failed!!!!!!!!!!!!!!!\n");
				}
				else
				{
					rtd_pr_vbe_notice("scaler vbe ioctl code=VBE_IOC_GET_EMPHASIS_CURRENT success!!!!!!!!!!!!!!!\n");
				}
			}
			break;
		}
		case VBE_IOC_SET_VCOMPatternDraw:
		{
			unsigned char	*m_pCacheStartAddr = NULL;
			VBE_DISP_VCOM_PATTERN_DRAW_T dispVcomPatDraw;

			if(copy_from_user((void *)&dispVcomPatDraw, (const void __user *)arg, sizeof(VBE_DISP_VCOM_PATTERN_DRAW_T))) {
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SET_VCOMPatternDraw failed!!!!!!!!!!!!!!!\n");
			} else {

				m_pCacheStartAddr = (unsigned char *)dvr_malloc(dispVcomPatDraw.nSize*sizeof(UINT16));
				if(m_pCacheStartAddr == NULL){
					#ifndef CONFIG_ARM64
					rtd_pr_vbe_err("[ERROR]VBE_IOC_SET_VCOMPatternDraw Allocate size=%x fail\n",dispVcomPatDraw.nSize*sizeof(UINT16));
					#endif
					return FALSE;
				}

				//get vcom pattern
				if(copy_from_user((void *)m_pCacheStartAddr, (const void __user *)to_user_ptr(dispVcomPatDraw.u16pVcomPattern), dispVcomPatDraw.nSize*sizeof(UINT16)))
				{
					retval = -EFAULT;
					rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SET_VCOMPatternDraw  vcom pattern failed!!!!!!!!!!!!!!!\n");
				}
				HAL_VBE_DISP_VCOMPatternDraw((UINT16*)m_pCacheStartAddr, dispVcomPatDraw.nSize);

				if(m_pCacheStartAddr){
					dvr_free((void *)m_pCacheStartAddr);
				}

			}
			break;
		}
		case VBE_IOC_SET_VCOMPatternCtrl:
		{
			KADP_DISP_PANEL_VCOM_PAT_CTRL_T vcomCtrl;

			if(copy_from_user((void *)&vcomCtrl, (const void __user *)arg, sizeof(KADP_DISP_PANEL_VCOM_PAT_CTRL_T))) {
				retval = -EFAULT;
				rtd_pr_vbe_debug("scaler vbe ioctl code=VBE_IOC_SET_VCOMPatternCtrl failed!!!!!!!!!!!!!!!\n");
			} else {
				HAL_VBE_DISP_VCOMPatternCtrl(vcomCtrl);
			}
			break;
		}
        case VBE_IOC_SET_PANEL_TI_MODE:
        {
            unsigned char tiMode;

            if (copy_from_user((void *)&tiMode, (const void __user *)arg, sizeof(unsigned char)))
            {
                retval = -EFAULT;
                rtd_pr_vbe_debug("scaler vbe ioctl code = VBE_IOC_SET_PANEL_TI_MODE failed!!!!!!!!!!!!!!!\n");
            }
            else
            {
                HAL_VBE_DISP_SetPanelTiMode(tiMode);
            }
            break;
        }
        case VBE_IOC_GET_PANEL_TI_MODE:
        {
            unsigned char tiMode;

            HAL_VBE_DISP_GetPanelTiMode(&tiMode);
            if (copy_to_user((void __user *)arg, (void *)&tiMode, sizeof(unsigned char)))
            {
                retval = -EFAULT;
                rtd_pr_vbe_debug("scaler vbe ioctl code = VBE_IOC_GET_PANEL_TI_MODE failed!!!!!!!!!!!!!!!\n");
            }
            break;
        }
        case VBE_IOC_SET_PANEL_BIT_MODE:
        {
            unsigned char bitMode;

            if (copy_from_user((void *)&bitMode, (const void __user *)arg, sizeof(unsigned char)))
            {
                retval = -EFAULT;
                rtd_pr_vbe_debug("scaler vbe ioctl code = VBE_IOC_SET_PANEL_BIT_MODE failed!!!!!!!!!!!!!!!\n");
            }
            else
            {
                HAL_VBE_DISP_SetPanelBitMode(bitMode);
            }
            break;
        }
        case VBE_IOC_GET_PANEL_BIT_MODE:
        {
            unsigned char bitMode;

            HAL_VBE_DISP_GetPanelBitMode(&bitMode);
            if (copy_to_user((void __user *)arg, (void *)&bitMode, sizeof(unsigned char)))
            {
                retval = -EFAULT;
                rtd_pr_vbe_debug("scaler vbe ioctl code = VBE_IOC_GET_PANEL_BIT_MODE failed!!!!!!!!!!!!!!!\n");
            }
            break;
        }
        case VBE_IOC_SET_SGF_DUAL_MODE:
        {
            unsigned char dualMode;

            if (copy_from_user((void *)&dualMode, (const void __user *)arg, sizeof(unsigned char)))
            {
                retval = -EFAULT;
                rtd_pr_vbe_debug("scaler vbe ioctl code = VBE_IOC_SET_SGF_DUAL_MODE failed!!!!!!!!!!!!!!!\n");
            }
            else
            {
                HAL_VBE_DISP_SetPanelSFGDualMode(dualMode);
            }
            break;
        }
        case VBE_IOC_GET_SGF_DUAL_MODE:
        {
            unsigned char dualMode;

            HAL_VBE_DISP_GetPanelSFGDualMode(&dualMode);
            if (copy_to_user((void __user *)arg, (void *)&dualMode, sizeof(unsigned char)))
            {
                retval = -EFAULT;
                rtd_pr_vbe_debug("scaler vbe ioctl code = VBE_IOC_GET_SGF_DUAL_MODE failed!!!!!!!!!!!!!!!\n");
            }
            break;
        }
#ifdef CONFIG_DYNAMIC_PANEL_SELECT
        case VBE_IOC_SET_DLG_MODE:
        {
            BOOLEAN dlgEnable;
            if(copy_from_user((void *)&dlgEnable, (const void __user *)arg, sizeof(BOOLEAN)))
            {
                retval = -EFAULT;
                rtd_pr_vbe_emerg("scaler vsc ioctl code=VSC_IOC_SET_DLG_MODE copy_from_user failed!!!!!!!!!!!!!!!\n");
            }
            else
            {
                HAL_VBE_Panel_Set_DlgEnable(dlgEnable);
                if(vbe_check_dlg_mode_change())
                {
                    dlg_callback_handler(E_DLG_ENABLE_DLG_MASK);
                }
                else
                {
                    dlg_callback_handler(E_DLG_DISABLE_DLG_MASK);
                }
            }

            break;
        }

#endif
        case VBE_IOC_GET_PANEL_INFO:
        {
            KADP_DISP_PANEL_INFO_T dispPanelInfo = {0};

            HAL_DISP_PANEL_GetPanelInfo(&dispPanelInfo);

            if (copy_to_user((void __user *)arg, (void *)&dispPanelInfo, sizeof(KADP_DISP_PANEL_INFO_T)))
            {
                retval = -EFAULT;
                rtd_pr_vbe_err("scaler vbe ioctl code = VBE_IOC_GET_PANEL_BIT_MODE failed!!!!!!!!!!!!!!!\n");
            }
            break;
        }
		default:
			retval = -EFAULT;
			rtd_pr_vbe_debug("Scaler vbe disp: ioctl code = %d is invalid!!!!!!!!!!!!!!!1\n", cmd);
			break ;
		}
	return retval;
}

#if defined(CONFIG_COMPAT) && defined(CONFIG_ARM64)
long vbe_compat_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	return vbe_ioctl(file,cmd,arg);
}
#endif

struct file_operations vbe_fops= {
	.owner =    THIS_MODULE,
	.open  =    vbe_open,
	.release =  vbe_release,
	.read  =    vbe_read,
	.write = 	vbe_write,
	.unlocked_ioctl =    vbe_ioctl,
#if defined(CONFIG_COMPAT) && defined(CONFIG_ARM64)
  	.compat_ioctl = vbe_compat_ioctl,
#endif
};

#ifdef CONFIG_PM
static const struct dev_pm_ops vbe_pm_ops =
{
	.suspend    = vbe_suspend_instanboot,
	.resume     = vbe_resume_instanboot,
	.complete	= vbe_resume_Backlight_TurnOn,
#ifdef CONFIG_HIBERNATION
	.freeze 	= vbe_suspend,
	.thaw 		= vbe_resume,
	.poweroff 	= vbe_suspend,
	.restore 	= vbe_resume,
#endif
};
#endif // CONFIG_PM
#endif
static bool vbe_isr_registered = false;
bool is_veb_isr_registered(void)
{
	return vbe_isr_registered;
}

#ifndef BUILD_QUICK_SHOW

static int vbe_probe(struct platform_device *pdev)
{
	int irq = 0, ret = 0;

	if (Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2() ||
		Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3()) {

		irq = platform_get_irq(pdev, 0);
		if (irq < 0) {
			rtd_pr_vbe_emerg("[vbe_timer_display_interrupt]: can't get irq. ret=%d\n", irq);
			/* ignore if fail */
			goto done;
		}

		/* disable timer first to prevent unexpected event before we're ready */
		create_timer(7, TIMER_CLOCK/4000, COUNTER); // 25 us
		//Disable Interrupt
		rtk_timer_control(7, HWT_INT_DISABLE);
		// disable timer7
		IoReg_Write32(TIMER_TC7CR_reg, 0);
		// write 1 clear
		IoReg_Write32(TIMER_ISR_reg, TIMER_ISR_tc7_int_mask);

		ret = request_irq(irq, vbe_timer_display_interrupt,
				IRQF_SHARED, "timer_dv", vbe_timer_display_interrupt);
		if (ret)
			rtd_pr_vbe_emerg("[vbe_timer_display_interrupt]: can't get assigned irq:%lu\n",
					irq_get_irq_data(irq)->hwirq);
		else
			vbe_isr_registered = true;
	}

done:
	return 0;
}


static int vbe_remove(struct platform_device *pdev)
{
	int irq;

	if (Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2() ||
		Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3()) {

		irq = platform_get_irq(pdev, 0);
		if (irq >= 0) {
			/* disable timer first to prevent unexpected event after free irq */
			create_timer(7, TIMER_CLOCK/4000, COUNTER); // 25 us
			//Disable Interrupt
			rtk_timer_control(7, HWT_INT_DISABLE);
			// disable timer7
			IoReg_Write32(TIMER_TC7CR_reg, 0);
			// write 1 clear
			IoReg_Write32(TIMER_ISR_reg, TIMER_ISR_tc7_int_mask);

			vbe_isr_registered = false;
			free_irq(irq, (void *)vbe_timer_display_interrupt);
		}
	}

	return 0;
}


static const struct of_device_id vbe_of_match[] = {
	{
		.compatible = "realtek,vbe",
	},
	{},
};

MODULE_DEVICE_TABLE(of, vbe_of_match);


static struct platform_driver vbe_device_driver = {
	.probe = vbe_probe,
	.remove = vbe_remove,
	.driver =
	{
        .name         = VBE_DEVICE_NAME,
        .bus          = &platform_bus_type,
#ifdef CONFIG_PM
		.pm         = &vbe_pm_ops,
#endif
		.of_match_table = of_match_ptr(vbe_of_match),
	},
};

static char *vbe_devnode(struct device *dev, umode_t *mode)
{
	return NULL;
}
#ifdef CONFIG_SUPPORT_SCALER_MODULE
// FIXME : early param does not supported for kernel module, please use another way to instead
#else
static int __init rtk_panel_get_pmic(char *cmdline)
{

    static char *mode;
	UINT32 pmic_mode;
    mode = cmdline;
    if(cmdline == NULL){
		rtd_pr_vbe_emerg("rtk_panel_get_pmic cmd=NULL\n");
    }else{
        pmic_mode = simple_strtoul(mode, &mode, 10);
		rtd_pr_vbe_emerg("rtk_panel_get_pmic : pmic_mode=%d !!!\n", pmic_mode);
		Set_PANEL_PMIC_Type(pmic_mode);
    }
    return 0;
}
early_param("pmic_setting_index", rtk_panel_get_pmic);
#endif

#define S_OK		0x10000000
void rpc_send_debug_cmd(const char *pattern, int length)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long ret = S_OK;
#endif
	unsigned long vir_addr;
	unsigned long phy_addr;
	char *command = NULL;

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(length+1, GFP_DCU1_LIMIT, (void **)&command);
	if (!vir_addr) {
		rtd_pr_vbe_err("[%s %d]alloc string memory fail\n",__FUNCTION__,__LINE__);
		return;
	}
	phy_addr = (unsigned long)dvr_to_phys((void *)vir_addr);
	memset((void *)command, '\0', length+1);
	memcpy((void *)command, (void *)pattern, length);

	rtd_pr_vbe_debug("rtkvdec: debug string(%s), length(%d)\n", command, length);
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	if (send_rpc_command(RPC_VIDEO, RPC_VCPU_DEBUG_COMMAND, phy_addr, length, &ret))
		rtd_pr_vbe_err("rtkvdec: debug string(%s) fail %d\n", pattern, (int)ret);
#endif
	dvr_free((void *)vir_addr);
	return;
}

int vbe_module_init(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_PWM)
	extern void register_get_disp_htotal_ptr(void* fun_ptr);
	extern void register_get_disp_vtotal_ptr(void* fun_ptr);
	extern void register_get_disp_dclk_ptr(void* fun_ptr);
	extern void register_get_disp_refresh_rate_ptr(void* fun_ptr);
#endif

	int result;
	int vbe_devno;
	char cmd_buf[25];
	dev_t devno = 0;//vbe device number
	PANEL_CONFIG_PARAMETER *panel_parameter = NULL;
	ppoverlay_memc_mux_ctrl_RBUS ppoverlay_memc_mux_ctrl_reg;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result_rpc =0;
#endif
	VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE *config_data;
	unsigned long vir_addr, vir_addr_noncache;
	unsigned int phy_addr;

	result = alloc_chrdev_region(&devno, vbe_minor, vbe_nr_devs,VBE_DEVICE_NAME);
	vbe_major = MAJOR(devno);
	if (result < 0) {
		rtd_pr_vbe_warn("VBE_DEVICE: can't get major %d\n", vbe_major);
		return result;
	}

	rtd_pr_vbe_debug("VBE_DEVICE init module major number = %d\n", vbe_major);
	vbe_devno = MKDEV(vbe_major, vbe_minor);

	vbe_class = class_create(THIS_MODULE, VBE_DEVICE_NAME);

	if (IS_ERR(vbe_class))
	{
		rtd_pr_vbe_debug("scalevbe: can not create class...\n");
	    result=PTR_ERR(vbe_class);
		goto fail_class_create;
	}
	vbe_class->devnode = vbe_devnode;

	result = platform_driver_register(&vbe_device_driver);
	if (result) {
		rtd_pr_vbe_emerg("%s: can not register platform driver, ret=%d\n", __func__, result);
    	goto fail_platform_driver_register;
	}

	cdev_init(&vbe_cdev, &vbe_fops);
	vbe_cdev.owner = THIS_MODULE;
   	vbe_cdev.ops = &vbe_fops;
	result = cdev_add (&vbe_cdev, vbe_devno, 1);
	if (result)
	{
		rtd_pr_vbe_notice("Error %d adding VBE_DEVICE!\n", result);
		goto fail_cdev_init;
	}
	device_create(vbe_class, NULL, MKDEV(vbe_major, 0), NULL,VBE_DEVICE_NAME);
	sema_init(&VBE_Semaphore, 1);
	sema_init(&VBE_LowDelay_Semaphore, 1);
	sema_init(&VBE_RGBW_Resume_Semaphore, 1);
	panel_parameter =  (PANEL_CONFIG_PARAMETER *)&platform_info.panel_parameter;
	Panel_InitParameter(panel_parameter);
	Panel_LVDS_To_Hdmi_Converter_Parameter();
	modestate_decide_pixel_mode();
	if (Get_PANEL_VFLIP_ENABLE()) {
		vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE), GFP_DCU1_LIMIT, (void**)&vir_addr_noncache);
		//phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
		phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);
		config_data = (VIDEO_RPC_VOUT_VODMA_VFLIP_ENABLE *)vir_addr_noncache;
		config_data->videoPlane = VO_VIDEO_PLANE_V1;
		config_data->enable =Get_PANEL_VFLIP_ENABLE();
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
		if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_ConfigVodmaVFlip,phy_addr, 0, &result_rpc))
		{
			rtd_pr_vbe_debug("RPC fail!!\n");
		}
#endif
		dvr_free((void *)vir_addr);
	}

#if 1 // Eric@20180807 HDMI converter board don't enable film mode change flow
	if((Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI)|| (Get_DISPLAY_PANEL_TYPE() == P_LVDS_TO_HDMI)){
		strcpy(cmd_buf, "hevc_film_mode_manual=1");
	    rpc_send_debug_cmd(cmd_buf,23);
		strcpy(cmd_buf, "hevc_film_mode=0");
	    rpc_send_debug_cmd(cmd_buf,16);
	}
#endif

	ppoverlay_memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	if(ppoverlay_memc_mux_ctrl_reg.memcdtg_golden_vs == 1){
		vbe_disp_set_dynamic_memc_bypass_flag(TRUE);
	}else{
		vbe_disp_set_dynamic_memc_bypass_flag(FALSE);
	}
#ifdef CONFIG_DYNAMIC_PANEL_SELECT
    vbe_dlg_init(dlg_callback_handler);
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_PWM)
	register_get_disp_htotal_ptr(Get_DISP_HORIZONTAL_TOTAL);//register Get_DISP_HORIZONTAL_TOTAL for pwm driver 
	register_get_disp_vtotal_ptr(Get_DISP_VERTICAL_TOTAL);//register Get_DISP_VERTICAL_TOTAL for pwm driver 
	register_get_disp_dclk_ptr(Get_DISPLAY_CLOCK_TYPICAL);//register Get_DISPLAY_CLOCK_TYPICAL for pwm driver 
	register_get_disp_refresh_rate_ptr(Get_DISPLAY_REFRESH_RATE);//register Get_DISPLAY_REFRESH_RATE for pwm driver 
#endif

	return 0;//Success

fail_cdev_init:
	platform_driver_unregister(&vbe_device_driver);
fail_platform_driver_register:
	class_destroy(vbe_class);
fail_class_create:
	vbe_class = NULL;
	unregister_chrdev_region(vbe_devno, vbe_nr_devs);
	return result;
}

void __exit vbe_module_exit(void)
{
	dev_t devno = MKDEV(vbe_major, vbe_minor);
	rtd_pr_vbe_info("vbe clean module vbe_major = %d\n", vbe_major);

  	device_destroy(vbe_class, MKDEV(vbe_major, 0));
  	class_destroy(vbe_class);
	vbe_class=NULL;
	cdev_del(&vbe_cdev);

	platform_driver_unregister(&vbe_device_driver);

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, vbe_nr_devs);
}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(vbe_module_init);
module_exit(vbe_module_exit);
#endif
#endif
