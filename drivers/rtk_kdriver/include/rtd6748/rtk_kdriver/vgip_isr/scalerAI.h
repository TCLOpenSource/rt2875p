#ifndef _SCALER_AI_H_
#define _SCALER_AI_H_


#include "gal/rtk_se_export.h"
#include <tvscalercontrol/vip/icm.h>
#include <tvscalercontrol/vip/ai_pq.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <ioctrl/ai/ai_cmd_id.h>
/*******************************************************************************
*Definitions*
******************************************************************************/
#define I3DMA_MULTICROP_MODE_AS_SUB			1
#define I3DMA_MULTICROP_MODE_AUTO			2
#define I3DMA_MULTICROP_MODE				I3DMA_MULTICROP_MODE_AS_SUB

#define ai_calMode_offset 0
#define ai_calMode_gain 1

//viptable 1261 column 1
typedef enum{
	AI_MODE_OFF=0,		//0 AI disable
	AI_MODE_ON,			//1 AI enable ( enable i3dma capture)
	AI_MODE_DBG,		//2 AI enable ( enable i3dma capture) and show more debug message at kernel
}AI_MODE;

//viptable 1261 column 2
typedef enum{
	FACE_MODE_OFF=0,	//0 face detect disable
	FACE_MODE_ON,		//1 face detect enable
	FACE_MODE_DEMO,		//2 face detect enable and display debug osd
}FACE_MODE;

//viptable 1261 column 3
typedef enum{
	SQM_MODE_OFF=0,		//0 sqm detect disable
	SQM_MODE_ON,		//1 sqm detect enable
	SQM_MODE_DEMO,		//2 sqm detect enable and display debug osd
	SQM_MODE_DBG,		//3 sqm detect enable and display confidence percentage on debug osd
}SQM_MODE;

//viptable 1261 column 4
typedef enum{
	CONT_MODE_OFF=0,	//0 content detect disable
	CONT_MODE_ON,		//1 content detect enable
	CONT_MODE_DEMO,		//2 content detect enable and display debug osd
	CONT_MODE_DBG,		//3 content detect enable and display confidence percentage on debug osd
}CONT_MODE;

//viptable 1261 column 5
typedef enum{
	GENRE_MODE_OFF=0,	//0 genre detect disable
	GENRE_MODE_ON,		//1 genre detect enable
	GENRE_MODE_DEMO,	//2 genre detect enable and display debug osd
	GENRE_MODE_DBG,		//3 genre detect enable and display confidence percentage on debug osd
}GENRE_MODE;

//viptable 1261 column 6
typedef enum{
	DEPTH_MODE_OFF=0,				//0 depthmap detect disable
	DEPTH_MODE_ON,					//1 depthmap detect enable
	DEPTH_MODE_DEMO,				//2 depthmap detect enable and display result on partial debug osd
	DEPTH_MODE_ON_WITH_FULL_OSD,	//3 depthmap detect enable and display result on full debug osd
}DEPTH_MODE;

//viptable 1261 column 7
typedef enum{
	OBJ_MODE_OFF=0,					//0 object segment detect disable
	OBJ_MODE_ON,					//1 object segment detect enable
	OBJ_MODE_DEMO,					//2 object segment detect enable and display result on full debug osd
	OBJ_MODE_ON_WITH_PART_OSD,		//3 object segment detect enable and display result on partial debug osd
}OBJ_MODE;

//viptable 1261 column 10
typedef enum{
	LOGO_MODE_OFF=0,				//0 logo detect disable
	LOGO_MODE_ON,					//1 logo detect enable
	LOGO_MODE_DEMO,					//2 logo detect enable and display on debug osd
}LOGO_MODE;

//viptable 1261 column 11
typedef enum{
       NOISE_MODE_OFF=0,       //0 NOISE detect disable
       NOISE_MODE_ON,          //1 NOISE detect enable
       NOISE_MODE_DEMO,        //2 NOISE detect enable and display debug osd
       NOISE_MODE_DBG,         //3 NOISE detect enable and display confidence percentage on debug osd
}NOISE_MODE;

/*******************************************************************************
 * Structure
 ******************************************************************************/

typedef struct
{
	unsigned int top;
	unsigned int bot;
	unsigned int left;
	unsigned int right;
} crop_range;

