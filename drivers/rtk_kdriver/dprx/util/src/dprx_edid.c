/*=============================================================
 * File:    dprx_edid.c
 *
 * Desc:    DPRX adapter for RTK TV
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
#include <dprx_edid.h>


//----------------------------------------------------------------------------------------
// DPRX Operations
//----------------------------------------------------------------------------------------

/*------------------------------------------------
 * Func : dprx_edid_init
 *
 * Desc : init DPRX EDID device
 *
 * Para : p_adp : handle of DPRX EDID HW
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int dprx_edid_init(edid_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.init)
        return p_dev->ops.init(&p_dev->ctx);
    return 0;
}


/*------------------------------------------------
 * Func : dprx_edid_uninit
 *
 * Desc : unint DPRX adapter. this function should
 *        be called when adapter to be destroyed
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int dprx_edid_uninit(edid_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.uninit)
        return p_dev->ops.uninit(&p_dev->ctx);

    return 0;
}

/*------------------------------------------------
 * Func : dprx_edid_enable
 *
 * Desc : enable EDID
 *
 * Para : p_adp  : handle of DPRX adapter
 *        enable : 0 : disable, others : enable
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_edid_enable(edid_dev_t* p_dev, unsigned char enable)
{
    if (p_dev && p_dev->ops.enable)
        return p_dev->ops.enable(&p_dev->ctx, enable);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_edid_reset
 *
 * Desc : reset EDID HW engine
 *
 * Para : p_dev  : handle of DPRX EDID HW
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_edid_reset(edid_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.reset)
        return p_dev->ops.reset(&p_dev->ctx);

    return 0;
}

/*------------------------------------------------
 * Func : dprx_edid_set_edid
 *
 * Desc : set EDID of the DPRX adapter
 *
 * Para : p_dev  : handle of DPRX EDID HW
 *        p_edid : edid data
 *        edid_len : size of edid data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_edid_set_edid(edid_dev_t* p_dev, unsigned char* p_edid, unsigned short edid_len)
{
    if (p_dev && p_dev->ops.set_edid)
        return p_dev->ops.set_edid(&p_dev->ctx, p_edid, edid_len);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_edid_get_edid
 *
 * Desc : set EDID of the DPRX adapter
 *
 * Para : p_dev  : handle of DPRX EDID HW
 *        p_edid : edid data
 *        edid_len : size of edid data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_edid_get_edid(edid_dev_t* p_dev, unsigned char* p_edid, unsigned short edid_len)
{
    if (p_dev && p_dev->ops.get_edid)
        return p_dev->ops.get_edid(&p_dev->ctx, p_edid, edid_len);

    return 0;
}


//----------------------------------------------------------------------------------------
// Low Level API
//----------------------------------------------------------------------------------------


/*------------------------------------------------
 * Func : alloc_dprx_edid_device
 *
 * Desc : alocate a dprx_edid device
 *
 * Para : N/A
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
edid_dev_t* alloc_dprx_edid_device(void)
{
    edid_dev_t* p_dev = dprx_osal_malloc(sizeof(edid_dev_t));

    if (p_dev)
        memset(p_dev, 0, sizeof(edid_dev_t));

    return p_dev;
}


/*------------------------------------------------
 * Func : destroy_dprx_edid_device
 *
 * Desc : dsatroy a dprx edid device
 *
 * Para : N/A
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
void destroy_dprx_edid_device(edid_dev_t* p_dev)
{
    if (p_dev)
    {
        dprx_edid_enable(p_dev, 0);
        dprx_edid_uninit(p_dev);
        dprx_osal_mfree(p_dev);
    }
}
