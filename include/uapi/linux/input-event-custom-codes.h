#ifndef __INPUT_EVENT_CUSTOM_CODES_H__
#define __INPUT_EVENT_CUSTOM_CODES_H__


/*
 * Event types
 */
#define EV_KEY_RAW		0x19
#define EV_KEY_TCL              EV_KEY_RAW

/*
 * Keys and buttons
 *
 * Most of the keys/buttons are modeled after USB HUT 1.12
 * (see http://www.usb.org/developers/hidpage).
 * Abbreviations in the comments:
 * AC - Application Control
 * AL - Application Launch Button
 * SC - System Control
 */

#define KEY_11                  90
#define KEY_12                  91
#define KEY_3_LINE_INPUT        92
#define KEY_D_DATA_BML          93
#define KEY_GUIDE               94
#define KEY_TERMINAL            95
#define KEY_SATELLITE_BS        96
#define KEY_SATELLITE_CS        97
#define KEY_RECORD_LIST         98

#define KEY_MORE_INFO           99
#define KEY_FAST_BACK           101
#define KEY_FAST_FORWARD        102
#define KEY_T_LAUNCH            103
#define KEY_T_SCHEDULE          104

#define KEY_NUMBER              0x233
#define KEY_YOUTUBE             0x239
#define KEY_NETFLIX_FORWARD     0x23e
#define KEY_NETFLIX_NEXT        0X240
#define KEY_NETFLIX_FF          0x241
#define KEY_NETFLIX_STOP        0x242
#define KEY_NETFLIX_FB          0x243
#define KEY_CMP                 0x24d
#define KEY_VGA                 0x24e
#define KEY_AV1                 0x24f
#define KEY_MEDIA_PLAY          0x23f
#define KEY_TV_ON               0x253
#define KEY_TV_OFF              0x254
#define KEY_HDMI3               0x252
#define KEY_HDMI4               0x255
#define KEY_HDMI_1              0x256
#define KEY_HDMI2               0x257
#define KEY_AV2                 0x258
#define KEY_APP_CAREVISION      0X25a
#define KEY_GLOBO_PLAY          0x24a
#define KEY_LANG                199
#define KEY_RF_LINK_STOP        252
#define KEY_MTS                 253
#define KEY_TCL_TUNER           0xfe
#define KEY_RC_DOT              0x10a
#define KEY_USB_MEDIA           0x1e5
#define KEY_LAST_CHANNEL        0x1c7
#define KEY_3D_MODE_AEHA             0x17f
#define KEY_TV_INPUT_AEHA            0x1d7
#define KEY_MEDIA_RECORD_AEHA        167
#define KEY_MOUSE_KEY   84

#define KEY_TTX_SUBPAGE	590
#define KEY_FAV_ADD		591
#define KEY_FAV 		592
#define KEY_FAV_SUB 	593
#define KEY_TTX_CANCEL 	594
#define KEY_REC_LIST 	595


#define KEY_JUMP        596
#define KEY_CAPTION     598

#define KEY_3D_MODE_TCL     0x17f
#define KEY_TV_INPUT_TCL	0x1d7


#define KEY_PICTURE 249
#define KEY_RecentApplicationsDialog 249
#define KEY_SCREENSHOT	250


#define KEY_COOL_LONG    508  //0x1fc
#define KEY_COOL    751  //0x2ef
#define KEY_CHILD_COOL   528 //0x210
#define KEY_CHILD_POWER  530 //0x212
#define KEY_CHILD_HOME   532 //0x214
#define KEY_MENU_LONG    536  //0x218
#define KEY_TV_INPUT_LONG    537  //0x219
#define KEY_SYSTEM_INFO    547  //0x223
#define KEY_NETWORK    548  //0x224
#define KEY_MYAPP    549  //0x225
#define KEY_RESET    550  //0x226
#define KEY_CUSTOMER_KEY_CHANGE    551  //0x227


#define KEY_VOLUME_UP_ANDROID_O      0x2fe
#define KEY_VOLUME_DOWN_ANDROID_O    0x2fd
#define KEY_MUTE_ANDROID_O           0x2fc



#define KEY_24G_TO_INFRARED 0x1bb
#define KEY_INFRARED_TO_24G 0x1bc
#define KEY_KK_LANGUAGE 0x1bd
#define KEY_KK_NET 0x1be
#define KEY_KK_VGA 0x1bf



 //wangzhh add for factory remote control 20130604
