/*------------------------------------------------------------------------------
  Copyright 2016-2017 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
/**
 @file    sony_lnbc.h

          This file provides the LNB controller control interface.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_LNBC_H
#define SONY_LNBC_H

#include "sony_common.h"
#include "sony_i2c.h"

/**
 @brief Definition of the state of the driver.
*/
typedef enum {
    SONY_LNBC_STATE_UNKNOWN,    /**< Unknown */
    SONY_LNBC_STATE_SLEEP,      /**< Sleep */
    SONY_LNBC_STATE_ACTIVE      /**< Active */
} sony_lnbc_state_t;

/**
 @brief Definition of the voltage.
*/
typedef enum {
    SONY_LNBC_VOLTAGE_LOW,      /**< Low voltage. */
    SONY_LNBC_VOLTAGE_HIGH,     /**< High voltage. */
    SONY_LNBC_VOLTAGE_AUTO,     /**< Control voltage by demodulator. (For single cable, using TXEN) */
} sony_lnbc_voltage_t;

/**
 @brief Definition of the tone state.
*/
typedef enum {
    SONY_LNBC_TONE_OFF,         /**< Not output tone signal. */
    SONY_LNBC_TONE_ON,          /**< Output tone signal. */
    SONY_LNBC_TONE_AUTO,        /**< Control tone by demodulator. (Default) */
} sony_lnbc_tone_t;

/**
 @brief Transmit mode.
*/
typedef enum {
    SONY_LNBC_TRANSMIT_MODE_TX,     /**< TX mode. */
    SONY_LNBC_TRANSMIT_MODE_RX,     /**< RX mode. */
    SONY_LNBC_TRANSMIT_MODE_AUTO,   /**< Control transmit mode by demodulator. (For receiving DiSEqC2.x reply) */
} sony_lnbc_transmit_mode_t;

/**
 @brief Config ID for SetConfig API.
*/
typedef enum {
    SONY_LNBC_CONFIG_ID_TONE_INTERNAL,  /**< Tone mode (0: External tone, 1: Internal tone). */
    SONY_LNBC_CONFIG_ID_LOW_VOLTAGE,    /**< Definition of voltage for "Low voltage". */
    SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE    /**< Definition of voltage for "High voltage". */
} sony_lnbc_config_id_t;

/*
 * LNB controller diagnostic status bit definitions used for GetDiagStatus return value.
 */
#define SONY_LNBC_DIAG_STATUS_THERMALSHUTDOWN 0x00000001 /**< Thermal shutdown triggered. */
#define SONY_LNBC_DIAG_STATUS_OVERCURRENT     0x00000002 /**< Over current protection triggered. */
#define SONY_LNBC_DIAG_STATUS_CABLEOPEN       0x00000004 /**< Cable is not connected. */
#define SONY_LNBC_DIAG_STATUS_VOUTOUTOFRANGE  0x00000008 /**< Output voltage is out of range. */
#define SONY_LNBC_DIAG_STATUS_VINOUTOFRANGE   0x00000010 /**< Input voltage is out of range. */
#define SONY_LNBC_DIAG_STATUS_BACKBIAS        0x00000020 /**< Back bias is detected. */
#define SONY_LNBC_DIAG_STATUS_LNBCDISABLE     0x00010000 /**< LNB controller is now disabled. */

