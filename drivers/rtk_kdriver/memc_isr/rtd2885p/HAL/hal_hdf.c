/**
 * @file hal_hdf.c
 * @brief This file is for HDF loading.
 * @date October.28.2014
 * @version V0.0.1
 * @author Martin Yan
 * @par Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#include "memc_isr/HAL/hal_hdf.h"
#include "memc_isr/include/hdf.h"
#include "memc_isr/Common/kw_debug.h"
#include "memc_isr/Common/kw_common.h"
#include "memc_isr/MID/mid_mode.h"
#include "memc_isr/Driver/regio.h"
#include "memc_reg_def.h"
#include <memc_isr/scalerMEMC.h>
#include "memc_isr/PQL/PQLPlatformDefs.h"
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include "memc_isr/include/memc_lib.h"

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
	#include <scalercommon/scalerDrvCommon.h>
#endif

#define FILE_HEADER_SIZE		64
#define PAGE_HEADER_SIZE		16
#define PAGE_ADDRESS_OFFSET		4
#define PAGE_SIZE_OFFSET		8
#define MEMC_REG_BASE_OFFSET 0xB8099000
#define PAGE_ID_OFFSET			5
#define CLEARBIT(val, bit_pos) (val & ~(1 << bit_pos))

/**
 * @brief This function load default hdf.
 * @param [in] VOID.
 * @retval VOID
*/
extern unsigned int Get_DISPLAY_REFRESH_RATE(void);
unsigned char HAL_HDF_AddressCheck(unsigned int u32_address)
{
#if 1//#ifdef KID23_TO_WEBOS//k23

	unsigned char ret = _ADDR_CASE_NULL;
	if(u32_address == (MC_DMA_MC_LF_DMA_WR_Rule_check_up_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (MC_DMA_MC_HF_DMA_WR_Rule_check_up_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (MC_DMA_MC_LF_I_DMA_RD_Rule_check_up_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (MC_DMA_MC_HF_I_DMA_RD_Rule_check_up_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (MC_DMA_MC_LF_P_DMA_RD_Rule_check_up_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (MC_DMA_MC_HF_P_DMA_RD_Rule_check_up_reg-MEMC_REG_BASE_OFFSET))
	{
		ret = _ADDR_CASE_MC_UP_BOUND;
		return ret;
	}
	else if(u32_address == (MC_DMA_MC_LF_DMA_WR_Rule_check_low_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (MC_DMA_MC_HF_DMA_WR_Rule_check_low_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (MC_DMA_MC_LF_I_DMA_RD_Rule_check_low_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (MC_DMA_MC_HF_I_DMA_RD_Rule_check_low_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (MC_DMA_MC_LF_P_DMA_RD_Rule_check_low_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (MC_DMA_MC_HF_P_DMA_RD_Rule_check_low_reg-MEMC_REG_BASE_OFFSET))
	{
		ret = _ADDR_CASE_MC_LOW_BOUND;
		return ret;
	}
	else if(u32_address == (DBUS_WRAPPER_Me_uplimit_addr_reg-MEMC_REG_BASE_OFFSET))
	{
		ret = _ADDR_CASE_ME_UP_BOUND;
		return ret;
	}
	else if(u32_address == (DBUS_WRAPPER_Me_downlimit_addr_reg-MEMC_REG_BASE_OFFSET))
	{
		ret = _ADDR_CASE_ME_LOW_BOUND;
		return ret;
	}
	else if(u32_address == (ME_SHARE_DMA_ME_DMA_WR_Rule_check_up_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (ME_SHARE_DMA_ME_DMA_RD_Rule_check_up_reg-MEMC_REG_BASE_OFFSET))
	{
		ret = _ADDR_CASE_ME_SHARE_UP_BOUND;
		return ret;
	}
	else if(u32_address == (ME_SHARE_DMA_ME_DMA_WR_Rule_check_low_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (ME_SHARE_DMA_ME_DMA_RD_Rule_check_low_reg-MEMC_REG_BASE_OFFSET))
	{
		ret = _ADDR_CASE_ME_SHARE_LOW_BOUND;
		return ret;
	}
	else if(u32_address == (MVINFO_DMA_MVINFO_WR_Rule_check_up_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (MVINFO_DMA_MVINFO_RD_Rule_check_up_reg-MEMC_REG_BASE_OFFSET))
	{
		ret = _ADDR_CASE_MV_INFO_UP_BOUND;
		return ret;
	}
	else if(u32_address == (MVINFO_DMA_MVINFO_WR_Rule_check_low_reg-MEMC_REG_BASE_OFFSET)
		|| u32_address == (MVINFO_DMA_MVINFO_RD_Rule_check_low_reg-MEMC_REG_BASE_OFFSET))
	{
		ret = _ADDR_CASE_MV_INFO_LOW_BOUND;
		return ret;
	}
	else{
		return ret;
	}
#else
	return 0;
#endif	
}

#if 1
VOID HAL_HDF_LimitCheck(unsigned int u32Page_Reg_Addr, unsigned int *u32Page_Reg_Val)
{
	SCALER_MEMC_DMASTARTADDRESS_T* memc_dma_address_info;
	memc_dma_address_info = (SCALER_MEMC_DMASTARTADDRESS_T*)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_INITIALIZATION);

	if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MC_UP_BOUND){
	#if CONFIG_MC_8_BUFFER
		if(Get_DISPLAY_REFRESH_RATE() <= 60){
			*u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr + KMC_TOTAL_SIZE/*KMC_TOTAL_SIZE_8buf_8bit*/);
		}else{
			*u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr + KMC_TOTAL_SIZE_8buf_10bit);
		}
	#else
		*u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr + KMC_TOTAL_SIZE_6buf_10bit);
	#endif
		rtd_pr_memc_info("[HAL_HDF_LimitCheck][_ADDR_CASE_MC_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, *u32Page_Reg_Val);
	}
	else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MC_LOW_BOUND){
		*u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr);
		rtd_pr_memc_info("[HAL_HDF_LimitCheck][_ADDR_CASE_MC_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, *u32Page_Reg_Val);
	}
	else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_UP_BOUND){
		*u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + ME_TOTAL_SIZE);
		rtd_pr_memc_info("[HAL_HDF_LimitCheck][_ADDR_CASE_ME_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, *u32Page_Reg_Val);
	}
	else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_LOW_BOUND){
		*u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr);
		rtd_pr_memc_info("[HAL_HDF_LimitCheck][_ADDR_CASE_ME_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, *u32Page_Reg_Val);
	}
	else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_SHARE_UP_BOUND){
		//u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + ME_TOTAL_SIZE + ME_SHARE_TOTAL_SIZE);
		*u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + ME_TOTAL_SIZE);
		rtd_pr_memc_info("[HAL_HDF_LimitCheck][_ADDR_CASE_ME_SHARE_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, *u32Page_Reg_Val);
	}
	else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_SHARE_LOW_BOUND){
		//u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + ME_TOTAL_SIZE);
		*u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr);
		rtd_pr_memc_info("[HAL_HDF_LimitCheck][_ADDR_CASE_ME_SHARE_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, *u32Page_Reg_Val);
	}
	else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MV_INFO_UP_BOUND){
		//u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + KME_TOTAL_SIZE);
		*u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + ME_TOTAL_SIZE);
		rtd_pr_memc_info("[HAL_HDF_LimitCheck][_ADDR_CASE_MV_INFO_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, *u32Page_Reg_Val);
	}
	else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MV_INFO_LOW_BOUND){
		//u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + ME_TOTAL_SIZE + ME_SHARE_TOTAL_SIZE);
		*u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr);
		rtd_pr_memc_info("[HAL_HDF_LimitCheck][_ADDR_CASE_MV_INFO_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, *u32Page_Reg_Val);
	}

}

VOID HAL_HDF_DMA_Disable(unsigned int u32Page_Reg_Addr, unsigned int *u32Page_Reg_Val)
{
	unsigned int u32_address = u32Page_Reg_Addr + MEMC_REG_BASE_OFFSET;

	if( u32_address == MC_DMA_MC_LF_DMA_WR_Ctrl_reg		|| u32_address == MC_DMA_MC_HF_DMA_WR_Ctrl_reg ||
		u32_address == MC_DMA_MC_LF_I_DMA_RD_Ctrl_reg	|| u32_address == MC_DMA_MC_HF_I_DMA_RD_Ctrl_reg ||
		u32_address == MC_DMA_MC_LF_P_DMA_RD_Ctrl_reg	|| u32_address == MC_DMA_MC_HF_P_DMA_RD_Ctrl_reg ||
		u32_address == ME_SHARE_DMA_ME_DMA_WR_Ctrl_reg	|| u32_address == ME_SHARE_DMA_ME_DMA_RD_Ctrl_reg ||
		u32_address == MVINFO_DMA_MVINFO_WR_Ctrl_reg	|| u32_address == MVINFO_DMA_MVINFO_RD_Ctrl_reg ) {
		
		// bit0=1 means dma is on, therefore closes the dma by setting bit0 to 0
		if( *u32Page_Reg_Val & 1 ) {
			*u32Page_Reg_Val = CLEARBIT(*u32Page_Reg_Val, 0);	// set bit0 to 0
			rtd_pr_memc_info("[MEMC][%s][%d] Disable the DMA %x\n\r", __FUNCTION__, __LINE__, u32_address);
		}
	}

	if( u32_address == KME_DM_TOP0_KME_00_AGENT_reg		|| u32_address == KME_DM_TOP0_KME_01_AGENT_reg ||
		u32_address == KME_DM_TOP0_KME_02_AGENT_reg 	|| u32_address == KME_DM_TOP0_KME_03_AGENT_reg || 
		u32_address == KME_DM_TOP0_KME_04_AGENT_reg		|| u32_address == KME_DM_TOP0_KME_05_AGENT_reg ||
		u32_address == KME_DM_TOP0_KME06AGENT_reg		|| u32_address == KME_DM_TOP0_KME_07_AGENT_reg ||
		u32_address == KME_DM_TOP1_KME_DM_TOP1_14_reg	|| u32_address == KME_DM_TOP1_KME_DM_TOP1_1C_reg ||
		u32_address == KME_DM_TOP1_KME_DM_TOP1_38_reg 	|| u32_address == KME_DM_TOP1_KME_DM_TOP1_40_reg ||
		u32_address == KME_DM_TOP1_KME_DM_TOP1_5C_reg 	|| u32_address == KME_DM_TOP1_KME_DM_TOP1_64_reg ||
		u32_address == KME_DM_TOP1_KME_DM_TOP1_80_reg 	|| u32_address == KME_DM_TOP1_KME_DM_TOP1_88_reg ||
		u32_address == KME_DM_TOP2_MV01_AGENT_reg 		|| u32_address == KME_DM_TOP2_MV02_AGENT_reg ||
	 	u32_address == KME_DM_TOP2_MV03_AGENT_reg 		|| u32_address == KME_DM_TOP2_MV04_AGENT_reg ||
		u32_address == KME_DM_TOP2_MV05_AGENT_reg 		|| u32_address == KME_DM_TOP2_MV06_AGENT_reg ||
	 	u32_address == KME_DM_TOP2_MV07_AGENT_reg 		|| u32_address == KME_DM_TOP2_MV08_AGENT_reg ||
	 	u32_address == KME_DM_TOP2_MV09_AGENT_reg 		|| u32_address == KME_DM_TOP2_MV10_AGENT_reg ||
		u32_address == KME_DM_TOP2_MV11_AGENT_reg 		|| u32_address == KME_DM_TOP2_MV12_AGENT_reg ||
		u32_address == KME_DM_TOP3_SRND_KME_00_AGENT_reg|| u32_address == KME_DM_TOP3_SRND_KME_02_AGENT_reg	||
		u32_address == KME_DM_TOP3_SRND_KME_03_AGENT_reg|| u32_address == KME_DM_TOP3_SRND_MV01_AGENT_reg	||
		u32_address == KME_DM_TOP3_SRND_MV02_AGENT_reg	|| u32_address == KME_DM_TOP3_SRND_MV03_AGENT_reg	||
		u32_address == KME_DM_TOP3_SRND_MV04_AGENT_reg	|| u32_address == KME_DM_TOP3_SRND_MV05_AGENT_reg	||
		u32_address == KME_DM_TOP3_SRND_MV06_AGENT_reg	|| u32_address == KME_DM_TOP3_SRND_MV07_AGENT_reg	||
		u32_address == KME_DM_TOP3_SRND_MV08_AGENT_reg	|| u32_address == KME_DM_TOP3_SRND_MV09_AGENT_reg	||
		u32_address == KME_DM_TOP3_SRND_MV10_AGENT_reg	|| u32_address == KME_DM_TOP3_SRND_MV11_AGENT_reg	||
		u32_address == KME_DM_TOP3_SRND_MV12_AGENT_reg	|| u32_address == KME_DM_TOP3_MV13_AGENT_reg	||
		u32_address == KME_DM_TOP3_MV14_AGENT_reg ) {

		// bit20=1 means dma is on, therefore closes the dma by setting bit20 to 0
		if( (*u32Page_Reg_Val >> 20) & 1 ) {
			*u32Page_Reg_Val = CLEARBIT(*u32Page_Reg_Val, 20);	// set bit20 to 0
			rtd_pr_memc_info("[MEMC][%s][%d] Disable the DMA %x\n\r", __FUNCTION__, __LINE__, u32_address );
		}
	}

	//MEMC_Lib_LoadBringUpScript_Disable_DMA
}

BOOL Get_hdf_Page_Offset( unsigned int u32Page_Id, unsigned int *u32Page_Offset )
{
	unsigned int i = 0;
	unsigned int u32PageSize = 0;
	unsigned int u32PageNum = (u8HDF_Array[7] << 24) + (u8HDF_Array[6] << 16) + (u8HDF_Array[5] << 8) + u8HDF_Array[4];
	unsigned int u32Offset = FILE_HEADER_SIZE;
	
	for(i = 0; i < u32PageNum; i++)
	{
		u32PageSize = (u8HDF_Array[u32Offset + PAGE_SIZE_OFFSET + 3] << 24) + 
					  (u8HDF_Array[u32Offset + PAGE_SIZE_OFFSET + 2] << 16) + 
					  (u8HDF_Array[u32Offset + PAGE_SIZE_OFFSET + 1] << 8) + 
					   u8HDF_Array[u32Offset + PAGE_SIZE_OFFSET];
	
		// find the page id
		if( u32Page_Id == u8HDF_Array[u32Offset + PAGE_ID_OFFSET ] ) {
			*u32Page_Offset = u32Offset;
			return TRUE;
		}
		u32Offset = u32Offset + PAGE_HEADER_SIZE + u32PageSize;
	}
	return FALSE;

}

unsigned int Get_hdf_Table_Value(unsigned int u32Reg_Addr, unsigned int u32Start_Bit, unsigned int u32End_Bit, unsigned int u32Page_Offset)
{
	unsigned int u32Page_ID = 0;
	unsigned int u32Offset = 0;
	unsigned int u32Value = 0;
	unsigned int u32Tmp = 0;
	
	if( u32Start_Bit > u32End_Bit ) {
		rtd_pr_memc_emerg("[MEMC][%s][%d]Fail to get value from hdf. Addr:%x, Start Bit:%d, End Bit:%d\n", __FUNCTION__, __LINE__, u32Reg_Addr, u32Start_Bit, u32End_Bit);
		return 0;
	}

	u32Page_ID = ( u32Reg_Addr >> 8 ) & 0xff;
	u32Offset = u32Page_Offset + PAGE_HEADER_SIZE + (u32Reg_Addr & 0xff);
	u32Value = (u8HDF_Array[u32Offset+3] << 24) + (u8HDF_Array[u32Offset+2] << 16) + (u8HDF_Array[u32Offset+1] << 8) + u8HDF_Array[u32Offset];

	if( u32Start_Bit == 0 && u32End_Bit == 31 ) {
		return u32Value;
	}
	if( u32Start_Bit == u32End_Bit ) {
		return ( u32Value >> u32Start_Bit ) & 0x1;
	}

	u32Tmp = ( 1 << ( u32End_Bit - u32Start_Bit + 1 ) ) - 1;
	return ( u32Value >> u32Start_Bit ) & u32Tmp;
}

void Show_Page_Register(unsigned int u32Page_Id)
{
	unsigned int u32AddressOffset = MEMC_REG_BASE_OFFSET + u32Page_Id * 0x100;
	unsigned int u32Page_Reg_Idx = 0;

	if( u32Page_Id == 79 || u32Page_Id > 98 ) {
		return;
	}

	rtd_pr_memc_emerg("[MEMC]Show Register (Page id=%x)\r\n", u32Page_Id );

	for(u32Page_Reg_Idx = 0; u32Page_Reg_Idx < 16; u32Page_Reg_Idx++)
	{
		rtd_pr_memc_emerg("%x, %x, %x, %x\r\n",
   						rtd_inl( u32AddressOffset + (u32Page_Reg_Idx * 16) ),
   						rtd_inl( u32AddressOffset + (u32Page_Reg_Idx * 16) + 4 ),
   						rtd_inl( u32AddressOffset + (u32Page_Reg_Idx * 16) + 8 ),
   						rtd_inl( u32AddressOffset + (u32Page_Reg_Idx * 16) + 12 ) );
	}
}

void Show_Page_HDF(unsigned int u32Page_Id)
{
	unsigned int u32Page_Offset = 0;
	unsigned int u32PageSize = 0;
	unsigned int u32Page_Reg_Idx = 0;
	
	if( Get_hdf_Page_Offset( u32Page_Id, &u32Page_Offset ) == FALSE ) {
		rtd_pr_memc_emerg("[Show_HDF] Fail to get page offset\r\n");
		return;
	}

	u32PageSize = (u8HDF_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 3] << 24) + 
				  (u8HDF_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 2] << 16) + 
				  (u8HDF_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 1] << 8) + 
				   u8HDF_Array[u32Page_Offset + PAGE_SIZE_OFFSET];

	// check the page id
	if( u32Page_Id != u8HDF_Array[u32Page_Offset + PAGE_ID_OFFSET ] ) {
		rtd_pr_memc_emerg("[Show_HDF] Fail to match page id\r\n");
		return;
	}
	rtd_pr_memc_emerg("[MEMC]Show HDF (Page id=%x)\r\n", u32Page_Id );

	u32Page_Offset = u32Page_Offset + PAGE_HEADER_SIZE;
	for(u32Page_Reg_Idx = 0; u32Page_Reg_Idx < u32PageSize / 16; u32Page_Reg_Idx++)
	{
		rtd_pr_memc_emerg("%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\r\n",
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 1],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 2],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 3],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 4],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 5],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 6],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 7],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 8],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 9],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 10],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 11],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 12],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 13],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 14],
   						u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 16 + 15] );
	}
}
#endif

