KBUILD_CFLAGS += -Werror
obj-$(CONFIG_RTK_AI_DRV) += rtk-ai.o
rtk-ai-objs := rtk_ai_device.o rtk_ai_model_control.o rtk_ai_audio.o
obj-$(CONFIG_RTK_AICAM_DRV) += rtk_aicam_device.o

ifeq (y,$(filter y, $(CONFIG_RTK_AI_OPTEE)))
rtk-ai-objs += nn_ta.o
endif