// lesley 0828
typedef struct
{
	int canvas_w;
	int canvas_h;
	int num;
	unsigned int color[6];
	KGAL_RECT_T ai_block[6];
} SE_DRAW;

// end lesley 0828

typedef struct {
	unsigned int dst_x;
	unsigned int dst_y;
	unsigned int dst_w;
	unsigned int dst_h;
	unsigned int dst_phyaddr;
	unsigned int dst_phyaddr_uv;
	unsigned int dst_fmt;
	unsigned int src_x;
	unsigned int src_y;
	unsigned int src_w;
	unsigned int src_h;
	unsigned int src_phyaddr;
	unsigned int src_phyaddr_uv;
	unsigned int src_fmt;
	unsigned int src_pitch_y;
	//unsigned int src_pitch_c;
	crop_range src_crop_range;
} SE_NN_info;



// lesley 0910
typedef struct {
	int dcc_enhance;
	int dcc_curve[8];

	int icm_curve[8];

	int shp_edge_gain_pos;
	int shp_edge_gain_neg;
	int shp_texture_gain_pos;
	int shp_texture_gain_neg;
	int shp_ver_edge_gain_pos;
	int shp_ver_edge_gain_neg;
	int shp_ver_texture_gain_pos;
	int shp_ver_texture_gain_neg;

} DB_AI_RTK;
// end lesley 0910

// lesley 1002_1
typedef struct {
	int x;
	int y;
	int h_norm;
	int s_norm;
	int i_norm;
	int wt;
} TOOL_AI_INFO_ICM;

typedef struct {
	TOOL_AI_INFO_ICM icm[6];
} TOOL_AI_INFO;
// end lesley 1002_1

// lesley 1007
typedef struct {
	int hue_target_lo3; //-60~300
	int hue_target_lo2; //-60~300
	int hue_target_lo1; //-60~300
	int hue_target_hi1; //-60~300
	int hue_target_hi2; //-60~300
	int hue_target_hi3; //-60~300
    int hue_target_lo2_ratio; //0~100
    int hue_target_hi2_ratio; //0~100
	int h_adj_th_p_norm; //0~360
	int h_adj_th_n_norm; //0~360
} FW_AI_ICM_hue;

typedef struct {
	int sat_target_lo3; //0~100
	int sat_target_lo2; //0~100
	int sat_target_lo1; //0~100
	int sat_target_hi1; //0~100
	int sat_target_hi2; //0~100
	int sat_target_hi3; //0~100
    int sat_target_lo2_ratio; //0~100
    int sat_target_hi2_ratio; //0~100
	int s_adj_th_p_norm; //0~100
	int s_adj_th_n_norm; //0~100
} FW_AI_ICM_sat;

typedef struct {
	int val_target_lo1; // 0~255
	int val_target_hi1; // 0~255
	int val_target_lo2_ratio; // 0~100
	int val_target_hi2_ratio; // 0~100
	int v_adj_th_p_norm; // 0~255
	int v_adj_th_n_rorm; // 0~255
} FW_AI_ICM_val;

typedef struct {
	FW_AI_ICM_hue hue_tune;
	FW_AI_ICM_sat sat_tune;
	FW_AI_ICM_val val_tune;

	int dcc_user_curve[32];
} TOOL_AI_TUNING;

typedef struct {
	FW_AI_ICM_hue hue_tune;
	FW_AI_ICM_sat sat_tune;
	FW_AI_ICM_val val_tune;

} DRV_AI_Tune_ICM_table;

typedef struct {
	int dcc_user[32];
} DRV_AI_Tune_DCC_table;

// end lesley 1007

