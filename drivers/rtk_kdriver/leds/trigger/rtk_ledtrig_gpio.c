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

struct gpio_trig_data {
	struct led_classdev *led;

	unsigned desired_brightness;	/* desired brightness when led is on */
	unsigned inverted;		/* true when gpio is inverted */
	unsigned gpio;			/* gpio that triggers the leds */
};

static irqreturn_t gpio_trig_irq(int irq, void *_led)
{
	struct led_classdev *led = _led;
	struct gpio_trig_data *gpio_data = led->trigger_data;
	int tmp;

	tmp = gpio_get_value_cansleep(gpio_data->gpio);
	if (gpio_data->inverted)
		tmp = !tmp;

	if (tmp) {
		if (gpio_data->desired_brightness)
			led_set_brightness_nosleep(gpio_data->led,
					   gpio_data->desired_brightness);
		else
			led_set_brightness_nosleep(gpio_data->led, LED_FULL);
	} else {
		led_set_brightness_nosleep(gpio_data->led, LED_OFF);
	}

	return IRQ_HANDLED;
}

ssize_t rtk_gpio_trig_brightness_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led = dev_get_drvdata(dev);
	struct gpio_trig_data *gpio_data = led->trigger_data;

	return sprintf(buf, "%u\n", gpio_data->desired_brightness);
}
EXPORT_SYMBOL(rtk_gpio_trig_brightness_show);

ssize_t rtk_gpio_trig_brightness_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
	struct led_classdev *led = dev_get_drvdata(dev);
	struct gpio_trig_data *gpio_data = led->trigger_data;
	unsigned desired_brightness;
	int ret;

	ret = sscanf(buf, "%u", &desired_brightness);
	if (ret < 1 || desired_brightness > 255) {
		dev_err(dev, "invalid value\n");
		return -EINVAL;
	}

	gpio_data->desired_brightness = desired_brightness;

	return n;
}
EXPORT_SYMBOL(rtk_gpio_trig_brightness_store);

ssize_t rtk_gpio_trig_inverted_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led = dev_get_drvdata(dev);
	struct gpio_trig_data *gpio_data = led->trigger_data;

	return sprintf(buf, "%u\n", gpio_data->inverted);
}
EXPORT_SYMBOL(rtk_gpio_trig_inverted_show);

ssize_t rtk_gpio_trig_inverted_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
	struct led_classdev *led = dev_get_drvdata(dev);
	struct gpio_trig_data *gpio_data = led->trigger_data;
	unsigned long inverted;
	int ret;

	ret = kstrtoul(buf, 10, &inverted);
	if (ret < 0)
		return ret;

	if (inverted > 1)
		return -EINVAL;

	gpio_data->inverted = inverted;

	/* After inverting, we need to update the LED. */
	gpio_trig_irq(0, led);

	return n;
}
EXPORT_SYMBOL(rtk_gpio_trig_inverted_store);

ssize_t rtk_gpio_trig_gpio_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led = dev_get_drvdata(dev);
	struct gpio_trig_data *gpio_data = led->trigger_data;

	return sprintf(buf, "%u\n", gpio_data->gpio);
}
EXPORT_SYMBOL(rtk_gpio_trig_gpio_show);

ssize_t rtk_gpio_trig_gpio_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
	struct led_classdev *led = dev_get_drvdata(dev);
	struct gpio_trig_data *gpio_data = led->trigger_data;
	unsigned gpio;
	int ret;

	ret = sscanf(buf, "%u", &gpio);
	if (ret < 1) {
		dev_err(dev, "couldn't read gpio number\n");
		return -EINVAL;
	}

	if (gpio_data->gpio == gpio)
		return n;

	if (!gpio) {
		if (gpio_data->gpio != 0)
			free_irq(gpio_to_irq(gpio_data->gpio), led);
		gpio_data->gpio = 0;
		return n;
	}

	ret = request_threaded_irq(gpio_to_irq(gpio), NULL, gpio_trig_irq,
			IRQF_ONESHOT | IRQF_SHARED | IRQF_TRIGGER_RISING
			| IRQF_TRIGGER_FALLING, "ledtrig-gpio", led);
	if (ret) {
		dev_err(dev, "request_irq failed with error %d\n", ret);
	} else {
		if (gpio_data->gpio != 0)
			free_irq(gpio_to_irq(gpio_data->gpio), led);
		gpio_data->gpio = gpio;
		/* After changing the GPIO, we need to update the LED. */
		gpio_trig_irq(0, led);
	}

	return ret ? ret : n;
}
EXPORT_SYMBOL(rtk_gpio_trig_gpio_store);

static int gpio_trig_activate(struct led_classdev *led)
{
	struct gpio_trig_data *gpio_data;

	gpio_data = kzalloc(sizeof(*gpio_data), GFP_KERNEL);
	if (!gpio_data)
		return -1;

	gpio_data->led = led;
	led->trigger_data = gpio_data;
	led->activated = true;

	return 0;
}

static void gpio_trig_deactivate(struct led_classdev *led)
{
	struct gpio_trig_data *gpio_data = led->trigger_data;

	if (led->activated) {
		if (gpio_data->gpio != 0)
			free_irq(gpio_to_irq(gpio_data->gpio), led);
		kfree(gpio_data);
		led->activated = false;
	}
}

static struct led_trigger gpio_led_trigger = {
	.name		= "gpio",
	.activate	= gpio_trig_activate,
	.deactivate	= gpio_trig_deactivate,
};

int gpio_trig_init(void)
{
	return led_trigger_register(&gpio_led_trigger);
}
//module_init(gpio_trig_init);

void gpio_trig_exit(void)
{
	led_trigger_unregister(&gpio_led_trigger);
}
//module_exit(gpio_trig_exit);

MODULE_AUTHOR("Felipe Balbi <me@felipebalbi.com>");
MODULE_DESCRIPTION("GPIO LED trigger");
MODULE_LICENSE("GPL");
