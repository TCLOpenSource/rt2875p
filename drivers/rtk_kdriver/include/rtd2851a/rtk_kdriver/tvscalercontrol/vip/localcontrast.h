/* 20140523 Local Contrast header file*/
#ifndef  __LOCALCONTRAST_H__
#define  __LOCALCONTRAST_H__

#ifdef __cplusplus
extern "C" {
#endif
/* some include about scaler*/
#include <scaler/vipCommon.h>
#include <scaler/vipRPCCommon.h>

void drvif_color_set_LC_Enable(unsigned char enable);
void drvif_color_set_LC_Global_Ctrl(DRV_LC_Global_Ctrl *ptr);
void drvif_color_set_LC_Backlight_Decision(DRV_LC_Backlight_Decision *ptr);
void drvif_color_set_LC_Spatial_Filter(DRV_LC_Spatial_Filter *ptr);
void drvif_color_set_LC_Temporal_Filter(DRV_LC_Temporal_Filter *ptr);
void drvif_color_set_LC_Backlight_Profile_Interpolation(DRV_LC_Backlight_Profile_Interpolation *ptr);
//void drvif_color_set_LC_BL_Profile_Interpolation_Table(unsigned int *ptr);
void drvif_color_set_LC_BL_Profile_Interpolation_Table(unsigned int *ptr,unsigned char mode,unsigned char tab_size);

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

#if 0//juwen, LC, APL, 170626 /*mac6 hw no implement*/
void drvif_color_get_LC_APL(unsigned int *ptr_out);
#endif
void drvif_color_check_LC_bypass_case(void);

unsigned char drvif_color_get_LC_blk_hnum(void);
unsigned char drvif_color_get_LC_blk_vnum(void);
void drvif_color_get_LC_APL(unsigned int *ptr_out, unsigned int bufferSz);
void drvif_color_get_LC_Histogram(unsigned int *plc_out);
void drvif_color_get_LC_Blight_dat(unsigned int *ptr_out, unsigned int bufferSz);
unsigned char drvif_color_get_LC_size_support(void);
void drvif_color_set_LC_size_support(unsigned char u1_support);

#ifdef __cplusplus
}
#endif

#endif // __LOCALCONTRAST_H__


