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

#ifndef _PCID_H
#define _PCID_H
/*============================ Module dependency  ===========================*/

#include <scaler/vipCommon.h>

/*================================ Definitions ==============================*/
#define PCID_TABLE_LEN		289
#define POD_TABLE_LEN		578 //289*2

/*PCID2 Structure*/
#define _REF_COLOR_R 0
#define _REF_COLOR_G 1
#define _REF_COLOR_B 2

#define _REF_POINT_PREV 0
#define _REF_POINT_CURR 1
#define _REF_POINT_NEXT 2

#define _REF_LINE_PREV 0
#define _REF_LINE_CURR 1

/*================================== Variables ==============================*/

/*===================================  Types ================================*/
typedef struct {
	unsigned char line_sel;
	unsigned char point_sel;
	unsigned char color_sel;
} DRV_pcid2_pixel_ref_setting_t;

typedef struct {
	DRV_pcid2_pixel_ref_setting_t line1_even_r;
	DRV_pcid2_pixel_ref_setting_t line1_even_g;
	DRV_pcid2_pixel_ref_setting_t line1_even_b;

	DRV_pcid2_pixel_ref_setting_t line1_odd_r;
	DRV_pcid2_pixel_ref_setting_t line1_odd_g;
	DRV_pcid2_pixel_ref_setting_t line1_odd_b;

	DRV_pcid2_pixel_ref_setting_t line2_even_r;
	DRV_pcid2_pixel_ref_setting_t line2_even_g;
	DRV_pcid2_pixel_ref_setting_t line2_even_b;

	DRV_pcid2_pixel_ref_setting_t line2_odd_r;
	DRV_pcid2_pixel_ref_setting_t line2_odd_g;
	DRV_pcid2_pixel_ref_setting_t line2_odd_b;

	DRV_pcid2_pixel_ref_setting_t line3_even_r;
	DRV_pcid2_pixel_ref_setting_t line3_even_g;
	DRV_pcid2_pixel_ref_setting_t line3_even_b;

	DRV_pcid2_pixel_ref_setting_t line3_odd_r;
	DRV_pcid2_pixel_ref_setting_t line3_odd_g;
	DRV_pcid2_pixel_ref_setting_t line3_odd_b;

	DRV_pcid2_pixel_ref_setting_t line4_even_r;
	DRV_pcid2_pixel_ref_setting_t line4_even_g;
	DRV_pcid2_pixel_ref_setting_t line4_even_b;

	DRV_pcid2_pixel_ref_setting_t line4_odd_r;
	DRV_pcid2_pixel_ref_setting_t line4_odd_g;
	DRV_pcid2_pixel_ref_setting_t line4_odd_b;

} DRV_pcid2_ref_setting_t;

typedef struct {
	// LSB addition
	/* //mac6p removed
	bool lsb_add_en;
	bool lsb_add_nr_en;
	bool lsb_add_slope_check_en;
	int lsb_add_nr_thd;
	*/
	// Noise reduction
	bool pcid_thd_en;
	bool pcid_thd_mode;
	int pcid_tbl1_r_th;
	int pcid_tbl1_g_th;
	int pcid_tbl1_b_th;
	int pcid_tbl2_r_th;
	int pcid_tbl2_g_th;
	int pcid_tbl2_b_th;
	// XTR
	bool xtr_tbl1_en;
	bool xtr_tbl2_en;
	// Mode selection
	bool boundary_mode;
	// Subpixel reference setting
	DRV_pcid2_ref_setting_t pixel_ref;
	} DRV_pcid2_data_t;

typedef enum {
	_PCID2_COLOR_R = 0,
	_PCID2_COLOR_G,
	_PCID2_COLOR_B,
	_PCID2_COLOR_ALL,
	_PCID2_COLOR_ERR
} DRV_pcid2_channel_t;
#if 0//ndef CONFIG_PANEL_TCON_BIN_COMPILE
typedef struct
{
	int valc_en;
	int hpf_clip;
	int hpf_weight[16];
	int sat_weight[16];
} DRV_pcid_valc_t;
#endif //CONFIG_PANEL_TCON_BIN_COMPILE
typedef struct {
	bool RgnWeightEnable;
	unsigned int Sta_x;
	unsigned int Sta_y;
	unsigned char InputSz;
	unsigned char SftSel;
	unsigned char HorWeight[25];
	unsigned char VerWeight[10];
} DRV_pcid2_RgnWeight_t;

typedef struct {
	DRV_pcid2_data_t pcid2_data;
	unsigned int pPcid2PolTable[3][8];
	DRV_pcid_valc_t pcid_valc;
	char POD_DATA[6][289];
} DRV_POD_DATA_table_t;


/* PCID */
void drvif_color_pcid_table(unsigned char *pPcidtable);
void drvif_color_pcid(unsigned char bpcid);
void drvif_color_pcid_enable(unsigned char bpcid_enable);
void drvif_color_pcid_valuetable(unsigned int *pPcidtable);
void drvif_color_pcid_poltable(void);
void drvif_color_pcid_TblTrans_Target2Delta(unsigned char *TargetTbl);
void drvif_color_get_pcid_enable(unsigned char* bpcid_enable);
/* PCID2 aka. PXLOD */
void drvif_color_pcid2_enable(unsigned char bpcid2);
void drvif_color_pcid2_valuetable(unsigned int * ValueTBL, unsigned int TblSel, DRV_pcid2_channel_t Channel);
void drvif_color_pcid2_poltable(unsigned int* pPcid2PolTable, DRV_pcid2_channel_t color);
void drvif_color_pcid2_pixel_setting(DRV_pcid2_data_t* pPcid2Setting);
void drvif_color_get_pcid2_enable(unsigned char* bpcid2);
void drvif_color_get_pcid2_pixel_setting(DRV_pcid2_data_t* pPcid2Setting);

void drvif_color_set_pcid_data_access_read(unsigned short x, unsigned short y, unsigned char loc, unsigned char cross_bar_en);
char drvif_color_get_pcid_data_access_read(unsigned short *R, unsigned short *G, unsigned short *B);
unsigned int drvif_color_get_pcid_data_access_read_R_mask(void);

/* VALC */
void drvif_color_pcid_VALC_enable(unsigned char ucEnable);
void drvif_color_get_pcid_VALC_enable(int* ucEnable);
/* VALC high frequency and saturation region protection */
void drvif_color_pcid_VALC_protection(DRV_pcid_valc_t* pVALCprotectSetting);
void drvif_color_get_pcid_VALC_protection(DRV_pcid_valc_t* pVALCprotectSetting);

/* Regional weighting */
void drvif_color_set_pcid_regional_weight_setting(DRV_pcid_valc_t* pRgnWeight);
void drvif_color_get_pcid_regional_weight_setting(DRV_pcid_valc_t* pRgnWeight);

/* For RTICE */
void drvif_color_get_pcid_valuetable(unsigned int *pRetTbl, unsigned int TblSel, DRV_pcid2_channel_t Channel);
#endif


