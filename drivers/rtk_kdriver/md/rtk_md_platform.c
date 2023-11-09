#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include "rtk_md_debug.h"
#include <rtk_kdriver/md/rtk_md.h>
#include <rtk_kdriver/rtk_crt.h>
#include <linux/pm_runtime.h>
#include <linux/suspend.h>
#include "rtk_md_hw.h"

#ifdef CONFIG_RTK_LOW_POWER_MODE
#include <rtk_kdriver/rtk-kdrv-common.h>
#endif

#define GET_RUNTIME_CNT(pdev) (atomic_read(&pdev->power.usage_count))
#define GET_RUNTIME_AUTO(pdev) (pdev->power.runtime_auto)
#define GET_RUNTIME_STATUS(pdev) \
	(pdev->power.runtime_status == RPM_ACTIVE)?"active"\
	:(pdev->power.runtime_status == RPM_RESUMING)?"resuming"\
	:(pdev->power.runtime_status == RPM_SUSPENDED)?"suspended":"suspending"

#define MD_PM_LOG(LEVEL, GROUP, DEV, TYPE, VENDOR, MESSAGE) \
    do { \
        PDEIFO("LowPower::%s::%s::%s::%d::%s-%s::%s::ok::%s::%s::(runtime_status: %s)::(runtime_auto: %d)\n", \
                LEVEL, GROUP, "/dev/md0", GET_RUNTIME_CNT(DEV), \
                (current->group_leader?current->group_leader->comm:"-"), current->comm, \
                TYPE, VENDOR, MESSAGE, GET_RUNTIME_STATUS(DEV), GET_RUNTIME_AUTO(DEV)); \
    } while(0)

extern struct device *md_device;
extern struct md_dev *md_drv_dev;

#define MD_PLATORM_DEV_NAME "rtk-md"

static int rtk_md_pm_suspend_noirq(struct device *dev)
{
    struct platform_device *pdev = container_of(dev, struct platform_device, dev);
    MD_PM_LOG("info", "rtk-md", (&pdev->dev), "stop", "realtek", "device STR PM disable");
    pm_runtime_disable(&pdev->dev);
#ifdef CONFIG_RTK_MD_USE_SW
    down(&md_drv_dev->sem);
    rtk_md_suspend_noirq();
    up(&md_drv_dev->sem);
#else
    rtk_md_suspend_noirq();
#endif
    return 0;
}

static int rtk_md_pm_resume_noirq(struct device *dev)
{
    struct platform_device *pdev = container_of(dev, struct platform_device, dev);
    rtk_md_resume_noirq();
    MD_PM_LOG("info", "rtk-md", (&pdev->dev), "start", "realtek", "device STR PM enable");
    pm_runtime_enable(&pdev->dev);
    return 0;
}

static int rtk_md_pm_resume_early(struct device *dev)
{
    rtk_md_resume_early();
    return 0;
}


static int rtk_md_pm_resume(struct device *dev)
{
    PDEIFO("[MD]%s\n", __func__);
    return 0;
}

static int rtk_md_pm_poweroff(struct device *dev)
{
    rtk_md_poweroff();
    return 0;
}

static int rtk_md_pm_restore(struct device *dev)
{
    rtk_md_restore();
    return 0;
}

static int rtk_md_pm_freeze_noirq(struct device *dev)
{
    rtk_md_freeze_noirq();
    return 0;
}

int rtk_md_pm_runtime_suspend(struct device *dev)
{
    struct platform_device *pdev = container_of(dev, struct platform_device, dev);
    PDEIFO("[MD] @(%s:%d)\n", __func__, __LINE__);
    MD_PM_LOG("info", "rtk-md", (&pdev->dev), "suspend", "realtek", "system pm suspend");
    if (down_interruptible(&md_drv_dev->sem))
        return -EBUSY;
    if (rtk_md_is_clk_on()) {
#ifndef CONFIG_RTK_MD_USE_SW		
        rtk_md_store_kv_reg();
#endif
        CRT_CLK_OnOff(MD, CLK_OFF, NULL);
    } 
    up(&md_drv_dev->sem);
    return 0;
}

int rtk_md_pm_runtime_resume(struct device *dev)
{
    struct platform_device *pdev = container_of(dev, struct platform_device, dev);
    PDEIFO("[MD] @(%s:%d)\n", __func__, __LINE__);
    MD_PM_LOG("info", "rtk-md", (&pdev->dev), "resume", "realtek", "system pm resume");
    CRT_CLK_OnOff(MD, CLK_ON, NULL);
#ifndef CONFIG_RTK_MD_USE_SW		
    rtk_md_init_kv_reg();
#endif
    return 0;
}

static const struct dev_pm_ops md_pm_ops = {
    .suspend_noirq      = rtk_md_pm_suspend_noirq,
    .resume_noirq       = rtk_md_pm_resume_noirq,
    .resume_early       = rtk_md_pm_resume_early,
    .resume             = rtk_md_pm_resume,
    .poweroff_late      = rtk_md_pm_poweroff,
    .restore_early      = rtk_md_pm_restore,
    .freeze_noirq       = rtk_md_pm_freeze_noirq,
    .runtime_suspend    = rtk_md_pm_runtime_suspend,
    .runtime_resume     = rtk_md_pm_runtime_resume,
};

#ifdef CONFIG_OF
static const struct of_device_id md_of_match[] = {
    { .compatible = "realtek,rtk-md" },
    { },
};
MODULE_DEVICE_TABLE(of, md_of_match);
#endif



static int __init md_probe(struct platform_device *pdev)
{
    int irq;
    int ret;

    irq = platform_get_irq(pdev, 0);
    if (irq < 0) {
        PDEERR("%s: fail to get irq. ret=%d\n", __func__, irq);
        return irq;
    }

    if (unlikely(!md_device)) {
        PDEERR("%s: md_deivce is NULL!\n", __func__);
        return -ENODEV;
    }

    ret = rtk_md_init(md_device, irq);
    if(ret != 0)
		return ret;
    pm_runtime_forbid(&pdev->dev);
    pm_runtime_set_active(&pdev->dev);
    pm_runtime_enable(&pdev->dev);//enable runtime PM
#ifdef CONFIG_RTK_LOW_POWER_MODE
	rtk_lpm_add_device(&pdev->dev);
#endif

    return 0;
}


static int md_remove(struct platform_device *pdev)
{
    rtk_md_uninit(platform_get_irq(pdev, 0));
    return 0;
}


static struct platform_driver md_device_driver = {
    .remove = md_remove,
    .driver = {
        .name           = MD_PLATORM_DEV_NAME,
        .bus            = &platform_bus_type,
        .pm             = &md_pm_ops,
        .of_match_table = of_match_ptr(md_of_match),
    },
};


__init int md_platform_init(void)
{
    return platform_driver_probe(&md_device_driver, md_probe);
}


__exit void md_platform_uninit(void)
{
    platform_driver_unregister(&md_device_driver);
}
