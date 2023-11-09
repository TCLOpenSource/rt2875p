/*=============================================================
 * File:    dprx_vfe_sysfs.c
 *
 * Desc:    DPRX VFE API
 *
 * AUTHOR:  kevin_wang@realtek.com
 *
 * Vresion: 0.0.1
 *
 *------------------------------------------------------------
 * Copyright (c) Realtek Semiconductor Corporation, 2021
 *
 * All rights reserved.
 *
 *============================================================*/
#include <linux/syscalls.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <dprx_adapter.h>
#include <dprx_vfe_api.h>
#include <dprx_vfe.h>
#include <dprxfun.h>
#ifdef CONFIG_DPRX_BUILD_IN_TEST
#include <dprx_vfe-build_in_test.h>
#endif
#if IS_ENABLED(CONFIG_RTK_MEASURE)
#include <measure/rtk_measure.h>
#endif

const char* _get_dprx_type_str(DP_TYPE type)
{
    switch(type)
    {
    case DP_TYPE_NONE:  return "None";
    case DP_TYPE_DP:    return "DP";
    case DP_TYPE_mDP:   return "mDP";
    case DP_TYPE_USB_TYPE_C:    return "TypeC";
    case DP_TYPE_PSEUDO_HDMI_DP:    return "HDMI";
    case DP_TYPE_PSEUDO_DP:     return "Pseudo";
    default:            return "Unknown";
    }
}

const char* _get_dprx_color_space_str(
    DP_COLOR_SPACE_E    color_space
    )
{
    switch(color_space)
    {
    case DP_COLOR_SPACE_RGB:    return "RGB";
    case DP_COLOR_SPACE_YUV444: return "YUV444";
    case DP_COLOR_SPACE_YUV422: return "YUV422";
    case DP_COLOR_SPACE_YUV420: return "YUV420";
    case DP_COLOR_SPACE_YONLY:  return "Y-Only";
    case DP_COLOR_SPACE_RAW:    return "RAW";
    default:                    return "Unknown";
    }
}

const char* _get_dprx_colorimetry_str(
    DP_COLORIMETRY_E    colorimetry
    )
{
    switch(colorimetry)
    {
    case DP_COLORIMETRY_LEGACY_RGB: return "LEGACY RGB";
    case DP_COLORIMETRY_SRGB:       return "SRGB";
    case DP_COLORIMETRY_XRGB:       return "XRGB";
    case DP_COLORIMETRY_SCRGB:      return "SCRGB";
    case DP_COLORIMETRY_ADOBE_RGB:  return "ADOBE RGB";
    case DP_COLORIMETRY_DCI_P3:     return "DCI P3";
    case DP_COLORIMETRY_CUSTOM_COLOR_PROFILE:    return "COLOR PROFILE";
    case DP_COLORIMETRY_BT_2020_RGB:return "BT.2020 RGB";
    case DP_COLORIMETRY_BT_601:     return "BT 601";
    case DP_COLORIMETRY_BT_709:     return "BT 709";
    case DP_COLORIMETRY_XV_YCC_601: return "XV YCC 601";
    case DP_COLORIMETRY_XV_YCC_709: return "XV YCC 709";
    case DP_COLORIMETRY_S_YCC_601:  return "S YCC 601";
    case DP_COLORIMETRY_OP_YCC_601: return "OP YCC 601";
    case DP_COLORIMETRY_BT_2020_YCCBCCRC: return "BT.2020 YCCBCCRC";
    case DP_COLORIMETRY_BT_2020_YCBCR: return "BT.2020 YCBCR";
    case DP_COLORIMETRY_DICOM_PS314:return "DICOM PS 3.14";
    case DP_COLORIMETRY_YONLY:      return "Y-Only";
    case DP_COLORIMETRY_RAW:        return "RAW";
    default:                        return "Unknown";
    }
}

const char* _get_dprx_dynamic_range_str(
    DP_DYNAMIC_RANGE_E    dynamic_range
    )
{
    switch(dynamic_range)
    {
    case DP_DYNAMIC_RANGE_VESA: return "VESA Range(Full)";
    case DP_DYNAMIC_RANGE_CTA:  return "CTA Range(limited)";
    default:                    return "Unknown";
    }
}

const char* _get_dprx_content_type_str(
    DP_CONTENT_TYPE_E    content_type
    )
{
    switch(content_type)
    {
    case DP_CONTENT_TYPE_NOT_DEFINED:   return "Not Defined";
    case DP_CONTENT_TYPE_GRAPHICS:      return "Graphics";
    case DP_CONTENT_TYPE_PHOTO:         return "Photo";
    case DP_CONTENT_TYPE_VIDEO:         return "Video";
    case DP_CONTENT_TYPE_GAME:          return "Game";
    default:                            return "Unknown";
    }
}


const char* _get_dprx_eotf_str(
    unsigned char       eotf
    )
{
    switch(eotf)
    {
    case 0:     return "Traditional gamma - SDR Luminance Range";
    case 1:     return "Traditional gamma - HDR Luminance Range";
    case 2:     return "SMPTE ST 2084";
    case 3:     return "HLG based on ITU-RBT.2100-0 [50]";
    default:    return "Reserved";
    }
}

