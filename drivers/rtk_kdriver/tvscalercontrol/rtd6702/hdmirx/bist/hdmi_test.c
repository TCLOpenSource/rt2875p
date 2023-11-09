/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2019
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ============================
 *
 * file:    hdmi_debug.c
 *
 * author:    Taiwan, Eric Liu
 * date:
 * version:    1.0
 *

======================= How to use ============================

In bring up status, there are no user space program to init hdmi driver,
that will make verify difficult.

 How to use:

 cd /sys/class/misc/rtk_hdmi

 x =0,1,2,3 (channel number)
 echo hdmi_driver_init=x > debug_ctrl

*============================================================*/


#include <linux/kthread.h> //kthread_create()?Bkthread_run()
#include <linux/err.h> //IS_ERR()?BPTR_ERR()
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>

#include <linux/slab.h>
#include "../hdmi_common.h"
#include "../hdmi_vfe_config.h"
#include <tvscalercontrol/hdmirx/hdmi_vfe.h>
#include <hdmirx/bist/hdmi_test.h>

static struct task_struct *hdmi_task;
static bool enable_hdmi_init =false;
static int previous_channel_port = -1;

static int hdmirx_detect_task(void *arg)
{
    while (!kthread_should_stop())
    {
        vfe_hdmi_drv_detect_mode();
        usleep_range(10*1000, 10*1000);
    }

    return 0;
}


bool hdmi_detect_thread_enable(unsigned char on)
{
    int err = 0;

    if (on)
    {
        hdmi_task = kthread_create(hdmirx_detect_task, NULL, "hdmirx_detect_task");

        if (IS_ERR(hdmi_task))
        {
            err = PTR_ERR(hdmi_task);
            hdmi_task = NULL;

            HDMI_EMG("%s , Unable to start kernel hdmi_detect_thread_enable thread (err_id = %d) .\n", __func__,err);
            return false;
            }

            wake_up_process(hdmi_task);
            HDMI_PRINTF("%s hdmi infopack thread started\n" , __func__ );
    }
    else
    {
        HDMI_PRINTF("=====hdmi_detect_thread_enable kthread_exit=====\n");

        if (!IS_ERR(hdmi_task)){
            int ret = kthread_stop(hdmi_task);
            HDMI_PRINTF("=====hdmi_detect_thread_enable thread function has run %ds=====\n", ret);
            hdmi_task = NULL;
        }
    }

    return true;
}


void hdmi_rx_init(int port)
{
    unsigned char hdmi_port;

    HDMI_PRINTF("=====hdmi_rx_init=====\n");

    if (!enable_hdmi_init) {

        vfe_hdmi_drv_init();
        vfe_hdmi_drv_open();

        hdmi_detect_thread_enable(1);

        enable_hdmi_init = true;
    }

    if (hdmi_vfe_get_hdmi_port(port, &hdmi_port)<0) {
        HDMI_EMG("[vfe_hdmi_drv_connect][- ARG ERR] get hdmi port for ch = %d failed\n", port);
        return ;
    }

    if (previous_channel_port != -1)
    {
        vfe_hdmi_drv_disconnect(previous_channel_port);
    }

    previous_channel_port = port;

    newbase_hdmi_enable_hpd(hdmi_port);

    vfe_hdmi_drv_connect(port);    // connect HDMI Port

}

