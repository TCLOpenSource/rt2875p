/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2021
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for APLSatGain related functions.
 *
 * @author 		Bill Chou
 * @date 		2023/02/03
 * @version 	1
 */

#ifndef _APLSATGAIN_H
#define _APLSATGAIN_H

#define APLSATGAIN_Table_NUM 1

typedef struct {
	unsigned short APL_div_factor;
	unsigned short APL_factor;
} DRV_APLSatGain_APL_factor;

typedef struct {
	unsigned char shift_bit;
	unsigned char sat_mode;
	unsigned char APL_blend_weight;
	unsigned char blenking_grey_en;
}DRV_APLSatGain_Ctrl_Common;

typedef struct {
	unsigned short gain_by_apl_curve_tbl0[17];
	unsigned short gain_by_apl_curve_tbl1[17];
} DRV_APLSatGain_gain_by_apl;

typedef struct {
	unsigned short gain_by_sat_curve_tbl0[17];
	unsigned short gain_by_sat_curve_tbl1[17];
} DRV_APLSatGain_gain_by_sat;

typedef struct {
	unsigned char enable;
	DRV_APLSatGain_Ctrl_Common ctrl;
	DRV_APLSatGain_gain_by_apl gain_by_apl;
	DRV_APLSatGain_gain_by_sat gain_by_sat;
} DRV_APLSatgain_Table;

void drvif_color_set_APLSatGain_Enable(unsigned char enable);
void drvif_color_set_APLSatGain_APL_factor(DRV_APLSatGain_APL_factor *factor);
void drvif_color_set_APLSatGain_Ctrl_Common(DRV_APLSatGain_Ctrl_Common *ptr);
void drvif_color_set_APLSatGain_Gain_by_APL(DRV_APLSatGain_gain_by_apl *curve, unsigned char table_sel);
void drvif_color_set_APLSatGain_Gain_by_sat(DRV_APLSatGain_gain_by_sat *curve, unsigned char table_sel);
unsigned short drvif_color_get_APLSatGain_RGBAPL(void);
unsigned short drvif_color_get_APLSatGain_YAPL(void);
unsigned short drvif_color_get_APLSatGain_BlendAPL(void);
unsigned short drvif_color_get_APLSatGain_FrameSat(void);
unsigned short drvif_color_get_APLSatGain_BlankingGrey(void);

#endif