const char* _get_dprx_drr_str(
    DP_DRR_MODE_E       mode
    )
{
    switch(mode)
    {
    case DP_DRR_MODE_FRR:           return "Fix Refresh Rate";
    case DP_DRR_MODE_AMD_FREE_SYNC: return "AMD Free Sync";
    case DP_DRR_MODE_ADAPTIVE_SYNC: return "Adaptive Sync";
    default:                        return "Unknown";
    }
}

int _get_dprx_link_symbol_rate_mbps(
    DP_LINK_RATE        link_rate
    )
{
    switch(link_rate)
    {
    case DP_LINK_RATE_RBR_1p62G:    return 162;
    case DP_LINK_RATE_HBR1_2p7G:    return 270;
    case DP_LINK_RATE_HBR2_5p4G:    return 540;
    case DP_LINK_RATE_HBR3_8p1G:    return 810;
    case DP_LINK_RATE_UHBR10_10G:   return 1000;
    case DP_LINK_RATE_UHBR13_13p5G: return 1350;
    case DP_LINK_RATE_UHBR20_20G:   return 2000;
    case DP_LINK_RATE_MAX:
    default:                        return 0;
    }
}

static unsigned int _dump_dprx_vfe_status(
    char*           buf,
    int             count
    )
{
    int  n;
    int  i;
    char *ptr = buf;
    unsigned char rx_port_count = 0;
    DP_PORT_CAPABILITY cap;

    n = scnprintf(ptr, count, "---------------- DPRX STATUS (ver: %s) --------------\n", "V1.0");
    ptr+=n; count-=n;

    vfe_dprx_drv_get_rx_port_cnt(&rx_port_count);

    n = scnprintf(ptr, count, "VFE   : port_count = %d, connected port= %d\n",
        rx_port_count, (g_connected_dprx) ? g_connected_dprx->ch : -1);
    ptr+=n; count-=n;

    for (i=0; i<rx_port_count; i++)
    {
        vfe_dprx_drv_get_rx_port_capability(i, &cap);
        n = scnprintf(ptr, count, "DP%d   : type=%d (%s), max_lane=%d, link_rate_mask=%x, fec_support=%d, dsc_support=%d,\n",
                i, cap.type, _get_dprx_type_str(cap.type),
                cap.lane_cnt, cap.link_rate_mask, cap.capability.fec_support, cap.capability.dsc_support);
        ptr+=n; count-=n;
    }

    return ptr - buf;
}

#if IS_ENABLED(CONFIG_RTK_MEASURE)

//--------------------------------------------------
// Func  : _gen_offms_flag
//
// Desc  : generate offms flag by DPRX timing.
//         DPRX timing will provide color space and
//         pixel mode info to generate OFFMS ctrl flag
//
// Param : port : dprx port
//
// Retn  : offms ctrl flag
//--------------------------------------------------
OFFMS_CTRL_FALGS _gen_offms_flag(DPRX_TIMING_INFO_T* p_timing)
{
    OFFMS_CTRL_FALGS offms_flag = 0;

    switch(p_timing->pixel_mode_offms)
    {
    case DPRX_OUT_PIXEL_MODE_1P: offms_flag = OFFMS_FLAGS_PIXEL_MODE(OFFMS_1_PIXEL_MODE); break;
    case DPRX_OUT_PIXEL_MODE_2P: offms_flag = OFFMS_FLAGS_PIXEL_MODE(OFFMS_2_PIXEL_MODE); break;
    case DPRX_OUT_PIXEL_MODE_4P: offms_flag = OFFMS_FLAGS_PIXEL_MODE(OFFMS_4_PIXEL_MODE); break;
    default:                     offms_flag = OFFMS_FLAGS_PIXEL_MODE(OFFMS_1_PIXEL_MODE); break;
    }

    if (p_timing->color_space == DP_COLOR_SPACE_YUV420)
        offms_flag |= OFFMS_FLAGS_YUV420_MODE(1);

    return offms_flag;
}

#endif

