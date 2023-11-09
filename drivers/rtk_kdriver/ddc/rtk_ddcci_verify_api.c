#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <rtk_kdriver/io.h>
#include "rtk_ddcci_verify_api.h"
#include "rtk_ddcci_verify.h"

//****************************************************************************
// CODE TABLES
//****************************************************************************
unsigned char g_ddcci_cp_string_hdmi[] =
{
    RTK_DDCCI_CAPABILITIES_STRING_HDMI
};

unsigned char g_ddcci_cp_string_dp[] =
{
    RTK_DDCCI_CAPABILITIES_STRING_DP
};

//****************************************************************************
// VARIABLE DECLARATIONS
//****************************************************************************
unsigned char g_ddcci_rx_buf[RTK_DDCCI_RXBUF_LENGTH];
unsigned char g_ddcci_tx_buf[RTK_DDCCI_TXBUF_LENGTH];

//--------------------------------------------------
// Description  : Initial TX variables and Send Null-Message
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void rtk_ddcci_user_init_tx(void)
{
    // Send Null-Message
    g_ddcci_tx_buf[RTK_DDCCI_SOURCE] = RTK_DDCCI_DEST_ADDRESS;
    g_ddcci_tx_buf[RTK_DDCCI_LENGTH] = RTK_DDCCI_CONTROL_STATUS_FLAG;
    g_ddcci_tx_buf[RTK_DDCCI_COMMAND] = rtk_ddcci_user_cal_checksum();

    rtk_ddcci_send_data(g_ddcci_tx_buf);
}

//--------------------------------------------------
// Description  : Initial RX variables
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void rtk_ddcci_user_init_rx(void)
{
    // Clear buffer for new message
    memset(g_ddcci_rx_buf, 0, RTK_DDCCI_RXBUF_LENGTH);
}

//--------------------------------------------------
// Description  : Caculate checksum
// Input Value  : None
// Output Value : sum --> Checksum value
//--------------------------------------------------
unsigned char rtk_ddcci_user_cal_checksum(void)
{
    unsigned char count = 0;
    unsigned char check_len = 0;
    unsigned char sum = RTK_DDCCI_VIRTUAL_HOST_ADDRESS;

    check_len = (g_ddcci_tx_buf[RTK_DDCCI_LENGTH] & 0x7F) + 2;

    for(count = 0; count < check_len; count++)
    {
        sum = sum ^ g_ddcci_tx_buf[count];
    }

    return sum;
}

//--------------------------------------------------
// Description  : DDCCI VCP Feature Reply Max and Present Value
// Input Value  : ucOpcode --> OPCODE
//                  max --> Maximum Value
//                  present --> Present Value
// Output Value : None
//--------------------------------------------------
void rtk_ddcci_user_set_vcp_reply_value(unsigned char type_opcode, unsigned int max, unsigned int present)
{
    g_ddcci_tx_buf[RTK_DDCCI_TYPE_CODE] = type_opcode;
    g_ddcci_tx_buf[RTK_DDCCI_MAX_HIGH_BYTE] = HIBYTE(max);
    g_ddcci_tx_buf[RTK_DDCCI_MAX_LOW_BYTE] = LOBYTE(max);
    g_ddcci_tx_buf[RTK_DDCCI_PRESENT_HIGH_BYTE] = HIBYTE(present);
    g_ddcci_tx_buf[RTK_DDCCI_PRESENT_LOW_BYTE] = LOBYTE(present);
}

//--------------------------------------------------
// Description  : DDCCI Get and Reply Timing Report
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void rtk_ddcci_user_get_timing_report(void)
{
    unsigned char polarity = 0;

    polarity = (GET_INPUT_TIMING_H_POLARITY() | (GET_INPUT_TIMING_V_POLARITY() << 1));

    g_ddcci_tx_buf[RTK_DDCCI_SOURCE] = RTK_DDCCI_DEST_ADDRESS;
    g_ddcci_tx_buf[RTK_DDCCI_TIMING_MSG_COMMAND] = RTK_DDCCI_CMD_REPLY_TIMING_MESSAGE;
    g_ddcci_tx_buf[RTK_DDCCI_TIMING_MSG_OPCODE] = RTK_DDCCI_OPCODE_TIMING_MSG;
    g_ddcci_tx_buf[RTK_DDCCI_TIMING_STATUS] = 0x00;

    // H sync pol :Bit 1 = 1:positive H-sync polarity
    if((bool)(polarity & RTK_DDCCI_BIT0) == RTK_DDCCI_TRUE)
    {
        g_ddcci_tx_buf[RTK_DDCCI_TIMING_STATUS] |= RTK_DDCCI_BIT1;
    }

    // V sync pol :Bit 0 = 1:positive V-sync polarity
    if((bool)(polarity & RTK_DDCCI_BIT1) == RTK_DDCCI_TRUE)
    {
        g_ddcci_tx_buf[RTK_DDCCI_TIMING_STATUS] |= RTK_DDCCI_BIT0;
    }

    g_ddcci_tx_buf[RTK_DDCCI_TIMING_HFREQ_HBYTE] = HIBYTE(GET_INPUT_TIMING_HFREQ() * 10);
    g_ddcci_tx_buf[RTK_DDCCI_TIMING_HFREQ_LBYTE] = LOBYTE(GET_INPUT_TIMING_HFREQ() * 10);
    g_ddcci_tx_buf[RTK_DDCCI_TIMING_VFREQ_HBYTE] = HIBYTE(GET_INPUT_TIMING_VFREQ() * 10);
    g_ddcci_tx_buf[RTK_DDCCI_TIMING_VFREQ_LBYTE] = LOBYTE(GET_INPUT_TIMING_VFREQ() * 10);
    g_ddcci_tx_buf[RTK_DDCCI_TIMING_VFREQ_LBYTE + 1] = rtk_ddcci_user_cal_checksum();

    rtk_ddcci_send_data(g_ddcci_tx_buf);
}