#define	KEY_VOLUME_DETECT  0x1c4
#define	KEY_FACTORY_MENU 0x1c5
#define	KEY_BURN_TV 0x1c6
#define	KEY_KK_FAC_RESET 0x1c7
#define	KEY_QUICK_MENU   0x1c8
#define	KEY_BALANCE 0x1c8

#define   KEY_BACKLIGHT_DETECT 0x1c9
#define   KEY_MIC_DETECT 0x1ca
#define   KEY_KK_F1 0x1cb
#define   KEY_KK_F2 0x1cc
#define   KEY_KK_F3  0x1cd
#define   KEY_KK_PROGRAM_FAVORITE  0x1ce
#define   KEY_KK_PROGRAM_LIST  0x1cf



 //wangzhh add for factory remote control 20130604
#define	KEY_KK_CHANNEL_RETURN  0x1e5
#define   KEY_KK_EPG  0x1e6
#define   KEY_KK_3D     0x1e7

#define	KEY_USB  0x1e8


#define	KEY_VOICE_SEARCH 0x1e9
#define	KEY_START_HANDWRITING 0x1ea
#define KEY_FINISH_HANDWRITING 0x1eb
#define KEY_HEADPHONE_INSET 0x1ec
#define KEY_HEADPHONE_PULLOUT  0x1ed

#define KEY_MOUSE_CENTERED  0x1ee
#define KEY_MOUSE_OK_DOWN  0x1ef
#define KEY_MOUSE_OK_UP  0x1f0

#define KEY_MORE	0x20c
// chenshibing,add for bluetooth remote control
#define KEY_BTCTL   0x20d
#define KEY_SET         0x231

#define KEY_PICTURE_MODE    	0x220
#define KEY_SOUND_MODE      	0x221
#define KEY_DISPLAY_RATIO   	0x222
#define KEY_DISPLAY     	0x223
#define KEY_VII         	0x224
#define KEY_SOURCE      	0x225
#define KEY_SHORTCUT   		0x226
#define KEY_HAIER_USB		KEY_MEDIA
#define KEY_RC_RF_UNCONNECT     0x22b
#define KEY_RF_LINK_S           0x22f  
#define KEY_RF_LINK_F           0x22d 
#define KEY_GLOBAL_PLAY		0x251	/* Set Brightness to Maximum */



#define KEY_CARD_READER 0x259
//wangzhh add for customer 20130530
#define KEY_HOLD 0x26a
#define KEY_FREEZE   0x26b
#define KEY_AV    0x26c
#define KEY_YPBPR1 0x26d
#define KEY_HDMI1   0x26e
#define KEY_KK_TV   0x26f



#define KEY_PIP             0x27c
#define KEY_POP             0x27d
#define KEY_MP              0x27e
#define KEY_TOOLS           0x27f

//hardy20140318
#define KEY_FACTORY_LOGOLED			0x280
#define KEY_FACTORY_TEST			0x281
#define KEY_FACTORY_PRESETCH		0x282
#define KEY_FACTORY_AUTOSCANCH		0x283
//end



#define KEY_AUDIOMTS			0x284
#define KEY_AUDIOMODE			0x285
#define KEY_FAVCH			0x287


