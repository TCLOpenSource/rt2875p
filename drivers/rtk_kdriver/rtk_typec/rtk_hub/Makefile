KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/rtk_typec/rtk_hub/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/rtk_typec/rtk_hub/$(CHIP_NAME)/
obj-$(CONFIG_RTK_KDRV_USB_HUB) += rtk-usb-hub.o 
rtk-usb-hub-y := rtk_usb_hub.o rtk_usb_hub_fw.o rtk_usb_hub_smbus.o rtk_usb_hub_camera.o
rtk-usb-hub-y += $(CHIP_NAME)/rtk_usb_hub_wrapper.o