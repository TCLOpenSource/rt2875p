/**
 * @file mid_memc.c
 * @brief This file implements all memc api.
 * @date August.30.2014
 * @version V0.0.1
 * @author Martin Yan
 * @par Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#include "memc_isr/Common/kw_common.h"
#include "memc_isr/Common/kw_debug.h"
#include "memc_isr/MID/mid_memc.h"
#include "memc_isr/Common/memc_type.h"
#include "memc_isr/include/PQLAPI.h"
#include "memc_isr/PQL/PQLPlatformDefs.h"

#include <rtd_log/rtd_module_log.h>

/**
 * @brief This function set MEMC deblur level.
 * @param [in] iLevel Specify memc deblur level.
 * @retval TRUE 		indicates success
 * @retval FALSE		indicates fail.
*/
BOOL Mid_MEMC_SetMEMCDeblurLevel(unsigned short u16Level)
{
	BOOL	bRet = TRUE;
	_PQL_Command Command;
	rtd_pr_memc_notice("[%s] deblur=%d\n ", __func__, u16Level );
	Command.u4_memc_mode_deblur_lvl = (unsigned char)u16Level;
	PQL_Command(PQL_CMD_MEMC_MODE_DEBLUR, &Command);
	return bRet;
}

/**
 * @brief This function set MEMC dejudderlevel.
 * @param [in] iLevel Specify memc dejudder level.
 * @retval TRUE 		indicates success
 * @retval FALSE		indicates fail.
*/
BOOL Mid_MEMC_SetMEMCDejudderLevel(unsigned short u16Level)
{
	BOOL	bRet = TRUE;
	_PQL_Command Command;
	rtd_pr_memc_notice("[%s] dejudder=%d\n ", __func__, u16Level );
	Command.u4_memc_mode_dejudder_lvl = (unsigned char)u16Level;
	PQL_Command(PQL_CMD_MEMC_MODE_DEJUDDER, &Command);
	return bRet;
}

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
												unsigned char u8_in_logo_blk_vsft)
{
	BOOL	bRet = TRUE;
	_PQL_Command Command;

	Command._stc_memc_module_ratio.u8_in_me1_pix_hsft = u8_in_me1_pix_hsft;
	Command._stc_memc_module_ratio.u8_in_me1_pix_vsft = u8_in_me1_pix_vsft;
	Command._stc_memc_module_ratio.u8_in_me1_blk_hsft = u8_in_me1_blk_hsft;
	Command._stc_memc_module_ratio.u8_in_me1_blk_vsft = u8_in_me1_blk_vsft;
	Command._stc_memc_module_ratio.u8_in_me2_pix_hsft = u8_in_me2_pix_hsft;
	Command._stc_memc_module_ratio.u8_in_me2_pix_vsft = u8_in_me2_pix_vsft;
	Command._stc_memc_module_ratio.u8_in_me2_blk_hsft = u8_in_me2_blk_hsft;
	Command._stc_memc_module_ratio.u8_in_me2_blk_vsft = u8_in_me2_blk_vsft;
	Command._stc_memc_module_ratio.u8_in_dh_blk_hsft = u8_in_dh_blk_hsft;
	Command._stc_memc_module_ratio.u8_in_dh_blk_vsft = u8_in_dh_blk_vsft;
	Command._stc_memc_module_ratio.u8_in_logo_pix_hsft = u8_in_logo_pix_hsft;
	Command._stc_memc_module_ratio.u8_in_logo_pix_vsft = u8_in_logo_pix_vsft;
	Command._stc_memc_module_ratio.u8_in_logo_blk_hsft = u8_in_logo_blk_hsft;
	Command._stc_memc_module_ratio.u8_in_logo_blk_vsft = u8_in_logo_blk_vsft;

	PQL_Command(PQL_CMD_MEMC_MODULE_RATIO, &Command);
	return bRet;
}

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
BOOL Mid_MEMC_SetMEMCModuleRatio(unsigned char u8_in_me1_pix_sft,unsigned char u8_in_me1_blk_sft,unsigned char u8_in_me2_pix_sft,unsigned char u8_in_me2_blk_sft)
{
	BOOL	bRet = TRUE;
	_PQL_Command Command;
	Command._stc_memc_module_ratio.u8_in_me1_pix_sft = u8_in_me1_pix_sft;
	Command._stc_memc_module_ratio.u8_in_me1_blk_sft = u8_in_me1_blk_sft;
	Command._stc_memc_module_ratio.u8_in_me2_pix_sft = u8_in_me2_pix_sft;
	Command._stc_memc_module_ratio.u8_in_me2_blk_sft = u8_in_me2_blk_sft;
	PQL_Command(PQL_CMD_MEMC_MODULE_RATIO, &Command);
	return bRet;
}
#endif

