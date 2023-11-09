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
#ifdef CONFIG_DPRX_GET_IC_VERSION
#include <mach/platform.h>
#endif

#define GET_RTK_DPRX_PORT(p_adp)      ((p_adp && p_adp->p_private) ? (RTK_DPRX_ADP*) p_adp->p_private : NULL)

//================ EXT CONFIG Related ==================
#ifdef DPRX_EXT_IC_VERSION
    #define DPRX_DRV_SET_IC_VERSION(port, val)              dprx_drv_set_ext_config(port, DPRX_EXT_IC_VERSION, val);
#else
    #define DPRX_DRV_SET_IC_VERSION(port, val)
#endif

#ifdef DPRX_EXT_WD_ENABLE
    #define DPRX_DRV_SET_VIDEO_WATCH_DOG(port, val)         dprx_drv_set_ext_config(port, DPRX_EXT_WD_ENABLE, val);
#else
    #define DPRX_DRV_SET_VIDEO_WATCH_DOG(port, val)
#endif

//================ Definitions of variable ==================

RTK_DPRX_ADP* g_p_current_dprx_port = NULL;
DPRX_SDP_HANDLER g_dprx_sdp_handler;
UINT32 g_dprx_sdp_debug_ctrl = 0;
edid_dev_t*   g_p_dprx_edid = NULL;
static unsigned char g_p_dprx_edid_resume = 0;
static unsigned char g_dprx_edid_reference_cnt = 0;

#ifndef CONFIG_DPRX_VFE_ENABLE_HPD_LOW_GUARD_INTERVAL
#define DPRX_PORT_SWAP_DP_HPD_TOGGLE_TIME       500  // Mircsoft Surface Pro takes more than 500ms to detect long hpd toggle
#define DPRX_PORT_SWAP_TYPEC_HPD_TOGGLE_TIME   1300  // Lindy HDMI to typeC converter takes more than 1200 mse to detect hpd
#endif

#ifdef CONFIG_DPRX_EANBLE_FREESYNC
UINT8 g_force_freesync_off = 0;
#endif
UINT8 g_force_demode = 0;
UINT8 g_force_max_link_rate = 0;  // disabled
UINT8 g_force_fake_tps1_lt = 0;  // disabled
UINT8 g_mute_if_lane_dealign = 1;

//#define FORCE_DEFAULT_EDID
#ifdef FORCE_DEFAULT_EDID

// ARG Free Sync EDID
UINT8 default_edid[] =
{
    0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x06,0xb3,0x2a,0x27,0xff,0xff,0xff,0xff,
    0x27,0x1d,0x01,0x04,0xb5,0x3c,0x22,0x78,0x3b,0x0b,0x10,0xb0,0x4c,0x45,0xa8,0x26,
    0x0a,0x50,0x54,0xbf,0xcf,0x00,0x71,0x4f,0x81,0xc0,0x81,0x40,0x81,0x80,0xd1,0xc0,
    0xd1,0xfc,0x95,0x00,0xb3,0x00,0x4d,0xd0,0x00,0xa0,0xf0,0x70,0x3e,0x80,0x30,0x20,
    0x35,0x00,0x54,0x4f,0x21,0x00,0x00,0x1a,0x00,0x00,0x00,0xfd,0x0c,0x30,0x90,0x4a,
    0x4a,0x81,0x01,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xfc,0x00,0x52,
    0x4f,0x47,0x20,0x58,0x47,0x32,0x37,0x55,0x51,0x0a,0x20,0x20,0x00,0x00,0x00,0xff,
    0x00,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x02,0xb4,
    0x02,0x03,0x3a,0xf2,0x50,0x60,0x61,0x01,0x02,0x03,0x11,0x12,0x13,0x04,0x0e,0x0f,
    0x1d,0x1e,0x1f,0x10,0x3f,0xe2,0x00,0xd5,0x23,0x09,0x07,0x07,0x83,0x01,0x00,0x00,
    0x65,0x03,0x0c,0x00,0x10,0x00,0xe3,0x05,0xc0,0x00,0xe6,0x06,0x05,0x01,0x70,0x70,
    0x07,0x68,0x1a,0x00,0x00,0x01,0x0b,0x30,0x90,0x00,0x56,0x5e,0x00,0xa0,0xa0,0xa0,
    0x29,0x50,0x30,0x20,0x35,0x00,0x54,0x4f,0x21,0x00,0x00,0x1a,0x86,0xbc,0x00,0x50,
    0xa0,0xa0,0x55,0x50,0x08,0x20,0x78,0x00,0x54,0x4f,0x21,0x00,0x00,0x1e,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x87,
    0x70,0x12,0x67,0x00,0x00,0x03,0x01,0x64,0x9b,0x12,0x01,0x04,0xff,0x0e,0x4f,0x00,
    0x07,0x00,0x1f,0x00,0x6f,0x08,0x52,0x00,0x44,0x00,0x07,0x00,0x15,0x4c,0x01,0x04,
    0xff,0x0e,0x3b,0x00,0x07,0x80,0x1f,0x00,0x6f,0x08,0x40,0x00,0x02,0x00,0x04,0x00,
    0x7b,0x5a,0x01,0x04,0xff,0x0e,0x4f,0x00,0x07,0x00,0x1f,0x00,0x6f,0x08,0x68,0x00,
    0x5a,0x00,0x07,0x00,0x3a,0x9b,0x01,0x04,0xff,0x0e,0x4f,0x00,0x07,0x80,0x1f,0x00,
    0x6f,0x08,0x4d,0x00,0x02,0x00,0x04,0x00,0x47,0xf8,0x01,0x04,0xff,0x0e,0x4f,0x00,
    0x07,0x00,0x1f,0x00,0x6f,0x08,0x7e,0x00,0x70,0x00,0x07,0x00,0x52,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x90,
};

#endif

#ifdef CONFIG_ENABLE_TYPEC_DPRX

static DPRX_PORT_CONFIG g_dprx_typec_cable_cfg[] =
{
    {.aux_pn_swap = 1, .lane3_pn_swap = 1, .lane2_pn_swap = 0, .lane1_pn_swap = 1, .lane0_pn_swap = 0, .lane3_mapping = 1, .lane2_mapping = 3, .lane1_mapping = 0, .lane0_mapping = 2},         //Case A
    {.aux_pn_swap = 1, .lane3_pn_swap = 1, .lane2_pn_swap = 0, .lane1_pn_swap = 1, .lane0_pn_swap = 0, .lane3_mapping = 1, .lane2_mapping = 0, .lane1_mapping = 3, .lane0_mapping = 2},         //Case B
    {.aux_pn_swap = 1, .lane3_pn_swap = 1, .lane2_pn_swap = 0, .lane1_pn_swap = 1, .lane0_pn_swap = 0, .lane3_mapping = 1, .lane2_mapping = 0, .lane1_mapping = 2, .lane0_mapping = 3},         //Case C
    {.aux_pn_swap = 1, .lane3_pn_swap = 1, .lane2_pn_swap = 0, .lane1_pn_swap = 1, .lane0_pn_swap = 0, .lane3_mapping = 1, .lane2_mapping = 0, .lane1_mapping = 2, .lane0_mapping = 3},         //Case D
    {.aux_pn_swap = 0, .lane3_pn_swap = 0, .lane2_pn_swap = 1, .lane1_pn_swap = 0, .lane0_pn_swap = 1, .lane3_mapping = 2, .lane2_mapping = 3, .lane1_mapping = 1, .lane0_mapping = 0},         //Case E

    {.aux_pn_swap = 0, .lane3_pn_swap = 1, .lane2_pn_swap = 0, .lane1_pn_swap = 1, .lane0_pn_swap = 0, .lane3_mapping = 3, .lane2_mapping = 1, .lane1_mapping = 2, .lane0_mapping = 0},         //Case A Flip
    {.aux_pn_swap = 0, .lane3_pn_swap = 1, .lane2_pn_swap = 0, .lane1_pn_swap = 1, .lane0_pn_swap = 0, .lane3_mapping = 3, .lane2_mapping = 2, .lane1_mapping = 1, .lane0_mapping = 0},         //Case B Flip
    {.aux_pn_swap = 0, .lane3_pn_swap = 1, .lane2_pn_swap = 0, .lane1_pn_swap = 1, .lane0_pn_swap = 0, .lane3_mapping = 3, .lane2_mapping = 2, .lane1_mapping = 0, .lane0_mapping = 1},         //Case C Flip
    {.aux_pn_swap = 0, .lane3_pn_swap = 1, .lane2_pn_swap = 0, .lane1_pn_swap = 1, .lane0_pn_swap = 0, .lane3_mapping = 3, .lane2_mapping = 2, .lane1_mapping = 0, .lane0_mapping = 1},         //Case D Flip
    {.aux_pn_swap = 1, .lane3_pn_swap = 0, .lane2_pn_swap = 1, .lane1_pn_swap = 0, .lane0_pn_swap = 1, .lane3_mapping = 0, .lane2_mapping = 1, .lane1_mapping = 3, .lane0_mapping = 2},         //Case E Flip
};

#endif

#ifdef VFSM_0_WAIT_LINK_READY

const char* _get_dprx_vfsm_str(UINT8 vfsm)
{
    switch(vfsm)
    {
    case VFSM_0_WAIT_LINK_READY:        return "WAIT_LINK_READY";
    case VFSM_1_WAIT_VIDEO_STREAM_PRESENT:  return "WAIT_VIDEO_STREAM_PRESENT";
    case VFSM_2_WAIT_MSA_INFO:          return "WAIT_MSA_INFO";
    case VFSM_3_GET_VIDEO_INFO:         return "GET_VIDEO_INFO";
    case VFSM_4_GET_MEASURE_INFO:       return "MEASURE";
    case VFSM_5_SET_DISP_FORMAT:        return "SET_DISP_FORMAT";
    case VFSM_6_SET_STREAM_CLOCK_GEN:   return "SET_STREAM_CLOCK_GEN";
    case VFSM_7_ENABLE_VPLL_TRACKING:   return "ENABLE_PLL_TRACKING";
    case VFSM_8_CHK_VIDEO_FIFO:         return "CHK_VIDEO_FIFO";
    case VFSM_9_VIDEO_OUT_ENABLE:       return "VIDEO_ENABLE";
    default:                            return "Unknwon";
    }
}

#endif

void _set_dprx_ic_version(UINT8 port)
{
    #ifdef CONFIG_DPRX_GET_IC_VERSION
    switch(get_ic_version())
    {
        case VERSION_A:
            DPRX_DRV_SET_IC_VERSION(port, 0);
            break;
        case VERSION_B:
            DPRX_DRV_SET_IC_VERSION(port, 1);
            break;
        default:
            DPRX_DRV_SET_IC_VERSION(port, 0);
    }
    #endif
}

/*------------------------------------------------
 * Func : _get_max_lane_count
 *
 * Desc : get maximum lane count
 *
 * Para : p_adp  : handle of DPRX adapter
 *
 * Retn : lane count = min ( max adp lane count, max cable lane count)
 *-----------------------------------------------*/
UINT8 _get_max_lane_count(RTK_DPRX_ADP* p_dprx)
{
#ifdef CONFIG_ENABLE_TYPEC_DPRX
    if (p_dprx->type == DP_TYPE_USB_TYPE_C)
    {
        UINT8 cable_lane_count = 4;

        // for typec, the maximum lane count will related to cable config
        switch(p_dprx->cable_type)
        {
        case CABLE_CFG_TYPEC_CFG_B:
        case CABLE_CFG_TYPEC_CFG_B_FLIP:
        case CABLE_CFG_TYPEC_CFG_D:
        case CABLE_CFG_TYPEC_CFG_D_FLIP:
            cable_lane_count = 2;   // for config B/D, 2 lanes are available
            break;
        default:
            break;
        }

        if (cable_lane_count < p_dprx->sink_cap.max_lane_count)
            return cable_lane_count;
    }
#endif

    return p_dprx->sink_cap.max_lane_count;
}


