/*==========================================================================
    * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/

/**
 * @file
 * 	This file is for PQMask tuning pq related parameters functions.
 *
 * @author 	Wei Yuan Hsu
 * @date 	20210323
 * @version 0.1
 */

/****************************** Header files ******************************/
// platform
#include <rtd_log/rtd_module_log.h>
#include <mach/rtk_platform.h>
#include <scaler/vipRPCCommon.h>
#include <linux/string.h>
// library
#include <rtk_ai.h>
#include <tvscalercontrol/vip/scalerPQMaskColorLib.h>
#include <tvscalercontrol/vip/scalerPQMaskCtrl.h>
#include <rtk_kadp_se.h>
#include <tvscalercontrol/vip/ai_pq.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/vip/localcontrast.h>
#include <tvscalercontrol/vip/icm.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <vgip_isr/scalerAI.h>
#include <scaler/vipCommon.h>
/****************************** Configurations ******************************/
#define TAG_NAME "VPQMASK_COLOR"

/****************************** Gobal module parameters ******************************/
static unsigned int PQMaskPalette[PQMASK_COLOR_PALETTE_MAX][MODULE_GRP_MAX][PQMASK_MAPCURVE_LEN];
static PQMASK_COLOR_MGR_T PQMaskColorMgr = {
	// ModuleSetting
	{
		// Nr
		{
			1,
			1,
			1,
		},
		// ICM
		{
			0, 0,
			1, 32, 0,
			0, 32, 0,
		},
		// Sharpness
		{
			1,
			1,
			{ 6, 17, 28, 39, 50, 90, 130, 170, 210 }, // SobelUpbnd
			1,
			{ 5, 15, 25, 40, 50, 75, 100, 125, 150 }, // ZDiffUpbnd
			1,
			{ 32, 28, 24, 20, 8, 4, 2, 1, 0 }, // TextUpbnd
			1,
			{ 44, 32, 21, 10, 0, 0, 0, -12, -29, }, // TextLV
			{ -72, -50, -36, -24, 0, 0, 18, 36, 64, }, // TextGainPos
			{ -72, -50, -36, -24, 0, 0, 18, 36, 64, }, // TextGainPosV
			{ -104, -68,  -54, -24, -5, 0, 26, 54, 108, }, // TextGainNeg
			{ -104, -68,  -54, -24, -5, 0, 26, 54, 108, }, // TextGainNegV
			{ -72, -50, -36, -24, 0, 0, 18, 36, 64, }, // TextHVPos
			{ -72, -50, -36, -24, 0, 0, 18, 36, 64, }, // TextHVPosV
			{ -104, -68,  -54, -24, -5, 0, 26, 54, 108, }, // TextHVNeg
			{ -104, -68,  -54, -24, -5, 0, 26, 54, 108, }, // TextHVNegV
			{ 39, 29, 19, 0, 0, 0, -10, -24, }, // TextLVV
		},
		// LocalContrast
		{
			1, 0,
			1, 64,
		}
	},
	// SemanticWeightTbl
	{
		//	basic,	sky,	people,	sea,	plant,	arti,
		//											text
		{	128,	196,	128,	128,	128,	 64	},	// NR
		{	128,	192,	128,	160,	128,	128	},	// DECON
		{	128,	192,	128,	 96,	184,	160	},	// EDGE
		{	128,	 16,	128,	192,	216,	248	},	// TEXT
		{	128,	128,	128,	128,	128,	128	},	// H
		{	128,	176,	128,	128,	148,	128	},	// S
		{	128,	130,	128,	128,	116,	128	},	// I
		{	128,	 32,	128,	192,	208,	224 },	// LC
	},
	// DepthWeightTbl
	{
		// NR
		{
			/*  0*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 16*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 32*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 48*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 64*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 80*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 96*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*112*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*128*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*144*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*160*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*176*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*192*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*208*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*224*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*240*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		},
		// DECON
		{
			/*  0*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 16*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 32*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 48*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 64*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 80*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 96*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*112*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*128*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*144*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*160*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*176*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*192*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*208*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*224*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*240*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		},
		// EDGE
		{
			/*  0*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 16*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 32*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 48*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 64*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 80*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 96*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*112*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*128*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*144*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*160*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*176*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*192*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*208*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*224*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*240*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		},
		// TEXT
		{
			/*  0*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 16*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 32*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 48*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 64*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 80*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 96*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*112*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*128*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*144*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*160*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*176*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*192*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*208*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*224*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*240*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		},
		// HUE
		{
			/*  0*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 16*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 32*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 48*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 64*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 80*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 96*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*112*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*128*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*144*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*160*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*176*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*192*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*208*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*224*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*240*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		},
		// SAT
		{
			/*  0*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 16*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 32*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 48*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 64*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 80*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 96*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*112*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*128*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*144*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*160*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*176*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*192*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*208*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*224*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*240*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
 		},
		// INT
		{
			/*  0*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 16*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 32*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 48*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 64*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 80*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 96*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*112*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*128*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*144*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*160*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*176*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*192*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*208*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*224*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*240*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		},
		// LC
		{
			/*  0*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 16*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 32*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 48*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 64*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 80*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/* 96*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*112*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*128*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*144*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*160*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*176*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*192*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*208*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*224*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
			/*240*/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		},
	},
	// depth histogram
	{
		// info
		{ 0, AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, 4, 4 },
		// data
		{
			// pts histogram
			0, { 0, },
			// percentage histogram
			0, { 0, },
		}
	},
	// semantic histogram
	{
		// info
		{ 0, AI_SEMANTIC_OUTPUT_WIDTH, AI_SEMANTIC_OUTPUT_HEIGHT, 4, 4 },
		// data
		{
			// pts histogram
			0, { 0, },
			// percentage histogram
			0, { 0, },
		}
	},
	// image by semantic histogram, calculated from ap
	{
		{
			{
				// info
				{ 0, AI_SEMANTIC_OUTPUT_WIDTH, AI_SEMANTIC_OUTPUT_HEIGHT, 4, 4 },
				// data
				{
					// pts histogram
					0, { 0, },
					// percentage histogram
					0, { 0, },
				}
			},
		},
	},
	// image by depth histogram
	{ 0, AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, 4, 4 },
	50, // low thl, far <= thl < mid
	200, // high thl, mid <= thl < near
	{
		// far
		{
			// channel y
			{
				// info
				{ 1, AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, 4, 4 },
				// data
				{
					// pts histogram
					0, { 0, },
					// percentage histogram
					0, { 0, },
				}
			},
			// channel u
			{
				// info
				{ 1, AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, 4, 4 },
				// data
				{
					// pts histogram
					0, { 0, },
					// percentage histogram
					0, { 0, },
				}
			},
			// channel v
			{
				// info
				{ 1, AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, 4, 4 },
				// data
				{
					// pts histogram
					0, { 0, },
					// percentage histogram
					0, { 0, },
				}
			},
		},
		// mid
		{
			// channel y
			{
				// info
				{ 1, AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, 4, 4 },
				// data
				{
					// pts histogram
					0, { 0, },
					// percentage histogram
					0, { 0, },
				}
			},
			// channel u
			{
				// info
				{ 1, AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, 4, 4 },
				// data
				{
					// pts histogram
					0, { 0, },
					// percentage histogram
					0, { 0, },
				}
			},
			// channel v
			{
				// info
				{ 1, AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, 4, 4 },
				// data
				{
					// pts histogram
					0, { 0, },
					// percentage histogram
					0, { 0, },
				}
			},
		},
		// near
		{
			// channel y
			{
				// info
				{ 1, AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, 4, 4 },
				// data
				{
					// pts histogram
					0, { 0, },
					// percentage histogram
					0, { 0, },
				}
			},
			// channel u
			{
				// info
				{ 1, AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, 4, 4 },
				// data
				{
					// pts histogram
					0, { 0, },
					// percentage histogram
					0, { 0, },
				}
			},
			// channel v
			{
				// info
				{ 1, AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, 4, 4 },
				// data
				{
					// pts histogram
					0, { 0, },
					// percentage histogram
					0, { 0, },
				}
			},
		},
	},
	// Status
	{
		0,	// bSceneChange : scene change signal
		0,	// SCRstFrm : scene change frames
		0,	// LocalContrastRstWeight : just record the current LC reset weighting
		0,	// ForceCurFrm : force the current output for a period of times
		1,	// bReloadDepPalette : to reload palette when depth weighting table change
		1,	// bReloadSemPalette : to reload palette when semantic weighting table change
		AI_GENRE_TYPE_NUM,	// AiGenre : for PQ tuning
		0,	// bGenerCGStatusChange : AiGenre result CG pattern change (enter/leave CG pattern)
		0,	// Genre_CG_Frm : CG Pattern TNR frames
		0,	// bReloadDepthPalette_AiGenre_CG : to reload the depth palette when CG pattern detected
		0,	// bReloadSemanticPalette_AiGenre_CG : to reload the semantic palette when CG pattern detected
	},
	// Parameters
	{
		255, 0, 	// FusionRatio_Dep/FusionRatio_Sem : blending weighting on Depth/Semantic
		223, 32, 	// FusionRatio_Pre/FusionRatio_Cur : blending weighting on Pre/Cur
		1, 			// SpatialFltCycle : Cycle
		{    0, 1368,    0, 1368 },	// SEHorCoef : Spatial filter SE horizontal coefficient
		{    0, 1368,    0, 1368 },	// SEVerCoef : Spatial filter SE vertical coefficient
		1, 							// HorOddType : SE hor FIR type ( 0:even 1:odd )
		1, 							// VerOddType : SE ver FIR type ( 0:even 1:odd )
		/* Scene change control */
		1,			// bSceneChangeEnable : enable or disable scene change detection
		320, 		// SceneChangeRstFrm : frame to bypass curve and frame to target cruve
		207, 48 , 	// ScFusionRatio_toRst_Pre/ScFusionRatio_toRst_Cur : blending weight on Pre/Cur toward reset when scene change happend
		247, 8  , 	// ScFusionRatio_toTarget_Pre/ScFusionRatio_toTarget_Cur : blending weight on Pre/Cur toward target when scene change happend
		/* Genre CG pattern control */
		1,			// bGenre_CG_Enable : enable or disable cg pattern detection
		32 ,		// Genre_CG_toTargetFrm : frame to bypass curve and frame to target cruve
		239,  16,	// Genre_CG_FunctionRatio_Pre/Genre_CG_FunctionRatio_Cur : blending weight on Pre/Cur toward target when enter/leave CG pattern
	}
};

