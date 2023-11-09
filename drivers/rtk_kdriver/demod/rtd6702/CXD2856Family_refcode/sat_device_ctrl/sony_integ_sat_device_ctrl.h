/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
/**
 @file    sony_integ_sat_device_ctrl.h

          This file provide functions which control satellite device.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_INTEG_SAT_DEVICE_CTRL_H
#define SONY_INTEG_SAT_DEVICE_CTRL_H

#include "sony_integ.h"
#include "sony_demod_sat_device_ctrl.h"
#include "sony_singlecable_command.h"
#include "sony_singlecable2_command.h"

/**
 @brief LNB controller wakeup time in ms.

 The time that LNB controller wake up.
 In common, LNB controller needs time to output stable volage after waking up.
 In addition, connected DiSEqC / Single Cable switches may need additional time to start up.
 The user can change it depends on LNB controller and other situation.
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_LNBC_WAKEUP_TIME_MS 500

/**
 @brief Device voltage change time in ms.

 The time between "return from sony_lnbc_t::SetVoltage()" and completing voltage change.
 This value depends on the LNB controller device, circuit, etc.
 Please confirm with your implementation/environment and change it accordingly.
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_DEVICE_VOLTAGE_CHANGE_TIME 15

/**
 @brief Pre transmission wait time defined in DiSEqC1 standard.

 For one shot DiSEqC transmissions, the required wait time is >15ms, whilst for repeated messages the required wait time is >25ms.
 This timing definition is used for both transmission modes so the greater of the two times has been used.
 Please refer to "DiSEqC Update and Recommendations for Implementation Version 2.1" Figure 3 for more information.

 There is no need to modify this value.
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_DISEQC1_SPEC_PRE_TX_WAIT 26

/**
 @brief Pre transmission wait time defined in DiSEqC2 standard.

 This value is selected as the same value as pre transmission wait time defined in DiSEqC1 standard,
 because there is no clear timing description about DiSEqC2 in DiSEqC standard documents.

 There is no need to modify this value.
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_DISEQC2_SPEC_PRE_TX_WAIT 26

/**
 @brief HW setting for wait time between each event(DiSEqC command, tone burst and continuous tone) in ms for DiSEqC1 sequence.

 If SONY_INTEG_SAT_DEVICE_CTRL_DEVICE_VOLTAGE_CHANGE_TIME is set correctly,
 then there is no need to modify this value.
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_DISEQC1_TXIDLE_MS  (SONY_INTEG_SAT_DEVICE_CTRL_DISEQC1_SPEC_PRE_TX_WAIT + SONY_INTEG_SAT_DEVICE_CTRL_DEVICE_VOLTAGE_CHANGE_TIME)


/**
 @brief HW setting for wait time between each event(DiSEqC command, tone burst and continuous tone) in ms for DiSEqC2 sequence.

 If SONY_INTEG_SAT_DEVICE_CTRL_DEVICE_VOLTAGE_CHANGE_TIME is set correctly,
 then there is no need to modify this value.
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_DISEQC2_TXIDLE_MS  (SONY_INTEG_SAT_DEVICE_CTRL_DISEQC2_SPEC_PRE_TX_WAIT + SONY_INTEG_SAT_DEVICE_CTRL_DEVICE_VOLTAGE_CHANGE_TIME)

/**
 @brief HW setting for wait time between finish to receive reply message and start continuous tone.

 There is no need to modify this value.
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_DISEQC2_RXIDLE_MS 5

/**
 @brief HW setting for wait time between finish to receive reply message and start continuous tone.

 There is no need to modify this value.
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_RXIDLE_MS 5

/**
 @brief HW setting for wait time between each event(Voltage change and Single cable command) in ms for Single cable sequence.

 This value(X) should be "4 < X < 22" from Singlecable standard EN50494.
 And it should be "X > SONY_INTEG_SAT_DEVICE_CTRL_DEVICE_VOLTAGE_CHANGE_TIME".
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_TXIDLE_MS ((SONY_INTEG_SAT_DEVICE_CTRL_DEVICE_VOLTAGE_CHANGE_TIME < 5) ? 5 : SONY_INTEG_SAT_DEVICE_CTRL_DEVICE_VOLTAGE_CHANGE_TIME)
#if SONY_INTEG_SAT_DEVICE_CTRL_DEVICE_VOLTAGE_CHANGE_TIME >= 22
#error "SONY_INTEG_SAT_DEVICE_CTRL_DEVICE_VOLTAGE_CHANGE_TIME exceeds single cable standard timing requirement."
#endif /* SONY_INTEG_SAT_DEVICE_CTRL_DEVICE_VOLTAGE_CHANGE_TIME >= 22 */

