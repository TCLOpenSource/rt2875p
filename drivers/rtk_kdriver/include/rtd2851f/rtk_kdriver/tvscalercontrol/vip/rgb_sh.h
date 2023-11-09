#ifndef _VIP_LIB_RGB_SHPLIB_H_
#define _VIP_LIB_RGB_SHPLIB_H_

/* ----- ----- ----- ----- ----- HEADER ----- ----- ----- ----- ----- */

/* ----- ----- ----- ----- ----- STRUCTURE ----- ----- ----- ----- ----- */
typedef struct {
	unsigned char rgb_sharp_enable;
	unsigned char rgb_sharp_yuvmode;
	unsigned short rgb2y_r;
	unsigned short rgb2y_g;
	unsigned short rgb2y_b;
	unsigned char rgblimit_pos;
	unsigned char rgblimit_neg;
} VIP_RGB_SHP_rgb2y;

typedef struct {
	signed short hpf_c_0;
	signed short hpf_c_1;
	signed short hpf_c_2;
} VIP_RGB_SHP_hpf5;

typedef struct {
	unsigned char d2_lv;
	unsigned char d2_gain_pos;
	unsigned char d2_gain_neg;
	unsigned short d2_hv_pos;
	unsigned short d2_hv_neg;
	unsigned char d2_gpos2;
	unsigned char d2_gneg2;
	unsigned char d2_lv2;
} VIP_RGB_SHP_getd2v;

typedef struct {
	unsigned char emf_enable;
	unsigned char emf_range_h;
	unsigned char emf_range_v;
	unsigned char emf_weak;
	signed short emf_offset_0;
	signed char emf_gain_0;
	unsigned short emf_seg_x_0;
	signed short emf_offset_1;
	signed char emf_gain_1;
	unsigned short emf_seg_x_1;
	signed short emf_offset_2;
	signed char emf_gain_2;
} VIP_RGB_SHP_emf;

typedef struct {
	VIP_RGB_SHP_rgb2y rgb2y;
	VIP_RGB_SHP_hpf5 hpf5;
	VIP_RGB_SHP_getd2v getd2v;
	VIP_RGB_SHP_emf emf;
} VIP_RGB_SHP_LEVEL_TABLE;

/* ----- ----- ----- ----- ----- FUNCTION ----- ----- ----- ----- ----- */

void VPQ_RgbShp_RegCtrl_SetRgb_shp_reg_db_apply(void);
void VPQ_RgbShp_RegCtrl_SetRgb_shp_reg_table(VIP_RGB_SHP_LEVEL_TABLE* x);
void VPQ_RgbShp_Lib_SetRgb_shp_PqLevel(unsigned char gain);
void VPQ_RgbShp_Lib_SetRgb_shp_Default(void);
void VPQ_RgbShp_Lib_SetRgb_shp_byGain(unsigned char x_table,int xfenzi,int xfenmu);


#endif