/****************************** Internal Functions ******************************/

/****************************** Functions ******************************/

/**
 * @brief
 * init the PQ module settings
 */
void scalerPQMaskColor_init(void) {

	DRV_LC_ToneMapping_PQmask LCTmap;
	DRV_LC_Sharpness_PQmask LCSharp;
	void * TablePtr = NULL;
	int i=0, j=0;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	// initial some strutcture
	memset(&PQMaskColorMgr.ImgByDepthHist , 0, PQMASK_DEPDIST_NUM*PQMASK_IMGCH_NUM*sizeof(PQMASK_HIST_T)); // info column is not important
	memset(&PQMaskColorMgr.Status, 0, sizeof(PQMASK_COLOR_STATUS_T));
	memset(&PQMaskColorMgr.SemanticHist.Data, 0, sizeof(PQMASK_HIST_DATA_T));
	memset(&PQMaskColorMgr.DepthHist.Data, 0, sizeof(PQMASK_HIST_DATA_T));
	for(i=0; i<PQMASK_LABEL_NUM; i++) {
		for(j=0; j<PQMASK_IMGCH_NUM; j++) {
			// first unit info is the real information
			if( i==0 && j==0 ) {
				memset(&PQMaskColorMgr.ImgBySemanticHist[i][j].Data, 0, sizeof(PQMASK_HIST_DATA_T));
			} else {
				memset(&PQMaskColorMgr.ImgBySemanticHist[i][j], 0, sizeof(PQMASK_HIST_T));
			}
		}
	}
	memset(&LCTmap, 0, sizeof(DRV_LC_ToneMapping_PQmask));
	memset(&LCSharp, 0, sizeof(DRV_LC_Sharpness_PQmask));
	memset(PQMaskPalette, 0x80, sizeof(unsigned int)*PQMASK_COLOR_PALETTE_MAX*MODULE_GRP_MAX*PQMASK_MAPCURVE_LEN); // reset to 128 to avoid some obvious problem

	// copy semantic/depth weighting from table
	TablePtr = scaler_VIP_PQMask_GetSemanticTable(1);	
	if( TablePtr != NULL ) {
		memcpy((void *)PQMaskColorMgr.SemanticWeightTbl, TablePtr, PQMASK_MODULE_NUM*PQMASK_LABEL_NUM*sizeof(unsigned char));
	} else {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] init default semantic weight table fail\n", __FUNCTION__);
	}
	scalerPQMaskColor_SetReloadPalette(PQMASK_COLOR_PALETTE_SEMANTIC);


	TablePtr = scaler_VIP_PQMask_GetDepthTable(1);
	if( TablePtr != NULL ) {
		memcpy((void *)PQMaskColorMgr.DepthWeightTbl, TablePtr, PQMASK_MODULE_NUM*PQMASK_MAPCURVE_LEN*sizeof(unsigned char));
	} else {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] init default depth weight table fail\n", __FUNCTION__);
	}
	scalerPQMaskColor_SetReloadPalette(PQMASK_COLOR_PALETTE_DEPTH);

	// NR
	// (drvif_color_DRV_SNR_PQ_Mask)(drvif_color_Set_Block_DeContour_Ctrl), setting in vip_default_panel.c (semantic_en)
	drvif_color_DRV_SNR_PQ_Mask_Enable(PQMaskColorMgr.ModuleSetting.Nr.bNrEnable);
	// De-contour
	// setting in pq_misc_table (semantic_alpha)
	drvif_color_Set_I_De_Contour_PQ_Mask_Enable(PQMaskColorMgr.ModuleSetting.Nr.bIDecontEnable);
	drvif_color_Set_Block_DeContour_PQ_Mask_Enable(PQMaskColorMgr.ModuleSetting.Nr.bIBlockDecontEnable);

	// ICM
	drvif_color_icm_Set_pqmask_hue(PQMaskColorMgr.ModuleSetting.ICM.bHueEnable, PQMaskColorMgr.ModuleSetting.ICM.HueGain);
	drvif_color_icm_Set_pqmask_sat(PQMaskColorMgr.ModuleSetting.ICM.bSatEnable, PQMaskColorMgr.ModuleSetting.ICM.SatGain, PQMaskColorMgr.ModuleSetting.ICM.SatOffset);
	drvif_color_icm_Set_pqmask_int(PQMaskColorMgr.ModuleSetting.ICM.bIntEnable, PQMaskColorMgr.ModuleSetting.ICM.IntGain, PQMaskColorMgr.ModuleSetting.ICM.IntOffset);
	drvif_color_icm_Set_pqmask_debug_gain(3);

	// Sharpness
	scalerPQMaskColor_UpdateLabelPK();

	// Localcontrast
	// TmapFactor = (weight/2 - offset)
	// ShpRatio = ((ratio*weight/128) * gain) / 64
	LCTmap.lc_PQmask_tmap_En = PQMaskColorMgr.ModuleSetting.LocalContrast.bToneMappingEnable;
	LCTmap.lc_PQmask_offset = PQMaskColorMgr.ModuleSetting.LocalContrast.ToneMappingOffset;
	LCSharp.lc_PQmask_shp_En = PQMaskColorMgr.ModuleSetting.LocalContrast.bSharpEnable;
	LCSharp.lc_PQmask_shp_gain = PQMaskColorMgr.ModuleSetting.LocalContrast.SharpGain;
	LCSharp.lc_PQmask_shp_debug_shift = 0;
	drvif_color_set_LC_Tonemapping_PQmask(&LCTmap);
	drvif_color_set_LC_Sharpness_PQmask(&LCSharp);

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] end\n", __FUNCTION__);
}

