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
#include <dprx_hpd.h>


//----------------------------------------------------------------------------------------
// DPRX Operations
//----------------------------------------------------------------------------------------

/*------------------------------------------------
 * Func : dprx_hpd_init
 *
 * Desc : init DPRX HPD device
 *
 * Para : p_adp : handle of DPRX HPD HW
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int dprx_hpd_init(hpd_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.init)
        return p_dev->ops.init(&p_dev->ctx);
    return 0;
}


/*------------------------------------------------
 * Func : dprx_hpd_uninit
 *
 * Desc : unint DPRX adapter. this function should
 *        be called when adapter to be destroyed
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int dprx_hpd_uninit(hpd_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.uninit)
        return p_dev->ops.uninit(&p_dev->ctx);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_hpd_get_connect_status
 *
 * Desc : get cable connection status
 *
 * Para : p_dev : handle of DPRX adapter
 *
 * Retn : 0 : disconnect, 1 : connected
 *-----------------------------------------------*/
int dprx_hpd_get_connect_status(hpd_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.get_connect_status)
        return p_dev->ops.get_connect_status(&p_dev->ctx);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_hpd_get_aux_status
 *
 * Desc : get cable aux status
 *
 * Para : p_dev : handle of DPRX adapter
 *        p_aux_status : aux status output (optional)
 *             b0 : aux_n status
 *             b1 : aux_p status
 *
 * Retn : 0 : disconnect, 1 : connected
 *-----------------------------------------------*/
int dprx_hpd_get_aux_status(hpd_dev_t* p_dev, unsigned char* p_aux_status)
{
    if (p_dev && p_dev->ops.get_aux_status)
        return p_dev->ops.get_aux_status(&p_dev->ctx, p_aux_status);

    if (p_aux_status)
        *p_aux_status = 0;

    return 0;
}


/*------------------------------------------------
 * Func : dprx_hpd_set_hpd
 *
 * Desc : set/unset HPD signal of the DPRX adapter
 *
 * Para : p_dev : handle of DPRX adapter
 *        on    : 0 : HPD low, others : HPD high
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_hpd_set_hpd(hpd_dev_t* p_dev, unsigned char on)
{
    if (p_dev && p_dev->ops.set_hpd)
        return p_dev->ops.set_hpd(&p_dev->ctx, on);

    return 0;
}

/*------------------------------------------------
 * Func : rtk_dprx_set_hpd_toggle
 *
 * Desc : set HPD toggle signal of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *        delay_ms  : the Interval of HPD toggle
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int dprx_hpd_set_hpd_toggle(hpd_dev_t* p_dev, unsigned short delay_ms)
{
    if (p_dev && p_dev->ops.set_hpd)
        return p_dev->ops.set_hpd_toggle(&p_dev->ctx, delay_ms);

    return 0;
}

/*------------------------------------------------
 * Func : dprx_hpd_set_irq_hpd
 *
 * Desc : set HPD irq signal of the DPRX adapter
 *
 * Para : p_dev : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_hpd_set_irq_hpd(hpd_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.set_irq_hpd)
        return p_dev->ops.set_irq_hpd(&p_dev->ctx);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_hpd_get_hpd
 *
 * Desc : get HPD status of the DPRX adapter
 *
 * Para : p_dev : handle of DPRX adapter
 *
 * Retn : 0 : HPD low, 1 : HPD high
 *-----------------------------------------------*/
int dprx_hpd_get_hpd(hpd_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.get_hpd)
        return p_dev->ops.get_hpd(&p_dev->ctx);

    return 0;
}

/*------------------------------------------------
 * Func : dprx_hpd_get_cable_config
 *
 * Desc : get cable config of the DPRX adapter
 *
 * Para : p_dev : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0 : failed
 *-----------------------------------------------*/
int dprx_hpd_get_cable_config(hpd_dev_t* p_dev, DPRX_CABLE_CFG* p_cable_cfg)
{
    if (p_dev && p_dev->ops.get_hpd)
        return p_dev->ops.get_cable_config(&p_dev->ctx, p_cable_cfg);

    return 0;
}

/*------------------------------------------------
 * Func : dprx_hpd_set_typec_re_alt_mode
 *
 * Desc : Re-alt mode of the DPRX adapter
 *
 * Para : p_dev : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0 : failed
 *-----------------------------------------------*/
int dprx_hpd_set_typec_re_alt_mode(hpd_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.get_hpd)
        return p_dev->ops.set_typec_re_alt_mode(&p_dev->ctx);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_hpd_register_event_handler
 *
 * Desc : register even handler callback
 *
 * Para : p_dev : handle of DPRX adapter
 *        p_handler : event handler callback
 *
 * Retn : 0 : successed, 0 : failed
 *-----------------------------------------------*/
int dprx_hpd_register_event_handler(
    hpd_dev_t*      p_dev,
    DPRX_HPD_EVENT_HANDLER* p_handler
    )
{
    if (p_dev && p_dev->ops.register_event_handler)
        return p_dev->ops.register_event_handler(&p_dev->ctx, p_handler);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_hpd_set_event_handler_enable
 *
 * Desc : Enable / Disble HPD event handler
 *
 * Para : p_dev : handle of DPRX adapter
 *        enable : 0 : disable, others : enable
 *
 * Retn : 0 : successed, 0 : failed
 *-----------------------------------------------*/
int dprx_hpd_set_event_handler_enable(
    hpd_dev_t*      p_dev,
    unsigned char   enable
    )
{
    if (p_dev && p_dev->ops.enable_event_handler)
        return p_dev->ops.enable_event_handler(&p_dev->ctx, enable);

    return 0;
}


//----------------------------------------------------------------------------------------
// MISC API
//----------------------------------------------------------------------------------------

#ifdef CONFIG_DPRX_VFE_ENABLE_HPD_LOW_GUARD_INTERVAL

/*------------------------------------------------
 * Func : _get_hpd_low_guard_time
 *
 * Desc : compute remain time for hpd low gard time
 *
 * Para : curr_time :
 *        pre_hpd_low_time_stamp :
 *        guard_time :
 *
 * Retn : remain guard time im ms
 *-----------------------------------------------*/
unsigned int _get_hpd_low_guard_time(
    unsigned long curr_time,
    unsigned long pre_hpd_low_time_stamp,
    unsigned int  guard_time
    )
{
    unsigned long time_diff = 0;

    if (guard_time==0)
        return 0;

    if (curr_time > pre_hpd_low_time_stamp)
        time_diff = curr_time - pre_hpd_low_time_stamp;
    else
        time_diff = curr_time; // time wrap around, to simpilfy, just keep the new value

    return (time_diff > guard_time) ? 0 : (guard_time - time_diff);
}

#endif

//----------------------------------------------------------------------------------------
// Low Level API
//----------------------------------------------------------------------------------------


/*------------------------------------------------
 * Func : alloc_dprx_hpd_device
 *
 * Desc : alocate a dprx_edid device
 *
 * Para : N/A
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
hpd_dev_t* alloc_dprx_hpd_device(void)
{
    hpd_dev_t* p_dev = dprx_osal_malloc(sizeof(hpd_dev_t));

    if (p_dev)
        memset(p_dev, 0, sizeof(hpd_dev_t));

    return p_dev;
}


/*------------------------------------------------
 * Func : destroy_dprx_hpd_device
 *
 * Desc : dsatroy a dprx edid device
 *
 * Para : N/A
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
void destroy_dprx_hpd_device(hpd_dev_t* p_dev)
{
    if (p_dev)
    {
        dprx_hpd_uninit(p_dev);
        dprx_osal_mfree(p_dev);
    }
}

