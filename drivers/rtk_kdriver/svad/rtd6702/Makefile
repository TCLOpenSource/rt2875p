KBUILD_CFLAGS += -Werror

KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/include
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/IPC/generate/include/system
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/IPC/include

obj-$(CONFIG_RTK_KDRV_SVAD) += rtk-svad.o
rtk-svad-objs := rtk_svad_drv.o rtk_svad_api.o rtk_svad_model_g.o rtk_svad_model_l.o 

KBUILD_CFLAGS += $(DEFS)