/* drivers/media/platform/rtk-cec/rtk_cec.c
 *
 * Samsung rtk CEC driver
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This driver is based on the "cec interface driver for exynos soc" by
 * SangPil Moon.
 */

#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <media/cec.h>
#include <linux/of_irq.h>
#include <linux/delay.h>
#include "rtk_cec_defines.h"
#include "rtk_cec_reg.h"
#include "rtk_cec.h"
#include "rtk_cec_sysfs.h"

static int rtk_cec_adap_enable(struct cec_adapter *adap, bool enable)
{
    struct rtk_cec *cec = adap->priv;
    return rtk_cec_hw_enable(&cec->priv_data, enable);
}

static int rtk_cec_adap_log_addr(struct cec_adapter *adap, u8 logical_addr)
{
    struct rtk_cec *cec = adap->priv;
    unsigned short rx_mask = 0;
    rx_mask = (unsigned short)((1 << logical_addr) | 0x8000);
    return rtk_cec_hw_set_rx_mask(&cec->priv_data, rx_mask);
}

static int rtk_cec_adap_transmit(struct cec_adapter *adap, u8 attempts,
                                 u32 signal_free_time, struct cec_msg *msg)
{
    struct rtk_cec *cec = adap->priv;

    signal_free_time = (signal_free_time > CEC_SIGNAL_FREE_TIME_NEXT_XFER) ? CEC_SIGNAL_FREE_TIME_NEXT_XFER : signal_free_time;
    if(signal_free_time)
        usleep_range(signal_free_time * 3000, signal_free_time * 3000);
  
    rtk_cec_hw_set_retry_num(&cec->priv_data, 0);    
    return rtk_cec_hw_xmit_message(&cec->priv_data, msg);

}

static const struct cec_adap_ops rtk_cec_adap_ops = {
    .adap_enable = rtk_cec_adap_enable,
    .adap_log_addr = rtk_cec_adap_log_addr,
    .adap_transmit = rtk_cec_adap_transmit,
};

static void rtk_cec_adapter_config(struct cec_adapter  *adap)
{
    if(adap) {
        adap->log_addrs.num_log_addrs = 1;
        //adap->log_addrs.log_addr[0] = CEC_LOG_ADDR_TV;
        //adap->log_addrs.log_addr_mask = (1 << CEC_LOG_ADDR_TV);
        adap->log_addrs.log_addr_type[0] = CEC_LOG_ADDR_TYPE_TV;
        adap->log_addrs.cec_version = CEC_VERSION_1_4;
        adap->log_addrs.vendor_id = CEC_VENDOR_ID;
        //adap->log_addrs.primary_device_type[0] = CEC_LOG_ADDR_TYPE_TV;
        adap->is_configured = false;
        adap->is_configuring = false;
    }
}

static int rtk_cec_probe(struct platform_device *pdev)
{
    u32 caps = CEC_CAP_LOG_ADDRS | CEC_CAP_TRANSMIT | CEC_CAP_PHYS_ADDR ;
    struct device_node *np = NULL;
    struct rtk_cec *cec = NULL;
    int ret = 0;
    cec = devm_kzalloc(&pdev->dev, sizeof(*cec), GFP_KERNEL);
    if (!cec)
        return -ENOMEM;

    cec->dev = &pdev->dev;

    np =  pdev->dev.of_node;
    if (!np) {
        cec_error("%s there is no device node\n",__func__);
        return -ENODEV;
    }

    cec->irq = irq_of_parse_and_map(np, 0);
    if (cec->irq == 0)
        return -ENODEV;;
    

    cec->adap = cec_allocate_adapter(&rtk_cec_adap_ops, cec,
                                     CEC_NAME,  caps, CEC_MAX_LOG_ADDRS);
    ret = PTR_ERR_OR_ZERO(cec->adap);
    if (ret)
        return ret;

    rtk_cec_adapter_config(cec->adap);
    
    ret = cec_register_adapter(cec->adap, cec->dev);
    if (ret) {
        cec_delete_adapter(cec->adap);
        return ret;
    }

    cec->priv_data.dev = cec->dev;
    cec->priv_data.adap = cec->adap;

     ret = rtk_cec_hw_init(&cec->priv_data, cec->irq);
     if(ret != 0) {
        cec_unregister_adapter(cec->adap);
        cec_delete_adapter(cec->adap);
        return ret;
     }

    platform_set_drvdata(pdev, cec);

    rtk_cec_sysfs_init();

    return 0;
}

static int rtk_cec_remove(struct platform_device *pdev)
{
    struct rtk_cec *cec = platform_get_drvdata(pdev);
    rtk_cec_sysfs_uninit();
    rtk_cec_hw_uninit(&cec->priv_data, cec->irq);
    cec_unregister_adapter(cec->adap);
    cec_delete_adapter(cec->adap);
    return 0;
}



static const struct of_device_id rtk_cec_of_match[] = {
    { .compatible = "realtek,rtk-cec" },
    {}
};

#ifdef CONFIG_PM
int rtk_cec_pm_suspend_noirq(struct device* dev)
{
    struct platform_device *pdev = container_of(dev, struct platform_device, dev);
    if(pdev) {
        struct rtk_cec *cec = platform_get_drvdata(pdev);
        rtk_cec_hw_suspend(&cec->priv_data);
   }
    return 0;
}

int rtk_cec_pm_resume_noirq(struct device* dev)
{
     struct platform_device *pdev = container_of(dev, struct platform_device, dev);
    if(pdev) {
        struct rtk_cec *cec = platform_get_drvdata(pdev);
        rtk_cec_hw_resume(&cec->priv_data);
   }
    return 0;   
}
static const struct dev_pm_ops rtk_cec_pm_ops = {
    .suspend_noirq = rtk_cec_pm_suspend_noirq,
    .resume_noirq = rtk_cec_pm_resume_noirq,
#ifdef CONFIG_HIBERNATION
    .freeze_noirq = rtk_cec_pm_suspend_noirq,
    .thaw_noirq = rtk_cec_pm_resume_noirq,
    .poweroff_noirq = rtk_cec_pm_suspend_noirq,
    .restore_noirq = rtk_cec_pm_resume_noirq,
#endif
};
#endif

MODULE_DEVICE_TABLE(of, rtk_cec_of_match);

static struct platform_driver rtk_cec_pdrv = {
    .probe  = rtk_cec_probe,
    .remove = rtk_cec_remove,
    .driver = {
        .name       = CEC_NAME,
        .of_match_table = rtk_cec_of_match,
#ifdef CONFIG_PM
        .pm		= &rtk_cec_pm_ops,
#endif
    },
};

module_platform_driver(rtk_cec_pdrv);

MODULE_AUTHOR("liangliang_song <liangliang_song@apowertec.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RTK CEC driver");
