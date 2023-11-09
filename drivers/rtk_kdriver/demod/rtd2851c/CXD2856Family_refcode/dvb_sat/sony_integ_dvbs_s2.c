/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/04/13
  Modification ID : 8382e363f23ef874189bb623136d4ecd6d419004
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_demod.h"
#include "sony_integ_dvbs_s2.h"
#include "sony_demod_dvbs_s2_monitor.h"

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_integ_dvbs_s2_Tune (sony_integ_t * pInteg,
                                       sony_dvbs_s2_tune_param_t * pTuneParam)
{
    sony_stopwatch_t stopwatch;
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_DVBS2;
    uint32_t elapsedTime = 0;
    uint8_t isContinue = 1;
    uint32_t timeout = 0;
    sony_demod_lock_result_t lockStatus = SONY_DEMOD_LOCK_RESULT_NOTDETECT;

    SONY_TRACE_ENTER ("sony_integ_dvbs_s2_Tune");

    if ((!pInteg) || (!pInteg->pDemod) ||
        (!pTuneParam) || (pTuneParam->symbolRateKSps == 0)){
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

    result = sony_demod_dvbs_s2_Tune (pInteg->pDemod, pTuneParam);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (pTuneParam->system == SONY_DTV_SYSTEM_ANY){
        dtvSystem = SONY_DTV_SYSTEM_DVBS;
    } else {
        dtvSystem = pTuneParam->system;
    }

    if ((pInteg->pTunerSat) && (pInteg->pTunerSat->Tune)) {
        /* Enable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = pInteg->pTunerSat->Tune (pInteg->pTunerSat,
                                          pTuneParam->centerFreqKHz,
                                          dtvSystem,
                                          pTuneParam->symbolRateKSps);
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

    result = sony_stopwatch_start (&stopwatch);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (pTuneParam->system == SONY_DTV_SYSTEM_ANY){
        timeout = ((3600000 + (pTuneParam->symbolRateKSps - 1)) / pTuneParam->symbolRateKSps) + 150;
    } else {
        timeout = ((3000000 + (pTuneParam->symbolRateKSps - 1)) / pTuneParam->symbolRateKSps) + 150;
    }

    isContinue = 1;
    while (isContinue) {
        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_stopwatch_elapsed (&stopwatch, &elapsedTime);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = sony_demod_dvbs_s2_CheckTSLock (pInteg->pDemod, &lockStatus);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        if (lockStatus == SONY_DEMOD_LOCK_RESULT_LOCKED) {
            /* Lock */
            SONY_TRACE_RETURN (SONY_RESULT_OK);
        }

        if (elapsedTime > timeout) {
            isContinue = 0;
        } else {
            result = sony_stopwatch_sleep (&stopwatch, SONY_INTEG_DVBS_S2_TUNE_POLLING_INTERVAL);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
    }

    {
        /* Additional wait sequence for pilot off signal */
        uint8_t plscLock = 0;
        uint8_t pilotOn = 0;
        result = sony_demod_dvbs_s2_monitor_Pilot (pInteg->pDemod, &plscLock, &pilotOn);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        if ((plscLock != 0) && (pilotOn == 0)) {
            result = sony_stopwatch_start (&stopwatch);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            if (pTuneParam->system == SONY_DTV_SYSTEM_ANY){
                timeout = ((3600000 + (pTuneParam->symbolRateKSps - 1)) / pTuneParam->symbolRateKSps) + 150;
            } else {
                timeout = ((3000000 + (pTuneParam->symbolRateKSps - 1)) / pTuneParam->symbolRateKSps) + 150;
            }

            isContinue = 1;
            while (isContinue) {
                /* Check cancellation. */
                result = sony_integ_CheckCancellation (pInteg);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                result = sony_stopwatch_elapsed (&stopwatch, &elapsedTime);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }

                result = sony_demod_dvbs_s2_CheckTSLock (pInteg->pDemod, &lockStatus);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }

                if (lockStatus == SONY_DEMOD_LOCK_RESULT_LOCKED) {
                    /* Lock */
                    SONY_TRACE_RETURN (SONY_RESULT_OK);
                }

                if (elapsedTime > timeout) {
                    isContinue = 0;
                } else {
                    result = sony_stopwatch_sleep (&stopwatch, SONY_INTEG_DVBS_S2_TUNE_POLLING_INTERVAL);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN (result);
                    }
                }
            }
        }
    }
    SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
}

sony_result_t sony_integ_dvbs_s2_monitor_RFLevel (sony_integ_t * pInteg,
                                                  int32_t * pRFLeveldB)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t ifagc = 0;
    int32_t gain = 0;
    SONY_TRACE_ENTER("sony_integ_dvbs_s2_monitor_RFLevel");

    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pTunerSat) ||
        (!pInteg->pTunerSat->AGCLevel2AGCdB) || (!pRFLeveldB)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)){
        /* This api is accepted in Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_dvbs_s2_monitor_IFAGCOut (pInteg->pDemod, &ifagc);
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
