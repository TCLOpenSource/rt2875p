/*------------------------------------------------------------------------------
  Copyright 2016-2019 Sony Semiconductor Solutions Corporation

  Last Updated    : 2019/06/28
  Modification ID : 03e95bfeb910b454772d71d7172ca87e80bdcd01
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbs_s2_monitor.h

          This file provides the DVB-S/S2 demodulator monitor interface.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DEMOD_DVBS_S2_MONITOR_H
#define SONY_DEMOD_DVBS_S2_MONITOR_H

#include "sony_common.h"
#include "sony_demod.h"

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/

/**
 @brief Monitors the synchronisation state of the DVB-S/S2 demodulator.

 @param pDemod The demodulator instance.
 @param pTSLockStat Indicates the TS lock condition.
        - 0: TS not locked.
        - 1: TS locked.

 @return SONY_RESULT_OK if pTSLockStat is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbs_s2_monitor_SyncStat (sony_demod_t * pDemod,
                                                   uint8_t * pTSLockStat);

/**
 @brief Monitor the carrier offset of the currently tuned channel.
        To get the estimated center frequency of the current channel:
        Fest = Ftune (KHz) + pOffset (KHz) ;

 @param pDemod The demodulator instance.
 @param pOffset The detected carrier offset in KHz.

 @return SONY_RESULT_OK if pOffset is valid.
*/
sony_result_t sony_demod_dvbs_s2_monitor_CarrierOffset (sony_demod_t * pDemod,
                                                        int32_t * pOffset);

/**
 @brief Monitor the IFAGC value of the DVB-S/S2 demodulator.
        Actual dB gain dependent on attached tuner.

 @param pDemod The demodulator instance.
 @param pIFAGC The IFAGC output register value. Range 0 - 0xFFF. Unitless.

 @return SONY_RESULT_OK if pIFAGC is valid.
*/
sony_result_t sony_demod_dvbs_s2_monitor_IFAGCOut (sony_demod_t * pDemod,
                                                   uint32_t * pIFAGC);

/**
 @brief Monitor the System which current receiving.

 @param pDemod The demodulator instance.
 @param pSystem The system of current receiving signal.

 @return SONY_RESULT_OK if pSystem is valid.
*/
sony_result_t sony_demod_dvbs_s2_monitor_System (sony_demod_t * pDemod,
                                                 sony_dtv_system_t * pSystem);

/**
 @brief Monitor the symbol rate of the currently tuned channel in Sps (Symbols per Second).

 @param pDemod The demodulator instance.
 @param pSymbolRateSps The symbol rate in Sps (Symbols per Second).

 @return SONY_RESULT_OK if pSymbolRateSps is valid.
*/
sony_result_t sony_demod_dvbs_s2_monitor_SymbolRate (sony_demod_t * pDemod,
                                                     uint32_t * pSymbolRateSps);

/**
 @brief Monitors the I/Q sense used by the DVB-S/S2 demodulator.

 @param pDemod The demodulator instance.
 @param pSense The detected I/Q sense.

 @return SONY_RESULT_OK if successful and pSense is valid.
*/
sony_result_t sony_demod_dvbs_s2_monitor_IQSense (sony_demod_t * pDemod,
                                                  sony_demod_sat_iq_sense_t * pSense);

/**
 @brief Monitor the CNR estimation made by the demodulator.

 @param pDemod The demodulator instance.
 @param pCNR The returned CNR in dB x 1000.

 @return SONY_RESULT_OK if pCNR is valid.
*/
sony_result_t sony_demod_dvbs_s2_monitor_CNR (sony_demod_t * pDemod,
                                              int32_t * pCNR);

