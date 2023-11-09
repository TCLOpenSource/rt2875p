#ifndef __RTK_DDCCI_VERIFY_API_H__
#define __RTK_DDCCI_VERIFY_API_H__

#include <ioctrl/ddcci/ddcci_cmd_id.h>

#define RTK_DDCCI_RXBUF_LENGTH                                     DDC_BUF_LENGTH //32
#define RTK_DDCCI_TXBUF_LENGTH                                     DDC_BUF_LENGTH //32

#define RTK_DDCCI_CAPABILITIES_STRING_HDMI                               "(prot(monitor)type(LCD)model(RTK)cmds(01 02 03 07 0C E3 F3)vcp(02 04 05 06 08 0B 0C 10 12 14(01 02 04 05 06 08 0B) 16 18 1A 52 60(01 03 04 0F 10 11 12) 87 AC AE B2 B6 C6 C8 CA CC(01 02 03 04 06 0A 0D) D6(01 04 05) DF FD FF)mswhql(1)asset_eep(40)mccs_ver(2.2))"
#define RTK_DDCCI_CAPABILITIES_STRING_DP                                 "(prot(monitor)type(LCD)model(RTK)cmds(01 02 03 07 0C E3 F3)vcp(02 04 05 06 08 0B 0C 10 12 14(01 02 04 05 06 08 0B) 16 18 1A 52 60(01 03 04 0F 10 11 12) 87 AC AE B2 B6 C6 C8 CA CC(01 02 03 04 06 0A 0D) D6(01 04 05) DF FD FF)mswhql(1)asset_eep(40)mccs_ver(2.2))"

#define RTK_DDCCI_UNIFORMITY_SUPPORT         RTK_DDCCI_OFF
#define RTK_DDCCI_UNIFORMITY_LEVEL0_SUPPORT  RTK_DDCCI_OFF
#define RTK_DDCCI_BACKLIGHT_DIMMING_SUPPORT  RTK_DDCCI_OFF

#define RTK_DDCCI_TRUE                                   1
#define RTK_DDCCI_FALSE                                  0
#define RTK_DDCCI_ENABLE                                 1
#define RTK_DDCCI_DISABLE                                0
#define RTK_DDCCI_ON                                     1
#define RTK_DDCCI_OFF                                    0
#define RTK_DDCCI_SUCCESS                                1
#define RTK_DDCCI_FAIL                                   0


//****************************************************************************
// Global Macros
//****************************************************************************
#define LOBYTE(w)                               ((unsigned char)(w))
#define HIBYTE(w)                               ((unsigned char)(((unsigned int)(w) >> 8) & 0x00FF))
#define LOWORD(dw)                              ((unsigned int)(dw))
#define HIWORD(dw)                              ((unsigned int)((dw) >> 16))

//--------------------------------------------------
// Definitions of Bits
//--------------------------------------------------
#define RTK_DDCCI_BIT0                                   0x0001U
#define RTK_DDCCI_BIT1                                   0x0002U

typedef enum
{
    DDCCI_PORT_NONE = 0x00,
    DDCCI_PORT_HDMI,
    DDCCI_PORT_DP,
} DDCCI_INPUT_PORT_TYPE_T;


//****************************************************************************
// LAYER DEFINITIONS
//****************************************************************************
//--------------------------------------------------
// Definitions of DDCCI Address
//--------------------------------------------------
#define RTK_DDCCI_DEST_ADDRESS                         0x6E
#define RTK_DDCCI_SRC_ADDRESS                          0x51
#define RTK_DDCCI_CONTROL_STATUS_FLAG                  0x80
#define RTK_DDCCI_SRC_RTK_ADDRESS                      0x71
#define RTK_DDCCI_RTK_CMD_TYPE                         0x77

//--------------------------------------------------
// Definitions of DDCCI Protocol Items
//--------------------------------------------------
#define RTK_DDCCI_SOURCE                               0
#define RTK_DDCCI_LENGTH                               1
#define RTK_DDCCI_COMMAND                              2
#define RTK_DDCCI_SOURCE_OPCODE                        3

#define RTK_DDCCI_RESULT_CODE                          3
#define RTK_DDCCI_SINK_OPCODE                          4
#define RTK_DDCCI_TYPE_CODE                            5
#define RTK_DDCCI_MAX_HIGH_BYTE                        6
#define RTK_DDCCI_MAX_LOW_BYTE                         7
#define RTK_DDCCI_PRESENT_HIGH_BYTE                    8
#define RTK_DDCCI_PRESENT_LOW_BYTE                     9

#define RTK_DDCCI_SET_HIGH_BYTE                        4
#define RTK_DDCCI_SET_LOW_BYTE                         5

#define RTK_DDCCI_OFS_HIGH_BYTE                        3
#define RTK_DDCCI_OFS_LOW_BYTE                         4

