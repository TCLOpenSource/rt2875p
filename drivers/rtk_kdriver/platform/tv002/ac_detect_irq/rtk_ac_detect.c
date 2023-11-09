/*
 *  Copyright (C) 2017 Realtek
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/mbus.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>

#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <rbus/misc_reg.h>
#include <rtk_gpio.h>
#include <mach/platform.h>
#include <mach/pcbMgr.h>
#include <rbus/sb2_reg.h>

#include "rtk_ac_detect.h"

#define AC_DET_DRV_NAME  "ac_det_drv"

//#define FAKE_AC_DET_PIN
#ifdef FAKE_AC_DET_PIN
#define DET_PIN_NAME    "PIN_AC_DETECT"
#else
#define DET_PIN_NAME    "PIN_DC_MON_SOC"
#endif

static struct platform_device rtk_ac_det_device = {
    .name   = AC_DET_DRV_NAME,
    .id     = -1,
};

static struct platform_driver rtk_ac_det_driver = {
    .driver    = {
        .name  = AC_DET_DRV_NAME,
        .owner = THIS_MODULE,
    },
    .suspend   = NULL,
    .resume    = NULL,
};

static struct ac_det_info ac_detection;

/************************************************************************
 *
 * Please design struct or func[prepare() and action()] depend on project.
 *
 ************************************************************************/
/************************************************************************
 * gpio info for AC dection hit
 ************************************************************************/
struct tv002_customer_info{
    u32 gpio_group;
    u32 gpio_num;
    RTK_GPIO_ID gid;
};
struct tv002_customer_info tv002_customer_pin;

/************************************************************************
 * require of TV002 is ¡§POWER1 signal line pull High during AC OFF sequence.¡¨
 * #define PIN_POWER1              RTD287O_BGA_22x22_PIN_AB21_ISO_GPO_41(0, 0)
 ************************************************************************/
int tv002_ac_det_action(struct ac_det_info* ac_det)

{
    int ret = 0;
    struct tv002_customer_info* control_pin;
    u32 gpio_group;
    u32 rtk_gpio_num;
    RTK_GPIO_ID gid;

    AD_INFO("%s(%d) AC detection action.gid(0x%x)\n",
            __func__,__LINE__,ac_det->gid);

    control_pin = (struct tv002_customer_info*)ac_det->customer_info;

    gpio_group = control_pin->gpio_group;
    rtk_gpio_num= control_pin->gpio_num;
    gid = control_pin->gid;

    AD_INFO("%s: group=0x%x num=0x%x gid=0x%d\n",
                __func__, gpio_group,rtk_gpio_num,gid);

    rtk_gpio_output(rtk_gpio_id(MIS_GPIO, 33), 0);  //mute1
    rtk_gpio_output(rtk_gpio_id(MIS_GPIO, 57), 0);  //mute2

    ret = rtk_gpio_output(gid, 1);

    return ret;
}

/************************************************************************
 * require is ¡§POWER1 signal line pull High during AC OFF sequence.¡¨
 * #define PIN_POWER1              RTD287O_BGA_22x22_PIN_AB21_ISO_GPO_41(0, 0)
 ************************************************************************/
