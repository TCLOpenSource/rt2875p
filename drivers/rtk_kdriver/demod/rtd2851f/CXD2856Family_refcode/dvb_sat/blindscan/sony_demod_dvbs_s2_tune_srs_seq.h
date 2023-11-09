/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_tune_srs_seq.h

          This file provides defines for TuneSRS sequence.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_TUNE_SRS_SEQ_H
#define SONY_DEMOD_DVBS_S2_TUNE_SRS_SEQ_H

#include "sony_common.h"
#include "sony_demod.h"
#include "sony_demod_dvbs_s2.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_utility.h"
#include "sony_demod_dvbs_s2_blindscan_seq_common.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_bt.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_cs.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_fs.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_pm.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_ss.h"
#include "sony_demod_dvbs_s2_tune_srs.h"

/*------------------------------------------------------------------------------
 Definitions
------------------------------------------------------------------------------*/
/**
 @brief Definition of array size used for TuneSRS in this driver.

        The user need not to change this value directly.
*/
#define SONY_DEMOD_DVBS_S2_TUNE_SRS_POWER_MAX  500

/**
 @brief Definition of array size used for TuneSRS in this driver.

 The data size is depend on the input signal as follow.
 - Search range.
 - Power spectrum.
 - etc...

 User can get the maximum used data size by sony_demod_dvbs_s2_blindscan_data_storage_t::maxUsedCount.
 If the driver returns SONY_RESULT_ERROR_OVERFLOW, please increase this value.
*/
#define SONY_DEMOD_DVBS_S2_TUNE_SRS_DATA_MAX  5000

/**
 @brief State for TuneSRS sequence.
*/
typedef enum {
    TUNE_SRS_SEQ_STATE_START,       /**< Start */
    TUNE_SRS_SEQ_STATE_SS1_FIN,     /**< SS1 fin */
    TUNE_SRS_SEQ_STATE_RF_TUNED,    /**< RF tuned */
    TUNE_SRS_SEQ_STATE_CS1_FIN,     /**< CS1 fin */
    TUNE_SRS_SEQ_STATE_CS1_FIN2,    /**< CS1 fin2 */
    TUNE_SRS_SEQ_STATE_SS2_FIN,     /**< SS2 fin */
    TUNE_SRS_SEQ_STATE_SS3_FIN,     /**< SS3 fin */
    TUNE_SRS_SEQ_STATE_CAND2,       /**< Cand2 */
    TUNE_SRS_SEQ_STATE_BT_START,    /**< Start BT */
    TUNE_SRS_SEQ_STATE_BT_END       /**< End BT */,
    TUNE_SRS_SEQ_STATE_FINISH,      /**< Finish */
    TUNE_SRS_SEQ_STATE_UNKNOWN      /**< Unknown */
} sony_demod_dvbs_s2_tune_srs_seq_state_t;

/**
 @brief structure for scan sequence.
*/
typedef struct {

    /**
     @brief Common parameters used by each sub sequence.
    */
    sony_demod_dvbs_s2_blindscan_seq_common_t commonParams;

    /**
     @brief Finish/Continue flag.

            - 0: Finish the sequence.
            - 1: Continue the sequence.
    */
    uint8_t isContinue;

    /**
     @brief Signal detect flag.

            After "isContinue == 0", please see this flag.
            If isDetect == 1, TuneSRS detected signal.
            If isDetect == 0, TuneSRS didn't detect signal.
    */
    uint8_t isDetect;

    /**
     @brief State of main sequence.
    */
    uint32_t seqState;

    sony_demod_dvbs_s2_blindscan_subseq_bt_t subseqBT; /**< Sub sequence for BT. */
    sony_demod_dvbs_s2_blindscan_subseq_pm_t subseqPM; /**< Sub sequence for PM. */
    sony_demod_dvbs_s2_blindscan_subseq_ss_t subseqSS; /**< Sub sequence for SS. */
    sony_demod_dvbs_s2_blindscan_subseq_fs_t subseqFS; /**< Sub sequence for FS. */
    sony_demod_dvbs_s2_blindscan_subseq_cs_t subseqCS; /**< Sub sequence for CS. */

    uint32_t centerFreqKHz;     /**< Center frequency in KHz from argument */
    uint32_t rangeFreqMHz;      /**< Capture range in MHz from argument */
    uint32_t minFreqKHz;        /**< Min freq calculated from centerFreqKHz and rangeFreqMHz */
    uint32_t maxFreqKHz;        /**< Max freq calculated from centerFreqKHz and rangeFreqMHz */
    uint32_t minSymbolRateKSps; /**< Min symbol rate in KSps. (Fixed to  1000) */
    uint32_t maxSymbolRateKSps; /**< Max symbol rate in KSps. (Fixed to 45000) */

    uint32_t powerMinFreqKHz1;  /**< Min frequency for 1st power search in KHz. */
    uint32_t powerMaxFreqKHz1;  /**< Max frequency for 1st power search in KHz. */
    uint32_t powerMinFreqKHz2;  /**< Min frequency for 2nd power search in KHz. */
    uint32_t powerMaxFreqKHz2;  /**< Max frequency for 2nd power search in KHz. */
    uint32_t powerMinFreqKHz3;  /**< Min frequency for 3rd power search in KHz. */
    uint32_t powerMaxFreqKHz3;  /**< Max frequency for 3rd power search in KHz. */

    sony_demod_dvbs_s2_blindscan_data_t * pDetectedList;    /**< Detected list */

    sony_demod_dvbs_s2_blindscan_data_t * pCandList1;       /**< Candidate list 1 */
    sony_demod_dvbs_s2_blindscan_data_t * pCandLast1;       /**< Last pointer of candidate list 1 */
    sony_demod_dvbs_s2_blindscan_data_t * pCandCurrent;     /**< Current pointer of candidate list 1 */

    sony_demod_dvbs_s2_blindscan_data_t * pCandList2;       /**< Candidate list 2 */
    sony_demod_dvbs_s2_blindscan_data_t * pCandLast2;       /**< Last pointer of candidate list 2 */

    sony_demod_dvbs_s2_blindscan_power_t * pPowerList;      /**< Power list */
    sony_demod_dvbs_s2_blindscan_power_t * pPowerLast;      /**< Last pointer of power list */

    /**
     @brief Power data for storage.
    */
    sony_demod_dvbs_s2_blindscan_power_t powerArray[SONY_DEMOD_DVBS_S2_TUNE_SRS_POWER_MAX];
    /**
     @brief Data for storage.
    */
    sony_demod_dvbs_s2_blindscan_data_t dataArray[SONY_DEMOD_DVBS_S2_TUNE_SRS_DATA_MAX];

    /**
     @brief Instance for detected signal.
    */
    sony_dvbs_s2_tune_param_t tuneParam;

} sony_demod_dvbs_s2_tune_srs_seq_t;

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Prepare to start TuneSRS sequence.

 @param pSeq The sequence instance.
 @param pDemod Demodulator instance.
 @param centerFreqKHz Center frequency in KHz.
 @param freqRange Frequency range.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_tune_srs_seq_Initialize (sony_demod_dvbs_s2_tune_srs_seq_t * pSeq,
                                                          sony_demod_t * pDemod,
                                                          uint32_t centerFreqKHz,
                                                          sony_demod_dvbs_s2_tune_srs_freq_range_t freqRange);

/**
 @brief Execute one sequence for TuneSRS.

 @param pSeq The sequence instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_tune_srs_seq_Sequence (sony_demod_dvbs_s2_tune_srs_seq_t * pSeq);

#endif /* SONY_DEMOD_DVBS_S2_TUNE_SRS_SEQ_H */
