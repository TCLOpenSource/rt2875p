//Copyright (C) 2007-2013 Realtek Semiconductor Corporation.
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#include <rtd_log/rtd_module_log.h>

#include "memc_isr/scalerMEMC.h"
//#include "tvscalercontrol/scaler_vpqmemcdev.h"

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

static int memc_isr_suspend(struct platform_device *dev, pm_message_t state);
static int memc_isr_resume(struct platform_device *dev);

static struct platform_device *memc_isr_platform_devs = NULL;
static struct platform_driver memc_isr_platform_driver = {
#ifdef CONFIG_PM
        .suspend                = memc_isr_suspend,
        .resume                 = memc_isr_resume,
#endif
        .driver = {
                .name           = "memc_isr",
                .bus            = &platform_bus_type,
        },
};

#ifdef CONFIG_PM
static int memc_isr_suspend(struct platform_device *dev, pm_message_t state)
{
	//unsigned int u32_interrupt_reg = 0;

	//rtd_pr_memc_notice("[MEMC_ISR]%s %d\n",__func__,__LINE__);

	//memc_suspend();

	//if(Scaler_MEMC_CLK_Check() == FALSE){
	//	return FALSE;
	//}
	// disable interrupt
	//reg_kmc_int_en
	//u32_interrupt_reg = rtd_inl(0xB8099018);//MC_TOP__REG_KMC_IN_INT_SEL_ADDR
	//u32_interrupt_reg &= 0xf0ffffff;
	//rtd_outl(0xB8099018, u32_interrupt_reg);//MC_TOP__REG_KMC_IN_INT_SEL_ADDR

	//reg_post_int_en
	//u32_interrupt_reg = rtd_inl(0xB809D008);//KPOST_TOP__REG_POST_INT_SOURCE_SEL_ADDR
	//u32_interrupt_reg &= 0xfffffe1f;
	//rtd_outl(0xB809D008, u32_interrupt_reg);//KPOST_TOP__REG_POST_INT_SOURCE_SEL_ADDR

	//rtd_pr_memc_notice("[MEMC_ISR] suspend done\n");

        return 0;

}


extern unsigned char u1_DoPowerSaving;
//unsigned char MEMC_isr_resume_done_jerry = 0;
static int memc_isr_resume(struct platform_device *dev)
{
  	rtd_pr_memc_notice("[MEMC_ISR]%s %d\n",__func__,__LINE__);

	// disable route to VCPU1
	//rtd_outl(SYS_REG_INT_CTRL_MEMC_reg, BIT(SYS_REG_INT_CTRL_MEMC_memc_int_vcpu_routing_en_shift));
	// disable route to VCPU2
	//rtd_outl(SYS_REG_INT_CTRL_MEMC_reg, BIT(SYS_REG_INT_CTRL_MEMC_memc_int_vcpu2_routing_en_shift));
	// enable route to SCPU
	//rtd_outl(SYS_REG_INT_CTRL_MEMC_reg, (SYS_REG_INT_CTRL_MEMC_memc_int_scpu_routing_en_mask|SYS_REG_INT_CTRL_MEMC_write_data_mask));

	u1_DoPowerSaving = 0;
	//MEMC_isr_resume_done_jerry = 1;
	rtd_pr_memc_notice("[MEMC_ISR] resume done\n");

        return 0;
}
#endif

irqreturn_t memc_isr(int irq, void *dev_id)
{

	Scaler_MEMC_ISR();

	return IRQ_HANDLED;
}

extern u32 gic_irq_find_mapping(u32 hwirq);
static int __init memc_isr_init_irq(void)
{
#if 0
	/* Request IRQ */
	if(request_irq(gic_irq_find_mapping(IRQ_MEMC),
                   memc_isr,
                   IRQF_SHARED,
                   "MEMC ISR",
                   (void *)memc_isr_platform_devs))
	{
		rtd_pr_memc_err("memc_isr: cannot register IRQ %d\n", gic_irq_find_mapping(IRQ_MEMC));
		return -ENXIO;
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

	rtd_pr_memc_info("memc_isr: register IRQ %d\n", gic_irq_find_mapping(IRQ_MEMC));
#endif
	return 0;
}

static int __init memc_isr_init_module(void)
{
	int result;
	memc_isr_platform_devs = platform_device_register_simple("memc_isr", -1, NULL, 0);
        if (platform_driver_register(&memc_isr_platform_driver) != 0) {
                rtd_pr_memc_info("memc_isr: can not register platform driver...\n");
                result = -EINVAL;
                return result;
        }
	result = memc_isr_init_irq();
	if (result < 0) {
		rtd_pr_memc_err("memc_isr: can not register irq...\n");
		return result;
	}

	return 0;
}

void memc_isr_exit_module(void)
{
//	free_irq(gic_irq_find_mapping(IRQ_MEMC), (void *)memc_isr);
}

module_init(memc_isr_init_module);
module_exit(memc_isr_exit_module);
