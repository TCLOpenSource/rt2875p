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
#include <dprx_hpd-rtktv.h>
#include <rtk_gpio.h>

#define GET_RTK_HPD_DEV(p_ctx)          ((p_ctx) ? ((rtk_hpd_dev_t*) p_ctx->p_private) : NULL)

#define MAX_HPD_LOW_PERIOD_MS       3000

#ifdef CONFIG_RTK_HPD_RTKTV_SUPPORT_EVENT_HANDLER

DEFINE_DPRX_OSAL_SPINLOCK(rtk_tv_hpd);


/*------------------------------------------------
 * Func : _hpd_gpio_irq
 *
 * Desc : gpio interrupt handler
 *
 * Para : gid : gpio idx
 *        assert : 0 : deassert, others : assert
 *        dev_id : private data
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static void _hpd_gpio_irq(
    RTK_GPIO_ID         gid,
    unsigned char       assert,
    void*               dev_id
    )
{
    dev_ctx_t* p_ctx = (dev_ctx_t*) dev_id;
    rtk_hpd_dev_t* p_dev = GET_RTK_HPD_DEV(p_ctx);

    dprx_osal_spin_lock(&rtk_tv_hpd);

    if (p_dev && p_dev->event_handler.event_handler)
        p_dev->event_handler.event_handler(p_dev->event_handler.p_ctx, DPRX_HPD_EVENT_CONNECT_STATUS_UPDATE);

    dprx_osal_spin_unlock(&rtk_tv_hpd);
}

#endif

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
    rtk_hpd_dev_t* p_dev = GET_RTK_HPD_DEV(p_ctx);

    if (p_dev==NULL)
        return -1;

    if (p_dev->cfg.aux_n.valid)
        rtk_gpio_set_dir((RTK_GPIO_ID)p_dev->cfg.aux_n.gid, 0);  // set input

    if (p_dev->cfg.aux_p.valid)
        rtk_gpio_set_dir((RTK_GPIO_ID)p_dev->cfg.aux_p.gid, 0);  // set input

    if (p_dev->cfg.det.valid)
    {
        rtk_gpio_set_dir((RTK_GPIO_ID)p_dev->cfg.det.gid, 0);  // set input

#ifdef CONFIG_RTK_HPD_RTKTV_SUPPORT_EVENT_HANDLER
        rtk_gpio_set_irq_enable((RTK_GPIO_ID)p_dev->cfg.det.gid, 0);
        rtk_gpio_request_irq((RTK_GPIO_ID)p_dev->cfg.det.gid, _hpd_gpio_irq, "DP_DET_MONITOR", p_ctx);
#endif
    }

    if (p_dev->cfg.hpd.valid)
    {
        if (p_dev->cfg.qs_mode==0)  // force hpd to low if quick show is not enabled
        {
            DPRX_HPD_INFO("Normal HPD init, force hpd low\n");
            rtk_gpio_output((RTK_GPIO_ID)p_dev->cfg.hpd.gid, 0);     // pull low hpd
            p_dev->hpd_low_timestamp_ms = dprx_odal_get_system_time_ms();  // save hpd low time
        }
        else
        {
            DPRX_HPD_INFO("QS HPD init, do not change HPD state\n");
        }

        rtk_gpio_set_dir((RTK_GPIO_ID)p_dev->cfg.hpd.gid, 1);    // set output
    }

    if (p_dev->cfg.hpd_low_peroid_ms > MAX_HPD_LOW_PERIOD_MS)
        p_dev->cfg.hpd_low_peroid_ms = MAX_HPD_LOW_PERIOD_MS;    // limit hpd low period ms

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
    rtk_hpd_dev_t* p_dev = GET_RTK_HPD_DEV(p_ctx);

    if (p_dev==NULL)
        return -1;

    if (p_dev->cfg.aux_n.valid)
        rtk_gpio_set_dir((RTK_GPIO_ID)p_dev->cfg.aux_n.gid, 0);  // set input

    if (p_dev->cfg.aux_p.valid)
        rtk_gpio_set_dir((RTK_GPIO_ID)p_dev->cfg.aux_p.gid, 0);  // set input

    if (p_dev->cfg.det.valid)
    {
        rtk_gpio_set_dir((RTK_GPIO_ID)p_dev->cfg.det.gid, 0);   // set input

#ifdef CONFIG_RTK_HPD_RTKTV_SUPPORT_EVENT_HANDLER
        rtk_gpio_set_irq_enable((RTK_GPIO_ID)p_dev->cfg.det.gid, 0);
        rtk_gpio_free_irq((RTK_GPIO_ID)p_dev->cfg.det.gid, p_ctx);
#endif
    }

    if (p_dev->cfg.hpd.valid)
    {
        rtk_gpio_output((RTK_GPIO_ID)p_dev->cfg.hpd.gid, 0);     // pull low hpd
        rtk_gpio_set_dir((RTK_GPIO_ID)p_dev->cfg.hpd.gid, 0);    // set output
        p_dev->hpd_low_timestamp_ms = dprx_odal_get_system_time_ms();  // save hpd low time
    }

    return 0;
}


#ifdef ENABLE_DPRX_AUX_CHANNEL_DEBOUNCE

/*------------------------------------------------
 * Func : _ops_get_aux_status
 *
 * Desc : get cable connection status
 *
 * Para : p_dev : handle of DPRX adapter
 *        p_aux_status : aux status output (optional)
 *
 * Retn : 0 : disconnect, 1 : connected
 *-----------------------------------------------*/
