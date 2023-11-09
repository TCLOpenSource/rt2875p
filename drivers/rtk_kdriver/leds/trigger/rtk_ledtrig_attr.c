// SPDX-License-Identifier: GPL-2.0-only
/*
 * ledtrig-gio.c - LED Trigger Based on GPIO events
 *
 * Copyright 2009 Felipe Balbi <me@felipebalbi.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include "../../../leds/leds.h"

#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_GPIO)
extern ssize_t rtk_gpio_trig_brightness_show(struct device *dev,
		struct device_attribute *attr, char *buf);
extern ssize_t rtk_gpio_trig_brightness_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n);
extern ssize_t rtk_gpio_trig_inverted_show(struct device *dev,
		struct device_attribute *attr, char *buf);
extern ssize_t rtk_gpio_trig_inverted_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n);
extern ssize_t rtk_gpio_trig_gpio_show(struct device *dev,
		struct device_attribute *attr, char *buf);
extern ssize_t rtk_gpio_trig_gpio_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n);
#endif
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_TIMER)
extern ssize_t rtk_timer_led_delay_on_show(struct device *dev,
		struct device_attribute *attr, char *buf);
extern ssize_t rtk_timer_led_delay_on_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size);
extern ssize_t rtk_timer_led_delay_off_show(struct device *dev,
		struct device_attribute *attr, char *buf);
extern ssize_t rtk_timer_led_delay_off_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size);
#endif
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_ONESHOT)
extern ssize_t rtk_oneshot_led_shot(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size);
extern ssize_t rtk_oneshot_led_invert_show(struct device *dev,
		struct device_attribute *attr, char *buf);
extern ssize_t rtk_oneshot_led_invert_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size);
extern ssize_t rtk_oneshot_led_delay_on_show(struct device *dev,
		struct device_attribute *attr, char *buf);
extern ssize_t rtk_oneshot_led_delay_on_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size);
extern ssize_t rtk_oneshot_led_delay_off_show(struct device *dev,
		struct device_attribute *attr, char *buf);
extern ssize_t rtk_oneshot_led_delay_off_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size);
#endif

static ssize_t led_desired_brightness_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
    struct led_classdev *led = dev_get_drvdata(dev);

    if(led == NULL || led->trigger == NULL){
        dev_err(dev, "L%d:led trigger not register\n", __LINE__);
        return -EINVAL;
    }
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_GPIO)
    if(strcmp(led->trigger->name, "gpio") == 0){
        return rtk_gpio_trig_brightness_show(dev, attr, buf);
    }
#endif
    dev_err(dev, "L%d:Please change trigger mode to:%s\n", __LINE__, led->trigger->name);
    return -1;

}

static ssize_t led_desired_brightness_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
    struct led_classdev *led = dev_get_drvdata(dev);

    if(led == NULL || led->trigger == NULL){
        dev_err(dev, "L%d:led trigger not register\n", __LINE__);
        return -EINVAL;
    }
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_GPIO)
    if(strcmp(led->trigger->name, "gpio") == 0){
        return rtk_gpio_trig_brightness_store(dev, attr, buf, n);
    }
#endif
    dev_err(dev, "L%d:Please change trigger mode to:%s\n", __LINE__, led->trigger->name);
    return -1;
}

static ssize_t led_gpio_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
    struct led_classdev *led = dev_get_drvdata(dev);

    if(led == NULL || led->trigger == NULL){
        dev_err(dev, "L%d:led trigger not register\n", __LINE__);
        return -EINVAL;
    }
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_GPIO)
    if(strcmp(led->trigger->name, "gpio") == 0){
        return rtk_gpio_trig_gpio_show(dev, attr, buf);
    }
#endif
    dev_err(dev, "L%d:Please change trigger mode to:%s\n", __LINE__, led->trigger->name);
    return -1;

}

static ssize_t led_gpio_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
    struct led_classdev *led = dev_get_drvdata(dev);

    if(led == NULL || led->trigger == NULL){
        dev_err(dev, "L%d:led trigger not register\n", __LINE__);
        return -EINVAL;
    }
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_GPIO)
    if(strcmp(led->trigger->name, "gpio") == 0){
        return rtk_gpio_trig_gpio_store(dev, attr, buf, n);
    }
#endif
    dev_err(dev, "L%d:Please change trigger mode to:%s\n", __LINE__, led->trigger->name);
    return -1;
}

static ssize_t led_delay_on_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
    struct led_classdev *led = dev_get_drvdata(dev);

    if(led == NULL || led->trigger == NULL){
        dev_err(dev, "L%d:led trigger not register\n", __LINE__);
        return -EINVAL;
    }
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_TIMER)
    if(strcmp(led->trigger->name, "timer") == 0){
        return rtk_timer_led_delay_on_show(dev, attr, buf);
    }
#endif
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_ONESHOT)
    if(strcmp(led->trigger->name, "oneshot") == 0){
        return rtk_oneshot_led_delay_on_show(dev, attr, buf);
    }
#endif
    dev_err(dev, "L%d:Please change trigger mode to:%s\n", __LINE__, led->trigger->name);
    return -1;
}

static ssize_t led_delay_on_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
    struct led_classdev *led = dev_get_drvdata(dev);

    if(led == NULL || led->trigger == NULL){
        dev_err(dev, "L%d:led trigger not register\n", __LINE__);
        return -EINVAL;
    }
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_TIMER)
    if(strcmp(led->trigger->name, "timer") == 0){
        return rtk_timer_led_delay_on_store(dev, attr, buf, n);
    }
#endif
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_ONESHOT)
    if(strcmp(led->trigger->name, "oneshot") == 0){
        return rtk_oneshot_led_delay_on_store(dev, attr, buf, n);
    }
#endif
    dev_err(dev, "L%d:Please change trigger mode to:%s\n", __LINE__, led->trigger->name);
    return -1;
}

static ssize_t led_delay_off_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
    struct led_classdev *led = dev_get_drvdata(dev);

    if(led == NULL || led->trigger == NULL){
        dev_err(dev, "L%d:led trigger not register\n", __LINE__);
        return -EINVAL;
    }
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_TIMER)
    if(strcmp(led->trigger->name, "timer") == 0){
        return rtk_timer_led_delay_off_show(dev, attr, buf);
    }
#endif
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_ONESHOT)
    if(strcmp(led->trigger->name, "oneshot") == 0){
        return rtk_oneshot_led_delay_off_show(dev, attr, buf);
    }
#endif
    dev_err(dev, "L%d:Please change trigger mode to:%s\n", __LINE__, led->trigger->name);
    return -1;
}

static ssize_t led_delay_off_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
    struct led_classdev *led = dev_get_drvdata(dev);

    if(led == NULL || led->trigger == NULL){
        dev_err(dev, "L%d:led trigger not register\n", __LINE__);
        return -EINVAL;
    }
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_TIMER)
    if(strcmp(led->trigger->name, "timer") == 0){
        return rtk_timer_led_delay_off_store(dev, attr, buf, n);
    }
#endif
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_ONESHOT)
    if(strcmp(led->trigger->name, "oneshot") == 0){
        return rtk_oneshot_led_delay_off_store(dev, attr, buf, n);
    }
#endif
    dev_err(dev, "L%d:Please change trigger mode to:%s\n", __LINE__, led->trigger->name);
    return -1;
}

static ssize_t led_invert_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
    struct led_classdev *led = dev_get_drvdata(dev);

    if(led == NULL || led->trigger == NULL){
        dev_err(dev, "L%d:led trigger not register\n", __LINE__);
        return -EINVAL;
    }
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_GPIO)
    if(strcmp(led->trigger->name, "gpio") == 0){
        return rtk_gpio_trig_inverted_show(dev, attr, buf);
    }
#endif
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_ONESHOT)
    if(strcmp(led->trigger->name, "oneshot") == 0){
        return rtk_oneshot_led_invert_show(dev, attr, buf);
    }
#endif
    dev_err(dev, "L%d:Please change trigger mode to:%s\n", __LINE__, led->trigger->name);
    return -1;
}

static ssize_t led_invert_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
    struct led_classdev *led = dev_get_drvdata(dev);

    if(led == NULL || led->trigger == NULL){
        dev_err(dev, "L%d:led trigger not register\n", __LINE__);
        return -EINVAL;
    }
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_GPIO)
    if(strcmp(led->trigger->name, "gpio") == 0){
        return rtk_gpio_trig_inverted_store(dev, attr, buf, n);
    }
#endif
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_ONESHOT)
    if(strcmp(led->trigger->name, "oneshot") == 0){
        return rtk_oneshot_led_invert_store(dev, attr, buf, n);
    }
#endif
    dev_err(dev, "L%d:Please change trigger mode to:%s\n", __LINE__, led->trigger->name);
    return -1;
}

static ssize_t led_shot(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
    struct led_classdev *led = dev_get_drvdata(dev);

    if(led == NULL || led->trigger == NULL){
        dev_err(dev, "L%d:led trigger not register\n", __LINE__);
        return -EINVAL;
    }
#if IS_ENABLED(CONFIG_RTK_LEDS_TRIGGER_ONESHOT)
    if(strcmp(led->trigger->name, "oneshot") == 0){
        return rtk_oneshot_led_shot(dev, attr, buf, n);
    }
#endif
    dev_err(dev, "L%d:Please change trigger mode to:%s\n", __LINE__, led->trigger->name);
    return -1;
}

static DEVICE_ATTR(desired_brightness, 0664, led_desired_brightness_show, led_desired_brightness_store);
static DEVICE_ATTR(gpio, 0664, led_gpio_show, led_gpio_store);
static DEVICE_ATTR(delay_on, 0664, led_delay_on_show, led_delay_on_store);
static DEVICE_ATTR(delay_off, 0664, led_delay_off_show, led_delay_off_store);
static DEVICE_ATTR(invert, 0664, led_invert_show, led_invert_store);
static DEVICE_ATTR(shot, 0220, NULL, led_shot);

void creat_led_device_node(struct device *dev)
{
    device_create_file(dev, &dev_attr_desired_brightness);
    device_create_file(dev, &dev_attr_gpio);
    device_create_file(dev, &dev_attr_delay_on);
    device_create_file(dev, &dev_attr_delay_off);
    device_create_file(dev, &dev_attr_invert);
    device_create_file(dev, &dev_attr_shot);
}
EXPORT_SYMBOL(creat_led_device_node);

void remove_led_device_node(struct device *dev)
{
    device_remove_file(dev, &dev_attr_desired_brightness);
    device_remove_file(dev, &dev_attr_gpio);
    device_remove_file(dev, &dev_attr_delay_on);
    device_remove_file(dev, &dev_attr_delay_off);
    device_remove_file(dev, &dev_attr_invert);
    device_remove_file(dev, &dev_attr_shot);
}
EXPORT_SYMBOL(remove_led_device_node);
 
MODULE_LICENSE("GPL");