/**
 * @brief
 * clean up the related resource
 */
void scalerPQMaskColor_exit(void) {
	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);
	rtd_printk(KERN_INFO, TAG_NAME, "[%s] end\n", __FUNCTION__);
}

/**
 * @brief
 * disable all the PQ mask related HW block
 */
void scalerPQMaskColor_disable(void) {

	DRV_LC_ToneMapping_PQmask LCTmap;
	DRV_LC_Sharpness_PQmask LCSharp;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	memset(&LCTmap, 0x0, sizeof(DRV_LC_ToneMapping_PQmask));
	memset(&LCSharp, 0x0, sizeof(DRV_LC_Sharpness_PQmask));
	LCSharp.lc_PQmask_shp_gain = 64;

	drvif_color_DRV_SNR_PQ_Mask_Enable(0);
	drvif_color_Set_I_De_Contour_PQ_Mask_Enable(0);
	drvif_color_Set_Block_DeContour_PQ_Mask_Enable(0);
	drvif_color_icm_Set_pqmask_hue(0, 32);
	drvif_color_icm_Set_pqmask_sat(0, 32, 0);
	drvif_color_icm_Set_pqmask_int(0, 32, 0);
	drvif_set_labelpk_enable(0);
	drvif_color_set_LC_Tonemapping_PQmask(&LCTmap);
	drvif_color_set_LC_Sharpness_PQmask(&LCSharp);

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] end\n", __FUNCTION__);
}

/**
 * @brief
 * get the internal structure pointer
 * @param InfoIndex
 * info index
 * @return void*
 * pointer to the specific structure
 */
void *scalerPQMaskColor_GetStruct(unsigned int InfoIndex) {
	void * RetPtr = NULL;
	switch(InfoIndex) {
		case PQMASK_COLORSTRUCT_MGR:
			RetPtr = (void *)&PQMaskColorMgr;
			break;
		case PQMASK_COLORSTRUCT_SEMATNIIC_HISTO:
			RetPtr = (void *)&PQMaskColorMgr.SemanticHist;
			break;
		case PQMASK_COLORSTRUCT_DEPTH_HISTO:
			RetPtr = (void *)&PQMaskColorMgr.DepthHist;
			break;
		case PQMASK_COLORSTRUCT_PALETTE:
			RetPtr = (void *)PQMaskPalette;
			break;
		default:
			rtd_printk(KERN_EMERG, TAG_NAME, "[%s] index = %d error\n", __FUNCTION__, InfoIndex);
			RetPtr = NULL;
			break;
	}
	return RetPtr;
}

/**
 * @brief
 * generate the semantic information histogram
 * Hypothesis
 * 		1. semantic information is 8 bit data, value range = 0~(PQMASK_MODULE_NUM-1)
 */
void scalerPQMaskColor_SemanticHistogram(void *pInput) {

	int px=0, py=0;
	int histIdx=0;
	unsigned char *pDat = (unsigned char *)pInput;

#if 0 // time measurement
	unsigned int t0=0, t1=0;
	t0 = drvif_Get_90k_Lo_clk();
#endif

	if( PQMaskColorMgr.SemanticHist.Info.enable == 0 )
		return;

	// error protection
	if( (PQMaskColorMgr.SemanticHist.Info.hJmp) == 0 || (PQMaskColorMgr.SemanticHist.Info.vJmp == 0) ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d] hJmp = %d, vJmp = %d, error",
			__FUNCTION__, __LINE__, PQMaskColorMgr.SemanticHist.Info.hJmp, PQMaskColorMgr.SemanticHist.Info.vJmp);
		return;
	}

	memset(&PQMaskColorMgr.SemanticHist.Data, 0, sizeof(PQMASK_HIST_DATA_T));

	for(py=0; py<(PQMaskColorMgr.SemanticHist.Info.height); py=py+(PQMaskColorMgr.SemanticHist.Info.vJmp)) {
		for(px=0; px<(PQMaskColorMgr.SemanticHist.Info.width); px=px+(PQMaskColorMgr.SemanticHist.Info.hJmp)) {
			histIdx = pDat[py*PQMaskColorMgr.SemanticHist.Info.width+px];
			if( histIdx < PQMASK_MODULE_NUM ) {
				PQMaskColorMgr.SemanticHist.Data.Histogram[histIdx]++;
				PQMaskColorMgr.SemanticHist.Data.TotalPts++;
			} else {
				// rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d] index %d over maximum %d, error\n", __FUNCTION__, __LINE__, histIdx, PQMASK_MODULE_NUM);
			}
		}
	}

	PQMaskColorMgr.SemanticHist.Data.TotalPts = PQMaskColorMgr.SemanticHist.Data.TotalPts | 0x1;
	for(px=0; px<PQMASK_MODULE_NUM; px++) {
		PQMaskColorMgr.SemanticHist.Data.NorHist[px] = (((PQMaskColorMgr.SemanticHist.Data.Histogram[px]*1000)+500) / PQMaskColorMgr.SemanticHist.Data.TotalPts);
		PQMaskColorMgr.SemanticHist.Data.TotalPercent = PQMaskColorMgr.SemanticHist.Data.TotalPercent + PQMaskColorMgr.SemanticHist.Data.NorHist[px];
	}

#if 0 // time measurement
	t1 = drvif_Get_90k_Lo_clk();
	rtd_printk(KERN_EMERG, TAG_NAME, "90k diff = %d\n", t1-t0);
#endif

#if 0 // debug log
	for(px=0; px<PQMASK_MODULE_NUM; px++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] %4d/1000, %d\n", px, PQMaskColorMgr.SemanticHist.Data.NorHist[px], PQMaskColorMgr.SemanticHist.Data.Histogram[px]);
	}
	rtd_printk(KERN_EMERG, TAG_NAME, "Total %d pts, Total %d percentage\n", PQMaskColorMgr.SemanticHist.Data.TotalPts, PQMaskColorMgr.SemanticHist.Data.TotalPercent);
#endif

}

/**
 * @brief
 * generate the depth information histogram
 * Hypothesis
 * 		1. depth information is 8 bit data, value range is 0~255
 */
void scalerPQMaskColor_DepthHistogram(void *pInput) {

	int px=0, py=0;
	int histIdx=0;
	unsigned char *pDat = (unsigned char *)pInput;

#if 0 // time measurement
	unsigned int t0=0, t1=0;
	t0 = drvif_Get_90k_Lo_clk();
#endif

	if( PQMaskColorMgr.DepthHist.Info.enable == 0 )
		return;

	// error protection
	if( (PQMaskColorMgr.DepthHist.Info.hJmp) == 0 || (PQMaskColorMgr.DepthHist.Info.vJmp == 0) ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d] hJmp = %d, vJmp = %d, error",
			__FUNCTION__, __LINE__, PQMaskColorMgr.DepthHist.Info.hJmp, PQMaskColorMgr.DepthHist.Info.vJmp);
		return;
	}

	memset(&PQMaskColorMgr.DepthHist.Data, 0, sizeof(PQMASK_HIST_DATA_T));

	for(py=0; py<(PQMaskColorMgr.DepthHist.Info.height); py=py+(PQMaskColorMgr.DepthHist.Info.vJmp)) {
		for(px=0; px<(PQMaskColorMgr.DepthHist.Info.width); px=px+(PQMaskColorMgr.DepthHist.Info.hJmp)) {
			histIdx = pDat[py*PQMaskColorMgr.DepthHist.Info.width+px] >> (8-PQMASK_HISTO_BIN_SFT);
			if( histIdx < PQMASK_HISTO_BIN ) {
				PQMaskColorMgr.DepthHist.Data.Histogram[histIdx]++;
				PQMaskColorMgr.DepthHist.Data.TotalPts++;
			} else {
				rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d] index %d over maximum %d, error\n", __FUNCTION__, __LINE__, histIdx, PQMASK_HISTO_BIN);
			}
		}
	}

	PQMaskColorMgr.DepthHist.Data.TotalPts = PQMaskColorMgr.DepthHist.Data.TotalPts | 0x1;
	for(px=0; px<PQMASK_HISTO_BIN; px++) {
		PQMaskColorMgr.DepthHist.Data.NorHist[px] = (((PQMaskColorMgr.DepthHist.Data.Histogram[px]*1000)+500) / PQMaskColorMgr.DepthHist.Data.TotalPts);
		PQMaskColorMgr.DepthHist.Data.TotalPercent = PQMaskColorMgr.DepthHist.Data.TotalPercent + PQMaskColorMgr.DepthHist.Data.NorHist[px];
	}

