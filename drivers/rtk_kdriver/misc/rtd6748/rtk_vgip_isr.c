//Copyright (C) 2007-2013 Realtek Semiconductor Corporation.
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
//#include <asm/io.h>
#include <linux/uaccess.h>

#ifdef CONFIG_OF
#include <linux/of_irq.h>
#include <linux/of_address.h>
#endif

#include "rtk_vgip_isr.h"
#include <rbus/vgip_reg.h>
#include <rbus/sub_vgip_reg.h>
#include <rbus/dma_vgip_reg.h>
#include <rbus/h3ddma_reg.h>
#include <rbus/onms_reg.h>
#include <rbus/mdomain_cap_reg.h>
#include <rbus/pinmux_reg.h>
#include <rbus/gpio_reg.h>

#include "memc_isr/Driver/reg_offset.h"
//#include "memc_isr/Driver/KI7416_reg.h"
#include "vgip_isr/scalerVIP.h"
#include "vgip_isr/scalerVideo.h"
#include "vgip_isr/scalerDCC.h"
#include "vgip_isr/scalerDI.h"
#include "rbus/sb2_reg.h"
//#include "rbus/tve_reg.h"
//#include "rbus/tve_dma_reg.h"
#include <rbus/mdomain_cap_reg.h>
#include <rbus/mdomain_disp_reg.h>
#include <rbus/pst_i2rnd_reg.h>
#include <rbus/timer_reg.h>//for sdnr line count debug
//#include "rbus/crt_reg.h"
//#include "rbus/pll_reg_reg.h"
//#include "rbus/ppoverlay_reg.h"
#include "tvscalercontrol/scaler/scalerstruct.h"
#include "tvscalercontrol/vip/scalerColor.h"
#include "tvscalercontrol/vip/scalerColor_tv006.h"
#include "tvscalercontrol/vip/localcontrast.h"//juwen, LC, APL
#include "tvscalercontrol/vip/vip_reg_def.h"
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/i3ddma/i3ddma.h>
#include <vo/rtk_vo.h>
#include <rtd_log/rtd_module_log.h>
#include <tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>
//#include <rtk_kdriver/RPCDriver.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
//Add game mode measurement @Crixus 20170621
#ifdef CONFIG_I3DDMA_PATTERN_MEASUREMENT
#include <rbus/dma_vgip_ptg_reg.h>
#include <rbus/vgip_ptg_reg.h>
#include <rbus/h3ddma_rgb2yuv_dither_4xxto4xx_reg.h>
#include <ioctrl/scaler/vsc_cmd_id.h>
#endif
#include <rbus/mdomain_vi_sub_reg.h>
#include <rbus/sfg_reg.h>
#include <rbus/scaleup_reg.h>
#include <rbus/software1_reg.h>
#include <mach/platform.h>
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

//#define SDNR_LINECNT_DEBUG//for ENC RL6557-451

#ifdef CONFIG_I2RND_ENABLE
#include <rbus/vodma_reg.h>
#include <tvscalercontrol/scalerdrv/scaler_i2rnd.h>
#endif
#ifdef CONFIG_PST_ENABLE
#include <tvscalercontrol/scalerdrv/scaler_pst.h>
#endif
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#include <tvscalercontrol/hdmirx/hdmi_vfe.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
#define NO_DVS_INFO_MAX 6
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// FixMe, 20190919
//#include "vgip_isr/scalerAI.h"

#define Seamless_Freeze_Frames_Max 15

/* Function Prototype */
static int vgip_isr_suspend(struct platform_device *dev, pm_message_t state);
static int vgip_isr_resume(struct platform_device *dev);
static long vgip_isr_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int vgip_isr_remove(struct platform_device *pdev);
extern unsigned char get_MEMC_bypass_status_refer_platform_model(void);
extern UINT8 drv_memory_Get_multibuffer_flag(void);
extern UINT8 drv_memory_Get_capture_multibuffer_flag(void);
extern void drv_memory_Set_multibuffer_number_pre(UINT8 number);
extern UINT8 drv_memory_Get_multibuffer_number(void);
extern UINT8 drv_memory_Get_multibuffer_number_pre(void);
extern void drv_memory_capture_multibuffer_switch_buffer_number(void);
extern void drv_memory_display_multibuffer_switch_buffer_number(void);
extern enum PANEL_RES_TYPE get_panel_res(void);

extern int updateVODMASetting(unsigned int);
extern void scalerVIP_film_FWmode(void);
extern void MEMC_Lib_Freeze(unsigned char enable);
#ifdef CONFIG_ENABLE_HDMI_NN
extern unsigned int scaler_I3ddma_ISR(void);
extern void h3ddma_nn_set_buffer_addr(unsigned char mode);

extern UINT8 get_nn_force_i3ddma_enable(void);
extern void h3ddma_set_file_mode_buffer_status(void);
#endif
extern void MEMC_Lib_Freeze(unsigned char enable);
extern VIDEO_FW_CONTRL_MEMC_TYPE get_video_fw_memc_freeze_type(unsigned char display);
extern void set_video_fw_memc_freeze_type(unsigned char display,unsigned char value);

/* Variables */

DOLBY_HDMI_VSIF_T pre_dolby_hdmi_vsif_mode;

static int INIT_DOLBY_MODE = 0xFF;

#ifdef CONFIG_SUPPORT_DOLBY_VSIF
static int dolby_vsif_mode_change_count;
static int INIT2_DOLBY_MODE = 0xFE;
#endif
static const int threashold_of_dolby_vsif_mode_change = 0;

extern int flag_dolby_vsif_change;
extern void set_vsc_check_dolby_mode_change_flag(void);
unsigned char dolby_mode_change_triger_mute = FALSE;

extern int pktCrc32Cnt;
//Add game mode measurement @Crixus 20170621

static BOOL I2R_PVR_MAIN_flag = TRUE;
#ifdef CONFIG_I2RND_ENABLE
static BOOL I2R_PVR_SUB_flag = TRUE;
#endif
unsigned int main_repeat_mask_pre = 0;
unsigned int sub_repeat_mask_pre = 0;
unsigned char sub_pst_check_stage = _DISABLE;
extern unsigned char ini_ReadSrcDet;
extern unsigned char g_IsNot4KVO;
static unsigned char Enter_Hist = TRUE;
extern unsigned char hdmi_crc_freeze_status;
extern unsigned char bAIInited;

#if 0
static struct class *vgip_isr_class = NULL;
static struct cdev vgip_isr_cdev;
#endif

#ifdef CONFIG_OF
static const struct of_device_id of_vgip_isr_ids[] = {
	{ .compatible = "realtek,vgip_isr" },
	{}
};

MODULE_DEVICE_TABLE(of, of_vgip_isr_ids);
#endif

static struct platform_device *vgip_isr_platform_devs = NULL;
static struct platform_driver vgip_isr_platform_driver = {
	.remove			= vgip_isr_remove,
#ifdef CONFIG_PM
	.suspend		= vgip_isr_suspend,
	.resume			= vgip_isr_resume,
#endif
	.driver = {
		.name		= "vgip_isr",
		.bus		= &platform_bus_type,
#ifdef CONFIG_OF
		.of_match_table = of_vgip_isr_ids,
#endif
	},
};

#ifdef CONFIG_RTK_KDRV_DV
extern vfe_hdmi_vsi_t hdmi_dolby_vsi_content;
#endif

//static struct platform_driver online_isr_platform_driver;

struct file_operations vgip_isr_fops = {
	.owner                  = THIS_MODULE,
	.unlocked_ioctl         = vgip_isr_ioctl,
};
extern void scalerAI_execute_NN(void);
#ifdef CONFIG_PM
static int vgip_isr_suspend(struct platform_device *dev, pm_message_t state)
{
	rtd_pr_vgip_isr_notice("[VGIP_ISR]%s %d\n",__func__,__LINE__);

        // disable VGIP interrupt
        rtd_clearbits(VGIP_VGIP_CHN1_CTRL_reg, BIT(25));

	rtd_pr_vgip_isr_notice("[VGIP_ISR] suspend done\n");

	return 0;
}

static int vgip_isr_resume(struct platform_device *dev)
{
	rtd_pr_vgip_isr_notice("[VGIP_ISR]%s %d\n",__func__,__LINE__);

        // disable route to VCPU
        rtd_outl(SYS_REG_INT_CTRL_VCPU_reg, _BIT2);
        // enable route to SCPU
        rtd_outl(SYS_REG_INT_CTRL_SCPU_reg,_BIT2|_BIT0);
        // enable VGIP interrupt
        rtd_setbits(VGIP_VGIP_CHN1_CTRL_reg, BIT(25));

	ini_ReadSrcDet=0;

	rtd_pr_vgip_isr_notice("[VGIP_ISR] resume done\n");

	return 0;
}

#endif

static long vgip_isr_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	dummy_vgip_isr_struct dummy;
	int ret = 0;

	switch (cmd) {
	case VGIP_ISR_IOCT_DUMMY:
		ret = copy_from_user(&dummy, (void *)arg, sizeof(dummy));
		break;
	};

	return ret;
}

//#define debug_ISR
#ifdef CONFIG_RTK_KDRV_DV
#include <dolby_adapter.h>
#include <dolby_device.h>
enum dolby_vsem_type pre_dolby_vsem_type;
#endif

void reset_pre_dolby_hdmi_vsif_mode (void)
{
    pre_dolby_hdmi_vsif_mode = INIT_DOLBY_MODE;
#ifdef CONFIG_RTK_KDRV_DV
    pre_dolby_vsem_type = INIT_DOLBY_MODE;
#endif
}

irqreturn_t OnlineMeasureISR(int irq, void *dev_id)
{
	extern void triggle_onms_handler(unsigned char display);
	unsigned char online1_flag = 0, online2_flag = 0;
#ifdef CONFIG_FORCE_RUN_I3DDMA
    unsigned char online3_flag = 0;
#endif
	vgip_int_ctl_RBUS int_ctl_reg;
	onms_onms1_status_RBUS onms_onms1_status_reg;
	onms_onms1_interrupt_en_RBUS onms_onms1_interrupt_en_reg;
	onms_onms2_status_RBUS onms_onms2_status_reg;
	onms_onms2_interrupt_en_RBUS onms_onms2_interrupt_en_reg;
#ifdef CONFIG_FORCE_RUN_I3DDMA
	onms_onms3_status_RBUS onms_onms3_status_reg;
	onms_onms3_interrupt_en_RBUS onms_onms3_interrupt_en_reg;
#endif
	int_ctl_reg.regValue = scaler_rtd_inl( VGIP_INT_CTL_reg );
#ifdef CONFIG_FORCE_RUN_I3DDMA
	if ( int_ctl_reg.onms1_int_ie == 0 && int_ctl_reg.onms2_int_ie == 0 && int_ctl_reg.onms3_int_ie == 0)	// _BIT6
		return IRQ_NONE;
#else
	if ( int_ctl_reg.onms1_int_ie == 0 && int_ctl_reg.onms2_int_ie == 0)	// _BIT6
		return IRQ_NONE;
#endif
	onms_onms1_status_reg.regValue = scaler_rtd_inl(ONMS_onms1_status_reg);
	onms_onms1_interrupt_en_reg.regValue = scaler_rtd_inl(ONMS_onms1_interrupt_en_reg);
	if((onms_onms1_status_reg.regValue & onms_onms1_interrupt_en_reg.regValue)>0)
		online1_flag = 1;
	else
		online1_flag = 0;
	onms_onms2_status_reg.regValue = scaler_rtd_inl(ONMS_onms2_status_reg);
	onms_onms2_interrupt_en_reg.regValue = scaler_rtd_inl(ONMS_onms2_interrupt_en_reg);
	if((onms_onms2_status_reg.regValue & onms_onms2_interrupt_en_reg.regValue)>0)
		online2_flag = 1;
	else
		online2_flag = 0;

#ifdef CONFIG_FORCE_RUN_I3DDMA
	onms_onms3_status_reg.regValue = scaler_rtd_inl(ONMS_onms3_status_reg);
	onms_onms3_interrupt_en_reg.regValue = scaler_rtd_inl(ONMS_onms3_interrupt_en_reg);
	if((onms_onms3_status_reg.regValue & onms_onms3_interrupt_en_reg.regValue)>0)
		online3_flag = 1;
	else
		online3_flag = 0;
#endif

#ifdef CONFIG_FORCE_RUN_I3DDMA
	if(online1_flag || online2_flag || online3_flag)
#else
	if(online1_flag || online2_flag)
#endif
	{//process online measure interrupt
		if (online1_flag) {
			//step 1 clean onlinemeasure interrupt pending
			//scaler_rtd_outl(ONMS_onms1_status_reg, onms_onms1_status_reg.regValue);
			//step 2 disable onlinemeasure interrupt
			scaler_rtd_outl(ONMS_onms1_interrupt_en_reg, 0);
			int_ctl_reg.onms1_int_ie = 0;
			scaler_rtd_outl(VGIP_INT_CTL_reg, int_ctl_reg.regValue);
			//Syep3 pass RPC to Notify kernel scaler code to process signel not stable issue
			//FIX ME

		}

#ifdef CONFIG_FORCE_RUN_I3DDMA
		if (online3_flag) {
			//step 1 clean onlinemeasure interrupt pending
			//scaler_rtd_outl(ONMS_onms1_status_reg, onms_onms1_status_reg.regValue);
			//step 2 disable onlinemeasure interrupt
			scaler_rtd_outl(ONMS_onms3_interrupt_en_reg, 0);
			int_ctl_reg.onms3_int_ie = 0;
			scaler_rtd_outl(VGIP_INT_CTL_reg, int_ctl_reg.regValue);
			//Syep3 pass RPC to Notify kernel scaler code to process signel not stable issue
			triggle_onms_handler(SLR_MAIN_DISPLAY);
		}
		if(online3_flag || online1_flag)
		{
			triggle_onms_handler(SLR_MAIN_DISPLAY);//main online error
		}
#else
		if(online1_flag)
		{
			triggle_onms_handler(SLR_MAIN_DISPLAY);//main online error
		}
#endif
		if (online2_flag) {
			//step 1 clean onlinemeasure interrupt pending
			//scaler_rtd_outl(ONMS_onms2_status_reg, onms_onms2_status_reg.regValue);
			//step 2 disable onlinemeasure interrupt
			scaler_rtd_outl(ONMS_onms2_interrupt_en_reg, 0);
			int_ctl_reg.onms2_int_ie = 0;
			scaler_rtd_outl(VGIP_INT_CTL_reg, int_ctl_reg.regValue);
			//Syep3 pass RPC to Notify kernel scaler code to process signel not stable issue
			//FIX ME
			triggle_onms_handler(SLR_SUB_DISPLAY);//sub online error
		}
		return IRQ_HANDLED;
	}else{
		return IRQ_NONE;
	}
}


