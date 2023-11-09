/**
 * @file hal_dm.c
 * @brief This file is for  DM controller.
 * @date September.12.2014
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
#include "memc_isr/PQL/PQLPlatformDefs.h"
#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
#include <rbus/mc_dma_reg.h>
#include <rbus/me_share_dma_reg.h>
#endif

#ifdef CONFIG_ARM64 //ARM32 compatible
// for register dump
#include <tvscalercontrol/io/ioregdrv.h>
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
#endif

/**
 * @brief This function get kmc 00 start address 0
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKmc00StartAddress0(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_00_start_address0
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(MC_DMA_MC_WDMA_CTRL0_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function get kmc 00 start address 1
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKmc00StartAddress1(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_00_start_address1
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(MC_DMA_MC_WDMA_CTRL1_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function get kmc 00 start address 2
 * @param [in] VOID
 * @retval Address
*/
#define	Kmc00StartAddress2_reg	0xb8099408
unsigned int HAL_DM_GetKmc00StartAddress2(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_00_start_address2
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(Kmc00StartAddress2_reg, 0, 31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function get kmc 00 start address 3
 * @param [in] VOID
 * @retval Address
*/
#define	Kmc00StartAddress3_reg	0xb809940c
unsigned int HAL_DM_GetKmc00StartAddress3(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_00_start_address3
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(Kmc00StartAddress3_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function get kmc 00 start address 4
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKmc00StartAddress4(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_00_start_address4
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(MC_DMA_MC_LF_WDMA_NUM_BL_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function get kmc 00 start address 5
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKmc00StartAddress5(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_00_start_address5
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(MC_DMA_MC_LF_WDMA_MSTART0_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function get kmc 00 line offset address
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKmc00LineOffsetAddress(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_00_line_offset_addr
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(MC_DMA_MC_LF_WDMA_MEND0_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function set kmc 00 end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00EndAddress0(unsigned int u32Address)
{
	//reg_kmc_00_end_address0
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mend0_RBUS memc_mc_dma_mc_lf_wdma_mend0_reg;
	memc_mc_dma_mc_lf_wdma_mend0_reg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MEND0_reg);
	memc_mc_dma_mc_lf_wdma_mend0_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND0_reg,memc_mc_dma_mc_lf_wdma_mend0_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MEND0_reg_ADDR no mat,MC_DMA_MC_LF_WDMA_MEND0_reg_BITSTART no mat,MC_DMA_MC_LF_WDMA_MEND0_reg_BITEND no mat,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00EndAddress1(unsigned int u32Address)
{
        //reg_kmc_00_end_address1
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mend1_RBUS memc_mc_dma_mc_lf_wdma_mend1_eg;
	memc_mc_dma_mc_lf_wdma_mend1_eg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MEND1_reg);
	memc_mc_dma_mc_lf_wdma_mend1_eg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND1_reg,memc_mc_dma_mc_lf_wdma_mend1_eg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MSTART2_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00EndAddress2(unsigned int u32Address)
{
	//reg_kmc_00_end_address2
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mend2_RBUS memc_mc_dma_mc_lf_wdma_mend2_eg;
	memc_mc_dma_mc_lf_wdma_mend2_eg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MEND2_reg);
	memc_mc_dma_mc_lf_wdma_mend2_eg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND2_reg,memc_mc_dma_mc_lf_wdma_mend2_eg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MSTART3_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 3
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00EndAddress3(unsigned int u32Address)
{
	//reg_kmc_00_end_address3
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mend3_RBUS memc_mc_dma_mc_lf_wdma_mend3_eg;
	memc_mc_dma_mc_lf_wdma_mend3_eg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MEND3_reg);
	memc_mc_dma_mc_lf_wdma_mend3_eg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND3_reg,memc_mc_dma_mc_lf_wdma_mend3_eg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MSTART4_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 4
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00EndAddress4(unsigned int u32Address)
{
	//reg_kmc_00_end_address4
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mend4_RBUS memc_mc_dma_mc_lf_wdma_mend4_eg;
	memc_mc_dma_mc_lf_wdma_mend4_eg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MEND4_reg);
	memc_mc_dma_mc_lf_wdma_mend4_eg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND4_reg,memc_mc_dma_mc_lf_wdma_mend4_eg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MSTART5_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 5
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00EndAddress5(unsigned int u32Address)
{
	//reg_kmc_00_end_address5
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mend5_RBUS memc_mc_dma_mc_lf_wdma_mend5_eg;
	memc_mc_dma_mc_lf_wdma_mend5_eg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MEND5_reg);
	memc_mc_dma_mc_lf_wdma_mend5_eg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND5_reg,memc_mc_dma_mc_lf_wdma_mend5_eg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MSTART6_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 6
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
#define	Kmc00EndAddress6_reg	0xb80994d4
VOID HAL_DM_SetKmc00EndAddress6(unsigned int u32Address)
{
	//reg_kmc_00_end_address6
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mend6_RBUS memc_mc_dma_mc_lf_wdma_mend6_eg;
	memc_mc_dma_mc_lf_wdma_mend6_eg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MEND6_reg);
	memc_mc_dma_mc_lf_wdma_mend6_eg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND6_reg,memc_mc_dma_mc_lf_wdma_mend6_eg.regValue);
	#else
	WriteRegister(Kmc00EndAddress6_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 7
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00EndAddress7(unsigned int u32Address)
{
	//reg_kmc_00_end_address7
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mend7_RBUS memc_mc_dma_mc_lf_wdma_mend7_eg;
	memc_mc_dma_mc_lf_wdma_mend7_eg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MEND7_reg);
	memc_mc_dma_mc_lf_wdma_mend7_eg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MEND7_reg,memc_mc_dma_mc_lf_wdma_mend7_eg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_DMA_WR_Ctrl_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 lr offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00LROffsetAddress(unsigned int u32Address)
{
	//reg_kmc_00_lr_offset_addr
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	WriteRegister(MC_DMA_MC_LF_WDMA_MSTART7_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 00 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00LineOffsetAddress(unsigned int u32Address)
{
	//reg_kmc_00_line_offset_addr
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_lstep_RBUS memc_mc_dma_mc_lf_wdma_lstep_reg;
	memc_mc_dma_mc_lf_wdma_lstep_reg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_LSTEP_reg);
	#if IC_K5LP
	u32Address = 0x00000004; //only for k5lp memory bug
	#endif
	memc_mc_dma_mc_lf_wdma_lstep_reg.line_offset_addr=u32Address>>4;
	rtd_outl(MC_DMA_MC_LF_WDMA_LSTEP_reg,memc_mc_dma_mc_lf_wdma_lstep_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MEND0_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 01 start address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01StartAddress0(unsigned int u32Address)
{
	//reg_kmc_00_start_address0
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mstart0_RBUS memc_mc_dma_mc_lf_wdma_mstart0_reg;
	memc_mc_dma_mc_lf_wdma_mstart0_reg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MSTART0_reg);
	memc_mc_dma_mc_lf_wdma_mstart0_reg.regValue=u32Address;   // dma bug   >>4
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART0_reg,memc_mc_dma_mc_lf_wdma_mstart0_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_WDMA_CTRL0_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 01 start address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01StartAddress1(unsigned int u32Address)
{
	//reg_kmc_00_start_address1
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mstart1_RBUS memc_mc_dma_mc_lf_wdma_mstart1_reg;
	memc_mc_dma_mc_lf_wdma_mstart1_reg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MSTART1_reg);
	memc_mc_dma_mc_lf_wdma_mstart1_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART1_reg,memc_mc_dma_mc_lf_wdma_mstart1_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_WDMA_CTRL1_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 01 start address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01StartAddress2(unsigned int u32Address)
{
	//reg_kmc_00_start_address2
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mstart2_RBUS memc_mc_dma_mc_lf_wdma_mstart2_reg;
	memc_mc_dma_mc_lf_wdma_mstart2_reg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MSTART2_reg);
	memc_mc_dma_mc_lf_wdma_mstart2_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART2_reg,memc_mc_dma_mc_lf_wdma_mstart2_reg.regValue);
	#else
	WriteRegister(Kmc00StartAddress2_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 01 start address 3
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01StartAddress3(unsigned int u32Address)
{
	//reg_kmc_00_start_address3
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mstart3_RBUS memc_mc_dma_mc_lf_wdma_mstart3_reg;
	memc_mc_dma_mc_lf_wdma_mstart3_reg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MSTART3_reg);
	memc_mc_dma_mc_lf_wdma_mstart3_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART3_reg,memc_mc_dma_mc_lf_wdma_mstart3_reg.regValue);
	#else
	WriteRegister(Kmc00StartAddress3_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 01 start address 4
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01StartAddress4(unsigned int u32Address)
{
	//reg_kmc_00_start_address4
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mstart4_RBUS memc_mc_dma_mc_lf_wdma_mstart4_reg;
	memc_mc_dma_mc_lf_wdma_mstart4_reg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MSTART4_reg);
	memc_mc_dma_mc_lf_wdma_mstart4_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART4_reg,memc_mc_dma_mc_lf_wdma_mstart4_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_NUM_BL_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 01 start address 5
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01StartAddress5(unsigned int u32Address)
{
	//reg_kmc_00_start_address5
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mstart5_RBUS memc_mc_dma_mc_lf_wdma_mstart5_reg;
	memc_mc_dma_mc_lf_wdma_mstart5_reg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MSTART5_reg);
	memc_mc_dma_mc_lf_wdma_mstart5_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART5_reg,memc_mc_dma_mc_lf_wdma_mstart5_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MSTART0_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 01 start address 5
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01StartAddress6(unsigned int u32Address)
{
	//reg_kmc_00_start_address6
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mstart6_RBUS memc_mc_dma_mc_lf_wdma_mstart6_reg;
	memc_mc_dma_mc_lf_wdma_mstart6_reg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MSTART6_reg);
	memc_mc_dma_mc_lf_wdma_mstart6_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART6_reg,memc_mc_dma_mc_lf_wdma_mstart6_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_DMA_WR_Rule_check_up_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 01 start address 7
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01StartAddress7(unsigned int u32Address)
{
	//reg_kmc_00_start_address7
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_mstart7_RBUS memc_mc_dma_mc_lf_wdma_mstart7_reg;
	memc_mc_dma_mc_lf_wdma_mstart7_reg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_MSTART7_reg);
	memc_mc_dma_mc_lf_wdma_mstart7_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_LF_WDMA_MSTART7_reg,memc_mc_dma_mc_lf_wdma_mstart7_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_DMA_WR_Rule_check_low_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 01 lr offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01LROffsetAddress(unsigned int u32Address)
{
	//reg_kmc_01_lr_offset_addr
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	WriteRegister(MC_DMA_MC_LF_DMA_WR_Water_Monitor_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 01 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
#define	Kmc01LineOffsetAddress_reg	0xb8099474
VOID HAL_DM_SetKmc01LineOffsetAddress(unsigned int u32Address)
{
	//reg_kmc_01_line_offset_addr
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_lstep_RBUS memc_mc_dma_mc_hf_wdma_lstep_reg;
	memc_mc_dma_mc_hf_wdma_lstep_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_LSTEP_reg);
	#if (IC_K5LP)
	u32Address = 0x00000004; //only for k5lp memory bug
	#endif
	memc_mc_dma_mc_hf_wdma_lstep_reg.line_offset_addr=u32Address>>4;
	rtd_outl(MC_DMA_MC_HF_WDMA_LSTEP_reg,memc_mc_dma_mc_hf_wdma_lstep_reg.regValue);
	#else
	WriteRegister(Kmc01LineOffsetAddress_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 00 start address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00StartAddress0(unsigned int u32Address)
{
	//reg_kmc_01_start_address0
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mstart0_RBUS memc_mc_dma_mc_hf_wdma_mstart0_reg;
	memc_mc_dma_mc_hf_wdma_mstart0_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MSTART0_reg);
	memc_mc_dma_mc_hf_wdma_mstart0_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART0_reg,memc_mc_dma_mc_hf_wdma_mstart0_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MEND3_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 00 start address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00StartAddress1(unsigned int u32Address)
{
	//reg_kmc_01_start_address1
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mstart1_RBUS memc_mc_dma_mc_hf_wdma_mstart1_reg;
	memc_mc_dma_mc_hf_wdma_mstart1_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MSTART1_reg);
	memc_mc_dma_mc_hf_wdma_mstart1_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART1_reg,memc_mc_dma_mc_hf_wdma_mstart1_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MEND4_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 00 start address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00StartAddress2(unsigned int u32Address)
{
	//reg_kmc_01_start_address1
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mstart2_RBUS memc_mc_dma_mc_hf_wdma_mstart2_reg;
	memc_mc_dma_mc_hf_wdma_mstart2_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MSTART2_reg);
	memc_mc_dma_mc_hf_wdma_mstart2_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART2_reg,memc_mc_dma_mc_hf_wdma_mstart2_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MEND5_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 00 start address 3
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00StartAddress3(unsigned int u32Address)
{
	//reg_kmc_01_start_address3
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mstart3_RBUS memc_mc_dma_mc_hf_wdma_mstart3_reg;
	memc_mc_dma_mc_hf_wdma_mstart3_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MSTART3_reg);
	memc_mc_dma_mc_hf_wdma_mstart3_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART3_reg,memc_mc_dma_mc_hf_wdma_mstart3_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MEND6_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 00 start address 4
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00StartAddress4(unsigned int u32Address)
{
	//reg_kmc_01_start_address4
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mstart4_RBUS memc_mc_dma_mc_hf_wdma_mstart4_reg;
	memc_mc_dma_mc_hf_wdma_mstart4_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MSTART4_reg);
	memc_mc_dma_mc_hf_wdma_mstart4_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART4_reg,memc_mc_dma_mc_hf_wdma_mstart4_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MEND7_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 00 start address 5
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00StartAddress5(unsigned int u32Address)
{
	//reg_kmc_01_start_address5
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mstart5_RBUS memc_mc_dma_mc_hf_wdma_mstart5_reg;
	memc_mc_dma_mc_hf_wdma_mstart5_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MSTART5_reg);
	memc_mc_dma_mc_hf_wdma_mstart5_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART5_reg,memc_mc_dma_mc_hf_wdma_mstart5_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_LSTEP_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 00 start address 6
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00StartAddress6(unsigned int u32Address)
{
	//reg_kmc_01_start_address6
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mstart6_RBUS memc_mc_dma_mc_hf_wdma_mstart6_reg;
	memc_mc_dma_mc_hf_wdma_mstart6_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MSTART6_reg);
	memc_mc_dma_mc_hf_wdma_mstart6_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART6_reg,memc_mc_dma_mc_hf_wdma_mstart6_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_DMA_WR_status_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function set kmc 00 start address 7
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc00StartAddress7(unsigned int u32Address)
{
	//reg_kmc_01_start_address7
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mstart7_RBUS memc_mc_dma_mc_hf_wdma_mstart7_reg;
	memc_mc_dma_mc_hf_wdma_mstart7_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MSTART7_reg);
	memc_mc_dma_mc_hf_wdma_mstart7_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MSTART7_reg,memc_mc_dma_mc_hf_wdma_mstart7_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_DMA_WR_status2_reg,0,31,u32Address);
	#endif
}

/**
 * @brief This function get kmc 01 start address 0
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKmc01StartAddress0(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_01_start_address0
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(MC_DMA_MC_LF_WDMA_MEND3_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function get kmc 01 start address 1
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKmc01StartAddress1(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_01_start_address1
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(MC_DMA_MC_LF_WDMA_MEND4_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function get kmc 00 start address 2
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKmc01StartAddress2(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_01_start_address2
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(MC_DMA_MC_LF_WDMA_MEND5_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function get kmc 00 start address 3
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKmc01StartAddress3(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_01_start_address3
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(MC_DMA_MC_LF_WDMA_MEND6_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function get kmc 00 start address 4
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKmc01StartAddress4(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_01_start_address4
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(MC_DMA_MC_LF_WDMA_MEND7_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function get kmc 00 start address 5
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKmc01StartAddress5(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_01_start_address5
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(MC_DMA_MC_LF_WDMA_LSTEP_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function get kmc 00 line offset address
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKmc01LineOffsetAddress(VOID)
{
	unsigned int u32Val = 0;
	//reg_kmc_01_line_offset_addr
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	ReadRegister(Kmc01LineOffsetAddress_reg,0,31,&u32Val);
	#endif
	return u32Val;
}

/**
 * @brief This function set kmc 00 end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01EndAddress0(unsigned int u32Address)
{
	//reg_kmc_01_end_address0
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mend0_RBUS memc_mc_dma_mc_hf_wdma_mend0_reg;
	memc_mc_dma_mc_hf_wdma_mend0_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MEND0_reg);
	memc_mc_dma_mc_hf_wdma_mend0_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND0_reg,memc_mc_dma_mc_hf_wdma_mend0_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_DMA_WR_Rule_check_up_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01EndAddress1(unsigned int u32Address)
{
	//reg_kmc_01_end_address1
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mend1_RBUS memc_mc_dma_mc_hf_wdma_mend1_reg;
	memc_mc_dma_mc_hf_wdma_mend1_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MEND1_reg);
	memc_mc_dma_mc_hf_wdma_mend1_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND1_reg,memc_mc_dma_mc_hf_wdma_mend1_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_DMA_WR_Rule_check_low_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01EndAddress2(unsigned int u32Address)
{
	//reg_kmc_01_end_address2
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mend2_RBUS memc_mc_dma_mc_hf_wdma_mend2_reg;
	memc_mc_dma_mc_hf_wdma_mend2_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MEND2_reg);
	memc_mc_dma_mc_hf_wdma_mend2_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND2_reg,memc_mc_dma_mc_hf_wdma_mend2_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_DMA_WR_Ctrl_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 3
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01EndAddress3(unsigned int u32Address)
{
	//reg_kmc_01_end_address3
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mend3_RBUS memc_mc_dma_mc_hf_wdma_mend3_reg;
	memc_mc_dma_mc_hf_wdma_mend3_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MEND3_reg);
	memc_mc_dma_mc_hf_wdma_mend3_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND3_reg,memc_mc_dma_mc_hf_wdma_mend3_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_DMA_WR_status_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 4
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01EndAddress4(unsigned int u32Address)
{
	//reg_kmc_01_end_address4
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mend4_RBUS memc_mc_dma_mc_hf_wdma_mend4_reg;
	memc_mc_dma_mc_hf_wdma_mend4_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MEND4_reg);
	memc_mc_dma_mc_hf_wdma_mend4_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND4_reg,memc_mc_dma_mc_hf_wdma_mend4_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_DMA_WR_status1_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 5
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01EndAddress5(unsigned int u32Address)
{
	//reg_kmc_01_end_address5
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mend5_RBUS memc_mc_dma_mc_hf_wdma_mend5_reg;
	memc_mc_dma_mc_hf_wdma_mend5_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MEND5_reg);
	memc_mc_dma_mc_hf_wdma_mend5_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND5_reg,memc_mc_dma_mc_hf_wdma_mend5_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_DMA_WR_status2_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 6
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01EndAddress6(unsigned int u32Address)
{
	//reg_kmc_01_end_address6
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mend6_RBUS memc_mc_dma_mc_hf_wdma_mend6_reg;
	memc_mc_dma_mc_hf_wdma_mend6_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MEND6_reg);
	memc_mc_dma_mc_hf_wdma_mend6_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND6_reg,memc_mc_dma_mc_hf_wdma_mend6_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_DMA_WR_status1_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc 00 end address 7
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmc01EndAddress7(unsigned int u32Address)
{
	//reg_kmc_01_end_address7
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_mend7_RBUS memc_mc_dma_mc_hf_wdma_mend7_reg;
	memc_mc_dma_mc_hf_wdma_mend7_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_MEND7_reg);
	memc_mc_dma_mc_hf_wdma_mend7_reg.regValue=u32Address;
	rtd_outl(MC_DMA_MC_HF_WDMA_MEND7_reg,memc_mc_dma_mc_hf_wdma_mend7_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_DMA_WR_Water_Monitor_reg,0,31,u32Address); 
	#endif
}

/**
 * @brief This function set kmc vtotal
 * @param [in] unsigned int u32Vtotal
 * @retval VOID
*/
VOID HAL_DM_SetKmcVtotal(unsigned int u32Vtotal)
{
	//reg_kmc_vtotal
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	WriteRegister(MC_DMA_MC_HF_WDMA_MEND3_reg,0,11,u32Vtotal);
	#endif
}

VOID HAL_DM_SetKmcHVsize(unsigned int u32Hsize,unsigned int u32Vsize)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_kmc_hv_size
	mc_dma_mc_wdma_ctrl1_RBUS memc_mc_dma_mc_wdma_ctrl1_reg;
	memc_mc_dma_mc_wdma_ctrl1_reg.regValue= rtd_inl(MC_DMA_MC_WDMA_CTRL1_reg);
	memc_mc_dma_mc_wdma_ctrl1_reg.width=u32Hsize;
	memc_mc_dma_mc_wdma_ctrl1_reg.height=u32Vsize;
	rtd_outl(MC_DMA_MC_WDMA_CTRL1_reg,memc_mc_dma_mc_wdma_ctrl1_reg.regValue);
	#endif
}
VOID HAL_DM_SetKmc_LF_NUM_BL(unsigned int u32NUM,unsigned int u32BL,unsigned int u32REMAIN)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_wdma_num_bl_RBUS memc_mc_dma_mc_lf_wdma_num_bl_reg;
	memc_mc_dma_mc_lf_wdma_num_bl_reg.regValue= rtd_inl(MC_DMA_MC_LF_WDMA_NUM_BL_reg);
	memc_mc_dma_mc_lf_wdma_num_bl_reg.num=u32NUM;
	memc_mc_dma_mc_lf_wdma_num_bl_reg.bl=u32BL;
	memc_mc_dma_mc_lf_wdma_num_bl_reg.remain=u32REMAIN;
	rtd_outl(MC_DMA_MC_LF_WDMA_NUM_BL_reg,memc_mc_dma_mc_lf_wdma_num_bl_reg.regValue);
	#endif
}
VOID HAL_DM_SetKmc_HF_NUM_BL(unsigned int u32NUM,unsigned int u32BL,unsigned int u32REMAIN)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_wdma_num_bl_RBUS memc_mc_dma_mc_hf_wdma_num_bl_reg;
	memc_mc_dma_mc_hf_wdma_num_bl_reg.regValue= rtd_inl(MC_DMA_MC_HF_WDMA_NUM_BL_reg);
	memc_mc_dma_mc_hf_wdma_num_bl_reg.num=u32NUM;
	memc_mc_dma_mc_hf_wdma_num_bl_reg.bl=u32BL;
	memc_mc_dma_mc_hf_wdma_num_bl_reg.remain=u32REMAIN;
	rtd_outl(MC_DMA_MC_HF_WDMA_NUM_BL_reg,memc_mc_dma_mc_hf_wdma_num_bl_reg.regValue);
	#endif
}

/**
 * @brief This function set kmc input 3d enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmcInput3DEnable(BOOL bEnable)
{
	//reg_kmc_input_3d_enable
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)  
	WriteRegister(MC_DMA_MC_HF_WDMA_MEND6_reg,3,3,(unsigned int)bEnable);
	#endif
}

/**
 * @brief This function set kmc input 3d Mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_DM_SetKmcInput3DMode(unsigned int u32Mode)
{
	//reg_kmc_input_3d_mode
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
 	WriteRegister(MC_DMA_MC_HF_WDMA_MEND6_reg,4,7,u32Mode);	
	#endif
}

/**
 * @brief This function set kmc output 3d enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmcOutput3DEnable(BOOL bEnable)
{
	//reg_kmc_output_3d_enable
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	WriteRegister(MC_DMA_MC_HF_WDMA_MEND6_reg,8,8,(unsigned int)bEnable);	
	#endif
}

/**
 * @brief This function set kmc output 3d Mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_DM_SetKmcOutput3DMode(unsigned int u32Mode)
{
	//reg_kmc_output_3d_mode
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
 	WriteRegister(MC_DMA_MC_HF_WDMA_MEND6_reg,9,10,u32Mode);	
	#endif
}

/**
 * @brief This function set kmc 422 enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmc422Enable(BOOL bEnable)
{
	//reg_kmc_mode422_enable
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)  
	mc_dma_mc_wdma_ctrl0_RBUS memc_mc_dma_mc_wdma_ctrl0_reg;
	mc_dma_mc_rdma_ctrl_RBUS memc_mc_dma_mc_rdma_ctrl_reg;
	memc_mc_dma_mc_wdma_ctrl0_reg.regValue= rtd_inl(MC_DMA_MC_WDMA_CTRL0_reg);
	memc_mc_dma_mc_rdma_ctrl_reg.regValue= rtd_inl(MC_DMA_MC_RDMA_CTRL_reg);
	memc_mc_dma_mc_wdma_ctrl0_reg.dat_fmt=(unsigned int)!bEnable;
	memc_mc_dma_mc_rdma_ctrl_reg.dat_fmt=(unsigned int)!bEnable;
	rtd_outl(MC_DMA_MC_WDMA_CTRL0_reg,memc_mc_dma_mc_wdma_ctrl0_reg.regValue);
	rtd_outl(MC_DMA_MC_RDMA_CTRL_reg,memc_mc_dma_mc_rdma_ctrl_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_WDMA_MEND6_reg,0,0,(unsigned int)bEnable);
	#endif
}

/**
 * @brief This function set kmc vflip enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmcVflipEnable(BOOL bEnable)
{
	//reg_kmc_vflip_enable
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)  
	mc_dma_mc_rdma_ctrl_RBUS memc_mc_dma_mc_rdma_ctrl_reg;
	memc_mc_dma_mc_rdma_ctrl_reg.regValue= rtd_inl(MC_DMA_MC_RDMA_CTRL_reg);
	memc_mc_dma_mc_rdma_ctrl_reg.vflip_enable=(unsigned int)bEnable;
	rtd_outl(MC_DMA_MC_RDMA_CTRL_reg,memc_mc_dma_mc_rdma_ctrl_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_WDMA_MEND6_reg,11,11,(unsigned int)bEnable);
	#endif
}

/**
 * @brief This function set dm pr mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_DM_SetPRMode(unsigned int u32Mode)
{
	//reg_kmc_pr_mode
	#if !(IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	WriteRegister(MC_DMA_MC_HF_WDMA_MEND6_reg,17,17,u32Mode);
	#endif
}

/**
 * @brief This function set kmc 00 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmc00WriteEnable(BOOL bEnable)
{
	//reg_kmc_00_write_enable
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_dma_wr_ctrl_RBUS memc_mc_dma_mc_lf_dma_wr_ctrl_reg;
	memc_mc_dma_mc_lf_dma_wr_ctrl_reg.regValue= rtd_inl(MC_DMA_MC_LF_DMA_WR_Ctrl_reg);
	memc_mc_dma_mc_lf_dma_wr_ctrl_reg.dma_enable=(unsigned int)bEnable;
	rtd_outl(MC_DMA_MC_LF_DMA_WR_Ctrl_reg,memc_mc_dma_mc_lf_dma_wr_ctrl_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_LF_WDMA_MEND1_reg,20,20,(unsigned int)bEnable);
	#endif
}

/**
 * @brief This function set kmc 01 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
#define	Kmc01WriteEnable_reg	0xb8099478
VOID HAL_DM_SetKmc01WriteEnable(BOOL bEnable)
{
	//reg_kmc_01_write_enable
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_dma_wr_ctrl_RBUS memc_mc_dma_mc_hf_dma_wr_ctrl_reg;
	memc_mc_dma_mc_hf_dma_wr_ctrl_reg.regValue= rtd_inl(MC_DMA_MC_HF_DMA_WR_Ctrl_reg);
	memc_mc_dma_mc_hf_dma_wr_ctrl_reg.dma_enable=(unsigned int)bEnable;
	rtd_outl(MC_DMA_MC_HF_DMA_WR_Ctrl_reg,memc_mc_dma_mc_hf_dma_wr_ctrl_reg.regValue);
	#else
	WriteRegister(Kmc01WriteEnable_reg,20,20,(unsigned int)bEnable);
	#endif
}

/**
 * @brief This function set kmc 04 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmc04ReadEnable(BOOL bEnable)
{
	//reg_kmc_04_read_enable
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_dma_wr_ctrl_RBUS memc_mc_dma_mc_hf_dma_wr_ctrl_reg;
	memc_mc_dma_mc_hf_dma_wr_ctrl_reg.regValue= rtd_inl(MC_DMA_MC_LF_I_DMA_RD_Ctrl_reg);
	memc_mc_dma_mc_hf_dma_wr_ctrl_reg.dma_enable=(unsigned int)bEnable;
	rtd_outl(MC_DMA_MC_LF_I_DMA_RD_Ctrl_reg,memc_mc_dma_mc_hf_dma_wr_ctrl_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_WDMA_MSTART3_reg,20,20,(unsigned int)bEnable);
	#endif
}

/**
 * @brief This function set kmc 05 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmc05ReadEnable(BOOL bEnable)
{
	//reg_kmc_05_read_enable
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_i_dma_rd_ctrl_RBUS memc_mc_dma_mc_hf_i_dma_rd_ctrl_reg;
	memc_mc_dma_mc_hf_i_dma_rd_ctrl_reg.regValue= rtd_inl(MC_DMA_MC_HF_I_DMA_RD_Ctrl_reg);
	memc_mc_dma_mc_hf_i_dma_rd_ctrl_reg.dma_enable=(unsigned int)bEnable;
	rtd_outl(MC_DMA_MC_HF_I_DMA_RD_Ctrl_reg,memc_mc_dma_mc_hf_i_dma_rd_ctrl_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_WDMA_MSTART5_reg,20,20,(unsigned int)bEnable);
	#endif
}

/**
 * @brief This function set kmc 06 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmc06ReadEnable(BOOL bEnable)
{
	//reg_kmc_06_read_enable
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_lf_p_dma_rd_ctrl_RBUS memc_mc_dma_mc_lf_p_dma_rd_ctrl_reg;
	memc_mc_dma_mc_lf_p_dma_rd_ctrl_reg.regValue= rtd_inl(MC_DMA_MC_LF_P_DMA_RD_Ctrl_reg);
	memc_mc_dma_mc_lf_p_dma_rd_ctrl_reg.dma_enable=(unsigned int)bEnable;
	rtd_outl(MC_DMA_MC_LF_P_DMA_RD_Ctrl_reg,memc_mc_dma_mc_lf_p_dma_rd_ctrl_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_WDMA_MSTART7_reg,20,20,(unsigned int)bEnable);
	#endif
}

/**
 * @brief This function set kmc 07 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmc07ReadEnable(BOOL bEnable)
{
	//reg_kmc_07_read_enable
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	mc_dma_mc_hf_p_dma_rd_ctrl_RBUS memc_mc_dma_mc_hf_p_dma_rd_ctrl_reg;
	memc_mc_dma_mc_hf_p_dma_rd_ctrl_reg.regValue= rtd_inl(MC_DMA_MC_HF_P_DMA_RD_Ctrl_reg);
	memc_mc_dma_mc_hf_p_dma_rd_ctrl_reg.dma_enable=(unsigned int)bEnable;
	rtd_outl(MC_DMA_MC_HF_P_DMA_RD_Ctrl_reg,memc_mc_dma_mc_hf_p_dma_rd_ctrl_reg.regValue);
	#else
	WriteRegister(MC_DMA_MC_HF_WDMA_MEND1_reg,20,20,(unsigned int)bEnable);
	#endif
}

/**
 * @brief This function get kme 00 start address 0
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKme00StartAddress0(VOID)
{
	unsigned int u32Val = 0;
	//reg_kme_00_start_address0
	ReadRegister(KME_DM_TOP0_KME_00_START_ADDRESS0_reg,0,31,&u32Val);
	return u32Val;
}

/**
 * @brief This function get kme 00 start address 1
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKme00StartAddress1(VOID)
{
	unsigned int u32Val = 0;
	//reg_kme_00_start_address0
	ReadRegister(KME_DM_TOP0_KME_00_START_ADDRESS1_reg,0,31,&u32Val);
	return u32Val;
}

/**
 * @brief This function get kme 00 start address 2
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKme00StartAddress2(VOID)
{
	unsigned int u32Val = 0;
	//reg_kme_00_start_address0
	ReadRegister(KME_DM_TOP0_KME_00_START_ADDRESS2_reg,0,31,&u32Val);
	return u32Val;
}

/**
 * @brief This function get kme 00 start address 3
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKme00StartAddress3(VOID)
{
	unsigned int u32Val = 0;
	//reg_kme_00_start_address0
	ReadRegister(KME_DM_TOP0_KME_00_START_ADDRESS3_reg,0,31,&u32Val);
	return u32Val;
}

/**
 * @brief This function get kme 00 start address 4
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKme00StartAddress4(VOID)
{
	unsigned int u32Val = 0;
	//reg_kme_00_start_address0
	ReadRegister(KME_DM_TOP0_KME_00_START_ADDRESS4_reg,0,31,&u32Val);
	return u32Val;
}

/**
 * @brief This function get kme 00 start address 5
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKme00StartAddress5(VOID)
{
	unsigned int u32Val = 0;
	//reg_kme_00_start_address0
	ReadRegister(KME_DM_TOP0_KME_00_START_ADDRESS5_reg,0,31,&u32Val);
	return u32Val;
}

/**
 * @brief This function get kme 00 line offset address
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKme00LineOffsetAddress(VOID)
{
	unsigned int u32Val = 0;
	//reg_kme_00_line_offset_addr
	ReadRegister(KME_DM_TOP0_KME_00_LINE_OFFSET_ADDR_reg,0,31,&u32Val);	
	return u32Val;
}

/**
 * @brief This function set kme 00 end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00EndAddress0(unsigned int u32Address)
{
	//reg_kme_00_end_address0
	WriteRegister(KME_DM_TOP0_KME_00_END_ADDRESS0_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00EndAddress1(unsigned int u32Address)
{
	//reg_kme_00_end_address0
	WriteRegister(KME_DM_TOP0_KME_00_END_ADDRESS1_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 end address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00EndAddress2(unsigned int u32Address)
{
	//reg_kme_00_end_address0
	WriteRegister(KME_DM_TOP0_KME_00_END_ADDRESS2_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 end address 3
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00EndAddress3(unsigned int u32Address)
{
	//reg_kme_00_end_address0
	WriteRegister(KME_DM_TOP0_KME_00_END_ADDRESS3_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 end address 4
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00EndAddress4(unsigned int u32Address)
{
	//reg_kme_00_end_address0
	WriteRegister(KME_DM_TOP0_KME_00_END_ADDRESS4_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 end address 5
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00EndAddress5(unsigned int u32Address)
{
	//reg_kme_00_end_address0
	WriteRegister(KME_DM_TOP0_KME_00_END_ADDRESS5_reg,0,31,u32Address);	
}

#if 1  // K4Lp
/**
 * @brief This function set kme 00 end address 6
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00EndAddress6(unsigned int u32Address)
{
	//reg_kme_00_end_address6
	WriteRegister(KME_DM_TOP2_KME_00_END_ADDRESS6_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 end address 7
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00EndAddress7(unsigned int u32Address)
{
	//reg_kme_00_end_address7
	WriteRegister(KME_DM_TOP2_KME_00_END_ADDRESS7_reg,0,31,u32Address);	
}
#endif

/**
 * @brief This function set kme 08 end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme08EndAddress0(unsigned int u32Address)
{
	//reg_kme_08_end_address0
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_08_reg, 0, 31, u32Address);	
}

/**
 * @brief This function set kme 08 end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme08EndAddress1(unsigned int u32Address)
{
	//reg_kme_08_end_address1
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_0C_reg, 0, 31, u32Address);	
}

/**
 * @brief This function set kme 10 end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme10EndAddress0(unsigned int u32Address)
{
	//reg_kme_10_end_address0
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_2C_reg, 0, 31, u32Address);	
}

/**
 * @brief This function set kme 10 end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme10EndAddress1(unsigned int u32Address)
{
	//reg_kme_10_end_address1
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_30_reg, 0, 31, u32Address);	
}

/**
 * @brief This function get kme 12 start address 0
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKme12StartAddress0(VOID)
{
	unsigned int u32Val = 0;
	//reg_kme_00_start_address0
	ReadRegister(KME_DM_TOP1_KME_DM_TOP1_48_reg,0,31,&u32Val);
	return u32Val;
}

/**
 * @brief This function get kme 12 start address 1
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKme12StartAddress1(VOID)
{
	unsigned int u32Val = 0;
	//reg_kme_00_start_address0
	ReadRegister(KME_DM_TOP1_KME_DM_TOP1_4C_reg,0,31,&u32Val);
	return u32Val;
}

/**
 * @brief This function get kme 12 line offset address
 * @param [in] VOID
 * @retval Address
*/
unsigned int HAL_DM_GetKme12LineOffsetAddress(VOID)
{
	unsigned int u32Val = 0;
	//reg_kme_00_line_offset_addr
	ReadRegister(KME_DM_TOP1_KME_DM_TOP1_58_reg,0,31,&u32Val);	
	return u32Val;
}

/**
 * @brief This function set kme 12 end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme12EndAddress0(unsigned int u32Address)
{
	//reg_kme_00_end_address0
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_50_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 12 end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme12EndAddress1(unsigned int u32Address)
{
	//reg_kme_00_end_address0
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_54_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 14 end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme14EndAddress0(unsigned int u32Address)
{
	//reg_kme_14_end_address0
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_74_reg, 0, 31, u32Address);	
}

/**
 * @brief This function set kme 14 end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme14EndAddress1(unsigned int u32Address)
{
	//reg_kme_14_end_address1
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_78_reg, 0, 31, u32Address);	
}

/**
 * @brief This function set kme 00 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme00WriteEnable(BOOL bEnable)
{
	//reg_kme_00_write_enable
	WriteRegister(KME_DM_TOP0_KME_00_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 01 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme01ReadEnable(BOOL bEnable)
{
	// reg_kme_01_read_enable
	WriteRegister(KME_DM_TOP0_KME_01_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 02 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme02ReadEnable(BOOL bEnable)
{
	// reg_kme_02_read_enable
	WriteRegister(KME_DM_TOP0_KME_02_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 03 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme03ReadEnable(BOOL bEnable)
{
	// reg_kme_03_read_enable
	WriteRegister(KME_DM_TOP0_KME_03_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 04 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme04WriteEnable(BOOL bEnable)
{
	//reg_kme_04_write_enable
	WriteRegister(KME_DM_TOP0_KME_04_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 05 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme05WriteEnable(BOOL bEnable)
{
	//reg_kme_05_write_enable
	WriteRegister(KME_DM_TOP0_KME_05_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 06 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme06ReadEnable(BOOL bEnable)
{
	// reg_kme_06_read_enable
	WriteRegister(KME_DM_TOP0_KME06AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 07 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme07ReadEnable(BOOL bEnable)
{
	// reg_kme_07_read_enable
	WriteRegister(KME_DM_TOP0_KME_07_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 08 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme08WriteEnable(BOOL bEnable)
{
	// reg_kme_08_write_enable
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_14_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 09 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme09ReadEnable(BOOL bEnable)
{
	// reg_kme_09_read_enable
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_1C_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 10 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme10WriteEnable(BOOL bEnable)
{
	// reg_kme_10_write_enable
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_38_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 11 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme11ReadEnable(BOOL bEnable)
{
	// reg_kme_11_read_enable
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_40_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 12 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme12WriteEnable(BOOL bEnable)
{
	// reg_kme_12_write_enable
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_5C_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 13 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme13ReadEnable(BOOL bEnable)
{
	// reg_kme_13_read_enable
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_64_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 14 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme14WriteEnable(BOOL bEnable)
{
	// reg_kme_14_write_enable
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_80_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 15 read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKme15ReadEnable(BOOL bEnable)
{
	// reg_kme_15_read_enable
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_88_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme input 3d enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmeInput3DEnable(BOOL bEnable)
{
	//reg_kme_input_3d_enable
	WriteRegister(KME_DM_TOP0_KME_SOURCE_MODE_reg,0,0,(unsigned int)bEnable);	
}

/**
 * @brief This function set kme input 3d Mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_DM_SetKmeInput3DMode(unsigned int u32Mode)
{
	//reg_kme_input_3d_mode
 	WriteRegister(KME_DM_TOP0_KME_SOURCE_MODE_reg,1,4,u32Mode);		
}

/**
 * @brief This function set kme output 3d enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmeOutput3DEnable(BOOL bEnable)
{
	//reg_kme_output_3d_enable
	WriteRegister(KME_DM_TOP0_KME_SOURCE_MODE_reg,5,5,(unsigned int)bEnable);
}

/**
 * @brief This function set kme output 3d Mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_DM_SetKmeOutput3DMode(unsigned int u32Mode)
{
	//reg_kme_output_3d_mode
 	WriteRegister(KME_DM_TOP0_KME_SOURCE_MODE_reg,6,7,u32Mode);
}


/**
 * @brief This function set kme ME H/V active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_DM_SetKmeMEActive(unsigned int u32Hactive, unsigned int u32Vactive)
{
	// reg_kme_me_hactive
	WriteRegister(KME_DM_TOP0_KME_ME_RESOLUTION_reg,0,11,u32Hactive);
	// reg_kme_me_vactive
	WriteRegister(KME_DM_TOP0_KME_ME_RESOLUTION_reg,12,23,u32Vactive);
}

/**
 * @brief This function set kme MV H/V active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_DM_SetKmeMVActive(unsigned int u32Hactive, unsigned int u32Vactive)
{
	// reg_kme_mv_hactive
	WriteRegister(KME_DM_TOP0_KME_MV_RESOLUTION_reg,0,11,u32Hactive);
	// reg_kme_mv_vactive
	WriteRegister(KME_DM_TOP0_KME_MV_RESOLUTION_reg,12,23,u32Vactive);
}

/**
 * @brief This function set kme me hnum
 * @param [in] unsigned int u32Hnum
 * @retval VOID
*/
VOID HAL_DM_SetKmeMEHNum(unsigned int u32Hnum)
{
	// reg_kme_me_hnum
	WriteRegister(KME_DM_TOP0_KME_ME_HNUM_reg,0,7,u32Hnum);
}

/**
 * @brief This function set kme mv hnum
 * @param [in] unsigned int u32Hnum
 * @retval VOID
*/
VOID HAL_DM_SetKmeMVHNum(unsigned int u32Hnum)
{
	// reg_kme_mv_hnum
	WriteRegister(KME_DM_TOP0_KME_ME_HNUM_reg,8,15,u32Hnum);
}

/**
 * @brief This function set kme ppi hnum
 * @param [in] unsigned int u32Hnum
 * @retval VOID
*/
VOID HAL_DM_SetKmePPIHNum(unsigned int u32Hnum)
{
	// reg_kme_ppi_hnum
	WriteRegister(KME_DM_TOP0_KME_ME_HNUM_reg,16,23,u32Hnum);
}

/**
 * @brief This function set kme vflip enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmeVflipEnable(BOOL bEnable)
{
	//reg_kme_vflip_enable
#if 1  // K4Lp
	WriteRegister(KME_DM_TOP0_KME_SOURCE_MODE_reg,8,8,(unsigned int)bEnable);	
#else
	WriteRegister(KME_TOP_KME_TOP_04_reg,31,31,(unsigned int)bEnable);	
#endif
}

/**
 * @brief This function set kme logo vflip enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmeLogoVflipEnable(BOOL bEnable)
{
	//reg_km_logo_vflip_en
	WriteRegister(KME_LOGO0_KME_LOGO0_F0_reg,16,16,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 00 start address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00StartAddress0(unsigned int u32Address)
{
	//reg_kme_00_start_address0
	WriteRegister(KME_DM_TOP0_KME_00_START_ADDRESS0_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 start address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00StartAddress1(unsigned int u32Address)
{
	//reg_kme_00_start_address1
	WriteRegister(KME_DM_TOP0_KME_00_START_ADDRESS1_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 start address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00StartAddress2(unsigned int u32Address)
{
	//reg_kme_00_start_address2
	WriteRegister(KME_DM_TOP0_KME_00_START_ADDRESS2_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 start address 3
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00StartAddress3(unsigned int u32Address)
{
	//reg_kme_00_start_address3
	WriteRegister(KME_DM_TOP0_KME_00_START_ADDRESS3_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 start address 4
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00StartAddress4(unsigned int u32Address)
{
	//reg_kme_00_start_address4
	WriteRegister(KME_DM_TOP0_KME_00_START_ADDRESS4_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 start address 5
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00StartAddress5(unsigned int u32Address)
{
	//reg_kme_00_start_address5
	WriteRegister(KME_DM_TOP0_KME_00_START_ADDRESS5_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 start address 6
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00StartAddress6(unsigned int u32Address)
{
	//reg_kme_00_start_address6
	WriteRegister(KME_DM_TOP2_KME_00_START_ADDRESS6_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 start address 7
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00StartAddress7(unsigned int u32Address)
{
	//reg_kme_00_start_address7
	WriteRegister(KME_DM_TOP2_KME_00_START_ADDRESS7_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 04 start address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme04StartAddress0(unsigned int u32Address)
{
	//reg_kme_04_start_address0
	WriteRegister(KME_DM_TOP0_KME_04_START_ADDRESS0_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 04 start address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme04StartAddress1(unsigned int u32Address)
{
	//reg_kme_04_start_address1
	WriteRegister(KME_DM_TOP0_KME_04_START_ADDRESS1_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 05 start address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme05StartAddress0(unsigned int u32Address)
{
	//reg_kme_05_start_address0
	WriteRegister(KME_DM_TOP0_KME_05_START_ADDRESS0_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 05 start address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme05StartAddress1(unsigned int u32Address)
{
	//reg_kme_05_start_address1
	WriteRegister(KME_DM_TOP0_KME_05_START_ADDRESS1_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 08 start address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme08StartAddress0(unsigned int u32Address)
{
	//reg_kme_08_start_address0
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_00_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 08 start address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme08StartAddress1(unsigned int u32Address)
{
	//reg_kme_08_start_address1
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_04_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 10 start address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme10StartAddress0(unsigned int u32Address)
{
	//reg_kme_10_start_address0
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_24_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 10 start address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme10StartAddress1(unsigned int u32Address)
{
	//reg_kme_10_start_address1
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_28_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 12 start address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme12StartAddress0(unsigned int u32Address)
{
	//reg_kme_12_start_address0
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_48_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 12 start address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme12StartAddress1(unsigned int u32Address)
{
	//reg_kme_12_start_address1
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_4C_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 14 start address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme14StartAddress0(unsigned int u32Address)
{
	//reg_kme_14_start_address0
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_6C_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 14 start address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme14StartAddress1(unsigned int u32Address)
{
	//reg_kme_14_start_address1
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_70_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 01 start address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv01StartAddress0(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV01_START_ADDRESS0_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 02 start address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv02StartAddress0(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV02_START_ADDRESS11_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 04 start address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv04StartAddress0(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV04_START_ADDRESS0_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 04 start address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv04StartAddress1(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV04_START_ADDRESS1_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 04 start address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv04StartAddress2(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP0_MV04_START_ADDRESS2_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 05 start address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv05StartAddress0(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV05_START_ADDRESS0_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 05 start address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv05StartAddress1(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV05_START_ADDRESS1_reg,0,31,u32Address);	
}

#ifdef LBME2_DM_BUF
/**
 * @brief This function set kmv 12 start address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12StartAddress0(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_start_address0_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address0_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address0_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 start address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12StartAddress1(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_start_address1_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address1_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address1_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 start address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12StartAddress2(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_start_address2_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address2_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address2_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 start address 3
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12StartAddress3(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_start_address3_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address3_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address3_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 start address 4
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12StartAddress4(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_start_address4_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address4_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address4_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 start address 5
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12StartAddress5(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_start_address5_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address5_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address5_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 start address 6
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12StartAddress6(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_start_address6_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address6_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address6_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 start address 7
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12StartAddress7(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_start_address7_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address7_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_start_address7_BITEND no mat,u32Address);	
}
#endif

/**
 * @brief This function set kmv 01 end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv01EndAddress0(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV01_END_ADDRESS0_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 02 end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv02EndAddress0(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV02_END_ADDRESS0_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 04 end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv04EndAddress0(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV04_END_ADDRESS0_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 04 end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv04EndAddress1(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV04_END_ADDRESS1_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 04 end address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv04EndAddress2(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP0_MV04_END_ADDRESS2_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 05 end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv05EndAddress0(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV05_END_ADDRESS0_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 05 end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv05EndAddress1(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV05_END_ADDRESS1_reg,0,31,u32Address);	
}

#ifdef LBME2_DM_BUF
/**
 * @brief This function set kmv 12 end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12EndAddress0(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_end_address0_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address0_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address0_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12EndAddress1(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_end_address1_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address1_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address1_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 end address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12EndAddress2(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_end_address2_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address2_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address2_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 end address 3
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12EndAddress3(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_end_address3_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address3_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address3_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 end address 4
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12EndAddress4(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_end_address4_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address4_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address4_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 end address 5
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12EndAddress5(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_end_address5_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address5_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address5_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 end address 6
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12EndAddress6(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_end_address6_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address6_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address6_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 end address 7
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12EndAddress7(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_end_address7_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address7_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_end_address7_BITEND no mat,u32Address);	
}
#endif

/**
 * @brief This function set kmv 01 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv01LineOffsetAddress(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV_01_LINE_OFFSET_ADDR_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 01 LR offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv01LROffsetAddress(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV01_LR_OFFSET_ADDR_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 02 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv02LineOffsetAddress(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV_02_LINE_OFFSET_ADDR_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 02 LR offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv02LROffsetAddress(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV02_LR_OFFSET_ADDR_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 04 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv04LineOffsetAddress(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV_04_LINE_OFFSET_ADDR_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 04 LR offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv04LROffsetAddress(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV04_LR_OFFSET_ADDR_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 05 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv05LineOffsetAddress(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV_05_LINE_OFFSET_ADDR_reg,0,31,u32Address);	
}

/**
 * @brief This function set kmv 05 LR offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv05LROffsetAddress(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP2_MV05_LR_OFFSET_ADDR_reg,0,31,u32Address);	
}

#ifdef LBME2_DM_BUF
/**
 * @brief This function set kmv 12 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12LineOffsetAddress(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_line_offset_addr_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_line_offset_addr_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_line_offset_addr_BITEND no mat,u32Address);	
}

/**
 * @brief This function set kmv 12 LR offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKmv12LROffsetAddress(unsigned int u32Address)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_lr_offset_addr_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_lr_offset_addr_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_lr_offset_addr_BITEND no mat,u32Address);	
}
#endif


/**
 * @brief This function set ME1RPT_ppfv start address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetME1RPT_ppfv_StartAddress0(unsigned int u32Address)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_ME1RPT_ppfv_start_address0
	me_share_dma_me1_wdma0_mstart0_RBUS me_share_dma_me1_wdma0_mstart0_reg;
	me_share_dma_me1_wdma0_mstart0_reg.regValue= rtd_inl(ME_SHARE_DMA_ME1_WDMA0_MSTART0_reg);
	me_share_dma_me1_wdma0_mstart0_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_ME1_WDMA0_MSTART0_reg,me_share_dma_me1_wdma0_mstart0_reg.regValue);
	#endif
}
/**
 * @brief This function set ME1RPT_ppfv end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetME1RPT_ppfv_EndAddress0(unsigned int u32Address)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_ME1RPT_ppfv_end_address0
	me_share_dma_me1_wdma0_mend0_RBUS me_share_dma_me1_wdma0_mend0_reg;
	me_share_dma_me1_wdma0_mend0_reg.regValue= rtd_inl(ME_SHARE_DMA_ME1_WDMA0_MEND0_reg);
	me_share_dma_me1_wdma0_mend0_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_ME1_WDMA0_MEND0_reg,me_share_dma_me1_wdma0_mend0_reg.regValue);
	#endif
}
/**
 * @brief This function set ME1RPT_ppfv start address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetME1RPT_ppfv_StartAddress1(unsigned int u32Address)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_ME1RPT_ppfv_start_address1
	me_share_dma_me1_wdma0_mstart1_RBUS me_share_dma_me1_wdma0_mstart1_reg;
	me_share_dma_me1_wdma0_mstart1_reg.regValue= rtd_inl(ME_SHARE_DMA_ME1_WDMA0_MSTART1_reg);
	me_share_dma_me1_wdma0_mstart1_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_ME1_WDMA0_MSTART1_reg,me_share_dma_me1_wdma0_mstart1_reg.regValue);
	#endif
}
/**
 * @brief This function set ME1RPT_ppfv end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetME1RPT_ppfv_EndAddress1(unsigned int u32Address)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_ME1RPT_ppfv_end_address1
	me_share_dma_me1_wdma0_mend1_RBUS me_share_dma_me1_wdma0_mend1_reg;
	me_share_dma_me1_wdma0_mend1_reg.regValue= rtd_inl(ME_SHARE_DMA_ME1_WDMA0_MEND1_reg);
	me_share_dma_me1_wdma0_mend1_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_ME1_WDMA0_MEND1_reg,me_share_dma_me1_wdma0_mend1_reg.regValue);
	#endif
}
/**
 * @brief This function set ME1RPT_pfv start address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetME1RPT_pfv_StartAddress0(unsigned int u32Address)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_ME1RPT_pfv_start_address0
	me_share_dma_me1_wdma1_mstart0_RBUS me_share_dma_me1_wdma1_mstart0_reg;
	me_share_dma_me1_wdma1_mstart0_reg.regValue= rtd_inl(ME_SHARE_DMA_ME1_WDMA1_MSTART0_reg);
	me_share_dma_me1_wdma1_mstart0_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_MSTART0_reg,me_share_dma_me1_wdma1_mstart0_reg.regValue);
	#endif
}
/**
 * @brief This function set ME1RPT_pfv end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetME1RPT_pfv_EndAddress0(unsigned int u32Address)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_ME1RPT_pfv_end_address0
	me_share_dma_me1_wdma1_mend0_RBUS me_share_dma_me1_wdma1_mend0_reg;
	me_share_dma_me1_wdma1_mend0_reg.regValue= rtd_inl(ME_SHARE_DMA_ME1_WDMA1_MEND0_reg);
	me_share_dma_me1_wdma1_mend0_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_MEND0_reg,me_share_dma_me1_wdma1_mend0_reg.regValue);
	#endif
}
/**
 * @brief This function set ME1RPT_pfv start address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetME1RPT_pfv_StartAddress1(unsigned int u32Address)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_ME1RPT_pfv_start_address1
	me_share_dma_me1_wdma1_mstart1_RBUS me_share_dma_me1_wdma1_mstart1_reg;
	me_share_dma_me1_wdma1_mstart1_reg.regValue= rtd_inl(ME_SHARE_DMA_ME1_WDMA1_MSTART1_reg);
	me_share_dma_me1_wdma1_mstart1_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_MSTART1_reg,me_share_dma_me1_wdma1_mstart1_reg.regValue);
	#endif
}
/**
 * @brief This function set ME1RPT_pfv end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetME1RPT_pfv_EndAddress1(unsigned int u32Address)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_ME1RPT_pfv_end_address1
	me_share_dma_me1_wdma1_mend1_RBUS me_share_dma_me1_wdma1_mend1_reg;
	me_share_dma_me1_wdma1_mend1_reg.regValue= rtd_inl(ME_SHARE_DMA_ME1_WDMA1_MEND1_reg);
	me_share_dma_me1_wdma1_mend1_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_MEND1_reg,me_share_dma_me1_wdma1_mend1_reg.regValue);
	#endif
}
/**
 * @brief This function set ME1RPT_pfv start address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetME1RPT_pfv_StartAddress2(unsigned int u32Address)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_ME1RPT_pfv_start_address2
	me_share_dma_me1_wdma1_mstart2_RBUS me_share_dma_me1_wdma1_mstart2_reg;
	me_share_dma_me1_wdma1_mstart2_reg.regValue= rtd_inl(ME_SHARE_DMA_ME1_WDMA1_MSTART2_reg);
	me_share_dma_me1_wdma1_mstart2_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_MSTART2_reg,me_share_dma_me1_wdma1_mstart2_reg.regValue);	
	#endif
}
/**
 * @brief This function set ME1RPT_pfv end address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetME1RPT_pfv_EndAddress2(unsigned int u32Address)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_ME1RPT_pfv_end_address2
	me_share_dma_me1_wdma1_mend2_RBUS me_share_dma_me1_wdma1_mend2_reg;
	me_share_dma_me1_wdma1_mend2_reg.regValue= rtd_inl(ME_SHARE_DMA_ME1_WDMA1_MEND2_reg);
	me_share_dma_me1_wdma1_mend2_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_MEND2_reg,me_share_dma_me1_wdma1_mend2_reg.regValue);	
	#endif
}

/**
 * @brief This function set ME1RPT_ppfv line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetME1RPT_ppfv_LineOffsetAddress(unsigned int u32Address)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_kme_00_line_offset_addr
	me_share_dma_me1_wdma0_lstep_RBUS me_share_dma_me1_wdma0_lstep_reg;
	me_share_dma_me1_wdma0_lstep_reg.regValue= rtd_inl(ME_SHARE_DMA_ME1_WDMA0_LSTEP_reg);
	me_share_dma_me1_wdma0_lstep_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_ME1_WDMA0_LSTEP_reg,me_share_dma_me1_wdma0_lstep_reg.regValue);
	#endif
}
/**
 * @brief This function set ME1RPT_pfv line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetME1RPT_pfv_LineOffsetAddress(unsigned int u32Address)
{
	#if (IC_K5LP || IC_K6LP || IC_K7LP || IC_K8LP)
	//reg_kme_00_line_offset_addr
	me_share_dma_me1_wdma1_lstep_RBUS me_share_dma_me1_wdma1_lstep_reg;
	me_share_dma_me1_wdma1_lstep_reg.regValue= rtd_inl(ME_SHARE_DMA_ME1_WDMA1_LSTEP_reg);
	me_share_dma_me1_wdma1_lstep_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_ME1_WDMA1_LSTEP_reg,me_share_dma_me1_wdma1_lstep_reg.regValue);
	#endif
}


/**
 * @brief This function set IPME start address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEStartAddress0(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address0
	me_share_dma_ipme_wdma_mstart0_RBUS me_share_dma_ipme_wdma_mstart0_reg;
	me_share_dma_ipme_wdma_mstart0_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART0_reg);
	me_share_dma_ipme_wdma_mstart0_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART0_reg,me_share_dma_ipme_wdma_mstart0_reg.regValue);
	#endif

}
/**
 * @brief This function set IPME start address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEStartAddress1(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address1
	me_share_dma_ipme_wdma_mstart1_RBUS me_share_dma_ipme_wdma_mstart1_reg;
	me_share_dma_ipme_wdma_mstart1_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART1_reg);
	me_share_dma_ipme_wdma_mstart1_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART1_reg,me_share_dma_ipme_wdma_mstart1_reg.regValue);
	#endif

}

/**
 * @brief This function set IPME start address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEStartAddress2(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address2
	me_share_dma_ipme_wdma_mstart2_RBUS me_share_dma_ipme_wdma_mstart2_reg;
	me_share_dma_ipme_wdma_mstart2_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART2_reg);
	me_share_dma_ipme_wdma_mstart2_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART2_reg,me_share_dma_ipme_wdma_mstart2_reg.regValue);
	#endif

}

/**
 * @brief This function set IPME start address 3
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEStartAddress3(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address3
	me_share_dma_ipme_wdma_mstart3_RBUS me_share_dma_ipme_wdma_mstart3_reg;
	me_share_dma_ipme_wdma_mstart3_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART3_reg);
	me_share_dma_ipme_wdma_mstart3_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART3_reg,me_share_dma_ipme_wdma_mstart3_reg.regValue);
	#endif

}

/**
 * @brief This function set IPME start address 4
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEStartAddress4(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address4
	me_share_dma_ipme_wdma_mstart4_RBUS me_share_dma_ipme_wdma_mstart4_reg;
	me_share_dma_ipme_wdma_mstart4_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART4_reg);
	me_share_dma_ipme_wdma_mstart4_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART4_reg,me_share_dma_ipme_wdma_mstart4_reg.regValue);
	#endif

}

/**
 * @brief This function set IPME start address 5
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEStartAddress5(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address5
	me_share_dma_ipme_wdma_mstart5_RBUS me_share_dma_ipme_wdma_mstart5_reg;
	me_share_dma_ipme_wdma_mstart5_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART5_reg);
	me_share_dma_ipme_wdma_mstart5_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART5_reg,me_share_dma_ipme_wdma_mstart5_reg.regValue);
	#endif
}

/**
 * @brief This function set IPME start address 6
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEStartAddress6(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address6
	me_share_dma_ipme_wdma_mstart6_RBUS me_share_dma_ipme_wdma_mstart6_reg;
	me_share_dma_ipme_wdma_mstart6_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART6_reg);
	me_share_dma_ipme_wdma_mstart6_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART6_reg,me_share_dma_ipme_wdma_mstart6_reg.regValue);
	#endif
}

/**
 * @brief This function set IPME start address 7
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEStartAddress7(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address7
	me_share_dma_ipme_wdma_mstart7_RBUS me_share_dma_ipme_wdma_mstart7_reg;
	me_share_dma_ipme_wdma_mstart7_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MSTART7_reg);
	me_share_dma_ipme_wdma_mstart7_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MSTART7_reg,me_share_dma_ipme_wdma_mstart7_reg.regValue);	
	#endif
}
/**
 * @brief This function set IPME end address 0
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEEndAddress0(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address0
	me_share_dma_ipme_wdma_mend0_RBUS me_share_dma_ipme_wdma_mend0_reg;
	me_share_dma_ipme_wdma_mend0_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND0_reg);
	me_share_dma_ipme_wdma_mend0_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND0_reg,me_share_dma_ipme_wdma_mend0_reg.regValue);
	#endif
}
/**
 * @brief This function set IPME end address 1
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEEndAddress1(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address1
	me_share_dma_ipme_wdma_mend1_RBUS me_share_dma_ipme_wdma_mend1_reg;
	me_share_dma_ipme_wdma_mend1_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND1_reg);
	me_share_dma_ipme_wdma_mend1_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND1_reg,me_share_dma_ipme_wdma_mend1_reg.regValue);
	#endif
}

/**
 * @brief This function set IPME end address 2
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEEndAddress2(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address2
	me_share_dma_ipme_wdma_mend2_RBUS me_share_dma_ipme_wdma_mend2_reg;
	me_share_dma_ipme_wdma_mend2_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND2_reg);
	me_share_dma_ipme_wdma_mend2_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND2_reg,me_share_dma_ipme_wdma_mend2_reg.regValue);
	#endif
}

/**
 * @brief This function set IPME end address 3
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEEndAddress3(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address3
	me_share_dma_ipme_wdma_mend3_RBUS me_share_dma_ipme_wdma_mend3_reg;
	me_share_dma_ipme_wdma_mend3_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND3_reg);
	me_share_dma_ipme_wdma_mend3_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND3_reg,me_share_dma_ipme_wdma_mend3_reg.regValue);
	#endif
}

/**
 * @brief This function set IPME end address 4
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEEndAddress4(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address4
	me_share_dma_ipme_wdma_mend4_RBUS me_share_dma_ipme_wdma_mend4_reg;
	me_share_dma_ipme_wdma_mend4_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND4_reg);
	me_share_dma_ipme_wdma_mend4_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND4_reg,me_share_dma_ipme_wdma_mend4_reg.regValue);
	#endif
}

/**
 * @brief This function set IPME end address 5
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEEndAddress5(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address5
	me_share_dma_ipme_wdma_mend5_RBUS me_share_dma_ipme_wdma_mend5_reg;
	me_share_dma_ipme_wdma_mend5_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND5_reg);
	me_share_dma_ipme_wdma_mend5_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND5_reg,me_share_dma_ipme_wdma_mend5_reg.regValue);
	#endif
}

/**
 * @brief This function set IPME end address 6
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEEndAddress6(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address6
	me_share_dma_ipme_wdma_mend6_RBUS me_share_dma_ipme_wdma_mend6_reg;
	me_share_dma_ipme_wdma_mend6_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND6_reg);
	me_share_dma_ipme_wdma_mend6_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND6_reg,me_share_dma_ipme_wdma_mend6_reg.regValue);
	#endif
}

/**
 * @brief This function set IPME end address 7
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMEEndAddress7(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_start_address7
	me_share_dma_ipme_wdma_mend7_RBUS me_share_dma_ipme_wdma_mend7_reg;
	me_share_dma_ipme_wdma_mend7_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_MEND7_reg);
	me_share_dma_ipme_wdma_mend7_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_MEND7_reg,me_share_dma_ipme_wdma_mend7_reg.regValue);	
	#endif
}
/**
 * @brief This function set IPME line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetIPMELineOffsetAddress(unsigned int u32Address)
{
	#if (IC_K5LP)
	//reg_IPME_line_offset_addr
	me_share_dma_ipme_wdma_lstep_RBUS me_share_dma_ipme_wdma_lstep_reg;
	me_share_dma_ipme_wdma_lstep_reg.regValue= rtd_inl(ME_SHARE_DMA_IPME_WDMA_LSTEP_reg);
	me_share_dma_ipme_wdma_lstep_reg.regValue=u32Address;
	rtd_outl(ME_SHARE_DMA_IPME_WDMA_LSTEP_reg,me_share_dma_ipme_wdma_lstep_reg.regValue);
	#endif
}

/**
 * @brief This function set kme 01 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv01WriteEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV01_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 02 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv02WriteEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV02_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 04 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv04WriteEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV04_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 05 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv05WriteEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV05_AGENT_reg,20,20,(unsigned int)bEnable);
}

#ifdef LBME2_DM_BUF
/**
 * @brief This function set kme 12 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv12WriteEnable(BOOL bEnable)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_write_enable_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_write_enable_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_write_enable_BITEND no mat,(unsigned int)bEnable);
}
#endif

/**
 * @brief This function set kme 03 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv03ReadEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV03_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 06 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv06ReadEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV06_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 07 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv07ReadEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV07_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 08 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv08ReadEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV08_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 09 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv09ReadEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV09_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 10 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv10ReadEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV10_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 11 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv11ReadEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV11_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set kme 12 write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmv12ReadEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV12_AGENT_reg,20,20,(unsigned int)bEnable);
}

/**
 * @brief This function set MV info write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetMVInfoWriteEnable(BOOL bEnable)
{
	WriteRegister(MVINFO_DMA_MVINFO_WR_Ctrl_reg, 0, 0, (unsigned int)bEnable);
}

/**
 * @brief This function set MV info read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetMVInfoReadEnable(BOOL bEnable)
{
	WriteRegister(MVINFO_DMA_MVINFO_RD_Ctrl_reg, 0, 0, (unsigned int)bEnable);
}

/**
 * @brief This function set ME share write enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetMEshareWriteEnable(BOOL bEnable)
{
	rtd_pr_memc_notice("[%s]wdma_en=(%x);IP_en=(%x)\r\n",__FUNCTION__, rtd_inl(ME_SHARE_DMA_ME_DMA_WR_Ctrl_reg),rtd_inl(ME_SHARE_DMA_KME_WR_client_en_reg));
	WriteRegister(ME_SHARE_DMA_ME_DMA_WR_Ctrl_reg, 0, 0, (unsigned int)bEnable);
}

/**
 * @brief This function set ME share read enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetMEshareReadEnable(BOOL bEnable)
{
	rtd_pr_memc_notice("[%s]rdma_en=(%x);IP_en=(%x)\r\n",__FUNCTION__, rtd_inl(ME_SHARE_DMA_ME_DMA_RD_Ctrl_reg),rtd_inl(ME_SHARE_DMA_KME_RD_client_en_reg));
	WriteRegister(ME_SHARE_DMA_ME_DMA_RD_Ctrl_reg, 0, 0, (unsigned int)bEnable);
}

/**
 * @brief This function set kmv 01 hnum
 * @param [in] unsigned int u32Hnum
 * @retval VOID
*/
VOID HAL_DM_SetKmv01Hnum(unsigned int u32Hnum)
{
	WriteRegister(KME_DM_TOP2_MV01_AGENT_reg,22,29,u32Hnum);
}

/**
 * @brief This function set kmv 02 hnum
 * @param [in] unsigned int u32Hnum
 * @retval VOID
*/
VOID HAL_DM_SetKmv02Hnum(unsigned int u32Hnum)
{
	WriteRegister(KME_DM_TOP2_MV02_AGENT_reg,22,29,u32Hnum);
}

/**
 * @brief This function set kmv 04 hnum
 * @param [in] unsigned int u32Hnum
 * @retval VOID
*/
VOID HAL_DM_SetKmv04Hnum(unsigned int u32Hnum)
{
	WriteRegister(KME_DM_TOP2_MV04_AGENT_reg,22,29,u32Hnum);
}

/**
 * @brief This function set kmv 05 hnum
 * @param [in] unsigned int u32Hnum
 * @retval VOID
*/
VOID HAL_DM_SetKmv05Hnum(unsigned int u32Hnum)
{
	WriteRegister(KME_DM_TOP2_MV05_AGENT_reg,22,29,u32Hnum);
}

#ifdef LBME2_DM_BUF
/**
 * @brief This function set kmv 12 hnum
 * @param [in] unsigned int u32Hnum
 * @retval VOID
*/
VOID HAL_DM_SetKmv12Hnum(unsigned int u32Hnum)
{
//	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_hnum_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_hnum_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_hnum_BITEND no mat,u32Hnum);
}
#endif

/**
 * @brief This function set kmv 01 Active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_DM_SetKmv01Active(unsigned int u32Hactive, unsigned int u32Vactive)
{
	WriteRegister(KME_DM_TOP2_MV01_RESOLUTION_reg,0,11,u32Hactive);
	WriteRegister(KME_DM_TOP2_MV01_RESOLUTION_reg,12,23,u32Vactive);
}

/**
 * @brief This function set kmv 02 Active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_DM_SetKmv02Active(unsigned int u32Hactive, unsigned int u32Vactive)
{
	WriteRegister(KME_DM_TOP2_MV02_RESOLUTION_reg,0,11,u32Hactive);
	WriteRegister(KME_DM_TOP2_MV02_RESOLUTION_reg,12,23,u32Vactive);
}

/**
 * @brief This function set kmv 04 Active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_DM_SetKmv04Active(unsigned int u32Hactive, unsigned int u32Vactive)
{
	WriteRegister(KME_DM_TOP2_MV04_RESOLUTION_reg,0,11,u32Hactive);
	WriteRegister(KME_DM_TOP2_MV04_RESOLUTION_reg,12,23,u32Vactive);
}

/**
 * @brief This function set kmv 05 Active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_DM_SetKmv05Active(unsigned int u32Hactive, unsigned int u32Vactive)
{
	WriteRegister(KME_DM_TOP2_MV05_RESOLUTION_reg,0,11,u32Hactive);
	WriteRegister(KME_DM_TOP2_MV05_RESOLUTION_reg,12,23,u32Vactive);
}

#ifdef LBME2_DM_BUF
/**
 * @brief This function set kmv 12 Active
 * @param [in] unsigned int u32Hactive
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_DM_SetKmv12Active(unsigned int u32Hactive, unsigned int u32Vactive)
{
	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_hactive_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_hactive_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_hactive_BITEND no mat,u32Hactive);
	WriteRegister(FRC_TOP__KME_DM_TOP2__mv12_vactive_ADDR no mat,FRC_TOP__KME_DM_TOP2__mv12_vactive_BITSTART no mat,FRC_TOP__KME_DM_TOP2__mv12_vactive_BITEND no mat,u32Vactive);
}
#endif

/**
 * @brief This function set kmv input 3d enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmvInput3DEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV_SOURCE_MODE_reg,0,0,(unsigned int)bEnable);
}

/**
 * @brief This function set kmv output 3d enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_DM_SetKmvOutput3DEnable(BOOL bEnable)
{
	WriteRegister(KME_DM_TOP2_MV_SOURCE_MODE_reg,1,1,(unsigned int)bEnable);
}

/**
 * @brief This function set kmv me1 output 3d mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_DM_SetKmvME1Output3DMode(unsigned int u32Mode)
{
	WriteRegister(KME_DM_TOP2_MV_SOURCE_MODE_reg,2,3,u32Mode);
	WriteRegister(KME_DM_TOP2_MV_SOURCE_MODE_reg,4,5,u32Mode);
	WriteRegister(KME_DM_TOP2_MV_SOURCE_MODE_reg,6,7,u32Mode);
}

/**
 * @brief This function set kmv me2 output 3d mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_DM_SetKmvME2Output3DMode(unsigned int u32Mode)
{
	WriteRegister(KME_DM_TOP2_MV_SOURCE_MODE_reg,8,9,u32Mode);
	WriteRegister(KME_DM_TOP2_MV_SOURCE_MODE_reg,10,11,u32Mode);
	WriteRegister(KME_DM_TOP2_MV_SOURCE_MODE_reg,12,13,u32Mode);
	WriteRegister(KME_DM_TOP2_MV_SOURCE_MODE_reg,14,15,u32Mode);
}

/**
 * @brief This function set kme 00 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00LROffsetAddress(unsigned int u32Address)
{
	WriteRegister(KME_DM_TOP0_KME_00_LR_OFFSET_ADDR_reg,0,31,u32Address);	
}

/**
 * @brief This function set kme 00 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme00LineOffsetAddress(unsigned int u32Address)
{
	//reg_kme_00_line_offset_addr
	WriteRegister(KME_DM_TOP0_KME_00_LINE_OFFSET_ADDR_reg,0,31,u32Address);
}

/**
 * @brief This function set kme 08 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme08LineOffsetAddress(unsigned int u32Address)
{
	//reg_kme_08_line_offset_addr
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_10_reg,0,31,u32Address);
}

/**
 * @brief This function set kme 10 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme10LineOffsetAddress(unsigned int u32Address)
{
	//reg_kme_10_line_offset_addr
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_34_reg,0,31,u32Address);
}

/**
 * @brief This function set kme 12 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme12LineOffsetAddress(unsigned int u32Address)
{
	//reg_kme_12_line_offset_addr
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_58_reg,0,31,u32Address);
}

/**
 * @brief This function set kme 14 line offset address
 * @param [in] unsigned int u32Address
 * @retval VOID
*/
VOID HAL_DM_SetKme14LineOffsetAddress(unsigned int u32Address)
{
	//reg_kme_14_line_offset_addr
	WriteRegister(KME_DM_TOP1_KME_DM_TOP1_7C_reg,0,31,u32Address);
}

