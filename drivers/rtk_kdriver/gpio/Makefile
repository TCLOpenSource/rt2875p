KBUILD_CFLAGS += -Werror

obj-$(CONFIG_RTK_KDRV_GPIO) += rtk-gpio.o
rtk-gpio-objs := rtk_gpio_$(CHIP_NAME).o rtk_gpio-dev.o rtk_gpio-db.o rtk_gpio-debug.o io_wrapper.o

ifdef CONFIG_RTK_KDRV_GPIO_BUILT_IN_TEST
rtk-gpio-objs += rtk_gpio-bist.o
endif

ifdef CONFIG_RTK_KDRV_GPIO_WITH_SYSFS_DEBUG
rtk-gpio-objs += rtk_gpio-attr.o
endif

ifdef CONFIG_RTK_KDRV_GPIO_GPIOLIB_SUPPORT
rtk-gpio-objs += rtk_gpio-chipset.o
endif

ifdef CONFIG_RTK_KDRV_MIO
rtk-gpio-objs += rtk_mio_gpio.o
endif

ifdef CONFIG_RTK_KDRV_GPIO_HWCNT
rtk-gpio-objs += rtk_gpio_hwcnt.o
endif
