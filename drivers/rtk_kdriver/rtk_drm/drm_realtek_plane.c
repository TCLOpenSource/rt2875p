#include <linux/version.h>

#include "drm_realtek_drv.h"

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0))
#include <drm/drmP.h>
#endif

#include <drm/drm_plane_helper.h>

#if defined(RTK_DRM_USE_ATOMIC)
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0))
#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_drv.h>
#else
#include <drm/drm_gem_framebuffer_helper.h>
#endif
#endif

#include "drm_realtek_gem.h"
#include "realtek_gdma_adapter.h"
#include "kernel_compatibility.h"

const uint32_t gdma_formats[] = {
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_XBGR8888,	
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_RGBA8888,
	DRM_FORMAT_BGRA8888,
	DRM_FORMAT_RGB565,
};

const uint64_t gdma_format_modifiers[]={
 DRM_FORMAT_MOD_LINEAR,
 DRM_FORMAT_MOD_PVR_FBCDC_8x8_V13 ,
 DRM_FORMAT_MOD_PVR_FBCDC_LOSSY25_8x8_V13,
 DRM_FORMAT_MOD_PVR_FBCDC_LOSSY50_8x8_V13,
 DRM_FORMAT_MOD_PVR_FBCDC_LOSSY75_8x8_V13,
	
 DRM_FORMAT_MOD_PVR_FBCDC_16x4_V13,
 DRM_FORMAT_MOD_PVR_FBCDC_LOSSY25_16x4_V13,
 DRM_FORMAT_MOD_PVR_FBCDC_LOSSY50_16x4_V13,
 DRM_FORMAT_MOD_PVR_FBCDC_LOSSY75_16x4_V13,

 DRM_FORMAT_MOD_INVALID
};

#if defined(RTK_DRM_USE_ATOMIC)
static int rtk_drm_plane_helper_atomic_check(struct drm_plane *plane,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0))
					 struct drm_atomic_state *atomic_state)
#else
					 struct drm_plane_state *state)
#endif
{
	struct drm_crtc_state *crtc_new_state;
	int ret = 0;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0))
	struct drm_plane_state *state = drm_atomic_get_new_plane_state(atomic_state, plane);
#endif
	if (!state->crtc)
		return 0;

	crtc_new_state = drm_atomic_get_new_crtc_state(state->state,
						       state->crtc);

	if(plane->type == DRM_PLANE_TYPE_PRIMARY)
	{
		ret = drm_atomic_helper_check_plane_state(state, crtc_new_state,
						   DRM_PLANE_HELPER_NO_SCALING/2,
						   DRM_PLANE_HELPER_NO_SCALING,
						   false, true);
	}else{
		ret = drm_atomic_helper_check_plane_state(state, crtc_new_state,
						   DRM_PLANE_HELPER_NO_SCALING/2,
						   DRM_PLANE_HELPER_NO_SCALING,
						   true, true);
	}
	
	return ret;
}

static void rtk_drm_plane_helper_atomic_update(struct drm_plane *plane,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0))
						 struct drm_atomic_state *atomic_state)
#else
						 struct drm_plane_state *old_state)
#endif

{
	struct drm_plane_state *plane_state = plane->state;
	struct drm_framebuffer *fb = plane_state->fb;

	if (fb) {
		rtk_drm_plane_set_surface(plane_state->crtc, plane, fb,
				      plane_state->src_x, plane_state->src_y);
	}
}

static const struct drm_plane_helper_funcs rtk_drm_plane_helper_funcs = {
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0))
	.prepare_fb =  drm_gem_plane_helper_prepare_fb,
#else
	.prepare_fb =  drm_gem_fb_prepare_fb,
#endif
	.atomic_check = rtk_drm_plane_helper_atomic_check,
	.atomic_update = rtk_drm_plane_helper_atomic_update,

};

static const struct drm_plane_funcs rtk_drm_plane_funcs = {
	.update_plane = drm_atomic_helper_update_plane,
	.disable_plane = drm_atomic_helper_disable_plane,
	.destroy = rtk_drm_plane_destroy,
	.reset = drm_atomic_helper_plane_reset,
	.atomic_duplicate_state = drm_atomic_helper_plane_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_plane_destroy_state,
};
#else
#define rtk_drm_plane_funcs drm_primary_helper_funcs
#endif

