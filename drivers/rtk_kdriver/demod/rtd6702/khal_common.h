
/******************************************************************************
 *	 DTV LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 1999 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file hal_common.h
 *
 *	Common Used Type Definitions.
 *
 *	@author 	ieeum.lee
 *	@version	1.0
 *	@date	2013. 5. 22
 *	@note	common Used Type Def. extracted from common.h
 *	@see
 */

/******************************************************************************
	Header File Guarder
******************************************************************************/

#ifndef _HAL_COMMON_H_
#define _HAL_COMMON_H_

//#include <stdio.h>
//#include <stdlib.h>
//#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


// Common type definitions

#ifndef UINT8
typedef	unsigned char			__UINT8;
#define UINT8 __UINT8
#endif

#ifndef UINT08
typedef	unsigned char			__UINT08;
#define UINT08 __UINT08
#endif

#ifndef SINT8
typedef	signed char				__SINT8;
#define SINT8 __SINT8
#endif

#ifndef SINT08
typedef	signed char				__SINT08;
#define SINT08 __SINT08
#endif

#ifndef CHAR
typedef	char					__CHAR;
#define CHAR __CHAR
#endif

#ifndef UINT16
typedef	unsigned short			__UINT16;
#define UINT16 __UINT16
#endif

#ifndef SINT16
typedef	signed short			__SINT16;
#define SINT16 __SINT16
#endif

#ifndef UINT32
typedef	unsigned int			__UINT32;
#define UINT32 __UINT32
#endif

#ifndef SINT32
typedef signed int				__SINT32;
#define SINT32 __SINT32
#endif

#ifndef BOOLEAN
#ifndef _EMUL_WIN
typedef	unsigned int			__BOOLEAN;
#define BOOLEAN __BOOLEAN
#else
typedef	unsigned char		__BOOLEAN;
#define BOOLEAN __BOOLEAN
#endif
#endif

#ifndef ULONG
typedef unsigned long			__ULONG;
#define ULONG __ULONG
#endif

#ifndef SLONG
typedef signed long				__SLONG;
#define SLONG __SLONG
#endif

#ifndef UINT64
typedef	unsigned long long		__UINT64;
#define UINT64 __UINT64
#endif

#ifndef SINT64
typedef	signed long long		__SINT64;
#define SINT64 __SINT64
#endif



#ifndef TRUE
#define TRUE					(1)
#endif

#ifndef FALSE
#define FALSE					(0)
#endif

#ifndef ON_STATE
#define ON_STATE				(1)
#endif

#ifndef OFF_STATE
#define OFF_STATE				(0)
#endif

#ifndef ON
#define ON						(1)
#endif

#ifndef OFF
#define OFF						(0)
#endif

#ifndef NULL
#define NULL					((void *)0)
#endif


#ifndef API_STATE_T

/**
 *	API return type definitions
 *
 *  \- API_OK				: success (no error) <P>
 *	\- API_ERROR			: generic error <P>
 *	\- API_INVALID_PARAMS	: input parameter error <P>
 *	\- API_NOT_SUPPORTED	: not supported <P>
 *	\- API_NOT_PERMITTED	: no permission <P>
 *	\- API_TIMEOUT			: time-out <P>
 */
typedef enum
{
	API_OK   					=	0,
	OK							=	0,
	API_ERROR     				=	-1,
	API_NOT_OK					= 	-1,
	NOT_OK						=	-1,
	PARAMETER_ERROR				=   -2,
	API_INVALID_PARAMS			=	-2,
	INVALID_PARAMS				=	-2,
	API_NOT_ENOUGH_RESOURCE		=	-3,
	NOT_ENOUGH_RESOURCE			=	-3,
	API_NOT_SUPPORTED			=	-4,
	NOT_SUPPORTED				=	-4,
	API_NOT_PERMITTED			=	-5,
	NOT_PERMITTED				=	-5,
	API_TIMEOUT					=	-6,
	TIMEOUT						=	-6,
	NO_DATA_RECEIVED			=	-7,
	DN_BUF_OVERFLOW				=	-8,
	DLNA_NOT_CONNECTED			=	-9,  		/**< DLNA Á¢¼Ó ¿¡·¯ ¹ß»ý½Ã*/
	UP_BUF_OVERFLOW				=	-10,
}	_API_STATE_T;


