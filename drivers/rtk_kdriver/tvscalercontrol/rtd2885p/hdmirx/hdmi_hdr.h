#ifndef __HDMI_HDR_H__
#define __HDMI_HDR_H__

#ifdef UT_flag
#undef CONFIG_SUPPORT_SCALER
#endif

extern void newbase_hdmi_dv_hdr_enable(unsigned char detectEn);
extern void newbase_hdmi_hdr10_enable(unsigned char detectEn);
extern unsigned char newbase_hdmi_dv_hdr_md_info_check(void);
extern unsigned char newbase_hdmi_hdr10_md_info_check(void);
extern void newbase_hdmi_hdr_disable(void);
extern HDMI_bool newbase_hdmi_is_drm_info_ready(void);
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
extern void newbase_hdmi_dolby_vision_enable(unsigned char port, DOLBY_HDMI_VSIF_T HDR_Type);
#else
#define newbase_hdmi_dolby_vision_enable(port, HDR_Type)    {}
#endif

#endif //__HDMI_HDR_H__
