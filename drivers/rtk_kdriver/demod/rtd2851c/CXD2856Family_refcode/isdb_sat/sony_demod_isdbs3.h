/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/09/02
  Modification ID : 4de34284da29630b3c6ce40872f48f739b35e390
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_isdbs3.h

          This file provides the demodulator control interface specific to ISDB-S3.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_ISDBS3_H
#define SONY_DEMOD_ISDBS3_H

#include "sony_common.h"
#include "sony_demod.h"
#include "sony_isdbs3.h"

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief The tune parameters for a ISDB-S3 signal
*/
typedef struct sony_isdbs3_tune_param_t {
    uint32_t centerFreqKHz;                   /**< Center frequency(kHz) of the ISDB-S3 channel */
    uint16_t streamid;                        /**< StreamID */
    sony_isdbs3_streamid_type_t streamidType; /**< StreamID Type */
} sony_isdbs3_tune_param_t;

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Enable acquisition on the demodulator for ISDB-S3 channels.  Called from
        the integration layer ::sony_integ_isdbs3_Tune API.

 @param pDemod The demodulator instance
 @param pTuneParam The tune parameters.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_Tune (sony_demod_t * pDemod,
                                      sony_isdbs3_tune_param_t * pTuneParam);

/**
 @brief Put the demodulator into ::SONY_DEMOD_STATE_SLEEP state.  Can be called
        from Active, Shutdown or Sleep states.  Called from the integration layer
        ::sony_demod_Sleep API.

 @param pDemod The demodulator instance

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_Sleep (sony_demod_t * pDemod);

/**
 @brief Check ISDB-S3 TMCC lock status.

 @note  This API is for checking lock status in tuning stage.
        After tuning (while receiving the signal),
        please use sony_demod_isdbs3_monitor_SyncStat instead
        to check current lock status.

 @param pDemod The demodulator instance
 @param pLock TMCC lock state

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_CheckTMCCLock (sony_demod_t * pDemod,
                                               sony_demod_lock_result_t * pLock);

/**
 @brief Check ISDB-S3 TSTLV lock status.

 @note  This API is for checking lock status in tuning stage.
        After tuning (while receiving the signal),
        please use sony_demod_isdbs3_monitor_SyncStat instead
        to check current lock status.

 @param pDemod The demodulator instance
 @param pLock TSTLV lock state

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_CheckTSTLVLock (sony_demod_t * pDemod,
                                                sony_demod_lock_result_t * pLock);


/**
 @brief Set ISDB-S3 Stream ID.

 @param pDemod The demodulator instance
 @param stream_id Stream ID

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_SetStreamID (sony_demod_t * pDemod, uint16_t stream_id);

#endif /* SONY_DEMOD_ISDBS3_H */