/*------------------------------------------------
 * Func : rtk_dprx_ddc_init
 *
 * Desc : init ddc of rtk dprx
 *
 * Para : p_dprx : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_ddc_init(void)
{
    DPRX_ADP_INFO("rtk_dprx_ddc_init (%d) (reference count=%d)\n", DPRX_AUX_DDC, g_dprx_edid_reference_cnt);

    if (g_p_dprx_edid==NULL)
    {
        // create EDID data
#ifdef CONFIG_DPRX_DRV_ENABLE_VIRTUAL_PLATFORM
        g_p_dprx_edid = alloc_dprx_edid_device();
#else
  #ifdef CONFIG_DPRX_VFE_ENABLE_QUICK_SHOW
        g_p_dprx_edid = create_rtk_ddc_edid_device_ex(DPRX_AUX_DDC, dprx_platform_get_quick_show_enable() ? RTK_DDC_EDID_QS_INIT : 0);
  #else
        g_p_dprx_edid = create_rtk_ddc_edid_device(DPRX_AUX_DDC);
  #endif
#endif
        g_dprx_edid_reference_cnt = 0;

        if (g_p_dprx_edid==NULL)
        {
            DPRX_ADP_WARN("rtk_dprx_ddc_init failed, create dprx ddc (%d) failed\n", DPRX_AUX_DDC);
            return -1;
        }

        if (dprx_edid_init(g_p_dprx_edid) <0)
        {
            DPRX_ADP_INFO("rtk_dprx_ddc_init failed, init dprx ddc (%d) failed\n", DPRX_AUX_DDC);
            destroy_dprx_edid_device(g_p_dprx_edid);
            g_p_dprx_edid = NULL;
            return -1;
        }

        if (dprx_platform_get_quick_show_enable())
        {
            g_dprx_edid_reference_cnt = 1;   // skip edid reset flow, if QS is enabled already
            return 0;
        }
        dprx_edid_enable(g_p_dprx_edid, 0);
        dprx_edid_reset(g_p_dprx_edid);
        g_dprx_edid_reference_cnt = 0;
    }

    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_ddc_uninit
 *
 * Desc : uninit ddc
 *
 * Para : p_dprx : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_ddc_uninit(void)
{
    DPRX_ADP_INFO("rtk_dprx_ddc_uninit (%d) (reference count=%d)\n", DPRX_AUX_DDC, g_dprx_edid_reference_cnt);

    if (g_p_dprx_edid==NULL)
        return 0;  // already released, do nothing

    if (g_dprx_edid_reference_cnt)
        g_dprx_edid_reference_cnt--;

    DPRX_ADP_INFO("rtk_dprx_ddc_uninit (%d) success (reference count=%d)\n", DPRX_AUX_DDC, g_dprx_edid_reference_cnt);

    if (g_dprx_edid_reference_cnt==0)
    {
        DPRX_ADP_INFO("destroy release ddc device\n");
        dprx_edid_enable(g_p_dprx_edid, 0);          // disable EDID
        destroy_dprx_edid_device(g_p_dprx_edid);     // free edid device
        g_p_dprx_edid = NULL;
    }

    return 0;
}

#ifdef CONFIG_DPRX_VFE_ENABLE_QUICK_SHOW

/*------------------------------------------------
 * Func : rtk_dprx_port_qs_init
 *
 * Desc : do QS init. When QS enabled, HW has been
 *        Initialized in Kboot, here we have to
 *        resync FW status from HW
 *
 * Para : p_dprx : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_port_qs_init(
    RTK_DPRX_ADP*   p_dprx
    )
{
    if (p_dprx == NULL)
        return 0;  // invalid handle, do nothing

    if (p_dprx->qs_init_ready==0 || p_dprx->qs_connect_ready==0)
    {
        DPRX_ADP_INFO("rtk_dprx_port_qs_init : not current port, skip it\n");
        return 0;  // invalid handle, do nothing
    }

    DPRX_ADP_INFO("rtk_dprx_port_qs_init : current port, do qs init\n");

    // 1. setup low level driver port configuration
#ifdef CONFIG_ENABLE_TYPEC_DPRX
    memcpy(&g_dprx_port_config, &p_dprx->port_cfg, sizeof(DPRX_PORT_CONFIG));
#endif

    // 2. setup low level driver sink capability
    memcpy(&g_dprx_sink_capability, &p_dprx->sink_cap, sizeof(DPRX_SINK_CAPABILITY));

    // 3. setup FW EDID state
    if (p_dprx->qs_edid_ready==0)
    {
        dprx_edid_get_edid(g_p_dprx_edid, p_dprx->sink_edid, sizeof(p_dprx->sink_edid));  // read edid to DDC sram to get EDID
        p_dprx->qs_edid_ready = 1;
    }

    dprx_edid_enable(g_p_dprx_edid, 1); // force enable edid access
    g_dprx_edid_reference_cnt = 1; // force edid reference count to 1

    // 4. setup FW hpd state
    p_dprx->hpd_enable = 1;  // force hpd enable
    g_p_current_dprx_port = p_dprx;

    // 5. do driver qs init
    return dprx_drv_qs_init(0);
}

#endif

/*------------------------------------------------
 * Func : rtk_dprx_port_swap
 *
 * Desc : change dprx port to dprx low level driver
 *
 * Para : p_dprx : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_port_swap(
    RTK_DPRX_ADP*   p_dprx
    )
{
    if (g_p_current_dprx_port == p_dprx)
        return 0;  // same dprx port, do nothing

    // A. stop current port (if necessary)
    if (g_p_current_dprx_port)
    {
        DPRX_ADP_INFO("rtk_dprx_port_swap : stop current port\n");

        g_p_current_dprx_port->connected = 0;   // disconnect current port

        // 1. stop dprx port output

        // 2. current port hpd off
        if (g_p_current_dprx_port->p_hpd)
            dprx_hpd_set_hpd(g_p_current_dprx_port->p_hpd, 0);   // force set current port hpd to low

        // 3. ddc/edid off
        dprx_edid_enable(g_p_dprx_edid, 0);

        // 4. release ddc reference count
        g_dprx_edid_reference_cnt = 0;

        // 5. clear sdp info
        memset(&g_dprx_sdp_handler, 0, sizeof(g_dprx_sdp_handler));

#ifdef DPRX_SUPPORT_SET_LOW_POWER_MODE
        // 6. set lpm state
        DPRX_ADP_INFO("rtk_dprx_port_swap : set current port to low power mode\n");
        dprx_drv_set_low_power_mode_enable(0, 1);
        g_p_current_dprx_port->lpm_enable = 1;
#endif
        g_p_current_dprx_port->link_round = 0;
        g_p_current_dprx_port->link_state = 0;
        g_p_current_dprx_port->link_sub_state = 0;
    }

    // B. start new current port
    if (p_dprx)
    {
        DPRX_ADP_INFO("rtk_dprx_port_swap : start new DPRX port\n");

        // b.1 setup new dprx
#ifdef CONFIG_ENABLE_TYPEC_DPRX
        memcpy(&g_dprx_port_config, &p_dprx->port_cfg, sizeof(DPRX_PORT_CONFIG));
        DPRX_ADP_INFO("set dprx port (aux_phy_port_idx=%d, mac_phy_port_idx=%d)\n", g_dprx_port_config.aux_sel, g_dprx_port_config.phy_sel);
#endif
        memcpy(&g_dprx_sink_capability, &p_dprx->sink_cap, sizeof(DPRX_SINK_CAPABILITY));

        if (g_force_max_link_rate && g_dprx_sink_capability.max_link_rate != g_force_max_link_rate)
            g_dprx_sink_capability.max_link_rate = g_force_max_link_rate;  // override max_link_rate

        g_dprx_sink_capability.max_lane_count = _get_max_lane_count(p_dprx);  // determine max lane count

#ifdef DPRX_SINK_CAP_LINK_TRAIN_CTRL_FAKE_TP1_LT
        if (g_force_fake_tps1_lt)
            g_dprx_sink_capability.link_train_ctrl |= DPRX_SINK_CAP_LINK_TRAIN_CTRL_FAKE_TP1_LT;  // allow to change fake tp1_lt
        else
            g_dprx_sink_capability.link_train_ctrl = 0;  // over
#endif

        DPRX_ADP_INFO("set dprx port (max_lane_count=%d, max_link_rate=%02x)\n",
            g_dprx_sink_capability.max_lane_count,
            g_dprx_sink_capability.max_link_rate);

#ifdef CONFIG_DPRX_EANBLE_FREESYNC
        if (g_force_freesync_off && g_dprx_sink_capability.freesync_support)
        {
            DPRX_ADP_WARN("g_force_freesync_off is set, force disable freesync\n");
            g_dprx_sink_capability.freesync_support = 0;
        }
#endif

        dprx_drv_init(0);

        // b.2 update edid
        dprx_edid_set_edid(g_p_dprx_edid, p_dprx->sink_edid, sizeof(p_dprx->sink_edid));  // write edid to sram

        // 3. ddc/edid on
        dprx_edid_enable(g_p_dprx_edid, 1);
        g_dprx_edid_reference_cnt = 1;

        // 4. setup hpd
        if (p_dprx->p_hpd)
        {
#ifdef CONFIG_ENABLE_TYPEC_DPRX
            dprx_hpd_set_hpd(p_dprx->p_hpd, 0); // hpd should be reset 0 no matter the hpd value

  #ifdef CONFIG_DPRX_VFE_ENABLE_HPD_LOW_GUARD_INTERVAL
            // no delay, let HPD engine to guarantee the guard interval
  #else
            switch(p_dprx->type)
            {
                case DP_TYPE_USB_TYPE_C:
                    DPRX_ADP_INFO("rtk_dprx_port_swaped, do hpd toggle (%dms)\n", DPRX_PORT_SWAP_TYPEC_HPD_TOGGLE_TIME);
                    dprx_osal_msleep(DPRX_PORT_SWAP_TYPEC_HPD_TOGGLE_TIME);
                break;
                default :
                    DPRX_ADP_INFO("rtk_dprx_port_swaped, do hpd toggle (%dms)\n", DPRX_PORT_SWAP_DP_HPD_TOGGLE_TIME);
                    dprx_osal_msleep(DPRX_PORT_SWAP_DP_HPD_TOGGLE_TIME);
            }
  #endif
#endif
            dprx_hpd_set_hpd(p_dprx->p_hpd, p_dprx->hpd_enable);
        }

        // 5. clear sdp info
        memset(&g_dprx_sdp_handler, 0, sizeof(g_dprx_sdp_handler));

#ifdef DPRX_SUPPORT_SET_LOW_POWER_MODE
        // 6. set lpm state
        if (p_dprx->p_hpd ==NULL || dprx_hpd_get_connect_status(p_dprx->p_hpd)==0)
        {
            DPRX_ADP_INFO("rtk_dprx_port_swaped without cable, enter low power mode\n");
            dprx_drv_set_low_power_mode_enable(0, 1);
            p_dprx->lpm_enable = 1;
        }
        else if (p_dprx->lpm_enable)
        {
            DPRX_ADP_INFO("rtk_dprx_port_swaped with cable under low power mode, exit low power mode\n");
            p_dprx->lpm_enable = 0;
        }
        else
        {
            DPRX_ADP_INFO("rtk_dprx_port_swaped with cable under normal mode\n");
        }
#endif

        // 6. Set IC version
        _set_dprx_ic_version(0);
    }

    // C. set it to current port
    g_p_current_dprx_port = p_dprx;

    return 0;
}

#ifdef CONFIG_ENABLE_TYPEC_DPRX

/*------------------------------------------------
 * Func : rtk_dprx_set_typec_port_config
 *
 * Desc : set typeC config
 *
 * Para : p_dprx : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_set_typec_port_config(RTK_DPRX_ADP* p_dprx, DPRX_CABLE_CFG* p_type_c_dp_cfg)
{
    int type_id = 0;
    if ((p_type_c_dp_cfg==NULL)||(p_dprx==NULL))
        return -1;

    switch(*p_type_c_dp_cfg)
    {
        case CABLE_CFG_TYPEC_CFG_A: type_id = 0; break;
        case CABLE_CFG_TYPEC_CFG_B: type_id = 1; break;
        case CABLE_CFG_TYPEC_CFG_C: type_id = 2; break;
        case CABLE_CFG_TYPEC_CFG_D: type_id = 3; break;
        case CABLE_CFG_TYPEC_CFG_E: type_id = 4; break;
        case CABLE_CFG_TYPEC_CFG_A_FLIP: type_id = 5; break;
        case CABLE_CFG_TYPEC_CFG_B_FLIP: type_id = 6; break;
        case CABLE_CFG_TYPEC_CFG_C_FLIP: type_id = 7; break;
        case CABLE_CFG_TYPEC_CFG_D_FLIP: type_id = 8; break;
        case CABLE_CFG_TYPEC_CFG_E_FLIP: type_id = 9; break;
        default:
            type_id = 0;
            break;
    }

    p_dprx->cable_type = *p_type_c_dp_cfg;
    p_dprx->port_cfg.aux_pn_swap = g_dprx_typec_cable_cfg[type_id].aux_pn_swap;
    p_dprx->port_cfg.lane0_pn_swap = g_dprx_typec_cable_cfg[type_id].lane0_pn_swap;
    p_dprx->port_cfg.lane1_pn_swap = g_dprx_typec_cable_cfg[type_id].lane1_pn_swap;
    p_dprx->port_cfg.lane2_pn_swap = g_dprx_typec_cable_cfg[type_id].lane2_pn_swap;
    p_dprx->port_cfg.lane3_pn_swap = g_dprx_typec_cable_cfg[type_id].lane3_pn_swap;
    p_dprx->port_cfg.lane0_mapping = g_dprx_typec_cable_cfg[type_id].lane0_mapping;
    p_dprx->port_cfg.lane1_mapping = g_dprx_typec_cable_cfg[type_id].lane1_mapping;
    p_dprx->port_cfg.lane2_mapping = g_dprx_typec_cable_cfg[type_id].lane2_mapping;
    p_dprx->port_cfg.lane3_mapping = g_dprx_typec_cable_cfg[type_id].lane3_mapping;

    DP_PLAT_INFO("lane0_phy_idx = %d, lane1_phy_idx = %d, lane2_phy_idx = %d, lane3_phy_idx = %d, \
                 lane0_pn_swap = %d,  lane1_pn_swap = %d, lane2_pn_swap = %d, lane3_pn_swap = %d\n" , \
                p_dprx->port_cfg.lane0_mapping, p_dprx->port_cfg.lane1_mapping, p_dprx->port_cfg.lane2_mapping, \
                p_dprx->port_cfg.lane3_mapping, p_dprx->port_cfg.lane0_pn_swap, p_dprx->port_cfg.lane0_pn_swap, \
                p_dprx->port_cfg.lane2_pn_swap, p_dprx->port_cfg.lane3_pn_swap);
    return 1;
}

#endif


/*------------------------------------------------
 * Func : rtk_dprx_init
 *
 * Desc : init DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_init(
    RTK_DPRX_ADP*   p_dprx
    )
{
    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("init rtk dprx port failed, p_dprx==NULL!!!! \n");
        return -1;
    }

    // init global ddc
    if (rtk_dprx_ddc_init()<0)
    {
        DPRX_ADP_WARN("init rtk dprx port failed, init global ddc failed!!!! \n");
        return -1;
    }

    rtk_dprx_plat_init();  // do platform initialization

#ifdef CONFIG_DPRX_VFE_ENABLE_QUICK_SHOW
    if (p_dprx->qs_init_ready)
    {
        DPRX_ADP_INFO("init rtk dprx port in QS mode (conneced=%d)\n", p_dprx->qs_connect_ready);

        // reset parameter
        p_dprx->connected = p_dprx->qs_connect_ready;
        p_dprx->hpd_enable = 1;  // QS is alway hpd enable

        // init port
        if (p_dprx->connected)
        {
            // QS port. dp QS init
            if (rtk_dprx_port_qs_init(p_dprx)<0)
            {
                g_p_current_dprx_port = NULL; // clear current port

#ifdef CONFIG_ENABLE_TYPEC_DPRX
#ifdef FORCE_RE_ALT_MODE
                if (p_dprx->p_hpd && p_dprx->type == DP_TYPE_USB_TYPE_C)
                {
                    DPRX_ADP_INFO("TypeC : force do re-alt\n");
                    dprx_hpd_set_typec_re_alt_mode(p_dprx->p_hpd);
                }
#endif
#endif

                rtk_dprx_port_swap(p_dprx);   // QS init fail, do port swap to instead
            }
            else
            {
                // QS init pass
                dprx_hpd_init(p_dprx->p_hpd);

#ifdef CONFIG_ENABLE_TYPEC_DPRX
                if (p_dprx->type == DP_TYPE_USB_TYPE_C)
                {
                    dprx_hpd_get_cable_config(p_dprx->p_hpd, &p_dprx->cable_type);   // read back current cable status
                    rtk_dprx_set_typec_port_config(p_dprx, &p_dprx->cable_type);
                }
#endif
            }

            g_p_current_dprx_port = p_dprx;   // set DP port to current port
        }
        else
        {
            dprx_hpd_init(p_dprx->p_hpd);  // QS port. dp QS init
        }
        p_dprx->qs_init_ready = 0; // clear qs init ready flag
        p_dprx->qs_open_ready = 1; // force open ready
    }
    else
#endif
    {
        DPRX_ADP_INFO("init rtk dprx port in normal mode\n");

        // reset parameter
        p_dprx->connected = 0;
        p_dprx->hpd_enable = 0;

        // init port
        if (g_p_current_dprx_port==NULL)
        {
#ifdef CONFIG_ENABLE_TYPEC_DPRX
#ifdef FORCE_RE_ALT_MODE
            if (p_dprx->p_hpd && p_dprx->type == DP_TYPE_USB_TYPE_C)
            {
                DPRX_ADP_INFO("TypeC : force do re-alt\n");
                dprx_hpd_init(p_dprx->p_hpd);
                dprx_hpd_set_typec_re_alt_mode(p_dprx->p_hpd);
            }
#endif
#endif
            rtk_dprx_port_swap(p_dprx);
        }
        else
        {
            if (p_dprx->p_hpd)
            {
                dprx_hpd_init(p_dprx->p_hpd);
                dprx_hpd_set_hpd(p_dprx->p_hpd, 0); // need to foce hpd to low
#ifdef CONFIG_ENABLE_TYPEC_DPRX
#ifdef FORCE_RE_ALT_MODE
            if (p_dprx->type == DP_TYPE_USB_TYPE_C)
            {
                DPRX_ADP_INFO("TypeC : force do re-alt\n");
                dprx_hpd_set_typec_re_alt_mode(p_dprx->p_hpd);
            }
#endif
#endif
            }
        }
    }
    rtk_dprx_plat_interrupt_enable();

    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_open
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
int rtk_dprx_open(
    RTK_DPRX_ADP*   p_dprx
    )
{
    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("open rtk dprx port failed, p_dprx==NULL!!!! \n");
        return -1;
    }

    // set DPRX port to current port (if no current port present)
    if (g_p_current_dprx_port == NULL)
    {
        rtk_dprx_port_swap(p_dprx);     // change dprx port
    }
    else if (g_p_current_dprx_port!=p_dprx)
    {
#ifdef CONFIG_DPRX_VFE_ENABLE_QUICK_SHOW
        if (p_dprx->qs_open_ready)
        {
            // Do not update hpd status in QS mode
            p_dprx->qs_open_ready = 0;  // clear qs open flag
            return 0;
        }
#endif
        rtk_dprx_set_hpd(p_dprx, 0); // hpd should be reset 0 no matter the hpd value
    }

    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_close
 *
 * Desc : close DPRX adapter. after close, the DPRX
 *        adapter will stop operation.
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_close(
    RTK_DPRX_ADP*   p_dprx
    )
{
    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("close rtk dprx port failed, p_dprx==NULL!!!! \n");
        return -1;
    }

    if (g_p_current_dprx_port==p_dprx)
    {
        rtk_dprx_port_swap(NULL);  // set current port to null. it will stop current port immedately
        p_dprx->connected = 0;
        p_dprx->hpd_enable = 0;    // reset hpd status
    }
    else
    {
        if (p_dprx->p_hpd)
            dprx_hpd_set_hpd(p_dprx->p_hpd, 0);  // set hpd low
        p_dprx->connected = 0;     // reset connect
        p_dprx->hpd_enable = 0;    // reset hpd status
    }

    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_uninit
 *
 * Desc : unint DPRX adapter. this function should
 *        be called when adapter to be destroyed
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_uninit(
    RTK_DPRX_ADP*   p_dprx
    )
{
    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("uninit rtk dprx port failed, p_dprx==NULL!!!! \n");
        return -1;
    }

    rtk_dprx_plat_interrupt_disable();

    rtk_dprx_close(p_dprx);    // force clese dprx port

    if (p_dprx->p_hpd)
        destroy_dprx_hpd_device(p_dprx->p_hpd);   // destroy HPD

    rtk_dprx_ddc_uninit();     // release ddc
    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_connect
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
int rtk_dprx_connect(
    RTK_DPRX_ADP*   p_dprx
    )
{
    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("connect rtk dprx port failed, p_dprx==NULL!!!! \n");
        return -1;
    }

    if (g_p_current_dprx_port != p_dprx)
    {
        rtk_dprx_port_swap(p_dprx);     // change to new port
    }
#ifdef DPRX_GET_DEFAULT_EDID_FROM_DDC_SRAM
    else if (p_dprx->qs_edid_reload_request)
    {
        // DPRX is current port, but need to reload edid to ddc sram
        dprx_edid_set_edid(g_p_dprx_edid, p_dprx->sink_edid, sizeof(p_dprx->sink_edid));  // write edid to sram
        p_dprx->qs_edid_reload_request = 0;
    }
    dprx_edid_enable(g_p_dprx_edid, 1);  // force enable edid
#endif

    p_dprx->connected = 1;          // setup connect flag
    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_disconnect
 *
 * Desc : disable DPRX adapter. after detect, the
 *        DPRX adapter detect/output audio/video data
 *
 * Para : p_dprx : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_disconnect(
    RTK_DPRX_ADP*   p_dprx
    )
{
    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("disconnect rtk dprx port failed, p_dprx==NULL!!!! \n");
        return -1;
    }

    // stop dprx A/V output (current port only)
    if (g_p_current_dprx_port==p_dprx)
    {
        DPRX_ADP_INFO("rtk_dprx_disconnect : stop av output\n");
        // setup av mute state
    }

    // clear connect flag
    p_dprx->connected = 0;
    rtk_dprx_set_ext_ctrl(p_dprx, ADP_EXCTL_WATCH_DOG_ENABLE, 0); //Disable DPRX WatchDog
    DPRX_ADP_INFO("disconnect rtk dprx port successed \n");
    return 0;
}


#ifdef CONFIG_ENABLE_TYPEC_DPRX

/*------------------------------------------------
 * Func : _check_typec_config
 *
 * Desc : check typeC config status.
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : type c config not changed
 *        1 : type c config changed
 *-----------------------------------------------*/
