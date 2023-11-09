# SPDX-License-Identifier: GPL-2.0

ifdef CONFIG_RTK_KDRIVER_SUPPORT

ifneq ($(CHIP_NAME),)
KBUILD_CPPFLAGS += -Idrivers/rtk_kdriver/include/$(CHIP_NAME)
KBUILD_CPPFLAGS += -Idrivers/rtk_kdriver/include/$(CHIP_NAME)/rtk_kdriver
KBUILD_CPPFLAGS += -Idrivers/rtk_kdriver/common_$(CHIP_NAME)/include/
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/common_$(CHIP_NAME)/IPC/generate/include/system
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/common_$(CHIP_NAME)/IPC/include
else
KBUILD_CPPFLAGS += -Idrivers/rtk_kdriver/include
KBUILD_CPPFLAGS += -Idrivers/rtk_kdriver/include/rtk_kdriver
KBUILD_CPPFLAGS += -Idrivers/rtk_kdriver/common/include/
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/common/IPC/generate/include/system
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/common/IPC/include
endif

endif

obj-$(CONFIG_ION_SYSTEM_HEAP) += ion_sys_heap.o
ion_sys_heap-y := ion_system_heap.o ion_page_pool.o

obj-$(CONFIG_ION_CMA_HEAP) += ion_cma_heap.o

