/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
#include "sony_integ_sat_device_ctrl.h"

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t WaitTransmit (sony_integ_t * pInteg,
                                   uint32_t interval,
                                   uint32_t timeout);

static sony_result_t WaitTransmitSW (sony_integ_t * pInteg,
                                     uint32_t timeout1,
                                     uint32_t timeout2,
                                     uint32_t timeout3);

static sony_result_t singlecableTunerInitialize (sony_tuner_sat_t * pTuner);

static sony_result_t singlecableTunerTune (sony_tuner_sat_t * pTuner,
                                           uint32_t centerFreqKHz,
                                           sony_dtv_system_t dtvSystem,
                                           uint32_t symbolRateKSps);

static sony_result_t singlecableTunerSleep (sony_tuner_sat_t * pTuner);

static sony_result_t singlecableTunerShutdown (sony_tuner_sat_t * pTuner);

static sony_result_t singlecableTunerAGCLevel2AGCdB (sony_tuner_sat_t * pTuner,
                                                     uint32_t AGCLevel,
                                                     int32_t * pAGCdB);

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t sony_integ_sat_device_ctrl_Enable (sony_integ_t * pInteg, uint8_t enable)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_Enable");

    if ((!pInteg) || (!pInteg->pDemod)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_sat_device_ctrl_Enable (pInteg->pDemod, enable);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if (enable) {
        if ((pInteg->pLnbc) && (pInteg->pLnbc->WakeUp)) {
            /* Call the lnb controller WakeUp implementation */
            if (pInteg->pLnbc->state == SONY_LNBC_STATE_SLEEP) {
                result = pInteg->pLnbc->WakeUp (pInteg->pLnbc);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                /* Wait for LNBC stable voltage output */
                SONY_SLEEP (SONY_INTEG_SAT_DEVICE_CTRL_LNBC_WAKEUP_TIME_MS);
            }
        }
    } else {
        if ((pInteg->pLnbc) && (pInteg->pLnbc->Sleep)) {
            /* Sleep the lnb controller */
            result = pInteg->pLnbc->Sleep (pInteg->pLnbc);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_sat_device_ctrl_SetVoltageTone (sony_integ_t * pInteg,
                                                         uint8_t isVoltageHigh,
                                                         uint8_t isContinuousToneOn)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_lnbc_voltage_t voltage;
    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_SetVoltageTone");

    if ((!pInteg) || (!pInteg->pDemod) ||
        (!pInteg->pLnbc) || (!pInteg->pLnbc->SetVoltage)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pLnbc->tone != SONY_LNBC_TONE_AUTO) ||
        (pInteg->pLnbc->voltage == SONY_LNBC_VOLTAGE_AUTO)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Enable satellite device control function */
    result = sony_integ_sat_device_ctrl_Enable (pInteg, 1);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Change voltage by LNB controller driver. */
    voltage = isVoltageHigh ? SONY_LNBC_VOLTAGE_HIGH : SONY_LNBC_VOLTAGE_LOW;
    result = pInteg->pLnbc->SetVoltage (pInteg->pLnbc, voltage);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Change tone by demodulator */
    result = sony_demod_sat_device_ctrl_OutputTone (pInteg->pDemod, isContinuousToneOn);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_sat_device_ctrl_TransmitToneBurst (sony_integ_t * pInteg,
                                                            uint8_t isVoltageHigh,
                                                            sony_toneburst_mode_t toneBurstMode,
                                                            uint8_t isContinuousToneOn)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_lnbc_voltage_t voltage;
    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_TransmitToneBurst");

    if ((!pInteg) || (!pInteg->pDemod) ||
        (!pInteg->pLnbc) || (!pInteg->pLnbc->SetVoltage)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pLnbc->tone != SONY_LNBC_TONE_AUTO) ||
        (pInteg->pLnbc->voltage == SONY_LNBC_VOLTAGE_AUTO)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Enable satellite device control function */
    result = sony_integ_sat_device_ctrl_Enable (pInteg, 1);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (toneBurstMode == SONY_TONEBURST_MODE_OFF){
        result =  sony_integ_sat_device_ctrl_SetVoltageTone (pInteg, isVoltageHigh, isContinuousToneOn);
        SONY_TRACE_RETURN (result);
    }

    /* Stop continuous tone */
    result = sony_demod_sat_device_ctrl_OutputTone (pInteg->pDemod, 0);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Disable DiSEqC command */
    result = sony_demod_sat_device_ctrl_SetDiseqcCommand (pInteg->pDemod, 0, NULL, 0, NULL, 0);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set tone burst */
    result = sony_demod_sat_device_ctrl_SetToneBurst (pInteg->pDemod, toneBurstMode);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set TxIdle time */
    result = sony_demod_sat_device_ctrl_SetTxIdleTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_DISEQC1_TXIDLE_MS);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set repeat to repeat time */
    result = sony_demod_sat_device_ctrl_SetR2RTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_DISEQC1_R2R_TIME_MS);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Disable DiSEqC reply */
    result = sony_demod_sat_device_ctrl_SetDiseqcReplyMode (pInteg->pDemod, 0, 0);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Voltage control */
    voltage = isVoltageHigh ? SONY_LNBC_VOLTAGE_HIGH : SONY_LNBC_VOLTAGE_LOW;
    result = pInteg->pLnbc->SetVoltage (pInteg->pLnbc, voltage);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* === Start transmit === */
    result = sony_demod_sat_device_ctrl_StartTransmit (pInteg->pDemod);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = WaitTransmit(pInteg, SONY_INTEG_DISEQC_TB_POL, 1000);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Continuous tone */
    result = sony_demod_sat_device_ctrl_OutputTone (pInteg->pDemod, isContinuousToneOn);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_sat_device_ctrl_TransmitDiseqcCommand (sony_integ_t * pInteg,
                                                                uint8_t isVoltageHigh,
                                                                sony_toneburst_mode_t toneBurstMode,
                                                                uint8_t isContinuousToneOn,
                                                                sony_diseqc_message_t * pCommand1,
                                                                uint8_t repeatCount1,
                                                                sony_diseqc_message_t * pCommand2,
                                                                uint8_t repeatCount2)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_lnbc_voltage_t voltage;
    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_TransmitDiseqcCommand");

    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pLnbc) || (!pInteg->pLnbc->SetVoltage) ||
        (!pCommand1) || (repeatCount1 == 0) || (repeatCount1 >= 16) || (repeatCount2 >= 16)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pLnbc->tone != SONY_LNBC_TONE_AUTO) ||
        (pInteg->pLnbc->voltage == SONY_LNBC_VOLTAGE_AUTO)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Clear cancel flag */
    sony_atomic_set (&(pInteg->cancel), 0);

    /* Enable satellite device control function */
    result = sony_integ_sat_device_ctrl_Enable (pInteg, 1);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Stop continuous tone */
    result = sony_demod_sat_device_ctrl_OutputTone (pInteg->pDemod, 0);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set DiSEqC command */
    result = sony_demod_sat_device_ctrl_SetDiseqcCommand (pInteg->pDemod, 1, pCommand1, repeatCount1, pCommand2, repeatCount2);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set tone burst */
    result = sony_demod_sat_device_ctrl_SetToneBurst (pInteg->pDemod, toneBurstMode);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set TxIdle time */
    result = sony_demod_sat_device_ctrl_SetTxIdleTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_DISEQC1_TXIDLE_MS);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set repeat to repeat time */
    result = sony_demod_sat_device_ctrl_SetR2RTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_DISEQC1_R2R_TIME_MS);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Disable DiSEqC reply */
    result = sony_demod_sat_device_ctrl_SetDiseqcReplyMode (pInteg->pDemod, 0, 0);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Voltage control */
    voltage = isVoltageHigh ? SONY_LNBC_VOLTAGE_HIGH : SONY_LNBC_VOLTAGE_LOW;
    result = pInteg->pLnbc->SetVoltage (pInteg->pLnbc, voltage);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* === Start transmit === */
    result = sony_demod_sat_device_ctrl_StartTransmit (pInteg->pDemod);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = WaitTransmit(pInteg,
                          SONY_INTEG_DISEQC_DISEQC_TRANSMIT_POL,
                          (uint32_t)1000 * (repeatCount1 + repeatCount2));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Continuous tone */
    result = sony_demod_sat_device_ctrl_OutputTone (pInteg->pDemod, isContinuousToneOn);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_sat_device_ctrl_TransmitDiseqcCommandWithReply (sony_integ_t * pInteg,
                                                                         uint8_t isVoltageHigh,
                                                                         sony_toneburst_mode_t toneBurstMode,
                                                                         uint8_t isContinuousToneOn,
                                                                         sony_diseqc_message_t * pCommand,
                                                                         sony_diseqc_message_t * pReply)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_lnbc_voltage_t voltage;
    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_TransmitDiseqcCommandWithReply");

    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pLnbc) || (!pInteg->pLnbc->SetVoltage) ||
        (!pCommand) || (!pReply)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pLnbc->tone != SONY_LNBC_TONE_AUTO) ||
        (pInteg->pLnbc->voltage == SONY_LNBC_VOLTAGE_AUTO)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Clear cancel flag */
    sony_atomic_set (&(pInteg->cancel), 0);

    /* Enable satellite device control function */
    result = sony_integ_sat_device_ctrl_Enable (pInteg, 1);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_sat_device_ctrl_SetDiseqcReplyMask (pInteg->pDemod, 0xE4, 0x03);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Stop continuous tone */
    result = sony_demod_sat_device_ctrl_OutputTone (pInteg->pDemod, 0);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set DiSEqC command */
    result = sony_demod_sat_device_ctrl_SetDiseqcCommand (pInteg->pDemod, 1, pCommand, 1, NULL, 0);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set tone burst */
    result = sony_demod_sat_device_ctrl_SetToneBurst (pInteg->pDemod, toneBurstMode);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set TxIdle time */
    result = sony_demod_sat_device_ctrl_SetTxIdleTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_DISEQC2_TXIDLE_MS);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set RxIdle time */
    result = sony_demod_sat_device_ctrl_SetRxIdleTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_DISEQC2_RXIDLE_MS);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set repeat to repeat time */
    result = sony_demod_sat_device_ctrl_SetR2RTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_DISEQC1_R2R_TIME_MS);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Enable DiSEqC reply */
    result = sony_demod_sat_device_ctrl_SetDiseqcReplyMode (pInteg->pDemod, 1, SONY_INTEG_SAT_DEVICE_CTRL_DISEQC2_RTO_TIME_MS);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Voltage control */
    voltage = isVoltageHigh ? SONY_LNBC_VOLTAGE_HIGH : SONY_LNBC_VOLTAGE_LOW;
    result = pInteg->pLnbc->SetVoltage (pInteg->pLnbc, voltage);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* === Start transmit === */
    result = sony_demod_sat_device_ctrl_StartTransmit (pInteg->pDemod);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (pInteg->pLnbc->transmitMode == SONY_LNBC_TRANSMIT_MODE_AUTO) {
        result = WaitTransmit(pInteg, SONY_INTEG_DISEQC_DISEQC_TRANSMIT_POL, 1400);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    } else {
        result = WaitTransmitSW(pInteg, 500, 600, 1400);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    }

    /* Output continuous tone if required. */
    result = sony_demod_sat_device_ctrl_OutputTone (pInteg->pDemod, isContinuousToneOn);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_sat_device_ctrl_GetReplyMessage (pInteg->pDemod, pReply);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_sat_device_ctrl_TransmitSinglecableCommand (sony_integ_t * pInteg,
                                                                     sony_diseqc_message_t * pCommand)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_result_t tempResult = SONY_RESULT_OK;
    uint8_t enableTXEN = 0;
    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_TransmitSinglecableCommand");

    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pLnbc) || (!pInteg->pLnbc->SetVoltage) || (!pCommand)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pLnbc->tone != SONY_LNBC_TONE_AUTO){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Clear cancel flag */
    sony_atomic_set (&(pInteg->cancel), 0);

    /* Enable satellite device control function */
    result = sony_integ_sat_device_ctrl_Enable (pInteg, 1);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_sat_device_ctrl_GetTXENMode (pInteg->pDemod, &enableTXEN);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Stop continuous tone. */
    result = sony_demod_sat_device_ctrl_OutputTone (pInteg->pDemod, 0);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set tone burst (Disable). */
    result = sony_demod_sat_device_ctrl_SetToneBurst (pInteg->pDemod, SONY_TONEBURST_MODE_OFF);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set single cable command. */
    result = sony_demod_sat_device_ctrl_SetDiseqcCommand (pInteg->pDemod, 1, pCommand, 1, NULL, 0);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set TxIdle time */
    result = sony_demod_sat_device_ctrl_SetTxIdleTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_TXIDLE_MS);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set repeat to repeat time (Fixed to 0ms) */
    result = sony_demod_sat_device_ctrl_SetR2RTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_R2R_TIME_MS);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set reply mode (Disable). */
    result = sony_demod_sat_device_ctrl_SetDiseqcReplyMode (pInteg->pDemod, 0, 0);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    if (enableTXEN == 0){
        /* Change voltage to "High" */
        result = pInteg->pLnbc->SetVoltage (pInteg->pLnbc, SONY_LNBC_VOLTAGE_HIGH);
        if (result != SONY_RESULT_OK){
            goto End;
        }
    }

    /* === Start transmit === */
    result = sony_demod_sat_device_ctrl_StartTransmit (pInteg->pDemod);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Wait for finish transmit */
    result = WaitTransmit (pInteg, SONY_INTEG_DISEQC_SINGLECABLE_TRANSMIT_POL, 1000);
    if (result != SONY_RESULT_OK){
        goto End;
    }

