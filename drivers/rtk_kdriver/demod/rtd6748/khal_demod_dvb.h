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
#ifndef _KHAL_DEMOD_DVB_H_
#define	_KHAL_DEMOD_DVB_H_

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
//COMMON
extern	int 	KHAL_DEMOD_DVB_Probe(UINT8 portI2C);
extern 	int 	KHAL_DEMOD_DVB_Initialize(void);
extern 	int 	KHAL_DEMOD_DVB_ChangeTransMedia(KHAL_DEMOD_TRANS_SYSTEM_T transSystem);
extern	int		KHAL_DEMOD_DVB_OperMode(KHAL_DEMOD_TRANS_SYSTEM_T *pOperMode);
extern 	int 	KHAL_DEMOD_DVB_CheckFrequencyOffset(SINT32 *pFreqOffset);
extern 	int 	KHAL_DEMOD_DVB_GetCellID(UINT16 *pCellID);
extern 	int 	KHAL_DEMOD_DVB_ControlTSMode(BOOLEAN bIsSerial);
extern 	int 	KHAL_DEMOD_DVB_ControlOutput(BOOLEAN bEnableOutput);
extern	int 	KHAL_DEMOD_DVB_PKT_ERR(UINT32 *pPKT_Err_Acu, BOOLEAN bReset);
extern 	int 	KHAL_DEMOD_DVB_GetPacketError(UINT32 *pFecPkerr);
extern 	int 	KHAL_DEMOD_DVB_GetSQI(UINT8 *pSQI);
extern 	int 	KHAL_DEMOD_DVB_GetBER(UINT32 *pBER);
extern 	int 	KHAL_DEMOD_DVB_GetAGC(UINT32 *pAGC);
extern 	int 	KHAL_DEMOD_DVB_GetSNR(UINT32 *pSNR);
extern 	int 	KHAL_DEMOD_DVB_GetFWVersion(UINT32 *pFWVersion);
extern 	int 	KHAL_DEMOD_DVB_DebugMenu(void);
extern  int     KHAL_DEMOD_SetI2C(void *i2c_cli);
extern  int     KHAL_DEMOD_DVB_GetTsClkRate(UINT32 *pTsClkRate);

//DVB-T/T2
extern 	int 	KHAL_DEMOD_DVB_DVBT_SetDemod(KHAL_DEMOD_DVBT_SET_PARAM_T paramStruct);
extern	int		KHAL_DEMOD_DVB_DVBT2_SetDemod(KHAL_DEMOD_DVBT2_SET_PARAM_T paramStruct);
extern 	int 	KHAL_DEMOD_DVB_DVBT_TunePostJob(BOOLEAN *pFinished);
extern 	int 	KHAL_DEMOD_DVB_DVBT_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState);
extern	int		KHAL_DEMOD_DVB_DVBT_GetSignalState(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState);
extern	int		KHAL_DEMOD_DVB_DVBT2_GetSignalState(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState);
extern 	int 	KHAL_DEMOD_DVB_DVBT_CheckSpecialData(KHAL_DEMOD_SPECDATA_DVBT_T *pSpecDVBT);
extern  int		KHAL_DEMOD_DVB_DVBT2_CheckSpecialData(KHAL_DEMOD_SPECDATA_DVBT2_T *pSpecDVBT2);
extern  int		KHAL_DEMOD_DVB_DVBT2_ChangePLP(UINT8 unPLP);
extern  int		KHAL_DEMOD_DVB_DVBT2_GetMultiPLPInfo(KHAL_MULTI_TS_INFO_T *pPlpInfo);

//DVB-C
extern 	int		KHAL_DEMOD_DVB_DVBC_SetDemod(KHAL_DEMOD_DVBC_SET_PARAM_T paramStruct);
extern 	int		KHAL_DEMOD_DVB_DVBC_TunePostJob(BOOLEAN *pFinished);
extern 	int		KHAL_DEMOD_DVB_DVBC_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState);
extern	int 	KHAL_DEMOD_DVB_DVBC_GetSignalState(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState);
extern 	int 	KHAL_DEMOD_DVB_DVBC_CheckSpecialData(KHAL_DEMOD_SPECDATA_DVBC_T *pSpecDVBC);

//DVB-S/S2
extern	int		KHAL_DEMOD_DVB_DVBS_SetDemod(KHAL_DEMOD_DVBS_SET_PARAM_T halDVBSParam);
extern	int		KHAL_DEMOD_DVB_DVBS_TunePostJob(BOOLEAN *pFinished);
extern	int		KHAL_DEMOD_DVB_DVBS2_TunePostJob(BOOLEAN *pFinished);
extern	int		KHAL_DEMOD_DVB_DVBS_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState);
extern	int		KHAL_DEMOD_DVB_DVBS_GetSignalState(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState);
extern	int		KHAL_DEMOD_DVB_DVBS_CheckSpecialData(KHAL_DEMOD_SPECDATA_DVBS_T *pSpecDVBS);
extern	int		KHAL_DEMOD_DVB_DVBS_22Khz_Tone(BOOLEAN b22kOn);
extern	int		KHAL_DEMOD_DVB_DVBS_Send_Diseqc(UINT8* pCmd, UINT8 u8CmdSize);
extern	int		KHAL_DEMOD_DVB_DVBS_Send_Tone_Burst(BOOLEAN bTone1);
extern	int		KHAL_DEMOD_DVB_DVBS_BlindScan_Init(UINT32 StartFreqMhz, UINT32 EndFreqMhz);
extern	int 	KHAL_DEMOD_DVB_DVBS_BlindScan_GetTunerFreq(UINT32 *TunerCenterFreqMhz, UINT32 *TunerCutOffFreqKhz);
extern	int 	KHAL_DEMOD_DVB_DVBS_BlindScan_NextFreq(BOOLEAN *bBlindScanEnd);
extern	int 	KHAL_DEMOD_DVB_DVBS_BlindScan_GetStatus(UINT8 *Status, BOOLEAN *bBlindScanLock);
extern	int 	KHAL_DEMOD_DVB_DVBS_BlindScan_End(void);
extern	int 	KHAL_DEMOD_DVB_DVBS_BlindScan_GetFoundTP(UINT32 *FreqMhz, UINT32 *SymbolRateKhz);
extern	int 	KHAL_DEMOD_DVB_DVBS_GetPacketError(UINT32 *pPacketError);
extern	int 	KHAL_DEMOD_DVB_DVBS_GetSQI(UINT8 *pSQI);
extern	int 	KHAL_DEMOD_DVB_DVBS_GetBER(UINT32 *pBER);
extern	int 	KHAL_DEMOD_DVB_DVBS_GetAGC(UINT32 *pAGC);
extern	int 	KHAL_DEMOD_DVB_DVBS_GetSNR(UINT32 *pSNR);
#endif /* End of _DEMOD_LG115X_DVB_H_ */

