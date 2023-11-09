/*=============================================================
 * File:    dprx_adapter-rtk_dprx-wrapper-uboot.c
 *
 * Desc:    driver wrapper for uboot
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


static int virt_platfrom_init_ready = 0;
static int virt_platfrom_irq_ready = 0;


/*------------------------------------------------------------------
 * Func : rtk_dprx_plat_init
 *
 * Desc : dprx platform init
 *
 * Parm : N/A
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_dprx_plat_init(void)
{
    if (virt_platfrom_init_ready==0)
    {
        virt_platform_init();
        virt_platfrom_init_ready++;
    }

    return 0; // do nothing
}


/*------------------------------------------------------------------
 * Func : rtk_dprx_plat_interrupt_enable
 *
 * Desc : enable dprx interrupt
 *
 * Parm : N/A
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_dprx_plat_interrupt_enable(void)
{
    if (virt_platfrom_irq_ready==0)
        virt_platform_request_irq(DPRX_AUX_IRQ, newbase_dprx_drv_interrupt_handler_exint0);   // register irq
    return 0;
}


/*------------------------------------------------------------------
 * Func : rtk_dprx_plat_interrupt_disable
 *
 * Desc : disable dprx interrupt
 *
 * Parm : N/A
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_dprx_plat_interrupt_disable(void)
{
    if (virt_platfrom_irq_ready==0)
        virt_platform_request_irq(DPRX_AUX_IRQ, NULL);   // register irq
    return 0;
}
