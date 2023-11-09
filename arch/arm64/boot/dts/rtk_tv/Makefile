# SPDX-License-Identifier: GPL-2.0-only

dtb-$(CONFIG_ARCH_RTK2851A) += rtd2851a64_Q.dtb
dtb-$(CONFIG_ARCH_RTK2851C) += rtd2851c.dtb
dtb-$(CONFIG_ARCH_RTK2851F) += rtd2851f.dtb
dtb-$(CONFIG_ARCH_RTK2885M) += rtd2885m64_R.dtb
#dtb-$(CONFIG_ARCH_RTK6748) += rtd6748.dtb
dtb-$(CONFIG_ARCH_RTK6702) += rtd6702.dtb
dtb-$(CONFIG_ARCH_RTK2885P) += rtd2885p.dtb

# upstream DTC_INCLUDE and dtc_cpp_flags is not appendable, need to rework Makefile.lib or put to dtc/include-prefixes
DTC_INCLUDE += $(srctree)/drivers/rtk_kdriver/include/$(CHIP_NAME)/rtk_kdriver


.PHONY: dtbo.img


