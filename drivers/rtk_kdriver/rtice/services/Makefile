obj-$(CONFIG_RTK_KDRV_RTICE_SYSTEM_TOOL) += rtk_ice-system.o
obj-$(CONFIG_RTK_KDRV_RTICE_IOMEM_TOOL) += rtk_ice-iomem.o
obj-$(CONFIG_RTK_KDRV_RTICE_AP_TOOL)   += rtk_ice-ap.o 
obj-$(CONFIG_RTK_KDRV_RTICE_MC_MEASURE) += rtk_ice_mc_measure_$(CHIP_NAME).o rtk_ice-latency-$(CHIP_NAME).o
obj-$(CONFIG_RTK_KDRV_RTICE_I2C_TOOL)   += rtk_ice-i2c.o

## this for 'esb' intruction support, used by rtk_ice-iommu.c
CFLAGS_rtk_ice-iomem.o = -mcpu=cortex-a55