#define KEY_TPVFAC_TEST1		0x290
#define KEY_TPVFAC_TEST2		0x291
#define KEY_TPVFAC_CLK			0x292	//CLK(test clock  timer)
#define KEY_TPVFAC_VOL_MAX		0x293	//Vol-MAX(set volume to max)
#define KEY_TPVFAC_VOL_BUZZ		0x294	//Vol-Buzz(set volume by require)
#define KEY_TPVFAC_VOL_L		0x295	//VOL-L(set the volume balance to Left SPK,Sound Left)
#define KEY_TPVFAC_VOL_R		0x296	//VOL-R(set the volume balance to Right SPK,Sound Right)
#define KEY_TPVFAC_CC_TT		0x297	//CC/TT (enable/disable CC/TT function)
#define KEY_TPVFAC_B_I			0x298	//Burn in mode On/Off
#define KEY_TPVFAC_CH_SCAN		0x299	//auto scan the channel / exit scan
#define KEY_TPVFAC_PRE_CH		0x29A	//Pre-CH(Factory Presetting channel OK)
#define KEY_TPVFAC_ADTV			0x29B	//Anterna  Cable(change source to TV, and set to Antenna/Digital TV)
#define KEY_TPVFAC_RST			0x29C	//RST(reset to default for Factory settings)
#define KEY_TPVFAC_FAC			0x29D	//FAC
#define KEY_TPVFAC_CSM			0x29E	//CSM
#define KEY_TPVFAC_PATTERN		0x29F	//Pattern(set Pattern to Red/Green/ Blue/White/Black/Off)
#define KEY_TPVFAC_CTC			0x2A0	//CTC(show color temperature---e.g:normal/warm/cool)
#define KEY_TPVFAC_CH_UP		0x2A1	//Ch+
#define KEY_TPVFAC_CH_DOWN		0x2A2	//Ch-
#define KEY_TPVFAC_MENU			0x2A3	//MENU
#define KEY_TPVFAC_MUTE			0x2A4	//MUTE
#define KEY_TPVFAC_UP			0x2A5	//UP
#define KEY_TPVFAC_DOWN			0x2A6	//DOWN
#define KEY_TPVFAC_OK			0x2A7	//OK/ENTER
#define KEY_TPVFAC_PIC			0x2A8	//PIC(Change picture mode--- e.g.  User/vivid/standard/movie etc))
#define KEY_TPVFAC_AUDIO		0x2A9	//Audio(enable/disable SRS, or change the audio mode)
#define KEY_TPVFAC_2D3D			0x2AA	//2D/3D(switch 3D /2D Mode )
#define KEY_TPVFAC_ARC			0x2AB	//ARC(ENABLE/DISABLE ARC)
#define KEY_TPVFAC_CI			0x2AC	//CI+(display CI+KEY Status)
#define KEY_TPVFAC_VIRGIN		0x2AD	//Virgin(Virgin Mode on/off)
#define KEY_TPVFAC_CVBS			0x2AE	//CVBS(Change Source to AV)
#define KEY_TPVFAC_YPBPR		0x2AF	//YPbPr SCART(Change Source to YPbPr)
#define KEY_TPVFAC_HDMI			0x2B0	//HDMI(Change Source to HDMI)
#define KEY_TPVFAC_VGA			0x2B1	//VGA(Change Source to VGA)
#define KEY_TPVFAC_CLONE		0x2B2	//CLONE(test clone box port)
#define KEY_TPVFAC_DCR			0x2B3	//DCR(Enable/Disable DCR function)
#define KEY_TPVFAC_ADC			0x2B4	//ADC(only activity when source in  YPbPr & PC)
#define KEY_TPVFAC_BLK			0x2B5	//BLK(Backlight set to 50%/100%/0%)
#define KEY_TPVFAC_WP			0x2B6	//W/P(EDID Write Control)
#define KEY_TPVFAC_LS			0x2B7	//Light sensor(ENABLE/DISABLE Light sensor)
#define KEY_TPVFAC_USB			0x2B8	//USB(Change Source to USB)
#define KEY_TPVFAC_RJ45			0x2B9	//RJ45(test RJ45 port)
#define KEY_TPVFAC_RS232		0x2BA	//RS232(RS232 ON/OFF)
#define KEY_TPVFAC_0			0x2BB	//Number 0 Key
#define KEY_TPVFAC_1			0x2BC
#define KEY_TPVFAC_2			0x2BD
#define KEY_TPVFAC_3			0x2BE
#define KEY_TPVFAC_4			0x2BF
#define KEY_TPVFAC_5			0x2C0
#define KEY_TPVFAC_6			0x2C1
#define KEY_TPVFAC_7			0x2C2
#define KEY_TPVFAC_8			0x2C3
#define KEY_TPVFAC_9			0x2C4

//lyg,20140325,CVT remote
#define KEY_CVTFAC_10_P			0x290
#define KEY_CVTFAC_DOT			0x291
#define KEY_CVTAUTO				0x292
#define KEY_CVTFAC_PVR			0x293
#define KEY_CVTFAC_TUNE			0x294
#define KEY_CVTFAC_CALIBRATION1	0x295
#define KEY_CVTFAC_CALIBRATION2	0x296
#define KEY_CVTFAC_VERSION		0x297
#define KEY_CVTFAC_AGING_MODE	0x298
#define KEY_CVTFAC_PANEL1		0x299
#define KEY_CVTFAC_PANEL2		0x29a
#define KEY_CVTFAC_F1			0x29b
#define KEY_CVTFAC_F2			0x29c
#define KEY_CVTFAC_F3			0x29d
#define KEY_CVTFAC_F4			0x29e
#define KEY_CVTFAC_F5			0x29f
#define KEY_CVTFAC_F6			0x2a0
#define KEY_CVTFAC_F7			0x2a1
#define KEY_CVTFAC_NICAM		0x2a2
#define KEY_CVTFAC_TTXCC		0x2a3
#define KEY_CVTFAC_RESET		0x2a4
//lyg,20140325,CVT remote end