#define RTK_DDCCI_TIMING_MSG_COMMAND                   1
#define RTK_DDCCI_TIMING_MSG_OPCODE                    2
#define RTK_DDCCI_TIMING_STATUS                        3
#define RTK_DDCCI_TIMING_HFREQ_HBYTE                   4
#define RTK_DDCCI_TIMING_HFREQ_LBYTE                   5
#define RTK_DDCCI_TIMING_VFREQ_HBYTE                   6
#define RTK_DDCCI_TIMING_VFREQ_LBYTE                   7

//--------------------------------------------------
// Definitions of DDCCI Commands
//--------------------------------------------------
#define RTK_DDCCI_CMD_GET_VCP_FEATURE                  0x01
#define RTK_DDCCI_CMD_GET_VCP_FEATURE_REPLY            0x02
#define RTK_DDCCI_CMD_SET_VCP_FEATURE                  0x03
#define RTK_DDCCI_CMD_REPLY_TIMING_MESSAGE             0x06
#define RTK_DDCCI_CMD_GET_TIMING_REPORT                0x07
#define RTK_DDCCI_CMD_SAVE_CURRENT_SETTINGS            0x0C
#define RTK_DDCCI_CMD_CAPABILITIES_REQUEST_REPLY       0xE3
#define RTK_DDCCI_CMD_CAPABILITIES_REQUEST             0xF3
#define RTK_DDCCI_CMD_SWITCH_DEBUG_SALVE_REQUEST       0xAA
#define RTK_DDCCI_CMD_GET_FW_INFO_REQUEST              0xC9
#define RTK_DDCCI_CMD_SET_SMBUS_BLOCK_REQUEST          0xC8

#define RTK_DDCCI_OPCODE_GET_CURRENT_FW_VER_REQUEST    0x01
#define RTK_DDCCI_OPCODE_GET_FW_VER_LOCATE_REQUEST     0x02
#define RTK_DDCCI_OPCODE_GET_DUAL_BANK_INFO_REQUEST    0x09
#define RTK_DDCCI_OPCODE_SWTICH_DEBUG_SALVE_REQUEST    0x05


#define RTK_DDCCI_VIRTUAL_HOST_ADDRESS                 0x50

