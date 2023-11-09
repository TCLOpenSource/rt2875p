/**
 * @file hal_dehalo.c
 * @brief This file is for dehalo register setting
 * @date Nov.14.2014
 * @version V0.0.1
 * @author Martin Yan
 * @par Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#include "memc_isr/Common/kw_debug.h"
#include "memc_isr/Common/kw_common.h"
#include "memc_isr/Driver/regio.h"
#include "memc_reg_def.h"

/**
 * @brief This function set Dehalo on/off
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetEnable(BOOL bEnable)
{
	//reg_dh_en
	WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, (unsigned int)bEnable);
}

/**
 * @brief This function set Dehalo row size
 * @param [in] unsigned int u32Row_Size
 * @retval VOID
*/
VOID HAL_Dehalo_SetRowSize(unsigned int u32Row_Size)
{
	//reg_dh_mv_row_size
	WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,9,10,u32Row_Size);
}

/**
 * @brief This function set Dehalo block size
 * @param [in] unsigned int u32Block_Size
 * @retval VOID
*/
VOID HAL_Dehalo_SetBlockSize(unsigned int u32Block_Size)
{
	//reg_dh_blk_size
	WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,11,11,u32Block_Size);
}

/**
 * @brief This function set Dehalo logo row size
 * @param [in] unsigned int u32Row_Size
 * @retval VOID
*/
VOID HAL_Dehalo_SetLogoRowSize(unsigned int u32Row_Size)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,12,13,u32Row_Size);
}

/**
 * @brief This function set Dehalo logo block size
 * @param [in] unsigned int u32Block_Size
 * @retval VOID
*/
VOID HAL_Dehalo_SetLogoBlockSize(unsigned int u32Block_Size)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,14,14,u32Block_Size);
}

/**
 * @brief This function set Dehalo YX ratio
 * @param [in] unsigned int u32YXRatio
 * @retval VOID
*/
VOID HAL_Dehalo_SetYXRatio(unsigned int u32YXRatio)
{
	//reg_dh_blk_size
	WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,7,8,u32YXRatio);
}

