//Copyright (C) 2007-2013 Realtek Semiconductor Corporation.
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <mach/platform.h>

#ifdef CONFIG_OF
#include <linux/of_irq.h>
#include <linux/of_address.h>
#endif

#include "rtk_ddomain_isr.h"
#include <rbus/ppoverlay_reg.h>
#include <rbus/vodma_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/vgip_reg.h>
#include <rbus/dma_vgip_reg.h>
#include <rtk_ai.h>
#include <scaler/vipRPCCommon.h>
#include <scaler/scalerCommon.h>
#include <rbus/blu_reg.h>
#include <rbus/mdomain_cap_reg.h>
#include <rbus/mdomain_disp_reg.h>
#include "vgip_isr/scalerAI.h"
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/still_logo_detect.h>
#include <tvscalercontrol/vip/scalerPQMaskCtrl.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <vgip_isr/scalerVIP.h>
#include <vgip_isr/scalerDCC.h>
#include <base_types_sub.h>
#include <tvscalercontrol/vip/ai_pq.h>
// for register dump
#include <tvscalercontrol/io/ioregdrv.h>
#include <rbus/ldspi_reg.h>

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

//extern webos_info_t  webos_tooloption;
//extern webos_strInfo_t webos_strToolOption;
/* Function Prototype */
static unsigned int mv_den_end_count;
static unsigned int mv_den_sta_count;
static int ddomain_isr_suspend(struct platform_device *dev, pm_message_t state);
static int ddomain_isr_resume(struct platform_device *dev);
extern unsigned char invoutputgamma_isr_flag;

#ifdef CONFIG_OF
static const struct of_device_id of_ddomain_isr_ids[] = {
	{ .compatible = "realtek,ddomain_isr" },
	{}
};

MODULE_DEVICE_TABLE(of, of_ddomain_isr_ids);
#endif

static struct platform_device *ddomain_isr_platform_devs = NULL;
static struct platform_driver ddomain_isr_platform_driver = {
#ifdef CONFIG_PM
	.suspend		= ddomain_isr_suspend,
	.resume			= ddomain_isr_resume,
#endif
	.driver = {
		.name		= "ddomain_isr",
		.bus		= &platform_bus_type,
#ifdef CONFIG_OF
		.of_match_table = of_ddomain_isr_ids,
#endif
	},
};

unsigned int ddomain_print_count = 0;
extern unsigned char bPictureEnabled;
unsigned short APL[60*34]={0};

#ifdef CONFIG_PM
static int ddomain_isr_suspend(struct platform_device *dev, pm_message_t state)
{
	ppoverlay_dtg_ie_2_RBUS ppoverlay_dtg_ie_2_reg;

	rtd_pr_ddomain_isr_notice("[DDOMAIN_ISR]%s %d\n",__func__,__LINE__);

	//enable dtg_ie2 interrupt
	ppoverlay_dtg_ie_2_reg.regValue = rtd_inl(PPOVERLAY_DTG_ie_2_reg);
	ppoverlay_dtg_ie_2_reg.memcdtgv_den_end_event_ie_2 = 0;
	ppoverlay_dtg_ie_2_reg.ivs_rm_rising_ie_2 = 0;
	ppoverlay_dtg_ie_2_reg.mv_den_sta_event_ie_2 = 0;
	ppoverlay_dtg_ie_2_reg.mv_den_end_event_ie_2 = 0;
	rtd_outl(PPOVERLAY_DTG_ie_2_reg, ppoverlay_dtg_ie_2_reg.regValue);

	rtd_pr_ddomain_isr_notice("[DDOMAIN_ISR] suspend done\n");

	return 0;
}

static int ddomain_isr_resume(struct platform_device *dev)
{
	sys_reg_int_ctrl_scpu_RBUS sys_reg_int_ctrl_scpu_reg;
	ppoverlay_dtg_ie_2_RBUS ppoverlay_dtg_ie_2_reg;

	rtd_pr_ddomain_isr_notice("[DDOMAIN_ISR]%s %d\n",__func__,__LINE__);

	// enable route to SCPU,  Dctl_int_2_scpu_routing_en
	sys_reg_int_ctrl_scpu_reg.regValue = 0;
	sys_reg_int_ctrl_scpu_reg.dctl_int_2_scpu_routing_en = 1;
	sys_reg_int_ctrl_scpu_reg.write_data = 1;
	rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, sys_reg_int_ctrl_scpu_reg.regValue);

	//enable dtg_ie2 interrupt
	ppoverlay_dtg_ie_2_reg.regValue = rtd_inl(PPOVERLAY_DTG_ie_2_reg);
	ppoverlay_dtg_ie_2_reg.dtg_ie_2 = 1;
	ppoverlay_dtg_ie_2_reg.memcdtgv_den_end_event_ie_2 = 1;
	ppoverlay_dtg_ie_2_reg.mv_den_sta_event_ie_2 = 1;
	ppoverlay_dtg_ie_2_reg.mv_den_end_event_ie_2 = 1;
	rtd_outl(PPOVERLAY_DTG_ie_2_reg, ppoverlay_dtg_ie_2_reg.regValue);

	rtd_pr_ddomain_isr_notice("[DDOMAIN_ISR] resume done\n");

	return 0;
}

#endif

unsigned int ddomain_isr_get_mv_den_end_count(void)
{
	return mv_den_end_count;
}

unsigned int ddomain_isr_get_mv_den_sta_count(void)
{
	return mv_den_sta_count;
}

EXPORT_SYMBOL(ddomain_isr_get_mv_den_end_count);
EXPORT_SYMBOL(ddomain_isr_get_mv_den_sta_count);

