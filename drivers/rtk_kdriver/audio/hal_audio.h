
/** @file rhal_audio.h
 *
 *
 *
 *  @version 1.0
 *  @date 2017.09.14
 *  @note
 *  @see
 */

/******************************************************************************
    Header File Guarder
******************************************************************************/
#ifndef _RHAL_AUDIO_H_
#define _RHAL_AUDIO_H_

#include "audio_hw_port.h"
#include "audio_hw_atv.h"
#include "audio_hw_aio.h"
#include "audio_hw_app.h"
#include "audio_hw_driver.h"
#include "audio_hw_ioctl.h"
#include "hal_common.h"

/**
 * HAL AUDIO MS12 Version.
 *
 */
typedef  enum
{
	HAL_AUDIO_MS12_1_3		= 0, /*MS12 v1.3 Dose not support TVATMOS, AC4*//*Default value*/
	HAL_AUDIO_MS12_2_0		= 1, /*MS12 v2.0 Support TVATMOS, AC4*/
	HAL_AUDIO_MS12_MAX	= HAL_AUDIO_MS12_2_0,
} HAL_AUDIO_MS12_VERSION_T;

/**
 * HAL AUDIO Decoder Index.
 *
 */
typedef  enum
{
	HAL_AUDIO_ADEC0		= 0,
	HAL_AUDIO_ADEC1		= 1,
	HAL_AUDIO_ADEC_MAX	= HAL_AUDIO_ADEC1,
} HAL_AUDIO_ADEC_INDEX_T;

/**
 * HAL AUDIO Index.
 *
 */
typedef  enum
{
	HAL_AUDIO_INDEX0	= 0, /*Audio Decoder Input 0*/
	HAL_AUDIO_INDEX1	= 1, /*Audio Decoder Input 1*/
	HAL_AUDIO_INDEX2	= 2, /*Audio Mixer Input 0*/
	HAL_AUDIO_INDEX3	= 3, /*Audio Mixer Input 1*/
	HAL_AUDIO_INDEX4	= 4, /*Audio Mixer Input 2*/
	HAL_AUDIO_INDEX5	= 5, /*Audio Mixer Input 3*/
	HAL_AUDIO_INDEX6	= 6, /*Audio Mixer Input 4*/
	HAL_AUDIO_INDEX7	= 7, /*Audio Mixer Input 5*/
	HAL_AUDIO_INDEX8	= 8, /*Audio Mixer Input 6*/
	HAL_AUDIO_INDEX9	= 9, /*Audio Mixer Input 7*/
	HAL_AUDIO_INDEX_MAX	= HAL_AUDIO_INDEX9,
} HAL_AUDIO_INDEX_T;


/**
 * HAL AUDIO Mixer Index.
 *
 */
typedef  enum
{
	HAL_AUDIO_MIXER0	= 0,	/* G-Streamer Mixer Input 0 */
	HAL_AUDIO_MIXER1	= 1,	/* G-Streamer Mixer Input 1 */
	HAL_AUDIO_MIXER2	= 2,	/* G-Streamer Mixer Input 2 */
	HAL_AUDIO_MIXER3	= 3,	/* G-Streamer Mixer Input 3 */
	HAL_AUDIO_MIXER4	= 4,	/* G-Streamer Mixer Input 4 */
	HAL_AUDIO_MIXER5	= 5,	/* HAL  AUDIO Mixer Input 5 */
	HAL_AUDIO_MIXER6	= 6,	/* ALSA AUDIO Mixer Input 6 */
	HAL_AUDIO_MIXER7	= 7,	/* ALSA AUDIO Mixer Input 7 */
	HAL_AUDIO_MIXER_MAX	= HAL_AUDIO_MIXER7,
} HAL_AUDIO_MIXER_INDEX_T;

/**
 * HAL AUDIO TP Index.
 *
 */
typedef  enum
{
	HAL_AUDIO_TP0		= 0,
	HAL_AUDIO_TP1		= 1,
	HAL_AUDIO_TP_MAX	= HAL_AUDIO_TP1,
} HAL_AUDIO_TP_INDEX_T;

/**
 * HAL AUDIO HDMI Port Index.
 * HAL_AUDIO_HDMI_SWITCH is defined for HDMI Switch Model and 1 Port Only in SoC.
 *
 */
typedef  enum
{
	HAL_AUDIO_HDMI0			= 0,	/* H15  : HDMI2.0 Port 0. */
	HAL_AUDIO_HDMI1			= 1,	/* H15  : HDMI2.0 Port 1. */
	HAL_AUDIO_HDMI2 		= 2,	/* H15  : HDMI1.4 Port 0 with ARC. */
	HAL_AUDIO_HDMI3 		= 3,	/* H15  : HDMI1.4 Port 1 with MHL. */
	HAL_AUDIO_HDMI_SWITCH 	= 4,	/* M14+ : HDMI1.4 Port with HDMI Switch Model. */
	HAL_AUDIO_HDMI_MAX		= HAL_AUDIO_HDMI_SWITCH,
} HAL_AUDIO_HDMI_INDEX_T;

/**
 * HAL AUDIO Sound Output Mode.
 *
 */
typedef  enum
{
	HAL_AUDIO_NO_OUTPUT     = 0x00,
	HAL_AUDIO_SPK           = 0x01,
	HAL_AUDIO_SPDIF         = 0x02,
	HAL_AUDIO_SB_SPDIF      = 0x04,
	HAL_AUDIO_SB_PCM        = 0x08,
	HAL_AUDIO_SB_CANVAS     = 0x10,
	HAL_AUDIO_HP            = 0x20,
	HAL_AUDIO_SCART         = 0x40,
	HAL_AUDIO_SPDIF_ES      = 0x80,
	HAL_AUDIO_BT            = 0x100,
} HAL_AUDIO_SNDOUT_T;

/**
 * HAL AUDIO Sound Output LR Mode.
 *
 */
typedef enum
{
	HAL_AUDIO_SNDOUT_LRMODE_LR 		= 0,
	HAL_AUDIO_SNDOUT_LRMODE_LL 		= 1,
	HAL_AUDIO_SNDOUT_LRMODE_RR 		= 2,
	HAL_AUDIO_SNDOUT_LRMODE_MIX		= 3,
} HAL_AUDIO_SNDOUT_LRMODE_T;

/**
 * HAL AUDIO Resource Type for Inter Connection.
 *
 */
typedef  enum
{
	HAL_AUDIO_RESOURCE_SDEC0			=  0,
	HAL_AUDIO_RESOURCE_SDEC1			=  1,
	HAL_AUDIO_RESOURCE_ATP0				=  2,
	HAL_AUDIO_RESOURCE_ATP1				=  3,
	HAL_AUDIO_RESOURCE_ADC				=  4,
	HAL_AUDIO_RESOURCE_HDMI				=  5,
	HAL_AUDIO_RESOURCE_AAD				=  6,
	HAL_AUDIO_RESOURCE_SYSTEM			=  7,	/* Clip or LMF Play */
	HAL_AUDIO_RESOURCE_ADEC0			=  8,
	HAL_AUDIO_RESOURCE_ADEC1			=  9,
	HAL_AUDIO_RESOURCE_AENC0			= 10,
	HAL_AUDIO_RESOURCE_AENC1			= 11,
	HAL_AUDIO_RESOURCE_SE				= 12,
	HAL_AUDIO_RESOURCE_OUT_SPK			= 13,	/* Speaker */
	HAL_AUDIO_RESOURCE_OUT_SPDIF		= 14,	/* SPDIF Ouput */
	HAL_AUDIO_RESOURCE_OUT_SB_SPDIF		= 15,	/* Sound Bar(SPDIF) : Mixer Output */
	HAL_AUDIO_RESOURCE_OUT_SB_PCM		= 16,	/* Sound Bar(PCM)   : Mixer Output(Wireless) */
	HAL_AUDIO_RESOURCE_OUT_SB_CANVAS	= 17,	/* Sound Bar(CANVAS): Sound Engine Output */
	HAL_AUDIO_RESOURCE_OUT_HP			= 18,	/* Must be controlled by audio decoder.*/
	HAL_AUDIO_RESOURCE_OUT_SCART		= 19,	/* Must be controlled by audio decoder.*/
	HAL_AUDIO_RESOURCE_MIXER0			= 20,	/* Audio Mixer Input 0. */
	HAL_AUDIO_RESOURCE_MIXER1			= 21,	/* Audio Mixer Input 1. */
	HAL_AUDIO_RESOURCE_MIXER2			= 22,	/* Audio Mixer Input 2. */
	HAL_AUDIO_RESOURCE_MIXER3			= 23,	/* Audio Mixer Input 3. */
	HAL_AUDIO_RESOURCE_MIXER4			= 24,	/* Audio Mixer Input 4. */
	HAL_AUDIO_RESOURCE_MIXER5			= 25,	/* Audio Mixer Input 5. */
	HAL_AUDIO_RESOURCE_MIXER6			= 26,	/* Audio Mixer Input 6. */
	HAL_AUDIO_RESOURCE_MIXER7			= 27,	/* Audio Mixer Input 7. */
	HAL_AUDIO_RESOURCE_OUT_SPDIF_ES		= 28,	/* SPDIF ES Ouput Only */
	HAL_AUDIO_RESOURCE_HDMI0			= 29,	/* Audio HDMI Input 0. */
	HAL_AUDIO_RESOURCE_HDMI1			= 30,	/* Audio HDMI Input 1. */
	HAL_AUDIO_RESOURCE_HDMI2			= 31,	/* Audio HDMI Input 2. */
	HAL_AUDIO_RESOURCE_HDMI3			= 32,	/* Audio HDMI Input 3. */
	HAL_AUDIO_RESOURCE_SWITCH			= 33,	/* Audio HDMI Input with switch. */
	HAL_AUDIO_RESOURCE_DP				= 34,	/* Audio Display Port. */
	HAL_AUDIO_RESOURCE_MAX				= 35,	/* Audio Resource Max */
	HAL_AUDIO_RESOURCE_NO_CONNECTION 	= 0XFF,
} HAL_AUDIO_RESOURCE_T;

/**
 * HAL AUDIO Source Format Type.
 *
 */
typedef  enum
{
    HAL_AUDIO_SRC_TYPE_UNKNOWN     = 0,
    HAL_AUDIO_SRC_TYPE_PCM         = 1,
    HAL_AUDIO_SRC_TYPE_AC3         = 2,
    HAL_AUDIO_SRC_TYPE_EAC3        = 3,
    HAL_AUDIO_SRC_TYPE_MPEG        = 4,
    HAL_AUDIO_SRC_TYPE_AAC         = 5,
    HAL_AUDIO_SRC_TYPE_HEAAC       = 6,
    HAL_AUDIO_SRC_TYPE_DRA         = 7,
    HAL_AUDIO_SRC_TYPE_MP3         = 8,
    HAL_AUDIO_SRC_TYPE_DTS         = 9,
    HAL_AUDIO_SRC_TYPE_SIF         = 10,
    HAL_AUDIO_SRC_TYPE_SIF_BTSC    = 11,
    HAL_AUDIO_SRC_TYPE_SIF_A2      = 12,
    HAL_AUDIO_SRC_TYPE_DEFAULT     = 13,
    HAL_AUDIO_SRC_TYPE_NONE        = 14,
    HAL_AUDIO_SRC_TYPE_DTS_HD_MA   = 15,
    HAL_AUDIO_SRC_TYPE_DTS_EXPRESS = 16,
    HAL_AUDIO_SRC_TYPE_DTS_CD      = 17,
    HAL_AUDIO_SRC_TYPE_EAC3_ATMOS  = 18,
    HAL_AUDIO_SRC_TYPE_AC4         = 19,
    HAL_AUDIO_SRC_TYPE_AC4_ATMOS   = 20,
    HAL_AUDIO_SRC_TYPE_MPEG_H      = 21,
    HAL_AUDIO_SRC_TYPE_MAT         = 22,
    HAL_AUDIO_SRC_TYPE_MAT_ATMOS   = 23,
    HAL_AUDIO_SRC_TYPE_OPUS        = 24,
    HAL_AUDIO_SRC_TYPE_VOBIS       = 25,
    HAL_AUDIO_SRC_TYPE_TRUEHD      = 26,
    HAL_AUDIO_SRC_TYPE_FLAC        = 27,
    HAL_AUDIO_SRC_TYPE_RA          = 28,
    HAL_AUDIO_SRC_TYPE_DTS_X       = 29
} HAL_AUDIO_SRC_TYPE_T;

/**
 * HAL AUDIO Source Input Type.
 * TP, PCM, SPDIF
 *
 */
typedef  enum
{
	HAL_AUDIO_IN_PORT_NONE		=  0,
	HAL_AUDIO_IN_PORT_TP		=  1,	// From TPA Stream Input
	HAL_AUDIO_IN_PORT_SPDIF 	=  2,	// From SERIAL INTERFACE 0
	HAL_AUDIO_IN_PORT_SIF		=  3,	// From Analog Front End (SIF)
	HAL_AUDIO_IN_PORT_ADC		=  4,	// Fron ADC Input
	HAL_AUDIO_IN_PORT_HDMI		=  5,	// From HDMI
	HAL_AUDIO_IN_PORT_I2S		=  6,	// From I2S
	HAL_AUDIO_IN_PORT_SYSTEM	=  7,	// From System
	HAL_AUDIO_IN_PORT_DP		=  8,	// From DP
} HAL_AUDIO_IN_PORT_T;

/**
 * HAL AUDIO Dual-Mono Output Mode Type.
 *
 */
typedef enum
{
	HAL_AUDIO_DUALMONO_MODE_LR		= 0,
	HAL_AUDIO_DUALMONO_MODE_LL		= 1,
	HAL_AUDIO_DUALMONO_MODE_RR		= 2,
	HAL_AUDIO_DUALMONO_MODE_MIX		= 3,
} HAL_AUDIO_DUALMONO_MODE_T;

/**
 * HAL AUDIO SPDIF Type.
 *
 */
typedef  enum
{
	HAL_AUDIO_SPDIF_NONE			= 0,
	HAL_AUDIO_SPDIF_PCM				= 1,
	HAL_AUDIO_SPDIF_AUTO			= 2,
	HAL_AUDIO_SPDIF_AUTO_NODTS  	= 3,
	HAL_AUDIO_SPDIF_AUTO_AAC		= 4,
	HAL_AUDIO_SPDIF_AUTO_AAC_NODTS  = 5,
	HAL_AUDIO_SPDIF_FORCE_AC3       = 6,
	HAL_AUDIO_SPDIF_FORCE_AC3_DTS   = 7,
	HAL_AUDIO_SPDIF_BYPASS          = 8,
	HAL_AUDIO_SPDIF_BYPASS_NODTS    = 9,
	HAL_AUDIO_SPDIF_BYPASS_AAC      = 10,
	HAL_AUDIO_SPDIF_BYPASS_AAC_NODTS= 11,
} HAL_AUDIO_SPDIF_MODE_T;

/**
 * HAL AUDIO ARC Type.
 *
 */
typedef  enum
{
	HAL_AUDIO_ARC_NONE					= 0,
	HAL_AUDIO_ARC_PCM 					= 1,
	HAL_AUDIO_ARC_AUTO					= 2,
	HAL_AUDIO_ARC_AUTO_NODTS            = 3,
	HAL_AUDIO_ARC_AUTO_AAC				= 4,
	HAL_AUDIO_ARC_AUTO_AAC_NODTS        = 5,
	HAL_AUDIO_ARC_AUTO_EAC3				= 6,
	HAL_AUDIO_ARC_AUTO_EAC3_NODTS       = 7,
	HAL_AUDIO_ARC_AUTO_EAC3_AAC 		= 8,
	HAL_AUDIO_ARC_AUTO_EAC3_AAC_NODTS   = 9,
	HAL_AUDIO_ARC_AUTO_MAT              = 10,
	HAL_AUDIO_ARC_AUTO_MAT_NODTS        = 11,
	HAL_AUDIO_ARC_AUTO_MAT_AAC          = 12,
	HAL_AUDIO_ARC_AUTO_MAT_AAC_NODTS    = 13,
	HAL_AUDIO_ARC_FORCE_AC3         	= 14,
	HAL_AUDIO_ARC_FORCE_EAC3        	= 15,
	HAL_AUDIO_ARC_FORCE_AC3_DTS     	= 16,
	HAL_AUDIO_ARC_FORCE_EAC3_DTS    	= 17,
	HAL_AUDIO_ARC_FORCE_MAT     	    = 18,
	HAL_AUDIO_ARC_FORCE_MAT_DTS    	    = 19,
	HAL_AUDIO_ARC_BYPASS                = 20,
	HAL_AUDIO_ARC_BYPASS_NODTS          = 21,
	HAL_AUDIO_ARC_BYPASS_EAC3           = 22,
	HAL_AUDIO_ARC_BYPASS_EAC3_NODTS     = 23,
	HAL_AUDIO_ARC_BYPASS_EAC3_AAC       = 24,
	HAL_AUDIO_ARC_BYPASS_EAC3_AAC_NODTS = 25,
	HAL_AUDIO_ARC_BYPASS_AAC            = 26,
	HAL_AUDIO_ARC_BYPASS_AAC_NODTS      = 27,
	HAL_AUDIO_ARC_BYPASS_MAT            = 28,
	HAL_AUDIO_ARC_BYPASS_MAT_NODTS      = 29,
	HAL_AUDIO_ARC_BYPASS_MAT_AAC        = 30,
	HAL_AUDIO_ARC_BYPASS_MAT_AAC_NODTS  = 31,
	HAL_AUDIO_ARC_FORCE_MAT_NODTSHD     = 32,
	HAL_AUDIO_ARC_BYPASS_MAT_NODTSHD    = 33,
	HAL_AUDIO_ARC_BYPASS_MAT_AAC_NODTSHD = 34,
	HAL_AUDIO_ARC_AUTO_MAT_NODTSHD      = 35,
	HAL_AUDIO_ARC_AUTO_MAT_AAC_NODTSHD  = 36,
	HAL_AUDIO_ARC_AUTO_EAC3_DTSHD       = 37,
	HAL_AUDIO_ARC_AUTO_EAC3_AAC_DTSHD   = 38,
	HAL_AUDIO_ARC_FORCE_EAC3_DTSHD      = 39,
	HAL_AUDIO_ARC_BYPASS_EAC3_DTSHD     = 40,
	HAL_AUDIO_ARC_BYPASS_EAC3_AAC_DTSHD = 41,
} HAL_AUDIO_ARC_MODE_T;

