#Copyright (C) 2007-2014 Realtek Semiconductor Corporation.

KBUILD_CFLAGS += -Wall -Werror
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/japan4k_dmx/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/demux_tv001/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/tp/$(CHIP_NAME)/

obj-$(CONFIG_RTK_KDRV_TP) += rtk-tp.o

rtk-tp-objs := tp_drv.o \
		tp_drv_verify.o \
		tp_drv_global.o \
        tp_drv_api.o \
        tp_reg_ctrl.o \
		tp_drv_lock.o \
		tp_drv_dump.o \
		tp_ta.o \
		$(CHIP_NAME)/tp_soc.o
