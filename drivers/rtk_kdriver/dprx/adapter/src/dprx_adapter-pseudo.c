/*=============================================================
 * File:    dprx_adapter-pseudo.c
 *
 * Desc:    Pseudo DPRX adapter for RTK TV
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
#include <dprx_adapter-pseudo.h>

//----------------------------------------------------------------------------------------
// DPRX operations
//----------------------------------------------------------------------------------------

/*------------------------------------------------
 * Func : _ops_dprx_init
 *
 * Desc : init DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_init(dprx_adp_t* p_adp)
{
    DPRX_ADP_DBG("_ops_dprx_init!!!!\n");
    return 0;
}


/*------------------------------------------------
 * Func : _ops_dprx_uninit
 *
 * Desc : unint DPRX adapter. this function should
 *        be called when adapter to be destroyed
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_uninit(dprx_adp_t* p_adp)
{
    DPRX_ADP_DBG("_ops_dprx_uninit!!!!\n");
    return 0;
}


/*------------------------------------------------
 * Func : _ops_dprx_open
 *
 * Desc : open DPRX adapter. after open, the DPRX
 *        adapter will become operational. caller
 *        should call open before any other operations
 *        excepts init/uninit
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_open(dprx_adp_t* p_adp)
{
    DPRX_ADP_DBG("_ops_dprx_open!!!!\n");
    return 0;
}


/*------------------------------------------------
 * Func : _ops_dprx_close
 *
 * Desc : close DPRX adapter. after close, the DPRX
 *        adapter will stop operation.
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_close(dprx_adp_t* p_adp)
{
    DPRX_ADP_DBG("_ops_dprx_close!!!!\n");
    return 0;
}


/*------------------------------------------------
 * Func : _ops_dprx_connect
 *
 * Desc : connect DPRX adapter. after connect, the
 *        DPRX adapter will begin audio/video format
 *        detection and able to output audio/video
 *        after detect.
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_connect(dprx_adp_t* p_adp)
{
    DPRX_ADP_DBG("_ops_dprx_connect!!!!\n");
    return 0;
}

/*------------------------------------------------
 * Func : _ops_dprx_disconnect
 *
 * Desc : disable DPRX adapter. after detect, the
 *        DPRX adapter detect/output audio/video data
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_disconnect(dprx_adp_t* p_adp)
{
    DPRX_ADP_DBG("_ops_dprx_disconnect!!!!\n");
    return 0;
}

// HPD/Detect

/*------------------------------------------------
 * Func : _ops_dprx_get_connect_status
 *
 * Desc : get cable connection status
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : disconnect, 1 : connected
 *-----------------------------------------------*/
static int _ops_dprx_get_connect_status(dprx_adp_t* p_adp)
{
    DPRX_ADP_DBG("_ops_dprx_get_connect_status!!!!\n");
    return 0;
}

/*------------------------------------------------
 * Func : _ops_dprx_set_hpd
 *
 * Desc : set/unset HPD signal of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *        on    : 0 : HPD low, others : HPD high
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_set_hpd(dprx_adp_t* p_adp, unsigned char on)
{
    DPRX_ADP_DBG("_ops_dprx_set_hpd!!!!\n");
    return 0;
}

/*------------------------------------------------
 * Func : _ops_dprx_get_hpd
 *
 * Desc : get HPD status of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : HPD low, 1 : HPD high
 *-----------------------------------------------*/
static int _ops_dprx_get_hpd(dprx_adp_t* p_adp)
{
    DPRX_ADP_DBG("_ops_dprx_get_hpd!!!!\n");
    return 0;
}

// EDID operations


/*------------------------------------------------
 * Func : _ops_dprx_set_edid
 *
 * Desc : set EDID of the DPRX adapter
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_edid : edid data
 *        edid_len : size of edid data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_set_edid(
    dprx_adp_t*             p_adp,
    unsigned char*          p_edid,
    unsigned short          edid_len
    )
{
    DPRX_ADP_DBG("_ops_dprx_set_edid!!!!\n");
    return 0;
}


/*------------------------------------------------
 * Func : _ops_dprx_get_edid
 *
 * Desc : get EDID of the DPRX adapter
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_edid : edid data
 *        edid_len : size of edid data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_edid(
    dprx_adp_t*             p_adp,
    unsigned char*          p_edid,
    unsigned short          edid_len
    )
{
    DPRX_ADP_DBG("_ops_dprx_get_edid!!!!\n");
    return 0;
}

//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------

static dprx_adp_ops_t g_pseudo_dprx_ops =
{
    .init       = _ops_dprx_init,
    .uninit     = _ops_dprx_uninit,
    .open       = _ops_dprx_open,
    .close      = _ops_dprx_close,
    .connect    = _ops_dprx_connect,
    .disconnect = _ops_dprx_disconnect,

    // HPD/Detect
    .get_connect_status = _ops_dprx_get_connect_status,
    .set_hpd    = _ops_dprx_set_hpd,
    .get_hpd    = _ops_dprx_get_hpd,

    // EDID
    .set_edid   = _ops_dprx_set_edid,
    .get_edid   = _ops_dprx_get_edid,
};

dprx_dev_t* create_pseudo_dprx_adapter(void)
{
    dprx_dev_t* p_dev = alloc_dprx_device();

    if (p_dev)
    {
        p_dev->adp.name = "PSEUDO_DPRX";
        p_dev->adp.p_private = NULL;
        memcpy(&p_dev->ops, &g_pseudo_dprx_ops, sizeof(dprx_adp_ops_t));
    }

    return p_dev;
}

