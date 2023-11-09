/**
 * @file mid_misc.h
 * @brief This file implements misc header.
 * @date September.3.2014
 * @version V0.0.1
 * @author Martin Yan
 * @par Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#ifndef _MID_MISC_HEADER_INCLUDE_
#define _MID_MISC_HEADER_INCLUDE_

/**
 * @brief This function set demo window Enable
 * @param [in] unsigned char ubyWinID: 0~7
 * @param [in] BOOL bEnable
 * @retval [Out] true: success, false: fail
*/
BOOL Mid_MISC_SetDemoWindowEnable(unsigned char ubyWinID, BOOL bEnable);

/**
 * @brief This function get demo window Enable
 * @param [in] unsigned char ubyWinID
 * @retval  BOOL bEnable
*/
BOOL Mid_MISC_GetDemoWindowEnable(unsigned char ubyWinID);

/**
 * @brief This function set demo window mode
 * @param [in] BOOL bMode: 0: inside MEMC OFF; 1: outside MEMC OFF
 * @retval VOID
*/
VOID Mid_MISC_SetDemoWindowMode(BOOL bMode);

/**
 * @brief This function get demo window mode
 * @retval BOOL bMode: 0: inside MEMC OFF; 1: outside MEMC OFF
*/
BOOL Mid_MISC_GetDemoWindowMode(VOID);

/**
 * @brief This function set demo window settings
 * @param [in] unsigned char ubyWinID: 0~7
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @param [in] unsigned int u32Top 
 * @param [in] unsigned int u32Bottom 
 * @retval [Out] true: success, false: fail
*/
BOOL Mid_MISC_SetDemoWindowSettings(unsigned char ubyWinID, unsigned int u32Left, unsigned int u32Right, unsigned int u32Top, unsigned int u32Bottom);

/**
 * @brief This function get demo window settings
 * @param [in] unsigned char ubyWinID
 * @param [out] unsigned int u32Left
 * @param [out] unsigned int u32Right
 * @param [out] unsigned int u32Top 
 * @param [out] unsigned int u32Bottom 
 * @retval VOID
*/
#if 1  // k4lp
BOOL Mid_MISC_GetDemoWindowSettings(unsigned char ubyWinID, unsigned int *pu32Left, unsigned int *pu32Right, unsigned int *pu32Top, unsigned int *pu32Bottom);
#else
VOID Mid_MISC_GetDemoWindowSettings(unsigned char ubyWinID, unsigned int *pu32Left, unsigned int *pu32Right, unsigned int *pu32Top, unsigned int *pu32Bottom);
#endif

/**
 * @brief This function set input data mapping
 * @param [in] unsigned int u32Data_Mapping 
 * @retval VOID
*/
VOID Mid_MISC_SetInputDataMapping(unsigned int u32Data_Mapping);

/**
 * @brief This function get input data mapping
 * @param [in] VOID
 * @retval data mapping(range:0~7)
*/
unsigned int Mid_MISC_GetInputDataMapping(VOID);

/**
 * @brief This function set nr data mapping
 * @param [in] unsigned int u32Data_Mapping 
 * @retval VOID
*/
VOID Mid_MISC_SetNRDataMapping(unsigned int u32Data_Mapping);

/**
 * @brief This function get nr data mapping
 * @param [in] VOID
 * @retval data mapping(range:0~7)
*/
unsigned int Mid_MISC_GetNRDataMapping(VOID);

/**
 * @brief This function set output data mapping
 * @param [in] unsigned int u32Data_Mapping 
 * @retval VOID
*/
VOID Mid_MISC_SetOutputDataMapping(unsigned int u32Data_Mapping);

/**
 * @brief This function get output data mapping
 * @param [in] VOID
 * @retval data mapping(range:0~7)
*/
unsigned int Mid_MISC_GetOutputDataMapping(VOID);

/**
 * @brief This function set black insert line mode enable
 * @param [in] BOOL bEnable 
 * @retval VOID
*/
VOID Mid_MISC_SetBlackInsertLineModeEnable(BOOL bEnable);

/**
 * @brief This function set black insert line mode pattern number
 * @param [in] unsigned int u32PatternNum 
 * @retval VOID
*/
VOID Mid_MISC_SetBlackInsertLineModePatternNum(unsigned int u32PatternNum);

/**
 * @brief This function set black insert line mode pattern
 * @param [in] unsigned int u32Pattern 
 * @retval VOID
*/
VOID Mid_MISC_SetBlackInsertLineModeLPattern(unsigned int u32Pattern);

