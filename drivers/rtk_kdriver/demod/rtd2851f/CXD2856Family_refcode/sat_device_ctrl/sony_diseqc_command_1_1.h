/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
/**
 @file    sony_diseqc_command_1_1.h

          This file provides functions for generating "DiSEqC1.1" commands.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DISEQC_COMMAND_1_1_H
#define SONY_DISEQC_COMMAND_1_1_H

#include "sony_common.h"
#include "sony_diseqc_command.h"

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Create DiSEqC command to select switch S1 input A (deselect input B).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetS1A (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select switch S2 input A (deselect input B).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetS2A (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select switch S3 input A (deselect input B).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetS3A (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select switch S4 input A (deselect input B).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetS4A (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select switch S1 input B (deselect input A).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetS1B (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select switch S2 input B (deselect input A).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetS2B (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select switch S3 input B (deselect input A).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetS3B (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to select switch S4 input B (deselect input A).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetS4B (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to write to Port group 1 (Uncommitted switches).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.
 @param data Data to send.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_WriteN1 (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address,
                                           uint8_t data);

/**
 @brief Create DiSEqC command to write channel frequency (BCD string).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.
 @param frequency Frequency in KHz.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_WriteFreq (sony_diseqc_message_t * pMessage,
                                             sony_diseqc_framing_t framing,
                                             sony_diseqc_address_t address,
                                             uint32_t frequency);

#endif /* SONY_DISEQC_COMMAND_1_1_H */
