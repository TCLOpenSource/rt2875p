#if 0
/**
 * @file memc_type.h
 * @brief This header file defines memc data type.
 * @date Septempber.3.2014
 * @version V0.0.1
 * @author MartinYan
 * @par KW type trace V0.0.1 - Copyright (C) 2014 Kiwi-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#include "memc_isr/include/kw_type.h"

#ifndef _MEMC_TYPE_HEADER_
#define _MEMC_TYPE_HEADER_

/*! Input signal resolution enum. */
typedef enum 
{
	INPUT_RESOLUTION_1920X1080,					///< 1920X1080 resolution
	INPUT_RESOLUTION_1920X540,					///< 1920X540  resolution
	INPUT_RESOLUTION_3840X2160,					///< 3840x2160 resolution
	INPUT_RESOLUTION_3840X1080,					///< 3840x2160 resolution
	INPUT_RESOLUTION_3840X540,					///< 3840X540 resolution
	INPUT_RESOLUTION_1920X2160,					///< 1920X2160 resolutio
	INPUT_RESOLUTION_768X480,					///< 768x480 resolution
	INPUT_RESOLUTION_1280X720,					///< 1280x720 resolution
	
	INPUT_RESOLUTION_INVALID	= 0xFF,
}INPUT_RESOLUTION;

/*! Output signal resolution enum. */
typedef enum 
{
	OUTPUT_RESOLUTION_1920X1080,				///< 1920x1080 resolution
	OUTPUT_RESOLUTION_1920X540,					///< 1920x540  resolution
	OUTPUT_RESOLUTION_3840X2160,				///< 3840x2160 resolution
	OUTPUT_RESOLUTION_3840X1080,				///< 3840x1080 resolution
	OUTPUT_RESOLUTION_3840X540,					///< 3840X540 resolution
	OUTPUT_RESOLUTION_768X480,					///< 768x480 resolution
	OUTPUT_RESOLUTION_1280X720,					///< 1280x720 resolution
	
	OUTPUT_RESOLUTION_INVALID	= 0xFF,
}OUTPUT_RESOLUTION;

/*! Input signal frame rate enum. */
typedef enum 
{
	INPUT_FRAMERATE_24HZ,						///< input 24Hz
	INPUT_FRAMERATE_25HZ,						///< input 25Hz
	INPUT_FRAMERATE_30HZ,						///< input 30Hz

	INPUT_FRAMERATE_48HZ,						///< input 48Hz
	INPUT_FRAMERATE_50HZ,						///< input 50Hz
	INPUT_FRAMERATE_60HZ,						///< input 60Hz
	INPUT_FRAMERATE_96HZ,						///< input 96Hz	
	INPUT_FRAMERATE_100HZ,						///< input 100Hz
	INPUT_FRAMERATE_120HZ,						///< input 120Hz

	INPUT_FRAMERATE_INVALID		= 0xFF,
}INPUT_FRAMERATE;

/*! Output signal frame rate enum. */
typedef enum 
{
	OUTPUT_FRAMERATE_48HZ,						///< output 48Hz
	OUTPUT_FRAMERATE_50HZ,						///< output 50Hz
	OUTPUT_FRAMERATE_60HZ,						///< output 60Hz
	OUTPUT_FRAMERATE_96HZ,						///< output 96Hz
	OUTPUT_FRAMERATE_100HZ,						///< output 100Hz
	OUTPUT_FRAMERATE_120HZ,						///< output 120Hz
	OUTPUT_FRAMERATE_200HZ,						///< output 200Hz
	OUTPUT_FRAMERATE_240HZ,						///< output 240Hz
	OUTPUT_FRAMERATE_FPGA30HZ,					///< output fpga 4k 30hz
	
	OUTPUT_FRAMERATE_MAX,
	OUTPUT_FRAMERATE_INVALID	= 0xFF,
}OUTPUT_FRAMERATE;

/*! Input format enum. */
typedef enum 
{
	INPUT_FORMAT_2D,							///< memc input is 2D
	INPUT_FORMAT_3D_SBS,						///< memc input is 3D SBS
	INPUT_FORMAT_3D_TB,							///< memc input is 3D TB
	INPUT_FORMAT_3D_LR,							///< memc input is 3D LR
	INPUT_FORMAT_3D_LBL,						///< memc input is 3D LBL

	INPUT_FORMAT_INVALID		= 0xFF,
}INPUT_FORMAT;

