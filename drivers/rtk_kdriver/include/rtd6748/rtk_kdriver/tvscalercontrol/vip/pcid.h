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

/*================================ Definitions ==============================*/
#define PCID_TBL_LEN	 19*19
#define PCID_WTTBL_LEN   8*24
#define LGE_POD_TBL_LEN	1848

/*PCID2 Structure*/
#define _REF_COLOR_R 0
#define _REF_COLOR_G 1
#define _REF_COLOR_B 2

#define _REF_POINT_PREV 0
#define _REF_POINT_CURR 1
#define _REF_POINT_NEXT 2

#define _REF_LINE_PREV 0
#define _REF_LINE_CURR 1

#define RGN_NA 255

/*================================== Variables ==============================*/
extern unsigned char Rgn4x4MappingTbl[16][4];
extern unsigned char Rgn1x10MappingTbl[10][2];
extern unsigned int pcidRgnTbl[16][3][PCID_TBL_LEN];
extern unsigned int pcidRgnWtTbl[PCID_WTTBL_LEN];

extern unsigned int pcidRowBnd[17];
extern unsigned int pcidColBnd[17];
extern unsigned int TransTbl[361];

/*===================================  Types ================================*/
enum {
	_PCID_DATA_DEFAULT = 0,
	_PCID_DATA_LGE,
	_PCID_DATA_MAX
};

enum {
	_PCID_PXLREF_DEFAULT = 0,
	_PCID_PXLREF_SINGLE_GATE,
	_PCID_PXLREF_DUAL_GATE,
	_PCID_PXLREF_LGE,
	_PCID_PXLREF_BOE_43_45,
        _PCID_PXLREF_BOE_FAB_43_45,
	_PCID_PXLREF_MAX
};

enum {
	_PCID_RGN_17x17_4x4 = 0,
	_PCID_RGN_19x19_1x10,
	_PCID_RGN_MAX
};

enum {
	_PCID_RGNTBL_DEFAULT = 0,
	_PCID_RGNTBL_LGE = 0,
};

typedef struct {
	unsigned char line_sel;
	unsigned char point_sel;
	unsigned char color_sel;
} DRV_pcid_pixel_ref_setting_t;

typedef struct {
	DRV_pcid_pixel_ref_setting_t even_r;
	DRV_pcid_pixel_ref_setting_t even_g;
	DRV_pcid_pixel_ref_setting_t even_b;

	DRV_pcid_pixel_ref_setting_t odd_r;
	DRV_pcid_pixel_ref_setting_t odd_g;
	DRV_pcid_pixel_ref_setting_t odd_b;
} DRV_pcid_line_ref_setting_t;

typedef struct {
	DRV_pcid_line_ref_setting_t line1;
	DRV_pcid_line_ref_setting_t line2;
	DRV_pcid_line_ref_setting_t line3;
	DRV_pcid_line_ref_setting_t line4;
} DRV_pcid_ref_setting_t;

typedef struct {
	// Noise reduction
	bool thd_en;
	bool thd_mode;
	unsigned int tbl1_r_th;
	unsigned int tbl1_g_th;
	unsigned int tbl1_b_th;
	// XTR
	bool xtr_tbl1_en;
	// Mode selection
	unsigned char FstBndMode;
	unsigned int FstBndCFG;
	unsigned char GenBndMode;
	unsigned int GenBndCFG;
} DRV_pcid_data_t;

typedef struct{
	unsigned char Enable;
	unsigned int Width;
	unsigned int Height;
	unsigned int HorBldStep;
       unsigned int VerBldStep;	
}DRV_pcid_RgnWt_t;

typedef struct {
	unsigned char Rgn1x10Enable;
	unsigned char Tbl19x19Enable;
	unsigned int HorIdx[3];
	unsigned int VerIdx[9];
	unsigned char HorBldStep[3];
	unsigned char VerBldStep[9];
	unsigned int ColBnd[17];
	unsigned int RowBnd[17];
	DRV_pcid_RgnWt_t RgnWt;
} DRV_pcid_RgnTbl_t;


