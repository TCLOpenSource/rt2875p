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

#ifndef _GIBI_OD_H
#define _GIBI_OD_H
/*============================ Module dependency  ===========================*/
/*#include "rtd_types.h"*/

#include <scaler/vipCommon.h>

/*===================================  Types ================================*/

/*================================== Variables ==============================*/

/*================================ Definitions ==============================*/
#define GIBI_FORMAT_444          0
#define GIBI_FORMAT_422          1
#define GIBI_FORMAT_420          2

#define GIBI_BIT_COUNT          8  /* 8 bits*/
#define OD_table_length	289

#ifndef CONFIG_VPQ_TABLE_GENERATE
#define CONFIG_VPQ_TABLE_GENERATE 0
#endif

#if CONFIG_VPQ_TABLE_GENERATE
typedef char bool;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
#endif

#define OD_TABLE_MODE_INVERSE	-1
#define OD_TABLE_MODE_DELTA		1
#define OD_TABLE_MODE_TARGET	2
#if 0//ndef CONFIG_PANEL_TCON_BIN_COMPILE
typedef struct {
	unsigned int table_deltamode[OD_table_length];
	unsigned int table_targetmode[OD_table_length];
	unsigned int table_inversemode[OD_table_length];
} DRV_od_table_t;

typedef struct {
	unsigned char type;
	unsigned char gain;
} DRV_od_table_mode_t;
#endif //CONFIG_PANEL_TCON_BIN_COMPILE
/*================================ Functions ==============================*/
void drvif_color_od_for_demo(void);
void drvif_color_od(unsigned char bOD);
bool drvif_color_od_pqc(UINT16 height, UINT16 width, UINT8 bit_sel, UINT8 dma_mode, UINT8 FrameLimitBit, int dataColor, int dataFormat);
void drvif_color_od_Ymode(void);
void drvif_color_od_table_seperate(unsigned int *pODtable, unsigned char tableType);
void drvif_color_od_table_seperate_write(unsigned int *pODtable, unsigned char tableType, char channel);
void drvif_color_od_table_read(unsigned char *pODtable);
void drvif_color_od_table_seperate_read(unsigned char *pODtable, unsigned char tableType);
void drvif_color_od_table(unsigned int *pODtable, unsigned char targetmode);
void drvif_color_safe_od_enable(unsigned char bEnable, unsigned char bOD_OnOff_Switch);
void drvif_color_od_gain(unsigned char ucGain);
unsigned char drvif_color_get_od_gain(unsigned char channel);

void drvif_color_set_od_bits(unsigned char FrameLimitBit, unsigned char input_bit_sel);
unsigned char drvif_color_get_od_bits(void);
unsigned char drvif_color_get_od_en(void);
unsigned char drvif_color_get_od_dma_mode(void);
unsigned char drvif_color_set_od_dma_enable(unsigned char enable);
unsigned char drvif_color_get_od_dma_enable(void);
unsigned char drvif_color_od_calc_max_bit(unsigned short width, unsigned short height, unsigned char dma_mode, unsigned int mem_size/*byte*/);

#endif

