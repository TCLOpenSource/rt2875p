/******************************************************************************
 *   DTV LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *   Copyright(c) 2008 by LG Electronics Inc.
 *
 *   All rights reserved. No part of this work may be reproduced, stored in a
 *   retrieval system, or transmitted by any means without prior written
 *   permission of LG Electronics Inc.
 *****************************************************************************/

/** @file demod_lg115x_vq.h
 *
 *  Brief description.
 *  Detailed description starts here.
 *
 *  @author		jeongpil.yun(jeongpil.yun@lge.com)
 *  @version	0.1
 *  @date		2010.01.13
 *  @note
 *  @see
 */

/******************************************************************************
	Header File Guarder
******************************************************************************/
#ifndef _KHAL_DEMOD_VQI_H_
#define	_KHAL_DEMOD_VQI_H_

#include "khal_demod_common.h"



/******************************************************************************
	Control Constants
******************************************************************************/


/******************************************************************************
	File Inclusions
******************************************************************************/


//#define	GBB_QV_I2C_ID_DTV 	0XB2	// LGDT1150-VSB/QAM   // jeongpil.yun

/******************************************************************************
 	Constant Definitions
******************************************************************************/
//#define FPGA_DEMOD_TEST

/******************************************************************************
	Macro Definitions
******************************************************************************/
/******************************************************************************
	Type Definitions
******************************************************************************/
/**
 * GBB RF mode
 */
typedef enum
{
	GBB_AIR_MODE = 0,				///< mode type is air band
	GBB_CABLE_MODE = 1				///< mode type is cable band
} GBB_VQI_MODE_T;

/**
 * GBB Cable channel mode
 */
typedef enum
{
	GBB_STD_BAND = 0,				///< frequency band is STD
	GBB_IRC_BAND,					///< frequency band is IRC
	GBB_HRC_BAND					///< frequency band is HRC
} GBB_VQI_BAND_T;

/**
 * GBB  channel table
 */
typedef struct
{
	UINT8	chanIndex;				///< channel index
	UINT32	freqCenter;				///< center frequency
	UINT8	count;					///< channel count
	UINT16  offset;					///< frequency offset
} GBB_VQI_TABLE_CHANNEL_T;
/*Need check*/
/**
 * structure for strength and quality scaling.
*/
/******************************************************************************
	Function Declaration
******************************************************************************/

extern  UINT32	KHAL_DEMOD_VQI_Probe(struct i2c_client *client);
extern  int		KHAL_DEMOD_VQI_Initialize(void);
extern  int		KHAL_DEMOD_VQI_ChangeTransSystem(KHAL_DEMOD_TRANS_SYSTEM_T transSystem);
extern  int  	KHAL_DEMOD_VQI_ATSC_SetDemod(KHAL_DEMOD_ATSC_SET_PARAM_T paramStruct);
extern  int  	KHAL_DEMOD_VQI_ISDBT_SetDemod(KHAL_DEMOD_ISDBT_SET_PARAM_T paramStruct);
extern  int		KHAL_DEMOD_VQI_VSB_TunePostJob(BOOLEAN *pFinished);
extern  int		KHAL_DEMOD_VQI_QAM_TunePostJob(BOOLEAN *pFinished);
extern  int		KHAL_DEMOD_VQI_ISDBT_TunePostJob(BOOLEAN *pFinished);
extern 	int 	KHAL_DEMOD_VQI_VSB_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState);
extern 	int 	KHAL_DEMOD_VQI_QAM_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState);
extern 	int 	KHAL_DEMOD_VQI_ISDBT_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState);
extern 	int 	KHAL_DEMOD_VQI_QAM_CheckSpecialData(KHAL_DEMOD_SPECDATA_QAM_T *pSpecQAM);
extern	int		KHAL_DEMOD_VQI_CheckSignalStatus(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState);
extern	int 	KHAL_DEMOD_VQI_CheckFrequencyOffset(SINT32 *pFreqOffset);
extern 	int 	KHAL_DEMOD_VQI_ControlTSMode(BOOLEAN bIsSerial);
extern	int		KHAL_DEMOD_VQI_ControlOutput(BOOLEAN bEnableOutput);
extern 	int 	KHAL_DEMOD_VQI_DebugMenu(void);
#endif /* End of _DEMOD_LG115X_VQI_H_ */