struct drm_plane *rtk_drm_plane_create(struct drm_device *dev,
				   enum drm_plane_type type)
{
	struct rtk_drm_private *dev_priv = dev->dev_private;
	struct drm_plane *plane;
	const uint32_t *supported_formats;
	uint32_t num_supported_formats;
	uint32_t possible_crtcs = 0;
	int err;

	switch (dev_priv->version) {
	case RTK_DRM_VERSION_GDMA_ADAPTER:
		supported_formats = gdma_formats;
		num_supported_formats = ARRAY_SIZE(gdma_formats);
		break;
	default:
		DRM_ERROR("Unsupported RTK DRM version\n");
		err = -EINVAL;
		goto err_exit;
	}

	plane = kzalloc(sizeof(*plane), GFP_KERNEL);
	if (!plane) {
		err = -ENOMEM;
		goto err_exit;
	}

	if(dev_priv->crtc)
		possible_crtcs = drm_crtc_mask(dev_priv->crtc);
	err = drm_universal_plane_init(dev, plane, possible_crtcs, &rtk_drm_plane_funcs,
				       supported_formats,
				       num_supported_formats,
				       gdma_format_modifiers, type, NULL);
	if (err)
		goto err_plane_free;

#if defined(RTK_DRM_USE_ATOMIC)
	drm_plane_helper_add(plane, &rtk_drm_plane_helper_funcs);
#endif

	DRM_DEBUG_DRIVER("[PLANE:%d]\n", plane->base.id);

	return plane;

err_plane_free:
	kfree(plane);
err_exit:
	return ERR_PTR(err);
}

void rtk_drm_plane_destroy(struct drm_plane *plane)
{
   drm_plane_cleanup(plane);
   kfree(plane);
}

void rtk_drm_plane_set_surface(struct drm_crtc *crtc, struct drm_plane *plane,
			   struct drm_framebuffer *fb,
			   const uint32_t src_x, const uint32_t src_y)
{
	struct rtk_drm_private *dev_priv = plane->dev->dev_private;
	struct rtk_drm_framebuffer *rtk_fb = to_rtk_drm_framebuffer(fb);
	struct drm_plane_state *new_plane_state = plane->state;
	unsigned int pitch = fb->pitches[0];
	uint64_t address = rtk_drm_gem_get_phy_addr(rtk_fb->obj[0]);
	uint32_t format;
	uint64_t modifier = fb->modifier;

	format = rtk_drm_fb_format(fb);

	if(modifier != DRM_FORMAT_MOD_LINEAR && (src_x || src_y))
	{
		DRM_ERROR("No support for x, y offset when framebuffer is with modifier (0x%llu)! (%u,%u)\n",
			modifier, src_x, src_y);
		return;
	}

	/*
	 * NOTE: If the buffer dimensions are less than the current mode then
	 * the output will appear in the top left of the screen. This can be
	 * centered by adjusting horizontal active start, right border start,
	 * vertical active start and bottom border start. At this point it's
	 * not entirely clear where this should be done. On the one hand it's
	 * related to pdp_crtc_helper_mode_set but on the other hand there
	 * might not always be a call to pdp_crtc_helper_mode_set. This needs
	 * to be investigated.
	 */
	switch (dev_priv->version) {
	case RTK_DRM_VERSION_GDMA_ADAPTER:
		if(drm_drv_uses_atomic_modeset(crtc->dev))
			rtk_drm_gdma_set_surface_atomic(crtc->dev->dev,
					      drm_plane_index(plane), address, 0,
					      new_plane_state->crtc_x, new_plane_state->crtc_y,
					      new_plane_state->crtc_w, new_plane_state->crtc_h,
					      new_plane_state->src_x >> 16, new_plane_state->src_y >> 16,
					      new_plane_state->src_w >> 16, new_plane_state->src_h >> 16, pitch,
					      format, modifier,
					      255,
					      false);
		else
			rtk_drm_gdma_set_surface(crtc->dev->dev,
					      drm_plane_index(plane), address, 0,
					      src_x, src_y,
					      fb->width, fb->height, pitch,
					      format, modifier,
					      255,
					      false);
		break;
	default:
			BUG();
	}
}
