/*=============================================================
 * File:    dprx_adapter-rtk_dprx-wrapper.c
 *
 * Desc:    driver wrapper for DPRX low level driver
 *
 * AUTHOR:  kevin_wang@realtek.com
 *
 * Vresion: 0.0.1
 *
 *------------------------------------------------------------
 * Copyright (c) Realtek Semiconductor Corporation, 2022
 *
 * All rights reserved.
 *
 *============================================================*/
#include <dprx_platform.h>
#include <dprx_adapter-rtk_dprx.h>
#include <dprx_adapter-rtk_dprx-priv.h>
#ifdef CONFIG_OPTEE_HDCP14
#include <hdcp/hdcp14_optee.h>
#endif
#ifdef CONFIG_OPTEE_HDCP2
#include <hdcp/hdcp2_optee.h>
#endif

extern RTK_DPRX_ADP* g_p_current_dprx_port;
extern edid_dev_t*   g_p_dprx_edid;
extern UINT8 g_mute_if_lane_dealign;
static dprx_hdcp2_message* hdcp2_handle_msg;

//--------------------------------------------------
// Func  : dprx_ext_drvif_set_hotplug
//
// Desc  : set hpd level
//
// Param : port : dprx port
//         on   : 0 : hpd low, others : hpd high
//
// Retn  : N/A
//--------------------------------------------------
void dprx_ext_drvif_set_hotplug(
    UINT8           port,
    UINT8           on
    )
{
    if (g_p_current_dprx_port && g_p_current_dprx_port->p_hpd)
        dprx_hpd_set_hpd(g_p_current_dprx_port->p_hpd, on);
}

//--------------------------------------------------
// Func  : dprx_ext_drvif_set_irq_hotplug
//
// Desc  : set irq hpd
//
// Param : port : dprx port
//
// Retn  : N/A
//--------------------------------------------------
void dprx_ext_drvif_set_irq_hotplug(
    UINT8           port
    )
{
    rtk_dprx_set_irq_hpd(g_p_current_dprx_port);
}


//--------------------------------------------------
// Func  : dprx_ext_drvif_get_hotplug_status
//
// Desc  : get hpd level
//
// Param : port : dprx port
//
// Retn  : 0 : hpd low, others : hpd high
//--------------------------------------------------
UINT8 dprx_ext_drvif_get_hotplug_status(
    UINT8           port
    )
{
    UINT8 ret = 0;

    if (g_p_current_dprx_port && g_p_current_dprx_port->p_hpd)
        ret = dprx_hpd_get_hpd(g_p_current_dprx_port->p_hpd);

    return ret;
}


//--------------------------------------------------
// Func  : dprx_ext_drvif_get_cable_status
//
// Desc  : get cable connect status
//
// Param : port : dprx port
//
// Retn  : 0 : cable disconnect, others : cable connected
//--------------------------------------------------
UINT8 dprx_ext_drvif_get_cable_status(
    UINT8           port
    )
{
    return (rtk_dprx_get_connect_status(g_p_current_dprx_port)) ? 1 : 0;
}


//--------------------------------------------------
// Func  : dprx_ext_drvif_ddc_reset
//
// Desc  : do ddc reset
//
// Param : port : dprx port
//
// Retn  : N/A
//--------------------------------------------------
void dprx_ext_drvif_ddc_reset(
    UINT8           port
    )
{
    if (g_p_dprx_edid)
        dprx_edid_reset(g_p_dprx_edid);
}



//--------------------------------------------------
// Func  : dprx_ext_drvif_set_hotplug_exint0
//
// Desc  : set hpd level
//
// Param : port : dprx port
//         on   : 0 : hpd low, others : hpd high
//
// Retn  : N/A
//--------------------------------------------------
void dprx_ext_drvif_set_hotplug_exint0(
    UINT8           port,
    UINT8           on
    )
{
    dprx_ext_drvif_set_hotplug(port, on);
}

//--------------------------------------------------
// Func  : dprx_ext_drvif_set_irq_hotplug_exint0
//
// Desc  : set irq hpd
//
// Param : port : dprx port
//
// Retn  : N/A
//--------------------------------------------------
void dprx_ext_drvif_set_irq_hotplug_exint0(
    UINT8           port
    )
{
    dprx_ext_drvif_set_irq_hotplug(port);
}