/*! Output format enum. */
typedef enum 
{
	OUTPUT_FORMAT_2D,							///< memc output is 2D
	OUTPUT_FORMAT_3D_SG,						///< memc output is 3DSG
	OUTPUT_FORMAT_3D_PR,						///< memc output is 3DPR
	OUTPUT_FORMAT_3D_SG_LLRR,					///< memc output is 3DSG LLRR

	OUTPUT_FORMAT_INVALID		= 0xFF,
}OUTPUT_FORMAT;

/*! Output mode enum. */
typedef enum 
{
	RGB_IN_VIDEO_OUT,							///< memc RGB in and video output
	RGB_IN_PC_OUT,								///< memc RGB in and pc output
	YUV_IN_VIDEO_OUT, 							///< memc YUV444 in and video output
	YUV_IN_PC_OUT,								///< memc YUV444 in and pc output
	INOUT_MODE_INVALID			= 0xFF,
}INOUT_MODE;

/*! define timing structure */
typedef struct
{
	unsigned int					u32HTotal;			///< H total
	unsigned int					u32HActive;			///< H active
	unsigned int					u32HSWidth;			///< H sync width
	unsigned int					u32HSBPorch;		///< H sync backporch
	unsigned int					u32VTotal;			///< V total
	unsigned int					u32VActive;			///< V active
	unsigned int					u32VSWidth;			///< V sync width
	unsigned int					u32VSBPorch;		///< V sync backporch
}TIMING_PARA_ST;

/*! MEMC mode enum. */
typedef enum 
{
	MEMC_MODE_SMOOTH,							///< Smooth mode
	MEMC_MODE_CLEAR,							///< Clear mode
	MEMC_MODE_NORMAL,							///< Normal mode
	MEMC_MODE_OFF,								///< Off mode
	MEMC_MODE_USER,								///< User mode, can manually set dejudder and deblur level
}MEMC_MODE_EN;

/*! MEMC RIM mode. */
typedef enum 
{
	MEMC_RIM_MODE_0,						///< memc rim mode 0
	MEMC_RIM_MODE_1,						///< memc rim mode 1
}RIM_MODE;

/*! Post horizontal scaler ratio. */
typedef enum 
{
	POST_HSC_BYPASS	= 1,
	POST_HSC_X2		= 2,
	POST_HSC_X4		= 4,
}POST_SCALER_RATIO_H;

/*! Post vertical scaler ratio. */
typedef enum 
{
	POST_VSC_BYPASS	= 1,
	POST_VSC_X2		= 2,
	POST_VSC_X4		= 4,
}POST_SCALER_RATIO_V;

/*! define Panel timing structure */
typedef struct 
{
	OUTPUT_FRAMERATE	enOutputFrameRate;	// Output frame rate
	unsigned int				u32Htotal;			// H total
	unsigned int				u32HtotalMax;		// H total max
	unsigned int				u32HtotalMin;		// H total min
	unsigned int				u32Hactive;			// H active
	unsigned int				u32HSWidth;			// H sync width
	unsigned int				u32HSBPorch;		// H back porch
	unsigned int				u32Vtotal;			// V total
	unsigned int				u32VtotalMax;		// V total max
	unsigned int				u32VtotalMin;		// V total min
	unsigned int				u32Vactive;			// V active
	unsigned int				u32VSWidth;			// V sync width
	unsigned int				u32VSBPorch;		// V back porch
}PANEL_PARAMETERS_ST;

/*! define Panel data structure */
typedef struct 
{
	unsigned int				u32ID;					// ID
	unsigned int				u32Version;				// Version
	unsigned int				b2Type		:2;			// 0:2D,1:SG,2:PR
	unsigned int				b2TXBitNum	:2;			// 0:8 bit,1:10 bit,2:12 bit
	unsigned int				b2TXMode	:2;			// 0:VESA,1:JEIDA
	unsigned int				reserve     :26;		// reserve
	unsigned int				u32PanelPara_Idx;		// panel parameters idx
	PANEL_PARAMETERS_ST	stPanelParamertes[OUTPUT_FRAMERATE_MAX];	// Panel parameters
}PANEL_DATA_ST;

/*! Sample Format. */
typedef enum
{
	SAMPLE_FORMAT_444,
	SAMPLE_FORMAT_422,
	SAMPLE_FORMAT_MAX
}CS_SAMPLE_FORMAT;

/*! CSC Mode*/
typedef enum
{
	CSC_MODE_RGB_YUV_601_FULL, //=0
	CSC_MODE_YUV_RGB_601_FULL,
	CSC_MODE_RGB_YUV_709_FULL,
	CSC_MODE_YUV_RGB_709_FULL,
	CSC_MODE_RGB_YUV_601_LIMIT,
	CSC_MODE_YUV_RGB_601_LIMIT,
	CSC_MODE_RGB_YUV_709_LIMIT,
	CSC_MODE_YUV_RGB_709_LIMIT,
	CSC_MODE_USER_DEFINE,
}CSC_MODE;

