
#include <linux/module.h>
#include <linux/version.h>
#include <linux/component.h>
#include <linux/of_platform.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0))
#include <drm/drm_drv.h>
#include <drm/drm_file.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_prime.h>
#include <drm/drm_print.h>
#include <drm/drm_vblank.h>
#else
#include <drm/drmP.h>
#endif

#include "drm_realtek_drv.h"
#include "drm_realtek_gem.h"
#include "rtk_drm.h"

#include <rtk_kdriver/gal/rtk_gdma_export.h>


#define DRIVER_NAME "rtk_drm"
#define DRIVER_DESC "Realtek DRM Display Driver"
#define DRIVER_DATE "2022/05/23"

#if defined(RTK_DRM_USE_ATOMIC)
#include <drm/drm_atomic_helper.h>

#define RTK_DRIVER_ATOMIC DRIVER_ATOMIC
#else
#define RTK_DRIVER_ATOMIC 0
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#define	RTK_DRIVER_PRIME 0
#else
#define	RTK_DRIVER_PRIME DRIVER_PRIME
#endif

/* This header must always be included last */
#include "kernel_compatibility.h"

static bool display_enable = true;
static unsigned int output_device = 1;

void GDMA_RegisterIrqCallBackFunc(void (*fp)(void *), void *);

module_param(display_enable, bool, 0444);
MODULE_PARM_DESC(display_enable, "Enable all displays (default: Y)");

module_param(output_device, uint, 0444);
MODULE_PARM_DESC(output_device, "RTK DRM output device (default: GDMA)");

void rtk_drm_irq_handler(void *data)
{
	struct drm_device *dev = data;
	struct drm_crtc *crtc;

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head)
		rtk_drm_crtc_irq_handler(crtc);
}

static int rtk_drm_early_load(struct drm_device *dev)
{
	struct rtk_drm_private *dev_priv;
	int err;

	DRM_DEBUG("loading %s device\n", to_platform_device(dev->dev)->name);

	platform_set_drvdata(to_platform_device(dev->dev), dev);

	dev_priv = kzalloc(sizeof(*dev_priv), GFP_KERNEL);
	if (!dev_priv)
		return -ENOMEM;

	dev->dev_private = dev_priv;
	dev_priv->dev = dev;
	dev_priv->version = (enum rtk_drm_version)to_platform_device(dev->dev)->id_entry->driver_data;
	dev_priv->display_enabled = display_enable;

	dev_priv->outdev = (enum rtk_drm_output_device)output_device;

	dev_priv->fbc_capable = true;

	dev_priv->gem_priv = rtk_drm_gem_init(dev);
	if (!dev_priv->gem_priv) {
		DRM_ERROR("gem initialisation failed\n");
		err = -ENOMEM;
		goto err_dev_priv_free;
	}

	err = rtk_drm_modeset_early_init(dev_priv);
	if (err) {
		DRM_ERROR("early modeset initialisation failed (err=%d)\n",
			  err);
		goto err_gem_cleanup;
	}

	err = drm_vblank_init(dev_priv->dev, 1);
	if (err) {
		DRM_ERROR("failed to complete vblank init (err=%d)\n", err);
		goto err_modeset_late_cleanup;
	}

	switch (dev_priv->version) {
	case RTK_DRM_VERSION_GDMA_ADAPTER:
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0)) || IS_ENABLED(CONFIG_DRM_LEGACY)
		dev->irq_enabled = true;
	#endif
		GDMA_RegisterIrqCallBackFunc(rtk_drm_irq_handler, (void *)dev);
		break;
	default:		
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0)) || IS_ENABLED(CONFIG_DRM_LEGACY)
		dev->irq_enabled = false;
#endif
		break;
	}


#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0))
	dev->vblank_disable_allowed = 1;
#endif

	return 0;

err_modeset_late_cleanup:
	rtk_drm_modeset_late_cleanup(dev_priv);
err_gem_cleanup:
	rtk_drm_gem_cleanup(dev_priv->gem_priv);
err_dev_priv_free:
	kfree(dev_priv);
	return err;
}

static int rtk_drm_late_load(struct drm_device *dev)
{
	struct rtk_drm_private *dev_priv = dev->dev_private;
	int err;

	err = rtk_drm_modeset_late_init(dev_priv);
	if (err) {
		DRM_ERROR("late modeset initialisation failed (err=%d)\n",
			  err);
		return err;
	}

	return 0;
}

