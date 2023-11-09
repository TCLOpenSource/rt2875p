#include "ir_input.h"
#if defined(CONFIG_RTK_KDRV_YK_54LU)
static struct venus_key yk_54lu_keys[] = {
    {0x17e8, KEY_IMAGE_MODE},   // CMD_PICTURE_MODE
    {0x0af5, KEY_INPUT_NUMBER},
    {0x0cf3, KEY_POWER},
    {0x18e7, KEY_SOUND_MODE_YK_54LU},   // CMD_SOUND_MODE
    {0x4fb0, KEY_DISPLAY_MODE}, // CMD_DISPLAY_RATIO
    //{0x0bf4, KEY_INTRO},  // CMD_INTRO
    {0x1ce3, KEY_VII},  // CMD_VII
    //{0x1ae5, KEY_FREEZE}, // CMD_FREEZE
    {0x16e9, KEY_SCREEN_DISPLAY},   // CMD_DISPLAY
    {0x11ee, KEY_MENU}, // CMD_MENU
    //{0x4db2, KEY_HOME},   //CMD_HOME
    {0x0ff0, KEY_TV_INPUT_YK}, //CMD_SOURCE
    {0x5ba4, KEY_BACK}, // CMD_RETURN
    {0x51ae, KEY_SHORTCUT}, //CMD_SHORTCUT
    {0x12ed, KEY_CHANNELUP},    // CMD_CHANNEL_INC
    {0x13ec, KEY_CHANNELDOWN},  // CMD_CHANNEL_DEC
    {0x0df2, KEY_MUTE},
    {0x14eb, KEY_VOLUMEUP},
    {0x15ea, KEY_VOLUMEDOWN},
    {0x4ab5, KEY_MEDIA_STOP},
    {0x43bc, KEY_DOWN},
    {0x44bb, KEY_LEFT},
    {0x45ba, KEY_RIGHT},
    {0x46b9, KEY_ENTER},//KEY_SELECT},
    {0x01fe, KEY_1},
    {0x02fd, KEY_2},
    {0x03fc, KEY_3},
    {0x04fb, KEY_4},
    {0x05fa, KEY_5},
    {0x06f9, KEY_6},
    {0x07f8, KEY_7},
    {0x08f7, KEY_8},
    {0x09f6, KEY_9},
    {0x00ff, KEY_0},
    {0x42bd, KEY_UP},
    {0x49b6, KEY_MEDIA_PLAY_PAUSE},
    {0x40bf, KEY_MEDIA_REWIND}, // CMD_FRWD
    {0x41be, KEY_MEDIA_FAST_FORWARD},   //CMD_FFWD
    {0x47b8, KEY_MEDIA_PREVIOUS},   // CMD_PREV
    {0x48b7, KEY_MEDIA_NEXT},
    //{0x57a8, KEY_SKIP},   // CMD_CMSKIP
    //{0x59a6, KEY_INSTANT_REPLAY}, //CMD_INSTANT_REPLAY
    //{0x58a7, KEY_MEDIA_FUNCTION}, // CMD_SFWD
    //{0x5aa5, KEY_ZOOM},
    {0x53ac, KEY_COOCAA},   // CMD_GUIDE
    {0x7986, KEY_SHARE},
    {0x0bf4, KEY_ALTERNATE},
    {0x609F, KEY_EPG}, // KEY_ENTER_EPG
    {0x629d, KEY_RED},
    {0x639c, KEY_GREEN},
    {0x649b, KEY_YELLOW},
    {0x659a, KEY_BLUE},
    {0x7887, KEY_HOME},
    {0x1de2, KEY_3D_DEL},
    {0x19e6, KEY_3D_MODE_YK},
    {0x6996, KEY_TIME_SPOT},
    {0x9669, KEY_USER_SWITCH},
    {0x56a9, KEY_MEDIA_SONG_SYSTEM},
    {0x5da2, KEY_MEDIA_RELATIONSHIP},
    {0x55aa, KEY_MEDIA_BOOKING},
    {0x5ca3, KEY_FAVORITES},
    {0x57a8, KEY_MEDIA_AUDIO_CONTROL},
    {0x58a7, KEY_MEDIA_FUNCTION},
    {0x54ab, KEY_MEDIA_ORIGINAL_SOUNDTRACK},
    {0x4bb4, KEY_MEDIA_RECORD_YK},
    {0x5aa5, KEY_MEDIA_SELECTED_SONGS},
    {0x5ea1, KEY_MEDIA_PRIORITY},
    {0x5fa0, KEY_MEDIA_DELETE},
    {0x59a6, KEY_MEDIA_SCORE_DISPLAY},
    {0x3fc0, KEY_FACTORY_FACTORY_MODE},
    {0x3ec1, KEY_FACTORY_RESET},
    {0x3dc2, KEY_FACTORY_SOURCE_ADD},
    {0x3cc3, KEY_FACTORY_SOURCE_REDUCE},
    {0x3ac5, KEY_FACTORY_BUS_OFF},
    {0x39c6, KEY_FACTORY_AGING_MODE},
    {0x38c7, KEY_FACTORY_AUTO_ADC},
    {0x36c9, KEY_FACTORY_RF_AGC},
    {0x22dd, KEY_FACTORY_BARCODE},
    {0x21de, KEY_FACTORY_SEARCH_UP},
    {0x20df, KEY_FACTORY_SEARCH_DOWN},
    {0x28d7, KEY_FACTORY_WHITE_BALANCE},
    {0x0001, KEY_NEW_SHUTTLE_LEFT},
    {0x0002, KEY_NEW_SHUTTLE_RIGHT},
    {0x4db2, KEY_LOCAL_MEDIA},
    {0x9c63, KEY_RESEARCH},
    {0x758a, KEY_SETTING},
    {0xc13e, KEY_SHUTTLE_LEFT_SPEED_1},
    {0xc23d, KEY_SHUTTLE_LEFT_SPEED_2},
    {0xc33c, KEY_SHUTTLE_LEFT_SPEED_3},
    {0xc43b, KEY_SHUTTLE_LEFT_SPEED_4},
    {0xc53a, KEY_SHUTTLE_LEFT_SPEED_5},
    {0xf10e, KEY_SHUTTLE_RIGHT_SPEED_1},
    {0xf20d, KEY_SHUTTLE_RIGHT_SPEED_2},
    {0xf30c, KEY_SHUTTLE_RIGHT_SPEED_3},
    {0xf40b, KEY_SHUTTLE_RIGHT_SPEED_4},
    {0xf50a, KEY_SHUTTLE_RIGHT_SPEED_5},
    {0x9a65, KEY_VOICE_END},
    {0x6699, KEY_POWER_LONG},
    {0x50af, KEY_HOME_LONG},
    {0x1be4, KEY_BACK_LONG},
    {0x1fe0, KEY_NOTIFICATION},
    {0x6a95, KEY_CENTER_LONG},
    {0x6b94, KEY_SEEK_SIGNAL},
    {0x7689, KEY_MYSTEP},
    {0x3bc4, KEY_FACTORY_OUTSET},
    {0x37c8, KEY_FACTORY_AV1},
    {0x35ca, KEY_FACTORY_AV2},
    {0x34cb, KEY_FACTORY_AV3},
    {0x33cc, KEY_FACTORY_S1},
    {0x32cd, KEY_FACTORY_YUV1},
    {0x31ce, KEY_FACTORY_YUV2},
    {0x30cf, KEY_FACTORY_VGA},
    {0x2fd0, KEY_FACTORY_HDMI1},
    {0x2ed1, KEY_FACTORY_HDMI2},
    {0x2dd2, KEY_FACTORY_HDMI3},
    {0x2cd3, KEY_FACTORY_KALA_OK},
    {0x2bd4, KEY_FACTORY_UPLAYER},
    {0x2ad5, KEY_FACTORY_LAN},
    {0x29d6, KEY_FACTORY_DREAM_PANEL},
    {0x28d7, KEY_FACTORY_WHITE_BALANCE},
    {0x24db, KEY_FACTORY_ALONE_LISTEN},
    {0x23dc, KEY_FACTORY_CA_CARD},
    {0x9b64, KEY_VOICE},
    {0x26d9, KEY_FACTORY_DTMB},
    {0x27d8, KEY_FACTORY_PLC},
    {0x25da, KEY_FACTORY_BLUETOOTH},
    {0x807f, KEY_USER_MOUSE},
    {0X817e, KEY_USER_VOICE},
    {0x7b84, KEY_COOL_LONG},
    {0x8b74, KEY_COOL},
    {0x8976, KEY_CHILD_COOL},
    {0x916e, KEY_CHILD_POWER},
    {0x936c, KEY_CHILD_HOME},
    {0x857a, KEY_MENU_LONG},
    {0x7c83, KEY_TV_INPUT_LONG},
    {0xa956, KEY_SYSTEM_INFO},
    {0xab54, KEY_NETWORK},
    {0xac53, KEY_MYAPP},
    {0x1ae5, KEY_RESET},
    {0x1ce3, KEY_CUSTOMER_KEY_CHANGE}
};

