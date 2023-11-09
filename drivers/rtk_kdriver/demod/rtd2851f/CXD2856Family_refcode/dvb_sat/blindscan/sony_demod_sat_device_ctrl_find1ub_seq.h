/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_sat_device_ctrl_find1ub_seq.h

          This file provides definitions for Find1UB sequence function.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_SEQ_H
#define SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_SEQ_H

#include "sony_common.h"
#include "sony_demod.h"
#include "sony_singlecable_command.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_utility.h"
#include "sony_demod_dvbs_s2_blindscan_seq_common.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_pm.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_ss.h"

/**
 @brief Definition of search range in MHz.

 This value is affected to the array size in Find1UB sequence.
 Please set the value depend on users application.

 Default value is 1220(MHz).
 It is 940MHz - 2160MHz. ( with some margin )
*/
#define SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_SEARCH_RANGE_MHZ    1220

/**
 @brief Definition of array size used in this driver.

 The data size is depend on the input signal as follow.
 - Search range.
 - Power spectrum.
 - etc...

 User can get the maximum used data size by sony_demod_dvbs_s2_blindscan_data_storage_t::maxUsedCount.
 If the driver returns SONY_RESULT_ERROR_OVERFLOW, please increase this value.
*/
//#define SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_DATA_MAX           10000

/**
 @brief Definition of array size used in this driver.

 This value is calculated by SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_SEARCH_RANGE_MHZ.
 So user doesn't need to change this value directly.
*/
#define SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_POWER_MAX ((SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_SEARCH_RANGE_MHZ + 45) * 2)

/**
 @brief Wait time in ms after sending ODU Single Cable commands.

 This value depends on your Single Cable Switch.
 Please confirm it with your environment and change it accordingly.
*/
#define SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_ODU_TX_WAIT 300

/**
 @brief state of Find1UB sequence
*/
typedef enum {
    FIND1UB_SEQ_STATE_START,          /**< Start */
    FIND1UB_SEQ_STATE_SS_START,       /**< SS start */
    FIND1UB_SEQ_STATE_SS_FIN,         /**< SS fin */
    FIND1UB_SEQ_STATE_TRIAL1,         /**< Trial1 */
    FIND1UB_SEQ_STATE_TRIAL2,         /**< Trial2 */
    FIND1UB_SEQ_STATE_TRIAL3,         /**< Trial3 */
    FIND1UB_SEQ_STATE_TRIAL4,         /**< Trial4 */
    FIND1UB_SEQ_STATE_TRIAL5,         /**< Trial5 */
    FIND1UB_SEQ_STATE_TRIAL6,         /**< Trial6 */
    FIND1UB_SEQ_STATE_TRIAL7,         /**< Trial7 */
    FIND1UB_SEQ_STATE_TRIAL8,         /**< Trial8 */
    FIND1UB_SEQ_STATE_TRIAL9,         /**< Trial9 */
    FIND1UB_SEQ_STATE_FINISH,         /**< Finish */
    FIND1UB_SEQ_STATE_TUNING_ON,      /**< Tuning at UB on */
    FIND1UB_SEQ_STATE_TUNING_OFF,     /**< Tuning at UB off */
    FIND1UB_SEQ_STATE_AGC_ON,         /**< Getting AGC at UB on */
    FIND1UB_SEQ_STATE_AGC_OFF,        /**< Getting AGC at UB off */
    FIND1UB_SEQ_STATE_UNKNOWN         /**< Unknown */
} sony_demod_sat_device_ctrl_find1ub_seq_state_t;

/**
 @brief Structure for find1ub sequence.
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

    sony_demod_sat_device_ctrl_find1ub_seq_state_t seqState; /**< State of Find1UB sequence. */
    sony_demod_dvbs_s2_blindscan_subseq_pm_t subseqPM;       /**< Data for sub sequence PM. */
    sony_demod_dvbs_s2_blindscan_subseq_ss_t subseqSS;       /**< Data for sub sequence SS. */

    uint32_t minFreqKHz;        /**< Min frequency in KHz which set by Find1UB API */
    uint32_t maxFreqKHz;        /**< Max frequency in KHz which set by Find1UB API */

    uint32_t minPowerFreqKHz;   /**< Min frequency in KHz which need for power spectrum */
    uint32_t maxPowerFreqKHz;   /**< Max frequency in KHz which need for power spectrum */

    uint8_t commandReq; /**< Request to transmit command */
    sony_diseqc_message_t message; /**< Single cable command data */
    uint8_t targetUB; /**< Target user band to detect */
    uint32_t ubFreqKHz; /**< Frequency of target UB. If this is 0 at isContinue = 0, driver can't detect target UB. */
    int32_t pmax_p1;   /**< Max power at the 1st trial */
    uint32_t pmax_fn;  /**< Frequency of max power at Nth trial */
    int32_t pmax_pn;   /**< Max power at the Nth trial */
    int32_t loopCount; /**< Loop count in the trial sequence */
    int32_t powerOn;   /**< Power at UB on */
    int32_t powerOff;  /**< Power at UB off */
    int32_t offsetKHz; /**< Offset frequency between "pmax_fn" and "tuned frequency" */
    uint8_t ubNumber; /**< UB Number. */

    /**
     @brief Power data for storage.
    */
    sony_demod_dvbs_s2_blindscan_power_t powerArray[SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_POWER_MAX];
    /**
     @brief Data for storage.
    */
    sony_demod_dvbs_s2_blindscan_data_t dataArray[1];

} sony_demod_sat_device_ctrl_find1ub_seq_t;

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Prepare to start Find1UB sequence.

 @param pSeq The sequence instance.
 @param pDemod Demodulator instance.
 @param minFreqKHz Minimum frequency of the scan range in KHz.
 @param maxFreqKHz Maximum frequency of the scan range in KHz.
 @param targetUB Target user band which user want to find. (1-8)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_find1ub_seq_Initialize (sony_demod_sat_device_ctrl_find1ub_seq_t * pSeq,
                                                                 sony_demod_t * pDemod,
                                                                 uint32_t minFreqKHz,
                                                                 uint32_t maxFreqKHz,
                                                                 uint8_t targetUB);

/**
 @brief Execute one sequence for Find1UB.

 @param pSeq The sequence instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_find1ub_seq_Sequence (sony_demod_sat_device_ctrl_find1ub_seq_t * pSeq);

#endif /* SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_SEQ_H */