static void rtk_drm_early_unload(struct drm_device *dev)
{
	struct rtk_drm_private *dev_priv = dev->dev_private;

#if defined(CONFIG_DRM_FBDEV_EMULATION) && defined(RTK_DRM_USE_ATOMIC)
	drm_atomic_helper_shutdown(dev);
#endif
	switch (dev_priv->version) {
	case RTK_DRM_VERSION_GDMA_ADAPTER:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0)) || IS_ENABLED(CONFIG_DRM_LEGACY)
		dev->irq_enabled = false;
#endif
		GDMA_RegisterIrqCallBackFunc(NULL, NULL);
		break;
	default:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0)) || IS_ENABLED(CONFIG_DRM_LEGACY)
		dev->irq_enabled = false;
#endif
		break;
	}

	rtk_drm_modeset_early_cleanup(dev_priv);
}

static void rtk_drm_late_unload(struct drm_device *dev)
{
	struct rtk_drm_private *dev_priv = dev->dev_private;

	DRM_INFO("unloading %s device.\n", to_platform_device(dev->dev)->name);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	/* Called by drm_dev_fini in Linux 4.11.0 and later */
	drm_vblank_cleanup(dev_priv->dev);
#endif
	rtk_drm_modeset_late_cleanup(dev_priv);
	rtk_drm_gem_cleanup(dev_priv->gem_priv);

	kfree(dev_priv);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0))
static int rtk_drm_load(struct drm_device *dev, unsigned long flags)
{
	int err;

	err = rtk_drm_early_load(dev);
	if (err)
		return err;

	err = rtk_drm_late_load(dev);
	if (err) {
		rtk_drm_late_unload(dev);
		return err;
	}

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
static int rtk_drm_unload(struct drm_device *dev)
#else
static void rtk_drm_unload(struct drm_device *dev)
#endif
{
	rtk_drm_early_unload(dev);
	rtk_drm_late_unload(dev);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
	return 0;
#endif
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0))
static void rtk_drm_preclose(struct drm_device *dev, struct drm_file *file)
{
	struct drm_crtc *crtc;

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head)
		rtk_drm_crtc_flip_event_cancel(crtc, file);
}
#endif

#if !defined(CONFIG_DRM_FBDEV_EMULATION)
static inline void rtk_drm_teardown_drm_config(struct drm_device *dev)
{
#if defined(RTK_DRM_USE_ATOMIC)
	drm_atomic_helper_shutdown(dev);
#else
	struct drm_crtc *crtc;

	DRM_INFO("%s: %s device\n", __func__, to_platform_device(dev->dev)->name);

	/*
	 * When non atomic driver is in use, manually trigger ->set_config
	 * with an empty mode set associated to this crtc.
	 */
	drm_modeset_lock_all(dev);
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		if (crtc->primary->fb) {
			struct drm_mode_set mode_set = { .crtc = crtc };
			int err;

			err = drm_mode_set_config_internal(&mode_set);
			if (err)
				DRM_ERROR("failed to disable crtc %p (err=%d)\n",
					  crtc, err);
		}
	}
	drm_modeset_unlock_all(dev);
#endif
}
#endif /* !defined(CONFIG_DRM_FBDEV_EMULATION) */

static void rtk_drm_lastclose(struct drm_device *dev)
{
#if defined(CONFIG_DRM_FBDEV_EMULATION)
	struct rtk_drm_private *dev_priv = dev->dev_private;
	struct rtk_drm_fbdev *fbdev = dev_priv->fbdev;
	int err;

	if (fbdev) {
		/*
		 * This is a fbdev driver, therefore never attempt to shutdown
		 * on a client disconnecting.
		 */
		err = drm_fb_helper_restore_fbdev_mode_unlocked(&fbdev->helper);
		if (err)
			DRM_ERROR("failed to restore mode (err=%d)\n", err);
	}
#else
	rtk_drm_teardown_drm_config(dev);
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
int rtk_drm_enable_vblank(struct drm_crtc *crtc)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
static int rtk_drm_enable_vblank(struct drm_device *dev, unsigned int pipe)
#else
static int rtk_drm_enable_vblank(struct drm_device *dev, int pipe)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
	struct drm_device *dev = crtc->dev;
	unsigned int pipe      = drm_crtc_index(crtc);
#endif
	struct rtk_drm_private *dev_priv = dev->dev_private;

	switch (pipe) {
	case 0:
		rtk_drm_crtc_set_vblank_enabled(dev_priv->crtc, true);
		break;
	default:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
		DRM_ERROR("invalid crtc %u\n", pipe);
#else
		DRM_ERROR("invalid crtc %d\n", pipe);
#endif
		return -EINVAL;
	}

	DRM_DEBUG("vblank interrupts enabled for crtc %d\n", pipe);

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
void rtk_drm_disable_vblank(struct drm_crtc *crtc)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
static void rtk_drm_disable_vblank(struct drm_device *dev, unsigned int pipe)
#else
static void rtk_drm_disable_vblank(struct drm_device *dev, int pipe)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
	struct drm_device *dev = crtc->dev;
	unsigned int pipe      = drm_crtc_index(crtc);
#endif
	struct rtk_drm_private *dev_priv = dev->dev_private;

	switch (pipe) {
	case 0:
		rtk_drm_crtc_set_vblank_enabled(dev_priv->crtc, false);
		break;
	default:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
		DRM_ERROR("invalid crtc %u\n", pipe);
#else
		DRM_ERROR("invalid crtc %d\n", pipe);
#endif
		return;
	}

	DRM_DEBUG("vblank interrupts disabled for crtc %d\n", pipe);
}

