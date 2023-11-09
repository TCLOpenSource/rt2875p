# Makefile for the Realtek Audio HW drivers.

obj-$(CONFIG_RTK_KDRV_AUDIO_HW) += audio_hw.o
audio_hw-objs := audio_hw_driver.o audio_hw_ioctl.o audio_hw_aio.o audio_hw_app.o audio_hw_atv.o audio_hw_rpc.o audio_hw_pm.o audio_hw_earc_ioctl.o

obj-$(CONFIG_RTK_KDRV_AUDIO_HW_EARC) += audio_hw_earc.o
audio_hw_earc-objs := audio_hw_earc_driver.o audio_hw_earc_ioctl.o audio_hw_earc_pm.o

KBUILD_CFLAGS += -Werror