struct  venus_key_table yk_54lu_key_table = {
    .keys = yk_54lu_keys,
    .size = ARRAY_SIZE(yk_54lu_keys),
};

static struct venus_key yk_54lu_keys_ex[] = {
    {0x03fc, KEY_FACTORY_WLAN_SCAN_YK},
    {0x05fa, KEY_FACTORY_WHITE_DEBUG},
    {0x01fe, KEY_FACTORY_LOGOUT},
    {0x07f8, KEY_FACTORY_SPEAKER_SWITCH},
    {0x02fd, KEY_FACTORY_SERIAL232},
};

struct  venus_key_table yk_54lu_key_table_ex = {
       .keys = yk_54lu_keys_ex,
       .size = ARRAY_SIZE(yk_54lu_keys_ex),
};

#endif
#if defined(CONFIG_RTK_KDRV_YK_76J)
static struct venus_key yk_76j_keys[] = {
    {0x17e8, KEY_PICTURE_MODE}, // CMD_PICTURE_MODE
    {0x0af5, KEY_INPUT_NUMBER},
    {0x0cf3, KEY_POWER},
    {0x18e7, KEY_SOUND_MODE},   // CMD_SOUND_MODE
    {0x4fb0, KEY_DISPLAY_MODE}, // CMD_DISPLAY_RATIO
    {0x16e9, KEY_INFO}, // CMD_INTRO
    {0x1ce3, KEY_VII},  // CMD_VII
    //{0x1ae5, KEY_FREEZE}, // CMD_FREEZE
    //{0x16e9, KEY_SCREEN_DISPLAY}, // CMD_DISPLAY