/**
 * HAL AUDIO HDMI Format Type.
 *
 */
typedef enum
{
	HAL_AUDIO_HDMI_DVI			= 0,
	HAL_AUDIO_HDMI_NO_AUDIO		= 1,
	HAL_AUDIO_HDMI_PCM			= 2,
	HAL_AUDIO_HDMI_AC3			= 3,
	HAL_AUDIO_HDMI_DTS			= 4,
	HAL_AUDIO_HDMI_AAC			= 5,
	HAL_AUDIO_HDMI_DEFAULT		= 6,
	HAL_AUDIO_HDMI_MPEG			= 10,
	HAL_AUDIO_HDMI_DTS_HD_MA	= 11,
	HAL_AUDIO_HDMI_DTS_EXPRESS	= 12,
	HAL_AUDIO_HDMI_DTS_CD		= 13,
	HAL_AUDIO_HDMI_EAC3			= 14,
	HAL_AUDIO_HDMI_EAC3_ATMOS	= 15,
	HAL_AUDIO_HDMI_MAT		    = 16,
	HAL_AUDIO_HDMI_MAT_ATMOS    = 17,
} HAL_AUDIO_HDMI_TYPE_T;

/**
 * AUDIO Sampling Frequency Index.
 */
typedef enum
{
	HAL_AUDIO_SAMPLING_FREQ_NONE		=	     0,
	HAL_AUDIO_SAMPLING_FREQ_4_KHZ		=	  4000,
	HAL_AUDIO_SAMPLING_FREQ_8_KHZ		=	  8000,
	HAL_AUDIO_SAMPLING_FREQ_11_025KHZ	=	 11025,
	HAL_AUDIO_SAMPLING_FREQ_12_KHZ		=	 12000,
	HAL_AUDIO_SAMPLING_FREQ_16_KHZ		=	 16000,
	HAL_AUDIO_SAMPLING_FREQ_22_05KHZ	=	 22050,
	HAL_AUDIO_SAMPLING_FREQ_24_KHZ		=	 24000,
	HAL_AUDIO_SAMPLING_FREQ_32_KHZ		=	 32000,
	HAL_AUDIO_SAMPLING_FREQ_44_1KHZ		=	 44100,
	HAL_AUDIO_SAMPLING_FREQ_48_KHZ		=	 48000,
	HAL_AUDIO_SAMPLING_FREQ_64_KHZ		=	 64000,
	HAL_AUDIO_SAMPLING_FREQ_88_2KHZ		=	 88200,
	HAL_AUDIO_SAMPLING_FREQ_96_KHZ		=	 96000,
	HAL_AUDIO_SAMPLING_FREQ_128_KHZ		=	128000,
	HAL_AUDIO_SAMPLING_FREQ_176_4KHZ	=	176400,
	HAL_AUDIO_SAMPLING_FREQ_192_KHZ		=	192000,
	HAL_AUDIO_SAMPLING_FREQ_768_KHZ		=	768000,
	HAL_AUDIO_SAMPLING_FREQ_DEFAULT		=	999000,
} HAL_AUDIO_SAMPLING_FREQ_T;

/**
 * HAL AUDIO Country Type.
 *
 */
typedef  enum
{
	HAL_AUDIO_SIF_TYPE_NONE				= 0x0000,		///< INIT TYPE : NONE
	HAL_AUDIO_SIF_ATSC_SELECT			= 0x0001,		///< INIT TYPE : TV Systems for A2 enabled in default ATSC system
	HAL_AUDIO_SIF_KOREA_A2_SELECT		= 0x0002,		///< INIT TYPE : TV Systems for A2 enabled in Korea A2 system
	HAL_AUDIO_SIF_BTSC_SELECT			= 0x0004,		///< INIT TYPE : TV Systems for BTSC enabled in ATSC(CO, CF) or DVB(Taiwan) system
	HAL_AUDIO_SIF_BTSC_BR_SELECT		= 0x0008,		///< INIT TYPE : TV Systems for BTSC enabled in ATSC(Brazil) system
	HAL_AUDIO_SIF_BTSC_US_SELECT		= 0x0010,		///< INIT TYPE : TV Systems for BTSC enabled in ATSC(US) system
	HAL_AUDIO_SIF_DVB_SELECT 			= 0x0020,		///< INIT TYPE : TV Systems for EU in default DVB system
	HAL_AUDIO_SIF_DVB_ID_SELECT			= 0x0040,		///< INIT TYPE : TV Systems for ID(Indonesia) in DVB(PAL B/G) system
	HAL_AUDIO_SIF_DVB_IN_SELECT			= 0x0080,		///< INIT TYPE : TV Systems for IN(India) in DVB(PAL B) system
	HAL_AUDIO_SIF_DVB_CN_SELECT			= 0x0100,		///< INIT TYPE : TV Systems for CN(China, Hong Kone) in DVB system
	HAL_AUDIO_SIF_DVB_AJJA_SELECT		= 0x0200,		///< INIT TYPE : TV Systems for AJ(Asia JooDong), JA(JooAang Asia) in DVB system
	HAL_AUDIO_SIF_TYPE_MAX				= 0x0FFF,		///< INIT TYPE : MAX
	HAL_AUDIO_FULL_INIT                 = 0x8000,       ///< INIT TYPE : Use for RHAL initialize with full function
	HAL_AUDIO_LITE_INIT                 = 0x8001,       ///< INIT TYPE : Use for RHAL initialize with lite function
} HAL_AUDIO_SIF_TYPE_T;

/**
 * HAL AUDIO SIF Input Source Type.
 *
 */
typedef enum
{
	HAL_AUDIO_SIF_INPUT_EXTERNAL	= 0,
	HAL_AUDIO_SIF_INPUT_INTERNAL	= 1,
} HAL_AUDIO_SIF_INPUT_T;

/**
 * HAL AUDIO SIF FM Radio.
 *
 */
typedef enum{
	HAL_AUDIO_ATV_FM_RADIO_STOP = 0,
	HAL_AUDIO_ATV_FM_RADIO_SCAN,
	HAL_AUDIO_ATV_FM_RADIO_PLAY,
} HAL_AUDIO_ATV_FM_RADIO_MODE;

/**
 * HAL AUDIO SIF Sound System Type.
 *
 */
typedef enum
{
	HAL_AUDIO_SIF_SYSTEM_BG			= 0x00,
	HAL_AUDIO_SIF_SYSTEM_I			= 0x01,
	HAL_AUDIO_SIF_SYSTEM_DK			= 0x02,
	HAL_AUDIO_SIF_SYSTEM_L			= 0x03,
	HAL_AUDIO_SIF_SYSTEM_MN			= 0x04,
	HAL_AUDIO_SIF_SYSTEM_UNKNOWN	= 0xF0,
} HAL_AUDIO_SIF_SOUNDSYSTEM_T;

/**
 * HAL AUDIO SIF Sound Standard Mode Type.
 *
 */
typedef enum
{
	HAL_AUDIO_SIF_MODE_DETECT	= 0,
	HAL_AUDIO_SIF_BG_NICAM		= 1,
	HAL_AUDIO_SIF_BG_FM			= 2,
	HAL_AUDIO_SIF_BG_A2			= 3,
	HAL_AUDIO_SIF_I_NICAM		= 4,
	HAL_AUDIO_SIF_I_FM			= 5,
	HAL_AUDIO_SIF_DK_NICAM		= 6,
	HAL_AUDIO_SIF_DK_FM			= 7,
	HAL_AUDIO_SIF_DK1_A2		= 8,
	HAL_AUDIO_SIF_DK2_A2		= 9,
	HAL_AUDIO_SIF_DK3_A2		= 10,
	HAL_AUDIO_SIF_L_NICAM		= 11,
	HAL_AUDIO_SIF_L_AM			= 12,
	HAL_AUDIO_SIF_MN_A2			= 13,
	HAL_AUDIO_SIF_MN_BTSC		= 14,
	HAL_AUDIO_SIF_MN_EIAJ		= 15,
	HAL_AUDIO_SIF_MN_FM			= 16,
	HAL_AUDIO_SIF_NUM_SOUND_STD	= 17,
	HAL_AUDIO_SIF_UNKNOWN		= 18,
} HAL_AUDIO_SIF_STANDARD_T;

typedef enum
{
	/*		For TV001		*/
	HAL_SIF_OVER_DEVIATION_50K  = 0,
	HAL_SIF_OVER_DEVIATION_100K,
	HAL_SIF_OVER_DEVIATION_200K,
	HAL_SIF_OVER_DEVIATION_384K,
	HAL_SIF_OVER_DEVIATION_540K,
	/*		For TV002 and TV003		*/
	HAL_SIF_OVER_DEVIATION_150K,
	HAL_SIF_OVER_DEVIATION_462K,

	HAL_SIF_OVER_DEVIATION_BUTT
}HAL_SIF_OVER_DEVIATION_E;

/**
 * HAL AUDIO SIF Analog Audio Setting Parameter.
 *
 */
typedef enum
{
	HAL_AUDIO_SIF_SET_PAL_MONO					=	0x00,	// PAL Mono
	HAL_AUDIO_SIF_SET_PAL_MONO_FORCED			=	0x01,	// PAL Mono Force Mono
	HAL_AUDIO_SIF_SET_PAL_STEREO				=	0x02,	// PAL Stereo
	HAL_AUDIO_SIF_SET_PAL_STEREO_FORCED			=	0x03,	// PAL Stereo Force Mono
	HAL_AUDIO_SIF_SET_PAL_DUALI					=	0x04,	// PAL Dual I
	HAL_AUDIO_SIF_SET_PAL_DUALII				=	0x05,	// PAL Dual II
	HAL_AUDIO_SIF_SET_PAL_DUALI_II				=	0x06,	// PAL Dual I+II
	HAL_AUDIO_SIF_SET_PAL_NICAM_MONO			=	0x07,	// PAL NICAM Mono
	HAL_AUDIO_SIF_SET_PAL_NICAM_MONO_FORCED		=	0x08,	// PAL NICAM Mono Force Mono
	HAL_AUDIO_SIF_SET_PAL_NICAM_STEREO			=	0x09,	// PAL NICAM Stereo
	HAL_AUDIO_SIF_SET_PAL_NICAM_STEREO_FORCED	=	0x0A,	// PAL NICAM Stereo Force Mono
	HAL_AUDIO_SIF_SET_PAL_NICAM_DUALI			=	0x0B,	// PAL NICAM Dual I
	HAL_AUDIO_SIF_SET_PAL_NICAM_DUALII			=	0x0C,	// PAL NICAM Dual II
	HAL_AUDIO_SIF_SET_PAL_NICAM_DUALI_II		=	0x0D,	// PAL NICAM Dual I+II
	HAL_AUDIO_SIF_SET_PAL_NICAM_DUAL_FORCED		=	0x0E,	// PAL NICAM Dual Forced Mono(Not Supported)
	HAL_AUDIO_SIF_SET_PAL_UNKNOWN				=	0x0F,	// PAL Unkown State
	HAL_AUDIO_SIF_SET_NTSC_A2_MONO				=	0x10,	// NTSC(A2) Mono
	HAL_AUDIO_SIF_SET_NTSC_A2_STEREO			=	0x11,	// NTSC(A2) Stereo
	HAL_AUDIO_SIF_SET_NTSC_A2_SAP				=	0x12,	// NTSC(A2) SAP
	HAL_AUDIO_SIF_SET_NTSC_A2_UNKNOWN			=	0x13,	// NTSC(A2) Unkown State
	HAL_AUDIO_SIF_SET_NTSC_BTSC_MONO			=	0x14,	// NTSC(BTSC) Mono
	HAL_AUDIO_SIF_SET_NTSC_BTSC_STEREO			=	0x15,	// NTSC(BTSC) Stereo
	HAL_AUDIO_SIF_SET_NTSC_BTSC_SAP_MONO		=	0x16,	// NTSC(BTSC) SAP Mono
	HAL_AUDIO_SIF_SET_NTSC_BTSC_SAP_STEREO		=	0x17,	// NTSC(BTSC) SAP Stereo
	HAL_AUDIO_SIF_SET_NTSC_BTSC_UNKNOWN			=	0x18,	// NTSC(BTSC) Unkown State
} HAL_AUDIO_SIF_MODE_SET_T;

/**
 * HAL AUDIO SIF Standard Type.
 *
 */
typedef enum
{
	HAL_AUDIO_SIF_NICAM					= 0,
	HAL_AUDIO_SIF_A2					= 1,
	HAL_AUDIO_SIF_FM					= 2,
	HAL_AUDIO_SIF_DETECTING_AVALIBILITY = 3,
} HAL_AUDIO_SIF_AVAILE_STANDARD_T;

/**
 * HAL AUDIO SIF Exist Type.
 *
 */
typedef enum
{
	HAL_AUDIO_SIF_ABSENT	 			= 0,
	HAL_AUDIO_SIF_PRESENT 		 		= 1,
	HAL_AUDIO_SIF_DETECTING_EXSISTANCE	= 2,
} HAL_AUDIO_SIF_EXISTENCE_INFO_T;

/**
 * HAL AUDIO SIF Analog Audio Getting Parameter.
 *
 */
typedef enum
{
	HAL_AUDIO_SIF_GET_PAL_MONO				=	0x00,	// PAL Mono
	HAL_AUDIO_SIF_GET_PAL_STEREO			=	0x01,	// PAL Stereo
	HAL_AUDIO_SIF_GET_PAL_DUAL				=	0x02,	// PAL Dual
	HAL_AUDIO_SIF_GET_PAL_NICAM_MONO		=	0x03,	// PAL NICAM Mono
	HAL_AUDIO_SIF_GET_PAL_NICAM_STEREO		=	0x04,	// PAL NICAM Stereo
	HAL_AUDIO_SIF_GET_PAL_NICAM_DUAL		=	0x05,	// PAL NICAM Dual
	HAL_AUDIO_SIF_GET_PAL_UNKNOWN			=	0x06,	// PAL Unkown State
	HAL_AUDIO_SIF_GET_NTSC_A2_MONO			=	0x10,	// NTSC(A2) Mono
	HAL_AUDIO_SIF_GET_NTSC_A2_STEREO		=	0x11,	// NTSC(A2) Stereo
	HAL_AUDIO_SIF_GET_NTSC_A2_SAP			=	0x12,	// NTSC(A2) SAP
	HAL_AUDIO_SIF_GET_NTSC_A2_UNKNOWN		=	0x13,	// NTSC(A2) Unkown State
	HAL_AUDIO_SIF_GET_NTSC_BTSC_MONO		=	0x14,	// NTSC(BTSC) Mono
	HAL_AUDIO_SIF_GET_NTSC_BTSC_STEREO		=	0x15,	// NTSC(BTSC) Stereo
	HAL_AUDIO_SIF_GET_NTSC_BTSC_SAP_MONO	=	0x16,	// NTSC(BTSC) SAP Mono
	HAL_AUDIO_SIF_GET_NTSC_BTSC_SAP_STEREO	=	0x17,	// NTSC(BTSC) SAP Stereo
	HAL_AUDIO_SIF_GET_NTSC_BTSC_UNKNOWN		=	0x18,	// NTSC(BTSC) Unkown State
} HAL_AUDIO_SIF_MODE_GET_T;

/**
 * HAL AUDIO Copy Protection Type
 *
 */
typedef  enum
{
	HAL_AUDIO_SPDIF_COPY_FREE		= 0,	/* cp-bit : 1, L-bit : 0 */
	HAL_AUDIO_SPDIF_COPY_NO_MORE	= 1,	/* cp-bit : 0, L-bit : 1 */
	HAL_AUDIO_SPDIF_COPY_ONCE		= 2,	/* cp-bit : 0, L-bit : 0 */
	HAL_AUDIO_SPDIF_COPY_NEVER		= 3,	/* cp-bit : 0, L-bit : 1 */
} HAL_AUDIO_SPDIF_COPYRIGHT_T;

/**
 * HAL AUDIO Volume Structure
 * dB Scale		:     main,	fine
 *   -127dB Min 	: 0x00(-127dB)	 0x00(0 dB)
 * - 125.9325 dB 	: 0x01(-126 dB), 0x01(1/16 dB)
 * - 121.8125 dB 	: 0x05(-122 dB), 0x03(3/16 dB)
 * -0.9375dB        : 0x7E(-1dB),    0x01(1/16dB)
 *    0 dB          : 0x7F(0 dB),    0x00(0 dB)
 *    30dB          : 0x9D(30dB),    0x00(0 dB)
 */
