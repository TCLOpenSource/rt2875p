/* 20160608 Still Logo Detect header file*/
#ifndef  __STILLLOGODETECT_H__
#define  __STILLLOGODETECT_H__

#ifdef __cplusplus
extern "C" {
#endif
/* some include about scaler*/
#include <scaler/vipCommon.h>


#if 1//k7 add
#define SLD_Table_Default_NUM 2
#define Local_Gain_Tbl_NUM 1

#define LC_MAX_H_NUM 60
#define LC_MAX_V_NUM 34

typedef enum {
	osd_condition = 0,
	still_condition,
	news_condition,
	noise_condition,
	RGB_box_condition,
	normal_condition,
	num_condition
} DRV_SLD_t;

typedef struct
{
	int  sld_add_comp_dy;
	int  sld_scale_mode;
	int  sld_disable_scale_up;
	int  sld_disable_lpf;
	int  sld_idx_count_period;
	int  sld_idx_input_sel;
	int  sld_window_en;
	int  sld_hpf_type;
	int  sld_global_drop;
	int  sld_blend_en;
	int  sld_debug_mode;
	int  sld_en;
}DRV_SLD_sld_control;


typedef struct
{
	int  sld_width;
	int  sld_height;
}DRV_SLD_sld_size;


typedef struct
{
	int  coef_b;
	int  coef_g;
	int  coef_r;
}DRV_SLD_sld_rgb2y_coef;

typedef struct
{
	int  sld_curve_seg_0;	
	int  sld_curve_seg_1;
	int  sld_curve_seg_2;
	unsigned int  sld_new_curve_gain_0;
	unsigned int  sld_new_curve_gain_1;	
	unsigned int  sld_new_curve_gain_2;
	unsigned int  sld_new_curve_gain_3;
	unsigned int  sld_new_curve_offset_0;
	unsigned int  sld_new_curve_offset_1;	
	unsigned int  sld_new_curve_offset_2;
	unsigned int  sld_new_curve_offset_3;
	
}DRV_SLD_sld_Y_curve_map;

typedef struct
{
	int  sld_idx_sign_2;
	int  sld_idx_step_2;
	int  sld_idx_sign_0;
	int  sld_idx_step_0;
}DRV_SLD_sld_hpf_1;

typedef struct
{
	int  sld_inter_thl_0;
	int  sld_inter_thl_1;
	int  sld_inter_thl_2;
	int  sld_inter_thl_3;
	int  sld_inter_thl_4;
	int  sld_inter_thl_5;
	int  sld_inter_thl_6;
	int  sld_inter_thl_7;
	int  sld_inter_thl_8;
	int  sld_inter_thl_9;
	int  sld_inter_thl_10;
	int  sld_inter_thl_11;
}DRV_SLD_sld_in_logo_thl_0;

typedef struct
{
	int  sld_idx_acc_condition_and;	
	int  sld_hpf_logo_add_thl;
	int  sld_frame_diffy_thl;
	int  sld_y_idx_thl;
	int  sld_max_idx_thl;
}DRV_SLD_sld_acc_0;
		
typedef struct
{
	int  sld_pixel_diff_thl;
	int  sld_hpf_diff_thl;
	int  sld_cnt_thl;
}DRV_SLD_sld_drop;

typedef struct
{
	int  neighbor2_diffy_drop_thl;	
	int  neighbor1_diffy_drop_thl;
	int  neighbor0_diffy_drop_thl;
}DRV_SLD_sld_neighbor_drop_0;

typedef struct
{
	int  neighbor_diffy_notdrop_thl;
}DRV_SLD_sld_neighbor_not_drop;

typedef struct
{
	int  lpf_shift_bit;		
	int  coef_0_0;
	int  coef_0_1;
	int  coef_0_2;
	int  coef_0_3;
	int  coef_0_4;	
	int  coef_1_0;
	int  coef_1_1;
	int  coef_1_2;
	int  coef_1_3;
	int  coef_1_4;	
	int  coef_2_0;
	int  coef_2_1;
	int  coef_2_2;
	int  coef_2_3;
	int  coef_2_4;		
	int  coef_3_0;
	int  coef_3_1;
	int  coef_3_2;
	int  coef_3_3;
	int  coef_3_4;	
	int  coef_4_0;
	int  coef_4_1;
	int  coef_4_2;
	int  coef_4_3;
	int  coef_4_4;	
}DRV_SLD_sld_lpf_0;

typedef struct
{
	int  sld_window_apply_out;
	int  sld_window_pos1_y;
	int  sld_window_pos1_x;	
	int  sld_window_pos0_y;
	int  sld_window_pos0_x;
}DRV_SLD_sld_apply_window_0;

typedef struct
{
	int  max_pre_logo_idx_counter;
	int  near_max_pre_logo_idx_counter;
	int  hpf_line_count;
	
	
	int  blocknum_with_alpha_noise;

	int	 sld_logo_cnt;
	int	 sld_drop_cnt;
         
	int	 sld_max_pre_idx;
	
}DRV_SLD_sld_fw_read_only;

typedef struct
{
	int  fw_pixel_gain;	
	int  fw_idx_ctrl_step;
	int  fw_idx_freeze_for_full_frame;
	int  fw_idx_ctrl_en;
}DRV_SLD_sld_fw_control_0;

typedef struct
{
	int  max_pre_logo_tolerance;
}DRV_SLD_sld_fw_control_2;


typedef struct
{
	int  idx_slow_down_h;
	int  idx_slow_down_l;
	int  idx_slow_down_step;
	
}DRV_SLD_sld_drop_slow_0;

typedef struct
{
	int  big_temporal_diff_up;
	int  big_temporal_diff_low;
	int  temporal_counter_thl;	
	int  temporal_diff;
	
}DRV_SLD_sld_temporal_noise_1;


typedef struct
{
	int  idx_drop_protect_idx_neighbor_tolerance;
	int  idx_drop_protect_idx_thl;	
	int  idx_drop_protect_counter_neighbor_sameidx;
}DRV_SLD_sld_idx_protect_by_neighbor_0;



 typedef struct
{
	int  hpf_diff_thl_for_logo_region;
	int  pixel_diff_thl_for_logo_region;	
	int  in_logo_idx_thl;
}DRV_SLD_sld_idx_not_drop_0;


typedef struct
{
	int  compensate_neighbor_tolerance;
	int  idx_compensate_start_thl;	
	int  idx_compensate_neighbor_counter_thl;
}DRV_SLD_sld_idx_compensate_by_neighbor_0;


typedef struct
{
	int  do_connect_seg_idx_lower_bound;
	int  idx_period_check_once;
}DRV_SLD_sld_temporal_idx_compensate;


typedef struct
{
	int  hpf_in_line_thl;
}DRV_SLD_sld_hpf_in_line;


typedef struct
{
	int  detect_tnoise_en;
	int  check_drop_pattern_start;
	int  patternregion_hpf;
	int  patternregion_y;	
	int  patternedge_hpf;
	int  patternedge_y;	
	int  grouph_diffmaxmin;
	int  diffavg;
}DRV_SLD_sld_noise_pattern_0;


typedef struct
{
	int  sld_new_idx_offset_0;
	int  sld_new_idx_offset_1;	
	int  sld_new_idx_offset_2;
	int  sld_new_idx_offset_3;	
	int  sld_new_idx_gain_0;
	int  sld_new_idx_gain_1;	
	int  sld_new_idx_gain_2;
	int  sld_new_idx_gain_3;	
	int  sld_idx_seg_0;	
	int  sld_idx_seg_1;
	int  sld_idx_seg_2;
}DRV_SLD_sld_idx_map_seg;


typedef struct
{
	int  hist_shift_bit;
	int  hist_idx_thl;
}DRV_SLD_sld_histogram_0;

typedef struct
{
	int  y_lower_bound_hist_0;
	int  y_lower_bound_hist_1;
	int  y_lower_bound_hist_2;
	int  y_lower_bound_hist_3;
	int  y_lower_bound_hist_4;
	int  y_lower_bound_hist_5;
	int  y_lower_bound_hist_6;
	int  y_lower_bound_hist_7;
}DRV_SLD_sld_histogram_1;


typedef struct
{
	int  hist_7;
	int  hist_6;
	int  hist_5;
	int  hist_4;
	int  hist_3;
	int  hist_2;
	int  hist_1;
	int  hist_0;
}DRV_SLD_sld_histogram_ro_0;




typedef struct {
	DRV_SLD_sld_control						  sld_control;
	DRV_SLD_sld_size                          sld_size;
	DRV_SLD_sld_rgb2y_coef                    sld_rgb2y_coef;

	DRV_SLD_sld_Y_curve_map                 sld_Y_curve_map;
	
	DRV_SLD_sld_hpf_1                         sld_hpf_1;
	DRV_SLD_sld_in_logo_thl_0                 sld_in_logo_thl_0;
	DRV_SLD_sld_acc_0                         sld_acc_0;
	DRV_SLD_sld_drop                          sld_drop;
	DRV_SLD_sld_neighbor_drop_0               sld_neighbor_drop_0;
	DRV_SLD_sld_neighbor_not_drop             sld_neighbor_not_drop;

	DRV_SLD_sld_lpf_0                         sld_lpf_0;
	DRV_SLD_sld_apply_window_0                sld_apply_window_0;

	DRV_SLD_sld_fw_control_0                  sld_fw_control_0;
	DRV_SLD_sld_fw_control_2                  sld_fw_control_2;
	DRV_SLD_sld_drop_slow_0                   sld_drop_slow_0;

	DRV_SLD_sld_temporal_noise_1              sld_temporal_noise_1;
	DRV_SLD_sld_idx_protect_by_neighbor_0     sld_idx_protect_by_neighbor_0;
	DRV_SLD_sld_idx_not_drop_0                sld_idx_not_drop_0;
	DRV_SLD_sld_idx_compensate_by_neighbor_0  sld_idx_compensate_by_neighbor_0;
	DRV_SLD_sld_temporal_idx_compensate       sld_temporal_idx_compensate;
	DRV_SLD_sld_hpf_in_line                   sld_hpf_in_line;
	DRV_SLD_sld_noise_pattern_0               sld_noise_pattern_0;
	DRV_SLD_sld_idx_map_seg                   sld_idx_map_seg;
	DRV_SLD_sld_histogram_0                   sld_histogram_0;
	DRV_SLD_sld_histogram_1                   sld_histogram_1;
} DRV_SLD_Table;

#endif




void SLD_k6_HAL(int Y1, int Y2, int Y3, int gain_low,unsigned char UIsel_L);
#if 0 /*SLD, hack, elieli*/ //k5l hw remove
void SLD_k6_HAL(int Y1, int Y2, int Y3, int gain_low,unsigned char UIsel_L);
//void drvif_color_set_SLD_4k_dma_setting(void);
#if 0//k7, delete
void drvif_color_set_SLD_Enable(unsigned char enable);
void drvif_color_set_SLD_control(DRV_SLD_Ctrl *ptr);
void drvif_color_set_SLD_input_size(DRV_SLD_Size *ptr);
void drvif_color_set_SLD_curve_map(DRV_SLD_CurveMap *ptr);
void drvif_color_set_SLD_drop(DRV_SLD_DROP *ptr);
void drvif_color_set_SLD_cnt_thl(unsigned int cnt_th);
#endif
//void fwif_k6_sld(int Y1, int Y2, int Y3, int gain_low,unsigned char UIsel_L, unsigned int cnt_th);
#endif

#if 1
// jimmy 20191127, for SLD
typedef struct _sld_work_struct
{
	unsigned char do_counter_update;
	unsigned char do_reset_counter;
	unsigned char do_reset_full;
	unsigned char LD_APL_valid;
	unsigned char read_ok;
	short osd_sta_blkx;
	short osd_sta_blky;
	short osd_end_blkx;
	short osd_end_blky;
	struct work_struct sld_apply_work;
} sld_work_struct;

typedef struct _sld_work_debug_struct
{
	unsigned short pr_En;
	unsigned short pr_point_x;
	unsigned short pr_point_y;
	unsigned short pr_delay;
	short mark_val;
} sld_work_debug_struct;

#define demura_filter_size 9
#define demura_filter_size_half 4	// demura_filter_size>>1
typedef struct _sld_work_filter_struct
{
	unsigned short filter_w[demura_filter_size*demura_filter_size];
	unsigned short filter_shift;	
	unsigned short filter_gain;
} sld_work_filter_struct;

typedef struct _DRV_sld_condition_struct
{
	unsigned char osd_condition;
	unsigned char still_condition;
	unsigned char news_condition;
	unsigned char noise_condition;
	unsigned char RGB_box_condition;
	unsigned char normal_condition;
} DRV_sld_condition_struct;

typedef struct {
    unsigned short SLD_local_gain_table[576];
} DRV_SLD_local_gain_tbl;

typedef struct _VIP_SLD_DMA_CTRL {
	unsigned int size;
	unsigned int phy_addr_align;
	unsigned int *pVir_addr_align;

} VIP_SLD_DMA_CTRL;

VIP_SLD_DMA_CTRL* drvif_fwif_color_Get_SLD_APL_DMA_CTRL(unsigned char index);
char drvif_fwif_color_Get_SLD_APL(unsigned short *pTBL, unsigned int num);
/* Main Functions */
void memc_logo_to_demura_read(void);
void memc_logo_to_demura_init(void);
/* For RTICE command to debug */
void dump_logo_detect_to_file(unsigned char start_byte);
void dump_sld_calc_buffer_to_file(void);

#endif

void drvif_color_set_SLD_Enable(unsigned char enable);
void drvif_color_set_SLD_global_drop(void);
void drvif_color_set_SLD_disable_global_drop(void);
void drvif_color_set_SLD_control(DRV_SLD_sld_control *ptr);
void drvif_color_set_SLD_window_en(DRV_SLD_sld_control *ptr);
void drvif_color_set_SLD_size(DRV_SLD_sld_size *ptr);
void drvif_color_set_SLD_rgb2y_coef(DRV_SLD_sld_rgb2y_coef *ptr);
void drvif_color_set_SLD_Y_curve_map(DRV_SLD_sld_Y_curve_map *ptr);
void drvif_color_set_SLD_Idx_curve_map(DRV_SLD_sld_idx_map_seg *ptr);
void drvif_color_set_SLD_Idx_add_con(DRV_SLD_sld_acc_0 *ptr);
void drvif_color_set_SLD_8dir_12tab(DRV_SLD_sld_in_logo_thl_0 *ptr);
void drvif_color_set_SLD_lpf(DRV_SLD_sld_lpf_0 *ptr);
void drvif_color_set_SLD_step(DRV_SLD_sld_hpf_1 *ptr);
void drvif_color_set_SLD_drop(DRV_SLD_sld_drop *ptr,DRV_SLD_sld_idx_not_drop_0 *ptr_in_logo, DRV_SLD_sld_neighbor_not_drop *ptr_n_notDrop,DRV_SLD_sld_neighbor_drop_0 *ptr_nDrop, DRV_SLD_sld_idx_protect_by_neighbor_0 *ptr_dropProtect);
void drvif_color_set_SLD_compensate_spatial(DRV_SLD_sld_idx_compensate_by_neighbor_0 *ptr);
void drvif_color_set_SLD_compensate_temporal(DRV_SLD_sld_temporal_idx_compensate *ptr);
void drvif_color_set_SLD_control_minus_step(DRV_SLD_sld_drop_slow_0 *ptr);
void drvif_color_set_SLD_fw_control_step(DRV_SLD_sld_fw_control_0 *ptr);
void drvif_color_set_SLD_fw_control(DRV_SLD_sld_fw_control_0 *ptr);
void drvif_color_set_SLD_sld_temporal_noise(DRV_SLD_sld_temporal_noise_1 *ptr);
void drvif_color_set_SLD_sld_window(DRV_SLD_sld_apply_window_0 *ptr);
void drvif_color_set_SLD_sld_histogram(DRV_SLD_sld_histogram_1 *ptr, DRV_SLD_sld_histogram_0 *ptr_0);
void drvif_color_set_SLD_LGD_Tnoise_pattern(DRV_SLD_sld_noise_pattern_0 *ptr);
void drvif_color_set_SLD_hpf_in_line_thl(DRV_SLD_sld_hpf_in_line *ptr);
void drvif_color_set_SLD_max_pre_logo_tolerance(DRV_SLD_sld_fw_control_2 *ptr);
void drvif_color_SLD_LPF_ur(void);
void drvif_color_get_SLD_max_pre_idx(unsigned int* sld_max_pre_idx);
void drvif_color_get_SLD_fw_read_only(DRV_SLD_sld_fw_read_only *ptr);
void drvif_color_get_histogram_bin(DRV_SLD_sld_histogram_ro_0 *ptr);
int fwif_color_sld(void);
char drvif_color_set_SLD_CTRL(unsigned int phy_addr, unsigned int add_size, unsigned char scale_mode,int panel_width, int panel_height);
void drif_color_SLD_apply_normal_setting(void);

unsigned char drvif_color_set_SLD_LCblk_inform(void);
unsigned char drvif_color_set_SLD_LCblk_inform(void);
void drvif_SLD_table_set(unsigned char tbl_sel, DRV_SLD_local_gain_tbl *ptr_tbl);
void drvif_SLD_table_get(unsigned char tbl_sel, unsigned short *ptr_tbl);
void drvif_SLD_table_enable(unsigned char tbl_sel, unsigned char enable);
char drvif_SLD_Block_APL_Read_by_DMA_ISR(void/*unsigned short *pout_tbl*/);
void drvif_SLD_Block_APL_init(unsigned int p_addr0, unsigned int p_addr1);

#ifdef __cplusplus
}
#endif

#endif // __LOCALCONTRAST_H__


