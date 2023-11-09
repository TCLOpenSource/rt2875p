/**
 * @file hal_omet.h
 * @brief This file is for OMET register settings.
 * @date Sep.01.2014
 * @version V0.0.1
 * @author Lixiangjun
 * @par MEMC Library V0.0.1 - Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#ifndef _HAL_OMET_HEADER_
#define _HAL_OMET_HEADER_

#include "kw_common.h"
#include "regio.h"

/**
 * @brief This function set MEMC two gains.
 * @param [in] ubyFirstGain		Specify first gain.
 * @param [in] ubySecondGain	Specify second gain.
*/
VOID HAL_OMET_SetTwoGains(unsigned char ubyFirstGain, unsigned char ubySecondGain);

/**
 * @brief This function get OMET two gains.
 * @param [out] pubyFirstGain	first gain return.
 * @param [out] pubySecondGain	second gain return.
*/
VOID HAL_OMET_GetTwoGains(unsigned char *pubyFirstGain, unsigned char *pubySecondGain);

/**
 * @brief This function set pedestal setting.
 * @param [in] ubyGain		pedestal gain setting
*/
VOID HAL_OMET_SetPedestal(unsigned char ubyGain);

/**
 * @brief This function get pedestal setting.
 * @retval prdestal setting
*/
unsigned char HAL_OMET_GetPedestal(VOID);

/**
 * @brief This function get OMET red power.
 * @retval R power
*/
unsigned short HAL_OMET_GetRedPower(VOID);

/**
 * @brief This function get OMET green power.
 * @retval G power
*/
unsigned short HAL_OMET_GetGreenPower(VOID);

/**
 * @brief This function get OMET blue power.
 * @retval B power
*/
unsigned short HAL_OMET_GetBluePower(VOID);

/**
 * @brief This function set OMET window setting.
 * @param [in] ubyIndex		Indicate which control, valid value is 0 or 1
 * @param [in] u16HStart	hstart pixel
 * @param [in] u16Width		width pixel, the window is hstart, hstart + width.
 * @param [in] u16VStart	vstart line
 * @param [in] u16Height	height lines, the window is vstart + height
*/
VOID HAL_OMET_SetWindowByIndex(unsigned char ubyIndex, unsigned short u16HStart, unsigned short u16Width, unsigned short u16VStart, unsigned short u16Height);

/**
 * @brief This function get OMET window setting.
 * @param [in] ubyIndex		Indicate which control, valid value is 0 or 1
 * @param [in] pu16HStart	hstart pixel
 * @param [in] pu16Width	width pixel, the window is hstart, hstart + width.
 * @param [in] pu16VStart	vstart line
 * @param [in] pu16Height	height lines, the window is vstart + height
*/
VOID HAL_OMET_GetWindowByIndex(unsigned char ubyIndex, unsigned short *pu16HStart, unsigned short *pu16Width, unsigned short *pu16VStart, unsigned short *pu16Height);

/**
 * @brief This function set OMET 3 parameter, dfilt and two threshhold.
 * @param [in] ubyIndex				Indicate which control, valid value is 0 or 1
 * @param [in] u16DFilt				dfilt value setting
 * @param [in] u16ThreshHold0		threshhold0 value setting.
 * @param [in] u16ThreshHold1		threshhold1 value setting.
*/
VOID HAL_OMET_SetCtrlByIndex(unsigned char ubyIndex, unsigned short u16DFilt, unsigned short u16ThreshHold0, unsigned short u16ThreshHold1);

/**
 * @brief This function get OMET 3 parameter, dfilt and two threshhold.
 * @param [in] ubyIndex				Indicate which control, valid value is 0 or 1
 * @param [out] puiDFilt			dfilt value setting
 * @param [out] puiThreshHold0		threshhold0 value setting.
 * @param [out] puiThreshHold1		threshhold1 value setting.
*/
VOID HAL_OMET_GetCtrlByIndex(unsigned char ubyIndex, unsigned short *pu16DFilt, unsigned short *pu16ThreshHold0, unsigned short *pu16ThreshHold1);

/**
 * @brief This function get OMET 4 pixel count.
 * @param [out] pu16PixelCount0		Pixel count 0.
 * @param [out] pu16PixelCount1		Pixel count 1.
 * @param [out] pu16PixelCount2		Pixel count 2.
 * @param [out] pu16PixelCount3		Pixel count 3.
*/
VOID HAL_OMET_GetPixelCountByIndex(unsigned short *pu16PixelCount0, unsigned short *pu16PixelCount1, unsigned short *pu16PixelCount2, unsigned short *pu16PixelCount3);

/**
 * @brief This function get OMET 256*24bits histogram. The histo and OMET result should be calced and new gain should be set before next frame.
		  Typically, its 50Hz or 60Hz, for 100Hz and 120Hz, some frames maybe skipped.
 * @param [in/out] puiHistogram			Histogram buffer
 * @param [in] iBufSize					buffer size.
 * @retval TRUE 		indicates success
 * @retval FALSE		indicates fail.
*/
BOOL HAL_OMET_GetHistogram(unsigned int *puiHistogram, int			 iBufSize);

#endif
