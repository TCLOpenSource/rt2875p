/**
 * @file hal_crtc.h
  * @brief This file is for crtc register setting
  * @date Nov.14.2014
  * @version V0.0.1
  * @author Martin Yan
  * @par Copyright (C) 2014 Real-Image Co. LTD
  * @par History:
  * 		 Version: 0.0.1
 */

#ifndef _HAL_CRTC_HEADER_INCLUDE_
#define _HAL_CRTC_HEADER_INCLUDE_

#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/memc_type.h"

/**
 * @brief This function set frc timing
 * @param [in] TIMING_PARA_ST stFRCTiming
 * @retval VOID
*/
VOID HAL_CRTC_SetFRCTiming(TIMING_PARA_ST stFRCTiming);

/**
 * @brief This function set osd timing
 * @param [in] u32Hact
 * @param [in] u32Vact
 * @retval VOID
*/
VOID HAL_CRTC_SetOSDTiming(unsigned int u32Hact, unsigned int u32Vact);

/**
 * @brief This function set panel timing
 * @param [in] TIMING_PARA_ST stFRCTiming
 * @retval VOID
*/
VOID HAL_CRTC_SetPanelTiming(PANEL_PARAMETERS_ST stPanelTiming);

/**
 * @brief This function get ME orig vtrig delay
 * @param [in] VOID
 * @retval unsigned int u32ME_Orig_Vtrig_dly
*/
unsigned int HAL_CRTC_GetMEOrigVtrigDelay(VOID);

/**
 * @brief This function set ME orig vtrig delay
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_CRTC_SetMEOrigVtrigDelay(unsigned int u32Val);

/**
 * @brief This function get ME vtrig delay
 * @param [in] VOID
 * @retval unsigned int u32ME_Vtrig_dly
*/
unsigned int HAL_CRTC_GetMEVtrigDelay(VOID);
/**
 * @brief This function get MC vtrig delay
 * @param [in] VOID
 * @retval unsigned int u32MC_Vtrig_dly
*/
unsigned int HAL_CRTC_GetMCVtrigDelay(VOID);

/**
 * @brief This function set frc_vs_bporch
 * @param [in] unsigned int u32FRC_VS_Bporch
 * @retval VOID
*/
VOID HAL_CRTC_SetFRCVSBporch(unsigned int u32FRC_VS_Bporch);

/**
 * @brief This function set frc vtrig delay
 * @param [in] unsigned int u32FRC_Vtrig_dly
 * @retval VOID
*/
VOID HAL_CRTC_SetFRCVtrigDelay(unsigned int u32FRC_Vtrig_dly);

/**
 * @brief This function set frc vtrig delay
 * @param [in] unsigned int u32FRC_Vtrig_dly
 * @retval VOID
*/
VOID HAL_CRTC_SetLbmeVtrigDelay(unsigned int u32Lbme_Vtrig_dly);

/**
 * @brief This function set frc pre de l num
 * @param [in] unsigned int u32Frc_Prede_LNum
 * @retval VOID
*/
VOID HAL_CRTC_SetFrcPredeLNum(unsigned int u32Frc_Prede_LNum);

/**
 * @brief This function set dehalo vtrig delay
 * @param [in] unsigned int u32DEHALO_Vtrig_dly
 * @retval VOID
*/
VOID HAL_CRTC_SetDehaloVtrigDelay(unsigned int u32DEHALO_Vtrig_dly);

/**
 * @brief This function set dec vtrig delay
 * @param [in] unsigned int u32DEC_Vtrig_dly
 * @retval VOID
*/
VOID HAL_CRTC_SetDecVtrigDelay(unsigned int u32DEC_Vtrig_dly);

/**
 * @brief This function set mc vtrig delay
 * @param [in] unsigned int u32MC_Vtrig_dly
 * @retval VOID
*/
VOID HAL_CRTC_SetMCVtrigDelay(unsigned int u32MC_Vtrig_dly);

/**
 * @brief This function set mc vtrig delay long
 * @param [in] unsigned int u32MC_Vtrig_dly_long
 * @retval VOID
*/
VOID HAL_CRTC_SetMCVtrigDelayLong(unsigned int u32MC_Vtrig_dly_long);

/**
 * @brief This function set lbmc vtrig delay
 * @param [in] unsigned int u32Lbmc_Vtrig_dly
 * @retval VOID
*/
VOID HAL_CRTC_SetLbmcVtrigDelay(unsigned int u32Lbmc_Vtrig_dly);

#if 1
/**
 * @brief This function set me1 vactive
 * @param [in] unsigned int u32ME_Vactive
 * @retval VOID
*/
VOID HAL_CRTC_SetME1Vactive(unsigned int u32ME_Vactive);

/**
 * @brief This function set me2 vactive
 * @param [in] unsigned int u32ME_Vactive
 * @retval VOID
*/
VOID HAL_CRTC_SetME2Vactive(unsigned int u32ME_Vactive);
#else
/**
 * @brief This function set me vactive
 * @param [in] unsigned int u32ME_Vactive
 * @retval VOID
*/
VOID HAL_CRTC_SetMEVactive(unsigned int u32ME_Vactive);
#endif

/**
 * @brief This function set dehalo vactive
 * @param [in] unsigned int u32Dehalo_Vactive
 * @retval VOID
*/
VOID HAL_CRTC_SetDehaloVactive(unsigned int u32Dehalo_Vactive);

