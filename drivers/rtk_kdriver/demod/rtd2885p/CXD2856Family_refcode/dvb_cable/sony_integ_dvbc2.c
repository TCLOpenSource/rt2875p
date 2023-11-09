/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/04/13
  Modification ID : 8382e363f23ef874189bb623136d4ecd6d419004
------------------------------------------------------------------------------*/
#include "sony_integ.h"
#include "sony_integ_dvbc2.h"
#include "sony_demod.h"
#include "sony_demod_dvbc2_monitor.h"

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_integ_dvbc2_Tune(sony_integ_t * pInteg,
                                    sony_dvbc2_tune_param_t * pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbc2_tune_seq_t tuneSeq;
    sony_stopwatch_t timer;
    uint32_t sleepStartTime = 0;
    uint32_t elapsedTime = 0;

    SONY_TRACE_ENTER ("sony_integ_dvbc2_Tune");

    if ((!pInteg) || (!pTuneParam) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (!pInteg->pDemod->scanMode) {
        /* Clear cancellation flag. */
        sony_atomic_set (&(pInteg->cancel), 0);
    }

    /* Check bandwidth validity */
    if ((pTuneParam->bandwidth != SONY_DTV_BW_6_MHZ) && (pTuneParam->bandwidth != SONY_DTV_BW_8_MHZ)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    /* Confirm Tune parameters */
    if (pTuneParam->c2TuningFrequencyKHz == 0){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

#ifdef SONY_DEMOD_SUPPORT_SAT
    /* Sleep satellite tuner if active. */
    result = sony_integ_sat_SetTunerSleep (pInteg);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
#endif

    /* Start stopwatch to measure total waiting time */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_dvbc2_InitTuneSeq (pInteg->pDemod, pTuneParam, &tuneSeq);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    while (tuneSeq.running) {
        if (tuneSeq.sleepTime == 0) {
            /* Set current total time before calling demod layer function. */
            /* This is timed from scan start */
            result = sony_stopwatch_elapsed(&timer, &tuneSeq.currentTime);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            result = sony_demod_dvbc2_TuneSeq (pInteg->pDemod, &tuneSeq);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            /* Check if RF tuning is invoked in demod layer. */
            if (tuneSeq.tuneRequired) {
                /* Enable I2C repeater for tuner comms */
                if ((pInteg->pTunerTerrCable) && (pInteg->pTunerTerrCable->Tune)) {
                    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }

                    result = pInteg->pTunerTerrCable->Tune (pInteg->pTunerTerrCable, tuneSeq.rfFrequency, SONY_DTV_SYSTEM_DVBC2, pTuneParam->bandwidth);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }

                    tuneSeq.tuneRequired = 0; /* FALSE */

                    /* Disable I2C repeater for tuner comms */
                    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }
                }
            }

            if (tuneSeq.sleepTime > 0) {
                /* Sleep requested, store start time now */
                result = sony_stopwatch_elapsed(&timer, &sleepStartTime);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }
            }
        }

        if (tuneSeq.sleepTime > 0) {
            /*    This polling interval is not always same as interval in scan sequence.
                This interval affects interval for checking cancellation flag. */
            result = sony_stopwatch_sleep(&timer, SONY_DVBC2_CHECK_CANCEL_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            result = sony_stopwatch_elapsed(&timer, &elapsedTime);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            if (elapsedTime - sleepStartTime >= tuneSeq.sleepTime) {
                /* Finish sleep */
                tuneSeq.sleepTime = 0;
            }
        }

        /* Sanity check on overall wait time. */
        result = sony_stopwatch_elapsed(&timer, &elapsedTime);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (elapsedTime > SONY_DVBC2_MAX_TUNE_DURATION) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
        }

        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    result = tuneSeq.lockResult;

    /* Deferred exit if tune failed */
    if (tuneSeq.lockResult != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (tuneSeq.lockResult);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_dvbc2_GetChannelTuneParameters (sony_integ_t * pInteg,
                                                         uint32_t frequencyKHz,
                                                         sony_dtv_bandwidth_t bandwidth,
                                                         sony_dvbc2_tune_param_t * pTuneParamArray,
                                                         uint16_t * pNumTuneParams,
                                                         uint32_t * pNextChannelFrequencyKHz)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbc2_get_tune_params_seq_t getTuneParamsSeq;
    sony_dvbc2_tune_param_t tuneParam;
    sony_stopwatch_t timer;
    uint32_t sleepStartTime = 0;
    uint32_t elapsedTime = 0;

    SONY_TRACE_ENTER ("sony_integ_dvbc2_GetChannelTuneParameters");

    if ((!pInteg) || (!pInteg->pDemod) || (!pTuneParamArray) || (!pNumTuneParams) || (!pNextChannelFrequencyKHz)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Clear cancellation flag. */
    sony_atomic_set (&(pInteg->cancel), 0);

    /* Check bandwidth validity */
    if ((bandwidth != SONY_DTV_BW_6_MHZ) && (bandwidth != SONY_DTV_BW_8_MHZ)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

#ifdef SONY_DEMOD_SUPPORT_SAT
    /* Sleep satellite tuner if active. */
    result = sony_integ_sat_SetTunerSleep (pInteg);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
#endif

    /* Start stopwatch to measure total waiting time */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Initialize the Get Tune Params structure */
    result = sony_demod_dvbc2_InitGetTuneParamsSeq (pInteg->pDemod, &getTuneParamsSeq, frequencyKHz, pTuneParamArray);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Set default values for return parameters */
    *pNumTuneParams = 0;
    /* Next channel frequency based on C2 system bandwidth */
    if (bandwidth == SONY_DTV_BW_8_MHZ) {
        *pNextChannelFrequencyKHz = frequencyKHz + 2000;
    }
    else if (bandwidth == SONY_DTV_BW_6_MHZ) {
        *pNextChannelFrequencyKHz = frequencyKHz + 1500;
    }
    else {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }

    /* Set the tune center frequency to the current step for demod initialisation */
    tuneParam.c2TuningFrequencyKHz = frequencyKHz;
    tuneParam.bandwidth = bandwidth;
    tuneParam.dataPLPId = 0;
    tuneParam.dataSliceId = 0;
    tuneParam.isDependentStaticDS = 0;
    tuneParam.rfTuningFrequencyKHz = 0;

    /* Enable acquisition on the demodulator. */
    result = sony_demod_dvbc2_Tune (pInteg->pDemod, &(tuneParam));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if ((pInteg->pTunerTerrCable) && (pInteg->pTunerTerrCable->Tune)) {
        /* Enable I2C repeater for tuner comms */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Perform RF tuning. */
        result = pInteg->pTunerTerrCable->Tune (pInteg->pTunerTerrCable, frequencyKHz, SONY_DTV_SYSTEM_DVBC2, bandwidth);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Disable I2C repeater for tuner comms */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    /* Perform Soft reset to start acquisition */
    result = sony_demod_TuneEnd (pInteg->pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    while (getTuneParamsSeq.running) {
        if (getTuneParamsSeq.sleepTime == 0) {
            /* Set current total time before calling demod layer function. */
            /* This is timed from scan start */
            result = sony_stopwatch_elapsed(&timer, &getTuneParamsSeq.currentTime);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            result = sony_demod_dvbc2_GetTuneParamsSeq (pInteg->pDemod, &getTuneParamsSeq);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            if (getTuneParamsSeq.sleepTime > 0) {
                /* Sleep requested, store start time now */
                result = sony_stopwatch_elapsed(&timer, &sleepStartTime);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }
            }
        }

        if (getTuneParamsSeq.sleepTime > 0) {
            /* This polling interval is not always same as interval in scan sequence.
               This interval affects interval for checking cancellation flag. */
            result = sony_stopwatch_sleep(&timer, SONY_DVBC2_CHECK_CANCEL_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            result = sony_stopwatch_elapsed(&timer, &elapsedTime);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            if (elapsedTime - sleepStartTime >= getTuneParamsSeq.sleepTime) {
                /* Finish sleep */
                getTuneParamsSeq.sleepTime = 0;
            }
        }

        /* Sanity check on overall wait time. */
        result = sony_stopwatch_elapsed(&timer, &elapsedTime);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (elapsedTime > SONY_DVBC2_MAX_TUNE_DURATION) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
        }

        /* Handle internal loop errors (unlock / timeout etc) */
        if (getTuneParamsSeq.loopResult != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (getTuneParamsSeq.loopResult);
        }

        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set tune parameter count */
    *pNumTuneParams = getTuneParamsSeq.tuneParamIndex;

    if (*pNumTuneParams > 0) {
        /* Valid channel tune parameters found, return calculated next channel frequency */
        *pNextChannelFrequencyKHz = getTuneParamsSeq.nextChannelFrequency;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_dvbc2_Scan (sony_integ_t * pInteg,
                                     sony_integ_dvbc2_scan_param_t * pScanParam,
                                     sony_integ_dvbc2_scan_callback_t callBack)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_integ_dvbc2_scan_result_t scanResult;
    sony_dvbc2_tune_param_t tuneParams[256];
    uint16_t numTuneParams;
    uint32_t nextChannelFrequencyKHz;
    uint32_t scanFreqKHz = 0;

    SONY_TRACE_ENTER ("sony_integ_dvbc2_Scan");

    if ((!pInteg) || (!pScanParam) || (!callBack) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Clear cancellation flag. */
    sony_atomic_set (&(pInteg->cancel), 0);

    /* Ensure the scan parameters are valid. */
    if (pScanParam->endFrequencyKHz < pScanParam->startFrequencyKHz) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Check bandwidth validity */
    if ((pScanParam->bandwidth != SONY_DTV_BW_6_MHZ) && (pScanParam->bandwidth != SONY_DTV_BW_8_MHZ)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    /* Set scan mode enabled */
    result = sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Set starting frequecy for scan */
    scanFreqKHz = pScanParam->startFrequencyKHz;

    while (scanFreqKHz <= pScanParam->endFrequencyKHz) {
        result = sony_integ_dvbc2_GetChannelTuneParameters (pInteg, scanFreqKHz, pScanParam->bandwidth, &tuneParams[0],
                                                            &numTuneParams, &nextChannelFrequencyKHz);

        if (result == SONY_RESULT_OK) {
            uint8_t tuneParamIndex = 0;
            uint8_t channelCount = 0;
            sony_result_t tuneResult;
            uint8_t currentDs = 0;
            uint8_t currentDsTuneSuccess = 0;

            for (tuneParamIndex = 0 ; tuneParamIndex < numTuneParams ; tuneParamIndex++)
            {
                if ((currentDsTuneSuccess == 0) ||(tuneParams[tuneParamIndex].dataSliceId != currentDs)) {
                    /* If this is the first acquisition for the tune parameter set, the data
                    * slice has changed or the previous tune to this data slice failed, perform
                    * a full tune. */
                    tuneResult = sony_integ_dvbc2_Tune (pInteg, &tuneParams[tuneParamIndex]);
                    if (tuneResult == SONY_RESULT_OK) {
                        currentDsTuneSuccess = 1;
                    } else {
                        currentDsTuneSuccess = 0;
                    }
                }
                else {
                    /* Select a different PLP in the same data slice */
                    tuneResult = sony_demod_dvbc2_SetPlpId (pInteg->pDemod, tuneParams[tuneParamIndex].dataPLPId);
                }

                if (tuneResult == SONY_RESULT_OK) {
                    /* Channel parameters are confirmed, provide channel callback */
                    if (callBack) {
                        scanResult.centerFreqKHz = tuneParams[tuneParamIndex].c2TuningFrequencyKHz;
                        scanResult.tuneParam = tuneParams[tuneParamIndex];
                        scanResult.tuneResult = SONY_RESULT_OK;
                        callBack (pInteg, &scanResult, pScanParam);
                    }

                    channelCount++;
                    currentDs = tuneParams[tuneParamIndex].dataSliceId;
                }

                /* Check cancellation. */
                tuneResult = sony_integ_CheckCancellation (pInteg);
                if (tuneResult != SONY_RESULT_OK) {
                    sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                    SONY_TRACE_RETURN (tuneResult);
                }
            }

            if (channelCount == 0) {
                /* Channel parameters were not successfully acquired to, provide progress callback */
                if (callBack) {
                    scanResult.centerFreqKHz = scanFreqKHz;
                    scanResult.tuneResult = SONY_RESULT_ERROR_UNLOCK;
                    callBack (pInteg, &scanResult, pScanParam);
                }
            }
        }
        else if ((result == SONY_RESULT_ERROR_UNLOCK) || (result == SONY_RESULT_ERROR_TIMEOUT)) {
            /* Provide progress callback */
            if (callBack) {
                scanResult.centerFreqKHz = scanFreqKHz;
                scanResult.tuneResult = result;
                callBack (pInteg, &scanResult, pScanParam);
            }
        }
        else {
            /* Serious error */
            sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
            SONY_TRACE_RETURN (result);
        }

        scanFreqKHz = nextChannelFrequencyKHz;
    }

    /* Clear scan mode */
    result = sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_dvbc2_WaitTSLock (sony_integ_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;

    SONY_TRACE_ENTER ("sony_integ_dvbc2_WaitTSLock");

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Wait for TS lock */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    for (;;) {
        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (elapsed >= SONY_DVBC2_WAIT_TS_LOCK) {
            continueWait = 0;
        }

        result = sony_demod_dvbc2_CheckTSLock (pInteg->pDemod, &lock);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        switch (lock) {
        case SONY_DEMOD_LOCK_RESULT_LOCKED:
            SONY_TRACE_RETURN (SONY_RESULT_OK);

        case SONY_DEMOD_LOCK_RESULT_UNLOCKED:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);

        default:
            /* continue waiting... */
            break;
        }

        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (continueWait) {
            result = sony_stopwatch_sleep (&timer, SONY_DVBC2_WAIT_LOCK_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }
        else {
            result = SONY_RESULT_ERROR_TIMEOUT;
            break;
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_dvbc2_monitor_RFLevel (sony_integ_t * pInteg, int32_t * pRFLeveldB)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_dvbc2_monitor_RFLevel");

    if ((!pInteg) || (!pInteg->pDemod) || (!pRFLeveldB)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pInteg->pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (!pInteg->pTunerTerrCable) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    if (pInteg->pTunerTerrCable->ReadRFLevel) {
        /* Enable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = pInteg->pTunerTerrCable->ReadRFLevel (pInteg->pTunerTerrCable, pRFLeveldB);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Disable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    } else if (pInteg->pTunerTerrCable->CalcRFLevelFromAGC) {
        uint32_t ifAgc;

        result = sony_demod_dvbc2_monitor_IFAGCOut(pInteg->pDemod, &ifAgc);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = pInteg->pTunerTerrCable->CalcRFLevelFromAGC (pInteg->pTunerTerrCable, ifAgc, pRFLeveldB);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    SONY_TRACE_RETURN (result);
}

