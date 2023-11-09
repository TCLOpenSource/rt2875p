/*=============================================================
 * File:    dprx_types.h
 *
 * Desc:    common data structure defintions
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

#ifndef __DPRX_TYPES_H__
#define __DPRX_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    DPRX_DRV_NO_ERR = 0,
    DPRX_DRV_STATE_ERR,
    DPRX_DRV_ARG_ERR,
    DPRX_DRV_HW_ERR,
}DPRX_DRV_ERR;

//--------------------------------------------------------
// DP Type
//--------------------------------------------------------
typedef enum {
    DP_TYPE_NONE        = 0,
    DP_TYPE_DP          = 1,
    DP_TYPE_mDP         = 2,
    DP_TYPE_USB_TYPE_C  = 3,
    DP_TYPE_PSEUDO_HDMI_DP = 4,
    DP_TYPE_PSEUDO_DP   = 5,
    DP_TYPE_MAX,
}DP_TYPE;

//--------------------------------------------------------
// Type C DP Cable Type
//--------------------------------------------------------
typedef enum
{
    CABLE_CFG_DP = 0,
    CABLE_CFG_TYPEC_CFG_A = 0xA,
    CABLE_CFG_TYPEC_CFG_B = 0xB,
    CABLE_CFG_TYPEC_CFG_C = 0xC,
    CABLE_CFG_TYPEC_CFG_D = 0xD,
    CABLE_CFG_TYPEC_CFG_E = 0xE,
    CABLE_CFG_TYPEC_CFG_A_FLIP = 0x8A,
    CABLE_CFG_TYPEC_CFG_B_FLIP = 0x8B,
    CABLE_CFG_TYPEC_CFG_C_FLIP = 0x8C,
    CABLE_CFG_TYPEC_CFG_D_FLIP = 0x8D,
    CABLE_CFG_TYPEC_CFG_E_FLIP = 0x8E,
}DPRX_CABLE_CFG;

//--------------------------------------------------------
// Link Layer data Types
//--------------------------------------------------------

typedef enum
{
    DP_LANE_MODE_1_LANE = 1,
    DP_LANE_MODE_2_LANE = 2,
    DP_LANE_MODE_4_LANE = 4,
}DP_LANE_MODE;

typedef enum
{
    DP_LINK_RATE_RBR_1p62G = 0,
    DP_LINK_RATE_HBR1_2p7G,
    DP_LINK_RATE_HBR2_5p4G,
    DP_LINK_RATE_HBR3_8p1G,
    DP_LINK_RATE_UHBR10_10G,
    DP_LINK_RATE_UHBR13_13p5G,
    DP_LINK_RATE_UHBR20_20G,
    DP_LINK_RATE_MAX,
}DP_LINK_RATE;

typedef enum
{
    DP_CHANNEL_CODING_8b10b = 0,
    DP_CHANNEL_CODING_128b132b,
    DP_CHANNEL_CODING_MAX,
}DP_CHANNEL_CODING;

#define DP_LINK_RATE_MASK(x)        (1<<x)
#define DP_LINK_RATE_RBR_MASK       DP_LINK_RATE_MASK(DP_LINK_RATE_RBR_1p62G)
#define DP_LINK_RATE_HBR1_MASK      DP_LINK_RATE_MASK(DP_LINK_RATE_HBR1_2p7G)
#define DP_LINK_RATE_HBR2_MASK      DP_LINK_RATE_MASK(DP_LINK_RATE_HBR2_5p4G)
#define DP_LINK_RATE_HBR3_MASK      DP_LINK_RATE_MASK(DP_LINK_RATE_HBR3_8p1G)
#define DP_LINK_RATE_UHBR10_MASK    DP_LINK_RATE_MASK(DP_LINK_RATE_UHBR10_10G)
#define DP_LINK_RATE_UHBR13_MASK    DP_LINK_RATE_MASK(DP_LINK_RATE_UHBR13_13p5G)
#define DP_LINK_RATE_UHBR20_MASK    DP_LINK_RATE_MASK(DP_LINK_RATE_UHBR20_20G)

typedef struct
{
    DP_TYPE             type;
    DP_LANE_MODE        lane_cnt;
    unsigned int        link_rate_mask;   // see DP_LINK_RATE_XXX_MASK

    struct
    {
        unsigned char   fec_support:1;
        unsigned char   dsc_support:1;
        unsigned char   reserved:6;
    }capability;
}DP_PORT_CAPABILITY;

typedef struct
{
    unsigned int valid:1;
    unsigned int lock:1;
    unsigned int reserved:14;
    unsigned int error_cnt:16;
}DP_CH_STATUS_T;

typedef struct
{
    DP_LANE_MODE        lane_mode;
    DP_LINK_RATE        link_rate;
    DP_CHANNEL_CODING   channel_coding;
    unsigned char       fec_enable:1;
    unsigned char       scarmbling_enable:1;
    unsigned char       channel_align:1;  // link locked and channel align
    unsigned char       reserved:5;
    DP_CH_STATUS_T      channel_status[4];
}DP_LINK_STATUS_T;

typedef enum
{
    DPRX_STREAM_TYPE_UNKNOWN =0,
    DPRX_STREAM_TYPE_AUDIO_ONLY,        // DP Stram is audio only
    DPRX_STREAM_TYPE_VIDEO_ONLY,        // DP Stram is video only
    DPRX_STREAM_TYPE_AUDIO_VIDEO,       // DP Stram has audio and video
    DPRX_STREAM_TYPE_MAX,
}DP_STREAM_TYPE_E;

//--------------------------------------------------------
// VBID
//--------------------------------------------------------
typedef struct
{
    unsigned char b0_vblink : 1;
    unsigned char b1_field_id : 1;
    unsigned char b2_interlace : 1;
    unsigned char b3_no_video_stream : 1;
    unsigned char b4_audio_mute : 1;
    unsigned char b5_hdcp_sync_detect : 1;
    unsigned char b6_compressed_stream : 1;
    unsigned char b7_reserved : 1;
}DPRX_VBID_INFO_T;

//--------------------------------------------------------
// MSA
//--------------------------------------------------------

typedef struct
{
    unsigned long  mvid;
    unsigned long  nvid;
    unsigned short htotal;
    unsigned short vtotal;
    unsigned short hstart;
    unsigned short vstart;
    unsigned char  hsync_polarity;
    unsigned char  vsync_polarity;
    unsigned short hsync_width;
    unsigned short vsync_width;
    unsigned short hwidth;
    unsigned short vwidth;
    unsigned char  misc0;
    unsigned char  misc1;
    unsigned long  reserved;
}DPRX_MSA_INFO_T;

//--------------------------------------------------------
// Video data Types
//--------------------------------------------------------

typedef enum
{
    DP_PTG_REGEN_HV_MODE = 0,
    DP_PTG_REGEN_NO_VSYNC_MODE,
    DP_PTG_REGEN_RESERVE_0,
    DP_PTG_REGEN_RESERVE_1,
}DP_PTG_REGEN_E;

typedef enum
{
    DP_COLOR_SPACE_RGB = 0,
    DP_COLOR_SPACE_YUV444,
    DP_COLOR_SPACE_YUV422,
    DP_COLOR_SPACE_YUV420,
    DP_COLOR_SPACE_YONLY,
    DP_COLOR_SPACE_RAW,
    DP_COLOR_SPACE_UNKNOWN,
}DP_COLOR_SPACE_E;

typedef enum
{
    DP_COLOR_DEPTH_6B = 6,
    DP_COLOR_DEPTH_7B = 7,
    DP_COLOR_DEPTH_8B = 8,
    DP_COLOR_DEPTH_10B = 10,
    DP_COLOR_DEPTH_12B = 12,
    DP_COLOR_DEPTH_14B = 14,
    DP_COLOR_DEPTH_16B = 16,
    DP_COLOR_DEPTH_UNKNOWN = 0,
}DP_COLOR_DEPTH_E;

typedef enum
{
    DP_COLORIMETRY_LEGACY_RGB = 0,
    DP_COLORIMETRY_SRGB,
    DP_COLORIMETRY_XRGB,
    DP_COLORIMETRY_SCRGB,
    DP_COLORIMETRY_ADOBE_RGB,
    DP_COLORIMETRY_DCI_P3,
    DP_COLORIMETRY_CUSTOM_COLOR_PROFILE,
    DP_COLORIMETRY_BT_2020_RGB,
    DP_COLORIMETRY_BT_601,
    DP_COLORIMETRY_BT_709,
    DP_COLORIMETRY_XV_YCC_601,
    DP_COLORIMETRY_XV_YCC_709,
    DP_COLORIMETRY_S_YCC_601,
    DP_COLORIMETRY_OP_YCC_601,
    DP_COLORIMETRY_BT_2020_YCCBCCRC,
    DP_COLORIMETRY_BT_2020_YCBCR,
    DP_COLORIMETRY_DICOM_PS314,
    DP_COLORIMETRY_YONLY,
    DP_COLORIMETRY_RAW,
    DP_COLORIMETRY_UNKNOWN,
}DP_COLORIMETRY_E;

typedef enum
{
    DP_DYNAMIC_RANGE_VESA =0, //full range
    DP_DYNAMIC_RANGE_CTA,        //limited range
}DP_DYNAMIC_RANGE_E;

typedef enum
{
    DP_CONTENT_TYPE_NOT_DEFINED =0,
    DP_CONTENT_TYPE_GRAPHICS,
    DP_CONTENT_TYPE_PHOTO,
    DP_CONTENT_TYPE_VIDEO,
    DP_CONTENT_TYPE_GAME,
}DP_CONTENT_TYPE_E;

typedef enum
{
    DP_DRR_MODE_FRR = 0,              // FIX Refresh Rate
    DP_DRR_MODE_AMD_FREE_SYNC = 1,    // AMD Free Sync
    DP_DRR_MODE_ADAPTIVE_SYNC,        // DP Adaptive Sync
    DP_DRR_MODE_UNKNWON,
}DP_DRR_MODE_E;

typedef struct
{
    unsigned char version;          // version 1 / version 2

    // PB6
    unsigned char freesync_supported : 1;
    unsigned char freesync_enabled : 1;
    unsigned char freesync_activate : 1;
    unsigned char native_color_space_active : 1;    // version==2 (PB9 present)
    unsigned char brightness_control_active : 1;    // version==2 (PB10 present)
    unsigned char local_diming_disalbe : 1;         // version==2
    unsigned char reserved_pb6 : 2;
    unsigned char min_refresh_rate;                 // PB7
    unsigned char max_refresh_rate;                 // PB8

    // PB9 : only valid when native_color_space_active is 1  (version==2)
    unsigned char srgb_eotf_active : 1;       // b0
    unsigned char bt709_eotf_active : 1;      // b1
    unsigned char gamma_2p2_eotf_active : 1;  // b2
    unsigned char gamma_2p6_eotf_active : 1;  // b3
    unsigned char pq_eotf_active : 1;         // b5
    unsigned char reserved_pb9 : 3;

    // PB10 : only valid when native_color_space_active is 1  (version==2)
    unsigned char brightness_control;

}__attribute__((packed)) DP_AMD_FREE_SYNC_INFO_T;

typedef struct
{
    DP_COLOR_SPACE_E  color_space;
    DP_COLOR_DEPTH_E  color_depth;
    DP_PTG_REGEN_E ptg_mode;

    unsigned short htotal;      // htotal (unit:pixel) HDE rising to HDE rising
    unsigned short hporch;      // hproch (unit:pixel) HDE falling to HDE rising
    unsigned short hstart;      // hstart (unit:pixel) HS rising edge to HDE rising
    unsigned short hsync;       // hsync  (unit:pixel) HS rising to HS falling
    unsigned short hact;        // hact   (unit:pixel) HDE rising to HDE falling
    unsigned short vtotal;      // vtotal (unit:line)  VDE rising to VDE rising
    unsigned short vporch;      // vporch (unit:line)  VDE falling to VDE rising
    unsigned short vstart;      // vporch (unit:line)  VS rising to VDE rising
    unsigned short vsync;       // vsync  (unit:line)  VS rising to VS falling
    unsigned short vact;        // vact   (unit:line)  VDE rising to VDE falling

    unsigned short hfreq_hz;    // H freq
    unsigned short vfreq_hz_x100;  // vfreq  (unit: 0.01Hz)

    unsigned short hs_polarity : 1;  // hs polarith : 0 : negative, 1 : positive
    #define HS_POL_POSITIVE  1
    #define HS_POL_NEGAITVE  0

    unsigned short vs_polarity : 1;  // vs polarith : 0 : negative, 1 : positive
    #define VS_POL_POSITIVE  1
    #define VS_POL_NEGAITVE  0

    unsigned short is_interlace : 1;
    unsigned short is_dsc : 1;
    unsigned short drr_mode : 4;  // see DP_DRR_MODE_E
    unsigned short is_dolby_hdr : 1;
    unsigned short pixel_mode : 3;
    unsigned short pixel_mode_offms : 3;
    #define DPRX_OUT_PIXEL_MODE_1P   1
    #define DPRX_OUT_PIXEL_MODE_2P   2
    #define DPRX_OUT_PIXEL_MODE_4P   4    
    unsigned short reversed : 1;

    unsigned short curr_vfreq_hz_x100;  // current vfreq (unit: 0.01Hz) only valid when drr_mode != DP_DRR_MODE_FRR
    unsigned char  isALLM;
}DPRX_TIMING_INFO_T;

typedef enum
{
    DP_DRM_EOTF_SDR_LUMINANCE_RANGE,
    DP_DRM_EOTF_HDR_LUMINANCE_RANGE,
    DP_DRM_EOTF_SMPTE_ST_2084,
    DP_DRM_EOTF_HLG,
    DP_DRM_EOTF_RESERVED_4,
    DP_DRM_EOTF_RESERVED_5,
    DP_DRM_EOTF_RESERVED_6,
    DP_DRM_EOTF_RESERVED_7,
    DP_DRM_EOTF_MAX,
}DP_DRM_EOTF_T;

typedef enum
{
    DP_DRM_META_TYPE1,
    DP_DRM_META_RESERVED1,
    DP_DRM_META_RESERVED2,
    DP_DRM_META_RESERVED3,
    DP_DRM_META_RESERVED4,
    DP_DRM_META_RESERVED5,
    DP_DRM_META_RESERVED6,
    DP_DRM_META_RESERVED7,
}DP_DRM_META_DESC_T;

typedef struct{
    unsigned char  nVersion;
    unsigned char  nLength;
    unsigned char  eEOTFtype;
    unsigned char  eMeta_Desc;
    unsigned short display_primaries_x0;
    unsigned short display_primaries_y0;
    unsigned short display_primaries_x1;
    unsigned short display_primaries_y1;
    unsigned short display_primaries_x2;
    unsigned short display_primaries_y2;
    unsigned short white_point_x;
    unsigned short white_point_y;
    unsigned short max_display_mastering_luminance;
    unsigned short min_display_mastering_luminance;
    unsigned short maximum_content_light_level;
    unsigned short maximum_frame_average_light_level;
}DPRX_DRM_INFO_T;

typedef enum
{
    DP_PACKET_STATUS_NOT_RECEIVED,    /**< DPRX packet status */
    DP_PACKET_STATUS_STOPPED,
    DP_PACKET_STATUS_UPDATED,
    DP_PACKET_STATUS_MAX
} DP_PACKET_STATUS_T;


