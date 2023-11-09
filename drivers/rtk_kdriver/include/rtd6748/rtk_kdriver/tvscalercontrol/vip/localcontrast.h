/* 20140523 Local Contrast header file*/
#ifndef  __LOCALCONTRAST_H__
#define  __LOCALCONTRAST_H__

#ifdef __cplusplus
extern "C" {
#endif
/* some include about scaler*/
#include <scaler/vipCommon.h>
#include <scaler/vipRPCCommon.h>

//extern unsigned int SLC_Curve[256][33];

#define LC_GSR_REPORT_H_BLKS 60
#define LC_GSR_REPORT_V_BLKS 34

#define CDLC_Table_NUM 1
#define LC_overlay_Table_NUM 1
#define LC_ToneMapping_Yavg_Yin_Blend_Table_NUM 1
#define LC_ToneMapping_PQmask_Table_NUM 1
#define LC_Sharpness_PQmask_Table_NUM 1

#if 0 //move to vipCommon.h

#define LC_BL_Profile_Table_NUM 6
#define LC_BL_Profile_Table_ROW 4
#define LC_BL_Profile_Table_COLUMN 20
#define LC_Table_NUM 10
#define LC_De_Cont_Level 4 //off,low,middle,high
#define SLC_Table_NUM 1

//juwen, add LC : tone mapping curve
#define LC_Curve_ToneM_PointSlope 256

typedef struct {
	unsigned char lc_local_sharp_en;
	unsigned char lc_tone_mapping_en;
	unsigned char lc_hist_mode;
	unsigned char lc_tenable;
	unsigned char lc_valid;
} DRV_LC_Global_Ctrl;


typedef struct {
	unsigned char lc_tmp_pos0thd;
	unsigned char lc_tmp_pos1thd;
	unsigned char lc_tmp_posmingain;
	unsigned char lc_tmp_posmaxgain;
	unsigned char lc_tmp_neg0thd;
	unsigned char lc_tmp_neg1thd;
	unsigned char lc_tmp_negmingain;
	unsigned char lc_tmp_negmaxgain;
	unsigned char lc_tmp_maxdiff;
	unsigned char lc_tmp_scenechangegain1;
} DRV_LC_Temporal_Filter;


typedef struct {
	unsigned char lc_tab_hsize;
	unsigned char lc_hboundary;
	unsigned char lc_hinitphase1;
	unsigned char lc_hinitphase2;
	unsigned char lc_hinitphase3_left;
	unsigned char lc_tab_vsize;
	unsigned char lc_vboundary;
	unsigned char lc_vinitphase1;
	unsigned char lc_vinitphase2;
	unsigned char lc_vinitphase3_left;
	unsigned char lc_blight_update_en;
	unsigned char lc_blight_sw_mode;
	unsigned char lc_table_sw_mode;

} DRV_LC_Backlight_Profile_Interpolation;

typedef struct {
	unsigned char lc_demo_en;
	unsigned char lc_demo_mode;
	unsigned short lc_demo_top;
	unsigned short lc_demo_bottom;
	unsigned short lc_demo_left;
	unsigned short lc_demo_right;
} DRV_LC_Demo_Window;

typedef struct {
	unsigned short lc_tmap_g0_th1;
	unsigned short lc_tmap_g0_th2;
	unsigned short lc_tmap_g0_th3;
	unsigned short lc_tmap_g0_th4;
	unsigned short lc_tmap_g0_th5;
	unsigned short lc_tmap_g0_th6;
	unsigned short lc_tmap_g0_th7;
	unsigned short lc_tmap_g0_th8;
	unsigned short lc_tmap_g0_th9;
	unsigned short lc_tmap_g0_th10;
	unsigned short lc_tmap_g0_th11;
	unsigned short lc_tmap_g0_th12;
	unsigned short lc_tmap_g0_th13;
	unsigned short lc_tmap_g0_th14;
	unsigned short lc_tmap_g0_th15;
	unsigned short lc_tmap_g0_th16;
	unsigned short lc_tmap_g0_th17;
	unsigned short lc_tmap_g0_th18;
	unsigned short lc_tmap_g0_th19;
	unsigned short lc_tmap_g0_th20;
	unsigned short lc_tmap_g0_th21;
	unsigned short lc_tmap_g0_th22;
	unsigned short lc_tmap_g0_th23;
	unsigned short lc_tmap_g0_th24;
	unsigned short lc_tmap_g0_th25;
	unsigned short lc_tmap_g0_th26;
	unsigned short lc_tmap_g0_th27;
	unsigned short lc_tmap_g0_th28;
	unsigned short lc_tmap_g0_th29;
	unsigned short lc_tmap_g0_th30;
	unsigned short lc_tmap_g0_th31;
} DRV_LC_ToneMapping_Grid0;

typedef struct {
	unsigned short lc_tmap_g2_th1;
	unsigned short lc_tmap_g2_th2;
	unsigned short lc_tmap_g2_th3;
	unsigned short lc_tmap_g2_th4;
	unsigned short lc_tmap_g2_th5;
	unsigned short lc_tmap_g2_th6;
	unsigned short lc_tmap_g2_th7;
	unsigned short lc_tmap_g2_th8;
	unsigned short lc_tmap_g2_th9;
	unsigned short lc_tmap_g2_th10;
	unsigned short lc_tmap_g2_th11;
	unsigned short lc_tmap_g2_th12;
	unsigned short lc_tmap_g2_th13;
	unsigned short lc_tmap_g2_th14;
	unsigned short lc_tmap_g2_th15;
	unsigned short lc_tmap_g2_th16;
	unsigned short lc_tmap_g2_th17;
	unsigned short lc_tmap_g2_th18;
	unsigned short lc_tmap_g2_th19;
	unsigned short lc_tmap_g2_th20;
	unsigned short lc_tmap_g2_th21;
	unsigned short lc_tmap_g2_th22;
	unsigned short lc_tmap_g2_th23;
	unsigned short lc_tmap_g2_th24;
	unsigned short lc_tmap_g2_th25;
	unsigned short lc_tmap_g2_th26;
	unsigned short lc_tmap_g2_th27;
	unsigned short lc_tmap_g2_th28;
	unsigned short lc_tmap_g2_th29;
	unsigned short lc_tmap_g2_th30;
	unsigned short lc_tmap_g2_th31;
} DRV_LC_ToneMapping_Grid2;

typedef struct {
	unsigned char lc_tmap_curve0_grid;
	unsigned short lc_tmap_curve0_tone;
	unsigned char lc_tmap_curve1_grid;
	unsigned short lc_tmap_curve1_tone;
	unsigned char lc_tmap_curve2_grid;
	unsigned short lc_tmap_curve2_tone;
	unsigned char lc_tmap_curve3_grid;
	unsigned short lc_tmap_curve3_tone;
	unsigned char lc_tmap_curve4_grid;
	unsigned short lc_tmap_curve4_tone;
	unsigned char lc_tmap_curve5_grid;
	unsigned short lc_tmap_curve5_tone;
	unsigned char lc_tmap_curve6_grid;
	unsigned short lc_tmap_curve6_tone;
	unsigned char lc_tmap_curve7_grid;
	unsigned short lc_tmap_curve7_tone;
} DRV_LC_ToneMapping_CurveSelect;


typedef struct {
	unsigned char lc_tmap_slope_unit;
	unsigned char lc_tmap_blend_factor;
} DRV_LC_ToneMapping_Blend;

typedef struct {
	unsigned char lc_gain_by_yin_0;
	unsigned char lc_gain_by_yin_1;
	unsigned char lc_gain_by_yin_2;
	unsigned char lc_gain_by_yin_3;
	unsigned char lc_gain_by_yin_4;
	unsigned char lc_gain_by_yin_5;
	unsigned char lc_gain_by_yin_6;
	unsigned char lc_gain_by_yin_7;
	unsigned char lc_gain_by_yin_8;
	unsigned char lc_gain_by_yin_9;
	unsigned char lc_gain_by_yin_10;
	unsigned char lc_gain_by_yin_11;
	unsigned char lc_gain_by_yin_12;
	unsigned char lc_gain_by_yin_13;
	unsigned char lc_gain_by_yin_14;
	unsigned char lc_gain_by_yin_15;
	unsigned char lc_gain_by_yin_16;
	unsigned char lc_gain_by_yin_17;
	unsigned char lc_gain_by_yin_18;
	unsigned char lc_gain_by_yin_19;
	unsigned char lc_gain_by_yin_20;
	unsigned char lc_gain_by_yin_21;
	unsigned char lc_gain_by_yin_22;
	unsigned char lc_gain_by_yin_23;
	unsigned char lc_gain_by_yin_24;
	unsigned char lc_gain_by_yin_25;
	unsigned char lc_gain_by_yin_26;
	unsigned char lc_gain_by_yin_27;
	unsigned char lc_gain_by_yin_28;
	unsigned char lc_gain_by_yin_29;
	unsigned char lc_gain_by_yin_30;
	unsigned char lc_gain_by_yin_31;
	unsigned char lc_gain_by_yin_32;
	unsigned char lc_gain_by_yin_divisor;
} DRV_LC_Shpnr_Gain1st;

typedef struct {
	unsigned char lc_gain_by_ydiff_0;
	unsigned char lc_gain_by_ydiff_1;
	unsigned char lc_gain_by_ydiff_2;
	unsigned char lc_gain_by_ydiff_3;
	unsigned char lc_gain_by_ydiff_4;
	unsigned char lc_gain_by_ydiff_5;
	unsigned char lc_gain_by_ydiff_6;
	unsigned char lc_gain_by_ydiff_7;
	unsigned char lc_gain_by_ydiff_8;
	unsigned char lc_gain_by_ydiff_9;
	unsigned char lc_gain_by_ydiff_10;
	unsigned char lc_gain_by_ydiff_11;
	unsigned char lc_gain_by_ydiff_12;
	unsigned char lc_gain_by_ydiff_13;
	unsigned char lc_gain_by_ydiff_14;
	unsigned char lc_gain_by_ydiff_15;
	unsigned char lc_gain_by_ydiff_16;
	unsigned char lc_gain_by_ydiff_17;
	unsigned char lc_gain_by_ydiff_18;
	unsigned char lc_gain_by_ydiff_19;
	unsigned char lc_gain_by_ydiff_20;
	unsigned char lc_gain_by_ydiff_21;
	unsigned char lc_gain_by_ydiff_22;
	unsigned char lc_gain_by_ydiff_23;
	unsigned char lc_gain_by_ydiff_24;
	unsigned char lc_gain_by_ydiff_25;
	unsigned char lc_gain_by_ydiff_26;
	unsigned char lc_gain_by_ydiff_27;
	unsigned char lc_gain_by_ydiff_28;
	unsigned char lc_gain_by_ydiff_29;
	unsigned char lc_gain_by_ydiff_30;
	unsigned char lc_gain_by_ydiff_31;
	unsigned char lc_gain_by_ydiff_32;
	unsigned char lc_gain_by_ydiff_divisor;
} DRV_LC_Shpnr_Gain2nd;

typedef struct {
	unsigned char lc_ydiff_measure_en;
	unsigned short lc_ydiff_abs_th;

} DRV_LC_Diff_Ctrl0;

#if 1//juwen, k5lp
typedef struct {
	int LC_saturation_rgb2yuv_m11;
	int LC_saturation_rgb2yuv_m12;
	int LC_saturation_rgb2yuv_m13;
	int LC_saturation_rgb2yuv_m21;
	int LC_saturation_rgb2yuv_m22;
	int LC_saturation_rgb2yuv_m23;
	int LC_saturation_rgb2yuv_m31;
	int LC_saturation_rgb2yuv_m32;
	int LC_saturation_rgb2yuv_m33;
} DRV_LC_saturation_r2y;

typedef struct {
	int LC_saturation_yuv2rgb_k11;
	int LC_saturation_yuv2rgb_k12;
	int LC_saturation_yuv2rgb_k13;

	int LC_saturation_yuv2rgb_k22;
	int LC_saturation_yuv2rgb_k23;
	int LC_saturation_yuv2rgb_k32;
	int LC_saturation_yuv2rgb_k33;
} DRV_LC_saturation_y2r;


typedef struct {
	DRV_LC_saturation_r2y LC_saturation_r2y;
	DRV_LC_saturation_y2r LC_saturation_y2r;
	unsigned char LC_saturation_en;
	unsigned int LC_no_saturation_enhance_thl;
	unsigned short LC_saturation_blend_factor;
	int LC_saturation_Gain;
} DRV_LC_saturation;


//juwen, k5lp
typedef struct {
	unsigned int lc_tmap_blend_factor_thl_0;
	unsigned int lc_tmap_blend_factor_thl_1;
	unsigned int lc_tmap_blend_factor_thl_2;
	unsigned int lc_tmap_blend_factor_thl_3;
	unsigned int lc_tmap_blend_factor_thl_4;
	unsigned int lc_tmap_blend_factor_thl_5;
	unsigned int lc_tmap_blend_factor_thl_6;
	unsigned int lc_tmap_blend_factor_thl_7;

	unsigned char lc_tmap_blend_factor_ratio_0;
	unsigned char lc_tmap_blend_factor_ratio_1;
	unsigned char lc_tmap_blend_factor_ratio_2;
	unsigned char lc_tmap_blend_factor_ratio_3;
	unsigned char lc_tmap_blend_factor_ratio_4;
	unsigned char lc_tmap_blend_factor_ratio_5;
	unsigned char lc_tmap_blend_factor_ratio_6;
	unsigned char lc_tmap_blend_factor_ratio_7;
	unsigned char lc_tmap_blend_factor_ratio_8;
} DRV_LC_avoid_flicker_ratio_thl;

//juwen, k5lp
typedef struct {
	unsigned short lc_tmap_diff_thl_0;
	unsigned short lc_tmap_diff_thl_1;
	unsigned short lc_tmap_diff_thl_2;
	unsigned short lc_tmap_diff_thl_3;
	unsigned short lc_tmap_diff_thl_4;
	unsigned short lc_tmap_diff_thl_5;
	unsigned short lc_tmap_diff_thl_6;
	unsigned short lc_tmap_diff_thl_7;
	unsigned short lc_tmap_diff_thl_8;
	unsigned short lc_tmap_diff_thl_9;
	unsigned short lc_tmap_diff_thl_10;
	unsigned short lc_tmap_diff_thl_11;
	unsigned short lc_tmap_diff_thl_12;
	unsigned short lc_tmap_diff_thl_13;
	unsigned short lc_tmap_diff_thl_14;
	unsigned short lc_tmap_diff_thl_15;
	unsigned short lc_tmap_diff_thl_16;
} DRV_LC_avoid_flicker_diff_thl;


//juwen, k5lp
typedef struct {
	DRV_LC_avoid_flicker_ratio_thl LC_avoid_flicker_ratio_thl;
	DRV_LC_avoid_flicker_diff_thl LC_avoid_flicker_diff_thl;

	unsigned char lc_tmap_avoid_flicker_en;
	unsigned int lc_tmap_diff_counter;
	unsigned int lc_tmap_slow_add_thl;
} DRV_LC_avoid_flicker;

// jimmy, LC decontour, k5lp
typedef struct {
	unsigned char LC_decont_en;
	unsigned char LC_decont_shift;
	unsigned short LC_decont_alpha_0;
	unsigned short LC_decont_alpha_1;
	unsigned short LC_decont_alpha_2;
	unsigned short LC_decont_alpha_3;
	unsigned short LC_decont_alpha_4;
	unsigned short LC_decont_alpha_5;
	unsigned short LC_decont_alpha_6;
	unsigned short LC_decont_alpha_7;
	unsigned short LC_decont_alpha_8;
	unsigned short LC_decont_alpha_9;
	unsigned short LC_decont_alpha_10;
	unsigned short LC_decont_alpha_11;
	unsigned short LC_decont_alpha_12;
	unsigned short LC_decont_alpha_13;
	unsigned short LC_decont_alpha_14;
	unsigned short LC_decont_alpha_15;
} DRV_LC_decontour;


#endif

typedef struct {
	DRV_LC_Global_Ctrl LC_Global_Ctrl;
	DRV_LC_Temporal_Filter LC_Temporal_Filter;
	DRV_LC_Backlight_Profile_Interpolation LC_Backlight_Profile_Interpolation;
	DRV_LC_Demo_Window LC_Demo_Window;
	DRV_LC_ToneMapping_Grid0 LC_ToneMapping_Grid0;
	DRV_LC_ToneMapping_Grid2 LC_ToneMapping_Grid2;
	DRV_LC_ToneMapping_CurveSelect LC_ToneMapping_CurveSelect;
	DRV_LC_ToneMapping_Blend LC_ToneMapping_Blend;
	DRV_LC_Shpnr_Gain1st LC_Shpnr_Gain1st;
	DRV_LC_Shpnr_Gain2nd LC_Shpnr_Gain2nd;
	DRV_LC_Diff_Ctrl0 LC_Diff_Ctrl0;
} DRV_Local_Contrast_Table;

//juwen, 1028, set blk num/size and factor
typedef struct {
	unsigned char nBlk_Hnum;
	unsigned char nBlk_Vnum;
	unsigned short  nBlk_Hsize;
	unsigned short  nBlk_Vsize;
	unsigned int lc_hfactor;
	unsigned int lc_vfactor;
} LC_Region_Num_Size;

#endif

/* LOCAL CONTRAST */
void drvif_color_set_LC_Enable(unsigned char enable);
void drvif_color_set_LC_Global_Ctrl(DRV_LC_Global_Ctrl *ptr);
void drvif_color_set_LC_Temporal_Filter(DRV_LC_Temporal_Filter *ptr);
void drvif_color_set_LC_Backlight_Profile_Interpolation(DRV_LC_Backlight_Profile_Interpolation *ptr);
//void drvif_color_set_LC_BL_Profile_Interpolation_Table(unsigned int *ptr);
void drvif_color_set_LC_BL_Profile_Interpolation_Table(unsigned short *ptr,unsigned char mode,unsigned char tab_size);

void drvif_color_set_LC_Demo_Window(DRV_LC_Demo_Window *ptr);
void drvif_color_set_LC_Backlight_Profile_Interpolation_Table(int ptr[6][4][20]);

void drvif_color_set_LC_ToneMapping_Grid0(DRV_LC_ToneMapping_Grid0 *ptr);
void drvif_color_set_LC_ToneMapping_Grid2(DRV_LC_ToneMapping_Grid2 *ptr);

void drvif_color_set_LC_ToneMapping_CurveSelect(DRV_LC_ToneMapping_CurveSelect *ptr);
void drvif_color_set_LC_ToneMapping_Blend(DRV_LC_ToneMapping_Blend *ptr);
void drvif_color_get_LC_ToneMapping_Blend(DRV_LC_ToneMapping_Blend *ptr);

void drvif_color_set_LC_Shpnr_Gain1st(DRV_LC_Shpnr_Gain1st *ptr);
void drvif_color_set_LC_Shpnr_Gain2nd(DRV_LC_Shpnr_Gain2nd *ptr);
void drvif_color_set_LC_Diff_Ctrl0(DRV_LC_Diff_Ctrl0 *ptr);
//juwen, add LC : tone mapping curve
void drvif_color_set_LC_ToneMappingSlopePoint(unsigned int *ptr);
void drvif_color_get_LC_ToneMappingSlopePoint(unsigned int *ptr_out);
void drvif_color_set_LC_DebugMode(unsigned char DebugMode);

char drvif_color_get_LC_Enable(void);
char drvif_color_get_LC_DebugMode(void);
void drvif_color_set_HV_Num_Fac_Size(VIP_DRV_Local_Contrast_Region_Num_Size *LC_Region_Num_Size);

//juwen, LC, APL, 170626
void drvif_color_get_LC_APL(unsigned int *ptr_out);
void drvif_color_get_LC_APL_GSR(unsigned short *ptr_out);

//juwen, k5lp
void drvif_color_set_LC_saturation(DRV_LC_saturation *ptr);
void drvif_color_set_LC_avoid_flicker(DRV_LC_avoid_flicker *ptr);

// jimmy, LC decontour, k5lp
void drvif_color_set_LC_decontour(DRV_LC_decontour *ptr);
void drvif_color_get_LC_decontour(DRV_LC_decontour *ptr);
void drvif_color_get_LC_Histogram(unsigned int *plc_out);
	unsigned char drvif_color_get_LC_size_support(void);
	void drvif_color_set_LC_size_support(unsigned char u1_support);

enum
{
	_SLC_COLOR_RGN_LIMIT = 0,
	_SLC_COLOR_RGN_FULL
};

enum
{
	_SLC_COLOR_SPACE_601 = 0,
	_SLC_COLOR_SPACE_709
};

typedef struct
{
	unsigned char RightSftBits;
	unsigned char RangeSel;
	unsigned char FormatSel;
	unsigned char PxlSel;
	unsigned int HueOffset;
	unsigned int SatLowBnd;
} DRV_SLC_Histogram_Info;

typedef struct
{
	unsigned char HorBlkNum;
	unsigned char VerBlkNum;
	unsigned int HorBlkSize;
	unsigned int VerBlkSize;
} DRV_SLC_Histogram_Size;

typedef struct
{
	DRV_SLC_Histogram_Size Sz;
	DRV_SLC_Histogram_Info Info;
} DRV_SLC_Histogram;

typedef struct
{
	unsigned int H_Factor;
	unsigned int H_InitPhase1;
	unsigned int H_InitPhase2;
	unsigned int V_Factor;
	unsigned int V_InitPhase1;
	unsigned int V_InitPhase2;
} DRV_SLC_Curve_Size;

typedef struct
{
	DRV_SLC_Curve_Size Sz;
	unsigned int Curve[256][33];
} DRV_SLC_Curve;

typedef struct
{
	DRV_SLC_Histogram Histogram;
	DRV_SLC_Curve Curve;
} DRV_SLC;

typedef struct _DYNAMIC_SLC_CTRL_ST
{
	unsigned char u8LC_Target_Y_en;
	unsigned char u8LC_Hist_H;
	unsigned char u8LC_Hist_V;
	unsigned char u8Target_Y_EQ_Lv;
	unsigned char u8LC_Target_Y_face_en;
	unsigned char u8LC_Target_Y_face_weight;

	unsigned char u8TGT_YTH[4]; //0~31 bin
	unsigned short u16Target_Y_EQ_GAIN[5]; //128=1
	short s16Target_Y_EQ_OFF[5]; //-128~128

	unsigned char u8APL_gain_en; //0:disable, 1:APL GlobalGain, 2:APL CurveGain, 3: APL GlobalGain+CurveGain
	unsigned short u16APL_TH[4];//0~1023
	unsigned short u16APL_GAIN_LC[3];//128=1
	unsigned short u16APL_Curve[8][33];//0~4095

	unsigned char u8Sat_BK_CTRL; //0~31 bin
	unsigned short u8Sat_BK_Gain; //0~256

	unsigned char u8FinalBlendWeight; //0~128
} DYNAMIC_SLC_CTRL_ST;

typedef struct _DYNAMIC_SLC_CTRL_RTICE_ST
{
	short u8LC_Target_Y_en;
	short u8LC_Hist_H;
	short u8LC_Hist_V;
	short u8Target_Y_EQ_Lv;
	short u8LC_Target_Y_face_en;
	short u8LC_Target_Y_face_weight;

	short u8TGT_YTH[4]; //0~31 bin
	short u16Target_Y_EQ_GAIN[5]; //128=1
	short s16Target_Y_EQ_OFF[5]; //-128~128

	short u8APL_gain_en; //0:disable, 1:APL GlobalGain, 2:APL CurveGain, 3: APL GlobalGain+CurveGain
	short u16APL_TH[4];//0~1023
	short u16APL_GAIN_LC[3];//128=1
	short u16APL_Curve[8][33];//0~4095

	short u8Sat_BK_CTRL; //0~31 bin
	short u8Sat_BK_Gain; //0~256

	short u8FinalBlendWeight; //0~128
} DYNAMIC_SLC_CTRL_RTICE_ST;


/* SOFTWARE BASE LOCAL CONTRAST SET FUNCTIONS */
void drvif_color_set_SLC_Hist_Enable(unsigned char bEnable); // ok
void drvif_color_set_SLC_Hist_Info(DRV_SLC_Histogram_Info* ptr); // ok
void drvif_color_set_SLC_Hist_Size(DRV_SLC_Histogram_Size* ptr); // ok

void drvif_color_set_SLC_Curve_Enable(unsigned char bEnable); // ok
void drvif_color_set_SLC_Curve_Size(DRV_SLC_Curve_Size *ptr); // ok
unsigned char drvif_color_set_SLC_Curve(unsigned int *pCurve, unsigned char Blk); // ok

/* SOFTWARE BASE LOCAL CONTRAST GET FUNCTIONS */
unsigned char drvif_color_get_SLC_Hist_HorBlkNum(void);
unsigned char drvif_color_get_SLC_Hist_VerBlkNum(void);
unsigned char drvif_color_get_SLC_Hist(unsigned int *pHist, unsigned char Channel, unsigned char Blk); // ok
unsigned char drvif_color_get_SLC_Curve(unsigned int *pCurve, unsigned char Blk); // ok

/* SOFTWARE LOCAL CONTRAST AUTO FUNCTIONS*/
void SoftwareBaseLocalContrast_SimpleDCC(
	/*IN */ DYNAMIC_SLC_CTRL_ST *pCtrl,
	/*IN */ unsigned int *pHistogram,
	/*OUT*/ unsigned int *pCurve
);
void SoftwareBaseLocalContrast_FaceProtection(void);
void SoftwareBaseLocalContrast_APL_Gain(
	/*IN */ DYNAMIC_SLC_CTRL_ST *pCtrl,
	/*IN */ unsigned int *pHistogram,
	/*OUT*/ unsigned int *pCurve
);
void SoftwareBaseLocalContrast_Black_Level_Saturation_Protect(
	/*IN */ DYNAMIC_SLC_CTRL_ST *pCtrl,
	/*OUT*/ unsigned int *pCurve
);
void SoftwareBaseLocalContrast_Final_BlendWeight(
	/*IN */ DYNAMIC_SLC_CTRL_ST *pCtrl,
	/*OUT*/ unsigned int *pCurve
);

/* COLOR DEPENDENCY LOCAL CONTRAST */
typedef struct
{
	unsigned char Enable;
	unsigned int UCenter;
	unsigned int VCenter;
	unsigned char Gain;
	unsigned int U_Curve[9];
	unsigned int V_Curve[9];
} DRV_CDLC_Offset;

typedef struct
{
	unsigned char Enable;
	unsigned int U_Curve[9];
	unsigned int V_Curve[9];
} DRV_CDLC_Shape;

typedef struct
{
	unsigned char Enable;
	unsigned char Shift;
	unsigned int Curve[9];
} DRV_CDLC_Distance;

typedef struct
{
	unsigned char Y_BldEnable;
	unsigned char U_BldEnable;
	unsigned char V_BldEnable;
	DRV_CDLC_Offset Offset;
	DRV_CDLC_Shape Shape;
	DRV_CDLC_Distance Distance;
} DRV_CDLC;

/* COLOR DEPENDENCY LOCAL CONTRAST SET FUNCTIONS */
void drvif_color_set_CDLC_Enable(unsigned char bYEnable, unsigned char bUEnable, unsigned char bVEnable);
void drvif_color_set_CDLC_Offset(DRV_CDLC_Offset *pOffset);
void drvif_color_set_CDLC_Shape(DRV_CDLC_Shape *pShape);
void drvif_color_set_CDLC_Distance(DRV_CDLC_Distance *pDistance);

/*  LOCAL CONTRAST PIP OVERLAY APL */

//Bill, 20210315, set overlay APL
typedef struct {
	unsigned char lc_overlay_mode;
	unsigned int lc_overlay_value;
	unsigned char lc_overlay_xblk2;
	unsigned char lc_overlay_xblk1;
	unsigned char lc_overlay_yblk2;
	unsigned char lc_overlay_yblk1;
} DRV_LC_overlay_APL;

/*  LOCAL CONTRAST PIP OVERLAY APL SET FUNCTIONS */
void drvif_color_set_LC_overlayAPL(DRV_LC_overlay_APL *overlay);

/* OVERLAY CONTROL */
void drvif_color_set_overlay_control(unsigned char bOverlayEn);

/* LC TONEMAPPING YAVG YIN BLEND */

typedef struct {
	unsigned char lc_yavg_yin_blend_en;
	unsigned char blend_factor[8];
} DRV_LC_ToneMapping_Yavg_Yin_Blend;

void drvif_color_set_LC_Tonemapping_Yavg_Yin_Blend(DRV_LC_ToneMapping_Yavg_Yin_Blend *blend);

/*  LOCAL CONTRAST TONEMAPPING PQMASK */

typedef struct {
	unsigned char lc_PQmask_tmap_En;
	unsigned char lc_PQmask_offset;
} DRV_LC_ToneMapping_PQmask;

void drvif_color_set_LC_Tonemapping_PQmask(DRV_LC_ToneMapping_PQmask *tmap_pqmask);

/*  LOCAL SHARPNESS PQMASK */

typedef struct {
	unsigned char lc_PQmask_shp_En;
	unsigned char lc_PQmask_shp_debug_shift;
	unsigned char lc_PQmask_shp_gain;
} DRV_LC_Sharpness_PQmask;

void drvif_color_set_LC_Sharpness_PQmask(DRV_LC_Sharpness_PQmask *shp_pqmask);


void drvif_convert_DSLC_st(unsigned char direct, DYNAMIC_SLC_CTRL_ST *drv, DYNAMIC_SLC_CTRL_RTICE_ST *rtice);

#ifdef __cplusplus
}
#endif

#endif // __LOCALCONTRAST_H__