#if 1
VOID HAL_HDF_Load(VOID)
{
#ifdef USE_LOAD_HDF

	unsigned int u32Page_Idx = 0;
	unsigned int u32PageNum = (u8HDF_Array[7] << 24) + (u8HDF_Array[6] << 16) + (u8HDF_Array[5] << 8) + u8HDF_Array[4];
	
#ifdef BUILD_QUICK_SHOW
	MEMC_LibBlueScreen(1);
#endif

	Mid_Mode_DisableMCDMA();
	Mid_Mode_DisableMEDMA(FALSE);

	for(u32Page_Idx = 0; u32Page_Idx < u32PageNum; u32Page_Idx++) {
		if(0 /*u32Page_Idx == _PAGE_ID_PQL0 ||u32Page_Idx == _PAGE_ID_PQL1 || u32Page_Idx == _PAGE_ID_Software1 */ ) {
			continue;
		}
		HAL_HDF_Load_Page_u8HDF(u32Page_Idx);
	}
	//MEMC_SetHDF_PQL0();
	//MEMC_SetHDF_PQL1();
	//MEMC_SetHDF_Software1();

#endif
}


#else
VOID HAL_HDF_Load(VOID)
{
#ifdef USE_LOAD_HDF
	unsigned int u32PageNum = 0;
	unsigned int u32PageSize = 0;
	unsigned int u32Page_Idx = 0;
	unsigned int u32Page_Offset = 0;
	unsigned int u32Page_Addr = 0;
	unsigned int u32Page_Reg_Idx = 0;
	unsigned int u32Page_Reg_Addr = 0;
	unsigned int u32Page_Reg_Val = 0;
	SCALER_MEMC_DMASTARTADDRESS_T* memc_dma_address_info;
	memc_dma_address_info = (SCALER_MEMC_DMASTARTADDRESS_T*)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_INITIALIZATION);

	u32PageNum = (u8HDF_Array[7] << 24) + (u8HDF_Array[6] << 16) + (u8HDF_Array[5] << 8) + u8HDF_Array[4];
	u32Page_Offset = FILE_HEADER_SIZE;
	for(u32Page_Idx = 0; u32Page_Idx < u32PageNum; u32Page_Idx++)
	{	
		//LogPrintf(DBG_MSG, "u32Page_Idx = %d\r\n", u32Page_Idx);

		u32PageSize = (u8HDF_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 3] << 24) + 
					  (u8HDF_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 2] << 16) + 
					  (u8HDF_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 1] << 8) + 
					   u8HDF_Array[u32Page_Offset + PAGE_SIZE_OFFSET];
		u32Page_Addr = (u8HDF_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET + 3] << 24) + 
					   (u8HDF_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET + 2] << 16) + 
					   (u8HDF_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET + 1] << 8) + 
					    u8HDF_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET];
