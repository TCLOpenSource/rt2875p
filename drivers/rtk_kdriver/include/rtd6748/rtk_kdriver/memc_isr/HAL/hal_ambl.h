/**
 * @file hal_amb.c
 * @brief This file is for amblight register settings.
 * @date Dec.18.2014
 * @version V0.0.1
 * @author Martin Yan
 * @par MEMC Library V0.0.1 - Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#ifndef _HAL_OMET_HEADER_
#define _HAL_OMET_HEADER_

#include "memc_isr/Common/kw_common.h"
#include "memc_isr/Driver/regio.h"

/**
 * @brief This function get amblight R,G,B APL value.
 * @param [in] u16Length data length, maximum length is 640(32*20).
 * @param [out] pubyAPL_R	R APL value.
 * @param [out] pubyAPL_G	G APL value.
 * @param [out] pubyAPL_B	B APL value.
*/
VOID HAL_AMBL_GetAPL(unsigned short u16Length, unsigned char *pubyAPL_R, unsigned char *pubyAPL_G, unsigned char *pubyAPL_B);

/**
 * @brief This function set amblight enable or disable
 * @param [in] bEnalbe enable or disable amblight
*/
VOID HAL_AMBL_SetEnable(BOOL bEnalbe);

/**
 * @brief This function set amblight cell size
 * @param [in] u32Length cell length
 * @param [in] u32Length cell width
*/
VOID HAL_AMBL_SetCellSize(unsigned int u32Width, unsigned int u32Length);

/**
 * @brief This function set amblight size
 * @param [in] u32Length length
 * @param [in] u32Length width
*/
VOID HAL_AMBL_SetSize(unsigned int u32Width, unsigned int u32Length);

/**
 * @brief This function set amblight multiply
 * @param [in] u32Mutpl
*/
VOID HAL_AMBL_SetMutpl(unsigned int u32Mutpl);

/**
 * @brief This function get amblight shift
 * @param [out] u32Shift
*/
unsigned int HAL_AMBL_GetShift(VOID);

/**
 * @brief This function set amblight Frq
 * @param [in] u32Frq
*/
VOID HAL_AMBL_SetFrq(unsigned int u32Frq);

#endif
