#include <linux/kernel.h>
#include <media/cec.h>

#include "rtk_cec_defines.h"
#include "rtk_cec_msg.h"
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

static cec_msg_config_st msg_cfg_table[] =
{
    { -1   , 0, 0, "CEC_POLLING"},
    { 0x82, 1, 0, "CEC_ACTIVE_SOURCE"},
    { 0x04, 1, 0, "CEC_IMAGE_VIEW_ON"},
    { 0x0d, 1, 0, "CEC_TEXT_VIEW_ON"},
    { 0x9d, 1, 0, "CEC_INACTIVE_SOURCE"},
    { 0x85, 1, 0, "CEC_REQUEST_ACTIVE_SOURCE"},
    { 0x80, 1, 0, "CEC_ROUTING_CHANGE"},
    { 0x81, 1, 0, "CEC_ROUTING_INFORMATION"},
    { 0x86, 1, 0, "CEC_SET_STREAM_PATH"},
    { 0x36, 1, 0, "CEC_STANDBY"},
    { 0x0b, 1, 0, "CEC_RECORD_OFF"},
    { 0x09, 1, 0, "CEC_RECORD_ON"},
    { 0x0a, 1, 0, "CEC_RECORD_STATUS"},
    { 0x0f, 1, 0, "CEC_RECORD_TV_SCREEN"},
    { 0x9e, 1, 0, "CEC_CEC_VERSION"},
    { 0x9f, 1, 0, "CEC_GET_CEC_VERSION"},
    { 0x83, 1, 0, "CEC_GIVE_PHYSICAL_ADDR"},
    { 0x91, 1, 0, "CEC_GET_MENU_LANGUAGE"},
    { 0x84, 1, 0, "CEC_REPORT_PHYSICAL_ADDR"},
    { 0x32, 1, 0, "CEC_SET_MENU_LANGUAGE"},
    { 0xa6, 1, 0, "CEC_REPORT_FEATURES"},
    { 0xa5, 1, 0, "CEC_GIVE_FEATURES"},
    { 0x42, 1, 0, "CEC_DECK_CONTROL"},
    { 0x1b, 1, 0, "CEC_DECK_STATUS"},
    { 0x1a, 1, 0, "CEC_GIVE_DECK_STATUS"},
    { 0x41, 1, 0, "CEC_PLAY"},
    { 0x08, 1, 0, "CEC_GIVE_TUNER_DEVICE_STATUS"},
    { 0x92, 1, 0, "CEC_SELECT_ANALOGUE_SERVICE"},
    { 0x93, 1, 0, "CEC_SELECT_DIGITAL_SERVICE"},
    { 0x07, 1, 0, "CEC_TUNER_DEVICE_STATUS"},
    { 0x06, 1, 0, "CEC_TUNER_STEP_DECREMENT"},
    { 0x05, 1, 0, "CEC_TUNER_STEP_INCREMENT"},
    { 0x87, 0, 0, "CEC_DEVICE_VENDOR_ID"},
    { 0x8c, 1, 0, "CEC_GIVE_DEVICE_VENDOR_ID"},
    { 0x89, 1, 0, "CEC_VENDOR_COMMAND"},
    { 0xa0, 1, 0, "CEC_VENDOR_COMMAND_WITH_ID"},
    { 0x8a, 1, 0, "CEC_VENDOR_REMOTE_BUTTON_DOWN"},
    { 0x8b, 1, 0, "CEC_VENDOR_REMOTE_BUTTON_UP"},
    { 0x64, 1, 0, "CEC_SET_OSD_STRING"},
    { 0x46, 1, 0, "CEC_GIVE_OSD_NAME"},
    { 0x47, 1, 0, "CEC_SET_OSD_NAME"},
    { 0x8d, 1, 0, "CEC_MENU_REQUEST"},
    { 0x8e, 1, 0, "CEC_MENU_STATUS"},
    { 0x44, 1, 0, "CEC_USER_CONTROL_PRESSED"},
    { 0x45, 1, 0, "CEC_USER_CONTROL_RELEASED"},
    { 0x8f, 1, 0, "CEC_GIVE_DEVICE_POWER_STATUS"},
    { 0x90, 1, 0, "CEC_REPORT_POWER_STATUS"},
    { 0x00, 1, 0, "CEC_FEATURE_ABORT"},
    { 0xff, 1, 0, "CEC_ABORT"},
    { 0x71, 1, 0, "CEC_GIVE_AUDIO_STATUS"},
    { 0x7d, 1, 0, "CEC_GIVE_SYSTEM_AUDIO_MODE_STATUS"},
    { 0x7a, 1, 0, "CEC_REPORT_AUDIO_STATUS"},
    { 0xa3, 1, 0, "CEC_REPORT_SHORT_AUDIO_DESCRIPTOR"},
    { 0xa4, 1, 0, "CEC_REQUEST_SHORT_AUDIO_DESCRIPTOR"},
    { 0x72, 1, 0, "CEC_SET_SYSTEM_AUDIO_MODE"},
    { 0x70, 1, 0, "CEC_SYSTEM_AUDIO_MODE_REQUEST"},
    { 0x7e, 1, 0, "CEC_SYSTEM_AUDIO_MODE_STATUS"},
    { 0x9a, 1, 0, "CEC_SET_AUDIO_RATE"},
    { 0xc0, 1, 0, "CEC_INITIATE_ARC"},
    { 0xc1, 1, 0, "CEC_REPORT_ARC_INITIATED"},
    { 0xc2, 1, 0, "CEC_REPORT_ARC_TERMINATED"},
    { 0xc3, 1, 0, "CEC_REQUEST_ARC_INITIATION"},
    { 0xc4, 1, 0, "CEC_REQUEST_ARC_TERMINATION"},
    { 0xc5, 1, 0, "CEC_TERMINATE_ARC"},
    { 0xa7, 1, 0, "CEC_REQUEST_CURRENT_LATENCY"},
    { 0xa8, 1, 0, "CEC_REPORT_CURRENT_LATENCY"},
};

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

