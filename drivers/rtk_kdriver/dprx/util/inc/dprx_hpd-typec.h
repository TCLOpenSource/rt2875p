/*=============================================================
 * File:    dprx_hpd-typec.h
 *
 * Desc:    DPRX typec detect
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
#ifndef __RTK_TYPEC_HPD_H__
#define __RTK_TYPEC_HPD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <dprx_hpd.h>

typedef struct
{
    hpd_dev_t       hpd_dev;
    unsigned char   typec_id;
    unsigned char   alt_mode_status;
    unsigned short  hpd_low_guard_interval_ms;
    unsigned long   hpd_low_timestamp_ms;   // latest hpd low time
}typec_hpd_dev_t;

//================= API of DPRX HPD ==============

extern hpd_dev_t* create_rtk_tv_typec_hpd_device(unsigned char idx);

#ifdef __cplusplus
}
#endif

#endif // __RTK_TYPEC_HPD_H__