//lyg,20140403,MTC remote
#define KEY_MTCFAC_DTV1			0x2a5
#define KEY_MTCFAC_DTV2			0x2a6
#define KEY_MTCFAC_DTV3			0x2a7
#define KEY_MTCFAC_REC			0x2a8
#define KEY_MTCFAC_ATV1			0x2a9
#define KEY_MTCFAC_ATV2			0x2aa
#define KEY_MTCFAC_ATV3			0x2ab
#define KEY_MTCFAC_SCART1		0x2ac
#define KEY_MTCFAC_SCART2		0x2ad
#define KEY_MTCFAC_DVD			0x2ae
#define KEY_MTCFAC_DVB_S		0x2af
#define KEY_MTCFAC_DVD_USB		0x2b0
#define KEY_MTCFAC_DVD_DISK		0x2b1
#define KEY_MTCFAC_DVB_S_RESET	0x2b2
#define KEY_MTCFAC_PLAY			0x2b3
#define KEY_MTCFAC_STOP			0x2b4
#define KEY_MTCFAC_RESET		0x2b5

#define KEY_11_AEHA                  KEY_NEWS
#define KEY_12_AEHA                 KEY_VOICEMAIL
#define KEY_3_LINE_INPUT_AEHA        KEY_ADDRESSBOOK
#define KEY_D_DATA_BML_AEHA         KEY_MESSENGER
#define KEY_TERMINAL_AEHA            KEY_DISPLAYTOGGLE
#define KEY_SATELLITE_BS_AEHA        KEY_SPELLCHECK
#define KEY_SATELLITE_CS_AEHA        KEY_LOGOFF
#define KEY_RECORD_LIST_AEHA         KEY_DOLLAR
#define KEY_T_SCHEDULE_AEHA          KEY_EURO
#define KEY_ALL_APP             0x24c
#define KEY_RF_LINK_S2          0x1b5
#define KEY_BUG_REPORT          0x1b4 
#define KEY_HULU	0x23a
#define KEY_FREEVIEW_PLAY       0x26b

#if defined(CONFIG_RTK_KDRV_YK_54LU_SHUTTLE)|| defined(CONFIG_RTK_KDRV_YK_54LU)
#define KEY_NEW_SHUTTLE_LEFT   		26
#define KEY_NEW_SHUTTLE_RIGHT  	 	27
#define KEY_CENTER					28
#define KEY_MEDIA_NEXT				163
#define KEY_MEDIA_PLAY_PAUSE		164
#define KEY_MEDIA_PREVIOUS			165
#define KEY_MEDIA_STOP				166
#define KEY_MEDIA_RECORD_YK			167
#define KEY_MEDIA_REWIND			168
#define KEY_SENSE_ALL 				183
#define KEY_SENSE_MENU				184
#define KEY_SENSE_CENTER			185
#define KEY_SENSE_BACK				186
#define KEY_SENSE_VOLUME_UP			187
#define KEY_SENSE_VOLUME_DOWN		188
#define KEY_SENSE_CHANNEL_UP		189
#define KEY_SENSE_CHANNEL_DOWN		190
#define KEY_SENSE_LEAVE				191
#define KEY_MEDIA_FAST_FORWARD		208

#define KEY_SCREEN_DISPLAY     		0x166

#define KEY_3D_DEL					0x1d0
#define KEY_ALTERNATE				0x1d1
#define KEY_VOICE       			0x1d3
#define KEY_TV_INPUT_YK				0x1d2
#define KEY_SHARE					0x1d4
#define KEY_IMAGE_MODE    			0x1d5
#define KEY_SOUND_MODE_YK_54LU      		0x1d6
#define KEY_DISPLAY_MODE   			0x1d7
#define KEY_MEDIA_SONG_SYSTEM		0x1d9
#define KEY_MEDIA_RELATIONSHIP		0x1da
#define KEY_MEDIA_BOOKING			0x1db
#define KEY_MEDIA_AUDIO_CONTROL		0x1dc
#define KEY_MEDIA_FUNCTION			0x1dd
#define KEY_MEDIA_ORIGINAL_SOUNDTRACK	0x1de
#define KEY_MEDIA_SELECTED_SONGS	0x1df

