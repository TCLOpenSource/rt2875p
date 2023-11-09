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


/*===================================  Types ================================*/

/*================================== Variables ==============================*/

/*================================ Definitions ==============================*/
#define OD_table_length		1089
//#define OD_table_length	289
#ifndef CONFIG_VPQ_TABLE_GENERATE
#define CONFIG_VPQ_TABLE_GENERATE 0
#endif

#if CONFIG_VPQ_TABLE_GENERATE
typedef char bool;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
#endif

typedef struct {
	UINT8 enable;
	UINT32 coef_r;
	UINT32 coef_g;
	UINT32 coef_b;
} DRV_od_ymode;
typedef struct {
	UINT8 r_en;
	UINT8 g_en;
	UINT8 b_en;
	UINT8 mode;
	UINT8 r_th;
	UINT8 g_th;
	UINT8 b_th;
} DRV_od_nr;
typedef struct {
	UINT8 r_gain;
	UINT8 g_gain;
	UINT8 b_gain;
} DRV_od_gain;

typedef struct {
	UINT16 vact;
	UINT16 hact;
	UINT8 dma_mode;
	UINT8 bit_sel;
	UINT8 pixel_encoding;
	UINT8 dma_2p_enable;
	UINT8 dma_clk_sel;
	UINT8 od_4k120_mode;
} DRV_od_dma_ctrl;
typedef struct {
	UINT8 cap_en;
	UINT32 start_addr1;
	UINT32 start_addr2;	
	UINT32 up_limit;
	UINT32 low_limit;
	UINT32 line_step;
	UINT32 line_num;
	UINT32 burst_length;
	UINT32 line128_num;
	UINT8 max_outstanding;
	UINT8 rolling_en;
	UINT32 rolling_space;
} DRV_od_dma_pair;
typedef struct {
	UINT8 bypass_en;
	UINT8 compensation_en;
	UINT8 fisrt_line_more_en;
	UINT8 guarantee_max_qp_en;
	UINT8 first_predict_nc_mode;
	UINT8 variation_maxmin_en;
	UINT8 pqdc_saturation_en;
	UINT8 g_ratio;
	UINT8 block_limit_bit;
	UINT8 line_limit_bit;
	UINT8 frame_limit_bit;
	UINT8 line_more_bit;
	UINT8 cmp_ctrl_para0;
	UINT8 cmp_ctrl_para2;
} DRV_od_dma_comp;
typedef struct {
	UINT8 comp_en;
	UINT8 line_mode;
	UINT32 width_div32;
	UINT32 height;
	UINT8 data_format;
	UINT8 data_bitwidth;
	UINT8 data_color;
	UINT32 line_sum_bit;
	UINT8 old_400_mode;
	UINT8 two_line_predict;
	UINT8 qp_mode;
	UINT8 dic_mode;
	UINT8 jump4_en;
} DRV_od_dma_pqc_pair;
typedef struct {
	UINT32 dynamic_allocate_ratio_max;
	UINT32 dynamic_allocate_ratio_min;
	UINT32 dynamic_allocate_line;
	UINT32 dynamic_allocate_less;
	UINT32 dynamic_allocate_more;
} DRV_od_dma_pqc_alloc;
typedef struct {
	UINT32 balance_rb_ov_th;
	UINT32 balance_rb_ud_th;
	UINT32 balance_rb_give;
	UINT32 balance_g_ov_th;
	UINT32 balance_g_ud_th;
	UINT32 balance_g_give;
} DRV_od_dma_pqc_balance;
typedef struct {
	DRV_od_dma_ctrl dma_ctrl;
	DRV_od_dma_pair dma_pair;
	DRV_od_dma_comp dma_comp;
	DRV_od_dma_pqc_pair dma_pqc_pair;
	DRV_od_dma_pqc_alloc dma_pqc_alloc;
	DRV_od_dma_pqc_balance dma_pqc_balance;
} DRV_od_dma;
typedef struct{
	UINT8 bias_en;
	UINT8 bias_r_en;
	UINT8 bias_g_en;
	UINT8 bias_b_en;
	UINT8 bias_y_en;
	UINT8 bias_m_en;
	UINT8 bias_c_en;
	UINT8 gray2rgb_en;
	UINT8 gray2cmy_en;
	UINT8 color2gray_en;
	UINT8 y_diff_neg_en;
	UINT8 y_diff_pos_en;
	UINT8 offset_clamp_en;
} DRV_od_sat_y_ctrl;

typedef struct{
	UINT8 gray_ofst_gain;
	UINT8 c_ofst_gain;
	UINT8 m_ofst_gain;
	UINT8 y_ofst_gain;
	UINT8 r_ofst_gain;
	UINT8 g_ofst_gain;
	UINT8 b_ofst_gain;
} DRV_od_sat_y_ofst_gain;

typedef struct{
	UINT8 gray_diff_th;
	UINT8 r_diff_th;
	UINT8 g_diff_th;
	UINT8 b_diff_th;
	UINT8 rg_diff_th;
	UINT8 gb_diff_th;
	UINT8 br_diff_th;
	UINT8 diff_th_factor;
} DRV_od_sat_y_thd;

typedef struct {
	UINT8 offset_hi_th;
	UINT8 offset_lo_th;
	UINT8 ydiff_lo_th;
	UINT8 ydiff_hi_th;
	UINT8 offset_slope;
	UINT8 offset_hi_th_gray;
	UINT8 offset_lo_th_gray;
	UINT8 ydiff_lo_th_gray;
	UINT8 ydiff_hi_th_gray;
	UINT8 offset_slope_gray;
} DRV_od_sat_y_slope;