typedef enum
{
    DP_AVI_CSC_RGB,    /**< DPRX CSC info */
    DP_AVI_CSC_YCBCR422,
    DP_AVI_CSC_YCBCR444,
    DP_AVI_CSC_FUTURE
} DP_AVI_CSC_T;


typedef enum
{
    DP_AVI_ACTIVE_INFO_INVALID,       /**< DPRX info */
    DP_AVI_ACTIVE_INFO_VALID
} DP_AVI_ACTIVE_INFO_T;


typedef enum
{
    DP_AVI_BAR_INFO_INVALID,          /**< DPRX info */
    DP_AVI_BAR_INFO_VERTICALVALID,
    DP_AVI_BAR_INFO_HORIZVALID,
    DP_AVI_BAR_INFO_VERTHORIZVALID
} DP_AVI_BAR_INFO_T;


typedef enum
{
    DP_AVI_SCAN_INFO_NODATA,          /**< DPRX info */
    DP_AVI_SCAN_INFO_OVERSCANNED,
    DP_AVI_SCAN_INFO_UNDERSCANNED,
    DP_AVI_SCAN_INFO_FUTURE
} DP_AVI_SCAN_INFO_T;


typedef enum
{
    DP_AVI_COLORIMETRY_NODATA,       /**< DPRX info */
    DP_AVI_COLORIMETRY_SMPTE170,
    DP_AVI_COLORIMETRY_ITU709,
    DP_AVI_COLORIMETRY_FUTURE
} DP_AVI_COLORIMETRY_T;


