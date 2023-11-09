#ifndef __RTK_DDCCI_VERIFY_H__
#define __RTK_DDCCI_VERIFY_H__

#include <ioctrl/ddcci/ddcci_cmd_id.h>

#define RTK_DDCCI_AUDIO_SUPPORT  RTK_DDCCI_DISABLE
#define RTK_DDCCI_CONTRAST_SUPPORT  RTK_DDCCI_OFF
#define RTK_DDCCI_BRIGHTNESS_SUPPORT  RTK_DDCCI_OFF
#define RTK_DDCCI_SHARPNESS_SUPPORT  RTK_DDCCI_OFF


//--------------------------------------------------
// Definitions of Contrast Range (Real Value)
//--------------------------------------------------
#define RTK_DDCCI_CONTRAST_MAX                               178
#define RTK_DDCCI_CONTRAST_MIN                               78
#define RTK_DDCCI_CONTRAST_CENTER                            (((RTK_DDCCI_CONTRAST_MAX - RTK_DDCCI_CONTRAST_MIN) / 2) + RTK_DDCCI_CONTRAST_MIN)

//--------------------------------------------------
// Definitions of Backlight Range
//--------------------------------------------------
#define RTK_DDCCI_BACKLIGHT_MAX                              255
#define RTK_DDCCI_BACKLIGHT_MIN                              30
#define RTK_DDCCI_BACKLIGHT_CENTER                           (((RTK_DDCCI_BACKLIGHT_MAX - RTK_DDCCI_BACKLIGHT_MIN) / 2) + RTK_DDCCI_BACKLIGHT_MIN)

//--------------------------------------------------
// Definitions of Brightness Range
//--------------------------------------------------
#define RTK_DDCCI_BRIGHTNESS_MAX                             178
#define RTK_DDCCI_BRIGHTNESS_MIN                             78
#define RTK_DDCCI_BRIGHTNESS_CENTER                          (((RTK_DDCCI_BRIGHTNESS_MAX - RTK_DDCCI_BRIGHTNESS_MIN) / 2) + RTK_DDCCI_BRIGHTNESS_MIN)

//--------------------------------------------------
// Definitions of Color Temperature
//--------------------------------------------------
typedef enum
{
    RTK_DDCCI_CT_9300,
    RTK_DDCCI_CT_7500,
    RTK_DDCCI_CT_6500,
    RTK_DDCCI_CT_5800,
    RTK_DDCCI_CT_SRGB,
    RTK_DDCCI_CT_USER,
    RTK_DDCCI_CT_COLORTEMP_AMOUNT = RTK_DDCCI_CT_USER,
}DDCCI_COLOR_TEMP_DEF_T;

typedef enum
{
    RTK_DDCCI_ENGLISH = 0,
    RTK_DDCCI_CHINESE_T,
    RTK_DDCCI_FRENCH,
    RTK_DDCCI_GERMAN,
    RTK_DDCCI_SPANISH,
    RTK_DDCCI_CHINESE_S,
    RTK_DDCCI_JAPAN,
    RTK_DDCCI_LANGUAGE_AMOUNT
}DDCCI_OSD_LANG_DEF_T;

typedef enum
{
    RTK_DDCCI_OSD_EVENT_NONE_MSG = 0,
    RTK_DDCCI_OSD_EVENT_SHOW_NOSIGNAL_MSG,
    RTK_DDCCI_OSD_EVENT_SHOW_NOCABLE_MSG,
    RTK_DDCCI_OSD_EVENT_SHOW_NOSUPPORT_MSG,
    RTK_DDCCI_OSD_EVENT_SHOW_FAIL_SAFE_MODE_MSG,
    RTK_DDCCI_OSD_EVENT_SHOW_POWER_SAVING_EVENT_MSG,
    RTK_DDCCI_OSD_EVENT_GO_TO_POWER_SAVING_EVENT_MSG,
    RTK_DDCCI_OSD_EVENT_SHOW_CHANGE_SOURCE_MENU_MSG,
    RTK_DDCCI_OSD_EVENT_WAKE_UP_SWITCH_DDCCI_PORT_MSG,
    RTK_DDCCI_OSD_EVENT_DO_AUTO_CONFIG_MSG,
    RTK_DDCCI_OSD_EVENT_ACTIVE_DO_ONE_TIME_MSG,
    RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_SYSTEMDATA_MSG,
    RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_MODEUSERDATA_MSG,
    RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_OSDUSERDATA_MSG,
    RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_ADCDATA_MSG,
    RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_COLORPROC_MSG,
    RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_BRICON_MSG,
    RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_OSDSIXCOLORDATA_MSG,
    RTK_DDCCI_OSD_EVENT_DP_VERSIOM_MSG,
}DDCCI_OSD_EVENT_MSG_T;

typedef enum
{
    RTK_DDCCI_POWER_STATUS_AC_ON = 0x00,
    RTK_DDCCI_POWER_STATUS_OFF,
    RTK_DDCCI_POWER_STATUS_NORMAL,
    RTK_DDCCI_POWER_STATUS_SAVING,
    RTK_DDCCI_POWER_STATUS_NONE = 0x0F,
} DDCCI_POWER_STATUS_T;

typedef struct
{
    unsigned char osd_event_msg;
    unsigned char osd_key_message;
} DDCCI_OSD_INFO_T;

typedef struct
{
    unsigned int brightness;
    unsigned int contrast;
} DDCCI_BRI_CON_DATA_T;

// Struct for all Timing Info
typedef struct
{
    unsigned char h_polarity : 1;             // HSP
    unsigned char v_polarity : 1;             // VSP
    unsigned int h_freq;               // Horizontal Freq. (unit: 0.1kHz)
    unsigned int v_freq;               // Vertical Freq. (unit: 0.1Hz)
} DDCCI_TIMING_INFO_T;