#define KEY_MEDIA_PRIORITY			0x1e0
#define KEY_MEDIA_DELETE			0x1e1
#define KEY_MEDIA_SCORE_DISPLAY		0x1e2
#define KEY_INPUT_NUMBER			0x1e3
#define KEY_3D_MODE_YK    		0x1e4

#define KEY_SEEK_SIGNAL				0x1f1
#define KEY_CENTER_LONG				0x1f2
#define KEY_LOCAL_MEDIA				0x1f3
#define KEY_RESEARCH				0x1f4
#define KEY_SETTING					0x1f5
#define KEY_MYSTEP					0x1f6

#define KEY_FACTORY_FACTORY_MODE	0x200
#define KEY_FACTORY_RESET			0x201
#define KEY_FACTORY_SOURCE_ADD		0x202
#define KEY_FACTORY_SOURCE_REDUCE	0x203
#define KEY_FACTORY_OUTSET			0x204
#define KEY_FACTORY_BUS_OFF			0x205
#define KEY_FACTORY_AGING_MODE		0x206
#define KEY_FACTORY_AUTO_ADC		0x207
#define KEY_FACTORY_AV1				0x208
#define KEY_FACTORY_RF_AGC			0x209
#define KEY_FACTORY_AV2				0x20a
#define KEY_FACTORY_AV3				0x20b

#define KEY_FACTORY_S1				0x2c0
#define KEY_FACTORY_YUV1			0x2c1
#define KEY_FACTORY_YUV2			0x2c2
#define KEY_FACTORY_VGA				0x2c3
#define KEY_FACTORY_HDMI1			0x2c4
#define KEY_FACTORY_HDMI2			0x2c5
#define KEY_FACTORY_HDMI3			0x2c6
#define KEY_FACTORY_KALA_OK			0x2c7
#define KEY_FACTORY_UPLAYER			0x2c8
#define KEY_FACTORY_LAN				0x2c9
#define KEY_FACTORY_DREAM_PANEL		0x2ca
#define KEY_FACTORY_WHITE_BALANCE	0x2cb
#define KEY_FACTORY_ALONE_LISTEN	0x2cc
#define KEY_FACTORY_CA_CARD			0x2cd
#define KEY_FACTORY_BARCODE			0x2ce
#define KEY_FACTORY_SEARCH_UP		0x2cf

#define KEY_FACTORY_SEARCH_DOWN		0x2d0
#define KEY_CHILD_LOCK 				0x2d1
#define KEY_SHUTTLE_LEFT_SPEED_1	0x2d4
#define KEY_SHUTTLE_LEFT_SPEED_2	0x2d5
#define KEY_SHUTTLE_LEFT_SPEED_3	0x2d6
#define KEY_SHUTTLE_LEFT_SPEED_4	0x2d7
#define KEY_SHUTTLE_LEFT_SPEED_5	0x2d8
#define KEY_SHUTTLE_RIGHT_SPEED_1	0x2da
#define KEY_SHUTTLE_RIGHT_SPEED_2	0x2db
#define KEY_SHUTTLE_RIGHT_SPEED_3	0x2dc
#define KEY_SHUTTLE_RIGHT_SPEED_4	0x2dd
#define KEY_SHUTTLE_RIGHT_SPEED_5	0x2de

#define KEY_USER_MOUSE	0x2f1
#define KEY_USER_VOICE	0x2f2

#define KEY_TIME_SPOT				0x2e0
#define KEY_USER_SWITCH				0x2e1
#define KEY_VOICE_END				0x2e2
#define KEY_POWER_LONG				0x2e3
#define KEY_COOCAA					0x2e4
#define KEY_HOME_LONG				0x2e5
#define KEY_BACK_LONG				0x2e6
#define KEY_NOTIFICATION			0x2e7
#define KEY_FACTORY_LOGOUT			0X2d2
#define KEY_FACTORY_DTMB			0X20d
#define KEY_FACTORY_PLC				0x20c
#define KEY_FACTORY_BLUETOOTH		0x1f8
#define KEY_FACTORY_SPEAKER_SWITCH	0x2d3
#define KEY_FACTORY_SERIAL232		0x2d9
#define KEY_FACTORY_WLAN_SCAN_YK 		0x2df
#define KEY_FACTORY_WHITE_DEBUG         0x2f1
#define KEY_FACTORY_WLAN_SCAN 		0x2e8