#define TV002_AC_DET_TARGET_PIN "PIN_POWER_EN"
int tv002_ac_det_prepare(struct ac_det_info* ac_det)
{
    int ret = -1;
    u64 rtk_tmp_gpio;

    /* check pinmux, direction of gpio  */
    AD_INFO("%s(%d) AC detection action.gid(0x%x)\n",
            __func__,__LINE__,ac_det->gid);

    /* check gpio info which want to control */
    if(pcb_mgr_get_enum_info_byname(TV002_AC_DET_TARGET_PIN, &rtk_tmp_gpio)==0)
    {
        u32 gpio_group;
        u32 rtk_gpio_num;
        RTK_GPIO_ID gid;

        AD_INFO("%s got.(0x%llx)\n",TV002_AC_DET_TARGET_PIN, (u64)rtk_tmp_gpio);

        gpio_group      = GET_PIN_TYPE(rtk_tmp_gpio);
        rtk_gpio_num    = GET_PIN_INDEX(rtk_tmp_gpio);

        switch(gpio_group){
        case PCB_PIN_TYPE_GPIO:
            AD_INFO("%s: PIN type is PCB_PIN_TYPE_GPIO\n",__func__);
            if (rtk_gpio_num > MIS_GPIO_CNT) {
                AD_NOTICE
                    ("Error Pin define,Skip AC_DET function\n");
                goto AC_DET_OUT;
            }
            gpio_group = MIS_GPIO;
            break;
        case PCB_PIN_TYPE_ISO_GPIO:
            AD_INFO("%s: PIN type is PCB_PIN_TYPE_ISO_GPIO\n",__func__);
            if(rtk_gpio_num > ISO_GPIO_CNT) {
                AD_NOTICE
                    ("Error Pin define,Skip AC_DET function\n");
                goto AC_DET_OUT;
            }
            gpio_group = ISO_GPIO;
            break;
        default:
            AD_INFO("%s: PIN group not match\n",__func__);
            goto AC_DET_OUT;

        }

        gid = rtk_gpio_id(gpio_group, rtk_gpio_num);
#if 0
        AD_ALERT("chcek %s pcb num\n",TV002_AC_DET_TARGET_PIN);
        AD_ALERT("0x%08x(GPDIR_1)=0x%08x 0x%08x(GPDATO_1)=0x%08x 0x%08x(PIN MUX)=0x%08x\n",
            0xb8061104,rtd_inl(0xb8061104),
            0xb806110c,rtd_inl(0xb806110c),
            0xb8060220,rtd_inl(0xb8060220));

        AD_ALERT("%s: group=0x%x num=0x%x gid=0x%d\n",
            __func__, gpio_group,rtk_gpio_num,gid);
#endif
        tv002_customer_pin.gpio_group = gpio_group;
        tv002_customer_pin.gpio_num = rtk_gpio_num;
        tv002_customer_pin.gid = gid;

        ac_det->customer_info = &tv002_customer_pin;
        ret = 0;
    }

AC_DET_OUT:
    return ret;
}

/* Add func of action depend on customer here */
struct ac_det_ops tv002_ac_det_ops = {
    .ac_det_action = tv002_ac_det_action,
    .ac_det_prepare = tv002_ac_det_prepare,
};

static __attribute__((unused))
void rtk_ac_det_isr(unsigned int gid, unsigned char assert, void *dev)
{
    struct ac_det_info *ac_det_pin = dev;
    u32 pin_inv;
    int gid_val;

    u32 pin_idx;
    u32 pin_type;

    pin_type = (u32)GET_PIN_TYPE(ac_det_pin->pcb_enum);
    pin_idx  = (u32)GET_PIN_INDEX(ac_det_pin->pcb_enum);
    pin_inv  = (u32)GET_GPIO_INVERT(ac_det_pin->pcb_enum);

#if 0
    AD_ALERT("%s int got.(0x%llx)\n",DET_PIN_NAME, (u64) ac_det_pin->pcb_enum);
    AD_ALERT( "Type=0x%x INDEX=0x%x INVERT=0x%x\n",
        pin_type,pin_idx,pin_inv);
    AD_ALERT( "group=0x%x num=0x%x gid=0x%x\n",
        ac_det_pin->gpio_group,ac_det_pin->gpio_num,ac_det_pin->gid);
#endif
    gid_val = rtk_gpio_input(ac_det_pin->gid);

    if(gid_val < 0){
        AD_NOTICE( "get gid[0x%x] fail\n",ac_det_pin->gid);
        return;
    }else{
        if(pin_inv == gid_val ){
            /* isr request status hit */
            local_irq_disable();
            preempt_disable();
            ac_det_pin->ops->ac_det_action(ac_det_pin);
            while(1);
        }
    }

}

