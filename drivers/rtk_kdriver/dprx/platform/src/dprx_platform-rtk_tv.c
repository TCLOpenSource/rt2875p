/*=============================================================
 * File:    dprx_platform-rtk_tv.c
 *
 * Desc:    DPRX Platform for RTK TV
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
#include <dprx_platform-rtk_tv.h>
#ifdef CONFIG_DPRX_VFE_ENABLE_QUICK_SHOW
#include <quick_show/quick_show.h>
#endif

static unsigned char g_dprx_channel_port[PLAT_DPRX_PORT_NUM];
static DP_PORT_CFG   g_dprx_port_cfg[PLAT_DPRX_PORT_NUM];

/*------------------------------------------------
 * Func : _get_pcb_rx_port_info
 *
 * Desc : get pcb rx port configurations from PCB Mgr
 *
 * Para : port  : dp port
 *        p_cfg : port configirations
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _get_pcb_rx_port_info(unsigned char port, DP_PORT_CFG* p_cfg)
{
    unsigned long long val;
    char name[16] ={0};

    if (p_cfg==NULL)
        return -1;

    memset(p_cfg, 0, sizeof(DP_PORT_CFG));

    snprintf(name, sizeof(name)-1, "DP_PORT%d_CFG", port+1);

    if (pcb_mgr_get_enum_info_byname(name, &val) < 0)
    {
        DP_PLAT_WARN("Get PCB Param - %s failed \n" , name );
        return -1;
    }
    else
    {
        DP_PLAT_WARN("Get PCB Param - %s successed \n" , name );
    }

    if (GET_DP_PORT_VALID(val)==0)
    {
        DP_PLAT_WARN("parse pcb parameter - %s failed. invalid port config, force to null dp\n" , name);
        p_cfg->type = DP_TYPE_NONE;
        return 0;
    }

    switch(GET_DP_PORT_TYPE(val))
    {
    case DP_PORT_TYPE_DP:
    case DP_PORT_TYPE_mDP:

        // type
        p_cfg->type = (GET_DP_PORT_TYPE(val)==DP_PORT_TYPE_mDP) ? DP_TYPE_mDP : DP_TYPE_DP;

        // aux edid
        p_cfg->config.dp_mdp.edid.external_edid = 0;
        p_cfg->config.dp_mdp.edid.ddc_i2c_idx   = GET_DP_DDC_CH_SEL(val);

        // aux configuration
        p_cfg->config.dp_mdp.aux.idx            = GET_DP_AUX_CH_SEL(val);
        p_cfg->config.dp_mdp.aux.pn_swap        = GET_DP_AUX_PN_SWAP(val);

        // phy configuration
        p_cfg->config.dp_mdp.phy.idx            = GET_DP_PHY_ID(val);

        switch(GET_MAX_LANE_CNT(val))
        {
        case 0:
            p_cfg->config.dp_mdp.phy.max_lane_count = 1;
            break;
        case 1:
            p_cfg->config.dp_mdp.phy.max_lane_count = 2;
            break;
        default:
        case 2:
            p_cfg->config.dp_mdp.phy.max_lane_count = 4;
            break;
        }

        p_cfg->config.dp_mdp.phy.lane0_phy_idx  = GET_DP_LANE0_PHY_MAPPING(val);
        p_cfg->config.dp_mdp.phy.lane1_phy_idx  = GET_DP_LANE1_PHY_MAPPING(val);
        p_cfg->config.dp_mdp.phy.lane2_phy_idx  = GET_DP_LANE2_PHY_MAPPING(val);
        p_cfg->config.dp_mdp.phy.lane3_phy_idx  = GET_DP_LANE3_PHY_MAPPING(val);
        p_cfg->config.dp_mdp.phy.lane0_pn_swap  = GET_DP_LANE0_PN_SWAP(val);
        p_cfg->config.dp_mdp.phy.lane1_pn_swap  = GET_DP_LANE1_PN_SWAP(val);
        p_cfg->config.dp_mdp.phy.lane2_pn_swap  = GET_DP_LANE2_PN_SWAP(val);
        p_cfg->config.dp_mdp.phy.lane3_pn_swap  = GET_DP_LANE3_PN_SWAP(val);

        // DET GPIO
        switch(GET_DP_DET_GPIO_TYPE(val))
        {
        case DP_DET_GPIO_TYPE_MISC:
            p_cfg->config.dp_mdp.hpd_det.det_gpio_valid   = 1;
            p_cfg->config.dp_mdp.hpd_det.det_gpio_type    = 0;  // misc
            p_cfg->config.dp_mdp.hpd_det.det_gpio         = GET_DP_DET_GPIO_ID(val);
            break;
        case DP_DET_GPIO_TYPE_ISO:
            p_cfg->config.dp_mdp.hpd_det.det_gpio_valid   = 1;
            p_cfg->config.dp_mdp.hpd_det.det_gpio_type    = 1;  // iso
            p_cfg->config.dp_mdp.hpd_det.det_gpio         = GET_DP_DET_GPIO_ID(val);;
            break;
        default:
        case DP_DET_GPIO_TYPE_NONE:
            break;
        }

        // HPD GPIO
        switch(GET_DP_HPD_GPIO_TYPE(val))
        {
        case DP_HPD_GPIO_TYPE_MISC:
            p_cfg->config.dp_mdp.hpd_det.hpd_gpio_valid   = 1;
            p_cfg->config.dp_mdp.hpd_det.hpd_gpio_type    = 0;  // misc
            p_cfg->config.dp_mdp.hpd_det.hpd_gpio         = GET_DP_HPD_GPIO_ID(val);
            break;
        case DP_HPD_GPIO_TYPE_ISO:
            p_cfg->config.dp_mdp.hpd_det.hpd_gpio_valid   = 1;
            p_cfg->config.dp_mdp.hpd_det.hpd_gpio_type    = 1;  // iso
            p_cfg->config.dp_mdp.hpd_det.hpd_gpio         = GET_DP_HPD_GPIO_ID(val);;
            break;
        default:
        case DP_HPD_GPIO_TYPE_NONE:
            break;
        }

        // AUX_P GPIO
        switch(GET_DP_AUX_P_GPIO_TYPE(val))
        {
        case DP_AUX_P_GPIO_TYPE_MISC:
            p_cfg->config.dp_mdp.hpd_det.aux_p_gpio_valid   = 1;
            p_cfg->config.dp_mdp.hpd_det.aux_p_gpio_type    = 0;  // misc
            p_cfg->config.dp_mdp.hpd_det.aux_p_gpio         = GET_DP_AUX_P_GPIO_ID(val);
            break;
        case DP_AUX_P_GPIO_TYPE_ISO:
            p_cfg->config.dp_mdp.hpd_det.aux_p_gpio_valid   = 1;
            p_cfg->config.dp_mdp.hpd_det.aux_p_gpio_type    = 1;  // iso
            p_cfg->config.dp_mdp.hpd_det.aux_p_gpio         = GET_DP_AUX_P_GPIO_ID(val);;
            break;
        default:
        case DP_AUX_P_GPIO_TYPE_NONE:
            break;
        }

        // AUX_N GPIO
        switch(GET_DP_AUX_N_GPIO_TYPE(val))
        {
        case DP_AUX_N_GPIO_TYPE_MISC:
            p_cfg->config.dp_mdp.hpd_det.aux_n_gpio_valid   = 1;
            p_cfg->config.dp_mdp.hpd_det.aux_n_gpio_type    = 0;  // misc
            p_cfg->config.dp_mdp.hpd_det.aux_n_gpio         = GET_DP_AUX_N_GPIO_ID(val);
            break;
        case DP_AUX_N_GPIO_TYPE_ISO:
            p_cfg->config.dp_mdp.hpd_det.aux_n_gpio_valid   = 1;
            p_cfg->config.dp_mdp.hpd_det.aux_n_gpio_type    = 1;  // iso
            p_cfg->config.dp_mdp.hpd_det.aux_n_gpio         = GET_DP_AUX_N_GPIO_ID(val);;
            break;
        default:
        case DP_AUX_N_GPIO_TYPE_NONE:
            break;
        }
        break;

    case DP_PORT_TYPE_TYPE_C:

        // type
        p_cfg->type = DP_TYPE_USB_TYPE_C;

        // aux edid
        p_cfg->config.typec_dp.edid.external_edid = 0;
        p_cfg->config.typec_dp.edid.ddc_i2c_idx   = GET_DP_DDC_CH_SEL(val);

        // aux configuration
        p_cfg->config.typec_dp.aux.idx            = GET_DP_AUX_CH_SEL(val);
        p_cfg->config.typec_dp.aux.pn_swap        = GET_DP_AUX_PN_SWAP(val);

        // phy configuration
        p_cfg->config.typec_dp.phy.idx            = GET_DP_PHY_ID(val);

        switch(GET_MAX_LANE_CNT(val))
        {
        case 0:
            p_cfg->config.typec_dp.phy.max_lane_count = 1;
            break;
        case 1:
            p_cfg->config.typec_dp.phy.max_lane_count = 2;
            break;
        default:
        case 2:
            p_cfg->config.typec_dp.phy.max_lane_count = 4;
            break;
        }
        p_cfg->config.typec_dp.phy.lane0_phy_idx  = GET_DP_LANE0_PHY_MAPPING(val);
        p_cfg->config.typec_dp.phy.lane1_phy_idx  = GET_DP_LANE1_PHY_MAPPING(val);
        p_cfg->config.typec_dp.phy.lane2_phy_idx  = GET_DP_LANE2_PHY_MAPPING(val);
        p_cfg->config.typec_dp.phy.lane3_phy_idx  = GET_DP_LANE3_PHY_MAPPING(val);
        p_cfg->config.typec_dp.phy.lane0_pn_swap  = GET_DP_LANE0_PN_SWAP(val);
        p_cfg->config.typec_dp.phy.lane1_pn_swap  = GET_DP_LANE1_PN_SWAP(val);
        p_cfg->config.typec_dp.phy.lane2_pn_swap  = GET_DP_LANE2_PN_SWAP(val);
        p_cfg->config.typec_dp.phy.lane3_pn_swap  = GET_DP_LANE3_PN_SWAP(val);

        // PD configuration
        p_cfg->config.typec_dp.pd_port_idx = GET_DP_PORT_TYPEC_DP_SEL(val);
        break;

    case DP_PORT_TYPE_PSEUDO_DP:

        p_cfg->type = DP_TYPE_PSEUDO_DP;
        break;

    default:
        DP_PLAT_WARN("parse pcb parameter - %s failed. unknown dp type %d\n", name, (unsigned char) GET_DP_PORT_TYPE(val));
        p_cfg->type = DP_TYPE_NONE;
        return 0;
    }

    return 0;
}


/*------------------------------------------------
 * Func : _load_dprx_cfg
 *
 * Desc : load DPRX configuration
 *
 * Para : N/A
 *
 * Retn : N/A
 *-----------------------------------------------*/
