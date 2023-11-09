/*
 * LEDs driver for GPIOs
 *
 * Copyright (C) 2007 8D Technologies inc.
 * Raphael Assenat <raph@8d.com>
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include "rtk_leds.h"
#include <rtk_kdriver/rtk_gpio.h>
#include <rtk_kdriver/rtk_gpio-chipset.h>
#include <rtk_kdriver/pcbMgr.h>
#include <rtk_kdriver/rtk-kdrv-common.h>

static int androidboot_quiescent = 0;
static char *leds_node_name[]={"led0","led1","led2","led3","led4","led5","led6","led7","led8","led9"};

static int leds_count = 0;
RTK_PCB_LEDS_ENUM leds_name_list[MAX_LEDS_COUNT];
static struct platform_device *g_led_platform_dev;
struct gpio_leds_priv *priv;

struct gpio_led_data {
    struct led_classdev cdev;
    struct gpio_desc *gpiod;
    u8 can_sleep;
    u8 blinking;
    gpio_blink_set_t platform_gpio_blink_set;
};

static inline struct gpio_led_data *
            cdev_to_gpio_led_data(struct led_classdev *led_cdev)
{
    return container_of(led_cdev, struct gpio_led_data, cdev);
}

static void gpio_led_set(struct led_classdev *led_cdev,
    enum led_brightness value)
{
    struct gpio_led_data *led_dat = cdev_to_gpio_led_data(led_cdev);
    int level;

    if (value == LED_OFF)
        level = 0;
    else
        level = 1;

    if (led_dat->blinking) {
        led_dat->platform_gpio_blink_set(led_dat->gpiod, level,
                         NULL, NULL);
        led_dat->blinking = 0;
    } else {
        if (led_dat->can_sleep)
            gpiod_set_value_cansleep(led_dat->gpiod, level);
        else
            gpiod_set_value(led_dat->gpiod, level);
    }
}

static int gpio_led_set_blocking(struct led_classdev *led_cdev,
    enum led_brightness value)
{
    gpio_led_set(led_cdev, value);
    return 0;
}

static int gpio_blink_set(struct led_classdev *led_cdev,
    unsigned long *delay_on, unsigned long *delay_off)
{
    struct gpio_led_data *led_dat = cdev_to_gpio_led_data(led_cdev);

    led_dat->blinking = 1;
    return led_dat->platform_gpio_blink_set(led_dat->gpiod, GPIO_LED_BLINK,
                        delay_on, delay_off);
}

static int create_gpio_led(const struct gpio_led *template,
    struct gpio_led_data *led_dat, struct device *parent,
    struct device_node *np, gpio_blink_set_t blink_set)
{
    int ret, state;

    led_dat->gpiod = template->gpiod;
    if (!led_dat->gpiod) {
        /*
         * This is the legacy code path for platform code that
         * still uses GPIO numbers. Ultimately we would like to get
         * rid of this block completely.
         */
        unsigned long flags = GPIOF_OUT_INIT_LOW;

        /* skip leds that aren't available */
        if (!gpio_is_valid(template->gpio)) {
            dev_info(parent, "Skipping unavailable LED gpio %d (%s)\n",
                    template->gpio, template->name);
            return 0;
        }

        if (template->active_low)
            flags |= GPIOF_ACTIVE_LOW;

        ret = devm_gpio_request_one(parent, template->gpio, flags,
                        template->name);
        if (ret < 0)
            return ret;

        led_dat->gpiod = gpio_to_desc(template->gpio);

        if (!led_dat->gpiod)
            return -EINVAL;
    }

    led_dat->cdev.name = template->name;
    led_dat->cdev.default_trigger = template->default_trigger;
    led_dat->can_sleep = gpiod_cansleep(led_dat->gpiod);
    if (!led_dat->can_sleep)
        led_dat->cdev.brightness_set = gpio_led_set;
    else
        led_dat->cdev.brightness_set_blocking = gpio_led_set_blocking;
    led_dat->blinking = 0;
    if (blink_set) {
        led_dat->platform_gpio_blink_set = blink_set;
        led_dat->cdev.blink_set = gpio_blink_set;
    }
    if (template->default_state == LEDS_GPIO_DEFSTATE_KEEP) {
        state = gpiod_get_value_cansleep(led_dat->gpiod);
        if (state < 0)
            return state;
    } else {
        state = (template->default_state == LEDS_GPIO_DEFSTATE_ON);
    }

    led_dat->cdev.brightness = state ? LED_FULL : LED_OFF;
    if (!template->retain_state_suspended)
        led_dat->cdev.flags |= LED_CORE_SUSPENDRESUME;
    if (template->panic_indicator)
        led_dat->cdev.flags |= LED_PANIC_INDICATOR;
    if (template->retain_state_shutdown)
        led_dat->cdev.flags |= LED_RETAIN_AT_SHUTDOWN;

    ret = gpiod_direction_output(led_dat->gpiod, state);
    if (ret < 0)
        return ret;

    return devm_led_classdev_register(parent, &led_dat->cdev);
}

