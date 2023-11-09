/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2020
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for color related functions.
 *
 * @author 		
 * @date 		2020/03/13
 * @version 	1
 */

#ifndef _SIW_PDF_H
#define _SIW_PDF_H
/*============================ Module dependency  ===========================*/
/*================================ Definitions ==============================*/
#define NUM_PDF_tbl 2
/*================================== Variables ==============================*/
/*===================================  Types ================================*/
typedef struct
{
        int  range_top;
        int  range_bot;
        int  range_left;
        int  range_right;

}DRV_PDF_fg_range_t;

/*fg*/
typedef struct
{
        int  xtalk_fg_gl_0;
        int  xtalk_fg_gl_1;
        int  xtalk_fg_gl_2;

}DRV_PDF_fg_gl_t;

typedef struct
{
	int  fg_gl_r[4][4];
	int  fg_gl_g[4][4];
	int  fg_gl_b[4][4];

}DRV_PDF_fg_gl_inUnit_t;

typedef struct
{
	int  fg_opr_r[4][4];
	int  fg_opr_g[4][4];
	int  fg_opr_b[4][4];

}DRV_PDF_fg_sel_opr_t;

typedef struct
{
	int  xtalk_fg_det_threshold_0;
	int  xtalk_fg_det_threshold_1;
	int  xtalk_fg_det_threshold_2;
	int  xtalk_fg_det_threshold_3;
	int  xtalk_fg_det_threshold_4;
	int  xtalk_fg_det_threshold_5;
	int  xtalk_fg_det_threshold_6;
	int  xtalk_fg_det_threshold_7;
}DRV_PDF_fg_temporal_det_t;

typedef struct
{
	int  xtalk_fg_undet_threshold_0;
	int  xtalk_fg_undet_threshold_1;
	int  xtalk_fg_undet_threshold_2;
	int  xtalk_fg_undet_threshold_3;
	int  xtalk_fg_undet_threshold_4;
	int  xtalk_fg_undet_threshold_5;
	int  xtalk_fg_undet_threshold_6;
	int  xtalk_fg_undet_threshold_7;
}DRV_PDF_fg_temporal_undet_t;

typedef struct
{
	int  xtalk_fg_hit_threshold_0;
	int  xtalk_fg_hit_threshold_1;
	int  xtalk_fg_hit_threshold_2;
	int  xtalk_fg_hit_threshold_3;
	int  xtalk_fg_hit_threshold_4;
	int  xtalk_fg_hit_threshold_5;
	int  xtalk_fg_hit_threshold_6;
	int  xtalk_fg_hit_threshold_7;
}DRV_PDF_fg_area_hit_t;

typedef struct
{
	DRV_PDF_fg_gl_t fg_gl[8];
	DRV_PDF_fg_temporal_det_t fg_temporal_det;
	DRV_PDF_fg_temporal_undet_t fg_temporal_undet;
	DRV_PDF_fg_area_hit_t fg_area_hit;	
	DRV_PDF_fg_gl_inUnit_t fg_gl_inUnit;
	DRV_PDF_fg_sel_opr_t fg_sel_opr;
}DRV_PDF_fg_t;


/*bg*/
typedef struct
{
        int  xtalk_bg_gl_0;
        int  xtalk_bg_gl_1;
        int  xtalk_bg_gl_2;

}DRV_PDF_bg_gl_t;

typedef struct
{
	int  bg_gl_r[4][4];
	int  bg_gl_g[4][4];
	int  bg_gl_b[4][4];

}DRV_PDF_bg_gl_inUnit_t;

typedef struct
{
	int  bg_opr_r[4][4];
	int  bg_opr_g[4][4];
	int  bg_opr_b[4][4];

}DRV_PDF_bg_sel_opr_t;


typedef struct
{
	int  xtalk_bg_det_threshold_0;
	int  xtalk_bg_det_threshold_1;
	int  xtalk_bg_det_threshold_2;
	int  xtalk_bg_det_threshold_3;
	int  xtalk_bg_det_threshold_4;
	int  xtalk_bg_det_threshold_5;
	int  xtalk_bg_det_threshold_6;
	int  xtalk_bg_det_threshold_7;
}DRV_PDF_bg_temporal_det_t;

typedef struct
{
	int  xtalk_bg_undet_threshold_0;
	int  xtalk_bg_undet_threshold_1;
	int  xtalk_bg_undet_threshold_2;
	int  xtalk_bg_undet_threshold_3;
	int  xtalk_bg_undet_threshold_4;
	int  xtalk_bg_undet_threshold_5;
	int  xtalk_bg_undet_threshold_6;
	int  xtalk_bg_undet_threshold_7;
}DRV_PDF_bg_temporal_undet_t;

typedef struct
{
	int  xtalk_bg_hit_threshold_0;
	int  xtalk_bg_hit_threshold_1;
	int  xtalk_bg_hit_threshold_2;
	int  xtalk_bg_hit_threshold_3;
	int  xtalk_bg_hit_threshold_4;
	int  xtalk_bg_hit_threshold_5;
	int  xtalk_bg_hit_threshold_6;
	int  xtalk_bg_hit_threshold_7;
}DRV_PDF_bg_area_hit_t;

typedef struct
{
	DRV_PDF_bg_gl_t bg_gl[8];
	DRV_PDF_bg_temporal_det_t bg_temporal_det;
	DRV_PDF_bg_temporal_undet_t bg_temporal_undet;
	DRV_PDF_bg_area_hit_t bg_area_hit;	
	DRV_PDF_bg_gl_inUnit_t bg_gl_inUnit;
	DRV_PDF_bg_sel_opr_t bg_sel_opr;
	
}DRV_PDF_bg_t;

typedef struct
{
	unsigned char Enable;
	DRV_PDF_fg_range_t fg_range;
	DRV_PDF_fg_t fg;
	DRV_PDF_bg_t bg;	
}DRV_PDF_Ctrl_t;

/*=================================== Functions ================================*/
void drvif_color_set_pdf_enable(unsigned char bEnable);
void drvif_color_set_pdf_fg_range(DRV_PDF_fg_range_t *pFG);
void drvif_color_set_pdf_fg_gl(DRV_PDF_fg_t *fg);
void drvif_color_set_pdf_fg_temporal_det(DRV_PDF_fg_temporal_det_t *pfg_temporal_det);
void drvif_color_set_pdf_fg_temporal_undet(DRV_PDF_fg_temporal_undet_t *pfg_temporal_undet);
void drvif_color_set_pdf_fg_area_hit(DRV_PDF_fg_area_hit_t *pfg_area_hit);
void drvif_color_set_pdf_fg_gl_inUnit(DRV_PDF_fg_gl_inUnit_t *pfg_gl_inUnit);
void drvif_color_set_pdf_fg_sel_opr(DRV_PDF_fg_sel_opr_t *pfg_sel_opr);
void drvif_color_set_pdf_bg_gl(DRV_PDF_bg_t *bg);
void drvif_color_set_pdf_bg_temporal_det(DRV_PDF_bg_temporal_det_t *pbg_temporal_det);
void drvif_color_set_pdf_bg_temporal_undet(DRV_PDF_bg_temporal_undet_t *pbg_temporal_undet);
void drvif_color_set_pdf_bg_area_hit(DRV_PDF_bg_area_hit_t *pbg_area_hit);
void drvif_color_set_pdf_bg_gl_inUnit(DRV_PDF_bg_gl_inUnit_t *pbg_gl_inUnit);
void drvif_color_set_pdf_bg_sel_opr(DRV_PDF_bg_sel_opr_t *pbg_sel_opr);



#endif
