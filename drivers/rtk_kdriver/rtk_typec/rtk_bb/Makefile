KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/rtk_typec/rtk_bb/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/rtk_typec/rtk_bb/$(CHIP_NAME)/
obj-$(CONFIG_RTK_KDRV_USB_BB) += rtk-usb-bb.o 
rtk-usb-bb-y := rtk_bb.o
rtk-usb-bb-y += $(CHIP_NAME)/rtk_bb_wrapper.o