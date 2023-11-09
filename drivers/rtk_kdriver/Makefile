###################################################################
# Setup Include Path
###################################################################
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/mach-rtk/include
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/$(CHIP_NAME)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/

KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/$(CHIP_NAME)/rtk_kdriver
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/misc/$(CHIP_NAME)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/gal/$(CHIP_NAME)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/tvscalercontrol/$(CHIP_NAME)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/emcu/$(CHIP_NAME)
ifneq ($(CONFIG_BYPASSMEMC_FOR_CODESIZE_SAVING),y)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/memc_isr/$(CHIP_NAME)
endif
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/rtk_kdriver
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/dolby_idk_1_6_1_1
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/include/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/IPC/generate/include/system
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/IPC/include

KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/ir/inc

ifeq ($(CONFIG_RTK_8KCODEC),y)
KBUILD_CPPFLAGS += -Idrivers/rtk_codec
endif
ifeq (y,$(filter y, $(CONFIG_CUSTOMER_TV002)))
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/platform/tv002/pq_ctrl_tv002/include
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/platform/tv002/pq_ctrl_tv002
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/platform/tv002/
endif

KBUILD_CPPFLAGS += -DLITTLE_ENDIAN
KBUILD_CFLAGS += -fno-unsigned-char

###################################################################
# Must have
###################################################################
obj-$(CONFIG_REALTEK_MEMORY_MANAGEMENT) += rmm/
obj-$(CONFIG_RTK_KDRV_CRT) += crt/$(CHIP_NAME)/

###################################################################
# Generic drivers
###################################################################

obj-$(CONFIG_ARCH_RTK) += mach-rtk/

#PCBMGR
obj-$(CONFIG_REALTEK_PCBMGR) += pcbmgr/

# RPC
ifeq ($(CONFIG_ARM64),y)
obj-$(CONFIG_RTK_KDRV_RPC) += RPC64/
else
obj-$(CONFIG_RTK_KDRV_RPC) += RPC/
endif

# TEE
obj-$(CONFIG_RTK_KDRV_TEE) += rtk_tee/

# rtk_semaphore
obj-$(CONFIG_RTK_SEMAPHORE) += misc/$(CHIP_NAME)/rtk_semaphore.o
obj-$(CONFIG_RTK_SEMAPHORE_TEST) += misc/$(CHIP_NAME)/rtk_semaphore_test.o


# EMCU
obj-$(CONFIG_RTK_KDRV_EMCU) += emcu/

# RTD LOGBUF
ifneq ($(CHIP_NAME),rtd2851a)
obj-$(CONFIG_REALTEK_LOGBUF) += misc/rtd_logger.o
else
obj-$(CONFIG_REALTEK_LOGBUF) += misc/$(CHIP_NAME)/rtd_logger.o
endif
obj-$(CONFIG_REALTEK_LOGBUF) += misc/rtd_log.o

#RTD LOG EVENT FORMAT
ifneq ($(CONFIG_RTK_KDRV_COMMON),m)
rtleventlog_path = $(shell pwd)/drivers/rtk_kdriver/rtleventlog
rtleventlog_exist = $(wildcard $(rtleventlog_path))
ifneq ($(rtleventlog_exist),)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/rtleventlog/
obj-$(CONFIG_REALTEK_EVENT_LOG) += rtleventlog/
else
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/misc/rtd_event_log/
obj-$(CONFIG_REALTEK_EVENT_LOG) += misc/rtd_event_log/
endif
endif

# HW-MONITOR
obj-$(CONFIG_RTK_KDRV_HW_MONITOR) += hw_monitor/

#DBG HELP
obj-$(CONFIG_RTK_ONLINE_HELP) += misc/$(CHIP_NAME)/rtk_online_help.o

obj-y += platform/
# GPIO
obj-$(CONFIG_RTK_KDRV_GPIO) += gpio/

obj-y += leds/
#obj-$(CONFIG_RTK_KDRV_GPIO) += lnbgpio/

# SWITCH
obj-$(CONFIG_RTK_KDRV_SWITCH) += switch/
# EXTCON
obj-$(CONFIG_RTK_KDRV_EXTCON)  += extcon/

