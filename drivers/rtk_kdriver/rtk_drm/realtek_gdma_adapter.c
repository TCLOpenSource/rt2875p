#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include "realtek_gdma_adapter.h"

#include <rbus/gdma_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rtk_kdriver/gal/rtk_gdma_export.h>
#include <rtk_kdriver/gal/rtk_gdma_export_user.h>
#include <rtk_kdriver/io.h>
#include "rtk_drm_fourcc.h"

static PICTURE_LAYERS_OBJECT gdma_layers_obj_atomic;
static int gdma_layer_index = 0;

int GDMA_ReceivePictureNoBlock(PICTURE_LAYERS_OBJECT *data);

bool rtk_drm_gdma_clocks_set(struct device *dev, 
			 u32 clock_freq, u32 dev_num,
			 u32 hdisplay, u32 vdisplay)
{
	return true;
}

void rtk_drm_gdma_set_updates_enabled(struct device *dev, bool enable)
{

}

void rtk_drm_gdma_set_syncgen_enabled(struct device *dev, bool enable)
{

}

void rtk_drm_gdma_set_powerdwn_enabled(struct device *dev, bool enable)
{

}

void rtk_drm_gdma_set_vblank_enabled(struct device *dev, bool enable)
{
	if(enable)
		rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vact_end(1) | GDMA_OSD_INTEN_osd1_vsync(1));
	else
		rtd_outl(GDMA_OSD_INTST_reg, ~1); ;
}

bool rtk_drm_gdma_check_and_clear_vblank(struct device *dev)
{
	return true;
}

void rtk_drm_gdma_set_plane_enabled(struct device *dev,  u32 plane, bool enable)
{
	GDMA_DISPLAY_PLANE gdma_plane;
	gdma_plane = GDMA_PLANE_OSD1 + plane;
	printk(KERN_EMERG"[rtk][drm] %s %d: plane %u enable:%d",__func__,__LINE__, plane, enable);
	GDMA_ConfigOSDxEnableFast(gdma_plane, enable);
}

void rtk_drm_gdma_reset_planes(struct device *dev )
{
}

void rtk_drm_gdma_set_surface(struct device *dev,
			  u32 plane, u32 address, u32 offset,
			  u32 posx, u32 posy,
			  u32 width, u32 height, u32 stride,
			  u32 format, u64 modifier, u32 alpha, bool blend)
{
	GDMA_PICTURE_OBJECT *picObj;
	static PICTURE_LAYERS_OBJECT *picLayerObj = NULL;
#if defined(CONFIG_CUSTOMER_TV006)
	u32 compressred_ratio;
#endif

	if (picLayerObj == NULL) {
		picLayerObj = (PICTURE_LAYERS_OBJECT *)kmalloc(sizeof(PICTURE_LAYERS_OBJECT), GFP_KERNEL);
		if (picLayerObj == NULL) {
			pr_err(KERN_EMERG"%s %d kmalloc failed \n", __FUNCTION__,__LINE__);
			return;
		}
	}

	memset(picLayerObj, 0, sizeof(PICTURE_LAYERS_OBJECT));
	
	if(modifier != DRM_FORMAT_MOD_LINEAR)
	{
		picLayerObj->layer_num = 1;
		picLayerObj->layer[0].fbdc_num = 1;

		picObj = &picLayerObj->layer[0].fbdc[0];
	}else{
		picLayerObj->layer_num = 1;
		picLayerObj->layer[0].normal_num = 1;
		
		picObj = &picLayerObj->layer[0].normal[0];
	}
		
	picObj->alpha = alpha;
	picObj->clear_x.value = 0;
	picObj->clear_y.value = 0;
	picObj->colorkey = -1;
	picObj->key_en = 0;

	if(modifier != DRM_FORMAT_MOD_LINEAR)
	{
	#if defined(CONFIG_CUSTOMER_TV006)
		picObj->compressed = 1;
		picObj->fbdc_format = FBDC_COLOR_FORMAT_U8U8U8U8;
		picObj->src_type = SRC_FBDC;
		switch(modifier){
			case DRM_FORMAT_MOD_PVR_FBCDC_8x8_V13:
			case DRM_FORMAT_MOD_PVR_FBCDC_16x4_V13:
				compressred_ratio = 0;
				break;
			case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY75_8x8_V13:
			case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY75_16x4_V13:
				compressred_ratio = 1;
				break;
			case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY50_8x8_V13:
			case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY50_16x4_V13:
				compressred_ratio = 2;
				break;
			case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY25_8x8_V13:
			case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY25_16x4_V13:
				compressred_ratio = 3;
				break;
			default:
				DRM_ERROR("Unsupported modifier:0x%llu\n", modifier);
				break;
		}
		picObj->compressed_ratio = compressred_ratio;
	#endif
	}else{
		picObj->compressed = 0;
		picObj->src_type = SRC_NORMAL;
		switch(format){
		case DRM_FORMAT_RGBA8888:
		case DRM_FORMAT_RGBX8888:
			picObj->format = VO_OSD_COLOR_FORMAT_RGBA8888;
			picObj->rgb_order = VO_OSD_COLOR_RGB;
			break;

		case DRM_FORMAT_BGRA8888:
		case DRM_FORMAT_BGRX8888:
			picObj->format = VO_OSD_COLOR_FORMAT_RGBA8888;
			picObj->rgb_order = VO_OSD_COLOR_BGR;
			break;

		case DRM_FORMAT_ARGB8888:
		case DRM_FORMAT_XRGB8888:
			picObj->format = VO_OSD_COLOR_FORMAT_ARGB8888;
			picObj->rgb_order = VO_OSD_COLOR_RGB;
			break;

		case DRM_FORMAT_ABGR8888:
		case DRM_FORMAT_XBGR8888:	
			picObj->format = VO_OSD_COLOR_FORMAT_ARGB8888;
			picObj->rgb_order = VO_OSD_COLOR_BGR;
			break;

		case DRM_FORMAT_BGR565:
			picObj->format = VO_OSD_COLOR_FORMAT_RGB565;
			picObj->rgb_order = VO_OSD_COLOR_BGR;
			break;

		case DRM_FORMAT_RGB565:
			picObj->format = VO_OSD_COLOR_FORMAT_RGB565;
			picObj->rgb_order = VO_OSD_COLOR_RGB;
			break;
		default:
			printk(KERN_EMERG"%s %d format 0x%x not support yet!\n",__func__, __LINE__, format);
			return;
		}
	}

	picObj->dst_height = GDMA_MAX_HEIGHT;
	picObj->dst_width = GDMA_MAX_WIDTH;
	picObj->dst_x = 0;
	picObj->dst_y = 0;

	picObj->layer_used = 1;
	picObj->pitch = stride;

	picObj->plane = VO_GRAPHIC_OSD1;
	picObj->plane_ar.plane_alpha_r = 0xff;
	picObj->plane_ar.plane_alpha_a = 0xff;
	picObj->plane_gb.plane_alpha_g = 0xff;
	picObj->plane_gb.plane_alpha_b = 0xff;
	picObj->show = 1;
	picObj->syncInfo.syncstamp = 0;
	//picObj->scale_factor = 1;
	picObj->width = width;
	picObj->height = height;
	picObj->x = posx;
	picObj->y = posy;
	picObj->address = address;
	picObj->offlineOrder = C0;
	picLayerObj->layer[0].onlineOrder = C0;

	GDMA_ReceivePictureNoBlock(picLayerObj);

}

