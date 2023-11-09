#include "drm_realtek_drv.h"

#include <linux/moduleparam.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0))
#include <drm/drm_drv.h>
#include <drm/drm_print.h>
#else
#include <drm/drmP.h>
#endif

#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
#include <drm/drm_gem.h>
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
#define drm_gem_fb_create(...) rtk_drm_framebuffer_create(__VA_ARGS__)
#else
#include <drm/drm_gem_framebuffer_helper.h>
#endif

#if defined(RTK_DRM_USE_ATOMIC)
#include <drm/drm_atomic_helper.h>
#endif

#include "rtk_drm_fourcc.h"
#include "realtek_gdma_adapter.h"

#include "kernel_compatibility.h"

static bool async_flip_enable = false;

module_param(async_flip_enable, bool, 0444);

MODULE_PARM_DESC(async_flip_enable, "Enable support for 'faked' async flipping (default: N)");

static inline int
drm_mode_fb_cmd2_validate(const struct drm_mode_fb_cmd2 *mode_cmd)
{
	switch (mode_cmd->pixel_format) {
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
		break;
	default:
		DRM_ERROR_RATELIMITED("pixel format not supported (format = %u)\n", mode_cmd->pixel_format);
		return -EINVAL;
	}

	if (mode_cmd->flags & DRM_MODE_FB_INTERLACED) {
		DRM_ERROR_RATELIMITED("interlaced framebuffers not supported\n");
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
	switch (mode_cmd->modifier[0]) {
	case DRM_FORMAT_MOD_PVR_FBCDC_8x8_V13:
	case DRM_FORMAT_MOD_PVR_FBCDC_16x4_V13:
	case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY75_8x8_V13:
	case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY75_16x4_V13:
	case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY50_8x8_V13:
	case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY50_16x4_V13:
	case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY25_8x8_V13:
	case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY25_16x4_V13:
	case DRM_FORMAT_MOD_LINEAR:
		break;
	default:
		DRM_ERROR_RATELIMITED("format modifier 0x%llx is not supported\n",
			  mode_cmd->modifier[0]);
		return -EINVAL;
	}
#endif

	return 0;
}

static void rtk_drm_framebuffer_destroy(struct drm_framebuffer *fb)
{
	struct rtk_drm_framebuffer *rtk_fb = to_rtk_drm_framebuffer(fb);

	DRM_DEBUG_DRIVER("[FB:%d]\n", fb->base.id);

	drm_framebuffer_cleanup(fb);

	drm_gem_object_put(rtk_fb->obj[0]);

	kfree(rtk_fb);
}

static int rtk_drm_framebuffer_create_handle(struct drm_framebuffer *fb,
					 struct drm_file *file,
					 unsigned int *handle)
{
	struct rtk_drm_framebuffer *rtk_fb = to_rtk_drm_framebuffer(fb);

	DRM_DEBUG_DRIVER("[FB:%d]\n", fb->base.id);

	return drm_gem_handle_create(file, rtk_fb->obj[0], handle);
}

static const struct drm_framebuffer_funcs rtk_drm_framebuffer_funcs = {
	.destroy = rtk_drm_framebuffer_destroy,
	.create_handle = rtk_drm_framebuffer_create_handle,
	.dirty = NULL,
};

static inline int
rtk_drm_framebuffer_init(struct rtk_drm_private *dev_priv,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) || \
	(defined(CHROMIUMOS_KERNEL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)))
		     const
#endif
		     struct drm_mode_fb_cmd2 *mode_cmd,
		     struct rtk_drm_framebuffer *rtk_fb,
		     struct drm_gem_object *obj)
{
	struct drm_framebuffer *fb;

	if (!rtk_fb)
		return -EINVAL;

	fb = to_drm_framebuffer(rtk_fb);
	rtk_fb->obj[0] = obj;

	drm_helper_mode_fill_fb_struct(dev_priv->dev, fb, mode_cmd);

	return drm_framebuffer_init(dev_priv->dev, fb, &rtk_drm_framebuffer_funcs);
}

int rtk_drm_modeset_validate_init(struct rtk_drm_private *dev_priv,
			      struct drm_mode_fb_cmd2 *mode_cmd,
			      struct rtk_drm_framebuffer *pdp_fb,
			      struct drm_gem_object *obj)
{
	int err;

	err = drm_mode_fb_cmd2_validate(mode_cmd);
	if (err)
		return err;

