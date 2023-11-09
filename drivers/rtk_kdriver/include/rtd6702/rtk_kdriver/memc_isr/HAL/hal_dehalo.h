/**
 * @file hal_dehalo.h
  * @brief This file is for dehalo register setting
  * @date Nov.14.2014
  * @version V0.0.1
  * @author Martin Yan
  * @par Copyright (C) 2014 Real-Image Co. LTD
  * @par History:
  * 		 Version: 0.0.1
 */

#ifndef _HAL_DEHALO_HEADER_INCLUDE_
#define _HAL_DEHALO_HEADER_INCLUDE_

#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/memc_type.h"

/**
 * @brief This function set Dehalo on/off
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetEnable(BOOL bEnable);

/**
 * @brief This function set Dehalo row size
 * @param [in] unsigned int u32Row_Size
 * @retval VOID
*/
VOID HAL_Dehalo_SetRowSize(unsigned int u32Row_Size);

/**
 * @brief This function set Dehalo block size
 * @param [in] unsigned int u32Block_Size
 * @retval VOID
*/
VOID HAL_Dehalo_SetBlockSize(unsigned int u32Block_Size);

/**
 * @brief This function set Dehalo logo row size
 * @param [in] unsigned int u32Row_Size
 * @retval VOID
*/
VOID HAL_Dehalo_SetLogoRowSize(unsigned int u32Row_Size);

/**
 * @brief This function set Dehalo logo block size
 * @param [in] unsigned int u32Block_Size
 * @retval VOID
*/
VOID HAL_Dehalo_SetLogoBlockSize(unsigned int u32Block_Size);

/**
 * @brief This function set Dehalo YX ratio
 * @param [in] unsigned int u32YXRatio
 * @retval VOID
*/
VOID HAL_Dehalo_SetYXRatio(unsigned int u32YXRatio);

/**
 * @brief This function set Dehalo block number
 * @param [in] unsigned int u32Block_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetBlockNum(unsigned int u32Block_Num);

/**
 * @brief This function set Dehalo row number
 * @param [in] unsigned int u32Row_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetRowNum(unsigned int u32Row_Num);

/**
 * @brief This function set Dehalo vbuf block number
 * @param [in] unsigned int u32Block_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetVbufBlockNum(unsigned int u32Block_Num);

#if 1
/**
 * @brief This function set Dehalo logo block number
 * @param [in] unsigned int u32Block_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetLogoBlockNum(unsigned int u32Block_Num);

/**
 * @brief This function set Dehalo logo row number
 * @param [in] unsigned int u32Row_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetLogoRowNum(unsigned int u32Row_Num);
#endif

/**
 * @brief This function set Dehalo vbuf row number
 * @param [in] unsigned int u32Row_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetVbufRowNum(unsigned int u32Row_Num);

/**
 * @brief This function set Dehalo PR enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPREnable(BOOL bEnable);

/**
 * @brief This function set Dehalo MV RD intval
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetMVRDIntval(BOOL bEnable);

/**
 * @brief This function set half v timing
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetHalfVTiming(BOOL bEnable);

/**
 * @brief This function set patt size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPattSize(BOOL bEnable);

/**
 * @brief This function set out timing mux
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_Dehalo_SetOutTimimgMux(unsigned int u32Val);

/**
 * @brief This function set rd lr flow wr latch
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetRDLRFlowWRLatch(BOOL bEnable);


/**
 * @brief This function set sw reset
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetSWReset(BOOL bEnable);

/**
 * @brief This function set Dehalo patt block / row number
 * @param [in] unsigned int u32Block_Num
 * @param [in] unsigned int u32Row_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetPatternBlockRow(unsigned int u32Block_Num, unsigned int u32Row_Num);

/**
 * @brief This function set Dehalo vbuf block / row number
 * @param [in] unsigned int u32Block_Num
 * @param [in] unsigned int u32Row_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetVbufBlockRow(unsigned int u32Block_Num, unsigned int u32Row_Num);

/**
 * @brief This function set Dehalo show enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetShowEnable(BOOL bEnable);
/**
 * @brief This function set Dehalo show select
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetShowSelect(unsigned int u32Select);

/**
 * @brief This function set Dehalo erosion mode
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetErosionMode(BOOL bEnable);

#if 1
/**
 * @brief This function set Dehalo pre-filter dummy number
 * @param [in] unsigned int u32Block_dummy_Num
 * @param [in] unsigned int u32Row_dummy_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetPreFltDummyNum(unsigned int u32Block_dummy_Num, unsigned int u32Row_dummy_Num);

/**
 * @brief This function set Dehalo post-filter dummy number
 * @param [in] unsigned int u32Block_dummy_Num
 * @param [in] unsigned int u32Row_dummy_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetPostFltDummyNum(unsigned int u32Block_dummy_Num, unsigned int u32Row_dummy_Num);

/**
 * @brief This function set me2 in 480x270 mode
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetME2in480x270Mode(BOOL bEnable);

/**
 * @brief This function set pre pattern block size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPrePattBlockSize(BOOL bEnable);

/**
 * @brief This function set pre pattern row size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPrePattRowSize(BOOL bEnable);

/**
 * @brief This function set middle pattern block size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetMedPattBlockSize(BOOL bEnable);

/**
 * @brief This function set middle pattern row size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetMedPattRowSize(BOOL bEnable);

/**
 * @brief This function set post pattern block size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPstPattBlockSize(BOOL bEnable);

/**
 * @brief This function set post pattern row size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPstPattRowSize(BOOL bEnable);

/**
 * @brief This function set phase mv erosion timing enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPHMVErosionTimingEnable(BOOL bEnable);

/**
 * @brief This function set RT1 erosion enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetRT1ErosionEnable(BOOL bEnable);

/**
 * @brief This function set cross board erosion enable only for FPGA
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetCrossBDErosionEnable(BOOL bEnable);

/**
 * @brief This function set disable dehalo occl rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_Dehalo_SetDisableOCCLRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

/**
 * @brief This function enable dehalo region interval x
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_Dehalo_SetRegionIntervalX(unsigned int u32Val);

/**
 * @brief This function enable dehalo region interval y
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_Dehalo_SetRegionIntervalY(unsigned int u32Val);

/**
 * @brief This function set dehalo logo row mask
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_Dehalo_SetLogoRowMask(unsigned int u32Val);

#endif

#endif

