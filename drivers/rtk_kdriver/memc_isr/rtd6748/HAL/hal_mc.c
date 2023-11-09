/**
 * @file hal_mc.c
 * @brief This file is for mc register setting
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
#include "memc_isr/Common/memc_type.h"
#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_reg_def.h"
#include <tvscalercontrol/panel/panelapi.h>
#include "memc_isr/Platform/memc_change_size.h"

extern unsigned int Get_DISPLAY_PANEL_TYPE(void);
extern unsigned char Scaler_MEMC_GetPanelSizeByDisp(void);

/**
 * @brief This function set kmc top in interrupt select
 * @param [in] 7 for input
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetInINTSel(unsigned int u32Sel)
{
	//reg_kmc_in_int_sel
	WriteRegister(KMC_TOP_kmc_top_18_reg,0,2,u32Sel);
}

/**
 * @brief This function set kmc top in interrupt source
 * @param [in] 1
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetInINTSource(unsigned int u32Sor)
{
	//reg_kmc_int_source_sel
	WriteRegister(KMC_TOP_kmc_top_18_reg,19,19,u32Sor);
}

/**
 * @brief This function set kmc top in interrupt horizantal number
 * @param [in] H number
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetInINTHNum(unsigned int u32Hnum)
{
	//reg_kmc_in_vtotal_int
	WriteRegister(KMC_TOP_kmc_top_18_reg,7,18,u32Hnum);
}


/**
 * @brief This function set kmc top in interrupt enable
 * @param [in] u32IntType: 0 for H sync, 1 for V sync, 2 for Abnormal
 * @param [in] bEnable
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetInINTEnable(unsigned int u32IntType, BOOL bEnable)
{
	unsigned int u32InINT_en = 0;
	ReadRegister(KMC_TOP_kmc_top_18_reg,24,27, &u32InINT_en);

	if(bEnable)
	{
		u32InINT_en = u32InINT_en | (1 << u32IntType);
	}
	else
	{
		u32InINT_en = u32InINT_en & (~(1 << u32IntType));
	}

	//reg_kmc_int_en
	WriteRegister(KMC_TOP_kmc_top_18_reg,24,27,u32InINT_en);
}

BOOL HAL_KMC_TOP_GetInINTEnable(unsigned int u32IntType)
{
       unsigned int v = 0;

       ReadRegister(KMC_TOP_kmc_top_18_reg,24,27,&v);
       return (v >> u32IntType) & 0x1;
}

/**
 * @brief This function set kmc top in interrupt clear
 * @param [in] eIntType: 0 for H sync, 1 for V sync, 2 for Abnormal
 * @param [in] bWclr
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetInINTWclr(INT_TYPE eIntType, BOOL bWclr)
{
	unsigned int u32InINT_Clear = 0;
	ReadRegister(KMC_TOP_kmc_top_18_reg,20,23, &u32InINT_Clear);

	if(bWclr)
		u32InINT_Clear = u32InINT_Clear | ( 1 << (unsigned int)eIntType);
	else
		u32InINT_Clear = u32InINT_Clear & (~( 1 << (unsigned int)eIntType));

	//reg_kmc_int_en
	WriteRegister(KMC_TOP_kmc_top_18_reg,20,23, u32InINT_Clear);
}

/**
 * @brief This function get kmc top in interrupt status
 * @retval unsigned int
*/
unsigned int HAL_KMC_TOP_GetInINTStatus(VOID)
{
	unsigned int u32InINT_Sts = 0;
	//ReadRegister(KMC_TOP_kmc_top_F8_reg,24,27, &u32InINT_Sts);
	ReadRegister(KMC_TOP_kmc_top_F8_reg, 27, 30, &u32InINT_Sts);
	return u32InINT_Sts;
}

/**
 * @brief This function set kmc top pattern size
 * @param [in] TIMING_PARA_ST stInputTiming
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetPatternSize(TIMING_PARA_ST stInputTiming)
{
	//reg_top_patt_htotal
	WriteRegister(KMC_TOP_kmc_top_00_reg,0,15,stInputTiming.u32HTotal/2);
	//reg_top_patt_vtotal
	WriteRegister(KMC_TOP_kmc_top_00_reg,16,31,stInputTiming.u32VTotal/2);
	//reg_top_patt_hact
	WriteRegister(KMC_TOP_kmc_top_04_reg,0,15,stInputTiming.u32HActive/2);
	//reg_top_patt_vact
	WriteRegister(KMC_TOP_kmc_top_04_reg,16,31,stInputTiming.u32VActive/2);
}

/**
 * @brief This function set kmc knr 422 to 444 enable
 * @param [in] unsigned int bEnable
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnr422to444Enable(BOOL bEnable)
{
	//kmc_top__reg_knr_422to444
	WriteRegister(KMC_TOP_kmc_top_34_reg,12,12,(unsigned int)bEnable);
}

/**
 * @brief This function set kmc knr csc enable
 * @param [in] unsigned int bEnable
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnrCSCEnable(BOOL bEnable)
{
	//kmc_top__reg_knr_convert_on
	WriteRegister(KMC_TOP_kmc_top_58_reg,24,24,(unsigned int)bEnable);
}

/**
 * @brief This function set kmc knr csc mode
 * @param [in] unsigned int bEnable
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnrCSCMode(unsigned int u32Mode)
{
	//kmc_top__reg_knr_convert_mode
	WriteRegister(KMC_TOP_kmc_top_58_reg,25,28,u32Mode);
}

/**
 * @brief This function set kmc top knr trig gen mode
 * @param [in] unsigned int u32knr_trig_gen_mode
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnrTrigGenMode(unsigned int u32knr_trig_gen_mode)
{
	//reg_knr_row_trig_dly
	WriteRegister(KMC_TOP_kmc_top_30_reg,23,23,u32knr_trig_gen_mode);
}

/**
 * @brief This function set kmc top knr row trig delay
 * @param [in] unsigned int u32knr_row_trig_dly
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnrRowTrigDelay(unsigned int u32knr_row_trig_dly)
{
	//reg_knr_row_trig_dly
	WriteRegister(KMC_TOP_kmc_top_30_reg,0,11,u32knr_row_trig_dly);
}

/**
 * @brief This function set kmc top knr half v active
 * @param [in] unsigned int u32knr_half_vactive
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnrHalfVactive(unsigned int u32knr_half_vactive)
{
	//reg_knr_half_v_active
	WriteRegister(KMC_TOP_kmc_top_30_reg,12,22,u32knr_half_vactive);
}

/**
 * @brief This function set kmc top knr h active
 * @param [in] unsigned int u32knr_hactive
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetKnrHactive(unsigned int u32knr_hactive)
{
	//reg_knr_h_active
	WriteRegister(KMC_TOP_kmc_top_34_reg,0,11,u32knr_hactive);
}

/**
 * @brief This function set kmc top ipmc hde center value
 * @param [in] unsigned int u32ipmc_hde_center_value
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetIPMCHDECenterValue(unsigned int u32ipmc_hde_center_value)
{
	//reg_kmc_ipmc_hde_center_value
	WriteRegister(KMC_TOP_kmc_top_3C_reg,0,9,u32ipmc_hde_center_value);
}

/**
 * @brief This function set kmc top ipmc mode
 * @param [in] unsigned int u32ipmc_mode
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetIPMCMode(unsigned int u32ipmc_mode)
{
	//reg_kmc_ipmc_mode
	WriteRegister(KMC_TOP_kmc_top_3C_reg,20,23,u32ipmc_mode);
}

/**
 * @brief This function set kmc top ipmc hde active
 * @param [in] unsigned int u32ipmc_hde_active
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetIPMCHDEActive(unsigned int u32ipmc_hde_active)
{
	//reg_kmc_ipmc_hde_active
	WriteRegister(KMC_TOP_kmc_top_40_reg,0,10,u32ipmc_hde_active);
}

/**
 * @brief This function set kmc top ipmc v active
 * @param [in] unsigned int u32ipmc_v_active
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetIPMCVactive(unsigned int u32ipmc_v_active)
{
	//reg_kmc_ipmc_v_active
	WriteRegister(KMC_TOP_kmc_top_40_reg,11,22,u32ipmc_v_active);
}

/**
 * @brief This function set kmc top data mapping
 * @param [in] unsigned int u32Data_Mapping
 * @retval VOID
*/
VOID HAL_KMC_TOP_SetDataMapping(unsigned int u32Data_Mapping)
{
	//reg_knr_convert_map
	WriteRegister(KMC_TOP_kmc_top_58_reg,29,31,(unsigned int)u32Data_Mapping);
}

