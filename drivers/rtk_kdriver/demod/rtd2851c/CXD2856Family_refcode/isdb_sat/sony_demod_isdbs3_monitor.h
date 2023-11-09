/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_isdbs3_monitor.h

          This file provides the ISDB-S3 demodulator monitor interface.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_ISDBS3_MONITOR_H
#define SONY_DEMOD_ISDBS3_MONITOR_H

#include "sony_demod.h"
#include "sony_isdbs3.h"

/*------------------------------------------------------------------------------
  Defines
------------------------------------------------------------------------------*/

#define SONY_DEMOD_ISDBS3_MONITOR_PRELDPCBER_INVALID 0xFFFFFFFF  /**< BER value if the BER value is invalid */
#define SONY_DEMOD_ISDBS3_MONITOR_PREBCHBER_INVALID  0xFFFFFFFF  /**< BER value if the BER value is invalid */
#define SONY_DEMOD_ISDBS3_MONITOR_POSTBCHFER_INVALID 0xFFFFFFFF  /**< FER value if the FER value is invalid */

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Monitors the synchronisation state of the demodulator.

 @param pDemod The demodulator instance.
 @param pAGCLockStat Indicates the AGC lock condition.
        - 0: AGC not locked.
        - 1: AGC locked.
 @param pTSTLVLockStat Indicates the TSTLV lock condition.
        - 0: TSTLV not locked.
        - 1: TSTLV locked.
 @param pTMCCLockStat Indicates TMCC lock condition.
        - 0: TMCC not locked.
        - 1: TMCC locked.

 @return SONY_RESULT_OK if pAGCLockStat, pTSTLVLockStat and pTMCCLockStat is valid,
         otherwise an error is returned..
*/
sony_result_t sony_demod_isdbs3_monitor_SyncStat (sony_demod_t * pDemod,
                                                  uint8_t * pAGCLockStat,
                                                  uint8_t * pTSTLVLockStat,
                                                  uint8_t * pTMCCLockStat);

/**
 @brief Monitors the detected carrier offset of the currently tuned channel.

        To get the estimated center frequency of the current channel:
        Freq_Est = Freq_Tune + pOffset;

        This function will compensate for an inverting tuner architecture if the
        demodulator has been configured accordignly using ::SONY_DEMOD_CONFIG_SPECTRUM_INV
        config option for ::sony_demod_SetConfig.

 @param pDemod The demodulator instance.
 @param pOffset The detected carrier offset in KHz.

 @return SONY_RESULT_OK if pOffset is valid.
*/
sony_result_t sony_demod_isdbs3_monitor_CarrierOffset (sony_demod_t * pDemod,
                                                       int32_t * pOffset);

/**
 @brief Monitor the IFAGC value of ISDB-S3 demodulator.
        Actual dB gain dependent on attached tuner.

 @param pDemod The demodulator instance.
 @param pIFAGCOut The IFAGC output register value. Range 0 - 0x1FFF. Unitless.

 @return SONY_RESULT_OK if pIFAGC is valid.
*/
sony_result_t sony_demod_isdbs3_monitor_IFAGCOut (sony_demod_t * pDemod,
                                                  uint32_t * pIFAGCOut);

/**
 @brief Monitor the I/Q sense of ISDB-S3 demodulator.

 @param pDemod The demodulator instance.
 @param pSense The detected I/Q sense.

 @return SONY_RESULT_OK if pSense is valid.
*/
sony_result_t sony_demod_isdbs3_monitor_IQSense (sony_demod_t * pDemod,
                                                 sony_demod_sat_iq_sense_t * pSense);

/**
 @brief Monitor the CNR estimation made by the demodulator.

 @param pDemod The demodulator instance.
 @param pCNR The returned CNR in dB x 1000.

 @return SONY_RESULT_OK if pCNR is valid.
*/
sony_result_t sony_demod_isdbs3_monitor_CNR (sony_demod_t * pDemod,
                                             int32_t * pCNR);

