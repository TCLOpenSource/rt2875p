#ifndef __SCALER_VPQDEV_EXTERN_H
#define  __SCALER_VPQDEV_EXTERN_H


/**
 * Video Picture Quality Information.
 *
 * @see DDI Implementation Guide
*/
#include <scaler/vipCommon.h>
#include <scaler/scalerDrvCommon.h>

#include <tvscalercontrol/vip/film.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/icm.h>
//#include <tvscalercontrol/vip/scalerColor_tv010.h>
#include <tvscalercontrol/scaler/scalercolor_engmenu.h>

typedef enum{
	PQ_DEV_EXTERN_NOTHING = 0,
	PQ_DEV_EXTERN_INIT_DONE,
	PQ_DEV_EXTERN_OPEN_DONE,
	PQ_DEV_EXTERN_UNINIT,
	PQ_DEV_EXTERN_CLOSE,
} PQ_DEV_EXTERN_STATUS;

typedef struct _RTK_PQModule_T {
	unsigned int PQModule;
	unsigned char onOff;
} RTK_PQModule_T;
typedef struct _RTK_DEMO_MODE_T {
	unsigned int DEMO_MODE;
	unsigned char onOff;
} RTK_DEMO_MODE_T;

typedef struct _RTK_TV002_SR_T {
	unsigned short mode;// 0: off, 1: auto, 2: manual
	unsigned short value;// 0~100
} RTK_TV002_SR_T;

typedef struct _RTK_Data_Access_T {
	unsigned int type;
	unsigned int index[2];
	unsigned int YUV[3];
} RTK_Data_Access_T;


typedef struct _HAL_VPQ_Panel_Dither_T {
	unsigned char Enable;
	unsigned char default_vip_panel_bit;
	unsigned char nTableType;
} HAL_VPQ_Panel_Dither_T;

//TV030
typedef struct _LC_BLK_Num_Size
{
        unsigned char nBlk_Hnum;
        unsigned char nBlk_Vnum;
}LC_BLK_Num_Size;

typedef struct _LC_BLKAPL
{
       UINT32 iRow;
       UINT32 iCol;
}LC_BLKAPL;

typedef struct _RTK_COLOR_GAMUT_T
{
	unsigned char mode;
	signed short		gamutmatrix_00;		// 3x3 matrix
	signed short		gamutmatrix_01;
	signed short		gamutmatrix_02;
	signed short		gamutmatrix_10;
	signed short		gamutmatrix_11;
	signed short		gamutmatrix_12;
	signed short		gamutmatrix_20;
	signed short		gamutmatrix_21;
	signed short		gamutmatrix_22;

}RTK_COLOR_GAMUT_T;

typedef struct _INTERNAL_PARTTEN
{
	unsigned char enable;
	unsigned short r_Val;
	unsigned short g_Val;
	unsigned short b_Val;

}INTERNAL_PARTTEN;

typedef struct _RTK_BRIGHTNESS_LUT_OETF {
	 unsigned short 	un16_length;
	 unsigned short 	pf_LUT[256];
}RTK_BRIGHTNESS_LUT_OETF;

typedef struct
{
	 short	 BT709Target[3][3];
	 short	 BT2020Target[3][3];
	 short	 DCIP3Target[3][3];
	 short	 ADOBERGBTarget[3][3];	 
}RTK_VPQ_sRGB_Matrix;

typedef struct
{
   UINT8 flag_0;
   UINT8 flag_1;
   UINT8 flag_2;
   UINT8 flag_3;
   UINT8 flag_4;
   UINT8 flag_5;
   UINT8 flag_6;
   UINT8 flag_7;
   UINT8 flag_8;
   UINT8 flag_9;

}RTK_VIP_Flag;

typedef enum _VPQ_EXTERN_GET_DYNAMIC_RANGE_ITEMS {
	VPQ_DYNAMIC_RANGE_SDR = 0,
	VPQ_DYNAMIC_RANGE_DOLBY_VISION,
	VPQ_DYNAMIC_RANGE_HDR10,
	VPQ_DYNAMIC_RANGE_HLG,
	VPQ_DYNAMIC_RANGE_TC,
	VPQ_DYNAMIC_RANGE_HDR10_PlUS,
	VPQ_DYNAMIC_RANGE_HDREFFECT,
	VPQ_DYNAMIC_RANGE_SDR_MAX_RGB,

	VPQ_DYNAMIC_RANGE_HLG_14,
	VPQ_DYNAMIC_RANGE_HLG_18,

	VPQ_EXTERN_GET_DYNAMIC_RANGE_ITEMS_MAX,

} VPQ_EXTERN_GET_DYNAMIC_RANGE_ITEMS;

void vpq_extern_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop);

#endif

