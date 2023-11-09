#include <linux/kernel.h>
#include "rtk_cec_msg.h"
#include <uapi/linux/cec.h>

#define MAX_MESSAGE_LINE_SIZE 2048

#if 0
int check_cec_msg_valid(struct cec_msg *msg)
{
    if (!msg)
        return -1;

    if (!msg->len || (msg->len > 2 && msg->len != (cec_msg->param_cnt + 2)))
        return -1;

    if (cec_msg_initiator(msg) == cec_msg_destination(msg))
        return -1;

    return 0;
}
#endif

unsigned char *get_logic_addr_name_by_logic_addr(unsigned char logic_addr)
{
    switch (logic_addr) {
        case 0:
            return "TV";
        case 1:
            return "RECORD_1";
        case 2:
            return "RECORD_2";
        case 3:
            return "TUNER_1";
        case 4:
            return "PLAYBACK_1";
        case 5:
            return "AUDIOSYSTEM";
        case 6:
            return "TUNER_2";
        case 7:
            return "TUNER_3";
        case 8:
            return "PLAYBACK_2";
        case 9:
            return "RECORD_3";
        case 0xA:
            return "TUNER_4";
        case 0xB:
            return "PLAYBACK_3";
        case 0xC:
            return "BACKUP_1";
        case 0xD:
            return "BACKUP_2";
        case 0xE:
            return "SPECIFIC";
        case 0xF:
            return "BROADCAST";
        default:
            return "ERR_LOGIC_ADDR";
    }
}



