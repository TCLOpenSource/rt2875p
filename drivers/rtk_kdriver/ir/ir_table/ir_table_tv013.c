#include "ir_input.h"
#if defined(CONFIG_RTK_KDRV_HAIER_IR)
static struct venus_key haier_tv_keys[] = {
    //{0x0df2, KEY_PICTURE_MODE},   // CMD_PICTURE_MODE

    //{0x0bf4, KEY_DELETE},//

    {0x0bf4, KEY_POWER},
    {0x15ea, KEY_SOURCE},   //CMD_SOURCE

    {0xdf2, KEY_VIDEO},
    {0x12ed, KEY_AUDIO},
    {0x13ec, KEY_ZOOM},
    //{0x1fe0, KEY_},//

    {0x5ba4, KEY_EPG}, //


    //{0x12ed, KEY_SOUND_MODE}, // CMD_SOUND_MODE
    //{0x13ec, KEY_DISPLAY_RATIO},  // CMD_DISPLAY_RATIO
    //{0x0bf4, KEY_INTRO},  // CMD_INTRO

    //{0x1ce3, KEY_VII},    // CMD_VII

    //{0x1ae5, KEY_FREEZE}, // CMD_FREEZE


    //{0x20df, KEY_DISPLAY},    // CMD_DISPLAY


    //{0x51ae, KEY_SHORTCUT},   //CMD_SHORTCUT

    //{0x4ab5, KEY_STOP},



    {0x00ff, KEY_1},
    {0x01fe, KEY_2},
    {0x02fd, KEY_3},
    {0x03fc, KEY_4},
    {0x04fb, KEY_5},
    {0x05fa, KEY_6},
    {0x06f9, KEY_7},
    {0x07f8, KEY_8},
    {0x08f7, KEY_9},
    {0x20df, KEY_DISPLAY},//
    {0x09f6, KEY_0},
    {0x0af5, KEY_LAST},//INTRO


    {0x19e6, KEY_CHANNELUP},    // CMD_CHANNEL_INC
    {0x18e7, KEY_CHANNELDOWN},  // CMD_CHANNEL_DEC
    {0x0cf3, KEY_MUTE},
    {0x1be4, KEY_VOLUMEUP},
    {0x1ae5, KEY_VOLUMEDOWN},
    {0x916e, KEY_VOLUMEUP},
    {0x9966, KEY_VOLUMEDOWN},
    {0x8a75, KEY_3D_MODE},//3D



    //{0x7c83, KEY_},//
    //{0x8c73, KEY_},//
    //{0x90},//
    {0x4db2, KEY_HOME}, //CMD_HOME
    {0x639c, KEY_HOME}, //CMD_HOME
    {0xab54, KEY_HOME},     //CMD_HOME
    {0x906f, KEY_RecentApplicationsDialog},
    {0x0ff0, KEY_SCREENSHOT},
    //{0x8b,  },//
    {0x1ce3, KEY_MENU}, // CMD_MENU


    //{0x708f, KEY_},  //
    //{0x748b, KEY_},//

    {0x649b, KEY_UP},
    {0x6699, KEY_DOWN},
    {0x659a, KEY_LEFT},
    {0x6798, KEY_RIGHT},
    {0x6897, KEY_ENTER},
    {0x708f, KEY_PAGEUP},
    {0x748b, KEY_PAGEDOWN},

    //{0x0ff0, KEY_FREEZE},//
    {0x6996, KEY_BACK},

    {0x5ea1, KEY_RED},
    {0x5fa0, KEY_GREEN},
    {0x619e, KEY_YELLOW},
    {0x629d, KEY_BLUE},

    {0x6c93, KEY_PLAY},
    {0x6a95, KEY_REWIND},   // CMD_FRWD
    {0x6b94, KEY_FASTFORWARD},  //CMD_FFWD
    //{0xa3 },//



    //{0x28d7, KEY_PREVIOUS},   // CMD_PREV
    //{0x29d6, KEY_NEXT},
    //{0x57a8, KEY_SKIP},   // CMD_CMSKIP
    //{0x59a6, KEY_INSTANT_REPLAY}, //CMD_INSTANT_REPLAY
    //{0x32cd, KEY_SLOW},   // CMD_SFWD