#define API_STATE_T	_API_STATE_T
#endif

typedef API_STATE_T		DTV_STATUS;
typedef API_STATE_T		DTV_STATUS_T;

typedef struct HAL_OUT_ENUM{
	int    PreEnum; //opcode
	int    PostEnum; // packet size
} sHAL_OUT_ENUM;

#ifdef __cplusplus
}
#endif

/* below definitions should be removed ? */

#endif  /* _HAL_COMMON_H_ */
/*****************************************************************************************
 * SIC R&D LAB, LG ELECTRONICS INC., SEOUL, KOREA
 * COPYRIGHT(c) 2011,2012 by LG Electronics Inc.
 *
 * All rights reserved. No part of this work covered by this copyright hereon
 * may be reproduced, stored in a retrieval system, in any form
 * or by any means, electronic, mechanical, photocopying, recording
 * or otherwise, without the prior written  permission of LG Electronics.
 ****************************************************************************************/

/** @file
 *
 *  Brief description.
 *  Detailed description starts here.
 *
 *  @author     wonsik.do
 *  @version    1.0
 *  @date       2013-05-21
 *  @note       Additional information.
 */

#ifndef	__HAL_COMMON_H__
#define	__HAL_COMMON_H__

/*----------------------------------------------------------------------------------------
	Control Constants
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
    File Inclusions
----------------------------------------------------------------------------------------*/


#ifdef	__cplusplus
extern "C"
{
#endif /* __cplusplus */

/*----------------------------------------------------------------------------------------
	Constant Definitions
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
	Macro Definitions
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
    Type Definitions
----------------------------------------------------------------------------------------*/
#if 0
typedef enum HAL_STATUS_T_
{
	HAL_OK					= 0, 	/**< success (no error) */
	HAL_NOT_OK				= -1,	/**< generic error */
	HAL_INVALID_PARAMS		= -2, 	/**< input parameter error */
	HAL_NOT_ENOUGH_RESOURCE	= -3,	/**< not enough resource */
	HAL_NOT_SUPPORTED		= -4,	/**< not supported */
	HAL_NOT_PERMITTED		= -5,	/**< not permitted */
	HAL_TIMEOUT				= -6,	/**< timeout */
	HAL_NO_DATA_RECEIVED	= -7,	/**< no data received */
	HAL_DN_BUF_OVERFLOW 	= -8	/**< buffer overflow error */
} HAL_STATUS_T;
#endif

typedef enum
{
	HAL_BOARD_PT_1ST	= 0,
	HAL_BOARD_PT_2ND	= 1,
	HAL_BOARD_PT_3RD	= 2,
	HAL_BOARD_PT_4TH	= 3,
	HAL_BOARD_PT_5TH	= 4,

	HAL_BOARD_DV_1ST	= 5,
	HAL_BOARD_DV_2ND	= 6,
	HAL_BOARD_DV_3RD	= 7,

	HAL_BOARD_PV_1ST	= 8,
	HAL_BOARD_PV_2ND	= 9,
	HAL_BOARD_PV_3RD	= 10,

	HAL_BOARD_PQ_1ST	= 11,
	HAL_BOARD_PQ_2ND	= 12,
	HAL_BOARD_PQ_3RD	= 13,

	HAL_BOARD_MP_1ST	= 14,
	HAL_BOARD_MP_2ND	= 15,

	HAL_NUM_OF_EVENT_BOARD
} HAL_EVENT_BOARD_T;

typedef struct HAL_Status_Flag {
	int Initialized;
	int Opened;
	int	Connected;
} HAL_Status_Flag_t;
/*----------------------------------------------------------------------------------------
	Extern Function Prototype Declaration
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
	Extern Variables
----------------------------------------------------------------------------------------*/

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif /* _PROTOS_H_ */

