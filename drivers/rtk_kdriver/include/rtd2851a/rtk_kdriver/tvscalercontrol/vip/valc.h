/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2009
  * All rights reserved.
  * ========================================================================*/

/**
 * @file
 * 	This file is for gibi and od related functions.
 *
 * @author 	$Author$
 * @date 	$Date$
 * @version $Revision$
 */

/**
 * @addtogroup color
 * @{
 */

#ifndef _VALC_H
#define _VALC_H
/*============================ Module dependency  ===========================*/

/*================================ Definitions ==============================*/
#define VALC_TBL_LEN 17

/*================================== Variables ==============================*/
extern unsigned int valcTbl[3][2][VALC_TBL_LEN];

/*===================================  Types ================================*/
typedef union {
	unsigned int b32Value;
	struct
	{
		unsigned int dummy0:6;
		unsigned int tbl1Value:10;
		unsigned int dummy1:6;
		unsigned int tbl2Value:10;
	};
} DRV_VALC_Tbl2SRAM_Val;

typedef enum {
	_VALC_PAT_H1V1 = 0,
	_VALC_PAT_H2V1,
	_VALC_PAT_H1V2,
	_VALC_PAT_H2V2,
	_VALC_PAT_ERR
} DRV_valc_pattern_t;

typedef enum {
	_VALC_COLOR_R = 0,
	_VALC_COLOR_G,
	_VALC_COLOR_B,
	_VALC_COLOR_ALL,
	_VALC_COLOR_ERR
} DRV_valc_channel_t;

typedef struct {
	unsigned char Weight[16];
	unsigned char ClipSel;
} DRV_valc_hpf_ctrl_t;

typedef struct {
	unsigned char Weight[16];
} DRV_valc_sat_ctrl_t;

typedef struct {
	unsigned char SpatialPatternSel;
	DRV_valc_sat_ctrl_t SATCtrl;
	DRV_valc_hpf_ctrl_t HPFCtrl;
} DRV_valc_ctrl_t;

/*=================================== Functions ================================*/
void drvif_color_set_valc_enable(unsigned char bEnable);
void drvif_color_set_valc_SpatialPattern(unsigned char PatternSel);
void drvif_color_set_valc_HPF_protect(DRV_valc_hpf_ctrl_t* pHPFCtrl);
void drvif_color_set_valc_SAT_protect(DRV_valc_sat_ctrl_t* pSATCtrl);

void drvif_color_set_valc_table(unsigned int* pTbl, DRV_valc_channel_t ChannelSel);
void drvif_color_get_valc_table(unsigned int* pTbl, DRV_valc_channel_t ChannelSel);
unsigned char driver_color_get_valc_enable(void);

#endif

