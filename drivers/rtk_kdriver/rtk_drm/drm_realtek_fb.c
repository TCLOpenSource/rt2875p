#if defined(CONFIG_DRM_FBDEV_EMULATION)
#include <linux/version.h>
#include <linux/export.h>
#include <linux/mm.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0))
#include <drm/drmP.h>
#endif
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>

#include "drm_realtek_gem.h"
#include "kernel_compatibility.h"

#define FBDEV_NAME "rtkdrmfb"

static struct fb_ops rtk_drm_fbdev_ops = {
	.owner = THIS_MODULE,
	.fb_check_var = drm_fb_helper_check_var,
	.fb_set_par = drm_fb_helper_set_par,
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
	.fb_pan_display = drm_fb_helper_pan_display,
	.fb_blank = drm_fb_helper_blank,
	.fb_setcmap = drm_fb_helper_setcmap,
	.fb_debug_enter = drm_fb_helper_debug_enter,
	.fb_debug_leave = drm_fb_helper_debug_leave,
};


static struct fb_info *
rtk_drm_fbdev_helper_alloc(struct drm_fb_helper *helper)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0))
	struct device *dev = helper->dev->dev;
	struct fb_info *info;
	int ret;

	info = framebuffer_alloc(0, dev);
	if (!info)
		return ERR_PTR(-ENOMEM);

	ret = fb_alloc_cmap(&info->cmap, 256, 0);
	if (ret)
		goto err_release;

	info->apertures = alloc_apertures(1);
	if (!info->apertures) {
		ret = -ENOMEM;
		goto err_free_cmap;
	}

	helper->fbdev = info;

	return info;

err_free_cmap:
	fb_dealloc_cmap(&info->cmap);
err_release:
	framebuffer_release(info);
	return ERR_PTR(ret);
#else
	return drm_fb_helper_alloc_fbi(helper);
#endif
}

static inline void
rtk_drm_fbdev_helper_fill_info(struct drm_fb_helper *helper,
			   struct drm_fb_helper_surface_size *sizes,
			   struct fb_info *info,
			   struct drm_mode_fb_cmd2 __maybe_unused *mode_cmd)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
	drm_fb_helper_fill_fix(info, mode_cmd->pitches[0], helper->fb->depth);
	drm_fb_helper_fill_var(info, helper, sizes->fb_width,
			       sizes->fb_height);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0))
	drm_fb_helper_fill_fix(info, mode_cmd->pitches[0],
			       helper->fb->format->depth);
	drm_fb_helper_fill_var(info, helper, helper->fb->width,
			       helper->fb->height);
#else
	drm_fb_helper_fill_info(info, helper, sizes);
#endif
}

static int rtk_drm_fbdev_probe(struct drm_fb_helper *helper,
			   struct drm_fb_helper_surface_size *sizes)
{
	struct rtk_drm_fbdev *rtk_fbdev =
		container_of(helper, struct rtk_drm_fbdev, helper);
	struct drm_framebuffer *fb =
		to_drm_framebuffer(&rtk_fbdev->fb);
	struct rtk_drm_gem_private *gem_priv = rtk_fbdev->priv->gem_priv;
	struct drm_device *dev = helper->dev;
	struct drm_mode_fb_cmd2 mode_cmd;
	struct rtk_drm_gem_object *gem_obj;
	struct drm_gem_object *obj;
	struct fb_info *info;
	void __iomem *vaddr;
	size_t obj_size;
	int err;

	if (helper->fb)
		return 0;

	mutex_lock(&dev->struct_mutex);

	/* Create a framebuffer */
	info = rtk_drm_fbdev_helper_alloc(helper);
	if (!info) {
		err = -ENOMEM;
		goto err_unlock_dev;
	}

	memset(&mode_cmd, 0, sizeof(mode_cmd));
	mode_cmd.pitches[0] =
		sizes->surface_width * DIV_ROUND_UP(sizes->surface_bpp, 8);
	mode_cmd.width = sizes->surface_width;
	mode_cmd.height = sizes->surface_height;
	printk(KERN_EMERG"%s %d width:%u height:%u %u %u %u %u",__func__,__LINE__, sizes->surface_width, sizes->surface_height, sizes->fb_width, sizes->fb_height, sizes->surface_bpp, sizes->surface_depth);
	mode_cmd.pixel_format = drm_mode_legacy_fb_format(sizes->surface_bpp,
							  sizes->surface_depth);
	obj_size = PAGE_ALIGN(mode_cmd.height * mode_cmd.pitches[0]);

	obj = rtk_drm_gem_object_create(dev, gem_priv, obj_size, 0);
	if (IS_ERR(obj)) {
		err = PTR_ERR(obj);
		goto err_unlock_dev;
	}

	err = rtk_drm_gem_object_get_pages(obj);
	if(err){
		goto err_gem_destroy;
	}

