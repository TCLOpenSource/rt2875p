KBUILD_CFLAGS += -Werror

KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/include
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/IPC/generate/include/system
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/IPC/include

obj-$(CONFIG_RTK_KDRV_VAD) += rtk-vad.o
rtk-vad-objs := rtk_vad_drv.o rtk_vad_api.o

KBUILD_CFLAGS += $(DEFS)