/**
 * @brief This function get kmc top data mapping
 * @param [in] VOID
 * @retval data mapping
*/
unsigned int HAL_KMC_TOP_GetDataMapping(VOID)
{
	unsigned int u32Data_Mapping = 0;
	//reg_knr_convert_map
	ReadRegister(KMC_TOP_kmc_top_58_reg,29,31,&u32Data_Mapping);
	return u32Data_Mapping;
}

/**
 * @brief This function set lbmc fhd mode
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetLBMCFHDMode(BOOL bEnable)
{
	WriteRegister(LBMC_LBMC_20_reg,30,30, (unsigned int)bEnable);
}

/**
 * @brief This function set lbmc H/V active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_MC_SetLBMCActive(unsigned int u32Hactive, unsigned int u32Vactive)
{
	//reg_lbmc_h_act[11:0]
	WriteRegister(LBMC_LBMC_20_reg,0,12,u32Hactive);
	//reg_lbmc_v_act[11:0]
	WriteRegister(LBMC_LBMC_20_reg,16,28,u32Vactive);
}

/**
 * @brief This function set lbmc 3d mode
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetLBMC3DMode(BOOL bEnable)
{
	//reg_lbmc_3dmode
	WriteRegister(LBMC_LBMC_24_reg,3,3,(unsigned int)bEnable);
}

/**
 * @brief This function set lbmc pr mode
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetLBMCPRMode(BOOL bEnable)
{
	//lbmc_pr_mod
	WriteRegister(LBMC_LBMC_20_reg,13,13,(unsigned int)bEnable);
}

/**
 * @brief This function set lbmc pc mode
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetLBMCPCMode(BOOL bEnable)
{
	//reg_lbmcpc_mode
	WriteRegister(LBMC_LBMC_24_reg,4,4,(unsigned int)bEnable);
}

/**
 * @brief This function set lbmc vflip enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetLBMCVflipEnable(BOOL bEnable)
{
	//reg_lbmc_vflip
	WriteRegister(LBMC_LBMC_24_reg,6,6,(unsigned int)bEnable);
}

/**
 * @brief This function set mc h half size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetMCHhalfsize(BOOL bEnable)
{
	//reg_mc_h_half_size
	WriteRegister(MC_MC_30_reg,0,0,(unsigned int)bEnable);
}

/**
 * @brief This function set mc v half size
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetMCVhalfsize(BOOL bEnable)
{
	//reg_mc_v_half_size
	WriteRegister(MC_MC_30_reg,1,1,(unsigned int) bEnable);
}

/**
 * @brief This function set mc H/V active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_MC_SetMCActive(unsigned int u32Hactive, unsigned int u32Vactive)
{
	//reg_mc_h_act[11:0]
	WriteRegister(MC_MC_34_reg,0,11,u32Hactive);
	//reg_mc_v_act[11:0]
	WriteRegister(MC_MC_34_reg,16,27,u32Vactive);
}

/**
 * @brief This function set mc block size
 * @param [in] unsigned int u32Block_Size
 * @retval VOID
*/
VOID HAL_MC_SetMCBlockSize(unsigned int u32Block_Size)
{
	//reg_mc_blk_size[3:2]
	WriteRegister(MC_MC_38_reg,0,3,u32Block_Size);
}

/**
 * @brief This function get mc block size
 * @param [in] VOID
 * @retval unsigned int u32Block_Size
*/
unsigned int HAL_MC_GetMCBlockSize(VOID)
{
	unsigned int u32Block_Size = 0;
	//reg_mc_blk_size[3:2]
	ReadRegister(MC_MC_38_reg,0,3,&u32Block_Size);
	return u32Block_Size;
}

/**
 * @brief This function set mc mv scaler
 * @param [in] unsigned int u32MV_Scaler
 * @retval VOID
*/
VOID HAL_MC_SetMCMVScaler(unsigned int u32MV_Scaler)
{
	//reg_mc_mv_scale[1:0]
	WriteRegister(MC_MC_38_reg,4,7,u32MV_Scaler);
}

