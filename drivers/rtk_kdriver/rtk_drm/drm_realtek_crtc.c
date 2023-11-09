
#include "rtk_drm_linux_fence.h"
#include "drm_realtek_drv.h"

#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0))
#include <drm/drm_vblank.h>
#else
#include <drm/drmP.h>
#endif

#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_plane_helper.h>

#include "rtk_drm_dma_resv.h"
#include "drm_realtek_gem.h"

#include "realtek_gdma_adapter.h"


#if defined(RTK_DRM_USE_ATOMIC)
#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic.h>
#endif

#include "kernel_compatibility.h"

enum rtk_drm_crtc_flip_status {
	RTK_DRM_CRTC_FLIP_STATUS_NONE = 0,
	RTK_DRM_CRTC_FLIP_STATUS_PENDING,
	RTK_DRM_CRTC_FLIP_STATUS_DONE,
};

struct rtk_drm_flip_data {
	struct dma_fence_cb base;
	struct drm_crtc *crtc;
	struct dma_fence *wait_fence;
};


void rtk_drm_crtc_set_plane_enabled(struct drm_crtc *crtc, bool enable)
{
	struct rtk_drm_private *dev_priv = crtc->dev->dev_private;
	struct drm_plane * plane;

	switch (dev_priv->version) {
	case RTK_DRM_VERSION_GDMA_ADAPTER:
		list_for_each_entry(plane, &crtc->dev->mode_config.plane_list, head)
		{
			if(crtc->state->plane_mask & drm_plane_index(plane))
				rtk_drm_gdma_set_plane_enabled(crtc->dev->dev, drm_plane_index(plane), enable);
		}
		break;
	default:
		BUG();
	}
}

static void rtk_drm_crtc_set_syncgen_enabled(struct drm_crtc *crtc, bool enable)
{
	struct rtk_drm_private *dev_priv = crtc->dev->dev_private;

	switch (dev_priv->version) {
	case RTK_DRM_VERSION_GDMA_ADAPTER:
			rtk_drm_gdma_set_syncgen_enabled(crtc->dev->dev, enable);
		break;
	default:
		BUG();
	}
}

static void rtk_drm_crtc_set_enabled(struct drm_crtc *crtc, bool enable)
{
	struct rtk_drm_private *dev_priv = crtc->dev->dev_private;

	if (enable) {
		rtk_drm_crtc_set_syncgen_enabled(crtc, enable);
		rtk_drm_crtc_set_plane_enabled(crtc, dev_priv->display_enabled);
		drm_crtc_vblank_on(crtc);
	} else {
		drm_crtc_vblank_off(crtc);
		rtk_drm_crtc_set_plane_enabled(crtc, enable);
		rtk_drm_crtc_set_syncgen_enabled(crtc, enable);
	}
}

static void rtk_drm_crtc_mode_set(struct drm_crtc *crtc,
			      struct drm_display_mode *adjusted_mode)
{
	/*
	 * ht   = horizontal total
	 * hbps = horizontal back porch start
	 * has  = horizontal active start
	 * hlbs = horizontal left border start
	 * hfps = horizontal front porch start
	 * hrbs = horizontal right border start
	 *
	 * vt   = vertical total
	 * vbps = vertical back porch start
	 * vas  = vertical active start
	 * vtbs = vertical top border start
	 * vfps = vertical front porch start
	 * vbbs = vertical bottom border start
	 */
	struct rtk_drm_private *dev_priv = crtc->dev->dev_private;
	uint32_t ht = adjusted_mode->htotal;
	uint32_t hbps = adjusted_mode->hsync_end - adjusted_mode->hsync_start;
	uint32_t has = (adjusted_mode->htotal - adjusted_mode->hsync_start);
	uint32_t hlbs = has;
	uint32_t hfps = (hlbs + adjusted_mode->hdisplay);
	uint32_t hrbs = hfps;
	uint32_t vt = adjusted_mode->vtotal;
	uint32_t vbps = adjusted_mode->vsync_end - adjusted_mode->vsync_start;
	uint32_t vas = (adjusted_mode->vtotal - adjusted_mode->vsync_start);
	uint32_t vtbs = vas;
	uint32_t vfps = (vtbs + adjusted_mode->vdisplay);
	uint32_t vbbs = vfps;


