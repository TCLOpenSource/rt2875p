# Makefile for the Realtek HDCP OPTEE drivers.
#
#KBUILD_CFLAGS += -Wall -Werror
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/rtk_kdriver
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver

obj-$(CONFIG_RTK_HDCP_OPTEE) += hdcp_optee.o
hdcp_optee-objs += hdcp_optee_drv.o

ifneq (n,$(filter n, $(CONFIG_OPTEE_HDCP2)))
hdcp_optee-objs += hdcp2_optee.o
endif

ifneq (n,$(filter n, $(CONFIG_OPTEE_HDCP14)))
hdcp_optee-objs += hdcp14_optee.o
endif
