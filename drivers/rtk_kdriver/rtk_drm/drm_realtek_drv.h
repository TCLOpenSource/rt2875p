
#if !defined(__DRM_PDP_DRV_H__)
#define __DRM_PDP_DRV_H__

#include <linux/version.h>
#include <linux/wait.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0))
#include <drm/drm_fourcc.h>
#else
#include <drm/drmP.h>
#endif

#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_mm.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
#include <drm/drm_plane.h>
#endif

#include "rtk_drm.h"

#define RTK_DRM_USE_ATOMIC
#define RTK_DRM_FBDEV_EMULATION_ENABLE 0


struct rtk_drm_gem_context;
enum rtk_drm_crtc_flip_status;
struct rtk_drm_flip_data;
struct rtk_drm_gem_private;

struct tc_realtek_platform_data;

struct rtk_drm_private {
	struct drm_device *dev;
#if defined(CONFIG_DRM_FBDEV_EMULATION)
	struct rtk_drm_fbdev *fbdev;
#endif

	enum rtk_drm_version version;

	/* created by rtk_drm_gem_init */
	struct rtk_drm_gem_private	*gem_priv;

	/* preferred output device */
	enum rtk_drm_output_device outdev;
	uint32_t interrupt;

	/* FBC Decompression module support  */
	bool fbc_capable;

	/* initialised by rtk_drm_modeset_early_init */
	struct drm_plane *plane;
	struct drm_plane * overlay_plane;
	struct drm_crtc *crtc;
	struct drm_connector *connector;
	struct drm_encoder *encoder;

	bool display_enabled;
};

struct rtk_drm_crtc {
	struct drm_crtc base;

	uint32_t number;

	wait_queue_head_t flip_pending_wait_queue;

	/* Reuse the drm_device event_lock to protect these */
	atomic_t flip_status;
	struct drm_pending_vblank_event *flip_event;
	struct drm_framebuffer *old_fb;
	struct rtk_drm_flip_data *flip_data;
	bool flip_async;
};

#define to_rtk_drm_crtc(crtc) container_of(crtc, struct rtk_drm_crtc, base)

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
struct drm_gem_object;

struct rtk_drm_framebuffer {
	struct drm_framebuffer base;
	struct drm_gem_object *obj[1];
};

#define to_rtk_drm_framebuffer(fb) container_of(fb, struct rtk_drm_framebuffer, base)
#define to_drm_framebuffer(fb) (&(fb)->base)
#else
#define rtk_drm_framebuffer drm_framebuffer
#define to_rtk_drm_framebuffer(fb) (fb)
#define to_drm_framebuffer(fb) (fb)
#endif

#if defined(CONFIG_DRM_FBDEV_EMULATION)
struct rtk_drm_fbdev {
	struct drm_fb_helper helper;
	struct rtk_drm_framebuffer fb;
	struct rtk_drm_private *priv;
	u8 preferred_bpp;
};
#endif

static inline u32 rtk_drm_fb_cpp(struct drm_framebuffer *fb)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
	return fb->format->cpp[0];
#else
	return fb->bits_per_pixel / 8;
#endif
}

static inline u32 rtk_drm_fb_format(struct drm_framebuffer *fb)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
	return fb->format->format;
#else
	return fb->pixel_format;
#endif
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0))
int rtk_drm_debugfs_init(struct drm_minor *minor);
#else
void rtk_drm_debugfs_init(struct drm_minor *minor);
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
void rtk_drm_debugfs_cleanup(struct drm_minor *minor);
#endif

struct drm_plane *rtk_drm_plane_create(struct drm_device *dev,
				   enum drm_plane_type type);
void rtk_drm_plane_destroy(struct drm_plane *plane);

void rtk_drm_plane_set_surface(struct drm_crtc *crtc, struct drm_plane *plane,
			   struct drm_framebuffer *fb,
			   const uint32_t src_x, const uint32_t src_y);

struct drm_crtc *rtk_drm_crtc_create(struct drm_device *dev, uint32_t number,
				 struct drm_plane *primary_plane);
void rtk_drm_crtc_set_plane_enabled(struct drm_crtc *crtc, bool enable);
void rtk_drm_crtc_set_vblank_enabled(struct drm_crtc *crtc, bool enable);
void rtk_drm_crtc_irq_handler(struct drm_crtc *crtc);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0))
void rtk_drm_crtc_flip_event_cancel(struct drm_crtc *crtc, struct drm_file *file);
#endif

struct drm_connector *rtk_drm_connector_create(struct drm_device *dev);

struct drm_encoder *rtk_drm_encoder_create(struct drm_device *dev);

int rtk_drm_modeset_early_init(struct rtk_drm_private *dev_priv);
int rtk_drm_modeset_late_init(struct rtk_drm_private *dev_priv);
void rtk_drm_modeset_early_cleanup(struct rtk_drm_private *dev_priv);
void rtk_drm_modeset_late_cleanup(struct rtk_drm_private *dev_priv);

#if defined(CONFIG_DRM_FBDEV_EMULATION)
struct rtk_drm_fbdev *rtk_drm_fbdev_create(struct rtk_drm_private *dev);
void rtk_drm_fbdev_destroy(struct rtk_drm_fbdev *fbdev);
#endif

int rtk_drm_modeset_validate_init(struct rtk_drm_private *dev_priv,
			      struct drm_mode_fb_cmd2 *mode_cmd,
			      struct rtk_drm_framebuffer *pdp_fb,
			      struct drm_gem_object *obj);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
int rtk_drm_enable_vblank(struct drm_crtc *crtc);
void rtk_drm_disable_vblank(struct drm_crtc *crtc);
#endif

#endif /* !defined(__DRM_PDP_DRV_H__) */
