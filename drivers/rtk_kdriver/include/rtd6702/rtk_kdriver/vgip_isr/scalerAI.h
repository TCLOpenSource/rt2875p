#ifndef _SCALER_AI_H_
#define _SCALER_AI_H_

#include <linux/fs.h> 
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
	
} DRV_AI_Tune_ICM_table;

typedef struct {	
	unsigned short dcc_user[32];	
} DRV_AI_Tune_DCC_table;


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
	// the 90k clock value
	unsigned int Time[AI_CAPIMG_RECORD_FRAME];
	unsigned int PreCropTime[AI_CAPIMG_RECORD_FRAME];
	unsigned int CurCropTime[AI_CAPIMG_RECORD_FRAME];
	unsigned int PreFullTime[AI_CAPIMG_RECORD_FRAME];
	unsigned int CurFullTime[AI_CAPIMG_RECORD_FRAME];
} AI_CAPTURE_IMG_RECORD_T; // 128*13 = 1664

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

extern void  drvif_color_get_dcc_adjust_value(int val_info , int *value_delta, unsigned int *sAccLUT); //get from   drvif_color_get_dcc_current_curve
// end chen 0417

// chen 0805
void AI_face_DCC_blending_value_global(int max_face_size);
// end chen 0805

void AI_face_Sharp_blending_value_global(void);
void AI_face_ICM_blending_value_global(int max_face_size);
void AI_face_Decont_blending_value(int faceIdx, AIInfo face);//mk2

void scalerAI_face_pq_init(unsigned char mode);

void drivef_tool_ai_info_set(int idx, int h_norm, int s_norm, int i_norm);
void drivef_tool_ai_info_get(TOOL_AI_INFO *ptr);
void drivef_tool_ai_db_set(DB_AI_RTK *ptr);

#ifdef CONFIG_SCALER_ENABLE_V4L2 //#if 0 // V4L2_ERR
void drvif_color_get_DB_AI_DCC(CHIP_DCC_T *ptr);
void drvif_color_get_DB_AI_ICM(CHIP_CM_RESULT_T *v4l2_data);
void drvif_color_get_DB_AI_SHP(CHIP_SHARPNESS_UI_T *ptCHIP_SHARPNESS_UI_T);
void drvif_color_set_DB_AI_DCC(void);
void drvif_color_set_DB_AI_ICM(void);
void drvif_color_set_DB_AI_SHP(void);
#endif
void drivef_ai_tune_icm_set(DRV_AI_Tune_ICM_table *ptr);
void drivef_ai_tune_icm_get(DRV_AI_Tune_ICM_table *ptr);
void drivef_ai_tune_dcc_set(DRV_AI_Tune_DCC_table *ptr, unsigned char enable);
void drivef_ai_tune_dcc_get(DRV_AI_Tune_DCC_table *ptr);
void drivef_ai_dcc_user_curve_get(unsigned short *ptr);
void drivef_color_AI_obj_dcc_user_curve_init(unsigned char enable);

int scalerAI_PQ_offset_get(int ip_type, int tuning_item);
void scalerAI_PQ_get(void);

void set_vdec_securestatus_aipq(unsigned char status);
void set_dtv_securestatus_aipq(unsigned char status);
unsigned char get_svp_protect_status_aipq(void);

int scalerAI_pq_scene_mode_ctrl_tv001(unsigned char aipq_scene_mode);
int scalerAI_pq_sqm_mode_ctrl_tv001(unsigned char aipq_sqm_mode);
int scalerAI_pq_mode_ctrl_tv001(unsigned char stereo_face);
int scalerAI_pq_depth_mode_ctrl_tv001(unsigned char aipq_depth_mode);

int ScalerAI_CheckBufferReady(struct file * filp);

int scalerAI_enableLowPowerMode(int enable);

#endif /* _AI_H_ */
