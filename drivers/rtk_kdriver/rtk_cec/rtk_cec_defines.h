
#ifndef _RTK_CEC_DEFINES_H_
#define _RTK_CEC_DEFINES_H_

#include <rtd_log/rtd_module_log.h>

#define _BIT(x)         (1UL << x)
#define TEST_BIT(x, i) (x & (1<<i))
#define STANBY_RESPONSE_GIVE_POWER_STATUS       _BIT(0)
#define STANBY_RESPONSE_POOL                    _BIT(1)
#define STANBY_RESPONSE_GIVE_PHYSICAL_ADDR      _BIT(2)
#define STANBY_RESPONSE_GET_CEC_VERISON         _BIT(3)
#define STANBY_RESPONSE_GIVE_DEVICE_VENDOR_ID   _BIT(4)
#define STANBY_WAKEUP_BY_ROUTING_INFO           _BIT(26)
#define STANBY_WAKEUP_BY_ROUTING_CHANGE         _BIT(27)
#define STANBY_WAKEUP_BY_REQUEST_AUDIO_SYSTEM   _BIT(28)
#define STANBY_WAKEUP_BY_USER_CONTROL           _BIT(29)
#define STANBY_WAKEUP_BY_IMAGE_VIEW_ON          _BIT(30)
#define STANBY_WAKEUP_BY_SET_STREAM_PATH        _BIT(31)

typedef enum {
    CEC_VERSION_1_1 = 0x00,
    CEC_VERSION_1_2 = 0x01,
    CEC_VERSION_1_2A = 0x02,
    CEC_VERSION_1_3 = 0x03,
    CEC_VERSION_1_3A = 0x04,
    CEC_VERSION_1_4 = 0x05,
    CEC_VERSION_UNKNOWN = 0xFF,
} CEC_VERSION;


#define CEC_WAKEUP_BY_SET_STREAM_PATH       0x01
#define CEC_WAKEUP_BY_PLAY_CMD              0x02
#define CEC_WAKEUP_BY_IMAGE_VIEW_ON         0x04
#define CEC_MSG_REPORT_POWER_STATUS         0x90
#define CEC_MSG_DEVICE_VENDOR_ID            0x87
#define CEC_MSG_SET_STREAM_PATH             0x86
#define CEC_MSG_ROUTING_INFO                0x81
#define CEC_MSG_ROUTING_CHANGE              0x80
#define CEC_MSG_REQUEST_SYSTEM_AUDIO        0x70
#define CEC_MSG_USER_CONTROL_PRESSED        0x44
#define CEC_MSG_PLAY                        0x41
#define CEC_MSG_STANDBY                     0x36
#define CEC_MSG_IMAGE_VIEW_ON               0x04
#define CEC_MSG_FEATURE_ABORT               0x00
#define CEC_MSG_REPORT_PHYSICAL_ADDRESS     0x84
#define CEC_MSG_GIVE_PHYSICAL_ADDRESS       0x83

#define CEC_DEVICE_TV                       0x00
#define CEC_DEVICE_RECORDING_DEVICE         0x01
#define CEC_DEVICE_RESERVED                 0x02
#define CEC_DEVICE_TUNER                    0x03
#define CEC_DEVICE_PLAYBACK_DEVICE          0x04
#define CEC_DEVICE_AUDIO_SYSTEM             0x05
#define CEC_DEVICE_PURE_CEC_SWITCH          0x06
#define CEC_DEVICE_VIDEO_PROCESSOR          0x07

#define CEC_POWER_STATUS_ON                 0x00
#define CEC_POWER_STATUS_STANDBY            0x01
#define CEC_POWER_STATUS_STANDBY_TO_ON      0x02
#define CEC_POWER_STATUS_ON_TO_STANDBY      0x03

#define CEC_ABORT_UNKNOWN_OPCODE            0x00
#define CEC_ABORT_NOT_IN_CORECT_MODE        0x01
#define CEC_ABORT_CAN_NOT_PROVIDE_SOURCE    0x02
#define CEC_ABORT_INVALID_OPERAND           0x03
#define CEC_ABORT_REFUSED                   0x04

#define CEC_USER_CONTROL_POWER_TOGGLE       0x6B
#define CEC_USER_CONTROL_POWER_ON           0x6D
#define CEC_USER_CONTROL_POWER              0x40

#define CEC_VENDOR_ID     0x00e091

#define CEC_NAME    "rtk-cec"
#define CEC_TX_TIMEOUT      (HZ<< 1)

#define cec_dbg(fmt, args...)   rtd_pr_cec_debug("[DBG] " fmt, ## args)

#define cec_tx_dbg(fmt, args...)  rtd_pr_cec_debug("[TX DBG] " fmt, ## args)

#define cec_rx_dbg(fmt, args...)    rtd_pr_cec_debug("[RX DBG] " fmt, ## args)

#define cec_info(fmt, args...)      rtd_pr_cec_info("[Info] "  fmt, ## args)
#define cec_warn(fmt, args...)      rtd_pr_cec_warn("[Warn] "  fmt, ## args)
#define cec_error(fmt, args...)     rtd_pr_cec_err("[Error] " fmt, ## args)


extern unsigned int g_cec_debug_enable;  // 0: close all, 1: open all, 2: smart parsing
extern unsigned int g_cec_send_enable;  // 0: close, 1: open
#define CEC_MSG_NAME_MAX_LENGTH 40

typedef struct
{
    int opcode;
    unsigned char print_enable;
    unsigned int msg_count;
    const char msg_name[CEC_MSG_NAME_MAX_LENGTH];
}cec_msg_config_st;

#endif