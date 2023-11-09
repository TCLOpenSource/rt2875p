/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
/**
 @file    sony_diseqc_command_1_2.h

          This file provides functions for generating "DiSEqC1.2" commands.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DISEQC_COMMAND_1_2_H
#define SONY_DISEQC_COMMAND_1_2_H

#include "sony_common.h"
#include "sony_diseqc_command.h"

/**
 @brief Mode for motor control command.

 This is used by sony_diseqc_command_DriveEast() and sony_diseqc_command_DriveWest().
*/
typedef enum {
    SONY_DISEQC_MOTOR_MODE_STEPPED, /**< amount is "step size". */
    SONY_DISEQC_MOTOR_MODE_TIMED,   /**< amount is "timeout time" in ms. */
    SONY_DISEQC_MOTOR_MODE_NONSTOP  /**< amount is ignored. */
} sony_diseqc_motor_mode_t;

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Create DiSEqC command to write Analogue value A0 (e.g. Skew).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.
 @param data Data to send.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_WriteA0 (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address,
                                           uint8_t data);

/**
 @brief Create DiSEqC command to write Analogue value A1.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.
 @param data Data to send.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_WriteA1 (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address,
                                           uint8_t data);

/**
 @brief Create DiSEqC command to stop Positioner movement.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_Halt (sony_diseqc_message_t * pMessage,
                                        sony_diseqc_framing_t framing,
                                        sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to disable Limits.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_LimitsOff (sony_diseqc_message_t * pMessage,
                                             sony_diseqc_framing_t framing,
                                             sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to set East Limit (& Enable recommended).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_LimitE (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to set West Limit (& Enable recommended).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_LimitW (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to drive Motor East (with optional timeout/steps).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.
 @param mode Motor mode.
 @param amount The amount of motor moving.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_DriveEast (sony_diseqc_message_t * pMessage,
                                             sony_diseqc_framing_t framing,
                                             sony_diseqc_address_t address,
                                             sony_diseqc_motor_mode_t mode,
                                             uint8_t amount);

/**
 @brief Create DiSEqC command to drive Motor West (with optional timeout/steps).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.
 @param mode Motor mode.
 @param amount The amount of motor moving.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_DriveWest (sony_diseqc_message_t * pMessage,
                                             sony_diseqc_framing_t framing,
                                             sony_diseqc_address_t address,
                                             sony_diseqc_motor_mode_t mode,
                                             uint8_t amount);

/**
 @brief Create DiSEqC command to store Satellite Position & Enable Limits.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.
 @param posNumber Position number.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_Storenn (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address,
                                           uint8_t posNumber);

/**
 @brief Create DiSEqC command to drive Motor to Satellite Position nn.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.
 @param posNumber Position number.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_Gotonn (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address,
                                          uint8_t posNumber);

/**
 @brief Create DiSEqC command to (re-) calculate Satellite Positions.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.
 @param pData Address of the data buffer.
 @param length Length of the data buffer.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetPosns (sony_diseqc_message_t * pMessage,
                                            sony_diseqc_framing_t framing,
                                            sony_diseqc_address_t address,
                                            uint8_t * pData,
                                            uint8_t length);

#endif /* SONY_DISEQC_COMMAND_1_2_H */
