# SPDX-License-Identifier: GPL-2.0
ifdef CONFIG_RTK_KDRIVER_SUPPORT
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/include/
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/include/rtk_kdriver
endif
ccflags-$(CONFIG_USB_DWC2_DEBUG)	+= -DDEBUG
ccflags-$(CONFIG_USB_DWC2_VERBOSE)	+= -DVERBOSE_DEBUG

obj-$(CONFIG_USB_DWC2)			+= dwc2.o
dwc2-y					:= core.o core_intr.o platform.o
dwc2-y					+= params.o

ifneq ($(filter y,$(CONFIG_USB_DWC2_HOST) $(CONFIG_USB_DWC2_DUAL_ROLE)),)
	dwc2-y				+= hcd.o hcd_intr.o
	dwc2-y				+= hcd_queue.o hcd_ddma.o
endif

ifneq ($(filter y,$(CONFIG_USB_DWC2_PERIPHERAL) $(CONFIG_USB_DWC2_DUAL_ROLE)),)
	dwc2-y       			+= gadget.o
endif

ifneq ($(CONFIG_DEBUG_FS),)
	dwc2-y				+= debugfs.o
endif

# NOTE: The previous s3c-hsotg peripheral mode only driver has been moved to
# this location and renamed gadget.c. When building for dynamically linked
# modules, dwc2.ko will get built for host mode, peripheral mode, and dual-role
# mode. The PCI bus interface module will called dwc2_pci.ko and the platform
# interface module will be called dwc2_platform.ko.

obj-$(CONFIG_USB_DWC2_PCI)		+= dwc2_pci.o
dwc2_pci-y				:= pci.o
