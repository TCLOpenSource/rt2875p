#ifndef __RTK_DDC_OSAL_H__
#define __RTK_DDC_OSAL_H__

#ifdef BUILD_QUICK_SHOW

#include <malloc.h>
#include <timer.h>
#include <rtd_log/rtd_module_log.h>
#include <vsprintf.h>
#include <string.h>
#include <rtk_kdriver/io.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))
#endif

#define udelay(x)               usleep(x)
#define mdelay(x)               udelay(x*1000)

#define EFAULT                  34
#define EINVAL                  33
#define spinlock_t              int
#define spin_lock(x)
#define spin_unlock(x)
#define EXPORT_SYMBOL(x)

struct class
{
    unsigned int val;
};

#else
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <rtk_kdriver/io.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

#endif


#endif
