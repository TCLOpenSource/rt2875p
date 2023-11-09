#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <rtk_kdriver/io.h>
#include <linux/kthread.h>

#include "rtk_ddc_dbg.h"
#include "rtk_ddcci_priv.h"
#include "rtk_ddcci_verify_api.h"
#include "rtk_ddcci_verify.h"

//****************************************************************************
// DEFINITIONS / MACROS
//****************************************************************************

#define RTK_DDCCI_FIRMWARE_VERSION                     0x01

#define RTK_DDCCI_BLACKLEVEL_DEFAULT                   0x50

// RC
#define RTK_DDCCI_CMD_GETVCP_RC_NO_ERROR               0x00
#define RTK_DDCCI_CMD_GETVCP_RC_UNSUPPORTED            0x01

// TP
#define RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER          0x00
#define RTK_DDCCI_CMD_GETVCP_TP_MOMENTARY              0x01

//--------------------------------------------------
// Definitions of Contrast Range (Real Value)
//--------------------------------------------------
#define RTK_DDCCI_COLORTEMP_USER_MAX                         255
#define RTK_DDCCI_COLORTEMP_USER_MIN                         0
#define RTK_DDCCI_COLORTEMP_USER_CENTER                      (((RTK_DDCCI_COLORTEMP_USER_MAX - RTK_DDCCI_COLORTEMP_USER_MIN) / 2) + RTK_DDCCI_COLORTEMP_USER_MIN)

//****************************************************************************
// STRUCT / TYPE / ENUM DEFINITTIONS
//****************************************************************************
unsigned char g_ddcci_coloer_preset_val[] = {0x08, 0x06, 0x05, 0x04, 0x01, 0x0B, 0x02};

unsigned char g_ddcci_language_define[] = {0x02, 0x01, 0x03, 0x04, 0x0A, 0x0D, 0x06};

//****************************************************************************
// VARIABLE DECLARATIONS
//****************************************************************************
unsigned char g_ddcci_red_black_level   = RTK_DDCCI_BLACKLEVEL_DEFAULT;
unsigned char g_ddcci_green_black_level = RTK_DDCCI_BLACKLEVEL_DEFAULT;
unsigned char g_ddcci_blue_black_level  = RTK_DDCCI_BLACKLEVEL_DEFAULT;
unsigned char g_ddcci_new_control_val = 1;
unsigned char g_ddcci_active_val;
unsigned char g_ddcci_colortemp_unsupport;

bool g_ddcci_force_ps_status = RTK_DDCCI_FALSE;

DDCCI_OSD_INFO_T g_ddcci_osd_info;
DDCCI_BRI_CON_DATA_T g_ddcci_bri_con_data;
bool g_ddcci_run_command = RTK_DDCCI_FAIL;
DDCCI_TIMING_INFO_T g_ddcci_input_timing_info;
DDCCI_OSD_USER_DATA_TYPE_T g_ddcci_user_data;
DDCCI_COLOR_PROC_DATA_TYPE_T g_ddcci_color_proc_data;
unsigned char g_ddcci_digital_h_position;
unsigned char g_ddcci_digital_v_position;

//****************************************************************************
// FUNCTION DEFINITIONS
//****************************************************************************
//--------------------------------------------------
// Description  : DDC Handler Process
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void rtk_ddcci_handler(void)
{
    if(rtk_ddcci_get_osd_status() == RTK_DDCCI_ON)
    {
        // Check if Data is needed to be received
        // Reply data to source according to the request
        if(GET_RUN_DDCCI_COMMAND() == RTK_DDCCI_SUCCESS)
        {
            switch(g_ddcci_rx_buf[RTK_DDCCI_COMMAND])
            {
                case RTK_DDCCI_CMD_GET_VCP_FEATURE:
                    rtk_ddcci_get_vcp_feature();
                    break;

                case RTK_DDCCI_CMD_SET_VCP_FEATURE:
                    rtk_ddcci_set_vcp_feature();
                    break;

                case RTK_DDCCI_CMD_GET_TIMING_REPORT:
                    rtk_ddcci_user_get_timing_report();
                    break;

                case RTK_DDCCI_CMD_SAVE_CURRENT_SETTINGS:
                    break;

                case RTK_DDCCI_CMD_CAPABILITIES_REQUEST:
                    rtk_ddcci_user_get_capabilities_request();
                    break;

                default:
                    rtk_ddcci_user_init_tx();
                    break;
            }

            rtk_ddcci_user_init_rx();

            SET_RUN_DDCCI_COMMAND(RTK_DDCCI_FAIL);
        }
    }
}