/**
 * @brief This function set mc block number
 * @param [in] unsigned int u32Block_Num
 * @retval VOID
*/
VOID HAL_MC_SetMCBlockNum(unsigned int u32Block_Num)
{
	//reg_mc_blk_num[9:0]
	WriteRegister(MC_MC_3C_reg,0,9,u32Block_Num);
#if 1
	WriteRegister(MC_MC_2C_reg,3,11,u32Block_Num);	
	WriteRegister(MC_MC_24_reg,9,17,u32Block_Num);		
#endif
}

/**
 * @brief This function set mc row number
 * @param [in] unsigned int u32Block_Num
 * @retval VOID
*/
VOID HAL_MC_SetMCRowNum(unsigned int u32Row_Num)
{
	//reg_mc_row_num[9:0]
	WriteRegister(MC_MC_3C_reg,16,25,u32Row_Num);
#if 1
	WriteRegister(MC_MC_2C_reg,12,20,u32Row_Num);
	WriteRegister(MC_MC_24_reg,18,26,u32Row_Num);		
#endif
}

#if 1
/**
 * @brief This function set mc mv pattern block size
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCMVPatBlkSize(unsigned int u32Val)
{
	WriteRegister(MC_MC_28_reg,0,0,u32Val);
}

/**
 * @brief This function set mc mv pattern Row size
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCMVPatRowSize(unsigned int u32Val)
{
	WriteRegister(MC_MC_28_reg,1,1,u32Val);
}

/**
 * @brief This function set mc dehalo half v timing
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCDhHalfVTiming(unsigned int u32Val)
{
	WriteRegister(MC_MC_28_reg,29,29,u32Val);
}

/**
 * @brief This function set mc dehalo Row size
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCDhRowSize(unsigned int u32Val)
{
	WriteRegister(MC_MC_2C_reg,0,1,u32Val);
}

/**
 * @brief This function set mc mv read interval
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCMVRDIntval(unsigned int u32Val)
{
	WriteRegister(MC_MC_2C_reg,2,2,u32Val);
}
#endif

/**
 * @brief This function set mc blend mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_MC_SetMCBlendMode(unsigned int u32Mode)
{
#if (IC_K6LP || IC_K8LP) // merlin4_blend_mode
	if(u32Mode == 1){
		WriteRegister(LBMC_LBMC_24_reg, 4, 4, 0x1); //0xb8099924[4]
		WriteRegister(LBMC_LBMC_24_reg, 13, 13, 0x0); //0xb8099924[13] //IZ mode
		WriteRegister(LBMC_LBMC_24_reg, 10, 10, 0); //0xb8099924[10]
		WriteRegister(LBMC_LBMC_EC_reg, 1, 1, 1); // lbmc_db_apply
		rtd_pr_memc_notice("[HAL_MC_SetMCBlendMode][Repeat mode][IZ][9924,%x,][90k=%x]\n\r", rtd_inl(LBMC_LBMC_24_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
	}
	else if(u32Mode == 2){
		WriteRegister(LBMC_LBMC_24_reg, 4, 4, 0x1); //0xb8099924[4]
		WriteRegister(LBMC_LBMC_24_reg, 13, 13, 0x1); //0xb8099924[13] //PZ mode
		WriteRegister(LBMC_LBMC_24_reg, 10, 10, 0); //0xb8099924[10]
		WriteRegister(LBMC_LBMC_EC_reg, 1, 1, 1); // lbmc_db_apply
		rtd_pr_memc_notice("[HAL_MC_SetMCBlendMode][Repeat mode][PZ][9924,%x,][90k=%x]\n\r", rtd_inl(LBMC_LBMC_24_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
	}
	else{
		WriteRegister(LBMC_LBMC_24_reg, 4, 4, 0x0); //0xb8099924[4]
		WriteRegister(LBMC_LBMC_24_reg, 13, 13, 0x0); //0xb8099924[13] //reset
		WriteRegister(LBMC_LBMC_24_reg, 10, 10, 1); //0xb8099924[10]
		WriteRegister(LBMC_LBMC_EC_reg, 1, 1, 1); // lbmc_db_apply
		rtd_pr_memc_notice("[HAL_MC_SetMCBlendMode][Blending mode][9924,%x,][90k=%x]\n\r", rtd_inl(LBMC_LBMC_24_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
	}
#else
	WriteRegister(MC_MC_B0_reg,0,2,u32Mode);
#endif
}

/**
 * @brief This function set mc scene change enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetMCSceneChangeEnable(BOOL bEnable)
{
	//reg_mc_scene_change_fb
	//rtd_pr_memc_notice("HAL_MC_SetMCSceneChangeEnable=%d\n",bEnable);
#if 1
	WriteRegister(MC_MC_30_reg,8,8,(unsigned int)bEnable);
#else
	WriteRegister(MC_MC_30_reg,10,10,(unsigned int)bEnable);
#endif
}

/**
 * @brief This function set mc scene change enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetMCSceneChange_fb(BOOL bEnable)
{
	//reg_mc_scene_change_fb
	//rtd_pr_memc_notice("HAL_MC_SetMCSceneChange_fb=%d\n",bEnable);
#if 1
	WriteRegister(MC_MC_30_reg,10,10,(unsigned int)bEnable);
#else
	WriteRegister(MC_MC_30_reg,10,10,(unsigned int)bEnable);
#endif
}

/**
 * @brief This function set MC PT Zero type1
 * @param [in] u32Type: 0: iz; 1: pz; 2: avg; 3: phase blend; 4: I/P by phase; 5: med3
 * @retval VOID
*/
VOID HAL_MC_SetPTZeroType1(unsigned int u32Type)
{
	//reg_mc_pt_z_type1
	WriteRegister(MC_MC_A4_reg,8,10,u32Type);
}

/**
 * @brief This function set MC PT Zero type2
 * @param [in] u32Type: 0: iz; 1: pz; 2: avg; 3: phase blend; 4: I/P by phase; 5: med3
 * @retval VOID
*/
VOID HAL_MC_SetPTZeroType2(unsigned int u32Type)
{
	//reg_mc_pt_z_type2
	WriteRegister(MC_MC_A4_reg,12,14,u32Type);
}

/**
 * @brief This function set mc logo mode
 * @param [in] unsigned int u32LogoMode
 * @retval VOID
*/
VOID HAL_MC_SetLogoMode(unsigned int u32LogoMode)
{
	//reg_mc_logo_en
	WriteRegister(MC2_MC2_20_reg,0,1,u32LogoMode);
}