unsigned char *get_opcode_name_by_opcode(unsigned char opcode)
{
    static char op_hex[3] = {0};
    switch (opcode) {
        case 0x82:
            return "CEC_ACTIVE_SOURCE";
        case 0x04:
            return "CEC_IMAGE_VIEW_ON";
        case 0x0d:
            return "CEC_TEXT_VIEW_ON";
        case 0x9d:
            return "CEC_INACTIVE_SOURCE";
        case 0x85:
            return "CEC_REQUEST_ACTIVE_SOURCE";
        case 0x80:
            return "CEC_ROUTING_CHANGE";
        case 0x81:
            return "CEC_ROUTING_INFORMATION";
        case 0x86:
            return "CEC_SET_STREAM_PATH";
        case 0x36:
            return "CEC_STANDBY";
        case 0x0f:
            return "CEC_RECORD_TV_SCREEN";
        case 0x9e:
            return "CEC_CEC_VERSION";
        case 0x9f:
            return "CEC_GET_CEC_VERSION";
        case 0x83:
            return "CEC_GIVE_PHYSICAL_ADDR";
        case 0x91:
            return "CEC_GET_MENU_LANGUAGE";
        case 0x84:
            return "CEC_REPORT_PHYSICAL_ADDR";
        case 0x32:
            return "CEC_SET_MENU_LANGUAGE";
        case 0xa6:
            return "CEC_REPORT_FEATURES";
        case 0xa5:
            return "CEC_GIVE_FEATURES";
        case 0x42:
            return "CEC_DECK_CONTROL";
        case 0x1b:
            return "CEC_DECK_STATUS";
        case 0x1a:
            return "CEC_GIVE_DECK_STATUS";
        case 0x41:
            return "CEC_PLAY";
        case 0x08:
            return "CEC_GIVE_TUNER_DEVICE_STATUS";
        case 0x92:
            return "CEC_SELECT_ANALOGUE_SERVICE";
        case 0x93:
            return "CEC_SELECT_DIGITAL_SERVICE";
        case 0x07:
            return "CEC_TUNER_DEVICE_STATUS";
        case 0x06:
            return "CEC_TUNER_STEP_DECREMENT";
        case 0x05:
            return "CEC_TUNER_STEP_INCREMENT";
        case 0x87:
            return "CEC_DEVICE_VENDOR_ID";
        case 0x8c:
            return "CEC_GIVE_DEVICE_VENDOR_ID";
        case 0x89:
            return "CEC_VENDOR_COMMAND";
        case 0xa0:
            return "CEC_VENDOR_COMMAND_WITH_ID";
        case 0x8a:
            return "CEC_VENDOR_REMOTE_BUTTON_DOWN";
        case 0x8b:
            return "CEC_VENDOR_REMOTE_BUTTON_UP";
        case 0x64:
            return "CEC_SET_OSD_STRING";
        case 0x46:
            return "CEC_GIVE_OSD_NAME";
        case 0x47:
            return "CEC_SET_OSD_NAME";
        case 0x8d:
            return "CEC_MENU_REQUEST";
        case 0x8e:
            return "CEC_MENU_STATUS";
        case 0x44:
            return "CEC_USER_CONTROL_PRESSED";
        case 0x45:
            return "CEC_USER_CONTROL_RELEASED";
        case 0x8f:
            return "CEC_GIVE_DEVICE_POWER_STATUS";
        case 0x90:
            return "CEC_REPORT_POWER_STATUS";
        case 0x00:
            return "CEC_FEATURE_ABORT";
        case 0xff:
            return "CEC_ABORT";
        case 0x71:
            return "CEC_GIVE_AUDIO_STATUS";
        case 0x7d:
            return "CEC_GIVE_SYSTEM_AUDIO_MODE_STATUS";
        case 0x7a:
            return "CEC_REPORT_AUDIO_STATUS";
        case 0xa3:
            return "CEC_REPORT_SHORT_AUDIO_DESCRIPTOR";
        case 0xa4:
            return "CEC_REQUEST_SHORT_AUDIO_DESCRIPTOR";
        case 0x72:
            return "CEC_SET_SYSTEM_AUDIO_MODE";
        case 0x70:
            return "CEC_SYSTEM_AUDIO_MODE_REQUEST";
        case 0x7e:
            return "CEC_SYSTEM_AUDIO_MODE_STATUS";
        case 0x9a:
            return "CEC_SET_AUDIO_RATE";
        case 0xc0:
            return "CEC_INITIATE_ARC";
        case 0xc1:
            return "CEC_REPORT_ARC_INITIATED";
        case 0xc2:
            return "CEC_REPORT_ARC_TERMINATED";
        case 0xc3:
            return "CEC_REQUEST_ARC_INITIATION";
        case 0xc4:
            return "CEC_REQUEST_ARC_TERMINATION";
        case 0xc5:
            return "CEC_TERMINATE_ARC";
        case 0xa7:
            return "CEC_REQUEST_CURRENT_LATENCY";
        case 0xa8:
            return "CEC_REPORT_CURRENT_LATENCY";
        default:
            snprintf(op_hex, 3, "%02x", (unsigned int)opcode);
            return op_hex;
    }
}

static inline int rtk_cec_msg_opcode(const cm_buff *msg)
{
    return msg->len > 1 ? msg->data[1] : -1;
}

static inline __u8 rtk_cec_msg_initiator(const cm_buff *msg)
{
    return msg->data[0] >> 4;
}

static inline __u8 rtk_cec_msg_destination(const cm_buff *msg)
{
    return msg->data[0] & 0xf;
}

#define OP_VALUE(value) (value), (#value)

static int op_value_get_name(int value, unsigned char **p_name, ...)
{
    int ret = -1;
    int opcode;
    va_list argv;
    va_start(argv, p_name);
    while (1) {
        opcode = va_arg(argv, int);
        if (opcode == -1) {
            break;
        }
        if (opcode == value) {
            *p_name = va_arg(argv, unsigned char *);
            /* match return 0 */
            ret = 0;
            break;
        }
        else {
            va_arg(argv, unsigned char *);
        }
    };
    va_end(argv);
    return ret;
}

int cec_msg_get_phy_addr(unsigned char *data, unsigned char *buf, unsigned int buf_len)
{
    return snprintf(buf, buf_len, "Physical Address: [%01x.%01x.%01x.%01x] ", data[0] >> 4, data[0] & 0xf, data[1] >> 4,
                    data[1] & 0xf);
}

