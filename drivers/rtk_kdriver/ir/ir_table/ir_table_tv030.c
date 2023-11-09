#include "ir_input.h"

static struct venus_key tcl_jp_tv_keys[] = {

    // customize for jp
    {0x50, KEY_0},
    {0x51, KEY_11_AEHA},
    {0x52, KEY_12_AEHA},
    {0x53, KEY_3_LINE_INPUT_AEHA},
    {0x59, KEY_D_DATA_BML_AEHA},
    {0x71, KEY_MORE},
    {0x63, KEY_TERMINAL_AEHA},
    {0x65, KEY_SATELLITE_BS_AEHA},
    {0x5E, KEY_SATELLITE_CS_AEHA},
    {0x5D, KEY_RECORD_LIST_AEHA},
    {0x5A, KEY_TIME},
    {0x5B, KEY_T_SCHEDULE_AEHA},

	// for RC310 90 JP remote controllers, replace MTS and Tuner temporarily
    {0x55, KEY_TERMINAL_AEHA},
    {0x66, KEY_SATELLITE_BS_AEHA},

    // common for tcl
    //{0x00, KEY_CARD_READER},  // CMD_CARD_READER
    { 0x0c, KEY_AV1},  // AV1
    { 0x0d, KEY_AV2},  // AV2
    { 0x01, KEY_ZOOMOUT},    // CMD_FAVCH
    //{0x04, KEY_FREEZE},   // CMD_FREEZE
    //{0x05, KEY_ROI},  // CMD_ROI
    //{0x08, KEY_TOOLS},    // CMD_TOOLS
    { 0x0b, KEY_ENTER},  // CMD_ENTER 
    { 0x10, KEY_NETFLIX},  // CMD_ENTER 
    { 0x12, KEY_GLOBO_PLAY},   // CMD_MENU
    { 0x13, KEY_MENU},   // CMD_MENU
    //{0x16, KEY_AUDIO},
    //{0x17, KEY_VIDEO},
    { 0x17, KEY_GREEN},  // CMD_OPTION_GREEN
    { 0x19, KEY_GLOBAL_PLAY},//GLOBAL_PLAY
    //{0x1a, KEY_WIDE}, // CMD_WIDE
    { 0x1b, KEY_YELLOW}, // CMD_OPTION_YELLOW
    { 0x1c, KEY_QUICK_MENU},//QUICK_MENU
    { 0x1d, KEY_YOUTUBE},
    { 0x20, KEY_LANG },
    { 0x21, KEY_LAST_CHANNEL},
    { 0x25, KEY_HULU},
    { 0x27, KEY_BLUE},   // CMD_OPTION_BLUE   
    { 0x30, KEY_SET},//set
    { 0x32, KEY_NUMBER},
    { 0x45, KEY_NETFLIX_FORWARD},  // CMD_POP
    { 0x46, KEY_TV_ON}, 
    { 0x47, KEY_TV_OFF}, 
    { 0x48, KEY_HDMI_1}, 
    { 0x49, KEY_HDMI2}, 
    { 0x4a, KEY_HDMI3}, 
    { 0x4b, KEY_HDMI4}, 
    { 0x4c, KEY_CMP}, 
    { 0x4d, KEY_VGA}, 
    { 0xF1, KEY_APP_CAREVISION}, //APP_CAREVISION
    //{ 0x55, KEY_TCL_TUNER},  

    { 0x5c, KEY_TV_INPUT_AEHA}, //CMD_SOURCE
    //{0x5d, KEY_CHANNEL_DEC},  // CMD_CHANNEL_DEC
    { 0x60, KEY_ALL_APP}, 
    { 0x61, KEY_ZOOMIN}, 
    { 0x62, KEY_MENU},//menu
    //{ 0x66, KEY_MTS },//mts
    { 0x67, KEY_3D_MODE_AEHA}, //3D_MODE
    { 0x6c, KEY_RC_DOT}, //3D_MODE
    { 0x6f, KEY_ZOOM },       // (C)ASPECT RATIO
    { 0x7f, KEY_SUBTITLE},//tcl