/**
 @brief Monitor PER.

 @param pDemod The demodulator instance.
 @param pPER The returned PER in 1e6.

 @return SONY_RESULT_OK if pPER is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbs_s2_monitor_PER (sony_demod_t * pDemod,
                                              uint32_t * pPER);

/**
 @brief Monitor the DVB-S Code Rate.

 @param pDemod The demodulator instance.
 @param pCodeRate Code rate.

 @return SONY_RESULT_OK if pCodeRate is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbs_monitor_CodeRate (sony_demod_t * pDemod,
                                                sony_dvbs_coderate_t * pCodeRate);

/**
 @brief Monitor pre viterbi BER.

 @param pDemod The demodulator instance.
 @param pBER The returned BER * 10^7.

 @return SONY_RESULT_OK if pBER is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbs_monitor_PreViterbiBER (sony_demod_t * pDemod,
                                                     uint32_t * pBER);

/**
 @brief Monitor pre RS BER.

 @param pDemod The demodulator instance.
 @param pBER The returned BER * 10^7.

 @return SONY_RESULT_OK if pBER is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbs_monitor_PreRSBER (sony_demod_t * pDemod,
                                                uint32_t * pBER);

/**
 @brief Monitor the DVB-S2 PLS code.

 @param pDemod The demodulator instance.
 @param pPLSCode The returned PLS code. The IC does not support short frames,
                 so sony_dvbs2_plscode_t::isShortFrame is always set to 0:normal.

 @return SONY_RESULT_OK if pPLSCode is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbs2_monitor_PLSCode (sony_demod_t * pDemod,
                                                sony_dvbs2_plscode_t * pPLSCode);

/**
 @brief Monitor pre LDPC BER.

 @param pDemod The demodulator instance.
 @param pBER The returned BER * 10^7.

 @return SONY_RESULT_OK if pBER is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbs2_monitor_PreLDPCBER (sony_demod_t * pDemod,
                                                   uint32_t * pBER);

/**
 @brief Monitor pre BCH BER.

 @param pDemod The demodulator instance.
 @param pBER The returned BER * 10^9.

 @return SONY_RESULT_OK if pBER is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbs2_monitor_PreBCHBER (sony_demod_t * pDemod,
                                                  uint32_t * pBER);

/**
 @brief Monitor post BCH FER.

 @param pDemod The demodulator instance.
 @param pFER The returned FER * 10^6.

 @return SONY_RESULT_OK if pFER is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbs2_monitor_PostBCHFER (sony_demod_t * pDemod,
                                                   uint32_t * pFER);

/**
 @brief Monitor DVB-S2 base-band Header (BBHEADER).

 @param pDemod The demodulator instance.
 @param pBBHeader The returned base-band Header (BBHEADER) information.

 @return SONY_RESULT_OK if pBBHeader is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbs2_monitor_BBHeader (sony_demod_t * pDemod,
                                                 sony_dvbs2_bbheader_t * pBBHeader);

/**
 @brief Monitor DVB-S/S2 scan information.

        This API is called from BlindScan API.

 @param pDemod The demodulator instance.
 @param pTSLock Indicates the TS lock condition.
        - 0: TS not locked.
        - 1: TS locked.
 @param pOffset Carrier offset frequency (KHz).
        - If pTSLock = 0, this value is invalid.
        Please refer to sony_demod_dvbs_s2_monitor_CarrierOffset.
 @param pSystem The system of current receiving signal.
        - If pTSLock = 0, this value is invalid.

 @return SONY_RESULT_OK if all values are valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbs_s2_monitor_ScanInfo (sony_demod_t * pDemod,
                                                   uint8_t * pTSLock,
                                                   int32_t * pOffset,
                                                   sony_dtv_system_t * pSystem);

/**
 @brief Check pilot on/off

        This API is called from BlindScan API.

 @param pDemod         The demodulator instance
 @param pPlscLock      Address of PLSC lock status.
 @param pPilotOn       Address of Pilot flag (0:Off, 1:On)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbs_s2_monitor_Pilot (sony_demod_t * pDemod,
                                                uint8_t * pPlscLock,
                                                uint8_t * pPilotOn);

/**
 @brief Monitor TS bit rate in Kbps.

 @param pDemod The demodulator instance.
 @param pTSRateKbps The returned TS bit rate in Kbps.

 @return SONY_RESULT_OK if successful and pTSRateKbps valid.
*/
sony_result_t sony_demod_dvbs_s2_monitor_TSRate (sony_demod_t * pDemod,
                                                 uint32_t * pTSRateKbps);

/**
 @brief Monitor Rolloff parameter.

 @param pDemod The demodulator instance.
 @param pRolloff The returned pRolloff.

 @return SONY_RESULT_OK if pRolloff is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_dvbs2_monitor_Rolloff (sony_demod_t * pDemod,
                                                uint8_t * pRolloff);

#endif /* SONY_DEMOD_DVBS2_MONITOR_H */
