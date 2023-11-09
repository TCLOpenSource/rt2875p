/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbc2_monitor.h

          This file provides the DVB-C2 demodulator monitor interface.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_DVBC2_MONITOR_H
#define SONY_DEMOD_DVBC2_MONITOR_H

#include "sony_demod.h"

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Monitor the IFAGC value of the C2 demodulator.
        Actual dB gain dependent on attached tuner.

 @param pDemod The demodulator instance.
 @param pIFAGC The IFAGC output register value. Range 0x000 - 0xFFF. Unitless.

 @return SONY_RESULT_OK if pIFAGC is valid.
*/
sony_result_t sony_demod_dvbc2_monitor_IFAGCOut (sony_demod_t * pDemod,
                                                 uint32_t * pIFAGC);

/**
 @brief Monitors the channel spectrum sense.  To ensure correct polarity detection
        please use the ::SONY_DEMOD_CONFIG_SPECTRUM_INV config option in
        ::sony_demod_SetConfig to select the appropriate spectrum inversion
        for the tuner output.

 @param pDemod The demodulator instance.
 @param pSense Indicates the spectrum sense.

 @return SONY_RESULT_OK if successful and pSense is valid.
*/
sony_result_t sony_demod_dvbc2_monitor_SpectrumSense (sony_demod_t * pDemod,
                                                      sony_demod_terr_cable_spectrum_sense_t * pSense);

/**
 @brief Monitors the detected carrier offset of the currently tuned channel, using
        the continual pilot (CP) estimation from the demodulator.

        To get the estimated center frequency of the current channel:
        Freq_Est = Freq_Tune + pOffset;

        This function will compensate for an inverting tuner architecture if the
        demodulator has been configured accordignly using ::SONY_DEMOD_CONFIG_SPECTRUM_INV
        config option for ::sony_demod_SetConfig.

 @param pDemod The demodulator instance.
 @param pOffset The detected carrier offset in Hz.

 @return SONY_RESULT_OK if pOffset is valid.
*/
sony_result_t sony_demod_dvbc2_monitor_CarrierOffset (sony_demod_t * pDemod,
                                                      int32_t * pOffset);

/**
 @brief Monitor the SNR of the DVB-C2 demodulator, clipped to a
        maximum of 50dB.

 @param pDemod The demodulator instance.
 @param pSNR The returned SNR in dB x 1000.

 @return SONY_RESULT_OK if pSNR is valid.
*/
sony_result_t sony_demod_dvbc2_monitor_SNR (sony_demod_t * pDemod,
                                            int32_t * pSNR);

/**
 @brief Monitor the pre-LDPC BER. This provides the
        data PLP BER in multiple PLP case.

 @param pDemod The demodulator instance.
 @param pBER The returned BER x 1e7.

 @return SONY_RESULT_OK if the pBER is valid.
*/
sony_result_t sony_demod_dvbc2_monitor_PreLDPCBER (sony_demod_t * pDemod,
                                                   uint32_t * pBER);

/**
 @brief Monitor the pre-BCH BER.

 @param pDemod The demodulator instance.
 @param pBER The returned BER x 1e9.

 @return SONY_RESULT_OK if pBER is valid.
*/
sony_result_t sony_demod_dvbc2_monitor_PreBCHBER (sony_demod_t * pDemod,
                                                  uint32_t * pBER);

/**
 @brief Monitor the post BCH FER (FEC block error rate) parameters.

 @param pDemod The demod instance.
 @param pFER The estimated FER x 1e6.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbc2_monitor_PostBCHFER (sony_demod_t * pDemod,
                                                   uint32_t * pFER);

/**
 @brief Monitor the Packet Error Number.

 @param pDemod The demodulator instance.
 @param pPEN The returned Packet Error Number.

 @return SONY_RESULT_OK if pPEN is valid.
*/
sony_result_t sony_demod_dvbc2_monitor_PacketErrorNumber (sony_demod_t * pDemod,
                                                          uint32_t * pPEN);