/**
 @brief HW setting for wait time between repeated DiSEqC1 messages (Repeat To Repeat Time) in ms.

 This value specifies time between the end of the first message and the start of the repeated message.
 There is no need to modify this value.
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_DISEQC1_R2R_TIME_MS 400

/**
 @brief HW setting for wait time between repeated Single cable messages (Repeat To Repeat Time) in ms.

 This value specifies time between the end of the first message and the top of the repeated message.
 There is no need to modify this value.
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_R2R_TIME_MS 0

/**
 @brief HW setting for DiSEqC2 reply wait time out(RTO) in ms.

 There is no need to modify this value.
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_DISEQC2_RTO_TIME_MS 400

/**
 @brief HW setting for Singlecable2 reply wait time out(RTO) in ms.

 There is no need to modify this value.
*/
#define SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE2REPLY_RTO_TIME_MS 100

/**
 @brief Polling interval for sony_integ_sat_device_ctrl_TransmitToneBurst() in ms.
*/
#define SONY_INTEG_DISEQC_TB_POL                        100

/**
 @brief Polling interval for sony_integ_sat_device_ctrl_TransmitDiseqcCommand()
        and sony_integ_sat_device_ctrl_TransmitDiseqcCommandWithReply() in ms.
*/
#define SONY_INTEG_DISEQC_DISEQC_TRANSMIT_POL           10

/**
 @brief Polling interval for sony_integ_sat_device_ctrl_TransmitSinglecableCommand() in ms.
*/
#define SONY_INTEG_DISEQC_SINGLECABLE_TRANSMIT_POL      10

/**
 @brief Version of single cable standard
*/
typedef enum {
    SONY_SINGLECABLE_VERSION_1_EN50494,     /**< Single cable 1 (EN50494) */
    SONY_SINGLECABLE_VERSION_2_EN50607,     /**< Single cable 2 (EN50607) */
    SONY_SINGLECABLE_VERSION_UNKNOWN
} sony_singlecable_version_t;

/**
 @brief Parameters for tuning in single cable environment.
*/
typedef struct {
    sony_integ_t * pInteg;                  /**< Integ part. */
    sony_tuner_sat_t * pTunerReal;          /**< Tuner instance. */

    sony_singlecable_version_t version;     /**< Single cable version. */

    sony_singlecable_address_t address;     /**< Address byte. */
    sony_singlecable_bank_t bank;           /**< Signal bank. */
    uint8_t ubSlot;                         /**< UB slot number. (1-8) */
    uint32_t ubSlotFreqKHz;                 /**< UB slot center frequency in KHz. */
    uint8_t enableMDUMode;                  /**< MDU mode enable/disable. */
    uint8_t PINCode;                        /**< PIN Code for MDU commands. */

    uint8_t scd2_ubSlot;                    /**< UB slot number. (1-32) (for SCD2) */
    uint32_t scd2_ubSlotFreqMHz;            /**< UB slot center frequency in *MHz*. (for SCD2) */
    uint8_t scd2_uncommittedSwitches;       /**< Uncommitted switches setting. (for SCD2) */
    uint8_t scd2_committedSwitches;         /**< Committed switches setting. (for SCD2) */
    uint8_t scd2_enableMDUMode;             /**< MDU mode enable/disable. (for SCD2) */
    uint8_t scd2_PINCode;                   /**< PIN Code for MDU commands. (for SCD2) */

    /**
     * Pointer to the function which sends the DiSEqC message by the other demodulator.
     * If this is null, this demodulator will send the DiSEqC message for the single cable control.
     * This will be used for the case when there is only one LNB controller shared between multiple tuners.
     */
    sony_result_t (*pTransmitByOtherDemod)(sony_diseqc_message_t * pMessage);
} sony_integ_singlecable_tuner_data_t;

