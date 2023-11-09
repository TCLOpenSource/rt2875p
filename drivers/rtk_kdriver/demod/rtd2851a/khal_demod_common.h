/******************************************************************************
 *   DTV LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *   Copyright(c) 1999 by LG Electronics Inc.
 *
 *   All rights reserved. No part of this work may be reproduced, stored in a
 *   retrieval system, or transmitted by any means without prior written
 *   permission of LG Electronics Inc.
 *****************************************************************************/

/** @file hal_demod.h
 *
 *  HAL 함수 header 파일.
 *
 *
 *  @author	Harrison(hyungkyu.noh@lge.com)
 *  @reviser
 *  @version	1.1
 *  @date		2013.05.27
 *  @note
 *  @see
 */

/******************************************************************************
 	Header File Guarder
******************************************************************************/
#ifndef _KHAL_DEMOD_COMMON_H_
#define _KHAL_DEMOD_COMMON_H_

/******************************************************************************
 #include 파일들 (File Inclusions)
******************************************************************************/
//#include <linux/types.h>
//#include <linux/printk.h>
//#include <linux/delay.h>
//#include <linux/string.h>
//#include "webos_dvb_common_adapter.h"
#include "khal_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 	상수 정의(Constant Definitions)
******************************************************************************/

/******************************************************************************
    매크로 함수 정의 (Macro Definitions)
******************************************************************************/
#ifndef TRUE
#define TRUE                    (1)
#endif

#ifndef FALSE
#define FALSE                   (0)
#endif

#ifndef ON_STATE
#define ON_STATE                (1)
#endif

#ifndef OFF_STATE
#define OFF_STATE               (0)
#endif

#ifndef ON
#define ON                      (1)
#endif

#ifndef OFF
#define OFF                     (0)
#endif

#ifndef NULL
#define NULL                    ((void *)0)
#endif


/******************************************************************************
	형 정의 (Type Definitions)
******************************************************************************/
typedef enum {
	/* TERRESTRIAL */
	KHAL_DEMOD_TRANS_SYS_VSB		= 0x00,			/* 5 bit : Don't exceed */
	KHAL_DEMOD_TRANS_SYS_DVBT,						/* 5 bit : Don't exceed */
	KHAL_DEMOD_TRANS_SYS_DVBT2,						/* 5 bit : Don't exceed */
	KHAL_DEMOD_TRANS_SYS_DTMB,						/* 5 bit : Don't exceed */
	KHAL_DEMOD_TRANS_SYS_ISDBT,						/* 5 bit : Don't exceed */

	/* CABLE */
	KHAL_DEMOD_TRANS_SYS_DVBC,						/* 5 bit : Don't exceed */
	KHAL_DEMOD_TRANS_SYS_DVBC2,						/* 5 bit : Don't exceed */
	KHAL_DEMOD_TRANS_SYS_QAM,						/* 5 bit : Don't exceed */
	KHAL_DEMOD_TRANS_SYS_ISDBC,						/* 5 bit : Don't exceed */

	/* SATELLITE */
	KHAL_DEMOD_TRANS_SYS_DVBS,						/* 5 bit : Don't exceed */
	KHAL_DEMOD_TRANS_SYS_DVBS2,						/* 5 bit : Don't exceed */
	KHAL_DEMOD_TRANS_SYS_BS,						/* 4 bit : Don't exceed */
	KHAL_DEMOD_TRANS_SYS_CS,						/* 5 bit : Don't exceed */

	/* ANALOG */
	KHAL_DEMOD_TRANS_SYS_NTSC,						/* 5 bit : Don't exceed */
	KHAL_DEMOD_TRANS_SYS_PAL,						/* 5 bit : Don't exceed */

	KHAL_DEMOD_TRANS_SYS_END,						/* 5 bit : Don't exceed */

	KHAL_DEMOD_TRANS_SYS_UNKNOWN	= 0x1F,			/* 5 bit : Don't exceed */
} KHAL_DEMOD_TRANS_SYSTEM_T;