static unsigned int _dump_dprx_adp_status(
    unsigned char   ch,
    char*           buf,
    int             count
    )
{
    dprx_dev_t* p_dprx = _vfe_dprx_drv_get_dprx_adapter(ch);
    int  n;
    char *ptr = buf;
    DP_LINK_STATUS_T   link_status;
    DPRX_TIMING_INFO_T timing;
    DPRX_HDCP_STATUS_T hdcp_status;
    DPRX_DRM_INFO_T    drm_info;
    DP_PIXEL_ENCODING_FORMAT_T pixel_format;

    if (p_dprx==NULL)
        return 0;

    n = scnprintf(ptr, count, "\n---------------- DP[%d] STATUS (connect=%d)--------------\n", ch, (g_connected_dprx==p_dprx) ? 1 : 0);
    ptr+=n; count-=n;

    //---------------------------------------
    // Cable Status
    //---------------------------------------
    n = scnprintf(ptr, count, "Cable : det=%d, hpd=%d\n",
            dprx_adp_get_connect_status(p_dprx), dprx_adp_get_hpd(p_dprx));
    ptr+=n; count-=n;

    //---------------------------------------
    // Link Status
    //---------------------------------------
    memset(&link_status, 0, sizeof(link_status));

    dprx_adp_get_link_status(p_dprx, &link_status);
    n = scnprintf(ptr, count, "Link  : lane=%d, link_rate=%d, channel_coding=%d, fec=%d, scr=%d, ch_align=%d\n",
            link_status.lane_mode,
            _get_dprx_link_symbol_rate_mbps(link_status.link_rate),
            link_status.channel_coding,
            link_status.fec_enable,
            link_status.scarmbling_enable,
            link_status.channel_align);
    ptr+=n; count-=n;

    //---------------------------------------
    // Video Status
    //---------------------------------------
    memset(&timing, 0, sizeof(timing));

    dprx_adp_get_video_timing(p_dprx, &timing);
    n = scnprintf(ptr, count, "VIDEO : h_total=%d, v_total=%d, h_act=%d, v_act=%d, h_freq=%d, v_freq=%d.%d /(actual_vfreq=%d.%d), interlace=%d, drr_mode=%d, de_mode=%d, dolby_hdr=%d\n",
        timing.htotal, timing.vtotal,
        timing.hact, timing.vact,
        timing.hfreq_hz, timing.vfreq_hz_x100/100, timing.vfreq_hz_x100%100,
        timing.curr_vfreq_hz_x100/100, timing.curr_vfreq_hz_x100%100,
        timing.is_interlace,
        timing.drr_mode,
        (timing.ptg_mode == DP_PTG_REGEN_NO_VSYNC_MODE) ? 1 : 0,
        timing.is_dolby_hdr);
    ptr+=n; count-=n;

#if IS_ENABLED(CONFIG_RTK_MEASURE)
    //---------------------------------------
    // MEASURE Status
    //---------------------------------------
    if (timing.hact && timing.vact)
    {
        MEASURE_TIMING_T measre_timing;
        memset(&measre_timing, 0, sizeof(measre_timing));

        rtk_offms_measure_video(OFFMS_SOURCE_DPRX, &measre_timing, _gen_offms_flag(&timing));

        n = scnprintf(ptr, count, "OFFMS : h_total=%d, v_total=%d, h_act=%d, v_act=%d, h_freq=%d, v_freq=%d.%d, h_proch=%d, v_proch=%d, h_sta=%d, v_sta=%d, h_front=%d, v_front=%d, polarity=%x (b1:vsp=%s, b0:hsp=%s)\n",
            measre_timing.h_total,
            measre_timing.v_total,
            measre_timing.h_act_len,
            measre_timing.v_act_len,
            measre_timing.h_freq/10,
            measre_timing.v_freq/1000,
            measre_timing.v_freq%1000,
            measre_timing.h_total - measre_timing.h_act_len,
            measre_timing.v_total - measre_timing.v_act_len,
            measre_timing.h_act_sta,
            measre_timing.v_act_sta,
            measre_timing.h_total - measre_timing.h_act_len - measre_timing.h_act_sta,
            measre_timing.v_total - measre_timing.v_act_len - measre_timing.v_act_sta,
            measre_timing.polarity,
           (measre_timing.polarity & 0x2) ? "Pos" : "Neg",
           (measre_timing.polarity & 0x1) ? "Pos" : "Neg");
        ptr+=n; count-=n;
    }
#endif

    //---------------------------------------
    // PIXEL Status
    //---------------------------------------
    memset(&pixel_format, 0, sizeof(pixel_format));
    dprx_adp_get_pixel_encoding_info(p_dprx,&pixel_format);
    n = scnprintf(ptr, count, "PIXEL : color_space=%s, depth=%d bits, colorimetry=%s, dynamic_range=%s, content_type=%s\n",
            _get_dprx_color_space_str(pixel_format.color_space),
            pixel_format.color_depth,
            _get_dprx_colorimetry_str(pixel_format.colorimetry),
            _get_dprx_dynamic_range_str(pixel_format.dynamic_range),
            _get_dprx_content_type_str(pixel_format.content_type));
    ptr+=n; count-=n;

    //---------------------------------------
    // DRR info
    //---------------------------------------
    dprx_adp_get_drm_info(p_dprx, &drm_info);
    n = scnprintf(ptr, count, "DRR   : %s (%d)  \n",
        _get_dprx_drr_str(drvif_Dprx_GetDrrMode()), drvif_Dprx_GetDrrMode());
    ptr+=n; count-=n;

    //---------------------------------------
    // FreeSync
    //---------------------------------------
    if (drvif_Dprx_GetAMDFreeSyncEnable())
    {
        DP_AMD_FREE_SYNC_INFO_T fs_info;

        memset(&fs_info, 0, sizeof(DP_AMD_FREE_SYNC_INFO_T));

        drvif_Dprx_GetAMDFreeSyncStatus(&fs_info);

        switch(fs_info.version)
        {
        case 1:
            n = scnprintf(ptr, count, "AMD SPD : ver=%d, fs_support=%d, fs_en=%d, fs_act=%d, refresh_rate=%d-%d\n",
                    fs_info.version,
                    fs_info.freesync_supported,
                    fs_info.freesync_enabled,
                    fs_info.freesync_activate,
                    fs_info.min_refresh_rate,
                    fs_info.max_refresh_rate);
            ptr+=n; count-=n;
            break;

        case 2:
            n = scnprintf(ptr, count, "AMD SPD : ver=%d, fs_support=%d, fs_en=%d, fs_act=%d, native_color_act=%d, brightness_ctrl_act=%d, ld_disable=%d, refresh_rate=%d-%d\n",
                    fs_info.version,
                    fs_info.freesync_supported,
                    fs_info.freesync_enabled,
                    fs_info.freesync_activate,
                    fs_info.native_color_space_active,
                    fs_info.brightness_control_active,
                    fs_info.local_diming_disalbe,
                    fs_info.min_refresh_rate,
                    fs_info.max_refresh_rate);
            ptr+=n; count-=n;
            break;

        default:
            n = scnprintf(ptr, count, "AMD SPD : ver=%d (Unknown Version) \n", fs_info.version);
            ptr+=n; count-=n;
            break;
        }
    }

    //---------------------------------------
    // DRM info
    //---------------------------------------
    memset(&drm_info, 0, sizeof(drm_info));

    dprx_adp_get_drm_info(p_dprx, &drm_info);

    n = scnprintf(ptr, count, "DRM   : ver=%02x, length=%d, eotf=%d (%s), eMeta_Desc=%02x ...\n",
        drm_info.nVersion, drm_info.nLength, drm_info.eEOTFtype, _get_dprx_eotf_str(drm_info.eEOTFtype), drm_info.eMeta_Desc);
    ptr+=n; count-=n;

    //---------------------------------------
    // HDCP Status
    //---------------------------------------
    memset(&hdcp_status, 0, sizeof(hdcp_status));
    dprx_adp_get_hdcp_status(p_dprx, &hdcp_status);
    n = scnprintf(ptr, count, "HDCP  : mode=%d, state=%d\n",  hdcp_status.mode, hdcp_status.state);
    ptr+=n; count-=n;

    //---------------------------------------
    // Adapter Status
    //---------------------------------------
    n = dprx_adp_attr_show(p_dprx, ADP_ATTR_STATUS, ptr, count);
    ptr+=n; count-=n;

    return ptr - buf;
}