//--------------------------------------------------
// Description  : DDCCI Get VCP Feature & VCP Feature Reply
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void rtk_ddcci_get_vcp_feature(void)
{
    unsigned char temp_type[2] = {0};

    g_ddcci_tx_buf[RTK_DDCCI_SOURCE] = RTK_DDCCI_DEST_ADDRESS;
    g_ddcci_tx_buf[RTK_DDCCI_LENGTH] = RTK_DDCCI_CONTROL_STATUS_FLAG | 0x08;
    g_ddcci_tx_buf[RTK_DDCCI_COMMAND] = RTK_DDCCI_CMD_GET_VCP_FEATURE_REPLY;
    g_ddcci_tx_buf[RTK_DDCCI_RESULT_CODE] = RTK_DDCCI_CMD_GETVCP_RC_NO_ERROR;
    g_ddcci_tx_buf[RTK_DDCCI_SINK_OPCODE] = g_ddcci_rx_buf[RTK_DDCCI_SOURCE_OPCODE];

    switch(g_ddcci_rx_buf[RTK_DDCCI_SOURCE_OPCODE])
    {
        case RTK_DDCCI_OPCODE_VCP_NEW_CONTROL_VALUE:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0x0002, g_ddcci_new_control_val);
            break;

        case RTK_DDCCI_OPCODE_VCP_FACTORY_RESET:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_MOMENTARY, 0x0001, 0x00);
            break;

        case RTK_DDCCI_OPCODE_VCP_RECAL_BRI_CON:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_MOMENTARY, 0x0001, 0x00);
            break;

        case RTK_DDCCI_OPCODE_VCP_GEOMETRY_RESET:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_MOMENTARY, 0x0001, 0x00);
            break;

        case RTK_DDCCI_OPCODE_VCP_COLOR_RESET:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_MOMENTARY, 0x0001, 0x00);
            break;

        case RTK_DDCCI_OPCODE_VCP_COLOR_TEMP_INCREMENT:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0x0000, 100);
            break;

        case RTK_DDCCI_OPCODE_VCP_COLOR_TEMP_REQUEST:
            switch(GET_COLOR_TEMP_TYPE())
            {
                case RTK_DDCCI_CT_SRGB:
                    rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, (93 - 30), (40 - 30));
                    break;

                case RTK_DDCCI_CT_9300:
                    rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, (93 - 30), (93 - 30));
                    break;

                case RTK_DDCCI_CT_7500:
                    rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, (93 - 30), (75 - 30));
                    break;

                case RTK_DDCCI_CT_6500:
                    rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, (93 - 30), (65 - 30));
                    break;

                case RTK_DDCCI_CT_5800:
                    rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, (93 - 30), (50 - 30));
                    break;

                default:
                    rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, (93 - 30), (65 - 30));
                    break;
            }
            break;

        case RTK_DDCCI_OPCODE_VCP_BACKLIGHT:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 100,
                                            (unsigned int)rtk_ddcci_user_adjust_real_value_to_percent(GET_OSD_BACKLIGHT(), RTK_DDCCI_BACKLIGHT_MAX, RTK_DDCCI_BACKLIGHT_MIN, RTK_DDCCI_BACKLIGHT_CENTER));
            break;

        case RTK_DDCCI_OPCODE_VCP_CONTRAST:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 100,
                                            (unsigned int)rtk_ddcci_user_adjust_real_value_to_percent(GET_OSD_CONTRAST(), RTK_DDCCI_CONTRAST_MAX, RTK_DDCCI_CONTRAST_MIN, RTK_DDCCI_CONTRAST_CENTER));
            break;

        case RTK_DDCCI_OPCODE_VCP_SELECT_COLOR_PRESET:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0x000B,
                                            g_ddcci_coloer_preset_val[GET_COLOR_TEMP_TYPE()]);
            break;

        case RTK_DDCCI_OPCODE_VCP_RED_GAIN:
            temp_type[0] = GET_COLOR_TEMP_TYPE();

            if(GET_COLOR_TEMP_TYPE() != RTK_DDCCI_CT_USER)
            {
                SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_USER);

                rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());
            }

            temp_type[1] = rtk_ddcci_user_adjust_real_value_to_percent(GET_COLOR_TEMP_TYPE_USER_R(),
                                                            RTK_DDCCI_COLORTEMP_USER_MAX,
                                                            RTK_DDCCI_COLORTEMP_USER_MIN,
                                                            RTK_DDCCI_COLORTEMP_USER_CENTER);

            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 100, temp_type[1]);

            SET_COLOR_TEMP_TYPE(temp_type[0]);

            rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());
            break;

        case RTK_DDCCI_OPCODE_VCP_GREEN_GAIN:
            temp_type[0] = GET_COLOR_TEMP_TYPE();

            if(GET_COLOR_TEMP_TYPE() != RTK_DDCCI_CT_USER)
            {
                SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_USER);

                rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());
            }

            temp_type[1] = rtk_ddcci_user_adjust_real_value_to_percent(GET_COLOR_TEMP_TYPE_USER_G(),
                                                            RTK_DDCCI_COLORTEMP_USER_MAX,
                                                            RTK_DDCCI_COLORTEMP_USER_MIN,
                                                            RTK_DDCCI_COLORTEMP_USER_CENTER);

            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 100, temp_type[1]);

            SET_COLOR_TEMP_TYPE(temp_type[0]);

            rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());
            break;

        case RTK_DDCCI_OPCODE_VCP_BLUE_GAIN:
            temp_type[0] = GET_COLOR_TEMP_TYPE();

            if(GET_COLOR_TEMP_TYPE() != RTK_DDCCI_CT_USER)
            {
                SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_USER);

                rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());
            }

            temp_type[1] = rtk_ddcci_user_adjust_real_value_to_percent(GET_COLOR_TEMP_TYPE_USER_B(),
                                                            RTK_DDCCI_COLORTEMP_USER_MAX,
                                                            RTK_DDCCI_COLORTEMP_USER_MIN,
                                                            RTK_DDCCI_COLORTEMP_USER_CENTER);

            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 100, temp_type[1]);

            SET_COLOR_TEMP_TYPE(temp_type[0]);

            rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());
            break;

        case RTK_DDCCI_OPCODE_VCP_AUTO_SET_UP:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_MOMENTARY, 0x0001, 0x00);
            break;

        case RTK_DDCCI_OPCODE_VCP_HPOSITION:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 100, GET_DIGITAL_H_POSITION());
            break;

        case RTK_DDCCI_OPCODE_VCP_VPOSITION:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 100, GET_DIGITAL_V_POSITION());
            break;

        case RTK_DDCCI_OPCODE_VCP_ACTIVE_CONTROL:
            break;

        case RTK_DDCCI_OPCODE_VCP_INPUT_SOURCE:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0x03, rtk_ddcci_osd_disp_judge_source_type());
            break;

