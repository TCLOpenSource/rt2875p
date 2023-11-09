/*------------------------------------------------------------------------------
  Copyright 2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
/**
 @file    sony_integ_sat_device_ctrl_singlecable_auto_install.h

          This file provide functions useful for single cable 1 (EN50494) automatic installation.
          This file located in blindscan folder because blind scanning technique is used.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_AUTO_INSTALL_H
#define SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_AUTO_INSTALL_H

#include "sony_integ.h"
#include "sony_demod_sat_device_ctrl_find1ub_seq.h"
#include "sony_demod_sat_device_ctrl_detect_cw_seq.h"

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief Return codes for Detect YES/NO results.
*/
typedef enum {
    SONY_INTEG_SAT_DEVICE_CTRL_DETECT_YES_NO_RESULT_YES,      /**< Detected YES Signal for Detect YES/NO. */
    SONY_INTEG_SAT_DEVICE_CTRL_DETECT_YES_NO_RESULT_NO,       /**< Detected NO Signal for Detect YES/NO. */
    SONY_INTEG_SAT_DEVICE_CTRL_DETECT_YES_NO_RESULT_NOTDETECT,/**< Could not detect any YES/NO signal. */
} sony_integ_sat_device_ctrl_detect_yes_no_result_t;

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Find one user band.

 @note  LNB controller and satellite device control function will be automatically enabled.

 @param pInteg The integration part instance.
 @param pSeq The address of sequence instance.
 @param minFreqKHz The minimum frequency of search range in KHz.
 @param maxFreqKHz The maximum frequency of search range in KHz.
 @param userbandId The user band ID.(1-8)
 @param pFreqKHz The address of detected frequency in KHz.
                 If this value is 0, driver can't detect target user band.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_Find1UB (sony_integ_t * pInteg,
                                                  sony_demod_sat_device_ctrl_find1ub_seq_t * pSeq,
                                                  uint32_t minFreqKHz,
                                                  uint32_t maxFreqKHz,
                                                  uint8_t userbandId,
                                                  uint32_t * pFreqKHz);

/**
 @brief Detect YES/NO answer from SCIF.

 @note  LNB controller and satellite device control function will be automatically enabled.

 @param pInteg The integration part instance.
 @param pSeq The address of sequence instance.
 @param ubFreqKHz The frequency of the target user band in KHz.
 @param noFreqOffsetKHz The frequency offset for NO signal from the target user band in KHz. (20,000 is default)
 @param yesMarginFreqKHz The margin frequency for YES signal. (500 is default)
 @param noMarginFreqKHz The margin frequency for NO signal. (1000 is default)
 @param pResult The address of result for this API.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_sat_device_ctrl_DetectYesNo (sony_integ_t * pInteg,
                                                      sony_demod_sat_device_ctrl_detect_cw_seq_t * pSeq,
                                                      uint32_t ubFreqKHz,
                                                      uint32_t noFreqOffsetKHz,
                                                      uint32_t yesMarginFreqKHz,
                                                      uint32_t noMarginFreqKHz,
                                                      sony_integ_sat_device_ctrl_detect_yes_no_result_t * pResult);

#endif /* SONY_INTEG_SAT_DEVICE_CTRL_SINGLECABLE_AUTO_INSTALL_H */