static void _set_wakeup_by_aux(unsigned char enable)
{
    if (enable)
        g_dprx_vfe_suspend_mode |= WAKEUP_BY_AUX;
    else
        g_dprx_vfe_suspend_mode &= ~WAKEUP_BY_AUX;
}

static void _set_wakeup_by_signal(unsigned char enable)
{
    if (enable)
        g_dprx_vfe_suspend_mode |= WAKEUP_BY_SIGNAL;
    else
        g_dprx_vfe_suspend_mode &= ~WAKEUP_BY_SIGNAL;
}

//===========================================================================
// Func : dprx_show_status
//===========================================================================

static ssize_t dprx_show_status(
    struct device*              dev,
    struct device_attribute*    attr,
    char*                       buf
    )
{
    int  i, n;
    int  count = PAGE_SIZE;
    char *ptr = buf;

    n = _dump_dprx_vfe_status(ptr, count);
    ptr+=n; count-=n;

    for (i=0; i<MAX_DPRX_PORT; i++)
    {
        n = _dump_dprx_adp_status(i, ptr, count);
        ptr+=n; count-=n;
    }
    return ptr - buf;
}


static DEVICE_ATTR(status, 0444, dprx_show_status, NULL);


//===========================================================================
// Func : dprx_show_dpcd
//===========================================================================

static unsigned int  g_dpcd_mnt_adrr = 0xFFFFFFFF;

static ssize_t dprx_show_dpcd(
    struct device*              dev,
    struct device_attribute*    attr,
    char*                       buf
    )
{
    int  i, n;
    int  count = PAGE_SIZE;
    char *ptr = buf;

    for (i=0; i<MAX_DPRX_PORT; i++)
    {
        dprx_dev_t* p_dprx = _vfe_dprx_drv_get_dprx_adapter(i);

        n = dprx_adp_attr_show(p_dprx, ADP_ATTR_DPCD, ptr, count);
        ptr+=n; count-=n;
    }

    n = scnprintf(ptr, count, "\n\n--------------------------------------------------\n"); ptr+=n; count-=n;
    n = scnprintf(ptr, count, "command: echo <DPCD_CMD> > dpcd\n");                     ptr+=n; count-=n;
    n = scnprintf(ptr, count, " watch dpcd : watch <DPCD_ADDR>\n");                     ptr+=n; count-=n;
    n = scnprintf(ptr, count, " set dpcd   : <DPCD_ADDR>=<DPCD_VAL>\n");                ptr+=n; count-=n;
    n = scnprintf(ptr, count, "--------------------------------------------------\n");  ptr+=n; count-=n;

    if (g_connected_dprx && g_dpcd_mnt_adrr <=0xFFFFF)
    {
        unsigned char dpcd_val = 0xde;
        dprx_adp_get_dpcd(g_connected_dprx, g_dpcd_mnt_adrr, &dpcd_val, 1);

        n = scnprintf(ptr, count, "watch dpcd - addr : %05x, val=%02x\n", g_dpcd_mnt_adrr, dpcd_val); ptr+=n; count-=n;
    }
    n = scnprintf(ptr, count, "--------------------------------------------------\n\n"); ptr+=n; count-=n;

    return ptr - buf;
}