static int _ops_get_aux_status(
    dev_ctx_t*      p_ctx,
    unsigned char*  p_aux_status
    )
{
    rtk_hpd_dev_t* p_dev = GET_RTK_HPD_DEV(p_ctx);
    unsigned char  aux_pn = 0;
    unsigned char  i;

    if (p_dev==NULL)
        return -1;

    // for the platform that without Aux detect Pin, use det pin to instead
    if (p_dev->cfg.aux_n.valid==0 && p_dev->cfg.aux_p.valid==0 && p_dev->cfg.det.valid)
    {
        if (rtk_gpio_input((RTK_GPIO_ID)p_dev->cfg.det.gid)==0)
            aux_pn = 0x1; // connected status
        else
            aux_pn = 0x2; // disconnected
        goto end_proc;
    }

    // Connected : AUX_N = 3.3V, AUX_P = 0V
    // Disconnected : AUX_N = 0V, AUX_P = 3.3V
    for (i=0; i<10; i++)
    {
        aux_pn = 0;

        if (p_dev->cfg.aux_n.valid && rtk_gpio_input((RTK_GPIO_ID)p_dev->cfg.aux_n.gid))
            aux_pn |= 0x1;

        if (p_dev->cfg.aux_p.valid && rtk_gpio_input((RTK_GPIO_ID)p_dev->cfg.aux_p.gid))
            aux_pn |= 0x2;

        switch(aux_pn)
        {
        case 1:   //  Connected : AUX_N = 3.3V, AUX_P = 0V
        case 3:   //  Connected : AUX_N = 3.3V, AUX_P = 3.3V (transition state)
            goto end_proc;
            break;
        case 0:   //  connected but Source pull AUX_N to low : AUX_N = 0V, AUX_P = 0V  (Source Suspend)
        case 2:   //  Disconnected : AUX_N = 3.3V, AUX_P = 0V
            // do not return immedately, need debounce
            break;
        default:
           // should not happen
           break;
        }
    }

end_proc:
    if (p_aux_status)
        *p_aux_status = aux_pn;

    if (aux_pn == 0x2 || aux_pn ==0)  // AUX_N = High, AUX_P=Low
        return 0;

    return 1;
}

#else

