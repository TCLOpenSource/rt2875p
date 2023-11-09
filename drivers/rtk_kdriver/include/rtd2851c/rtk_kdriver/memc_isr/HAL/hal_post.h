/**
 * @file hal_post.h
  * @brief This file is for post register setting
  * @date Nov.14.2014
  * @version V0.0.1
  * @author Martin Yan
  * @par Copyright (C) 2014 Real-Image Co. LTD
  * @par History:
  * 		 Version: 0.0.1
 */

#ifndef _HAL_POST_HEADER_INCLUDE_
#define _HAL_POST_HEADER_INCLUDE_

#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/memc_type.h"

/**
 * @brief This function set post pattern size
 * @param [in] TIMING_PARA_ST stOutputTiming
 * @retval VOID
*/
VOID HAL_POST_SetPatternSize(TIMING_PARA_ST stOutputTiming);

/**
 * @brief This function set post pr mode enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetPREnable(BOOL bEnable);

/**
 * @brief This function set post hde center value
 * @param [in] unsigned int u32CenterValue
 * @retval VOID
*/
VOID HAL_POST_SetHdeCenterValue(unsigned int u32CenterValue);

/**
 * @brief This function set post 4 port hde center value
 * @param [in] unsigned int u32CenterValue
 * @retval VOID
*/
VOID HAL_POST_Set4PortHdeCenterValue(unsigned int u32CenterValue);

/**
 * @brief This function set v scaler bypass
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetVscalerBypass(BOOL bEnable);

/**
 * @brief This function set v scaler mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_POST_SetVscalerMode(unsigned int u32Mode);

/**
 * @brief This function set v active
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_POST_SetVActive(unsigned int u32Vactive);

/**
 * @brief This function set h scaler bypass
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetHscalerBypass(BOOL bEnable);

/**
 * @brief This function set h scaler mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_POST_SetHscalerMode(unsigned int u32Mode);

/**
 * @brief This function set h scaler offset
 * @param [in] unsigned int u32Offset
 * @retval VOID
*/
VOID HAL_POST_SetHscalerOffset(unsigned int u32Offset);

/**
 * @brief This function set h scaler coef
 * @param [in] unsigned int u32Coef
 * @retval VOID
*/
VOID HAL_POST_SetHscalerCoef(unsigned int u32Coef);

/**
 * @brief This function set h scaler coef1
 * @param [in] unsigned int u32Coef
 * @retval VOID
*/
VOID HAL_POST_SetHscalerCoef1(unsigned int u32Coef);

/**
 * @brief This function set h scaler mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_POST_SetHscalerMode_C(unsigned int u32Mode);

/**
 * @brief This function set h scaler offset
 * @param [in] unsigned int u32Offset
 * @retval VOID
*/
VOID HAL_POST_SetHscalerOffset_C(unsigned int u32Offset);

/**
 * @brief This function set h scaler coef
 * @param [in] unsigned int u32Coef
 * @retval VOID
*/
VOID HAL_POST_SetHscalerCoef_C(unsigned int u32Coef);

/**
 * @brief This function set h scaler coef1
 * @param [in] unsigned int u32Coef
 * @retval VOID
*/
VOID HAL_POST_SetHscalerCoef1_C(unsigned int u32Coef);

/**
 * @brief This function set csc enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetCSCEnable(BOOL bEnable);

/**
 * @brief This function set post csc mode
 * @param [in] CSC_MODE enCSC_Mode
 * @retval VOID
*/
VOID HAL_POST_SetCSCMode(CSC_MODE enCSC_Mode);

/**
 * @brief This function set post mute screen enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetMuteScreenEnable(BOOL bEnable);

/**
 * @brief This function set post mute screen RGB
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetMuteScreenRGBEnable(unsigned int u32RGB);

/**
 * @brief This function set post data mapping
 * @param [in] unsigned int u32Data_Mapping
 * @retval VOID
*/
VOID HAL_POST_SetDataMapping(unsigned int u32Data_Mapping);

/**
 * @brief This function get post data mapping
 * @param [in] VOID
 * @retval data mapping
*/
unsigned int HAL_POST_GetDataMapping(VOID);

/**
 * @brief This function set black insert line mode enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetBlackInsertLineModeEnable(BOOL bEnable);

/**
 * @brief This function set black insert line mode pattern number
 * @param [in] unsigned int u32PatternNum
 * @retval VOID
*/
VOID HAL_POST_SetBlackInsertLineModePatternNum(unsigned int u32PatternNum);

/**
 * @brief This function set black insert line mode pattern
 * @param [in] unsigned int u32Pattern
 * @retval VOID
*/
VOID HAL_POST_SetBlackInsertLineModeLPattern(unsigned int u32Pattern);

/**
 * @brief This function set black insert line mode pattern
 * @param [in] unsigned int u32Pattern
 * @retval VOID
*/
VOID HAL_POST_SetBlackInsertLineModeRPattern(unsigned int u32Pattern);

/**
 * @brief This function set black insert frame mode enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetBlackInsertFrameModeEnable(BOOL bEnable);

/**
 * @brief This function set kpost out interrupt select
 * @param [in] 7 for FRC, 6 for OSD, 5 for Out
 * @retval VOID
*/
VOID HAL_POST_SetOutINTSel(unsigned int u32Sel);

/**
 * @brief This function set kpost out interrupt source
 * @param [in] 1
 * @retval VOID
*/
VOID HAL_POST_SetOutINTSource(unsigned int u32Sor);

/**
 * @brief This function set kpost out interrupt horizantal number
 * @param [in] H number
 * @retval VOID
*/
VOID HAL_POST_SetOutINTHNum(unsigned int u32Hnum);

/**
 * @brief This function set kpost out interrupt enable
 * @param [in] u32IntType: 0 for H sync, 1 for V sync
 * @param [in] bEnable
 * @retval VOID
*/
VOID HAL_POST_SetOutINTEnable(unsigned int u32IntType, BOOL bEnable);
BOOL HAL_POST_GetOutINTEnable(unsigned int u32IntType);

/**
 * @brief This function set post cursor enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetCursorEnable(BOOL bEnable);

/**
 * @brief This function set post cursor show
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetCursorShow(BOOL bEnable);

/**
 * @brief This function set post cursor position
 * @param [in] reg_post_cursor_h
 * @param [in] reg_post_cursor_v
 * @retval VOID
*/
VOID HAL_POST_SetCursorPosition(unsigned int u32Hposition, unsigned int u32Vposition);

/**
 * @brief This function set post cursor width
 * @param [in] unsigned int u32Width(0~15)
 * @retval VOID
*/
VOID HAL_POST_SetCursorWidth(unsigned int u32Width);

/**
 * @brief This function set post cursor color
 * @param [in] unsigned int u32Width(0~7)
 * @retval VOID
*/
VOID HAL_POST_SetCursorColor(unsigned int u32Color);

/**
 * @brief This function set kpost top in interrupt clear
 * @param [in] eIntType: 0 for H sync, 1 for V sync
 * @param [in] bWclr
 * @retval VOID
*/
VOID HAL_KPOST_TOP_SetOutINTWclr(INT_TYPE eIntType, BOOL bWclr);
/**
 * @brief This function get kost top out interrupt status
 * @retval unsigned int
*/
unsigned int HAL_KPOST_TOP_GetOutINTStatus(VOID);

#endif