    { 0x9e, KEY_LIST},
    { 0xa0, KEY_RF_LINK_S },//ok+back=bug report for new and old bt controllers; 
    { 0xa3, KEY_RC_RF_UNCONNECT },
    { 0xa5, KEY_AUDIO },      // (C)SOUND_MODE
    { 0xa6, KEY_UP},
    { 0xa7, KEY_DOWN},
    { 0xa8, KEY_RIGHT},
    { 0xa9, KEY_LEFT},
    { 0xaa, KEY_RF_LINK_F},
    { 0xab, KEY_RF_LINK_STOP},
    { 0xac, KEY_NETFLIX_NEXT},
    { 0xad, KEY_RF_LINK_S2 },//home+menu=audiopair for new bt controllers
    { 0xae, KEY_BUG_REPORT },//ok+back=bug report for new remote controllers
    { 0xc0, KEY_MUTE},//tcl
    { 0xc3, KEY_REPLY},    // CMD_DISPLAY tcl
    { 0xc5, KEY_TV},//tv
    { 0xc6, KEY_9},
    { 0xc7, KEY_8},
    { 0xc8, KEY_7},
    { 0xc9, KEY_6},
    { 0xca, KEY_5},
    { 0xcb, KEY_4},
    { 0xcc, KEY_3},
    { 0xcd, KEY_2},
    { 0xce, KEY_1},
    { 0xcf, KEY_0},
    { 0xd0, KEY_VOLUMEUP},   // CMD_VOL_UP
    //{0xd1, KEY_DELETE},
    { 0xd1, KEY_VOLUMEDOWN},     // CMD_VOL_DOWN
    { 0xd2, KEY_CHANNELUP},
    { 0xd3, KEY_CHANNELDOWN},
    { 0xd5, KEY_POWER},
    { 0xd8, KEY_BACK},
    { 0xe0, KEY_NETFLIX_STOP},
    { 0xe1, KEY_TEXT },
    { 0xe2, KEY_NETFLIX_FB},    
    { 0xe3, KEY_NETFLIX_FF},
    { 0xe5, KEY_EPG},    // CMD_EPG
    { 0xe6, KEY_PAUSECD},    
    { 0xe8, KEY_MEDIA_RECORD_AEHA},    
    { 0xea, KEY_MEDIA_PLAY},
    { 0xed, KEY_PICTURE },     // (C)PICTURE_MODE
    { 0xf1, KEY_APP_CAREVISION},  // AV1
    { 0xf3, KEY_FN_F1 },      // APP_CAREVISION
    { 0xf7, KEY_HOME},   // CMD_GUIDE tcl
    { 0xf8, KEY_SLEEP},
    { 0xf9, KEY_EXIT},
    { 0xfd, KEY_USB_MEDIA},      // (C)USB
    { 0xff, KEY_RED},    // CMD_OPTION_RED
};
struct  venus_key_table tcl_jp_tv_key_table = {
    .keys = tcl_jp_tv_keys,
    .size = ARRAY_SIZE(tcl_jp_tv_keys),
};


//end add by TCL_FACTORY
static struct venus_key tcl_nec_tv_keys[] = {
    //customize for korea
    { 0xc3, KEY_INFO},// INFO