static ssize_t dprx_store_dpcd(struct device *dev,
                   struct device_attribute *attr,
                   const char *buf, size_t count)
{
    unsigned int  addr;
    unsigned int  val;
    unsigned char dpcd_val = 0xDE;
    if (sscanf(buf, "%5x=%x", &addr, &val)==2)
    {

        DP_VFE_INFO("addr=%05x, val=%02x\n", addr, val);

        if (addr <= 0xFFFFF && val <= 0xFF)
        {
            g_dpcd_mnt_adrr = addr;
            dpcd_val = val & 0xFF;
            dprx_adp_set_dpcd(g_connected_dprx, g_dpcd_mnt_adrr, &dpcd_val, 1);
        }
        else
        {
            g_dpcd_mnt_adrr = 0xFFFFFFFF;
        }
    }
    else if (sscanf(buf, "watch %5x", &addr)==1)
    {
        g_dpcd_mnt_adrr = (addr <= 0xFFFFF) ? addr : 0xFFFFFFFF;
    }
    else
    {
        DP_VFE_INFO("unkonwn command %s (%d)\n", buf, sscanf(buf, "%5x=%x", &addr, &val));
    }

    return count;
}

static DEVICE_ATTR(dpcd, 0644, dprx_show_dpcd, dprx_store_dpcd);




//===========================================================================
// Func : dprx_show_edid
//===========================================================================

static ssize_t dprx_show_edid(
    struct device*              dev,
    struct device_attribute*    attr,
    char*                       buf
    )
{
    int  i, j, n;
    int  count = PAGE_SIZE;
    char *ptr = buf;
    unsigned char edid[256];

    for (i=0; i<MAX_DPRX_PORT; i++)
    {
        n = scnprintf(ptr, count, "\n---------------- DP[%d] EDID --------------\n", i);
        ptr+=n; count-=n;

        if (vfe_dprx_drv_read_edid(i, edid, sizeof(edid))==DPRX_DRV_NO_ERR)
        {
            for (j=0; j<sizeof(edid); j+=16)
            {
                n = scnprintf(ptr, count, "%03x | %02x %02x %02x %02x _ %02x %02x %02x %02x _ %02x %02x %02x %02x _ %02x %02x %02x %02x\n",
                    j,
                    edid[j],    edid[j+1],  edid[j+2],  edid[j+3],
                    edid[j+4],  edid[j+5],  edid[j+6],  edid[j+7],
                    edid[j+8],  edid[j+9],  edid[j+10], edid[j+11],
                    edid[j+12], edid[j+13], edid[j+14], edid[j+15]);
                ptr+=n; count-=n;
            }
        }
    }

    return ptr - buf;
}

static DEVICE_ATTR(edid, 0444, dprx_show_edid, NULL);

const char* _get_sdp_type_str(DP_SDP_TYPE type)
{
    switch(type)
    {
    case DP_SDP_TYPE_VSC: return "VSC";
    case DP_SDP_TYPE_HDR: return "HDR";
    case DP_SDP_TYPE_SPD: return "SPD";
    case DP_SDP_TYPE_DVS: return "Dolby Vision";
    case DP_SDP_TYPE_ADP_SYNC: return "Adaptive Sync";
    case DP_SDP_TYPE_VSC_EXT_VESA: return "VSC_EXT_VESA";
    default:              return "unknown";
    }
}

const char* content_type_str(unsigned char type)
{
    switch(type)
    {
    case 0:  return "Undefine";
    case 1:  return "Graphics";
    case 2:  return "Photo";
    case 3:  return "Video";
    case 4:  return "Game";
    default: return "Reserved";
    }
}

