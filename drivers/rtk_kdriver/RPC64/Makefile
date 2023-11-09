#KASAN_SANITIZE := n
#
# Makefile for the linux kernel.
#

KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/include/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/IPC/generate/include/system
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/IPC/include
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/rtk_kdriver
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver

obj-$(CONFIG_RTK_KDRV_RPC) += rtk-rpc.o
rtk-rpc-objs := RPCDriver64.o RPCintr64.o RPCpoll64.o RPCkern64.o ../misc/$(CHIP_NAME)/rtk_avcpu.o