/**
 @brief The data for single cable sequence.
*/
typedef struct {
    sony_tuner_sat_t tunerSinglecable;                        /**< Tuner instance for single cable. */
    sony_integ_singlecable_tuner_data_t tunerSinglecableData; /**< Single cable tuner data. */
} sony_integ_singlecable_data_t;

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Enable/disable satellite device control function.

 @note  LNB controller will be set to Active/Sleep state.
        Note that satellite device control functions like ::sony_integ_sat_device_ctrl_SetVoltageTone
        automatically enable satellite device control function.

 @param pInteg The integration part instance.
 @param enable Enable/disable setting.
                   - 0: Disable
                   - 1: Enable

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_Enable (sony_integ_t * pInteg, uint8_t enable);

/**
 @brief Change voltage and tone.

 @note  LNB controller and satellite device control function will be automatically enabled.

 @param pInteg The integration part instance which already created.
 @param isVoltageHigh Voltage.
                        - 0: Low voltage.
                        - 1: High voltage.
 @param isContinuousToneOn Continuous tone state.
                            - 0: Don't output continuous tone.
                            - 1: Output continuous tone.
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_SetVoltageTone (sony_integ_t * pInteg,
                                                         uint8_t isVoltageHigh,
                                                         uint8_t isContinuousToneOn);

/**
 @brief Transmit a tone burst.

 @note  LNB controller and satellite device control function will be automatically enabled.

 This API does following flow.
   - Stop continuous tone.
   - Change voltage.
   - Output tone burst.
   - Output continuous tone if required.

 @param pInteg The integration part instance which already created.
 @param isVoltageHigh Voltage.
                        - 0: Low voltage.
                        - 1: High voltage.
 @param toneBurstMode Tone burst mode.
 @param isContinuousToneOn Continuout tone value.
                            - 0: Don't output continuous tone.
                            - 1: Output continuous tone.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_TransmitToneBurst (sony_integ_t * pInteg,
                                                            uint8_t isVoltageHigh,
                                                            sony_toneburst_mode_t toneBurstMode,
                                                            uint8_t isContinuousToneOn);

/**
 @brief Transmit DiSEqC command.

 @note  LNB controller and satellite device control function will be automatically enabled.

 This API does following flow.
   - Stop continuous tone.
   - Change voltage.
   - Output DiSEqC command1.
   - Output DiSEqC command2 if required.
   - (Repeat DiSEqC commands if required.)
   - Output tone burst.
   - Output continuous tone if required.

 @param pInteg The integration part instance which already created.
 @param isVoltageHigh Voltage.
                        - 0: Low voltage.
                        - 1: High voltage.
 @param toneBurstMode Tone burst mode.
 @param isContinuousToneOn Continuout tone value.
                            - 0: Don't output continuous tone.
                            - 1: Output continuous tone.
 @param pCommand1 1st DiSEqC command.
 @param repeatCount1 Repeat count for 1st DiSEqC command.
 @param pCommand2 2nd DiSEqC command.
 @param repeatCount2 Repeat count for 2nd DiSEqC command.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_TransmitDiseqcCommand (sony_integ_t * pInteg,
                                                                uint8_t isVoltageHigh,
                                                                sony_toneburst_mode_t toneBurstMode,
                                                                uint8_t isContinuousToneOn,
                                                                sony_diseqc_message_t * pCommand1,
                                                                uint8_t repeatCount1,
                                                                sony_diseqc_message_t * pCommand2,
                                                                uint8_t repeatCount2);

/**
 @brief Transmit DiSEqC command and receive reply from device.

 @note  LNB controller and satellite device control function will be automatically enabled.

 This API does following flow.
   - Stop continuous tone.
   - Change voltage.
   - Output DiSEqC command.
   - Receive reply message from slave device.
   - Output tone burst.
   - Output continuous tone if required.

 @param pInteg The integration part instance which already created.
 @param isVoltageHigh Voltage.
                        - 0: Low voltage.
                        - 1: High voltage.
 @param toneBurstMode Tone burst mode.
 @param isContinuousToneOn Continuout tone value.
                            - 0: Don't output continuous tone.
                            - 1: Output continuous tone.
 @param pCommand DiSEqC command.
 @param pReply DiSEqC reply from slave device.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_TransmitDiseqcCommandWithReply (sony_integ_t * pInteg,
                                                                         uint8_t isVoltageHigh,
                                                                         sony_toneburst_mode_t toneBurstMode,
                                                                         uint8_t isContinuousToneOn,
                                                                         sony_diseqc_message_t * pCommand,
                                                                         sony_diseqc_message_t * pReply);

/**
 @brief Transmit a single cable command.

 @note  LNB controller and satellite device control function will be automatically enabled.

 @param pInteg The integration part instance which already created.
 @param pCommand The single cable command instance.
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_TransmitSinglecableCommand (sony_integ_t * pInteg,
                                                                     sony_diseqc_message_t * pCommand);


/**
 @brief Transmit a single cable2 command.

 @note  LNB controller and satellite device control function will be automatically enabled.

 @param pInteg The integration part instance which already created.
 @param pCommand The single cable command instance.
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_TransmitSinglecable2Command (sony_integ_t * pInteg,
                                                                      sony_diseqc_message_t * pCommand);

/**
 @brief Transmit a single cable2 command and receive reply from device.

 @note  LNB controller and satellite device control function will be automatically enabled.

 @param pInteg The integration part instance which already created.
 @param pCommand The single cable command instance.
 @param pReply Reply from slave device.
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_TransmitSinglecable2CommandWithReply (sony_integ_t * pInteg,
                                                                               sony_diseqc_message_t * pCommand,
                                                                               sony_diseqc_message_t * pReply);

/**
 @brief Enable Single cable mode.

        If the Single cable mode is set, Tune, TuneSRS, BlindScan functions automatically
        generate and send single cable commands.
        sony_integ_sat_device_ctrl_SetSinglecableTunerParams or sony_integ_sat_device_ctrl_SetSinglecable2TunerParams
        should be called before calling Tune, TuneSRS, BlindScan functions.

 @param pInteg The integration part instance which already created.
 @param pSinglecableData The data for single cable mode.
                         Note that this instance should be kept allocated while single cable function is enabled.
 @param pTransmitByOtherDemod User registered callback is required only when the LNB controller
                              is controlled by the other demodulator. This allows the user to
                              transmit the single cable commands to the LNBC through the other
                              demodulator while taking care of the multi-threaded situations.
                              Set this parameter to NULL, when the LNB controller is controlled
                              by the current demodulator.
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_EnableSinglecable (sony_integ_t * pInteg,
                                                            sony_integ_singlecable_data_t * pSinglecableData,
                                                            sony_result_t (*pTransmitByOtherDemod)(sony_diseqc_message_t * pMessage));

/**
 @brief Disable Single cable mode.

 @param pInteg The integration part instance which already created.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_DisableSinglecable (sony_integ_t * pInteg);

/**
 @brief Change tuning parameters for single cable command.

 @param pInteg The integration part instance.
 @param address Address byte.
 @param bank Signal bank.
 @param ubSlot UB slot number. (1-8)
 @param ubSlotFreqKHz UB slot center frequency in KHz.
 @param enableMDUMode Enable/Disable MDU mode.
                        - 0: Disable.
                        - 1: Enable.
 @param PINCode The valid Pin Code for MDU commands. This value is ignored when the argument enableMDUMode is 0.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_SetSinglecableTunerParams (sony_integ_t * pInteg,
                                                                    sony_singlecable_address_t address,
                                                                    sony_singlecable_bank_t bank,
                                                                    uint8_t ubSlot,
                                                                    uint32_t ubSlotFreqKHz,
                                                                    uint8_t enableMDUMode,
                                                                    uint8_t PINCode);

/**
 @brief Change tuning parameters for SingleCable2 command.

 @param pInteg The integration part instance.
 @param ubSlot UB slot number. (1-32)
 @param ubSlotFreqMHz UB slot center frequency in *MHz*.
 @param uncommittedSwitches Uncommitted switches setting.
 @param committedSwitches Committed switches setting.
 @param enableMDUMode Enable/Disable MDU mode.
                        - 0: Disable.
                        - 1: Enable.
 @param PINCode The valid Pin Code for MDU commands. This value is ignored when the argument enableMDUMode is 0.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_SetSinglecable2TunerParams (sony_integ_t * pInteg,
                                                                     uint8_t ubSlot,
                                                                     uint32_t ubSlotFreqMHz,
                                                                     uint8_t uncommittedSwitches,
                                                                     uint8_t committedSwitches,
                                                                     uint8_t enableMDUMode,
                                                                     uint8_t PINCode);

#endif /* SONY_INTEG_SAT_DEVICE_CTRL_H */