#if 0 // time measurement
	t1 = drvif_Get_90k_Lo_clk();
	rtd_printk(KERN_EMERG, TAG_NAME, "90k diff = %d\n", t1-t0);
#endif

#if 0 // debug log
	for(px=0; px<PQMASK_HISTO_BIN; px++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] %4d/1000, %d\n", px, PQMaskColorMgr.DepthHist.Data.NorHist[px], PQMaskColorMgr.DepthHist.Data.Histogram[px]);
	}
	rtd_printk(KERN_EMERG, TAG_NAME, "Total %d pts, Total %d percentage\n", PQMaskColorMgr.DepthHist.Data.TotalPts, PQMaskColorMgr.DepthHist.Data.TotalPercent);
#endif

}

/**
 * @brief
 * generate the image vs. depth information histogram
 * @param pImgInput : input data with NV12
 * @param pInfoInput : input depth information
 */
void scalerPQMaskColor_Img2DepthHistogram(void *pImgInput, void *pInfoInput) {

	int px=0, py=0;
	int sectionIdx = 0, histIdx=0;
	int yDatIdx = 0, uvDatIdx = 0;
	int yDat = 0, uDat = 0, vDat = 0;
	unsigned char *pDat = (unsigned char *)pInfoInput;
	unsigned char *pImg = (unsigned char *)pImgInput;

#if 0 // time measurement
	unsigned int t0=0, t1=0;
	t0 = drvif_Get_90k_Lo_clk();
#endif

	if( PQMaskColorMgr.ImgByDepthHistInfo.enable == 0 ) {
		return;
	}

	// error protection
	if( (PQMaskColorMgr.ImgByDepthHistInfo.hJmp) == 0 || (PQMaskColorMgr.ImgByDepthHistInfo.vJmp == 0) ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d] hJmp = %d, vJmp = %d, error",
			__FUNCTION__, __LINE__, PQMaskColorMgr.ImgByDepthHistInfo.hJmp, PQMaskColorMgr.ImgByDepthHistInfo.vJmp);
		return;
	}

	if( PQMaskColorMgr.ThlLow > PQMaskColorMgr.ThlHigh ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d] threshold error, low = %d, high = %d\n",
			__FUNCTION__, __LINE__, PQMaskColorMgr.ThlLow, PQMaskColorMgr.ThlHigh);
		return;
	}

	if( pImg == NULL ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d] depth src img not ready\n", __FUNCTION__, __LINE__);
		return;
	}

	memset(&PQMaskColorMgr.ImgByDepthHist, 0x0, PQMASK_DEPDIST_NUM*PQMASK_IMGCH_NUM*sizeof(PQMASK_HIST_T));

	for(py=0; py<AI_DEPTHMAP_HEIGHT; py=py+(PQMaskColorMgr.ImgByDepthHistInfo.vJmp)) {
		for(px=0; px<AI_DEPTHMAP_WIDTH; px=px+(PQMaskColorMgr.ImgByDepthHistInfo.hJmp)) {

			// depth data
			if( pDat[py*AI_DEPTHMAP_WIDTH+px] <= PQMaskColorMgr.ThlLow )
				sectionIdx = PQMASK_DEPDIST_FAR;
			else if( pDat[py*AI_DEPTHMAP_WIDTH+px] > PQMaskColorMgr.ThlLow && pDat[py*AI_DEPTHMAP_WIDTH+px] <= PQMaskColorMgr.ThlHigh )
				sectionIdx = PQMASK_DEPDIST_MID;
			else
				sectionIdx = PQMASK_DEPDIST_NEAR;

			// image data
			yDatIdx = py*AI_DEPTHMAP_WIDTH+px;
			uvDatIdx = (AI_DEPTHMAP_WIDTH*AI_DEPTHMAP_HEIGHT) + (py/2)*AI_DEPTHMAP_WIDTH + (px/2);
			yDat = pImg[yDatIdx];
			uDat = pImg[uvDatIdx+0];
			vDat = pImg[uvDatIdx+1];

			histIdx = yDat >> (8-PQMASK_HISTO_BIN_SFT);
			if( histIdx < PQMASK_HISTO_BIN) {
				PQMaskColorMgr.ImgByDepthHist[sectionIdx][PQMASK_IMGCH_Y].Data.Histogram[histIdx]++;
				PQMaskColorMgr.ImgByDepthHist[sectionIdx][PQMASK_IMGCH_Y].Data.TotalPts++;
			} else {
				rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d] index %d over maximum %d, error\n", __FUNCTION__, __LINE__, histIdx, PQMASK_HISTO_BIN);
			}

			histIdx = uDat >> (8-PQMASK_HISTO_BIN_SFT);
			if( histIdx < PQMASK_HISTO_BIN) {
				PQMaskColorMgr.ImgByDepthHist[sectionIdx][PQMASK_IMGCH_U].Data.Histogram[histIdx]++;
				PQMaskColorMgr.ImgByDepthHist[sectionIdx][PQMASK_IMGCH_U].Data.TotalPts++;
			} else {
				rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d] index %d over maximum %d, error\n", __FUNCTION__, __LINE__, histIdx, PQMASK_HISTO_BIN);
			}

			histIdx = vDat >> (8-PQMASK_HISTO_BIN_SFT);
			if( histIdx < PQMASK_HISTO_BIN) {
				PQMaskColorMgr.ImgByDepthHist[sectionIdx][PQMASK_IMGCH_V].Data.Histogram[histIdx]++;
				PQMaskColorMgr.ImgByDepthHist[sectionIdx][PQMASK_IMGCH_V].Data.TotalPts++;
			} else {
				rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%d] index %d over maximum %d, error\n", __FUNCTION__, __LINE__, histIdx, PQMASK_HISTO_BIN);
			}
		}
	}

#if 0 // time measurement
	t1 = drvif_Get_90k_Lo_clk();
	rtd_printk(KERN_EMERG, TAG_NAME, "90k diff = %d\n", t1-t0);
#endif