    //{0x32cd, KEY_ZOOM},

    //{0x7986, KEY_COFFEE}, // three display share key
    //{0x0bf4, KEY_CAPSLOCK}, // KEY Alternate

    {0x1fe0, KEY_HAIER_USB}, // CMD_GUIDE
};

struct  venus_key_table haier_key_tv_table = {
    .keys = haier_tv_keys,
    .size = ARRAY_SIZE(haier_tv_keys),
};
static struct venus_key haier_key_Fackeys[] = {
#if 1
    {0x3ec1, KEY_FACTORY_FACTORY_MODE}, // CMD_TPV
    {0x0ff0, KEY_1},
    {0x12ed, KEY_2},
    {0x4ab5, KEY_3},
    {0x30cf, KEY_4},
    {0x10ef, KEY_5},
    {0x11ee, KEY_6},
    {0x32cd, KEY_7},
    {0x35ca, KEY_8},
    {0x36c9, KEY_9},
    {0x33cc, KEY_0},

    {0x56a9, KEY_CHANNELUP},    // CMD_CHANNEL_INC
    {0x55aa, KEY_CHANNELDOWN},  // CMD_CHANNEL_DEC
    {0x37c8, KEY_MUTE},
    {0x5fa0, KEY_VOLUMEUP},
    {0x609f, KEY_VOLUMEDOWN},

    {0x26d9, KEY_UP},
    {0x27d8, KEY_DOWN},
    {0x5da2, KEY_LEFT},
    {0x5ea1, KEY_RIGHT},
    {0x0ef1, KEY_ENTER},
    {0x708f, KEY_HAIER_USB}, // CMD_GUIDE
    {0x22dd, KEY_3D_MODE},//3D
    {0x6d92, KEY_FACTORY_TEST},
    {0x7b84, KEY_FACTORY_VGA},
    {0x00ff, KEY_FACTORY_AV1},
    {0x01fe, KEY_FACTORY_YUV1},
    {0x02fd, KEY_FACTORY_HDMI1},
    {0x758a, KEY_FACTORY_LOGOLED},
    {0x3fc0, KEY_TV},
    {0x46b9, KEY_PICTURE_MODE},
    {0x5aa5, KEY_FACTORY_RESET},
    {0x31ce, KEY_FACTORY_AUTO_ADC},
    {0x54ab, KEY_FACTORY_AGING_MODE},
    {0x57a8, KEY_FACTORY_AUTOSCANCH},
    {0x708f, KEY_FACTORY_UPLAYER},
    {0x6b94, KEY_FACTORY_PRESETCH},
#endif

};

struct  venus_key_table haier_key_fac_table = {
    .keys = haier_key_Fackeys,
    .size = ARRAY_SIZE(haier_key_Fackeys),
};

