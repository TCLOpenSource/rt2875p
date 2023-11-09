/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/08/01
  Modification ID : b30d76210d343216ea52b88e9b450c8fd5c0359f
------------------------------------------------------------------------------*/
/**
 @file    sony_integ_j83b.h

          This file provides the integration layer interface for J.83B specific
          demodulator functions.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_INTEG_J83B_H
#define SONY_INTEG_J83B_H

#include "sony_demod.h"
#include "sony_integ.h"
#include "sony_demod_j83b.h"

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
#define SONY_J83B_WAIT_DEMOD_LOCK            1000    /**< 1s timeout for wait demodulator lock */
#define SONY_J83B_WAIT_LOCK_INTERVAL         10      /**< 10ms interval for demodulator lock polls */

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief The parameters used for J.83B scanning.
*/
typedef struct sony_integ_j83b_scan_param_t {
    /**
     @brief The start frequency in kHz for scanning
    */
    uint32_t startFrequencyKHz;

    /**
     @brief The end frequency in kHz for scanning
    */
    uint32_t endFrequencyKHz;

    /**
     @brief The step frequency in kHz for scanning
    */
    uint32_t stepFrequencyKHz;

    /**
     @brief The bandwidth to use for tuning during the scan
            For J.83B, this parameter is used for specifying symbol rate.
            ::SONY_DTV_BW_J83B_5_06_5_36_MSPS or ::SONY_DTV_BW_J83B_5_60_MSPS are available for this parameter.
    */
    sony_dtv_bandwidth_t bandwidth;

} sony_integ_j83b_scan_param_t;

/**
 @brief The structure used to return a channel located or progress update
        as part of a J.83B scan.
*/
typedef struct sony_integ_j83b_scan_result_t {
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
     @brief The tune params for located J.83B channel.
    */
    sony_j83b_tune_param_t tuneParam;
} sony_integ_j83b_scan_result_t;

/*------------------------------------------------------------------------------
 Function Pointers
------------------------------------------------------------------------------*/
/**
 @brief Callback function that is called for every attempted frequency during a J.83B
        scan.  For successful channel results the function is called after TS lock
        is achieved.

 @param pInteg The driver instance.
 @param pResult The current scan result.
 @param pScanParam The current scan parameters.
*/
typedef void (*sony_integ_j83b_scan_callback_t) (sony_integ_t * pInteg,
                                                 sony_integ_j83b_scan_result_t * pResult,
                                                 sony_integ_j83b_scan_param_t * pScanParam);

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
sony_result_t sony_integ_j83b_Tune (sony_integ_t * pInteg,
                                    sony_j83b_tune_param_t * pTuneParam);

/**
 @brief Performs a scan over the spectrum specified.

        Blocks the calling thread while scanning. Use ::sony_integ_Cancel to cancel
        the operation at any time.

 @param pInteg The driver instance.
 @param pScanParam The scan parameters.
 @param callBack User registered call-back to receive scan progress information and
        notification of found channels. The call back is called for every attempted
        frequency during a scan.

 @return SONY_RESULT_OK if scan completed successfully.

*/
sony_result_t sony_integ_j83b_Scan (sony_integ_t * pInteg,
                                    sony_integ_j83b_scan_param_t * pScanParam,
                                    sony_integ_j83b_scan_callback_t callBack);

/**
 @brief Polls the demodulator waiting for TS lock over a maximum of 1s at 10ms intervals

 @param pInteg The driver instance

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_j83b_WaitTSLock (sony_integ_t * pInteg);

/**
 @brief The RF Level monitor calculates an estimated RF level based on either the demodulator
        IFAGC level or tuner internal RSSI monitor. If any compensation for external hardware
        such as, LNA, attenuators is required, then the user should make adjustment in the
        corresponding function.

 @param pInteg The driver instance
 @param pRFLeveldB The RF Level estimation in dB * 1000

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_j83b_monitor_RFLevel (sony_integ_t * pInteg, int32_t * pRFLeveldB);

#endif /* SONY_INTEG_J83B_H */