extern unsigned char fwif_color_icm_set_pillar_RGB_Offset_byD_ISR(void);
extern unsigned char update_ICM_Pillar_Offset;
extern void scalerAI_face_PQ_set(void);
extern void scalerAI_scene_PQ_set(void);
extern void fwif_color_GSR_setrgbbrightness(unsigned short* APL, unsigned int HSize, unsigned int VSize);
extern unsigned short apl_init_value;
extern unsigned char vbe_disp_get_print_debug_line_count(void);
extern unsigned int Check_smooth_toggle_update_flag_isr(unsigned char display);
extern void framesync_set_vtotal_by_vodma_freerun_period_isr(void);
extern unsigned char scaler_get_VoTrackingI3DDMA_frequency_update_in_isr(void);
extern void scaler_set_VoTrackingI3DDMA_frequency_update_in_isr(unsigned char bEnable);
extern unsigned char SLD_SW_En;
extern unsigned char SLD_DMA_Ready;
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
extern unsigned char bAIInited;
extern UINT8 reset_face_apply;
// 0622 lsy
//extern UINT8 vpq_stereo_face;
extern RTK_AI_PQ_mode aipq_mode;
// end 0622 lsy
extern unsigned char PQ_set_done;
extern DRV_AI_Ctrl_table ai_ctrl;
extern int ai_face_rtk_mode;
extern int dcc_user_curve129[129];
extern unsigned char dcc_user_curve_write_flag;
extern void dcc_user_curve_write_table_tv006(UINT8 display, UINT8 Total_Curve_segment, int *Apply_Curve, int *to_SRAM_Curve, UINT8 *write_flag, UINT8 *apply_flag);
#endif


#if IS_ENABLED(CONFIG_RTK_AI_DRV)
extern unsigned char bAIInited;
extern UINT8 reset_face_apply;
extern UINT8 vpq_stereo_face;
extern unsigned char PQ_set_done;
extern DRV_AI_Ctrl_table ai_ctrl;
extern int ai_face_rtk_mode;
extern int dcc_user_curve129[129];
extern unsigned char dcc_user_curve_write_flag;
extern void dcc_user_curve_write_table_tv006(UINT8 display, UINT8 Total_Curve_segment, int *Apply_Curve, int *to_SRAM_Curve, UINT8 *write_flag, UINT8 *apply_flag);
#endif
extern void uzu_video_latency_pattern_ctrl(void);
extern void Scaler_Set_Reset_VODMA_DisableEnable_Cancel_Flag(unsigned int ucEnable);
extern unsigned int Scaler_Get_Reset_VODMA_DisableEnable_Cancel_Flag(void);

#define RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_24HZ 41*90 //8ms
#define RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_25HZ 40*90 //8ms
#define RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_30HZ 33*90 //8ms
#define RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_50HZ 20*90 //8ms
#define RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_60HZ 16*90 //8ms
#define RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_100HZ 10*90 //8ms
#define RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_120HZ 8*90 //8ms
//extern unsigned int speedup_rerun_resetvo_by_90K;
#define RESET_VODMA_4K_PANEL_RESERVED_LINE 15
#define RESET_VODMA_2K_PANEL_RESERVED_LINE 5

