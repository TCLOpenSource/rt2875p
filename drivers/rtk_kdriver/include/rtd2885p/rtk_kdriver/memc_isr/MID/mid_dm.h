/**
 * @file mid_dm.h
 * @brief This file is the header for DM controller.
 * @date Nov.19.2014
 * @version V0.0.1
 * @author Martin Yan
 * @par Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/
#ifndef _MID_DM_HEADER_INCLUDE_
#define _MID_DM_HEADER_INCLUDE_

#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/memc_type.h"

/**
 * @brief This function set kme start address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMEStartAddress(ME_DM_ADDRESS_ST *pMEDMAddr);

/**
 * @brief This function set kme end address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMEEndAddress(ME_DM_ADDRESS_ST *pMEDMAddr);

/**
 * @brief This function set kme buffer offset address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMEOffsetAddress(ME_DM_ADDRESS_ST *pMEDMAddr);

/**
 * @brief This function set kme1 RPT start address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKME1RPTStartAddress(ME1RPT_DM_ADDRESS_ST *pME1RPTDMAddr);
/**
 * @brief This function set kme1 RPT end address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKME1RPTEndAddress(ME1RPT_DM_ADDRESS_ST *pME1RPTDMAddr);
/**
 * @brief This function set kme1 RPT buffer offset address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKME1RPTOffsetAddress(ME1RPT_DM_ADDRESS_ST *pME1RPTDMAddr);
/**
 * @brief This function set ipme start address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetIPMEStartAddress(IPME_DM_ADDRESS_ST *pIPMEDMAddr);
/**
 * @brief This function set ipme end address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetIPMEEndAddress(IPME_DM_ADDRESS_ST *pIPMEDMAddr);
/**
 * @brief This function set ipme buffer offset address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetIPMEOffsetAddress(IPME_DM_ADDRESS_ST *pIPMEDMAddr);
/**
 * @brief This function set kmc start address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMCStartAddress(MC_DM_ADDRESS_ST *pMCDMAddr);

/**
 * @brief This function set kmv start address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMVStartAddress(MV_DM_ADDRESS_ST *pMVDMAddr);

/**
 * @brief This function set kme end address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMVEndAddress(MV_DM_ADDRESS_ST *pMVDMAddr);

/**
 * @brief This function set kmv buffer offset address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMVOffsetAddress(MV_DM_ADDRESS_ST *pMVDMAddr);

/**
 * @brief This function set kmc end address
 * @param [in] VOID
 * @retval VOID
*/
VOID Mid_DM_SetKMCEndAddress(MC_DM_ADDRESS_ST *pMCDMAddr);

#if 1  // k4lp
/**
 * @brief This function set kmc offset address
 * @param [in] unsigned int u32LineOffset_HF
 * @param [in] unsigned int u32LROffset_HF
 * @param [in] unsigned int u32LineOffset_HF
 * @param [in] unsigned int u32LROffset_LF
 * @retval VOID
*/
VOID Mid_DM_SetKMCOffsetAddress(unsigned int u32LineOffset_HF, unsigned int u32LROffset_HF, unsigned int u32LineOffset_LF, unsigned int u32LROffset_LF);

#else
/**
 * @brief This function set kmc offset address
 * @param [in] INOUT_MODE eMode
 * @retval VOID
*/
VOID Mid_DM_SetKMCOffsetAddress(unsigned int u32LineOffset, unsigned int u32LROffset);

#endif

/**
 * @brief This function set kmc engine enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID Mid_DM_SetKMCEngineEnable(BOOL bEnable);

/**
 * @brief This function set kme engine enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID Mid_DM_SetKMEEngineEnable(BOOL bEnable,  INOUT_MODE emom, INPUT_FORMAT emif, OUTPUT_FORMAT emof);

/**
 * @brief This function set kmv engine enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
#if 1  // k4lp
VOID Mid_DM_SetKMVEngineEnable(BOOL bEnable, INOUT_MODE emom);
#else
VOID Mid_DM_SetKMVEngineEnable(BOOL bEnable);
#endif

/**
 * @brief This function set kmv vbuf enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID Mid_DM_SetKMEVbufEnable(BOOL bEnable);

#endif
