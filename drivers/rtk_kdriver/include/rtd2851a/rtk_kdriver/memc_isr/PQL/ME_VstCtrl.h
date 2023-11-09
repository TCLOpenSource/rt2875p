#ifndef _ME_VST_CTRL_DEF_H
#define _ME_VST_CTRL_DEF_H

typedef struct
{
	unsigned char u1_update_hist_en;
}_PARAM_ME_VST_CTRL;

typedef struct
{
	unsigned char u1_detect;
	unsigned char u1_detect_blackBG_VST_trure;
	unsigned char u8_boundarySta;
	unsigned char u8_boundaryEnd;
}_OUTPUT_ME_VST_CTRL;

enum
{
	/* 0 */ SEC0 = 0,
	/* 1 */ SEC1,
	/* 2 */ SEC2,
	/* 3 */ SEC3,
	/* 4 */ SEC_NUM
};

enum
{
	/* 0 */ X_NEG = 0,
	/* 1 */ X_ZER,
	/* 2 */ X_POS,
	/* 3 */ X_NUM
};

enum
{
	/* 0  */ Y_NEG_8 = 0,
	/* 1  */ Y_NEG_7,
	/* 2  */ Y_NEG_6,
	/* 3  */ Y_NEG_5,
	/* 4  */ Y_NEG_4,
	/* 5  */ Y_NEG_3,
	/* 6  */ Y_NEG_2,
	/* 7  */ Y_NEG_1,
	/* 8  */ Y_ZER_0,
	/* 9  */ Y_POS_1,
	/* 10 */ Y_POS_2,
	/* 11 */ Y_POS_3,
	/* 12 */ Y_POS_4,
	/* 13 */ Y_POS_5,
	/* 14 */ Y_POS_6,
	/* 15 */ Y_POS_7,
	/* 16 */ Y_POS_8,
	/* 17 */ Y_NUM
};

VOID ME_Vst_Init(_OUTPUT_ME_VST_CTRL *pOutput);
VOID ME_Vst_Proc(const _PARAM_ME_VST_CTRL *pParam, _OUTPUT_ME_VST_CTRL *pOutput);

#endif