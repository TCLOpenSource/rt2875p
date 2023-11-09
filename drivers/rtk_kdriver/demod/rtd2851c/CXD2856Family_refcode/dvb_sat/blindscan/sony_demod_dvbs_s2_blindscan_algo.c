/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_utility.h"
#include "sony_demod_dvbs_s2_blindscan_seq.h"
#include "sony_demod_dvbs_s2_blindscan_trace.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t moving_average (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                     sony_demod_dvbs_s2_blindscan_power_t * pPowerList,
                                     int32_t maLength);

static sony_result_t wagiri (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                             sony_demod_dvbs_s2_blindscan_power_t * pPowerList,
                             uint32_t clipStep,
                             uint32_t minSymbolRateKSps,
                             uint32_t maxSymbolRateKSps,
                             uint32_t cferr,
                             uint8_t isMpMode,
                             sony_demod_dvbs_s2_blindscan_data_t * pCandList);

static sony_result_t delete_duplicated_area (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                             sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                             uint32_t cferr);

static sony_result_t get_power_range (sony_demod_dvbs_s2_blindscan_power_t * pPowerList,
                                      int32_t * pPowerMin,
                                      int32_t * pPowerMax);

static sony_result_t select_candidate (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                       sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                       uint32_t minSymbolRateKSps,
                                       uint32_t maxSymbolRateKSps,
                                       uint8_t isMpMode);

static sony_result_t pick_duplicated_area (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                           sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                           uint32_t cferr);