End:
    if (enableTXEN == 0){
        /* Change voltage to "Low" */
        tempResult = pInteg->pLnbc->SetVoltage (pInteg->pLnbc, SONY_LNBC_VOLTAGE_LOW);
        if (tempResult != SONY_RESULT_OK){
            SONY_TRACE_RETURN (tempResult);
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_sat_device_ctrl_TransmitSinglecable2Command (sony_integ_t * pInteg,
                                                                      sony_diseqc_message_t * pCommand)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_result_t tempResult = SONY_RESULT_OK;
    uint8_t enableTXEN = 0;
    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_TransmitSinglecable2Command");

    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pLnbc) || (!pInteg->pLnbc->SetVoltage) || (!pCommand)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pLnbc->tone != SONY_LNBC_TONE_AUTO){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Clear cancel flag */
    sony_atomic_set (&(pInteg->cancel), 0);

    result = sony_demod_sat_device_ctrl_GetTXENMode (pInteg->pDemod, &enableTXEN);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* TXEN is not available for Single Cable 2 */
    if (enableTXEN) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /* Enable satellite device control function */
    result = sony_integ_sat_device_ctrl_Enable (pInteg, 1);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Stop continuous tone. */
    result = sony_demod_sat_device_ctrl_OutputTone (pInteg->pDemod, 0);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set tone burst (Disable). */
    result = sony_demod_sat_device_ctrl_SetToneBurst (pInteg->pDemod, SONY_TONEBURST_MODE_OFF);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set single cable command. */
    result = sony_demod_sat_device_ctrl_SetDiseqcCommand (pInteg->pDemod, 1, pCommand, 1, NULL, 0);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set TxIdle time */
    result = sony_demod_sat_device_ctrl_SetTxIdleTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_TXIDLE_MS);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set repeat to repeat time (Fixed to 0ms) */
    result = sony_demod_sat_device_ctrl_SetR2RTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_R2R_TIME_MS);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set reply mode (Disable). */
    result = sony_demod_sat_device_ctrl_SetDiseqcReplyMode (pInteg->pDemod, 0, 0);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Change voltage to "High" */
    result = pInteg->pLnbc->SetVoltage (pInteg->pLnbc, SONY_LNBC_VOLTAGE_HIGH);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* === Start transmit === */
    result = sony_demod_sat_device_ctrl_StartTransmit (pInteg->pDemod);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Wait for finish transmit */
    result = WaitTransmit (pInteg, SONY_INTEG_DISEQC_SINGLECABLE_TRANSMIT_POL, 1000);
    if (result != SONY_RESULT_OK){
        goto End;
    }

