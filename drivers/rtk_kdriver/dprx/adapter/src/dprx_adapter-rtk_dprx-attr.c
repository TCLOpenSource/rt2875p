/*=============================================================
 * File:    dprx_adapter-rtk_dprx-priv.c
 *
 * Desc:    Internal driver for RTK DPRX adapter
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
#include <dprx_adapter-rtk_dprx-plat.h>

extern RTK_DPRX_ADP* g_p_current_dprx_port;
extern const char* _get_dprx_vfsm_str(UINT8 vfsm);

#ifdef CONFIG_DPRX_EANBLE_FREESYNC
extern UINT8 g_force_freesync_off;
#endif

extern UINT8 g_force_max_link_rate;
extern UINT8 g_force_fake_tps1_lt;  // disabled
extern UINT8 g_force_demode;
extern UINT8 g_dprx_detected_status;
extern DPRX_SDP_HANDLER g_dprx_sdp_handler;
extern UINT8 g_mute_if_lane_dealign;


/*------------------------------------------------
 * Func : rtk_dprx_attr_show_status
 *
 * Desc : show adapter status
 *
 * Para : p_dprx : handle of DPRX adapter
 *        p_buf  : data buffer
 *        cnt    : buffer size
 *
 * Retn : number if bytes handled
 *-----------------------------------------------*/
int rtk_dprx_attr_show_status(
    RTK_DPRX_ADP*   p_dprx,
    UINT8*          p_buf,
    UINT32          count
    )
{
    UINT32 n;
    UINT8 *ptr = p_buf;

    //---------------------------------------
    // USB ALT
    //---------------------------------------
    if (p_dprx->type == DP_TYPE_USB_TYPE_C && p_dprx->p_hpd)
    {
        DPRX_CABLE_CFG cable_cfg;
        dprx_hpd_get_cable_config(p_dprx->p_hpd, &cable_cfg);

        n = snprintf(ptr, count, "TYPEC : DP alt mode=%d, cable_flip=%d, config=%x\n",
                dprx_hpd_get_aux_status(p_dprx->p_hpd, NULL),
                (cable_cfg & 0x80) ? 1 : 0,
                (cable_cfg & 0xF));
        ptr+=n; count-=n;
    }

    if (g_p_current_dprx_port==p_dprx || p_dprx->connected)
    {
#ifdef VFSM_0_WAIT_LINK_READY
        //---------------------------------------
        // VFSM Status
        //---------------------------------------
        n = snprintf(ptr, count, "VFSM  : fsm=%d(%s)\n", GET_VIDEO_FSM(), _get_dprx_vfsm_str(GET_VIDEO_FSM()));
        ptr+=n; count-=n;
#endif
    }

    return ptr - p_buf;
}



/*------------------------------------------------
 * Func : rtk_dprx_attr_show_dpcd
 *
 * Desc : show adapter dpcd
 *
 * Para : p_dprx : handle of DPRX adapter
 *        p_buf  : data buffer
 *        cnt    : buffer size
 *
 * Retn : number if bytes handled
 *-----------------------------------------------*/
