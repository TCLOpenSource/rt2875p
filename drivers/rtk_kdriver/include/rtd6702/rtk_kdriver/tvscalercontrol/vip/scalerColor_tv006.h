#ifndef __SCALER_COLOR_TV006_H__
#define __SCALER_COLOR_TV006_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <tvscalercontrol/vip/scalerColor.h>
#ifdef CONFIG_SCALER_ENABLE_V4L2
#include <linux/v4l2-ext/videodev2-ext.h>
#endif


#ifndef SINT8
typedef	signed char				__SINT8;
#define SINT8 __SINT8
#endif

#ifndef SINT16
typedef	signed short			__SINT16;
#define SINT16 __SINT16
#endif

#define HDR_DMA_Status_MAX 20
#define OETF_PROG_SIZE_TV006 (33*2)
#define OETF_SIZE_TV006 (1024)
#define EOTF_SIZE_TV006 (1024)

typedef enum {
	RGB2YUV_BLACK_LEVEL_UNDEFINE = 0,
	RGB2YUV_BLACK_LEVEL_LOW,
	RGB2YUV_BLACK_LEVEL_HIGH,
	RGB2YUV_BLACK_LEVEL_AUTO,
} RGB2YUV_BLACK_LEVEL;

typedef enum {
	ACCESS_MODE_READ = 0,
	ACCESS_MODE_GET = ACCESS_MODE_READ,
	ACCESS_MODE_WRITE,
	ACCESS_MODE_SET = ACCESS_MODE_WRITE,
	ACCESS_MODE_MAX,
} ACCESS_MODE;
//------------------------------NR Start -------------------------------------------------

typedef struct{
	BOOLEAN 	Spatial_Enable_Y;
	UINT8		Gain;
	UINT16		Thd_UpBnd;
	UINT8		Thd_LowBnd;
	UINT8		Thd_Step;
	UINT8		SpatialThlY;
	UINT8		SpatialThl2Y;
	UINT8		SpatialThl4Y;
	UINT8	        WeightYSel;
}CHIP_LPFY_T;

typedef struct{
	BOOLEAN 	Spatial_Enable_C;
	UINT8		LPResultWeightC;
	UINT8		SpatialThlC;
	UINT8		SpatialThl2C;
	UINT8		SpatialThl4C;
	UINT8		WeightCSel;
}CHIP_LPFC_T;


typedef struct{
	UINT16	Curve_step1;
	UINT16	Curve_step2;
	UINT16	Curve_step3;
	UINT16	Curve_step4;
	UINT16	Curve_step5;
	UINT16	Curve_step6;
	UINT16	Curve_step7;
	UINT16	Curve_step8;
}CHIP_NR_CURVEMAPPING_STEP_T;

typedef struct{
	UINT8	Weight1_0;
	UINT8	Weight1_1;
	UINT8	Weight1_2;
	UINT8	Weight1_3;
	UINT8	Weight1_4;
	UINT8	Weight1_5;
	UINT8	Weight1_6;
	UINT8	Weight1_7;
	UINT8	Weight1_8;
	UINT8	Weight2_0;
	UINT8	Weight2_1;
	UINT8	Weight2_2;
	UINT8	Weight2_3;
	UINT8	Weight2_4;
	UINT8	Weight2_5;
	UINT8	Weight2_6;
	UINT8	Weight2_7;
	UINT8	Weight2_8;
}CHIP_NR_CURVEMAPPING_WEIGHT_UI_T;

typedef struct{
	UINT8		Weight1Y;
	UINT8		Weight2Y;
}CHIP_BLENDING_T;

typedef struct{
// RTNR Old Mode_General Ctrl
	BOOLEAN	Y_enable;
	UINT8	Mad[15][10];
}CHIP_TNRY_T;

typedef struct{
	BOOLEAN	C_enable;
	UINT8	Mad[7][10];
}CHIP_TNRC_T;



typedef struct{
	BOOLEAN 	Edge_LPF_enable;
	UINT8		Edge_Theshold;
	UINT8		Edge_Theshold_Step;
	BOOLEAN 	Far_Var_farEdge_refine_Enable;
	UINT8		Range;
	UINT8		Far_Var_Gain;
	UINT8		Far_Var_offset;
	UINT8		Hdiff_Frac_Shift;
	UINT8		varDiff_refine_gain;
	UINT8		Vdiff_Frac_Shift;
	BOOLEAN 	Decay_enable;
	UINT8		Hdecay_Start;
	UINT8		Hdecay_Step;
	UINT8		Vdecay_Start;
	UINT8		Vdecay_Step;
	UINT8		MNR_Weight;
	UINT8		Blending_Mode;
	BOOLEAN 	BlendByEdge_enable; // K5LP add
	BOOLEAN 	Decay_all_enable; // K5LP add
}CHIP_MNR_T;

typedef struct{
	BOOLEAN 	MosquitoNR_Enable;
	UINT8		Positive_nLevel_Offset;
	UINT8		Positive_nLevel_Shift;
	UINT8		Negative_nLevel_Offset;
	UINT8		Negative_nLevel_Shift;
	UINT8		Edge_Strength_Gain;
	UINT8		Threshold;
	UINT8		Threshold_Step;
	UINT8		Shift;
}CHIP_MNR_UI_T;


typedef struct{
	UINT8		MpegResultWeight_H;
	UINT16	MpegUpperbound_client;
}CHIP_MPEGNR_UI_T;

typedef struct{
	CHIP_LPFY_T LPF_Y;
	CHIP_LPFC_T LPF_C;
	CHIP_NR_CURVEMAPPING_STEP_T	CurveMappingStep;
	CHIP_NR_CURVEMAPPING_WEIGHT_UI_T CurveMappingWeight;
	CHIP_BLENDING_T BLENDING;
	CHIP_TNRY_T TNR_Y;
	CHIP_TNRC_T TNR_C;
} CHIP_NOISE_REDUCTION_T;


typedef struct{
	UINT16 NR_LEVEL;
        CHIP_NOISE_REDUCTION_T NR_TABLE;
}RTK_NOISE_REDUCTION_T;

typedef struct{
	CHIP_MNR_T MNR_T;
	CHIP_MNR_UI_T MNR_UI_T;
	CHIP_MPEGNR_UI_T MPEGNR_UI_T;
}CHIP_MPEG_NOISE_REDUCTION_T;

typedef struct{
	UINT16 MPEG_LEVEL;
        CHIP_MPEG_NOISE_REDUCTION_T MNR_TABLE;
}RTK_MPEG_NOISE_REDUCTION_T;



//------------------------------Nr End----------------------------------------------------

typedef struct {
	BOOLEAN	chroma_gain_en;
	BOOLEAN	chroma_gain_mode;
	BOOLEAN	chroma_gain_lookup_mode;
	UINT8	chroma_gain_base;
	BOOLEAN	chroma_enhance_above_tab_select_main;
	BOOLEAN	chroma_enhance_below_tab_select_main;
	BOOLEAN	saturation_byy_u_tab_select_main;
	BOOLEAN	saturation_byy_v_tab_select_main;

	BOOLEAN	chroma_gain_limit_en;
	UINT8	chroma_gain_limit_seg[8];
	SINT8	chroma_gain_tab0_seg[17];
	SINT8	chroma_gain_tab1_seg[17];
} CHIP_DCC_CHROMA_GAIN_T;

typedef struct {
	UINT16	y0_center;
	UINT16	u0_center;
	UINT16	v0_center;
	UINT8	y0_range;
	UINT8	u0_range;
	UINT8	v0_range;

	UINT16	y1_center;
	UINT16	u1_center;
	UINT16	v1_center;
	UINT8	y1_range;
	UINT8	u1_range;
	UINT8	v1_range;

	UINT16	y2_center;
	UINT16	u2_center;
	UINT16	v2_center;
	UINT8	y2_range;
	UINT8	u2_range;
	UINT8	v2_range;

	UINT16	y3_center;
	UINT16	u3_center;
	UINT16	v3_center;
	UINT8	y3_range;
	UINT8	u3_range;
	UINT8	v3_range;
} CHIP_DCC_SKIN_TONE_RANGE_T;