#define AI_CAPIMG_RECORD_FRAME (128)
typedef struct AI_CAPTURE_IMG_RECORD {
	// the reference counter for model to capture image
	unsigned int ResoluationCount[AI_CAPIMG_RECORD_FRAME];
	// model capture image situation
	unsigned int Genre[AI_CAPIMG_RECORD_FRAME];		// (2) VIP_NN_BUFFER_224GENRE - CROP
	unsigned int SQM[AI_CAPIMG_RECORD_FRAME];		// (3) VIP_NN_BUFFER_240X135CROP - CROP
	unsigned int Scene[AI_CAPIMG_RECORD_FRAME];		// (0) VIP_NN_BUFFER_224FULL
	unsigned int UltraFace[AI_CAPIMG_RECORD_FRAME];	// (1) VIP_NN_BUFFER_320FULL
	unsigned int DepthMap[AI_CAPIMG_RECORD_FRAME];	// (4) VIP_NN_BUFFER_192FULL
	unsigned int ObjectSeg[AI_CAPIMG_RECORD_FRAME]; // (5) VIP_NN_BUFFER_256FULL
	unsigned int Semantic[AI_CAPIMG_RECORD_FRAME];	// (6) VIP_NN_BUFFER_480FULL
	unsigned int Noise[AI_CAPIMG_RECORD_FRAME];             // (7) VIP_NN_BUFFER_240X136CROP - CROP

	// the 90k clock value
	unsigned int Time[AI_CAPIMG_RECORD_FRAME];
	unsigned int PreCropTime[AI_CAPIMG_RECORD_FRAME];
	unsigned int CurCropTime[AI_CAPIMG_RECORD_FRAME];
	unsigned int PreFullTime[AI_CAPIMG_RECORD_FRAME];
	unsigned int CurFullTime[AI_CAPIMG_RECORD_FRAME];
} AI_CAPTURE_IMG_RECORD_T; // 128*13 = 1664

typedef struct _VIP_NN_BUFFER_LEN{
	unsigned int data_len;
	unsigned int flag_len;
	unsigned int info_len;
}VIP_NN_BUFFER_LEN;

#if 1
#define HUESEGMAX	60
#define SATSEGMAX	12
#define ITNSEGMAX	8
#define LC_Curve_ToneM_PointSlope 256

typedef enum{
	PQ_SCENE_Autumn=0,//0
	PQ_SCENE_Desert,
	PQ_SCENE_Forest,
	PQ_SCENE_Grass,
	PQ_SCENE_Mountains,

	PQ_SCENE_Ocean,// 5
	PQ_SCENE_Other,
	PQ_SCENE_People,
	PQ_SCENE_Sky,
	PQ_SCENE_Snowfield,
	PQ_SCENE_Sports,//10

	PQ_SCENE_Flower,
	PQ_SCENE_Highway,
	PQ_SCENE_Night,
	PQ_SCENE_Sunrise,
	PQ_SCENE_Volcano,
	PQ_SCENE_Building,
	PQ_SCENE_Food,//17

	PQ_SCENE_TYPE_NUM_TV030,
} PQ_SCENE_TYPE_TV030;

typedef struct {
	unsigned char ai_iEdgeSmooth_calMode;
	int mixer_weight;
	 char IESM_3tap_gain;// Merlin4
	 char IESM_5tap_gain;// Merlin4

} iEdgeSmooth_Offset_table;
//-------------------------------------
typedef struct {
	unsigned char RTNR_y_enable;
	unsigned char RTNR_c_enable;
} RTNR_Offset_Ctrl;
typedef struct {
	unsigned char ai_rtnr_y_k_calMode;
	char k0;
	char k1;
	char k2;
	char k3;
	char k4;
	char k5;
	char k6;
	char k7;
	char k8;
	char k9;
	char k10;
	char k11;
	char k12;
	char k13;
	char k14;
	char k15;
} RTNR_y_k_Offset_TBL;
typedef struct {
	unsigned char ai_rtnr_c_k_calMode;
        char  cp_temporal_c_k0:3;
        char  cp_temporal_c_k1:3;
        char  cp_temporal_c_k2:3;
        char  cp_temporal_c_k3:3;
        char  cp_temporal_c_k4:3;
        char  cp_temporal_c_k5:3;
        char  cp_temporal_c_k6:3;
        char  cp_temporal_c_k7:3;
} RTNR_c_k_Offset_TBL;
typedef struct {
	unsigned char ai_rtnr_Y_th_calMode;
	 char cp_temporalthly0;
	 char cp_temporalthly1;
	 char cp_temporalthly2;
	 char cp_temporalthly3;
	 char cp_temporalthly4;
	 char cp_temporalthly5;
	 char cp_temporalthly6;
	 char cp_temporalthly7;
	 char cp_temporalthly8;
	 char cp_temporalthly9;
	 char cp_temporalthly10;
	 char cp_temporalthly11;
	 char cp_temporalthly12;
	 char cp_temporalthly13;
	 char cp_temporalthly14;
} RTNR_Y_th_Offset_TBL;
typedef struct {
	unsigned char ai_RTNR_C_th_calMode;
	 char cp_temporalthlc0;
	 char cp_temporalthlc1;
	 char cp_temporalthlc2;
	 char cp_temporalthlc3;
	 char cp_temporalthlc4;
	 char cp_temporalthlc5;
	 char cp_temporalthlc6;
} RTNR_C_th_Offset_TBL;
typedef struct {
	RTNR_Offset_Ctrl ai_TNR_Offset_Ctrl;
	RTNR_y_k_Offset_TBL ai_RTNR_y_k_Offset_TBL;
	RTNR_c_k_Offset_TBL ai_RTNR_c_k_Offset_TBL;
	RTNR_Y_th_Offset_TBL ai_RTNR_th_Offset_TBL;
	RTNR_C_th_Offset_TBL ai_RTNR_C_th_Offset_TBL;
} RTNR_Offset_table;
//-------------------------------------
typedef struct {
	unsigned char cp_spatialenablec;
	unsigned char cp_spatialenabley;
	unsigned char curvemappingmode_en;
} SNR_Offset_Ctrl;