//		rtd_pr_memc_emerg("[jerry_HAL_HDF_Load_02][%s][%d][,%d,%d,%d,%d,%d,]\n", __FUNCTION__, __LINE__, u32Page_Idx, u32PageNum, u32Page_Addr, u32PageSize, u32Page_Reg_Addr);
//		if(u32Page_Idx < 46){  
			for(u32Page_Reg_Idx = 0; u32Page_Reg_Idx < u32PageSize / 4; u32Page_Reg_Idx++)
			{
				u32Page_Reg_Addr = u32Page_Addr + u32Page_Reg_Idx * 4;
				u32Page_Reg_Val = (u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 3] << 24) + 
						   		  (u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 2] << 16) + 
						   		  (u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 1] << 8) + 
						    	   u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4];
				if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MC_UP_BOUND){
					#if CONFIG_MC_8_BUFFER
					if(Get_DISPLAY_REFRESH_RATE() <= 60){
						u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr + KMC_TOTAL_SIZE/*KMC_TOTAL_SIZE_8buf_8bit*/);
					}else{
						u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr + KMC_TOTAL_SIZE_8buf_10bit);
					}
					#else
					u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr + KMC_TOTAL_SIZE_6buf_10bit);
					#endif
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_MC_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MC_LOW_BOUND){
					u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_MC_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_UP_BOUND){
					u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + ME_TOTAL_SIZE);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_ME_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_LOW_BOUND){
					u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_ME_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_SHARE_UP_BOUND){
					//u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + ME_TOTAL_SIZE + ME_SHARE_TOTAL_SIZE);
					u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + ME_TOTAL_SIZE);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_ME_SHARE_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_SHARE_LOW_BOUND){
					//u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + ME_TOTAL_SIZE);
					u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_ME_SHARE_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MV_INFO_UP_BOUND){
					//u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + KME_TOTAL_SIZE);
					u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + ME_TOTAL_SIZE);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_MV_INFO_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MV_INFO_LOW_BOUND){
					//u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + ME_TOTAL_SIZE + ME_SHARE_TOTAL_SIZE);
					u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_MV_INFO_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}

				WriteRegister(u32Page_Reg_Addr+MEMC_REG_BASE_OFFSET, 0, 31, u32Page_Reg_Val);
			}
