/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
/**
 @file    sony_diseqc_command_1_0.h

          This file provides functions for generating "DiSEqC1.0" commands.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DISEQC_COMMAND_1_0_H
#define SONY_DISEQC_COMMAND_1_0_H

#include "sony_common.h"
#include "sony_diseqc_command.h"

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Create DiSEqC command to reset DiSEqC microcontroller.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_Reset (sony_diseqc_message_t * pMessage,
                                         sony_diseqc_framing_t framing,
                                         sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to switch peripheral power supply off.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_Standby (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to switch peripheral power supply on.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_PowerOn (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select the Low Local Oscillator frequency.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetLo (sony_diseqc_message_t * pMessage,
                                         sony_diseqc_framing_t framing,
                                         sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command select Vertical Polarisation (or Right circular).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetVR (sony_diseqc_message_t * pMessage,
                                         sony_diseqc_framing_t framing,
                                         sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command select Satellite position A (or position C).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetPosA (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select Switch Option A (e.g. positions A/B).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetSOA (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select the High Local Oscillator frequency.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetHi (sony_diseqc_message_t * pMessage,
                                         sony_diseqc_framing_t framing,
                                         sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select Horizontal Polarisation (or Left circular).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetHL (sony_diseqc_message_t * pMessage,
                                         sony_diseqc_framing_t framing,
                                         sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select Satellite position B (or position D).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetPosB (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select Switch Option B (e.g. positions C/D).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetSOB (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to write to Port group 0 (Committed switches).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.
 @param data Data to send.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_WriteN0 (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address,
                                           uint8_t data);

#endif /* SONY_DISEQC_COMMAND_1_0_H */
