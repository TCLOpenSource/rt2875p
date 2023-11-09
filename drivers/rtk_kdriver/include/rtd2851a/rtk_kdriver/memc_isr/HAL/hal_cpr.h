/**
 * @file hal_cpr.h
  * @brief This file is for cpr register setting
  * @date Nov.18.2014
  * @version V0.0.1
  * @author Martin Yan
  * @par Copyright (C) 2014 Real-Image Co. LTD
  * @par History:
  * 		 Version: 0.0.1
 */

#ifndef _HAL_CPR_HEADER_INCLUDE_
#define _HAL_CPR_HEADER_INCLUDE_

#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/memc_type.h"

/**
 * @brief This function set cpr h/v active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_CPR_SetActive(unsigned int u32Hactive, unsigned int u32Vactive);

/**
 * @brief This function set encode 3d enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_CPR_SetEncode3DEnable(BOOL bEnable);

/**
 * @brief This function set encode 3d mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_CPR_SetEncode3DMode(unsigned int u32Mode);

/**
 * @brief This function set decode 3d enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_CPR_SetDecode3DEnable(BOOL bEnable);

/**
 * @brief This function set decode 3d mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_CPR_SetDecode3DMode(unsigned int u32Mode);

/**
 * @brief This function set 422 enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_CPR_Set422Enable(BOOL bEnable);

/**
 * @brief This function set budget packet
 * @param [in] unsigned int u32Budget_Packet
 * @retval VOID
  * @ 2015-08-17, GZ asked, to seperate Hf, Lf param
*/
VOID HAL_CPR_SetBudgetPacket(unsigned int u32Budget_Packet_Lf, unsigned int u32Budget_Packet_Hf);


/**
 * @brief This function set last line enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_CPR_SetLastLineEnable(BOOL bEnable);

/**
 * @brief This function set LR inverse
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_CPR_SetLRInverse(BOOL bEnable);

#endif
