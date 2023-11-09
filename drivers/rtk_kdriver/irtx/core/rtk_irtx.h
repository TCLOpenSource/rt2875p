/*Copyright (C) 2007-2013 Realtek Semiconductor Corporation.*/
#ifndef __RTK_IRTX_H__
#define __RTK_IRTX_H__

#include <linux/device.h>
#include "rtk_irtx_debug.h"
#include "rtk_irtx_cmb.h"

extern struct bus_type irtx_bus_type;

typedef struct
{
    unsigned long id;
    char *name;
    struct device dev;
} irtx_device;

#define to_irtx_device(x)  container_of(x, irtx_device, dev)

static inline void *irtx_get_drvdata(irtx_device *device)
{
    return dev_get_drvdata(&device->dev);
}

static inline void irtx_set_drvdata(irtx_device *device, void *data)
{
    struct device *dev = &device->dev;
    dev_set_drvdata(dev, data);
}

extern int register_irtx_device(irtx_device *device);
extern void unregister_irtx_device(irtx_device *device);

typedef struct
{
    char *name;
    struct device_driver drv;
    int (*probe) (irtx_device *dev);
    void (*remove) (irtx_device *dev);
    int (*xmit) (irtx_device *dev, IRTX_CMD_BUFF *cmg, unsigned long flags);
    int (*enable) (irtx_device *dev, unsigned int on_off);
    int (*suspend) (irtx_device *dev);
    int (*resume) (irtx_device *dev);
    int (*set_global_interrupt)(irtx_device *dev, unsigned int onoff);
    int (*set_default_level)(irtx_device *dev, unsigned int level);

} irtx_driver;


#define to_irtx_driver(x)  container_of(x, irtx_driver, drv)

extern int register_irtx_driver(irtx_driver *driver);
extern void unregister_irtx_driver(irtx_driver *driver);


#endif /*__RTK_CEC_CORE_H__*/