typedef enum
{
    DP_AVI_PICTURE_ARC_NODATA,        /**< DPRX info */
    DP_AVI_PICTURE_ARC_4_3,
    DP_AVI_PICTURE_ARC_16_9,
    DP_AVI_PICTURE_ARC_FUTURE
} DP_AVI_PICTURE_ARC_T;


typedef enum
{
    DP_AVI_ACTIVE_FORMAT_ARC_PICTURE, /**< DPRX info */
    DP_AVI_ACTIVE_FORMAT_ARC_4_3CENTER,
    DP_AVI_ACTIVE_FORMAT_ARC_16_9CENTER,
    DP_AVI_ACTIVE_FORMAT_ARC_14_9CENTER,
    DP_AVI_ACTIVE_FORMAT_ARC_OTHER
} DP_AVI_ACTIVE_FORMAT_ARC_T;


typedef enum
{
    DP_AVI_SCALING_NOSCALING,     /**< DPRX info */
    DP_AVI_SCALING_HSCALING,
    DP_AVI_SCALING_VSCALING,
    DP_AVI_SCALING_HVSCALING
} DP_AVI_SCALING_T;


typedef enum
{
    DP_AVI_IT_CONTENT_NODATA, /**< DPRX info */
    DP_AVI_IT_CONTENT_ITCONTENT
} DP_AVI_IT_CONTENT_T;


