#Copyright (C) 2007-2017 Realtek Semiconductor Corporation.
#
# Makefile for the linux kernel.
#

# DDC driver

KBUILD_CFLAGS += -Werror

obj-$(CONFIG_RTK_KDRV_DDC) += rtk-ddc.o
rtk-ddc-objs = rtk_ddc_drv.o rtk_ddc_priv.o rtk_ddc_lib.o rtk_ddc_sysfs.o

ifeq ($(filter y, $(CONFIG_ARCH_RTK2851C) $(CONFIG_ARCH_RTK2885P) $(CONFIG_ARCH_RTK2851F)),y)
ifeq ($(filter y, $(CONFIG_RTK_KDRV_DDCCI)),y)
rtk-ddc-objs += rtk_ddcci_priv.o rtk_ddcci_drv.o rtk_ddcci_sysfs.o rtk_ddcci_verify.o rtk_ddcci_verify_api.o
endif
endif

