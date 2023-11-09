/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/04/13
  Modification ID : 8382e363f23ef874189bb623136d4ecd6d419004
------------------------------------------------------------------------------*/

#include "sony_integ.h"
#include "sony_integ_dvbt2.h"
#include "sony_demod.h"
#include "sony_demod_dvbt2_monitor.h"

/*------------------------------------------------------------------------------
Local Function Prototypes
------------------------------------------------------------------------------*/
/**
 @brief Waits for demodulator lock, polling ::sony_demod_dvbt2_monitor_SyncStat
        at 10ms intervals.  Called as part of the Tune process.
*/
static sony_result_t dvbt2_WaitDemodLock (sony_integ_t * pInteg, sony_dvbt2_profile_t profile);

/**
 @brief Waits for L1 Post to be valid to ensure that subsequent calls to
        L1 based monitors do not return HW State error.  Polls
        ::sony_demod_dvbt2_CheckL1PostValid at 10ms intervals.  Called as
        part of the Tune, BlindTune and Scan processes.
*/
static sony_result_t dvbt2_WaitL1PostLock (sony_integ_t * pInteg);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_integ_dvbt2_Tune(sony_integ_t * pInteg,
                                    sony_dvbt2_tune_param_t * pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_dvbt2_Tune");

    if ((!pInteg) || (!pTuneParam) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Clear cancellation flag. */
    sony_atomic_set (&(pInteg->cancel), 0);

    /* Check bandwidth validity */
    if ((pTuneParam->bandwidth != SONY_DTV_BW_1_7_MHZ) && (pTuneParam->bandwidth != SONY_DTV_BW_5_MHZ) &&
        (pTuneParam->bandwidth != SONY_DTV_BW_6_MHZ) && (pTuneParam->bandwidth != SONY_DTV_BW_7_MHZ) &&
        (pTuneParam->bandwidth != SONY_DTV_BW_8_MHZ)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    /* Check for valid profile selection */
    if ((pTuneParam->profile != SONY_DVBT2_PROFILE_BASE) && (pTuneParam->profile != SONY_DVBT2_PROFILE_LITE)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

#ifdef SONY_DEMOD_SUPPORT_SAT
    /* Sleep satellite tuner if active. */
    result = sony_integ_sat_SetTunerSleep (pInteg);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
#endif

    /* Configure for manual PLP selection. */
    result = sony_demod_dvbt2_SetPLPConfig (pInteg->pDemod, 0x00, pTuneParam->dataPlpId);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Configure the DVB-T2 profile without recovery */
    result = sony_demod_dvbt2_SetProfile(pInteg->pDemod, pTuneParam->profile);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Tune the demodulator */
    result = sony_demod_dvbt2_Tune (pInteg->pDemod, pTuneParam);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if ((pInteg->pTunerTerrCable) && (pInteg->pTunerTerrCable->Tune)) {
        /* Enable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Tune the RF part */
        result = pInteg->pTunerTerrCable->Tune (pInteg->pTunerTerrCable, pTuneParam->centerFreqKHz, SONY_DTV_SYSTEM_DVBT2, pTuneParam->bandwidth);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Disable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    /* Reset the demod to enable acquisition */
    result = sony_demod_TuneEnd (pInteg->pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Wait for demodulator lock */
    result = dvbt2_WaitDemodLock (pInteg, pTuneParam->profile);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Wait for TS lock */
    result = sony_integ_dvbt2_WaitTSLock (pInteg, pTuneParam->profile);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* In DVB-T2, L1 Post information may not immediately be valid after acquisition
     * (L1POST_OK bit != 1).  This wait loop handles such cases.  This issue occurs
     * only under clean signal lab conditions, and will therefore not extend acquistion
     * time under normal conditions.
     */
    result = dvbt2_WaitL1PostLock (pInteg);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Confirm correct PLP selection in acquisition */
    {
        uint8_t plpNotFound;

        result = sony_demod_dvbt2_monitor_DataPLPError (pInteg->pDemod, &plpNotFound);
        if (result == SONY_RESULT_ERROR_HW_STATE) {
            /* Demod lock is lost causing monitor to fail, return UNLOCK instead of HW STATE */
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);
        }
        else if (result != SONY_RESULT_OK) {
            /* Serious error, so return result */
            SONY_TRACE_RETURN (result);
        }

        if (plpNotFound) {
            result = SONY_RESULT_OK_CONFIRM;
            pTuneParam->tuneInfo = SONY_DEMOD_DVBT2_TUNE_INFO_INVALID_PLP_ID;
        }
        else {
            pTuneParam->tuneInfo = SONY_DEMOD_DVBT2_TUNE_INFO_OK;
        }
    }

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_integ_dvbt2_Scan(sony_integ_t * pInteg,
                                    sony_integ_dvbt2_scan_param_t * pScanParam,
                                    sony_integ_dvbt2_scan_callback_t callBack)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_integ_dvbt2_scan_result_t scanResult;

    SONY_TRACE_ENTER ("sony_integ_dvbt2_Scan");

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

    if (pScanParam->stepFrequencyKHz == 0) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Check bandwidth validity */
    if ((pScanParam->bandwidth != SONY_DTV_BW_1_7_MHZ) && (pScanParam->bandwidth != SONY_DTV_BW_5_MHZ) &&
        (pScanParam->bandwidth != SONY_DTV_BW_6_MHZ) && (pScanParam->bandwidth != SONY_DTV_BW_7_MHZ) &&
        (pScanParam->bandwidth != SONY_DTV_BW_8_MHZ)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    /* Set the start frequency */
    scanResult.centerFreqKHz = pScanParam->startFrequencyKHz;

    /* Set scan mode enabled */
    result = sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBT2, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Scan routine */
    while (scanResult.centerFreqKHz <= pScanParam->endFrequencyKHz) {
        sony_dvbt2_profile_t profileFound = SONY_DVBT2_PROFILE_ANY;
        uint8_t channelComplete = 0;
        sony_dvbt2_profile_t blindTuneProfile = pScanParam->t2Profile;

        while (!channelComplete) {
            scanResult.tuneResult = sony_integ_dvbt2_BlindTune(pInteg, scanResult.centerFreqKHz, pScanParam->bandwidth, blindTuneProfile, &profileFound);
            switch (scanResult.tuneResult) {
            case SONY_RESULT_OK:
                {
                    uint8_t numPLPs = 0;
                    uint8_t plpIds[255];
                    uint8_t mixed;
                    uint8_t i;

                    scanResult.dvbt2TuneParam.centerFreqKHz = scanResult.centerFreqKHz;
                    scanResult.dvbt2TuneParam.bandwidth = pScanParam->bandwidth;
                    scanResult.dvbt2TuneParam.tuneInfo = SONY_DEMOD_DVBT2_TUNE_INFO_OK;
                    scanResult.dvbt2TuneParam.profile = profileFound;

                    result = sony_integ_dvbt2_Scan_PrepareDataPLPLoop(pInteg, plpIds, &numPLPs, &mixed);
                    if (result == SONY_RESULT_ERROR_HW_STATE) {
                        /* Callback to application for progress updates */
                        scanResult.tuneResult = SONY_RESULT_ERROR_UNLOCK;
                        callBack (pInteg, &scanResult, pScanParam);

                        /* Error in monitored data, ignore current channel */
                        channelComplete = 1;
                    }
                    else if (result != SONY_RESULT_OK) {
                        /* Serious error occurred -> cancel operation. */
                        sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBT2, 0x00);
                        SONY_TRACE_RETURN (result);
                    }
                    else {
                        /* Set PLP ID in tune parameter structure and provide callback for first PLP */
                        scanResult.dvbt2TuneParam.dataPlpId = plpIds[0];
                        callBack (pInteg, &scanResult, pScanParam);

                        /* Callback for each subsequent PLP in current profile */
                        for (i = 1; i < numPLPs; i++) {
                            result = sony_integ_dvbt2_Scan_SwitchDataPLP(pInteg, mixed, plpIds[i], profileFound);
                            if (result != SONY_RESULT_OK) {
                                sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBT2, 0x00);
                                SONY_TRACE_RETURN (result);
                            }
                            /* Set PLP ID in tune parameter structure and provide callback */
                            scanResult.dvbt2TuneParam.dataPlpId = plpIds[i];
                            callBack (pInteg, &scanResult, pScanParam);
                        }

                        /* If profile is ANY, check for mixed profile channels */
                        if (blindTuneProfile == SONY_DVBT2_PROFILE_ANY) {
                            /* Check for mixed profiles available */
                            if (mixed) {
                                /* Set Blind Tune parameters to DVB-T2 only and the other profile
                                 * compared to that already located */
                                if (profileFound == SONY_DVBT2_PROFILE_BASE) {
                                    blindTuneProfile = SONY_DVBT2_PROFILE_LITE;
                                }
                                else if (profileFound == SONY_DVBT2_PROFILE_LITE) {
                                    blindTuneProfile = SONY_DVBT2_PROFILE_BASE;
                                }
                                else {
                                    /* Serious error occurred -> cancel operation. */
                                    sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBT2, 0x00);
                                    SONY_TRACE_RETURN (result);
                                }
                            }
                            else {
                                /* Channel is not mixed profile so continue to next frequency */
                                channelComplete = 1;
                            }
                        }
                        else {
                            /* Profile is fixed therefore this frequency is complete */
                            channelComplete = 1;
                        }
                    }
                }
                break;

            /* Intentional fall-through. */
            case SONY_RESULT_ERROR_UNLOCK:
            case SONY_RESULT_ERROR_TIMEOUT:
                /* Channel not found, callback to application for progress updates */
                callBack (pInteg, &scanResult, pScanParam);

                /* Go to next frequency */
                channelComplete = 1;
                break;

            default:
                {
                    /* Serious error occurred -> cancel operation. */
                    sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBT2, 0x00);
                    SONY_TRACE_RETURN (scanResult.tuneResult);
                }
            }
        }

        scanResult.centerFreqKHz += pScanParam->stepFrequencyKHz;

        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBT2, 0x00);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Clear scan mode */
    result = sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBT2, 0x00);

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_integ_dvbt2_Scan_PrepareDataPLPLoop(sony_integ_t * pInteg, uint8_t pPLPIds[], uint8_t *pNumPLPs, uint8_t *pMixed)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_dvbt2_Scan_PrepareDataPLPLoop");

    if ((!pInteg) || (!pPLPIds) || (!pNumPLPs) || (!pMixed)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Obtain the T2 PLP list from Data PLP monitor */
    result = sony_demod_dvbt2_monitor_DataPLPs (pInteg->pDemod, pPLPIds, pNumPLPs);
    if (result == SONY_RESULT_OK) {
        /* Check for mixed profile channels */
        sony_dvbt2_ofdm_t ofdm;

        result = sony_demod_dvbt2_monitor_OFDM (pInteg->pDemod, &ofdm);
        if (result == SONY_RESULT_OK) {
            *pMixed = ofdm.mixed;
        }
    }

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_integ_dvbt2_Scan_SwitchDataPLP(sony_integ_t * pInteg, uint8_t mixed, uint8_t plpId, sony_dvbt2_profile_t profile)
{
    sony_result_t result = SONY_RESULT_OK;
    uint16_t plpAcquisitionTime = 0;
    sony_stopwatch_t timer;

    SONY_TRACE_ENTER ("sony_integ_dvbt2_Scan_SwitchDataPLP");

    if (!pInteg) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_dvbt2_SetPLPConfig (pInteg->pDemod, 0x00, plpId);
    if (result != SONY_RESULT_OK) {
        /* Serious error occurred -> cancel operation. */
        SONY_TRACE_RETURN (result);
    }

    if ((profile == SONY_DVBT2_PROFILE_BASE) && (mixed)) {
        plpAcquisitionTime = 510;
    }
    else if ((profile == SONY_DVBT2_PROFILE_LITE) && (mixed)) {
        plpAcquisitionTime = 1260;
    }
    else {
        plpAcquisitionTime = 260;
    }

    /* Start stopwatch to measure PLP acquisition time */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    for (;;) {
        uint32_t elapsed;

        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (elapsed >= plpAcquisitionTime) {
            break; /* finish waiting */
        } else {
            result = sony_stopwatch_sleep (&timer, SONY_DVBT2_WAIT_LOCK_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }
    }

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_integ_dvbt2_BlindTune(sony_integ_t * pInteg,
                                         uint32_t centerFreqKHz,
                                         sony_dtv_bandwidth_t bandwidth,
                                         sony_dvbt2_profile_t profile,
                                         sony_dvbt2_profile_t * pProfileTuned)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dvbt2_tune_param_t tuneParam;

    SONY_TRACE_ENTER ("sony_integ_dvbt2_BlindTune");

    if ((!pInteg) || (!pInteg->pDemod) || (!pProfileTuned)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (!pInteg->pDemod->scanMode) {
        /* Clear cancellation flag. */
        sony_atomic_set (&(pInteg->cancel), 0);
    }

    /* Confirm the demod is in a valid state to accept this API */
    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Check bandwidth validity */
    if ((bandwidth != SONY_DTV_BW_1_7_MHZ) && (bandwidth != SONY_DTV_BW_5_MHZ) &&
        (bandwidth != SONY_DTV_BW_6_MHZ) && (bandwidth != SONY_DTV_BW_7_MHZ) &&
        (bandwidth != SONY_DTV_BW_8_MHZ)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

#ifdef SONY_DEMOD_SUPPORT_SAT
    /* Sleep satellite tuner if active. */
    result = sony_integ_sat_SetTunerSleep (pInteg);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
#endif

    /* Attempt DVB-T2 acquisition */
    tuneParam.bandwidth = bandwidth;
    tuneParam.centerFreqKHz = centerFreqKHz;
    tuneParam.dataPlpId = 0; /* Not used in blind acquisition */

    /* Configure for automatic PLP selection. */
    result = sony_demod_dvbt2_SetPLPConfig (pInteg->pDemod, 0x01, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Configure the DVB-T2 profile without recovery */
    result = sony_demod_dvbt2_SetProfile(pInteg->pDemod, profile);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Tune the demodulator */
    result = sony_demod_dvbt2_Tune (pInteg->pDemod, &tuneParam);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if ((pInteg->pTunerTerrCable) && (pInteg->pTunerTerrCable->Tune)) {
        /* Enable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Tune the RF part */
        result = pInteg->pTunerTerrCable->Tune (pInteg->pTunerTerrCable, tuneParam.centerFreqKHz, SONY_DTV_SYSTEM_DVBT2, tuneParam.bandwidth);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Disable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    /* Reset the demod to enable acquisition */
    result = sony_demod_TuneEnd (pInteg->pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Wait for demodulator lock */
    result = dvbt2_WaitDemodLock (pInteg, profile);
    switch (result) {
    case SONY_RESULT_OK:
        /* In DVB-T2, L1 Post information may not immediately be valid after acquisition
            * (L1POST_OK bit != 1).  This wait loop handles such cases.  This issue occurs
            * only under clean signal lab conditions, and will therefore not extend acquistion
            * time under normal conditions.
            */
        result = dvbt2_WaitL1PostLock (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (profile == SONY_DVBT2_PROFILE_ANY) {
            /* Obtain the current profile if detection was automatic. */
            result = sony_demod_dvbt2_monitor_Profile (pInteg->pDemod, pProfileTuned);
            if (result == SONY_RESULT_ERROR_HW_STATE) {
                /* Demod lock is lost causing monitor to fail, return UNLOCK instead of HW STATE */
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);
            }
            else if (result != SONY_RESULT_OK) {
                /* Serious error, so return result */
                SONY_TRACE_RETURN (result);
            }
        }
        else {
            /* Else, set the tuned profile to the input parameter */
            *pProfileTuned = profile;
        }

        break;

    /* Intentional fall-through */
    case SONY_RESULT_ERROR_TIMEOUT:
    case SONY_RESULT_ERROR_UNLOCK:
        break;

    default:
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_integ_dvbt2_WaitTSLock (sony_integ_t * pInteg, sony_dvbt2_profile_t profile)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    uint16_t timeout = 0;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;

    SONY_TRACE_ENTER ("sony_integ_dvbt2_WaitTSLock");

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

    if (profile == SONY_DVBT2_PROFILE_BASE) {
        timeout = SONY_DVBT2_BASE_WAIT_TS_LOCK;
    }
    else if (profile == SONY_DVBT2_PROFILE_LITE) {
        timeout = SONY_DVBT2_LITE_WAIT_TS_LOCK;
    }
    else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for (;;) {
        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (elapsed >= timeout) {
            continueWait = 0;
        }

        result = sony_demod_dvbt2_CheckTSLock (pInteg->pDemod, &lock);
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
            result = sony_stopwatch_sleep (&timer, SONY_DVBT2_WAIT_LOCK_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } else {
            result = SONY_RESULT_ERROR_TIMEOUT;
            break;
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_dvbt2_monitor_RFLevel (sony_integ_t * pInteg, int32_t * pRFLeveldB)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_dvbt2_monitor_RFLevel");

    if ((!pInteg) || (!pInteg->pDemod) || (!pRFLeveldB)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pInteg->pDemod->system != SONY_DTV_SYSTEM_DVBT2) {
        /* Not DVB-T2*/
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

        result = sony_demod_dvbt2_monitor_IFAGCOut(pInteg->pDemod, &ifAgc);
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

sony_result_t sony_integ_dvbt2_monitor_SSI (sony_integ_t * pInteg, uint8_t * pSSI)
{
    int32_t rfLevel;
    sony_dvbt2_plp_constell_t qam;
    sony_dvbt2_plp_code_rate_t codeRate;
    int32_t prel;
    int32_t tempSSI = 0;
    sony_result_t result = SONY_RESULT_OK;

    static const int32_t pRefdBm1000[4][8] = {
    /*    1/2,    3/5,    2/3,    3/4,    4/5,    5/6,    1/3,    2/5                */
        {-96000, -95000, -94000, -93000, -92000, -92000, -98000, -97000}, /* QPSK    */
        {-91000, -89000, -88000, -87000, -86000, -86000, -93000, -92000}, /* 16-QAM  */
        {-86000, -85000, -83000, -82000, -81000, -80000, -89000, -88000}, /* 64-QAM  */
        {-82000, -80000, -78000, -76000, -75000, -74000, -86000, -84000}, /* 256-QAM */
    };

    SONY_TRACE_ENTER ("sony_integ_dvbt2_monitor_SSI");

    if ((!pInteg) || (!pInteg->pDemod) || (!pSSI)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pInteg->pDemod->system != SONY_DTV_SYSTEM_DVBT2) {
        /* Not DVB-T2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Get estimated RF Level */
    result = sony_integ_dvbt2_monitor_RFLevel (pInteg, &rfLevel);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Get PLP constellation */
    result = sony_demod_dvbt2_monitor_QAM (pInteg->pDemod, SONY_DVBT2_PLP_DATA, &qam);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Get PLP code rate */
    result = sony_demod_dvbt2_monitor_CodeRate (pInteg->pDemod, SONY_DVBT2_PLP_DATA, &codeRate);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Ensure correct plp info. */
    if ((codeRate > SONY_DVBT2_R2_5) || (qam > SONY_DVBT2_QAM256)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    /* prel = prec - pref */
    prel = rfLevel - pRefdBm1000[qam][codeRate];

    /* SSI (Signal Strength Indicator) is calculated from:
     *
     * if (prel < -15dB)              SSI = 0
     * if (-15dB <= prel < 0dB)       SSI = (2/3) * (prel + 15)
     * if (0dB <= prel < 20dB)        SSI = 4 * prel + 10
     * if (20dB <= prel < 35dB)       SSI = (2/3) * (prel - 20) + 90
     * if (prel >= 35dB)              SSI = 100
     */
    if (prel < -15000) {
        tempSSI = 0;
    }
    else if (prel < 0) {
        /* Note : prel and 2/3 scaled by 10^3 so divide by 10^6 added */
        tempSSI = ((2 * (prel + 15000)) + 1500) / 3000;
    }
    else if (prel < 20000) {
        /* Note : prel scaled by 10^3 so divide by 10^3 added */
        tempSSI = (((4 * prel) + 500) / 1000) + 10;
    }
    else if (prel < 35000) {
        /* Note : prel and 2/3 scaled by 10^3 so divide by 10^6 added */
        tempSSI = (((2 * (prel - 20000)) + 1500) / 3000) + 90;
    }
    else {
        tempSSI = 100;
    }

    /* Clip value to 100% */
    *pSSI = (tempSSI > 100)? 100 : (uint8_t)tempSSI;

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
Local Functions
------------------------------------------------------------------------------*/
static sony_result_t dvbt2_WaitDemodLock (sony_integ_t * pInteg, sony_dvbt2_profile_t profile)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    uint16_t timeout = 0;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;

    SONY_TRACE_ENTER ("dvbt2_WaitDemodLock");

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Wait for demod lock */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if (profile == SONY_DVBT2_PROFILE_BASE) {
        timeout = SONY_DVBT2_BASE_WAIT_DEMOD_LOCK;
    }
    else if ((profile == SONY_DVBT2_PROFILE_LITE) || (profile == SONY_DVBT2_PROFILE_ANY)) {
        timeout = SONY_DVBT2_LITE_WAIT_DEMOD_LOCK;
    }
    else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    for (;;) {
        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (elapsed >= timeout) {
            continueWait = 0;
        }

        result = sony_demod_dvbt2_CheckDemodLock (pInteg->pDemod, &lock);
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
            result = sony_stopwatch_sleep (&timer, SONY_DVBT2_WAIT_LOCK_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } else {
            result = SONY_RESULT_ERROR_TIMEOUT;
            break;
        }
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t dvbt2_WaitL1PostLock (sony_integ_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;
    uint8_t l1PostValid;

    SONY_TRACE_ENTER ("dvbt2_WaitL1PostLock");

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    for (;;) {
        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Check for timeout condition */
        if (elapsed >= SONY_DVBT2_L1POST_TIMEOUT) {
            continueWait = 0;
        }

        result = sony_demod_dvbt2_CheckL1PostValid (pInteg->pDemod, &l1PostValid);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* If L1 Post is valid, return from loop, else continue waiting */
        if (l1PostValid) {
            SONY_TRACE_RETURN (SONY_RESULT_OK);
        }

        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (continueWait) {
            result = sony_stopwatch_sleep (&timer, SONY_DVBT2_WAIT_LOCK_INTERVAL);
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