#ifdef APVR_ISR_DEBUG
int idma_get_field_mode_sub(void)
{

    return SUB_VGIP_VGIP_CHN2_STATUS_get_ch2_field_pol(rtd_inl(SUB_VGIP_VGIP_CHN2_STATUS_reg)) ? 0:1;//1: 0;
}
#endif

int buf_even_addr = 0;
int buf_odd_addr = 0;
int buf_prog_mode = 0;
#define TVE_VFLIP_SIZE_OFFSET 720*575*2

#ifdef CONFIG_HDR_SDR_SEAMLESS
unsigned char hdmi_position_seamless = FALSE;//for hdr sdr seamless condition for vgip start
unsigned char hdmi_finish_seamless = FALSE;//for hdr sdr seamless condition for vgip end
#endif



#ifdef CONFIG_I3DDMA_PATTERN_MEASUREMENT
void video_latency_pattern_ctrl(void)
{//call by dma vgip interrupt
	extern VSC_VIDEO_LATENCY_PATTERN_T hal_video_latency_pattern_info;//record hal info
	extern unsigned char latency_pattern_change;//record paramter is change or not
	extern spinlock_t* get_latency_pattern_spinlock(void);
	VSC_VIDEO_LATENCY_PATTERN_T video_latency_pattern;
	dma_vgip_ptg_ptg_dma_act_hsta_width_RBUS dma_vgip_ptg_ptg_dma_act_hsta_width_reg;
	dma_vgip_ptg_ptg_dma_act_vsta_width_RBUS dma_vgip_ptg_ptg_dma_act_vsta_width_reg;
	h3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_RBUS i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg;
	dma_vgip_ptg_dma_ptg_ctrl_RBUS dma_vgip_ptg_dma_ptg_ctrl_reg;
	vodma_vodma_line_st_RBUS vodma_vodma_line_st_reg;
	vodma_vodma_line_st_reg.regValue = rtd_inl(VODMA_VODMA_LINE_ST_reg);

	if(latency_pattern_change)
	{//letency parameter change
		spin_lock(get_latency_pattern_spinlock());
		latency_pattern_change = FALSE;
		memcpy(&video_latency_pattern, &hal_video_latency_pattern_info, sizeof(VSC_VIDEO_LATENCY_PATTERN_T));
		spin_unlock(get_latency_pattern_spinlock());
		//Eric@20170712 if input fast case and pattern change from black to white, need check vo linecnt to decide set pattern or not
		if(drvif_i3ddma_triplebuf_flag() && (video_latency_pattern.bPatternType == KADP_VSC_PATTERN_WHITE)){
			if(vodma_vodma_line_st_reg.line_cnt < 100){
				//trigger pattern set in next dma isr
				latency_pattern_change = TRUE;
				return;
			}
		}
		vodma_vodma_line_st_reg.regValue = rtd_inl(VODMA_VODMA_LINE_ST_reg);
		if(video_latency_pattern.bOnOff)
		{
				dma_vgip_ptg_dma_ptg_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_PTG_DMA_PTG_CTRL_reg);
				if(dma_vgip_ptg_dma_ptg_ctrl_reg.dma_color_bar_mode_en == 0)
				{//PTG disable -> enable
					/*
					1. ST_GPIO_60
					0xb8060238[23:20]=f
					IO_Set:  0xb8061104[30] = 1
					GPO_Set: 0xb806110c[30] GPO hi, 0 GPO lo

					2. Need to enable DMA VGIP act_end interrupt
					3. Set dummy degister to trigger.
					*/
					IoReg_SetBits(PINMUX_ST_GPIO_ST_CFG_12_reg, _BIT23|_BIT22|_BIT21|_BIT20);
					IoReg_Write32(GPIO_GPDIR_1_reg, _BIT30|_BIT0);//GPIO
					IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//default low
				}
				//set PTG window size
				dma_vgip_ptg_ptg_dma_act_hsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_PTG_PTG_DMA_ACT_HSTA_Width_reg);
				dma_vgip_ptg_ptg_dma_act_hsta_width_reg.ptg_dma_ih_act_sta = video_latency_pattern.overlayWindow.x;
				dma_vgip_ptg_ptg_dma_act_hsta_width_reg.ptg_dma_ih_act_wid = video_latency_pattern.overlayWindow.w;
				IoReg_Write32(DMA_VGIP_PTG_PTG_DMA_ACT_HSTA_Width_reg, dma_vgip_ptg_ptg_dma_act_hsta_width_reg.regValue);

				dma_vgip_ptg_ptg_dma_act_vsta_width_reg.regValue = IoReg_Read32(DMA_VGIP_PTG_PTG_DMA_ACT_VSTA_Width_reg);
				dma_vgip_ptg_ptg_dma_act_vsta_width_reg.ptg_dma_iv_act_sta = video_latency_pattern.overlayWindow.y;
				dma_vgip_ptg_ptg_dma_act_vsta_width_reg.ptg_dma_iv_act_len = video_latency_pattern.overlayWindow.h;
				IoReg_Write32(DMA_VGIP_PTG_PTG_DMA_ACT_VSTA_Width_reg, dma_vgip_ptg_ptg_dma_act_vsta_width_reg.regValue);


				i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.regValue = IoReg_Read32(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg);
				if(video_latency_pattern.bPatternType == KADP_VSC_PATTERN_WHITE){//white
					if(i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.en_rgb2yuv){
						//black => white and GPIO high
						IoReg_Write32(DMA_VGIP_PTG_DMA_PTG_C0Y_Cb_Cr_reg, 0x00ffffff);
						IoReg_Write32(DMA_VGIP_PTG_DMA_PTG_C1Y_Cb_Cr_reg, 0x00ffffff);
					}else{
						//black => white and GPIO high
						IoReg_Write32(DMA_VGIP_PTG_DMA_PTG_C0Y_Cb_Cr_reg, 0x00ff8080);
						IoReg_Write32(DMA_VGIP_PTG_DMA_PTG_C1Y_Cb_Cr_reg, 0x00ff8080);
					}
					IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30|_BIT0);//white
				}
				else{//balck
					if(i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.en_rgb2yuv){
						//white => black and GPIO low
						IoReg_Write32(DMA_VGIP_PTG_DMA_PTG_C0Y_Cb_Cr_reg, 0x00000000);
						IoReg_Write32(DMA_VGIP_PTG_DMA_PTG_C1Y_Cb_Cr_reg, 0x00000000);
					}else{
						//white => black and GPIO low
						IoReg_Write32(DMA_VGIP_PTG_DMA_PTG_C0Y_Cb_Cr_reg, 0x00008080);
						IoReg_Write32(DMA_VGIP_PTG_DMA_PTG_C1Y_Cb_Cr_reg, 0x00008080);
					}
					IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//black
				}

				//set I3DDMA pattern
				dma_vgip_ptg_dma_ptg_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_PTG_DMA_PTG_CTRL_reg);
				dma_vgip_ptg_dma_ptg_ctrl_reg.dma_color_bar_mode_en = 1;
				dma_vgip_ptg_dma_ptg_ctrl_reg.dma_shift_size_mode = 1;//enable window setting
				//Eric@20180626 component source need set dma_ptg_den_sel = 1
				if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_ADC) || (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_AVD))
					dma_vgip_ptg_dma_ptg_ctrl_reg.dma_ptg_den_sel = 1;
				else
					dma_vgip_ptg_dma_ptg_ctrl_reg.dma_ptg_den_sel = 0;
				dma_vgip_ptg_dma_ptg_ctrl_reg.dma_color_num = 0;//use 2-color
				IoReg_Write32(DMA_VGIP_PTG_DMA_PTG_CTRL_reg, dma_vgip_ptg_dma_ptg_ctrl_reg.regValue);

		}
		else
		{
				//disable GPIO
				//IoReg_ClearBits(0xb800086c, _BIT31|_BIT30|_BIT29|_BIT28);
				//IoReg_Write32(0xb801bd10, _BIT4);//GPIO
				IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//default low

				//disable I3DDMA pattern
				dma_vgip_ptg_dma_ptg_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_PTG_DMA_PTG_CTRL_reg);
				dma_vgip_ptg_dma_ptg_ctrl_reg.dma_color_bar_mode_en = 0;
				IoReg_Write32(DMA_VGIP_PTG_DMA_PTG_CTRL_reg, dma_vgip_ptg_dma_ptg_ctrl_reg.regValue);
		}

	}

}
#endif

void vdec_video_latency_pattern_ctrl(void)
{//call by dma vgip interrupt
	extern VSC_VIDEO_LATENCY_PATTERN_T hal_video_latency_pattern_info;//record hal info
	extern unsigned char latency_pattern_change;//record paramter is change or not
	extern spinlock_t* get_latency_pattern_spinlock(void);
	VSC_VIDEO_LATENCY_PATTERN_T video_latency_pattern;
	vgip_ptg_ptg_chn1_act_hsta_width_RBUS vgip_ptg_ptg_chn1_act_hsta_width_reg;
	vgip_ptg_ptg_chn1_act_vsta_width_RBUS vgip_ptg_ptg_chn1_act_vsta_width_reg;
	vgip_ptg_ptg_ctrl_RBUS vgip_ptg_ptg_ctrl_reg;

	if(latency_pattern_change)
	{//letency parameter change
		spin_lock(get_latency_pattern_spinlock());
		latency_pattern_change = FALSE;
		memcpy(&video_latency_pattern, &hal_video_latency_pattern_info, sizeof(VSC_VIDEO_LATENCY_PATTERN_T));
		spin_unlock(get_latency_pattern_spinlock());
		if(video_latency_pattern.bOnOff)
		{
				vgip_ptg_ptg_ctrl_reg.regValue = IoReg_Read32(VGIP_PTG_PTG_CTRL_reg);
				if(vgip_ptg_ptg_ctrl_reg.color_bar_mode_en == 0)
				{//PTG disable -> enable
					/*
					1. ST_GPIO_60
					0xb8060238[23:20]=f
					IO_Set:  0xb8061104[30] = 1
					GPO_Set: 0xb8061124[30] GPO hi, 0 GPO lo

					2. Need to enable DMA VGIP act_end interrupt
					3. Set dummy degister to trigger.
					*/
					IoReg_SetBits(PINMUX_ST_GPIO_ST_CFG_12_reg, _BIT23|_BIT22|_BIT21|_BIT20);
					IoReg_Write32(GPIO_GPDIR_1_reg, _BIT30|_BIT0);//GPIO
					IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//default low

					//disable memc candance, set b809d730[4] = 1
					IoReg_SetBits(SOFTWARE1_SOFTWARE1_12_reg, _BIT4);
				}
				//set PTG window size
				vgip_ptg_ptg_chn1_act_hsta_width_reg.regValue = IoReg_Read32(VGIP_PTG_PTG_CHN1_ACT_HSTA_Width_reg);
				vgip_ptg_ptg_chn1_act_hsta_width_reg.ptg_ch1_ih_act_sta = video_latency_pattern.overlayWindow.x;
				vgip_ptg_ptg_chn1_act_hsta_width_reg.ptg_ch1_ih_act_wid = video_latency_pattern.overlayWindow.w;
				IoReg_Write32(VGIP_PTG_PTG_CHN1_ACT_HSTA_Width_reg, vgip_ptg_ptg_chn1_act_hsta_width_reg.regValue);

				vgip_ptg_ptg_chn1_act_vsta_width_reg.regValue = IoReg_Read32(VGIP_PTG_PTG_CHN1_ACT_VSTA_Width_reg);
				vgip_ptg_ptg_chn1_act_vsta_width_reg.ptg_ch1_iv_act_sta = video_latency_pattern.overlayWindow.y;
				vgip_ptg_ptg_chn1_act_vsta_width_reg.ptg_ch1_iv_act_len = video_latency_pattern.overlayWindow.h;
				IoReg_Write32(VGIP_PTG_PTG_CHN1_ACT_VSTA_Width_reg, vgip_ptg_ptg_chn1_act_vsta_width_reg.regValue);

				if(RGB2YUV_ICH1_RGB2YUV_CTRL_get_en_rgb2yuv(IoReg_Read32(RGB2YUV_ICH1_RGB2YUV_CTRL_reg))==1) // enable rgb to yuv, rgb input
				{
					if(video_latency_pattern.bPatternType == KADP_VSC_PATTERN_WHITE){//white
						//black => white and GPIO high
						IoReg_Write32(VGIP_PTG_PTG_C0Y_Cb_Cr_reg, 0x00ffffff);
						IoReg_Write32(VGIP_PTG_PTG_C1Y_Cb_Cr_reg, 0x00ffffff);
						IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30|_BIT0);//white
					}
					else{//balck
						//white => black and GPIO low
						IoReg_Write32(VGIP_PTG_PTG_C0Y_Cb_Cr_reg, 0x00000000);
						IoReg_Write32(VGIP_PTG_PTG_C1Y_Cb_Cr_reg, 0x00000000);
						IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//black
					}
				}else{	// yuv input
					if(video_latency_pattern.bPatternType == KADP_VSC_PATTERN_WHITE){//white
						//black => white and GPIO high
						IoReg_Write32(VGIP_PTG_PTG_C0Y_Cb_Cr_reg, 0x00ff8080);
						IoReg_Write32(VGIP_PTG_PTG_C1Y_Cb_Cr_reg, 0x00ff8080);
						IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30|_BIT0);//white
					}
					else{//balck
						//white => black and GPIO low
						IoReg_Write32(VGIP_PTG_PTG_C0Y_Cb_Cr_reg, 0x00008080);
						IoReg_Write32(VGIP_PTG_PTG_C1Y_Cb_Cr_reg, 0x00008080);
						IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//black
					}
				}

				//set I3DDMA pattern
				vgip_ptg_ptg_ctrl_reg.regValue = IoReg_Read32(VGIP_PTG_PTG_CTRL_reg);
				vgip_ptg_ptg_ctrl_reg.color_bar_mode_en = 1;
				vgip_ptg_ptg_ctrl_reg.shift_size_mode = 1;//enable window setting
				vgip_ptg_ptg_ctrl_reg.color_num = 0;//use 2-color
				IoReg_Write32(VGIP_PTG_PTG_CTRL_reg, vgip_ptg_ptg_ctrl_reg.regValue);

		}
		else
		{
				//disable GPIO
				//IoReg_ClearBits(0xb800086c, _BIT31|_BIT30|_BIT29|_BIT28);
				//IoReg_Write32(0xb801bd10, _BIT4);//GPIO
				IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//default low

				//disable I3DDMA pattern
				vgip_ptg_ptg_ctrl_reg.regValue = IoReg_Read32(VGIP_PTG_PTG_CTRL_reg);
				vgip_ptg_ptg_ctrl_reg.color_bar_mode_en = 0;
				IoReg_Write32(VGIP_PTG_PTG_CTRL_reg, vgip_ptg_ptg_ctrl_reg.regValue);
				//recovery memc candance, set b809d730[4] = 1
				IoReg_ClearBits(SOFTWARE1_SOFTWARE1_12_reg, _BIT4);
		}

	}

}

