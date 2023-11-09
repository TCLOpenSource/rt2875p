/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/08/01
  Modification ID : b30d76210d343216ea52b88e9b450c8fd5c0359f
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_j83b.h

          This file provides the demodulator control interface specific to J.83B.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_J83B_H
#define SONY_DEMOD_J83B_H

#include "sony_common.h"
#include "sony_demod.h"

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief The tune parameters for a J.83B signal
*/
typedef struct sony_j83b_tune_param_t {
    uint32_t centerFreqKHz;                   /**< Center frequency in kHz of the J.83B channel */
    /**
     @brief Bandwidth of the J.83B channel
            For J.83B, this parameter is used for specifying symbol rate.
            ::SONY_DTV_BW_J83B_5_06_5_36_MSPS or ::SONY_DTV_BW_J83B_5_60_MSPS are available for this parameter.
    */
    sony_dtv_bandwidth_t bandwidth;
} sony_j83b_tune_param_t;

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Enable acquisition on the demodulator for J.83B channels.  Called from
        the integration layer ::sony_integ_j83b_Tune API.

 @param pDemod  The demodulator instance
 @param pTuneParam The tune parameters.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_j83b_Tune (sony_demod_t * pDemod, sony_j83b_tune_param_t * pTuneParam);

/**
 @brief Put the demodulator into ::SONY_DEMOD_STATE_SLEEP state.  Can be called
        from Active, Shutdown or Sleep states.  Called from the integration layer
        ::sony_integ_Sleep API.

 @param pDemod  The demodulator instance

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_j83b_Sleep (sony_demod_t * pDemod);

/**
 @brief Check J.83B TS lock status.

 @note  This API is for checking lock status in tuning stage.
        After tuning (while receiving the signal),
        please use sony_demod_j83b_monitor_SyncStat instead
        to check current lock status.

 @param pDemod The demodulator instance
 @param pLock TS lock state

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_j83b_CheckTSLock (sony_demod_t * pDemod,
                                           sony_demod_lock_result_t * pLock);

#endif /* SONY_DEMOD_J83B_H */