void rtk_drm_gdma_set_surface_atomic(struct device *dev,
			u32 plane, u32 address, u32 offset,
			int32_t crtcx, int32_t crtcy, u32 crtcw, u32 crtch,
			u32 srcx, u32 srcy, u32 srcw, u32 srch, u32 stride,
			u32 format, u64 modifier, u32 alpha, bool blend)
{
  GDMA_PICTURE_OBJECT *picObj;
  static PICTURE_LAYERS_OBJECT *picLayerObj = &gdma_layers_obj_atomic;
#if defined(CONFIG_CUSTOMER_TV006)
  u32 compressred_ratio;
#endif

  if(modifier != DRM_FORMAT_MOD_LINEAR)
  {
	  picLayerObj->layer[gdma_layer_index].fbdc_num = 1;
	  picObj = &picLayerObj->layer[gdma_layer_index].fbdc[0];
  }else{
	  picLayerObj->layer[gdma_layer_index].normal_num = 1;
	  picObj = &picLayerObj->layer[gdma_layer_index].normal[0];
  }
	  
  picObj->alpha = alpha;
  picObj->clear_x.value = 0;
  picObj->clear_y.value = 0;
  picObj->colorkey = -1;
  picObj->key_en = 0;

  if(modifier != DRM_FORMAT_MOD_LINEAR)
  {
#if defined(CONFIG_CUSTOMER_TV006)
	  picObj->compressed = 1;
	  picObj->fbdc_format = FBDC_COLOR_FORMAT_U8U8U8U8;
	  picObj->src_type = SRC_FBDC;
	  switch(modifier){
		  case DRM_FORMAT_MOD_PVR_FBCDC_8x8_V13:
		  case DRM_FORMAT_MOD_PVR_FBCDC_16x4_V13:
			  compressred_ratio = 0;
			  break;
		  case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY75_8x8_V13:
		  case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY75_16x4_V13:
			  compressred_ratio = 1;
			  break;
		  case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY50_8x8_V13:
		  case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY50_16x4_V13:
			  compressred_ratio = 2;
			  break;
		  case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY25_8x8_V13:
		  case DRM_FORMAT_MOD_PVR_FBCDC_LOSSY25_16x4_V13:
			  compressred_ratio = 3;
			  break;
		  default:
			  DRM_ERROR("Unsupported modifier:0x%llu\n", modifier);
			  break;
	  }
	  picObj->compressed_ratio = compressred_ratio;
#endif
  }else{
	  picObj->compressed = 0;
	  picObj->src_type = SRC_NORMAL;
	  switch(format){
	  case DRM_FORMAT_RGBA8888:
	  case DRM_FORMAT_RGBX8888:
		  picObj->format = VO_OSD_COLOR_FORMAT_RGBA8888;
		  picObj->rgb_order = VO_OSD_COLOR_RGB;
		  break;

	  case DRM_FORMAT_BGRA8888:
	  case DRM_FORMAT_BGRX8888:
		  picObj->format = VO_OSD_COLOR_FORMAT_RGBA8888;
		  picObj->rgb_order = VO_OSD_COLOR_BGR;
		  break;

	  case DRM_FORMAT_ARGB8888:
	  case DRM_FORMAT_XRGB8888:
		  picObj->format = VO_OSD_COLOR_FORMAT_ARGB8888;
		  picObj->rgb_order = VO_OSD_COLOR_RGB;
		  break;

	  case DRM_FORMAT_ABGR8888:
	  case DRM_FORMAT_XBGR8888:
		  picObj->format = VO_OSD_COLOR_FORMAT_ARGB8888;
		  picObj->rgb_order = VO_OSD_COLOR_BGR;
		  break;
      case DRM_FORMAT_BGR565:
          picObj->format = VO_OSD_COLOR_FORMAT_RGB565;
          picObj->rgb_order = VO_OSD_COLOR_BGR;
          break;

      case DRM_FORMAT_RGB565:
          picObj->format = VO_OSD_COLOR_FORMAT_RGB565;
          picObj->rgb_order = VO_OSD_COLOR_RGB;
          break;

	  default:
		  printk(KERN_EMERG"%s %d format 0x%x not support yet!\n",__func__, __LINE__, format);
		  return;
	  }
  }

  picObj->dst_height = crtch;
  picObj->dst_width = crtcw;
  picObj->dst_x = crtcx;
  picObj->dst_y = crtcy;
  picObj->layer_used = 1;
  picObj->pitch = stride;

  picObj->plane = VO_GRAPHIC_OSD1 + plane;
  picObj->plane_ar.plane_alpha_r = 0xff;
  picObj->plane_ar.plane_alpha_a = 0xff;
  picObj->plane_gb.plane_alpha_g = 0xff;
  picObj->plane_gb.plane_alpha_b = 0xff;
  picObj->show = 1;
  picObj->syncInfo.syncstamp = 0;
  //picObj->scale_factor = 1;
  picObj->width = srcw;
  picObj->height = srch;
  picObj->x = srcx;
  picObj->y = srcy;
  picObj->address = address;
  picObj->offlineOrder = C0 + plane;
  picLayerObj->layer[gdma_layer_index].onlineOrder = C0 + plane;
  
  gdma_layer_index ++;


}

