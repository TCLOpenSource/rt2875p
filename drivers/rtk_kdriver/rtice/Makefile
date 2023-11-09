#obj-y    += core/
#obj-y    += services/
obj-$(CONFIG_RTK_KDRV_RTICE)    += rtice2.o
rtice2-objs += ./core/rtk_ice-aux.o ./core/rtk_ice-cmd.o ./core/rtk_ice-in-uart.o ./core/rtk_ice.o

ifdef CONFIG_RTK_KDRV_RTICE_SYSTEM_TOOL
rtice2-objs += ./services/rtk_ice-system.o
endif
ifdef CONFIG_RTK_KDRV_RTICE_IOMEM_TOOL
rtice2-objs += ./services/rtk_ice-iomem.o
endif

ifdef CONFIG_RTK_KDRV_RTICE_I2C_TOOL
rtice2-objs += ./services/rtk_ice-i2c.o
endif
ifdef CONFIG_RTK_KDRV_RTICE_AP_TOOL
rtice2-objs += ./services/rtk_ice-ap.o
endif
ifdef CONFIG_RTK_KDRV_RTICE_MC_MEASURE
rtice2-objs += ./services/rtk_ice_mc_measure_$(CHIP_NAME).o ./services/rtk_ice-latency-$(CHIP_NAME).o
endif
rtice2-objs += ./core/rtk_ice-in-dev.o
