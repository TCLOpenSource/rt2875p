/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_blindscan_subseq_pm.h

          This file provides PM sub sequence for TuneSRS/BlindScan.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_BLINDSCAN_SUBSEQ_PM_H
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_SUBSEQ_PM_H

#include "sony_common.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"

/**
 @brief State of PM sub sequence
*/
typedef enum {
    PM_STATE_START,         /**< Start */
    PM_STATE_WAITING_CSFIN, /**< Wait for CS fin */
    PM_STATE_UNKNOWN        /**< Unknown */
} sony_demod_dvbs_s2_blindscan_pm_state_t;

/**
 @brief Structure for PM sub sequence
*/
typedef struct {

    /**
     @brief Pointer of common parameters used by each sub sequence
    */
    sony_demod_dvbs_s2_blindscan_seq_common_t * pCommonParams;

    uint8_t isEnable;                               /**< Enable flag (0: disable, 1: enable). */
    sony_demod_dvbs_s2_blindscan_pm_state_t state;  /**< State. */
    sony_stopwatch_t stopwatch;                     /**< Stopwatch for internal use. */

    int32_t freqOffsetKHz;                          /**< Frequency offset in KHz. */
    uint16_t power;                                 /**< Power value */


} sony_demod_dvbs_s2_blindscan_subseq_pm_t;

/**
 @brief Initialize PM sub sequence

 @param pSeq Instance for PM sub sequence.
 @param pCommonParams Pointer of common parameters.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_pm_Initialize (sony_demod_dvbs_s2_blindscan_subseq_pm_t * pSeq,
                                                                 sony_demod_dvbs_s2_blindscan_seq_common_t * pCommonParams);

/**
 @brief Start PM sub sequence

 @param pSeq Instance for PM sub sequence.
 @param freqOffsetKHz Frequency offset in KHz.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_pm_Start (sony_demod_dvbs_s2_blindscan_subseq_pm_t * pSeq,
                                                            int32_t freqOffsetKHz);

/**
 @brief Execute one sequence for PM sub sequence

 @param pSeq Instance for PM sub sequence.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_pm_Sequence (sony_demod_dvbs_s2_blindscan_subseq_pm_t * pSeq);

#endif /* SONY_DEMOD_DVBS_S2_BLINDSCAN_SUBSEQ_PM_H */
