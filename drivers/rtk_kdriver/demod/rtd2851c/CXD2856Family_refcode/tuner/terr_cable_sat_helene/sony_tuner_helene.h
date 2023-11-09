/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**

 @file    sony_tuner_helene.h

          This file provides the DVB port of the Sony HELENE tuner driver.

          This driver wraps around the Ascot3 driver provided by
          sony_helene.h by using an instance of the Ascot3 (sony_helene_t)
          driver in the ::sony_tuner_terr_cable_t::user pointer.

          Please note, if the Helene tuner is used in a system without the
          terrestrial or satellite demodulator components then the unused
          structure, pTunerSat or pTunerTerrCable must be declared prior to
          calling tuner create function.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_TUNER_HELENE_H_
#define SONY_TUNER_HELENE_H_

/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include "sony_tuner_terr_cable.h"
#include "sony_tuner_sat.h"
#include "sony_helene.h"

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
extern const char* sony_tuner_helene_version;              /**< HELENE driver version */

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
#define SONY_TUNER_HELENE_OFFSET_CUTOFF_HZ         50000   /**< Maximum carrier offset frequency before requiring a retune */

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

/**
 @brief Creates an instance of the HELENE tuner driver

 @param pTunerTerrCable The terrestrial/cable tuner driver instance to create.
        Memory must have been allocated for this instance before creation.
 @param pTunerSat The satellite tuner driver instance to create. Memory must
        have been allocated for this instance before creation.
 @param xtalFreq The crystal frequency of the tuner.
 @param i2cAddress The I2C address of the HELENE device.
        Typically 0xC0.
 @param pI2c The I2C driver that the tuner driver will use for
        communication.
 @param configFlags See ::SONY_HELENE_CONFIG_EXT_REF,
            ::SONY_HELENE_CONFIG_SLEEP_DISABLEXTAL,
            ::SONY_HELENE_CONFIG_LOOPFILTER_INTERNAL,
            ::SONY_HELENE_CONFIG_SAT_LNA_OFF,
            ::SONY_HELENE_CONFIG_POWERSAVE_TERR_NORMAL,
            ::SONY_HELENE_CONFIG_POWERSAVE_TERR_RFIN_MATCHING
            ::SONY_HELENE_CONFIG_POWERSAVE_TERR_RF_ACTIVE
            ::SONY_HELENE_CONFIG_POWERSAVE_SAT_NORMAL
            ::SONY_HELENE_CONFIG_POWERSAVE_SAT_RFIN_MATCHING
            ::SONY_HELENE_CONFIG_POWERSAVE_SAT_RF_ACTIVE
            ::SONY_HELENE_CONFIG_POWERSAVE_SAT_RFIN_MATCHING
            ::SONY_HELENE_CONFIG_OUTLMT_ATV_1_5Vpp
            ::SONY_HELENE_CONFIG_OUTLMT_ATV_1_2Vpp
            ::SONY_HELENE_CONFIG_OUTLMT_DTV_1_5Vpp
            ::SONY_HELENE_CONFIG_OUTLMT_DTV_1_2Vpp
            ::SONY_HELENE_CONFIG_OUTLMT_STV_0_75Vpp
            ::SONY_HELENE_CONFIG_OUTLMT_STV_0_6Vpp
            ::SONY_HELENE_CONFIG_REFOUT_500mVpp
            ::SONY_HELENE_CONFIG_REFOUT_400mVpp
            ::SONY_HELENE_CONFIG_REFOUT_600mVpp
            ::SONY_HELENE_CONFIG_REFOUT_800mVpp
            defined in
            \link sony_helene.h \endlink
 @param pHeleneTuner The Helene tuner driver pointer to use.
        Memory must have been allocated for the Helene driver structure.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_tuner_helene_Create (sony_tuner_terr_cable_t * pTunerTerrCable,
                                        sony_tuner_sat_t *pTunerSat,
                                        sony_helene_xtal_t xtalFreq,
                                        uint8_t i2cAddress,
                                        sony_i2c_t * pI2c,
                                        uint32_t configFlags,
                                        sony_helene_t * pHeleneTuner);

/**
 @brief Write to GPIO0 or GPIO1.

 @param pTuner  Instance of the tuner driver.
 @param id      Pin ID 0 = GPIO0, 1 = GPIO1
 @param value   Output logic level, 0 = Low, 1 = High

 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_tuner_helene_SetGPO (sony_tuner_terr_cable_t * pTuner, uint8_t id, uint8_t value);

/**
 @brief Read from GPIO1.

 @param pTuner  Instance of the tuner driver.
 @param pValue  Read logic level, 0 = Low, 1 = High

 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_tuner_helene_GetGPI1 (sony_tuner_terr_cable_t * pTuner, uint8_t * pValue);

/**
 @brief RF filter compensation setting for VHF-L band.
        (Please see RFVGA Description of datasheet.)
        New setting will become effective after next tuning.

        mult = VL_TRCKOUT_COEFF(8bit unsigned) / 128
        ofs  = VL_TRCKOUT_OFS(8bit 2s complement)
        (compensated value) = (original value) * mult + ofs

 @param pTuner Instance of the tuner driver.
 @param coeff  VL_TRCKOUT_COEFF (multiplier)
 @param offset VL_TRCKOUT_OFS (additional term)

 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_tuner_helene_RFFilterConfig (sony_tuner_terr_cable_t * pTuner, uint8_t coeff, uint8_t offset);

#endif /* SONY_TUNER_HELENE_H_ */
