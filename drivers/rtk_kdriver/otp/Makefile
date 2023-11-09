KBUILD_CPPFLAGS += -I ./
obj-$(CONFIG_RTK_KDRV_OTP) += rtk-otp.o
rtk-otp-objs := rtk_otp_util.o rtk_otp_hw.o rtk_otp_dev.o  rtk_otp_region.o rtk_otp_region_api.o
									
ifeq ($(CONFIG_RTK_KDRV_HDCP_KEY_FROM_OTP),y)
rtk-otp-objs += rtk_otp_hdcp_util.o
endif