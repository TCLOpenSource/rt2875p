/*------------------------------------------------------------------------------
  Copyright 2017 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
/**
 @file    sony_integ_dvbs_s2_blindscan.h

          This file provides the integration layer interface for DVB-S/S2 blindscan
          related functions.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_INTEG_DVBS_S2_BLINDSCAN_H
#define SONY_INTEG_DVBS_S2_BLINDSCAN_H

#include "sony_common.h"
#include "sony_tuner_sat.h"
#include "sony_demod.h"
#include "sony_demod_dvbs_s2.h"
#include "sony_demod_dvbs_s2_blindscan_seq.h"
#include "sony_demod_dvbs_s2_tune_srs.h"
#include "sony_demod_dvbs_s2_tune_srs_seq.h"
#include "sony_integ.h"

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
/**
 @brief Polling interval for BlindScan sequence in ms.
*/
#define SONY_INTEG_DVBS_S2_BLINDSCAN_POLLING_INTERVAL   10

/**
 @brief Polling interval for TuneSRS sequence in ms.
*/
#define SONY_INTEG_DVBS_S2_TUNE_SRS_POLLING_INTERVAL    10

/*------------------------------------------------------------------------------
 Enums
------------------------------------------------------------------------------*/
/**
 @brief Definition of event id for callback from BlindScan.
*/
typedef enum {
    SONY_INTEG_DVBS_S2_BLINDSCAN_EVENT_DETECT,    /**< Detect channel. */
    SONY_INTEG_DVBS_S2_BLINDSCAN_EVENT_PROGRESS   /**< Update progress. */
} sony_integ_dvbs_s2_blindscan_event_id_t;

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief The TuneSRS functions parameters for a DVB-S/S2 signal.
*/
typedef struct {
    /**
     @brief Center frequency in KHz of the DVB-S/S2 channel.
    */
    uint32_t centerFreqKHz;
    /**
     @brief Frequency range found by TuneSRS.
    */
    sony_demod_dvbs_s2_tune_srs_freq_range_t freqRange;
} sony_integ_dvbs_s2_tune_srs_param_t;

/**
 @brief Definition of the parameters for BlindScan.
*/
typedef struct {
    /**
     @brief Min frequency of scan range in KHz.

            Please set the min frequency of channel which you would like to detect in KHz.
            (i.e.  950000)
    */
    uint32_t minFreqKHz;
    /**
     @brief Max frequency of scan range.

            Please set the min frequency of channel which you would like to detect in KHz.
            (i.e. 2150000)
    */
    uint32_t maxFreqKHz;
    /**
     @brief Min symbol rate of scan range.

            Please set the min symbol rate of channel which you would like to detect in KSps.
            (i.e.    1000)
    */
    uint32_t minSymbolRateKSps;
    /**
     @brief Max symbol rate of scan range.

            Please set the max symbol rate of channel which you would like to detect in KSps.
            (i.e.   45000)
    */
    uint32_t maxSymbolRateKSps;

} sony_integ_dvbs_s2_blindscan_param_t;

/**
 @brief Definition of the result of BlindScan.
*/
typedef struct {
    /**
     @brief The cause of callback.

            When callback function was called, please check it first.
    */
    sony_integ_dvbs_s2_blindscan_event_id_t eventId;
    /**
     @brief Detected channel information.

            If "eventId == SONY_INTEG_DVBS_S2_BLINDSCAN_EVENT_DETECT", this value is valid.
    */
    sony_dvbs_s2_tune_param_t tuneParam;
    /**
     @brief Progress.

            If "eventId == SONY_INTEG_DVBS_S2_BLINDSCAN_EVENT_PROGRESS", this value is valid.
            The range of this value is 0 - 100 in percentage.
    */
    uint8_t progress;
}sony_integ_dvbs_s2_blindscan_result_t;

/*------------------------------------------------------------------------------
 Function Pointers
------------------------------------------------------------------------------*/
/**
 @brief Callback function that is called for every attempted frequency during a DVB-S/S2
        scan.  For successful channel results the function is called before TS lock
        is achieved.

 @param pInteg The driver instance.
 @param pResult The current scan result.
*/
typedef void (*sony_integ_dvbs_s2_blindscan_callback_t) (sony_integ_t * pInteg,
                                                         sony_integ_dvbs_s2_blindscan_result_t * pResult);

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/

/**
 @brief Tune SRS (Symbol Rate Search) performs a blind acquisition to the
        channel with wider range than ::sony_integ_dvbs_s2_Tune(). It captures
        an available signal having Symbol Rate of 1 to 45 MSps and centred
        within  +/- 10 MHz of the RF frequency specified by the pParam struct.
        This enables acquisition to a DVB-S or DVB-S2 channel where the center
        frequency or symbol rate is not precisely known.

        The located channel tune parameters are then written into the pTuneParam
        structure, which can subsequently be passed to ::sony_integ_dvbs_s2_Tune
        for quicker acquisition.

        Blocks the calling thread until the TS has locked or has timed out.
        Use ::sony_integ_Cancel to cancel the operation at any time.

 @param pInteg The driver instance.
 @param pSeq The sequence instance.
 @param pParam Parameter for TuneSRS.
 @param pTuneParam The detected channel information.

 @return SONY_RESULT_OK if tuned successfully to the channel and pTuneParam is valid.
*/
sony_result_t sony_integ_dvbs_s2_TuneSRS (sony_integ_t * pInteg,
                                          sony_demod_dvbs_s2_tune_srs_seq_t * pSeq,
                                          sony_integ_dvbs_s2_tune_srs_param_t * pParam,
                                          sony_dvbs_s2_tune_param_t * pTuneParam);

/**
 @brief BlindScan searches channels within the frequency and symbol rate range set
        by pParam. The whole process is carried out in three steps. In the first step,
        it finds channels having Symbol Rate more than 20 MSps. In the second step, it
        finds channels having Symbol Rate between 5 to 20 MSps. In the third step, it
        finds channels having Symbol Rate between 1 to 5 MSps.

        Callback function is called during each step at the following points.
          - Detected channel
          - Update progress
        The source of callback is determined by checking sony_integ_dvbs_s2_blindscan_result_t::eventId.
        Please see explanation in ::sony_integ_dvbs_s2_blindscan_result_t structure.

        Blocks the calling thread until the BlindScan has finished.
        Use ::sony_integ_Cancel to cancel the operation at any time.

 @param pInteg The integration part instance.
 @param pSeq The sequence instance.
 @param pParam Scan parameters.
 @param callback Callback function pointer.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_dvbs_s2_BlindScan (sony_integ_t * pInteg,
                                            sony_demod_dvbs_s2_blindscan_seq_t * pSeq,
                                            sony_integ_dvbs_s2_blindscan_param_t * pParam,
                                            sony_integ_dvbs_s2_blindscan_callback_t callback);

#endif /* SONY_INTEG_DVBS_S2_BLINDSCAN_H */