typedef struct {
	UINT16	D_DCC_SKIN_EN;

	UINT8	Y0_seg[8];
	UINT8	Y1_seg[8];
	UINT8	Y2_seg[8];
	UINT8	Y3_seg[8];
	UINT8	C0_seg[8];
	UINT8	C1_seg[8];
	UINT8	C2_seg[8];
	UINT8	C3_seg[8];
} CHIP_DCC_SKIN_TONE_GAIN_T;


typedef struct {
	CHIP_DCC_SKIN_TONE_RANGE_T	range;
	CHIP_DCC_SKIN_TONE_GAIN_T	value;
} CHIP_DCC_SKINTONE_T;

typedef struct {
	CHIP_DCC_CHROMA_GAIN_T	stChromaGain;
	CHIP_DCC_SKINTONE_T	stSkinTone;
	UINT8 DccVal;
} CHIP_DCC_T;


/* set picture ctrl*/
typedef enum {
	PIC_CTRL_Contrast = 0,
	PIC_CTRL_Brightness,
	PIC_CTRL_Color,
	PIC_CTRL_Tint,
	PIC_CTRL_ITEM_MAX,
} VPQ_PIC_CTRL_ITEM;

typedef struct _VPQ_SetPicCtrl_Mapping_T {
	UINT8 Index_0[PIC_CTRL_ITEM_MAX];
	UINT8 Index_50[PIC_CTRL_ITEM_MAX];
	UINT8 Index_100[PIC_CTRL_ITEM_MAX];

} VPQ_SetPicCtrl_Mapping_T;

typedef struct _VPQ_SetPicCtrl_T {
	UINT8 wId;
	short siVal[PIC_CTRL_ITEM_MAX];
	short Gain_Val[PIC_CTRL_ITEM_MAX];
} VPQ_SetPicCtrl_T;

/* ---------- 20150602 to follow LG DB---------------*/
/**********************************************************************
	SHP
***********************************************************************/

typedef struct{
	short	edge_mode;
	short	sobel_mode;
	short	edgextrange;
	short	sobel_upbnd;
	short	sobel_stepbit;
	short	zdiff_upbnd;
	short	zdiff_stepbit;
	short	zigsftbit_reg;
	short	texture_enable;
	short	tex_upbnd;
	short	tex_stepbit;
	short	lpf_mode;
	short	lpf_filter;
}CHIP_SEGMENTATION_T; //13

typedef struct{
	short	edgeCoringEn;
	short	edgeGainEn;
	short	bigEdgeRatio;
	short	smallEdgeRatio;
	short	deltaEdgeRatio;
	short	smallEdgeUlevel;
	short	deltaEdgeUlevel;
	short	edgeCoringClip;
	short	edgeGainBound;
	short	edgeSerRange;
	short	Edge_delta_ext_R;
}CHIP_EDGE_CORING_MODE_T; //11

typedef struct{
	short	linearY_Umode;
	short	linearY_Lmode;
	short	linearY_UB;
	short	linearY_LB;
}CHIP_Y_REMAPPING_T; //4

typedef struct{
	short	edgpk_en;
	short	edge_select_filter;
	short	edge_lpf_shift;
	short	coef_c0_0;
	short	coef_c1_0;
	short	coef_c2_0;
	short	coef_c3_0;
	short	coef_c4_0;
	short	coef_c5_0;
	short	coef_c6_0;
	short	coef_c7_0;
	short	coef_c8_0;
	short	coef_c9_0;
	short	coef_c10_0;
	short	coef_c0_1;
	short	coef_c0_2;
	short	coef_c0_3;
	short	coef_c0_4;
}CHIP_EDGE_HPF_T; //18

typedef struct{
	short	pos_s1;
	short	pos_s2;
	short	pos_s3;
	short	pos_s4;
	short	pos_s5;
	short	pos_s6;
	short	pos_s7;
	short	pos_s8;
	short	pos_s9;
	short	pos_s10;
	short	pos_s11;
	short	pos_s12;
	short	pos_s13;
	short	pos_s14;
	short	neg_s1;
	short	neg_s2;
	short	neg_s3;
	short	neg_s4;
	short	neg_s5;
	short	neg_s6;
	short	neg_s7;
	short	neg_s8;
	short	neg_s9;
	short	neg_s10;
	short	neg_s11;
	short	neg_s12;
	short	neg_s13;
	short	neg_s14;
}CHIP_EDGE_CURVEMAPPING_STEP_T;//28

typedef struct{
	short	Gain_Pos;
	short	Gain_Neg;
	short	Gain_Pos2;
	short	Gain_Neg2;
	short	Range_Pos;
	short	Range_Neg;
	short	Coring;
	short	Edge_Shift;
	short	Edge_curve_en;
	short	pos_g0;
	short	pos_g1;
	short	pos_g2;
	short	pos_g3;
	short	pos_g4;
	short	pos_g5;
	short	pos_g6;
	short	pos_g7;
	short	pos_g8;
	short	pos_g9;
	short	pos_g10;
	short	pos_g11;
	short	pos_g12;
	short	pos_g13;
	short	pos_g14;
	short	neg_g0;
	short	neg_g1;
	short	neg_g2;
	short	neg_g3;
	short	neg_g4;
	short	neg_g5;
	short	neg_g6;
	short	neg_g7;
	short	neg_g8;
	short	neg_g9;
	short	neg_g10;
	short	neg_g11;
	short	neg_g12;
	short	neg_g13;
	short	neg_g14;
}CHIP_EDGE_CURVEMAPPING_UI_T; //39

typedef struct{
	short	texpk_en;
	short	text_select_filter;
	short	tex_lpf_shift;
	short	coef_c0_0;
	short	coef_c1_0;
	short	coef_c2_0;
	short	coef_c3_0;
	short	coef_c4_0;
	short	coef_c5_0;
	short	coef_c6_0;
	short	coef_c7_0;
	short	coef_c8_0;
	short	coef_c9_0;
	short	coef_c10_0;
	short	coef_c0_1;
	short	coef_c0_2;
	short	coef_c0_3;
	short	coef_c0_4;
}CHIP_TEXTURE_HPF_T; //18

typedef struct{
	short	pos_s1;
	short	pos_s2;
	short	pos_s3;
	short	pos_s4;
	short	pos_s5;
	short	pos_s6;
	short	pos_s7;
	short	pos_s8;
	short	pos_s9;
	short	pos_s10;
	short	pos_s11;
	short	pos_s12;
	short	pos_s13;
	short	pos_s14;
	short	neg_s1;
	short	neg_s2;
	short	neg_s3;
	short	neg_s4;
	short	neg_s5;
	short	neg_s6;
	short	neg_s7;
	short	neg_s8;
	short	neg_s9;
	short	neg_s10;
	short	neg_s11;
	short	neg_s12;
	short	neg_s13;
	short	neg_s14;
}CHIP_TEXTURE_CURVEMAPPING_STEP_T;//28

typedef struct{
	short	Gain_Pos;
	short	Gain_Neg;
	short	Gain_Pos2;
	short	Gain_Neg2;
	short	Range_Pos;
	short	Range_Neg;
	short	Coring;
	short	Tex_Shift;
	short	enable;
	short	pos_g0;
	short	pos_g1;
	short	pos_g2;
	short	pos_g3;
	short	pos_g4;
	short	pos_g5;
	short	pos_g6;
	short	pos_g7;
	short	pos_g8;
	short	pos_g9;
	short	pos_g10;
	short	pos_g11;
	short	pos_g12;
	short	pos_g13;
	short	pos_g14;
	short	neg_g0;
	short	neg_g1;
	short	neg_g2;
	short	neg_g3;
	short	neg_g4;
	short	neg_g5;
	short	neg_g6;
	short	neg_g7;
	short	neg_g8;
	short	neg_g9;
	short	neg_g10;
	short	neg_g11;
	short	neg_g12;
	short	neg_g13;
	short	neg_g14;
}CHIP_TEXTURE_CURVEMAPPING_UI_T;//39

typedef struct {
	short	h9_c0;
	short	h9_c1;
	short	h9_c2;
	short	h9_c3;
	short	h9_c4;
	short	h13_c0;
	short	h13_c1;
	short	h13_c2;
	short	h13_c3;
	short	h13_c4;
	short	h13_c5;
	short	h13_c6;
	short	h21_c0;
	short	h21_c1;
	short	h21_c2;
	short	h21_c3;
	short	h21_c4;
	short	h21_c5;
	short	h21_c6;
	short	h21_c7;
	short	h21_c8;
	short	h21_c9;
	short	h21_c10;
	short	v91_c0;
	short	v91_c1;
	short	v91_c2;
	short	v91_c3;
	short	v91_c4;
	short	v92_c0;
	short	v92_c1;
	short	v92_c2;
	short	v92_c3;
	short	v92_c4;
	short	v93_c0;
	short	v93_c1;
	short	v93_c2;
	short	v93_c3;
	short	v93_c4;
} CHIP_COMMON_HPF_T;//38

