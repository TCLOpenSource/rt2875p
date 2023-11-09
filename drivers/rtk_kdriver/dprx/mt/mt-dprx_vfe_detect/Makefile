#Set this to @ to keep the makefile quiet
ifndef SILENCE
	SILENCE = @
endif

#---- Outputs ----#
COMPONENT_NAME = mt-dprx_vfe

#--- Inputs ----#
CPP_PLATFORM = Gcc
PROJECT_HOME_DIR = .
CPPUTEST_USE_VPATH = Y
CPPUTEST_USE_EXTENSIONS = Y

#--- compile options ----#
CONFIG_DPRX_DRV_ENABLE_HDCP=n
CONFIG_DPRX_DRV_ENABLE_AUDIO=n
CONFIG_DPRX_DRV_ENABLE_PHY=y

# for production code config
CPPUTEST_CPPFLAGS += -D CONFIG_DPRX_PRJ_MAC8P_MT
CPPUTEST_CPPFLAGS += -D CONFIG_DPRX_DRV_ENABLE_VIRTUAL_PLATFORM
CPPUTEST_CPPFLAGS += -D PLATFORM_LINUX_USER
CPPUTEST_CPPFLAGS += -D CONFIG_DPRX_DRV_ENABLE_RTK_DPRX_ADAPTER
#CPPUTEST_CPPFLAGS += -D CONFIG_DPRX_DRV_USE_EXTERNAL_EXT_DRVIF

#CPPUTEST_CPPFLAGS += -D DEBUG_VIRTUAL_AUX_DPCD_RW
#CPPUTEST_CPPFLAGS += -D DEBUG_VIRTUAL_DPCD
#CPPUTEST_CPPFLAGS += -D DPRX_COMPACT_DEBUG_MODE
#CPPUTEST_CPPFLAGS += -D DPRX_DEBUG_LINK_TRAINING
CPPUTEST_CPPFLAGS += -D DPRX_DEBUG_DETECT_MODE

# --- SRC_FILES and SRC_DIRS ---
# Production code files are compiled and put into
# a library to link with the test runner.
#
# Test code of the same name overrides
# production code at link time.
#
# SRC_FILES specifies individual production
# code files.
#
# SRC_DIRS specifies directories containing
# production code C and CPP files.
#
# SRC_DIRS =

UT_ROOT  = ../
SUT_ROOT = $(UT_ROOT)/../
SUT_DIR  = $(UT_ROOT)/../
KDRV_INC_PATH = $(SUT_DIR)/../include/rtk_kdriver/dprx

#####################################
# DPRX VFE library
#####################################
SRC_FILES += $(SUT_DIR)/vfe/src/dprx_vfe_api.c
SRC_FILES += $(SUT_DIR)/vfe/src/dprx_vfe.c
SRC_FILES += $(SUT_DIR)/adapter/src/dprx_adapter-rtk_dprx.c
SRC_FILES += $(SUT_DIR)/adapter/src/dprx_adapter-rtk_dprx-priv.c
SRC_FILES += $(SUT_DIR)/adapter/src/dprx_adapter-rtk_dprx-wrapper.c
SRC_FILES += $(SUT_DIR)/adapter/src/dprx_adapter-rtk_dprx-plat-virtual_platform.c
SRC_FILES += $(SUT_DIR)/adapter/src/dprx_adapter-pseudo.c
SRC_FILES += $(SUT_DIR)/adapter/src/dprx_adapter.c
SRC_FILES += $(SUT_DIR)/platform/src/dprx_platform-rtk_tv.c
SRC_FILES += $(SUT_DIR)/util/src/dprx_edid.c
SRC_FILES += $(SUT_DIR)/util/src/dprx_hpd.c
SRC_FILES += $(SUT_DIR)/util/src/dprx_hpd-dprx_virt_plat.c

#####################################

#####################################
SRC_FILES += $(SUT_DIR)/driver/api/src/ScalerDpRx.c
SRC_FILES += $(SUT_DIR)/driver/api/src/ScalerDpRx_EXINT0.c
SRC_FILES += $(SUT_DIR)/driver/api/src/ScalerDpRx_WDINT.c
SRC_FILES += $(SUT_DIR)/driver/api/src/dprx_api.c

