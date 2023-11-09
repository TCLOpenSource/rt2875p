/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
/**
 @file    sony_integ_dvbs_s2.h

          This file provides the integration layer interface for DVB-S/S2 specific
          demodulator functions.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_INTEG_DVBS_S2_H
#define SONY_INTEG_DVBS_S2_H

#include "sony_common.h"
#include "sony_tuner_sat.h"
#include "sony_demod.h"
#include "sony_demod_dvbs_s2.h"
#include "sony_integ.h"

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
/**
 @brief Polling interval for Tune sequence in ms.
*/
#define SONY_INTEG_DVBS_S2_TUNE_POLLING_INTERVAL        10

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/

/**
 @brief Performs acquisition to the channel specified by the pTuneParam struct.
        If the channel system is unknown set the pTuneParam.system to
        ::SONY_DTV_SYSTEM_ANY to enable a blind acquisition.
        In this case the tuned system can be obtained by calling
        ::sony_demod_dvbs_s2_monitor_System.

        Blocks the calling thread until the TS has locked or has timed out.
        Use ::sony_integ_Cancel to cancel the operation at any time.

        The frequency capture range depends on the target symbol rate and it
        is automatically handled by hardware.
        - Symbol Rate >= 20MSps : +/- 10MHz
        - Symbol Rate <  20MSps : Half of Symbol Rate.(i.e. 5MSps : +/- 2.5MHz)

        However, if Symbol Rate is less than 20 MSps and signal condition is good,
        then frequency capture range is "equal to Symbol Rate". (i.e. 5MSps : +/- 5MHz)

 @param pInteg The driver instance.
 @param pTuneParam The parameters required for the tune.

 @return SONY_RESULT_OK if tuned successfully to the channel.
*/
sony_result_t sony_integ_dvbs_s2_Tune (sony_integ_t * pInteg,
                                       sony_dvbs_s2_tune_param_t * pTuneParam);

/**
 @brief This function returns the estimated RF level based on either the demodulator IFAGC
        level or a tuner internal RSSI monitor.  If any compensation for external hardware
        such as, LNA, attenuators is required, then the user should make adjustment in the
        corresponding function.

 @param pInteg The driver instance
 @param pRFLeveldB The RF Level estimation in dB * 1000

 @return SONY_RESULT_OK if successful and pRFLeveldB is valid.
*/
sony_result_t sony_integ_dvbs_s2_monitor_RFLevel (sony_integ_t * pInteg,
                                                  int32_t * pRFLeveldB);

#endif /* SONY_INTEG_DVBS_S2_H */
