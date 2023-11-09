#
# Makefile for the linux kernel.
#
KBUILD_CFLAGS += -Werror 
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/pwm/$(CHIP_NAME)/
obj-$(CONFIG_RTK_KDRV_PWM) += rtk-pwm.o
rtk-pwm-objs := rtk_pwm_crt.o rtk_pwm_local_dimming.o rtk_pwm_reg.o rtk_pwm_func.o rtk_pwm_attr.o rtk_pwm.o rtk_pwm_monitor.o rtk_pwm_rtice.o
