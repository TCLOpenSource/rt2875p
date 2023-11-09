/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_demod_dvbs_s2_blindscan.h"
#include "sony_demod_dvbs_s2_blindscan_seq.h"
#include "sony_demod_dvbs_s2_blindscan_trace.h"

sony_result_t sony_demod_dvbs_s2_blindscan_subseq_pm_Initialize (sony_demod_dvbs_s2_blindscan_subseq_pm_t * pSeq,
                                                                 sony_demod_dvbs_s2_blindscan_seq_common_t * pCommonParams)
{
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_subseq_pm_Initialize");

    if ((!pSeq) || (!pCommonParams)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isEnable = 0;
    pSeq->state = PM_STATE_START;
    pSeq->pCommonParams = pCommonParams;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_blindscan_subseq_pm_Start (sony_demod_dvbs_s2_blindscan_subseq_pm_t * pSeq,
                                                            int32_t freqOffsetKHz)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_subseq_pm_Start");
    if(!pSeq) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    pSeq->isEnable = 1;
    pSeq->state = PM_STATE_START;
    pSeq->freqOffsetKHz = freqOffsetKHz;
    pSeq->power = 0;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_blindscan_subseq_pm_Sequence (sony_demod_dvbs_s2_blindscan_subseq_pm_t * pSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t elapsedTime = 0;
    uint8_t csfin = 0;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_subseq_pm_Sequence");

    if(!pSeq) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(pSeq->state)
    {
    case PM_STATE_START:
        result = sony_demod_dvbs_s2_blindscan_SetCFFine (pSeq->pCommonParams->pDemod, pSeq->freqOffsetKHz);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        /* Scan start */
        result = sony_demod_dvbs_s2_blindscan_PS_START (pSeq->pCommonParams->pDemod);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        pSeq->state = PM_STATE_WAITING_CSFIN;
        pSeq->pCommonParams->waitTime = 0;
        result = sony_stopwatch_start (&pSeq->stopwatch);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        break;

    case PM_STATE_WAITING_CSFIN:
        result = sony_stopwatch_elapsed (&pSeq->stopwatch, &elapsedTime);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = sony_demod_dvbs_s2_blindscan_GetCSFIN (pSeq->pCommonParams->pDemod, &csfin);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        if (csfin){
            result = sony_demod_dvbs_s2_blindscan_GetPSPow (pSeq->pCommonParams->pDemod, &pSeq->power);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            /* Scan ack */
            result = sony_demod_dvbs_s2_blindscan_PS_RACK (pSeq->pCommonParams->pDemod);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            /* End */
            pSeq->isEnable = 0;
            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_PM_POWER (pSeq->freqOffsetKHz, pSeq->power);
        } else {
            if (elapsedTime >= 1000){
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
            } else {
                /* Continue to wait */
            }
        }
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SONY_TRACE_RETURN (result);
}