void pif_video_latency_pattern_ctrl(void)
{//call by dma vgip interrupt
	extern VSC_VIDEO_LATENCY_PATTERN_T hal_video_latency_pattern_info;//record hal info
	extern unsigned char latency_pattern_change;//record paramter is change or not
	extern spinlock_t* get_latency_pattern_spinlock(void);
	VSC_VIDEO_LATENCY_PATTERN_T video_latency_pattern;
	sfg_sfg_patgen_ctrl_RBUS sfg_sfg_patgen_ctrl_reg;

	if(latency_pattern_change)
	{//letency parameter change
		spin_lock(get_latency_pattern_spinlock());
		latency_pattern_change = FALSE;
		memcpy(&video_latency_pattern, &hal_video_latency_pattern_info, sizeof(VSC_VIDEO_LATENCY_PATTERN_T));
		spin_unlock(get_latency_pattern_spinlock());
		if(video_latency_pattern.bOnOff)
		{
				sfg_sfg_patgen_ctrl_reg.regValue = IoReg_Read32(SFG_SFG_patgen_ctrl_reg);
				if(sfg_sfg_patgen_ctrl_reg.sfg_pat_en == 0)
				{//PTG disable -> enable
					/*
					1. ST_GPIO_60
					0xb8060238[23:20]=f
					IO_Set:  0xb8061104[30] = 1
					GPO_Set: 0xb806110c[30] GPO hi, 0 GPO lo

					2. Need to enable DMA VGIP act_end interrupt
					3. Set dummy degister to trigger.
					*/
					IoReg_SetBits(PINMUX_ST_GPIO_ST_CFG_12_reg, _BIT23|_BIT22|_BIT21|_BIT20);
					IoReg_Write32(GPIO_GPDIR_1_reg, _BIT30|_BIT0);//GPIO
					IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//default low
				}

				if(video_latency_pattern.bPatternType == KADP_VSC_PATTERN_WHITE){//white
					//black => white and GPIO high
					sfg_sfg_patgen_ctrl_reg.sfg_pat_mode = 0;
					IoReg_Write32(SFG_SFG_patgen_ctrl_reg, sfg_sfg_patgen_ctrl_reg.regValue);
					IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30|_BIT0);//white
				}
				else{//balck
					//white => black and GPIO low
					sfg_sfg_patgen_ctrl_reg.sfg_pat_mode = 2;
					IoReg_Write32(SFG_SFG_patgen_ctrl_reg, sfg_sfg_patgen_ctrl_reg.regValue);
					IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//black
				}

				//set SFG pattern
				sfg_sfg_patgen_ctrl_reg.regValue = IoReg_Read32(SFG_SFG_patgen_ctrl_reg);
				sfg_sfg_patgen_ctrl_reg.sfg_pat_en = 1;
				IoReg_Write32(SFG_SFG_patgen_ctrl_reg, sfg_sfg_patgen_ctrl_reg.regValue);

		}
		else
		{
				//disable GPIO
				//IoReg_ClearBits(0xb800086c, _BIT31|_BIT30|_BIT29|_BIT28);
				//IoReg_Write32(0xb801bd10, _BIT4);//GPIO
				IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//default low

				//disable SFG pattern
				sfg_sfg_patgen_ctrl_reg.regValue = IoReg_Read32(SFG_SFG_patgen_ctrl_reg);
				sfg_sfg_patgen_ctrl_reg.sfg_pat_en = 0;
				IoReg_Write32(SFG_SFG_patgen_ctrl_reg, sfg_sfg_patgen_ctrl_reg.regValue);
		}

	}

}

void uzu_video_latency_pattern_ctrl(void)
{//call by master dtg interrupt
	extern VSC_VIDEO_LATENCY_PATTERN_T hal_video_latency_pattern_info;//record hal info
	extern unsigned char latency_pattern_change;//record paramter is change or not
	extern spinlock_t* get_latency_pattern_spinlock(void);
	VSC_VIDEO_LATENCY_PATTERN_T video_latency_pattern;
	scaleup_d_uzu_globle_ctrl_RBUS scaleup_d_uzu_globle_ctrl_reg;
	scaleup_ds_uzu_globle_ctrl_RBUS scaleup_ds_uzu_globle_ctrl_reg;

	if(latency_pattern_change)
	{//letency parameter change
		spin_lock(get_latency_pattern_spinlock());
		latency_pattern_change = FALSE;
		memcpy(&video_latency_pattern, &hal_video_latency_pattern_info, sizeof(VSC_VIDEO_LATENCY_PATTERN_T));
		spin_unlock(get_latency_pattern_spinlock());
		if(video_latency_pattern.bOnOff)
		{
				scaleup_d_uzu_globle_ctrl_reg.regValue = IoReg_Read32(SCALEUP_D_UZU_Globle_Ctrl_reg);
				scaleup_ds_uzu_globle_ctrl_reg.regValue = IoReg_Read32(SCALEUP_DS_UZU_Globle_Ctrl_reg);
				if(scaleup_d_uzu_globle_ctrl_reg.patgen_sel == 0)
				{//PTG disable -> enable
					/*
					1. ST_GPIO_60
					0xb8060238[23:20]=f
					IO_Set:  0xb8061104[30] = 1
					GPO_Set: 0xb806110c[30] GPO hi, 0 GPO lo

					2. Need to enable DMA VGIP act_end interrupt
					3. Set dummy degister to trigger.
					*/
					IoReg_SetBits(PINMUX_ST_GPIO_ST_CFG_12_reg, _BIT23|_BIT22|_BIT21|_BIT20);
					IoReg_Write32(GPIO_GPDIR_1_reg, _BIT30|_BIT0);//GPIO
					IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//default low
				}

				IoReg_Write32(SCALEUP_D_UZU_PATGEN_FRAME_TOGGLE_reg, 0x3bf);
				IoReg_Write32(SCALEUP_DS_UZU_PATGEN_FRAME_TOGGLE_reg, 0x3bf);

				if(video_latency_pattern.bPatternType == KADP_VSC_PATTERN_WHITE){//white
					//black => white and GPIO high
					scaleup_d_uzu_globle_ctrl_reg.regValue = IoReg_Read32(SCALEUP_D_UZU_Globle_Ctrl_reg);
					scaleup_d_uzu_globle_ctrl_reg.patgen_mode = 2;
					IoReg_Write32(SCALEUP_D_UZU_Globle_Ctrl_reg, scaleup_d_uzu_globle_ctrl_reg.regValue);
					scaleup_ds_uzu_globle_ctrl_reg.regValue = IoReg_Read32(SCALEUP_DS_UZU_Globle_Ctrl_reg);
					scaleup_ds_uzu_globle_ctrl_reg.patgen_mode = 2;
					IoReg_Write32(SCALEUP_DS_UZU_Globle_Ctrl_reg, scaleup_ds_uzu_globle_ctrl_reg.regValue);
					IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30|_BIT0);//white
				}
				else{//balck
					//white => black and GPIO low
					scaleup_d_uzu_globle_ctrl_reg.regValue = IoReg_Read32(SCALEUP_D_UZU_Globle_Ctrl_reg);
					scaleup_d_uzu_globle_ctrl_reg.patgen_mode = 4;
					IoReg_Write32(SCALEUP_D_UZU_Globle_Ctrl_reg, scaleup_d_uzu_globle_ctrl_reg.regValue);
					scaleup_ds_uzu_globle_ctrl_reg.regValue = IoReg_Read32(SCALEUP_DS_UZU_Globle_Ctrl_reg);
					scaleup_ds_uzu_globle_ctrl_reg.patgen_mode = 4;
					IoReg_Write32(SCALEUP_DS_UZU_Globle_Ctrl_reg, scaleup_ds_uzu_globle_ctrl_reg.regValue);
					IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//black
				}

				//set uzu pattern
				scaleup_d_uzu_globle_ctrl_reg.regValue = IoReg_Read32(SCALEUP_D_UZU_Globle_Ctrl_reg);
				scaleup_d_uzu_globle_ctrl_reg.patgen_sync = 1;
				scaleup_d_uzu_globle_ctrl_reg.patgen_sel = 1;
				IoReg_Write32(SCALEUP_D_UZU_Globle_Ctrl_reg, scaleup_d_uzu_globle_ctrl_reg.regValue);
				scaleup_ds_uzu_globle_ctrl_reg.regValue = IoReg_Read32(SCALEUP_DS_UZU_Globle_Ctrl_reg);
				scaleup_ds_uzu_globle_ctrl_reg.patgen_sync = 1;
				scaleup_ds_uzu_globle_ctrl_reg.patgen_sel = 1;
				IoReg_Write32(SCALEUP_DS_UZU_Globle_Ctrl_reg, scaleup_ds_uzu_globle_ctrl_reg.regValue);

		}
		else
		{
				//disable GPIO
				//IoReg_ClearBits(0xb800086c, _BIT31|_BIT30|_BIT29|_BIT28);
				//IoReg_Write32(0xb801bd10, _BIT4);//GPIO
				IoReg_Write32(GPIO_GPDATO_1_reg, _BIT30);//default low

				//set uzu pattern
				scaleup_d_uzu_globle_ctrl_reg.regValue = IoReg_Read32(SCALEUP_D_UZU_Globle_Ctrl_reg);
				scaleup_d_uzu_globle_ctrl_reg.patgen_sync = 0;
				scaleup_d_uzu_globle_ctrl_reg.patgen_sel = 0;
				IoReg_Write32(SCALEUP_D_UZU_Globle_Ctrl_reg, scaleup_d_uzu_globle_ctrl_reg.regValue);
				scaleup_ds_uzu_globle_ctrl_reg.regValue = IoReg_Read32(SCALEUP_DS_UZU_Globle_Ctrl_reg);
				scaleup_ds_uzu_globle_ctrl_reg.patgen_sync = 0;
				scaleup_ds_uzu_globle_ctrl_reg.patgen_sel = 0;
				IoReg_Write32(SCALEUP_DS_UZU_Globle_Ctrl_reg, scaleup_ds_uzu_globle_ctrl_reg.regValue);
		}

	}

}

extern UINT32 Scaler_Pst_Callback_Check_Vaildinfo(void);
extern void wakeup_window_delay_info_callback(unsigned char display);
extern void isr_control_hdmi_avmute(unsigned char display);

extern void scalerAI_postprocessing(void);
extern void scalerAI_preprocessing(void);
extern void clean_flag(void);
unsigned int vgip_isr_cnt = 0;
unsigned int i3ddma_cap_isr_cnt = 0;
extern struct semaphore sem_se_ai;
extern int sem_se_ai_flag;
extern void hdmi_4k120_se_wakeup(void);

unsigned int memc_freeze_record_time = 0;

void memc_force_unfreeze_check(void)
{
    #define TIMEOUT_THRESHOULD (1000*90)//1 second
    _RPC_system_setting_info* RPC_SysInfo = NULL;
    RPC_SysInfo = scaler_GetShare_Memory_RPC_system_setting_info_Struct();
    if(RPC_SysInfo)
    {
        if(RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Status])
        {
            unsigned int cost_time;
            unsigned int record_time = memc_freeze_record_time;
            unsigned int cur_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);

            if(cur_time < record_time)
        {//system time is already overflow
                cost_time = (0xffffffff - record_time) + cur_time;
            }
            else
            {
                cost_time = cur_time - record_time;
            }
            if(cost_time >= TIMEOUT_THRESHOULD)
            {
                pr_info("DM_freeze, un-freeze check\n");
                MEMC_Lib_Freeze(0);
                RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Status] = 0;
                printk(KERN_ERR " ###[ERR] func:%s timeout####\r\n",__FUNCTION__);

            }
        }
    }

}

