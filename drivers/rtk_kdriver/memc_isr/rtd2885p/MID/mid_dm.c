/**
 * @file mid_dm.c
 * @brief This file is for DM controller.
 * @date Nov.19.2014
 * @version V0.0.1
 * @author Martin Yan
 * @par Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/
#ifdef CONFIG_ARM64 //ARM32 compatible
#include <linux/kernel.h>
#endif

#include "memc_isr/Common/kw_common.h"
#include "memc_isr/Common/kw_debug.h"
#include "memc_isr/Common/memc_type.h"
#include "memc_isr/MID/mid_mode.h"
#include "memc_isr/HAL/hal_dm.h"
#include "memc_isr/HAL/hal_me.h"
#include "memc_isr/PQL/PQLPlatformDefs.h"
//#include <rtd_log/rtd_module.h>

/**
 * @brief This function set kme start address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMEStartAddress(ME_DM_ADDRESS_ST *pMEDMAddr)
{
#if 0 // IC_K7LP
	return;

	HAL_DM_SetKme00StartAddress0(pMEDMAddr->stBuf_00_0.u32StartAddress);
	HAL_DM_SetKme00StartAddress1(pMEDMAddr->stBuf_00_1.u32StartAddress);
	HAL_DM_SetKme00StartAddress2(pMEDMAddr->stBuf_00_2.u32StartAddress);
	HAL_DM_SetKme00StartAddress3(pMEDMAddr->stBuf_00_3.u32StartAddress);
	HAL_DM_SetKme00StartAddress4(pMEDMAddr->stBuf_00_4.u32StartAddress);
	HAL_DM_SetKme00StartAddress5(pMEDMAddr->stBuf_00_5.u32StartAddress);
	HAL_DM_SetKme00StartAddress6(pMEDMAddr->stBuf_00_6.u32StartAddress);
	HAL_DM_SetKme00StartAddress7(pMEDMAddr->stBuf_00_7.u32StartAddress);

	HAL_DM_SetKme08StartAddress0(pMEDMAddr->stBuf_08_0.u32StartAddress);
	HAL_DM_SetKme08StartAddress1(pMEDMAddr->stBuf_08_1.u32StartAddress);

	HAL_DM_SetKme10StartAddress0(pMEDMAddr->stBuf_10_0.u32StartAddress);
	HAL_DM_SetKme10StartAddress1(pMEDMAddr->stBuf_10_1.u32StartAddress);

	HAL_DM_SetKme12StartAddress0(pMEDMAddr->stBuf_12_0.u32StartAddress);
	HAL_DM_SetKme12StartAddress1(pMEDMAddr->stBuf_12_1.u32StartAddress);

	HAL_DM_SetKme14StartAddress0(pMEDMAddr->stBuf_14_0.u32StartAddress);
	HAL_DM_SetKme14StartAddress1(pMEDMAddr->stBuf_14_1.u32StartAddress);
#endif
}

/**
 * @brief This function set kme end address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMEEndAddress(ME_DM_ADDRESS_ST *pMEDMAddr)
{
#if 0 // IC_K7LP
	return;

	unsigned int u32MEVSize = Mid_Mode_Get_MEVSize();

	pMEDMAddr->stBuf_00_0.u32EndAddress = pMEDMAddr->stBuf_00_0.u32StartAddress + pMEDMAddr->u32Buf_00_LineOffset * u32MEVSize;
	pMEDMAddr->stBuf_00_1.u32EndAddress = pMEDMAddr->stBuf_00_1.u32StartAddress + pMEDMAddr->u32Buf_00_LineOffset * u32MEVSize;
	pMEDMAddr->stBuf_00_2.u32EndAddress = pMEDMAddr->stBuf_00_2.u32StartAddress + pMEDMAddr->u32Buf_00_LineOffset * u32MEVSize;
	pMEDMAddr->stBuf_00_3.u32EndAddress = pMEDMAddr->stBuf_00_3.u32StartAddress + pMEDMAddr->u32Buf_00_LineOffset * u32MEVSize;
	pMEDMAddr->stBuf_00_4.u32EndAddress = pMEDMAddr->stBuf_00_4.u32StartAddress + pMEDMAddr->u32Buf_00_LineOffset * u32MEVSize;
	pMEDMAddr->stBuf_00_5.u32EndAddress = pMEDMAddr->stBuf_00_5.u32StartAddress + pMEDMAddr->u32Buf_00_LineOffset * u32MEVSize;
	pMEDMAddr->stBuf_00_6.u32EndAddress = pMEDMAddr->stBuf_00_6.u32StartAddress + pMEDMAddr->u32Buf_00_LineOffset * u32MEVSize;
	pMEDMAddr->stBuf_00_7.u32EndAddress = pMEDMAddr->stBuf_00_7.u32StartAddress + pMEDMAddr->u32Buf_00_LineOffset * u32MEVSize;

	#if IC_K6LP
	pMEDMAddr->stBuf_08_0.u32EndAddress = pMEDMAddr->stBuf_08_0.u32StartAddress + pMEDMAddr->u32Buf_08_LineOffset * u32MEVSize;
	pMEDMAddr->stBuf_08_1.u32EndAddress = pMEDMAddr->stBuf_08_1.u32StartAddress + pMEDMAddr->u32Buf_08_LineOffset * u32MEVSize;

	pMEDMAddr->stBuf_10_0.u32EndAddress = pMEDMAddr->stBuf_10_0.u32StartAddress + pMEDMAddr->u32Buf_10_LineOffset * u32MEVSize;
	pMEDMAddr->stBuf_10_1.u32EndAddress = pMEDMAddr->stBuf_10_1.u32StartAddress + pMEDMAddr->u32Buf_10_LineOffset * u32MEVSize;

	pMEDMAddr->stBuf_12_0.u32EndAddress = pMEDMAddr->stBuf_12_0.u32StartAddress + pMEDMAddr->u32Buf_12_LineOffset * u32MEVSize;
	pMEDMAddr->stBuf_12_1.u32EndAddress = pMEDMAddr->stBuf_12_1.u32StartAddress + pMEDMAddr->u32Buf_12_LineOffset * u32MEVSize;

	pMEDMAddr->stBuf_14_0.u32EndAddress = pMEDMAddr->stBuf_14_0.u32StartAddress + pMEDMAddr->u32Buf_14_LineOffset * u32MEVSize/4;
	pMEDMAddr->stBuf_14_1.u32EndAddress = pMEDMAddr->stBuf_14_1.u32StartAddress + pMEDMAddr->u32Buf_14_LineOffset * u32MEVSize/4;
	#else
	pMEDMAddr->stBuf_12_0.u32EndAddress = pMEDMAddr->stBuf_12_0.u32StartAddress + pMEDMAddr->u32Buf_12_LineOffset * u32MEVSize;
	#endif

	HAL_DM_SetKme00EndAddress0(pMEDMAddr->stBuf_00_0.u32EndAddress);
	HAL_DM_SetKme00EndAddress1(pMEDMAddr->stBuf_00_1.u32EndAddress);
	HAL_DM_SetKme00EndAddress2(pMEDMAddr->stBuf_00_2.u32EndAddress);
	HAL_DM_SetKme00EndAddress3(pMEDMAddr->stBuf_00_3.u32EndAddress);
	HAL_DM_SetKme00EndAddress4(pMEDMAddr->stBuf_00_4.u32EndAddress);
	HAL_DM_SetKme00EndAddress5(pMEDMAddr->stBuf_00_5.u32EndAddress);
	HAL_DM_SetKme00EndAddress6(pMEDMAddr->stBuf_00_6.u32EndAddress);
	HAL_DM_SetKme00EndAddress7(pMEDMAddr->stBuf_00_7.u32EndAddress);

	#if IC_K6LP
	HAL_DM_SetKme08EndAddress0(pMEDMAddr->stBuf_08_0.u32EndAddress);
	HAL_DM_SetKme08EndAddress1(pMEDMAddr->stBuf_08_1.u32EndAddress);
	HAL_DM_SetKme10EndAddress0(pMEDMAddr->stBuf_10_0.u32EndAddress);
	HAL_DM_SetKme10EndAddress1(pMEDMAddr->stBuf_10_1.u32EndAddress);
	HAL_DM_SetKme14EndAddress0(pMEDMAddr->stBuf_14_0.u32EndAddress);
	HAL_DM_SetKme14EndAddress1(pMEDMAddr->stBuf_14_1.u32EndAddress);
	#endif

	HAL_DM_SetKme12EndAddress0(pMEDMAddr->stBuf_12_0.u32EndAddress);
	HAL_DM_SetKme12EndAddress1(pMEDMAddr->stBuf_12_0.u32EndAddress);
#endif
}

/**
 * @brief This function set kme buffer offset address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMEOffsetAddress(ME_DM_ADDRESS_ST *pMEDMAddr)
{
#if 0 // IC_K7LP
	return;

	HAL_DM_SetKme00LineOffsetAddress(pMEDMAddr->u32Buf_00_LineOffset);
	HAL_DM_SetKme00LROffsetAddress(pMEDMAddr->u32Buf_00_LROffset);
	HAL_DM_SetKme08LineOffsetAddress(pMEDMAddr->u32Buf_08_LineOffset);
	HAL_DM_SetKme10LineOffsetAddress(pMEDMAddr->u32Buf_10_LineOffset);
	HAL_DM_SetKme12LineOffsetAddress(pMEDMAddr->u32Buf_12_LineOffset);
	HAL_DM_SetKme14LineOffsetAddress(pMEDMAddr->u32Buf_14_LineOffset);
#endif
}

/**
 * @brief This function set kme1 RPT start address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKME1RPTStartAddress(ME1RPT_DM_ADDRESS_ST *pME1RPTDMAddr)
{
#if 0 // IC_K7LP
	return;

	HAL_DM_SetME1RPT_ppfv_StartAddress0(pME1RPTDMAddr->stBuf_PPFV_0.u32StartAddress);
	HAL_DM_SetME1RPT_ppfv_StartAddress1(pME1RPTDMAddr->stBuf_PPFV_1.u32StartAddress);
	HAL_DM_SetME1RPT_pfv_StartAddress0(pME1RPTDMAddr->stBuf_PFV_0.u32StartAddress);
	HAL_DM_SetME1RPT_pfv_StartAddress1(pME1RPTDMAddr->stBuf_PFV_1.u32StartAddress);
	HAL_DM_SetME1RPT_pfv_StartAddress2(pME1RPTDMAddr->stBuf_PFV_2.u32StartAddress);
#endif
}
/**
 * @brief This function set kme1 RPT end address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKME1RPTEndAddress(ME1RPT_DM_ADDRESS_ST *pME1RPTDMAddr)
{
#if 0 // IC_K7LP
	return;

	HAL_DM_SetME1RPT_ppfv_EndAddress0(pME1RPTDMAddr->stBuf_PPFV_0.u32EndAddress);
	HAL_DM_SetME1RPT_ppfv_EndAddress1(pME1RPTDMAddr->stBuf_PPFV_1.u32EndAddress);
	HAL_DM_SetME1RPT_pfv_EndAddress0(pME1RPTDMAddr->stBuf_PFV_0.u32EndAddress);
	HAL_DM_SetME1RPT_pfv_EndAddress1(pME1RPTDMAddr->stBuf_PFV_1.u32EndAddress);
	HAL_DM_SetME1RPT_pfv_EndAddress2(pME1RPTDMAddr->stBuf_PFV_2.u32EndAddress);
#endif
}
/**
 * @brief This function set kme1 RPT buffer offset address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKME1RPTOffsetAddress(ME1RPT_DM_ADDRESS_ST *pME1RPTDMAddr)
{
#if 0 // IC_K7LP
	return;

	HAL_DM_SetME1RPT_ppfv_LineOffsetAddress(pME1RPTDMAddr->u32Buf_PPFV_LineOffset);
	HAL_DM_SetME1RPT_pfv_LineOffsetAddress(pME1RPTDMAddr->u32Buf_PFV_LineOffset);
#endif
}
/**
 * @brief This function set ipme start address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetIPMEStartAddress(IPME_DM_ADDRESS_ST *pIPMEDMAddr)
{
#if 0 // IC_K7LP
	return;

	HAL_DM_SetIPMEStartAddress0(pIPMEDMAddr->stBuf_IPME_0.u32StartAddress);
	HAL_DM_SetIPMEStartAddress1(pIPMEDMAddr->stBuf_IPME_1.u32StartAddress);
	HAL_DM_SetIPMEStartAddress2(pIPMEDMAddr->stBuf_IPME_2.u32StartAddress);
	HAL_DM_SetIPMEStartAddress3(pIPMEDMAddr->stBuf_IPME_3.u32StartAddress);
	HAL_DM_SetIPMEStartAddress4(pIPMEDMAddr->stBuf_IPME_4.u32StartAddress);
	HAL_DM_SetIPMEStartAddress5(pIPMEDMAddr->stBuf_IPME_5.u32StartAddress);
	HAL_DM_SetIPMEStartAddress6(pIPMEDMAddr->stBuf_IPME_6.u32StartAddress);
	HAL_DM_SetIPMEStartAddress7(pIPMEDMAddr->stBuf_IPME_7.u32StartAddress);
#endif
}
/**
 * @brief This function set ipme end address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetIPMEEndAddress(IPME_DM_ADDRESS_ST *pIPMEDMAddr)
{
#if 0 // IC_K7LP
	return;

	HAL_DM_SetIPMEEndAddress0(pIPMEDMAddr->stBuf_IPME_0.u32EndAddress);
	HAL_DM_SetIPMEEndAddress1(pIPMEDMAddr->stBuf_IPME_1.u32EndAddress);
	HAL_DM_SetIPMEEndAddress2(pIPMEDMAddr->stBuf_IPME_2.u32EndAddress);
	HAL_DM_SetIPMEEndAddress3(pIPMEDMAddr->stBuf_IPME_3.u32EndAddress);
	HAL_DM_SetIPMEEndAddress4(pIPMEDMAddr->stBuf_IPME_4.u32EndAddress);
	HAL_DM_SetIPMEEndAddress5(pIPMEDMAddr->stBuf_IPME_5.u32EndAddress);
	HAL_DM_SetIPMEEndAddress6(pIPMEDMAddr->stBuf_IPME_6.u32EndAddress);
	HAL_DM_SetIPMEEndAddress7(pIPMEDMAddr->stBuf_IPME_7.u32EndAddress);
#endif
}
/**
 * @brief This function set ipme buffer offset address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetIPMEOffsetAddress(IPME_DM_ADDRESS_ST *pIPMEDMAddr)
{
#if 0 // IC_K7LP
	return;

	HAL_DM_SetIPMELineOffsetAddress(pIPMEDMAddr->u32Buf_IPME_LineOffset);
#endif
}
/**
 * @brief This function set kmv start address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMVStartAddress(MV_DM_ADDRESS_ST *pMVDMAddr)
{
#if 0 // IC_K7LP
	return;

	HAL_DM_SetKmv01StartAddress0(pMVDMAddr->stBuf_01_0.u32StartAddress);
	HAL_DM_SetKmv02StartAddress0(pMVDMAddr->stBuf_02_0.u32StartAddress);
	HAL_DM_SetKmv04StartAddress0(pMVDMAddr->stBuf_04_0.u32StartAddress);
	HAL_DM_SetKmv04StartAddress1(pMVDMAddr->stBuf_04_1.u32StartAddress);
	HAL_DM_SetKmv04StartAddress2(pMVDMAddr->stBuf_04_2.u32StartAddress);
	HAL_DM_SetKmv05StartAddress0(pMVDMAddr->stBuf_05_0.u32StartAddress);
	HAL_DM_SetKmv05StartAddress1(pMVDMAddr->stBuf_05_1.u32StartAddress);
	#ifdef LBME2_DM_BUF
	HAL_DM_SetKmv12StartAddress0(pMVDMAddr->stBuf_12_0.u32StartAddress);
	HAL_DM_SetKmv12StartAddress1(pMVDMAddr->stBuf_12_1.u32StartAddress);
	HAL_DM_SetKmv12StartAddress2(pMVDMAddr->stBuf_12_2.u32StartAddress);
	HAL_DM_SetKmv12StartAddress3(pMVDMAddr->stBuf_12_3.u32StartAddress);
	HAL_DM_SetKmv12StartAddress4(pMVDMAddr->stBuf_12_4.u32StartAddress);
	HAL_DM_SetKmv12StartAddress5(pMVDMAddr->stBuf_12_5.u32StartAddress);
	HAL_DM_SetKmv12StartAddress6(pMVDMAddr->stBuf_12_6.u32StartAddress);
	HAL_DM_SetKmv12StartAddress7(pMVDMAddr->stBuf_12_7.u32StartAddress);
	#endif
#endif
}

/**
 * @brief This function set kme end address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMVEndAddress(MV_DM_ADDRESS_ST *pMVDMAddr)
{
#if 0 // IC_K7LP
	return;

	unsigned int u32MERowNum = Mid_Mode_Get_MERowNum();
	unsigned int u32ME2RowNum = Mid_Mode_Get_ME2RowNum();
#ifdef LBME2_DM_BUF
	unsigned int u32ME2VSize = Mid_Mode_Get_ME2VSize();
#endif

	pMVDMAddr->stBuf_01_0.u32EndAddress = pMVDMAddr->stBuf_01_0.u32StartAddress + pMVDMAddr->u32Buf_01_LineOffset * u32ME2RowNum;
	pMVDMAddr->stBuf_02_0.u32EndAddress = pMVDMAddr->stBuf_02_0.u32StartAddress + pMVDMAddr->u32Buf_02_LineOffset * u32MERowNum;
	pMVDMAddr->stBuf_04_0.u32EndAddress = pMVDMAddr->stBuf_04_0.u32StartAddress + pMVDMAddr->u32Buf_04_LineOffset * u32MERowNum;
	pMVDMAddr->stBuf_04_1.u32EndAddress = pMVDMAddr->stBuf_04_1.u32StartAddress + pMVDMAddr->u32Buf_04_LineOffset * u32MERowNum;
	pMVDMAddr->stBuf_04_2.u32EndAddress = pMVDMAddr->stBuf_04_2.u32StartAddress + pMVDMAddr->u32Buf_04_LineOffset * u32MERowNum;
	pMVDMAddr->stBuf_05_0.u32EndAddress = pMVDMAddr->stBuf_05_0.u32StartAddress + pMVDMAddr->u32Buf_05_LineOffset * u32MERowNum;
	pMVDMAddr->stBuf_05_1.u32EndAddress = pMVDMAddr->stBuf_05_1.u32StartAddress + pMVDMAddr->u32Buf_05_LineOffset * u32MERowNum;
#ifdef LBME2_DM_BUF
	pMVDMAddr->stBuf_12_0.u32EndAddress = pMVDMAddr->stBuf_12_0.u32StartAddress + pMVDMAddr->u32Buf_12_LineOffset * u32ME2VSize;
	pMVDMAddr->stBuf_12_1.u32EndAddress = pMVDMAddr->stBuf_12_1.u32StartAddress + pMVDMAddr->u32Buf_12_LineOffset * u32ME2VSize;
	pMVDMAddr->stBuf_12_2.u32EndAddress = pMVDMAddr->stBuf_12_2.u32StartAddress + pMVDMAddr->u32Buf_12_LineOffset * u32ME2VSize;
	pMVDMAddr->stBuf_12_3.u32EndAddress = pMVDMAddr->stBuf_12_3.u32StartAddress + pMVDMAddr->u32Buf_12_LineOffset * u32ME2VSize;
	pMVDMAddr->stBuf_12_4.u32EndAddress = pMVDMAddr->stBuf_12_4.u32StartAddress + pMVDMAddr->u32Buf_12_LineOffset * u32ME2VSize;
	pMVDMAddr->stBuf_12_5.u32EndAddress = pMVDMAddr->stBuf_12_5.u32StartAddress + pMVDMAddr->u32Buf_12_LineOffset * u32ME2VSize;
	pMVDMAddr->stBuf_12_6.u32EndAddress = pMVDMAddr->stBuf_12_6.u32StartAddress + pMVDMAddr->u32Buf_12_LineOffset * u32ME2VSize;
	pMVDMAddr->stBuf_12_7.u32EndAddress = pMVDMAddr->stBuf_12_7.u32StartAddress + pMVDMAddr->u32Buf_12_LineOffset * u32ME2VSize;
#endif	

	HAL_DM_SetKmv01EndAddress0(pMVDMAddr->stBuf_01_0.u32EndAddress);
	HAL_DM_SetKmv02EndAddress0(pMVDMAddr->stBuf_02_0.u32EndAddress);
	HAL_DM_SetKmv04EndAddress0(pMVDMAddr->stBuf_04_0.u32EndAddress);
	HAL_DM_SetKmv04EndAddress1(pMVDMAddr->stBuf_04_1.u32EndAddress);
	HAL_DM_SetKmv04EndAddress2(pMVDMAddr->stBuf_04_2.u32EndAddress);
	HAL_DM_SetKmv05EndAddress0(pMVDMAddr->stBuf_05_0.u32EndAddress);
	HAL_DM_SetKmv05EndAddress1(pMVDMAddr->stBuf_05_1.u32EndAddress);
#ifdef LBME2_DM_BUF
	HAL_DM_SetKmv12EndAddress0(pMVDMAddr->stBuf_12_0.u32EndAddress);
	HAL_DM_SetKmv12EndAddress1(pMVDMAddr->stBuf_12_1.u32EndAddress);
	HAL_DM_SetKmv12EndAddress2(pMVDMAddr->stBuf_12_2.u32EndAddress);
	HAL_DM_SetKmv12EndAddress3(pMVDMAddr->stBuf_12_3.u32EndAddress);
	HAL_DM_SetKmv12EndAddress4(pMVDMAddr->stBuf_12_4.u32EndAddress);
	HAL_DM_SetKmv12EndAddress5(pMVDMAddr->stBuf_12_5.u32EndAddress);
	HAL_DM_SetKmv12EndAddress6(pMVDMAddr->stBuf_12_6.u32EndAddress);
	HAL_DM_SetKmv12EndAddress7(pMVDMAddr->stBuf_12_7.u32EndAddress);
#endif	

#endif
}

/**
 * @brief This function set kmv buffer offset address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMVOffsetAddress(MV_DM_ADDRESS_ST *pMVDMAddr)
{
#if 0 // IC_K7LP
	return;

	HAL_DM_SetKmv01LineOffsetAddress(pMVDMAddr->u32Buf_01_LineOffset);
	HAL_DM_SetKmv01LROffsetAddress(pMVDMAddr->u32Buf_01_LROffset);
	HAL_DM_SetKmv02LineOffsetAddress(pMVDMAddr->u32Buf_02_LineOffset);
	HAL_DM_SetKmv02LROffsetAddress(pMVDMAddr->u32Buf_02_LROffset);
	HAL_DM_SetKmv04LineOffsetAddress(pMVDMAddr->u32Buf_04_LineOffset);
	HAL_DM_SetKmv04LROffsetAddress(pMVDMAddr->u32Buf_04_LROffset);
	HAL_DM_SetKmv05LineOffsetAddress(pMVDMAddr->u32Buf_05_LineOffset);
	HAL_DM_SetKmv05LROffsetAddress(pMVDMAddr->u32Buf_05_LROffset);
#ifdef LBME2_DM_BUF
	HAL_DM_SetKmv12LineOffsetAddress(pMVDMAddr->u32Buf_12_LineOffset);
	HAL_DM_SetKmv12LROffsetAddress(pMVDMAddr->u32Buf_12_LROffset);
#endif	

#endif
}

/**
 * @brief This function set kmc end address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMCStartAddress(MC_DM_ADDRESS_ST *pMCDMAddr)
{
#if 0 // IC_K7LP
	return;

#if 1
	HAL_DM_SetKmc01StartAddress0(pMCDMAddr->stBuf_LF_0.u32StartAddress);
	HAL_DM_SetKmc01StartAddress1(pMCDMAddr->stBuf_LF_1.u32StartAddress);
	HAL_DM_SetKmc01StartAddress2(pMCDMAddr->stBuf_LF_2.u32StartAddress);
	HAL_DM_SetKmc01StartAddress3(pMCDMAddr->stBuf_LF_3.u32StartAddress);
	HAL_DM_SetKmc01StartAddress4(pMCDMAddr->stBuf_LF_4.u32StartAddress);
	HAL_DM_SetKmc01StartAddress5(pMCDMAddr->stBuf_LF_5.u32StartAddress);
	HAL_DM_SetKmc01StartAddress6(pMCDMAddr->stBuf_LF_6.u32StartAddress);
	HAL_DM_SetKmc01StartAddress7(pMCDMAddr->stBuf_LF_7.u32StartAddress);

	HAL_DM_SetKmc00StartAddress0(pMCDMAddr->stBuf_HF_0.u32StartAddress);
	HAL_DM_SetKmc00StartAddress1(pMCDMAddr->stBuf_HF_1.u32StartAddress);
	HAL_DM_SetKmc00StartAddress2(pMCDMAddr->stBuf_HF_2.u32StartAddress);
	HAL_DM_SetKmc00StartAddress3(pMCDMAddr->stBuf_HF_3.u32StartAddress);
	HAL_DM_SetKmc00StartAddress4(pMCDMAddr->stBuf_HF_4.u32StartAddress);
	HAL_DM_SetKmc00StartAddress5(pMCDMAddr->stBuf_HF_5.u32StartAddress);
	HAL_DM_SetKmc00StartAddress6(pMCDMAddr->stBuf_HF_6.u32StartAddress);
	HAL_DM_SetKmc00StartAddress7(pMCDMAddr->stBuf_HF_7.u32StartAddress);
#else
	HAL_DM_SetKmc01StartAddress0(pMCDMAddr->stBuf_HF_0.u32StartAddress);
	HAL_DM_SetKmc01StartAddress1(pMCDMAddr->stBuf_HF_1.u32StartAddress);
	HAL_DM_SetKmc01StartAddress2(pMCDMAddr->stBuf_HF_2.u32StartAddress);
	HAL_DM_SetKmc01StartAddress3(pMCDMAddr->stBuf_HF_3.u32StartAddress);
	HAL_DM_SetKmc01StartAddress4(pMCDMAddr->stBuf_HF_4.u32StartAddress);
	HAL_DM_SetKmc01StartAddress5(pMCDMAddr->stBuf_HF_5.u32StartAddress);
	HAL_DM_SetKmc01StartAddress6(pMCDMAddr->stBuf_HF_6.u32StartAddress);
	HAL_DM_SetKmc01StartAddress7(pMCDMAddr->stBuf_HF_7.u32StartAddress);

	HAL_DM_SetKmc00StartAddress0(pMCDMAddr->stBuf_LF_0.u32StartAddress);
	HAL_DM_SetKmc00StartAddress1(pMCDMAddr->stBuf_LF_1.u32StartAddress);
	HAL_DM_SetKmc00StartAddress2(pMCDMAddr->stBuf_LF_2.u32StartAddress);
	HAL_DM_SetKmc00StartAddress3(pMCDMAddr->stBuf_LF_3.u32StartAddress);
	HAL_DM_SetKmc00StartAddress4(pMCDMAddr->stBuf_LF_4.u32StartAddress);
	HAL_DM_SetKmc00StartAddress5(pMCDMAddr->stBuf_LF_5.u32StartAddress);
	HAL_DM_SetKmc00StartAddress6(pMCDMAddr->stBuf_LF_6.u32StartAddress);
	HAL_DM_SetKmc00StartAddress7(pMCDMAddr->stBuf_LF_7.u32StartAddress);
#endif
#endif
}

/**
 * @brief This function set kmc end address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMCEndAddress(MC_DM_ADDRESS_ST *pMCDMAddr)
{
#if 0 // IC_K7LP
	return;

	unsigned int u32FRC_Vact = Mid_Mode_Get_ME_Vact();
#if 0
	pMCDMAddr->stBuf_HF_0.u32EndAddress = pMCDMAddr->stBuf_HF_0.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_1.u32EndAddress = pMCDMAddr->stBuf_HF_1.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_2.u32EndAddress = pMCDMAddr->stBuf_HF_2.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_3.u32EndAddress = pMCDMAddr->stBuf_HF_3.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_4.u32EndAddress = pMCDMAddr->stBuf_HF_4.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_5.u32EndAddress = pMCDMAddr->stBuf_HF_5.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_6.u32EndAddress = pMCDMAddr->stBuf_HF_6.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_7.u32EndAddress = pMCDMAddr->stBuf_HF_7.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;

	HAL_DM_SetKmc00EndAddress0(pMCDMAddr->stBuf_HF_0.u32EndAddress);
	HAL_DM_SetKmc00EndAddress1(pMCDMAddr->stBuf_HF_1.u32EndAddress);
	HAL_DM_SetKmc00EndAddress2(pMCDMAddr->stBuf_HF_2.u32EndAddress);
	HAL_DM_SetKmc00EndAddress3(pMCDMAddr->stBuf_HF_3.u32EndAddress);
	HAL_DM_SetKmc00EndAddress4(pMCDMAddr->stBuf_HF_4.u32EndAddress);
	HAL_DM_SetKmc00EndAddress5(pMCDMAddr->stBuf_HF_5.u32EndAddress);
	HAL_DM_SetKmc00EndAddress6(pMCDMAddr->stBuf_HF_6.u32EndAddress);
	HAL_DM_SetKmc00EndAddress7(pMCDMAddr->stBuf_HF_7.u32EndAddress);

	pMCDMAddr->stBuf_LF_0.u32EndAddress = pMCDMAddr->stBuf_LF_0.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_1.u32EndAddress = pMCDMAddr->stBuf_LF_1.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_2.u32EndAddress = pMCDMAddr->stBuf_LF_2.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_3.u32EndAddress = pMCDMAddr->stBuf_LF_3.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_4.u32EndAddress = pMCDMAddr->stBuf_LF_4.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_5.u32EndAddress = pMCDMAddr->stBuf_LF_5.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_6.u32EndAddress = pMCDMAddr->stBuf_LF_6.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_7.u32EndAddress = pMCDMAddr->stBuf_LF_7.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;

	HAL_DM_SetKmc01EndAddress0(pMCDMAddr->stBuf_LF_0.u32EndAddress);
	HAL_DM_SetKmc01EndAddress1(pMCDMAddr->stBuf_LF_1.u32EndAddress);
	HAL_DM_SetKmc01EndAddress2(pMCDMAddr->stBuf_LF_2.u32EndAddress);
	HAL_DM_SetKmc01EndAddress3(pMCDMAddr->stBuf_LF_3.u32EndAddress);
	HAL_DM_SetKmc01EndAddress4(pMCDMAddr->stBuf_LF_4.u32EndAddress);
	HAL_DM_SetKmc01EndAddress5(pMCDMAddr->stBuf_LF_5.u32EndAddress);
	HAL_DM_SetKmc01EndAddress6(pMCDMAddr->stBuf_LF_6.u32EndAddress);
	HAL_DM_SetKmc01EndAddress7(pMCDMAddr->stBuf_LF_7.u32EndAddress);
#else
	pMCDMAddr->stBuf_LF_0.u32EndAddress = pMCDMAddr->stBuf_LF_0.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_1.u32EndAddress = pMCDMAddr->stBuf_LF_1.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_2.u32EndAddress = pMCDMAddr->stBuf_LF_2.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_3.u32EndAddress = pMCDMAddr->stBuf_LF_3.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_4.u32EndAddress = pMCDMAddr->stBuf_LF_4.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_5.u32EndAddress = pMCDMAddr->stBuf_LF_5.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_6.u32EndAddress = pMCDMAddr->stBuf_LF_6.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_LF_7.u32EndAddress = pMCDMAddr->stBuf_LF_7.u32StartAddress + pMCDMAddr->u32LineOffset_VDLF * u32FRC_Vact / 2;

	HAL_DM_SetKmc00EndAddress0(pMCDMAddr->stBuf_LF_0.u32EndAddress);
	HAL_DM_SetKmc00EndAddress1(pMCDMAddr->stBuf_LF_1.u32EndAddress);
	HAL_DM_SetKmc00EndAddress2(pMCDMAddr->stBuf_LF_2.u32EndAddress);
	HAL_DM_SetKmc00EndAddress3(pMCDMAddr->stBuf_LF_3.u32EndAddress);
	HAL_DM_SetKmc00EndAddress4(pMCDMAddr->stBuf_LF_4.u32EndAddress);
	HAL_DM_SetKmc00EndAddress5(pMCDMAddr->stBuf_LF_5.u32EndAddress);
	HAL_DM_SetKmc00EndAddress6(pMCDMAddr->stBuf_LF_6.u32EndAddress);
	HAL_DM_SetKmc00EndAddress7(pMCDMAddr->stBuf_LF_7.u32EndAddress);

	pMCDMAddr->stBuf_HF_0.u32EndAddress = pMCDMAddr->stBuf_HF_0.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_1.u32EndAddress = pMCDMAddr->stBuf_HF_1.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_2.u32EndAddress = pMCDMAddr->stBuf_HF_2.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_3.u32EndAddress = pMCDMAddr->stBuf_HF_3.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_4.u32EndAddress = pMCDMAddr->stBuf_HF_4.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_5.u32EndAddress = pMCDMAddr->stBuf_HF_5.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_6.u32EndAddress = pMCDMAddr->stBuf_HF_6.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;
	pMCDMAddr->stBuf_HF_7.u32EndAddress = pMCDMAddr->stBuf_HF_7.u32StartAddress + pMCDMAddr->u32LineOffset_VDHF * u32FRC_Vact / 2;

	HAL_DM_SetKmc01EndAddress0(pMCDMAddr->stBuf_HF_0.u32EndAddress);
	HAL_DM_SetKmc01EndAddress1(pMCDMAddr->stBuf_HF_1.u32EndAddress);
	HAL_DM_SetKmc01EndAddress2(pMCDMAddr->stBuf_HF_2.u32EndAddress);
	HAL_DM_SetKmc01EndAddress3(pMCDMAddr->stBuf_HF_3.u32EndAddress);
	HAL_DM_SetKmc01EndAddress4(pMCDMAddr->stBuf_HF_4.u32EndAddress);
	HAL_DM_SetKmc01EndAddress5(pMCDMAddr->stBuf_HF_5.u32EndAddress);
	HAL_DM_SetKmc01EndAddress6(pMCDMAddr->stBuf_HF_6.u32EndAddress);
	HAL_DM_SetKmc01EndAddress7(pMCDMAddr->stBuf_HF_7.u32EndAddress);
#endif
#endif
}

#if 1  // K4lp
/**
 * @brief This function set kmc offset address
 * @param [in] unsigned int u32LineOffset_HF
 * @param [in] unsigned int u32LROffset_HF
 * @param [in] unsigned int u32LineOffset_HF
 * @param [in] unsigned int u32LROffset_LF
 * @retval VOID
*/
VOID Mid_DM_SetKMCOffsetAddress(unsigned int u32LineOffset_HF, unsigned int u32LROffset_HF, unsigned int u32LineOffset_LF, unsigned int u32LROffset_LF)
{
#if 0 // IC_K7LP
	return;

#if 1
	// KMC_00
	HAL_DM_SetKmc00LROffsetAddress(u32LROffset_LF);
	HAL_DM_SetKmc00LineOffsetAddress(u32LineOffset_LF);
	
	//KMC_01
	HAL_DM_SetKmc01LROffsetAddress(u32LROffset_HF);
	HAL_DM_SetKmc01LineOffsetAddress(u32LineOffset_HF);
#else
	// KMC_00
	HAL_DM_SetKmc00LROffsetAddress(u32LROffset_HF);
	HAL_DM_SetKmc00LineOffsetAddress(u32LineOffset_HF);
	
	//KMC_01
	HAL_DM_SetKmc01LROffsetAddress(u32LROffset_LF);
	HAL_DM_SetKmc01LineOffsetAddress(u32LineOffset_LF);
#endif
#endif
}
#else
/**
 * @brief This function set kmc offset address
 * @param [in] INOUT_MODE eMode
 * @retval VOID
*/
VOID Mid_DM_SetKMCOffsetAddress(unsigned int u32LineOffset, unsigned int u32LROffset)
{
	// KMC_00
	HAL_DM_SetKmc00LROffsetAddress(u32LROffset);
	HAL_DM_SetKmc00LineOffsetAddress(u32LineOffset);

	//KMC_01
	HAL_DM_SetKmc01LROffsetAddress(u32LROffset);
	HAL_DM_SetKmc01LineOffsetAddress(u32LineOffset);
}
#endif

