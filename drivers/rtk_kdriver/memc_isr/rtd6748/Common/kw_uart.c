#if 1
/**
 * @file common.c
 * @brief This file implements common stdio/memory/string library function. These function only support MEMC library.
 * @date August.24.2014
 * @version V0.0.1
 * @author Lixiangjun
 * @par MEMC Library V0.0.1 - Copyright (C) 2014 Kiwi-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/
#include "memc_isr/Common/kw_common.h"
#ifdef WIN32
#include <stdio.h>
#endif

/**
 * @brief This exported function output one character through uart. Please provide this function for debug.
 * @param [in] ch		Character want to output
*/
VOID Uart_Printf(char ch)
{
	//printf("%c", ch);
}

/**
 * @brief This exported function receive strings through uart. This function does not block, it gratch data from buffer then return. Please provide this function for debug.
 * @param [in] pcBuf		String buffer for received string
 * @param [in] iBufSize		Buf size including string termiante 0 byte.
 * @return Real string length received
*/
int Uart_Receive(char *pcBuf, int iBufSize) {
	return 0;
}


#else
/**
 * @file common.c
 * @brief This file implements common stdio/memory/string library function. These function only support MEMC library.
 * @date August.24.2014
 * @version V0.0.1
 * @author Lixiangjun
 * @par MEMC Library V0.0.1 - Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/
#include "memc_isr/Common/kw_common.h"
#ifdef WIN32
#include <stdio.h>
#endif

/**
 * @brief This exported function output one character through uart. Please provide this function for debug.
 * @param [in] ch		Character want to output
*/
VOID Uart_Printf(char ch)
{
	//printf("%c", ch);
}

/**
 * @brief This exported function receive strings through uart. This function does not block, it gratch data from buffer then return. Please provide this function for debug.
 * @param [in] pcBuf		String buffer for received string
 * @param [in] iBufSize		Buf size including string termiante 0 byte.
 * @return Real string length received
*/
int Uart_Receive(char *pcBuf, int iBufSize)
{
	return 0;
}


#endif