#else
#define KEY_VOICE       				0x228
#define KEY_3D_DEL						0x229
#define KEY_MEDIA_SONG_SYSTEM			0x22a
#define KEY_MEDIA_RELATIONSHIP			0x22b
#define KEY_MEDIA_BOOKING				0x22c
#define KEY_MEDIA_AUDIO_CONTROL			0x22d
#define KEY_MEDIA_FUNCTION				0x22e
#define KEY_MEDIA_ORIGINAL_SOUNDTRACK	0x22f

#define KEY_MEDIA_SELECTED_SONGS	0x230
#define KEY_MEDIA_PRIORITY			0x231
#define KEY_MEDIA_DELETE			0x232
#define KEY_MEDIA_SCORE_DISPLAY		0x233
#define KEY_INPUT_NUMBER			0x234
#define KEY_SHUTTLE_LEFT_SPEED_1	0x235
#define KEY_SHUTTLE_LEFT_SPEED_2	0x236
#define KEY_SHUTTLE_LEFT_SPEED_3	0x237
#define KEY_SHUTTLE_LEFT_SPEED_4	0x238
#define KEY_SHUTTLE_LEFT_SPEED_5	0x239
#define KEY_SHUTTLE_RIGHT_SPEED_1	0x23a
#define KEY_SHUTTLE_RIGHT_SPEED_2	0x23b
#define KEY_SHUTTLE_RIGHT_SPEED_3	0x23c
#define KEY_SHUTTLE_RIGHT_SPEED_4	0x23d
#define KEY_SHUTTLE_RIGHT_SPEED_5	0x23e
#define KEY_TIME_SPOT				0x23f

#define KEY_USER_SWITCH				0x240
#define KEY_VOICE_END				0x241
#define KEY_POWER_LONG				0x242
#define KEY_FACTORY_FACTORY_MODE	0x243
#define KEY_FACTORY_RESET			0x244
#define KEY_FACTORY_SOURCE_ADD		0x245
#define KEY_FACTORY_SOURCE_REDUCE	0x246
#define KEY_FACTORY_OUTSET		0x247
#define KEY_FACTORY_BUS_OFF		0x248
#define KEY_FACTORY_AGING_MODE	0x249
#define KEY_FACTORY_AUTO_ADC	0x24a
#define KEY_FACTORY_AV1			0x24b
#define KEY_FACTORY_RF_AGC		0x24c
#define KEY_FACTORY_AV2			0x24d
#define KEY_FACTORY_AV3			0x24e
#define KEY_FACTORY_S1			0x24f
#define KEY_FACTORY_YUV1		0x250
#define KEY_FACTORY_YUV2		0x251
#define KEY_FACTORY_VGA			0x252
#define KEY_FACTORY_HDMI1		0x253
#define KEY_FACTORY_HDMI2		0x254
#define KEY_FACTORY_HDMI3		0x255
#define KEY_FACTORY_KALA_OK		0x256
#define KEY_FACTORY_UPLAYER		0x257
#define KEY_FACTORY_LAN			0x258
#define KEY_FACTORY_DREAM_PANEL		0x259
#define KEY_FACTORY_WHITE_BALANCE	0x25a
#define KEY_FACTORY_ALONE_LISTEN	0x25b
#define KEY_FACTORY_CA_CARD			0x25c
#define KEY_FACTORY_BARCODE			0x25d
#define KEY_FACTORY_SEARCH_UP		0x25e
#define KEY_FACTORY_SEARCH_DOWN		0x25f

#define KEY_SENSE_ALL 			0x260
#define KEY_SENSE_MENU			0x261
#define KEY_SENSE_CENTER		0x262
#define KEY_SENSE_BACK			0x263
#define KEY_SENSE_VOLUME_UP		0x264
#define KEY_SENSE_VOLUME_DOWN	0x265
#define KEY_SENSE_CHANNEL_UP	0x266
#define KEY_SENSE_CHANNEL_DOWN	0x267
#define KEY_SENSE_LEAVE			0x268
#define KEY_CHILD_LOCK 			0x269

