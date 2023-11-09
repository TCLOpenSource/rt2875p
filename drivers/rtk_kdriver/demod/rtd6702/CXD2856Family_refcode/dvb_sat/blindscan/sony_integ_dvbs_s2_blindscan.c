/*------------------------------------------------------------------------------
  Copyright 2017 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/04/13
  Modification ID : 8382e363f23ef874189bb623136d4ecd6d419004
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_demod.h"
#include "sony_integ_dvbs_s2_blindscan.h"
#include "sony_demod_dvbs_s2_blindscan.h"
#include "sony_demod_dvbs_s2_blindscan_seq.h"
#include "sony_demod_dvbs_s2_tune_srs_seq.h"

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_integ_dvbs_s2_TuneSRS (sony_integ_t * pInteg,
                                          sony_demod_dvbs_s2_tune_srs_seq_t * pSeq,
                                          sony_integ_dvbs_s2_tune_srs_param_t * pParam,
                                          sony_dvbs_s2_tune_param_t * pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t elapsedTime = 0;
    sony_stopwatch_t stopwatch;
    SONY_TRACE_ENTER ("sony_integ_dvbs_s2_TuneSRS");

    if ((!pInteg) || (!pInteg->pDemod) || (!pSeq) || (!pParam) || (!pTuneParam) ||
        (!pInteg->pTunerSat) || (!pInteg->pTunerSat->AGCLevel2AGCdB) ||
        (!pInteg->pTunerSat->Tune)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) &&
        (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)){
        /* This api is accepted in Sleep and Active states only */
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

    result = sony_demod_dvbs_s2_tune_srs_seq_Initialize (pSeq,
                                                         pInteg->pDemod,
                                                         pParam->centerFreqKHz,
                                                         pParam->freqRange);
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
            result = sony_demod_dvbs_s2_tune_srs_seq_Sequence (pSeq);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            /* Start stopwatch */
            result= sony_stopwatch_start (&stopwatch);
            if (result != SONY_RESULT_OK){
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
                uint32_t symbolRateKSps = pSeq->commonParams.tuneReq.symbolRateKSps;

                /* Clear request flag. */
                pSeq->commonParams.tuneReq.isRequest = 0;

                /* Symbol rate */
                if (symbolRateKSps == 0) {
                    /* Symbol rate setting for power spectrum */
                    symbolRateKSps = pInteg->pTunerSat->symbolRateKSpsForSpectrum;
                }

                /* Enable the I2C repeater */
                result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                /* RF Tune */
                result = pInteg->pTunerSat->Tune (pInteg->pTunerSat,
                                                  pSeq->commonParams.tuneReq.frequencyKHz,
                                                  pSeq->commonParams.tuneReq.system,
                                                  symbolRateKSps);
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
            result = sony_stopwatch_sleep (&stopwatch, SONY_INTEG_DVBS_S2_TUNE_SRS_POLLING_INTERVAL);
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

    if (pSeq->isDetect){
        /* Tune successful. */
        pTuneParam->centerFreqKHz = pSeq->tuneParam.centerFreqKHz;
        pTuneParam->symbolRateKSps = pSeq->tuneParam.symbolRateKSps;
        pTuneParam->system = pSeq->tuneParam.system;

        /* TS output enable */
        result = sony_demod_dvbs_s2_blindscan_SetTSOut (pInteg->pDemod, 1);
    } else {
        result = SONY_RESULT_ERROR_UNLOCK;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_dvbs_s2_BlindScan (sony_integ_t * pInteg,
                                            sony_demod_dvbs_s2_blindscan_seq_t * pSeq,
                                            sony_integ_dvbs_s2_blindscan_param_t * pParam,
                                            sony_integ_dvbs_s2_blindscan_callback_t callback)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t elapsedTime = 0;
    sony_stopwatch_t stopwatch;
    sony_integ_dvbs_s2_blindscan_result_t blindscanResult;

    SONY_TRACE_ENTER("sony_integ_dvbs_s2_BlindScan");

    if ((!pInteg) || (!pInteg->pDemod) || (!pSeq) || (!pParam) || (!callback) ||
        (!pInteg->pTunerSat) || (!pInteg->pTunerSat->AGCLevel2AGCdB) ||
        (!pInteg->pTunerSat->Tune) || (!pInteg->pTunerSat->Sleep)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) &&
        (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)){
        /* This api is accepted in Sleep and Active states only */
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

    result = sony_demod_dvbs_s2_blindscan_seq_Initialize (pSeq,
                                                          pInteg->pDemod,
                                                          pParam->minFreqKHz,
                                                          pParam->maxFreqKHz,
                                                          pParam->minSymbolRateKSps,
                                                          pParam->maxSymbolRateKSps);
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
            result = sony_demod_dvbs_s2_blindscan_seq_Sequence (pSeq);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            /* Start stopwatch */
            result = sony_stopwatch_start (&stopwatch);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

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

            if (pSeq->commonParams.tuneReq.isRequest){
                uint32_t symbolRateKSps = pSeq->commonParams.tuneReq.symbolRateKSps;

                /* Clear request flag. */
                pSeq->commonParams.tuneReq.isRequest = 0;

                /* Symbol rate */
                if (symbolRateKSps == 0) {
                    /* Symbol rate setting for power spectrum */
                    symbolRateKSps = pInteg->pTunerSat->symbolRateKSpsForSpectrum;
                }

                /* Enable the I2C repeater */
                result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                /* RF Tune */
                result = pInteg->pTunerSat->Tune (pInteg->pTunerSat,
                                                  pSeq->commonParams.tuneReq.frequencyKHz,
                                                  pSeq->commonParams.tuneReq.system,
                                                  symbolRateKSps);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                pSeq->commonParams.tuneReq.frequencyKHz = pInteg->pTunerSat->frequencyKHz;
                /* Disable the I2C repeater */
                result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
            }

            if (pSeq->commonParams.detInfo.isDetect){

                /* Clear detect flag */
                pSeq->commonParams.detInfo.isDetect = 0;

                /* Prepare callback information.(Detected channel) */
                blindscanResult.eventId = SONY_INTEG_DVBS_S2_BLINDSCAN_EVENT_DETECT;
                blindscanResult.tuneParam.system = pSeq->commonParams.detInfo.system;
                blindscanResult.tuneParam.centerFreqKHz = pSeq->commonParams.detInfo.centerFreqKHz;
                blindscanResult.tuneParam.symbolRateKSps = pSeq->commonParams.detInfo.symbolRateKSps;

                /* TS output enable */
                result = sony_demod_dvbs_s2_blindscan_SetTSOut (pInteg->pDemod, 1);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }

                /* Callback */
                callback(pInteg, &blindscanResult);

                /* TS output disable */
                result = sony_demod_dvbs_s2_blindscan_SetTSOut (pInteg->pDemod, 0);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
            }

            {
                /* Progress calculation */
                uint8_t progress = 0;
                uint8_t rangeMin = pSeq->commonParams.progressInfo.majorMinProgress;
                uint8_t rangeMax = pSeq->commonParams.progressInfo.majorMaxProgress;
                uint8_t minorProgress = pSeq->commonParams.progressInfo.minorProgress;
                progress = rangeMin + (((rangeMax - rangeMin) * minorProgress) / 100);

                if (pSeq->commonParams.progressInfo.progress < progress){
                    pSeq->commonParams.progressInfo.progress = progress;
                    /* Prepare callback information.(Progress) */
                    blindscanResult.eventId = SONY_INTEG_DVBS_S2_BLINDSCAN_EVENT_PROGRESS;
                    blindscanResult.progress = progress;
                    /* Callback */
                    callback(pInteg, &blindscanResult);
                }
            }

        } else {
            /* waiting */
            result = sony_stopwatch_sleep (&stopwatch, SONY_INTEG_DVBS_S2_BLINDSCAN_POLLING_INTERVAL);
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

    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = pInteg->pTunerSat->Sleep(pInteg->pTunerSat);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_dvbs_s2_Sleep (pSeq->commonParams.pDemod);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN(result);
}