/**
 * tune mode.
 */
typedef enum {
	KHAL_DEMOD_TUNE_NORMAL					= 0x10,		/* 0001 XXXX */

	KHAL_DEMOD_TUNE_MANUAL					= 0X20,

	/*	---------- SCAN 	---------------------	*/
	KHAL_DEMOD_TUNE_SCAN					= 0x30,		/* 0010 XXXX */

	KHAL_DEMOD_TUNE_SCAN_START,				/* ATV : Full Scan */

	/*	---------- SPECIFIC 	---------------------	*/
	KHAL_DEMOD_TUNE_SPECIFIC					= 0x40,		/* 0100 XXXX */
	KHAL_DEMOD_TUNE_SPEC_DVBT_HMLP,			/* DVBT : Hierarchy Mode */
	KHAL_DEMOD_TUNE_SPEC_DVBC_FIXED_DATA,	/* DVBC : Use Fixed NIT data */


	/*	---------- UNKNOWN 	---------------------	*/
	KHAL_DEMOD_TUNE_UNKNOWN					= 0x80,		/* 1000 XXXX */

	/*	---------- MASK 	---------------------	*/
	KHAL_DEMOD_TUNE_MODE_MASK					= 0xF0,		/* 1111 XXXX */


} KHAL_DEMOD_TUNE_MODE_T;

/**
 * Lock state.
 */
typedef enum {
	KHAL_DEMOD_LOCK_OK    = 0x00,
	KHAL_DEMOD_LOCK_FAIL,
	KHAL_DEMOD_LOCK_UNSTABLE,
	KHAL_DEMOD_LOCK_WEAK = 0x10,
	KHAL_DEMOD_LOCK_POOR,
	KHAL_DEMOD_LOCK_ATV_PROGRESS,
	KHAL_DEMOD_LOCK_NEVER_LOCK,
	KHAL_DEMOD_LOCK_UNKNOWN = 0x80
} KHAL_DEMOD_LOCK_STATE_T;

/**
 * channel bandwidth
 */
typedef enum {
	KHAL_DEMOD_CH_BW_8M	= 0,
	KHAL_DEMOD_CH_BW_7M,
	KHAL_DEMOD_CH_BW_6M,
	KHAL_DEMOD_CH_BW_UNKNOWN
} KHAL_DEMOD_CHANNEL_BW_T;

/**
 * TPS : constellation.
 */
typedef enum 	/* 4 bit */
{
	KHAL_DEMOD_TPS_CONST_QPSK 		= 0x00,
	KHAL_DEMOD_TPS_CONST_DQPSK,
	KHAL_DEMOD_TPS_CONST_QAM_4NR,
	KHAL_DEMOD_TPS_CONST_QAM_4,
	KHAL_DEMOD_TPS_CONST_PSK_8,
	KHAL_DEMOD_TPS_CONST_VSB_8,
	KHAL_DEMOD_TPS_CONST_QAM_16,
	KHAL_DEMOD_TPS_CONST_QAM_32,
	KHAL_DEMOD_TPS_CONST_QAM_64,
	KHAL_DEMOD_TPS_CONST_QAM_128,
	KHAL_DEMOD_TPS_CONST_QAM_256,

	KHAL_DEMOD_TPS_CONST_END,

	KHAL_DEMOD_TPS_CONST_UNKNOWN 	= 0x0F
} KHAL_DEMOD_TPS_CONSTELLATION_T;

/**
 * TPS : code rate.
 */
