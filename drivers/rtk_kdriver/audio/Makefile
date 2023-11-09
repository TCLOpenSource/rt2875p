KBUILD_CFLAGS += -Werror

KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/audio
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/audio_hw/$(CHIP_NAME)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/include/rtk_kdriver/hal_inc
KBUILD_CPPFLAGS += -I$(srctree)/drivers/rtk_kdriver/demux_tv001

obj-$(CONFIG_RTK_KDRV_AUDIO) += rtkaudio.o
rtkaudio-objs := rtkaudio_driver.o rtkaudio_debug.o rtkaudio_dmabuf.o audio_rpc.o

rtkaudio-objs += khal_audio.o kadp_audio.o
rtkaudio-objs += audio_base.o audio_refclock.o audio_pin.o audio_util.o audio_common.o
rtkaudio-objs += audio_in.o audio_flow.o audio_dec.o audio_out.o audio_dtvcom.o
#rtkaudio-objs += resample_coef_system.o
#rtkaudio-objs += rtkcompress_driver.o

obj-$(CONFIG_SND_KDRV_REALTEK) += rtksnd.o
rtksnd-objs := alsa/rtk_snd.o
rtksnd-objs += alsa/rtk_snd_hwdep.o

KBUILD_CFLAGS += $(DEFS)
