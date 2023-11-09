/******************************************************************************
 *   DTV LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *   Copyright(c) 2008 by LG Electronics Inc.
 *
 *   All rights reserved. No part of this work may be reproduced, stored in a
 *   retrieval system, or transmitted by any means without prior written
 *   permission of LG Electronics Inc.
 *****************************************************************************/

/** @file demod_lg115x_dvb.h
 *
 *  DVB T/C Demod.
 *
 *  @author		Jeongpil Yun(jeongpil.yun@lge.com)
 *  @version	0.1
 *  @date		2010.01.13
 *  @see
 */

/******************************************************************************
	Header File Guarder
******************************************************************************/
#ifndef _KHAL_DEMOD_ATSC3_H_
#define	_KHAL_DEMOD_ATSC3_H_

#include "khal_demod_common.h"

/******************************************************************************
	Control Constants
******************************************************************************/
//#define FPGA_DEMOD_TEST

/******************************************************************************
	File Inclusions
******************************************************************************/

/******************************************************************************
 	Constant Definitions
******************************************************************************/

/******************************************************************************
	Macro Definitions
******************************************************************************/

/******************************************************************************
	Type Definitions
******************************************************************************/

/******************************************************************************
	Function Declaration
******************************************************************************/
extern	UINT32 	KHAL_DEMOD_ATSC3_Probe(UINT8 portI2C);
extern 	int 	KHAL_DEMOD_ATSC3_Initialize(void);
extern 	int 	KHAL_DEMOD_ATSC3_ChangeTransSystem(KHAL_DEMOD_TRANS_SYSTEM_T transSystem);ct);
extern	int		KHAL_DEMOD_ATSC3_TunePostJob(BOOLEAN *pFinished);
extern 	int 	KHAL_DEMOD_ATSC3_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState);
extern 	int 	KHAL_DEMOD_ATSC3_CheckSpecialData(KHAL_DEMOD_SPECDATA_ATSC3_T *pSpecATSC3);
extern 	int 	KHAL_DEMOD_ATSC3_CheckFrequencyOffset(SINT32 *pFreqOffset);
extern 	int 	KHAL_DEMOD_ATSC3_GetFWVersion(UINT32 *pFWVersion);
extern 	int 	KHAL_DEMOD_ATSC3_GetSQI(UINT8 *pSQI);
extern 	int 	KHAL_DEMOD_ATSC3_GetPacketError(UINT32 *pPacketError);
extern 	int 	KHAL_DEMOD_ATSC3_GetBER(UINT32 *pBER);
extern 	int 	KHAL_DEMOD_ATSC3_GetAGC(UINT32 *pAGC);
extern 	int 	KHAL_DEMOD_ATSC3_GetSNR(UINT32 *pSNR);
extern 	int 	KHAL_DEMOD_ATSC3_ControlOutput(BOOLEAN bEnableOutput);
extern 	int 	KHAL_DEMOD_ATSC3_ControlTSMode(BOOLEAN bIsSerial);
extern	int		KHAL_DEMOD_ATSC3_CheckSignalStatus(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState);
extern  int	    KHAL_DEMOD_ATSC3_Monitor(KHAL_DEMOD_LOCK_STATE_T *pLockState);
extern 	int 	KHAL_DEMOD_ATSC3_DebugMenu(void);
extern  int 	KHAL_DEMOD_ATSC3_Get_MPLP_Info(KHAL_DEMOD_ATSC3_MULTI_PLP_ID_T *pPlpParams, KHAL_DEMOD_ATSC3_MULTI_PLP_ID_SEL_T plpSel);
extern  int 	KHAL_DEMOD_ATSC3_PLP_Select(UINT8 plp0_id, UINT8 plp1_id, UINT8 plp2_id, UINT8 plp3_id);
extern  int 	KHAL_DEMOD_ATSC3_SetDemodExpand(KHAL_DEMOD_ATSC3_SET_PARAM_T paramStruct, UINT8 plp0_id, UINT8 plp1_id, UINT8 plp2_id, UINT8 plp3_id);
#endif /* End of _KHAL_DEMOD_ATSC3_H_ */