typedef enum 	/* 4 bit */
{
	KHAL_DEMOD_TPS_CODE_1_2 		= 0x00,
	KHAL_DEMOD_TPS_CODE_1_3,
	KHAL_DEMOD_TPS_CODE_1_4,
	KHAL_DEMOD_TPS_CODE_2_3,
	KHAL_DEMOD_TPS_CODE_3_4,
	KHAL_DEMOD_TPS_CODE_2_5,
	KHAL_DEMOD_TPS_CODE_3_5,
	KHAL_DEMOD_TPS_CODE_4_5,
	KHAL_DEMOD_TPS_CODE_5_6,
	KHAL_DEMOD_TPS_CODE_6_7,
	KHAL_DEMOD_TPS_CODE_7_8,
	KHAL_DEMOD_TPS_CODE_8_9,
	KHAL_DEMOD_TPS_CODE_9_10,

	KHAL_DEMOD_TPS_CODE_END,

	KHAL_DEMOD_TPS_CODE_UNKNOWN 	= 0x0F
} KHAL_DEMOD_TPS_CODERATE_T;

/**
 * TPS : guard interval.
 */
typedef enum 	/* 4 bit */
{
	KHAL_DEMOD_TPS_GUARD_1_4 		= 0x00,
	KHAL_DEMOD_TPS_GUARD_1_8,
	KHAL_DEMOD_TPS_GUARD_1_9,
	KHAL_DEMOD_TPS_GUARD_1_16 ,
	KHAL_DEMOD_TPS_GUARD_1_32,
	KHAL_DEMOD_TPS_GUARD_1_128,
	KHAL_DEMOD_TPS_GUARD_19_128,
	KHAL_DEMOD_TPS_GUARD_19_256,

	KHAL_DEMOD_TPS_GUARD_420_C,
	KHAL_DEMOD_TPS_GUARD_420_V,
	KHAL_DEMOD_TPS_GUARD_595,
	KHAL_DEMOD_TPS_GUARD_945_C,
	KHAL_DEMOD_TPS_GUARD_945_V,

	KHAL_DEMOD_TPS_GUARD_END,

	KHAL_DEMOD_TPS_GUARD_UNKNOWN 	= 0x0F
} KHAL_DEMOD_TPS_GUARD_INTERVAL_T;

/**
 * TPS : carrier mode.
 */
typedef enum 	/* 4 bit */
{
	KHAL_DEMOD_TPS_CARR_1K 			= 0x00,
	KHAL_DEMOD_TPS_CARR_2K,
	KHAL_DEMOD_TPS_CARR_4K,
	KHAL_DEMOD_TPS_CARR_8K,
	KHAL_DEMOD_TPS_CARR_16K,
	KHAL_DEMOD_TPS_CARR_32K,

	KHAL_DEMOD_TPS_CARR_SC,
	KHAL_DEMOD_TPS_CARR_MC,

	KHAL_DEMOD_TPS_CARR_END,

	KHAL_DEMOD_TPS_CARR_UNKNOWN 	= 0x0F
} KHAL_DEMOD_TPS_CARRIER_MODE_T;

/**
 * TPS : hierarchy.
 */
typedef enum 	/* 3 bit */
{
	KHAL_DEMOD_TPS_HIERA_NONE 		= 0x00,
	KHAL_DEMOD_TPS_HIERA_1,
	KHAL_DEMOD_TPS_HIERA_2,
	KHAL_DEMOD_TPS_HIERA_4,

	KHAL_DEMOD_TPS_HIERA_END,

	KHAL_DEMOD_TPS_HIERA_UNKNOWN 	= 0x07
} KHAL_DEMOD_TPS_HIERARCHY_T;

/**
 * Audio
 */

typedef enum {
	KHAL_DEMOD_AUDIO_SIF_SYSTEM_BG		= 0x00,
	KHAL_DEMOD_AUDIO_SIF_SYSTEM_I,
	KHAL_DEMOD_AUDIO_SIF_SYSTEM_DK,
	KHAL_DEMOD_AUDIO_SIF_SYSTEM_L,
	KHAL_DEMOD_AUDIO_SIF_SYSTEM_MN,
	KHAL_DEMOD_AUDIO_SIF_SYSTEM_LP,
	KHAL_DEMOD_AUDIO_SIF_SYSTEM_END,
	KHAL_DEMOD_AUDIO_SIF_SYSTEM_UNKNOWN = 0xf0
} KHAL_DEMOD_AUDIO_SIF_SOUNDSYSTEM_T;


