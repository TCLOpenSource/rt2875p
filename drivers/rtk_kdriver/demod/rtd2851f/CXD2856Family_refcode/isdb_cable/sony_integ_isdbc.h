/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/08/01
  Modification ID : b30d76210d343216ea52b88e9b450c8fd5c0359f
------------------------------------------------------------------------------*/
/**
 @file    sony_integ_isdbc.h

          This file provides the integration layer interface for ISDB-C specific
          demodulator functions.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_INTEG_ISDBC_H
#define SONY_INTEG_ISDBC_H

#include "sony_demod.h"
#include "sony_integ.h"
#include "sony_demod_isdbc.h"

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
#define SONY_ISDBC_WAIT_DEMOD_LOCK           1000    /**< 1s timeout for wait demodulator lock */
#define SONY_ISDBC_WAIT_LOCK_INTERVAL        10      /**< 10ms interval for demodulator lock polls */

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Performs acquisition to the channel.
        Blocks the calling thread until the TS has locked or has timed out.
        Use ::sony_integ_Cancel to cancel the operation at any time.

 @param pInteg The driver instance.
 @param pTuneParam The parameters required for the tune.

 @return SONY_RESULT_OK if tuned successfully to the channel.
*/
sony_result_t sony_integ_isdbc_Tune (sony_integ_t * pInteg,
                                     sony_isdbc_tune_param_t * pTuneParam);

/**
 @brief Polls the demodulator waiting for TS lock over a maximum of 1s at 10ms intervals

 @param pInteg The driver instance

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_isdbc_WaitTSLock (sony_integ_t * pInteg);

/**
 @brief The RF Level monitor calculates an estimated RF level based on either the demodulator
        IFAGC level or tuner internal RSSI monitor. If any compensation for external hardware
        such as, LNA, attenuators is required, then the user should make adjustment in the
        corresponding function.

 @param pInteg The driver instance
 @param pRFLeveldB The RF Level estimation in dB * 1000

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_isdbc_monitor_RFLevel (sony_integ_t * pInteg, int32_t * pRFLeveldB);

#endif /* SONY_INTEG_ISDBC_H */
