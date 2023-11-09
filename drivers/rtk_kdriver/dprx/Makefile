#
# Makefile for the Realtek DPRX drivers.
#
KBUILD_CFLAGS += -Wall -Werror
KBUILD_CFLAGS += -I drivers/rtk_kdriver/include/rtk_kdriver/dprx
KBUILD_CFLAGS += -I drivers/rtk_kdriver/dprx/
KBUILD_CFLAGS += -I drivers/rtk_kdriver/dprx/vfe/inc
KBUILD_CFLAGS += -I drivers/rtk_kdriver/dprx/osal/inc
KBUILD_CFLAGS += -I drivers/rtk_kdriver/dprx/util/inc
KBUILD_CFLAGS += -I drivers/rtk_kdriver/dprx/platform/inc
KBUILD_CFLAGS += -I drivers/rtk_kdriver/dprx/adapter/inc

#==================================
# VFE
#==================================
dprx_vfe-objs := $(SRC)/vfe/src/dprx_vfe_api.o
dprx_vfe-objs += $(SRC)/vfe/src/dprx_vfe.o
dprx_vfe-objs += $(SRC)/vfe/src/dprx_drvif.o
dprx_vfe-objs += $(SRC)/vfe/src/dprx_vfe-linux_module.o

ifdef CONFIG_RTK_DPRX_SYSFS
dprx_vfe-objs += $(SRC)/vfe/src/dprx_vfe-linux_sysfs.o
endif

ifdef CONFIG_DPRX_BUILD_IN_TEST
dprx_vfe-objs += $(SRC)/vfe/src/dprx_vfe-build_in_test.o
endif

#==================================
# Platform
#==================================
dprx_vfe-objs  += $(SRC)/platform/src/dprx_platform-rtk_tv.o

#==================================
# Util
#==================================
dprx_vfe-objs += $(SRC)/util/src/dprx_edid.o
dprx_vfe-objs += $(SRC)/util/src/dprx_edid-rtkddc.o
dprx_vfe-objs += $(SRC)/util/src/dprx_hpd.o
dprx_vfe-objs += $(SRC)/util/src/dprx_hpd-rtktv.o
dprx_vfe-objs += $(SRC)/util/src/dprx_hpd-typec.o

KBUILD_CFLAGS += -D CONFIG_RTK_HPD_RTKTV_SUPPORT_EVENT_HANDLER

ifdef CONFIG_DPRX_DRV_ENABLE_VIRTUAL_PLATFORM
dprx_vfe-objs += $(SRC)/util/src/dprx_hpd-dprx_virt_plat.o
endif

#==================================
# Adapter
#==================================
dprx_vfe-objs += $(SRC)/adapter/src/dprx_adapter.o
dprx_vfe-objs += $(SRC)/adapter/src/dprx_adapter-pseudo.o

############## RTK DPRX Adapter #############

ifdef CONFIG_DPRX_DRV_ENABLE_RTK_DPRX_ADAPTER

# DPRX DRV

ifdef CONFIG_BUILD_DPRX_DRV
include  $(obj)/Makefile.dprx_drv
else
include  $(obj)/Makefile.dprx_prebuild
endif

ifdef CONFIG_DPRX_VFE_ENABLE_QUICK_SHOW
KBUILD_CFLAGS += -D DPRX_GET_DEFAULT_EDID_FROM_DDC_SRAM
#KBUILD_CFLAGS += -D DEBUG_DEFAULT_EDID
endif

KBUILD_CFLAGS += -D CHECK_DPRX_OTP_SETTING
KBUILD_CFLAGS += -DENABLE_DPRX_LT_EVENT_MONITOR

dprx_vfe-objs += $(SRC)/adapter/src/dprx_adapter-rtk_dprx.o
dprx_vfe-objs += $(SRC)/adapter/src/dprx_adapter-rtk_dprx-priv.o
dprx_vfe-objs += $(SRC)/adapter/src/dprx_adapter-rtk_dprx-wrapper.o

ifdef CONFIG_RTK_DPRX_SYSFS
dprx_vfe-objs += $(SRC)/adapter/src/dprx_adapter-rtk_dprx-attr.o
endif

ifdef CONFIG_DPRX_DRV_ENABLE_VIRTUAL_PLATFORM
dprx_vfe-objs += $(SRC)/adapter/src/dprx_adapter-rtk_dprx-plat-virtual_platform.o
else
dprx_vfe-objs += $(SRC)/adapter/src/dprx_adapter-rtk_dprx-plat-linux.o
endif

endif

obj-$(CONFIG_RTK_DPRX) += dprx_vfe.o