    //{0xb748, KEY_MENU},   // CMD_QUICK

    {0x11ee, KEY_MENU}, // CMD_MENU
    {0x0ff0, KEY_SOURCE},
    //{0x4db2, KEY_HOME},   //CMD_HOME
    //{0x0ff0, KEY_TV_INPUT_YK},   //CMD_SOURCE
    {0x0bf4, KEY_BACK}, // CMD_RETURN
    {0x51ae, KEY_SHORTCUT}, //CMD_SHORTCUT
    {0x12ed, KEY_CHANNELUP},    // CMD_CHANNEL_INC
    {0x13ec, KEY_CHANNELDOWN},  // CMD_CHANNEL_DEC
    {0x0df2, KEY_MUTE},
    {0x14eb, KEY_VOLUMEUP},
    {0x15ea, KEY_VOLUMEDOWN},
    {0x4ab5, KEY_MEDIA_STOP},
    {0x43bc, KEY_DOWN},
    {0x44bb, KEY_LEFT},
    {0x45ba, KEY_RIGHT},
    {0x46b9, KEY_ENTER},//KEY_SELECT},
    {0x01fe, KEY_1},
    {0x02fd, KEY_2},
    {0x03fc, KEY_3},
    {0x04fb, KEY_4},
    {0x05fa, KEY_5},
    {0x06f9, KEY_6},
    {0x07f8, KEY_7},
    {0x08f7, KEY_8},
    {0x09f6, KEY_9},
    {0x00ff, KEY_0},
    {0x42bd, KEY_UP},
    {0xb54a, KEY_EXIT},
    {0xac53, KEY_AUDIO},
    {0x0ef1, KEY_SLEEP},
    {0xb24d, KEY_SUBTITLE},
    {0x49b6, KEY_MEDIA_PLAY_PAUSE},
    {0x40bf, KEY_MEDIA_REWIND}, // CMD_FRWD
    {0x41be, KEY_MEDIA_FAST_FORWARD},   //CMD_FFWD
    {0x47b8, KEY_MEDIA_PREVIOUS},   // CMD_PREV
    {0x48b7, KEY_MEDIA_NEXT},
    //{0x57a8, KEY_SKIP},   // CMD_CMSKIP
    //{0x59a6, KEY_INSTANT_REPLAY}, //CMD_INSTANT_REPLAY
    //{0x58a7, KEY_MEDIA_FUNCTION}, // CMD_SFWD
    //{0x5aa5, KEY_ZOOM},
    {0x53ac, KEY_COOCAA},   // CMD_GUIDE
    //{0x7986, KEY_SHARE},
    //{0x0bf4, KEY_ALTERNATE},
    {0xb44b, KEY_EPG}, // KEY_ENTER_EPG
    {0xad52, KEY_FAVORITES},
    {0xab54, KEY_CAPTION},
    {0xb14e, KEY_RECORD},
    {0xa857, KEY_FREEZE},