#if(RTK_DDCCI_AUDIO_SUPPORT == RTK_DDCCI_ENABLE)
        case RTK_DDCCI_OPCODE_VCP_AUDIO_VOLUME:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 100, GET_OSD_VOLUME());
            break;
#endif

        case RTK_DDCCI_OPCODE_VCP_RED_BLACK_LEVEL:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 100, g_ddcci_red_black_level);
            break;

        case RTK_DDCCI_OPCODE_VCP_GREEN_BLACK_LEVEL:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 100, g_ddcci_green_black_level);
            break;

        case RTK_DDCCI_OPCODE_VCP_BLUE_BLACK_LEVEL:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 100, g_ddcci_blue_black_level);
            break;

        case RTK_DDCCI_OPCODE_VCP_PRODUCTKEY:
            g_ddcci_tx_buf[RTK_DDCCI_LENGTH] = RTK_DDCCI_CONTROL_STATUS_FLAG | 9;
            g_ddcci_tx_buf[RTK_DDCCI_COMMAND] = RTK_DDCCI_CMD_GET_VCP_FEATURE_REPLY;
            break;

        case RTK_DDCCI_OPCODE_VCP_HFREQ:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, LOWORD(((unsigned long)GET_INPUT_TIMING_HFREQ() * 100)), HIWORD(((unsigned long)GET_INPUT_TIMING_HFREQ() * 100)));
            break;

        case RTK_DDCCI_OPCODE_VCP_VFREQ:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0xFFFF, GET_INPUT_TIMING_VFREQ()*10);
            break;

        case RTK_DDCCI_OPCODE_VCP_PANEL_TYPE:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0x01, 1);
            break;

        case RTK_DDCCI_OPCODE_VCP_MONITOR_TYPE:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0x05, 3);
            break;

        case RTK_DDCCI_OPCODE_VCP_APPLICATION_ENABLE:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0xFF, 0x5a);
            break;

        case RTK_DDCCI_OPCODE_VCP_CONTROLLER_TYPE:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0x0000, 0x0009);
            break;

        case RTK_DDCCI_OPCODE_VCP_FW_LEVEL:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0xFFFF, RTK_DDCCI_FIRMWARE_VERSION);
            break;

        case RTK_DDCCI_OPCODE_VCP_OSD_ON_OFF:
            temp_type[0] = 1;
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0x02, temp_type[0]);
            break;

        case RTK_DDCCI_OPCODE_VCP_OSD_LANGUAGE:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0x0D, g_ddcci_language_define[GET_OSD_LANGUAGE()]);
            break;

        case RTK_DDCCI_OPCODE_VCP_POWER_MODE:
            if(rtk_ddcci_get_power_status() == RTK_DDCCI_POWER_STATUS_OFF)
            {
                temp_type[0] = 4;
            }
            else if(rtk_ddcci_get_power_status() == RTK_DDCCI_POWER_STATUS_SAVING)
            {
                temp_type[0] = 2;
            }
            else
            {
                temp_type[0] = 1;
            }
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0x0005, temp_type[0]);
            break;

        case RTK_DDCCI_OPCODE_VCP_VERSION:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0xFFFF, 0x0202);
            break;

        case RTK_DDCCI_OPCODE_VCP_SHARPNESS:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0x0004, GET_OSD_SHARPNESS());
            break;

        case RTK_DDCCI_OPCODE_VCP_DELL_DDM_SUPPORT:
            rtk_ddcci_user_set_vcp_reply_value(RTK_DDCCI_CMD_GETVCP_TP_SET_PARAMETER, 0xFFFF, 0x01);
            break;

        default:
            g_ddcci_tx_buf[RTK_DDCCI_RESULT_CODE] = RTK_DDCCI_CMD_GETVCP_RC_UNSUPPORTED;
            break;
    }
    g_ddcci_tx_buf[RTK_DDCCI_PRESENT_LOW_BYTE + 1] = rtk_ddcci_user_cal_checksum();

    rtk_ddcci_send_data(g_ddcci_tx_buf);
}

