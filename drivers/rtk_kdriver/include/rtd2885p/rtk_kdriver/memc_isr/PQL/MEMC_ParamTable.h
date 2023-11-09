#ifndef _MEMC_PARAMTABLE_H
#define _MEMC_PARAMTABLE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LOGO_DETECT_EDGE_HORIZONTAL = 0,
	LOGO_DETECT_EDGE_P45,
	LOGO_DETECT_EDGE_VERTICAL,
	LOGO_DETECT_EDGE_N45,
	LOGO_DETECT_EDGE_NUM,
}_LOGO_DETECT_EDGE_TABLE;

typedef enum {
	SEARCH_RANGE_TYPE_OVER_SEARCH_RANGE = 0,
	SEARCH_RANGE_TYPE_NORMAL,
	SEARCH_RANGE_TYPE_NUM,
}_SEARCH_RANGE_TYPE_TABLE;

typedef struct
{
	unsigned char u8_logo_blkgrdhor_th;
	unsigned char u8_logo_blkgrdp45_th;
	unsigned char u8_logo_blkgrdver_th;
	unsigned char u8_logo_blkgrdn45_th;
}_LOGO_DETECT_EDGE_Param;

typedef struct
{
	bool u1_mc_fblvl_filter_en;
	unsigned char u2_mc_fblvl_filter_mode;
	unsigned char u8_mc_fblvl_filter_cut;
	unsigned char u8_mc_fblvl_filter_th;
	unsigned char u8_mc_fblvl_filter_avg_th;
	unsigned char u6_mc_fblvl_filter_num; 
}_MC_FBLVL_FILTER_PARAM;

typedef struct
{
	unsigned char u4_logo_erosion_th_panning;
	unsigned char u4_logo_erosion_th_fastmotion;
	unsigned char u4_logo_erosion_th_normal;
}_MC_LOGO_EROSION_TH_Param;

typedef struct
{
	unsigned int u32_tuning_threshold_PureVideo;
	unsigned int u32_tuning_threshold_Panning;
}_SC_TUNING_TH_PARAM;

typedef struct
{
	unsigned char u7_ME1_Y;
	unsigned char u7_ME2_Y;
}_SEARCH_RANGE_CLIP_PARAM;

typedef struct
{
	unsigned int u32_FB_bad_reg_low_th;
	unsigned int u32_FB_bad_reg_mid_th;
	unsigned int u32_FB_bad_reg_high_th;
}_FB_BAD_REGION_TH_PARAM;


typedef struct
{
	unsigned int u27_Fb_tc_th_low;
	unsigned int u27_Fb_tc_th_high;
}_FB_TC_TH_PARAM;

typedef struct
{
	unsigned short u12_bbd_h_precise_active_th;
	unsigned short u12_bbd_h_sketchy_active_th;
	unsigned short u13_bbd_v_precise_active_th;
	unsigned short u13_bbd_v_sketchy_active_th;
}_BBD_ACTIVE_TH_Param;

typedef struct
{
	unsigned short s16_resolution_width;
	unsigned short s16_resolution_height;
	unsigned short s16_rim_bound[_RIM_NUM];
	unsigned char u8_H_Divide;
	unsigned char u8_V_Divide;
	unsigned char u8_SlowIn_MinShiftX;
	unsigned char u8_SlowIn_MinShiftY;
	unsigned char u8_RimDiffTh_cof;
	unsigned int u32_apl_size;
	unsigned char u8_scale_H;
	unsigned char u8_scale_V;
}_RimCtrl_Param;

typedef struct
{
	unsigned char u8_top_rim;
	unsigned int u32_sad_offset;
}_SAD_SHIFT_PARAM;

typedef struct
{
	_RimCtrl_Param RimCtrl_Param;
	_BBD_ACTIVE_TH_Param BBD_ACTIVE_TH_Param;
}_RIM_Param;

typedef struct
{
	_LOGO_DETECT_EDGE_Param LOGO_DETECT_EDGE_Param;
	_MC_LOGO_EROSION_TH_Param MC_LOGO_EROSION_TH_Param;
}_LOGO_Param;

typedef struct
{
	_FB_BAD_REGION_TH_PARAM FB_BAD_REGION_TH_PARAM;
	_FB_TC_TH_PARAM FB_TC_TH_PARAM;
	_MC_FBLVL_FILTER_PARAM MC_FBLVL_FILTER_PARAM;
}_FB_Param;

typedef struct
{
	_SC_TUNING_TH_PARAM SC_TUNING_TH_PARAM;
}_SC_Param;

typedef struct
{
	_SEARCH_RANGE_CLIP_PARAM SEARCH_RANGE_CLIP_PARAM[SEARCH_RANGE_TYPE_NUM];
}_MV_SEARCHRANGE_Param;

typedef struct
{
	unsigned char u2_mc_vartap_sel; // 0:9tap, 1:17tap, 2:33tap
}_MC_Param;

typedef struct
{
	unsigned char u2_dh_phflt_med9flt_data_sel;
}_DEHALO_PARAM;

typedef struct
{
	unsigned char u2_bi_blk_res_sel; // 1:240x135, 0:480x270
}_BI_PARAM;

typedef struct
{
	_SAD_SHIFT_PARAM SAD_SHIFT_Param[_PQL_OUT_RESOLUTION_MAX];
	_RIM_Param	RIM_Param[_PQL_OUT_RESOLUTION_MAX];
	_LOGO_Param LOGO_Param[_PQL_OUT_RESOLUTION_MAX];
	_FB_Param FB_Param[_PQL_OUT_RESOLUTION_MAX];
	_SC_Param SC_Param[_PQL_OUT_RESOLUTION_MAX];
	_MV_SEARCHRANGE_Param MV_SEARCHRANGE_Param[_PQL_OUT_RESOLUTION_MAX];
	_MC_Param MC_Param[_PQL_OUT_RESOLUTION_MAX];
	_DEHALO_PARAM DEHALO_PARAM[_PQL_OUT_RESOLUTION_MAX];
	_BI_PARAM BI_PARAM[_PQL_OUT_RESOLUTION_MAX];
}_MEMC_PARAM_TABLE;

#ifdef __cplusplus
}
#endif

#endif

