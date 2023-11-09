/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/08/01
  Modification ID : b30d76210d343216ea52b88e9b450c8fd5c0359f
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_isdbc.h

          This file provides the demodulator control interface specific to ISDB-C.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_ISDBC_H
#define SONY_DEMOD_ISDBC_H

#include "sony_common.h"
#include "sony_demod.h"
#include "sony_isdbc.h"

/*------------------------------------------------------------------------------
  Enumerations
------------------------------------------------------------------------------*/
/**
 @brief TSMF output format.
*/
typedef enum {
    /**
     @brief Treat the input stream as not containing multiple TS. Pass through mode.
    */
    SONY_DEMOD_ISDBC_TSMF_MODE_SINGLE,
    /**
     @brief Treat the input stream as containing multiple TS. Output slot of the
            selected ID, other slots will be replaced to NULL.
    */
    SONY_DEMOD_ISDBC_TSMF_MODE_MULTIPLE,
    /**
     @brief Automatically select between the single mode and multiple mode.
            After meeting the following two conditions once, output format will be change
            Multiple mdoe.
            - TSMF header synchronization is locked.
            - CRC check is OK
    */
    SONY_DEMOD_ISDBC_TSMF_MODE_AUTO
} sony_demod_isdbc_tsmf_mode_t;

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief The tune parameters for a ISDB-C signal
*/
typedef struct sony_isdbc_tune_param_t {
    uint32_t centerFreqKHz;                  /**< Center frequency in kHz of the ISDB-C channel */
    sony_demod_isdbc_tsmf_mode_t tsmfMode;   /**< TSMF output format */
    /**
     @brief This parameter is used to tsId indicates whether TSID or relative TS number.
            If Relative TS number is selected networkId will be ignored.
     */
    sony_isdbc_tsid_type_t tsidType;
    uint16_t tsid;                           /**< TSID */
    uint16_t networkId;                      /**< Network ID */
} sony_isdbc_tune_param_t;

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Enable acquisition on the demodulator for ISDB-C channels.  Called from
        the integration layer ::sony_integ_isdbc_Tune API.

 @param pDemod  The demodulator instance
 @param pTuneParam The tune parameters.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbc_Tune (sony_demod_t * pDemod, sony_isdbc_tune_param_t * pTuneParam);

/**
 @brief Put the demodulator into ::SONY_DEMOD_STATE_SLEEP state.  Can be called
        from Active, Shutdown or Sleep states.  Called from the integration layer
        ::sony_integ_Sleep API.

 @param pDemod  The demodulator instance

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbc_Sleep (sony_demod_t * pDemod);

/**
 @brief Check ISDB-C TS lock status.

 @note  This API is for checking lock status in tuning stage.
        After tuning (while receiving the signal),
        please use sony_demod_isdbc_monitor_SyncStat instead
        to check current lock status.

 @param pDemod The demodulator instance
 @param pLock TS lock state

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbc_CheckTSLock (sony_demod_t * pDemod,
                                           sony_demod_lock_result_t * pLock);

#endif /* SONY_DEMOD_ISDBC_H */