//--------------------------------------------------
// Definitions of DDCCI VCP Opcode
//--------------------------------------------------
#define RTK_DDCCI_OPCODE_VCP_CODE_PAGE                 0x00
#define RTK_DDCCI_OPCODE_VCP_DEGAUSS                   0x01
#define RTK_DDCCI_OPCODE_VCP_NEW_CONTROL_VALUE         0x02
#define RTK_DDCCI_OPCODE_VCP_SOFT_CONTROLS             0x03
#define RTK_DDCCI_OPCODE_VCP_FACTORY_RESET             0x04
#define RTK_DDCCI_OPCODE_VCP_RECAL_BRI_CON             0x05
#define RTK_DDCCI_OPCODE_VCP_GEOMETRY_RESET            0x06
#define RTK_DDCCI_OPCODE_VCP_COLOR_RESET               0x08
#define RTK_DDCCI_OPCODE_VCP_FACTORY_TV_RESET          0x0A
#define RTK_DDCCI_OPCODE_VCP_COLOR_TEMP_INCREMENT      0x0B
#define RTK_DDCCI_OPCODE_VCP_COLOR_TEMP_REQUEST        0x0C
#define RTK_DDCCI_OPCODE_VCP_CLOCK                     0x0E
#define RTK_DDCCI_OPCODE_VCP_BACKLIGHT                 0x10
#define RTK_DDCCI_OPCODE_VCP_FLESH_TONE_ENHANCEMENT    0x11
#define RTK_DDCCI_OPCODE_VCP_CONTRAST                  0x12
#define RTK_DDCCI_OPCODE_VCP_SELECT_COLOR_PRESET       0x14
#define RTK_DDCCI_OPCODE_VCP_RED_GAIN                  0x16
#define RTK_DDCCI_OPCODE_VCP_COLOR_VISION_COMPENSATION 0x17
#define RTK_DDCCI_OPCODE_VCP_GREEN_GAIN                0x18
#define RTK_DDCCI_OPCODE_VCP_BLUE_GAIN                 0x1A
#define RTK_DDCCI_OPCODE_VCP_FOCUS                     0x1C
#define RTK_DDCCI_OPCODE_VCP_AUTO_SET_UP               0x1E
#define RTK_DDCCI_OPCODE_VCP_AUTO_COLOR                0x1F
#define RTK_DDCCI_OPCODE_VCP_HPOSITION                 0x20
#define RTK_DDCCI_OPCODE_VCP_HSIZE                     0x22
#define RTK_DDCCI_OPCODE_VCP_HPINCUSHION               0x24
#define RTK_DDCCI_OPCODE_VCP_HPINCUSHION_BALANCE       0x26
#define RTK_DDCCI_OPCODE_VCP_HCONVERGENCE_RB           0x28
#define RTK_DDCCI_OPCODE_VCP_HCONVERGENCE_MG           0x29
#define RTK_DDCCI_OPCODE_VCP_HLINEARITY                0x2A
#define RTK_DDCCI_OPCODE_VCP_HLINEARITY_BALANCE        0x2C
#define RTK_DDCCI_OPCODE_VCP_GRAY_SCALE_EXPANSION      0x2E
#define RTK_DDCCI_OPCODE_VCP_VPOSITION                 0x30
#define RTK_DDCCI_OPCODE_VCP_VSIZE                     0x32
#define RTK_DDCCI_OPCODE_VCP_VPINCUSHION               0x34
#define RTK_DDCCI_OPCODE_VCP_VPINCUSHION_BALANCE       0x36
#define RTK_DDCCI_OPCODE_VCP_VCONVERGENCE_RB           0x38
#define RTK_DDCCI_OPCODE_VCP_VCONVERGENCE_MG           0x39
#define RTK_DDCCI_OPCODE_VCP_VLINEARITY                0x3A
#define RTK_DDCCI_OPCODE_VCP_VLINEARITY_BALANCE        0x3C
#define RTK_DDCCI_OPCODE_VCP_CLOCK_PHASE               0x3E
#define RTK_DDCCI_OPCODE_VCP_HPARALLELOGRAM            0x40
#define RTK_DDCCI_OPCODE_VCP_VPARALLELOGRAM            0x41
#define RTK_DDCCI_OPCODE_VCP_HKEYSTONE                 0x42
#define RTK_DDCCI_OPCODE_VCP_VKEYSTONE                 0x43
#define RTK_DDCCI_OPCODE_VCP_ROTATION                  0x44
#define RTK_DDCCI_OPCODE_VCP_TOP_CORNER_FLARE          0x46
#define RTK_DDCCI_OPCODE_VCP_TOP_CORNER_HOOK           0x48
#define RTK_DDCCI_OPCODE_VCP_BOTTOM_CORNER_FLARE       0x4A
#define RTK_DDCCI_OPCODE_VCP_BOTTOM_CORNER_HOOK        0x4C
#define RTK_DDCCI_OPCODE_TIMING_MSG                    0x4E
#define RTK_DDCCI_OPCODE_VCP_ACTIVE_CONTROL            0x52
#define RTK_DDCCI_OPCODE_VCP_COLORTEMP                 0x54
#define RTK_DDCCI_OPCODE_VCP_HMOIRE                    0x56
#define RTK_DDCCI_OPCODE_VCP_VMOIRE                    0x58
#define RTK_DDCCI_OPCODE_VCP_SAT_RED                   0x59
#define RTK_DDCCI_OPCODE_VCP_SAT_YELLOW                0x5A
#define RTK_DDCCI_OPCODE_VCP_SAT_GREEN                 0x5B
#define RTK_DDCCI_OPCODE_VCP_SAT_CYAN                  0x5C
#define RTK_DDCCI_OPCODE_VCP_SAT_BLUE                  0x5D
#define RTK_DDCCI_OPCODE_VCP_SAT_MAGENTA               0x5E
#define RTK_DDCCI_OPCODE_VCP_INPUT_SOURCE              0x60
#define RTK_DDCCI_OPCODE_VCP_AUDIO_VOLUME              0x62
#define RTK_DDCCI_OPCODE_VCP_SPEAKER_SELECT            0x63
#define RTK_DDCCI_OPCODE_VCP_MICROPHONE_VOLUME         0x64
#define RTK_DDCCI_OPCODE_VCP_AMBIENT_LIGHT_SENSOR      0x66
#define RTK__DDCCI_OPCODE_VCP_BACKLIGHT_LEVEL_WHITE     0x6B
#define RTK_DDCCI_OPCODE_VCP_RED_BLACK_LEVEL           0x6C
#define RTK__DDCCI_OPCODE_VCP_BACKLIGHT_LEVEL_RED       0x6D
#define RTK_DDCCI_OPCODE_VCP_GREEN_BLACK_LEVEL         0x6E
#define RTK__DDCCI_OPCODE_VCP_BACKLIGHT_LEVEL_GREEN     0x6F
#define RTK_DDCCI_OPCODE_VCP_BLUE_BLACK_LEVEL          0x70
#define RTK__DDCCI_OPCODE_VCP_BACKLIGHT_LEVEL_BLUE      0x71
#define RTK_DDCCI_OPCODE_VCP_GAMMA                     0x72
#define RTK_DDCCI_OPCODE_VCP_LUT_SIZE                  0x73
#define RTK_DDCCI_OPCODE_VCP_SINGLE_POINT_LUT          0x74
#define RTK_DDCCI_OPCODE_VCP_BLOCK_LUT                 0x75
#define RTK_DDCCI_OPCODE_VCP_REMOTE_PROC_CALL          0x76
#define RTK_DDCCI_OPCODE_VCP_ID_DATA_OPERATION         0x78
#define RTK_DDCCI_OPCODE_VCP_ADJUST_ZOOM               0x7C
#define RTK_DDCCI_OPCODE_VCP_PRODUCTKEY                0x7E
#define RTK_DDCCI_OPCODE_VCP_HMIRROR                   0x82
#define RTK_DDCCI_OPCODE_VCP_VMIRROR                   0x84
#define RTK_DDCCI_OPCODE_VCP_DISPLAY_SCALING           0x86
#define RTK_DDCCI_OPCODE_VCP_SHARPNESS                 0x87
#define RTK_DDCCI_OPCODE_VCP_VELOCITY_SCAN_MODULATION  0x88
#define RTK_DDCCI_OPCODE_VCP_TV_SATURATION             0x8A
#define RTK_DDCCI_OPCODE_VCP_TV_CHANNEL_UP_DOWN        0x8B
#define RTK_DDCCI_OPCODE_VCP_TV_SHARPNESS              0x8C
#define RTK_DDCCI_OPCODE_VCP_AUDIO_MUTE                0x8D
#define RTK_DDCCI_OPCODE_VCP_TV_CONTRAST               0x8E
#define RTK_DDCCI_OPCODE_VCP_AUDIO_TREBLE              0x8F
#define RTK_DDCCI_OPCODE_VCP_HUE                       0x90
#define RTK_DDCCI_OPCODE_VCP_AUDIO_BASS                0x91
#define RTK_DDCCI_OPCODE_VCP_TV_BLACK_LEVEL            0x92
#define RTK_DDCCI_OPCODE_VCP_AUDIO_BALANCE             0x93
#define RTK_DDCCI_OPCODE_VCP_AUDIO_PROCESSOR_MODE      0x94
#define RTK_DDCCI_OPCODE_VCP_WINDOW_POSITION_TL_X      0x95
#define RTK_DDCCI_OPCODE_VCP_WINDOW_POSITION_TL_Y      0x96
#define RTK_DDCCI_OPCODE_VCP_WINDOW_POSITION_BR_X      0x97
#define RTK_DDCCI_OPCODE_VCP_WINDOW_POSITION_BR_Y      0x98
#define RTK_DDCCI_OPCODE_VCP_WINDOW_BACKGROUND         0x9A
#define RTK_DDCCI_OPCODE_VCP_RED                       0x9B
#define RTK_DDCCI_OPCODE_VCP_YELLOW                    0x9C
#define RTK_DDCCI_OPCODE_VCP_GREEN                     0x9D
#define RTK_DDCCI_OPCODE_VCP_CYAN                      0x9E
#define RTK_DDCCI_OPCODE_VCP_BLUE                      0x9F
#define RTK_DDCCI_OPCODE_VCP_MAGENDA                   0xA0
#define RTK_DDCCI_OPCODE_VCP_AUTO_SETUP                0xA2
#define RTK_DDCCI_OPCODE_VCP_WINDOW_MASK_CONTROL       0xA4
#define RTK_DDCCI_OPCODE_VCP_WINDOW_SELECT             0xA5
#define RTK_DDCCI_OPCODE_VCP_WINDOW_SIZE               0xA6
#define RTK_DDCCI_OPCODE_VCP_WINDOW_TRANSPARENCY       0xA7
#define RTK_DDCCI_OPCODE_VCP_SYNC_TYPE                 0xA8
#define RTK_DDCCI_OPCODE_VCP_HFREQ                     0xAC
#define RTK_DDCCI_OPCODE_VCP_VFREQ                     0xAE
#define RTK_DDCCI_OPCODE_VCP_STORE_SETTING             0xB0
#define RTK_DDCCI_OPCODE_VCP_PANEL_TYPE                0xB2
#define RTK_DDCCI_OPCODE_VCP_RGB_ORDERING              0xB4
#define RTK_DDCCI_OPCODE_VCP_COLOR_CODING              0xB5
#define RTK_DDCCI_OPCODE_VCP_MONITOR_TYPE              0xB6
#define RTK_DDCCI_OPCODE_VCP_MONITOR_STATUS            0xB7
#define RTK_DDCCI_OPCODE_VCP_PACKET_COUNT              0xB8
#define RTK_DDCCI_OPCODE_VCP_MONITOR_X_ORIGIN          0xB9
#define RTK_DDCCI_OPCODE_VCP_MONITOR_Y_ORIGIN          0xBA
#define RTK_DDCCI_OPCODE_VCP_HEADER_ERROR_COUNT        0xBB
#define RTK_DDCCI_OPCODE_VCP_BODY_CRC_ERROR_COUNT      0xBC
#define RTK_DDCCI_OPCODE_VCP_CLIENT_ID                 0xBD
#define RTK_DDCCI_OPCODE_VCP_LINK_CONTROL              0xBE
#define RTK_DDCCI_OPCODE_VCP_USAGE_TIME                0xC0
#define RTK_DDCCI_OPCODE_VCP_DISP_DESCRIPTOR_LENGTH    0xC2
#define RTK_DDCCI_OPCODE_VCP_TRANSMIT_DISP_DESCRIPTOR  0xC3
#define RTK_DDCCI_OPCODE_VCP_DISP_DESCRIPTOR_EN        0xC4
#define RTK_DDCCI_OPCODE_VCP_APPLICATION_ENABLE        0xC6
#define RTK_DDCCI_OPCODE_VCP_DISP_ENABLE_KEY           0xC7
#define RTK_DDCCI_OPCODE_VCP_CONTROLLER_TYPE           0xC8
#define RTK_DDCCI_OPCODE_VCP_FW_LEVEL                  0xC9
#define RTK_DDCCI_OPCODE_VCP_OSD_ON_OFF                0xCA
#define RTK_DDCCI_OPCODE_VCP_OSD_LANGUAGE              0xCC
#define RTK_DDCCI_OPCODE_VCP_STATUS_INDICATOR          0xCD
#define RTK_DDCCI_OPCODE_VCP_AUXILIARY_DISP_SIZE       0xCE
#define RTK_DDCCI_OPCODE_VCP_AUXILIARY_DISP_DATA       0xCF
#define RTK_DDCCI_OPCODE_VCP_OUTPUT_SELECT             0xD0
#define RTK_DDCCI_OPCODE_VCP_ASSET_TAG                 0xD2
#define RTK_DDCCI_OPCODE_VCP_STEREO_VIDEO_MODE         0xD4
#define RTK_DDCCI_OPCODE_VCP_POWER_MODE                0xD6
#define RTK_DDCCI_OPCODE_VCP_AUXILIARY_POWER_OUTPUT    0xD7
#define RTK_DDCCI_OPCODE_VCP_SCAN_MODE                 0xDA
#define RTK_DDCCI_OPCODE_VCP_IMAGE_MODE                0xDB
#define RTK_DDCCI_OPCODE_VCP_DISPLAY_APPLICATION       0xDC
#define RTK_DDCCI_OPCODE_VCP_SCRACHPAD                 0xDE
#define RTK_DDCCI_OPCODE_VCP_VERSION                   0xDF
#define RTK_DDCCI_OPCODE_VCP_CONTROL_LOCK              0xE3
#define RTK_DDCCI_OPCODE_VCP_MANUFACTURER              0xFD