/**
 * @brief This function set Dehalo block number
 * @param [in] unsigned int u32Block_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetBlockNum(unsigned int u32Block_Num)
{
	//reg_dh_blk_num[7:0]
	WriteRegister(KME_DEHALO_KME_DEHALO_10_reg,23,31,u32Block_Num);
}

/**
 * @brief This function set Dehalo row number
 * @param [in] unsigned int u32Row_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetRowNum(unsigned int u32Row_Num)
{
	//reg_dh_row_num[7:0]
	WriteRegister(KME_DEHALO_KME_DEHALO_14_reg,0,8,u32Row_Num);
}

#if 1  // K4Lp
/**
 * @brief This function set Dehalo logo block number
 * @param [in] unsigned int u32Block_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetLogoBlockNum(unsigned int u32Block_Num)
{
	//reg_dh_blk_num[7:0]
	WriteRegister(KME_DEHALO_KME_DEHALO_14_reg,9,17,u32Block_Num);
}

/**
 * @brief This function set Dehalo logo row number
 * @param [in] unsigned int u32Row_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetLogoRowNum(unsigned int u32Row_Num)
{
	//reg_dh_row_num[7:0]
	WriteRegister(KME_DEHALO_KME_DEHALO_14_reg,18,26,u32Row_Num);
}
#endif

/**
 * @brief This function set Dehalo vbuf block number
 * @param [in] unsigned int u32Block_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetVbufBlockNum(unsigned int u32Block_Num)
{
	//reg_dh_vbuf_blk_num
//	WriteRegister(FRC_TOP__KME_DEHALO__dh_vbuf_blk_num_ADDR no mat,FRC_TOP__KME_DEHALO__dh_vbuf_blk_num_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_vbuf_blk_num_BITEND no mat,u32Block_Num);
}

/**
 * @brief This function set Dehalo vbuf row number
 * @param [in] unsigned int u32Row_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetVbufRowNum(unsigned int u32Row_Num)
{
	//reg_dh_vbuf_row_num
//	WriteRegister(FRC_TOP__KME_DEHALO__dh_vbuf_row_num_ADDR no mat,FRC_TOP__KME_DEHALO__dh_vbuf_row_num_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_vbuf_row_num_BITEND no mat,u32Row_Num);
}

/**
 * @brief This function set Dehalo PR enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPREnable(BOOL bEnable)
{
	//reg_dh_pr_read
//	WriteRegister(FRC_TOP__KME_DEHALO2__dh_pr_read_ADDR no mat,FRC_TOP__KME_DEHALO2__dh_pr_read_BITSTART no mat,FRC_TOP__KME_DEHALO2__dh_pr_read_BITEND no mat, (unsigned int)bEnable);
}

/**
 * @brief This function set Dehalo MV RD intval
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetMVRDIntval(BOOL bEnable)
{
//	WriteRegister(FRC_TOP__KME_DEHALO__dh_mv_rd_intval_ADDR no mat,FRC_TOP__KME_DEHALO__dh_mv_rd_intval_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_mv_rd_intval_BITEND no mat, (unsigned int)bEnable);
}

/**
 * @brief This function set half v timing
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetHalfVTiming(BOOL bEnable)
{
//	WriteRegister(FRC_TOP__KME_DEHALO__dh_half_v_timing_ADDR no mat,FRC_TOP__KME_DEHALO__dh_half_v_timing_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_half_v_timing_BITEND no mat, (unsigned int)bEnable);
}

/**
 * @brief This function set patt size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPattSize(BOOL bEnable)
{
//	WriteRegister(FRC_TOP__KME_DEHALO2__dh_pre_patt_blk_size_ADDR no mat,FRC_TOP__KME_DEHALO2__dh_pre_patt_blk_size_BITSTART no mat,FRC_TOP__KME_DEHALO2__dh_pre_patt_blk_size_BITEND no mat, (unsigned int)bEnable);
//	WriteRegister(FRC_TOP__KME_DEHALO2__dh_pre_patt_row_size_ADDR no mat,FRC_TOP__KME_DEHALO2__dh_pre_patt_row_size_BITSTART no mat,FRC_TOP__KME_DEHALO2__dh_pre_patt_row_size_BITEND no mat, (unsigned int)bEnable);
//	WriteRegister(FRC_TOP__KME_DEHALO2__dh_pst_patt_blk_size_ADDR no mat,FRC_TOP__KME_DEHALO2__dh_pst_patt_blk_size_BITSTART no mat,FRC_TOP__KME_DEHALO2__dh_pst_patt_blk_size_BITEND no mat, (unsigned int)bEnable);
//	WriteRegister(FRC_TOP__KME_DEHALO2__dh_pst_patt_row_size_ADDR no mat,FRC_TOP__KME_DEHALO2__dh_pst_patt_row_size_BITSTART no mat,FRC_TOP__KME_DEHALO2__dh_pst_patt_row_size_BITEND no mat, (unsigned int)bEnable);
}

/**
 * @brief This function set out timing mux
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_Dehalo_SetOutTimimgMux(unsigned int u32Val)
{
//	WriteRegister(FRC_TOP__KME_DEHALO2__dh_out_tim_mux_ADDR no mat,FRC_TOP__KME_DEHALO2__dh_out_tim_mux_BITSTART no mat,FRC_TOP__KME_DEHALO2__dh_out_tim_mux_BITEND no mat, u32Val);
}

/**
 * @brief This function set rd lr flow wr latch
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetRDLRFlowWRLatch(BOOL bEnable)
{
//	WriteRegister(FRC_TOP__KME_DEHALO2__dh_rd_lr_flow_wrlatch_ADDR no mat,FRC_TOP__KME_DEHALO2__dh_rd_lr_flow_wrlatch_BITSTART no mat,FRC_TOP__KME_DEHALO2__dh_rd_lr_flow_wrlatch_BITEND no mat, (unsigned int)bEnable);
}

/**
 * @brief This function set sw reset
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetSWReset(BOOL bEnable)
{
//	WriteRegister(FRC_TOP__KME_DEHALO__dh_sw_reset_ADDR no mat,FRC_TOP__KME_DEHALO__dh_sw_reset_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_sw_reset_BITEND no mat, (unsigned int)bEnable);
}

/**
 * @brief This function set Dehalo patt block / row number
 * @param [in] unsigned int u32Block_Num
 * @param [in] unsigned int u32Row_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetPatternBlockRow(unsigned int u32Block_Num, unsigned int u32Row_Num)
{
//	WriteRegister(FRC_TOP__KME_DEHALO2__dh_patt_blk_num_ADDR no mat,FRC_TOP__KME_DEHALO2__dh_patt_blk_num_BITSTART no mat,FRC_TOP__KME_DEHALO2__dh_patt_blk_num_BITEND no mat,u32Block_Num);
//	WriteRegister(FRC_TOP__KME_DEHALO2__dh_patt_row_num_ADDR no mat,FRC_TOP__KME_DEHALO2__dh_patt_row_num_BITSTART no mat,FRC_TOP__KME_DEHALO2__dh_patt_row_num_BITEND no mat,u32Row_Num);
}

/**
 * @brief This function set Dehalo vbuf block / row number
 * @param [in] unsigned int u32Block_Num
 * @param [in] unsigned int u32Row_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetVbufBlockRow(unsigned int u32Block_Num, unsigned int u32Row_Num)
{
//	WriteRegister(FRC_TOP__KME_DEHALO__dh_vbuf_blk_num_ADDR no mat,FRC_TOP__KME_DEHALO__dh_vbuf_blk_num_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_vbuf_blk_num_BITEND no mat,u32Block_Num);
//	WriteRegister(FRC_TOP__KME_DEHALO__dh_vbuf_row_num_ADDR no mat,FRC_TOP__KME_DEHALO__dh_vbuf_row_num_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_vbuf_row_num_BITEND no mat,u32Row_Num);
}

/**
 * @brief This function set Dehalo show enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetShowEnable(BOOL bEnable)
{
//	WriteRegister(FRC_TOP__KME_DEHALO__dh_show_en_ADDR no mat,FRC_TOP__KME_DEHALO__dh_show_en_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_show_en_BITEND no mat,(unsigned int)bEnable);
}

/**
 * @brief This function set Dehalo show select
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetShowSelect(unsigned int u32Select)
{
//	WriteRegister(FRC_TOP__KME_DEHALO__dh_show_sel_ADDR no mat,FRC_TOP__KME_DEHALO__dh_show_sel_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_show_sel_BITEND no mat,u32Select);
}

/**
 * @brief This function set Dehalo erosion mode
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetErosionMode(BOOL bEnable)
{
//	WriteRegister(FRC_TOP__KME_DEHALO2__dh_ero_mod_ADDR no mat,FRC_TOP__KME_DEHALO2__dh_ero_mod_BITSTART no mat,FRC_TOP__KME_DEHALO2__dh_ero_mod_BITEND no mat,(BOOL)bEnable);
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_DEHALO__dh_rt1_ersion_en_ADDR no mat,FRC_TOP__KME_DEHALO__dh_rt1_ersion_en_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_rt1_ersion_en_BITEND no mat,(unsigned int)bEnable);
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_DEHALO__dh_phbv0_ersion_en_ADDR no mat,FRC_TOP__KME_DEHALO__dh_phbv0_ersion_en_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_phbv0_ersion_en_BITEND no mat,(unsigned int)bEnable);
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_DEHALO__dh_phbv1_ersion_en_ADDR no mat,FRC_TOP__KME_DEHALO__dh_phbv1_ersion_en_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_phbv1_ersion_en_BITEND no mat,(unsigned int)bEnable);
}

/**
 * @brief This function set Dehalo pre-filter dummy number
 * @param [in] unsigned int u32Block_dummy_Num
 * @param [in] unsigned int u32Row_dummy_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetPreFltDummyNum(unsigned int u32Block_dummy_Num, unsigned int u32Row_dummy_Num)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_D8_reg,6,9, u32Block_dummy_Num);
	WriteRegister(KME_DEHALO_KME_DEHALO_D8_reg,10,13, u32Row_dummy_Num);
}

/**
 * @brief This function set Dehalo post-filter dummy number
 * @param [in] unsigned int u32Block_dummy_Num
 * @param [in] unsigned int u32Row_dummy_Num
 * @retval VOID
*/
VOID HAL_Dehalo_SetPostFltDummyNum(unsigned int u32Block_dummy_Num, unsigned int u32Row_dummy_Num)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_C0_reg,16,19, u32Block_dummy_Num);
	WriteRegister(KME_DEHALO_KME_DEHALO_C0_reg,20,23, u32Row_dummy_Num);
}