typedef enum {
	_PCID_COLOR_R = 0,
	_PCID_COLOR_G,
	_PCID_COLOR_B,
	_PCID_COLOR_ALL,
	_PCID_COLOR_ERR
} DRV_pcid_channel_t;

typedef enum {
	_PCID_RGN_4x4 = 0,
	_PCID_RGN_1x10,
	_PCID_RGN_ERR
} DRV_pcid_RgnType_t;


typedef enum {
	_PCLRC_PCLRC_LOD = 0,
	_PCLRC_LOD_PCLRC,
	_PCLRC_Pos_ERR
} DRV_siw_Pclrc_Pos_t;

/*=================================== Functions ================================*/
void drvif_color_set_pcid_enable(unsigned char bpcid);
void drvif_color_set_pcid2_enable(unsigned char bpcid2);
void drvif_color_set_Tcon_pcid1_pcid2_LGPOD_fcic_enable(unsigned char pcid1_en,unsigned char pcid_2_en,unsigned char LGPOD_en,unsigned char fcic_en);

void drvif_color_set_pcid_regional_table_setting(DRV_pcid_RgnTbl_t* pRgnTbl);
void drvif_color_set_pcid_data_setting(DRV_pcid_data_t* pPcid2Setting);
void drvif_color_set_pcid_pixel_reference_setting(DRV_pcid_ref_setting_t* pPxlRef);
void drvif_color_set_pcid_regional_table_valuetable(unsigned int *pTblValue, unsigned int RgnSel, DRV_pcid_channel_t Channel);
void drvif_color_set_pcid_regional_weight_table_valuetable(unsigned int*pTblValue);
void drvif_color_get_pcid_regional_table_setting(DRV_pcid_RgnTbl_t* pRgnTbl);
void drvif_color_get_pcid_pixel_reference_setting(DRV_pcid_ref_setting_t* pPxlRef);
void drvif_color_get_pcid_data_setting(DRV_pcid_data_t* pPcid2Setting);

void drvif_color_get_pcid_regional_table_valuetable(unsigned int *pRetTbl, unsigned int RgnSel, DRV_pcid_channel_t Channel);
void drvif_color_get_pcid_regional_weight_table_valuetable(unsigned int*pTblValue);
char drvif_color_get_pcid_enable(void);
char drvif_color_get_pcid2_enable(void);
unsigned char drvif_color_get_pcid_TableSize(void);
DRV_pcid_RgnType_t drvif_color_get_pcid_RegionType(void);

void drvif_color_set_pcid_data_access_read(unsigned short x, unsigned short y, unsigned char loc, unsigned char cross_bar_en);
char drvif_color_get_pcid_data_access_read(unsigned short *R, unsigned short *G, unsigned short *B);
unsigned int drvif_color_get_pcid_data_access_read_R_mask(void);

void drvif_color_set_siw_PCLRC_valuetable(unsigned char *pTblValue);
void drvif_color_get_siw_PCLRC_valuetable(unsigned char *pTblValue);
void drvif_color_enable_PCLRC_CLK(unsigned char bEnable);
void drvif_color_PCLRC_Register_Setting(void);

void drvif_color_set_enable_PCLRC_Funciton(unsigned char bEnable,unsigned char position);
void drvif_color_get_enable_PCLRC_Funciton(unsigned char *bEnable,unsigned char *position);

void drvif_color_set_LGD_POD(unsigned char* pTblValue);
void drvif_color_set_LGD_POD_enable(unsigned char blgdpod);
void drvif_color_enable_LGD_POD_CLK(void);
void drvif_color_enable_LGD_POD_double_buffer(void);
void drvif_color_set_PCLRC_enable(unsigned char bEnable);


#endif