int _check_typec_config(
    RTK_DPRX_ADP*   p_dprx
    )
{
    int ret = 0;
    if (p_dprx && p_dprx->type == DP_TYPE_USB_TYPE_C)
    {
        DPRX_CABLE_CFG p_cable_cfg = 0;
        ret = dprx_hpd_get_cable_config(p_dprx->p_hpd, &p_cable_cfg);

        if(ret && p_dprx->cable_type != p_cable_cfg && g_p_current_dprx_port == p_dprx)
        {
            DPRX_ADP_INFO("dprx_hpd_get_cable_config : cable_status %x -> %x\n", p_dprx->cable_type, p_cable_cfg);
            rtk_dprx_set_typec_port_config(p_dprx, &p_cable_cfg);
            rtk_dprx_port_swap(NULL);       // change to NULL port
            rtk_dprx_port_swap(p_dprx);     // change to type c port
            p_dprx->connected = 1;
            return 1;
        }
    }
    return 0;
}

#endif



/*------------------------------------------------
 * Func : rtk_dprx_get_connect_status
 *
 * Desc : get cable connection status
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : disconnect, 1 : connected
 *-----------------------------------------------*/
int rtk_dprx_get_connect_status(
    RTK_DPRX_ADP*   p_dprx
    )
{
    int ret = 0;

    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("get connect status rtk dprx port failed, p_dprx==NULL!!!! \n");
        return 0;
    }

#ifdef CONFIG_ENABLE_TYPEC_DPRX
    if (p_dprx->type == DP_TYPE_USB_TYPE_C)
        _check_typec_config(p_dprx);
