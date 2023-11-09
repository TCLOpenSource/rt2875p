#
# Makefile for the linux kernel.
#

ifdef CONFIG_RTK_KDRIVER_SUPPORT
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/$(CHIP_NAME)
endif
KBUILD_CPPFLAGS += -I$(srctree)/arch/$(ARCH)/mach-rtk/include
obj-$(CONFIG_CRYPTO_DEV_RTK_ZACC) += zacc_common.o zacc_dec.o zacc_enc.o