static unsigned char m_msg_cfg_table_size =  sizeof(msg_cfg_table)/sizeof(cec_msg_config_st);

unsigned int cec_get_cfg_table_index(int opcode)
{
    cec_msg_config_st* p_table = msg_cfg_table;
    unsigned int table_index = 0;
    unsigned char i = 0;
	
	// rtd_pr_cec_info("[cec] m_msg_cfg_table_size = %d, opcode = %d", m_msg_cfg_table_size, opcode);

    for(i = 0; i<m_msg_cfg_table_size; i++ )
    {
        if(p_table[i].opcode == opcode)
        {
            table_index = i;
            break;
        }
    }

    return table_index;
}

void cec_msg_decode(struct cec_msg *msg,  unsigned char *mode, unsigned char *buf, unsigned int buf_len)
{
    unsigned char *src_name = get_logic_addr_name_by_logic_addr(cec_msg_initiator(msg));
    unsigned char *dst_name = get_logic_addr_name_by_logic_addr(cec_msg_destination(msg));
    int opcode = cec_msg_opcode(msg);

    unsigned int table_index = cec_get_cfg_table_index(opcode);
    char op_name[CEC_MSG_NAME_MAX_LENGTH];
    memset(op_name, '\0', sizeof(op_name)); 

    if(g_cec_debug_enable ==0)
        return;

    if(g_cec_debug_enable ==1)
    {
        strncpy(op_name, msg_cfg_table[table_index].msg_name, CEC_MSG_NAME_MAX_LENGTH);
        op_name[CEC_MSG_NAME_MAX_LENGTH-1] = '\0';
        if (opcode == -1) {
            if (0 == strcmp(mode, "TX DONE"))
                snprintf(buf, buf_len, "%s %-11s-> %-11s : <CEC_POLLING_MESSAGE>", mode, src_name, dst_name);
        } else {
            if (msg->len > 2) {
                unsigned char paramters[48] = {0};
                hex_dump_to_buffer(&msg->msg[2], msg->len - 2, 32, 1, paramters, 48, 0);
                snprintf(buf, buf_len, "%s %-11s-> %-11s : <%s> %s", mode, src_name, dst_name, op_name, paramters);
            }
            else
                snprintf(buf, buf_len, "%s %-11s-> %-11s : <%s> ", mode, src_name, dst_name, op_name);
        }
    }

    if(g_cec_debug_enable ==2)
    {
        unsigned char paramters[48] = {0};
        hex_dump_to_buffer(&msg->msg[2], msg->len - 2, 32, 1, paramters, 48, 0);

        msg_cfg_table[table_index].msg_count++;
        if(msg_cfg_table[table_index].print_enable)
        {
            if (table_index == 0)
            {
                snprintf(buf, buf_len, "%s %-11s-> %-11s : <CEC_POLLING_MESSAGE>", mode, src_name, dst_name);
            }
            else
            {
                strncpy(op_name, msg_cfg_table[table_index].msg_name, CEC_MSG_NAME_MAX_LENGTH);
                op_name[CEC_MSG_NAME_MAX_LENGTH-1] = '\0';
                snprintf(buf, buf_len, "%s %-11s-> %-11s : <%s> %s", mode, src_name, dst_name, op_name, paramters);
            }
        }
    }

    if (strlen(buf))
        cec_info("%s\n", buf);
}

void cec_msg_dump(struct cec_msg *msg, unsigned char *status)
{
    unsigned char str[256] = {0};
    cec_msg_decode(msg, status, str, 256);
}

void cec_msg_set_print_enable(int opcode, unsigned char enable)
{
    unsigned int table_index = cec_get_cfg_table_index(opcode);
    msg_cfg_table[table_index].print_enable = enable;
}

int cec_msg_get_print_enable(int opcode)
{
    unsigned int table_index = cec_get_cfg_table_index(opcode);
    return msg_cfg_table[table_index].print_enable;
}

void cec_msg_clear_msg_count(int opcode)
{
    unsigned int table_index = cec_get_cfg_table_index(opcode);
    msg_cfg_table[table_index].msg_count = 0;
}

int cec_msg_get_msg_count(int opcode)
{
    unsigned int table_index = cec_get_cfg_table_index(opcode);
    return msg_cfg_table[table_index].msg_count;
}