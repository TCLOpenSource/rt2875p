#
# Makefile for the linux kernel.
#
KBUILD_CFLAGS += -Werror

ifdef CONFIG_RTK_KDRV_MIO
obj-$(CONFIG_RTK_KDRV_PCMCIA) += rtk-pcmcia.o
rtk-pcmcia-objs := rtk_pcmcia_mio.o rtk_pcmcia_attr.o
else
obj-$(CONFIG_RTK_KDRV_PCMCIA) += rtk-pcmcia.o
rtk-pcmcia-objs := rtk_pcmcia.o rtk_pcmcia_attr.o
endif

