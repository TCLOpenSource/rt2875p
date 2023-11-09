/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author danielwei@realtek.com
 *
 *****************************************************************************/
#ifndef WEBOS_HAL_SRC_DEMOD_RTK_DEMOD_COMMON_H_
#define WEBOS_HAL_SRC_DEMOD_RTK_DEMOD_COMMON_H_
//#include <kadp_DTVTunerControl.h>
//#include <kadp_log.h>
//#include <rhal_sdec.h>
//#include <kadp_rhal_vrpcdev.h>
//#ifdef RTK_CHECK_DEADLOCK_THR
//#include <rtk_deadlock_stub_chk.h>
//#endif

#include "khal_demod_common.h"


#ifdef __cplusplus
extern "C" {
#endif

TV_SYSTEM trans_TV_SYSTEM(KHAL_DEMOD_TRANS_SYSTEM_T transSystem, KHAL_DEMOD_CHANNEL_BW_T eChannelBW);
TV_SYSTEM trans_ATV_SYSTEM(KHAL_DEMOD_TRANS_SYSTEM_T transSystem, KHAL_DEMOD_AUDIO_SIF_SOUNDSYSTEM_T SIFFormat);
int trans_Hierarchy_Translate(int value, int blgeflag);
int trans_ConstelCode_Translate(int value, int blgeflag);
int trans_CarrierMode_Translate(int value, int blgeflag);
int trans_GuardCode_Translate(int value, int blgeflag);
int trans_CodeRate_Translate(int value, int blgeflag);
//for DVBS

int DVBS_trans_codeRate_Translate(int codeRate, BOOLEAN blgeflag);
int DVBS_trans_constellation_Translate(int constellation, BOOLEAN blgeflag);
//forDTMB
int DTMB_trans_carrierMode_Translate(int carrierMode, BOOLEAN blgeflag);
int DTMB_trans_guardInterval_Translate(int value1, int value2, BOOLEAN blgeflag);
int DTMB_trans_codeRate_Translate(int codeRate, BOOLEAN blgeflag);
int DTMB_trans_constellation_Translate(int constellation, BOOLEAN blgeflag);

#ifdef __cplusplus
}
#endif

#endif  // WEBOS_HAL_SRC_DEMOD_RTK_DEMOD_COMMON_H_
