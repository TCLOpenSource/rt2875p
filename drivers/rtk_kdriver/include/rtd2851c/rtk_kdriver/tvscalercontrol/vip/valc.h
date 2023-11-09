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

//#ifndef _VALC_H
#define _VALC_H
/*============================ Module dependency  ===========================*/
#include <tvscalercontrol/vip/pcid.h>/*sync TV030*/

/*================================ Definitions ==============================*/
#define VALC_TBL_LEN 129
#define PCID_VALC_TBL_LEN 257
/*================================== Variables ==============================*/
/*===================================  Types ================================*/
/*typedef union {
	unsigned int b32Value;
	struct
	{
		unsigned int dummy0:6;
		unsigned int tbl1Value:10;
		unsigned int dummy1:6;
		unsigned int tbl2Value:10;
	};
} DRV_VALC_Tbl2SRAM_Val;*/

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

typedef union
{
	signed int b32Value;
	struct
	{
		short b32_16:16;
		short b15_00:16;
	};
} DRV_VALC_Tbl2SRAM_Val;

//#else

/*typedef union
{
	signed int b32Value;
	struct
	{
		short b15_00:16;
		short b32_16:16;
	};
} DRV_VALC_Tbl2SRAM_Val;*/

//#endif

typedef struct {
	unsigned char line_sel;
	unsigned char point_sel;
	unsigned char color_sel;
} DRV_pcid2_pixel_ref_setting_t_val;

typedef struct
{
	DRV_pcid2_pixel_ref_setting_t_val even_r;
	DRV_pcid2_pixel_ref_setting_t_val even_g;
	DRV_pcid2_pixel_ref_setting_t_val even_b;

	DRV_pcid2_pixel_ref_setting_t_val odd_r;
	DRV_pcid2_pixel_ref_setting_t_val odd_g;
	DRV_pcid2_pixel_ref_setting_t_val odd_b;
} DRV_pcid2_line_ref_setting_t_val;

typedef struct {
	DRV_pcid2_line_ref_setting_t_val line1;
	DRV_pcid2_line_ref_setting_t_val line2;
	DRV_pcid2_line_ref_setting_t_val line3;
	DRV_pcid2_line_ref_setting_t_val line4;
} DRV_pcid2_ref_setting_t_val;

typedef struct {
	// Mode selection
	unsigned char boundary_mode;
	// intp_enable
	unsigned char intp_bit_sel;
	unsigned char first_line_tab;
	// Subpixel reference setting
	DRV_pcid2_ref_setting_t_val pixel_ref;
} DRV_pcid2_data_t_val;

/*=================================== Functions ================================*/
void drvif_color_set_valc_enable(unsigned char bEnable);
void drvif_color_set_valc_SpatialPattern(unsigned char PatternSel);
void drvif_color_set_valc_HPF_protect(DRV_valc_hpf_ctrl_t* pHPFCtrl);
void drvif_color_set_valc_SAT_protect(DRV_valc_sat_ctrl_t* pSATCtrl);

void drvif_color_set_pcid_valc_poltable(unsigned int * pPcid2PolTable, DRV_pcid2_channel_t color);
void drvif_color_set_pcid_valc_pixel_setting_verify(DRV_pcid2_data_t_val* pPcid2Setting);
void drvif_color_set_pcid_valc_control_setting(DRV_pcid2_data_t_val* pPcid2Setting);
void drvif_color_set_valc_table(unsigned int* pTbl, unsigned int TblSel,DRV_valc_channel_t ChannelSel);
void drvif_color_get_valc_table(unsigned int* pTbl, unsigned int TblSel,DRV_valc_channel_t ChannelSel);
unsigned char driver_color_get_valc_enable(void);
void drvif_color_set_pcid_valc_pixel_setting(DRV_pcid2_data_t_val* pPcid2Setting);

//#endif

