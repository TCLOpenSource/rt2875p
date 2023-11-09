/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_integ_dvbc2.h

          This file provides the integration layer interface for DVB-C2 specific
          demodulator functions.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_INTEG_DVBC2_H
#define SONY_INTEG_DVBC2_H

#include "sony_demod.h"
#include "sony_integ.h"
#include "sony_demod_dvbc2.h"

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
#define SONY_DVBC2_WAIT_DEMOD_LOCK          2000 /**< 2s timeout for wait demodulator lock process */
#define SONY_DVBC2_WAIT_TS_LOCK             2000 /**< 2s timeout for wait TS lock process */
#define SONY_DVBC2_WAIT_LOCK_INTERVAL       10   /**< 10ms interval used in wait demodulator and TS lock functions */

/**
 @brief The duration in ms between cancelation checks in the sleep routine for
        the tune and scan sequences.
*/
#define SONY_DVBC2_CHECK_CANCEL_INTERVAL    10

/**
 @brief Maximum duration for a C2 scan routine before reporting an error.  20
        minutes.
*/
#define SONY_DVBC2_MAX_SCAN_DURATION        1200000

/**
 @brief Maximum duration for a C2 tune routine before reporting an error.  20
        seconds.
*/
#define SONY_DVBC2_MAX_TUNE_DURATION        20000

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief The parameters used for DVB-C2 scanning.
*/
typedef struct sony_integ_dvbc2_scan_param_t {
    /**
     @brief The start frequency in kHz for scanning
    */
    uint32_t startFrequencyKHz;

    /**
     @brief The end frequency in kHz for scanning
    */
    uint32_t endFrequencyKHz;

    /**
     @brief The bandwidth to use for tuning during the scan
    */
    sony_dtv_bandwidth_t bandwidth;
} sony_integ_dvbc2_scan_param_t;

/**
 @brief The structure used to return a channel located or progress update
        as part of a DVB-C2 scan.
*/
typedef struct sony_integ_dvbc2_scan_result_t {
    /**
     @brief Indicates the current frequency just attempted for the scan.  This would
            primarily be used to calculate scan progress from the scan parameters.
    */
    uint32_t centerFreqKHz;

    /**
     @brief Indicates if the tune result at the current frequency.  SONY_RESULT_OK
            means that a channel is locked and the tuneParam structure contains
            the channel infomration.
    */
    sony_result_t tuneResult;

    /**
     @brief The tune params for located DVB-C2 channels.
    */
    sony_dvbc2_tune_param_t tuneParam;
} sony_integ_dvbc2_scan_result_t;

/*------------------------------------------------------------------------------
 Function Pointers
------------------------------------------------------------------------------*/
/**
 @brief Callback function that is called for every attempted frequency during a DVB-C2
        scan.  For successful channel results the function is called after demodulator
        lock but before TS lock is achieved.

        NOTE: This funciton is invoked for each PLP in each data slice in the system.

 @param pInteg The driver instance.
 @param pResult The current scan result.
 @param pScanParam The current scan parameters.

*/
typedef void (*sony_integ_dvbc2_scan_callback_t) (sony_integ_t * pInteg,
                                                  sony_integ_dvbc2_scan_result_t * pResult,
                                                  sony_integ_dvbc2_scan_param_t * pScanParam);

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
sony_result_t sony_integ_dvbc2_Tune (sony_integ_t * pInteg,
                                     sony_dvbc2_tune_param_t * pTuneParam);

/**
 @brief Obtains a list of tune parameters for the provided channel frequency and bandwidth.
        A new entry is used for each PLP and data slice combination.

        Blocks the calling thread while scanning. Use ::sony_integ_Cancel to cancel
        the operation at any time.

 @param pInteg The driver instance.
 @param frequencyKHz Centre frequency used to detect valid channels.
 @param bandwidth DVB-C2 andwith used to detect valid channels.
 @param pTuneParamArray List of unique channel tune parameters for the selected centre frequency.
 @param pNumTuneParams Number of tune parameters in pTuneParamArray.
 @param pNextChannelFrequencyKHz The centre frequency of the next valid channel location.

 @return SONY_RESULT_OK if scan completed successfully.

*/
sony_result_t sony_integ_dvbc2_GetChannelTuneParameters (sony_integ_t * pInteg,
                                                         uint32_t frequencyKHz,
                                                         sony_dtv_bandwidth_t bandwidth,
                                                         sony_dvbc2_tune_param_t * pTuneParamArray,
                                                         uint16_t * pNumTuneParams,
                                                         uint32_t * pNextChannelFrequencyKHz);

/**
 @brief Performs a scan over the spectrum specified.

        Blocks the calling thread while scanning. Use ::sony_integ_Cancel to cancel
        the operation at any time.

 @param pInteg The driver instance.
 @param pScanParam The scan parameters.
 @param callBack User registered call-back to receive scan progress information and
        notification of found channels. The call back is called for every attempted
        C2 tuning frequency during a scan.

 @return SONY_RESULT_OK if scan completed successfully.

*/
sony_result_t sony_integ_dvbc2_Scan (sony_integ_t * pInteg,
                                     sony_integ_dvbc2_scan_param_t * pScanParam,
                                     sony_integ_dvbc2_scan_callback_t callBack);

/**
 @brief Polls the demodulator waiting for TS lock over a maximum of 2s at 10ms intervals

 @param pInteg The driver instance.

 @return SONY_RESULT_OK if scan completed successfully.
*/
sony_result_t sony_integ_dvbc2_WaitTSLock (sony_integ_t * pInteg);

/**
 @brief The RF Level monitor calculates an estimated RF level based on either the demodulator
        IFAGC level or tuner internal RSSI monitor. If any compensation for external hardware
        such as, LNA, attenuators is required, then the user should make adjustment in the
        corresponding function.

 @param pInteg  The driver instance
 @param pRFLeveldB  The RF Level estimation in dB * 1000

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_dvbc2_monitor_RFLevel (sony_integ_t * pInteg, int32_t * pRFLeveldB);

#endif /* SONY_INTEG_DVBC2_H */
