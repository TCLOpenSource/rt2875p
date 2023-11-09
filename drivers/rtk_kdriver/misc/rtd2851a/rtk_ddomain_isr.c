//Copyright (C) 2007-2013 Realtek Semiconductor Corporation.
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "rtk_ddomain_isr.h"
#include <rbus/ppoverlay_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/vgip_reg.h>

#include "vgip_isr/scalerAI.h"
#include <util/rtk_util_isr_wait.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <rtk_ai.h>
#include <tvscalercontrol/vip/ai_pq.h>
// for register dump
#include <tvscalercontrol/io/ioregdrv.h>
#include <base_types.h>

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

/* Function Prototype */
extern unsigned char power_on_enable_dtg_ie2_interrupt(void);
static unsigned int mv_den_end_count;
static unsigned int mv_den_sta_count;
static unsigned int memcdtg_den_sta_count;
static unsigned int memcdtg_den_end_count;
static int ddomain_isr_suspend(struct platform_device *dev, pm_message_t state);
static int ddomain_isr_resume(struct platform_device *dev);

static struct platform_device *ddomain_isr_platform_devs = NULL;
static struct platform_driver ddomain_isr_platform_driver = {
#ifdef CONFIG_PM
	.suspend		= ddomain_isr_suspend,
	.resume			= ddomain_isr_resume,
#endif
	.driver = {
		.name		= "ddomain_isr",
		.bus		= &platform_bus_type,
	},
};


#ifdef CONFIG_PM
static int ddomain_isr_suspend(struct platform_device *dev, pm_message_t state)
{
	ppoverlay_dtg_ie_2_RBUS ppoverlay_dtg_ie_2_reg;

	rtd_pr_ddomain_isr_notice("[DDOMAIN_ISR]%s %d\n",__func__,__LINE__);

	//enable dtg_ie2 interrupt
	ppoverlay_dtg_ie_2_reg.regValue = rtd_inl(PPOVERLAY_DTG_ie_2_reg);
	ppoverlay_dtg_ie_2_reg.mv_den_sta_event_ie_2 = 0;
	ppoverlay_dtg_ie_2_reg.mv_den_end_event_ie_2 = 0;
    ppoverlay_dtg_ie_2_reg.memcdtgv_den_sta_event_ie_2 = 0;
    //ppoverlay_dtg_ie_2_reg.memcdtgv_den_end_event_ie_2 = 0;
	rtd_outl(PPOVERLAY_DTG_ie_2_reg, ppoverlay_dtg_ie_2_reg.regValue);

	rtd_pr_ddomain_isr_notice("[DDOMAIN_ISR] suspend done\n");

	return 0;
}

static int ddomain_isr_resume(struct platform_device *dev)
{
	sys_reg_int_ctrl_scpu_RBUS sys_reg_int_ctrl_scpu_reg;

	rtd_pr_ddomain_isr_notice("[DDOMAIN_ISR]%s %d\n",__func__,__LINE__);

	// enable route to SCPU,  Dctl_int_2_scpu_routing_en
	sys_reg_int_ctrl_scpu_reg.regValue = 0;
	sys_reg_int_ctrl_scpu_reg.dctl_int_2_scpu_routing_en = 1;
	sys_reg_int_ctrl_scpu_reg.write_data = 1;
	rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, sys_reg_int_ctrl_scpu_reg.regValue);

	//enable dtg_ie2 interrupt
	power_on_enable_dtg_ie2_interrupt();

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

unsigned int ddomain_isr_get_memcdtg_den_sta_count(void)
{
    return memcdtg_den_sta_count;
}

unsigned int ddomain_isr_get_memcdtg_den_end_count(void)
{
	return memcdtg_den_end_count;
}

EXPORT_SYMBOL(ddomain_isr_get_mv_den_end_count);
EXPORT_SYMBOL(ddomain_isr_get_mv_den_sta_count);
EXPORT_SYMBOL(ddomain_isr_get_memcdtg_den_sta_count);
EXPORT_SYMBOL(ddomain_isr_get_memcdtg_den_end_count);

#if defined(CONFIG_RTK_AI_DRV)
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

RUTIL_ISR_WAIT_ADV_EXPORT(ISR_WAIT_MV_DEN_STA);
RUTIL_ISR_WAIT_ADV_EXPORT(ISR_WAIT_MV_DEN_END);
RUTIL_ISR_WAIT_ADV_EXPORT(ISR_WAIT_MEMCDTG_DEN_STA);

