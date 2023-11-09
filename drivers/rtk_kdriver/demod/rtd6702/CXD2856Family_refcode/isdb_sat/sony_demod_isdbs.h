/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/09/02
  Modification ID : 4de34284da29630b3c6ce40872f48f739b35e390
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_isdbs.h

          This file provides the demodulator control interface specific to ISDB-S.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_ISDBS_H
#define SONY_DEMOD_ISDBS_H

#include "sony_common.h"
#include "sony_demod.h"
#include "sony_isdbs.h"

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief The tune parameters for a ISDB-S signal
*/
typedef struct sony_isdbs_tune_param_t {
    uint32_t centerFreqKHz;                  /**< Center frequency(kHz) of the ISDB-S channel */
    uint16_t tsid;                           /**< TSID */
    sony_isdbs_tsid_type_t tsidType;         /**< TSID Type */
} sony_isdbs_tune_param_t;

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Enable acquisition on the demodulator for ISDB-S channels.  Called from
        the integration layer ::sony_integ_isdbs_Tune API.

 @param pDemod The demodulator instance
 @param pTuneParam The tune parameters.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs_Tune (sony_demod_t * pDemod,
                                     sony_isdbs_tune_param_t * pTuneParam);

/**
 @brief Put the demodulator into ::SONY_DEMOD_STATE_SLEEP state.  Can be called
        from Active, Shutdown or Sleep states.  Called from the integration layer
        ::sony_demod_Sleep API.

 @param pDemod The demodulator instance

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs_Sleep (sony_demod_t * pDemod);

/**
 @brief Check ISDB-S TS lock status.

 @note  This API is for checking lock status in tuning stage.
        After tuning (while receiving the signal),
        please use sony_demod_isdbs_monitor_SyncStat instead
        to check current lock status.

 @param pDemod The demodulator instance
 @param pLock TS lock state

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs_CheckTSLock (sony_demod_t * pDemod,
                                            sony_demod_lock_result_t * pLock);

#endif /* SONY_DEMOD_ISDBS_H */
