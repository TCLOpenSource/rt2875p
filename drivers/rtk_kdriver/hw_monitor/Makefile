#
# Makefile for the linux kernel.
#

obj-$(CONFIG_RTK_KDRV_HW_MONITOR) += rtk-hw_monitor.o
rtk-hw_monitor-objs := rtk_hw_monitor.o rtk_busto.o rtk_dc_simulator.o 

ifdef CONFIG_REALTEK_ARM_WRAPPER
rtk-hw_monitor-objs += arm_wrapper.o
endif

ifdef CONFIG_RTK_KDRV_SB2
rtk-hw_monitor-objs += rtk_sb2.o
endif

ifdef CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
rtk-hw_monitor-objs += rtk_dc_mt.o
endif

rtk-hw_monitor-objs += rtk_mem_prison.o