#define RTK_DDCCI_OPCODE_VCP_DELL_DDM_SUPPORT          0xF1
#define RTK_DDCCI_OPCODE_VCP_DISP_CALIBRATION          0xFC

//--------------------------------------------------
// Definitions of Display Calibration Protocol Items
//--------------------------------------------------
// Display Calibration Start
#define RTK_DDCCI_DISP_CALIB_STEP                      4
#define RTK_DDCCI_DISP_CALIB_TYPE                      5
#define RTK_DDCCI_DISP_CALIB_OCC_COLORSPACE            6
#define RTK_DDCCI_DISP_CALIB_HDR_FLAG                  7
#define RTK_DDCCI_DISP_CALIB_TYPE_II                   8

// Display Calibration Save RGB Gain
#define RTK_DDCCI_DISP_CALIB_RGB_GAIN                  5

// Display Calibration Measure Gray Patterns
#define RTK_DDCCI_DISP_CALIB_LUT_LENGTH                5
#define RTK_DDCCI_DISP_CALIB_LUT_ID                    6

// Display Calibration OCC Matrix
#define RTK_DDCCI_DISP_CALIB_OCC_MATRIX                6

// Display Calibration Show Gray1~7 10bit
#define RTK_DDCCI_DISP_CALIB_GRAY_HIGH                 5
#define RTK_DDCCI_DISP_CALIB_GRAY_LOW                  6