//		}
//		rtd_pr_memc_emerg("[jerry_HAL_HDF_Load_03][%s][%d][,%d,%d,%d,%d,%d,]\n", __FUNCTION__, __LINE__, u32Page_Idx, u32PageNum, u32Page_Addr, u32PageSize, u32Page_Reg_Addr);
		u32Page_Offset = u32Page_Offset + PAGE_HEADER_SIZE + u32PageSize;
		if(u32Page_Idx == u32PageNum - 1)
		{
			rtd_pr_memc_notice( "Load hdf done... Total %d pages\r\n", u32PageNum);
		}
	}
#endif
}

#endif


VOID HAL_HDF_Load_Page_u8HDF(unsigned int page)
{
	unsigned int u32Page_Idx = 0;
	unsigned int u32PageSize = 0;
	unsigned int u32Page_Addr = 0;
	unsigned int u32Page_Reg_Idx = 0;
	unsigned int u32Page_Reg_Addr = 0;
	unsigned int u32Page_Reg_Val = 0;
	unsigned int u32PageNum = (u8HDF_Array[7] << 24) + (u8HDF_Array[6] << 16) + (u8HDF_Array[5] << 8) + u8HDF_Array[4];
	unsigned int u32Page_Offset = FILE_HEADER_SIZE;

	for(u32Page_Idx = 0; u32Page_Idx < u32PageNum; u32Page_Idx++)
	{
		u32PageSize = (u8HDF_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 3] << 24) + 
					  (u8HDF_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 2] << 16) + 
					  (u8HDF_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 1] << 8) + 
					   u8HDF_Array[u32Page_Offset + PAGE_SIZE_OFFSET];

		// check the page id
		if( page != u8HDF_Array[u32Page_Offset + PAGE_ID_OFFSET ] ) {
			u32Page_Offset = u32Page_Offset + PAGE_HEADER_SIZE + u32PageSize;
			continue;
		}
		u32Page_Addr = (u8HDF_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET + 3] << 24) + 
					   (u8HDF_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET + 2] << 16) + 
					   (u8HDF_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET + 1] << 8) + 
					    u8HDF_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET];

		for(u32Page_Reg_Idx = 0; u32Page_Reg_Idx < u32PageSize / 4; u32Page_Reg_Idx++)
		{
			u32Page_Reg_Addr = u32Page_Addr + u32Page_Reg_Idx * 4;
			u32Page_Reg_Val = (u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 3] << 24) + 
					   		  (u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 2] << 16) + 
					   		  (u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 1] << 8) + 
					    	   u8HDF_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4];

			HAL_HDF_LimitCheck( u32Page_Reg_Addr, &u32Page_Reg_Val);
			HAL_HDF_DMA_Disable( u32Page_Reg_Addr, &u32Page_Reg_Val);
			#ifdef BUILD_QUICK_SHOW
			if(u32Page_Reg_Addr+MEMC_REG_BASE_OFFSET==KPOST_TOP_KPOST_TOP_60_reg){
				u32Page_Reg_Val = u32Page_Reg_Val | 0x40000000;//keep blue screen in quick show
			}
			#endif
			WriteRegister(u32Page_Reg_Addr+MEMC_REG_BASE_OFFSET, 0, 31, u32Page_Reg_Val);
		}
		rtd_pr_memc_info("[%s][%d][%d,%d,%d,%d,%d,]\n", __FUNCTION__, __LINE__, page, u32PageNum, u32Page_Addr, u32PageSize, u32Page_Reg_Addr);
		break;
	}
}

