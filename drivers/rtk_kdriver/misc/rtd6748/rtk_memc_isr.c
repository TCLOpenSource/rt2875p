//Copyright (C) 2007-2013 Realtek Semiconductor Corporation.
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include "memc_isr/scalerMEMC.h"
#ifndef CONFIG_MEMC_NOTSUPPORT
#include "memc_reg_def.h"
#endif
#include "scaler_vpqmemcdev.h"
#include <rbus/sys_reg_reg.h>
#include "rbus/ppoverlay_reg.h"

#ifdef CONFIG_ARM64 //ARM32 compatible
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
#endif

static int __exit memc_isr_remove(struct platform_device *pdev);
static int memc_isr_suspend(struct platform_device *dev, pm_message_t state);
static int memc_isr_resume(struct platform_device *dev);

static const struct of_device_id memc_isr_of_match[] = {
	{
		.compatible = "realtek,memc_isr",
	},
	{},
};
MODULE_DEVICE_TABLE(of, memc_isr_of_match);

static struct platform_driver memc_isr_platform_driver = {
        .remove                 = memc_isr_remove,
#ifdef CONFIG_PM
        .suspend                = memc_isr_suspend,
        .resume                 = memc_isr_resume,
#endif
        .driver = {
                .name           = "memc_isr",
                .bus            = &platform_bus_type,
                .of_match_table = of_match_ptr(memc_isr_of_match),
        },
};

#ifdef CONFIG_PM
extern unsigned char get_MEMC_bypass_status_refer_platform_model(void);
static int memc_isr_suspend(struct platform_device *dev, pm_message_t state)
{
	unsigned int u32_interrupt_reg = 0;

	rtd_pr_memc_notice("[MEMC_ISR]%s %d\n",__func__,__LINE__);

	memc_suspend();

	if( (Scaler_MEMC_CLK_Check() == FALSE) || (get_MEMC_bypass_status_refer_platform_model() == TRUE)){
		return FALSE;
	}
	// disable interrupt
	//reg_kmc_int_en
	u32_interrupt_reg = rtd_inl(KMC_TOP_kmc_top_18_reg);//MC_TOP__REG_KMC_IN_INT_SEL_ADDR
	u32_interrupt_reg &= 0xf0ffffff;
	rtd_outl(KMC_TOP_kmc_top_18_reg, u32_interrupt_reg);//MC_TOP__REG_KMC_IN_INT_SEL_ADDR

	//reg_post_int_en
	u32_interrupt_reg = rtd_inl(KPOST_TOP_KPOST_TOP_08_reg);//KPOST_TOP__REG_POST_INT_SOURCE_SEL_ADDR
	u32_interrupt_reg &= 0xfffffe1f;
	rtd_outl(KPOST_TOP_KPOST_TOP_08_reg, u32_interrupt_reg);//KPOST_TOP__REG_POST_INT_SOURCE_SEL_ADDR

	rtd_pr_memc_notice("[MEMC_ISR] suspend done\n");

        return 0;

}

extern unsigned char u1_DoPowerSaving;
unsigned char MEMC_isr_resume_done_jerry = 0;
static int memc_isr_resume(struct platform_device *dev)
{
  	rtd_pr_memc_notice("[MEMC_ISR]%s %d\n",__func__,__LINE__);

	// disable route to VCPU1
	rtd_outl(SYS_REG_INT_CTRL_MEMC_reg, BIT(SYS_REG_INT_CTRL_MEMC_memc_int_vcpu_routing_en_shift));
	// disable route to VCPU2
	rtd_outl(SYS_REG_INT_CTRL_MEMC_reg, BIT(SYS_REG_INT_CTRL_MEMC_memc_int_vcpu2_routing_en_shift));
	// enable route to SCPU
	rtd_outl(SYS_REG_INT_CTRL_MEMC_reg, (SYS_REG_INT_CTRL_MEMC_memc_int_scpu_routing_en_mask|SYS_REG_INT_CTRL_MEMC_write_data_mask));

	u1_DoPowerSaving = 0;
	MEMC_isr_resume_done_jerry = 1;
	rtd_pr_memc_notice("[MEMC_ISR] resume done\n");

        return 0;
}
#endif