typedef struct{
	short	Slope_Gain_Enable;
	short	DIR;
	short	FH;
	short	Gain_boundPos;
	short	Gain_boundNeg;
	short	Gain_extendPos;
	short	Gain_extendNeg;
	short	flat_enable;
	short	flat_offset;
	short	flat_gain;
	short	flat_range;//k5lp add
}CHIP_SLOPE_GAIN_MASK_T;//11

typedef struct {
	short vpk_en;
	short vpk_c0;
	short vpk_c1;
	short vpk_c2;
	short vpk_c3;
	short vpk_c4;
} CHIP_VERTICAL_HPF_T;//6

typedef struct {
	short	pos_s1;
	short	pos_s2;
	short	pos_s3;
	short	pos_s4;
	short	pos_s5;
	short	pos_s6;
	short	pos_s7;
	short	pos_s8;
	short	pos_s9;
	short	pos_s10;
	short	pos_s11;
	short	pos_s12;
	short	pos_s13;
	short	pos_s14;
	short	neg_s1;
	short	neg_s2;
	short	neg_s3;
	short	neg_s4;
	short	neg_s5;
	short	neg_s6;
	short	neg_s7;
	short	neg_s8;
	short	neg_s9;
	short	neg_s10;
	short	neg_s11;
	short	neg_s12;
	short	neg_s13;
	short	neg_s14;
} CHIP_VERTICAL_CURVEMAPPING_STEP_T;//28

typedef struct {
	short	Gain_Pos;
	short	Gain_Neg;
	short	Coring;
	short	Gain_Pos2;
	short	Gain_Neg2;
	short	Range_Pos;
	short	Range_Neg;
	short	V_Parameters_En;
	short	Edge_Gain_Pos;
	short	Edge_Gain_Neg;
	short	Edge_Coring;
	short	Edge_Gain_Pos2;
	short	Edge_Gain_Neg2;
	short	Edge_Range_Pos;
	short	Edge_Range_Neg;
	short	v_tex_en;
	short	v_dering_en;
	short	enable;
	short	pos_g0;
	short	pos_g1;
	short	pos_g2;
	short	pos_g3;
	short	pos_g4;
	short	pos_g5;
	short	pos_g6;
	short	pos_g7;
	short	pos_g8;
	short	pos_g9;
	short	pos_g10;
	short	pos_g11;
	short	pos_g12;
	short	pos_g13;
	short	pos_g14;
	short	neg_g0;
	short	neg_g1;
	short	neg_g2;
	short	neg_g3;
	short	neg_g4;
	short	neg_g5;
	short	neg_g6;
	short	neg_g7;
	short	neg_g8;
	short	neg_g9;
	short	neg_g10;
	short	neg_g11;
	short	neg_g12;
	short	neg_g13;
	short	neg_g14;
} CHIP_VERTICAL_CURVEMAPPING_UI_T;//48

typedef struct{
	short	Slope_Gain_V_Enable;
	short	DIR;
	short	Gain_boundPos;
	short	Gain_boundNeg;
	short	Gain_extendPos;
	short	Gain_extendNeg;
	short	flat_v_enable;
	short	flat_offset_v;
	short	flat_gain_v;
	short	flat_range_v;// k5lp add
}CHIP_VERTICAL_SLOPE_GAIN_MASK_T;//10

typedef struct {
	short	gain_by_y_en;
	short	gain_by_y_v_en;
	short	gain_by_y_tex_en;
	short	gain_by_y_edg_en;
	short	lpf_weight_en;
	short	lpf_weight_edg_en;
	short	lpf_weight_tex_en;
	short	max_min_gain_en;
	short	max_min_v_en;
	short	max_min_tex_en;
	short	max_min_edg_en;
	short	lv_by_Y_enable;
	short	lv_by_y_edg_enable;
	short	lv_by_y_v_edg_enable;
	short	lv_by_y_tex_enable;
	short	lv_by_y_v_enable;
	short	coef_c0;
	short	coef_c1;
	short	coef_c2;
	short	coef_c3;
	short	coef_c4;
	short	coef_c5;
	short	maxmin_s1;
	short	maxmin_s2;
	short	maxmin_s3;
	short	maxmin_s4;
	short	maxmin_s5;
	short	maxmin_s6;
	short	maxmin_g0;
	short	maxmin_g1;
	short	maxmin_g2;
	short	maxmin_g3;
	short	maxmin_g4;
	short	maxmin_g5;
	short	maxmin_g6;
	short	blending_s1;
	short	blending_s2;
	short	blending_s3;
	short	blending_s4;
	short	blending_s5;
	short	blending_s6;
	short	blending_g0;
	short	blending_g1;
	short	blending_g2;
	short	blending_g3;
	short	blending_g4;
	short	blending_g5;
	short	blending_g6;
	short	y_s1;
	short	y_s2;
	short	y_s3;
	short	y_s4;
	short	y_s5;
	short	y_s6;
	short	y_g0;
	short	y_g1;
	short	y_g2;
	short	y_g3;
	short	y_g4;
	short	y_g5;
	short	y_g6;
} CHIP_MKIII_T;//61

typedef struct {
	short	i_dcti_enable;
	short	i_dcti_uv_sync_enable;
	short	i_dcti_uv_align_enable;
	short	i_dcti_mode;
	short	i_dcti_gain;
	short	i_dcti_Max_search_range;
	short	i_dcti_Threshold_of_smooth;
	short	d_dcti_enable;
	short	d_dcti_uv_sync_enable;
	short	d_dcti_uv_align_enable;
	short	d_dcti_mode;
	short	d_dcti_gain;
	short	d_dcti_Max_search_range;
	short	d_dcti_Threshold_of_smooth;
	short	vcti_enable;
	short	vcti_type;
	short	vcti_gain;
	short	vcti_type3_thd;
} CHIP_CTI_T;//18

typedef struct {
	short	d_LTI_enable;
	short	d_LTI_can_shift;
	short	d_LTI_maxlen;
	short	d_LTI_pnth;
	short	ffd2_Enable;
	short	Data_sel;
	short	ovc_extend;//  k5lp add
	short	ffd0_start;// k5lp add
	short	D1_CP_Shift;
	short	DCP_th;
	short	Flat_s_range;
	short	Flat_score_th1;
	short	Ffd1_th;
	short	Ffd1_ratio;
	short	Ffd2_range2R;
	short	Ffd2_range2L;
	short	Ffd2_ratio;
	short	gain_pos;
	short	offset_pos;
	short	gain_neg;
	short	offset_neg;
	short	Tnoff0;
	short	Tnoff1;
	short	Tnoff2;
	short	Tnoff3;
	short	Tnoff4;
	short	Tnoff5;
	short	Tnoff6;
	short	Tnoff7;
	short	Tnoff8;
	short	Tnoff9;
	short	Tnoff10;
	short	Tnoff11;
	short	Tnoff12;
	short	Tnoff13;
	short	Tnoff14;
}CHIP_HLTI_T;//36

typedef struct{
	short	d_LTI_enable;
	short	Can_shift_v;
	short	maxlen_v;
	short	pnth_v;
	short	ovc_extend;// k5lp add
	short	ffd0_start;// k5lp add
	short	ffd1_th_v;
	short	ffd1_ratio_v;
	short	gain_pos_v;
	short	offset_pos_v;
	short	gain_neg_v;
	short	offset_neg_v;
	short	Tnoff0_v;
	short	Tnoff1_v;
	short	Tnoff2_v;
	short	Tnoff3_v;
	short	Tnoff4_v;
	short	Tnoff5_v;
	short	Tnoff6_v;
}CHIP_VLTI_T; //19

typedef struct {
	short	mb_peaking_en;
	short	h_c0;
	short	h_c1;
	short	h_c2;
	short	h_c3;
	short	v_c0;
	short	v_c1;
	short	v_c2;
} CHIP_MULTIBAND_PEAKING_HPF_T;//8

