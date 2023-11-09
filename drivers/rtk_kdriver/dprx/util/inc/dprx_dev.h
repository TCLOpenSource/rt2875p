/*=============================================================
 * File:    dprx_dev.h
 *
 * Desc:    DPRX device
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
#ifndef __DPRX_DEV_H__
#define __DPRX_DEV_H__

typedef struct
{
    const char* name;
    void* p_private;    // device private data
}dev_ctx_t;


#endif // __DPRX_EDID_H__
