#include "ir_input.h"

static struct venus_key rtk_mk5_tv_keys[] = {
    {0x12, KEY_POWER},
    {0x11, KEY_EJECTCD},    // CMD_EJECT
    {0x10, KEY_1},
    {0x5B, KEY_2},
    {0x59, KEY_3},
    {0x1B, KEY_TV}, // CMD_SOURCE
    {0x18, KEY_4},
    {0x5A, KEY_5},
    {0x58, KEY_6},
    {0x1A, KEY_EPG},
    {0x14, KEY_7},
    {0x56, KEY_8},
    {0x54, KEY_9},
    {0x19, KEY_RECORD}, // CMD_FORCE_RECORD
    {0x57, KEY_SEARCH}, //widget
    {0x55, KEY_0},
    {0x17, KEY_ZOOM},
    //{0x15, KEY_FORCE_TIMESHIFT},  //CMD_FORCE_TIMESHIFT
    {0x4F, KEY_MENU},   // CMD_GUIDE
    {0x4D, KEY_UP},
    {0x16, KEY_BACK},   // CMD_RETURN
    {0x0C, KEY_LEFT},
    {0x4C, KEY_INSERT},
    {0x0E, KEY_RIGHT},
    {0x08, KEY_MENU},
    {0x48, KEY_DOWN},
    //{0x09, KEY_DISPLAY},  // CMD_DISPLAY
    {0x4B, KEY_REWIND}, // CMD_FRWD
    {0x49, KEY_PLAY},
    {0x0B, KEY_STOP},
    {0x0A, KEY_FASTFORWARD},    // CMD_FFWD
    {0x47, KEY_PREVIOUS},   // CMD_PREV
    {0x45, KEY_VOLUMEDOWN},     // CMD_VOL_DOWN
    {0x07, KEY_VOLUMEUP},   // CMD_VOL_UP
    {0x06, KEY_NEXT},
    {0x04, KEY_MUTE},
    //{0x46, KEY_PSCAN},    // CMD_PSCAN
    {0x44, KEY_SETUP},
    {0x05, KEY_SEARCH},
    {0x00, KEY_RED},    // CMD_OPTION_RED
    {0x42, KEY_GREEN},  // CMD_OPTION_GREEN
    {0x40, KEY_YELLOW}, // CMD_OPTION_YELLOW
    {0x01, KEY_BLUE},   // CMD_OPTION_BLUE
    {0x43, KEY_AUDIO},
    {0x41, KEY_SUBTITLE},   // CMD_STITLE
    //{0x03, KEY_REPEAT},   // CMD_REPEAT
    {0x02, KEY_SLOW},   // CMD_SFWD
#ifdef CONFIG_RTK_KDRV_SUPPORT_ALC_COMMAND
    /*Only take the position*/
    {0x0, KEY_ALC_HELLO_MIDOLI},
    {0x0, KEY_ALC_HELLO_MENU},
    {0x0, KEY_ALC_HELLO_VOLUME_LOUDER},
    {0x0, KEY_ALC_HELLO_VOLUME_DOWN},
    {0x0, KEY_ALC_CONFIRM},
    {0x0, KEY_ALC_START_PLAY},
    {0x0, KEY_ALC_PAUSE},
    {0x0, KEY_ALC_STOP},
    {0x0, KEY_ALC_GO_PREVIOUS},
    {0x0, KEY_ALC_GO_NEXT},
    {0x0, KEY_ALC_GO_RIGHT},
    {0x0, KEY_ALC_GO_LEFT},
#endif
};

struct  venus_key_table rtk_mk5_tv_key_table = {
    .keys = rtk_mk5_tv_keys,
    .size = ARRAY_SIZE(rtk_mk5_tv_keys),
};