#if 0 // debug log

	rtd_printk(KERN_EMERG, TAG_NAME, "Threshold low = %d, high = %d\n", PQMaskColorMgr.ThlLow, PQMaskColorMgr.ThlHigh);

	rtd_printk(KERN_EMERG, TAG_NAME, "FAR YCH\n");
	for(px=0; px<PQMASK_HISTO_BIN; px++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] %d\n", px, PQMaskColorMgr.ImgByDepthHist[PQMASK_DEPDIST_FAR][PQMASK_IMGCH_Y].Data.Histogram[px]);
	}
	rtd_printk(KERN_EMERG, TAG_NAME, "FAR UCH\n");
	for(px=0; px<PQMASK_HISTO_BIN; px++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] %d\n", px, PQMaskColorMgr.ImgByDepthHist[PQMASK_DEPDIST_FAR][PQMASK_IMGCH_U].Data.Histogram[px]);
	}
	rtd_printk(KERN_EMERG, TAG_NAME, "FAR VCH\n");
	for(px=0; px<PQMASK_HISTO_BIN; px++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] %d\n", px, PQMaskColorMgr.ImgByDepthHist[PQMASK_DEPDIST_FAR][PQMASK_IMGCH_V].Data.Histogram[px]);
	}

	rtd_printk(KERN_EMERG, TAG_NAME, "MID YCH\n");
	for(px=0; px<PQMASK_HISTO_BIN; px++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] %d\n", px, PQMaskColorMgr.ImgByDepthHist[PQMASK_DEPDIST_MID][PQMASK_IMGCH_Y].Data.Histogram[px]);
	}
	rtd_printk(KERN_EMERG, TAG_NAME, "MID UCH\n");
	for(px=0; px<PQMASK_HISTO_BIN; px++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] %d\n", px, PQMaskColorMgr.ImgByDepthHist[PQMASK_DEPDIST_MID][PQMASK_IMGCH_U].Data.Histogram[px]);
	}
	rtd_printk(KERN_EMERG, TAG_NAME, "MID VCH\n");
	for(px=0; px<PQMASK_HISTO_BIN; px++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] %d\n", px, PQMaskColorMgr.ImgByDepthHist[PQMASK_DEPDIST_MID][PQMASK_IMGCH_V].Data.Histogram[px]);
	}

	rtd_printk(KERN_EMERG, TAG_NAME, "NEAR YCH\n");
	for(px=0; px<PQMASK_HISTO_BIN; px++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] %d\n", px, PQMaskColorMgr.ImgByDepthHist[PQMASK_DEPDIST_NEAR][PQMASK_IMGCH_Y].Data.Histogram[px]);
	}
	rtd_printk(KERN_EMERG, TAG_NAME, "NEAR UCH\n");
	for(px=0; px<PQMASK_HISTO_BIN; px++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] %d\n", px, PQMaskColorMgr.ImgByDepthHist[PQMASK_DEPDIST_NEAR][PQMASK_IMGCH_U].Data.Histogram[px]);
	}
	rtd_printk(KERN_EMERG, TAG_NAME, "NEAR VCH\n");
	for(px=0; px<PQMASK_HISTO_BIN; px++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] %d\n", px, PQMaskColorMgr.ImgByDepthHist[PQMASK_DEPDIST_NEAR][PQMASK_IMGCH_V].Data.Histogram[px]);
	}
#endif

}

/**
 * @brief
 * origanize the image by semantic label histogram information
 * statistic action is done by ap
 */
void scalerPQMaskColor_Img2SemanticHistogram(void) {

	int labels = 0, channel = 0, i=0;

#if 0 // time measurement
	unsigned int t0=0, t1=0;
	t0 = drvif_Get_90k_Lo_clk();
#endif

	if( PQMaskColorMgr.ImgBySemanticHist[0][0].Info.enable == 0 )
		return;

	for(labels=0; labels<PQMASK_LABEL_NUM; labels++) {
		for(channel=0; channel<PQMASK_IMGCH_NUM; channel++){
			PQMaskColorMgr.ImgBySemanticHist[labels][channel].Data.TotalPts = 0;
			for(i=0; i<PQMASK_HISTO_BIN; i++) {
				PQMaskColorMgr.ImgBySemanticHist[labels][channel].Data.TotalPts += PQMaskColorMgr.ImgBySemanticHist[labels][channel].Data.Histogram[i];
			}
		}
	}

#if 0 // time measurement
	t1 = drvif_Get_90k_Lo_clk();
	rtd_printk(KERN_EMERG, TAG_NAME, "90k diff = %d\n", t1-t0);
#endif

}

/**
 * @brief
 * construct the semantic palette for se action
 * @param group
 * @param pPalette
 */
void scalerPQMaskColor_Label2Clut(unsigned int group, KGAL_PALETTE_INFO_T *pPalette) {

	pPalette->length = 256;
	pPalette->startpale = 0;
	if( group >= MODULE_GRP_MAX )
		group = 0;
	memcpy(pPalette->palette, PQMaskPalette[PQMASK_COLOR_PALETTE_SEMANTIC][group], sizeof(unsigned int)*PQMASK_LABEL_NUM);

#if 0
	rtd_printk(KERN_EMERG, TAG_NAME, "[%s] clut trans module [%-*s] [%-*s] [%-*s]\n",
		__FUNCTION__,
		PQMASK_STRING_LEN, PQMaskModuleName[idx1],
		PQMASK_STRING_LEN, PQMaskModuleName[idx2],
		PQMASK_STRING_LEN, PQMaskModuleName[idx3]
	);
#endif
#if 0
	for(i=0; i<(256/4); i++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] 0x%08x [%3d] 0x%08x [%3d] 0x%08x [%3d] 0x%08x\n",
			(i*4)+0, pPalette->palette[(i*4)+0],
			(i*4)+1, pPalette->palette[(i*4)+1],
			(i*4)+2, pPalette->palette[(i*4)+2],
			(i*4)+3, pPalette->palette[(i*4)+3]
		);
	}
#endif

}

/**
 * @brief
 * construct the depth palette for se action
 * @param group
 * @param pPalette
 */
void scalerPQMaskColor_Depth2Clut(unsigned int group, KGAL_PALETTE_INFO_T *pPalette) {

	pPalette->length = 256;
	pPalette->startpale = 0;
	if( group >= MODULE_GRP_MAX )
		group = 0;
	memcpy(pPalette->palette, PQMaskPalette[PQMASK_COLOR_PALETTE_DEPTH][group], sizeof(unsigned int)*PQMASK_MAPCURVE_LEN);

#if 0
	rtd_printk(KERN_EMERG, TAG_NAME, "[%s] clut trans module [%-*s] [%-*s] [%-*s]\n",
		__FUNCTION__,
		PQMASK_STRING_LEN, PQMaskModuleName[idx1],
		PQMASK_STRING_LEN, PQMaskModuleName[idx2],
		PQMASK_STRING_LEN, PQMaskModuleName[idx3]
	);
#endif
#if 0
	for(i=0; i<(256/4); i++) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%3d] 0x%08x [%3d] 0x%08x [%3d] 0x%08x [%3d] 0x%08x\n",
			(i*4)+0, pPalette->palette[(i*4)+0],
			(i*4)+1, pPalette->palette[(i*4)+1],
			(i*4)+2, pPalette->palette[(i*4)+2],
			(i*4)+3, pPalette->palette[(i*4)+3]
		);
	}
#endif

}

/**
 * @brief
 * Make the scene change flag
 */
void scalerPQMaskColor_SceneChange(void) {

	_clues *SmartPic_clue = NULL;
	_RPC_clues *RPC_SmartPic_clue = NULL;

	SmartPic_clue = fwif_color_Get_SmartPic_clue();
	RPC_SmartPic_clue = (_RPC_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);

	if( PQMaskColorMgr.Param.bSceneChangeEnable != 0 ) {
		PQMaskColorMgr.Status.bSceneChange = PQMaskColorMgr.Status.bSceneChange | SmartPic_clue->SceneChange_by_diff_mean_Dev | RPC_SmartPic_clue->SceneChange;
#if 0
		if(PQMaskColorMgr.Status.bSceneChange != 0) {
			rtd_printk(KERN_EMERG, TAG_NAME, "[%s] scene change happen\n", __FUNCTION__);
		}
#endif
	} else {
		PQMaskColorMgr.Status.bSceneChange = 0;
	}
}

/**
 * @brief
 * Get the genre information from ai model
 */
void scalerPQMaskColor_GetGenre(void) {

	int CurrentGenre = drvif_color_get_Genre_info();

	if( PQMaskColorMgr.Param.bGenre_CG_Enable != 0 ) {
		if( CurrentGenre == AI_GENRE_PATTERN ) {
			if( PQMaskColorMgr.Status.AiGenre != AI_GENRE_PATTERN ) {
				PQMaskColorMgr.Status.bGenerCGStatusChange = 1;
				PQMaskColorMgr.Status.bReloadDepthPalette_AiGenre_CG = 1;
				rtd_printk(KERN_EMERG, TAG_NAME, "[%s] enter cg pattern\n", __FUNCTION__);
			}
		} else {
			if( PQMaskColorMgr.Status.AiGenre == AI_GENRE_PATTERN ) {
				PQMaskColorMgr.Status.bGenerCGStatusChange = 1;
				scalerPQMaskColor_SetReloadPalette(PQMASK_COLOR_PALETTE_DEPTH);
				rtd_printk(KERN_EMERG, TAG_NAME, "[%s] leave cg pattern\n", __FUNCTION__);
			}
		}
		PQMaskColorMgr.Status.AiGenre = CurrentGenre;
	} else {
		PQMaskColorMgr.Status.AiGenre = AI_GENRE_TYPE_NUM;
	}
}

