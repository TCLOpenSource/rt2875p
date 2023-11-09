/**
 * @file hal_mc.h
  * @brief This file is for mc register setting
  * @date Nov.14.2014
  * @version V0.0.1
  * @author Martin Yan
  * @par Copyright (C) 2014 Real-Image Co. LTD
  * @par History:
  * 		 Version: 0.0.1
 */

#ifndef _HAL_MC_HEADER_INCLUDE_
#define _HAL_MC_HEADER_INCLUDE_

#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/memc_type.h"

/**
 * @brief This function set kmc top in interrupt select
 * @param [in] 7 for input
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetInINTSel(unsigned int u32Sel);

/**
 * @brief This function set kmc top in interrupt source
 * @param [in] 1
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetInINTSource(unsigned int u32Sor);

/**
 * @brief This function set kmc top in interrupt horizantal number
 * @param [in] H number
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetInINTHNum(unsigned int u32Hnum);

/**
 * @brief This function set kmc top in interrupt enable
 * @param [in] u32IntType: 0 for H sync, 1 for V sync, 2 for Abnormal
 * @param [in] bEnable
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetInINTEnable(unsigned int u32IntType, BOOL bEnable);
BOOL HAL_KMC_TOP_GetInINTEnable(unsigned int u32IntType);

/**
 * @brief This function set kmc top in interrupt clear
 * @param [in] eIntType: 0 for H sync, 1 for V sync, 2 for Abnormal
 * @param [in] bWclr
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetInINTWclr(INT_TYPE eIntType, BOOL bWclr);

/**
 * @brief This function get kmc top in interrupt status
 * @retval unsigned int
*/
unsigned int HAL_KMC_TOP_GetInINTStatus(VOID);

/**
 * @brief This function set kmc top pattern size
 * @param [in] TIMING_PARA_ST stInputTiming
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetPatternSize(TIMING_PARA_ST stInputTiming);

/**
 * @brief This function set kmc knr 422 to 444 enable
 * @param [in] unsigned int bEnable
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnr422to444Enable(BOOL bEnable);

/**
 * @brief This function set kmc knr csc enable
 * @param [in] unsigned int bEnable
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnrCSCEnable(BOOL bEnable);

/**
 * @brief This function set kmc knr csc mode
 * @param [in] unsigned int bEnable
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnrCSCMode(unsigned int u32Mode);

/**
 * @brief This function set kmc top knr trig gen mode
 * @param [in] unsigned int u32knr_trig_gen_mode
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnrTrigGenMode(unsigned int u32knr_trig_gen_mode);

/**
 * @brief This function set kmc top knr row trig delay
 * @param [in] unsigned int u32knr_row_trig_dly
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnrRowTrigDelay(unsigned int u32knr_row_trig_dly);

/**
 * @brief This function set kmc top knr half v active
 * @param [in] unsigned int u32knr_half_vactive
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnrHalfVactive(unsigned int u32knr_half_vactive);

/**
 * @brief This function set kmc top knr h active
 * @param [in] unsigned int u32knr_hactive
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnrHactive(unsigned int u32knr_hactive);

/**
 * @brief This function set kmc top ipmc hde center value
 * @param [in] unsigned int u32ipmc_hde_center_value
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetIPMCHDECenterValue(unsigned int u32ipmc_hde_center_value);

/**
 * @brief This function set kmc top ipmc mode
 * @param [in] unsigned int u32ipmc_mode
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetIPMCMode(unsigned int u32ipmc_mode);

/**
 * @brief This function set kmc top ipmc hde active
 * @param [in] unsigned int u32ipmc_hde_active
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetIPMCHDEActive(unsigned int u32ipmc_hde_active);

/**
 * @brief This function set kmc top ipmc v active
 * @param [in] unsigned int u32ipmc_v_active
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetIPMCVactive(unsigned int u32ipmc_v_active);

/**
 * @brief This function set kmc top data mapping
 * @param [in] unsigned int u32Data_Mapping
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetDataMapping(unsigned int u32Data_Mapping);

/**
 * @brief This function get kmc top data mapping
 * @param [in] VOID
 * @retval data mapping
*/
unsigned int HAL_KMC_TOP_GetDataMapping(VOID);

