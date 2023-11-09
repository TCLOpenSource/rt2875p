/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_blindscan_subseq_bt.h

          This file provides BT sub sequence for TuneSRS/BlindScan APIs.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_BLINDSCAN_SUBSEQ_BT_H
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_SUBSEQ_BT_H

#include "sony_common.h"
#include "sony_dtv.h"
#include "sony_demod_dvbs_s2_blindscan_seq_common.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"

/**
 @brief State of BT sub sequence
*/
typedef enum {
    BT_STATE_INIT,          /**< Initialized */
    BT_STATE_START,         /**< Started */
    BT_STATE_RF_TUNED,      /**< RF tuned */
    BT_STATE_WAIT_SRSFIN,   /**< Wait for SRS fin */
    BT_STATE_WAIT_TSLOCK,   /**< Wait for TS lock */
    BT_STATE_WAIT_TSLOCK2,  /**< Wait for TS lock (2nd) */
    BT_STATE_UNKNOWN        /**< Unknown state */
} sony_demod_dvbs_s2_blindscan_bt_state_t;

/**
 @brief Structure for BT sub sequence
*/
typedef struct {
    /**
     @brief Pointer of common parameters used by each sub sequence
    */
    sony_demod_dvbs_s2_blindscan_seq_common_t * pCommonParams;

    uint8_t isEnable;                               /**< Enable flag (0: disable, 1: enable) */
    sony_demod_dvbs_s2_blindscan_bt_state_t state;  /**< State */
    sony_stopwatch_t stopwatch;                     /**< Stopwatch used in this sequence */
    uint32_t timeout;                               /**< Timeout time in ms. */

    uint32_t centerFreqKHz;                         /**< Center frequency in KHz set by main sequence. */
    uint32_t candSymbolRateKSps;                    /**< Candidate symbol rate in KSps set by main sequence. */
    uint32_t minCandSymbolRateKSps;                 /**< Min symbol rate of candidate in KSps set by main sequence. */
    uint32_t maxCandSymbolRateKSps;                 /**< Max symbol rate of candidate in KSps set by main sequence. */
    uint32_t minSymbolRateKSps;                     /**< Min symbol rate of search range set by main sequence. */
    uint32_t maxSymbolRateKSps;                     /**< Max symbol rate of search range set by main sequence. */

    /* Result of BT sequence. */
    uint8_t isLocked;                               /**< Lock flag (0 : Not TS locked, 1: TS locked) */
    uint32_t detSymbolRateSps;                      /**< Detected symbol rate in Sps. */
    int32_t detCarrierOffsetKHz;                    /**< Detected carrier offset frequency in KHz. */
    sony_dtv_system_t detSystem;                    /**< Detected system (DVB-S/S2) */

} sony_demod_dvbs_s2_blindscan_subseq_bt_t;

/**
 @brief Initialize BT sub sequence

 @param pSeq Instance for BT sub sequence.
 @param pCommonParams Pointer of common parameters.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_bt_Initialize (sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeq,
                                                                 sony_demod_dvbs_s2_blindscan_seq_common_t * pCommonParams);

/**
 @brief Start BT sub sequence

 @param pSeq Instance for BT sub sequence.
 @param centerFreqKHz Center frequency in KHz.
 @param candSymbolRateKSps Candidate symbol rate in KSps.
 @param minCandSymbolRateKSps Min symbol rate of candidate in KSps.
 @param maxCandSymbolRateKSps Max symbol rate of candidate in KSps.
 @param minSymbolRateKSps Min symbol rate of search range.
 @param maxSymbolRateKSps Max symbol rate of search range.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_bt_Start (sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeq,
                                                            uint32_t centerFreqKHz,
                                                            uint32_t candSymbolRateKSps,
                                                            uint32_t minCandSymbolRateKSps,
                                                            uint32_t maxCandSymbolRateKSps,
                                                            uint32_t minSymbolRateKSps,
                                                            uint32_t maxSymbolRateKSps);

/**
 @brief Execute one sequence for BT sub sequence

 @param pSeq Instance for BT sub sequence.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_bt_Sequence (sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeq);

#endif /* SONY_DEMOD_DVBS_S2_BLINDSCAN_SUBSEQ_BT_H */