typedef enum
{
    DP_AVI_EXT_COLORIMETRY_XVYCC601,
    DP_AVI_EXT_COLORIMETRY_XVYCC709,
    DP_AVI_EXT_COLORIMETRY_SYCC601, // add sYCC 601
    DP_AVI_EXT_COLORIMETRY_ADOBEYCC601, // add Adobe YCC 601
    DP_AVI_EXT_COLORIMETRY_ADOBERGB, // add Adobe RGB
    DP_AVI_EXT_COLORIMETRY_BT2020_YCCBCCRC, // add BT 2020 CL
    DP_AVI_EXT_COLORIMETRY_BT2020_RGBORYCBCR, // add BT 2020 NCL
    DP_AVI_EXT_COLORIMETRY_XVRESERED
} DP_AVI_EXT_COLORIMETRY_T;


typedef enum
{
    DP_AVI_RGB_QUANTIZATION_RANGE_DEFAULT,        /**< DPRX info */
    DP_AVI_RGB_QUANTIZATION_RANGE_LIMITEDRANGE,
    DP_AVI_RGB_QUANTIZATION_RANGE_FULLRANGE,
    DP_AVI_RGB_QUANTIZATION_RANGE_RESERVED
} DP_AVI_RGB_QUANTIZATION_RANGE_T;