int _dump_vsc_info(DP_SDP_DATA_T* p_sdp, char* p_buf, unsigned int count)
{
    int n;
    char *ptr = p_buf;

    if (p_sdp && p_sdp->seq_num)
    {
        n = scnprintf(ptr, count, "------ VSC info ------\n");
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "DB16 | %02x  [7:4] Pixel Encoding=%x, [3:0] colorimetry=%x\n",
                    p_sdp->pb[16], p_sdp->pb[16]>>4, p_sdp->pb[16]& 0xF);
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "DB17 | %02x  [7] DyanmicRange=%d (%s), [2:1] ComponentDepth=%d (%d bits)\n",
                    p_sdp->pb[17],(p_sdp->pb[17] >>7) & 0x1,
                   ((p_sdp->pb[17] >>7) & 0x1) ? "CTA" : "VESA",
                   (p_sdp->pb[17] & 0x7), 6+((p_sdp->pb[17] & 0x7)<<1));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "DB18 | %02x  [2:1] Content Type=%x(%s)\n",
                    p_sdp->pb[18],
                   (p_sdp->pb[18] & 0x3),
                    content_type_str((p_sdp->pb[18] & 0x3)));

        ptr+=n; count-=n;
    }

    return ptr - p_buf;
}

int _dump_sdp_data(unsigned char ch, DP_SDP_TYPE type, char* p_buf, unsigned int count)
{
    DP_SDP_DATA_T sdp_data;
    int n;
    char *ptr = p_buf;

    // read sdp
    vfe_dprx_drv_get_port_sdp_data(ch, type, &sdp_data);

    // print banner
    n = scnprintf(ptr, count, "\n----------------------------------------------------\n"); ptr+=n; count-=n;

    n = scnprintf(ptr, count, " %s SDP : count=%d, time=%d, pass_time=%d ms \n", _get_sdp_type_str(type), sdp_data.seq_num, sdp_data.time_stamp,
                    (sdp_data.seq_num) ? ((unsigned int) dprx_odal_get_system_time_ms())-sdp_data.time_stamp : 0);
    ptr+=n; count-=n;

    n = scnprintf(ptr, count, "----------------------------------------------------\n"); ptr+=n; count-=n;

    // print data
    n = scnprintf(ptr, count, "HB[0]  | %02x %02x %02x %02x\n", sdp_data.hb[0],sdp_data.hb[1],sdp_data.hb[2],sdp_data.hb[3]);
    ptr+=n; count-=n;

    n = scnprintf(ptr, count, "DB[0]  | %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x \n",
            sdp_data.pb[0],sdp_data.pb[1],sdp_data.pb[2],sdp_data.pb[3],
            sdp_data.pb[4],sdp_data.pb[5],sdp_data.pb[6],sdp_data.pb[7],
            sdp_data.pb[8],sdp_data.pb[9],sdp_data.pb[10],sdp_data.pb[11],
            sdp_data.pb[12],sdp_data.pb[13],sdp_data.pb[14],sdp_data.pb[15]);
    ptr+=n; count-=n;

    n = scnprintf(ptr, count, "DB[16] | %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x \n",
            sdp_data.pb[16],sdp_data.pb[17],sdp_data.pb[18],sdp_data.pb[19],
            sdp_data.pb[20],sdp_data.pb[21],sdp_data.pb[22],sdp_data.pb[23],
            sdp_data.pb[24],sdp_data.pb[25],sdp_data.pb[26],sdp_data.pb[27],
            sdp_data.pb[28],sdp_data.pb[29],sdp_data.pb[30],sdp_data.pb[31]);
    ptr+=n; count-=n;

    switch (type)
    {
    case DP_SDP_TYPE_VSC:
        n = _dump_vsc_info(&sdp_data, ptr, count);
        ptr+=n; count-=n;
        break;

    default:
        break; // do nothing
    }

    return ptr - p_buf;
}

int _dump_msa_data(unsigned char ch, char* p_buf, unsigned int count)
{
    DPRX_MSA_INFO_T msa;
    int n;
    char *ptr = p_buf;

    // read sdp
    memset(&msa, 0, sizeof(msa));
    vfe_dprx_drv_get_port_msa_info(ch, &msa);

    // print banner
    n = scnprintf(ptr, count, "\n----------------------------------------------------\n"); ptr+=n; count-=n;
    n = scnprintf(ptr, count, " MSA  \n");ptr+=n; count-=n;
    n = scnprintf(ptr, count, "----------------------------------------------------\n"); ptr+=n; count-=n;

    // print data
    n = snprintf(ptr, count, "mvid=%d, nvid=%d\n", (unsigned int) msa.mvid, (unsigned int) msa.nvid); ptr+=n; count-=n;
    n = snprintf(ptr, count, "h_total=%d, h_act=%d\n", msa.htotal, msa.hwidth); ptr+=n; count-=n;
    n = snprintf(ptr, count, "v_total=%d, v_act=%d\n", msa.vtotal, msa.vwidth); ptr+=n; count-=n;
    n = snprintf(ptr, count, "vs_pol=%d,  vs_width=%d, v_start=%d\n", msa.vsync_polarity, msa.vwidth, msa.vstart); ptr+=n; count-=n;
    n = snprintf(ptr, count, "hs_pol=%d,  hs_width=%d, h_start=%d\n", msa.hsync_polarity, msa.hwidth, msa.hstart); ptr+=n; count-=n;
    n = snprintf(ptr, count, "misc0=%02x\n", msa.misc0);                        ptr+=n; count-=n;
    n = snprintf(ptr, count, "misc1=%02x (b6: use_vsc_color=%d)\n", msa.misc1, (msa.misc1 >>6) & 0x1); ptr+=n; count-=n;

    return ptr - p_buf;
}