irqreturn_t ddomain_isr(int irq, void *dev_id)
{
    extern void memc_force_unfreeze_check(void);//avoid no body unfreeze
	ppoverlay_dtg_ie_2_RBUS dtg_ie_2_reg;
	ppoverlay_dtg_pending_status_2_RBUS dtg_pending_status_2_reg;
	vodma_vodma_pvs_free_RBUS vodma_vodma_pvs_free_reg;
	vodma_vodma_pvs_ctrl_RBUS vodma_vodma_pvs_ctrl_reg;
	vodma_vodma_reg_db_ctrl_RBUS vodma_vodma_reg_db_ctrl_reg;
	static vodma_vodma_v1_dcfg_RBUS vodma_vodma_v1_dcfg_reg;
	static vodma_decomp_ctrl0_RBUS vodma_decomp_ctrl0_reg;
	static	vodma_vodma_dma_option_RBUS vodma_vodma_dma_option_reg;
	ppoverlay_dtg_m_remove_input_vsync_RBUS ppoverlay_dtg_m_remove_input_vsync_reg;
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	
	ldspi_wclr_ctrl_RBUS ldspi_wclr_ctrl_reg;
	ldspi_rx_data_ctrl_RBUS ldspi_rx_data_ctrl;
	UINT8 handle_mv_den_sta_flag = 0;
	UINT8 handle_mv_den_end_flag = 0;
	UINT8 handle_memcdtgv_den_sta_flag = 0;
	UINT8 handle_memcdtgv_den_end_flag = 0;
	static UINT8 print_period = 0;
	unsigned int ret=IRQ_NONE;
	UINT8 handle_disp_frc2fsync_flag = 0;
	UINT8 handle_ivs_rm_rising_flag = 0;
	static UINT8 b_resetvodma = 0;
	static UINT8 printCounter = 0;
	static UINT32 u32_reset_vodma_enable_time_record=0;
	unsigned int vodma_reset_period_threshold = 0;
	static UINT8 more_chance_for_pending = 5;// 16.6ms*5 = 80ms, give more try chance for isr pending
	unsigned int vFreq=0;
	unsigned int reset_vodma_reserved_line_count=0;
#if 0	//framesync error debug
	mdomain_cap_ich12_ibuff_status_RBUS mdomain_cap_ich12_ibuff_status_reg;
#endif

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	// 0622 lsy
	//UINT8 AI_PQ_set_flag = ((vpq_stereo_face!=AI_PQ_AP_OFF) || reset_face_apply);
	UINT8 AI_PQ_set_flag = ((aipq_mode.ap_mode!=AI_MODE_OFF) || reset_face_apply);
	// end 0622 lsy
	int dcc_user_curve_sram[TableSeg_num_Max] = {0};
	unsigned char dcc_user_curve_apply_flag = 0;
	static int ai_face_disabled = 1;
#endif
	extern void drvif_scaler_enable_frc2fsync_interrupt(UINT8 bEnable);
	extern void drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(UINT8 bEnable);

	if((get_panel_res() == PANEL_RES_FHD) || (get_panel_res() == PANEL_RES_HD)){
		reset_vodma_reserved_line_count = RESET_VODMA_2K_PANEL_RESERVED_LINE;
	}else{
		reset_vodma_reserved_line_count = RESET_VODMA_4K_PANEL_RESERVED_LINE;
        if(Get_DISP_VERTICAL_TOTAL() - Get_DISP_DEN_END_VPOS() < 45)
        {
            reset_vodma_reserved_line_count = RESET_VODMA_2K_PANEL_RESERVED_LINE;
        }
	}

	ldspi_wclr_ctrl_reg.regValue =  IoReg_Read32(LDSPI_WCLR_CTRL_reg);
	ldspi_rx_data_ctrl.regValue = IoReg_Read32(LDSPI_RX_data_CTRL_reg);

	dtg_ie_2_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_ie_2_reg);
	dtg_pending_status_2_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_pending_status_2_reg);
	handle_mv_den_sta_flag = dtg_pending_status_2_reg.mv_den_sta_event_2;
	handle_mv_den_end_flag = dtg_pending_status_2_reg.mv_den_end_event_2;
	handle_memcdtgv_den_sta_flag = dtg_pending_status_2_reg.memcdtgv_den_sta_event_2;
	handle_memcdtgv_den_end_flag = dtg_pending_status_2_reg.memcdtgv_den_end_event_2;
	handle_disp_frc2fsync_flag = dtg_pending_status_2_reg.disp_frc2fsync_event_2;
	handle_ivs_rm_rising_flag = dtg_pending_status_2_reg.ivs_rm_rising_2;

	if (dtg_ie_2_reg.mv_den_end_event_ie_2 && handle_mv_den_end_flag) {
		if(vbe_disp_get_print_debug_line_count()==3)
		{
			if((print_period>100)){
				rtd_pr_ddomain_isr_notice("mv_end line(%d, %d, %d, %d, %d) fsync:%d 90k:%d\n",
					(DMA_VGIP_DMA_VGIP_LC_get_dma_line_cnt(IoReg_Read32(DMA_VGIP_DMA_VGIP_LC_reg))),
					(VODMA_VODMA_LINE_ST_get_line_cnt(IoReg_Read32(VODMA_VODMA_LINE_ST_reg))),
					(VGIP_VGIP_CHN1_LC_get_ch1_line_cnt(IoReg_Read32(VGIP_VGIP_CHN1_LC_reg))),
					(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg))),
					(PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg))),
					PPOVERLAY_Display_Timing_CTRL1_get_disp_frc_fsync(IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg)),
					drvif_Get_90k_Lo_clk());
				print_period=0;
			}else{
				print_period++;
			}
		}
		if(PPOVERLAY_Display_Timing_CTRL1_get_disp_frc_fsync(IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg))==1){
			if(Scaler_Get_Reset_VODMA_Flag()){
				rtd_pr_ddomain_isr_emerg("fsync ok! no reset vo. 90k:%d\n",drvif_Get_90k_Lo_clk());
				b_resetvodma = 0;
				Scaler_Set_Reset_VODMA_Flag(_DISABLE);
			}
		}
		if(Scaler_Get_Reset_VODMA_Flag() && !(b_resetvodma)){//VODMA reset flow enable
#ifdef CONFIG_RESET_VODMA_BY_NEW_MODE
				if(PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_output_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)) == 0)
#else
				if(PPOVERLAY_uzudtg_fractional_fsync_get_uzudtg_multiple_vsync(IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg))
				== PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_output_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)))
#endif
				{
#ifdef CONFIG_RESET_VODMA_BY_NEW_MODE
					if(((PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)))+reset_vodma_reserved_line_count
					< PPOVERLAY_DTG_VO_LC_get_dtg_vo_vln(IoReg_Read32(PPOVERLAY_DTG_VO_LC_reg))) || (more_chance_for_pending == 0))
