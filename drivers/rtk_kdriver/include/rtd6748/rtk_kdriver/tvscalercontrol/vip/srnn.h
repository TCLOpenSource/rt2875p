#ifndef _SRNN_H_
#define _SRNN_H_

#include <scaler/vipCommon.h>

#if 0 //moved to vipCommon
/*******************************************************************************
*Definitions*
******************************************************************************/
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
#define SRNN_Ctrl_Table_MAX 6
#define SRNN_Weight_Num_mode2 7380
#define SRNN_Weight_Num_mode3 14364
#define SRNN_Weight_Num_mode4 14364
#define SRNN_Weight_Num_mode5 14729
#define SRNN_Weight_Num_mode6 28356
#define SRNN_Weight_Num_mode7 29232
#define SRNN_Weight_Layers 16


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

// chen 210312

typedef struct {
    unsigned char bbbsr_layers_blend_en;             
    unsigned char bbbsr_layers_blend_weight_hi;			
    unsigned char bbbsr_layers_blend_weight_lo; 		 
    unsigned char bbbsr_dir_confid_len_blend; 		
} DRV_srnn_two_layers_blend_ctrl;

typedef struct {
    unsigned char bbbsr_b1_weight_hi;						
    unsigned char bbbsr_b1_weight_lo;						
    unsigned char bbbsr_b1_slope;								
    unsigned char bbbsr_b1_maxmin_loth;					
    
    unsigned char bbbsr_b1_condition_select; 
} DRV_srnn_two_layers_blend1;

typedef struct {
    unsigned char bbbsr_b2_weight_hi;				
    unsigned char bbbsr_b2_weight_lo;				
    unsigned char bbbsr_b2_slope;						
    unsigned char bbbsr_b2_dir_loth;				
} DRV_srnn_two_layers_blend2;

typedef struct {
    unsigned char bbbsr_b3_weight_hi;				
    unsigned char bbbsr_b3_weight_lo;				 
    unsigned char bbbsr_b3_slope;				 
    unsigned char bbbsr_b3_max_clipth;				  
    
    unsigned char bbbsr_b3_min_loth;	 
} DRV_srnn_two_layers_blend3;

typedef struct {
    unsigned char bbbsr_b4_weight_hi;       
    unsigned char bbbsr_b4_weight_lo;       
    unsigned char bbbsr_b4_slope;        
    unsigned char bbbsr_b4_var_loth;       
    
    unsigned char bbbsr_b4_var_select;  
} DRV_srnn_two_layers_blend4;

//end chen 210312

typedef struct {
    DRV_srnn_ctrl 							srnn_ctrl;                     // 6 
    DRV_srnn_diff_clip1 				srnn_diff_clip1;               // 7
    DRV_srnn_diff_clip2 				srnn_diff_clip2;               // 3
    DRV_srnn_diff_clip3 				srnn_diff_clip3;               // 12
    DRV_srnn_weight 						srnn_weight;                   // 4
		DRV_srnn_flat_gradient 			srnn_flat_gradient;            // 6
		DRV_srnn_delta_y_control 		srnn_delta_y_control;          // 3
		
		// chen 210312
		DRV_srnn_two_layers_blend_ctrl	srnn_two_layers_blend_ctrl;	// 4
		DRV_srnn_two_layers_blend1			srnn_two_layers_blend1;			// 5
		DRV_srnn_two_layers_blend2			srnn_two_layers_blend2;			// 4		
		DRV_srnn_two_layers_blend3			srnn_two_layers_blend3;			// 5
		DRV_srnn_two_layers_blend4			srnn_two_layers_blend4;			// 5		
		// end chen 210312

} DRV_srnn_table;
/*
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
*/
#endif
/*******************************************************************************
*functions*
******************************************************************************/

void drvif_srnn_clk_enable(unsigned char clk_srnn_en);
											   
													   
unsigned char drvif_srnn_bbbsr_enable_get(void);
//unsigned char drvif_srnn_bbbsr_enable_get(void);
void drvif_srnn_bbbsr_enable_set(unsigned char bbbsr_en);
//void drvif_srnn_func_enable_set(unsigned char srnn_en);
void drvif_srnn_model_set(unsigned short *para, unsigned char ai_sr_mode);
void drvif_srnn_model_get(unsigned short *para, unsigned int ai_sr_mode);
void drvif_color_Set_NNSR_model_By_DMA(unsigned char NNSR_Ctrl, unsigned short *pArray, unsigned int addr, unsigned int* vir_addr);
short drvif_NNSR_DMA_Err_Check(unsigned char showMSG);
unsigned char drvif_Get_NNSR_DMA_Table_Done(unsigned char tbl_idx, unsigned char showMSG);
void drvif_NNSR_Table_Done_Reset(unsigned char tbl_idx);
void drvif_NNSR_Err_Reset(void);
char drvif_NNSR_Wait_for_DMA_Apply_Done(void);
void drvif_srnn_test_set(unsigned char a);
unsigned short drvif_srnn_weight_blend_output_scaler(unsigned short pin_arry_a, unsigned short pin_arry_b, int alpha, int scale ,int int_bits, int fra_bits);
// chen 0324
void drvif_srnn_bbbsr_table(DRV_srnn_table *ptr);
//end chen 0324

//cy_kuan 0202


#endif /* _SRNN_H_ */

