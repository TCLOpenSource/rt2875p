
#include "drm_realtek_drv.h"

#include <linux/moduleparam.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0))
#else
#include <drm/drmP.h>
#endif

#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>

#if defined(RTK_DRM_USE_ATOMIC)
#include <drm/drm_atomic_helper.h>
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0))
#include <drm/drm_probe_helper.h>
#endif

#include "kernel_compatibility.h"

struct rtk_drm_mode_data {
	int hdisplay;
	int vdisplay;
	int vrefresh;
	bool reduced_blanking;
	bool interlaced;
	bool margins;
};

static const struct rtk_drm_mode_data rtk_drm_extra_modes[] = {
	{
		.hdisplay = 3840,
		.vdisplay = 2160,
		.vrefresh = 60,
		.reduced_blanking = false,
		.interlaced = false,
		.margins = false,
	},
	{
		.hdisplay = 1920,
		.vdisplay = 1080,
		.vrefresh = 60,
		.reduced_blanking = false,
		.interlaced = false,
		.margins = false,
	},
	{
		.hdisplay = 1366,
		.vdisplay = 768,
		.vrefresh = 60,
		.reduced_blanking = false,
		.interlaced = false,
		.margins = false,
	},
};

static char preferred_mode_name[DRM_DISPLAY_MODE_LEN] = "\0";

module_param_string(rtk_drm_preferred_mode,
		    preferred_mode_name,
		    DRM_DISPLAY_MODE_LEN,
		    0444);

MODULE_PARM_DESC(rtk_drm_preferred_mode,
		 "Specify the preferred mode (if supported), e.g. 1280x1024.");


static int rtk_drm_add_extra_modes(struct drm_connector *connector)
{
	struct drm_display_mode *mode;
	int num_modes;
	int i;

	for (i = 0, num_modes = 0; i < ARRAY_SIZE(rtk_drm_extra_modes); i++) {
		mode = drm_cvt_mode(connector->dev,
				    rtk_drm_extra_modes[i].hdisplay,
				    rtk_drm_extra_modes[i].vdisplay,
				    rtk_drm_extra_modes[i].vrefresh,
				    rtk_drm_extra_modes[i].reduced_blanking,
				    rtk_drm_extra_modes[i].interlaced,
				    rtk_drm_extra_modes[i].margins);
		if (mode) {
			drm_mode_probed_add(connector, mode);
			num_modes++;
		}
	}

	return num_modes;
}

static int rtk_drm_connector_helper_get_modes(struct drm_connector *connector)
{
	struct drm_device *dev = connector->dev;
	int num_modes;
	int len = strlen(preferred_mode_name);

	if (len)
		dev_info(dev->dev, "detected rtk_drm_preferred_mode=%s\n",
					preferred_mode_name);
	else
		dev_info(dev->dev, "no rtk_drm_preferred_mode\n");

	num_modes = drm_add_modes_noedid(connector,
					 dev->mode_config.max_width,
					 dev->mode_config.max_height);

	num_modes += rtk_drm_add_extra_modes(connector);
	if (num_modes) {
		struct drm_display_mode *pref_mode = NULL;

		if (len) {
			struct drm_display_mode *mode;
			struct list_head *entry;

			list_for_each(entry, &connector->probed_modes) {
				mode = list_entry(entry,
						  struct drm_display_mode,
						  head);
				if (!strcmp(mode->name, preferred_mode_name)) {
					pref_mode = mode;
					break;
				}
			}
		}

		if (pref_mode)
			pref_mode->type |= DRM_MODE_TYPE_PREFERRED;
		else
			drm_set_preferred_mode(connector,
					       dev->mode_config.max_width,
					       dev->mode_config.max_height);
	}

	drm_mode_sort(&connector->probed_modes);

	DRM_DEBUG_DRIVER("[CONNECTOR:%d:%s] found %d modes\n",
			 connector->base.id,
			 connector->name,
			 num_modes);

	return num_modes;
}

