/*------------------------------------------------------------------------------
  Copyright 2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
#include "sony_integ_sat_device_ctrl_singlecable_auto_install.h"
#include "sony_integ_sat_device_ctrl.h"

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t getCWPower (sony_integ_t * pInteg,
                                 sony_demod_sat_device_ctrl_detect_cw_seq_t * pSeq,
                                 uint32_t freqKHz,
                                 uint32_t rangeKHz,
                                 int32_t * pPower);

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t sony_integ_sat_device_ctrl_Find1UB (sony_integ_t * pInteg,
                                                  sony_demod_sat_device_ctrl_find1ub_seq_t * pSeq,
                                                  uint32_t minFreqKHz,
                                                  uint32_t maxFreqKHz,
                                                  uint8_t targetUB,
                                                  uint32_t * pFreqKHz)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t elapsedTime = 0;
    sony_stopwatch_t stopwatch;
    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_Find1UB");

    if ((!pInteg) || (!pInteg->pDemod) || (!pSeq) || (!pFreqKHz) ||
        (!pInteg->pTunerSat) || (!pInteg->pTunerSat->AGCLevel2AGCdB) ||
        (!pInteg->pTunerSat->Tune)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pInteg->pDemod->isSinglecable){
        /* Current Demod/Tuner is configured for Single Cable but
           this API is accepted in non-Single Cable (Normal) mode only. */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    sony_atomic_set (&(pInteg->cancel), 0);

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
    /* Sleep terrestrial/cable tuner if active. */
    result = sony_integ_terr_cable_SetTunerSleep (pInteg);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