typedef struct HAL_AUDIO_VOLUME
{
	UINT8	mainVol;	// 1 dB step, -127 ~ +30 dB.
	UINT8	fineVol;  	// 1/16 dB step, 0dB ~ 15/16dB
} HAL_AUDIO_VOLUME_T ;

/**
 * HAL AUDIO DOLBY DRC Mode.
 *
 * @see
*/
typedef  enum
{
	HAL_AUDIO_DOLBY_LINE_MODE	= 0,
	HAL_AUDIO_DOLBY_RF_MODE		= 1,
	HAL_AUDIO_DOLBY_DRC_OFF		= 2,
} HAL_AUDIO_DOLBY_DRC_MODE_T;

/**
 * HAL AUDIO DOLBY Downmix Mode.
 *
*/
typedef  enum
{
	HAL_AUDIO_LORO_MODE		= 0,
	HAL_AUDIO_LTRT_MODE		= 1,
} HAL_AUDIO_DOWNMIX_MODE_T;

/**
 * HAL AUDIO Channel Mode.
 *
 * @see
*/
typedef enum HAL_AUDIO_MODE
{
	HAL_AUDIO_MODE_MONO	 			= 0,
	HAL_AUDIO_MODE_JOINT_STEREO 	= 1,
	HAL_AUDIO_MODE_STEREO		 	= 2,
	HAL_AUDIO_MODE_DUAL_MONO 		= 3,
	HAL_AUDIO_MODE_MULTI			= 4,
	HAL_AUDIO_MODE_UNKNOWN			= 5,
} HAL_AUDIO_MODE_T;

/**
 * HAL AUDIO TP Channel Mode.
 *
 * @see
*/
typedef enum HAL_AUDIO_CHANNEL_MODE
{
    HAL_AUDIO_CH_MODE_MONO                   = 0,
    HAL_AUDIO_CH_MODE_JOINT_STEREO           = 1,
    HAL_AUDIO_CH_MODE_STEREO                 = 2,
    HAL_AUDIO_CH_MODE_DUAL_MONO              = 3,
    HAL_AUDIO_CH_MODE_MULTI                  = 4,
    HAL_AUDIO_CH_MODE_UNKNOWN                = 5,
    HAL_AUDIO_CH_MODE_2_1_FL_FR_LFE          = 6,
    HAL_AUDIO_CH_MODE_3_0_FL_FR_RC           = 7,
    HAL_AUDIO_CH_MODE_3_1_FL_FR_RC_LFE       = 8,
    HAL_AUDIO_CH_MODE_4_0_FL_FR_RL_RR        = 9,
    HAL_AUDIO_CH_MODE_4_1_FL_FR_RL_RR_LFE    = 10,
    HAL_AUDIO_CH_MODE_5_0_FL_FR_FC_RL_RR     = 11,
    HAL_AUDIO_CH_MODE_5_1_FL_FR_FC_RL_RR_LFE = 12,
} HAL_AUDIO_CHANNEL_MODE_T;


/**
 * HAL AUDIO Trick Mode Type.
 *
 */
typedef  enum
{
	HAL_AUDIO_TRICK_NONE				= 0, 	///<  rate : None, TP Live Play
	HAL_AUDIO_TRICK_PAUSE				= 1, 	///<  rate : Pause, DVR Play
	HAL_AUDIO_TRICK_NORMAL_PLAY			= 2, 	///<  rate : Normal Play, DVR Play
	HAL_AUDIO_TRICK_SLOW_MOTION_0P25X 	= 3,	///<  rate : 0.25 Play
	HAL_AUDIO_TRICK_SLOW_MOTION_0P50X 	= 4,	///<  rate : 0.50 Play
	HAL_AUDIO_TRICK_SLOW_MOTION_0P80X 	= 5,	///<  rate : 0.80 Play
	HAL_AUDIO_TRICK_FAST_FORWARD_1P20X 	= 6, 	///<  rate : 1.20 Play
	HAL_AUDIO_TRICK_FAST_FORWARD_1P50X 	= 7, 	///<  rate : 1.50 Play
	HAL_AUDIO_TRICK_FAST_FORWARD_2P00X	= 8,	///<  rate : 2.00 Play
	HAL_AUDIO_TRICK_ONE_FRAME_DECODE	= 9,    ///<  rate : one frame decode
} HAL_AUDIO_TRICK_MODE_T;


/**
 * HAL AUDIO Encoder Index.
 *
 */
typedef  enum
{
	HAL_AUDIO_AENC0		= 0,
	HAL_AUDIO_AENC1		= 1,
	HAL_AUDIO_AENC_MAX	= HAL_AUDIO_AENC1,
} HAL_AUDIO_AENC_INDEX_T ;

/**
 * HAL AUDIO AENC encode format
 */
typedef enum
{
	HAL_AUDIO_AENC_ENCODE_MP3 	= 0,	/* Encode MP3 format */
	HAL_AUDIO_AENC_ENCODE_AAC	= 1,	/* Encode AAC format */
} HAL_AUDIO_AENC_ENCODING_FORMAT_T;

/**
 * HAL AUDIO AENC encode Status
 */
typedef enum
{
    HAL_AUDIO_AENC_STATUS_STOP              = 0,
    HAL_AUDIO_AENC_STATUS_PLAY              = 1,
    HAL_AUDIO_AENC_STATUS_ABNORMAL  = 2,
} HAL_AUDIO_AENC_STATUS_T;

/**
 * HAL AUDIO AENC encode # of channel
 */
typedef enum
{
    HAL_AUDIO_AENC_MONO     = 0,
    HAL_AUDIO_AENC_STEREO   = 1,
} HAL_AUDIO_AENC_CHANNEL_T;

/**
 * HAL AUDIO AENC encode bitrate
 */
typedef enum
{
    HAL_AUDIO_AENC_BIT_48K          = 0,
    HAL_AUDIO_AENC_BIT_56K          = 1,
    HAL_AUDIO_AENC_BIT_64K          = 2,
    HAL_AUDIO_AENC_BIT_80K          = 3,
    HAL_AUDIO_AENC_BIT_112K         = 4,
    HAL_AUDIO_AENC_BIT_128K         = 5,
    HAL_AUDIO_AENC_BIT_160K         = 6,
    HAL_AUDIO_AENC_BIT_192K         = 7,
    HAL_AUDIO_AENC_BIT_224K         = 8,
    HAL_AUDIO_AENC_BIT_256K         = 9,
    HAL_AUDIO_AENC_BIT_320K         = 10,
} HAL_AUDIO_AENC_BITRATE_T;


/**
* HAL AUDIO Language Code Type.
*
*/
typedef  enum
{
    HAL_AUDIO_LANG_CODE_ISO639_1    = 0,    /* 2bytes example : 'e''n'00   */
    HAL_AUDIO_LANG_CODE_ISO639_2    = 1,    /* 3bytes example : 'e''n''g'0 */
} HAL_AUDIO_LANG_CODE_TYPE_T;

/**
* HAL AUDIO AC4 AD Type.
*
*/
typedef enum HAL_AUDIO_AC4_AD_TYPE
{
    HAL_AUDIO_AC4_AD_TYPE_NONE  = 0,    /* None */
    HAL_AUDIO_AC4_AD_TYPE_VI    = 1,    /* Visually Impaired (VI) - Default */
    HAL_AUDIO_AC4_AD_TYPE_HI    = 2,    /* Hearing Impaired (HI) */
    HAL_AUDIO_AC4_AD_TYPE_C     = 3,    /* Commentary (C) */
    HAL_AUDIO_AC4_AD_TYPE_E     = 4,    /* Emergency (E) */
    HAL_AUDIO_AC4_AD_TYPE_VO    = 5,    /* Voice Over (VO) */
} HAL_AUDIO_AC4_AD_TYPE_T;


/**
 * HAL AUDIO PCM Input Mode.
 *
 */
typedef  enum
{
	HAL_AUDIO_PCM_SB_PCM		= 0,	/* Sound Bar Output : No SE Ouput */
	HAL_AUDIO_PCM_SB_CANVAS		= 1,	/* Sound Bar Output : SE Ouput */
	HAL_AUDIO_PCM_I2S			= 2,	/* I2S(L/R) Input for AEC(Audio Echo Cancellation) */
	HAL_AUDIO_PCM_INPUT_MAX		= 3,	/* PCM Input Max */
} HAL_AUDIO_PCM_INPUT_T;

/**
 * HAL AUDIO Input/Output I2S Port.
 *
 */
typedef  enum
{
	HAL_AUDIO_INOUT_I2S0		= 0x01,	/* I2S0(L/R) Input/Output for AEC(Audio Echo Cancellation) */
	HAL_AUDIO_INOUT_I2S1		= 0x02, /* I2S1(Lh/Rh) Input/Output for AEC(Audio Echo Cancellation) */
	HAL_AUDIO_INOUT_I2S2		= 0x04, /* I2S2(Lr/Rr) Input/Output for AEC(Audio Echo Cancellation) */
	HAL_AUDIO_INOUT_I2S3		= 0x08, /* I2S3(C/Lf) Input/Output for AEC(Audio Echo Cancellation) */
} HAL_AUDIO_INOUT_I2S_T;

typedef enum {
	HAL_ATV_MAIN_STD_MONO		= 0,
	HAL_ATV_MAIN_STD_NICAM		= 1,
	HAL_ATV_MAIN_STD_A2			= 2,
	HAL_ATV_MAIN_STD_BTSC		= 3,
	HAL_ATV_MAIN_STD_UNKNOW	= 4,
} HAL_ATV_MAIN_STD_TYPE_T;

typedef enum {
	HAL_ATV_SOUND_SELECT_MONO		= 0,
	HAL_ATV_SOUND_SELECT_STEREO		= 1,
	HAL_ATV_SOUND_SELECT_LANGA		= 2,
	HAL_ATV_SOUND_SELECT_LANGB		= 3,
	HAL_ATV_SOUND_SELECT_LANGAB		= 4,
	HAL_ATV_SOUND_SELECT_SAP		= 5,
} HAL_ATV_SOUND_SELECT;

typedef enum {
	HAL_ATV_SOUND_MODE_MONO		= 0,
	HAL_ATV_SOUND_MODE_STEREO		= 1,
	HAL_ATV_SOUND_MODE_DUAL		= 2,
	HAL_ATV_SOUND_MODE_SAP_MONO	= 3,
	HAL_ATV_SOUND_MODE_SAP_STEREO	= 4,
	HAL_ATV_SOUND_MODE_AUTO		= 5,
} HAL_ATV_SOUND_MODE;

typedef struct HAL_AUDIO_AC3_ES_INFO{
    UINT8 bitRate;
    UINT8 sampleRate;
    UINT8 channelNum;
    UINT8 EAC3;       	/* AC3 0x0, EAC3 0x1*/
} HAL_AUDIO_AC3_ES_INFO_T;

/**
 * HAL AUDIO MPEG ES Info
 *
 * @see
*/
typedef struct HAL_AUDIO_MPEG_ES_INFO{
    UINT8 bitRate;
    UINT8 sampleRate;
    UINT8 layer;
    UINT8 channelNum;
} HAL_AUDIO_MPEG_ES_INFO_T;

/**
 * HAL AUDIO HE-AAC ES Info
 *
 * @see
*/
typedef struct HAL_AUDIO_HEAAC_ES_INFO{
    UINT8 version;    		  	/* AAC = 0x0, HE-AACv1 = 0x1, HE-AACv2 = 0x2 */
    UINT8 transmissionformat;   /* LOAS/LATM = 0x0, ADTS = 0x1*/
    UINT8 channelNum;
} HAL_AUDIO_HEAAC_ES_INFO_T;


/**
 * HAL AUDIO ES Info
 *
 * @see
*/
typedef struct HAL_AUDIO_ES_INFO{
    HAL_AUDIO_SRC_TYPE_T 		adecFormat;
    HAL_AUDIO_MODE_T 			audioMode;
    HAL_AUDIO_HEAAC_ES_INFO_T 	heAAcEsInfo;
    HAL_AUDIO_MPEG_ES_INFO_T  	mpegESInfo;
    HAL_AUDIO_AC3_ES_INFO_T   	ac3ESInfo;
} HAL_AUDIO_ES_INFO_T;

/**
 * HAL AUDIO SIF Sound Information.
 *
 */
typedef struct {
	UINT8	isNicamSystem; // is current  std nicam system
	UINT8	isPlayDigital; // is current setting playing nicam digital part or analog part
	HAL_ATV_SOUND_SELECT		current_select; // current select
	HAL_AUDIO_SIF_STANDARD_T	sound_std;
	HAL_ATV_SOUND_MODE		ana_soundmode;
	HAL_ATV_SOUND_MODE		dig_soundmode;
	HAL_ATV_MAIN_STD_TYPE_T	std_type; // mono or btsc or a2 or nicam
} HAL_ATV_SOUND_INFO_T;

/**
* HAL AUDIO SIF FM RADIO Info
*
*/
typedef struct {
	UINT8 CARRIER_SHIFT_VALUE;
	UINT8 CARRIER_DEVIATION;
} HAL_AUDIO_SIF_CARRIER_INFO_T;

/**
 * HAL AUDIO AC3 Info
 *
 * @see
*/
typedef struct HAL_AUDIO_DIGITAL_INFO {
    HAL_AUDIO_SRC_TYPE_T 		adecFormat;
    HAL_AUDIO_MODE_T 			audioMode;
    UINT16 sampleRate;
    UINT8 EAC3;       	/* AC3 0x0, EAC3 0x1*/
} HAL_AUDIO_DIGITAL_INFO_T;

typedef struct HAL_AUDIO_AENC_INFO
{
    //Get Info for debugging on DDI
    HAL_AUDIO_AENC_STATUS_T           status;  // current ENC Status
    HAL_AUDIO_AENC_ENCODING_FORMAT_T  codec;   // current ENC Codec

    UINT32  errorCount;                        // current ENC error counter
    UINT32  inputCount;                        // current ENC input counter - we distinguish whether or not enter data from input.
    UINT32  underflowCount;                    // current ENC underflowCnt in kernel space - we distinguish which module have problem between muxer and encdoer
    UINT32  overflowCount;                     // current ENC overflowCnt - we distinguish

    //Set Info - it is applied realtime, no matter stop&start
    HAL_AUDIO_AENC_CHANNEL_T          channel; // number of channel
    HAL_AUDIO_AENC_BITRATE_T          bitrate; // bitrate
} HAL_AUDIO_AENC_INFO_T;


/**
 *  HAL AUDIO PCM DATA TYPE
 *
 */
typedef struct HAL_AUDIO_PCM_DATA
{
	UINT8	index;		// PCM index
	UINT32	pts;		// PTS(unit : 90Khz clock base, max value : 0xFFFFFFFF)
	UINT8	*pData;		// pointer to Audio Data
	UINT32	dataLen;	// Audio Data Length
} HAL_AUDIO_PCM_DATA_T;

/**
 * HAL AUDIO PCM Capture Info
 * The PCM other parameter is pre-defined as follows.(little endian, singed)
 *
*/
typedef struct HAL_AUDIO_PCM_INFO{
	HAL_AUDIO_SAMPLING_FREQ_T	samplingFreq;		/* 48000 : 48Khz, 44100 : 44.1Khz */
	UINT32 						numOfChannel;		/* 2  : stereo, 1 : mono */
	UINT32 						bitPerSample;		/* 16 : 16 bit, 8 : 8 bit */
	HAL_AUDIO_INOUT_I2S_T		inOutI2SPort;		/* I2S Input/Output Port Bit Mask.(I2S0 & I2S1 => 0x03) */
} HAL_AUDIO_PCM_INFO_T;

typedef struct HAL_AUDIO_AENC_DATA
{
       UINT32  index;          // Encoder index
       UINT64  pts;            // PTS
       UINT8   *pData;         // pointer to Audio Data
       UINT32  dataLen;        // Audio Data Length
       UINT8   *pRStart;       // start pointer of buffer
       UINT8   *pREnd;         // end pointer of buffer
}HAL_AUDIO_AENC_DATA_T;

/**
 *  HAL AUDIO CALLBACK FUNCTION
 *
 */
typedef DTV_STATUS_T (*pfnAENCDataHandling)(HAL_AUDIO_AENC_DATA_T *pMsg);
typedef DTV_STATUS_T (*pfnPCMDataHandling)(HAL_AUDIO_PCM_DATA_T *pMsg);
typedef DTV_STATUS_T (*pfnPCMSending)(UINT8 *pBuf, UINT16 length);

typedef void (*pfnHdmiFmtNotify)(void* pAudioType);

/**
 * HAL AUDIO Decoder Debug Information Definition
 *
 */
