/*=============================================================
 * File:    dprx_hpd.h
 *
 * Desc:    DPRX HPD detect
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
#ifndef __DPRX_HPD_H__
#define __DPRX_HPD_H__

#include <dprx_osal.h>
#include <dprx_types.h>
#include <dprx_dev.h>  // dev_ctx_t

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DPRX_HPD_EVENT_NONE = 0,
    DPRX_HPD_EVENT_CONNECT_STATUS_UPDATE,
}DPRX_HPD_EVENT;

typedef struct
{
    void* p_ctx;
    void (*event_handler)(void* p_ctx, DPRX_HPD_EVENT event);
}DPRX_HPD_EVENT_HANDLER;

typedef struct
{
    int (*init)   (dev_ctx_t* p_ctx);
    int (*uninit) (dev_ctx_t* p_ctx);
    int (*get_connect_status)(dev_ctx_t* p_adp);           // get connect status (0: cable disconnected, 1 : cable connected)
    int (*get_aux_status)(dev_ctx_t* p_adp, unsigned char* p_aux_status); // get aux status
    int (*set_hpd)(dev_ctx_t* p_adp, unsigned char on);    // set HPD (0: HPD low, 1 : HPD High)
    int (*set_hpd_toggle)(dev_ctx_t* p_adp, unsigned short ms_delay);    // ms_delay
    int (*set_irq_hpd)(dev_ctx_t* p_adp);                  // set IRQ HPD
    int (*get_hpd)(dev_ctx_t* p_adp);                      // get HPD (0: HPD low, 1 : HPD High)
    int (*get_cable_config)(dev_ctx_t* p_adp, DPRX_CABLE_CFG* p_cable_cfg); // get cable config
    int (*set_typec_re_alt_mode)(dev_ctx_t* p_adp);        // set realt mode

    //------------------------------
    int (*register_event_handler)(dev_ctx_t* p_adp, DPRX_HPD_EVENT_HANDLER* p_handler);
    int (*enable_event_handler)(dev_ctx_t* p_adp, unsigned char on);  // enable event handler

}hpd_dev_ops_t;

typedef struct
{
    dev_ctx_t      ctx;
    hpd_dev_ops_t  ops;
}hpd_dev_t;

//================= API of DPRX HPD ==============

extern hpd_dev_t* alloc_dprx_hpd_device(void);
extern void destroy_dprx_hpd_device(hpd_dev_t* p_dev);

extern int dprx_hpd_init(hpd_dev_t* p_dev);
extern int dprx_hpd_get_connect_status(hpd_dev_t* p_dev);
extern int dprx_hpd_get_aux_status(hpd_dev_t* p_dev, unsigned char* p_aux_status);
extern int dprx_hpd_set_hpd(hpd_dev_t* p_dev, unsigned char on);
extern int dprx_hpd_set_hpd_toggle(hpd_dev_t* p_dev, unsigned short ms_delay);
extern int dprx_hpd_set_irq_hpd(hpd_dev_t* p_dev);
extern int dprx_hpd_get_hpd(hpd_dev_t* p_dev);
extern int dprx_hpd_get_cable_config(hpd_dev_t* p_dev, DPRX_CABLE_CFG* p_cable_cfg);
extern int dprx_hpd_set_typec_re_alt_mode(hpd_dev_t* p_dev);

extern int dprx_hpd_register_event_handler(hpd_dev_t* p_dev, DPRX_HPD_EVENT_HANDLER* p_handler);
extern int dprx_hpd_set_event_handler_enable(hpd_dev_t* p_dev, unsigned char enable);

//================= API of DPRX HPD Low Guard Interval ==============
#ifdef CONFIG_DPRX_VFE_ENABLE_HPD_LOW_GUARD_INTERVAL
#define  DEFAULT_DP_HPD_LOW_PERIOD_MS           700   // Mircsoft Surface Pro takes more than 500ms to detect long hpd toggle
#define  DEFAULT_TYPEC_HPD_LOW_PERIOD_MS       1300   // Lindy HDMI to typeC converter takes more than 1200 mse to detect hpd
extern   unsigned int _get_hpd_low_guard_time(unsigned long curr_time, unsigned long pre_hpd_low_time_stamp, unsigned int  guard_time);
#else
#define  DEFAULT_DP_HPD_LOW_PERIOD_MS             0   // Don't use HPD Guard Interval (controlled by FW)
#define  DEFAULT_TYPEC_HPD_LOW_PERIOD_MS          0   // Don't use HPD Guard Interval (controlled by FW)
#define  _get_hpd_low_guard_time(args...)        (0)
#endif

//======== DPRX HPD debug message control ========
#define DPRX_HPD_ERR(fmt, args...)         dprx_err("[DP][HPD][ERR] " fmt, ## args)
#define DPRX_HPD_WARN(fmt, args...)        dprx_warn("[DP][HPD][WARN] " fmt, ## args)
#define DPRX_HPD_INFO(fmt, args...)        dprx_info("[DP][HPD][INFO] " fmt, ## args)
#define DPRX_HPD_DBG(fmt, args...)         dprx_dbg("[DP][HPD][DEBUG] " fmt, ## args)

#ifdef __cplusplus
}
#endif

#endif // __DPRX_HPD_H__