typedef struct {
	unsigned char ai_SNR_Curve_Mapping_calMode;
	 char w_0;
	 char w_1;
	 char w_2;
	 char w_3;
	 char w_4;
	 char w_5;
	 char w_6;
	 char w_7;
	 char w_8;
} SNR_Curve_Mapping_Offset_TBL;
typedef struct {
	SNR_Offset_Ctrl ai_SNR_Offset_Ctrl;
	SNR_Curve_Mapping_Offset_TBL ai_SNR_Curve_Mapping_w1_Offset_TBL;
	SNR_Curve_Mapping_Offset_TBL ai_SNR_Curve_Mapping_w2_Offset_TBL;
} SNR_Offset_table;
//-------------------------------------
typedef struct {
	unsigned char mosquito_detect_en;
	unsigned char mosquitonr_vertical_en;
	unsigned char edge_lpf_en;
	unsigned char SNR_maxmin_range;
} MNR_Offset_Ctrl;
typedef struct {
	unsigned char ai_SNR_mnr_calMode;
	short SNR_mnr_edgethd1;
	short SNR_mnr_edgethd2;
	short SNR_mnr_edgethd3;
	short SNR_mnr_edgethd4;
	short SNR_mnr_edgethd5;
	short SNR_mnr_edgethd6;
	short SNR_mnr_edgethd7;
	short SNR_mnr_edgethd8;
	short SNR_mosquitonr_nlevel_positive_offset;
	short SNR_mosquitonr_nlevel_positive_shift;
	short SNR_mosquitonr_nlevel_negative_offset;
	short SNR_mosquitonr_nlevel_negative_shift;
	short SNR_mnr_nlevel_maxfrac_mode;
	short SNR_mosquitonr_nlevel_mid_offset;
	short SNR_mosquitonr_nlevel_mid_shift;
} SNR_mnr_edgeth_Offset_TBL;
typedef struct {
	MNR_Offset_Ctrl ai_MNR_Offset_Ctrl;
	SNR_mnr_edgeth_Offset_TBL ai_SNR_mnr_edgeth_Offset_TBL;

} MNR_Offset_table;
//-------------------------------------
typedef struct {
	unsigned char ai_Dirsu_pixel_calMode;

	 char AngDiff_Lowbnd;
	 char AngDiff_Step;
	 char Mag_Lowbnd;
	 char Mag_Step;

	 char Conf_Step;
	 char Conf_Offset;
	 char Conf_Lowbnd;
} Dirsu_pixel_Offset_table;
typedef struct {
	unsigned char ai_Dirsu_Angle_transition_calMode;

	 char UZU_TransANG_sec0;
	 char UZU_TransANG_sec1;
	 char UZU_TransANG_sec2;
	 char UZU_TransANG_sec3;

	 char UZU_TransANG_step0;
	 char UZU_TransANG_step1;
	 char UZU_TransANG_step2;
	 char UZU_TransANG_step3;

} Dirsu_Angle_transition_Offset_table;
typedef struct {
	Dirsu_pixel_Offset_table ai_Dirsu_pixel_Offset_Ctrl;
	Dirsu_Angle_transition_Offset_table ai_Dirsu_Angle_transition_Offset_TBL;
} Directional_SU_Offset_table;
//-------------------------------------
typedef struct {
	unsigned char dlti_en;
	unsigned char dlti_v_en;
	unsigned char dlti_h_en;
	unsigned char ai_DLTI_calMode;
} DLTI_Ctrl_Offset_table;
typedef struct {
	 short Tnoff0;
	 short Tnoff1;
	 short Tnoff2;
	 short Tnoff3;
	 short Tnoff4;
	 short Tnoff5;
	 short Tnoff6;
	 short Tnoff7;
	 short Tnoff8;
	 short Tnoff9;
	 short Tnoff10;
	 short Tnoff11;
	 short Tnoff12;
	 short Tnoff13;
	 short Tnoff14;
} DLTI_Tnoff;
typedef struct {
	DLTI_Ctrl_Offset_table ai_DLTI_Ctrl_Offset_Ctrl;
	DLTI_Tnoff DLTI_H_Tnoff;
	DLTI_Tnoff DLTI_V_Tnoff;
} DLTI_Offset_table;
//-------------------------------------
typedef struct {
	 int lc_tmap_slope_unit;
	 int lc_tmap_blend_factor;
} LC_ToneMapping_Blend_offset;
typedef struct {
	unsigned char m_lc_comp_en;
	LC_ToneMapping_Blend_offset LC_ToneMapping_Blend;
	unsigned char lc_local_sharp_en;
	unsigned char lc_tone_mapping_en;
} LC_Global_Ctrl_Offset_table;
typedef struct {
	unsigned char ai_LC_Gain_Curve_calMode;
	DRV_LC_Shpnr_Gain1st LC_Shpnr_Gain1st;
	DRV_LC_Shpnr_Gain2nd LC_Shpnr_Gain2nd;
} LC_Gain_Curve_Offset_table;
typedef struct {
	LC_Global_Ctrl_Offset_table ai_LC_Ctrl_Offset_Ctrl;
	LC_Gain_Curve_Offset_table ai_LC_Gain_Curve_Offset_table;
} LC_Offset_table;
//-------------------------------------
typedef struct {
	unsigned char Dcti_en;
	unsigned char ai_I_DCTI_calMode;
	//Main Control
	 char uvgain;
	 char uvgain_v;
	//Range mode
	 char maxlen;
	 char psmth;
	//Filter mode
	 char th_coring;
	 char maxminlen;
	//Blending Weight
	 char th_tran;
	 char th_tran_v;
	 char th_gtran;
	 char th_gtran_v;
	 char th_smooth;
	 char th_smooth_v;
	 char rate_smooth;
	 char rate_smooth_v;
	 char th_stair;
	 char th_stair_v;
} I_DCTI_Offset_table;

