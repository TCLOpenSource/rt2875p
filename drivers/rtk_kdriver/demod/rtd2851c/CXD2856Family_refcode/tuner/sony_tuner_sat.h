/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_tuner_sat.h

          This file provides the tuner control interface specific to satellite.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_TUNER_SAT_H
#define SONY_TUNER_SAT_H

#include "sony_common.h"
#include "sony_i2c.h"
#include "sony_dtv.h"

/**
 @brief Definition of the Satellite tuner driver API.
*/
typedef struct sony_tuner_sat_t {
    uint8_t i2cAddress;         /**< I2C address. */
    sony_i2c_t * pI2c;          /**< I2C driver instance. */
    uint32_t flags;             /**< Flags that can be used by tuner drivers. */
    uint32_t frequencyKHz;      /**< Current RF frequency(KHz) tuned. */
    sony_dtv_system_t system;   /**< Current system tuned. */
    uint32_t symbolRateKSps;    /**< Cuurent symbol rate(KSps) tuned. */
    uint32_t symbolRateKSpsForSpectrum; /**< Symbol rate setting to get power spectrum. */

    /**
     @brief Initialize the tuner.

     @param pTuner Instance of the tuner driver.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*Initialize) (struct sony_tuner_sat_t * pTuner);

    /**
     @brief Tune to a given frequency with a given system and symbol rate.
            Tuner driver can block while waiting for PLL locked indication (if required).

     @param pTuner Instance of the tuner driver.
     @param centerFreqKHz RF frequency to tune too (kHz)
     @param system The type of channel to tune too (DVB-S/S2).
     @param symbolRateKSps The symbol rate of the channel in KHz.

     @return SONY_RESULT_OK if tuner successfully tuned.
    */
    sony_result_t (*Tune) (struct sony_tuner_sat_t * pTuner,
                           uint32_t centerFreqKHz,
                           sony_dtv_system_t dtvSystem,
                           uint32_t symbolRateKSps);

    /**
     @brief Sleep the tuner device (if supported).

     @param pTuner Instance of the tuner driver.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*Sleep) (struct sony_tuner_sat_t * pTuner);

    /**
     @brief Shutdown the tuner device (if supported).

     @param pTuner Instance of the tuner driver.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*Shutdown) (struct sony_tuner_sat_t * pTuner);

    /**
     @brief Calculate AGC (dB * 100) from AGC level from demodulator.

     @param pTuner Instance of the tuner driver.
     @param AGCLevel The value from sony_demod_dvbs_monitor_IFAGCOut() API.
     @param pAGCdB Address of AGC (dB * 100) value.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*AGCLevel2AGCdB) (struct sony_tuner_sat_t * pTuner,
                                     uint32_t AGCLevel,
                                     int32_t * pAGCdB);

    void * user;                /**< User defined data. */

} sony_tuner_sat_t;

#endif /* SONY_TUNER_SAT_H */
