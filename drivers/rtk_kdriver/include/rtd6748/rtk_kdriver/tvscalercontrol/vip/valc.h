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
#define VALC_TBL_LEN 4096

/*================================== Variables ==============================*/
extern unsigned int valcTbl[3][2][VALC_TBL_LEN];

/*===================================  Types ================================*/
enum {
	VALC_PATTERN_H1V1 = 0,
	VALC_PATTERN_H1V2,
	VALC_PATTERN_H2V1,
	VALC_PATTERN_H2V2,
	VALC_PATTERN_MAX
};

typedef enum {
	_VALC_COLOR_R = 0,
	_VALC_COLOR_G,
	_VALC_COLOR_B,
	_VALC_COLOR_ERR
} DRV_valc_channel_t;

typedef struct {
	unsigned char Cyclic_H;
	unsigned char Cyclic_V;
	unsigned char Pattern[8][16];
} DRV_valc_pattern_ctrl_t;

typedef struct {
	unsigned char Weight[16];
	unsigned char ClipSel;
} DRV_valc_hpf_ctrl_t;

typedef struct {
	unsigned char Weight[16];
} DRV_valc_sat_ctrl_t;

typedef struct {
	DRV_valc_pattern_ctrl_t Pattern;
	DRV_valc_sat_ctrl_t SATCtrl;
	DRV_valc_hpf_ctrl_t HPFCtrl;
} DRV_valc_ctrl_t;

/*=================================== Functions ================================*/
void drvif_color_set_valc_enable(unsigned char bEnable);
void drvif_color_set_valc_SpatialPattern(DRV_valc_pattern_ctrl_t* pPATCtrl);
void drvif_color_set_valc_HPF_protect(DRV_valc_hpf_ctrl_t* pHPFCtrl);
void drvif_color_set_valc_SAT_protect(DRV_valc_sat_ctrl_t* pSATCtrl);

void drvif_color_set_valc_table(unsigned int* pTbl, DRV_valc_channel_t ChannelSel);
void drvif_color_get_valc_table(unsigned int* pTbl, DRV_valc_channel_t ChannelSel);
unsigned char driver_color_get_valc_enable(void);
void drvif_color_get_valc_SpatialPattern(DRV_valc_pattern_ctrl_t* pPATCtrl);
void drvif_color_get_valc_HPF_protect(DRV_valc_hpf_ctrl_t* pHPFCtrl);
void drvif_color_get_valc_SAT_protect(DRV_valc_sat_ctrl_t* pSATCtrl);
void drvif_color_set_valc_location(char set);

#endif

