/**
 * @file hal_me.c
 * @brief This file is for kme me register setting
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
#include "memc_isr/HAL/hal_me.h"
#include "memc_reg_def.h"
#ifdef CONFIG_ARM64 //ARM32 compatible
#include <linux/kernel.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include "memc_isr/PQL/PQLPlatformDefs.h"
//#include <rtd_log/rtd_module.h>

#undef rtd_outl
#define rtd_outl(x, y)     								IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)     									IoReg_Read32(x)
#undef rtd_maskl
#define rtd_maskl(x, y, z)     							IoReg_Mask32(x,y,z)
#undef rtd_setbits
#define rtd_setbits(offset, Mask) rtd_outl(offset, (rtd_inl(offset) | Mask))
#undef rtd_clearbits
#define rtd_clearbits(offset, Mask) rtd_outl(offset, ((rtd_inl(offset) & ~(Mask))))
#else
#include "io.h"
#endif



/**
 * @brief This function set KME ipme in mode
 * @param [in] unsigned int u32In_mode
 * @retval VOID
*/
VOID HAL_KME_IPME_SetInMode(unsigned int u32In_mode)
{
	//reg_ipme_in_mode
//	WriteRegister(FRC_TOP__KME_IPME__ipme_in_mode_ADDR no mat,FRC_TOP__KME_IPME__ipme_in_mode_BITSTART no mat,FRC_TOP__KME_IPME__ipme_in_mode_BITEND no mat,u32In_mode);
}

/**
 * @brief This function set KME ipme h fetch mode
 * @param [in] unsigned int u32HFetch_Mode
 * @retval VOID
*/
VOID HAL_KME_IPME_SetHFetchMode(unsigned int u32HFetch_Mode)
{
	//reg_ipme_h_fetch_mode
	WriteRegister(KME_IPME_KME_IPME_00_reg,6,7,u32HFetch_Mode);	
}

/**
 * @brief This function set KME ipme v fetch mode
 * @param [in] unsigned int u32VFetch_Mode
 * @retval VOID
*/
VOID HAL_KME_IPME_SetVFetchMode(unsigned int u32VFetch_Mode)
{
	//reg_ipme_v_fetch_mode
	WriteRegister(KME_IPME_KME_IPME_00_reg,10,11,u32VFetch_Mode);	
}

/**
 * @brief This function set KME ipme2 v fetch mode
 * @param [in] unsigned int u32VFetch_Mode
 * @retval VOID
*/
VOID HAL_KME_IPME2_SetVFetchMode(unsigned int u32VFetch_Mode)
{
	//reg_ipme_v_fetch_mode
//	WriteRegister(FRC_TOP__KME_IPME1__ipme_v_480x270_mode_ADDR no mat,FRC_TOP__KME_IPME1__ipme_v_480x270_mode_BITSTART no mat,FRC_TOP__KME_IPME1__ipme_v_480x270_mode_BITEND no mat,u32VFetch_Mode);	
}

/**
 * @brief This function set KME ipme h center value
 * @param [in] unsigned int u32H_Center_Value
 * @retval VOID
*/
VOID HAL_KME_IPME_SetHCenterValue(unsigned int u32H_Center_Value)
{
	//reg_ipme_h_center_value
//	WriteRegister(FRC_TOP__KME_IPME__ipme_h_center_value_ADDR no mat,FRC_TOP__KME_IPME__ipme_h_center_value_BITSTART no mat,FRC_TOP__KME_IPME__ipme_h_center_value_BITEND no mat,u32H_Center_Value);
}

/**
 * @brief This function set KME ipme v center value
 * @param [in] unsigned int u32V_Center_Value
 * @retval VOID
*/
VOID HAL_KME_IPME_SetVCenterValue(unsigned int u32V_Center_Value)
{
	//reg_ipme_v_center_value
//	WriteRegister(FRC_TOP__KME_IPME__ipme_v_center_value_ADDR no mat,FRC_TOP__KME_IPME__ipme_v_center_value_BITSTART no mat,FRC_TOP__KME_IPME__ipme_v_center_value_BITEND no mat,u32V_Center_Value);
}

/**
 * @brief This function set KME ipme2 v center value
 * @param [in] unsigned int u32V_Center_Value
 * @retval VOID
*/
VOID HAL_KME_IPME2_SetVCenterValue(unsigned int u32V_Center_Value)
{
	//reg_ipme_v_480x270_i_v_center
//	WriteRegister(FRC_TOP__KME_IPME1__ipme_v_480x270_i_v_center_ADDR no mat,FRC_TOP__KME_IPME1__ipme_v_480x270_i_v_center_BITSTART no mat,FRC_TOP__KME_IPME1__ipme_v_480x270_i_v_center_BITEND no mat,u32V_Center_Value);
}

/**
 * @brief This function set KME ipme h scaler active
 * @param [in] unsigned int u32Scaler_Hactive
 * @retval VOID
*/
VOID HAL_KME_IPME_SetScalerHactive(unsigned int u32Scaler_Hactive)
{
	//reg_ipme_scaler_h_active
	WriteRegister(KME_IPME_KME_IPME_0C_reg,0,9,u32Scaler_Hactive);
}

/**
 * @brief This function set KME ipme pscaler v active
 * @param [in] unsigned int u32Pscaler_Vactive
 * @retval VOID
*/
VOID HAL_KME_IPME_SetPscalerVactive(unsigned int u32Pscaler_Vactive)
{
	//reg_ipme_pscaler_v_active
	WriteRegister(KME_IPME_KME_IPME_24_reg,0,11,u32Pscaler_Vactive);
}

/**
 * @brief This function set KME LBME total line number
 * @param [in] u32 u32TotalLineNum
 * @retval VOID
*/
VOID HAL_KME_LBME_SetTotalLineNum(unsigned int u32TotalLineNum)
{
	// reg_lbme_total_line_num
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_14_reg,0,9,u32TotalLineNum);	
}

/**
 * @brief This function set KME LBME rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_LBME_SetRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_04_reg,0,10,u32Top);
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_04_reg,11,21,u32Bottom);	
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_00_reg,0,10,u32Left);	
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_00_reg,11,21,u32Right);
}

#if 1  // K4Lp
/**
 * @brief This function set KME LBME meander rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_LBME_SetMeanderRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_20_reg,0,10,u32Top);
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_20_reg,11,21,u32Bottom);	
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_1C_reg,0,10,u32Left);	
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_1C_reg,11,21,u32Right);
}

/**
 * @brief This function set KME LBME2 rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_LBME2_SetRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(KME_LBME2_TOP_KME_LBME2_TOP_04_reg,0,10,u32Top);
	WriteRegister(KME_LBME2_TOP_KME_LBME2_TOP_04_reg,11,21,u32Bottom);	
	WriteRegister(KME_LBME2_TOP_KME_LBME2_TOP_00_reg,0,10,u32Left);	
	WriteRegister(KME_LBME2_TOP_KME_LBME2_TOP_00_reg,11,21,u32Right);
}
#endif

/**
 * @brief This function set KME LBME total pixel number
 * @param [in] u32 u32TotalPixelNum
 * @retval VOID
*/
VOID HAL_KME_LBME_SetTotalPixelNum(unsigned int u32TotalPixelNum)
{
	// reg_lbme_total_pixel_num
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_14_reg,10,19,u32TotalPixelNum);	
}

/**
 * @brief This function set KME LBME block number per line
 * @param [in] u32 u32blk_num_perline
 * @retval VOID
*/
VOID HAL_KME_LBME_SetBlkNumPerLine(unsigned int u32blk_num_perline)
{
	// reg_lbme_blk_num_perline
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_18_reg,16,23,u32blk_num_perline);
}

/**
 * @brief This function set KME LBME debug hactive
 * @param [in] u32 u32debug_hactive
 * @retval VOID
*/
VOID HAL_KME_LBME_SetDebugHactive(unsigned int u32debug_hactive)
{
	// reg_lbme_debug_hact
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_10_reg,20,27,u32debug_hactive);
}

/**
 * @brief This function set KME LBME total linebuf number
 * @param [in] u32 u32TotalLinebufNum
 * @retval VOID
*/
VOID HAL_KME_LBME_SetTotalLinebufNum(unsigned int u32TotalLinebufNum)
{
	// reg_lbme_total_linebuf_num
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_18_reg,0,7,u32TotalLinebufNum);	
}

/**
 * @brief This function set KME LBME2 total line number
 * @param [in] u32 u32TotalLineNum
 * @retval VOID
*/
VOID HAL_KME2_LBME_SetTotalLineNum(unsigned int u32TotalLineNum)
{
	// reg_me2_lbme_total_line_num
#if 1
	WriteRegister(KME_LBME2_TOP_KME_LBME2_TOP_14_reg,0,9,u32TotalLineNum);	
#else
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_14_reg,0,9,u32TotalLineNum);	
#endif
}

/**
 * @brief This function set KME LBME2 total pixel number
 * @param [in] u32 u32TotalPixelNum
 * @retval VOID
*/
VOID HAL_KME2_LBME_SetTotalPixelNum(unsigned int u32TotalPixelNum)
{
	// reg_me2_lbme_total_pixel_num
	WriteRegister(KME_LBME2_TOP_KME_LBME2_TOP_14_reg,10,19,u32TotalPixelNum);	
}

/**
 * @brief This function set KME2 LBME block number per line
 * @param [in] u32 u32blk_num_perline
 * @retval VOID
*/
VOID HAL_KME2_LBME_SetBlkNumPerLine(unsigned int u32blk_num_perline)
{
	// reg_me2_lbme_blk_num_perline
#if 1
	WriteRegister(KME_LBME2_TOP_KME_LBME2_TOP_14_reg,0,9,u32blk_num_perline);
#else
	WriteRegister(KME_LBME_TOP_KME_LBME_TOP_18_reg,16,23,u32blk_num_perline);
#endif
}

/**
 * @brief This function set KME2 LBME debug hactive
 * @param [in] u32 u32debug_hactive
 * @retval VOID
*/
VOID HAL_KME2_LBME_SetDebugHactive(unsigned int u32debug_hactive)
{
	// reg_me2_lbme_debug_hact
	//WriteRegister(FRC_TOP__KME_ME2_LBME_TOP__reg_me2_lbme_debug_hact_ADDR no mat,FRC_TOP__KME_ME2_LBME_TOP__reg_me2_lbme_debug_hact_BITSTART no mat,FRC_TOP__KME_ME2_LBME_TOP__reg_me2_lbme_debug_hact_BITEND no mat,u32debug_hactive);
}

/**
 * @brief This function set KME2 LBME total linebuf number
 * @param [in] u32 u32TotalLinebufNum
 * @retval VOID
*/
VOID HAL_KME2_LBME_SetTotalLinebufNum(unsigned int u32TotalLinebufNum)
{
	// reg_me2_lbme_total_linebuf_num
	//WriteRegister(FRC_TOP__KME_ME2_LBME_TOP__reg_me2_lbme_total_linebuf_num_ADDR no mat,FRC_TOP__KME_ME2_LBME_TOP__reg_me2_lbme_total_linebuf_num_BITSTART no mat,FRC_TOP__KME_ME2_LBME_TOP__reg_me2_lbme_total_linebuf_num_BITEND no mat,u32TotalLinebufNum);	
}

/**
 * @brief This function set kme vbuf hactive
 * @param [in] unsigned int u32VBuf_Hactive
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_Hactive(unsigned int u32VBuf_Hactive)
{
	// reg_vbuf_hactive
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_00_reg,0,8,u32VBuf_Hactive);
}

/**
 * @brief This function set kme vbuf vactive
 * @param [in] unsigned int u32VBuf_Vactive
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_Vactive(unsigned int u32VBuf_Vactive)
{
	// reg_vbuf_vactive
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_00_reg,9,17,u32VBuf_Vactive);
}

/**
 * @brief This function set lbme update num
 * @param [in] unsigned int u32Lbme_Update_Num
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_LBMEUpdateNum(unsigned int u32Lbme_Update_Num)
{
	// reg_vbuf_lbme_update_num_1row
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_00_reg,19,26,u32Lbme_Update_Num);
}

/**
 * @brief This function set lbme trig mid row num
 * @param [in] unsigned int u32Lbme_TrigMidRow_Num
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_LBMETrigMidRowNum(unsigned int u32Lbme_TrigMidRow_Num)
{
	// reg_vbuf_lbme_trig_mid_row_num
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_30_reg,0,8,u32Lbme_TrigMidRow_Num);
}

/**
 * @brief This function set lbme h shift bit
 * @param [in] unsigned int u32Lbme_HShift
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_LBMEHShift(unsigned int u32Lbme_HShift)
{
	// reg_vbuf_lbme_h_addr_shift_bit
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_48_reg,20,20,u32Lbme_HShift);
}

/**
 * @brief This function set lbme v shift bit
 * @param [in] unsigned int u32Lbme_VShift
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_LBMEVShift(unsigned int u32Lbme_VShift)
{
	// reg_vbuf_lbme_v_addr_shift_bit
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_48_reg,21,21,u32Lbme_VShift);
}

/**
 * @brief This function set kme2 vbuf pfv hactive
 * @param [in] unsigned int u32VBuf_Hactive
 * @retval VOID
*/
VOID HAL_KME2_VBUF_Set_PFVHactive(unsigned int u32VBuf_Hactive)
{
	// reg_kme_me2_vbuf_pfv_blksize_hact
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_1C_reg,0,8,u32VBuf_Hactive);
}

/**
 * @brief This function set kme vbuf pfv vactive
 * @param [in] unsigned int u32VBuf_Vactive
 * @retval VOID
*/
VOID HAL_KME2_VBUF_Set_PFVVactive(unsigned int u32VBuf_Vactive)
{
	// reg_kme_me2_vbuf_pfv_blksize_vact
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_1C_reg,9,17,u32VBuf_Vactive);
}

#if 1
/**
 * @brief This function set kme2 vbuf phase hactive
 * @param [in] unsigned int u32VBuf_Hactive
 * @retval VOID
*/
VOID HAL_KME2_VBUF_Set_PHHactive(unsigned int u32VBuf_Hactive)
{
	// reg_kme_me2_vbuf_pfv_blksize_hact
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_1C_reg,18,26,u32VBuf_Hactive);
}

/**
 * @brief This function set kme vbuf phase vactive
 * @param [in] unsigned int u32VBuf_Vactive
 * @retval VOID
*/
VOID HAL_KME2_VBUF_Set_PHVactive(unsigned int u32VBuf_Vactive)
{
	// reg_kme_me2_vbuf_ph_blksize_vact
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_20_reg,0,8,u32VBuf_Vactive);
}
#endif

/**
 * @brief This function set kme2 vbuf 3dpr enable
 * @param [in] unsigned int u32VBuf_Vactive
 * @retval VOID
*/
VOID HAL_KME2_VBUF_Set_3DPREnable(BOOL bEnable)
{
	// reg_kme_me2_vbuf_3dpr_en
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_18_reg,23,23,(unsigned int)bEnable);
}

/**
 * @brief This function set kme logo0 h fetch mode
 * @param [in] unsigned int u32Logo_Hfetch_mode
 * @retval VOID
*/
VOID HAL_KME_LOGO0_Set_HFetchMode(unsigned int u32Logo_Hfetch_mode)
{
	// reg_km_logo_h_fetch_mode   
	WriteRegister(KME_LOGO0_KME_LOGO0_00_reg,0,1,u32Logo_Hfetch_mode);
}

/**
 * @brief This function set kme logo0 v fetch mode
 * @param [in] unsigned int u32Logo_Vfetch_mode
 * @retval VOID
*/
VOID HAL_KME_LOGO0_Set_VFetchMode(unsigned int u32Logo_Vfetch_mode)
{
	// reg_km_logo_v_fetch_mode   
	WriteRegister(KME_LOGO0_KME_LOGO0_00_reg,2,3,u32Logo_Vfetch_mode);	
}

/**
 * @brief This function set kme logo0 pscaler v active
 * @param [in] unsigned int u32Logo_Pscaler_Vactive
 * @retval VOID
*/
VOID HAL_KME_LOGO0_Set_PscalerVactive(unsigned int u32Logo_Pscaler_Vactive)
{
	// reg_km_logo_pscaler_v_active   
	WriteRegister(KME_LOGO0_KME_LOGO0_00_reg,20,31,u32Logo_Pscaler_Vactive);	
}