typedef struct {
	short	mb_h_en;
	short	H_Gain_Pos;
	short	H_Gain_Neg;
	short	H_Coring;
	short	H_Gain_Pos2;
	short	H_Gain_Neg2;
	short	H_Range_Pos;
	short	H_Range_Neg;
	short	H_Edge_Shift;
	short	mb_v_en;
	short	V_Gain_Pos;
	short	V_Gain_Neg;
	short	V_Coring;
	short	V_Gain_Pos2;
	short	V_Gain_Neg2;
	short	V_Range_Pos;
	short	V_Range_Neg;
	short	V_Edge_Shift;
} CHIP_MULTIBAND_PEAKING_UI_T;//18

typedef struct{
	short	ise_en;
	short	ise_step;
	short	ise_scale;
	short	ise_ub;
	short	ise_h_st;
	short	ise_h_end;
	short	ise_v_st;
	short	ise_v_end;
}CHIP_ISE_T; //8

typedef struct{
	short	tex_flat_filter_en;
	short	coef_c0_2;
	short	coef_c0_1;
	short	coef_c0_0;
	short	coef_c1_0;
	short	coef_c2_0;
	short	coef_c3_0;
}CHIP_FLAT_FILTER_T;//7






typedef struct{
	short	Maxmin_Limit_En;
	short	Maxmin_Shift;
	short	sr_var_thd;
	short	Maxmin_Mode;
	short	Maxmin_Range;
	short	Maxmin_Thd;
	short	Edge_Range;
	short	Delta_Gain;
	short	LPF_Range;
	short	lpf_range_en;
	short	lpf_range_thd;
	short	lpf_range_gain;
	short	lpf_range_step;
	short	lpf_gain;
	short	lpf_edge_en;
	short	lpf_edge_thd;
	short	lpf_edge_gain;
	short	lpf_edge_step;
	short	bif_range_en;
	short	bif_thd0;
	short	bif_thd1;
	short	bif_thd2;
	short	bif_thd3;
	short	bif_thd4;
	short	bif_thd5;
	short	bif_thd6;
	short	bif_thd7;
	short	bif_range_thd0;
	short	bif_range_thd1;
	short	bif_range_gain0;
	short	bif_range_gain1;
	short	bif_range_step0;
	short	bif_range_step1;
	short	bif_gain;
	short	bif_edge_en;
	short	bif_edge_thd;
	short	bif_edge_gain;
	short	bif_edge_step;
	short	by_gain_en;
	short	diffdir_cond_e;
	short	pgain0;
	short	pgain1;
	short	pgain2;
	short	pgain3;
	short	pgain4;
	short	pgain5;
	short	pgain6;
	short	pgain7;
	short	pgain8;
	short	ngain0;
	short	ngain1;
	short	ngain2;
	short	ngain3;
	short	ngain4;
	short	ngain5;
	short	ngain6;
	short	ngain7;
	short	ngain8;
}CHIP_SECOND_SR_T;

typedef struct{
	short	emf_enable;
	short	emf_range;
	short	emf_shiftbit;
	short	uBlend_wt;
	short	fix_extend_pos;
	short	fix_extend_neg;
	short	seg0_x;
	short	seg1_x;
	short	gain0;
	short	gain1;
	short	gain2;
	short	Offset0;
	short	v5; //k5lp add
	short	h11; //k5lp add
} CHIP_EDGE_EMF_UI_T;

typedef struct {
	short	emf_enable;
	short	emf_range;
	short	emf_shiftbit;
	short	blend_wt;
	short	fix_extend_pos;
	short	fix_extend_neg;
	short	seg0_x;
	short	seg1_x;
	short	gain0;
	short	gain1;
	short	gain2;
	short	Offset0;
	short	emf_texgain;//k5lp add
	short	v5; //k5lp add
	short	h11; //k5lp add
} CHIP_TEXTURE_EMF_UI_T;

typedef struct {
	short	vemf_en;
	short	vext_reg;
} CHIP_VERTICAL_EMF_UI_T;

typedef struct {
	CHIP_SEGMENTATION_T		        stSegmentation;//#1 13
	CHIP_EDGE_CORING_MODE_T	                stEdgeCoringMode;//#2 11
	CHIP_Y_REMAPPING_T		        stYRemapping;//#3 4
	CHIP_EDGE_HPF_T			        stEdgeHPF;//#4 18
	CHIP_EDGE_CURVEMAPPING_STEP_T	        stEdgeCurveMappingStep;//#5 28
	CHIP_EDGE_CURVEMAPPING_UI_T	        stEdgeCurveMappingUI;//#6 39
	CHIP_TEXTURE_HPF_T			stTextureHPF;//#7 18
	CHIP_TEXTURE_CURVEMAPPING_STEP_T	stTextureCurveMappingStep;//#8 28
	CHIP_TEXTURE_CURVEMAPPING_UI_T		stTextureCurveMappingUI;//#9 39
	CHIP_COMMON_HPF_T	                stCommonHPF;//#10 38
	CHIP_SLOPE_GAIN_MASK_T	                stSlopeGainMask;//#11 11
	CHIP_VERTICAL_HPF_T		        stVerticalHPF;//#12 6
	CHIP_VERTICAL_CURVEMAPPING_STEP_T	stVerticalCurveMappingStep;//#13 28
	CHIP_VERTICAL_CURVEMAPPING_UI_T		stVerticalCurveMappingUI;//#14 48
	CHIP_VERTICAL_SLOPE_GAIN_MASK_T	        stVerticalSlopeGainMask; //#15 10
	CHIP_MKIII_T	                        stMKIII;//#16 61
	CHIP_CTI_T		                stCTI;//#17 18
	CHIP_HLTI_T	                        stHLTI;//#18 36
	CHIP_VLTI_T	                        stVLTI;//#19 19
	CHIP_MULTIBAND_PEAKING_HPF_T	        stMultibandPeakingHPF; //#20 8
	CHIP_MULTIBAND_PEAKING_UI_T	        stMultibandPeakingUI; //#21 18
	CHIP_ISE_T	                        stIse; //#22 8
	CHIP_FLAT_FILTER_T	                stFlatfilter; //#23 7
	CHIP_EDGE_EMF_UI_T	                stEdgeEMF; //#24   14
	CHIP_TEXTURE_EMF_UI_T	                stTextureEMF;//#25  15
	CHIP_VERTICAL_EMF_UI_T	                stVerticalEMF;//#26  2

} CHIP_SHARPNESS_T;

typedef struct{
	short	ring_h_c0;
	short	ring_h_c1;
	short	ring_h_c2;
	short	ring_h_c3;
	short	ring_h_c4;
	short	ring_h_c5;
	short	ring_v_c0;
	short	ring_v_c1;
	short	ring_v_c2;
	short	ring_v_c3;
	short	ring_v_c4;
	short	Ring_Gen_Gain;
	short	Cur_Period;
	short	Cur_Gain;
	short	Cur_Offset;
	short	pndu; //k5lp add
}CHIP_SR_UI_T; //16

typedef struct {
	short	ring_h_c0;
	short	ring_h_c1;
	short	ring_h_c2;
	short	ring_h_c3;
	short	ring_h_c4;
	short	ring_h_c5;
	short	ring_v_c0;
	short	ring_v_c1;
	short	ring_v_c2;
	short	ring_v_c3;
	short	ring_v_c4;
	short	Ring_Gen_Gain;
	short	Cur_Period;
	short	Cur_Gain;
	short	Cur_Offset;
	short	pndu; //k5lp add
}CHIP_SUPER_RESOLUTION_T;



typedef struct {
	CHIP_EDGE_EMF_UI_T	stEdgeEMF;
	CHIP_TEXTURE_EMF_UI_T	stTextureEMF;
	CHIP_VERTICAL_EMF_UI_T	stVerticalEMF;
} CHIP_EDGE_ENHANCE_T;

typedef struct {
	CHIP_SHARPNESS_T		stSharpness;
} CHIP_SHARPNESS_UI_T;

typedef struct {
	CHIP_EDGE_ENHANCE_T		stEdgeEnhance;
} CHIP_EDGE_ENHANCE_UI_T;

typedef struct {
	short SR_LEVEL;
	CHIP_SR_UI_T sr_ui;
} RTK_SR_UI_T;

