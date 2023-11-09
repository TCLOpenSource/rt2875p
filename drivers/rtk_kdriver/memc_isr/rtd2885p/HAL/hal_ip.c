/**
 * @file hal_ip.c
 * @brief This file is for ip register setting
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
#include "memc_reg_def.h"


/**
 * @brief This function set ippre csc enable
 * @param [in] BOOL bEnable true for enable, false for diable
 * @retval VOID
*/
VOID HAL_IP_SetIPPRECSCEnable(BOOL bEnable)
{
	if(bEnable)
	{
		//reg_convert_on_h
		WriteRegister(IPPRE_IPPRE_94_reg,24,24,1);
		//reg_convert_on_l
		WriteRegister(IPPRE_IPPRE_B0_reg,24,24,1);
	}
	else
	{
		//reg_convert_on_h
		WriteRegister(IPPRE_IPPRE_94_reg,24,24,0);	
		//reg_convert_on_l
		WriteRegister(IPPRE_IPPRE_B0_reg,24,24,0);
	}
}

/**
 * @brief This function set ippre csc mode
 * @param [in] CSC_MODE enCSC_Mode
 * @retval VOID
*/
VOID HAL_IP_SetIPPRECSCMode(CSC_MODE enCSC_Mode)
{
	//reg_convert_mode_h
	WriteRegister(IPPRE_IPPRE_94_reg,25,28,(unsigned int)enCSC_Mode);
	//reg_convert_mode_l
	WriteRegister(IPPRE_IPPRE_B0_reg,25,28,(unsigned int)enCSC_Mode);
}

/**
 * @brief This function set ippre 444to422 Enable
 * @param [in]  bool bEnable
 * @retval VOID
*/
VOID HAL_IP_SetIPPRE444to422Enable(BOOL bEnable)
{
	WriteRegister(IPPRE_IPPRE_04_reg,2,2, (unsigned int)bEnable);
}

/**
 * @brief This function set ippre retiming
 * @param [in] unsigned int u32Hact
 * @param [in] unsigned int u32Vact 
 * @retval VOID
*/
VOID HAL_IP_SetIPPRERetiming(unsigned int u32Hact, unsigned int u32Vact)
{
	//reg_ippre_retiming_hact
	WriteRegister(IPPRE_IPPRE_08_reg,12,23,u32Hact/2);
	//reg_ippre_retiming_vact
	WriteRegister(IPPRE_IPPRE_08_reg,0,11,u32Vact);
}

/**
 * @brief This function get ippre retiming dummy de
 * @param [in] VOID
 * @param [in] unsigned int u32Vact 
 * @retval retiming dummy de
*/
unsigned int HAL_IP_GetIPPRERetimingDummyDE(VOID)
{
	unsigned int uiVal = 0;
	ReadRegister(IPPRE_IPPRE_08_reg,24,27, &uiVal);
	return uiVal;
}

/**
 * @brief This function set ippre pattern size
 * @param [in] TIMING_PARA_ST stInputTiming 
 * @retval VOID
*/
VOID HAL_IP_SetIPPREPatternSize(TIMING_PARA_ST stInputTiming)
{
	//reg_hact[11:0]
	WriteRegister(IPPRE_IPPRE_10_reg,0,11,stInputTiming.u32HActive/2);
	//reg_vact[12:0]
	WriteRegister(IPPRE_IPPRE_10_reg,16,28,stInputTiming.u32VActive);
	//reg_htotal[11:0]
	WriteRegister(IPPRE_IPPRE_14_reg,0,11,stInputTiming.u32HTotal/2);
	//reg_vtotal[11:0]
	WriteRegister(IPPRE_IPPRE_14_reg,16,28,stInputTiming.u32VTotal);	
}

/**
 * @brief This function set ippre use mute sync enable
 * @param [in]  bool bEnable
 * @retval VOID
*/
VOID HAL_IP_SetIPPREMuteUseMuteSyncEnable(BOOL bEnable)
{
	//reg_ippre_use_mute_vs
	WriteRegister(IPPRE_IPPRE_B4_reg,27,27, (unsigned int)bEnable);
	//reg_ippre_use_mute_hs
	//WriteRegister(IPPRE_IPPRE_B4_reg,28,28, (unsigned int)bEnable);
}