#####################################
SRC_FILES += $(SUT_DIR)/driver/aux_channel/src/dprx_aux.c
SRC_FILES += $(SUT_DIR)/driver/aux_channel/src/dprx_aux_mac.c
SRC_FILES += $(SUT_DIR)/driver/aux_channel/src/dprx_aux_phy.c
SRC_FILES += $(SUT_DIR)/driver/aux_channel/src/dprx_aux_dpcd.c
SRC_FILES += $(SUT_DIR)/driver/aux_channel/src/dprx_aux_hdcp.c
SRC_FILES += $(SUT_DIR)/driver/aux_channel/src/dprx_aux_link_train.c

SRC_FILES += $(SUT_DIR)/driver/aux_channel/src/dprx_aux-exint0.c
SRC_FILES += $(SUT_DIR)/driver/aux_channel/src/dprx_aux_mac-exint0.c
SRC_FILES += $(SUT_DIR)/driver/aux_channel/src/dprx_aux_dpcd-exint0.c
SRC_FILES += $(SUT_DIR)/driver/aux_channel/src/dprx_aux_hdcp-exint0.c
SRC_FILES += $(SUT_DIR)/driver/aux_channel/src/dprx_aux_test_sink-exint0.c
SRC_FILES += $(SUT_DIR)/driver/aux_channel/src/dprx_aux_link_train-exint0.c

#####################################

SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_mac.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_link.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_signal_detect.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_fec.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_vbid_msa.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_sdp.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_video.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_video_pll.c

SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_measure.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_hdcp.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_freesync.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_mac_exint0.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_link-exint0.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_fec-exint0.c

SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_hdcp-exint0.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_mac-wdint.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_hdcp-wdint.c

ifeq ($(CONFIG_DPRX_DRV_ENABLE_AUDIO), y)

CPPUTEST_CPPFLAGS += -D CONFIG_DPRX_DRV_ENABLE_AUDIO

SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_audio.c
SRC_FILES += $(SUT_DIR)/driver/mac/src/dprx_audio-exint0.c

endif

#####################################

ifeq ($(CONFIG_DPRX_DRV_ENABLE_PHY), y)

CPPUTEST_CPPFLAGS += -D CONFIG_DPRX_DRV_ENABLE_PHY

SRC_FILES += $(SUT_DIR)/driver/phy/src/dprx_phy.c
SRC_FILES += $(SUT_DIR)/driver/phy/src/dprx_phy-exint0.c
SRC_FILES += $(SUT_DIR)/driver/phy/src/dprx_phy-wdint.c
SRC_FILES += $(SUT_DIR)/driver/phy/src/dprx_phy_p0.c

endif

#####################################

ifeq ($(CONFIG_DPRX_DRV_ENABLE_HDCP), y)

CPPUTEST_CPPFLAGS += -D CONFIG_DPRX_DRV_ENABLE_HDCP

SRC_FILES += $(SUT_DIR)/driver/hdcp/src/dprx_hdcp_rx.c
SRC_FILES += $(SUT_DIR)/driver/hdcp/src/dprx_hdcp_rx-exint0.c

SRC_FILES += $(SUT_DIR)/driver/hdcp/src/dprx_hdcp2_rx.c
SRC_FILES += $(SUT_DIR)/driver/hdcp/src/dprx_hdcp2_rx-exint0.c

SRC_FILES += $(SUT_DIR)/driver/hdcp/src/hdcp2_cipher.c
SRC_FILES += $(SUT_DIR)/driver/hdcp/src/hdcp2_cipher-exint0.c

endif

SRC_FILES += $(SUT_DIR)/driver/virtual_platform/src/virtual_platform.c

# --- TEST_SRC_FILES and TEST_SRC_DIRS ---
# Test files are always included in the build.
# Production code is pulled into the build unless
# it is overriden by code of the same name in the
# test code.
#
# TEST_SRC_FILES specifies individual test files to build.
# TEST_SRC_DIRS, builds everything in the directory
#TEST_SRC_DIRS = \
#	.\

# TEST_SRC_FILE LIST START (for auto gen control)
TEST_SRC_FILES += $(UT_ROOT)/mock/pcbMgr_fff.c
TEST_SRC_FILES += $(UT_ROOT)/mock/dprx_pcb_config_fff.c
TEST_SRC_FILES += TEST_dprx_vfe_detect.cpp
TEST_SRC_FILES += AllTests.cpp


# INCLUDE_DIRS are searched in order after the included file's
# containing directory
INCLUDE_DIRS += $(CPPUTEST_HOME)/include
INCLUDE_DIRS += $(UT_ROOT)/fff/
INCLUDE_DIRS += $(UT_ROOT)/mock/
INCLUDE_DIRS += $(UT_ROOT)/mock/include