End:
    /* Change voltage to "Low" */
    tempResult = pInteg->pLnbc->SetVoltage (pInteg->pLnbc, SONY_LNBC_VOLTAGE_LOW);
    if (tempResult != SONY_RESULT_OK){
        SONY_TRACE_RETURN (tempResult);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_sat_device_ctrl_TransmitSinglecable2CommandWithReply (sony_integ_t * pInteg,
                                                                               sony_diseqc_message_t * pCommand,
                                                                               sony_diseqc_message_t * pReply)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_result_t tempResult = SONY_RESULT_OK;
    uint8_t enableTXEN = 0;
    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_TransmitSinglecable2CommandWithReply");

    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pLnbc) || (!pInteg->pLnbc->SetVoltage) ||
        (!pCommand) || (!pReply)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pLnbc->tone != SONY_LNBC_TONE_AUTO){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Clear cancel flag */
    sony_atomic_set (&(pInteg->cancel), 0);

    result = sony_demod_sat_device_ctrl_GetTXENMode (pInteg->pDemod, &enableTXEN);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* TXEN is not available for Single Cable 2 */
    if (enableTXEN) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /* Enable satellite device control function */
    result = sony_integ_sat_device_ctrl_Enable (pInteg, 1);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_sat_device_ctrl_SetDiseqcReplyMask (pInteg->pDemod, 0x74, 0x93);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Stop continuous tone */
    result = sony_demod_sat_device_ctrl_OutputTone (pInteg->pDemod, 0);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set DiSEqC command */
    result = sony_demod_sat_device_ctrl_SetDiseqcCommand (pInteg->pDemod, 1, pCommand, 1, NULL, 0);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set tone burst */
    result = sony_demod_sat_device_ctrl_SetToneBurst (pInteg->pDemod, SONY_TONEBURST_MODE_OFF);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set TxIdle time */
    result = sony_demod_sat_device_ctrl_SetTxIdleTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_TXIDLE_MS);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set RxIdle time */
    result = sony_demod_sat_device_ctrl_SetRxIdleTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_RXIDLE_MS);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Set repeat to repeat time */
    result = sony_demod_sat_device_ctrl_SetR2RTime (pInteg->pDemod, SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_R2R_TIME_MS);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Enable Reply */
    result = sony_demod_sat_device_ctrl_SetDiseqcReplyMode (pInteg->pDemod, 1, SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE2REPLY_RTO_TIME_MS);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* Change voltage to "High" */
    result = pInteg->pLnbc->SetVoltage (pInteg->pLnbc, SONY_LNBC_VOLTAGE_HIGH);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    /* === Start transmit === */
    result = sony_demod_sat_device_ctrl_StartTransmit (pInteg->pDemod);
    if (result != SONY_RESULT_OK){
        goto End;
    }

    if (pInteg->pLnbc->transmitMode == SONY_LNBC_TRANSMIT_MODE_AUTO) {
        result = WaitTransmit(pInteg, SONY_INTEG_DISEQC_SINGLECABLE_TRANSMIT_POL, 1400);
        if (result != SONY_RESULT_OK){
            goto End;
        }
    } else {
        result = WaitTransmitSW(pInteg, 500, 600, 1400);
        if (result != SONY_RESULT_OK){
            goto End;
        }
    }