typedef struct
{
    unsigned int backlight;
    unsigned char language : 4;
    unsigned char colortemp_type : 4;
    unsigned char sharpness : 3;
} DDCCI_OSD_USER_DATA_TYPE_T;

typedef struct
{
    unsigned int colortemp_r;
    unsigned int colortemp_g;
    unsigned int colortemp_b;
} DDCCI_COLOR_PROC_DATA_TYPE_T;

//--------------------------------------------------
// MACRO
//--------------------------------------------------
#define GET_OSD_EVENT_MESSAGE()                     (g_ddcci_osd_info.osd_event_msg)
#define SET_OSD_EVENT_MESSAGE(x)                    (g_ddcci_osd_info.osd_event_msg = (x))

#define GET_OSD_CONTRAST()                          (g_ddcci_bri_con_data.contrast)
#define SET_OSD_CONTRAST(x)                         (g_ddcci_bri_con_data.contrast = (x))
#define GET_OSD_BRIGHTNESS()                        (g_ddcci_bri_con_data.brightness)
#define SET_OSD_BRIGHTNESS(x)                       (g_ddcci_bri_con_data.brightness = (x))
#define GET_OSD_BACKLIGHT()                         (g_ddcci_user_data.backlight)
#define SET_OSD_BACKLIGHT(x)                        (g_ddcci_user_data.backlight = (x))
#define GET_OSD_SHARPNESS()                         (g_ddcci_user_data.sharpness)
#define SET_OSD_SHARPNESS(x)                        (g_ddcci_user_data.sharpness = (x))
#define GET_COLOR_TEMP_TYPE()                       (g_ddcci_user_data.colortemp_type)
#define SET_COLOR_TEMP_TYPE(x)                      (g_ddcci_user_data.colortemp_type = (x))
#define GET_COLOR_TEMP_TYPE_USER_R()                (g_ddcci_color_proc_data.colortemp_r >> 4)
#define SET_COLOR_TEMP_TYPE_USER_R(x)               (g_ddcci_color_proc_data.colortemp_r = ((x) << 4))
#define GET_COLOR_TEMP_TYPE_USER_G()                (g_ddcci_color_proc_data.colortemp_g >> 4)
#define SET_COLOR_TEMP_TYPE_USER_G(x)               (g_ddcci_color_proc_data.colortemp_g = ((x) << 4))
#define GET_COLOR_TEMP_TYPE_USER_B()                (g_ddcci_color_proc_data.colortemp_b >> 4)
#define SET_COLOR_TEMP_TYPE_USER_B(x)               (g_ddcci_color_proc_data.colortemp_b = ((x) << 4))

#define GET_FORCE_POW_SAV_STATUS()                  (g_ddcci_force_ps_status)
#define SET_FORCE_POW_SAV_STATUS(x)                 (g_ddcci_force_ps_status = (x))
#define GET_DIGITAL_H_POSITION()                       (g_ddcci_digital_h_position)
#define SET_DIGITAL_H_POSITION(x)                      (g_ddcci_digital_h_position = (x))
#define GET_DIGITAL_V_POSITION()                       (g_ddcci_digital_v_position)
#define SET_DIGITAL_V_POSITION(x)                      (g_ddcci_digital_v_position = (x))

#define GET_INPUT_TIMING_H_POLARITY()                (g_ddcci_input_timing_info.h_polarity)
#define GET_INPUT_TIMING_V_POLARITY()                (g_ddcci_input_timing_info.v_polarity)
#define GET_INPUT_TIMING_HFREQ()                     (g_ddcci_input_timing_info.h_freq)
#define GET_INPUT_TIMING_VFREQ()                     (g_ddcci_input_timing_info.v_freq)

#define GET_OSD_LANGUAGE()                          (g_ddcci_user_data.language)
#define SET_OSD_LANGUAGE(x)                         (g_ddcci_user_data.language = (x))

#define GET_RUN_DDCCI_COMMAND()                        (g_ddcci_run_command)
#define SET_RUN_DDCCI_COMMAND(x)                       (g_ddcci_run_command = (x))

//****************************************************************************
// FUNCTION DECLARATIONS
//****************************************************************************
void rtk_ddcci_handler(void);
void rtk_ddcci_get_vcp_feature(void);
bool rtk_ddcci_set_vcp_feature_check(void);
void rtk_ddcci_set_vcp_feature(void);
void rtk_ddcci_reset_all_settings(void);
void rtk_ddcci_color_reset(void);
void rtk_ddcci_send_data(unsigned char *write_array);

//****************************************************************************
// INTERFACE VARIABLE DECLARATIONS
//****************************************************************************
extern unsigned char g_ddcci_rx_buf[RTK_DDCCI_RXBUF_LENGTH];
extern unsigned char g_ddcci_tx_buf[RTK_DDCCI_TXBUF_LENGTH];
extern DDCCI_OSD_INFO_T g_ddcci_osd_info;
extern DDCCI_BRI_CON_DATA_T g_ddcci_bri_con_data;
extern bool g_ddcci_run_command;
extern DDCCI_TIMING_INFO_T g_ddcci_input_timing_info;
extern DDCCI_OSD_USER_DATA_TYPE_T g_ddcci_user_data;
extern DDCCI_COLOR_PROC_DATA_TYPE_T g_ddcci_color_proc_data;
extern unsigned char g_ddcci_digital_h_position;
extern unsigned char g_ddcci_digital_v_position;

#endif //__RTK_DDCCI_VERIFY_H__