static struct venus_key rtk_mk5_2_tv_keys[] = {
    {0x5f, KEY_POWER},
    {0x5b, KEY_MUTE},
    {0x53, KEY_1},
    {0x50, KEY_2},
    {0x12, KEY_3},
    //{0x40, KEY_CHANNEL_INC},  // CMD_CHANNEL_INC
    {0x40, KEY_CHANNELUP},
    {0x4f, KEY_4},
    {0x4c, KEY_5},
    {0x0e, KEY_6},
    //{0x5d, KEY_CHANNEL_DEC},  // CMD_CHANNEL_DEC
    {0x5d, KEY_CHANNELDOWN},
    {0x4b, KEY_7},
    {0x48, KEY_8},
    {0x0a, KEY_9},
    {0x03, KEY_VOLUMEUP},   // CMD_VOL_UP
    {0x47, KEY_DELETE},
    {0x44, KEY_0},
    {0x06, KEY_ENTER},  // CMD_ENTER
    {0x1f, KEY_VOLUMEDOWN},     // CMD_VOL_DOWN
    {0x57, KEY_SOURCE}, //CMD_SOURCE
    //{0x08, KEY_TOOLS},    // CMD_TOOLS
    {0x59, KEY_UP},
    {0x51, KEY_DOWN},
    {0x56, KEY_LEFT},
    {0x14, KEY_RIGHT},
    {0x55, KEY_ENTER},
    //{0x46, KEY_PIP},  // CMD_PIP
    //{0x45, KEY_POP},  // CMD_POP
    {0x42, KEY_HOME},   // CMD_GUIDE
    //{0x41, KEY_MP},   // CMD_MP
    {0x52, KEY_JUMP},   // CMD_JUMP
    {0x10, KEY_BACK},   // CMD_RETURN
    {0x4e, KEY_RED},    // CMD_OPTION_RED
    {0x4d, KEY_GREEN},  // CMD_OPTION_GREEN
    {0x0c, KEY_YELLOW}, // CMD_OPTION_YELLOW
    {0x0d, KEY_BLUE},   // CMD_OPTION_BLUE
    {0x01, KEY_FAVCH},    // CMD_FAVCH
    {0x4a, KEY_EPG},    // CMD_EPG
    {0x49, KEY_INFO},
    //{0x49, KEY_PROGINFO}, // CMD_PROGINFO
    {0x09, KEY_MENU},   // CMD_MENU
    //{0x43, KEY_},     // CMD_NODEFINED
    {0x58, KEY_SLEEP},
    {0x1b, KEY_DISPLAY},    // CMD_DISPLAY
    {0x1a, KEY_DISPLAY_RATIO}, // CMD_WIDE
    {0x5e, KEY_AUDIOMODE},  // CMD_AUDIOMODE
    {0x54, KEY_CAPTION},    // CMD_CAPTION
    {0x17, KEY_VIDEO},
    {0x16, KEY_AUDIO},
    {0x5a, KEY_AUDIOMTS},     // CMD_AUDIOMTS
    //{0x05, KEY_ROI},  // CMD_ROI
    {0x04, KEY_FREEZE}, // CMD_FREEZE
    {0x00, KEY_CARD_READER},    // CMD_CARD_READER
    {0x43, KEY_TEXT},   // CMD_TELETEXT
    //{0x5e, KEY_SUBTITLE},   // CMD_SUBTITLE

    {0x46, KEY_PIP},  // CMD_PIP
    {0x45, KEY_POP},  // CMD_POP
    {0x41, KEY_MP},   // CMD_MP
    {0x08, KEY_TOOLS},    // CMD_TOOLS
};

struct  venus_key_table rtk_mk5_2_tv_key_table = {
    .keys = rtk_mk5_2_tv_keys,
    .size = ARRAY_SIZE(rtk_mk5_2_tv_keys),
};

static struct venus_key rtk_hisense_tv_keys[] = {
    {0x0d, KEY_POWER},
    {0x44, KEY_VOLUMEUP},
    {0x43, KEY_VOLUMEDOWN},
    {0x16, KEY_UP},
    {0x17, KEY_DOWN},
    {0x19, KEY_LEFT},
    {0x18, KEY_RIGHT},
    {0x15, KEY_ENTER},
    {0x48, KEY_BACK},
    {0x14, KEY_MENU},
};

struct  venus_key_table rtk_hisense_tv_key_table = {
    .keys = rtk_hisense_tv_keys,
    .size = ARRAY_SIZE(rtk_hisense_tv_keys),
};