/**
 * @brief This function set MEMC fallback level.
 * @param [in] iLevel Specify memc fallback level.
 * @retval TRUE 		indicates success
 * @retval FALSE		indicates fail.
*/
BOOL Mid_MEMC_SetMEMCFBLevel(unsigned short u16Level)
{
	BOOL	bRet = TRUE;
	_PQL_Command Command;
	Command.u4_memc_mode_fb_lvl = (unsigned char)u16Level;
	PQL_Command(PQL_CMD_MEMC_MODE_FB, &Command);
	return bRet;
}

/**
 * @brief This function set MEMC RIM window. The rim window defines out black region while on film scene.
 * @param [in] u32Top			rim window video vertical start line
 * @param [in] u32Bottom			rim window video vertical end line
 * @param [in] u32Left 			rim window video left start line
 * @param [in] u32Right			rim window video right end line
 * @retval TRUE 				indicates success
 * @retval FALSE				indicates fail
*/
BOOL Mid_MEMC_SetRimWindow(unsigned int u32Top, unsigned int u32Bottom, unsigned int u32Left, unsigned int u32Right)
{
	BOOL	bRet = FALSE;

	return bRet;
}

/**
 * @brief This function set pulldown_mode.
 * @param [in] enInputFrameRate Specify memc input framerate.
 * @param [in] enOutputFrameRate Specify memc output framerate.
 * @param [in] TruMotionMode Specify memc mode(OFF/CLEAR/NORMAL/SMOOTH).
 * @param [in] enRealCinema Specify cinema mode.
 * @retval TRUE 		indicates success
 * @retval FALSE		indicates fail.
*/
BOOL Mid_MEMC_SetPullDownMode(INPUT_FRAMERATE enInputFrameRate, OUTPUT_FRAMERATE enOutputFrameRate, unsigned char bTruMotionMode, unsigned char enRealCinema)
{
	BOOL	bRet = TRUE;
	BOOL	bEnRealCinema = (enRealCinema == TRUE);
	PQL_OUTPUT_PULLDOWN_MODE	mPullDownMode = _PQL_PULLDOWN_MODE_ELSE;
	_PQL_Command Command;

	// 2D/3D
	// all@24hz in, TruMotion off, Cinema on : 55 pull down@120hz out, 22 pull down@48hz out
	// all@24hz in, TruMotion off, Cinema off : 64 pull down@120hz out, 32 pull down@60hz out
	// all@25hz in, TruMotion off, Cinema off : 44 pull down@100hz out, 22 pull down@50hz out
	// all@30hz in, TruMotion off, Cinema off : 44 pull down@120hz out, 22 pull down@60hz out
	// all@50hz in, TruMotion off, Cinema off : 22 pull down@100hz out, bypass@50hz out
	// all@60hz in, TruMotion off, Cinema off : 22 pull down@120hz out, bypass@60hz out
	// all@60hz 32,TruMotion off, Cinema on : 55 pull down@120hz out, 32 pull down@60hz out
	// all@60hz 32,TruMotion off, Cinema off : 64 pull down@120hz out, 32 pull down@60hz out
	if(enInputFrameRate == INPUT_FRAMERATE_24HZ && (enOutputFrameRate == OUTPUT_FRAMERATE_48HZ || enOutputFrameRate == OUTPUT_FRAMERATE_120HZ)\
		&& bTruMotionMode == FALSE && bEnRealCinema == TRUE)
	{
		mPullDownMode = _PQL_IN24HZ_TRUMOTOFF_CINEMAON;
	}
	else if(enInputFrameRate == INPUT_FRAMERATE_24HZ && (enOutputFrameRate == OUTPUT_FRAMERATE_60HZ || enOutputFrameRate == OUTPUT_FRAMERATE_120HZ)\
		&& bTruMotionMode == FALSE && bEnRealCinema == FALSE)
	{
		mPullDownMode = _PQL_IN24HZ_TRUMOTOFF_CINEMAOFF;
	}
	else if(enInputFrameRate == INPUT_FRAMERATE_25HZ && (enOutputFrameRate == OUTPUT_FRAMERATE_50HZ || enOutputFrameRate == OUTPUT_FRAMERATE_100HZ)\
		&& bTruMotionMode == FALSE && bEnRealCinema == FALSE)
	{
		mPullDownMode = _PQL_IN25HZ_TRUMOTOFF_CINEMAOFF;
	}
	else if(enInputFrameRate == INPUT_FRAMERATE_30HZ && (enOutputFrameRate == OUTPUT_FRAMERATE_60HZ || enOutputFrameRate == OUTPUT_FRAMERATE_120HZ)\
		&& bTruMotionMode == FALSE && bEnRealCinema == FALSE)
	{
		mPullDownMode = _PQL_IN30HZ_TRUMOTOFF_CINEMAOFF;
	}
	else if(enInputFrameRate == INPUT_FRAMERATE_50HZ && (enOutputFrameRate == OUTPUT_FRAMERATE_50HZ || enOutputFrameRate == OUTPUT_FRAMERATE_100HZ)\
		&& bTruMotionMode == FALSE && bEnRealCinema == FALSE)
	{
		mPullDownMode = _PQL_IN50HZ_TRUMOTOFF_CINEMAOFF;
	}
	else if(enInputFrameRate == INPUT_FRAMERATE_60HZ && (/*enOutputFrameRate == OUTPUT_FRAMERATE_60HZ || */enOutputFrameRate == OUTPUT_FRAMERATE_120HZ)\
		&& bTruMotionMode == FALSE && bEnRealCinema == TRUE)
	{
		mPullDownMode = _PQL_IN60HZ_TRUMOTOFF_CINEMAON;
	}
	else if(enInputFrameRate == INPUT_FRAMERATE_60HZ && (/*enOutputFrameRate == OUTPUT_FRAMERATE_60HZ || */enOutputFrameRate == OUTPUT_FRAMERATE_120HZ)\
		&& bTruMotionMode == FALSE && bEnRealCinema == FALSE)
	{
		mPullDownMode = _PQL_IN60HZ_TRUMOTOFF_CINEMAOFF;
	}
	else if(bTruMotionMode == FALSE)
	{
		mPullDownMode = _PQL_INXHZ_TRUMOTOFF;
	}

	Command.u8_pulldown_mode= mPullDownMode;
//	rtd_pr_memc_info("[MEMC]mPullDownMode = %d (InFR=%d, OutFR=%d, isMemcON=%d, isCinemaON=%d)\r\n",
//		mPullDownMode, enInputFrameRate, enOutputFrameRate, bTruMotionMode,bEnRealCinema);
	PQL_Command(PQL_CMD_PULLDOWNMODE, &Command);

	rtd_pr_memc_notice("mPullDownMode = %d (InFR=%d, OutFR=%d, isMemcON=%d, isCinemaON=%d)\r\n",
		mPullDownMode, enInputFrameRate, enOutputFrameRate, bTruMotionMode,bEnRealCinema);

	return bRet;
}




