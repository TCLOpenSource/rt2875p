/*=============================================================
 * File:    dprx_vfe-build_in_test.c
 *
 * Desc:    DPRX build in test
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/kthread.h>
#include <dprx_adapter.h>
#include <dprx_vfe_api.h>
#include <dprx_vfe.h>
#if IS_ENABLED(CONFIG_RTK_MEASURE)
#include <measure/rtk_measure.h>
extern OFFMS_CTRL_FALGS _gen_offms_flag(DPRX_TIMING_INFO_T* p_timing);
#endif

static struct task_struct *p_adapter_timing_detect_tsk = NULL;
static unsigned char dprx_channel = 1;

static void dump_timing_info(DPRX_TIMING_INFO_T *p_timing_info)
{
    DPRX_ADP_WARN("=========== Timing ==========\n");
    DPRX_ADP_WARN("color_space : %d             \n", p_timing_info->color_space);
    DPRX_ADP_WARN("color_depth : %d             \n", p_timing_info->color_depth);
    DPRX_ADP_WARN("interlace : %d               \n", p_timing_info->is_interlace);
    DPRX_ADP_WARN("dsc : %d, allm=%d            \n", p_timing_info->is_dsc, p_timing_info->isALLM);
    DPRX_ADP_WARN("drr_mode : %d, curr_vfreq_hz_x100=%d \n", p_timing_info->drr_mode , p_timing_info->curr_vfreq_hz_x100);
    DPRX_ADP_WARN("-----------------------------\n");
    DPRX_ADP_WARN("        | H       | V        \n");
    DPRX_ADP_WARN("--------+---------+----------\n");
    DPRX_ADP_WARN(" total  | %5d     | %5d      \n", p_timing_info->htotal, p_timing_info->vtotal);
    DPRX_ADP_WARN(" active | %5d     | %5d      \n", p_timing_info->hact, p_timing_info->vact);
    DPRX_ADP_WARN(" porch  | %5d     | %5d      \n", p_timing_info->hporch, p_timing_info->vporch);
    DPRX_ADP_WARN(" start  | %5d     | %5d      \n", p_timing_info->hstart, p_timing_info->vstart);
    DPRX_ADP_WARN(" sync   | %5d     | %5d      \n", p_timing_info->hsync, p_timing_info->vsync);
    DPRX_ADP_WARN(" freq   | %5d     | %5d      \n", p_timing_info->hfreq_hz, p_timing_info->vfreq_hz_x100);
    DPRX_ADP_WARN("-----------------------------\n");
}

#if IS_ENABLED(CONFIG_RTK_MEASURE)

static void dump_measure_timing_info(MEASURE_TIMING_T *p_timing_info)
{
    DP_VFE_WARN("=========== OFFMS Timing ==========\n");
    DP_VFE_WARN("-----------------------------\n");
    DP_VFE_WARN("        | H       | V        \n");
    DP_VFE_WARN("--------+---------+----------\n");
    DP_VFE_WARN(" total  | %5d     | %5d      \n", p_timing_info->h_total, p_timing_info->v_total);
    DP_VFE_WARN(" active | %5d     | %5d      \n", p_timing_info->h_act_len, p_timing_info->v_act_len);
    DP_VFE_WARN(" start  | %5d     | %5d      \n", p_timing_info->h_act_sta, p_timing_info->v_act_sta);
    DP_VFE_WARN(" freq   | %5d     | %5d      \n", p_timing_info->h_freq, p_timing_info->v_freq);
    DP_VFE_WARN("-----------------------------\n");
}

#endif

static int dprx_timing_detect_tsk(void *p)//This task run HDMI source timing detecting after source connect
{
    unsigned char ch = dprx_channel;
    unsigned char pre_connect_state = 0;
    unsigned char connect_state = 0;
    DPRX_TIMING_INFO_T timing;
    unsigned char timing_ready = 0;

    current->flags &= ~PF_NOFREEZE;

    vfe_dprx_drv_init();

    vfe_dprx_drv_open();

    DP_VFE_INFO("dprx test thread start(ch=%d)\n", ch);

    vfe_dprx_drv_connect(ch);  // connect DPRX 2

    vfe_dprx_drv_enable_hpd(ch);  // enable hpd
    DP_VFE_INFO("enable hpd\n");

    DP_VFE_INFO("start connect status check\n");

    while (1)
    {
        usleep_range(300*1000, 300*1000);

        if (ch != dprx_channel)
        {
            DP_VFE_INFO("current channel changed, change connect channel %d -> %d\n", ch, dprx_channel);
            ch = dprx_channel;
            vfe_dprx_drv_connect(ch);
        }

        if (vfe_dprx_drv_get_connection_state(ch, &connect_state)<0)
        {
            DP_VFE_WARN("get_dprx_connnect_state(ch=%d) failed\n", ch);
        }
        else
        {
            if (pre_connect_state != connect_state)
            {
                DP_VFE_WARN("get_dprx_connnect_state(ch=%d) successed, state = %d\n", ch, connect_state);
                pre_connect_state = connect_state;
            }

            if (connect_state)
            {
                int rr = vfe_dprx_drv_source_check(ch);

                if (rr!=DPRX_DRV_NO_ERR)
                {
                    if (timing_ready)
                        timing_ready = 0;

                    continue;
                }

                memset(&timing, 0, sizeof(timing));
                vfe_dprx_drv_get_port_timing_info(ch, &timing);

                if (timing_ready==0) {
                    dump_timing_info(&timing);
                }

#if IS_ENABLED(CONFIG_RTK_MEASURE)
                if (timing_ready==0 && timing.hact && timing.vact)
                {
                    MEASURE_TIMING_T measre_timing;
                    memset(&measre_timing, 0, sizeof(measre_timing));

                    if (rtk_offms_measure_video(OFFMS_SOURCE_DPRX, &measre_timing, _gen_offms_flag(&timing))==0)
                    {
                        DP_VFE_WARN("DPRX measure failed\n");
                    }
                    else
                    {
                        dump_measure_timing_info(&measre_timing);
                        timing_ready=1;
                    }
                }
#endif
            }
            else
            {
                timing_ready = 0;
            }
        }

        if (freezing(current))
        {
            try_to_freeze();
        }
        if (kthread_should_stop())
        {
            break;
        }
    }

    vfe_dprx_drv_close();
    vfe_dprx_drv_uninit();
    do_exit(0);
    return 0;
}


int dprx_vfe_set_build_in_test_enable(unsigned char enable)
{
    DP_VFE_INFO("dprx_vfe_set_build_in_test_enable(%d)\n", enable);
    if (enable)
    {
        if (p_adapter_timing_detect_tsk==NULL)
        {
            p_adapter_timing_detect_tsk = kthread_create(dprx_timing_detect_tsk, NULL, "dprx_timing_detect_tsk");

            if (p_adapter_timing_detect_tsk)
            {
                DP_VFE_INFO("create dprx_timing_detect_tsk thread success\n");
                wake_up_process(p_adapter_timing_detect_tsk);
            }
            else
            {
                DP_VFE_ERR("create dprx_timing_detect_tsk thread fail\n");
                return -1;
            }
        }
    }
    else
    {
        if (p_adapter_timing_detect_tsk)
        {
            kthread_stop(p_adapter_timing_detect_tsk);
            p_adapter_timing_detect_tsk = NULL;
            DP_VFE_ERR("create dprx_timing_detect_tsk thread stop\n");
        }
    }
    return 0;
}


int dprx_vfe_get_build_in_test_enable(void)
{
    return (p_adapter_timing_detect_tsk) ? 1 : 0;
}


int dprx_vfe_set_build_in_test_channel(unsigned char ch)
{
    if (ch > MAX_DPRX_PORT)
        ch = 0;

    dprx_channel = ch;
    return 0;
}

int dprx_vfe_get_build_in_test_channel(void)
{
    return dprx_channel;
}
