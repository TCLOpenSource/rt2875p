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
#ifndef _KHAL_DEMOD_ISDB_H_
#define	_KHAL_DEMOD_ISDB_H_

#include "khal_demod_common.h"



/******************************************************************************
	Control Constants
******************************************************************************/


/******************************************************************************
	File Inclusions
******************************************************************************/




/******************************************************************************
 	Constant Definitions
******************************************************************************/

/******************************************************************************
	Macro Definitions
******************************************************************************/

#define	__F__		__FUNCTION__
#define	__L__		__LINE__
/******************************************************************************
	Type Definitions
******************************************************************************/
/*typedef enum KHAL_DEMOD_ISDB_TU_FLAG
{
	KHAL_DEMOD_ISDB_TU_FLAG_START    = 0x90,
	KHAL_DEMOD_ISDB_TU_FLAG_ING,
	KHAL_DEMOD_ISDB_TU_FLAG_STOP,
	KHAL_DEMOD_ISDB_TU_FLAG_FINISH,

	TU_FLAG_UNKNOWN 	= 0xF0
} KHAL_DEMOD_ISDB_TU_FLAG_T;*/
/**
 * structure for strength and quality scaling.
*/

/******************************************************************************
	Function Declaration
******************************************************************************/
extern  UINT32	KHAL_DEMOD_VQI_ISDBT_Probe(struct i2c_client *client);
extern  int		KHAL_DEMOD_VQI_ISDBT_Initialize(void);
extern  int		KHAL_DEMOD_VQI_ISDBT_ChangeTransSystem(KHAL_DEMOD_TRANS_SYSTEM_T transSystem);
extern  int  	KHAL_DEMOD_VQI_ISDBT_SetDemod(KHAL_DEMOD_ISDBT_SET_PARAM_T paramStruct);
extern  int		KHAL_DEMOD_VQI_ISDBT_TunePostJob(BOOLEAN *pFinished);
extern 	int 	KHAL_DEMOD_VQI_ISDBT_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState);
extern 	int 	KHAL_DEMOD_VQI_ISDBT_CheckSpecialData(KHAL_DEMOD_SPECDATA_ISDBT_T *pSpecISDB);
extern	int		KHAL_DEMOD_VQI_ISDBT_CheckSignalStatus(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState);
extern	int 	KHAL_DEMOD_VQI_ISDBT_CheckFrequencyOffset(SINT32 *pFreqOffset);
extern 	int 	KHAL_DEMOD_VQI_ISDBT_ControlTSMode(BOOLEAN bIsSerial);
extern	int		KHAL_DEMOD_VQI_ISDBT_ControlOutput(BOOLEAN bEnableOutput);
extern  int	    KHAL_DEMOD_VQI_ISDBT_Monitor(KHAL_DEMOD_LOCK_STATE_T *pLockState);
extern  int     KHAL_DEMOD_VQI_ISDBT_GetFWVersion(UINT32 *pFWVersion);
extern  BOOLEAN KHAL_DEMOD_GetEmergencyAlertFlagStatus(void);
extern 	int 	KHAL_DEMOD_ISDBT_DebugMenu(void);

#endif /* End of _DEMOD_LG115X_ISDB_H_ */