// Display Calibration Show Results
#define RTK_DDCCI_DISP_CALIB_FINAL_TYPE                5
#define RTK_DDCCI_DISP_CALIB_OGC_CHKSUM                6
#define RTK_DDCCI_DISP_CALIB_OCC_CHKSUM                7
#define RTK_DDCCI_DISP_CALIB_DICOM_CHKSUM              8
#define RTK_DDCCI_DISP_CALIB_OGC_GAMMA_CT_INFO         9
#define RTK_DDCCI_DISP_CALIB_OGC_RGBGAMMA_CHKSUM       10
#define RTK_DDCCI_DISP_CALIB_DIMMING_OCC_CHKSUM        11
#define RTK_DDCCI_DISP_CALIB_FREESYNC_II_CHKSUM        12
#define RTK_DDCCI_DISP_CALIB_RGB_GAIN_CHKSUM           6
#define RTK_DDCCI_DISP_CALIB_HEADER_CHKSUM             7

#define RTK_DDCCI_DISP_CALIB_END_TYPE                  5

// Display Calibration Verification
#define RTK_DDCCI_DISP_CALIB_VERIFY_RED                5
#define RTK_DDCCI_DISP_CALIB_VERIFY_GREEN              7
#define RTK_DDCCI_DISP_CALIB_VERIFY_BLUE               9