    //{0x00, KEY_CARD_READER},  // CMD_CARD_READER
    //{ 0x0c, KEY_AV1},  // AV1
    //{ 0x0d, KEY_AV2},  // AV2
    //{ 0x01, KEY_ZOOMOUT},    // CMD_FAVCH
    //{0x04, KEY_FREEZE},   // CMD_FREEZE
    //{0x05, KEY_ROI},  // CMD_ROI
    //{0x08, KEY_TOOLS},    // CMD_TOOLS
    { 0x0b, KEY_ENTER},  // CMD_ENTER
    { 0x10, KEY_NETFLIX},  // CMD_ENTER
    //{ 0x12, KEY_GLOBO_PLAY},   // CMD_MENU
    { 0x13, KEY_MENU},   // CMD_MENU
    //{0x16, KEY_AUDIO},
    //{0x17, KEY_VIDEO},
    { 0x17, KEY_GREEN},  // CMD_OPTION_GREEN
    //{ 0x19, KEY_GLOBAL_PLAY},//GLOBAL_PLAY
    //{0x1a, KEY_WIDE}, // CMD_WIDE
    { 0x1b, KEY_YELLOW}, // CMD_OPTION_YELLOW
    { 0x1c, KEY_QUICK_MENU},//QUICK_MENU
    //{ 0x1d, KEY_YOUTUBE},
    { 0x20, KEY_LANG },
    //{ 0x21, KEY_LAST_CHANNEL},
    { 0x27, KEY_BLUE},   // CMD_OPTION_BLUE
    { 0x30, KEY_SET},//set
    { 0x32, KEY_NUMBER},
    //{ 0x45, KEY_NETFLIX_FORWARD},  // CMD_POP
    //{ 0x46, KEY_TV_ON},
    //{ 0x47, KEY_TV_OFF},
    //{ 0x48, KEY_HDMI_1},
    //{ 0x49, KEY_HDMI2},
    //{ 0x4a, KEY_HDMI3},
    //{ 0x4b, KEY_HDMI4},
    //{ 0x4c, KEY_CMP},
    //{ 0x4d, KEY_VGA},
    //{ 0xF1, KEY_APP_CAREVISION}, //APP_CAREVISION
    //{ 0x55, KEY_TCL_TUNER},

    { 0x5c, KEY_TV_INPUT_TCL}, //CMD_SOURCE
    //{0x5d, KEY_CHANNEL_DEC},  // CMD_CHANNEL_DEC
    //{ 0x60, KEY_ALL_APP},
    //{ 0x61, KEY_ZOOMIN},
    //{ 0x62, KEY_MENU},//menu
    { 0x66, KEY_MTS },//mts
    //{ 0x67, KEY_3D_MODE_TCL}, //3D_MODE
    { 0x6c, KEY_RC_DOT}, //3D_MODE
    { 0x6f, KEY_ZOOM },       // (C)ASPECT RATIO
    { 0x7f, KEY_SUBTITLE},//tcl

