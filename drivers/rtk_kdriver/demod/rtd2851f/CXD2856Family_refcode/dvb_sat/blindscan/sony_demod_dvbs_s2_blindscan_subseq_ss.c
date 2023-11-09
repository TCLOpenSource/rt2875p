/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_math.h"
#include "sony_demod.h"
#include "sony_demod_dvbs_s2.h"
#include "sony_demod_dvbs_s2_monitor.h"
#include "sony_demod_dvbs_s2_blindscan.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_utility.h"
#include "sony_demod_dvbs_s2_blindscan_seq.h"

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_ss_Initialize (sony_demod_dvbs_s2_blindscan_subseq_ss_t * pSeq,
                                                                 sony_demod_dvbs_s2_blindscan_seq_common_t * pCommonParams)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_subseq_ss_Initialize");

    if ((!pSeq) || (!pCommonParams)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isEnable = 0;
    pSeq->state = SS_STATE_INIT;

    pSeq->maxFreqKHz = 0;
    pSeq->minFreqKHz = 0;
    pSeq->stepFreqKHz = 0;
    pSeq->tunerStepFreqKHz = 0;

    pSeq->pCommonParams = pCommonParams;

    result = sony_demod_dvbs_s2_blindscan_AllocPower (&(pSeq->pCommonParams->storage), &(pSeq->pPowerList));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_blindscan_subseq_ss_Start (sony_demod_dvbs_s2_blindscan_subseq_ss_t * pSeq,
                                                            uint32_t minFreqKHz,
                                                            uint32_t maxFreqKHz,
                                                            uint32_t stepFreqKHz,
                                                            uint32_t tunerStepFreqKHz,
                                                            uint8_t powerSmooth)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_subseq_ss_Start");

    if (!pSeq){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isEnable = 1;
    pSeq->state = SS_STATE_START;

    pSeq->minFreqKHz = minFreqKHz;
    pSeq->maxFreqKHz = maxFreqKHz;
    pSeq->stepFreqKHz = stepFreqKHz;
    pSeq->tunerStepFreqKHz = tunerStepFreqKHz;

    pSeq->pCommonParams->waitTime = 0;

    pSeq->ocfr_csk = powerSmooth;

    result = sony_demod_dvbs_s2_blindscan_ClearPowerList (&(pSeq->pCommonParams->storage), pSeq->pPowerList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }
    pSeq->pPowerListLast = pSeq->pPowerList;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_blindscan_subseq_ss_Sequence(sony_demod_dvbs_s2_blindscan_subseq_ss_t * pSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t csfin = 0;
    uint8_t csrdend = 0;
    int32_t csfreq = 0;
    uint32_t cspow = 0;
    uint32_t elapsedTime = 0;
    int32_t offsetFreq = 0;
    uint32_t ckaFreqKHz = 0;
    sony_demod_dvbs_s2_blindscan_power_t * pTempPower = NULL;

    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_subseq_ss_Sequence");

    if(!pSeq) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(pSeq->state)
    {
    case SS_STATE_START:
        result = sony_demod_dvbs_s2_blindscan_PS_INIT (pSeq->pCommonParams->pDemod, pSeq->ocfr_csk);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        offsetFreq = (int32_t)pSeq->tunerStepFreqKHz / 2;
        ckaFreqKHz = pSeq->pCommonParams->ckahFreqKHz;

        result = sony_demod_dvbs_s2_blindscan_PS_SET (pSeq->pCommonParams->pDemod,
                                                      -offsetFreq,
                                                      offsetFreq,
                                                      pSeq->stepFreqKHz,
                                                      ckaFreqKHz);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        pSeq->currentFreqKHz = (uint32_t)((int32_t)pSeq->minFreqKHz + offsetFreq);
        pSeq->state = SS_STATE_RF_TUNING;
        pSeq->pCommonParams->waitTime = 0;
        break;

    case SS_STATE_RF_TUNING:
        pSeq->pCommonParams->tuneReq.isRequest = 1;
        pSeq->pCommonParams->tuneReq.frequencyKHz = pSeq->currentFreqKHz;
        pSeq->pCommonParams->tuneReq.system = SONY_DTV_SYSTEM_DVBS;
        pSeq->pCommonParams->tuneReq.symbolRateKSps = 0;
        pSeq->pCommonParams->waitTime = 0;
        pSeq->state = SS_STATE_RF_TUNED;
        break;

    case SS_STATE_RF_TUNED:
        result = sony_demod_dvbs_s2_monitor_IFAGCOut (pSeq->pCommonParams->pDemod, &(pSeq->agcLevel));
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        pSeq->pCommonParams->agcInfo.isRequest = 1;
        pSeq->pCommonParams->agcInfo.agcLevel = pSeq->agcLevel;
        pSeq->pCommonParams->waitTime = 0;
        pSeq->state = SS_STATE_AGC_CALCULATED;
        break;

    case SS_STATE_AGC_CALCULATED:
        pSeq->agc_x100dB = pSeq->pCommonParams->agcInfo.agc_x100dB;
        result = sony_demod_dvbs_s2_blindscan_PS_START (pSeq->pCommonParams->pDemod);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        result = sony_stopwatch_start (&pSeq->stopwatch);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        pSeq->state = SS_STATE_WAIT_CSFIN;
        pSeq->pCommonParams->waitTime = 10;
        break;


    case SS_STATE_WAIT_CSFIN:
        result = sony_stopwatch_elapsed (&pSeq->stopwatch, &elapsedTime);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        result = sony_demod_dvbs_s2_blindscan_GetCSFIN (pSeq->pCommonParams->pDemod, &csfin);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        if (csfin){
            pSeq->state = SS_STATE_READ_CS;
            pSeq->pCommonParams->waitTime = 0;
        } else {
            if (elapsedTime > 10000){
                /* Timeout error */
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
            }
        }
        break;

    case SS_STATE_READ_CS:
        result = sony_demod_dvbs_s2_blindscan_GetCSRDEND(pSeq->pCommonParams->pDemod, &csrdend);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        if (csrdend){
            if ((pSeq->currentFreqKHz + (pSeq->tunerStepFreqKHz / 2)) < pSeq->maxFreqKHz){
                pSeq->currentFreqKHz += pSeq->tunerStepFreqKHz;
                pSeq->state = SS_STATE_RF_TUNING;
                pSeq->pCommonParams->waitTime = 0;
                /* Go to next loop */
            } else {
                pSeq->state = SS_STATE_END;
                pSeq->pCommonParams->waitTime = 0;
            }
        } else {
            ckaFreqKHz = pSeq->pCommonParams->ckahFreqKHz;

            result = sony_demod_dvbs_s2_blindscan_GetCSINFO (pSeq->pCommonParams->pDemod, &csfreq, &cspow);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            /* Add to power list */
            result = sony_demod_dvbs_s2_blindscan_AllocPower (&(pSeq->pCommonParams->storage), &pTempPower);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            /* Power data (frequency) */
            if (csfreq >= 0){
                pTempPower->freqKHz = (uint32_t)((int32_t)(pSeq->pCommonParams->tuneReq.frequencyKHz) + (((csfreq * (int32_t)ckaFreqKHz) + (int32_t)1024) / (int32_t)2048));
            } else {
                pTempPower->freqKHz = (uint32_t)((int32_t)(pSeq->pCommonParams->tuneReq.frequencyKHz) + (((csfreq * (int32_t)ckaFreqKHz) - (int32_t)1024) / (int32_t)2048));
            }

            if (pTempPower->freqKHz > pSeq->maxFreqKHz){
                result = sony_demod_dvbs_s2_blindscan_FreePower (&(pSeq->pCommonParams->storage), pTempPower);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
            } else {
                /*
                 * Power[dB]     = 10 * log10 (CSPOW/2^14) - gain_db
                 * Power[x100dB] = (1000 * log10 (CSPOW/2^14)) - gain_db_x100
                 *               = (1000 * (log10(CSPOW) - log10(2^14))) - gain_db_x100
                 *               = (10 * (log10(CSPOW) - 421)) - gain_db_x100
                 *
                 * log10() in this driver returns "result x 100".
                 */
                pTempPower->power = (int32_t)((sony_math_log10 (cspow) - 421) * 10) - pSeq->pCommonParams->agcInfo.agc_x100dB;

                /* Add data to list */
                pSeq->pPowerListLast->pNext = pTempPower;
                pSeq->pPowerListLast = pSeq->pPowerListLast->pNext;
            }

            result = sony_demod_dvbs_s2_blindscan_PS_RACK (pSeq->pCommonParams->pDemod);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    case SS_STATE_END:
        pSeq->isEnable = 0;
        result = sony_demod_dvbs_s2_blindscan_PS_FIN (pSeq->pCommonParams->pDemod);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        pSeq->pCommonParams->waitTime = 0;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SONY_TRACE_RETURN (result);
}