typedef struct {
	unsigned char M_dcti_en;
	unsigned char dcti_h_en;
	unsigned char dcti_v_en;
	unsigned char ai_D_DCTI_calMode;
	//Main Control
	char uvgain;
	//Range mode
	char maxlen;
	char	psmth;
	//Filter mode
	char th_coring;
	char maxminlen;
	//Blending Weight
	char th_tran;
	char th_gtran;
	char rate_smooth;
	//vertical
	char uvgain_ver;
} D_DCTI_Offset_table;

typedef struct _YUV2RGB_UV_Offset {
	 char Uoffset[VIP_YUV2RGB_Y_Seg_Max];
	 char Voffset[VIP_YUV2RGB_Y_Seg_Max];
	 char Ustep[VIP_YUV2RGB_Y_Seg_Max];
	 char Vstep[VIP_YUV2RGB_Y_Seg_Max];
} YUV2RGB_UV_Offset;

typedef struct {
	unsigned char UVoffsetByY_main_en;
	unsigned char ai_UVoffsetByY_calMode;
	YUV2RGB_UV_Offset UV_Offset;
}UVOffsetbyY_Offset_table;

typedef struct
{
	int ai_x_center_0;
	int ai_y_center_0;
	int ai_range_0;
	int ai_octa_tang_dir0;
	int ai_octa_tang_mode0;
	int ai_blending_0_inside_ratio;
	int ai_blending_0_ratio_0;
	int ai_blending_0_ratio_1;
	int ai_blending_0_ratio_2;
	int ai_blending_0_ratio_3;
}
AI_DCC_Face0_info;