/*------------------------------------------------
 * Func : _ops_get_aux_status
 *
 * Desc : get cable connection status
 *
 * Para : p_dev : handle of DPRX adapter
 *        p_aux_status : aux status output (optional)
 *
 * Retn : 0 : disconnect, 1 : connected
 *-----------------------------------------------*/
static int _ops_get_aux_status(
    dev_ctx_t*      p_ctx,
    unsigned char*  p_aux_status
    )
{
    rtk_hpd_dev_t* p_dev = GET_RTK_HPD_DEV(p_ctx);
    unsigned char  aux_pn = 0;

    if (p_dev==NULL)
        return -1;

    // for the platform that without Aux detect Pin, use det pin to instead
    if (p_dev->cfg.aux_n.valid==0 && p_dev->cfg.aux_p.valid==0 && p_dev->cfg.det.valid)
    {
        if (rtk_gpio_input((RTK_GPIO_ID)p_dev->cfg.det.gid)==0)
            aux_pn = 0x1; // connected status
        else
            aux_pn = 0x2; // disconnected
        goto end_proc;
    }

    // Connected : AUX_N = 3.3V, AUX_P = 0V
    // Disconnected : AUX_N = 0V, AUX_P = 3.3V
    if (p_dev->cfg.aux_n.valid && rtk_gpio_input((RTK_GPIO_ID)p_dev->cfg.aux_n.gid))
        aux_pn |= 0x1;

    if (p_dev->cfg.aux_p.valid && rtk_gpio_input((RTK_GPIO_ID)p_dev->cfg.aux_p.gid))
        aux_pn |= 0x2;

end_proc:
    if (p_aux_status)
        *p_aux_status = aux_pn;

    if (aux_pn == 0x1)  // AUX_N = High, AUX_P=Low
        return 1;

    return 0;
}

#endif


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
    rtk_hpd_dev_t* p_dev = GET_RTK_HPD_DEV(p_ctx);

    if (p_dev==NULL)
        return -1;

    // DET pin is optional, if DET pin present, return DET status
    // Otherwise, use aux status to instead
    if (p_dev->cfg.det.valid)
        return rtk_gpio_input((RTK_GPIO_ID)p_dev->cfg.det.gid) ? 0 : 1;

    return _ops_get_aux_status(p_ctx, NULL);   // Use aux status to instead
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
    rtk_hpd_dev_t* p_dev = GET_RTK_HPD_DEV(p_ctx);

    if (p_dev==NULL)
        return -1;

    if (p_dev->cfg.hpd.valid)
    {
        if (on)
        {
            unsigned int guard_time = _get_hpd_low_guard_time(dprx_odal_get_system_time_ms(),
                                            p_dev->hpd_low_timestamp_ms,
                                            p_dev->cfg.hpd_low_peroid_ms);
            if (guard_time)
            {
                DPRX_HPD_INFO("Set HPD on with in guard interval(%d ms), delay HPD high for %d ms\n",
                    p_dev->cfg.hpd_low_peroid_ms, guard_time);
                dprx_osal_msleep(guard_time);
            }
        }
        else
        {
            if (rtk_gpio_output_get((RTK_GPIO_ID)p_dev->cfg.hpd.gid))  // update hdp low time stamp when hpd from high to low
                p_dev->hpd_low_timestamp_ms = dprx_odal_get_system_time_ms();
        }

        DPRX_HPD_INFO("Set HPD = %d\n", on);
        rtk_gpio_output((RTK_GPIO_ID)p_dev->cfg.hpd.gid, (on) ? 1 : 0);
        rtk_gpio_set_dir((RTK_GPIO_ID)p_dev->cfg.hpd.gid, 1);    // set output
    }

    return 0;
}