//--------------------------------------------------
// Description  : Check DDCCI Set VCP Feature
// Input Value  : None
// Output Value : None
//--------------------------------------------------
bool rtk_ddcci_set_vcp_feature_check(void)
{
    switch(g_ddcci_rx_buf[RTK_DDCCI_SOURCE_OPCODE])
    {
        case RTK_DDCCI_OPCODE_VCP_FACTORY_RESET:
        case RTK_DDCCI_OPCODE_VCP_RECAL_BRI_CON:
        case RTK_DDCCI_OPCODE_VCP_GEOMETRY_RESET:
        case RTK_DDCCI_OPCODE_VCP_COLOR_RESET:

            if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0)
            {
                return RTK_DDCCI_FALSE;
            }

            break;

        case RTK_DDCCI_OPCODE_VCP_NEW_CONTROL_VALUE:
        case RTK_DDCCI_OPCODE_VCP_AUTO_SET_UP:
        case RTK_DDCCI_OPCODE_VCP_AUTO_COLOR:

            if((g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0) || (g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] >= 3))
            {
                return RTK_DDCCI_FALSE;
            }

            break;

        case RTK_DDCCI_OPCODE_VCP_SELECT_COLOR_PRESET:

            if((g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0) || (g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] >= 0x0E))
            {
                return RTK_DDCCI_FALSE;
            }

            break;

        case RTK_DDCCI_OPCODE_VCP_OSD_LANGUAGE:

            if((g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0) || (g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] >= 0x26))
            {
                return RTK_DDCCI_FALSE;
            }

            break;

        case RTK_DDCCI_OPCODE_VCP_POWER_MODE:

            if((g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0) || (g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] >= 0x06))
            {
                return RTK_DDCCI_FALSE;
            }

            break;

        default:
            break;
    }

    return RTK_DDCCI_TRUE;
}

//--------------------------------------------------
// Description  : DDCCI Set VCP Feature
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void rtk_ddcci_set_vcp_feature(void)
{
    unsigned char temp_type = 0;

    if(rtk_ddcci_set_vcp_feature_check() == RTK_DDCCI_FAIL)
    {
        rtk_ddcci_user_init_tx();
    }
    else
    {
        switch(g_ddcci_rx_buf[RTK_DDCCI_SOURCE_OPCODE])
        {
            case RTK_DDCCI_OPCODE_VCP_NEW_CONTROL_VALUE:

                if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0x01)
                {
                    g_ddcci_new_control_val = 0x01;
                }

                break;

#if(RTK_DDCCI_SHARPNESS_SUPPORT == RTK_DDCCI_ON)
            case RTK_DDCCI_OPCODE_VCP_SHARPNESS:

                if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] > 4)
                {
                    g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] = 4;
                }
                SET_OSD_SHARPNESS(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE]);

                rtk_ddcci_adjust_sharpness(rtk_ddcci_user_get_input_port_type());
                SET_OSD_EVENT_MESSAGE(RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_OSDUSERDATA_MSG);

                break;