// Display Calibration Set FW Info
#define RTK_DDCCI_DISP_CALIB_FW_INFO                   5

// Display Calibration Set Gamma
#define RTK_DDCCI_DISP_CALIB_INDEX                     5

// Display Calibration Set PCM
#define RTK_DDCCI_DISP_CALIB_PCM_COLOR_DOMAIN          5

// Display Calibration Reply Project Support
#define RTK_DDCCI_DISP_CALIB_OGCOCCDICOM_SUPPORT       2
#define RTK_DDCCI_DISP_CALIB_OGC_TOTAL_GAMMA           3
#define RTK_DDCCI_DISP_CALIB_OGC_TOTAL_CT              4
#define RTK_DDCCI_DISP_CALIB_PANEL_BITNUM              5

// Display Calibration Reply FW Version
#define RTK_DDCCI_DISP_CALIB_FW_VERSION_H              2
#define RTK_DDCCI_DISP_CALIB_FW_VERSION_L              3

// Display Calibration Reply No Tone Mapping Info
#define RTK_DDCCI_DISP_CALIB_MAX_LV                    2
#define RTK_DDCCI_DISP_CALIB_MAX_FRAME_AVE_LV          3
#define RTK_DDCCI_DISP_CALIB_MIN_LV                    4

// Display Calibration Reply FW Mode
#define RTK_DDCCI_DISP_CALIB_FW_SUPPORT                2

// Display Calibration Reply Progress
#define RTK_DDCCI_DISP_CALIB_STRING_1                  2
#define RTK_DDCCI_DISP_CALIB_STRING_2                  3
#define RTK_DDCCI_DISP_CALIB_STRING_3                  4
#define RTK_DDCCI_DISP_CALIB_STRING_4                  5

// Display Calibration Write Uniformity LUT
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_INDEX          5
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_CHKSUM         6
// Save HDR IG LUT
#define RTK_DDCCI_DISP_CALIB_IGLUT_INDEX               5
#define RTK_DDCCI_DISP_CALIB_IGLUT_START               6
#define RTK_DDCCI_DISP_CALIB_HDRMATRIX_START           5
#define RTK_DDCCI_DISP_CALIB_SET_HDR_IG_INDEX          5
#define RTK_DDCCI_DISP_CALIB_SET_HDR_COLOR_MATRIX      6
#define RTK_DDCCI_DISP_CALIB_HDR10_NOTM_BOUND_INDEX    5

#define RTK_DDCCI_DISP_CALIB_COLORMATRIX_TYPE          5
#define RTK_DDCCI_DISP_CALIB_COLORMATRIX_CHECKSUM      6

#define RTK_DDCCI_DISP_CALIB_COLORMATRIX2_TYPE         5
#define RTK_DDCCI_DISP_CALIB_COLORMATRIX2_NUM          6
#define RTK_DDCCI_DISP_CALIB_COLORMATRIX2_CHECKSUM     7

//Save Freesync II EDID info
#define RTK_DDCCI_DISP_CALIB_EDID_INDEX                5
#define RTK_DDCCI_DISP_CALIB_FREESYNC_II_IGLUT_INDEX   5
#define RTK_DDCCI_DISP_CALIB_FREESYNC_II_IGLUT_START   6
#define RTK_DDCCI_DISP_CALIB_FREESYNC_II_MATRIX_START  5

//Save DIMMING NOTM DCR THD
#define RTK_DDCCI_DISP_CALIB_NOTM_DCR_THD_INDEX        5

//Set Backlight
#define RTK__DDCCI_DISP_CALIB_BACKLIGHT_INDEX           5

// Set Boost
#define RTK_DDCCI_DISP_CALIB_BOOST_INDEX               5

//OGC OCC all mode Lv Ratio Data
#define RTK_DDCCI_DISP_CALIB_LVRATIO_INDEX             5

// OCC Multi Panel Color Matching Data Access
#define RTK_DDCCI_DISP_CALIB_COLOR_MATCHING_INDEX      5

// dimming Data start index
#define RTK_DDCCI_DISP_DIMMING_TYPE_LOCAL              0
#define RTK_DDCCI_DISP_DIMMING_TYPE_GLOBAL             1
#define RTK_DDCCI_DISP_CALIB_DIMMING_INDEX             5
#define RTK_DDCCI_DISP_CALIB_DIMMING_DATA_START        6

