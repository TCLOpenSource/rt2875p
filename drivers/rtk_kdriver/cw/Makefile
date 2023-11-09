#
# Makefile for the Realtek mcp drivers.
#

KBUILD_CPPFLAGS += -I ./

obj-$(CONFIG_RTK_KDRV_CW) += rtk-cw.o
rtk-cw-objs := rtk_cw_dev.o  rtk_cw_util.o

ifeq ($(CONFIG_RTK_KDRV_CW_SIGNEL_CPU),y)
rtk-cw-objs += scpu/rtk_cw_adapter_scpu.o
endif

