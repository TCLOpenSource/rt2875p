/*=============================================================
 * File:    dprx_platform.h
 *
 * Desc:    DPRX Platform Dependet API
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

#ifndef __DPRX_PLATFORM_H__
#define __DPRX_PLATFORM_H__

#include <dprx_osal.h>
#include <dprx_types.h>

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------
 * DP_PORT_CFG
 *-----------------------------------------------------------------------*/

typedef struct
{
    unsigned char   external_edid:1;  // what type of EDID to be used
    unsigned char   ddc_i2c_idx:7 ;   // which ddc/i2c channel to be used for EDID
}DP_EDID_CFG;

typedef struct
{
    unsigned char   idx;          // which aux channel to be used
    unsigned char   pn_swap;      // aux channel to be used PN swapped?
}DP_AUX_CH_CFG;

typedef struct
{
    unsigned int  idx:2;              // which phy to be used
    unsigned int  max_lane_count:3;   // number of lane : 1/2/4, others: invalid
    unsigned int  lane0_phy_idx :2;   // lane0 phy idx (for lane swap)
    unsigned int  lane1_phy_idx :2;   // lane1 phy idx (for lane swap)
    unsigned int  lane2_phy_idx :2;   // lane2 phy idx (for lane swap)
    unsigned int  lane3_phy_idx :2;   // lane3 phy idx (for lane swap)
    unsigned int  lane0_pn_swap :1;   // lane0 PN SWAP
    unsigned int  lane1_pn_swap :1;   // lane1 PN SWAP
    unsigned int  lane2_pn_swap :1;   // lane2 PN SWAP
    unsigned int  lane3_pn_swap :1;   // lane3 PN SWAP
    unsigned int  reserved :15;       // reserved for future
}DP_PHY_CFG;

typedef struct
{
    unsigned char  hpd_gpio_valid   : 1;
    unsigned char  hpd_gpio_type    : 1;   // 0 : misc / 1 : iso
    unsigned char  det_gpio_valid   : 1;
    unsigned char  det_gpio_type    : 1;   // 0 : misc / 1 : iso
    unsigned char  aux_n_gpio_valid : 1;
    unsigned char  aux_n_gpio_type  : 1;   // 0 : misc / 1 : iso
    unsigned char  aux_p_gpio_valid : 1;
    unsigned char  aux_p_gpio_type  : 1;   // 0 : misc / 1 : iso
    unsigned char  hpd_gpio;
    unsigned char  det_gpio;
    unsigned char  aux_p_gpio;
    unsigned char  aux_n_gpio;

    // extra control flag
    unsigned char  flags;
    #define DP_HPD_QS_INIT       0x1
}DP_HPD_DET_CFG;

typedef struct
{
    DP_TYPE         type;
    unsigned char   qs_ctrl;               // quick show control flag
    #define DP_QS_INIT_READY      0x1      // indicate the port has been initialized by QS
    #define DP_QS_CONNECT_READY   0x2      // indicate the port has been connected by QS

    union
    {
        struct
        {
            DP_EDID_CFG     edid;
            DP_AUX_CH_CFG   aux;
            DP_PHY_CFG      phy;
            DP_HPD_DET_CFG  hpd_det;
        }dp_mdp;

        struct
        {
            DP_EDID_CFG     edid;
            DP_AUX_CH_CFG   aux;
            DP_PHY_CFG      phy;
            unsigned char   pd_port_idx;    // PD port to be used
        }typec_dp;

        struct
        {
            unsigned char   hdmi_port_idx;  // which HDMI port to use
        }pseudo_hdmi_dp;

        struct
        {
            unsigned char   flags;          // reserved for future
        }pseudo_dp;
    }config;
}DP_PORT_CFG;


/*-----------------------------------------------------------------------
 * DP Platform API
 *-----------------------------------------------------------------------*/


/*------------------------------------------------
 * Func : dprx_platform_init
 *
 * Desc : init Platform config setting
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR
 *-----------------------------------------------*/
extern int dprx_platform_init(void);


/*------------------------------------------------
 * Func : dprx_platform_uninit
 *
 * Desc : uninit DPRX platform. all resources
 *        will be released.
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR
 *-----------------------------------------------*/
extern int dprx_platform_uninit(void);


/*------------------------------------------------
 * Func : dprx_platform_get_port_config
 *
 * Desc : get config of a given DP port
 *
 * Para : idx :
 *
 * Retn : 0 : success, -1 failed
 *-----------------------------------------------*/
extern int dprx_platform_get_port_config(unsigned char idx, DP_PORT_CFG* p_cfg);


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
extern int dprx_platform_get_dprx_port(unsigned char channel, unsigned char* p_port);


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
extern int dprx_platform_get_dprx_channel(unsigned char port, unsigned char* p_channel);

#ifdef CONFIG_DPRX_VFE_ENABLE_QUICK_SHOW

/*------------------------------------------------
 * Func : dprx_platform_get_quick_show_enable
 *
 * Desc : check if quick show enabled
 *
 * Para : N/A
 *
 * Retn : 0 : disabled, others : enabled
 *-----------------------------------------------*/
extern int dprx_platform_get_quick_show_enable(void);

/*------------------------------------------------
 * Func : dprx_platform_get_quick_show_channel
 *
 * Desc : get quick show channel
 *
 * Para : N/A
 *
 * Retn : -1 : failed, 0 ~ N : quick show channel
 *-----------------------------------------------*/
extern int dprx_platform_get_quick_show_channel(void);

#else

#define dprx_platform_get_quick_show_enable()       0
#define dprx_platform_get_quick_show_channel()      -1

#endif

//======== Platform debug message control ========
#define DP_PLAT_ERR(fmt, args...)         dprx_err("[DP][PLAT][ERR] " fmt, ## args)
#define DP_PLAT_WARN(fmt, args...)        dprx_warn("[DP][PLAT][WARN] " fmt, ## args)
#define DP_PLAT_INFO(fmt, args...)        dprx_info("[DP][PLAT][INFO] " fmt, ## args)
#define DP_PLAT_DBG(fmt, args...)         dprx_dbg("[DP][PLAT][DEBUG] " fmt, ## args)

/////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif


#endif // __DPRX_PLATFORM_H__