static struct venus_key haier_key_CVTkeys[] = { //lyg,20140325,for CVT remote
    {0x01fe, KEY_POWER},                //cvt power key
    {0x11ee, KEY_MUTE},                 //cvt mute key
    {0x05fa, KEY_DISPLAY},              //cvt display key
    {0x15ea, KEY_ZOOM},                 //cvt INOUT key(do as ZOOM)
    {0x44bb, KEY_VOLUMEUP},             //cvt V+ key
    {0x45ba, KEY_VOLUMEDOWN},           //cvt V- key
    {0x48b7, KEY_CHANNELUP},            // cvt CH+ key
    {0x49b6, KEY_CHANNELDOWN},          // cvt CH- key
    {0x02fd, KEY_1},                    // cvt 1 key
    {0x12ed, KEY_2},                    // cvt 2 key
    {0x06f9, KEY_3},                    // cvt 3 key
    {0x16e9, KEY_4},                    // cvt 4 key
    {0x03fc, KEY_5},                    // cvt 5 key
    {0x13ec, KEY_6},                    // cvt 6 key
    {0x07f8, KEY_7},                    // cvt 7 key
    {0x17e8, KEY_8},                    // cvt 8 key
    {0x00ff, KEY_9},                    // cvt 9 key
    {0x10ef, KEY_0},                    // cvt 0 key
    {0x04fb, KEY_CVTFAC_10_P},          // cvt 10 plus key  --CVT function
    {0x14eb, KEY_CVTFAC_DOT},           // cvt dot key  --CVT function
    {0x56a9, KEY_UP},                   // cvt up key
    {0x50af, KEY_DOWN},                 // cvt down key
    {0x47b8, KEY_LEFT},                 // cvt left key
    {0x4bb4, KEY_RIGHT},                // cvt right key
    {0x57a8, KEY_ENTER},                // cvt enter(OK) key
    {0x54ab, KEY_CVTAUTO},              // cvt auto key --CVT function
    {0x4ab5, KEY_BACK},                 // cvt DTV_MENU key
    {0x46b9, KEY_MENU},                 // cvt ATV_MENU key
    {0x4cb3, KEY_HOME},                 //cvt DVD_SETUP key,lyg,20140402,for enter luncher
    {0x40bf, KEY_SOURCE},               // cvt SOURCE key
    {0x51ae, KEY_VIDEO},                // cvt zoom(pic mode) key
    {0x4db2, KEY_AUDIO},                // cvt EPG(sound mode) key
    {0x59a6, KEY_CVTFAC_PVR},           // cvt PVR key  --CVT function
    {0x0af5, KEY_CVTFAC_TUNE},          // cvt TUNE key  --CVT function
    {0x09f6, KEY_HAIER_USB},            // cvt MEDIA key
    {0x19e6, KEY_CVTFAC_CALIBRATION1},  // cvt FAC_CALIBRATION1 key  --CVT function
    {0x0df2, KEY_CVTFAC_CALIBRATION2},  // cvt FAC_CALIBRATION2 key  --CVT function
    {0x0ef1, KEY_CVTFAC_VERSION},       // cvt VERSION key   --CVT function
    {0x0bf4, KEY_CVTFAC_AGING_MODE},    // cvt AGING_MODE key    --CVT function
    {0x1be4, KEY_CVTFAC_PANEL1},        // cvt PANEL1 key    --CVT function
    {0x0ff0, KEY_CVTFAC_PANEL2},        // cvt PANEL2 key    --CVT function
    {0x08f7, KEY_CVTFAC_F1},            // cvt F1 key    --CVT function
    {0x18e7, KEY_CVTFAC_F2},            // cvt F2 key    --CVT function
    {0x0cf3, KEY_SOURCE},           // cvt F3 key,lyg,20140402,for change source
    {0x1de2, KEY_CVTFAC_F4},            // cvt F4 key    --CVT function
    {0x1ee1, KEY_CVTFAC_F5},            // cvt F5 key    --CVT function
    {0x1fe0, KEY_CVTFAC_F6},            // cvt F6 key    --CVT function
    {0x1ce3, KEY_CVTFAC_F7},            // cvt F7 key    --CVT function

    {0x55aa, KEY_CVTFAC_NICAM},         // cvt NICAM key     --CVT function
    {0x41be, KEY_CVTFAC_TTXCC},         // cvt TEXT/CC key   --CVT function
    {0x1ae5, KEY_CVTFAC_RESET},         // cvt RESET key     --CVT function
};

struct  venus_key_table haier_key_cvt_table = {  //lyg,20140325,for CVT remote
    .keys = haier_key_CVTkeys,
    .size = ARRAY_SIZE(haier_key_CVTkeys),
};