    { 0x9e, KEY_LIST},
    { 0xa0, KEY_RF_LINK_S },//ok+back=bug report for new and old bt controllers;
    { 0xa3, KEY_RC_RF_UNCONNECT },
    //{ 0xa5, KEY_AUDIO },      // (C)SOUND_MODE
    { 0xa6, KEY_UP},
    { 0xa7, KEY_DOWN},
    { 0xa8, KEY_RIGHT},
    { 0xa9, KEY_LEFT},
    { 0xaa, KEY_RF_LINK_F},
    { 0xab, KEY_RF_LINK_STOP},
    { 0xac, KEY_NETFLIX_NEXT},
    { 0xad, KEY_RF_LINK_S2 },//home+menu=audiopair for new bt controllers
    { 0xae, KEY_BUG_REPORT },//ok+back=bug report for new remote controllers
    { 0xc0, KEY_MUTE},//tcl
    //{ 0xc3, KEY_REPLY},    // CMD_DISPLAY tcl
    //{ 0xc5, KEY_TV},//tv
    { 0xc6, KEY_9},
    { 0xc7, KEY_8},
    { 0xc8, KEY_7},
    { 0xc9, KEY_6},
    { 0xca, KEY_5},
    { 0xcb, KEY_4},
    { 0xcc, KEY_3},
    { 0xcd, KEY_2},
    { 0xce, KEY_1},
    { 0xcf, KEY_0},
    { 0xd0, KEY_VOLUMEUP},   // CMD_VOL_UP
    //{0xd1, KEY_DELETE},
    { 0xd1, KEY_VOLUMEDOWN},     // CMD_VOL_DOWN
    { 0xd2, KEY_CHANNELUP},
    { 0xd3, KEY_CHANNELDOWN},
    { 0xd5, KEY_POWER},
    { 0xd8, KEY_BACK},
    { 0xe0, KEY_NETFLIX_STOP},
    //{ 0xe1, KEY_TEXT },
    { 0xe2, KEY_NETFLIX_FB},
    { 0xe3, KEY_NETFLIX_FF},
    { 0xe5, KEY_EPG},    // CMD_EPG
    { 0xe6, KEY_PAUSECD},
    //{ 0xe8, KEY_MEDIA_RECORD_AEHA},
    { 0xea, KEY_MEDIA_PLAY},
    //{ 0xed, KEY_PICTURE },     // (C)PICTURE_MODE
    //{ 0xf1, KEY_APP_CAREVISION},  // AV1
    //{ 0xf3, KEY_FN_F1 },      // APP_CAREVISION
    { 0xf7, KEY_HOME},   // CMD_GUIDE tcl
    //{ 0xf8, KEY_SLEEP},
    { 0xf9, KEY_EXIT},
    //{ 0xfd, KEY_USB_MEDIA},      // (C)USB
    { 0xff, KEY_RED},    // CMD_OPTION_RED
};
struct  venus_key_table tcl_nec_tv_key_table = {
    .keys = tcl_nec_tv_keys,
    .size = ARRAY_SIZE(tcl_nec_tv_keys),
};

//end add by TCL_FACTORY
static struct venus_key tcl_tv_keys[] = {

    //{0x00, KEY_CARD_READER},  // CMD_CARD_READER
    { 0x30, KEY_AV1},  // AV1
    { 0xb0, KEY_AV2},  // AV2
    { 0x80, KEY_ZOOMOUT},    // CMD_FAVCH
    //{0x20, KEY_FREEZE},   // CMD_FREEZE
    //{0xa0, KEY_ROI},  // CMD_ROI
    //{0x10, KEY_TOOLS},    // CMD_TOOLS
    { 0xd0, KEY_ENTER},  // CMD_ENTER
    { 0x08, KEY_NETFLIX},  // CMD_ENTER
    { 0x48, KEY_GLOBO_PLAY},   // CMD_MENU
    { 0xc8, KEY_MENU},   // CMD_MENU
    { 0xa8, KEY_FREEVIEW_PLAY},   // CMD_FREEVIEW_PLAY
    //{0x68, KEY_AUDIO},
    //{0xe8, KEY_VIDEO},
    { 0xe8, KEY_GREEN},  // CMD_OPTION_GREEN
    { 0x98, KEY_GLOBAL_PLAY},//GLOBAL_PLAY
    //{0x58, KEY_WIDE}, // CMD_WIDE
    { 0xd8, KEY_YELLOW}, // CMD_OPTION_YELLOW
    { 0x38, KEY_QUICK_MENU},//QUICK_MENU
    { 0xb8, KEY_YOUTUBE},
    { 0x04, KEY_LANG },
    { 0x84, KEY_LAST_CHANNEL},
    { 0xe4, KEY_BLUE},   // CMD_OPTION_BLUE
    { 0x0c, KEY_SET},//set
    { 0x4c, KEY_NUMBER},
    { 0xa2, KEY_NETFLIX_FORWARD},  // CMD_POP
    { 0x62, KEY_TV_ON},
    { 0xe2, KEY_TV_OFF},
    { 0x12, KEY_HDMI_1},
    { 0x92, KEY_HDMI2},
    { 0x52, KEY_HDMI3},
    { 0xd2, KEY_HDMI4},
    { 0x32, KEY_CMP},
    { 0xb2, KEY_VGA},
    { 0x8f, KEY_APP_CAREVISION}, //APP_CAREVISION
    { 0xaa, KEY_TCL_TUNER},

