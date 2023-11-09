/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_demod_dvbs_s2_monitor.h"
#include "sony_demod_dvbs_s2_blindscan.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_seq.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_fs.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_bt.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t finish_ok (sony_demod_dvbs_s2_blindscan_subseq_fs_t * pSeq);
static sony_result_t updateProgress (sony_demod_dvbs_s2_blindscan_subseq_fs_t * pSeq);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t sony_demod_dvbs_s2_blindscan_subseq_fs_Initialize (sony_demod_dvbs_s2_blindscan_subseq_fs_t * pSeq,
                                                                 sony_demod_dvbs_s2_blindscan_subseq_bt_t * pSeqBT,
                                                                 sony_demod_dvbs_s2_blindscan_seq_common_t * pCommonParams)
{
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_subseq_fs_Initialize");

    if ((!pSeq) || (!pSeqBT) || (!pCommonParams)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isEnable = 0;
    pSeq->state = FS_STATE_START;
    pSeq->pSeqBT = pSeqBT;
    pSeq->pCommonParams = pCommonParams;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_blindscan_subseq_fs_Start (sony_demod_dvbs_s2_blindscan_subseq_fs_t * pSeq,
                                                            sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                                            uint32_t minSymbolRateKSps,
                                                            uint32_t maxSymbolRateKSps,
                                                            sony_demod_dvbs_s2_blindscan_data_t * pChannelList)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrent = NULL;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_subseq_fs_Start");

    if ((!pSeq) || (!pCandList) || (!pChannelList)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isEnable = 1;
    pSeq->state = FS_STATE_START;
    pSeq->pCandList = pCandList;
    pSeq->pCandCurrent = pSeq->pCandList->pNext;
    pSeq->pChannelList = pChannelList;
    pSeq->minSymbolRateKSps = minSymbolRateKSps;
    pSeq->maxSymbolRateKSps = maxSymbolRateKSps;

    pSeq->candIndex = 1;
    pSeq->candCount = 1;
    pCurrent = pCandList->pNext;
    while(pCurrent){
        pSeq->candCount++;
        pCurrent = pCurrent->pNext;
    }
    result = updateProgress (pSeq);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    pSeq->pChannelLast = pChannelList;
    while(pSeq->pChannelLast->pNext){
        pSeq->pChannelLast = pSeq->pChannelLast->pNext;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_blindscan_subseq_fs_Sequence (sony_demod_dvbs_s2_blindscan_subseq_fs_t * pSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pChannelCurrent = NULL;
    uint32_t candFreq = 0;
    uint32_t candSR = 0;
    uint32_t candMinSR = 0;
    uint32_t candMaxSR = 0;
    uint32_t channelFreq = 0;
    uint32_t channelSR = 0;
    uint8_t isTry = 0;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_subseq_fs_Sequence");

    if(!pSeq) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(pSeq->state)
    {
    case FS_STATE_START:
        result = updateProgress (pSeq);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        if (pSeq->pCandCurrent){
            /* Cand is exist */
            candFreq = pSeq->pCandCurrent->data.candidate.centerFreqKHz;
            candSR = pSeq->pCandCurrent->data.candidate.symbolRateKSps;
            candMinSR = pSeq->pCandCurrent->data.candidate.minSymbolRateKSps;
            candMaxSR = pSeq->pCandCurrent->data.candidate.maxSymbolRateKSps;

            isTry = 1;
            if (candMaxSR > pSeq->minSymbolRateKSps){
                pChannelCurrent = pSeq->pChannelList->pNext;
                while(pChannelCurrent){
                    channelFreq = pChannelCurrent->data.candidate.centerFreqKHz;
                    channelSR = pChannelCurrent->data.candidate.symbolRateKSps;

                    if (((channelFreq - (channelSR/2)) <= candFreq) && (candFreq <= (channelFreq + (channelSR/2)))){
                        /* duplicated */
                        isTry = 0;
                        break;
                    }
                    /* Next channel */
                    pChannelCurrent = pChannelCurrent->pNext;
                }
            } else {
                isTry = 0;
            }

            pSeq->candIndex++;
            pSeq->pCandCurrent = pSeq->pCandCurrent->pNext;

            if ((isTry) && (candMaxSR > 1000)) {
                result = sony_demod_dvbs_s2_blindscan_subseq_bt_Start (pSeq->pSeqBT,
                                                                       candFreq,
                                                                       candSR,
                                                                       candMinSR,
                                                                       candMaxSR,
                                                                       pSeq->minSymbolRateKSps,
                                                                       pSeq->maxSymbolRateKSps);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                pSeq->state = FS_STATE_WAIT_BTFIN;
                pSeq->pCommonParams->waitTime =0;
            }
        } else {
            /* Finish */
            result = finish_ok(pSeq);
        }
        break;

    case FS_STATE_WAIT_BTFIN:
        /* Check result */
        if (pSeq->pSeqBT->isLocked){
            uint8_t isExist = 0;
            sony_demod_dvbs_s2_blindscan_data_t * pCurrent = pSeq->pChannelList->pNext;
            uint32_t detFrequencyKHz = (uint32_t)((int32_t)(pSeq->pCommonParams->tuneReq.frequencyKHz) + pSeq->pSeqBT->detCarrierOffsetKHz);
            uint32_t detSymbolRateKSps = (pSeq->pSeqBT->detSymbolRateSps + 500) / 1000;
            uint32_t chFrequencyKHz = 0;
            uint32_t chSymbolRateKSps = 0;

            isExist = 0;
            while(pCurrent){

                chFrequencyKHz = pCurrent->data.channelInfo.centerFreqKHz;
                chSymbolRateKSps = pCurrent->data.channelInfo.symbolRateKSps/2;

                if ((detFrequencyKHz >= (chFrequencyKHz - chSymbolRateKSps)) &&
                    (detFrequencyKHz <= (chFrequencyKHz + chSymbolRateKSps))){
                    /* It is already detected channel */
                    isExist = 1;
                    break;
                }
                pCurrent = pCurrent->pNext;
            }

            if (isExist == 0){

                /* Detect signal */
                pSeq->pCommonParams->detInfo.isDetect = 1;
                pSeq->pCommonParams->detInfo.centerFreqKHz = detFrequencyKHz;
                pSeq->pCommonParams->detInfo.symbolRateKSps = detSymbolRateKSps;
                pSeq->pCommonParams->detInfo.system = pSeq->pSeqBT->detSystem;

                result = sony_demod_dvbs_s2_blindscan_AllocData (&(pSeq->pCommonParams->storage), &pTemp);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }

                pTemp->data.channelInfo.centerFreqKHz = pSeq->pCommonParams->detInfo.centerFreqKHz;
                pTemp->data.channelInfo.symbolRateKSps = pSeq->pCommonParams->detInfo.symbolRateKSps;
                pTemp->data.channelInfo.system = pSeq->pCommonParams->detInfo.system;

                pSeq->pChannelLast->pNext = pTemp;
                pSeq->pChannelLast = pSeq->pChannelLast->pNext;
            }
        } else {
            /* Unlock */
            pSeq->pCommonParams->detInfo.isDetect = 0;
        }
        pSeq->state = FS_STATE_START;
        pSeq->pCommonParams->waitTime = 0;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t finish_ok (sony_demod_dvbs_s2_blindscan_subseq_fs_t * pSeq)
{
    SONY_TRACE_ENTER ("finish_ok");
    if(!pSeq) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    pSeq->isEnable = 0;
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t updateProgress (sony_demod_dvbs_s2_blindscan_subseq_fs_t * pSeq)
{
    SONY_TRACE_ENTER ("updateProgress");
    if(!pSeq) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    if (pSeq->candIndex > pSeq->candCount){
        pSeq->candIndex = pSeq->candCount;
    }
    pSeq->pCommonParams->progressInfo.minorProgress = (uint8_t)((pSeq->candIndex * 100) / pSeq->candCount);
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