typedef struct {
	unsigned char Sharpness_OSD_bottom;
	unsigned char Sharpness_OSD_mid;
	unsigned char Sharpness_OSD_top;
	unsigned char Sharpness_ChipValue_bottom;
	unsigned char Sharpness_ChipValue_mid;
	unsigned char Sharpness_ChipValue_top;
} Sharpness_OSD_Remap_Curve;

typedef union
{
    UINT32 Value;
    struct{
        UINT32  tnoff0:4;
        UINT32  tngain0:5;
        UINT32  tnoff1:4;
        UINT32  tngain1:5;
        UINT32  tnoff2:4;
        UINT32  tngain2:5;
        UINT32  res1:5;
    };
}VIP_SHP_LTI_GainOffset;


/*typedef struct{
	CHIP_HLTI_T h_lti;
	CHIP_VLTI_T v_lti;

}CHIP_LTI_T;*/



typedef struct {
BOOLEAN decontour_en;
UINT8 Idecontour_level; // 0~9
UINT8 Ddecontour_curve[16]; // 0~63 each
}CHIP_DECONTOUR_T;


typedef struct {
	UINT16 UI_Lv;
        CHIP_DECONTOUR_T De_Contour_Table;
} RTK_DECONTOUR_T;


/*=========ICM start=========*/
#define	CHIP_CM_HUE_SEG	60
#define	CHIP_CM_SAT_SEG	12
#define	CHIP_CM_ITN_SEG	8

#define	CHIP_CM_TBLPOINT	8
#define	RGB_OFFSET_TABLE_TV006 0
#define	TV006_ICM_DB_ITEM_NUM 728
#define	TV006_ICM_DB_COLOR_REGION_NUM 683


typedef enum {
	CHIP_CM_COLOR_REGION_RED,
	CHIP_CM_COLOR_REGION_GRN,
	CHIP_CM_COLOR_REGION_BLU,
	CHIP_CM_COLOR_REGION_CYN,
	CHIP_CM_COLOR_REGION_MGT,
	CHIP_CM_COLOR_REGION_YLW,
	CHIP_CM_COLOR_REGION_SKIN,
	CHIP_CM_COLOR_REGION_RESERVED1,
	CHIP_CM_COLOR_REGION_RESERVED2,
	CHIP_CM_COLOR_REGION_MAX
} CHIP_CM_COLOR_REGION_T;

typedef struct{
	BOOLEAN		enable;		/*bool*/
	CHIP_CM_COLOR_REGION_T		regionNum;	/*CHIP_CM_COLOR_REGION_T*/
	UINT16		hue_x[CHIP_CM_TBLPOINT];
	UINT16		hue_g[CHIP_CM_TBLPOINT];
	UINT16		sat_x[CHIP_CM_TBLPOINT];
	UINT16		sat_g[CHIP_CM_TBLPOINT];
	UINT16		Int_x[CHIP_CM_TBLPOINT];
	UINT16		Int_g[CHIP_CM_TBLPOINT];
	UINT16		r_offset_by_hue[CHIP_CM_TBLPOINT];
	UINT16		g_offset_by_hue[CHIP_CM_TBLPOINT];
	UINT16		b_offset_by_hue[CHIP_CM_TBLPOINT];
}CHIP_COLOR_REGION_T;

typedef struct{
	CHIP_COLOR_REGION_T stColorRegion[CHIP_CM_COLOR_REGION_MAX];
	UINT16		gain_by_sat_for_rgb_offset[CHIP_CM_TBLPOINT];
	UINT16		gain_by_int_for_rgb_offset[CHIP_CM_TBLPOINT];
	BOOLEAN keepY;
}CHIP_COLOR_REGION_TYPE_T;

typedef struct {
	CHIP_COLOR_REGION_TYPE_T	stColorRegionType;
} CHIP_CM_REGION_T;


typedef struct {
	UINT8	stColorGainHue;
	UINT8	stColorGainSat;
	UINT8	stColorGainInt;
} CHIP_COLOR_LEVEL_GAIN_T;


typedef struct {
	UINT8	masterGain;
	UINT8	RGBOffset_masterGain;
	CHIP_COLOR_LEVEL_GAIN_T	stColorGain;
} CHIP_COLOR_GAIN_T;


typedef struct {
	CHIP_COLOR_GAIN_T		stColor[CHIP_CM_COLOR_REGION_MAX];
} CHIP_COLOR_CONTROL_T;


typedef struct {
	CHIP_COLOR_REGION_TYPE_T	stRgn;
	CHIP_COLOR_CONTROL_T		stCtrl;
} CHIP_COLOR_ICM_GAIN_ADJUST_T;


typedef struct {
	UINT16 rgn_idx[CHIP_CM_HUE_SEG];
	UINT8 rgn_pillar[CHIP_CM_HUE_SEG];
	UINT16 rgn_gain[CHIP_CM_HUE_SEG];
	UINT8 rgn_pillar_rgn[CHIP_CM_HUE_SEG];
	int RGB_offset_curve[3][HUESEGMAX];
	UINT8 pillar_num;
} CM_HUE_SECTION;


typedef struct {
	UINT16 rgn_idx[CHIP_CM_SAT_SEG];
	UINT8 rgn_pillar[CHIP_CM_SAT_SEG];
	UINT16 rgn_gain[CHIP_CM_COLOR_REGION_MAX][CHIP_CM_SAT_SEG];
	UINT8 pillar_num;
} CM_SAT_SECTION;


typedef struct {
	UINT16 rgn_idx[CHIP_CM_ITN_SEG];
	UINT8 rgn_pillar[CHIP_CM_ITN_SEG];
	UINT16 rgn_gain[CHIP_CM_COLOR_REGION_MAX][CHIP_CM_ITN_SEG];
	UINT8 pillar_num;
} CM_ITN_SECTION;


typedef enum {
	COLOR_MODE_NONE = 0,
	COLOR_MODE_ADV,
	COLOR_MODE_EXP
} COLOR_MODE;

typedef struct
{
	SINT8 sCmRegion;// 0~16
	SINT8 sSatGainValue;// -30~30
	SINT8 sHueGainValue;// -30~30
	SINT8 sLuminanceValue;// -30~30
	SINT8 sFreshColorValue;// 0~3:off,low,med,high
} ICM_CM_PARAM_T;

typedef struct
{
	SINT8 sCmPreferred;// 0:skin,1:blus sky,2:grass
	SINT8 sPreferredColorValue;// -5~5
	SINT8 sFreshColorValue;// 0~3:off,low,med,high
} ICM_PC_PARAM_T;

typedef struct {
	unsigned char ctrl[4];
	CHIP_CM_REGION_T cm_rgn;
} CHIP_CM_REGION_EXT_T;

typedef struct {
	unsigned char ctrl[4];
	ICM_PC_PARAM_T ui_param;
	CHIP_COLOR_CONTROL_T cm_ctrl;
} CHIP_COLOR_CONTROL_ADV_EXT_T;

typedef struct {
	unsigned char ctrl[4];
	ICM_CM_PARAM_T ui_param;
	CHIP_COLOR_CONTROL_T cm_ctrl;
} CHIP_COLOR_CONTROL_EXP_EXT_T;

typedef struct {
	unsigned char ctrl[4];
	UINT32 uFcVal;
	CHIP_COLOR_CONTROL_T cm_ctrl;
} CHIP_COLOR_CONTROL_FRESH_T;
/*=========ICM end==========*/

/*=========ICM for RTICE protocol start==========*/
typedef struct {
	UINT16	enable;
	UINT16	regionNum;
	UINT16	hue_x[CHIP_CM_TBLPOINT];
	UINT16	hue_g[CHIP_CM_TBLPOINT];
	UINT16	sat_x[CHIP_CM_TBLPOINT];
	UINT16	sat_g[CHIP_CM_TBLPOINT];
	UINT16	Int_x[CHIP_CM_TBLPOINT];
	UINT16	Int_g[CHIP_CM_TBLPOINT];
} CHIP_COLOR_REGION_RTICE_T;

typedef struct {
	CHIP_COLOR_REGION_RTICE_T	stColorRegion[CHIP_CM_COLOR_REGION_MAX];
} CHIP_COLOR_REGION_TYPE_RTICE_T;

typedef struct {
	UINT16	stColorGainHue;
	UINT16	stColorGainSat;
	UINT16	stColorGainInt;
} CHIP_COLOR_LEVEL_GAIN_RTICE_T;

