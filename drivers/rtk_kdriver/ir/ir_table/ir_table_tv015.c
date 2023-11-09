#include "ir_input.h"
#if defined(CONFIG_RTK_KDRV_TV015_IR)
static struct venus_key tv015_ir_aoc_tv_keys[] = {
    {0x01, KEY_POWER},

    {0x41, KEY_0},
    {0x15, KEY_1},
    {0x16, KEY_2},
    {0x17, KEY_3},
    {0x19, KEY_4},
    {0x1A, KEY_5},
    {0x1B, KEY_6},
    {0x1D, KEY_7},
    {0x1E, KEY_8},
    {0x1F, KEY_9},
    {0x4B, KEY_INPUT_NUMBER},// CMD_POINT ?
    {0x44, KEY_JUMP},

    {0x40, KEY_3D_MODE},    // CMD_3D_MODE_BANNER ?
    {0x03, KEY_VIDEO},
    {0x09, KEY_AUDIO},
    {0x14, KEY_ZOOM},       // CMD_WIDE ?

    {0x18, KEY_CHANNELUP},
    {0x1C, KEY_CHANNELDOWN},
    {0x0C, KEY_VOLUMEUP},   // CMD_VOL_UP
    {0x10, KEY_VOLUMEDOWN}, // CMD_VOL_DOWN
    {0x0a, KEY_MENU},       // CMD_MENU
    {0x04, KEY_MUTE},

    {0x08, KEY_SOURCE},     // CMD_SOURCE
    {0x51, KEY_HOME},       // CMD_GUIDE(ATV=>IPTV)
    {0x46, KEY_TV},         // CMD_SOURCE_ATV1 ?
    {0x06, KEY_AV},         // CMD_SOURCE_AV1 ?

    {0x0b, KEY_UP},
    {0x0f, KEY_DOWN},
    {0x49, KEY_LEFT},
    {0x4a, KEY_RIGHT},
    {0x0d, KEY_ENTER},      // CMD_ENTER
    {0x54, KEY_BACK},       // CMD_RETURN?
    {0x55, KEY_OPTION},     // CMD_OPTION_MENU ?

    {0x60, KEY_RED},        // CMD_OPTION_RED
    {0x61, KEY_GREEN},      // CMD_OPTION_GREEN
    {0x62, KEY_YELLOW},     // CMD_OPTION_YELLOW
    {0x63, KEY_BLUE},       // CMD_OPTION_BLUE

    {0x5c, KEY_PREVIOUS},   // CMD_PREV_FILE ?
    {0x58, KEY_REWIND},     // CMD_FRWD ?
    {0x57, KEY_FASTFORWARD},// CMD_FFWD ?
    {0x5f, KEY_NEXT},       // CMD_NEXT_FILE ?
    {0x5e, KEY_PAUSE},      // CMD_PAUSE ?
    {0x5d, KEY_STOP},       // CMD_STOP ?

    {0x56, KEY_DELETE},     // CMD_REPEAT ?
    {0x02, KEY_INFO},       // CMD_SUBTITLE ?
    {0x13, KEY_SLEEP},      // CMD_NICAM ?
    {0x53, KEY_CAPTION},    // CMD_DISPLAY
    {0x52, KEY_EPG},        // CMD_liyin
    {0x12, KEY_FREEZE},     // CMD_FREEZE


    {0x56, KEY_CARD_READER},// CMD_CARD_READER

};

struct  venus_key_table tv015_ir_aoc_tv_key_table = {
    .keys = tv015_ir_aoc_tv_keys,
    .size = ARRAY_SIZE(tv015_ir_aoc_tv_keys),
};

static struct venus_key tv015_ir_fac_tv_keys[] = {
//  {0x01, KEY_POWER},

    {0x33, KEY_0},
    {0x0F, KEY_1},
    {0x12, KEY_2},
    {0x4A, KEY_3},
    {0x30, KEY_4},
    {0x10, KEY_5},
    {0x11, KEY_6},
    {0x32, KEY_7},
    {0x35, KEY_8},
    {0x36, KEY_9},
    {0x22, KEY_3D_MODE},        //KEY_TPVFAC_2D3D
    {0x56, KEY_CHANNELUP},      //KEY_TPVFAC_CH_UP
    {0x55, KEY_CHANNELDOWN},    //KEY_TPVFAC_CH_DOWN
    {0x21, KEY_MENU},           //KEY_TPVFAC_MENU
    {0x37, KEY_MUTE},           //KEY_TPVFAC_MUTE
    {0x26, KEY_UP},             //KEY_TPVFAC_UP
    {0x27, KEY_DOWN},           //KEY_TPVFAC_DOWN
    {0x0E, KEY_ENTER},          //KEY_TPVFAC_OK

    //Add TPVFactoryIrDA Key already exist
    //{0x7B, KEY_FACTORY_VGA},  //
    //{0x02, KEY_FACTORY_HDMI1},

