
#include <linux/version.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0))
#include <drm/drmP.h>
#endif

#include <drm/drm_crtc.h>

#include "drm_realtek_drv.h"

#include "kernel_compatibility.h"

static void rtk_drm_encoder_helper_dpms(struct drm_encoder *encoder, int mode)
{
}

static bool
rtk_drm_encoder_helper_mode_fixup(struct drm_encoder *encoder,
				   const struct drm_display_mode *mode,
				   struct drm_display_mode *adjusted_mode)
{
	return true;
}

static void rtk_drm_encoder_helper_prepare(struct drm_encoder *encoder)
{
}

static void rtk_drm_encoder_helper_commit(struct drm_encoder *encoder)
{
}

static void
rtk_drm_encoder_helper_mode_set(struct drm_encoder *encoder,
				 struct drm_display_mode *mode,
				 struct drm_display_mode *adjusted_mode)
{
}

static void rtk_drm_encoder_destroy(struct drm_encoder *encoder)
{
	struct rtk_drm_private *dev_priv = encoder->dev->dev_private;

	DRM_DEBUG_DRIVER("[ENCODER:%d:%s]\n",
			 encoder->base.id,
			 encoder->name);

	drm_encoder_cleanup(encoder);

	kfree(encoder);
	dev_priv->encoder = NULL;
}

static const struct drm_encoder_helper_funcs rtk_drm_encoder_helper_funcs = {
	.dpms = rtk_drm_encoder_helper_dpms,
	.mode_fixup = rtk_drm_encoder_helper_mode_fixup,
	.prepare = rtk_drm_encoder_helper_prepare,
	.commit = rtk_drm_encoder_helper_commit,
	.mode_set = rtk_drm_encoder_helper_mode_set,
	.detect = NULL,
	.disable = NULL,
};

static const struct drm_encoder_funcs rtk_drm_encoder_funcs = {
	.reset = NULL,
	.destroy = rtk_drm_encoder_destroy,
};

struct drm_encoder *
rtk_drm_encoder_create(struct drm_device *dev)
{
	struct drm_encoder *encoder;
	int err;

	encoder = kzalloc(sizeof(*encoder), GFP_KERNEL);
	if (!encoder)
		return ERR_PTR(-ENOMEM);

	err = drm_encoder_init(dev,
			       encoder,
			       &rtk_drm_encoder_funcs,
			       DRM_MODE_ENCODER_TMDS,
			       NULL);
	if (err) {
		DRM_ERROR("Failed to initialise encoder");
		return ERR_PTR(err);
	}
	drm_encoder_helper_add(encoder, &rtk_drm_encoder_helper_funcs);

	/*
	 * This is a bit field that's used to determine which
	 * CRTCs can drive this encoder.
	 */
	encoder->possible_crtcs = 0x1;

	DRM_DEBUG_DRIVER("[ENCODER:%d:%s]\n",
			 encoder->base.id,
			 encoder->name);

	return encoder;
}