void cec_msg_decode(cm_buff *msg,  unsigned char *mode, unsigned char *buf, unsigned int buf_len)
{
    unsigned char *src_name = get_logic_addr_name_by_logic_addr(rtk_cec_msg_initiator(msg));
    unsigned char *dst_name = get_logic_addr_name_by_logic_addr(rtk_cec_msg_destination(msg));
    int opcode = rtk_cec_msg_opcode(msg);
    unsigned char tmp_buff[128] = {0};
    unsigned char *paramters = "";
    int ret = 0;

    if (msg->len > 2 || msg->len == 1) {
        switch (opcode)
        {
            case CEC_MSG_CEC_VERSION:
                if (msg->len != 3)
                    goto DEFAULT;
                ret = op_value_get_name((int)msg->data[2], &paramters,
                                        OP_VALUE(CEC_OP_CEC_VERSION_1_3A),
                                        OP_VALUE(CEC_OP_CEC_VERSION_1_4),
                                        OP_VALUE(CEC_OP_CEC_VERSION_2_0), -1);
                if (ret)
                    goto DEFAULT;
                break;

            case CEC_MSG_REPORT_POWER_STATUS:
                if (msg->len != 3)
                    goto DEFAULT;
                ret = op_value_get_name((int)msg->data[2], &paramters,
                                        OP_VALUE(CEC_OP_POWER_STATUS_ON),
                                        OP_VALUE(CEC_OP_POWER_STATUS_STANDBY),
                                        OP_VALUE(CEC_OP_POWER_STATUS_TO_ON),
                                        OP_VALUE(CEC_OP_POWER_STATUS_TO_STANDBY), -1);
                if (ret)
                    goto DEFAULT;
                break;
            case CEC_MSG_SET_OSD_NAME:
                memset(tmp_buff, 0, 128);
                memcpy(tmp_buff, &msg->data[2], msg->len - 2 > 128 ? 128 : msg->len - 2);
                paramters = tmp_buff;
                break;
            case CEC_MSG_MENU_REQUEST:
                if (msg->len != 3)
                    goto DEFAULT;
                ret = op_value_get_name((int)msg->data[2], &paramters,
                                        OP_VALUE(CEC_OP_MENU_REQUEST_ACTIVATE),
                                        OP_VALUE(CEC_OP_MENU_REQUEST_DEACTIVATE),
                                        OP_VALUE(CEC_OP_MENU_REQUEST_QUERY), -1);
                if (ret)
                    goto DEFAULT;
                break;
            case CEC_MSG_MENU_STATUS:
                if (msg->len != 3)
                    goto DEFAULT;
                ret = op_value_get_name((int)msg->data[2], &paramters,
                                        OP_VALUE(CEC_OP_MENU_STATE_ACTIVATED),
                                        OP_VALUE(CEC_OP_MENU_STATE_DEACTIVATED), -1);
                if (ret)
                    goto DEFAULT;
                break;
            case CEC_MSG_SYSTEM_AUDIO_MODE_STATUS:
            case CEC_MSG_SET_SYSTEM_AUDIO_MODE:
                if (msg->len != 3)
                    goto DEFAULT;
                ret = op_value_get_name((int)msg->data[2], &paramters,
                                        OP_VALUE(CEC_OP_SYS_AUD_STATUS_OFF),
                                        OP_VALUE(CEC_OP_SYS_AUD_STATUS_ON), -1);
                if (ret)
                    goto DEFAULT;
                break;
            case CEC_MSG_REPORT_AUDIO_STATUS:
                if (msg->len != 3)
                    goto DEFAULT;
                ret = op_value_get_name((int)(msg->data[2] >> 7), &paramters,
                                        OP_VALUE(CEC_OP_AUD_MUTE_STATUS_OFF),
                                        OP_VALUE(CEC_OP_AUD_MUTE_STATUS_ON), -1);
                if (ret)
                    goto DEFAULT;
                snprintf(tmp_buff, 128, "%s Volume:%d", paramters, (int)(msg->data[2] & 0x7f));
                paramters = tmp_buff;
                break;
            case CEC_MSG_DECK_CONTROL:
                if (msg->len != 3)
                    goto DEFAULT;
                ret = op_value_get_name((int)msg->data[2], &paramters,
                                        OP_VALUE(CEC_OP_DECK_CTL_MODE_SKIP_FWD),
                                        OP_VALUE(CEC_OP_DECK_CTL_MODE_SKIP_REV),
                                        OP_VALUE(CEC_OP_DECK_CTL_MODE_STOP),
                                        OP_VALUE(CEC_OP_DECK_CTL_MODE_EJECT), -1);
                if (ret)
                    goto DEFAULT;
                break;
            case CEC_MSG_GIVE_DECK_STATUS:
            case CEC_MSG_GIVE_TUNER_DEVICE_STATUS:
                if (msg->len != 3)
                    goto DEFAULT;
                ret = op_value_get_name((int)msg->data[2], &paramters,
                                        OP_VALUE(CEC_OP_STATUS_REQ_ON),
                                        OP_VALUE(CEC_OP_STATUS_REQ_OFF),
                                        OP_VALUE(CEC_OP_STATUS_REQ_ONCE), -1);
                if (ret)
                    goto DEFAULT;
                break;
            case CEC_MSG_DECK_STATUS:
                if (msg->len != 3)
                    goto DEFAULT;
                ret = op_value_get_name((int)msg->data[2], &paramters,
                                        OP_VALUE(CEC_OP_DECK_INFO_PLAY),
                                        OP_VALUE(CEC_OP_DECK_INFO_RECORD),
                                        OP_VALUE(CEC_OP_DECK_INFO_PLAY_REV),
                                        OP_VALUE(CEC_OP_DECK_INFO_STILL),
                                        OP_VALUE(CEC_OP_DECK_INFO_SLOW),
                                        OP_VALUE(CEC_OP_DECK_INFO_SLOW_REV),
                                        OP_VALUE(CEC_OP_DECK_INFO_FAST_FWD),
                                        OP_VALUE(CEC_OP_DECK_INFO_FAST_REV),
                                        OP_VALUE(CEC_OP_DECK_INFO_NO_MEDIA),
                                        OP_VALUE(CEC_OP_DECK_INFO_STOP),
                                        OP_VALUE(CEC_OP_DECK_INFO_SKIP_FWD),
                                        OP_VALUE(CEC_OP_DECK_INFO_SKIP_REV),
                                        OP_VALUE(CEC_OP_DECK_INFO_INDEX_SEARCH_FWD),
                                        OP_VALUE(CEC_OP_DECK_INFO_INDEX_SEARCH_REV),
                                        OP_VALUE(CEC_OP_DECK_INFO_OTHER), -1);
                if (ret)
                    goto DEFAULT;
                break;
            case CEC_MSG_PLAY:
                if (msg->len != 3)
                    goto DEFAULT;
                ret = op_value_get_name((int)msg->data[2], &paramters,
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_FWD),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_REV),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_STILL),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_FAST_FWD_MIN),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_FAST_FWD_MED),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_FAST_FWD_MAX),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_FAST_REV_MIN),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_FAST_REV_MED),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_FAST_REV_MAX),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_SLOW_FWD_MIN),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_SLOW_FWD_MED),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_SLOW_FWD_MAX),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_SLOW_REV_MIN),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_SLOW_REV_MED),
                                        OP_VALUE(CEC_OP_PLAY_MODE_PLAY_SLOW_REV_MAX), -1);
                if (ret)
                    goto DEFAULT;
                break;
            case CEC_MSG_RECORD_STATUS:
                if (msg->len != 3)
                    goto DEFAULT;
                ret =  op_value_get_name((int)msg->data[2], &paramters,
                                         OP_VALUE(CEC_OP_RECORD_STATUS_CUR_SRC),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_DIG_SERVICE),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_ANA_SERVICE),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_EXT_INPUT),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_NO_DIG_SERVICE),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_NO_ANA_SERVICE),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_NO_SERVICE),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_INVALID_EXT_PLUG),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_INVALID_EXT_PHYS_ADDR),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_UNSUP_CA),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_NO_CA_ENTITLEMENTS),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_CANT_COPY_SRC),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_NO_MORE_COPIES),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_NO_MEDIA),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_PLAYING),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_ALREADY_RECORDING),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_MEDIA_PROT),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_NO_SIGNAL),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_MEDIA_PROBLEM),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_NO_SPACE),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_PARENTAL_LOCK),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_TERMINATED_OK),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_ALREADY_TERM),
                                         OP_VALUE(CEC_OP_RECORD_STATUS_OTHER), -1);
                if (ret)
                    goto DEFAULT;
                break;
            case CEC_MSG_ACTIVE_SOURCE:
            case CEC_MSG_INACTIVE_SOURCE:
            case CEC_MSG_SET_STREAM_PATH:
            case CEC_MSG_ROUTING_INFORMATION:
            case CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST:
                if (msg->len != 4)
                    goto DEFAULT;
                cec_msg_get_phy_addr(&msg->data[2], tmp_buff, 128);
                paramters = tmp_buff;
                break;
            case CEC_MSG_REPORT_PHYSICAL_ADDR:
                if (msg->len != 5)
                    goto DEFAULT;
                cec_msg_get_phy_addr(&msg->data[2], tmp_buff, 128);
                snprintf(tmp_buff + strlen(tmp_buff), 128 - strlen(tmp_buff), "Device Type:%s",
                         get_logic_addr_name_by_logic_addr(msg->data[4]));
                paramters = tmp_buff;
                break;
            case CEC_MSG_ROUTING_CHANGE:
                if (msg->len != 6)
                    goto DEFAULT;
                cec_msg_get_phy_addr(&msg->data[2], tmp_buff, 128);
                snprintf(tmp_buff + strlen(tmp_buff), 128 - strlen(tmp_buff), "-> ");
                cec_msg_get_phy_addr(&msg->data[4], tmp_buff + strlen(tmp_buff), 128 - strlen(tmp_buff));
                paramters = tmp_buff;
                break;
            case CEC_MSG_FEATURE_ABORT:
                if (msg->len != 4)
                    goto DEFAULT;
                ret = op_value_get_name((int)msg->data[3], &paramters,
                                        OP_VALUE(CEC_OP_ABORT_UNRECOGNIZED_OP),
                                        OP_VALUE(CEC_OP_ABORT_INCORRECT_MODE),
                                        OP_VALUE(CEC_OP_ABORT_NO_SOURCE),
                                        OP_VALUE(CEC_OP_ABORT_INVALID_OP),
                                        OP_VALUE(CEC_OP_ABORT_REFUSED),
                                        OP_VALUE(CEC_OP_ABORT_UNDETERMINED), -1);
                if (ret)
                    snprintf(tmp_buff, 128, "Opcode: %s Reason: %02x",
                         get_opcode_name_by_opcode(msg->data[2]), (unsigned int)msg->data[3]);
                else
                    snprintf(tmp_buff, 128, "Opcode: %s Reason: %s",
                         get_opcode_name_by_opcode(msg->data[2]), paramters);
                paramters = tmp_buff;
                break;
            case -1:
                /* polling msg not print here */
                return;
            default:
DEFAULT:
                hex_dump_to_buffer(&msg->data[2], msg->len - 2, 32, 1, tmp_buff, 48, 0);
                paramters = tmp_buff;
                break;
        }
    }

    snprintf(buf, buf_len, "%s %-11s-> %-11s : <%s> %s", mode, src_name, dst_name, get_opcode_name_by_opcode((char)opcode), paramters);
    if (strlen(buf))
        cec_info("%s\n", buf);
}

extern unsigned int g_cec_debug_enable;

void cec_msg_dump(cm_buff *msg, unsigned char *status)
{
    if (g_cec_debug_enable) {
        unsigned char str[256] = {0};
        cec_msg_decode(msg, status, str, 256);
    }
}