#endif

            case RTK_DDCCI_OPCODE_VCP_INPUT_SOURCE:

                if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0x01)
                {
                    SET_FORCE_POW_SAV_STATUS(RTK_DDCCI_FALSE);
                    rtk_ddcci_switch_input_port();
                }
                else if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0x03)
                {
                    SET_FORCE_POW_SAV_STATUS(RTK_DDCCI_FALSE);
                    rtk_ddcci_switch_input_port();
                }
                else if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0x04)
                {
                    SET_FORCE_POW_SAV_STATUS(RTK_DDCCI_FALSE);
                    rtk_ddcci_switch_input_port();
                }
                else if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0x0F)
                {
                    SET_FORCE_POW_SAV_STATUS(RTK_DDCCI_FALSE);
                    rtk_ddcci_switch_input_port();
                }
                else if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0x10)
                {
                    SET_FORCE_POW_SAV_STATUS(RTK_DDCCI_FALSE);
                    rtk_ddcci_switch_input_port();
                }
                else if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0x11)
                {
                    SET_FORCE_POW_SAV_STATUS(RTK_DDCCI_FALSE);
                    rtk_ddcci_switch_input_port();
                }
                else if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 0x12)
                {
                    SET_FORCE_POW_SAV_STATUS(RTK_DDCCI_FALSE);
                    rtk_ddcci_switch_input_port();
                }
                break;

            case RTK_DDCCI_OPCODE_VCP_FACTORY_RESET:

                rtk_ddcci_reset_all_settings();

                SET_DIGITAL_H_POSITION(50);
                SET_DIGITAL_V_POSITION(50);
                rtk_ddcci_adjust_h_position(GET_DIGITAL_H_POSITION());
                rtk_ddcci_adjust_v_position(GET_DIGITAL_V_POSITION());

                break;

            case RTK_DDCCI_OPCODE_VCP_RECAL_BRI_CON:

                rtk_ddcci_restore_bricon();
                rtk_ddcci_restore_backlight();

                rtk_ddcci_adjust_backlight(GET_OSD_BACKLIGHT());

#if(RTK_DDCCI_CONTRAST_SUPPORT == RTK_DDCCI_ON)
                rtk_ddcci_adjust_contrast(GET_OSD_CONTRAST());
#endif

#if(RTK_DDCCI_BRIGHTNESS_SUPPORT == RTK_DDCCI_ON)
                rtk_ddcci_adjust_brightness(GET_OSD_BRIGHTNESS());
#endif
                break;

            case RTK_DDCCI_OPCODE_VCP_GEOMETRY_RESET:

                SET_DIGITAL_H_POSITION(50);
                SET_DIGITAL_V_POSITION(50);
                rtk_ddcci_adjust_h_position(GET_DIGITAL_H_POSITION());
                rtk_ddcci_adjust_v_position(GET_DIGITAL_V_POSITION());

                break;

            case RTK_DDCCI_OPCODE_VCP_COLOR_RESET:

                rtk_ddcci_color_reset();

                break;

            case RTK_DDCCI_OPCODE_VCP_COLOR_TEMP_REQUEST:

                g_ddcci_colortemp_unsupport = (g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] + 30);
                if(g_ddcci_colortemp_unsupport >= 93)
                {
                    SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_9300);
                }
                else if((g_ddcci_colortemp_unsupport < 85) && (g_ddcci_colortemp_unsupport >= 75))
                {
                    SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_7500);
                }
                else if((g_ddcci_colortemp_unsupport < 75) && (g_ddcci_colortemp_unsupport >= 65))
                {
                    SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_6500);
                }
                else if((g_ddcci_colortemp_unsupport < 60) && (g_ddcci_colortemp_unsupport >= 50))
                {
                    SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_5800);
                }
                else
                {
                    SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_SRGB);
                }

                rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());

#if(RTK_DDCCI_CONTRAST_SUPPORT == RTK_DDCCI_ON)
                rtk_ddcci_adjust_contrast(GET_OSD_CONTRAST());