/**
 @brief Monitors the synchronisation state of the C2 demodulator.

 @note  Note that early unlock condition should be used in tuning stage ONLY
        to detect that there are no desired signal in current frequency quickly.
        After tuning, early unlock condition should NOT be used to
        know current demodulator lock status.
        Especially, DVB-C2 early unlock condition does NOT change after tuning.
        (keeps the value decided in tuning stage.)

 @param pDemod The demodulator instance.
 @param pSyncStat The demodulator state.
        - 0: WAIT_GO,       : Wait for process to start
        - 1: WAIT_AGC,      : Wait for AGC lock
        - 2: WAIT_CCOD,     : Wait for coarse frequency offset estimation
        - 3: WAIT_C2FS,     : Wait for preamble symbol lock
        - 4: WAIT_L1SEQ,    : Wait for L1 decode
        - 5: WAIT_DMD_OK,   : Wait for demodulator lock
        - 6: DMD_OK         : Demodulator locked
 @param pTSLockStat Indicates the TS lock condition.
        - 0: TS not locked.
        - 1: TS locked.
 @param pUnlockDetected Indicates an early unlock condition
        - 0: No early unlock.
        - 1: Early unlock detected.

 @return SONY_RESULT_OK if pSyncStat, pTSLockStat is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbc2_monitor_SyncStat (sony_demod_t * pDemod,
                                                 uint8_t * pSyncStat,
                                                 uint8_t * pTSLockStat,
                                                 uint8_t * pUnlockDetected);

/**
 @brief Monitor the demodulator *estimated* DVB-C2 TS (Transport Stream) rate.

 @param pDemod The demodulator instance.
 @param pTSRateKbps The estimated TS rate in kbps.

 @return SONY_RESULT_OK if pTSRateKbps is valid.
*/
sony_result_t sony_demod_dvbc2_monitor_TSRate (sony_demod_t * pDemod,
                                               uint32_t * pTSRateKbps);

/**
 @brief Monitor the DVB-C2 L1 signalling information. L1 signalling
        information is available when TS is locked, or earlier in the
        acquisition sequence, after the P1 symbols have been decoded.
        In the case of being tuned to a Data Slice with a broadband notch
        then no L1 signalling is available.

 @param pDemod The demodulator instance.
 @param l1Data Pointer to receive the L1 signalling information.

 @return SONY_RESULT_OK if the L1 Data is valid.
*/
sony_result_t sony_demod_dvbc2_monitor_L1Data (sony_demod_t * pDemod,
                                               sony_dvbc2_l1_t * l1Data);

/**
 @brief Monitor the DVB-C2 active PLP QAM. For multiple PLP systems,
        then able to monitor both the data and common PLP.
        For single PLP systems, only able to monitor the data PLP.

 @param pDemod The demodulator instance.
 @param type The type of the PLP to monitor.
 @param pQAM To receive the PLP QAM information into.

 @return SONY_RESULT_OK if pPLPInfo is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbc2_monitor_QAM (sony_demod_t * pDemod,
                                            sony_dvbc2_plp_type_t type,
                                            sony_dvbc2_constellation_t * pQAM);

/**
 @brief Monitor the DVB-C2 active PLP Code Length. For multiple PLP systems,
        then able to monitor both the data and common PLP. For single PLP
        systems, only able to monitor the data PLP.

 @param pDemod The demodulator instance.
 @param type The type of the PLP to monitor.
 @param pCodeLength To receive the PLP code length information into.

 @return SONY_RESULT_OK if pPLPInfo is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbc2_monitor_LDPCCodeLength (sony_demod_t * pDemod,
                                                       sony_dvbc2_plp_type_t type,
                                                       sony_dvbc2_ldpc_code_length_t * pCodeLength);

/**
 @brief Monitor the DVB-C2 active PLP Code Rate. For multiple PLP systems,
        then able to monitor both the data and common PLP. For single PLP
        systems, only able to monitor the data PLP.

 @param pDemod The demodulator instance.
 @param type The type of the PLP to monitor.
 @param pCodeRate To receive the PLP code rate information into.

 @return SONY_RESULT_OK if pPLPInfo is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbc2_monitor_CodeRate (sony_demod_t * pDemod,
                                                 sony_dvbc2_plp_type_t type,
                                                 sony_dvbc2_plp_code_rate_t * pCodeRate);

/**
 @brief Monitor the sampling frequency offset value.

 @param pDemod The demodulator instance.
 @param pPPM The sampling frequency offset in ppm x 100.
             Range: +/- 220ppm.

 @return SONY_RESULT_OK if pPPM is valid.
*/
sony_result_t sony_demod_dvbc2_monitor_SamplingOffset (sony_demod_t * pDemod,
                                                       int32_t * pPPM);

/**
 @brief Monitor the DVB-C2 PER (Packet Error Rate).

 @param pDemod The demod instance.
 @param pPER The estimated PER x 1e6.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbc2_monitor_PER (sony_demod_t * pDemod,
                                            uint32_t * pPER);

/**
 @brief Monitor the Guard Interval of the current tuned channel.

 @param pDemod The demodulator instance.
 @param pGI The guard interval.

 @return SONY_RESULT_OK if successful and pGI valid.
*/
sony_result_t sony_demod_dvbc2_monitor_GI (sony_demod_t * pDemod,
                                           sony_dvbc2_l1_gi_t * pGI);

#endif /* SONY_DEMOD_DVBC2_MONITOR_H */
