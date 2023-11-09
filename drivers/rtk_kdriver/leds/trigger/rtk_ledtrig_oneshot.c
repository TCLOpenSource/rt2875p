// SPDX-License-Identifier: GPL-2.0-only
/*
 * One-shot LED Trigger
 *
 * Copyright 2012, Fabio Baltieri <fabio.baltieri@gmail.com>
 *
 * Based on ledtrig-timer.c by Richard Purdie <rpurdie@openedhand.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include "../../../leds/leds.h"

#define DEFAULT_DELAY 100

struct oneshot_trig_data {
	unsigned int invert;
};

ssize_t rtk_oneshot_led_shot(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct oneshot_trig_data *oneshot_data = led_cdev->trigger_data;

	led_blink_set_oneshot(led_cdev,
			&led_cdev->blink_delay_on, &led_cdev->blink_delay_off,
			oneshot_data->invert);

	/* content is ignored */
	return size;
}
EXPORT_SYMBOL(rtk_oneshot_led_shot);

ssize_t rtk_oneshot_led_invert_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct oneshot_trig_data *oneshot_data = led_cdev->trigger_data;

	return sprintf(buf, "%u\n", oneshot_data->invert);
}
EXPORT_SYMBOL(rtk_oneshot_led_invert_show);

ssize_t rtk_oneshot_led_invert_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct oneshot_trig_data *oneshot_data = led_cdev->trigger_data;
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	oneshot_data->invert = !!state;

	if (oneshot_data->invert)
		led_set_brightness_nosleep(led_cdev, LED_FULL);
	else
		led_set_brightness_nosleep(led_cdev, LED_OFF);

	return size;
}
EXPORT_SYMBOL(rtk_oneshot_led_invert_store);

ssize_t rtk_oneshot_led_delay_on_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);

	return sprintf(buf, "%lu\n", led_cdev->blink_delay_on);
}
EXPORT_SYMBOL(rtk_oneshot_led_delay_on_show);

ssize_t rtk_oneshot_led_delay_on_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	led_cdev->blink_delay_on = state;

	return size;
}
EXPORT_SYMBOL(rtk_oneshot_led_delay_on_store);

ssize_t rtk_oneshot_led_delay_off_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);

	return sprintf(buf, "%lu\n", led_cdev->blink_delay_off);
}
EXPORT_SYMBOL(rtk_oneshot_led_delay_off_show);

ssize_t rtk_oneshot_led_delay_off_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	led_cdev->blink_delay_off = state;

	return size;
}
EXPORT_SYMBOL(rtk_oneshot_led_delay_off_store);

static int oneshot_trig_activate(struct led_classdev *led_cdev)
{
	struct oneshot_trig_data *oneshot_data;

	oneshot_data = kzalloc(sizeof(*oneshot_data), GFP_KERNEL);
	if (!oneshot_data)
		return -1;

	led_cdev->trigger_data = oneshot_data;
	led_cdev->blink_delay_on = DEFAULT_DELAY;
	led_cdev->blink_delay_off = DEFAULT_DELAY;

	led_cdev->activated = true;

	return 0;
}

static void oneshot_trig_deactivate(struct led_classdev *led_cdev)
{
	struct oneshot_trig_data *oneshot_data = led_cdev->trigger_data;

	if (led_cdev->activated) {
		kfree(oneshot_data);
		led_cdev->activated = false;
	}

	/* Stop blinking */
	led_set_brightness(led_cdev, LED_OFF);
}

static struct led_trigger oneshot_led_trigger = {
	.name     = "oneshot",
	.activate = oneshot_trig_activate,
	.deactivate = oneshot_trig_deactivate,
};

int oneshot_trig_init(void)
{
	return led_trigger_register(&oneshot_led_trigger);
}

void oneshot_trig_exit(void)
{
	led_trigger_unregister(&oneshot_led_trigger);
}

//module_init(oneshot_trig_init);
//module_exit(oneshot_trig_exit);
MODULE_AUTHOR("Fabio Baltieri <fabio.baltieri@gmail.com>");
MODULE_DESCRIPTION("One-shot LED trigger");
MODULE_LICENSE("GPL v2");