    { 0x3a, KEY_TV_INPUT_TCL}, //CMD_SOURCE
    //{0x5d, KEY_CHANNEL_DEC},  // CMD_CHANNEL_DEC
    { 0x06, KEY_ALL_APP},
    { 0x86, KEY_ZOOMIN},
    { 0x46, KEY_MENU},//menu
    { 0x66, KEY_MTS },//mts
    { 0xe6, KEY_3D_MODE_TCL}, //3D_MODE
    { 0x36, KEY_RC_DOT}, //3D_MODE
    { 0xf6, KEY_ZOOM },       // (C)ASPECT RATIO
    { 0xfe, KEY_SUBTITLE},//tcl

    { 0x79, KEY_LIST},
    { 0x05, KEY_RF_LINK_S },//ok+back=bug report for new and old bt controllers;
    { 0xc5, KEY_RC_RF_UNCONNECT },
    { 0xa5, KEY_AUDIO },      // (C)SOUND_MODE
    { 0x65, KEY_UP},
    { 0xe5, KEY_DOWN},
    { 0x15, KEY_RIGHT},
    { 0x95, KEY_LEFT},
    { 0x55, KEY_RF_LINK_F},
    { 0xd5, KEY_RF_LINK_STOP},
    { 0x35, KEY_NETFLIX_NEXT},
    { 0xb5, KEY_RF_LINK_S2 },//home+menu=audiopair for new bt controllers
    { 0x75, KEY_BUG_REPORT },//ok+back=bug report for new remote controllers
    { 0x03, KEY_MUTE},//tcl
    { 0xc3, KEY_REPLY},    // CMD_DISPLAY tcl
    { 0xa3, KEY_TV},//tv
    { 0x63, KEY_9},
    { 0xe3, KEY_8},
    { 0x13, KEY_7},
    { 0x93, KEY_6},
    { 0x53, KEY_5},
    { 0xd3, KEY_4},
    { 0x33, KEY_3},
    { 0xb3, KEY_2},
    { 0x73, KEY_1},
    { 0xf3, KEY_0},
    { 0x0b, KEY_VOLUMEUP},   // CMD_VOL_UP
    //{0xd1, KEY_DELETE},
    { 0x8b, KEY_VOLUMEDOWN},     // CMD_VOL_DOWN
    { 0x4b, KEY_CHANNELUP},
    { 0xcb, KEY_CHANNELDOWN},
    { 0xab, KEY_POWER},
    { 0x1b, KEY_BACK},
    { 0x07, KEY_NETFLIX_STOP},
    { 0x87, KEY_TEXT },
    { 0x47, KEY_NETFLIX_FB},
    { 0xc7, KEY_NETFLIX_FF},
    { 0xa7, KEY_EPG},    // CMD_EPG
    { 0x67, KEY_PAUSECD},
    { 0x17, KEY_MEDIA_RECORD_AEHA},
    { 0x57, KEY_MEDIA_PLAY},
    { 0xb7, KEY_PICTURE },     // (C)PICTURE_MODE
    { 0x8f, KEY_APP_CAREVISION},  // AV1
    { 0xcf, KEY_FN_F1 },      // APP_CAREVISION
    { 0xef, KEY_HOME},   // CMD_GUIDE tcl
    { 0x1f, KEY_SLEEP},
    { 0x9f, KEY_EXIT},
    { 0xbf, KEY_USB_MEDIA},      // (C)USB
    { 0xff, KEY_RED},    // CMD_OPTION_RED
};
struct  venus_key_table tcl_tv_key_table = {
    .keys = tcl_tv_keys,
    .size = ARRAY_SIZE(tcl_tv_keys),
};