void forcebg_memc_force_unfreeze(void)
{
    _RPC_system_setting_info* RPC_SysInfo = NULL;
    RPC_SysInfo = scaler_GetShare_Memory_RPC_system_setting_info_Struct();
    if(RPC_SysInfo)
    {
        if(RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Status])
        {
            pr_info("DM_freeze, unfreeze force,\n");
            MEMC_Lib_Freeze(0);
            RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Status] = 0;
            printk(KERN_ERR " ### func:%s enable forcebg unfreeze memc####\r\n", __FUNCTION__);
        }
    }

}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
void pq_automa_task( struct tasklet_struct *data )
#else
void pq_automa_task( unsigned long data )
#endif
{
	if(I2R_PVR_MAIN_flag){
		if ( Scaler_DispGetInfoPtr() != NULL && Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE ){

                        Enter_Hist = drv_memory_get_vdec_direct_low_latency_mode() ? FALSE:TRUE;
			if(Enter_Hist){

				scalerVIP_colorHistISR(_CHANNEL1);  // Erin

				if(fwif_color_GetAutoMAFlag()) {

					scalerVIP_colorMaAutoISR(_CHANNEL1); //Thur 20070920

					if( g_IsNot4KVO && Scaler_Get3DEnable() && scalerVIP_check_vdc_hw_pwr() && AVD_Clock_Flag_detection()) {
						scalerVideo_adaptive3DAdjust();
						/*
						if(Scaler_InputSrcGetMainChFrom() ==_SRC_FROM_VDC) {//VDC source enter DI => refer to external accumulation
							scalerVideo_adaptive3DAdjust();//wjchen 1225, for costomer2 new adaptive 3d threshold
						} else { //"NOT" VDC source enter DI => refer to vdc internal accumulation
							scalerVideo_adaptive3DAdjust();//wjchen 1225, for costomer2 new adaptive 3d threshold
						}
						*/
					}
				}

			}
		}
	}

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	// AI application
		if(bAIInited)
		{
			//rtd_pr_vgip_isr_info("henrydebug", "=== Vgip_isr start ===\n");

			// manually activate
			/*if(bAIInited && runNN == 0 && rtd_inl(0xb802e4f0) == 0x80000000)
			{
				scalerAI_execute_NN();
				runNN = 1;
			}*/

			scalerAI_postprocessing();
			// end AI

			if(sem_se_ai_flag==1) up(&sem_se_ai);
			vgip_isr_cnt++;
		}
#endif

	/* adaptive OD LUT for VRR */
	//fwif_color_od_table_dynamic_for_VRR();// k7 not verified
	return;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
DECLARE_TASKLET(pq_automa_tasklet, pq_automa_task);
#else
DECLARE_TASKLET(pq_automa_tasklet, pq_automa_task, 0);
#endif

irqreturn_t vgip_isr(int irq, void *dev_id)
{

extern void trigger_mute_off_by_isr(unsigned char display, unsigned char no_delay);
#ifdef SDNR_LINECNT_DEBUG//for ENC RL6557-451
scaledown_ich1_line_cnt_RBUS scaledown_ich1_line_cnt_reg;
static unsigned int counter_vgipsta =0;//vgip start 90k counter
static unsigned int counter_vgipend =0;//vgip end 90k counter
static unsigned int counter_sdnrend =0;//sdnr end 90k counter
static unsigned int sdnr_lc_vgipsta =0;//vgip start sdnr line count
static unsigned int sdnr_lc_vgipend =0;//vgip end sdnr line count
static unsigned int sdnr_lc_sdnrend =0;//sdnr end sdnr line count
#endif


#ifdef debug_ISR

static unsigned int time_cnt1=0;
unsigned int reg_value = 0;
reg_value = rtd_inl(COLOR_DCC_D_DCC_FWUSED_3_reg/*0xb802e4f8*/);
if((reg_value & BIT(30))){
	time_cnt1++;
}

int time0, time1, time2;
static int exe_cnt=0, exe_cnt_2=0;
static int time0s_set=0, time1s_set=0, time2s_set;
static int time0s_start=0, time1s_start=0, time2s_start=0;
static int time0s_end=0, time1s_end=0, time2s_end=0;
static int time0s_cnt=0, time1s_cnt=0, time2s_cnt=0;
int line_cnt, line_info;

exe_cnt++;

#endif

#ifdef APVR_ISR_DEBUG
static unsigned int apvr_cur_time = 0;
static unsigned int apvr_pre_time = 0;
static int apvr_cur_field = -1;
static int apvr_pre_field = -1;
#endif

#ifdef CONFIG_HDR_SDR_SEAMLESS
extern bool get_support_vo_force_v_top(unsigned int func_flag);
seamless_change_sync_info *seamless_info_sharememory = NULL;
static unsigned int i3ddma_target_color_format = I3DDMA_COLOR_YUV422;
extern void dynamic_change_hdr_setting(unsigned char hdr_enable, I3DDMA_COLOR_SPACE_T color_format);
extern void dynamic_change_hdr_setting_hdmi_position(unsigned char hdr_enable, I3DDMA_COLOR_SPACE_T color_format, unsigned char hdr_type);
extern void dynamic_change_hdr_setting_hdmi_finish(unsigned char hdr_enable, I3DDMA_COLOR_SPACE_T color_format, unsigned char hdr_type);
#endif

#if 0


	unsigned int status;

	status = rtd_inl(0xb8022214);

	if (!(status & BIT(25)) && !(status & BIT(24)))
		return IRQ_NONE;

	if (status & BIT(24)) {
		rtd_setbits(0xb8022214, BIT(24));
	}

	if (status & BIT(25)) {
		rtd_setbits(0xb8022214, BIT(25));
	}

#else
	// Handle Flag
	unsigned char handle_ch1_sta_flag = 0;
	unsigned char handle_ch1_end_flag = 0;
	unsigned char handle_ch2_sta_flag = 0;
	unsigned char handle_ch2_end_flag = 0;
	unsigned char handle_dma_ch3_sta_flag = 0;
	unsigned char handle_dma_ch3_end_flag = 0;
	UINT8 handle_h3ddma_Cap_last_wr_flag = 0;
	unsigned char handle_h3ddma_Cap3_last_wr_flag = 0;
	unsigned char handle_ch1_sdnr_act_end_flag = 0;
	unsigned char handle_ch2_sdnr_act_end_flag = 0;

#ifdef CONFIG_I2RND_ENABLE
	vodma_vodma_i2rnd_m_flag_RBUS vo_i2rnd_m_flag;
	vodma_vodma_i2rnd_s_flag_RBUS vo_i2rnd_s_flag;
	vodma_vodma_i2rnd_st_RBUS vodma_i2rnd_st_reg;
#endif
	_RPC_system_setting_info* RPC_SysInfo = NULL;
	_system_setting_info* SysInfo = NULL;

	/*==main channel==*/
	vgip_int_ctl_RBUS int_ctl_reg;
	vgip_vgip_chn1_ctrl_RBUS vgip_chn1_ctrl_reg;
	vgip_vgip_chn1_status_RBUS vgip_chn1_status_reg;
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_chn1_ctrl_reg;
	dma_vgip_dma_vgip_status_RBUS dma_vgip_chn1_status_reg;
	h3ddma_interrupt_enable_RBUS h3ddma_interrupt_enable_reg;
	h3ddma_cap0_cap_status_RBUS h3ddma_cap0_cap_status_reg;
	h3ddma_cap3_cap_status_RBUS h3ddma_cap3_cap_status_reg;
	histogram_ich1_hist_status_RBUS ich1_hist_status;
	vgip_vgip_chn1_wde_RBUS vgip_vgip_chn1_wde_reg;//for sdnr interrupt @Crixus 20170518
	mdomain_cap_ddr_in2status_RBUS ddr_in2status_reg;

#ifdef CONFIG_ENABLE_HDMI_NN
	//static unsigned long long ullFrameCnt = 0;
	//h3ddma_cap1_cap_status_RBUS h3ddma_cap1_cap_status_reg;
#endif

	/*==sub channel==*/
	#if 1
	sub_vgip_vgip_chn2_ctrl_RBUS vgip_chn2_ctrl_reg;
	sub_vgip_vgip_chn2_status_RBUS vgip_chn2_status_reg;
	sub_vgip_vgip_chn2_wde_RBUS sub_vgip_vgip_chn2_wde_reg;//for sdnr interrupt @Crixus 20170518
	#endif


	unsigned char bEnable_main, bEnable_sub;

	//static unsigned char runNN = 0;

        /*==20171228, pinyen modify for Dolby HDMI to avoid pink gabage when the begining from SDR to DolbyHDR*/
        #ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
        ppoverlay_double_buffer_ctrl3_RBUS ppoverlay_double_buffer_ctrl3_reg;
        #endif

	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;
	RPC_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();

	RPC_SysInfo = scaler_GetShare_Memory_RPC_system_setting_info_Struct();
	SysInfo = scaler_GetShare_Memory_system_setting_info_Struct();
	vgip_chn1_ctrl_reg.regValue = rtd_inl(VGIP_VGIP_CHN1_CTRL_reg);
	vgip_chn1_status_reg.regValue = rtd_inl(VGIP_VGIP_CHN1_STATUS_reg);
	// check IDMA VGIP interrupt pending
	dma_vgip_chn1_ctrl_reg.regValue = scaler_rtd_inl(DMA_VGIP_DMA_VGIP_CTRL_reg);
	dma_vgip_chn1_status_reg.regValue = scaler_rtd_inl(DMA_VGIP_DMA_VGIP_STATUS_reg);
	h3ddma_interrupt_enable_reg.regValue = scaler_rtd_inl(H3DDMA_Interrupt_Enable_reg);
	h3ddma_cap0_cap_status_reg.regValue = scaler_rtd_inl(H3DDMA_CAP0_Cap_Status_reg);
	h3ddma_cap3_cap_status_reg.regValue = scaler_rtd_inl(H3DDMA_CAP3_Cap_Status_reg);
	ich1_hist_status.regValue = rtd_inl(HISTOGRAM_ICH1_Hist_Status_reg);
	//sdnr interrupt
	vgip_vgip_chn1_wde_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_WDE_reg);

	ddr_in2status_reg.regValue = rtd_inl(MDOMAIN_CAP_DDR_In2Status_reg);

	int_ctl_reg.regValue = rtd_inl( VGIP_INT_CTL_reg );
	if ( int_ctl_reg.vgip_int_ie == 0 ){	// _BIT3
		//return IRQ_NONE;//can not return, because other ip will cause interrupt
	}

	/********main channel*******/
	if (vgip_chn1_ctrl_reg.ch1_vact_start_ie &&  vgip_chn1_status_reg.ch1_vact_start && int_ctl_reg.vgip_int_ie){	// _BIT24
		ich1_hist_status.ch1_his_his_irq = 1;
		handle_ch1_sta_flag = 1;
	}

	if (vgip_chn1_ctrl_reg.ch1_vact_end_ie && vgip_chn1_status_reg.ch1_vact_end && int_ctl_reg.vgip_int_ie){	// _BIT25
		handle_ch1_end_flag = 1;

		if(Scaler_Pst_Callback_Check_Vaildinfo()){
			wakeup_window_delay_info_callback(0);
			//rtd_pr_vgip_isr_emerg("[PROFILE]DISR (%x), line:%d \n", IoReg_Read32(0xB801B6B8), PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)));
		}
	}

	if (dma_vgip_chn1_ctrl_reg.dma_vact_start_ie && dma_vgip_chn1_status_reg.dma_vact_start && int_ctl_reg.vgip_int_ie) {	// _BIT24
		handle_dma_ch3_sta_flag = 1;
	}

	if (dma_vgip_chn1_ctrl_reg.dma_vact_end_ie && dma_vgip_chn1_status_reg.dma_vact_end && int_ctl_reg.vgip_int_ie) {		// _BIT25
		handle_dma_ch3_end_flag = 1;
	}

	if(h3ddma_interrupt_enable_reg.cap0_last_wr_ie && h3ddma_cap0_cap_status_reg.cap0_cap_last_wr_flag)
		handle_h3ddma_Cap_last_wr_flag = 1;

	if(h3ddma_interrupt_enable_reg.cap3_last_wr_ie && h3ddma_cap3_cap_status_reg.cap3_cap_last_wr_flag) {

		handle_h3ddma_Cap3_last_wr_flag = 1;
	}

	//sdnr interrupt
	if(vgip_vgip_chn1_wde_reg.ch1_dispi_vact_end_ie && vgip_chn1_status_reg.ch1_sdnr_vact_end)
		handle_ch1_sdnr_act_end_flag = 1;


#ifdef SDNR_LINECNT_DEBUG//for ENC RL6557-451
	scaledown_ich1_line_cnt_reg.regValue = rtd_inl( SCALEDOWN_ICH1_line_cnt_reg);
	if(handle_ch1_sta_flag)
	{
		sdnr_lc_vgipsta = scaledown_ich1_line_cnt_reg.sdnr_ch1_line_cnt;
		counter_vgipsta = rtd_inl(TIMER_SCPU_CLK90K_LO_reg)/90;
	}
	if(handle_ch1_sdnr_act_end_flag)
	{
		sdnr_lc_sdnrend = scaledown_ich1_line_cnt_reg.sdnr_ch1_line_cnt;
		counter_sdnrend = rtd_inl(TIMER_SCPU_CLK90K_LO_reg)/90;
	}
	if(handle_ch1_end_flag)
	{
		sdnr_lc_vgipend = scaledown_ich1_line_cnt_reg.sdnr_ch1_line_cnt;
		counter_vgipend = rtd_inl(TIMER_SCPU_CLK90K_LO_reg)/90;
		rtd_pr_vgip_isr_notice("\r\n#### func:%s sdnr lc info (%d %d %d) time(%d %d %d)####\r\n",__FUNCTION__, sdnr_lc_vgipsta, sdnr_lc_sdnrend, sdnr_lc_vgipend, counter_vgipsta, counter_sdnrend, counter_vgipend);
	}

#endif
        if(handle_dma_ch3_sta_flag)
	{
		Scaler_Set_Record_I3DDMA_Capture_Index(H3DDMA_CAP0_Cap_Status_get_cap0_cap_block_sel(rtd_inl(H3DDMA_CAP0_Cap_Status_reg)));
	}

	if(handle_ch1_sta_flag)
	{//check hdmi av mute status flow
		isr_control_hdmi_avmute(SLR_MAIN_DISPLAY);
	}
	/*Handle SMF BBD*/
	if(handle_ch1_sta_flag)
	{
		scalerVIP_Get_BlackDetection_EN(&bEnable_main, &bEnable_sub);
		scalerVIP_SMF_BlackDetection(bEnable_main);
	}
//#ifdef CONFIG_REALTEK_2K_MODEL_ENABLED
	if((get_panel_res() == PANEL_RES_FHD)||(get_panel_res() == PANEL_RES_HD)){
		if(handle_ch1_sta_flag && (drv_memory_Get_multibuffer_flag()==1) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE))
		{
			mdomain_cap_ddr_in1ctrl_RBUS mdomain_cap_ddr_in1ctrl_reg;
			mdomain_cap_ddr_in1ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Ctrl_reg);
			if (drv_memory_Get_capture_multibuffer_flag() == 0) {
				drv_memory_Set_multibuffer_number_pre(drv_memory_Get_multibuffer_number());
				if((drv_memory_Get_multibuffer_number_pre()-1) != mdomain_cap_ddr_in1ctrl_reg.in1_bufnum)
					drv_memory_capture_multibuffer_switch_buffer_number();
			}
		}

		if(handle_ch1_end_flag && (drv_memory_Get_multibuffer_flag()==1) &&(drv_memory_Get_capture_multibuffer_flag()==1))
		{

			drv_memory_display_multibuffer_switch_buffer_number();
		}
	}
