KBUILD_CFLAGS += -Wall -Werror
ifeq ($(CHIP_NAME),$(filter $(CHIP_NAME),rtd2851c rtd2851f))
KBUILD_CPPFLAGS += -DCONFIG_RTK_MD_USE_SW
endif
obj-$(CONFIG_RTK_KDRV_MD) += rtk-md.o
rtk-md-objs := rtk_md_dev.o rtk_md.o rtk_md_platform.o rtk_md_hw.o rtk_md_attr.o rtk_md_test.o