/*------------------------------------------------
 * Func : _ops_set_hpd_toggle
 *
 * Desc : set HPD toggle signal of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *        delay_ms  : the Interval of HPD toggle
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_set_hpd_toggle(dev_ctx_t* p_ctx, unsigned short delay_ms)
{
    _ops_set_hpd(p_ctx, 0);
    dprx_osal_msleep(delay_ms);
    _ops_set_hpd(p_ctx, 1);

    return 1;
}

/*------------------------------------------------
 * Func : _ops_set_irq_hpd
 *
 * Desc : set HPD irq signal of the DPRX adapter
 *
 * Para : p_dev : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_set_irq_hpd(dev_ctx_t* p_ctx)
{
    rtk_hpd_dev_t* p_dev = GET_RTK_HPD_DEV(p_ctx);

    if (p_dev==NULL)
        return -1;

    if (p_dev->cfg.hpd.valid) {
        DPRX_HPD_INFO("Set HPD IRQ\n");
        rtk_gpio_output((RTK_GPIO_ID)p_dev->cfg.hpd.gid, 0);
        rtk_gpio_set_dir((RTK_GPIO_ID)p_dev->cfg.hpd.gid, 1);    // set output

        dprx_osal_usleep(600);

        rtk_gpio_output((RTK_GPIO_ID)p_dev->cfg.hpd.gid, 1);
        rtk_gpio_set_dir((RTK_GPIO_ID)p_dev->cfg.hpd.gid, 1);    // set output
    }

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
    rtk_hpd_dev_t* p_dev = GET_RTK_HPD_DEV(p_ctx);

    if (p_dev==NULL)
        return -1;

    if (p_dev->cfg.hpd.valid)
        return rtk_gpio_output_get((RTK_GPIO_ID)p_dev->cfg.hpd.gid);

    return 0;
}

#ifdef CONFIG_RTK_HPD_RTKTV_SUPPORT_EVENT_HANDLER

/*------------------------------------------------
 * Func : _ops_register_event_handler
 *
 * Desc : register even handler callback
 *
 * Para : p_ctx : handle of HPD
 *        p_handler : event handler callback
 *
 * Retn : 0 : successed, 0 : failed
 *-----------------------------------------------*/
static int _ops_register_event_handler(
    dev_ctx_t*      p_ctx,
    DPRX_HPD_EVENT_HANDLER* p_handler
    )
{
    rtk_hpd_dev_t* p_dev = GET_RTK_HPD_DEV(p_ctx);

    if (p_dev==NULL)
        return -1;
    else
    {
        dprx_osal_spin_lock_irqsave(&rtk_tv_hpd);

        if (p_handler)
            memcpy(&p_dev->event_handler, p_handler, sizeof(DPRX_HPD_EVENT_HANDLER));
        else
            memset(&p_dev->event_handler, 0, sizeof(DPRX_HPD_EVENT_HANDLER));

        dprx_osal_spin_unlock_irqrestore(&rtk_tv_hpd);
    }

    return 0;
}


/*------------------------------------------------
 * Func : _ops_enable_event_handler
 *
 * Desc : Enable / Disble HPD event handler
 *
 * Para : p_ctx : handle of HPD
 *        enable : 0 : disable, others : enable
 *
 * Retn : 0 : successed, 0 : failed
 *-----------------------------------------------*/
static int _ops_enable_event_handler(
    dev_ctx_t*      p_ctx,
    unsigned char   enable
    )
{
    rtk_hpd_dev_t* p_dev = GET_RTK_HPD_DEV(p_ctx);

    if (p_dev==NULL)
        return -1;

    if (p_dev->cfg.det.valid && enable)
    {
        rtk_gpio_set_debounce((RTK_GPIO_ID)p_dev->cfg.det.gid, 0);
        rtk_gpio_set_irq_enable((RTK_GPIO_ID)p_dev->cfg.det.gid, 1);
    }
    else
    {
        rtk_gpio_set_irq_enable((RTK_GPIO_ID)p_dev->cfg.det.gid, 0);
    }

    return 0;
}

#endif