typedef struct HAL_AUDIO_ADEC_INFO
{
	HAL_AUDIO_SRC_TYPE_T   				curAdecFormat;				/* Current ADEC Format Type */
	HAL_AUDIO_SRC_TYPE_T   				prevAdecFormat;				/* Previous ADEC Format Type */
	HAL_AUDIO_IN_PORT_T					curAdecInputPort;			/* Current ADEC Input Type */
	HAL_AUDIO_IN_PORT_T					prevAdecInputPort;			/* Previous ADEC Input Type */
	UINT8								curADCPortNum;				/* Current ADC Port Number */
	UINT8								prevADCPortNum;				/* Previous ADC Port Number */
	HAL_AUDIO_HDMI_INDEX_T				curHDMIPortNum;				/* Current HDMI Port Number */
	HAL_AUDIO_HDMI_INDEX_T				prevHDMIPortNum;			/* Previous HDMI Port Number */

	HAL_AUDIO_RESOURCE_T				curTPInputPort;				/* Current ADEC TP Input Port(HAL_AUDIO_RESOURCE_SDEC0/1) */
	HAL_AUDIO_RESOURCE_T				prevTPInputPort;			/* Previous ADEC TP Input Port(HAL_AUDIO_RESOURCE_SDEC0/1) */
	HAL_AUDIO_RESOURCE_T				curAENCInput;				/* Current ADEC to AENC Input Port(HAL_AUDIO_RESOURCE_AENC0/1) */
	HAL_AUDIO_RESOURCE_T				prevAENCInput;				/* Previous ADEC to AENC Input Port(HAL_AUDIO_RESOURCE_AENC0/1) */

	BOOLEAN								bAdecStart;					/* Current ADEC Start Status */
	BOOLEAN								bAdecESExist;				/* Current ADEC ES Exist Status */
	HAL_AUDIO_DUALMONO_MODE_T			curAdecDualmonoMode;		/* Current ADEC Dual Mono Mode Status */

	HAL_AUDIO_SIF_INPUT_T 				sifSource;					/* Currnet SIF Source Input Status */
	HAL_AUDIO_SIF_TYPE_T 				curSifType;					/* Currnet SIF Type Status */
	BOOLEAN								bHighDevOnOff;				/* Currnet High DEV ON/OFF Status */
	HAL_AUDIO_SIF_SOUNDSYSTEM_T			curSifBand;					/* Currnet SIF Sound Band(Sound System) */
	HAL_AUDIO_SIF_STANDARD_T			curSifStandard;				/* Currnet SIF Sound Standard */
	HAL_AUDIO_SIF_EXISTENCE_INFO_T		curSifIsA2;					/* Currnet SIF A2 Exist Status */
	HAL_AUDIO_SIF_EXISTENCE_INFO_T		curSifIsNicam;				/* Currnet SIF NICAM Exist Status */
	HAL_AUDIO_SIF_MODE_SET_T 			curSifModeSet;				/* Currnet SIF Sound Mode Set Status */
	HAL_AUDIO_SIF_MODE_GET_T 			curSifModeGet;				/* Currnet SIF Sound Mode Get Status */

	HAL_AUDIO_HDMI_TYPE_T  				curHdmiAudioType;			/* Current HDMI Audio Format Type */
	HAL_AUDIO_SAMPLING_FREQ_T			curHdmiSamplingFreq;		/* Current HDMI PCM Sampling Frequency */
	HAL_AUDIO_SPDIF_COPYRIGHT_T			curHdmiCopyInfo;			/* HDMI Input Copy Info.(Cbit info 16bit : to set FREE/ONCE/NEVER for SPDIF Ouput) */
} HAL_AUDIO_ADEC_INFO_T;

/**
 * HAL AUDIO HAL Driver Information Definition
 *
 */
typedef struct HAL_AUDIO_COMMON_INFO
{
	HAL_AUDIO_SPDIF_MODE_T 		curAudioSpdifMode;			/* Current Audio SPDIF Output Mode. */
	BOOLEAN						bAudioSpdifOutPCM;			/* Current Audio SPDIF Output is PCM. */
	BOOLEAN						bCurAudioSpdifOutPCM;		/* Current UI Setting SPDIF Output is PCM. */

	HAL_AUDIO_SPDIF_COPYRIGHT_T	curSpdifCopyInfo;			/* SPDIF Output SCMS(Serail Copy Management System) => Copy Info. : FREE/ONCE/NEVER */
	UINT8						curSpdifCategoryCode;		/* Current Audio SPDIF Category Code */

	UINT32						curSPKOutDelay;
	UINT32						curSPDIFOutDelay;
	UINT32						curHPOutDelay;
	UINT32						curSCARTOutDelay;

	HAL_AUDIO_VOLUME_T			curSPKOutVolume;
	HAL_AUDIO_VOLUME_T			curSPDIFOutVolume;
	HAL_AUDIO_VOLUME_T			curHPOutVolume;
	HAL_AUDIO_VOLUME_T			curSCARTOutVolume;

	BOOLEAN						curSPKMuteStatus;
	BOOLEAN						curSPDIFMuteStatus;
	BOOLEAN						curHPMuteStatus;
	BOOLEAN						curSCARTMuteStatus;
} HAL_AUDIO_COMMON_INFO_T;



// according to  layout / TV006 setting
typedef enum _HAL_AUDIO_BBADC_SRC
{
    AUDIO_BBADC_SRC_AIN1_PORT_NUM = 0,
    AUDIO_BBADC_SRC_AIN2_PORT_NUM = 1,
    AUDIO_BBADC_SRC_AIN3_PORT_NUM = 2,
    AUDIO_BBADC_SRC_MIC1_PORT_NUM = 3,
    AUDIO_BBADC_SRC_MIC2_PORT_NUM = 4,
    AUDIO_BBADC_SRC_AIO_PORT_NUM  = 5,
    AUDIO_BBADC_SRC_AIO2_PORT_NUM = 6
} HAL_AUDIO_BBADC_SRC;

typedef enum
{
	HAL_AUDIO_DBX_PARAM_TYPE_ONOFF,
	HAL_AUDIO_DBX_PARAM_TYPE_TOTAL_SONICS,
	HAL_AUDIO_DBX_PARAM_TYPE_TOTAL_SURROUND,
	HAL_AUDIO_DBX_PARAM_TYPE_TOTAL_VOLUME,
} HAL_AUDIO_DBXTV_PARAM_TYPE;

typedef	struct {
	SINT32		ModeType;
	SINT32		TruBassGain;
	SINT32		TruBassSize;
	SINT32		FocusGain;
	SINT32		InputGain;
	SINT32		IsTSXTEnable;
	SINT32		IsHeadphoneMode;
	SINT32		IsTruBassEnable;
	SINT32		IsFocusEnable;
	SINT32		DefinitionGain;		// for HD upgrade version only
	SINT32		IsDefEnable;		// for HD upgrade version only
	SINT32      Volume_compensate;
} HAL_APP_TSXT_CFG;

/************ BASSBACK *************/
typedef struct {
    SINT32 Enable_Sys;
    SINT32 StopBandFreq_Sys;
    SINT32 HarmonicGain;
    SINT32 vb_enable;
    SINT32 Sys_EnhanceFreq;
    SINT32 Tru_StopbandFreq;
    SINT32 Set_BassGain;
} HAL_AUDIO_BASSBACK_TV_MODE_T;

typedef struct {
    SINT32 Enable_Sys;
    SINT32 StopBandFreq_Sys;
    SINT32 HarmonicGain;
    SINT32 SubHarmonicGain;
    SINT32 TreHarmonicGain;
    SINT32 vb_enable;
    SINT32 Sys_EnhanceFreq;
    SINT32 Tru_StopbandFreq;
    SINT32 Set_BassGain;
    SINT32 Enable_HPF;
    SINT32 StopBandFreq_HPF;
    SINT32 Enable_PF;
    SINT32 Gain_PF;
    SINT32 CenterFreq_PF;
    SINT32 BandWidth_PF;
} HAL_AUDIO_BASSBACK_PCBU_MODE_T;

typedef enum
{
    HAL_AUDIO_MIC_IN = 0,
    HAL_AUDIO_OTHER_IN,
    HAL_AUDIO_INPUT_MAX
} HAL_AUDIO_INPUT_SOURCE_T;
/************ BASSBACK *************/

/************ TruVolume *************/
typedef struct {
    SINT32 enable;       /* 1 for TRUE, 0 for FALSE  */
    SINT32 InputGain;    /* 0.0-32.0 for inputGain range, default is 1.0  */
    SINT32 OutputGain;   /* 0.0-32.0 for outputGain range, default is 0.5  */
    SINT32 BypassGain;   /* 0.0~1.0  for BypassGain, default is 1.0 */
    SINT32 mode;         /* 0 for light mode, 1 for normal mode, 2 for heavy mode  */
    SINT32 MaxGain;      /* 0.25~256 for volume Max gain, default is 16 */
    SINT32 NormalizerEnable; /* 1 for TRUE, 0 for FALSE  */
    SINT32 BlockSize; /* 160~464 for Volume Block Size, default is 256*/
} HAL_AUDIO_TRUVOLUME_MODE_T;

typedef enum {
	HAL_AUDIO_AO_CHANNEL_OUT_STEREO = 0x0,
	HAL_AUDIO_AO_CHANNEL_OUT_L_TO_R = 0x1,
	HAL_AUDIO_AO_CHANNEL_OUT_R_TO_L = 0x2,
	HAL_AUDIO_AO_CHANNEL_OUT_LR_SWAP = 0x3,
	HAL_AUDIO_AO_CHANNEL_OUT_LR_MIXED = 0x4,
} HAL_TRACK_MODE;

typedef struct {
	SLONG HDMI_VSDB_delay;
} HAL_AUDIO_HDMI_OUT_VSDB_DATA_T;

typedef struct {
	SINT32 chnum;
	SINT32 samplebit;
	SINT32 samplerate;
	UINT32 dynamicRange;
	UINT8 emphasis;
	UINT8 mute;
} HAL_AUDIO_PCM_FORMAT_T;

typedef struct {
	SLONG instanceID;
	HAL_AUDIO_PCM_FORMAT_T pcmFormat;
	SLONG endianType;
	SLONG pBitstreamRB;
	SLONG iteration;
} HAL_AUDIO_SOUND_EVENT_T;

typedef enum {
	HAL_PRIVATEINFO_AUDIO_FORMAT_PARSER_CAPABILITY = 0,
	HAL_PRIVATEINFO_AUDIO_DECODER_CAPABILITY = 1,
	HAL_PRIVATEINFO_AUDIO_CONFIG_CMD_BS_INFO = 2,
	HAL_PRIVATEINFO_AUDIO_CHECK_LPCM_ENDIANESS = 3,
	HAL_PRIVATEINFO_AUDIO_CONFIG_CMD_AO_DELAY_INFO = 4,
	HAL_PRIVATEINFO_AUDIO_AO_CHANNEL_VOLUME_LEVEL = 5,
	HAL_PRIVATEINFO_AUDIO_GET_FLASH_PIN = 6,
	HAL_PRIVATEINFO_AUDIO_RELEASE_FLASH_PIN = 7,
	HAL_PRIVATEINFO_AUDIO_GET_MUTE_N_VOLUME = 8,
	HAL_PRIVATEINFO_AUDIO_AO_MONITOR_FULLNESS = 9,
	HAL_PRIVATEINFO_AUDIO_CONTROL_FLASH_VOLUME = 10,
	HAL_PRIVATEINFO_AUDIO_CONTROL_DAC_SWITCH = 11,
	HAL_PRIVATEINFO_AUDIO_CONTROL_AO_GIVE_UP_SET_CLOCK = 12,
	HAL_PRIVATEINFO_AUDIO_DEC_DELAY_RP = 13,
	HAL_PRIVATEINFO_AUDIO_AI_SET_OUT_FMT = 14,
	HAL_PRIVATEINFO_AUDIO_AI_SWITCH_FOCUS = 15,
	HAL_PRIVATEINFO_AUDIO_SUPPORT_SAMPLERATE = 16,
	HAL_PRIVATEINFO_AUDIO_GET_AVAILABLE_AO_OUTPUT_PIN = 17,
	HAL_PRIVATEINFO_AUDIO_GET_PCM_IN_PIN = 18,
	HAL_PRIVATEINFO_AUDIO_RELEASE_PCM_IN_PIN = 19,
	HAL_PRIVATEINFO_AUDIO_FIRMWARE_CAPABILITY = 20,
	HAL_PRIVATEINFO_AUDIO_SRC_PROCESS_DONE = 21,
	HAL_PRIVATEINFO_AUDIO_DEC_SRC_ENABLE = 22,
	HAL_PRIVATEINFO_AUDIO_QUERY_FLASH_PIN = 23,
	HAL_PRIVATEINFO_AUDIO_SET_FLASH_PIN = 24,
	HAL_PRIVATEINFO_AUDIO_SET_SYSTEM_PROCESS_PID = 25,
	HAL_PRIVATEINFO_AUDIO_CONFIG_VIRTUALX_PARAM = 26,
	HAL_PRIVATEINFO_AUDIO_GET_CURR_AO_VOLUME = 27,
} HAL_AUDIO_ENUM_PRIVAETINFO;

typedef struct {
	SLONG instanceID;
	HAL_AUDIO_ENUM_PRIVAETINFO type;
	SLONG privateInfo[16];
} HAL_AUDIO_RPC_PRIVATEINFO_PARAMETERS_T;

typedef struct {
	SLONG instanceID;
	SLONG privateInfo[16];
} HAL_AUDIO_RPC_PRIVATEINFO_RETURNVAL_T;

typedef enum {
	HAL_AUDIO_CONFIG_CMD_SPEAKER = 0,
	HAL_AUDIO_CONFIG_CMD_AGC = 1,
	HAL_AUDIO_CONFIG_CMD_SPDIF = 2,
	HAL_AUDIO_CONFIG_CMD_VOLUME = 3,
	HAL_AUDIO_CONFIG_CMD_MUTE = 4,
	HAL_AUDIO_CONFIG_CMD_AO_MIC = 5,
	HAL_AUDIO_CONFIG_CMD_AO_AUX = 6,
	HAL_AUDIO_CONFIG_CMD_AO_ECHO_REVERB = 7,
	HAL_AUDIO_CONFIG_CMD_AO_MIC1_VOLUME = 8,
	HAL_AUDIO_CONFIG_CMD_AO_MIC2_VOLUME = 9,
	HAL_AUDIO_CONFIG_CMD_AO_DEC_VOLUME = 10,
	HAL_AUDIO_CONFIG_CMD_AO_AUX_VOLUME = 11,
	HAL_AUDIO_CONFIG_CMD_DAC_SWITCH = 12,
	HAL_AUDIO_CONFIG_CMD_DD_DUALMONO = 13,
	HAL_AUDIO_CONFIG_CMD_DD_SCALE = 14,
	HAL_AUDIO_CONFIG_CMD_DD_DOWNMIXMODE = 15,
	HAL_AUDIO_CONFIG_CMD_DD_COMP = 16,
	HAL_AUDIO_CONFIG_CMD_DD_LFE = 17,
	HAL_AUDIO_CONFIG_CMD_DD_STEREOMIX = 18,
	HAL_AUDIO_CONFIG_CMD_DIGITAL_OUT = 19,
	HAL_AUDIO_CONFIG_CMD_EXCLUSIVE = 20,
	HAL_AUDIO_CONFIG_CMD_AC3_SPDIF_RAW = 21,
	HAL_AUDIO_CONFIG_CMD_AC3_HDMI_RAW = 22,
	HAL_AUDIO_CONFIG_CMD_DTS_SPDIF_RAW = 23,
	HAL_AUDIO_CONFIG_CMD_DTS_HDMI_RAW = 24,
	HAL_AUDIO_CONFIG_CMD_MPG_SPDIF_RAW = 25,
	HAL_AUDIO_CONFIG_CMD_MPG_HDMI_RAW = 26,
	HAL_AUDIO_CONFIG_CMD_AAC_SPDIF_RAW = 27,
	HAL_AUDIO_CONFIG_CMD_AAC_HDMI_RAW = 28,
	HAL_AUDIO_CONFIG_CMD_MLP_HDMI_RAW = 29,
	HAL_AUDIO_CONFIG_CMD_DDP_HDMI_RAW = 30,
	HAL_AUDIO_CONFIG_CMD_HDMI_CHANNEL_OUT = 31,
	HAL_AUDIO_CONFIG_CMD_FORCE_CHANNEL_CODEC = 32,
	HAL_AUDIO_CONFIG_CMD_MLP_SPDIF_RAW = 33,
	HAL_AUDIO_CONFIG_CMD_DDP_SPDIF_RAW = 34,
	HAL_AUDIO_CONFIG_CMD_MAX_OUTPUT_SAMPLERATE = 35,
	HAL_AUDIO_CONFIG_CMD_USB_DEVICE_SUPPORT_SAMPLERATE = 36,
	HAL_AUDIO_CONFIG_CMD_KARAOKE_MODE = 37,
	HAL_AUDIO_CONFIG_CMD_BRAZIL_LATM_AAC = 38,
	HAL_AUDIO_CONFIG_CMD_DRC_PERCENT = 39,
	HAL_AUDIO_CONFIG_CMD_LICENSE_MODE = 40,
	HAL_AUDIO_CONFIG_CMD_AUDIO_TYPE_PRIORITY = 41,
	HAL_AUDIO_CONFIG_CMD_SPDIF_OUTPUT_SWITCH = 42,
	HAL_AUDIO_CONFIG_CMD_CUSTOMIZE_REQUEST = 43,
	HAL_AUDIO_CONFIG_CMD_DEC_PLAYBACK_RATE = 44,
	HAL_AUDIO_CONFIG_CMD_SAMPLERATE_TOLERANCE = 45,
	HAL_AUDIO_CONFIG_CMD_KARAOKE_GENDER_CHANGE = 46,
	HAL_AUDIO_CONFIG_CMD_SET_DEC_DELAY_RP = 47,
	HAL_AUDIO_CONFIG_CMD_AO_OUTPUT_CONFIG = 48,
	HAL_AUDIO_CONFIG_CMD_SET_AI_VOLUME = 49,
	HAL_AUDIO_CONFIG_CMD_AO_DEC_MUTE = 50,
	HAL_AUDIO_CONFIG_TFAGC_DEBUG_PRINT = 51,
	HAL_AUDIO_CONFIG_ASF_DEBUG_PRINT = 52,
	HAL_AUDIO_CONFIG_CMD_AO_OUTPUT_VOLUME = 53,
	HAL_AUDIO_CONFIG_CMD_KARAOKE_MODE_FORTV_PROJECT = 54,
	HAL_AUDIO_CONFIG_CMD_AVOUT_FROM_HW = 56,
	HAL_AUDIO_CONFIG_CMD_CONFIG_AIO_AIO1 = 57,
	HAL_AUDIO_CONFIG_CMD_CONFIG_AIO_AIO2 = 58,
	HAL_AUDIO_CONFIG_CMD_CONFIG_AIO_HEADPHONE = 59,
	HAL_AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_VOLUME = 60,
	HAL_AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_MUTE = 61,
	HAL_AUDIO_CONFIG_CMD_SET_SPDIF_CS_INFO = 62,
	HAL_AUDIO_CONFIG_CMD_HDMI_ARC = 63,
	HAL_AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_PB_VOLUME = 64,
	HAL_AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_PB_MUTE = 65,
	HAL_AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_TS_VOLUME = 66,
	HAL_AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_TS_MUTE = 67,
	HAL_AUDIO_CONFIG_CMD_AO_FLASH_PB_VOLUME = 68,
	HAL_AUDIO_CONFIG_CMD_AO_FLASH_PB_MUTE = 69,
	HAL_AUDIO_CONFIG_CMD_AO_FLASH_TS_VOLUME = 70,
	HAL_AUDIO_CONFIG_CMD_AO_FLASH_TS_MUTE = 71,
	HAL_AUDIO_CONFIG_CMD_AI_CEI_ENABLE = 72,
	HAL_AUDIO_CONFIG_CMD_AI_CEI_SETMODE = 73,
	HAL_AUDIO_CONFIG_CMD_AD_DESCRIPTOR = 74,
	HAL_AUDIO_CONFIG_CMD_DEFAULT_DIALNORM = 75,
	HAL_AUDIO_CONFIG_CMD_AI_CEI_SET_PARAMETER = 76,
	HAL_AUDIO_CONFIG_CMD_AI_CEI_GET_PARAMETER = 77,
	HAL_AUDIO_CONFIG_CMD_AI_TRANSFER_MATRIX = 78,
	HAL_AUDIO_CONFIG_CMD_SET_VOLUME = 100,
	HAL_AUDIO_CONFIG_CMD_SET_MUTE = 101,
	HAL_AUDIO_CONFIG_CMD_SET_DELAY = 102,
	HAL_AUDIO_CONFIG_CMD_SET_SCART_ON = 103,
	HAL_AUDIO_CONFIG_CMD_SET_NO_DELAY_PARAM = 104,
	HAL_AUDIO_CONFIG_CMD_SET_SPDIF_CS_ALL = 105,
	HAL_AUDIO_CONFIG_CMD_ARC = 106,
	HAL_AUDIO_CONFIG_CMD_NTFY_HAL_HDMI_FMT = 107,
	HAL_AUDIO_CONFIG_CMD_AANDROID_DOLBY_DYN_RANGE = 108,
	HAL_AUDIO_CONFIG_CMD_AANDROID_HEAAC_DYN_RANGE = 109,
	HAL_AUDIO_CONFIG_CMD_AANDROID_MPEG_AUD_LEVEL = 110,
	HAL_AUDIO_CONFIG_CMD_AANDROID_HEAAC_AUD_LEVEL = 111,
	HAL_AUDIO_CONFIG_CMD_SET_PCM_INFO = 112,
	HAL_AUDIO_CONFIG_CMD_GET_TASK_STARTED = 113,
	HAL_AUDIO_CONFIG_CMD_DBX_DEBUG = 114,
	HAL_AUDIO_CONFIG_CMD_DBX_CONFIG = 115,
	HAL_AUDIO_CONFIG_CMD_DBX_TAB_CONFIG = 116,
	HAL_AUDIO_CONFIG_CMD_AO_ENCODER_ENABLE = 117,
	HAL_AUDIO_CONFIG_CMD_SET_GAP_PTS = 118,
} HAL_AUDIO_CONFIG_CMD_MSG;

