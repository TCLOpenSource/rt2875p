/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
/**
 @file    sony_diseqc_command_2_0.h

          This file provides functions for generating "DiSEqC2.0" commands.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DISEQC_COMMAND_2_0_H
#define SONY_DISEQC_COMMAND_2_0_H

#include "sony_common.h"
#include "sony_diseqc_command.h"

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Create DiSEqC command to clear the "Reset" flag.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_ClrReset (sony_diseqc_message_t * pMessage,
                                            sony_diseqc_framing_t framing,
                                            sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to set Contention flag.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_SetContend (sony_diseqc_message_t * pMessage,
                                              sony_diseqc_framing_t framing,
                                              sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to return address only if Contention flag is set.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_Contend (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to clear Contention flag.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_ClrContend (sony_diseqc_message_t * pMessage,
                                              sony_diseqc_framing_t framing,
                                              sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to return address unless Contention flag is set.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_Address (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to change address only if Contention flag is set.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.
 @param data Data to send.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_MoveC (sony_diseqc_message_t * pMessage,
                                         sony_diseqc_framing_t framing,
                                         sony_diseqc_address_t address,
                                         uint8_t data);

/**
 @brief Create DiSEqC command to change address unless Contention flag is set.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.
 @param data Data to send.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_Move (sony_diseqc_message_t * pMessage,
                                        sony_diseqc_framing_t framing,
                                        sony_diseqc_address_t address,
                                        uint8_t data);

/**
 @brief Create DiSEqC command to read Status register flags.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_Status (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to read Configuration flags (peripheral hardware).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_Config (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to read Switching state flags (Committed port).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_Switch0 (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to read Switching state flags (Uncommitted port).

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_Switch1 (sony_diseqc_message_t * pMessage,
                                           sony_diseqc_framing_t framing,
                                           sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to read Analogue value A0.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_ReadA0 (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to read Analogue value A1.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_ReadA1 (sony_diseqc_message_t * pMessage,
                                          sony_diseqc_framing_t framing,
                                          sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to read current frequency [Reply = BCD string].

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_LOstring (sony_diseqc_message_t * pMessage,
                                            sony_diseqc_framing_t framing,
                                            sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to read current frequency table entry number.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_LOnow (sony_diseqc_message_t * pMessage,
                                         sony_diseqc_framing_t framing,
                                         sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to read Lo frequency table entry number.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_LOLo (sony_diseqc_message_t * pMessage,
                                        sony_diseqc_framing_t framing,
                                        sony_diseqc_address_t address);

/**
 @brief Create DiSEqC command to read Hi frequency table entry number.

 @param pMessage The message instance.
 @param framing Framing byte.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_diseqc_command_LOHi (sony_diseqc_message_t * pMessage,
                                        sony_diseqc_framing_t framing,
                                        sony_diseqc_address_t address);

#endif /* SONY_DISEQC_COMMAND_2_0_H */