	return rtk_drm_framebuffer_init(dev_priv, mode_cmd, pdp_fb, obj);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
static struct drm_framebuffer *
rtk_drm_framebuffer_create(struct drm_device *dev,
		       struct drm_file *file,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) || \
	(defined(CHROMIUMOS_KERNEL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)))
		       const
#endif
		       struct drm_mode_fb_cmd2 *mode_cmd)
{
	struct rtk_drm_private *dev_priv = dev->dev_private;
	struct drm_gem_object *obj;
	struct rtk_drm_framebuffer *rtk_fb;
	int err;

	obj = drm_gem_object_lookup(file, mode_cmd->handles[0]);
	if (!obj) {
		DRM_ERROR("failed to find buffer with handle %u\n",
			  mode_cmd->handles[0]);
		err = -ENOENT;
		goto err_out;
	}

	rtk_fb = kzalloc(sizeof(*rtk_fb), GFP_KERNEL);
	if (!rtk_fb) {
		err = -ENOMEM;
		goto err_obj_put;
	}

	err = rtk_drm_framebuffer_init(dev_priv, mode_cmd, rtk_fb, obj);
	if (err) {
		DRM_ERROR("failed to initialise framebuffer (err=%d)\n", err);
		goto err_free_fb;
	}

	DRM_DEBUG_DRIVER("[FB:%d]\n", rtk_fb->base.base.id);

	return &rtk_fb->base;

err_free_fb:
	kfree(rtk_fb);
err_obj_put:
	drm_gem_object_put(obj);
err_out:
	return ERR_PTR(err);
}
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)) */


/*************************************************************************
 * DRM mode config callbacks
 **************************************************************************/

static struct drm_framebuffer *
rtk_drm_fb_create(struct drm_device *dev,
			struct drm_file *file,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) || (defined(CHROMIUMOS_KERNEL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)))
			const
#endif
			struct drm_mode_fb_cmd2 *mode_cmd)
{
	struct drm_framebuffer *fb;
	int err;

	err = drm_mode_fb_cmd2_validate(mode_cmd);
	if (err)
		return ERR_PTR(err);

	fb = drm_gem_fb_create(dev, file, mode_cmd);
	if (IS_ERR(fb))
		goto out;

	DRM_DEBUG_DRIVER("[FB:%d]\n", fb->base.id);

out:
	return fb;
}

static const struct drm_mode_config_funcs rtk_drm_mode_config_funcs = {
	.fb_create = rtk_drm_fb_create,
	.output_poll_changed = NULL,
#if defined(RTK_DRM_USE_ATOMIC)
	.atomic_check = drm_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
#endif
};