typedef struct {
	HAL_AUDIO_CONFIG_CMD_MSG msgID;
	UINT32 value[15];
} HAL_AUDIO_CONFIG_COMMAND_T;

typedef struct {
	SLONG enable;
	SLONG bandnum;
	SLONG data_addr;
	SLONG upper_bandlimit_addr;
} HAL_AUDIO_SPECTRUM_CFG_T;

typedef enum {
    HAL_AUDIO_DTS_DEC_DRC,
    HAL_AUDIO_DTS_DEC_LFEMIX,
} HAL_AUDIO_DTS_DEC_PARAM_TYPE;

/**
 * AO pinID define
 *  according to AUDIO_IO_PIN defin in common
 */
typedef  enum
{
    HAL_AUDIO_BT_PCM_OUT 	= 5, // according to AUDIO_IO_PIN defin in common
    HAL_AUDIO_BT_PCM_OUT1	= 13,// according to AUDIO_IO_PIN defin in common
    HAL_AUDIO_BT_PCM_OUT2	= 14,// according to AUDIO_IO_PIN defin in common
    HAL_AUDIO_BT_PCM_OUT3	= 23,// according to AUDIO_IO_PIN defin in common
} HAL_AUDIO_BT_OUT_PIN;

/**
*ATV SCAN PRIORIY
**/
typedef enum {
	SIF_MTS_PRIO_BTSC		= 0,
	SIF_MTS_PRIO_EIAJ		= 1,
	SIF_MTS_PRIO_MONO		= 2,
	SIF_MTS_PRIO_DK			= 3,
	SIF_MTS_PRIO_L			= 4,
} HAL_SIF_MTS_PRIORITY_T;

/* ============================ for DAP API ==================================================*/
#define HAL_MS_MIN_MAX_CHANNEL     6                 /**< Minimum value for max. number of channels        */
#define HAL_MS_MAX_MAX_CHANNEL     8                 /**< Maximum value for max. number of channels        */
#define HAL_DAP_MAX_BANDS          (20)
#define HAL_DAP_IEQ_MAX_BANDS      HAL_DAP_MAX_BANDS
#define HAL_DAP_GEQ_MAX_BANDS      HAL_DAP_MAX_BANDS
#define HAL_DAP_REG_MAX_BANDS      HAL_DAP_MAX_BANDS
#define HAL_DAP_OPT_MAX_BANDS      HAL_DAP_MAX_BANDS
#define HAL_DAP_MAX_CHANNELS       (HAL_MS_MAX_MAX_CHANNEL)

/**
 * HAL AUDIO DAP API Type
 *
 */
typedef enum {
	HAL_AUDIO_SET_DAP_ENABLE                = 0,
	HAL_AUDIO_GET_DAP_ENABLE                = 1,
	HAL_AUDIO_SET_DAP_PARAM                 = 2,
	HAL_AUDIO_GET_DAP_PARAM                 = 3,
	HAL_AUDIO_SET_DAP_DIALOGUE_ENHANCER     = 4,
	HAL_AUDIO_GET_DAP_DIALOGUE_ENHANCER     = 5,
	HAL_AUDIO_SET_DAP_VOLUME_LEVELER        = 6,
	HAL_AUDIO_GET_DAP_VOLUME_LEVELER        = 7,
	HAL_AUDIO_SET_DAP_INTELLIGENT_EQUALIZER = 8,
	HAL_AUDIO_GET_DAP_INTELLIGENT_EQUALIZER = 9,
	HAL_AUDIO_SET_DAP_SURROUND_DECODER      = 10,
	HAL_AUDIO_GET_DAP_SURROUND_DECODER      = 11,
	HAL_AUDIO_SET_DAP_MEDIA_INTELLIGENCE    = 12,
	HAL_AUDIO_GET_DAP_MEDIA_INTELLIGENCE    = 13,
	HAL_AUDIO_SET_DAP_SURROUND_VIRTUALIZER  = 14,
	HAL_AUDIO_GET_DAP_SURROUND_VIRTUALIZER  = 15,
	HAL_AUDIO_SET_DAP_GRAPHICAL_EQUALIZER   = 16,
	HAL_AUDIO_GET_DAP_GRAPHICAL_EQUALIZER   = 17,
	HAL_AUDIO_SET_DAP_AUDIO_OPTIMIZER       = 18,
	HAL_AUDIO_GET_DAP_AUDIO_OPTIMIZER       = 19,
	HAL_AUDIO_SET_DAP_AUDIO_REGULATOR       = 20,
	HAL_AUDIO_GET_DAP_AUDIO_REGULATOR       = 21,
	HAL_AUDIO_SET_DAP_BASS_ENHANCER         = 22,
	HAL_AUDIO_GET_DAP_BASS_ENHANCER         = 23,
	HAL_AUDIO_SET_DAP_VIRTUAL_BASS          = 24,
	HAL_AUDIO_GET_DAP_VIRTUAL_BASS          = 25,
	HAL_AUDIO_SET_DAP_BASS_EXTRACTION       = 26,
	HAL_AUDIO_GET_DAP_BASS_EXTRACTION       = 27,
	HAL_AUDIO_SET_DAP_DMX_TYPE              = 28,
	HAL_AUDIO_GET_DAP_DMX_TYPE              = 29,
	HAL_AUDIO_SET_DAP_VIRTUALIZER_ENABLE    = 30,
	HAL_AUDIO_GET_DAP_VIRTUALIZER_ENABLE    = 31,
	HAL_AUDIO_SET_DAP_HEADPHONE_REVERB      = 32,
	HAL_AUDIO_GET_DAP_HEADPHONE_REVERB      = 33,
	HAL_AUDIO_SET_DAP_SPEAKER_START         = 34,
	HAL_AUDIO_GET_DAP_SPEAKER_START         = 35,
	HAL_AUDIO_SET_DAP_DE_DUCKING            = 36,
	HAL_AUDIO_GET_DAP_DE_DUCKING            = 37,
	HAL_AUDIO_SET_DAP_PREGAIN               = 38,
	HAL_AUDIO_GET_DAP_PREGAIN               = 39,
	HAL_AUDIO_SET_DAP_POSTGAIN              = 40,
	HAL_AUDIO_GET_DAP_POSTGAIN              = 41,
} HAL_AUDIO_DAP_PARAM_TYPE;

typedef struct
{
    int de_enable;
    int de_amount;

    /* for MS12_v1 */
    int de_ducking;
} HAL_DAP_PARAM_DIALOGUE_ENHANCER;

typedef struct
{
    int leveler_amount;

    /* for MS12_v1 */
    int leveler_enable;
    int leveler_ignore_il;

    /* for MS12_v2 */
    int leveler_setting;

    int leveler_input;
    int leveler_output;

} HAL_DAP_PARAM_VOLUME_LEVELER;

typedef struct
{
    int ieq_enable;
    int ieq_amount;
    int ieq_nb_bands;
    int a_ieq_band_center[HAL_DAP_IEQ_MAX_BANDS];
    int a_ieq_band_target[HAL_DAP_IEQ_MAX_BANDS];
} HAL_DAP_PARAM_INTELLIGENT_EQUALIZER;

typedef struct
{
    int mi_ieq_enable;
    int mi_dv_enable;
    int mi_de_enable;
    int mi_surround_enable;
} HAL_DAP_PARAM_MEDIA_INTELLIGENCE;

typedef struct
{
    int speaker_angle;
    int surround_boost;

    /* for MS12_v1 */
    int virtualizer_enable;
    int headphone_reverb;
    int speaker_start;

    /* for MS12_v2 */
    int virtualizer_mode;
} HAL_DAP_PARAM_SURROUND_VIRTUALIZER;

typedef struct
{
    int eq_enable;
    int eq_nb_bands;
    int a_geq_band_center[HAL_DAP_GEQ_MAX_BANDS];
    int a_geq_band_target[HAL_DAP_GEQ_MAX_BANDS];
} HAL_DAP_PARAM_GRAPHICAL_EQUALIZER;

typedef struct
{
    int optimizer_enable;
    int optimizer_nb_bands;
    int a_opt_band_center_freq[HAL_DAP_OPT_MAX_BANDS];
    int a_opt_band_gain[HAL_DAP_MAX_CHANNELS][HAL_DAP_OPT_MAX_BANDS];
} HAL_DAP_PARAM_AUDIO_OPTIMIZER;

typedef struct
{
    int reg_nb_bands;
    int a_reg_band_center[HAL_DAP_REG_MAX_BANDS];
    int a_reg_low_thresholds[HAL_DAP_REG_MAX_BANDS];
    int a_reg_high_thresholds[HAL_DAP_REG_MAX_BANDS];
    int a_reg_isolated_bands[HAL_DAP_REG_MAX_BANDS];

    int regulator_overdrive;
    int regulator_timbre;
    int regulator_distortion;
    int regulator_mode;
    int regulator_enable;
} HAL_DAP_PARAM_AUDIO_REGULATOR;

typedef struct
{
    int bass_enable;
    int bass_boost;
    int bass_cutoff;
    int bass_width;
} HAL_DAP_PARAM_BASS_ENHANCER;

typedef struct
{
    int b_virtual_bass_enabled;
    int vb_mode;
    int vb_low_src_freq;
    int vb_high_src_freq;
    int vb_overall_gain;
    int vb_slope_gain;
    int vb_subgain[3];
    int vb_mix_low_freq;
    int vb_mix_high_freq;
} HAL_DAP_PARAM_VIRTUAL_BASS;

typedef struct
{
    int drc_type;
    int speaker_lfe;
    int pregain;
    int postgain;
    int systemgain;
    int surround_decoder_enable;
    int height_filter_mode;
    int calibration_boost;
    int leveler_input;
    int leveler_output;
    int modeler_enable;
    int modeler_calibration;
    int volmax_boost;

    HAL_DAP_PARAM_DIALOGUE_ENHANCER dialogue_enhancer;
    HAL_DAP_PARAM_VOLUME_LEVELER volume_leveler;
    HAL_DAP_PARAM_INTELLIGENT_EQUALIZER intelligent_equalizer;
    HAL_DAP_PARAM_MEDIA_INTELLIGENCE media_intelligence;
    HAL_DAP_PARAM_SURROUND_VIRTUALIZER surround_virtualizer;
    HAL_DAP_PARAM_GRAPHICAL_EQUALIZER graphical_equalizer;
    HAL_DAP_PARAM_AUDIO_OPTIMIZER audio_optimizer;
    HAL_DAP_PARAM_AUDIO_REGULATOR audio_regulator;
    HAL_DAP_PARAM_BASS_ENHANCER bass_enhancer;
    HAL_DAP_PARAM_VIRTUAL_BASS virtual_bass;
} HAL_DAP_PARAM;
/* ===========================================================================================*/