struct gpio_leds_priv {
    int num_leds;
    struct gpio_led_data leds[];
};

static inline int sizeof_gpio_leds_priv(int num_leds)
{
    return sizeof(struct gpio_leds_priv) +
        (sizeof(struct gpio_led_data) * num_leds);
}

static struct gpio_leds_priv *gpio_leds_create(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    int i = 0;
    int ret = 0;

    priv = devm_kzalloc(dev, sizeof_gpio_leds_priv(leds_count), GFP_KERNEL);
    if (!priv)
        return ERR_PTR(-ENOMEM);

    for(i = 0; i < leds_count && i < MAX_LEDS_COUNT; i++){

        struct gpio_led_data *led_dat = &priv->leds[i];
        struct gpio_led led = {};
        led.name = leds_node_name[i];
        ret = rtk_leds_parse_and_register_dev(&leds_name_list[i], &led);
        if (ret < 0) {
            return ERR_PTR(ret);
        }

        ret = create_gpio_led(&led, led_dat, dev, NULL, NULL);
        if (ret < 0) {
            return ERR_PTR(ret);
        }
        //creat_led_device_node(led_dat->cdev.dev);
        priv->num_leds++;
    }

    return priv;
}

static const struct of_device_id of_gpio_leds_match[] = {
    { .compatible = "leds-gpio", },
    {},
};

MODULE_DEVICE_TABLE(of, of_gpio_leds_match);

static int rtk_gpio_led_probe(struct platform_device *pdev)
{
    priv = gpio_leds_create(pdev);
    if (IS_ERR(priv))
        return PTR_ERR(priv);

    platform_set_drvdata(pdev, priv);

    if(androidboot_quiescent == 0){
        rtk_led_set("PIN_LED_R", LED_OFF);
        rtk_led_set("PIN_LED_B", LED_ON);
    }

    return 0;
}

static void rtk_gpio_led_shutdown(struct platform_device *pdev)
{
    int i;
    struct gpio_leds_priv *priv = platform_get_drvdata(pdev);

    if(androidboot_quiescent == 0){
        rtk_led_set("PIN_LED_B", LED_OFF);
        rtk_led_set("PIN_LED_R", LED_ON);
    }

    for (i = 0; i < priv->num_leds; i++) {
        struct gpio_led_data *led = &priv->leds[i];

        if (!(led->cdev.flags & LED_RETAIN_AT_SHUTDOWN))
            gpio_led_set(&led->cdev, LED_OFF);
    }
}

int rtk_leds_parse_and_register_dev(RTK_PCB_LEDS_ENUM *pcbinfo, struct gpio_led *led)
{
    int ret = 0;
    RTK_GPIO_GROUP gpio_group = MIS_GPIO;
    int gpio_num   = 0;
    unsigned long long ullPcbMgrValue = 0x0;

    ret = pcb_mgr_get_enum_info_byname(pcbinfo->name, &ullPcbMgrValue);

    if(ret != 0)
        return -1;

    gpio_num = GET_PIN_INDEX(ullPcbMgrValue);
    led->default_state = (GET_GPIO_INIT_VALUE(ullPcbMgrValue) == 0)?LEDS_GPIO_DEFSTATE_OFF : LEDS_GPIO_DEFSTATE_ON;

    if (GET_PIN_TYPE(ullPcbMgrValue) == PCB_PIN_TYPE_GPIO) {
        gpio_group = MIS_GPIO;
        led->gpio = MIS_GPIO_OFST + gpio_num;
    } else if (GET_PIN_TYPE(ullPcbMgrValue) == PCB_PIN_TYPE_ISO_GPIO) {
        gpio_group = ISO_GPIO;
        led->gpio = ISO_GPIO_OFST + gpio_num;
    } else {
        return -1;
    }

    led->default_trigger  = "oneshot";
    led->retain_state_suspended = led->retain_state_shutdown = 1;
    led->active_low = 0;

    return 0;
}