VOID HAL_HDF_LoadDefault( unsigned char *u8_ACT_Array)
{
#ifdef USE_LOAD_HDF
	unsigned int u32PageNum = 0;
	unsigned int u32PageSize = 0;
	unsigned int u32Page_Idx = 0;
	unsigned int u32Page_Offset = 0;
	unsigned int u32Page_Addr = 0;
	unsigned int u32Page_Reg_Idx = 0;
	unsigned int u32Page_Reg_Addr = 0;
	unsigned int u32Page_Reg_Val = 0;
	SCALER_MEMC_DMASTARTADDRESS_T* memc_dma_address_info;
	memc_dma_address_info = (SCALER_MEMC_DMASTARTADDRESS_T*)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_INITIALIZATION);

	u32PageNum = (u8_ACT_Array[7] << 24) + (u8_ACT_Array[6] << 16) + (u8_ACT_Array[5] << 8) + u8_ACT_Array[4];
	u32Page_Offset = FILE_HEADER_SIZE;
	for(u32Page_Idx = 0; u32Page_Idx < u32PageNum; u32Page_Idx++)
	{	
//		rtd_pr_memc_emerg("[MEMC_HDF_01]u32Page_Idx = %d\r\n", u32Page_Idx);

		u32PageSize = (u8_ACT_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 3] << 24) + 
					  (u8_ACT_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 2] << 16) + 
					  (u8_ACT_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 1] << 8) + 
					   u8_ACT_Array[u32Page_Offset + PAGE_SIZE_OFFSET];
		u32Page_Addr = (u8_ACT_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET + 3] << 24) + 
					   (u8_ACT_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET + 2] << 16) + 
					   (u8_ACT_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET + 1] << 8) + 
					    u8_ACT_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET];