# PWM
obj-$(CONFIG_RTK_KDRV_PWM) += pwm/
# PCMCIA
obj-$(CONFIG_RTK_KDRV_PCMCIA) += pcmcia/

# SMARTCARD
obj-$(CONFIG_RTK_KDRV_SMARTCARD) += smartcard/

# CEC
obj-$(CONFIG_RTK_KDRV_CEC) += rtk_cec/

# DDC
obj-$(CONFIG_RTK_KDRV_DDC) += ddc/
# CP
obj-$(CONFIG_RTK_KDRV_MCP) += mcp/

# CW
obj-$(CONFIG_RTK_KDRV_CW) += cw/

# OTP
obj-$(CONFIG_RTK_KDRV_OTP) += otp/

# RNG
obj-$(CONFIG_RTK_KDRV_RNG) += rng/

# MEASURE
obj-$(CONFIG_RTK_MEASURE) += measure/

# DSCD
obj-$(CONFIG_RTK_DSCD) += dscd/

#USBDUMP
ifneq ($(CHIP_NAME),rtd2851a)
obj-$(CONFIG_RTK_USBDUMP_ENABLE) += misc/rtk_usbdump.o
else
obj-$(CONFIG_RTK_USBDUMP_ENABLE) += misc/$(CHIP_NAME)/rtk_usbdump.o
endif

#LZMA
obj-$(CONFIG_CRYPTO_RTK_LZMA) += misc/rtk_zacc.o
# MD
obj-$(CONFIG_RTK_KDRV_MD) += md/

# RTC
obj-$(CONFIG_RTK_KDRV_RTC) += rtc/$(CHIP_NAME)/

# Watch Dog
ifneq ($(CHIP_NAME),rtd2851a)
obj-$(CONFIG_RTK_KDRV_WATCHDOG) += misc/rtk_watchdog.o
else
obj-$(CONFIG_RTK_KDRV_WATCHDOG) += misc/$(CHIP_NAME)/rtk_watchdog.o
endif

# Thermal Sensor
ifneq ($(CHIP_NAME),rtd2851a)
obj-$(CONFIG_RTK_KDRV_THERMAL_SENSOR) += thermal/
else
obj-$(CONFIG_RTK_KDRV_THERMAL_SENSOR) += thermal/$(CHIP_NAME)/
endif

# DFT phase scan
obj-$(CONFIG_RTK_KDRV_DFT) += dft/$(CHIP_NAME)/

# power/battery simulator
obj-$(CONFIG_RTK_POWER) += power/

# CPU Freq
obj-$(CONFIG_RTK_KDRV_CPU_FREQUENCY) += rtk-cpufreq.o
rtk-cpufreq-objs := misc/$(CHIP_NAME)/rtk_cpufreq.o misc/$(CHIP_NAME)/rtk_clock.o

# QoS
obj-$(CONFIG_RTK_KDRV_QOS_CONTROL) += rtk-qos.o
rtk-qos-objs := misc/$(CHIP_NAME)/rtk_qos.o

# HW Watchpoint
obj-$(CONFIG_RTK_KDRV_WATCHPOINT) += misc/$(CHIP_NAME)/rtk_venus_watch.o

# Frame Buffer
obj-$(CONFIG_RTK_KDRV_VENUS_FB) += misc/$(CHIP_NAME)/rtk_venusfb.o

ifeq ($(CONFIG_FB),m)
obj-$(CONFIG_RTK_KDRV_FB_NOTIFY) += misc/$(CHIP_NAME)/rtk_fb_notify.o
endif

# LSADC
obj-$(CONFIG_RTK_KDRV_LSADC) += lsadc/


# Keypad Driver
obj-$(CONFIG_RTK_KDRV_KEYPAD) += keypad/


# UART
obj-$(CONFIG_RTK_KDRV_SERIAL_8250) += uart/

# RTICE Driver
obj-$(CONFIG_RTK_KDRV_RTICE) += rtice/

# IRDA Driver
obj-$(CONFIG_RTK_KDRV_IR) += ir/

# dccti driver
obj-$(CONFIG_RTK_DCCTI_DRV) += dccti/


# SCALER
obj-$(CONFIG_SUPPORT_SCALER) += tvscalercontrol/$(CHIP_NAME)/

# HDCP
obj-$(CONFIG_RTK_HDCP_OPTEE) += hdcp/

