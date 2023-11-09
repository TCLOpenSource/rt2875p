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
#ifndef _KHAL_DEMOD_DTMB_H_
#define	_KHAL_DEMOD_DTMB_H_

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


/******************************************************************************
	Function Declaration
******************************************************************************/
extern  UINT32	KHAL_DEMOD_VQI_DTMB_Probe(struct i2c_client *client);
extern  int		KHAL_DEMOD_VQI_DTMB_Initialize(void);
extern  int		KHAL_DEMOD_VQI_DTMB_ChangeTransSystem(KHAL_DEMOD_TRANS_SYSTEM_T transSystem);
extern  int  	KHAL_DEMOD_VQI_DTMB_SetDemod(KHAL_DEMOD_DTMB_SET_PARAM_T paramStruct);
extern  int		KHAL_DEMOD_VQI_DTMB_TunePostJob(BOOLEAN *pFinished);
extern 	int 	KHAL_DEMOD_VQI_DTMB_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState);
extern 	int 	KHAL_DEMOD_VQI_DTMB_CheckSpecialData(KHAL_DEMOD_SPECDATA_DTMB_T *pSpecDTMB);
extern	int		KHAL_DEMOD_VQI_DTMB_CheckSignalStatus(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState);
extern	int 	KHAL_DEMOD_VQI_DTMB_CheckFrequencyOffset(SINT32 *pFreqOffset);
extern 	int 	KHAL_DEMOD_VQI_DTMB_ControlTSMode(BOOLEAN bIsSerial);
extern	int		KHAL_DEMOD_VQI_DTMB_ControlOutput(BOOLEAN bEnableOutput);
extern  int     KHAL_DEMOD_VQI_DTMB_GetFWVersion(UINT32 *pFWVersion);
extern 	int 	KHAL_DEMOD_DTMB_DebugMenu(void);
#endif /* End of _DEMOD_LG115X_ISDB_H_ */
