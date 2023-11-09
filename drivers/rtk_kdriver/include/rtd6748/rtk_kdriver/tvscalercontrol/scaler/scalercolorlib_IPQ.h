#ifndef __SCALERCOLOR_LIB_IPQ_H__
#define __SCALERCOLOR_LIB_IPQ_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Header include
******************************************************************************/


#include <tvscalercontrol/vip/icm.h>
#include <tvscalercontrol/scaler/source.h>
#include <tvscalercontrol/scaler/vipinclude.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/scalerColor.h>


/*******************************************************************************
 * Structure
 ******************************************************************************/
/*typedef struct*/
/*{*/
/*} MID_example_Param_t;*/

typedef struct _GammaCurve {
   	VIP_Output_Gamma_CTRL_ITEM outputgamma_ctrl;
    unsigned int  tGammaTable[512*4];
} GammaCurve;

typedef struct _INTERNAL_PARTTEN
{
	unsigned char enable;
	unsigned short r_Val;
	unsigned short g_Val;
	unsigned short b_Val;

}INTERNAL_PARTTEN;

typedef enum{
		PQ_MODE_BYPASS=0,
		PQ_MODE_NORMAL,
		PQ_MODE_xvycc,
		PQ_MODE_HDR,
		PQ_MODE_SDR2HDR,
		PQ_MODE_FOOTBALL,
		PQ_MODE_DOLBY,
		PQ_MODE_FILM,
		PQ_MODE_DCIP3,
		PQ_MODE_ENHANCE_WIDE,
		PQ_MODE_HDR_ENHANCE_WIDE,
		PQ_MODE_SHOP_DEMO,
		PQ_MODE_MAX = 0xff,
}RTK_VPQ_MODE_TYPE_TV030;

typedef struct _BRIGHTNESS_LUT_OETF {
	unsigned short	   un16_length;
	unsigned short	   pf_LUT[256];
}BRIGHTNESS_LUT_OETF;

typedef struct
{
	 short	 BT709Target[3][3];
	 short	 BT2020Target[3][3];
	 short	 DCIP3Target[3][3];
	 short	 ADOBERGBTarget[3][3];	 
     short   BT601_525Target[3][3];	
     short   BT601_625Target[3][3];
}RTK_VPQ_sRGB_Matrix;

typedef struct _RTK_TableSize_Gamma {
	unsigned short nTableSize;
	unsigned short pu16Gamma_r[256];
	unsigned short pu16Gamma_g[256];
	unsigned short pu16Gamma_b[256];
} RTK_TableSize_Gamma;
/*==================== Definitions ================= */
void Scaler_Set_Partten4AutoGamma(unsigned char  Enable, unsigned short  r_Val,unsigned short  g_Val,unsigned short  b_Val);
void Scaler_Set_PictureMode_PQsetting(RTK_VPQ_MODE_TYPE_TV030 mode);

UINT8 fwif_color_get_DM2_HDR_histogram_MaxRGB(UINT32* HDR_info);
UINT8 fwif_color_get_DM2_HDR_histogram_TV030(UINT32* HDR_info);
unsigned char fwif_color_get_HDR_YUV2RGB_SDR_MAX_RGB_by_HFC_TV043(unsigned char color_format);
void Scaler_SetBrightness_LUT_OETF(unsigned short *Out_T , BRIGHTNESS_LUT_OETF *pData);
void Scaler_SetOETF2Gamma(BRIGHTNESS_LUT_OETF *pData);
void Scaler_SetIPQ_EN(unsigned char value);
unsigned char Scaler_GetIPQ_EN(void);
void Scaler_SetIPQCurve_EN(unsigned char value);
unsigned char Scaler_GetIPQCurve_EN(void);
unsigned char fwif_color_get_colorspacergb2yuvtransfer_table_SDR_MAX_RGB_TV043(unsigned char channel, unsigned char nSrcType, unsigned char nSD_HD, bool bUse_HDMI_Range_Detect);
unsigned char fwif_color_get_I3DDMA_colorspacergb2yuvtransfer_table_SDR_MAX_RGB_TV043(unsigned char channel, unsigned char nSrcType, unsigned char nSD_HD, bool bUse_HDMI_Range_Detect);
unsigned char fwif_color_get_colorspacergb2yuvtransfer_table_TV043(unsigned char channel, unsigned char nSrcType, unsigned char nSD_HD, bool bUse_HDMI_Range_Detect);

void fwif_color_set_OutputGamma_System(void);
void fwif_color_gamma_encode_TableSize_2(RTK_TableSize_Gamma *pData);
void fwif_color_set_UV_Offset_tv043(VIP_YUV2RGB_LEVEL_SELECT which_Lv);

#ifdef __cplusplus
}
#endif

#endif /* __SCALER_LIB_H__*/