/**
 * @brief This function set mc block logo h size
 * @param [in] unsigned int u32Hsize
 * @retval VOID
*/
VOID HAL_MC_SetBlockLogoHSize(unsigned int u32Hsize)
{
	//reg_mc_blk_logo_h_size
	WriteRegister(MC2_MC2_20_reg,2,2,u32Hsize);
}

/**
 * @brief This function set mc pixel logo h size
 * @param [in] unsigned int u32Hsize
 * @retval VOID
*/
VOID HAL_MC_SetPixelLogoHSize(unsigned int u32Hsize)
{
	//reg_mc_pxl_logo_h_size
	WriteRegister(MC2_MC2_20_reg,4,4,u32Hsize);
}

/**
 * @brief This function set mc block logo v size
 * @param [in] unsigned int u32Vsize
 * @retval VOID
*/
VOID HAL_MC_SetBlockLogoVSize(unsigned int u32Vsize)
{
	//reg_mc_blk_logo_v_size
	WriteRegister(MC2_MC2_20_reg,3,3,u32Vsize);
}

/**
 * @brief This function set mc pixel logo v size
 * @param [in] unsigned int u32Vsize
 * @retval VOID
*/
VOID HAL_MC_SetPixelLogoVSize(unsigned int u32Vsize)
{
	//reg_mc_pxl_logo_v_size
	WriteRegister(MC2_MC2_20_reg,5,5,u32Vsize);
}

/**
 * @brief This function set kphase 3d fs enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetKphase3DFSEnable(BOOL bEnable)
{
	//reg_kphase_in_3d_fs_enable
	WriteRegister(KPHASE_kphase_00_reg,7,7,(unsigned int)bEnable);
	//reg_kphase_in_lr_ctl_for_index
	WriteRegister(KPHASE_kphase_10_reg,26,26,(unsigned int)bEnable);
}

/**
 * @brief This function set kphase 3d enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetKphase3DEnable(BOOL bEnable)
{
	//reg_kphase_out_3d_enable
	WriteRegister(KPHASE_kphase_0C_reg,31,31,(unsigned int)bEnable);
}

/**
 * @brief This function set 3d mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_MC_SetKphase3DMode(unsigned int u32Mode)
{
	//reg_kphase_3d_mode
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KME_TOP__kphase_3d_mode_ADDR no mat,FRC_TOP__KME_TOP__kphase_3d_mode_BITSTART no mat,FRC_TOP__KME_TOP__kphase_3d_mode_BITEND no mat,u32Mode);
}

/**
 * @brief This function set me1 index bsize
 * @param [in] unsigned int u32Bsize
 * @retval VOID
*/
VOID HAL_MC_SetKphaseME1IndexBsize(unsigned int u32Bsize)
{
	WriteRegister(KPHASE_kphase_10_reg,8,11,u32Bsize);
}

/**
 * @brief This function set me2 index bsize
 * @param [in] unsigned int u32Bsize
 * @retval VOID
*/
VOID HAL_MC_SetKphaseME2IndexBsize(unsigned int u32Bsize)
{
	WriteRegister(KPHASE_kphase_78_reg,11,14,u32Bsize);
}

/**
 * @brief This function set mc index bsize
 * @param [in] unsigned int u32Bsize
 * @retval VOID
*/
VOID HAL_MC_SetKphaseMCIndexBsize(unsigned int u32Bsize)
{
	//reg_kphase_in_mc_index_bsize
	WriteRegister(KPHASE_kphase_10_reg,12,15,u32Bsize);
}

/**
 * @brief This function set decode data clear enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetKphaseDecDataClrEnable(BOOL bEnable)
{
	//reg_kphase_dec_data_vs_clr_en
	WriteRegister(KPHASE_kphase_78_reg,30,30,(unsigned int)bEnable);
}

/**
 * @brief This function set kphase force mc lf index
 * @param [in] unsigned int u32Idx
 * @retval VOID
*/
VOID HAL_MC_SetKphaseForceMCLFIdx(unsigned int u32Idx)
{
	//reg_kphase_force_mc_lf_index
	WriteRegister(KPHASE_kphase_00_reg,20,23,u32Idx);
}

/**
 * @brief This function set kphase force mc lf index enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetKphaseForceMCLFIdxEnable(unsigned int u32Idx)
{
	//reg_kphase_force_mc_lf_index_en
	WriteRegister(KPHASE_kphase_00_reg,24,24,u32Idx);
}

/**
 * @brief This function set kphase force mc hf index
 * @param [in] unsigned int u32Idx
 * @retval VOID
*/
VOID HAL_MC_SetKphaseForceMCHFIdx(unsigned int u32Idx)
{
	//reg_kphase_force_mc_hf_index
	WriteRegister(KPHASE_kphase_00_reg,25,26,u32Idx);
}