    //Add New TPVFactoryIrDA Key
    {0x6D, KEY_TPVFAC_TEST1},
    {0x92, KEY_TPVFAC_TEST2},
    {0x73, KEY_TPVFAC_CLK},
    {0x5F, KEY_TPVFAC_VOL_MAX},
    {0x60, KEY_TPVFAC_VOL_BUZZ},
    {0x5D, KEY_TPVFAC_VOL_L},
    {0x5E, KEY_TPVFAC_VOL_R},
    {0x4B, KEY_TPVFAC_CC_TT},
    {0x54, KEY_TPVFAC_B_I},
    {0x57, KEY_TPVFAC_CH_SCAN},
    {0x6B, KEY_TPVFAC_PRE_CH},
    {0x3F, KEY_TPVFAC_ADTV},
    {0x5A, KEY_TPVFAC_RST},
    {0x3E, KEY_TPVFAC_FAC},
    {0x50, KEY_TPVFAC_CSM},
    {0x62, KEY_TPVFAC_PATTERN},
    {0x03, KEY_TPVFAC_CTC},
    //{0x56, KEY_TPVFAC_CH_UP},
    //{0x55, KEY_TPVFAC_CH_DOWN},
    //{0x21, KEY_TPVFAC_MENU},
    //{0x37, KEY_TPVFAC_MUTE},
    //{0x26, KEY_TPVFAC_UP},
    //{0x27, KEY_TPVFAC_DOWN},
    //{0x0E, KEY_TPVFAC_OK},
    {0x46, KEY_TPVFAC_PIC},
    {0x53, KEY_TPVFAC_AUDIO},
    //{0x22, KEY_TPVFAC_2D3D},
    {0x23, KEY_TPVFAC_ARC},
    {0x24, KEY_TPVFAC_CI},
    {0x51, KEY_TPVFAC_VIRGIN},
    {0x00, KEY_TPVFAC_CVBS},
    {0x01, KEY_TPVFAC_YPBPR},
    {0x02, KEY_TPVFAC_HDMI},
    {0x7B, KEY_TPVFAC_VGA},
    {0x72, KEY_TPVFAC_CLONE},
    {0x34, KEY_TPVFAC_DCR},
    {0x31, KEY_TPVFAC_ADC},
    {0x0D, KEY_TPVFAC_BLK},
    {0x69, KEY_TPVFAC_WP},
    {0x25, KEY_TPVFAC_LS},
    {0x70, KEY_TPVFAC_USB},
    {0x71, KEY_TPVFAC_RJ45},
    {0x28, KEY_TPVFAC_RS232},
    //{0x33, KEY_TPVFAC_0},
    //{0x0F, KEY_TPVFAC_1},
    //{0x12, KEY_TPVFAC_2},
    //{0x4A, KEY_TPVFAC_3},
    //{0x30, KEY_TPVFAC_4},
    //{0x10, KEY_TPVFAC_5},
    //{0x11, KEY_TPVFAC_6},
    //{0x32, KEY_TPVFAC_7},
    //{0x35, KEY_TPVFAC_8},
    //{0x36, KEY_TPVFAC_9},
};

struct  venus_key_table tv015_ir_fac_tv_key_table = {
    .keys = tv015_ir_fac_tv_keys,
    .size = ARRAY_SIZE(tv015_ir_fac_tv_keys),
};

static struct venus_key tv015_ir_rc6_tv_keys[] = {
    {0x0C, KEY_POWER},

    {0x00, KEY_0},
    {0x01, KEY_1},
    {0x02, KEY_2},
    {0x03, KEY_3},
    {0x04, KEY_4},
    {0x05, KEY_5},
    {0x06, KEY_6},
    {0x07, KEY_7},
    {0x08, KEY_8},
    {0x09, KEY_9},

    {0x4C, KEY_CHANNELUP},
    {0x20, KEY_CHANNELUP},
    {0x4D, KEY_CHANNELDOWN},
    {0x21, KEY_CHANNELDOWN},
    {0x10, KEY_VOLUMEUP},   // CMD_VOL_UP
    {0x11, KEY_VOLUMEDOWN}, // CMD_VOL_DOWN
    {0x40, KEY_MENU},       // CMD_MENU
    {0x0D, KEY_MUTE},

    {0x38, KEY_SOURCE},     // CMD_SOURCE
    {0x54, KEY_HOME},       // CMD_GUIDE
    {0x58, KEY_UP},
    {0x59, KEY_DOWN},
    {0x5A, KEY_LEFT},
    {0x5B, KEY_RIGHT},
    {0x5C, KEY_ENTER},      // CMD_ENTER
    //{0x12, KEY_ENTER},    // ?
    //{0x47, KEY_DELETE},   // ?
    //{0x70, KEY_JUMP},     // CMD_JUMP
    {0x0A, KEY_BACK},       // CMD_RETURN
    {0x6D, KEY_RED},        // CMD_OPTION_RED
    {0x6E, KEY_GREEN},      // CMD_OPTION_GREEN ?
    {0x6F, KEY_YELLOW},     // CMD_OPTION_YELLOW ?
    {0x70, KEY_BLUE},       // CMD_OPTION_BLUE
    {0xCC, KEY_EPG},        // CMD_EPG

    //{0x13, KEY_SLEEP},
    {0xF5, KEY_DISPLAY},    // CMD_DISPLAY
    {0x0F, KEY_INFO},       // ?
    {0xD2, KEY_LIST},
    {0x2C, KEY_PLAY},
    {0x31, KEY_STOP},
    {0x30, KEY_PAUSE},
    {0x2B, KEY_REWIND},     // CMD_FRWD
    {0x28, KEY_FASTFORWARD},// CMD_FFWD
    {0xBF, KEY_SETUP},
    {0xB4, KEY_SEARCH},
    //{0x03, KEY_VIDEO},
    //{0x52, KEY_AUDIO},
    //{0x56, KEY_CARD_READER},  // CMD_CARD_READER
};

struct  venus_key_table tv015_ir_rc6_tv_key_table = {
    .keys = tv015_ir_rc6_tv_keys,
    .size = ARRAY_SIZE(tv015_ir_rc6_tv_keys),
};

#endif