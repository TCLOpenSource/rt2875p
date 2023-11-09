#
# Makefile for the Realtek mcp drivers.
#
KBUILD_CPPFLAGS += -I ./
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/mcp/$(CHIP_NAME)
KBUILD_CFLAGS += -Wall -Werror
obj-$(CONFIG_RTK_KDRV_MCP) += rtk-mcp.o

rtk-mcp-objs := rtk_mcp.o  rtk_mcp_dev.o rtk_mcp_test.o
rtk-mcp-objs += rcp/rtk_rcp.o rcp/rtk_rcp_dev.o rcp/rtk_rcp_test.o

ifeq ($(CHIP_NAME),$(filter $(CHIP_NAME),rtd2851c rtd2851f))
rtk-mcp-objs += $(CHIP_NAME)/sha2-ce-glue.o

ifeq ($(shell expr $(VERSION) \>= 5), 1)
ifeq ($(shell expr $(PATCHLEVEL) \>= 15), 1)
rtk-mcp-objs += $(CHIP_NAME)/sha2-ce-core_5_15.o	
else
rtk-mcp-objs += $(CHIP_NAME)/sha2-ce-core_5_4.o	
endif
else
rtk-mcp-objs += $(CHIP_NAME)/sha2-ce-core_5_4.o	
endif

endif

ifeq ($(CONFIG_RTK_KDRV_MCP_NAPI),y)
rtk-mcp-objs += napi/mcp_cipher.o \
        napi/mcp_dbg.o \
        napi/mcp_aux.o \
        napi/mcp_dgst.o napi/mcp_dgst_dev.o napi/aes_h.o napi/sha1.o napi/mcp_buff.o\
        napi/mcp_cipher.o napi/mcp_cipher_dev.o napi/aes.o napi/des.o napi/tdes.o \
        napi/camellia.o \


ifdef CONFIG_RTK_KDRV_MCP_SHA256
rtk-mcp-objs += napi/sha256.o
endif
endif

ifeq ($(CONFIG_RTK_KDRV_MCP_RSA),y)
ifdef CONFIG_RTK_KDRV_MCP_RSA
rtk-mcp-objs += rsa/rtk_rsa_api.o rsa/rtk_rsa_lib.o \
			rsa/rtk_rsa_util.o rsa/rtk_rsa_hw_fun.o
endif

ifdef CONFIG_RTK_KDRV_MCP_RSA_TEST
rtk-mcp-objs += rsa/rtk_rsa_test.o
endif
endif
