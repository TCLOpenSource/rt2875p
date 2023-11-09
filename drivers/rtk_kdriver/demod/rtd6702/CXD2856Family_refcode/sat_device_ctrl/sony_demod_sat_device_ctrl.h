/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_sat_device_ctrl.h

          This file provide satellite device control function for demodulator.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_SAT_DEVICE_CONTROL_H
#define SONY_DEMOD_SAT_DEVICE_CONTROL_H

#include "sony_common.h"
#include "sony_demod.h"

/**
 @brief Definition of length of DiSEqC / Single cable message.
*/
#define SONY_DISEQC_MESSAGE_LENGTH 12

/**
 @brief Message structure for DiSEqC / Single cable message.
*/
typedef struct {
    uint8_t data[SONY_DISEQC_MESSAGE_LENGTH];   /**< Data of the message. */
    uint8_t length;                             /**< Length of the message. */
} sony_diseqc_message_t;

/**
 @brief DSQOUT signal mode.
*/
typedef enum {
    SONY_DSQOUT_MODE_PWM,       /**< Output PWM signal from demodulator. */
    SONY_DSQOUT_MODE_ENVELOPE   /**< Output envelope signal from demodulator. */
} sony_dsqout_mode_t;

/**
 @brief RXEN signal mode.
*/
typedef enum {
    SONY_RXEN_MODE_NORMAL,      /**< Normal mode. (Output "1" during device waits reply from slave device.) */
    SONY_RXEN_MODE_INV,         /**< Inverted mode. (Output "0" during device waits reply from slave device.) */
    SONY_RXEN_MODE_FIXED_LOW,   /**< Fixed to low always. */
    SONY_RXEN_MODE_FIXED_HIGH   /**< Fixed to high always. */
} sony_rxen_mode_t;

/**
 @brief TXEN signal mode.
*/
typedef enum {
    SONY_TXEN_MODE_NORMAL,      /**< Normal mode. (Output "1" during device sends DiSEqC command.) */
    SONY_TXEN_MODE_INV,         /**< Inverted mode. (Output "0" during device sends DiSEqC command.) */
    SONY_TXEN_MODE_FIXED_LOW,   /**< Fixed to low always. */
    SONY_TXEN_MODE_FIXED_HIGH   /**< Fixed to high always. */
} sony_txen_mode_t;

/**
 @brief Tone burst mode.
*/
typedef enum {
    SONY_TONEBURST_MODE_OFF,    /**< Not send tone burst. */
    SONY_TONEBURST_MODE_A,      /**< "0" Tone burst. (Satellite A) */
    SONY_TONEBURST_MODE_B       /**< "1" Data burst. (Satellite B) */
} sony_toneburst_mode_t;

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Enable/disable satellite device ctrl function.

        Setup DSQOUT/DSQIN pin enable/disable.

 @param pDemod Demodulator instance.
 @param enable Enable/disable setting.
                   - 0: Disable
                   - 1: Enable

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_Enable (sony_demod_t * pDemod, uint8_t enable);

/**
 @brief Configure for "DSQOUT" signal.

 @param pDemod Demodulator instance.
 @param mode DSQOUT signal mode. (Default value is PWM)
 @param toneFreqKHz Tone signal frequency. Currently, only 22(KHz) and 44(KHz) are supported.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_DSQOUTSetting (sony_demod_t * pDemod,
                                                        sony_dsqout_mode_t mode,
                                                        uint8_t toneFreqKHz);

/**
 @brief Configure for "RXEN" signal.

 @param pDemod Demodulator instance.
 @param mode RXEN signal mode.
 @param posDelay Positive delay in ms.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_RXENSetting (sony_demod_t * pDemod,
                                                      sony_rxen_mode_t mode,
                                                      uint8_t posDelay);

/**
 @brief Configure for "TXEN" signal.

 @param pDemod Demodulator instance.
 @param mode TXEN signal mode.
 @param posDelay Positive delay in ms.
 @param negDelay Negative delay in ms.

 @return
*/
sony_result_t sony_demod_sat_device_ctrl_TXENSetting (sony_demod_t * pDemod,
                                                      sony_txen_mode_t mode,
                                                      uint8_t posDelay,
                                                      uint8_t negDelay);

