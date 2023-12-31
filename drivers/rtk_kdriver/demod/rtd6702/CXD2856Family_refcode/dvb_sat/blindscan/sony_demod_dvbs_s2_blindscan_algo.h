/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_blindscan_algo.h

          This file provides the algorithms for BlindScan/TuneSRS function.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_BLINDSCAN_ALGO_H
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_ALGO_H

#include "sony_common.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
/**
 @brief Get candidate from power spectrum for multipath.

 @param pStorage Storage instance.
 @param pPowerList Power spectrum data.
 @param clipStep Slicing step in dB x 100.
 @param minSymbolRateKSps Min symbol rate in KSps.
 @param maxSymbolRateKSps Max symbol rate in KSps.
 @param cferr Error frequency in KHz.
 @param pCandList Candidate list generated by this function.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_algo_GetCandidateMp (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                sony_demod_dvbs_s2_blindscan_power_t * pPowerList,
                                                                uint32_t clipStep,
                                                                uint32_t minSymbolRateKSps,
                                                                uint32_t maxSymbolRateKSps,
                                                                uint32_t cferr,
                                                                sony_demod_dvbs_s2_blindscan_data_t * pCandList);

/**
 @brief Get candidate from power spectrum.

 @param pStorage Storage instance.
 @param pPowerList Power spectrum data.
 @param clipStep Slicing step in dB x 100.
 @param minSymbolRateKSps Min symbol rate in KSps.
 @param maxSymbolRateKSps Max symbol rate in KSps.
 @param cferr Error frequency in KHz.
 @param pCandList Candidate list generated by this function.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_algo_GetCandidateNml (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                 sony_demod_dvbs_s2_blindscan_power_t * pPowerList,
                                                                 uint32_t clipStep,
                                                                 uint32_t minSymbolRateKSps,
                                                                 uint32_t maxSymbolRateKSps,
                                                                 uint32_t cferr,
                                                                 sony_demod_dvbs_s2_blindscan_data_t * pCandList);

/**
 @brief Separate candidate for low symbol rate search

        pCandList is updated by this function.

 @param pStorage Storage instance.
 @param pCandList Candidate list.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_algo_SeparateCandidate (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                   sony_demod_dvbs_s2_blindscan_data_t * pCandList);

/**
 @brief Reduce candidate which is out of range.

        pCandList is updated by this function.

 @param pStorage Storage instance.
 @param pCandList Candidate list.
 @param pChannelList Detected channel list.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_algo_ReduceCandidate (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                 sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                                                 sony_demod_dvbs_s2_blindscan_data_t * pChannelList);

/**
 @brief Delete duplicated candidate

        pCandList is updated by this function.

 @param pStorage Storage instance.
 @param pCandList Candidate list.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_algo_DeleteDuplicate (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                 sony_demod_dvbs_s2_blindscan_data_t * pCandList);

/**
 @brief Delete duplicated candidate

        pCandList is updated by this function.

 @param pStorage Storage instance.
 @param pCandList Candidate list.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_algo_DeleteDuplicate2 (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                  sony_demod_dvbs_s2_blindscan_data_t * pCandList);

/**
 @brief Get non detected area from detected channel list.

 @param pStorage Storage instance.
 @param minPowerFreqKHz Min frequency in KHz.
 @param maxPowerFreqKHz Max frequency in KHz.
 @param resolutionKHz Resolution in KHz.
 @param pChannelList Detected channel list.
 @param pBandList Not detected area list.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_algo_GetNonDetectedBand (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                    uint32_t minPowerFreqKHz,
                                                                    uint32_t maxPowerFreqKHz,
                                                                    uint32_t resolutionKHz,
                                                                    sony_demod_dvbs_s2_blindscan_data_t * pChannelList,
                                                                    sony_demod_dvbs_s2_blindscan_data_t * pBandList);

/**
 @brief Sort candidate list by symbol rate.

 @param pStorage Storage instance.
 @param pCandList Candidate list.
 @param baseSymbolRateKSps Sort base symbol rate in KSps.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_algo_SortBySymbolrate (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                  sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                                                  uint32_t baseSymbolRateKSps);

/**
 @brief Sort candidate list by frequency.

 @param pStorage Storage instance.
 @param pCandList Candidate list.
 @param baseFrequencyKHz Sort base frequency in KHz.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_algo_SortByFrequency (sony_demod_dvbs_s2_blindscan_data_storage_t * pStorage,
                                                                 sony_demod_dvbs_s2_blindscan_data_t * pCandList,
                                                                 uint32_t baseFrequencyKHz);

#endif /* SONY_DEMOD_DVBS_S2_BLINDSCAN_ALGO_H */