typedef enum
{
    DP_AVI_YCC_QUANTIZATION_RANGE_LIMITEDRANGE, /**< DPRX info */
    DP_AVI_YCC_QUANTIZATION_RANGE_FULLRANGE,
    DP_AVI_YCC_QUANTIZATION_RANGE_RESERVED
} DP_AVI_YCC_QUANTIZATION_RANGE_T;


typedef enum
{
    DP_AVI_CONTENT_TYPE_GRAPHICS,
    DP_AVI_CONTENT_TYPE_PHOTO,
    DP_AVI_CONTENT_TYPE_CINEMA,
    DP_AVI_CONTENT_TYPE_GAME,
    DP_AVI_CONTENT_TYPE_MAX
} DP_AVI_CONTENT_TYPE_T;


typedef enum
{
    DP_AVI_ADDITIONAL_COLORIMETRY_DCI_P3_D65,
    DP_AVI_ADDITIONAL_COLORIMETRY_DCI_P3_THEATER,
    DP_AVI_ADDITIONAL_COLORIMETRY_RESERVED,
    DP_AVI_ADDITIONAL_COLORIMETRY_MAX
} DP_AVI_ADDITIONAL_COLORIMETRY_T;


typedef struct
{
    unsigned char type;            /**< packet type */
    unsigned char version;         /**< packet version */
    unsigned char length;          /**< packet length */
} DP_IN_PACKET_T;


