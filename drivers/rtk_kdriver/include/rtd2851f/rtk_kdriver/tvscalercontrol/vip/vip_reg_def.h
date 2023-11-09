#ifndef __VIP_REG_DEF_H__
#define __VIP_REG_DEF_H__

#include "scaler/vipCommon.h"
#include "scaler/vipRPCCommon.h"

#include "rbus/color_reg.h"
#include "rbus/color_dcc_reg.h"
#include "rbus/color_dlcti_reg.h"
#include "rbus/color_icm_reg.h"
#include "rbus/color_sharp_reg.h"
#include "rbus/color_mb_peaking_reg.h"
//#include "rbus/color_mb_su_peaking_reg.h" /* merlin3 added but mac6 removed*/
#include "rbus/con_bri_reg.h"
#include "rbus/di_reg.h"
#include "rbus/dither_reg.h"
#include "rbus/gamma_reg.h"
#include "rbus/histogram_reg.h"
#include "rbus/nr_reg.h"
#include "rbus/mpegnr_reg.h"
#include "rbus/profile_reg.h"
#include "rbus/rgb2yuv_reg.h"
#include "rbus/scaledown_reg.h"
#include "rbus/hsd_dither_reg.h"
#include "rbus/scaleup_reg.h"
#include "rbus/yuv2rgb_reg.h"
#include "rbus/iedge_smooth_reg.h"
#include <rbus/ppoverlay_reg.h>
#include "rbus/od_reg.h"
#include "rbus/od_dma_reg.h"	/*Elsie 20131206*/
#include <rbus/vgip_reg.h>
#include "rbus/outputgamma_reg.h"
#include "rbus/inv_gamma_reg.h"
#include "rbus/vdpq_reg.h"
#include "scaler/vipCommon.h"
#include "scaler/vipRPCCommon.h"
#include "rbus/peaking_reg.h" /*JZ 20140705*/
#include "rbus/color_temp_reg.h"
#include "rbus/color_uvcoring_reg.h"
#include "rbus/lc_reg.h"
#include "rbus/blu_reg.h" /*Hawaii 20140526*/
#include "rbus/ldspi_reg.h"
#include "rbus/pinmux_lvdsphy_reg.h"
#include "rbus/dm_reg.h"/* for hdr lut table, elieli*/
#include "rbus/invoutputgamma_reg.h"	/*jyyang 20160616*/
#include "rbus/rgbw_reg.h"	/*jyyang 20160616*/
#include "rbus/main_dither_reg.h"
#include "rbus/colormap_reg.h"
#ifdef VIP_CONFIG_DUAL_CHENNEL
#include "rbus/sub_dither_reg.h"
#endif //VIP_CONFIG_DUAL_CHENNEL

#include "rbus/c3dlut_reg.h"

#ifdef CONFIG_HW_SUPPORT_I_DE_XC
#include "rbus/de_xcxl_reg.h"
#endif //CONFIG_HW_SUPPORT_I_DE_XC

#include "rbus/ipq_decontour_reg.h"

#ifdef CONFIG_HW_SUPPORT_SLD
#include #include "rbus/color_sld_reg.h"/*juwen 20160616*/
#endif //CONFIG_HW_SUPPORT_SLD

#ifdef CONFIG_HW_SUPPORT_D_HISTOGRAM
#include "rbus/d_histogram_reg.h"/*Terrence 20160701*/
#endif //CONFIG_HW_SUPPORT_D_HISTOGRAM

//#include "rbus/two_step_uzu_reg.h" /*jyyang 20160614*/ //mac6 removed
#include "rbus/demura_reg.h"

#define scaler_rtd_inl		rtd_inl
#define scaler_rtd_outl		rtd_outl
#define scaler_rtd_maskl	rtd_maskl

/*20101224 nick187 , temporarily add def. below for compiler (should be modified after video CPU is ready)------------*/
/**
 * GDE Buffer for Device Driver
 */

typedef struct _SCALERDRV_BUFFERINFO {
		unsigned int	phyAddr;			/* return physical addr to display lib*/
		unsigned int	memSize;			/* memory size*/
		unsigned int	offset;				/* offset of vma*/
		unsigned int	flag;				/* 0: first memory, 1: Second memory*/
} SCALERDRV_BUFFERINFO;

/*#ifdef ENABLE_DCR*/
/**
 * DCR Table
 */
typedef struct _SCALERDRV_DCRTABLE {
		unsigned char DCR_TABLE[5][5];	/* DCR table*/
} SCALERDRV_DCRTABLE;
/*#endif*/

/*#ifdef VIP_AUTOMA_OVER_AP*/
#define New_DCR_TABLE_NUM    3
#define DCR_TABLE_ROW_NUM    6
#define DCR_TABLE_COL_NUM    5
#define DCT_TABLE_NUM    5
typedef struct _ISR_SCALERDRV_DCRTABLE {
		unsigned char nTPV_New_DCR;
		unsigned char nDCRDefault;
		unsigned char DCR_Table[New_DCR_TABLE_NUM][DCR_TABLE_ROW_NUM][DCR_TABLE_COL_NUM];    /*20121121 and*/
		unsigned char DCR_TABLE[DCT_TABLE_NUM][5];	/* DCR table*/
} ISR_SCALERDRV_DCRTABLE;
/*#endif */ /*#ifdef CONFIG_ENABLE_VIP_PSEUDO_RPC*/
/**
 * ioctl commands.
 */


/*-----------------------------------------------------------------------------------------*/
#endif