/**
 @brief Output tone signal.

        Tone signal output status is changed after calling this function immediately.

 @param pDemod Demodulator instance.
 @param isEnable Continuous tone mode.
                    - 0: Disable (Not output)
                    - 1: Enable (Output)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_OutputTone (sony_demod_t * pDemod,
                                                     uint8_t isEnable);

/**
 @brief Set "Tone burst" mode to demodulator.

        Tone burst signal outputs after calling sony_demod_sat_device_ctrl_StartTransmit() function.

 @param pDemod Demodulator instance.
 @param toneBurstMode Tone burst mode.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_SetToneBurst (sony_demod_t * pDemod,
                                                       sony_toneburst_mode_t toneBurstMode);

/**
 @brief Set DiSEqC command to demodulator.

        DiSEqC commands outputs after calling sony_demod_sat_device_ctrl_StartTransmit() function.

 @param pDemod Demodulator instance.
 @param isEnable DiSEqC command mode.
                    - 0: Disable (Not output any DiSEqC command)
                    - 1: Enable (Output)
 @param pCommand1 The address of 1st DiSEqC command.
 @param count1 Transmitting count for 1st DiSEqC command.
 @param pCommand2 The address of 2nd DiSEqC command.
                  (NULL if you want to transmit only one DiSEqC command.)
 @param count2 Transmitting count for 2nd DiSEqC command.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_SetDiseqcCommand (sony_demod_t * pDemod,
                                                           uint8_t isEnable,
                                                           sony_diseqc_message_t * pCommand1,
                                                           uint8_t count1,
                                                           sony_diseqc_message_t * pCommand2,
                                                           uint8_t count2);

/**
 @brief Set TxIdle time.
            - DiSEqC command.
            - Tone burst.

 @param pDemod Demodulator instance.
 @param idleTimeMs Idle time in ms.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_SetTxIdleTime (sony_demod_t * pDemod,
                                                        uint8_t idleTimeMs);

/**
 @brief Set RxIdle time.
            - DiSEqC command with reply.

 @param pDemod Demodulator instance.
 @param idleTimeMs Idle time in ms.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_SetRxIdleTime (sony_demod_t * pDemod,
                                                        uint8_t idleTimeMs);

/**
 @brief Set repeat to repeat time between each command.

 @param pDemod  Demodulator instance.
 @param r2rTime Idle time in ms.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_SetR2RTime (sony_demod_t * pDemod,
                                                     uint32_t r2rTime);

/**
 @brief Set DiSEqC reply parameters.

 @param pDemod Demodulator instance.
 @param isEnable DiSEqC reply mode.
                    - 0: No reply mode.
                    - 1: Reply mode.
 @param replyTimeoutMs Reply timeout time in ms.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_SetDiseqcReplyMode (sony_demod_t * pDemod,
                                                             uint8_t isEnable,
                                                             uint16_t replyTimeoutMs);

/**
 @brief Start transmit sequence.

 @param pDemod Demodulator instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_StartTransmit (sony_demod_t * pDemod);

/**
 @brief Get transmit status.

 @param pDemod Demodulator instance.
 @param pStatus Transmit status.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_sat_device_ctrl_GetTransmitStatus (sony_demod_t * pDemod,
                                                            uint8_t * pStatus);

/**
 @brief Get reply message.

 @param pDemod Demodulator instance.
 @param pReplyMessage Reply message instance.

 @return SONY_RESULT_OK if successful and pMessage is valid.
*/
sony_result_t sony_demod_sat_device_ctrl_GetReplyMessage (sony_demod_t * pDemod,
                                                          sony_diseqc_message_t * pReplyMessage);

/**
 @brief Get RXEN mode

 @param pDemod Demodulator instance.
 @param pIsEnable RXEN mode.
                   - 0: Disable
                   - 1: Enable

 @return SONY_RESULT_OK if successful and pIsEnable is valid.
*/
sony_result_t sony_demod_sat_device_ctrl_GetRXENMode (sony_demod_t * pDemod,
                                                      uint8_t * pIsEnable);

/**
 @brief Get TXEN mode

 @param pDemod Demodulator instance.
 @param pIsEnable TXEN mode.
                   - 0: Disable
                   - 1: Enable

 @return SONY_RESULT_OK if successful and pIsEnable is valid.
*/
sony_result_t sony_demod_sat_device_ctrl_GetTXENMode (sony_demod_t * pDemod,
                                                      uint8_t * pIsEnable);

/**
 @brief Set DiSEqC reply mask setting.

 @param pDemod Demodulator instance.
 @param pattern Pattern to allow receive as 1st byte of reply message.
 @param mask Mask for pattern data.

 @return SONY_RESULT_OK if successful and pIsEnable is valid.
*/
sony_result_t sony_demod_sat_device_ctrl_SetDiseqcReplyMask (sony_demod_t * pDemod,
                                                             uint8_t pattern,
                                                             uint8_t mask);

/**
 @brief Change the single cable mode of demod driver.

 @param pDemod Demodulator instance.
 @param enable Single cable mode.
                   - 0: Disable
                   - 1: Enable

 @return SONY_RESULT_OK if successful and pIsEnable is valid.
*/
sony_result_t sony_demod_sat_device_ctrl_EnableSinglecable (sony_demod_t * pDemod,
                                                            uint8_t enable);

#endif /* SONY_DEMOD_SAT_DEVICE_CONTROL_H */
