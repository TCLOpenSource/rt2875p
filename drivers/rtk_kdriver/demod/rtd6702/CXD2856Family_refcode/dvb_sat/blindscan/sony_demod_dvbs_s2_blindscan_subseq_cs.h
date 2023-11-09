/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_blindscan_subseq_cs.h

          This file provides CS sub sequence for TuneSRS/BlindScan.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_BLINDSCAN_SUBSEQ_CS_H
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_SUBSEQ_CS_H

#include "sony_common.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_pm.h"

/**
 @brief State of CS sub sequence
*/
typedef enum {
    CS_STATE_START,             /**< Start */
    CS_STATE_PEAK_SEARCH_START, /**< Peak search start */
    CS_STATE_PEAK_SEARCHING,    /**< Peak searching */
    CS_STATE_LOWER_SEARCHING,   /**< Lower edge searching */
    CS_STATE_UPPER_SEARCHING,   /**< Upper edge searching */
    CS_STATE_UNKNOWN            /**< Unknown */
} sony_demod_dvbs_s2_blindscan_cs_state_t;

/**
 @brief Structure for CS sub sequence
*/
typedef struct {

    /**
     @brief Pointer of common parameters used by each sub sequence
    */
    sony_demod_dvbs_s2_blindscan_seq_common_t * pCommonParams;
    /**
     @brief Pointer of PM sub sequence instance
    */
    sony_demod_dvbs_s2_blindscan_subseq_pm_t * pSeqPM;

    uint8_t isEnable;                               /**< Enable flag (0: disable, 1: enable). */
    sony_demod_dvbs_s2_blindscan_cs_state_t state;  /**< State. */
    uint32_t agcLevel;                              /**< AGC level from register. */
    int32_t agc_x100dB;                             /**< Calculated AGC (x 100dB) by tuner driver. */
    int8_t index;                                   /**< Index for internal use. */
    int32_t peakPower;                              /**< Peak power. */
    int32_t peakFreqOfsKHz;                         /**< Peak frequency offset in KHz. */
    int32_t lowerFreqKHz;                           /**< Lower frequency in KHz. */
    int32_t upperFreqKHz;                           /**< Upper frequency in KHz. */
    uint8_t isFin;                                  /**< Flags whether this sequence finished or not. */

    int32_t freqOffsetKHz;                          /**< Frequency offset in KHz. */

    uint8_t isExist;                                /**< Result of this CS sub sequence (0 : Signal is not exist, 1 : Signal is exist) */
    uint32_t coarseSymbolRateKSps;                  /**< Result of this CS sub sequence (Symbol rate in KSps) */

} sony_demod_dvbs_s2_blindscan_subseq_cs_t;

/**
 @brief Initialize CS sub sequence

 @param pSeq Instance for CS sub sequence.
 @param pSeqPM Pointer of PM sub sequence instance.
 @param pCommonParams Pointer of common parameters.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_cs_Initialize (sony_demod_dvbs_s2_blindscan_subseq_cs_t * pSeq,
                                                                 sony_demod_dvbs_s2_blindscan_subseq_pm_t * pSeqPM,
                                                                 sony_demod_dvbs_s2_blindscan_seq_common_t * pCommonParams);

/**
 @brief Start CS sub sequence

 @param pSeq Instance for CS sub sequence.
 @param freqOffsetKHz Frequency offset in KHz.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_cs_Start (sony_demod_dvbs_s2_blindscan_subseq_cs_t * pSeq,
                                                            int32_t freqOffsetKHz);

/**
 @brief Execute one sequence for CS sub sequence

 @param pSeq Instance for CS sub sequence.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_cs_Sequence (sony_demod_dvbs_s2_blindscan_subseq_cs_t * pSeq);

#endif /* SONY_DEMOD_DVBS_S2_BLINDSCAN_SUBSEQ_CS_H */
