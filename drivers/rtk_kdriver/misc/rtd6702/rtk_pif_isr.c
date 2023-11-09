//Copyright (C) 2007-2013 Realtek Semiconductor Corporation.
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/of_device.h>

#ifdef CONFIG_ARM64
#endif

#include "rtk_ddomain_isr.h"
#include <rbus/ppoverlay_reg.h>
#include <rbus/timer_reg.h>
#include <rbus/sfg_reg.h>
#include <rbus/sys_reg_reg.h>
#include <scaler/scalerCommon.h>
#include <scaler_vscdev.h>
#include <scaler_vbedev.h>
#include <rbus/vgip_reg.h>
#include <rbus/tcon_reg.h>
#include <rtd_log/rtd_module_log.h>

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


/* Function Prototype */
static int pif_isr_init_irq(struct platform_device *pdev);
static void pif_isr_deinit_irq(struct platform_device *pdev);
static int pif_isr_probe(struct platform_device *pdev);
static int pif_isr_remove(struct platform_device *pdev);
static int pif_isr_suspend(struct platform_device *dev, pm_message_t state);
static int pif_isr_resume(struct platform_device *dev);

extern unsigned char vbe_disp_oled_orbit_enable;
extern unsigned char vbe_disp_oled_orbit_mode;
//static ORBIT_PIXEL_SHIFT_STRUCT pre_orbit_shift = {0};

#ifdef CONFIG_OF
static const struct of_device_id of_pif_ids[] = {
     { .compatible = "realtek,pif_isr" },
     {}
};
MODULE_DEVICE_TABLE(of, of_pif_ids);
#endif


static struct platform_driver pif_isr_platform_driver = {
	.probe			= pif_isr_probe,
	.remove			= pif_isr_remove,
#ifdef CONFIG_PM
	.suspend		= pif_isr_suspend,
	.resume			= pif_isr_resume,
#endif
	.driver = {
		.name		= "pif_isr",
		.bus		= &platform_bus_type,
		.of_match_table = of_match_ptr(of_pif_ids),
	},
};

static int pif_isr_probe(struct platform_device *pdev)
{
	int ret = 0;

	ret = pif_isr_init_irq(pdev);
	if (ret) {
		rtd_pr_pif_isr_err("pif_isr: can not register irq...\n");
		return ret;
	}

	return 0;
}


static int pif_isr_remove(struct platform_device *pdev)
{
	pif_isr_deinit_irq(pdev);
	return 0;
}


#ifdef CONFIG_PM
static int pif_isr_suspend(struct platform_device *dev, pm_message_t state)
{
	sfg_sfg_irq_ctrl_0_RBUS sfg_sfg_irq_ctrl_0_reg;

	rtd_pr_pif_isr_notice("[pif_ISR]%s %d\n",__func__,__LINE__);

	//enable sfg interrupt
	sfg_sfg_irq_ctrl_0_reg.regValue = rtd_inl(SFG_SFG_irq_ctrl_0_reg);
	sfg_sfg_irq_ctrl_0_reg.sfg_tim_irq_en = 0;	//sfg vsync output
	rtd_outl(SFG_SFG_irq_ctrl_0_reg, sfg_sfg_irq_ctrl_0_reg.regValue);

	rtd_pr_pif_isr_notice("[pif_ISR] suspend done\n");

	return 0;
}

static int pif_isr_resume(struct platform_device *dev)
{
	sys_reg_int_ctrl_scpu_RBUS sys_reg_int_ctrl_scpu_reg;
	sfg_sfg_irq_ctrl_0_RBUS sfg_sfg_irq_ctrl_0_reg;

	rtd_pr_pif_isr_notice("[pif_ISR]%s %d\n",__func__,__LINE__);

	// enable route to SCPU,  Dctl_int_2_scpu_routing_en
	sys_reg_int_ctrl_scpu_reg.regValue = 0;
	sys_reg_int_ctrl_scpu_reg.osd_int_scpu_routing_en = 1;  //Interrupt enable (osd_int & pif_irq)
	sys_reg_int_ctrl_scpu_reg.write_data = 1;
	rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, sys_reg_int_ctrl_scpu_reg.regValue);

	//enable sfg interrupt
	sfg_sfg_irq_ctrl_0_reg.regValue = rtd_inl(SFG_SFG_irq_ctrl_0_reg);
	sfg_sfg_irq_ctrl_0_reg.sfg_tim_irq_en = 5;	//sfg vsync output
	rtd_outl(SFG_SFG_irq_ctrl_0_reg, sfg_sfg_irq_ctrl_0_reg.regValue);

	rtd_pr_pif_isr_notice("[pif_ISR] resume done\n");

	return 0;
}

#endif