static struct platform_driver rtk_gpio_led_driver = {
    .probe        = rtk_gpio_led_probe,
    .shutdown    = rtk_gpio_led_shutdown,
    .driver        = {
        .name    = "leds-gpio",
        .of_match_table = of_gpio_leds_match,
    },
};

static int led_state = LED_OFF;
int rtk_led_set(char *name,int enable)
{
    int i = 0;
    int ret = 0;
    int gpio_num = 0;
    RTK_GPIO_GROUP gpio_group = MIS_GPIO;
    int gpio_index = 0;
    int current_gpio_num = 0;
    int current_gpio_base = MIS_GPIO_OFST;
    unsigned long long ullPcbMgrValue = 0x0;

    if(!name)
        return -1;

    ret = pcb_mgr_get_enum_info_byname(name, &ullPcbMgrValue);
    if(ret != 0)
        return -1;

    if (GET_PIN_TYPE(ullPcbMgrValue) == PCB_PIN_TYPE_GPIO) {
        gpio_group = MIS_GPIO;
        current_gpio_base = MIS_GPIO_OFST;
    } else if (GET_PIN_TYPE(ullPcbMgrValue) == PCB_PIN_TYPE_ISO_GPIO) {
        gpio_group = ISO_GPIO;
        current_gpio_base = ISO_GPIO_OFST;
    } else {
        return -1;
    }

    gpio_index = GET_PIN_INDEX(ullPcbMgrValue);
    current_gpio_num = gpio_index + current_gpio_base;

    for (i = 0; i < priv->num_leds; i++) {
        struct gpio_led_data *led = &priv->leds[i];
        gpio_num = desc_to_gpio(led->gpiod);
        if(current_gpio_num == gpio_num){

            if(enable == LED_OFF)
                led_state = LED_OFF;
            else
                led_state = LED_ON;

            gpio_led_set(&led->cdev, led_state);

        }
    }
    return 0;
}
EXPORT_SYMBOL(rtk_led_set);
/*
int rtk_led_blink_set(char *name,unsigned long delayon, unsigned long delayoff)
{
    int i = 0;
    int ret = 0;
    int gpio_num = 0;
    RTK_GPIO_GROUP gpio_group = MIS_GPIO;
    int gpio_index = 0;
    int current_gpio_num = 0;
    int current_gpio_base = MIS_GPIO_OFST;
    unsigned long long ullPcbMgrValue = 0x0;

    if(!name)
        return -1;

    ret = pcb_mgr_get_enum_info_byname(name, &ullPcbMgrValue);
    if(ret != 0)
        return -1;

    if (GET_PIN_TYPE(ullPcbMgrValue) == PCB_PIN_TYPE_GPIO) {
        gpio_group = MIS_GPIO;
        current_gpio_base = MIS_GPIO_OFST;
    } else if (GET_PIN_TYPE(ullPcbMgrValue) == PCB_PIN_TYPE_ISO_GPIO) {
        gpio_group = ISO_GPIO;
        current_gpio_base = ISO_GPIO_OFST;
    } else {
        return -1;
    }

    gpio_index = GET_PIN_INDEX(ullPcbMgrValue);
    current_gpio_num = gpio_index + current_gpio_base;

    for (i = 0; i < priv->num_leds; i++) {
        struct gpio_led_data *led = &priv->leds[i];
        gpio_num = desc_to_gpio(led->gpiod);
        if(current_gpio_num == gpio_num){
            led_state = LED_ON;
            led_blink_set(&led->cdev, &delayon, &delayoff);
        }
    }
    return 0;
}
EXPORT_SYMBOL(rtk_led_blink_set);

int rtk_led_oneshot_set(char *name,unsigned long delayon, unsigned long delayoff, int invert)
{
    int i = 0;
    int ret = 0;
    int gpio_num = 0;
    RTK_GPIO_GROUP gpio_group = MIS_GPIO;
    int gpio_index = 0;
    int current_gpio_num = 0;
    int current_gpio_base = MIS_GPIO_OFST;
    unsigned long long ullPcbMgrValue = 0x0;

    if(!name)
        return -1;

    ret = pcb_mgr_get_enum_info_byname(name, &ullPcbMgrValue);
    if(ret != 0)
        return -1;

    if (GET_PIN_TYPE(ullPcbMgrValue) == PCB_PIN_TYPE_GPIO) {
        gpio_group = MIS_GPIO;
        current_gpio_base = MIS_GPIO_OFST;
    } else if (GET_PIN_TYPE(ullPcbMgrValue) == PCB_PIN_TYPE_ISO_GPIO) {
        gpio_group = ISO_GPIO;
        current_gpio_base = ISO_GPIO_OFST;
    } else {
        return -1;
    }

    gpio_index = GET_PIN_INDEX(ullPcbMgrValue);
    current_gpio_num = gpio_index + current_gpio_base;

    for (i = 0; i < priv->num_leds; i++) {
        struct gpio_led_data *led = &priv->leds[i];
        gpio_num = desc_to_gpio(led->gpiod);
        if(current_gpio_num == gpio_num){
            led_state = LED_ON;
            led_blink_set_oneshot(&led->cdev, &delayon, &delayoff,invert);
        }
    }
    return 0;
}
EXPORT_SYMBOL(rtk_led_oneshot_set);
*/