	switch (dev_priv->version) {
	case RTK_DRM_VERSION_GDMA_ADAPTER:
		rtk_drm_gdma_set_updates_enabled(crtc->dev->dev, false);
		rtk_drm_gdma_reset_planes(crtc->dev->dev);
		rtk_drm_gdma_mode_set(crtc->dev->dev, 
			     adjusted_mode->hdisplay, adjusted_mode->vdisplay,
			     hbps, ht, has,
			     hlbs, hfps, hrbs,
			     vbps, vt, vas,
			     vtbs, vfps, vbbs,
			     adjusted_mode->flags & DRM_MODE_FLAG_NHSYNC,
			     adjusted_mode->flags & DRM_MODE_FLAG_NVSYNC);
		rtk_drm_gdma_set_powerdwn_enabled(crtc->dev->dev, false);
		rtk_drm_gdma_set_updates_enabled(crtc->dev->dev, true);
		break;

	default:
		BUG();
	}
}


static bool rtk_drm_crtc_helper_mode_fixup(struct drm_crtc *crtc,
					const struct drm_display_mode *mode,
					struct drm_display_mode *adjusted_mode)
{
	return true;
}

static void rtk_drm_crtc_flip_complete(struct drm_crtc *crtc);

#if defined(RTK_DRM_USE_ATOMIC)
static void rtk_drm_crtc_helper_mode_set_nofb(struct drm_crtc *crtc)
{
	rtk_drm_crtc_mode_set(crtc, &crtc->state->adjusted_mode);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
static void rtk_drm_crtc_helper_atomic_begin(struct drm_crtc *crtc,
					 struct drm_atomic_state *state)
#else
static void rtk_drm_crtc_helper_atomic_begin(struct drm_crtc *crtc,
					 struct drm_crtc_state *old_crtc_state)
#endif
{
	rtk_drm_gdma_atomic_begin();
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
static void rtk_drm_crtc_helper_atomic_flush(struct drm_crtc *crtc,
					 struct drm_atomic_state *state)
#else
static void rtk_drm_crtc_helper_atomic_flush(struct drm_crtc *crtc,
					 struct drm_crtc_state *old_crtc_state)
#endif
{
	struct drm_crtc_state *new_crtc_state = crtc->state;
	int i, plane_count = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
	struct drm_atomic_state *atomic_state = state;
	struct drm_crtc_state *old_crtc_state;

	for_each_oldnew_crtc_in_state(state, crtc, old_crtc_state, new_crtc_state, i) {
		if (!new_crtc_state->active || !old_crtc_state->active)
			return;
	}

#else
	struct drm_atomic_state *atomic_state = old_crtc_state->state;

	if (!new_crtc_state->active || !old_crtc_state->active)
		return;
#endif


	for(i=0; i < atomic_state->dev->mode_config.num_total_plane; i++)
		if(atomic_state->planes[i].ptr)
			plane_count++;

	rtk_drm_gdma_atomic_flush(plane_count);

	if (crtc->state->event) {
		struct rtk_drm_crtc *rtk_crtc = to_rtk_drm_crtc(crtc);
		unsigned long flags;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		rtk_crtc->flip_async = new_crtc_state->async_flip;
#else
		rtk_crtc->flip_async = !!(new_crtc_state->pageflip_flags
					  & DRM_MODE_PAGE_FLIP_ASYNC);
#endif
		if (rtk_crtc->flip_async)
			WARN_ON(drm_crtc_vblank_get(crtc) != 0);

		spin_lock_irqsave(&crtc->dev->event_lock, flags);
		rtk_crtc->flip_event = crtc->state->event;
		crtc->state->event = NULL;

		atomic_set(&rtk_crtc->flip_status, RTK_DRM_CRTC_FLIP_STATUS_DONE);
		spin_unlock_irqrestore(&crtc->dev->event_lock, flags);

		if (rtk_crtc->flip_async)
			rtk_drm_crtc_flip_complete(crtc);
	}
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
static void rtk_drm_crtc_helper_atomic_enable(struct drm_crtc *crtc,
					  struct drm_atomic_state *state)
#else
static void rtk_drm_crtc_helper_atomic_enable(struct drm_crtc *crtc,
					  struct drm_crtc_state *old_crtc_state)
#endif
{
	rtk_drm_crtc_set_enabled(crtc, true);

	if (crtc->state->event) {
		struct rtk_drm_crtc *rtk_crtc = to_rtk_drm_crtc(crtc);
		unsigned long flags;

		WARN_ON(drm_crtc_vblank_get(crtc) != 0);

		spin_lock_irqsave(&crtc->dev->event_lock, flags);
		rtk_crtc->flip_event = crtc->state->event;
		crtc->state->event = NULL;

		atomic_set(&rtk_crtc->flip_status, RTK_DRM_CRTC_FLIP_STATUS_DONE);
		spin_unlock_irqrestore(&crtc->dev->event_lock, flags);
	}
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
static void rtk_drm_crtc_helper_atomic_disable(struct drm_crtc *crtc,
					  struct drm_atomic_state *state)
#else
static void rtk_drm_crtc_helper_atomic_disable(struct drm_crtc *crtc,
					   struct drm_crtc_state *old_crtc_state)
#endif
{
	rtk_drm_crtc_set_enabled(crtc, false);

	if (crtc->state->event) {
		unsigned long flags;

		spin_lock_irqsave(&crtc->dev->event_lock, flags);
		drm_crtc_send_vblank_event(crtc, crtc->state->event);
		crtc->state->event = NULL;
		spin_unlock_irqrestore(&crtc->dev->event_lock, flags);
	}
}
#else
static void rtk_drm_crtc_helper_dpms(struct drm_crtc *crtc, int mode)
{
}

static void rtk_drm_crtc_helper_prepare(struct drm_crtc *crtc)
{
	rtk_drm_crtc_set_enabled(crtc, false);
}

static void rtk_drm_crtc_helper_commit(struct drm_crtc *crtc)
{
	rtk_drm_crtc_set_enabled(crtc, true);
}

static int rtk_drm_crtc_helper_mode_set_base_atomic(struct drm_crtc *crtc,
						struct drm_framebuffer *fb,
						int x, int y,
						enum mode_set_atomic atomic)
{
	if (x < 0 || y < 0)
		return -EINVAL;

	rtk_drm_plane_set_surface(crtc, crtc->primary, fb,
			      (uint32_t) x, (uint32_t) y);

	return 0;
}

static int rtk_drm_crtc_helper_mode_set_base(struct drm_crtc *crtc,
					 int x, int y,
					 struct drm_framebuffer *old_fb)
{
	if (!crtc->primary->fb) {
		DRM_ERROR("no framebuffer\n");
		return 0;
	}

	return rtk_drm_crtc_helper_mode_set_base_atomic(crtc,
						    crtc->primary->fb,
						    x, y,
						    0);
}

static int rtk_drm_crtc_helper_mode_set(struct drm_crtc *crtc,
				    struct drm_display_mode *mode,
				    struct drm_display_mode *adjusted_mode,
				    int x, int y,
				    struct drm_framebuffer *old_fb)
{
	rtk_drm_crtc_mode_set(crtc, adjusted_mode);

	return rtk_drm_crtc_helper_mode_set_base(crtc, x, y, old_fb);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
static void rtk_drm_crtc_helper_load_lut(struct drm_crtc *crtc)
{
}
#endif

static void rtk_drm_crtc_helper_disable(struct drm_crtc *crtc)
{
	struct rtk_drm_crtc *rtk_crtc = to_rtk_drm_crtc(crtc);
	enum rtk_drm_crtc_flip_status status;

	rtk_drm_crtc_set_enabled(crtc, false);

	status = atomic_read(&rtk_crtc->flip_status);
	if (status != RTK_DRM_CRTC_FLIP_STATUS_NONE) {
		long lerr;

		lerr = wait_event_timeout(
			rtk_crtc->flip_pending_wait_queue,
			atomic_read(&rtk_crtc->flip_status)
					!= RTK_DRM_CRTC_FLIP_STATUS_PENDING,
			30 * HZ);
		if (!lerr)
			DRM_ERROR("Failed to wait for pending flip\n");
		else if (!rtk_crtc->flip_async)
			rtk_drm_crtc_flip_complete(crtc);
	}
}
#endif /* defined(RTK_DRM_USE_ATOMIC) */


static void rtk_drm_crtc_destroy(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct rtk_drm_private *dev_priv = dev->dev_private;
	struct rtk_drm_crtc *rtk_crtc = to_rtk_drm_crtc(crtc);

	DRM_DEBUG_DRIVER("[CRTC:%d]\n", crtc->base.id);

	drm_crtc_cleanup(crtc);

	kfree(rtk_crtc);
	dev_priv->crtc = NULL;
}

static void rtk_drm_crtc_flip_complete(struct drm_crtc *crtc)
{
	struct rtk_drm_crtc *rtk_crtc = to_rtk_drm_crtc(crtc);
	unsigned long flags;

	spin_lock_irqsave(&crtc->dev->event_lock, flags);

	/* The flipping process has been completed so reset the flip state */
	atomic_set(&rtk_crtc->flip_status, RTK_DRM_CRTC_FLIP_STATUS_NONE);
	rtk_crtc->flip_async = false;

#if !defined(RTK_DRM_USE_ATOMIC)
	if (rtk_crtc->flip_data) {
		dma_fence_put(rtk_crtc->flip_data->wait_fence);
		kfree(rtk_crtc->flip_data);
		rtk_crtc->flip_data = NULL;
	}
#endif

	if (rtk_crtc->flip_event) {
		drm_crtc_send_vblank_event(crtc, rtk_crtc->flip_event);
		rtk_crtc->flip_event = NULL;
	}

	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);
}

#if !defined(RTK_DRM_USE_ATOMIC)
static void rtk_drm_crtc_flip(struct drm_crtc *crtc)
{
	struct rtk_drm_crtc *rtk_crtc = to_rtk_drm_crtc(crtc);
	struct drm_framebuffer *old_fb;

	WARN_ON(atomic_read(&to_rtk_drm_crtc(crtc)->flip_status)
			!= RTK_DRM_CRTC_FLIP_STATUS_PENDING);

	old_fb = rtk_crtc->old_fb;
	rtk_crtc->old_fb = NULL;

	/*
	 * The graphics stream registers latch on vsync so we can go ahead and
	 * do the flip now.
	 */
	(void) rtk_drm_crtc_helper_mode_set_base(crtc, crtc->x, crtc->y, old_fb);

	atomic_set(&rtk_crtc->flip_status, RTK_DRM_CRTC_FLIP_STATUS_DONE);
	wake_up(&rtk_crtc->flip_pending_wait_queue);

	if (rtk_crtc->flip_async)
		rtk_drm_crtc_flip_complete(crtc);
}

static void rtk_drm_crtc_flip_cb(struct dma_fence *fence, struct dma_fence_cb *cb)
{
	struct rtk_drm_flip_data *flip_data =
		container_of(cb, struct rtk_drm_flip_data, base);
	rtk_drm_crtc_flip(flip_data->crtc);
}

static void rtk_drm_crtc_flip_schedule_cb(struct dma_fence *fence,
				      struct dma_fence_cb *cb)
{
	struct rtk_drm_flip_data *flip_data =
		container_of(cb, struct rtk_drm_flip_data, base);
	int err = 0;

	if (flip_data->wait_fence){
		err = dma_fence_add_callback(flip_data->wait_fence,
					     &flip_data->base,
					     rtk_drm_crtc_flip_cb);
	}

	if (!flip_data->wait_fence || err) {
		if (err && err != -ENOENT)
			DRM_ERROR("flip failed to wait on old buffer\n");
		rtk_drm_crtc_flip_cb(flip_data->wait_fence, &flip_data->base);
	}
}

static int rtk_drm_crtc_flip_schedule(struct drm_crtc *crtc,
				  struct drm_gem_object *obj,
				  struct drm_gem_object *old_obj)
{
	struct rtk_drm_crtc *rtk_crtc = to_rtk_drm_crtc(crtc);
	struct dma_resv *resv = rtk_drm_gem_get_resv(obj);
	struct dma_resv *old_resv = rtk_drm_gem_get_resv(old_obj);
	struct rtk_drm_flip_data *flip_data;
	struct dma_fence *fence;
	int err;

	flip_data = kmalloc(sizeof(*flip_data), GFP_KERNEL);
	if (!flip_data)
		return -ENOMEM;

	flip_data->crtc = crtc;

	ww_mutex_lock(&old_resv->lock, NULL);
	flip_data->wait_fence =
		dma_fence_get(dma_resv_get_excl(old_resv));

	if (old_resv != resv) {
		ww_mutex_unlock(&old_resv->lock);
		ww_mutex_lock(&resv->lock, NULL);
	}

	fence = dma_fence_get(dma_resv_get_excl(resv));
	ww_mutex_unlock(&resv->lock);

	rtk_crtc->flip_data = flip_data;
	atomic_set(&rtk_crtc->flip_status, RTK_DRM_CRTC_FLIP_STATUS_PENDING);

	if (fence) {
		err = dma_fence_add_callback(fence, &flip_data->base,
					     rtk_drm_crtc_flip_schedule_cb);
		dma_fence_put(fence);
		if (err && err != -ENOENT)
			goto err_set_flip_status_none;
	}

	if (!fence || err == -ENOENT) {
		rtk_drm_crtc_flip_schedule_cb(fence, &flip_data->base);
		err = 0;
	}

	return err;

err_set_flip_status_none:
	atomic_set(&rtk_crtc->flip_status, RTK_DRM_CRTC_FLIP_STATUS_NONE);
	dma_fence_put(flip_data->wait_fence);
	kfree(flip_data);
	return err;
}

static int rtk_drm_crtc_page_flip(struct drm_crtc *crtc,
			      struct drm_framebuffer *fb,
			      struct drm_pending_vblank_event *event,
			      uint32_t page_flip_flags
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
			      , struct drm_modeset_acquire_ctx *ctx
#endif
			     )
{
	struct rtk_drm_crtc *rtk_crtc = to_rtk_drm_crtc(crtc);
	struct rtk_drm_framebuffer *rtk_fb = to_rtk_drm_framebuffer(fb);
	struct rtk_drm_framebuffer *rtk_old_fb =
		to_rtk_drm_framebuffer(crtc->primary->fb);
	enum rtk_drm_crtc_flip_status status;
	unsigned long flags;
	int err;

	spin_lock_irqsave(&crtc->dev->event_lock, flags);
	status = atomic_read(&rtk_crtc->flip_status);
	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);

	if (status != RTK_DRM_CRTC_FLIP_STATUS_NONE)
		return -EBUSY;

	if (!(page_flip_flags & DRM_MODE_PAGE_FLIP_ASYNC)) {
		err = drm_crtc_vblank_get(crtc);
		if (err)
			return err;
	}

	rtk_crtc->old_fb = crtc->primary->fb;
	rtk_crtc->flip_event = event;
	rtk_crtc->flip_async = !!(page_flip_flags & DRM_MODE_PAGE_FLIP_ASYNC);

	/* Set the crtc primary plane to point to the new framebuffer */
	crtc->primary->fb = fb;

	err = rtk_drm_crtc_flip_schedule(crtc, rtk_fb->obj[0], rtk_old_fb->obj[0]);
	if (err) {
		crtc->primary->fb = rtk_crtc->old_fb;
		rtk_crtc->old_fb = NULL;
		rtk_crtc->flip_event = NULL;
		rtk_crtc->flip_async = false;

		DRM_ERROR("failed to schedule flip (err=%d)\n", err);
		goto err_vblank_put;
	}

	return 0;

err_vblank_put:
	if (!(page_flip_flags & DRM_MODE_PAGE_FLIP_ASYNC))
		drm_crtc_vblank_put(crtc);
	return err;
}
#endif /* !defined(RTK_DRM_USE_ATOMIC) */

static const struct drm_crtc_helper_funcs rtk_drm_crtc_helper_funcs = {
	.mode_fixup = rtk_drm_crtc_helper_mode_fixup,
#if defined(RTK_DRM_USE_ATOMIC)
	.mode_set_nofb = rtk_drm_crtc_helper_mode_set_nofb,
	.atomic_begin = rtk_drm_crtc_helper_atomic_begin,
	.atomic_flush = rtk_drm_crtc_helper_atomic_flush,
	.atomic_enable = rtk_drm_crtc_helper_atomic_enable,
	.atomic_disable = rtk_drm_crtc_helper_atomic_disable,
#else
	.dpms = rtk_drm_crtc_helper_dpms,
	.prepare = rtk_drm_crtc_helper_prepare,
	.commit = rtk_drm_crtc_helper_commit,
	.mode_set = rtk_drm_crtc_helper_mode_set,
	.mode_set_base = rtk_drm_crtc_helper_mode_set_base,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	.load_lut = rtk_drm_crtc_helper_load_lut,
#endif
	.mode_set_base_atomic = rtk_drm_crtc_helper_mode_set_base_atomic,
	.disable = rtk_drm_crtc_helper_disable,
#endif
};

static const struct drm_crtc_funcs rtk_drm_crtc_funcs = {
	.destroy = rtk_drm_crtc_destroy,
#if defined(RTK_DRM_USE_ATOMIC)
	.reset = drm_atomic_helper_crtc_reset,
	.set_config = drm_atomic_helper_set_config,
	.page_flip = drm_atomic_helper_page_flip,
	.atomic_duplicate_state = drm_atomic_helper_crtc_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_crtc_destroy_state,
#else
	.set_config = drm_crtc_helper_set_config,
	.page_flip = rtk_drm_crtc_page_flip,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
	.enable_vblank  = rtk_drm_enable_vblank,
	.disable_vblank = rtk_drm_disable_vblank,
#endif
};


struct drm_crtc *rtk_drm_crtc_create(struct drm_device *dev, uint32_t number,
				 struct drm_plane *primary_plane)
{
	struct rtk_drm_crtc *rtk_crtc;
	int err;

	rtk_crtc = kzalloc(sizeof(*rtk_crtc), GFP_KERNEL);
	if (!rtk_crtc) {
		err = -ENOMEM;
		goto err_exit;
	}

	init_waitqueue_head(&rtk_crtc->flip_pending_wait_queue);
	atomic_set(&rtk_crtc->flip_status, RTK_DRM_CRTC_FLIP_STATUS_NONE);
	rtk_crtc->number = number;

	switch (number) {
	case 0:
	{
		break;
	}
	default:
		DRM_ERROR("invalid crtc number %u\n", number);
		err = -EINVAL;
		goto err_crtc_free;
	}

	err = drm_crtc_init_with_planes(dev, &rtk_crtc->base, primary_plane,
					NULL, &rtk_drm_crtc_funcs, NULL);
	if (err) {
		DRM_ERROR("CRTC init with planes failed");
		goto err_crtc_free;
	}

	drm_crtc_helper_add(&rtk_crtc->base, &rtk_drm_crtc_helper_funcs);

	DRM_DEBUG_DRIVER("[CRTC:%d]\n", rtk_crtc->base.base.id);

	return &rtk_crtc->base;

err_crtc_free:
	kfree(rtk_crtc);
err_exit:
	return ERR_PTR(err);
}

void rtk_drm_crtc_set_vblank_enabled(struct drm_crtc *crtc, bool enable)
{
	struct rtk_drm_private *dev_priv = crtc->dev->dev_private;

	switch (dev_priv->version) {
	case RTK_DRM_VERSION_GDMA_ADAPTER:
		rtk_drm_gdma_set_vblank_enabled(crtc->dev->dev, enable);
		break;
	default:
		BUG();
	}
}

void rtk_drm_crtc_irq_handler(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct rtk_drm_private *dev_priv = dev->dev_private;
	struct rtk_drm_crtc *rtk_crtc = to_rtk_drm_crtc(crtc);
	bool handled;

	switch (dev_priv->version) {
	case RTK_DRM_VERSION_GDMA_ADAPTER:
		handled = rtk_drm_gdma_check_and_clear_vblank(dev->dev);
		break;
	default:
		handled = false;
		break;
	}

	if (handled) {
		enum rtk_drm_crtc_flip_status status;

		drm_handle_vblank(dev, rtk_crtc->number);

		status = atomic_read(&rtk_crtc->flip_status);
		if (status == RTK_DRM_CRTC_FLIP_STATUS_DONE) {
			if (!rtk_crtc->flip_async) {
				rtk_drm_crtc_flip_complete(crtc);
#if !defined(RTK_DRM_USE_ATOMIC)
				drm_crtc_vblank_put(crtc);
#endif
			}
		}
	}
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0))
void rtk_drm_crtc_flip_event_cancel(struct drm_crtc *crtc, struct drm_file *file)
{
	struct rtk_drm_crtc *rtk_crtc = to_rtk_drm_crtc(crtc);
	unsigned long flags;

	spin_lock_irqsave(&crtc->dev->event_lock, flags);

	if (rtk_crtc->flip_event &&
	    rtk_crtc->flip_event->base.file_priv == file) {
		rtk_crtc->flip_event->base.destroy(&rtk_crtc->flip_event->base);
		rtk_crtc->flip_event = NULL;
	}

	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);
}
#endif
