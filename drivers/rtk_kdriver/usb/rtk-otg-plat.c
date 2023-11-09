/**
 * created by Realtek liangliang_song@apowertec.com
 *
 *
 * Author: liangliang_song<liangliang_song@apowertec.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <rbus/sys_reg_reg.h>
#include <rtk_kdriver/io.h>
#include <rbus/usb2_otg_reg.h>
#include <rtk_kdriver/rtk-kdrv-common.h>
#include "rtk-otg-plat.h"
#include "rtk-otg-plat-wrapper.h"

static bool usbotg = 0;
#ifdef CONFIG_RTK_KDRV_USB_OTG_PLATFORM
static int rtk_otg_switch_handler(char *buf)
{
        char *p = NULL;
        if ((p = strstr(buf, "on")) != NULL || (p = strstr(buf, "1")) != NULL)
                usbotg = 1;
        return 0;
}
__setup("usbotg", rtk_otg_switch_handler);
#else

static int parse_otg_switch_handler(void)
{
    char strings[16] = {0};

    if(rtk_parse_commandline_equal("usbotg", strings,sizeof(strings)) == 0)
    {
        rtk_otg_plat_err("Error : can't get usbotg from bootargs\n");
        return -1;
    }

    if(strcmp(strings, "on") == 0 || strcmp(strings, "1") == 0) {
        usbotg = 1;
    }
    else {
        usbotg = 0;
    }

    return 0;
}

#endif

bool rtk_otg_is_enable(void)
{
        return usbotg;
}
EXPORT_SYMBOL(rtk_otg_is_enable);

/*Otg can exported via hub or via typea directly in some platform, if so, default via hub*/
static bool g_otg_via_hub_port = true;

#ifdef CONFIG_RTK_KDRV_USB_OTG_PLATFORM
static int rtk_otg_port_handler(char *buf)
{
        char *p = NULL;
        if ((p = strstr(buf, "TYPEA")) != NULL)
                g_otg_via_hub_port = false;
	 else
                g_otg_via_hub_port = true;
        return 0;
}
__setup("usbotg_port", rtk_otg_port_handler);
#else

static int parse_otg_port_handler(void)
{
    char strings[16] = {0};

    if(rtk_parse_commandline_equal("usbotg_port", strings,sizeof(strings)) == 0)
    {
        rtk_otg_plat_err("Error : can't get usbotg_port from bootargs\n");
        return -1;
    }

    if(strcmp(strings, "TYPEA") == 0) {
        g_otg_via_hub_port = false;
    }
    else {
        g_otg_via_hub_port = true;
    }

    return 0;
}

#endif

bool rtk_otg_is_via_hub_port(void)
{
        return g_otg_via_hub_port;
}
EXPORT_SYMBOL(rtk_otg_is_via_hub_port);


static int rtk_otg_plat_remove_child(struct device *dev, void *unused)
{
        struct platform_device *pdev = to_platform_device(dev);

        platform_device_unregister(pdev);

        return 0;
}

static int rtk_otg_plat_get_id_from_dts(struct platform_device *pdev, 
								unsigned int *p_id)
{
        struct device_node *p_node;

        if(!pdev || !p_id)
                return -EINVAL;

        p_node =  pdev->dev.of_node;
        if (!p_node)
                return -ENODEV;

        if(of_property_read_u32_index(p_node, "id", 0, p_id) != 0)
                return -EIO;
        return 0;
}


static ssize_t get_otg_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);	 
	return snprintf(buf, PAGE_SIZE, "otg mode %s\n", rtk_otg_plat_get_otg_mode(pdev) ? "enable" : "disable");
}

static ssize_t set_otg_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int val;
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	if (sscanf(buf, "%u", &val) == 1) {
		if(val)
		  	rtk_otg_plat_set_otg_mode(pdev, true);
		else
			rtk_otg_plat_set_otg_mode(pdev, false);
	}
	return count;
}

static DEVICE_ATTR(otg_mode, S_IRUGO | S_IWUSR,
                   get_otg_mode, set_otg_mode);

const struct attribute *rtk_otg_plat_dev_attrs[] =
{
        &dev_attr_otg_mode.attr,
	 NULL,
};

static const struct attribute_group rtk_otg_plat_dev_attr_grp =
{
        .attrs = (struct attribute **)rtk_otg_plat_dev_attrs,
};

