#include <linux/clk.h>
#include <linux/module.h>
#include <asm/delay.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <rtk_kdriver/io.h>

#include <mach/pcbMgr.h>
#include <mach/platform.h>
#include <mach/pcbMgr.h>
#include <rbus/sb2_reg.h>
#include "rtk_usb_core.h"

#define DRIVER_DESC "RTK USB generic platform driver"

#if IS_ENABLED(CONFIG_RTK_KDRV_XHCI_HCD_PLATFORM)
void usb3_crt_on(void);
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM)
void usb2_crt_on(void);
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_USB_OTG_PLATFORM)
bool rtk_otg_is_enable(void);
void rtk_otg_crt_on_off(bool on);
#endif

int __init rtk_usb_init(void);
void rtk_usb_exit(void);

static void power_on_all_usb_host(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_XHCI_HCD_PLATFORM)
    usb3_crt_on();
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM)
    usb2_crt_on();
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_USB_OTG_PLATFORM)
    if(rtk_otg_is_enable())
        rtk_otg_crt_on_off(true);
#endif
}


static int rtk_usb_platform_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *node = pdev->dev.of_node;
    int error = 0;

    power_on_all_usb_host();

    error = of_platform_populate(node, NULL, NULL, dev); /* populate all children nodes of this device */
    if (error)
        RTK_USB_ERR("failed to create rtk usb platform\n");

#if 0
    if (sysfs_create_group(&dev->kobj, &dev_attr_grp)) {
        RTK_USB_WARN("Create self-defined sysfs attributes fail \n");
    }
#endif

    return error;
}


static int rtk_usb_platform_remove(struct platform_device *pdev)
{
#if 0
    sysfs_remove_group(&pdev->dev.kobj, &dev_attr_grp);
#endif
    of_platform_depopulate(&pdev->dev);
    platform_set_drvdata(pdev, NULL);

    return 0;
}


static int rtk_usb_platform_resume(struct device *dev)
{
    power_on_all_usb_host();

    return 0;
}


static int rtk_usb_platform_restore(struct device *dev)
{
    power_on_all_usb_host();

    return 0;
}


static const struct dev_pm_ops rtk_usb_platform_pm_ops = {
    .resume = rtk_usb_platform_resume,
    .restore = rtk_usb_platform_restore,
};


static const struct of_device_id rtk_usb_of_match[] = {
    { .compatible = "rtk,usb-platform", },
    {},
};
MODULE_DEVICE_TABLE(of, rtk_usb_of_match);


static struct platform_driver rtk_usb_platform_driver = {
    .probe      = rtk_usb_platform_probe,
    .remove     = rtk_usb_platform_remove,
    .driver     = {
        .name = "usb-platform",
        .pm = &rtk_usb_platform_pm_ops,
        .of_match_table = rtk_usb_of_match,
    }
};


static int __init rtk_usb_platform_init(void)
{
    int ret = 0;
    ret = rtk_usb_init();
    if(ret != 0) {
        	RTK_USB_ERR("Realtek USB Core driver init failed\n");
		goto FAL_INIT_USB_CORE;
    }	
	ret = platform_driver_register(&rtk_usb_platform_driver);
    if(ret != 0) {
		RTK_USB_ERR("Realtek USB Platorm driver init failed\n");
		goto FAIL_INIT_USB_PLATFORM;
    }
    return ret;
FAIL_INIT_USB_PLATFORM:
    rtk_usb_exit();
FAL_INIT_USB_CORE:
    return ret;
}
module_init(rtk_usb_platform_init);


static void __exit rtk_usb_platform_cleanup(void)
{
    platform_driver_unregister(&rtk_usb_platform_driver);
    rtk_usb_exit();
}
module_exit(rtk_usb_platform_cleanup);


MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("Jason Chiu");
MODULE_LICENSE("GPL");
