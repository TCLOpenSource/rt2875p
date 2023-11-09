/*------------------------------------------------------------------------------
  Copyright 2012-2013 Sony Semiconductor Solutions Corporation

  Last Updated  : 2013/05/15
  File Revision : 1.0.3.0
------------------------------------------------------------------------------*/
/**
 @file    sony_horus3a.h

          This file provides the HORUS3A tuner control interface.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_HORUS3A_H
#define SONY_HORUS3A_H

#include "sony_common.h"
#include "sony_i2c.h"

/*------------------------------------------------------------------------------
  Defines
------------------------------------------------------------------------------*/
/**
 @brief Version of this driver.
*/
#define SONY_HORUS3A_VERSION     "1.0.3.0"

/**
 @brief Default I2C slave address of the HORUS3A tuner.
*/
#define SONY_HORUS3A_ADDRESS     0xC0

/*------------------------------------------------------------------------------
  Enums
------------------------------------------------------------------------------*/
/**
 @brief HORUS3A tuner state.
*/
typedef enum {
    SONY_HORUS3A_STATE_UNKNOWN,
    SONY_HORUS3A_STATE_SLEEP,
    SONY_HORUS3A_STATE_ACTIVE
} sony_horus3a_state_t;

/**
 @brief Satellite broadcasting system definitions supported by HORUS3A.
*/
typedef enum {
    SONY_HORUS3A_TV_SYSTEM_UNKNOWN,
    SONY_HORUS3A_STV_ISDBS,     /**< ISDB-S */
    SONY_HORUS3A_STV_DVBS,      /**< DVB-S */
    SONY_HORUS3A_STV_DVBS2      /**< DVB-S2 */
} sony_horus3a_tv_system_t;

/*------------------------------------------------------------------------------
  Structs
------------------------------------------------------------------------------*/
/**
 @brief The HORUS3A tuner definition which allows control of the HORUS3A tuner device
        through the defined set of functions.
*/
typedef struct sony_horus3a_t {
    uint32_t                 xtalFreqMHz;    /**< Xtal frequency for HORUS3A in MHz. */
    uint8_t                  i2cAddress;     /**< I2C slave address of the HORUS3A tuner (8-bit form - 8'bxxxxxxx0) */
    sony_i2c_t*              pI2c;           /**< I2C API instance. */
    uint32_t                 flags;          /**< ORed value of SONY_HORUS3A_CONFIG_XXXX */

    /* For saving current setting */
    sony_horus3a_state_t     state;          /**< The driver operating state. */
    uint32_t                 frequencykHz;   /**< Currently RF frequency(kHz) tuned. */
    sony_horus3a_tv_system_t tvSystem;       /**< Current broadcasting system tuned. */
    uint32_t                 symbolRateksps; /**< Current symbol rate(ksym/s) tuned. */

    void*                    user;           /**< User defined data. */
} sony_horus3a_t;

/*
 Config flag definitions. (ORed value should be set to flags argument of Create API.)
*/

/**
 @brief REFOUT output Off
*/
#define SONY_HORUS3A_CONFIG_REFOUT_OFF              0x80000000

/**
 @brief Keep Xtal enable in power save state.
        Should be used for Xtal shared system.
        Not only for the master HORUS3A which has Xtal, but also slave HORUS3A which receive the clock signal.
*/
#define SONY_HORUS3A_CONFIG_POWERSAVE_ENABLEXTAL    0x40000000

/**
 @brief Use external Xtal
        Should be used for slave HORUS3A in Xtal shared system.
*/
#define SONY_HORUS3A_CONFIG_EXT_REF                 0x20000000

/**
 @brief Internal LNA enable
*/
#define SONY_HORUS3A_CONFIG_LNA_ENABLE              0x10000000

/**
 @brief Stop internal LNA in power save state.
        This flag is ignored if SONY_HORUS3A_CONFIG_LNA_ENABLE is not used.
*/
#define SONY_HORUS3A_CONFIG_POWERSAVE_STOPLNA       0x08000000

/**
 @name IQ output setting.
       One of following value should be used.
*/
/**@{*/
#define SONY_HORUS3A_CONFIG_IQOUT_DIFFERENTIAL      0x00000000  /**< IQ Output is Differential */
#define SONY_HORUS3A_CONFIG_IQOUT_SINGLEEND         0x01000000  /**< IQ Output is Single-Ended (+4.4dB) */
#define SONY_HORUS3A_CONFIG_IQOUT_SINGLEEND_LOWGAIN 0x02000000  /**< IQ Output is Single-Ended (-2.4dB) */
#define SONY_HORUS3A_CONFIG_IQOUT_MASK              0x03000000  /**< DON'T set this value. (Internal use only) */
/**@}*/

/*------------------------------------------------------------------------------
  APIs
------------------------------------------------------------------------------*/
/**
 @brief Set up the HORUS3A tuner driver.

        This MUST be called before calling sony_horus3a_Initialize.

 @param pTuner      Reference to memory allocated for the HORUS3A instance.
                    The create function will setup this HORUS3A instance.
 @param xtalFreqMHz The frequency of the HORUS3A crystal.
 @param i2cAddress  The HORUS3A tuner I2C slave address in 8-bit form.
 @param pI2c        The I2C APIs that the HORUS3A driver will use as the I2C interface.
 @param flags       Configuration flags. It should be ORed value of SONY_HORUS3A_CONFIG_XXXX.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_horus3a_Create(sony_horus3a_t *pTuner, uint32_t xtalFreqMHz,
    uint8_t i2cAddress, sony_i2c_t *pI2c, uint32_t flags);

/**
 @brief Initialize the HORUS3A tuner.

        This MUST be called before calling sony_horus3a_Tune.

 @param pTuner       The HORUS3A tuner instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_horus3a_Initialize(sony_horus3a_t *pTuner);

/**
 @brief Tune to a given RF frequency with broadcasting system.

 @param pTuner         The HORUS3A tuner instance.
 @param frequencykHz   RF frequency to tune. (kHz)
 @param tvSystem       The type of broadcasting system to tune.
 @param symbolRateksps Symbol rate to tune. (ksps)

 @return SONY_RESULT_OK if tuner successfully tuned.
*/
sony_result_t sony_horus3a_Tune(sony_horus3a_t *pTuner, uint32_t frequencykHz,
    sony_horus3a_tv_system_t tvSystem, uint32_t symbolRateksps);

/**
 @brief Put the HORUS3A tuner into Sleep state.

        From this state the tuner can be directly tuned by calling sony_horus3a_Tune.

 @param pTuner       The HORUS3A tuner instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_horus3a_Sleep(sony_horus3a_t *pTuner);

/**
 @brief Write to GPO

 @param pTuner       The HORUS3A tuner instance.
 @param output       Output logic level, 0 = Low, 1 = High

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_horus3a_SetGPO(sony_horus3a_t *pTuner, uint8_t output);

#endif /* SONY_HORUS3A_H */