/**
 * @brief
 * Force se output current result only
 * @param Frames
 * times
 */
void scalerPQMaskColor_ForceOutputCur(unsigned char Frames) {
	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start, frame = %d\n", __FUNCTION__, Frames);
	PQMaskColorMgr.Status.ForceCurFrm = Frames;
	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);
}

/**
 * @brief
 * prepare the palette for se action
 */
void scalerPQMaskColor_ConstructPalette(void) {

	int i=0, group=0, tmpValue=0, delta=0, sqm_en=0;
	int idx0=0, idx1=0, idx2=0;
	unsigned char ChannelA=0, ChannelR=0, ChannelG=0, ChannelB=0;
	int ActualTmapBld=0;
	int RemapLocalContrastWeight[PQMASK_MAPCURVE_LEN];
	DRV_LC_ToneMapping_Blend TmapBld;
	int DepthWeightTbl_SQM[PQMASK_MODULE_NUM] = {0};
	static int ai_resolution_pre = 0;
	extern int ai_resolution;
	extern RTK_AI_PQ_mode aipq_mode;
	extern short AI_DepthWeightTbl_SQM_TBL[AI_RESOLUTION_TYPE_NUM][PQMASK_MODULE_NUM];

	for(i=0; i<PQMASK_MODULE_NUM; i++) {
		DepthWeightTbl_SQM[i] = AI_DepthWeightTbl_SQM_TBL[ai_resolution][i];
	}
	sqm_en = aipq_mode.sqm_mode?1:0;

	if (ai_resolution != ai_resolution_pre) {
		PQMaskColorMgr.Status.bReloadDepPalette = 1; //update SQM diff
		ai_resolution_pre = ai_resolution;
	}

	// rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	// calibrate the local contrast weight dc to 128
	if( PQMaskColorMgr.Status.bReloadDepPalette != 0 || PQMaskColorMgr.Status.bReloadSemPalette != 0 ) {
		drvif_color_get_LC_ToneMapping_Blend(&TmapBld);
		ActualTmapBld = (TmapBld.lc_tmap_blend_factor > 128) ? 128 : TmapBld.lc_tmap_blend_factor;
		PQMaskColorMgr.Status.LocalContrastRstWeight = ActualTmapBld*2;
		if( PQMaskColorMgr.Status.LocalContrastRstWeight > 255 )
			PQMaskColorMgr.Status.LocalContrastRstWeight = 255;
	}

	// depth palette
	if( PQMaskColorMgr.Status.AiGenre == AI_GENRE_PATTERN ) {

		if( PQMaskColorMgr.Status.bReloadDepthPalette_AiGenre_CG != 0 ) {

			rtd_printk(KERN_INFO, TAG_NAME, "[%s] update depth palette for CG start\n", __FUNCTION__);

			// fill in the BYPASS mapping curve
			for(group=0; group<MODULE_GRP_MAX; group++) {

				idx0 = (SEProcGrpMapping[group][0] >= PQMASK_MODULE_NUM)? PQMASK_MODULE_NR : SEProcGrpMapping[group][0];
				idx1 = (SEProcGrpMapping[group][1] >= PQMASK_MODULE_NUM)? PQMASK_MODULE_NR : SEProcGrpMapping[group][1];
				idx2 = (SEProcGrpMapping[group][2] >= PQMASK_MODULE_NUM)? PQMASK_MODULE_NR : SEProcGrpMapping[group][2];

				for(i=0; i<PQMASK_MAPCURVE_LEN; i++) {
					ChannelA = 0;
					ChannelR = (idx0==PQMASK_MODULE_LC)? PQMaskColorMgr.Status.LocalContrastRstWeight : 128;
					ChannelG = (idx1==PQMASK_MODULE_LC)? PQMaskColorMgr.Status.LocalContrastRstWeight : 128;
					ChannelB = (idx2==PQMASK_MODULE_LC)? PQMaskColorMgr.Status.LocalContrastRstWeight : 128;

					PQMaskPalette[PQMASK_COLOR_PALETTE_DEPTH][group][i] = (ChannelA<<24) + (ChannelR<<16) + (ChannelG<<8) + ChannelB;
				}
			}

			PQMaskColorMgr.Status.bReloadDepthPalette_AiGenre_CG = 0;

			rtd_printk(KERN_INFO, TAG_NAME, "[%s] update depth palette for CG done\n", __FUNCTION__);
		}

	} else {

		if( PQMaskColorMgr.Status.bReloadDepPalette != 0 ) {

			rtd_printk(KERN_INFO, TAG_NAME, "[%s] update depth palette\n", __FUNCTION__);

			// local contrast mapping curve
			memset(RemapLocalContrastWeight, 0x0, sizeof(int)*PQMASK_MAPCURVE_LEN);
			delta = aipq_mode.sqm_mode?DepthWeightTbl_SQM[PQMASK_MODULE_LC]:0;
			for( i=0; i<PQMASK_MAPCURVE_LEN; i++ ) {
				tmpValue = PQMaskColorMgr.DepthWeightTbl[PQMASK_MODULE_LC][i]+delta;
				tmpValue = CLAMP(tmpValue, 0, 255);
				RemapLocalContrastWeight[i] = (tmpValue-128) + PQMaskColorMgr.Status.LocalContrastRstWeight;
				if( RemapLocalContrastWeight[i] > 255 )
					RemapLocalContrastWeight[i] = 255;
				if( RemapLocalContrastWeight[i] < 0 )
					RemapLocalContrastWeight[i] = 0;
			}

			// fill in the mapping curve
			for(group=0; group<MODULE_GRP_MAX; group++) {

				idx0 = (SEProcGrpMapping[group][0] >= PQMASK_MODULE_NUM)? PQMASK_MODULE_NR : SEProcGrpMapping[group][0];
				idx1 = (SEProcGrpMapping[group][1] >= PQMASK_MODULE_NUM)? PQMASK_MODULE_NR : SEProcGrpMapping[group][1];
				idx2 = (SEProcGrpMapping[group][2] >= PQMASK_MODULE_NUM)? PQMASK_MODULE_NR : SEProcGrpMapping[group][2];

				for(i=0; i<PQMASK_MAPCURVE_LEN; i++) {

					ChannelA = 0;
					tmpValue = PQMaskColorMgr.DepthWeightTbl[idx0][i]+(DepthWeightTbl_SQM[idx0]*sqm_en);
					ChannelR = (idx0==PQMASK_MODULE_LC)? RemapLocalContrastWeight[i] : CLAMP(tmpValue, 0, 255);
					tmpValue = PQMaskColorMgr.DepthWeightTbl[idx1][i]+(DepthWeightTbl_SQM[idx1]*sqm_en);
					ChannelG = (idx1==PQMASK_MODULE_LC)? RemapLocalContrastWeight[i] : CLAMP(tmpValue, 0, 255);
					tmpValue = PQMaskColorMgr.DepthWeightTbl[idx2][i]+(DepthWeightTbl_SQM[idx2]*sqm_en);
					ChannelB = (idx2==PQMASK_MODULE_LC)? RemapLocalContrastWeight[i] : CLAMP(tmpValue, 0, 255);

					PQMaskPalette[PQMASK_COLOR_PALETTE_DEPTH][group][i] = (ChannelA<<24) + (ChannelR<<16) + (ChannelG<<8) + ChannelB;

				}

			}

			PQMaskColorMgr.Status.bReloadDepPalette = 0;

	#if 0
			rtd_printk(KERN_EMERG, TAG_NAME, "Depth palette\n");
			for( group=0; group<MODULE_GRP_MAX; group++ ) {
				for( i=0; i<PQMASK_MAPCURVE_LEN; i++ ) {
					rtd_printk(KERN_EMERG, TAG_NAME, "[%1d][%3d] 0x%08x\n", group, i, PQMaskPalette[PQMASK_COLOR_PALETTE_DEPTH][group][i]);
				}
			}
	#endif

		}
	}

	// semantic palette
	if( PQMaskColorMgr.Status.AiGenre == AI_GENRE_PATTERN ) {

		if( PQMaskColorMgr.Status.bReloadSemanticPalette_AiGenre_CG != 0 ) {

			rtd_printk(KERN_INFO, TAG_NAME, "[%s] update semantic palette for CG start\n", __FUNCTION__);

			// fill in the BYPASS mapping curve
			for(group=0; group<MODULE_GRP_MAX; group++) {

				idx0 = (SEProcGrpMapping[group][0] >= PQMASK_MODULE_NUM)? PQMASK_MODULE_NR : SEProcGrpMapping[group][0];
				idx1 = (SEProcGrpMapping[group][1] >= PQMASK_MODULE_NUM)? PQMASK_MODULE_NR : SEProcGrpMapping[group][1];
				idx2 = (SEProcGrpMapping[group][2] >= PQMASK_MODULE_NUM)? PQMASK_MODULE_NR : SEProcGrpMapping[group][2];

				for(i=0; i<PQMASK_LABEL_NUM; i++) {

					ChannelA = 0;
					ChannelR = (idx0==PQMASK_MODULE_LC)? PQMaskColorMgr.Status.LocalContrastRstWeight : 128;
					ChannelG = (idx1==PQMASK_MODULE_LC)? PQMaskColorMgr.Status.LocalContrastRstWeight : 128;
					ChannelB = (idx2==PQMASK_MODULE_LC)? PQMaskColorMgr.Status.LocalContrastRstWeight : 128;

					PQMaskPalette[PQMASK_COLOR_PALETTE_SEMANTIC][group][i] = (ChannelA<<24) + (ChannelR<<16) + (ChannelG<<8) + ChannelB;

				}

			}

			PQMaskColorMgr.Status.bReloadSemanticPalette_AiGenre_CG = 0;

			rtd_printk(KERN_INFO, TAG_NAME, "[%s] update semantic palette for CG done\n", __FUNCTION__);

		}

	} else {

		if( PQMaskColorMgr.Status.bReloadSemPalette != 0 ) {

			rtd_printk(KERN_INFO, TAG_NAME, "[%s] update semantic palette\n", __FUNCTION__);

			// local contrast mapping curve
			memset(RemapLocalContrastWeight, 0x0, sizeof(int)*PQMASK_MAPCURVE_LEN);
			for( i=0; i<PQMASK_LABEL_NUM; i++ ) {
				RemapLocalContrastWeight[i] = (PQMaskColorMgr.SemanticWeightTbl[PQMASK_MODULE_LC][i]-128) + PQMaskColorMgr.Status.LocalContrastRstWeight;
				if( RemapLocalContrastWeight[i] > 255 )
					RemapLocalContrastWeight[i] = 255;
				if( RemapLocalContrastWeight[i] < 0 )
					RemapLocalContrastWeight[i] = 0;
			}

			// fill in the mapping curve
			for(group=0; group<MODULE_GRP_MAX; group++) {

				idx0 = (SEProcGrpMapping[group][0] >= PQMASK_MODULE_NUM)? PQMASK_MODULE_NR : SEProcGrpMapping[group][0];
				idx1 = (SEProcGrpMapping[group][1] >= PQMASK_MODULE_NUM)? PQMASK_MODULE_NR : SEProcGrpMapping[group][1];
				idx2 = (SEProcGrpMapping[group][2] >= PQMASK_MODULE_NUM)? PQMASK_MODULE_NR : SEProcGrpMapping[group][2];

				for(i=0; i<PQMASK_LABEL_NUM; i++) {

					ChannelA = 0;
					ChannelR = (idx0==PQMASK_MODULE_LC)? RemapLocalContrastWeight[i] : PQMaskColorMgr.SemanticWeightTbl[idx0][i];
					ChannelG = (idx1==PQMASK_MODULE_LC)? RemapLocalContrastWeight[i] : PQMaskColorMgr.SemanticWeightTbl[idx1][i];
					ChannelB = (idx2==PQMASK_MODULE_LC)? RemapLocalContrastWeight[i] : PQMaskColorMgr.SemanticWeightTbl[idx2][i];

					PQMaskPalette[PQMASK_COLOR_PALETTE_SEMANTIC][group][i] = (ChannelA<<24) + (ChannelR<<16) + (ChannelG<<8) + ChannelB;

				}

			}

			PQMaskColorMgr.Status.bReloadSemPalette = 0;

	#if 0
			rtd_printk(KERN_EMERG, TAG_NAME, "Semantic palette\n");
			for( group=0; group<MODULE_GRP_MAX; group++ ) {
				for( i=0; i<PQMASK_LABEL_NUM; i++ ) {
					rtd_printk(KERN_EMERG, TAG_NAME, "[%1d][%3d] 0x%08x\n", group, i, PQMaskPalette[PQMASK_COLOR_PALETTE_SEMANTIC][group][i]);
				}
			}
	#endif

		}
	}
	// rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);
}

