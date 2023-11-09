#ifndef _SCALER_PQMASKCOLORLIB_H_
#define _SCALER_PQMASKCOLORLIB_H_
/*******************************************************************************
 * Header files
*******************************************************************************/
#include <tvscalercontrol/vip/scalerPQMaskCtrl.h>
#include <tvscalercontrol/vip/peaking.h>
#include <gal/rtd6702/rtk_kadp_se.h>
#include <scaler/vipCommon.h>

/*******************************************************************************
 * Definitions
*******************************************************************************/
#if 0 // move to vipCommon.h
#define PQMASK_MAPCURVE_LEN		256
#endif
#define PQMASK_HISTO_BIN_SFT	(5)
#define PQMASK_HISTO_BIN		(1<<PQMASK_HISTO_BIN_SFT)

/*******************************************************************************
 * Structure
*******************************************************************************/
enum {
	/* 0 */ PQMASK_COLOR_PALETTE_DEPTH = 0,
	/* 1 */ PQMASK_COLOR_PALETTE_SEMANTIC,
	/* 2 */ PQMASK_COLOR_PALETTE_MAX,
};

enum {
	/* 0 */ PQMASK_COLORSTRUCT_MGR = 0,
	/* 1 */ PQMASK_COLORSTRUCT_SEMATNIIC_HISTO,
	/* 2 */ PQMASK_COLORSTRUCT_DEPTH_HISTO,
	/* 3 */ PQMASK_COLORSTRUCT_NUM,
};

enum {
	/* 0 */ PQMASK_IMGCH_Y = 0,
	/* 1 */ PQMASK_IMGCH_U,
	/* 2 */ PQMASK_IMGCH_V,
	/* 3 */ PQMASK_IMGCH_NUM,
};

enum {
	/* 0 */ PQMASK_DEPDIST_FAR = 0,
	/* 1 */ PQMASK_DEPDIST_MID,
	/* 2 */ PQMASK_DEPDIST_NEAR,
	/* 3 */ PQMASK_DEPDIST_NUM,
};

typedef struct PQMASK_COLOR_NR {
	unsigned int bNrEnable;
	unsigned int bIDecontEnable;
	unsigned int bIBlockDecontEnable;
} PQMASK_COLOR_NR_T;

typedef struct PQMASK_COLOR_ICM {
	unsigned int bHueEnable;
	unsigned int HueGain;
	unsigned int bSatEnable;
	unsigned int SatGain;
	unsigned int bIntEnable;
	unsigned int IntGain;
} PQMASK_COLOR_ICM_T;

typedef struct PQMASK_COLOR_TBL {
	PQMASK_COLOR_NR_T Nr;
	PQMASK_COLOR_ICM_T ICM;
	DRV_LABEL_PK Sharpness;
} PQMASK_COLOR_TBL_T;

typedef struct COLOR_ARGB {
	unsigned char A;
	unsigned char R;
	unsigned char G;
	unsigned char B;
} COLOR_ARGB_T;

typedef struct PQMASK_HIST_INFO {
	unsigned int enable;
	unsigned int width;
	unsigned int height;
	unsigned int hJmp;
	unsigned int vJmp;
} PQMASK_HIST_INFO_T;

typedef struct PQMASK_HIST_DATA {
	unsigned int TotalPts;
	unsigned int Histogram[PQMASK_HISTO_BIN];
	unsigned int TotalPercent; // thousandth total
	unsigned int NorHist[PQMASK_HISTO_BIN]; // thousandth histogram
} PQMASK_HIST_DATA_T;

typedef struct PQMASK_HIST {
	PQMASK_HIST_INFO_T Info;
	PQMASK_HIST_DATA_T Data;
} PQMASK_HIST_T;

typedef struct PQMASK_COLOR_STATUS {
	unsigned char bSceneChange;
	int SCRstFrm;
	int ForceCurFrm;
	unsigned char bReloadDepPalette;
	unsigned char bReloadSemPalette;
} PQMASK_COLOR_STATUS_T;

typedef struct PQMASK_COLOR_PARAM {
	// informaiton fusion
	unsigned char FusionRatio_Dep; // effect on depth
	unsigned char FusionRatio_Sem; // effect on semantic
	// temporal filter control
	unsigned char FusionRatio_Pre; // effect on pre
	unsigned char FusionRatio_Cur; // effect on cur
	// spatial filter control
	unsigned char SpatialFltCycle;
	unsigned int SEHorCoef[4]; // for SE
	unsigned int SEVerCoef[4];
	unsigned char HorOddType;
	unsigned char VerOddType;
	// scene change
	unsigned char SceneChangeRstFrm;
	unsigned char ScFusionRatio_Pre;
	unsigned char ScFusionRatio_Cur;
} PQMASK_COLOR_PARAM_T;

typedef struct PQMASK_COLOR_MGR {
	// HW module setting
	PQMASK_COLOR_TBL_T ModuleSetting;
	// Weighting table
	unsigned char SemanticWeightTbl[PQMASK_MODULE_NUM][PQMASK_LABEL_NUM];
	unsigned char DepthWeightTbl[PQMASK_MODULE_NUM][PQMASK_MAPCURVE_LEN];
	// Histogram
	PQMASK_HIST_T DepthHist;
	PQMASK_HIST_T SemanticHist;
	//
	PQMASK_HIST_T ImgBySemanticHist[PQMASK_LABEL_NUM][PQMASK_IMGCH_NUM];
	PQMASK_HIST_INFO_T ImgByDepthHistInfo;
	unsigned char ThlLow; // far <= thl < mid
	unsigned char ThlHigh; // mid <= thl < near
	PQMASK_HIST_T ImgByDepthHist[PQMASK_DEPDIST_NUM][PQMASK_IMGCH_NUM];
	//
	PQMASK_COLOR_STATUS_T Status;
	PQMASK_COLOR_PARAM_T Param;
} PQMASK_COLOR_MGR_T;

/*******************************************************************************
 * Parameters
*******************************************************************************/

/*******************************************************************************
 * functions
*******************************************************************************/
void scalerPQMaskColor_init(void);
void scalerPQMaskColor_disable(void);
void *scalerPQMaskColor_GetStruct(unsigned int InfoIndex); //

void scalerPQMaskColor_SemanticHistogram(void *pInput);
void scalerPQMaskColor_DepthHistogram(void *pInput);
void scalerPQMaskColor_Img2DepthHistogram(void *pImgInput, void *pInfoInput);
void scalerPQMaskColor_Img2SemanticHistogram(void);
void scalerPQMaskColor_Label2Clut(unsigned int group, KGAL_PALETTE_INFO_T *pPalette);
void scalerPQMaskColor_Depth2Clut(unsigned int group, KGAL_PALETTE_INFO_T *pPalette);
void scalerPQMaskColor_SceneChange(void);
void scalerPQMaskColor_ForceOutputCur(unsigned char Frames);
void scalerPQMaskColor_ConstructPalette(void);
void scalerPQMaskColor_SetReloadPalette(unsigned char PaletteSel);

// 
void scalerPQMaskColor_UpdateLabelPK(void);

// RTICE commands
void scalerPQMaskColor_GetSemanticWeight(unsigned char *Buf);
void scalerPQMaskColor_SetSemanticWeight(unsigned char *Buf);
void scalerPQMaskColor_GetDepthWeight(unsigned char *Buf);
void scalerPQMaskColor_SetDepthWeight(unsigned char *Buf);

// V4L2 commands
int scalerPQMask_V4L2_SetDepthWeight(unsigned char ModuleSel, unsigned char *Buf);
#endif