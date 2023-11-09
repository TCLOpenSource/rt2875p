#ifndef _SRNN_H_
#define _SRNN_H_

#include <scaler/vipCommon.h>

/*******************************************************************************
*Definitions*
******************************************************************************/
#define SRNN_Weight_Num_mode0 19044
#define SRNN_Weight_Num_mode1 19920
#define SRNN_Weight_Num_mode2 5052
#define SRNN_Weight_Num_mode4 22000

#if 0 //moved to vipCommon
/*******************************************************************************
*Definitions*
******************************************************************************/
#define SRNN_Adjust_Table_MAX 6
/*******************************************************************************
 * Structure
 ******************************************************************************/
typedef struct {
// chen 0324
//  UINT8 srnn_func_en;
    unsigned char bbbsr_en;
    unsigned char wdsr_y_delta_gain;
    unsigned char bbbsr_var_diff_clip_hith;
    unsigned char bbbsr_var_diff_clip_loth;
    unsigned char bbbsr_matrix33_select1_en;
    unsigned char bbbsr_matrix33_select2_en;
} DRV_srnn_ctrl;
typedef struct {
    unsigned char bbbsr_blend_diff_clip1_en;
    unsigned char bbbsr_clip_hi_method;
    unsigned char bbbsr_min_var_diff_th;
    unsigned char bbbsr_min_var_diff_clip_hi_min;
    unsigned char bbbsr_min_var_diff_clip_hi_gain;
    unsigned char bbbsr_max_min_var_gain;
    unsigned char bbbsr_slope_clip1;
} DRV_srnn_diff_clip1;
typedef struct {
    unsigned char bbbsr_blend_diff_clip2_en;
    unsigned char bbbsr_max_var_loth;
    unsigned char bbbsr_slope_clip2;
} DRV_srnn_diff_clip2;
typedef struct {
    unsigned char bbbsr_blend_diff_clip3_en;
    unsigned char bbbsr_dir_confid_len;
    unsigned char bbbsr_dir_confid_loth;
    unsigned char bbbsr_slope_clip3;
    unsigned char bbbsr_max_var_loth2;
    unsigned char bbbsr_slope_clip3_weight;
    unsigned char bbbsr_dir_weight_dy_en;
    unsigned char bbbsr_dir_weight_dy_gain1;
    unsigned char bbbsr_dir_weight_dy_th;
    unsigned char bbbsr_dir_weight_dy_hith;
    unsigned char bbbsr_dir_weight_dy_gain2;
    unsigned char bbbsr_dir_weight_dy_loth;
} DRV_srnn_diff_clip3;
typedef struct {
    unsigned char bbbsr_var_cur_loth;
    unsigned char bbbsr_var_cur_w_loth;
    unsigned char bbbsr_var_cur_w_hith;
    unsigned char bbbsr_slope_weight_var;
} DRV_srnn_weight;

// chen 0324
typedef struct {
    unsigned char bbbsr_diff_max_flat_en;
    unsigned char bbbsr_diff_delta_coring;
    unsigned char bbbsr_diff_delta_gain;
    unsigned char bbbsr_diff_max_select;
    unsigned char bbbsr_diff_max_flat_coring;
    unsigned char bbbsr_diff_max_flat_gain;
} DRV_srnn_flat_gradient;

typedef struct {
    unsigned char bbbsr_delta_y_gain_en;
    unsigned char bbbsr_delta_y_coring;
    unsigned char bbbsr_diff_delta_y_gain;
} DRV_srnn_delta_y_control;
//end chen 0324

typedef struct {
    DRV_srnn_ctrl srnn_ctrl;                                // 6 //chen 0324 //7
    DRV_srnn_diff_clip1 srnn_diff_clip1;                    // 7
    DRV_srnn_diff_clip2 srnn_diff_clip2;                    // 3
    DRV_srnn_diff_clip3 srnn_diff_clip3;                    // 12
    DRV_srnn_weight srnn_weight;                            // 4
//  DRV_srnn_flat_gradient srnn_flat_gradient;				// 9
// chen 0324
	DRV_srnn_flat_gradient srnn_flat_gradient;            	// 6
	DRV_srnn_delta_y_control srnn_delta_y_control;          // 3
//end chen 0324
} DRV_srnn_table;
#endif

/*******************************************************************************
*functions*
******************************************************************************/

void drvif_srnn_clk_enable(unsigned char clk_srnn_en);
unsigned char drvif_srnn_bbbsr_enable_get(void);
void drvif_srnn_bbbsr_enable_set(unsigned char bbbsr_en);
void drvif_srnn_head_bias_set(unsigned short *para, int idx);
void drvif_srnn_tail_bias_set(int ai_sr_mode, unsigned short *para, int idx);
void drvif_srnn_resblk_3x3_conv_read(unsigned short *para, int i);
void drvif_srnn_resblk_3x3_conv_set(unsigned short *para, int i);
void drvif_srnn_resblk_1x3_conv_bias_get(int sr_mode,int resblk, unsigned short *para, int count);
void drvif_srnn_resblk_1x3_conv_bias_set(int sr_mode,int resblk, unsigned short *para, int count);
void drvif_srnn_model_get(unsigned short *para, unsigned int ai_sr_mode);
void drvif_srnn_model_set(unsigned short *para,unsigned char ai_sr_mode);
int drif_color_NNSR_Tbl2DMA_mode0(unsigned short *pinArray, unsigned short *poutArray);
int drif_color_NNSR_Tbl2DMA_mode1(unsigned short *pinArray, unsigned short *poutArray);
int drif_color_NNSR_Tbl2DMA_mode2(unsigned short *pinArray, unsigned short *poutArray);
void drvif_color_Set_NNSR_model_By_DMA(unsigned char ai_sr_mode, unsigned short *pArray, unsigned int addr, unsigned int* vir_addr);
short drvif_NNSR_DMA_Err_Check(unsigned char showMSG);
void drvif_NNSR_Err_Reset(void);
unsigned char drvif_Get_NNSR_DMA_Table_Done(unsigned char tbl_idx, unsigned char showMSG);
void drvif_NNSR_Table_Done_Reset(unsigned char tbl_idx);
char drvif_NNSR_Wait_for_DMA_Apply_Done(void);
unsigned short clamp_nnsr(unsigned short output, int max, int min);
void drvif_srnn_weight_blend(unsigned short *pin_arry_a, unsigned short *pin_arry_b, unsigned short *pout_arry, int size, int alpha, int scale ,int int_bits, int fra_bits);
unsigned short drvif_srnn_weight_blend_output_scaler(unsigned short pin_arry_a, unsigned short pin_arry_b, int alpha, int scale ,int int_bits, int fra_bits);
int drvif_srnn_get_ai_sr_mode(void);
// chen 0324
void drvif_srnn_bbbsr_table(DRV_srnn_table *ptr);
//end chen 0324

#endif /* _SRNN_H_ */
