/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_blindscan_utility.h

          This file provides the functions for alloc/free data.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_BLINDSCAN_UTILITY_H
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_UTILITY_H

#include "sony_common.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_seq.h"

/**
 @brief Initialize storage instance.

 @param pStorage Storage instance.
 @param pPowerArrayTop Pointer of array for power data.
 @param powerArrayLength Length of array for power data "pPowerArrayTop".
 @param pDataArrayTop Pointer of array for data.
 @param dataArrayLength Length of array for data "pDataArrayTop".

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_StorageInit (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                        sony_demod_dvbs_s2_blindscan_power_t * pPowerArrayTop,
                                                        int32_t powerArrayLength,
                                                        sony_demod_dvbs_s2_blindscan_data_t * pDataArrayTop,
                                                        int32_t dataArrayLength);

/**
 @brief Allocate power data.

 @param pStorage Storage instance.
 @param ppPower Address of address for power data.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_AllocPower (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                       sony_demod_dvbs_s2_blindscan_power_t ** ppPower);

/**
 @brief Free power data.

 @param pStorage Storage instance.
 @param pPower Address for power data.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_FreePower (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                      sony_demod_dvbs_s2_blindscan_power_t * pPower);

/**
 @brief Clear power list.

 @param pStorage Storage instance.
 @param pListTop Address for power list.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_ClearPowerList (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                           sony_demod_dvbs_s2_blindscan_power_t * pListTop);

/**
 @brief Allocate data.

 @param pStorage Storage instance.
 @param ppData Address of address for data.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_AllocData (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                      sony_demod_dvbs_s2_blindscan_data_t ** ppData);

/**
 @brief Free data.

 @param pStorage Storage instance.
 @param pData Address for data.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_FreeData (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                     sony_demod_dvbs_s2_blindscan_data_t * pData);

/**
 @brief Clear data list.

 @param pStorage Storage instance.
 @param pListTop Address for data list.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_ClearDataList (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                          sony_demod_dvbs_s2_blindscan_data_t * pListTop);

#endif /* SONY_DEMOD_DVBS_S2_BLINDSCAN_UTILITY_H */