static
int __init rtk_ac_det_probe(struct platform_device *pdev)
{
    int ret = -1;
    u64 rtk_tmp_gpio;
    struct ac_det_info *ac_det_pin = &ac_detection;

    if(pcb_mgr_get_enum_info_byname(DET_PIN_NAME, &rtk_tmp_gpio)==0)
    {
        u32 gpio_group;
        u32 rtk_gpio_num;
        RTK_GPIO_ID gid;

        ac_det_pin->pcb_enum = rtk_tmp_gpio;
        AD_NOTICE("%s got.(0x%llx)\n",DET_PIN_NAME, (u64) ac_det_pin->pcb_enum);

        gpio_group      = GET_PIN_TYPE(rtk_tmp_gpio);
        rtk_gpio_num    = GET_PIN_INDEX(rtk_tmp_gpio);

#ifdef FAKE_AC_DET_PIN
        AD_ALERT("gpio_group = %x\n", gpio_group);
        AD_ALERT("rtk_gpio_num = %x\n", rtk_gpio_num);

        /* Has 296x board AC detect pin? */
        rtk_tmp_gpio    = 0x0015303;
        gpio_group      = GET_PIN_TYPE(rtk_tmp_gpio);
        rtk_gpio_num    = GET_PIN_INDEX(rtk_tmp_gpio);
        AD_ALERT("\nfor test change PIN_AC_DETECT to 0x%08llx.\n",
               rtk_tmp_gpio);
        AD_ALERT("gpio_group = %x\n", gpio_group);
        AD_ALERT("rtk_gpio_num = %x\n", rtk_gpio_num);
#endif
        switch(gpio_group){
        case PCB_PIN_TYPE_GPIO:
            AD_NOTICE("PIN type is PCB_PIN_TYPE_GPIO\n");
            if (rtk_gpio_num > MIS_GPIO_CNT) {
                AD_NOTICE
                    ("Error Pin define,Skip AC_DET function\n");
                goto AC_DET_OUT;
            }
            gpio_group = MIS_GPIO;
            break;
        case PCB_PIN_TYPE_ISO_GPIO:
            AD_NOTICE("PIN type is PCB_PIN_TYPE_ISO_GPIO\n");
            if(rtk_gpio_num > ISO_GPIO_CNT) {
                AD_NOTICE
                    ("Error Pin define,Skip AC_DET function\n");
                goto AC_DET_OUT;
            }
            gpio_group = ISO_GPIO;
            break;
        default:
            AD_NOTICE("PIN group not match\n");
            goto AC_DET_OUT;

        }

        ac_det_pin->gid = rtk_gpio_id(gpio_group, rtk_gpio_num);
        ac_det_pin->gpio_group = gpio_group;
        ac_det_pin->ops = &tv002_ac_det_ops;

        gid = ac_det_pin->gid;

        if(ac_det_pin->ops->ac_det_prepare(ac_det_pin)){
            goto AC_DET_OUT;
        }

        /* RTK_GPIO_DEBOUNCE_100us;RTK_GPIO_DEBOUNCE_1ms */
        if(rtk_gpio_set_debounce(gid,RTK_GPIO_DEBOUNCE_100us)){
            goto AC_DET_OUT;
        }
        if(rtk_gpio_set_dir(gid, 0)){
            goto AC_DET_OUT;
        }
        if(rtk_gpio_clear_isr(gid)){
            goto AC_DET_OUT;
        }
        if (rtk_gpio_request_irq(gid,rtk_ac_det_isr,AC_DET_DRV_NAME, ac_det_pin)){
            goto AC_DET_OUT;
        }
        if(rtk_gpio_set_irq_enable(gid, 1)){
            goto AC_DET_OUT;
        }

        ret = 0;
    }

AC_DET_OUT:
    if(ret){
        AD_INFO("%s: request gpio isr fail\n", AC_DET_DRV_NAME);
    }

    return ret;

}


static
int __init rtk_ac_det_module_init(void)
{
    int retval = -1;


    if(platform_device_register(&rtk_ac_det_device))
        goto FINISH_OUT;

    if(platform_driver_probe(&rtk_ac_det_driver, rtk_ac_det_probe))
        goto FINISH_OUT;

    retval = 0;

FINISH_OUT:
    AD_INFO("RTK AC detection driver install %s\n",(retval<0)?"fail":"success");

    return retval;
}

/************************************************************************
 *
 ************************************************************************/
static
void __exit rtk_ac_det_module_exit(void)
{
    platform_driver_unregister(&rtk_ac_det_driver);

    AD_INFO("RTK AC detection driver uninstall success\n");
}


module_init(rtk_ac_det_module_init);
module_exit(rtk_ac_det_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexkh <alexkh@realtek.com>");

