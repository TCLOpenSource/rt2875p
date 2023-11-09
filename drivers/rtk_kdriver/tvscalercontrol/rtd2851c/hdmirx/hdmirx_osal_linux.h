/*=============================================================
 * File:    hdmirx_osal.h
 *
 * Desc:    HDMIRX OS Dependet API
 *
 * AUTHOR:  kevin_wang@realtek.com
 *
 * Vresion: 0.0.1
 *
 *------------------------------------------------------------
 * Copyright (c) Realtek Semiconductor Corporation, 2021
 *
 * All rights reserved.
 *
 *============================================================*/

#ifndef __HDMIRX_OSAL_LINUX_H__
#define __HDMIRX_OSAL_LINUX_H__
#include <linux/delay.h>

#include <linux/freezer.h>

#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/kthread.h>

#include <linux/module.h>

#include <linux/printk.h>

#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/of_device.h>
#ifdef __cplusplus
extern "C" {
#endif

#define hdmirx_osal_malloc(size)           kmalloc(size, GFP_KERNEL)
#define hdmirx_osal_mfree(ptr)             kfree(ptr)
#define hdmirx_osal_msleep(ms)             msleep(ms)

#define hdmirx_err(fmt, args...)           rtd_pr_hdmi_err(fmt, ## args)
#define hdmirx_warn(fmt, args...)          rtd_pr_hdmi_warn(fmt, ## args)
#define hdmirx_info(fmt, args...)          rtd_pr_hdmi_info(fmt, ## args)
#define hdmirx_dbg(fmt, args...)           rtd_pr_hdmi_debug(fmt, ## args)

#ifdef __cplusplus
}
#endif



#endif // __DPRX_OSAL_LINUX_H__