static void _load_dprx_cfg(void)
{
    unsigned int i;
    unsigned long long val = 0;

    memset(g_dprx_channel_port, 0xFF, sizeof(g_dprx_channel_port));
    memset(g_dprx_port_cfg, 0, sizeof(g_dprx_port_cfg));

    // get port order config
    if (pcb_mgr_get_enum_info_byname("DP_PORT_ORDER", &val) < 0)
    {
        DP_PLAT_WARN("Get PCB Param - DP_PORT_ORDER failed, force to default value\n");

        for (i=0; i<PLAT_DPRX_PORT_NUM; i++) {
            g_dprx_channel_port[i] = i;
        }
    }
    else
    {
        for (i=0; i<PLAT_DPRX_PORT_NUM; i++)
        {
            g_dprx_channel_port[i] = val & 0xF;

            if (g_dprx_channel_port[i]==0)
                g_dprx_channel_port[i] = 0xFF;
            else
                g_dprx_channel_port[i]--;
            val >>= 4;
        }
    }

    // get port config
    for (i=0; i<PLAT_DPRX_PORT_NUM; i++)
    {
        if (_get_pcb_rx_port_info(i, &g_dprx_port_cfg[i])<0)
            g_dprx_port_cfg[i].type = DP_TYPE_PSEUDO_DP;   // force init it as a pseudo DP
    }
}