/**
 * @brief This function set lbmc fhd mode
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetLBMCFHDMode(BOOL bEnable);

/**
 * @brief This function set lbmc H/V active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_MC_SetLBMCActive(unsigned int u32Hactive, unsigned int u32Vactive);

/**
 * @brief This function set lbmc 3d mode
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetLBMC3DMode(BOOL bEnable);

/**
 * @brief This function set lbmc pr mode
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetLBMCPRMode(BOOL bEnable);

/**
 * @brief This function set lbmc pc mode
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetLBMCPCMode(BOOL bEnable);

/**
 * @brief This function set lbmc vflip enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetLBMCVflipEnable(BOOL bEnable);

/**
 * @brief This function set mc h half size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetMCHhalfsize(BOOL bEnable);

/**
 * @brief This function set mc v half size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetMCVhalfsize(BOOL bEnable);

/**
 * @brief This function set mc H/V active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_MC_SetMCActive(unsigned int u32Hactive, unsigned int u32Vactive);

/**
 * @brief This function set mc block size
 * @param [in] unsigned int u32Block_Size
 * @retval VOID
*/
VOID HAL_MC_SetMCBlockSize(unsigned int u32Block_Size);

/**
 * @brief This function get mc block size
 * @param [in] VOID
 * @retval unsigned int u32Block_Size
*/
unsigned int HAL_MC_GetMCBlockSize(VOID);

/**
 * @brief This function set mc mv scaler
 * @param [in] unsigned int u32MV_Scaler
 * @retval VOID
*/
VOID HAL_MC_SetMCMVScaler(unsigned int u32MV_Scaler);

/**
 * @brief This function set mc block number
 * @param [in] unsigned int u32Block_Num
 * @retval VOID
*/
VOID HAL_MC_SetMCBlockNum(unsigned int u32Block_Num);

/**
 * @brief This function set mc row number
 * @param [in] unsigned int u32Block_Num
 * @retval VOID
*/
VOID HAL_MC_SetMCRowNum(unsigned int u32Row_Num);

#if 1  // K4Lp
/**
 * @brief This function set mc mv pattern block size
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCMVPatBlkSize(unsigned int u32Val);

/**
 * @brief This function set mc mv pattern Row size
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCMVPatRowSize(unsigned int u32Val);

/**
 * @brief This function set mc dehalo half v timing
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCDhHalfVTiming(unsigned int u32Val);

/**
 * @brief This function set mc dehalo Row size
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCDhRowSize(unsigned int u32Val);

/**
 * @brief This function set mc mv read interval
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCMVRDIntval(unsigned int u32Val);
#endif

/**
 * @brief This function set mc blend mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_MC_SetMCBlendMode(unsigned int u32Mode);

/**
 * @brief This function set mc scene change enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetMCSceneChangeEnable(BOOL bEnable);

/**
 * @brief This function set mc scene change enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetMCSceneChange_fb(BOOL bEnable);

/**
 * @brief This function set MC PT Zero type1
 * @param [in] u32Type: 0: iz; 1: pz; 2: avg; 3: phase blend; 4: I/P by phase; 5: med3
 * @retval VOID
*/
VOID HAL_MC_SetPTZeroType1(unsigned int u32Type);

/**
 * @brief This function set MC PT Zero type2
 * @param [in] u32Type: 0: iz; 1: pz; 2: avg; 3: phase blend; 4: I/P by phase; 5: med3
 * @retval VOID
*/
VOID HAL_MC_SetPTZeroType2(unsigned int u32Type);

/**
 * @brief This function set mc logo mode
 * @param [in] unsigned int u32LogoMode
 * @retval VOID
*/
VOID HAL_MC_SetLogoMode(unsigned int u32LogoMode);

/**
 * @brief This function set mc block logo h size
 * @param [in] unsigned int u32Hsize
 * @retval VOID
*/
VOID HAL_MC_SetBlockLogoHSize(unsigned int u32Hsize);