/**
 @brief Definition of the LNB controller driver API.
*/
typedef struct sony_lnbc_t {

    /* ======== Following members are NOT cleared by Initialize ======== */

    uint8_t i2cAddress;                     /**< I2C address. */
    sony_i2c_t * pI2c;                      /**< I2C driver instance. */
    int32_t isInternalTone;                 /**< Tone mode flag. (0: External tone, 1: Internal tone) */
    int32_t lowVoltage;                     /**< Definition of low voltage. */
    int32_t highVoltage;                    /**< Definition of high voltage. */
    void * user;                            /**< User defined data. */

    /* ======== Following members are cleared by Initialize (Temporary information) ======== */

    /*
     * NOTE : voltage == SONY_LNBC_VOLTAGE_AUTO,
     *        tone == SONY_LNBC_TONE_AUTO,
     *        transmitMode == SONY_LNBC_TRANSMIT_MODE_AUTO
     *        cases should not be cleared by Initialize API if they are supported.
     */

    sony_lnbc_state_t state;                /**< The state of this driver */
    sony_lnbc_voltage_t voltage;            /**< Current voltage. */
    sony_lnbc_tone_t tone;                  /**< Current tone status. */
    sony_lnbc_transmit_mode_t transmitMode; /**< Current transmit mode. */

    /* ======== Following function pointer members are NOT cleared by Initialize ======== */

    /**
     @brief Initialize the LNB controller.

     When user call this API, the LNB controller's state is following.
       - Voltage      : Low
       - Tone         : Auto (Control tone by demodulator)
       - TransmitMode : TX mode

     @param pLnbc Instance of the LNB controller driver.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*Initialize) (struct sony_lnbc_t * pLnbc);

    /**
     @brief Configure to LNB controller

     The parameters set by this API are initialized to default value by Initialize API.

     @param pLnbc Instance of the LNB controller driver.
     @param configId Configure parameter ID.
     @param value The parameter value.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*SetConfig) (struct sony_lnbc_t * pLnbc, sony_lnbc_config_id_t configId, int32_t value);

    /**
     @brief Set voltage.

     Control output voltage. (switching low voltage (about 13V) and high voltage (about 18V))
     Normally, voltage should be controlled explicitly by this API
     and SONY_LNBC_VOLTAGE_AUTO should NOT be used.
     SONY_LNBC_VOLTAGE_AUTO can be used only for single cable 1,
     using TXEN signal from demodulator GPIO pins.

     @param pLnbc Instance of the LNB controller driver.
     @param voltage Voltage to set.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*SetVoltage) (struct sony_lnbc_t * pLnbc, sony_lnbc_voltage_t voltage);

    /**
     @brief Set tone mode.

     Control tone output enable/disable.
     This API is NOT called from integ/demod part of code.
     All integ APIs assumes that SONY_LNBC_TONE_AUTO is always set.
     This means that demodulator DSQOUT pin should be connected to the LNB controller
     and all LNB controller can control tone ON/OFF by DSQOUT signal.

     @param pLnbc Instance of the LNB controller driver.
     @param tone Tone mode.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*SetTone) (struct sony_lnbc_t * pLnbc, sony_lnbc_tone_t tone);

    /**
     @brief Set transmit mode.

     Switch TX (transmit) mode and RX (receive) mode.
     Note that to use RX mode, the LNB controller has to support DiSEqC2.X feature.
     SONY_LNBC_TRANSMIT_MODE_AUTO can be used if the LNB controller can switch TX/RX without calling this API.
      - If the LNB controller can switch TX/RX by RXEN signal from demodulator.
        RXEN output from GPIO pin setting will be necessary.
      - If the LNB controller can switch TX/RX automatically by DSQOUT signal from demodulator.

     @param pLnbc Instance of the LNB controller driver.
     @param mode Transmit mode.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*SetTransmitMode) (struct sony_lnbc_t * pLnbc, sony_lnbc_transmit_mode_t mode);

    /**
     @brief Sleep the LNB controller.

     @param pLnbc Instance of the LNB controller driver.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*Sleep) (struct sony_lnbc_t * pLnbc);

    /**
     @brief Return from sleep the LNB controller.

     When user call this API, the LNB controller's state is kept before sleep.

     @param pLnbc Instance of the LNB controller driver.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*WakeUp) (struct sony_lnbc_t * pLnbc);

    /**
     @brief Read LNB controller diagnostic status.

     Depend on LNB controller, supported status monitor is different.
     Please check each LNB controller driver code in detail.

     @param pLnbc Instance of the LNB controller driver.
     @param pStatus ORed value of SONY_LNBC_DIAG_STATUS_XXX.
     @param pStatusSupported ORed value of supported SONY_LNBC_DIAG_STATUS_XXX. (NULL can be used)

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*GetDiagStatus) (struct sony_lnbc_t * pLnbc, uint32_t * pStatus, uint32_t * pStatusSupported);

} sony_lnbc_t;

#endif /* SONY_LNBC_H */