typedef struct {
	UINT16 				masterGain;
	CHIP_COLOR_LEVEL_GAIN_RTICE_T	stColorGain;
} CHIP_COLOR_GAIN_RTICE_T;

typedef struct {
	CHIP_COLOR_GAIN_RTICE_T		stColor[CHIP_CM_COLOR_REGION_MAX];
} CHIP_COLOR_CONTROL_RTICE_T;

typedef struct {
	CHIP_COLOR_REGION_TYPE_RTICE_T	stRgn;
	CHIP_COLOR_CONTROL_RTICE_T	stCtrl;
} CHIP_COLOR_ICM_GAIN_ADJUST_RTICE_T;
/*=========ICM for RTICE protocol end==========*/

/*=========Gamut start==========*/
typedef enum {
	IDX_3DLUT_BYPASS, //idx 0
	IDX_3DLUT_BT2020_WCG, //idx 1
	IDX_3DLUT_BT2020_OLED, //idx 2
	IDX_3DLUT_BT2020_Sharp, //idx 3
	IDX_3DLUT_BT2020_NCG, //idx 4
	IDX_3DLUT_BT709_WCG_49, //idx 5
	IDX_3DLUT_BT709_WCG_55, //idx 6
	IDX_3DLUT_BT709_WCG_60, //idx 7
	IDX_3DLUT_BT709_WCG_65, //idx 8
	IDX_3DLUT_BT709_Sharp_70, //idx 9
	IDX_3DLUT_BIG_TABLE_MAX, //idx 10
	IDX_3DLUT_EXT_TABLE_START = IDX_3DLUT_BIG_TABLE_MAX, //idx 10
	IDX_3DLUT_55B6 = IDX_3DLUT_EXT_TABLE_START, //idx 10
	IDX_3DLUT_65B6, //idx 11
	IDX_3DLUT_43UH61, //idx 12
	IDX_3DLUT_49UH61, //idx 13
	IDX_3DLUT_55UH61, //idx 14
	IDX_3DLUT_65UH61, //idx 15
	IDX_3DLUT_INX50, //idx 16
	IDX_3DLUT_INX65, //idx 17
	IDX_3DLUT_BOE49, //idx 18
	IDX_3DLUT_BOE55, //idx 19
	IDX_3DLUT_60UH61, //idx 20
	IDX_3DLUT_55UH765, //idx 21
	IDX_3DLUT_60UH765, //idx 22
	IDX_3DLUT_65UH765, //idx 23
	IDX_3DLUT_43_PHT, //idx 24
	IDX_3DLUT_49_PHT, //idx 25
	IDX_3DLUT_MAX,
} GAMUT_3D_LUT_IDX;

#define LUT3D_EXTEND_TBL_Num   (IDX_3DLUT_MAX-IDX_3DLUT_EXT_TABLE_START)

typedef struct {
	unsigned short r_data;
	unsigned short g_data;
	unsigned short b_data;
} GAMUT_3D_LUT_PT_T;

typedef struct {
	unsigned char index;
	GAMUT_3D_LUT_PT_T pt[9][9][9];
} GAMUT_3D_LUT_9x9x9_T;

typedef struct {
	unsigned char index;
	GAMUT_3D_LUT_PT_T pt[4913];
} GAMUT_3D_LUT_17x17x17_T;

typedef struct {
	UINT32 		invgamma_enable;	// inverse gamma & gamma enable,  0: disable, 1: enable
	SINT16		gamutmatrix_00;		// 3x3 matrix
	SINT16		gamutmatrix_01;
	SINT16		gamutmatrix_02;
	SINT16		gamutmatrix_10;
	SINT16		gamutmatrix_11;
	SINT16		gamutmatrix_12;
	SINT16		gamutmatrix_20;
	SINT16		gamutmatrix_21;
	SINT16		gamutmatrix_22;
	BOOLEAN 	b3DLUT_enable;		// 3D LUT,  0: disable, 1: enable
	UINT8		u3DLUT_index;		// 3D LUT table index number
	BOOLEAN 	b3DLUT_internalselect;
} CHIP_COLOR_GAMUT_MAPPING_IO_T;
/*=========Gamut end==========*/

typedef enum {
	STOP_VPQ_IOC_START = 0,
	STOP_VPQ_IOC_SET_FreshContrast = STOP_VPQ_IOC_START,
	STOP_VPQ_IOC_SET_FreshContrastLUT,
	STOP_VPQ_IOC_SET_SAT_LUT,
	STOP_VPQ_LED_LDGetAPL,
	STOP_VPQ_IOC_MAX,
} VPQ_IOCTL_STOP_RUN;

typedef enum {
	VPQ_GAMMA_MODE_INV_OFF_INNER_BYPASS = 0,
	VPQ_GAMMA_MODE_INV_OFF_INNER_ON,
	VPQ_GAMMA_MODE_INV_ON_INNER_BYPASS,
	VPQ_GAMMA_MODE_INV_ON_INNER_ON,
} VPQ_COLOR_GAMMA_MODE;

typedef struct
{
	int wId;
	int uCgVal;
} RTK_COLOR_GAIN_T;

typedef enum
{
	DISP_HIGH_LUM1_MSE_ON = 0,
	DISP_HIGH_LUM2,
	DISP_LOW_POWER1,
	DISP_MLE_MODE_OFF,
	DISP_HIGH_LUM1_MSE_OFF,
	DISP_LOW_POWER2,
	DISP_LOW_POWER2_SC_OFF,
	DISP_MPLUS_MODE5, // Add MHE Mode 09/20/2018
	DISP_MPLUS_MODE6, // Add MHE Mode 09/20/2018
	DISP_MPLUS_MODE7, // Add MHE Mode 09/20/2018
	DISP_MPLUS_MODE8, // Add MHE Mode 09/20/2018
	DISP_MLE_MODE_MAX,
} DISP_MPLUS_MODE_T;

typedef enum {
 HAL_VPQ_HDR_MODE_SDR,
 HAL_VPQ_HDR_MODE_DOLBY,
 HAL_VPQ_HDR_MODE_HDR10,
 HAL_VPQ_HDR_MODE_HLG,
 HAL_VPQ_HDR_MODE_TECHNICOLOR,
 HAL_VPQ_HDR_MODE_ST2094,
 HAL_VPQ_HDR_MODE_HDREFFECT,
 HAL_VPQ_HDR_MODE_SDR_MAX_RGB,
 HAL_VPQ_HDR_MODE_MAX
}HAL_VPQ_HDR_MODE_T;

typedef enum {
 HAL_VPQ_COLORIMETRY_BT601 = 0,
 HAL_VPQ_COLORIMETRY_BT709,
 HAL_VPQ_COLORIMETRY_BT2020,
 HAL_VPQ_COLORIMETRY_MAX
}HAL_VPQ_COLORIMETRY_T;


typedef struct{
	UINT8	M_LC_Comp_En;	//Master enable
	UINT8	LC_Tone_Mapping_En;	//Tone Mapping enable
	UINT8	LC_tmap_slope_unit;	//slope (0, 1)
	UINT8	LC_tmap_blend_factor;	//blend (0~255)
	UINT8	LC_tmap_curve_Grid[8];	// Grid setting
	UINT16	LC_tmap_curve_tone[8];	// Tone setting
	UINT16	LC_ToneMappingSlopePont_Table[256];	// Tone Mapping curve data (8 curves)
	UINT8	LC_saturation_En;
	SINT16	LC_saturation_gain;
	UINT8	LC_saturation_blend_factor;
	UINT8	LC_no_saturation_enhance_thl;
} CHIP_LOCAL_CONTRAST_T;

void fwif_color_set_rgb2yuv_black_level_tv006(RGB2YUV_BLACK_LEVEL lv);
RGB2YUV_BLACK_LEVEL fwif_color_get_rgb2yuv_black_level_tv006(void);
unsigned char fwif_color_get_colorspacergb2yuvtransfer_table_tv006(unsigned char channel, unsigned char nSrcType, unsigned char nSD_HD, bool bUse_HDMI_Range_Detect);

