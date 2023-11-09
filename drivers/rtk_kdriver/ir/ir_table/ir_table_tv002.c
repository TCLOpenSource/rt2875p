#include "ir_input.h"


static struct sony_key tv002_keys[] = {
    {0x1, 0x15, KEY_POWER}, // POWER
    {0x1, 0x25, KEY_SOURCE},     // VIDEO INPUT
    {0x1, 0x33, KEY_RIGHT},
    {0x1, 0x34, KEY_LEFT},
    {0x1, 0x74, KEY_UP},
    {0x1, 0x75, KEY_DOWN},
    {0x1, 0x60, KEY_HOME},       // HOME MENU
    {0x1, 0x63, KEY_BACK},       // BACK
    {0x1, 0x65, KEY_ENTER},      // OK
    {0x1, 0x00, KEY_1},
    {0x1, 0x01, KEY_2},
    {0x1, 0x02, KEY_3},
    {0x1, 0x03, KEY_4},
    {0x1, 0x04, KEY_5},
    {0x1, 0x05, KEY_6},
    {0x1, 0x06, KEY_7},
    {0x1, 0x07, KEY_8},
    {0x1, 0x08, KEY_9},
    {0x1, 0x09, KEY_0},
    {0x1, 0x10, KEY_CHANNELUP},          // P ^
    {0x1, 0x11, KEY_CHANNELDOWN},        // P v
    {0x1, 0x12, KEY_VOLUMEUP},
    {0x1, 0x13, KEY_VOLUMEDOWN},
    {0x1, 0x14, KEY_MUTE},
    {0x1, 0x3A, KEY_DISPLAY},    // CMD_DISPLAY
    {0x1, 0x3B, KEY_RETUEN_CH},  // CMD_JUMP
    {0x1, 0x3E, KEY_PIC_OFF},    // PICTURE_OFF
    {0x1, 0x0E, KEY_HOME},       // CMD_GUIDE
    {0x1, 0x38, KEY_DISPLAY_RATIO},      // CMD_WIDE
    {0x1, 0x36, KEY_SLEEP},
    {0x1, 0x17, KEY_AUDIO},
    {0x1, 0x24, KEY_TV},
    {0x97, 0x18, KEY_STOP},       // CMD_STOP
    {0x97, 0x19, KEY_PAUSE},      // CMD_PAUSE
    {0x97, 0x1A, KEY_PLAY},
    {0x97, 0x1B, KEY_REWIND},     // CMD_FRWD
    {0x97, 0x1C, KEY_FASTFORWARD},// CMD_FFWD
    {0x97, 0x3C, KEY_PREVIOUS},   // CMD_PREV
    {0x97, 0x3D, KEY_NEXT},
    {0x97, 0x23, KEY_BACK},       // BACK
    {0x97, 0x5B, KEY_HOME},       // CMD_GUIDE
    {0x97, 0x36, KEY_MENU},       // OPTIONS key as DISCOVERY
    {0x97, 0x25, KEY_RED},        // CMD_OPTION_RED
    {0x97, 0x26, KEY_GREEN},      // CMD_OPTION_GREEN
    {0x97, 0x27, KEY_YELLOW},     // CMD_OPTION_YELLOW
    {0x97, 0x24, KEY_BLUE},       // CMD_OPTION_BLUE
    {0x97, 0x28, KEY_SUBTITLE},   // CMD_SUBTITLE
    {0x97, 0x1D, KEY_DOT},        // DOT
    {0x1A, 0x78, KEY_JUMP},       // SCENE key as ACTION MENU key
    {0x1A, 0x73, KEY_MENU},       // Discovery key as menu
    {0x1A, 0x58, KEY_SYNC_MENU},  // SYNC menu key
    {0x1A, 0x55, KEY_JUMP},       // photo frame key as ACTION_MENU key
    {0x1A, 0x7C, KEY_NETFLIX},    // NETFLIX key
    {0xA4, 0x3D, KEY_DISPLAY_RATIO},       // WIDE key
    {0xA4, 0x5B, KEY_EPG},        // Guilde key
    {0x17, 0x3D, KEY_NEXT},
    {0x17, 0x24, KEY_BLUE},
    {0x17, 0x27, KEY_YELLOW},
    {0xC4, 0x4B, KEY_JUMP},       // Action menu
    {0xC4, 0x46, KEY_GOOGLE_PLAY},    // Google TV key
};

struct  sony_key_table tv002_key_table = {
    .keys = tv002_keys,
    .size = ARRAY_SIZE(tv002_keys),
};