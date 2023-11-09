#ifndef _VIP_LIB_CBREVLIB_H_
#define _VIP_LIB_CBREVLIB_H_

/* ----- ----- ----- ----- ----- HEADER ----- ----- ----- ----- ----- */

/* ----- ----- ----- ----- ----- STRUCTURE ----- ----- ----- ----- ----- */
typedef struct {
	unsigned char cbrev_enable;
	unsigned char cbrev_mode;
	unsigned char cbrev_lr_bit_y;
	unsigned char cbrev_lr_biu_y;
	unsigned char cbrev_cccoring;
	unsigned char cbrev_gain;
	unsigned char cbrev_shape_adj_F5_gain;
	unsigned char cbrev_gain_neg;
	unsigned char cbrev_localmax_method;
	unsigned char cbrev_weak;
} VIP_CBREV_LEVEL_ROW0;

typedef struct {
	unsigned short cbrev_force_max_y_ub;
	unsigned short cbrev_force_max_y_lb;
	unsigned short cbrev_adapt_y_ub;
	unsigned short cbrev_adapt_y_lb;
} VIP_CBREV_LEVEL_ROW1;

typedef struct {
	unsigned char cbrev_flat_en;
	unsigned char cbrev_flat_chroma;
	unsigned char cbrev_flat_chroma_th;
	unsigned char cbrev_flat_chroma_luma;
	unsigned char cbrev_flat_gain;
	signed char cbrev_flat_offset;
} VIP_CBREV_LEVEL_FLAT;

typedef struct {
	VIP_CBREV_LEVEL_ROW0 row0;
	VIP_CBREV_LEVEL_ROW1 row1;
	VIP_CBREV_LEVEL_FLAT flat;
} VIP_CBREV_LEVEL_TABLE;

/* ----- ----- ----- ----- ----- FUNCTION ----- ----- ----- ----- ----- */

void VPQ_Cbrev_RegCtrl_SetCbrev_reg_db_apply(void);
void VPQ_Cbrev_RegCtrl_SetCbrev_reg_table(VIP_CBREV_LEVEL_TABLE* x);
void VPQ_Cbrev_Lib_SetCbrev_PqLevel(unsigned char gain);
void VPQ_Cbrev_Lib_SetCbrev_Default(void);

#endif