void rtk_drm_gdma_atomic_begin(void)
{
	gdma_layer_index = 0;
	memset(&gdma_layers_obj_atomic, 0, sizeof(PICTURE_LAYERS_OBJECT));
}

void rtk_drm_gdma_atomic_flush(int plane_num)
{
	if(plane_num){
		gdma_layers_obj_atomic.layer_num = plane_num;
		GDMA_ReceivePictureNoBlock(&gdma_layers_obj_atomic);
	}
	gdma_layer_index = 0;
	memset(&gdma_layers_obj_atomic, 0, sizeof(PICTURE_LAYERS_OBJECT));
}

void rtk_drm_gdma_mode_set(struct device *dev, 
		       u32 h_display, u32 v_display,
		       u32 hbps, u32 ht, u32 has,
		       u32 hlbs, u32 hfps, u32 hrbs,
		       u32 vbps, u32 vt, u32 vas,
		       u32 vtbs, u32 vfps, u32 vbbs,
		       bool nhsync, bool nvsync )
{

}

void rtk_drm_get_screen_size(u32 * hdisplay, u32 * vdisplay)
{
	ppoverlay_memcdtg_dh_den_start_end_RBUS dtg_dh;
	ppoverlay_memcdtg_dv_den_start_end_RBUS dtg_dv;
	unsigned int actvie_h = 0;
	unsigned int actvie_v = 0;

	dtg_dh.regValue = rtd_inl(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg);
	dtg_dv.regValue = rtd_inl(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg);

	actvie_h = dtg_dh.memcdtg_dh_den_end-dtg_dh.memcdtg_dh_den_sta;
	actvie_v = dtg_dv.memcdtg_dv_den_end-dtg_dv.memcdtg_dv_den_sta;

	if ((actvie_h > 3800) && (actvie_h <= 3850))
		actvie_h = 3840;
	else if ((actvie_h > 1900) && (actvie_h <= 1930))
		actvie_h = 1920;

	if ((actvie_v > 2100) && (actvie_v <= 2170))
		actvie_v = 2160;
	else if ((actvie_v > 1000) && (actvie_v <= 1090))
		actvie_v = 1080;

	if(actvie_h == 0 || actvie_v ==0) {
		actvie_h = 1920;
		actvie_v = 1080;
		printk(KERN_EMERG"[rtk][drm] %s %d: forced to set Disp 2k1k.\n",__func__,__LINE__);
	}

	printk(KERN_EMERG"[rtk][drm] %s %d: Disp %u x %u.\n",__func__,__LINE__,actvie_h, actvie_v);
	*hdisplay = actvie_h;
	*vdisplay = actvie_v;

	return;

}
