/*
    this file is used to record tag name of each module
    it can also be used to generate the print api of each module
*/
#ifndef __RTK_MODULE_TAG_H__
#define __RTK_MODULE_TAG_H__
#define TAG_NAME_DSCD             "DSCD"
#define TAG_NAME_MEASURE             "MEASURE"
#define TAG_NAME_QOS             "QOS"
#define TAG_NAME_RMM             "RMM"
#define TAG_NAME_SPI_DEV             "SPI_DEV"
#define TAG_NAME_SPI_NOR             "SPI_NOR"
#define TAG_NAME_RTK_TEE             "RTK_TEE"
#define TAG_NAME_RPC             "RPC"
#define TAG_NAME_CRT             "CRT"
#define TAG_NAME_PLATFORM             "PLATFORM"
#define TAG_NAME_POWER             "POWER"
#define TAG_NAME_MACH_RTK             "MACH_RTK"
#define TAG_NAME_HW_MONITOR             "HW_MONITOR"
#define TAG_NAME_DFT             "DFT"
#define TAG_NAME_VENUSFB             "VENUSFB"
#define TAG_NAME_GDECMP             "GDECMP"
#define TAG_NAME_UART             "UART"
#define TAG_NAME_RTICE             "RTICE"
#define TAG_NAME_SWITCH             "SWITCH"
#define TAG_NAME_PCB_MGR             "PCB_MGR"
#define TAG_NAME_OMX             "OMX"
#define TAG_NAME_CRM             "CRM"
#define TAG_NAME_RTC             "RTC"
#define TAG_NAME_VDEC             "VDEC"
#define TAG_NAME_USB_GADGET             "USB_GADGET"
#define TAG_NAME_ZQ             "ZQ"
#define TAG_NAME_DDR_SCAN             "DDR_SCAN"
#define TAG_NAME_CPU_FREQ             "CPU_FREQ"
#define TAG_NAME_OHCI_PLAT             "OHCI_PLAT"
#define TAG_NAME_RTK_USB             "RTK_USB"
#define TAG_NAME_OTG_PLAT             "OTG_PLAT"
#define TAG_NAME_XHCI_DRD             "XHCI_DRD"
#define TAG_NAME_XHCI_PLAT             "XHCI_PLAT"
#define TAG_NAME_EHCI_PLAT             "EHCI_PLAT"
#define TAG_NAME_AVD                "AVD"
#define TAG_NAME_ADC                "ADC"
#define TAG_NAME_VDC                "VDC"
#define TAG_NAME_VBI                "VBI"
#define TAG_NAME_ONMS               "ONMS"
#define TAG_NAME_OFFMS              "OFFMS"
#define TAG_NAME_MEMC               "MEMC"
#define TAG_NAME_MEMC_LIB           "MEMC_LIB"
#define TAG_NAME_NEW_GAME_MODE      "NEW_GAME_MODE"
#define TAG_NAME_SCALER_MEMORY      "SCALER_MEMORY"
#define TAG_NAME_VBE                "VBE"
#define TAG_NAME_VPQ                "VPQ"
#define TAG_NAME_VPQ_GSR            "VPQ_GSR"
#define TAG_NAME_HDMITX             "HDMITX"
#define TAG_NAME_VFE                "VFE"
#define TAG_NAME_VSC                "VSC"
#define TAG_NAME_I3DDMA             "I3DDMA"
#define TAG_NAME_IOREG              "IOREG"
#define TAG_NAME_SMT                "SMT"
#define TAG_NAME_HDR                "HDR"
#define TAG_NAME_VR                 "VR"
#define TAG_NAME_VT                 "VT"
#define TAG_NAME_VO                 "VO"
#define TAG_NAME_VGIP_ISR           "VGIP_ISR"
#define TAG_NAME_DDOMAIN_ISR        "DDOMAIN_ISR"
#define TAG_NAME_PIF_ISR            "PIF_ISR"
#define TAG_NAME_HIST               "HIST"
#define TAG_NAME_VPQ_AI             "VPQ_AI"
#define TAG_NAME_VPQEX              "VPQEX"
#define TAG_NAME_VPQUT              "VPQUT"
#define TAG_NAME_VPQMASK            "VPQMASK"
#define TAG_NAME_VIP_RINGBUF        "VIP_RINGBUF"
#define TAG_NAME_VPQMASK_COLOR      "VPQMASK_COLOR"
#define TAG_NAME_SRNN               "SRNN"
#define TAG_NAME_VPQ_OD             "VPQ_OD"
#define TAG_NAME_FCIC               "VPQ_FCIC"
#define TAG_NAME_VPQ_ISR            "VPQ_ISR"
#define TAG_NAME_AI_DBG             "AI_DBG"
#define TAG_NAME_RTK_TSK            "RTK_UTIL_TASKS"
#define TAG_NAME_SE                 "SE"
#define TAG_NAME_GDMA               "GDMA"
#define TAG_NAME_DPRX               "DPRX"
#define TAG_NAME_QUICK_SHOW         "QUICK_SHOW"