extern BOOL Mid_MISC_GetInINTStatus(INT_TYPE enIntType);
extern VOID Mid_MISC_IntINTLock(INT_TYPE enIntType);
extern BOOL Mid_MISC_GetInINTEnable(INT_TYPE enIntType);
extern BOOL Mid_MISC_GetOutINTStatus(INT_TYPE enIntType);
extern VOID Mid_MISC_OutINTLock(INT_TYPE enIntType);
extern BOOL Mid_MISC_GetOutINTEnable(INT_TYPE enIntType);
extern VOID MEMC_LibOutputHSInterrupt(VOID);
extern VOID Mid_MISC_IntINTUnLock(INT_TYPE enIntType);
extern VOID Mid_MISC_IntINTCLear(INT_TYPE enIntType);
extern VOID Mid_MISC_OutINTUnLock(INT_TYPE enIntType);
extern VOID Mid_MISC_OutINTCLear(INT_TYPE enIntType);
extern unsigned char MEMC_Lib_get_memc_PowerSaving_Mode(VOID);
extern int64_t MEMC_GetPTS(void);

static irqreturn_t memc_isr(int irq, void *dev_id)
{
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
	return RT_ISR_HANDLED;
#else

#if	1
	unsigned char handle_GetInINTEnable_flag = 0;
	unsigned char handle_GetOutINTEnable_Vsync_flag = 0;
	unsigned char handle_GetOutINTEnable_Hsync_flag = 0;
	ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
	UINT32 u32_rb_val = 0;
	unsigned int time_1 = 0, time_2 = 0, time_3 = 0, time_4 = 0, time_5 = 0, time_6 = 0;
	#if 0
	unsigned int i = 0, cnt = 0;
	#endif
	unsigned char log_en = 0;
	//UINT32 u32_rb_MEMC_MUX = 0;

#ifdef MEMC_ISR_Times_debug
		static unsigned int sEnterCnt = 0;
		static unsigned int sEnterCnt_in = 0;
		static unsigned int sEnterCnt_out_H = 0;
		static unsigned int sEnterCnt_out_V = 0;
		static int64_t sLastPts_1s = 0;
		int64_t currentPts = MEMC_GetPTS();		

		if(currentPts < sLastPts_1s){
			sLastPts_1s = currentPts;
			sEnterCnt = 0;
		}				
#endif

	u32_rb_val = rtd_inl(SYS_REG_SYS_CLKEN3_reg);	
	u32_rb_val = (u32_rb_val>>31);
	memc_mux_ctrl_reg.regValue = rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg);

	if(Mid_MISC_GetInINTStatus(INT_TYPE_VSYNC))
	{
		Mid_MISC_IntINTLock(INT_TYPE_VSYNC);
		if(Mid_MISC_GetInINTEnable(INT_TYPE_VSYNC)){			
			handle_GetInINTEnable_flag = 1;
		}
		#ifdef MEMC_ISR_Times_debug		
		sEnterCnt_in++;
		/*if(currentPts - sLastPts_1s > 90090){			
			rtd_pr_memc_notice("in(cnt:%d)'\n", sEnterCnt_in);
			sEnterCnt_in = 0;
		}*/		
		#endif		
	}

	if(Mid_MISC_GetOutINTStatus(INT_TYPE_HSYNC))
	{
		Mid_MISC_OutINTLock(INT_TYPE_HSYNC);
		if(Mid_MISC_GetOutINTEnable(INT_TYPE_HSYNC)){	
			handle_GetOutINTEnable_Hsync_flag = 1;
		}
		#ifdef MEMC_ISR_Times_debug		
		sEnterCnt_out_H++;
		/*if(currentPts - sLastPts_1s > 90090){			
			rtd_pr_memc_notice("out_H(cnt:%d)'\n", sEnterCnt_out_H);
			sEnterCnt_out_H = 0;
		}*/		
		#endif		
		
	}

	if(Mid_MISC_GetOutINTStatus(INT_TYPE_VSYNC))
	{
		Mid_MISC_OutINTLock(INT_TYPE_VSYNC);
		if(Mid_MISC_GetOutINTEnable(INT_TYPE_VSYNC)){
			handle_GetOutINTEnable_Vsync_flag = 1;
		}
		#ifdef MEMC_ISR_Times_debug		
		sEnterCnt_out_V++;
		/*if(currentPts - sLastPts_1s > 90090){			
			rtd_pr_memc_notice("out_V(cnt:%d)'\n", sEnterCnt_out_V);
			sEnterCnt_out_V = 0;
		}*/				
		#endif		
	}

	if (u32_rb_val == 1 && (MEMC_Lib_get_memc_PowerSaving_Mode()!=1)){

		if(memc_mux_ctrl_reg.memc_outmux_sel){
			Scaler_MEMC_ISR();
		}
		MEMC_Interrupt_CheckRunning();
		log_en = rtd_inl(SOFTWARE2_SOFTWARE2_60_reg)>>31;
		
		if(memc_mux_ctrl_reg.memc_outmux_sel){
			if(handle_GetInINTEnable_flag){
				time_1 = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
				#if 0
				for(i = 0; i < 10000; i++){
					cnt++;
					rtd_outl(SOFTWARE2_SOFTWARE2_56_reg, cnt);
				}
				#else
				Scaler_MEMC_ISR_InputVSInterrupt();
				#endif
				time_2 = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
				if(log_en){
					rtd_pr_memc_notice("[in_time_diff,%d,ms]\n", (time_2-time_1)/90);
				}
			}
			
			if(handle_GetOutINTEnable_Vsync_flag){
				time_3 = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
				#if 0
				for(i = 0; i < 10000; i++){
					cnt++;
					rtd_outl(SOFTWARE2_SOFTWARE2_57_reg, cnt);
				}
				#else
				Scaler_MEMC_ISR_OutputVSInterrupt();
				#endif
				time_4 = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
				if(log_en){
					rtd_pr_memc_notice("[out_v_time_diff,%d,ms]\n", (time_4-time_3)/90);
				}
			}

			if(handle_GetOutINTEnable_Hsync_flag){			
				time_5 = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
				#if 0
				for(i = 0; i < 10000; i++){
					cnt++;
					rtd_outl(SOFTWARE2_SOFTWARE2_58_reg, cnt);
				}
				#else
				MEMC_LibOutputHSInterrupt();
				#endif
				time_6 = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
				if(log_en){
					rtd_pr_memc_notice("[out_h_time_diff,%d,ms]\n", (time_6-time_5)/90);
				}
			}
		}	
		
	}
	else {			
		if (u32_rb_val != 1) {
			//rtd_pr_memc_notice("[Scaler_MEMC_ISR] memc clock off, disable isr now!!\n\r");
		}
	}

	if(handle_GetOutINTEnable_Vsync_flag){
		Mid_MISC_OutINTUnLock(INT_TYPE_VSYNC);
		Mid_MISC_OutINTCLear(INT_TYPE_VSYNC);
	}

	if(handle_GetOutINTEnable_Hsync_flag){
		Mid_MISC_OutINTUnLock(INT_TYPE_HSYNC); 
		Mid_MISC_OutINTCLear(INT_TYPE_HSYNC);
	}	

	if(handle_GetInINTEnable_flag){
		Mid_MISC_IntINTUnLock(INT_TYPE_VSYNC);
		Mid_MISC_IntINTCLear(INT_TYPE_VSYNC);
	}

	#ifdef MEMC_ISR_Times_debug
	sEnterCnt++;
	if(currentPts - sLastPts_1s > 90090){
		rtd_pr_memc_notice("(all_cnt : ,%d,%d,%d,%d,)(time,%ld,%ld,%ld,)[time_diff,%d,%d,%d,]\n", sEnterCnt, sEnterCnt_in, sEnterCnt_out_V, sEnterCnt_out_H,
		(long)currentPts, (long)sLastPts_1s, (long)(currentPts - sLastPts_1s), time_2-time_1, time_4-time_3, time_6-time_5);
		sLastPts_1s = currentPts;
		sEnterCnt = 0;
		sEnterCnt_in = 0;
		sEnterCnt_out_H = 0;
		sEnterCnt_out_V = 0;
	}	
	#endif
	

	if( handle_GetInINTEnable_flag || handle_GetOutINTEnable_Vsync_flag || handle_GetOutINTEnable_Hsync_flag ){
		//ROSPrintfInternal("[memc_isr]interrupt finish correct (%d,%d)\n",handle_den_end_event_flag,handle_GetInINTEnable_flag);
		return IRQ_HANDLED;
	}else{
		//ROSPrintfInternal("[memc_isr]interrupt finish wrong (%d)(%x,%x)\n",handle_GetInINTEnable_flag,IoReg_Read32(0xb8000290),IoReg_Read32(0xb80280f4));	
		return IRQ_NONE;
	}	

