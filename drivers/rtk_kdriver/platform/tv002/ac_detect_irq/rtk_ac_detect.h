/*
 *  Copyright (C) 2010 Realtek Semiconductors, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RTK_AC_DET_H
#define __RTK_AC_DET_H

#include <rtd_log/rtd_module_log.h>
#define DEV_NAME "ADET"

struct ac_det_info;
struct ac_det_ops {
    /* To do anthing you want to do when AC detected */
    int (*ac_det_action)(struct ac_det_info* ac_det);
    /* Prepare or check before AC detection */
    int (*ac_det_prepare)(struct ac_det_info* ac_det);

    /* add more func here. if needed. */
};

struct ac_det_info{
    u64 pcb_enum;
    u32 gpio_group;
    u32 gpio_num;
    RTK_GPIO_ID gid;
    struct ac_det_ops* ops;
    void * customer_info;
};

#define _ZERO       0x00
#define _BIT0       0x01
#define _BIT1       0x02
#define _BIT2       0x04
#define _BIT3       0x08
#define _BIT4       0x10
#define _BIT5       0x20
#define _BIT6       0x40
#define _BIT7       0x80
#define _BIT8       0x0100
#define _BIT9       0x0200
#define _BIT10      0x0400
#define _BIT11      0x0800
#define _BIT12      0x1000
#define _BIT13      0x2000
#define _BIT14      0x4000
#define _BIT15      0x8000
#define _BIT16      0x10000
#define _BIT17      0x20000
#define _BIT18      0x40000
#define _BIT19      0x80000
#define _BIT20      0x100000
#define _BIT21      0x200000
#define _BIT22      0x400000
#define _BIT23      0x800000
#define _BIT24      0x1000000
#define _BIT25      0x2000000
#define _BIT26      0x4000000
#define _BIT27      0x8000000
#define _BIT28      0x10000000
#define _BIT29      0x20000000
#define _BIT30      0x40000000
#define _BIT31      0x80000000

/************************************************************************
 *  rtk log define
 ************************************************************************/
 #define rtd_fmt(tag, fmt) "["tag"] "fmt
#define AD_EMERG(fmt, args...)      \
	rtd_pr_platform_emerg(rtd_fmt(DEV_NAME, fmt) , ## args)

#define AD_ALERT(fmt, args...)   \
	rtd_pr_platform_alert(rtd_fmt(DEV_NAME, fmt) , ## args)

#define AD_CRIT(fmt, args...)   \
	rtd_pr_platform_crit(rtd_fmt(DEV_NAME, fmt) , ## args)

#define AD_ERR(fmt, args...)   \
	rtd_pr_platform_err(rtd_fmt(DEV_NAME, fmt) , ## args)

#define AD_WARNING(fmt, args...)   \
	rtd_pr_platform_warn(rtd_fmt(DEV_NAME, fmt) , ## args
)


#define AD_NOTICE(fmt, args...)   
\
	rtd_pr_platform_notice(rtd_fmt(DEV_NAME, fmt) , ## args
)


#define AD_INFO(fmt, args...)   
\
	rtd_pr_platform_info(rtd_fmt(DEV_NAME, fmt) , ## args
)


#define AD_DEBUG(fmt, args...)   
\
	rtd_pr_platform_debug(rtd_fmt(DEV_NAME, fmt) , ## args
#endif //__RTK_AC_DET_H