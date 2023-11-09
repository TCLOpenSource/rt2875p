/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/09/02
  Modification ID : 4de34284da29630b3c6ce40872f48f739b35e390
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_isdbs_monitor.h

          This file provides the ISDB-S demodulator monitor interface.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_ISDBS_MONITOR_H
#define SONY_DEMOD_ISDBS_MONITOR_H

#include "sony_demod.h"
#include "sony_isdbs.h"

/*------------------------------------------------------------------------------
  Defines
------------------------------------------------------------------------------*/

#define SONY_DEMOD_ISDBS_MONITOR_PRERSBER_INVALID  0xFFFFFFFF  /**< BER value if the BER value is invalid */
#define SONY_DEMOD_ISDBS_MONITOR_PER_INVALID       0xFFFFFFFF  /**< PER value if the PER value is invalid */

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Monitors the synchronisation state of the demodulator.

 @param pDemod The demodulator instance.
 @param pAGCLockStat Indicates the AGC lock condition.
        - 0: AGC not locked.
        - 1: AGC locked.
 @param pTSLockStat Indicates the TS lock condition.
        - 0: TS not locked.
        - 1: TS locked.
 @param pTMCCLockStat Indicates TMCC lock condition.
        - 0: TMCC not locked.
        - 1: TMCC locked.

 @return SONY_RESULT_OK if pAGCLockStat, pTSLockStat and pTMCCLockStat is valid,
         otherwise an error is returned..
*/
sony_result_t sony_demod_isdbs_monitor_SyncStat (sony_demod_t * pDemod,
                                                 uint8_t * pAGCLockStat,
                                                 uint8_t * pTSLockStat,
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
sony_result_t sony_demod_isdbs_monitor_CarrierOffset (sony_demod_t * pDemod,
                                                      int32_t * pOffset);

/**
 @brief Monitor the IFAGC value of ISDB-S demodulator.
        Actual dB gain dependent on attached tuner.

 @param pDemod The demodulator instance.
 @param pIFAGCOut The IFAGC output register value. Range 0 - 0x1FFF. Unitless.

 @return SONY_RESULT_OK if pIFAGC is valid.
*/
sony_result_t sony_demod_isdbs_monitor_IFAGCOut (sony_demod_t * pDemod,
                                                 uint32_t * pIFAGCOut);

/**
 @brief Monitor the pre-RS BER for each hierarchies and TMCC.

        If the target BER is invalid, SONY_DEMOD_ISDBS_MONITOR_PRERSBER_INVALID (0xFFFFFFFF) is set.

 @param pDemod   The demodulator instance.
 @param pBERH    The estimated high hierarchy BER x 1e7.
 @param pBERL    The estimated low hierarchy BER x 1e7.
 @param pBERTMCC The estimated TMCC BER x 1e7.

 @return SONY_RESULT_OK if pBER is valid.
*/
sony_result_t sony_demod_isdbs_monitor_PreRSBER (sony_demod_t * pDemod,
                                                 uint32_t * pBERH, uint32_t * pBERL, uint32_t * pBERTMCC);

/**
 @brief Monitor the CNR estimation made by the demodulator.

 @param pDemod The demodulator instance.
 @param pCNR The returned CNR in dB x 1000.

 @return SONY_RESULT_OK if pCNR is valid.
*/
sony_result_t sony_demod_isdbs_monitor_CNR (sony_demod_t * pDemod,
                                            int32_t * pCNR);


/**
 @brief Monitor the ISDB-S PER (Packet Error Rate).

        If the target PER is invalid, SONY_DEMOD_ISDBS_MONITOR_PER_INVALID (0xFFFFFFFF) is set.

 @param pDemod The demod instance.
 @param pPERH The estimated high hierarchy PER value (Post reed solomon decoder) x 1e6.
 @param pPERL The estimated low hierarchy PER value (Post reed solomon decoder) x 1e6.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs_monitor_PER (sony_demod_t * pDemod,
                                            uint32_t * pPERH, uint32_t * pPERL);

/**
  @brief Monitor the TMCC information

  @param pDemod    The demod instance.
  @param pTMCCInfo TMCC information struct instance.

  @return SONY_RESULT_OK if successful and pTMCCInfo is valid.
*/
sony_result_t sony_demod_isdbs_monitor_TMCCInfo (sony_demod_t*           pDemod,
                                                 sony_isdbs_tmcc_info_t * pTMCCInfo);

/**
 @brief Monitor the modulation, code rate in TMCC information.

 @param pDemod  The demodulator instance.
 @param pModCodH The high hierarchy modulation and the code rate.
 @param pModCodL The low hierarchy modulation and the code rate.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs_monitor_ModCod (sony_demod_t * pDemod,
                                               sony_isdbs_modcod_t * pModCodH, sony_isdbs_modcod_t * pModCodL);

/**
 @brief Monitor the slot number in TMCC information.

 @param pDemod The demodulator instance.
 @param pSlotNumH The high hierarchy slot number.
 @param pSlotNumL The low hierarchy slot number.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs_monitor_SlotNum (sony_demod_t * pDemod,
                                                uint8_t * pSlotNumH, uint8_t * pSlotNumL);

/**
 @brief Monitor the site diversity infomation in TMCC information.

 @param pDemod The demodulator instance.
 @param pSiteDiversityInfo The site diversity infomation in TMCC.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs_monitor_SiteDiversityInfo (sony_demod_t * pDemod,
                                                          uint8_t * pSiteDiversityInfo);


/**
 @brief Monitor the Emergency Warning Broadcasting System(EWS) detection flag.

 @param pDemod The demodulator instance.
 @param pEWSChange The value of EWS change detection flag.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs_monitor_EWSChange (sony_demod_t * pDemod,
                                                  uint8_t * pEWSChange);

/**
 @brief Monitor the TMCC information change detection flag.

 @param pDemod The demodulator instance.
 @param pTMCCChange The value of TMCC change detection flag.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs_monitor_TMCCChange (sony_demod_t * pDemod,
                                                   uint8_t * pTMCCChange);

/**
 @brief Clear the Emergency Warning Broadcasting System(EWS) detection flag.

 @param pDemod The demodulator instance.
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs_ClearEWSChange (sony_demod_t * pDemod);


/**
 @brief Clear the TMCC Change detection flag.

 @param pDemod The demodulator instance.
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs_ClearTMCCChange (sony_demod_t * pDemod);

/**
 @brief Monitor the unmatch flag between specified TSID and recieved TSIDs.

 @param pDemod The demodulator instance.
 @param pTSIDError The value of TSID unmatch flag.
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbs_monitor_TSIDError (sony_demod_t * pDemod, uint8_t * pTSIDError);

#endif /* SONY_DEMOD_ISDBS_MONITOR_H */