/**
 * DTS VX parameter type
**/
typedef enum {
  HAL_AUDIO_VX_PARAM_VX_ENABLE,
  HAL_AUDIO_VX_PARAM_VX_INPUT_MODE,
  HAL_AUDIO_VX_PARAM_VX_OUTPUT_MODE,
  HAL_AUDIO_VX_PARAM_VX_HEADROOM_GAIN,
  HAL_AUDIO_VX_PARAM_VX_PROC_OUTPUT_GAIN,
  HAL_AUDIO_VX_PARAM_VX_REFERENCE_LEVEL,
  HAL_AUDIO_VX_PARAM_VX_GAIN_COMPENSATION,
  HAL_AUDIO_VX_PARAM_VX_PROCESS_DISCARD,
  HAL_AUDIO_VX_PARAM_TSX_ENABLE,
  HAL_AUDIO_VX_PARAM_TSX_PASSIVEMATRIXUPMIX_ENABLE,
  HAL_AUDIO_VX_PARAM_TSX_HEIGHT_UPMIX_ENABLE,
  HAL_AUDIO_VX_PARAM_TSX_PHANTOM_CENTER_GAIN,
  HAL_AUDIO_VX_PARAM_TSX_CENTER_GAIN,
  HAL_AUDIO_VX_PARAM_TSX_HORIZ_VIR_EFF_CTRL,
  HAL_AUDIO_VX_PARAM_TSX_HEIGHTMIX_COEFF,
  HAL_AUDIO_VX_PARAM_TSX_HEIGHTOUTCH_GAIN,
  HAL_AUDIO_VX_PARAM_TSX_LFE_GAIN,
  HAL_AUDIO_VX_PARAM_TSX_LRMIX_RATIO2CTRGAIN,
  HAL_AUDIO_VX_PARAM_TSX_PRECOND_FRONT,
  HAL_AUDIO_VX_PARAM_TSX_PRECOND_SURND,
  HAL_AUDIO_VX_PARAM_TSX_TOPSPK_LOC_CTRL,
  HAL_AUDIO_VX_PARAM_TSX_FRONT_WIDE_ENABLE,
  HAL_AUDIO_VX_PARAM_TSX_HEIGHT_VIRTUALIZER_ENABLE,
  HAL_AUDIO_VX_PARAM_TSX_FRONT_SURROUND_ENABLE,
  HAL_AUDIO_VX_PARAM_TSX_HEIGHT_DISCARD,
  HAL_AUDIO_VX_PARAM_VX_DC_ENABLE,
  HAL_AUDIO_VX_PARAM_VX_DC_CONTROL,
  HAL_AUDIO_VX_PARAM_VX_DEF_ENABLE,
  HAL_AUDIO_VX_PARAM_VX_DEF_CONTROL,
  HAL_AUDIO_VX_PARAM_VX_CS2TO3_ENABLE,
  HAL_AUDIO_VX_PARAM_TSX_CUSTOMCOEFF_STU,
  HAL_AUDIO_VX_PARAM_TSX_CONTROLS_STU,
  HAL_AUDIO_VX_PARAM_TBHDX_FRONT_ENABLE,
  HAL_AUDIO_VX_PARAM_TBHDX_FRONT_MODE,
  HAL_AUDIO_VX_PARAM_TBHDX_FRONT_SPKSIZE,
  HAL_AUDIO_VX_PARAM_TBHDX_FRONT_DYNAMICS,
  HAL_AUDIO_VX_PARAM_TBHDX_FRONT_HP_ENABLE,
  HAL_AUDIO_VX_PARAM_TBHDX_FRONT_HPORDER,
  HAL_AUDIO_VX_PARAM_TBHDX_FRONT_BASSLVL,
  HAL_AUDIO_VX_PARAM_TBHDX_FRONT_EXTBASS,
  HAL_AUDIO_VX_PARAM_TBHDX_FRONT_INPUT_GAIN,
  HAL_AUDIO_VX_PARAM_TBHDX_FRONT_BYPASS_GAIN,
  HAL_AUDIO_VX_PARAM_TBHDX_FRONT_CUSTOMCOEFF_STU,
  HAL_AUDIO_VX_PARAM_TBHDX_REAR_ENABLE,
  HAL_AUDIO_VX_PARAM_TBHDX_REAR_MODE,
  HAL_AUDIO_VX_PARAM_TBHDX_REAR_SPKSIZE,
  HAL_AUDIO_VX_PARAM_TBHDX_REAR_DYNAMICS,
  HAL_AUDIO_VX_PARAM_TBHDX_REAR_HP_ENABLE,
  HAL_AUDIO_VX_PARAM_TBHDX_REAR_HPORDER,
  HAL_AUDIO_VX_PARAM_TBHDX_REAR_BASSLVL,
  HAL_AUDIO_VX_PARAM_TBHDX_REAR_EXTBASS,
  HAL_AUDIO_VX_PARAM_TBHDX_REAR_INPUT_GAIN,
  HAL_AUDIO_VX_PARAM_TBHDX_REAR_BYPASS_GAIN,
  HAL_AUDIO_VX_PARAM_TBHDX_REAR_CUSTOMCOEFF_STU,
  HAL_AUDIO_VX_PARAM_TBHDX_CENTER_ENABLE,
  HAL_AUDIO_VX_PARAM_TBHDX_CENTER_SPKSIZE,
  HAL_AUDIO_VX_PARAM_TBHDX_CENTER_DYNAMICS,
  HAL_AUDIO_VX_PARAM_TBHDX_CENTER_HP_ENABLE,
  HAL_AUDIO_VX_PARAM_TBHDX_CENTER_HPORDER,
  HAL_AUDIO_VX_PARAM_TBHDX_CENTER_BASSLVL,
  HAL_AUDIO_VX_PARAM_TBHDX_CENTER_EXTBASS,
  HAL_AUDIO_VX_PARAM_TBHDX_CENTER_INPUT_GAIN,
  HAL_AUDIO_VX_PARAM_TBHDX_CENTER_BYPASS_GAIN,
  HAL_AUDIO_VX_PARAM_TBHDX_CENTER_CUSTOMCOEFF_STU,
  HAL_AUDIO_VX_PARAM_TBHDX_SRRND_ENABLE,
  HAL_AUDIO_VX_PARAM_TBHDX_SRRND_MODE,
  HAL_AUDIO_VX_PARAM_TBHDX_SRRND_SPKSIZE,
  HAL_AUDIO_VX_PARAM_TBHDX_SRRND_DYNAMICS,
  HAL_AUDIO_VX_PARAM_TBHDX_SRRND_HP_ENABLE,
  HAL_AUDIO_VX_PARAM_TBHDX_SRRND_HPORDER,
  HAL_AUDIO_VX_PARAM_TBHDX_SRRND_BASSLVL,
  HAL_AUDIO_VX_PARAM_TBHDX_SRRND_EXTBASS,
  HAL_AUDIO_VX_PARAM_TBHDX_SRRND_INPUT_GAIN,
  HAL_AUDIO_VX_PARAM_TBHDX_SRRND_BYPASS_GAIN,
  HAL_AUDIO_VX_PARAM_TBHDX_SRRND_CUSTOMCOEFF_STU,
  HAL_AUDIO_VX_PARAM_TBHDX_DELAY_MATCHING_GAIN,
  HAL_AUDIO_VX_PARAM_TBHDX_PROCESS_DISCARD,
  HAL_AUDIO_VX_PARAM_TBHDX_DELAY,
  HAL_AUDIO_VX_PARAM_TBHDX_CONTROLS_STU,
  HAL_AUDIO_VX_PARAM_TBHDX_APP_FIRST_PARAM,
  HAL_AUDIO_VX_PARAM_TBHDX_APP_SPKSIZE,
  HAL_AUDIO_VX_PARAM_TBHDX_APP_HPRATIO_F32,
  HAL_AUDIO_VX_PARAM_TBHDX_APP_EXTBASS_F32,
  HAL_AUDIO_VX_PARAM_TBHDX_APP_LAST_PARAM,
  HAL_AUDIO_VX_PARAM_MBHL_ENABLE,
  HAL_AUDIO_VX_PARAM_MBHL_IO_MODE,
  HAL_AUDIO_VX_PARAM_MBHL_ALG_DELAY,
  HAL_AUDIO_VX_PARAM_MBHL_ALG_GAIN,
  HAL_AUDIO_VX_PARAM_MBHL_BYPASS_GAIN,
  HAL_AUDIO_VX_PARAM_MBHL_REFERENCE_LEVEL,
  HAL_AUDIO_VX_PARAM_MBHL_VOLUME,
  HAL_AUDIO_VX_PARAM_MBHL_VOLUME_STEP,
  HAL_AUDIO_VX_PARAM_MBHL_BALANCE_STEP,
  HAL_AUDIO_VX_PARAM_MBHL_OUTPUT_GAIN,
  HAL_AUDIO_VX_PARAM_MBHL_BOOST,
  HAL_AUDIO_VX_PARAM_MBHL_THRESHOLD,
  HAL_AUDIO_VX_PARAM_MBHL_SLOW_OFFSET,
  HAL_AUDIO_VX_PARAM_MBHL_FAST_ATTACK,
  HAL_AUDIO_VX_PARAM_MBHL_FAST_RELEASE,
  HAL_AUDIO_VX_PARAM_MBHL_SLOW_ATTACK,
  HAL_AUDIO_VX_PARAM_MBHL_SLOW_RELEASE,
  HAL_AUDIO_VX_PARAM_MBHL_DELAY,
  HAL_AUDIO_VX_PARAM_MBHL_ENVELOPE_FREQUENCY,
  HAL_AUDIO_VX_PARAM_MBHL_MODE,
  HAL_AUDIO_VX_PARAM_MBHL_CROSS_LOW,
  HAL_AUDIO_VX_PARAM_MBHL_CROSS_MID,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_ATTACK,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_LOW_RELEASE,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_LOW_RATIO,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_LOW_THRESH,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_LOW_MAKEUP,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_MID_RELEASE,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_MID_RATIO,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_MID_THRESH,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_MID_MAKEUP,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_HIGH_RELEASE,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_HIGH_RATIO,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_HIGH_THRESH,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_HIGH_MAKEUP,
  HAL_AUDIO_VX_PARAM_MBHL_GAIN_REDUCTION,
  HAL_AUDIO_VX_PARAM_MBHL_COMP_GAIN_REDUCTION,
  HAL_AUDIO_VX_PARAM_MBHL_PROCESS_DISCARD,
  HAL_AUDIO_VX_PARAM_MBHL_AC_ENABLE,
  HAL_AUDIO_VX_PARAM_MBHL_AC_LEVEL,
  HAL_AUDIO_VX_PARAM_MBHL_CP_ENABLE,
  HAL_AUDIO_VX_PARAM_MBHL_CP_LEVEL,
  HAL_AUDIO_VX_PARAM_MBHL_COEF_STU,
  HAL_AUDIO_VX_PARAM_MBHL_CONTROLS_STU,
  HAL_AUDIO_VX_PARAM_MBHL_LAST_PARAM,
  HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_ENABLE,
  HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_IO_MODE,
  HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_TARGET_LOUDNESS,
  HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_PRESET,
  HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_DISCARD,
  HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_LATENCY_MODE,
  HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_DELAY,
  HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_CONTROLS_STU,
  HAL_AUDIO_VX_PARAM_GEQ10B_ENABLE,
  HAL_AUDIO_VX_PARAM_GEQ10B_CHANNEL_CTRL_MASK,
  HAL_AUDIO_VX_PARAM_GEQ10B_INPUT_GAIN,
  HAL_AUDIO_VX_PARAM_GEQ10B_BAND0_GAIN,
  HAL_AUDIO_VX_PARAM_GEQ10B_BAND1_GAIN,
  HAL_AUDIO_VX_PARAM_GEQ10B_BAND2_GAIN,
  HAL_AUDIO_VX_PARAM_GEQ10B_BAND3_GAIN,
  HAL_AUDIO_VX_PARAM_GEQ10B_BAND4_GAIN,
  HAL_AUDIO_VX_PARAM_GEQ10B_BAND5_GAIN,
  HAL_AUDIO_VX_PARAM_GEQ10B_BAND6_GAIN,
  HAL_AUDIO_VX_PARAM_GEQ10B_BAND7_GAIN,
  HAL_AUDIO_VX_PARAM_GEQ10B_BAND8_GAIN,
  HAL_AUDIO_VX_PARAM_GEQ10B_BAND9_GAIN,
  HAL_AUDIO_VX_PARAM_GEQ10B_DISCARD,
  HAL_AUDIO_VX_PARAM_GEQ10B_CONTROLS_STU,
  HAL_AUDIO_VX_PARAM_AEQ_ENABLE,
  HAL_AUDIO_VX_PARAM_AEQ_CHANNEL_CTRL_MASK,
  HAL_AUDIO_VX_PARAM_AEQ_INPUT_GAIN,
  HAL_AUDIO_VX_PARAM_AEQ_OUTPUT_GAIN,
  HAL_AUDIO_VX_PARAM_AEQ_BYPASS_GAIN,
  HAL_AUDIO_VX_PARAM_AEQ_CONTROLS_STU,
  HAL_AUDIO_VX_PARAM_AEQ_CUSTOMCOEFF_STU,
  HAL_AUDIO_VX_PARAM_AEQ_PROCESS_DISCARD,
  HAL_AUDIO_VX_PARAM_MULTIRATE_ENABLE,
  HAL_AUDIO_VX_PARAM_MULTIRATE_FILTER_MODE,
  HAL_AUDIO_VX_PARAM_MULTIRATE_IN_MODE,
  HAL_AUDIO_VX_PARAM_MULTIRATE_OUT_MODE,
  HAL_AUDIO_VX_PARAM_MULTIRATE_UPMIX_ENABLE,
  HAL_AUDIO_VX_PARAM_MULTIRATE_DELAY_COMPENSATION,
  HAL_AUDIO_VX_PARAM_MULTIRATE_GAIN_COMPENSATION,
  HAL_AUDIO_VX_PARAM_MULTIRATE_CONTROLS_STU,
  HAL_AUDIO_VX_PARAM_TSX_LPR_GAIN = -1,
  HAL_AUDIO_VX_PARAM_TSX_FRNT_CTRL = -2,
  HAL_AUDIO_VX_PARAM_TSX_SRND_CTRL = -3,
  HAL_AUDIO_VX_PARAM_TBHDX_ENABLE = -4,
  HAL_AUDIO_VX_PARAM_TBHDX_MONO_MODE = -5,
  HAL_AUDIO_VX_PARAM_TBHDX_MAXGAIN = -6,
  HAL_AUDIO_VX_PARAM_TBHDX_TEMP_GAIN = -7,
  HAL_AUDIO_VX_PARAM_TBHDX_PROCESS_DISCARD_I32 = -8,
  HAL_AUDIO_VX_PARAM_TBHDX_APP_SPKSIZE_I32 = -9,
  HAL_AUDIO_VX_PARAM_TSX_PROCESS_DISCARD = -10,
  HAL_AUDIO_VX_PARAM_TBHDX_SPKSIZE = -11,
  HAL_AUDIO_VX_PARAM_TBHDX_HP_ENABLE = -12,
  HAL_AUDIO_VX_PARAM_TBHDX_HPORDER = -13,
  HAL_AUDIO_VX_PARAM_MBHL_PROCESS_DISCARD_I32 = -14,
  HAL_AUDIO_VX_PARAM_MBHL_APP_FIRST_PARAM = -15,
  HAL_AUDIO_VX_PARAM_MBHL_APP_FRT_LOWCROSS_F32 = -16,
  HAL_AUDIO_VX_PARAM_MBHL_APP_FRT_MIDCROSS_F32 = -17,
  HAL_AUDIO_VX_PARAM_MBHL_APP_LAST_PARAM = -18,
} HAL_AUDIO_VX_PARAM_TYPE_T;

/**
 * TRUVOLUME HD parameter type
**/
typedef enum {
  HAL_AUDIO_TRUVOLUMEHD_PARAM_ENABLE,
  HAL_AUDIO_TRUVOLUMEHD_PARAM_TARGET_LOUNDNESS,
  HAL_AUDIO_TRUVOLUMEHD_PARAM_PRESET,
} HAL_AUDIO_TRUVOLUMEHD_PARAM_TYPE_T;

/**
 * ES push mode type
**/
typedef enum {
    HAL_AUDIO_ES_NORMAL = 0,
    HAL_AUDIO_ES_TEST   = 1,
    HAL_AUDIO_ES_MAX    = HAL_AUDIO_ES_TEST,
} HAL_AUDIO_ES_MODE_T;

/**
 *  Area type
**/
typedef enum {
    HAL_AUDIO_AREA_DEFAULT      = 0,
    HAL_AUDIO_AREA_DVB_OR_DTMB  = 1,
    HAL_AUDIO_AREA_ATSC         = 2,
    HAL_AUDIO_AREA_ISDB         = 3,
    HAL_AUDIO_AREA_PA_Analog    = 4,
    HAL_AUDIO_AREA_MAX          = 5     // must be the last
} HAL_AUDIO_AREA_TYPE_T;

typedef struct {
    unsigned int capability;
    unsigned int auth_key[4];
    unsigned char devhash[32];
} AUDIO_CAP;

/**
* HAL AUDIO AEC Source type
*
*/
typedef  enum
{
    HAL_AUDIO_AEC_INTERNAL_LOOP     = 0x01,
    HAL_AUDIO_AEC_I2S_EXTERNEL_LOOP = 0x02,
    HAL_AUDIO_AEC_SOURCE_MAX,   // must be the last
} HAL_AUDIO_AEC_SOURCE_T;

typedef struct HAL_AUDIO_RHAL_INFO
{
    UINT32                          type;
    UINT32                          enable;
    UINT32                          dialEnhanceGain;
    UINT32                          IsExist;
    UINT32                          databytes;
    UINT32                          firstLang;
    UINT32                          delayTime;
    UINT32                          bManualMode;
    UINT16                          u16var;
    UINT32                          u32var[5];
    SINT32                          s32var;

    HAL_AUDIO_ADEC_INDEX_T          adecindex;
    HAL_AUDIO_ADEC_INFO_T           adecinfo;
    HAL_AUDIO_SRC_TYPE_T            audiotype;
    HAL_AUDIO_DOLBY_DRC_MODE_T      drcMode;
    HAL_AUDIO_DOWNMIX_MODE_T        downmixMode;
    HAL_AUDIO_DUALMONO_MODE_T       outputMode;
    HAL_AUDIO_TRICK_MODE_T          eTrickMode;
    HAL_TRACK_MODE                  eTrackMode;
    HAL_AUDIO_VOLUME_T              volume;
    HAL_AUDIO_SNDOUT_T              soundOutType;
    HAL_AUDIO_MIXER_INDEX_T         mixerIndex;
    HAL_AUDIO_AC4_AD_TYPE_T         enADType;
    HAL_AUDIO_LANG_CODE_TYPE_T      enCodeType;
    HAL_AUDIO_ARC_MODE_T            eARCMode;
    HAL_AUDIO_SIF_SOUNDSYSTEM_T     setSoundSystem;
    HAL_AUDIO_SIF_TYPE_T            eSifType;
    HAL_AUDIO_SIF_STANDARD_T        sifStandard;
    HAL_AUDIO_SIF_AVAILE_STANDARD_T availStandard;
    HAL_AUDIO_TP_INDEX_T            tpIndex;
    HAL_AUDIO_BBADC_SRC             portNum;
    HAL_AUDIO_RESOURCE_T            inputConnect;
    HAL_AUDIO_SIF_INPUT_T           sifSource;
    HAL_AUDIO_SIF_MODE_SET_T        sifAudioMode;
    HAL_SIF_OVER_DEVIATION_E        overDeviation;
    HAL_AUDIO_INDEX_T               audioIndex;
    HAL_AUDIO_HDMI_INDEX_T          hdmiIndex;
    HAL_AUDIO_SPDIF_MODE_T          eSPDIFMode;
    HAL_AUDIO_SPDIF_COPYRIGHT_T     copyInfo;
    HAL_AUDIO_PCM_INPUT_T           apcmIndex;
    HAL_AUDIO_BT_OUT_PIN            pinID;
    HAL_AUDIO_DAP_PARAM_TYPE        dap_paramType;
    HAL_AUDIO_TRUVOLUMEHD_PARAM_TYPE_T tvolhd_paramType;
    HAL_AUDIO_PCM_INFO_T            info;
    HAL_AUDIO_DBXTV_PARAM_TYPE      paramType;
    HAL_AUDIO_INPUT_SOURCE_T        eSource;
    HAL_AUDIO_DTS_DEC_PARAM_TYPE    dts_paramType;
    HAL_AUDIO_AREA_TYPE_T           area_type;
    HAL_AUDIO_AEC_SOURCE_T          aec_source;
    HAL_AUDIO_VX_PARAM_TYPE_T       vx_paramtype;
} HAL_AUDIO_RHAL_INFO_T;