#endif
                rtk_ddcci_disable_osd();

                break;

            case RTK_DDCCI_OPCODE_VCP_BACKLIGHT:

                if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] > 100)
                {
                    g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] = 100;
                }

                SET_OSD_BACKLIGHT(rtk_ddcci_user_adjust_percent_to_real_value(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE], RTK_DDCCI_BACKLIGHT_MAX, RTK_DDCCI_BACKLIGHT_MIN, RTK_DDCCI_BACKLIGHT_CENTER));

                rtk_ddcci_adjust_backlight(GET_OSD_BACKLIGHT());
                SET_OSD_EVENT_MESSAGE(RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_OSDUSERDATA_MSG);

                break;

            case RTK_DDCCI_OPCODE_VCP_CONTRAST:

                if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] > 100)
                {
                    g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] = 100;
                }

                SET_OSD_CONTRAST(rtk_ddcci_user_adjust_percent_to_real_value(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE], RTK_DDCCI_CONTRAST_MAX, RTK_DDCCI_CONTRAST_MIN, RTK_DDCCI_CONTRAST_CENTER));

#if(RTK_DDCCI_CONTRAST_SUPPORT == RTK_DDCCI_ON)
                rtk_ddcci_adjust_contrast(GET_OSD_CONTRAST());
#endif
                SET_OSD_EVENT_MESSAGE(RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_BRICON_MSG);

                break;

            case RTK_DDCCI_OPCODE_VCP_SELECT_COLOR_PRESET:

                switch(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE])
                {
                    case 0x0B:
                        SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_USER);
                        break;

                    case 0x05:
                        SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_6500);
                        break;

                    case 0x08:
                        SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_9300);
                        break;

                    case 0x01:
                        SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_SRGB);
                        break;

                    case 0x04:
                        SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_5800);
                        break;

                    case 0x06:
                        SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_7500);
                        break;

                    default:
                        break;
                }

                rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());

#if(RTK_DDCCI_CONTRAST_SUPPORT == RTK_DDCCI_ON)
                rtk_ddcci_adjust_contrast(GET_OSD_CONTRAST());
#endif
                rtk_ddcci_disable_osd();

                break;

            case RTK_DDCCI_OPCODE_VCP_RED_GAIN:
                // Only write at customer color
                if(GET_COLOR_TEMP_TYPE() != RTK_DDCCI_CT_USER)
                {
                    temp_type = GET_COLOR_TEMP_TYPE();

                    SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_USER);

                    rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());

                    SET_COLOR_TEMP_TYPE_USER_R(rtk_ddcci_user_adjust_percent_to_real_value(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE],
                                                                                  RTK_DDCCI_COLORTEMP_USER_MAX,
                                                                                  RTK_DDCCI_COLORTEMP_USER_MIN,
                                                                                  RTK_DDCCI_COLORTEMP_USER_CENTER));

                    rtk_ddcci_save_color_setting(GET_COLOR_TEMP_TYPE());

                    SET_COLOR_TEMP_TYPE(temp_type);

                    rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());
                }
                else
                {
                    SET_COLOR_TEMP_TYPE_USER_R(rtk_ddcci_user_adjust_percent_to_real_value(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE],
                                                                                  RTK_DDCCI_COLORTEMP_USER_MAX,
                                                                                  RTK_DDCCI_COLORTEMP_USER_MIN,
                                                                                  RTK_DDCCI_COLORTEMP_USER_CENTER));
#if(RTK_DDCCI_CONTRAST_SUPPORT == RTK_DDCCI_ON)
                    rtk_ddcci_adjust_contrast(GET_OSD_CONTRAST());
#endif

                    SET_OSD_EVENT_MESSAGE(RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_COLORPROC_MSG);
                }

                break;

            case RTK_DDCCI_OPCODE_VCP_GREEN_GAIN:
                // Only write at customer color
                if(GET_COLOR_TEMP_TYPE() != RTK_DDCCI_CT_USER)
                {
                    temp_type = GET_COLOR_TEMP_TYPE();

                    SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_USER);

                    rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());

                    SET_COLOR_TEMP_TYPE_USER_G(rtk_ddcci_user_adjust_percent_to_real_value(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE],
                                                                                  RTK_DDCCI_COLORTEMP_USER_MAX,
                                                                                  RTK_DDCCI_COLORTEMP_USER_MIN,
                                                                                  RTK_DDCCI_COLORTEMP_USER_CENTER));

                    rtk_ddcci_save_color_setting(GET_COLOR_TEMP_TYPE());

                    SET_COLOR_TEMP_TYPE(temp_type);

                    rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());
                }
                else
                {
                    SET_COLOR_TEMP_TYPE_USER_G(rtk_ddcci_user_adjust_percent_to_real_value(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE],
                                                                                  RTK_DDCCI_COLORTEMP_USER_MAX,
                                                                                  RTK_DDCCI_COLORTEMP_USER_MIN,
                                                                                  RTK_DDCCI_COLORTEMP_USER_CENTER));
