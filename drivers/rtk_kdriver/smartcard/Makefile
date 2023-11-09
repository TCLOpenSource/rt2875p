KBUILD_CFLAGS += -Werror
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/smartcard/core/

rtk_scd-objs = core/rtk_scd_core.o \
        core/rtk_scd_buff.o \
        core/rtk_scd_dev.o \
        core/rtk_scd_atr.o \
        core/rtk_scd_debug.o \
        core/rtk_scd_sysfs.o \
        adapter/rtk_scd_ops.o \
        adapter/rtk_scd_priv.o

obj-$(CONFIG_RTK_KDRV_SMARTCARD)  += rtk_scd.o