#endif
				{
					rtd_pr_ddomain_isr_emerg("#FRANK Enable VODMA reset, vtotal:%d uzudelay:%d, dtg_in(%d), uzu(%d.%d) line(%d.%d.%d.%d), fsync:%d, 90k:%d\n",
					PPOVERLAY_uzudtg_DV_TOTAL_get_uzudtg_dv_total(IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg)),
					PPOVERLAY_uzudtg_control1_get_dtgm2uzuvs_line(IoReg_Read32(PPOVERLAY_uzudtg_control1_reg)),
					PPOVERLAY_DTG_FRAME_CNT3_get_dtg_input_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT3_reg)),
					PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_input_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)),
					PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_output_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)),
					(VODMA_VODMA_LINE_ST_get_line_cnt(IoReg_Read32(VODMA_VODMA_LINE_ST_reg))),
					(VGIP_VGIP_CHN1_LC_get_ch1_line_cnt(IoReg_Read32(VGIP_VGIP_CHN1_LC_reg))),
					(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg))),
					(PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg))),
					PPOVERLAY_Display_Timing_CTRL1_get_disp_frc_fsync(IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg)),
					drvif_Get_90k_Lo_clk());

					//disable VODMA DMA ++
					vodma_vodma_v1_dcfg_reg.regValue = IoReg_Read32(VODMA_VODMA_V1_DCFG_reg);
					vodma_decomp_ctrl0_reg.regValue = IoReg_Read32(VODMA_DECOMP_CTRL0_reg);
					vodma_vodma_dma_option_reg.regValue = IoReg_Read32(VODMA_VODMA_DMA_OPTION_reg);
					IoReg_ClearBits(VODMA_VODMA_V1_DCFG_reg, VODMA_VODMA_V1_DCFG_vodma_go_mask);
					IoReg_ClearBits(VODMA_DECOMP_CTRL0_reg, VODMA_DECOMP_CTRL0_decomp_wrap_en_mask);
					IoReg_ClearBits(VODMA_VODMA_DMA_OPTION_reg, VODMA_VODMA_DMA_OPTION_cmd_trans_en_shift);

					vodma_vodma_reg_db_ctrl_reg.regValue = IoReg_Read32(VODMA_VODMA_REG_DB_CTRL_reg);
					vodma_vodma_reg_db_ctrl_reg.vodma_db_en = 0;
					vodma_vodma_reg_db_ctrl_reg.vodmavsg_db_en = 0;
					IoReg_Write32(VODMA_VODMA_REG_DB_CTRL_reg, vodma_vodma_reg_db_ctrl_reg.regValue);
					vodma_vodma_reg_db_ctrl_reg.vodma_db_en = 1;
					vodma_vodma_reg_db_ctrl_reg.vodmavsg_db_en = 1;
					IoReg_Write32(VODMA_VODMA_REG_DB_CTRL_reg, vodma_vodma_reg_db_ctrl_reg.regValue);
					//disable VODMA DMA --

					vodma_vodma_pvs_ctrl_reg.regValue = IoReg_Read32(VODMA_VODMA_PVS_CTRL_reg);
					vodma_vodma_pvs_free_reg.regValue = IoReg_Read32(VODMA_VODMA_PVS_Free_reg);
					vodma_vodma_pvs_ctrl_reg.pvs0_free_vs_sel = 3;//reset by uzu DVS
					vodma_vodma_pvs_free_reg.pvs0_free_vs_reset_en = 1; //reset by Vsync
					IoReg_Write32(VODMA_VODMA_PVS_CTRL_reg, vodma_vodma_pvs_ctrl_reg.regValue);
					IoReg_Write32(VODMA_VODMA_PVS_Free_reg, vodma_vodma_pvs_free_reg.regValue);

					b_resetvodma = 1;
					printCounter = 3;
					more_chance_for_pending = 5;//16.6ms*5
					u32_reset_vodma_enable_time_record = drvif_Get_90k_Lo_clk();
				} else{
					rtd_pr_ddomain_isr_emerg("den end > voreset target (%d.%d). wait next frame = %d\n",
						(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg))),
						(PPOVERLAY_DTG_VO_LC_get_dtg_vo_vln(IoReg_Read32(PPOVERLAY_DTG_VO_LC_reg))),
						more_chance_for_pending);

					printCounter = 3;
					more_chance_for_pending--;
				}
			}
		}
		//if VO iv_src_sel is freerun, need  change DVtotal in next input frame
		if((VODMA_VODMA_PVS0_Gen_get_iv_src_sel(IoReg_Read32(VODMA_VODMA_PVS0_Gen_reg)) == 0) && (scaler_get_VoTrackingI3DDMA_frequency_update_in_isr()==TRUE)){
			//check smooth toggle finish
			if(Check_smooth_toggle_update_flag_isr(_MAIN_DISPLAY) == 0){
				framesync_set_vtotal_by_vodma_freerun_period_isr();
			}
		}
		fwif_color_icm_set_pillar_by_ISR();
	}
	if (dtg_ie_2_reg.mv_den_sta_event_ie_2 && handle_mv_den_sta_flag) {//data start isr
		if(vbe_disp_get_print_debug_line_count()==2)
		{
			if((print_period>100)){
				 rtd_pr_ddomain_isr_notice("mv_sta line(%d, %d, %d, %d, %d) fsync:%d 90k:%d\n",
					(DMA_VGIP_DMA_VGIP_LC_get_dma_line_cnt(IoReg_Read32(DMA_VGIP_DMA_VGIP_LC_reg))),
					(VODMA_VODMA_LINE_ST_get_line_cnt(IoReg_Read32(VODMA_VODMA_LINE_ST_reg))),
					(VGIP_VGIP_CHN1_LC_get_ch1_line_cnt(IoReg_Read32(VGIP_VGIP_CHN1_LC_reg))),
					(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg))),
					(PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg))),
					PPOVERLAY_Display_Timing_CTRL1_get_disp_frc_fsync(IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg)),
					drvif_Get_90k_Lo_clk());
				print_period=0;
			}else{
				print_period++;
			}
		}
#ifdef CONFIG_UZU_PATTERN_MEASUREMENT
		uzu_video_latency_pattern_ctrl();