/**
 * signal state.
 */
typedef struct {
	BOOLEAN bSignalValid;		/*	1 bit */
	UINT8   strength;
	UINT8   quality;
	UINT32	packetError;
	UINT32	unBER;			/* unit : e-10 */
	UINT32	unAGC;
	UINT32	unSNR;
} KHAL_DEMOD_SIGNAL_STATE_T;

/**
 * Set Parameter : ATSC.
 */
typedef struct {
	KHAL_DEMOD_TUNE_MODE_T			tuneMode;
	KHAL_DEMOD_TRANS_SYSTEM_T		transSystem;
	KHAL_DEMOD_CHANNEL_BW_T			eChannelBW;
	BOOLEAN							bSpectrumInv;
	KHAL_DEMOD_TPS_CONSTELLATION_T	constellation;
} KHAL_DEMOD_ATSC_SET_PARAM_T;

/**
 * Set Parameter : ISDB.
 */
typedef struct {
	KHAL_DEMOD_TUNE_MODE_T			tuneMode;
	KHAL_DEMOD_TRANS_SYSTEM_T		transSystem;
	KHAL_DEMOD_CHANNEL_BW_T			eChannelBW;
	BOOLEAN							bSpectrumInv;
	KHAL_DEMOD_TPS_CARRIER_MODE_T	carrierMode;
	KHAL_DEMOD_TPS_GUARD_INTERVAL_T	guardInterval;
	KHAL_DEMOD_TPS_CODERATE_T		codeRate;
	KHAL_DEMOD_TPS_CONSTELLATION_T	constellation;
} KHAL_DEMOD_ISDBT_SET_PARAM_T;

/**
 * Set Parameter : DVB-T.
 */
typedef struct {
	KHAL_DEMOD_TUNE_MODE_T			tuneMode;
	KHAL_DEMOD_TRANS_SYSTEM_T		transSystem;
	KHAL_DEMOD_CHANNEL_BW_T			eChannelBW;
	BOOLEAN							bSpectrumInv;
	BOOLEAN							bProfileHP;
	KHAL_DEMOD_TPS_HIERARCHY_T		hierarchy;
	KHAL_DEMOD_TPS_CARRIER_MODE_T	carrierMode;
	KHAL_DEMOD_TPS_GUARD_INTERVAL_T	guardInterval;
	KHAL_DEMOD_TPS_CODERATE_T		codeRate;
	KHAL_DEMOD_TPS_CONSTELLATION_T	constellation;
} KHAL_DEMOD_DVBT_SET_PARAM_T;

/**
 * Set Parameter : DVB-T2.
 */
typedef struct {
	KHAL_DEMOD_TUNE_MODE_T                tuneMode;
	KHAL_DEMOD_TRANS_SYSTEM_T             transSystem;
	KHAL_DEMOD_CHANNEL_BW_T               eChannelBW;
	BOOLEAN                              bSpectrumInv;
	KHAL_DEMOD_TPS_CARRIER_MODE_T         carrierMode;
	KHAL_DEMOD_TPS_GUARD_INTERVAL_T       guardInterval;
	KHAL_DEMOD_TPS_CODERATE_T             codeRate;
	KHAL_DEMOD_TPS_CONSTELLATION_T        constellation;
	UINT8                                unPLP;
} KHAL_DEMOD_DVBT2_SET_PARAM_T;

/**
 * Set Parameter : DVB-C.
 */
typedef struct {
	KHAL_DEMOD_TUNE_MODE_T			tuneMode;
	KHAL_DEMOD_TRANS_SYSTEM_T		transSystem;
	KHAL_DEMOD_CHANNEL_BW_T			eChannelBW;
	UINT32							frequency;
	UINT16 							symbolRate;
	BOOLEAN							bSpectrumInv;
	KHAL_DEMOD_TPS_CONSTELLATION_T	constellation;
} KHAL_DEMOD_DVBC_SET_PARAM_T;

