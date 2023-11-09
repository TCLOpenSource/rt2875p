obj-$(CONFIG_RTK_KDRV_VDEC) += rtk_vdec.o
rtk_vdec-objs := rtk_vdec_driver.o rtk_vdec_mm.o rtk_vdec_utils.o rtk_vdec_svp.o

ifneq ($(ARCH),arm64)
KBUILD_CFLAGS += -Werror
endif
