/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_blindscan_data.h

          This file provides the definition of data used in BlindScan function.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_BLINDSCAN_DATA_H
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_DATA_H

#include "sony_common.h"
#include "sony_dtv.h"

/**
 @brief Power data
*/
typedef struct sony_demod_dvbs_s2_blindscan_power_t {
    struct sony_demod_dvbs_s2_blindscan_power_t * pNext;/**< Pointer of next item. */
    uint32_t freqKHz;                                   /**< Frequency in KHz. */
    int32_t power;                                      /**< Power data in dB x 100 */
} sony_demod_dvbs_s2_blindscan_power_t;

/**
 @brief Data used in BlindScan/TuneSRS sequence.
*/
typedef struct sony_demod_dvbs_s2_blindscan_data_t {
    struct sony_demod_dvbs_s2_blindscan_data_t * pNext; /**< The pointer of next item. */
    /**
     @brief Data type used in BlindScan/TuneSRS.

            "union" is used to share memory between multiple purposes.
    */
    union data_t {
        /**
         @brief Power data.
        */
        struct power_t {
            uint32_t freqKHz;           /**< Frequency in KHz. */
            int32_t power;              /**< Power in dB x 100. */
        } power;

        /**
         @brief Band information data.
        */
        struct band_t {
            uint32_t minFreqKHz;        /**< Min frequency in KHz. */
            uint32_t maxFreqKHz;        /**< Max frequency in KHz. */
        } band;

        /**
         @brief  Candidate data.
        */
        struct candidate_t {
            uint32_t centerFreqKHz;     /**< Center frequency in KHz. */
            uint32_t symbolRateKSps;    /**< Target symbol rate in KSps. */
            uint32_t minSymbolRateKSps; /**< Minimum symbol rate in range of candidate in KSps. */
            uint32_t maxSymbolRateKSps; /**< Maximum symbol rate in range of candidate in KSps. */
        } candidate;

        /**
         @brief Channel data.
        */
        struct channel_t {
            uint32_t centerFreqKHz;     /**< Center frequency in KHz. */
            uint32_t symbolRateKSps;    /**< Symbol rate in KSps. */
            sony_dtv_system_t system;   /**< System. */
        } channelInfo;
    } data;
} sony_demod_dvbs_s2_blindscan_data_t;

/**
 @brief Storage structure
*/
typedef struct {
    sony_demod_dvbs_s2_blindscan_power_t availablePowerList; /**< List of available power data. */
    sony_demod_dvbs_s2_blindscan_power_t * pPowerArrayTop;   /**< Top of power data array. */
    int32_t powerArrayLength;                                /**< Length of power data array. */

    sony_demod_dvbs_s2_blindscan_data_t availableDataList;   /**< List of available data. */
    sony_demod_dvbs_s2_blindscan_data_t * pDataArrayTop;     /**< Top of data array. */
    int32_t dataArrayLength;                                 /**< Length of data array. */

    int32_t currentUsedCount;                                /**< Current used data count. */
    int32_t maxUsedCount;                                    /**< Max used data count. */
    int32_t currentUsedPowerCount;                           /**< Current used power data count. */
    int32_t maxUsedPowerCount;                               /**< Max used power data count. */
} sony_demod_dvbs_s2_blindscan_data_storage_t;

#endif /* SONY_DEMOD_DVBS_S2_BLINDSCAN_DATA_H */