//		rtd_pr_memc_emerg("[MEMC_HDF_02][%s][%d][,%d,%d,%d,%d,%d,]\n", __FUNCTION__, __LINE__, u32Page_Idx, u32PageNum, u32Page_Addr, u32PageSize, u32Page_Reg_Addr);
//		if(u32Page_Idx < 46){
			for(u32Page_Reg_Idx = 0; u32Page_Reg_Idx < u32PageSize / 4; u32Page_Reg_Idx++)
			{
				u32Page_Reg_Addr = u32Page_Addr + u32Page_Reg_Idx * 4;
				u32Page_Reg_Val = (u8_ACT_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 3] << 24) + 
						   		  (u8_ACT_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 2] << 16) + 
						   		  (u8_ACT_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 1] << 8) + 
						    	   u8_ACT_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4];

				#if 1
				HAL_HDF_LimitCheck(u32Page_Reg_Addr, &u32Page_Reg_Val);
				#else
				if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MC_UP_BOUND){
					u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr + KMC_TOTAL_SIZE);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_MC_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MC_LOW_BOUND){
					u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_MC_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_UP_BOUND){
					u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + KME_TOTAL_SIZE);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_ME_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_LOW_BOUND){
					u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_ME_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				#endif
				WriteRegister(u32Page_Reg_Addr, 0, 31, u32Page_Reg_Val);
			}