	gem_obj = to_rtk_drm_gem_obj(obj);

	vaddr = gem_obj->virt_addr;
	if (!vaddr) {
		err = PTR_ERR(vaddr);
		goto err_gem_destroy;
	}

	/* Zero fb memory, fb_memset accounts for iomem address space */
	fb_memset(vaddr, 0, obj_size);

	err = rtk_drm_modeset_validate_init(rtk_fbdev->priv, &mode_cmd,
					&rtk_fbdev->fb, obj);
	if (err)
		goto err_gem_unmap;

	helper->fb = fb;
	helper->fbdev = info;

	/* Fill out the Linux framebuffer info */
	strlcpy(info->fix.id, FBDEV_NAME, sizeof(info->fix.id));
	rtk_drm_fbdev_helper_fill_info(helper, sizes, info, &mode_cmd);
	info->par = helper;
	info->flags = FBINFO_DEFAULT | FBINFO_HWACCEL_DISABLED;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0))
	info->flags |= FBINFO_CAN_FORCE_OUTPUT;
#endif
	info->fbops = &rtk_drm_fbdev_ops;
	info->fix.smem_start = gem_obj->phy_addr;
	info->fix.smem_len = obj_size;
	info->screen_base = vaddr;
	info->screen_size = obj_size;
	info->apertures->ranges[0].base = gem_obj->phy_addr;
	info->apertures->ranges[0].size = obj_size;

	mutex_unlock(&dev->struct_mutex);
	return 0;

err_gem_unmap:
err_gem_destroy:
	rtk_drm_gem_object_free_priv(gem_priv, obj);

err_unlock_dev:
	mutex_unlock(&dev->struct_mutex);

	DRM_ERROR(FBDEV_NAME " - %s failed (err=%d)\n", __func__, err);
	return err;
}

static const struct drm_fb_helper_funcs rtk_drm_fbdev_helper_funcs = {
	.fb_probe = rtk_drm_fbdev_probe,
};

struct rtk_drm_fbdev *rtk_drm_fbdev_create(struct rtk_drm_private *dev_priv)
{
	struct rtk_drm_fbdev *rtk_fbdev;
	int err;

	rtk_fbdev = kzalloc(sizeof(*rtk_fbdev), GFP_KERNEL);
	if (!rtk_fbdev)
		return ERR_PTR(-ENOMEM);

	drm_fb_helper_prepare(dev_priv->dev, &rtk_fbdev->helper,
			      &rtk_drm_fbdev_helper_funcs);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
	err = drm_fb_helper_init(dev_priv->dev, &rtk_fbdev->helper, 1, 1);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0))
	err = drm_fb_helper_init(dev_priv->dev, &rtk_fbdev->helper, 1);
#else
	err = drm_fb_helper_init(dev_priv->dev, &rtk_fbdev->helper);
#endif
	if (err)
		goto err_free_fbdev;

	rtk_fbdev->priv = dev_priv;
	rtk_fbdev->preferred_bpp = 32;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 3, 0))
	drm_fb_helper_single_add_all_connectors(&rtk_fbdev->helper);
#endif

	/* Call ->fb_probe() */
	err = drm_fb_helper_initial_config(&rtk_fbdev->helper, rtk_fbdev->preferred_bpp);
	if (err)
		goto err_fb_helper_fini;

	DRM_DEBUG_DRIVER(FBDEV_NAME " - fb device registered\n");
	return rtk_fbdev;

err_fb_helper_fini:
	drm_fb_helper_fini(&rtk_fbdev->helper);

err_free_fbdev:
	kfree(rtk_fbdev);

	DRM_ERROR(FBDEV_NAME " - %s, failed (err=%d)\n", __func__, err);
	return ERR_PTR(err);
}

void rtk_drm_fbdev_destroy(struct rtk_drm_fbdev *rtk_fbdev)
{
	struct rtk_drm_framebuffer *rtk_fb;
	struct drm_framebuffer *fb;

	if (!rtk_fbdev)
		return;

	drm_fb_helper_unregister_fbi(&rtk_fbdev->helper);
	rtk_fb = &rtk_fbdev->fb;

	rtk_drm_gem_object_put_pages(rtk_fb->obj[0]);
	drm_gem_object_put(rtk_fb->obj[0]);

	drm_fb_helper_fini(&rtk_fbdev->helper);

	fb = to_drm_framebuffer(rtk_fb);

	/**
	 * If the driver's probe function hasn't been called
	 * (due to deferred setup of the framebuffer device),
	 * then the framebuffer won't have been initialised.
	 * Check this before attempting to clean it up.
	 */
	if (fb && fb->dev)
		drm_framebuffer_cleanup(fb);

	kfree(rtk_fbdev);
}
#endif /* CONFIG_DRM_FBDEV_EMULATION */
