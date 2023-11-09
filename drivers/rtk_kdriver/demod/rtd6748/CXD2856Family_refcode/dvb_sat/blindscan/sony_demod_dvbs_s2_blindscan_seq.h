/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_blindscan_seq.h

          This file provides definitions for BlindScan sequence function.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_BLINDSCAN_SEQ_H
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_SEQ_H

#include "sony_common.h"
#include "sony_demod.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_utility.h"
#include "sony_demod_dvbs_s2_blindscan_seq_common.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_bt.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_cs.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_fs.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_pm.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_ss.h"

/**
 @brief Definition of search range in MHz.

 This value is affected to the array size in BlindScan sequence.
 Please set the value depend on users application.

 Default value is 1220(MHz).
 It is 940MHz - 2160MHz. ( with some margin )
*/
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_SEARCH_RANGE_MHZ    1220

/**
 @brief Definition of array size used in this driver.

 The data size is depend on the input signal as follow.
 - Search range.
 - Power spectrum.
 - etc...

 User can get the maximum used data size by sony_demod_dvbs_s2_blindscan_data_storage_t::maxUsedCount.
 If the driver returns SONY_RESULT_ERROR_OVERFLOW, please increase this value.
*/
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_DATA_MAX           10000

/**
 @brief Definition of array size used in this driver.

 This value is calculated by SONY_DEMOD_DVBS_S2_BLINDSCAN_SEARCH_RANGE_MHZ.
 So user doesn't need to change this value directly.
*/
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_POWER_MAX ((SONY_DEMOD_DVBS_S2_BLINDSCAN_SEARCH_RANGE_MHZ + 45) * 2)

/**
 @brief state of BlindScan sequence
*/
typedef enum {
    BLINDSCAN_SEQ_STATE_START,          /**< Start */
    BLINDSCAN_SEQ_STATE_SS1_FIN,        /**< SS1 fin */
    BLINDSCAN_SEQ_STATE_STAGE1_FIN,     /**< Stage1 fin */
    BLINDSCAN_SEQ_STATE_SS2_START,      /**< SS2 start */
    BLINDSCAN_SEQ_STATE_SS2_FIN,        /**< SS2 fin */
    BLINDSCAN_SEQ_STATE_FS2_START,      /**< FS2 start */
    BLINDSCAN_SEQ_STATE_FS2_FIN,        /**< FS2 fin */
    BLINDSCAN_SEQ_STATE_CS_PREPARING,   /**< CS preparing */
    BLINDSCAN_SEQ_STATE_CS_TUNED,       /**< CS tuned */
    BLINDSCAN_SEQ_STATE_CS_FIN,         /**< CS fin */
    BLINDSCAN_SEQ_STATE_FS3_START,      /**< FS3 start */
    BLINDSCAN_SEQ_STATE_FINISH,         /**< Finish */
    BLINDSCAN_SEQ_STATE_UNKNOWN         /**< Unknown */
} sony_demod_dvbs_s2_blindscan_seq_state_t;

/**
 @brief Structure for scan sequence.
*/
typedef struct {
    /**
     @brief Common parameters used by each sub sequence
    */
    sony_demod_dvbs_s2_blindscan_seq_common_t commonParams;

    /**
     @brief Finish/Continue flag.

     - 0: Finish the sequence.
     - 1: Continue the sequence.
    */
    uint8_t isContinue;

    sony_demod_dvbs_s2_blindscan_seq_state_t seqState;    /**< State of BlindScan sequence. */
    sony_demod_dvbs_s2_blindscan_subseq_bt_t subseqBT;    /**< Data for sub sequence BT. */
    sony_demod_dvbs_s2_blindscan_subseq_pm_t subseqPM;    /**< Data for sub sequence PM. */
    sony_demod_dvbs_s2_blindscan_subseq_ss_t subseqSS;    /**< Data for sub sequence SS. */
    sony_demod_dvbs_s2_blindscan_subseq_fs_t subseqFS;    /**< Data for sub sequence FS. */
    sony_demod_dvbs_s2_blindscan_subseq_cs_t subseqCS;    /**< Data for sub sequence CS. */

    uint32_t minFreqKHz;        /**< Min frequency in KHz which set by BlindScan API */
    uint32_t maxFreqKHz;        /**< Max frequency in KHz which set by BlindScan API */
    uint32_t minSymbolRateKSps; /**< Min symbol rate in KSps which set by BlindScan API */
    uint32_t maxSymbolRateKSps; /**< Max symbol rate in KSps which set by BlindScan API */

    uint32_t minPowerFreqKHz;   /**< Min frequency in KHz which need for power spectrum */
    uint32_t maxPowerFreqKHz;   /**< Max frequency in KHz which need for power spectrum */

    sony_demod_dvbs_s2_blindscan_data_t * pDetectedList;    /**< Detected channel list */

    sony_demod_dvbs_s2_blindscan_data_t * pBandList;        /**< Band list */
    sony_demod_dvbs_s2_blindscan_data_t * pBandCurrent;     /**< Current processing band data */

    sony_demod_dvbs_s2_blindscan_data_t * pCandList1;       /**< Candidate list 1 */
    sony_demod_dvbs_s2_blindscan_data_t * pCandList2;       /**< Candidate list 2 */
    sony_demod_dvbs_s2_blindscan_data_t * pCandCurrent;     /**< The pointer of candidate */
    sony_demod_dvbs_s2_blindscan_data_t * pCandLast;        /**< The last pointer of candidate list */

    uint32_t candCount; /**< Candidate total count for calculate progress */
    uint32_t candIndex; /**< Candidate current number for calculate progress */

    /**
     @brief Power data for storage.
    */
    sony_demod_dvbs_s2_blindscan_power_t powerArray[SONY_DEMOD_DVBS_S2_BLINDSCAN_POWER_MAX];
    /**
     @brief Data for storage.
    */
    sony_demod_dvbs_s2_blindscan_data_t dataArray[SONY_DEMOD_DVBS_S2_BLINDSCAN_DATA_MAX];

} sony_demod_dvbs_s2_blindscan_seq_t;

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Prepare to start BlindScan sequence.

 @param pSeq The sequence instance.
 @param pDemod Demodulator instance.
 @param minFreqKHz Minimum frequency of the scan range in KHz.
 @param maxFreqKHz Maximum frequency of the scan range in KHz.
 @param minSymbolRateKSps Minimum symbol rate of the scan range in KSps.
 @param maxSymbolRateKSps Maximum symbol rate of the scan range in KSps.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_seq_Initialize (sony_demod_dvbs_s2_blindscan_seq_t * pSeq,
                                                           sony_demod_t * pDemod,
                                                           uint32_t minFreqKHz,
                                                           uint32_t maxFreqKHz,
                                                           uint32_t minSymbolRateKSps,
                                                           uint32_t maxSymbolRateKSps);

/**
 @brief Execute one sequence for BlindScan.

 @param pSeq The sequence instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_seq_Sequence (sony_demod_dvbs_s2_blindscan_seq_t * pSeq);

#endif /* SONY_DEMOD_DVBS_S2_BLINDSCAN_SEQ_H */
