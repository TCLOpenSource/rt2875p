#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of_platform.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/freezer.h>
#include <mach/pcbMgr.h>
#include <linux/uaccess.h>
#include <rtk_kdriver/io.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/usb2_hub_reg.h>
#include <linux/suspend.h>
#include <linux/notifier.h>
#include <mach/rtk_platform.h>
#include "rtk_usb_hub.h"
#include "rtk_usb_hub_wrapper.h"
#include "rtk_usb_hub_fw.h"
#include "rtk_usb_hub_smbus.h"
#include "rtk_usb_hub_debug.h"
#include "rtk_usb_hub_camera.h"
#define ENABLE_SMBUF_POLL_STATUS
#define DRIVER_DESC "RTK USB hub generic platform driver"
#ifdef ENABLE_SMBUF_POLL_STATUS
struct task_struct *usb_hub_task = NULL;
static bool is_in_suspend_flow = false;
#endif

static bool download_usb_hub_fw(unsigned long dst_addr, unsigned int size)
{
    unsigned char *ubuf;
    unsigned int i;
    unsigned int count;

    rtd_outl(USB2_HUB_HUB_RAM_I_DATA_SEL_reg, 0x1);    // Enable SW control flow

    ubuf = (unsigned char *)dst_addr;

    // Store USB Hub to IMEM
    for (i = 0; i < size; i++) {
	rtd_outl(USB2_HUB_HUB_RAM_I_WR_ADDR_reg, i);
        rtd_outl(USB2_HUB_HUB_RAM_I_WR_DATA_reg, ubuf[i]);
    }

    RTK_USB_HUB_WARNING("EMCU compare data\n");

   rtd_outl(USB2_HUB_HUB_RAM_I_RD_ADDR_reg, 0x1);
   count = 0;
   while(count++ < 10000) {
	if(rtd_inl(USB2_HUB_HUB_RAM_I_RD_ADDR_reg) == 0x1)
		break;
   }

   rtd_outl(USB2_HUB_HUB_RAM_I_RD_ADDR_reg, 0x0);
   count = 0;
   while(count++ < 10000) {
	if(rtd_inl(USB2_HUB_HUB_RAM_I_RD_ADDR_reg) == 0x0)
		break;
   }

    // Clear IMEM address
    for (i = 0; i < size; i++) {
	unsigned char tmp = 0;
	tmp = rtd_inl(USB2_HUB_HUB_RAM_I_RD_DATA_reg);
        if(tmp != ubuf[i]) {
            RTK_USB_HUB_WARNING("[ERROR] Compare fail for USB hub bin:%u %02x %02x\n", i, tmp , ubuf[i]);
            goto ERR_GET_EMCU_FW_FAILED;
        }
	count = 0;
	while(count++ < 10000) {
		if(rtd_inl(USB2_HUB_HUB_RAM_I_RD_ADDR_reg) == (i + 1))
			break;

	}
    }
    rtd_outl(USB2_HUB_HUB_RAM_I_DATA_SEL_reg, 0x0);    // Disable SW control flow
    return true;

ERR_GET_EMCU_FW_FAILED:
    return false;
}


#if IS_ENABLED(CONFIG_RTK_KDRV_USB_OTG_PLATFORM)
extern bool rtk_otg_is_enable(void);
extern bool rtk_otg_is_via_hub_port(void);
#endif
void rtk_usb_hub_set_default_dphy_mux(bool to_tv)
{
	if(to_tv) {
		rtk_usb_hub_dsp_enable(false);
#if IS_ENABLED(CONFIG_RTK_KDRV_USB_OTG_PLATFORM)
		if(rtk_otg_is_enable())
			rtk_usb_hub_usp_enable(true);
		else
			rtk_usb_hub_usp_enable(false);
#endif
	} else {
		rtk_usb_hub_dsp_enable(true);
		rtk_usb_hub_usp_enable(true);
	}
}