#if(RTK_DDCCI_CONTRAST_SUPPORT == RTK_DDCCI_ON)
                    rtk_ddcci_adjust_contrast(GET_OSD_CONTRAST());
#endif

                    SET_OSD_EVENT_MESSAGE(RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_COLORPROC_MSG);
                }

                break;

            case RTK_DDCCI_OPCODE_VCP_BLUE_GAIN:
                // Only write at customer color
                if(GET_COLOR_TEMP_TYPE() != RTK_DDCCI_CT_USER)
                {
                    temp_type = GET_COLOR_TEMP_TYPE();

                    SET_COLOR_TEMP_TYPE(RTK_DDCCI_CT_USER);

                    rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());

                    SET_COLOR_TEMP_TYPE_USER_B(rtk_ddcci_user_adjust_percent_to_real_value(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE],
                                                                                  RTK_DDCCI_COLORTEMP_USER_MAX,
                                                                                  RTK_DDCCI_COLORTEMP_USER_MIN,
                                                                                  RTK_DDCCI_COLORTEMP_USER_CENTER));

                    rtk_ddcci_save_color_setting(GET_COLOR_TEMP_TYPE());

                    SET_COLOR_TEMP_TYPE(temp_type);

                    rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());
                }
                else
                {
                    SET_COLOR_TEMP_TYPE_USER_B(rtk_ddcci_user_adjust_percent_to_real_value(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE],
                                                                                  RTK_DDCCI_COLORTEMP_USER_MAX,
                                                                                  RTK_DDCCI_COLORTEMP_USER_MIN,
                                                                                  RTK_DDCCI_COLORTEMP_USER_CENTER));
#if(RTK_DDCCI_CONTRAST_SUPPORT == RTK_DDCCI_ON)
                    rtk_ddcci_adjust_contrast(GET_OSD_CONTRAST());
#endif

                    SET_OSD_EVENT_MESSAGE(RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_COLORPROC_MSG);
                }

                break;

            case RTK_DDCCI_OPCODE_VCP_HPOSITION:
                SET_DIGITAL_H_POSITION(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE]);
                rtk_ddcci_adjust_h_position(GET_DIGITAL_H_POSITION());
                break;

            case RTK_DDCCI_OPCODE_VCP_VPOSITION:
                SET_DIGITAL_V_POSITION(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE]);
                rtk_ddcci_adjust_v_position(GET_DIGITAL_V_POSITION());

                break;

            case RTK_DDCCI_OPCODE_VCP_ACTIVE_CONTROL:

                g_ddcci_active_val = g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE];

                break;

#if(RTK_DDCCI_AUDIO_SUPPORT == RTK_DDCCI_ON)
            case RTK_DDCCI_OPCODE_VCP_AUDIO_VOLUME:

                if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] > 100)
                {
                    g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] = 100;
                }

                SET_OSD_VOLUME(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE]);

                rtk_ddcci_adjust_audio_volume(GET_OSD_VOLUME());
                SET_OSD_EVENT_MESSAGE(RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_OSDUSERDATA_MSG);

                break;