# from now on, only 2851a(mac7p) use legacy makefile structure
ifneq ($(CONFIG_ARCH_RTK2851A),y)
# VGIP ISR Driver (moved to tvscalercontrol/Makefile)
# DDOMAIN ISR Driver(moved to tvscalercontrol/Makefile)
# PIF ISR Driver(moved to tvscalercontrol/Makefile)
# MEMC ISR Driver(moved to tvscalercontrol/Makefile)
# HDMI RX(moved to tvscalercontrol/Makefile)
else
# DV
obj-$(CONFIG_RTK_KDRV_DV) += dolby_idk_1_6_1_1/

# VGIP ISR Driver
ifneq ($(CONFIG_ARCH_RTK2851A),y)
obj-$(CONFIG_SUPPORT_SCALER) += misc/$(CHIP_NAME)/rtk_vgip_isr.o
endif
obj-$(CONFIG_SUPPORT_SCALER) += vgip_isr/$(CHIP_NAME)/

# DDOMAIN ISR Driver
obj-$(CONFIG_SUPPORT_SCALER) += misc/$(CHIP_NAME)/rtk_ddomain_isr.o

# PIF ISR Driver
ifneq ($(CONFIG_ARCH_RTK2851A),y)
#obj-$(CONFIG_SUPPORT_SCALER) += misc/$(CHIP_NAME)/rtk_pif_isr.o
endif

# MEMC ISR Driver
obj-$(CONFIG_SUPPORT_SCALER) += misc/$(CHIP_NAME)/rtk_memc_isr.o
#obj-y += misc/$(CHIP_NAME)/rtk_memc_isr.o
ifneq ($(CONFIG_BYPASSMEMC_FOR_CODESIZE_SAVING),y)
obj-$(CONFIG_SUPPORT_SCALER) += memc_isr/$(CHIP_NAME)/
endif

# HDMI RX
obj-$(CONFIG_RTK_HDMI_RX) += tvscalercontrol/$(CHIP_NAME)/hdmirx/

endif

# DP RX
obj-$(CONFIG_RTK_DPRX) += dprx/

# LG BOARD
obj-$(CONFIG_RTK_KDRV_LG_BOARD) += misc/$(CHIP_NAME)/rtk_lg_board.o


# LNB Uart patch
#obj-$(CONFIG_RTK_KDRV_LNB_UART) += lnbuart/

# Pinmux Driver
obj-$(CONFIG_RTK_KDRV_PINMUX) += rtk-pinmux.o
ifneq ($(CHIP_NAME),rtd2851a)
rtk-pinmux-objs := misc/$(CHIP_NAME)/rtk_pinmux.o misc/$(CHIP_NAME)/pinmux_internal.o
else
rtk-pinmux-objs := misc/$(CHIP_NAME)/rtk_pinmux.o
endif

# GAL Driver
obj-$(CONFIG_RTK_KDRV_SE) += gal/$(CHIP_NAME)/
obj-$(CONFIG_RTK_KDRV_GDMA) += gal/$(CHIP_NAME)/
obj-$(CONFIG_CUSTOMER_TV006) += gal/$(CHIP_NAME)/
obj-$(CONFIG_RTK_KDRV_REFDECMP) += gal/$(CHIP_NAME)/

# rtk drm
obj-$(CONFIG_DRM_REALTEK) += rtk_drm/
# Misc Driver
obj-$(CONFIG_RTK_KDRV_MISC_DEV) += misc/$(CHIP_NAME)/misc.o

# MMC
ifeq ($(filter y, $(CONFIG_ARCH_RTK2851A) $(CONFIG_ARCH_RTK2851C) $(CONFIG_ARCH_RTK2851F) $(CONFIG_ARCH_RTK2885P)),y)
obj-$(CONFIG_MMC_RTKEMMC_PLUS) += mmc/$(CHIP_NAME)/
else
obj-$(CONFIG_MMC_RTKEMMC_PLUS) += mmc/rtd67xx/
endif

# I2C Driver
obj-$(CONFIG_RTK_KDRV_I2C) += i2c/

# AMP init driver
# Please notice that this is only for REALTEK_AMP_TAS5711 & REALTEK_AMP_ACL1310
# If use different IC, please change this.
obj-$(CONFIG_RTK_KDRV_AMP) += amp/

