/*=============================================================
 * File:    dprx_adapter-rtk_dprx-platform.h
 *
 * Desc:    DPRX Adapter
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
#ifndef __DPRX_ADAPTER_RTK_DPRX_PLATFORM_H__
#define __DPRX_ADAPTER_RTK_DPRX_PLATFORM_H__

/*------------------------------------------------------------------
 * Func : rtk_dprx_plat_init
 *
 * Desc : dprx platform init
 *
 * Parm : N/A
 *
 * Retn : 0
 *------------------------------------------------------------------*/
extern int rtk_dprx_plat_init(void);


/*------------------------------------------------------------------
 * Func : rtk_dprx_plat_interrupt_enable
 *
 * Desc : enable dprx interrupt
 *
 * Parm : N/A
 *
 * Retn : 0
 *------------------------------------------------------------------*/
extern int rtk_dprx_plat_interrupt_enable(void);


/*------------------------------------------------------------------
 * Func : rtk_dprx_plat_interrupt_disable
 *
 * Desc : disable dprx interrupt
 *
 * Parm : N/A
 *
 * Retn : 0
 *------------------------------------------------------------------*/
extern int rtk_dprx_plat_interrupt_disable(void);


/*------------------------------------------------------------------
 * Func : rtk_dprx_lt_event_message_enable
 *
 * Desc : link training debug message control
 *
 * Parm : p_dprx : handle of DPRX
 *
 * Retn : 0 : event message disable, 1 : event message enable
 *------------------------------------------------------------------*/
extern int rtk_dprx_lt_event_message_enable(RTK_DPRX_ADP* p_dprx);

#endif // __DPRX_ADAPTER_RTK_DPRX_PLATFORM_H__