typedef struct HAL_AUDIO_RHAL_INFO_ADDR
{
    HAL_AUDIO_ADEC_INDEX_T          adecindex;
    unsigned int                    addr;
    unsigned int                    size;
} HAL_AUDIO_RHAL_INFO_ADDR_T;

typedef struct HAL_AUDIO_RHAL_CONFIG
{
    HAL_AUDIO_RHAL_INFO_T           rhalinfo;
    unsigned int                    type;
    unsigned int                    addr[10];
    unsigned int                    size[10];
} HAL_AUDIO_RHAL_CONFIG_T;

typedef struct HAL_AUDIO_RHAL_BUFFER_INFO
{
    UINT32 MaxSize;
    UINT32 FreeSize;
} HAL_AUDIO_RHAL_BUFFER_INFO_T;

DTV_STATUS_T RHAL_AUDIO_Init_ATV_Driver(HAL_AUDIO_SIF_TYPE_T eSifType);
DTV_STATUS_T SetSPDIFOutVolume(HAL_AUDIO_VOLUME_T volume);
DTV_STATUS_T RHAL_AUDIO_SetMS12Version(HAL_AUDIO_MS12_VERSION_T eMS12Version);
DTV_STATUS_T RHAL_AUDIO_InitializeModule(HAL_AUDIO_SIF_TYPE_T eSifType);
DTV_STATUS_T RHAL_QS_AUDIO_InitializeModule(HAL_AUDIO_SIF_TYPE_T eSifType);
DTV_STATUS_T RHAL_AUDIO_SetSPKOutput(UINT8 i2sNumber, HAL_AUDIO_SAMPLING_FREQ_T samplingFreq);

/* Open, Close */
DTV_STATUS_T RHAL_AUDIO_TP_Open(HAL_AUDIO_TP_INDEX_T tpIndex);
DTV_STATUS_T RHAL_AUDIO_TP_Close(HAL_AUDIO_TP_INDEX_T tpIndex);
DTV_STATUS_T RHAL_AUDIO_ADC_Open(HAL_AUDIO_BBADC_SRC portNum);
DTV_STATUS_T RHAL_AUDIO_ADC_Close(HAL_AUDIO_BBADC_SRC portNum);
DTV_STATUS_T RHAL_AUDIO_HDMI_Open(void);
DTV_STATUS_T RHAL_AUDIO_HDMI_Close(void);
DTV_STATUS_T RHAL_AUDIO_DP_Open(void);
DTV_STATUS_T RHAL_AUDIO_DP_Close(void);

DTV_STATUS_T RHAL_AUDIO_HDMI_OpenPort(HAL_AUDIO_HDMI_INDEX_T hdmiIndex);
DTV_STATUS_T RHAL_AUDIO_HDMI_ClosePort(HAL_AUDIO_HDMI_INDEX_T hdmiIndex);
DTV_STATUS_T RHAL_AUDIO_AAD_Open(void);
DTV_STATUS_T RHAL_AUDIO_AAD_Close(void);
DTV_STATUS_T RHAL_AUDIO_ADEC_Open(HAL_AUDIO_ADEC_INDEX_T adecIndex);
DTV_STATUS_T RHAL_AUDIO_ADEC_Close(HAL_AUDIO_ADEC_INDEX_T adecIndex);
DTV_STATUS_T RHAL_AUDIO_AENC_Open(HAL_AUDIO_AENC_INDEX_T aencIndex);
DTV_STATUS_T RHAL_AUDIO_AENC_Close(HAL_AUDIO_AENC_INDEX_T aencIndex);
DTV_STATUS_T RHAL_AUDIO_SE_Open(void);
DTV_STATUS_T RHAL_AUDIO_SE_Close(void);
DTV_STATUS_T RHAL_AUDIO_SNDOUT_Open(HAL_AUDIO_SNDOUT_T soundOutType);
DTV_STATUS_T RHAL_AUDIO_SNDOUT_Close(HAL_AUDIO_SNDOUT_T soundOutType);


/* Connect & Disconnect */
DTV_STATUS_T RHAL_AUDIO_TP_Connect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect);
DTV_STATUS_T RHAL_AUDIO_TP_Disconnect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect);
DTV_STATUS_T RHAL_AUDIO_ADC_Connect(HAL_AUDIO_BBADC_SRC portNum);
DTV_STATUS_T RHAL_AUDIO_ADC_Disconnect(HAL_AUDIO_BBADC_SRC portNum);
DTV_STATUS_T RHAL_AUDIO_HDMI_Connect(void);
DTV_STATUS_T RHAL_AUDIO_HDMI_Disconnect(void);
DTV_STATUS_T RHAL_AUDIO_DP_Connect(void);
DTV_STATUS_T RHAL_AUDIO_DP_Disconnect(void);
DTV_STATUS_T RHAL_AUDIO_HDMI_ConnectPort(HAL_AUDIO_HDMI_INDEX_T hdmiIndex);
DTV_STATUS_T RHAL_AUDIO_HDMI_DisconnectPort(HAL_AUDIO_HDMI_INDEX_T hdmiIndex);
DTV_STATUS_T RHAL_AUDIO_AAD_Connect(void);
DTV_STATUS_T RHAL_AUDIO_AAD_Disconnect(void);
DTV_STATUS_T RHAL_AUDIO_ADEC_Connect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect);
DTV_STATUS_T RHAL_AUDIO_ADEC_Disconnect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect);
DTV_STATUS_T RHAL_AUDIO_AMIX_Connect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect);
DTV_STATUS_T RHAL_AUDIO_AMIX_Disconnect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect);
DTV_STATUS_T RHAL_AUDIO_AENC_Connect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect);
DTV_STATUS_T RHAL_AUDIO_AENC_Disconnect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect);
DTV_STATUS_T RHAL_AUDIO_SE_Connect(HAL_AUDIO_RESOURCE_T inputConnect);
DTV_STATUS_T RHAL_AUDIO_SE_Disconnect(HAL_AUDIO_RESOURCE_T inputConnect);
DTV_STATUS_T RHAL_AUDIO_SNDOUT_Connect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect);
DTV_STATUS_T RHAL_AUDIO_SNDOUT_Disconnect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect);

/* Start & Stop */
DTV_STATUS_T RHAL_AUDIO_StartDecoding(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_SRC_TYPE_T audioType);
DTV_STATUS_T RHAL_AUDIO_StopDecoding(HAL_AUDIO_ADEC_INDEX_T adecIndex);
DTV_STATUS_T RHAL_AUDIO_SetMainDecoderOutput(HAL_AUDIO_ADEC_INDEX_T adecIndex);
DTV_STATUS_T RHAL_AUDIO_SetMainAudioOutput(HAL_AUDIO_INDEX_T audioIndex);


/* HDMI */
DTV_STATUS_T RHAL_AUDIO_HDMI_GetAudioMode(HAL_AUDIO_HDMI_TYPE_T *pHDMIMode);
DTV_STATUS_T RHAL_AUDIO_HDMI_SetAudioReturnChannel(BOOLEAN bOnOff);
DTV_STATUS_T RHAL_AUDIO_HDMI_SetEnhancedAudioReturnChannel(BOOLEAN bOnOff);
DTV_STATUS_T RHAL_AUDIO_HDMI_GetCopyInfo(HAL_AUDIO_SPDIF_COPYRIGHT_T *pCopyInfo);

/* Decoder */
DTV_STATUS_T RHAL_AUDIO_PauseAdec(HAL_AUDIO_ADEC_INDEX_T adecIndex);
DTV_STATUS_T RHAL_AUDIO_ResumeAdec(HAL_AUDIO_ADEC_INDEX_T adecIndex);
DTV_STATUS_T RHAL_AUDIO_FlushAdec(HAL_AUDIO_ADEC_INDEX_T adecIndex);
DTV_STATUS_T RHAL_AUDIO_SetDolbyDRCMode(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_DOLBY_DRC_MODE_T drcMode);
DTV_STATUS_T RHAL_AUDIO_SetDownMixMode(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_DOWNMIX_MODE_T downmixMode);
DTV_STATUS_T RHAL_AUDIO_GetDecodingType(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_SRC_TYPE_T *pAudioType);
DTV_STATUS_T RHAL_AUDIO_GetAdecStatus(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_ADEC_INFO_T *pAudioAdecInfo);
DTV_STATUS_T RHAL_AUDIO_SetDualMonoOutMode(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_DUALMONO_MODE_T outputMode);
DTV_STATUS_T RHAL_AUDIO_TP_GetESInfo(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_ES_INFO_T *pAudioESInfo);
BOOLEAN RHAL_AUDIO_TP_IsESExist(HAL_AUDIO_ADEC_INDEX_T adecIndex);
DTV_STATUS_T RHAL_AUDIO_TP_GetAudioPTS(HAL_AUDIO_ADEC_INDEX_T adecIndex, UINT32 *pPts);
DTV_STATUS_T RHAL_AUDIO_TP_GetAudioPTS64(HAL_AUDIO_ADEC_INDEX_T adecIndex, UINT64 *pPts);
DTV_STATUS_T RHAL_AUDIO_TP_SetAudioDescriptionMain(HAL_AUDIO_ADEC_INDEX_T adecIndex, BOOLEAN bOnOff);
DTV_STATUS_T RHAL_AUDIO_SetTrickMode(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_TRICK_MODE_T eTrickMode);
DTV_STATUS_T RHAL_AUDIO_TP_GetBufferStatus(HAL_AUDIO_ADEC_INDEX_T adecIndex, UINT32 *pMaxSize, UINT32 *pFreeSize);

/* Volume, Mute & Delay */
DTV_STATUS_T RHAL_AUDIO_SetDecoderInputGain(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_VOLUME_T volume);
DTV_STATUS_T RHAL_AUDIO_SetDecoderDelayTime(HAL_AUDIO_ADEC_INDEX_T adecIndex, UINT32 delayTime);
DTV_STATUS_T RHAL_AUDIO_SetMixerInputGain(HAL_AUDIO_MIXER_INDEX_T mixerIndex, HAL_AUDIO_VOLUME_T volume);
DTV_STATUS_T RHAL_AUDIO_SetGlobalDecoderGain(HAL_AUDIO_VOLUME_T volume);
DTV_STATUS_T RHAL_AUDIO_SetDmicGain(HAL_AUDIO_VOLUME_T volume);

DTV_STATUS_T RHAL_AUDIO_SetSPKOutVolume(HAL_AUDIO_VOLUME_T volume);
DTV_STATUS_T RHAL_AUDIO_SetSPDIFOutVolume(HAL_AUDIO_VOLUME_T volume);
DTV_STATUS_T RHAL_AUDIO_SetHPOutVolume(HAL_AUDIO_VOLUME_T volume, BOOLEAN bForced);
DTV_STATUS_T RHAL_AUDIO_SetSCARTOutVolume(HAL_AUDIO_VOLUME_T volume, BOOLEAN bForced);
DTV_STATUS_T RHAL_AUDIO_SetAudioDescriptionVolume(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_VOLUME_T volume);
DTV_STATUS_T RHAL_AUDIO_SetDecoderInputMute(HAL_AUDIO_ADEC_INDEX_T adecIndex, BOOLEAN bOnOff);
DTV_STATUS_T RHAL_AUDIO_SetDecoderInputESMute(HAL_AUDIO_ADEC_INDEX_T adecIndex, BOOLEAN bOnOff);
DTV_STATUS_T RHAL_AUDIO_SetMixerInputMute(HAL_AUDIO_MIXER_INDEX_T mixerIndex, BOOLEAN bOnOff);
DTV_STATUS_T RHAL_AUDIO_SetSPKOutMute(BOOLEAN bOnOff);
DTV_STATUS_T RHAL_AUDIO_SetSPDIFOutMute(BOOLEAN bOnOff);
DTV_STATUS_T RHAL_AUDIO_SetHPOutMute(BOOLEAN bOnOff);
DTV_STATUS_T RHAL_AUDIO_GetDecoderInputGain(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_VOLUME_T *volume);
DTV_STATUS_T RHAL_AUDIO_GetMixerInputGain(HAL_AUDIO_MIXER_INDEX_T mixerIndex, HAL_AUDIO_VOLUME_T *volume);
DTV_STATUS_T RHAL_AUDIO_GetDecoderInputMute(HAL_AUDIO_ADEC_INDEX_T adecIndex, BOOLEAN *bOnOff);
DTV_STATUS_T RHAL_AUDIO_GetMixerInputMute(HAL_AUDIO_MIXER_INDEX_T mixerIndex, BOOLEAN *bOnOff);

DTV_STATUS_T RHAL_AUDIO_GetSPKOutMuteStatus(BOOLEAN *pOnOff);
DTV_STATUS_T RHAL_AUDIO_GetSPDIFOutMuteStatus(BOOLEAN *pOnOff);
DTV_STATUS_T RHAL_AUDIO_GetHPOutMuteStatus(BOOLEAN *pOnOff);
DTV_STATUS_T RHAL_AUDIO_GetSCARTOutMuteStatus(BOOLEAN *pOnOff);

DTV_STATUS_T RHAL_AUDIO_SetSPKOutDelayTime(UINT32 delayTime, BOOLEAN bForced);
DTV_STATUS_T RHAL_AUDIO_SetSPDIFOutDelayTime(UINT32 delayTime, BOOLEAN bForced);
DTV_STATUS_T RHAL_AUDIO_SetHPOutDelayTime(UINT32 delayTime, BOOLEAN bForced);
DTV_STATUS_T RHAL_AUDIO_SetSCARTOutDelayTime(UINT32 delayTime, BOOLEAN bForced);
DTV_STATUS_T RHAL_AUDIO_GetStatusInfo(HAL_AUDIO_COMMON_INFO_T *pAudioStatusInfo);
DTV_STATUS_T RHAL_AUDIO_SNDOUT_GetLatency(HAL_AUDIO_SNDOUT_T soundOutType, int *latency_ms);

DTV_STATUS_T RHAL_AUDIO_AndroidDolbyDynamicRange(UINT32 bMode);
DTV_STATUS_T RHAL_AUDIO_AndroidHEAACDynamicRange(UINT32 bMode);
DTV_STATUS_T RHAL_AUDIO_AndroidMPEGAudioLevel(UINT32 bValue);
DTV_STATUS_T RHAL_AUDIO_AndroidHEAACAudioLevel(UINT32 bValue);

/* SPDIF(Sound Bar) */
DTV_STATUS_T RHAL_AUDIO_SPDIF_SetOutputType(HAL_AUDIO_SPDIF_MODE_T eSPDIFMode, BOOLEAN bForced);
DTV_STATUS_T RHAL_AUDIO_SPDIF_SetCopyInfo(HAL_AUDIO_SPDIF_COPYRIGHT_T copyInfo);
DTV_STATUS_T RHAL_AUDIO_SPDIF_SetCategoryCode(UINT8 categoryCode);
DTV_STATUS_T RHAL_AUDIO_ARC_SetOutputType(HAL_AUDIO_ARC_MODE_T eARCMode, BOOLEAN bForced); // for TB24 ARC DD+ output
DTV_STATUS_T RHAL_AUDIO_SPDIF_SetEnable(BOOLEAN bOnOff);
DTV_STATUS_T RHAL_AUDIO_ATMOS_EncodeOnOff(BOOLEAN bOnOff);

