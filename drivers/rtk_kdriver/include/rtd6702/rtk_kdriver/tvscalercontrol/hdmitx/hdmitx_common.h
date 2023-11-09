#ifndef _HDMITX_COMMON_H_
#define _HDMITX_COMMON_H_


typedef enum
{
    _VENDOR_MODEL = 0x00,
    _EDID_VERSION,
    _EDID_SEG_READ_SUPPORT,
    _YCC444_SUPPORT,
    _YCC422_SUPPORT,
    _FIRST_EXT_DTD_ADDR,
    _LPCM_FS_SUPPORT,
    _3D_LPCM_FS_SUPPORT,
    _VCDB_QY,
    _VCDB_QS,
    _YCC420_SUPPORT,
    _DRR_SUPPORT,
    _AI_SUPPORT,
    _DC48_SUPPORT,
    _DC36_SUPPORT,
    _DC30_SUPPORT,
    _DC_Y444_SUPPORT,
    _SCDC_PRESENT,
    _RR_CAPABLE,
    _LTE_340MCSC_SCRAMBLE,
    _DC48_420_SUPPORT,
    _DC36_420_SUPPORT,
    _DC30_420_SUPPORT,
    _HDMI_MAX_TMDS_CLK,
    _MAX_FRL_RATE,
    _DSC_VERSION_12,
    _DSC_NATIVE_420,
    _DSC_ALL_BPP,
    _DSC_16BPC,
    _DSC_12BPC,
    _DSC_10BPC,
    _DSC_MAX_FRL_RATE,
    _DSC_MAX_SLICES,
    _DSC_TOTAL_CHUNKKBYTE,
    _DID_EXT_SUPPORT,
    _SRC_PHYSICAL_ADDR,
    _VSD_8K_SUPPORT,
} EnumEdidFeatureType;

typedef enum{
    _POWER_ACTION_AC_ON_TO_NORMAL=0,
    _POWER_ACTION_PS_TO_NORMAL,
    _POWER_ACTION_OFF_TO_NORMAL,
    _POWER_ACTION_OFF_TO_FAKE_OFF,
    _POWER_ACTION_NORMAL_TO_PS,
    _POWER_ACTION_NORMAL_TO_OFF,
    _POWER_ACTION_AC_ON_TO_OFF,
    _POWER_ACTION_PS_TO_OFF
}EnumPowerAction;

typedef enum _EnumColorIMETryType{
    _COLORIMETRY_RGB_SRGB=0,
    _COLORIMETRY_RGB_XRRGB,
    _COLORIMETRY_RGB_SCRGB,
    _COLORIMETRY_RGB_ADOBERGB,
    _COLORIMETRY_RGB_DCI_P3,
    _COLORIMETRY_YCC_ITUR_BT709,
    _COLORIMETRY_YCC_SMPTE_170M,
    _COLORIMETRY_YCC_XVYCC601,
    _COLORIMETRY_YCC_XVYCC709,
    _COLORIMETRY_EXT,
    _COLORMETRY_RESERVED
}EnumColorIMETryType;


typedef enum _EnumColorIMETryExtType{
    _COLORIMETRY_EXT_RGB_SRGB=0,
    _COLORIMETRY_EXT_RGB_XRRGB,
    _COLORIMETRY_EXT_RGB_SCRGB,
    _COLORIMETRY_EXT_RGB_ADOBERGB,
    _COLORIMETRY_EXT_RGB_DCI_P3,
    _COLORIMETRY_EXT_RGB_ITUR_BT2020,
    _COLORIMETRY_EXT_YCC_ITUR_BT709,
    _COLORIMETRY_EXT_YCC_XVYCC601,
    _COLORIMETRY_EXT_YCC_XVYCC709,
    _COLORIMETRY_EXT_YCC_SYCC601,
    _COLORIMETRY_EXT_YCC_ADOBEYCC601,
    _COLORIMETRY_EXT_YCC_ITUR_BT2020_CL,
    _COLORIMETRY_EXT_YCC_ITUR_BT2020_NCL,
    _COLORIMETRY_EXT_RESERVED
}EnumColorIMETryExtType;


//--------------------------------------------------
// Enumerations of Quantization Range (RGB)
//--------------------------------------------------
typedef enum
{
    _RGB_QUANTIZATION_FULL_RANGE = 0x00,
    _RGB_QUANTIZATION_LIMIT_RANGE = 0x01,
    _RGB_QUANTIZATION_DEFAULT_RANGE = 0x02,
    _RGB_QUANTIZATION_RESERVED = 0x03,
} EnumColorRGBQuantization;

typedef enum
{
    _YCC_QUANTIZATION_LIMIT_RANGE = 0x00,
    _YCC_QUANTIZATION_FULL_RANGE = 0x01,
    _YCC_QUANTIZATION_RESERVED = 0x02,
} EnumColorYCCQuantization;

typedef enum _EnumOutputStreamType{
    _STREAM_TYPE_NONE=0,
    _STREAM_TYPE_SST,
    _STREAM_TYPE_DSC,
}EnumOutputStreamType;

typedef enum _EnumOutputPowerStatus{
    _POWER_STATUS_OFF = 0,
    _POWER_STATUS_NORMAL,
}EnumOutputPowerStatus;


typedef enum _EnumHdmi21FrlType{
    _HDMI21_FRL_NONE=0,
    _HDMI21_FRL_3G=1,
    _HDMI21_FRL_6G_3LANES,
    _HDMI21_FRL_6G_4LANES,
    _HDMI21_FRL_8G,
    _HDMI21_FRL_10G,
    _HDMI21_FRL_12G
}EnumHdmi21FrlType;

typedef enum _EnumHdmitxTimingStdType{
    _HDMITX_STD_HDMI=0,
    _HDMITX_STD_VESA,
    _HDMITX_STD_STB6500,
}EnumHdmitxTimingStdType;


typedef struct {
	unsigned int blank;
	unsigned int active;
	unsigned int sync;
	unsigned int front;
	unsigned int back;
} cea_timing_h_param_st;

typedef struct {
	unsigned int space;
	unsigned int blank;
	unsigned int act_video;
	unsigned int active;
	unsigned int sync;
	unsigned int front;
	unsigned int back;
} cea_timing_v_param_st;


typedef struct {
    unsigned int video_code;
    cea_timing_h_param_st h;
    cea_timing_v_param_st v;
    unsigned int pixelFreq;
} hdmi_tx_timing_gen_st;
#endif // #ifndef _HDMITX_COMMON_H_

