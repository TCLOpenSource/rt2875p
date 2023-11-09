KBUILD_CFLAGS += -Werror

obj-$(CONFIG_RTK_KDRV_LEDS) += rtk-led.o
# LED Triggers
#obj-$(CONFIG_LEDS_TRIGGERS)		+= trigger/

ifdef CONFIG_RTK_KDRV_I2C_LED
rtk-led-i2c-objs += led_i2c/rtk_led_interface.o led_i2c/led.o
endif


obj-$(CONFIG_RTK_KDRV_SERIAL_LED) += rtk_led_serial.o
rtk_led_serial-objs +=  led_i2c/rtk_led_serial.o

ifdef CONFIG_RTK_KDRV_GPIO_LED
rtk-led-objs += led_gpio/rtk_leds.o
endif

ifdef CONFIG_RTK_KDRV_GPIO_LED
#rtk-led-objs += trigger/rtk_ledtrig_attr.o
endif

ifdef CONFIG_RTK_LEDS_TRIGGER_TIMER
rtk-led-objs += trigger/rtk_ledtrig_timer.o
endif

ifdef CONFIG_RTK_LEDS_TRIGGER_ONESHOT
rtk-led-objs += trigger/rtk_ledtrig_oneshot.o
endif

ifdef CONFIG_RTK_LEDS_TRIGGER_GPIO
rtk-led-objs += trigger/rtk_ledtrig_gpio.o
endif
