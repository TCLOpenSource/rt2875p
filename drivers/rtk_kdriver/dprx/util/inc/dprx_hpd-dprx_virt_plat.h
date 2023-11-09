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
#ifndef __DPRX_HPD_DPRX_VIRT_PLAT_H__
#define __DPRX_HPD_DPRX_VIRT_PLAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <dprx_hpd.h>

typedef struct
{
    hpd_dev_t       hpd_dev;
    unsigned char   port;
}VIRT_DPRX_HPD;

//================= API of DPRX HPD ==============

extern hpd_dev_t* create_dprx_virt_plat_hpd_device(unsigned char port);

#ifdef __cplusplus
}
#endif

#endif // __DPRX_HPD_DPRX_VIRT_PLAT_H__