typedef struct
{
    DP_AVI_CSC_T ePixelEncoding;          /**< DPRX info */
    DP_AVI_ACTIVE_INFO_T eActiveInfo;     /**< DPRX info */   /* A0 */
    DP_AVI_BAR_INFO_T   eBarInfo;         /**< DPRX info */   /* B1B0 */
    DP_AVI_SCAN_INFO_T   eScanInfo;           /**< DPRX info */   /* S1S0 */
    DP_AVI_COLORIMETRY_T eColorimetry;        /**< DPRX info */   /* C1C0 */
    DP_AVI_PICTURE_ARC_T ePictureAspectRatio; /**< DPRX info */   /* M1M0 */
    DP_AVI_ACTIVE_FORMAT_ARC_T eActiveFormatAspectRatio;/**< DPRX info */ /* R3R0 */
    DP_AVI_SCALING_T eScaling;                        /**< DPRX info */   /* SC1SC0 */

    unsigned char VideoIdCode;                              /**< DPRX info */   /* VICn */
    unsigned char PixelRepeat;                              /**< DPRX info */

    DP_AVI_IT_CONTENT_T eITContent;           /**< DPRX info */   /*ITC */
    DP_AVI_EXT_COLORIMETRY_T eExtendedColorimetry;    /**< DPRX info */   /* EC2EC1EC0 */
    DP_AVI_RGB_QUANTIZATION_RANGE_T eRGBQuantizationRange;    /**< DPRX info */   /* Q1Q0 */
    DP_AVI_YCC_QUANTIZATION_RANGE_T eYCCQuantizationRange;    /**< DPRX info */   /* YQ1YQ0 */
    DP_AVI_CONTENT_TYPE_T eContentType;           /**< DPRX info */   /* Contents Type */

    /* bar info */
    unsigned short  TopBarEndLineNumber;             /**< DPRX info */
    unsigned short  BottomBarStartLineNumber;        /**< DPRX info */
    unsigned short  LeftBarEndPixelNumber;           /**< DPRX info */
    unsigned short  RightBarEndPixelNumber;          /**< DPRX info */

    DP_AVI_ADDITIONAL_COLORIMETRY_T eAdditionalColorimetry;         /**< DPRX info */ /*ACE3ACE0*/

    unsigned char f14;    /**< DPRX info */  /*F143F140*/

    DP_PACKET_STATUS_T packetStatus;  /**< DPRX info */
    DP_IN_PACKET_T packet;                    /**< DPRX info */
} DPRX_AVI_INFO_T;

typedef struct
{
    DP_COLOR_SPACE_E  color_space;
    DP_COLOR_DEPTH_E  color_depth;
    DP_COLORIMETRY_E  colorimetry;
    DP_DYNAMIC_RANGE_E  dynamic_range;
    DP_CONTENT_TYPE_E  content_type;
}DP_PIXEL_ENCODING_FORMAT_T;

//--------------------------------------------------------
// SDP data Types
//--------------------------------------------------------

typedef enum
{
    DP_SDP_TYPE_VSC,  // VSC
    DP_SDP_TYPE_HDR,  // DRM info
    DP_SDP_TYPE_SPD,  // SPD
    DP_SDP_TYPE_DVS,  // Dolby Vision VSIF
    DP_SDP_TYPE_ADP_SYNC,  // Adaptive Sync SDP
    DP_SDP_TYPE_VSC_EXT_VESA,  // VSC_EXT_VESA
}DP_SDP_TYPE;

#define SDP_HDR_LEN     4
#define SDP_DATA_LEN    32

typedef struct
{
    // SDP data
    unsigned char       hb[SDP_HDR_LEN];
    unsigned char       pb[SDP_DATA_LEN];
    // extra information
    unsigned int        seq_num;
    unsigned int        time_stamp;
}DP_SDP_DATA_T;

//--------------------------------------------------------
// Audio data Types
//--------------------------------------------------------