static struct  venus_key tcl_dvd_keys[] = {
    {0x639c7881, KEY_POWER},
    {0x7f807881, KEY_VOLUMEUP},
    {0x77887881, KEY_VOLUMEDOWN},
    {0x35ca7881, KEY_MUTE},
    {0xe7187881, KEY_POWER},
    {0xed127881, KEY_VOLUMEUP},
    {0xec137881, KEY_VOLUMEDOWN},
    {0xe6197881, KEY_MUTE},
    {0x7f808877, KEY_POWER},
    {0x6e918877, KEY_VOLUMEUP},
    {0x6d928877, KEY_VOLUMEDOWN},
    {0x7e818877, KEY_MUTE},
    {0xe817ac53, KEY_POWER},
    {0xed12ac53, KEY_VOLUMEUP},
    {0xea15ac53, KEY_VOLUMEDOWN},
    {0xe41bac53, KEY_MUTE},
    {0xe817af50, KEY_POWER},
    {0xed12af50, KEY_VOLUMEUP},
    {0xea15af50, KEY_VOLUMEDOWN},
    {0xf40baf50, KEY_MUTE},
    {0xef101645, KEY_POWER},
    {0xdf201645, KEY_VOLUMEUP},
    {0xde211645, KEY_VOLUMEDOWN},
    {0xe01f1645, KEY_MUTE},
    {0xf7080240, KEY_POWER},
    {0xfd020240, KEY_VOLUMEUP},
    {0xfc030240, KEY_VOLUMEDOWN},
    {0xf6090240, KEY_MUTE},
    {0xed12c738, KEY_POWER},
    {0xf10ec738, KEY_VOLUMEUP},
    {0xf00fc738, KEY_VOLUMEDOWN},
    {0xe718c738, KEY_MUTE},
    {0xf50ae31c, KEY_POWER},
    {0xa35ce31c, KEY_VOLUMEUP},
    {0xa25de31c, KEY_VOLUMEDOWN},
    {0xf40be31c, KEY_MUTE},
    {0xf708e718, KEY_POWER},
    {0xfd02e718, KEY_VOLUMEUP},
    {0xfc03e718, KEY_VOLUMEDOWN},
    {0xf609e718, KEY_MUTE},
    {0xeb140e0e, KEY_POWER},
    {0xed120e0e, KEY_VOLUMEUP},
    {0xec130e0e, KEY_VOLUMEDOWN},
    {0xea150e0e, KEY_MUTE},
    {0x28d7f708, KEY_POWER},
    {0x7c83f708, KEY_VOLUMEUP},
    {0x7986f708, KEY_VOLUMEDOWN},
    {0x20dff708, KEY_MUTE},
    {0xf00f0808, KEY_POWER},
    {0xf30c0808, KEY_VOLUMEUP},
    {0xf20d0808, KEY_VOLUMEDOWN},
    {0xf10e0808, KEY_MUTE},
    {0xfd020707, KEY_POWER},
    {0xf8070707, KEY_VOLUMEUP},
    {0xf40b0707, KEY_VOLUMEDOWN},
    {0xf00f0707, KEY_MUTE},
    {0xf00f0606, KEY_POWER},
    {0xf30c0606, KEY_VOLUMEUP},
    {0xf20d0606, KEY_VOLUMEDOWN},
    {0xf10e0606, KEY_MUTE},
    {0xfd020505, KEY_POWER},
    {0xf8070505, KEY_VOLUMEUP},
    {0xf40b0505, KEY_VOLUMEDOWN},
    {0xf00f0505, KEY_MUTE},
    {0xf708fb04, KEY_POWER},
    {0xfd02fb04, KEY_VOLUMEUP},
    {0xfc03fb04, KEY_VOLUMEDOWN},
    {0xf609fb04, KEY_MUTE},
    {0xf00f1004, KEY_POWER},
    {0xf30c1004, KEY_VOLUMEUP},
    {0xf20d1004, KEY_VOLUMEDOWN},
    {0xf10e1004, KEY_MUTE},
    {0xee11fc03, KEY_POWER},
    {0xac53fc03, KEY_VOLUMEUP},
    {0xa857fc03, KEY_VOLUMEDOWN},
    {0xb748fc03, KEY_VOLUMEUP},
    {0xb24dfc03, KEY_VOLUMEDOWN},
    {0xec13fc03, KEY_MUTE},
    //add for koreal bf40
    {0xeb14bf40, KEY_TV_INPUT_TCL},//CMD_SOURCE
    {0xad52bf40, KEY_LAST_CHANNEL},//FACTORY_REMOTE_BACK
    {0xb748bf40, KEY_HOME},
    {0xe619bf40, KEY_UP},
    {0xe21dbf40, KEY_DOWN},
    {0xbd42bf40, KEY_LEFT},
    {0xbf40bf40, KEY_RIGHT},
    {0xde21bf40, KEY_ENTER},
    {0xa45bbf40, KEY_MENU},
    {0xbc43bf40, KEY_EXIT},
    {0xe51abf40, KEY_VOLUMEUP},
    {0xe11ebf40, KEY_VOLUMEDOWN},
    {0xe41bbf40, KEY_CHANNELUP},
    {0xe01fbf40, KEY_CHANNELDOWN},
    {0xfe01bf40, KEY_1},
    {0xfd02bf40, KEY_2},
    {0xfc03bf40, KEY_3},
    {0xfb04bf40, KEY_4},
    {0xfa05bf40, KEY_5},
    {0xf906bf40, KEY_6},
    {0xf807bf40, KEY_7},
    {0xf708bf40, KEY_8},
    {0xf609bf40, KEY_9},
    {0xff00bf40, KEY_0},
};

