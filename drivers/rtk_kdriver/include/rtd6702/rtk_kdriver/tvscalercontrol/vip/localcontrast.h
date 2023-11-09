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
	unsigned int Curve[256][35];
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
/* SLC DMA function */
short drvif_SLC_DMA_Err_Check(unsigned char showMSG);
void drvif_SLC_Err_Reset(void);
char drvif_SLC_Wait_for_DMA_Apply_Done(void);
void drvif_color_Set_SLC_By_DMA( unsigned int addr, unsigned int* vir_addr, unsigned int *pTBL);

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

void drvif_convert_DSLC_st(unsigned char direct, DYNAMIC_SLC_CTRL_ST *drv, DYNAMIC_SLC_CTRL_RTICE_ST *rtice);

/* COLOR DEPENDENCY LOCAL CONTRAST SET FUNCTIONS */
void drvif_color_set_CDLC_Enable(unsigned char bYEnable, unsigned char bUEnable, unsigned char bVEnable);
void drvif_color_set_CDLC_Offset(DRV_CDLC_Offset *pOffset);
void drvif_color_set_CDLC_Shape(DRV_CDLC_Shape *pShape);
void drvif_color_set_CDLC_Distance(DRV_CDLC_Distance *pDistance);

/*  LOCAL CONTRAST PIP OVERLAY APL SET FUNCTIONS */
void drvif_color_set_LC_overlayAPL(DRV_LC_overlay_APL *overlay);

/* OVERLAY CONTROL */
void drvif_color_set_overlay_control(unsigned char bOverlayEn);

#ifdef __cplusplus
}
#endif

#endif // __LOCALCONTRAST_H__