typedef enum
{
    DP_AUDIO_CODING_NO_AUDIO = 0,
    DP_AUDIO_CODING_LPCM,
    DP_AUDIO_CODING_AC3,
    DP_AUDIO_CODING_EAC3,
    DP_AUDIO_CODING_EAC3_ATMOS,
    DP_AUDIO_CODING_MAT,
    DP_AUDIO_CODING_MAT_ATMOS,
    DP_AUDIO_CODING_TRUEHD,
    DP_AUDIO_CODING_TRUEHD_ATMOS,
    DP_AUDIO_CODING_AAC,
    DP_AUDIO_CODING_MPEG,
    DP_AUDIO_CODING_DTS,
    DP_AUDIO_CODING_DTS_HD_MA,
    DP_AUDIO_CODING_DTS_EXPRESS,
    DP_AUDIO_CODING_DTS_CD,
    DP_AUDIO_CODING_UNKNOWN,
}DP_AUDIO_CODING_TYPE;

typedef struct
{
    DP_AUDIO_CODING_TYPE    coding_type;
    unsigned char           ch_num;
    unsigned int            sampling_frequency_khz_x100;
}DP_AUDIO_STATUS_T;

//--------------------------------------------------------
// HDCP related
//--------------------------------------------------------

typedef enum
{
    DP_HDCP_VER_HDCP13 = 0,
    DP_HDCP_VER_HDCP22,
    DP_HDCP_VER_HDCP23,
}DP_HDCP_VER_E;

typedef enum
{
    DP_HDCP_MODE_OFF = 0,
    DP_HDCP_MODE_HDCP_1P3,
    DP_HDCP_MODE_HDCP_2P2,
    DP_HDCP_MODE_HDCP_2P3,
}DP_HDCP_MODE;

typedef enum
{
    DP_HDCP_STATE_IDEL = 0,
    DP_HDCP_STATE_AUTH_START,
    DP_HDCP_STATE_AUTH_DONE,
}DP_HDCP_STATE;

typedef enum
{
    DP_HDCP_CAPBILITY_HDCP13 = (0x1 << DP_HDCP_VER_HDCP13),
    DP_HDCP_CAPBILITY_HDCP22 = (0x1 << DP_HDCP_VER_HDCP22),
    DP_HDCP_CAPBILITY_HDCP23 = (0x1 << DP_HDCP_VER_HDCP23),
    DP_HDCP_CAPBILITY_HDCP_REPEATER = (0x1 << 7),
}DP_HDCP_CAPABILITY_E;

#define HDCP1X_KSV_INFO_LEN     5
#define HDCP1X_KEY_SIZE         325

#define HDCP2X_RECEIVER_ID_LEN  5
#define HDCP2X_KEY_SIZE         878

typedef struct
{
    DP_HDCP_VER_E    version;
    union
    {
        unsigned char hdcp1x_key[HDCP1X_KEY_SIZE];
        unsigned char hdcp2x_key[HDCP2X_KEY_SIZE];
    };

}DPRX_HDCP_KEY_T;

typedef struct
{
    DP_HDCP_MODE    mode;       // current HDCP mode
    DP_HDCP_STATE   state;      // hdcp state

    struct
    {
        unsigned char an[8];
        unsigned char aksv[HDCP1X_KSV_INFO_LEN];
        unsigned char bksv[HDCP1X_KSV_INFO_LEN];
        unsigned char ri[2];
        unsigned char bcaps;
        unsigned char bstatus[2];
    }hdcp1x;

    struct
    {
        unsigned char   receiverid[HDCP2X_RECEIVER_ID_LEN];
    }hdcp2x;

}DPRX_HDCP_STATUS_T;

#define DP_INFOFRAME_PACKET_LEN		28
#define DP_VENDOR_SPECIFIC_REGID_LEN	3
#define DP_VENDOR_SPECIFIC_PAYLOAD_LEN \
    (DP_INFOFRAME_PACKET_LEN - DP_VENDOR_SPECIFIC_REGID_LEN)

typedef struct
{
    unsigned char type;            /**< packet type */
    unsigned char version;         /**< packet version */
    unsigned char length;          /**< packet length */
}DP_INFOFRAME_HEADER_T;

typedef enum
{
    DP_VSI_VIDEO_FORMAT_NO_ADDITIONAL_FORMAT, //2//2D
    DP_VSI_VIDEO_FORMAT_EXTENDED_RESOLUTION_FORMAT,
    DP_VSI_VIDEO_FORMAT_3D_FORMAT
}DP_VSI_VIDEO_FORMAT_E;