static struct venus_key haier_key_MTCkeys[] = { //lyg,20140403,for MTC remote
    {0x8877, KEY_FACTORY_S1},//KEY_MTCFAC_SV1},
    {0x807f, KEY_SOURCE},//KEY_MTCFAC_SV2},
    {0x639c, KEY_MTCFAC_DTV1},              //mtc DTV1  --MTC function
    {0x629d, KEY_MTCFAC_DTV2},              //mtc DTV2  --MTC function
    {0x619e, KEY_MTCFAC_DTV3},              //mtc DTV3  --MTC function
    {0x609f, KEY_MTCFAC_REC},               //mtc REC  --MTC function
    {0x6b94, KEY_MTCFAC_ATV1},              //mtc ATV1  --MTC function
    {0x6a95, KEY_MTCFAC_ATV2},              //mtc ATV2  --MTC function
    {0x6996, KEY_MTCFAC_ATV3},              //mtc ATV3  --MTC function
    {0x6897, KEY_HAIER_USB},//KEY_MTCFAC_USB},
    {0x738c, KEY_FACTORY_AV1},//KEY_MTCFAC_AV1},
    {0x728d, KEY_FACTORY_AV1},//KEY_MTCFAC_AV2},
    {0x718e, KEY_FACTORY_VGA},//KEY_MTCFAC_PC},
    {0x708f, KEY_FACTORY_YUV1},//KEY_MTCFAC_YUV},
    {0x7b84, KEY_FACTORY_HDMI1},//KEY_MTCFAC_HDMI1},
    {0x7887, KEY_FACTORY_HDMI2},//KEY_MTCFAC_HDMI2},
    {0x7a85, KEY_UP},                   // KEY_MTCFAC_UP
    {0x8a75, KEY_DOWN},                 // KEY_MTCFAC_DOWN
    {0x837c, KEY_LEFT},                 // KEY_MTCFAC_LEFT
    {0x817e, KEY_RIGHT},                // KEY_MTCFAC_RIGHT
    {0x827d, KEY_ENTER},                // KEY_MTCFAC_ enter(OK) key
    {0x8b74, KEY_MENU},//an undefined key,redefined by lyg
    {0x8976, KEY_BACK},//an undefined key,redefined by lyg
    {0x936c, KEY_CVTFAC_VERSION},//KEY_MTCFAC_VERSION,COPY CVT FUCNTION
    {0x926b, KEY_MTCFAC_SCART1},                //mtc SCART1  --MTC function
    {0x916e, KEY_EXIT},
    {0x9768, KEY_MTCFAC_SCART2},                //mtc SCART2  --MTC function
    {0x9669, KEY_DISPLAY},//KEY_FACTORY_AV3},
    {0x956a, KEY_FACTORY_HDMI3},
    {0x8e71, KEY_MTCFAC_DVD},                   //mtc DVD  --MTC function
    {0x8d72, KEY_MTCFAC_DVB_S},                 //mtc DVB_S  --MTC function
    {0x8778, KEY_MTCFAC_DVD_USB},               //mtc DVD_USB  --MTC function
    {0x8679, KEY_MTCFAC_DVD_DISK},              //mtc DVD_DISK  --MTC function
    {0x857a, KEY_MTCFAC_DVB_S_RESET},           //mtc DVB_S_RESET  --MTC function
    {0x7f80, KEY_MTCFAC_PLAY},                  //mtc PLAY  --MTC function
    {0x7e81, KEY_MTCFAC_STOP},                  //mtc STOP  --MTC function
    {0x7d82, KEY_MTCFAC_RESET},                 //mtc RESET  --MTC function
    {0x7788, KEY_CVTAUTO},//KEY_MTCFAC_PC AUTO ADC},
    {0x7689, KEY_HOME},//an undefined key,redefined by lyg
    {0x758a, KEY_MENU},
    {0x748b, KEY_POWER},
    {0x748b, KEY_VOLUMEDOWN},//an undefined key,redefined by lyg
    {0x748b, KEY_VOLUMEUP},//an undefined key,redefined by lyg
    {0x748b, KEY_CHANNELDOWN},//an undefined key,redefined by lyg
    {0x748b, KEY_CHANNELUP},    //an undefined key,redefined by lyg
    {0x649b, KEY_HOME},//an undefined key,redefined by lyg
};

struct  venus_key_table haier_key_mtc_table = {  //lyg,20140403,for CVT remote
    .keys = haier_key_MTCkeys,
    .size = ARRAY_SIZE(haier_key_MTCkeys),
};

#endif