//--------------------------------------------------
// Func  : dprx_ext_drvif_get_hotplug_status_exint0
//
// Desc  : get hpd level
//
// Param : port : dprx port
//
// Retn  : 0 : hpd low, others : hpd high
//--------------------------------------------------
UINT8 dprx_ext_drvif_get_hotplug_status_exint0(
    UINT8           port
    )
{
    return dprx_ext_drvif_get_hotplug_status(port);
}


//--------------------------------------------------
// Func  : dprx_ext_drvif_hdcp2_command
//
// Desc  : do hdcp2 handler
//
// Param : port : hdcp port id
//         send_buf : Send packet of hdcp messsage include command id and buffer.
//         send_size : packet size.
//         rev_buf : Recive packet.
//         rev_size : Recive packet size.
//
// Retn  : N/A
//--------------------------------------------------
void dprx_ext_drvif_hdcp2_command(
    UINT8 port,
    UINT8 *send_buf,
    UINT32 send_size,
    UINT8 *rev_buf,
    UINT32 rev_size)
{
    //Fixed me
}

//--------------------------------------------------
// Func  : dprx_ext_drvif_hdcp2_command_exint0
//
// Desc  : do hdcp2 handler for exint0
//
// Param : port : hdcp port id
//         send_buf : Send packet of hdcp messsage include command id and buffer.
//         send_size : packet size.
//         rev_buf : Recive packet.
//         rev_size : Recive packet size.
//
// Retn  : N/A
//--------------------------------------------------
void dprx_ext_drvif_hdcp2_command_exint0(
    UINT8 port,
    UINT8 *send_buf,
    UINT32 send_size,
    UINT8 *rev_buf,
    UINT32 rev_size)
{
    //Fixed me
}

//--------------------------------------------------
// Func  : dprx_ext_drvif_get_hdcp2_memory_exint0
//
// Desc  : get hdcp2 memory.
//
// Param : N/A
//
// Retn  : cert_rx (522 Bytes)
//--------------------------------------------------
dprx_hdcp2_message* dprx_ext_drvif_get_hdcp2_memory_exint0(void)
{
    if(hdcp2_handle_msg == NULL)
        hdcp2_handle_msg = dprx_osal_malloc(sizeof(dprx_hdcp2_message));

    return hdcp2_handle_msg;
}

//--------------------------------------------------
// Func  : dprx_ext_drvif_get_hdcp2_memory
//
// Desc  : get hdcp2 memory.
//
// Param : N/A
//
// Retn  : cert_rx (522 Bytes)
//--------------------------------------------------
dprx_hdcp2_message* dprx_ext_drvif_get_hdcp2_memory(void)
{
    if(hdcp2_handle_msg == NULL)
        hdcp2_handle_msg = dprx_osal_malloc(sizeof(dprx_hdcp2_message));

    return hdcp2_handle_msg;
}

//--------------------------------------------------
// Func  : dprx_ext_drvif_hdcp2_cmd
//
// Desc  : do hdcp2 handler
//
// Param : port : dprx port
//         dprx_hdcp2_message : hdcp2 msg
//
// Retn  : N/A
//--------------------------------------------------
void dprx_ext_drvif_hdcp2_cmd(UINT8 port, dprx_hdcp2_message* msg)
{
#ifdef CONFIG_OPTEE_HDCP2
    if(*(msg->tx_msg_buf) == 8)
        msg->msg_id = HDCP2_CMD_POLLING_MESSAGE;
    else
        msg->msg_id = HDCP2_CMD_HANDLE_MESSAGE;

    optee_hdcp2_main(DP_OPTEE_HDCP2_MODE, msg->msg_id, port, msg->tx_msg_buf, msg->tx_length, msg->rx_msg_buf, msg->rx_length);
#endif
}

//--------------------------------------------------
// Func  : dprx_ext_drvif_hdcp2_cmd_exint0
//
// Desc  : do hdcp2 handler for exint0
//
// Param : port : dprx port
//         dprx_hdcp2_message : hdcp2 msg
//
// Retn  : N/A
//--------------------------------------------------
void dprx_ext_drvif_hdcp2_cmd_exint0(UINT8 port, dprx_hdcp2_message* msg)
{
#ifdef CONFIG_OPTEE_HDCP2
    if(*(msg->tx_msg_buf) == 8)
        msg->msg_id = HDCP2_CMD_POLLING_MESSAGE;
    else
        msg->msg_id = HDCP2_CMD_HANDLE_MESSAGE;

    optee_hdcp2_main(DP_OPTEE_HDCP2_MODE, msg->msg_id, port, msg->tx_msg_buf, msg->tx_length, msg->rx_msg_buf, msg->rx_length);
#endif
}