int pcb_mgr_get_leds_enum_info(RTK_PCB_LEDS_ENUM *pcbenumPtr)
{
    int i;
    int index = 0;
    PCB_PIN_TYPE_T     pin_type;

    struct linux_pcb_leds_enum_t  *linux_pcb_enum_ptr = (struct linux_pcb_leds_enum_t* )pcbenumPtr;

    if(linux_pcb_enum_ptr == NULL){
        return -1;
    }

    for(i = 0; i < PCB_ENUM_MAX; i++){
        if(memcmp(pcb_enum_all[i].name, "PIN_LEDS", 8) == 0){
            pin_type = GET_PIN_TYPE(pcb_enum_all[i].value);
            if(PCB_PIN_TYPE_GPIO == pin_type  || PCB_PIN_TYPE_ISO_GPIO == pin_type){
                strncpy(linux_pcb_enum_ptr[index].name, pcb_enum_all[i].name, PCB_ENUM_NAME_LEN_MAX-1);
                linux_pcb_enum_ptr[index].value = pcb_enum_all[i].value;
                index++;
            }
        }
    }

    return index;
}

#ifdef CONFIG_RTK_KDRV_LEDS
static int __init leds_ops_init(char *options)
{
    if(options == NULL){
        androidboot_quiescent = 0;
        RTK_LED_INFO("Leds androidboot.quiescent no define\n");
    }
    else{
        RTK_LED_INFO("androidboot_quiescent: %s\n", options);
        if (sscanf(options, "%d", &androidboot_quiescent) < 1){
            return 1;
        }
    }

    return 1;
}
early_param("androidboot.quiescent", leds_ops_init);
#endif

static int __init rtk_gpio_led_init(void)
{
    memset(leds_name_list,0,sizeof(RTK_PCB_LEDS_ENUM)*MAX_LEDS_COUNT);
    leds_count = pcb_mgr_get_leds_enum_info(leds_name_list);

#ifdef CONFIG_RTK_KDRV_LEDS_MODULE
    androidboot_quiescent = rtk_parse_commandline("androidboot.quiescen"); 
    RTK_LED_INFO("cmdline get androidboot.quiescen:%d \n",androidboot_quiescent);
#endif

    g_led_platform_dev =  platform_device_register_simple("leds-gpio", -1, NULL, 0);
    if (g_led_platform_dev == NULL)
        goto REGISTER_PLATFORM_DEVICE_FAIL;

    platform_driver_register(&rtk_gpio_led_driver);

    RTK_LED_INFO("leds driver init sucess\n");
    return 0;

REGISTER_PLATFORM_DEVICE_FAIL:
    platform_device_unregister(g_led_platform_dev);
    return -1;
}

static void __exit rtk_gpio_led_exit(void)
{

    platform_driver_unregister(&rtk_gpio_led_driver);

    platform_device_unregister(g_led_platform_dev);
}

module_init(rtk_gpio_led_init);
module_exit(rtk_gpio_led_exit);
MODULE_LICENSE("GPL");

