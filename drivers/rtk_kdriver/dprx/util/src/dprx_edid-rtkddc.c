/*=============================================================
 * File:    dprx_edid_rtk_ddc.c
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
#include <dprx_edid-rtkddc.h>
#include <rtk_kdriver/rtk_ddc_lib.h>

//---------------------------------------------------------------
// Definitions
//---------------------------------------------------------------

#define DDC_WRITE_DEBOUNCE  2
#define DDC_SDA_DELAY       1
#define MAX_EDID_SIZE       512

typedef struct
{
    edid_dev_t      edid_dev;
    unsigned char   idx;
    unsigned long   flags;
}rtk_ddc_dev_t;

//---------------------------------------------------------------
// OPS
//---------------------------------------------------------------

/*------------------------------------------------
 * Func : _ops_init
 *
 * Desc : init DPRX EDID device
 *
 * Para : p_ctx : handle of DPRX EDID HW
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_init(dev_ctx_t* p_ctx)
{
    rtk_ddc_dev_t* p_dev;

    if (p_ctx==NULL)
    {
        DPRX_EDID_ERR("RTK DDC init failed. invalid argument\n");
        return -1;
    }

    p_dev = (rtk_ddc_dev_t*) p_ctx->p_private;

    if ((p_dev->flags & RTK_DDC_EDID_QS_INIT)) {  // skip ddc initial if qs enabled
        DPRX_EDID_INFO("RTK DDC with QS init, skip DDC init procedure\n");
        return 0;
    }

    DPRX_EDID_INFO("RTK DDC init. (ddc_ch=%d), do crt on\n", p_dev->idx);
    rtk_ddc_lib_crt_on(p_dev->idx);  // enable ddc

    rtk_ddc_lib_enable(p_dev->idx, 0);  // disable ddc bus
    return 0;
}

/*------------------------------------------------
 * Func : _ops_uninit
 *
 * Desc : unint DPRX EDID device
 *
 * Para : p_ctx : handle of DPRX EDID HW
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_uninit(dev_ctx_t* p_ctx)
{
    rtk_ddc_dev_t* p_dev;

    if (p_ctx==NULL)
    {
        DPRX_EDID_ERR("RTK DDC uninit failed. invalid argument\n");
        return -1;
    }

    p_dev = (rtk_ddc_dev_t*) p_ctx->p_private;

    DPRX_EDID_INFO("RTK DDC uninit. (ddc_ch=%d)\n", p_dev->idx);
    rtk_ddc_lib_enable(p_dev->idx, 0);  // disable ddc bus

    return 0;
}

/*------------------------------------------------
 * Func : _ops_enable
 *
 * Desc : enable DPRX EDID device. after enable
 *        EDID will become readable
 *
 * Para : p_ctx : handle of DPRX EDID HW
 *        enable : 0 : disable, others : enable
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_enable(dev_ctx_t* p_ctx, unsigned char enable)
{
    rtk_ddc_dev_t* p_dev;

    if (p_ctx==NULL)
    {
        DPRX_EDID_ERR("RTK DDC enable failed. invalid argument\n");
        return -1;
    }

    p_dev = (rtk_ddc_dev_t*) p_ctx->p_private;

    if (rtk_ddc_lib_enable(p_dev->idx, (enable) ? 1 : 0)<0)
    {
        DPRX_EDID_ERR("RTK DDC enable ddc failed. (ddc_ch=%d)\n", p_dev->idx);
        return -1;
    }

    DPRX_EDID_INFO("RTK DDC enable ddc. (ddc_ch=%d)\n", p_dev->idx);

    rtk_ddc_lib_set_write_debounce(p_dev->idx, DDC_WRITE_DEBOUNCE);
    rtk_ddc_lib_set_sda_delay(p_dev->idx, DDC_SDA_DELAY);
    rtk_ddc_lib_aux_rx_enable(p_dev->idx, 1);

    return 0;
}

/*------------------------------------------------
 * Func : _ops_reset
 *
 * Desc : reset EDID HW engine
 *
 * Para : p_ctx : handle of DPRX EDID HW
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_reset(dev_ctx_t* p_ctx)
{
    rtk_ddc_dev_t* p_dev;

    if (p_ctx==NULL)
    {
        DPRX_EDID_ERR("RTK DDC reset failed. invalid argement\n");
        return -1;
    }

    p_dev = (rtk_ddc_dev_t*) p_ctx->p_private;

    if (rtk_ddc_lib_sync_reset(p_dev->idx)<0)
    {
        DPRX_EDID_ERR("RTK DDC reset failed. (ddc_ch=%d)\n", p_dev->idx);
        return -1;
    }

    DPRX_EDID_INFO("RTK DDC reset. (ddc_ch=%d)\n", p_dev->idx);

    rtk_ddc_lib_set_write_debounce(p_dev->idx, DDC_WRITE_DEBOUNCE);
    rtk_ddc_lib_set_sda_delay(p_dev->idx, DDC_SDA_DELAY);

    return 0;
}

/*------------------------------------------------
 * Func : dprx_edid_rtk_ddc_set_edid
 *
 * Desc : set EDID of the DPRX adapter
 *
 * Para : p_ctx :  handle of DPRX EDID HW
 *        p_edid : edid data
 *        edid_len : size of edid data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_set_edid(dev_ctx_t* p_ctx, unsigned char* p_edid, unsigned short edid_len)
{
    rtk_ddc_dev_t* p_dev;

    if (p_ctx==NULL || p_edid==NULL || (edid_len & 0x7F) || (edid_len > MAX_EDID_SIZE))
    {
        DPRX_EDID_ERR("RTK DDC write edid failed, invalid arguments\n");
        return -1;
    }

    p_dev = (rtk_ddc_dev_t*) p_ctx->p_private;

    if (rtk_ddc_lib_write_edid(p_dev->idx, p_edid, edid_len)<0)
    {
        DPRX_EDID_ERR("RTK DDC write edid failed, write data to edid failed\n");
        return -1;
    }

    DPRX_EDID_INFO("RTK DDC write edid pass. (ddc_ch=%d, len=%d)\n", p_dev->idx, edid_len);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_edid_rtk_ddc_get_edid
 *
 * Desc : set EDID of the DPRX adapter
 *
 * Para : p_ctx :  handle of DPRX EDID HW
 *        p_edid : edid data
 *        edid_len : size of edid data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_get_edid(dev_ctx_t* p_ctx, unsigned char* p_edid, unsigned short edid_len)
{
    rtk_ddc_dev_t* p_dev;

    if (p_ctx==NULL || p_edid==NULL || (edid_len & 0x7F) || (edid_len > MAX_EDID_SIZE))
    {
        DPRX_EDID_ERR("RTK DDC read edid failed, invalid arguments\n");
        return -1;
    }

    p_dev = (rtk_ddc_dev_t*) p_ctx->p_private;

    if (rtk_ddc_lib_read_edid(p_dev->idx, p_edid, edid_len)<0)
    {
        DPRX_EDID_ERR("RTK DDC read edid failed, write data to edid failed\n");
        return -1;
    }

    return 0;
}


/*------------------------------------------------
 * Func : create_rtk_ddc_edid_device
 *
 * Desc : create rtk ddc edid device
 *
 * Para : ddc_idx : ddc index (0/1/2...)
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
edid_dev_t* create_rtk_ddc_edid_device(unsigned char ddc_idx)
{
    return create_rtk_ddc_edid_device_ex(ddc_idx, 0);
}



/*------------------------------------------------
 * Func : create_rtk_ddc_edid_device_ex
 *
 * Desc : create rtk ddc edid device
 *
 * Para : ddc_idx : ddc index (0/1/2...)
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
edid_dev_t* create_rtk_ddc_edid_device_ex(
    unsigned char   ddc_idx,
    unsigned long   flags
    )
{
    rtk_ddc_dev_t* p_dev = dprx_osal_malloc(sizeof(rtk_ddc_dev_t));

    if (p_dev)
    {
        memset(p_dev, 0, sizeof(rtk_ddc_dev_t));
        p_dev->idx = ddc_idx;
        p_dev->flags = flags;
        p_dev->edid_dev.ctx.name = "RTK_DDC";
        p_dev->edid_dev.ctx.p_private = (void*) p_dev;
        p_dev->edid_dev.ops.init   = _ops_init;
        p_dev->edid_dev.ops.uninit = _ops_uninit;
        p_dev->edid_dev.ops.enable = _ops_enable;
        p_dev->edid_dev.ops.reset  = _ops_reset;
        p_dev->edid_dev.ops.set_edid = _ops_set_edid;
        p_dev->edid_dev.ops.get_edid = _ops_get_edid;
    }
    return &p_dev->edid_dev;
}
