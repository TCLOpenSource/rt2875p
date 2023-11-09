/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/09/02
  Modification ID : 4de34284da29630b3c6ce40872f48f739b35e390
------------------------------------------------------------------------------*/
/**
 @file    sony_integ_isdbs.h

          This file provides the integration layer interface for ISDB-S
          specific demodulator functions.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_INTEG_ISDBS_H
#define SONY_INTEG_ISDBS_H

#include "sony_demod.h"
#include "sony_integ.h"
#include "sony_demod_isdbs.h"

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
#define SONY_ISDBS_WAIT_TS_LOCK              1000    /**< 1s timeout for wait TS lock process for ISDB-S channels */
#define SONY_ISDBS_WAIT_LOCK_INTERVAL        10      /**< 10ms polling interval for demodulator and TS lock functions */

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
 Function Pointers
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Performs acquisition to an ISDB-S channel.
        Blocks the calling thread until the TS has locked or has timed out.
        Use ::sony_integ_Cancel to cancel the operation at any time.


 @param pInteg The driver instance.
 @param pTuneParam The parameters required for the tune.

 @return SONY_RESULT_OK if tuned successfully to the channel.
*/
sony_result_t sony_integ_isdbs_Tune (sony_integ_t * pInteg,
                                     sony_isdbs_tune_param_t * pTuneParam);


/**
 @brief Polls the demodulator waiting for TS lock at 10ms intervals up to a timeout of 1s.

 @param pInteg The driver instance

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_isdbs_WaitTSLock (sony_integ_t * pInteg);


/**
 @brief This function returns the estimated RF level based on demodulator gain measurements
        and a tuner dependent conversion calculation. The calculation provided in this monitor
        may require modifications for your own HW integration.

 @param pInteg The driver instance
 @param pRFLeveldB The RF Level estimation in dB * 1000

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_isdbs_monitor_RFLevel (sony_integ_t * pInteg, int32_t * pRFLeveldB);

#endif /* SONY_INTEG_ISDBS_H */
