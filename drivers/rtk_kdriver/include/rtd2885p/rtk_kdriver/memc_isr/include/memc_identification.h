/**
 * @file memc_lib.h
 * @brief This header file defines MEMC lib exported apis and data type.
 * @date August.9.2014
 * @version V0.0.2
 * @author Martin/Lixiangjun
 * @par MEMC Library V0.0.2 - Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.2
*/

#ifndef _MEMC_LIB_HEADER_INCLUDE_
#define _MEMC_LIB_HEADER_INCLUDE_

//#include "kw_common.h"
#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/memc_type.h"

/*memc isr monitor*/
#define CONFIG_MEMC_ISR_CNT_MAX 400
#define CONFIG_MEMC_IN_ISR_CNT_MAX CONFIG_MEMC_ISR_CNT_MAX
#define CONFIG_MEMC_OUT_ISR_CNT_MAX CONFIG_MEMC_ISR_CNT_MAX

#ifdef WIN32
#include "regio.h"
#endif

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

#define _ZERO                       0x00
#define _BIT0                       0x01
#define _BIT1                       0x02
#define _BIT2                       0x04
#define _BIT3                       0x08
#define _BIT4                       0x10
#define _BIT5                       0x20
#define _BIT6                       0x40
#define _BIT7                       0x80
#define _BIT8                       0x0100
#define _BIT9                       0x0200
#define _BIT10                      0x0400
#define _BIT11                      0x0800
#define _BIT12                      0x1000
#define _BIT13                      0x2000
#define _BIT14                      0x4000
#define _BIT15                      0x8000
#define _BIT16                      0x10000
#define _BIT17                      0x20000
#define _BIT18                      0x40000
#define _BIT19                      0x80000
#define _BIT20                      0x100000
#define _BIT21                      0x200000
#define _BIT22                      0x400000
#define _BIT23                      0x800000
#define _BIT24                      0x1000000
#define _BIT25                      0x2000000
#define _BIT26                      0x4000000
#define _BIT27                      0x8000000
#define _BIT28                      0x10000000
#define _BIT29                      0x20000000
#define _BIT30                      0x40000000
#define _BIT31                      0x80000000

void MEMC_info_identifiaction_TV001(void);
void MEMC_info_identifiaction_TV010(void);
unsigned char MEMC_ID_GetFlag(unsigned short prj,unsigned short nNumber);
unsigned char MEMC_info_compare(const short *vector1, const short *vector2, unsigned char length);

#endif