    {0xa05f, KEY_RED},
    {0xa15e, KEY_GREEN},
    {0xa25d, KEY_YELLOW},
    {0xa35c, KEY_BLUE},
    {0x7887, KEY_HOME},
    {0x1de2, KEY_3D_DEL},
    {0x19e6, KEY_3D_MODE_YK},
    {0x6996, KEY_TIME_SPOT},
    {0x9669, KEY_USER_SWITCH},
    {0x56a9, KEY_MEDIA_SONG_SYSTEM},
    {0x5da2, KEY_MEDIA_RELATIONSHIP},
    {0x55aa, KEY_MEDIA_BOOKING},
    {0x5ca3, KEY_FAVORITES},
    {0x57a8, KEY_MEDIA_AUDIO_CONTROL},
    {0x58a7, KEY_MEDIA_FUNCTION},
    {0x54ab, KEY_MEDIA_ORIGINAL_SOUNDTRACK},
    //{0x4bb4, KEY_MEDIA_RECORD_YK},
    {0x5aa5, KEY_MEDIA_SELECTED_SONGS},
    {0x5ea1, KEY_MEDIA_PRIORITY},
    {0x5fa0, KEY_MEDIA_DELETE},
    {0x59a6, KEY_MEDIA_SCORE_DISPLAY},
    {0x3fc0, KEY_FACTORY_FACTORY_MODE},
    {0x3ec1, KEY_FACTORY_RESET},
    {0x3dc2, KEY_FACTORY_SOURCE_ADD},
    {0x3cc3, KEY_FACTORY_SOURCE_REDUCE},
    {0x3ac5, KEY_FACTORY_BUS_OFF},
    {0x39c6, KEY_FACTORY_AGING_MODE},
    {0x38c7, KEY_FACTORY_AUTO_ADC},
    {0x36c9, KEY_FACTORY_RF_AGC},
    {0x22dd, KEY_FACTORY_BARCODE},
    {0x21de, KEY_FACTORY_SEARCH_UP},
    {0x20df, KEY_FACTORY_SEARCH_DOWN},
    {0x28d7, KEY_FACTORY_WHITE_BALANCE},
    {0x0001, KEY_NEW_SHUTTLE_LEFT},
    {0x0002, KEY_NEW_SHUTTLE_RIGHT},
    {0x4db2, KEY_LOCAL_MEDIA},
    {0x9c63, KEY_RESEARCH},
    {0x758a, KEY_SETTING},
    {0xc13e, KEY_SHUTTLE_LEFT_SPEED_1},
    {0xc23d, KEY_SHUTTLE_LEFT_SPEED_2},
    {0xc33c, KEY_SHUTTLE_LEFT_SPEED_3},
    {0xc43b, KEY_SHUTTLE_LEFT_SPEED_4},
    {0xc53a, KEY_SHUTTLE_LEFT_SPEED_5},
    {0xf10e, KEY_SHUTTLE_RIGHT_SPEED_1},
    {0xf20d, KEY_SHUTTLE_RIGHT_SPEED_2},
    {0xf30c, KEY_SHUTTLE_RIGHT_SPEED_3},
    {0xf40b, KEY_SHUTTLE_RIGHT_SPEED_4},
    {0xf50a, KEY_SHUTTLE_RIGHT_SPEED_5},
    {0x9a65, KEY_VOICE_END},
    {0x6699, KEY_POWER_LONG},
    {0x50af, KEY_HOME_LONG},
    {0x1be4, KEY_BACK_LONG},
    {0x1fe0, KEY_NOTIFICATION},
    {0x6a95, KEY_CENTER_LONG},
    {0x6b94, KEY_SEEK_SIGNAL},
    {0x7689, KEY_MYSTEP},
    {0x3bc4, KEY_FACTORY_OUTSET},
    {0x37c8, KEY_FACTORY_AV1},
    {0x35ca, KEY_FACTORY_AV2},
    {0x34cb, KEY_FACTORY_AV3},
    {0x33cc, KEY_FACTORY_S1},
    {0x32cd, KEY_FACTORY_YUV1},
    {0x31ce, KEY_FACTORY_YUV2},
    {0x30cf, KEY_FACTORY_VGA},
    {0x2fd0, KEY_FACTORY_HDMI1},
    {0x2ed1, KEY_FACTORY_HDMI2},
    {0x2dd2, KEY_FACTORY_HDMI3},
    {0x2cd3, KEY_FACTORY_KALA_OK},
    {0x2bd4, KEY_FACTORY_UPLAYER},
    {0x2ad5, KEY_FACTORY_LAN},
    {0x29d6, KEY_FACTORY_DREAM_PANEL},
    {0x28d7, KEY_FACTORY_WHITE_BALANCE},
    {0x24db, KEY_FACTORY_ALONE_LISTEN},
    {0x23dc, KEY_FACTORY_CA_CARD},
    {0x9b64, KEY_VOICE},
};

struct  venus_key_table yk_76j_key_table = {
    .keys = yk_76j_keys,
    .size = ARRAY_SIZE(yk_76j_keys),
};
#endif


