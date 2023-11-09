#ifndef _FILM_H_
#define _FILM_H_

/*============================ Module dependency  ===========================*/
/*#include "rtd_types.h"*/
/*================================  definitions ===============================*/

typedef struct {
	unsigned char film_status;
} DRV_film_mode;

typedef struct {
	unsigned char film_fw_en;
	unsigned char film_sawtooth_choose;
	unsigned char film_LeaveQuick_en;
	unsigned char film_VerScrollingText_en;
	unsigned char film_thumbnail_en;
	unsigned char film_pair1_en;
	unsigned char film_pair2_en;
	unsigned char film_pair_en_auto;
	unsigned char film_violate_strict_en;
	unsigned char film_PairRatio_all1;
	unsigned char film_PairRatio_all2;
	unsigned char film_PairRatio_sub1;
	unsigned char film_PairRatio_sub2;
	unsigned char film_SawtoothThl;
	unsigned char film_FrMotionThl;
	unsigned char film_FieldMotionSumThl;
	unsigned char film_SubFieldMotionSumThl;
	unsigned char film_FrStaticSThl;
	unsigned char film_SubFrStaticsSTh1;
	unsigned char film_FiStaticSThl;
	unsigned char film_SubFiStaticsSTh1;
	unsigned char film_FiMinSThl;
	unsigned char film_SubFiMinSTh1;
	unsigned short film_FiMaxSThl;
	unsigned short film_SubFiMaxSTh1;
	unsigned char film_film32_debounce_fieldnum;
	unsigned char film_film22_debounce_fieldnum;
	unsigned char film_film22_SawtoothTh1;
	unsigned char film_film22_FiStaticSThl;
	unsigned char film_film22_FieldMotionSumThl;
	unsigned char film_film22_SubFieldMotionSumThl;
} DRV_film_table_t;

typedef struct {
	unsigned char film_fw_en;
	unsigned char film_FrMotionThl;
	unsigned char film_bothdiff_range;
	unsigned char film_bothdiff_weight;
	unsigned char film_bothdiff_shift;
} DRV_film_row_px_frame;

typedef struct {
	unsigned char film_teethth_teeth_lb;// Merlin4
	unsigned char film_teethth_moving_lb;// Merlin4
	unsigned char film_teethth_prog_lb;// Merlin4
	unsigned char film_teethth_prog_ub;// Merlin5
	unsigned char film_teethth_en;// Merlin4
	unsigned char film_andor;
	unsigned char film_sawtooth_choose;// 1:lowpass 2:6diff
	unsigned char film_SawtoothThl;
	unsigned char film_film22_SawtoothTh1;// 6diff Thl
	unsigned char film_V_edge_pass;
	unsigned char film_R_edge_pass;
	unsigned char film_H_edge_pass;
	unsigned char film_L_edge_pass;
	unsigned char film_N_edge_pass;
	unsigned char film_SobelTHs;
	unsigned char film_SobelTHm;
	unsigned char film_mask_en;
	unsigned char film_mask_pass_count;
	unsigned char film_mask_FildDiff_thd;
	unsigned char film_mask_FrmDiff_thd;
	unsigned char film_teethth_moving_single_side;// Merlin5
	unsigned char film_teethth_timing;
	unsigned char film_teethth_teeth_both_en;
} DRV_film_row_px_field;

typedef struct {
	unsigned char film_FrStaticSThl;
	unsigned char film_FiStaticSThl;
	unsigned char film_FieldMotionSumThl;
	unsigned char film_FiMinSThl;
	unsigned short film_FiMaxSThl;
	unsigned char film_PairRatio_all1;
	unsigned char film_PairRatio_all2;
	unsigned char film_film22_FieldMotionSumThl;
	unsigned char film_film22_FiStaticSThl;
} DRV_film_row_th_all;

typedef struct {
	unsigned char film_SubFrStaticsSTh1;
	unsigned char film_SubFiStaticsSTh1;
	unsigned char film_SubFieldMotionSumThl;
	unsigned char film_SubFiMinSTh1;
	unsigned short film_SubFiMaxSTh1;
	unsigned char film_PairRatio_sub1;
	unsigned char film_PairRatio_sub2;
	unsigned char Film22_SubFieldMotionSumThl;
	unsigned char Film22_SubFiStaticSThl;
} DRV_film_row_th_sub;

typedef struct {
	unsigned char film_teethth_ratio;// Merlin4
	unsigned char film_pair_en_auto;
	unsigned char film_pair1_en;
	unsigned char film_pair2_en;
} DRV_film_row_pair;

typedef struct {
	unsigned char film_LeaveQuick_en;
	unsigned char film_VerScrollingText_en;
	unsigned char film_thumbnail_en;
	unsigned char film_violate_strict_en;
	unsigned char film_film32_debounce_fieldnum;
	unsigned char film_film22_debounce_fieldnum;
	unsigned char film_framestaticonly;// Merlin5
} DRV_film_row_frame;

typedef struct {
	unsigned char di_film_nonfilmregion_force;
	signed char di_film_nonfilmregion_s_big;
	signed char di_film_nonfilmregion_s_small;
	signed char di_film_nonfilmregion_s_both;
	signed char di_film_nonfilmregion_th;
	unsigned char di_film_nonfilmregion_up_debounce;
	unsigned char di_film_nonfilmregion_up_extend;
	unsigned char di_film_nonfilmregion_dn_debounce;
	unsigned char di_film_nonfilmregion_dn_extend;
} DRV_film_row_nonfilm;

typedef struct {
	DRV_film_row_px_frame film_row_px_frame;
	DRV_film_row_px_field film_row_px_field;
	DRV_film_row_th_all film_row_th_all;
	DRV_film_row_th_sub film_row_th_sub;
	DRV_film_row_pair film_row_pair;
	DRV_film_row_frame film_row_frame;
	DRV_film_row_nonfilm film_row_nonfilm;
} DRV_film_table_t2;

/*================================== Variables ==============================*/

/*================================== functions ==============================*/
void drvif_module_film_mode(DRV_film_mode *ptr);
void drvif_module_film_setting(DRV_film_table_t2 *ptr);
void drvif_module_film_setting_tv006(DRV_film_table_t *ptr);

void drvif_module_film_read_result(void);

#endif /* _FILM_H_ */