typedef struct
{
	int ai_x_center_1;
	int ai_y_center_1;
	int ai_range_1;
	int ai_octa_tang_dir1;
	int ai_octa_tang_mode1;
	int ai_blending_1_inside_ratio;
	int ai_blending_1_ratio_0;
	int ai_blending_1_ratio_1;
	int ai_blending_1_ratio_2;
	int ai_blending_1_ratio_3;
}
AI_DCC_Face1_info;

typedef struct
{
	int ai_x_center_2;
	int ai_y_center_2;
	int ai_range_2;
	int ai_octa_tang_dir2;
	int ai_octa_tang_mode2;
	int ai_blending_2_inside_ratio;
	int ai_blending_2_ratio_0;
	int ai_blending_2_ratio_1;
	int ai_blending_2_ratio_2;
	int ai_blending_2_ratio_3;
}
AI_DCC_Face2_info;

typedef struct
{
	int ai_x_center_3;
	int ai_y_center_3;
	int ai_range_3;
	int ai_octa_tang_dir3;
	int ai_octa_tang_mode3;
	int ai_blending_3_inside_ratio;
	int ai_blending_3_ratio_0;
	int ai_blending_3_ratio_1;
	int ai_blending_3_ratio_2;
	int ai_blending_3_ratio_3;
}
AI_DCC_Face3_info;

typedef struct
{
	int ai_x_center_4;
	int ai_y_center_4;
	int ai_range_4;
	int ai_octa_tang_dir4;
	int ai_octa_tang_mode4;
	int ai_blending_4_inside_ratio;
	int ai_blending_4_ratio_0;
	int ai_blending_4_ratio_1;
	int ai_blending_4_ratio_2;
	int ai_blending_4_ratio_3;
}
AI_DCC_Face4_info;

typedef struct
{
	int ai_x_center_5;
	int ai_y_center_5;
	int ai_range_5;
	int ai_octa_tang_dir5;
	int ai_octa_tang_mode5;
	int ai_blending_5_inside_ratio;
	int ai_blending_5_ratio_0;
	int ai_blending_5_ratio_1;
	int ai_blending_5_ratio_2;
	int ai_blending_5_ratio_3;
}
AI_DCC_Face5_info;

typedef struct {
	AI_DCC_global_setting 	dcc_ai_global;
	AI_DCC_Face0_info 	dcc_ai_face0_info ;
	AI_DCC_Face1_info  	dcc_ai_face1_info;
	AI_DCC_Face2_info 	dcc_ai_face2_info;
	AI_DCC_Face3_info  	dcc_ai_face3_info;
	AI_DCC_Face4_info 	dcc_ai_face4_info;
	AI_DCC_Face5_info 	dcc_ai_face5_info;
} AI_DCC_Ctrl_table_TV030;


typedef struct {
	DRV_AI_Tune_ICM_table AI_Tune_ICM_TBL[PQ_SCENE_TYPE_NUM_TV030];
	AI_DCC_Ctrl_table_TV030 AI_DCC_TBL[PQ_SCENE_TYPE_NUM_TV030];
	ICM_RGB_Offset_table tICM_RGB_Offset[PQ_SCENE_TYPE_NUM_TV030];
	iEdgeSmooth_Offset_table iEdgeSmooth_Offset[PQ_SCENE_TYPE_NUM_TV030];
	RTNR_Offset_table RTNR_Offset[PQ_SCENE_TYPE_NUM_TV030];
	SNR_Offset_table SNR_Offset[PQ_SCENE_TYPE_NUM_TV030];
	MNR_Offset_table MNR_Offset[PQ_SCENE_TYPE_NUM_TV030];
	I_DCTI_Offset_table I_DCTI_Offset[PQ_SCENE_TYPE_NUM_TV030];
	Directional_SU_Offset_table Directional_SU_Offset[PQ_SCENE_TYPE_NUM_TV030];
	DLTI_Offset_table DLTI_Offset[PQ_SCENE_TYPE_NUM_TV030];
	D_DCTI_Offset_table D_DCTI_Offset[PQ_SCENE_TYPE_NUM_TV030];
	VIP_CDS_Table CDS_TBL_forAI[PQ_SCENE_TYPE_NUM_TV030];
	UVOffsetbyY_Offset_table UVOffsetbyY_Offset[PQ_SCENE_TYPE_NUM_TV030] ;
	LC_Offset_table LC_Offset[PQ_SCENE_TYPE_NUM_TV030];
	unsigned int LC_ToneMappingSlopePoint_Table_forAI[PQ_SCENE_TYPE_NUM_TV030][LC_Curve_ToneM_PointSlope];
} SLR_VIP_TABLE_AIPQ;
#endif

