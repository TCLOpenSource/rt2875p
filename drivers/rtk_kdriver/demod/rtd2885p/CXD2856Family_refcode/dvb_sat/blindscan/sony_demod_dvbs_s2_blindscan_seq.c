/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_demod.h"
#include "sony_demod_dvbs_s2.h"
#include "sony_demod_dvbs_s2_blindscan.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_algo.h"
#include "sony_demod_dvbs_s2_blindscan_seq.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_bt.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_cs.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_fs.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_pm.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_ss.h"
#include "sony_demod_dvbs_s2_blindscan_utility.h"
#include "sony_demod_dvbs_s2_blindscan_trace.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t setProgress (sony_demod_dvbs_s2_blindscan_seq_t * pSeq,
                                  uint8_t minProgressRange,
                                  uint8_t maxProgressRange,
                                  uint8_t minorProgress);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t sony_demod_dvbs_s2_blindscan_seq_Initialize (sony_demod_dvbs_s2_blindscan_seq_t * pSeq,
                                                           sony_demod_t * pDemod,
                                                           uint32_t minFreqKHz,
                                                           uint32_t maxFreqKHz,
                                                           uint32_t minSymbolRateKSps,
                                                           uint32_t maxSymbolRateKSps)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_INIT();
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_seq_Initialize");

    if ((!pSeq) || (!pDemod)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isContinue = 1;
    pSeq->seqState = BLINDSCAN_SEQ_STATE_START;
    pSeq->minFreqKHz = minFreqKHz;
    pSeq->maxFreqKHz = maxFreqKHz;
    pSeq->minSymbolRateKSps = minSymbolRateKSps;
    pSeq->maxSymbolRateKSps = maxSymbolRateKSps;

    pSeq->minPowerFreqKHz = minFreqKHz - (((maxSymbolRateKSps * 675) + 999) / 1000);
    pSeq->minPowerFreqKHz = (pSeq->minPowerFreqKHz / 1000) * 1000;

    pSeq->maxPowerFreqKHz = maxFreqKHz + (((maxSymbolRateKSps * 675) + 999) / 1000);
    pSeq->maxPowerFreqKHz = ((pSeq->maxPowerFreqKHz + 999) / 1000) * 1000;

    pSeq->commonParams.pDemod = pDemod;
    pSeq->commonParams.detInfo.isDetect = 0;
    pSeq->commonParams.tuneReq.isRequest = 0;
    pSeq->commonParams.agcInfo.isRequest = 0;
    pSeq->commonParams.waitTime = 0;

    pSeq->commonParams.progressInfo.progress = 0;
    result = setProgress (pSeq, 0, 10, 0);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    pSeq->commonParams.ckalFreqKHz = 64000; /* CKAL */
    pSeq->commonParams.ckahFreqKHz = 96000; /* CKAH */

    /* Initialize the data storage */
    result = sony_demod_dvbs_s2_blindscan_StorageInit (&(pSeq->commonParams.storage),
                                                       pSeq->powerArray,
                                                       sizeof(pSeq->powerArray)/sizeof(pSeq->powerArray[0]),
                                                       pSeq->dataArray,
                                                       sizeof(pSeq->dataArray)/sizeof(pSeq->dataArray[0]));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    /* Initialize demodulator to start BlindScan. */
    result = sony_demod_dvbs_s2_blindscan_Initialize(pSeq->commonParams.pDemod);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_subseq_bt_Initialize (&pSeq->subseqBT, &pSeq->commonParams);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_subseq_pm_Initialize (&pSeq->subseqPM, &pSeq->commonParams);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_subseq_ss_Initialize (&pSeq->subseqSS, &pSeq->commonParams);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_subseq_fs_Initialize (&pSeq->subseqFS, &pSeq->subseqBT, &pSeq->commonParams);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_subseq_cs_Initialize (&pSeq->subseqCS, &pSeq->subseqPM, &pSeq->commonParams);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData(&(pSeq->commonParams.storage), &(pSeq->pCandList1));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData(&(pSeq->commonParams.storage), &(pSeq->pCandList2));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData(&(pSeq->commonParams.storage), &(pSeq->pDetectedList));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData(&(pSeq->commonParams.storage), &(pSeq->pBandList));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_blindscan_seq_Sequence (sony_demod_dvbs_s2_blindscan_seq_t * pSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_seq_Sequence");

    if(!pSeq) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pSeq->subseqSS.isEnable){
        /* SS seq */
        result = sony_demod_dvbs_s2_blindscan_subseq_ss_Sequence (&pSeq->subseqSS);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN(result);
        }
        {
            uint32_t current = pSeq->commonParams.tuneReq.frequencyKHz;
            uint32_t min = pSeq->minPowerFreqKHz;
            uint32_t max = pSeq->maxPowerFreqKHz;
            if (pSeq->commonParams.tuneReq.isRequest){
                if (min >= max){
                    pSeq->commonParams.progressInfo.minorProgress = 100;
                } else {
                    pSeq->commonParams.progressInfo.minorProgress = (uint8_t)(((current - min) * 100) / (max - min));
                }
            }
        }
    } else if (pSeq->subseqFS.isEnable){
        if (pSeq->subseqBT.isEnable){
            /* BT seq */
            result = sony_demod_dvbs_s2_blindscan_subseq_bt_Sequence (&pSeq->subseqBT);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }
        } else {
            /* FS seq */
            result = sony_demod_dvbs_s2_blindscan_subseq_fs_Sequence (&pSeq->subseqFS);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }
        }
    } else if (pSeq->subseqCS.isEnable){
        if (pSeq->subseqPM.isEnable){
            /* PM seq */
            result = sony_demod_dvbs_s2_blindscan_subseq_pm_Sequence(&pSeq->subseqPM);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }
        } else {
            /* CS seq */
            result = sony_demod_dvbs_s2_blindscan_subseq_cs_Sequence(&pSeq->subseqCS);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }
        }
    } else {
        /* Main sequence */
        switch(pSeq->seqState)
        {
        case BLINDSCAN_SEQ_STATE_START:
            if (pSeq->maxSymbolRateKSps >= 20000){
                /* === Stage 1 === */
                /* Get power spectrum (1st) */
                result = sony_demod_dvbs_s2_blindscan_subseq_ss_Start(&pSeq->subseqSS,
                                                                      pSeq->minPowerFreqKHz,
                                                                      pSeq->maxPowerFreqKHz,
                                                                      2000,
                                                                      20000,
                                                                      pSeq->commonParams.pDemod->dvbss2PowerSmooth);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }
                pSeq->commonParams.waitTime = 0;
                pSeq->seqState = BLINDSCAN_SEQ_STATE_SS1_FIN;
            } else {
                /* Skip stage 1 */
                pSeq->commonParams.waitTime = 0;
                pSeq->seqState = BLINDSCAN_SEQ_STATE_STAGE1_FIN;
            }
            break;

        case BLINDSCAN_SEQ_STATE_SS1_FIN:
            /* Get candidate (MP) */
            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_POWERLIST("2MHz", pSeq->subseqSS.pPowerList);
            result = sony_demod_dvbs_s2_blindscan_algo_GetCandidateMp (&(pSeq->commonParams.storage),
                                                                       pSeq->subseqSS.pPowerList,
                                                                       100,     /* slice :  1.00dB  */
                                                                       20000,   /* min   : 20  MSps */
                                                                       45000,   /* max   : 45  MSps */
                                                                       4000,    /* cferr :  4  MHz  */
                                                                       pSeq->pCandList1);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }

            result = sony_demod_dvbs_s2_blindscan_algo_SortBySymbolrate (&(pSeq->commonParams.storage), pSeq->pCandList1, 20000);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }

            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CANDLIST("fs1", pSeq->pCandList1);

            /* Start fine search */
            result = setProgress (pSeq, 10, 25, 0);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }

            result = sony_demod_dvbs_s2_blindscan_subseq_fs_Start(&pSeq->subseqFS, pSeq->pCandList1, 19900, 45000, pSeq->pDetectedList);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }

            pSeq->commonParams.waitTime = 0;
            pSeq->seqState = BLINDSCAN_SEQ_STATE_STAGE1_FIN;
            break;

        case BLINDSCAN_SEQ_STATE_STAGE1_FIN:
            if (pSeq->minSymbolRateKSps < 20000){
                /* Stage 2 or 3 */
                result = sony_demod_dvbs_s2_blindscan_ClearDataList (&(pSeq->commonParams.storage), pSeq->pCandList1);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }
                result = sony_demod_dvbs_s2_blindscan_ClearDataList (&(pSeq->commonParams.storage), pSeq->pCandList2);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }
                result = sony_demod_dvbs_s2_blindscan_algo_GetNonDetectedBand (&(pSeq->commonParams.storage),
                                                                               pSeq->minPowerFreqKHz,
                                                                               pSeq->maxPowerFreqKHz,
                                                                               500,
                                                                               pSeq->pDetectedList,
                                                                               pSeq->pBandList);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }
                pSeq->pBandCurrent = pSeq->pBandList->pNext;

                pSeq->commonParams.waitTime = 0;
                pSeq->seqState = BLINDSCAN_SEQ_STATE_SS2_START;

                result = setProgress (pSeq, 25, 35, 0);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }
            } else {
                /* Finish scan sequence */
                pSeq->commonParams.waitTime = 0;
                pSeq->seqState = BLINDSCAN_SEQ_STATE_FINISH;
            }
            break;

        case BLINDSCAN_SEQ_STATE_SS2_START:

            if (pSeq->pBandCurrent){
                result = sony_demod_dvbs_s2_blindscan_ClearPowerList (&(pSeq->commonParams.storage), pSeq->subseqSS.pPowerList);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }
                result = sony_demod_dvbs_s2_blindscan_subseq_ss_Start(&pSeq->subseqSS,
                                                                      pSeq->pBandCurrent->data.band.minFreqKHz,
                                                                      pSeq->pBandCurrent->data.band.maxFreqKHz,
                                                                      500,
                                                                      20000,
                                                                      pSeq->commonParams.pDemod->dvbss2PowerSmooth);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }
                pSeq->commonParams.waitTime = 0;
                pSeq->seqState = BLINDSCAN_SEQ_STATE_SS2_FIN;
            } else {
                /* Go to next */
                pSeq->commonParams.waitTime = 0;
                pSeq->seqState = BLINDSCAN_SEQ_STATE_FS2_START;
            }
            break;

        case BLINDSCAN_SEQ_STATE_SS2_FIN:
            {
                sony_demod_dvbs_s2_blindscan_data_t * pList;
                sony_demod_dvbs_s2_blindscan_data_t * pLast1;
                sony_demod_dvbs_s2_blindscan_data_t * pLast2;

                result = sony_demod_dvbs_s2_blindscan_AllocData (&pSeq->commonParams.storage, &pList);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }

                SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_POWERLIST("500KHz", pSeq->subseqSS.pPowerList);

                /* === For stage 2 === */
                result = sony_demod_dvbs_s2_blindscan_algo_GetCandidateNml (&(pSeq->commonParams.storage),
                                                                            pSeq->subseqSS.pPowerList,
                                                                            100,    /* slice :  1.00dB  */
                                                                            5000,   /* min   :  5  MSps */
                                                                            20000,  /* max   : 20  MSps */
                                                                            1000,   /* cferr :  1  MHz  */
                                                                            pList);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }

                result = sony_demod_dvbs_s2_blindscan_algo_ReduceCandidate (&(pSeq->commonParams.storage), pList, pSeq->pDetectedList);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }

                result = sony_demod_dvbs_s2_blindscan_algo_DeleteDuplicate (&(pSeq->commonParams.storage), pList);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }

                pLast1 = pSeq->pCandList1;
                while(pLast1->pNext){
                    pLast1 = pLast1->pNext;
                }
                pLast1->pNext = pList->pNext;
                pList->pNext = NULL;

                if (pSeq->minSymbolRateKSps <= 5000){
                    /* === For stage 3 === */
                    result = sony_demod_dvbs_s2_blindscan_algo_GetCandidateNml (&(pSeq->commonParams.storage),
                                                                                pSeq->subseqSS.pPowerList,
                                                                                200,    /* slice :  2.00dB  */
                                                                                1000,   /* min   :  1  MSps */
                                                                                5000,   /* max   :  5  MSps */
                                                                                1000,   /* cferr :  1  MHz  */
                                                                                pList);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN(result);
                    }

                    result = sony_demod_dvbs_s2_blindscan_algo_ReduceCandidate (&(pSeq->commonParams.storage), pList, pSeq->pDetectedList);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN(result);
                    }

                    result = sony_demod_dvbs_s2_blindscan_algo_DeleteDuplicate (&(pSeq->commonParams.storage), pList);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN(result);
                    }

                    /* to candlist2 */
                    pLast2 = pSeq->pCandList2;
                    while(pLast2->pNext){
                        pLast2 = pLast2->pNext;
                    }
                    pLast2->pNext = pList->pNext;
                    pList->pNext = NULL;
                }

                result = sony_demod_dvbs_s2_blindscan_FreeData (&pSeq->commonParams.storage, pList);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }

                pSeq->pBandCurrent = pSeq->pBandCurrent->pNext;
                pSeq->commonParams.waitTime = 0;
                pSeq->seqState = BLINDSCAN_SEQ_STATE_SS2_START;
            }
            break;

        case BLINDSCAN_SEQ_STATE_FS2_START:

            result = sony_demod_dvbs_s2_blindscan_algo_SortBySymbolrate (&(pSeq->commonParams.storage), pSeq->pCandList1, 20000);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }

            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CANDLIST("fs2", pSeq->pCandList1);

            result = setProgress (pSeq, 35, 55, 0);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }

            result = sony_demod_dvbs_s2_blindscan_subseq_fs_Start (&pSeq->subseqFS, pSeq->pCandList1, 4975, 20000, pSeq->pDetectedList);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }

            pSeq->commonParams.waitTime = 0;
            pSeq->seqState = BLINDSCAN_SEQ_STATE_FS2_FIN;
            break;


        case BLINDSCAN_SEQ_STATE_FS2_FIN:
            {
                sony_demod_dvbs_s2_blindscan_data_t * pCurrent = NULL;
                result = sony_demod_dvbs_s2_blindscan_ClearDataList (&(pSeq->commonParams.storage), pSeq->pCandList1);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }
                pSeq->pCandLast = pSeq->pCandList1;

                result = sony_demod_dvbs_s2_blindscan_algo_SeparateCandidate (&(pSeq->commonParams.storage), pSeq->pCandList2);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }

                result = sony_demod_dvbs_s2_blindscan_algo_DeleteDuplicate2 (&(pSeq->commonParams.storage), pSeq->pCandList2);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }

                result = sony_demod_dvbs_s2_blindscan_algo_ReduceCandidate (&(pSeq->commonParams.storage), pSeq->pCandList2, pSeq->pDetectedList);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }

                pSeq->pCandCurrent = pSeq->pCandList2->pNext;
                pSeq->seqState = BLINDSCAN_SEQ_STATE_CS_PREPARING;
                pSeq->commonParams.waitTime = 0;
                result = setProgress (pSeq, 55, 75, 0);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }
                /* For calculate progress */
                pCurrent = pSeq->pCandList2->pNext;
                pSeq->candCount = 1;
                pSeq->candIndex = 1;
                while(pCurrent){
                    pSeq->candCount++;
                    pCurrent = pCurrent->pNext;
                }
            }
            break;

        case BLINDSCAN_SEQ_STATE_CS_PREPARING:
            if (pSeq->candIndex > pSeq->candCount){
                pSeq->candIndex = pSeq->candCount;
            }
            pSeq->commonParams.progressInfo.minorProgress = (uint8_t)((pSeq->candIndex * 100) / pSeq->candCount);
            if (pSeq->pCandCurrent){
                uint8_t dflg = 0;
                uint32_t detFreqKHz = 0;
                uint32_t detSRKSps = 0;
                uint32_t candFreqKHz = pSeq->pCandCurrent->data.candidate.centerFreqKHz;
                sony_demod_dvbs_s2_blindscan_data_t * pCurrent = pSeq->pDetectedList->pNext;
                while(pCurrent){
                    detFreqKHz = pCurrent->data.channelInfo.centerFreqKHz;
                    detSRKSps = pCurrent->data.channelInfo.symbolRateKSps;
                    if (((detFreqKHz - (detSRKSps/2)) <= candFreqKHz) && (candFreqKHz <= (detFreqKHz + (detSRKSps/2)))){
                        dflg = 1;
                        break;
                    }
                    pCurrent = pCurrent->pNext;
                }
                if (dflg == 0){
                    pSeq->commonParams.tuneReq.isRequest = 1;
                    pSeq->commonParams.tuneReq.frequencyKHz = ((candFreqKHz + 500) / 1000) * 1000;
                    pSeq->commonParams.tuneReq.symbolRateKSps = 5000;
                    pSeq->commonParams.tuneReq.system = SONY_DTV_SYSTEM_DVBS;
                    pSeq->seqState = BLINDSCAN_SEQ_STATE_CS_TUNED;
                    pSeq->commonParams.waitTime = 0;
                } else {
                    /* Go to next candidate */
                    pSeq->candIndex++;
                    pSeq->pCandCurrent = pSeq->pCandCurrent->pNext;
                    pSeq->seqState = BLINDSCAN_SEQ_STATE_CS_PREPARING;
                    pSeq->commonParams.waitTime = 0;
                }
            } else {
                /* Go to next step */
                pSeq->seqState = BLINDSCAN_SEQ_STATE_FS3_START;
                pSeq->commonParams.waitTime = 0;
            }
            break;

        case BLINDSCAN_SEQ_STATE_CS_TUNED:
            {
                SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CS_TUNE(pSeq->commonParams.tuneReq.frequencyKHz);
                result = sony_demod_dvbs_s2_blindscan_subseq_cs_Start (&(pSeq->subseqCS), 0);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }
                pSeq->seqState = BLINDSCAN_SEQ_STATE_CS_FIN;
            }
            break;

        case BLINDSCAN_SEQ_STATE_CS_FIN:
            if (pSeq->subseqCS.isExist){
                sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
                result = sony_demod_dvbs_s2_blindscan_AllocData (&(pSeq->commonParams.storage), &pTemp);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }
                pTemp->data.candidate.centerFreqKHz = pSeq->commonParams.tuneReq.frequencyKHz;
                pTemp->data.candidate.symbolRateKSps = pSeq->subseqCS.coarseSymbolRateKSps;
                pTemp->data.candidate.minSymbolRateKSps = (pSeq->subseqCS.coarseSymbolRateKSps * 700) / 1000;
                pTemp->data.candidate.maxSymbolRateKSps = ((pSeq->subseqCS.coarseSymbolRateKSps * 1600) + 999) / 1000;
                pSeq->pCandLast->pNext = pTemp;
                pSeq->pCandLast = pSeq->pCandLast->pNext;
            }
            /* Go to next candidate */
            pSeq->candIndex++;
            pSeq->pCandCurrent = pSeq->pCandCurrent->pNext;
            pSeq->seqState = BLINDSCAN_SEQ_STATE_CS_PREPARING;
            pSeq->commonParams.waitTime = 0;
            break;

        case BLINDSCAN_SEQ_STATE_FS3_START:
            result = sony_demod_dvbs_s2_blindscan_algo_DeleteDuplicate (&(pSeq->commonParams.storage), pSeq->pCandList1);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }

            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CANDLIST("fs3", pSeq->pCandList1);

            result = setProgress (pSeq, 75, 100, 0);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }

            result = sony_demod_dvbs_s2_blindscan_subseq_fs_Start(&pSeq->subseqFS, pSeq->pCandList1, 1000, 5000, pSeq->pDetectedList);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }

            pSeq->seqState = BLINDSCAN_SEQ_STATE_FINISH;
            pSeq->commonParams.waitTime = 0;
            break;

        case BLINDSCAN_SEQ_STATE_FINISH:
            /* Finish */
            pSeq->isContinue = 0;
            pSeq->commonParams.waitTime = 0;
            result = setProgress (pSeq, 100, 100, 100);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
    }

    SONY_TRACE_RETURN (result);
}

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t setProgress (sony_demod_dvbs_s2_blindscan_seq_t * pSeq,
                                  uint8_t minProgressRange,
                                  uint8_t maxProgressRange,
                                  uint8_t minorProgress)
{
    SONY_TRACE_ENTER ("setProgress");

    if(!pSeq) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->commonParams.progressInfo.majorMinProgress = minProgressRange;
    pSeq->commonParams.progressInfo.majorMaxProgress = maxProgressRange;
    pSeq->commonParams.progressInfo.minorProgress = minorProgress;
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