/*------------------------------------------------
 * Func : dprx_platform_init
 *
 * Desc : init DPRX platorm
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR
 *-----------------------------------------------*/
int dprx_platform_init(void)
{
    _load_dprx_cfg();
    return 0;
}


/*------------------------------------------------
 * Func : dprx_platform_uninit
 *
 * Desc : uninit DPRX platform setting. all resources
 *        will be released.
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR
 *-----------------------------------------------*/
int dprx_platform_uninit(void)
{
    // TODO: release preallocated resources
    return 0;
}


/*------------------------------------------------
 * Func : dprx_platform_get_port_config
 *
 * Desc : get config of a given DP port
 *
 * Para : idx :
 *
 * Retn : 0 : success, -1 failed
 *-----------------------------------------------*/
int dprx_platform_get_port_config(unsigned char port, DP_PORT_CFG* p_cfg)
{
    if (p_cfg == NULL || port >= PLAT_DPRX_PORT_NUM)
        return -1;

    memcpy(p_cfg, &g_dprx_port_cfg[port], sizeof(DP_PORT_CFG));

#ifdef CONFIG_DPRX_VFE_ENABLE_QUICK_SHOW
    if (dprx_platform_get_quick_show_enable())
    {
        unsigned char ch = 0xF;
        dprx_platform_get_dprx_channel(port, &ch);

        p_cfg->qs_ctrl = DP_QS_INIT_READY;

        if (ch == dprx_platform_get_quick_show_channel())
            p_cfg->qs_ctrl |= DP_QS_CONNECT_READY;
    }
#endif
    return 0;
}