static unsigned int factory_keycodes[] =
{
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0,
	KEY_ENTER,
	KEY_HOME,
	KEY_UP,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_DOWN,
	KEY_DELETE,
	KEY_MUTE,
	KEY_VOLUMEDOWN,
	KEY_VOLUMEUP,
	KEY_POWER,
	KEY_MENU,
	KEY_BACK,
	KEY_RECORD,
	KEY_EXIT,
	KEY_LANG,
	KEY_PAUSECD,KEY_ALL_APPLICATIONS,
	KEY_REPLY,//aka INFO in IR_rca.kl
};

struct venus_key_table tcl_tv_dvd_key_table = {
    .keys = tcl_dvd_keys,
    .size = ARRAY_SIZE(tcl_dvd_keys),
};
#if defined(CONFIG_RTK_KDRV_TV030_IR )

void set_factory_key_table(struct input_dev *input_dev)
{
    unsigned int i = 0;

    unsigned int keycodes_len = 0;

	if(!input_dev || sizeof(factory_keycodes) == 0)
        return;

	keycodes_len = sizeof(factory_keycodes)/sizeof(factory_keycodes[0]);

	for (i = 0; i < keycodes_len; i++)
		set_bit(factory_keycodes[i], input_dev->keybit);
}

#define FactoryCustomerCodeMax      10
static unsigned int factory_customer_code_map[FactoryCustomerCodeMax] = {
    5, 6, 7, 8, 9, 11, 12, 13, 14, 15
};

int is_factory_customer_code(unsigned int customer_code)
{
    int i = 0;
    for(i = 0 ; i < FactoryCustomerCodeMax; i ++) {
        if(factory_customer_code_map[i] == customer_code)
            break;
    }
    if(i >= FactoryCustomerCodeMax) {
        return 0;
    } else {
        return 1;
    }
}

unsigned int change_bits_order(unsigned int value, unsigned int bits_len)
{
    unsigned int i = 0;
    unsigned int tmp = 0;
    for(i = 0; i < bits_len; i++) {
        tmp = (tmp << 1) | (value & 0x1);
        value = (value >> 1);
    }
    return tmp;
}

#endif