#endif

		if((Scaler_Get_Reset_VODMA_Flag()  && b_resetvodma) || (!Scaler_Get_Reset_VODMA_Flag() && Scaler_Get_Reset_VODMA_DisableEnable_Cancel_Flag() && b_resetvodma)){
			rtd_pr_ddomain_isr_emerg("#FRANK Disable VODMA reset, dtg_in:%d uzu(%d.%d) line(%d.%d.%d.%d), fsync:%d 90k:%d\n",
			PPOVERLAY_DTG_FRAME_CNT3_get_dtg_input_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT3_reg)),
			PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_input_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)),
			PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_output_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)),
			(VODMA_VODMA_LINE_ST_get_line_cnt(IoReg_Read32(VODMA_VODMA_LINE_ST_reg))),
			(VGIP_VGIP_CHN1_LC_get_ch1_line_cnt(IoReg_Read32(VGIP_VGIP_CHN1_LC_reg))),
			(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg))),
			(PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg))),
			PPOVERLAY_Display_Timing_CTRL1_get_disp_frc_fsync(IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg)),
			drvif_Get_90k_Lo_clk());

			//enable VODMA DMA ++
			IoReg_Write32(VODMA_VODMA_V1_DCFG_reg, vodma_vodma_v1_dcfg_reg.regValue);
			IoReg_Write32(VODMA_DECOMP_CTRL0_reg, vodma_decomp_ctrl0_reg.regValue);
			IoReg_Write32(VODMA_VODMA_DMA_OPTION_reg, vodma_vodma_dma_option_reg.regValue);

			vodma_vodma_reg_db_ctrl_reg.regValue = IoReg_Read32(VODMA_VODMA_REG_DB_CTRL_reg);
			vodma_vodma_reg_db_ctrl_reg.vodma_db_rdy = 1;
			IoReg_Write32(VODMA_VODMA_REG_DB_CTRL_reg, vodma_vodma_reg_db_ctrl_reg.regValue);
			//enable VODMA DMA --

			vodma_vodma_pvs_free_reg.regValue = IoReg_Read32(VODMA_VODMA_PVS_Free_reg);
			vodma_vodma_pvs_ctrl_reg.regValue = IoReg_Read32(VODMA_VODMA_PVS_CTRL_reg);
			vodma_vodma_pvs_free_reg.pvs0_free_vs_reset_en = 0; //disable reset by Vsync
			vodma_vodma_pvs_ctrl_reg.pvs0_free_vs_sel = 0;//disable reset by Main DVS
			IoReg_Write32(VODMA_VODMA_PVS_Free_reg, vodma_vodma_pvs_free_reg.regValue);
			IoReg_Write32(VODMA_VODMA_PVS_CTRL_reg, vodma_vodma_pvs_ctrl_reg.regValue);

			ppoverlay_dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
			ppoverlay_dtg_m_remove_input_vsync_reg.remove_half_ivs_cnt_reset = 1;
			IoReg_Write32(PPOVERLAY_DTG_M_Remove_input_vsync_reg, ppoverlay_dtg_m_remove_input_vsync_reg.regValue);
			ppoverlay_dtg_m_remove_input_vsync_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg);
			ppoverlay_dtg_m_remove_input_vsync_reg.remove_half_ivs_cnt_reset = 0;
			IoReg_Write32(PPOVERLAY_DTG_M_Remove_input_vsync_reg, ppoverlay_dtg_m_remove_input_vsync_reg.regValue);
			if((!Scaler_Get_Reset_VODMA_Flag() && Scaler_Get_Reset_VODMA_DisableEnable_Cancel_Flag() && b_resetvodma))
            {
                rtd_pr_vsc_emerg("Cancel b_resetvodma by reset timeout");
                Scaler_Set_Reset_VODMA_DisableEnable_Cancel_Flag(_DISABLE);
            }
            b_resetvodma = 0;
			
			vFreq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);

			if((vFreq>235)&&(vFreq<=245)){ // 24hz
				vodma_reset_period_threshold = RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_24HZ;
			}else if((vFreq>245)&&(vFreq<=255)){ // 25hz
				vodma_reset_period_threshold = RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_25HZ;
			}else if((vFreq>295)&&(vFreq<=305)){ //30Hz
				vodma_reset_period_threshold = RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_30HZ;
			}else if((vFreq>495)&&(vFreq<=505)){ //50Hz
				vodma_reset_period_threshold = RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_50HZ;
			}else if((vFreq>595)&&(vFreq<=605)){ //60Hz
				vodma_reset_period_threshold = RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_60HZ;
			}else if((vFreq>995)&&(vFreq<=1005)){ // 100hz
				vodma_reset_period_threshold = RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_100HZ;
			}else if((vFreq>1195)&&(vFreq<=1205)){ //120Hz
				vodma_reset_period_threshold = RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_120HZ;
			}else{
				vodma_reset_period_threshold = RESET_VODMA_INTERRUPT_PERIOD_THRESHOLD_24HZ;
				rtd_pr_ddomain_isr_emerg("[Warn]unknow freq=%d \n",vFreq);
			}

			if((drvif_Get_90k_Lo_clk() - u32_reset_vodma_enable_time_record) > vodma_reset_period_threshold){
				Scaler_Set_Reset_VODMA_Flag(_ENABLE);
				rtd_pr_ddomain_isr_emerg("Reset Vodma over threshold, reset again.\n");
			}else{
				Scaler_Set_Reset_VODMA_Flag(_DISABLE);
				//no need do VO reset again, enable timing fsync
				display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
				display_timing_ctrl1_reg.disp_fsync_en = _ENABLE;
				display_timing_ctrl1_reg.disp_fix_last_line_new= 0;
				display_timing_ctrl1_reg.fsync_fll_mode= _ENABLE;
				IoReg_Write32(PPOVERLAY_Display_Timing_CTRL1_reg, display_timing_ctrl1_reg.regValue);
			}
		}
	}
	if(dtg_ie_2_reg.ivs_rm_rising_ie_2 && handle_ivs_rm_rising_flag)
	{
		if(printCounter)
		{
			rtd_pr_ddomain_isr_notice("bbb #ivs uzu(%d.%d) line(%d, %d,%d,%d,%d) fsync:%d 90k:%d\n",
			PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_input_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)),
			PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_output_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)),
			(DMA_VGIP_DMA_VGIP_LC_get_dma_line_cnt(IoReg_Read32(DMA_VGIP_DMA_VGIP_LC_reg))),
			(VODMA_VODMA_LINE_ST_get_line_cnt(IoReg_Read32(VODMA_VODMA_LINE_ST_reg))),
			(VGIP_VGIP_CHN1_LC_get_ch1_line_cnt(IoReg_Read32(VGIP_VGIP_CHN1_LC_reg))),
			(PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg))),
			(PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg))),
			PPOVERLAY_Display_Timing_CTRL1_get_disp_frc_fsync(IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg)),
			drvif_Get_90k_Lo_clk());
			printCounter--;
		}

		dtg_pending_status_2_reg.regValue=0;
		dtg_pending_status_2_reg.ivs_rm_rising_2 = 1;//write clear status
		IoReg_Write32(PPOVERLAY_DTG_pending_status_2_reg, dtg_pending_status_2_reg.regValue);
		ret = IRQ_HANDLED;
	}
	if((dtg_ie_2_reg.disp_frc2fsync_event_ie_2==1)&&(handle_disp_frc2fsync_flag == 1)){
			rtd_pr_ddomain_isr_notice("#fsync ok (vl:%d, ul:%d)\n", VGIP_VGIP_CHN1_LC_get_ch1_line_cnt(IoReg_Read32(VGIP_VGIP_CHN1_LC_reg)),
			PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)));