# Audio HW Driver
obj-$(CONFIG_RTK_KDRV_AUDIO_HW) += audio_hw/$(CHIP_NAME)/
# Audio HW EARC Driver
obj-$(CONFIG_RTK_KDRV_AUDIO_HW_EARC) += audio_hw/$(CHIP_NAME)/

# ETN
obj-$(CONFIG_RTK_KDRV_R8168) += etn/


# DDR DM scan
obj-$(CONFIG_RTK_DRV_DDR_SCAN) += rtk-scan.o
rtk-scan-objs := misc/$(CHIP_NAME)/rtk_scan.o

# USB host controller
obj-y += usb/

# Quick-Show Driver
#obj-$(CONFIG_RTK_KDRV_QUICKSHOW) += quick_show/

# Bluetooth Driver
obj-$(CONFIG_BT_RTKBTUSB) += bluetooth/

###################################################################
# DTV Related drivers
###################################################################

#DEMOD Kernel driver
obj-$(CONFIG_RTK_KDRV_DEMOD) += demod/$(CHIP_NAME)/
obj-$(CONFIG_RTK_KDRV_KHAL_DEMOD) += demod/$(CHIP_NAME)/

#TP Kernel driver
obj-$(CONFIG_RTK_KDRV_TP) += tp/

#ATSC3 DMX
obj-$(CONFIG_RTK_KDRV_ATSC3_DMX) += atsc3_dmx/
obj-$(CONFIG_RTK_KDRV_DELIVERY) += delivery/

#JAPAN4k DMX
obj-$(CONFIG_RTK_KDRV_JP4K) += japan4k_dmx/

#ATSC3_ALP DMX
obj-$(CONFIG_RTK_KDRV_ATSC3_DMX) += atsc3_alp_dmx/
###################################################################
# HAL Related drivers
###################################################################

# CRM
obj-$(CONFIG_RTK_KDRV_CRM) += crm/

# OMX
obj-$(CONFIG_RTK_KDRV_OMX) += omx/

# Demux HAL Driver
ifeq (y,$(filter y, $(CONFIG_CUSTOMER_TV006)))
obj-$(CONFIG_RTK_KDRV_DEMUX) += demux
else
obj-$(CONFIG_RTK_KDRV_DEMUX) += demux_tv001/
endif

obj-$(CONFIG_RTK_KDRV_AUDIO) += audio/
obj-$(CONFIG_RTK_KDRV_VDEC) += vdec/$(CHIP_NAME)/

# SPI
obj-$(CONFIG_RTK_KDRV_SPI) += spi/$(CHIP_NAME)/

obj-$(CONFIG_RTK_AI_DRV) += rtkAiDrv/

# SPI_MASTER_GENERAL
obj-$(CONFIG_RTK_KDRV_SPI_MASTER_GENERAL) += spi_dev/$(CHIP_NAME)/

#obj-y += rtkAiDrv/
KBUILD_CFLAGS += -Werror

obj-y += rtk_dmic/

# IRTX
obj-$(CONFIG_RTK_KDRV_IRTX) += irtx/

# VAD
obj-$(CONFIG_RTK_KDRV_VAD) += vad/$(CHIP_NAME)/

# VAD
obj-$(CONFIG_RTK_KDRV_SVAD) += svad/$(CHIP_NAME)/

#pcie
ifeq ($(filter y, $(CONFIG_ARCH_RTK6702) $(CONFIG_ARCH_RTK2885P)),y)
obj-$(CONFIG_RTK_KDRV_PCIE) += pcie/$(CHIP_NAME)/
endif

#gadget
obj-$(CONFIG_RTK_KDRV_USB_GADGET) += gadget/

ifdef CONFIG_CFG80211
ifdef CONFIG_RTK_KDRV_RTLWIFI
rtlwifi_path = $(shell pwd)/drivers/rtk_kdriver/rtlwifi
rtlwifi_exist = $(wildcard $(rtlwifi_path))
ifneq ($(rtlwifi_exist),)
obj-$(CONFIG_RTL8822EU)		+= rtlwifi/rtl8822eu/
endif
endif
endif

obj-y+= rtk_typec/