int rtk_dprx_attr_show_dpcd(
    RTK_DPRX_ADP*   p_dprx,
    UINT8*          p_buf,
    UINT32          count
    )
{
    UINT32 n;
    UINT8 *ptr = p_buf;
    UINT8 buff[16];

    if (p_dprx==g_p_current_dprx_port)
    {
        //---------------------------------------
        // Receiver Capability
        //---------------------------------------
        memset(buff, 0, sizeof(buff));
        lib_dprx_dpcd_read_bytes(0, DPCD_00000_DPCD_REV, buff, 12);
        n = scnprintf(ptr, count, "\n- Receiver Capability\n00000h-0000Bh : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n\n",
            buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7],buff[8],buff[9],buff[10],buff[11]);
        ptr+=n; count-=n;

        //---------------------------------------
        // Link Config
        //---------------------------------------
        memset(buff, 0, sizeof(buff));
        lib_dprx_dpcd_read_bytes(0, 0x00100, buff, 8);
        n = scnprintf(ptr, count, "- Link Config \n00100h-00108h : %02x %02x %02x %02x %02x %02x %02x %02x %02x\n\n",
            buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7],buff[8]);
        ptr+=n; count-=n;

        //---------------------------------------
        // Device Status
        //---------------------------------------
        memset(buff, 0, sizeof(buff));
        lib_dprx_dpcd_read_bytes(0, 0x00200, buff, 12);
        n = scnprintf(ptr, count, "- Device Status\n00200h-0020Bh : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n\n",
            buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7],buff[8],buff[9],buff[10],buff[11]);
        ptr+=n; count-=n;

        //---------------------------------------
        // Source Device Specific
        //---------------------------------------
        memset(buff, 0, sizeof(buff));
        lib_dprx_dpcd_read_bytes(0, DPCD_00300_IEEE_OUI_1, buff, 12);
        n = scnprintf(ptr, count, "- Source Device Specific\n00400h-0040Bh : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n\n",
            buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7],buff[8],buff[9],buff[10],buff[11]);
        ptr+=n; count-=n;

        //---------------------------------------
        // Sink Device Specific
        //---------------------------------------
        memset(buff, 0, sizeof(buff));
        lib_dprx_dpcd_read_bytes(0, DPCD_00400_IEEE_OUI_1, buff, 12);
        n = scnprintf(ptr, count, "- Sink Device Specific\n00400h-0040Bh : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n\n",
            buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7],buff[8],buff[9],buff[10],buff[11]);
        ptr+=n; count-=n;

        //---------------------------------------
        // Power
        //---------------------------------------
        memset(buff, 0, sizeof(buff));
        lib_dprx_dpcd_read_bytes(0, 0x00600, buff, 1);
        n = scnprintf(ptr, count, "- Device Power Control\n00600h-00600h : %02x \n\n", buff[0]);
        ptr+=n; count-=n;

        //---------------------------------------
        // EXT Receiver Capability
        //---------------------------------------
        memset(buff, 0, sizeof(buff));
        lib_dprx_dpcd_read_bytes(0, 0x002200, buff, 12);
        n = scnprintf(ptr, count, "- EXT Receiver Capability\n02200h-0220Bh : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n\n",
            buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7],buff[8],buff[9],buff[10],buff[11]);
        ptr+=n; count-=n;
    }

    return ptr - p_buf;
}


/*------------------------------------------------
 * Func : rtk_dprx_attr_show_debug_ctrl
 *
 * Desc : show debug control attr
 *
 * Para : p_dprx : handle of DPRX adapter
 *        p_buf  : data buffer
 *        cnt    : buffer size
 *
 * Retn : number if bytes handled
 *-----------------------------------------------*/