/**
 * @brief This function set black insert line mode pattern
 * @param [in] unsigned int u32Pattern 
 * @retval VOID
*/
VOID Mid_MISC_SetBlackInsertLineModeRPattern(unsigned int u32Pattern);

/**
 * @brief This function set black insert frame mode enable
 * @param [in] BOOL bEnable 
 * @retval VOID
*/
VOID Mid_MISC_SetBlackInsertFrameModeEnable(BOOL bEnable);

/**
 * @brief This function set input interrupt enable
 * @param [in] BOOL bEnable 
 * @param [in] u32IntType: 0 for H sync, 1 for V sync, 2 for Abnormal 
 * @retval VOID
*/
VOID Mid_MISC_SetInINTEnable(INT_TYPE enIntType, BOOL bEnable);
BOOL Mid_MISC_GetInINTEnable(INT_TYPE enIntType);

/**
 * @brief This function set in interrupt horizantal number
 * @param [in] unsigned int u32Hnum 
 * @retval VOID
*/
VOID Mid_MISC_SetInINTHnum(unsigned int u32Hnum);

/**
 * @brief This function set output interrupt enable
 * @param [in] BOOL bEnable 
 * @param [in] u32IntType: 0 for H sync, 1 for V sync
 * @retval VOID
*/
VOID Mid_MISC_SetOutINTEnable(INT_TYPE enIntType, BOOL bEnable);
BOOL Mid_MISC_GetOutINTEnable(INT_TYPE enIntType);

/**
 * @brief This function set output interrupt horizantal number
 * @param [in] unsigned int u32Hnum 
 * @retval VOID
*/
VOID Mid_MISC_SetOutINTHnum(unsigned int u32Hnum);

/**
 * @brief This function set post cursor enable
 * @param [in] BOOL bEnable 
 * @retval VOID
*/
VOID Mid_MISC_SetPostCursorEnable(BOOL bEnable);

/**
 * @brief This function set post cursor show
 * @param [in] BOOL bEnable 
 * @retval VOID
*/
VOID Mid_MISC_SetPostCursorShow(BOOL bEnable);

/**
 * @brief This function set post cursor position
 * @param [in] reg_post_cursor_h
 * @param [in] reg_post_cursor_v
 * @retval VOID
*/
VOID Mid_MISC_SetPostCursorPosition(unsigned int u32Hposition, unsigned int u32Vposition);

/**
 * @brief This function set post cursor width
 * @param [in] unsigned int u32Width(0~15)
 * @retval VOID
*/
VOID Mid_MISC_SetPostCursorWidth(unsigned int u32Width);

/**
 * @brief This function set post cursor color
 * @param [in] unsigned int u32Width(0~7)
 * @retval VOID
*/
VOID Mid_MISC_SetPostCursorColor(unsigned int u32Color);

/**
 * @brief This function Lock Interrupt
 * @param [in] INT_TYPE enIntType 
 * @retval VOID
*/
VOID Mid_MISC_IntINTLock(INT_TYPE enIntType);
/**
 * @brief This function unLock Interrupt
 * @param [in] INT_TYPE enIntType 
 * @retval VOID
*/
VOID Mid_MISC_IntINTUnLock(INT_TYPE enIntType);
/**
 * @brief This function Clear Interrupt
 * @param [in] INT_TYPE enIntType 
 * @retval VOID
*/
VOID Mid_MISC_IntINTCLear(INT_TYPE enIntType);
/**
 * @brief This function Lock Out Interrupt
 * @param [in] INT_TYPE enIntType 
 * @retval VOID
*/
VOID Mid_MISC_OutINTLock(INT_TYPE enIntType);
/**
 * @brief This function unLock Out Interrupt
 * @param [in] INT_TYPE enIntType 
 * @retval VOID
*/
VOID Mid_MISC_OutINTUnLock(INT_TYPE enIntType);
/**
 * @brief This function Clear Out Interrupt
 * @param [in] INT_TYPE enIntType 
 * @retval VOID
*/
VOID Mid_MISC_OutINTCLear(INT_TYPE enIntType);

/**
 * @brief This function Get in intType sts
 * @param [in] INT_TYPE enIntType 
 * @retval BOOL
*/
BOOL Mid_MISC_GetInINTStatus(INT_TYPE enIntType);
/**
 * @brief This function Get out intType sts
 * @param [in] INT_TYPE enIntType 
 * @retval BOOL
*/
BOOL Mid_MISC_GetOutINTStatus(INT_TYPE enIntType);

VOID Mid_MISC_SetInputIntp_en(BOOL bEnable);

VOID Mid_MISC_SetOnefiftIntp_en(BOOL bEnable);

VOID Mid_MISC_SetOutputIntp_en(BOOL bEnable);

#endif
