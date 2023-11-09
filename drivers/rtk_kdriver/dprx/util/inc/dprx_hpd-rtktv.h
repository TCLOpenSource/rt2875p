/*=============================================================
 * File:    dprx_hpd.h
 *
 * Desc:    DPRX HPD detect
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
#ifndef __DPRX_HPD_RTK_TV_H__
#define __DPRX_HPD_RTK_TV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <dprx_platform.h>
#include <dprx_hpd.h>

typedef struct
{
    unsigned int    valid:1;
    unsigned int    gid:31;
}rtk_hpd_gpio_cfg;

typedef struct
{
    unsigned char     qs_mode;
    unsigned short    hpd_low_peroid_ms;      // minimum hpd low period
    rtk_hpd_gpio_cfg  det;
    rtk_hpd_gpio_cfg  hpd;
    rtk_hpd_gpio_cfg  aux_n;
    rtk_hpd_gpio_cfg  aux_p;
}rtk_hpd_cfg_t;

typedef struct
{
    hpd_dev_t       hpd_dev;
    rtk_hpd_cfg_t   cfg;
    DPRX_HPD_EVENT_HANDLER event_handler;
    unsigned long   hpd_low_timestamp_ms;   // latest hpd low time
}rtk_hpd_dev_t;

//================= API of DPRX HPD ==============

extern hpd_dev_t* create_rtk_tv_dprx_hpd_device(rtk_hpd_cfg_t* p_cfg);
extern hpd_dev_t* create_rtk_tv_dprx_hpd_device_ex(DP_HPD_DET_CFG* p_cfg);

#ifdef __cplusplus
}
#endif

#endif // __DPRX_HPD_RTK_TV_H__