#if 0	//I2D tracking debug
			rtd_pr_ddomain_isr_notice("i2d_phase = %d, LC = [%d, %d, %d]\n", PPOVERLAY_I2D_MEAS_phase_get_i2d_phase_err(IoReg_Read32(PPOVERLAY_I2D_MEAS_phase_reg)), VODMA_VODMA_LINE_ST_get_line_cnt(IoReg_Read32(VODMA_VODMA_LINE_ST_reg)), SCALEDOWN_ICH1_line_cnt_get_sdnr_ch1_line_cnt(IoReg_Read32(SCALEDOWN_ICH1_line_cnt_reg)), PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)));
#endif
			drvif_scaler_enable_frc2fsync_interrupt(0);
			drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(0);
            		ret = IRQ_HANDLED;
	}

	if (dtg_ie_2_reg.memcdtgv_den_sta_event_ie_2 && handle_memcdtgv_den_sta_flag) {//memc data start isr

		//if VO already sync i3, need  change DVtotal immediately
		if(check_vsc_connect_source())
		{
			if((VODMA_VODMA_PVS0_Gen_get_iv_src_sel(IoReg_Read32(VODMA_VODMA_PVS0_Gen_reg)) == 3) && (scaler_get_VoTrackingI3DDMA_frequency_update_in_isr()==TRUE)){
				//rtd_pr_ddomain_isr_notice("[ERIC_DBG] vo_free_period(%x), ivs_cnt(%x), dvs_cnt(%x,%x,%x), uzu_fc(%d,%d) 90k=%x\n", VODMA_VODMA_PVS_Free_get_pvs0_free_period(IoReg_Read32(0xb8005604)), IoReg_Read32(0xb8025cdc), IoReg_Read32(0xb80282e4), IoReg_Read32(0xb80282dc), IoReg_Read32(0xb80282ec),
				//	PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_input_frame_cnt(IoReg_Read32(0xb80284A0)), PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_output_frame_cnt(IoReg_Read32(0xb80284A0)), drvif_Get_90k_Lo_clk());
				//check smooth toggle finish
				if(Check_smooth_toggle_update_flag_isr(_MAIN_DISPLAY) == 0){
					framesync_set_vtotal_by_vodma_freerun_period_isr();
					vbe_disp_pwm_frequency_update(FALSE);

					if(Scaler_Get_VODMA_Tracking_I3DDMA()==0){
						//tracking done
						rtd_pr_ddomain_isr_notice("vo tracking done\n");
						scaler_set_VoTrackingI3DDMA_frequency_update_in_isr(FALSE);
					}
				}
			}
		}
		dtg_pending_status_2_reg.regValue=0;
		dtg_pending_status_2_reg.memcdtgv_den_sta_event_2 = 1;//write clear status
		IoReg_Write32(PPOVERLAY_DTG_pending_status_2_reg, dtg_pending_status_2_reg.regValue);
		ret = IRQ_HANDLED;
	}

	if (dtg_ie_2_reg.memcdtgv_den_end_event_ie_2 && handle_memcdtgv_den_end_flag) {//memc data end isr
		if (PPOVERLAY_MEMC_MUX_CTRL_get_memc_outmux_sel(IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg))) {
#ifdef CONFIG_CUSTOMER_TV002
			extern int scalerVIP_TV002_LD_Dynamic_Processing(void);
			scalerVIP_TV002_LD_Dynamic_Processing();
#endif
			scalerVIP_RADCR_main();
		}

		dtg_pending_status_2_reg.regValue=0;
		dtg_pending_status_2_reg.memcdtgv_den_end_event_2 = 1;//write clear status
		IoReg_Write32(PPOVERLAY_DTG_pending_status_2_reg, dtg_pending_status_2_reg.regValue);
		ret = IRQ_HANDLED;
	}

	if (dtg_ie_2_reg.mv_den_sta_event_ie_2 && handle_mv_den_sta_flag) {//data start isr
		fwif_color_DI_RTNR_Timing_Measure_Check_ISR();
		scalerDCC_Sync_RPC_Apply_Curve();

		mv_den_sta_count++;
		dtg_pending_status_2_reg.regValue=0;
		dtg_pending_status_2_reg.mv_den_sta_event_2 = 1;//write clear status
		IoReg_Write32(PPOVERLAY_DTG_pending_status_2_reg, dtg_pending_status_2_reg.regValue);
		BSP_ERROR_CHECK_REPORT(NULL);//check bsp error status
		ret = IRQ_HANDLED;
	}

	if (dtg_ie_2_reg.mv_den_end_event_ie_2 && handle_mv_den_end_flag) {		
		if (!PPOVERLAY_MEMC_MUX_CTRL_get_memc_outmux_sel(IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg))) {
#ifdef CONFIG_CUSTOMER_TV002
			extern int scalerVIP_TV002_LD_Dynamic_Processing(void);
			scalerVIP_TV002_LD_Dynamic_Processing();
#endif
			scalerVIP_RADCR_main();
		}