int _dump_vbid_data(unsigned char ch, char* p_buf, unsigned int count)
{
    DPRX_VBID_INFO_T vbid;
    int n;
    char *ptr = p_buf;

    // read sdp
    memset(&vbid, 0, sizeof(vbid));
    vfe_dprx_drv_get_port_vbid_info(ch, &vbid);

    // print banner
    n = scnprintf(ptr, count, "\n----------------------------------------------------\n"); ptr+=n; count-=n;
    n = scnprintf(ptr, count, " VBID  \n");                                                ptr+=n; count-=n;
    n = scnprintf(ptr, count, "----------------------------------------------------\n");   ptr+=n; count-=n;

    // print data
    n = snprintf(ptr, count, "b7:reserved=%d\n",          vbid.b7_reserved); ptr+=n; count-=n;
    n = snprintf(ptr, count, "b6:compressed_stream=%d\n", vbid.b6_compressed_stream); ptr+=n; count-=n;
    n = snprintf(ptr, count, "b5:hdcp_sync_detect=%d\n",  vbid.b5_hdcp_sync_detect); ptr+=n; count-=n;
    n = snprintf(ptr, count, "b4:audio_mute=%d\n",        vbid.b4_audio_mute); ptr+=n; count-=n;
    n = snprintf(ptr, count, "b3:no_video_stream=%d\n",   vbid.b3_no_video_stream); ptr+=n; count-=n;
    n = snprintf(ptr, count, "b2:interlace=%d\n",         vbid.b2_interlace); ptr+=n; count-=n;
    n = snprintf(ptr, count, "b1:field_id=%d\n",          vbid.b1_field_id); ptr+=n; count-=n;
    n = snprintf(ptr, count, "b0:vblink=%d\n",            vbid.b0_vblink); ptr+=n; count-=n;

    return ptr - p_buf;
}

//===========================================================================
// Func : dprx_show_sdp
//===========================================================================

static ssize_t dprx_show_sdp(
    struct device*              dev,
    struct device_attribute*    attr,
    char*                       buf
    )
{
    int  i, n;
    int  count = PAGE_SIZE;
    char *ptr = buf;
    unsigned char ch;
    DP_SDP_TYPE sdp_type[] =
    {
        DP_SDP_TYPE_VSC,
        DP_SDP_TYPE_HDR,
        DP_SDP_TYPE_SPD,
        DP_SDP_TYPE_DVS,
        DP_SDP_TYPE_ADP_SYNC,
        DP_SDP_TYPE_VSC_EXT_VESA,
    };

    if (vfe_dprx_drv_get_connected_channel(&ch) == DPRX_DRV_NO_ERR)
    {
        // VBID
        n = _dump_vbid_data(ch, ptr, count);
        ptr +=n; count-=n;

        // MSA
        n = _dump_msa_data(ch, ptr, count);
        ptr +=n; count-=n;

        // SDP packets
        for (i=0; i< (sizeof(sdp_type)/sizeof(DP_SDP_TYPE)); i++)
        {
            n = _dump_sdp_data(ch, sdp_type[i], ptr, count);
            ptr +=n; count-=n;
        }
    }

    return ptr - buf;
}

static DEVICE_ATTR(sdp, 0444, dprx_show_sdp, NULL);

//===========================================================================
// Func : dprx_attr_show_debug_ctrl
//===========================================================================

static ssize_t dprx_attr_show_debug_ctrl(
    struct device*              dev,
    struct device_attribute*    attr,
    char*                       buf
    )
{
    int  n;
    int  count = PAGE_SIZE;
    char *ptr = buf;

    //---------------------------------------------------
    // HPD toggle
    //---------------------------------------------------
    n = scnprintf(ptr, count, "hpd_toggle=0\n");
    ptr+=n; count-=n;

    //---------------------------------------------------
    // Wakeup by Aux
    //---------------------------------------------------
    n = scnprintf(ptr, count, "wakeup_by_aux=%d\n", (g_dprx_vfe_suspend_mode & WAKEUP_BY_AUX) ? 1 : 0);
    ptr+=n; count-=n;

    //---------------------------------------------------
    // Wakeup by Signal
    //---------------------------------------------------
    n = scnprintf(ptr, count, "wakeup_by_signal=%d\n", (g_dprx_vfe_suspend_mode & WAKEUP_BY_SIGNAL) ? 1 : 0);
    ptr+=n; count-=n;

    //---------------------------------------------------
    // Driver init test for bring up
    //---------------------------------------------------
#ifdef CONFIG_DPRX_BUILD_IN_TEST
    n = scnprintf(ptr, count, "build_in_test_en=%d\n", dprx_vfe_get_build_in_test_enable());
    ptr+=n; count-=n;

    n = scnprintf(ptr, count, "build_test_ch=%d\n", dprx_vfe_get_build_in_test_channel());
    ptr+=n; count-=n;
#endif

    if (g_connected_dprx)
    {
        n = scnprintf(ptr, count, "---------- debug control for current adapter --------\n");
        ptr+=n; count-=n;

        n = dprx_adp_attr_show(g_connected_dprx, ADP_ATTR_DEBUG_CTRL, ptr, count);
        ptr+=n; count-=n;
    }

    return ptr - buf;
}