//#endif
	/*********sub channel*******/
	#if 1
	// check channel 2 interrupt pending
	vgip_chn2_ctrl_reg.regValue = scaler_rtd_inl( SUB_VGIP_VGIP_CHN2_CTRL_reg );
	if (vgip_chn2_ctrl_reg.ch2_vact_start_ie || vgip_chn2_ctrl_reg.ch2_vact_end_ie) { //_BIT24 _BIT25 enable channel2 VActiveStainterrupt or VActiveEndinterrupt
		//channel 2 interrupt
		vgip_chn2_status_reg.regValue = scaler_rtd_inl( SUB_VGIP_VGIP_CHN2_STATUS_reg );

		if (vgip_chn2_ctrl_reg.ch2_vact_start_ie && vgip_chn2_status_reg.ch2_vact_start && int_ctl_reg.vgip_int_ie)	{//	_BIT24
			handle_ch2_sta_flag = 1;
			scalerVIP_Get_BlackDetection_EN(&bEnable_main, &bEnable_sub);
			scalerVIP_SMF_BlackDetection_SUB(bEnable_sub);


		}
		if (vgip_chn2_ctrl_reg.ch2_vact_end_ie && vgip_chn2_status_reg.ch2_vact_end && int_ctl_reg.vgip_int_ie) {		//	_BIT25

			handle_ch2_end_flag = 1;
		}
	}

	if(handle_ch2_sta_flag)
	{
		isr_control_hdmi_avmute(SLR_SUB_DISPLAY);
	}

	//sdnr channel 2 interrupt
	sub_vgip_vgip_chn2_wde_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_WDE_reg);
	if(sub_vgip_vgip_chn2_wde_reg.ch2_dispi_vact_end_ie && vgip_chn2_status_reg.ch2_sdnr_vact_end)
		handle_ch2_sdnr_act_end_flag = 1;

	#endif

	//if(handle_ch1_end_flag)
		//updateVODMASetting(0);

	if((!(vgip_chn1_ctrl_reg.ch1_vact_end_ie && vgip_chn1_status_reg.ch1_vact_end )) &&
		(handle_ch1_sta_flag || handle_ch1_end_flag || handle_ch2_sta_flag || handle_ch2_end_flag || handle_dma_ch3_sta_flag || handle_dma_ch3_end_flag))
	{
		rtd_outl(HISTOGRAM_ICH1_Hist_Status_reg, ich1_hist_status.regValue);
	}


#ifdef CONFIG_I2RND_ENABLE
	if(handle_ch1_sta_flag==1 || handle_ch1_end_flag==1){
		if(Scaler_I2rnd_get_timing_enable()==1){
			vodma_i2rnd_st_reg.regValue= rtd_inl(VODMA_vodma_i2rnd_st_reg);
			if(vodma_i2rnd_st_reg.i2rnd_st == 0x3){
				I2R_PVR_MAIN_flag = 1;
				I2R_PVR_SUB_flag = 0;
			}else if(vodma_i2rnd_st_reg.i2rnd_st == 0x5){
				I2R_PVR_MAIN_flag = 0;
				I2R_PVR_SUB_flag = 1;
			}else{
				I2R_PVR_MAIN_flag = 0;
				I2R_PVR_SUB_flag = 0;
			}
		}else{
			I2R_PVR_MAIN_flag = 1;
		}
	}
#else
	I2R_PVR_MAIN_flag = 1;
#endif
    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    /* check dolby status every frame
     * 1. check pause or not in OTT
     */
#ifdef CONFIG_RTK_KDRV_DV
#ifdef	CONFIG_DOLBY_IDK_1_6_1_1
    if (handle_ch1_sta_flag) {
        extern void dolby_adapter_execute_guard_func(void);
        VSC_INPUT_TYPE_T src_type = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
        HDR_MODE ott_hdr_mode = get_OTT_HDR_mode();

        if (src_type == VSC_INPUTSRC_VDEC && ott_hdr_mode == HDR_DOLBY_COMPOSER) {
            dolby_adapter_execute_guard_func();
        }
    }
#endif
#endif

#ifdef CONFIG_SUPPORT_DOLBY_VSIF
    if((handle_ch1_sta_flag && !get_scaler_qms_mode_flag()) || (handle_dma_ch3_sta_flag && get_scaler_qms_mode_flag()))
        //if (vgip_chn1_ctrl_reg.ch1_vact_start_ie &&  vgip_chn1_status_reg.ch1_vact_start)//if(handle_ch1_sta_flag)
	{	// _BIT24
	    dolby_mode_change_triger_mute= FALSE;
		if(drvif_Hdmi_get_infoframe_thread_stop()==_TRUE)
		{
				if(VSC_INPUTSRC_HDMI == Get_DisplayMode_Src(SLR_MAIN_DISPLAY))
				{
					extern unsigned char newbase_hdmi_get_current_display_port(void);
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
					extern void dolby_vision_parser_vsem_packet(bool set_type);
#endif
					unsigned char port = newbase_hdmi_get_current_display_port();
					bool apply_change_vsem_type = (pre_dolby_vsem_type != INIT_DOLBY_MODE) && (pre_dolby_vsem_type != INIT2_DOLBY_MODE);

					drvif_Hdmi_infoframe_update_for_isr();
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
					dolby_vision_parser_vsem_packet(apply_change_vsem_type);
#endif

					// hdmi task control task need to sync pre_dolby_hdmi_vsif_mode and current_dolby_hdmi_vsif_mode
					if (pre_dolby_hdmi_vsif_mode != get_HDMI_Dolby_VSIF_mode())
					{
                        if (pre_dolby_hdmi_vsif_mode == INIT_DOLBY_MODE) {
							pre_dolby_hdmi_vsif_mode = INIT2_DOLBY_MODE;
						} else if (pre_dolby_hdmi_vsif_mode == INIT2_DOLBY_MODE) {
							pre_dolby_hdmi_vsif_mode = get_HDMI_Dolby_VSIF_mode();
						} else {
							rtd_pr_vgip_isr_info("[%s:%d][Dolby] VSIF %d -> %d\n", __func__, __LINE__, pre_dolby_hdmi_vsif_mode,  get_HDMI_Dolby_VSIF_mode());
							++dolby_vsif_mode_change_count;
							set_dolby_vsif_change(TRUE);

							if (dolby_vsif_mode_change_count > threashold_of_dolby_vsif_mode_change) {
								if((get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_VSIF_LL) || (pre_dolby_hdmi_vsif_mode == DOLBY_HDMI_VSIF_LL))
									flag_dolby_vsif_change = 1;

								pre_dolby_hdmi_vsif_mode=get_HDMI_Dolby_VSIF_mode();
								dolby_mode_change_triger_mute = TRUE;
								drvif_Hdmi_SetDolbyVsifUpdateNoPktThreshold(NO_DVS_INFO_MAX);
								drvif_Hdmi_set_infoframe_thread_stop_nonseamphore_for_isr(FALSE);
							}
						}


#if 0
						{ // triger mute on
						rtd_maskl(PPOVERLAY_Main_Display_Control_RSV_reg, ~(_BIT1), _BIT1);
						      ppoverlay_double_buffer_ctrl2_reg.regValue = rtd_inl(PPOVERLAY_Double_Buffer_CTRL2_reg);
						      ppoverlay_double_buffer_ctrl2_reg.dmainreg_other_dbuf_en = 0;
						      rtd_outl(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
						}
#endif
					}
					else
					{

                        dolby_vsif_mode_change_count = 0;
#ifdef CONFIG_RTK_KDRV_DV
						if(0 != drvif_Hdmi_GetDolbyVsInfoFrame(&hdmi_dolby_vsi_content/*dolby_vsif_get_address()*/))
						{
							rtd_pr_vgip_isr_debug("vfe_hdmi_drv_get_port_vsi_info error :%x\n",vfe_hdmi_drv_get_port_vsi_info(&hdmi_dolby_vsi_content/*dolby_vsif_get_address()*/));
							rtd_pr_vgip_isr_debug("VSI=%x, %x,%x,%x,%x,%x,%x\n", hdmi_dolby_vsi_content.header.type , hdmi_dolby_vsi_content.header.version, hdmi_dolby_vsi_content.header.length , hdmi_dolby_vsi_content.checksum,hdmi_dolby_vsi_content.ieee_reg_id[0],hdmi_dolby_vsi_content.ieee_reg_id[1],hdmi_dolby_vsi_content.ieee_reg_id[2]/* ,hdmi_dolby_vsi_content.payload[0],hdmi_dolby_vsi_content.payload[1],dolby_vsif_get_address()->payload[0], dolby_vsif_get_address()->payload[1]*/);
							rtd_pr_vgip_isr_debug("payload[0]=%x\n",hdmi_dolby_vsi_content.payload[0]);

							if((get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_VSIF_LL)&& (get_HDMI_HDR_mode() == HDR_DOLBY_HDMI))
							{
#ifdef	CONFIG_DOLBY_IDK_1_6_1_1
								dolby_adapter_dolby_trigger_timer6();
#else
								dolby_trigger_timer6();
#endif
							}
						}
#endif

					}
					if (pre_dolby_vsem_type != dolby_adapter_get_dolby_vsem_type(port)) {
						if (pre_dolby_vsem_type == INIT_DOLBY_MODE) {
							pre_dolby_vsem_type = INIT2_DOLBY_MODE;
						} else if (pre_dolby_vsem_type == INIT2_DOLBY_MODE) {
							pre_dolby_vsem_type = dolby_adapter_get_dolby_vsem_type(port);
						} else {
							rtd_pr_vgip_isr_info("[%s:%d][Dolby] VSEM %d -> %d\n", __func__, __LINE__,
									pre_dolby_vsem_type,
									dolby_adapter_get_dolby_vsem_type(port));

							flag_dolby_vsif_change = 1;

							pre_dolby_vsem_type = dolby_adapter_get_dolby_vsem_type(port);
							dolby_mode_change_triger_mute = TRUE;
							drvif_Hdmi_SetDolbyVsifUpdateNoPktThreshold(NO_DVS_INFO_MAX);
							drvif_Hdmi_set_infoframe_thread_stop_nonseamphore_for_isr(FALSE);
						}
					}
                                        update_hdmi_qms_info_shamemory();//update QMS info to video fw
				}
                }
		else
		{
			if(get_scaler_qms_mode_flag())
				update_hdmi_qms_info_shamemory();//update QMS info to video fw when isr doesn't update the info yet

		}
	}
#endif
	if(handle_h3ddma_Cap_last_wr_flag)
	{
#if 0//no need for hw mode
		if(get_scaler_qms_mode_flag())
		update_qms_i3ddma_block_write_idx();//check and update qms write index
#endif
	}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#ifdef CONFIG_HDR_SDR_SEAMLESS

		if(get_i3ddma_change_format_flag())
		{
        if(handle_h3ddma_Cap_last_wr_flag || (handle_ch1_sta_flag && (get_hdmi_4k_hfr_mode() != HDMI_NON_4K120)))
        {//4k120 no use i3ddma so, has no i3ddma capture isr
			seamless_info_sharememory = (seamless_change_sync_info *)Scaler_GetShareMemVirAddr(SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_FLAG);
		    if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI))
            {
            	if(seamless_info_sharememory)
            	{
	                if((seamless_info_sharememory->hdmi_not_ready == 0) &&
						(get_hdr_semaless_active())
					)
	                {
                        if(decide_seamless_freeze_condition() && RPC_SysInfo && (RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Status] == 0) && (get_hdmi_4k_hfr_mode() == HDMI_NON_4K120))
                        {//memc freeze first
                            if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
                            {
                                rtd_pr_vgip_isr_notice("DM_freeze, freeze hdmi seamless,\n");
                                MEMC_Lib_Freeze(1);
                            }
                            else
                            {
                                rtd_pr_vgip_isr_notice("DM_freeze, no need freeze hdmi seamless not active\n");
                            }
                            memc_freeze_record_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
                            RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Status] = 1; /* enable*/
                            RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Cnt] = Seamless_Freeze_Frames_Max;
                        }
                        else
                        {
                            if(RPC_SysInfo)
                            {
                                RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Cnt] = Seamless_Freeze_Frames_Max;
                            }
                            spin_lock(get_i3ddma_change_spinlock());//lock i3ddma change spinlock
                            set_i3ddma_change_format_flag(FALSE);
                            i3ddma_target_color_format = seamless_info_sharememory->hdmi_target_color_format;
                            seamless_info_sharememory->hdmi_trigger_vo_hdr_mode = Scaler_ChangeUINT32Endian(seamless_info_sharememory->hdmi_target_hdr_info);
                            seamless_info_sharememory->hdmi_not_ready = 1;
                            hdmi_position_seamless = TRUE;//need to update pq and vtop 422 to 444
                            spin_unlock(get_i3ddma_change_spinlock());//unlock i3ddma change  spinlock
                            switch(i3ddma_target_color_format)
                            {//transfer I3DDMA_COLOR_SPACE_T to VO_CHROMA_FMT
                                case I3DDMA_COLOR_YUV422:
                                seamless_info_sharememory->hdmi_trigger_vo_color_format = Scaler_ChangeUINT32Endian(VO_CHROMA_YUV422);
                                break;

                                case I3DDMA_COLOR_YUV444:
                                seamless_info_sharememory->hdmi_trigger_vo_color_format = Scaler_ChangeUINT32Endian(VO_CHROMA_YUV444);
                                break;

                                case I3DDMA_COLOR_RGB:
                                seamless_info_sharememory->hdmi_trigger_vo_color_format = Scaler_ChangeUINT32Endian(VO_CHROMA_RGB888);
                                break;

                                default:
                                seamless_info_sharememory->hdmi_trigger_vo_color_format = Scaler_ChangeUINT32Endian(VO_CHROMA_YUV422);
                                break;
                            }

                            if(get_support_vo_force_v_top(support_sdr_max_rgb) == FALSE)//only false. vo need to change setting by seamless.
                            {
                                seamless_info_sharememory->hdmi_trigger_vo_change_flag = Scaler_ChangeUINT32Endian(1);//set change flag let vo change// need to update vo setting
                                /*//no need
                                if(!dvrif_i3ddma_compression_get_enable()){//if true, no need to i3ddma color setting
                                I3ddma_format_changed_seamless(get_backup_hdmi_info(), i3ddma_target_color_format);//i3ddma synamic change color
                                }
                                */
                            }
						}
	                }
            	}
            }
            else
            {//already leave hdmi
    			spin_lock(get_i3ddma_change_spinlock());//lock i3ddma change spinlock
    			if(seamless_info_sharememory)
    			{
	    			seamless_info_sharememory->hdmi_not_ready = 0;
					seamless_info_sharememory->hdmi_trigger_vo_change_flag = 0;
    			}
				set_i3ddma_change_format_flag(FALSE);
    			spin_unlock(get_i3ddma_change_spinlock());//unlock i3ddma change  spinlock
            }

		}
	}

	if(handle_ch1_sta_flag)
	{//seamless source (hdmi or vdec) record capture block and clear m capture done status for freeze
		if(VSC_INPUTSRC_HDMI == Get_DisplayMode_Src(SLR_MAIN_DISPLAY) && hdmi_position_seamless && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) && get_mdomain_driver_status())
		{//currently only hdmi need to clear m domain status for game mode seamless freeze
			clear_m_cap_done_status(SLR_MAIN_DISPLAY);//clear status for freeze check cap status
		}
		record_current_m_cap_block(SLR_MAIN_DISPLAY);//record m domain cpature block status for freeze use
	}

	if(handle_ch1_sta_flag && hdmi_position_seamless)
	{
		spin_lock(get_i3ddma_change_spinlock());//lock i3ddma change spinlock
		hdmi_position_seamless = FALSE;
		spin_unlock(get_i3ddma_change_spinlock());//unlock i3ddma change  spinlock
		seamless_info_sharememory = (seamless_change_sync_info *)Scaler_GetShareMemVirAddr(SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_FLAG);
		if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
		{
        	if(get_hdr_semaless_active())
        	{
        		if(seamless_info_sharememory && seamless_info_sharememory->hdmi_not_ready)
        		{
        			spin_lock(get_i3ddma_change_spinlock());//lock i3ddma change spinlock
					hdmi_finish_seamless = TRUE;
					spin_unlock(get_i3ddma_change_spinlock());//unlock i3ddma change  spinlock
					dynamic_change_hdr_setting_hdmi_position((Scaler_ChangeUINT32Endian((seamless_info_sharememory->hdmi_trigger_vo_hdr_mode)) & _BIT0)? TRUE : FALSE, i3ddma_target_color_format, Scaler_ChangeUINT32Endian((seamless_info_sharememory->hdmi_trigger_vo_hdr_mode)) >> 1);
        		}
        	}
			else
			{
				if(seamless_info_sharememory)
					seamless_info_sharememory->hdmi_not_ready = 0;
			}
		}
		else
        {//already leave hdmi //reset flag
			spin_lock(get_i3ddma_change_spinlock());//lock i3ddma change spinlock
			set_i3ddma_change_format_flag(FALSE);
			if(seamless_info_sharememory)
			{
				seamless_info_sharememory->hdmi_trigger_vo_change_flag = 0;
				seamless_info_sharememory->hdmi_not_ready = 0;
			}
			spin_unlock(get_i3ddma_change_spinlock());//unlock i3ddma change  spinlock
        }
	}
	if(handle_ch1_end_flag && hdmi_finish_seamless)
	{
		spin_lock(get_i3ddma_change_spinlock());//lock i3ddma change spinlock
		hdmi_finish_seamless = FALSE;
		spin_unlock(get_i3ddma_change_spinlock());//unlock i3ddma change  spinlock
		seamless_info_sharememory = (seamless_change_sync_info *)Scaler_GetShareMemVirAddr(SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_FLAG);
        if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) && (get_hdr_semaless_active()))
        {
        	if(seamless_info_sharememory && seamless_info_sharememory->hdmi_not_ready) {
                #if 0//move to before one frame
	           	if (RPC_SysInfo) {
					if(decide_seamless_freeze_condition())
					{
                        rtd_pr_vgip_isr_notice("DM_freeze, freeze hdmi seamless,\n");
                        MEMC_Lib_Freeze(1);
                        memc_freeze_record_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
                        RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Status] = 1; /* enable*/
                        RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Cnt] = Seamless_Freeze_Frames_Max;
                    }
	            }
                #endif
				dynamic_change_hdr_setting_hdmi_finish((Scaler_ChangeUINT32Endian((seamless_info_sharememory->hdmi_trigger_vo_hdr_mode)) & _BIT0)? TRUE : FALSE, i3ddma_target_color_format, Scaler_ChangeUINT32Endian((seamless_info_sharememory->hdmi_trigger_vo_hdr_mode)) >> 1);
        	}
        }
		spin_lock(get_i3ddma_change_spinlock());//lock i3ddma change spinlock
		set_i3ddma_change_format_flag(FALSE);
		if(seamless_info_sharememory)
			seamless_info_sharememory->hdmi_not_ready = 0;
		spin_unlock(get_i3ddma_change_spinlock());//unlock i3ddma change  spinlock

	}
    if(handle_ch1_end_flag)
    {
		seamless_info_sharememory = (seamless_change_sync_info *)Scaler_GetShareMemVirAddr(SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_FLAG);
        if(seamless_info_sharememory && seamless_info_sharememory->vdec_change_flag)
        {
            if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)
            {//run hdr change
                if(get_hdr_semaless_active())
                {
                    if (RPC_SysInfo) {
                        if(Scaler_ChangeUINT32Endian(seamless_info_sharememory->vdec_change_flag) == SEAMLESS_WITH_FREEZE)
                        {//need freeze
                            if(decide_seamless_freeze_condition())
                            {
                                if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
                                {
                                    rtd_pr_vgip_isr_notice("DM_freeze, freeze VDEC seamless,\n");
                                    MEMC_Lib_Freeze(1);
                                }
                                else
                                {
                                    rtd_pr_vgip_isr_notice("DM_freeze, no need freeze VDEC seamless not active\n");
                                }
                            }
                        }
                        else
                        {
                            set_display_forcebg_mask_by_isr(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_DTV_CHANNEL_CHANGE_HDR_SEAMLESS, 1);//set SLR_FORCE_BG_TYPE_DTV_CHANNEL_CHANGE_HDR_SEAMLESS mask
                        }
                        memc_freeze_record_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
                        RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Status] = 1; /* enable*/
                        RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Cnt] = Seamless_Freeze_Frames_Max;
                    }
                    dynamic_change_hdr_setting((seamless_info_sharememory->vdec_target_hdr_enable)? TRUE : FALSE, I3DDMA_COLOR_YUV422);
                    seamless_info_sharememory->vdec_change_flag = 0;
                }
			}
			else
			{
				seamless_info_sharememory->vdec_change_flag = 0;
			}
		}
        }

	if (RPC_SysInfo && handle_ch1_end_flag) {
		if (RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Status] == 1) {
			if (RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Cnt] != 0) {	/* cnt sattus*/
				RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Cnt] = ((RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Cnt]-1)<0)?(0):(RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Cnt]-1);
				//rtd_pr_vgip_isr_notice("[eli]cnt= %d, t= %d\n", pqSysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Cnt], scaler_rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
			} else {	/* un-freeze*/
                //printk("[eli]un-freeze, t= %d\n", scaler_rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
                //drv_memory_freeze_fw_mode(SLR_MAIN_DISPLAY, _DISABLE, get_current_m_cap_block(SLR_MAIN_DISPLAY));
                rtd_pr_vgip_isr_notice("DM_freeze, un-freeze,\n");
                MEMC_Lib_Freeze(0);
				RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Status] = 0;
				RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Cnt] = 0;
				if(Get_display_forcebg_mask(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_DTV_CHANNEL_CHANGE_HDR_SEAMLESS))
				{
					set_display_forcebg_mask_by_isr(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_DTV_CHANNEL_CHANGE_HDR_SEAMLESS, 0);//set SLR_FORCE_BG_TYPE_DTV_CHANNEL_CHANGE_HDR_SEAMLESS mask
					trigger_mute_off_by_isr(SLR_MAIN_DISPLAY, 1);//wake up to mute off
				}
			}
		}
	}

