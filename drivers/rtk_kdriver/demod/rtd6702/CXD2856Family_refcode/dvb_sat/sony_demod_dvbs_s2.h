/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2.h

          This file provides the demodulator control interface specific to DVB-S/S2.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_H
#define SONY_DEMOD_DVBS_S2_H

#include "sony_common.h"
#include "sony_demod.h"

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief The Tune parameters for a DVB-S/S2 signal.
*/
typedef struct {
    sony_dtv_system_t system;       /**< System of the channel. */
    uint32_t centerFreqKHz;         /**< Center frequency in kHz of the DVB-S/S2 channel. */
    uint32_t symbolRateKSps;        /**< Symbol rate in kHz of the DVB-S/S2 channel. */
} sony_dvbs_s2_tune_param_t;

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/

/**
 @brief Enable acquisition on the demodulator for DVB-S/S2 channels.
        Called from the integration layer ::sony_integ_dvbs_s2_Tune API.

 @param pDemod The demodulator instance
 @param pTuneParam The tune parameters.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_Tune (sony_demod_t * pDemod,
                                       sony_dvbs_s2_tune_param_t * pTuneParam);

/**
 @brief Put the demodulator into ::SONY_DEMOD_STATE_SLEEP state.
        Can be called from Active, Shutdown or Sleep states.
        Called from the integration layer ::sony_integ_Sleep API.

 @param pDemod  The demodulator instance

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_Sleep (sony_demod_t * pDemod);

/**
 @brief Check whether TS signal is locked or not locked.

 @note  This API is for checking lock status in tuning stage.
        After tuning (while receiving the signal),
        please use sony_demod_dvbs_s2_monitor_SyncStat instead
        to check current lock status.

 @param pDemod  The demodulator instance
 @param pLock   Address of lock status.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_CheckTSLock (sony_demod_t * pDemod,
                                              sony_demod_lock_result_t * pLock);

/**
 @brief Get the I/Q polarity of device.

        This API is called by driver internal.
        So user doesn't need to use it.

 @param pDemod    The demodulator instance
 @param pIsInvert Polarity of I/Q sense. (0: Not inverted. 1: Inverted)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_CheckIQInvert (sony_demod_t * pDemod,
                                                uint8_t * pIsInvert);

/**
 @brief Configure demodulator setting to demodulate symbol rate.

 @param pDemod         The demodulator instance
 @param symbolRateKSps Symbol rate in KSps.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_SetSymbolRate (sony_demod_t * pDemod,
                                                uint32_t symbolRateKSps);

#endif /* SONY_DEMOD_DVBS_S2_H */
