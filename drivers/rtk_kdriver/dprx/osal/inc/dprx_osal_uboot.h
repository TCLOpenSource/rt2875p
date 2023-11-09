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

#ifndef __DPRX_OSAL_UBOOT_H__
#define __DPRX_OSAL_UBOOT_H__

#include <malloc.h>
#include <timer.h>
#include <rtd_log/rtd_module_log.h>
#include <vsprintf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define dprx_osal_malloc(size)           malloc(size)
#define dprx_osal_mfree(ptr)             free(ptr)
#define dprx_osal_msleep(ms)             msleep(ms)
#define dprx_osal_usleep(us)             usleep(us)
#define dprx_odal_get_system_time_ms()   (0)

//-----------------------------------------------------
// Spin lock
//-----------------------------------------------------
#define dprx_osal_spinlock_t
#define DEFINE_DPRX_OSAL_SPINLOCK(x)
#define dprx_osal_spin_lock(lock)
#define dprx_osal_spin_unlock(lock)
#define dprx_osal_spin_lock_irqsave(lock)
#define dprx_osal_spin_unlock_irqrestore(lock)

//-----------------------------------------------------
// debug
//-----------------------------------------------------
#if 0
#define dprx_err(fmt, args...)           rtd_pr_dprx_err(fmt, ## args)
#define dprx_warn(fmt, args...)          rtd_pr_dprx_warn(fmt, ## args)
#define dprx_info(fmt, args...)          rtd_pr_dprx_info(fmt, ## args)
#define dprx_dbg(fmt, args...)           rtd_pr_dprx_debug(fmt, ## args)
#else
#define dprx_err(fmt, args...)           printf2("[E][DPRX]" fmt, ## args)
#define dprx_warn(fmt, args...)          printf2("[W][DPRX]" fmt, ## args)
#define dprx_info(fmt, args...)          printf2("[I][DPRX]" fmt, ## args)
#define dprx_dbg(fmt, args...)           printf2("[D][DPRX]" fmt, ## args)
#endif

#define EXPORT_SYMBOL(x)

#ifdef __cplusplus
}
#endif


#endif // __DPRX_OSAL_UBOOT_H__