#----------------------------------------------
# DPRX VFE
#----------------------------------------------
INCLUDE_DIRS += $(KDRV_INC_PATH)
INCLUDE_DIRS += $(SUT_DIR)/vfe/inc
INCLUDE_DIRS += $(SUT_DIR)/osal/inc
INCLUDE_DIRS += $(SUT_DIR)/util/inc
INCLUDE_DIRS += $(SUT_DIR)/platform/inc
INCLUDE_DIRS += $(SUT_DIR)/adapter/inc

#----------------------------------------------
# DPRX CUT
#----------------------------------------------
INCLUDE_DIRS += $(SUT_DIR)/driver/
INCLUDE_DIRS += $(SUT_DIR)/driver/api/inc/
INCLUDE_DIRS += $(SUT_DIR)/driver/aux_channel/inc/
INCLUDE_DIRS += $(SUT_DIR)/driver/virtual_platform/inc/
INCLUDE_DIRS += $(SUT_DIR)/driver/mac/inc/
INCLUDE_DIRS += $(SUT_DIR)/driver/phy/inc/
INCLUDE_DIRS += $(SUT_DIR)/driver/hdcp/inc/
INCLUDE_DIRS += $(SUT_DIR)/driver/include
INCLUDE_DIRS += $(SUT_DIR)/driver/util/hdcp14/

# MOCKS_SRC_DIRS = \
# 	../mock\

CPPUTEST_OBJS_DIR = test-obj
CPPUTEST_LIB_DIR = test-lib

# You may have to tweak these compiler flags
#    CPPUTEST_WARNINGFLAGS - apply to C and C++
#    CPPUTEST_CFLAGS - apply to C files only
#    CPPUTEST_CXXFLAGS - apply to C++ files only
#    CPPUTEST_CPPFLAGS - apply to C and C++ Pre-Processor
#
# If you get an error like this
#     TestPlugin.h:93:59: error: 'override' keyword is incompatible
#        with C++98 [-Werror,-Wc++98-compat] ...
# The compiler is basically telling you how to fix the
# build problem.  You would add this flag setting
#     CPPUTEST_CXXFLAGS += -Wno-c++14-compat

# Some flags to quiet clang
ifeq ($(shell $(CC) -v 2>&1 | grep -c "clang"), 1)
CPPUTEST_WARNINGFLAGS += -Wno-unknown-warning-option
CPPUTEST_WARNINGFLAGS += -Wno-covered-switch-default
CPPUTEST_WARNINGFLAGS += -Wno-reserved-id-macro
CPPUTEST_WARNINGFLAGS += -Wno-keyword-macro
CPPUTEST_WARNINGFLAGS += -Wno-documentation
CPPUTEST_WARNINGFLAGS += -Wno-missing-noreturn
endif

#CPPUTEST_WARNINGFLAGS += -Wall
CPPUTEST_WARNINGFLAGS += -Werror
#CPPUTEST_WARNINGFLAGS += -Wfatal-errors
#CPPUTEST_WARNINGFLAGS += -Wswitch-enum
CPPUTEST_WARNINGFLAGS += -Wswitch-default
CPPUTEST_WARNINGFLAGS += -Wno-format-nonliteral
CPPUTEST_WARNINGFLAGS += -Wno-sign-conversion
CPPUTEST_WARNINGFLAGS += -Wno-pedantic
CPPUTEST_WARNINGFLAGS += -Wno-shadow
CPPUTEST_WARNINGFLAGS += -Wno-missing-field-initializers
#CPPUTEST_WARNINGFLAGS += -Wno-unused-parameter
CPPUTEST_CFLAGS += -pedantic
CPPUTEST_CFLAGS += -Wno-missing-prototypes
CPPUTEST_CFLAGS += -Wno-strict-prototypes
CPPUTEST_CXXFLAGS += -Wno-c++14-compat
CPPUTEST_CXXFLAGS += --std=c++11
CPPUTEST_CXXFLAGS += -Wno-c++98-compat-pedantic
CPPUTEST_CXXFLAGS += -Wno-c++98-compat

# --- LD_LIBRARIES -- Additional needed libraries can be added here.
# commented out example specifies math library
#LD_LIBRARIES += -lm
# LD_LIBRARIES = -lpthread

# Look at $(CPPUTEST_HOME)/build/MakefileWorker.mk for more controls
ifeq ($(CPPUTEST_HOME),)
$(info CPPUTEST_HOME not set! See README.txt)
else
include $(CPPUTEST_HOME)/build/MakefileWorker.mk
endif