//Get Panel Info Command Type
#define RTK_DDCCI_DISP_CALIB_GET_FW_DEFINE             0
#define RTK_DDCCI_DISP_CALIB_GET_FW_VERSION            1
#define RTK_DDCCI_DISP_CALIB_GET_PANEL_LV_INFO         2
#define RTK_DDCCI_DISP_CALIB_GET_UNIFORMITY_FW_VERSION 3
#define RTK_DDCCI_DISP_CALIB_GET_FW_SUPPORT            4

// LD Boost Command Type
#define RTK_DDCCI_DISP_CALIB_BOOST_OFF                 0
#define RTK_DDCCI_DISP_CALIB_BOOST_ON                  1

// OCC Multi Panel Color Matching Data COMMAND
#define RTK_DDCCI_DISP_CALIB_COLOR_MATCHING_WRITE      0
#define RTK_DDCCI_DISP_CALIB_COLOR_MATCHING_READ       1
#define RTK_DDCCI_DISP_CALIB_COLOR_MATCHING_ERASE      2

//--------------------------------------------------
// Definitions of Display Calibration Opcode
//--------------------------------------------------
#define RTK_DDCCI_DISP_CALIB_OGC_MEASURE_RED           0x00
#define RTK_DDCCI_DISP_CALIB_OGC_MEASURE_BLACK         0x01
#define RTK_DDCCI_DISP_CALIB_OGC_MEASURE_BLUE          0x02
#define RTK_DDCCI_DISP_CALIB_OGC_MEASURE_GREEN         0x03
#define RTK_DDCCI_DISP_CALIB_OGC_MEASURE_WHITE         0x04
#define RTK_DDCCI_DISP_CALIB_OGC_MEASURE_GRAY1         0x05
#define RTK_DDCCI_DISP_CALIB_OGC_MEASURE_GRAY2         0x06
#define RTK_DDCCI_DISP_CALIB_OGC_MEASURE_GRAY3         0x07
#define RTK_DDCCI_DISP_CALIB_OGC_MEASURE_LAST_GRAY     0x14
#define RTK_DDCCI_DISP_CALIB_OGC_MEASURE_END           0xAA
#define RTK_DDCCI_DISP_CALIB_OGC_MEASURE_GRAY1TO7      0xA0

#define RTK_DDCCI_DISP_CALIB_END                       0x8F

#define RTK_DDCCI_DISP_CALIB_OCC_MEASURE_SRGB1         0x1D
#define RTK_DDCCI_DISP_CALIB_OCC_MEASURE_SRGB7         0x23
#define RTK_DDCCI_DISP_CALIB_OCC_MEASURE_SRGB21        0x31

#define RTK_DDCCI_DISP_CALIB_OGC_VERIFY_END            0x48
#define RTK_DDCCI_DISP_CALIB_OCC_VERIFY_END            0x50

#define RTK_DDCCI_DISP_CALIB_VERIFICATION              0xBB
#define RTK_DDCCI_DISP_CALIB_SET_GAMMA                 0xCC
#define RTK_DDCCI_DISP_CALIB_SET_PCM                   0xDD
#define RTK_DDCCI_DISP_CALIB_SET_DICOM                 0xEE
#define RTK_DDCCI_DISP_CALIB_GET_DEFINE                0xFF

#define RTK_DDCCI_DISP_CALIB_MEASURE_BG                0xAB
#define RTK_DDCCI_DISP_CALIB_BG_OFF                    0xAC
#define RTK_DDCCI_DISP_CALIB_GET_FW_INFO               0xAD

#define RTK_DDCCI_DISP_CALIB_SAVE_IGLUT                0xBC
#define RTK_DDCCI_DISP_CALIB_SAVE_HDR_MATRIX           0xBD
#define RTK_DDCCI_DISP_CALIB_CHK_COLORMATRIX           0xBE
#define RTK_DDCCI_DISP_CALIB_SET_HDR_MODE              0xBF

#define RTK_DDCCI_DISP_CALIB_LD_COMMAND                0xC0
#define RTK_DDCCI_DISP_CALIB_LD_BOOST_COMMAND          0xC1

#define RTK_DDCCI_DISP_CALIB_SET_BACKLIGHT             0xCD

#define RTK_DDCCI_DISP_CALIB_SAVE_FREESYNC_II_IGLUT    0xDB
#define RTK_DDCCI_DISP_CALIB_SAVE_PANEL_NATIVE_MATRIX  0xDC
#define RTK_DDCCI_DISP_CALIB_SET_FREESYNC_II           0xDE
#define RTK_DDCCI_DISP_CALIB_SAVE_EDID_INFO            0xDF
#define RTK_DDCCI_DISP_CALIB_SAVE_HDR10_NOTM_BOUNG     0xD0
#define RTK_DDCCI_DISP_CALIB_FREESYNC_II_MEASURE_WHITE 0xD1
#define RTK_DDCCI_DISP_CALIB_SAVE_NOTM_DCR_THD         0xD2
#define RTK_DDCCI_DISP_CALIB_PANEL_COLOR_MATCHING_DATA 0xD3
#define RTK_DDCCI_DISP_CALIB_PANEL_OGCOCC_LVRATIO      0xD4