/*******************************************************************************
*functions
******************************************************************************/
void scalerAI_Init(void);
char fw_scalerip_reset_NN(void);
VIP_NN_CTRL* scalerAI_Access_NN_CTRL_STRUCT(void);

void scalerAI_preprocessing(void);
void scalerAI_get_NN_buffer(unsigned int *phyaddr_y, unsigned int *phyaddr_c, VIP_NN_BUFFER_INDEX index);
void scalerAI_set_NN_buffer(VIP_NN_BUFFER_INDEX bufIdx,int params);
void scalerAI_scene_PQ_target_get(int genre, int *ptr);
void scalerAI_scene_PQ_target_set(int genre, int *ptr);
void scalerAI_face_PQ_set(void);
void scalerAI_obj_info_get(AIInfo *info, unsigned char *NN_flag);
void scalerAI_postprocessing(void);
void AI_scene_print_target_table(int scene_type);
void scalerAI_execute_NN(void);
int AI_scene_get_top(int *pfProb, int **topClasses, int outputCount, int topNum);


// chen 0417
void AI_face_dynamic_control(AIInfo face_in[6], int scene_change, unsigned char NN_flag);
void AI_face_win_pos_predict(int faceIdx, AIInfo face);
void AI_face_ICM_blending_value(int faceIdx, AIInfo face);
void AI_face_dynamic_ICM_offset(int faceIdx, AIInfo face, int *h_adj_o, int *s_adj_o, int *v_adj_o); // lesley 0808
void AI_face_ICM_tuning(int faceIdx, AIInfo face);
// chen 0426
void AI_face_ICM_adjust_valuen(int hue_info, int sat_info, int val_info, int *hue_delta, int *sat_delta, int *val_delta, COLORELEM ICM_TAB_ACCESS[8][12][60]);

// chen 0429
void AI_face_DCC_blending_value(int faceIdx, AIInfo face);
// end chen 0429

// chen 0527
void AI_face_Sharp_blending_value(int faceIdx, AIInfo face);
//end chen 0527

void AI_face_Decont_blending_value(int faceIdx, AIInfo face);//mk2
extern void  drvif_color_get_dcc_adjust_value(int val_info , int *value_delta, unsigned int *sAccLUT); //get from   drvif_color_get_dcc_current_curve
// end chen 0417

// chen 0805
void AI_face_DCC_blending_value_global(int max_face_size);
// end chen 0805

// lesley 0815
void AI_face_Sharp_blending_value_global(void);
// end lesley 0815

// lesley 0821
void AI_face_ICM_blending_value_global(int max_face_size);
// end lesley 0821

// lesley 0911
void scalerAI_face_pq_off(void);
void scalerAI_face_pq_on(unsigned char mode, unsigned char dcValue);
// end lesley 0911

// lesley 1002_1
void drivef_tool_ai_info_set(int idx, int h_norm, int s_norm, int i_norm);
void drivef_tool_ai_info_get(TOOL_AI_INFO *ptr);
// end lesley 1002_1
void drivef_tool_ai_db_set(DB_AI_RTK *ptr);

int drvif_color_get_Genre_info(void);
int drvif_color_get_Scene_info(void);

#ifdef CONFIG_SCALER_ENABLE_V4L2 //#if 0 // V4L2_ERR
// lesley 1014
void drvif_color_get_DB_AI_DCC(CHIP_DCC_T *ptr);
void drvif_color_get_DB_AI_ICM(CHIP_CM_RESULT_T *v4l2_data);
void drvif_color_get_DB_AI_SHP(CHIP_SHARPNESS_UI_T *ptCHIP_SHARPNESS_UI_T);
void drvif_color_set_DB_AI_DCC(void);
void drvif_color_set_DB_AI_ICM(void);
void drvif_color_set_DB_AI_SHP(void);
// lesley 1014
#endif

// lesley 1016
void drivef_ai_tune_icm_set(DRV_AI_Tune_ICM_table *ptr);
void drivef_ai_tune_icm_get(DRV_AI_Tune_ICM_table *ptr);
void drivef_ai_tune_dcc_set(DRV_AI_Tune_DCC_table *ptr, unsigned char enable);
void drivef_ai_tune_dcc_get(DRV_AI_Tune_DCC_table *ptr);
void drivef_ai_dcc_user_curve_get(int *ptr);
// end lesley 1016

