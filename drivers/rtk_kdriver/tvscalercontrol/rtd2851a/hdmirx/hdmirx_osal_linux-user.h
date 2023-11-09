/*=============================================================
 * File:    hdmirx_osal_linux_user.h
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

#ifndef __HDMIRX_OSAL_LINUX_USER_H__
#define __HDMIRX_OSAL_LINUX_USER_H__
#ifndef UT_flag
#include <no_os/spinlock.h>
#include <no_os/semaphore.h>
#include <no_os/jiffies.h>
#include <no_os/export.h>
#include <no_os/printk.h>
#include <string.h>
#include <malloc.h>
#include <timer.h>
#include <mach/rtk_platform.h>
#include <vsprintf.h>
#include <asm-generic/bug.h>

#define irqreturn_t         unsigned int

#define IRQ_HANDLED  1
#define IRQ_NONE     0

#define IRQ_HDMI     0


#endif

#ifndef BUG
    #define BUG()   //printf("BUG at %s:%d!\n", __FILE__, __LINE__)
#endif



#endif // __HDMIRX_OSAL_LINUX_USER_H__