extern void pif_video_latency_pattern_ctrl(void);
irqreturn_t pif_isr(int irq, void *dev_id)
{
	unsigned int ret=IRQ_NONE;
	sfg_sfg_irq_ctrl_1_RBUS sfg_sfg_irq_ctrl_1_reg;
    ppoverlay_double_buffer_ctrl2_RBUS ppoverlay_double_buffer_ctrl2_reg;
	//ORBIT_PIXEL_SHIFT_STRUCT orbit_shift;
	//static unsigned int cost_time=0;


	sfg_sfg_irq_ctrl_1_reg.regValue = IoReg_Read32(SFG_SFG_irq_ctrl_1_reg);

	if(sfg_sfg_irq_ctrl_1_reg.sfg_tim_irq_flag & _BIT2){
        ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
        //rtd_pr_pif_isr_info("pif_isr orbit: memc_dtgreg_dbuf_set= %d \n", ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set);
#if 0
		if(!ppoverlay_double_buffer_ctrl2_reg.memc_dtgreg_dbuf_set && vbe_disp_oled_orbit_enable)
		{
			orbit_shift = Get_Orbit_Shift_Data();
			if((pre_orbit_shift.x != orbit_shift.x) || (pre_orbit_shift.y != orbit_shift.y))
			{
				//rtd_pr_pif_isr_info("pif_isr: line count den_end=  %d\n", PPOVERLAY_new_meas1_linecnt_real_get_memcdtg_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg)) );
				if(vbe_disp_oled_orbit_mode == _VBE_PANEL_ORBIT_JUSTSCAN_MODE){
					//rtd_pr_pif_isr_info("pif_isr orbit: line count den_end=  %d\n", PPOVERLAY_new_meas1_linecnt_real_get_memcdtg_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg)) );

					if(vbe_disp_orbit_set_position_justscan(orbit_shift.x, orbit_shift.y) ==0)
					{
						vbe_disp_orbit_set_justscan_finish(TRUE);
						pre_orbit_shift = orbit_shift;
					}
				}
				else
				{
					pre_orbit_shift.x=0;
					pre_orbit_shift.y=0;
				}
			}
			else if(vbe_disp_oled_orbit_mode != _VBE_PANEL_ORBIT_JUSTSCAN_MODE)
			{
				pre_orbit_shift.x=0;
				pre_orbit_shift.y=0;
			}
		}
		ppoverlay_double_buffer_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL2_reg);
		ppoverlay_double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 1;
		IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL2_reg, ppoverlay_double_buffer_ctrl2_reg.regValue);
        }
#endif

#if 0	//ECN:RL6672-2036 verify after ic back
		if(vbe_disp_get_vrr_average_den_update()){
			if(vbe_disp_get_vrr_average_den_mode() == 1){
				vbe_disp_set_VRR_60Hz_average_data_enable_on(1);
			}else{
				vbe_disp_set_VRR_60Hz_average_data_enable_on(0);
			}
			vbe_disp_set_vrr_average_den_update(0);
		}
#endif
		IoReg_Write32(SFG_SFG_irq_ctrl_1_reg, _BIT2);
	}
	if(sfg_sfg_irq_ctrl_1_reg.sfg_tim_irq_flag & _BIT1){
#ifdef CONFIG_SFG_PATTERN_MEASUREMENT
		pif_video_latency_pattern_ctrl();
#endif

		IoReg_Write32(SFG_SFG_irq_ctrl_1_reg, _BIT1);
	}
	if(sfg_sfg_irq_ctrl_1_reg.sfg_tim_irq_flag & _BIT0){
		vbe_disp_dynamic_polarity_control_analyze_pattern();
/*
		//use tcon18 0xb802d458[7] decide enable 28s toggle or not.
		if(TCON_TCON18_Ctrl_get_tcon18_en(IoReg_Read32(TCON_TCON18_Ctrl_reg))){

			if(vbe_disp_tcon_28s_toggle_get_state()== DISP_TCON_TOGGLE_STATE_NONE){
				vbe_disp_tcon_28s_toggle_handle_state(DISP_TCON_TOGGLE_STATE_NONE);
				vbe_disp_tcon_28s_toggle_set_state(DISP_TCON_TOGGLE_STATE_INIT);
			}

			if(vbe_disp_tcon_28s_toggle_check_timeout()){
				if(vbe_disp_tcon_28s_toggle_get_state()== DISP_TCON_TOGGLE_STATE_INIT){
					vbe_disp_tcon_28s_toggle_set_state(DISP_TCON_TOGGLE_STATE_ACTIVE1);
				}else if(vbe_disp_tcon_28s_toggle_get_state()== DISP_TCON_TOGGLE_STATE_ACTIVE1){
					vbe_disp_tcon_28s_toggle_set_state(DISP_TCON_TOGGLE_STATE_ACTIVE2);
				}else if(vbe_disp_tcon_28s_toggle_get_state()== DISP_TCON_TOGGLE_STATE_ACTIVE2){
					vbe_disp_tcon_28s_toggle_set_state(DISP_TCON_TOGGLE_STATE_INIT);
				}else{
					vbe_disp_tcon_28s_toggle_set_state(DISP_TCON_TOGGLE_STATE_INIT);
				}
				vbe_disp_tcon_28s_toggle_handle_state(vbe_disp_tcon_28s_toggle_get_state());
			}
		}
*/

/*
		if(!cost_time){
			cost_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		}else{
			rtd_pr_pif_isr_err("[pif_isr] period:%d,(%x) \n", (IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)- cost_time)/90, IoReg_Read32(0xb8028248));
			cost_time = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		}
*/
		IoReg_Write32(SFG_SFG_irq_ctrl_1_reg, _BIT0);
		ret = IRQ_HANDLED;
	}
	return ret;
}

