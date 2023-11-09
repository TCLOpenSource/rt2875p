KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/include
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/include/rtk_kdriver
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/common/include
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/common/include/rbus
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/platform/tv002/pq_ctrl_tv002/include
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/platform/tv002/pq_ctrl_tv002
KBUILD_CPPFLAGS += -I drivers/rtk_kdriver/platform/tv002

KBUILD_CPPFLAGS += -DLITTLE_ENDIAN
KBUILD_CFLAGS += -fno-unsigned-char

mkfile_path:=$(shell pwd)/$(lastword $(MAKEFILE_LIST))
mkfile_path:=$(shell dirname $(mkfile_path))

SOPQ_LIB_EXT_API_SRC:=drivers/rtk_kdriver/platform/tv002/pq_ctrl_tv002/sopq_lib_ext_api.c
SOPQ_LIB_EXT_API_OBJ:=library/pq_lib_tv002.a

ifneq ("", "$(wildcard $(SOPQ_LIB_EXT_API_SRC))")
    SOPQ_LIB_EXT_API_OBJ:=sopq_lib_ext_api.o
endif

PRJ_SYSTEM_PATH := $(mkfile_path)/../../../../../../../system
KDIR := $(PRJ_SYSTEM_PATH)/../linux/linux-4.14
CROSS_COMPILE_PATH := $(PRJ_SYSTEM_PATH)/tmp/toolchain/asdk-6.4.1-a55-EL-4.4-g2.26-a32nut-170810/bin
CROSS_COMPILE := $(CROSS_COMPILE_PATH)/arm-linux-

obj-m += pq_ctrl_tv002_mod.o
pq_ctrl_tv002_mod-objs := pq_ctrl_tv002.o $(SOPQ_LIB_EXT_API_OBJ)

all:
	make -C $(KDIR) M=$(mkfile_path) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) modules V=1
	@#$(KDIR)/scripts/sign-file "sha256" "$(KDIR)/certs/rtk_signing_key.pem" "$(KDIR)/certs/signing_key.x509" pq_ctrl_tv002_mod.ko
clean:
	make -C $(KDIR) M=$(mkfile_path) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) clean V=1

lib:
	@if [ -f "sopq_lib_ext_api.o" ]; then \
        $(CROSS_COMPILE)ar -rcv pq_lib_tv002.a sopq_lib_ext_api.o; cp pq_lib_tv002.a pq_lib_tv002.a_shipped; \
        else echo "error: can't find sopq_lib_ext_api.o, please make source file first."; \
        fi;