/**
 * @brief This function set ippre mute set mute detect enable
 * @param [in]  bool bEnable
 * @retval VOID
*/
VOID HAL_IP_SetIPPREMuteSetMuteDetectEnable(BOOL bEnable)
{
	//reg_ippre_mute_det_function_en
	WriteRegister(IPPRE_IPPRE_E0_reg,24,24, (unsigned int)bEnable);
}

/**
 * @brief This function set ippre mute h active
 * @param [in] unsigned int u32Hact 
 * @param [in] unsigned int u32Hact_Min 
 * @param [in] unsigned int u32Hact_Max 
 * @retval VOID
*/
VOID HAL_IP_SetIPPREMuteHactive(unsigned int u32Hact, unsigned int u32Hact_Min, unsigned int u32Hact_Max)
{
	//reg_ippre_mute_hact
	WriteRegister(IPPRE_IPPRE_E0_reg,0,11, u32Hact);
	//reg_ippre_mute_hact_min
	WriteRegister(IPPRE_IPPRE_E4_reg,0,11, u32Hact_Min);
	//reg_ippre_mute_hact_max
	WriteRegister(IPPRE_IPPRE_E4_reg,12,23, u32Hact_Max);
}

/**
 * @brief This function set ippre v active
 * @param [in] unsigned int u32Vact 
 * @param [in] unsigned int u32Vact_Min 
 * @param [in] unsigned int u32Vact_Max 
 * @retval VOID
*/
VOID HAL_IP_SetIPPREMuteVactive(unsigned int u32Vact, unsigned int u32Vact_Min, unsigned int u32Vact_Max)
{
	//reg_ippre_mute_vact
	WriteRegister(IPPRE_IPPRE_E0_reg,12,23, u32Vact);
	//reg_ippre_mute_vact_min
	WriteRegister(IPPRE_IPPRE_B8_reg,0,11, u32Vact_Min);
	//reg_ippre_mute_vact_max
	WriteRegister(IPPRE_IPPRE_B8_reg,12,23, u32Vact_Max);
}

/**
 * @brief This function set ippre mute clock threshold
 * @param [in] unsigned int u32Clk_low_th 
 * @param [in] unsigned int u32Clk_high_th 
 * @retval VOID
*/
VOID HAL_IP_SetIPPREMuteClockThreshold(unsigned int u32Clk_low_th, unsigned int u32Clk_high_th)
{
	//reg_ippre_mute_freq_low_th
	WriteRegister(IPPRE_IPPRE_BC_reg,12,21, u32Clk_low_th);
	//reg_ippre_mute_freq_high_th
	WriteRegister(IPPRE_IPPRE_BC_reg,22,31, u32Clk_high_th);
}

/**
 * @brief This function set 3d fs lr inverse
 * @param [in] BOOL bEnable 
 * @retval VOID
*/
VOID HAL_IP_Set3DFSLRInverse(BOOL bEnable)
{
	//reg_patt_3d_fs_lr_inv
	WriteRegister(IPPRE_IPPRE_20_reg,2,2,(unsigned int)bEnable);	
}


/*#if RTK_MEMC_Performance_tunging_from_tv001
#if 0 // !REAL_CHIP	// FPGA
#else
#ifndef REAL_CHIP	// FPGA
#endif*/

#ifndef REAL_CHIP	// FPGA


/**
 * @brief This function set topA ipme retiming
 * @param [in] TIMING_PARA_ST stInputTiming
 * @retval VOID
*/
VOID HAL_IP_SetTOPA_IPMERetiming(TIMING_PARA_ST stInputTiming)
{
	//reg_sh7_ipme_hact
	WriteRegister(0xb809h028,0,11,stInputTiming.u32HActive/2);	//no RBUS in REAL_CHIP search JIRA RL6672-2390 for access
	//reg_sh7_ipme_vact
	WriteRegister(0xb809h028,12,23,stInputTiming.u32VActive);
}

