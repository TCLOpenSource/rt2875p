
/******************************************************************************
	Header File Guarder
******************************************************************************/
#ifndef _KHAL_DEMOD_ATV_H_
#define	_KHAL_DEMOD_ATV_H_

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
extern 	int	KHAL_DEMOD_ATV_Initialize(void);
extern 	int	KHAL_DEMOD_ATV_ChangeTransMedia(KHAL_DEMOD_TRANS_SYSTEM_T transSystem);
extern	int	KHAL_DEMOD_ATV_SetDemod(KHAL_DEMOD_ANALOG_CONFIG_T paramStruct);
extern	int	KHAL_DEMOD_ATV_TunePostJob(BOOLEAN *pFinished);
extern 	int	KHAL_DEMOD_ATV_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState);
extern  int	KHAL_DEMOD_ATV_CheckSignalState(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState);
extern 	int	KHAL_DEMOD_ATV_CheckFrequencyOffset(SINT32 *pFreqOffset);
extern  int	KHAL_DEMOD_ATV_GetFWVersion(UINT32 *pFWVersion);
extern  int	KHAL_DEMOD_ATV_DebugMenu(void);

#endif /* End of _KHAL_DEMOD_ATV_H_ */

