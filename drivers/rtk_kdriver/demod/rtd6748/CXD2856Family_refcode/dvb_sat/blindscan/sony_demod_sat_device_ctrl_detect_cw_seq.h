/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_sat_device_ctrl_detect_cw_seq.h

          This file provides definitions for DetectCW sequence function.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_SAT_DEVICE_CTRL_DETECT_CW_SEQ_H
#define SONY_DEMOD_SAT_DEVICE_CTRL_DETECT_CW_SEQ_H

#include "sony_common.h"
#include "sony_demod.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_utility.h"
#include "sony_demod_dvbs_s2_blindscan_seq_common.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_pm.h"

/**
 @brief state of DetectCW sequence
*/
typedef enum {
    DETECT_CW_SEQ_STATE_START,   /**< Start */
    DETECT_CW_SEQ_STATE_AGC_ON,
    DETECT_CW_SEQ_STATE2,
    DETECT_CW_SEQ_STATE3,
    DETECT_CW_SEQ_STATE4,
    DETECT_CW_SEQ_STATE5,
    DETECT_CW_STATE_UNKNOWN      /**< Unknown */
} sony_demod_sat_device_ctrl_detect_cw_seq_state_t;

/**
 @brief Structure for DetectCW sequence.
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

    sony_demod_sat_device_ctrl_detect_cw_seq_state_t seqState; /**< State of DetectCW sequence. */
    sony_demod_dvbs_s2_blindscan_subseq_pm_t subseqPM;       /**< Data for sub sequence PM. */

    uint32_t freqKHz;   /**< Target frequency in KHz */
    uint32_t rangeKHz;  /**< Range frequency in KHz */
    uint32_t stepKHz; /**< Step frequency in KHz */
    uint8_t isDetected; /**< The result of this API. */
    int32_t offsetFreqKHz; /**< Offset frequency in KHz. */
    int32_t currentOffsetFreqKHz; /**< Current offset frequency in KHz. */
    int32_t power;      /**< Power */

    /**
     @brief Power data for storage.
    */
    sony_demod_dvbs_s2_blindscan_power_t powerArray[1];

    /**
     @brief Data for storage.
    */
    sony_demod_dvbs_s2_blindscan_data_t dataArray[1];

} sony_demod_sat_device_ctrl_detect_cw_seq_t;

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Prepare to start DetectCW sequence.

 @param pSeq The sequence instance.
 @param pDemod Demodulator instance.
 @param freqKHz Target frequency in KHz.
 @param rangeKHz The range for search in KHz. (500, 1000, ...)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_detect_cw_seq_Initialize (sony_demod_sat_device_ctrl_detect_cw_seq_t * pSeq,
                                                                   sony_demod_t * pDemod,
                                                                   uint32_t freqKHz,
                                                                   uint32_t rangeKHz);

/**
 @brief Execute one sequence for DetectCW.

 @param pSeq The sequence instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_detect_cw_seq_Sequence (sony_demod_sat_device_ctrl_detect_cw_seq_t * pSeq);

#endif /* SONY_DEMOD_SAT_DEVICE_CTRL_DETECT_CW_SEQ_H */
