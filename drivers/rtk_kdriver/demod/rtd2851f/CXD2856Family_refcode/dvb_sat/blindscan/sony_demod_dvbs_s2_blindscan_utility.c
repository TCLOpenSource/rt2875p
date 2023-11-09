/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t incrementMemCount (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage);
static sony_result_t decrementMemCount (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage);
static sony_result_t incrementPowCount (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage);
static sony_result_t decrementPowCount (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t sony_demod_dvbs_s2_blindscan_StorageInit (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                        sony_demod_dvbs_s2_blindscan_power_t * pPowerArrayTop,
                                                        int32_t powerArrayLength,
                                                        sony_demod_dvbs_s2_blindscan_data_t * pDataArrayTop,
                                                        int32_t dataArrayLength)
{
    int32_t i = 0;
    sony_demod_dvbs_s2_blindscan_power_t * pPowerCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pDataCurrent = NULL;

    if ((!pStorage) || (!pPowerArrayTop) || (powerArrayLength <= 0) || (!pDataArrayTop) || (dataArrayLength <= 0)){
        return SONY_RESULT_ERROR_ARG;
    }

    pStorage->pPowerArrayTop = pPowerArrayTop;
    pStorage->powerArrayLength = powerArrayLength;
    pStorage->pDataArrayTop = pDataArrayTop;
    pStorage->dataArrayLength = dataArrayLength;

    pPowerCurrent = &(pStorage->availablePowerList);
    for(i = 0; i < powerArrayLength; i++){
        pPowerCurrent->pNext = (pPowerArrayTop + i);
        pPowerCurrent = pPowerCurrent->pNext;
    }
    pPowerCurrent->pNext = NULL;
    pStorage->currentUsedPowerCount = 0;
    pStorage->maxUsedPowerCount = 0;

    pDataCurrent = &(pStorage->availableDataList);
    for(i = 0; i < dataArrayLength; i++){
        pDataCurrent->pNext = (pDataArrayTop + i);
        pDataCurrent = pDataCurrent->pNext;
    }
    pDataCurrent->pNext = NULL;
    pStorage->currentUsedCount = 0;
    pStorage->maxUsedCount = 0;

    return SONY_RESULT_OK;
}

sony_result_t sony_demod_dvbs_s2_blindscan_AllocPower (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                       sony_demod_dvbs_s2_blindscan_power_t ** ppPower)
{
    if((!pStorage) || (!ppPower)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if(pStorage->availablePowerList.pNext != NULL){
        *ppPower = pStorage->availablePowerList.pNext;
        if((*ppPower)->pNext != NULL){
            pStorage->availablePowerList.pNext = (*ppPower)->pNext;
        } else {
            /* Alloc the last data. */
            pStorage->availablePowerList.pNext = NULL;
        }
    } else {
        /* Overflow. */
        return SONY_RESULT_ERROR_OVERFLOW;
    }
    (*ppPower)->pNext = NULL;
    return incrementPowCount (pStorage);
}

sony_result_t sony_demod_dvbs_s2_blindscan_FreePower (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                      sony_demod_dvbs_s2_blindscan_power_t * pPower)
{
    sony_demod_dvbs_s2_blindscan_power_t * pTemp = NULL;

    if((!pStorage) || (!pPower)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if(pStorage->availablePowerList.pNext != NULL){
        pTemp = pStorage->availablePowerList.pNext;
        pStorage->availablePowerList.pNext = pPower;
        pPower->pNext = pTemp;
    } else {
        /* Data is empty. */
        pStorage->availablePowerList.pNext = pPower;
        pPower->pNext = NULL;
    }
    return decrementPowCount (pStorage);
}

sony_result_t sony_demod_dvbs_s2_blindscan_ClearPowerList (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                           sony_demod_dvbs_s2_blindscan_power_t * pListTop)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_power_t * pCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_power_t * pTemp = NULL;

    if ((!pStorage) || (!pListTop)){
        return SONY_RESULT_ERROR_ARG;
    }

    pCurrent = pListTop->pNext;

    while (pCurrent){
        pTemp = pCurrent->pNext;
        result = sony_demod_dvbs_s2_blindscan_FreePower (pStorage, pCurrent);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        pCurrent = pTemp;
    }

    pListTop->pNext = NULL;

    return SONY_RESULT_OK;
}

sony_result_t sony_demod_dvbs_s2_blindscan_AllocData (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                      sony_demod_dvbs_s2_blindscan_data_t ** ppData)
{
    if ((!pStorage) || (!ppData)){
        return SONY_RESULT_ERROR_ARG;
    }

    if(pStorage->availableDataList.pNext){
        *ppData = pStorage->availableDataList.pNext;
        if((*ppData)->pNext){
            pStorage->availableDataList.pNext = (*ppData)->pNext;
        } else {
            /* Alloc the last data. */
            pStorage->availableDataList.pNext = NULL;
        }
    } else {
        /* Overflow. */
        return SONY_RESULT_ERROR_OVERFLOW;
    }
    (*ppData)->pNext = NULL;
    return incrementMemCount(pStorage);
}

sony_result_t sony_demod_dvbs_s2_blindscan_FreeData (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                     sony_demod_dvbs_s2_blindscan_data_t * pData)
{
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;

    if ((!pStorage) || (!pData)){
        return SONY_RESULT_ERROR_OTHER;
    }

    if(pStorage->availableDataList.pNext){
        pTemp = pStorage->availableDataList.pNext;
        pStorage->availableDataList.pNext = pData;
        pData->pNext = pTemp;
    } else {
        /* Data is empty. */
        pStorage->availableDataList.pNext = pData;
        pData->pNext = NULL;
    }
    return decrementMemCount(pStorage);
}

sony_result_t sony_demod_dvbs_s2_blindscan_ClearDataList (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                          sony_demod_dvbs_s2_blindscan_data_t * pListTop)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_data_t * pCurrent = NULL;
    sony_demod_dvbs_s2_blindscan_data_t * pTemp = NULL;

    if ((!pStorage) || (!pListTop)){
        return SONY_RESULT_ERROR_ARG;
    }

    pCurrent = pListTop->pNext;

    while (pCurrent){
        pTemp = pCurrent->pNext;
        result = sony_demod_dvbs_s2_blindscan_FreeData (pStorage, pCurrent);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        pCurrent = pTemp;
    }

    pListTop->pNext = NULL;

    return SONY_RESULT_OK;
}

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t incrementMemCount (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage)
{
    if (!pStorage){
        return SONY_RESULT_ERROR_ARG;
    }
    pStorage->currentUsedCount++;
    if(pStorage->currentUsedCount <= 0){
        /* Error(Overflow) */
        return SONY_RESULT_ERROR_OVERFLOW;
    }
    if(pStorage->maxUsedCount < pStorage->currentUsedCount){
        pStorage->maxUsedCount = pStorage->currentUsedCount;
    }
    return SONY_RESULT_OK;
}

static sony_result_t decrementMemCount (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage)
{
    if (!pStorage){
        return SONY_RESULT_ERROR_ARG;
    }
    if(pStorage->currentUsedCount == 0){
        /* Error(Overflow) */
        return SONY_RESULT_ERROR_OVERFLOW;
    } else {
        pStorage->currentUsedCount--;
    }
    return SONY_RESULT_OK;
}

static sony_result_t incrementPowCount (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage)
{
    if (!pStorage){
        return SONY_RESULT_ERROR_ARG;
    }
    pStorage->currentUsedPowerCount++;
    if(pStorage->currentUsedPowerCount <= 0){
        /* Error(Overflow) */
        return SONY_RESULT_ERROR_OVERFLOW;
    }
    if(pStorage->maxUsedPowerCount < pStorage->currentUsedPowerCount){
        pStorage->maxUsedPowerCount = pStorage->currentUsedPowerCount;
    }
    return SONY_RESULT_OK;
}

static sony_result_t decrementPowCount (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage)
{
    if (!pStorage){
        return SONY_RESULT_ERROR_ARG;
    }
    if(pStorage->currentUsedPowerCount == 0){
        /* Error(Overflow) */
        return SONY_RESULT_ERROR_OVERFLOW;
    } else {
        pStorage->currentUsedPowerCount--;
    }
    return SONY_RESULT_OK;
}
