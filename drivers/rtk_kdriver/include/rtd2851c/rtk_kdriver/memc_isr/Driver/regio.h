/**
 * @file regio.h
 * @brief This header file defines register read/write api for other module to use.
 * @date August.29.2014
 * @version V0.0.1
 * @author Lixiangjun
 * @par Register Define V0.0.1 - Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#ifndef _REGISTER_IO_HEADER_
#define _REGISTER_IO_HEADER_

#include "memc_isr/include/kw_type.h"
//#include "memc_isr/Driver/KI7436_reg.h"
//#include "memc_isr/Driver/KI7576_reg.h"
#ifdef DLL_IMPLEMENT
#ifdef __cplusplus
#define DLL_API extern "C" __declspec(dllexport)
#else /* else of __cplusplus*/
#define DLL_API __declspec(dllexport)
#endif /* End of #ifdef __cplusplus */
#else /* else of DLL_IMPLEMENT*/
#ifdef __cplusplus
#define DLL_API extern "C" __declspec(dllimport)
#else /* else of __cplusplus */
#ifdef WIN32
#define DLL_API __declspec(dllimport)
#else /* else of WIN32 */
#define DLL_API
#endif/* End of #ifdef WIN32 */
#endif/* End of #ifdef __cplusplus */
#endif/* End of #ifdef DLL_IMPLEMENT */

#ifdef WIN32
typedef BOOL (__stdcall * ReadRegCallback)(unsigned int uiAddr, unsigned int *puiVal);
DLL_API BOOL RegIOReadRegSetCallback(ReadRegCallback cb);

typedef BOOL (__stdcall * WriteRegCallback)(unsigned int uiAddr, unsigned int uiVal);
DLL_API BOOL RegIOWriteRegSetCallback(WriteRegCallback cb);
#endif

/**
 * @brief This function read specified register and returns named register value, the value has been processed for specified named register.
 * @param uiAddr [in]		Named Register address, 32 bits
 * @param ibitStart [in]	Named Register start bit in register address
 * @param ibitEnd [in]		Named Register end bit in register address
 * @param puiVal [in/out]	Named Register value
 * @retval TRUE 			indicates success
 * @retval FALSE			indicates fail.
*/
DLL_API BOOL ReadRegister(unsigned int uiAddr, int ibitStart, int ibitEnd, unsigned int *puiVal);

/**
 * @brief This function write specified register of named register value. The value must do not move bits.
 * @param uiAddr [in]		Named Register address, 32 bits
 * @param ibitStart [in]	Named Register start bit in register address
 * @param ibitEnd [in]		Named Register end bit in register address
 * @param uiVal [in]		Named Register value want to write
 * @retval TRUE 			indicates success
 * @retval FALSE			indicates fail.
*/
DLL_API BOOL WriteRegister(unsigned int uiAddr, int ibitStart, int ibitEnd, unsigned int uiVal);

/**
 * @brief rtk register read
 * @param uiAddr [in]		Named Register address, 32 bits
 * @param puiVal [in/out]	Named Register value
 * @retval TRUE 			indicates success
 * @retval FALSE			indicates fail.
*/
DLL_API BOOL RTKReadRegister(unsigned int uiAddr, unsigned int *puiVal);

/**
 * @brief rtk register write
 * @param uiAddr [in]		Named Register address, 32 bits
 * @param puiVal [in/out]	Named Register value
 * @retval TRUE 			indicates success
 * @retval FALSE			indicates fail.
*/
DLL_API BOOL RTKWriteRegister(unsigned int uiAddr, unsigned int uiVal);

#endif