//		}
//		rtd_pr_memc_emerg("[MEMC_HDF_03][%s][%d][,%d,%d,%d,%d,%d,]\n", __FUNCTION__, __LINE__, u32Page_Idx, u32PageNum, u32Page_Addr, u32PageSize, u32Page_Reg_Addr);
		u32Page_Offset = u32Page_Offset + PAGE_HEADER_SIZE + u32PageSize;
		if(u32Page_Idx == u32PageNum - 1)
		{
//			rtd_pr_memc_notice( "Load hdf done... Total %d pages\r\n", u32PageNum);
		}
	}
#endif
}


VOID HAL_HDF_Load_DefaultPage(unsigned int page, unsigned char *u8_ACT_Array)
{
		unsigned int u32PageNum = 0;
		unsigned int u32PageSize = 0;
		unsigned int u32Page_Idx = 0;
		unsigned int u32Page_Offset = 0;
		unsigned int u32Page_Addr = 0;
		unsigned int u32Page_Reg_Idx = 0;
		unsigned int u32Page_Reg_Addr = 0;
		unsigned int u32Page_Reg_Val = 0;
		SCALER_MEMC_DMASTARTADDRESS_T* memc_dma_address_info;
		memc_dma_address_info = (SCALER_MEMC_DMASTARTADDRESS_T*)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_INITIALIZATION);

		u32PageNum = (u8_ACT_Array[7] << 24) + (u8_ACT_Array[6] << 16) + (u8_ACT_Array[5] << 8) + u8_ACT_Array[4];
		u32Page_Offset = FILE_HEADER_SIZE;
		for(u32Page_Idx = 0; u32Page_Idx < u32PageNum; u32Page_Idx++)
		{	
//			rtd_pr_memc_emerg("[MEMC_HDF_01]u32Page_Idx = %d\r\n", u32Page_Idx);
	
			u32PageSize = (u8_ACT_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 3] << 24) + 
						  (u8_ACT_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 2] << 16) + 
						  (u8_ACT_Array[u32Page_Offset + PAGE_SIZE_OFFSET + 1] << 8) + 
						   u8_ACT_Array[u32Page_Offset + PAGE_SIZE_OFFSET];
			u32Page_Addr = (u8_ACT_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET + 3] << 24) + 
						   (u8_ACT_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET + 2] << 16) + 
						   (u8_ACT_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET + 1] << 8) + 
							u8_ACT_Array[u32Page_Offset + PAGE_ADDRESS_OFFSET];
//			rtd_pr_memc_emerg("[MEMC_HDF_02][%s][%d][,%d,%d,%d,%d,%d,]\n", __FUNCTION__, __LINE__, u32Page_Idx, u32PageNum, u32Page_Addr, u32PageSize, u32Page_Reg_Addr);

			if(u32Page_Addr == (page<<8)){
//				rtd_pr_memc_emerg("[MEMC_HDF_03][%s][%d]Page Match!! page= %d\n", __FUNCTION__, __LINE__, u32Page_Idx);			
				for(u32Page_Reg_Idx = 0; u32Page_Reg_Idx < u32PageSize / 4; u32Page_Reg_Idx++)
				{
					u32Page_Reg_Addr = u32Page_Addr + u32Page_Reg_Idx * 4;
					u32Page_Reg_Val = (u8_ACT_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 3] << 24) + 
									  (u8_ACT_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 2] << 16) + 
									  (u8_ACT_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 1] << 8) + 
									   u8_ACT_Array[u32Page_Offset + PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4];

					#if 1
					HAL_HDF_LimitCheck(u32Page_Reg_Addr, &u32Page_Reg_Val);
					#else
					if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MC_UP_BOUND){
						u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr + KMC_TOTAL_SIZE);
						rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_MC_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
					}
					else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MC_LOW_BOUND){
						u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr);
						rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_MC_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
					}
					else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_UP_BOUND){
						u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + KME_TOTAL_SIZE);
						rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_ME_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
					}
					else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_LOW_BOUND){
						u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr);
						rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_ME_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
					}
					#endif
					WriteRegister(u32Page_Reg_Addr, 0, 31, u32Page_Reg_Val);
				}
			}else{
//				rtd_pr_memc_emerg("[MEMC_HDF_04][%s][%d]Page Miss Match!! (%x,%x)\n", __FUNCTION__, __LINE__, u32Page_Addr,page);	
			}