static int rtk_drm_gem_mmap(struct file *file, struct vm_area_struct *vma)
{
	int err;

	err = drm_gem_mmap(file, vma);
	if (!err) {
		struct drm_file *file_priv = file->private_data;
		struct drm_device *dev = file_priv->minor->dev;
		struct drm_gem_object *obj;

		mutex_lock(&dev->struct_mutex);
		obj = vma->vm_private_data;

		if (obj->import_attach)
			err = dma_buf_mmap(obj->dma_buf, vma, 0);
		else
			err = rtk_drm_gem_object_get_pages(obj);

		mutex_unlock(&dev->struct_mutex);
		vma->vm_flags &= ~VM_PFNMAP;
		vma->vm_pgoff = 0;
	}
	return err;
}


static const struct drm_ioctl_desc rtk_drm_ioctls[] = {
	DRM_IOCTL_DEF_DRV(RTK_DRM_GEM_CREATE, rtk_drm_gem_object_create_ioctl,
				DRM_AUTH | DRM_UNLOCKED | DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(RTK_DRM_GEM_MMAP, rtk_drm_gem_object_mmap_ioctl,
				DRM_AUTH | DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RTK_DRM_GEM_CPU_PREP, rtk_drm_gem_object_cpu_prep_ioctl,
				DRM_AUTH | DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RTK_DRM_GEM_CPU_FINI, rtk_drm_gem_object_cpu_fini_ioctl,
				DRM_AUTH | DRM_UNLOCKED),
};

static const struct file_operations rtk_drm_driver_fops = {
	.owner		= THIS_MODULE,
	.open		= drm_open,
	.release	= drm_release,
	.unlocked_ioctl	= drm_ioctl,
	.mmap		= rtk_drm_gem_mmap,
	.poll		= drm_poll,
	.read		= drm_read,
	.llseek		= noop_llseek,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= drm_compat_ioctl,
#endif
};

static struct drm_driver rtk_drm_driver = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	.load				= NULL,
	.unload				= NULL,
#else
	.load				= rtk_drm_load,
	.unload				= rtk_drm_unload,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0))
	.preclose			= rtk_drm_preclose,
#endif
	.lastclose			= rtk_drm_lastclose,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)) && \
	(LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))
	.set_busid			= drm_platform_set_busid,
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0))
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
	.get_vblank_counter		= NULL,
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	.get_vblank_counter		= drm_vblank_no_hw_counter,
#else
	.get_vblank_counter		= drm_vblank_count,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0))
	.enable_vblank			= rtk_drm_enable_vblank,
	.disable_vblank			= rtk_drm_disable_vblank,
#endif

	.debugfs_init			= rtk_drm_debugfs_init,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
	.debugfs_cleanup		= rtk_drm_debugfs_cleanup,
#endif


	.prime_handle_to_fd		= drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle		= drm_gem_prime_fd_to_handle,


	.gem_prime_mmap 		= rtk_drm_gem_prime_mmap,
	.gem_prime_import		= rtk_drm_gem_prime_import,
	.gem_prime_import_sg_table	= rtk_drm_gem_prime_import_sg_table,

    // Set dumb_create to NULL to avoid xorg owning the display (if xorg is running).
	.dumb_create			= rtk_drm_gem_dumb_create,
	.dumb_map_offset		= rtk_drm_gem_dumb_map_offset,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0))
	.dumb_destroy			= rtk_drm_gem_dumb_destroy,
