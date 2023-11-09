/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_integ_dvbt_t2.h

          This file provides the integration layer interface for DVB-T and DVB-T2
          specific demodulator functions.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_INTEG_DVBT_T2_H
#define SONY_INTEG_DVBT_T2_H

#include "sony_demod.h"
#include "sony_integ.h"
#include "sony_integ_dvbt.h"
#include "sony_integ_dvbt2.h"
#include "sony_demod_dvbt.h"
#include "sony_demod_dvbt2.h"

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief The parameters used for DVB-T and DVB-T2 scanning.
*/
typedef struct sony_integ_dvbt_t2_scan_param_t{
    /**
     @brief The start frequency in kHz for scanning.  Ensure that this is
            aligned with the channel raster.
    */
    uint32_t startFrequencyKHz;

    /**
     @brief The end frequency in kHz for scanning.
    */
    uint32_t endFrequencyKHz;

    /**
     @brief The step frequency in kHz for scanning.
    */
    uint32_t stepFrequencyKHz;

    /**
     @brief The bandwidth to use for tuning during the scan
    */
    sony_dtv_bandwidth_t bandwidth;

    /**
     @brief The system to attempt to blind tune to at each step.  Use
            ::SONY_DTV_SYSTEM_ANY to run a multiple system scan (DVB-T and
            DVB-T2).
    */
    sony_dtv_system_t system;

    /**
     @brief The DVB-T2 profile to use for the blind tune.  Use
            ::SONY_DVBT2_PROFILE_ANY for mixed or Base and Lite spectrums.
    */
    sony_dvbt2_profile_t t2Profile;
}sony_integ_dvbt_t2_scan_param_t;

/**
 @brief The structure used to return a channel located or progress update
        as part of a DVB-T / DVB-T2 or combined scan.
*/
typedef struct sony_integ_dvbt_t2_scan_result_t{
    /**
     @brief Indicates the current frequency just attempted for the scan.  This would
            primarily be used to calculate scan progress from the scan parameters.
    */
    uint32_t centerFreqKHz;

    /**
     @brief Indicates if the tune result at the current frequency.  SONY_RESULT_OK
            means that a channel has been locked and one of the tuneParam structures
            contain the channel infomration.
    */
    sony_result_t tuneResult;

    /**
     @brief The system of the channel detected by the scan.  This should be used to determine
            which of the following tune param structs are valid.
    */
    sony_dtv_system_t system;

    /**
     @brief The tune params for a located DVB-T channel.
    */
    sony_dvbt_tune_param_t dvbtTuneParam;

    /**
     @brief The tune params for a located DVB-T2 channel.
    */
    sony_dvbt2_tune_param_t dvbt2TuneParam;
}sony_integ_dvbt_t2_scan_result_t;

/*------------------------------------------------------------------------------
 Function Pointers
------------------------------------------------------------------------------*/
/**
 @brief Callback function that is called for every attempted frequency during a
        scan.  For successful channel results the function is called after
        demodulator lock but before TS lock is achieved (DVB-T : TPS Lock,
        DVB-T2 : Demod Lock).

        NOTE: for DVB-T2 this function is invoked for each PLP within the signal.

 @param pInteg The driver instance.
 @param pResult The current scan result.
 @param pScanParam The current scan parameters.
*/
typedef void (*sony_integ_dvbt_t2_scan_callback_t) (sony_integ_t * pInteg,
                                                    sony_integ_dvbt_t2_scan_result_t * pResult,
                                                    sony_integ_dvbt_t2_scan_param_t * pScanParam);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Attempts to acquire to the channel at the center frequency provided. The
        system can be specified directly or set to ::SONY_DTV_SYSTEM_ANY to allow
        tuning to DVB-T or DVB-T2 for unknown cases.

        This function blocks the calling thread until the demod has locked or has
        timed out. Use ::sony_integ_Cancel to cancel the operation at any time.

        For TS lock please call the wait TS lock function
        ::sony_integ_dvbt_WaitTSLock or ::sony_integ_dvbt2_WaitTSLock.

        NOTE: For T2 the PLP selected will be the first found in the L1 pre signalling.
        Use ::sony_demod_dvbt2_monitor_DataPLPs to obtain a full list of PLPs contained
        in the T2 signal.

        Note: For DVB-T2 the output selected if profile is set to ::SONY_DVBT2_PROFILE_ANY
        will be determined by the first frame received.

        Note: If SONY_DVBT2_PROFILE_LITE or SONY_DVBT2_PROFILE_ANY are used for profile argument,
        DVB-T2 early unlock detection time and lock waiting timeout will be longer.

 @param pInteg The driver instance.
 @param centerFreqKHz The center frequency of the channel to attempt acquisition on
 @param bandwidth The bandwidth of the channel
 @param system The system to attempt to tune to, use ::SONY_DTV_SYSTEM_ANY to attempt
               both DVB-T and DVB-T2
 @param profile The DVB-T2 profile to detect, use ::SONY_DVBT2_PROFILE_ANY to detect Base
                or Lite.
 @param pSystemTuned The system of the channel located by the blind tune.
 @param pProfileTuned The DVB-T2 profile tuned by the blind tune.

 @return SONY_RESULT_OK if tuned successfully to the channel.
*/
sony_result_t sony_integ_dvbt_t2_BlindTune(sony_integ_t * pInteg,
                                           uint32_t centerFreqKHz,
                                           sony_dtv_bandwidth_t bandwidth,
                                           sony_dtv_system_t system,
                                           sony_dvbt2_profile_t profile,
                                           sony_dtv_system_t * pSystemTuned,
                                           sony_dvbt2_profile_t * pProfileTuned);

/**
 @brief Performs a scan over the spectrum specified.

        The scan can perform a multiple system scan for DVB-T and DVB-T2 channels by
        setting the ::sony_integ_dvbt_t2_scan_param_t::system to ::SONY_DTV_SYSTEM_ANY
        and setting the.

        Blocks the calling thread while scanning. Use ::sony_integ_Cancel to cancel
        the operation at any time.

 @param pInteg The driver instance.
 @param pScanParam The scan parameters.
 @param callBack User registered call-back to receive scan progress information and
        notification of found channels. The call back is called for every attempted
        frequency during a scan.

 @return SONY_RESULT_OK if scan completed successfully.

*/
sony_result_t sony_integ_dvbt_t2_Scan(sony_integ_t * pInteg,
                                      sony_integ_dvbt_t2_scan_param_t * pScanParam,
                                      sony_integ_dvbt_t2_scan_callback_t callBack);

/**
 @brief This function returns the estimated RF level based on either the demodulator IFAGC
        level or a tuner internal RSSI monitor.  If any compensation for external hardware
        such as, LNA, attenuators is required, then the user should make adjustment in the
        corresponding function.

 @param pInteg The driver instance
 @param pRFLeveldB The RF Level estimation in dB * 1000

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_dvbt_t2_monitor_RFLevel (sony_integ_t * pInteg, int32_t * pRFLeveldB);

#endif /* SONY_INTEG_DVBT_T2_H */