/**
 * Set Parameter : DVB-S.
 */
typedef struct {
	KHAL_DEMOD_TUNE_MODE_T          tuneMode;
	KHAL_DEMOD_TRANS_SYSTEM_T       transSystem;
	BOOLEAN							bSpectrumInv;
	UINT32                          symbolRate;
	KHAL_DEMOD_TPS_CODERATE_T       codeRate;
	KHAL_DEMOD_TPS_CONSTELLATION_T  constellation;
	UINT32                          freqKHz;
	BOOLEAN                         bIsBlind_search;
} KHAL_DEMOD_DVBS_SET_PARAM_T;

/**
 * Set Parameter : DVB-S2.
 */
typedef struct {
	KHAL_DEMOD_TUNE_MODE_T          tuneMode;
	KHAL_DEMOD_TRANS_SYSTEM_T       transSystem;
	BOOLEAN                         bSpectrumInv;
	UINT32                          symbolRate;
	KHAL_DEMOD_TPS_CODERATE_T       codeRate;
	KHAL_DEMOD_TPS_CONSTELLATION_T  constellation;
	UINT32                          freqKHz;
	BOOLEAN                         bIsBlind_search;
} KHAL_DEMOD_DVBS2_SET_PARAM_T;

/**
 * Set Parameter : DTMB.
 */
typedef struct {
	KHAL_DEMOD_TUNE_MODE_T			tuneMode;
	KHAL_DEMOD_TRANS_SYSTEM_T		transSystem;
	KHAL_DEMOD_CHANNEL_BW_T			eChannelBW;
	BOOLEAN                         bM720;			//interleave
	KHAL_DEMOD_TPS_CARRIER_MODE_T	carrierMode ;
	KHAL_DEMOD_TPS_GUARD_INTERVAL_T	guardInterval;
	KHAL_DEMOD_TPS_CODERATE_T		codeRate ;
	KHAL_DEMOD_TPS_CONSTELLATION_T	constellation;
} KHAL_DEMOD_DTMB_SET_PARAM_T;

/**
* Set Parameter : ANALOG.
*/
typedef struct {
	KHAL_DEMOD_TRANS_SYSTEM_T 		transSystem;
	BOOLEAN							bSpectrmInv;
	UINT32							ifFrq;
} KHAL_DEMOD_ANALOG_SET_PARAM_T;

/**
 * Special data : VSB.
 */
typedef struct {
	BOOLEAN							bCoChannel;		/* 1 bit */
	KHAL_DEMOD_TPS_CONSTELLATION_T	constellation;	/* 4 bit */
} KHAL_DEMOD_SPECDATA_VSB_T;

/**
 * Special data : QAM.
 */
typedef struct {
	BOOLEAN							bSpectrumInv;
	UINT8							cableBand;
	KHAL_DEMOD_TPS_CONSTELLATION_T	constellation;
} KHAL_DEMOD_SPECDATA_QAM_T;

/**
 * Special data : ISDB.
 */
typedef struct {
	BOOLEAN							bSpectrumInv;	/* 1 bit */
	BOOLEAN							bProfileHP;		/* 1 bit */
	KHAL_DEMOD_TPS_HIERARCHY_T		hierarchy;		/* 3 bit */
	KHAL_DEMOD_TPS_CARRIER_MODE_T	carrierMode;
	KHAL_DEMOD_TPS_GUARD_INTERVAL_T guardInterval;

	KHAL_DEMOD_TPS_CODERATE_T		codeRate;
	KHAL_DEMOD_TPS_CONSTELLATION_T	constellation;
} KHAL_DEMOD_SPECDATA_ISDBT_T;

/**
 * Special data : DVB-T.
 */
