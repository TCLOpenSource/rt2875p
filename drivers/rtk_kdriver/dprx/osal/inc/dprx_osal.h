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

#ifndef __DPRX_OSAL_H__
#define __DPRX_OSAL_H__

#ifdef PLATFORM_LINUX_USER
#include <dprx_osal_linux-user.h>
#elif defined (CONFIG_DPRX_PRJ_PLATFROM_UBOOT)
#include <dprx_osal_uboot.h>
#else
#include <dprx_osal_linux.h>
#endif

#define MODIFY_DPRX_API         1

#endif // __DPRX_OSAL_H__