#endif // End of #if(RTK_DDCCI_AUDIO_SUPPORT == RTK_DDCCI_ON)

            case RTK_DDCCI_OPCODE_VCP_RED_BLACK_LEVEL:

                g_ddcci_red_black_level = g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE];
                // rtk_ddcci_adjust_brightness(g_ddcci_red_black_level);                            // Please don't delete

                break;

            case RTK_DDCCI_OPCODE_VCP_GREEN_BLACK_LEVEL:

                g_ddcci_green_black_level = g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE];
                // rtk_ddcci_adjust_brightness(g_ddcci_green_black_level);                             // Please don't delete

                break;

            case RTK_DDCCI_OPCODE_VCP_BLUE_BLACK_LEVEL:

                g_ddcci_blue_black_level = g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE];
                // rtk_ddcci_adjust_brightness(g_ddcci_green_black_level);                             // Please don't delete

                break;

            case RTK_DDCCI_OPCODE_VCP_POWER_MODE:

                if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 5)
                {
                    SET_FORCE_POW_SAV_STATUS(RTK_DDCCI_FALSE);
                    rtk_ddcci_set_target_power_status(RTK_DDCCI_POWER_STATUS_OFF);
                }
                else if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 4)
                {
                    SET_FORCE_POW_SAV_STATUS(RTK_DDCCI_TRUE);
                    rtk_ddcci_set_target_power_status(RTK_DDCCI_POWER_STATUS_SAVING);
                }
                else if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 1)
                {
                    SET_FORCE_POW_SAV_STATUS(RTK_DDCCI_FALSE);
                    rtk_ddcci_set_target_power_status(RTK_DDCCI_POWER_STATUS_NORMAL);
                }

                break;

            case RTK_DDCCI_OPCODE_VCP_OSD_LANGUAGE:

                switch(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE])
                {
                    case 0x01:
                        SET_OSD_LANGUAGE(RTK_DDCCI_CHINESE_T);
                        break;

                    case 0x02:
                        SET_OSD_LANGUAGE(RTK_DDCCI_ENGLISH);
                        break;

                    case 0x03:
                        SET_OSD_LANGUAGE(RTK_DDCCI_FRENCH);
                        break;

                    case 0x04:
                        SET_OSD_LANGUAGE(RTK_DDCCI_GERMAN);
                        break;

                    case 0x0A:
                        SET_OSD_LANGUAGE(RTK_DDCCI_SPANISH);
                        break;

                    case 0x06:
                        SET_OSD_LANGUAGE(RTK_DDCCI_JAPAN);
                        break;

                    case 0x0D:
                        SET_OSD_LANGUAGE(RTK_DDCCI_CHINESE_S);
                        break;

                    default:
                        break;
                }

                rtk_ddcci_disable_osd();
                SET_OSD_EVENT_MESSAGE(RTK_DDCCI_OSD_EVENT_SAVE_NVRAM_OSDUSERDATA_MSG);

                break;

            case RTK_DDCCI_OPCODE_VCP_OSD_ON_OFF:

                if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 1)
                {
                    rtk_ddcci_disable_osd();
                }
                else if(g_ddcci_rx_buf[RTK_DDCCI_SET_LOW_BYTE] == 2)
                {
                    rtk_ddcci_enable_osd();
                }

                break;

            case RTK_DDCCI_OPCODE_VCP_DISP_CALIBRATION:

                rtk_ddcci_disp_calib_process();

                break;

            default:

                rtk_ddcci_user_init_tx();

                break;
        }
    }
}

//--------------------------------------------------
// Description  : Reset monitor
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void rtk_ddcci_reset_all_settings(void)
{
    rtk_ddcci_color_reset();
}

//--------------------------------------------------
// Description  : Reset color to default
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void rtk_ddcci_color_reset(void)
{
    g_ddcci_red_black_level = RTK_DDCCI_BLACKLEVEL_DEFAULT;
    g_ddcci_green_black_level = RTK_DDCCI_BLACKLEVEL_DEFAULT;
    g_ddcci_blue_black_level = RTK_DDCCI_BLACKLEVEL_DEFAULT;

    rtk_ddcci_load_color_setting(GET_COLOR_TEMP_TYPE());

    rtk_ddcci_restore_bricon();
    rtk_ddcci_restore_backlight();

    rtk_ddcci_adjust_backlight(GET_OSD_BACKLIGHT());

#if(RTK_DDCCI_CONTRAST_SUPPORT == RTK_DDCCI_ON)
    rtk_ddcci_adjust_contrast(GET_OSD_CONTRAST());
#endif

#if(RTK_DDCCI_BRIGHTNESS_SUPPORT == RTK_DDCCI_ON)
    rtk_ddcci_adjust_brightness(GET_OSD_BRIGHTNESS());
#endif
}

void rtk_ddcci_send_data(unsigned char *write_array)
{
     if (rtk_ddcci_xmit_message(write_array,DDC_BUF_LENGTH) != 0)
    {
        rtk_ddcci_isr_tx_fifo_t();
        RTK_DDC_INFO("[%s][%d] Send success\n",__FUNCTION__,__LINE__);
        return;
    }
    else
    {
        RTK_DDC_INFO("[%s][%d] Send fail\n",__FUNCTION__,__LINE__);
        return;
    }
}

int rtk_ddcci_dbg_thread(void *arg)
{
    unsigned char rx_msg[DDC_BUF_LENGTH] = {0};
    unsigned char rx_msg_len = 0;

    while(!kthread_should_stop())
    {
    	if (rtk_ddcci_rcv_message(rx_msg, &rx_msg_len) != 0)
    	{
     	    if (rx_msg_len<=DDC_BUF_LENGTH)
     	    {
     	        memcpy(g_ddcci_rx_buf, rx_msg, rx_msg_len);
     	        SET_RUN_DDCCI_COMMAND(RTK_DDCCI_SUCCESS);
     	    }
        }
    	rtk_ddcci_handler();

    	msleep(25);
    }
    return 0;
}