static enum drm_mode_status
rtk_drm_connector_helper_mode_valid(struct drm_connector *connector,
				    struct drm_display_mode *mode)
{
	if (mode->flags & DRM_MODE_FLAG_INTERLACE)
		return MODE_NO_INTERLACE;
	else if (mode->flags & DRM_MODE_FLAG_DBLSCAN)
		return MODE_NO_DBLESCAN;

	return MODE_OK;
}

#if !defined(RTK_DRM_USE_ATOMIC)
static struct drm_encoder *
rtk_drm_connector_helper_best_encoder(struct drm_connector *connector)
{
	/* Pick the first encoder we find */
	if (connector->encoder_ids[0] != 0) {
		struct drm_encoder *encoder;

		encoder = drm_encoder_find(connector->dev,
					   NULL,
					   connector->encoder_ids[0]);
		if (encoder) {
			DRM_DEBUG_DRIVER("[ENCODER:%d:%s] best for [CONNECTOR:%d:%s]\n",
					 encoder->base.id,
					 encoder->name,
					 connector->base.id,
					 connector->name);
			return encoder;
		}
	}

	return NULL;
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
static enum drm_connector_status
rtk_drm_connector_detect(struct drm_connector *connector,
			 bool force)
{
	/*
	 * It appears that there is no way to determine if a monitor
	 * is connected. This needs to be set to connected otherwise
	 * DPMS never gets set to ON.
	 */
	return connector_status_connected;
}
#endif

static void rtk_drm_connector_destroy(struct drm_connector *connector)
{
	struct rtk_drm_private *dev_priv = connector->dev->dev_private;

	DRM_DEBUG_DRIVER("[CONNECTOR:%d:%s]\n",
			 connector->base.id,
			 connector->name);

	drm_connector_cleanup(connector);

	kfree(connector);
	dev_priv->connector = NULL;
}

static void rtk_drm_connector_force(struct drm_connector *connector)
{
}

static struct drm_connector_helper_funcs rtk_drm_connector_helper_funcs = {
	.get_modes = rtk_drm_connector_helper_get_modes,
	.mode_valid = rtk_drm_connector_helper_mode_valid,
	/*
	 * For atomic, don't set atomic_best_encoder or best_encoder. This will
	 * cause the DRM core to fallback to drm_atomic_helper_best_encoder().
	 * This is fine as we only have a single connector and encoder.
	 */
#if !defined(RTK_DRM_USE_ATOMIC)
	.best_encoder = rtk_drm_connector_helper_best_encoder,
#endif
};

static const struct drm_connector_funcs rtk_drm_connector_funcs = {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
	.detect = rtk_drm_connector_detect,
#endif
	.fill_modes = drm_helper_probe_single_connector_modes,
	.destroy = rtk_drm_connector_destroy,
	.force = rtk_drm_connector_force,
#if defined(RTK_DRM_USE_ATOMIC)
	.reset = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
#else
	.dpms = drm_helper_connector_dpms,
#endif
};


struct drm_connector *
rtk_drm_connector_create(struct drm_device *dev)
{
	struct drm_connector *connector;

	connector = kzalloc(sizeof(*connector), GFP_KERNEL);
	if (!connector)
		return ERR_PTR(-ENOMEM);

	drm_connector_init(dev,
			   connector,
			   &rtk_drm_connector_funcs,
			   DRM_MODE_CONNECTOR_TV);
	drm_connector_helper_add(connector, &rtk_drm_connector_helper_funcs);

	connector->dpms = DRM_MODE_DPMS_OFF;
	connector->interlace_allowed = false;
	connector->doublescan_allowed = false;
	connector->display_info.subpixel_order = SubPixelHorizontalRGB;

	DRM_DEBUG_DRIVER("[CONNECTOR:%d:%s]\n",
			 connector->base.id,
			 connector->name);

	return connector;
}
