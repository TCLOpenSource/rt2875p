/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_singlecable2_command.h

          This file provides the function to generate "single cable2 commands".
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_SINGLECABLE2_COMMAND_H
#define SONY_SINGLECABLE2_COMMAND_H

#include "sony_common.h"
#include "sony_demod_sat_device_ctrl.h"

/*------------------------------------------------------------------------------
 Enumerations
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Create "ODU_Channel_change" command, addressing a Single Outdoor Unit.  This is used
        to retune a specific user band to a new centre frequency.

 @param pMessage The message instance.
 @param userbandId Userband-ID (1 - 32)
 @param ifFreqMHz IF frequency in MHz.(0 - 9 is special value.)
 @param uncommittedSwitches Switch selection for uncommitted switch.
 @param committedSwitches Switch selection for committed switch.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable2_command_ODU_Channel_change (sony_diseqc_message_t * pMessage,
                                                            uint8_t userbandId,
                                                            uint32_t ifFreqMHz,
                                                            uint8_t uncommittedSwitches,
                                                            uint8_t committedSwitches);

/**
 @brief Create "ODU_Channel_change_PIN" command, addressing a Multi-Dwelling Unit.  This is used
        to retune a specific user band to a new centre frequency.

 @param pMessage The message instance.
 @param userbandId Userband-ID (1 - 32)
 @param ifFreqMHz IF frequency in MHz.(0 - 9 is special value.)
 @param uncommittedSwitches Switch selection for uncommitted switch.
 @param committedSwitches Switch selection for committed switch.
 @param pin PIN code.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable2_command_ODU_Channel_change_PIN (sony_diseqc_message_t * pMessage,
                                                                uint8_t userbandId,
                                                                uint32_t ifFreqMHz,
                                                                uint8_t uncommittedSwitches,
                                                                uint8_t committedSwitches,
                                                                uint8_t pin);

/**
 @brief Create "ODU_PowerOFF" command, addressing a Single Outdoor Unit.  This is used to reduce
        power consumption and release a user band when the corresponding demodulator is turned off.

 @param pMessage The message instance.
 @param userbandId Userband-Id (1 - 32)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable2_command_ODU_PowerOFF (sony_diseqc_message_t * pMessage, uint8_t userbandId);

/**
 @brief Create "ODU_PowerOFF_PIN" command, addressing a Multi-Dwelling Unit.  This is used to reduce
        power consumption and release a user band when the corresponding demodulator is turned off.

 @param pMessage The message instance.
 @param userbandId Userband-Id (1 - 32)
 @param pin PIN code.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable2_command_ODU_PowerOFF_PIN (sony_diseqc_message_t * pMessage,
                                                          uint8_t userbandId,
                                                          uint8_t pin);

/**
 @brief Create "ODU_UB_avail" command.

 @param pMessage The message instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable2_command_ODU_UB_avail (sony_diseqc_message_t * pMessage);

/**
 @brief Create "ODU_UB_freq" command.

 @param pMessage The message instance.
 @param userbandId Userband-Id (1 - 32)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable2_command_ODU_UB_freq (sony_diseqc_message_t * pMessage,
                                                     uint8_t userbandId);

/**
 @brief Create "ODU_UB_inuse" command.

 @param pMessage The message instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable2_command_ODU_UB_inuse (sony_diseqc_message_t * pMessage);

/**
 @brief Create "ODU_UB_PIN" command.

 @param pMessage The message instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable2_command_ODU_UB_PIN (sony_diseqc_message_t * pMessage);

/**
 @brief Create "ODU_UB_switches" command.

 @param pMessage The message instance.
 @param userbandId Userband-Id (1 - 32)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable2_command_ODU_UB_switches (sony_diseqc_message_t * pMessage,
                                                         uint8_t userbandId);

#endif /* SONY_SINGLECABLE_COMMAND_H */
