/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2020
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for color related functions.
 *
 * @author 		Willy Hsu
 * @date 		2020/03/04
 * @version 	1
 */

#ifndef _FREQUENCY_DETECTION_H
#define _FREQUENCY_DETECTION_H
/*============================ Module dependency  ===========================*/
/*================================ Definitions ==============================*/
/*================================== Variables ==============================*/
/*===================================  Types ================================*/
typedef struct {
	int Coef[11];
	unsigned char Div;
	unsigned char Switch;
	unsigned char BitSft_A;
	unsigned char BitSft_B;
} DRV_FreqDet_Filter_t;

typedef struct {
	unsigned int HSta;
	unsigned int Width;
	unsigned int VSta;
	unsigned int Height;
} DRV_FreqDet_Region_t;

typedef struct {
	bool Enable;
	DRV_FreqDet_Region_t RgnCtrl;
	DRV_FreqDet_Filter_t Tap21;
	DRV_FreqDet_Filter_t Tap15[3];
} DRV_FreqDet_Ctrl_t;

typedef enum {
	_FD_FLT_TAP21_ = 0,
	_FD_FLT_TAP15_1,
	_FD_FLT_TAP15_2,
	_FD_FLT_TAP15_3,
	_FD_FLT_MAX_NUM
} DRV_FreqDet_FilterType_t;

typedef enum {
	_FD_HIST_PURE = 0,
	_FD_HIST_COMP,
	_FD_HIST_TYPE_ERR
} DRV_FreqDet_HistType_t;

//for rtice start
typedef struct {
	int	bitshift_a0;
	int	bitshift_b0;
	int	h_filter_switch_0;
	int	h_filter_band0_div;
	int	h_filter_band0_coef[21];
	int	bitshift_a1;
	int	bitshift_b1;
	int	h_filter_switch_1;
	int	h_filter_band1_div;
	int	h_filter_band1_coef[15];
	int	bitshift_a2;
	int	bitshift_b2;
	int	h_filter_switch_2;
	int	h_filter_band2_div;
	int	h_filter_band2_coef[15];
	int	bitshift_a3;
	int	bitshift_b3;
	int	h_filter_switch_3;
	int	h_filter_band3_div;
	int	h_filter_band3_coef[15];
} FREQ_DET_4BAND_FILTER_RTICE;

typedef struct {
	int	filter_h_start;
	int	filter_h_size;
	int	filter_v_start;
	int	filter_v_size;
} FREQ_DET_4BAND_FILTERRANGE_RTICE;

typedef struct {
	int	a0_hist[16];
	int	b0_hist[16];
	int	a1_hist[16];
	int	b1_hist[16];
	int	a2_hist[16];
	int	b2_hist[16];
	int	a3_hist[16];
	int	b3_hist[16];
} FREQ_DET_HISTOGRAM_RTICE;

typedef struct {
	FREQ_DET_4BAND_FILTERRANGE_RTICE range;
	FREQ_DET_4BAND_FILTER_RTICE filter;
	FREQ_DET_HISTOGRAM_RTICE hist;
} FREQ_DET_RTICE;
//for rtice end

/*=================================== Functions ================================*/
void drvif_color_set_frequency_detection_enable(unsigned char bEnable);
void drvif_color_set_set_frequency_detection_region(DRV_FreqDet_Region_t* pRgn);
void drvif_color_set_set_frequency_detection_filter(DRV_FreqDet_Filter_t* pFlt, DRV_FreqDet_FilterType_t FilterSel);

void drvif_color_get_ferquency_detection_histogram(unsigned int *pHist, DRV_FreqDet_FilterType_t FilterSel, DRV_FreqDet_HistType_t TypeSel);
#endif