/**
 * @brief This function set kmc engine enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID Mid_DM_SetKMCEngineEnable(BOOL bEnable)
{
#if 0 // IC_K7LP
	return;

	HAL_DM_SetKmc00WriteEnable(bEnable);
	HAL_DM_SetKmc01WriteEnable(bEnable);
	HAL_DM_SetKmc04ReadEnable(bEnable);
	HAL_DM_SetKmc05ReadEnable(bEnable);
	HAL_DM_SetKmc06ReadEnable(bEnable);
	HAL_DM_SetKmc07ReadEnable(bEnable);
#endif
}

/**
 * @brief This function set kme engine enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID Mid_DM_SetKMEEngineEnable(BOOL bEnable,  INOUT_MODE emom, INPUT_FORMAT emif, OUTPUT_FORMAT emof)
{
#if 0 // IC_K7LP
	return;

	if((emom == RGB_IN_VIDEO_OUT) || (emom == YUV_IN_VIDEO_OUT))
	{
		HAL_DM_SetKme00WriteEnable(bEnable);
		//HAL_DM_SetKme04WriteEnable(bEnable);
		//HAL_DM_SetKme05WriteEnable(bEnable);
	}
	else
	{
		HAL_DM_SetKme00WriteEnable(FALSE);
		//HAL_DM_SetKme04WriteEnable(FALSE);
		//HAL_DM_SetKme05WriteEnable(FALSE);
	}

	if((emif == INPUT_FORMAT_2D) && (emof == OUTPUT_FORMAT_2D))
	{
		if((emom == RGB_IN_VIDEO_OUT) || (emom == YUV_IN_VIDEO_OUT))
		{
			HAL_DM_SetKme08WriteEnable(bEnable);
			HAL_DM_SetKme10WriteEnable(bEnable);
			HAL_DM_SetKme12WriteEnable(bEnable);
			HAL_DM_SetKme14WriteEnable(bEnable);
		}
		else
		{
			HAL_DM_SetKme08WriteEnable(FALSE);
			HAL_DM_SetKme10WriteEnable(FALSE);
			HAL_DM_SetKme12WriteEnable(FALSE);
			HAL_DM_SetKme14WriteEnable(FALSE);
		}
	}
	else
	{
		HAL_DM_SetKme08WriteEnable(FALSE);
		HAL_DM_SetKme10WriteEnable(FALSE);
		HAL_DM_SetKme12WriteEnable(FALSE);
		HAL_DM_SetKme14WriteEnable(FALSE);
	}

	if((emom == RGB_IN_VIDEO_OUT) || (emom == YUV_IN_VIDEO_OUT))
	{
		HAL_DM_SetKme01ReadEnable(bEnable);
		HAL_DM_SetKme02ReadEnable(bEnable);
		HAL_DM_SetKme03ReadEnable(bEnable);
		//HAL_DM_SetKme06ReadEnable(bEnable);
		//HAL_DM_SetKme07ReadEnable(bEnable);
	}
	else
	{
		HAL_DM_SetKme01ReadEnable(FALSE);
		HAL_DM_SetKme02ReadEnable(FALSE);
		HAL_DM_SetKme03ReadEnable(FALSE);
		//HAL_DM_SetKme06ReadEnable(FALSE);
		//HAL_DM_SetKme07ReadEnable(FALSE);
	}

	if((emif == INPUT_FORMAT_2D) && (emof == OUTPUT_FORMAT_2D))
	{
		if((emom == RGB_IN_VIDEO_OUT) || (emom == YUV_IN_VIDEO_OUT))
		{
			HAL_DM_SetKme09ReadEnable(bEnable);
			HAL_DM_SetKme11ReadEnable(bEnable);
			HAL_DM_SetKme13ReadEnable(bEnable);
			HAL_DM_SetKme15ReadEnable(bEnable);
		}
		else
		{
			HAL_DM_SetKme09ReadEnable(FALSE);
			HAL_DM_SetKme11ReadEnable(FALSE);
			HAL_DM_SetKme13ReadEnable(FALSE);
			HAL_DM_SetKme15ReadEnable(FALSE);
		}
	}
	else
	{
		HAL_DM_SetKme09ReadEnable(FALSE);
		HAL_DM_SetKme11ReadEnable(FALSE);
		HAL_DM_SetKme13ReadEnable(FALSE);
		HAL_DM_SetKme15ReadEnable(FALSE);
	}
#endif
}

/**
 * @brief This function set kmv engine enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
#if 1  // k4lp
VOID Mid_DM_SetKMVEngineEnable(BOOL bEnable, INOUT_MODE emom)
{
#if 0 // IC_K7LP
	return;

	if((emom == RGB_IN_VIDEO_OUT) || (emom == YUV_IN_VIDEO_OUT))
	{
		HAL_DM_SetKmv01WriteEnable(bEnable);
		HAL_DM_SetKmv02WriteEnable(bEnable);
		HAL_DM_SetKmv04WriteEnable(bEnable);	
		HAL_DM_SetKmv05WriteEnable(bEnable);	
#ifdef LBME2_DM_BUF
		HAL_DM_SetKmv12WriteEnable(bEnable);
#endif		
		HAL_DM_SetKmv03ReadEnable(bEnable);
		HAL_DM_SetKmv06ReadEnable(bEnable);
		HAL_DM_SetKmv07ReadEnable(bEnable);
		HAL_DM_SetKmv08ReadEnable(bEnable);
		HAL_DM_SetKmv09ReadEnable(bEnable);
		HAL_DM_SetKmv10ReadEnable(bEnable);
		HAL_DM_SetKmv11ReadEnable(bEnable);
	}
	else
	{
		HAL_DM_SetKmv01WriteEnable(FALSE);
		HAL_DM_SetKmv02WriteEnable(FALSE);
		HAL_DM_SetKmv04WriteEnable(FALSE);	
		HAL_DM_SetKmv05WriteEnable(FALSE);
#ifdef LBME2_DM_BUF
		HAL_DM_SetKmv12WriteEnable(FALSE);
#endif	
		HAL_DM_SetKmv03ReadEnable(FALSE);
		HAL_DM_SetKmv06ReadEnable(FALSE);
		HAL_DM_SetKmv07ReadEnable(FALSE);
		HAL_DM_SetKmv08ReadEnable(FALSE);
		HAL_DM_SetKmv09ReadEnable(FALSE);
		HAL_DM_SetKmv10ReadEnable(FALSE);
		HAL_DM_SetKmv11ReadEnable(FALSE);
	}
#endif
}
#else
VOID Mid_DM_SetKMVEngineEnable(BOOL bEnable)
{
	HAL_DM_SetKmv01WriteEnable(bEnable);
	HAL_DM_SetKmv02WriteEnable(bEnable);
	HAL_DM_SetKmv04WriteEnable(bEnable);
	HAL_DM_SetKmv05WriteEnable(bEnable);
	HAL_DM_SetKmv12WriteEnable(bEnable);

	HAL_DM_SetKmv03ReadEnable(bEnable);
	HAL_DM_SetKmv06ReadEnable(bEnable);
	HAL_DM_SetKmv07ReadEnable(bEnable);
	HAL_DM_SetKmv08ReadEnable(bEnable);
	HAL_DM_SetKmv09ReadEnable(bEnable);
	HAL_DM_SetKmv10ReadEnable(bEnable);
	HAL_DM_SetKmv11ReadEnable(bEnable);
}

#endif

/**
 * @brief This function set kmv vbuf enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID Mid_DM_SetKMEVbufEnable(BOOL bEnable)
{
	HAL_KME_Set_Vbuf_Enable(bEnable);
}