/**
 * @brief
 * trigger reload the palette for se
 * @param PaletteSel
 * select which palette to reload
 */
void scalerPQMaskColor_SetReloadPalette(unsigned char PaletteSel) {

	//rtd_printk(KERN_INFO, TAG_NAME, "[%s] start, set palette %d\n", __FUNCTION__, PaletteSel);

	switch(PaletteSel) {
		case PQMASK_COLOR_PALETTE_DEPTH:
			PQMaskColorMgr.Status.bReloadDepPalette = 1;
			break;
		case PQMASK_COLOR_PALETTE_SEMANTIC:
			PQMaskColorMgr.Status.bReloadSemPalette = 1;
			break;
		default:
			rtd_printk(KERN_EMERG, TAG_NAME, "index error\n");
			break;
	}

	//rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);

}

/**
 * @brief
 * adjust the label peaking related reigster to align sharpness pqmask weighting 128 = bypass
 */
void scalerPQMaskColor_UpdateLabelPK(void) {

	static DRV_Sharpness_Table ShpCurSetting;
	DRV_LABEL_PK ShpLabelPkSetting;
	int i=0;
	int dividen=256;
	int sobelRatio=1;
	int zdiffRatio=1;
	int textRatio=1;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	drvif_color_Get_Sharpness_Table(&ShpCurSetting);

	memcpy(&ShpLabelPkSetting, &PQMaskColorMgr.ModuleSetting.Sharpness, sizeof(DRV_LABEL_PK));

	// Sobel
	if( PQMaskColorMgr.ModuleSetting.Sharpness.SobelUpbnd[4] == 0 ) {
		sobelRatio = dividen;
	} else {
		sobelRatio = (ShpCurSetting.TwoD_Peak_AdaptCtrl.Sobel_Upbnd*dividen)/PQMaskColorMgr.ModuleSetting.Sharpness.SobelUpbnd[4];
	}
	// Zdiff
	if( PQMaskColorMgr.ModuleSetting.Sharpness.ZDiffUpbnd[4] == 0 ) {
		zdiffRatio = dividen;
	} else {
		zdiffRatio = (ShpCurSetting.TwoD_Peak_AdaptCtrl.Zdiff_Upbnd*dividen)/PQMaskColorMgr.ModuleSetting.Sharpness.ZDiffUpbnd[4];
	}
	// Texture
	if( PQMaskColorMgr.ModuleSetting.Sharpness.TextUpbnd[4] == 0 ) {
		textRatio = dividen;
	} else {
		textRatio = (ShpCurSetting.TwoD_Peak_AdaptCtrl.Tex_Upbnd*dividen)/PQMaskColorMgr.ModuleSetting.Sharpness.TextUpbnd[4];
	}

	// rtd_printk(KERN_EMERG, "DEBUG", "[%s] ratio sobel = %d, zdiff = %d, text = %d\n", __FUNCTION__, sobelRatio, zdiffRatio, textRatio);

	for(i=0; i<9; i++) {
		ShpLabelPkSetting.SobelUpbnd[i] = (PQMaskColorMgr.ModuleSetting.Sharpness.SobelUpbnd[i]*sobelRatio + (dividen >> 1))/dividen;
		if( ShpLabelPkSetting.SobelUpbnd[i] > 255 )
			ShpLabelPkSetting.SobelUpbnd[i] = 255;
		ShpLabelPkSetting.ZDiffUpbnd[i] = (PQMaskColorMgr.ModuleSetting.Sharpness.ZDiffUpbnd[i]*zdiffRatio + (dividen >> 1))/dividen;
		if( ShpLabelPkSetting.ZDiffUpbnd[i] > 255 )
			ShpLabelPkSetting.ZDiffUpbnd[i] = 255;
		ShpLabelPkSetting.TextUpbnd[i] = (PQMaskColorMgr.ModuleSetting.Sharpness.TextUpbnd[i]*textRatio + (dividen >> 1))/dividen;
		if( ShpLabelPkSetting.TextUpbnd[i] > 255 )
			ShpLabelPkSetting.TextUpbnd[i] = 255;

		// rtd_printk(KERN_EMERG, "DEBUG", "[%s][%d] sobel = %3u, zdiff = %3u, text = %3u\n",
		// 	__FUNCTION__, i,
		// 	ShpLabelPkSetting.SobelUpbnd[i],
		// 	ShpLabelPkSetting.ZDiffUpbnd[i],
		// 	ShpLabelPkSetting.TextUpbnd[i]
		// );
	}

	drvif_set_labelpk(&ShpLabelPkSetting);

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] end\n", __FUNCTION__);
}

