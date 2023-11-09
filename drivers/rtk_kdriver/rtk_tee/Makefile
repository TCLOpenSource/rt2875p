#KASAN_SANITIZE := n
#
# rtk tee Makefile for the linux kernel.
#

ifneq ($(SOC_NAME),)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common_$(SOC_NAME)/include/
else
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/include/
endif

KBUILD_CPPFLAGS += -I$(srctree)/drivers/tee/optee/

obj-$(CONFIG_RTK_KDRV_TEE) += rtk-tee.o
rtk-tee-objs := rtk_shm_pool.o  optee_debugfs.o rtk_tee_call.o
