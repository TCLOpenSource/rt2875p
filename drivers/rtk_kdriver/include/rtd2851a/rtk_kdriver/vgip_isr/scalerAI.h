#ifndef _SCALER_AI_H_
#define _SCALER_AI_H_


#include "gal/rtk_se_export.h"
#include <tvscalercontrol/vip/icm.h>
#include <tvscalercontrol/vip/ai_pq.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
/*******************************************************************************
*Definitions*
******************************************************************************/
// total (3+3+280)*3 + 20
#define VIP_NN_FLAG_BUFFER_SIZE (3*1024) //3k
#define VIP_NN_INDX_BUFFER_SIZE (3*1024) //3k
#define VIP_NN_DATA_BUFFER_SIZE (280*1024) //280k
#define VIP_NN_960X540_DATA_BUFFER_SIZE (768*1024) //768k (960x540x1.5=759KB)
#define VIP_NN_INFO_BUFFER_SIZE (20*1024) //20k
//#define AI_SCENE_TYPE_NUM		9

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

typedef struct _VIP_NN_MEMORY_CTRL {
	unsigned int size;
	unsigned int phy_addr_align;
	unsigned int *pVir_addr_align;

} VIP_NN_MEMORY_CTRL;


typedef enum _VIP_NN_BUFFER_INDEX{
	VIP_NN_BUFFER_NOT_USED=0,
	VIP_NN_BUFFER_224FULL=1,
	VIP_NN_BUFFER_416FIT=2,
	VIP_NN_BUFFER_320X240FULL=3,
	VIP_NN_BUFFER_960X540=4,
	VIP_NN_BUFFER_MAX,
} VIP_NN_BUFFER_INDEX;

#define VIP_NN_BUFFER_NUM VIP_NN_BUFFER_MAX

typedef struct _VIP_NN_CTRL {
	//unsigned char PA_IDX; // SE write buffer idx
	VIP_NN_MEMORY_CTRL NN_flag_Addr[VIP_NN_BUFFER_NUM];
	VIP_NN_MEMORY_CTRL NN_data_Addr[VIP_NN_BUFFER_NUM];
	VIP_NN_MEMORY_CTRL NN_info_Addr[VIP_NN_BUFFER_NUM];
	VIP_NN_MEMORY_CTRL NN_indx_Addr;
} VIP_NN_CTRL;

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

/*******************************************************************************
*functions
******************************************************************************/
void scalerAI_Init(void);
char fw_scalerip_reset_NN(void);
VIP_NN_CTRL* scalerAI_Access_NN_CTRL_STRUCT(void);

void scalerAI_preprocessing(void);
unsigned char scalerAI_get_nn_buf_idx(void);
void scalerAI_set_nn_buf_idx(unsigned char idx);
void scalerAI_get_NN_buffer(unsigned int *phyaddr_y, unsigned int *phyaddr_c, VIP_NN_BUFFER_INDEX index);
void scalerAI_set_NN_buffer(VIP_NN_BUFFER_INDEX bufIdx);
void scalerAI_scene_PQ_reset(void);
void scalerAI_scene_PQ_set(void);
void scalerAI_face_PQ_set(void);
void scalerAI_obj_info_get(AIInfo *info, unsigned char *NN_flag);
void scalerAI_postprocessing(void);
void scalerAI_execute_NN(void);
int AI_scene_adjust_top(int **topClasses, int outputCount, int topNum, int searchRange);
void AI_scene_dynamic_control(int *info, unsigned char NN_flag);
int AI_scene_get_top(int *pfProb, int **topClasses, int outputCount, int topNum);
void AI_scene_mapping_class(int **topClasses, int topk);
#if 0 // mac7p AI TBD memc
void AI_scene_detect_movie_using_memc(int **topClasses);
#endif
void AI_scene_detect_face(int **topClasses);
void AI_scene_sharp_set(void);
void AI_scene_color_set(void);
void AI_scene_TNR_set(void);
void AI_scene_LC_set(void);
void AI_scene_NR_set(void);
void AI_scene_DCC_set(void);
void AI_scene_ICM_set(void);
void AI_scene_sharp_reset(void);
void AI_scene_color_reset(void);
void AI_scene_TNR_reset(void);
void AI_scene_LC_reset(void);
void AI_scene_NR_reset(void);

// chen 0417
void AI_face_dynamic_control(AIInfo face_in[6], int scene_change, unsigned char NN_flag);
void AI_face_octa_calculate(int width, int height, int range, int *dir, int *mode); // henry 0612
void AI_face_uvcenter_calculate(int cb_max, int cr_max, int cb_med, int cr_med, int *centerU, int *centerV); // henry 0612
void AI_face_win_pos_predict(int faceIdx, AIInfo face);
void AI_face_ICM_blending_value(int faceIdx, AIInfo face);
void AI_face_dynamic_ICM_offset(int faceIdx, AIInfo face, int *h_adj_o, int *s_adj_o, int *v_adj_o); // lesley 0808
void AI_face_ICM_tuning(int faceIdx, AIInfo face);
// chen 0426
void AI_face_ICM_adjust_valuen(int hue_info, int sat_info, int val_info, int *hue_delta, int *sat_delta, int *val_delta, COLORELEM ICM_TAB_ACCESS[ITNSEGMAX][SATSEGMAX][HUESEGMAX]);

// chen 0429
void AI_face_DCC_blending_value(int faceIdx, AIInfo face);
// end chen 0429

// chen 0527
void AI_face_Sharp_blending_value(int faceIdx, AIInfo face);
//end chen 0527

extern void  drvif_color_get_dcc_adjust_value(unsigned int val_info , int *value_delta, unsigned int *sAccLUT); //get from   drvif_color_get_dcc_current_curve
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
void scalerAI_ai_pq_off(void);
void scalerAI_ai_pq_on(unsigned char mode, unsigned char dcValue);
// end lesley 0911

// lesley 1002_1
void drivef_tool_ai_info_set(int idx, int h_norm, int s_norm, int i_norm);
void drivef_tool_ai_info_get(TOOL_AI_INFO *ptr);
// end lesley 1002_1

#if 0 // V4L2_ERR
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

#endif /* _AI_H_ */
