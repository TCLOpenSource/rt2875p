/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_demod.h"
#include "sony_demod_dvbs_s2.h"
#include "sony_demod_dvbs_s2_blindscan.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_algo.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_bt.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_cs.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_fs.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_pm.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_ss.h"
#include "sony_demod_dvbs_s2_blindscan_utility.h"
#include "sony_demod_dvbs_s2_tune_srs.h"
#include "sony_demod_dvbs_s2_tune_srs_seq.h"
#include "sony_demod_dvbs_s2_blindscan_trace.h"

sony_result_t sony_demod_dvbs_s2_tune_srs_seq_Initialize (sony_demod_dvbs_s2_tune_srs_seq_t * pSeq,
                                                          sony_demod_t * pDemod,
                                                          uint32_t centerFreqKHz,
                                                          sony_demod_dvbs_s2_tune_srs_freq_range_t freqRange)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_INIT();
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_tune_srs_seq_Initialize");

    if ((!pSeq) || (!pDemod)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(freqRange)
    {
    case SONY_DEMOD_DVBS_S2_TUNE_SRS_FREQ_RANGE_10MHz:
        pSeq->rangeFreqMHz = 10;
        break;

    case SONY_DEMOD_DVBS_S2_TUNE_SRS_FREQ_RANGE_5MHz:
        pSeq->rangeFreqMHz = 5;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isContinue = 1;
    pSeq->isDetect = 0;
    pSeq->seqState = TUNE_SRS_SEQ_STATE_START;
    pSeq->centerFreqKHz = centerFreqKHz;
    pSeq->minFreqKHz = centerFreqKHz - ((pSeq->rangeFreqMHz) * 1000);   /* Frequency capture range */
    pSeq->maxFreqKHz = centerFreqKHz + ((pSeq->rangeFreqMHz) * 1000);   /* Frequency capture range */
    pSeq->minSymbolRateKSps = 1000;                                     /* Symbol rate capture range (Fixed) */
    pSeq->maxSymbolRateKSps = 45000;                                    /* Symbol rate capture range (Fixed) */

    /*
     * 500KHz step search is for 1 - 20MSps range.
     *
     * Max(20MSps) * 1.35 / 2 = 13.5MSps -> 14MHz
     */
    pSeq->powerMinFreqKHz1 = pSeq->minFreqKHz - 14000;
    pSeq->powerMaxFreqKHz1 = pSeq->maxFreqKHz + 14000;
    /*
     * 2000KHz step search is for 20 - 45MSps range.
     *
     * Max(45MSps) * 1.35 / 2 = 30.375MSps
     * 30.375MHz - 14MHz = 16.375MHz -> 20MHz
     */
    pSeq->powerMinFreqKHz2 = pSeq->powerMaxFreqKHz1;
    pSeq->powerMaxFreqKHz2 = pSeq->powerMaxFreqKHz1 + 20000;
    pSeq->powerMinFreqKHz3 = pSeq->powerMinFreqKHz1 - 20000;
    pSeq->powerMaxFreqKHz3 = pSeq->powerMinFreqKHz1;

    if ((pSeq->powerMaxFreqKHz2 - pSeq->powerMinFreqKHz3) < 72000){
        pSeq->powerMinFreqKHz1 = pSeq->powerMinFreqKHz3;
        pSeq->powerMaxFreqKHz1 = pSeq->powerMaxFreqKHz2;
        pSeq->powerMinFreqKHz2 = 0;
        pSeq->powerMaxFreqKHz2 = 0;
        pSeq->powerMinFreqKHz3 = 0;
        pSeq->powerMaxFreqKHz3 = 0;
    }

    pSeq->commonParams.pDemod = pDemod;
    pSeq->commonParams.detInfo.isDetect = 0;
    pSeq->commonParams.tuneReq.isRequest = 0;
    pSeq->commonParams.agcInfo.isRequest = 0;
    pSeq->commonParams.waitTime = 0;

    pSeq->commonParams.ckalFreqKHz = 64000; /* CKAL */
    pSeq->commonParams.ckahFreqKHz = 96000; /* CKAH */

    /* Initialize the data storage */
    result = sony_demod_dvbs_s2_blindscan_StorageInit (&(pSeq->commonParams.storage),
                                                       pSeq->powerArray,
                                                       sizeof(pSeq->powerArray)/sizeof(pSeq->powerArray[0]),
                                                       pSeq->dataArray,
                                                       sizeof(pSeq->dataArray)/sizeof(pSeq->dataArray[0]));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Initialize demodulator to start BlindScan. */
    result = sony_demod_dvbs_s2_blindscan_Initialize(pSeq->commonParams.pDemod);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_dvbs_s2_blindscan_subseq_bt_Initialize (&pSeq->subseqBT, &pSeq->commonParams);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_dvbs_s2_blindscan_subseq_pm_Initialize (&pSeq->subseqPM, &pSeq->commonParams);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_dvbs_s2_blindscan_subseq_ss_Initialize (&pSeq->subseqSS, &pSeq->commonParams);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_dvbs_s2_blindscan_subseq_fs_Initialize (&pSeq->subseqFS, &pSeq->subseqBT, &pSeq->commonParams);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_dvbs_s2_blindscan_subseq_cs_Initialize (&pSeq->subseqCS, &pSeq->subseqPM, &pSeq->commonParams);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData(&(pSeq->commonParams.storage), &(pSeq->pCandList1));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }
    pSeq->pCandLast1 = pSeq->pCandList1;

    result = sony_demod_dvbs_s2_blindscan_AllocData(&(pSeq->commonParams.storage), &(pSeq->pCandList2));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }
    pSeq->pCandLast2 = pSeq->pCandList2;

    result = sony_demod_dvbs_s2_blindscan_AllocPower(&(pSeq->commonParams.storage), &(pSeq->pPowerList));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData(&(pSeq->commonParams.storage), &(pSeq->pDetectedList));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_tune_srs_seq_Sequence (sony_demod_dvbs_s2_tune_srs_seq_t * pSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_tune_srs_seq_Sequence");

    if (!pSeq){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pSeq->subseqSS.isEnable){
        /* SS seq */
        result = sony_demod_dvbs_s2_blindscan_subseq_ss_Sequence (&pSeq->subseqSS);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    } else if (pSeq->subseqBT.isEnable){
        /* BT seq */
        result = sony_demod_dvbs_s2_blindscan_subseq_bt_Sequence (&pSeq->subseqBT);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    } else if (pSeq->subseqCS.isEnable){
        if (pSeq->subseqPM.isEnable){
            /* PM seq */
            result = sony_demod_dvbs_s2_blindscan_subseq_pm_Sequence(&pSeq->subseqPM);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        } else {
            /* CS seq */
            result = sony_demod_dvbs_s2_blindscan_subseq_cs_Sequence(&pSeq->subseqCS);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
    } else {
        /* Main sequence */
        switch(pSeq->seqState)
        {
        case TUNE_SRS_SEQ_STATE_START:
            result = sony_demod_dvbs_s2_blindscan_subseq_ss_Start(&pSeq->subseqSS,
                                                                  pSeq->powerMinFreqKHz1,
                                                                  pSeq->powerMaxFreqKHz1,
                                                                  500,
                                                                  pSeq->powerMaxFreqKHz1 - pSeq->powerMinFreqKHz1,
                                                                  pSeq->commonParams.pDemod->dvbss2PowerSmooth);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            pSeq->commonParams.waitTime = 0;
            pSeq->seqState = TUNE_SRS_SEQ_STATE_SS1_FIN;
            break;

        case TUNE_SRS_SEQ_STATE_SS1_FIN:
            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_POWERLIST("1st", pSeq->subseqSS.pPowerList);
            result = sony_demod_dvbs_s2_blindscan_algo_GetCandidateNml (&(pSeq->commonParams.storage),
                                                                        pSeq->subseqSS.pPowerList,
                                                                        200,
                                                                        1000,
                                                                        5000,
                                                                        1000,
                                                                        pSeq->pCandList1);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            result = sony_demod_dvbs_s2_blindscan_algo_SeparateCandidate (&(pSeq->commonParams.storage), pSeq->pCandList1);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            result = sony_demod_dvbs_s2_blindscan_algo_DeleteDuplicate2 (&(pSeq->commonParams.storage), pSeq->pCandList1);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CANDLIST("1st(1-5)", pSeq->pCandList1);

            /* Tune for coarse search */
            pSeq->commonParams.tuneReq.isRequest = 1;
            pSeq->commonParams.tuneReq.frequencyKHz = pSeq->centerFreqKHz;
            pSeq->commonParams.tuneReq.symbolRateKSps = 15000;
            pSeq->commonParams.tuneReq.system = SONY_DTV_SYSTEM_DVBS;

            /* Set the top candidate */
            pSeq->pCandCurrent = pSeq->pCandList1->pNext;

            pSeq->commonParams.waitTime = 0;
            pSeq->seqState = TUNE_SRS_SEQ_STATE_RF_TUNED;
            break;

        case TUNE_SRS_SEQ_STATE_RF_TUNED:
            if (pSeq->pCandCurrent){
                int32_t offsetFreqKHz = (int32_t)(pSeq->pCandCurrent->data.candidate.centerFreqKHz) - (int32_t)(pSeq->commonParams.tuneReq.frequencyKHz);
                if ((((int32_t)((pSeq->rangeFreqMHz * 1000) + 3000) * (-1)) <= offsetFreqKHz) && (offsetFreqKHz <= (int32_t)((pSeq->rangeFreqMHz * 1000) + 3000))){
                    SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CS_TUNE (pSeq->commonParams.tuneReq.frequencyKHz);
                    result = sony_demod_dvbs_s2_blindscan_subseq_cs_Start (&(pSeq->subseqCS), offsetFreqKHz);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN (result);
                    }
                } else {
                    pSeq->subseqCS.isExist = 0;
                }
                pSeq->commonParams.waitTime = 0;
                pSeq->seqState = TUNE_SRS_SEQ_STATE_CS1_FIN;
            } else {
                result = sony_demod_dvbs_s2_blindscan_algo_DeleteDuplicate (&(pSeq->commonParams.storage), pSeq->pCandList2);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }

                result = sony_demod_dvbs_s2_blindscan_ClearDataList (&(pSeq->commonParams.storage), pSeq->pCandList1);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }

                pSeq->pCandList1->pNext = pSeq->pCandList2->pNext;
                pSeq->pCandList2->pNext = NULL;

                if (((pSeq->powerMaxFreqKHz2 - pSeq->powerMinFreqKHz2) == 0) || ((pSeq->powerMaxFreqKHz3 - pSeq->powerMinFreqKHz3) == 0)){
                    pSeq->commonParams.waitTime = 0;
                    pSeq->seqState = TUNE_SRS_SEQ_STATE_CAND2;
                } else {
                    pSeq->commonParams.waitTime = 0;
                    pSeq->seqState = TUNE_SRS_SEQ_STATE_CS1_FIN2;
                }
            }
            break;

        case TUNE_SRS_SEQ_STATE_CS1_FIN:
            if (pSeq->subseqCS.isExist){
                sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
                result = sony_demod_dvbs_s2_blindscan_AllocData (&(pSeq->commonParams.storage), &pTemp);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                pTemp->data.candidate.centerFreqKHz = (uint32_t)((int32_t)pSeq->commonParams.tuneReq.frequencyKHz + pSeq->subseqCS.freqOffsetKHz);
                pTemp->data.candidate.symbolRateKSps = pSeq->subseqCS.coarseSymbolRateKSps;
                pTemp->data.candidate.minSymbolRateKSps = (pSeq->subseqCS.coarseSymbolRateKSps * 700) / 1000;
                pTemp->data.candidate.maxSymbolRateKSps = ((pSeq->subseqCS.coarseSymbolRateKSps * 1600) + 999) / 1000;
                pSeq->pCandLast2->pNext = pTemp;
                pSeq->pCandLast2 = pSeq->pCandLast2->pNext;
            }
            pSeq->pCandCurrent = pSeq->pCandCurrent->pNext;
            pSeq->commonParams.waitTime = 0;
            pSeq->seqState = TUNE_SRS_SEQ_STATE_RF_TUNED;
            break;

        case TUNE_SRS_SEQ_STATE_CS1_FIN2:
            /* Set SS1 data to list */
            pSeq->pPowerList->pNext = pSeq->subseqSS.pPowerList->pNext;
            pSeq->subseqSS.pPowerList->pNext = NULL;

            result = sony_demod_dvbs_s2_blindscan_subseq_ss_Start (&(pSeq->subseqSS),
                                                                   pSeq->powerMinFreqKHz2,
                                                                   pSeq->powerMaxFreqKHz2,
                                                                   2000,
                                                                   pSeq->powerMaxFreqKHz2 - pSeq->powerMinFreqKHz2,
                                                                   pSeq->commonParams.pDemod->dvbss2PowerSmooth);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            pSeq->seqState = TUNE_SRS_SEQ_STATE_SS2_FIN;
            pSeq->commonParams.waitTime = 0;
            break;

        case TUNE_SRS_SEQ_STATE_SS2_FIN:
            /* Set SS2 data to list */
            pSeq->pPowerLast = pSeq->pPowerList;
            while(pSeq->pPowerLast->pNext){
                pSeq->pPowerLast = pSeq->pPowerLast->pNext;
            }
            pSeq->pPowerLast->pNext = pSeq->subseqSS.pPowerList->pNext;
            pSeq->subseqSS.pPowerList->pNext = NULL;

            result = sony_demod_dvbs_s2_blindscan_subseq_ss_Start (&(pSeq->subseqSS),
                                                                   pSeq->powerMinFreqKHz3,
                                                                   pSeq->powerMaxFreqKHz3,
                                                                   2000,
                                                                   pSeq->powerMaxFreqKHz3 - pSeq->powerMinFreqKHz3,
                                                                   pSeq->commonParams.pDemod->dvbss2PowerSmooth);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            pSeq->seqState = TUNE_SRS_SEQ_STATE_SS3_FIN;
            pSeq->commonParams.waitTime = 0;
            break;

        case TUNE_SRS_SEQ_STATE_SS3_FIN:

            /* Set SS3 data to list */
            pSeq->pPowerLast = pSeq->subseqSS.pPowerList;
            while(pSeq->pPowerLast->pNext){
                pSeq->pPowerLast = pSeq->pPowerLast->pNext;
            }
            pSeq->pPowerLast->pNext = pSeq->pPowerList->pNext;
            pSeq->pPowerList->pNext = pSeq->subseqSS.pPowerList->pNext;
            pSeq->seqState = TUNE_SRS_SEQ_STATE_CAND2;
            pSeq->commonParams.waitTime = 0;
            break;

        case TUNE_SRS_SEQ_STATE_CAND2:

            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CANDLIST("1st", pSeq->pCandList1);

            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_POWERLIST("2nd", pSeq->pPowerList);
            result = sony_demod_dvbs_s2_blindscan_algo_GetCandidateNml (&(pSeq->commonParams.storage),
                                                                        pSeq->pPowerList,
                                                                        100,
                                                                        20000,
                                                                        45000,
                                                                        4000,
                                                                        pSeq->pCandList2);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CANDLIST("2nd(20-45)", pSeq->pCandList2);

            pSeq->pCandLast1 = pSeq->pCandList1;
            while(pSeq->pCandLast1->pNext){
                pSeq->pCandLast1 = pSeq->pCandLast1->pNext;
            }
            pSeq->pCandLast1->pNext = pSeq->pCandList2->pNext;
            pSeq->pCandList2->pNext = NULL;


            result = sony_demod_dvbs_s2_blindscan_algo_GetCandidateNml (&(pSeq->commonParams.storage),
                                                                        pSeq->pPowerList,
                                                                        100,
                                                                        5000,
                                                                        20000,
                                                                        1000,
                                                                        pSeq->pCandList2);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CANDLIST("3rd(5-20)", pSeq->pCandList2);

            while(pSeq->pCandLast1->pNext){
                pSeq->pCandLast1 = pSeq->pCandLast1->pNext;
            }
            pSeq->pCandLast1->pNext = pSeq->pCandList2->pNext;
            pSeq->pCandList2->pNext = NULL;

            result = sony_demod_dvbs_s2_blindscan_algo_SortByFrequency (&(pSeq->commonParams.storage),
                                                                        pSeq->pCandList1,
                                                                        pSeq->centerFreqKHz);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CANDLIST ("final", pSeq->pCandList1);

            pSeq->pCandCurrent = pSeq->pCandList1->pNext;

            pSeq->seqState = TUNE_SRS_SEQ_STATE_BT_START;
            pSeq->commonParams.waitTime = 0;
            break;

        case TUNE_SRS_SEQ_STATE_BT_START:
            if (pSeq->pCandCurrent){
                int32_t offsetFreqKHz = (int32_t)(pSeq->pCandCurrent->data.candidate.centerFreqKHz) - (int32_t)(pSeq->centerFreqKHz);
                /* Performing Blind Tune on the candidates that lie within (rangeFreqMHz +- 3MHz) of the specified tune-frequency. */
                if ((((int32_t)((pSeq->rangeFreqMHz * 1000) + 3000) * (-1)) <= offsetFreqKHz) && (offsetFreqKHz <= (int32_t)((pSeq->rangeFreqMHz * 1000) + 3000))){
                    result = sony_demod_dvbs_s2_blindscan_subseq_bt_Start (&(pSeq->subseqBT),
                                                                           pSeq->pCandCurrent->data.candidate.centerFreqKHz,
                                                                           pSeq->pCandCurrent->data.candidate.symbolRateKSps,
                                                                           pSeq->pCandCurrent->data.candidate.minSymbolRateKSps,
                                                                           pSeq->pCandCurrent->data.candidate.maxSymbolRateKSps,
                                                                           pSeq->minSymbolRateKSps,
                                                                           pSeq->maxSymbolRateKSps);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN (result);
                    }
                    pSeq->seqState = TUNE_SRS_SEQ_STATE_BT_END;
                    pSeq->commonParams.waitTime = 0;
                } else {
                    /* Unlocked */
                    pSeq->isContinue = 0;
                    pSeq->isDetect = 0;
                }
            } else {
                /* Unlocked */
                pSeq->isContinue = 0;
                pSeq->isDetect = 0;
            }
            break;

        case TUNE_SRS_SEQ_STATE_BT_END:
            if (pSeq->subseqBT.isLocked){
                /* Locked */
                pSeq->isContinue = 0;
                pSeq->tuneParam.centerFreqKHz = (uint32_t)((int32_t)(pSeq->commonParams.tuneReq.frequencyKHz) + pSeq->subseqBT.detCarrierOffsetKHz);
                pSeq->tuneParam.symbolRateKSps = (pSeq->subseqBT.detSymbolRateSps + 500) / 1000;
                pSeq->tuneParam.system = pSeq->subseqBT.detSystem;
                if ((pSeq->centerFreqKHz - (pSeq->rangeFreqMHz * 1000) - 100) <= (pSeq->tuneParam.centerFreqKHz) &&
                    ((pSeq->tuneParam.centerFreqKHz) <= pSeq->centerFreqKHz + (pSeq->rangeFreqMHz * 1000) + 100)) {
                    /* Signal is in range. */
                    pSeq->isDetect = 1;
                } else {
                    /* Signal is out of range. */
                    pSeq->isDetect = 0;
                }
            }
            pSeq->pCandCurrent = pSeq->pCandCurrent->pNext;
            pSeq->seqState = TUNE_SRS_SEQ_STATE_BT_START;
            pSeq->commonParams.waitTime = 0;
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