bool usb_hub_is_in_suspend(void)
{
#ifdef ENABLE_SMBUF_POLL_STATUS
	return is_in_suspend_flow;
#else
	return false;
#endif
}

#ifdef ENABLE_SMBUF_POLL_STATUS
int _usb_hub_status_process(void *arg)
{
    while(!kthread_should_stop()) {
	 rtk_usb_hub_check_port();
        msleep(500);
    }
    return 0;
}

static int usb_hub_pm_event (struct notifier_block *this, unsigned long event, void *ptr)
{
	switch (event) {
	case PM_SUSPEND_PREPARE:
		is_in_suspend_flow = true;
		break;
	case PM_POST_SUSPEND:
		is_in_suspend_flow = false;
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block usb_hub_notifier = {
  .notifier_call = usb_hub_pm_event,
};
#endif

static bool rtk_usb_hub_init(void)
{
    bool ret = false;
    unsigned long usb_hub_addr;
    unsigned int usb_hub_size;

    // 0. Set U2 hub wrapper reset
    rtk_usb_hub_wrapper_clock_on();

    rtd_outl(USB2_HUB_HUB_CTRL0_reg, 0x01b4310);//div_1m_cnt div_400k_cnt  ups_vbus_det reg_force_pll on 0 reg_sw_clk_sel reg_sw_mode
    udelay(100);

    // 1. Load 8051 bin
    usb_hub_addr = (unsigned long)get_usb2_hub_bin(&usb_hub_size);
    if(!download_usb_hub_fw(usb_hub_addr, usb_hub_size)) {
        goto FAIL_GET_USB_HUB;
    }

    // 2. Load APHY
    rtk_usb_hub_load_phy_settings();

    // 3. Set CLK
    rtk_usb_hub_clk_on();

    // 4. Polling SMBUS (create thread)
#ifdef ENABLE_SMBUF_POLL_STATUS

    register_pm_notifier(&usb_hub_notifier);
    
    usb_hub_task = kthread_create(_usb_hub_status_process, NULL, "_usb_hub_status_process");
    if(IS_ERR(usb_hub_task)) {
        RTK_USB_HUB_WARNING("Create USB Hub Thread fail \n");
	 usb_hub_task = NULL;
    }
    if(usb_hub_task)
    	wake_up_process(usb_hub_task);
#endif
    // 5. Initial USB hub
    rtk_usb_hub_set_default_dphy_mux(true);

    ret = true;

FAIL_GET_USB_HUB:
    return ret;
}

static ssize_t get_dphy_mux(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t set_dphy_mux(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        int val = 0;

        if (sscanf(buf, "%u", &val) == 1)
                rtk_usb_hub_set_default_dphy_mux(!!val);
        return count;
}

static DEVICE_ATTR(dphy_mux, S_IRUGO | S_IWUSR,
                   get_dphy_mux, set_dphy_mux);

const struct attribute *usb_hub_platform_dev_attrs[] =
{
        &dev_attr_dphy_mux.attr,
	 NULL,
};

static const struct attribute_group usb_hub_platform_dev_attr_grp =
{
        .attrs = (struct attribute **)usb_hub_platform_dev_attrs,
};

static int __init rtk_usb_hub_platform_probe(struct platform_device *pdev)
{
    if(rtk_usb_hub_init() != true)
		return -1;

  if (sysfs_create_group(&pdev->dev.kobj, &usb_hub_platform_dev_attr_grp)) {
       RTK_USB_HUB_WARNING("Create self-defined sysfs attributes fail \n");
   }

    return 0;
}

static int rtk_usb_hub_platform_remove(struct platform_device *pdev)
{
    sysfs_remove_group(&pdev->dev.kobj, &usb_hub_platform_dev_attr_grp);
#ifdef ENABLE_SMBUF_POLL_STATUS
    if(usb_hub_task) {
	    kthread_stop(usb_hub_task);
	    usb_hub_task = NULL;
    }
    unregister_pm_notifier(&usb_hub_notifier);
#endif
    
    rtk_usb_hub_set_default_dphy_mux(true);
    rtk_usb_hub_wrapper_clock_off();
    return 0;
}

static int rtk_usb_hub_platform_resume(struct device *dev)
{
    if(rtk_usb_hub_init() == true)
		return 0;
    return -1;
}


static int rtk_usb_hub_platform_suspend(struct device *dev)
{
#ifdef ENABLE_SMBUF_POLL_STATUS
    if(usb_hub_task) {
	    kthread_stop(usb_hub_task);
	    usb_hub_task = NULL;
    }
#endif
    return 0;
}

static RAW_NOTIFIER_HEAD(g_typec_source_switch_chain);
int register_typec_source_switch_notifier(struct notifier_block *nb)
{
        return raw_notifier_chain_register(&g_typec_source_switch_chain, nb);
}

int unregister_typec_source_switch_notifier(struct notifier_block *nb)
{
        return raw_notifier_chain_unregister(&g_typec_source_switch_chain, nb);
}


int typec_source_switch_notifier_call_chain(unsigned long val, void *v)
{
        return raw_notifier_call_chain(&g_typec_source_switch_chain, val, v);
}
EXPORT_SYMBOL(typec_source_switch_notifier_call_chain);


static const struct dev_pm_ops rtk_usb_hub_platform_pm_ops = {
    .resume = rtk_usb_hub_platform_resume,
    .suspend = rtk_usb_hub_platform_suspend,
};


static const struct of_device_id rtk_usb_hub_of_match[] = {
    { .compatible = "rtk,usb-hub-platform", },
    {},
};
MODULE_DEVICE_TABLE(of, rtk_usb_hub_of_match);


static struct platform_driver rtk_usb_hub_platform_driver = {
    .probe      = rtk_usb_hub_platform_probe,
    .remove     = rtk_usb_hub_platform_remove,
    .driver     = {
        .name = "rtk-usb-hub-platform",
        .pm = &rtk_usb_hub_platform_pm_ops,
        .of_match_table = rtk_usb_hub_of_match,
    }
};


static int __init rtk_usb_hub_platform_init(void)
{
    int ret = 0;
#if IS_ENABLED(CONFIG_RTK_KDRV_USB_OTG_PLATFORM)
    if(!rtk_otg_is_enable()) {
        RTK_USB_HUB_INFO("OTG not enable, so disable usb hub\n");
        return 0;
    }
    if(get_product_type() != PRODUCT_TYPE_DIAS) {
	    if(!rtk_otg_is_via_hub_port()) {
			RTK_USB_HUB_INFO("OTG not via hub, so disable usb hub\n");
			return 0;
	    }
    }
#endif
    ret = rtk_usb_hub_register_notifier();
    if(ret != 0) {
	  RTK_USB_HUB_ERR("rtk_usb_hub_register_notifier failed\n");
	  return ret;
    }
    ret = platform_driver_register(&rtk_usb_hub_platform_driver);
    if(ret != 0) {
        rtk_usb_hub_unregister_notifier();
        RTK_USB_HUB_ERR("Realtek USB HUB Platorm driver init failed\n");
    }
    return ret;
}
#ifdef CONFIG_RTK_KDRV_USB_HUB
rootfs_initcall(rtk_usb_hub_platform_init);
#endif
#ifdef CONFIG_RTK_KDRV_USB_HUB_MODULE
module_init(rtk_usb_hub_platform_init);
#endif


static void __exit rtk_usb_hub_platform_cleanup(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_USB_OTG_PLATFORM)
    if(!rtk_otg_is_enable()) {
        return ;
    }
    if(get_product_type() != PRODUCT_TYPE_DIAS) {
	    if(!rtk_otg_is_via_hub_port())
			return;
    }
#endif
    rtk_usb_hub_unregister_notifier();
    platform_driver_unregister(&rtk_usb_hub_platform_driver);
}
module_exit(rtk_usb_hub_platform_cleanup);


MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