static int rtk_otg_plat_probe(struct platform_device *pdev)
{
	struct device           *dev = &pdev->dev;
	struct device_node      *node = dev->of_node;
	unsigned int id = 0;
	int    ret;
	struct resource *res_mem = NULL;
	struct RTK_OTG_PRIVATE_DATA *private_data = NULL;

	if(rtk_otg_plat_get_id_from_dts(pdev, &id) != 0)
		id = 0;
	
	private_data = (struct RTK_OTG_PRIVATE_DATA *)kzalloc(
			sizeof(struct RTK_OTG_PRIVATE_DATA), GFP_KERNEL);
	if(!private_data)
		return -ENOMEM;

	private_data->id = id;
	private_data->pdev = pdev;
	res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(res_mem)
		private_data->io_base = res_mem->start + 0xA0000000;
	platform_set_drvdata(pdev, private_data);	

	ret = rtk_otg_initial(pdev);
	if (ret) {
		rtk_otg_plat_err("failed to rtk_otg_initial\n");
		kfree(private_data);
		return ret;
	}
	
	if (sysfs_create_group(&pdev->dev.kobj, &rtk_otg_plat_dev_attr_grp)) {
		rtk_otg_plat_err("Create self-defined sysfs attributes fail \n");
		return -ENOMEM;
	}	

	if (node) {
		ret = of_platform_populate(node, NULL, NULL, dev);
		if (ret) {
			rtk_otg_plat_err("failed to add drd core\n");
			sysfs_remove_group(&pdev->dev.kobj, &rtk_otg_plat_dev_attr_grp);
			kfree(private_data);
			return ret;
		}
	} else {
		rtk_otg_plat_err("no device node, failed to add drd core\n");
		sysfs_remove_group(&pdev->dev.kobj, &rtk_otg_plat_dev_attr_grp);
		kfree(private_data);
		ret = -ENODEV;
		return ret;
	}
	rtk_otg_plat_set_local_interrupts(pdev, true);
	rtk_otg_plat_set_global_interrupts(pdev, true);
	rtk_otg_plat_info("otg [%u] probe, io_base:%x\n", private_data->id, private_data->io_base);
	return 0;
}

static int rtk_otg_plat_remove(struct platform_device *pdev)
{
	struct RTK_OTG_PRIVATE_DATA *private_data = NULL;	
	private_data = (struct RTK_OTG_PRIVATE_DATA *)platform_get_drvdata(pdev);
	sysfs_remove_group(&pdev->dev.kobj, &rtk_otg_plat_dev_attr_grp);
	device_for_each_child(&pdev->dev, NULL, rtk_otg_plat_remove_child);

	rtk_otg_plat_set_local_interrupts(pdev, false);
	rtk_otg_plat_set_global_interrupts(pdev, false);
	kfree(private_data);
	return 0;
}



static const struct of_device_id rtk_otg_plat_match[] =
{
        {
                .compatible = "rtk_dwc2"
        }, {
        }
};
MODULE_DEVICE_TABLE(of, rtk_otg_plat_match);

#ifdef CONFIG_PM_SLEEP
static int rtk_otg_plat_suspend(struct device *dev)
{
        struct platform_device *pdev = container_of(dev, struct platform_device, dev);

        rtk_otg_plat_set_local_interrupts(pdev, false);
        //rtk_otg_plat_set_global_interrupts(pdev, false);

        return 0;
}

static int rtk_otg_plat_resume(struct device *dev)
{
        struct platform_device *pdev = container_of(dev, struct platform_device, dev);
        int    ret;

        ret = rtk_otg_initial(pdev);
        if (ret)
                return ret;
        rtk_otg_plat_set_local_interrupts(pdev, true);
        rtk_otg_plat_set_global_interrupts(pdev, true);

        return 0;
}

static const struct dev_pm_ops rtk_otg_plat_dev_pm_ops =
{
        SET_SYSTEM_SLEEP_PM_OPS(rtk_otg_plat_suspend, rtk_otg_plat_resume)
};

#define DEV_PM_OPS      (&rtk_otg_plat_dev_pm_ops)
#else
#define DEV_PM_OPS      NULL
#endif /* CONFIG_PM_SLEEP */

static struct platform_driver rtk_otg_plat_driver =
{
        .probe          = rtk_otg_plat_probe,
        .remove         = rtk_otg_plat_remove,
        .driver         = {
                .name   = "rtk_otg_plat",
                .of_match_table = rtk_otg_plat_match,
                .pm     = DEV_PM_OPS,
        },
};
MODULE_ALIAS("platform:rtk_otg_plat");

static int __init rtk_otg_plat_init(void)
{

#ifdef CONFIG_RTK_KDRV_USB_OTG_PLATFORM_MODULE
        parse_otg_switch_handler();
        parse_otg_port_handler();
#endif
        if (!usbotg)
        {
                rtk_otg_plat_err("OTG is not enable. Use command: \"usbotg on\" in bootcode to enable it! \n");
                return 0;
        }
        return platform_driver_register(&rtk_otg_plat_driver);
}

late_initcall(rtk_otg_plat_init);


static void __exit rtk_otg_plat_exit(void)
{
        if (usbotg)
                platform_driver_unregister(&rtk_otg_plat_driver);
}
module_exit(rtk_otg_plat_exit);

MODULE_DESCRIPTION("Realtek DWC2 OTG Platform Driver");
MODULE_LICENSE("GPL");

