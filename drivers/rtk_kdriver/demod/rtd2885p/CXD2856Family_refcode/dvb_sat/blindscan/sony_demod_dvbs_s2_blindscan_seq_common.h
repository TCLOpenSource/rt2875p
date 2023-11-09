/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_blindscan_seq_common.h

          This file provides commonly used definitions for BlindScan function.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_BLINDSCAN_SEQ_COMMON_H
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_SEQ_COMMON_H

#include "sony_common.h"
#include "sony_demod.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"

/**
 @brief Detected channel information.
*/
typedef struct {
    /**
     @brief Detect channel flag.

     If this is "1(Detect)", please check the other parameters in this struct.
      - 0: Not detect.
      - 1: Detect.
    */
    uint8_t isDetect;
    /**
     @brief The system of the detected channel.

     This is valid if isDetect == 1.
    */
    sony_dtv_system_t system;

    /**
     @brief The center frequency of the detected channel in KHz.

     This is valid if isDetect == 1.
    */
    uint32_t centerFreqKHz;

    /**
     @brief The symbol rate of the detected channel in KSps.

     This is valid if isDetect == 1.
    */
    uint32_t symbolRateKSps;
} sony_demod_dvbs_s2_blindscan_det_info_t;

/**
 @brief Tune request information.
*/
typedef struct {
    /**
     @brief Tune request flag.

     If this is "1(Request)", driver request to tuning.
      - 0: Not detect.
      - 1: Detect.
    */
    uint8_t isRequest;

    /**
     @brief The frequency to tune in KHz.

     This is valid if isRequest == 1.
    */
    uint32_t frequencyKHz;

    /**
     @brief The system to tune.

     This is valid if isRequest == 1.
    */
    sony_dtv_system_t system;

    /**
     @brief The symbol rate to tune in KSps.

     This is valid if isRequest == 1.
    */
    uint32_t symbolRateKSps;

} sony_demod_dvbs_s2_blindscan_tune_req_t;

/**
 @brief The information for calculate AGC level.

        This parameter used in driver internal.
        User doesn't need to access this strcutrue.
*/
typedef struct {
    uint8_t isRequest;  /**< Calculate request flag. */
    uint32_t agcLevel;  /**< AGC level get from demodulator. */
    int32_t agc_x100dB; /**< The result of calculation. */
} sony_demod_dvbs_s2_blindscan_agc_info_t;

/**
 @brief Progress information for blind scan

        This information is used only for BlindScan.
        (TuneSRS doesn't use this parameter)

        User doesn't need to access this strcutrue.
*/
typedef struct {
    uint8_t progress;           /**< Total progress in "%". */
    uint8_t majorMinProgress;   /**< Current min range in "%". */
    uint8_t majorMaxProgress;   /**< Current max range in "%". */
    uint8_t minorProgress;      /**< Current sub progress in "%". */
} sony_demod_dvbs_s2_blindscan_progress_t;

/**
 @brief The BlindScan sequence data
*/
typedef struct {
    sony_demod_t * pDemod;                                  /**< Demod driver instance */
    uint32_t waitTime;                                      /**< Wait time in ms */
    sony_demod_dvbs_s2_blindscan_det_info_t detInfo;        /**< Detected channel information */
    sony_demod_dvbs_s2_blindscan_tune_req_t tuneReq;        /**< Request to tune information */
    sony_demod_dvbs_s2_blindscan_agc_info_t agcInfo;        /**< Request to calculate information */
    sony_demod_dvbs_s2_blindscan_data_storage_t storage;    /**< Data storage */
    sony_demod_dvbs_s2_blindscan_progress_t progressInfo;   /**< Progress information */
    uint32_t ckalFreqKHz;                                   /**< CKAL in KHz */
    uint32_t ckahFreqKHz;                                   /**< CKAH in KHz */
} sony_demod_dvbs_s2_blindscan_seq_common_t;

#endif /* SONY_DEMOD_DVBS_S2_BLINDSCAN_SEQ_COMMON_H */
