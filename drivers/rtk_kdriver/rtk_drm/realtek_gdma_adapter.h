#if !defined(__RTK_GDMA_ADAPTER_H__)
#define __RTK_GDMA_ADAPTER_H__

#include <linux/device.h>
#include <linux/types.h>
#include "rtk_drm_fourcc.h"

#define GDMA_MAX_HEIGHT 2160
#define GDMA_MAX_WIDTH 3840

bool rtk_drm_gdma_clocks_set(struct device *dev,
			 u32 clock_freq, u32 dev_num,
			 u32 hdisplay, u32 vdisplay);

void rtk_drm_gdma_set_updates_enabled(struct device *dev, bool enable);

void rtk_drm_gdma_set_syncgen_enabled(struct device *dev, bool enable);

void rtk_drm_gdma_set_powerdwn_enabled(struct device *dev, bool enable);

void rtk_drm_gdma_set_vblank_enabled(struct device *dev, bool enable);

bool rtk_drm_gdma_check_and_clear_vblank(struct device *dev);

void rtk_drm_gdma_set_plane_enabled(struct device *dev, u32 plane, bool enable);

void rtk_drm_gdma_reset_planes(struct device *dev );

void rtk_drm_gdma_set_surface_atomic(struct device *dev,
			u32 plane, u32 address, u32 offset,
			int32_t crtcx, int32_t crtcy, u32 crtcw, u32 crtch,
			u32 srcx, u32 srcy, u32 srcw, u32 srch, u32 stride,
			u32 format, u64 modifier, u32 alpha, bool blend);

void rtk_drm_gdma_set_surface(struct device *dev, 
			  u32 plane, u32 address, u32 offset,
			  u32 posx, u32 posy,
			  u32 width, u32 height, u32 stride,
			  u32 format, u64 modifier, u32 alpha, bool blend);

void rtk_drm_gdma_mode_set(struct device *dev,
		       u32 h_display, u32 v_display,
		       u32 hbps, u32 ht, u32 has,
		       u32 hlbs, u32 hfps, u32 hrbs,
		       u32 vbps, u32 vt, u32 vas,
		       u32 vtbs, u32 vfps, u32 vbbs,
		       bool nhsync, bool nvsync);

void rtk_drm_gdma_atomic_begin(void);

void rtk_drm_gdma_atomic_flush(int plane_num);
void rtk_drm_get_screen_size(u32 * hdisplay, u32 * vdisplay);

#endif /* __RTK_GDMA_ADAPTER_H__*/
