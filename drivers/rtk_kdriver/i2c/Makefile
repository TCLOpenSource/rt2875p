#
# Makefile for the i2c core.
#

#kdriver support
ifdef CONFIG_RTK_KDRIVER_SUPPORT
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include
endif
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/i2c/$(CHIP_NAME)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/i2c/busses

obj-$(CONFIG_RTK_KDRV_I2C) += rtk-i2c.o
rtk-i2c-objs := algos/i2c-algo-rtk.o busses/rtk_i2c.o busses/i2c-rtk-api.o busses/rtk_i2c_priv.o busses/rtk_i2c_priv_g2c.o busses/rtk_i2c_attr.o busses/rtk_i2c_test.o

ccflags-$(CONFIG_I2C_DEBUG_CORE) := -DDEBUG
KBUILD_CFLAGS += -Wall -Werror