#if(RTK_DDCCI_UNIFORMITY_SUPPORT == RTK_DDCCI_ON)
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_GET_OGC_HEADER 0x90
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_ERASE_FLASH    0x91
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_SET_GAMMA      0x92
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_GET_PCM_MATRIX 0x93
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_WRITE_START    0x94
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_WRITE          0x95
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_WRITE_END      0x96
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_VERIFY_START   0x97
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_VERIFY_END     0x98
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_GET_DEFINE     0x99
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_GET_CCT_GAIN   0x9A
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_GET_FM_DATA    0x9F
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_SET_PCM        0xA1
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_GET_LVRATIO    0xA2
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_GET_LEVEL_MODE 0xA3
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_GET_NEWLVRATIO 0xA4
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_SET_IG_GAMMA   0xA5

// level0
#if(RTK_DDCCI_UNIFORMITY_LEVEL0_SUPPORT == RTK_DDCCI_ON)
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_W_OFFSET       0x9B
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_W_OFFSET_END   0x9C
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_W_DECAY        0x9D
#define RTK_DDCCI_DISP_CALIB_UNIFORMITY_W_DECAY_END    0x9E
#endif// #if(RTK_DDCCI_UNIFORMITY_LEVEL0_SUPPORT == RTK_DDCCI_ON)
#endif

#if(RTK_DDCCI_BACKLIGHT_DIMMING_SUPPORT == RTK_DDCCI_ON)
#define RTK_DDCCI_DISP_CALIB_DIMMING_START             0xE0
#define RTK_DDCCI_DISP_CALIB_DIMMING_END               0xE1
#define RTK_DDCCI_DISP_CALIB_DIMMING_CHECKFW           0xE2
#define RTK_DDCCI_DISP_CALIB_DIMMING_SETPWM            0xE3
#define RTK_DDCCI_DISP_CALIB_DIMMING_SHOW_WHITE        0xE4
#define RTK_DDCCI_DISP_CALIB_DIMMING_SHOW_BLACK        0xE5
#endif

//****************************************************************************
// FUNCTION DECLARATIONS
//****************************************************************************
void rtk_ddcci_user_init_tx(void);
void rtk_ddcci_user_init_rx(void);
unsigned char rtk_ddcci_user_cal_checksum(void);
void rtk_ddcci_user_set_vcp_reply_value(unsigned char type_opcode, unsigned int max, unsigned int present);
void rtk_ddcci_user_get_timing_report(void);
void rtk_ddcci_user_get_capabilities_request(void);
void rtk_ddcci_user_get_cp_string_data(unsigned char *dest, DDCCI_INPUT_PORT_TYPE_T port_type, unsigned int start_index, unsigned int length);
unsigned int rtk_ddcci_user_get_cp_string_size(DDCCI_INPUT_PORT_TYPE_T port_type);

//****************************************************************************
//for test verify
//****************************************************************************
unsigned int rtk_ddcci_user_adjust_percent_to_real_value(unsigned char percent_value, unsigned int max, unsigned int min, unsigned int center);
unsigned char rtk_ddcci_user_adjust_real_value_to_percent(unsigned int real_value, unsigned int max, unsigned int min, unsigned int center);
DDCCI_INPUT_PORT_TYPE_T rtk_ddcci_user_get_input_port_type(void);

#define rtk_ddcci_get_osd_status(x)         (RTK_DDCCI_ON)
#define rtk_ddcci_load_color_setting(x)
#define rtk_ddcci_save_color_setting(x)
#define rtk_ddcci_restore_bricon(x)
#define rtk_ddcci_restore_backlight(x)
#define rtk_ddcci_osd_disp_judge_source_type(x)         (0)
#define rtk_ddcci_get_power_status(x)         (RTK_DDCCI_POWER_STATUS_AC_ON)
#define rtk_ddcci_switch_input_port(x)
#define rtk_ddcci_adjust_brightness(x)
#define rtk_ddcci_adjust_contrast(x)
#define rtk_ddcci_adjust_backlight(x)
#define rtk_ddcci_adjust_h_position(x)
#define rtk_ddcci_adjust_v_position(x)
#define rtk_ddcci_disable_osd(x)
#define rtk_ddcci_enable_osd(x)
#define rtk_ddcci_set_target_power_status(x)
#define rtk_ddcci_disp_calib_process(x)

#endif //__RTK_DDCCI_VERIFY_API_H__