irqreturn_t ddomain_isr(int irq, void *dev_id)
{
	ppoverlay_dtg_ie_2_RBUS dtg_ie_2_reg;
	ppoverlay_dtg_pending_status_2_RBUS dtg_pending_status_2_reg;
	UINT8 handle_mv_den_sta_flag = 0;
	UINT8 handle_mv_den_end_flag = 0;
    UINT8 handle_memc_den_sta_flag = 0;
    //UINT8 handle_memc_den_end_flag = 0;
	UINT8 handle_disp_frc2fsync_flag = 0;
	unsigned int ret=IRQ_NONE;
	extern void drvif_scaler_enable_frc2fsync_interrupt(UINT8 bEnable);
	extern void drvif_scaler_enable_frc2fsync_HW_speedup_vtotal(UINT8 bEnable);
#if defined(CONFIG_RTK_AI_DRV)
	UINT8 AI_PQ_set_flag = ((vpq_stereo_face!=AI_PQ_AP_OFF) || reset_face_apply);
	int dcc_user_curve_sram[TableSeg_num_Max] = {0};
	unsigned char dcc_user_curve_apply_flag = 0;
#endif

	dtg_ie_2_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_ie_2_reg);
	dtg_pending_status_2_reg.regValue = IoReg_Read32(PPOVERLAY_DTG_pending_status_2_reg);
    handle_mv_den_sta_flag   = dtg_pending_status_2_reg.mv_den_sta_event_2;
    handle_mv_den_end_flag   = dtg_pending_status_2_reg.mv_den_end_event_2;
    handle_memc_den_sta_flag = dtg_pending_status_2_reg.memcdtgv_den_sta_event_2;
    //handle_memc_den_end_flag = dtg_pending_status_2_reg.memcdtgv_den_end_event_2;
	handle_disp_frc2fsync_flag = dtg_pending_status_2_reg.disp_frc2fsync_event_2;

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

	if (dtg_ie_2_reg.mv_den_sta_event_ie_2 && handle_mv_den_sta_flag) {//data start isr
        RUTIL_ISR_WAIT_ADV_ISR(ISR_WAIT_MV_DEN_STA);    // it's fast

		fwif_color_DI_RTNR_Timing_Measure_Check_ISR();

		mv_den_sta_count++;
		dtg_pending_status_2_reg.regValue=0;
		dtg_pending_status_2_reg.mv_den_sta_event_2 = 1;//write clear status
		IoReg_Write32(PPOVERLAY_DTG_pending_status_2_reg, dtg_pending_status_2_reg.regValue);
        BSP_ERROR_CHECK_REPORT(NULL);//check bsp error status
		ret = IRQ_HANDLED;
	}

	if (dtg_ie_2_reg.mv_den_end_event_ie_2 && handle_mv_den_end_flag) {
        RUTIL_ISR_WAIT_ADV_ISR(ISR_WAIT_MV_DEN_END);    // it's fast and used to enable VO gating only
#if defined(CONFIG_RTK_AI_DRV)
	dcc_user_curve_write_table_tv006(SLR_MAIN_DISPLAY, TableSeg_num_Max, dcc_user_curve129, dcc_user_curve_sram, &dcc_user_curve_write_flag, &dcc_user_curve_apply_flag);

	if(bAIInited && AI_PQ_set_flag)
	{
		if(ai_ctrl.ai_global3.ip_isr_ctrl)// avoid cmd from v4l2, hal, osd..
		{
			if(ai_face_rtk_mode==1)
			{
				drvif_color_AI_obj_dcc_init(1);
				drvif_color_AI_obj_icm_init(1);
				drvif_color_AI_obj_srp_init(1);
			}
			else
			{
				drvif_color_AI_obj_dcc_init(0);
				drvif_color_AI_obj_icm_init(0);
				drvif_color_AI_obj_srp_init(0);
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
#endif

		fwif_color_icm_set_pillar_by_ISR();

		mv_den_end_count++;
		dtg_pending_status_2_reg.regValue=0;
		dtg_pending_status_2_reg.mv_den_end_event_2 = 1;//write clear status
		IoReg_Write32(PPOVERLAY_DTG_pending_status_2_reg, dtg_pending_status_2_reg.regValue);
		ret = IRQ_HANDLED;
	}

    if (dtg_ie_2_reg.memcdtgv_den_sta_event_ie_2 && handle_memc_den_sta_flag) {
        RUTIL_ISR_WAIT_ADV_ISR(ISR_WAIT_MEMCDTG_DEN_STA);

        memcdtg_den_sta_count++;
        dtg_pending_status_2_reg.regValue=0;
        dtg_pending_status_2_reg.memcdtgv_den_sta_event_2 = 1;//write clear status
        IoReg_Write32(PPOVERLAY_DTG_pending_status_2_reg, dtg_pending_status_2_reg.regValue);
        ret = IRQ_HANDLED;
    }

#if 0
    if (dtg_ie_2_reg.memcdtgv_den_end_event_ie_2 && handle_memc_den_end_flag) {

        memcdtg_den_end_count++;
        dtg_pending_status_2_reg.regValue=0;
        dtg_pending_status_2_reg.memcdtgv_den_end_event_2 = 1;//write clear status
        IoReg_Write32(PPOVERLAY_DTG_pending_status_2_reg, dtg_pending_status_2_reg.regValue);
        ret = IRQ_HANDLED;
    }
#endif

	return ret;
}

extern u32 gic_irq_find_mapping(u32 hwirq);//cnange interrupt register way
static int irq_ddomain_isr = -1;
static int __init ddomain_isr_init_irq(struct platform_device *pdev)
{
	sys_reg_int_ctrl_scpu_RBUS sys_reg_int_ctrl_scpu_reg;
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
	power_on_enable_dtg_ie2_interrupt();

	// rtd_pr_ddomain_isr_info("ddomain_isr: register IRQ virq:%d hwirq:%lu\n",
		// irq, irqd_to_hwirq(irq_get_irq_data(irq)));

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

static __init int ddomain_isr_init_module(void)
{

	int result;

	if (platform_driver_probe(&ddomain_isr_platform_driver, ddomain_isr_probe) != 0) {
		rtd_pr_ddomain_isr_info("ddomain_isr: can not register platform driver...\n");
		result = -EINVAL;
		return result;
	}

	return 0;

}

void __exit  ddomain_isr_exit_module(void)
{
	if (irq_ddomain_isr > -1) {
		free_irq(irq_ddomain_isr, ddomain_isr_platform_devs);
		irq_ddomain_isr = -1;
	}
	platform_driver_unregister(&ddomain_isr_platform_driver);
	ddomain_isr_platform_devs = NULL;
}

module_init(ddomain_isr_init_module);
module_exit(ddomain_isr_exit_module);