static int pif_isr_init_irq(struct platform_device *pdev)
{
	sys_reg_int_ctrl_scpu_RBUS sys_reg_int_ctrl_scpu_reg;
	sfg_sfg_irq_ctrl_0_RBUS sfg_sfg_irq_ctrl_0_reg;
	int irq, ret;


	IoReg_Write32(SFG_SFG_irq_ctrl_0_reg, IoReg_Read32(SFG_SFG_irq_ctrl_0_reg));

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		rtd_pr_pif_isr_err("pif_isr: cannot get IRQ. ret=%d\n", irq);
		return irq;
	}
	/* Request IRQ */
	ret = request_irq(irq, pif_isr, IRQF_SHARED, "PIF ISR", (void *)pdev);
	if (ret) {
		rtd_pr_pif_isr_err("pif_isr: cannot register IRQ %d. ret=%d\n", irq, ret);
		return ret;
	}

	// enable route to SCPU,  Dctl_int_2_scpu_routing_en
	sys_reg_int_ctrl_scpu_reg.regValue = 0;
	sys_reg_int_ctrl_scpu_reg.osd_int_scpu_routing_en = 1;  //Interrupt enable (osd_int & pif_irq)
	sys_reg_int_ctrl_scpu_reg.write_data = 1;
	rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, sys_reg_int_ctrl_scpu_reg.regValue);

	//enable sfg interrupt
	sfg_sfg_irq_ctrl_0_reg.regValue = rtd_inl(SFG_SFG_irq_ctrl_0_reg);
	sfg_sfg_irq_ctrl_0_reg.sfg_tim_irq_en = 7;	//sfg vsync output
	rtd_outl(SFG_SFG_irq_ctrl_0_reg, sfg_sfg_irq_ctrl_0_reg.regValue);

	rtd_pr_pif_isr_info("pif_isr: register IRQ %d\n", irq);

	return 0;
}


static void pif_isr_deinit_irq(struct platform_device *pdev)
{
	sys_reg_int_ctrl_scpu_RBUS sys_reg_int_ctrl_scpu_reg;
	sfg_sfg_irq_ctrl_0_RBUS sfg_sfg_irq_ctrl_0_reg;
	int irq;


	IoReg_Write32(SFG_SFG_irq_ctrl_0_reg, IoReg_Read32(SFG_SFG_irq_ctrl_0_reg));

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		rtd_pr_pif_isr_err("pif_isr: cannot get IRQ. ret=%d\n", irq);
		return;
	}

	// disable route to SCPU,  Dctl_int_2_scpu_routing_en
	sys_reg_int_ctrl_scpu_reg.regValue = 0;
	sys_reg_int_ctrl_scpu_reg.osd_int_scpu_routing_en = 1;  //Interrupt enable (osd_int & pif_irq)
	sys_reg_int_ctrl_scpu_reg.write_data = 0;
	rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, sys_reg_int_ctrl_scpu_reg.regValue);

	// disable sfg interrupt
	sfg_sfg_irq_ctrl_0_reg.regValue = rtd_inl(SFG_SFG_irq_ctrl_0_reg);
	sfg_sfg_irq_ctrl_0_reg.sfg_tim_irq_en = 0;	//sfg vsync output
	rtd_outl(SFG_SFG_irq_ctrl_0_reg, sfg_sfg_irq_ctrl_0_reg.regValue);

	free_irq(irq, (void *)pdev);
}


#if 0
static char *vgip_isr_devnode(struct device *dev, mode_t *mode)
{
	return NULL;
}
#endif

#ifdef CONFIG_SUPPORT_SCALER_MODULE
int pif_isr_init_module(void)
#else
static int pif_isr_init_module(void)
#endif
{
	int ret = 0;

	ret = platform_driver_register(&pif_isr_platform_driver);
	if (ret)
		rtd_pr_pif_isr_debug("pif_isr: can not register platform driver...\n");

	return ret;
}

void pif_isr_exit_module(void)
{
	platform_driver_unregister(&pif_isr_platform_driver);
}


#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(pif_isr_init_module);
module_exit(pif_isr_exit_module);
#endif

