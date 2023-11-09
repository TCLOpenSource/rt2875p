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
#include <dprx_vfe.h>
#include <dprx_adapter-pseudo.h>
#include <dprx_adapter-rtk_dprx.h>

dprx_dev_t* g_connected_dprx = NULL;

/*------------------------------------------------
 * Func : dprx_vfe_create_dprx_adapter_by_config
 *
 * Desc : create specific Platform config setting
 *
 * Para : p_cfg : port config
 *
 * Retn : NULL : failed / others : dprx handle
 *-----------------------------------------------*/
dprx_dev_t* dprx_vfe_create_dprx_adapter_by_config(DP_PORT_CFG* p_cfg)
{
    dprx_dev_t* p_dev = NULL;

    if (p_cfg==NULL)
    {
        DP_VFE_ERR("create DPRX adapter failed - invalid config\n");
        return NULL;
    }

    switch(p_cfg->type)
    {
    case DP_TYPE_DP:
    case DP_TYPE_mDP:
    case DP_TYPE_USB_TYPE_C:
#ifdef CONFIG_DPRX_DRV_ENABLE_RTK_DPRX_ADAPTER
        p_dev = create_rtk_dprx_adapter(p_cfg);
        break;
#endif
    case DP_TYPE_PSEUDO_DP:
        p_dev = create_pseudo_dprx_adapter();
        break;

    case DP_TYPE_PSEUDO_HDMI_DP:
    case DP_TYPE_NONE:
    case DP_TYPE_MAX:
    default:
        // do nothing
        break;
    }

    if (p_dev)
    {
        if (dprx_adp_init(p_dev)<0)
        {
            DP_VFE_ERR("init DPRX adapter failed\n");
            destroy_dprx_device(p_dev);
            return NULL;
        }

        DP_VFE_INFO("init DPRX adapter successed\n");
    }
    else
    {
        DP_VFE_ERR("create DPRX adapter failed\n");
    }

    return p_dev;
}