#endif

    /* Enable satellite device control function */
    result = sony_integ_sat_device_ctrl_Enable (pInteg, 1);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_sat_device_ctrl_find1ub_seq_Initialize (pSeq, pInteg->pDemod, minFreqKHz, maxFreqKHz, targetUB);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    while(pSeq->isContinue){
        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (pSeq->commonParams.waitTime == 0){
            /* Execute one sequence */
            result = sony_demod_sat_device_ctrl_find1ub_seq_Sequence (pSeq);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            /* Start stopwatch */
            result = sony_stopwatch_start (&stopwatch);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            /* Transmitting single cable command is requested */
            if (pSeq->commandReq){
                pSeq->commandReq = 0;

                result = sony_integ_sat_device_ctrl_TransmitSinglecableCommand(pInteg, &pSeq->message);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
            }

            /* AGC calculate is requested */
            if (pSeq->commonParams.agcInfo.isRequest){

                /* Clear request flag. */
                pSeq->commonParams.agcInfo.isRequest = 0;

                result = pInteg->pTunerSat->AGCLevel2AGCdB (pInteg->pTunerSat,
                                                            pSeq->commonParams.agcInfo.agcLevel,
                                                            &(pSeq->commonParams.agcInfo.agc_x100dB));
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
            }

            /* Tune is requested */
            if (pSeq->commonParams.tuneReq.isRequest){

                /* Clear request flag. */
                pSeq->commonParams.tuneReq.isRequest = 0;

                /* Enable the I2C repeater */
                result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                /* RF Tune */
                result = pInteg->pTunerSat->Tune (pInteg->pTunerSat,
                                                  pSeq->commonParams.tuneReq.frequencyKHz,
                                                  pSeq->commonParams.tuneReq.system,
                                                  pSeq->commonParams.tuneReq.symbolRateKSps);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                /* Set actual frequency to the driver. */
                pSeq->commonParams.tuneReq.frequencyKHz = pInteg->pTunerSat->frequencyKHz;
                /* Disable the I2C repeater */
                result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
            }
        } else {
            /* waiting */
            result = sony_stopwatch_sleep (&stopwatch, 10);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            result = sony_stopwatch_elapsed (&stopwatch, &elapsedTime);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            if(elapsedTime > pSeq->commonParams.waitTime){
                pSeq->commonParams.waitTime = 0;
            }
        }
    }

    *pFreqKHz = pSeq->ubFreqKHz;

    result = sony_integ_Sleep (pInteg);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_sat_device_ctrl_DetectYesNo (sony_integ_t * pInteg,
                                                      sony_demod_sat_device_ctrl_detect_cw_seq_t * pSeq,
                                                      uint32_t ubFreqKHz,
                                                      uint32_t noFreqOffsetKHz,
                                                      uint32_t yesMarginFreqKHz,
                                                      uint32_t noMarginFreqKHz,
                                                      sony_integ_sat_device_ctrl_detect_yes_no_result_t * pResult)
{
    sony_result_t result = SONY_RESULT_OK;
    int32_t yesPower = 0;
    int32_t noPower = 0;

    SONY_TRACE_ENTER ("sony_integ_sat_device_ctrl_DetectYesNo");

    if ((!pInteg) || (!pInteg->pDemod) || (!pSeq) || (!pResult)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pInteg->pDemod->isSinglecable){
        /* Current Demod/Tuner is configured for Single Cable but
           this API is accepted in non-Single Cable (Normal) mode only. */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    sony_atomic_set (&(pInteg->cancel), 0);

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
    /* Sleep terrestrial/cable tuner if active. */
    result = sony_integ_terr_cable_SetTunerSleep (pInteg);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
#endif

    result = getCWPower (pInteg, pSeq, ubFreqKHz, yesMarginFreqKHz, &yesPower);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = getCWPower (pInteg, pSeq, ubFreqKHz + noFreqOffsetKHz, noMarginFreqKHz, &noPower);
    if(result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if(yesPower >= noPower + 1000){
        /* Return YES. */
        *pResult = SONY_INTEG_SAT_DEVICE_CTRL_DETECT_YES_NO_RESULT_YES;
    } else if(noPower >= yesPower + 1000) {
        /* Return NO. */
        *pResult = SONY_INTEG_SAT_DEVICE_CTRL_DETECT_YES_NO_RESULT_NO;
    } else {
        /* Return NOT Detected. */
        *pResult = SONY_INTEG_SAT_DEVICE_CTRL_DETECT_YES_NO_RESULT_NOTDETECT;
    }

    result = sony_integ_Sleep (pInteg);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t getCWPower (sony_integ_t * pInteg,
                                 sony_demod_sat_device_ctrl_detect_cw_seq_t * pSeq,
                                 uint32_t freqKHz,
                                 uint32_t rangeKHz,
                                 int32_t * pPower)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t elapsedTime = 0;
    sony_stopwatch_t stopwatch;
    SONY_TRACE_ENTER ("getCWPower");

    if ((!pInteg) || (!pInteg->pDemod) || (!pSeq) || (!pPower) ||
        (!pInteg->pTunerSat) || (!pInteg->pTunerSat->AGCLevel2AGCdB)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_sat_device_ctrl_detect_cw_seq_Initialize (pSeq, pInteg->pDemod, freqKHz, rangeKHz);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    while(pSeq->isContinue){
        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (pSeq->commonParams.waitTime == 0){
            /* Execute one sequence */
            result = sony_demod_sat_device_ctrl_detect_cw_seq_Sequence (pSeq);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            /* Start stopwatch */
            result = sony_stopwatch_start (&stopwatch);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            /* AGC calculate is requested */
            if (pSeq->commonParams.agcInfo.isRequest){

                /* Clear request flag. */
                pSeq->commonParams.agcInfo.isRequest = 0;

                result = pInteg->pTunerSat->AGCLevel2AGCdB (pInteg->pTunerSat,
                                                            pSeq->commonParams.agcInfo.agcLevel,
                                                            &(pSeq->commonParams.agcInfo.agc_x100dB));
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
            }

            /* Tune is requested */
            if (pSeq->commonParams.tuneReq.isRequest){

                /* Clear request flag. */
                pSeq->commonParams.tuneReq.isRequest = 0;

                if ((pInteg->pTunerSat) && (pInteg->pTunerSat->Tune)) {
                    /* Enable the I2C repeater */
                    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN (result);
                    }
                    /* RF Tune */
                    result = pInteg->pTunerSat->Tune (pInteg->pTunerSat,
                                                      pSeq->commonParams.tuneReq.frequencyKHz,
                                                      pSeq->commonParams.tuneReq.system,
                                                      pSeq->commonParams.tuneReq.symbolRateKSps);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN (result);
                    }
                    /* Set actual frequency to the driver. */
                    pSeq->commonParams.tuneReq.frequencyKHz = pInteg->pTunerSat->frequencyKHz;
                    /* Disable the I2C repeater */
                    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN (result);
                    }
                }
            }
        } else {
            /* waiting */
            result = sony_stopwatch_sleep (&stopwatch, 10);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            result = sony_stopwatch_elapsed (&stopwatch, &elapsedTime);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            if(elapsedTime > pSeq->commonParams.waitTime){
                pSeq->commonParams.waitTime = 0;
            }
        }
    }

    *pPower = pSeq->power;

    SONY_TRACE_RETURN (result);
}