void fwif_color_set_ICM_table_driver_init_tv006(void);
void fwif_color_set_ICM_table_driver_init_elem_tv006(COLORELEM_TAB_T *pIcm_tab_elem);
void fwif_color_set_ICM_table_driver_tv006(COLORELEM_TAB_T *pIcm_tab_elem, unsigned char waiting);
unsigned char fwif_color_calc_ICM_gain_table_driver_tv006(CHIP_COLOR_REGION_TYPE_T *crRgn_in, CHIP_COLOR_CONTROL_T *crCtrl_in, COLORELEM_TAB_T *icmTab_in, COLORELEM_TAB_T *icmTab_out);
void fwif_color_icm_pillar_sendRPCtoVideo(unsigned char update_ICM_Pillar_Offset);
unsigned char fwif_color_set_ICM_RGB_offset_tv006(void);
unsigned char fwif_color_icm_set_pillar(void);
unsigned char fwif_color_icm_set_pillar_RGB_Offset_byD_ISR(void);

unsigned char fwif_color_set_YUV2RGB_UV_Offset_tv006(unsigned char display, unsigned char enable, unsigned char mode, DRV_VIP_YUV2RGB_UV_Offset *pUVOffset, DRV_VIP_YUV2RGB_UV_Offset_byUV_CURVE *uv_offset_byuv_curve);

unsigned char fwif_color_get_Decide_YUV2RGB_TBL_Index_tv006(unsigned char src_idx, unsigned char display, unsigned char Input_Data_Mode);
int fwif_color_set_Picture_Control_tv006(unsigned char display, VPQ_SetPicCtrl_T *p_setPicCtrl);
int fwif_color_setCon_Bri_Color_Tint_tv006(unsigned char display, _system_setting_info *system_info_struct);
/*unsigned char fwif_OsdMapToRegValue_tv006(APP_Video_OsdItemType_t bOsdItem, VPQ_SetPicCtrl_T *p_setPicCtrl);*/
void fwif_color_CalContrast_Compensation_tv006(unsigned char display, unsigned char Contrast_Gain, _system_setting_info *system_info_struct);
int fwif_color_CalCon_Bri_Color_Tint_Gain_tv006(unsigned char display, VPQ_SetPicCtrl_T *p_setPicCtrl, _system_setting_info *system_info_struct);
int fwif_color_setCon_Bri_Color_Tint_tv006_AI(unsigned char display, _system_setting_info *system_info_struct);
void fwif_color_set_YUV2RGB_tv006(unsigned char src_idx, unsigned char display, unsigned char which_table, unsigned char drvSkip_Flag_coefByY, unsigned char drvSkip_Flag_uvOffset);
char fwif_color_set_DSE_tv006(unsigned char display, VPQ_SAT_LUT_T *satLUT);
int fwif_color_update_YUV2RGB_CSC_tv006(unsigned char display);
#if 0 /* no more use because may confuse when code sync YUV2RGB matrix*/
char fwif_color_set_SaturationLUT_tv006(unsigned char display, VPQ_SAT_LUT_T *satLUT);
char fwif_color_get_SaturationLUT_tv006(unsigned char display, VPQ_SAT_LUT_T *satLUT);
#endif /* no more use because may confuse when code sync YUV2RGB matrix*/
char fwif_color_access_SaturationLUT_tv006(unsigned char display, VPQ_SAT_LUT_T *satLUT, ACCESS_MODE mode);
char fwif_color_access_Total_Sat_Gain_tv006(unsigned char display, VPQ_SAT_LUT_T *satLUT, ACCESS_MODE mode);
char fwif_color_access_Sat_Gain_tv006(unsigned char display, VPQ_SAT_LUT_T *satLUT, ACCESS_MODE mode);
char fwif_color_access_apl_color_gain_tv006(unsigned char display, int* pAplColorGain, ACCESS_MODE mode);

void fwif_color_Set_NR_Table_tv006(RTK_NOISE_REDUCTION_T *ptr);
RTK_NOISE_REDUCTION_T *fwif_color_Get_NR_Table_tv006(void);
void fwif_color_set_DB_NR_tv006(short *ptr ); //for rtice use

unsigned char fwif_color_GetDNR_tv006(void);
void fwif_color_SetDNR_tv006(unsigned char level);
void fwif_color_Set_NR_Input_tv006(unsigned char input);
unsigned char fwif_color_get_NR_Input_tv006(void);
void fwif_color_set_PQA_motion_threshold_TV006(void);
void fwif_color_get_PQA_motion_threshold_TV006(void);

void fwif_color_set_LocalContrast_table_TV006(CHIP_LOCAL_CONTRAST_T *param);
void fwif_color_set_LocalContrast_ToneMappingXY_SlopePoint_TV006(CHIP_LOCAL_CONTRAST_T *param);

void fwif_color_Set_MPEGNR_Table_tv006(RTK_MPEG_NOISE_REDUCTION_T *ptr);
RTK_MPEG_NOISE_REDUCTION_T *fwif_color_Get_MPEGNR_Table_tv006(void);
unsigned char fwif_color_MPEGDNR_tv006(void);
void fwif_color_Set_MPEGNR_tv006(unsigned char level);
void fwif_color_Set_MNR_Enable_Check_tv006(void);

void fwif_color_set_sharpness_table_TV006(CHIP_SHARPNESS_UI_T *tCHIP_SHARPNESS_UI_T, CHIP_SR_UI_T *ptCHIP_SR_UI_T);
void fwif_color_Get_sharpness_table_TV006(short *xTmp);

void fwif_color_set_sharpness_table_init_TV006(unsigned char src_idx, unsigned char value);
unsigned char fwif_TwoParts_Linear_Interpolation(Sharpness_OSD_Remap_Curve *ptr, unsigned char value);
void fwif_color_set_SharpnessFilterCoef_for_3D(CHIP_SHARPNESS_UI_T *CHIPShpTable, CHIP_SHARPNESS_UI_T *CHIPShpTable3DCorrection);
unsigned char  fwif_color_set_dcc_Curve_Write_tv006(signed int *sAccLUT);
unsigned char fwif_color_set_dcc_FreshContrast_tv006(CHIP_DCC_T *ptr);
unsigned char  fwif_color_set_bypass_dcc_Curve_Write_tv006(void);
void fwif_color_set_iDcti_dDcti_tv006(unsigned char i_dcti_val, unsigned char d_dcti_val, CHIP_CTI_T *set_cti_db);
void fwif_color_set_Vdcti_tv006(unsigned char v_dcti_val, CHIP_CTI_T *set_cti_db);
void fwif_color_set_dLti_tv006(unsigned char d_lti_val, CHIP_HLTI_T *H_LTI_db, CHIP_VLTI_T *V_LTI_db);

void fwif_color_set_MultiBand_Peaking_tv006(unsigned char multiBand_val, CHIP_MULTIBAND_PEAKING_HPF_T *mb_hpf, CHIP_MULTIBAND_PEAKING_UI_T* mb_UI);
void fwif_color_set_MultiBand_SU_Peaking_tv006(unsigned char multiBand_val, CHIP_MULTIBAND_PEAKING_HPF_T *mb_hpf, CHIP_MULTIBAND_PEAKING_UI_T* mb_UI);

void fwif_color_set_inv_gamma_encode_tv006(void);
void fwif_color_set_cds_table_tv006(unsigned char table_idx);
void fwif_color_set_DCC_Init_tv006(void);
/*unsigned char fwif_color_set_Gamut_3D_Lut_N9_to_N17_tv006(GAMUT_3D_LUT_9x9x9_T *pLut);*/
unsigned char fwif_color_set_Gamut_3D_Lut_buffer_tv006(GAMUT_3D_LUT_17x17x17_T *pLut);
unsigned char fwif_color_set_Gamut_3D_Lut_ext_buffer_tv006(GAMUT_3D_LUT_17x17x17_T *pLut);
char fwif_color_set_DDomain_3dLUT_16_TV006(unsigned short *p, unsigned char enable);
void fwif_color_set_color_temp_tv006(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con,
	short Red_Bri, short Green_Bri, short Blue_Bri, unsigned char waitSync);

char fwif_color_set_bt2020_Handler_tv006(unsigned char display, unsigned char Enable_Flag, unsigned char bt2020_Mode);
unsigned char fwif_color_vpq_stop_ioctl(VPQ_IOCTL_STOP_RUN id, ACCESS_MODE mode, unsigned char isStop);

unsigned char fwif_color_get_LD_GetAPL_TV006(UINT16* BLValue);
void fwif_color_SetDataFormatHandler_tv006(void);