// 0506 lsy
int scalerAI_PQ_offset_get(int ip_type, int tuning_item);
void scalerAI_PQ_get(void);
// end 0506 lsy

// 0520 lsy
void set_vdec_securestatus_aipq(unsigned char status);
void set_dtv_securestatus_aipq(unsigned char status);
unsigned char get_svp_protect_status_aipq(void);
int scalerAI_pq_mode_ctrl_tv006(unsigned char stereo_face, unsigned char dcValue);
// end 0520 lsy

// 0622 lsy
int scalerAI_pq_sqm_mode_ctrl_tv006(unsigned char aipq_sqm_mode);
// end 0622 lsy

int scalerAI_pq_scene_mode_ctrl_tv001(unsigned char aipq_scene_mode);
int scalerAI_pq_sqm_mode_ctrl_tv001(unsigned char aipq_sqm_mode);
int scalerAI_pq_mode_ctrl_tv001(unsigned char stereo_face, unsigned char dcValue);
int scalerAI_pq_depth_mode_ctrl_tv001(unsigned char aipq_depth_mode);
int scalerAI_pq_noise_mode_ctrl_tv001(unsigned char aipq_noise_mode);

int scalerAI_aq_mode_ctrl_tv001(unsigned char aq_mode);

int ScalerAI_CheckBufferReady(struct file * filp);

int scalerAI_enableLowPowerMode(int enable);
void scalerAI_apply_by_SCENE(void);
void Scaler_set_AI_iEdgeSmooth_offset(unsigned char TableIdx);
void Scaler_set_AI_TNR_offset(unsigned char TableIdx);
void fwif_color_set_AI_TNR_Y_K_offset(unsigned char TableIdx);
void fwif_color_set_AI_TNR_C_K_offset(unsigned char TableIdx);
void fwif_color_set_AI_TNR_Y_th_offset(unsigned char TableIdx);
void fwif_color_set_AI_TNR_C_th_offset(unsigned char TableIdx);
void Scaler_set_AI_SNR_offset(unsigned char TableIdx);
void fwif_color_set_AI_SNR_w1_Offset(unsigned char TableIdx);
void fwif_color_set_AI_SNR_w2_Offset(unsigned char TableIdx);
void Scaler_set_AI_MNR_offset(unsigned char TableIdx);
void fwif_color_set_AI_mnr_edgethd_Offset(unsigned char TableIdx);
void fwif_color_set_AI_mnr_nlevel_Offset(unsigned char TableIdx);
void Scaler_set_AI_Directional_SU_Offset(unsigned char TableIdx);
void fwif_color_set_AI_Dirsu_pixel_Offset(unsigned char TableIdx);
void fwif_color_set_AI_Dirsu_Angle_transition_Offset(unsigned char TableIdx);
void Scaler_set_AI_DLTI_Offset(unsigned char TableIdx);
void fwif_color_AI_dlti_smooth(unsigned int *temp_tar_dlti,unsigned char flag);
void Scaler_set_AI_LC_Offset(unsigned char TableIdx);
void Scaler_set_AI_IDCTI_Offset(unsigned char TableIdx);
void Scaler_set_AI_DDCTI_Offset(unsigned char TableIdx);
void Scaler_set_AI_UVOffsetbyY_Offset(unsigned char TableIdx);
void fwif_color_set_AI_UVOffsetbyY_Offset(unsigned char TableIdx);
void fwif_color_reset_AIPQ_offset_table(void);
void fwif_color_set_AI_DDCTI_Offset(unsigned char TableIdx);
void scalerAI_pq_scene_Type_Set(PQ_SCENE_TYPE_TV030 ai_scene_type);
PQ_SCENE_TYPE_TV030 scalerAI_pq_scene_Type_Get(void);
void drvif_color_AI_scene_LC_set(unsigned int *gain);
void drvif_color_AI_scene_LC_Shpnr_Gain1st_set(unsigned int *gain);
void fwif_color_set_LC_ToneMappingSlopePoint_Aipq(unsigned char src_idx,unsigned char TableIdx);


#endif /* _AI_H_ */
