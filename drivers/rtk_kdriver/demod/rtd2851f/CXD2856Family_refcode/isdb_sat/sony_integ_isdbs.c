/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/04/13
  Modification ID : 8382e363f23ef874189bb623136d4ecd6d419004
------------------------------------------------------------------------------*/
#include "sony_integ.h"
#include "sony_integ_isdbs.h"
#include "sony_demod.h"
#include "sony_demod_isdbs.h"
#include "sony_demod_isdbs_monitor.h"


/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_integ_isdbs_Tune (sony_integ_t * pInteg,
                                     sony_isdbs_tune_param_t * pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_isdbs_Tune");

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

    result = sony_demod_isdbs_Tune (pInteg->pDemod, pTuneParam);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if ((pInteg->pTunerSat) && (pInteg->pTunerSat->Tune)) {
        /* Enable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = pInteg->pTunerSat->Tune (pInteg->pTunerSat, pTuneParam->centerFreqKHz, SONY_DTV_SYSTEM_ISDBS, 28860);
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

    /* Wait for TS lock */
    result = sony_integ_isdbs_WaitTSLock (pInteg);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_isdbs_WaitTSLock (sony_integ_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;

    SONY_TRACE_ENTER ("sony_integ_isdbs_WaitTSLock");

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

        if (elapsed >= SONY_ISDBS_WAIT_TS_LOCK) {
            continueWait = 0;
        }

        result = sony_demod_isdbs_CheckTSLock (pInteg->pDemod, &lock);
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
            result = sony_stopwatch_sleep (&timer, SONY_ISDBS_WAIT_LOCK_INTERVAL);
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

sony_result_t sony_integ_isdbs_monitor_RFLevel (sony_integ_t * pInteg,
                                                int32_t * pRFLeveldB)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t ifagc = 0;
    int32_t gain = 0;
    SONY_TRACE_ENTER("sony_integ_isdbs_monitor_RFLevel");

    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pTunerSat) ||
        (!pInteg->pTunerSat->AGCLevel2AGCdB) || (!pRFLeveldB)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)){
        /* This api is accepted in Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_isdbs_monitor_IFAGCOut (pInteg->pDemod, &ifagc);
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