/*------------------------------------------------
 * Func : create_rtk_tv_dprx_hpd_device
 *
 * Desc : alocate a dprx_edid device
 *
 * Para : N/A
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
hpd_dev_t* create_rtk_tv_dprx_hpd_device(rtk_hpd_cfg_t* p_cfg)
{
    rtk_hpd_dev_t* p_dev = NULL;

    if (p_cfg==NULL ||
        p_cfg->aux_n.valid==0 ||
        p_cfg->aux_p.valid==0 ||
        p_cfg->hpd.valid==0)
    {
        DPRX_HPD_ERR("create_rtk_tv_dprx_hpd_device failed, invalid configuration, please check your configiration\n");
        return NULL;
    }

    p_dev = (rtk_hpd_dev_t*) dprx_osal_malloc(sizeof(rtk_hpd_dev_t));

    if (p_dev)
    {
        memset(p_dev, 0, sizeof(rtk_hpd_dev_t));
        p_dev->hpd_dev.ctx.name = "HPD";
        p_dev->hpd_dev.ctx.p_private = (void*) p_dev;

        p_dev->hpd_dev.ops.init   = _ops_init;
        p_dev->hpd_dev.ops.uninit  = _ops_uninit;
        p_dev->hpd_dev.ops.get_aux_status = _ops_get_aux_status;
        p_dev->hpd_dev.ops.get_connect_status = _ops_get_connect_status;
        p_dev->hpd_dev.ops.set_hpd = _ops_set_hpd;
        p_dev->hpd_dev.ops.set_hpd_toggle = _ops_set_hpd_toggle;
        p_dev->hpd_dev.ops.get_hpd = _ops_get_hpd;
        p_dev->hpd_dev.ops.set_irq_hpd = _ops_set_irq_hpd;
#ifdef CONFIG_RTK_HPD_RTKTV_SUPPORT_EVENT_HANDLER
        p_dev->hpd_dev.ops.register_event_handler = _ops_register_event_handler;
        p_dev->hpd_dev.ops.enable_event_handler = _ops_enable_event_handler;
#endif

        memcpy(&p_dev->cfg, p_cfg, sizeof(p_dev->cfg));
    }

    return &p_dev->hpd_dev;
}


#define GET_GPIO_ID(iso, gpio_id)       rtk_gpio_id((iso) ? ISO_GPIO : MIS_GPIO, gpio_id)


/*------------------------------------------------
 * Func : create_rtk_tv_dprx_hpd_device
 *
 * Desc : alocate a dprx_edid device
 *
 * Para : N/A
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
hpd_dev_t* create_rtk_tv_dprx_hpd_device_ex(DP_HPD_DET_CFG* p_cfg)
{
    rtk_hpd_cfg_t cfg;

    memset(&cfg, 0, sizeof(cfg));

    if (p_cfg->hpd_gpio_valid)
    {
        cfg.hpd.valid = 1;
        cfg.hpd.gid = GET_GPIO_ID(p_cfg->hpd_gpio_type, p_cfg->hpd_gpio);
    }

    if (p_cfg->det_gpio_valid)
    {
        cfg.det.valid = 1;
        cfg.det.gid = GET_GPIO_ID(p_cfg->det_gpio_type, p_cfg->det_gpio);
    }

    if (p_cfg->aux_n_gpio_valid)
    {
        cfg.aux_n.valid = 1;
        cfg.aux_n.gid = GET_GPIO_ID(p_cfg->aux_n_gpio_type, p_cfg->aux_n_gpio);
    }

    if (p_cfg->aux_p_gpio_valid)
    {
        cfg.aux_p.valid = 1;
        cfg.aux_p.gid = GET_GPIO_ID(p_cfg->aux_p_gpio_type, p_cfg->aux_p_gpio);
    }

    if ((p_cfg->flags & DP_HPD_QS_INIT))
    {
        cfg.qs_mode = 1;
    }

    cfg.hpd_low_peroid_ms = DEFAULT_DP_HPD_LOW_PERIOD_MS;

    return create_rtk_tv_dprx_hpd_device(&cfg);
}

