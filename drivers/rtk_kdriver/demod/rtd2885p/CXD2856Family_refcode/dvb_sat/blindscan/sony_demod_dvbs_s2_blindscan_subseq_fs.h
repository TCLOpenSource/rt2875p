/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_blindscan_subseq_fs.h

          This file provides FS sub sequence for TuneSRS/BlindScan.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_BLINDSCAN_SUBSEQ_FS_H
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_SUBSEQ_FS_H

#include "sony_common.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_bt.h"

/**
 @brief State of FS sub sequence
*/
typedef enum {
    FS_STATE_START,         /**< Start */
    FS_STATE_WAIT_BTFIN,    /**< Wait for BT fin */
    FS_STATE_FINISH,        /**< Finish FS */
    FS_STATE_UNKNOWN        /**< Unknown */
} sony_demod_dvbs_s2_blindscan_fs_state_t;

/**
 @brief Structure for FS sub sequence
*/
typedef struct {

    /**
     @brief Pointer of common parameters used by each sub sequence
    */
    sony_demod_dvbs_s2_blindscan_seq_common_t * pCommonParams;
    /**
     @brief Pointer of BT sub sequence instance
    */
    sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeqBT;

    uint8_t isEnable;                                   /**< Enable flag (0: disable, 1: enable). */
    sony_demod_dvbs_s2_blindscan_fs_state_t state;      /**< State. */

    sony_demod_dvbs_s2_blindscan_data_t * pCandList;    /**< Candidate list */
    sony_demod_dvbs_s2_blindscan_data_t * pChannelList; /**< Detected channel list */
    uint32_t minSymbolRateKSps;                         /**< Min symbol rate of search range in KSps */
    uint32_t maxSymbolRateKSps;                         /**< Max symbol rate of search range in KSps */

    sony_demod_dvbs_s2_blindscan_data_t * pCandCurrent; /**< Pointer of current candidate */
    sony_demod_dvbs_s2_blindscan_data_t * pChannelLast; /**< The last pionter of detected channel list */
    uint32_t candFreq;                                  /**< Frequency of candidate in KHz. */

    uint32_t candCount;                                 /**< Candidate number for calculate progress. */
    uint32_t candIndex;                                 /**< Candidate index for calculate progress. */

} sony_demod_dvbs_s2_blindscan_subseq_fs_t;

/**
 @brief Initialize FS sub sequence

 @param pSeq Instance for FS sub sequence.
 @param pSeqBT Pointer of BT sub sequence instance.
 @param pCommonParams Pointer of common parameters.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_fs_Initialize (sony_demod_dvbs_s2_blindscan_subseq_fs_t * pSeq,
                                                                 sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeqBT,
                                                                 sony_demod_dvbs_s2_blindscan_seq_common_t * pCommonParams);

/**
 @brief Start CS sub sequence

 @param pSeq Instance for FS sub sequence.
 @param pCandList Candidate list.
 @param minSymbolRateKSps Min symbol rate of search range in KSps.
 @param maxSymbolRateKSps Max symbol rate of search range in KSps.
 @param pChannelList Detected channel list.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_fs_Start (sony_demod_dvbs_s2_blindscan_subseq_fs_t * pSeq,
                                                            sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                                            uint32_t minSymbolRateKSps,
                                                            uint32_t maxSymbolRateKSps,
                                                            sony_demod_dvbs_s2_blindscan_data_t * pChannelList);

/**
 @brief Execute one sequence for FS sub sequence

 @param pSeq Instance for FS sub sequence.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_fs_Sequence (sony_demod_dvbs_s2_blindscan_subseq_fs_t * pSeq);

#endif /* SONY_DEMOD_DVBS_S2_BLINDSCAN_SUBSEQ_FS_H */