#ifdef CONFIG_CUSTOMER_TV002
		scalerVIP_VGIPISR_enable_status_check(DDOMAIN_DE_ISR_THREAD);
#endif
#if 0	//framesync error debug
		if (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == TRUE) {
			mdomain_cap_ich12_ibuff_status_reg.regValue = IoReg_Read32(MDOMAIN_CAP_ICH12_IBUFF_STATUS_reg);
			if(mdomain_cap_ich12_ibuff_status_reg.ibuff_ovf | mdomain_cap_ich12_ibuff_status_reg.fsync_ovf | mdomain_cap_ich12_ibuff_status_reg.fsync_udf)
			{
				rtd_pr_ddomain_isr_notice("[DDOMAIN_ISR]LC = [%d, %d, %d], clkgen=%x\n", VODMA_VODMA_LINE_ST_get_line_cnt(IoReg_Read32(VODMA_VODMA_LINE_ST_reg)), SCALEDOWN_ICH1_line_cnt_get_sdnr_ch1_line_cnt(IoReg_Read32(SCALEDOWN_ICH1_line_cnt_reg)), PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)), IoReg_Read32(VODMA_VODMA_CLKGEN_reg));
				rtd_pr_ddomain_isr_notice("[DDOMAIN_ISR]Frame Sync check error=%d %d %d\n", mdomain_cap_ich12_ibuff_status_reg.ibuff_ovf, mdomain_cap_ich12_ibuff_status_reg.fsync_ovf, mdomain_cap_ich12_ibuff_status_reg.fsync_udf);
			}
		}
#endif

		//if (strcmp(webos_strToolOption.eBackLight, "oled") == 0)
		if (SLD_SW_En == 1 || SLD_DMA_Ready == 1)
		{
			fwif_color_GSR_setrgbbrightness(&APL[0], 60, 34);
		}

#ifdef CONFIG_SUPPORT_SLD_BY_MEMC_DEMURA
		if (/*strcmp(webos_strToolOption.eBackLight, "oled") == 0*/SLD_SW_En == 1)
		{
			if(bPictureEnabled==1){
				memc_logo_to_demura_read();
			}
		}
#endif
		//if (strcmp(webos_strToolOption.eBackLight, "oled") == 0)
		if (SLD_DMA_Ready == 1)
		{
			fwif_color_sld();
		}


#ifdef VIP_SUPPORT_SLC
		if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE && Scaler_Get_Dynamic_SLC_Enable()) {
			fwif_color_dynamic_SLC_Curve_Ctrl();
		}
#endif

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
		dcc_user_curve_write_table_tv006(SLR_MAIN_DISPLAY, TableSeg_num_Max, dcc_user_curve129, dcc_user_curve_sram, &dcc_user_curve_write_flag, &dcc_user_curve_apply_flag);

		if(bAIInited && AI_PQ_set_flag)
		{
			if(ai_ctrl.ai_global3.ip_isr_ctrl)// avoid cmd from v4l2, hal, osd..
			{
				// init one time only, controled by ai_face_rtk_mode (bit 26)
				if(ai_face_rtk_mode==1 && ai_face_disabled==1)
				{
					//rtd_pr_vpq_ai_info("ai_face_rtk_mode=1, ai_face_disabled=1\n");
					drvif_color_AI_obj_dcc_init(1);
					drvif_color_AI_obj_icm_init(1);
					drvif_color_AI_obj_srp_init(1);
					ai_face_disabled = 0;
				}
				else if(ai_face_rtk_mode==0 && ai_face_disabled==0)
				{
					//rtd_pr_vpq_ai_info("ai_face_rtk_mode=%d, ai_face_disabled=0\n", ai_face_rtk_mode);
					drvif_color_AI_obj_dcc_init(0);
					drvif_color_AI_obj_icm_init(0);
					drvif_color_AI_obj_srp_init(0);
					ai_face_disabled = 1;
				}
			}
			/*if(vpq_stereo_face!=AI_PQ_AP_OFF && ai_ctrl.ai_global3.ip_isr_ctrl)
			{
				drvif_color_AI_obj_srp_init(1);
				drvif_color_AI_obj_dcc_init();
			}*/

			scalerAI_face_PQ_set();
			PQ_set_done = 1;
		}

		if( bAIInited )
			scalerPQMask_postprocessing_ddomain();