/**
 @brief Monitor pre LDPC BER for each layer.

 @param pDemod The demodulator instance.
 @param pBERH The estimated high hierarchy BER * 10^7.
 @param pBERL The estimated low hierarchy BER * 10^7.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_monitor_PreLDPCBER (sony_demod_t * pDemod,
                                                    uint32_t * pBERH, uint32_t * pBERL);

/**
 @brief Monitor pre BCH BER for each layer.

 @param pDemod The demodulator instance.
 @param pBERH The estimated high hierarchy BER * 10^9.
 @param pBERL The estimated low hierarchy BER * 10^9.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_monitor_PreBCHBER (sony_demod_t * pDemod,
                                                   uint32_t * pBERH, uint32_t * pBERL);

/**
 @brief Monitor post BCH FER for each layer.

 @param pDemod The demodulator instance.
 @param pFERH The estimated high hierarchy FER * 10^6.
 @param pFERL The estimated low hierarchy FER * 10^6.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_monitor_PostBCHFER (sony_demod_t * pDemod,
                                                    uint32_t * pFERH, uint32_t * pFERL);

/**
  @brief Monitor the TMCC information

  @param pDemod    The demod instance.
  @param pTMCCInfo TMCC information struct instance.

  @return SONY_RESULT_OK if successful and pTMCCInfo is valid.
*/
sony_result_t sony_demod_isdbs3_monitor_TMCCInfo (sony_demod_t * pDemod,
                                                  sony_isdbs3_tmcc_info_t * pTMCCInfo);

/**
 @brief Monitor the modulation, code rate of high/low hierarchies in currently tuned stream.

 @param pDemod  The demodulator instance.
 @param pModH The high hierarchy modulation.
 @param pModL The low hierarchy modulation.
 @param pCodH The high hierarchy code rate.
 @param pCodL The low hierarchy code rate.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_monitor_ModCod (sony_demod_t * pDemod,
                                                sony_isdbs3_mod_t * pModH, sony_isdbs3_mod_t * pModL,
                                                sony_isdbs3_cod_t * pCodH, sony_isdbs3_cod_t * pCodL);

/**
 @brief Monitor the modulation, code rate and slot number of high/low hierarchies in currently tuned stream.

 @note  This monitor uses TMCC information by ::sony_demod_isdbs3_monitor_TMCCInfo.

 @param pDemod The demodulator instance.
 @param pTmccInfo TMCC information struct instance.
 @param pModH The high hierarchy modulation.
 @param pModL The low hierarchy modulation.
 @param pCodH The high hierarchy code rate.
 @param pCodL The low hierarchy code rate.
 @param pSlotNumH The high hierarchy slot number.
 @param pSlotNumL The low hierarchy slot number.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_monitor_CurrentStreamModCodSlotNum (sony_demod_t *pDemod, sony_isdbs3_tmcc_info_t * pTmccInfo,
                                                                    sony_isdbs3_mod_t * pModH, sony_isdbs3_mod_t * pModL,
                                                                    sony_isdbs3_cod_t * pCodH, sony_isdbs3_cod_t * pCodL,
                                                                    uint8_t * pSlotNumH, uint8_t * pSlotNumL);

/**
 @brief Monitor the site diversity information in TMCC information.

        ISDB-S3 TMCC information includes 3 bit length uplink control information.
        This monitor returns MSB of the unlink control information.
        (Instructions for site diversity performing frame. Please refer to STD-B44.)
        If whole uplink control information is necessary,
        please use ::sony_demod_isdbs3_monitor_TMCCInfo.

 @param pDemod The demodulator instance.
 @param pSiteDiversityInfo The site diversity information in TMCC. (0 or 1)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_monitor_SiteDiversityInfo (sony_demod_t * pDemod,
                                                           uint8_t * pSiteDiversityInfo);


/**
 @brief Monitor the Emergency Warning Broadcasting System(EWS) detection flag.

 @param pDemod The demodulator instance.
 @param pEWSChange The value of EWS change detection flag.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_monitor_EWSChange (sony_demod_t * pDemod,
                                                   uint8_t * pEWSChange);

/**
 @brief Monitor the TMCC information change detection flag.

 @param pDemod The demodulator instance.
 @param pTMCCChange The value of TMCC change detection flag.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_monitor_TMCCChange (sony_demod_t * pDemod,
                                                    uint8_t * pTMCCChange);

/**
 @brief Clear the Emergency Warning Broadcasting System(EWS) detection flag.

 @param pDemod The demodulator instance.
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_ClearEWSChange (sony_demod_t * pDemod);


/**
 @brief Clear the TMCC Change detection flag.

 @param pDemod The demodulator instance.
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_ClearTMCCChange (sony_demod_t * pDemod);

/**
 @brief Monitor the unmatch flag between specified StreamID and recieved StreamID.

 @param pDemod The demodulator instance.
 @param pStreamIDError The value of StreamID unmatch flag.
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs3_monitor_StreamIDError (sony_demod_t * pDemod, uint8_t * pStreamIDError);

#endif /* SONY_DEMOD_ISDBS3_MONITOR_H */