int rtk_dprx_attr_show_debug_ctrl(
    RTK_DPRX_ADP*   p_dprx,
    UINT8*          p_buf,
    UINT32          count
    )
{
    UINT32 n;
    UINT8 *ptr = p_buf;

    n = snprintf(ptr, count, "force_max_link_rate=0x%02x (Off: 0, RBR: %x, HBR: %x, HBR2:%x, HBR3:%x)\n",
        g_force_max_link_rate, _DP_LINK_RBR, _DP_LINK_HBR, _DP_LINK_HBR2, _DP_LINK_HBR3);
    ptr+=n; count-=n;

#ifdef DPRX_SINK_CAP_LINK_TRAIN_CTRL_FAKE_TP1_LT
    n = snprintf(ptr, count, "force_fake_tps1_lt=%d\n", g_force_fake_tps1_lt);
    ptr+=n; count-=n;
#endif

    n = snprintf(ptr, count, "reset_vfsm=0\n");
    ptr+=n; count-=n;

    n = snprintf(ptr, count, "force_demode=%d\n", g_force_demode);
    ptr+=n; count-=n;

#ifdef CONFIG_DPRX_EANBLE_FREESYNC
    n = snprintf(ptr, count, "force_freesync_off=%d\n", g_force_freesync_off);
    ptr+=n; count-=n;
#endif

    n = snprintf(ptr, count, "dump_dolby_vision_sdp=%d\n", (g_dprx_sdp_debug_ctrl & DEBUG_DOLBY_VIOSN_SDP) ? 1 : 0);
    ptr+=n; count-=n;

    n = snprintf(ptr, count, "dump_amd_freesync_sdp=%d\n", (g_dprx_sdp_debug_ctrl & DEBUG_AMD_FREESYNC_SPD_SDP) ? 1 : 0);
    ptr+=n; count-=n;

    n = snprintf(ptr, count, "dump_vsc_ext_vesa_sdp=%d\n", (g_dprx_sdp_debug_ctrl & DEBUG_VSC_EXT_VESA_SDP) ? 1 : 0);
    ptr+=n; count-=n;

    n = snprintf(ptr, count, "mute_if_lane_dealign=%d\n", (g_mute_if_lane_dealign) ? 1 : 0);
    ptr+=n; count-=n;

    return ptr - p_buf;
}


/*------------------------------------------------
 * Func : rtk_dprx_attr_store_debug_ctrl
 *
 * Desc : store debug control attr
 *
 * Para : p_dprx : handle of DPRX adapter
 *        p_buf  : data buffer
 *        cnt    : buffer size
 *
 * Retn : number if bytes handled
 *-----------------------------------------------*/
int rtk_dprx_attr_store_debug_ctrl(
    RTK_DPRX_ADP*   p_dprx,
    UINT8*          p_buf,
    UINT32          count
    )
{
    int val;

    if (p_buf ==NULL || count ==0)
        return -1;

    if (sscanf(p_buf, "force_max_link_rate=0x%x", &val)==1)
    {
        if (val != g_force_max_link_rate)
        {
            switch(val)
            {
            case 0:
            case _DP_LINK_RBR:
            case _DP_LINK_HBR:
            case _DP_LINK_HBR2:
            case _DP_LINK_HBR3:
                break;
            default:
                DPRX_ADP_WARN("unknown max link rate - %x, skip it!!!\n", val);
                return -1;
            }

            g_force_max_link_rate = val;

            // restart current port
            if (g_p_current_dprx_port)
            {
                p_dprx = g_p_current_dprx_port;
                rtk_dprx_set_hpd(p_dprx, 0);
                dprx_osal_msleep(100);
                rtk_dprx_port_swap(NULL);
                rtk_dprx_port_swap(p_dprx);
                dprx_osal_msleep(100);
                rtk_dprx_set_hpd(p_dprx, 1);
            }
        }
    }
#ifdef DPRX_SINK_CAP_LINK_TRAIN_CTRL_FAKE_TP1_LT
    else if (sscanf(p_buf, "force_fake_tps1_lt=%5d", &val)==1)
    {
        g_force_fake_tps1_lt = (val) ? 1 : 0;
    }
#endif
    else if (sscanf(p_buf, "reset_vfsm=%5d", &val)==1)
    {
        dprx_drv_do_avmute(0);  // do avmute
        val = dprx_drv_source_check(0);  // redo source check
        DPRX_ADP_WARN("source check ret = %d\n", val);
        g_dprx_detected_status = _FALSE;  // force clear detect status
    }
    else if (sscanf(p_buf, "force_demode=%5d", &val)==1)
    {
        g_force_demode = (val) ? 1 : 0;
        rtk_dprx_set_hpd(p_dprx, 0);
        dprx_osal_msleep(500);
        rtk_dprx_set_hpd(p_dprx, 1);
    }
#ifdef CONFIG_DPRX_EANBLE_FREESYNC
    else if (sscanf(p_buf, "force_freesync_off=%5d", &val)==1)
    {
        g_force_freesync_off = (val) ? 1 : 0;
        rtk_dprx_set_hpd(p_dprx, 0);
        dprx_osal_msleep(500);
        rtk_dprx_set_hpd(p_dprx, 1);
    }
#endif
    else if (sscanf(p_buf, "dump_dolby_vision_sdp=%5d", &val)==1)
    {
        if (val)
            g_dprx_sdp_debug_ctrl |= DEBUG_DOLBY_VIOSN_SDP;
        else
            g_dprx_sdp_debug_ctrl &= ~DEBUG_DOLBY_VIOSN_SDP;
    }
    else if (sscanf(p_buf, "dump_amd_freesync_sdp=%5d", &val)==1)
    {
        if (val)
            g_dprx_sdp_debug_ctrl |= DEBUG_AMD_FREESYNC_SPD_SDP;
        else
            g_dprx_sdp_debug_ctrl &= ~DEBUG_AMD_FREESYNC_SPD_SDP;
    }
    else if (sscanf(p_buf, "dump_vsc_ext_vesa_sdp=%5d", &val)==1)
    {
        if (val)
            g_dprx_sdp_debug_ctrl |= DEBUG_VSC_EXT_VESA_SDP;
        else
            g_dprx_sdp_debug_ctrl &= ~DEBUG_VSC_EXT_VESA_SDP;
    }
    else if (sscanf(p_buf, "mute_if_lane_dealign=%5d", &val)==1)
    {
        g_mute_if_lane_dealign = (val) ? 1 : 0;
    }

    return count;
}



