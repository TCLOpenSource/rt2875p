/*=============================================================
 * File:    dprx_adapter-rtk_dprx-platform-linux.c
 *
 * Desc:    driver wrapper for Linux kernel
 *
 * AUTHOR:  kevin_wang@realtek.com
 *
 * Vresion: 0.0.1
 *
 *------------------------------------------------------------
 * Copyright (c) Realtek Semiconductor Corporation, 2022
 *
 * All rights reserved.
 *
 *============================================================*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/kthread.h>
#include <dprx_adapter.h>
#include <dprx_adapter-rtk_dprx-priv.h>
#include <dprx_adapter-rtk_dprx-plat.h>
#include <dprx_vfe_api.h>
#include <dprx_vfe.h>
#ifdef CONFIG_RTK_MEASURE
#include <measure/rtk_measure.h>
#endif

static int g_dprx_irq = -1;
static int g_dprx_irq_ready = 0;

static int g_dprx_aux_irq = -1;
static int g_dprx_aux_irq_ready = 0;

/*------------------------------------------------------------------
 * Func : rtk_dprx_lt_event_message_enable
 *
 * Desc : link training debug message control
 *
 * Parm : p_dprx : handle of DPRX
 *
 * Retn : 0 : event message disable, 1 : event message enable
 *------------------------------------------------------------------*/
int rtk_dprx_lt_event_message_enable(RTK_DPRX_ADP* p_dprx)
{
    return 1; // alway print debug message
}

/*------------------------------------------------------------------
 * Func : rtk_dprx_irq_handler
 *
 * Desc : irq handler
 *
 * Parm : pdev : handle of dprx device
 *
 * Retn : success / fail
 *------------------------------------------------------------------*/
irqreturn_t rtk_dprx_irq_handler(int irq, void *dev_id)
{
    irqreturn_t ret = IRQ_NONE;
    dprx_drv_interrupt_handler_exint0();
    dprx_drv_exit_interrupt_handler_exint0();
    return ret;
}


/*------------------------------------------------------------------
 * Func : rtk_dprx_plat_init
 *
 * Desc : dprx platform init
 *
 * Parm : N/A
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_dprx_plat_init(void)
{
    return 0; // do nothing
}


/*------------------------------------------------------------------
 * Func : rtk_dprx_plat_interrupt_enable
 *
 * Desc : enable dprx interrupt
 *
 * Parm : N/A
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_dprx_plat_interrupt_enable(void)
{
    if (g_dprx_irq_ready==0)
    {
        if (request_irq(g_dprx_irq, rtk_dprx_irq_handler, IRQF_SHARED, "DPRX_MAC", rtk_dprx_irq_handler))
        {
            DPRX_ADP_WARN("!!!!!!!!!!!!!!!!! Register DPRX Interrupt handler failed - IRQ %d !!!!!!!!!!!!!!!!\n", g_dprx_irq);
            return -1;
        }
        DPRX_ADP_INFO("!!!!!!!!!!!!!!!!! Register DPRX Interrupt handler successed - IRQ %d !!!!!!!!!!!!!!!!\n", g_dprx_irq);
        g_dprx_irq_ready = 1;
    }

    if (g_dprx_aux_irq_ready==0)
    {
        if (request_irq(g_dprx_aux_irq, rtk_dprx_irq_handler, IRQF_SHARED, "DPRX_AUX", rtk_dprx_irq_handler))
        {
            DPRX_ADP_WARN("!!!!!!!!!!!!!!!!! Register DP AUX Interrupt handler failed - IRQ %d !!!!!!!!!!!!!!!!\n", g_dprx_aux_irq);
            return -1;
        }

        DPRX_ADP_INFO("!!!!!!!!!!!!!!!!! Register DP AUX Interrupt handler successed - IRQ %d !!!!!!!!!!!!!!!!\n", g_dprx_aux_irq);
        g_dprx_aux_irq_ready = 1;
    }

    if (SYS_REG_INT_CTRL_SCPU_2_get_dp_auxrx_int_scpu_routing_en(rtd_inl(SYS_REG_INT_CTRL_SCPU_2_reg))==0)
    {
        rtd_outl(SYS_REG_INT_CTRL_SCPU_2_reg, SYS_REG_INT_CTRL_SCPU_2_dp_auxrx_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_2_write_data(1));

        DPRX_ADP_WARN("enable auxrx_int_scpu_routing !! SYS_REG_INT_CTRL_SCPU_2_reg=%08x (dp_auxrx_int_scpu_routing_en=%d)\n",
                dprx_inb(SYS_REG_INT_CTRL_SCPU_2_reg),
                SYS_REG_INT_CTRL_SCPU_2_get_dp_auxrx_int_scpu_routing_en(rtd_inl(SYS_REG_INT_CTRL_SCPU_2_reg)));
    }

    return 0;
}


/*------------------------------------------------------------------
 * Func : rtk_dprx_plat_interrupt_disable
 *
 * Desc : disable dprx interrupt
 *
 * Parm : N/A
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_dprx_plat_interrupt_disable(void)
{
    if (g_dprx_irq_ready)
    {
        free_irq(g_dprx_irq, rtk_dprx_irq_handler);
        g_dprx_irq_ready = 0;
    }

    if (g_dprx_aux_irq_ready)
    {
        free_irq(g_dprx_aux_irq, rtk_dprx_irq_handler);
        rtd_outl(SYS_REG_INT_CTRL_SCPU_2_reg, SYS_REG_INT_CTRL_SCPU_2_dp_auxrx_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_2_write_data(0));
        g_dprx_aux_irq_ready = 0;
    }

    return 0;
}


/////////////////////////////////////////////////////////////////////
// Platform device Interface
/////////////////////////////////////////////////////////////////////


#ifdef CONFIG_PM

static int rtk_dprx_pm_suspend(struct device *dev)
{
    DPRX_ADP_INFO("rtk_dprx_pm_suspend\n");
    vfe_dprx_drv_suspend();
    return 0;
}

static int rtk_dprx_pm_resume(struct device *dev)
{
    DPRX_ADP_INFO("rtk_dprx_pm_resume\n");
    vfe_dprx_drv_resume();
    return 0;
}

#ifdef CONFIG_HIBERNATION

static int rtk_dprx_pm_suspend_std(struct device *dev)
{
    DPRX_ADP_INFO("rtk_dprx_pm_suspend_std\n");
    return 0;
}
static int rtk_dprx_pm_resume_std(struct device *dev)
{
    DPRX_ADP_INFO("rtk_dprx_pm_resume_std\n");
    return 0;
}

#endif

static const struct dev_pm_ops rtk_dprx_pm_ops = {

    .suspend = rtk_dprx_pm_suspend,
    .resume  = rtk_dprx_pm_resume,

#ifdef CONFIG_HIBERNATION
    .freeze     = rtk_dprx_pm_suspend_std,
    .thaw       = rtk_dprx_pm_resume_std,
#endif
};


#endif

/*------------------------------------------------------------------
 * Func : rtk_dprx_drv_probe
 *
 * Desc : probe dprx device
 *
 * Parm : pdev : handle of dprx device
 *
 * Retn : success / fail
 *------------------------------------------------------------------*/
