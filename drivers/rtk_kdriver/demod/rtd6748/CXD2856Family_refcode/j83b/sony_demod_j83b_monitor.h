/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/08/01
  Modification ID : b30d76210d343216ea52b88e9b450c8fd5c0359f
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_j83b_monitor.h

          This file provides the J.83B demodulator monitor interface.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_J83B_MONITOR_H
#define SONY_DEMOD_J83B_MONITOR_H

#include "sony_demod.h"
#include "sony_j83b.h"

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Acquisition state monitor.

 @note  Note that early unlock condition should be used in tuning stage ONLY
        to detect that there are no desired signal in current frequency quickly.
        After tuning, early unlock condition should NOT be used to
        know current demodulator lock status.

 @param pDemod The demodulator instance.
 @param pARLock The demodulator lock state.
        - 0: AR not Locked,
        - 1: AR Locked,
 @param pTSLockStat Indicates the TS lock condition.
        - 0: TS not locked.
        - 1: TS locked.
 @param pUnlockDetected Indicates the early unlock condition, only valid
        when demodulator is set to J.83B scan mode enabled.
        - 0: No early unlock.
        - 1: Early unlock.

 @return SONY_RESULT_OK on success and pARLock, pTSLockStat and pUnlockDetected are valid.

*/
sony_result_t sony_demod_j83b_monitor_SyncStat (sony_demod_t * pDemod,
                                                uint8_t * pARLock,
                                                uint8_t * pTSLockStat,
                                                uint8_t * pUnlockDetected);

/**
 @brief Monitor the IFAGC value. The actual dB gain is dependent on the attached tuner.

 @param pDemod The demodulator instance.
 @param pIFAGCOut Value of IFAGC output register value. Unitless.

 @return SONY_RESULT_OK if successful and pIFAGCOut valid.
*/
sony_result_t sony_demod_j83b_monitor_IFAGCOut (sony_demod_t * pDemod,
                                                uint32_t * pIFAGCOut);

/**
 @brief Monitor the constellation of the currently tuned channel.

 @param pDemod The demodulator instance.
 @param pQAM The monitored constellation.

 @return SONY_RESULT_OK if successful and pQAM valid.
*/
sony_result_t sony_demod_j83b_monitor_QAM (sony_demod_t * pDemod,
                                           sony_j83b_constellation_t * pQAM);

/**
 @brief Monitor the symbol rate of the currently tuned channel.

 @param pDemod The demodulator instance.
 @param pSymRate Symbol rate (Symbol/sec).

 @return SONY_RESULT_OK if successful and pSymRate valid.
*/
sony_result_t sony_demod_j83b_monitor_SymbolRate (sony_demod_t * pDemod,
                                                  uint32_t * pSymRate);

/**
 @brief Monitor the sampling frequency offset value.

 @param pDemod The demodulator instance.
 @param pPPM The sampling frequency offset in ppm x 100.

 @return SONY_RESULT_OK if pPPM is valid.
*/
sony_result_t sony_demod_j83b_monitor_SamplingOffset (sony_demod_t * pDemod,
                                                      int32_t * pPPM);

/**
 @brief Monitors the detected carrier offset of the currently tuned channel.

        To get the estimated center frequency of the current channel:
        Freq_Est = Freq_Tune + pOffset;

        This function will compensate for an inverting tuner architecture if the
        demodulator has been configured accordignly using ::SONY_DEMOD_CONFIG_SPECTRUM_INV
        config option for ::sony_demod_SetConfig.

 @param pDemod The demodulator instance.
 @param pOffset Carrier offset value(kHz).

 @return SONY_RESULT_OK if successful and pOffset valid.
*/
sony_result_t sony_demod_j83b_monitor_CarrierOffset (sony_demod_t * pDemod,
                                                     int32_t * pOffset);

/**
 @brief Monitors the channel spectrum sense.  To ensure correct polarity detection
        please use the ::SONY_DEMOD_CONFIG_SPECTRUM_INV config option in
        ::sony_demod_SetConfig to select the appropriate spectrum inversion
        for the tuner output.

 @param pDemod The demodulator instance.
 @param pSense Indicates the spectrum sense.

 @return SONY_RESULT_OK if successful and pSense valid.
*/
sony_result_t sony_demod_j83b_monitor_SpectrumSense (sony_demod_t * pDemod,
                                                     sony_demod_terr_cable_spectrum_sense_t * pSense);

/**
 @brief Monitors the estimated SNR register value, clipped to a
        maximum of 50dB.

 @param pDemod The demodulator instance.
 @param pSNR The estimated SNR value in dB x 1000.

 @return SONY_RESULT_OK if successful and pSNR valid.
*/
sony_result_t sony_demod_j83b_monitor_SNR (sony_demod_t * pDemod,
                                           int32_t * pSNR);

/**
 @brief Monitor the Pre-RS BER.

 @param pDemod The demodulator instance.
 @param pBER BER value (Pre-Reed Solomon decoder) x 1e7.

 @return SONY_RESULT_OK if successful and pBER valid.
*/
sony_result_t sony_demod_j83b_monitor_PreRSBER (sony_demod_t * pDemod,
                                                uint32_t * pBER);

/**
 @brief Monitors the number RS (Reed Solomon) errors detected by the
        RS decoder over 1 second. Also known as the code word reject count.

 @param pDemod The demodulator instance.
 @param pPEN The number of RS errors detected over 1 second.

 @return SONY_RESULT_OK if successful and pPEN valid.
*/
sony_result_t sony_demod_j83b_monitor_PacketErrorNumber (sony_demod_t * pDemod,
                                                         uint32_t * pPEN);

/**
 @brief Monitor the J.83B PER (Packet Error Rate).

 @param pDemod The demod instance.
 @param pPER The estimated PER x 1e6.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_j83b_monitor_PER (sony_demod_t * pDemod,
                                           uint32_t * pPER);

/**
 @brief Monitor the Interleave mode

 @param pDemod The demod instance.
 @param pInterleave Indicates the interleave mode.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_j83b_monitor_Interleave (sony_demod_t * pDemod,
                                                  sony_j83b_interleave_t * pInterleave);

#endif /* SONY_DEMOD_J83B_MONITOR_H */
