/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_demod.h"
#include "sony_demod_dvbs_s2.h"
#include "sony_demod_dvbs_s2_monitor.h"
#include "sony_demod_dvbs_s2_blindscan.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_bt.h"
#include "sony_demod_dvbs_s2_blindscan_trace.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t finish_ok (sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeq);
static sony_result_t finish_ng (sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeq);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_bt_Initialize (sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeq,
                                                                 sony_demod_dvbs_s2_blindscan_seq_common_t * pCommonParams)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_subseq_bt_Initialize");

    if ((!pSeq) || (!pCommonParams)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isEnable = 0;
    pSeq->state = BT_STATE_INIT;
    pSeq->pCommonParams = pCommonParams;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_blindscan_subseq_bt_Start (sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeq,
                                                            uint32_t centerFreqKHz,
                                                            uint32_t candSymbolRateKSps,
                                                            uint32_t minCandSymbolRateKSps,
                                                            uint32_t maxCandSymbolRateKSps,
                                                            uint32_t minSymbolRateKSps,
                                                            uint32_t maxSymbolRateKSps)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_subseq_bt_Start");

    if (!pSeq){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isEnable = 1;
    pSeq->state = BT_STATE_START;
    pSeq->centerFreqKHz = centerFreqKHz;
    pSeq->candSymbolRateKSps = candSymbolRateKSps;
    pSeq->minCandSymbolRateKSps = minCandSymbolRateKSps;
    pSeq->maxCandSymbolRateKSps = maxCandSymbolRateKSps;
    pSeq->minSymbolRateKSps = minSymbolRateKSps;
    pSeq->maxSymbolRateKSps = maxSymbolRateKSps;
    pSeq->isLocked = 0;

    pSeq->pCommonParams->waitTime = 0;

    SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_START (centerFreqKHz, candSymbolRateKSps, minCandSymbolRateKSps, maxCandSymbolRateKSps, minSymbolRateKSps, maxSymbolRateKSps);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_blindscan_subseq_bt_Sequence (sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t srsfin = 0;
    uint8_t trllock = 0;
    uint8_t tslock = 0;
    uint32_t elapsedTime = 0;
    uint32_t detSymbolRateSps = 0;
    uint32_t detSymbolRateKSps = 0;
    uint32_t ratioMax = 0;
    uint32_t ratioMin = 0;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_subseq_bt_Sequence");

    if(!pSeq) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(pSeq->state)
    {
    case BT_STATE_START:

        if (pSeq->maxCandSymbolRateKSps < pSeq->minSymbolRateKSps){
            result = finish_ng (pSeq);
            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_RESULT("NG(range)");
            SONY_TRACE_RETURN (result);
        }

        if (pSeq->maxCandSymbolRateKSps >= 20000){
            /* HS mode */
            result = sony_demod_dvbs_s2_blindscan_SetSampleMode (pSeq->pCommonParams->pDemod, 1);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        } else {
            /* LS mode */
            result = sony_demod_dvbs_s2_blindscan_SetSampleMode (pSeq->pCommonParams->pDemod, 0);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }

        /* Clip 1 */
        if (pSeq->candSymbolRateKSps < pSeq->minSymbolRateKSps) {
            pSeq->candSymbolRateKSps = pSeq->minSymbolRateKSps;
        }
        if (pSeq->candSymbolRateKSps > pSeq->maxSymbolRateKSps){
            pSeq->candSymbolRateKSps = pSeq->maxSymbolRateKSps;
        }
        if (pSeq->minCandSymbolRateKSps < pSeq->minSymbolRateKSps) {
            pSeq->minCandSymbolRateKSps = pSeq->minSymbolRateKSps;
        }
        if (pSeq->minCandSymbolRateKSps > pSeq->maxSymbolRateKSps){
            pSeq->minCandSymbolRateKSps = pSeq->maxSymbolRateKSps;
        }
        if (pSeq->maxCandSymbolRateKSps < pSeq->minSymbolRateKSps) {
            pSeq->maxCandSymbolRateKSps = pSeq->minSymbolRateKSps;
        }
        if (pSeq->maxCandSymbolRateKSps > pSeq->maxSymbolRateKSps){
            pSeq->maxCandSymbolRateKSps = pSeq->maxSymbolRateKSps;
        }

        /* Clip 2 */
        if (pSeq->maxCandSymbolRateKSps < pSeq->candSymbolRateKSps){
            pSeq->maxCandSymbolRateKSps = pSeq->candSymbolRateKSps;
        }
        if (pSeq->maxCandSymbolRateKSps > (pSeq->candSymbolRateKSps * 2)){
            pSeq->maxCandSymbolRateKSps = (pSeq->candSymbolRateKSps * 2);
        }
        if (pSeq->minCandSymbolRateKSps > pSeq->candSymbolRateKSps){
            pSeq->minCandSymbolRateKSps = pSeq->candSymbolRateKSps;
        }

        /* Set target symbol rate */
        result = sony_demod_dvbs_s2_SetSymbolRate (pSeq->pCommonParams->pDemod, pSeq->candSymbolRateKSps);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        if(pSeq->candSymbolRateKSps == 0){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }

        ratioMax = ((pSeq->maxCandSymbolRateKSps * 1024) / (pSeq->candSymbolRateKSps)) - 1024;
        ratioMin = ((pSeq->minCandSymbolRateKSps * 1024) / (pSeq->candSymbolRateKSps));

        if (ratioMin >= 1024){
            ratioMin = 1023;
        }

        result = sony_demod_dvbs_s2_blindscan_SetSymbolRateRatio (pSeq->pCommonParams->pDemod, ratioMin, ratioMax);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        /* Tune */
        pSeq->pCommonParams->tuneReq.isRequest = 1;
        pSeq->pCommonParams->tuneReq.frequencyKHz = pSeq->centerFreqKHz;
        pSeq->pCommonParams->tuneReq.system = SONY_DTV_SYSTEM_DVBS;
        pSeq->pCommonParams->tuneReq.symbolRateKSps = pSeq->maxCandSymbolRateKSps;
        pSeq->pCommonParams->waitTime = 0;
        pSeq->state = BT_STATE_RF_TUNED;
        break;

    case BT_STATE_RF_TUNED:
        result = sony_demod_SoftReset (pSeq->pCommonParams->pDemod);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        result = sony_stopwatch_start (&(pSeq->stopwatch));
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        pSeq->pCommonParams->waitTime = 10;
        pSeq->state = BT_STATE_WAIT_SRSFIN;
        break;

    case BT_STATE_WAIT_SRSFIN:
        result = sony_stopwatch_elapsed (&(pSeq->stopwatch), &elapsedTime);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        result = sony_demod_dvbs_s2_blindscan_GetSRSFIN (pSeq->pCommonParams->pDemod, &srsfin);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        if (srsfin){
            /* Check TRL lock */
            result = sony_demod_dvbs_s2_blindscan_CheckTRLLock (pSeq->pCommonParams->pDemod, &trllock);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            if (trllock){
                /* Symbol rate information */
                result = sony_demod_dvbs_s2_monitor_SymbolRate (pSeq->pCommonParams->pDemod, &detSymbolRateSps);
                switch(result)
                {
                case SONY_RESULT_OK:
                    /* Set symbol rate */
                    pSeq->detSymbolRateSps = detSymbolRateSps;
                    break;

                case SONY_RESULT_ERROR_HW_STATE:
                    /* Not detect */
                    SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_RESULT("NG(SR)");
                    result = finish_ng (pSeq);
                    SONY_TRACE_RETURN (result);

                default:
                    SONY_TRACE_RETURN (result);
                }
                detSymbolRateKSps = (detSymbolRateSps + 500) / 1000;
                pSeq->timeout = ((3600000 + (detSymbolRateKSps - 1)) / detSymbolRateKSps) + 150;
                result = sony_stopwatch_start (&(pSeq->stopwatch));
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                pSeq->state = BT_STATE_WAIT_TSLOCK;
                pSeq->pCommonParams->waitTime = 10;
            } else {
                /* TRL unlock */
                SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_RESULT("NG(TRL unlock)");
                result = finish_ng (pSeq);
            }
        } else {
            if (elapsedTime > 10000){
                /* Timeout */
                SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_RESULT("NG(SRS timeout)");
                result = finish_ng (pSeq);
            } else {
                /* Continue */
                pSeq->pCommonParams->waitTime = 10;
                pSeq->state = BT_STATE_WAIT_SRSFIN;
            }
        }
        break;

    case BT_STATE_WAIT_TSLOCK:
        result = sony_stopwatch_elapsed (&(pSeq->stopwatch), &elapsedTime);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        /* Monitor TSLock, CarrierOffset, System */
        result = sony_demod_dvbs_s2_monitor_ScanInfo (pSeq->pCommonParams->pDemod,
                                                      &tslock,
                                                      &(pSeq->detCarrierOffsetKHz),
                                                      &(pSeq->detSystem));
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        if (tslock){
            /* Store system information */
            pSeq->pCommonParams->pDemod->system = pSeq->detSystem;

            /* Success */
            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_RESULT("OK");
            result = finish_ok (pSeq);
            SONY_TRACE_RETURN (result);
        } else {
            if (elapsedTime > pSeq->timeout){
                uint8_t plscLock = 0;
                uint8_t pilotOn = 0;
                /* Not detected channel */
                result = sony_demod_dvbs_s2_monitor_Pilot (pSeq->pCommonParams->pDemod, &plscLock, &pilotOn);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                if ((plscLock != 0) && (pilotOn == 0)) {
                    /* Calculate timeout time for Pilot off signal */
                    detSymbolRateKSps = (pSeq->detSymbolRateSps + 500) / 1000;
                    pSeq->timeout = ((3600000 + (detSymbolRateKSps - 1)) / detSymbolRateKSps) + 150;
                    /* Restart timer */
                    result = sony_stopwatch_start (&(pSeq->stopwatch));
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN (result);
                    }
                    pSeq->state = BT_STATE_WAIT_TSLOCK2;
                    pSeq->pCommonParams->waitTime = 10;
                } else {
                    SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_RESULT("NG( TS lock timeout)");
                    result = finish_ng (pSeq);
                    SONY_TRACE_RETURN (result);
                }
            } else {
                /* Continue to wait */
            }
        }
        break;

    case BT_STATE_WAIT_TSLOCK2:
        result = sony_stopwatch_elapsed (&(pSeq->stopwatch), &elapsedTime);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        /* Monitor TSLock, CarrierOffset, System */
        result = sony_demod_dvbs_s2_monitor_ScanInfo (pSeq->pCommonParams->pDemod,
                                                      &tslock,
                                                      &(pSeq->detCarrierOffsetKHz),
                                                      &(pSeq->detSystem));
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        if (tslock){
            /* Store system information */
            pSeq->pCommonParams->pDemod->system = pSeq->detSystem;

            /* Success */
            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_RESULT("OK2");
            result = finish_ok (pSeq);
            SONY_TRACE_RETURN (result);
        } else {
            if (elapsedTime > pSeq->timeout){
                SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_RESULT("NG2( TS lock timeout)");
                result = finish_ng (pSeq);
                SONY_TRACE_RETURN (result);
            } else {
                /* Continue to wait */
            }
        }
        break;

    default:
        SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_BT_RESULT("NG(SW state)");
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t finish_ok (sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeq)
{
    if(!pSeq) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isLocked = 1;
    pSeq->isEnable = 0;
    return SONY_RESULT_OK;
}

static sony_result_t finish_ng (sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeq)
{
    if(!pSeq) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isLocked = 0;
    pSeq->isEnable = 0;
    return SONY_RESULT_OK;
}