End:
    /* Change voltage to "Low" */
    tempResult = pInteg->pLnbc->SetVoltage (pInteg->pLnbc, SONY_LNBC_VOLTAGE_LOW);
    if (tempResult != SONY_RESULT_OK){
        SONY_TRACE_RETURN (tempResult);
    }

    if (result == SONY_RESULT_OK) {
        result = sony_demod_sat_device_ctrl_GetReplyMessage (pInteg->pDemod, pReply);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_sat_device_ctrl_EnableSinglecable (sony_integ_t * pInteg,
                                                            sony_integ_singlecable_data_t * pSinglecableData,
                                                            sony_result_t (*pTransmitByOtherDemod)(sony_diseqc_message_t * pMessage))
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_EnableSinglecable");

    if ((!pInteg) || (!pInteg->pDemod) ||
        (!pInteg->pTunerSat) || (!pInteg->pTunerSat->Initialize) ||
        (!pSinglecableData)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->isSinglecable){
        /* Current Demod/Tuner is already configured for Single Cable. */
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    } else {
        pSinglecableData->tunerSinglecableData.pInteg = pInteg;
        pSinglecableData->tunerSinglecableData.pTunerReal = pInteg->pTunerSat;
        pSinglecableData->tunerSinglecableData.version = SONY_SINGLECABLE_VERSION_UNKNOWN;
        pSinglecableData->tunerSinglecable.Initialize = singlecableTunerInitialize;
        pSinglecableData->tunerSinglecable.Tune = singlecableTunerTune;
        pSinglecableData->tunerSinglecable.Sleep = singlecableTunerSleep;
        pSinglecableData->tunerSinglecable.Shutdown = singlecableTunerShutdown;
        pSinglecableData->tunerSinglecable.AGCLevel2AGCdB = singlecableTunerAGCLevel2AGCdB;
        pSinglecableData->tunerSinglecable.user = &pSinglecableData->tunerSinglecableData;
        pSinglecableData->tunerSinglecableData.pTransmitByOtherDemod = pTransmitByOtherDemod;
        pInteg->pTunerSat = &pSinglecableData->tunerSinglecable;
        result = sony_demod_sat_device_ctrl_EnableSinglecable (pInteg->pDemod, 1);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_integ_sat_device_ctrl_DisableSinglecable (sony_integ_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_integ_singlecable_tuner_data_t * pSinglecableTunerData = NULL;

    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_DisableSinglecable");

    if ((!pInteg) || (!pInteg->pDemod) ||
        (!pInteg->pTunerSat) || (!pInteg->pTunerSat->Initialize)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->isSinglecable){
        if (pInteg->pTunerSat->user) {
            /* Current Demod/Tuner is already configured for Single Cable. */
            pSinglecableTunerData = (sony_integ_singlecable_tuner_data_t*)(pInteg->pTunerSat->user);
            pInteg->pTunerSat = pSinglecableTunerData->pTunerReal;
            result = sony_demod_sat_device_ctrl_EnableSinglecable (pInteg->pDemod, 0);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        } else {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
    } else {
        /* Not single cable mode. */
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_integ_sat_device_ctrl_SetSinglecableTunerParams (sony_integ_t * pInteg,
                                                                    sony_singlecable_address_t address,
                                                                    sony_singlecable_bank_t bank,
                                                                    uint8_t ubSlot,
                                                                    uint32_t ubSlotFreqKHz,
                                                                    uint8_t enableMDUMode,
                                                                    uint8_t PINCode)
{
    sony_integ_singlecable_tuner_data_t * pSinglecableTunerData = NULL;

    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_SetSinglecableTunerParams");

    if ((!pInteg) || (!pInteg->pTunerSat) || (!pInteg->pTunerSat->user)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (!pInteg->pDemod->isSinglecable){
        /* Current Demod/Tuner is not configured for Single Cable. */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    pSinglecableTunerData = (sony_integ_singlecable_tuner_data_t*)pInteg->pTunerSat->user;
    pSinglecableTunerData->version = SONY_SINGLECABLE_VERSION_1_EN50494;
    pSinglecableTunerData->address = address;
    pSinglecableTunerData->bank = bank;
    pSinglecableTunerData->ubSlot = ubSlot;
    pSinglecableTunerData->ubSlotFreqKHz = ubSlotFreqKHz;
    pSinglecableTunerData->enableMDUMode = enableMDUMode;
    pSinglecableTunerData->PINCode = PINCode;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_integ_sat_device_ctrl_SetSinglecable2TunerParams (sony_integ_t * pInteg,
                                                                     uint8_t ubSlot,
                                                                     uint32_t ubSlotFreqMHz,
                                                                     uint8_t uncommittedSwitches,
                                                                     uint8_t committedSwitches,
                                                                     uint8_t enableMDUMode,
                                                                     uint8_t PINCode)
{
    sony_integ_singlecable_tuner_data_t * pSinglecableTunerData = NULL;

    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_SetSinglecable2TunerParams");

    if ((!pInteg) || (!pInteg->pTunerSat) || (!pInteg->pTunerSat->user)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (!pInteg->pDemod->isSinglecable){
        /* Current Demod/Tuner is not configured for Single Cable. */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    pSinglecableTunerData = (sony_integ_singlecable_tuner_data_t*)pInteg->pTunerSat->user;
    pSinglecableTunerData->version = SONY_SINGLECABLE_VERSION_2_EN50607;
    pSinglecableTunerData->scd2_ubSlot = ubSlot;
    pSinglecableTunerData->scd2_ubSlotFreqMHz = ubSlotFreqMHz;
    pSinglecableTunerData->scd2_uncommittedSwitches = uncommittedSwitches;
    pSinglecableTunerData->scd2_committedSwitches = committedSwitches;
    pSinglecableTunerData->scd2_enableMDUMode = enableMDUMode;
    pSinglecableTunerData->scd2_PINCode = PINCode;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t WaitTransmit (sony_integ_t * pInteg,
                                   uint32_t interval,
                                   uint32_t timeout)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t isContinue = 0;
    uint8_t transmitStatus = 0;
    uint32_t elapsedTime = 0;
    sony_stopwatch_t stopwatch;

    SONY_TRACE_ENTER ("WaitTransmit");

    if (!pInteg) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_stopwatch_start(&stopwatch);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    isContinue = 1;
    while (isContinue){
        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_stopwatch_elapsed (&stopwatch, &elapsedTime);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = sony_demod_sat_device_ctrl_GetTransmitStatus (pInteg->pDemod, &transmitStatus);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        if (transmitStatus == 0x00){
            /* Success */
            SONY_TRACE_RETURN (SONY_RESULT_OK);
        } else {
            if(elapsedTime > timeout){
                /* Timeout error */
                isContinue = 0;
            } else {
                result = sony_stopwatch_sleep(&stopwatch, interval);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }
            }
        }
    }
    SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
}

static sony_result_t WaitTransmitSW (sony_integ_t * pInteg,
                                     uint32_t timeout1,
                                     uint32_t timeout2,
                                     uint32_t timeout3)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t isContinue = 0;
    uint8_t transmitStatus = 0;
    uint32_t elapsedTime = 0;
    sony_stopwatch_t stopwatch;

    SONY_TRACE_ENTER ("WaitTransmitSW");

    if (!pInteg) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_stopwatch_start (&stopwatch);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    isContinue = 1;
    while (isContinue){
        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_stopwatch_elapsed (&stopwatch, &elapsedTime);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = sony_demod_sat_device_ctrl_GetTransmitStatus (pInteg->pDemod, &transmitStatus);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        if (transmitStatus == 0x02){
            /* Go to next step */
            isContinue = 0;
        } else {
            if (elapsedTime > timeout1){
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
            }
        }
    }

    isContinue = 1;
    while (isContinue){
        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_stopwatch_elapsed (&stopwatch, &elapsedTime);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = sony_demod_sat_device_ctrl_GetTransmitStatus (pInteg->pDemod, &transmitStatus);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        if (transmitStatus != 0x02){
            /* Go to next step */
            isContinue = 0;
        } else {
            if (elapsedTime > timeout2){
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
            }
        }
    }

    result = pInteg->pLnbc->SetTransmitMode (pInteg->pLnbc, SONY_LNBC_TRANSMIT_MODE_RX);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    isContinue = 1;
    while (isContinue){
        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_stopwatch_elapsed (&stopwatch, &elapsedTime);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = sony_demod_sat_device_ctrl_GetTransmitStatus (pInteg->pDemod, &transmitStatus);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        if (transmitStatus == 0x00){
            /* Go to next step */
            isContinue = 0;
        } else {
            if (elapsedTime > timeout3){
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
            }
        }
    }

    result = pInteg->pLnbc->SetTransmitMode (pInteg->pLnbc, SONY_LNBC_TRANSMIT_MODE_TX);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*------------------------------------------------------------------------------
  Static Functions for dummy tuner
------------------------------------------------------------------------------*/
static sony_result_t singlecableTunerInitialize (sony_tuner_sat_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_integ_singlecable_tuner_data_t * pSinglecableTunerData = NULL;

    SONY_TRACE_ENTER ("singlecableTunerInitialize");

    if ((!pTuner) || (!pTuner->user)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSinglecableTunerData = (sony_integ_singlecable_tuner_data_t*)pTuner->user;

    result = pSinglecableTunerData->pTunerReal->Initialize (pSinglecableTunerData->pTunerReal);

    SONY_TRACE_RETURN (result);
}

static sony_result_t singlecableTunerTune (sony_tuner_sat_t * pTuner,
                                           uint32_t centerFreqKHz,
                                           sony_dtv_system_t dtvSystem,
                                           uint32_t symbolRateKSps)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_integ_singlecable_tuner_data_t * pSinglecableTunerData = NULL;
    sony_diseqc_message_t command;
    uint32_t scFreqKHz = 0;
    uint32_t tuFreqKHz = 0;

    SONY_TRACE_ENTER ("singlecableTunerTune");

    if ((!pTuner) || (!pTuner->user)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSinglecableTunerData = (sony_integ_singlecable_tuner_data_t*) pTuner->user;

    /* Create command */

    if (pSinglecableTunerData->version == SONY_SINGLECABLE_VERSION_1_EN50494) {
        if (pSinglecableTunerData->enableMDUMode) {
            result = sony_singlecable_command_ODU_Channel_change_MDU (&command,
                                                                      pSinglecableTunerData->address,
                                                                      pSinglecableTunerData->ubSlot,
                                                                      pSinglecableTunerData->ubSlotFreqKHz,
                                                                      pSinglecableTunerData->bank,
                                                                      centerFreqKHz,
                                                                      pSinglecableTunerData->PINCode);
        } else {
            result = sony_singlecable_command_ODU_Channel_change (&command,
                                                                  pSinglecableTunerData->address,
                                                                  pSinglecableTunerData->ubSlot,
                                                                  pSinglecableTunerData->ubSlotFreqKHz,
                                                                  pSinglecableTunerData->bank,
                                                                  centerFreqKHz);
        }
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        scFreqKHz = ((uint32_t) (command.data[3] & 0x03) << 8) | (uint32_t) (command.data[4] & 0xFF);
        scFreqKHz = ((scFreqKHz + 350) * 4000) - pSinglecableTunerData->ubSlotFreqKHz;

        if (pSinglecableTunerData->pTransmitByOtherDemod) {
            /* Transmit command by user's function */
            result = pSinglecableTunerData->pTransmitByOtherDemod (&command);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        } else {
            /* Transmit command */
            result = sony_integ_sat_device_ctrl_TransmitSinglecableCommand (pSinglecableTunerData->pInteg, &command);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }

        /* Calculate tuner tune frequency */
        if (pSinglecableTunerData->pInteg->pDemod->isSinglecableIqInv == 1) {
            /* Normal -> Invert(in case of single cable) */
            tuFreqKHz = pSinglecableTunerData->ubSlotFreqKHz - (centerFreqKHz - scFreqKHz);
        } else {
            /* Invert -> Normal(in case of single cable) */
            tuFreqKHz = pSinglecableTunerData->ubSlotFreqKHz + (centerFreqKHz - scFreqKHz);
        }
    } else if (pSinglecableTunerData->version == SONY_SINGLECABLE_VERSION_2_EN50607) {
        uint32_t scFreqMHz = (centerFreqKHz + 500) / 1000;
        if (scFreqMHz > 2147) {
            scFreqMHz = 2147;
        }

        if (pSinglecableTunerData->scd2_enableMDUMode) {
            result = sony_singlecable2_command_ODU_Channel_change_PIN (&command,
                                                                       pSinglecableTunerData->scd2_ubSlot,
                                                                       scFreqMHz,
                                                                       pSinglecableTunerData->scd2_uncommittedSwitches,
                                                                       pSinglecableTunerData->scd2_committedSwitches,
                                                                       pSinglecableTunerData->scd2_PINCode);
        } else {
            result = sony_singlecable2_command_ODU_Channel_change (&command,
                                                                   pSinglecableTunerData->scd2_ubSlot,
                                                                   scFreqMHz,
                                                                   pSinglecableTunerData->scd2_uncommittedSwitches,
                                                                   pSinglecableTunerData->scd2_committedSwitches);
        }
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        scFreqKHz = scFreqMHz * 1000;

        if (pSinglecableTunerData->pTransmitByOtherDemod) {
            /* Transmit command by user's function */
            result = pSinglecableTunerData->pTransmitByOtherDemod (&command);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        } else {
            /* Transmit command */
            result = sony_integ_sat_device_ctrl_TransmitSinglecable2Command (pSinglecableTunerData->pInteg, &command);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        /* Calculate tuner tune frequency */
        if (pSinglecableTunerData->pInteg->pDemod->isSinglecableIqInv == 1) {
            /* Normal -> Invert(in case of single cable) */
            tuFreqKHz = (pSinglecableTunerData->scd2_ubSlotFreqMHz * 1000) - (centerFreqKHz - scFreqKHz);
        } else {
            /* Invert -> Normal(in case of single cable) */
            tuFreqKHz = (pSinglecableTunerData->scd2_ubSlotFreqMHz * 1000) + (centerFreqKHz - scFreqKHz);
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    if (symbolRateKSps == 0) {
        /* Use symbol rate for spectrum. */
        symbolRateKSps = pSinglecableTunerData->pTunerReal->symbolRateKSpsForSpectrum;
    }

    /* Tune to UB slot */
    result = pSinglecableTunerData->pTunerReal->Tune (pSinglecableTunerData->pTunerReal, tuFreqKHz, dtvSystem, symbolRateKSps);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (pSinglecableTunerData->version == SONY_SINGLECABLE_VERSION_1_EN50494) {
        if (pSinglecableTunerData->pInteg->pDemod->isSinglecableIqInv == 1){
            /* Normal -> Invert(in case of single cable) */
            pTuner->frequencyKHz = scFreqKHz - (pSinglecableTunerData->pTunerReal->frequencyKHz - pSinglecableTunerData->ubSlotFreqKHz);
        } else {
            /* Invert -> Normal(in case of single cable) */
            pTuner->frequencyKHz = scFreqKHz + (pSinglecableTunerData->pTunerReal->frequencyKHz - pSinglecableTunerData->ubSlotFreqKHz);
        }
    } else {
        if (pSinglecableTunerData->pInteg->pDemod->isSinglecableIqInv == 1) {
            /* Normal -> Invert(in case of single cable) */
            pTuner->frequencyKHz = scFreqKHz - (pSinglecableTunerData->pTunerReal->frequencyKHz - (pSinglecableTunerData->scd2_ubSlotFreqMHz * 1000));
        } else {
            /* Invert -> Normal(in case of single cable) */
            pTuner->frequencyKHz = scFreqKHz + (pSinglecableTunerData->pTunerReal->frequencyKHz - (pSinglecableTunerData->scd2_ubSlotFreqMHz * 1000));
        }
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t singlecableTunerSleep (sony_tuner_sat_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_integ_singlecable_tuner_data_t * pSinglecableTunerData = NULL;

    SONY_TRACE_ENTER ("singlecableTunerSleep");

    if ((!pTuner) || (!pTuner->user)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSinglecableTunerData = (sony_integ_singlecable_tuner_data_t*)pTuner->user;

    result = pSinglecableTunerData->pTunerReal->Sleep (pSinglecableTunerData->pTunerReal);

    SONY_TRACE_RETURN (result);
}

static sony_result_t singlecableTunerShutdown (sony_tuner_sat_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_integ_singlecable_tuner_data_t * pSinglecableTunerData = NULL;

    SONY_TRACE_ENTER ("singlecableTunerShutdown");

    if ((!pTuner) || (!pTuner->user)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSinglecableTunerData = (sony_integ_singlecable_tuner_data_t*)pTuner->user;

    result = pSinglecableTunerData->pTunerReal->Shutdown (pSinglecableTunerData->pTunerReal);

    SONY_TRACE_RETURN (result);
}
static sony_result_t singlecableTunerAGCLevel2AGCdB (sony_tuner_sat_t * pTuner,
                                                     uint32_t AGCLevel,
                                                     int32_t * pAGCdB)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_integ_singlecable_tuner_data_t * pSinglecableTunerData = NULL;

    SONY_TRACE_ENTER ("singlecableTunerAGCLevel2AGCdB");

    if ((!pTuner) || (!pTuner->user) || (!pAGCdB)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSinglecableTunerData = (sony_integ_singlecable_tuner_data_t*)pTuner->user;

    result = pSinglecableTunerData->pTunerReal->AGCLevel2AGCdB (pSinglecableTunerData->pTunerReal, AGCLevel, pAGCdB);

    SONY_TRACE_RETURN (result);
}
