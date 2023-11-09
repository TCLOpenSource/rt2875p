/**
 * @file mid_memc.h
 * @brief This file implements all memc api.
 * @date August.30.2014
 * @version V0.0.1
 * @author Martin Yan
 * @par Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/
#ifndef _MID_MEMC_HEADER_INCLUDE_
#define _MID_MEMC_HEADER_INCLUDE_

#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/memc_type.h"

/**
 * @brief This function set MEMC deblur level. 
 * @param [in] iLevel Specify memc deblur level.
 * @retval TRUE 		indicates success
 * @retval FALSE		indicates fail.
*/
BOOL Mid_MEMC_SetMEMCDeblurLevel(unsigned short u16Level);

/**
 * @brief This function set MEMC dejudderlevel. 
 * @param [in] iLevel Specify memc dejudder level.
 * @retval TRUE 		indicates success
 * @retval FALSE		indicates fail.
*/
BOOL Mid_MEMC_SetMEMCDejudderLevel(unsigned short u16Level);

#if 1  // k4lp
/**
 * @brief This function set MEMC module ratio. 
 * @param [in] unsigned char u8_in_me1_pix_hsft
 * @param [in] unsigned char u8_in_me1_pix_vsft
 * @param [in] unsigned char u8_in_me1_blk_hsft
 * @param [in] unsigned char u8_in_me1_blk_vsft
 * @param [in] unsigned char u8_in_me2_pix_hsft
 * @param [in] unsigned char u8_in_me2_pix_vsft
 * @param [in] unsigned char u8_in_me2_blk_hsft
 * @param [in] unsigned char u8_in_me2_blk_vsft
 * @param [in] unsigned char u8_in_dh_blk_hsft
 * @param [in] unsigned char u8_in_dh_blk_vsft
 * @param [in] unsigned char u8_in_logo_pix_hsft
 * @param [in] unsigned char u8_in_logo_pix_vsft
 * @param [in] unsigned char u8_in_logo_blk_hsft
 * @param [in] unsigned char u8_in_logo_blk_vsft
 * @retval TRUE 		indicates success
 * @retval FALSE		indicates fail.
*/
BOOL Mid_MEMC_SetMEMCModuleRatio(unsigned char u8_in_me1_pix_hsft,
												unsigned char u8_in_me1_pix_vsft,
												unsigned char u8_in_me1_blk_hsft,
												unsigned char u8_in_me1_blk_vsft,
												unsigned char u8_in_me2_pix_hsft,
												unsigned char u8_in_me2_pix_vsft,
												unsigned char u8_in_me2_blk_hsft,
												unsigned char u8_in_me2_blk_vsft,
												unsigned char u8_in_dh_blk_hsft,
												unsigned char u8_in_dh_blk_vsft,
												unsigned char u8_in_logo_pix_hsft,
												unsigned char u8_in_logo_pix_vsft,
												unsigned char u8_in_logo_blk_hsft,
												unsigned char u8_in_logo_blk_vsft);

#else
/**
 * @brief This function set MEMC Module Ratio. 
 * @param [in] unsigned char pix sft.
 * @param [in] unsigned char blk sft.
 * @param [in] unsigned char pix sft.
 * @param [in] unsigned char blk sft.
 * @retval TRUE 		indicates success
 * @retval FALSE		indicates fail.
*/
BOOL Mid_MEMC_SetMEMCModuleRatio(unsigned char u8_in_me1_pix_sft,unsigned char u8_in_me1_blk_sft,unsigned char u8_in_me2_pix_sft,unsigned char u8_in_me2_blk_sft);
#endif

/**
 * @brief This function set MEMC fallback level.
 * @param [in] iLevel Specify memc fallback level.
 * @retval TRUE 		indicates success
 * @retval FALSE		indicates fail.
*/
BOOL Mid_MEMC_SetMEMCFBLevel(unsigned short u16Level);

/**
 * @brief This function set MEMC RIM window. The rim window defines out black region while on film scene.
 * @param [in] u32Top			rim window video vertical start line
 * @param [in] u32Bottom			rim window video vertical end line
 * @param [in] u32Left 			rim window video left start line
 * @param [in] u32Right			rim window video right end line
 * @retval TRUE 				indicates success
 * @retval FALSE				indicates fail
*/
BOOL Mid_MEMC_SetRimWindow(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right);

/**
 * @brief This function set pulldown_mode.
 * @param [in] enInputFrameRate Specify memc input framerate.
 * @param [in] enOutputFrameRate Specify memc output framerate.
 * @param [in] TruMotionMode Specify memc mode(OFF/CLEAR/NORMAL/SMOOTH).
 * @param [in] enRealCinema Specify cinema mode.
 * @retval TRUE 		indicates success
 * @retval FALSE		indicates fail.
*/
BOOL Mid_MEMC_SetPullDownMode(INPUT_FRAMERATE enInputFrameRate, OUTPUT_FRAMERATE enOutoutFrameRate, unsigned char bTruMotionMode, unsigned char enRealCinema);

#endif