/**
 * @brief This function set kphase force mc hf index enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetKphaseForceMCHFIdxEnable(unsigned int u32Idx)
{
	//reg_kphase_force_mc_hf_index_en
	WriteRegister(KPHASE_kphase_00_reg,27,27,u32Idx);
}

/**
 * @brief This function set demo window Enable
 * @param [in] unsigned char ubyWinID
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_MC_SetDemoWindowEnable(unsigned char ubyWinID, BOOL bEnable)
{
	unsigned int u32dw_en = 0;
	ReadRegister(MC_MC_60_reg,0,7, &u32dw_en);
	if(bEnable)
	{
		u32dw_en = u32dw_en | (1 << ubyWinID);
	}
	else
	{
		u32dw_en = u32dw_en & (~(1 << ubyWinID));
	}
	WriteRegister(MC_MC_60_reg,0,7, u32dw_en);
}

/**
 * @brief This function get demo window Enable
 * @param [in] unsigned char ubyWinID
 * @retval BOOL bEnable
*/
BOOL HAL_MC_GetDemoWindowEnable(unsigned char ubyWinID)
{
	unsigned int u32dw_en = 0;
	ReadRegister(MC_MC_60_reg,0,7, &u32dw_en);
	if(u32dw_en & (1 << ubyWinID))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
 * @brief This function set demo window mode
 * @param [in] BOOL bMode: 0: inside MEMC OFF; 1: outside MEMC OFF
 * @retval VOID
*/
VOID HAL_MC_SetDemoWindowMode(BOOL bMode)
{
	if(bMode == FALSE)
	{
		WriteRegister(MC_MC_60_reg,8,8, 0);
	}
	else
	{
		WriteRegister(MC_MC_60_reg,8,8, 1);
	}
}

/**
 * @brief This function get demo window mode
 * @retval BOOL bMode: 0: inside MEMC OFF; 1: outside MEMC OFF
*/
BOOL HAL_MC_GetDemoWindowMode(VOID)
{
	unsigned int u32dw_mode = 0;
	ReadRegister(MC_MC_60_reg,8,8, &u32dw_mode);

	if(u32dw_mode == 0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/**
 * @brief This function set demo window
 * @param [in] unsigned char ubyWinID
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @retval VOID
*/
VOID HAL_MC_SetDemoWindowSettings(unsigned char ubyWinID, unsigned int u32Left, unsigned int u32Right, unsigned int u32Top, unsigned int u32Bottom)
{
	if(ubyWinID == 0)
	{
		WriteRegister(MC_MC_68_reg,0,11, u32Left);
		WriteRegister(MC_MC_68_reg,16,27, u32Right);
		WriteRegister(MC_MC_64_reg,0,11, u32Top);
		WriteRegister(MC_MC_64_reg,16,27, u32Bottom);
	}
	else if(ubyWinID == 1)
	{
		WriteRegister(MC_MC_70_reg,0,11, u32Left);
		WriteRegister(MC_MC_70_reg,16,27, u32Right);
		WriteRegister(MC_MC_6C_reg,0,11, u32Top);
		WriteRegister(MC_MC_6C_reg,16,27, u32Bottom);
	}
	else if(ubyWinID == 2)
	{
		WriteRegister(MC_MC_78_reg,0,11, u32Left);
		WriteRegister(MC_MC_78_reg,16,27, u32Right);
		WriteRegister(MC_MC_74_reg,0,11, u32Top);
		WriteRegister(MC_MC_74_reg,16,27, u32Bottom);
	}
	else if(ubyWinID == 3)
	{
		WriteRegister(MC_MC_80_reg,0,11, u32Left);
		WriteRegister(MC_MC_80_reg,16,27, u32Right);
		WriteRegister(MC_MC_7C_reg,0,11, u32Top);
		WriteRegister(MC_MC_7C_reg,16,27, u32Bottom);
	}
	else if(ubyWinID == 4)
	{
		WriteRegister(MC_MC_88_reg,0,11, u32Left);
		WriteRegister(MC_MC_88_reg,16,27, u32Right);
		WriteRegister(MC_MC_84_reg,0,11, u32Top);
		WriteRegister(MC_MC_84_reg,16,27, u32Bottom);
	}
	else if(ubyWinID == 5)
	{
		WriteRegister(MC_MC_90_reg,0,11, u32Left);
		WriteRegister(MC_MC_90_reg,16,27, u32Right);
		WriteRegister(MC_MC_8C_reg,0,11, u32Top);
		WriteRegister(MC_MC_8C_reg,16,27, u32Bottom);
	}
	else if(ubyWinID == 6)
	{
		WriteRegister(MC_MC_98_reg,0,11, u32Left);
		WriteRegister(MC_MC_98_reg,16,27, u32Right);
		WriteRegister(MC_MC_94_reg,0,11, u32Top);
		WriteRegister(MC_MC_94_reg,16,27, u32Bottom);
	}
	else if(ubyWinID == 7)
	{
		WriteRegister(MC_MC_A0_reg,0,11, u32Left);
		WriteRegister(MC_MC_A0_reg,16,27, u32Right);
		WriteRegister(MC_MC_9C_reg,0,11, u32Top);
		WriteRegister(MC_MC_9C_reg,16,27, u32Bottom);
	}
}

/**
 * @brief This function get demo window
 * @param [in] unsigned char ubyWinID
 * @param [out] unsigned int u32Left
 * @param [out] unsigned int u32Right
 * @param [out] unsigned int u32Top
 * @param [out] unsigned int u32Bottom
 * @retval VOID
*/
VOID HAL_MC_GetDemoWindowSettings(unsigned char ubyWinID, unsigned int *pu32Left, unsigned int *pu32Right, unsigned int *pu32Top, unsigned int *pu32Bottom)
{
	if(ubyWinID == 0)
	{
		ReadRegister(MC_MC_68_reg,0,11, pu32Left);
		ReadRegister(MC_MC_68_reg,16,27, pu32Right);
		ReadRegister(MC_MC_64_reg,0,11, pu32Top);
		ReadRegister(MC_MC_64_reg,16,27, pu32Bottom);
	}
	else if(ubyWinID == 1)
	{
		ReadRegister(MC_MC_70_reg,0,11, pu32Left);
		ReadRegister(MC_MC_70_reg,16,27, pu32Right);
		ReadRegister(MC_MC_6C_reg,0,11, pu32Top);
		ReadRegister(MC_MC_6C_reg,16,27, pu32Bottom);
	}
	else if(ubyWinID == 2)
	{
		ReadRegister(MC_MC_78_reg,0,11, pu32Left);
		ReadRegister(MC_MC_78_reg,16,27, pu32Right);
		ReadRegister(MC_MC_74_reg,0,11, pu32Top);
		ReadRegister(MC_MC_74_reg,16,27, pu32Bottom);
	}
	else if(ubyWinID == 3)
	{
		ReadRegister(MC_MC_80_reg,0,11, pu32Left);
		ReadRegister(MC_MC_80_reg,16,27, pu32Right);
		ReadRegister(MC_MC_7C_reg,0,11, pu32Top);
		ReadRegister(MC_MC_7C_reg,16,27, pu32Bottom);
	}
	else if(ubyWinID == 4)
	{
		ReadRegister(MC_MC_88_reg,0,11, pu32Left);
		ReadRegister(MC_MC_88_reg,16,27, pu32Right);
		ReadRegister(MC_MC_84_reg,0,11, pu32Top);
		ReadRegister(MC_MC_84_reg,16,27, pu32Bottom);
	}
	else if(ubyWinID == 5)
	{
		ReadRegister(MC_MC_90_reg,0,11, pu32Left);
		ReadRegister(MC_MC_90_reg,16,27, pu32Right);
		ReadRegister(MC_MC_8C_reg,0,11, pu32Top);
		ReadRegister(MC_MC_8C_reg,16,27, pu32Bottom);
	}
	else if(ubyWinID == 6)
	{
		ReadRegister(MC_MC_98_reg,0,11, pu32Left);
		ReadRegister(MC_MC_98_reg,16,27, pu32Right);
		ReadRegister(MC_MC_94_reg,0,11, pu32Top);
		ReadRegister(MC_MC_94_reg,16,27, pu32Bottom);
	}
	else if(ubyWinID == 7)
	{
		ReadRegister(MC_MC_A0_reg,0,11, pu32Left);
		ReadRegister(MC_MC_A0_reg,16,27, pu32Right);
		ReadRegister(MC_MC_9C_reg,0,11, pu32Top);
		ReadRegister(MC_MC_9C_reg,16,27, pu32Bottom);
	}
}

/**
 * @brief This function set lbmc normal lf prefetch number
 * @param [in] BOOL bType: 0 for i, 1 for p
 * @param [in] ubyIdx(0~8)
 * @param [in] ubyPreFetch_Num
 * @retval VOID
*/
VOID HAL_MC_SetLBMCNormalLFPreFetchNum(BOOL bType, unsigned char ubyIdx, unsigned char ubyPreFetch_Num)
{
	if(!bType)	// i
	{
		switch(ubyIdx)
		{
			case 0:
				//nor_ilf_idx0_prefetch_num
				WriteRegister(LBMC_LBMC_2C_reg,0,6,ubyPreFetch_Num);
				break;
			case 1:
				//nor_ilf_idx1_prefetch_num
				WriteRegister(LBMC_LBMC_2C_reg,7,13,ubyPreFetch_Num);
				break;
			case 2:
				//nor_ilf_idx2_prefetch_num
				WriteRegister(LBMC_LBMC_2C_reg,14,20,ubyPreFetch_Num);
				break;
			case 3:
				//nor_ilf_idx3_prefetch_num
				WriteRegister(LBMC_LBMC_2C_reg,21,27,ubyPreFetch_Num);
				break;
			case 4:
				//nor_ilf_idx4_prefetch_num
				WriteRegister(LBMC_LBMC_30_reg,0,6,ubyPreFetch_Num);
				break;
			case 5:
				//nor_ilf_idx5_prefetch_num
				WriteRegister(LBMC_LBMC_30_reg,7,13,ubyPreFetch_Num);
				break;
			case 6:
				//nor_ilf_idx6_prefetch_num
				WriteRegister(LBMC_LBMC_30_reg,14,20,ubyPreFetch_Num);
				break;
			case 7:
				//nor_ilf_idx7_prefetch_num
				WriteRegister(LBMC_LBMC_30_reg,21,27,ubyPreFetch_Num);
				break;
			case 8:
				//nor_ilf_idx8_prefetch_num
				WriteRegister(LBMC_LBMC_34_reg,0,6,ubyPreFetch_Num);
				break;
			default:
				break;
		}
	}
	else	//	p
	{
		switch(ubyIdx)
		{
			case 0:
				//nor_plf_idx0_prefetch_num
				WriteRegister(LBMC_LBMC_38_reg,0,6,ubyPreFetch_Num);
				break;
			case 1:
				//nor_plf_idx1_prefetch_num
				WriteRegister(LBMC_LBMC_38_reg,7,13,ubyPreFetch_Num);
				break;
			case 2:
				//nor_plf_idx2_prefetch_num
				WriteRegister(LBMC_LBMC_38_reg,14,20,ubyPreFetch_Num);
				break;
			case 3:
				//nor_plf_idx3_prefetch_num
				WriteRegister(LBMC_LBMC_38_reg,21,27,ubyPreFetch_Num);
				break;
			case 4:
				//nor_plf_idx4_prefetch_num
				WriteRegister(LBMC_LBMC_3C_reg,0,6,ubyPreFetch_Num);
				break;
			case 5:
				//nor_plf_idx5_prefetch_num
				WriteRegister(LBMC_LBMC_3C_reg,7,13,ubyPreFetch_Num);
				break;
			case 6:
				//nor_plf_idx6_prefetch_num
				WriteRegister(LBMC_LBMC_3C_reg,14,20,ubyPreFetch_Num);
				break;
			case 7:
				//nor_plf_idx7_prefetch_num
				WriteRegister(LBMC_LBMC_3C_reg,21,27,ubyPreFetch_Num);
				break;
			case 8:
				//nor_plf_idx8_prefetch_num
				WriteRegister(LBMC_LBMC_40_reg,0,6,ubyPreFetch_Num);
				break;
			default:
				break;
		}
	}
}

/**
 * @brief This function set lbmc normal hf prefetch number
 * @param [in] BOOL bType: 0 for i, 1 for p
 * @param [in] ubyIdx(0~2)
 * @param [in] ubyPreFetch_Num
 * @retval VOID
*/
VOID HAL_MC_SetLBMCNormalHFPreFetchNum(BOOL bType, unsigned char ubyIdx, unsigned char ubyPreFetch_Num)
{
	if(!bType)	// i
	{
		switch(ubyIdx)
		{
			case 0:
				//nor_ihf_idx0_prefetch_num
				WriteRegister(LBMC_LBMC_44_reg,0,3,ubyPreFetch_Num);
				break;
			case 1:
				//nor_ihf_idx1_prefetch_num
				WriteRegister(LBMC_LBMC_44_reg,4,7,ubyPreFetch_Num);
				break;
			case 2:
				//nor_ihf_idx2_prefetch_num
				WriteRegister(LBMC_LBMC_44_reg,8,11,ubyPreFetch_Num);
				break;
			default:
				break;
		}
	}
	else	//	p
	{
		switch(ubyIdx)
		{
			case 0:
				//nor_phf_idx0_prefetch_num
				WriteRegister(LBMC_LBMC_44_reg,16,19,ubyPreFetch_Num);
				break;
			case 1:
				//nor_phf_idx1_prefetch_num
				WriteRegister(LBMC_LBMC_44_reg,20,23,ubyPreFetch_Num);
				break;
			case 2:
				//nor_phf_idx2_prefetch_num
				WriteRegister(LBMC_LBMC_44_reg,24,27,ubyPreFetch_Num);
				break;
			default:
				break;
		}
	}
}

/**
 * @brief This function set lbmc single lf prefetch number
 * @param [in] BOOL bType: 0 for i, 1 for p
 * @param [in] ubyIdx(0~2)
 * @param [in] ubyPreFetch_Num
 * @retval VOID
*/
VOID HAL_MC_SetLBMCSingleLFPreFetchNum(BOOL bType, unsigned char ubyIdx, unsigned char ubyPreFetch_Num)
{
	if(!bType)	// i
	{
		switch(ubyIdx)
		{
			case 0:
				//sing_ilf_idx0_prefetch_num
				WriteRegister(LBMC_LBMC_48_reg,0,6,ubyPreFetch_Num);
				break;
			case 1:
				//sing_ilf_idx1_prefetch_num
				WriteRegister(LBMC_LBMC_48_reg,7,13,ubyPreFetch_Num);
				break;
			case 2:
				//sing_ilf_idx2_prefetch_num
				WriteRegister(LBMC_LBMC_48_reg,14,20,ubyPreFetch_Num);
				break;
			case 3:
				//sing_ilf_idx3_prefetch_num
				WriteRegister(LBMC_LBMC_48_reg,21,27,ubyPreFetch_Num);
				break;
			case 4:
				//sing_ilf_idx4_prefetch_num
				WriteRegister(LBMC_LBMC_4C_reg,0,6,ubyPreFetch_Num);
				break;
			case 5:
				//sing_ilf_idx5_prefetch_num
				WriteRegister(LBMC_LBMC_4C_reg,7,13,ubyPreFetch_Num);
				break;
			case 6:
				//sing_ilf_idx6_prefetch_num
				WriteRegister(LBMC_LBMC_4C_reg,14,20,ubyPreFetch_Num);
				break;
			case 7:
				//sing_ilf_idx7_prefetch_num
				WriteRegister(LBMC_LBMC_4C_reg,21,27,ubyPreFetch_Num);
				break;
			case 8:
				//sing_ilf_idx8_prefetch_num
				WriteRegister(LBMC_LBMC_50_reg,0,6,ubyPreFetch_Num);
				break;
			default:
				break;
		}
	}
	else	//	p
	{
		switch(ubyIdx)
		{
			case 0:
				//sing_plf_idx0_prefetch_num
				WriteRegister(LBMC_LBMC_54_reg,0,6,ubyPreFetch_Num);
				break;
			case 1:
				//sing_plf_idx1_prefetch_num
				WriteRegister(LBMC_LBMC_54_reg,7,13,ubyPreFetch_Num);
				break;
			case 2:
				//sing_plf_idx2_prefetch_num
				WriteRegister(LBMC_LBMC_54_reg,14,20,ubyPreFetch_Num);
				break;
			case 3:
				//sing_plf_idx3_prefetch_num
				WriteRegister(LBMC_LBMC_54_reg,21,27,ubyPreFetch_Num);
				break;
			case 4:
				//sing_plf_idx4_prefetch_num
				WriteRegister(LBMC_LBMC_58_reg,0,6,ubyPreFetch_Num);
				break;
			case 5:
				//sing_plf_idx5_prefetch_num
				WriteRegister(LBMC_LBMC_58_reg,7,13,ubyPreFetch_Num);
				break;
			case 6:
				//sing_plf_idx6_prefetch_num
				WriteRegister(LBMC_LBMC_58_reg,14,20,ubyPreFetch_Num);
				break;
			case 7:
				//sing_plf_idx7_prefetch_num
				WriteRegister(LBMC_LBMC_58_reg,21,27,ubyPreFetch_Num);
				break;
			case 8:
				//sing_plf_idx8_prefetch_num
				WriteRegister(LBMC_LBMC_5C_reg,0,6,ubyPreFetch_Num);
				break;
			default:
				break;
		}
	}
}

/**
 * @brief This function set lbmc single hf prefetch number
 * @param [in] BOOL bType: 0 for i, 1 for p
 * @param [in] ubyIdx(0~2)
 * @param [in] ubyPreFetch_Num
 * @retval VOID
*/
VOID HAL_MC_SetLBMCSingleHFPreFetchNum(BOOL bType, unsigned char ubyIdx, unsigned char ubyPreFetch_Num)
{
	if(!bType)	// i
	{
		switch(ubyIdx)
		{
			case 0:
				//sing_ihf_idx0_prefetch_num
				WriteRegister(LBMC_LBMC_60_reg,0,4,ubyPreFetch_Num);
				break;
			case 1:
				//sing_ihf_idx1_prefetch_num
				WriteRegister(LBMC_LBMC_60_reg,5,9,ubyPreFetch_Num);
				break;
			case 2:
				//sing_ihf_idx2_prefetch_num
				WriteRegister(LBMC_LBMC_60_reg,10,14,ubyPreFetch_Num);
				break;
			default:
				break;
		}
	}
	else	//	p
	{
		switch(ubyIdx)
		{
			case 0:
				//sing_phf_idx0_prefetch_num
				WriteRegister(LBMC_LBMC_60_reg,16,20,ubyPreFetch_Num);
				break;
			case 1:
				//sing_phf_idx1_prefetch_num
				WriteRegister(LBMC_LBMC_60_reg,21,25,ubyPreFetch_Num);
				break;
			case 2:
				//sing_phf_idx2_prefetch_num
				WriteRegister(LBMC_LBMC_60_reg,26,30,ubyPreFetch_Num);
				break;
			default:
				break;
		}
	}
}

/**
 * @brief This function set mc rim 0
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_MC_SetMCRim0(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	mc_mc_50_RBUS mc_mc_50;
	mc_mc_54_RBUS mc_mc_54;
	mc_mc_50.regValue = rtd_inl(MC_MC_50_reg);
	mc_mc_54.regValue = rtd_inl(MC_MC_54_reg);

	mc_mc_50.mc_top_rim0 = u32Top;
	mc_mc_50.mc_bot_rim0 = u32Bottom;
	mc_mc_54.mc_lft_rim0 = u32Left;
	mc_mc_54.mc_rht_rim0 = u32Right;

	rtd_outl(MC_MC_50_reg, mc_mc_50.regValue);
	rtd_outl(MC_MC_54_reg, mc_mc_54.regValue);
#if 0
	//reg_mc_top_rim0
	WriteRegister(MC_MC_50_reg,0,12, u32Top);
	//reg_mc_bot_rim0
	WriteRegister(MC_MC_50_reg,16,28, u32Bottom);
	//reg_mc_lft_rim0
	WriteRegister(MC_MC_54_reg,0,12, u32Left);
	//reg_mc_rht_rim0
	WriteRegister(MC_MC_54_reg,16,28, u32Right);
#endif
}

/**
 * @brief This function set mc rim 1
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_MC_SetMCRim1(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	mc_mc_58_RBUS mc_mc_58;
	mc_mc_5c_RBUS mc_mc_5C;
	mc_mc_58.regValue = rtd_inl(MC_MC_58_reg);
	mc_mc_5C.regValue = rtd_inl(MC_MC_5C_reg);

	mc_mc_58.mc_top_rim1 = u32Top;
	mc_mc_58.mc_bot_rim1 = u32Bottom;
	mc_mc_5C.mc_lft_rim1 = u32Left;
	mc_mc_5C.mc_rht_rim1 = u32Right;

	rtd_outl(MC_MC_58_reg, mc_mc_58.regValue);
	rtd_outl(MC_MC_5C_reg, mc_mc_5C.regValue);
#if 0
	//reg_mc_top_rim1
	WriteRegister(MC_MC_58_reg,0,12, u32Top);
	//reg_mc_bot_rim1
	WriteRegister(MC_MC_58_reg,16,28, u32Bottom);
	//reg_mc_lft_rim1
	WriteRegister(MC_MC_5C_reg,0,12, u32Left);
	//reg_mc_rht_rim1
	WriteRegister(MC_MC_5C_reg,16,28, u32Right);
#endif
}

/**
 * @brief This function set mc var lp enable
 * @param [in] unsigned int u32En
 * @retval VOID
*/
VOID HAL_MC_SetVarLPFEn(unsigned int u32En)
{
	//reg_mc_var_lpf_en
	WriteRegister(MC2_MC2_50_reg,0,1,u32En);
}

/**
 * @brief This function set mc var lp 17 tap enable
 * @param [in] unsigned int BOOL
 * @retval VOID
*/
VOID HAL_MC_SetVar17TapEn(BOOL bEnable)
{

	//reg_mc_var17tap_en
	if(Scaler_MEMC_GetPanelSizeByDisp() == MEMC_SIZE_2K){
		WriteRegister(MC2_MC2_68_reg,18,19, 0x0);  //vartap_sel
	}
	else if(Scaler_MEMC_GetPanelSizeByDisp() == MEMC_SIZE_3K){
		WriteRegister(MC2_MC2_68_reg,18,19, 0x1);  //vartap_sel
	}
	else
		WriteRegister(MC2_MC2_68_reg,19,19,(unsigned int)bEnable);
}

/**
 * @brief This function set mc pr mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_MC_SetPRMode(unsigned int u32Mode)
{
	//reg_mc_v_l_r
	WriteRegister(MC_MC_30_reg,3,3,u32Mode);
}

/**
 * @brief This function set mc poly enable
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetPolyEnable(unsigned int u32Val)
{
	WriteRegister(MC_MC_40_reg,0,1,u32Val);
	WriteRegister(MC_MC_40_reg,6,7,u32Val);
}

#if 1
/**
 * @brief This function set block logo rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KMC_LOGO_Set_BlockRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(MC2_MC2_9C_reg,20,29, u32Top);	
	WriteRegister(MC2_MC2_A0_reg,0,9, u32Bottom);	
	WriteRegister(MC2_MC2_9C_reg,0,9, u32Left);	
	WriteRegister(MC2_MC2_9C_reg,10,19, u32Right);	
}

/**
 * @brief This function set pixel pixel rim
 * @param [in] unsigned int u32Top
 * @param [in] unsigned int u32Bottom
 * @param [in] unsigned int u32Left
 * @param [in] unsigned int u32Right
 * @retval VOID
*/
VOID HAL_KMC_LOGO_Set_PixelRim(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	WriteRegister(MC2_MC2_A4_reg,20,29, u32Top);	
	WriteRegister(MC2_MC2_A8_reg,0,9, u32Bottom);	
	WriteRegister(MC2_MC2_A4_reg,0,9, u32Left);	
	WriteRegister(MC2_MC2_A4_reg,10,19, u32Right);	
}

/**
 * @brief This function set mc block logo htotal
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCBLKLogoHtotal(unsigned int u32Val)
{
	WriteRegister(MC_MC_14_reg,0,13,u32Val);
}

/**
 * @brief This function set mc block logo hact
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCBLKLogoHact(unsigned int u32Val)
{
	WriteRegister(MC_MC_14_reg,14,27,u32Val);
}

/**
 * @brief This function set mc block logo row num
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCBLKLogoRowNum(unsigned int u32Val)
{
	WriteRegister(MC_MC_18_reg,8,15,u32Val);
}

/**
 * @brief This function set mc block logo row type
 * @param [in] unsigned int u32Val
 * @retval VOID
*/
VOID HAL_MC_SetMCBLKLogoRowType(unsigned int u32Val)
{
	WriteRegister(MC_MC_18_reg,6,7,u32Val);
}

/**
 * @brief This function set lbmc share hf enable
 * @param [in] unsigned int BOOL
 * @retval VOID
*/
VOID HAL_MC_SetLBMCShareHFEn(BOOL bEnable)
{
	WriteRegister(LBMC_LBMC_88_reg,13,13,(unsigned int)bEnable);
	WriteRegister(LBMC_LBMC_88_reg,22,22,(unsigned int)bEnable);
}
#endif

unsigned int HAL_MC_GetMC04_06_total_data_cnt(VOID)
{
	#if (IC_K4LP || IC_K3LP)
	unsigned int u32InINT_Sts = 0;
	ReadRegister(MC_DMA_MC_LF_P_DMA_RD_Ctrl_reg,0,21, &u32InINT_Sts);
	return u32InINT_Sts;
	#else
	return FALSE;
	#endif
}

/**
 * @brief This function enable  local fallback
 * @param [in] BOOL bEnalbe
 * @retval VOID
*/
VOID HAL_MC_SetLFBEnable(BOOL bEnalbe)
{
	if(!bEnalbe)
	{
		WriteRegister(MC_MC_28_reg,15,22, 0);	
	}	
	WriteRegister(MC_MC_28_reg,14,14, (unsigned int)(!bEnalbe));	
}

VOID HAL_PQL_TOP_SetInputIntp_en(BOOL bEnalbe)
{
	WriteRegister(HARDWARE_HARDWARE_63_reg,30,30, (unsigned int)bEnalbe);
}

VOID HAL_PQL_TOP_SetOnefiftIntp_en(BOOL bEnalbe)
{
	WriteRegister(HARDWARE_HARDWARE_63_reg,29,29, (unsigned int)bEnalbe);
}

VOID HAL_PQL_TOP_SetOutputIntp_en(BOOL bEnalbe)
{
	WriteRegister(HARDWARE_HARDWARE_63_reg,31,31, (unsigned int)bEnalbe);
}