/**
 * @brief This function set 2 port enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_IP_SetTOPA2PortEnable(BOOL bEnable)
{	
	WriteRegister(0xb809h008,16,16, (unsigned int)bEnable);	//no RBUS in REAL_CHIP search JIRA RL6672-2390 for access
}

/**
 * @brief This function set ramp enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_IP_SetTOPASH13Ramp(BOOL bEnable)
{	
	WriteRegister(0xb809h008,23,23, (unsigned int)bEnable);	//no RBUS in REAL_CHIP search JIRA RL6672-2390 for access
}

/**
 * @brief This function set topB mute timing
 * @param [in] TIMING_PARA_ST gstInputTiming
 * @param [in] TIMING_PARA_ST gstOutputTiming 
 * @retval VOID
*/
VOID HAL_IP_SetTOPBMuteTiming(TIMING_PARA_ST stInputTiming, TIMING_PARA_ST stOutputTiming)
{
	//reg_mute_htotal[11:0]
	WriteRegister(0xb809i000,0,11,stInputTiming.u32HTotal/2);	//no RBUS in REAL_CHIP search JIRA RL6672-2390 for access
	//reg_mute_vtotal[11:0]
	WriteRegister(0xb809i000,12,23,stInputTiming.u32VTotal);	
	//reg_mute_hs_bporch[7:0]
	WriteRegister(0xb809i000,24,31,stInputTiming.u32HSBPorch/2);	
	//reg_mute_hact[11:0]
	WriteRegister(0xb809i004,0,11,stInputTiming.u32HActive/2);	
	//reg_mute_vact[11:0]
	WriteRegister(0xb809i004,12,23,stInputTiming.u32VActive);	
	//reg_mute_vs_bporch[7:0]
	WriteRegister(0xb809i004,24,31,stInputTiming.u32VSBPorch);	
	//reg_mute_hs_width[3:0]
	WriteRegister(0xb809i008,0,3,stInputTiming.u32HSWidth/2);	
	//reg_mute_hs_width[3:0]
	WriteRegister(0xb809i008,4,7,stInputTiming.u32VSWidth);		
	//reg_mute_hact_min[11:0]
	WriteRegister(0xb809i008,8,19,stInputTiming.u32HActive/2 - 192);
	//reg_mute_hact_max[11:0]
	WriteRegister(0xb809i008,20,31,stInputTiming.u32HActive/2);	
	//reg_mute_vact_min[11:0]
	WriteRegister(0xb809i00c,0,11,stInputTiming.u32VActive - 50);
	//reg_mute_vact_max[11:0]
	WriteRegister(0xb809i00c,12,23,stInputTiming.u32VActive + 50);	
	//reg_inproc_mc_de_hact[11:0]
	WriteRegister(0xb809i018,17,28,stOutputTiming.u32HActive/2);	
	//reg_sh19_nr_hact
	WriteRegister(0xb809i01c,16,27,stInputTiming.u32HActive/2);	
}

/**
 * @brief This function set mute lr self gen
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_IP_SetTOPBMuteLRSelfGen(BOOL bEnable)
{	
	WriteRegister(0xb809i010,31,31, (unsigned int)bEnable);	//no RBUS in REAL_CHIP search JIRA RL6672-2390 for access
}

/**
 * @brief This function set mute force free run
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_IP_SetTOPBForceFreerun(BOOL bEnable)
{	
	WriteRegister(0xb809i014,7,7, (unsigned int)bEnable);	//no RBUS in REAL_CHIP search JIRA RL6672-2390 for access
}

/**
 * @brief This function set mute frame trig position
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_IP_SetTOPBFRMTrigPosition(unsigned int u32Val)
{	
	WriteRegister(0xb809i010,0,11, u32Val);	//no RBUS in REAL_CHIP search JIRA RL6672-2390 for access
}
#endif

/**
 * @brief This function set ip data mapping
 * @param [in] unsigned int u32Data_Mapping 
 * @retval VOID
*/
VOID HAL_IP_SetDataMapping(unsigned int u32Data_Mapping)
{
	//reg_convert_map_h
	WriteRegister(IPPRE_IPPRE_94_reg,29,31,(unsigned int)u32Data_Mapping);	
	//reg_convert_map_l
	WriteRegister(IPPRE_IPPRE_B0_reg,29,31,(unsigned int)u32Data_Mapping);		
}

/**
 * @brief This function get ip data mapping
 * @param [in] VOID
 * @retval data mapping
*/
unsigned int HAL_IP_GetDataMapping(VOID)
{
	unsigned int u32Data_Mapping = 0;
	//reg_convert_map_h
	ReadRegister(IPPRE_IPPRE_94_reg,29,31,&u32Data_Mapping);
	return u32Data_Mapping;
}

