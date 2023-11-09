#include "ir_input.h"
#if defined(CONFIG_RTK_KDRV_TV035_IR)
static struct venus_key konka_tv_keys[] = {
    {0x0b, KEY_POWER},
    {0x1c, KEY_SOURCE},
    {0x01, KEY_1},
    {0x02, KEY_2},
    {0x03, KEY_3},
    {0x04, KEY_4},
    {0x05, KEY_5},
    {0x06, KEY_6},
    {0x07, KEY_7},
    {0x08, KEY_8},
    {0x09, KEY_9},
    {0x0a, KEY_INFO},   // CMD_INFO
    {0x0, KEY_0},
    {0x0E,  KEY_DELETE },  //KEY_KK_CHANNEL_RETURN
    //New Add one
    {0x2a, KEY_KK_PROGRAM_LIST},

    {0x15, KEY_MENU},   // CMD_MENU

    //New Add three
    {0xff, KEY_KK_PROGRAM_FAVORITE},
    {0x27, KEY_PAGEUP},
    {0x24, KEY_PAGEDOWN},

    {0x2b, KEY_UP},
    {0x2c, KEY_DOWN},
    {0x2d, KEY_LEFT},
    {0x2e, KEY_RIGHT},
    {0x2f, KEY_ENTER},
    {0x38, KEY_HOME},   // CMD_GUIDE
    {0x30, KEY_BACK},   // CMD_RETURN
    {0x1a, KEY_RED},    // CMD_OPTION_RED
    {0x1f, KEY_GREEN},  // CMD_OPTION_GREEN
    {0x0c, KEY_YELLOW}, // CMD_OPTION_YELLOW
    {0x16, KEY_BLUE},   // CMD_OPTION_BLUE

    {0x14, KEY_MUTE},

    {0x12, KEY_VOLUMEDOWN},
    {0x13, KEY_VOLUMEUP},
    {0x10, KEY_CHANNELDOWN},
    {0x11, KEY_CHANNELUP},

    //New Add TEN
    {0x22, KEY_KK_EPG}, // CMD_EPG
    {0x36, KEY_KK_3D},  // KEY_3D_MODE
    {0x1d, KEY_PICTURE_MODE}, //KEY_PICTURE_MODE
    {0x1e, KEY_DISPLAY_RATIO},  // KEY_DISPLAY_RATIO
    {0x0d, KEY_SOUND_MODE},
    {0x19, KEY_FREEZE},  // FREEZE //wangzhh add for customer 20130530
    {0x32, KEY_KK_TV},   //TV
    {0x34, KEY_YPBPR1},  // YPBPR
    {0x31, KEY_HDMI1},   // CMD_HDMI
    {0x33, KEY_AV},  // AV
    {0x25, KEY_SEARCH},
    {0x50, KEY_USB}, // CMD_USB
    //wangzhh add for factory remote control 20130604
    {0x82, KEY_VOLUME_DETECT},  // CMD_USB
    {0x88, KEY_FACTORY_MENU},   // KEY_FACTORY
    {0x85, KEY_BURN_TV}, //KEY_BURN_TV
    {0x80, KEY_KK_FAC_RESET},   // KEY_FACTORY
    {0x81, KEY_BALANCE}, //KEY_BURN_TV
    {0x83, KEY_BACKLIGHT_DETECT},   // KEY_VOICE_SEARCH
    {0x84, KEY_MIC_DETECT},
    {0x8b, KEY_KK_F1},
    {0x8c, KEY_KK_F2},   //KEY_24G_TO_INFRARED
    {0x8d, KEY_KK_F3},   // KEY_INFRARED_TO_24G
    {0x8e, KEY_F4},  // KEY_HEADPHONE_INSET
    {0x98, KEY_F5},  // KEY_HEADPHONE_PULLOUT

    {0x86, KEY_KK_LANGUAGE},
    {0x8A, KEY_KK_NET},
    {0x95, KEY_KK_VGA},

    {0x62, KEY_MORE},
    {0x64, KEY_MOUSE_CENTERED},
    {0x65, KEY_MOUSE_OK_DOWN},
    {0x66, KEY_MOUSE_OK_UP},

    {0xff, KEY_VOICE_SEARCH},    // KEY_VOICE_SEARCH
    {0xff, KEY_START_HANDWRITING},
    {0xff, KEY_FINISH_HANDWRITING},
    {0xff, KEY_24G_TO_INFRARED}, //KEY_24G_TO_INFRARED
    {0xff, KEY_INFRARED_TO_24G}, // KEY_INFRARED_TO_24G
    {0xff, KEY_HEADPHONE_INSET}, // KEY_HEADPHONE_INSET
    {0xff, KEY_HEADPHONE_PULLOUT},   // KEY_HEADPHONE_PULLOUT

};

struct  venus_key_table konka_tv_key_table = {
    .keys = konka_tv_keys,
    .size = ARRAY_SIZE(konka_tv_keys),
};

#endif