/*------------------------------------------------
 * Func : dprx_platform_get_dprx_port
 *
 * Desc : get physical DPRX port id of a DP channel.
 *
 * Para : channel : Logical DP Channel
 *        p_port  : Physical DP port output
 *
 * Retn : 0 : success, -1 failed
 *-----------------------------------------------*/
int dprx_platform_get_dprx_port(unsigned char channel, unsigned char* p_port)
{
    if (p_port==NULL || channel >= PLAT_DPRX_PORT_NUM || g_dprx_channel_port[channel]==0xFF)
        return -1;

    *p_port = g_dprx_channel_port[channel];
    return 0;
}


/*------------------------------------------------
 * Func : dprx_platform_get_dprx_channel
 *
 * Desc : get logical DPRX channel of a given DP port.
 *
 * Para : port      : Physical DP port
 *        p_channel : Logical DP Channel output
 *
 * Retn : 0 : success, -1 failed
 *-----------------------------------------------*/
int dprx_platform_get_dprx_channel(unsigned char port, unsigned char* p_channel)
{
    int i;

    if (p_channel==NULL)
        return -1;

    for (i=0; i<PLAT_DPRX_PORT_NUM; i++)
    {
        if (g_dprx_channel_port[i]==port)
        {
            *p_channel = i;
            return 0;
        }
    }
    *p_channel = 0xFF;
    return -1;
}

#ifdef CONFIG_DPRX_VFE_ENABLE_QUICK_SHOW

/*------------------------------------------------
 * Func : dprx_platform_get_quick_show_enable
 *
 * Desc : check if quick show enabled
 *
 * Para : N/A
 *
 * Retn : is_QS_dp_enable()
 *-----------------------------------------------*/
int dprx_platform_get_quick_show_enable(void)
{
    return is_QS_dp_enable();
}


/*------------------------------------------------
 * Func : dprx_platform_get_quick_show_channel
 *
 * Desc : get quick show channel
 *
 * Para : N/A
 *
 * Retn : -1 : failed, 0 ~ N : quick show channel
 *-----------------------------------------------*/
int dprx_platform_get_quick_show_channel(void)
{
    if (is_QS_dp_enable())
        return get_QS_portnum();
    return -1;
}

#endif