#endif

#ifdef CONFIG_DPRX_FORCE_CHECK_MODE_FAIL_CABLE_OFF_ON
    if (p_dprx->fake_cable_off)
    {
        p_dprx->fake_cable_off--;
        DPRX_ADP_WARN("get connect status rtk dprx port failed, fake cable_off==%d!!!! \n", p_dprx->fake_cable_off);
        if (p_dprx->fake_cable_off==0)
            p_dprx->output_enable = 0;
        return 0;
    }
#endif

    if (p_dprx && p_dprx->p_hpd)
    {
        ret = dprx_hpd_get_connect_status(p_dprx->p_hpd);

        if (p_dprx->pre_cable_state != ret)
        {
            DPRX_ADP_INFO("cable state changed !!!! %d -> %d \n", p_dprx->pre_cable_state, ret);

            p_dprx->pre_cable_state = ret;

#ifdef DPRX_SUPPORT_SET_LOW_POWER_MODE
            if (g_p_current_dprx_port == p_dprx && (ret==0 || p_dprx->lpm_enable))
            {
                unsigned char connected = p_dprx->connected;

                if (ret==0) {
                    DPRX_ADP_INFO("cable disconnected !!!! do port reset\n");
                }
                else {
                    DPRX_ADP_INFO("cable connected with lpm_enable !!!! do port reset to exit low power mode\n");
                }

                rtk_dprx_port_swap(NULL);
                rtk_dprx_port_swap(p_dprx);
                p_dprx->connected = connected;
            }
#else
            if (g_p_current_dprx_port == p_dprx && ret==0)
            {
                unsigned char connected = p_dprx->connected;
                DPRX_ADP_INFO("cable disconnected !!!! do port reset\n");
                rtk_dprx_port_swap(NULL);
                rtk_dprx_port_swap(p_dprx);
                p_dprx->connected = connected;
            }
#endif
        }

#ifdef CHECK_SOURCE_IN_GET_CONNECT_STATE
        if (ret)
        {
            if ((dprx_odal_get_system_time_ms() < p_dprx->source_check_time) &&
                (dprx_odal_get_system_time_ms() > p_dprx->source_check_time + 30))
            {
                rtk_dprx_source_check(p_dprx);
            }
        }
#endif
    }

    return ret;
}

/*------------------------------------------------
 * Func : rtk_dprx_get_aux_status
 *
 * Desc : get aux status
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : aux disconnect, 1 : aux connected
 *-----------------------------------------------*/
int rtk_dprx_get_aux_status(
    RTK_DPRX_ADP*   p_dprx
    )
{
    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("get connect status rtk dprx port failed, p_dprx==NULL!!!! \n");
        return -1;
    }

#ifdef CONFIG_ENABLE_TYPEC_DPRX
    if (p_dprx->type == DP_TYPE_USB_TYPE_C)
        _check_typec_config(p_dprx);
#endif

    return (p_dprx && p_dprx->p_hpd) ? dprx_hpd_get_aux_status(p_dprx->p_hpd, NULL) : 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_set_hpd
 *
 * Desc : set HPD status of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_set_hpd(
    RTK_DPRX_ADP*   p_dprx,
    UINT8           enable
    )
{
    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("get hpd failed, invalid argments!!!!\n");
        return -1;
    }

    p_dprx->hpd_enable = (enable) ? 1 : 0;  // store hpd info

    if (p_dprx->p_hpd)
    {
        if (g_p_current_dprx_port==p_dprx)
            dprx_hpd_set_hpd(p_dprx->p_hpd, p_dprx->hpd_enable);   // enable/disable hpd
        else
            dprx_hpd_set_hpd(p_dprx->p_hpd, 0);   // always force hpd low if not current port
    }

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
int rtk_dprx_set_hpd_toggle(
    RTK_DPRX_ADP*   p_dprx,
    UINT16           delay_ms
    )
{
    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("set hpd toggle failed, invalid argments!!!!\n");
        return -1;
    }

    p_dprx->hpd_enable = 0;

    if (p_dprx->p_hpd)
    {
        if (g_p_current_dprx_port==p_dprx)
            dprx_hpd_set_hpd_toggle(p_dprx->p_hpd, delay_ms);   // hpd_toggle
    }

    p_dprx->hpd_enable = 1;

    return 0;
}

/*------------------------------------------------
 * Func : rtk_dprx_set_irq_hpd
 *
 * Desc : set IRQ HPD of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_set_irq_hpd(
    RTK_DPRX_ADP*   p_dprx
    )
{
    if (p_dprx==NULL)
        return 0;

    if (g_p_current_dprx_port==p_dprx)
        dprx_hpd_set_irq_hpd(p_dprx->p_hpd);   // set irq hpd

    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_get_hpd
 *
 * Desc : get HPD status of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : HPD low, 1 : HPD high
 *-----------------------------------------------*/
int rtk_dprx_get_hpd(
    RTK_DPRX_ADP*   p_dprx
    )
{
    if (p_dprx==NULL)
        return 0;

    if (g_p_current_dprx_port==p_dprx && p_dprx->p_hpd)
        return dprx_hpd_get_hpd(p_dprx->p_hpd);

    return p_dprx->hpd_enable;
}


/*------------------------------------------------
 * Func : rtk_dprx_set_edid
 *
 * Desc : set EDID of the DPRX adapter
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_edid : edid data
 *        edid_len : size of edid data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_set_edid(
    RTK_DPRX_ADP*   p_dprx,
    unsigned char*  p_edid,
    unsigned short  edid_len
    )
{
    if (p_dprx==NULL || g_p_dprx_edid==NULL ||
        p_edid==NULL || edid_len > sizeof(p_dprx->sink_edid))
    {
        DPRX_ADP_WARN("set edid failed, invalid argments!!!!\n");
        return -1;
    }

    // save new edid
    memset(p_dprx->sink_edid, 0, sizeof(p_dprx->sink_edid)); // clear all edid setting
#ifdef FORCE_DEFAULT_EDID
    memcpy(p_dprx->sink_edid, default_edid, sizeof(default_edid));             // store edid to memory
#else
    memcpy(p_dprx->sink_edid, p_edid, edid_len);             // store edid to memory
#endif

    // apply new edid
    if (g_p_current_dprx_port == p_dprx)
    {
        return dprx_edid_set_edid(g_p_dprx_edid, p_dprx->sink_edid, sizeof(p_dprx->sink_edid));  // write edid to sram
    }

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
int rtk_dprx_get_edid(
    RTK_DPRX_ADP*   p_dprx,
    unsigned char*  p_edid,
    unsigned short  edid_len
    )
{
    if (g_p_dprx_edid==NULL || p_dprx==NULL ||
        p_edid==NULL || edid_len > sizeof(p_dprx->sink_edid))
    {
        DPRX_ADP_WARN("get edid failed, invalid argments!!!!\n");
        return -1;
    }

    if (g_p_current_dprx_port == p_dprx)
    {
        return dprx_edid_get_edid(g_p_dprx_edid, p_edid, edid_len);  // current port, read edid to sram
    }
    else
    {
        memcpy(p_edid, p_dprx->sink_edid, edid_len);   // store edid to memory
    }

    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_get_stream_type
 *
 * Desc : get dprx port stream type
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_get_stream_type(
    RTK_DPRX_ADP*       p_dprx,
    DP_STREAM_TYPE_E*   p_type
    )
{
    DPRX_VIDEO_STREAM_STATUS video_stream_status;

    if (p_dprx==NULL || p_type==NULL)
        return -1;

    // check dprx port status
    if (g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
    {
        *p_type = DPRX_STREAM_TYPE_UNKNOWN;  // not current port, or not connected force return unknown
        return 0;
    }

    if (dprx_drv_get_stream_type(0, &video_stream_status))
    {
        if (video_stream_status.video_mute)
        {
            if (video_stream_status.audio_mute)
                *p_type = DPRX_STREAM_TYPE_UNKNOWN;
            else
                *p_type = DPRX_STREAM_TYPE_AUDIO_ONLY;
        }
        else
        {
            if (video_stream_status.audio_mute)
                *p_type = DPRX_STREAM_TYPE_VIDEO_ONLY;
            else
                *p_type = DPRX_STREAM_TYPE_AUDIO_VIDEO;
        }
    }

    return 0;
}


/*------------------------------------------------
 * Func : _ops_dprx_get_capability
 *
 * Desc : get dprx port hardware capability
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_get_capability(
    RTK_DPRX_ADP*       p_dprx,
    DP_PORT_CAPABILITY* p_cap
    )
{
    DPRX_SINK_CAPABILITY dprx_sink_capability;

    if (p_dprx==NULL || p_cap==NULL)
        return -1;

    memset(p_cap, 0, sizeof(DP_PORT_CAPABILITY));

    if (g_p_current_dprx_port== p_dprx)  // current port, get from DPCD
    {
        if (dprx_drv_get_rx_port_capability(0, &dprx_sink_capability) !=_TRUE)
            return -1;

        p_cap->type     = p_dprx->type;
        p_cap->lane_cnt = dprx_sink_capability.max_lane_count; // g_dprx_sink_capability.max_lane_count;//GET_DPRX_DRV_SINK_LANE_COUNT();

        switch(dprx_sink_capability.max_link_rate)
        {
        case _DP_LINK_NONE:
            p_cap->link_rate_mask = 0;
            break;
        case _DP_LINK_RBR:
            p_cap->link_rate_mask = DP_LINK_RATE_RBR_MASK;
            break;
        case _DP_LINK_HBR:
            p_cap->link_rate_mask = DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK;
            break;
        case _DP_LINK_HBR2:
            p_cap->link_rate_mask = DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK;
            break;
        case _DP_LINK_HBR3:
            p_cap->link_rate_mask = DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK;
            break;
        case _DP_LINK_HBR10G:
            p_cap->link_rate_mask = DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK | DP_LINK_RATE_UHBR10_MASK;
            break;
        default:
            p_cap->link_rate_mask = 0;
            DPRX_ADP_WARN("_ops_dprx_get_capability failed, !!!!, unknow max link bw - %02x\n", dprx_sink_capability.max_link_rate);
            break;
        }

        p_cap->capability.fec_support = dprx_sink_capability.fec_support;
        p_cap->capability.dsc_support = dprx_sink_capability.dsc_support;
    }
    else       // non current port, get from config
    {
        memset(p_cap, 0, sizeof(DP_PORT_CAPABILITY));
        p_cap->type     = p_dprx->type;
        p_cap->lane_cnt = p_dprx->sink_cap.max_lane_count;

        switch(p_dprx->sink_cap.max_link_rate)
        {
        case _DP_LINK_NONE:
            p_cap->link_rate_mask = 0;
            break;
        case _DP_LINK_RBR:
            p_cap->link_rate_mask = DP_LINK_RATE_RBR_MASK;
            break;
        case _DP_LINK_HBR:
            p_cap->link_rate_mask = DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK;
            break;
        case _DP_LINK_HBR2:
            p_cap->link_rate_mask = DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK;
            break;
        case _DP_LINK_HBR3:
            p_cap->link_rate_mask = DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK;
            break;
        case _DP_LINK_HBR10G:
            p_cap->link_rate_mask = DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK | DP_LINK_RATE_UHBR10_MASK;
            break;
        default:
            p_cap->link_rate_mask = 0;
            DPRX_ADP_WARN("_ops_dprx_get_capability failed, !!!!, unknow max link bw - %02x\n", p_dprx->sink_cap.max_link_rate);
            break;
        }

        p_cap->capability.fec_support = p_dprx->sink_cap.fec_support;
        p_cap->capability.dsc_support = p_dprx->sink_cap.dsc_support;
    }

    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_get_link_status
 *
 * Desc : get dprx port link status
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int rtk_dprx_get_link_status(
    RTK_DPRX_ADP*       p_dprx,
    DP_LINK_STATUS_T*   p_status
    )
{
    if (p_dprx==NULL || p_status==NULL)
        return -1;

    memset(p_status, 0, sizeof(DP_LINK_STATUS_T));

    if (g_p_current_dprx_port==p_dprx)
    {
        /* FIX ME : get real value from DPCD */
        p_status->lane_mode = newbase_dprx_dpcd_get_lane_count(0);  // get from dpcd ?
        p_status->channel_coding = newbase_dprx_dpcd_get_main_link_channel_coding_set(0);  // get from dpcd ?
        p_status->fec_enable = newbase_dprx_dpcd_get_fec_ready(0);  // get from dpcd ?
        p_status->scarmbling_enable = newbase_dprx_dpcd_get_tps_scramble_disable(0) ?  0 : 1 ;
        p_status->channel_align = newbase_dprx_dpcd_get_lane_align_status(0);  // get from dpcd ?
        switch(newbase_dprx_dpcd_get_link_bw_set(0))
        {
            case 0x6:
                p_status->link_rate = DP_LINK_RATE_RBR_1p62G;
                break;

            case 0xA:
                p_status->link_rate = DP_LINK_RATE_HBR1_2p7G;
                break;

            case 0x14:
                p_status->link_rate = DP_LINK_RATE_HBR2_5p4G;
                break;

            case 0x1E:
                p_status->link_rate = DP_LINK_RATE_HBR3_8p1G;
                break;

            case 0x1:
                p_status->link_rate = DP_LINK_RATE_UHBR10_10G;
                break;

            case 0x2:
                p_status->link_rate = DP_LINK_RATE_UHBR20_20G;
                break;

            case 0x4:
                p_status->link_rate = DP_LINK_RATE_UHBR13_13p5G;
                break;

            default :
                p_status->link_rate = DP_LINK_RATE_MAX;
                break;
        }
        /*
        p_status->channel_status[0] = ;  // get from dpcd ?
        p_status->channel_status[1] = ;  // get from dpcd ?
        p_status->channel_status[2] = ;  // get from dpcd ?
        p_status->channel_status[3] = ;  // get from dpcd ?
        */
    }

    return 0;
}