static ssize_t dprx_attr_store_debug_ctrl(struct device *dev,
                   struct device_attribute *attr,
                   const char *buf, size_t count)
{
    int val;

    if (sscanf(buf, "hpd_toggle=%5d", &val)==1)
    {
        vfe_dprx_drv_enable_hpd(0);
        msleep(val);
        vfe_dprx_drv_enable_hpd(1);
    }
    else if (sscanf(buf, "wakeup_by_aux=%5d", &val)==1)
    {
        _set_wakeup_by_aux((val) ? 1 : 0);
    }
    else if (sscanf(buf, "wakeup_by_signal=%5d", &val)==1)
    {
        _set_wakeup_by_signal((val) ? 1 : 0);
    }
#ifdef CONFIG_DPRX_BUILD_IN_TEST
    else if (sscanf(buf, "build_in_test_en=%5d", &val)==1)
    {
        dprx_vfe_set_build_in_test_enable((val) ? 1 : 0);
    }
    else if (sscanf(buf, "build_test_ch=%5d", &val)==1)
    {
        dprx_vfe_set_build_in_test_channel(val);
    }
#endif
    else if (g_connected_dprx)
    {
        return dprx_adp_attr_store(g_connected_dprx, ADP_ATTR_DEBUG_CTRL, (unsigned char *) buf, count);
    }

    return count;
}

static DEVICE_ATTR(debug_ctrl, 0644, dprx_attr_show_debug_ctrl, dprx_attr_store_debug_ctrl);


//===========================================================================
// Func : dprx_attr_show_debug_print
//===========================================================================

static ssize_t dprx_attr_show_debug_print(
    struct device*              dev,
    struct device_attribute*    attr,
    char*                       buf
    )
{
    int  n;
    int  count = PAGE_SIZE;
    char *ptr = buf;

    n = scnprintf(ptr, count, "vfe_trace=%d\n", (g_dprx_vfe_print_ctrl & DPRX_VFE_TRACE_EN) ? 1 : 0);
    ptr+=n; count-=n;

    return ptr - buf;
}


static ssize_t dprx_attr_store_debug_print(struct device *dev,
                   struct device_attribute *attr,
                   const char *buf, size_t count)
{
    int val;

    if (sscanf(buf, "vfe_trace=%d", &val)==1)
    {
        if (val)
            g_dprx_vfe_print_ctrl |= DPRX_VFE_TRACE_EN;
        else
            g_dprx_vfe_print_ctrl &= ~DPRX_VFE_TRACE_EN;
    }

    return count;
}

static DEVICE_ATTR(debug_print, 0644, dprx_attr_show_debug_print, dprx_attr_store_debug_print);

//////////////////////////////////////////////////////////////////////////////
// File Operations
//////////////////////////////////////////////////////////////////////////////

int dprx_open(struct inode *inode, struct file *file)
{
    return 0;
}

int dprx_release(struct inode *inode, struct file *file)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Module Init / Exit
//////////////////////////////////////////////////////////////////////////////

static struct file_operations dprx_fops =
{
    .owner   = THIS_MODULE,
    .open    = dprx_open,
    .release = dprx_release,
};

static struct miscdevice dprx_miscdev =
{
    MISC_DYNAMIC_MINOR, "rtk_dprx", &dprx_fops
};

int dprx_sysfs_init(void)
{
    DP_VFE_INFO("dprx_sysfs_init\n");
    if (misc_register(&dprx_miscdev))
    {
        DP_VFE_WARN("dprx_module_init failed - register misc device failed\n");
        return -ENODEV;
    }

    device_create_file(dprx_miscdev.this_device, &dev_attr_status);
    device_create_file(dprx_miscdev.this_device, &dev_attr_debug_ctrl);
    device_create_file(dprx_miscdev.this_device, &dev_attr_debug_print);
    device_create_file(dprx_miscdev.this_device, &dev_attr_dpcd);
    device_create_file(dprx_miscdev.this_device, &dev_attr_edid);
    device_create_file(dprx_miscdev.this_device, &dev_attr_sdp);
    return 0;
}

void dprx_syfss_exit(void)
{
    DP_VFE_INFO("dprx_syfss_exit\n");
    device_remove_file(dprx_miscdev.this_device, &dev_attr_status);
    device_remove_file(dprx_miscdev.this_device, &dev_attr_debug_ctrl);
    device_remove_file(dprx_miscdev.this_device, &dev_attr_debug_print);
    device_remove_file(dprx_miscdev.this_device, &dev_attr_dpcd);
    device_remove_file(dprx_miscdev.this_device, &dev_attr_edid);
    device_remove_file(dprx_miscdev.this_device, &dev_attr_sdp);

    misc_deregister(&dprx_miscdev);
}