#else
	.dumb_destroy			= drm_gem_dumb_destroy,
#endif

	.name				= DRIVER_NAME,
	.desc				= DRIVER_DESC,
	.date				= DRIVER_DATE,
	.major				= 1,
	.minor				= 1,
	.patchlevel			= 1,

#ifdef RTK_DRM_USE_ATOMIC
	.driver_features	= DRIVER_GEM | DRIVER_MODESET | DRIVER_ATOMIC,
#else
	.driver_features	= DRIVER_GEM | DRIVER_MODESET,
#endif
	.ioctls				= rtk_drm_ioctls,
	.num_ioctls			= ARRAY_SIZE(rtk_drm_ioctls),
	.fops				= &rtk_drm_driver_fops,
};

static int rtk_drm_probe(struct platform_device *pdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	struct drm_device *ddev;
	int ret;

	ddev = drm_dev_alloc(&rtk_drm_driver, &pdev->dev);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	if (IS_ERR(ddev))
		return PTR_ERR(ddev);
#else
	if (!ddev)
		return -ENOMEM;
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))
	/* Needed by drm_platform_set_busid */
	ddev->platformdev = pdev;
#endif
	/*
	 * The load callback, called from drm_dev_register, is deprecated,
	 * because of potential race conditions.
	 */
	BUG_ON(rtk_drm_driver.load != NULL);

	ret = rtk_drm_early_load(ddev);
	if (ret)
		goto err_drm_dev_put;

	ret = drm_dev_register(ddev, 0);
	if (ret)
		goto err_drm_dev_late_unload;

	ret = rtk_drm_late_load(ddev);
	if (ret)
		goto err_drm_dev_unregister;

	printk(KERN_EMERG"Initialized %s %d.%d.%d %s on minor %d\n",
		rtk_drm_driver.name,
		rtk_drm_driver.major,
		rtk_drm_driver.minor,
		rtk_drm_driver.patchlevel,
		rtk_drm_driver.date,
		ddev->primary->index);
	return 0;

err_drm_dev_unregister:
	drm_dev_unregister(ddev);
err_drm_dev_late_unload:
	rtk_drm_late_unload(ddev);
err_drm_dev_put:
	drm_dev_put(ddev);
	
	return	ret;
#else
	return drm_platform_init(&rtk_drm_driver, pdev);
#endif
}

static int rtk_drm_remove(struct platform_device *pdev)
{
	struct drm_device *ddev = platform_get_drvdata(pdev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	/*
	 * The unload callback, called from drm_dev_unregister, is
	 * deprecated.
	 */
	BUG_ON(rtk_drm_driver.unload != NULL);

	rtk_drm_early_unload(ddev);

	drm_dev_unregister(ddev);

	rtk_drm_late_unload(ddev);

	drm_dev_put(ddev);
#else
	drm_put_dev(ddev);
#endif
	return 0;
}

static void rtk_drm_shutdown(struct platform_device *pdev)
{
}

static struct platform_device_id rtk_drm_platform_device_id_table[] = {
	{ .name = "rtk_drm", .driver_data = RTK_DRM_VERSION_GDMA_ADAPTER },
	{ },
};

static struct platform_driver rtk_drm_platform_driver = {
	.probe		= rtk_drm_probe,
	.remove		= rtk_drm_remove,
	.shutdown	= rtk_drm_shutdown,
	.driver		= {
		.owner  = THIS_MODULE,
		.name	= DRIVER_NAME,
	},
	.id_table	= rtk_drm_platform_device_id_table,
};

static struct platform_device_info rtk_drm_device_info = {
	.name		= "rtk_drm",
	.id		= -1,
	.dma_mask	= DMA_BIT_MASK(32),
};

static struct platform_device *rtk_drm_dev;

static int __init rtk_drm_init(void)
{
	int err = 0;


	rtk_drm_dev = platform_device_register_full(&rtk_drm_device_info);
	if (IS_ERR(rtk_drm_dev)) {
		err = PTR_ERR(rtk_drm_dev);
		rtk_drm_dev = NULL;
		return err;
	}

	err = platform_driver_register(&rtk_drm_platform_driver);

	return err;

}

static void __exit rtk_drm_exit(void)
{
	if (rtk_drm_dev)
		platform_device_unregister(rtk_drm_dev);

	platform_driver_unregister(&rtk_drm_platform_driver);
}


module_init(rtk_drm_init);
module_exit(rtk_drm_exit);


MODULE_AUTHOR("Realtek");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