DP_COLOR_SPACE_E _to_dp_color_space(
    UINT8           color_space
    )
{
    switch (color_space)
    {
    case _COLOR_SPACE_RGB:      return DP_COLOR_SPACE_RGB;
    case _COLOR_SPACE_YCBCR444: return DP_COLOR_SPACE_YUV444;
    case _COLOR_SPACE_YCBCR422: return DP_COLOR_SPACE_YUV422;
    case _COLOR_SPACE_YCBCR420: return DP_COLOR_SPACE_YUV420;
    case _COLOR_SPACE_Y_ONLY:   return DP_COLOR_SPACE_YONLY;
    default:
    case _COLOR_SPACE_RAW:
        DPRX_ADP_WARN("_to_dp_color_space failed, !!!!, unknow type - %d, force RGB\n", color_space);
        return DP_COLOR_SPACE_RGB;
    }
}


DP_COLORIMETRY_E _to_dp_colorimetry(
    UINT8           colorimetry
    )
{
    switch (colorimetry)
    {
    case _COLORIMETRY_RGB_SRGB :       return DP_COLORIMETRY_SRGB;
    case _COLORIMETRY_RGB_ADOBERGB :   return DP_COLORIMETRY_ADOBE_RGB;
    case _COLORIMETRY_RGB_XRRGB :      return DP_COLORIMETRY_XRGB;
    case _COLORIMETRY_RGB_SCRGB :      return DP_COLORIMETRY_SCRGB;
    case _COLORIMETRY_RGB_DCI_P3 :     return DP_COLORIMETRY_DCI_P3;
    case _COLORIMETRY_RGB_COLOR_PROFILE : return DP_COLORIMETRY_CUSTOM_COLOR_PROFILE;
    case _COLORIMETRY_YCC_ITUR_BT601 : return DP_COLORIMETRY_BT_601;
    case _COLORIMETRY_YCC_ITUR_BT709 : return DP_COLORIMETRY_BT_709;
    case _COLORIMETRY_YCC_XVYCC601 :   return DP_COLORIMETRY_XV_YCC_601;
    case _COLORIMETRY_YCC_XVYCC709 :   return DP_COLORIMETRY_XV_YCC_709;
    case _COLORIMETRY_Y_ONLY :         return DP_COLORIMETRY_YONLY;
    case _COLORIMETRY_RAW :            return DP_COLORIMETRY_RAW;
    default :                          return DP_COLORIMETRY_UNKNOWN;
    }
}


DP_COLORIMETRY_E _to_dp_colorimetry_ext(
    UINT8           colorimetry_ext
    )
{
    switch (colorimetry_ext)
    {
    case _COLORIMETRY_EXT_RGB_SRGB :     return  DP_COLORIMETRY_SRGB;
    case _COLORIMETRY_EXT_RGB_ADOBERGB : return  DP_COLORIMETRY_ADOBE_RGB;
    case _COLORIMETRY_EXT_RGB_XRRGB :    return  DP_COLORIMETRY_XRGB;
    case _COLORIMETRY_EXT_RGB_SCRGB :    return  DP_COLORIMETRY_SCRGB;
    case _COLORIMETRY_EXT_RGB_DCI_P3 :   return  DP_COLORIMETRY_DCI_P3;
    case _COLORIMETRY_EXT_RGB_CUSTOM_COLOR_PROFILE : return  DP_COLORIMETRY_CUSTOM_COLOR_PROFILE;
    case _COLORIMETRY_EXT_YCC_ITUR_BT601 : return  DP_COLORIMETRY_BT_601;
    case _COLORIMETRY_EXT_YCC_ITUR_BT709 : return  DP_COLORIMETRY_BT_709;
    case _COLORIMETRY_EXT_YCC_XVYCC601 : return  DP_COLORIMETRY_XV_YCC_601;
    case _COLORIMETRY_EXT_YCC_XVYCC709 : return  DP_COLORIMETRY_XV_YCC_709;
    case _COLORIMETRY_EXT_YCC_ITUR_BT2020_CL :  return  DP_COLORIMETRY_BT_2020_YCCBCCRC;
    case _COLORIMETRY_EXT_YCC_ITUR_BT2020_NCL : return  DP_COLORIMETRY_BT_2020_YCBCR;
    case _COLORIMETRY_EXT_RGB_ITUR_BT2020 :     return  DP_COLORIMETRY_BT_2020_RGB;
    case _COLORIMETRY_EXT_YCC_SYCC601 :         return  DP_COLORIMETRY_S_YCC_601;
    case _COLORIMETRY_EXT_YCC_ADOBEYCC601 :     return  DP_COLORIMETRY_OP_YCC_601;
    case _COLORIMETRY_EXT_Y_ONLY_DICOM_PART14 : return  DP_COLORIMETRY_YONLY;
    case _COLORIMETRY_EXT_RAW_CUSTOM_COLOR_PROFILE : return  DP_COLORIMETRY_RAW;
    default : return  DP_COLORIMETRY_UNKNOWN;
    }
}

DP_CONTENT_TYPE_E _to_dp_content_type(
    UINT8           content_type
    )
{
    switch(content_type)
    {
    case 0 :  return DP_CONTENT_TYPE_NOT_DEFINED;
    case 1 :  return DP_CONTENT_TYPE_GRAPHICS;
    case 2 :  return DP_CONTENT_TYPE_PHOTO;
    case 3 :  return DP_CONTENT_TYPE_VIDEO;
    case 4 :  return DP_CONTENT_TYPE_GAME;
    default : return DP_CONTENT_TYPE_NOT_DEFINED;
    }
}

DP_DYNAMIC_RANGE_E _to_dp_dynamic_range(
    UINT8           dynamic_range
    )
{
    switch(dynamic_range)
    {
    case _DP_COLOR_QUANTIZATION_FULL :  return DP_DYNAMIC_RANGE_VESA;
    case _DP_COLOR_QUANTIZATION_LIMIT : return DP_DYNAMIC_RANGE_CTA;
    default : return DP_DYNAMIC_RANGE_CTA;
    }
}

DP_DRR_MODE_E _to_dp_drr_mode(DPRX_DRR_MODE_E mode)
{
    switch(mode)
    {
    case DPRX_DRR_MODE_FRR:           return DP_DRR_MODE_FRR;
    case DPRX_DRR_MODE_AMD_FREE_SYNC: return DP_DRR_MODE_AMD_FREE_SYNC;
    case DPRX_DRR_MODE_ADAPTIVE_SYNC: return DP_DRR_MODE_ADAPTIVE_SYNC;
    default:                          return DP_DRR_MODE_UNKNWON;
    }
}


int _check_sdp_buff_expiration(
    DPRX_SDP_PKT_BUFFER* p_sdp,
    UINT32               expire_time
    )
{
    if (p_sdp->pkt_count)
    {
        UINT32 elapsed_time = dprx_odal_get_system_time_ms();

        if (elapsed_time >= p_sdp->time_stamp)
            elapsed_time -= p_sdp->time_stamp;
        else  // time over run
            elapsed_time += (0xFFFFFFFF - p_sdp->time_stamp) ;

        if (elapsed_time > expire_time)
        {
            memset(p_sdp, 0, sizeof(DPRX_SDP_PKT_BUFFER));
            return 1;
        }
    }

    return 0;
}

/*------------------------------------------------
 * Func : _to_dp_pixel_mode
 *
 * Desc : convert DPRX_DRV_PIXEL_MODE_E pixel mode
 *        to DPRX_OUT_PIXEL_MODE
 *
 * Para : pixel_mode  : handle of DPRX adapter
 *
 * Retn : DPRX_OUT_PIXEL_MODE_1P
 *        DPRX_OUT_PIXEL_MODE_2P
 *        ...
 *-----------------------------------------------*/
unsigned char _to_dp_pixel_mode(DPRX_DRV_PIXEL_MODE_E pixel_mode)
{
    switch(pixel_mode) {
    case DPRX_DRV_ONE_PIXEL_MODE: return DPRX_OUT_PIXEL_MODE_1P;
    case DPRX_DRV_TWO_PIXEL_MODE: return DPRX_OUT_PIXEL_MODE_2P;
    default:
        DPRX_ADP_WARN("_to_dp_pixel_mode warning, unknow pixel mode %d, force 1p\n", pixel_mode);
        return DPRX_OUT_PIXEL_MODE_1P;
    }
}

void _check_info_frame_expiration(UINT32 frame_rate_hz)
{
    UINT32 frame_preoid;

    // get frame peroid
    if (frame_rate_hz < 20)
        frame_rate_hz = 20;    // Min frame rate
    else if (frame_rate_hz > 300)
        frame_rate_hz = 300;   // Max frame rate

    frame_preoid = ((1000 / frame_rate_hz) +1);  // frame perid in ms

    if (_check_sdp_buff_expiration(&g_dprx_sdp_handler.dvs, frame_preoid))    // for DolbyVision, the DVS should be updated every frame
    {
        DPRX_ADP_WARN("dolby vsion VSIF expired, force clear dolby vsif\n");
    }

    if (_check_sdp_buff_expiration(&g_dprx_sdp_handler.vsi, frame_preoid * 10))    // for non Dolby VSI
    {
        DPRX_ADP_WARN("VSI expired, force clear vsif\n");
    }

    if (_check_sdp_buff_expiration(&g_dprx_sdp_handler.adp_sync, frame_preoid))    // for Adaptive Sync, the DVS should be updated every frame
    {
        DPRX_ADP_WARN("Adaptive Sync SDP expired, force clear adaptive Sync SDP\n");
    }

    if (_check_sdp_buff_expiration(&g_dprx_sdp_handler.hdr, frame_preoid * 10))
    {
        DPRX_ADP_WARN("HDR INFO expired, force clear HDR SDP\n");
    }

    if (_check_sdp_buff_expiration(&g_dprx_sdp_handler.vsc, frame_preoid * 10))
    {
        DPRX_ADP_WARN("VSC INFO expired, force clear VSC\n");
    }

    if (_check_sdp_buff_expiration(&g_dprx_sdp_handler.spd, frame_preoid * 10))
    {
        DPRX_ADP_WARN("SPD INFO expired, force clear SPD, force disable freesync\n");
        g_dprx_sdp_handler.freesync_support = 0;
        g_dprx_sdp_handler.freesync_enable = 0;
        g_dprx_sdp_handler.freesync_activated = 0;
    }
}