#endif
	if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) &&
	(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) && !(Scaler_VOFromVDEC(Scaler_Get_CurVoInfo_plane()) == SOURCE_DIRECTVO))
	{
		VIDEO_FW_CONTRL_MEMC_TYPE memc_freeze_type = get_video_fw_memc_freeze_type(SLR_MAIN_DISPLAY);
		if(memc_freeze_type == VIDEO_FW_CONTRL_MEMC_FREEZE)
		{
			rtd_pr_vgip_isr_notice("MEMC_freeze, freeze DTV\n");
			MEMC_Lib_Freeze(1);
			set_video_fw_memc_freeze_type(SLR_MAIN_DISPLAY,VIDEO_FW_CONTRL_MEMC_NONE);
		}
		else if(memc_freeze_type == VIDEO_FW_CONTRL_MEMC_UNFREEZE)
		{
			if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_FRAMESYNC) && !(IoReg_Read32(VODMA_VODMA_CLKGEN_reg) & VODMA_VODMA_CLKGEN_en_fifo_full_gate_mask))
				rtd_pr_vgip_isr_notice("MEMC_unfreeze, need wait data fs done\n");
			else {
				rtd_pr_vgip_isr_notice("MEMC_unfreeze, unfreeze DTV\n");
				MEMC_Lib_Freeze(0);
				set_video_fw_memc_freeze_type(SLR_MAIN_DISPLAY,VIDEO_FW_CONTRL_MEMC_NONE);
			}
		}
	}

	/* for techniColor and ST2094 frame sync ISR*/
	if (handle_ch1_sta_flag == 1 && SysInfo != NULL && Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) {
		if(RPC_system_info_structure_table->Project_ID != VIP_Project_ID_TV030)
			fwif_color_DM2_Hist_AutoMode_ch1_sta_ISR();
		fwif_color_ScalerVIP_TC_metadata_frameSync();
		fwif_color_ScalerVIP_ST2094_frameSync();
	}

#ifdef TVE_FUNCTION_ENABLE
	if(handle_ch2_end_flag && scaler_scart_out_isr_get_enable())
	{
		if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP))
		{
			if(ddr_in2status_reg.in2_block_select == 0)
			{
				if(scaler_get_tve_vflip_enable())
				{
					buf_even_addr = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2_2ndAddr_reg)+TVE_VFLIP_SIZE_OFFSET;
					buf_odd_addr = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2_3rdAddr_reg)+TVE_VFLIP_SIZE_OFFSET;
				}
				else
				{
					buf_even_addr = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2_2ndAddr_reg);
					buf_odd_addr = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2_3rdAddr_reg);
				}
			}
			else if(ddr_in2status_reg.in2_block_select==1)
			{
				if(scaler_get_tve_vflip_enable())
				{
					buf_even_addr = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2_3rdAddr_reg)+TVE_VFLIP_SIZE_OFFSET;
					buf_odd_addr = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2Addr_reg)+TVE_VFLIP_SIZE_OFFSET;
				}
				else
				{
					buf_even_addr = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2_3rdAddr_reg);
					buf_odd_addr = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2Addr_reg);
				}

			}
			else if(ddr_in2status_reg.in2_block_select==2)
			{
				if(scaler_get_tve_vflip_enable())
				{
					buf_even_addr = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2Addr_reg)+TVE_VFLIP_SIZE_OFFSET;
					buf_odd_addr = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2_2ndAddr_reg)+TVE_VFLIP_SIZE_OFFSET;
				}
				else
				{
					buf_even_addr = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2Addr_reg);
					buf_odd_addr = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2_2ndAddr_reg);
				}

			}

			if(TVE_syncgen_status_i_get_i_field(scaler_rtd_inl(TVE_syncgen_status_i_reg)) == 0)
			{
				rtd_pr_vgip_isr_debug("Enter even field![%d,%d,%x]\n",h3ddma_cap0_cap_status_reg.cap_field,h3ddma_cap0_cap_status_reg.cap_block_sel,buf_odd_addr);
				scaler_rtd_maskl(TVE_intpos_i_reg, ~(TVE_intpos_i_write_en2_mask|TVE_intpos_i_int_vpos_mask), TVE_intpos_i_write_en2(1) | TVE_intpos_i_int_vpos(625/2+2));
				scaler_rtd_outl(TVE_DMA_TVEVDDMA_SATRT0_reg, buf_odd_addr/8+0x5a);	  //  dma addr  //?M3d dma?t¡èF8-?
				scaler_rtd_outl(TVE_DMA_TVEVDDMA_SATRT1_reg, buf_odd_addr/8+0x5a);	  //  dma addr
				scaler_rtd_outl(TVE_DMA_TVEVDDMA_SATRT2_reg, buf_odd_addr/8+0x5a);	  //  dma addr
				scaler_rtd_outl(TVE_DMA_TVEVDDMA_SATRT3_reg, buf_odd_addr/8+0x5a);
			}
			else
			{
				rtd_pr_vgip_isr_debug("Enter odd  field![%d,%d,%x]\n",h3ddma_cap0_cap_status_reg.cap_field,h3ddma_cap0_cap_status_reg.cap_block_sel,buf_even_addr);
				scaler_rtd_maskl(TVE_intpos_i_reg, ~(TVE_intpos_i_write_en2_mask|TVE_intpos_i_int_vpos_mask), TVE_intpos_i_write_en2(1) | TVE_intpos_i_int_vpos(2));
				scaler_rtd_outl(TVE_DMA_TVEVDDMA_SATRT0_reg, buf_even_addr/8);	  //  dma addr  //?M3d dma?t¡èF8-?
				scaler_rtd_outl(TVE_DMA_TVEVDDMA_SATRT1_reg, buf_even_addr/8);	  //  dma addr
				scaler_rtd_outl(TVE_DMA_TVEVDDMA_SATRT2_reg, buf_even_addr/8);	  //  dma addr
				scaler_rtd_outl(TVE_DMA_TVEVDDMA_SATRT3_reg, buf_even_addr/8);
			}


		}
		else
		{
			if(ddr_in2status_reg.in2_block_select == 0)
			{
				if(scaler_get_tve_vflip_enable())
				{
					buf_prog_mode = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2_3rdAddr_reg)+TVE_VFLIP_SIZE_OFFSET;
				}
				else
				{
					buf_prog_mode = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2_3rdAddr_reg);
				}
			}
			else if(ddr_in2status_reg.in2_block_select == 1)
			{
				if(scaler_get_tve_vflip_enable())
				{
					buf_prog_mode = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2Addr_reg)+TVE_VFLIP_SIZE_OFFSET;
				}
				else
				{
					buf_prog_mode = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2Addr_reg);
				}

			}
			else if(ddr_in2status_reg.in2_block_select == 2)
			{
				if(scaler_get_tve_vflip_enable())
				{
					buf_prog_mode = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2_2ndAddr_reg)+TVE_VFLIP_SIZE_OFFSET;
				}
				else
				{
					buf_prog_mode = scaler_rtd_inl(MDOMAIN_CAP_DDR_In2_2ndAddr_reg);
				}

			}

			{
				rtd_pr_vgip_isr_debug("prog Enter even field![%d,%d,%x]\n",h3ddma_cap0_cap_status_reg.cap_field,h3ddma_cap0_cap_status_reg.cap_block_sel,buf_prog_mode);

				scaler_rtd_outl(TVE_DMA_TVEVDDMA_SATRT0_reg, buf_prog_mode/8+0xb4);	  //  dma addr  //?M3d dma?t¡èF8-?
				scaler_rtd_outl(TVE_DMA_TVEVDDMA_SATRT1_reg, buf_prog_mode/8+0xb4);	  //  dma addr
				scaler_rtd_outl(TVE_DMA_TVEVDDMA_SATRT2_reg, buf_prog_mode/8+0xb4);	  //  dma addr
				scaler_rtd_outl(TVE_DMA_TVEVDDMA_SATRT3_reg, buf_prog_mode/8+0xb4);
			}

		}
	}
#endif