#define KEY_NEW_SHUTTLE_LEFT   	0x270
#define KEY_NEW_SHUTTLE_RIGHT   0x271
#define KEY_COOCAA				0x272
#define KEY_HOME_LONG			0x273
#define KEY_BACK_LONG			0x274
#define KEY_NOTIFICATION		0x275
#define KEY_CENTER_LONG			0x276
#define KEY_SEEK_SIGNAL			0x277
#define KEY_LOCAL_MEDIA			0x278
#define KEY_RESEARCH			0x279
#define KEY_SETTING				0x27a
#define KEY_MYSTEP				0x27b
#endif

#ifdef CONFIG_RTK_KDRV_SUPPORT_ALC_COMMAND
#ifdef CONFIG_RTK_KDRV_YK_54LU
#if defined(CONFIG_RTK_KDRV_YK_54LU_SHUTTLE) ||defined(CONFIG_RTK_KDRV_YK_54LU)
#define KEY_ALC_HELLO_MIDOLI	(KEY_POWER)
#define KEY_ALC_HELLO_MENU (KEY_HOME)
#define KEY_ALC_HELLO_VOLUME_LOUDER (KEY_VOLUMEUP)
#define KEY_ALC_HELLO_VOLUME_DOWN (KEY_VOLUMEDOWN)

#define KEY_ALC_CONFIRM	(KEY_CENTER)
#define KEY_ALC_START_PLAY (KEY_MEDIA_PLAY_PAUSE)
#define KEY_ALC_PAUSE (KEY_MEDIA_PLAY_PAUSE)
#define KEY_ALC_STOP (KEY_MEDIA_STOP)

#define KEY_ALC_GO_PREVIOUS (KEY_UP)
#define KEY_ALC_GO_NEXT (KEY_DOWN)
#define KEY_ALC_GO_RIGHT (KEY_RIGHT)
#define KEY_ALC_GO_LEFT (KEY_LEFT)
#define KEY_ALC_ALEXA (KEY_MUTE)
#else
#define KEY_ALC_HELLO_MIDOLI	(KEY_POWER)
#define KEY_ALC_HELLO_MENU (KEY_HOME)
#define KEY_ALC_HELLO_VOLUME_LOUDER (KEY_VOLUMEUP)
#define KEY_ALC_HELLO_VOLUME_DOWN (KEY_VOLUMEDOWN)

#define KEY_ALC_CONFIRM	(KEY_ENTER)
#define KEY_ALC_START_PLAY (KEY_RED)
#define KEY_ALC_PAUSE (KEY_GREEN)
#define KEY_ALC_STOP (KEY_BLUE)

#define KEY_ALC_GO_PREVIOUS (KEY_UP)
#define KEY_ALC_GO_NEXT (KEY_DOWN)
#define KEY_ALC_GO_RIGHT (KEY_RIGHT)
#define KEY_ALC_GO_LEFT (KEY_LEFT)
#define KEY_ALC_ALEXA (KEY_MUTE)
#endif
#else
#define KEY_ALC_HELLO_MIDOLI	0x2e8
#define KEY_ALC_HELLO_MENU 0x2e9
#define KEY_ALC_HELLO_VOLUME_LOUDER 0x2ea
#define KEY_ALC_HELLO_VOLUME_DOWN 0x2eb

#define KEY_ALC_CONFIRM	0x2ec
#define KEY_ALC_START_PLAY 0x2ed
#define KEY_ALC_PAUSE 0x2ee
#define KEY_ALC_STOP 0x2ef

#define KEY_ALC_GO_PREVIOUS	0x2f0
#define KEY_ALC_GO_NEXT 0x2f1
#define KEY_ALC_GO_RIGHT 0x2f2
#define KEY_ALC_GO_LEFT 0x2f3
#endif
#endif

#define KEY_SYNC_MENU			0x2f4
#define KEY_GOOGLE_PLAY			0x2f5
#define KEY_NETFLIX			0x2f6
#define KEY_RETUEN_CH			0x2f7
#define KEY_PIC_OFF			0x2f8
#define KEY_PRIME_VIDEO                 0x2f9
/* KEY_FREEVIEW_PLAY which defined above is for customer, add new definition here for Realtek */
#define KEY_FVP                         0x2fa
#define KEY_APPS                          0x2fb
#define KEY_VIRTUAL_KEYBOARD            0x2ff

#define KEY_RESERVER_MIN  (KEY_MAX - 6)
#define KEY_RESERVER_MAX (KEY_MAX - 1)



#endif
