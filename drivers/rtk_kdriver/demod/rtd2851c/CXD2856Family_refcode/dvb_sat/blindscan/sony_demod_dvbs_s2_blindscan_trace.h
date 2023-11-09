/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_blindscan_trace.h

          This file provides trace functions definication.
          If you would like to use this trace functions for TuneSRS/BlindScan,
          please enable "SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_ENABLE" definition,
          and implement following functions.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_H
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_H

#include "sony_common.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"

#ifdef SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_ENABLE
/**
 @brief Start trace function.

 This function is called at the beginning of TuneSRS/BlindScan.
 If you have any process to do at this timing, please implement it in this function.
 (i.e. Open output file, etc...)
*/
void sony_demod_dvbs_s2_blindscan_trace_init (void);
/**
 @brief Output power list.

 @param name The event name.
 @param pList Power data list.
*/
void sony_demod_dvbs_s2_blindscan_trace_powerlist (char * name, sony_demod_dvbs_s2_blindscan_power_t * pList);
/**
 @brief Output candidate list.

 @param name The event name.
 @param pList Candidate data list.
*/
void sony_demod_dvbs_s2_blindscan_trace_candlist (char * name, sony_demod_dvbs_s2_blindscan_data_t * pList);
/**
 @brief Start BT sub sequence.

 @param freqKHz Frequency in KHz.
 @param candSR Candidate symbol rate in KSps.
 @param minCandSR Min candidate symbol rate in KSps.
 @param maxCandSR Max candidate symbol rate in KSps.
 @param minSR Min search range symbol rate in KSps.
 @param maxSR Max search range symbol rate in KSps.
*/
void sony_demod_dvbs_s2_blindscan_trace_bt_start (uint32_t freqKHz, uint32_t candSR, uint32_t minCandSR, uint32_t maxCandSR, uint32_t minSR, uint32_t maxSR);
/**
 @brief Finish BT sub sequence.

 @param eventName The result of that BT sub sequence.
*/
void sony_demod_dvbs_s2_blindscan_trace_bt_result (char * eventName);
/**
 @brief RF tune for CS sub sequence.

 @param freqKHz Frequency in KHz.
*/
void sony_demod_dvbs_s2_blindscan_trace_cs_tune (uint32_t freqKHz);
/**
 @brief Start CS sub sequence.

 @param offsetFreqKHz Frequency offset in KHz.
*/
void sony_demod_dvbs_s2_blindscan_trace_cs_start (int32_t offsetFreqKHz);
/**
 @brief Finish CS sub sequence.

 @param coarseSR Detected symbol rate in KSps.
        If symbol rate was not detected coarseSR is 0 or 1.
*/
void sony_demod_dvbs_s2_blindscan_trace_cs_result (uint32_t coarseSR);
/**
 @brief PM sub sequence.

 @param offsetFreqKHz Frequency offset in KHz.
 @param power Power value.
*/
void sony_demod_dvbs_s2_blindscan_trace_pm_power (int32_t offsetFreqKHz, int32_t power);
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_INIT() sony_demod_dvbs_s2_blindscan_trace_init()
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_POWERLIST(A, B) sony_demod_dvbs_s2_blindscan_trace_powerlist(A, B)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CANDLIST(A, B) sony_demod_dvbs_s2_blindscan_trace_candlist(A, B)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_START(A, B, C, D, E, F) sony_demod_dvbs_s2_blindscan_trace_bt_start(A, B, C, D, E, F)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_RESULT(A) sony_demod_dvbs_s2_blindscan_trace_bt_result(A)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CS_TUNE(A) sony_demod_dvbs_s2_blindscan_trace_cs_tune(A)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CS_START(A) sony_demod_dvbs_s2_blindscan_trace_cs_start(A)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CS_RESULT(A) sony_demod_dvbs_s2_blindscan_trace_cs_result(A)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_PM_POWER(A, B) sony_demod_dvbs_s2_blindscan_trace_pm_power(A, B)
#else  /* SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_ENABLE */
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_INIT()
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_POWERLIST(A, B)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CANDLIST(A, B)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_START(A, B, C, D, E, F)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_RESULT(A)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CS_TUNE(A)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CS_START(A)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CS_RESULT(A)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_PM_POWER(A, B)
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_FIN()
#endif /* SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_ENABLE */

#endif /* SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_H */
