/*=============================================================
 * File:    dprx_osal_linux_user.h
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

#ifndef __DPRX_OSAL_LINUX_USER_H__
#define __DPRX_OSAL_LINUX_USER_H__

#include <stdio.h>
#include <stdbool.h> // for bool
#include <stdlib.h>  // malloc
#include <string.h>  // memset / memcpy...
#include <unistd.h>  // usleep

#define dprx_osal_malloc(size)           malloc(size)
#define dprx_osal_mfree(ptr)             free(ptr)
#define dprx_osal_msleep(ms)             usleep(ms * 1000)
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
#define dprx_err(fmt, args...)           printf(fmt, ## args)
#define dprx_warn(fmt, args...)          printf(fmt, ## args)
#define dprx_info(fmt, args...)          printf(fmt, ## args)
#define dprx_dbg(fmt, args...)           printf(fmt, ## args)

#define scnprintf(fmt, args...)          snprintf(fmt, ## args)

#define EXPORT_SYMBOL(x)

#endif // __DPRX_OSAL_LINUX_H__