/**
 * @brief This function set mc pixel logo h size
 * @param [in] unsigned int u32Hsize
 * @retval VOID
*/
VOID HAL_MC_SetPixelLogoHSize(unsigned int u32Hsize);

/**
 * @brief This function set mc block logo v size
 * @param [in] unsigned int u32Vsize
 * @retval VOID
*/
VOID HAL_MC_SetBlockLogoVSize(unsigned int u32Vsize);

/**
 * @brief This function set mc pixel logo v size
 * @param [in] unsigned int u32Vsize
 * @retval VOID
*/
VOID HAL_MC_SetPixelLogoVSize(unsigned int u32Vsize);

/**
 * @brief This function set kphase 3d fs enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetKphase3DFSEnable(BOOL bEnable);

/**
 * @brief This function set kphase 3d enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetKphase3DEnable(BOOL bEnable);

/**
 * @brief This function set 3d mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_MC_SetKphase3DMode(unsigned int u32Mode);

/**
 * @brief This function set me1 index bsize
 * @param [in] unsigned int u32Bsize
 * @retval VOID
*/
VOID HAL_MC_SetKphaseME1IndexBsize(unsigned int u32Bsize);

/**
 * @brief This function set me2 index bsize
 * @param [in] unsigned int u32Bsize
 * @retval VOID
*/
VOID HAL_MC_SetKphaseME2IndexBsize(unsigned int u32Bsize);

/**
 * @brief This function set mc index bsize
 * @param [in] unsigned int u32Bsize
 * @retval VOID
*/
VOID HAL_MC_SetKphaseMCIndexBsize(unsigned int u32Bsize);

/**
 * @brief This function set decode data clear enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetKphaseDecDataClrEnable(BOOL bEnable);

/**
 * @brief This function set kphase force mc lf index
 * @param [in] unsigned int u32Idx
 * @retval VOID
*/
VOID HAL_MC_SetKphaseForceMCLFIdx(unsigned int u32Idx);

/**
 * @brief This function set kphase force mc lf index enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetKphaseForceMCLFIdxEnable(unsigned int u32Idx);

/**
 * @brief This function set kphase force mc hf index
 * @param [in] unsigned int u32Idx
 * @retval VOID
*/
VOID HAL_MC_SetKphaseForceMCHFIdx(unsigned int u32Idx);

/**
 * @brief This function set kphase force mc hf index enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetKphaseForceMCHFIdxEnable(unsigned int u32Idx);

/**
 * @brief This function set demo window Enable
 * @param [in] unsigned char ubyWinID
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetDemoWindowEnable(unsigned char ubyWinID, BOOL bEnable);

/**
 * @brief This function get demo window Enable
 * @param [in] unsigned char ubyWinID
 * @retval  [out] BOOL bEnable
*/
BOOL HAL_MC_GetDemoWindowEnable(unsigned char ubyWinID);

/**
 * @brief This function set demo window mode
 * @param [in] BOOL bMode: 0: inside MEMC OFF; 1: outside MEMC OFF
 * @retval VOID
*/
VOID HAL_MC_SetDemoWindowMode(BOOL bMode);

/**
 * @brief This function get demo window mode
 * @retval [out] BOOL bMode: 0: inside MEMC OFF; 1: outside MEMC OFF
*/
BOOL HAL_MC_GetDemoWindowMode(VOID);

/**
 * @brief This function set demo window
 * @param [in] unsigned char ubyWinID
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @retval VOID
*/
VOID HAL_MC_SetDemoWindowSettings(unsigned char ubyWinID, unsigned int u32Left, unsigned int u32Right, unsigned int u32Top, unsigned int u32Bottom);

/**
 * @brief This function get demo window
 * @param [in] unsigned char ubyWinID
 * @param [out] unsigned int u32Left
 * @param [out] unsigned int u32Right
 * @param [out] unsigned int u32Top
 * @param [out] unsigned int u32Bottom
 * @retval VOID
*/
VOID HAL_MC_GetDemoWindowSettings(unsigned char ubyWinID, unsigned int *pu32Left, unsigned int *pu32Right, unsigned int *pu32Top, unsigned int *pu32Bottom);