//===============================================================================
// Attribute Interface
//===============================================================================


/*------------------------------------------------
 * Func : rtk_dprx_attr_store
 *
 * Desc : store adapter attr
 *
 * Para : p_dprx : handle of DPRX adapter
 *        attr   : attribute name
 *        p_buf  : data buffer
 *        cnt    : buffer size
 *
 * Retn : number if bytes handled
 *-----------------------------------------------*/
int rtk_dprx_attr_show(
    RTK_DPRX_ADP*   p_dprx,
    ADP_ATTR        attr,
    UINT8*          p_buf,
    UINT32          count
    )
{
    UINT32 n;
    UINT8 *ptr = p_buf;

    switch(attr)
    {
    case ADP_ATTR_STATUS:
        n = rtk_dprx_attr_show_status(p_dprx, ptr, count);
        ptr+=n; count-=n;
        break;

    case ADP_ATTR_DPCD:
        n = rtk_dprx_attr_show_dpcd(p_dprx, ptr, count);
        ptr+=n; count-=n;
        break;

    case ADP_ATTR_SDP:
        break;

    case ADP_ATTR_DEBUG_CTRL:
        n = rtk_dprx_attr_show_debug_ctrl(p_dprx, ptr, count);
        ptr+=n; count-=n;
        break;

    default:
        break;
    }

    return ptr - p_buf;
}


/*------------------------------------------------
 * Func : rtk_dprx_attr_store
 *
 * Desc : store adapter attr
 *
 * Para : p_dprx : handle of DPRX adapter
 *        attr   : attribute name
 *        p_buf  : data buffer
 *        cnt    : buffer size
 *
 * Retn : number if bytes handled
 *-----------------------------------------------*/
int rtk_dprx_attr_store(
    RTK_DPRX_ADP*   p_dprx,
    ADP_ATTR        attr,
    UINT8*          p_buf,
    UINT32          count
    )
{
    switch(attr)
    {
    case ADP_ATTR_STATUS:
        break;

    case ADP_ATTR_DPCD:
        break;

    case ADP_ATTR_SDP:
        break;

    case ADP_ATTR_DEBUG_CTRL:
        return rtk_dprx_attr_store_debug_ctrl(p_dprx, p_buf, count);
        break;

    default:
        break;
    }

    return count;
}
