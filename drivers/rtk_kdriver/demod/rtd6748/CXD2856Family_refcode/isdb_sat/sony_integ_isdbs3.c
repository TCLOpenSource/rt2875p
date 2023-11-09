/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/09/19
  Modification ID : c6dd9af39653338199a835e79fc5262ecbd324b6
------------------------------------------------------------------------------*/
#include "sony_integ.h"
#include "sony_integ_isdbs3.h"
#include "sony_demod.h"
#include "sony_demod_isdbs3.h"
#include "sony_demod_isdbs3_monitor.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
/*
 @brief Polls the demodulator waiting for TSTLV lock at 10ms intervals up to a timeout of 3s.
*/
static sony_result_t isdbs3_WaitTSTLVLock (sony_integ_t * pInteg);

/*
 @brief Polls the demodulator waiting for TMCC lock at 10ms intervals up to a timeout of 2s.
*/
static sony_result_t isdbs3_WaitTMCCLock (sony_integ_t * pInteg);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_integ_isdbs3_Tune (sony_integ_t * pInteg,
                                      sony_isdbs3_tune_param_t * pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_isdbs3_tmcc_info_t tmccInfo;
    sony_isdbs3_tune_param_t tuneParamRelativeStream;

    SONY_TRACE_ENTER ("sony_integ_isdbs3_Tune");

    if ((!pInteg) || (!pTuneParam) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
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

    if (pTuneParam->streamidType == SONY_ISDBS3_STREAMID_TYPE_RELATIVE_STREAM_NUMBER) {
        if (pTuneParam->streamid > 15) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }

        /* Create tune_param for relative stream number */
        tuneParamRelativeStream.centerFreqKHz = pTuneParam->centerFreqKHz;
        tuneParamRelativeStream.streamid = 0xFFFF;
        tuneParamRelativeStream.streamidType = SONY_ISDBS3_STREAMID_TYPE_STREAMID;

        result = sony_demod_isdbs3_Tune (pInteg->pDemod, &tuneParamRelativeStream);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    } else {
        result = sony_demod_isdbs3_Tune (pInteg->pDemod, pTuneParam);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    }

    if ((pInteg->pTunerSat) && (pInteg->pTunerSat->Tune)) {
        /* Enable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = pInteg->pTunerSat->Tune (pInteg->pTunerSat, pTuneParam->centerFreqKHz, SONY_DTV_SYSTEM_ISDBS3, 33756);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        /* Disable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    }

    result = sony_demod_TuneEnd(pInteg->pDemod);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Wait for TMCC lock */
    result = isdbs3_WaitTMCCLock (pInteg);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if (pTuneParam->streamidType == SONY_ISDBS3_STREAMID_TYPE_RELATIVE_STREAM_NUMBER) {
        /* Get StreamID from TMCC information */
        result = sony_demod_isdbs3_monitor_TMCCInfo(pInteg->pDemod, &tmccInfo);
        if (result == SONY_RESULT_ERROR_HW_STATE) {
            /* TMCC lock is lost causing monitor to fail, return UNLOCK instead of HW STATE */
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);
        } else if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (tmccInfo.streamTypeForEachRelativeStream[pTuneParam->streamid] == SONY_ISDBS3_STREAM_TYPE_NO_TYPE_ALLOCATED) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);
        }

        /* Set Stream ID */
        result = sony_demod_isdbs3_SetStreamID (pInteg->pDemod, tmccInfo.streamidForEachRelativeStream[pTuneParam->streamid]);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    } else {
        uint8_t streamIDError = 0;

        /* Check Stream ID */
        result = sony_demod_isdbs3_monitor_StreamIDError (pInteg->pDemod, &streamIDError);
        if (result == SONY_RESULT_ERROR_HW_STATE) {
            /* TMCC lock is lost causing monitor to fail, return UNLOCK instead of HW STATE */
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);
        } else if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (streamIDError) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);
        }
    }

    /* Wait for TSTLV lock */
    result = isdbs3_WaitTSTLVLock (pInteg);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_isdbs3_monitor_RFLevel (sony_integ_t * pInteg,
                                                 int32_t * pRFLeveldB)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t ifagc = 0;
    int32_t gain = 0;
    SONY_TRACE_ENTER("sony_integ_isdbs3_monitor_RFLevel");

    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pTunerSat) ||
        (!pInteg->pTunerSat->AGCLevel2AGCdB) || (!pRFLeveldB)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)){
        /* This api is accepted in Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_isdbs3_monitor_IFAGCOut (pInteg->pDemod, &ifagc);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = pInteg->pTunerSat->AGCLevel2AGCdB (pInteg->pTunerSat, ifagc, &gain);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* RF_Level = Input_Ref_Level - RF_Gain
     * For this demodulator
     *  - Input_Ref_Level = 0dB
     *  - Gain is in units dB*100
     * Therefore:
     * RF_Level (dB*1000) =  -10 * RF_Gain
     */
    *pRFLeveldB = gain * (-10);

    SONY_TRACE_RETURN(result);
}

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t isdbs3_WaitTSTLVLock (sony_integ_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;

    SONY_TRACE_ENTER ("isdbs3_WaitTSTLVLock");

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Wait for TSTLV lock */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    for (;;) {
        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (elapsed >= SONY_ISDBS3_WAIT_TSTLV_LOCK) {
            continueWait = 0;
        }

        result = sony_demod_isdbs3_CheckTSTLVLock (pInteg->pDemod, &lock);
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
            result = sony_stopwatch_sleep (&timer, SONY_ISDBS3_WAIT_LOCK_INTERVAL);
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

static sony_result_t isdbs3_WaitTMCCLock (sony_integ_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;

    SONY_TRACE_ENTER ("isdbs3_WaitTMCCLock");

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Wait for TMCC lock */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    for (;;) {
        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (elapsed >= SONY_ISDBS3_WAIT_TMCC_LOCK) {
            continueWait = 0;
        }

        result = sony_demod_isdbs3_CheckTMCCLock (pInteg->pDemod, &lock);
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
            result = sony_stopwatch_sleep (&timer, SONY_ISDBS3_WAIT_LOCK_INTERVAL);
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