/*------------------------------------------------
 * Func : rtk_dprx_get_video_timing
 *
 * Desc : get dprx port video timing
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_timing : pointer of timing info
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_get_video_timing(
    RTK_DPRX_ADP*       p_dprx,
    DPRX_TIMING_INFO_T* p_timing
    )
{
    DPRX_TIMING_INFO dprx_timing_info;

    if (p_dprx==NULL || p_timing==NULL)
        return -1;

    memset(p_timing, 0, sizeof(DPRX_TIMING_INFO_T));

    // check dprx port status
    if (g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
        return -1;

#ifdef CONFIG_DPRX_FORCE_CHECK_MODE_FAIL_CABLE_OFF_ON
    if (p_dprx->fake_cable_off)
        return -1;
#endif

#ifdef CHECK_SOURCE_IN_GET_CONNECT_STATE
    rtk_dprx_source_check(p_dprx);
#endif

    // get timing info from low level driver
    if (dprx_drv_get_timing_info(0, &dprx_timing_info)==_TRUE)
    {
        // check
        _check_info_frame_expiration(dprx_timing_info.vfreq_hz_x100/100);

        p_timing->htotal        = dprx_timing_info.htotal;
        p_timing->hstart        = dprx_timing_info.hstart;
        p_timing->hact          = dprx_timing_info.hact;
        p_timing->hsync         = dprx_timing_info.hsync;
        p_timing->hporch        = p_timing->htotal - p_timing->hact;
        p_timing->vtotal        = dprx_timing_info.vtotal;
        p_timing->vstart        = dprx_timing_info.vstart;
        p_timing->vact          = dprx_timing_info.vact;
        p_timing->vsync         = dprx_timing_info.vsync;
        p_timing->vporch        = p_timing->vtotal - p_timing->vact;
        p_timing->hfreq_hz      = dprx_timing_info.hfreq_hz;
        p_timing->vfreq_hz_x100 = dprx_timing_info.vfreq_hz_x100;
        p_timing->is_interlace  = dprx_timing_info.is_interlace;
        p_timing->is_dsc        = dprx_timing_info.is_dsc;
        p_timing->drr_mode      = _to_dp_drr_mode(dprx_timing_info.drr_mode);

        if (p_timing->drr_mode==DP_DRR_MODE_AMD_FREE_SYNC)
        {
            if ((g_dprx_sdp_handler.freesync_support==0) ||
                (g_dprx_sdp_handler.freesync_enable == 0 && g_dprx_sdp_handler.freesync_activated==0))
                p_timing->drr_mode = DP_DRR_MODE_FRR;
        }

        p_timing->curr_vfreq_hz_x100 = dprx_timing_info.curr_vfreq_hz_x100;
        p_timing->isALLM        = 0;       // not implemented yet

        p_timing->pixel_mode    = _to_dp_pixel_mode(dprx_timing_info.pixel_mode);
#ifdef DPRX_DRV_OFFMS_PIXEL_MODE_SUPPORT
        p_timing->pixel_mode_offms = _to_dp_pixel_mode(dprx_timing_info.pixel_mode_offms);
#else
        p_timing->pixel_mode_offms = DPRX_OUT_PIXEL_MODE_2P;  // for backward compatible (should be removed later)
#endif

        p_timing->is_dolby_hdr  = (g_dprx_sdp_handler.dvs.pkt_count) ? 1 : 0;
        p_timing->color_space   = _to_dp_color_space(dprx_timing_info.color_space);
        p_timing->color_depth   = (DP_COLOR_DEPTH_E)dprx_timing_info.color_depth;
        p_timing->hs_polarity   = HS_POL_POSITIVE;  // dprx drv always uses positive polarity
        p_timing->vs_polarity   = VS_POL_POSITIVE;  // dprx drv always uses positive polarity

        if (dprx_timing_info.drr_mode || g_force_demode)
            p_timing->ptg_mode = DP_PTG_REGEN_NO_VSYNC_MODE;
        else
            p_timing->ptg_mode = DP_PTG_REGEN_HV_MODE;

#ifdef CONFIG_DPRX_FORCE_CHECK_MODE_FAIL_CABLE_OFF_ON
        if (p_timing->htotal && p_timing->vtotal)
            p_dprx->output_enable = 1;
#endif
        return 0;
    }
    else
    {
        _check_info_frame_expiration(0);  // use default frame rate for infor frame check
    }

    return -1;
}


/*------------------------------------------------
 * Func : rtk_dprx_get_pixel_encoding_info
 *
 * Desc : get dprx port pixel format info
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_pixel : pointer of pixel encoding format
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_get_pixel_encoding_info(
    RTK_DPRX_ADP*       p_dprx,
    DP_PIXEL_ENCODING_FORMAT_T*    p_pixel
    )
{
    DPRX_PIXEL_ENCODING_FORMAT dprx_pixel_format;

    if (p_dprx==NULL || p_pixel==NULL)
        return -1;

    // check dprx port status
    if (g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
        return -1;

    memset(p_pixel, 0, sizeof(DP_PIXEL_ENCODING_FORMAT_T));

    if (dprx_drv_get_pixel_encoding_info(0, &dprx_pixel_format)!=_TRUE)
        return -1;

    // convert data types
    p_pixel->color_depth = dprx_pixel_format.color_depth_enum;

    p_pixel->color_space = _to_dp_color_space(dprx_pixel_format.color_space_enum);

    if (dprx_pixel_format.colorimetry_enum != _COLORIMETRY_EXT)
        p_pixel->colorimetry = _to_dp_colorimetry(dprx_pixel_format.colorimetry_enum);
    else
        p_pixel->colorimetry = _to_dp_colorimetry_ext(dprx_pixel_format.colorimetry_ext_enum);

    p_pixel->dynamic_range = _to_dp_dynamic_range(dprx_pixel_format.dynamic_range_enum);

    p_pixel->content_type = _to_dp_content_type(dprx_pixel_format.content_type_enum);

    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_get_drm_info
 *
 * Desc : get dprx port drm drm
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_drm : pointer of drm info
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_get_drm_info(
    RTK_DPRX_ADP*       p_dprx,
    DPRX_DRM_INFO_T*    p_drm
    )
{
#ifdef CONFIG_SUPPORT_GET_DRM_INFO

    DP_SDP_DATA_T hdr_sdp;

    if (p_drm==NULL)
        return -1;

    memset(p_drm, 0, sizeof(DPRX_DRM_INFO_T));

    if (p_dprx==NULL || g_p_current_dprx_port!=p_dprx ||
        p_dprx->connected==0 || rtk_dprx_get_sdp_data(p_dprx, DP_SDP_TYPE_HDR, &hdr_sdp)!=0)
        return -1;

    memcpy(p_drm, hdr_sdp.pb, sizeof(DPRX_DRM_INFO_T));
    return 0;

#else
    return -1;
#endif
}


/*------------------------------------------------
 * Func : rtk_dprx_get_vbid_data
 *
 * Desc : get vbid data from a given DPRX Port
 *
 * Para : p_dprx : handle of DPRX adapter
*         p_msa  : SDP type
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_get_vbid_data(
    RTK_DPRX_ADP*       p_dprx,
    DPRX_VBID_INFO_T*   p_vbid
    )
{
    UINT8 vbid;

    if (p_dprx==NULL || p_vbid==NULL)
        return -1;

    memset(p_vbid, 0, sizeof(DPRX_VBID_INFO_T));

    if (g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
        return 0;  // not current port, report zero mas

    vbid = 0xC0;//lib_dprx_vbid_read(0);
    memcpy(p_vbid, &vbid, 1);
    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_get_msa_data
 *
 * Desc : get msa data from a given DPRX Port
 *
 * Para : p_dprx : handle of DPRX adapter
*         p_msa  : SDP type
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_get_msa_data(
    RTK_DPRX_ADP*       p_dprx,
    DPRX_MSA_INFO_T*    p_msa
    )
{
    if (p_dprx==NULL || p_msa==NULL)
        return -1;

    memset(p_msa, 0, sizeof(DPRX_MSA_INFO_T));

    if (g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
        return 0;  // not current port, report zero mas

    p_msa->mvid         = lib_dprx_msa_mvid(0);
    p_msa->nvid         = lib_dprx_msa_nvid(0);
    p_msa->htotal       = lib_dprx_msa_h_total(0);
    p_msa->vtotal       = lib_dprx_msa_v_total(0);
    p_msa->hstart       = lib_dprx_msa_h_start(0);
    p_msa->vstart       = lib_dprx_msa_v_start(0);
    p_msa->hsync_polarity = lib_dprx_msa_hs_polarity(0);
    p_msa->vsync_polarity = lib_dprx_msa_vs_polarity(0);
    p_msa->hsync_width  = lib_dprx_msa_hs_width(0);
    p_msa->vsync_width  = lib_dprx_msa_vs_width(0);
    p_msa->hwidth       = lib_dprx_msa_h_active(0);
    p_msa->vwidth       = lib_dprx_msa_v_active(0);
    p_msa->misc0        = lib_dprx_msa_misc0(0);
    p_msa->misc1        = lib_dprx_msa_misc1(0);
    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_get_sdp_data
 *
 * Desc : get sdp data from a given DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        type   : SDP type
 *        p_data : spd data output
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_get_sdp_data(
    RTK_DPRX_ADP*       p_dprx,
    DP_SDP_TYPE         type,
    DP_SDP_DATA_T*      p_sdp
    )
{
    DPRX_SDP_PKT_BUFFER* p_sdp_buff = NULL;

    if (p_sdp==NULL || g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
        return -1;

    memset(p_sdp, 0, sizeof(DP_SDP_DATA_T));

    switch(type)
    {
    case DP_SDP_TYPE_VSC: p_sdp_buff = &g_dprx_sdp_handler.vsc; break;
    case DP_SDP_TYPE_SPD: p_sdp_buff = &g_dprx_sdp_handler.spd; break;
    case DP_SDP_TYPE_DVS: p_sdp_buff = &g_dprx_sdp_handler.dvs; break;
    case DP_SDP_TYPE_VSC_EXT_VESA: p_sdp_buff = &g_dprx_sdp_handler.vsc_ext_vesa; break;
    case DP_SDP_TYPE_HDR:
        p_sdp_buff = &g_dprx_sdp_handler.hdr;

        if (p_dprx->override_eotf_en) // to support override eotf, override eotf
        {
            p_sdp_buff->pkt_count++;
            p_sdp_buff->time_stamp = dprx_odal_get_system_time_ms();

            // fake hb
            p_sdp_buff->pkt.hb[0] = 0x00;
            p_sdp_buff->pkt.hb[1] = 0x87;   // pkt header
            p_sdp_buff->pkt.hb[2] = 0x1D;
            p_sdp_buff->pkt.hb[3] = 0x00;

            // fake db
            p_sdp_buff->pkt.pb[0] = 0x01;   // version
            p_sdp_buff->pkt.pb[1] = 0x1A;   // info frame size
            p_sdp_buff->pkt.pb[2] = p_dprx->override_eotf;  // return fake eotf
        }
        break;

    default:
        return -1;
    }

    if (p_sdp_buff->pkt_count)
    {
        p_sdp->seq_num = p_sdp_buff->pkt_count;
        p_sdp->time_stamp = p_sdp_buff->time_stamp;
        memcpy(p_sdp->hb, p_sdp_buff->pkt.hb, 4);  // copy header bytes
        memcpy(p_sdp->pb, p_sdp_buff->pkt.pb, 32); // copy data bytes
        return 0;
    }

    return -1;
}


#ifdef CONFIG_DPRX_STATUS_MONITOR

#define POLLING_INTERVAL        3000


/*------------------------------------------------
 * Func : _dprx_status_monitor
 *
 * Desc : monitor dprx status
 *
 * Para : port : dprx port
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
void _dprx_status_monitor(RTK_DPRX_ADP* p_dprx)
{
    DPRX_TIMING_INFO dprx_timing_info;

    if (p_dprx==NULL || g_p_current_dprx_port!=p_dprx ||
        p_dprx->connected==0)
        return ;

    if ((dprx_odal_get_system_time_ms() > p_dprx->status_monitor_time) &&
        (dprx_odal_get_system_time_ms() < p_dprx->status_monitor_time + POLLING_INTERVAL))
        return ;

    memset(&dprx_timing_info, 0, sizeof(dprx_timing_info));

    dprx_drv_get_timing_info(0, &dprx_timing_info);

    DPRX_ADP_WARN("Status : vfsm=%d(%s), crc=(%04x/%04x/%04x), vfreq=%d.%d (Hz)\n",
            GET_VIDEO_FSM(), _get_dprx_vfsm_str(GET_VIDEO_FSM()),
            g_dprx_video_crc[0], g_dprx_video_crc[1], g_dprx_video_crc[2],
            dprx_timing_info.curr_vfreq_hz_x100/100,
            (dprx_timing_info.curr_vfreq_hz_x100 % 100));

    // update polling time
    p_dprx->status_monitor_time = dprx_odal_get_system_time_ms();
}

#endif


/*------------------------------------------------
 * Func : rtk_dprx_source_check
 *
 * Desc : Check input source
 *
 * Para : port : dprx port
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_source_check(
    RTK_DPRX_ADP*       p_dprx
    )
{
    if (p_dprx==NULL)
        return -1;

    // check dprx port status
    if (g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
        return -1;

#ifdef CONFIG_DPRX_STATUS_MONITOR
    _dprx_status_monitor(p_dprx);
#endif

#ifdef CHECK_SOURCE_IN_GET_CONNECT_STATE
    p_dprx->source_check_time = dprx_odal_get_system_time_ms();
#endif

    // check input source
    if (dprx_drv_source_check(0)==_TRUE)
        return 0;

#ifdef CONFIG_DPRX_FORCE_CHECK_MODE_FAIL_CABLE_OFF_ON
    if (p_dprx->output_enable)
    {
        DPRX_ADP_WARN("check mode failed, !!!, stop video and restart fake cable off\n");
        p_dprx->fake_cable_off = 20;
    }
#endif

    return -1;
}


/*------------------------------------------------
 * Func : rtk_dprx_handle_online_measure_error
 *
 * Desc : handle on line measure error
 *
 * Para : p_dprx  : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_handle_online_measure_error(
    RTK_DPRX_ADP*       p_dprx
    )
{
#ifdef CONFIG_DPRX_VFE_ENABLE_ONLINE_MEASURE_ERROR
    INT8 ret = 0;
#endif
    // check dprx port status
    if (p_dprx==NULL || g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
    {
        DPRX_ADP_WARN("rtk_dprx_handle_online_measure_error failed, not current port\n");
        return -1;
    }

#ifdef CONFIG_DPRX_VFE_ENABLE_ONLINE_MEASURE_ERROR
    // stop video and restart timing check
    DPRX_ADP_WARN("rtk_dprx_handle_online_measure_error!!!, stop video and reset detect flow\n");
    ret = dprx_drv_source_check(0);  // redo source check
    DPRX_ADP_WARN("source check ret = %d\n", ret);
#endif

    return 0;
}

/*------------------------------------------------
 * Func : rtk_dprx_get_audio_status
 *
 * Desc : get dprx port audio status
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_audio_status : pointer of hdcp status
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_get_audio_status(
    RTK_DPRX_ADP*       p_dprx,
    DP_AUDIO_STATUS_T*  p_audio_status
    )
{
    if (p_dprx==NULL || p_audio_status==NULL)
        return -1;

    memset(p_audio_status, 0, sizeof(DP_AUDIO_STATUS_T));

    // check dprx port status
    if (g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
    {
        // force report no audio
        p_audio_status->coding_type = DP_AUDIO_CODING_NO_AUDIO;
        p_audio_status->ch_num = 0;
        p_audio_status->sampling_frequency_khz_x100 = 0;
        return 0;
    }

    // FIX ME: get audio status from Low level driver
    p_audio_status->coding_type = DP_AUDIO_CODING_LPCM;
    p_audio_status->ch_num = 5;
    p_audio_status->sampling_frequency_khz_x100 = 45000;
    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_get_hdcp_status
 *
 * Desc : get dprx port hdcp status
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_hdcp_status : pointer of hdcp status
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_get_hdcp_status(
    RTK_DPRX_ADP*       p_dprx,
    DPRX_HDCP_STATUS_T* p_hdcp_status
    )
{
    unsigned char rtx[8];
    unsigned char eks[16];

    if (p_dprx==NULL || p_hdcp_status==NULL)
        return -1;

    memset(p_hdcp_status, 0, sizeof(DPRX_HDCP_STATUS_T));

    // check dprx port status
    if (g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
    {
        p_hdcp_status->mode  = DP_HDCP_MODE_OFF;
        p_hdcp_status->state = DP_HDCP_STATE_IDEL;  // UNAUTH
        return 0;
    }

    // FIX ME: get hdcp status from Low level driver
    p_hdcp_status->mode  = DP_HDCP_MODE_OFF;
    p_hdcp_status->state = DP_HDCP_STATE_IDEL;  // UNAUTH

    rtk_dprx_get_dpcd(p_dprx, DPCD_68000_BKSV, p_hdcp_status->hdcp1x.bksv, HDCP1X_KSV_INFO_LEN);
    rtk_dprx_get_dpcd(p_dprx, DPCD_68007_AKSV_0, p_hdcp_status->hdcp1x.aksv, HDCP1X_KSV_INFO_LEN);
    rtk_dprx_get_dpcd(p_dprx, DPCD_6800C_AN_0, p_hdcp_status->hdcp1x.an, 8);
    rtk_dprx_get_dpcd(p_dprx, DPCD_68005_R0_PRIME, p_hdcp_status->hdcp1x.ri, 2);
    rtk_dprx_get_dpcd(p_dprx, DPCD_68028_BCAPS, &p_hdcp_status->hdcp1x.bcaps, 1);
    rtk_dprx_get_dpcd(p_dprx, DPCD_68029_BSTATUS, p_hdcp_status->hdcp1x.bstatus, 2);

    rtk_dprx_get_dpcd(p_dprx, DPCD_69000_RTX, rtx, 8);
    rtk_dprx_get_dpcd(p_dprx, DPCD_69318_E_DKEY_KS, eks, sizeof(eks));

    if (p_hdcp_status->hdcp1x.aksv[0]!=0 ||
        p_hdcp_status->hdcp1x.aksv[1]!=0 ||
        p_hdcp_status->hdcp1x.aksv[2]!=0 ||
        p_hdcp_status->hdcp1x.aksv[3]!=0 ||
        p_hdcp_status->hdcp1x.aksv[4]!=0)
    {
        p_hdcp_status->mode  = DP_HDCP_MODE_HDCP_1P3;

        if (p_hdcp_status->hdcp1x.ri[0]==0 && p_hdcp_status->hdcp1x.ri[1]==0)
            p_hdcp_status->state  = DP_HDCP_STATE_AUTH_START;
        else
            p_hdcp_status->state  = DP_HDCP_STATE_AUTH_DONE;

        return 0;
    }
    else if (rtx[0]!=0 || rtx[1]!=0 ||rtx[2]!=0 ||rtx[3]!=0 || rtx[4]!=0 || rtx[5]!=0 ||rtx[6]!=0 ||rtx[7]!=0)
    {
        p_hdcp_status->mode  = DP_HDCP_MODE_HDCP_2P3;

        if (eks[0]!=0 || eks[1]!=0 ||eks[2]!=0 ||eks[3]!=0 || eks[4]!=0 || eks[5]!=0 ||eks[6]!=0 ||eks[7]!=0 ||
            eks[8]!=0 || eks[9]!=0 || eks[10]!=0 ||eks[11]!=0 ||eks[12]!=0 || eks[13]!=0 || eks[14]!=0 ||eks[15]!=0 )
            p_hdcp_status->state  = DP_HDCP_STATE_AUTH_DONE;
        else
            p_hdcp_status->state  = DP_HDCP_STATE_AUTH_START;
    }

    return 0;
}


//===============================================================================
// DPCD related
//===============================================================================

/*------------------------------------------------
 * Func : rtk_dprx_set_dpcd
 *
 * Desc : set DPCD of the DPRX adapter
 *
 * Para : p_dprx  : handle of DPRX adapter
 *        address : dpcd address
 *        p_dpcd  : dpcd data
 *        len     : size of dpcd data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_set_dpcd(
    RTK_DPRX_ADP*       p_dprx,
    unsigned long       address,
    unsigned char*      p_dpcd,
    unsigned short      len
    )
{
    if (p_dprx==NULL || p_dpcd==NULL || len==0 || address > 0xFFFFF)
        return -1;

    if (g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
    {
        DPRX_ADP_WARN("rtk_dprx_set_dpcd to a not current port, just ignore it\n");
        return 0;
    }

    while (len--)
    {
        lib_dprx_dpcd_write_manual_mode(0, address++, *p_dpcd++);
    }

    return 0;
}

/*------------------------------------------------
 * Func : rtk_dprx_get_dpcd
 *
 * Desc : get DPCD of the DPRX adapter
 *
 * Para : p_dprx  : handle of DPRX adapter
 *        address : dpcd address
 *        p_dpcd  : dpcd data
 *        len     : size of dpcd data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_get_dpcd(
    RTK_DPRX_ADP*       p_dprx,
    unsigned long       address,
    unsigned char*      p_dpcd,
    unsigned short      len
    )
{
    if (p_dprx==NULL || p_dpcd==NULL || len==0 || address > 0xFFFFF)
        return -1;

    memset(p_dpcd, 0, len);

    if (g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
    {
        DPRX_ADP_WARN("rtk_dprx_get_dpcd from a not current port, return zero to instead\n");
        return 0;
    }

    lib_dprx_dpcd_read_bytes(0, address, p_dpcd, len);

    return 0;
}

//===============================================================================
// Extension Control
//===============================================================================

/*------------------------------------------------
 * Func : rtk_dprx_set_ext_ctrl
 *
 * Desc : set ext control of a DPRX Port
 *
 * Para : p_dprx : handle of DPRX adapter
 *        ctrl : ext control command
 *
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_set_ext_ctrl(
    RTK_DPRX_ADP*   p_dprx,
    ADP_EXCTL_ID    ctrl,
    unsigned int    val
    )
{
    if (p_dprx==NULL)
        return -1;

    switch(ctrl)
    {
    case ADP_EXCTL_OVERRIDE_EOTF_EN:
        p_dprx->override_eotf_en = (val) ? 1 : 0;
        break;
    case ADP_EXCTL_OVERRIDE_EOTF:
        if (val > 0x7)
            return -1;
        p_dprx->override_eotf = val;
        break;
    // PHY
    case ADP_EXCTL_MANUAL_EQ_EN:
        p_dprx->manul_eq_en = (val) ? 1 : 0;
        break;
    // HDCP
    case ADP_EXCTL_DISABLE_HDCP14:
        p_dprx->hdcp_1x_disable_en = (val) ? 1 : 0;
        break;
    case ADP_EXCTL_DISABLE_HDCP22:
        p_dprx->hdcp_2x_disable_en = (val) ? 1 : 0;
        break;
    case ADP_EXCTL_FRORCE_REAUTH_HDCP22:
        p_dprx->hdcp_2x_force_reauth = (val) ? 1 : 0;
        break;
    case ADP_EXCTL_WATCH_DOG_ENABLE:
        DPRX_DRV_SET_VIDEO_WATCH_DOG(0, (val) ? 1:0);
        break;
    default:
        return -1;
    }
    return 0;
}

/*------------------------------------------------
 * Func : rtk_dprx_set_ext_ctrl
 *
 * Desc : set ext control of a DPRX Port
 *
 * Para : p_dprx : handle of DPRX adapter
 *        ctrl : ext control command
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_get_ext_ctrl(
    RTK_DPRX_ADP*   p_dprx,
    ADP_EXCTL_ID    ctrl
    )
{
    if (p_dprx==NULL)
        return -1;

    switch(ctrl)
    {
    // HDR
    case ADP_EXCTL_OVERRIDE_EOTF_EN:     return p_dprx->override_eotf_en;        break;
    case ADP_EXCTL_OVERRIDE_EOTF:        return p_dprx->override_eotf;           break;
    // PHY
    case ADP_EXCTL_MANUAL_EQ_EN:         return p_dprx->manul_eq_en;             break;
    // HDCP
    case ADP_EXCTL_DISABLE_HDCP14:       return p_dprx->hdcp_1x_disable_en;      break;
    case ADP_EXCTL_DISABLE_HDCP22:       return p_dprx->hdcp_2x_disable_en;      break;
    case ADP_EXCTL_FRORCE_REAUTH_HDCP22: return p_dprx->hdcp_2x_force_reauth;    break;
        break;
    default:
        return -1;
    }

    return 0;
}

//----------------------------------------------------------------------------------------
// Power Management API
//----------------------------------------------------------------------------------------

/*------------------------------------------------
 * Func : rtk_dprx_suspend
 *
 * Desc : suspend a rtk dprx adapter
 *
 * Para : p_dprx  : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_suspend(
    RTK_DPRX_ADP*       p_dprx,
    UINT32              mode
    )
{
    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("suspend failed, invalid argments!!!!\n");
        return -1;
    }

    DPRX_ADP_INFO("suspend, p_dprx->suspend=%d!!!!\n", p_dprx->suspend);

    if (p_dprx->suspend==0)
    {
        p_dprx->suspend = 1; // setup suspend flag
        p_dprx->resume_connect = 0;

        // 2. power off DPRX if it is current port
        if (g_p_current_dprx_port == p_dprx)
        {
            if ((mode & WAKEUP_BY_AUX))
                newbase_dprx_dpcd_set_set_power_state(0, 0x5);  // change set the power state to D3
            else
                rtk_dprx_port_swap(NULL);     // deactiavte currect port

            p_dprx->resume_connect = 1;   // auto connect after resume
        }

        // 2. put hpd to low (optional)
        if ((mode & WAKEUP_BY_AUX)==0)
            dprx_hpd_set_hpd(p_dprx->p_hpd, 0);   // power down hpd
    }

    g_p_dprx_edid_resume = 0;

    return 0;
}


/*------------------------------------------------
 * Func : rtk_dprx_resume
 *
 * Desc : resume a rtk dprx adapter
 *
 * Para : p_dprx  : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int rtk_dprx_resume(
    RTK_DPRX_ADP*       p_dprx
    )
{
    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("resume failed, invalid argments!!!!\n");
        return -1;
    }

    DPRX_ADP_INFO("resume, p_dprx->suspend=%d, resume_connect=%d\n", p_dprx->suspend, p_dprx->resume_connect);

    // resume edid
    if (g_p_dprx_edid && g_p_dprx_edid_resume==0)
    {
        dprx_edid_reset(g_p_dprx_edid);  // force reset edid
        g_p_dprx_edid_resume = 1;
    }

    if (p_dprx->suspend)
    {
        // clear suspend flag
        p_dprx->suspend = 0;

#ifdef FORCE_RE_ALT_MODE
        if(p_dprx->type == DP_TYPE_USB_TYPE_C)
            dprx_hpd_set_typec_re_alt_mode(p_dprx->p_hpd);
#endif

        // reinit dprx mac
        if (p_dprx->resume_connect)
        {
            p_dprx->resume_connect = 0;
            DPRX_ADP_INFO("current port, do port swap to reconnect!!!!\n");
            rtk_dprx_port_swap(NULL);     // change dprx port
            rtk_dprx_plat_interrupt_enable();  // re-enable interrupt
            rtk_dprx_port_swap(p_dprx);   // change dprx port
        }
        else
        {
            DPRX_ADP_INFO("not current port, do nothing!!!!\n");
        }
    }

    return 0;
}


//----------------------------------------------------------------------------------------
// Event Handler API
//----------------------------------------------------------------------------------------

const char* sdp_type_str(SDP_TYPE_E sdp_type)
{
    switch(sdp_type)
    {
    case SDP_TYPE_AUD_TIMESTAMP:       return "AUDIO_TIME_STAMP";
    case SDP_TYPE_AUD_STREAM:          return "AUDIO_STREAM";
    case SDP_TYPE_EXTENSION:           return "EXT";
    case SDP_TYPE_AUD_COPY_MANAGEMENT: return "ACM";
    case SDP_TYPE_ISRC:                return "ISRC";
    case SDP_TYPE_VSC:                 return "VSC";
    case SDP_TYPE_PPS:                 return "PPS";
    case SDP_TYPE_VSC_EXT_VESA:        return "VSC_EXT_VESA";
    case SDP_TYPE_VSC_EXT_CEA:         return "VSC_EXT_CEA";
    case SDP_TYPE_ADAPTIVE_SYNC_SDP:   return "ADPT_SYNC";
    case SDP_TYPE_INFOFRAME_VENDOR_SPEC:  return "CTA-VSI";
    case SDP_TYPE_INFOFRAME_AUX_VID_INFO: return "CTA-AVI";
    case SDP_TYPE_INFOFRAME_SPD:       return "CTA-SPD";
    case SDP_TYPE_INFOFRAME_AUDIO:     return "CTA-AUDIO_INFO";
    case SDP_TYPE_INFOFRAME_HDR:       return "CTA-HDR";
    default:                           return "Unknwon";
    }
}

/*------------------------------------------------
 * Func : _update_sdp_debug_flag
 *
 * Desc : update sdp debug flag.
 *
 * Para :
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
void _update_sdp_debug_flag(void)
{
    g_dprx_sdp_handler.dvs.debug_en = (g_dprx_sdp_debug_ctrl & DEBUG_DOLBY_VIOSN_SDP) ? 1 : 0;
    g_dprx_sdp_handler.spd.debug_en = (g_dprx_sdp_debug_ctrl & DEBUG_AMD_FREESYNC_SPD_SDP) ? 1 : 0;
    g_dprx_sdp_handler.vsc_ext_vesa.debug_en = (g_dprx_sdp_debug_ctrl & DEBUG_VSC_EXT_VESA_SDP) ? 1 : 0;
}

/*------------------------------------------------
 * Func : rtk_dprx_handle_infoframe_sdp_update_event
 *
 * Desc : get sdp data from a given DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        sdp_type : sdp type
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
void rtk_dprx_handle_infoframe_sdp_update_event(
    RTK_DPRX_ADP*       p_dprx,
    SDP_TYPE_E          sdp_type
    )
{
    DPRX_SDP_PKT_BUFFER* p_sdp_buff = NULL;
    DPRX_SDP_PKT sdp_pkt;

    if (p_dprx==NULL || g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
        return ;

    dprx_drv_read_sdp_packet(0, sdp_type, &sdp_pkt);

    switch(sdp_type)
    {
    case SDP_TYPE_INFOFRAME_VENDOR_SPEC:
        if (sdp_pkt.pb[0]==0x1 && sdp_pkt.pb[1]==0x1B && sdp_pkt.pb[2]==0x46 && sdp_pkt.pb[3]==0xD0 && sdp_pkt.pb[4]==0x00)
            p_sdp_buff = &g_dprx_sdp_handler.dvs;
        else
            p_sdp_buff = &g_dprx_sdp_handler.vsi;
        break;

    case SDP_TYPE_INFOFRAME_HDR: p_sdp_buff = &g_dprx_sdp_handler.hdr; break;
    case SDP_TYPE_VSC:           p_sdp_buff = &g_dprx_sdp_handler.vsc; break;
    case SDP_TYPE_INFOFRAME_SPD: p_sdp_buff = &g_dprx_sdp_handler.spd; break;
    case SDP_TYPE_ADAPTIVE_SYNC_SDP: p_sdp_buff = &g_dprx_sdp_handler.adp_sync; break;
    case SDP_TYPE_VSC_EXT_VESA:  p_sdp_buff = &g_dprx_sdp_handler.vsc_ext_vesa; break;
    default:
        break;
    }

    if (p_sdp_buff==NULL)
        return ;  // ignrore

    p_sdp_buff->time_stamp = dprx_odal_get_system_time_ms();
    _update_sdp_debug_flag();

    if (p_sdp_buff->pkt_count<0xFFFFFFFF)
        p_sdp_buff->pkt_count++;          // increase packet count

    // restore pkt buffer
    memcpy(&p_sdp_buff->pkt, &sdp_pkt, sizeof(sdp_pkt));

    if (p_sdp_buff->debug_en)
    {
        DPRX_ADP_INFO("sdp_update (type=%02x, %s, %d) HB=%02x %02x %02x %02x, PB=%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x\n",
                sdp_type,
                sdp_type_str(sdp_type),
                p_sdp_buff->pkt_count,
                sdp_pkt.hb[0],  sdp_pkt.hb[1], sdp_pkt.hb[2], sdp_pkt.hb[3],
                sdp_pkt.pb[0],  sdp_pkt.pb[1], sdp_pkt.pb[2], sdp_pkt.pb[3],
                sdp_pkt.pb[4],  sdp_pkt.pb[5], sdp_pkt.pb[6], sdp_pkt.pb[7],
                sdp_pkt.pb[8],  sdp_pkt.pb[9], sdp_pkt.pb[10], sdp_pkt.pb[11],
                sdp_pkt.pb[12], sdp_pkt.pb[13], sdp_pkt.pb[14], sdp_pkt.pb[15],
                sdp_pkt.pb[16], sdp_pkt.pb[17], sdp_pkt.pb[18], sdp_pkt.pb[19],
                sdp_pkt.pb[20], sdp_pkt.pb[21], sdp_pkt.pb[22], sdp_pkt.pb[23],
                sdp_pkt.pb[24], sdp_pkt.pb[25], sdp_pkt.pb[26], sdp_pkt.pb[27],
                sdp_pkt.pb[28], sdp_pkt.pb[29], sdp_pkt.pb[30], sdp_pkt.pb[31]);
    }

#ifdef CONFIG_DPRX_EANBLE_FREESYNC
    if (sdp_type==SDP_TYPE_INFOFRAME_SPD)
    {
        UINT8 freesync_en = newbase_dprx_sdp_get_amd_spd_info(0, _SPD_INFO_FREESYNC_ENABLE) ? 1 : 0;
        UINT8 freesync_act = newbase_dprx_sdp_get_amd_spd_info(0, _SPD_INFO_FREESYNC_ENABLE) ? 1 : 0;
        UINT8 freesync_support = newbase_dprx_sdp_get_amd_spd_info(0, _SPD_INFO_FREESYNC_ENABLE) ? 1 : 0;

        if (g_dprx_sdp_handler.freesync_enable != freesync_en||
            g_dprx_sdp_handler.freesync_activated != freesync_act ||
            g_dprx_sdp_handler.freesync_support != freesync_support)
        {
            g_dprx_sdp_handler.freesync_enable    = freesync_en;
            g_dprx_sdp_handler.freesync_activated = freesync_act;
            g_dprx_sdp_handler.freesync_support   = freesync_support;

            DPRX_ADP_INFO("AMD SDP updated (fs_en=%d, fs_act=%d, fs_support=%d)\n",
                            freesync_en, freesync_act, freesync_support);
        }
    }
#endif
}

#ifdef ENABLE_DPRX_LT_EVENT_MONITOR

/*------------------------------------------------
 * Func : rtk_dprx_handle_lt_state_update_event
 *
 * Desc : handle lt state update event
 *
 * Para : p_adp : handle of DPRX adapter
 *        state : lt state type
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
void rtk_dprx_handle_lt_state_update_event(
    RTK_DPRX_ADP*       p_dprx,
    UINT8               state
    )
{
    if (p_dprx==NULL || g_p_current_dprx_port!=p_dprx || p_dprx->connected==0)
        return ;

    if (p_dprx->link_state != state)
    {
        if (p_dprx->link_state==_DP_NORMAL_TRAINING_PATTERN_1_RUNNING ||
            p_dprx->link_state==_DP_FAKE_TRAINING_PATTERN_1_RUNNING)
            p_dprx->link_round++;

        p_dprx->link_state = state;
        p_dprx->link_sub_state = 0;
    }
    else
    {
        p_dprx->link_sub_state++;
    }

    if (rtk_dprx_lt_event_message_enable(p_dprx))
    {
        DPRX_ADP_INFO("lt_state (%d-%02x-%d) lane=%d, bw=%02x (sink status : %02x-%02x-%02x)!!!\n",
            p_dprx->link_round,
            p_dprx->link_state,
            p_dprx->link_sub_state,
            newbase_dprx_dpcd_get_lane_count(0),
            newbase_dprx_dpcd_get_link_bw_set(0),
            newbase_dprx_dpcd_get_lane_01_status(0),
            newbase_dprx_dpcd_get_lane_23_status(0),
            newbase_dprx_dpcd_get_lane_align_status(0));
    }
}

#endif