static int rtk_dprx_drv_probe(
    struct platform_device*     pdev
    )
{
    g_dprx_irq = platform_get_irq_byname(pdev, "DPRX_MAC");
    g_dprx_aux_irq = platform_get_irq_byname(pdev, "DPRX_AUX");

    if (g_dprx_irq<0 || g_dprx_aux_irq <0)
    {
        DPRX_ADP_WARN("rtk_dprx_drv_probe failed : get irq failed - %d/%d\n",
        g_dprx_irq, g_dprx_aux_irq);
        return -1;
    }

    DPRX_ADP_WARN("rtk_dprx_drv_probe success : MAC_IRQ=%d/AUX_IRQ=%d\n",
        g_dprx_irq, g_dprx_aux_irq);

#ifdef CONFIG_DPRX_VFE_ENABLE_QUICK_SHOW

    if (dprx_platform_get_quick_show_channel()>=0)
    {
        DPRX_ADP_WARN("rtk_dprx_drv_probe with quick show enabled : qs_channel=%d\n",
            dprx_platform_get_quick_show_channel());

        vfe_dprx_drv_init();
        vfe_dprx_drv_open();
        vfe_dprx_drv_connect(dprx_platform_get_quick_show_channel());
    }

#endif

    return 0;
}


/*------------------------------------------------------------------
 * Func : rtk_dprx_drv_remove
 *
 * Desc : remove dprx driver
 *
 * Parm : pdev : handle of dprx device
 *
 * Retn : 0
 *------------------------------------------------------------------*/
static int rtk_dprx_drv_remove(
    struct platform_device*     pdev
    )
{
    rtk_dprx_plat_interrupt_disable();
    return 0;
}


static const struct of_device_id rtk_dprx_of_match[] =
{
    {
        .compatible = "realtek,dprx",
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, rtk_dprx_of_match);

static struct platform_driver rtk_dprx_platform_driver =
{
    .probe      = rtk_dprx_drv_probe,
    .remove     = rtk_dprx_drv_remove,
    .driver     =
    {
        .name = "rtk_dprx",
        .of_match_table = rtk_dprx_of_match,
#ifdef CONFIG_PM
        .pm    = &rtk_dprx_pm_ops,
#endif
    },
};

/////////////////////////////////////////////////////////////////////
// Module Interface
/////////////////////////////////////////////////////////////////////


/*------------------------------------------------------------------
 * Func : rtk_dprx_module_init
 *
 * Desc : init function of dprx module
 *
 * Parm : N/A
 *
 * Retn : success / fail
 *------------------------------------------------------------------*/
int rtk_dprx_module_init(void)
{
    return platform_driver_register(&rtk_dprx_platform_driver);
}


/*------------------------------------------------------------------
 * Func : rtk_dprx_module_exit
 *
 * Desc : exif function of rtk_dprx
 *
 * Parm : N/A
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_dprx_module_exit(void)
{
    platform_driver_unregister(&rtk_dprx_platform_driver);
}