//--------------------------------------------------
// Func  : dprx_ext_drvif_hdcp2_loadkey
//
// Desc  : load hdcp 2.2 key
//
// Param : N/A
//
// Retn  : N/A
//--------------------------------------------------
void dprx_ext_drvif_hdcp2_loadkey(void)
{
#ifdef CONFIG_OPTEE_HDCP2
    UINT8 uc_lc128[16];
    optee_hdcp2_main(DP_OPTEE_HDCP2_MODE, HDCP2_CMD_LOAD_KEY, 0, uc_lc128, 16, uc_lc128, 16);
#endif
}

//--------------------------------------------------
// Func  : dprx_ext_drvif_hdcp_loadkey
//
// Desc  : load hdcp 1.4 key
//
// Param : N/A
//
// Retn  : N/A
//--------------------------------------------------
void dprx_ext_drvif_hdcp_loadkey(void)
{
#ifdef CONFIG_OPTEE_HDCP14
    optee_hdcp14_load_key(DP_OPTEE_HDCP_MODE, 1);
#endif
}


#ifdef DPRX_ENABLE_EXT_EVENT_HANDLER

//--------------------------------------------------
// Func  : _proc_link_event
//
// Desc  : dprx link event handler
//
// Param : N/A
//
// Retn  : N/A
//--------------------------------------------------
static void _proc_link_event(
    UINT8           port,
    DPRX_EVENT_T    event_id
    )
{
    //DPRX_ADP_WARN("_proc_link_event (event=%04x)!!!\n", event_id);
    switch(GET_EVENT(event_id))
    {
    case LINK_TARIN_STATUS_UPDATE:
        if (g_p_current_dprx_port)
            rtk_dprx_handle_lt_state_update_event(g_p_current_dprx_port, GET_EVENT_PARM(event_id));
        break;

    case LINK_STATUS_UPDATE:
        if (g_mute_if_lane_dealign)
        {
            DPRX_ADP_WARN("lane align failed, do avmute!!!\n");
            dprx_drv_do_avmute(0);  // mute video
        }
        else
        {
            DPRX_ADP_WARN("lane align failed, do nothing!!!\n");
        }
        break;

    case LINK_REQUEST_HPD_IRQ:
        if (g_p_current_dprx_port)
            rtk_dprx_set_irq_hpd(g_p_current_dprx_port);
        break;
    default:
        break;
    }
}

//--------------------------------------------------
// Func  : _proc_sdp_event
//
// Desc  : dprx link event handler
//
// Param : N/A
//
// Retn  : N/A
//--------------------------------------------------
static void _proc_sdp_event(
    UINT8           port,
    DPRX_EVENT_T    event_id
    )
{
    DPRX_ADP_DBG("_proc_sdp_event (event=%04x)!!!\n", event_id);

    switch(GET_EVENT(event_id))
    {
    case SDP_UPDATE:
        rtk_dprx_handle_infoframe_sdp_update_event(g_p_current_dprx_port, GET_EVENT_PARM(event_id));
        break;

    case SDP_CHANGE:
        break;

    default:
        // do nothing
        break;
    }
}

//--------------------------------------------------
// Func  : _proc_hdcp_event
//
// Desc  : dprx link event handler
//
// Param : N/A
//
// Retn  : N/A
//--------------------------------------------------
static void _proc_hdcp_event(
    UINT8           port,
    DPRX_EVENT_T    event_id
    )
{
    DPRX_ADP_WARN("_proc_hdcp_event (event=%04x)!!!\n", event_id);
    // do nothing
}

//--------------------------------------------------
// Func  : dprx_ext_drvif_event_handler
//
// Desc  : dprx event handler
//
// Param : N/A
//
// Retn  : N/A
//--------------------------------------------------
void dprx_ext_drvif_event_handler(
    UINT8           port,
    DPRX_EVENT_T    event_id
    )
{
    //DPRX_ADP_WARN("dprx_ext_drvif_event_handler (event=%04x)!!!\n", event);

    switch(GET_EVENT_TYPE(event_id))
    {
    case DPRX_EVENT_TYPE_LINK:
        _proc_link_event(port, event_id);
        break;
    case DPRX_EVENT_TYPE_SDP:
        _proc_sdp_event(port, event_id);
        break;
    case DPRX_EVENT_TYPE_HDCP:
        _proc_hdcp_event(port, event_id);
        break;
    default:
        // do nothing
        break;
    }
}

#endif
