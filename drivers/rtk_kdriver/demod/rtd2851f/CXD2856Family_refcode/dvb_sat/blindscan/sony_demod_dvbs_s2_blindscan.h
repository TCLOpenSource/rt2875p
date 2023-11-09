/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_blindscan.h

          This file provides functions to control demodulator for BlindScan/TuneSRS.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_BLINDSCAN_H
#define SONY_DEMOD_DVBS_S2_BLINDSCAN_H

#include "sony_common.h"
#include "sony_demod.h"

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Initialize setting for starting BlindScan/TuneSRS

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_Initialize(sony_demod_t * pDemod);

/**
 @brief Initialize setting for power search.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.
 @param ocfr_csk The register value of OCFR_CSK (1 - 7).
                 In normal case, it should be 1.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_PS_INIT (sony_demod_t * pDemod, uint8_t ocfr_csk);

/**
 @brief Set parameters for power search.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.
 @param lowerOffsetKHz lower offset frequency in KHz.
 @param upperOffsetKHz upper offset frequency in KHz.
 @param stepKHz step frequency in KHz.
 @param ckaFreqKHz CKA frequency in KHz.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_PS_SET (sony_demod_t * pDemod,
                                                   int32_t lowerOffsetKHz,
                                                   int32_t upperOffsetKHz,
                                                   uint32_t stepKHz,
                                                   uint32_t ckaFreqKHz);

/**
 @brief Start power search.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_PS_START (sony_demod_t * pDemod);

/**
 @brief Set read ack.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_PS_RACK (sony_demod_t * pDemod);

/**
 @brief Finalize settings for power search.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_PS_FIN (sony_demod_t * pDemod);

/**
 @brief Initialize settings for coarse search.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_CS_INIT (sony_demod_t * pDemod);

/**
 @brief Finalize settings for coarse search.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_CS_FIN (sony_demod_t * pDemod);

/**
 @brief Get the finish flag for power data.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.
 @param pCSFIN CSFIN flag (0: Not finished, 1: Finished)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_GetCSFIN (sony_demod_t * pDemod,
                                                     uint8_t * pCSFIN);

/**
 @brief Get the finish flag for power data in buffer.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.
 @param pCSRDEND CSRDEND flag (0: Not ended, 1: Ended)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_GetCSRDEND (sony_demod_t * pDemod,
                                                       uint8_t * pCSRDEND);

/**
 @brief Get the frequency offset and power data.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.
 @param pCSFREQ Frequency offset in KHz.
 @param pCSPOW Power value.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_GetCSINFO (sony_demod_t * pDemod,
                                                      int32_t * pCSFREQ,
                                                      uint32_t * pCSPOW);

/**
 @brief Get the flag indicate symbol rate search finished.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.
 @param pSRSFIN The flag for symbol rate search finish.
                  - 0 : Symbol rate search does not finished.
                  - 1 : Symbol rate search finished.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_GetSRSFIN (sony_demod_t * pDemod,
                                                      uint8_t * pSRSFIN);

/**
 @brief Get the TRL lock flag.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.
 @param pIsTRLLock The TRL lock flag value.
                  - 0 : TRL is not locked.
                  - 1 : TRL is locked.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_CheckTRLLock (sony_demod_t * pDemod,
                                                         uint8_t * pIsTRLLock);

/**
 @brief Get power data.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.
 @param pPower Power value.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_GetPSPow (sony_demod_t * pDemod,
                                                     uint16_t * pPower);

/**
 @brief Get frequency offset to measure power.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.
 @param freqOffsetKHz Frequency offset in KHz.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_SetCFFine (sony_demod_t * pDemod,
                                                      int32_t freqOffsetKHz);

/**
 @brief Set sampling rage mode.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.
 @param isHSMode Sampling mode
                   - 0 : LS mode
                   - 1 : HS mode

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_SetSampleMode (sony_demod_t * pDemod,
                                                          uint8_t isHSMode);

/**
 @brief Set sampling rage mode.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.
 @param ratioMin Min symbol rate search ratio.
 @param ratioMax Max symbol rate search ratio.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_SetSymbolRateRatio (sony_demod_t * pDemod,
                                                               uint32_t ratioMin,
                                                               uint32_t ratioMax);

/**
 @brief Configure TS output setting.

        This function is used in this driver.
        User doesn't necessary to call it.

 @param pDemod The driver instance.
 @param enable Enable/Disable
                 - 0 : Disable
                 - 1 : Enable

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_blindscan_SetTSOut (sony_demod_t * pDemod,
                                                     uint8_t enable);

#endif /* SONY_DEMOD_DVBS_S2_BLINDSCAN_H */
