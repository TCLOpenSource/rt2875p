KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/$(CHIP_NAME)/rtk_kdriver
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/$(CHIP_NAME)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/staging/android/ion
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/gal/$(CHIP_NAME)

KBUILD_CFLAGS += -Werror

# SE
ifeq ($(CONFIG_RTK_KDRV_SE),m)
obj-m += rtk-se.o
rtk-se-objs := rtk_se_driver.o rtk_se_lib.o rtk_kadp_se.o rtk_se_stch.o
else
obj-$(CONFIG_RTK_KDRV_SE) += rtk_se_driver.o rtk_se_lib.o rtk_kadp_se.o rtk_se_stch.o
endif

# GDMA Driver
ifeq ($(CONFIG_RTK_KDRV_GDMA),m)
obj-m += rtk-gdma.o
rtk-gdma-objs := rtk_gdma.o rtk_osd_sr_driver.o osd_dump_agent.o rtk_gdma_split.o
else
obj-$(CONFIG_RTK_KDRV_GDMA)     += rtk_gdma.o rtk_osd_sr_driver.o osd_dump_agent.o rtk_gdma_split.o
endif

ifeq ($(CONFIG_RTK_KDRV_REFDECMP),m)
obj-m += rtk-refdecmp.o
rtk-refdecmp-objs := rtk_refdecmp.o rtk_refdecmp_ver.o
else
obj-$(CONFIG_RTK_KDRV_REFDECMP)     += rtk_refdecmp.o rtk_refdecmp_ver.o
endif
# GAL Driver
obj-$(CONFIG_RTK_KDRV_GAL)    += rtk_kadp_gal_gfx.o rtk_kadp_gdma_hal_gal.o rtk_kadp_hal_gal_surface.o