/**
 * @brief This function set clear MV enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_METOP1_Set_ClearMVEnable(BOOL bEnable)
{
	// reg_metop_sc_clr_sw_control_en   
	//WriteRegister(FRC_TOP__KME_ME_TOP1__reg_metop_sc_clr_sw_control_en_ADDR no mat,FRC_TOP__KME_ME_TOP1__reg_metop_sc_clr_sw_control_en_BITSTART no mat,FRC_TOP__KME_ME_TOP1__reg_metop_sc_clr_sw_control_en_BITEND no mat, (unsigned int)bEnable);	
	WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg,0,0, (unsigned int)bEnable);	
}

/**
 * @brief This function set me1_sc_en
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_METOP1_Set_SC_en(BOOL bEnable)
{
	// reg_me1_sc_me1_en
	//WriteRegister(FRC_TOP__KME_ME_TOP1__reg_metop_sc_clr_sw_control_en_ADDR no mat,FRC_TOP__KME_ME_TOP1__reg_metop_sc_clr_sw_control_en_BITSTART no mat,FRC_TOP__KME_ME_TOP1__reg_metop_sc_clr_sw_control_en_BITEND no mat, (unsigned int)bEnable);	
	WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg,9,9, (unsigned int)bEnable);	
	WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg,10,10, (unsigned int)bEnable);	
	WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg,11,11, (unsigned int)bEnable);	
	WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg,12,12, (unsigned int)bEnable);	
	WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg,13,13, (unsigned int)bEnable);	
}

/**
 * @brief This function set me1 rmv size
 * @param [in] unsigned int u32RMV_Hact
 * @param [in] unsigned int u32RMV_Vact
 * @retval VOID
*/
VOID HAL_KME_ME1_Set_RMVSize(unsigned int u32RMV_Hact, unsigned int u32RMV_Vact)
{
	WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_04_reg,0,8, u32RMV_Hact);	
	WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_04_reg,9,17, u32RMV_Vact);
}

/**
 * @brief This function set me2 rmv size
 * @param [in] unsigned int u32RMV_Hact
 * @param [in] unsigned int u32RMV_Vact
 * @retval VOID
*/
VOID HAL_KME_ME2_Set_RMVSize(unsigned int u32RMV_Hact, unsigned int u32RMV_Vact)
{
	WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_74_reg,13,21, u32RMV_Hact);	
	WriteRegister(KME_ME2_CALC0_KME_ME2_CALC0_74_reg,22,30, u32RMV_Vact);
	WriteRegister(KME_ME2_CALC1_KME_ME2_CALC1_3C_reg,21,29,u32RMV_Hact);
	WriteRegister(KME_ME2_CALC1_KME_ME2_CALC1_40_reg,0,8,u32RMV_Vact);
}

/**
 * @brief This function set me1 static size
 * @param [in] unsigned int u32Statis_Hact
 * @param [in] unsigned int u32Statis_Vact
 * @retval VOID
*/
VOID HAL_KME_ME1_Set_StatisSize(unsigned int u32Statis_Hact, unsigned int u32Statis_Vact)
{
	WriteRegister(KME_ME1_TOP6_KME_ME1_TOP6_00_reg,0,8, u32Statis_Hact);	
	WriteRegister(KME_ME1_TOP6_KME_ME1_TOP6_00_reg,9,17, u32Statis_Vact);
}

#if 1
/**
 * @brief This function set ph pfv h ratio
 * @param [in] unsigned int u32ph_pfv_h_ratio
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_PHPFHRatio(unsigned int u32ph_pfv_h_ratio)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,3,4,u32ph_pfv_h_ratio);
}
#endif

/**
 * @brief This function set ph pfv v ratio
 * @param [in] unsigned int u32ph_pfv_v_ratio
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_PHPFVRatio(unsigned int u32ph_pfv_v_ratio)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,1,2,u32ph_pfv_v_ratio);
}

#if 1
/**
 * @brief This function set ph lbme h ratio
 * @param [in] unsigned int u32ph_lbme_h_ratio
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_PHLBMEHRatio(unsigned int u32ph_lbme_h_ratio)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_C8_reg,11,12,u32ph_lbme_h_ratio);
}

/**
 * @brief This function set ph lbme v ratio
 * @param [in] unsigned int u32ph_lbme_v_ratio
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_PHLBMEVRatio(unsigned int u32ph_lbme_v_ratio)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_C8_reg,13,14,u32ph_lbme_v_ratio);
}
#endif

/**
 * @brief This function set pfv mv fetch in 1 row
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_PFVMVFetchIn1Row(unsigned int u32val)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_04_reg,20,21,u32val);
}	

/**
 * @brief This function set ppfv mv fetch in 1 row
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_PPFVMVFetchIn1Row(unsigned int u32val)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_0C_reg,12,13,u32val);
}

/**
 * @brief This function set vbuf blk vact
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_BlockVact(unsigned int u32val)
{
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_TOP__kme_vbuf_blk_vact_ADDR no mat,FRC_TOP__KME_TOP__kme_vbuf_blk_vact_BITSTART no mat,FRC_TOP__KME_TOP__kme_vbuf_blk_vact_BITEND no mat,u32val);
}

/**
 * @brief This function set vbuf row type
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_RowType(unsigned int u32val)
{
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_TOP__kme_vbuf_row_type_ADDR no mat,FRC_TOP__KME_TOP__kme_vbuf_row_type_BITSTART no mat,FRC_TOP__KME_TOP__kme_vbuf_row_type_BITEND no mat,u32val);
}

/**
 * @brief This function set vbuf wait cycle
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_VBUF_Set_WaitCycle(unsigned int u32MV_Trig, unsigned int u32ME_Trig)
{
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_34_reg,0,17,u32MV_Trig);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_38_reg,0,17,u32ME_Trig);
}

/**
 * @brief This function set work mode
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_Set_KME_WorkMode(unsigned int u32val)
{
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_TOP__kphase_me_work_mode_ADDR no mat,FRC_TOP__KME_TOP__kphase_me_work_mode_BITSTART no mat,FRC_TOP__KME_TOP__kphase_me_work_mode_BITEND no mat,u32val);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_30_reg,9,10,u32val);
// MEMC_K6L_BRING_UP //	WriteRegister(CRTC1_CRTC1_1C_reg,26,27,u32val);
}

/**
 * @brief This function set work mode
 * @param [in] unsigned int u32val
 * @retval VOID
*/
VOID HAL_KME_Set_Vspll_WorkMode(unsigned int u32val)
{
	WriteRegister(CRTC1_CRTC1_1C_reg,26,27,u32val);
}

/**
 * @brief This function set meander enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_MeanderEnable(BOOL bEnable)
{
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_TOP__frame_meander_enable_ADDR no mat,FRC_TOP__KME_TOP__frame_meander_enable_BITSTART no mat,FRC_TOP__KME_TOP__frame_meander_enable_BITEND no mat, (unsigned int)bEnable);	
}

/**
 * @brief This function set LBME2 3DPR Enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_LBME23DPREnable(BOOL bEnable)
{
//	WriteRegister(FRC_TOP__KME_ME2_LBME_TOP__reg_me2_lbme_3d_en_ADDR no mat,FRC_TOP__KME_ME2_LBME_TOP__reg_me2_lbme_3d_en_BITSTART no mat,FRC_TOP__KME_ME2_LBME_TOP__reg_me2_lbme_3d_en_BITEND no mat, (unsigned int)bEnable);	
}

#if 1
/**
 * @brief This function set ME1 vbuf pixel rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_Set_ME1VbufPixelRIM(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_44_reg,0,9, u32Top);	
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_44_reg,10,19, u32Bottom);	
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_48_reg,0,9, u32Left);	
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_48_reg,10,19, u32Right);	
}
#endif

/**
 * @brief This function set ME1 vbuf block rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_Set_ME1VbufBlockRIM(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,16,24, u32Top);	
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_40_reg,18,26, u32Bottom);	
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_40_reg,0,8, u32Left);	
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_40_reg,9,17, u32Right);	
}

/**
 * @brief This function set ME2 vbuf pfv block rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_Set_ME2VbufPFVBlockRIM(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_24_reg,0,8, u32Top);	
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_24_reg,9,17, u32Bottom);	
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_20_reg,9,17, u32Left);	
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_20_reg,18,26, u32Right);	
}

#if 1
/**
 * @brief This function set ME2 vbuf phase block rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_Set_ME2VbufPHBlockRIM(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_28_reg,9,17, u32Top);	
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_28_reg,18,26, u32Bottom);	
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_24_reg,18,26, u32Left);	
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_28_reg,0,8, u32Right);	
}

/**
 * @brief This function set ME2 vbuf pixel rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_Set_ME2VbufPixelRIM(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,0,9, u32Top);	
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,10,19, u32Bottom);	
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D4_reg,12,21, u32Left);	
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D4_reg,22,31, u32Right);	
}

/**
 * @brief This function set me2 135 to 270
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_SetME2135To270(BOOL bEnable)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,17,17,(unsigned int)bEnable);	
}

/**
 * @brief This function set me2 135 to 270 read interval
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_SetME2135To270RDInterval(unsigned int u32Val)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,18,24,u32Val);	
}

/**
 * @brief This function set me2 135 to 270 wait row numbers
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_SetME2135To270WaitRowNum(unsigned int u32Val)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_E0_reg,25,28,u32Val);	
}
#endif

/**
 * @brief This function set BBD window
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_Set_BBDWindow(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	bbd_bbd_window_start_point_RBUS bbd_bbd_window_start_point_reg;
	bbd_bbd_window_end_point_RBUS bbd_bbd_window_end_point_reg;

	bbd_bbd_window_start_point_reg.regValue = rtd_inl(BBD_BBD_WINDOW_START_POINT_reg);
	bbd_bbd_window_end_point_reg.regValue = rtd_inl(BBD_BBD_WINDOW_END_POINT_reg);

	bbd_bbd_window_start_point_reg.bbd_window_x_start = u32Top;
	bbd_bbd_window_start_point_reg.bbd_window_y_start = u32Left;
	bbd_bbd_window_end_point_reg.bbd_window_x_end = u32Bottom;
	bbd_bbd_window_end_point_reg.bbd_window_y_end = u32Right;

	rtd_outl(BBD_BBD_WINDOW_START_POINT_reg, bbd_bbd_window_start_point_reg.regValue);
	rtd_outl(BBD_BBD_WINDOW_END_POINT_reg, bbd_bbd_window_end_point_reg.regValue);
}

/**
 * @brief This function set ME2 LR Dec enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_ME2LRDecEnable(BOOL bEnable)
{
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_TOP__kphase_me2_lr_dec_en_ADDR no mat,FRC_TOP__KME_TOP__kphase_me2_lr_dec_en_BITSTART no mat,FRC_TOP__KME_TOP__kphase_me2_lr_dec_en_BITEND no mat, (unsigned int)bEnable);	
}

/**
 * @brief This function set 3d ppfv index
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_3DPPFVIDX(BOOL bEnable)
{
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_TOP__kphase_3d_ppfv_wr_idx_position_ADDR no mat,FRC_TOP__KME_TOP__kphase_3d_ppfv_wr_idx_position_BITSTART no mat,FRC_TOP__KME_TOP__kphase_3d_ppfv_wr_idx_position_BITEND no mat, (unsigned int)bEnable);	
}

/**
 * @brief This function set crtc lr enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_SetLREnable(BOOL bEnable)
{
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_TOP__kphase_3dsg_llrr_en_ADDR no mat,FRC_TOP__KME_TOP__kphase_3dsg_llrr_en_BITSTART no mat,FRC_TOP__KME_TOP__kphase_3dsg_llrr_en_BITEND no mat,(unsigned int)bEnable);	
}

/**
 * @brief This function set crtc lr cycle
 * @param [in] unsigned int u32Cycle
 * @retval VOID
*/
VOID HAL_KME_SetLRCycle(unsigned int u32Cycle)
{
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_TOP__kphase_me1_llrr_nums_ADDR no mat,FRC_TOP__KME_TOP__kphase_me1_llrr_nums_BITSTART no mat,FRC_TOP__KME_TOP__kphase_me1_llrr_nums_BITEND no mat,u32Cycle);	
}

/**
 * @brief This function set crtc lr pattern
 * @param [in] unsigned int u32Pattern
 * @retval VOID
*/
VOID HAL_KME_SetLRPattern(unsigned int u32Pattern)
{
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_TOP__kphase_me1_llrr_patt_ADDR no mat,FRC_TOP__KME_TOP__kphase_me1_llrr_patt_BITSTART no mat,FRC_TOP__KME_TOP__kphase_me1_llrr_patt_BITEND no mat,u32Pattern);	
}

/**
 * @brief This function set ME1 HV Ratio
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_ME1HVRatio(BOOL bEnable)
{
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_48_reg,22,22, (unsigned int)bEnable);	
}

/**
 * @brief This function set meander block rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @retval VOID
*/
VOID HAL_KME_SetMeanderBLKRIM(unsigned int u32Top, unsigned int u32Bottom)
{
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_84_reg,0,8,u32Top);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_84_reg,9,17,u32Bottom);	
}

/**
 * @brief This function set meander pixel rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @retval VOID
*/
VOID HAL_KME_SetMeanderPIXRIM(unsigned int u32Top, unsigned int u32Bottom)
{
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_84_reg,18,27,u32Top);
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_TOP__vbuf_meander_pix_rim_bot_ADDR no mat,FRC_TOP__KME_TOP__vbuf_meander_pix_rim_bot_BITSTART no mat,FRC_TOP__KME_TOP__vbuf_meander_pix_rim_bot_BITEND no mat,u32Bottom);	
}

/**
 * @brief This function set DM PFV PPFV interval
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_KME_ME2_SetDMPFVPPFVInterval(unsigned int u32Val)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_04_reg,8,15,u32Val);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_0C_reg,0,7,u32Val);	
}

#if 1
/**
 * @brief This function set lbme dm number in 1 row
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_KME_ME2_SetVbufLBMEDMNum1Row(unsigned int u32Val)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_38_reg,17,25,u32Val);
}

/**
 * @brief This function get 1st GMV offset number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stGMVOffetNum(VOID)
{
	unsigned int u32Val = 0;
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,18,20, &u32Val);
	return u32Val;
}

/**
 * @brief This function get 1st phase candidate number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stPHCandiNum(VOID)
{
	unsigned int u32Val = 0;
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,21,25, &u32Val);
	return u32Val;
}

/**
 * @brief This function get 1st phase candidate offset number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stPHCandiOffsetNum(VOID)
{
	unsigned int u32Val = 0;
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,26,28, &u32Val);
	return u32Val;
}

/**
 * @brief This function get 1st phv0 candidate number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stPFV0CandiNum(VOID)
{
	unsigned int u32Val = 0;
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,0,4, &u32Val);
	return u32Val;
}

/**
 * @brief This function get 1st phv0 candidate offset number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stPFV0CandiOffsetNum(VOID)
{
	unsigned int u32Val = 0;
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,5,7, &u32Val);
	return u32Val;
}

/**
 * @brief This function get 1st phv1 candidate number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stPFV1CandiNum(VOID)
{
	unsigned int u32Val = 0;
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,8,12, &u32Val);
	return u32Val;
}

/**
 * @brief This function get 1st phv1 candidate offset number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stPFV1CandiOffsetNum(VOID)
{
	unsigned int u32Val = 0;
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,13,15, &u32Val);
	return u32Val;
}

/**
 * @brief This function get 1st update candidate number
 * @param [in] VOID
 * @retval u32Val
*/
unsigned int HAL_KME_ME2_Get1stUpdateCandiNums(VOID)
{
	unsigned int u32Val = 0;
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg,18,23, &u32Val);
	return u32Val;
}

/**
 * @brief This function set block logo rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_LOGO_Set_BlockRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(KME_LOGO1_KME_LOGO1_04_reg,16,23, u32Top);	
	WriteRegister(KME_LOGO1_KME_LOGO1_04_reg,24,31, u32Bottom);	
	WriteRegister(KME_LOGO1_KME_LOGO1_04_reg,0,7, u32Left);	
	WriteRegister(KME_LOGO1_KME_LOGO1_04_reg,8,15, u32Right);	
}

/**
 * @brief This function set pixel pixel rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KME_LOGO_Set_PixelRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(KME_LOGO1_KME_LOGO1_DC_reg,0,9, u32Top);	
	WriteRegister(KME_LOGO1_KME_LOGO1_DC_reg,10,19, u32Bottom);	
	WriteRegister(KME_LOGO1_KME_LOGO1_D8_reg,0,9, u32Left);	
	WriteRegister(KME_LOGO1_KME_LOGO1_D8_reg,10,19, u32Right);	
}

/**
 * @brief This function set ME2 double scan enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_ME2DoubleScanEnable(BOOL bEnable)
{
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg,13,13, (unsigned int)bEnable);	
}

/**
 * @brief This function set me vflip
 * @param [in] BOOL bVflip
 * @retval VOID
*/
VOID HAL_KME_SetVflip(BOOL bVflip)
{
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_TOP__kme_system_vflip_enable_ADDR no mat,FRC_TOP__KME_TOP__kme_system_vflip_enable_BITSTART no mat,FRC_TOP__KME_TOP__kme_system_vflip_enable_BITEND no mat,(unsigned int)bVflip);
}

