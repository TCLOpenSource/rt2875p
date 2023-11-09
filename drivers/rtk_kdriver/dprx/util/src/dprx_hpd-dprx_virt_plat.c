/*=============================================================
 * File:    dprx_hpd.c
 *
 * Desc:    DPRX hpd for RTK TV
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
#include <dprx_hpd-dprx_virt_plat.h>
#include <virtual_platform.h>
#include <dprx_data_type.h>
#include <dprx_config.h>
#include <virtual_cable.h>

#define GET_VIRT_PLAT_HPD(p_ctx)        (VIRT_DPRX_HPD*)((p_ctx) ? p_ctx->p_private : NULL)


/*------------------------------------------------
 * Func : _ops_init
 *
 * Desc : init DPRX HPD device
 *
 * Para : p_adp : handle of DPRX HPD HW
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_init(dev_ctx_t* p_ctx)
{
    VIRT_DPRX_HPD* p_dev = GET_VIRT_PLAT_HPD(p_ctx);

    if (p_dev==NULL)
        return -1;

    virt_cable_set_hpd(0);
    return 0;
}


/*------------------------------------------------
 * Func : _ops_uninit
 *
 * Desc : unint DPRX adapter. this function should
 *        be called when adapter to be destroyed
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_uninit(dev_ctx_t* p_ctx)
{
    VIRT_DPRX_HPD* p_dev = GET_VIRT_PLAT_HPD(p_ctx);

    if (p_dev==NULL)
        return -1;

    virt_cable_set_hpd(0);
    return 0;
}


/*------------------------------------------------
 * Func : _ops_get_connect_status
 *
 * Desc : get cable connection status
 *
 * Para : p_dev : handle of DPRX adapter
 *
 * Retn : 0 : disconnect, 1 : connected
 *-----------------------------------------------*/
static int _ops_get_connect_status(dev_ctx_t* p_ctx)
{
    VIRT_DPRX_HPD* p_dev = GET_VIRT_PLAT_HPD(p_ctx);
    return (p_dev) ? virt_cable_get_cable_status() : 0;
}


/*------------------------------------------------
 * Func : _ops_set_hpd
 *
 * Desc : set/unset HPD signal of the DPRX adapter
 *
 * Para : p_dev : handle of DPRX adapter
 *        on    : 0 : HPD low, others : HPD high
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_set_hpd(dev_ctx_t* p_ctx, unsigned char on)
{
    VIRT_DPRX_HPD* p_dev = GET_VIRT_PLAT_HPD(p_ctx);

    if (p_dev==NULL)
        return -1;

    virt_cable_set_hpd(on);
    return 0;
}


/*------------------------------------------------
 * Func : _ops_get_hpd
 *
 * Desc : get HPD status of the DPRX adapter
 *
 * Para : p_dev : handle of DPRX adapter
 *
 * Retn : 0 : HPD low, 1 : HPD high
 *-----------------------------------------------*/
static int _ops_get_hpd(dev_ctx_t* p_ctx)
{
    VIRT_DPRX_HPD* p_dev = GET_VIRT_PLAT_HPD(p_ctx);

    return (p_dev) ? virt_cable_get_hpd() : 0;
}


/*------------------------------------------------
 * Func : create_dprx_virt_plat_hpd_device
 *
 * Desc : alocate a hpd device of DPRX hpd
 *
 * Para : port :
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
hpd_dev_t* create_dprx_virt_plat_hpd_device(
    unsigned char       port
    )
{
    VIRT_DPRX_HPD* p_dev = (VIRT_DPRX_HPD*) dprx_osal_malloc(sizeof(VIRT_DPRX_HPD));

    if (p_dev == NULL)
        return NULL;

    // hpd dev
    p_dev->hpd_dev.ctx.name = "VHPD";
    p_dev->hpd_dev.ctx.p_private = (void*) p_dev;

    p_dev->hpd_dev.ops.init = _ops_init;
    p_dev->hpd_dev.ops.uninit = _ops_uninit;
    p_dev->hpd_dev.ops.get_connect_status = _ops_get_connect_status;
    p_dev->hpd_dev.ops.set_hpd = _ops_set_hpd;
    p_dev->hpd_dev.ops.get_hpd = _ops_get_hpd;

    // hpd private data
    p_dev->port = port;

    return &p_dev->hpd_dev;
}

