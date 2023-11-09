/*=============================================================
 * File:    dprx_adapter-rtk_dprx-platform-quickshow.c
 *
 * Desc:    driver wrapper for quick show
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
#include <dprx_adapter.h>
#include <dprx_adapter-rtk_dprx-priv.h>
#include <dprx_adapter-rtk_dprx-plat.h>
//#include <dprx_vfe_api.h>
//#include <dprx_vfe.h>
#include <rbus/sys_reg_reg.h>

extern UINT8 newbase_dprx_aux_get_irq_status_exint0(UINT8 port);
extern UINT8 lib_dprx_dpcd_get_link_config_wr_irq_exint0(UINT8 port);
extern UINT8 _check_link_status(UINT8 port);

#ifdef ENABLE_DPRX_LT_EVENT_MONITOR

/*------------------------------------------------------------------
 * Func : rtk_dprx_lt_event_message_enable
 *
 * Desc : link training debug message control
 *
 * Parm : p_dprx : handle of DPRX
 *
 * Retn : 0 : event message disable, 1 : event message enable
 *------------------------------------------------------------------*/
int rtk_dprx_lt_event_message_enable(RTK_DPRX_ADP* p_dprx)
{
    if (p_dprx==NULL)
        return 0;

    switch(p_dprx->link_state)
    {
    case _DP_FAKE_TRAINING_PATTERN_1_RUNNING:
    case _DP_NORMAL_TRAINING_PATTERN_1_RUNNING:
        return 0;  // do not print message at lt1 state

    default:
        return 1;
    }
}

#endif

/*------------------------------------------------------------------
 * Func : rtk_dprx_do_link_training
 *
 * Desc : link training handler for quick show
 *
 * Parm : N/A
 *
 * Retn : 0 : link train fail, 1 : link train pass
 *------------------------------------------------------------------*/
int rtk_dprx_do_link_training(void)
{
    int ret = 0;

    if (newbase_dprx_aux_get_irq_status_exint0(0) && lib_dprx_dpcd_get_link_config_wr_irq_exint0(0))
    {
        dprx_drv_interrupt_handler_exint0();
        dprx_drv_exit_interrupt_handler_exint0();
    }

    if (GET_DP_AUX_RX0_LINK_TRAINING_STATUS()==_DP_FAKE_LINK_TRAINING_PASS ||
        GET_DP_AUX_RX0_LINK_TRAINING_STATUS()==_DP_NORMAL_LINK_TRAINING_PASS ||
        GET_DP_AUX_RX0_LINK_TRAINING_STATUS()==_DP_FAKE_LINK_TRAINING_PASS_VBIOS)
    {
        // check link status
        if (_check_link_status(0)==0)
        {
            SET_DP_AUX_RX0_LINK_TRAINING_STATUS(_DP_LINK_STATUS_FAIL);
            newbase_dprx_aux_handle_link_status_irq(0);
            return 0;
        }

        return 1;
    }

    return 0;
}


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
    // quick show doesn't use HW interrupt, force disable HW interrupt
    rtd_outl(SYS_REG_INT_CTRL_SCPU_2_reg, SYS_REG_INT_CTRL_SCPU_2_dp_auxrx_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_2_write_data(0));
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
    rtd_outl(SYS_REG_INT_CTRL_SCPU_2_reg, SYS_REG_INT_CTRL_SCPU_2_dp_auxrx_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_2_write_data(0));
    return 0;
}