// RTICE commands
void scalerPQMaskColor_GetSemanticWeight(unsigned char *Buf) {
	int i=0, j=0;
	for(i=0; i<PQMASK_MODULE_NUM; i++) {
		for(j=0; j<PQMASK_LABEL_NUM; j++) {
			Buf[i*PQMASK_LABEL_NUM + j] = PQMaskColorMgr.SemanticWeightTbl[i][j];
		}
	}
}

void scalerPQMaskColor_SetSemanticWeight(unsigned char *Buf) {
	int i=0, j=0;
	for(i=0; i<PQMASK_MODULE_NUM; i++) {
		for(j=0; j<PQMASK_LABEL_NUM; j++) {
			PQMaskColorMgr.SemanticWeightTbl[i][j] = Buf[i*PQMASK_LABEL_NUM + j];
		}
	}
	scalerPQMaskColor_SetReloadPalette(PQMASK_COLOR_PALETTE_SEMANTIC);
}

void scalerPQMaskColor_GetDepthWeight(unsigned char *Buf) {
	int i=0;
	for(i=0; i<PQMASK_MODULE_NUM; i++) {
		memcpy((void *)(Buf+i*PQMASK_MAPCURVE_LEN), (void *)PQMaskColorMgr.DepthWeightTbl[i], sizeof(unsigned char)*PQMASK_MAPCURVE_LEN);
	}
}

void scalerPQMaskColor_SetDepthWeight(unsigned char *Buf) {
	int i=0;
	for(i=0; i<PQMASK_MODULE_NUM; i++) {
		memcpy((void *)PQMaskColorMgr.DepthWeightTbl[i], (void *)(Buf+i*PQMASK_MAPCURVE_LEN), sizeof(unsigned char)*PQMASK_MAPCURVE_LEN);
	}
	scalerPQMaskColor_SetReloadPalette(PQMASK_COLOR_PALETTE_DEPTH);
}

// V4L2 commands
/**
 * @brief
 * support V4L2 command
 * @param ModuleSel
 * module to set the weight
 * @param Buf
 * input depth weight, assume data type is unsigned char
 * @return int
 */
int scalerPQMask_V4L2_SetDepthWeight(unsigned char ModuleSel, unsigned char *Buf) {

	if( ModuleSel >= PQMASK_MODULE_NUM ) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "[%s] ModuleSel = %d error\n", __FUNCTION__, ModuleSel);
		return -1;
	}

	memcpy((void *)PQMaskColorMgr.DepthWeightTbl[ModuleSel], (void *)Buf, sizeof(unsigned char)*PQMASK_MAPCURVE_LEN);
	scalerPQMaskColor_SetReloadPalette(PQMASK_COLOR_PALETTE_DEPTH);

	return 0;

}

//====for Demo Use Start====Remove it after Demo====
void scalerPQMaskColor_SetFunction_En(unsigned char bEn) {

	DRV_LC_ToneMapping_PQmask LCTmap;
	DRV_LC_Sharpness_PQmask LCSharp;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	// initial some structure

	// NR
	// (drvif_color_DRV_SNR_PQ_Mask)(drvif_color_Set_Block_DeContour_Ctrl), setting vip_default_panel.c
	PQMaskColorMgr.ModuleSetting.Nr.bNrEnable =bEn;
	PQMaskColorMgr.ModuleSetting.Nr.bIDecontEnable =bEn;
	PQMaskColorMgr.ModuleSetting.Nr.bIBlockDecontEnable=bEn;
	drvif_color_DRV_SNR_PQ_Mask_Enable(PQMaskColorMgr.ModuleSetting.Nr.bNrEnable);
	drvif_color_Set_I_De_Contour_PQ_Mask_Enable(PQMaskColorMgr.ModuleSetting.Nr.bIDecontEnable);
	drvif_color_Set_Block_DeContour_PQ_Mask_Enable(PQMaskColorMgr.ModuleSetting.Nr.bIBlockDecontEnable);

	// ICM
	PQMaskColorMgr.ModuleSetting.ICM.bHueEnable =bEn;
	PQMaskColorMgr.ModuleSetting.ICM.bSatEnable =bEn;
	PQMaskColorMgr.ModuleSetting.ICM.bIntEnable =bEn;
	drvif_color_icm_Set_pqmask_hue(PQMaskColorMgr.ModuleSetting.ICM.bHueEnable, PQMaskColorMgr.ModuleSetting.ICM.HueGain);
	drvif_color_icm_Set_pqmask_sat(PQMaskColorMgr.ModuleSetting.ICM.bSatEnable, PQMaskColorMgr.ModuleSetting.ICM.SatGain, PQMaskColorMgr.ModuleSetting.ICM.SatOffset);
	drvif_color_icm_Set_pqmask_int(PQMaskColorMgr.ModuleSetting.ICM.bIntEnable, PQMaskColorMgr.ModuleSetting.ICM.IntGain, PQMaskColorMgr.ModuleSetting.ICM.IntOffset);

	// Sharpness
	drvif_sharpness_pq_mask_en(bEn);

	//LC
	PQMaskColorMgr.ModuleSetting.LocalContrast.bToneMappingEnable =bEn;
	PQMaskColorMgr.ModuleSetting.LocalContrast.bSharpEnable =bEn;
	LCTmap.lc_PQmask_tmap_En = PQMaskColorMgr.ModuleSetting.LocalContrast.bToneMappingEnable;
	LCTmap.lc_PQmask_offset = PQMaskColorMgr.ModuleSetting.LocalContrast.ToneMappingOffset;
	LCSharp.lc_PQmask_shp_En = PQMaskColorMgr.ModuleSetting.LocalContrast.bSharpEnable;
	LCSharp.lc_PQmask_shp_gain = PQMaskColorMgr.ModuleSetting.LocalContrast.SharpGain;
	LCSharp.lc_PQmask_shp_debug_shift = 0;
	drvif_color_set_LC_Tonemapping_PQmask(&LCTmap);
	drvif_color_set_LC_Sharpness_PQmask(&LCSharp);

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] end\n", __FUNCTION__);

}

#include <rbus/color_reg.h>
#include <tvscalercontrol/io/ioregdrv.h>
void scalerPQMaskColor_Function_En_CheckDummyRegister(void)
{
	static unsigned char pre_pq_mask_en = 0;
	unsigned char pq_mask_en = 0;
	pq_mask_en = COLOR_D_VC_Global_CTRL_get_dummy_30_27(IoReg_Read32(COLOR_D_VC_Global_CTRL_reg))>>3;
	if (pre_pq_mask_en != pq_mask_en) {
		scalerPQMaskColor_SetFunction_En(pq_mask_en);
		pre_pq_mask_en = pq_mask_en;
	}
}
//====for Demo Use End====Remove it after Demo====
