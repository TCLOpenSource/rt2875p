#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include "rtk_irtx.h"

#ifdef CONFIG_RTK_KDRV_IRTX_CHARDEV
#include "rtk_irtx_dev.h"
#endif

#ifdef CONFIG_PM
static int irtx_bus_pm_suspend(struct device *dev)
{
    irtx_device *p_dev = to_irtx_device(dev);
    irtx_driver *p_drv = to_irtx_driver(dev->driver);
    return (p_drv->suspend) ? p_drv->suspend(p_dev) : 0;
}


static int irtx_bus_pm_resume(struct device *dev)
{
    irtx_device *p_dev = to_irtx_device(dev);
    irtx_driver *p_drv = to_irtx_driver(dev->driver);
    return (p_drv->resume) ? p_drv->resume(p_dev) : 0;
}

static const struct dev_pm_ops irtx_bus_pm_ops =
{
    .suspend = irtx_bus_pm_suspend,
    .resume = irtx_bus_pm_resume,
#ifdef CONFIG_HIBERNATION
    .freeze = irtx_bus_pm_suspend,
    .thaw = irtx_bus_pm_resume,
    .poweroff = irtx_bus_pm_suspend,
    .restore = irtx_bus_pm_resume,
#endif
};

#endif /* CONFIG_PM */

static int irtx_bus_match(struct device *dev, struct device_driver *drv)
{
    /*irtx bus have no idea to match device & driver, return 1 to pass all*/
    return 1;
}

struct bus_type irtx_bus_type =
{
    .name = "irtx",
    .match = irtx_bus_match,
#ifdef CONFIG_PM
    .pm = &irtx_bus_pm_ops,
#endif
};


static void irtx_device_release(struct device *dev)
{
    irtx_device *p_dev = to_irtx_device(dev);

    irtx_warn("irtx dev %s released\n", p_dev->name);
}


static int irtx_device_count = 0;

int register_irtx_device(irtx_device *device)
{
    struct device *dev = &device->dev;

    irtx_warn("register irtx device '%s' (%p) to irtx%d\n", device->name, dev,
              irtx_device_count);

    dev_set_name(dev, "irtx%d", irtx_device_count++);

    dev->bus = &irtx_bus_type;

    dev->release = irtx_device_release;

    return device_register(dev);
}


void unregister_irtx_device(irtx_device *device)
{
    device_unregister(&device->dev);
}


int irtx_drv_probe(struct device *dev)
{
    irtx_device *p_dev = to_irtx_device(dev);
    irtx_driver *p_drv = to_irtx_driver(dev->driver);
    irtx_warn("probe : irtx_dev '%s' (%p), irtx_drv '%s' (%p)\n", p_dev->name,
              dev, p_drv->name, dev->driver);

    if (!p_drv->probe)
        return -ENODEV;

    if (p_drv->probe(p_dev) == 0)
    {
#ifdef CONFIG_RTK_KDRV_IRTX_CHARDEV
        create_irtx_dev_node(p_dev);
#endif
        return 0;
    }

    return -ENODEV;
}


int irtx_drv_remove(struct device *dev)
{
    irtx_device *p_dev = to_irtx_device(dev);
    irtx_driver *p_drv = to_irtx_driver(dev->driver);

    irtx_warn("remove irtx_dev '%s'\n", p_dev->name);

    if (p_drv->remove)
        p_drv->remove(p_dev);

#ifdef CONFIG_RTK_KDRV_IRTX_CHARDEV
    remove_irtx_dev_node(p_dev);
#endif

    return 0;
}


void irtx_drv_shutdown(struct device *dev)
{
    irtx_device *p_dev = to_irtx_device(dev);
    irtx_driver *p_drv = to_irtx_driver(dev->driver);

    irtx_warn("shutdown irtx_dev '%s'\n", p_dev->name);
    if (p_drv->set_global_interrupt)
        p_drv->set_global_interrupt(p_dev, 0);
}


int irtx_drv_suspend(struct device *dev, pm_message_t state)
{
    irtx_device *p_dev = to_irtx_device(dev);
    irtx_driver *p_drv = to_irtx_driver(dev->driver);

    irtx_warn("suspend irtx_dev '%s'\n", p_dev->name);
    return (p_drv->suspend) ? p_drv->suspend(p_dev) : 0;
}


int irtx_drv_resume(struct device *dev)
{
    irtx_device *p_dev = to_irtx_device(dev);
    irtx_driver *p_drv = to_irtx_driver(dev->driver);

    irtx_warn("resume irtx_dev '%s'\n", p_dev->name);

    return (p_drv->resume) ? p_drv->resume(p_dev) : 0;
}


int register_irtx_driver(irtx_driver *driver)
{
    struct device_driver *drv = &driver->drv;

    drv->name = driver->name;
    drv->bus = &irtx_bus_type;
    drv->probe = irtx_drv_probe;
    drv->remove = irtx_drv_remove;
    drv->shutdown = irtx_drv_shutdown;
    drv->suspend = irtx_drv_suspend;
    drv->resume = irtx_drv_resume;

    irtx_warn("register irtx driver '%s' (%p)\n", drv->name, drv);

    return driver_register(drv);
}


void unregister_irtx_driver(irtx_driver *driver)
{
    struct device_driver *drv = &driver->drv;
    irtx_warn("unregister irtx driver '%s' (%p)\n", drv->name, &driver->drv);
    driver_unregister(&driver->drv);
}


static int __init irtx_core_init(void)
{
    irtx_warn("%s, register irtx_bus %p\n", __FUNCTION__, &irtx_bus_type);
    return bus_register(&irtx_bus_type);
}


static void __exit irtx_core_exit(void)
{
    bus_unregister(&irtx_bus_type);
}

module_init(irtx_core_init);
module_exit(irtx_core_exit);

EXPORT_SYMBOL(register_irtx_device);
EXPORT_SYMBOL(unregister_irtx_device);
EXPORT_SYMBOL(register_irtx_driver);
EXPORT_SYMBOL(unregister_irtx_driver);
