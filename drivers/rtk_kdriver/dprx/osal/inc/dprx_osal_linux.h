/*=============================================================
 * File:    dprx_osal.h
 *
 * Desc:    DPRX OS Dependet API
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

#ifndef __DPRX_OSAL_LINUX_H__
#define __DPRX_OSAL_LINUX_H__

//#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/freezer.h>
#include <linux/delay.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define dprx_osal_malloc(size)           kmalloc(size, GFP_KERNEL)
#define dprx_osal_mfree(ptr)             kfree(ptr)
#define dprx_osal_msleep(ms)             msleep(ms)

#ifdef CONFIG_DPRX_STATUS_MONITOR
#include <mach/rtk_timer.h>
#define dprx_odal_get_system_time_ms()   rtk_timer_misc_90k_ms()
#define dprx_osal_usleep(us)             udelay(us)
#else
#define dprx_odal_get_system_time_ms()   (0)
#define dprx_osal_usleep(us)
#endif

//-----------------------------------------------------
// Spin lock
//-----------------------------------------------------
#define dprx_osal_spinlock_t                      spinlock_t

#define DEFINE_DPRX_OSAL_SPINLOCK(x)              DEFINE_SPINLOCK(x)

#define dprx_osal_spin_lock(lock)                 spin_lock(lock)
#define dprx_osal_spin_unlock(lock)               spin_unlock(lock)

#define dprx_osal_spin_lock_irqsave(lock)         unsigned long flags = 0; \
                                                  spin_lock_irqsave(lock, flags)
#define dprx_osal_spin_unlock_irqrestore(lock)    spin_unlock_irqrestore(lock, (unsigned long) flags)

//-----------------------------------------------------
// debug
//-----------------------------------------------------
#define dprx_err(fmt, args...)           rtd_pr_dprx_err(fmt, ## args)
#define dprx_warn(fmt, args...)          rtd_pr_dprx_warn(fmt, ## args)
#define dprx_info(fmt, args...)          rtd_pr_dprx_info(fmt, ## args)
#define dprx_dbg(fmt, args...)           rtd_pr_dprx_debug(fmt, ## args)

//-----------------------------------------------------
// Define
//-----------------------------------------------------
#define DPRX_ASYNCHRONOUS_DETECT_TASK

#ifndef EXPORT_SYMBOL
#define EXPORT_SYMBOL(x)
#endif

#ifdef __cplusplus
}
#endif


#endif // __DPRX_OSAL_LINUX_H__