/**
 * @brief This function set me vbuf_en
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_KME_Set_Vbuf_Enable(BOOL bEnable)
{
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 0, 0, (unsigned int)bEnable);
}

VOID HAL_ME2_cand_wrtDriver( _str_ME2_Candidate_table *CandiTbl)
{

#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP || IC_K24)//#if RTK_MEMC_Performance_tunging_from_tv001
	unsigned int u8_candiNum_me2_1st = 0, u8_candiNum_me2_2nd = 0;
	kme_me2_vbuf_top_kme_me2_vbuf_top_2c_RBUS kme_me2_vbuf_top_2C_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_30_RBUS kme_me2_vbuf_top_30_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_4c_RBUS kme_me2_vbuf_top_4C_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_34_RBUS kme_me2_vbuf_top_34_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_50_RBUS kme_me2_vbuf_top_50_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_54_RBUS kme_me2_vbuf_top_54_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_58_RBUS kme_me2_vbuf_top_58_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_5c_RBUS kme_me2_vbuf_top_5C_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_60_RBUS kme_me2_vbuf_top_60_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_64_RBUS kme_me2_vbuf_top_64_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_68_RBUS kme_me2_vbuf_top_68_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_6c_RBUS kme_me2_vbuf_top_6C_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_70_RBUS kme_me2_vbuf_top_70_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_74_RBUS kme_me2_vbuf_top_74_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_78_RBUS kme_me2_vbuf_top_78_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_7c_RBUS kme_me2_vbuf_top_7C_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_80_RBUS kme_me2_vbuf_top_80_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_84_RBUS kme_me2_vbuf_top_84_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_88_RBUS kme_me2_vbuf_top_88_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_8c_RBUS kme_me2_vbuf_top_8C_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_90_RBUS kme_me2_vbuf_top_90_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_94_RBUS kme_me2_vbuf_top_94_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_98_RBUS kme_me2_vbuf_top_98_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_9c_RBUS kme_me2_vbuf_top_9C_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_a0_RBUS kme_me2_vbuf_top_A0_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_a4_RBUS kme_me2_vbuf_top_A4_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_a8_RBUS kme_me2_vbuf_top_A8_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_ac_RBUS kme_me2_vbuf_top_AC_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_b0_RBUS kme_me2_vbuf_top_B0_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_b4_RBUS kme_me2_vbuf_top_B4_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_b8_RBUS kme_me2_vbuf_top_B8_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_bc_RBUS kme_me2_vbuf_top_BC_RBUS;
#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)	
	kme_me2_vbuf_top_kme_me2_vbuf_top_cc_RBUS kme_me2_vbuf_top_CC_RBUS;
	kme_me2_vbuf_top_kme_me2_vbuf_top_d0_RBUS kme_me2_vbuf_top_D0_RBUS;
#endif

	kme_me2_vbuf_top_2C_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg);
	kme_me2_vbuf_top_30_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg);
	kme_me2_vbuf_top_4C_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg);
	kme_me2_vbuf_top_34_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg);
	kme_me2_vbuf_top_50_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg);
	kme_me2_vbuf_top_54_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg);
	kme_me2_vbuf_top_58_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg);
	kme_me2_vbuf_top_5C_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg);
	kme_me2_vbuf_top_60_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg);
	kme_me2_vbuf_top_64_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_64_reg);
	kme_me2_vbuf_top_68_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_68_reg);
	kme_me2_vbuf_top_6C_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_6C_reg);
	kme_me2_vbuf_top_70_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_70_reg);
	kme_me2_vbuf_top_74_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_74_reg);
	kme_me2_vbuf_top_78_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_78_reg);
	kme_me2_vbuf_top_7C_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_7C_reg);
	kme_me2_vbuf_top_80_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_80_reg);
	kme_me2_vbuf_top_84_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg);
	kme_me2_vbuf_top_88_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg);
	kme_me2_vbuf_top_8C_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg);
	kme_me2_vbuf_top_90_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg);
	kme_me2_vbuf_top_94_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg);
	kme_me2_vbuf_top_98_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg);
	kme_me2_vbuf_top_9C_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_9C_reg);
	kme_me2_vbuf_top_A0_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A0_reg);
	kme_me2_vbuf_top_A4_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A4_reg);
	kme_me2_vbuf_top_A8_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A8_reg);
	kme_me2_vbuf_top_AC_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_AC_reg);
	kme_me2_vbuf_top_B0_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B0_reg);
	kme_me2_vbuf_top_B4_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B4_reg);
	kme_me2_vbuf_top_B8_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B8_reg);
	kme_me2_vbuf_top_BC_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_BC_reg);
#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)	
	kme_me2_vbuf_top_CC_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_CC_reg);
	kme_me2_vbuf_top_D0_RBUS.regValue = rtd_inl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D0_reg);
#endif

	u8_candiNum_me2_1st = 	(CandiTbl->_cand_num._1st_zmv_en) + (CandiTbl->_cand_num._1st_gmv_en) +
							(CandiTbl->_cand_num._1st_ph_cand_num * CandiTbl->_cand_num._1st_ph_offset_cand_num) +
							(CandiTbl->_cand_num._1st_update_cand_num) +
							(CandiTbl->_cand_num._1st_pfv0_cand_num * CandiTbl->_cand_num._1st_pfv0_offset_cand_num) +
							(CandiTbl->_cand_num._1st_pfv1_cand_num * CandiTbl->_cand_num._1st_pfv1_offset_cand_num);

	u8_candiNum_me2_2nd = 	(CandiTbl->_cand_num._2nd_zmv_en) + (CandiTbl->_cand_num._2nd_gmv_en) +
							(CandiTbl->_cand_num._2nd_ph_cand_num * CandiTbl->_cand_num._2nd_ph_offset_cand_num) +
							(CandiTbl->_cand_num._2nd_update_cand_num) +
							(CandiTbl->_cand_num._2nd_pfv0_cand_num * CandiTbl->_cand_num._2nd_pfv0_offset_cand_num) +
							(CandiTbl->_cand_num._2nd_pfv1_cand_num * CandiTbl->_cand_num._2nd_pfv1_offset_cand_num);

	//rtd_pr_memc_notice("[%s] me2 : %d %d\n", __func__ , u8_candiNum_me2_1st, u8_candiNum_me2_2nd);

//======== for _str_cand_num	========
	kme_me2_vbuf_top_2C_RBUS.kme_me2_vbuf_1st_ph_candi_nums = CandiTbl->_cand_num._1st_ph_cand_num;
	//kme_me2_vbuf_top_2C_RBUS.kme_me2_vbuf_1st_ph_candi_offset_nums = CandiTbl->_cand_num._1st_ph_offset_cand_num;
	kme_me2_vbuf_top_30_RBUS.kme_me2_vbuf_1st_updt_candi_nums = CandiTbl->_cand_num._1st_update_cand_num;
	kme_me2_vbuf_top_30_RBUS.kme_me2_vbuf_1st_pfv0_candi_nums = CandiTbl->_cand_num._1st_pfv0_cand_num;
	//kme_me2_vbuf_top_30_RBUS.kme_me2_vbuf_1st_pfv0_candi_offset_nums = CandiTbl->_cand_num._1st_pfv0_offset_cand_num;
	kme_me2_vbuf_top_30_RBUS.kme_me2_vbuf_1st_pfv1_candi_nums = CandiTbl->_cand_num._1st_pfv1_cand_num;
	//kme_me2_vbuf_top_30_RBUS.kme_me2_vbuf_1st_pfv1_candi_offset_nums = CandiTbl->_cand_num._1st_pfv1_offset_cand_num;
	kme_me2_vbuf_top_4C_RBUS.kme_me2_vbuf_1st_ph_candi_dediff_en = CandiTbl->_cand_num._1st_ph_cand_dediff_en;
	kme_me2_vbuf_top_4C_RBUS.kme_me2_vbuf_1st_pfv0_candi_dediff_en = CandiTbl->_cand_num._1st_pfv0_cand_dediff_en;
	kme_me2_vbuf_top_4C_RBUS.kme_me2_vbuf_1st_pfv1_candi_dediff_en = CandiTbl->_cand_num._1st_pfv1_cand_dediff_en;
	kme_me2_vbuf_top_30_RBUS.kme_me2_vbuf_1st_zmv_en = CandiTbl->_cand_num._1st_zmv_en;
	kme_me2_vbuf_top_30_RBUS.kme_me2_vbuf_1st_gmv_en = CandiTbl->_cand_num._1st_gmv_en;

	kme_me2_vbuf_top_30_RBUS.kme_me2_vbuf_2nd_ph_candi_nums = CandiTbl->_cand_num._2nd_ph_cand_num;
	//kme_me2_vbuf_top_34_RBUS.kme_me2_vbuf_2nd_ph_candi_offset_nums = CandiTbl->_cand_num._2nd_ph_offset_cand_num;
	kme_me2_vbuf_top_34_RBUS.kme_me2_vbuf_2nd_updt_candi_nums = CandiTbl->_cand_num._2nd_update_cand_num;
	kme_me2_vbuf_top_34_RBUS.kme_me2_vbuf_2nd_pfv0_candi_nums = CandiTbl->_cand_num._2nd_pfv0_cand_num;
	//kme_me2_vbuf_top_34_RBUS.kme_me2_vbuf_2nd_pfv0_candi_offset_nums = CandiTbl->_cand_num._2nd_pfv0_offset_cand_num;
	kme_me2_vbuf_top_34_RBUS.kme_me2_vbuf_2nd_pfv1_candi_nums = CandiTbl->_cand_num._2nd_pfv1_cand_num;
	//kme_me2_vbuf_top_34_RBUS.kme_me2_vbuf_2nd_pfv1_candi_offset_nums = CandiTbl->_cand_num._2nd_pfv1_offset_cand_num;
	kme_me2_vbuf_top_84_RBUS.kme_me2_vbuf_2nd_ph_candi_dediff_en = CandiTbl->_cand_num._2nd_ph_cand_dediff_en;
	kme_me2_vbuf_top_84_RBUS.kme_me2_vbuf_2nd_pfv0_candi_dediff_en = CandiTbl->_cand_num._2nd_pfv0_cand_dediff_en;
	kme_me2_vbuf_top_84_RBUS.kme_me2_vbuf_2nd_pfv1_candi_dediff_en = CandiTbl->_cand_num._2nd_pfv1_cand_dediff_en;
	kme_me2_vbuf_top_34_RBUS.kme_me2_vbuf_2nd_zmv_en = CandiTbl->_cand_num._2nd_zmv_en;
	kme_me2_vbuf_top_34_RBUS.kme_me2_vbuf_2nd_gmv_en = CandiTbl->_cand_num._2nd_gmv_en;

//======= for _str_cand_location ========
	kme_me2_vbuf_top_4C_RBUS.kme_me2_vbuf_1st_ph_candi_0_x_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand0.offset_x;
	kme_me2_vbuf_top_4C_RBUS.kme_me2_vbuf_1st_ph_candi_0_y_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand0.offset_y;
	kme_me2_vbuf_top_4C_RBUS.kme_me2_vbuf_1st_ph_candi_1_x_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand1.offset_x;
	kme_me2_vbuf_top_4C_RBUS.kme_me2_vbuf_1st_ph_candi_1_y_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand1.offset_y;
	kme_me2_vbuf_top_4C_RBUS.kme_me2_vbuf_1st_ph_candi_2_x_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand2.offset_x;
	kme_me2_vbuf_top_4C_RBUS.kme_me2_vbuf_1st_ph_candi_2_y_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand2.offset_y;
	kme_me2_vbuf_top_4C_RBUS.kme_me2_vbuf_1st_ph_candi_3_x_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand3.offset_x;
	kme_me2_vbuf_top_4C_RBUS.kme_me2_vbuf_1st_ph_candi_3_y_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand3.offset_y;
	kme_me2_vbuf_top_50_RBUS.kme_me2_vbuf_1st_ph_candi_4_x_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand4.offset_x;
	kme_me2_vbuf_top_50_RBUS.kme_me2_vbuf_1st_ph_candi_4_y_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand4.offset_y;
	kme_me2_vbuf_top_50_RBUS.kme_me2_vbuf_1st_ph_candi_5_x_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand5.offset_x;
	kme_me2_vbuf_top_50_RBUS.kme_me2_vbuf_1st_ph_candi_5_y_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand5.offset_y;
	kme_me2_vbuf_top_50_RBUS.kme_me2_vbuf_1st_ph_candi_6_x_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand6.offset_x;
	kme_me2_vbuf_top_50_RBUS.kme_me2_vbuf_1st_ph_candi_6_y_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand6.offset_y;
	kme_me2_vbuf_top_50_RBUS.kme_me2_vbuf_1st_ph_candi_7_x_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand7.offset_x;
	kme_me2_vbuf_top_50_RBUS.kme_me2_vbuf_1st_ph_candi_7_y_offset = CandiTbl->_cand_location._1st_ph_cand._ph_cand7.offset_y;

	kme_me2_vbuf_top_60_RBUS.kme_me2_vbuf_1st_updt_candi_0_x_offset = CandiTbl->_cand_location._1st_update_cand._update_cand0.offset_x;
	kme_me2_vbuf_top_60_RBUS.kme_me2_vbuf_1st_updt_candi_0_y_offset = CandiTbl->_cand_location._1st_update_cand._update_cand0.offset_y;
	kme_me2_vbuf_top_60_RBUS.kme_me2_vbuf_1st_updt_candi_1_x_offset = CandiTbl->_cand_location._1st_update_cand._update_cand1.offset_x;
	kme_me2_vbuf_top_60_RBUS.kme_me2_vbuf_1st_updt_candi_1_y_offset = CandiTbl->_cand_location._1st_update_cand._update_cand1.offset_y;
	kme_me2_vbuf_top_60_RBUS.kme_me2_vbuf_1st_updt_candi_2_x_offset = CandiTbl->_cand_location._1st_update_cand._update_cand2.offset_x;
	kme_me2_vbuf_top_60_RBUS.kme_me2_vbuf_1st_updt_candi_2_y_offset = CandiTbl->_cand_location._1st_update_cand._update_cand2.offset_y;
	kme_me2_vbuf_top_60_RBUS.kme_me2_vbuf_1st_updt_candi_3_x_offset = CandiTbl->_cand_location._1st_update_cand._update_cand3.offset_x;
	kme_me2_vbuf_top_60_RBUS.kme_me2_vbuf_1st_updt_candi_3_y_offset = CandiTbl->_cand_location._1st_update_cand._update_cand3.offset_y;

	kme_me2_vbuf_top_50_RBUS.kme_me2_vbuf_1st_pfv0_candi_0_x_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand0.offset_x;
	kme_me2_vbuf_top_50_RBUS.kme_me2_vbuf_1st_pfv0_candi_0_y_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand0.offset_y;
	kme_me2_vbuf_top_54_RBUS.kme_me2_vbuf_1st_pfv0_candi_1_x_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand1.offset_x;
	kme_me2_vbuf_top_54_RBUS.kme_me2_vbuf_1st_pfv0_candi_1_y_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand1.offset_y;
	kme_me2_vbuf_top_54_RBUS.kme_me2_vbuf_1st_pfv0_candi_2_x_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand2.offset_x;
	kme_me2_vbuf_top_54_RBUS.kme_me2_vbuf_1st_pfv0_candi_2_y_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand2.offset_y;
	kme_me2_vbuf_top_54_RBUS.kme_me2_vbuf_1st_pfv0_candi_3_x_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand3.offset_x;
	kme_me2_vbuf_top_54_RBUS.kme_me2_vbuf_1st_pfv0_candi_3_y_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand3.offset_y;
	kme_me2_vbuf_top_54_RBUS.kme_me2_vbuf_1st_pfv0_candi_4_x_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand4.offset_x;
	kme_me2_vbuf_top_54_RBUS.kme_me2_vbuf_1st_pfv0_candi_4_y_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand4.offset_y;
	kme_me2_vbuf_top_54_RBUS.kme_me2_vbuf_1st_pfv0_candi_5_x_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand5.offset_x;
	kme_me2_vbuf_top_54_RBUS.kme_me2_vbuf_1st_pfv0_candi_5_y_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand5.offset_y;
	kme_me2_vbuf_top_58_RBUS.kme_me2_vbuf_1st_pfv0_candi_6_x_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand6.offset_x;
	kme_me2_vbuf_top_58_RBUS.kme_me2_vbuf_1st_pfv0_candi_6_y_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand6.offset_y;
	kme_me2_vbuf_top_58_RBUS.kme_me2_vbuf_1st_pfv0_candi_7_x_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand7.offset_x;
	kme_me2_vbuf_top_58_RBUS.kme_me2_vbuf_1st_pfv0_candi_7_y_offset = CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand7.offset_y;

	kme_me2_vbuf_top_58_RBUS.kme_me2_vbuf_1st_pfv1_candi_0_x_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand0.offset_x;
	kme_me2_vbuf_top_58_RBUS.kme_me2_vbuf_1st_pfv1_candi_0_y_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand0.offset_y;
	kme_me2_vbuf_top_58_RBUS.kme_me2_vbuf_1st_pfv1_candi_1_x_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand1.offset_x;
	kme_me2_vbuf_top_58_RBUS.kme_me2_vbuf_1st_pfv1_candi_1_y_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand1.offset_y;
	kme_me2_vbuf_top_58_RBUS.kme_me2_vbuf_1st_pfv1_candi_2_x_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand2.offset_x;
	kme_me2_vbuf_top_58_RBUS.kme_me2_vbuf_1st_pfv1_candi_2_y_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand2.offset_y;
	kme_me2_vbuf_top_5C_RBUS.kme_me2_vbuf_1st_pfv1_candi_3_x_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand3.offset_x;
	kme_me2_vbuf_top_5C_RBUS.kme_me2_vbuf_1st_pfv1_candi_3_y_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand3.offset_y;
	kme_me2_vbuf_top_5C_RBUS.kme_me2_vbuf_1st_pfv1_candi_4_x_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand4.offset_x;
	kme_me2_vbuf_top_5C_RBUS.kme_me2_vbuf_1st_pfv1_candi_4_y_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand4.offset_y;
	kme_me2_vbuf_top_5C_RBUS.kme_me2_vbuf_1st_pfv1_candi_5_x_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand5.offset_x;
	kme_me2_vbuf_top_5C_RBUS.kme_me2_vbuf_1st_pfv1_candi_5_y_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand5.offset_y;
	kme_me2_vbuf_top_5C_RBUS.kme_me2_vbuf_1st_pfv1_candi_6_x_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand6.offset_x;
	kme_me2_vbuf_top_5C_RBUS.kme_me2_vbuf_1st_pfv1_candi_6_y_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand6.offset_y;
	kme_me2_vbuf_top_5C_RBUS.kme_me2_vbuf_1st_pfv1_candi_7_x_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand7.offset_x;
	kme_me2_vbuf_top_5C_RBUS.kme_me2_vbuf_1st_pfv1_candi_7_y_offset = CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand7.offset_y;

	kme_me2_vbuf_top_84_RBUS.kme_me2_vbuf_2nd_ph_candi_0_x_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand0.offset_x;
	kme_me2_vbuf_top_84_RBUS.kme_me2_vbuf_2nd_ph_candi_0_y_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand0.offset_y;
	kme_me2_vbuf_top_84_RBUS.kme_me2_vbuf_2nd_ph_candi_1_x_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand1.offset_x;
	kme_me2_vbuf_top_84_RBUS.kme_me2_vbuf_2nd_ph_candi_1_y_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand1.offset_y;
	kme_me2_vbuf_top_84_RBUS.kme_me2_vbuf_2nd_ph_candi_2_x_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand2.offset_x;
	kme_me2_vbuf_top_84_RBUS.kme_me2_vbuf_2nd_ph_candi_2_y_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand2.offset_y;
	kme_me2_vbuf_top_84_RBUS.kme_me2_vbuf_2nd_ph_candi_3_x_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand3.offset_x;
	kme_me2_vbuf_top_84_RBUS.kme_me2_vbuf_2nd_ph_candi_3_y_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand3.offset_y;
	kme_me2_vbuf_top_88_RBUS.kme_me2_vbuf_2nd_ph_candi_4_x_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand4.offset_x;
	kme_me2_vbuf_top_88_RBUS.kme_me2_vbuf_2nd_ph_candi_4_y_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand4.offset_y;
	kme_me2_vbuf_top_88_RBUS.kme_me2_vbuf_2nd_ph_candi_5_x_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand5.offset_x;
	kme_me2_vbuf_top_88_RBUS.kme_me2_vbuf_2nd_ph_candi_5_y_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand5.offset_y;
	kme_me2_vbuf_top_88_RBUS.kme_me2_vbuf_2nd_ph_candi_6_x_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand6.offset_x;
	kme_me2_vbuf_top_88_RBUS.kme_me2_vbuf_2nd_ph_candi_6_y_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand6.offset_y;
	kme_me2_vbuf_top_88_RBUS.kme_me2_vbuf_2nd_ph_candi_7_x_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand7.offset_x;
	kme_me2_vbuf_top_88_RBUS.kme_me2_vbuf_2nd_ph_candi_7_y_offset = CandiTbl->_cand_location._2nd_ph_cand._ph_cand7.offset_y;

	kme_me2_vbuf_top_98_RBUS.kme_me2_vbuf_2nd_updt_candi_0_x_offset = CandiTbl->_cand_location._2nd_update_cand._update_cand0.offset_x;
	kme_me2_vbuf_top_98_RBUS.kme_me2_vbuf_2nd_updt_candi_0_y_offset = CandiTbl->_cand_location._2nd_update_cand._update_cand0.offset_y;
	kme_me2_vbuf_top_98_RBUS.kme_me2_vbuf_2nd_updt_candi_1_x_offset = CandiTbl->_cand_location._2nd_update_cand._update_cand1.offset_x;
	kme_me2_vbuf_top_98_RBUS.kme_me2_vbuf_2nd_updt_candi_1_y_offset = CandiTbl->_cand_location._2nd_update_cand._update_cand1.offset_y;
	kme_me2_vbuf_top_98_RBUS.kme_me2_vbuf_2nd_updt_candi_2_x_offset = CandiTbl->_cand_location._2nd_update_cand._update_cand2.offset_x;
	kme_me2_vbuf_top_98_RBUS.kme_me2_vbuf_2nd_updt_candi_2_y_offset = CandiTbl->_cand_location._2nd_update_cand._update_cand2.offset_y;
	kme_me2_vbuf_top_98_RBUS.kme_me2_vbuf_2nd_updt_candi_3_x_offset = CandiTbl->_cand_location._2nd_update_cand._update_cand3.offset_x;
	kme_me2_vbuf_top_98_RBUS.kme_me2_vbuf_2nd_updt_candi_3_y_offset = CandiTbl->_cand_location._2nd_update_cand._update_cand3.offset_y;

	kme_me2_vbuf_top_88_RBUS.kme_me2_vbuf_2nd_pfv0_candi_0_x_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand0.offset_x;
	kme_me2_vbuf_top_88_RBUS.kme_me2_vbuf_2nd_pfv0_candi_0_y_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand0.offset_y;
	kme_me2_vbuf_top_8C_RBUS.kme_me2_vbuf_2nd_pfv0_candi_1_x_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand1.offset_x;
	kme_me2_vbuf_top_8C_RBUS.kme_me2_vbuf_2nd_pfv0_candi_1_y_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand1.offset_y;
	kme_me2_vbuf_top_8C_RBUS.kme_me2_vbuf_2nd_pfv0_candi_2_x_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand2.offset_x;
	kme_me2_vbuf_top_8C_RBUS.kme_me2_vbuf_2nd_pfv0_candi_2_y_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand2.offset_y;
	kme_me2_vbuf_top_8C_RBUS.kme_me2_vbuf_2nd_pfv0_candi_3_x_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand3.offset_x;
	kme_me2_vbuf_top_8C_RBUS.kme_me2_vbuf_2nd_pfv0_candi_3_y_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand3.offset_y;
	kme_me2_vbuf_top_8C_RBUS.kme_me2_vbuf_2nd_pfv0_candi_4_x_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand4.offset_x;
	kme_me2_vbuf_top_8C_RBUS.kme_me2_vbuf_2nd_pfv0_candi_4_y_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand4.offset_y;
	kme_me2_vbuf_top_8C_RBUS.kme_me2_vbuf_2nd_pfv0_candi_5_x_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand5.offset_x;
	kme_me2_vbuf_top_8C_RBUS.kme_me2_vbuf_2nd_pfv0_candi_5_y_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand5.offset_y;
	kme_me2_vbuf_top_90_RBUS.kme_me2_vbuf_2nd_pfv0_candi_6_x_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand6.offset_x;
	kme_me2_vbuf_top_90_RBUS.kme_me2_vbuf_2nd_pfv0_candi_6_y_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand6.offset_y;
	kme_me2_vbuf_top_90_RBUS.kme_me2_vbuf_2nd_pfv0_candi_7_x_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand7.offset_x;
	kme_me2_vbuf_top_90_RBUS.kme_me2_vbuf_2nd_pfv0_candi_7_y_offset = CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand7.offset_y;

	kme_me2_vbuf_top_90_RBUS.kme_me2_vbuf_2nd_pfv1_candi_0_x_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand0.offset_x;
	kme_me2_vbuf_top_90_RBUS.kme_me2_vbuf_2nd_pfv1_candi_0_y_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand0.offset_y;
	kme_me2_vbuf_top_90_RBUS.kme_me2_vbuf_2nd_pfv1_candi_1_x_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand1.offset_x;
	kme_me2_vbuf_top_90_RBUS.kme_me2_vbuf_2nd_pfv1_candi_1_y_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand1.offset_y;
	kme_me2_vbuf_top_90_RBUS.kme_me2_vbuf_2nd_pfv1_candi_2_x_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand2.offset_x;
	kme_me2_vbuf_top_90_RBUS.kme_me2_vbuf_2nd_pfv1_candi_2_y_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand2.offset_y;
	kme_me2_vbuf_top_94_RBUS.kme_me2_vbuf_2nd_pfv1_candi_3_x_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand3.offset_x;
	kme_me2_vbuf_top_94_RBUS.kme_me2_vbuf_2nd_pfv1_candi_3_y_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand3.offset_y;
	kme_me2_vbuf_top_94_RBUS.kme_me2_vbuf_2nd_pfv1_candi_4_x_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand4.offset_x;
	kme_me2_vbuf_top_94_RBUS.kme_me2_vbuf_2nd_pfv1_candi_4_y_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand4.offset_y;
	kme_me2_vbuf_top_94_RBUS.kme_me2_vbuf_2nd_pfv1_candi_5_x_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand5.offset_x;
	kme_me2_vbuf_top_94_RBUS.kme_me2_vbuf_2nd_pfv1_candi_5_y_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand5.offset_y;
	kme_me2_vbuf_top_94_RBUS.kme_me2_vbuf_2nd_pfv1_candi_6_x_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand6.offset_x;
	kme_me2_vbuf_top_94_RBUS.kme_me2_vbuf_2nd_pfv1_candi_6_y_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand6.offset_y;
	kme_me2_vbuf_top_94_RBUS.kme_me2_vbuf_2nd_pfv1_candi_7_x_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand7.offset_x;
	kme_me2_vbuf_top_94_RBUS.kme_me2_vbuf_2nd_pfv1_candi_7_y_offset = CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand7.offset_y;

//======== _str_cand_pnt ==========
	kme_me2_vbuf_top_64_RBUS.kme_me2_vbuf_1st_ph_candi_st_pnt0 = CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt0;
	kme_me2_vbuf_top_64_RBUS.kme_me2_vbuf_1st_ph_candi_st_pnt1 = CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt1;
	kme_me2_vbuf_top_64_RBUS.kme_me2_vbuf_1st_ph_candi_st_pnt2 = CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt2;
	kme_me2_vbuf_top_68_RBUS.kme_me2_vbuf_1st_ph_candi_st_pnt3 = CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt3;
	kme_me2_vbuf_top_68_RBUS.kme_me2_vbuf_1st_ph_candi_st_pnt4 = CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt4;
	kme_me2_vbuf_top_68_RBUS.kme_me2_vbuf_1st_ph_candi_st_pnt5 = CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt5;
	kme_me2_vbuf_top_68_RBUS.kme_me2_vbuf_1st_ph_candi_st_pnt6 = CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt6;
	kme_me2_vbuf_top_6C_RBUS.kme_me2_vbuf_1st_ph_candi_st_pnt7 = CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt7;

	kme_me2_vbuf_top_80_RBUS.kme_me2_vbuf_1st_candi_updt_pnt0 = CandiTbl->_cand_pnt._1st_update_pnt.update_pnt0;
	kme_me2_vbuf_top_80_RBUS.kme_me2_vbuf_1st_candi_updt_pnt1 = CandiTbl->_cand_pnt._1st_update_pnt.update_pnt1;
	kme_me2_vbuf_top_80_RBUS.kme_me2_vbuf_1st_candi_updt_pnt2 = CandiTbl->_cand_pnt._1st_update_pnt.update_pnt2;
	kme_me2_vbuf_top_80_RBUS.kme_me2_vbuf_1st_candi_updt_pnt3 = CandiTbl->_cand_pnt._1st_update_pnt.update_pnt3;

	kme_me2_vbuf_top_6C_RBUS.kme_me2_vbuf_1st_pfv0_candi_st_pnt0 = CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt0;
	kme_me2_vbuf_top_6C_RBUS.kme_me2_vbuf_1st_pfv0_candi_st_pnt1 = CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt1;
	kme_me2_vbuf_top_70_RBUS.kme_me2_vbuf_1st_pfv0_candi_st_pnt2 = CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt2;
	kme_me2_vbuf_top_70_RBUS.kme_me2_vbuf_1st_pfv0_candi_st_pnt3 = CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt3;
	kme_me2_vbuf_top_70_RBUS.kme_me2_vbuf_1st_pfv0_candi_st_pnt4 = CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt4;
	kme_me2_vbuf_top_70_RBUS.kme_me2_vbuf_1st_pfv0_candi_st_pnt5 = CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt5;
	kme_me2_vbuf_top_74_RBUS.kme_me2_vbuf_1st_pfv0_candi_st_pnt6 = CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt6;
	kme_me2_vbuf_top_74_RBUS.kme_me2_vbuf_1st_pfv0_candi_st_pnt7 = CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt7;

	kme_me2_vbuf_top_74_RBUS.kme_me2_vbuf_1st_pfv1_candi_st_pnt0 = CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt0;
	kme_me2_vbuf_top_78_RBUS.kme_me2_vbuf_1st_pfv1_candi_st_pnt1 = CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt1;
	kme_me2_vbuf_top_78_RBUS.kme_me2_vbuf_1st_pfv1_candi_st_pnt2 = CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt2;
	kme_me2_vbuf_top_78_RBUS.kme_me2_vbuf_1st_pfv1_candi_st_pnt3 = CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt3;
	kme_me2_vbuf_top_78_RBUS.kme_me2_vbuf_1st_pfv1_candi_st_pnt4 = CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt4;
	kme_me2_vbuf_top_7C_RBUS.kme_me2_vbuf_1st_pfv1_candi_st_pnt5 = CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt5;
	kme_me2_vbuf_top_7C_RBUS.kme_me2_vbuf_1st_pfv1_candi_st_pnt6 = CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt6;
	kme_me2_vbuf_top_7C_RBUS.kme_me2_vbuf_1st_pfv1_candi_st_pnt7 = CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt7;

	kme_me2_vbuf_top_6C_RBUS.kme_me2_vbuf_1st_ph_candi_dediff_pnt = CandiTbl->_cand_pnt._1st_others_pnt._1st_ph_cand_dediff_pnt;
	kme_me2_vbuf_top_74_RBUS.kme_me2_vbuf_1st_pfv0_candi_dediff_pnt = CandiTbl->_cand_pnt._1st_others_pnt._1st_pfv0_cand_dediff_pnt;
	kme_me2_vbuf_top_7C_RBUS.kme_me2_vbuf_1st_pfv1_candi_dediff_pnt = CandiTbl->_cand_pnt._1st_others_pnt._1st_pfv1_cand_dediff_pnt;
	kme_me2_vbuf_top_60_RBUS.kme_me2_vbuf_1st_candi_zmv_pnt = CandiTbl->_cand_pnt._1st_others_pnt._1st_zmv_pnt;
	kme_me2_vbuf_top_64_RBUS.kme_me2_vbuf_1st_candi_gmv_pnt = CandiTbl->_cand_pnt._1st_others_pnt._1st_gmv_pnt;

#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	kme_me2_vbuf_top_D0_RBUS.kme_me2_vbuf_phase_gain0 = CandiTbl->_ph_index_setting._ph_index0.gain;
	kme_me2_vbuf_top_D0_RBUS.kme_me2_vbuf_phase_gain1 = CandiTbl->_ph_index_setting._ph_index1.gain;
	kme_me2_vbuf_top_D0_RBUS.kme_me2_vbuf_phase_gain2 = CandiTbl->_ph_index_setting._ph_index2.gain;
	kme_me2_vbuf_top_D0_RBUS.kme_me2_vbuf_phase_gain3 = CandiTbl->_ph_index_setting._ph_index3.gain;
	kme_me2_vbuf_top_CC_RBUS.kme_me2_vbuf_phase_offset0 = CandiTbl->_ph_index_setting._ph_index0.offset;
	kme_me2_vbuf_top_CC_RBUS.kme_me2_vbuf_phase_offset1 = CandiTbl->_ph_index_setting._ph_index1.offset;
	kme_me2_vbuf_top_CC_RBUS.kme_me2_vbuf_phase_offset2 = CandiTbl->_ph_index_setting._ph_index2.offset;
	kme_me2_vbuf_top_CC_RBUS.kme_me2_vbuf_phase_offset3 = CandiTbl->_ph_index_setting._ph_index3.offset;
#endif

	kme_me2_vbuf_top_9C_RBUS.kme_me2_vbuf_2nd_ph_candi_st_pnt0 = CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt0;
	kme_me2_vbuf_top_9C_RBUS.kme_me2_vbuf_2nd_ph_candi_st_pnt1 = CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt1;
	kme_me2_vbuf_top_A0_RBUS.kme_me2_vbuf_2nd_ph_candi_st_pnt2 = CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt2;
	kme_me2_vbuf_top_A0_RBUS.kme_me2_vbuf_2nd_ph_candi_st_pnt3 = CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt3;
	kme_me2_vbuf_top_A0_RBUS.kme_me2_vbuf_2nd_ph_candi_st_pnt4 = CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt4;
	kme_me2_vbuf_top_A0_RBUS.kme_me2_vbuf_2nd_ph_candi_st_pnt5 = CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt5;
	kme_me2_vbuf_top_A4_RBUS.kme_me2_vbuf_2nd_ph_candi_st_pnt6 = CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt6;
	kme_me2_vbuf_top_A4_RBUS.kme_me2_vbuf_2nd_ph_candi_st_pnt7 = CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt7;

	kme_me2_vbuf_top_B8_RBUS.kme_me2_vbuf_2nd_candi_updt_pnt0 = CandiTbl->_cand_pnt._2nd_update_pnt.update_pnt0;
	kme_me2_vbuf_top_B8_RBUS.kme_me2_vbuf_2nd_candi_updt_pnt1 = CandiTbl->_cand_pnt._2nd_update_pnt.update_pnt1;
	kme_me2_vbuf_top_B8_RBUS.kme_me2_vbuf_2nd_candi_updt_pnt2 = CandiTbl->_cand_pnt._2nd_update_pnt.update_pnt2;
	kme_me2_vbuf_top_BC_RBUS.kme_me2_vbuf_2nd_candi_updt_pnt3 = CandiTbl->_cand_pnt._2nd_update_pnt.update_pnt3;

	kme_me2_vbuf_top_A4_RBUS.kme_me2_vbuf_2nd_pfv0_candi_st_pnt0 = CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt0;
	kme_me2_vbuf_top_A8_RBUS.kme_me2_vbuf_2nd_pfv0_candi_st_pnt1 = CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt1;
	kme_me2_vbuf_top_A8_RBUS.kme_me2_vbuf_2nd_pfv0_candi_st_pnt2 = CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt2;
	kme_me2_vbuf_top_A8_RBUS.kme_me2_vbuf_2nd_pfv0_candi_st_pnt3 = CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt3;
	kme_me2_vbuf_top_A8_RBUS.kme_me2_vbuf_2nd_pfv0_candi_st_pnt4 = CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt4;
	kme_me2_vbuf_top_AC_RBUS.kme_me2_vbuf_2nd_pfv0_candi_st_pnt5 = CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt5;
	kme_me2_vbuf_top_AC_RBUS.kme_me2_vbuf_2nd_pfv0_candi_st_pnt6 = CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt6;
	kme_me2_vbuf_top_AC_RBUS.kme_me2_vbuf_2nd_pfv0_candi_st_pnt7 = CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt7;

	kme_me2_vbuf_top_B0_RBUS.kme_me2_vbuf_2nd_pfv1_candi_st_pnt0 = CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt0;
	kme_me2_vbuf_top_B0_RBUS.kme_me2_vbuf_2nd_pfv1_candi_st_pnt1 = CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt1;
	kme_me2_vbuf_top_B0_RBUS.kme_me2_vbuf_2nd_pfv1_candi_st_pnt2 = CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt2;
	kme_me2_vbuf_top_B0_RBUS.kme_me2_vbuf_2nd_pfv1_candi_st_pnt3 = CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt3;
	kme_me2_vbuf_top_B4_RBUS.kme_me2_vbuf_2nd_pfv1_candi_st_pnt4 = CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt4;
	kme_me2_vbuf_top_B4_RBUS.kme_me2_vbuf_2nd_pfv1_candi_st_pnt5 = CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt5;
	kme_me2_vbuf_top_B4_RBUS.kme_me2_vbuf_2nd_pfv1_candi_st_pnt6 = CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt6;
	kme_me2_vbuf_top_B4_RBUS.kme_me2_vbuf_2nd_pfv1_candi_st_pnt7 = CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt7;

	kme_me2_vbuf_top_A4_RBUS.kme_me2_vbuf_2nd_ph_candi_dediff_pnt = CandiTbl->_cand_pnt._2nd_others_pnt._2nd_ph_cand_dediff_pnt;
	kme_me2_vbuf_top_AC_RBUS.kme_me2_vbuf_2nd_pfv0_candi_dediff_pnt = CandiTbl->_cand_pnt._2nd_others_pnt._2nd_pfv0_cand_dediff_pnt;
	kme_me2_vbuf_top_B8_RBUS.kme_me2_vbuf_2nd_pfv1_candi_dediff_pnt = CandiTbl->_cand_pnt._2nd_others_pnt._2nd_pfv1_cand_dediff_pnt;
	kme_me2_vbuf_top_9C_RBUS.kme_me2_vbuf_2nd_candi_zmv_pnt = CandiTbl->_cand_pnt._2nd_others_pnt._2nd_zmv_pnt;
	kme_me2_vbuf_top_9C_RBUS.kme_me2_vbuf_2nd_candi_gmv_pnt = CandiTbl->_cand_pnt._2nd_others_pnt._2nd_gmv_pnt;

#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	kme_me2_vbuf_top_D0_RBUS.kme_me2_vbuf_phase_gain0 = CandiTbl->_ph_index_setting._ph_index0.gain;
	kme_me2_vbuf_top_D0_RBUS.kme_me2_vbuf_phase_gain1 = CandiTbl->_ph_index_setting._ph_index1.gain;
	kme_me2_vbuf_top_D0_RBUS.kme_me2_vbuf_phase_gain2 = CandiTbl->_ph_index_setting._ph_index2.gain;
	kme_me2_vbuf_top_D0_RBUS.kme_me2_vbuf_phase_gain3 = CandiTbl->_ph_index_setting._ph_index3.gain;
	kme_me2_vbuf_top_CC_RBUS.kme_me2_vbuf_phase_offset0 = CandiTbl->_ph_index_setting._ph_index0.offset;
	kme_me2_vbuf_top_CC_RBUS.kme_me2_vbuf_phase_offset1 = CandiTbl->_ph_index_setting._ph_index1.offset;
	kme_me2_vbuf_top_CC_RBUS.kme_me2_vbuf_phase_offset2 = CandiTbl->_ph_index_setting._ph_index2.offset;
	kme_me2_vbuf_top_CC_RBUS.kme_me2_vbuf_phase_offset3 = CandiTbl->_ph_index_setting._ph_index3.offset;
#endif

	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg, kme_me2_vbuf_top_2C_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, kme_me2_vbuf_top_30_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg, kme_me2_vbuf_top_4C_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, kme_me2_vbuf_top_34_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg, kme_me2_vbuf_top_50_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg, kme_me2_vbuf_top_54_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg, kme_me2_vbuf_top_58_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg, kme_me2_vbuf_top_5C_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, kme_me2_vbuf_top_60_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_64_reg, kme_me2_vbuf_top_64_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_68_reg, kme_me2_vbuf_top_68_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_6C_reg, kme_me2_vbuf_top_6C_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_70_reg, kme_me2_vbuf_top_70_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_74_reg, kme_me2_vbuf_top_74_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_78_reg, kme_me2_vbuf_top_78_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_7C_reg, kme_me2_vbuf_top_7C_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_80_reg, kme_me2_vbuf_top_80_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg, kme_me2_vbuf_top_84_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg, kme_me2_vbuf_top_88_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg, kme_me2_vbuf_top_8C_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg, kme_me2_vbuf_top_90_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg, kme_me2_vbuf_top_94_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg, kme_me2_vbuf_top_98_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_9C_reg, kme_me2_vbuf_top_9C_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A0_reg, kme_me2_vbuf_top_A0_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A4_reg, kme_me2_vbuf_top_A4_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A8_reg, kme_me2_vbuf_top_A8_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_AC_reg, kme_me2_vbuf_top_AC_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B0_reg, kme_me2_vbuf_top_B0_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B4_reg, kme_me2_vbuf_top_B4_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B8_reg, kme_me2_vbuf_top_B8_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_BC_reg, kme_me2_vbuf_top_BC_RBUS.regValue);
#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)	
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_CC_reg, kme_me2_vbuf_top_CC_RBUS.regValue);
	rtd_outl(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D0_reg, kme_me2_vbuf_top_D0_RBUS.regValue);	
#endif
	
#else
	unsigned int u8_candiNum_me2_1st = 0, u8_candiNum_me2_2nd = 0;

	u8_candiNum_me2_1st = 	(CandiTbl->_cand_num._1st_zmv_en) + (CandiTbl->_cand_num._1st_gmv_en) +
							(CandiTbl->_cand_num._1st_ph_cand_num * CandiTbl->_cand_num._1st_ph_offset_cand_num) +
							(CandiTbl->_cand_num._1st_update_cand_num) +
							(CandiTbl->_cand_num._1st_pfv0_cand_num * CandiTbl->_cand_num._1st_pfv0_offset_cand_num) +
							(CandiTbl->_cand_num._1st_pfv1_cand_num * CandiTbl->_cand_num._1st_pfv1_offset_cand_num);

	u8_candiNum_me2_2nd = 	(CandiTbl->_cand_num._2nd_zmv_en) + (CandiTbl->_cand_num._2nd_gmv_en) +
							(CandiTbl->_cand_num._2nd_ph_cand_num * CandiTbl->_cand_num._2nd_ph_offset_cand_num) +
							(CandiTbl->_cand_num._2nd_update_cand_num) +
							(CandiTbl->_cand_num._2nd_pfv0_cand_num * CandiTbl->_cand_num._2nd_pfv0_offset_cand_num) +
							(CandiTbl->_cand_num._2nd_pfv1_cand_num * CandiTbl->_cand_num._2nd_pfv1_offset_cand_num);

	rtd_pr_memc_notice( "[%s] me2 : %d %d\n", __func__ , u8_candiNum_me2_1st, u8_candiNum_me2_2nd);

//======== for _str_cand_num	========
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg, 21, 25, 			CandiTbl->_cand_num._1st_ph_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_2C_reg, 26, 28, 	CandiTbl->_cand_num._1st_ph_offset_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 18, 23, 		CandiTbl->_cand_num._1st_update_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 0, 4, 			CandiTbl->_cand_num._1st_pfv0_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 5, 7, 	CandiTbl->_cand_num._1st_pfv0_offset_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 8, 12, 			CandiTbl->_cand_num._1st_pfv1_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 13, 15, 	CandiTbl->_cand_num._1st_pfv1_offset_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg, 1, 1, 		CandiTbl->_cand_num._1st_ph_cand_dediff_en);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg, 3, 3, 		CandiTbl->_cand_num._1st_pfv0_cand_dediff_en);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg, 5, 5, 		CandiTbl->_cand_num._1st_pfv1_cand_dediff_en);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 16, 16, 					CandiTbl->_cand_num._1st_zmv_en);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 17, 17, 					CandiTbl->_cand_num._1st_gmv_en);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 27, 31, 			CandiTbl->_cand_num._2nd_ph_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 0, 2, 		CandiTbl->_cand_num._2nd_ph_offset_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 21, 26, 		CandiTbl->_cand_num._2nd_update_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 3, 7, 			CandiTbl->_cand_num._2nd_pfv0_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 8, 10, 	CandiTbl->_cand_num._2nd_pfv0_offset_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 11, 15, 		CandiTbl->_cand_num._2nd_pfv1_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 16, 18, 	CandiTbl->_cand_num._2nd_pfv1_offset_cand_num);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg, 1, 1, 		CandiTbl->_cand_num._2nd_ph_cand_dediff_en);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg, 3, 3, 		CandiTbl->_cand_num._2nd_pfv0_cand_dediff_en);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg, 5, 5, 		CandiTbl->_cand_num._2nd_pfv1_cand_dediff_en);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 19, 19, 					CandiTbl->_cand_num._2nd_zmv_en);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 20, 20, 					CandiTbl->_cand_num._2nd_gmv_en);

//======= for _str_cand_location ========
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg, 8, 10, 		CandiTbl->_cand_location._1st_ph_cand._ph_cand0.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg, 11, 13, 	CandiTbl->_cand_location._1st_ph_cand._ph_cand0.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg, 14, 16, 	CandiTbl->_cand_location._1st_ph_cand._ph_cand1.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg, 17, 19, 	CandiTbl->_cand_location._1st_ph_cand._ph_cand1.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg, 20, 22, 	CandiTbl->_cand_location._1st_ph_cand._ph_cand2.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg, 23, 25, 	CandiTbl->_cand_location._1st_ph_cand._ph_cand2.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg, 26, 28, 	CandiTbl->_cand_location._1st_ph_cand._ph_cand3.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg, 29, 31, 	CandiTbl->_cand_location._1st_ph_cand._ph_cand3.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg, 0, 2, 		CandiTbl->_cand_location._1st_ph_cand._ph_cand4.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg, 3, 5, 		CandiTbl->_cand_location._1st_ph_cand._ph_cand4.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg, 6, 8, 		CandiTbl->_cand_location._1st_ph_cand._ph_cand5.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg, 9, 11, 		CandiTbl->_cand_location._1st_ph_cand._ph_cand5.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg, 12, 14, 	CandiTbl->_cand_location._1st_ph_cand._ph_cand6.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg, 15, 17, 	CandiTbl->_cand_location._1st_ph_cand._ph_cand6.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg, 18, 20, 	CandiTbl->_cand_location._1st_ph_cand._ph_cand7.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg, 21, 23, 	CandiTbl->_cand_location._1st_ph_cand._ph_cand7.offset_y);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 0, 2, 	CandiTbl->_cand_location._1st_update_cand._update_cand0.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 3, 5,		CandiTbl->_cand_location._1st_update_cand._update_cand0.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 6, 8,		CandiTbl->_cand_location._1st_update_cand._update_cand1.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 9, 11,	CandiTbl->_cand_location._1st_update_cand._update_cand1.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 12, 14, 	CandiTbl->_cand_location._1st_update_cand._update_cand2.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 15, 17, 	CandiTbl->_cand_location._1st_update_cand._update_cand2.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 18, 20, 	CandiTbl->_cand_location._1st_update_cand._update_cand3.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 21, 23, 	CandiTbl->_cand_location._1st_update_cand._update_cand3.offset_y);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg, 24, 26, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand0.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg, 27, 29, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand0.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg, 0, 2, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand1.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg, 3, 5, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand1.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg, 6, 8, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand2.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg, 9, 11, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand2.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg, 12, 14, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand3.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg, 15, 17, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand3.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg, 18, 20, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand4.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg, 21, 23, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand4.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg, 24, 26, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand5.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg, 27, 29, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand5.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg, 0, 2, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand6.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg, 3, 5, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand6.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg, 6, 8, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand7.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg, 9, 11, 	CandiTbl->_cand_location._1st_pfv0_cand._pfv0_cand7.offset_y);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg, 12, 14, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand0.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg, 15, 17, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand0.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg, 18, 20, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand1.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg, 21, 23, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand1.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg, 24, 26, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand2.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg, 27, 29, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand2.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg, 0, 2, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand3.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg, 3, 5, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand3.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg, 6, 8, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand4.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg, 9, 11, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand4.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg, 12, 14, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand5.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg, 15, 17, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand5.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg, 18, 20, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand6.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg, 21, 23, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand6.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg, 24, 26, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand7.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg, 27, 29, 	CandiTbl->_cand_location._1st_pfv1_cand._pfv1_cand7.offset_y);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg, 7, 9, 		CandiTbl->_cand_location._2nd_ph_cand._ph_cand0.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg, 10, 12, 	CandiTbl->_cand_location._2nd_ph_cand._ph_cand0.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg, 13, 15, 	CandiTbl->_cand_location._2nd_ph_cand._ph_cand1.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg, 16, 18, 	CandiTbl->_cand_location._2nd_ph_cand._ph_cand1.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg, 19, 21, 	CandiTbl->_cand_location._2nd_ph_cand._ph_cand2.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg, 22, 24, 	CandiTbl->_cand_location._2nd_ph_cand._ph_cand2.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg, 25, 27, 	CandiTbl->_cand_location._2nd_ph_cand._ph_cand3.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg, 28, 30, 	CandiTbl->_cand_location._2nd_ph_cand._ph_cand3.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg, 0, 2, 		CandiTbl->_cand_location._2nd_ph_cand._ph_cand4.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg, 3, 5, 		CandiTbl->_cand_location._2nd_ph_cand._ph_cand4.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg, 6, 8, 		CandiTbl->_cand_location._2nd_ph_cand._ph_cand5.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg, 9, 11, 		CandiTbl->_cand_location._2nd_ph_cand._ph_cand5.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg, 12, 14, 	CandiTbl->_cand_location._2nd_ph_cand._ph_cand6.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg, 15, 17, 	CandiTbl->_cand_location._2nd_ph_cand._ph_cand6.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg, 18, 20, 	CandiTbl->_cand_location._2nd_ph_cand._ph_cand7.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg, 21, 23, 	CandiTbl->_cand_location._2nd_ph_cand._ph_cand7.offset_y);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg, 0, 2, 	CandiTbl->_cand_location._2nd_update_cand._update_cand0.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg, 3, 5,		CandiTbl->_cand_location._2nd_update_cand._update_cand0.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg, 6, 8,		CandiTbl->_cand_location._2nd_update_cand._update_cand1.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg, 9, 11,	CandiTbl->_cand_location._2nd_update_cand._update_cand1.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg, 12, 14, 	CandiTbl->_cand_location._2nd_update_cand._update_cand2.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg, 15, 17, 	CandiTbl->_cand_location._2nd_update_cand._update_cand2.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg, 18, 20, 	CandiTbl->_cand_location._2nd_update_cand._update_cand3.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg, 21, 23, 	CandiTbl->_cand_location._2nd_update_cand._update_cand3.offset_y);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg, 24, 26, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand0.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg, 27, 29, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand0.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg, 0, 2, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand1.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg, 3, 5, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand1.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg, 6, 8, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand2.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg, 9, 11, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand2.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg, 12, 14, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand3.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg, 15, 17, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand3.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg, 18, 20, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand4.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg, 21, 23, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand4.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg, 24, 26, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand5.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg, 27, 29, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand5.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg, 0, 2, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand6.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg, 3, 5, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand6.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg, 6, 8, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand7.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg, 9, 11, 	CandiTbl->_cand_location._2nd_pfv0_cand._pfv0_cand7.offset_y);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg, 12, 14, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand0.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg, 15, 17, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand0.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg, 18, 20, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand1.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg, 21, 23, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand1.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg, 24, 26, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand2.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg, 27, 29, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand2.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg, 0, 2, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand3.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg, 3, 5, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand3.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg, 6, 8, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand4.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg, 9, 11, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand4.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg, 12, 14, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand5.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg, 15, 17, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand5.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg, 18, 20, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand6.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg, 21, 23, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand6.offset_y);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg, 24, 26, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand7.offset_x);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg, 27, 29, 	CandiTbl->_cand_location._2nd_pfv1_cand._pfv1_cand7.offset_y);

//======== _str_cand_pnt ==========
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_64_reg, 8, 15, 	CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt0);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_64_reg, 16, 23, 	CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt1);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_64_reg, 24, 31, 	CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt2);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_68_reg, 0, 7, 		CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt3);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_68_reg, 8, 15, 	CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt4);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_68_reg, 16, 23, 	CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt5);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_68_reg, 24, 31, 	CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt6);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_6C_reg, 0, 7, 		CandiTbl->_cand_pnt._1st_ph_pnt.ph_pnt7);
	
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_80_reg, 0, 7, 		CandiTbl->_cand_pnt._1st_update_pnt.update_pnt0);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_80_reg, 8, 15, 		CandiTbl->_cand_pnt._1st_update_pnt.update_pnt1);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_80_reg, 16, 23, 	CandiTbl->_cand_pnt._1st_update_pnt.update_pnt2);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_80_reg, 24, 31, 	CandiTbl->_cand_pnt._1st_update_pnt.update_pnt3);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_6C_reg, 16, 23, 	CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt0);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_6C_reg, 24, 31, 	CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt1);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_70_reg, 0, 7, 	CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt2);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_70_reg, 8, 15, 	CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt3);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_70_reg, 16, 23, 	CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt4);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_70_reg, 24, 31, 	CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt5);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_74_reg, 0, 7, 	CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt6);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_74_reg, 8, 15, 	CandiTbl->_cand_pnt._1st_pfv0_pnt.pfv0_pnt7);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_74_reg, 24, 31, 	CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt0);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_78_reg, 0, 7, 	CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt1);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_78_reg, 8, 15, 	CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt2);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_78_reg, 16, 23, 	CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt3);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_78_reg, 24, 31, 	CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt4);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_7C_reg, 0, 7, 	CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt5);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_7C_reg, 8, 15, 	CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt6);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_7C_reg, 16, 23, 	CandiTbl->_cand_pnt._1st_pfv1_pnt.pfv1_pnt7);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_6C_reg, 8, 15, 		CandiTbl->_cand_pnt._1st_others_pnt._1st_ph_cand_dediff_pnt);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_74_reg, 16, 23, 	CandiTbl->_cand_pnt._1st_others_pnt._1st_pfv0_cand_dediff_pnt);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_7C_reg, 24, 31, 	CandiTbl->_cand_pnt._1st_others_pnt._1st_pfv1_cand_dediff_pnt);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg, 24, 31, 			CandiTbl->_cand_pnt._1st_others_pnt._1st_zmv_pnt);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_64_reg, 0, 7, 			CandiTbl->_cand_pnt._1st_others_pnt._1st_gmv_pnt);
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)	
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D0_reg, 7, 12, 		CandiTbl->_ph_index_setting._ph_index0.gain);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D0_reg, 13, 18, 	CandiTbl->_ph_index_setting._ph_index1.gain);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D0_reg, 19, 24, 	CandiTbl->_ph_index_setting._ph_index2.gain);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D0_reg, 25, 30, 	CandiTbl->_ph_index_setting._ph_index3.gain);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_CC_reg, 0, 6, 	CandiTbl->_ph_index_setting._ph_index0.offset);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_CC_reg, 7, 13, 	CandiTbl->_ph_index_setting._ph_index1.offset);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_CC_reg, 14, 20, 	CandiTbl->_ph_index_setting._ph_index2.offset);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_CC_reg, 21, 27, 	CandiTbl->_ph_index_setting._ph_index3.offset);
	#endif

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_9C_reg, 16, 23, 	CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt0);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_9C_reg, 24, 31, 	CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt1);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A0_reg, 0, 7, 		CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt2);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A0_reg, 8, 15, 	CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt3);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A0_reg, 16, 23, 	CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt4);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A0_reg, 24, 31, 	CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt5);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A4_reg, 0, 7, 		CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt6);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A4_reg, 8, 15, 	CandiTbl->_cand_pnt._2nd_ph_pnt.ph_pnt7);
	
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B8_reg, 8, 15, 		CandiTbl->_cand_pnt._2nd_update_pnt.update_pnt0);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B8_reg, 16, 23, 	CandiTbl->_cand_pnt._2nd_update_pnt.update_pnt1);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B8_reg, 24, 31, 	CandiTbl->_cand_pnt._2nd_update_pnt.update_pnt2);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_BC_reg, 0, 7, 		CandiTbl->_cand_pnt._2nd_update_pnt.update_pnt3);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A4_reg, 24, 31, 	CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt0);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A8_reg, 0, 7, 	CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt1);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A8_reg, 8, 15, 	CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt2);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A8_reg, 16, 23, 	CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt3);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A8_reg, 24, 31, 	CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt4);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_AC_reg, 0, 7, 	CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt5);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_AC_reg, 8, 15, 	CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt6);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_AC_reg, 16, 23, 	CandiTbl->_cand_pnt._2nd_pfv0_pnt.pfv0_pnt7);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B0_reg, 0, 7, 	CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt0);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B0_reg, 8, 15, 	CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt1);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B0_reg, 16, 23, 	CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt2);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B0_reg, 24, 31, 	CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt3);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B4_reg, 0, 7, 	CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt4);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B4_reg, 8, 15, 	CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt5);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B4_reg, 16, 23, 	CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt6);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B4_reg, 24, 31, 	CandiTbl->_cand_pnt._2nd_pfv1_pnt.pfv1_pnt7);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_A4_reg, 16, 23, 	CandiTbl->_cand_pnt._2nd_others_pnt._2nd_ph_cand_dediff_pnt);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_AC_reg, 24, 31, 	CandiTbl->_cand_pnt._2nd_others_pnt._2nd_pfv0_cand_dediff_pnt);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_B8_reg, 0, 7, 	CandiTbl->_cand_pnt._2nd_others_pnt._2nd_pfv1_cand_dediff_pnt);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_9C_reg, 0, 7, 			CandiTbl->_cand_pnt._2nd_others_pnt._2nd_zmv_pnt);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_9C_reg, 8, 15, 			CandiTbl->_cand_pnt._2nd_others_pnt._2nd_gmv_pnt);

	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//======== _str_ph_index_setting ========== 
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D0_reg, 7, 12, 		CandiTbl->_ph_index_setting._ph_index0.gain);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D0_reg, 13, 18, 	CandiTbl->_ph_index_setting._ph_index1.gain);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D0_reg, 19, 24, 	CandiTbl->_ph_index_setting._ph_index2.gain);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D0_reg, 25, 30, 	CandiTbl->_ph_index_setting._ph_index3.gain);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_CC_reg, 0, 6, 	CandiTbl->_ph_index_setting._ph_index0.offset);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_CC_reg, 7, 13, 	CandiTbl->_ph_index_setting._ph_index1.offset);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_CC_reg, 14, 20, 	CandiTbl->_ph_index_setting._ph_index2.offset);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_CC_reg, 21, 27, 	CandiTbl->_ph_index_setting._ph_index3.offset);
	#endif

#endif


}

VOID HAL_ME1_cand_wrtDriver( _str_ME1_Candidate_table *CandiTbl)
{

#if RTK_MEMC_Performance_tunging_from_tv001
	unsigned int u8_candiNum_me_ip = 0, u8_candiNum_me_pi = 0;
	kme_vbuf_top_kme_vbuf_top_10_RBUS kme_vbuf_top_10_RBUS;
	kme_vbuf_top_kme_vbuf_top_14_RBUS kme_vbuf_top_14_RBUS;
	kme_vbuf_top_kme_vbuf_top_18_RBUS kme_vbuf_top_18_RBUS;
	kme_vbuf_top_kme_vbuf_top_1c_RBUS kme_vbuf_top_1C_RBUS;
	kme_vbuf_top_kme_vbuf_top_20_RBUS kme_vbuf_top_20_RBUS;
	kme_vbuf_top_kme_vbuf_top_24_RBUS kme_vbuf_top_24_RBUS;
	kme_vbuf_top_kme_vbuf_top_28_RBUS kme_vbuf_top_28_RBUS;
	kme_vbuf_top_kme_vbuf_top_2c_RBUS kme_vbuf_top_2C_RBUS;
	kme_me1_top0_kme_me1_top0_1c_RBUS kme_me1_top0_1C_RBUS;
	kme_me1_top0_kme_me1_top0_20_RBUS kme_me1_top0_20_RBUS;
	kme_me1_top0_kme_me1_top0_24_RBUS kme_me1_top0_24_RBUS;
	kme_me1_top0_kme_me1_top0_28_RBUS kme_me1_top0_28_RBUS;
	kme_me1_top0_kme_me1_top0_2c_RBUS kme_me1_top0_2C_RBUS;
	kme_me1_top0_kme_me1_top0_5c_RBUS kme_me1_top0_5C_RBUS;
	kme_me1_top0_kme_me1_top0_60_RBUS kme_me1_top0_60_RBUS;
	kme_me1_top0_kme_me1_top0_64_RBUS kme_me1_top0_64_RBUS;
	kme_me1_top0_kme_me1_top0_68_RBUS kme_me1_top0_68_RBUS;
	kme_me1_top0_kme_me1_top0_6c_RBUS kme_me1_top0_6C_RBUS;
	kme_me1_top0_kme_me1_top0_b0_RBUS kme_me1_top0_B0_RBUS;
	kme_me1_top0_kme_me1_top0_b4_RBUS kme_me1_top0_B4_RBUS;
	kme_me1_top0_kme_me1_top0_b8_RBUS kme_me1_top0_B8_RBUS;
	kme_me1_top0_kme_me1_top0_bc_RBUS kme_me1_top0_BC_RBUS;
	kme_me1_top0_kme_me1_top0_c0_RBUS kme_me1_top0_C0_RBUS;
	kme_me1_top0_kme_me1_top0_c4_RBUS kme_me1_top0_C4_RBUS;

	kme_vbuf_top_10_RBUS.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_10_reg);
	kme_vbuf_top_14_RBUS.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_14_reg);
	kme_vbuf_top_18_RBUS.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_18_reg);
	kme_vbuf_top_1C_RBUS.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg);
	kme_vbuf_top_20_RBUS.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_20_reg);
	kme_vbuf_top_24_RBUS.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_24_reg);
	kme_vbuf_top_28_RBUS.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_28_reg);
	kme_vbuf_top_2C_RBUS.regValue = rtd_inl(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg);
	kme_me1_top0_1C_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg);
	kme_me1_top0_20_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_20_reg);
	kme_me1_top0_24_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_24_reg);
	kme_me1_top0_28_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_28_reg);
	kme_me1_top0_2C_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_2C_reg);
	kme_me1_top0_5C_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg);
	kme_me1_top0_60_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_60_reg);
	kme_me1_top0_64_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_64_reg);
	kme_me1_top0_68_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_68_reg);
	kme_me1_top0_6C_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_6C_reg);
	kme_me1_top0_B0_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_B0_reg);
	kme_me1_top0_B4_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_B4_reg);
	kme_me1_top0_B8_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_B8_reg);
	kme_me1_top0_BC_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_BC_reg);
	kme_me1_top0_C0_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_C0_reg);
	kme_me1_top0_C4_RBUS.regValue = rtd_inl(KME_ME1_TOP0_KME_ME1_TOP0_C4_reg);
	
	u8_candiNum_me_ip = CandiTbl->_cand_ip_num._zmv_en + CandiTbl->_cand_ip_num._gmv_en +
						CandiTbl->_cand_ip_num._cand_en[0] + CandiTbl->_cand_ip_num._cand_en[1] +
						CandiTbl->_cand_ip_num._cand_en[2] + CandiTbl->_cand_ip_num._cand_en[3] +
						CandiTbl->_cand_ip_num._cand_en[4] + CandiTbl->_cand_ip_num._cand_en[5] +
						CandiTbl->_cand_ip_num._cand_en[6] + CandiTbl->_cand_ip_num._cand_en[7] +
						CandiTbl->_cand_ip_num._cand_en[8] + CandiTbl->_cand_ip_num._cand_en[9] +
						CandiTbl->_cand_ip_num._upt_cand_en[0] + CandiTbl->_cand_ip_num._upt_cand_en[1] +
						CandiTbl->_cand_ip_num._upt_cand_en[2] + CandiTbl->_cand_ip_num._upt_cand_en[3];

	u8_candiNum_me_pi = CandiTbl->_cand_pi_num._zmv_en + CandiTbl->_cand_pi_num._gmv_en +
						CandiTbl->_cand_pi_num._cand_en[0] + CandiTbl->_cand_pi_num._cand_en[1] +
						CandiTbl->_cand_pi_num._cand_en[2] + CandiTbl->_cand_pi_num._cand_en[3] +
						CandiTbl->_cand_pi_num._cand_en[4] + CandiTbl->_cand_pi_num._cand_en[5] +
						CandiTbl->_cand_pi_num._cand_en[6] + CandiTbl->_cand_pi_num._cand_en[7] +
						CandiTbl->_cand_pi_num._cand_en[8] + CandiTbl->_cand_pi_num._cand_en[9] +
						CandiTbl->_cand_pi_num._upt_cand_en[0] + CandiTbl->_cand_pi_num._upt_cand_en[1] +
						CandiTbl->_cand_pi_num._upt_cand_en[2] + CandiTbl->_cand_pi_num._upt_cand_en[3];

	//rtd_pr_memc_notice("[%s] me1 ip %d pi %d\n", __func__ , u8_candiNum_me_ip, u8_candiNum_me_ip);

	// ip cand enable
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_cand0_en = CandiTbl->_cand_ip_num._cand_en[0];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_cand1_en = CandiTbl->_cand_ip_num._cand_en[1];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_cand2_en = CandiTbl->_cand_ip_num._cand_en[2];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_cand3_en = CandiTbl->_cand_ip_num._cand_en[3];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_cand4_en = CandiTbl->_cand_ip_num._cand_en[4];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_cand5_en = CandiTbl->_cand_ip_num._cand_en[5];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_cand6_en = CandiTbl->_cand_ip_num._cand_en[6];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_cand7_en = CandiTbl->_cand_ip_num._cand_en[7];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_cand8_en = CandiTbl->_cand_ip_num._cand_en[8];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_cand9_en = CandiTbl->_cand_ip_num._cand_en[9];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_update0_en = CandiTbl->_cand_ip_num._upt_cand_en[0];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_update1_en = CandiTbl->_cand_ip_num._upt_cand_en[1];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_update2_en = CandiTbl->_cand_ip_num._upt_cand_en[2];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_update3_en = CandiTbl->_cand_ip_num._upt_cand_en[3];
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_zmv_en = CandiTbl->_cand_ip_num._zmv_en;
	kme_vbuf_top_10_RBUS.vbuf_ip_1st_gmv_en = CandiTbl->_cand_ip_num._gmv_en;

	// pi cand enable
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_cand0_en = CandiTbl->_cand_pi_num._cand_en[0];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_cand1_en = CandiTbl->_cand_pi_num._cand_en[1];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_cand2_en = CandiTbl->_cand_pi_num._cand_en[2];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_cand3_en = CandiTbl->_cand_pi_num._cand_en[3];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_cand4_en = CandiTbl->_cand_pi_num._cand_en[4];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_cand5_en = CandiTbl->_cand_pi_num._cand_en[5];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_cand6_en = CandiTbl->_cand_pi_num._cand_en[6];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_cand7_en = CandiTbl->_cand_pi_num._cand_en[7];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_cand8_en = CandiTbl->_cand_pi_num._cand_en[8];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_cand9_en = CandiTbl->_cand_pi_num._cand_en[9];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_update0_en = CandiTbl->_cand_pi_num._upt_cand_en[0];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_update1_en = CandiTbl->_cand_pi_num._upt_cand_en[1];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_update2_en = CandiTbl->_cand_pi_num._upt_cand_en[2];
	kme_vbuf_top_14_RBUS.vbuf_pi_1st_update3_en = CandiTbl->_cand_pi_num._upt_cand_en[3];

	// ip offset
	kme_vbuf_top_18_RBUS.vbuf_ip_1st_cand0_offsetx = CandiTbl->_cand_ip_location._cand[0].offset_x;
	kme_vbuf_top_18_RBUS.vbuf_ip_1st_cand0_offsety = CandiTbl->_cand_ip_location._cand[0].offset_y;
	kme_vbuf_top_18_RBUS.vbuf_ip_1st_cand1_offsetx = CandiTbl->_cand_ip_location._cand[1].offset_x;
	kme_vbuf_top_18_RBUS.vbuf_ip_1st_cand1_offsety = CandiTbl->_cand_ip_location._cand[1].offset_y;
	kme_vbuf_top_18_RBUS.vbuf_ip_1st_cand2_offsetx = CandiTbl->_cand_ip_location._cand[2].offset_x;
	kme_vbuf_top_18_RBUS.vbuf_ip_1st_cand2_offsety = CandiTbl->_cand_ip_location._cand[2].offset_y;
	kme_vbuf_top_18_RBUS.vbuf_ip_1st_cand3_offsetx = CandiTbl->_cand_ip_location._cand[3].offset_x;
	kme_vbuf_top_18_RBUS.vbuf_ip_1st_cand3_offsety = CandiTbl->_cand_ip_location._cand[3].offset_y;
	kme_vbuf_top_18_RBUS.vbuf_ip_1st_cand4_offsetx = CandiTbl->_cand_ip_location._cand[4].offset_x;
	kme_vbuf_top_18_RBUS.vbuf_ip_1st_cand4_offsety = CandiTbl->_cand_ip_location._cand[4].offset_y;
	kme_vbuf_top_1C_RBUS.vbuf_ip_1st_cand5_offsetx = CandiTbl->_cand_ip_location._cand[5].offset_x;
	kme_vbuf_top_1C_RBUS.vbuf_ip_1st_cand5_offsety = CandiTbl->_cand_ip_location._cand[5].offset_y;
	kme_vbuf_top_1C_RBUS.vbuf_ip_1st_cand6_offsetx = CandiTbl->_cand_ip_location._cand[6].offset_x;
	kme_vbuf_top_1C_RBUS.vbuf_ip_1st_cand6_offsety = CandiTbl->_cand_ip_location._cand[6].offset_y;
	kme_vbuf_top_1C_RBUS.vbuf_ip_1st_cand7_offsetx = CandiTbl->_cand_ip_location._cand[7].offset_x;
	kme_vbuf_top_1C_RBUS.vbuf_ip_1st_cand7_offsety = CandiTbl->_cand_ip_location._cand[7].offset_y;
	kme_vbuf_top_1C_RBUS.vbuf_ip_1st_cand8_offsetx = CandiTbl->_cand_ip_location._cand[8].offset_x;
	kme_vbuf_top_1C_RBUS.vbuf_ip_1st_cand8_offsety = CandiTbl->_cand_ip_location._cand[8].offset_y;
	kme_vbuf_top_1C_RBUS.vbuf_ip_1st_cand9_offsetx = CandiTbl->_cand_ip_location._cand[9].offset_x;
	kme_vbuf_top_1C_RBUS.vbuf_ip_1st_cand9_offsety = CandiTbl->_cand_ip_location._cand[9].offset_y;

	kme_vbuf_top_20_RBUS.vbuf_ip_1st_update0_offsetx = CandiTbl->_cand_ip_location._upt[0].offset_x;
	kme_vbuf_top_20_RBUS.vbuf_ip_1st_update0_offsety = CandiTbl->_cand_ip_location._upt[0].offset_y;
	kme_vbuf_top_20_RBUS.vbuf_ip_1st_update1_offsetx = CandiTbl->_cand_ip_location._upt[1].offset_x;
	kme_vbuf_top_20_RBUS.vbuf_ip_1st_update1_offsety = CandiTbl->_cand_ip_location._upt[1].offset_y;
	kme_vbuf_top_20_RBUS.vbuf_ip_1st_update2_offsetx = CandiTbl->_cand_ip_location._upt[2].offset_x;
	kme_vbuf_top_20_RBUS.vbuf_ip_1st_update2_offsety = CandiTbl->_cand_ip_location._upt[2].offset_y;
	kme_vbuf_top_20_RBUS.vbuf_ip_1st_update3_offsetx = CandiTbl->_cand_ip_location._upt[3].offset_x;
	kme_vbuf_top_20_RBUS.vbuf_ip_1st_update3_offsety = CandiTbl->_cand_ip_location._upt[3].offset_y;
	
	// pi offset
	kme_vbuf_top_24_RBUS.vbuf_pi_1st_cand0_offsetx = CandiTbl->_cand_pi_location._cand[0].offset_x;
	kme_vbuf_top_24_RBUS.vbuf_pi_1st_cand0_offsety = CandiTbl->_cand_pi_location._cand[0].offset_y;
	kme_vbuf_top_24_RBUS.vbuf_pi_1st_cand1_offsetx = CandiTbl->_cand_pi_location._cand[1].offset_x;
	kme_vbuf_top_24_RBUS.vbuf_pi_1st_cand1_offsety = CandiTbl->_cand_pi_location._cand[1].offset_y;
	kme_vbuf_top_24_RBUS.vbuf_pi_1st_cand2_offsetx = CandiTbl->_cand_pi_location._cand[2].offset_x;
	kme_vbuf_top_24_RBUS.vbuf_pi_1st_cand2_offsety = CandiTbl->_cand_pi_location._cand[2].offset_y;
	kme_vbuf_top_24_RBUS.vbuf_pi_1st_cand3_offsetx = CandiTbl->_cand_pi_location._cand[3].offset_x;
	kme_vbuf_top_24_RBUS.vbuf_pi_1st_cand3_offsety = CandiTbl->_cand_pi_location._cand[3].offset_y;
	kme_vbuf_top_24_RBUS.vbuf_pi_1st_cand4_offsetx = CandiTbl->_cand_pi_location._cand[4].offset_x;
	kme_vbuf_top_24_RBUS.vbuf_pi_1st_cand4_offsety = CandiTbl->_cand_pi_location._cand[4].offset_y;
	kme_vbuf_top_28_RBUS.vbuf_pi_1st_cand5_offsetx = CandiTbl->_cand_pi_location._cand[5].offset_x;
	kme_vbuf_top_28_RBUS.vbuf_pi_1st_cand5_offsety = CandiTbl->_cand_pi_location._cand[5].offset_y;
	kme_vbuf_top_28_RBUS.vbuf_pi_1st_cand6_offsetx = CandiTbl->_cand_pi_location._cand[6].offset_x;
	kme_vbuf_top_28_RBUS.vbuf_pi_1st_cand6_offsety = CandiTbl->_cand_pi_location._cand[6].offset_y;
	kme_vbuf_top_28_RBUS.vbuf_pi_1st_cand7_offsetx = CandiTbl->_cand_pi_location._cand[7].offset_x;
	kme_vbuf_top_28_RBUS.vbuf_pi_1st_cand7_offsety = CandiTbl->_cand_pi_location._cand[7].offset_y;
	kme_vbuf_top_28_RBUS.vbuf_pi_1st_cand8_offsetx = CandiTbl->_cand_pi_location._cand[8].offset_x;
	kme_vbuf_top_28_RBUS.vbuf_pi_1st_cand8_offsety = CandiTbl->_cand_pi_location._cand[8].offset_y;
	kme_vbuf_top_28_RBUS.vbuf_pi_1st_cand9_offsetx = CandiTbl->_cand_pi_location._cand[9].offset_x;
	kme_vbuf_top_28_RBUS.vbuf_pi_1st_cand9_offsety = CandiTbl->_cand_pi_location._cand[9].offset_y;
	
	kme_vbuf_top_2C_RBUS.vbuf_pi_1st_update0_offsetx = CandiTbl->_cand_pi_location._upt[0].offset_x;
	kme_vbuf_top_2C_RBUS.vbuf_pi_1st_update0_offsety = CandiTbl->_cand_pi_location._upt[0].offset_y;
	kme_vbuf_top_2C_RBUS.vbuf_pi_1st_update1_offsetx = CandiTbl->_cand_pi_location._upt[1].offset_x;
	kme_vbuf_top_2C_RBUS.vbuf_pi_1st_update1_offsety = CandiTbl->_cand_pi_location._upt[1].offset_y;
	kme_vbuf_top_2C_RBUS.vbuf_pi_1st_update2_offsetx = CandiTbl->_cand_pi_location._upt[2].offset_x;
	kme_vbuf_top_2C_RBUS.vbuf_pi_1st_update2_offsety = CandiTbl->_cand_pi_location._upt[2].offset_y;
	kme_vbuf_top_2C_RBUS.vbuf_pi_1st_update3_offsetx = CandiTbl->_cand_pi_location._upt[3].offset_x;
	kme_vbuf_top_2C_RBUS.vbuf_pi_1st_update3_offsety = CandiTbl->_cand_pi_location._upt[3].offset_y;

	// ip pnt
	kme_me1_top0_28_RBUS.me1_ip_cddpnt_zmv = CandiTbl->_cand_ip_pnt._cand_zmv_pnt;
	kme_me1_top0_2C_RBUS.me1_ip_cddpnt_gmv = CandiTbl->_cand_ip_pnt._cand_gmv_pnt;
	kme_me1_top0_2C_RBUS.me1_ip_cddpnt_ppi = CandiTbl->_cand_ip_pnt._cand_ppi_pnt;
	kme_me1_top0_1C_RBUS.me1_ip_cddpnt_st0 = CandiTbl->_cand_ip_pnt._cand_pnt[0];
	kme_me1_top0_1C_RBUS.me1_ip_cddpnt_st1 = CandiTbl->_cand_ip_pnt._cand_pnt[1];
	kme_me1_top0_1C_RBUS.me1_ip_cddpnt_st2 = CandiTbl->_cand_ip_pnt._cand_pnt[2];
	kme_me1_top0_1C_RBUS.me1_ip_cddpnt_st3 = CandiTbl->_cand_ip_pnt._cand_pnt[3];
	kme_me1_top0_20_RBUS.me1_ip_cddpnt_st4 = CandiTbl->_cand_ip_pnt._cand_pnt[4];
	kme_me1_top0_20_RBUS.me1_ip_cddpnt_st5 = CandiTbl->_cand_ip_pnt._cand_pnt[5];
	kme_me1_top0_20_RBUS.me1_ip_cddpnt_st6 = CandiTbl->_cand_ip_pnt._cand_pnt[6];
	kme_me1_top0_20_RBUS.me1_ip_cddpnt_st7 = CandiTbl->_cand_ip_pnt._cand_pnt[7];
	kme_me1_top0_24_RBUS.me1_ip_cddpnt_st8 = CandiTbl->_cand_ip_pnt._cand_pnt[8];
	kme_me1_top0_24_RBUS.me1_ip_cddpnt_st9 = CandiTbl->_cand_ip_pnt._cand_pnt[9];
	//check
	kme_me1_top0_64_RBUS.me1_pi_cddpnt_rnd0 = CandiTbl->_cand_ip_pnt._cand_upt_pnt[0];
	kme_me1_top0_64_RBUS.me1_pi_cddpnt_rnd1 = CandiTbl->_cand_ip_pnt._cand_upt_pnt[1];
	kme_me1_top0_68_RBUS.me1_pi_cddpnt_rnd2 = CandiTbl->_cand_ip_pnt._cand_upt_pnt[2];
	kme_me1_top0_68_RBUS.me1_pi_cddpnt_rnd3 = CandiTbl->_cand_ip_pnt._cand_upt_pnt[3];

	// ip pnt meander
	kme_me1_top0_B0_RBUS.me1_meander_ip_cddpnt_st0 = CandiTbl->_cand_ip_meander_pnt._cand_pnt[0];
	kme_me1_top0_B0_RBUS.me1_meander_ip_cddpnt_st1 = CandiTbl->_cand_ip_meander_pnt._cand_pnt[1];
	kme_me1_top0_B0_RBUS.me1_meander_ip_cddpnt_st2 = CandiTbl->_cand_ip_meander_pnt._cand_pnt[2];
	kme_me1_top0_B0_RBUS.me1_meander_ip_cddpnt_st3 = CandiTbl->_cand_ip_meander_pnt._cand_pnt[3];
	kme_me1_top0_B4_RBUS.me1_meander_ip_cddpnt_st4 = CandiTbl->_cand_ip_meander_pnt._cand_pnt[4];
	kme_me1_top0_B4_RBUS.me1_meander_ip_cddpnt_st5 = CandiTbl->_cand_ip_meander_pnt._cand_pnt[5];
	kme_me1_top0_B4_RBUS.me1_meander_ip_cddpnt_st6 = CandiTbl->_cand_ip_meander_pnt._cand_pnt[6];
	kme_me1_top0_B4_RBUS.me1_meander_ip_cddpnt_st7 = CandiTbl->_cand_ip_meander_pnt._cand_pnt[7];
	kme_me1_top0_B8_RBUS.me1_meander_ip_cddpnt_st8 = CandiTbl->_cand_ip_meander_pnt._cand_pnt[8];
	kme_me1_top0_B8_RBUS.me1_meander_ip_cddpnt_st9 = CandiTbl->_cand_ip_meander_pnt._cand_pnt[9];

	// pi pnt
	kme_me1_top0_5C_RBUS.me1_pi_cddpnt_st0 = CandiTbl->_cand_pi_pnt._cand_pnt[0];
	kme_me1_top0_5C_RBUS.me1_pi_cddpnt_st1 = CandiTbl->_cand_pi_pnt._cand_pnt[1];
	kme_me1_top0_5C_RBUS.me1_pi_cddpnt_st2 = CandiTbl->_cand_pi_pnt._cand_pnt[2];
	kme_me1_top0_5C_RBUS.me1_pi_cddpnt_st3 = CandiTbl->_cand_pi_pnt._cand_pnt[3];
	kme_me1_top0_60_RBUS.me1_pi_cddpnt_st4 = CandiTbl->_cand_pi_pnt._cand_pnt[4];
	kme_me1_top0_60_RBUS.me1_pi_cddpnt_st5 = CandiTbl->_cand_pi_pnt._cand_pnt[5];
	kme_me1_top0_60_RBUS.me1_pi_cddpnt_st6 = CandiTbl->_cand_pi_pnt._cand_pnt[6];
	kme_me1_top0_60_RBUS.me1_pi_cddpnt_st7 = CandiTbl->_cand_pi_pnt._cand_pnt[7];
	kme_me1_top0_64_RBUS.me1_pi_cddpnt_st8 = CandiTbl->_cand_pi_pnt._cand_pnt[8];
	kme_me1_top0_64_RBUS.me1_pi_cddpnt_st9 = CandiTbl->_cand_pi_pnt._cand_pnt[9];
	kme_me1_top0_64_RBUS.me1_pi_cddpnt_rnd0 = CandiTbl->_cand_pi_pnt._cand_upt_pnt[0];
	kme_me1_top0_64_RBUS.me1_pi_cddpnt_rnd1 = CandiTbl->_cand_pi_pnt._cand_upt_pnt[1];
	kme_me1_top0_68_RBUS.me1_pi_cddpnt_rnd2 = CandiTbl->_cand_pi_pnt._cand_upt_pnt[2];
	kme_me1_top0_68_RBUS.me1_pi_cddpnt_rnd3 = CandiTbl->_cand_pi_pnt._cand_upt_pnt[3];
	kme_me1_top0_6C_RBUS.me1_pi_cddpnt_gmv = CandiTbl->_cand_pi_pnt._cand_gmv_pnt;
	kme_me1_top0_68_RBUS.me1_pi_cddpnt_zmv = CandiTbl->_cand_pi_pnt._cand_zmv_pnt;

	// pi pnt meander
	kme_me1_top0_BC_RBUS.me1_meander_pi_cddpnt_st0 = CandiTbl->_cand_pi_meander_pnt._cand_pnt[0];
	kme_me1_top0_BC_RBUS.me1_meander_pi_cddpnt_st1 = CandiTbl->_cand_pi_meander_pnt._cand_pnt[1];
	kme_me1_top0_BC_RBUS.me1_meander_pi_cddpnt_st2 = CandiTbl->_cand_pi_meander_pnt._cand_pnt[2];
	kme_me1_top0_BC_RBUS.me1_meander_pi_cddpnt_st3 = CandiTbl->_cand_pi_meander_pnt._cand_pnt[3];
	kme_me1_top0_C0_RBUS.me1_meander_pi_cddpnt_st4 = CandiTbl->_cand_pi_meander_pnt._cand_pnt[4];
	kme_me1_top0_C0_RBUS.me1_meander_pi_cddpnt_st5 = CandiTbl->_cand_pi_meander_pnt._cand_pnt[5];
	kme_me1_top0_C0_RBUS.me1_meander_pi_cddpnt_st6 = CandiTbl->_cand_pi_meander_pnt._cand_pnt[6];
	kme_me1_top0_C0_RBUS.me1_meander_pi_cddpnt_st7 = CandiTbl->_cand_pi_meander_pnt._cand_pnt[7];
	kme_me1_top0_C4_RBUS.me1_meander_pi_cddpnt_st8 = CandiTbl->_cand_pi_meander_pnt._cand_pnt[8];
	kme_me1_top0_C4_RBUS.me1_meander_pi_cddpnt_st9 = CandiTbl->_cand_pi_meander_pnt._cand_pnt[9];

	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, kme_vbuf_top_10_RBUS.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, kme_vbuf_top_14_RBUS.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, kme_vbuf_top_18_RBUS.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, kme_vbuf_top_1C_RBUS.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, kme_vbuf_top_20_RBUS.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, kme_vbuf_top_24_RBUS.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, kme_vbuf_top_28_RBUS.regValue);
	rtd_outl(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, kme_vbuf_top_2C_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, kme_me1_top0_1C_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, kme_me1_top0_20_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, kme_me1_top0_24_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, kme_me1_top0_28_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_2C_reg, kme_me1_top0_2C_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, kme_me1_top0_64_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, kme_me1_top0_68_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_B0_reg, kme_me1_top0_B0_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_B4_reg, kme_me1_top0_B4_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_B8_reg, kme_me1_top0_B8_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, kme_me1_top0_5C_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, kme_me1_top0_60_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, kme_me1_top0_64_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, kme_me1_top0_68_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_6C_reg, kme_me1_top0_6C_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_BC_reg, kme_me1_top0_BC_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_C0_reg, kme_me1_top0_C0_RBUS.regValue);
	rtd_outl(KME_ME1_TOP0_KME_ME1_TOP0_C4_reg, kme_me1_top0_C4_RBUS.regValue);

	
#else
	
	unsigned int u8_candiNum_me_ip = 0, u8_candiNum_me_pi = 0;

	u8_candiNum_me_ip = CandiTbl->_cand_ip_num._zmv_en + CandiTbl->_cand_ip_num._gmv_en +
						CandiTbl->_cand_ip_num._cand_en[0] + CandiTbl->_cand_ip_num._cand_en[1] +
						CandiTbl->_cand_ip_num._cand_en[2] + CandiTbl->_cand_ip_num._cand_en[3] +
						CandiTbl->_cand_ip_num._cand_en[4] + CandiTbl->_cand_ip_num._cand_en[5] +
						CandiTbl->_cand_ip_num._cand_en[6] + CandiTbl->_cand_ip_num._cand_en[7] +
						CandiTbl->_cand_ip_num._cand_en[8] + CandiTbl->_cand_ip_num._cand_en[9] +
						CandiTbl->_cand_ip_num._upt_cand_en[0] + CandiTbl->_cand_ip_num._upt_cand_en[1] +
						CandiTbl->_cand_ip_num._upt_cand_en[2] + CandiTbl->_cand_ip_num._upt_cand_en[3];

	u8_candiNum_me_pi = CandiTbl->_cand_pi_num._zmv_en + CandiTbl->_cand_pi_num._gmv_en +
						CandiTbl->_cand_pi_num._cand_en[0] + CandiTbl->_cand_pi_num._cand_en[1] +
						CandiTbl->_cand_pi_num._cand_en[2] + CandiTbl->_cand_pi_num._cand_en[3] +
						CandiTbl->_cand_pi_num._cand_en[4] + CandiTbl->_cand_pi_num._cand_en[5] +
						CandiTbl->_cand_pi_num._cand_en[6] + CandiTbl->_cand_pi_num._cand_en[7] +
						CandiTbl->_cand_pi_num._cand_en[8] + CandiTbl->_cand_pi_num._cand_en[9] +
						CandiTbl->_cand_pi_num._upt_cand_en[0] + CandiTbl->_cand_pi_num._upt_cand_en[1] +
						CandiTbl->_cand_pi_num._upt_cand_en[2] + CandiTbl->_cand_pi_num._upt_cand_en[3];

	rtd_pr_memc_notice( "[%s] me1 ip %d pi %d\n", __func__ , u8_candiNum_me_ip, u8_candiNum_me_ip);

	// ip cand enable
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16, CandiTbl->_cand_ip_num._zmv_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 15, 15, CandiTbl->_cand_ip_num._gmv_en);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 1, 1, CandiTbl->_cand_ip_num._cand_en[0]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 2, 2, CandiTbl->_cand_ip_num._cand_en[1]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3, CandiTbl->_cand_ip_num._cand_en[2]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4, CandiTbl->_cand_ip_num._cand_en[3]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5, CandiTbl->_cand_ip_num._cand_en[4]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6, CandiTbl->_cand_ip_num._cand_en[5]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7, CandiTbl->_cand_ip_num._cand_en[6]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8, CandiTbl->_cand_ip_num._cand_en[7]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 9, 9, CandiTbl->_cand_ip_num._cand_en[8]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 10, 10, CandiTbl->_cand_ip_num._cand_en[9]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, CandiTbl->_cand_ip_num._upt_cand_en[0]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, CandiTbl->_cand_ip_num._upt_cand_en[1]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, CandiTbl->_cand_ip_num._upt_cand_en[2]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 14, 14, CandiTbl->_cand_ip_num._upt_cand_en[3]);

	// pi cand enable
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0, CandiTbl->_cand_pi_num._cand_en[0]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 1, 1, CandiTbl->_cand_pi_num._cand_en[1]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2, CandiTbl->_cand_pi_num._cand_en[2]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3, CandiTbl->_cand_pi_num._cand_en[3]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4, CandiTbl->_cand_pi_num._cand_en[4]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5, CandiTbl->_cand_pi_num._cand_en[5]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6, CandiTbl->_cand_pi_num._cand_en[6]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7, CandiTbl->_cand_pi_num._cand_en[7]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8, CandiTbl->_cand_pi_num._cand_en[8]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 9, 9, CandiTbl->_cand_pi_num._cand_en[9]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, CandiTbl->_cand_pi_num._upt_cand_en[0]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, CandiTbl->_cand_pi_num._upt_cand_en[1]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, CandiTbl->_cand_pi_num._upt_cand_en[2]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 13, 13, CandiTbl->_cand_pi_num._upt_cand_en[3]);

	// ip offset
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 0, 2, CandiTbl->_cand_ip_location._cand[0].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 3, 5, CandiTbl->_cand_ip_location._cand[0].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 6, 8, CandiTbl->_cand_ip_location._cand[1].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 9, 11, CandiTbl->_cand_ip_location._cand[1].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 12, 14, CandiTbl->_cand_ip_location._cand[2].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 15, 17, CandiTbl->_cand_ip_location._cand[2].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 18, 20, CandiTbl->_cand_ip_location._cand[3].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 21, 23, CandiTbl->_cand_ip_location._cand[3].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 24, 26, CandiTbl->_cand_ip_location._cand[4].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 27, 29, CandiTbl->_cand_ip_location._cand[4].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 0, 2, CandiTbl->_cand_ip_location._cand[5].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 3, 5, CandiTbl->_cand_ip_location._cand[5].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 6, 8, CandiTbl->_cand_ip_location._cand[6].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 9, 11, CandiTbl->_cand_ip_location._cand[6].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 12, 14, CandiTbl->_cand_ip_location._cand[7].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 15, 17, CandiTbl->_cand_ip_location._cand[7].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 18, 20, CandiTbl->_cand_ip_location._cand[8].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 21, 23, CandiTbl->_cand_ip_location._cand[8].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 24, 26, CandiTbl->_cand_ip_location._cand[9].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 27, 29, CandiTbl->_cand_ip_location._cand[9].offset_y);

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 0, 2, CandiTbl->_cand_ip_location._upt[0].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 3, 5, CandiTbl->_cand_ip_location._upt[0].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 6, 8, CandiTbl->_cand_ip_location._upt[1].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 9, 11, CandiTbl->_cand_ip_location._upt[1].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 12, 14, CandiTbl->_cand_ip_location._upt[2].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 15, 17, CandiTbl->_cand_ip_location._upt[2].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 18, 20, CandiTbl->_cand_ip_location._upt[3].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 21, 23, CandiTbl->_cand_ip_location._upt[3].offset_y);

	// pi offset
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 0, 2, CandiTbl->_cand_pi_location._cand[0].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 3, 5, CandiTbl->_cand_pi_location._cand[0].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 6, 8, CandiTbl->_cand_pi_location._cand[1].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 9, 11, CandiTbl->_cand_pi_location._cand[1].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 12, 14, CandiTbl->_cand_pi_location._cand[2].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 15, 17, CandiTbl->_cand_pi_location._cand[2].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 18, 20, CandiTbl->_cand_pi_location._cand[3].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 21, 23, CandiTbl->_cand_pi_location._cand[3].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 24, 26, CandiTbl->_cand_pi_location._cand[4].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 27, 29, CandiTbl->_cand_pi_location._cand[4].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 0, 2, CandiTbl->_cand_pi_location._cand[5].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 3, 5, CandiTbl->_cand_pi_location._cand[5].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 6, 8, CandiTbl->_cand_pi_location._cand[6].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 9, 11, CandiTbl->_cand_pi_location._cand[6].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 12, 14, CandiTbl->_cand_pi_location._cand[7].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 15, 17, CandiTbl->_cand_pi_location._cand[7].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 18, 20, CandiTbl->_cand_pi_location._cand[8].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 21, 23, CandiTbl->_cand_pi_location._cand[8].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 24, 26, CandiTbl->_cand_pi_location._cand[9].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 27, 29, CandiTbl->_cand_pi_location._cand[9].offset_y);

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 0, 2, CandiTbl->_cand_pi_location._upt[0].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 3, 5, CandiTbl->_cand_pi_location._upt[0].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 6, 8, CandiTbl->_cand_pi_location._upt[1].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 9, 11, CandiTbl->_cand_pi_location._upt[1].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 12, 14, CandiTbl->_cand_pi_location._upt[2].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 15, 17, CandiTbl->_cand_pi_location._upt[2].offset_y);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 18, 20, CandiTbl->_cand_pi_location._upt[3].offset_x);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 21, 23, CandiTbl->_cand_pi_location._upt[3].offset_y);

	// ip pnt
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_2C_reg, 0, 12, CandiTbl->_cand_ip_pnt._cand_gmv_pnt);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, CandiTbl->_cand_ip_pnt._cand_zmv_pnt);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_2C_reg, 16, 28, CandiTbl->_cand_ip_pnt._cand_ppi_pnt);

	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 0, 7, CandiTbl->_cand_ip_pnt._cand_pnt[0]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 8, 15, CandiTbl->_cand_ip_pnt._cand_pnt[1]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 16, 23, CandiTbl->_cand_ip_pnt._cand_pnt[2]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, CandiTbl->_cand_ip_pnt._cand_pnt[3]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 0, 7, CandiTbl->_cand_ip_pnt._cand_pnt[4]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 8, 15, CandiTbl->_cand_ip_pnt._cand_pnt[5]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, CandiTbl->_cand_ip_pnt._cand_pnt[6]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 24, 31, CandiTbl->_cand_ip_pnt._cand_pnt[7]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 0, 7, CandiTbl->_cand_ip_pnt._cand_pnt[8]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 8, 15, CandiTbl->_cand_ip_pnt._cand_pnt[9]);

	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 16, 23, CandiTbl->_cand_ip_pnt._cand_upt_pnt[0]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 24, 31, CandiTbl->_cand_ip_pnt._cand_upt_pnt[1]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 0, 7, CandiTbl->_cand_ip_pnt._cand_upt_pnt[2]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 8, 15, CandiTbl->_cand_ip_pnt._cand_upt_pnt[3]);

	// ip pnt meander
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_B0_reg, 0, 7, CandiTbl->_cand_ip_meander_pnt._cand_pnt[0]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_B0_reg, 8, 15, CandiTbl->_cand_ip_meander_pnt._cand_pnt[1]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_B0_reg, 16, 23, CandiTbl->_cand_ip_meander_pnt._cand_pnt[2]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_B0_reg, 24, 31, CandiTbl->_cand_ip_meander_pnt._cand_pnt[3]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_B4_reg, 0, 7, CandiTbl->_cand_ip_meander_pnt._cand_pnt[4]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_B4_reg, 8, 15, CandiTbl->_cand_ip_meander_pnt._cand_pnt[5]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_B4_reg, 16, 23, CandiTbl->_cand_ip_meander_pnt._cand_pnt[6]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_B4_reg, 24, 31, CandiTbl->_cand_ip_meander_pnt._cand_pnt[7]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_B8_reg, 0, 7, CandiTbl->_cand_ip_meander_pnt._cand_pnt[8]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_B8_reg, 8, 15, CandiTbl->_cand_ip_meander_pnt._cand_pnt[9]);

	// pi pnt
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_6C_reg, 0, 12, CandiTbl->_cand_pi_pnt._cand_gmv_pnt);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, CandiTbl->_cand_pi_pnt._cand_zmv_pnt);

	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 0, 7, CandiTbl->_cand_pi_pnt._cand_pnt[0]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 8, 15, CandiTbl->_cand_pi_pnt._cand_pnt[1]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 16, 23, CandiTbl->_cand_pi_pnt._cand_pnt[2]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, CandiTbl->_cand_pi_pnt._cand_pnt[3]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 0, 7, CandiTbl->_cand_pi_pnt._cand_pnt[4]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 8, 15, CandiTbl->_cand_pi_pnt._cand_pnt[5]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, CandiTbl->_cand_pi_pnt._cand_pnt[6]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 24, 31, CandiTbl->_cand_pi_pnt._cand_pnt[7]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 0, 7, CandiTbl->_cand_pi_pnt._cand_pnt[8]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 8, 15, CandiTbl->_cand_pi_pnt._cand_pnt[9]);

	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 16, 23, CandiTbl->_cand_pi_pnt._cand_upt_pnt[0]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 24, 31, CandiTbl->_cand_pi_pnt._cand_upt_pnt[1]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 0, 7, CandiTbl->_cand_pi_pnt._cand_upt_pnt[2]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 8, 15, CandiTbl->_cand_pi_pnt._cand_upt_pnt[3]);

	// pi pnt meander
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_BC_reg, 0, 7, CandiTbl->_cand_pi_meander_pnt._cand_pnt[0]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_BC_reg, 8, 15, CandiTbl->_cand_pi_meander_pnt._cand_pnt[1]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_BC_reg, 16, 23, CandiTbl->_cand_pi_meander_pnt._cand_pnt[2]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_BC_reg, 24, 31, CandiTbl->_cand_pi_meander_pnt._cand_pnt[3]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_C0_reg, 0, 7, CandiTbl->_cand_pi_meander_pnt._cand_pnt[4]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_C0_reg, 8, 15, CandiTbl->_cand_pi_meander_pnt._cand_pnt[5]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_C0_reg, 16, 23, CandiTbl->_cand_pi_meander_pnt._cand_pnt[6]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_C0_reg, 24, 31, CandiTbl->_cand_pi_meander_pnt._cand_pnt[7]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_C4_reg, 0, 7, CandiTbl->_cand_pi_meander_pnt._cand_pnt[8]);
	WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_C4_reg, 8, 15, CandiTbl->_cand_pi_meander_pnt._cand_pnt[9]);
#endif
}

#endif