typedef enum
{
    DP_VSI_3D_STRUCTURE_FRAME_PACKING,		//*0
    DP_VSI_3D_STRUCTURE_FIELD_ALTERNATIVE,	//*1
    DP_VSI_3D_STRUCTURE_LINE_ALTERNATIVE,		//*2
    DP_VSI_3D_STRUCTURE_SIDEBYSIDE_FULL,		//*3
    DP_VSI_3D_STRUCTURE_L_DEPTH,				//*4
    DP_VSI_3D_STRUCTURE_L_DEPTH_GRAPHICS,	//*5
    DP_VSI_3D_STRUCTURE_TOP_BOTTOM,			//*6
    /*reserved 7*/
    DP_VSI_3D_STRUCTURE_SIDEBYSIDE_HALF	=	0x08,
    DP_VSI_3D_STRUCTURE_TOP_BOTTOM_DIRECTV
}DP_VSI_3D_STRUCTURE_E;

typedef enum
{
    DP_VSI_3D_EXT_DATA_HOR_SUB_SAMPL_0,
    DP_VSI_3D_EXT_DATA_HOR_SUB_SAMPL_1,
    DP_VSI_3D_EXT_DATA_HOR_SUB_SAMPL_2,
    DP_VSI_3D_EXT_DATA_HOR_SUB_SAMPL_3,
    DP_VSI_3D_EXT_DATA_QUINCUNX_MATRIX_0 = 0x04,
    DP_VSI_3D_EXT_DATA_QUINCUNX_MATRIX_1,
    DP_VSI_3D_EXT_DATA_QUINCUNX_MATRIX_2,
    DP_VSI_3D_EXT_DATA_QUINCUNX_MATRIX_3
}DP_VSI_3D_EXT_DATA_E;

typedef enum
{
    DP_RESERVED,
    DP_VSI_VIC_4K2K_30HZ,
    DP_VSI_VIC_4K2K_25HZ,
    DP_VSI_VIC_4K2K_24HZ,
    DP_VSI_VIC_4K2K_24HZ_SMPTE
}DP_VSI_VIC_E;

typedef enum
{
    DP_WAKEUP_MODE_NO_WAKEUP = 0,
    DP_WAKEUP_MODE_BY_AUX,
    DP_WAKEUP_MODE_BY_SIGNAL,
    DP_WAKEUP_MODE_BY_AUX_OR_SIGNAL,
}DP_WAKEUP_MODE_E;

typedef struct
{
	unsigned char port;
	DP_INFOFRAME_HEADER_T header;
	unsigned char	checksum;
	unsigned char	ieee_reg_id[DP_VENDOR_SPECIFIC_REGID_LEN];
	unsigned char	payload[DP_VENDOR_SPECIFIC_PAYLOAD_LEN];
	DP_PACKET_STATUS_T packetStatus;				/**< DPRX VSI info */

//willychou add
	DP_VSI_VIDEO_FORMAT_E vidoe_fmt;
	DP_VSI_3D_STRUCTURE_E struct_3d;
	DP_VSI_3D_EXT_DATA_E extdata_3d;
	DP_VSI_VIC_E vic;
}DP_VSI_T;

//--------------------------------------------------------
// Extension Control
//--------------------------------------------------------
typedef enum
{
    // EOTF
    DP_EXCTL_OVERRIDE_EOTF_EN = 0,
    DP_EXCTL_OVERRIDE_EOTF,

    // PHY
    DP_EXCTL_MANUAL_EQ_EN,

    // HDCP
    DP_EXCTL_DISABLE_HDCP14,
    DP_EXCTL_DISABLE_HDCP22,
    DP_EXCTL_FRORCE_REAUTH_HDCP22,
    DP_EXCTL_MAX,

    //WatchDag
    DP_EXCTL_WATCH_DOG_ENABLE,
}DP_EXCTL_ID;

typedef struct
{
    DP_EXCTL_ID id;
    unsigned char param1;
    unsigned char param2;
    unsigned char param3;
}DP_EXT_CTRL;

#ifdef __cplusplus
}
#endif

#endif  //__DPRX_TYPES_H__