#define TAG_NAME_HDCP               "HDCP"
#define TAG_NAME_DELIVERY           "DELI"


#define TAG_NAME_SE_FILM            "SE_FILM"

// PQ
#define TAG_NAME_DPQ                "DPQ"
#define TAG_NAME_PQ                 "PQ"
#define TAG_NAME_VPQ_AI             "VPQ_AI"

// MMC/SDCARD/SPI
#define TAG_NAME_EMMC               "EMMC"
#define TAG_NAME_EVENTLOG               "EVENTLOG"
#define TAG_NAME_SDIO               "CARD"
#define TAG_NAME_SPI                "SPI"
#define TAG_NAME_SPI_MASTER         "rtk_spi_master"

// USB_BB/ USB_HUB / PCIE
#define TAG_NAME_USB_BB             "USB_BB"
#define TAG_NAME_USB_HUB            "USB_HUB"
#define TAG_NAME_PCIE               "PCIE"

// ETH
#define TAG_NAME_ETH		        "ETH"

// HDMI
#define TAG_NAME_HDMI               "HDMI"

// Audio
#define TAG_NAME_AMP                "AMP"
#define TAG_NAME_AUDIO_HW           "AudioHW"
#define TAG_NAME_ADSP               "ADSP"

// TP / DMX / ATSC3 / ATSC3_ALP / JAPAN4K / Delivery
#define TAG_NAME_TP                 "TP"
#define TAG_NAME_DEMUX              "DEMUX"
#define TAG_NAME_ATSC3              "ATSC3"
#define TAG_NAME_ATSC3ALP           "ATSC3ALP"
#define TAG_NAME_JAPAN4K            "JAPAN4K"

// DEMOD/LNB/DISEQC
#define TAG_NAME_DEMOD              "DEMOD"
#define TAG_NAME_DISEQC             "DISEQC"
#define TAG_NAME_LNB_UART           "LNBUART"
#define TAG_NAME_LNB_GPIO           "LNBGPIO"

// CP / RSA
#define TAG_NAME_MD                 "MD"
#define TAG_NAME_CW                 "CW"
#define TAG_NAME_MCP                "MCP"
#define TAG_NAME_OTP                "OTP"
#define TAG_NAME_RCP                "RCP"
#define TAG_NAME_RNG                "RNG"
#define TAG_NAME_RSA                "RSA"

// DCMT/HWMNT
#define TAG_NAME_ARM_WARP           "WRAP"
#define TAG_NAME_DCMT               "DCMT"
#define TAG_NAME_HWM                "HWM"
#define TAG_NAME_THERMAL            "THERMAL"
#define TAG_NAME_SEMA               "SEMAPHORE"

// MISC
#define TAG_NAME_CEC                "CEC"
#define TAG_NAME_DDC                "DDC"
#define TAG_NAME_DDC_CI             "DDC_CI"
#define TAG_NAME_EXTCON             "RTK_EXTCON"
#define TAG_NAME_GPIO               "GPIO"
#define TAG_NAME_KEYPAD             "KEYPAD"
#define TAG_NAME_LED                "LED"
#define TAG_NAME_LED_SERIAL         "LEDSERIAL"
#define TAG_NAME_LSADC              "LSADC"
#define TAG_NAME_I2C                "I2C"
#define TAG_NAME_IR			        "IR"
#define TAG_NAME_IRRC               "IRMD"
#define TAG_NAME_IRTX               "IRTX"
#define TAG_NAME_PCMCIA             "PCMCIA"
#define TAG_NAME_PWM                "PWM"
#define TAG_NAME_PWM_LD             "PWM_LD"
#define TAG_NAME_SCD                "SCD"

// EMCU
#define TAG_NAME_EMCU               "EMCU"

// SYSTEM MISC : RTDLOG
#define TAG_NAME_MISC               "MISC"
#define TAG_NAME_MISC_INTERRUPT     "MISC_DRIVER_INTERRUPT"
#define TAG_NAME_USB_DUMP           "USBDUMP"
#define TAG_NAME_RTDLOG             "RTDLOG"
#define TAG_NAME_ONLINE_HELP        "ONLINE_HELP"
#define TAG_NAME_LG_COUNTRY_TYPE    "LG_COUNTRY_TYPE"

#endif /*__RTK_MODULE_TAG_H__*/
