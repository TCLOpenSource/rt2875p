/*=============================================================
 * File:    dprx_vfe_api.c
 *
 * Desc:    DPRX VFE API
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
#include <dprx_vfe_api.h>
#include <dprx_vfe.h>

static dprx_dev_t* g_p_dprx[MAX_DPRX_PORT];

#ifdef CONFIG_DPRX_FORCE_EANBLE_WAKEUP_BY_AUX
unsigned char g_dprx_vfe_suspend_mode = WAKEUP_BY_AUX;
#else
unsigned char g_dprx_vfe_suspend_mode = 0;
#endif

dprx_dev_t* _vfe_dprx_drv_get_dprx_adapter(unsigned char ch)
{
    return (ch < MAX_DPRX_PORT) ? g_p_dprx[ch] :  NULL;
}

static unsigned int g_vfe_dprx_init_ready = 0;
static unsigned int g_vfe_dprx_open_ready = 0;
static unsigned char g_vfe_dprx_port_count = 0;
unsigned char g_dprx_vfe_print_ctrl = 0; // for debug message control

#ifdef CONFIG_DPRX_VFE_ENABLE_VFE_STATE_CHECK
#define CHK_DPRX_VFE_OPEN_STATE()           (g_vfe_dprx_init_ready && g_vfe_dprx_open_ready) ? 1 : 0)
#else
#define CHK_DPRX_VFE_OPEN_STATE()           (1)
#endif

/*------------------------------------------------
 * Func : vfe_dprx_drv_init
 *
 * Desc : init DPRX driver. allocate all
 *        resources will be created.
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_init(void)
{
    unsigned char ch;
    dprx_dev_t* p_adp;

    DP_VFE_TRACE();

    if (g_vfe_dprx_init_ready)
    {
        DP_VFE_ERR("vfe_dprx_drv_init failed, init twice\n");
        return DPRX_DRV_NO_ERR;
    }

    dprx_platform_init();

    memset(g_p_dprx, 0, sizeof(g_p_dprx));

    g_connected_dprx = NULL;
    g_vfe_dprx_port_count = 0;

    for(ch = 0; ch < MAX_DPRX_PORT; ch++)
    {
        DP_PORT_CFG port_cfg;
        unsigned char port;

        if (dprx_platform_get_dprx_port(ch, &port)<0)
        {
            DP_VFE_WARN("DP[%d] get hw port failed, no hw port exists\n", ch);
            continue;  // no physical DPRX port associated
        }

        if (dprx_platform_get_port_config(port, &port_cfg)<0)
        {
            DP_VFE_ERR("DP[%d] get hw port config %d failed\n", ch, port);
            continue;
        }

        g_vfe_dprx_port_count=ch+1;  // keep the most valid channel as the dprx port count

        DP_VFE_INFO("DP[%d] get hw port config %d successed. type=%d\n", ch, port, port_cfg.type);

        g_p_dprx[ch] = dprx_vfe_create_dprx_adapter_by_config(&port_cfg);

        if (g_p_dprx[ch]==NULL)
            DP_VFE_WARN("DP[%d] create DPRX adapter failed\n", ch);
        else
        {
            DP_VFE_INFO("DP[%d] create DPRX adapter successed\n", ch);
            p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);
            p_adp->ch = ch;
        }
    }

    g_vfe_dprx_init_ready=1;
    g_vfe_dprx_open_ready=0;
    DP_VFE_ERR("vfe_dprx_drv_init successed, g_vfe_dprx_init_ready=%d\n", g_vfe_dprx_init_ready);
    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_init);

/*------------------------------------------------
 * Func : vfe_dprx_drv_uninit
 *
 * Desc : uninit DPRX driver. all resources
 *        will be released.
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_uninit(void)
{
    unsigned char ch;

    DP_VFE_TRACE();

    if (g_vfe_dprx_init_ready==0)
    {
        DP_VFE_ERR("vfe_dprx_drv_uninit failed, not initialized\n");
        return DPRX_DRV_NO_ERR;
    }

    for(ch = 0; ch < MAX_DPRX_PORT; ch++)
    {
        if (g_p_dprx[ch]==NULL)
            continue;

        g_connected_dprx = NULL;
        destroy_dprx_device(g_p_dprx[ch]);
        g_p_dprx[ch] = NULL;
    }

    dprx_platform_uninit();

    g_vfe_dprx_init_ready = 0;
    g_vfe_dprx_open_ready = 0;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_uninit);

/*------------------------------------------------
 * Func : vfe_dprx_drv_open
 *
 * Desc : open DPRX driver. should be called before
 *        any operations
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_open(void)
{
    unsigned char ch;

    DP_VFE_TRACE();

    if (g_vfe_dprx_open_ready++)
    {
        DP_VFE_ERR("vfe_dprx_drv_open, opern multi times (%d)\n", g_vfe_dprx_open_ready);
        return DPRX_DRV_NO_ERR;
    }

    DP_VFE_INFO("vfe_dprx_drv_open success\n");

    for(ch = 0; ch < MAX_DPRX_PORT; ch++)
    {
        if (g_p_dprx[ch])
            dprx_adp_open(g_p_dprx[ch]);
    }

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_open);

/*------------------------------------------------
 * Func : vfe_dprx_drv_close
 *
 * Desc : close DPRX driver.
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_close(void)
{
    unsigned char ch;

    DP_VFE_TRACE();

    if (g_vfe_dprx_open_ready==0)
    {
        DP_VFE_ERR("vfe_dprx_drv_close, not open yet (%d)\n", g_vfe_dprx_open_ready);
        return DPRX_DRV_NO_ERR;
    }

    if (g_vfe_dprx_open_ready--)
    {
        DP_VFE_ERR("vfe_dprx_drv_close, open count %d > 0, do not close\n", g_vfe_dprx_open_ready);
        return DPRX_DRV_NO_ERR;
    }

    for(ch = 0; ch < MAX_DPRX_PORT; ch++)
    {
        if (g_p_dprx[ch])
            dprx_adp_close(g_p_dprx[ch]);
    }

    DP_VFE_INFO("vfe_dprx_drv_close success\n");

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_close);

/*------------------------------------------------
 * Func : vfe_dprx_drv_connect
 *
 * Desc : close DPRX driver.
 *
 * Para : ch : DP channel 0~3
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_connect(unsigned char ch)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    DP_VFE_TRACE_EX("ch=%d\n", ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL)
    {
        DP_VFE_ERR("DP[%d] connected failed, no DPRX port detected\n", ch);
        return DPRX_DRV_ARG_ERR;
    }

    if (p_adp == g_connected_dprx)  // already connected
        return DPRX_DRV_NO_ERR;

    if (g_connected_dprx!=NULL)  // disconnect previous channel
        dprx_adp_disconnect(g_connected_dprx);

    g_connected_dprx = p_adp;
    DP_VFE_INFO("DP[%d] connected\n", ch);
    dprx_adp_connect(p_adp);
    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_connect);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_connected_channel
 *
 * Desc : get current connected channel
 *
 * Para : p_ch : current channel output
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_connected_channel(unsigned char* p_ch)
{
    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_ch==NULL || g_connected_dprx==NULL)
        return DPRX_DRV_ARG_ERR;

    *p_ch = g_connected_dprx->ch;
    return DPRX_DRV_NO_ERR;
}

/*------------------------------------------------
 * Func : vfe_dprx_drv_disconnect
 *
 * Desc : disconnect a given DPRX frontend.
 *        after disalbed, DPRX frontend will be decode.
 *
 * Para : ch : DP channel 0~3
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_disconnect(unsigned char ch)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    DP_VFE_TRACE_EX("ch=%d\n", ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp && p_adp == g_connected_dprx)
    {
        DP_VFE_INFO("DP[%d] disconnected\n", ch);

        dprx_adp_disconnect(g_connected_dprx);

        g_connected_dprx = NULL;

        return DPRX_DRV_NO_ERR;
    }
    else
    {
        if (p_adp)
        {
            // Question : Humm... What should we do in this case
            DP_VFE_ERR("DP[%d] disconnect failed, it is not connected DPRX channel\n", ch);
        }
        else
        {
            DP_VFE_ERR("DP[%d] disconnect failed, no DPRX port detected\n", ch);
        }

        return DPRX_DRV_ARG_ERR;
    }
}

EXPORT_SYMBOL(vfe_dprx_drv_disconnect);
/*------------------------------------------------
 * Func : vfe_dprx_drv_set_wakeup_mode
 *
 * Desc : set dprx wakeup mode
 *
 * Para : mode :
 *  DP_WAKEUP_MODE_NO_WAKEUP
 *  DP_WAKEUP_MODE_BY_AUX
 *  DP_WAKEUP_MODE_BY_SIGNAL
 *  DP_WAKEUP_MODE_BY_AUX_OR_SIGNAL
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_ARG_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_set_wakeup_mode(DP_WAKEUP_MODE_E mode)
{
    unsigned char set_mode;
    switch(mode)
    {
        case DP_WAKEUP_MODE_BY_AUX:
            set_mode = WAKEUP_BY_AUX;
            break;

        case DP_WAKEUP_MODE_BY_SIGNAL:
            set_mode = WAKEUP_BY_SIGNAL;
            break;

        case DP_WAKEUP_MODE_BY_AUX_OR_SIGNAL:
            set_mode = (WAKEUP_BY_AUX | WAKEUP_BY_SIGNAL);
            break;

        case DP_WAKEUP_MODE_NO_WAKEUP:
        default:
            set_mode = 0;
            break;
    }
    g_dprx_vfe_suspend_mode = set_mode;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_set_wakeup_mode);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_wakeup_mode
 *
 * Desc : get dprx wakeup mode
 *
 * Para : p_mode : wakeup mode output
 *  DP_WAKEUP_MODE_NO_WAKEUP
 *  DP_WAKEUP_MODE_BY_AUX
 *  DP_WAKEUP_MODE_BY_SIGNAL
 *  DP_WAKEUP_MODE_BY_AUX_OR_SIGNAL
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_ARG_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_wakeup_mode(DP_WAKEUP_MODE_E* p_mode)
{
    if (p_mode==NULL)
        return DPRX_DRV_ARG_ERR;

    if ((g_dprx_vfe_suspend_mode & (WAKEUP_BY_AUX|WAKEUP_BY_SIGNAL))==(WAKEUP_BY_AUX|WAKEUP_BY_SIGNAL))
        *p_mode = DP_WAKEUP_MODE_BY_AUX_OR_SIGNAL;
    else if ((g_dprx_vfe_suspend_mode & WAKEUP_BY_AUX))
        *p_mode = DP_WAKEUP_MODE_BY_AUX;
    else if ((g_dprx_vfe_suspend_mode & WAKEUP_BY_SIGNAL))
        *p_mode = DP_WAKEUP_MODE_BY_SIGNAL;
    else
        *p_mode = 0;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_wakeup_mode);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_rx_port_cnt
 *
 * Desc : get number of rx port
 *
 * Para : port_cnt : rx port output
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_ARG_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_rx_port_cnt(unsigned char* p_port_cnt)
{
    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_port_cnt==NULL)
        return DPRX_DRV_ARG_ERR;

    *p_port_cnt = g_vfe_dprx_port_count;
    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_rx_port_cnt);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_rx_port_capability
 *
 * Desc : get capability of rx port
 *
 * Para : ch : DP channel 0~3
 *        p_cap : DP port capability output
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_rx_port_capability(unsigned char ch, DP_PORT_CAPABILITY* p_cap)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_cap==NULL || p_adp ==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_get_capability(p_adp, p_cap)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_rx_port_capability);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_link_status
 *
 * Desc : get link status of rx port
 *
 * Para : ch : DP channel 0~3
 *        p_status : link status output
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_link_status(unsigned char ch, DP_LINK_STATUS_T* p_status)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_status==NULL || p_adp ==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_get_link_status(p_adp, p_status)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_link_status);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_stream_type
 *
 * Desc : get stream type of rx port
 *
 * Para : ch : DP channel 0~3
 *        p_type : stream type output
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_stream_type(unsigned char ch, DP_STREAM_TYPE_E* p_type)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_type==NULL || p_adp ==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_get_stream_type(p_adp, p_type)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_stream_type);

//===============================================================================
// HPD / DET related
//===============================================================================


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_connection_state
 *
 * Desc : get cable connect state of a DPRX port
 *
 * Para : ch : DPRX channel
 *        p_connect_state : connect state output
 *           0 : disconnect, others : connect
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_connection_state(unsigned char ch, unsigned char *p_connect_state)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    DP_VFE_TRACE_EX("ch=%d\n", ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_connect_state==NULL || p_adp ==NULL)
        return DPRX_DRV_ARG_ERR;

    *p_connect_state = dprx_adp_get_connect_status(p_adp);
    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_connection_state);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_aux_state
 *
 * Desc : get cable aux statue.
 *        for DP, it means PD Aux P/N is at correct level
 *        for TypeC, it mens TypeC enter DP alt mode
 *
 * Para : p_dev : handle of DPRX adapter
 *        p_aux_state :
 *           0 : aux is not ready to use
 *           1 : aux is ready to use
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_aux_state(unsigned char ch, unsigned char *p_aux_state)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_aux_state==NULL || p_adp ==NULL)
        return DPRX_DRV_ARG_ERR;

    *p_aux_state = dprx_adp_get_aux_status(p_adp);
    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_aux_state);

/*------------------------------------------------
 * Func : vfe_dprx_drv_enable_hpd
 *
 * Desc : enable HPD of all DPRX port
 *
 * Para : on : 0 : disable hpd, others : enable hpd
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_enable_hpd(unsigned char on)
{
    unsigned char ch;

    DP_VFE_TRACE_EX("on=%d\n", on);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    for(ch = 0; ch < MAX_DPRX_PORT; ch++)
    {
        if (g_p_dprx[ch])
            dprx_adp_set_hpd(g_p_dprx[ch], on);
    }

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_enable_hpd);

/*------------------------------------------------
 * Func : vfe_dprx_drv_set_hpd
 *
 * Desc : enable / disable HPD of a DPRX port
 *
 * Para : ch : DPRX channel
 *        on : 0 : disable hpd, others : enable hpd
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_set_hpd(unsigned char ch, unsigned char on)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    DP_VFE_TRACE_EX("ch=%d, on=%d\n", ch, on);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL)
        return DPRX_DRV_ARG_ERR;

    dprx_adp_set_hpd(p_adp, on);
    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_set_hpd);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_hpd
 *
 * Desc : get HPD status of a DPRX port
 *
 * Para : ch : DPRX channel
 *        p_hpd_state : hpd state output
 *           0 : hpd disabled,
 *           others : hpd enabled
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_hpd(
    unsigned char   ch,
    unsigned char*  p_hpd_state
    )
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL || p_hpd_state==NULL)
        return DPRX_DRV_ARG_ERR;

    *p_hpd_state = dprx_adp_get_hpd(p_adp);

    return DPRX_DRV_NO_ERR;
}


/*------------------------------------------------
 * Func : vfe_dprx_drv_hpd_toggle
 *
 * Desc : do HPD toggle on a given DPRX port
 *
 * Para : ch : DPRX channel
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_hpd_toggle(unsigned char ch)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    DP_VFE_TRACE_EX("ch=%d\n", ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL)
        return DPRX_DRV_ARG_ERR;

    dprx_adp_set_hpd(p_adp, 0);
    dprx_osal_msleep(10);
    dprx_adp_set_hpd(p_adp, 1);

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_hpd_toggle);

/*------------------------------------------------
 * Func : vfe_dprx_drv_hpd_toggle_ex
 *
 * Desc : do HPD toggle on a given DPRX port
 *
 * Para : ch : DPRX channel
 *        delay_ms  : the Interval of HPD toggle
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_hpd_toggle_ex(unsigned char ch, unsigned short delay_ms)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    DP_VFE_TRACE_EX("ch=%d\n", ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL)
        return DPRX_DRV_ARG_ERR;

    dprx_adp_set_hpd_toggle(p_adp, delay_ms);

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_hpd_toggle_ex);

/*------------------------------------------------
 * Func : vfe_dprx_drv_set_hpd_low_delay
 *
 * Desc : set hpd low delay
 *
 * Para : delay_ms : hpd low delay ms
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_set_hpd_low_delay(unsigned int delay_ms)
{
    // TODO : store hpd low delay here
    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_set_hpd_low_delay);

//===============================================================================
// EDID related
//===============================================================================


/*------------------------------------------------
 * Func : vfe_dprx_drv_read_edid
 *
 * Desc : read edid from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_edid : edid data
 *        len : edid len
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_read_edid(unsigned char ch, unsigned char* p_edid, unsigned short len)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    DP_VFE_TRACE_EX("ch=%d\n", ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL || p_edid==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_get_edid(p_adp, p_edid, len)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_read_edid);

 /*------------------------------------------------
 * Func : vfe_dprx_drv_write_edid
 *
 * Desc : write edid to a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_edid : edid data
 *        len : edid len
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_write_edid(unsigned char ch, const unsigned char* p_edid, unsigned short len)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    DP_VFE_TRACE_EX("ch=%d\n", ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL || p_edid==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_set_edid(p_adp, (unsigned char*) p_edid, len)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_write_edid);

//===============================================================================
// DPCD related
//===============================================================================

/*------------------------------------------------
 * Func : vfe_dprx_drv_read_dpcd
 *
 * Desc : read dpcd from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        address : dpcd address
 *        p_dpcd : dpcd output
 *        len : dpcd len
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_read_dpcd(
    unsigned char   ch,
    unsigned long   address,
    unsigned char*  p_dpcd,
    unsigned short  len
    )
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    DP_VFE_TRACE_EX("ch=%d\n", ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL || p_dpcd==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_get_dpcd(p_adp, address, (unsigned char*) p_dpcd, len)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

/*------------------------------------------------
 * Func : vfe_dprx_drv_write_dpcd
 *
 * Desc : write dpcd from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        address : dpcd address
 *        p_dpcd : dpcd to be written
 *        len : dpcd len
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_write_dpcd(
    unsigned char   ch,
    unsigned long   address,
    unsigned char*  p_dpcd,
    unsigned short  len
    )
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    DP_VFE_TRACE_EX("ch=%d\n", ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL || p_dpcd==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_set_dpcd(p_adp, address, (unsigned char*) p_dpcd, len)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_vbid_info
 *
 * Desc : get vbid information from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_vbid : vbid info
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_port_vbid_info(
    unsigned char       ch,
    DPRX_VBID_INFO_T*   p_vbid
    )
{
     dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL || p_vbid==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_get_vbid_info(p_adp, p_vbid)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

//===============================================================================
// MSA related
//===============================================================================


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_msa_info
 *
 * Desc : get msa information from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_msa : msa info
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_port_msa_info(
    unsigned char    ch,
    DPRX_MSA_INFO_T* p_msa
    )
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL || p_msa==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_get_msa_info(p_adp, p_msa)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_sdp_data
 *
 * Desc : get sdp data from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        type   : SDP type
 *        p_data : spd data output
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int vfe_dprx_drv_get_port_sdp_data(
    unsigned char   ch,
    DP_SDP_TYPE     type,
    DP_SDP_DATA_T*  p_data
    )
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL || p_data==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_get_sdp_data(p_adp, type, p_data)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}


//===============================================================================
// Video related
//===============================================================================

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_avi_info
 *
 * Desc : get avi information from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_avi_info : avi info
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_port_avi_info(unsigned char ch, DPRX_AVI_INFO_T *p_avi_info)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL || p_avi_info==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_get_avi_info(p_adp, p_avi_info)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_port_avi_info);


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_drm_info
 *
 * Desc : get drm information from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_drm_info : drm info
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_port_drm_info(unsigned char ch, DPRX_DRM_INFO_T *p_drm_info)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL || p_drm_info==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_get_drm_info(p_adp, p_drm_info)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_port_drm_info);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_timing_info
 *
 * Desc : write edid to a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_timing_info : timing info
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_port_timing_info(unsigned char ch, DPRX_TIMING_INFO_T *p_timing_info)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    DP_VFE_TRACE_EX("ch=%d\n", ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL || p_timing_info==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_get_video_timing(p_adp, p_timing_info)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_port_timing_info);

#ifdef CONFIG_DPRX_VFE_ENABLE_ONLINE_MEASURE_ERROR

/*------------------------------------------------
 * Func : vfe_dprx_drv_handle_online_measure_error
 *
 * Desc : handle online measure error
 *
 * Para : flags : ertra control flag
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_handle_online_measure_error(unsigned char flags)
{
    DP_VFE_TRACE_EX("flags=%d\n", flags);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (g_connected_dprx)
    {
        dprx_adp_set_ext_ctrl(g_connected_dprx, ADP_EXCTL_WATCH_DOG_ENABLE, 0); // Disable Video Watch Dog
        dprx_adp_event_handler(g_connected_dprx, DPRX_ADP_EVENT_ONLINE_MEASURE_FAIL); // send event to current dprx port
    }

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_handle_online_measure_error);

#endif

/*------------------------------------------------
 * Func : vfe_dprx_drv_source_check
 *
 * Desc : Check input source
 *
 * Para : ch : DP channel 0~3
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_source_check(unsigned char ch)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    DP_VFE_TRACE_EX("ch=%d\n", ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp ==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_source_check(p_adp)<0)
        return DPRX_DRV_HW_ERR;
    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_source_check);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_avmute_status
 *
 * Desc : write edid to a given DPRX Port
 *
 * Para : p_avmute : AV mute status
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_avmute_status(unsigned char* p_avmute)
{
    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_avmute==NULL)
        return DPRX_DRV_ARG_ERR;

    *p_avmute = 0;

    if (g_connected_dprx)
        *p_avmute = dprx_adp_get_avmute(g_connected_dprx);

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_avmute_status);


//===============================================================================
// Audio related
//===============================================================================


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_audio_coding_type
 *
 * Desc : get audio coding status of current DPRX Port
 *
 * Para : p_audio_status : audio status
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_audio_status(DP_AUDIO_STATUS_T *p_audio_status)
{
    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_audio_status==NULL)
        return DPRX_DRV_ARG_ERR;

    if (g_connected_dprx)
    {
        dprx_adp_get_audio_status(g_connected_dprx, p_audio_status);
    }

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_audio_status);

//===============================================================================
// HDCP related
//===============================================================================


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_hdcp_capability
 *
 * Desc : get hdcp capability pf DPRX VFE
 *
 * Para : p_hdcp_cap : bit mask of HDCP capability
 *          DP_HDCP_CAPBILITY_HDCP13
 *          DP_HDCP_CAPBILITY_HDCP22
 *	        DP_HDCP_CAPBILITY_HDCP23
 *          DP_HDCP_CAPBILITY_HDCP_REPEATER
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_hdcp_capability(unsigned int *p_hdcp_cap)
{
    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_hdcp_cap==NULL)
        return DPRX_DRV_ARG_ERR;

    *p_hdcp_cap = DP_HDCP_CAPBILITY_HDCP13 | DP_HDCP_CAPBILITY_HDCP22 | DP_HDCP_CAPBILITY_HDCP23;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_hdcp_capability);

/*------------------------------------------------
 * Func : vfe_dprx_drv_write_hdcp_key
 *
 * Desc : write hdcp key
 *
 * Para : p_key : hdcp  1.x/2.x key
 *        apply : immediate apply
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_write_hdcp_key(DPRX_HDCP_KEY_T * p_key, unsigned char apply)
{
    // TODO : store key to hdcp manager

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_write_hdcp_key);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_hdcp_capability
 *
 * Desc : get hdcp capability pf DPRX VFE
 *
 * Para : ch : DPRX channel
 *        p_hdcp_status : hdcp status
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_hdcp_status(unsigned char ch, DPRX_HDCP_STATUS_T *p_hdcp_status)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    if (p_adp==NULL || p_hdcp_status==NULL)
        return DPRX_DRV_ARG_ERR;

    if (dprx_adp_get_hdcp_status(p_adp, p_hdcp_status)<0)
        return DPRX_DRV_HW_ERR;

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_get_hdcp_status);

//===============================================================================
// Extension Control
//===============================================================================
/*------------------------------------------------
 * Func : vfe_dprx_drv_set_ext_ctrl
 *
 * Desc : set ext control of a DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_ext_ctrl : ext_ctrl
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_set_ext_ctrl(unsigned char ch, DP_EXT_CTRL* p_ext_ctrl)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);
    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;
    if (p_adp==NULL)
        return DPRX_DRV_ARG_ERR;
    if (dprx_adp_set_ext_ctrl(p_adp, (ADP_EXCTL_ID)p_ext_ctrl->id, p_ext_ctrl->param1)<0)
        return DPRX_DRV_HW_ERR;
    return DPRX_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_dprx_drv_set_ext_ctrl);
/*------------------------------------------------
 * Func : vfe_dprx_drv_get_ext_ctrl
 *
 * Desc : get ext control of a DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_ext_ctrl : ext_ctrl
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_ext_ctrl(unsigned char ch, DP_EXT_CTRL* p_ext_ctrl)
{
    dprx_dev_t* p_adp = _vfe_dprx_drv_get_dprx_adapter(ch);
    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;
    if (p_adp==NULL)
        return DPRX_DRV_ARG_ERR;
    p_ext_ctrl->param1 = dprx_adp_get_ext_ctrl(p_adp, (ADP_EXCTL_ID)p_ext_ctrl->id);
    return DPRX_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_dprx_drv_get_ext_ctrl);

//===============================================================================
// Power Management related
//===============================================================================
#ifdef CONFIG_PM

/*------------------------------------------------
 * Func : vfe_dprx_drv_suspend
 *
 * Desc : suspend
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_suspend(void)
{
    unsigned char ch;

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    for(ch = 0; ch < MAX_DPRX_PORT; ch++)
    {
        if (g_p_dprx[ch])
            dprx_adp_suspend(g_p_dprx[ch], g_dprx_vfe_suspend_mode);
    }

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_suspend);

/*------------------------------------------------
 * Func : vfe_dprx_drv_resume
 *
 * Desc : suspend
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_resume(void)
{
   unsigned char ch;

    if (!CHK_DPRX_VFE_OPEN_STATE())
        return DPRX_DRV_STATE_ERR;

    for(ch = 0; ch < MAX_DPRX_PORT; ch++)
    {
        if (g_p_dprx[ch])
            dprx_adp_resume(g_p_dprx[ch]);
    }

    return DPRX_DRV_NO_ERR;
}

EXPORT_SYMBOL(vfe_dprx_drv_resume);

#endif
