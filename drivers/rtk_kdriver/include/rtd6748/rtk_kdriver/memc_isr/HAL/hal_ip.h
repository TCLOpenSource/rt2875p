/**
 * @file hal_ip.h
  * @brief This file is for ip register setting
  * @date Nov.14.2014
  * @version V0.0.1
  * @author Martin Yan
  * @par Copyright (C) 2014 Real-Image Co. LTD
  * @par History:
  * 		 Version: 0.0.1
 */

#ifndef _HAL_IP_HEADER_INCLUDE_
#define _HAL_IP_HEADER_INCLUDE_

#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/memc_type.h"

/**
 * @brief This function set ippre csc enable
 * @param [in] BOOL bEnable true for enable, false for diable
 * @retval VOID
*/
VOID HAL_IP_SetIPPRECSCEnable(BOOL bEnable);

/**
 * @brief This function set ippre csc mode
 * @param [in] CSC_MODE enCSC_Mode
 * @retval VOID
*/
VOID HAL_IP_SetIPPRECSCMode(CSC_MODE enCSC_Mode);

/**
 * @brief This function set ippre 444to422 Enable
 * @param [in]  bool bEnable
 * @retval VOID
*/
VOID HAL_IP_SetIPPRE444to422Enable(BOOL bEnable);

/**
 * @brief This function set ippre retiming
 * @param [in] unsigned int u32Hact
 * @param [in] unsigned int u32Vact 
 * @retval VOID
*/
VOID HAL_IP_SetIPPRERetiming(unsigned int u32Hact, unsigned int u32Vact);

/**
 * @brief This function get ippre retiming dummy de
 * @param [in] VOID
 * @param [in] unsigned int u32Vact 
 * @retval retiming dummy de
*/
unsigned int HAL_IP_GetIPPRERetimingDummyDE(VOID);

/**
 * @brief This function set ippre pattern size
 * @param [in] TIMING_PARA_ST stInputTiming 
 * @retval VOID
*/
VOID HAL_IP_SetIPPREPatternSize(TIMING_PARA_ST stInputTiming);

/**
 * @brief This function set ippre use mute sync enable
 * @param [in]  bool bEnable
 * @retval VOID
*/
VOID HAL_IP_SetIPPREMuteUseMuteSyncEnable(BOOL bEnable);

/**
 * @brief This function set ippre mute set mute detect enable
 * @param [in]  bool bEnable
 * @retval VOID
*/
VOID HAL_IP_SetIPPREMuteSetMuteDetectEnable(BOOL bEnable);

/**
 * @brief This function set ippre mute h active
 * @param [in] unsigned int u32Hact 
 * @param [in] unsigned int u32Hact_Min 
 * @param [in] unsigned int u32Hact_Max 
 * @retval VOID
*/
VOID HAL_IP_SetIPPREMuteHactive(unsigned int u32Hact, unsigned int u32Hact_Min, unsigned int u32Hact_Max);

/**
 * @brief This function set ippre v active
 * @param [in] unsigned int u32Vact 
 * @param [in] unsigned int u32Vact_Min 
 * @param [in] unsigned int u32Vact_Max 
 * @retval VOID
*/
VOID HAL_IP_SetIPPREMuteVactive(unsigned int u32Vact, unsigned int u32Vact_Min, unsigned int u32Vact_Max);

/**
 * @brief This function set ippre mute clock threshold
 * @param [in] unsigned int u32Clk_low_th 
 * @param [in] unsigned int u32Clk_high_th 
 * @retval VOID
*/
VOID HAL_IP_SetIPPREMuteClockThreshold(unsigned int u32Clk_low_th, unsigned int u32Clk_high_th);

/**
 * @brief This function set 3d fs lr inverse
 * @param [in] BOOL bEnable 
 * @retval VOID
*/
VOID HAL_IP_Set3DFSLRInverse(BOOL bEnable);

#ifndef REAL_CHIP	// FPGA
/**
 * @brief This function set topA ipme retiming
 * @param [in] TIMING_PARA_ST stInputTiming
 * @retval VOID
*/
VOID HAL_IP_SetTOPA_IPMERetiming(TIMING_PARA_ST stInputTiming);

/**
 * @brief This function set 2 port enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_IP_SetTOPA2PortEnable(BOOL bEnable);

#if 1  // K4Lp
/**
 * @brief This function set ramp enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_IP_SetTOPASH13Ramp(BOOL bEnable);
#endif

/**
 * @brief This function set topB mute timing
 * @param [in] TIMING_PARA_ST stInputTiming
 * @param [in] TIMING_PARA_ST stOutputTiming 
 * @retval VOID
*/
VOID HAL_IP_SetTOPBMuteTiming(TIMING_PARA_ST stInputTiming, TIMING_PARA_ST stOutputTiming);

/**
 * @brief This function set mute lr self gen
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_IP_SetTOPBMuteLRSelfGen(BOOL bEnable);

#if 1  // K4Lp
/**
 * @brief This function set mute force free run
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_IP_SetTOPBForceFreerun(BOOL bEnable);

/**
 * @brief This function set mute frame trig position
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_IP_SetTOPBFRMTrigPosition(unsigned int u32Val);
#endif
#endif

/**
 * @brief This function set ip data mapping
 * @param [in] unsigned int u32Data_Mapping 
 * @retval VOID
*/
VOID HAL_IP_SetDataMapping(unsigned int u32Data_Mapping);

/**
 * @brief This function get ip data mapping
 * @param [in] VOID
 * @retval data mapping
*/
unsigned int HAL_IP_GetDataMapping(VOID);

#endif
