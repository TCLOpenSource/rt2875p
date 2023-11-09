ifneq ($(CONFIG_DRM),)
drm_realtek-y := drm_realtek_crtc.o \
		  drm_realtek_connector.o \
		  drm_realtek_debugfs.o \
		  drm_realtek_drv.o \
		  drm_realtek_gem.o \
		  drm_realtek_modeset.o \
		  drm_realtek_plane.o \
		  drm_realtek_encoder.o \
		  realtek_gdma_adapter.o 

drm_realtek-y += drm_realtek_fb.o

obj-$(CONFIG_DRM_REALTEK) += drm_realtek.o
endif
