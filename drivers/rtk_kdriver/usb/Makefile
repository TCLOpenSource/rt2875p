KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/usb/include/

obj-$(CONFIG_RTK_KDRV_USB) += rtk_usb.o
rtk_usb-y := usb-platform.o rtk_usb_core.o rtk_usb_helper.o

rtk-xhci-plat-hcd-y := xhci-plat.o xhci-sysfs.o xhci-plat-hw.o
rtk-xhci-plat-hcd-y += $(CHIP_NAME)/xhci-plat-hw-wrapper.o
obj-$(CONFIG_RTK_KDRV_XHCI_HCD_PLATFORM) += rtk-xhci-plat-hcd.o

rtk-ehci-plat-y += ehci-plat.o ehci-sysfs.o ehci-plat-hw.o
rtk-ehci-plat-y += $(CHIP_NAME)/ehci-plat-hw-wrapper.o
obj-$(CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM) += rtk-ehci-plat.o

rtk-ohci-plat-y += ohci-plat.o
obj-$(CONFIG_RTK_KDRV_OHCI_HCD_PLATFORM) += rtk-ohci-plat.o

rtk-xhci-plat-drd-y += xhci-plat-drd.o
rtk-xhci-plat-drd-y += $(CHIP_NAME)/xhci-plat-drd-hw-wrapper.o
obj-$(CONFIG_RTK_KDRV_XHCI_DRD_PLATFORM) += rtk-xhci-plat-drd.o

rtk-otg-plat-drv-y += rtk-otg-plat.o
rtk-otg-plat-drv-y += $(CHIP_NAME)/rtk-otg-plat-wrapper.o
obj-$(CONFIG_RTK_KDRV_USB_OTG_PLATFORM) += rtk-otg-plat-drv.o