/*! CSC Mode*/
typedef enum
{
	INT_TYPE_HSYNC,
	INT_TYPE_VSYNC,
	INT_TYPE_ABNORMAL,
}INT_TYPE;

/*! DM Buff address*/
typedef struct
{
	unsigned int u32StartAddress;
	unsigned int u32EndAddress;
}DM_BUF_ADDRESS;

/*!DM Start Address Struct*/
typedef struct
{
	unsigned int u32ME_StartAddress;
	unsigned int u32MV_StartAddress;
	unsigned int u32MC_StartAddress;
}DM_Start_ADDRESS_ST;

/*!MC DM Address Struct*/
typedef struct 
{
	DM_BUF_ADDRESS stBuf_HF_0;
	DM_BUF_ADDRESS stBuf_HF_1;
	DM_BUF_ADDRESS stBuf_HF_2;
	DM_BUF_ADDRESS stBuf_HF_3;
	DM_BUF_ADDRESS stBuf_HF_4;
	DM_BUF_ADDRESS stBuf_HF_5;
	DM_BUF_ADDRESS stBuf_HF_6;
	DM_BUF_ADDRESS stBuf_HF_7;
	
	DM_BUF_ADDRESS stBuf_LF_0;
	DM_BUF_ADDRESS stBuf_LF_1;
	DM_BUF_ADDRESS stBuf_LF_2;
	DM_BUF_ADDRESS stBuf_LF_3;
	DM_BUF_ADDRESS stBuf_LF_4;
	DM_BUF_ADDRESS stBuf_LF_5;
	DM_BUF_ADDRESS stBuf_LF_6;
	DM_BUF_ADDRESS stBuf_LF_7;
	
	unsigned int 		   u32LineOffset_VDHF;
	unsigned int		   u32LROffset_VDHF;
	unsigned int		   u32LineOffset_VDLF;
	unsigned int		   u32LROffset_VDLF;

	unsigned int		   u32LineOffset_PC;
	unsigned int		   u32LROffset_PC;
}MC_DM_ADDRESS_ST;

/*!ME DM Address Struct*/
typedef struct 
{
	DM_BUF_ADDRESS stBuf_00_0;
	DM_BUF_ADDRESS stBuf_00_1;
	DM_BUF_ADDRESS stBuf_00_2;
	DM_BUF_ADDRESS stBuf_00_3;
	DM_BUF_ADDRESS stBuf_00_4;
	DM_BUF_ADDRESS stBuf_00_5;
	DM_BUF_ADDRESS stBuf_00_6;
	DM_BUF_ADDRESS stBuf_00_7;
	unsigned int 		   u32Buf_00_LineOffset;
	unsigned int		   u32Buf_00_LROffset;

	DM_BUF_ADDRESS stBuf_08_0;
	DM_BUF_ADDRESS stBuf_08_1;
	unsigned int 		   u32Buf_08_LineOffset;

	DM_BUF_ADDRESS stBuf_10_0;
	DM_BUF_ADDRESS stBuf_10_1;
	unsigned int 		   u32Buf_10_LineOffset;

	DM_BUF_ADDRESS stBuf_12_0;
	DM_BUF_ADDRESS stBuf_12_1;
	unsigned int 		   u32Buf_12_LineOffset;
	
	DM_BUF_ADDRESS stBuf_14_0;
	DM_BUF_ADDRESS stBuf_14_1;	
	unsigned int 		   u32Buf_14_LineOffset;
}ME_DM_ADDRESS_ST;

/*!MV DM Address Struct*/
typedef struct 
{
	DM_BUF_ADDRESS stBuf_01_0;
	unsigned int 		   u32Buf_01_LineOffset;
	unsigned int		   u32Buf_01_LROffset;
	
	DM_BUF_ADDRESS stBuf_02_0;
	unsigned int 		   u32Buf_02_LineOffset;
	unsigned int		   u32Buf_02_LROffset;

	DM_BUF_ADDRESS stBuf_04_0;
	DM_BUF_ADDRESS stBuf_04_1;
	DM_BUF_ADDRESS stBuf_04_2;
	unsigned int 		   u32Buf_04_LineOffset;
	unsigned int		   u32Buf_04_LROffset;
	
	DM_BUF_ADDRESS stBuf_05_0;
	DM_BUF_ADDRESS stBuf_05_1;
	unsigned int 		   u32Buf_05_LineOffset;
	unsigned int		   u32Buf_05_LROffset;	

	DM_BUF_ADDRESS stBuf_12_0;
	DM_BUF_ADDRESS stBuf_12_1;
	DM_BUF_ADDRESS stBuf_12_2;
	DM_BUF_ADDRESS stBuf_12_3;
	DM_BUF_ADDRESS stBuf_12_4;
	DM_BUF_ADDRESS stBuf_12_5;
	DM_BUF_ADDRESS stBuf_12_6;
	DM_BUF_ADDRESS stBuf_12_7;
	unsigned int 		   u32Buf_12_LineOffset;
	unsigned int		   u32Buf_12_LROffset;	
}MV_DM_ADDRESS_ST;

