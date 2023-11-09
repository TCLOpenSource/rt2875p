/**
 * @file hal_omet.c
 * @brief This file is for OMET register settings.
 * @date Sep.01.2014
 * @version V0.0.1
 * @author Lixiangjun
 * @par MEMC Library V0.0.1 - Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#include "memc_isr/Common/kw_common.h"
#include "memc_isr/Driver/regio.h"
#include "memc_reg_def.h"

/**
 * @brief This function set MEMC two gains.
 * @param [in] ubyFirstGain		Specify first gain.
 * @param [in] ubySecondGain	Specify second gain.
*/
VOID HAL_OMET_SetTwoGains(unsigned char ubyFirstGain, unsigned char ubySecondGain)
{
	/* set register*/
	//bRet = WriteRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, (unsigned int)(ubyFirstGain & 1));
}

/**
 * @brief This function get OMET two gains.
 * @param [out] pubyFirstGain	first gain return.
 * @param [out] pubySecondGain	second gain return.
*/
VOID HAL_OMET_GetTwoGains(unsigned char *pubyFirstGain, unsigned char *pubySecondGain)
{
	unsigned int	uiVal = 0;


	/* set register*/
	//bRet = ReadRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, &uiVal);
	*pubyFirstGain = (unsigned char)(uiVal & 1);
}

/**
 * @brief This function set pedestal setting.
 * @param [in] ubyGain		pedestal gain setting
*/
VOID HAL_OMET_SetPedestal(unsigned char ubyGain)
{
	/* set register*/
	//bRet = WriteRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, (unsigned int)(ubyGain));
}

/**
 * @brief This function get pedestal setting.
 * @retval prdestal setting
*/
unsigned char HAL_OMET_GetPedestal(VOID)
{
	unsigned int	u32Val = 0;

	/* set register*/
	//bRet = ReadRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, &u32Val);
	return (unsigned char)(u32Val & 0xff);
}

/**
 * @brief This function get OMET red power.
 * @retval R power
*/
unsigned short HAL_OMET_GetRedPower(VOID)
{
	unsigned int	u32Val = 0;

	/* set register*/
	//bRet = ReadRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, &u32Val);
	return (unsigned short)(u32Val & 0xffff);
}

/**
 * @brief This function get OMET green power.
 * @retval G power
*/
unsigned short HAL_OMET_GetGreenPower(VOID)
{
	unsigned int	u32Val = 0;

	/* set register*/
	//bRet = ReadRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, &u32Val);
	return (unsigned short)(u32Val & 0xffff);
}

/**
 * @brief This function get OMET blue power.
 * @retval B power
*/
unsigned short HAL_OMET_GetBluePower(VOID)
{
	unsigned int	u32Val = 0;

	/* set register*/
	//bRet = ReadRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, &u32Val);
	return (unsigned short)(u32Val & 0xffff);
}

/**
 * @brief This function set OMET window setting.
 * @param [in] ubyIndex		Indicate which control, valid value is 0 or 1
 * @param [in] u16HStart	hstart pixel
 * @param [in] u16Width		width pixel, the window is hstart, hstart + width.
 * @param [in] u16VStart	vstart line
 * @param [in] u16Height	height lines, the window is vstart + height
*/
VOID HAL_OMET_SetWindowByIndex(unsigned char ubyIndex, unsigned short u16HStart, unsigned short u16Width, unsigned short u16VStart, unsigned short u16Height)
{
	//unsigned int	uiVal = 0;


	/* set register*/
	//bRet = ReadRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, &uiVal);
	//*pusGain = (unsigned short)(uiVal & 0xffff);
}

/**
 * @brief This function get OMET window setting.
 * @param [in] ubyIndex		Indicate which control, valid value is 0 or 1
 * @param [in] pu16HStart	hstart pixel
 * @param [in] pu16Width	width pixel, the window is hstart, hstart + width.
 * @param [in] pu16VStart	vstart line
 * @param [in] pu16Height	height lines, the window is vstart + height
*/
VOID HAL_OMET_GetWindowByIndex(unsigned char ubyIndex, unsigned short *pu16HStart, unsigned short *pu16Width, unsigned short *pu16VStart, unsigned short *pu16Height)
{
	//unsigned int	uiVal = 0;


	/* set register*/
	//bRet = ReadRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, &uiVal);
	//*pusGain = (unsigned short)(uiVal & 0xffff);
}

/**
 * @brief This function set OMET 3 parameter, dfilt and two threshhold.
 * @param [in] ubyIndex				Indicate which control, valid value is 0 or 1
 * @param [in] u16DFilt				dfilt value setting
 * @param [in] u16ThreshHold0		threshhold0 value setting.
 * @param [in] u16ThreshHold1		threshhold1 value setting.
*/
VOID HAL_OMET_SetCtrlByIndex(unsigned char ubyIndex, unsigned short u16DFilt, unsigned short u16ThreshHold0, unsigned short u16ThreshHold1)
{
	//unsigned int	uiVal = 0;


	/* set register*/
	//bRet = ReadRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, &uiVal);
	//*pusGain = (unsigned short)(uiVal & 0xffff);
}

/**
 * @brief This function get OMET 3 parameter, dfilt and two threshhold.
 * @param [in] ubyIndex				Indicate which control, valid value is 0 or 1
 * @param [out] puiDFilt			dfilt value setting
 * @param [out] puiThreshHold0		threshhold0 value setting.
 * @param [out] puiThreshHold1		threshhold1 value setting.
*/
VOID HAL_OMET_GetCtrlByIndex(unsigned char ubyIndex, unsigned short *pu16DFilt, unsigned short *pu16ThreshHold0, unsigned short *pu16ThreshHold1)
{
	//unsigned int	uiVal = 0;


	/* set register*/
	//bRet = ReadRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, &uiVal);
	//*pusGain = (unsigned short)(uiVal & 0xffff);
}

/**
 * @brief This function get OMET 4 pixel count.
 * @param [out] pu16PixelCount0		Pixel count 0.
 * @param [out] pu16PixelCount1		Pixel count 1.
 * @param [out] pu16PixelCount2		Pixel count 2.
 * @param [out] pu16PixelCount3		Pixel count 3.
*/
VOID HAL_OMET_GetPixelCountByIndex(unsigned short *pu16PixelCount0, unsigned short *pu16PixelCount1, unsigned short *pu16PixelCount2, unsigned short *pu16PixelCount3)
{
	//unsigned int	uiVal = 0;


	/* set register*/
	//bRet = ReadRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, &uiVal);
	//*pusGain = (unsigned short)(uiVal & 0xffff);
}

/**
 * @brief This function get OMET 256*24bits histogram. The histo and OMET result should be calced and new gain should be set before next frame.
		  Typically, its 50Hz or 60Hz, for 100Hz and 120Hz, some frames maybe skipped.
 * @param [in/out] puiHistogram			Histogram buffer
 * @param [in] iBufSize					buffer size.
 * @retval TRUE 		indicates success
 * @retval FALSE		indicates fail.
*/
BOOL HAL_OMET_GetHistogram(unsigned int *puiHistogram, int iBufSize)
{
	BOOL	bRet = FALSE;
	//unsigned int	uiVal = 0;


	/* set register*/
	//bRet = ReadRegister(FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_ADDR no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITSTART no mat,FRC_TOP__kmc_top__reg_top_patt_htotal_ADDR_BITEND no mat, &uiVal);
	//*pusGain = (unsigned short)(uiVal & 0xffff);

	return bRet;
}
