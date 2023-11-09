/**
 * @file hal_hdf.h
 * @brief This file is for HDF loading.
 * @date October.28.2014
 * @version V0.0.1
 * @author Martin Yan
 * @par Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/
#ifndef _HAL_HDF_HEADER_INCLUDE_
#define _HAL_HDF_HEADER_INCLUDE_

#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/memc_type.h"

#define USE_LOAD_HDF
#define FILE_HEADER_SIZE		64
#define PAGE_HEADER_SIZE		16
#define PAGE_CONTENT_SIZE		256


typedef enum _MEMC_HDF_ACT_TYPE {
	MEMC_ACT_LoadDefault =0,
	MEMC_ACT_LoadDefaultPage,
	MEMC_ACT_LoadPage,
	MEMC_ACT_ERR = 0xff,
} MEMC_HDF_ACT_MODE;

typedef enum _MEMC_HDF_INSTR_MODE {
	MEMC_INSTR_LOAD =0,
	MEMC_INSTR_WRITE,
	MEMC_INSTR_ERR = 0xff,
} MEMC_HDF_INSTR_TYPE;

/**
 * @brief This function load default hdf.
 * @param [in] VOID.
 * @retval VOID
*/
typedef enum  
{
	_ADDR_CASE_NULL, // 0
	_ADDR_CASE_MC_UP_BOUND,
	_ADDR_CASE_MC_LOW_BOUND,
	_ADDR_CASE_ME_UP_BOUND,
	_ADDR_CASE_ME_LOW_BOUND,
	_ADDR_CASE_ME_SHARE_UP_BOUND,
	_ADDR_CASE_ME_SHARE_LOW_BOUND,
	_ADDR_CASE_MV_INFO_UP_BOUND,
	_ADDR_CASE_MV_INFO_LOW_BOUND,
}ADDR_CASE_ID;


typedef enum
{
	_PAGE_ID_PQL0 = 69,
	_PAGE_ID_PQL1 = 70,
	_PAGE_ID_Software1 = 71,
}_PAGE_ID;

BOOL Get_hdf_Page_Offset( unsigned int u32Page_Id, unsigned int *u32Page_Offset );
unsigned int Get_hdf_Table_Value(unsigned int u32Reg_Addr, unsigned int u32Start_Bit, unsigned int u32End_Bit, unsigned int u32Page_Offset);
void Show_Page_Register(unsigned int u32Page_Id);
void Show_Page_HDF(unsigned int u32Page_Id);
VOID HAL_HDF_Load_Page_u8HDF(unsigned int page);


VOID HAL_HDF_Load(VOID);
VOID HAL_HDF_LoadDefault(unsigned char *u8_ACT_Array);
VOID HAL_HDF_Load_DefaultPage(unsigned int page, unsigned char *u8_ACT_Array);
VOID HAL_HDF_Load_Page(unsigned int page, unsigned char *u8_ACT_Array);
unsigned int HAL_MEMC_HDF_ACT(unsigned int HDF_TYPE,unsigned int page, unsigned char *u8_ACT_Array);

#endif
