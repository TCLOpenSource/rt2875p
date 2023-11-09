/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2021
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for color related functions.
 *
 * @author 		Bill Chou
 * @date 		2021/03/16
 * @version 	1
 */

#ifndef _GSR2_H
#define _GSR2_H

#define GSR2_Table_NUM 1
#define GSR2_IDX 17

typedef struct {
	unsigned short gsr2_tonemap_curve[GSR2_IDX];
} DRV_GSR2_Tonemap_curve;

typedef struct {
	unsigned short gsr2_sat_mode;
	unsigned short gsr2_gain_by_sat_curve[GSR2_IDX];
} DRV_GSR2_gain_by_sat;

typedef struct {
	unsigned char gsr_gain_by_Y_coef_B;
	unsigned char gsr_gain_by_Y_coef_G;
	unsigned char gsr_gain_by_Y_coef_R;
	unsigned char gsr_gain_by_Y_en;
} DRV_GSR2_gain_by_y;

typedef struct {
	unsigned char GSR2_global_gain;
	DRV_GSR2_Tonemap_curve GSR2_tonemap_curve;
	DRV_GSR2_gain_by_sat GSR2_gain_by_sat;
	DRV_GSR2_gain_by_y GSR2_gain_by_y;
	DRV_GSR2_Tonemap_curve GSR2_gain_by_y_curve; // curve for gain_by_Y mode
} DRV_GSR2_Table;

#define VIP_SLD2GlobalSLD_CNT_Score_Num 8	// value should be power of 2 for cal
#define VIP_SLD2GlobalSLD_CNT_Score_Num_Shift 3 
typedef struct _VIP_SLD2GlobalSLD_CTRL {
	unsigned short smooth_step;
	unsigned short Debug_Mode_Log;	
	unsigned short Debug_Mode_Log_Delay;

	unsigned short Logo_TH;
	unsigned short SLD_CNT_Score[VIP_SLD2GlobalSLD_CNT_Score_Num+1];	// 1024 = 1

	unsigned short reserved_0;
	unsigned short reserved_1;
	unsigned short reserved_2;
	unsigned short reserved_3;
	unsigned short reserved_4;

	unsigned short Debug_Blending_Score_by_th;
} VIP_SLD2GlobalSLD_CTRL;

typedef struct _VIP_SLD2GlobalSLD_INFO {
	unsigned short Blending_Score;
	unsigned short Blending_Score_by_th;
	unsigned short Blending_Score_Final;
} VIP_SLD2GlobalSLD_INFO;

void drvif_color_set_GSR2_Global_Gain(unsigned char global_gain);
void drvif_color_set_GSR2_ToneMapping(DRV_GSR2_Tonemap_curve *curve);
void drvif_color_set_GSR2_Gain_by_Sat(DRV_GSR2_gain_by_sat *sat);
void drvif_color_set_GSR2_Gain_by_Y(DRV_GSR2_gain_by_y *gain_y);
VIP_SLD2GlobalSLD_CTRL* drvif_fwif_color_get_SLD2GlobalSLD_CTRL(void);
VIP_SLD2GlobalSLD_INFO* drvif_fwif_color_get_SLD2GlobalSLD_INFO(void);
DRV_GSR2_Table* drvif_fwif_color_get_SLD2GlobalSLD_TBL(void);

#endif