static sony_result_t limit_range (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                  sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                  uint32_t minSymbolRateKSps,
                                  uint32_t maxSymbolRateKSps);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t sony_demod_dvbs_s2_blindscan_algo_GetCandidateMp (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                sony_demod_dvbs_s2_blindscan_power_t * pPowerList,
                                                                uint32_t clipStep,
                                                                uint32_t minSymbolRateKSps,
                                                                uint32_t maxSymbolRateKSps,
                                                                uint32_t cferr,
                                                                sony_demod_dvbs_s2_blindscan_data_t * pCandList)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_algo_GetCandidateMp");

    if ((!pStorage) || (!pPowerList) || (!pCandList)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* wagiri mp */
    result = wagiri (pStorage,
                     pPowerList,
                     clipStep,
                     (minSymbolRateKSps * 65) / 100,
                     ((maxSymbolRateKSps * 135) + 99) / 100,
                     cferr,
                     1,
                     pCandList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = moving_average (pStorage, pPowerList, 250);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    /* wagiri mp */
    result = wagiri (pStorage,
                     pPowerList,
                     clipStep,
                     (minSymbolRateKSps * 65) / 100,
                     ((maxSymbolRateKSps * 135) + 99) / 100,
                     cferr,
                     1,
                     pCandList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = delete_duplicated_area (pStorage, pCandList, cferr);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CANDLIST ("GetCandMp", pCandList);

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_blindscan_algo_GetCandidateNml (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                 sony_demod_dvbs_s2_blindscan_power_t * pPowerList,
                                                                 uint32_t clipStep,
                                                                 uint32_t minSymbolRateKSps,
                                                                 uint32_t maxSymbolRateKSps,
                                                                 uint32_t cferr,
                                                                 sony_demod_dvbs_s2_blindscan_data_t * pCandList)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_demod_dvbs_s2_blindscan_algo_GetCandidateNml");

    /* wagiri nml */
    result = wagiri (pStorage,
                     pPowerList,
                     clipStep,
                     (minSymbolRateKSps * 65) / 100,
                     ((maxSymbolRateKSps * 135) + 99) / 100,
                     cferr,
                     0,
                     pCandList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = delete_duplicated_area (pStorage, pCandList, cferr);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = limit_range (pStorage,
                          pCandList,
                          (minSymbolRateKSps * 65) / 100,
                          ((maxSymbolRateKSps * 135) + 99) / 100);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_CANDLIST ("GetCandNml", pCandList);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_blindscan_algo_SeparateCandidate (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                   sony_demod_dvbs_s2_blindscan_data_t * pCandList)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_data_t * pCandCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pList = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pLast = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
    int32_t candSymbolRate = 0;
    int32_t candFrequency = 0;
    int32_t symbolRate = 0;
    int32_t frequency = 0;
    SONY_TRACE_ENTER("sony_demod_dvbs_s2_blindscan_algo_SeparateCandidate");

    if ((!pStorage) || (!pCandList)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }
    pLast = pList;

    pCandCurrent = pCandList->pNext;
    while(pCandCurrent){
        candFrequency = (int32_t)(pCandCurrent->data.candidate.centerFreqKHz);
        candSymbolRate = (int32_t)(pCandCurrent->data.candidate.maxSymbolRateKSps / 2);

        for (symbolRate = (candSymbolRate * (-1)); symbolRate <= candSymbolRate; symbolRate += 1000){
            frequency = candFrequency + symbolRate;

            result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }

            pTemp->data.candidate.centerFreqKHz = (uint32_t)frequency;
            pTemp->data.candidate.symbolRateKSps = 0;
            pTemp->data.candidate.minSymbolRateKSps = 0;
            pTemp->data.candidate.maxSymbolRateKSps = 0;

            pLast->pNext = pTemp;
            pLast = pLast->pNext;
        }

        pCandCurrent = pCandCurrent->pNext;
    }

    result = sony_demod_dvbs_s2_blindscan_ClearDataList (pStorage, pCandList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pCandList->pNext = pList->pNext;
    pList->pNext = NULL;

    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_blindscan_algo_ReduceCandidate (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                 sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                                                 sony_demod_dvbs_s2_blindscan_data_t * pChannelList)
{
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pCandCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pChannelCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pList = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pLast = NULL;
    sony_result_t result = SONY_RESULT_OK;
    uint8_t dflg = 0;
    uint32_t candFreq = 0;
    uint32_t candSR = 0;
    uint32_t candMinSR = 0;
    uint32_t candMaxSR = 0;
    uint32_t channelFreq = 0;
    uint32_t channelSR = 0;

    SONY_TRACE_ENTER("sony_demod_dvbs_s2_blindscan_algo_ReduceCandidate");

    if ((!pCandList) || (!pChannelList)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }
    pLast = pList;

    pCandCurrent = pCandList->pNext;
    while (pCandCurrent){
        dflg = 0;
        candFreq = pCandCurrent->data.candidate.centerFreqKHz;
        candSR = pCandCurrent->data.candidate.symbolRateKSps;
        candMinSR = pCandCurrent->data.candidate.minSymbolRateKSps;
        candMaxSR = pCandCurrent->data.candidate.maxSymbolRateKSps;
        pChannelCurrent = pChannelList->pNext;
        while (pChannelCurrent){
            channelFreq = pChannelCurrent->data.channelInfo.centerFreqKHz;
            channelSR = pChannelCurrent->data.channelInfo.symbolRateKSps;
            if (((channelFreq - (channelSR / 2)) < candFreq) && (candFreq < (channelFreq + (channelSR / 2)))){
                dflg = 1;
                break;
            }
            /* Next */
            pChannelCurrent = pChannelCurrent->pNext;
        }
        if (dflg == 0){
            result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }
            pTemp->data.candidate.centerFreqKHz = candFreq;
            pTemp->data.candidate.symbolRateKSps = candSR;
            pTemp->data.candidate.minSymbolRateKSps = candMinSR;
            pTemp->data.candidate.maxSymbolRateKSps = candMaxSR;
            pLast->pNext = pTemp;
            pLast = pLast->pNext;
        }
        /* Next */
        pCandCurrent = pCandCurrent->pNext;
    }

    /* Clear CandList */
    result = sony_demod_dvbs_s2_blindscan_ClearDataList (pStorage, pCandList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pCandList->pNext = pList->pNext;
    pList->pNext = NULL;

    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_blindscan_algo_DeleteDuplicate (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                 sony_demod_dvbs_s2_blindscan_data_t * pCandList)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrent2 = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pList = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pLast = NULL;
    uint8_t dflg = 0;
    uint32_t freqA = 0;
    uint32_t srA = 0;
    uint32_t freqB = 0;
    uint32_t srB = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_algo_DeleteDuplicate");

    if ((!pStorage) || (!pCandList)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pCurrent = pCandList->pNext;

    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }
    pLast = pList;

    while(pCurrent){
        pCurrent2 = pCurrent->pNext;
        dflg = 0;
        freqA = pCurrent->data.candidate.centerFreqKHz;
        srA = pCurrent->data.candidate.symbolRateKSps;
        while(pCurrent2){
            freqB = pCurrent2->data.candidate.centerFreqKHz;
            srB = pCurrent2->data.candidate.symbolRateKSps;
            if((freqA == freqB) && (srA == srB)){
                dflg = 1;
                break;
            }
            /* Go to next */
            pCurrent2 = pCurrent2->pNext;
        }
        if(dflg == 0){
            result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }
            pTemp->data.candidate.centerFreqKHz = pCurrent->data.candidate.centerFreqKHz;
            pTemp->data.candidate.symbolRateKSps = pCurrent->data.candidate.symbolRateKSps;
            pTemp->data.candidate.minSymbolRateKSps = pCurrent->data.candidate.minSymbolRateKSps;
            pTemp->data.candidate.maxSymbolRateKSps = pCurrent->data.candidate.maxSymbolRateKSps;
            pLast->pNext = pTemp;
            pLast = pLast->pNext;
        }
        /* Go to next */
        pCurrent = pCurrent->pNext;
    }

    result = sony_demod_dvbs_s2_blindscan_ClearDataList (pStorage, pCandList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pCandList->pNext = pList->pNext;

    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_blindscan_algo_DeleteDuplicate2 (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                  sony_demod_dvbs_s2_blindscan_data_t * pCandList)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_data_t * pList;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp;
    sony_demod_dvbs_s2_blindscan_data_t * pLast;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrent;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrent2;
    uint8_t dflg = 0;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_algo_DeleteDuplicate2");

    if ((!pStorage) || (!pCandList)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }
    pLast = pList;

    pCurrent = pCandList->pNext;
    while(pCurrent){
        dflg = 0;
        pCurrent2 = pList->pNext;
        while(pCurrent2){
            if (pCurrent->data.candidate.centerFreqKHz == pCurrent2->data.candidate.centerFreqKHz){
                dflg = 1;
                break;
            }
            pCurrent2 = pCurrent2->pNext;
        }
        if (dflg == 0){
            result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }
            pTemp->data.candidate.centerFreqKHz = pCurrent->data.candidate.centerFreqKHz;
            pTemp->data.candidate.symbolRateKSps = pCurrent->data.candidate.symbolRateKSps;
            pTemp->data.candidate.minSymbolRateKSps = pCurrent->data.candidate.minSymbolRateKSps;
            pTemp->data.candidate.maxSymbolRateKSps = pCurrent->data.candidate.maxSymbolRateKSps;
            pLast->pNext = pTemp;
            pLast = pLast->pNext;
        }

        pCurrent = pCurrent->pNext;
    }

    result = sony_demod_dvbs_s2_blindscan_ClearDataList (pStorage, pCandList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pCandList->pNext = pList->pNext;
    pList->pNext = NULL;

    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_blindscan_algo_GetNonDetectedBand (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                    uint32_t minPowerFreqKHz,
                                                                    uint32_t maxPowerFreqKHz,
                                                                    uint32_t resolutionKHz,
                                                                    sony_demod_dvbs_s2_blindscan_data_t * pChannelList,
                                                                    sony_demod_dvbs_s2_blindscan_data_t * pBandList)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pBandLast = NULL;
    uint32_t freqKHz = 0;
    uint32_t cf = 0;
    uint32_t sr = 0;
    uint8_t dflg = 0;
    uint32_t minFreqKHz = 0;
    uint32_t maxFreqKHz = 0;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_blindscan_algo_GetNonDetectedBand");

    if ((!pStorage) || (!pChannelList) || (!pBandList)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pBandLast = pBandList;

    minFreqKHz = minPowerFreqKHz;
    maxFreqKHz = 0;
    for (freqKHz = minPowerFreqKHz; freqKHz <= maxPowerFreqKHz; freqKHz += resolutionKHz){
        pCurrent = pChannelList->pNext;
        dflg = 0;
        while(pCurrent){
            cf = pCurrent->data.channelInfo.centerFreqKHz;
            sr = pCurrent->data.channelInfo.symbolRateKSps;
            if (((cf - (sr/2)) <= freqKHz) && (freqKHz <= (cf + (sr/2)))){
                dflg = 1;
                break;
            }
            /* Go to next */
            pCurrent = pCurrent->pNext;
        }
        if (dflg == 1){
            if ((minFreqKHz > 0) && (maxFreqKHz > 0)){
                result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN(result);
                }
                pTemp->data.band.minFreqKHz = minFreqKHz;
                pTemp->data.band.maxFreqKHz = maxFreqKHz;
                pBandLast->pNext = pTemp;
                pBandLast = pBandLast->pNext;
            }
            minFreqKHz = 0;
            maxFreqKHz = 0;
        } else {
            if (minFreqKHz == 0){
                minFreqKHz = freqKHz;
            } else {
                maxFreqKHz = freqKHz;
            }
        }
    }
    if ((minFreqKHz > 0) && (maxFreqKHz > 0)){
        result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN(result);
        }
        pTemp->data.band.minFreqKHz = minFreqKHz;
        pTemp->data.band.maxFreqKHz = maxFreqKHz;
        pBandLast->pNext = pTemp;
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_blindscan_algo_SortBySymbolrate (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                  sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                                                  uint32_t baseSymbolRateKSps)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t isContinue = 0;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pList = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pListBase = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pListCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pListNext = NULL;
    uint32_t current;
    uint32_t next;
    uint32_t target;

    SONY_TRACE_ENTER("sony_demod_dvbs_s2_blindscan_algo_SortBySymbolrate");

    if (!pCandList) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    while (pCandList->pNext){
        /* Get 1st data to "Temp" from CandList. */
        pTemp = pCandList->pNext;
        pCandList->pNext = pTemp->pNext;
        pTemp->pNext = NULL;
        if (pTemp->data.candidate.symbolRateKSps > baseSymbolRateKSps){
            target = pTemp->data.candidate.symbolRateKSps - baseSymbolRateKSps;
        } else {
            target = baseSymbolRateKSps - pTemp->data.candidate.symbolRateKSps;
        }

        isContinue = 1;
        pListBase = pList;
        while (isContinue){
            if (pListBase->pNext){
                /* Get current */
                pListCurrent = pListBase->pNext;
                if (pListCurrent->data.candidate.symbolRateKSps > baseSymbolRateKSps){
                    current = pListCurrent->data.candidate.symbolRateKSps - baseSymbolRateKSps;
                } else {
                    current = baseSymbolRateKSps - pListCurrent->data.candidate.symbolRateKSps;
                }

                if (pListCurrent->pNext){
                    /* Get next */
                    pListNext = pListCurrent->pNext;
                    if (pListNext->data.candidate.symbolRateKSps > baseSymbolRateKSps){
                        next = pListNext->data.candidate.symbolRateKSps - baseSymbolRateKSps;
                    } else {
                        next = baseSymbolRateKSps - pListNext->data.candidate.symbolRateKSps;
                    }

                    if (target < current){
                        /*
                         *      Target
                         *        |
                         * [Base]---[Cur]---[Next]---
                         */
                        pTemp->pNext = pListCurrent;
                        pListBase->pNext = pTemp;
                        isContinue = 0;
                    } else if (target < next){
                        /*
                         *              Target
                         *                |
                         * [Base]---[Cur]---[Next]---
                         */
                        pTemp->pNext = pListNext;
                        pListCurrent->pNext = pTemp;
                        isContinue = 0;
                    } else {
                        /* Continue */
                    }
                } else {
                    if(target < current){
                        /*
                         *      Target
                         *        |
                         * [Base]---[Cur]---(null)
                         */
                        pTemp->pNext = pListCurrent;
                        pListBase->pNext = pTemp;
                        isContinue = 0;
                    } else {
                        /*
                         *              Target
                         *                |
                         * [Base]---[Cur]---(null)
                         */
                        pListCurrent->pNext = pTemp;
                        isContinue = 0;
                    }
                }
                /* Update [Base] */
                pListBase = pListBase->pNext;
            } else {
                /* List is empty */
                pListBase->pNext = pTemp;
                isContinue = 0;
            }
        }
    }

    pCandList->pNext = pList->pNext;
    pList->pNext = NULL;

    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_blindscan_algo_SortByFrequency (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                 sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                                                 uint32_t baseFrequencyKHz)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t isContinue = 0;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pList = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pListBase = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pListCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pListNext = NULL;
    uint32_t current;
    uint32_t next;
    uint32_t target;

    SONY_TRACE_ENTER("sony_demod_dvbs_s2_blindscan_algo_SortByFrequency");

    if(!pCandList) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    while (pCandList->pNext){
        /* Get 1st data to "Temp" from CandList. */
        pTemp = pCandList->pNext;
        pCandList->pNext = pTemp->pNext;
        pTemp->pNext = NULL;
        if (pTemp->data.candidate.centerFreqKHz > baseFrequencyKHz){
            target = pTemp->data.candidate.centerFreqKHz - baseFrequencyKHz;
        } else {
            target = baseFrequencyKHz - pTemp->data.candidate.centerFreqKHz;
        }

        isContinue = 1;
        pListBase = pList;
        while (isContinue){
            if (pListBase->pNext){
                /* Get current */
                pListCurrent = pListBase->pNext;
                if (pListCurrent->data.candidate.centerFreqKHz > baseFrequencyKHz){
                    current = pListCurrent->data.candidate.centerFreqKHz - baseFrequencyKHz;
                } else {
                    current = baseFrequencyKHz - pListCurrent->data.candidate.centerFreqKHz;
                }

                if (pListCurrent->pNext){
                    /* Get next */
                    pListNext = pListCurrent->pNext;
                    if (pListNext->data.candidate.centerFreqKHz > baseFrequencyKHz){
                        next = pListNext->data.candidate.centerFreqKHz - baseFrequencyKHz;
                    } else {
                        next = baseFrequencyKHz - pListNext->data.candidate.centerFreqKHz;
                    }

                    if (target < current){
                        /*
                         *      Target
                         *        |
                         * [Base]---[Cur]---[Next]---
                         */
                        pTemp->pNext = pListCurrent;
                        pListBase->pNext = pTemp;
                        isContinue = 0;
                    } else if (target < next){
                        /*
                         *              Target
                         *                |
                         * [Base]---[Cur]---[Next]---
                         */
                        pTemp->pNext = pListNext;
                        pListCurrent->pNext = pTemp;
                        isContinue = 0;
                    } else {
                        /* Continue */
                    }
                } else {
                    if(target < current){
                        /*
                         *      Target
                         *        |
                         * [Base]---[Cur]---(null)
                         */
                        pTemp->pNext = pListCurrent;
                        pListBase->pNext = pTemp;
                        isContinue = 0;
                    } else {
                        /*
                         *              Target
                         *                |
                         * [Base]---[Cur]---(null)
                         */
                        pListCurrent->pNext = pTemp;
                        isContinue = 0;
                    }
                }
                /* Update [Base] */
                pListBase = pListBase->pNext;
            } else {
                /* List is empty */
                pListBase->pNext = pTemp;
                isContinue = 0;
            }
        }
    }

    pCandList->pNext = pList->pNext;
    pList->pNext = NULL;

    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t moving_average (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                     sony_demod_dvbs_s2_blindscan_power_t * pPowerList,
                                     int32_t maLength)
{
    sony_demod_dvbs_s2_blindscan_power_t * pCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
    sony_demod_dvbs_s2_blindscan_power_t * pMaLast = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pBufferList = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pLast = NULL;

    int32_t powerTemp = 0;
    int32_t power = 0;
    int32_t sum = 0;
    int32_t index = 0;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("moving_average");

    if((!pStorage) || (!pPowerList) || (maLength == 0)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pBufferList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pLast = pBufferList;

    /* Get lower edge data */
    if(pPowerList->pNext){
        power = pPowerList->pNext->power;
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    sum = 0;
    /* Buffer for lower half */
    for(index = 0; index < (maLength/2); index++){
        result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN(result);
        }
        pTemp->data.power.power = power;
        pLast->pNext = pTemp;
        pLast = pLast->pNext;
        sum += power;
    }

    /*
     *            <---maLength/2---->
     * Buffer (*)--[ ]--[ ]--[ ]--[ ]...
     *
     * Power  (*)--[x]--[ ]--[ ]--[ ]...
     */
    pCurrent = pPowerList->pNext;
    pMaLast = pPowerList->pNext;
    for (index = 0; index < (maLength/2); index++){
        if (pMaLast){
            power = pMaLast->power;
            if (pMaLast->pNext){
                pMaLast = pMaLast->pNext;
            }
        }
        sum += power;
    }

    while (pCurrent){
        /* Store power */
        powerTemp = pCurrent->power;
        /* Update data */
        pCurrent->power = sum / maLength;

        /* === Prepare for next data. === */
        sum -= pBufferList->pNext->data.power.power;

        /* Remove 1st data from BufferList.
         *            <---maLength/2---->
         * Buffer (*)--[ ]--[ ]--[ ]--[ ]...
         *              |
         *              Remove
         */
        pTemp = pBufferList->pNext;
        pBufferList->pNext = pTemp->pNext;
        result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pTemp);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        if(pMaLast)
        {
            power = pMaLast->power;
            if(pMaLast->pNext){
                pMaLast = pMaLast->pNext;
            }
        }

        /* Add data to last of BufferList.
         *            <---maLength/2---->
         * Buffer (*)--[ ]--[ ]--[ ]--[ ]...
         *                                 |
         *                                 Add
         */
        result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        pTemp->data.power.power = powerTemp;
        pLast->pNext = pTemp;
        pLast = pLast->pNext;
        sum += power;

        pCurrent = pCurrent->pNext;
    }

    result = sony_demod_dvbs_s2_blindscan_ClearDataList (pStorage, pBufferList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pBufferList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t wagiri (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                             sony_demod_dvbs_s2_blindscan_power_t * pPowerList,
                             uint32_t clipStep,
                             uint32_t minSymbolRateKSps,
                             uint32_t maxSymbolRateKSps,
                             uint32_t cferr,
                             uint8_t isMpMode,
                             sony_demod_dvbs_s2_blindscan_data_t * pCandList)
{
    sony_result_t result = SONY_RESULT_OK;
    int32_t level = 0;
    int32_t powerMin = 0;
    int32_t powerMax = 0;
    uint8_t state = 0;
    uint32_t startFreqKHz = 0;
    uint32_t stopFreqKHz = 0;
    uint32_t ctFrequencyKHz = 0;
    uint32_t ctSymbolRateKSps = 0;
    sony_demod_dvbs_s2_blindscan_power_t * pCurrentPower = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pLastCand = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTempList = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTempCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTempData = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTempList2 = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTempCurrent2 = NULL;

    SONY_TRACE_ENTER ("wagiri");

    if((!pStorage) || (!pPowerList)|| (!pCandList)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if(!pPowerList->pNext){
        /* Power data is empty */
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    /* Get last data */
    pLastCand = pCandList;
    while(pLastCand->pNext){
        pLastCand = pLastCand->pNext;
    }

    result = get_power_range (pPowerList, &powerMin, &powerMax);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTempList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTempList2);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pTempCurrent2 = pTempList2;

    for (level = powerMax; level >= powerMin; level -= (int32_t)clipStep){
        /* Initialize state */
        state = 2;
        pTempCurrent = pTempList;

        /* Set 1st data. */
        pCurrentPower = pPowerList->pNext;
        while(pCurrentPower){

            if(pCurrentPower->freqKHz == 0){
                break;
            }

            if(pCurrentPower->power > level){
                switch(state)
                {
                case 0:
                    /* start edge. */
                    startFreqKHz = pCurrentPower->freqKHz;
                    state = 1;
                    break;

                case 1:
                    /* do nothing. */
                    break;

                case 2:
                    /*
                     * do nothing.
                     * wait for "power <= clipLevel"
                     */
                    break;

                default:
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
                }
            } else {
                switch(state)
                {
                case 0:
                    /* do nothing. */
                    break;

                case 1:
                    /* Stop edge. */
                    stopFreqKHz = pCurrentPower->freqKHz;
                    state = 0;
                    ctFrequencyKHz = ((stopFreqKHz + startFreqKHz + 1000) / 2000) * 1000;
                    ctSymbolRateKSps = stopFreqKHz - startFreqKHz;

                    /* Alloc data */
                    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTempData);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN(result);
                    }

                    pTempData->data.candidate.centerFreqKHz = ctFrequencyKHz;
                    pTempData->data.candidate.symbolRateKSps = ctSymbolRateKSps;
                    pTempData->data.candidate.minSymbolRateKSps = 0;
                    pTempData->data.candidate.maxSymbolRateKSps = 0;

                    /* Add data */
                    pTempCurrent->pNext = pTempData;
                    pTempCurrent = pTempCurrent->pNext;
                    break;

                case 2:
                    /* Entry point. */
                    state = 0;
                    break;

                default:
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
                }
            }

            /* Go to next data. */
            pCurrentPower = pCurrentPower->pNext;
        }

        /* Get candidate from once wagiri result. */
        result = select_candidate (pStorage, pTempList, minSymbolRateKSps, maxSymbolRateKSps, isMpMode);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN(result);
        }

        /* Add TempList data to TempList2 */
        while (pTempCurrent2->pNext){
            pTempCurrent2 = pTempCurrent2->pNext;
        }
        pTempCurrent2->pNext = pTempList->pNext;
        pTempList->pNext = NULL;
    }

    result = pick_duplicated_area (pStorage, pTempList2, cferr);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pLastCand->pNext = pTempList2->pNext;
    pTempList2->pNext = NULL;

    result = sony_demod_dvbs_s2_blindscan_ClearDataList (pStorage, pTempList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }
    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pTempList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_ClearDataList (pStorage, pTempList2);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }
    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pTempList2);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t delete_duplicated_area (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                             sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                             uint32_t cferr)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrentA = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrentB = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pList = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pListCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
    uint32_t a_cf = 0;
    uint32_t a_min = 0;
    uint32_t a_max = 0;
    uint32_t b_cf = 0;
    uint32_t b_min = 0;
    uint32_t b_max = 0;
    int32_t ms = 0;
    SONY_TRACE_ENTER("delete_duplicated_area");

    if(!pCandList) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pListCurrent = pList;

    pCurrentA = pCandList->pNext;

    while (pCurrentA){
        a_cf = pCurrentA->data.candidate.centerFreqKHz;
        a_min = pCurrentA->data.candidate.minSymbolRateKSps;
        a_max = pCurrentA->data.candidate.maxSymbolRateKSps;
        if (a_cf != 0){
            pCurrentB = pCurrentA->pNext;
            while (pCurrentB){
                b_cf = pCurrentB->data.candidate.centerFreqKHz;
                b_min = pCurrentB->data.candidate.minSymbolRateKSps;
                b_max = pCurrentB->data.candidate.maxSymbolRateKSps;
                if (a_min < b_min){
                    ms = (int32_t)((a_min + 3) / 4) - (int32_t)cferr;
                } else {
                    ms = (int32_t)((b_min + 3) / 4) - (int32_t)cferr;
                }
                if (ms < 0){
                    ms = 0;
                }
                if (((a_cf >= b_cf) && ((a_cf - b_cf) <= (uint32_t)ms)) || ((b_cf >= a_cf) && ((b_cf - a_cf) <= (uint32_t)ms))){
                    if ((b_min <= a_max) && (a_max <= b_max)){
                        a_max = b_max;
                        pCurrentB->data.candidate.centerFreqKHz = 0;
                    } else if ((b_min < a_min) && (a_min < b_max)){
                        a_min = b_min;
                        pCurrentB->data.candidate.centerFreqKHz = 0;
                    } else {
                        /* Do nothing */
                    }
                }
                pCurrentB = pCurrentB->pNext;
            }
            result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }
            pTemp->data.candidate.centerFreqKHz = a_cf;
            pTemp->data.candidate.symbolRateKSps = (a_min + a_max) / 2;
            pTemp->data.candidate.minSymbolRateKSps = a_min;
            pTemp->data.candidate.maxSymbolRateKSps = a_max;
            pListCurrent->pNext = pTemp;
            pListCurrent = pListCurrent->pNext;
        }
        pCurrentA = pCurrentA->pNext;
    }

    result = sony_demod_dvbs_s2_blindscan_ClearDataList (pStorage, pCandList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pCandList->pNext = pList->pNext;

    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t get_power_range (sony_demod_dvbs_s2_blindscan_power_t * pPowerList,
                                      int32_t * pPowerMin,
                                      int32_t * pPowerMax)
{
    sony_demod_dvbs_s2_blindscan_power_t * pCurrent = NULL;
    int32_t minValue = 0;
    int32_t maxValue = 0;

    SONY_TRACE_ENTER("get_power_range");

    if((!pPowerList) || (!pPowerList->pNext) || (!pPowerMin) || (!pPowerMax)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pCurrent = pPowerList->pNext;
    minValue = pCurrent->power;
    maxValue = pCurrent->power;

    while(pCurrent->pNext){
        pCurrent = pCurrent->pNext;
        if (pCurrent->freqKHz > 0){
            if (minValue > pCurrent->power){
                minValue = pCurrent->power;
            }
            if (maxValue < pCurrent->power){
                maxValue = pCurrent->power;
            }
        }
    }

    *pPowerMin = minValue;
    *pPowerMax = maxValue;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t select_candidate (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                       sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                       uint32_t minSymbolRateKSps,
                                       uint32_t maxSymbolRateKSps,
                                       uint8_t isMpMode)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrent1 = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrent2 = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pOutList = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pOutLast = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
    uint32_t freq1 = 0;
    uint32_t sr1 = 0;
    uint32_t freq2 = 0;
    uint32_t sr2 = 0;
    uint32_t mpFrequencyKHz = 0;
    uint32_t mpSymbolRateKSps = 0;
    uint32_t space = 0;

    SONY_TRACE_ENTER("select_candidate");

    if ((!pStorage) || (!pCandList)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Alloc */
    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pOutList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pOutLast = pOutList;

    if(isMpMode){
        pCurrent1 = pCandList->pNext;
        while(pCurrent1){
            /* Get freq1 and sr1 */
            freq1 = pCurrent1->data.candidate.centerFreqKHz;
            sr1 = pCurrent1->data.candidate.symbolRateKSps;

            pCurrent2 = pCurrent1->pNext;
            while(pCurrent2){
                /* Get freq2 and sr2 */
                freq2 = pCurrent2->data.candidate.centerFreqKHz;
                sr2 = pCurrent2->data.candidate.symbolRateKSps;

                mpFrequencyKHz   = ((freq2 + ( sr2      / 2)) + (freq1 - (sr1 / 2))) / 2;
                mpSymbolRateKSps =  (freq2 + ( sr2      / 2)) - (freq1 - (sr1 / 2));
                space            =  (freq2 - ((sr2 + 1) / 2)) - (freq1 + (sr1 / 2));

                if((space < ((mpSymbolRateKSps + 1)/2)) && (minSymbolRateKSps <= mpSymbolRateKSps) && (mpSymbolRateKSps <= maxSymbolRateKSps)){
                    /* Add candidate */
                    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN(result);
                    }
                    pTemp->data.candidate.centerFreqKHz = mpFrequencyKHz;
                    pTemp->data.candidate.symbolRateKSps = mpSymbolRateKSps;
                    pTemp->data.candidate.minSymbolRateKSps = 0;
                    pTemp->data.candidate.maxSymbolRateKSps = 0;
                    pOutLast->pNext = pTemp;
                    pOutLast = pOutLast->pNext;
                } else {
                    break;
                }

                /* Go to next */
                pCurrent2 = pCurrent2->pNext;
            }

            /* Go to next */
            pCurrent1 = pCurrent1->pNext;
        }
    }

    pCurrent1 = pCandList->pNext;
    while(pCurrent1){
        freq1 = pCurrent1->data.candidate.centerFreqKHz;
        sr1 = pCurrent1->data.candidate.symbolRateKSps;

        if ((minSymbolRateKSps <= sr1) && (sr1 <= maxSymbolRateKSps)){
            /* Add candidate */
            result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }
            pTemp->data.candidate.centerFreqKHz = freq1;
            pTemp->data.candidate.symbolRateKSps = sr1;
            pTemp->data.candidate.minSymbolRateKSps = 0;
            pTemp->data.candidate.maxSymbolRateKSps = 0;
            pOutLast->pNext = pTemp;
            pOutLast = pOutLast->pNext;
        }

        /* Go to next */
        pCurrent1 = pCurrent1->pNext;
    }

    result = sony_demod_dvbs_s2_blindscan_ClearDataList (pStorage, pCandList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pCandList->pNext = pOutList->pNext;

    /* Free */
    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pOutList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t pick_duplicated_area (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                           sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                           uint32_t cferr)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrent1 = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrent2 = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pInList = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pOutList = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pOutCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
    uint32_t freq1 = 0;
    uint32_t sr1 = 0;
    uint32_t freq2 = 0;
    uint32_t sr2 = 0;
    uint32_t freqUpper = 0;
    uint32_t freqLower = 0;
    uint32_t srUpper = 0;
    uint32_t srLower = 0;
    uint32_t minSymbolRateKSps = 0;
    uint32_t maxSymbolRateKSps = 0;

    SONY_TRACE_ENTER("pick_duplicated_area");

    if ((!pStorage) || (!pCandList)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Alloc */
    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pInList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }
    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pOutList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pInList->pNext = pCandList->pNext;
    pOutCurrent = pOutList;

    pCurrent1 = pCandList->pNext;
    while(pCurrent1){
        freq1 = pCurrent1->data.candidate.centerFreqKHz;
        sr1 = pCurrent1->data.candidate.symbolRateKSps;

        pCurrent2 = pCurrent1->pNext;
        while(pCurrent2){
            freq2 = pCurrent2->data.candidate.centerFreqKHz;
            sr2 = pCurrent2->data.candidate.symbolRateKSps;

            freqUpper = (freq1 > freq2) ? freq1 : freq2;
            freqLower = (freq1 > freq2) ? freq2 : freq1;
            srUpper = (sr1 > sr2) ? sr1 : sr2;
            srLower = (sr1 > sr2) ? sr2 : sr1;
            if ((freqUpper - freqLower) <= cferr){
                minSymbolRateKSps =  (srUpper *  7)      / 10;
                maxSymbolRateKSps = ((srLower * 16) + 9) / 10;
                if (minSymbolRateKSps < maxSymbolRateKSps){
                    /* Add candidate */
                    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN(result);
                    }
                    pTemp->data.candidate.centerFreqKHz = ((((freq1 + freq2) / 2) + 500) / 1000) * 1000;
                    pTemp->data.candidate.symbolRateKSps = 0;
                    pTemp->data.candidate.minSymbolRateKSps = minSymbolRateKSps;
                    pTemp->data.candidate.maxSymbolRateKSps = maxSymbolRateKSps;
                    pOutCurrent->pNext = pTemp;
                    pOutCurrent = pOutCurrent->pNext;
                }
            }

            /* Go to next */
            pCurrent2 = pCurrent2->pNext;
        }

        /* Go to next */
        pCurrent1 = pCurrent1->pNext;
    }

    result = sony_demod_dvbs_s2_blindscan_ClearDataList (pStorage, pInList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pCandList->pNext = pOutList->pNext;

    /* Free */
    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pInList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }
    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pOutList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t limit_range (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                  sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                  uint32_t minSymbolRateKSps,
                                  uint32_t maxSymbolRateKSps)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pList = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pLast = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;
    uint32_t lim_min = 0;
    uint32_t lim_max = 0;
    uint32_t center = 0;

    SONY_TRACE_ENTER("limit_range");

    if(!pCandList) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pLast = pList;

    pCurrent = pCandList->pNext;
    while (pCurrent){
        /* lim_min */
        if (pCurrent->data.candidate.minSymbolRateKSps < minSymbolRateKSps){
            lim_min = minSymbolRateKSps;
        } else {
            lim_min = pCurrent->data.candidate.minSymbolRateKSps;
        }
        /* lim_max */
        if (pCurrent->data.candidate.maxSymbolRateKSps > maxSymbolRateKSps){
            lim_max = maxSymbolRateKSps;
        } else {
            lim_max = pCurrent->data.candidate.maxSymbolRateKSps;
        }
        if ((lim_min < 30000) && (20000 < lim_max)){
            if ((20000 < lim_min) && (lim_max < 30000)){
                center = (lim_min + lim_max) / 2;
            } else if ((lim_min <= 20000) && (lim_max < 30000)){
                center = (20000 + lim_max) / 2;
            } else if ((20000 < lim_min) && (30000 <= lim_max)){
                center = (30000 + lim_min) / 2;
            } else {
                center = 25000;
            }
        } else {
            center = (lim_min + lim_max) / 2;
        }

        result = sony_demod_dvbs_s2_blindscan_AllocData (pStorage, &pTemp);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN(result);
        }

        pTemp->data.candidate.centerFreqKHz = pCurrent->data.candidate.centerFreqKHz;
        pTemp->data.candidate.symbolRateKSps = center;
        pTemp->data.candidate.minSymbolRateKSps = lim_min;
        pTemp->data.candidate.maxSymbolRateKSps = lim_max;

        pLast->pNext = pTemp;
        pLast = pLast->pNext;

        /* Next data */
        pCurrent = pCurrent->pNext;
    }

    result = sony_demod_dvbs_s2_blindscan_ClearDataList (pStorage, pCandList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    pCandList->pNext = pList->pNext;
    pList->pNext = NULL;

    result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pList);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}