#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
	if(handle_ch1_sta_flag) {
		static unsigned char b_hdmiDolby;

		if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI && get_HDMI_HDR_mode() == HDR_DOLBY_HDMI) {
			if(b_hdmiDolby) {
				HDR_DolbyVision_HDMI_Quick_Mute_Watch();
			} else {
				b_hdmiDolby = true;
				HDR_DolbyVision_HDMI_Quick_Mute_Reset();
			}

		} else {
			b_hdmiDolby = false;
			HDR_DolbyVision_HDMI_Quick_Mute_Reset();
		}
	}
#endif

//>>>Start 20171228, pinyen modify for Dolby HDMI to avoid pink gabage when the begining from SDR to DolbyHDR
#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)&& dolby_mode_change_triger_mute && handle_ch1_end_flag)//interrupt timing
{
	rtd_pr_vgip_isr_info("[%s:%d][Dolby] enbale forcebg by dolby mode change\n", __func__ ,__LINE__);

	set_display_forcebg_mask(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_DOLBY, TRUE); /* mute when dolby vsif change */
	set_vsc_check_dolby_mode_change_flag();

	rtd_maskl(PPOVERLAY_Main_Display_Control_RSV_reg, ~(_BIT1), _BIT1);
	ppoverlay_double_buffer_ctrl3_reg.regValue = rtd_inl(PPOVERLAY_Double_Buffer_CTRL3_reg);
	ppoverlay_double_buffer_ctrl3_reg.dmainreg_other_dbuf_en = 0;
	rtd_outl(PPOVERLAY_Double_Buffer_CTRL3_reg, ppoverlay_double_buffer_ctrl3_reg.regValue);

}
#endif
//<<<End 20171228, pinyen modify for Dolby HDMI to avoid pink gabage when the begining from SDR to DolbyHDR

	if(get_HDMI_HDR_mode() == HDR_DOLBY_HDMI){
#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
		if ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
				&& ((get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_VSIF_STD)
					|| (get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_h14B_VSIF))) {

			if((HDR_DolbyVision_Get_CurHdmiMode() > DOLBY_HDMI_OFF) && handle_ch1_end_flag){
				// send RPC to Linux kernel -- HDMI mode data end for B05 setting apply
				//HDR_DOLBY_VISION_HDMI_Send_Write_Index_RPC();
			}

			// only check md_parser data end interrupt when md_parser enabled
			if(((HDR_DolbyVision_Get_CurHdmiMode() > DOLBY_HDMI_OFF) && handle_h3ddma_Cap3_last_wr_flag)
					|| ((HDR_DolbyVision_Get_CurHdmiMode() == DOLBY_HDMI_OFF) && handle_dma_ch3_end_flag)) {

				if((HDR_DolbyVision_Get_CurHdmiMode() == DOLBY_HDMI_OFF) && handle_dma_ch3_end_flag) {
					pktCrc32Cnt=0; //reset crc count;
					hdmi_crc_freeze_status = hdmi_crc_status_init;//reset hdmi_crc_freeze_status, fix ML3RTANDN-1194
				}

				HDR_DolbyVision_HDMI_handler();
			}/*else if((HDR_DolbyVision_Get_CurHdmiMode() > DOLBY_HDMI_OFF) && handle_ch1_end_flag){
			// send RPC to Linux kernel -- HDMI mode data end for B05 setting apply
			HDR_DOLBY_VISION_HDMI_Send_Write_Index_RPC();
			}*/
		}

#endif
	}

	if(rtd_inl(VGIP_VGIP_VBISLI_reg)&_BIT24){
		if(handle_ch1_sta_flag==1)
			drv_memory_debug_print_buf_idx();
	}

#ifdef CONFIG_I2RND_ENABLE
	if(handle_ch1_sta_flag && main_repeat_mask_pre){
		//reset repeat mask
		vodma_i2rnd_st_reg.regValue= rtd_inl(VODMA_vodma_i2rnd_st_reg);
		if(vodma_i2rnd_st_reg.i2rnd_st == 0x3){
			vo_i2rnd_m_flag.regValue = IoReg_Read32(VODMA_vodma_i2rnd_m_flag_reg);
			if(main_repeat_mask_pre == 1)
			{
				vo_i2rnd_m_flag.i2rnd_hw_mask_repeat_frame_m_en = 1;
				IoReg_Write32(VODMA_vodma_i2rnd_m_flag_reg, vo_i2rnd_m_flag.regValue);
				main_repeat_mask_pre =0;
			}
			else
			{
				main_repeat_mask_pre --;
			}
		}
	}

	if(handle_ch1_sta_flag && sub_repeat_mask_pre){
		//reset s1 repeat mask
		vodma_i2rnd_st_reg.regValue= rtd_inl(VODMA_vodma_i2rnd_st_reg);
		if(vodma_i2rnd_st_reg.i2rnd_st == 0x5){
			vo_i2rnd_s_flag.regValue = IoReg_Read32(VODMA_vodma_i2rnd_s_flag_reg);
			if(sub_repeat_mask_pre == 1)
			{
				vo_i2rnd_s_flag.i2rnd_hw_mask_repeat_frame_s_en = 1;
				IoReg_Write32(VODMA_vodma_i2rnd_s_flag_reg, vo_i2rnd_s_flag.regValue);
				sub_repeat_mask_pre =0;
			}
			else
			{
				sub_repeat_mask_pre --;
			}
		}
	}

	if(get_i2rnd_pst_write_pnt_counter()){
		if(handle_ch1_sta_flag){
			vodma_i2rnd_st_reg.regValue= IoReg_Read32(VODMA_vodma_i2rnd_st_reg);
			vo_i2rnd_m_flag.regValue = IoReg_Read32(VODMA_vodma_i2rnd_m_flag_reg);
			//main path
			if(vodma_i2rnd_st_reg.i2rnd_st == 0x3){
				//check main repeat mask
				if((vo_i2rnd_m_flag.i2rnd_hw_mask_repeat_frame_m_en == 1)||(main_repeat_mask_pre && (vo_i2rnd_m_flag.i2rnd_hw_mask_repeat_frame_m_en == 0))){
					main_repeat_mask_pre = main_repeat_mask_pre + get_i2rnd_pst_write_pnt_counter();
					vo_i2rnd_m_flag.i2rnd_hw_mask_repeat_frame_m_en = 0;
					IoReg_Write32(VODMA_vodma_i2rnd_m_flag_reg, vo_i2rnd_m_flag.regValue);
				}
				//for debug
				//vgip_vgip_chn1_lc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_LC_reg);
				//new_meas1_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);
				//[ANDROIDTV-3561]remove set display index which include semaphore make inerrupt abonormal. @Crixus 20171026
				if (Scaler_I2rnd_get_timing_enable()) {
					Scaler_i2rnd_set_write_point(I2RND_MAIN_S0_TABLE);
#ifdef CONFIG_PST_ENABLE
					Scaler_pst_set_write_point(_MAIN_MD);
#endif
				}
				//for debug
				//rtd_pr_vgip_isr_emerg("[crixus][vgip line_cnt = %d][uzudtg line cnt = %d]s0_i2r write num = %d, pst_main write_cmd_num = %d\n", vgip_vgip_chn1_lc_reg.ch1_line_cnt, new_meas1_linecnt_real_reg.uzudtg_line_cnt_rt, Scaler_I2rnd_get_write_cmd_num(I2RND_MAIN_S0_TABLE), Scaler_pst_main_md_get_write_cmd_num());
				//Eric@0310 copy current buffer to all buffer after write point changed
				//Scaler_i2rnd_buffer_copy();
				Scaler_pst_buffer_copy(_MAIN_MD);
				set_i2rnd_pst_write_pnt_counter(0);
				//set_i2rnd_s0_pst_check_stage(MAIN_PST_TOP_CTRL_STAGE_CHECK_RW_PTR);
			}
		}
	}

	//s1 pst @Crixus 20171113
	if(get_i2rnd_s1_pst_write_pnt_counter()){
		if(handle_ch1_sta_flag){
			vodma_i2rnd_st_reg.regValue= IoReg_Read32(VODMA_vodma_i2rnd_st_reg);
			vo_i2rnd_s_flag.regValue = IoReg_Read32(VODMA_vodma_i2rnd_s_flag_reg);
			//sub path
			if(vodma_i2rnd_st_reg.i2rnd_st == 0x5){
				//check main repeat mask
				if((vo_i2rnd_s_flag.i2rnd_hw_mask_repeat_frame_s_en == 1)||(sub_repeat_mask_pre && (vo_i2rnd_s_flag.i2rnd_hw_mask_repeat_frame_s_en == 0))){

					sub_repeat_mask_pre = sub_repeat_mask_pre + get_i2rnd_pst_write_pnt_counter();
					vo_i2rnd_s_flag.i2rnd_hw_mask_repeat_frame_s_en = 0;
					IoReg_Write32(VODMA_vodma_i2rnd_s_flag_reg, vo_i2rnd_s_flag.regValue);
				}
				//for debug
				//mdomain_cap_ddr_in1status_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg);
				//mdomain_disp_ddr_substatus_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubStatus_reg);
				//vgip_vgip_chn1_lc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_LC_reg);
				//new_meas1_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);

				//[ANDROIDTV-3561]remove set display index which include semaphore make inerrupt abonormal. @Crixus 20171026
				if (Scaler_I2rnd_get_timing_enable()) {
					Scaler_i2rnd_set_write_point(I2RND_SUB_S1_TABLE);
#ifdef CONFIG_PST_ENABLE
					Scaler_pst_set_write_point(_SUB_MD);
#endif
				}
				//for debug
				//rtd_pr_vgip_isr_emerg("[crixus][vgip line_cnt = %d][uzudtg line cnt = %d]s1_i2r write num = %d, pst_sub write_cmd_num = %d\n", vgip_vgip_chn1_lc_reg.ch1_line_cnt, new_meas1_linecnt_real_reg.uzudtg_line_cnt_rt, Scaler_I2rnd_get_write_cmd_num(I2RND_SUB_S1_TABLE), Scaler_pst_sub_md_get_write_cmd_num());
				//rtd_pr_vgip_isr_emerg("cap_in1_sub = %d, sub_disp = %d\n", mdomain_cap_ddr_in1status_reg.in1_sub_block_select, mdomain_disp_ddr_substatus_reg.sub_block_select);
				Scaler_pst_buffer_copy(_SUB_MD);
				set_i2rnd_s1_pst_write_pnt_counter(0);
				//set_i2rnd_s1_pst_check_stage(SUB_PST_TOP_CTRL_STAGE_CHECK_RW_PTR);
			}
		}
	}
#endif

#if 0 //debug : bypass
	// Check if handled IRQ
	if (handle_ch1_sta_flag || handle_ch1_end_flag)
		scaler_rtd_outl(VGIP_VGIP_CHN1_STATUS_reg, vgip_chn1_status_reg.regValue);	//write clear status

	if (handle_dma_ch3_sta_flag || handle_dma_ch3_end_flag)
		scaler_rtd_outl(DMA_VGIP_DMA_VGIP_STATUS_reg, dma_vgip_chn1_status_reg.regValue);	//write clear status

	#if 1// sub channel
		if(handle_ch2_sta_flag || handle_ch2_end_flag)
			scaler_rtd_outl(SUB_VGIP_VGIP_CHN2_STATUS_reg, vgip_chn2_status_reg.regValue);	//write clear status
	#endif

	if(handle_ch1_sta_flag
		 || handle_ch2_sta_flag
		 || handle_ch2_end_flag
		 || handle_dma_ch3_sta_flag
		 || handle_dma_ch3_end_flag){
		return IRQ_HANDLED;
	}else{
		return IRQ_NONE;
	}
#endif



#ifdef debug_ISR
	if((reg_value & BIT(27))){

		time0s_cnt++;

		line_info = rtd_inl(VGIP_VGIP_CHN1_LC_reg);
		line_cnt = ((line_info & 0x00FFF000)>>12);

		time0 = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
		if(time0s_set==0) {
			time0s_start = time0;
			time0s_set=1;
			time0s_cnt=0;
		}
		time0s_end = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);

		if((time0s_end-time0s_start)>=90000) {
			rtd_pr_vgip_isr_debug("[VCPU]VGIP isr runs %d times=%d\n\n",time0s_cnt, (time0s_end-time0s_start));
			rtd_pr_vgip_isr_debug("[VCPU]VGIP isr line = %d\n\n", line_cnt);
			rtd_pr_vgip_isr_debug("[VCPU]VGIP time_cnt1 = %d\n\n", time_cnt1);
			rtd_pr_vgip_isr_debug("[VCPU]VGIP sta_flag = %d,end_flag=%d \n\n", handle_ch1_sta_flag, handle_ch1_end_flag);
			rtd_pr_vgip_isr_debug("[VCPU]VGIP start_ie = %d,vact_start=%d \n\n", vgip_chn1_ctrl_reg.ch1_vact_start_ie, vgip_chn1_status_reg.ch1_vact_start);
			rtd_pr_vgip_isr_debug("[VCPU]VGIP end_ie = %d,vact_end=%d \n\n", vgip_chn1_ctrl_reg.ch1_vact_end_ie, vgip_chn1_status_reg.ch1_vact_end);
			time0s_set = 0;
		}

	}
#endif

	//===============================
	// Handle Channel 1 VactStart Interrupt
	//===============================
	if ( handle_ch1_sta_flag){

#ifdef debug_ISR

if((reg_value & BIT(29))){

	exe_cnt++;
	time1s_cnt++;

	line_info = rtd_inl(VGIP_VGIP_CHN1_LC_reg);
	line_cnt = ((line_info & 0x00FFF000)>>12);

	time1 = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
	if(time1s_set==0) {
		time1s_start = time1;
		time1s_set=1;
		time1s_cnt=0;
	}
	time1s_end = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);

	if(exe_cnt%120==0) {
		rtd_pr_vgip_isr_debug("[VCPU][sta]VGIP isr line = %d\n\n", line_cnt);
		exe_cnt = 0;
	}

	if((time1s_end-time1s_start)>=90000) {
		rtd_pr_vgip_isr_debug("[VCPU][sta]VGIP isr runs %d times=%d\n\n",time1s_cnt, (time1s_end-time1s_start));
		rtd_pr_vgip_isr_debug("[VCPU][sta]VGIP isr line = %d\n\n", line_cnt);
		rtd_pr_vgip_isr_debug("[VCPU][sta]VGIP time_cnt1 = %d\n\n", time_cnt1);
		time1s_set = 0;
	}

}