#endif

		if (update_ICM_Pillar_Offset == 1) {
			fwif_color_icm_set_pillar_RGB_Offset_byD_ISR();
			update_ICM_Pillar_Offset = 0;
		}

        memc_force_unfreeze_check();

		mv_den_end_count++;
		dtg_pending_status_2_reg.regValue=0;
		dtg_pending_status_2_reg.mv_den_end_event_2 = 1;//write clear status
		IoReg_Write32(PPOVERLAY_DTG_pending_status_2_reg, dtg_pending_status_2_reg.regValue);
		ret = IRQ_HANDLED;
	}

	//Localdimming
	/*
	{
		static unsigned int timer1,timer2;
		timer1 = IoReg_Read32(0xB801B6B8);//(TIMER_SCPU_CLK90K_LO_reg);
		printk("timer = %d \n",(timer1-timer2)/90);
		timer2 = timer1;
	}
	*/
	if(ldspi_rx_data_ctrl.tx_done_int_en && ldspi_wclr_ctrl_reg.tx_done && ldspi_rx_data_ctrl.ld_spi_int2_en){		
		if(fwif_color_Get_LD_Init_Done()) {
		 	fwif_color_LocalDimming_Process();
		}
		ldspi_wclr_ctrl_reg.regValue = 0;
		ldspi_wclr_ctrl_reg.tx_done = 1;
		IoReg_Write32(LDSPI_WCLR_CTRL_reg, ldspi_wclr_ctrl_reg.regValue);
		ret = IRQ_HANDLED;
	}
	/*
	if (dtg_ie_2_reg.mv_den_end_event_ie_2 && handle_mv_den_end_flag) {
		if(invoutputgamma_isr_flag){
			invoutputgamma_isr_flag = 0;
			fwif_color_set_InvOutputGamma_System(2);
		}
	}
	*/
	return ret;
}

static int irq_ddomain_isr = -1;
static int __init ddomain_isr_init_irq(struct platform_device *pdev)
{
	sys_reg_int_ctrl_scpu_RBUS sys_reg_int_ctrl_scpu_reg;
	ppoverlay_dtg_ie_2_RBUS ppoverlay_dtg_ie_2_reg;
	int irq;

	IoReg_Write32(PPOVERLAY_DTG_pending_status_2_reg, IoReg_Read32(PPOVERLAY_DTG_pending_status_2_reg));

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	/* Request IRQ */
	if(request_irq(irq,
                   ddomain_isr,
                   IRQF_SHARED,
                   "DDOMAIN ISR",
                   (void *)pdev))
	{
		rtd_pr_ddomain_isr_err("ddomain_isr: cannot register IRQ %d\n", irq);
		return -ENXIO;
	}

	irq_ddomain_isr = irq;

	// enable route to SCPU,  Dctl_int_2_scpu_routing_en
	sys_reg_int_ctrl_scpu_reg.regValue = 0;
	sys_reg_int_ctrl_scpu_reg.dctl_int_2_scpu_routing_en = 1;
	sys_reg_int_ctrl_scpu_reg.write_data = 1;
	rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, sys_reg_int_ctrl_scpu_reg.regValue);

	//enable dtg_ie2 interrupt
	ppoverlay_dtg_ie_2_reg.regValue = rtd_inl(PPOVERLAY_DTG_ie_2_reg);
	ppoverlay_dtg_ie_2_reg.dtg_ie_2 = 1;
	ppoverlay_dtg_ie_2_reg.ivs_rm_rising_ie_2 = 1;
	ppoverlay_dtg_ie_2_reg.memcdtgv_den_sta_event_ie_2 = 1;
	ppoverlay_dtg_ie_2_reg.memcdtgv_den_end_event_ie_2 = 1;
	ppoverlay_dtg_ie_2_reg.mv_den_sta_event_ie_2 = 1;
	ppoverlay_dtg_ie_2_reg.mv_den_end_event_ie_2 = 1;
	rtd_outl(PPOVERLAY_DTG_ie_2_reg, ppoverlay_dtg_ie_2_reg.regValue);

	rtd_pr_ddomain_isr_info("ddomain_isr: register IRQ virq:%d hwirq:%lu\n",
			irq, irqd_to_hwirq(irq_get_irq_data(irq)));

	return 0;
}
#if 0
static char *vgip_isr_devnode(struct device *dev, mode_t *mode)
{
	return NULL;
}
#endif

int __init ddomain_isr_probe(struct platform_device *pdev)
{
	int result;

	result = ddomain_isr_init_irq(pdev);
	if (result < 0) {
		rtd_pr_ddomain_isr_err("ddomain_isr: can not register irq...\n");
		return result;
	}

	ddomain_isr_platform_devs = pdev;

	return 0;
}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
int __init ddomain_isr_init_module(void)
#else
static int __init ddomain_isr_init_module(void)
#endif
{

	int result;

	if (platform_driver_probe(&ddomain_isr_platform_driver, ddomain_isr_probe) != 0) {
                rtd_pr_ddomain_isr_info("ddomain_isr: can not register platform driver...\n");
                result = -EINVAL;
                return result;
        }

	return 0;

}

void __exit ddomain_isr_exit_module(void)
{
	if (irq_ddomain_isr > -1) {
		free_irq(irq_ddomain_isr, ddomain_isr_platform_devs);
		irq_ddomain_isr = -1;
	}
	platform_driver_unregister(&ddomain_isr_platform_driver);
	ddomain_isr_platform_devs = NULL;
}


#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(ddomain_isr_init_module);
module_exit(ddomain_isr_exit_module);
#endif

