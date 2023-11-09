KBUILD_CFLAGS += -Werror

KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/include
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/IPC/generate/include/system
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/common/IPC/include

obj-y += rtk_svad_drv.o
obj-y += rtk_svad_api.o
obj-y += rtk_svad_model_g.o
obj-y += rtk_svad_model_l.o

KBUILD_CFLAGS += $(DEFS)