/* AAD */
DTV_STATUS_T RHAL_AUDIO_SIF_SetInputSource(HAL_AUDIO_SIF_INPUT_T sifSource);
DTV_STATUS_T RHAL_AUDIO_SIF_SetHighDevMode(BOOLEAN bOnOff);
DTV_STATUS_T RHAL_Audio_ATV_VERIFY_SET_PATH(void);
DTV_STATUS_T RHAL_Audio_SIF_SET_MTS_PRIORITY(HAL_SIF_MTS_PRIORITY_T Priority);
DTV_STATUS_T RHAL_Audio_ScanFmRadioMode(HAL_AUDIO_SIF_STANDARD_T *SoundSTD, HAL_AUDIO_SIF_CARRIER_INFO_T *SIF_CARRIER_INFO);
DTV_STATUS_T RHAL_Audio_SetFmRadioMode(AUDIO_ATV_FM_RADIO_MODE Enable);
DTV_STATUS_T RHAL_AUDIO_SIF_GetSoundInfo(HAL_ATV_SOUND_INFO_T *SoundInfo);
DTV_STATUS_T RHAL_AUDIO_SIF_SetBandSetup(HAL_AUDIO_SIF_TYPE_T eSifType, HAL_AUDIO_SIF_SOUNDSYSTEM_T sifBand);
DTV_STATUS_T RHAL_AUDIO_SIF_SetModeSetup(HAL_AUDIO_SIF_STANDARD_T sifStandard);
DTV_STATUS_T RHAL_AUDIO_SIF_SetUserAnalogMode(HAL_AUDIO_SIF_MODE_SET_T sifAudioMode);
DTV_STATUS_T RHAL_AUDIO_SIF_SetA2ThresholdLevel(UINT16 thrLevel);
DTV_STATUS_T RHAL_AUDIO_SIF_SetNicamThresholdLevel(UINT16 thrLevel);
DTV_STATUS_T RHAL_AUDIO_SIF_GetBandDetect(HAL_AUDIO_SIF_SOUNDSYSTEM_T soundSystem, UINT32 *pBandStrength);
DTV_STATUS_T RHAL_AUDIO_SIF_DetectSoundSystem(HAL_AUDIO_SIF_SOUNDSYSTEM_T setSoundSystem, BOOLEAN bManualMode, HAL_AUDIO_SIF_SOUNDSYSTEM_T *pDetectSoundSystem, UINT32 *pSignalQuality);
DTV_STATUS_T RHAL_AUDIO_SIF_DetectSoundSystemStd(HAL_AUDIO_SIF_SOUNDSYSTEM_T setSoundSystem, BOOLEAN bManualMode, HAL_AUDIO_SIF_SOUNDSYSTEM_T *pDetectSoundSystem, HAL_AUDIO_SIF_STANDARD_T *pDetectSoundStd, UINT32 *pSignalQuality);
DTV_STATUS_T RHAL_AUDIO_SIF_NoSignalMute(BOOLEAN Enable);
DTV_STATUS_T RHAL_AUDIO_SIF_AutoChangeSoundMode(BOOLEAN Enable);
DTV_STATUS_T RHAL_AUDIO_SIF_AutoChangeSoundStd(BOOLEAN Enable);
DTV_STATUS_T RHAL_AUDIO_SIF_FirstTimePlayMono(BOOLEAN Enable);
DTV_STATUS_T RHAL_AUDIO_SIF_SetFwPriority(UINT16 Priority);
DTV_STATUS_T RHAL_AUDIO_SIF_CheckNicamDigital(HAL_AUDIO_SIF_EXISTENCE_INFO_T *pIsNicamDetect);
DTV_STATUS_T RHAL_AUDIO_SIF_CheckAvailableSystem(HAL_AUDIO_SIF_AVAILE_STANDARD_T standard, HAL_AUDIO_SIF_EXISTENCE_INFO_T *pAvailability);
DTV_STATUS_T RHAL_AUDIO_SIF_CheckA2DK(HAL_AUDIO_SIF_STANDARD_T standard, HAL_AUDIO_SIF_EXISTENCE_INFO_T *pAvailability);
DTV_STATUS_T RHAL_AUDIO_SIF_GetA2StereoLevel(UINT16 *pLevel);
DTV_STATUS_T RHAL_AUDIO_SIF_GetNicamThresholdLevel(UINT16 *pLevel);
DTV_STATUS_T RHAL_AUDIO_SIF_GetNicamStable(UINT16 *IsStable);
DTV_STATUS_T RHAL_AUDIO_SIF_GetCurAnalogMode(HAL_AUDIO_SIF_MODE_GET_T *pSifAudioMode);
BOOLEAN RHAL_AUDIO_SIF_IsSIFExist(void);
DTV_STATUS_T RHAL_AUDIO_SIF_SetAudioEQMode(BOOLEAN bOnOff);

/* AENC: Empty function */
DTV_STATUS_T RHAL_AUDIO_AENC_Start(HAL_AUDIO_AENC_INDEX_T aencIndex, HAL_AUDIO_AENC_ENCODING_FORMAT_T audioType);
DTV_STATUS_T RHAL_AUDIO_AENC_Stop(HAL_AUDIO_AENC_INDEX_T aencIndex);
DTV_STATUS_T RHAL_AUDIO_AENC_RegCallback(HAL_AUDIO_AENC_INDEX_T aencIndex, pfnAENCDataHandling pfnCallBack);
DTV_STATUS_T RHAL_AUDIO_AENC_SetInfo(HAL_AUDIO_AENC_INDEX_T aencIndex, HAL_AUDIO_AENC_INFO_T info);
DTV_STATUS_T RHAL_AUDIO_AENC_GetInfo(HAL_AUDIO_AENC_INDEX_T aencIndex, HAL_AUDIO_AENC_INFO_T *pInfo);
DTV_STATUS_T RHAL_AUDIO_AENC_SetVolume(HAL_AUDIO_AENC_INDEX_T aencIndex, HAL_AUDIO_VOLUME_T volume);
DTV_STATUS_T RHAL_AUDIO_AENC_CopyData(HAL_AUDIO_AENC_INDEX_T aencIndex, UINT8 *pDest, UINT8 *pBufAddr, UINT32 datasize, UINT8 *pRStart, UINT8 *pREnd);
DTV_STATUS_T RHAL_AUDIO_AENC_ReleaseData(HAL_AUDIO_AENC_INDEX_T aencIndex, UINT8 *pBufAddr, UINT32 datasize);

DTV_STATUS_T RHAL_AUDIO_PCM_SetVolume(HAL_AUDIO_PCM_INPUT_T apcmIndex, HAL_AUDIO_VOLUME_T volume);
DTV_STATUS_T RHAL_AUDIO_PCM_SetVolume_ByPin(HAL_AUDIO_PCM_INPUT_T apcmIndex, HAL_AUDIO_VOLUME_T volume, HAL_AUDIO_BT_OUT_PIN pinID);
DTV_STATUS_T RHAL_AUDIO_PCM_SetMute_ByPin(BOOLEAN bOnOff, HAL_AUDIO_BT_OUT_PIN pinID);
DTV_STATUS_T RHAL_AUDIO_FinalizeModule(void);      /* function body is empty */

DTV_STATUS_T RHAL_AUDIO_SIF_GetSoundStandard(HAL_AUDIO_SIF_STANDARD_T *standard);
DTV_STATUS_T RHAL_AUDIO_SIF_DetectComplete(SINT32 *isOff);
DTV_STATUS_T RHAL_AUDIO_SIF_SetOverDeviation(HAL_SIF_OVER_DEVIATION_E overDeviation);

/* Digital info */
DTV_STATUS_T RHAL_AUDIO_GetDigitalInfo(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_DIGITAL_INFO_T*pAudioDigitalInfo);
DTV_STATUS_T RHAL_AUDIO_SwpSetSRS_TrusurroundHD(BOOLEAN para, HAL_APP_TSXT_CFG* pTSXT);
DTV_STATUS_T RHAL_AUDIO_DBXTV_SET_TABLE(UINT32* table, UINT32 size);
DTV_STATUS_T RHAL_AUDIO_DBXTV_SET_SOUND_EFFECT(HAL_AUDIO_DBXTV_PARAM_TYPE paramType, UINT8 mode);
DTV_STATUS_T RHAL_AUDIO_DBXTV_DEBUG_CMD(UINT32 cmd, UINT32* param, UINT32 paramSize);
#define RHAL_AUDIO_Effect_GetDigitalInfo(adecIndex, pAudioDigitalInfo) RHAL_AUDIO_GetDigitalInfo(adecIndex, pAudioDigitalInfo)

/// for system request, only stop flow directly  no need to delete it.
// pair of HAL_AUDIO_ResumeAndRunAdec
// pvr use
DTV_STATUS_T RHAL_AUDIO_PauseAndStopFlowAdec(HAL_AUDIO_ADEC_INDEX_T adecIndex);
DTV_STATUS_T RHAL_AUDIO_ResumeAndRunAdec(HAL_AUDIO_ADEC_INDEX_T adecIndex);
DTV_STATUS_T RHAL_AUDIO_SetInputVolume(HAL_AUDIO_INPUT_SOURCE_T eSource,SINT32 sVol);
#define RHAL_AUDIO_Effect_SetInputVolume(eSource, sVol) RHAL_AUDIO_SetInputVolume(eSource, sVol)

DTV_STATUS_T RHAL_AUDIO_setTrackMode(HAL_TRACK_MODE eMode);
#define RHAL_AUDIO_Effect_setTrackMode(eMode) RHAL_AUDIO_setTrackMode(eMode)

DTV_STATUS_T RHAL_AUDIO_SendConfig(HAL_AUDIO_CONFIG_COMMAND_T *audioConfig);
DTV_STATUS_T RHAL_AUDIO_PrivateInfo(HAL_AUDIO_RPC_PRIVATEINFO_PARAMETERS_T *parameter, HAL_AUDIO_RPC_PRIVATEINFO_RETURNVAL_T *ret);

DTV_STATUS_T RHAL_AUDIO_SetAudioSpectrumData(HAL_AUDIO_SPECTRUM_CFG_T *config);
DTV_STATUS_T RHAL_AUDIO_SetAudioAuthorityKey(SLONG customer_key);
DTV_STATUS_T RHAL_AUDIO_SetAudioCapability(AUDIO_CAP cap);
DTV_STATUS_T RHAL_AUDIO_SetModelHash(char *model_hash);
DTV_STATUS_T RHAL_AUDIO_SetAVSyncOffset(HAL_AUDIO_HDMI_OUT_VSDB_DATA_T *info);
DTV_STATUS_T RHAL_AUDIO_SetVolumeWithFade(HAL_AUDIO_VOLUME_T volume, UINT8 duration, UINT8 easing);

DTV_STATUS_T RHAL_AUDIO_SetDTSDecoderParam(HAL_AUDIO_DTS_DEC_PARAM_TYPE paramType, UINT32 paramValue);
DTV_STATUS_T RHAL_AUDIO_SetTruVolume(BOOLEAN para, HAL_AUDIO_TRUVOLUME_MODE_T* pTruVolume);
DTV_STATUS_T RHAL_AUDIO_GetESInfo(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_ES_INFO_T *pAudioESInfo);

/* DAP API */
DTV_STATUS_T RHAL_AUDIO_SetDAPParam(HAL_AUDIO_DAP_PARAM_TYPE paramType, void *paramValue, int paramValueSize);
DTV_STATUS_T RHAL_AUDIO_GetDAPParam(HAL_AUDIO_DAP_PARAM_TYPE paramType, void *paramValue, int paramValueSize);
DTV_STATUS_T RHAL_AUDIO_SetDAPTuningData(void *pData, int dataBytes);

DTV_STATUS_T RHAL_AUDIO_VX_SetParameter(HAL_AUDIO_VX_PARAM_TYPE_T param_type, void* value);
DTV_STATUS_T RHAL_AUDIO_VX_GetParameter(HAL_AUDIO_VX_PARAM_TYPE_T param_type, void* value);

DTV_STATUS_T RHAL_AUDIO_TruvolumeHD_SetParameter(HAL_AUDIO_TRUVOLUMEHD_PARAM_TYPE_T param_type, void* value);
DTV_STATUS_T RHAL_AUDIO_TruvolumeHD_GetParameter(HAL_AUDIO_TRUVOLUMEHD_PARAM_TYPE_T param_type, void* value);

/**
Set OTT Mode Dolby MS12 v1.x or v2.x
**/
DTV_STATUS_T RHAL_AUDIO_SetDolbyOTTMode(BOOLEAN bIsOTTEnable, BOOLEAN bIsATMOSLockingEnable);

/* */
DTV_STATUS_T RHAL_AUDIO_SetAnalogAdjDB(HAL_AUDIO_AREA_TYPE_T area_type);

/* AC-4 decoder */
/* AC-4 Auto Presenetation Selection. Refer to "Selection using system-level preferences" of "Dolby MS12 Multistream Decoder Implementation integration manual" */
/* When Auto Presentation selection is set during decoding,
   if setting value is identical with the value already set, audio should be decoded without disconnecting and noise.
   if setting value is different with the value already set, audio can be disconnected withtout noise. */
/* ISO 639-1 or ISO 639-2 can be used as Language Code type. */
/* Language Code type is decided by enCodetype(HAL_AUDIO_LANG_CODE_TYPE_T) */
/* ISO 639-1 code is defined upper 2byte of UINT32 and rest 2byte includes 0 example : 'e''n'00 */
/* ISO 639-2 code is defined upper 3byte of UINT32 and rest 1byte includes 0 example : 'e''n''g'0 */
DTV_STATUS_T RHAL_AUDIO_AC4_SetAutoPresentationFirstLanguage(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_LANG_CODE_TYPE_T enCodeType, UINT32 firstLang);     // default : none
DTV_STATUS_T RHAL_AUDIO_AC4_SetAutoPresentationSecondLanguage(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_LANG_CODE_TYPE_T enCodeType, UINT32 secondLang);   // default : none
DTV_STATUS_T RHAL_AUDIO_AC4_SetAutoPresentationADMixing(HAL_AUDIO_ADEC_INDEX_T adecIndex, BOOLEAN bIsEnable);                // default: FALSE
DTV_STATUS_T RHAL_AUDIO_AC4_SetAutoPresentationADType(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_AC4_AD_TYPE_T enADType);   // default : 'VI'
DTV_STATUS_T RHAL_AUDIO_AC4_SetAutoPresentationPrioritizeADType(HAL_AUDIO_ADEC_INDEX_T adecIndex, BOOLEAN bIsEnable);        // default : FALSE
DTV_STATUS_T RHAL_AUDIO_AC4_SetPresentationGroupIndex(HAL_AUDIO_ADEC_INDEX_T adecIndex, UINT32 pres_group_idx);

/* AC-4 Dialogue Enhancement */
/* This Enhancement Gain just affect to AC-4 codec and DAP Enhancement amount does not affect AC-4 codec.
   When AC-4 Dialog Enhancement Gain is set during decoding,
   if setting value is identical with the value already set, audio should be decoded without disconnecting and noise.
   if setting value is different with the value already set, audio should be decoded without disconnecting and noise. */
DTV_STATUS_T RHAL_AUDIO_AC4_SetDialogueEnhancementGain(HAL_AUDIO_ADEC_INDEX_T adecIndex, UINT8 dialEnhanceGain); //Gain should be 0~12 in dB, default : 0

DTV_STATUS_T RHAL_AUDIO_SoundConfig_Data(UINT32* pData1, UINT32 size1, UINT32* pData2, UINT32 size2);
DTV_STATUS_T RHAL_AUDIO_SoundEffect_Level(UINT32 param1, UINT32 param2, UINT32 param3);
DTV_STATUS_T RHAL_AUDIO_SetLowLatencyMode(BOOLEAN bOnOff);
DTV_STATUS_T RHAL_AUDIO_SetAECReference_Source(HAL_AUDIO_AEC_SOURCE_T aec_source);

/* AI Sound Related */
DTV_STATUS_T RHAL_AUDIO_AISound_Enable(BOOLEAN bOnOff);
DTV_STATUS_T RHAL_AUDIO_BTMic_Enable(BOOLEAN bOnOff);


/*
    Below RSDEC copy from rhal sdec define (hal_src/libkadaptor/inc/kadp_sdec.h)
*/
typedef enum
{
  RSDEC_ENUM_TYPE_DEST_VDEC0,
  RSDEC_ENUM_TYPE_DEST_VDEC1,
  RSDEC_ENUM_TYPE_DEST_ADEC0,
  RSDEC_ENUM_TYPE_DEST_ADEC1,
  RSDEC_ENUM_TYPE_DEST_SUBTITLE,
  RSDEC_ENUM_TYPE_DEST_TELETEXT,
  RSDEC_ENUM_TYPE_DEST_TSO,    // for Dump TS

  /* To consist with kadp_sdec_connect in ANDROIDTV, extend partial enum to fit SINA artitecture */
  RSDEC_ENUM_TYPE_DEST_STC0,
  RSDEC_ENUM_TYPE_DEST_STC1,
  RSDEC_ENUM_TYPE_DEST_ISDB_CC,
  RSDEC_ENUM_TYPE_DEST_ISDB_CC_SUPERIMPOSE,
  /*---------------------------------------------------------------------------------------------*/
} RSDEC_ENUM_TYPE_DEST_T ;

typedef struct
{
// physical address of RINGBUFFER_HEADER, we have set each column in ringbuffer header in advance (convert to big endian)
    UINT32 bsRingHeader ;    // physical address of BitStream RingBuffer header (RINGBUFFER_HEADER)
    UINT32 ibRingHeader ;    // physical address of InBand RingBuffer header (RINGBUFFER_HEADER)
    UINT32 refClock ;        // physical address of REFCLOCK
    SINT32 bsHeaderSize;
    SINT32 ibHeaderSize;
    SINT32 refClockHeaderSize;

} DEMUX_STRUCT_CONNECT_RESULT_T ;

/*
    Below RSDEC copy from rhal sdec define (hal_src/libkadaptor/inc/kdrv_sdec.h)
*/

typedef enum {
	DEMUX_PRIVATEINFO_NONE_ = 0,
	DEMUX_PRIVATEINFO_NOTIFY_FLUSHED_ = 1,      /* demux will send new segment and decode command to inband queue after audio/video decoder is flushed */
	DEMUX_PRIVATEINFO_AUDIO_FORMAT_ = 2,        /* new format, first long data is AUDIO_DEC_TYPE, the others are privateInfo[0~7]*/
	DEMUX_PRIVATEINFO_VIDEO_DECODE_MODE_ = 3,   /* video decoder mode : DECODE_MODE*/
	DEMUX_PRIVATEINFO_VIDEO_FREERUN_THRESHOLD_ = 4,	 /* Video FreeRun Threshold */
	DEMUX_PRIVATEINFO_PCR_TRACK_ = 5,   /* Pcr Track enable/disable */
	DEMUX_PRIVATEINFO_AUDIO_PAUSE_ = 6,
	DEMUX_PRIVATEINFO_APVR_PLAYBACK_START_ = 7,  /* To nitify APVR playback start */
} DEMUX_PRIVATEINFO__T ;

#endif /* _HAL_AUDIO_H_ */