/**
 * @brief This function set mc vactive
 * @param [in] unsigned int u32MC_Vactive
 * @retval VOID
*/
VOID HAL_CRTC_SetMCVactive(unsigned int u32MC_Vactive);

/**
 * @brief This function set pixel logo vactive
 * @param [in] unsigned int u32Plogo_Vactive
 * @retval VOID
*/
VOID HAL_CRTC_SetPLogoVactive(unsigned int u32Plogo_Vactive);

/**
 * @brief This function set me row type
 * @param [in] unsigned int u32ME_Row_Type
 * @retval VOID
*/
VOID HAL_CRTC_SetME1RowType(unsigned int u32ME_Row_Type);

/**
 * @brief This function set me2 row type
 * @param [in] unsigned int u32ME_Row_Type
 * @retval VOID
*/
VOID HAL_CRTC_SetME2RowType(unsigned int u32ME_Row_Type);

/**
 * @brief This function set dehalo row type
 * @param [in] unsigned int u32Dehalo_Row_Type
 * @retval VOID
*/
VOID HAL_CRTC_SetDehaloRowType(unsigned int u32Dehalo_Row_Type);

/**
 * @brief This function set mc row type
 * @param [in] unsigned int u32MC_Row_Type
 * @retval VOID
*/
VOID HAL_CRTC_SetMCRowType(unsigned int u32MC_Row_Type);

/**
 * @brief This function set plogo row type
 * @param [in] unsigned int u32Plogo_Row_Type
 * @retval VOID
*/
VOID HAL_CRTC_SetPlogoRowType(unsigned int u32Plogo_Row_Type);

/**
 * @brief This function set plogo fetch row type
 * @param [in] unsigned int u32Plogo_fetch_Row_Type
 * @retval VOID
*/
VOID HAL_CRTC_SetPlogoFetchRowType(unsigned int u32Plogo_fetch_Row_Type);

/**
 * @brief This function set frc prede pnum
 * @param [in] unsigned int u32FRC_prede_pnum
 * @retval VOID
*/
VOID HAL_CRTC_SetFRCPreDEPNum(unsigned int u32FRC_prede_pnum);

/**
 * @brief This function set panel vtrig delay
 * @param [in] unsigned int u32Panel_Vtrig_dly
 * @retval VOID
*/
VOID HAL_CRTC_SetOutVtrigDelay(unsigned int u32Panel_Vtrig_dly);

/**
 * @brief This function set n to m
 * @param [in] unsigned int u32N
 * @param [in] unsigned int u32M
 * @retval VOID
*/
VOID HAL_CRTC_SetN2M(unsigned int u32N, unsigned int u32M);

/**
 * @brief This function set CRTC slave mode enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_CRTC_SetSlaveModeEnable(BOOL bEnable);

/**
 * @brief This function set lbmc normal lf delay
 * @param [in] unsigned char ubyIdx (0~8)
 * @param [in] unsigned char ubyDly
 * @retval VOID
*/
VOID HAL_CRTC_SetLBMCNormalLFDly(unsigned char ubyIdx, unsigned char ubyDly);

/**
 * @brief This function set lbmc normal hf delay
 * @param [in] unsigned char ubyIdx (0~3)
 * @param [in] unsigned char ubyDly
 * @retval VOID
*/
VOID HAL_CRTC_SetLBMCNormalHFDly(unsigned char ubyIdx, unsigned char ubyDly);

/**
 * @brief This function set lbmc signle lf delay
 * @param [in] unsigned char ubyIdx (0~8)
 * @param [in] unsigned char ubyDly
 * @retval VOID
*/
VOID HAL_CRTC_SetLBMCSingleLFDly(unsigned char ubyIdx, unsigned char ubyDly);

/**
 * @brief This function set lbmc single hf delay
 * @param [in] unsigned char ubyIdx (0~3)
 * @param [in] unsigned char ubyDly
 * @retval VOID
*/
VOID HAL_CRTC_SetLBMCSingleHFDly(unsigned char ubyIdx, unsigned char ubyDly);

/**
 * @brief This function get vspll lock status
 * @retval True for lock
 * @retval False for unlock
*/
BOOL HAL_CRTC_GetVSPLLStatus(VOID);

/**
 * @brief This function set crtc lr enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_CRTC_SetLREnable(BOOL bEnable);

/**
 * @brief This function set crtc lr cycle
 * @param [in] unsigned int u32Cycle
 * @retval VOID
*/
VOID HAL_CRTC_SetLRCycle(unsigned int u32Cycle);

/**
 * @brief This function set crtc lr pattern
 * @param [in] unsigned int u32Pattern
 * @retval VOID
*/
VOID HAL_CRTC_SetLRPattern(unsigned int u32Pattern);

/**
 * @brief This function set crtc me to dehalo vtrig delay
 * @param [in] unsigned int u32Pre_dly
 * @param [in] unsigned int u32Pst_dly
 * @param [in] unsigned int u32Dob_dly
 * @retval VOID
*/
VOID HAL_CRTC_SetME2DHVtrigDly(unsigned int u32Pre_dly, unsigned int u32Pst_dly, unsigned int u32Dob_dly);

#endif