/**
 * @brief This function set me2 in 480x270 mode
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetME2in480x270Mode(BOOL bEnable)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_14_reg,27,27,(unsigned int)bEnable);
}

/**
 * @brief This function set pre pattern block size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPrePattBlockSize(BOOL bEnable)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_18_reg,27,27, (unsigned int)bEnable);
}

/**
 * @brief This function set pre pattern row size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPrePattRowSize(BOOL bEnable)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_18_reg,28,28, (unsigned int)bEnable);
}

/**
 * @brief This function set middle pattern block size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetMedPattBlockSize(BOOL bEnable)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_18_reg,29,29, (unsigned int)bEnable);
}

/**
 * @brief This function set middle pattern row size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetMedPattRowSize(BOOL bEnable)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_18_reg,30,30, (unsigned int)bEnable);
}

/**
 * @brief This function set post pattern block size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPstPattBlockSize(BOOL bEnable)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_34_reg,18,18, (unsigned int)bEnable);
}

/**
 * @brief This function set post pattern row size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPstPattRowSize(BOOL bEnable)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_34_reg,19,19, (unsigned int)bEnable);
}

/**
 * @brief This function set phase mv erosion timing enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetPHMVErosionTimingEnable(BOOL bEnable)
{
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_DEHALO__dh_phmv_ero_timg_en_ADDR no mat,FRC_TOP__KME_DEHALO__dh_phmv_ero_timg_en_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_phmv_ero_timg_en_BITEND no mat, (unsigned int)bEnable);
}

/**
 * @brief This function set RT1 erosion enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetRT1ErosionEnable(BOOL bEnable)
{
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_DEHALO__dh_rt1_ero_en_ADDR no mat,FRC_TOP__KME_DEHALO__dh_rt1_ero_en_BITSTART no mat,FRC_TOP__KME_DEHALO__dh_rt1_ero_en_BITEND no mat, (unsigned int)bEnable);
}

/**
 * @brief This function set cross board erosion enable only for FPGA
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_Dehalo_SetCrossBDErosionEnable(BOOL bEnable)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_BC_reg,26,26, (unsigned int)bEnable);
}

/**
 * @brief This function set disable dehalo occl rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_Dehalo_SetDisableOCCLRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_E4_reg,0,8, u32Top);	
	WriteRegister(KME_DEHALO_KME_DEHALO_E4_reg,9,17, u32Bottom);	
	WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8, u32Left);	
	WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17, u32Right);	
}

/**
 * @brief This function enable dehalo region interval x
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_Dehalo_SetRegionIntervalX(unsigned int u32Val)
{
	WriteRegister(KME_DEHALO2_KME_DEHALO2_30_reg,2,8, u32Val);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_30_reg,25,31, u32Val);
}

/**
 * @brief This function enable dehalo region interval y
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_Dehalo_SetRegionIntervalY(unsigned int u32Val)
{
	WriteRegister(KME_DEHALO2_KME_DEHALO2_30_reg,9,16, u32Val);
	WriteRegister(KME_DEHALO2_KME_DEHALO2_34_reg,0,7, u32Val);
}

/**
 * @brief This function set dehalo logo row mask
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_Dehalo_SetLogoRowMask(unsigned int u32Val)
{
	WriteRegister(KME_DEHALO_KME_DEHALO_1C_reg,28,28, u32Val);
}