typedef struct {
	BOOLEAN							bSpectrumInv;		/* 1 bit */
	BOOLEAN							bProfileHP;		/* 1 bit */
	KHAL_DEMOD_TPS_HIERARCHY_T		hierarchy;		/* 3 bit */

	KHAL_DEMOD_TPS_CARRIER_MODE_T	carrierMode;		/* 4 bit */
	KHAL_DEMOD_TPS_GUARD_INTERVAL_T	guardInterval;		/* 4 bit */

	KHAL_DEMOD_TPS_CODERATE_T		codeRate;		/* 4 bit */
	KHAL_DEMOD_TPS_CONSTELLATION_T	constellation;		/* 4 bit */
} KHAL_DEMOD_SPECDATA_DVBT_T;

/**
 * Special data : DVB-T2.
 */
typedef struct {
	BOOLEAN  bSpectrumInv;
	KHAL_DEMOD_TPS_CARRIER_MODE_T    carrierMode;
	KHAL_DEMOD_TPS_GUARD_INTERVAL_T  guardInterval;

	KHAL_DEMOD_TPS_CODERATE_T        codeRate;
	KHAL_DEMOD_TPS_CONSTELLATION_T   constellation;
	UINT8  unPLP;      // For DVBT2 - PLP ID
} KHAL_DEMOD_SPECDATA_DVBT2_T;

/**
 * Special data : DVB-C.
 */
typedef struct {
	BOOLEAN							bSpectrumInv;
	KHAL_DEMOD_TPS_CONSTELLATION_T	constellation;
	UINT16							symbolRate;
} KHAL_DEMOD_SPECDATA_DVBC_T;

/**
 * Special data : DVB-S.
 */
typedef struct {
	BOOLEAN                         bIsDVBS2;        /* 1 bit */
	BOOLEAN                         bSpectrumInv;    /* 1 bit */
	UINT32                          symbolRate;
	KHAL_DEMOD_TPS_CODERATE_T        codeRate;
	KHAL_DEMOD_TPS_CONSTELLATION_T   constellation;
} KHAL_DEMOD_SPECDATA_DVBS_T;

/**
 * Special data : DVB-S2.
 */
typedef struct {
	BOOLEAN                         bIsDVBS2;        /* 1 bit */
	BOOLEAN                         bSpectrumInv;    /* 1 bit */
	UINT32                          symbolRate;
	KHAL_DEMOD_TPS_CODERATE_T        codeRate;
	KHAL_DEMOD_TPS_CONSTELLATION_T   constellation;
} KHAL_DEMOD_SPECDATA_DVBS2_T;

/**
 * Special data : DTMB.
 */
typedef struct {
	BOOLEAN							bM720;			/* 1 bit */
	KHAL_DEMOD_TPS_CARRIER_MODE_T	carrierMode;	/* 4 bit */
	KHAL_DEMOD_TPS_GUARD_INTERVAL_T	guardInterval;	/* 4 bit */
	KHAL_DEMOD_TPS_CODERATE_T		codeRate;		/* 4 bit */
	KHAL_DEMOD_TPS_CONSTELLATION_T	constellation;	/* 4 bit */
} KHAL_DEMOD_SPECDATA_DTMB_T;

/**
 * DVBT2 : PLP.
 */
typedef struct {
	UINT8   PLPCount;                   /* count of multiple TS*/
	UINT8   paPLPID[256];               /* arrary of PLP ID */
} KHAL_MULTI_TS_INFO_T;

/**
*ANALOG
*/
typedef struct {
	UINT32								centerFreq;
	UINT32								tunedFreq;
	BOOLEAN								bSpectrmInv;
	KHAL_DEMOD_TRANS_SYSTEM_T			transSystem;
	KHAL_DEMOD_TUNE_MODE_T				tuneMode;
	KHAL_DEMOD_CHANNEL_BW_T 				channelBW;
	KHAL_DEMOD_AUDIO_SIF_SOUNDSYSTEM_T	audioSystem;

} KHAL_DEMOD_ANALOG_CONFIG_T;


#ifdef __cplusplus
}
#endif

#endif /* _KHAL_AUDIO_H_ */