#endif /*! End of #ifndef _REAL_TYPE_HEADER_ */


#else
/**
 * @file kw_type.h
 * @brief This header file defines Real-Image data type and common used macro.
 * @date August.24.2014
 * @version V0.0.1
 * @author Lixiangjun
 * @par KW type trace V0.0.1 - Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#ifndef _REAL_TYPE_HEADER_
#define _REAL_TYPE_HEADER_

/*! Define data type */
#if 1

/*! Define data type */
//typedef char				SCHAR;				///< signed SCHAR, 8 bits
//typedef short				SINT16;				///< define short as SINT16
//typedef int					SINT32;				///< define long as SINT32

//typedef unsigned char		UBYTE;				///< 8 bits unsigned data
//#ifndef UINT16
//typedef unsigned short		UINT16;				///< 16 bits unsigned data
//#endif
//#ifndef UINT32
//typedef unsigned int			UINT32;				///< define int as UINT32
//#endif

#else

#include <base_types.h>

#ifndef SINT32
#define SINT32	INT32
#endif

#ifndef SINT16
#define SINT16	INT16
#endif

#ifndef SCHAR
#define SCHAR	INT8
#endif

#ifndef UBYTE
#define UBYTE	UINT8
#endif

#endif

#ifndef BOOL
#ifdef WIN32
typedef int					BOOL;
#else
typedef unsigned char		BOOL;				///< define BOOL as unsigned char 8 bits
#endif
#endif

#ifndef VOID
typedef void				VOID;				///< define VOID
#endif

/*! define null pointer */
/*! c++ NULL define is different with c */
#ifndef NULL
#ifndef __cplusplus
#define NULL				((VOID *)0)
#else
#define NULL				0
#endif /* End of #ifndef __cplusplus */
#endif /* End of #ifndef NULL */

/*! define TRUE */
#ifndef TRUE
#define TRUE				1
#endif/* End of #ifndef TRUE */

/*! define FALSE */
#ifndef FALSE
#define FALSE				0
#endif/* End of #ifndef FALSE */

/*! define BIT0~BIT31 */
#define BIT_0		0x00000001	///< define BIT_0
#define BIT_1 		0x00000002	///< define BIT_1
#define BIT_2 		0x00000004	///< define BIT_2
#define BIT_3 		0x00000008	///< define BIT_3
#define BIT_4 		0x00000010	///< define BIT_4
#define BIT_5 		0x00000020	///< define BIT_5
#define BIT_6 		0x00000040	///< define BIT_6
#define BIT_7 		0x00000080	///< define BIT_7
#define BIT_8 		0x00000100	///< define BIT_8
#define BIT_9 		0x00000200	///< define BIT_9
#define BIT_10 		0x00000400	///< define BIT_10
#define BIT_11 		0x00000800	///< define BIT_11
#define BIT_12 		0x00001000	///< define BIT_12
#define BIT_13 		0x00002000	///< define BIT_13
#define BIT_14 		0x00004000	///< define BIT_14
#define BIT_15 		0x00008000	///< define BIT_15
#define BIT_16 		0x00010000	///< define BIT_16
#define BIT_17 		0x00020000	///< define BIT_17
#define BIT_18 		0x00040000	///< define BIT_18
#define BIT_19 		0x00080000	///< define BIT_19
#define BIT_20 		0x00100000	///< define BIT_20
#define BIT_21 		0x00200000	///< define BIT_21
#define BIT_22 		0x00400000	///< define BIT_22
#define BIT_23 		0x00800000	///< define BIT_23
#define BIT_24 		0x01000000	///< define BIT_24
#define BIT_25 		0x02000000	///< define BIT_25
#define BIT_26 		0x04000000	///< define BIT_26
#define BIT_27 		0x08000000	///< define BIT_27
#define BIT_28 		0x10000000	///< define BIT_28
#define BIT_29 		0x20000000	///< define BIT_29
#define BIT_30 		0x40000000	///< define BIT_30
#define BIT_31 		0x80000000	///< define BIT_31

#endif /* End of #ifndef _REAL_TYPE_HEADER_ */


#endif
