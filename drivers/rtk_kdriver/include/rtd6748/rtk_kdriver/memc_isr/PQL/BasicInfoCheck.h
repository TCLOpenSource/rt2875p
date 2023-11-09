#ifndef _BASICINFOCHECK_H
#define _BASICINFOCHECK_H

#ifdef __cplusplus
extern "C" {
#endif
//#include "memc_isr/PQL/FRC_SceneAnalysis.h"
#define TAG_NAME_MEMC "MEMC"
#define __MEMC_SELF_CALIB__				("MEMC_SELF_CALIB")
#define __MEMC_AUTO_REGRESSION__		("MEMC_AUTO_REGRESSION")
#define __MEMC_GET_REGRESSION_DATA__	("MEMC_GET_REGRESSION_DATA")
#define __MEMC_GET_BASICINFO_DATA__		("MEMC_GET_BASICINFO_DATA")

#define	REGRESSION_SC_MAX_DATA		(20)
#define	REGRESSION_GFB_MAX_DATA		(7)
#define	REGRESSION_FB_MAX_TIME		(170)
#define	REGRESSION_FB_MAX_DATA		(REGRESSION_FB_MAX_TIME) // record data every 1 second
#define	REGRESSION_LOGO_MAX_TIME	(170)
#define	REGRESSION_LOGO_MAX_DATA	(REGRESSION_LOGO_MAX_TIME) // record data every 1 second
#define RGN21						(21)

typedef struct
{
	unsigned char u8_result;
	unsigned short u16_total_num;
	unsigned short u16_correct_num;
}_Self_Check_Result;

typedef struct
{
	unsigned short u13_bar_width;
	unsigned short u13_bar_height;
	unsigned char u8_speed_h;
	unsigned char u8_speed_v;
	unsigned char u5_mot_len;
	unsigned int u32_mot_seq;
	unsigned short u10_bar_color_r;
	unsigned short u10_bar_color_g;
	unsigned short u10_bar_color_b;
	unsigned char u1_pattern_mode;
	unsigned char u2_bg_type;
	unsigned char u3_bg_type0;
	unsigned short u8_checkerboard_width;
	unsigned short u8_checkerboard_height;
}_PTG_Param;

typedef struct
{
	unsigned char u8_r_id;
	unsigned char u8_g_id;
	unsigned char u8_b_id;
}_APL_RGB;

typedef struct{
	unsigned short u10_r;
	unsigned short u10_g;
	unsigned short u10_b;
}_PTG_RGB;

typedef struct{
	unsigned short u16_x;
	unsigned short u16_y;
}_PTG_POS;

typedef struct{
	_PTG_POS PTG_pos;
	_PTG_RGB PTG_color;
}_PTG_POS_COLOR;

typedef struct
{
	unsigned int u13_square_width;
	unsigned int u13_square_height;
	unsigned int u8_speed;
	unsigned int u8_checkerboard_width;
	unsigned int u8_checkerboard_height;
}_SAD_PTG_Param;

typedef struct
{
	unsigned int u10_color;
	unsigned int u8_checkerboard_width;
	unsigned int u8_checkerboard_height;
}_DTL_PTG_Param;

typedef enum{
	PTG_DEFAULT = 0,
	PTG_FINAL_CHECK_WITH_SC_1,
	PTG_FINAL_CHECK_WITHOUT_SC_1,
	PTG_FINAL_CHECK_WITH_SC_2,
	PTG_FINAL_CHECK_WITHOUT_SC_2,
	PTG_CHECK_SC_LV2,
	PTG_CHECK_SC_LV3,
	PTG_GET_APL_DATA,
	PTG_GET_GMVCNT_DATA,
	PTG_GET_DTL_DATA,
	PTG_CHECK_CHECKERBOARD_POS,
	PTG_CHECK_SQUARE_SIZE,
	PTG_CHECK_LBMC_1,
	PTG_CHECK_LBMC_2,
	PTG_CHECK_LBMC_3,
	PTG_CHECK_LBMC_4,
	PTG_INDEX_NUM,
}PTG_TABLE_INDEX;

typedef enum{
	CHECKERBOARD_SIZE_100 = 0,
	CHECKERBOARD_SIZE_64,
	CHECKERBOARD_SIZE_50,
	CHECKERBOARD_SIZE_32,
	CHECKERBOARD_SIZE_25,
	CHECKERBOARD_SIZE_16,
	CHECKERBOARD_SIZE_10,
	CHECKERBOARD_SIZE_NUM,
}CHECKERBOARD_SIZE_INDEX;

typedef enum{
	INTERNAL_AUTOTEST_APL = 0,
	INTERNAL_AUTOTEST_DTL,
	INTERNAL_AUTOTEST_SAD,
	INTERNAL_AUTOTEST_SC,
	INTERNAL_AUTOTEST_LBMC,
	INTERNAL_AUTOTEST_YUV,
	INTERNAL_AUTOTEST_NUM,
}INTERNAL_AUTOTEST_INDEX;

typedef enum{
	REALPATTERN_AUTOTEST_FB_BADREGION = 0,
	REALPATTERN_AUTOTEST_NUM,
}REALPATTERN_AUTOTEST_INDEX;

typedef enum{
	ENVIRONMENT_CHECK_APL = 0,
	ENVIRONMENT_CHECK_PTG_POS,
	ENVIRONMENT_CHECK_PTG_SQUARE,	
	ENVIRONMENT_CHECK_DDR_BOUNDARY,
	ENVIRONMENT_CHECK_RIM_BOUNDARY,
	ENVIRONMENT_CHECK_LBMC,
	ENVIRONMENT_CHECK_GMV,
	ENVIRONMENT_CHECK_REGION,
	ENVIRONMENT_CHECK_NUM,
}ENVIRONMENT_CHECK_INDEX;

typedef enum{
	AUTOTEST_TYPE_ENVIRONMENT_CHECK = 0,
	AUTOTEST_TYPE_INTERNAL_AUTOTEST,
	AUTOTEST_TYPE_NUM,
}AUTOTEST_TYPE_INDEX;

typedef enum{
	AUTOREGRESSION_SC_transient_11_to_22 = 0,
	AUTOREGRESSION_SC_transient_11_to_32,
	AUTOREGRESSION_SC_009,
	AUTOREGRESSION_SC_023,
	AUTOREGRESSION_SC_059,
	AUTOREGRESSION_SC_100,
	AUTOREGRESSION_SC_115,
	AUTOREGRESSION_SC_304,
	AUTOREGRESSION_SC_340,
//can't play
//	AUTOREGRESSION_SC_THE_MATRIX_RELOADED_4khdr_5_short,
			//can play
	AUTOREGRESSION_SC_NUM,
}AUTOREGRESSION_SC_INDEX;

typedef enum{
//	AUTOREGRESSION_FB_009,
//	AUTOREGRESSION_FB_012,
//	AUTOREGRESSION_FB_129,
//	AUTOREGRESSION_FB_130,
//	AUTOREGRESSION_FB_134,
//	AUTOREGRESSION_FB_135,
	AUTOREGRESSION_GFB_171,
//	AUTOREGRESSION_FB_182,
//	AUTOREGRESSION_FB_202,
//	AUTOREGRESSION_FB_316,
//	AUTOREGRESSION_FB_317,
//	AUTOREGRESSION_FB_364,
	AUTOREGRESSION_GFB_365,
	AUTOREGRESSION_GFB_376,
//	AUTOREGRESSION_FB_421,
//hard to check fb
//	AUTOREGRESSION_FB_Dehalo_MI3_2, //can play
//can't play
//	AUTOREGRESSION_FB_324,
//	AUTOREGRESSION_FB_340,			//can play
//	AUTOREGRESSION_FB_387,
	AUTOREGRESSION_GFB_NUM,
}AUTOREGRESSION_FB_INDEX;

typedef enum{
	AUTOREGRESSION_LFB_009,
	AUTOREGRESSION_LFB_012,
	AUTOREGRESSION_LFB_129,
	AUTOREGRESSION_LFB_130,
	AUTOREGRESSION_LFB_134,
	AUTOREGRESSION_LFB_NUM,
}AUTOREGRESSION_LFB_INDEX;

typedef enum{
	AUTOREGRESSION_LOGO_2K_logo_zebra,
	AUTOREGRESSION_LOGO_204,
	AUTOREGRESSION_LOGO_205,
	AUTOREGRESSION_LOGO_480,
	AUTOREGRESSION_LOGO_NUM,
}AUTOREGRESSION_LOGO_INDEX;

typedef enum{
	AUTOREGRESSION_SC = 0,
	AUTOREGRESSION_GFB,
	AUTOREGRESSION_LFB,
	AUTOREGRESSION_LOGO,
	AUTOREGRESSION_TYPE_NUM,
}AUTOREGRESSION_TYPE_INDEX;

typedef enum{
	SELF_TEST_FAIL = 0, 
	SELF_TEST_PASS,
	SELF_TEST_INIT,
	SELF_TEST_PROCESS,
	SELF_TEST_NUM,
}SELF_TEST_RESULT;

typedef enum{
	POS_CHECK_LEFT_UP = 0,
	POS_CHECK_RIGHT_DOWN,
	POS_CHECK_NUM,
}IPPRE_PTG_POS_CHECK_INDEX;

typedef enum{
	FB_BADREGION_CNT_LOW_THRESH = 0,
	FB_BADREGION_CNT_MID_THRESH,
	FB_BADREGION_CNT_HIGH_THRESH,
	FB_BADREGION_CNT_ALL_THRESH,
	FB_BADREGION_CNT_NUM,
}_FB_BADREGION_CNT;

typedef struct
{
	unsigned int u32_time[REGRESSION_SC_MAX_DATA];
}_AUTOREGRESSION_SINGE_RESULT_SC;

typedef struct
{
	unsigned int u21_LFB_lvl_cnt[REGRESSION_FB_MAX_DATA][RGN21];
}_AUTOREGRESSION_SINGE_RESULT_LFB;

typedef struct
{
	unsigned int u32_GFB_lvl_cnt[REGRESSION_FB_MAX_DATA];
}_AUTOREGRESSION_SINGE_RESULT_GFB;

typedef struct
{
	unsigned int u32_detect_logo_cnt[REGRESSION_LOGO_MAX_DATA][RGN21];
}_AUTOREGRESSION_SINGE_RESULT_LOGO;

typedef struct
{
	_AUTOREGRESSION_SINGE_RESULT_SC SC[AUTOREGRESSION_SC_NUM];
	_AUTOREGRESSION_SINGE_RESULT_GFB GFB[AUTOREGRESSION_GFB_NUM];
	_AUTOREGRESSION_SINGE_RESULT_LFB LFB[AUTOREGRESSION_LFB_NUM];
	_AUTOREGRESSION_SINGE_RESULT_LOGO PLOGO[AUTOREGRESSION_LOGO_NUM];
	_AUTOREGRESSION_SINGE_RESULT_LOGO BLOGO[AUTOREGRESSION_LOGO_NUM];
}_AUTOREGRESSION_RESULT;


VOID BasicInfoCheck_Init(void);
unsigned char BasicInfoCheck_cal_result(unsigned char AUTOTEST_TYPE, unsigned char AUTOTEST_INDEX, unsigned short u16_correct_num, unsigned short u16_total_num);
VOID BasicInfoCheck_Show_APL_Info(void);
VOID BasicInfoCheck_Show_DTL_Info(void);
VOID BasicInfoCheck_Show_SAD_Info(void);
VOID BasicInfoCheck_Show_Basic_Info(void);
VOID BasicInfoCheck_Environment_check(void);
VOID BasicInfoCheck_IPPRE_PTG_check(void);
VOID BasicInfoCheck_SC_Check(void);
VOID BasicInfoCheck_RealPattern_check(void);
void BasicInfoCheck_AutoRegression(void);


#ifdef __cplusplus
}
#endif

#endif

