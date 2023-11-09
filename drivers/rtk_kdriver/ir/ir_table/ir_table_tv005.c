#include "ir_input.h"
#if defined(CONFIG_RTK_KDRV_TV005_IR)
static struct venus_key tv005_keys[] = {
    {0x14, KEY_SOURCE}, // VIDEO INPUT
    {0x12, KEY_POWER},  // POWER
    {0x44, KEY_TV}, // ???
    {0x59, KEY_PICTURE_MODE},   // PICTURE SIZE
    {0x0c, KEY_SUBTITLE},   // SUBTITLE
    {0x13, KEY_LANGUAGE},   // STEREO or DUAL
    {0x01, KEY_1},
    {0x02, KEY_2},
    {0x03, KEY_3},
    {0x04, KEY_4},
    {0x05, KEY_5},
    {0x06, KEY_6},
    {0x07, KEY_7},
    {0x08, KEY_8},
    {0x09, KEY_9},
    {0x47, KEY_LIST},   // CHANNEL LIST
    {0x00, KEY_0},
    {0x17, KEY_TEXT},   // TELETEXT
    {0x1a, KEY_VOLUMEUP},
    {0x1e, KEY_VOLUMEDOWN},
    {0x16, KEY_INFO},   // i+ or INDEX
    {0x10, KEY_MUTE},
    {0x45, KEY_PROGRAM},    // ???
    {0x1b, KEY_CHANNELUP},  // P ^
    {0x1f, KEY_CHANNELDOWN},    // P v
    {0x5b, KEY_MENU},
    {0x63, KEY_SHORTCUT},   // QUICK
    {0x19, KEY_UP},
    {0x1d, KEY_DOWN},
    {0x42, KEY_LEFT},
    {0x40, KEY_RIGHT},
    {0x21, KEY_ENTER},  // OK
    {0x64, KEY_BACK},   // BACK
    {0x43, KEY_EXIT},   // EXIT
    {0x48, KEY_RED},
    {0x49, KEY_GREEN},
    {0x4a, KEY_YELLOW},
    {0x4b, KEY_BLUE},
    {0x62, KEY_AUDIO},  // ???
    {0x68, KEY_STOP},   // STOP
    {0x67, KEY_PLAY},   // PLAY
    {0x22, KEY_PAUSE},  // PAUSE or STEP or STILL
    {0x54, KEY_PREVIOUS},   // SKIP - or REVEAL or |<<
    {0x53, KEY_HOLD},   // REW or HOLD
    {0x0d, KEY_ZOOM},   // FF or FULL-TOP-BOTTOM
    {0x15, KEY_NEXT},   // SKIP + or TIME or TXCL or >>|
    {0x34, KEY_HOME},
};

struct  venus_key_table tv005_key_table = {
    .keys = tv005_keys,
    .size = ARRAY_SIZE(tv005_keys),
};
#endif