//void fwif_color_set_HDR_Dolby_mode_by_HFC(UINT32 V_top_index, UINT8 apply);
UINT8 fwif_color_DM2_EOTF_OETF_3D_LUT_by_HFC(UINT8 HFC_bit, HDR_flow_control_timing HDR_timing);
UINT8 fwif_color_DM2_HDR1_2_EOTF_OETF_3D_LUT_by_HFC(UINT8 HFC_bit, HDR_sub_flow_control_timing HDR_timing);


#if 0
void fwif_color_SetDataFormatHandler_tv006_Seamless(unsigned char HDR_Mode);
#endif
void fwif_color_3d_lut_init_tv006(void);
unsigned char fwif_color_3d_lut_access_index(ACCESS_MODE access, unsigned char setIdx);

void fwif_color_set_cinema_mode_en(unsigned char bEn);
unsigned char fwif_color_get_cinema_mode_en(void);
int fwif_color_set_dcc_force_write_flag_RPC(unsigned sendRPC, unsigned int force_write);
void fwif_color_set_out_gamma_encode_tv006(void);
void fwif_color_set_SiW_ModeChange_TV006(UINT8 *pFwData, UINT32 size);
#ifdef CONFIG_SCALER_ENABLE_V4L2
void fwif_color_set_Mplus_ModeChange_TV006(enum v4l2_ext_vbe_mplus_mode mode_sel);
#endif
char fwif_color_set_DM2_EOTF_TV006(UINT32* EOTF_table32, unsigned char check_err);
char fwif_color_set_DM2_HDR1_2_EOTF_TV006(UINT32* EOTF_table32, unsigned char check_err);
char fwif_color_set_DM2_OETF_TV006(UINT16* OETF_table16, unsigned char check_err);
char fwif_color_set_DM2_HDR1_2_OETF_TV006(UINT16* OETF_table16, unsigned char check_err);
char fwif_color_set_DM2_OETF_Conv_TV006(UINT16* in_OETF_table16, UINT16* out_OETF_table16, unsigned char tbl_sel);
char fwif_color_set_DM2_ToneMapping_TV006(UINT16* TM_Table);
char fwif_color_set_DM2_HDR1_2_ToneMapping_TV006(UINT16* TM_Table);
UINT8 fwif_color_get_DM2_HDR_histogram_TV006(UINT32* HDR_info);
UINT8 fwif_color_get_DM_HDR10_enable_TV006(void);
void hdr_resume(void);
void fwif_color_set_HDR_TV006(void);
void fwif_color_set_HDR10_runmode(unsigned char display);
char fwif_color_Wait_3dLUT_Dimension_DB_done(void);
char fwif_color_Wait_HDR1_2_3dLUT_Dimension_DB_done(void);
char fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(void *p, unsigned char check_err);
char fwif_color_set_DM_HDR_3dLUT_17x17x17_16_TV006(void *p, unsigned char check_err);
char fwif_color_set_DM_HDR1_2_3dLUT_17x17x17_16_TV006(void *p, unsigned char check_err);
void fwif_color_set_HDR_Dolby_mode_Enable(unsigned char display, unsigned char HDR_type);
char fwif_color_set_DM_HDR_3dLUT_INDEX_TV006(unsigned int *pTBL);
char fwif_color_set_DM_HDR1_2_3dLUT_INDEX_TV006(unsigned int *pTBL);
void fwif_color_set_DM2_Hist_AutoMode_Enable(unsigned char En);
unsigned char fwif_color_get_DM2_Hist_AutoMode_Enable_Flag(void);
unsigned char fwif_color_get_DM2_Hist_AutoMode_Counter(void);
void fwif_color_set_DM2_Hist_AutoMode_Enable_Flag(unsigned char En);
void fwif_color_set_DM2_Hist_AutoMode_Counter(unsigned char Val);
unsigned char fwif_color_DM2_Hist_AutoMode_ch1_sta_ISR(void);
unsigned char fwif_color_DM2_Hist_AutoMode_ch1_end_ISR(void);
unsigned int* fwif_color_DM2_Hist_AutoMode_TBL_Get(unsigned char idx);
unsigned char fwif_color_DM2_Sat_Hue_Hist_ch1_sta_ISR(void);
char fwif_color_DM2_Sat_Hist_TBL_Get(unsigned int* pSatTbl);
char fwif_color_DM2_Hue_Hist_TBL_Get(unsigned int* pHueTbl);

void fwif_color_set_LD_Backlight_Decision_Replace_TV006(KADP_LED_BACKLIGHT_DECISION *LED_DB_LUT_BACKLIGHT_DECISION, DRV_LD_Backlight_Decision *LD_Backlight_Decision);
void fwif_color_set_LD_Spatial_Filter_Replace_TV006(KADP_LED_SPATIAL_FILTER *LED_DB_LUT_SPATIAL_FILTER, DRV_LD_Spatial_Filter *LD_Spatial_Filter);
void fwif_color_set_LD_Temporal_Filter_Replace_TV006(KADP_LED_TEMPORAL_FILTER *LED_DB_LUT_TEMPORAL_FILTER, DRV_LD_Temporal_Filter *LD_Temporal_Filter);
void fwif_color_set_LD_Spatial_Remap_Replace_TV006(KADP_LED_REMAP_CURVE *LED_DB_LUT_REMAP_CURVE, DRV_LD_Spatial_Remap *LD_Spatial_Remap);
void fwif_color_set_LD_Data_Compensation_NewMode_2DTable_Replace_TV006(KADP_LED_DATA_COMP *LED_DB_LUT_DATA_COMP, DRV_LD_Data_Compensation_NewMode_2DTable *LD_Data_Compensation_NewMode_2DTable);
void fwif_color_set_LDSetDBLUT_TV006(KADP_LED_DB_LUT_T *LED_DB_LUT);
void fwif_color_set_output_gamma_4CHDGA_TV006 (UINT32 *pRedGammaTable, UINT32 * pGreenGammaTable, UINT32 * pBlueGammaTable, UINT32 * pWhiteGammaTable, UINT16 nTableSize);
void fwif_color_set_output_gamma_PTG_TV006 (unsigned char bCtrl, unsigned short R_value,unsigned short G_value,unsigned short B_value,unsigned short W_value);
#if 0	/* juwen, move to "tc_hdr", elieli */
void fwif_color_TC_Enable(UINT8 En);
#endif
void fwif_color_set_SetBOERGBWBypass_TV006(unsigned char BOE_EN);
char fwif_color_DM2_EOTF_OETF_3D_24x24x24_LUT_Handler(HAL_VPQ_HDR_MODE_T HDR_MODE);
void fwif_color_MplusSet(unsigned short *pRegisterSet, unsigned char panelMaker);
void fwif_color_MplusGet(unsigned short *pRegisterSet, unsigned char panelMaker);
void fwif_color_SetFrameGainLimit_TV006(unsigned short nFrameGainLimit);
void fwif_color_GetFrameGainLimit_TV006(unsigned short *nFrameGainLimit);
void fwif_color_SetPixelGainLimit_TV006(unsigned short nPixelGainLimit);
void fwif_color_GetPixelGainLimit_TV006(unsigned short *nPixelGainLimit);
void fwif_color_set_fcic_TV006(unsigned int *u32pTSCICTbl, unsigned int u32Tscicsize, unsigned char *u8pControlTbl, unsigned int u32Ctrlsize, unsigned char bCtrl);
void fwif_colorMap_sRGB_SetMatrix_TV006(unsigned char display, short (*sRGB_Matrix)[3], unsigned char forceUpdate);
void fwif_set_Gamut_Matrix_TV006(unsigned char display, short (*gamut_Matrix)[3]);
void fwif_color_Set_De_Contour_tv006(RTK_DECONTOUR_T *level);
#ifdef CONFIG_SCALER_ENABLE_V4L2
void fwif_color_set_ICM_tv006_null_case(struct v4l2_ext_cm_ui_status cmuiInfo);
#endif


void fwif_color_set_BFI_En_by_InputFrameRate(unsigned char bEn);

unsigned char fwif_color_get_BFI_En_by_InputFrameRate(void);

void fwif_color_set_BFI_En_by_UI(unsigned char bEn);

unsigned char fwif_color_get_BFI_En_by_UI(void);

void fwif_color_set_BFI_En(unsigned char bEn);

unsigned char fwif_color_get_BFI_En(void);



#ifdef __cplusplus
}
#endif

#endif /* __SCALER_COLOR_TV006_H__*/