typedef struct{
	DRV_od_sat_y_ctrl ctrl;
	DRV_od_sat_y_ofst_gain gain;
	DRV_od_sat_y_thd thd;
	DRV_od_sat_y_slope slope;
} DRV_od_sat_y;

typedef struct{
	UINT8 enable;
	UINT32 factor;
	UINT16 offset;
	UINT8 vergain[32];
} DRV_OD_VerGain;

typedef struct {
	UINT8 enable;
	UINT8 edge_mode;
	UINT8 coeff0;
	UINT8 coeff1;
	UINT8 coeff2;
	UINT8 coeff3;
	UINT8 coeff4;
	UINT8 r_diff_nr_th;
	UINT8 g_diff_nr_th;
	UINT8 b_diff_nr_th;
} DRV_od_scaling;

typedef struct{
	UINT8 enable;
	UINT8 weight;
	UINT8 nr_thd;
	UINT8 edge_hhf_thd;
} DRV_od_localgain;

typedef struct {
	DRV_od_ymode od_ymode;
	DRV_od_scaling od_scaling;
	DRV_od_localgain od_localgain;
	DRV_od_nr od_nr[2];
	DRV_od_gain od_gain[2];
} DRV_od_ctrl;

typedef struct {
	UINT8 cur_idx[33];
	UINT8 pre_idx[33];
	UINT8 value[3][1089];
} DRV_od_lut_ctrl;

/*================================ Functions ==============================*/
unsigned int drvifget_align_value(unsigned int input, unsigned int align_value);
unsigned char drvif_color_od_enable_get(void);
void drvif_color_od_enable_set(unsigned char od_enable);
unsigned char drvif_color_od_rec_enable_get(void);
void drvif_color_od_rec_enable_set(unsigned char od_rec_enable);
UINT8 drvif_color_od_scaling_enable_get(void);
void drvif_color_od_scaling_enable_set(UINT8 od_scaling_enable);
DRV_od_ymode drvif_color_od_Ymode_get(void);
void drvif_color_od_Ymode_set(DRV_od_ymode arg);
unsigned char drvif_color_od_gain_get(unsigned char channel);
void drvif_color_od_gain_set(DRV_od_gain* od_delta_gain);
void drvif_color_set_od_gain_RGB(unsigned char rGain, unsigned char gGain, unsigned char bGain);
void drvif_color_od_nr_set(DRV_od_nr* od_nr);
void drvif_color_od_scaling_set(DRV_od_scaling od_scaling);
void drvif_color_od_localgain_set(DRV_od_localgain od_localgain);
void drvif_color_od_sat_y_set(DRV_od_sat_y od_sat_y);
void drvif_color_od_set_vergain(DRV_OD_VerGain* od_vergain);
bool drvif_color_od_dma_line_size_get(DRV_od_dma *oddma);
bool drvif_color_od_dma_frame_size_get(DRV_od_dma *oddma);
void drvif_color_od_dma_addr_set(DRV_od_dma *oddma);
void drvif_color_od_dma_ctrl_set(DRV_od_dma *oddma);
void drvif_color_od_dma_pair_set(DRV_od_dma *oddma);
unsigned char drvif_color_od_dma_enable_get(void);
void drvif_color_od_dma_cap_enable_set(unsigned char enable);
void drvif_color_od_dma_disp_enable_set(unsigned char enable);
void drvif_color_od_dma_pqc_set(DRV_od_dma *oddma);
void drvif_color_od_dma_pqc_bit_set(DRV_od_dma * oddma);
void drvif_color_od_dma_pqc_pair_set(DRV_od_dma *oddma);
void drvif_color_od_dma_pqc_enable_set(unsigned char bypass_en, unsigned char pqc_enable);
void drvif_color_od_freeze(unsigned char freeze);
//void drvif_color_od(unsigned char bOD);
bool drvif_color_od_pqc(UINT16 height, UINT16 width, UINT8 bit_sel, UINT8 mode, UINT8 FrameLimitBit, int dataColor, int dataFormat);
bool drvif_color_od_pqc_120hz(UINT16 height, UINT16 width, UINT8 bit_sel, UINT8 dma_mode, UINT8 FrameLimitBit, int dataColor, int dataFormat);
DRV_od_lut_ctrl* drvif_color_od_access_lut_struct(bool recursive);
void drvif_color_od_table_17x17_set(unsigned char *pODtable, unsigned char tableType, unsigned char colorChannel);
void drvif_color_od_table_17x17_get(unsigned char *pODtable);
void drvif_color_od_table_33x33_set(unsigned char *pODtable, unsigned char tableType, unsigned char colorChannel);
void drvif_color_od_table_33x33_get(bool recursive, unsigned char *pODtable);
void drvif_color_od_table_33x33_set_dbus(UINT8 tableType, UINT32 lut_addr, UINT8 colorChannel);
void drif_color_od_lut_struct_value_set(unsigned char *pODtable);
void drif_color_od_lut_index_get(bool recursive, unsigned char *preIdx, unsigned char *curIdx);
void drif_color_od_lut_index_set(bool recursive, unsigned char *preIdx, unsigned char *curIdx);
void drvif_color_safe_od_enable(unsigned char bEnable, unsigned char bOD_OnOff_Switch);
void drvif_color_set_od_bits(unsigned char FrameLimitBit);
unsigned char drvif_color_get_od_bits(void);
unsigned char drvif_color_get_od_dma_mode(void);
char drvif_color_set_od_2p(unsigned char mode);
void drvif_color_set_od_dma_HW_reset(void);

#endif