int rtk_drm_modeset_early_init(struct rtk_drm_private *dev_priv)
{
	struct drm_device *dev = dev_priv->dev;
	int err;
	uint32_t screen_width = 0, screen_height = 0;

	drm_mode_config_init(dev);

	dev->mode_config.funcs = &rtk_drm_mode_config_funcs;
	dev->mode_config.min_width = 0;
	dev->mode_config.min_height = 0;

	switch (dev_priv->version) {
	case RTK_DRM_VERSION_GDMA_ADAPTER:
		rtk_drm_get_screen_size(&screen_width, &screen_height);
		dev->mode_config.max_width = screen_width;
		dev->mode_config.max_height = screen_height;
		break;
	default:
		BUG();
	}

	DRM_INFO("max_width is %d\n", dev->mode_config.max_width);
	DRM_INFO("max_height is %d\n", dev->mode_config.max_height);

	dev->mode_config.fb_base = 0;
	dev->mode_config.async_page_flip = async_flip_enable;

	DRM_INFO("%s async flip support is %s\n", dev->driver->name, async_flip_enable ? "enabled" : "disabled");

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
	dev->mode_config.allow_fb_modifiers = true;
#endif

	dev_priv->plane = rtk_drm_plane_create(dev, DRM_PLANE_TYPE_PRIMARY);
	if (IS_ERR(dev_priv->plane)) {
		DRM_ERROR("failed to create a primary plane\n");
		err = PTR_ERR(dev_priv->plane);
		goto err_config_cleanup;
	}

	dev_priv->crtc = rtk_drm_crtc_create(dev, 0, dev_priv->plane);
	if (IS_ERR(dev_priv->crtc)) {
		DRM_ERROR("failed to create a CRTC\n");
		err = PTR_ERR(dev_priv->crtc);
		goto err_config_cleanup;
	}

	dev_priv->overlay_plane = rtk_drm_plane_create(dev, DRM_PLANE_TYPE_OVERLAY);
	if (IS_ERR(dev_priv->overlay_plane)) {
		DRM_ERROR("failed to create a overlay plane\n");
		err = PTR_ERR(dev_priv->overlay_plane);
		goto err_config_cleanup;
	}

	switch (dev_priv->version) {
	case RTK_DRM_VERSION_GDMA_ADAPTER:
		dev_priv->connector = rtk_drm_connector_create(dev);
		if (IS_ERR(dev_priv->connector)) {
			DRM_ERROR("failed to create a connector\n");
			err = PTR_ERR(dev_priv->connector);
			goto err_config_cleanup;
		}

		dev_priv->encoder = rtk_drm_encoder_create(dev);
		if (IS_ERR(dev_priv->encoder)) {
			DRM_ERROR("failed to create an encoder\n");
			err = PTR_ERR(dev_priv->encoder);
			goto err_config_cleanup;
		}

		err = drm_connector_attach_encoder(dev_priv->connector,
						   dev_priv->encoder);
		if (err) {
			DRM_ERROR("can't attach [ENCODER:%d:%s] to [CONNECTOR:%d:%s] (err=%d)\n",
				  dev_priv->encoder->base.id,
				  dev_priv->encoder->name,
				  dev_priv->connector->base.id,
				  dev_priv->connector->name,
				  err);
			goto err_config_cleanup;
		}
		break;

	default:
		BUG();
	}

	DRM_DEBUG_DRIVER("initialised\n");

	return 0;

err_config_cleanup:
	drm_mode_config_cleanup(dev);

	return err;
}

static inline int rtk_drm_modeset_init_fbdev(struct rtk_drm_private *dev_priv)
{
#if defined(CONFIG_DRM_FBDEV_EMULATION)
	struct rtk_drm_fbdev *fbdev;
	int err;

	fbdev = rtk_drm_fbdev_create(dev_priv);
	if (IS_ERR(fbdev)) {
		DRM_ERROR("failed to create a fb device");
		return PTR_ERR(fbdev);
	}
	
	dev_priv->fbdev = fbdev;

	/*
	 * pdpdrmfb is registered and available for userspace to use. If this
	 * is the only or primary device, fbcon has already bound a tty to it,
	 * and the following call will take no effect. However, this may be
	 * essential in order to sync the display when fbcon was already bound
	 * to a different tty (and fbdev). This triggers ->set_config() which
	 * will in turn set up a config and then do a modeset.
	 */
	err = drm_fb_helper_restore_fbdev_mode_unlocked(&dev_priv->fbdev->helper);
	
	if (err) {
		DRM_ERROR("failed to set mode (err=%d)\n", err);
		return err;
	}
#endif
	return 0;
}

int rtk_drm_modeset_late_init(struct rtk_drm_private *dev_priv)
{
	struct drm_device *ddev = dev_priv->dev;
	int err;

	drm_mode_config_reset(ddev);

#if RTK_DRM_FBDEV_EMULATION_ENABLE
	err = rtk_drm_modeset_init_fbdev(dev_priv);
	if (err)
		DRM_INFO("fbdev init failure is not fatal, continue anyway.\n");
#endif

	if (dev_priv->connector != NULL) {
		err = drm_connector_register(dev_priv->connector);
		if (err) {
			DRM_ERROR("[CONNECTOR:%d:%s] failed to register (err=%d)\n",
				  dev_priv->connector->base.id,
				  dev_priv->connector->name,
				  err);
			return err;
		}
	}
	return 0;
}

void rtk_drm_modeset_early_cleanup(struct rtk_drm_private *dev_priv)
{
//	if (dev_priv->connector != NULL) {
//		drm_connector_unregister(dev_priv->connector);
//	}

}

void rtk_drm_modeset_late_cleanup(struct rtk_drm_private *dev_priv)
{
#if RTK_DRM_FBDEV_EMULATION_ENABLE
#if defined(CONFIG_DRM_FBDEV_EMULATION)
	rtk_drm_fbdev_destroy(dev_priv->fbdev);
#endif
#endif

	drm_mode_config_cleanup(dev_priv->dev);

	DRM_DEBUG_DRIVER("cleaned up\n");
}