//--------------------------------------------------
// Description  : DDCCI Capabilities Request & Reply
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void rtk_ddcci_user_get_capabilities_request(void)
{
    unsigned int offset = 0;
    unsigned char send_len = 0;
    DDCCI_INPUT_PORT_TYPE_T port_type = rtk_ddcci_user_get_input_port_type();

    unsigned int capibility_len = rtk_ddcci_user_get_cp_string_size(port_type);

    offset = ((((unsigned int)g_ddcci_rx_buf[RTK_DDCCI_OFS_HIGH_BYTE]) << 8) & 0xFF00) | (g_ddcci_rx_buf[RTK_DDCCI_OFS_LOW_BYTE]);

    if(offset >= capibility_len)
    {
        send_len = 0;
    }
    else if(capibility_len > (offset + RTK_DDCCI_TXBUF_LENGTH - 6))
    {
        send_len = RTK_DDCCI_TXBUF_LENGTH - 6;
    }
    else
    {
        send_len = capibility_len - offset;
    }

    g_ddcci_tx_buf[RTK_DDCCI_SOURCE] = RTK_DDCCI_DEST_ADDRESS;
    g_ddcci_tx_buf[RTK_DDCCI_LENGTH] = RTK_DDCCI_CONTROL_STATUS_FLAG | (send_len + 3);
    g_ddcci_tx_buf[RTK_DDCCI_COMMAND] = RTK_DDCCI_CMD_CAPABILITIES_REQUEST_REPLY;
    g_ddcci_tx_buf[RTK_DDCCI_OFS_HIGH_BYTE] = g_ddcci_rx_buf[RTK_DDCCI_OFS_HIGH_BYTE];
    g_ddcci_tx_buf[RTK_DDCCI_OFS_LOW_BYTE] = g_ddcci_rx_buf[RTK_DDCCI_OFS_LOW_BYTE];

    rtk_ddcci_user_get_cp_string_data(&g_ddcci_tx_buf[RTK_DDCCI_OFS_LOW_BYTE + 1], port_type, offset, send_len);

    g_ddcci_tx_buf[RTK_DDCCI_OFS_LOW_BYTE + 1 + send_len] = rtk_ddcci_user_cal_checksum();

    rtk_ddcci_send_data(g_ddcci_tx_buf);
}

//--------------------------------------------------
// Description  : Get capability string data
// Input Value  : dest -> destination
//                port_type -> port type
//                start_index -> data offset
//                length -> data length
// Output Value : None
//--------------------------------------------------
void rtk_ddcci_user_get_cp_string_data(unsigned char *dest, DDCCI_INPUT_PORT_TYPE_T port_type, unsigned int start_index, unsigned int length)
{
    switch(port_type)
    {
        case DDCCI_PORT_HDMI:
            memcpy(dest, &g_ddcci_cp_string_hdmi[start_index], length);
            break;

        case DDCCI_PORT_DP:
            memcpy(dest, &g_ddcci_cp_string_dp[start_index], length);
            break;

        default:
            break;
    }

}

//--------------------------------------------------
// Description  : Get capability string size
// Input Value  : port_type -> port type
// Output Value : size
//--------------------------------------------------
unsigned int rtk_ddcci_user_get_cp_string_size(DDCCI_INPUT_PORT_TYPE_T port_type)
{
    switch(port_type)
    {
        case DDCCI_PORT_HDMI:
            return (sizeof(g_ddcci_cp_string_hdmi) - 1);

        case DDCCI_PORT_DP:
            return (sizeof(g_ddcci_cp_string_dp) - 1);

        default:
            return 0;
    }

}

unsigned int rtk_ddcci_user_adjust_percent_to_real_value(unsigned char percent_value, unsigned int max, unsigned int min, unsigned int center)
{
    //Reply 0 for test verify
    return ((unsigned int)0);
}

unsigned char rtk_ddcci_user_adjust_real_value_to_percent(unsigned int real_value, unsigned int max, unsigned int min, unsigned int center)
{
    //Reply 0 for test verify
    return ((unsigned char)0);
}

DDCCI_INPUT_PORT_TYPE_T rtk_ddcci_user_get_input_port_type(void)
{
    //Reply DDCCI_PORT_HDMI for test verify
    return DDCCI_PORT_HDMI;
}