#else

	if(I2R_PVR_MAIN_flag){
		/*==Film mode check==*/
		if (Scaler_Get_InputSrcInit() == NULL || Scaler_DispGetInfoPtr() ==NULL ){ // wait init
			; // do nothing
		} else {
			if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE){ //in active area
				if ( fwif_color_GetAutoMAFlag() && Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE)) { // Enable auto_ma and interlace input
					scalerVIP_film_FWmode_film_reset();
					#if 0
						unsigned char isrc = Scaler_InputSrcGetType(SLR_MAIN_DISPLAY);
						if (isrc == _SRC_VO || isrc == _SRC_USB || isrc == _SRC_WIFI) {

							extern FILM_FW_ShareMem_ISR *g_Share_Memory_FILM_FW;
							extern unsigned char GMV_status_for_Filim;

							if (g_Share_Memory_FILM_FW) {
								if(GMV_status_for_Filim == 0){
									g_Share_Memory_FILM_FW->st_DI_STATUS_VPAN = scalerVIP_DI_MiddleWare_GetStatus(STATUS_VPAN);
								} else {
									g_Share_Memory_FILM_FW->st_DI_STATUS_VPAN = 1;
								}
								g_Share_Memory_FILM_FW->flag_Trigger = 0x1;
							}
						}
					#else
						scalerVIP_film_FWmode();  // FW film
					#endif
				}
			}
		}
	}
#endif
			vgip_chn1_status_reg.ch1_vact_start = 1;	//write clear status

	}// end if handle_ch1_sta_flag

	//===============================
	// Handle Channel 1 VactEndInterrupt
	//===============================
#ifndef debug_ISR
	if(I2R_PVR_MAIN_flag && (handle_ch1_sta_flag || handle_ch1_end_flag || handle_ch2_sta_flag || handle_ch2_end_flag || handle_dma_ch3_sta_flag || handle_dma_ch3_end_flag)){
		if((scalerVIP_Get_PT_NO()!=0)&&(!vgip_chn1_ctrl_reg.ch1_vact_end_ie)) {
			scalerVIP_Set_Parameter_forLGM2(0); // reset value
		}
	}
#endif


	if ( handle_ch1_end_flag) {
#ifdef debug_ISR

if((reg_value & BIT(30))){

	exe_cnt_2++;
	time2s_cnt++;

	line_info = rtd_inl(VGIP_VGIP_CHN1_LC_reg);
	line_cnt = ((line_info & 0x00FFF000)>>12);

	time2 = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
	if(time2s_set==0) {
		time2s_start = time1;
		time2s_set=1;
		time2s_cnt=0;
	}
	time2s_end = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);

	if(exe_cnt_2%120==0) {
		rtd_pr_vgip_isr_debug("[VCPU][end]VGIP isr line = %d\n\n", line_cnt);
		exe_cnt_2 = 0;
	}

	if((time2s_end-time2s_start)>=90000) {
		rtd_pr_vgip_isr_debug("[VCPU][end]VGIP isr runs %d times=%d\n\n",time2s_cnt, (time2s_end-time2s_start));
		rtd_pr_vgip_isr_debug("[VCPU][end]VGIP isr line = %d\n\n", line_cnt);
		rtd_pr_vgip_isr_debug("[VCPU][end]VGIP time_cnt1 = %d\n\n", time_cnt1);
		time2s_set = 0;
	}

}

#else
	tasklet_schedule(&pq_automa_tasklet);
#endif

#ifdef CONFIG_ENABLE_HDMI_NN
				// Check NN write done
				//h3ddma_cap1_cap_status_reg.regValue = IoReg_Read32(H3DDMA_CAP1_Cap_Status_reg);
				if ((get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1) || (get_hdmi_4k_hfr_mode() == HDMI_3K120))
				{//need check
					scaler_I3ddma_ISR();
					//h3ddma_nn_set_buffer_addr(0);
					h3ddma_set_file_mode_buffer_status();

					//IoReg_Write32(H3DDMA_CAP1_Cap_Status_reg, h3ddma_cap1_cap_status_reg.regValue);
				}
#endif

			vgip_chn1_status_reg.ch1_vact_end = 1;	//write clear status

	}

	if(get_i3ddma_4k120_flag())
	{
		if(handle_h3ddma_Cap_last_wr_flag)
		{
			update_i3ddma_4k120_cap_isr();
			//IoReg_Write32(H3DDMA_CAP0_Cap_Status_reg, h3ddma_cap0_cap_status_reg.regValue);
			i3ddma_cap_isr_cnt ++;
			hdmi_4k120_se_wakeup();
			//rtd_pr_vgip_isr_emerg("vgip_isr:i3ddma_cap_isr_cnt=%d\n",i3ddma_cap_isr_cnt);
		}
	}

#ifdef CONFIG_FRAMERATE_DEBUG_IN_VGIPISR
	vbe_disp_debug_framerate_change_msg();
#endif

#ifndef CONFIG_SFG_PATTERN_MEASUREMENT
#ifndef CONFIG_UZU_PATTERN_MEASUREMENT
	//Eric@20180626 vdec source need trigger in vgip start isr and use vgip pattern gen
	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) || ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) && ((get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1) || (get_hdmi_4k_hfr_mode() == HDMI_3K120)))) {
		if(handle_ch1_sta_flag)
		{
			vdec_video_latency_pattern_ctrl();//control latency pattern
		}
	}
	//Add game mode measurement @Crixus 20170621
#ifdef CONFIG_I3DDMA_PATTERN_MEASUREMENT
		else if(handle_dma_ch3_sta_flag)
		{
			video_latency_pattern_ctrl();//control latency pattern
		}
#endif
#endif
#endif


	//===============================
	// clear DMA VGIP interrupt status
	//===============================
	if(handle_dma_ch3_sta_flag)
		dma_vgip_chn1_status_reg.dma_vact_start = 1;

	if(handle_dma_ch3_end_flag)
		dma_vgip_chn1_status_reg.dma_vact_end = 1;

	if(handle_h3ddma_Cap_last_wr_flag)
		h3ddma_cap0_cap_status_reg.cap0_cap_last_wr_flag = 1;


	if(handle_h3ddma_Cap3_last_wr_flag)
		h3ddma_cap3_cap_status_reg.cap3_cap_last_wr_flag = 1;


/*********sub channel************/
	#if 1//sub channel
	//===============================
	// Handle Channel 1 VactStart Interrupt
	//===============================
		if ( handle_ch2_sta_flag ){
			#if 0
			#ifdef APVR_ISR_DEBUG
			apvr_cur_time=rtd_inl(TIMER_SCPU_CLK90K_LO_reg)/90;
			apvr_cur_field = idma_get_field_mode_sub();
			if((apvr_cur_field == apvr_pre_field) || ((apvr_cur_time-apvr_pre_time) > 30))
			{
				rtd_pr_vgip_isr_emerg("@@@@@apvr sub vgip isr same cur_field:%d, pre_field:%d, timeintval:%d@@@@@\n", apvr_cur_field,apvr_pre_field,apvr_cur_time-apvr_pre_time);
			}
			apvr_pre_time = apvr_cur_time;
			apvr_pre_field= apvr_cur_field;
			#endif
			// enable a2tov enable
			rtd_outl(REG_SB2_CPU_INT_EN, _BIT28|_BIT0);
			// enable a2tov status
			rtd_outl(REG_SB2_CPU_INT, _BIT28|_BIT0);
			#endif
			vgip_chn2_status_reg.ch2_vact_start = 1;

		}// end if handle_ch2_sta_flag

	//===============================
	// Handle Channel 2 VactEnd Interrupt
	//===============================
		if ( handle_ch2_end_flag ){

			vgip_chn2_status_reg.ch2_vact_end = 1;
		}// end if handle_ch2_end_flag
	#endif

	//===============================
	// Handle SDNR VactEnd Interrupt
	//===============================
	if(handle_ch1_sdnr_act_end_flag){
		vgip_chn1_status_reg.ch1_sdnr_vact_end = 1;
	}

	if(handle_ch2_sdnr_act_end_flag){
		vgip_chn2_status_reg.ch2_sdnr_vact_end = 1;
	}


	// Check if handled IRQ
	if (handle_ch1_sta_flag || handle_ch1_end_flag || handle_ch1_sdnr_act_end_flag)
		scaler_rtd_outl(VGIP_VGIP_CHN1_STATUS_reg, vgip_chn1_status_reg.regValue);	//write clear status

	if (handle_dma_ch3_sta_flag || handle_dma_ch3_end_flag)
		scaler_rtd_outl(DMA_VGIP_DMA_VGIP_STATUS_reg, dma_vgip_chn1_status_reg.regValue);	//write clear status

	if(handle_h3ddma_Cap_last_wr_flag)
		scaler_rtd_outl(H3DDMA_CAP0_Cap_Status_reg, h3ddma_cap0_cap_status_reg.regValue);

	if (handle_h3ddma_Cap3_last_wr_flag)
		scaler_rtd_outl(H3DDMA_CAP3_Cap_Status_reg, h3ddma_cap3_cap_status_reg.regValue);

	#if 1// sub channel
	if(handle_ch2_sta_flag || handle_ch2_end_flag || handle_ch2_sdnr_act_end_flag)
		scaler_rtd_outl(SUB_VGIP_VGIP_CHN2_STATUS_reg, vgip_chn2_status_reg.regValue);	//write clear status
	#endif

#endif

	if(handle_ch1_sta_flag
		 || handle_ch1_end_flag
		 || handle_ch2_sta_flag
		 || handle_ch2_end_flag
		 || handle_dma_ch3_sta_flag
		 || handle_dma_ch3_end_flag
		 || handle_h3ddma_Cap_last_wr_flag
		 || handle_h3ddma_Cap3_last_wr_flag
		 || handle_ch1_sdnr_act_end_flag
		 || handle_ch2_sdnr_act_end_flag){
		return IRQ_HANDLED;
	}else{
		return IRQ_NONE;
	}
}

static int irq_vgip_isr = -1;
static int __init vgip_isr_init_irq(struct platform_device *pdev)
{
	/* Request IRQ */

	int irq, ret = 0;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	irq_vgip_isr = irq;


	ret = request_irq(irq, vgip_isr, IRQF_SHARED,
					"VGIP ISR", (void *)pdev);
	if (ret)
	{
		rtd_pr_vgip_isr_err("vgip_isr: cannot register IRQ. ret=%d\n", ret);
		return ret;
	}

	ret = request_irq(irq, OnlineMeasureISR, IRQF_SHARED,
					"OnlineMeasure ISR", (void *)"ONMS_ISR");
    if (ret)
	{
		rtd_pr_vgip_isr_err("online_isr_platform_driver_isr: cannot register IRQ .ret=%d\n", ret);
		free_irq(irq, (void *)pdev);
		return ret;
	}

	irq_vgip_isr = irq;

 	// disable route to VCPU
	rtd_outl(SYS_REG_INT_CTRL_VCPU_reg, _BIT2);
	// enable route to SCPU
	rtd_outl(SYS_REG_INT_CTRL_SCPU_reg,_BIT2|_BIT0);
	// enable VGIP interrupt
	rtd_setbits(VGIP_VGIP_CHN1_CTRL_reg, BIT(25));

	rtd_pr_vgip_isr_info("vgip_isr: register IRQ (hwirq=%lu, virq=%d)\n", irq_to_desc(irq)->irq_data.hwirq, irq);

	return 0;
}
#if 0
static char *vgip_isr_devnode(struct device *dev, mode_t *mode)
{
	return NULL;
}
#endif

static int __init vgip_isr_probe(struct platform_device *pdev)
{
	int result;

	result = vgip_isr_init_irq(pdev);
	if (result < 0) {
		rtd_pr_vgip_isr_err("vgip_isr: can not register irq...\n");
		return result;
	}

	vgip_isr_platform_devs = pdev;

	return 0;
}


static int __exit vgip_isr_remove(struct platform_device *pdev)
{
	if (irq_vgip_isr > -1) {
		free_irq(irq_vgip_isr, (void *)pdev);
		irq_vgip_isr = -1;
	}

	vgip_isr_platform_devs = NULL;

	return 0;
}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
int __init vgip_isr_init_module(void)
#else
static int __init vgip_isr_init_module(void)
#endif
{

#if 0
	int result;
	dev_t dev = MKDEV(VGIP_ISR_MAJOR, 0);

	rtd_pr_vgip_isr_notice("vgip_isr: vgip_isr driver for Realtek Media Processors(2016/04/28)\n");

	result = register_chrdev_region(dev, 1, "vgip_isr");
	if (result < 0) {
		rtd_pr_vgip_isr_debug("vgip_isr: can not get chrdev region...\n");
		return result;
	}
#if 0 //. remove for osd
	vgip_isr_class = class_create(THIS_MODULE, "vgip_isr");
	if (IS_ERR(vgip_isr_class)) {
		rtd_pr_vgip_isr_debug("vgip_isr: can not create class...\n");
		result = PTR_ERR(vgip_isr_class);
		goto fail_class_create;
	}
#endif

	vgip_isr_class->devnode = vgip_isr_devnode;

	vgip_isr_platform_devs = platform_device_register_simple("vgip_isr", -1, NULL, 0);
	if (platform_driver_register(&vgip_isr_device_driver) != 0) {
		rtd_pr_vgip_isr_debug("vgip_isr: can not register platform driver...\n");
		result = -EINVAL;
		goto fail_platform_driver_register;
	}

	result = vgip_isr_init_irq();
	if (result < 0) {
		rtd_pr_vgip_isr_err("vgip_isr: can not register irq...\n");
		return result;
	}

	cdev_init(&vgip_isr_cdev, &vgip_isr_fops);
	vgip_isr_cdev.owner = THIS_MODULE;
	vgip_isr_cdev.ops = &vgip_isr_fops;
	result = cdev_add(&vgip_isr_cdev, dev, 1);
	if (result < 0) {
		rtd_pr_vgip_isr_debug("vgip_isr: can not add character device...\n");
		goto fail_cdev_init;
	}
	device_create(vgip_isr_class, NULL, dev, NULL, "vgip_isr");

	return 0;

fail_cdev_init:
	platform_driver_unregister(&vgip_isr_device_driver);
fail_platform_driver_register:
	platform_device_unregister(vgip_isr_platform_devs);
	vgip_isr_platform_devs = NULL;
	class_destroy(vgip_isr_class);
fail_class_create:
	vgip_isr_class = NULL;
	unregister_chrdev_region(dev, 1);
	return result;

#else
	int result;
	if (platform_driver_probe(&vgip_isr_platform_driver, vgip_isr_probe) != 0) {
		rtd_pr_vgip_isr_debug("vgip_isr: can not register platform driver...\n");
		result = -EINVAL;
		return result;
	}

	return 0;
#endif

}

void __exit vgip_isr_exit_module(void)
{
	platform_driver_unregister(&vgip_isr_platform_driver);
}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(vgip_isr_init_module);
module_exit(vgip_isr_exit_module);
#endif