//			rtd_pr_memc_emerg("[MEMC_HDF_05][%s][%d][,%d,%d,%d,%d,%d,]\n", __FUNCTION__, __LINE__, u32Page_Idx, u32PageNum, u32Page_Addr, u32PageSize, u32Page_Reg_Addr);
			u32Page_Offset = u32Page_Offset + PAGE_HEADER_SIZE + u32PageSize;
			if(u32Page_Idx == u32PageNum - 1)
			{
				rtd_pr_memc_notice( "Load hdf done... Total %d pages\r\n", u32PageNum);
			}
		}

}

VOID HAL_HDF_Load_Page(unsigned int page, unsigned char *u8_ACT_Array)
{
		unsigned int u32PageSize = 0;
		unsigned int u32Page_Addr = 0;
		unsigned int u32Page_Reg_Idx = 0;
		unsigned int u32Page_Reg_Addr = 0;
		unsigned int u32Page_Reg_Val = 0;
		SCALER_MEMC_DMASTARTADDRESS_T* memc_dma_address_info;
		memc_dma_address_info = (SCALER_MEMC_DMASTARTADDRESS_T*)Scaler_GetShareMemVirAddr(SCALERIOC_MEMC_INITIALIZATION);

				
		u32PageSize = (u8_ACT_Array[PAGE_SIZE_OFFSET + 3] << 24) + 
					  (u8_ACT_Array[PAGE_SIZE_OFFSET + 2] << 16) + 
					  (u8_ACT_Array[PAGE_SIZE_OFFSET + 1] << 8) + 
					   u8_ACT_Array[PAGE_SIZE_OFFSET];
		u32Page_Addr = (u8_ACT_Array[PAGE_ADDRESS_OFFSET + 3] << 24) + 
					   (u8_ACT_Array[PAGE_ADDRESS_OFFSET + 2] << 16) + 
					   (u8_ACT_Array[PAGE_ADDRESS_OFFSET + 1] << 8) + 
						u8_ACT_Array[PAGE_ADDRESS_OFFSET];
		
//		rtd_pr_memc_emerg("[MEMC_HDF_01][%s][%d][%d,%d,%d,]\n", __FUNCTION__, __LINE__, u32Page_Addr, u32PageSize, u32Page_Reg_Addr);
		if(u32Page_Addr == (page<<8)){
			for(u32Page_Reg_Idx = 0; u32Page_Reg_Idx < u32PageSize / 4; u32Page_Reg_Idx++)
			{
				u32Page_Reg_Addr = u32Page_Addr + u32Page_Reg_Idx * 4;
				u32Page_Reg_Val = (u8_ACT_Array[PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 3] << 24) + 
								  (u8_ACT_Array[PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 2] << 16) + 
								  (u8_ACT_Array[PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4 + 1] << 8) + 
								   u8_ACT_Array[PAGE_HEADER_SIZE + u32Page_Reg_Idx * 4];

				#if 1
				HAL_HDF_LimitCheck(u32Page_Reg_Addr, &u32Page_Reg_Val);
				#else
				if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MC_UP_BOUND){
					u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr + KMC_TOTAL_SIZE);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_MC_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_MC_LOW_BOUND){
					u32Page_Reg_Val = (memc_dma_address_info->MC00_start_addr);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_MC_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_UP_BOUND){
					u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr + KME_TOTAL_SIZE);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_ME_UP_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				else if(HAL_HDF_AddressCheck(u32Page_Reg_Addr) == _ADDR_CASE_ME_LOW_BOUND){
					u32Page_Reg_Val = (memc_dma_address_info->ME_start_addr);
					rtd_pr_memc_notice("[HAL_HDF_Load][_ADDR_CASE_ME_LOW_BOUND][%x = %x]\n\r", u32Page_Reg_Addr, u32Page_Reg_Val);
				}
				#endif
				WriteRegister(u32Page_Reg_Addr, 0, 31, u32Page_Reg_Val);
			}
		}
//		rtd_pr_memc_emerg("[MEMC_HDF_02][%s][%d][%d,%d,%d,]\n", __FUNCTION__, __LINE__, u32Page_Addr, u32PageSize, u32Page_Reg_Addr);

}

unsigned int HAL_MEMC_HDF_ACT(unsigned int HDF_ACT,unsigned int page, unsigned char *u8_ACT_Array)
{
	unsigned int ret =0;

	if(u8_ACT_Array == NULL && (HDF_ACT != MEMC_ACT_LoadDefault) ){
//		rtd_pr_memc_emerg("[MEMC_HDF][%s][%d][u8_ACT_Array NULL!!]\n", __FUNCTION__, __LINE__);	
		return MEMC_ACT_ERR;
	}	
	
	switch(HDF_ACT)
	{
		case MEMC_ACT_LoadDefault:
				if(u8_ACT_Array == NULL){
					HAL_HDF_Load();
				}else{
					HAL_HDF_LoadDefault(u8_ACT_Array);
				}
			break;
			
		case MEMC_ACT_LoadDefaultPage:
				HAL_HDF_Load_DefaultPage(page,u8_ACT_Array);
			break;
			
		case MEMC_ACT_LoadPage:
				HAL_HDF_Load_Page(page,u8_ACT_Array);
			break;				

		default:

 			break;
	}

	return ret;

}

