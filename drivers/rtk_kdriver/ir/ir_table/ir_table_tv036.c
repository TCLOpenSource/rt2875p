#include "ir_input.h"

static struct venus_key Hikeen_10D_tv_keys[] = 
{
	{0x1C, KEY_POWER},
	{0x08, KEY_MUTE},

	{0x54, KEY_1},	
	{0x16, KEY_2},
	{0x15, KEY_3},

    {0x14, KEY_4},
	{0x50, KEY_5},     
	{0x12, KEY_6},

	{0x11, KEY_7},
    {0x10, KEY_8},
	{0x0E, KEY_9},

    {0x4C, KEY_JUMP},	  //channel recall
	{0x0D, KEY_0},
	{0x0C, KEY_3D_MODE},    // -/--
	
	{0x1F, KEY_SUBTITLE},
	{0x1B, KEY_VIDEO},	//P.mode
	{0x03, KEY_SOUND_MODE},//S.mode

	{0x18, KEY_SOURCE},	
    {0x17, KEY_INFO},
	{0x49, KEY_MENU},
	{0x0A, KEY_BACK},//EXIT

	{0x1A, KEY_UP},		
    {0x48, KEY_DOWN},	
	{0x47, KEY_LEFT},
	{0x07, KEY_RIGHT},
	{0x06, KEY_ENTER},
	{0x5A, KEY_HOME},

	{0x4B, KEY_VOLUMEUP},
    {0x4F, KEY_VOLUMEDOWN},	
	{0x09, KEY_CHANNELUP},
	{0x05, KEY_CHANNELDOWN},

    {0x4A, KEY_SLEEP},	
	{0x4E, KEY_AUDIO},      //Nicam key
	{0x1E, KEY_TTX_SUBPAGE},     //subpage

	{0x58, KEY_FAV_ADD},	// TT hold
	{0x5C, KEY_FAV},	// FAVORITES and Mix
	{0x40, KEY_FAV_SUB},             // TT reveal
	{0x02, KEY_TTX_CANCEL},          //TTX canel

	{0x5D, KEY_RED},		
	{0x59, KEY_GREEN},	    
	{0x51, KEY_YELLOW},    
	{0x55, KEY_BLUE},	

	{0x5E, KEY_FREEZE},    //FREEZE //TT Title
	{0x5A, KEY_LOCAL_MEDIA}, 
	{0x52, KEY_TEXT},		//TTX TEXT
	{0x56, KEY_LIST},    //TT index	

	{0x5B, KEY_FASTFORWARD},	// FF
	{0x5F, KEY_REWIND},    		//FR
	{0x53, KEY_PREVIOUS},		//prev
	{0x57, KEY_NEXT},			//next

	{0x43, KEY_EPG},
	{0x41, KEY_PLAYPAUSE},
	{0x01, KEY_STOP},
	{0x19, KEY_ZOOM},  //aspect && TT SIZE

	{0x42, KEY_RADIO},       //dtv/radio
	{0x45, KEY_PVR},       //rec.task
	{0x46, KEY_REC_LIST},       //rec.file
	{0x1D, KEY_RECORD},//

};

struct  venus_key_table Hikeen_10D_tv_key_table = {
	.keys = Hikeen_10D_tv_keys,
	.size = ARRAY_SIZE(Hikeen_10D_tv_keys),
};