#else
	Scaler_MEMC_ISR();
	return IRQ_HANDLED;

#endif
#endif
}

extern u32 gic_irq_find_mapping(u32 hwirq);
static int __init memc_isr_init_irq(struct platform_device *pdev)
{
	int irq, ret;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;


	ret = request_irq(irq, memc_isr, IRQF_SHARED, "MEMC ISR",
			(void *)pdev);
	if (ret) {
		dev_err(&pdev->dev, "memc_isr: cannot register IRQ %ld\n", irqd_to_hwirq(irq_get_irq_data(irq))); //fix me
		return ret;
	}

	//setup IRQ route and enable IRQ
	// disable route to VCPU1
	rtd_outl(SYS_REG_INT_CTRL_MEMC_reg, BIT(SYS_REG_INT_CTRL_MEMC_memc_int_vcpu_routing_en_shift));
	// disable route to VCPU2
	rtd_outl(SYS_REG_INT_CTRL_MEMC_reg, BIT(SYS_REG_INT_CTRL_MEMC_memc_int_vcpu2_routing_en_shift));
	// enable route to SCPU
	rtd_outl(SYS_REG_INT_CTRL_MEMC_reg, 0x00100001);
	// enable MEMC interrupt
	//??

	dev_info(&pdev->dev, "memc_isr: register IRQ %ld\n", irqd_to_hwirq(irq_get_irq_data(irq)));//fix me

	return 0;
}


static int __init memc_isr_probe(struct platform_device *pdev)
{
	int ret = 0;

	if(get_MEMC_bypass_status_refer_platform_model() == TRUE)
		return 0;

	ret = memc_isr_init_irq(pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "memc_isr: can not register irq...\n");
		return ret;
	}

	return 0;
}


static int __exit memc_isr_remove(struct platform_device *pdev)
{
	int irq;

	irq = platform_get_irq(pdev, 0);
	if (irq >= 0)
		free_irq(irq, (void *)pdev);

	return 0;
}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
int __init memc_isr_init_module(void)
#else
static int __init memc_isr_init_module(void)
#endif
{
	return platform_driver_probe(&memc_isr_platform_driver, memc_isr_probe);
}

void memc_isr_exit_module(void)
{
	platform_driver_unregister(&memc_isr_platform_driver);
}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(memc_isr_init_module);
module_exit(memc_isr_exit_module);
#endif