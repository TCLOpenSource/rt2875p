#
# Makefile for the linux kernel.
#
KBUILD_CFLAGS += -Werror
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/rtk_kdriver
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/ir/inc
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver
# Inpute device

obj-$(CONFIG_RTK_KDRV_IR) += rtk-irrc.o
rtk-irrc-objs += irrc_dev.o irrc_core.o irrc_sysfs.o irrc_filter.o


ifdef CONFIG_RTK_KDRV_RAW_MODE
rtk-irrc-objs += irrc_raw.o
endif

ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
rtk-irrc-objs += ir_input.o ir_input_intercept.o
endif

ifdef CONFIG_RTK_KDRV_INPUT_DEVICE_MOUSE
rtk-irrc-objs += ir_input_mouse.o
endif

ifdef CONFIG_RTK_KDRV_RECORD_KEY
rtk-irrc-objs += irrc_record.o
endif

obj-$(CONFIG_RTK_KDRV_IR_MOD_KEY_TOOL) += irrc_modulate.o

ifdef CONFIG_RTK_KDRV_SUPPORT_ALC_COMMAND
rtk-irrc-objs += rtk_voice_dev.o
endif

ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
rtk-irrc-objs += ir_table/ir_table_default.o
rtk-irrc-objs += ir_table/ir_table_others.o
rtk-irrc-objs += ir_table/ir_table_user.o
rtk-irrc-objs += ir_table/ir_table_tv001.o
rtk-irrc-objs += ir_table/ir_table_tv002.o
rtk-irrc-objs += ir_table/ir_table_tv010.o
rtk-irrc-objs += ir_table/ir_table_tv005.o
rtk-irrc-objs += ir_table/ir_table_tv013.o
rtk-irrc-objs += ir_table/ir_table_tv015.o
rtk-irrc-objs += ir_table/ir_table_tv030.o
rtk-irrc-objs += ir_table/ir_table_tv035.o
rtk-irrc-objs += ir_table/ir_table_tv036.o
rtk-irrc-objs += ir_table/ir_table_tv051.o
endif

KBUILD_CFLAGS += -Werror
