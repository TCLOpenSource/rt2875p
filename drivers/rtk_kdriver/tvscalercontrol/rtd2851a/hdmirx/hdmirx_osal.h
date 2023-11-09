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

#ifndef __HDMIRX_OSAL_H__
#define __HDMIRX_OSAL_H__
#ifndef BUILD_QUICK_SHOW
#include <generated/autoconf.h>
#endif

#if (defined PLATFORM_LINUX_USER)  || (defined BUILD_QUICK_SHOW) || (defined UT_flag)
#include "hdmirx_osal_linux-user.h"
#else
#include "hdmirx_osal_linux.h"
#endif

#endif // __HDMIRX_OSAL_H__