/**
 * @brief This function set lbmc normal lf prefetch number
 * @param [in] BOOL bType: 0 for i, 1 for p
 * @param [in] ubyIdx(0~8)
 * @param [in] ubyPreFetch_Num
 * @retval VOID
*/
VOID HAL_MC_SetLBMCNormalLFPreFetchNum(BOOL bType, unsigned char ubyIdx, unsigned char ubyPreFetch_Num);

/**
 * @brief This function set lbmc normal hf prefetch number
 * @param [in] BOOL bType: 0 for i, 1 for p
 * @param [in] ubyIdx(0~2)
 * @param [in] ubyPreFetch_Num
 * @retval VOID
*/
VOID HAL_MC_SetLBMCNormalHFPreFetchNum(BOOL bType, unsigned char ubyIdx, unsigned char ubyPreFetch_Num);

/**
 * @brief This function set lbmc single lf prefetch number
 * @param [in] BOOL bType: 0 for i, 1 for p
 * @param [in] ubyIdx(0~2)
 * @param [in] ubyPreFetch_Num
 * @retval VOID
*/
VOID HAL_MC_SetLBMCSingleLFPreFetchNum(BOOL bType, unsigned char ubyIdx, unsigned char ubyPreFetch_Num);

/**
 * @brief This function set lbmc single hf prefetch number
 * @param [in] BOOL bType: 0 for i, 1 for p
 * @param [in] ubyIdx(0~2)
 * @param [in] ubyPreFetch_Num
 * @retval VOID
*/
VOID HAL_MC_SetLBMCSingleHFPreFetchNum(BOOL bType, unsigned char ubyIdx, unsigned char ubyPreFetch_Num);

/**
 * @brief This function set mc rim 0
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_MC_SetMCRim0(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

/**
 * @brief This function set mc rim 1
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_MC_SetMCRim1(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

/**
 * @brief This function set mc var lp enable
 * @param [in] unsigned int u32En
 * @retval VOID
*/
VOID HAL_MC_SetVarLPFEn(unsigned int u32En);

/**
 * @brief This function set mc var lp 17 tap enable
 * @param [in] unsigned int BOOL
 * @retval VOID
*/
VOID HAL_MC_SetVar17TapEn(BOOL bEnable);

/**
 * @brief This function set mc pr mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_MC_SetPRMode(unsigned int u32Mode);

/**
 * @brief This function set mc poly enable
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetPolyEnable(unsigned int u32Val);

#if 1  // K4Lp
/**
 * @brief This function set block logo rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KMC_LOGO_Set_BlockRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

/**
 * @brief This function set pixel pixel rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KMC_LOGO_Set_PixelRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

/**
 * @brief This function set mc block logo htotal
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCBLKLogoHtotal(unsigned int u32Val);

/**
 * @brief This function set mc block logo hact
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCBLKLogoHact(unsigned int u32Val);

/**
 * @brief This function set mc block logo row num
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCBLKLogoRowNum(unsigned int u32Val);

/**
 * @brief This function set mc block logo row type
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCBLKLogoRowType(unsigned int u32Val);

/**
 * @brief This function set lbmc share hf enable
 * @param [in] unsigned int BOOL
 * @retval VOID
*/
VOID HAL_MC_SetLBMCShareHFEn(BOOL bEnable);

#endif

/**
 * @brief This function get kmc04_06_total_data_cnt
 * @retval unsigned int
*/
unsigned int HAL_MC_GetMC04_06_total_data_cnt(VOID);

#endif

/**
 * @brief This function enable  local fallback
 * @param [in] BOOL bEnalbe
 * @retval VOID
*/
VOID HAL_MC_SetLFBEnable(BOOL bEnalbe);


VOID HAL_PQL_TOP_SetInputIntp_en(BOOL bEnalbe);


VOID HAL_PQL_TOP_SetOnefiftIntp_en(BOOL bEnalbe);


VOID HAL_PQL_TOP_SetOutputIntp_en(BOOL bEnalbe);


