/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 *  This file is for peaking coring related functions.
 *
 * @author  $Author$ Flora
 * @date    $Date$ 2007/04/02
 * @version     $Revision$
 */

/**
 * @addtogroup
 * @{
 */

/*============================ Module dependency  ===========================*/
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scalerdrv/mode.h>
#include <tvscalercontrol/scaler/source.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/vip/di_ma.h>
#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <linux/semaphore.h>
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"

/*================================== Variables ==============================*/
unsigned char set_edge_smooth_flag;
unsigned char set_egsm_en;
unsigned char set_shp2D_en;
unsigned char egsm_on2off;
unsigned char egsm_off2on;
unsigned char shp2D_on2off;
unsigned char shp2D_off2on;
/*static unsigned short (* CDS_ini)[5][20] = NULL;*/
extern unsigned char Table_CDS;

/*================================ Definitions ==============================*/

#define CDS_CM_Enable 		0
#define CDS_CM_Gain_Blur 		1
#define CDS_CM_G_Pos_Min		2
#define CDS_CM_G_Pos_Max	3
#define CDS_CM_G_Neg_Min		4
#define CDS_CM_G_Neg_Max	5
#define CDS_CM_HV_POS_Min	6
#define CDS_CM_HV_POS_Max	7
#define CDS_CM_HV_NEG_Min	8
#define CDS_CM_HV_NEG_Max	9
#define CDS_CM_LV_Min		10
#define CDS_CM_LV_Max		11

#define SHARPNESS_LOG 0


#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_PEAKING_DEBUG,fmt,##args)


/*================================== Function ===============================*/
#if 0
signed char drvif_color_check_source(unsigned char display)
{
	unsigned char which_source = 255;
	unsigned char video_format ;
	unsigned char ForceHDMIIntoVIPMode = 0;

	unsigned char input_src_type;
	unsigned char input_src_from;
	unsigned char Frame_Packing_mode = 0;

#ifdef CONFIG_Force_HDMI_RGB_INTO_VIP_Mode
	ForceHDMIIntoVIPMode = 1;
#endif

	input_src_type = Scaler_InputSrcGetType((SCALER_DISP_CHANNEL)display);
	input_src_from = Scaler_InputSrcGetFrom((SCALER_DISP_CHANNEL)display);

	if (input_src_from == _SRC_FROM_VDC) {
		if (input_src_type == _SRC_TV) {
			switch (Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MODE_CURR)) {
			case _MODE_480I: /* NTSC*/
				if (drvif_module_vdc_ReadMode(VDC_SET) == VDC_MODE_PALM)
					which_source = VIP_QUALITY_TV_PAL_M;
				else
					which_source = VIP_QUALITY_TV_NTSC;
				break;
			case _MODE_576I: /* PAL*/
				which_source = VIP_QUALITY_TV_PAL;
				break;
			default: /* PAL*/
				which_source = VIP_QUALITY_TV_PAL;
				break;
			}
		} else if (input_src_type == _SRC_CVBS) {
			switch (Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MODE_CURR)) {
			case _MODE_480I: /* NTSC*/
				if (drvif_module_vdc_ReadMode(VDC_SET) == VDC_MODE_PALM)
					which_source = VIP_QUALITY_CVBS_PAL_M;
				else
					which_source = VIP_QUALITY_CVBS_NTSC;
				break;
			case _MODE_576I:/* PAL*/
				which_source = VIP_QUALITY_CVBS_PAL;
				break;
			default:/* PAL*/
				which_source = VIP_QUALITY_CVBS_PAL;
				break;
			}
		} else if (input_src_type == _SRC_SV) { /* vincent lee---20110113*/
			switch (Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MODE_CURR)) {
			case _MODE_480I: /* NTSC*/
				if (drvif_module_vdc_ReadMode(VDC_SET) == VDC_MODE_PALM)
					which_source = VIP_QUALITY_CVBS_S_PAL_M;
				else
					which_source = VIP_QUALITY_CVBS_S_NTSC;
				break;
			case _MODE_576I: /* PAL*/
				which_source = VIP_QUALITY_CVBS_S_PAL;
				break;
			default: /* PAL*/
				which_source = VIP_QUALITY_CVBS_S_PAL;
				break;
			}
		}
	} else if (input_src_type == _SRC_VGA) {
		which_source = VIP_QUALITY_VGA;
	} else if ((input_src_type == _SRC_VO) && (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 0) &&
		(Scaler_InputSrcFormat(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_DATA_ARRAY_IDX)) == TVD_INFMT_VODMA0)) {
		/* 20110301 peggy: TVD_INFMT_VODMA0 is DTV source, remove fwif_color_isUSBSource*/
		/*(fwif_color_isUSBSource() == FALSE))*/ /*DTV Source */ /* Erin '100920*/

		switch (Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MODE_CURR)) {
		case _MODE_480I:
			which_source = VIP_QUALITY_DTV_480I;
			break;
		case _MODE_576I:
			which_source = VIP_QUALITY_DTV_576I;
			break;
		case  _MODE_480P:
			which_source = VIP_QUALITY_DTV_480P;
			break;
		case _MODE_576P:
			which_source = VIP_QUALITY_DTV_576P;
			break;
		case _MODE_720P50:
			which_source = VIP_QUALITY_DTV_720P;
			break;
		case _MODE_720P60:
			which_source = VIP_QUALITY_DTV_720P;
			break;
		case _MODE_1080I25:
			which_source = VIP_QUALITY_DTV_1080I;
			break;
		case _MODE_1080I30:
			which_source = VIP_QUALITY_DTV_1080I;
			break;
		case _MODE_1080P50:
			which_source = VIP_QUALITY_DTV_1080P;
			break;
		case _MODE_1080P60:
			which_source = VIP_QUALITY_DTV_1080P;
			break;
		default:
			which_source = VIP_QUALITY_DTV_480I;
			break;
		}
	} else if ((input_src_type == _SRC_VO) && (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 0)
		&& Scaler_InputSrcFormat(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_DATA_ARRAY_IDX)) == TVD_INFMT_VODMA1) {
		/* 20110301 peggy: TVD_INFMT_VODMA1 is USB source, remove fwif_color_isUSBSource*/
		/*&& (fwif_color_isUSBSource() == TRUE))*/ /*USB Source */ /* Erin '100920*/

		/*USER: Vincent_Lee  DATE: 2012/06/08  TODO: is JPEG?*/
		if (Scaler_VOFromJPEG(Scaler_Get_CurVoInfo_plane()) == 1)
			which_source = VIP_QUALITY_DTV_JPEG;
		else {
			switch (Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MODE_CURR)) {
			case _MODE_480I:
				which_source = VIP_QUALITY_PVR_480I;
				break;
			case _MODE_576I:
				which_source = VIP_QUALITY_PVR_576I;
				break;
			case  _MODE_480P:
				which_source = VIP_QUALITY_PVR_480P;
				break;
			case _MODE_576P:
				which_source = VIP_QUALITY_PVR_576P;
				break;
			case _MODE_720P50:
				which_source = VIP_QUALITY_PVR_720P;
				break;
			case _MODE_720P60:
				which_source = VIP_QUALITY_PVR_720P;
				break;
			case _MODE_1080I25:
				which_source = VIP_QUALITY_PVR_1080I;
				break;
			case _MODE_1080I30:
				which_source = VIP_QUALITY_PVR_1080I;
				break;
			case _MODE_1080P50:
				which_source = VIP_QUALITY_PVR_1080P;
				break;
			case _MODE_1080P60:
				which_source = VIP_QUALITY_PVR_1080P;
				break;
			default:
				which_source = VIP_QUALITY_PVR_480I;
				break;
			}
		}
	} else if ((input_src_type == _SRC_YPBPR)  || (input_src_type == _SRC_COMPONENT)) { /* vincent lee---20110113*/
		switch (Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MODE_CURR)) {
		case _MODE_480I:
		which_source = VIP_QUALITY_YPbPr_480I;
		break;
		case _MODE_576I:
		which_source = VIP_QUALITY_YPbPr_576I;
		break;
		case  _MODE_480P:
		which_source = VIP_QUALITY_YPbPr_480P;
		break;
		case _MODE_576P:
		which_source = VIP_QUALITY_YPbPr_576P;
		break;
		case _MODE_720P50:
		which_source = VIP_QUALITY_YPbPr_720P;
		break;
		case _MODE_720P60:
		which_source = VIP_QUALITY_YPbPr_720P;
		break;
		case _MODE_1080I25:
		which_source = VIP_QUALITY_YPbPr_1080I;
		break;
		case _MODE_1080I30:
		which_source = VIP_QUALITY_YPbPr_1080I;
		break;
		case _MODE_1080P50:
		which_source = VIP_QUALITY_YPbPr_1080P;
		break;
		case _MODE_1080P60:
		which_source = VIP_QUALITY_YPbPr_1080P;
		break;
		default:
		which_source = VIP_QUALITY_YPbPr_720P;
		break;
		}
	} else {
		video_format = drvif_Hdmi_GetColorSpace();
		#if defined(TV032_1)
		/*zhang_dc 2012.08.28 change for HDMI display of NviDIA display card.*/
		if (drvif_IsHDMI())
		#else
		if (drvif_IsHDMI() && ((drvif_Hdmi_HdcpEnabled()) || (video_format != 0) || (ForceHDMIIntoVIPMode) || (drvif_Hdmi_IsAudioLock())))
		#endif
		{ /* HDMI*/
			/* checkk if HDMI mode, not DVI mode, modify this by Jerry Wu 20100120,*/
			/* add HDCP check 20100224*/
			/* add YUV mode into this mode 20100903 Jerry Wu*/
			switch (Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MODE_CURR)) {
			case _MODE_480I:
				which_source = VIP_QUALITY_HDMI_480I;
				break;
			case _MODE_576I:
				which_source = VIP_QUALITY_HDMI_576I;
				break;
			case  _MODE_480P:
				which_source = VIP_QUALITY_HDMI_480P;
				break;
			case _MODE_576P:
				which_source = VIP_QUALITY_HDMI_576P;
				break;
#if ((defined BUILD_TV005_1_ISDB) || (defined TV_002_100_ATV))
			case _MODE_720P24:
				which_source = VIP_QUALITY_HDMI_720P;
				break;
			case _MODE_720P30:
				which_source = VIP_QUALITY_HDMI_720P;
				break;
#endif
			case _MODE_720P50:
				which_source = VIP_QUALITY_HDMI_720P;
				break;
			case _MODE_720P60:
				which_source = VIP_QUALITY_HDMI_720P;
				break;
			case _MODE_1080I25:
				which_source = VIP_QUALITY_HDMI_1080I;
				break;
			case _MODE_1080I30:
				which_source = VIP_QUALITY_HDMI_1080I;
				break;
			case _MODE_1080P50:
				which_source = VIP_QUALITY_HDMI_1080P;
				break;
			case _MODE_1080P60:
				which_source = VIP_QUALITY_HDMI_1080P;
				break;

			/*USER: Vincent_Lee  DATE: 2011/08/12  TODO: Add new CASE for 1080P24 mode*/
			case _MODE_1080P23:
			case _MODE_1080P24:
			case _MODE_1080P25:
			case _MODE_1080P29:
			case _MODE_1080P30:
				which_source = VIP_QUALITY_HDMI_1080P;
				break;

			/*USER: egger  DATE: 2011/09/21  TODO: Add new CASE for frame packing mode*/
			case _MODE_NEW:

				Frame_Packing_mode = Scaler_Get3D_IsFramePacking();
				if (Frame_Packing_mode == 1)
					which_source = VIP_QUALITY_HDMI_720P;
				else if (Frame_Packing_mode == 2)
					which_source = VIP_QUALITY_HDMI_1080P;
				else
					which_source = VIP_QUALITY_VGA; /* VGA Timing*/
				break;

			default:
				/*which_source = VIP_QUALITY_HDMI_720P;*/
				which_source = VIP_QUALITY_VGA; /* VGA Timing*/
				break;
			}
		} else { /* DVI*/
			which_source = VIP_QUALITY_VGA;
		}

    }
    /*VIPprintf("\n--- __lhh__  -1  Source = %d\n", which_source);*/
    return which_source;
}
#endif

void drvif_color_2dpk_MKIII_init(unsigned char display, unsigned char level)
{

}

void drvif_color_sharpness_adjust(unsigned char display, unsigned char level)
{

}

void drvif_color_edgesmooth_postshp_adjust(unsigned char display, unsigned char level)
{

}

void drvif_color_2dpk_edge_Coef_apply(unsigned char display, unsigned char level)
{

}


void drvif_color_2dpk_Vertical_Coef_apply(unsigned char display, unsigned char level)
{

}

void drvif_color_TwoD_peaking_AdaptiveCtrl(unsigned char display, unsigned char level)
{

}

/*===============================================================*/
void drvif_color_icm_cds_cm_off(void)
{

	#if 0 /* cds need new driver cychen2*/
	color_sharp_dm_cds_cm1_u_bound_0_RBUS color_sharp_dm_cds_cm1_u_bound_0_reg;
	color_sharp_dm_cds_cm1_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_0_reg);
	color_sharp_dm_cds_cm1_u_bound_0_reg.cds_enable = 0;
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_0_reg, color_sharp_dm_cds_cm1_u_bound_0_reg.regValue);
	#endif
}




void drvif_color_set_cds_table(VIP_CDS_Table *ptr)
{
	color_sharp_dm_cds_peaking_gain_bound_ctrl_RBUS	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg;
	color_sharp_dm_cds_cm0_u_bound_0_RBUS			color_sharp_dm_cds_cm0_u_bound_0_reg;
	color_sharp_dm_cds_cm0_v_bound_0_RBUS			color_sharp_dm_cds_cm0_v_bound_0_reg;
	color_sharp_dm_cds_cm0_u_bound_1_RBUS			color_sharp_dm_cds_cm0_u_bound_1_reg;
	color_sharp_dm_cds_cm0_v_bound_1_RBUS			color_sharp_dm_cds_cm0_v_bound_1_reg;
	color_sharp_dm_cds_peaking_gain_cm0_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm0_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm0_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm0_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm0_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm0_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm0_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm0_edge_reg;
	color_sharp_dm_cds_cm1_u_bound_0_RBUS			color_sharp_dm_cds_cm1_u_bound_0_reg;
	color_sharp_dm_cds_cm1_v_bound_0_RBUS			color_sharp_dm_cds_cm1_v_bound_0_reg;
	color_sharp_dm_cds_cm1_u_bound_1_RBUS			color_sharp_dm_cds_cm1_u_bound_1_reg;
	color_sharp_dm_cds_cm1_v_bound_1_RBUS			color_sharp_dm_cds_cm1_v_bound_1_reg;
	color_sharp_dm_cds_peaking_gain_cm1_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm1_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm1_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm1_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm1_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm1_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm1_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm1_edge_reg;
	color_sharp_dm_cds_cm2_u_bound_0_RBUS			color_sharp_dm_cds_cm2_u_bound_0_reg;
	color_sharp_dm_cds_cm2_v_bound_0_RBUS			color_sharp_dm_cds_cm2_v_bound_0_reg;
	color_sharp_dm_cds_cm2_u_bound_1_RBUS			color_sharp_dm_cds_cm2_u_bound_1_reg;
	color_sharp_dm_cds_cm2_v_bound_1_RBUS			color_sharp_dm_cds_cm2_v_bound_1_reg;
	color_sharp_dm_cds_peaking_gain_cm2_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm2_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm2_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm2_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm2_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm2_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm2_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm2_edge_reg;
	color_sharp_dm_cds_cm3_u_bound_0_RBUS			color_sharp_dm_cds_cm3_u_bound_0_reg;
	color_sharp_dm_cds_cm3_v_bound_0_RBUS			color_sharp_dm_cds_cm3_v_bound_0_reg;
	color_sharp_dm_cds_cm3_u_bound_1_RBUS			color_sharp_dm_cds_cm3_u_bound_1_reg;
	color_sharp_dm_cds_cm3_v_bound_1_RBUS			color_sharp_dm_cds_cm3_v_bound_1_reg;
	color_sharp_dm_cds_peaking_gain_cm3_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm3_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm3_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm3_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm3_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm3_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm3_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm3_edge_reg;
	// Vertical CDS since Merlin2
	color_sharp_dm_cds_peaking_gain_cm0_v_RBUS	color_sharp_dm_cds_peaking_gain_cm0_v_reg;
	color_sharp_dm_cds_peaking_bound_cm0_v_RBUS	color_sharp_dm_cds_peaking_bound_cm0_v_reg;
	color_sharp_dm_cds_peaking_gain_cm0_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg;
	color_sharp_dm_cds_peaking_gain_cm1_v_RBUS	color_sharp_dm_cds_peaking_gain_cm1_v_reg;
	color_sharp_dm_cds_peaking_bound_cm1_v_RBUS	color_sharp_dm_cds_peaking_bound_cm1_v_reg;
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg;
	color_sharp_dm_cds_peaking_gain_cm2_v_RBUS	color_sharp_dm_cds_peaking_gain_cm2_v_reg;
	color_sharp_dm_cds_peaking_bound_cm2_v_RBUS	color_sharp_dm_cds_peaking_bound_cm2_v_reg;
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg;
	color_sharp_dm_cds_peaking_gain_cm3_v_RBUS	color_sharp_dm_cds_peaking_gain_cm3_v_reg;
	color_sharp_dm_cds_peaking_bound_cm3_v_RBUS	color_sharp_dm_cds_peaking_bound_cm3_v_reg;
	color_sharp_dm_cds_peaking_gain_cm3_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg;
	/* Read from register*/
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_BOUND_CTRL_reg);
	color_sharp_dm_cds_cm0_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_0_reg);
	color_sharp_dm_cds_cm0_v_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM0_V_BOUND_0_reg);
	color_sharp_dm_cds_cm0_u_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_1_reg);
	color_sharp_dm_cds_cm0_v_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM0_V_BOUND_1_reg);
	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_EDGE_reg);

	color_sharp_dm_cds_cm1_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_0_reg);
	color_sharp_dm_cds_cm1_v_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_V_BOUND_0_reg);
	color_sharp_dm_cds_cm1_u_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_1_reg);
	color_sharp_dm_cds_cm1_v_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_V_BOUND_1_reg);
	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_EDGE_reg);

	color_sharp_dm_cds_cm2_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_0_reg);
	color_sharp_dm_cds_cm2_v_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM2_V_BOUND_0_reg);
	color_sharp_dm_cds_cm2_u_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_1_reg);
	color_sharp_dm_cds_cm2_v_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM2_V_BOUND_1_reg);
	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_EDGE_reg);

	color_sharp_dm_cds_cm3_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM3_U_BOUND_0_reg);
	color_sharp_dm_cds_cm3_v_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM3_V_BOUND_0_reg);
	color_sharp_dm_cds_cm3_u_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM3_U_BOUND_1_reg);
	color_sharp_dm_cds_cm3_v_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM3_V_BOUND_1_reg);
	color_sharp_dm_cds_peaking_gain_cm3_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm3_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_EDGE_reg);

	color_sharp_dm_cds_peaking_gain_cm0_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_reg);
	color_sharp_dm_cds_peaking_bound_cm0_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_reg);
	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_EDG_reg);
	color_sharp_dm_cds_peaking_gain_cm1_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_reg);
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_reg);
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_EDG_reg);
	color_sharp_dm_cds_peaking_gain_cm2_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_reg);
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_reg);
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_EDG_reg);
	color_sharp_dm_cds_peaking_gain_cm3_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_V_reg);
	color_sharp_dm_cds_peaking_bound_cm3_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_V_reg);
	color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_V_EDG_reg);

	/* Global*/
	color_sharp_dm_cds_cm0_u_bound_0_reg.cds_enable = ptr->CDS_Global_Ctrl.cds_enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_edg_enable = ptr->CDS_Global_Ctrl.cds_edg_enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_tex_enable = ptr->CDS_Global_Ctrl.cds_tex_enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_v_enable = ptr->CDS_Global_Ctrl.cds_v_enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_v_edg_enable = ptr->CDS_Global_Ctrl.cds_v_edg_enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_lpf_weight_enable = ptr->CDS_Global_Ctrl.cds_lpf_weight_enable;

	/* CM0*/
	color_sharp_dm_cds_cm0_u_bound_0_reg.u_lb_0 = ptr->CDS_CM0_Ctrl.u_lb;
	color_sharp_dm_cds_cm0_u_bound_0_reg.u_ub_0 = ptr->CDS_CM0_Ctrl.u_ub;
	color_sharp_dm_cds_cm0_v_bound_0_reg.v_lb_0 = ptr->CDS_CM0_Ctrl.v_lb;
	color_sharp_dm_cds_cm0_v_bound_0_reg.v_ub_0 = ptr->CDS_CM0_Ctrl.v_ub;
	color_sharp_dm_cds_cm0_u_bound_1_reg.cu_0   = ptr->CDS_CM0_Ctrl.cu;
	color_sharp_dm_cds_cm0_u_bound_1_reg.cv_0   = ptr->CDS_CM0_Ctrl.cv;
	color_sharp_dm_cds_cm0_u_bound_1_reg.uv_rad_0 = ptr->CDS_CM0_Ctrl.uv_rad;

	color_sharp_dm_cds_cm0_v_bound_0_reg.cm0_conti_area = ptr->CDS_CM0_Ctrl.cm_conti_area;
	color_sharp_dm_cds_cm0_v_bound_0_reg.cm0_conti_enable = ptr->CDS_CM0_Ctrl.cm_conti_enable;
	color_sharp_dm_cds_cm0_u_bound_0_reg.cds_cm0_enable = ptr->CDS_CM0_Ctrl.cds_cm_enable;
	color_sharp_dm_cds_cm0_v_bound_0_reg.cm0_mode = ptr->CDS_CM0_Ctrl.cm_mode;

	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.gain_pos = ptr->CDS_CM0_Ctrl.tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.gain_neg = ptr->CDS_CM0_Ctrl.tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.lv = ptr->CDS_CM0_Ctrl.tex_lv;
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.hv_pos = ptr->CDS_CM0_Ctrl.tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.hv_neg = ptr->CDS_CM0_Ctrl.tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.gain_pos = ptr->CDS_CM0_Ctrl.edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.gain_neg = ptr->CDS_CM0_Ctrl.edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.lv = ptr->CDS_CM0_Ctrl.edge_lv;
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.hv_pos = ptr->CDS_CM0_Ctrl.edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.hv_neg = ptr->CDS_CM0_Ctrl.edge_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm0_v_reg.gain_pos = ptr->CDS_CM0_Ctrl.v_tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm0_v_reg.gain_neg = ptr->CDS_CM0_Ctrl.v_tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm0_v_reg.lv = ptr->CDS_CM0_Ctrl.v_tex_lv;
	color_sharp_dm_cds_peaking_bound_cm0_v_reg.hv_pos = ptr->CDS_CM0_Ctrl.v_tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm0_v_reg.hv_neg = ptr->CDS_CM0_Ctrl.v_tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.gain_pos = ptr->CDS_CM0_Ctrl.v_edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.gain_neg = ptr->CDS_CM0_Ctrl.v_edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.lv = ptr->CDS_CM0_Ctrl.v_edge_lv;
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.hv_pos = ptr->CDS_CM0_Ctrl.v_edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.hv_neg = ptr->CDS_CM0_Ctrl.v_edge_hv_neg;

	/* CM1*/
	color_sharp_dm_cds_cm1_u_bound_0_reg.u_lb_1 = ptr->CDS_CM1_Ctrl.u_lb;
	color_sharp_dm_cds_cm1_u_bound_0_reg.u_ub_1 = ptr->CDS_CM1_Ctrl.u_ub;
	color_sharp_dm_cds_cm1_v_bound_0_reg.v_lb_1 = ptr->CDS_CM1_Ctrl.v_lb;
	color_sharp_dm_cds_cm1_v_bound_0_reg.v_ub_1 = ptr->CDS_CM1_Ctrl.v_ub;
	color_sharp_dm_cds_cm1_u_bound_1_reg.cu_1   = ptr->CDS_CM1_Ctrl.cu;
	color_sharp_dm_cds_cm1_u_bound_1_reg.cv_1   = ptr->CDS_CM1_Ctrl.cv;
	color_sharp_dm_cds_cm1_u_bound_1_reg.uv_rad_1 = ptr->CDS_CM1_Ctrl.uv_rad;

	color_sharp_dm_cds_cm1_v_bound_0_reg.cm1_conti_area = ptr->CDS_CM1_Ctrl.cm_conti_area;
	color_sharp_dm_cds_cm1_v_bound_0_reg.cm1_conti_enable = ptr->CDS_CM1_Ctrl.cm_conti_enable;
	color_sharp_dm_cds_cm1_u_bound_0_reg.cds_cm1_enable = ptr->CDS_CM1_Ctrl.cds_cm_enable;
	color_sharp_dm_cds_cm1_v_bound_0_reg.cm1_mode = ptr->CDS_CM1_Ctrl.cm_mode;

	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.gain_pos = ptr->CDS_CM1_Ctrl.tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.gain_neg = ptr->CDS_CM1_Ctrl.tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.lv = ptr->CDS_CM1_Ctrl.tex_lv;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.hv_pos = ptr->CDS_CM1_Ctrl.tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.hv_neg = ptr->CDS_CM1_Ctrl.tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.gain_pos = ptr->CDS_CM1_Ctrl.edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.gain_neg = ptr->CDS_CM1_Ctrl.edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.lv = ptr->CDS_CM1_Ctrl.edge_lv;
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.hv_pos = ptr->CDS_CM1_Ctrl.edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.hv_neg = ptr->CDS_CM1_Ctrl.edge_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm1_v_reg.gain_pos = ptr->CDS_CM1_Ctrl.v_tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm1_v_reg.gain_neg = ptr->CDS_CM1_Ctrl.v_tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.lv = ptr->CDS_CM1_Ctrl.v_tex_lv;
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.hv_pos = ptr->CDS_CM1_Ctrl.v_tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.hv_neg = ptr->CDS_CM1_Ctrl.v_tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.gain_pos = ptr->CDS_CM1_Ctrl.v_edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.gain_neg = ptr->CDS_CM1_Ctrl.v_edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.lv = ptr->CDS_CM1_Ctrl.v_edge_lv;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.hv_pos = ptr->CDS_CM1_Ctrl.v_edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.hv_neg = ptr->CDS_CM1_Ctrl.v_edge_hv_neg;

	/* CM2*/
	color_sharp_dm_cds_cm2_u_bound_0_reg.u_lb_2 = ptr->CDS_CM2_Ctrl.u_lb;
	color_sharp_dm_cds_cm2_u_bound_0_reg.u_ub_2 = ptr->CDS_CM2_Ctrl.u_ub;
	color_sharp_dm_cds_cm2_v_bound_0_reg.v_lb_2 = ptr->CDS_CM2_Ctrl.v_lb;
	color_sharp_dm_cds_cm2_v_bound_0_reg.v_ub_2 = ptr->CDS_CM2_Ctrl.v_ub;
	color_sharp_dm_cds_cm2_u_bound_1_reg.cu_2   = ptr->CDS_CM2_Ctrl.cu;
	color_sharp_dm_cds_cm2_u_bound_1_reg.cv_2   = ptr->CDS_CM2_Ctrl.cv;
	color_sharp_dm_cds_cm2_u_bound_1_reg.uv_rad_2 = ptr->CDS_CM2_Ctrl.uv_rad;

	color_sharp_dm_cds_cm2_v_bound_0_reg.cm2_conti_area = ptr->CDS_CM2_Ctrl.cm_conti_area;
	color_sharp_dm_cds_cm2_v_bound_0_reg.cm2_conti_enable = ptr->CDS_CM2_Ctrl.cm_conti_enable;
	color_sharp_dm_cds_cm2_u_bound_0_reg.cds_cm2_enable = ptr->CDS_CM2_Ctrl.cds_cm_enable;
	color_sharp_dm_cds_cm2_v_bound_0_reg.cm2_mode = ptr->CDS_CM2_Ctrl.cm_mode;

	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.gain_pos = ptr->CDS_CM2_Ctrl.tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.gain_neg = ptr->CDS_CM2_Ctrl.tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.lv = ptr->CDS_CM2_Ctrl.tex_lv;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.hv_pos = ptr->CDS_CM2_Ctrl.tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.hv_neg = ptr->CDS_CM2_Ctrl.tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.gain_pos = ptr->CDS_CM2_Ctrl.edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.gain_neg = ptr->CDS_CM2_Ctrl.edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.lv = ptr->CDS_CM2_Ctrl.edge_lv;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.hv_pos = ptr->CDS_CM2_Ctrl.edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.hv_neg = ptr->CDS_CM2_Ctrl.edge_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm2_v_reg.gain_pos = ptr->CDS_CM2_Ctrl.v_tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm2_v_reg.gain_neg = ptr->CDS_CM2_Ctrl.v_tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.lv = ptr->CDS_CM2_Ctrl.v_tex_lv;
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.hv_pos = ptr->CDS_CM2_Ctrl.v_tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.hv_neg = ptr->CDS_CM2_Ctrl.v_tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.gain_pos = ptr->CDS_CM2_Ctrl.v_edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.gain_neg = ptr->CDS_CM2_Ctrl.v_edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.lv = ptr->CDS_CM2_Ctrl.v_edge_lv;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.hv_pos = ptr->CDS_CM2_Ctrl.v_edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.hv_neg = ptr->CDS_CM2_Ctrl.v_edge_hv_neg;

	/* CM3*/
	color_sharp_dm_cds_cm3_u_bound_0_reg.u_lb_3 = ptr->CDS_CM3_Ctrl.u_lb;
	color_sharp_dm_cds_cm3_u_bound_0_reg.u_ub_3 = ptr->CDS_CM3_Ctrl.u_ub;
	color_sharp_dm_cds_cm3_v_bound_0_reg.v_lb_3 = ptr->CDS_CM3_Ctrl.v_lb;
	color_sharp_dm_cds_cm3_v_bound_0_reg.v_ub_3 = ptr->CDS_CM3_Ctrl.v_ub;
	color_sharp_dm_cds_cm3_u_bound_1_reg.cu_3   = ptr->CDS_CM3_Ctrl.cu;
	color_sharp_dm_cds_cm3_u_bound_1_reg.cv_3   = ptr->CDS_CM3_Ctrl.cv;
	color_sharp_dm_cds_cm3_u_bound_1_reg.uv_rad_3 = ptr->CDS_CM3_Ctrl.uv_rad;

	color_sharp_dm_cds_cm3_v_bound_0_reg.cm3_conti_area = ptr->CDS_CM3_Ctrl.cm_conti_area;
	color_sharp_dm_cds_cm3_v_bound_0_reg.cm3_conti_enable = ptr->CDS_CM3_Ctrl.cm_conti_enable;
	color_sharp_dm_cds_cm3_u_bound_0_reg.cds_cm3_enable = ptr->CDS_CM3_Ctrl.cds_cm_enable;
	color_sharp_dm_cds_cm3_v_bound_0_reg.cm3_mode = ptr->CDS_CM3_Ctrl.cm_mode;

	color_sharp_dm_cds_peaking_gain_cm3_tex_reg.gain_pos = ptr->CDS_CM3_Ctrl.tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm3_tex_reg.gain_neg = ptr->CDS_CM3_Ctrl.tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.lv = ptr->CDS_CM3_Ctrl.tex_lv;
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.hv_pos = ptr->CDS_CM3_Ctrl.tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.hv_neg = ptr->CDS_CM3_Ctrl.tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm3_edge_reg.gain_pos = ptr->CDS_CM3_Ctrl.edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm3_edge_reg.gain_neg = ptr->CDS_CM3_Ctrl.edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.lv = ptr->CDS_CM3_Ctrl.edge_lv;
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.hv_pos = ptr->CDS_CM3_Ctrl.edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.hv_neg = ptr->CDS_CM3_Ctrl.edge_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm3_v_reg.gain_pos = ptr->CDS_CM3_Ctrl.v_tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm3_v_reg.gain_neg = ptr->CDS_CM3_Ctrl.v_tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm3_v_reg.lv = ptr->CDS_CM3_Ctrl.v_tex_lv;
	color_sharp_dm_cds_peaking_bound_cm3_v_reg.hv_pos = ptr->CDS_CM3_Ctrl.v_tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm3_v_reg.hv_neg = ptr->CDS_CM3_Ctrl.v_tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg.gain_pos = ptr->CDS_CM3_Ctrl.v_edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg.gain_neg = ptr->CDS_CM3_Ctrl.v_edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.lv = ptr->CDS_CM3_Ctrl.v_edge_lv;
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.hv_pos = ptr->CDS_CM3_Ctrl.v_edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.hv_neg = ptr->CDS_CM3_Ctrl.v_edge_hv_neg;

	/* Write to register*/
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_BOUND_CTRL_reg, color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_0_reg, color_sharp_dm_cds_cm0_u_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM0_V_BOUND_0_reg, color_sharp_dm_cds_cm0_v_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_1_reg, color_sharp_dm_cds_cm0_u_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM0_V_BOUND_1_reg, color_sharp_dm_cds_cm0_v_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_TEX_reg, color_sharp_dm_cds_peaking_gain_cm0_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_TEX_reg, color_sharp_dm_cds_peaking_bound_cm0_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm0_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm0_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_0_reg, color_sharp_dm_cds_cm1_u_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_V_BOUND_0_reg, color_sharp_dm_cds_cm1_v_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_1_reg, color_sharp_dm_cds_cm1_u_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_V_BOUND_1_reg, color_sharp_dm_cds_cm1_v_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_TEX_reg, color_sharp_dm_cds_peaking_gain_cm1_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_TEX_reg, color_sharp_dm_cds_peaking_bound_cm1_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm1_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm1_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_0_reg, color_sharp_dm_cds_cm2_u_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM2_V_BOUND_0_reg, color_sharp_dm_cds_cm2_v_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_1_reg, color_sharp_dm_cds_cm2_u_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM2_V_BOUND_1_reg, color_sharp_dm_cds_cm2_v_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_TEX_reg, color_sharp_dm_cds_peaking_gain_cm2_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_TEX_reg, color_sharp_dm_cds_peaking_bound_cm2_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm2_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm2_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_CM3_U_BOUND_0_reg, color_sharp_dm_cds_cm3_u_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM3_V_BOUND_0_reg, color_sharp_dm_cds_cm3_v_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM3_U_BOUND_1_reg, color_sharp_dm_cds_cm3_u_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM3_V_BOUND_1_reg, color_sharp_dm_cds_cm3_v_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_TEX_reg, color_sharp_dm_cds_peaking_gain_cm3_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_TEX_reg, color_sharp_dm_cds_peaking_bound_cm3_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm3_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm3_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_reg, color_sharp_dm_cds_peaking_gain_cm0_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_reg, color_sharp_dm_cds_peaking_bound_cm0_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_EDG_reg, color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_EDG_reg, color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_reg, color_sharp_dm_cds_peaking_gain_cm1_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_reg, color_sharp_dm_cds_peaking_bound_cm1_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_EDG_reg, color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_EDG_reg, color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_reg, color_sharp_dm_cds_peaking_gain_cm2_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_reg, color_sharp_dm_cds_peaking_bound_cm2_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_EDG_reg, color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_EDG_reg, color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_V_reg, color_sharp_dm_cds_peaking_gain_cm3_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_V_reg, color_sharp_dm_cds_peaking_bound_cm3_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_V_EDG_reg, color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_V_EDG_reg, color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.regValue);

}

void drvif_color_set_CDS_CM0_level(DRV_Sharpness_Level *ptr)
{
	color_sharp_dm_cds_peaking_gain_cm0_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm0_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm0_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm0_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm0_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm0_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm0_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm0_edge_reg;

	color_sharp_dm_cds_peaking_gain_cm0_v_RBUS	color_sharp_dm_cds_peaking_gain_cm0_v_reg;
	color_sharp_dm_cds_peaking_bound_cm0_v_RBUS	color_sharp_dm_cds_peaking_bound_cm0_v_reg;
	color_sharp_dm_cds_peaking_gain_cm0_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg;

	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_EDGE_reg);
	
	color_sharp_dm_cds_peaking_gain_cm0_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_reg);
	color_sharp_dm_cds_peaking_bound_cm0_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_reg);
	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_EDG_reg);

	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.gain_pos = ptr->Texture_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.gain_neg = ptr->Texture_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.hv_pos = ptr->Texture_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.hv_neg = ptr->Texture_Level.HV_NEG_Gain;

	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.gain_pos = ptr->Edge_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.gain_neg = ptr->Edge_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.hv_pos = ptr->Edge_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.hv_neg = ptr->Edge_Level.HV_NEG_Gain;

	color_sharp_dm_cds_peaking_gain_cm0_v_reg.gain_pos = ptr->Vertical_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm0_v_reg.gain_neg = ptr->Vertical_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm0_v_reg.hv_pos = ptr->Vertical_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm0_v_reg.hv_neg = ptr->Vertical_Level.HV_NEG_Gain;

	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.gain_pos = ptr->Vertical_Edge_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.gain_neg = ptr->Vertical_Edge_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.hv_pos = ptr->Vertical_Edge_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.hv_neg = ptr->Vertical_Edge_Level.HV_NEG_Gain;

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_TEX_reg, color_sharp_dm_cds_peaking_gain_cm0_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_TEX_reg, color_sharp_dm_cds_peaking_bound_cm0_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm0_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm0_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_reg, color_sharp_dm_cds_peaking_gain_cm0_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_reg, color_sharp_dm_cds_peaking_bound_cm0_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_EDG_reg, color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_EDG_reg, color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.regValue);
}

void drvif_color_set_CDS_CM1_level(DRV_Sharpness_Level *ptr)
{
	color_sharp_dm_cds_peaking_gain_cm1_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm1_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm1_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm1_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm1_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm1_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm1_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm1_edge_reg;

	color_sharp_dm_cds_peaking_gain_cm1_v_RBUS	color_sharp_dm_cds_peaking_gain_cm1_v_reg;
	color_sharp_dm_cds_peaking_bound_cm1_v_RBUS	color_sharp_dm_cds_peaking_bound_cm1_v_reg;
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg;

	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_EDGE_reg);
	
	color_sharp_dm_cds_peaking_gain_cm1_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_reg);
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_reg);
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_EDG_reg);

	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.gain_pos = ptr->Texture_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.gain_neg = ptr->Texture_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.hv_pos = ptr->Texture_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.hv_neg = ptr->Texture_Level.HV_NEG_Gain;

	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.gain_pos = ptr->Edge_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.gain_neg = ptr->Edge_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.hv_pos = ptr->Edge_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.hv_neg = ptr->Edge_Level.HV_NEG_Gain;

	color_sharp_dm_cds_peaking_gain_cm1_v_reg.gain_pos = ptr->Vertical_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm1_v_reg.gain_neg = ptr->Vertical_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.hv_pos = ptr->Vertical_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.hv_neg = ptr->Vertical_Level.HV_NEG_Gain;

	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.gain_pos = ptr->Vertical_Edge_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.gain_neg = ptr->Vertical_Edge_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.hv_pos = ptr->Vertical_Edge_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.hv_neg = ptr->Vertical_Edge_Level.HV_NEG_Gain;

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_TEX_reg, color_sharp_dm_cds_peaking_gain_cm1_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_TEX_reg, color_sharp_dm_cds_peaking_bound_cm1_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm1_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm1_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_reg, color_sharp_dm_cds_peaking_gain_cm1_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_reg, color_sharp_dm_cds_peaking_bound_cm1_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_EDG_reg, color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_EDG_reg, color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.regValue);
}

void drvif_color_set_CDS_CM2_level(DRV_Sharpness_Level *ptr)
{
	color_sharp_dm_cds_peaking_gain_cm2_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm2_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm2_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm2_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm2_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm2_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm2_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm2_edge_reg;

	color_sharp_dm_cds_peaking_gain_cm2_v_RBUS	color_sharp_dm_cds_peaking_gain_cm2_v_reg;
	color_sharp_dm_cds_peaking_bound_cm2_v_RBUS	color_sharp_dm_cds_peaking_bound_cm2_v_reg;
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg;

	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_EDGE_reg);
	
	color_sharp_dm_cds_peaking_gain_cm2_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_reg);
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_reg);
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_EDG_reg);

	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.gain_pos = ptr->Texture_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.gain_neg = ptr->Texture_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.hv_pos = ptr->Texture_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.hv_neg = ptr->Texture_Level.HV_NEG_Gain;

	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.gain_pos = ptr->Edge_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.gain_neg = ptr->Edge_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.hv_pos = ptr->Edge_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.hv_neg = ptr->Edge_Level.HV_NEG_Gain;

	color_sharp_dm_cds_peaking_gain_cm2_v_reg.gain_pos = ptr->Vertical_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm2_v_reg.gain_neg = ptr->Vertical_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.hv_pos = ptr->Vertical_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.hv_neg = ptr->Vertical_Level.HV_NEG_Gain;

	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.gain_pos = ptr->Vertical_Edge_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.gain_neg = ptr->Vertical_Edge_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.hv_pos = ptr->Vertical_Edge_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.hv_neg = ptr->Vertical_Edge_Level.HV_NEG_Gain;

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_TEX_reg, color_sharp_dm_cds_peaking_gain_cm2_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_TEX_reg, color_sharp_dm_cds_peaking_bound_cm2_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm2_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm2_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_reg, color_sharp_dm_cds_peaking_gain_cm2_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_reg, color_sharp_dm_cds_peaking_bound_cm2_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_EDG_reg, color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_EDG_reg, color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.regValue);
}

void drvif_color_set_CDS_CM3_level(DRV_Sharpness_Level *ptr)
{
	color_sharp_dm_cds_peaking_gain_cm3_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm3_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm3_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm3_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm3_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm3_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm3_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm3_edge_reg;

	color_sharp_dm_cds_peaking_gain_cm3_v_RBUS	color_sharp_dm_cds_peaking_gain_cm3_v_reg;
	color_sharp_dm_cds_peaking_bound_cm3_v_RBUS	color_sharp_dm_cds_peaking_bound_cm3_v_reg;
	color_sharp_dm_cds_peaking_gain_cm3_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg;

	color_sharp_dm_cds_peaking_gain_cm3_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm3_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_EDGE_reg);
	
	color_sharp_dm_cds_peaking_gain_cm3_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_V_reg);
	color_sharp_dm_cds_peaking_bound_cm3_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_V_reg);
	color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_V_EDG_reg);

	color_sharp_dm_cds_peaking_gain_cm3_tex_reg.gain_pos = ptr->Texture_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm3_tex_reg.gain_neg = ptr->Texture_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.hv_pos = ptr->Texture_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.hv_neg = ptr->Texture_Level.HV_NEG_Gain;

	color_sharp_dm_cds_peaking_gain_cm3_edge_reg.gain_pos = ptr->Edge_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm3_edge_reg.gain_neg = ptr->Edge_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.hv_pos = ptr->Edge_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.hv_neg = ptr->Edge_Level.HV_NEG_Gain;

	color_sharp_dm_cds_peaking_gain_cm3_v_reg.gain_pos = ptr->Vertical_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm3_v_reg.gain_neg = ptr->Vertical_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm3_v_reg.hv_pos = ptr->Vertical_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm3_v_reg.hv_neg = ptr->Vertical_Level.HV_NEG_Gain;

	color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg.gain_pos = ptr->Vertical_Edge_Level.G_Pos_Gain;
	color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg.gain_neg = ptr->Vertical_Edge_Level.G_Neg_Gain;
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.hv_pos = ptr->Vertical_Edge_Level.HV_POS_Gain;
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.hv_neg = ptr->Vertical_Edge_Level.HV_NEG_Gain;

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_TEX_reg, color_sharp_dm_cds_peaking_gain_cm3_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_TEX_reg, color_sharp_dm_cds_peaking_bound_cm3_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm3_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm3_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_V_reg, color_sharp_dm_cds_peaking_gain_cm3_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_V_reg, color_sharp_dm_cds_peaking_bound_cm3_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_V_EDG_reg, color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_V_EDG_reg, color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.regValue);
}

void drvif_color_emfmk2(DRV_EMFMk2 *ptr)
{
	color_sharp_dm_emfmk2_ctrl_RBUS color_sharp_dm_emfmk2_ctrl_reg;

	if (!ptr)
		return;

	color_sharp_dm_emfmk2_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_EMFMK2_CTRL_reg);

	color_sharp_dm_emfmk2_ctrl_reg.emf_mode_sel = ptr->Emf_Model;
	color_sharp_dm_emfmk2_ctrl_reg.lowbnd = ptr->Lowbnd;

	IoReg_Write32(COLOR_SHARP_DM_EMFMK2_CTRL_reg, color_sharp_dm_emfmk2_ctrl_reg.regValue);

}


/**
 * Sharpness
 * Set default register value
**/

void drvif_color_sharpness_init(unsigned char display)
{
	/*enable functions*/
	color_sharp_dm_nr_shp_ctrl_RBUS color_sharp_dm_nr_shp_ctrl_reg;

	color_sharp_dm_emfmk2_ctrl_RBUS color_sharp_dm_emfmk2_ctrl_reg;


	/*enable functions*/
	color_sharp_dm_nr_shp_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_NR_SHP_CTRL_reg);
	/*color_sharp_dm_nr_shp_ctrl_reg.clp_enable = 1;*/
	/*color_sharp_dm_nr_shp_ctrl_reg.emf_enable = 0;*/
//	color_sharp_dm_nr_shp_ctrl_reg.edg_emf_enable = 0;		//tv006 hal function control
//	color_sharp_dm_nr_shp_ctrl_reg.tex_emf_enable = 0;		//tv006 hal function control
	color_sharp_dm_nr_shp_ctrl_reg.peaking_enable = 1;
	IoReg_Write32(COLOR_SHARP_DM_NR_SHP_CTRL_reg, color_sharp_dm_nr_shp_ctrl_reg.regValue);

	/*CLP*/
/*	color_sharp_dm_chroma_lowpass_RBUS color_sharp_dm_chroma_lowpass_reg;*/
/*	color_sharp_dm_chroma_lowpass_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CHROMA_LOWPASS_reg);*/
/*	color_sharp_dm_chroma_lowpass_reg.blur_fac = 8;//CLP_BLUR_FAC;*/
/*	IoReg_Write32(COLOR_SHARP_DM_CHROMA_LOWPASS_reg, color_sharp_dm_chroma_lowpass_reg.regValue);*/

	/*-=== emfmk2 ===-*/
	color_sharp_dm_emfmk2_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_EMFMK2_CTRL_reg);

	/*--ic default---*/

	color_sharp_dm_emfmk2_ctrl_reg.declinebit = 0;
	color_sharp_dm_emfmk2_ctrl_reg.blendstepbit = 5;
	color_sharp_dm_emfmk2_ctrl_reg.lowbnd = 0x64;
	color_sharp_dm_emfmk2_ctrl_reg.aov_range = 0;
	color_sharp_dm_emfmk2_ctrl_reg.emf_mk2_debug = 0;
	color_sharp_dm_emfmk2_ctrl_reg.emf_mode_sel = 0;

	IoReg_Write32(COLOR_SHARP_DM_EMFMK2_CTRL_reg, color_sharp_dm_emfmk2_ctrl_reg.regValue);

}

void drvif_color_Sharpness_Vertical_Gain_adjust(unsigned char display, unsigned char level)
{

		color_sharp_dm_segpk_vpk2_RBUS color_sharp_dm_segpk_vpk2_reg;
		color_sharp_dm_segpk_vpk2_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK2_reg);
		/*color_sharp_dm_segpk_vpk2_reg.vpk_gain_neg = level << 4;*/
		/*color_sharp_dm_segpk_vpk2_reg.vpk_gain_pos = level << 4;*/
		/*color_sharp_dm_segpk_vpk2_reg.vpk_lv = 5;*/
		IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK2_reg, color_sharp_dm_segpk_vpk2_reg.regValue);

}

void drvif_color_set_Vertical_Peaking_Enable(unsigned char bEnable)
{
	color_sharp_dm_segpk_vpk4_RBUS color_sharp_dm_segpk_vpk4;
	color_sharp_dm_segpk_vpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK4_reg);

	if (bEnable == true)
		color_sharp_dm_segpk_vpk4.vpk_en = bEnable;
	else
		color_sharp_dm_segpk_vpk4.vpk_en = 0;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK4_reg, color_sharp_dm_segpk_vpk4.regValue);
}


void drvif_color_2dpk_init(unsigned char display, unsigned char level)
{

	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*level = 0 ->old  1: -> new*/
	color_sharp_dm_segpk_ctrl1_RBUS color_sharp_dm_segpk_ctrl1;
	color_sharp_dm_segpk_ctrl2_RBUS color_sharp_dm_segpk_ctrl2;
	color_sharp_dm_segpk_ctrl3_RBUS color_sharp_dm_segpk_ctrl3;
	color_sharp_dm_segpk_vpk1_RBUS color_sharp_dm_segpk_vpk1;
	color_sharp_dm_segpk_vpk2_RBUS color_sharp_dm_segpk_vpk2;
	color_sharp_dm_segpk_vpk3_RBUS color_sharp_dm_segpk_vpk3;
	color_sharp_dm_segpk_vpk4_RBUS color_sharp_dm_segpk_vpk4;
	/*color_sharp_dm_segpk_edgpk1_RBUS color_sharp_dm_segpk_edgpk1;*/
	color_sharp_dm_segpk_edgpk2_RBUS color_sharp_dm_segpk_edgpk2;
	color_sharp_dm_segpk_edgpk3_RBUS color_sharp_dm_segpk_edgpk3;
	color_sharp_dm_segpk_edgpk4_RBUS color_sharp_dm_segpk_edgpk4;
	color_sharp_shp_dlti_ctrl_1_RBUS color_sharp_shp_dlti_ctrl_1;
	unsigned char Scaler_3DMode = 0;

	color_sharp_dm_segpk_ctrl1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL1_reg);
	color_sharp_dm_segpk_ctrl2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL2_reg);
	color_sharp_dm_segpk_ctrl3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL3_reg);
	color_sharp_dm_segpk_vpk1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK1_reg);
	color_sharp_dm_segpk_vpk2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK2_reg);
	color_sharp_dm_segpk_vpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK3_reg);
	color_sharp_dm_segpk_vpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK4_reg);
/*	color_sharp_dm_segpk_edgpk1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK1_reg);*/
	color_sharp_dm_segpk_edgpk2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK2_reg);
	color_sharp_dm_segpk_edgpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK3_reg);
	color_sharp_dm_segpk_edgpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK4_reg);
	color_sharp_shp_dlti_ctrl_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_CTRL_1_reg);
	/*shp_dlti_ctrl_2.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_CTRL_2_reg);*/
	/*shp_dlti_ctrl_3.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_CTRL_3_reg);*/
	/*shp_dlti_gain_offset_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_1_reg);*/
	/*shp_dlti_gain_offset_2.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_2_reg);*/
	/*shp_dlti_gain_offset_3.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_3_reg);*/
	/*shp_dlti_gain_offset_4.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_4_reg);*/
	/*shp_dlti_gain_offset_5.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_5_reg);*/

	/*color_sharp_dm_segpk_ctrl1.enhance_mode = 2; */ /* pk + dlti*/
	color_sharp_dm_segpk_ctrl1.edgextrange = 0x6;
	color_sharp_dm_segpk_ctrl1.sobel_stepbit = 0x2;

	if ((VIP_system_info_structure_table->Input_src_Type) == _SRC_YPBPR || (VIP_system_info_structure_table->Input_src_Type) == _SRC_HDMI) {
		color_sharp_dm_segpk_ctrl1.sobel_upbnd = 0x78;
	} else if ((VIP_system_info_structure_table->Input_src_Type) == _SRC_CVBS) {
		color_sharp_dm_segpk_ctrl1.sobel_upbnd = 0x64;
	} else {
		color_sharp_dm_segpk_ctrl1.sobel_upbnd = 0x50;
	}
	color_sharp_dm_segpk_ctrl1.sobelmode = 0;
	color_sharp_dm_segpk_ctrl1.edgmode = 0;
	color_sharp_dm_segpk_ctrl1.texpk_en = 1;
	color_sharp_dm_segpk_ctrl1.edgpk_en = 1;
	// removed color_sharp_dm_segpk_ctrl1.edgext_en = 1;
	/*color_sharp_dm_segpk_ctrl1.segpk_mode = 2;*/

	color_sharp_dm_segpk_ctrl2.pkdlti_comsign = 0;
	/*color_sharp_dm_segpk_ctrl2.pkdlti_difsign = 0;*/
	color_sharp_dm_segpk_ctrl2.zigsftbit_reg = 1;
	color_sharp_dm_segpk_ctrl2.zdiff_stepbit = 0;
	color_sharp_dm_segpk_ctrl2.zdiff_upbnd = 64;

	color_sharp_dm_segpk_ctrl3.tex_en = 1;
	// removed color_sharp_dm_segpk_ctrl3.actsftbit_reg = 1;
	color_sharp_dm_segpk_ctrl3.tex_stepbit = 0;
	color_sharp_dm_segpk_ctrl3.tex_upbnd = 0xa;

	color_sharp_dm_segpk_vpk1.vpk_c0 = 126;
	color_sharp_dm_segpk_vpk1.vpk_c1 = 10;
	color_sharp_dm_segpk_vpk1.vpk_c2 = (unsigned char)-73;
	color_sharp_dm_segpk_vpk2.vpk_lv = 7;
	color_sharp_dm_segpk_vpk2.vpk_gain_pos = 0x30;
	color_sharp_dm_segpk_vpk2.vpk_gain_neg = 0x30;
	color_sharp_dm_segpk_vpk3.vpk_hv_neg = 0x40;
	color_sharp_dm_segpk_vpk3.vpk_hv_pos = 0x40;
	color_sharp_dm_segpk_vpk4.vemf_en = 1;
	color_sharp_dm_segpk_vpk2.v_tex_en = 1;
	color_sharp_dm_segpk_vpk4.vext_reg = 8;
	/*color_sharp_dm_segpk_vpk4.vpk_en = 1;*/

	/*color_sharp_dm_segpk_edgpk1.c0 = 64;*/
	/*color_sharp_dm_segpk_edgpk1.c1 = 0;*/
	/*color_sharp_dm_segpk_edgpk1.sh_b0 = 0;*/
	/*color_sharp_dm_segpk_edgpk1.sh_b1 = 0;*/
	/*color_sharp_dm_segpk_edgpk2.c2 = 0;*/
	/*color_sharp_dm_segpk_edgpk2.c3 = 0;*/
	/*color_sharp_dm_segpk_edgpk2.c4 = 0;*/
	/*color_sharp_dm_segpk_edgpk2.c5 =(unsigned char)-32;*/
	/*color_sharp_dm_segpk_edgpk3.gain_blr = 0;*/
	color_sharp_dm_segpk_edgpk3.gain_pos = 0xff;
	color_sharp_dm_segpk_edgpk3.gain_neg = 0xff;
	color_sharp_dm_segpk_edgpk4.hv_pos = 512;
	color_sharp_dm_segpk_edgpk4.hv_neg = 512;
	color_sharp_dm_segpk_edgpk4.lv = 16;

	color_sharp_shp_dlti_ctrl_1.dlti_en = 1;

	if (level == 0) { /* old*/
		color_sharp_dm_segpk_ctrl1.segpk_mode = 0; /* pk_tex only*/
		// removed color_sharp_dm_segpk_ctrl1.enhance_mode = 0; /* pk*/
		color_sharp_dm_segpk_vpk4.vpk_en = 0;
	} else { /* new*/
		color_sharp_dm_segpk_ctrl1.segpk_mode = 2;
		// removed color_sharp_dm_segpk_ctrl1.enhance_mode = 2; /* pk + dlti*/
		Scaler_3DMode = Scaler_Get3DMode((SCALER_DISP_CHANNEL)display);/*check 3d mode*/
		VIPprintf("Sharpness Scaler_3DMode =%d \n", Scaler_3DMode);

		if ((Scaler_3DMode == SLR_3DMODE_2D)
			|| (Scaler_3DMode == SLR_3DMODE_3D_AUTO_CVT_2D)
			|| (Scaler_3DMode == SLR_3DMODE_3D_SBS_CVT_2D)
			|| (Scaler_3DMode == SLR_3DMODE_3D_TB_CVT_2D)
			|| (Scaler_3DMode == SLR_3DMODE_3D_FP_CVT_2D)) {
				color_sharp_dm_segpk_vpk4.vpk_en = 1;
				/*flag_vPeakingEn = 1;*/
		} else { /* 3d mode in d domain*/
				color_sharp_dm_segpk_vpk4.vpk_en = 0;
				/*flag_vPeakingEn = 0;*/
		}
	}


	IoReg_Write32(COLOR_SHARP_DM_SEGPK_CTRL1_reg, color_sharp_dm_segpk_ctrl1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_CTRL2_reg, color_sharp_dm_segpk_ctrl2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_CTRL3_reg, color_sharp_dm_segpk_ctrl3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK1_reg, color_sharp_dm_segpk_vpk1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK2_reg, color_sharp_dm_segpk_vpk2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK3_reg, color_sharp_dm_segpk_vpk3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK4_reg, color_sharp_dm_segpk_vpk4.regValue);
	/*IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK1_reg, color_sharp_dm_segpk_edgpk1.regValue);*/
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK2_reg, color_sharp_dm_segpk_edgpk2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK3_reg, color_sharp_dm_segpk_edgpk3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK4_reg, color_sharp_dm_segpk_edgpk4.regValue);

	IoReg_Write32(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, color_sharp_shp_dlti_ctrl_1.regValue);
	/*IoReg_Write32(COLOR_SHARP_SHP_DLTI_CTRL_2_reg, shp_dlti_ctrl_2.regValue);*/
	/*IoReg_Write32(COLOR_SHARP_SHP_DLTI_CTRL_3_reg, shp_dlti_ctrl_3.regValue);*/
	/*IoReg_Write32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_1_reg, shp_dlti_gain_offset_1.regValue);*/
	/*IoReg_Write32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_2_reg, shp_dlti_gain_offset_2.regValue);*/
	/*IoReg_Write32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_3_reg, shp_dlti_gain_offset_3.regValue);*/
	/*IoReg_Write32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_4_reg, shp_dlti_gain_offset_4.regValue);*/
	/*IoReg_Write32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_5_reg, shp_dlti_gain_offset_5.regValue);*/
}

void drvif_color_2dpk_seg(DRV_2Dpk_Seg *ptr)
{
	color_sharp_dm_segpk_ctrl1_RBUS color_sharp_dm_segpk_ctrl1;
	color_sharp_dm_segpk_ctrl2_RBUS color_sharp_dm_segpk_ctrl2;

	if (!ptr)
		return;

	color_sharp_dm_segpk_ctrl1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL1_reg);
	color_sharp_dm_segpk_ctrl2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL2_reg);

	color_sharp_dm_segpk_ctrl1.segpk_mode =  ptr->Segpk_Mode;
	color_sharp_dm_segpk_ctrl1.sobel_upbnd = ptr->Sobel_Upbnd;
	color_sharp_dm_segpk_ctrl2.zdiff_upbnd = ptr->Zdiff_Upbnd;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_CTRL1_reg, color_sharp_dm_segpk_ctrl1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_CTRL2_reg, color_sharp_dm_segpk_ctrl2.regValue);

}
void drvif_color_2dpk_tex_detect(DRV_2Dpk_Tex_Detect *ptr)
{
	color_sharp_dm_segpk_ctrl3_RBUS color_sharp_dm_segpk_ctrl3;

	if (!ptr)
		return;

	color_sharp_dm_segpk_ctrl3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL3_reg);

	color_sharp_dm_segpk_ctrl3.tex_upbnd = ptr->Tex_Upbnd;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_CTRL3_reg, color_sharp_dm_segpk_ctrl3.regValue);

}


#if 0
void drvif_color_shp_dlti(DRV_Shp_Dlti *ptr)
{
	VIPprintf("Dear ZhiMing : The real function is drvif_color_Set_D_DLTI_Table. We apologize for your inconvenience.");
}
	#endif

void drvif_color_set_Sharpness_level(DRV_Sharpness_Level *ptr)
{
	color_sharp_dm_nr_shp_ctrl_RBUS color_sharp_dm_nr_shp_ctrl_reg;


	/*Set Edge Gain*/
	color_sharp_dm_segpk_edgpk3_RBUS color_sharp_dm_segpk_edgpk3;
	color_sharp_dm_segpk_edgpk4_RBUS color_sharp_dm_segpk_edgpk4;

	/*Set Texture Gain*/
	color_sharp_dm_peaking_gain_RBUS color_sharp_dm_peaking_gain_reg;
	color_sharp_dm_peaking_bound_1_RBUS color_sharp_dm_peaking_bound_1_reg;

	/* vertical gain*/
	color_sharp_dm_segpk_vpk2_RBUS color_sharp_dm_segpk_vpk2;
	color_sharp_dm_segpk_vpk3_RBUS color_sharp_dm_segpk_vpk3;

	/* vertical edge gain*/
	color_sharp_dm_segpk_vpk5_RBUS color_sharp_dm_segpk_vpk5;
	color_sharp_dm_segpk_vpk6_RBUS color_sharp_dm_segpk_vpk6;

	color_sharp_dm_nr_shp_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_NR_SHP_CTRL_reg);
	color_sharp_dm_nr_shp_ctrl_reg.db_en=1;
	color_sharp_dm_nr_shp_ctrl_reg.db_load=0;
	IoReg_Write32(COLOR_SHARP_DM_NR_SHP_CTRL_reg,color_sharp_dm_nr_shp_ctrl_reg.regValue);


	/*Set Edge Gain*/
	color_sharp_dm_segpk_edgpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK3_reg);
	color_sharp_dm_segpk_edgpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK4_reg);

	color_sharp_dm_segpk_edgpk3.gain_pos = ptr->Edge_Level.G_Pos_Gain;
	color_sharp_dm_segpk_edgpk3.gain_neg = ptr->Edge_Level.G_Neg_Gain;
	color_sharp_dm_segpk_edgpk4.hv_pos = ptr->Edge_Level.HV_POS_Gain;
	color_sharp_dm_segpk_edgpk4.hv_neg = ptr->Edge_Level.HV_NEG_Gain;
	color_sharp_dm_segpk_edgpk4.lv = ptr->Edge_Level.LV_Gain;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK3_reg, color_sharp_dm_segpk_edgpk3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK4_reg, color_sharp_dm_segpk_edgpk4.regValue);

	/*Set Texture Gain*/
	color_sharp_dm_peaking_gain_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_GAIN_reg);
	color_sharp_dm_peaking_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_BOUND_1_reg);

	color_sharp_dm_peaking_gain_reg.gain_pos = ptr->Texture_Level.G_Pos_Gain;
	color_sharp_dm_peaking_gain_reg.gain_neg = ptr->Texture_Level.G_Neg_Gain;
	color_sharp_dm_peaking_bound_1_reg.hv_pos = ptr->Texture_Level.HV_POS_Gain;
	color_sharp_dm_peaking_bound_1_reg.hv_neg = ptr->Texture_Level.HV_NEG_Gain;
	color_sharp_dm_peaking_bound_1_reg.lv = ptr->Texture_Level.LV_Gain;

	IoReg_Write32(COLOR_SHARP_DM_PEAKING_GAIN_reg, color_sharp_dm_peaking_gain_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_PEAKING_BOUND_1_reg, color_sharp_dm_peaking_bound_1_reg.regValue);

	/* vertical gain*/
	color_sharp_dm_segpk_vpk2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK2_reg);
	color_sharp_dm_segpk_vpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK3_reg);

	color_sharp_dm_segpk_vpk2.vpk_gain_pos = ptr->Vertical_Level.G_Pos_Gain;
	color_sharp_dm_segpk_vpk2.vpk_gain_neg = ptr->Vertical_Level.G_Neg_Gain;
	color_sharp_dm_segpk_vpk2.vpk_lv = ptr->Vertical_Level.LV_Gain;
	color_sharp_dm_segpk_vpk3.vpk_hv_pos = ptr->Vertical_Level.HV_POS_Gain;
	color_sharp_dm_segpk_vpk3.vpk_hv_neg = ptr->Vertical_Level.HV_NEG_Gain;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK2_reg, color_sharp_dm_segpk_vpk2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK3_reg, color_sharp_dm_segpk_vpk3.regValue);


	/* vertical edge gain*/
	color_sharp_dm_segpk_vpk5.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK5_reg);
	color_sharp_dm_segpk_vpk6.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK6_reg);

	color_sharp_dm_segpk_vpk5.vpk_edg_gain_pos= ptr->Vertical_Edge_Level.G_Pos_Gain;
	color_sharp_dm_segpk_vpk5.vpk_edg_gain_neg = ptr->Vertical_Edge_Level.G_Neg_Gain;
	color_sharp_dm_segpk_vpk5.vpk_edg_lv = ptr->Vertical_Edge_Level.LV_Gain;
	color_sharp_dm_segpk_vpk6.vpk_edg_hv_pos = ptr->Vertical_Edge_Level.HV_POS_Gain;
	color_sharp_dm_segpk_vpk6.vpk_edg_hv_neg = ptr->Vertical_Edge_Level.HV_NEG_Gain;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK5_reg, color_sharp_dm_segpk_vpk5.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK6_reg, color_sharp_dm_segpk_vpk6.regValue);



	color_sharp_dm_nr_shp_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_NR_SHP_CTRL_reg);
	color_sharp_dm_nr_shp_ctrl_reg.db_en=1;
	color_sharp_dm_nr_shp_ctrl_reg.db_load=1;
	IoReg_Write32(COLOR_SHARP_DM_NR_SHP_CTRL_reg,color_sharp_dm_nr_shp_ctrl_reg.regValue);


}
void drvif_color_get_Sharpness_level(DRV_Sharpness_Level *ptr)
{

        /*Set Edge Gain*/
        color_sharp_dm_segpk_edgpk3_RBUS color_sharp_dm_segpk_edgpk3;
        color_sharp_dm_segpk_edgpk4_RBUS color_sharp_dm_segpk_edgpk4;

        /*Set Texture Gain*/
        color_sharp_dm_peaking_gain_RBUS color_sharp_dm_peaking_gain_reg;
        color_sharp_dm_peaking_bound_1_RBUS color_sharp_dm_peaking_bound_1_reg;

        /* vertical gain*/
        color_sharp_dm_segpk_vpk2_RBUS color_sharp_dm_segpk_vpk2;
        color_sharp_dm_segpk_vpk3_RBUS color_sharp_dm_segpk_vpk3;

        /* vertical edge gain*/
        color_sharp_dm_segpk_vpk5_RBUS color_sharp_dm_segpk_vpk5;
        color_sharp_dm_segpk_vpk6_RBUS color_sharp_dm_segpk_vpk6;


        /*Set Edge Gain*/
        color_sharp_dm_segpk_edgpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK3_reg);
        color_sharp_dm_segpk_edgpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK4_reg);

        ptr->Edge_Level.G_Pos_Gain = color_sharp_dm_segpk_edgpk3.gain_pos;
        ptr->Edge_Level.G_Neg_Gain = color_sharp_dm_segpk_edgpk3.gain_neg;
        ptr->Edge_Level.HV_POS_Gain = color_sharp_dm_segpk_edgpk4.hv_pos;
        ptr->Edge_Level.HV_NEG_Gain = color_sharp_dm_segpk_edgpk4.hv_neg;
        ptr->Edge_Level.LV_Gain = color_sharp_dm_segpk_edgpk4.lv;


        /*Set Texture Gain*/
        color_sharp_dm_peaking_gain_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_GAIN_reg);
        color_sharp_dm_peaking_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_BOUND_1_reg);

        ptr->Texture_Level.G_Pos_Gain = color_sharp_dm_peaking_gain_reg.gain_pos ;
        ptr->Texture_Level.G_Neg_Gain = color_sharp_dm_peaking_gain_reg.gain_neg;
        ptr->Texture_Level.HV_POS_Gain = color_sharp_dm_peaking_bound_1_reg.hv_pos;
        ptr->Texture_Level.HV_NEG_Gain = color_sharp_dm_peaking_bound_1_reg.hv_neg;
        ptr->Texture_Level.LV_Gain = color_sharp_dm_peaking_bound_1_reg.lv;


        /* vertical gain*/
        color_sharp_dm_segpk_vpk2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK2_reg);
        color_sharp_dm_segpk_vpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK3_reg);

        ptr->Vertical_Level.G_Pos_Gain = color_sharp_dm_segpk_vpk2.vpk_gain_pos;
        ptr->Vertical_Level.G_Neg_Gain = color_sharp_dm_segpk_vpk2.vpk_gain_neg;
        ptr->Vertical_Level.LV_Gain = color_sharp_dm_segpk_vpk2.vpk_lv;
        ptr->Vertical_Level.HV_POS_Gain = color_sharp_dm_segpk_vpk3.vpk_hv_pos;
        ptr->Vertical_Level.HV_NEG_Gain = color_sharp_dm_segpk_vpk3.vpk_hv_neg;


        /* vertical edge gain*/
        color_sharp_dm_segpk_vpk5.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK5_reg);
        color_sharp_dm_segpk_vpk6.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK6_reg);

        ptr->Vertical_Edge_Level.G_Pos_Gain = color_sharp_dm_segpk_vpk5.vpk_edg_gain_pos;
        ptr->Vertical_Edge_Level.G_Neg_Gain = color_sharp_dm_segpk_vpk5.vpk_edg_gain_neg;
        ptr->Vertical_Edge_Level.LV_Gain = color_sharp_dm_segpk_vpk5.vpk_edg_lv;
        ptr->Vertical_Edge_Level.HV_POS_Gain = color_sharp_dm_segpk_vpk6.vpk_edg_hv_pos;
        ptr->Vertical_Edge_Level.HV_NEG_Gain = color_sharp_dm_segpk_vpk6.vpk_edg_hv_neg;

}

void drvif_color_set_Sharpness_level_only_gain(DRV_Sharpness_Level *ptr)
{
	color_sharp_dm_nr_shp_ctrl_RBUS color_sharp_dm_nr_shp_ctrl_reg;

	/*Set Edge Gain*/
	color_sharp_dm_segpk_edgpk3_RBUS color_sharp_dm_segpk_edgpk3;

	/*Set Texture Gain*/
	color_sharp_dm_peaking_gain_RBUS color_sharp_dm_peaking_gain_reg;

	/* vertical gain*/
	color_sharp_dm_segpk_vpk2_RBUS color_sharp_dm_segpk_vpk2;

	/* vertical edge gain*/
	color_sharp_dm_segpk_vpk5_RBUS color_sharp_dm_segpk_vpk5;

	/*Set Edge Gain*/
	color_sharp_dm_segpk_edgpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK3_reg);

	color_sharp_dm_segpk_edgpk3.gain_pos = ptr->Edge_Level.G_Pos_Gain;
	color_sharp_dm_segpk_edgpk3.gain_neg = ptr->Edge_Level.G_Neg_Gain;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK3_reg, color_sharp_dm_segpk_edgpk3.regValue);

	/*Set Texture Gain*/
	color_sharp_dm_peaking_gain_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_GAIN_reg);

	color_sharp_dm_peaking_gain_reg.gain_pos = ptr->Texture_Level.G_Pos_Gain;
	color_sharp_dm_peaking_gain_reg.gain_neg = ptr->Texture_Level.G_Neg_Gain;

	IoReg_Write32(COLOR_SHARP_DM_PEAKING_GAIN_reg, color_sharp_dm_peaking_gain_reg.regValue);

	/* vertical gain*/
	color_sharp_dm_segpk_vpk2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK2_reg);

	color_sharp_dm_segpk_vpk2.vpk_gain_pos = ptr->Vertical_Level.G_Pos_Gain;
	color_sharp_dm_segpk_vpk2.vpk_gain_neg = ptr->Vertical_Level.G_Neg_Gain;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK2_reg, color_sharp_dm_segpk_vpk2.regValue);

	/* vertical edge gain*/
	color_sharp_dm_segpk_vpk5.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK5_reg);

	color_sharp_dm_segpk_vpk5.vpk_edg_gain_pos= ptr->Vertical_Edge_Level.G_Pos_Gain;
	color_sharp_dm_segpk_vpk5.vpk_edg_gain_neg = ptr->Vertical_Edge_Level.G_Neg_Gain;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK5_reg, color_sharp_dm_segpk_vpk5.regValue);

	color_sharp_dm_nr_shp_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_NR_SHP_CTRL_reg);
	color_sharp_dm_nr_shp_ctrl_reg.db_en=1;
	color_sharp_dm_nr_shp_ctrl_reg.db_load=1;
	IoReg_Write32(COLOR_SHARP_DM_NR_SHP_CTRL_reg,color_sharp_dm_nr_shp_ctrl_reg.regValue);


}

void drvif_color_Set_Sharpness_db_apply_1(void)
{
	color_sharp_dm_nr_shp_ctrl_1_RBUS reg_color_sharp_dm_nr_shp_ctrl_1_reg;
	reg_color_sharp_dm_nr_shp_ctrl_1_reg.regValue=IoReg_Read32(COLOR_SHARP_DM_NR_SHP_CTRL_1_reg);
	reg_color_sharp_dm_nr_shp_ctrl_1_reg.db_load=1;
	IoReg_Write32(COLOR_SHARP_DM_NR_SHP_CTRL_1_reg,reg_color_sharp_dm_nr_shp_ctrl_1_reg.regValue);
}

void drvif_color_Set_Sharpness_PowerSaving_2D_tex(DRV_Sharpness_Table *ptr)
{
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
	color_sharp_dm_2d_shp_tex_0_RBUS	color_sharp_dm_2d_shp_tex_0;
	color_sharp_dm_2d_shp_tex_1_RBUS	color_sharp_dm_2d_shp_tex_1;
	color_sharp_dm_2d_shp_tex_1_1_RBUS	color_sharp_dm_2d_shp_tex_1_1;
	color_sharp_dm_2d_shp_tex_2_RBUS	color_sharp_dm_2d_shp_tex_2;
	color_sharp_dm_2d_shp_tex_4_RBUS	color_sharp_dm_2d_shp_tex_4;
	color_sharp_dm_2d_shp_tex_6_RBUS	color_sharp_dm_2d_shp_tex_6;
	color_sharp_dm_2d_shp_tex_7_RBUS	color_sharp_dm_2d_shp_tex_7;
	/*DC Value Check*/
	if (SHARPNESS_LOG) {

		int sum;
		sum = 0;
		sum = ptr->TwoD_tex.C0 +
		      2*(ptr->TwoD_tex.C1+ptr->TwoD_tex.C2+ptr->TwoD_tex.C3+ptr->TwoD_tex.C4+ptr->TwoD_tex.C5) +
		      2*(ptr->TwoD_tex.C6+ptr->TwoD_tex.C7+ptr->TwoD_tex.C8+ptr->TwoD_tex.C9+ptr->TwoD_tex.C10) +
		      2*(ptr->TwoD_tex.C0H1+ptr->TwoD_tex.C0H2+ptr->TwoD_tex.C0H3+ptr->TwoD_tex.C0H4);
		if (sum != 0 || sum !=4<<(ptr->Texture_Shp.Edg_D2_Shift_Bit))
			VIPprintf("[Warnning] [%s-%s-%d] [Text Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);

	}
	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
	color_sharp_dm_2d_shp_tex_0.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_0_reg);
	color_sharp_dm_2d_shp_tex_1.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_1_reg);
	color_sharp_dm_2d_shp_tex_1_1.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_1_1_reg);
	color_sharp_dm_2d_shp_tex_2.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_2_reg);
	color_sharp_dm_2d_shp_tex_4.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_4_reg);
	color_sharp_dm_2d_shp_tex_6.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_6_reg);
	color_sharp_dm_2d_shp_tex_7.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_7_reg);
	color_sharp_dm_sr_ctrl.h_tex_filter_en= ptr->TwoD_tex.en_tex;
	IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl.regValue);
	color_sharp_dm_2d_shp_tex_0.coef_c0_0 = ptr->TwoD_tex.C0;
	color_sharp_dm_2d_shp_tex_0.coef_c1_0 = ptr->TwoD_tex.C1;
	color_sharp_dm_2d_shp_tex_0.coef_c2_0 = ptr->TwoD_tex.C2;
	color_sharp_dm_2d_shp_tex_1.coef_c3_0 = ptr->TwoD_tex.C3;
	color_sharp_dm_2d_shp_tex_1.coef_c4_0 = ptr->TwoD_tex.C4;
	color_sharp_dm_2d_shp_tex_1.coef_c5_0 = ptr->TwoD_tex.C5;
	color_sharp_dm_2d_shp_tex_1.coef_c6_0 = ptr->TwoD_tex.C6;
	color_sharp_dm_2d_shp_tex_1_1.coef_c7_0 = ptr->TwoD_tex.C7;
	color_sharp_dm_2d_shp_tex_1_1.coef_c8_0 = ptr->TwoD_tex.C8;
	color_sharp_dm_2d_shp_tex_1_1.coef_c9_0 = ptr->TwoD_tex.C9;
	color_sharp_dm_2d_shp_tex_1_1.coef_c10_0 = ptr->TwoD_tex.C10;
	// Merlin3, 2D filters shrink to 1D1D
	color_sharp_dm_2d_shp_tex_2.coef_c0_1 = ptr->TwoD_tex.C0H1;
	color_sharp_dm_2d_shp_tex_4.coef_c0_2 = ptr->TwoD_tex.C0H2;
	color_sharp_dm_2d_shp_tex_6.coef_c0_3 = ptr->TwoD_tex.C0H3;
	color_sharp_dm_2d_shp_tex_7.coef_c0_4 = ptr->TwoD_tex.C0H4;
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_0_reg, color_sharp_dm_2d_shp_tex_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_1_reg, color_sharp_dm_2d_shp_tex_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_1_1_reg, color_sharp_dm_2d_shp_tex_1_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_2_reg, color_sharp_dm_2d_shp_tex_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_4_reg, color_sharp_dm_2d_shp_tex_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_6_reg, color_sharp_dm_2d_shp_tex_6.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_7_reg, color_sharp_dm_2d_shp_tex_7.regValue);
/*	VIPprintf("==== texture =====%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", ptr->MK3_Table.Text_C0, ptr->MK3_Table.Text_C1, ptr->MK3_Table.Text_C2, ptr->MK3_Table.Text_C3, ptr->MK3_Table.Text_C4, ptr->MK3_Table.Text_C5,*/
/*		ptr->MK3_Table.Text_C6, ptr->MK3_Table.Text_C7, ptr->MK3_Table.Text_C8, ptr->MK3_Table.Text_C9, ptr->MK3_Table.Text_C10, ptr->MK3_Table.Text_C11, ptr->MK3_Table.Text_C12, ptr->MK3_Table.Text_C13, ptr->MK3_Table.Text_C14);*/
}

void drvif_color_Set_Sharpness_PowerSaving_2D_edg(DRV_Sharpness_Table *ptr)
{
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
	color_sharp_dm_2d_shp_edge_0_RBUS	color_sharp_dm_2d_shp_edge_0;
	color_sharp_dm_2d_shp_edge_1_RBUS	color_sharp_dm_2d_shp_edge_1;
	color_sharp_dm_2d_shp_edge_1_1_RBUS	color_sharp_dm_2d_shp_edge_1_1;
	color_sharp_dm_2d_shp_edge_2_RBUS	color_sharp_dm_2d_shp_edge_2;
	color_sharp_dm_2d_shp_edge_4_RBUS	color_sharp_dm_2d_shp_edge_4;
	color_sharp_dm_2d_shp_edge_6_RBUS	color_sharp_dm_2d_shp_edge_6;
	color_sharp_dm_2d_shp_edge_7_RBUS	color_sharp_dm_2d_shp_edge_7;
	/*DC Value Check*/
	if (SHARPNESS_LOG) {

		int sum;
		sum = 0;
		sum = ptr->TwoD_edg.C0 +
		      2*(ptr->TwoD_edg.C1+ptr->TwoD_edg.C2+ptr->TwoD_edg.C3+ptr->TwoD_edg.C4+ptr->TwoD_edg.C5) +
		      2*(ptr->TwoD_edg.C6+ptr->TwoD_edg.C7+ptr->TwoD_edg.C8+ptr->TwoD_edg.C9+ptr->TwoD_edg.C10) +
		      2*(ptr->TwoD_edg.C0H1+ptr->TwoD_edg.C0H2+ptr->TwoD_edg.C0H3+ptr->TwoD_edg.C0H4);
		if (sum != 0 || sum !=4<<(ptr->Edge_Shp.Edg_D2_Shift_Bit))
			VIPprintf("[Warnning] [%s-%s-%d] [Edge Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);

	}
	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
	color_sharp_dm_2d_shp_edge_0.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_0_reg);
	color_sharp_dm_2d_shp_edge_1.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_1_reg);
	color_sharp_dm_2d_shp_edge_1_1.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_1_1_reg);
	color_sharp_dm_2d_shp_edge_2.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_2_reg);
	color_sharp_dm_2d_shp_edge_4.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_4_reg);
	color_sharp_dm_2d_shp_edge_6.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_6_reg);
	color_sharp_dm_2d_shp_edge_7.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_7_reg);

	color_sharp_dm_sr_ctrl.h_edg_filter_en= ptr->TwoD_edg.en_edg;
	color_sharp_dm_2d_shp_edge_0.coef_c0_0 = ptr->TwoD_edg.C0;
	color_sharp_dm_2d_shp_edge_0.coef_c1_0 = ptr->TwoD_edg.C1;
	color_sharp_dm_2d_shp_edge_0.coef_c2_0 = ptr->TwoD_edg.C2;
	color_sharp_dm_2d_shp_edge_1.coef_c3_0 = ptr->TwoD_edg.C3;
	color_sharp_dm_2d_shp_edge_1.coef_c4_0 = ptr->TwoD_edg.C4;
	color_sharp_dm_2d_shp_edge_1.coef_c5_0 = ptr->TwoD_edg.C5;
	color_sharp_dm_2d_shp_edge_1.coef_c6_0 = ptr->TwoD_edg.C6;
	color_sharp_dm_2d_shp_edge_1_1.coef_c7_0 = ptr->TwoD_edg.C7;
	color_sharp_dm_2d_shp_edge_1_1.coef_c8_0 = ptr->TwoD_edg.C8;
	color_sharp_dm_2d_shp_edge_1_1.coef_c9_0 = ptr->TwoD_edg.C9;
	color_sharp_dm_2d_shp_edge_1_1.coef_c10_0 = ptr->TwoD_edg.C10;
	// Merlin3, 2D filters shrink to 1D1D
	color_sharp_dm_2d_shp_edge_2.coef_c0_1 = ptr->TwoD_edg.C0H1;
	color_sharp_dm_2d_shp_edge_4.coef_c0_2 = ptr->TwoD_edg.C0H2;
	color_sharp_dm_2d_shp_edge_6.coef_c0_3 = ptr->TwoD_edg.C0H3;
	color_sharp_dm_2d_shp_edge_7.coef_c0_4 = ptr->TwoD_edg.C0H4;
	IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_0_reg, color_sharp_dm_2d_shp_edge_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_1_reg, color_sharp_dm_2d_shp_edge_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_1_1_reg, color_sharp_dm_2d_shp_edge_1_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_2_reg, color_sharp_dm_2d_shp_edge_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_4_reg, color_sharp_dm_2d_shp_edge_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_6_reg, color_sharp_dm_2d_shp_edge_6.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_7_reg, color_sharp_dm_2d_shp_edge_7.regValue);
}

void drvif_color_Set_Sharpness_PowerSaving_SR_H0(DRV_Sharpness_Table *ptr)
{
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
	color_sharp_dm_sr_9tap_hpf_h_coef_1_RBUS	color_sharp_dm_sr_9tap_hpf_h_coef_1;

	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
	color_sharp_dm_sr_9tap_hpf_h_coef_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_h_coef_1_reg);

	color_sharp_dm_sr_ctrl.sr_0_tex_en= ptr->SR_H9.en_tex;
	color_sharp_dm_sr_ctrl.sr_0_edg_en= ptr->SR_H9.en_edg;
	color_sharp_dm_sr_9tap_hpf_h_coef_1.c1 = ptr->SR_H9.C1;
	color_sharp_dm_sr_9tap_hpf_h_coef_1.c2 = ptr->SR_H9.C2;
	color_sharp_dm_sr_9tap_hpf_h_coef_1.c3 = ptr->SR_H9.C3;
	color_sharp_dm_sr_9tap_hpf_h_coef_1.c4 = ptr->SR_H9.C4;

	IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_9tap_hpf_h_coef_1_reg, color_sharp_dm_sr_9tap_hpf_h_coef_1.regValue);
}

void drvif_color_Set_Sharpness_PowerSaving_SR_H1(DRV_Sharpness_Table *ptr)
{
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
	color_sharp_dm_sr_13tap_hpf_h_coef_0_RBUS	color_sharp_dm_sr_13tap_hpf_h_coef_0;
	color_sharp_dm_sr_13tap_hpf_h_coef_1_RBUS	color_sharp_dm_sr_13tap_hpf_h_coef_1;

	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
	color_sharp_dm_sr_13tap_hpf_h_coef_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_13tap_hpf_h_coef_0_reg);
	color_sharp_dm_sr_13tap_hpf_h_coef_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_13tap_hpf_h_coef_1_reg);

	color_sharp_dm_sr_ctrl.sr_1_tex_en= ptr->SR_H13.en_tex;
	color_sharp_dm_sr_ctrl.sr_1_edg_en= ptr->SR_H13.en_edg;
	color_sharp_dm_sr_13tap_hpf_h_coef_0.c0 = ptr->SR_H13.C0;
	color_sharp_dm_sr_13tap_hpf_h_coef_0.c1 = ptr->SR_H13.C1;
	color_sharp_dm_sr_13tap_hpf_h_coef_0.c2 = ptr->SR_H13.C2;
	color_sharp_dm_sr_13tap_hpf_h_coef_1.c3 = ptr->SR_H13.C3;
	color_sharp_dm_sr_13tap_hpf_h_coef_1.c4 = ptr->SR_H13.C4;
	color_sharp_dm_sr_13tap_hpf_h_coef_1.c5 = ptr->SR_H13.C5;
	color_sharp_dm_sr_13tap_hpf_h_coef_1.c6 = ptr->SR_H13.C6;
	IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_13tap_hpf_h_coef_0_reg, color_sharp_dm_sr_13tap_hpf_h_coef_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_13tap_hpf_h_coef_1_reg, color_sharp_dm_sr_13tap_hpf_h_coef_1.regValue);
}

void drvif_color_Set_Sharpness_PowerSaving_SR_H2(DRV_Sharpness_Table *ptr)
{
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
	color_sharp_dm_sr_21tap_hpf_h_coef_0_RBUS	color_sharp_dm_sr_21tap_hpf_h_coef_0;
	color_sharp_dm_sr_21tap_hpf_h_coef_1_RBUS	color_sharp_dm_sr_21tap_hpf_h_coef_1;
	color_sharp_dm_sr_21tap_hpf_h_coef_2_RBUS	color_sharp_dm_sr_21tap_hpf_h_coef_2;

	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
	color_sharp_dm_sr_21tap_hpf_h_coef_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_21tap_hpf_h_coef_0_reg);
	color_sharp_dm_sr_21tap_hpf_h_coef_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_21tap_hpf_h_coef_1_reg);
	color_sharp_dm_sr_21tap_hpf_h_coef_2.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_21tap_hpf_h_coef_2_reg);

	color_sharp_dm_sr_ctrl.sr_2_tex_en= ptr->SR_H21.en_tex;
	color_sharp_dm_sr_ctrl.sr_2_edg_en= ptr->SR_H21.en_edg;
	color_sharp_dm_sr_21tap_hpf_h_coef_0.c0 = ptr->SR_H21.C0 ;
	color_sharp_dm_sr_21tap_hpf_h_coef_0.c1 = ptr->SR_H21.C1 ;
	color_sharp_dm_sr_21tap_hpf_h_coef_0.c2 = ptr->SR_H21.C2 ;
	color_sharp_dm_sr_21tap_hpf_h_coef_1.c3 = ptr->SR_H21.C3 ;
	color_sharp_dm_sr_21tap_hpf_h_coef_1.c4 = ptr->SR_H21.C4 ;
	color_sharp_dm_sr_21tap_hpf_h_coef_1.c5 = ptr->SR_H21.C5 ;
	color_sharp_dm_sr_21tap_hpf_h_coef_1.c6 = ptr->SR_H21.C6 ;
	color_sharp_dm_sr_21tap_hpf_h_coef_2.c7 = ptr->SR_H21.C7 ;
	color_sharp_dm_sr_21tap_hpf_h_coef_2.c8 = ptr->SR_H21.C8 ;
	color_sharp_dm_sr_21tap_hpf_h_coef_2.c9 = ptr->SR_H21.C9 ;
	color_sharp_dm_sr_21tap_hpf_h_coef_2.c10= ptr->SR_H21.C10;
	IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_21tap_hpf_h_coef_0_reg, color_sharp_dm_sr_21tap_hpf_h_coef_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_21tap_hpf_h_coef_1_reg, color_sharp_dm_sr_21tap_hpf_h_coef_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_21tap_hpf_h_coef_2_reg, color_sharp_dm_sr_21tap_hpf_h_coef_2.regValue);
}

void drvif_color_Set_Sharpness_PowerSaving_SR_H3(DRV_Sharpness_Table *ptr)
{
	color_sharp_dm_sr_ctrl_2_RBUS color_sharp_dm_sr_ctrl_2;
	color_sharp_dm_sr_11tap_hpf_h_coef_0_RBUS	color_sharp_dm_sr_11tap_hpf_h_coef_0;
	color_sharp_dm_sr_11tap_hpf_h_coef_1_RBUS	color_sharp_dm_sr_11tap_hpf_h_coef_1;
	color_sharp_dm_sr_bias_h_coef_RBUS color_sharp_dm_sr_bias_h_coef;

	color_sharp_dm_sr_ctrl_2.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_2_reg);
	color_sharp_dm_sr_11tap_hpf_h_coef_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_11tap_hpf_h_coef_0_reg);
	color_sharp_dm_sr_11tap_hpf_h_coef_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_11tap_hpf_h_coef_1_reg);
	color_sharp_dm_sr_bias_h_coef.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_BIAS_h_coef_reg);

	color_sharp_dm_sr_ctrl_2.sr_3_tex_en = ptr->SR_Ring_H11.en_tex;// k6 have to fixed value
	color_sharp_dm_sr_ctrl_2.sr_3_edg_en = ptr->SR_Ring_H11.en_edg;// k6 have to fixed value

	color_sharp_dm_sr_11tap_hpf_h_coef_0.c0 = ptr->SR_Ring_H11.C0;
	color_sharp_dm_sr_11tap_hpf_h_coef_0.c1 = ptr->SR_Ring_H11.C1;
	color_sharp_dm_sr_11tap_hpf_h_coef_1.c2 = ptr->SR_Ring_H11.C2;
	color_sharp_dm_sr_11tap_hpf_h_coef_1.c3 = ptr->SR_Ring_H11.C3;
	color_sharp_dm_sr_11tap_hpf_h_coef_1.c4 = ptr->SR_Ring_H11.C4;
	color_sharp_dm_sr_11tap_hpf_h_coef_1.c5 = ptr->SR_Ring_H11.C5;
    color_sharp_dm_sr_bias_h_coef.cb1 = ptr->SR_Ring_H11.C0H0;
    color_sharp_dm_sr_bias_h_coef.cb2 = ptr->SR_Ring_H11.C0H1;
    color_sharp_dm_sr_bias_h_coef.cblv1 = ptr->SR_Ring_H11.C0H2;
    color_sharp_dm_sr_bias_h_coef.cblv2 = ptr->SR_Ring_H11.C0H3;
	IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_2_reg, color_sharp_dm_sr_ctrl_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_11tap_hpf_h_coef_0_reg, color_sharp_dm_sr_11tap_hpf_h_coef_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_11tap_hpf_h_coef_1_reg, color_sharp_dm_sr_11tap_hpf_h_coef_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_BIAS_h_coef_reg, color_sharp_dm_sr_bias_h_coef.regValue);
}

void drvif_color_Set_Sharpness_PowerSaving_SR_V0(DRV_Sharpness_Table *ptr)
{
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
	color_sharp_dm_sr_9tap_hpf_v_coef_0_1_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_0_1;

	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
	color_sharp_dm_sr_9tap_hpf_v_coef_0_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_0_1_reg);

	color_sharp_dm_sr_ctrl.sr_v_0_tex_en= ptr->SR_V9_1.en_tex;
	color_sharp_dm_sr_ctrl.sr_v_0_edg_en= ptr->SR_V9_1.en_edg;
	color_sharp_dm_sr_9tap_hpf_v_coef_0_1.c1 = ptr->SR_V9_1.C1;
	color_sharp_dm_sr_9tap_hpf_v_coef_0_1.c2 = ptr->SR_V9_1.C2;
	color_sharp_dm_sr_9tap_hpf_v_coef_0_1.c3 = ptr->SR_V9_1.C3;
	color_sharp_dm_sr_9tap_hpf_v_coef_0_1.c4 = ptr->SR_V9_1.C4;
	IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_0_1_reg, color_sharp_dm_sr_9tap_hpf_v_coef_0_1.regValue);
}

void drvif_color_Set_Sharpness_PowerSaving_SR_V1(DRV_Sharpness_Table *ptr)
{
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
	color_sharp_dm_sr_9tap_hpf_v_coef_1_0_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_1_0;
	color_sharp_dm_sr_9tap_hpf_v_coef_1_1_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_1_1;

	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
	color_sharp_dm_sr_9tap_hpf_v_coef_1_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_1_0_reg);
	color_sharp_dm_sr_9tap_hpf_v_coef_1_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_1_1_reg);

	color_sharp_dm_sr_ctrl.sr_v_1_tex_en= ptr->SR_V9_2.en_tex;
	color_sharp_dm_sr_ctrl.sr_v_1_edg_en= ptr->SR_V9_2.en_edg;
	color_sharp_dm_sr_9tap_hpf_v_coef_1_0.c0 = ptr->SR_V9_2.C0;
	color_sharp_dm_sr_9tap_hpf_v_coef_1_1.c1 = ptr->SR_V9_2.C1;
	color_sharp_dm_sr_9tap_hpf_v_coef_1_1.c2 = ptr->SR_V9_2.C2;
	color_sharp_dm_sr_9tap_hpf_v_coef_1_1.c3 = ptr->SR_V9_2.C3;
	color_sharp_dm_sr_9tap_hpf_v_coef_1_1.c4 = ptr->SR_V9_2.C4;
	IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_1_0_reg, color_sharp_dm_sr_9tap_hpf_v_coef_1_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_1_1_reg, color_sharp_dm_sr_9tap_hpf_v_coef_1_1.regValue);
}

void drvif_color_Set_Sharpness_PowerSaving_SR_V2(DRV_Sharpness_Table *ptr)
{
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
	color_sharp_dm_sr_9tap_hpf_v_coef_2_0_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_2_0;
	color_sharp_dm_sr_9tap_hpf_v_coef_2_1_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_2_1;

	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
	color_sharp_dm_sr_9tap_hpf_v_coef_2_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_2_0_reg);
	color_sharp_dm_sr_9tap_hpf_v_coef_2_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_2_1_reg);

	color_sharp_dm_sr_ctrl.sr_v_2_tex_en= ptr->SR_V9_3.en_tex;
	color_sharp_dm_sr_ctrl.sr_v_2_edg_en= ptr->SR_V9_3.en_edg;
	color_sharp_dm_sr_9tap_hpf_v_coef_2_0.c0 = ptr->SR_V9_3.C0;
	color_sharp_dm_sr_9tap_hpf_v_coef_2_1.c1 = ptr->SR_V9_3.C1;
	color_sharp_dm_sr_9tap_hpf_v_coef_2_1.c2 = ptr->SR_V9_3.C2;
	color_sharp_dm_sr_9tap_hpf_v_coef_2_1.c3 = ptr->SR_V9_3.C3;
	color_sharp_dm_sr_9tap_hpf_v_coef_2_1.c4 = ptr->SR_V9_3.C4;
	IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_2_0_reg, color_sharp_dm_sr_9tap_hpf_v_coef_2_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_2_1_reg, color_sharp_dm_sr_9tap_hpf_v_coef_2_1.regValue);
}

void drvif_color_Set_Sharpness_PowerSaving_SR_V3(DRV_Sharpness_Table *ptr)
{
    color_sharp_dm_sr_ctrl_2_RBUS color_sharp_dm_sr_ctrl_2;
    color_sharp_dm_sr_9tap_hpf_v_coef_3_0_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_3_0;
    color_sharp_dm_sr_9tap_hpf_v_coef_3_1_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_3_1;
	color_sharp_dm_sr_bias_v_coef_RBUS color_sharp_dm_sr_bias_v_coef;

    color_sharp_dm_sr_ctrl_2.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_2_reg);
    color_sharp_dm_sr_9tap_hpf_v_coef_3_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_3_0_reg);
    color_sharp_dm_sr_9tap_hpf_v_coef_3_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_3_1_reg);
    color_sharp_dm_sr_bias_v_coef.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_BIAS_v_coef_reg);

    color_sharp_dm_sr_ctrl_2.sr_v_3_tex_en = ptr->SR_Ring_V9.en_tex;//merlin5 need set fixed value
    color_sharp_dm_sr_ctrl_2.sr_v_3_edg_en = ptr->SR_Ring_V9.en_edg;//merlin5 need set fixed value
    color_sharp_dm_sr_9tap_hpf_v_coef_3_0.c0 = ptr->SR_Ring_V9.C0;
    color_sharp_dm_sr_9tap_hpf_v_coef_3_1.c1 = ptr->SR_Ring_V9.C1;
    color_sharp_dm_sr_9tap_hpf_v_coef_3_1.c2 = ptr->SR_Ring_V9.C2;
    color_sharp_dm_sr_9tap_hpf_v_coef_3_1.c3 = ptr->SR_Ring_V9.C3;
    color_sharp_dm_sr_9tap_hpf_v_coef_3_1.c4 = ptr->SR_Ring_V9.C4;
    color_sharp_dm_sr_bias_v_coef.cb1 = ptr->SR_Ring_V9.C0H0;
    color_sharp_dm_sr_bias_v_coef.cb2 = ptr->SR_Ring_V9.C0H1;
    color_sharp_dm_sr_bias_v_coef.cblv1 = ptr->SR_Ring_V9.C0H2;
    color_sharp_dm_sr_bias_v_coef.cblv2 = ptr->SR_Ring_V9.C0H3;
    IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_2_reg, color_sharp_dm_sr_ctrl_2.regValue);
    IoReg_Write32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_3_0_reg, color_sharp_dm_sr_9tap_hpf_v_coef_3_0.regValue);
    IoReg_Write32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_3_1_reg, color_sharp_dm_sr_9tap_hpf_v_coef_3_1.regValue);
    IoReg_Write32(COLOR_SHARP_DM_SR_BIAS_v_coef_reg, color_sharp_dm_sr_bias_v_coef.regValue);
}

void drvif_color_Set_Sharpness_PowerSaving_VPK(DRV_Sharpness_Table *ptr)
{

	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
	color_sharp_dm_segpk_vpk1_RBUS color_sharp_dm_segpk_vpk1;
	color_sharp_dm_segpk_vpk4_RBUS color_sharp_dm_segpk_vpk4;

	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
	color_sharp_dm_segpk_vpk1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK1_reg);
	color_sharp_dm_segpk_vpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK4_reg);

    //drvif_Wait_UZUDTG_start(10, -200); //sw workaround

	color_sharp_dm_sr_ctrl.vpk_tex_en= ptr->Vertical.en_tex;
	color_sharp_dm_sr_ctrl.vpk_edg_en= ptr->Vertical.en_edg;
	color_sharp_dm_segpk_vpk1.vpk_c0 = ptr->Vertical.C0;
	color_sharp_dm_segpk_vpk1.vpk_c1 = ptr->Vertical.C1;
	color_sharp_dm_segpk_vpk1.vpk_c2 = ptr->Vertical.C2;
	color_sharp_dm_segpk_vpk4.vpk_c3 = ptr->Vertical.C3;
	color_sharp_dm_segpk_vpk4.vpk_c4 = ptr->Vertical.C4;
	IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK1_reg, color_sharp_dm_segpk_vpk1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK4_reg, color_sharp_dm_segpk_vpk4.regValue);
}

void drvif_color_Set_Sharpness_PowerSaving_SR_X(DRV_Sharpness_Table *ptr)
{
	color_sharp_dm_sr_x_coef_0_RBUS	color_sharp_dm_sr_x_coef_0;
	color_sharp_dm_sr_x_coef_1_RBUS	color_sharp_dm_sr_x_coef_1;

	color_sharp_dm_sr_x_coef_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_X_coef_0_reg);
	color_sharp_dm_sr_x_coef_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_X_coef_1_reg);

	color_sharp_dm_sr_x_coef_0.tex_en= ptr->SR_X.en_tex;
	color_sharp_dm_sr_x_coef_0.edg_en= ptr->SR_X.en_edg;
	color_sharp_dm_sr_x_coef_0.c0 = ptr->SR_X.C0;
	color_sharp_dm_sr_x_coef_1.c1 = ptr->SR_X.C1;
	color_sharp_dm_sr_x_coef_1.c2 = ptr->SR_X.C2;
	color_sharp_dm_sr_x_coef_1.c3 = ptr->SR_X.C3;
	color_sharp_dm_sr_x_coef_1.c4 = ptr->SR_X.C4;
	IoReg_Write32(COLOR_SHARP_DM_SR_X_coef_0_reg, color_sharp_dm_sr_x_coef_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_X_coef_1_reg, color_sharp_dm_sr_x_coef_1.regValue);
}

void drvif_color_Set_Sharpness_VPK3_init(void)
{
}

DRV_Sharpness_Table drv_shp_table_for_sub_En;
extern unsigned char TV006_WB_Pattern_En;
char drvif_color_Set_SHP_sub_En(void)
{
	DRV_Sharpness_Table *ptr;
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl_Varible;
    color_sharp_dm_sr_ctrl_2_RBUS color_sharp_dm_sr_ctrl_2;
	char ret = 0;
	unsigned char ori_pip_swap;
	unsigned char shp_data_access_check_flag = 0;
	//scaleup_d_uzu_globle_ctrl_RBUS d_uzu_Globle_Ctrl_reg;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	color_sharp_dm_2d_shp_y_remap_RBUS color_sharp_dm_2d_shp_y_remap_reg;
	color_sharp_access_data_0_RBUS color_sharp_access_data_0_reg;
	color_sharp_access_data_1_RBUS color_sharp_access_data_1_reg;
	color_sharp_access_data_2_RBUS color_sharp_access_data_2_reg;
	color_sharp_access_data_3_RBUS color_sharp_access_data_3_reg;
	int timeoutcnt;
	unsigned int t0 ,t1, t2, t3;
	unsigned long flags;//for spin_lock_irqsave

	if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
		&& (Check_AP_Set_Enable_ForceBG(SLR_MAIN_DISPLAY) == FALSE)) {
		rtd_pr_vpq_info("SHP_sub_En_pip, skip\n");
		ret = -1;
	} else if (Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) {
		rtd_pr_vpq_info("SHP_sub_En_pip, sub is active, skip\n");
		ret = -1;
	} else {
		t0 = drvif_Get_90k_Lo_clk();
		ptr = &drv_shp_table_for_sub_En;
		// set data access check point
		color_sharp_access_data_0_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_0_reg);
		color_sharp_access_data_1_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_1_reg);
		color_sharp_access_data_2_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_2_reg);
		color_sharp_access_data_3_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_3_reg);
		color_sharp_access_data_0_reg.read_en = 1;
		color_sharp_access_data_1_reg.startx = 0x300;
		color_sharp_access_data_1_reg.starty = 0x600;
		IoReg_Write32(COLOR_SHARP_Access_Data_0_reg, color_sharp_access_data_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_Access_Data_1_reg, color_sharp_access_data_1_reg.regValue);

		// set line mode, pip swap, and uzu mute
		color_sharp_dm_2d_shp_y_remap_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_Y_REMAP_reg);
		color_sharp_dm_2d_shp_y_remap_reg.lineary_lmode = 0;
		IoReg_Write32(COLOR_SHARP_DM_2D_SHP_Y_REMAP_reg, color_sharp_dm_2d_shp_y_remap_reg.regValue);

		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
		ori_pip_swap = display_timing_ctrl2_reg.pip_swap_en;
		display_timing_ctrl2_reg.pip_swap_en = 1;
		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

		rtd_pr_vpq_info("SHP_sub_En_pip, pip in bg, ori_pip_swap=%d,\n", ori_pip_swap);
		ret = 0;

		// wait 1ms
		t1 = drvif_Get_90k_Lo_clk();
		usleep_range(2000, 3000);

		timeoutcnt = 0x400;
		while ((color_sharp_access_data_0_reg.read_en == 1) && (timeoutcnt>10) && (shp_data_access_check_flag == 1)) {
			color_sharp_access_data_0_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_0_reg);
			timeoutcnt--;
			usleep_range(100, 200);
		}

		t2 = drvif_Get_90k_Lo_clk();

		// check sharpness data access
		color_sharp_access_data_0_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_0_reg);
		color_sharp_access_data_2_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_2_reg);
		color_sharp_access_data_3_reg.regValue = IoReg_Read32(COLOR_SHARP_Access_Data_3_reg);

		if (((color_sharp_access_data_0_reg.read_en != 0) || (color_sharp_access_data_2_reg.y00 != 0)  || (color_sharp_access_data_2_reg.y01 != 0)
			 || (color_sharp_access_data_3_reg.y10 != 0)  || (color_sharp_access_data_3_reg.y11 != 0) || (timeoutcnt <= 10))
			 && (shp_data_access_check_flag == 1)) {
			// data access check skip
			color_sharp_dm_2d_shp_y_remap_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_Y_REMAP_reg);
			color_sharp_dm_2d_shp_y_remap_reg.lineary_lmode = ptr->MK3_Table.LinearY_LM;
			IoReg_Write32(COLOR_SHARP_DM_2D_SHP_Y_REMAP_reg, color_sharp_dm_2d_shp_y_remap_reg.regValue);

			down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
			spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
			display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
			display_timing_ctrl2_reg.pip_swap_en = ori_pip_swap;
			IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
			spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
			up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

			t3 = drvif_Get_90k_Lo_clk();

			rtd_pr_vpq_info("SHP_sub_En_pip, data access check skip.=%d, t0~3=%x,%x,%x,%x, dataAccess=%x,%x,%x,timeoutcnt=%x,\n",
				ori_pip_swap, t0, t1, t2, t3, color_sharp_access_data_0_reg.regValue, color_sharp_access_data_2_reg.regValue,
				color_sharp_access_data_3_reg.regValue, timeoutcnt);
			ret = -1;

		} else {
			// set shp
			color_sharp_dm_sr_ctrl_Varible.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
			color_sharp_dm_sr_ctrl_2.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_2_reg);
			color_sharp_dm_sr_ctrl_Varible.h_tex_filter_en= ptr->TwoD_tex.en_tex;
			color_sharp_dm_sr_ctrl_Varible.h_edg_filter_en= ptr->TwoD_edg.en_edg;
			color_sharp_dm_sr_ctrl_Varible.sr_0_tex_en= ptr->SR_H9.en_tex;
			color_sharp_dm_sr_ctrl_Varible.sr_0_edg_en= ptr->SR_H9.en_edg;
			color_sharp_dm_sr_ctrl_Varible.sr_1_tex_en= ptr->SR_H13.en_tex;
			color_sharp_dm_sr_ctrl_Varible.sr_1_edg_en= ptr->SR_H13.en_edg;
			color_sharp_dm_sr_ctrl_Varible.sr_2_tex_en= ptr->SR_H21.en_tex;
			color_sharp_dm_sr_ctrl_Varible.sr_2_edg_en= ptr->SR_H21.en_edg;
			color_sharp_dm_sr_ctrl_Varible.sr_v_0_tex_en= ptr->SR_V9_1.en_tex;
			color_sharp_dm_sr_ctrl_Varible.sr_v_0_edg_en= ptr->SR_V9_1.en_edg;
			color_sharp_dm_sr_ctrl_Varible.sr_v_1_tex_en= ptr->SR_V9_2.en_tex;
			color_sharp_dm_sr_ctrl_Varible.sr_v_1_edg_en= ptr->SR_V9_2.en_edg;
			color_sharp_dm_sr_ctrl_Varible.sr_v_2_tex_en= ptr->SR_V9_3.en_tex;
			color_sharp_dm_sr_ctrl_Varible.sr_v_2_edg_en= ptr->SR_V9_3.en_edg;
			color_sharp_dm_sr_ctrl_2.sr_v_3_tex_en= ptr->SR_Ring_V9.en_tex;//merlin5 need set fixed value
			color_sharp_dm_sr_ctrl_2.sr_v_3_edg_en= ptr->SR_Ring_V9.en_edg;//merlin5 need set fixed value
			color_sharp_dm_sr_ctrl_2.sr_3_tex_en= ptr->SR_Ring_H11.en_tex;//merlin5 need set fixed value
			color_sharp_dm_sr_ctrl_2.sr_3_edg_en= ptr->SR_Ring_H11.en_edg;//merlin5 need set fixed value
			color_sharp_dm_sr_ctrl_Varible.vpk_tex_en= ptr->Vertical.en_tex;
			color_sharp_dm_sr_ctrl_Varible.vpk_edg_en= ptr->Vertical.en_edg;
			IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl_Varible.regValue);
			IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_2_reg, color_sharp_dm_sr_ctrl_2.regValue);

			// resort line mode, pip swap, and disable uzu mute
			color_sharp_dm_2d_shp_y_remap_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_Y_REMAP_reg);
			color_sharp_dm_2d_shp_y_remap_reg.lineary_lmode = ptr->MK3_Table.LinearY_LM;
			IoReg_Write32(COLOR_SHARP_DM_2D_SHP_Y_REMAP_reg, color_sharp_dm_2d_shp_y_remap_reg.regValue);

			down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
			spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
			display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
			display_timing_ctrl2_reg.pip_swap_en = ori_pip_swap;
			IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
			spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
			up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

			t3 = drvif_Get_90k_Lo_clk();

			rtd_pr_vpq_info("SHP_sub_En_pip, pip resort in bg, ori_pip_swap=%d, t0~3=%x,%x,%x,%x, dataAccess=%x,%x,%x\n",
				ori_pip_swap, t0, t1, t2, t3, color_sharp_access_data_0_reg.regValue, color_sharp_access_data_2_reg.regValue,
				color_sharp_access_data_3_reg.regValue);
		}
	}
	return ret;
#if 0
	ppoverlay_new_meas2_linecnt_real_RBUS new_meas2_linecnt_real_reg;
	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	color_sharp_dm_2d_shp_y_remap_RBUS color_sharp_dm_2d_shp_y_remap_reg;

	int timeoutcnt = 0x012500;
	short denEnd, denST;
	unsigned int lineCNT;
	//unsigned int t0 ,t1, t2,t3;
	unsigned int temp, value;

	dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	denST = dv_den_start_end_reg.dv_den_sta;
	denEnd = dv_den_start_end_reg.dv_den_end;

	color_sharp_dm_2d_shp_y_remap_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_Y_REMAP_reg);
	color_sharp_dm_2d_shp_y_remap_reg.lineary_lmode = 0;
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_Y_REMAP_reg, color_sharp_dm_2d_shp_y_remap_reg.regValue);

	t0 = drvif_Get_90k_Lo_clk();

	usleep_range(50000, 100000);

	IoReg_Write32(0xb802B9F8, 0x033a0680);	// set shp x,y
	IoReg_SetBits(0xb802B9F4, _BIT0);
	do {
                temp = IoReg_Read32(0xb802B9F4);
				timeoutcnt--;
	} while(((temp&_BIT0) == 1) && (timeoutcnt>10));
	value = IoReg_Read32(0xb802B9Fc);
	temp = IoReg_Read32(0xb802B9F4);
	t1 = drvif_Get_90k_Lo_clk();

	t2 = drvif_Get_90k_Lo_clk();
	timeoutcnt = 0x032500;
	do {
                new_meas2_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
			lineCNT = new_meas2_linecnt_real_reg.uzudtg_dly_line_cnt_rt;
                if((lineCNT < denST ) || (lineCNT >= denEnd)) {
                        break;
                }
	} while(timeoutcnt-- != 0);

	if(timeoutcnt == 0){
	        rtd_pr_vpq_emerg("drvif_color_Set_Sharpness_PowerSaving_Mute, timeout \n");
	}

	t3 = drvif_Get_90k_Lo_clk();


	new_meas2_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);

	rtd_pr_vpq_info("drvif_color_Set_Sharpness_PowerSaving_Mute, line=%d, sta=%d, end=%d,t=%x,%x,%x,%x,value=%x,temp=%x,\n",
		new_meas2_linecnt_real_reg.uzudtg_dly_line_cnt_rt, denST, denEnd, t0, t1, t2, t3, value, temp);
#endif
}
void drvif_color_Set_Sharpness_Table(DRV_Sharpness_Table *ptr)
{
	color_sharp_dm_nr_shp_ctrl_RBUS	color_sharp_dm_nr_shp_ctrl;

	/* no set value yet
	color_sharp_dm_segpk_dbg_RBUS	color_sharp_dm_segpk_dbg;
	color_sharp_dm_peaking_bound_1_RBUS	color_sharp_dm_peaking_bound_1;
	color_sharp_dm_segpk_edgpk4_RBUS	color_sharp_dm_segpk_edgpk4;
	color_sharp_dm_gain_curve_debug_RBUS	color_sharp_dm_gain_curve_debug;
	*/

	if (!ptr)
		return;

	/* no set value yet
	color_sharp_dm_segpk_dbg.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_DBG_reg);
	color_sharp_dm_peaking_bound_1.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_BOUND_1_reg);
	color_sharp_dm_segpk_edgpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK4_reg);
	color_sharp_dm_gain_curve_debug.regValue = IoReg_Read32(COLOR_SHARP_DM_GAIN_CURVE_DEBUG_reg);
	*/

	color_sharp_dm_nr_shp_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_NR_SHP_CTRL_reg);
	color_sharp_dm_nr_shp_ctrl.db_en=1;
	color_sharp_dm_nr_shp_ctrl.db_load=0;
	IoReg_Write32(COLOR_SHARP_DM_NR_SHP_CTRL_reg, color_sharp_dm_nr_shp_ctrl.regValue);

/* peaking enable control*/
	{
	color_d_vc_global_ctrl_RBUS d_vc_global_ctrl_reg;

	d_vc_global_ctrl_reg.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);

	color_sharp_dm_nr_shp_ctrl.peaking_enable = ptr->TwoD_Peak_AdaptCtrl.Peaking_Enable;
	d_vc_global_ctrl_reg.m_sharp_en = ptr->TwoD_Peak_AdaptCtrl.Peaking_Enable;
	d_vc_global_ctrl_reg.write_enable_4 =1;

	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_reg.regValue);
	}
/* What the drvif_color_Set_Sharpness_Table committer needs to copy starts here. */
/* set 2D Adapt Ctrl start, set row 0, 1*/
	{
	color_sharp_dm_segpk_ctrl1_RBUS color_sharp_dm_segpk_ctrl1;
	color_sharp_dm_segpk_ctrl2_RBUS color_sharp_dm_segpk_ctrl2;
	color_sharp_dm_segpk_ctrl3_RBUS color_sharp_dm_segpk_ctrl3;
	color_sharp_dm_segpk_ctrl4_RBUS color_sharp_dm_segpk_ctrl4;
	color_sharp_dm_segpk_edge_mono_RBUS color_sharp_dm_segpk_edge_mono;
	color_sharp_dm_2d_shp_tex_flat_0_RBUS	color_sharp_dm_2d_shp_tex_flat_0;
	color_sharp_dm_vpk_hpk_add_RBUS	color_sharp_dm_vpk_hpk_add;

	color_sharp_dm_segpk_ctrl1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL1_reg);
	color_sharp_dm_segpk_ctrl2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL2_reg);
	color_sharp_dm_segpk_ctrl3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL3_reg);
	color_sharp_dm_segpk_ctrl4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL4_reg);
	color_sharp_dm_segpk_edge_mono.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGE_MONO_reg);
	color_sharp_dm_2d_shp_tex_flat_0.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_FLAT_0_reg);
	color_sharp_dm_vpk_hpk_add.regValue = IoReg_Read32(COLOR_SHARP_DM_VPK_HPK_ADD_reg);

	color_sharp_dm_segpk_ctrl1.edgmode = ptr->TwoD_Peak_AdaptCtrl.Edge_Mode;
	color_sharp_dm_segpk_ctrl1.sobelmode = ptr->TwoD_Peak_AdaptCtrl.Sobel_Mode;
	color_sharp_dm_segpk_ctrl1.edgextrange = ptr->TwoD_Peak_AdaptCtrl.Edgextrange;
	color_sharp_dm_segpk_ctrl1.sobel_upbnd = ptr->TwoD_Peak_AdaptCtrl.Sobel_Upbnd;
	color_sharp_dm_segpk_ctrl1.sobel_stepbit = ptr->TwoD_Peak_AdaptCtrl.Sobel_Stepbit;
	color_sharp_dm_segpk_ctrl2.zdiff_upbnd = ptr->TwoD_Peak_AdaptCtrl.Zdiff_Upbnd;
	color_sharp_dm_segpk_ctrl2.zdiff_stepbit = ptr->TwoD_Peak_AdaptCtrl.Zdiff_Stepbit;
	color_sharp_dm_segpk_ctrl2.zigsftbit_reg = ptr->TwoD_Peak_AdaptCtrl.Zigsftbit_Reg;
	color_sharp_dm_segpk_ctrl3.tex_en = ptr->TwoD_Peak_AdaptCtrl.Tex_En;
	color_sharp_dm_segpk_ctrl3.tex_upbnd = ptr->TwoD_Peak_AdaptCtrl.Tex_Upbnd;
	color_sharp_dm_segpk_ctrl3.tex_stepbit = ptr->TwoD_Peak_AdaptCtrl.Tex_Stepbit;
	// removed color_sharp_dm_segpk_ctrl3.actsftbit_reg = ptr->TwoD_Peak_AdaptCtrl.Actsftbit_Reg;
	color_sharp_dm_segpk_ctrl2.tex_h_range= ptr->TwoD_Peak_AdaptCtrl._5x5_h;/*merlin3*/
	color_sharp_dm_segpk_ctrl2.tex_v_range= ptr->TwoD_Peak_AdaptCtrl._5x5_v;/*merlin3*/
	color_sharp_dm_segpk_ctrl3.tex_exclude_smooth= ptr->TwoD_Peak_AdaptCtrl.Tex_Exclude_Smooth;/*merlin3*/
	color_sharp_dm_2d_shp_tex_flat_0.tex_flat_filter_en = ptr->Texture_Flat_Shp.en_tex;
	color_sharp_dm_segpk_ctrl1.edg_tex_blending_lpf_filter = ptr->TwoD_Peak_AdaptCtrl.Edg_Tex_Blending_Lpf_Filter;
	color_sharp_dm_segpk_ctrl1.edg_tex_blending_lpf_filter_en = ptr->TwoD_Peak_AdaptCtrl.Edg_Tex_Blending_Lpf_En;
	color_sharp_dm_vpk_hpk_add.hpf_vpf_maxmin_en = ptr->TwoD_Peak_AdaptCtrl.Hpf_Vpf_Maxmin_En;/*magellan2*/
	color_sharp_dm_vpk_hpk_add.src_sel = ptr->TwoD_Peak_AdaptCtrl.Src_Sel;/*magellan2*/
	color_sharp_dm_segpk_ctrl2.pkdlti_comsign = ptr->TwoD_Peak_AdaptCtrl.PkDlti_comsign;
	color_sharp_dm_segpk_ctrl2.sobelrange_h= ptr->TwoD_Peak_AdaptCtrl.Sobel_h;/*merlin*/
	color_sharp_dm_segpk_ctrl2.sobelrange_v= ptr->TwoD_Peak_AdaptCtrl.Sobel_v;/*merlin*/
	// removed color_sharp_dm_segpk_ctrl3.acts_offset= ptr->TwoD_Peak_AdaptCtrl.Acts_offset;/*merlin*/
	color_sharp_dm_segpk_ctrl3.small_th1= ptr->TwoD_Peak_AdaptCtrl.Small_thl;/*merlin*/
	color_sharp_dm_segpk_ctrl2.sobel_hspace= ptr->TwoD_Peak_AdaptCtrl.Sobel_hspace;/*merlin3*/
	color_sharp_dm_segpk_ctrl2.sobel_vspace= ptr->TwoD_Peak_AdaptCtrl.Sobel_vspace;/*merlin3*/
	color_sharp_dm_segpk_ctrl4.edg_dlti = ptr->TwoD_Peak_AdaptCtrl.edg_dlti;
	color_sharp_dm_segpk_ctrl4.edg_dlti_tex = ptr->TwoD_Peak_AdaptCtrl.edg_dlti_tex;
	color_sharp_dm_segpk_ctrl4.zdiffl_en = ptr->TwoD_Peak_AdaptCtrl.zdiffl_en;
	color_sharp_dm_segpk_ctrl4.zdiffl_lowbnd = ptr->TwoD_Peak_AdaptCtrl.zdiffl_lowbnd;
	color_sharp_dm_segpk_ctrl4.zdiffl_stepbit = ptr->TwoD_Peak_AdaptCtrl.zdiffl_stepbit;
	color_sharp_dm_segpk_edge_mono.edge_mono_en = ptr->TwoD_Peak_AdaptCtrl.edge_mono_en;
	color_sharp_dm_segpk_edge_mono.edge_mono_stepbit = ptr->TwoD_Peak_AdaptCtrl.edge_mono_stepbit;
	color_sharp_dm_segpk_edge_mono.edge_mono_stepbiu = ptr->TwoD_Peak_AdaptCtrl.edge_mono_stepbiu;
	color_sharp_dm_segpk_edge_mono.edge_mono_upbnd = ptr->TwoD_Peak_AdaptCtrl.edge_mono_upbnd;
	color_sharp_dm_segpk_edge_mono.edge_mono_lowbnd = ptr->TwoD_Peak_AdaptCtrl.edge_mono_lowbnd;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_CTRL1_reg, color_sharp_dm_segpk_ctrl1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_CTRL2_reg, color_sharp_dm_segpk_ctrl2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_CTRL3_reg, color_sharp_dm_segpk_ctrl3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_CTRL4_reg, color_sharp_dm_segpk_ctrl4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGE_MONO_reg, color_sharp_dm_segpk_edge_mono.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_FLAT_0_reg, color_sharp_dm_2d_shp_tex_flat_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_VPK_HPK_ADD_reg, color_sharp_dm_vpk_hpk_add.regValue);
	}
/* set 2D Adapt Ctrl end */

/* set Edge/tex peaking start, set row 2 */
	{
	color_sharp_dm_segpk_edgpk2_RBUS color_sharp_dm_segpk_edgpk2;
	color_sharp_dm_segpk_edgpk3_RBUS color_sharp_dm_segpk_edgpk3;
	color_sharp_dm_peaking_bound_0_RBUS color_sharp_dm_peaking_bound_0;
	color_sharp_dm_peaking_gain_RBUS color_sharp_dm_peaking_gain;

	color_sharp_dm_segpk_edgpk2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK2_reg);
	color_sharp_dm_segpk_edgpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK3_reg);
	color_sharp_dm_peaking_bound_0.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_BOUND_0_reg);
	color_sharp_dm_peaking_gain.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_GAIN_reg);

	color_sharp_dm_segpk_edgpk2.lv2 = ptr->Edge_Shp.LV2;
	color_sharp_dm_segpk_edgpk3.gain_pos2 = ptr->Edge_Shp.G_Pos2;
	color_sharp_dm_segpk_edgpk3.gain_neg2 = ptr->Edge_Shp.G_Neg2;
	color_sharp_dm_segpk_edgpk3.edg_d2_shift_bit = ptr->Edge_Shp.Edg_D2_Shift_Bit; /* sirius add*/

	color_sharp_dm_peaking_bound_0.lv2 = ptr->Texture_Shp.LV2;
	color_sharp_dm_peaking_gain.gain_pos2 = ptr->Texture_Shp.G_Pos2;
	color_sharp_dm_peaking_gain.gain_neg2 = ptr->Texture_Shp.G_Neg2;
	color_sharp_dm_peaking_gain.tex_d2_shift_bit = ptr->Texture_Shp.Edg_D2_Shift_Bit;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK2_reg, color_sharp_dm_segpk_edgpk2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK3_reg, color_sharp_dm_segpk_edgpk3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_PEAKING_BOUND_0_reg, color_sharp_dm_peaking_bound_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_PEAKING_GAIN_reg, color_sharp_dm_peaking_gain.regValue);
	}
/* set Edge Peaking End */

/* set Vertical Coef apply start, set row 4 */
	{

	color_sharp_dm_segpk_vpk1_RBUS color_sharp_dm_segpk_vpk1;
	color_sharp_dm_segpk_vpk2_RBUS	color_sharp_dm_segpk_vpk2;
	color_sharp_dm_segpk_vpk3_RBUS	color_sharp_dm_segpk_vpk3;
	color_sharp_dm_segpk_vpk4_RBUS color_sharp_dm_segpk_vpk4;
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;

	color_sharp_dm_segpk_vpk1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK1_reg);
	color_sharp_dm_segpk_vpk2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK2_reg);
	color_sharp_dm_segpk_vpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK3_reg);
	color_sharp_dm_segpk_vpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK4_reg);
	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);

	/*DC Value Check*/
	if (SHARPNESS_LOG) {
		if ((ptr->Vertical.C0 + 2*(ptr->Vertical.C1 + ptr->Vertical.C2 + ptr->Vertical.C3 + ptr->Vertical.C4) != 0))
			VIPprintf("[Warnning] [%s-%s-%d] [Vertical Coef]DC Not Balance!\n", __FILE__, __func__, __LINE__);

	}

	//drvif_color_Set_Sharpness_PowerSaving_VPK(ptr);// no used
	color_sharp_dm_sr_ctrl.vpk_tex_en= ptr->Vertical.vpk_sr_tex;
	color_sharp_dm_sr_ctrl.vpk_edg_en= ptr->Vertical.vpk_sr_edg;
	color_sharp_dm_segpk_vpk1.vpk_c0 = ptr->Vertical.C0;
	color_sharp_dm_segpk_vpk1.vpk_c1 = ptr->Vertical.C1;
	color_sharp_dm_segpk_vpk1.vpk_c2 = ptr->Vertical.C2;
	color_sharp_dm_segpk_vpk4.vpk_c3 = ptr->Vertical.C3;
	color_sharp_dm_segpk_vpk4.vpk_c4 = ptr->Vertical.C4;

	color_sharp_dm_segpk_vpk4.vext_reg = ptr->Vertical.Vext_Reg;
	color_sharp_dm_segpk_vpk4.vemf_en = ptr->Vertical.Vemf_En;
	color_sharp_dm_segpk_vpk2.v_tex_en = ptr->Vertical.V_Tex_en;
	color_sharp_dm_segpk_vpk3.gain_neg2 = ptr->Vertical.G_Neg2;
	color_sharp_dm_segpk_vpk3.gain_pos2 = ptr->Vertical.G_Pos2;
	color_sharp_dm_segpk_vpk3.lv2 = ptr->Vertical.LV2;


	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK1_reg, color_sharp_dm_segpk_vpk1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK2_reg, color_sharp_dm_segpk_vpk2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK3_reg, color_sharp_dm_segpk_vpk3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK4_reg, color_sharp_dm_segpk_vpk4.regValue);
	//IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl.regValue); // fixed value

	// removed color_sharp_dm_segpk_vpk4.vpk_edg_en= ptr->Vertical.enable_edge;/*merlin*/

	}
/* set Vertical Coef apply end */

/* set Emf Parameter start, set row5 */
	{
	color_sharp_dm_peaking_emf_texture_RBUS	color_sharp_dm_peaking_emf_texture;
	color_sharp_dm_emfmk2_ctrl_texture_RBUS	color_sharp_dm_emfmk2_ctrl_texture;
	color_sharp_dm_emf_ext_0_texture_RBUS	color_sharp_dm_emf_ext_0_texture;
	color_sharp_dm_emf_ext_1_texture_RBUS	color_sharp_dm_emf_ext_1_texture;
	color_sharp_dm_emf_ext_2_texture_RBUS   color_sharp_dm_emf_ext_2_texture;
	color_sharp_dm_peaking_emf_RBUS	color_sharp_dm_peaking_emf;
	color_sharp_dm_emfmk2_ctrl_RBUS	color_sharp_dm_emfmk2_ctrl;
	color_sharp_dm_emf_ext_0_RBUS	color_sharp_dm_emf_ext_0;
	color_sharp_dm_emf_ext_1_RBUS	color_sharp_dm_emf_ext_1;

	color_sharp_dm_peaking_emf_texture.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_EMF_TEXTURE_reg);
	color_sharp_dm_emfmk2_ctrl_texture.regValue = IoReg_Read32(COLOR_SHARP_DM_EMFMK2_CTRL_TEXTURE_reg);
	color_sharp_dm_emf_ext_0_texture.regValue = IoReg_Read32(COLOR_SHARP_DM_EMF_EXT_0_TEXTURE_reg);
	color_sharp_dm_emf_ext_1_texture.regValue = IoReg_Read32(COLOR_SHARP_DM_EMF_EXT_1_TEXTURE_reg);
	color_sharp_dm_emf_ext_2_texture.regValue = IoReg_Read32(COLOR_SHARP_DM_EMF_EXT_2_TEXTURE_reg);
	color_sharp_dm_peaking_emf.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_EMF_reg);
	color_sharp_dm_emfmk2_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_EMFMK2_CTRL_reg);
	color_sharp_dm_emf_ext_0.regValue = IoReg_Read32(COLOR_SHARP_DM_EMF_EXT_0_reg);
	color_sharp_dm_emf_ext_1.regValue = IoReg_Read32(COLOR_SHARP_DM_EMF_EXT_1_reg);

	/*color_sharp_dm_nr_shp_ctrl.emf_enable = ptr->EMF_Table.EMF_En;*/
	color_sharp_dm_nr_shp_ctrl.edg_emf_enable = ptr->Edge_EMF_Table.EMF_En;
	color_sharp_dm_nr_shp_ctrl.tex_emf_enable = ptr->Text_EMF_Table.EMF_En;

	color_sharp_dm_emfmk2_ctrl.emf_shift = ptr->Edge_EMF_Table.EMF_Shift;
	color_sharp_dm_peaking_emf.seg0_gain_sel = ptr->Edge_EMF_Table.Seg_0_Gain;
	color_sharp_dm_peaking_emf.seg1_gain_sel = ptr->Edge_EMF_Table.Seg_1_Gain;
	color_sharp_dm_peaking_emf.seg2_gain_sel = ptr->Edge_EMF_Table.Seg_2_Gain;
	color_sharp_dm_peaking_emf.seg0_offset = ptr->Edge_EMF_Table.Seg_0_Offset;
	color_sharp_dm_emf_ext_0.emf_fixextent_neg = ptr->Edge_EMF_Table.emf_fixextent_neg;
	color_sharp_dm_emf_ext_0.emf_fixextent_pos = ptr->Edge_EMF_Table.emf_fixextent_pos;
	color_sharp_dm_emf_ext_0.seg1_offset = ptr->Edge_EMF_Table.Seg_1_Offset;
	color_sharp_dm_emf_ext_1.seg2_offset = ptr->Edge_EMF_Table.Seg_2_Offset;
	color_sharp_dm_emf_ext_1.seg0_x = ptr->Edge_EMF_Table.Seg_0_x;
	color_sharp_dm_emf_ext_1.seg1_x = ptr->Edge_EMF_Table.Seg_1_x;
	color_sharp_dm_emfmk2_ctrl.blend_wt = ptr->Edge_EMF_Table.Blend_wt;
	color_sharp_dm_peaking_emf.emf_range_v= ptr->Edge_EMF_Table.v5;
	color_sharp_dm_emfmk2_ctrl.emf_range_h= ptr->Edge_EMF_Table.h11;
	color_sharp_dm_emf_ext_2_texture.emf_tex_dff = ptr->Text_EMF_Table.emf_tex_dff;
	color_sharp_dm_emf_ext_2_texture.emf_tex_dff_ub = ptr->Text_EMF_Table.emf_tex_dff_ub; 

	color_sharp_dm_emfmk2_ctrl_texture.emf_shift = ptr->Text_EMF_Table.EMF_Shift;
	color_sharp_dm_peaking_emf_texture.seg0_gain_sel = ptr->Text_EMF_Table.Seg_0_Gain;
	color_sharp_dm_peaking_emf_texture.seg1_gain_sel = ptr->Text_EMF_Table.Seg_1_Gain;
	color_sharp_dm_peaking_emf_texture.seg2_gain_sel = ptr->Text_EMF_Table.Seg_2_Gain;
	color_sharp_dm_peaking_emf_texture.seg0_offset = ptr->Text_EMF_Table.Seg_0_Offset;
	color_sharp_dm_emf_ext_0_texture.emf_fixextent_neg = ptr->Text_EMF_Table.emf_fixextent_neg;
	color_sharp_dm_emf_ext_0_texture.emf_fixextent_pos = ptr->Text_EMF_Table.emf_fixextent_pos;
	color_sharp_dm_emf_ext_0_texture.tex_share_v_enable = ptr->Text_EMF_Table.tex_share_v_enable;
	color_sharp_dm_emf_ext_0_texture.tex_share_v_blend_wt = ptr->Text_EMF_Table.tex_share_v_blend_wt;
	color_sharp_dm_emf_ext_0_texture.seg1_offset = ptr->Text_EMF_Table.Seg_1_Offset;
	color_sharp_dm_emf_ext_1_texture.seg2_offset = ptr->Text_EMF_Table.Seg_2_Offset;
	color_sharp_dm_emf_ext_1_texture.seg0_x = ptr->Text_EMF_Table.Seg_0_x;
	color_sharp_dm_emf_ext_1_texture.seg1_x = ptr->Text_EMF_Table.Seg_1_x;
	color_sharp_dm_emfmk2_ctrl_texture.blend_wt = ptr->Text_EMF_Table.Blend_wt;
	color_sharp_dm_peaking_emf_texture.texgain= ptr->Text_EMF_Table.EMF_texgain;
	color_sharp_dm_emfmk2_ctrl_texture.emf_range_v= ptr->Text_EMF_Table.v5;
	color_sharp_dm_emfmk2_ctrl_texture.emf_range_h= ptr->Text_EMF_Table.h11;

	IoReg_Write32(COLOR_SHARP_DM_PEAKING_EMF_TEXTURE_reg, color_sharp_dm_peaking_emf_texture.regValue);
	IoReg_Write32(COLOR_SHARP_DM_EMFMK2_CTRL_TEXTURE_reg, color_sharp_dm_emfmk2_ctrl_texture.regValue);
	IoReg_Write32(COLOR_SHARP_DM_EMF_EXT_0_TEXTURE_reg, color_sharp_dm_emf_ext_0_texture.regValue);
	IoReg_Write32(COLOR_SHARP_DM_EMF_EXT_1_TEXTURE_reg, color_sharp_dm_emf_ext_1_texture.regValue);
	IoReg_Write32(COLOR_SHARP_DM_PEAKING_EMF_reg, color_sharp_dm_peaking_emf.regValue);
	IoReg_Write32(COLOR_SHARP_DM_EMFMK2_CTRL_reg, color_sharp_dm_emfmk2_ctrl.regValue);
	IoReg_Write32(COLOR_SHARP_DM_EMF_EXT_0_reg, color_sharp_dm_emf_ext_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_EMF_EXT_1_reg, color_sharp_dm_emf_ext_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_EMF_EXT_2_TEXTURE_reg, color_sharp_dm_emf_ext_2_texture.regValue);
	}
/* set Emf Parameter End */

/* set MK3 start */
	{
	color_sharp_dm_mkiii_shp_ctrl_RBUS	color_sharp_dm_mkiii_shp_ctrl;
	color_sharp_dm_2d_shp_y_remap_RBUS	color_sharp_dm_2d_shp_y_remap;

	color_sharp_dm_segpk_edge_coring_0_RBUS color_sharp_dm_segpk_edge_coring_0;
	color_sharp_dm_segpk_edge_coring_1_RBUS color_sharp_dm_segpk_edge_coring_1;
	color_sharp_dm_peaking_h_lvd_RBUS color_sharp_dm_peaking_h_lvd;
	color_sharp_dm_peaking_v_lvd_RBUS color_sharp_dm_peaking_v_lvd;
	color_sharp_dm_segpk_edgpk_sourceslope_RBUS color_sharp_dm_segpk_edgpk_sourceslope;
	color_sharp_dm_segpk_edgpk_sourceslope_flat_RBUS	color_sharp_dm_segpk_edgpk_sourceslope_flat;
	color_sharp_dm_segpk_sourceslope_extend_RBUS color_sharp_dm_segpk_sourceslope_extend;

	color_sharp_dm_sours_lpf_0_RBUS color_sharp_dm_sours_lpf_0;
	color_sharp_dm_sours_lpf_1_RBUS color_sharp_dm_sours_lpf_1;
	color_sharp_dm_maxmin_shp_gain_0_RBUS color_sharp_dm_maxmin_shp_gain_0;
	color_sharp_dm_maxmin_shp_gain_1_RBUS color_sharp_dm_maxmin_shp_gain_1;
	color_sharp_dm_maxmin_shp_gain_2_RBUS color_sharp_dm_maxmin_shp_gain_2;
	color_sharp_dm_maxmin_shp_gain_3_RBUS color_sharp_dm_maxmin_shp_gain_3;
	color_sharp_dm_maxmin_shp_step_0_RBUS color_sharp_dm_maxmin_shp_step_0;
	color_sharp_dm_maxmin_shp_step_1_RBUS color_sharp_dm_maxmin_shp_step_1;
	color_sharp_dm_maxmin_shp_step_2_RBUS color_sharp_dm_maxmin_shp_step_2;
	color_sharp_dm_maxmin_shp_step_3_RBUS color_sharp_dm_maxmin_shp_step_3;

	color_sharp_dm_segpk_vpk5_RBUS	color_sharp_dm_segpk_vpk5;
	color_sharp_dm_maxmin_lpf_weit_0_RBUS color_sharp_dm_maxmin_lpf_weit_0;
	color_sharp_dm_maxmin_lpf_weit_1_RBUS color_sharp_dm_maxmin_lpf_weit_1;
	color_sharp_dm_maxmin_lpf_weit_2_RBUS color_sharp_dm_maxmin_lpf_weit_2;
	color_sharp_dm_maxmin_lpf_weit_3_RBUS color_sharp_dm_maxmin_lpf_weit_3;
	color_sharp_dm_maxmin_lpf_step_0_RBUS color_sharp_dm_maxmin_lpf_step_0;
	color_sharp_dm_maxmin_lpf_step_1_RBUS color_sharp_dm_maxmin_lpf_step_1;
	color_sharp_dm_maxmin_lpf_step_2_RBUS color_sharp_dm_maxmin_lpf_step_2;
	color_sharp_dm_maxmin_lpf_step_3_RBUS color_sharp_dm_maxmin_lpf_step_3;
	color_sharp_dm_maxmin_gain_by_y_gain_0_RBUS color_sharp_dm_maxmin_gain_by_y_gain_0;
	color_sharp_dm_maxmin_gain_by_y_gain_1_RBUS color_sharp_dm_maxmin_gain_by_y_gain_1;
	color_sharp_dm_maxmin_gain_by_y_gain_2_RBUS color_sharp_dm_maxmin_gain_by_y_gain_2;
	color_sharp_dm_maxmin_gain_by_y_gain_3_RBUS color_sharp_dm_maxmin_gain_by_y_gain_3;
	color_sharp_dm_maxmin_gain_by_y_step_0_RBUS color_sharp_dm_maxmin_gain_by_y_step_0;
	color_sharp_dm_maxmin_gain_by_y_step_1_RBUS color_sharp_dm_maxmin_gain_by_y_step_1;
	color_sharp_dm_maxmin_gain_by_y_step_2_RBUS color_sharp_dm_maxmin_gain_by_y_step_2;
	color_sharp_dm_maxmin_gain_by_y_step_3_RBUS color_sharp_dm_maxmin_gain_by_y_step_3;

	color_sharp_dm_2d_shp_edge_0_RBUS	color_sharp_dm_2d_shp_edge_0;
	color_sharp_dm_2d_shp_edge_1_RBUS	color_sharp_dm_2d_shp_edge_1;
	color_sharp_dm_2d_shp_edge_1_1_RBUS color_sharp_dm_2d_shp_edge_1_1;
	color_sharp_dm_2d_shp_edge_2_RBUS	color_sharp_dm_2d_shp_edge_2;
	color_sharp_dm_2d_shp_edge_4_RBUS	color_sharp_dm_2d_shp_edge_4;
	color_sharp_dm_2d_shp_edge_6_RBUS	color_sharp_dm_2d_shp_edge_6;
	color_sharp_dm_2d_shp_edge_7_RBUS	color_sharp_dm_2d_shp_edge_7;

	color_sharp_dm_2d_shp_tex_0_RBUS	color_sharp_dm_2d_shp_tex_0;
	color_sharp_dm_2d_shp_tex_1_RBUS	color_sharp_dm_2d_shp_tex_1;
	color_sharp_dm_2d_shp_tex_1_1_RBUS	color_sharp_dm_2d_shp_tex_1_1;
	color_sharp_dm_2d_shp_tex_2_RBUS	color_sharp_dm_2d_shp_tex_2;
	color_sharp_dm_2d_shp_tex_4_RBUS	color_sharp_dm_2d_shp_tex_4;
	color_sharp_dm_2d_shp_tex_6_RBUS	color_sharp_dm_2d_shp_tex_6;
	color_sharp_dm_2d_shp_tex_7_RBUS	color_sharp_dm_2d_shp_tex_7;

	color_sharp_dm_segpk_edgpk2_RBUS color_sharp_dm_segpk_edgpk2;
	color_sharp_dm_peaking_bound_0_RBUS color_sharp_dm_peaking_bound_0;
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
	color_sharp_dm_segpk_vpk3_RBUS	color_sharp_dm_segpk_vpk3;

	color_sharp_dm_mkiii_shp_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_MKIII_SHP_CTRL_reg);
	color_sharp_dm_2d_shp_y_remap.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_Y_REMAP_reg);

	color_sharp_dm_segpk_edge_coring_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGE_CORING_0_reg);
	color_sharp_dm_segpk_edge_coring_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGE_CORING_1_reg);
	color_sharp_dm_peaking_h_lvd.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_H_LVD_reg);
	color_sharp_dm_peaking_v_lvd.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_V_LVD_reg);
	color_sharp_dm_segpk_edgpk_sourceslope.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK_SOURCESLOPE_reg);
	color_sharp_dm_segpk_edgpk_sourceslope_flat.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK_SOURCESLOPE_FLAT_reg);
	color_sharp_dm_segpk_sourceslope_extend.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_SOURCESLOPE_EXTEND_reg);

	color_sharp_dm_sours_lpf_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SOURS_LPF_0_reg);
	color_sharp_dm_sours_lpf_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SOURS_LPF_1_reg);
	color_sharp_dm_maxmin_shp_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_GAIN_0_reg);
	color_sharp_dm_maxmin_shp_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_GAIN_1_reg);
	color_sharp_dm_maxmin_shp_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_GAIN_2_reg);
	color_sharp_dm_maxmin_shp_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_GAIN_3_reg);
	color_sharp_dm_maxmin_shp_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_STEP_0_reg);
	color_sharp_dm_maxmin_shp_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_STEP_1_reg);
	color_sharp_dm_maxmin_shp_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_STEP_2_reg);
	color_sharp_dm_maxmin_shp_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_STEP_3_reg);

	color_sharp_dm_segpk_vpk5.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK5_reg);
	color_sharp_dm_maxmin_lpf_weit_0.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_0_reg);
	color_sharp_dm_maxmin_lpf_weit_1.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_1_reg);
	color_sharp_dm_maxmin_lpf_weit_2.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_2_reg);
	color_sharp_dm_maxmin_lpf_weit_3.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_3_reg);
	color_sharp_dm_maxmin_lpf_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_STEP_0_reg);
	color_sharp_dm_maxmin_lpf_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_STEP_1_reg);
	color_sharp_dm_maxmin_lpf_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_STEP_2_reg);
	color_sharp_dm_maxmin_lpf_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_STEP_3_reg);
	color_sharp_dm_maxmin_gain_by_y_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_0_reg);
	color_sharp_dm_maxmin_gain_by_y_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_1_reg);
	color_sharp_dm_maxmin_gain_by_y_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_2_reg);
	color_sharp_dm_maxmin_gain_by_y_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_3_reg);
	color_sharp_dm_maxmin_gain_by_y_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_STEP_0_reg);
	color_sharp_dm_maxmin_gain_by_y_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_STEP_1_reg);
	color_sharp_dm_maxmin_gain_by_y_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_STEP_2_reg);
	color_sharp_dm_maxmin_gain_by_y_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_STEP_3_reg);

	color_sharp_dm_2d_shp_edge_0.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_0_reg);
	color_sharp_dm_2d_shp_edge_1.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_1_reg);
	color_sharp_dm_2d_shp_edge_1_1.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_1_1_reg);
	color_sharp_dm_2d_shp_edge_2.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_2_reg);
	color_sharp_dm_2d_shp_edge_4.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_4_reg);
	color_sharp_dm_2d_shp_edge_6.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_6_reg);
	color_sharp_dm_2d_shp_edge_7.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_7_reg);

	color_sharp_dm_2d_shp_tex_0.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_0_reg);
	color_sharp_dm_2d_shp_tex_1.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_1_reg);
	color_sharp_dm_2d_shp_tex_1_1.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_1_1_reg);
	color_sharp_dm_2d_shp_tex_2.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_2_reg);
	color_sharp_dm_2d_shp_tex_4.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_4_reg);
	color_sharp_dm_2d_shp_tex_6.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_6_reg);
	color_sharp_dm_2d_shp_tex_7.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_7_reg);

	color_sharp_dm_segpk_edgpk2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK2_reg);
	color_sharp_dm_peaking_bound_0.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_BOUND_0_reg);
	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
	color_sharp_dm_segpk_vpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK3_reg);

	/*====== mkiii en , filter select =====*/
	/*color_sharp_dm_nr_shp_ctrl.mkiii_en = ptr->MK3_Table.Mkiii_En;*/
	/*====== row023 Mark_III LPF ======*/
	/*DC Value Check*/
	if (SHARPNESS_LOG) {
		if (ptr->MK3_Table.Lpf2d_C0 + 4*(ptr->MK3_Table.Lpf2d_C1+ptr->MK3_Table.Lpf2d_C2+ptr->MK3_Table.Lpf2d_C3+ptr->MK3_Table.Lpf2d_C5)+8*(ptr->MK3_Table.Lpf2d_C4) != 64)
			VIPprintf("[Warnning] [%s-%s-%d] [MKIII LPF ]DC Not Balance!\n", __FILE__, __func__, __LINE__);

	}

	color_sharp_dm_sours_lpf_0.coef_c0 = ptr->MK3_Table.Lpf2d_C0;
	color_sharp_dm_sours_lpf_0.coef_c1 = ptr->MK3_Table.Lpf2d_C1;
	color_sharp_dm_sours_lpf_0.coef_c2 = ptr->MK3_Table.Lpf2d_C2;
	color_sharp_dm_sours_lpf_0.coef_c3 = ptr->MK3_Table.Lpf2d_C3;
	color_sharp_dm_sours_lpf_1.coef_c4 = ptr->MK3_Table.Lpf2d_C4;
	color_sharp_dm_sours_lpf_1.coef_c5 = ptr->MK3_Table.Lpf2d_C5;
	/*====== row024 Gain by max-min curve ======*/
	color_sharp_dm_maxmin_shp_gain_0.s0 = ptr->MK3_Table.MaxMin_Gain_0;
	color_sharp_dm_maxmin_shp_gain_0.s1 = ptr->MK3_Table.MaxMin_Gain_1;
	color_sharp_dm_maxmin_shp_gain_1.s2 = ptr->MK3_Table.MaxMin_Gain_2;
	color_sharp_dm_maxmin_shp_gain_1.s3 = ptr->MK3_Table.MaxMin_Gain_3;
	color_sharp_dm_maxmin_shp_gain_2.s4 = ptr->MK3_Table.MaxMin_Gain_4;
	color_sharp_dm_maxmin_shp_gain_2.s5 = ptr->MK3_Table.MaxMin_Gain_5;
	color_sharp_dm_maxmin_shp_gain_3.s6 = ptr->MK3_Table.MaxMin_Gain_6;
	color_sharp_dm_maxmin_shp_step_0.s1 = ptr->MK3_Table.MaxMin_Step_1;
	color_sharp_dm_maxmin_shp_step_1.s2 = ptr->MK3_Table.MaxMin_Step_2;
	color_sharp_dm_maxmin_shp_step_1.s3 = ptr->MK3_Table.MaxMin_Step_3;
	color_sharp_dm_maxmin_shp_step_2.s4 = ptr->MK3_Table.MaxMin_Step_4;
	color_sharp_dm_maxmin_shp_step_2.s5 = ptr->MK3_Table.MaxMin_Step_5;
	color_sharp_dm_maxmin_shp_step_3.s6 = ptr->MK3_Table.MaxMin_Step_6;
	color_sharp_dm_mkiii_shp_ctrl.max_min_gain_en = ptr->MK3_Table.MaxMin_Gain_En;
	color_sharp_dm_mkiii_shp_ctrl.max_min_gain_tex_en = ptr->MK3_Table.max_min_gain_tex_en;
	color_sharp_dm_mkiii_shp_ctrl.max_min_gain_edg_en = ptr->MK3_Table.max_min_gain_edg_en;
	color_sharp_dm_mkiii_shp_ctrl.max_min_gain_v_en = ptr->MK3_Table.max_min_gain_v_en;
	/*====== row025 LPF Source blending by max-min ======*/
	color_sharp_dm_maxmin_lpf_weit_0.s0 = ptr->MK3_Table.Lpf_Weit_0;
	color_sharp_dm_maxmin_lpf_weit_0.s1 = ptr->MK3_Table.Lpf_Weit_1;
	color_sharp_dm_maxmin_lpf_weit_1.s2 = ptr->MK3_Table.Lpf_Weit_2;
	color_sharp_dm_maxmin_lpf_weit_1.s3 = ptr->MK3_Table.Lpf_Weit_3;
	color_sharp_dm_maxmin_lpf_weit_2.s4 = ptr->MK3_Table.Lpf_Weit_4;
	color_sharp_dm_maxmin_lpf_weit_2.s5 = ptr->MK3_Table.Lpf_Weit_5;
	color_sharp_dm_maxmin_lpf_weit_3.s6 = ptr->MK3_Table.Lpf_Weit_6;
	color_sharp_dm_maxmin_lpf_step_0.s1 = ptr->MK3_Table.Lpf_Step_1;
	color_sharp_dm_maxmin_lpf_step_1.s2 = ptr->MK3_Table.Lpf_Step_2;
	color_sharp_dm_maxmin_lpf_step_1.s3 = ptr->MK3_Table.Lpf_Step_3;
	color_sharp_dm_maxmin_lpf_step_2.s4 = ptr->MK3_Table.Lpf_Step_4;
	color_sharp_dm_maxmin_lpf_step_2.s5 = ptr->MK3_Table.Lpf_Step_5;
	color_sharp_dm_maxmin_lpf_step_3.s6 = ptr->MK3_Table.Lpf_Step_6;
	color_sharp_dm_mkiii_shp_ctrl.lpf_weigh_en = ptr->MK3_Table.Lpf_Weigh_En;
	color_sharp_dm_mkiii_shp_ctrl.lpf_weigh_tex_en = ptr->MK3_Table.lpf_weigh_tex_en;
	color_sharp_dm_mkiii_shp_ctrl.lpf_weigh_edg_en = ptr->MK3_Table.lpf_weigh_edg_en;
	/*====== row026 Gain by Y curve ======*/
	color_sharp_dm_maxmin_gain_by_y_gain_0.s0 = ptr->MK3_Table.Gain_By_Y_Gain_0;
	color_sharp_dm_maxmin_gain_by_y_gain_0.s1 = ptr->MK3_Table.Gain_By_Y_Gain_1;
	color_sharp_dm_maxmin_gain_by_y_gain_1.s2 = ptr->MK3_Table.Gain_By_Y_Gain_2;
	color_sharp_dm_maxmin_gain_by_y_gain_1.s3 = ptr->MK3_Table.Gain_By_Y_Gain_3;
	color_sharp_dm_maxmin_gain_by_y_gain_2.s4 = ptr->MK3_Table.Gain_By_Y_Gain_4;
	color_sharp_dm_maxmin_gain_by_y_gain_2.s5 = ptr->MK3_Table.Gain_By_Y_Gain_5;
	color_sharp_dm_maxmin_gain_by_y_gain_3.s6 = ptr->MK3_Table.Gain_By_Y_Gain_6;
	color_sharp_dm_maxmin_gain_by_y_step_0.s1 = ptr->MK3_Table.Gain_By_Y_Step_1;
	color_sharp_dm_maxmin_gain_by_y_step_1.s2 = ptr->MK3_Table.Gain_By_Y_Step_2;
	color_sharp_dm_maxmin_gain_by_y_step_1.s3 = ptr->MK3_Table.Gain_By_Y_Step_3;
	color_sharp_dm_maxmin_gain_by_y_step_2.s4 = ptr->MK3_Table.Gain_By_Y_Step_4;
	color_sharp_dm_maxmin_gain_by_y_step_2.s5 = ptr->MK3_Table.Gain_By_Y_Step_5;
	color_sharp_dm_maxmin_gain_by_y_step_3.s6 = ptr->MK3_Table.Gain_By_Y_Step_6;
	color_sharp_dm_mkiii_shp_ctrl.gain_by_y_en = ptr->MK3_Table.Gain_By_Y_En;
	color_sharp_dm_mkiii_shp_ctrl.gain_by_y_tex_en = ptr->MK3_Table.gain_by_y_tex_en;
	color_sharp_dm_mkiii_shp_ctrl.gain_by_y_edg_en = ptr->MK3_Table.gain_by_y_edg_en;
	color_sharp_dm_mkiii_shp_ctrl.gain_by_y_v_en = ptr->MK3_Table.gain_by_y_v_en;
	color_sharp_dm_segpk_edgpk2.lv_by_y_enable = ptr->MK3_Table.lv_by_y_enable;
	color_sharp_dm_peaking_bound_0.lv_by_y_tex_enable = ptr->MK3_Table.lv_by_y_tex_enable;
	color_sharp_dm_segpk_edgpk2.lv_by_y_edg_enable = ptr->MK3_Table.lv_by_y_edg_enable;
	color_sharp_dm_segpk_vpk3.lv_by_y_v_enable = ptr->MK3_Table.lv_by_y_v_enable;
	color_sharp_dm_segpk_vpk5.lv_by_y_v_edg_enable = ptr->MK3_Table.lv_by_y_v_edg_enable;
	/*===== edge coef ====*/
	//drvif_color_Set_Sharpness_PowerSaving_2D_edg(ptr);
	color_sharp_dm_sr_ctrl.h_edg_filter_en= ptr->TwoD_edg.en_edg;
	color_sharp_dm_2d_shp_edge_0.coef_c0_0 = ptr->TwoD_edg.C0;
	color_sharp_dm_2d_shp_edge_0.coef_c1_0 = ptr->TwoD_edg.C1;
	color_sharp_dm_2d_shp_edge_0.coef_c2_0 = ptr->TwoD_edg.C2;
	color_sharp_dm_2d_shp_edge_1.coef_c3_0 = ptr->TwoD_edg.C3;
	color_sharp_dm_2d_shp_edge_1.coef_c4_0 = ptr->TwoD_edg.C4;
	color_sharp_dm_2d_shp_edge_1.coef_c5_0 = ptr->TwoD_edg.C5;
	color_sharp_dm_2d_shp_edge_1.coef_c6_0 = ptr->TwoD_edg.C6;
	color_sharp_dm_2d_shp_edge_1_1.coef_c7_0 = ptr->TwoD_edg.C7;
	color_sharp_dm_2d_shp_edge_1_1.coef_c8_0 = ptr->TwoD_edg.C8;
	color_sharp_dm_2d_shp_edge_1_1.coef_c9_0 = ptr->TwoD_edg.C9;
	color_sharp_dm_2d_shp_edge_1_1.coef_c10_0 = ptr->TwoD_edg.C10;
	// Merlin3, 2D filters shrink to 1D1D
	color_sharp_dm_2d_shp_edge_2.coef_c0_1 = ptr->TwoD_edg.C0H1;
	color_sharp_dm_2d_shp_edge_4.coef_c0_2 = ptr->TwoD_edg.C0H2;
	color_sharp_dm_2d_shp_edge_6.coef_c0_3 = ptr->TwoD_edg.C0H3;
	color_sharp_dm_2d_shp_edge_7.coef_c0_4 = ptr->TwoD_edg.C0H4;

	/*===== text coef ====*/
	//drvif_color_Set_Sharpness_PowerSaving_2D_tex(ptr);
	/*DC Value Check*/
	if (SHARPNESS_LOG) {

		int sum;
		sum = 0;
		sum = ptr->TwoD_tex.C0 +
		      2*(ptr->TwoD_tex.C1+ptr->TwoD_tex.C2+ptr->TwoD_tex.C3+ptr->TwoD_tex.C4+ptr->TwoD_tex.C5) +
		      2*(ptr->TwoD_tex.C6+ptr->TwoD_tex.C7+ptr->TwoD_tex.C8+ptr->TwoD_tex.C9+ptr->TwoD_tex.C10) +
		      2*(ptr->TwoD_tex.C0H1+ptr->TwoD_tex.C0H2+ptr->TwoD_tex.C0H3+ptr->TwoD_tex.C0H4);
		if (sum != 0 || sum !=4<<(ptr->Texture_Shp.Edg_D2_Shift_Bit))
			VIPprintf("[Warnning] [%s-%s-%d] [Text Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);

	}

	color_sharp_dm_sr_ctrl.h_tex_filter_en= ptr->TwoD_tex.en_tex;
	color_sharp_dm_2d_shp_tex_0.coef_c0_0 = ptr->TwoD_tex.C0;
	color_sharp_dm_2d_shp_tex_0.coef_c1_0 = ptr->TwoD_tex.C1;
	color_sharp_dm_2d_shp_tex_0.coef_c2_0 = ptr->TwoD_tex.C2;
	color_sharp_dm_2d_shp_tex_1.coef_c3_0 = ptr->TwoD_tex.C3;
	color_sharp_dm_2d_shp_tex_1.coef_c4_0 = ptr->TwoD_tex.C4;
	color_sharp_dm_2d_shp_tex_1.coef_c5_0 = ptr->TwoD_tex.C5;
	color_sharp_dm_2d_shp_tex_1.coef_c6_0 = ptr->TwoD_tex.C6;
	color_sharp_dm_2d_shp_tex_1_1.coef_c7_0 = ptr->TwoD_tex.C7;
	color_sharp_dm_2d_shp_tex_1_1.coef_c8_0 = ptr->TwoD_tex.C8;
	color_sharp_dm_2d_shp_tex_1_1.coef_c9_0 = ptr->TwoD_tex.C9;
	color_sharp_dm_2d_shp_tex_1_1.coef_c10_0 = ptr->TwoD_tex.C10;
	// Merlin3, 2D filters shrink to 1D1D
	color_sharp_dm_2d_shp_tex_2.coef_c0_1 = ptr->TwoD_tex.C0H1;
	color_sharp_dm_2d_shp_tex_4.coef_c0_2 = ptr->TwoD_tex.C0H2;
	color_sharp_dm_2d_shp_tex_6.coef_c0_3 = ptr->TwoD_tex.C0H3;
	color_sharp_dm_2d_shp_tex_7.coef_c0_4 = ptr->TwoD_tex.C0H4;


	/*====== Edge Coring ======*/
	color_sharp_dm_segpk_edge_coring_1.edge_coring_en = ptr->MK3_Table.EdgeCoring_En;
	color_sharp_dm_segpk_edge_coring_1.edge_gain_en = ptr->MK3_Table.EdgeCoring_Gain_En;
	color_sharp_dm_segpk_edge_coring_1.deltaedge_ulevel = ptr->MK3_Table.EdgeCoring_DeltaEdge_Ulevel;
	color_sharp_dm_segpk_edge_coring_1.edge_coring_clip = ptr->MK3_Table.EdgeCoring_Clip;
	color_sharp_dm_segpk_edge_coring_1.edge_gain_bound = ptr->MK3_Table.EdgeCoring_Gain_Bound;
	color_sharp_dm_segpk_edge_coring_1.edge_ser_range = ptr->MK3_Table.EdgeCoring_Ser_Range;
	color_sharp_dm_segpk_edge_coring_0.bigedge_ratio = ptr->MK3_Table.EdgeCoring_Bigedge_Ratio;
	color_sharp_dm_segpk_edge_coring_0.smalledge_ratio = ptr->MK3_Table.EdgeCoring_Smalledge_Ratio;
	color_sharp_dm_segpk_edge_coring_0.deltaedge_ratio = ptr->MK3_Table.EdgeCoring_Deltaedge_Ratio;
	color_sharp_dm_segpk_edge_coring_0.smalledge_ulevel = ptr->MK3_Table.EdgeCoring_Smalledge_Ulevel;
	color_sharp_dm_segpk_edge_coring_1.edge_delta_ext_range =  ptr->MK3_Table.Edge_Delta_Ext_Range;/*mangellan2*/
	color_sharp_dm_peaking_h_lvd.range = ptr->MK3_Table.h_lvd_range;
	color_sharp_dm_peaking_h_lvd.gaim = ptr->MK3_Table.h_lvd_gaim;
	color_sharp_dm_peaking_h_lvd.gain = ptr->MK3_Table.h_lvd_gain;
	color_sharp_dm_peaking_v_lvd.range = ptr->MK3_Table.v_lvd_range;
	color_sharp_dm_peaking_v_lvd.gaim = ptr->MK3_Table.v_lvd_gaim;
	color_sharp_dm_peaking_v_lvd.gain = ptr->MK3_Table.v_lvd_gain;

	color_sharp_dm_2d_shp_y_remap.lineary_lb = ptr->MK3_Table.LinearY_LB;
	color_sharp_dm_2d_shp_y_remap.lineary_ub = ptr->MK3_Table.LinearY_UB;
	color_sharp_dm_2d_shp_y_remap.lineary_lmode = ptr->MK3_Table.LinearY_LM;
	color_sharp_dm_2d_shp_y_remap.lineary_umode = ptr->MK3_Table.LinearY_UM;

	color_sharp_dm_segpk_edgpk_sourceslope.dir = ptr->MK3_Table.dir;
	color_sharp_dm_segpk_sourceslope_extend.flat_method = ptr->MK3_Table.flat_method;
	color_sharp_dm_segpk_edgpk_sourceslope.gain_boundneg = ptr->MK3_Table.Gain_boundNeg;
	color_sharp_dm_segpk_edgpk_sourceslope.gain_boundpos = ptr->MK3_Table.Gain_boundPos;
	color_sharp_dm_segpk_edgpk_sourceslope.gain_extendneg = ptr->MK3_Table.Gain_extendNeg;
	color_sharp_dm_segpk_edgpk_sourceslope.gain_extendpos = ptr->MK3_Table.Gain_extendPos;
	color_sharp_dm_segpk_edgpk_sourceslope.slope_gain_en = ptr->MK3_Table.Slope_gain_en;
	color_sharp_dm_segpk_edgpk_sourceslope_flat.flat_enable = ptr->MK3_Table.flat_enable;
	color_sharp_dm_segpk_edgpk_sourceslope_flat.flat_offset = ptr->MK3_Table.flat_offset;
	color_sharp_dm_segpk_edgpk_sourceslope_flat.flat_gain = ptr->MK3_Table.flat_gain;
	color_sharp_dm_segpk_edgpk_sourceslope_flat.flat_v_enable = ptr->MK3_Table.flat_v_enable;
	color_sharp_dm_segpk_edgpk_sourceslope_flat.flat_offset_v = ptr->MK3_Table.flat_offset_v;
	color_sharp_dm_segpk_edgpk_sourceslope_flat.flat_gain_v = ptr->MK3_Table.flat_gain_v;
	color_sharp_dm_segpk_sourceslope_extend.lowbound= ptr->MK3_Table.lowbound;
	color_sharp_dm_segpk_sourceslope_extend.flat_range= ptr->MK3_Table.flat_range_h;

	IoReg_Write32(COLOR_SHARP_DM_MKIII_SHP_CTRL_reg, color_sharp_dm_mkiii_shp_ctrl.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_Y_REMAP_reg, color_sharp_dm_2d_shp_y_remap.regValue);

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGE_CORING_0_reg, color_sharp_dm_segpk_edge_coring_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGE_CORING_1_reg, color_sharp_dm_segpk_edge_coring_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_PEAKING_H_LVD_reg, color_sharp_dm_peaking_h_lvd.regValue);
	IoReg_Write32(COLOR_SHARP_DM_PEAKING_V_LVD_reg, color_sharp_dm_peaking_v_lvd.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK_SOURCESLOPE_reg, color_sharp_dm_segpk_edgpk_sourceslope.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK_SOURCESLOPE_FLAT_reg, color_sharp_dm_segpk_edgpk_sourceslope_flat.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_SOURCESLOPE_EXTEND_reg, color_sharp_dm_segpk_sourceslope_extend.regValue);

	IoReg_Write32(COLOR_SHARP_DM_SOURS_LPF_0_reg, color_sharp_dm_sours_lpf_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SOURS_LPF_1_reg, color_sharp_dm_sours_lpf_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_SHP_GAIN_0_reg, color_sharp_dm_maxmin_shp_gain_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_SHP_GAIN_1_reg, color_sharp_dm_maxmin_shp_gain_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_SHP_GAIN_2_reg, color_sharp_dm_maxmin_shp_gain_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_SHP_GAIN_3_reg, color_sharp_dm_maxmin_shp_gain_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_SHP_STEP_0_reg, color_sharp_dm_maxmin_shp_step_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_SHP_STEP_1_reg, color_sharp_dm_maxmin_shp_step_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_SHP_STEP_2_reg, color_sharp_dm_maxmin_shp_step_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_SHP_STEP_3_reg, color_sharp_dm_maxmin_shp_step_3.regValue);

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK5_reg, color_sharp_dm_segpk_vpk5.regValue);

	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_0_reg, color_sharp_dm_maxmin_lpf_weit_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_1_reg, color_sharp_dm_maxmin_lpf_weit_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_2_reg, color_sharp_dm_maxmin_lpf_weit_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_3_reg, color_sharp_dm_maxmin_lpf_weit_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_LPF_STEP_0_reg, color_sharp_dm_maxmin_lpf_step_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_LPF_STEP_1_reg, color_sharp_dm_maxmin_lpf_step_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_LPF_STEP_2_reg, color_sharp_dm_maxmin_lpf_step_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_LPF_STEP_3_reg, color_sharp_dm_maxmin_lpf_step_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_0_reg, color_sharp_dm_maxmin_gain_by_y_gain_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_1_reg, color_sharp_dm_maxmin_gain_by_y_gain_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_2_reg, color_sharp_dm_maxmin_gain_by_y_gain_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_3_reg, color_sharp_dm_maxmin_gain_by_y_gain_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_STEP_0_reg, color_sharp_dm_maxmin_gain_by_y_step_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_STEP_1_reg, color_sharp_dm_maxmin_gain_by_y_step_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_STEP_2_reg, color_sharp_dm_maxmin_gain_by_y_step_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_STEP_3_reg, color_sharp_dm_maxmin_gain_by_y_step_3.regValue);

	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_0_reg, color_sharp_dm_2d_shp_edge_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_1_reg, color_sharp_dm_2d_shp_edge_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_1_1_reg, color_sharp_dm_2d_shp_edge_1_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_2_reg, color_sharp_dm_2d_shp_edge_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_4_reg, color_sharp_dm_2d_shp_edge_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_6_reg, color_sharp_dm_2d_shp_edge_6.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_EDGE_7_reg, color_sharp_dm_2d_shp_edge_7.regValue);

	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_0_reg, color_sharp_dm_2d_shp_tex_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_1_reg, color_sharp_dm_2d_shp_tex_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_1_1_reg, color_sharp_dm_2d_shp_tex_1_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_2_reg, color_sharp_dm_2d_shp_tex_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_4_reg, color_sharp_dm_2d_shp_tex_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_6_reg, color_sharp_dm_2d_shp_tex_6.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_7_reg, color_sharp_dm_2d_shp_tex_7.regValue);

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK2_reg, color_sharp_dm_segpk_edgpk2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_PEAKING_BOUND_0_reg, color_sharp_dm_peaking_bound_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK3_reg, color_sharp_dm_segpk_vpk3.regValue);
	//IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl.regValue); // fixed value
	}
/* set MK3 end */

/* set After filter start */ /*magellan2*/
	{
	color_sharp_dm_vc_after_filter_0_RBUS	color_sharp_dm_vc_after_filter_0;
	color_sharp_dm_vc_after_filter_1_RBUS	color_sharp_dm_vc_after_filter_1;

	color_sharp_dm_vc_after_filter_0.regValue = IoReg_Read32(COLOR_SHARP_DM_VC_AFTER_FILTER_0_reg);
	color_sharp_dm_vc_after_filter_1.regValue = IoReg_Read32(COLOR_SHARP_DM_VC_AFTER_FILTER_1_reg);

	/*DC Value Check*/
	if (SHARPNESS_LOG) {

		int sum;
		sum = 0;
		sum = ptr->After_Filter.C0 +
		      2*(ptr->After_Filter.C1 + ptr->After_Filter.C2 + ptr->After_Filter.C3 + ptr->After_Filter.C4);
		if (!(sum == 0 || sum == 256))
			VIPprintf("[Warnning] [%s-%s-%d] [After Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);
	}

	color_sharp_dm_vc_after_filter_0.coef_c0 = ptr->After_Filter.C0;
	color_sharp_dm_vc_after_filter_0.coef_c1 = ptr->After_Filter.C1;
	color_sharp_dm_vc_after_filter_0.coef_c2 = ptr->After_Filter.C2;
	color_sharp_dm_vc_after_filter_0.coef_c3 = ptr->After_Filter.C3;
	color_sharp_dm_vc_after_filter_1.coef_c4 = ptr->After_Filter.C4;
	color_sharp_dm_vc_after_filter_1.enable = ptr->After_Filter.Enable;
	color_sharp_dm_vc_after_filter_1.hpf_gain = ptr->After_Filter.HPF_Gain;
	// #if K7_not_K8
	// color_sharp_dm_vc_after_filter_1.mode = ptr->After_Filter.Mode;
	// #endif
	color_sharp_dm_vc_after_filter_1.lv = ptr->After_Filter.lv;
	color_sharp_dm_vc_after_filter_1.ub = ptr->After_Filter.ub;

	IoReg_Write32(COLOR_SHARP_DM_VC_AFTER_FILTER_0_reg, color_sharp_dm_vc_after_filter_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_VC_AFTER_FILTER_1_reg, color_sharp_dm_vc_after_filter_1.regValue);
	}
/* set After filter end */

/* set V Gain Curve start */ /*magellan2*/
	{
	color_sharp_dm_vertical_pos_gain_curve_gain_0_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_0;
	color_sharp_dm_vertical_pos_gain_curve_gain_1_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_1;
	color_sharp_dm_vertical_pos_gain_curve_gain_2_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_2;
	color_sharp_dm_vertical_pos_gain_curve_gain_3_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_3;
	color_sharp_dm_vertical_pos_gain_curve_gain_4_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_4;
	color_sharp_dm_vertical_pos_gain_curve_gain_5_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_5;
	color_sharp_dm_vertical_pos_gain_curve_gain_6_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_6;
	color_sharp_dm_vertical_pos_gain_curve_gain_7_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_7;
	color_sharp_dm_vertical_pos_gain_curve_step_0_RBUS	color_sharp_dm_vertical_pos_gain_curve_step_0;
	color_sharp_dm_vertical_pos_gain_curve_step_1_RBUS	color_sharp_dm_vertical_pos_gain_curve_step_1;
	color_sharp_dm_vertical_pos_gain_curve_step_2_RBUS	color_sharp_dm_vertical_pos_gain_curve_step_2;
	color_sharp_dm_vertical_pos_gain_curve_step_3_RBUS	color_sharp_dm_vertical_pos_gain_curve_step_3;
	color_sharp_dm_vertical_pos_gain_curve_step_4_RBUS	color_sharp_dm_vertical_pos_gain_curve_step_4;
	color_sharp_dm_vertical_neg_gain_curve_gain_0_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_0;
	color_sharp_dm_vertical_neg_gain_curve_gain_1_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_1;
	color_sharp_dm_vertical_neg_gain_curve_gain_2_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_2;
	color_sharp_dm_vertical_neg_gain_curve_gain_3_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_3;
	color_sharp_dm_vertical_neg_gain_curve_gain_4_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_4;
	color_sharp_dm_vertical_neg_gain_curve_gain_5_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_5;
	color_sharp_dm_vertical_neg_gain_curve_gain_6_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_6;
	color_sharp_dm_vertical_neg_gain_curve_gain_7_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_7;
	color_sharp_dm_vertical_neg_gain_curve_step_0_RBUS	color_sharp_dm_vertical_neg_gain_curve_step_0;
	color_sharp_dm_vertical_neg_gain_curve_step_1_RBUS	color_sharp_dm_vertical_neg_gain_curve_step_1;
	color_sharp_dm_vertical_neg_gain_curve_step_2_RBUS	color_sharp_dm_vertical_neg_gain_curve_step_2;
	color_sharp_dm_vertical_neg_gain_curve_step_3_RBUS	color_sharp_dm_vertical_neg_gain_curve_step_3;
	color_sharp_dm_vertical_neg_gain_curve_step_4_RBUS	color_sharp_dm_vertical_neg_gain_curve_step_4;

	color_sharp_dm_vertical_pos_gain_curve_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_0_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_1_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_2_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_3_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_4.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_4_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_5.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_5_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_6.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_6_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_7.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_7_reg);
	color_sharp_dm_vertical_pos_gain_curve_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_0_reg);
	color_sharp_dm_vertical_pos_gain_curve_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_1_reg);
	color_sharp_dm_vertical_pos_gain_curve_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_2_reg);
	color_sharp_dm_vertical_pos_gain_curve_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_3_reg);
	color_sharp_dm_vertical_pos_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_4_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_0_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_1_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_2_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_3_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_4.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_4_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_5.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_5_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_6.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_6_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_7.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_7_reg);
	color_sharp_dm_vertical_neg_gain_curve_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_0_reg);
	color_sharp_dm_vertical_neg_gain_curve_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_1_reg);
	color_sharp_dm_vertical_neg_gain_curve_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_2_reg);
	color_sharp_dm_vertical_neg_gain_curve_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_3_reg);
	color_sharp_dm_vertical_neg_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_4_reg);

	color_sharp_dm_vertical_pos_gain_curve_gain_0.s0 = ptr->V_Pos_Curve.G0;
	color_sharp_dm_vertical_pos_gain_curve_gain_0.s1 = ptr->V_Pos_Curve.G1;
	color_sharp_dm_vertical_pos_gain_curve_gain_1.s2 = ptr->V_Pos_Curve.G2;
	color_sharp_dm_vertical_pos_gain_curve_gain_1.s3 = ptr->V_Pos_Curve.G3;
	color_sharp_dm_vertical_pos_gain_curve_gain_2.s4 = ptr->V_Pos_Curve.G4;
	color_sharp_dm_vertical_pos_gain_curve_gain_2.s5 = ptr->V_Pos_Curve.G5;
	color_sharp_dm_vertical_pos_gain_curve_gain_3.s6 = ptr->V_Pos_Curve.G6;
	color_sharp_dm_vertical_pos_gain_curve_gain_3.s7 = ptr->V_Pos_Curve.G7;
	color_sharp_dm_vertical_pos_gain_curve_gain_4.s8 = ptr->V_Pos_Curve.G8;
	color_sharp_dm_vertical_pos_gain_curve_gain_4.s9 = ptr->V_Pos_Curve.G9;
	color_sharp_dm_vertical_pos_gain_curve_gain_5.s10 = ptr->V_Pos_Curve.G10;
	color_sharp_dm_vertical_pos_gain_curve_gain_5.s11 = ptr->V_Pos_Curve.G11;
	color_sharp_dm_vertical_pos_gain_curve_gain_6.s12 = ptr->V_Pos_Curve.G12;
	color_sharp_dm_vertical_pos_gain_curve_gain_6.s13 = ptr->V_Pos_Curve.G13;
	color_sharp_dm_vertical_pos_gain_curve_gain_7.s14 = ptr->V_Pos_Curve.G14;

	color_sharp_dm_vertical_pos_gain_curve_step_0.s1 = ptr->V_Pos_Curve.S1;
	color_sharp_dm_vertical_pos_gain_curve_step_0.s2 = ptr->V_Pos_Curve.S2;
	color_sharp_dm_vertical_pos_gain_curve_step_0.s3 = ptr->V_Pos_Curve.S3;
	color_sharp_dm_vertical_pos_gain_curve_step_1.s4 = ptr->V_Pos_Curve.S4;
	color_sharp_dm_vertical_pos_gain_curve_step_1.s5 = ptr->V_Pos_Curve.S5;
	color_sharp_dm_vertical_pos_gain_curve_step_1.s6 = ptr->V_Pos_Curve.S6;
	color_sharp_dm_vertical_pos_gain_curve_step_2.s7 = ptr->V_Pos_Curve.S7;
	color_sharp_dm_vertical_pos_gain_curve_step_2.s8 = ptr->V_Pos_Curve.S8;
	color_sharp_dm_vertical_pos_gain_curve_step_2.s9 = ptr->V_Pos_Curve.S9;
	color_sharp_dm_vertical_pos_gain_curve_step_3.s10 = ptr->V_Pos_Curve.S10;
	color_sharp_dm_vertical_pos_gain_curve_step_3.s11 = ptr->V_Pos_Curve.S11;
	color_sharp_dm_vertical_pos_gain_curve_step_3.s12 = ptr->V_Pos_Curve.S12;
	color_sharp_dm_vertical_pos_gain_curve_step_4.s13 = ptr->V_Pos_Curve.S13;
	color_sharp_dm_vertical_pos_gain_curve_step_4.s14 = ptr->V_Pos_Curve.S14;
	color_sharp_dm_vertical_neg_gain_curve_step_4.edg_curve_bypass = ptr->V_Pos_Curve.edg_curve_bypass;

	color_sharp_dm_vertical_neg_gain_curve_gain_0.s0 = ptr->V_Neg_Curve.G0;
	color_sharp_dm_vertical_neg_gain_curve_gain_0.s1 = ptr->V_Neg_Curve.G1;
	color_sharp_dm_vertical_neg_gain_curve_gain_1.s2 = ptr->V_Neg_Curve.G2;
	color_sharp_dm_vertical_neg_gain_curve_gain_1.s3 = ptr->V_Neg_Curve.G3;
	color_sharp_dm_vertical_neg_gain_curve_gain_2.s4 = ptr->V_Neg_Curve.G4;
	color_sharp_dm_vertical_neg_gain_curve_gain_2.s5 = ptr->V_Neg_Curve.G5;
	color_sharp_dm_vertical_neg_gain_curve_gain_3.s6 = ptr->V_Neg_Curve.G6;
	color_sharp_dm_vertical_neg_gain_curve_gain_3.s7 = ptr->V_Neg_Curve.G7;
	color_sharp_dm_vertical_neg_gain_curve_gain_4.s8 = ptr->V_Neg_Curve.G8;
	color_sharp_dm_vertical_neg_gain_curve_gain_4.s9 = ptr->V_Neg_Curve.G9;
	color_sharp_dm_vertical_neg_gain_curve_gain_5.s10 = ptr->V_Neg_Curve.G10;
	color_sharp_dm_vertical_neg_gain_curve_gain_5.s11 = ptr->V_Neg_Curve.G11;
	color_sharp_dm_vertical_neg_gain_curve_gain_6.s12 = ptr->V_Neg_Curve.G12;
	color_sharp_dm_vertical_neg_gain_curve_gain_6.s13 = ptr->V_Neg_Curve.G13;
	color_sharp_dm_vertical_neg_gain_curve_gain_7.s14 = ptr->V_Neg_Curve.G14;

	color_sharp_dm_vertical_neg_gain_curve_step_0.s1 = ptr->V_Neg_Curve.S1;
	color_sharp_dm_vertical_neg_gain_curve_step_0.s2 = ptr->V_Neg_Curve.S2;
	color_sharp_dm_vertical_neg_gain_curve_step_0.s3 = ptr->V_Neg_Curve.S3;
	color_sharp_dm_vertical_neg_gain_curve_step_1.s4 = ptr->V_Neg_Curve.S4;
	color_sharp_dm_vertical_neg_gain_curve_step_1.s5 = ptr->V_Neg_Curve.S5;
	color_sharp_dm_vertical_neg_gain_curve_step_1.s6 = ptr->V_Neg_Curve.S6;
	color_sharp_dm_vertical_neg_gain_curve_step_2.s7 = ptr->V_Neg_Curve.S7;
	color_sharp_dm_vertical_neg_gain_curve_step_2.s8 = ptr->V_Neg_Curve.S8;
	color_sharp_dm_vertical_neg_gain_curve_step_2.s9 = ptr->V_Neg_Curve.S9;
	color_sharp_dm_vertical_neg_gain_curve_step_3.s10 = ptr->V_Neg_Curve.S10;
	color_sharp_dm_vertical_neg_gain_curve_step_3.s11 = ptr->V_Neg_Curve.S11;
	color_sharp_dm_vertical_neg_gain_curve_step_3.s12 = ptr->V_Neg_Curve.S12;
	color_sharp_dm_vertical_neg_gain_curve_step_4.s13 = ptr->V_Neg_Curve.S13;
	color_sharp_dm_vertical_neg_gain_curve_step_4.s14 = ptr->V_Neg_Curve.S14;

	color_sharp_dm_vertical_neg_gain_curve_step_4.enable = ptr->V_Neg_Curve.enable;

	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_0_reg, color_sharp_dm_vertical_pos_gain_curve_gain_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_1_reg, color_sharp_dm_vertical_pos_gain_curve_gain_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_2_reg, color_sharp_dm_vertical_pos_gain_curve_gain_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_3_reg, color_sharp_dm_vertical_pos_gain_curve_gain_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_4_reg, color_sharp_dm_vertical_pos_gain_curve_gain_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_5_reg, color_sharp_dm_vertical_pos_gain_curve_gain_5.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_6_reg, color_sharp_dm_vertical_pos_gain_curve_gain_6.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_7_reg, color_sharp_dm_vertical_pos_gain_curve_gain_7.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_0_reg, color_sharp_dm_vertical_pos_gain_curve_step_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_1_reg, color_sharp_dm_vertical_pos_gain_curve_step_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_2_reg, color_sharp_dm_vertical_pos_gain_curve_step_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_3_reg, color_sharp_dm_vertical_pos_gain_curve_step_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_4_reg, color_sharp_dm_vertical_pos_gain_curve_step_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_0_reg, color_sharp_dm_vertical_neg_gain_curve_gain_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_1_reg, color_sharp_dm_vertical_neg_gain_curve_gain_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_2_reg, color_sharp_dm_vertical_neg_gain_curve_gain_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_3_reg, color_sharp_dm_vertical_neg_gain_curve_gain_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_4_reg, color_sharp_dm_vertical_neg_gain_curve_gain_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_5_reg, color_sharp_dm_vertical_neg_gain_curve_gain_5.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_6_reg, color_sharp_dm_vertical_neg_gain_curve_gain_6.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_7_reg, color_sharp_dm_vertical_neg_gain_curve_gain_7.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_0_reg, color_sharp_dm_vertical_neg_gain_curve_step_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_1_reg, color_sharp_dm_vertical_neg_gain_curve_step_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_2_reg, color_sharp_dm_vertical_neg_gain_curve_step_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_3_reg, color_sharp_dm_vertical_neg_gain_curve_step_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_4_reg, color_sharp_dm_vertical_neg_gain_curve_step_4.regValue);
	}
/* set V Gain Curve end */

/* set Edge Gain Curve start */ /*magellan2*/
	{
	color_sharp_dm_edge_pos_gain_curve_gain_0_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_0;
	color_sharp_dm_edge_pos_gain_curve_gain_1_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_1;
	color_sharp_dm_edge_pos_gain_curve_gain_2_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_2;
	color_sharp_dm_edge_pos_gain_curve_gain_3_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_3;
	color_sharp_dm_edge_pos_gain_curve_gain_4_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_4;
	color_sharp_dm_edge_pos_gain_curve_gain_5_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_5;
	color_sharp_dm_edge_pos_gain_curve_gain_6_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_6;
	color_sharp_dm_edge_pos_gain_curve_gain_7_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_7;
	color_sharp_dm_edge_pos_gain_curve_step_0_RBUS	color_sharp_dm_edge_pos_gain_curve_step_0;
	color_sharp_dm_edge_pos_gain_curve_step_1_RBUS	color_sharp_dm_edge_pos_gain_curve_step_1;
	color_sharp_dm_edge_pos_gain_curve_step_2_RBUS	color_sharp_dm_edge_pos_gain_curve_step_2;
	color_sharp_dm_edge_pos_gain_curve_step_3_RBUS	color_sharp_dm_edge_pos_gain_curve_step_3;
	color_sharp_dm_edge_pos_gain_curve_step_4_RBUS	color_sharp_dm_edge_pos_gain_curve_step_4;
	color_sharp_dm_edge_neg_gain_curve_gain_0_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_0;
	color_sharp_dm_edge_neg_gain_curve_gain_1_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_1;
	color_sharp_dm_edge_neg_gain_curve_gain_2_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_2;
	color_sharp_dm_edge_neg_gain_curve_gain_3_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_3;
	color_sharp_dm_edge_neg_gain_curve_gain_4_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_4;
	color_sharp_dm_edge_neg_gain_curve_gain_5_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_5;
	color_sharp_dm_edge_neg_gain_curve_gain_6_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_6;
	color_sharp_dm_edge_neg_gain_curve_gain_7_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_7;
	color_sharp_dm_edge_neg_gain_curve_step_0_RBUS	color_sharp_dm_edge_neg_gain_curve_step_0;
	color_sharp_dm_edge_neg_gain_curve_step_1_RBUS	color_sharp_dm_edge_neg_gain_curve_step_1;
	color_sharp_dm_edge_neg_gain_curve_step_2_RBUS	color_sharp_dm_edge_neg_gain_curve_step_2;
	color_sharp_dm_edge_neg_gain_curve_step_3_RBUS	color_sharp_dm_edge_neg_gain_curve_step_3;
	color_sharp_dm_edge_neg_gain_curve_step_4_RBUS	color_sharp_dm_edge_neg_gain_curve_step_4;

	color_sharp_dm_edge_pos_gain_curve_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_0_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_1_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_2_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_3_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_4.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_4_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_5.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_5_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_6.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_6_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_7.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_7_reg);
	color_sharp_dm_edge_pos_gain_curve_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_step_0_reg);
	color_sharp_dm_edge_pos_gain_curve_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_step_1_reg);
	color_sharp_dm_edge_pos_gain_curve_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_step_2_reg);
	color_sharp_dm_edge_pos_gain_curve_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_step_3_reg);
	color_sharp_dm_edge_pos_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_step_4_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_0_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_1_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_2_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_3_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_4.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_4_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_5.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_5_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_6.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_6_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_7.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_7_reg);
	color_sharp_dm_edge_neg_gain_curve_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_step_0_reg);
	color_sharp_dm_edge_neg_gain_curve_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_step_1_reg);
	color_sharp_dm_edge_neg_gain_curve_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_step_2_reg);
	color_sharp_dm_edge_neg_gain_curve_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_step_3_reg);
	color_sharp_dm_edge_neg_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_step_4_reg);

	color_sharp_dm_edge_pos_gain_curve_gain_0.s0 = ptr->Edge_Pos_Curve.G0;
	color_sharp_dm_edge_pos_gain_curve_gain_0.s1 = ptr->Edge_Pos_Curve.G1;
	color_sharp_dm_edge_pos_gain_curve_gain_1.s2 = ptr->Edge_Pos_Curve.G2;
	color_sharp_dm_edge_pos_gain_curve_gain_1.s3 = ptr->Edge_Pos_Curve.G3;
	color_sharp_dm_edge_pos_gain_curve_gain_2.s4 = ptr->Edge_Pos_Curve.G4;
	color_sharp_dm_edge_pos_gain_curve_gain_2.s5 = ptr->Edge_Pos_Curve.G5;
	color_sharp_dm_edge_pos_gain_curve_gain_3.s6 = ptr->Edge_Pos_Curve.G6;
	color_sharp_dm_edge_pos_gain_curve_gain_3.s7 = ptr->Edge_Pos_Curve.G7;
	color_sharp_dm_edge_pos_gain_curve_gain_4.s8 = ptr->Edge_Pos_Curve.G8;
	color_sharp_dm_edge_pos_gain_curve_gain_4.s9 = ptr->Edge_Pos_Curve.G9;
	color_sharp_dm_edge_pos_gain_curve_gain_5.s10 = ptr->Edge_Pos_Curve.G10;
	color_sharp_dm_edge_pos_gain_curve_gain_5.s11 = ptr->Edge_Pos_Curve.G11;
	color_sharp_dm_edge_pos_gain_curve_gain_6.s12 = ptr->Edge_Pos_Curve.G12;
	color_sharp_dm_edge_pos_gain_curve_gain_6.s13 = ptr->Edge_Pos_Curve.G13;
	color_sharp_dm_edge_pos_gain_curve_gain_7.s14 = ptr->Edge_Pos_Curve.G14;

	color_sharp_dm_edge_pos_gain_curve_step_0.s1 = ptr->Edge_Pos_Curve.S1;
	color_sharp_dm_edge_pos_gain_curve_step_0.s2 = ptr->Edge_Pos_Curve.S2;
	color_sharp_dm_edge_pos_gain_curve_step_0.s3 = ptr->Edge_Pos_Curve.S3;
	color_sharp_dm_edge_pos_gain_curve_step_1.s4 = ptr->Edge_Pos_Curve.S4;
	color_sharp_dm_edge_pos_gain_curve_step_1.s5 = ptr->Edge_Pos_Curve.S5;
	color_sharp_dm_edge_pos_gain_curve_step_1.s6 = ptr->Edge_Pos_Curve.S6;
	color_sharp_dm_edge_pos_gain_curve_step_2.s7 = ptr->Edge_Pos_Curve.S7;
	color_sharp_dm_edge_pos_gain_curve_step_2.s8 = ptr->Edge_Pos_Curve.S8;
	color_sharp_dm_edge_pos_gain_curve_step_2.s9 = ptr->Edge_Pos_Curve.S9;
	color_sharp_dm_edge_pos_gain_curve_step_3.s10 = ptr->Edge_Pos_Curve.S10;
	color_sharp_dm_edge_pos_gain_curve_step_3.s11 = ptr->Edge_Pos_Curve.S11;
	color_sharp_dm_edge_pos_gain_curve_step_3.s12 = ptr->Edge_Pos_Curve.S12;
	color_sharp_dm_edge_pos_gain_curve_step_4.s13 = ptr->Edge_Pos_Curve.S13;
	color_sharp_dm_edge_pos_gain_curve_step_4.s14 = ptr->Edge_Pos_Curve.S14;

	color_sharp_dm_edge_neg_gain_curve_gain_0.s0 = ptr->Edge_Neg_Curve.G0;
	color_sharp_dm_edge_neg_gain_curve_gain_0.s1 = ptr->Edge_Neg_Curve.G1;
	color_sharp_dm_edge_neg_gain_curve_gain_1.s2 = ptr->Edge_Neg_Curve.G2;
	color_sharp_dm_edge_neg_gain_curve_gain_1.s3 = ptr->Edge_Neg_Curve.G3;
	color_sharp_dm_edge_neg_gain_curve_gain_2.s4 = ptr->Edge_Neg_Curve.G4;
	color_sharp_dm_edge_neg_gain_curve_gain_2.s5 = ptr->Edge_Neg_Curve.G5;
	color_sharp_dm_edge_neg_gain_curve_gain_3.s6 = ptr->Edge_Neg_Curve.G6;
	color_sharp_dm_edge_neg_gain_curve_gain_3.s7 = ptr->Edge_Neg_Curve.G7;
	color_sharp_dm_edge_neg_gain_curve_gain_4.s8 = ptr->Edge_Neg_Curve.G8;
	color_sharp_dm_edge_neg_gain_curve_gain_4.s9 = ptr->Edge_Neg_Curve.G9;
	color_sharp_dm_edge_neg_gain_curve_gain_5.s10 = ptr->Edge_Neg_Curve.G10;
	color_sharp_dm_edge_neg_gain_curve_gain_5.s11 = ptr->Edge_Neg_Curve.G11;
	color_sharp_dm_edge_neg_gain_curve_gain_6.s12 = ptr->Edge_Neg_Curve.G12;
	color_sharp_dm_edge_neg_gain_curve_gain_6.s13 = ptr->Edge_Neg_Curve.G13;
	color_sharp_dm_edge_neg_gain_curve_gain_7.s14 = ptr->Edge_Neg_Curve.G14;

	color_sharp_dm_edge_neg_gain_curve_step_0.s1 = ptr->Edge_Neg_Curve.S1;
	color_sharp_dm_edge_neg_gain_curve_step_0.s2 = ptr->Edge_Neg_Curve.S2;
	color_sharp_dm_edge_neg_gain_curve_step_0.s3 = ptr->Edge_Neg_Curve.S3;
	color_sharp_dm_edge_neg_gain_curve_step_1.s4 = ptr->Edge_Neg_Curve.S4;
	color_sharp_dm_edge_neg_gain_curve_step_1.s5 = ptr->Edge_Neg_Curve.S5;
	color_sharp_dm_edge_neg_gain_curve_step_1.s6 = ptr->Edge_Neg_Curve.S6;
	color_sharp_dm_edge_neg_gain_curve_step_2.s7 = ptr->Edge_Neg_Curve.S7;
	color_sharp_dm_edge_neg_gain_curve_step_2.s8 = ptr->Edge_Neg_Curve.S8;
	color_sharp_dm_edge_neg_gain_curve_step_2.s9 = ptr->Edge_Neg_Curve.S9;
	color_sharp_dm_edge_neg_gain_curve_step_3.s10 = ptr->Edge_Neg_Curve.S10;
	color_sharp_dm_edge_neg_gain_curve_step_3.s11 = ptr->Edge_Neg_Curve.S11;
	color_sharp_dm_edge_neg_gain_curve_step_3.s12 = ptr->Edge_Neg_Curve.S12;
	color_sharp_dm_edge_neg_gain_curve_step_4.s13 = ptr->Edge_Neg_Curve.S13;
	color_sharp_dm_edge_neg_gain_curve_step_4.s14 = ptr->Edge_Neg_Curve.S14;

	color_sharp_dm_edge_neg_gain_curve_step_4.enable = ptr->Edge_Neg_Curve.enable;

	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_0_reg, color_sharp_dm_edge_pos_gain_curve_gain_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_1_reg, color_sharp_dm_edge_pos_gain_curve_gain_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_2_reg, color_sharp_dm_edge_pos_gain_curve_gain_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_3_reg, color_sharp_dm_edge_pos_gain_curve_gain_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_4_reg, color_sharp_dm_edge_pos_gain_curve_gain_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_5_reg, color_sharp_dm_edge_pos_gain_curve_gain_5.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_6_reg, color_sharp_dm_edge_pos_gain_curve_gain_6.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_7_reg, color_sharp_dm_edge_pos_gain_curve_gain_7.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_step_0_reg, color_sharp_dm_edge_pos_gain_curve_step_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_step_1_reg, color_sharp_dm_edge_pos_gain_curve_step_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_step_2_reg, color_sharp_dm_edge_pos_gain_curve_step_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_step_3_reg, color_sharp_dm_edge_pos_gain_curve_step_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_pos_gain_curve_step_4_reg, color_sharp_dm_edge_pos_gain_curve_step_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_0_reg, color_sharp_dm_edge_neg_gain_curve_gain_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_1_reg, color_sharp_dm_edge_neg_gain_curve_gain_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_2_reg, color_sharp_dm_edge_neg_gain_curve_gain_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_3_reg, color_sharp_dm_edge_neg_gain_curve_gain_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_4_reg, color_sharp_dm_edge_neg_gain_curve_gain_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_5_reg, color_sharp_dm_edge_neg_gain_curve_gain_5.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_6_reg, color_sharp_dm_edge_neg_gain_curve_gain_6.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_7_reg, color_sharp_dm_edge_neg_gain_curve_gain_7.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_step_0_reg, color_sharp_dm_edge_neg_gain_curve_step_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_step_1_reg, color_sharp_dm_edge_neg_gain_curve_step_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_step_2_reg, color_sharp_dm_edge_neg_gain_curve_step_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_step_3_reg, color_sharp_dm_edge_neg_gain_curve_step_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_edge_neg_gain_curve_step_4_reg, color_sharp_dm_edge_neg_gain_curve_step_4.regValue);
	}
/* set Edge Gain Curve end */

/* set Tex Gain Curve start */ /*magellan2*/
	{
	color_sharp_dm_tex_pos_gain_curve_gain_0_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_0;
	color_sharp_dm_tex_pos_gain_curve_gain_1_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_1;
	color_sharp_dm_tex_pos_gain_curve_gain_2_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_2;
	color_sharp_dm_tex_pos_gain_curve_gain_3_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_3;
	color_sharp_dm_tex_pos_gain_curve_gain_4_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_4;
	color_sharp_dm_tex_pos_gain_curve_gain_5_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_5;
	color_sharp_dm_tex_pos_gain_curve_gain_6_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_6;
	color_sharp_dm_tex_pos_gain_curve_gain_7_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_7;
	color_sharp_dm_tex_pos_gain_curve_step_0_RBUS	color_sharp_dm_tex_pos_gain_curve_step_0;
	color_sharp_dm_tex_pos_gain_curve_step_1_RBUS	color_sharp_dm_tex_pos_gain_curve_step_1;
	color_sharp_dm_tex_pos_gain_curve_step_2_RBUS	color_sharp_dm_tex_pos_gain_curve_step_2;
	color_sharp_dm_tex_pos_gain_curve_step_3_RBUS	color_sharp_dm_tex_pos_gain_curve_step_3;
	color_sharp_dm_tex_pos_gain_curve_step_4_RBUS	color_sharp_dm_tex_pos_gain_curve_step_4;
	color_sharp_dm_tex_neg_gain_curve_gain_0_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_0;
	color_sharp_dm_tex_neg_gain_curve_gain_1_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_1;
	color_sharp_dm_tex_neg_gain_curve_gain_2_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_2;
	color_sharp_dm_tex_neg_gain_curve_gain_3_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_3;
	color_sharp_dm_tex_neg_gain_curve_gain_4_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_4;
	color_sharp_dm_tex_neg_gain_curve_gain_5_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_5;
	color_sharp_dm_tex_neg_gain_curve_gain_6_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_6;
	color_sharp_dm_tex_neg_gain_curve_gain_7_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_7;
	color_sharp_dm_tex_neg_gain_curve_step_0_RBUS	color_sharp_dm_tex_neg_gain_curve_step_0;
	color_sharp_dm_tex_neg_gain_curve_step_1_RBUS	color_sharp_dm_tex_neg_gain_curve_step_1;
	color_sharp_dm_tex_neg_gain_curve_step_2_RBUS	color_sharp_dm_tex_neg_gain_curve_step_2;
	color_sharp_dm_tex_neg_gain_curve_step_3_RBUS	color_sharp_dm_tex_neg_gain_curve_step_3;
	color_sharp_dm_tex_neg_gain_curve_step_4_RBUS	color_sharp_dm_tex_neg_gain_curve_step_4;

	color_sharp_dm_tex_pos_gain_curve_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_0_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_1_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_2_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_3_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_4.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_4_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_5.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_5_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_6.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_6_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_7.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_7_reg);
	color_sharp_dm_tex_pos_gain_curve_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_step_0_reg);
	color_sharp_dm_tex_pos_gain_curve_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_step_1_reg);
	color_sharp_dm_tex_pos_gain_curve_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_step_2_reg);
	color_sharp_dm_tex_pos_gain_curve_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_step_3_reg);
	color_sharp_dm_tex_pos_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_step_4_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_0_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_1_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_2_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_3_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_4.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_4_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_5.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_5_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_6.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_6_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_7.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_7_reg);
	color_sharp_dm_tex_neg_gain_curve_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_step_0_reg);
	color_sharp_dm_tex_neg_gain_curve_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_step_1_reg);
	color_sharp_dm_tex_neg_gain_curve_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_step_2_reg);
	color_sharp_dm_tex_neg_gain_curve_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_step_3_reg);
	color_sharp_dm_tex_neg_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_step_4_reg);

	color_sharp_dm_tex_pos_gain_curve_gain_0.s0 = ptr->Tex_Pos_Curve.G0;
	color_sharp_dm_tex_pos_gain_curve_gain_0.s1 = ptr->Tex_Pos_Curve.G1;
	color_sharp_dm_tex_pos_gain_curve_gain_1.s2 = ptr->Tex_Pos_Curve.G2;
	color_sharp_dm_tex_pos_gain_curve_gain_1.s3 = ptr->Tex_Pos_Curve.G3;
	color_sharp_dm_tex_pos_gain_curve_gain_2.s4 = ptr->Tex_Pos_Curve.G4;
	color_sharp_dm_tex_pos_gain_curve_gain_2.s5 = ptr->Tex_Pos_Curve.G5;
	color_sharp_dm_tex_pos_gain_curve_gain_3.s6 = ptr->Tex_Pos_Curve.G6;
	color_sharp_dm_tex_pos_gain_curve_gain_3.s7 = ptr->Tex_Pos_Curve.G7;
	color_sharp_dm_tex_pos_gain_curve_gain_4.s8 = ptr->Tex_Pos_Curve.G8;
	color_sharp_dm_tex_pos_gain_curve_gain_4.s9 = ptr->Tex_Pos_Curve.G9;
	color_sharp_dm_tex_pos_gain_curve_gain_5.s10 = ptr->Tex_Pos_Curve.G10;
	color_sharp_dm_tex_pos_gain_curve_gain_5.s11 = ptr->Tex_Pos_Curve.G11;
	color_sharp_dm_tex_pos_gain_curve_gain_6.s12 = ptr->Tex_Pos_Curve.G12;
	color_sharp_dm_tex_pos_gain_curve_gain_6.s13 = ptr->Tex_Pos_Curve.G13;
	color_sharp_dm_tex_pos_gain_curve_gain_7.s14 = ptr->Tex_Pos_Curve.G14;

	color_sharp_dm_tex_pos_gain_curve_step_0.s1 = ptr->Tex_Pos_Curve.S1;
	color_sharp_dm_tex_pos_gain_curve_step_0.s2 = ptr->Tex_Pos_Curve.S2;
	color_sharp_dm_tex_pos_gain_curve_step_0.s3 = ptr->Tex_Pos_Curve.S3;
	color_sharp_dm_tex_pos_gain_curve_step_1.s4 = ptr->Tex_Pos_Curve.S4;
	color_sharp_dm_tex_pos_gain_curve_step_1.s5 = ptr->Tex_Pos_Curve.S5;
	color_sharp_dm_tex_pos_gain_curve_step_1.s6 = ptr->Tex_Pos_Curve.S6;
	color_sharp_dm_tex_pos_gain_curve_step_2.s7 = ptr->Tex_Pos_Curve.S7;
	color_sharp_dm_tex_pos_gain_curve_step_2.s8 = ptr->Tex_Pos_Curve.S8;
	color_sharp_dm_tex_pos_gain_curve_step_2.s9 = ptr->Tex_Pos_Curve.S9;
	color_sharp_dm_tex_pos_gain_curve_step_3.s10 = ptr->Tex_Pos_Curve.S10;
	color_sharp_dm_tex_pos_gain_curve_step_3.s11 = ptr->Tex_Pos_Curve.S11;
	color_sharp_dm_tex_pos_gain_curve_step_3.s12 = ptr->Tex_Pos_Curve.S12;
	color_sharp_dm_tex_pos_gain_curve_step_4.s13 = ptr->Tex_Pos_Curve.S13;
	color_sharp_dm_tex_pos_gain_curve_step_4.s14 = ptr->Tex_Pos_Curve.S14;

	color_sharp_dm_tex_neg_gain_curve_gain_0.s0 = ptr->Tex_Neg_Curve.G0;
	color_sharp_dm_tex_neg_gain_curve_gain_0.s1 = ptr->Tex_Neg_Curve.G1;
	color_sharp_dm_tex_neg_gain_curve_gain_1.s2 = ptr->Tex_Neg_Curve.G2;
	color_sharp_dm_tex_neg_gain_curve_gain_1.s3 = ptr->Tex_Neg_Curve.G3;
	color_sharp_dm_tex_neg_gain_curve_gain_2.s4 = ptr->Tex_Neg_Curve.G4;
	color_sharp_dm_tex_neg_gain_curve_gain_2.s5 = ptr->Tex_Neg_Curve.G5;
	color_sharp_dm_tex_neg_gain_curve_gain_3.s6 = ptr->Tex_Neg_Curve.G6;
	color_sharp_dm_tex_neg_gain_curve_gain_3.s7 = ptr->Tex_Neg_Curve.G7;
	color_sharp_dm_tex_neg_gain_curve_gain_4.s8 = ptr->Tex_Neg_Curve.G8;
	color_sharp_dm_tex_neg_gain_curve_gain_4.s9 = ptr->Tex_Neg_Curve.G9;
	color_sharp_dm_tex_neg_gain_curve_gain_5.s10 = ptr->Tex_Neg_Curve.G10;
	color_sharp_dm_tex_neg_gain_curve_gain_5.s11 = ptr->Tex_Neg_Curve.G11;
	color_sharp_dm_tex_neg_gain_curve_gain_6.s12 = ptr->Tex_Neg_Curve.G12;
	color_sharp_dm_tex_neg_gain_curve_gain_6.s13 = ptr->Tex_Neg_Curve.G13;
	color_sharp_dm_tex_neg_gain_curve_gain_7.s14 = ptr->Tex_Neg_Curve.G14;

	color_sharp_dm_tex_neg_gain_curve_step_0.s1 = ptr->Tex_Neg_Curve.S1;
	color_sharp_dm_tex_neg_gain_curve_step_0.s2 = ptr->Tex_Neg_Curve.S2;
	color_sharp_dm_tex_neg_gain_curve_step_0.s3 = ptr->Tex_Neg_Curve.S3;
	color_sharp_dm_tex_neg_gain_curve_step_1.s4 = ptr->Tex_Neg_Curve.S4;
	color_sharp_dm_tex_neg_gain_curve_step_1.s5 = ptr->Tex_Neg_Curve.S5;
	color_sharp_dm_tex_neg_gain_curve_step_1.s6 = ptr->Tex_Neg_Curve.S6;
	color_sharp_dm_tex_neg_gain_curve_step_2.s7 = ptr->Tex_Neg_Curve.S7;
	color_sharp_dm_tex_neg_gain_curve_step_2.s8 = ptr->Tex_Neg_Curve.S8;
	color_sharp_dm_tex_neg_gain_curve_step_2.s9 = ptr->Tex_Neg_Curve.S9;
	color_sharp_dm_tex_neg_gain_curve_step_3.s10 = ptr->Tex_Neg_Curve.S10;
	color_sharp_dm_tex_neg_gain_curve_step_3.s11 = ptr->Tex_Neg_Curve.S11;
	color_sharp_dm_tex_neg_gain_curve_step_3.s12 = ptr->Tex_Neg_Curve.S12;
	color_sharp_dm_tex_neg_gain_curve_step_4.s13 = ptr->Tex_Neg_Curve.S13;
	color_sharp_dm_tex_neg_gain_curve_step_4.s14 = ptr->Tex_Neg_Curve.S14;

	color_sharp_dm_tex_neg_gain_curve_step_4.enable = ptr->Tex_Neg_Curve.enable;

	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_0_reg, color_sharp_dm_tex_pos_gain_curve_gain_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_1_reg, color_sharp_dm_tex_pos_gain_curve_gain_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_2_reg, color_sharp_dm_tex_pos_gain_curve_gain_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_3_reg, color_sharp_dm_tex_pos_gain_curve_gain_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_4_reg, color_sharp_dm_tex_pos_gain_curve_gain_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_5_reg, color_sharp_dm_tex_pos_gain_curve_gain_5.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_6_reg, color_sharp_dm_tex_pos_gain_curve_gain_6.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_7_reg, color_sharp_dm_tex_pos_gain_curve_gain_7.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_step_0_reg, color_sharp_dm_tex_pos_gain_curve_step_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_step_1_reg, color_sharp_dm_tex_pos_gain_curve_step_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_step_2_reg, color_sharp_dm_tex_pos_gain_curve_step_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_step_3_reg, color_sharp_dm_tex_pos_gain_curve_step_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_pos_gain_curve_step_4_reg, color_sharp_dm_tex_pos_gain_curve_step_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_0_reg, color_sharp_dm_tex_neg_gain_curve_gain_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_1_reg, color_sharp_dm_tex_neg_gain_curve_gain_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_2_reg, color_sharp_dm_tex_neg_gain_curve_gain_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_3_reg, color_sharp_dm_tex_neg_gain_curve_gain_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_4_reg, color_sharp_dm_tex_neg_gain_curve_gain_4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_5_reg, color_sharp_dm_tex_neg_gain_curve_gain_5.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_6_reg, color_sharp_dm_tex_neg_gain_curve_gain_6.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_7_reg, color_sharp_dm_tex_neg_gain_curve_gain_7.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_step_0_reg, color_sharp_dm_tex_neg_gain_curve_step_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_step_1_reg, color_sharp_dm_tex_neg_gain_curve_step_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_step_2_reg, color_sharp_dm_tex_neg_gain_curve_step_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_step_3_reg, color_sharp_dm_tex_neg_gain_curve_step_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_step_4_reg, color_sharp_dm_tex_neg_gain_curve_step_4.regValue);
	}
/* set Tex Gain Curve end */

/* set SR start */
	{
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
	color_sharp_dm_sr_continual_detail_gain_RBUS	color_sharp_dm_sr_continual_detail_gain;
	color_sharp_dm_sr_ctrl_3_RBUS color_sharp_dm_sr_ctrl_3;

	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
	color_sharp_dm_sr_continual_detail_gain.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_continual_detail_gain_reg);
	color_sharp_dm_sr_ctrl_3.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_3_reg);

	color_sharp_dm_sr_continual_detail_gain.gain = ptr->SR_Control.Gain;
	color_sharp_dm_sr_continual_detail_gain.offset = ptr->SR_Control.Offset;
	color_sharp_dm_sr_continual_detail_gain.period = ptr->SR_Control.Period;
	color_sharp_dm_sr_continual_detail_gain.sr_detail_tex_en = ptr->SR_Control.sr_detail_tex_en;
	// removed color_sharp_dm_sr_continual_detail_gain.sr_detail_edg_en = ptr->SR_Control.sr_detail_edg_en;
	color_sharp_dm_sr_continual_detail_gain.sr_detail_v_tex_en = ptr->SR_Control.sr_detail_v_tex_en;
	// removed color_sharp_dm_sr_continual_detail_gain.sr_detail_v_edg_en = ptr->SR_Control.sr_detail_v_edg_en;
	color_sharp_dm_sr_ctrl_3.sr_1_touch_edge = ptr->SR_Control.sr_1_touch_edge;
	color_sharp_dm_sr_ctrl_3.sr_1_pxdiff_lim = ptr->SR_Control.sr_1_pxdiff_lim;

	/*DC Value Check*/
	if (SHARPNESS_LOG) {
		int sum;

		/*SR_H9 nonlinear*/

		/*SR_H13*/
		sum = 0;
		sum = ptr->SR_H13.C0 +
		      2*(ptr->SR_H13.C1 + ptr->SR_H13.C2 + ptr->SR_H13.C3 + ptr->SR_H13.C4 + ptr->SR_H13.C5 + ptr->SR_H13.C6);
		if (sum != 0)
			VIPprintf("[Warnning] [%s-%s-%d] [SR_H13 Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);

		/*SR_H21*/
		sum = 0;
		sum = ptr->SR_H21.C0 +
		      2*(ptr->SR_H21.C1 + ptr->SR_H21.C2 + ptr->SR_H21.C3 + ptr->SR_H21.C4 + ptr->SR_H21.C5 + ptr->SR_H21.C6 + ptr->SR_H21.C7 + ptr->SR_H21.C8 + ptr->SR_H21.C9 + ptr->SR_H21.C10);
		if (sum != 0)
			VIPprintf("[Warnning] [%s-%s-%d] [SR_H21 Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);

		/*SR_V9_1 nonlinear*/

		/*SR_V9_2*/
		sum = 0;
		sum = ptr->SR_V9_2.C0 +
		      2*(ptr->SR_V9_2.C1 + ptr->SR_V9_2.C2 + ptr->SR_V9_2.C3 + ptr->SR_V9_2.C4);
		if (sum != 0)
			VIPprintf("[Warnning] [%s-%s-%d] [SR_V9_2 Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);

		/*SR_V9_3*/
		sum = 0;
		sum = ptr->SR_V9_3.C0 +
		      2*(ptr->SR_V9_3.C1 + ptr->SR_V9_3.C2 + ptr->SR_V9_3.C3 + ptr->SR_V9_3.C4);
		if (sum != 0)
			VIPprintf("[Warnning] [%s-%s-%d] [SR_V9_3 Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);

		/*SR_Ring_H11*/
		sum = 0;
		sum = ptr->SR_Ring_H11.C0 +
		      2*(ptr->SR_Ring_H11.C1 + ptr->SR_Ring_H11.C2 + ptr->SR_Ring_H11.C3 + ptr->SR_Ring_H11.C4 + ptr->SR_Ring_H11.C5);
		if (sum != 0)
			VIPprintf("[Warnning] [%s-%s-%d] [SR_Ring_H11 Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);

		/*SR_Ring_V9*/
		sum = 0;
		sum = ptr->SR_Ring_V9.C0 +
		      2*(ptr->SR_Ring_V9.C1 + ptr->SR_Ring_V9.C2 + ptr->SR_Ring_V9.C3 + ptr->SR_Ring_V9.C4);
		if (sum != 0)
			VIPprintf("[Warnning] [%s-%s-%d] [SR_Ring_V9 Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);
	}

	//if (drvif_color_Set_Sharpness_PowerSaving_Mute(1) == 0) {
		drvif_color_Set_Sharpness_PowerSaving_SR_H0(ptr);
		drvif_color_Set_Sharpness_PowerSaving_SR_H1(ptr);
		drvif_color_Set_Sharpness_PowerSaving_SR_H2(ptr);
		drvif_color_Set_Sharpness_PowerSaving_SR_H3(ptr);
		drvif_color_Set_Sharpness_PowerSaving_SR_V0(ptr);
		drvif_color_Set_Sharpness_PowerSaving_SR_V1(ptr);
		drvif_color_Set_Sharpness_PowerSaving_SR_V2(ptr);
		drvif_color_Set_Sharpness_PowerSaving_SR_V3(ptr);
		drvif_color_Set_Sharpness_PowerSaving_SR_X(ptr);
		//drvif_color_Set_Sharpness_PowerSaving_Mute(0);
	//}

	//IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_reg, color_sharp_dm_sr_ctrl.regValue); // fixed value
	IoReg_Write32(COLOR_SHARP_DM_SR_Ctrl_3_reg, color_sharp_dm_sr_ctrl_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SR_continual_detail_gain_reg, color_sharp_dm_sr_continual_detail_gain.regValue);
	}
/* set SR end */
/* Merlin3 no Edge SM
	{

	color_sharp_dm_edge_sm_ctrl1.dirsm_rangh = ptr->Edge_SM.dirsm_rangh;
	color_sharp_dm_edge_sm_ctrl1.dirsm_rangv= ptr->Edge_SM.dirsm_rangv;
	color_sharp_dm_edge_sm_ctrl1.meandiff_rangeh= ptr->Edge_SM.meandiff_rangeh;
	color_sharp_dm_edge_sm_ctrl1.meandiff_rangev= ptr->Edge_SM.meandiff_rangev;
	color_sharp_dm_edge_sm_ctrl1.meandiff_shiftbit= ptr->Edge_SM.meandiff_shiftbit;
	color_sharp_dm_edge_sm_ctrl1.meandiff_step= ptr->Edge_SM.meandiff_step;
	color_sharp_dm_edge_sm_ctrl1.meandiff_lowbd= ptr->Edge_SM.meandiff_lowbd;
	color_sharp_dm_edge_sm_ctrl2.edgeindex_lowbd= ptr->Edge_SM.edgeindex_lowbd;
	color_sharp_dm_edge_sm_ctrl2.edgeindex_step= ptr->Edge_SM.edgeindex_step;
	color_sharp_dm_edge_sm_ctrl2.edge_method_sel= ptr->Edge_SM.edge_method_sel;

	}
 set Edge SM end
set Vertical edge start  */
	{
	color_sharp_dm_segpk_edgpk_vsourceslope_RBUS	color_sharp_dm_segpk_edgpk_vsourceslope;
	color_sharp_dm_segpk_edgpk_sourceslope_RBUS color_sharp_dm_segpk_edgpk_sourceslope;
	color_sharp_dm_segpk_sourceslope_extend_RBUS color_sharp_dm_segpk_sourceslope_extend;
	color_sharp_dm_segpk_vpk6_RBUS	color_sharp_dm_segpk_vpk6;

	color_sharp_dm_segpk_edgpk_vsourceslope.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK_VSOURCESLOPE_reg);
	color_sharp_dm_segpk_edgpk_sourceslope.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK_SOURCESLOPE_reg);
	color_sharp_dm_segpk_sourceslope_extend.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_SOURCESLOPE_EXTEND_reg);
	color_sharp_dm_segpk_vpk6.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK6_reg);

	color_sharp_dm_segpk_edgpk_vsourceslope.v_slope_gain_en= ptr->MK3_Table.Slope_gain_v_en;
	color_sharp_dm_segpk_edgpk_vsourceslope.dir_v= ptr->MK3_Table.dir_v;
	color_sharp_dm_segpk_edgpk_vsourceslope.gain_boundpos_v= ptr->MK3_Table.Gain_boundPos_v;
	color_sharp_dm_segpk_edgpk_vsourceslope.gain_boundneg_v= ptr->MK3_Table.Gain_boundNeg_v;
	color_sharp_dm_segpk_edgpk_vsourceslope.gain_extendpos_v= ptr->MK3_Table.Gain_extendPos_v;
	color_sharp_dm_segpk_edgpk_vsourceslope.gain_extendneg_v= ptr->MK3_Table.Gain_extendNeg_v;
	color_sharp_dm_segpk_sourceslope_extend.lowbound_v= ptr->MK3_Table.lowbound_v;
	color_sharp_dm_segpk_edgpk_sourceslope.use_ramp = ptr->MK3_Table.use_ramp;
	color_sharp_dm_segpk_sourceslope_extend.flat_range_v= ptr->MK3_Table.flat_range_v;

	color_sharp_dm_segpk_vpk6.edg_lv2 = ptr->Vertical.LV2_edge;
	color_sharp_dm_segpk_vpk6.vpk_edg_gain_pos2= ptr->Vertical.G_Pos2_edge;
	color_sharp_dm_segpk_vpk6.vpk_edg_gain_neg2= ptr->Vertical.G_Neg2_edge;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK_VSOURCESLOPE_reg, color_sharp_dm_segpk_edgpk_vsourceslope.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK_SOURCESLOPE_reg, color_sharp_dm_segpk_edgpk_sourceslope.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_SOURCESLOPE_EXTEND_reg, color_sharp_dm_segpk_sourceslope_extend.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK6_reg, color_sharp_dm_segpk_vpk6.regValue);
	}
/* set Vertical edge end  */
/* set Merlin2 start  */
	{
	color_sharp_dm_segpk_ise_RBUS	color_sharp_dm_segpk_ise;
	color_sharp_dm_segpk_ise_2_RBUS color_sharp_dm_segpk_ise_2;
	color_sharp_dm_2d_shp_tex_flat_0_RBUS	color_sharp_dm_2d_shp_tex_flat_0;
	color_sharp_dm_2d_shp_tex_flat_1_RBUS	color_sharp_dm_2d_shp_tex_flat_1;

	color_sharp_dm_ramp_peaking_0_RBUS color_sharp_dm_ramp_peaking_0;
	color_sharp_dm_ramp_peaking_1_RBUS color_sharp_dm_ramp_peaking_1;
	color_sharp_dm_ramp_peaking_2_RBUS color_sharp_dm_ramp_peaking_2;
	color_sharp_dm_ramp_peaking_3_RBUS color_sharp_dm_ramp_peaking_3;
	color_sharp_dm_ramp_peaking_v_0_RBUS color_sharp_dm_ramp_peaking_v_0;
	color_sharp_dm_ramp_peaking_v_1_RBUS color_sharp_dm_ramp_peaking_v_1;
	color_sharp_dm_ramp_peaking_v_2_RBUS color_sharp_dm_ramp_peaking_v_2;
	color_sharp_dm_ramp_peaking_v_3_RBUS color_sharp_dm_ramp_peaking_v_3;
	color_sharp_sub_idx_edg_gain_RBUS	color_sharp_sub_idx_edg_gain;
	color_sharp_sub_idx_edg_hv_RBUS	color_sharp_sub_idx_edg_hv;
	color_sharp_sub_idx_tex_gain_RBUS	color_sharp_sub_idx_tex_gain;
	color_sharp_sub_idx_tex_hv_RBUS	color_sharp_sub_idx_tex_hv;
	color_sharp_sub_idx_v_gain_RBUS	color_sharp_sub_idx_v_gain;
	color_sharp_sub_idx_v_hv_RBUS	color_sharp_sub_idx_v_hv;
	color_sharp_sub_idx_v_edg_gain_RBUS	color_sharp_sub_idx_v_edg_gain;
	color_sharp_sub_idx_v_edg_hv_RBUS	color_sharp_sub_idx_v_edg_hv;

	color_sharp_dm_segpk_ise.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_ISE_reg);
	color_sharp_dm_segpk_ise_2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_ISE_2_reg);

	color_sharp_dm_2d_shp_tex_flat_0.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_FLAT_0_reg);
	color_sharp_dm_2d_shp_tex_flat_1.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_FLAT_1_reg);

	color_sharp_dm_ramp_peaking_0.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_0_reg);
	color_sharp_dm_ramp_peaking_1.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_1_reg);
	color_sharp_dm_ramp_peaking_2.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_2_reg);
	color_sharp_dm_ramp_peaking_3.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_3_reg);
	color_sharp_dm_ramp_peaking_v_0.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_V_0_reg);
	color_sharp_dm_ramp_peaking_v_1.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_V_1_reg);
	color_sharp_dm_ramp_peaking_v_2.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_V_2_reg);
	color_sharp_dm_ramp_peaking_v_3.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_V_3_reg);
	color_sharp_sub_idx_edg_gain.regValue = IoReg_Read32(COLOR_SHARP_SUB_IDX_EDG_GAIN_reg);
	color_sharp_sub_idx_edg_hv.regValue = IoReg_Read32(COLOR_SHARP_SUB_IDX_EDG_HV_reg);
	color_sharp_sub_idx_tex_gain.regValue = IoReg_Read32(COLOR_SHARP_Sub_IDX_TEX_GAIN_reg);
	color_sharp_sub_idx_tex_hv.regValue = IoReg_Read32(COLOR_SHARP_SUB_IDX_TEX_HV_reg);
	color_sharp_sub_idx_v_gain.regValue = IoReg_Read32(COLOR_SHARP_Sub_IDX_V_GAIN_reg);
	color_sharp_sub_idx_v_hv.regValue = IoReg_Read32(COLOR_SHARP_SUB_IDX_V_HV_reg);
	color_sharp_sub_idx_v_edg_gain.regValue = IoReg_Read32(COLOR_SHARP_Sub_IDX_V_EDG_GAIN_reg);
	color_sharp_sub_idx_v_edg_hv.regValue = IoReg_Read32(COLOR_SHARP_SUB_IDX_V_EDG_HV_reg);

	color_sharp_sub_idx_edg_gain.sub_index_en= ptr->Sub_Gain_Modify.sub_index_en;
	color_sharp_sub_idx_edg_gain.gain_pos= ptr->Sub_Gain_Modify.EDG.Gain_Pos;
	color_sharp_sub_idx_edg_gain.gain_neg= ptr->Sub_Gain_Modify.EDG.Gain_Neg;
	color_sharp_sub_idx_edg_hv.hv_pos= ptr->Sub_Gain_Modify.EDG.HV_Pos;
	color_sharp_sub_idx_edg_hv.hv_neg= ptr->Sub_Gain_Modify.EDG.HV_Neg;
	color_sharp_sub_idx_edg_hv.lv= ptr->Sub_Gain_Modify.EDG.LV;
	color_sharp_sub_idx_tex_gain.gain_pos= ptr->Sub_Gain_Modify.TEX.Gain_Pos;
	color_sharp_sub_idx_tex_gain.gain_neg= ptr->Sub_Gain_Modify.TEX.Gain_Neg;
	color_sharp_sub_idx_tex_hv.hv_pos= ptr->Sub_Gain_Modify.TEX.HV_Pos;
	color_sharp_sub_idx_tex_hv.hv_neg= ptr->Sub_Gain_Modify.TEX.HV_Neg;
	color_sharp_sub_idx_tex_hv.lv= ptr->Sub_Gain_Modify.TEX.LV;
	color_sharp_sub_idx_v_gain.gain_pos= ptr->Sub_Gain_Modify.V.Gain_Pos;
	color_sharp_sub_idx_v_gain.gain_neg= ptr->Sub_Gain_Modify.V.Gain_Neg;
	color_sharp_sub_idx_v_hv.hv_pos= ptr->Sub_Gain_Modify.V.HV_Pos;
	color_sharp_sub_idx_v_hv.hv_neg= ptr->Sub_Gain_Modify.V.HV_Neg;
	color_sharp_sub_idx_v_hv.lv= ptr->Sub_Gain_Modify.V.LV;
	color_sharp_sub_idx_v_edg_gain.gain_pos= ptr->Sub_Gain_Modify.V_EDG.Gain_Pos;
	color_sharp_sub_idx_v_edg_gain.gain_neg= ptr->Sub_Gain_Modify.V_EDG.Gain_Neg;
	color_sharp_sub_idx_v_edg_hv.hv_pos= ptr->Sub_Gain_Modify.V_EDG.HV_Pos;
	color_sharp_sub_idx_v_edg_hv.hv_neg= ptr->Sub_Gain_Modify.V_EDG.HV_Neg;
	color_sharp_sub_idx_v_edg_hv.lv= ptr->Sub_Gain_Modify.V_EDG.LV;

	color_sharp_dm_segpk_ise.ise_en= ptr->SEGPK_ISE.ise_en;
	color_sharp_dm_segpk_ise.ise_step= ptr->SEGPK_ISE.ise_step;
	// color_sharp_dm_segpk_ise.ise_scale= ptr->SEGPK_ISE.ise_scale;
	color_sharp_dm_segpk_ise.ise_ub= ptr->SEGPK_ISE.ise_ub;
	color_sharp_dm_segpk_ise.ise_h_st= ptr->SEGPK_ISE.ise_h_st;
	color_sharp_dm_segpk_ise.ise_h_end= ptr->SEGPK_ISE.ise_h_end;
	color_sharp_dm_segpk_ise.ise_v_st= ptr->SEGPK_ISE.ise_v_st;
	color_sharp_dm_segpk_ise.ise_v_end= ptr->SEGPK_ISE.ise_v_end;
	color_sharp_dm_segpk_ise_2.ise_mul_max= ptr->SEGPK_ISE.ise_mul_max;
	color_sharp_dm_segpk_ise_2.ise_mul_min= ptr->SEGPK_ISE.ise_mul_min;

	color_sharp_dm_2d_shp_tex_flat_0.tex_flat_filter_en= ptr->Texture_Flat_Shp.en_tex;
	color_sharp_dm_2d_shp_tex_flat_0.coef_c0_0= ptr->Texture_Flat_Shp.C0;
	color_sharp_dm_2d_shp_tex_flat_1.coef_c1_0= ptr->Texture_Flat_Shp.C1;
	color_sharp_dm_2d_shp_tex_flat_1.coef_c2_0= ptr->Texture_Flat_Shp.C2;
	color_sharp_dm_2d_shp_tex_flat_1.coef_c3_0= ptr->Texture_Flat_Shp.C3;
	color_sharp_dm_2d_shp_tex_flat_0.coef_c0_1= ptr->Texture_Flat_Shp.C0H1;
	color_sharp_dm_2d_shp_tex_flat_0.coef_c0_2= ptr->Texture_Flat_Shp.C0H2;

	color_sharp_dm_ramp_peaking_0.ramppk_en          =ptr->Ramp_H.range          ;
	color_sharp_dm_ramp_peaking_0.ramp_h_avgline     =ptr->Ramp_H.avgline        ;
	color_sharp_dm_ramp_peaking_0.ramp_h_center_value=ptr->Ramp_H.center_value   ;
	color_sharp_dm_ramp_peaking_0.ramp_h_maxmin_value=ptr->Ramp_H.maxmin_value   ;
	color_sharp_dm_ramp_peaking_0.ramp_h_const_value =ptr->Ramp_H.const_value    ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved05     ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved06     ;
	color_sharp_dm_ramp_peaking_1.ramp_h_texgain=ptr->Ramp_H.texgain        ;
	// color_sharp_dm_ramp_peaking_1.ramp_h_weight0=ptr->Ramp_H.weight0        ;
	color_sharp_dm_ramp_peaking_1.ramp_h_weight1=ptr->Ramp_H.weight1        ;
	color_sharp_dm_ramp_peaking_1.ramp_h_weight2=ptr->Ramp_H.weight2        ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved11     ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved12     ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved13     ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved14     ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved15     ;
	color_sharp_dm_ramp_peaking_3.lv       =ptr->Ramp_H.getd2_lv       ;
	color_sharp_dm_ramp_peaking_2.gain_pos =ptr->Ramp_H.getd2_gain_pos ;
	color_sharp_dm_ramp_peaking_2.gain_neg =ptr->Ramp_H.getd2_gain_neg ;
	color_sharp_dm_ramp_peaking_3.hv_pos   =ptr->Ramp_H.getd2_hv_pos   ;
	color_sharp_dm_ramp_peaking_3.hv_neg   =ptr->Ramp_H.getd2_hv_neg   ;
	color_sharp_dm_ramp_peaking_2.gain_pos2=ptr->Ramp_H.getd2_gain_pos2;
	color_sharp_dm_ramp_peaking_2.gain_neg2=ptr->Ramp_H.getd2_gain_neg2;
	color_sharp_dm_ramp_peaking_2.lv2      =ptr->Ramp_H.getd2_lv2      ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved24     ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved25     ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved26     ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved27     ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved28     ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved29     ;
	// color_sharp_dm_ramp_peaking_0.ramppk_en=ptr->Ramp_H.reserved30     ;

	color_sharp_dm_ramp_peaking_v_0.vramppk_en          =ptr->Ramp_V.range          ;
	color_sharp_dm_ramp_peaking_v_0.ramp_v_avgline     =ptr->Ramp_V.avgline        ;
	color_sharp_dm_ramp_peaking_v_0.ramp_v_center_value=ptr->Ramp_V.center_value   ;
	color_sharp_dm_ramp_peaking_v_0.ramp_v_maxmin_value=ptr->Ramp_V.maxmin_value   ;
	color_sharp_dm_ramp_peaking_v_0.ramp_v_const_value =ptr->Ramp_V.const_value    ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved05     ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved06     ;
	color_sharp_dm_ramp_peaking_v_1.ramp_v_texgain=ptr->Ramp_V.texgain        ;
	// color_sharp_dm_ramp_peaking_v_1.ramp_v_weight0=ptr->Ramp_V.weight0        ;
	color_sharp_dm_ramp_peaking_v_1.ramp_v_weight1=ptr->Ramp_V.weight1        ;
	color_sharp_dm_ramp_peaking_v_1.ramp_v_weight2=ptr->Ramp_V.weight2        ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved11     ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved12     ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved13     ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved14     ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved15     ;
	color_sharp_dm_ramp_peaking_v_3.lv_v       =ptr->Ramp_V.getd2_lv       ;
	color_sharp_dm_ramp_peaking_v_2.gain_posv  =ptr->Ramp_V.getd2_gain_pos ;
	color_sharp_dm_ramp_peaking_v_2.gain_negv  =ptr->Ramp_V.getd2_gain_neg ;
	color_sharp_dm_ramp_peaking_v_3.hv_posv    =ptr->Ramp_V.getd2_hv_pos   ;
	color_sharp_dm_ramp_peaking_v_3.hv_negv    =ptr->Ramp_V.getd2_hv_neg   ;
	color_sharp_dm_ramp_peaking_v_2.gain_posv2 =ptr->Ramp_V.getd2_gain_pos2;
	color_sharp_dm_ramp_peaking_v_2.gain_negv2 =ptr->Ramp_V.getd2_gain_neg2;
	color_sharp_dm_ramp_peaking_v_2.lv2_v      =ptr->Ramp_V.getd2_lv2      ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved24     ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved25     ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved26     ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved27     ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved28     ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved29     ;
	// color_sharp_dm_ramp_peaking_v_0.ramppk_en=ptr->Ramp_V.reserved30     ;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_ISE_reg, color_sharp_dm_segpk_ise.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_ISE_2_reg, color_sharp_dm_segpk_ise_2.regValue);

	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_FLAT_0_reg, color_sharp_dm_2d_shp_tex_flat_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_2D_SHP_TEX_FLAT_1_reg, color_sharp_dm_2d_shp_tex_flat_1.regValue);

	IoReg_Write32(COLOR_SHARP_DM_RAMP_PEAKING_0_reg, color_sharp_dm_ramp_peaking_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_RAMP_PEAKING_1_reg, color_sharp_dm_ramp_peaking_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_RAMP_PEAKING_2_reg, color_sharp_dm_ramp_peaking_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_RAMP_PEAKING_3_reg, color_sharp_dm_ramp_peaking_3.regValue);
	IoReg_Write32(COLOR_SHARP_DM_RAMP_PEAKING_V_0_reg, color_sharp_dm_ramp_peaking_v_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_RAMP_PEAKING_V_1_reg, color_sharp_dm_ramp_peaking_v_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_RAMP_PEAKING_V_2_reg, color_sharp_dm_ramp_peaking_v_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_RAMP_PEAKING_V_3_reg, color_sharp_dm_ramp_peaking_v_3.regValue);
	IoReg_Write32(COLOR_SHARP_SUB_IDX_EDG_GAIN_reg, color_sharp_sub_idx_edg_gain.regValue);
	IoReg_Write32(COLOR_SHARP_SUB_IDX_EDG_HV_reg, color_sharp_sub_idx_edg_hv.regValue);
	IoReg_Write32(COLOR_SHARP_Sub_IDX_TEX_GAIN_reg, color_sharp_sub_idx_tex_gain.regValue);
	IoReg_Write32(COLOR_SHARP_SUB_IDX_TEX_HV_reg, color_sharp_sub_idx_tex_hv.regValue);
	IoReg_Write32(COLOR_SHARP_Sub_IDX_V_GAIN_reg, color_sharp_sub_idx_v_gain.regValue);
	IoReg_Write32(COLOR_SHARP_SUB_IDX_V_HV_reg, color_sharp_sub_idx_v_hv.regValue);
	IoReg_Write32(COLOR_SHARP_Sub_IDX_V_EDG_GAIN_reg, color_sharp_sub_idx_v_edg_gain.regValue);
	IoReg_Write32(COLOR_SHARP_SUB_IDX_V_EDG_HV_reg, color_sharp_sub_idx_v_edg_hv.regValue);
	}
/* set Merlin2 end  */
/* What the drvif_color_Set_Sharpness_Table committer needs to copy ends here. */

	{
	/*no set value yet
	color_sharp_dm_auto_shp_data_RBUS	color_sharp_dm_auto_shp_data;
	color_sharp_dm_auto_shp_fir_count_00_RBUS	color_sharp_dm_auto_shp_fir_count_00;
	color_sharp_dm_auto_shp_fir_count_01_RBUS	color_sharp_dm_auto_shp_fir_count_01;
	color_sharp_dm_auto_shp_fir_count_02_RBUS	color_sharp_dm_auto_shp_fir_count_02;
	color_sharp_dm_auto_shp_fir_count_03_RBUS	color_sharp_dm_auto_shp_fir_count_03;
	color_sharp_dm_auto_shp_fir_count_04_RBUS	color_sharp_dm_auto_shp_fir_count_04;
	color_sharp_dm_auto_shp_fir_count_05_RBUS	color_sharp_dm_auto_shp_fir_count_05;
	color_sharp_dm_auto_shp_fir_count_06_RBUS	color_sharp_dm_auto_shp_fir_count_06;
	color_sharp_dm_auto_shp_fir_count_07_RBUS	color_sharp_dm_auto_shp_fir_count_07;
	color_sharp_dm_auto_shp_fir_count_08_RBUS	color_sharp_dm_auto_shp_fir_count_08;
	color_sharp_dm_auto_shp_fir_count_09_RBUS	color_sharp_dm_auto_shp_fir_count_09;
	color_sharp_dm_auto_shp_fir_count_10_RBUS	color_sharp_dm_auto_shp_fir_count_10;
	color_sharp_dm_auto_shp_fir_count_11_RBUS	color_sharp_dm_auto_shp_fir_count_11;
	color_sharp_dm_auto_shp_fir_count_12_RBUS	color_sharp_dm_auto_shp_fir_count_12;
	color_sharp_dm_auto_shp_fir_count_13_RBUS	color_sharp_dm_auto_shp_fir_count_13;
	color_sharp_dm_auto_shp_fir_count_14_RBUS	color_sharp_dm_auto_shp_fir_count_14;
	color_sharp_dm_auto_shp_fir_count_15_RBUS	color_sharp_dm_auto_shp_fir_count_15;

	color_sharp_dm_auto_shp_data.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_DATA_reg);
	color_sharp_dm_auto_shp_fir_count_00.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_00_reg);
	color_sharp_dm_auto_shp_fir_count_01.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_01_reg);
	color_sharp_dm_auto_shp_fir_count_02.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_02_reg);
	color_sharp_dm_auto_shp_fir_count_03.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_03_reg);
	color_sharp_dm_auto_shp_fir_count_04.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_04_reg);
	color_sharp_dm_auto_shp_fir_count_05.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_05_reg);
	color_sharp_dm_auto_shp_fir_count_06.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_06_reg);
	color_sharp_dm_auto_shp_fir_count_07.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_07_reg);
	color_sharp_dm_auto_shp_fir_count_08.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_08_reg);
	color_sharp_dm_auto_shp_fir_count_09.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_09_reg);
	color_sharp_dm_auto_shp_fir_count_10.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_10_reg);
	color_sharp_dm_auto_shp_fir_count_11.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_11_reg);
	color_sharp_dm_auto_shp_fir_count_12.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_12_reg);
	color_sharp_dm_auto_shp_fir_count_13.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_13_reg);
	color_sharp_dm_auto_shp_fir_count_14.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_14_reg);
	color_sharp_dm_auto_shp_fir_count_15.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_15_reg);

	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_DATA_reg, color_sharp_dm_auto_shp_data.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_00_reg, color_sharp_dm_auto_shp_fir_count_00.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_01_reg, color_sharp_dm_auto_shp_fir_count_01.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_02_reg, color_sharp_dm_auto_shp_fir_count_02.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_03_reg, color_sharp_dm_auto_shp_fir_count_03.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_04_reg, color_sharp_dm_auto_shp_fir_count_04.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_05_reg, color_sharp_dm_auto_shp_fir_count_05.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_06_reg, color_sharp_dm_auto_shp_fir_count_06.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_07_reg, color_sharp_dm_auto_shp_fir_count_07.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_08_reg, color_sharp_dm_auto_shp_fir_count_08.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_09_reg, color_sharp_dm_auto_shp_fir_count_09.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_10_reg, color_sharp_dm_auto_shp_fir_count_10.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_11_reg, color_sharp_dm_auto_shp_fir_count_11.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_12_reg, color_sharp_dm_auto_shp_fir_count_12.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_13_reg, color_sharp_dm_auto_shp_fir_count_13.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_14_reg, color_sharp_dm_auto_shp_fir_count_14.regValue);
	IoReg_Write32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_15_reg, color_sharp_dm_auto_shp_fir_count_15.regValue);
	*/
	}

	IoReg_Write32(COLOR_SHARP_DM_NR_SHP_CTRL_reg, color_sharp_dm_nr_shp_ctrl.regValue);

	/* no set value yet
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_DBG_reg, color_sharp_dm_segpk_dbg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_PEAKING_BOUND_1_reg, color_sharp_dm_peaking_bound_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_EDGPK4_reg, color_sharp_dm_segpk_edgpk4.regValue);
	IoReg_Write32(COLOR_SHARP_DM_GAIN_CURVE_DEBUG_reg, color_sharp_dm_gain_curve_debug.regValue);
	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_reg.regValue);
	*/

	color_sharp_dm_nr_shp_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_NR_SHP_CTRL_reg);
	color_sharp_dm_nr_shp_ctrl.db_en=1;
	color_sharp_dm_nr_shp_ctrl.db_load=1;
	IoReg_Write32(COLOR_SHARP_DM_NR_SHP_CTRL_reg,color_sharp_dm_nr_shp_ctrl.regValue);
	drvif_color_Set_Sharpness_db_apply_1();
}

void drvif_color_Set_Sharpness_gain_by_y_only_gain(unsigned short gain[7])
{
	color_sharp_dm_maxmin_gain_by_y_gain_0_RBUS color_sharp_dm_maxmin_gain_by_y_gain_0;
	color_sharp_dm_maxmin_gain_by_y_gain_1_RBUS color_sharp_dm_maxmin_gain_by_y_gain_1;
	color_sharp_dm_maxmin_gain_by_y_gain_2_RBUS color_sharp_dm_maxmin_gain_by_y_gain_2;
	color_sharp_dm_maxmin_gain_by_y_gain_3_RBUS color_sharp_dm_maxmin_gain_by_y_gain_3;

	color_sharp_dm_maxmin_gain_by_y_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_0_reg);
	color_sharp_dm_maxmin_gain_by_y_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_1_reg);
	color_sharp_dm_maxmin_gain_by_y_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_2_reg);
	color_sharp_dm_maxmin_gain_by_y_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_3_reg);

	color_sharp_dm_maxmin_gain_by_y_gain_0.s0 = gain[0];
	color_sharp_dm_maxmin_gain_by_y_gain_0.s1 = gain[1];
	color_sharp_dm_maxmin_gain_by_y_gain_1.s2 = gain[2];
	color_sharp_dm_maxmin_gain_by_y_gain_1.s3 = gain[3];
	color_sharp_dm_maxmin_gain_by_y_gain_2.s4 = gain[4];
	color_sharp_dm_maxmin_gain_by_y_gain_2.s5 = gain[5];
	color_sharp_dm_maxmin_gain_by_y_gain_3.s6 = gain[6];

	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_0_reg, color_sharp_dm_maxmin_gain_by_y_gain_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_1_reg, color_sharp_dm_maxmin_gain_by_y_gain_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_2_reg, color_sharp_dm_maxmin_gain_by_y_gain_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_3_reg, color_sharp_dm_maxmin_gain_by_y_gain_3.regValue);
}

void drvif_color_Set_Sharpness_source_blending_only_weight(unsigned short weight[7])
{
	color_sharp_dm_maxmin_lpf_weit_0_RBUS color_sharp_dm_maxmin_lpf_weit_0;
	color_sharp_dm_maxmin_lpf_weit_1_RBUS color_sharp_dm_maxmin_lpf_weit_1;
	color_sharp_dm_maxmin_lpf_weit_2_RBUS color_sharp_dm_maxmin_lpf_weit_2;
	color_sharp_dm_maxmin_lpf_weit_3_RBUS color_sharp_dm_maxmin_lpf_weit_3;

	color_sharp_dm_maxmin_lpf_weit_0.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_0_reg);
	color_sharp_dm_maxmin_lpf_weit_1.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_1_reg);
	color_sharp_dm_maxmin_lpf_weit_2.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_2_reg);
	color_sharp_dm_maxmin_lpf_weit_3.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_3_reg);

	color_sharp_dm_maxmin_lpf_weit_0.s0 = weight[0];
	color_sharp_dm_maxmin_lpf_weit_0.s1 = weight[1];
	color_sharp_dm_maxmin_lpf_weit_1.s2 = weight[2];
	color_sharp_dm_maxmin_lpf_weit_1.s3 = weight[3];
	color_sharp_dm_maxmin_lpf_weit_2.s4 = weight[4];
	color_sharp_dm_maxmin_lpf_weit_2.s5 = weight[5];
	color_sharp_dm_maxmin_lpf_weit_3.s6 = weight[6];

	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_0_reg, color_sharp_dm_maxmin_lpf_weit_0.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_1_reg, color_sharp_dm_maxmin_lpf_weit_1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_2_reg, color_sharp_dm_maxmin_lpf_weit_2.regValue);
	IoReg_Write32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_3_reg, color_sharp_dm_maxmin_lpf_weit_3.regValue);
}

void drvif_color_Set_Sharpness_source_blending_en(unsigned char en, unsigned char edg_en, unsigned char tex_en)
{
	color_sharp_dm_mkiii_shp_ctrl_RBUS	color_sharp_dm_mkiii_shp_ctrl;
	color_sharp_dm_mkiii_shp_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_MKIII_SHP_CTRL_reg);

	color_sharp_dm_mkiii_shp_ctrl.lpf_weigh_en = en;
	color_sharp_dm_mkiii_shp_ctrl.lpf_weigh_tex_en = tex_en;
	color_sharp_dm_mkiii_shp_ctrl.lpf_weigh_edg_en = edg_en;

	IoReg_Write32(COLOR_SHARP_DM_MKIII_SHP_CTRL_reg, color_sharp_dm_mkiii_shp_ctrl.regValue);
}

void drvif_color_Set_Sharpness_tex_range(unsigned char h_range, unsigned char v_range)
{
	color_sharp_dm_segpk_ctrl2_RBUS	color_sharp_dm_segpk_ctrl2;
	color_sharp_dm_segpk_ctrl2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL2_reg);

	color_sharp_dm_segpk_ctrl2.tex_h_range = h_range;
	color_sharp_dm_segpk_ctrl2.tex_v_range = v_range;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_CTRL2_reg, color_sharp_dm_segpk_ctrl2.regValue);
}

void drvif_color_Set_D_DLTI_Table(VIP_D_DLTI_Table *ptr)
{
	if (!ptr)
		return;
/* set D_DLTI */
	{
	color_sharp_shp_dlti_ctrl_1_RBUS color_sharp_shp_dlti_ctrl_1;
	color_sharp_shp_dlti_ctrl_2_RBUS color_sharp_shp_dlti_ctrl_2;
	color_sharp_shp_dlti_ctrl_3_RBUS color_sharp_shp_dlti_ctrl_3;
	color_sharp_shp_dlti_ovsdu_bias_RBUS color_sharp_shp_dlti_ovsdu_bias;
	color_sharp_shp_dlti_gain_offset_1_RBUS color_sharp_shp_dlti_gain_offset_1;
	// color_sharp_shp_dlti_gain_offset_2_RBUS color_sharp_shp_dlti_gain_offset_2;
	// color_sharp_shp_dlti_gain_offset_3_RBUS color_sharp_shp_dlti_gain_offset_3;
	// color_sharp_shp_dlti_gain_offset_4_RBUS color_sharp_shp_dlti_gain_offset_4;
	// color_sharp_shp_dlti_gain_offset_5_RBUS color_sharp_shp_dlti_gain_offset_5;
	color_sharp_shp_dlti_over_enhance_gain_offset_0_RBUS color_sharp_shp_dlti_over_enhance_gain_offset_0;
	color_sharp_shp_dlti_over_enhance_gain_offset_1_RBUS color_sharp_shp_dlti_over_enhance_gain_offset_1;
	color_sharp_shp_dlti_v_ctrl_1_RBUS color_sharp_shp_dlti_v_ctrl_1;
	color_sharp_shp_dlti_v_ctrl_2_RBUS color_sharp_shp_dlti_v_ctrl_2;
	color_sharp_shp_dlti_v_ovsdu_bias_RBUS color_sharp_shp_dlti_v_ovsdu_bias;
	color_sharp_shp_dlti_v_gain_offset_0_RBUS color_sharp_shp_dlti_v_gain_offset_0;
	// color_sharp_shp_dlti_v_gain_offset_1_RBUS color_sharp_shp_dlti_v_gain_offset_1;
	color_sharp_shp_dlti_over_enhance_v_gain_offset_0_RBUS color_sharp_shp_dlti_over_enhance_v_gain_offset_0;
	color_sharp_shp_dlti_over_enhance_v_gain_offset_1_RBUS color_sharp_shp_dlti_over_enhance_v_gain_offset_1;

	color_sharp_shp_dlti_ctrl_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_CTRL_1_reg);
	color_sharp_shp_dlti_ctrl_2.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_CTRL_2_reg);
	color_sharp_shp_dlti_ctrl_3.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_CTRL_3_reg);
	color_sharp_shp_dlti_ovsdu_bias.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_OVSDU_BIAS_reg);
	color_sharp_shp_dlti_gain_offset_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_1_reg);
	// color_sharp_shp_dlti_gain_offset_2.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_2_reg);
	// color_sharp_shp_dlti_gain_offset_3.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_3_reg);
	// color_sharp_shp_dlti_gain_offset_4.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_4_reg);
	// color_sharp_shp_dlti_gain_offset_5.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_5_reg);
	color_sharp_shp_dlti_over_enhance_gain_offset_0.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_OVER_ENHANCE_GAIN_OFFSET_0_reg);
	color_sharp_shp_dlti_over_enhance_gain_offset_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_OVER_ENHANCE_GAIN_OFFSET_1_reg);
	color_sharp_shp_dlti_v_ctrl_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_V_CTRL_1_reg);
	color_sharp_shp_dlti_v_ctrl_2.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_V_CTRL_2_reg);
	color_sharp_shp_dlti_v_ovsdu_bias.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_V_OVSDU_BIAS_reg);
	color_sharp_shp_dlti_v_gain_offset_0.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_V_GAIN_OFFSET_0_reg);
	// color_sharp_shp_dlti_v_gain_offset_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_V_GAIN_OFFSET_1_reg);
	color_sharp_shp_dlti_over_enhance_v_gain_offset_0.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_OVER_ENHANCE_V_GAIN_OFFSET_0_reg);
	color_sharp_shp_dlti_over_enhance_v_gain_offset_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_OVER_ENHANCE_V_GAIN_OFFSET_1_reg);

	color_sharp_shp_dlti_ctrl_1.dlti_en = ptr->DLTI_Control.Dlti_en;
	color_sharp_shp_dlti_ovsdu_bias.alias_en = ptr->DLTI_Control.alias_en;// 2020

	color_sharp_shp_dlti_v_ctrl_1.dlti_h_en = ptr->DLTI_H_1.dlti_h_en;
	// removed color_sharp_shp_dlti_ctrl_1.ovc_en = ptr->DLTI_H_1.Ovc_en;
	color_sharp_shp_dlti_ctrl_1.data_sel = ptr->DLTI_H_1.Data_sel;
	// removed color_sharp_shp_dlti_ctrl_1.can_shift = ptr->DLTI_H_1.Can_shift;
	// color_sharp_shp_dlti_ctrl_1.pnth = ptr->DLTI_H_1.pnth;
	color_sharp_shp_dlti_ctrl_1.maxlen = ptr->DLTI_H_1.maxlen;
	color_sharp_shp_dlti_ovsdu_bias.bias = ptr->DLTI_H_1.bias;// 2020
	color_sharp_shp_dlti_ctrl_2.ffd1_ratio = ptr->DLTI_H_1.Ffd1_ratio;
	color_sharp_shp_dlti_ovsdu_bias.ovsdu = ptr->DLTI_H_1.ovsdu;// 2020
	// color_sharp_shp_dlti_ctrl_2.ffd0_start = ptr->DLTI_H_1.ffd0_start;
	color_sharp_shp_dlti_ovsdu_bias.bias_white = ptr->DLTI_H_1.bias_white;// 2020

	// removed color_sharp_shp_dlti_ctrl_1.tn_blend_en = ptr->DLTI_H_2.tn_blend_en;
	// removed color_sharp_shp_dlti_ctrl_1.tn_blend_mode = ptr->DLTI_H_2.tn_blend_mode;
	color_sharp_shp_dlti_ctrl_1.d1_cp_shift = ptr->DLTI_H_2.D1_CP_shift;
	color_sharp_shp_dlti_ctrl_1.dcp_th = ptr->DLTI_H_2.DCP_th;
	// removed color_sharp_shp_dlti_ctrl_1.rn_extend_en = ptr->DLTI_H_2.rn_extend_en;
	// color_sharp_shp_dlti_ctrl_1.ffd2_en = ptr->DLTI_H_2.ffd2_en;
	// color_sharp_shp_dlti_ctrl_3.ffd2_range2l = ptr->DLTI_H_2.Ffd2_range2L;
	// color_sharp_shp_dlti_ctrl_3.ffd2_range2r = ptr->DLTI_H_2.Ffd2_range2R;
	// color_sharp_shp_dlti_ctrl_3.ffd2_ratio = ptr->DLTI_H_2.Ffd2_ratio;
	color_sharp_shp_dlti_ctrl_2.flat_score_en = ptr->DLTI_H_2.flat_score_en;
	color_sharp_shp_dlti_ctrl_2.flat_score_thl = ptr->DLTI_H_2.flat_score_thl;
	color_sharp_shp_dlti_ctrl_2.flat_s_range = ptr->DLTI_H_2.flat_s_range;

	color_sharp_shp_dlti_gain_offset_1.tnoff0 = ptr->DLTI_H_Tnoff.Tnoff0;
	// color_sharp_shp_dlti_gain_offset_1.tnoff1 = ptr->DLTI_H_Tnoff.Tnoff1;
	// color_sharp_shp_dlti_gain_offset_1.tnoff2 = ptr->DLTI_H_Tnoff.Tnoff2;
	// color_sharp_shp_dlti_gain_offset_2.tnoff3 = ptr->DLTI_H_Tnoff.Tnoff3;
	// color_sharp_shp_dlti_gain_offset_2.tnoff4 = ptr->DLTI_H_Tnoff.Tnoff4;
	// color_sharp_shp_dlti_gain_offset_2.tnoff5 = ptr->DLTI_H_Tnoff.Tnoff5;
	// color_sharp_shp_dlti_gain_offset_3.tnoff6 = ptr->DLTI_H_Tnoff.Tnoff6;
	// color_sharp_shp_dlti_gain_offset_3.tnoff7 = ptr->DLTI_H_Tnoff.Tnoff7;
	// color_sharp_shp_dlti_gain_offset_3.tnoff8 = ptr->DLTI_H_Tnoff.Tnoff8;
	// color_sharp_shp_dlti_gain_offset_4.tnoff9 = ptr->DLTI_H_Tnoff.Tnoff9;
	// color_sharp_shp_dlti_gain_offset_4.tnoff10 = ptr->DLTI_H_Tnoff.Tnoff10;
	// color_sharp_shp_dlti_gain_offset_4.tnoff11 = ptr->DLTI_H_Tnoff.Tnoff11;
	// color_sharp_shp_dlti_gain_offset_5.tnoff12 = ptr->DLTI_H_Tnoff.Tnoff12;
	// color_sharp_shp_dlti_gain_offset_5.tnoff13 = ptr->DLTI_H_Tnoff.Tnoff13;
	// color_sharp_shp_dlti_gain_offset_5.tnoff14 = ptr->DLTI_H_Tnoff.Tnoff14;

	/* Merlin4 tngain computed by HW
	color_sharp_shp_dlti_gain_offset_1.tngain0 = ptr->DLTI_H_Tngain.Tngain0;
	color_sharp_shp_dlti_gain_offset_1.tngain1 = ptr->DLTI_H_Tngain.Tngain1;
	color_sharp_shp_dlti_gain_offset_1.tngain2 = ptr->DLTI_H_Tngain.Tngain2;
	color_sharp_shp_dlti_gain_offset_2.tngain3 = ptr->DLTI_H_Tngain.Tngain3;
	color_sharp_shp_dlti_gain_offset_2.tngain4 = ptr->DLTI_H_Tngain.Tngain4;
	color_sharp_shp_dlti_gain_offset_2.tngain5 = ptr->DLTI_H_Tngain.Tngain5;
	color_sharp_shp_dlti_gain_offset_3.tngain6 = ptr->DLTI_H_Tngain.Tngain6;
	color_sharp_shp_dlti_gain_offset_3.tngain7 = ptr->DLTI_H_Tngain.Tngain7;
	color_sharp_shp_dlti_gain_offset_3.tngain8 = ptr->DLTI_H_Tngain.Tngain8;
	color_sharp_shp_dlti_gain_offset_4.tngain9 = ptr->DLTI_H_Tngain.Tngain9;
	color_sharp_shp_dlti_gain_offset_4.tngain10 = ptr->DLTI_H_Tngain.Tngain10;
	color_sharp_shp_dlti_gain_offset_4.tngain11 = ptr->DLTI_H_Tngain.Tngain11;
	color_sharp_shp_dlti_gain_offset_5.tngain12 = ptr->DLTI_H_Tngain.Tngain12;
	color_sharp_shp_dlti_gain_offset_5.tngain13 = ptr->DLTI_H_Tngain.Tngain13;
	color_sharp_shp_dlti_gain_offset_5.tngain14 = ptr->DLTI_H_Tngain.Tngain14;
	//*/

	color_sharp_shp_dlti_over_enhance_gain_offset_0.over_enhance = ptr->DLTI_H_OverEnhance.over_enhance;
	color_sharp_shp_dlti_over_enhance_gain_offset_0.gain_pos = ptr->DLTI_H_OverEnhance.gain_pos;
	color_sharp_shp_dlti_over_enhance_gain_offset_1.gain_neg = ptr->DLTI_H_OverEnhance.gain_neg;
	color_sharp_shp_dlti_over_enhance_gain_offset_0.offset_pos = ptr->DLTI_H_OverEnhance.offset_pos;
	color_sharp_shp_dlti_over_enhance_gain_offset_1.offset_neg = ptr->DLTI_H_OverEnhance.offset_neg;

	color_sharp_shp_dlti_v_ctrl_1.dlti_v_en = ptr->DLTI_V_1.dlti_v_en;
	// removed color_sharp_shp_dlti_v_ctrl_1.ovc_v_en = ptr->DLTI_V_1.ovc_v_en;
	color_sharp_shp_dlti_v_ctrl_1.lpf_sel = ptr->DLTI_V_1.lpf_sel;
	// removed color_sharp_shp_dlti_v_ctrl_1.can_shift_v = ptr->DLTI_V_1.can_shift_v;
	// color_sharp_shp_dlti_v_ctrl_1.pnth_v = ptr->DLTI_V_1.pnth_v;
	color_sharp_shp_dlti_v_ctrl_2.maxlen_v = ptr->DLTI_V_1.maxlen_v;
	color_sharp_shp_dlti_v_ovsdu_bias.v_bias = ptr->DLTI_V_1.v_bias;// 2020
	color_sharp_shp_dlti_v_ctrl_2.ffd1_ratio = ptr->DLTI_V_1.ffd1_ratio;
	color_sharp_shp_dlti_v_ovsdu_bias.v_ovsdu = ptr->DLTI_V_1.v_ovsdu;// 2020
	// color_sharp_shp_dlti_v_ctrl_1.v_ffd0_start = ptr->DLTI_V_1.v_ffd0_start;
	color_sharp_shp_dlti_v_ovsdu_bias.v_bias_white = ptr->DLTI_V_1.v_bias_white;// 2020

	color_sharp_shp_dlti_v_gain_offset_0.tnoff0 = ptr->DLTI_V_Tnoff.Tnoff0;
	// color_sharp_shp_dlti_v_gain_offset_0.tnoff1 = ptr->DLTI_V_Tnoff.Tnoff1;
	// color_sharp_shp_dlti_v_gain_offset_0.tnoff2 = ptr->DLTI_V_Tnoff.Tnoff2;
	// color_sharp_shp_dlti_v_gain_offset_1.tnoff3 = ptr->DLTI_V_Tnoff.Tnoff3;
	// color_sharp_shp_dlti_v_gain_offset_1.tnoff4 = ptr->DLTI_V_Tnoff.Tnoff4;
	// color_sharp_shp_dlti_v_gain_offset_1.tnoff5 = ptr->DLTI_V_Tnoff.Tnoff5;
	// color_sharp_shp_dlti_v_gain_offset_1.tnoff6 = ptr->DLTI_V_Tnoff.Tnoff6;

	/* Merlin4 tngain computed by HW
	color_sharp_shp_dlti_v_gain_offset_0.tngain0 = ptr->DLTI_V_Tngain.Tngain0;
	color_sharp_shp_dlti_v_gain_offset_0.tngain1 = ptr->DLTI_V_Tngain.Tngain1;
	color_sharp_shp_dlti_v_gain_offset_0.tngain2 = ptr->DLTI_V_Tngain.Tngain2;
	color_sharp_shp_dlti_v_gain_offset_0.tngain3 = ptr->DLTI_V_Tngain.Tngain3;
	color_sharp_shp_dlti_v_gain_offset_1.tngain4 = ptr->DLTI_V_Tngain.Tngain4;
	color_sharp_shp_dlti_v_gain_offset_1.tngain5 = ptr->DLTI_V_Tngain.Tngain5;
	color_sharp_shp_dlti_v_gain_offset_1.tngain6 = ptr->DLTI_V_Tngain.Tngain6;
	//*/

	color_sharp_shp_dlti_v_ctrl_1.over_enhance_v = ptr->DLTI_V_OverEnhance.over_enhance_v;
	color_sharp_shp_dlti_over_enhance_v_gain_offset_0.gain_pos = ptr->DLTI_V_OverEnhance.gain_pos;
	color_sharp_shp_dlti_over_enhance_v_gain_offset_1.gain_neg = ptr->DLTI_V_OverEnhance.gain_neg;
	color_sharp_shp_dlti_over_enhance_v_gain_offset_0.offset_pos = ptr->DLTI_V_OverEnhance.offset_pos;
	color_sharp_shp_dlti_over_enhance_v_gain_offset_1.offset_neg = ptr->DLTI_V_OverEnhance.offset_neg;

	IoReg_Write32(COLOR_SHARP_SHP_DLTI_CTRL_1_reg, color_sharp_shp_dlti_ctrl_1.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_CTRL_2_reg, color_sharp_shp_dlti_ctrl_2.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_CTRL_3_reg, color_sharp_shp_dlti_ctrl_3.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_OVSDU_BIAS_reg, color_sharp_shp_dlti_ovsdu_bias.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_1_reg, color_sharp_shp_dlti_gain_offset_1.regValue);
	// IoReg_Write32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_2_reg, color_sharp_shp_dlti_gain_offset_2.regValue);
	// IoReg_Write32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_3_reg, color_sharp_shp_dlti_gain_offset_3.regValue);
	// IoReg_Write32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_4_reg, color_sharp_shp_dlti_gain_offset_4.regValue);
	// IoReg_Write32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_5_reg, color_sharp_shp_dlti_gain_offset_5.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_OVER_ENHANCE_GAIN_OFFSET_0_reg, color_sharp_shp_dlti_over_enhance_gain_offset_0.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_OVER_ENHANCE_GAIN_OFFSET_1_reg, color_sharp_shp_dlti_over_enhance_gain_offset_1.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_V_CTRL_1_reg, color_sharp_shp_dlti_v_ctrl_1.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_V_CTRL_2_reg, color_sharp_shp_dlti_v_ctrl_2.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_V_OVSDU_BIAS_reg, color_sharp_shp_dlti_v_ovsdu_bias.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_V_GAIN_OFFSET_0_reg, color_sharp_shp_dlti_v_gain_offset_0.regValue);
	// IoReg_Write32(COLOR_SHARP_SHP_DLTI_V_GAIN_OFFSET_1_reg, color_sharp_shp_dlti_v_gain_offset_1.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_OVER_ENHANCE_V_GAIN_OFFSET_0_reg, color_sharp_shp_dlti_over_enhance_v_gain_offset_0.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_OVER_ENHANCE_V_GAIN_OFFSET_1_reg, color_sharp_shp_dlti_over_enhance_v_gain_offset_1.regValue);
	}
/* set D_DLTI end*/
}
void drvif_color_Get_D_DLTI_Table(VIP_D_DLTI_Table *ptr)
{
        if (!ptr)
                return;
/* set D_DLTI */
        {
        color_sharp_shp_dlti_ctrl_1_RBUS color_sharp_shp_dlti_ctrl_1;
        color_sharp_shp_dlti_ctrl_2_RBUS color_sharp_shp_dlti_ctrl_2;
        color_sharp_shp_dlti_ctrl_3_RBUS color_sharp_shp_dlti_ctrl_3;
		color_sharp_shp_dlti_ovsdu_bias_RBUS color_sharp_shp_dlti_ovsdu_bias;// 2020
        color_sharp_shp_dlti_gain_offset_1_RBUS color_sharp_shp_dlti_gain_offset_1;
        // color_sharp_shp_dlti_gain_offset_2_RBUS color_sharp_shp_dlti_gain_offset_2;
        // color_sharp_shp_dlti_gain_offset_3_RBUS color_sharp_shp_dlti_gain_offset_3;
        // color_sharp_shp_dlti_gain_offset_4_RBUS color_sharp_shp_dlti_gain_offset_4;
        // color_sharp_shp_dlti_gain_offset_5_RBUS color_sharp_shp_dlti_gain_offset_5;
        color_sharp_shp_dlti_over_enhance_gain_offset_0_RBUS color_sharp_shp_dlti_over_enhance_gain_offset_0;
        color_sharp_shp_dlti_over_enhance_gain_offset_1_RBUS color_sharp_shp_dlti_over_enhance_gain_offset_1;
        color_sharp_shp_dlti_v_ctrl_1_RBUS color_sharp_shp_dlti_v_ctrl_1;
        color_sharp_shp_dlti_v_ctrl_2_RBUS color_sharp_shp_dlti_v_ctrl_2;
		color_sharp_shp_dlti_v_ovsdu_bias_RBUS color_sharp_shp_dlti_v_ovsdu_bias;
        color_sharp_shp_dlti_v_gain_offset_0_RBUS color_sharp_shp_dlti_v_gain_offset_0;
        // color_sharp_shp_dlti_v_gain_offset_1_RBUS color_sharp_shp_dlti_v_gain_offset_1;
        color_sharp_shp_dlti_over_enhance_v_gain_offset_0_RBUS color_sharp_shp_dlti_over_enhance_v_gain_offset_0;
        color_sharp_shp_dlti_over_enhance_v_gain_offset_1_RBUS color_sharp_shp_dlti_over_enhance_v_gain_offset_1;

        color_sharp_shp_dlti_ctrl_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_CTRL_1_reg);
        color_sharp_shp_dlti_ctrl_2.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_CTRL_2_reg);
        color_sharp_shp_dlti_ctrl_3.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_CTRL_3_reg);
		color_sharp_shp_dlti_ovsdu_bias.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_OVSDU_BIAS_reg);
        color_sharp_shp_dlti_gain_offset_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_1_reg);
        // color_sharp_shp_dlti_gain_offset_2.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_2_reg);
        // color_sharp_shp_dlti_gain_offset_3.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_3_reg);
        // color_sharp_shp_dlti_gain_offset_4.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_4_reg);
        // color_sharp_shp_dlti_gain_offset_5.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_5_reg);
        color_sharp_shp_dlti_over_enhance_gain_offset_0.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_OVER_ENHANCE_GAIN_OFFSET_0_reg);
        color_sharp_shp_dlti_over_enhance_gain_offset_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_OVER_ENHANCE_GAIN_OFFSET_1_reg);
        color_sharp_shp_dlti_v_ctrl_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_V_CTRL_1_reg);
        color_sharp_shp_dlti_v_ctrl_2.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_V_CTRL_2_reg);
		color_sharp_shp_dlti_v_ovsdu_bias.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_V_OVSDU_BIAS_reg);
        color_sharp_shp_dlti_v_gain_offset_0.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_V_GAIN_OFFSET_0_reg);
        // color_sharp_shp_dlti_v_gain_offset_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_V_GAIN_OFFSET_1_reg);
        color_sharp_shp_dlti_over_enhance_v_gain_offset_0.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_OVER_ENHANCE_V_GAIN_OFFSET_0_reg);
        color_sharp_shp_dlti_over_enhance_v_gain_offset_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_OVER_ENHANCE_V_GAIN_OFFSET_1_reg);

        ptr->DLTI_Control.Dlti_en       = color_sharp_shp_dlti_ctrl_1.dlti_en;
        ptr->DLTI_Control.alias_en      = color_sharp_shp_dlti_ovsdu_bias.alias_en;

        ptr->DLTI_H_1.dlti_h_en         = color_sharp_shp_dlti_v_ctrl_1.dlti_h_en;
        ptr->DLTI_H_1.Data_sel          = color_sharp_shp_dlti_ctrl_1.data_sel;
        ptr->DLTI_H_1.Can_shift         = 0;// removed color_sharp_shp_dlti_ctrl_1.can_shift;
       //  ptr->DLTI_H_1.pnth              = color_sharp_shp_dlti_ctrl_1.pnth;
        ptr->DLTI_H_1.maxlen            = color_sharp_shp_dlti_ctrl_1.maxlen;
        ptr->DLTI_H_1.bias              = color_sharp_shp_dlti_ovsdu_bias.bias;
        ptr->DLTI_H_1.Ffd1_ratio        = color_sharp_shp_dlti_ctrl_2.ffd1_ratio;
        ptr->DLTI_H_1.ovsdu             = color_sharp_shp_dlti_ovsdu_bias.ovsdu;
        // ptr->DLTI_H_1.ffd0_start        = color_sharp_shp_dlti_ctrl_2.ffd0_start;
        ptr->DLTI_H_1.bias_white        = color_sharp_shp_dlti_ovsdu_bias.bias_white;

        ptr->DLTI_H_2.D1_CP_shift       = color_sharp_shp_dlti_ctrl_1.d1_cp_shift;
        ptr->DLTI_H_2.DCP_th            = color_sharp_shp_dlti_ctrl_1.dcp_th;
        // ptr->DLTI_H_2.ffd2_en           = color_sharp_shp_dlti_ctrl_1.ffd2_en;
        // ptr->DLTI_H_2.Ffd2_range2L      = color_sharp_shp_dlti_ctrl_3.ffd2_range2l;
        // ptr->DLTI_H_2.Ffd2_range2R      = color_sharp_shp_dlti_ctrl_3.ffd2_range2r;
        // ptr->DLTI_H_2.Ffd2_ratio        = color_sharp_shp_dlti_ctrl_3.ffd2_ratio;
        ptr->DLTI_H_2.flat_score_en     = color_sharp_shp_dlti_ctrl_2.flat_score_en;
        ptr->DLTI_H_2.flat_score_thl    = color_sharp_shp_dlti_ctrl_2.flat_score_thl;
        ptr->DLTI_H_2.flat_s_range      = color_sharp_shp_dlti_ctrl_2.flat_s_range;

        ptr->DLTI_H_Tnoff.Tnoff0        = color_sharp_shp_dlti_gain_offset_1.tnoff0;
        // ptr->DLTI_H_Tnoff.Tnoff1        = color_sharp_shp_dlti_gain_offset_1.tnoff1;
        // ptr->DLTI_H_Tnoff.Tnoff2        = color_sharp_shp_dlti_gain_offset_1.tnoff2;
        // ptr->DLTI_H_Tnoff.Tnoff3        = color_sharp_shp_dlti_gain_offset_2.tnoff3;
        // ptr->DLTI_H_Tnoff.Tnoff4        = color_sharp_shp_dlti_gain_offset_2.tnoff4;
        // ptr->DLTI_H_Tnoff.Tnoff5        = color_sharp_shp_dlti_gain_offset_2.tnoff5;
        // ptr->DLTI_H_Tnoff.Tnoff6        = color_sharp_shp_dlti_gain_offset_3.tnoff6;
        // ptr->DLTI_H_Tnoff.Tnoff7        = color_sharp_shp_dlti_gain_offset_3.tnoff7;
        // ptr->DLTI_H_Tnoff.Tnoff8        = color_sharp_shp_dlti_gain_offset_3.tnoff8;
        // ptr->DLTI_H_Tnoff.Tnoff9        = color_sharp_shp_dlti_gain_offset_4.tnoff9;
        // ptr->DLTI_H_Tnoff.Tnoff10       = color_sharp_shp_dlti_gain_offset_4.tnoff10;
        // ptr->DLTI_H_Tnoff.Tnoff11       = color_sharp_shp_dlti_gain_offset_4.tnoff11;
        // ptr->DLTI_H_Tnoff.Tnoff12       = color_sharp_shp_dlti_gain_offset_5.tnoff12;
        // ptr->DLTI_H_Tnoff.Tnoff13       = color_sharp_shp_dlti_gain_offset_5.tnoff13;
        // ptr->DLTI_H_Tnoff.Tnoff14       = color_sharp_shp_dlti_gain_offset_5.tnoff14;
        ptr->DLTI_H_OverEnhance.over_enhance    = color_sharp_shp_dlti_over_enhance_gain_offset_0.over_enhance;
        ptr->DLTI_H_OverEnhance.gain_pos        = color_sharp_shp_dlti_over_enhance_gain_offset_0.gain_pos;
        ptr->DLTI_H_OverEnhance.gain_neg        = color_sharp_shp_dlti_over_enhance_gain_offset_1.gain_neg;
        ptr->DLTI_H_OverEnhance.offset_pos      = color_sharp_shp_dlti_over_enhance_gain_offset_0.offset_pos;
        ptr->DLTI_H_OverEnhance.offset_neg      = color_sharp_shp_dlti_over_enhance_gain_offset_1.offset_neg;

        ptr->DLTI_V_1.dlti_v_en                 = color_sharp_shp_dlti_v_ctrl_1.dlti_v_en;
        ptr->DLTI_V_1.lpf_sel                   = color_sharp_shp_dlti_v_ctrl_1.lpf_sel;
        ptr->DLTI_V_1.can_shift_v               = 0;// removed color_sharp_shp_dlti_v_ctrl_1.can_shift_v;
        // ptr->DLTI_V_1.pnth_v                    = color_sharp_shp_dlti_v_ctrl_1.pnth_v;
        ptr->DLTI_V_1.maxlen_v                  = color_sharp_shp_dlti_v_ctrl_2.maxlen_v;
        ptr->DLTI_V_1.v_bias                    = color_sharp_shp_dlti_v_ovsdu_bias.v_bias;
        ptr->DLTI_V_1.ffd1_ratio                = color_sharp_shp_dlti_v_ctrl_2.ffd1_ratio;
        ptr->DLTI_V_1.v_ovsdu                   = color_sharp_shp_dlti_v_ovsdu_bias.v_ovsdu;
        // ptr->DLTI_V_1.v_ffd0_start              = color_sharp_shp_dlti_v_ctrl_1.v_ffd0_start;
        ptr->DLTI_V_1.v_bias_white              = color_sharp_shp_dlti_v_ovsdu_bias.v_bias_white;

        ptr->DLTI_V_Tnoff.Tnoff0                = color_sharp_shp_dlti_v_gain_offset_0.tnoff0;
        // ptr->DLTI_V_Tnoff.Tnoff1                = color_sharp_shp_dlti_v_gain_offset_0.tnoff1;
        // ptr->DLTI_V_Tnoff.Tnoff2                = color_sharp_shp_dlti_v_gain_offset_0.tnoff2;
        // ptr->DLTI_V_Tnoff.Tnoff3                = color_sharp_shp_dlti_v_gain_offset_1.tnoff3;
        // ptr->DLTI_V_Tnoff.Tnoff4                = color_sharp_shp_dlti_v_gain_offset_1.tnoff4;
        // ptr->DLTI_V_Tnoff.Tnoff5                = color_sharp_shp_dlti_v_gain_offset_1.tnoff5;
        // ptr->DLTI_V_Tnoff.Tnoff6                = color_sharp_shp_dlti_v_gain_offset_1.tnoff6;

        ptr->DLTI_V_OverEnhance.over_enhance_v  = color_sharp_shp_dlti_v_ctrl_1.over_enhance_v;
        ptr->DLTI_V_OverEnhance.gain_pos        = color_sharp_shp_dlti_over_enhance_v_gain_offset_0.gain_pos;
        ptr->DLTI_V_OverEnhance.gain_neg        = color_sharp_shp_dlti_over_enhance_v_gain_offset_1.gain_neg;
        ptr->DLTI_V_OverEnhance.offset_pos      = color_sharp_shp_dlti_over_enhance_v_gain_offset_0.offset_pos;
        ptr->DLTI_V_OverEnhance.offset_neg      = color_sharp_shp_dlti_over_enhance_v_gain_offset_1.offset_neg;


        }
/* set D_DLTI end*/
}

void drvif_color_Set_D_DLTI_H_Tnoff_gain(unsigned char tnoff)
{
	color_sharp_shp_dlti_gain_offset_1_RBUS color_sharp_shp_dlti_gain_offset_1;

	color_sharp_shp_dlti_gain_offset_1.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_1_reg);

	color_sharp_shp_dlti_gain_offset_1.tnoff0 = tnoff;

	IoReg_Write32(COLOR_SHARP_SHP_DLTI_GAIN_OFFSET_1_reg, color_sharp_shp_dlti_gain_offset_1.regValue);
}

void drvif_color_Set_D_DLTI_V_Tnoff_gain(unsigned char tnoff)
{
	color_sharp_shp_dlti_v_gain_offset_0_RBUS color_sharp_shp_dlti_v_gain_offset_0;

	color_sharp_shp_dlti_v_gain_offset_0.regValue = IoReg_Read32(COLOR_SHARP_SHP_DLTI_V_GAIN_OFFSET_0_reg);

	color_sharp_shp_dlti_v_gain_offset_0.tnoff0 = tnoff;
	IoReg_Write32(COLOR_SHARP_SHP_DLTI_V_GAIN_OFFSET_0_reg, color_sharp_shp_dlti_v_gain_offset_0.regValue);
}

void drvif_color_Set_Sharpness_Other_Table(DRV_Sharpness_Other_Ctrl *ptr)
{
#if 0
	color_sharp_dm_segpk_ctrl1_RBUS color_sharp_dm_segpk_ctrl1;
	color_sharp_dm_segpk_vpk4_RBUS color_sharp_dm_segpk_vpk4;

	color_sharp_dm_nr_shp_ctrl_RBUS color_sharp_dm_nr_shp_ctrl_reg;
	color_sharp_dm_nr_shp_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_NR_SHP_CTRL_reg);
	color_sharp_dm_nr_shp_ctrl_reg.db_en=1;
	color_sharp_dm_nr_shp_ctrl_reg.db_load=0;
	IoReg_Write32(COLOR_SHARP_DM_NR_SHP_CTRL_reg,color_sharp_dm_nr_shp_ctrl_reg.regValue);



	if (!ptr)
		return;

	color_sharp_dm_segpk_ctrl1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL1_reg);
	color_sharp_dm_segpk_vpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK4_reg);

	color_sharp_dm_segpk_ctrl1.segpk_mode = ptr->segpk_mode;
	color_sharp_dm_segpk_ctrl1.edgpk_en = ptr->edgpk_en;
	color_sharp_dm_segpk_ctrl1.texpk_en = ptr->texpk_en;
	// removed color_sharp_dm_segpk_ctrl1.enhance_mode = ptr->enhance_mode;
	color_sharp_dm_segpk_vpk4.vpk_en = ptr->vpk_en;

	IoReg_Write32(COLOR_SHARP_DM_SEGPK_CTRL1_reg, color_sharp_dm_segpk_ctrl1.regValue);
	IoReg_Write32(COLOR_SHARP_DM_SEGPK_VPK4_reg, color_sharp_dm_segpk_vpk4.regValue);

	color_sharp_dm_nr_shp_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_NR_SHP_CTRL_reg);
	color_sharp_dm_nr_shp_ctrl_reg.db_en=1;
	color_sharp_dm_nr_shp_ctrl_reg.db_load=1;
	IoReg_Write32(COLOR_SHARP_DM_NR_SHP_CTRL_reg,color_sharp_dm_nr_shp_ctrl_reg.regValue);
#endif
}
void drvif_color_Get_Sharpness_Other_Table(DRV_Sharpness_Other_Ctrl *ptr)
{

	color_sharp_dm_segpk_ctrl1_RBUS color_sharp_dm_segpk_ctrl1;
	color_sharp_dm_segpk_vpk4_RBUS color_sharp_dm_segpk_vpk4;

	//color_sharp_dm_nr_shp_ctrl_RBUS color_sharp_dm_nr_shp_ctrl_reg;

	if (!ptr)
		return;

	color_sharp_dm_segpk_ctrl1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL1_reg);
	color_sharp_dm_segpk_vpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK4_reg);

	ptr->segpk_mode = color_sharp_dm_segpk_ctrl1.segpk_mode;
	ptr->edgpk_en = color_sharp_dm_segpk_ctrl1.edgpk_en;
	ptr->texpk_en = color_sharp_dm_segpk_ctrl1.texpk_en;
	ptr->vpk_en = color_sharp_dm_segpk_vpk4.vpk_en;
	ptr->edg_lpf_shift = 0;// removed
	ptr->tex_lpf_shift = 0;// removed

// removed color_sharp_dm_segpk_ctrl1.enhance_mode = ptr->enhance_mode;
}

void drvif_color_Set_MBPK_Table(VIP_MBPK_Table *ptr)
{

	color_mb_peaking_mb_peaking_ctrl_RBUS				color_mb_peaking_mb_peaking_ctrl;
	color_mb_peaking_mb_peaking_hor_filter_0_RBUS		color_mb_peaking_mb_peaking_hor_filter_0;
	color_mb_peaking_mb_peaking_hor_filter_1_RBUS		color_mb_peaking_mb_peaking_hor_filter_1;
	// removed color_mb_peaking_mb_peaking_hor_filter_2_RBUS		color_mb_peaking_mb_peaking_hor_filter_2;
	color_mb_peaking_mb_peaking_hor_gain_RBUS			color_mb_peaking_mb_peaking_hor_gain;
	color_mb_peaking_mb_peaking_hor_bound_RBUS		color_mb_peaking_mb_peaking_hor_bound;
	color_mb_peaking_mb_peaking_hor_gain_2_RBUS		color_mb_peaking_mb_peaking_hor_gain_2;
	color_mb_peaking_mb_peaking_ver_filter_RBUS		color_mb_peaking_mb_peaking_ver_filter;
	color_mb_peaking_mb_peaking_ver_gain_RBUS			color_mb_peaking_mb_peaking_ver_gain;
	color_mb_peaking_mb_peaking_ver_bound_RBUS		color_mb_peaking_mb_peaking_ver_bound;
	color_mb_peaking_mb_peaking_ver_gain_2_RBUS		color_mb_peaking_mb_peaking_ver_gain_2;
	color_mb_peaking_mb_double_buffer_ctrl_RBUS		color_mb_peaking_mb_double_buffer_ctrl;
	/*color_mb_peaking_mb_dummy_RBUS					color_mb_peaking_mb_dummy;*/
	color_mb_peaking_mb_de_over_hor_range_gain_RBUS color_mb_peaking_mb_de_over_hor_range_gain;
	color_mb_peaking_mb_de_over_ver_range_gain_RBUS color_mb_peaking_mb_de_over_ver_range_gain;
	color_mb_peaking_mb_snr_coring_RBUS color_mb_peaking_mb_snr_coring;
	color_mb_peaking_mb_wei_coring1_RBUS color_mb_peaking_mb_wei_coring1;
	color_mb_peaking_mb_wei_coring2_RBUS color_mb_peaking_mb_wei_coring2;
	color_mb_peaking_mb_peaking_de_over_h_s_0_RBUS color_mb_peaking_mb_peaking_de_over_h_s_0;
	color_mb_peaking_mb_peaking_de_over_h_s_1_RBUS color_mb_peaking_mb_peaking_de_over_h_s_1;
	color_mb_peaking_mb_peaking_de_over_h_s_2_RBUS color_mb_peaking_mb_peaking_de_over_h_s_2;
	color_mb_peaking_mb_peaking_de_over_h_b_0_RBUS color_mb_peaking_mb_peaking_de_over_h_b_0;
	color_mb_peaking_mb_peaking_de_over_h_b_1_RBUS color_mb_peaking_mb_peaking_de_over_h_b_1;
	color_mb_peaking_mb_peaking_de_over_h_b_2_RBUS color_mb_peaking_mb_peaking_de_over_h_b_2;
	color_mb_peaking_mb_peaking_de_over_v_s_0_RBUS color_mb_peaking_mb_peaking_de_over_v_s_0;
	color_mb_peaking_mb_peaking_de_over_v_s_1_RBUS color_mb_peaking_mb_peaking_de_over_v_s_1;
	color_mb_peaking_mb_peaking_de_over_v_s_2_RBUS color_mb_peaking_mb_peaking_de_over_v_s_2;
	color_mb_peaking_mb_peaking_de_over_v_b_0_RBUS color_mb_peaking_mb_peaking_de_over_v_b_0;
	color_mb_peaking_mb_peaking_de_over_v_b_1_RBUS color_mb_peaking_mb_peaking_de_over_v_b_1;
	color_mb_peaking_mb_peaking_de_over_v_b_2_RBUS color_mb_peaking_mb_peaking_de_over_v_b_2;
	color_mb_peaking_mb_peaking_emf_ctrl_RBUS color_mb_peaking_mb_peaking_emf_ctrl;
	color_mb_peaking_mb_peaking_emf_seg_gain_RBUS color_mb_peaking_mb_peaking_emf_seg_gain;
	color_mb_peaking_mb_peaking_emf_seg_offset_RBUS color_mb_peaking_mb_peaking_emf_seg_offset;

	if (!ptr)
		return;
	color_mb_peaking_mb_peaking_ctrl.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg);
	color_mb_peaking_mb_peaking_hor_filter_0.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Hor_FILTER_0_reg);
	color_mb_peaking_mb_peaking_hor_filter_1.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Hor_FILTER_1_reg);
	color_mb_peaking_mb_peaking_hor_gain.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Hor_GAIN_reg);
	color_mb_peaking_mb_peaking_hor_bound.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Hor_BOUND_reg);
	color_mb_peaking_mb_peaking_hor_gain_2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Hor_GAIN_2_reg);
	color_mb_peaking_mb_peaking_ver_filter.regValue =IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Ver_FILTER_reg);
	color_mb_peaking_mb_peaking_ver_gain.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Ver_GAIN_reg);
	color_mb_peaking_mb_peaking_ver_bound.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Ver_BOUND_reg);
	color_mb_peaking_mb_peaking_ver_gain_2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Ver_GAIN_2_reg);
	color_mb_peaking_mb_de_over_hor_range_gain.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_De_Over_Hor_RANGE_GAIN_reg);
	color_mb_peaking_mb_de_over_ver_range_gain.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_De_Over_Ver_RANGE_GAIN_reg);
	color_mb_peaking_mb_snr_coring.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_SNR_CORING_reg);
	color_mb_peaking_mb_wei_coring1.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_WEI_CORING1_reg);
	color_mb_peaking_mb_wei_coring2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_WEI_CORING2_reg);
	color_mb_peaking_mb_peaking_de_over_h_s_0.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_S_0_reg);
	color_mb_peaking_mb_peaking_de_over_h_s_1.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_S_1_reg);
	color_mb_peaking_mb_peaking_de_over_h_s_2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_S_2_reg);
	color_mb_peaking_mb_peaking_de_over_h_b_0.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_B_0_reg);
	color_mb_peaking_mb_peaking_de_over_h_b_1.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_B_1_reg);
	color_mb_peaking_mb_peaking_de_over_h_b_2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_B_2_reg);
	color_mb_peaking_mb_peaking_de_over_v_s_0.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_S_0_reg);
	color_mb_peaking_mb_peaking_de_over_v_s_1.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_S_1_reg);
	color_mb_peaking_mb_peaking_de_over_v_s_2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_S_2_reg);
	color_mb_peaking_mb_peaking_de_over_v_b_0.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_B_0_reg);
	color_mb_peaking_mb_peaking_de_over_v_b_1.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_B_1_reg);
	color_mb_peaking_mb_peaking_de_over_v_b_2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_B_2_reg);
	color_mb_peaking_mb_peaking_emf_ctrl.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_EMF_Ctrl_reg);
	color_mb_peaking_mb_peaking_emf_seg_gain.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_EMF_seg_gain_reg);
	color_mb_peaking_mb_peaking_emf_seg_offset.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_EMF_seg_offset_reg);

	//Enable doublue buffer
	color_mb_peaking_mb_double_buffer_ctrl.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg);
	color_mb_peaking_mb_double_buffer_ctrl.mb_db_en =1;
	IoReg_Write32(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, color_mb_peaking_mb_double_buffer_ctrl.regValue);

	//ROW0 VIP_MBPK_Ctrl
	color_mb_peaking_mb_peaking_ctrl.mb_v_en = ptr->MBPK_Ctrl.MB_V_En;
	color_mb_peaking_mb_peaking_ctrl.mb_h_en = ptr->MBPK_Ctrl.MB_H_En;
	color_mb_peaking_mb_peaking_ctrl.mb_ver_vfilter_data = ptr->MBPK_Ctrl.MB_Ver_Vfilter_data;
	color_mb_peaking_mb_peaking_ctrl.mb_peaking_en = ptr->MBPK_Ctrl.mb_peaking_en;
	color_mb_peaking_mb_peaking_ctrl.mb_snr_flag_en = ptr->MBPK_Ctrl.MB_SNR_flag_en;
	color_mb_peaking_mb_snr_coring.mb_mos_coring = ptr->MBPK_Ctrl.MB_MOS_coring;
	// removed color_mb_peaking_mb_snr_coring.mb_wei_coring = ptr->MBPK_Ctrl.MB_WEI_coring;
	color_mb_peaking_mb_peaking_ctrl.mb_de_over_hvmin_en= ptr->MBPK_Ctrl.MB_De_Over_HVmin_en;
	color_mb_peaking_mb_peaking_ctrl.mb_de_over_bsmaxmin= ptr->MBPK_Ctrl.MB_De_Over_BSmaxmin;

	//ROW1 VIP_MBPK_H_Coef
	/*DC Value Check*/
	if (SHARPNESS_LOG) {
		int sum;

		/*MB_Hor_Filter*/
		sum = 0;
		sum = ptr->MBPK_H_Coef.MB_Hor_Filter_C0 +
		      2*(
		      	ptr->MBPK_H_Coef.MB_Hor_Filter_C1 + ptr->MBPK_H_Coef.MB_Hor_Filter_C2 + ptr->MBPK_H_Coef.MB_Hor_Filter_C3
		      	// removed + ptr->MBPK_H_Coef.MB_Hor_Filter_C4 + ptr->MBPK_H_Coef.MB_Hor_Filter_C5 + ptr->MBPK_H_Coef.MB_Hor_Filter_C6
		      );
		if (sum != 0)
			VIPprintf("[Warnning] [%s-%s-%d] [MB_Hor_Filter Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);
	}

	color_mb_peaking_mb_peaking_hor_filter_0.mb_hor_filter_c0 = ptr->MBPK_H_Coef.MB_Hor_Filter_C0;
	color_mb_peaking_mb_peaking_hor_filter_0.mb_hor_filter_c1 = ptr->MBPK_H_Coef.MB_Hor_Filter_C1;
	color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c2 = ptr->MBPK_H_Coef.MB_Hor_Filter_C2;
	color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c3 = ptr->MBPK_H_Coef.MB_Hor_Filter_C3;
	// removed color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c4 = ptr->MBPK_H_Coef.MB_Hor_Filter_C4;
	// removed color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c5 = ptr->MBPK_H_Coef.MB_Hor_Filter_C5;
	// removed color_mb_peaking_mb_peaking_hor_filter_2.mb_hor_filter_c6 = ptr->MBPK_H_Coef.MB_Hor_Filter_C6;

	//ROW2 VIP_MBPK_H_Table
	color_mb_peaking_mb_peaking_hor_gain.mb_gain_neg = ptr->MBPK_H_Table.MB_Gain_Neg;
	color_mb_peaking_mb_peaking_hor_gain.mb_gain_pos = ptr->MBPK_H_Table.MB_Gain_Pos;
	color_mb_peaking_mb_peaking_hor_bound.mb_hv_neg = ptr->MBPK_H_Table.MB_HV_Neg;
	color_mb_peaking_mb_peaking_hor_bound.mb_hv_pos = ptr->MBPK_H_Table.MB_HV_Pos;
	color_mb_peaking_mb_peaking_hor_gain_2.mb_d2_shift_bit = ptr->MBPK_H_Table.MB_D2_shift_bit;
	color_mb_peaking_mb_peaking_hor_gain_2.mb_gain_neg2 = ptr->MBPK_H_Table.MB_Gain_Neg2;
	color_mb_peaking_mb_peaking_hor_gain_2.mb_gain_pos2 = ptr->MBPK_H_Table.MB_Gain_Pos2;
	color_mb_peaking_mb_peaking_hor_gain_2.mb_lv = ptr->MBPK_H_Table.MB_LV;
	color_mb_peaking_mb_peaking_hor_gain_2.mb_lv2 = ptr->MBPK_H_Table.MB_LV2;

	//ROW3 VIP_MBPK_V_Coef
	/*DC Value Check*/
	if (SHARPNESS_LOG) {
		int sum;

		/*MB_Ver_Filter*/
		sum = 0;
		sum = ptr->MBPK_V_Coef.MB_Ver_Filter_C0 +
		      2*(ptr->MBPK_V_Coef.MB_Ver_Filter_C1 + ptr->MBPK_V_Coef.MB_Ver_Filter_C2);
		if (sum != 0)
			VIPprintf("[Warnning] [%s-%s-%d] [MB_Ver_Filter Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);
	}

	color_mb_peaking_mb_peaking_ver_filter.mb_ver_filter_c0 = ptr->MBPK_V_Coef.MB_Ver_Filter_C0;
	color_mb_peaking_mb_peaking_ver_filter.mb_ver_filter_c1 = ptr->MBPK_V_Coef.MB_Ver_Filter_C1;
	color_mb_peaking_mb_peaking_ver_filter.mb_ver_filter_c2 = ptr->MBPK_V_Coef.MB_Ver_Filter_C2;

	//ROW4 VIP_MBPK_V_Table
	color_mb_peaking_mb_peaking_ver_gain.mb_gain_neg_v = ptr->MBPK_V_Table.MB_Gain_Neg_V;
	color_mb_peaking_mb_peaking_ver_gain.mb_gain_pos_v = ptr->MBPK_V_Table.MB_Gain_Pos_V;
	color_mb_peaking_mb_peaking_ver_bound.mb_hv_neg_v = ptr->MBPK_V_Table.MB_HV_Neg_V;
	color_mb_peaking_mb_peaking_ver_bound.mb_hv_pos_v = ptr->MBPK_V_Table.MB_HV_Pos_V;
	color_mb_peaking_mb_peaking_ver_gain_2.mb_d2_shift_bit_v = ptr->MBPK_V_Table.MB_D2_shift_bit_V;
	color_mb_peaking_mb_peaking_ver_gain_2.mb_gain_neg2_v = ptr->MBPK_V_Table.MB_Gain_Neg2_V;
	color_mb_peaking_mb_peaking_ver_gain_2.mb_gain_pos2_v = ptr->MBPK_V_Table.MB_Gain_Pos2_V;
	color_mb_peaking_mb_peaking_ver_gain_2.mb_lv2_v = ptr->MBPK_V_Table.MB_LV2_V;
	color_mb_peaking_mb_peaking_ver_gain_2.mb_lv_v = ptr->MBPK_V_Table.MB_LV_V;
	/*color_mb_peaking_mb_double_buffer_ctrl.;*/
	/*color_mb_peaking_mb_double_buffer_ctrl.;*/
	/*color_mb_peaking_mb_double_buffer_ctrl.;*/
	/*color_mb_peaking_mb_dummy;*/

	//ROW5 VIP_MBPK_H_Deovershoot0
	color_mb_peaking_mb_peaking_ctrl.mb_de_over_h_en = ptr->MBPK_H_Deovershoot0.MB_De_Over_H_en;
	// removed color_mb_peaking_mb_de_over_hor_range_gain.mb_de_over_h_s_gain = ptr->MBPK_H_Deovershoot0.MB_De_Over_H_S_gain;
	color_mb_peaking_mb_de_over_hor_range_gain.mb_de_over_h_b_range = ptr->MBPK_H_Deovershoot0.MB_De_Over_H_B_range;
	color_mb_peaking_mb_de_over_hor_range_gain.mb_de_over_h_s_range = ptr->MBPK_H_Deovershoot0.MB_De_Over_H_S_range;

	//ROW6 VIP_MBPK_H_Deovershoot1
	color_mb_peaking_mb_peaking_de_over_h_s_0.mb_de_over_h_s_x0 = ptr->MBPK_H_Deovershoot1.x0;
	color_mb_peaking_mb_peaking_de_over_h_s_0.mb_de_over_h_s_y0 = ptr->MBPK_H_Deovershoot1.y0;
	color_mb_peaking_mb_peaking_de_over_h_s_0.mb_de_over_h_s_a0 = ptr->MBPK_H_Deovershoot1.a0;
	color_mb_peaking_mb_peaking_de_over_h_s_1.mb_de_over_h_s_x1 = ptr->MBPK_H_Deovershoot1.x1;
	color_mb_peaking_mb_peaking_de_over_h_s_1.mb_de_over_h_s_y1 = ptr->MBPK_H_Deovershoot1.y1;
	color_mb_peaking_mb_peaking_de_over_h_s_1.mb_de_over_h_s_a1 = ptr->MBPK_H_Deovershoot1.a1;
	color_mb_peaking_mb_peaking_de_over_h_s_2.mb_de_over_h_s_x2 = ptr->MBPK_H_Deovershoot1.x2;
	color_mb_peaking_mb_peaking_de_over_h_s_2.mb_de_over_h_s_y2 = ptr->MBPK_H_Deovershoot1.y2;
	color_mb_peaking_mb_peaking_de_over_h_s_2.mb_de_over_h_s_a2 = ptr->MBPK_H_Deovershoot1.a2;

	//ROW7 VIP_MBPK_H_Deovershoot2
	color_mb_peaking_mb_peaking_de_over_h_b_0.mb_de_over_h_b_x0 = ptr->MBPK_H_Deovershoot2.x0;
	color_mb_peaking_mb_peaking_de_over_h_b_0.mb_de_over_h_b_y0 = ptr->MBPK_H_Deovershoot2.y0;
	color_mb_peaking_mb_peaking_de_over_h_b_0.mb_de_over_h_b_a0 = ptr->MBPK_H_Deovershoot2.a0;
	color_mb_peaking_mb_peaking_de_over_h_b_1.mb_de_over_h_b_x1 = ptr->MBPK_H_Deovershoot2.x1;
	color_mb_peaking_mb_peaking_de_over_h_b_1.mb_de_over_h_b_y1 = ptr->MBPK_H_Deovershoot2.y1;
	color_mb_peaking_mb_peaking_de_over_h_b_1.mb_de_over_h_b_a1 = ptr->MBPK_H_Deovershoot2.a1;
	color_mb_peaking_mb_peaking_de_over_h_b_2.mb_de_over_h_b_x2 = ptr->MBPK_H_Deovershoot2.x2;
	color_mb_peaking_mb_peaking_de_over_h_b_2.mb_de_over_h_b_y2 = ptr->MBPK_H_Deovershoot2.y2;
	color_mb_peaking_mb_peaking_de_over_h_b_2.mb_de_over_h_b_a2 = ptr->MBPK_H_Deovershoot2.a2;

	//ROW8 VIP_MBPK_V_Deovershoot0
	color_mb_peaking_mb_peaking_ctrl.mb_de_over_v_en = ptr->MBPK_V_Deovershoot0.MB_De_Over_V_en;
	// removed color_mb_peaking_mb_de_over_ver_range_gain.mb_de_over_v_s_gain = ptr->MBPK_V_Deovershoot0.MB_De_Over_V_S_gain;
	color_mb_peaking_mb_de_over_ver_range_gain.mb_de_over_v_b_range = ptr->MBPK_V_Deovershoot0.MB_De_Over_V_B_range;
	color_mb_peaking_mb_de_over_ver_range_gain.mb_de_over_v_s_range = ptr->MBPK_V_Deovershoot0.MB_De_Over_V_S_range;

	//ROW9 VIP_MBPK_V_Deovershoot1
	color_mb_peaking_mb_peaking_de_over_v_s_0.mb_de_over_v_s_x0 = ptr->MBPK_V_Deovershoot1.x0;
	color_mb_peaking_mb_peaking_de_over_v_s_0.mb_de_over_v_s_y0 = ptr->MBPK_V_Deovershoot1.y0;
	color_mb_peaking_mb_peaking_de_over_v_s_0.mb_de_over_v_s_a0 = ptr->MBPK_V_Deovershoot1.a0;
	color_mb_peaking_mb_peaking_de_over_v_s_1.mb_de_over_v_s_x1 = ptr->MBPK_V_Deovershoot1.x1;
	color_mb_peaking_mb_peaking_de_over_v_s_1.mb_de_over_v_s_y1 = ptr->MBPK_V_Deovershoot1.y1;
	color_mb_peaking_mb_peaking_de_over_v_s_1.mb_de_over_v_s_a1 = ptr->MBPK_V_Deovershoot1.a1;
	color_mb_peaking_mb_peaking_de_over_v_s_2.mb_de_over_v_s_x2 = ptr->MBPK_V_Deovershoot1.x2;
	color_mb_peaking_mb_peaking_de_over_v_s_2.mb_de_over_v_s_y2 = ptr->MBPK_V_Deovershoot1.y2;
	color_mb_peaking_mb_peaking_de_over_v_s_2.mb_de_over_v_s_a2 = ptr->MBPK_V_Deovershoot1.a2;

	//ROW10 VIP_MBPK_V_Deovershoot2
	color_mb_peaking_mb_peaking_de_over_v_b_0.mb_de_over_v_b_x0 = ptr->MBPK_V_Deovershoot2.x0;
	color_mb_peaking_mb_peaking_de_over_v_b_0.mb_de_over_v_b_y0 = ptr->MBPK_V_Deovershoot2.y0;
	color_mb_peaking_mb_peaking_de_over_v_b_0.mb_de_over_v_b_a0 = ptr->MBPK_V_Deovershoot2.a0;
	color_mb_peaking_mb_peaking_de_over_v_b_1.mb_de_over_v_b_x1 = ptr->MBPK_V_Deovershoot2.x1;
	color_mb_peaking_mb_peaking_de_over_v_b_1.mb_de_over_v_b_y1 = ptr->MBPK_V_Deovershoot2.y1;
	color_mb_peaking_mb_peaking_de_over_v_b_1.mb_de_over_v_b_a1 = ptr->MBPK_V_Deovershoot2.a1;
	color_mb_peaking_mb_peaking_de_over_v_b_2.mb_de_over_v_b_x2 = ptr->MBPK_V_Deovershoot2.x2;
	color_mb_peaking_mb_peaking_de_over_v_b_2.mb_de_over_v_b_y2 = ptr->MBPK_V_Deovershoot2.y2;
	color_mb_peaking_mb_peaking_de_over_v_b_2.mb_de_over_v_b_a2 = ptr->MBPK_V_Deovershoot2.a2;

	//Row11 VIP_MBPK_WEI_coring
	color_mb_peaking_mb_snr_coring.mb_wei_coring0 = ptr->MBPK_WEI_coring.MB_WEI_coring0;
	color_mb_peaking_mb_wei_coring1.mb_wei_coring1 = ptr->MBPK_WEI_coring.MB_WEI_coring1;
	color_mb_peaking_mb_wei_coring1.mb_wei_coring2 = ptr->MBPK_WEI_coring.MB_WEI_coring2;
	color_mb_peaking_mb_wei_coring1.mb_wei_coring3 = ptr->MBPK_WEI_coring.MB_WEI_coring3;
	color_mb_peaking_mb_wei_coring1.mb_wei_coring4 = ptr->MBPK_WEI_coring.MB_WEI_coring4;
	color_mb_peaking_mb_wei_coring2.mb_wei_coring5 = ptr->MBPK_WEI_coring.MB_WEI_coring5;
	color_mb_peaking_mb_wei_coring2.mb_wei_coring6 = ptr->MBPK_WEI_coring.MB_WEI_coring6;
	color_mb_peaking_mb_wei_coring2.mb_wei_coring7 = ptr->MBPK_WEI_coring.MB_WEI_coring7;
	color_mb_peaking_mb_wei_coring2.mb_wei_coring8 = ptr->MBPK_WEI_coring.MB_WEI_coring8;

	color_mb_peaking_mb_peaking_emf_ctrl.mb_emf_enable = ptr->MBPK_EMF.mb_emf_enable;
	color_mb_peaking_mb_peaking_emf_ctrl.mb_emf_range_h = ptr->MBPK_EMF.mb_emf_range_h;
	color_mb_peaking_mb_peaking_emf_ctrl.mb_emf_range_v = ptr->MBPK_EMF.mb_emf_range_v;
	color_mb_peaking_mb_peaking_emf_seg_gain.mb_emf_blend_wt = ptr->MBPK_EMF.mb_emf_blend_wt;
	color_mb_peaking_mb_peaking_emf_seg_offset.mb_emf_seg0_offset = ptr->MBPK_EMF.mb_emf_seg0_offset;
	color_mb_peaking_mb_peaking_emf_seg_gain.mb_emf_seg0_gain_sel = ptr->MBPK_EMF.mb_emf_seg0_gain_sel;
	color_mb_peaking_mb_peaking_emf_ctrl.mb_emf_seg0_x = ptr->MBPK_EMF.mb_emf_seg0_x;
	color_mb_peaking_mb_peaking_emf_seg_offset.mb_emf_seg1_offset = ptr->MBPK_EMF.mb_emf_seg1_offset;
	color_mb_peaking_mb_peaking_emf_seg_gain.mb_emf_seg1_gain_sel = ptr->MBPK_EMF.mb_emf_seg1_gain_sel;
	color_mb_peaking_mb_peaking_emf_ctrl.mb_emf_seg1_x = ptr->MBPK_EMF.mb_emf_seg1_x;
	color_mb_peaking_mb_peaking_emf_seg_offset.mb_emf_seg2_offset = ptr->MBPK_EMF.mb_emf_seg2_offset;
	color_mb_peaking_mb_peaking_emf_seg_gain.mb_emf_seg2_gain_sel = ptr->MBPK_EMF.mb_emf_seg2_gain_sel;

	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg, color_mb_peaking_mb_peaking_ctrl.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Hor_FILTER_0_reg, color_mb_peaking_mb_peaking_hor_filter_0.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Hor_FILTER_1_reg, color_mb_peaking_mb_peaking_hor_filter_1.regValue);
	// removed IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Hor_FILTER_2_reg, color_mb_peaking_mb_peaking_hor_filter_2.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Hor_GAIN_reg, color_mb_peaking_mb_peaking_hor_gain.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Hor_BOUND_reg, color_mb_peaking_mb_peaking_hor_bound.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Hor_GAIN_2_reg, color_mb_peaking_mb_peaking_hor_gain_2.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Ver_FILTER_reg, color_mb_peaking_mb_peaking_ver_filter.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Ver_GAIN_reg, color_mb_peaking_mb_peaking_ver_gain.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Ver_BOUND_reg, color_mb_peaking_mb_peaking_ver_bound.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Ver_GAIN_2_reg, color_mb_peaking_mb_peaking_ver_gain_2.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_De_Over_Hor_RANGE_GAIN_reg, color_mb_peaking_mb_de_over_hor_range_gain.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_De_Over_Ver_RANGE_GAIN_reg, color_mb_peaking_mb_de_over_ver_range_gain.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_SNR_CORING_reg, color_mb_peaking_mb_snr_coring.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_WEI_CORING1_reg, color_mb_peaking_mb_wei_coring1.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_WEI_CORING2_reg, color_mb_peaking_mb_wei_coring2.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_S_0_reg, color_mb_peaking_mb_peaking_de_over_h_s_0.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_S_1_reg, color_mb_peaking_mb_peaking_de_over_h_s_1.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_S_2_reg, color_mb_peaking_mb_peaking_de_over_h_s_2.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_B_0_reg, color_mb_peaking_mb_peaking_de_over_h_b_0.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_B_1_reg, color_mb_peaking_mb_peaking_de_over_h_b_1.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_B_2_reg, color_mb_peaking_mb_peaking_de_over_h_b_2.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_S_0_reg, color_mb_peaking_mb_peaking_de_over_v_s_0.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_S_1_reg, color_mb_peaking_mb_peaking_de_over_v_s_1.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_S_2_reg, color_mb_peaking_mb_peaking_de_over_v_s_2.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_B_0_reg, color_mb_peaking_mb_peaking_de_over_v_b_0.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_B_1_reg, color_mb_peaking_mb_peaking_de_over_v_b_1.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_B_2_reg, color_mb_peaking_mb_peaking_de_over_v_b_2.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_EMF_Ctrl_reg, color_mb_peaking_mb_peaking_emf_ctrl.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_EMF_seg_gain_reg, color_mb_peaking_mb_peaking_emf_seg_gain.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_EMF_seg_offset_reg, color_mb_peaking_mb_peaking_emf_seg_offset.regValue);

	//Enable doublue buffer
	color_mb_peaking_mb_double_buffer_ctrl.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg);
	color_mb_peaking_mb_double_buffer_ctrl.mb_db_en =1;
	color_mb_peaking_mb_double_buffer_ctrl.mb_db_apply =1;
	IoReg_Write32(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, color_mb_peaking_mb_double_buffer_ctrl.regValue);
}

void drvif_color_Get_MBPK_Table(VIP_MBPK_Table *ptr)
{

	color_mb_peaking_mb_peaking_ctrl_RBUS				color_mb_peaking_mb_peaking_ctrl;
	color_mb_peaking_mb_peaking_hor_filter_0_RBUS		color_mb_peaking_mb_peaking_hor_filter_0;
	color_mb_peaking_mb_peaking_hor_filter_1_RBUS		color_mb_peaking_mb_peaking_hor_filter_1;
	color_mb_peaking_mb_peaking_hor_gain_RBUS			color_mb_peaking_mb_peaking_hor_gain;
	color_mb_peaking_mb_peaking_hor_bound_RBUS		color_mb_peaking_mb_peaking_hor_bound;
	color_mb_peaking_mb_peaking_hor_gain_2_RBUS		color_mb_peaking_mb_peaking_hor_gain_2;
	color_mb_peaking_mb_peaking_ver_filter_RBUS		color_mb_peaking_mb_peaking_ver_filter;
	color_mb_peaking_mb_peaking_ver_gain_RBUS			color_mb_peaking_mb_peaking_ver_gain;
	color_mb_peaking_mb_peaking_ver_bound_RBUS		color_mb_peaking_mb_peaking_ver_bound;
	color_mb_peaking_mb_peaking_ver_gain_2_RBUS		color_mb_peaking_mb_peaking_ver_gain_2;
	color_mb_peaking_mb_de_over_hor_range_gain_RBUS color_mb_peaking_mb_de_over_hor_range_gain;
	color_mb_peaking_mb_de_over_ver_range_gain_RBUS color_mb_peaking_mb_de_over_ver_range_gain;
	color_mb_peaking_mb_snr_coring_RBUS color_mb_peaking_mb_snr_coring;
	color_mb_peaking_mb_wei_coring1_RBUS color_mb_peaking_mb_wei_coring1;
	color_mb_peaking_mb_wei_coring2_RBUS color_mb_peaking_mb_wei_coring2;
	color_mb_peaking_mb_peaking_de_over_h_s_0_RBUS color_mb_peaking_mb_peaking_de_over_h_s_0;
	color_mb_peaking_mb_peaking_de_over_h_s_1_RBUS color_mb_peaking_mb_peaking_de_over_h_s_1;
	color_mb_peaking_mb_peaking_de_over_h_s_2_RBUS color_mb_peaking_mb_peaking_de_over_h_s_2;
	color_mb_peaking_mb_peaking_de_over_h_b_0_RBUS color_mb_peaking_mb_peaking_de_over_h_b_0;
	color_mb_peaking_mb_peaking_de_over_h_b_1_RBUS color_mb_peaking_mb_peaking_de_over_h_b_1;
	color_mb_peaking_mb_peaking_de_over_h_b_2_RBUS color_mb_peaking_mb_peaking_de_over_h_b_2;
	color_mb_peaking_mb_peaking_de_over_v_s_0_RBUS color_mb_peaking_mb_peaking_de_over_v_s_0;
	color_mb_peaking_mb_peaking_de_over_v_s_1_RBUS color_mb_peaking_mb_peaking_de_over_v_s_1;
	color_mb_peaking_mb_peaking_de_over_v_s_2_RBUS color_mb_peaking_mb_peaking_de_over_v_s_2;
	color_mb_peaking_mb_peaking_de_over_v_b_0_RBUS color_mb_peaking_mb_peaking_de_over_v_b_0;
	color_mb_peaking_mb_peaking_de_over_v_b_1_RBUS color_mb_peaking_mb_peaking_de_over_v_b_1;
	color_mb_peaking_mb_peaking_de_over_v_b_2_RBUS color_mb_peaking_mb_peaking_de_over_v_b_2;
	color_mb_peaking_mb_peaking_emf_ctrl_RBUS color_mb_peaking_mb_peaking_emf_ctrl;
	color_mb_peaking_mb_peaking_emf_seg_gain_RBUS color_mb_peaking_mb_peaking_emf_seg_gain;
	color_mb_peaking_mb_peaking_emf_seg_offset_RBUS color_mb_peaking_mb_peaking_emf_seg_offset;

	if (!ptr)
		return;
	color_mb_peaking_mb_peaking_ctrl.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_CTRL_reg);
	color_mb_peaking_mb_peaking_hor_filter_0.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Hor_FILTER_0_reg);
	color_mb_peaking_mb_peaking_hor_filter_1.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Hor_FILTER_1_reg);
	color_mb_peaking_mb_peaking_hor_gain.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Hor_GAIN_reg);
	color_mb_peaking_mb_peaking_hor_bound.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Hor_BOUND_reg);
	color_mb_peaking_mb_peaking_hor_gain_2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Hor_GAIN_2_reg);
	color_mb_peaking_mb_peaking_ver_filter.regValue =IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Ver_FILTER_reg);
	color_mb_peaking_mb_peaking_ver_gain.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Ver_GAIN_reg);
	color_mb_peaking_mb_peaking_ver_bound.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Ver_BOUND_reg);
	color_mb_peaking_mb_peaking_ver_gain_2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Ver_GAIN_2_reg);
	color_mb_peaking_mb_de_over_hor_range_gain.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_De_Over_Hor_RANGE_GAIN_reg);
	color_mb_peaking_mb_de_over_ver_range_gain.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_De_Over_Ver_RANGE_GAIN_reg);
	color_mb_peaking_mb_snr_coring.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_SNR_CORING_reg);
	color_mb_peaking_mb_wei_coring1.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_WEI_CORING1_reg);
	color_mb_peaking_mb_wei_coring2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_WEI_CORING2_reg);
	color_mb_peaking_mb_peaking_de_over_h_s_0.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_S_0_reg);
	color_mb_peaking_mb_peaking_de_over_h_s_1.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_S_1_reg);
	color_mb_peaking_mb_peaking_de_over_h_s_2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_S_2_reg);
	color_mb_peaking_mb_peaking_de_over_h_b_0.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_B_0_reg);
	color_mb_peaking_mb_peaking_de_over_h_b_1.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_B_1_reg);
	color_mb_peaking_mb_peaking_de_over_h_b_2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_H_B_2_reg);
	color_mb_peaking_mb_peaking_de_over_v_s_0.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_S_0_reg);
	color_mb_peaking_mb_peaking_de_over_v_s_1.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_S_1_reg);
	color_mb_peaking_mb_peaking_de_over_v_s_2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_S_2_reg);
	color_mb_peaking_mb_peaking_de_over_v_b_0.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_B_0_reg);
	color_mb_peaking_mb_peaking_de_over_v_b_1.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_B_1_reg);
	color_mb_peaking_mb_peaking_de_over_v_b_2.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_De_Over_V_B_2_reg);
	color_mb_peaking_mb_peaking_emf_ctrl.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_EMF_Ctrl_reg);
	color_mb_peaking_mb_peaking_emf_seg_gain.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_EMF_seg_gain_reg);
	color_mb_peaking_mb_peaking_emf_seg_offset.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_EMF_seg_offset_reg);


	//ROW0 VIP_MBPK_Ctrl
	ptr->MBPK_Ctrl.MB_V_En = color_mb_peaking_mb_peaking_ctrl.mb_v_en;
	ptr->MBPK_Ctrl.MB_H_En = color_mb_peaking_mb_peaking_ctrl.mb_h_en;
	ptr->MBPK_Ctrl.MB_Ver_Vfilter_data = color_mb_peaking_mb_peaking_ctrl.mb_ver_vfilter_data;
	ptr->MBPK_Ctrl.mb_peaking_en = color_mb_peaking_mb_peaking_ctrl.mb_peaking_en;
	ptr->MBPK_Ctrl.MB_SNR_flag_en = color_mb_peaking_mb_peaking_ctrl.mb_snr_flag_en;
	ptr->MBPK_Ctrl.MB_MOS_coring = color_mb_peaking_mb_snr_coring.mb_mos_coring;
	ptr->MBPK_Ctrl.MB_De_Over_HVmin_en = color_mb_peaking_mb_peaking_ctrl.mb_de_over_hvmin_en;
	ptr->MBPK_Ctrl.MB_De_Over_BSmaxmin = color_mb_peaking_mb_peaking_ctrl.mb_de_over_bsmaxmin;

	//ROW1 VIP_MBPK_H_Coef
	/*DC Value Check*/

	ptr->MBPK_H_Coef.MB_Hor_Filter_C0 = color_mb_peaking_mb_peaking_hor_filter_0.mb_hor_filter_c0;
	ptr->MBPK_H_Coef.MB_Hor_Filter_C1 = color_mb_peaking_mb_peaking_hor_filter_0.mb_hor_filter_c1;
	ptr->MBPK_H_Coef.MB_Hor_Filter_C2 = color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c2;
	ptr->MBPK_H_Coef.MB_Hor_Filter_C3 = color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c3;

	//ROW2 VIP_MBPK_H_Table
	ptr->MBPK_H_Table.MB_Gain_Neg = color_mb_peaking_mb_peaking_hor_gain.mb_gain_neg;
	ptr->MBPK_H_Table.MB_Gain_Pos = color_mb_peaking_mb_peaking_hor_gain.mb_gain_pos;
	ptr->MBPK_H_Table.MB_HV_Neg = color_mb_peaking_mb_peaking_hor_bound.mb_hv_neg;
	ptr->MBPK_H_Table.MB_HV_Pos = color_mb_peaking_mb_peaking_hor_bound.mb_hv_pos;
	ptr->MBPK_H_Table.MB_D2_shift_bit =  color_mb_peaking_mb_peaking_hor_gain_2.mb_d2_shift_bit;
	ptr->MBPK_H_Table.MB_Gain_Neg2 = color_mb_peaking_mb_peaking_hor_gain_2.mb_gain_neg2;
	ptr->MBPK_H_Table.MB_Gain_Pos2 = color_mb_peaking_mb_peaking_hor_gain_2.mb_gain_pos2;
	ptr->MBPK_H_Table.MB_LV = color_mb_peaking_mb_peaking_hor_gain_2.mb_lv;
	ptr->MBPK_H_Table.MB_LV2 = color_mb_peaking_mb_peaking_hor_gain_2.mb_lv2;

	//ROW3 VIP_MBPK_V_Coef
	/*DC Value Check*/

	ptr->MBPK_V_Coef.MB_Ver_Filter_C0 = color_mb_peaking_mb_peaking_ver_filter.mb_ver_filter_c0;
	ptr->MBPK_V_Coef.MB_Ver_Filter_C1 = color_mb_peaking_mb_peaking_ver_filter.mb_ver_filter_c1;
	ptr->MBPK_V_Coef.MB_Ver_Filter_C2 = color_mb_peaking_mb_peaking_ver_filter.mb_ver_filter_c2;

	//ROW4 VIP_MBPK_V_Table
	ptr->MBPK_V_Table.MB_Gain_Neg_V = color_mb_peaking_mb_peaking_ver_gain.mb_gain_neg_v;
	ptr->MBPK_V_Table.MB_Gain_Pos_V = color_mb_peaking_mb_peaking_ver_gain.mb_gain_pos_v;
	ptr->MBPK_V_Table.MB_HV_Neg_V = color_mb_peaking_mb_peaking_ver_bound.mb_hv_neg_v;
	ptr->MBPK_V_Table.MB_HV_Pos_V = color_mb_peaking_mb_peaking_ver_bound.mb_hv_pos_v;
	ptr->MBPK_V_Table.MB_D2_shift_bit_V = color_mb_peaking_mb_peaking_ver_gain_2.mb_d2_shift_bit_v;
	ptr->MBPK_V_Table.MB_Gain_Neg2_V = color_mb_peaking_mb_peaking_ver_gain_2.mb_gain_neg2_v;
	ptr->MBPK_V_Table.MB_Gain_Pos2_V = color_mb_peaking_mb_peaking_ver_gain_2.mb_gain_pos2_v;
	ptr->MBPK_V_Table.MB_LV2_V = color_mb_peaking_mb_peaking_ver_gain_2.mb_lv2_v;
	ptr->MBPK_V_Table.MB_LV_V = color_mb_peaking_mb_peaking_ver_gain_2.mb_lv_v;

	//ROW5 VIP_MBPK_H_Deovershoot0
	ptr->MBPK_H_Deovershoot0.MB_De_Over_H_en = color_mb_peaking_mb_peaking_ctrl.mb_de_over_h_en;
	ptr->MBPK_H_Deovershoot0.MB_De_Over_H_B_range = color_mb_peaking_mb_de_over_hor_range_gain.mb_de_over_h_b_range;
	ptr->MBPK_H_Deovershoot0.MB_De_Over_H_S_range = color_mb_peaking_mb_de_over_hor_range_gain.mb_de_over_h_s_range;

	//ROW6 VIP_MBPK_H_Deovershoot1
	ptr->MBPK_H_Deovershoot1.x0 = color_mb_peaking_mb_peaking_de_over_h_s_0.mb_de_over_h_s_x0;
	ptr->MBPK_H_Deovershoot1.y0 = color_mb_peaking_mb_peaking_de_over_h_s_0.mb_de_over_h_s_y0;
	ptr->MBPK_H_Deovershoot1.a0 = color_mb_peaking_mb_peaking_de_over_h_s_0.mb_de_over_h_s_a0;
	ptr->MBPK_H_Deovershoot1.x1 = color_mb_peaking_mb_peaking_de_over_h_s_1.mb_de_over_h_s_x1;
	ptr->MBPK_H_Deovershoot1.y1 = color_mb_peaking_mb_peaking_de_over_h_s_1.mb_de_over_h_s_y1;
	ptr->MBPK_H_Deovershoot1.a1 = color_mb_peaking_mb_peaking_de_over_h_s_1.mb_de_over_h_s_a1;
	ptr->MBPK_H_Deovershoot1.x2 = color_mb_peaking_mb_peaking_de_over_h_s_2.mb_de_over_h_s_x2;
	ptr->MBPK_H_Deovershoot1.y2 = color_mb_peaking_mb_peaking_de_over_h_s_2.mb_de_over_h_s_y2;
	ptr->MBPK_H_Deovershoot1.a2 = color_mb_peaking_mb_peaking_de_over_h_s_2.mb_de_over_h_s_a2;

	//ROW7 VIP_MBPK_H_Deovershoot2
	ptr->MBPK_H_Deovershoot2.x0 = color_mb_peaking_mb_peaking_de_over_h_b_0.mb_de_over_h_b_x0;
	ptr->MBPK_H_Deovershoot2.y0 = color_mb_peaking_mb_peaking_de_over_h_b_0.mb_de_over_h_b_y0;
	ptr->MBPK_H_Deovershoot2.a0 = color_mb_peaking_mb_peaking_de_over_h_b_0.mb_de_over_h_b_a0;
	ptr->MBPK_H_Deovershoot2.x1 = color_mb_peaking_mb_peaking_de_over_h_b_1.mb_de_over_h_b_x1;
	ptr->MBPK_H_Deovershoot2.y1 = color_mb_peaking_mb_peaking_de_over_h_b_1.mb_de_over_h_b_y1;
	ptr->MBPK_H_Deovershoot2.a1 = color_mb_peaking_mb_peaking_de_over_h_b_1.mb_de_over_h_b_a1;
	ptr->MBPK_H_Deovershoot2.x2 = color_mb_peaking_mb_peaking_de_over_h_b_2.mb_de_over_h_b_x2;
	ptr->MBPK_H_Deovershoot2.y2 = color_mb_peaking_mb_peaking_de_over_h_b_2.mb_de_over_h_b_y2;
	ptr->MBPK_H_Deovershoot2.a2 = color_mb_peaking_mb_peaking_de_over_h_b_2.mb_de_over_h_b_a2;

	//ROW8 VIP_MBPK_V_Deovershoot0
	ptr->MBPK_V_Deovershoot0.MB_De_Over_V_en = color_mb_peaking_mb_peaking_ctrl.mb_de_over_v_en ;
	ptr->MBPK_V_Deovershoot0.MB_De_Over_V_B_range = color_mb_peaking_mb_de_over_ver_range_gain.mb_de_over_v_b_range;
	ptr->MBPK_V_Deovershoot0.MB_De_Over_V_S_range = color_mb_peaking_mb_de_over_ver_range_gain.mb_de_over_v_s_range;

	//ROW9 VIP_MBPK_V_Deovershoot1
	ptr->MBPK_V_Deovershoot1.x0 = color_mb_peaking_mb_peaking_de_over_v_s_0.mb_de_over_v_s_x0;
	ptr->MBPK_V_Deovershoot1.y0 = color_mb_peaking_mb_peaking_de_over_v_s_0.mb_de_over_v_s_y0;
	ptr->MBPK_V_Deovershoot1.a0 = color_mb_peaking_mb_peaking_de_over_v_s_0.mb_de_over_v_s_a0;
	ptr->MBPK_V_Deovershoot1.x1 = color_mb_peaking_mb_peaking_de_over_v_s_1.mb_de_over_v_s_x1;
	ptr->MBPK_V_Deovershoot1.y1 = color_mb_peaking_mb_peaking_de_over_v_s_1.mb_de_over_v_s_y1;
	ptr->MBPK_V_Deovershoot1.a1 = color_mb_peaking_mb_peaking_de_over_v_s_1.mb_de_over_v_s_a1;
	ptr->MBPK_V_Deovershoot1.x2 = color_mb_peaking_mb_peaking_de_over_v_s_2.mb_de_over_v_s_x2;
	ptr->MBPK_V_Deovershoot1.y2 = color_mb_peaking_mb_peaking_de_over_v_s_2.mb_de_over_v_s_y2;
	ptr->MBPK_V_Deovershoot1.a2 = color_mb_peaking_mb_peaking_de_over_v_s_2.mb_de_over_v_s_a2;

	//ROW10 VIP_MBPK_V_Deovershoot2
	ptr->MBPK_V_Deovershoot2.x0 = color_mb_peaking_mb_peaking_de_over_v_b_0.mb_de_over_v_b_x0;
	ptr->MBPK_V_Deovershoot2.y0 = color_mb_peaking_mb_peaking_de_over_v_b_0.mb_de_over_v_b_y0;
	ptr->MBPK_V_Deovershoot2.a0 = color_mb_peaking_mb_peaking_de_over_v_b_0.mb_de_over_v_b_a0;
	ptr->MBPK_V_Deovershoot2.x1 = color_mb_peaking_mb_peaking_de_over_v_b_1.mb_de_over_v_b_x1;
	ptr->MBPK_V_Deovershoot2.y1 = color_mb_peaking_mb_peaking_de_over_v_b_1.mb_de_over_v_b_y1;
	ptr->MBPK_V_Deovershoot2.a1 = color_mb_peaking_mb_peaking_de_over_v_b_1.mb_de_over_v_b_a1;
	ptr->MBPK_V_Deovershoot2.x2 = color_mb_peaking_mb_peaking_de_over_v_b_2.mb_de_over_v_b_x2;
	ptr->MBPK_V_Deovershoot2.y2 = color_mb_peaking_mb_peaking_de_over_v_b_2.mb_de_over_v_b_y2;
	ptr->MBPK_V_Deovershoot2.a2 = color_mb_peaking_mb_peaking_de_over_v_b_2.mb_de_over_v_b_a2;

	//Row11 VIP_MBPK_WEI_coring
	ptr->MBPK_WEI_coring.MB_WEI_coring0 = color_mb_peaking_mb_snr_coring.mb_wei_coring0 ;
	ptr->MBPK_WEI_coring.MB_WEI_coring1 = color_mb_peaking_mb_wei_coring1.mb_wei_coring1;
	ptr->MBPK_WEI_coring.MB_WEI_coring2 = color_mb_peaking_mb_wei_coring1.mb_wei_coring2;
	ptr->MBPK_WEI_coring.MB_WEI_coring3 = color_mb_peaking_mb_wei_coring1.mb_wei_coring3;
	ptr->MBPK_WEI_coring.MB_WEI_coring4 = color_mb_peaking_mb_wei_coring1.mb_wei_coring4;
	ptr->MBPK_WEI_coring.MB_WEI_coring5 = color_mb_peaking_mb_wei_coring2.mb_wei_coring5;
	ptr->MBPK_WEI_coring.MB_WEI_coring6 = color_mb_peaking_mb_wei_coring2.mb_wei_coring6;
	ptr->MBPK_WEI_coring.MB_WEI_coring7 = color_mb_peaking_mb_wei_coring2.mb_wei_coring7;
	ptr->MBPK_WEI_coring.MB_WEI_coring8 = color_mb_peaking_mb_wei_coring2.mb_wei_coring8;

	ptr->MBPK_EMF.mb_emf_enable = color_mb_peaking_mb_peaking_emf_ctrl.mb_emf_enable;
	ptr->MBPK_EMF.mb_emf_range_h = color_mb_peaking_mb_peaking_emf_ctrl.mb_emf_range_h;
	ptr->MBPK_EMF.mb_emf_range_v = color_mb_peaking_mb_peaking_emf_ctrl.mb_emf_range_v;
	ptr->MBPK_EMF.mb_emf_blend_wt = color_mb_peaking_mb_peaking_emf_seg_gain.mb_emf_blend_wt;
	ptr->MBPK_EMF.mb_emf_seg0_offset = color_mb_peaking_mb_peaking_emf_seg_offset.mb_emf_seg0_offset;
	ptr->MBPK_EMF.mb_emf_seg0_gain_sel = color_mb_peaking_mb_peaking_emf_seg_gain.mb_emf_seg0_gain_sel;
	ptr->MBPK_EMF.mb_emf_seg0_x = color_mb_peaking_mb_peaking_emf_ctrl.mb_emf_seg0_x;
	ptr->MBPK_EMF.mb_emf_seg1_offset = color_mb_peaking_mb_peaking_emf_seg_offset.mb_emf_seg1_offset;
	ptr->MBPK_EMF.mb_emf_seg1_gain_sel = color_mb_peaking_mb_peaking_emf_seg_gain.mb_emf_seg1_gain_sel;
	ptr->MBPK_EMF.mb_emf_seg1_x = color_mb_peaking_mb_peaking_emf_ctrl.mb_emf_seg1_x;
	ptr->MBPK_EMF.mb_emf_seg2_offset = color_mb_peaking_mb_peaking_emf_seg_offset.mb_emf_seg2_offset;
	ptr->MBPK_EMF.mb_emf_seg2_gain_sel = color_mb_peaking_mb_peaking_emf_seg_gain.mb_emf_seg2_gain_sel;
}

void drvif_color_Set_MBSU_Table(VIP_MBPK_Table *ptr) /*Merlin3*/
{

	color_mb_su_peaking_mb_su_peaking_ctrl_RBUS			color_mb_peaking_mb_peaking_ctrl;
	color_mb_su_peaking_mb_su_peaking_hor_filter_0_RBUS		color_mb_peaking_mb_peaking_hor_filter_0;
	color_mb_su_peaking_mb_su_peaking_hor_filter_1_RBUS		color_mb_peaking_mb_peaking_hor_filter_1;
	color_mb_su_peaking_mb_su_peaking_hor_filter_2_RBUS		color_mb_peaking_mb_peaking_hor_filter_2;
	color_mb_su_peaking_mb_su_peaking_hor_gain_RBUS			color_mb_peaking_mb_peaking_hor_gain;
	color_mb_su_peaking_mb_su_peaking_hor_bound_RBUS		color_mb_peaking_mb_peaking_hor_bound;
	color_mb_su_peaking_mb_su_peaking_hor_gain_2_RBUS		color_mb_peaking_mb_peaking_hor_gain_2;
	color_mb_su_peaking_mb_su_peaking_ver_filter_RBUS		color_mb_peaking_mb_peaking_ver_filter;
	color_mb_su_peaking_mb_su_peaking_ver_gain_RBUS			color_mb_peaking_mb_peaking_ver_gain;
	color_mb_su_peaking_mb_su_peaking_ver_bound_RBUS		color_mb_peaking_mb_peaking_ver_bound;
	color_mb_su_peaking_mb_su_peaking_ver_gain_2_RBUS		color_mb_peaking_mb_peaking_ver_gain_2;
	color_mb_su_peaking_mb_su_double_buffer_ctrl_RBUS		color_mb_peaking_mb_double_buffer_ctrl;
	/*color_mb_peaking_mb_dummy_RBUS					color_mb_peaking_mb_dummy;*/
	color_mb_su_peaking_mb_su_de_over_hor_range_gain_RBUS color_mb_peaking_mb_de_over_hor_range_gain;
	color_mb_su_peaking_mb_su_de_over_ver_range_gain_RBUS color_mb_peaking_mb_de_over_ver_range_gain;
	//color_mb_su_peaking_mb_su_snr_coring_RBUS color_mb_peaking_mb_snr_coring;
	color_mb_su_peaking_mb_su_peaking_de_over_h_s_0_RBUS color_mb_peaking_mb_peaking_de_over_h_s_0;
	color_mb_su_peaking_mb_su_peaking_de_over_h_s_1_RBUS color_mb_peaking_mb_peaking_de_over_h_s_1;
	color_mb_su_peaking_mb_su_peaking_de_over_h_s_2_RBUS color_mb_peaking_mb_peaking_de_over_h_s_2;
	color_mb_su_peaking_mb_su_peaking_de_over_h_b_0_RBUS color_mb_peaking_mb_peaking_de_over_h_b_0;
	color_mb_su_peaking_mb_su_peaking_de_over_h_b_1_RBUS color_mb_peaking_mb_peaking_de_over_h_b_1;
	color_mb_su_peaking_mb_su_peaking_de_over_h_b_2_RBUS color_mb_peaking_mb_peaking_de_over_h_b_2;
	color_mb_su_peaking_mb_su_peaking_de_over_v_s_0_RBUS color_mb_peaking_mb_peaking_de_over_v_s_0;
	color_mb_su_peaking_mb_su_peaking_de_over_v_s_1_RBUS color_mb_peaking_mb_peaking_de_over_v_s_1;
	color_mb_su_peaking_mb_su_peaking_de_over_v_s_2_RBUS color_mb_peaking_mb_peaking_de_over_v_s_2;
	color_mb_su_peaking_mb_su_peaking_de_over_v_b_0_RBUS color_mb_peaking_mb_peaking_de_over_v_b_0;
	color_mb_su_peaking_mb_su_peaking_de_over_v_b_1_RBUS color_mb_peaking_mb_peaking_de_over_v_b_1;
	color_mb_su_peaking_mb_su_peaking_de_over_v_b_2_RBUS color_mb_peaking_mb_peaking_de_over_v_b_2;

	if (!ptr)
		return;

	color_mb_peaking_mb_peaking_ctrl.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_CTRL_reg);
	color_mb_peaking_mb_peaking_hor_filter_0.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_FILTER_0_reg);
	color_mb_peaking_mb_peaking_hor_filter_1.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_FILTER_1_reg);
	color_mb_peaking_mb_peaking_hor_filter_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_FILTER_2_reg);
	color_mb_peaking_mb_peaking_hor_gain.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_GAIN_reg);
	color_mb_peaking_mb_peaking_hor_bound.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_BOUND_reg);
	color_mb_peaking_mb_peaking_hor_gain_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_GAIN_2_reg);
	color_mb_peaking_mb_peaking_ver_filter.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Ver_FILTER_reg);
	color_mb_peaking_mb_peaking_ver_gain.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Ver_GAIN_reg);
	color_mb_peaking_mb_peaking_ver_bound.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Ver_BOUND_reg);
	color_mb_peaking_mb_peaking_ver_gain_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Ver_GAIN_2_reg);
	color_mb_peaking_mb_de_over_hor_range_gain.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_De_Over_Hor_RANGE_GAIN_reg);
	color_mb_peaking_mb_de_over_ver_range_gain.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_De_Over_Ver_RANGE_GAIN_reg);
	color_mb_peaking_mb_peaking_de_over_h_s_0.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_S_0_reg);
	color_mb_peaking_mb_peaking_de_over_h_s_1.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_S_1_reg);
	color_mb_peaking_mb_peaking_de_over_h_s_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_S_2_reg);
	color_mb_peaking_mb_peaking_de_over_h_b_0.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_B_0_reg);
	color_mb_peaking_mb_peaking_de_over_h_b_1.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_B_1_reg);
	color_mb_peaking_mb_peaking_de_over_h_b_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_B_2_reg);
	color_mb_peaking_mb_peaking_de_over_v_s_0.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_S_0_reg);
	color_mb_peaking_mb_peaking_de_over_v_s_1.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_S_1_reg);
	color_mb_peaking_mb_peaking_de_over_v_s_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_S_2_reg);
	color_mb_peaking_mb_peaking_de_over_v_b_0.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_B_0_reg);
	color_mb_peaking_mb_peaking_de_over_v_b_1.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_B_1_reg);
	color_mb_peaking_mb_peaking_de_over_v_b_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_B_2_reg);

	//Enable doublue buffer
	color_mb_peaking_mb_double_buffer_ctrl.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_Double_Buffer_CTRL_reg);
	color_mb_peaking_mb_double_buffer_ctrl.mb_db_en =1;
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_Double_Buffer_CTRL_reg, color_mb_peaking_mb_double_buffer_ctrl.regValue);




	//ROW0 VIP_MBPK_Ctrl
	color_mb_peaking_mb_peaking_ctrl.mb_v_en = ptr->MBPK_Ctrl.MB_V_En;
	color_mb_peaking_mb_peaking_ctrl.mb_h_en = ptr->MBPK_Ctrl.MB_H_En;
	// color_mb_peaking_mb_peaking_ctrl.mb_ver_vfilter_data = ptr->MBPK_Ctrl.MB_Ver_Vfilter_data;
	color_mb_peaking_mb_peaking_ctrl.mb_peaking_en = ptr->MBPK_Ctrl.mb_peaking_en;
	// color_mb_peaking_mb_peaking_ctrl.mb_snr_flag_en = ptr->MBPK_Ctrl.MB_SNR_flag_en;
	// color_mb_peaking_mb_snr_coring.mb_mos_coring = ptr->MBPK_Ctrl.MB_MOS_coring;
	// color_mb_peaking_mb_snr_coring.mb_wei_coring = ptr->MBPK_Ctrl.MB_WEI_coring;
	color_mb_peaking_mb_peaking_ctrl.mb_de_over_hvmin_en= ptr->MBPK_Ctrl.MB_De_Over_HVmin_en;
	color_mb_peaking_mb_peaking_ctrl.mb_de_over_bsmaxmin= ptr->MBPK_Ctrl.MB_De_Over_BSmaxmin;

	//ROW1 VIP_MBPK_H_Coef
	/*DC Value Check*/
	if (SHARPNESS_LOG) {
		int sum;

		/*MB_Hor_Filter*/
		sum = 0;
		sum = ptr->MBPK_H_Coef.MB_Hor_Filter_C0 +
		      2*(ptr->MBPK_H_Coef.MB_Hor_Filter_C1 + ptr->MBPK_H_Coef.MB_Hor_Filter_C2 + ptr->MBPK_H_Coef.MB_Hor_Filter_C3 + ptr->MBPK_H_Coef.MB_Hor_Filter_C4 + ptr->MBPK_H_Coef.MB_Hor_Filter_C5 + ptr->MBPK_H_Coef.MB_Hor_Filter_C6);
		if (sum != 0)
			VIPprintf("[Warnning] [%s-%s-%d] [MB_Hor_Filter Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);
	}

	color_mb_peaking_mb_peaking_hor_filter_0.mb_hor_filter_c0 = ptr->MBPK_H_Coef.MB_Hor_Filter_C0;
	color_mb_peaking_mb_peaking_hor_filter_0.mb_hor_filter_c1 = ptr->MBPK_H_Coef.MB_Hor_Filter_C1;
	color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c2 = ptr->MBPK_H_Coef.MB_Hor_Filter_C2;
	color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c3 = ptr->MBPK_H_Coef.MB_Hor_Filter_C3;
	color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c4 = ptr->MBPK_H_Coef.MB_Hor_Filter_C4;
	color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c5 = ptr->MBPK_H_Coef.MB_Hor_Filter_C5;
	color_mb_peaking_mb_peaking_hor_filter_2.mb_hor_filter_c6 = ptr->MBPK_H_Coef.MB_Hor_Filter_C6;
	color_mb_peaking_mb_peaking_ctrl.mb_h_mode = ptr->MBPK_H_Coef.mb_h_mode;

	//ROW2 VIP_MBPK_H_Table
	color_mb_peaking_mb_peaking_hor_gain.mb_gain_neg = ptr->MBPK_H_Table.MB_Gain_Neg;
	color_mb_peaking_mb_peaking_hor_gain.mb_gain_pos = ptr->MBPK_H_Table.MB_Gain_Pos;
	color_mb_peaking_mb_peaking_hor_bound.mb_hv_neg = ptr->MBPK_H_Table.MB_HV_Neg;
	color_mb_peaking_mb_peaking_hor_bound.mb_hv_pos = ptr->MBPK_H_Table.MB_HV_Pos;
	color_mb_peaking_mb_peaking_hor_gain_2.mb_d2_shift_bit = ptr->MBPK_H_Table.MB_D2_shift_bit;
	color_mb_peaking_mb_peaking_hor_gain_2.mb_gain_neg2 = ptr->MBPK_H_Table.MB_Gain_Neg2;
	color_mb_peaking_mb_peaking_hor_gain_2.mb_gain_pos2 = ptr->MBPK_H_Table.MB_Gain_Pos2;
	color_mb_peaking_mb_peaking_hor_gain_2.mb_lv = ptr->MBPK_H_Table.MB_LV;
	color_mb_peaking_mb_peaking_hor_gain_2.mb_lv2 = ptr->MBPK_H_Table.MB_LV2;

	//ROW3 VIP_MBPK_V_Coef
	/*DC Value Check*/
	if (SHARPNESS_LOG) {
		int sum;

		/*MB_Ver_Filter*/
		sum = 0;
		sum = ptr->MBPK_V_Coef.MB_Ver_Filter_C0 +
		      2*(ptr->MBPK_V_Coef.MB_Ver_Filter_C1 + ptr->MBPK_V_Coef.MB_Ver_Filter_C2);
		if (sum != 0)
			VIPprintf("[Warnning] [%s-%s-%d] [MB_Ver_Filter Filter] DC Not Balance!\n", __FILE__, __func__, __LINE__);
	}

	color_mb_peaking_mb_peaking_ver_filter.mb_ver_filter_c0 = ptr->MBPK_V_Coef.MB_Ver_Filter_C0;
	color_mb_peaking_mb_peaking_ver_filter.mb_ver_filter_c1 = ptr->MBPK_V_Coef.MB_Ver_Filter_C1;
	color_mb_peaking_mb_peaking_ver_filter.mb_ver_filter_c2 = ptr->MBPK_V_Coef.MB_Ver_Filter_C2;

	//ROW4 VIP_MBPK_V_Table
	color_mb_peaking_mb_peaking_ver_gain.mb_gain_neg_v = ptr->MBPK_V_Table.MB_Gain_Neg_V;
	color_mb_peaking_mb_peaking_ver_gain.mb_gain_pos_v = ptr->MBPK_V_Table.MB_Gain_Pos_V;
	color_mb_peaking_mb_peaking_ver_bound.mb_hv_neg_v = ptr->MBPK_V_Table.MB_HV_Neg_V;
	color_mb_peaking_mb_peaking_ver_bound.mb_hv_pos_v = ptr->MBPK_V_Table.MB_HV_Pos_V;
	color_mb_peaking_mb_peaking_ver_gain_2.mb_d2_shift_bit_v = ptr->MBPK_V_Table.MB_D2_shift_bit_V;
	color_mb_peaking_mb_peaking_ver_gain_2.mb_gain_neg2_v = ptr->MBPK_V_Table.MB_Gain_Neg2_V;
	color_mb_peaking_mb_peaking_ver_gain_2.mb_gain_pos2_v = ptr->MBPK_V_Table.MB_Gain_Pos2_V;
	color_mb_peaking_mb_peaking_ver_gain_2.mb_lv2_v = ptr->MBPK_V_Table.MB_LV2_V;
	color_mb_peaking_mb_peaking_ver_gain_2.mb_lv_v = ptr->MBPK_V_Table.MB_LV_V;
	/*color_mb_peaking_mb_double_buffer_ctrl.;*/
	/*color_mb_peaking_mb_double_buffer_ctrl.;*/
	/*color_mb_peaking_mb_double_buffer_ctrl.;*/
	/*color_mb_peaking_mb_dummy;*/

	//ROW5 VIP_MBPK_H_Deovershoot0
	color_mb_peaking_mb_peaking_ctrl.mb_de_over_h_en = ptr->MBPK_H_Deovershoot0.MB_De_Over_H_en;
	// removed color_mb_peaking_mb_de_over_hor_range_gain.mb_de_over_h_s_gain = ptr->MBPK_H_Deovershoot0.MB_De_Over_H_S_gain;
	color_mb_peaking_mb_de_over_hor_range_gain.mb_de_over_h_b_range = ptr->MBPK_H_Deovershoot0.MB_De_Over_H_B_range;
	color_mb_peaking_mb_de_over_hor_range_gain.mb_de_over_h_s_range = ptr->MBPK_H_Deovershoot0.MB_De_Over_H_S_range;

	//ROW6 VIP_MBPK_H_Deovershoot1
	color_mb_peaking_mb_peaking_de_over_h_s_0.mb_su_de_over_h_s_x0 = ptr->MBPK_H_Deovershoot1.x0;
	color_mb_peaking_mb_peaking_de_over_h_s_0.mb_su_de_over_h_s_y0 = ptr->MBPK_H_Deovershoot1.y0;
	color_mb_peaking_mb_peaking_de_over_h_s_0.mb_su_de_over_h_s_a0 = ptr->MBPK_H_Deovershoot1.a0;
	color_mb_peaking_mb_peaking_de_over_h_s_1.mb_su_de_over_h_s_x1 = ptr->MBPK_H_Deovershoot1.x1;
	color_mb_peaking_mb_peaking_de_over_h_s_1.mb_su_de_over_h_s_y1 = ptr->MBPK_H_Deovershoot1.y1;
	color_mb_peaking_mb_peaking_de_over_h_s_1.mb_su_de_over_h_s_a1 = ptr->MBPK_H_Deovershoot1.a1;
	color_mb_peaking_mb_peaking_de_over_h_s_2.mb_su_de_over_h_s_x2 = ptr->MBPK_H_Deovershoot1.x2;
	color_mb_peaking_mb_peaking_de_over_h_s_2.mb_su_de_over_h_s_y2 = ptr->MBPK_H_Deovershoot1.y2;
	color_mb_peaking_mb_peaking_de_over_h_s_2.mb_su_de_over_h_s_a2 = ptr->MBPK_H_Deovershoot1.a2;

	//ROW7 VIP_MBPK_H_Deovershoot2
	color_mb_peaking_mb_peaking_de_over_h_b_0.mb_su_de_over_h_b_x0 = ptr->MBPK_H_Deovershoot2.x0;
	color_mb_peaking_mb_peaking_de_over_h_b_0.mb_su_de_over_h_b_y0 = ptr->MBPK_H_Deovershoot2.y0;
	color_mb_peaking_mb_peaking_de_over_h_b_0.mb_su_de_over_h_b_a0 = ptr->MBPK_H_Deovershoot2.a0;
	color_mb_peaking_mb_peaking_de_over_h_b_1.mb_su_de_over_h_b_x1 = ptr->MBPK_H_Deovershoot2.x1;
	color_mb_peaking_mb_peaking_de_over_h_b_1.mb_su_de_over_h_b_y1 = ptr->MBPK_H_Deovershoot2.y1;
	color_mb_peaking_mb_peaking_de_over_h_b_1.mb_su_de_over_h_b_a1 = ptr->MBPK_H_Deovershoot2.a1;
	color_mb_peaking_mb_peaking_de_over_h_b_2.mb_su_de_over_h_b_x2 = ptr->MBPK_H_Deovershoot2.x2;
	color_mb_peaking_mb_peaking_de_over_h_b_2.mb_su_de_over_h_b_y2 = ptr->MBPK_H_Deovershoot2.y2;
	color_mb_peaking_mb_peaking_de_over_h_b_2.mb_su_de_over_h_b_a2 = ptr->MBPK_H_Deovershoot2.a2;

	//ROW8 VIP_MBPK_V_Deovershoot0
	color_mb_peaking_mb_peaking_ctrl.mb_de_over_v_en = ptr->MBPK_V_Deovershoot0.MB_De_Over_V_en;
	// removed color_mb_peaking_mb_de_over_ver_range_gain.mb_de_over_v_s_gain = ptr->MBPK_V_Deovershoot0.MB_De_Over_V_S_gain;
	color_mb_peaking_mb_de_over_ver_range_gain.mb_de_over_v_b_range = ptr->MBPK_V_Deovershoot0.MB_De_Over_V_B_range;
	color_mb_peaking_mb_de_over_ver_range_gain.mb_de_over_v_s_range = ptr->MBPK_V_Deovershoot0.MB_De_Over_V_S_range;

	//ROW9 VIP_MBPK_V_Deovershoot1
	color_mb_peaking_mb_peaking_de_over_v_s_0.mb_su_de_over_v_s_x0 = ptr->MBPK_V_Deovershoot1.x0;
	color_mb_peaking_mb_peaking_de_over_v_s_0.mb_su_de_over_v_s_y0 = ptr->MBPK_V_Deovershoot1.y0;
	color_mb_peaking_mb_peaking_de_over_v_s_0.mb_su_de_over_v_s_a0 = ptr->MBPK_V_Deovershoot1.a0;
	color_mb_peaking_mb_peaking_de_over_v_s_1.mb_su_de_over_v_s_x1 = ptr->MBPK_V_Deovershoot1.x1;
	color_mb_peaking_mb_peaking_de_over_v_s_1.mb_su_de_over_v_s_y1 = ptr->MBPK_V_Deovershoot1.y1;
	color_mb_peaking_mb_peaking_de_over_v_s_1.mb_su_de_over_v_s_a1 = ptr->MBPK_V_Deovershoot1.a1;
	color_mb_peaking_mb_peaking_de_over_v_s_2.mb_su_de_over_v_s_x2 = ptr->MBPK_V_Deovershoot1.x2;
	color_mb_peaking_mb_peaking_de_over_v_s_2.mb_su_de_over_v_s_y2 = ptr->MBPK_V_Deovershoot1.y2;
	color_mb_peaking_mb_peaking_de_over_v_s_2.mb_su_de_over_v_s_a2 = ptr->MBPK_V_Deovershoot1.a2;

	//ROW10 VIP_MBPK_V_Deovershoot2
	color_mb_peaking_mb_peaking_de_over_v_b_0.mb_su_de_over_v_b_x0 = ptr->MBPK_V_Deovershoot2.x0;
	color_mb_peaking_mb_peaking_de_over_v_b_0.mb_su_de_over_v_b_y0 = ptr->MBPK_V_Deovershoot2.y0;
	color_mb_peaking_mb_peaking_de_over_v_b_0.mb_su_de_over_v_b_a0 = ptr->MBPK_V_Deovershoot2.a0;
	color_mb_peaking_mb_peaking_de_over_v_b_1.mb_su_de_over_v_b_x1 = ptr->MBPK_V_Deovershoot2.x1;
	color_mb_peaking_mb_peaking_de_over_v_b_1.mb_su_de_over_v_b_y1 = ptr->MBPK_V_Deovershoot2.y1;
	color_mb_peaking_mb_peaking_de_over_v_b_1.mb_su_de_over_v_b_a1 = ptr->MBPK_V_Deovershoot2.a1;
	color_mb_peaking_mb_peaking_de_over_v_b_2.mb_su_de_over_v_b_x2 = ptr->MBPK_V_Deovershoot2.x2;
	color_mb_peaking_mb_peaking_de_over_v_b_2.mb_su_de_over_v_b_y2 = ptr->MBPK_V_Deovershoot2.y2;
	color_mb_peaking_mb_peaking_de_over_v_b_2.mb_su_de_over_v_b_a2 = ptr->MBPK_V_Deovershoot2.a2;

	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_CTRL_reg, color_mb_peaking_mb_peaking_ctrl.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_FILTER_0_reg, color_mb_peaking_mb_peaking_hor_filter_0.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_FILTER_1_reg, color_mb_peaking_mb_peaking_hor_filter_1.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_FILTER_2_reg, color_mb_peaking_mb_peaking_hor_filter_2.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_GAIN_reg, color_mb_peaking_mb_peaking_hor_gain.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_BOUND_reg, color_mb_peaking_mb_peaking_hor_bound.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_GAIN_2_reg, color_mb_peaking_mb_peaking_hor_gain_2.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Ver_FILTER_reg, color_mb_peaking_mb_peaking_ver_filter.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Ver_GAIN_reg, color_mb_peaking_mb_peaking_ver_gain.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Ver_BOUND_reg, color_mb_peaking_mb_peaking_ver_bound.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Ver_GAIN_2_reg, color_mb_peaking_mb_peaking_ver_gain_2.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_De_Over_Hor_RANGE_GAIN_reg, color_mb_peaking_mb_de_over_hor_range_gain.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_De_Over_Ver_RANGE_GAIN_reg, color_mb_peaking_mb_de_over_ver_range_gain.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_S_0_reg, color_mb_peaking_mb_peaking_de_over_h_s_0.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_S_1_reg, color_mb_peaking_mb_peaking_de_over_h_s_1.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_S_2_reg, color_mb_peaking_mb_peaking_de_over_h_s_2.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_B_0_reg, color_mb_peaking_mb_peaking_de_over_h_b_0.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_B_1_reg, color_mb_peaking_mb_peaking_de_over_h_b_1.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_B_2_reg, color_mb_peaking_mb_peaking_de_over_h_b_2.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_S_0_reg, color_mb_peaking_mb_peaking_de_over_v_s_0.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_S_1_reg, color_mb_peaking_mb_peaking_de_over_v_s_1.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_S_2_reg, color_mb_peaking_mb_peaking_de_over_v_s_2.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_B_0_reg, color_mb_peaking_mb_peaking_de_over_v_b_0.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_B_1_reg, color_mb_peaking_mb_peaking_de_over_v_b_1.regValue);
	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_B_2_reg, color_mb_peaking_mb_peaking_de_over_v_b_2.regValue);

	//db apply
	color_mb_peaking_mb_double_buffer_ctrl.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_Double_Buffer_CTRL_reg);
	color_mb_peaking_mb_double_buffer_ctrl.mb_db_en=1;
	color_mb_peaking_mb_double_buffer_ctrl.mb_db_apply =1;

	IoReg_Write32(COLOR_MB_SU_PEAKING_MB_SU_Double_Buffer_CTRL_reg, color_mb_peaking_mb_double_buffer_ctrl.regValue); // DB apply
}
void drvif_color_Get_MBSU_Table(VIP_MBPK_Table *ptr) /*Merlin3*/
{

	color_mb_su_peaking_mb_su_peaking_ctrl_RBUS			color_mb_peaking_mb_peaking_ctrl;
	color_mb_su_peaking_mb_su_peaking_hor_filter_0_RBUS		color_mb_peaking_mb_peaking_hor_filter_0;
	color_mb_su_peaking_mb_su_peaking_hor_filter_1_RBUS		color_mb_peaking_mb_peaking_hor_filter_1;
	color_mb_su_peaking_mb_su_peaking_hor_filter_2_RBUS		color_mb_peaking_mb_peaking_hor_filter_2;
	color_mb_su_peaking_mb_su_peaking_hor_gain_RBUS			color_mb_peaking_mb_peaking_hor_gain;
	color_mb_su_peaking_mb_su_peaking_hor_bound_RBUS		color_mb_peaking_mb_peaking_hor_bound;
	color_mb_su_peaking_mb_su_peaking_hor_gain_2_RBUS		color_mb_peaking_mb_peaking_hor_gain_2;
	color_mb_su_peaking_mb_su_peaking_ver_filter_RBUS		color_mb_peaking_mb_peaking_ver_filter;
	color_mb_su_peaking_mb_su_peaking_ver_gain_RBUS			color_mb_peaking_mb_peaking_ver_gain;
	color_mb_su_peaking_mb_su_peaking_ver_bound_RBUS		color_mb_peaking_mb_peaking_ver_bound;
	color_mb_su_peaking_mb_su_peaking_ver_gain_2_RBUS		color_mb_peaking_mb_peaking_ver_gain_2;
	//color_mb_su_peaking_mb_su_double_buffer_ctrl_RBUS		color_mb_peaking_mb_double_buffer_ctrl;
	/*color_mb_peaking_mb_dummy_RBUS					color_mb_peaking_mb_dummy;*/
	color_mb_su_peaking_mb_su_de_over_hor_range_gain_RBUS color_mb_peaking_mb_de_over_hor_range_gain;
	color_mb_su_peaking_mb_su_de_over_ver_range_gain_RBUS color_mb_peaking_mb_de_over_ver_range_gain;
	//color_mb_su_peaking_mb_su_snr_coring_RBUS color_mb_peaking_mb_snr_coring;
	color_mb_su_peaking_mb_su_peaking_de_over_h_s_0_RBUS color_mb_peaking_mb_peaking_de_over_h_s_0;
	color_mb_su_peaking_mb_su_peaking_de_over_h_s_1_RBUS color_mb_peaking_mb_peaking_de_over_h_s_1;
	color_mb_su_peaking_mb_su_peaking_de_over_h_s_2_RBUS color_mb_peaking_mb_peaking_de_over_h_s_2;
	color_mb_su_peaking_mb_su_peaking_de_over_h_b_0_RBUS color_mb_peaking_mb_peaking_de_over_h_b_0;
	color_mb_su_peaking_mb_su_peaking_de_over_h_b_1_RBUS color_mb_peaking_mb_peaking_de_over_h_b_1;
	color_mb_su_peaking_mb_su_peaking_de_over_h_b_2_RBUS color_mb_peaking_mb_peaking_de_over_h_b_2;
	color_mb_su_peaking_mb_su_peaking_de_over_v_s_0_RBUS color_mb_peaking_mb_peaking_de_over_v_s_0;
	color_mb_su_peaking_mb_su_peaking_de_over_v_s_1_RBUS color_mb_peaking_mb_peaking_de_over_v_s_1;
	color_mb_su_peaking_mb_su_peaking_de_over_v_s_2_RBUS color_mb_peaking_mb_peaking_de_over_v_s_2;
	color_mb_su_peaking_mb_su_peaking_de_over_v_b_0_RBUS color_mb_peaking_mb_peaking_de_over_v_b_0;
	color_mb_su_peaking_mb_su_peaking_de_over_v_b_1_RBUS color_mb_peaking_mb_peaking_de_over_v_b_1;
	color_mb_su_peaking_mb_su_peaking_de_over_v_b_2_RBUS color_mb_peaking_mb_peaking_de_over_v_b_2;

	if (!ptr)
		return;

	color_mb_peaking_mb_peaking_ctrl.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_CTRL_reg);
	color_mb_peaking_mb_peaking_hor_filter_0.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_FILTER_0_reg);
	color_mb_peaking_mb_peaking_hor_filter_1.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_FILTER_1_reg);
	color_mb_peaking_mb_peaking_hor_filter_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_FILTER_2_reg);
	color_mb_peaking_mb_peaking_hor_gain.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_GAIN_reg);
	color_mb_peaking_mb_peaking_hor_bound.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_BOUND_reg);
	color_mb_peaking_mb_peaking_hor_gain_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Hor_GAIN_2_reg);
	color_mb_peaking_mb_peaking_ver_filter.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Ver_FILTER_reg);
	color_mb_peaking_mb_peaking_ver_gain.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Ver_GAIN_reg);
	color_mb_peaking_mb_peaking_ver_bound.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Ver_BOUND_reg);
	color_mb_peaking_mb_peaking_ver_gain_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_Ver_GAIN_2_reg);
	color_mb_peaking_mb_de_over_hor_range_gain.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_De_Over_Hor_RANGE_GAIN_reg);
	color_mb_peaking_mb_de_over_ver_range_gain.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_De_Over_Ver_RANGE_GAIN_reg);
	color_mb_peaking_mb_peaking_de_over_h_s_0.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_S_0_reg);
	color_mb_peaking_mb_peaking_de_over_h_s_1.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_S_1_reg);
	color_mb_peaking_mb_peaking_de_over_h_s_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_S_2_reg);
	color_mb_peaking_mb_peaking_de_over_h_b_0.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_B_0_reg);
	color_mb_peaking_mb_peaking_de_over_h_b_1.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_B_1_reg);
	color_mb_peaking_mb_peaking_de_over_h_b_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_H_B_2_reg);
	color_mb_peaking_mb_peaking_de_over_v_s_0.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_S_0_reg);
	color_mb_peaking_mb_peaking_de_over_v_s_1.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_S_1_reg);
	color_mb_peaking_mb_peaking_de_over_v_s_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_S_2_reg);
	color_mb_peaking_mb_peaking_de_over_v_b_0.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_B_0_reg);
	color_mb_peaking_mb_peaking_de_over_v_b_1.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_B_1_reg);
	color_mb_peaking_mb_peaking_de_over_v_b_2.regValue = IoReg_Read32(COLOR_MB_SU_PEAKING_MB_SU_PEAKING_De_Over_V_B_2_reg);



	ptr->MBPK_Ctrl.MB_V_En = color_mb_peaking_mb_peaking_ctrl.mb_v_en;
	ptr->MBPK_Ctrl.MB_H_En = color_mb_peaking_mb_peaking_ctrl.mb_h_en;
	ptr->MBPK_Ctrl.mb_peaking_en = color_mb_peaking_mb_peaking_ctrl.mb_peaking_en;
	ptr->MBPK_Ctrl.MB_De_Over_HVmin_en = color_mb_peaking_mb_peaking_ctrl.mb_de_over_hvmin_en ;
	ptr->MBPK_Ctrl.MB_De_Over_BSmaxmin = color_mb_peaking_mb_peaking_ctrl.mb_de_over_bsmaxmin ;

	//ROW1 VIP_MBPK_H_Coef
	/*DC Value Check*/
	color_mb_peaking_mb_peaking_hor_filter_0.mb_hor_filter_c0 = ptr->MBPK_H_Coef.MB_Hor_Filter_C0;
	color_mb_peaking_mb_peaking_hor_filter_0.mb_hor_filter_c1 = ptr->MBPK_H_Coef.MB_Hor_Filter_C1;
	color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c2 = ptr->MBPK_H_Coef.MB_Hor_Filter_C2;
	color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c3 = ptr->MBPK_H_Coef.MB_Hor_Filter_C3;
	color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c4 = ptr->MBPK_H_Coef.MB_Hor_Filter_C4;
	color_mb_peaking_mb_peaking_hor_filter_1.mb_hor_filter_c5 = ptr->MBPK_H_Coef.MB_Hor_Filter_C5;
	color_mb_peaking_mb_peaking_hor_filter_2.mb_hor_filter_c6 = ptr->MBPK_H_Coef.MB_Hor_Filter_C6;
	color_mb_peaking_mb_peaking_ctrl.mb_h_mode = ptr->MBPK_H_Coef.mb_h_mode;

	//ROW2 VIP_MBPK_H_Table
	ptr->MBPK_H_Table.MB_Gain_Neg =	color_mb_peaking_mb_peaking_hor_gain.mb_gain_neg;
	ptr->MBPK_H_Table.MB_Gain_Pos =	color_mb_peaking_mb_peaking_hor_gain.mb_gain_pos;
	ptr->MBPK_H_Table.MB_HV_Neg = color_mb_peaking_mb_peaking_hor_bound.mb_hv_neg;
	ptr->MBPK_H_Table.MB_HV_Pos = color_mb_peaking_mb_peaking_hor_bound.mb_hv_pos;
	ptr->MBPK_H_Table.MB_D2_shift_bit = color_mb_peaking_mb_peaking_hor_gain_2.mb_d2_shift_bit;
	ptr->MBPK_H_Table.MB_Gain_Neg2 = color_mb_peaking_mb_peaking_hor_gain_2.mb_gain_neg2;
	ptr->MBPK_H_Table.MB_Gain_Pos2 = color_mb_peaking_mb_peaking_hor_gain_2.mb_gain_pos2;
	ptr->MBPK_H_Table.MB_LV = color_mb_peaking_mb_peaking_hor_gain_2.mb_lv;
	ptr->MBPK_H_Table.MB_LV2 = color_mb_peaking_mb_peaking_hor_gain_2.mb_lv2;

	ptr->MBPK_V_Coef.MB_Ver_Filter_C0 = color_mb_peaking_mb_peaking_ver_filter.mb_ver_filter_c0;
	ptr->MBPK_V_Coef.MB_Ver_Filter_C1 = color_mb_peaking_mb_peaking_ver_filter.mb_ver_filter_c1;
	ptr->MBPK_V_Coef.MB_Ver_Filter_C2 = color_mb_peaking_mb_peaking_ver_filter.mb_ver_filter_c2;

	//ROW4 VIP_MBPK_V_Table
	ptr->MBPK_V_Table.MB_Gain_Neg_V = color_mb_peaking_mb_peaking_ver_gain.mb_gain_neg_v;
	ptr->MBPK_V_Table.MB_Gain_Pos_V = color_mb_peaking_mb_peaking_ver_gain.mb_gain_pos_v;
	ptr->MBPK_V_Table.MB_HV_Neg_V = color_mb_peaking_mb_peaking_ver_bound.mb_hv_neg_v;
	ptr->MBPK_V_Table.MB_HV_Pos_V = color_mb_peaking_mb_peaking_ver_bound.mb_hv_pos_v;
	ptr->MBPK_V_Table.MB_D2_shift_bit_V = color_mb_peaking_mb_peaking_ver_gain_2.mb_d2_shift_bit_v;
	ptr->MBPK_V_Table.MB_Gain_Neg2_V = color_mb_peaking_mb_peaking_ver_gain_2.mb_gain_neg2_v;
	ptr->MBPK_V_Table.MB_Gain_Pos2_V = color_mb_peaking_mb_peaking_ver_gain_2.mb_gain_pos2_v;
	ptr->MBPK_V_Table.MB_LV2_V = color_mb_peaking_mb_peaking_ver_gain_2.mb_lv2_v;
	ptr->MBPK_V_Table.MB_LV_V = color_mb_peaking_mb_peaking_ver_gain_2.mb_lv_v;

	//ROW5 VIP_MBPK_H_Deovershoot0
	ptr->MBPK_H_Deovershoot0.MB_De_Over_H_en = color_mb_peaking_mb_peaking_ctrl.mb_de_over_h_en;
	ptr->MBPK_H_Deovershoot0.MB_De_Over_H_B_range = color_mb_peaking_mb_de_over_hor_range_gain.mb_de_over_h_b_range;
	ptr->MBPK_H_Deovershoot0.MB_De_Over_H_S_range = color_mb_peaking_mb_de_over_hor_range_gain.mb_de_over_h_s_range;

													 //ROW6 VIP_MBPK_H_Deovershoot1
	ptr->MBPK_H_Deovershoot1.x0 = color_mb_peaking_mb_peaking_de_over_h_s_0.mb_su_de_over_h_s_x0;
	ptr->MBPK_H_Deovershoot1.y0 = color_mb_peaking_mb_peaking_de_over_h_s_0.mb_su_de_over_h_s_y0;
	ptr->MBPK_H_Deovershoot1.a0 = color_mb_peaking_mb_peaking_de_over_h_s_0.mb_su_de_over_h_s_a0;
	ptr->MBPK_H_Deovershoot1.x1 = color_mb_peaking_mb_peaking_de_over_h_s_1.mb_su_de_over_h_s_x1;
	ptr->MBPK_H_Deovershoot1.y1 = color_mb_peaking_mb_peaking_de_over_h_s_1.mb_su_de_over_h_s_y1;
	ptr->MBPK_H_Deovershoot1.a1 = color_mb_peaking_mb_peaking_de_over_h_s_1.mb_su_de_over_h_s_a1;
	ptr->MBPK_H_Deovershoot1.x2 = color_mb_peaking_mb_peaking_de_over_h_s_2.mb_su_de_over_h_s_x2;
	ptr->MBPK_H_Deovershoot1.y2 = color_mb_peaking_mb_peaking_de_over_h_s_2.mb_su_de_over_h_s_y2;
	ptr->MBPK_H_Deovershoot1.a2 = color_mb_peaking_mb_peaking_de_over_h_s_2.mb_su_de_over_h_s_a2;

												   //ROW7 VIP_MBPK_H_Deovershoot2
	ptr->MBPK_H_Deovershoot2.x0 = color_mb_peaking_mb_peaking_de_over_h_b_0.mb_su_de_over_h_b_x0;
	ptr->MBPK_H_Deovershoot2.y0 = color_mb_peaking_mb_peaking_de_over_h_b_0.mb_su_de_over_h_b_y0;
	ptr->MBPK_H_Deovershoot2.a0 = color_mb_peaking_mb_peaking_de_over_h_b_0.mb_su_de_over_h_b_a0;
	ptr->MBPK_H_Deovershoot2.x1 = color_mb_peaking_mb_peaking_de_over_h_b_1.mb_su_de_over_h_b_x1;
	ptr->MBPK_H_Deovershoot2.y1 = color_mb_peaking_mb_peaking_de_over_h_b_1.mb_su_de_over_h_b_y1;
	ptr->MBPK_H_Deovershoot2.a1 = color_mb_peaking_mb_peaking_de_over_h_b_1.mb_su_de_over_h_b_a1;
	ptr->MBPK_H_Deovershoot2.x2 = color_mb_peaking_mb_peaking_de_over_h_b_2.mb_su_de_over_h_b_x2;
	ptr->MBPK_H_Deovershoot2.y2 = color_mb_peaking_mb_peaking_de_over_h_b_2.mb_su_de_over_h_b_y2;
	ptr->MBPK_H_Deovershoot2.a2 = color_mb_peaking_mb_peaking_de_over_h_b_2.mb_su_de_over_h_b_a2;

	//ROW8 VIP_MBPK_V_Deovershoot0
	color_mb_peaking_mb_peaking_ctrl.mb_de_over_v_en = ptr->MBPK_V_Deovershoot0.MB_De_Over_V_en;
	color_mb_peaking_mb_de_over_ver_range_gain.mb_de_over_v_b_range = ptr->MBPK_V_Deovershoot0.MB_De_Over_V_B_range;
	color_mb_peaking_mb_de_over_ver_range_gain.mb_de_over_v_s_range = ptr->MBPK_V_Deovershoot0.MB_De_Over_V_S_range;

	//ROW9 VIP_MBPK_V_Deovershoot1
	ptr->MBPK_V_Deovershoot1.x0 = color_mb_peaking_mb_peaking_de_over_v_s_0.mb_su_de_over_v_s_x0;
	ptr->MBPK_V_Deovershoot1.y0 = color_mb_peaking_mb_peaking_de_over_v_s_0.mb_su_de_over_v_s_y0;
	ptr->MBPK_V_Deovershoot1.a0 = color_mb_peaking_mb_peaking_de_over_v_s_0.mb_su_de_over_v_s_a0;
	ptr->MBPK_V_Deovershoot1.x1 = color_mb_peaking_mb_peaking_de_over_v_s_1.mb_su_de_over_v_s_x1;
	ptr->MBPK_V_Deovershoot1.y1 = color_mb_peaking_mb_peaking_de_over_v_s_1.mb_su_de_over_v_s_y1;
	ptr->MBPK_V_Deovershoot1.a1 = color_mb_peaking_mb_peaking_de_over_v_s_1.mb_su_de_over_v_s_a1;
	ptr->MBPK_V_Deovershoot1.x2 = color_mb_peaking_mb_peaking_de_over_v_s_2.mb_su_de_over_v_s_x2;
	ptr->MBPK_V_Deovershoot1.y2 = color_mb_peaking_mb_peaking_de_over_v_s_2.mb_su_de_over_v_s_y2;
	ptr->MBPK_V_Deovershoot1.a2 = color_mb_peaking_mb_peaking_de_over_v_s_2.mb_su_de_over_v_s_a2;

							//ROW10 VIP_MBPK_V_Deovershoot2
	ptr->MBPK_V_Deovershoot2.x0 = color_mb_peaking_mb_peaking_de_over_v_b_0.mb_su_de_over_v_b_x0;
	ptr->MBPK_V_Deovershoot2.y0 = color_mb_peaking_mb_peaking_de_over_v_b_0.mb_su_de_over_v_b_y0;
	ptr->MBPK_V_Deovershoot2.a0 = color_mb_peaking_mb_peaking_de_over_v_b_0.mb_su_de_over_v_b_a0;
	ptr->MBPK_V_Deovershoot2.x1 = color_mb_peaking_mb_peaking_de_over_v_b_1.mb_su_de_over_v_b_x1;
	ptr->MBPK_V_Deovershoot2.y1 = color_mb_peaking_mb_peaking_de_over_v_b_1.mb_su_de_over_v_b_y1;
	ptr->MBPK_V_Deovershoot2.a1 = color_mb_peaking_mb_peaking_de_over_v_b_1.mb_su_de_over_v_b_a1;
	ptr->MBPK_V_Deovershoot2.x2 = color_mb_peaking_mb_peaking_de_over_v_b_2.mb_su_de_over_v_b_x2;
	ptr->MBPK_V_Deovershoot2.y2 = color_mb_peaking_mb_peaking_de_over_v_b_2.mb_su_de_over_v_b_y2;
	ptr->MBPK_V_Deovershoot2.a2 = color_mb_peaking_mb_peaking_de_over_v_b_2.mb_su_de_over_v_b_a2;


}

void drvif_color_Sharpness_Enable(unsigned char bEnable)
{
	color_sharp_dm_nr_shp_ctrl_RBUS color_sharp_dm_nr_shp_ctrl_reg;
	color_sharp_dm_nr_shp_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_NR_SHP_CTRL_reg);
	color_sharp_dm_nr_shp_ctrl_reg.peaking_enable = bEnable;
	IoReg_Write32(COLOR_SHARP_DM_NR_SHP_CTRL_reg, color_sharp_dm_nr_shp_ctrl_reg.regValue);
}


void drvif_color_Get_Sharpness_Table(DRV_Sharpness_Table *ptr)
{
	color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
    color_sharp_dm_sr_ctrl_2_RBUS color_sharp_dm_sr_ctrl_2;
    color_sharp_dm_sr_ctrl_3_RBUS color_sharp_dm_sr_ctrl_3;

	if (!ptr)
		return;

	color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);
    color_sharp_dm_sr_ctrl_2.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_2_reg);
    color_sharp_dm_sr_ctrl_3.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_3_reg);

	{
	color_sharp_dm_nr_shp_ctrl_RBUS color_sharp_dm_nr_shp_ctrl;
	color_sharp_dm_mkiii_shp_ctrl_RBUS	color_sharp_dm_mkiii_shp_ctrl;
	color_sharp_dm_segpk_ctrl1_RBUS color_sharp_dm_segpk_ctrl1;
	color_sharp_dm_segpk_ctrl2_RBUS color_sharp_dm_segpk_ctrl2;
	color_sharp_dm_segpk_ctrl3_RBUS color_sharp_dm_segpk_ctrl3;
	color_sharp_dm_segpk_ctrl4_RBUS color_sharp_dm_segpk_ctrl4;
	color_sharp_dm_segpk_edge_mono_RBUS color_sharp_dm_segpk_edge_mono;
	color_sharp_dm_segpk_dbg_RBUS	color_sharp_dm_segpk_dbg;
	color_sharp_dm_2d_shp_y_remap_RBUS	color_sharp_dm_2d_shp_y_remap;
	color_sharp_dm_peaking_gain_RBUS color_sharp_dm_peaking_gain;
	color_sharp_dm_peaking_bound_1_RBUS color_sharp_dm_peaking_bound_1;
	color_sharp_dm_peaking_bound_0_RBUS color_sharp_dm_peaking_bound_0;
	color_sharp_dm_2d_shp_tex_0_RBUS	color_sharp_dm_2d_shp_tex_0;
	color_sharp_dm_2d_shp_tex_1_RBUS	color_sharp_dm_2d_shp_tex_1;
	color_sharp_dm_2d_shp_tex_1_1_RBUS	color_sharp_dm_2d_shp_tex_1_1;
	color_sharp_dm_2d_shp_tex_2_RBUS	color_sharp_dm_2d_shp_tex_2;
	color_sharp_dm_2d_shp_tex_4_RBUS	color_sharp_dm_2d_shp_tex_4;
	color_sharp_dm_2d_shp_tex_6_RBUS	color_sharp_dm_2d_shp_tex_6;
	color_sharp_dm_2d_shp_tex_7_RBUS	color_sharp_dm_2d_shp_tex_7;
	color_sharp_dm_peaking_emf_texture_RBUS color_sharp_dm_peaking_emf_texture;
	color_sharp_dm_emfmk2_ctrl_texture_RBUS color_sharp_dm_emfmk2_ctrl_texture;
	color_sharp_dm_emf_ext_0_texture_RBUS color_sharp_dm_emf_ext_0_texture;
	color_sharp_dm_emf_ext_1_texture_RBUS color_sharp_dm_emf_ext_1_texture;
	color_sharp_dm_emf_ext_2_texture_RBUS color_sharp_dm_emf_ext_2_texture;
	color_sharp_dm_auto_shp_data_RBUS	color_sharp_dm_auto_shp_data;
	color_sharp_dm_auto_shp_fir_count_00_RBUS	color_sharp_dm_auto_shp_fir_count_00;
	color_sharp_dm_auto_shp_fir_count_01_RBUS	color_sharp_dm_auto_shp_fir_count_01;
	color_sharp_dm_auto_shp_fir_count_02_RBUS	color_sharp_dm_auto_shp_fir_count_02;
	color_sharp_dm_auto_shp_fir_count_03_RBUS	color_sharp_dm_auto_shp_fir_count_03;
	color_sharp_dm_auto_shp_fir_count_04_RBUS	color_sharp_dm_auto_shp_fir_count_04;
	color_sharp_dm_auto_shp_fir_count_05_RBUS	color_sharp_dm_auto_shp_fir_count_05;
	color_sharp_dm_auto_shp_fir_count_06_RBUS	color_sharp_dm_auto_shp_fir_count_06;
	color_sharp_dm_auto_shp_fir_count_07_RBUS	color_sharp_dm_auto_shp_fir_count_07;
	color_sharp_dm_auto_shp_fir_count_08_RBUS	color_sharp_dm_auto_shp_fir_count_08;
	color_sharp_dm_auto_shp_fir_count_09_RBUS	color_sharp_dm_auto_shp_fir_count_09;
	color_sharp_dm_auto_shp_fir_count_10_RBUS	color_sharp_dm_auto_shp_fir_count_10;
	color_sharp_dm_auto_shp_fir_count_11_RBUS	color_sharp_dm_auto_shp_fir_count_11;
	color_sharp_dm_auto_shp_fir_count_12_RBUS	color_sharp_dm_auto_shp_fir_count_12;
	color_sharp_dm_auto_shp_fir_count_13_RBUS	color_sharp_dm_auto_shp_fir_count_13;
	color_sharp_dm_auto_shp_fir_count_14_RBUS	color_sharp_dm_auto_shp_fir_count_14;
	color_sharp_dm_auto_shp_fir_count_15_RBUS	color_sharp_dm_auto_shp_fir_count_15;
	color_sharp_dm_segpk_edgpk3_RBUS	color_sharp_dm_segpk_edgpk3;
	color_sharp_dm_segpk_edgpk4_RBUS	color_sharp_dm_segpk_edgpk4;
	color_sharp_dm_segpk_edgpk2_RBUS	color_sharp_dm_segpk_edgpk2;
	color_sharp_dm_2d_shp_edge_0_RBUS color_sharp_dm_2d_shp_edge_0;
	color_sharp_dm_2d_shp_edge_1_RBUS color_sharp_dm_2d_shp_edge_1;
	color_sharp_dm_2d_shp_edge_1_1_RBUS color_sharp_dm_2d_shp_edge_1_1;
	color_sharp_dm_2d_shp_edge_2_RBUS color_sharp_dm_2d_shp_edge_2;
	color_sharp_dm_2d_shp_edge_4_RBUS color_sharp_dm_2d_shp_edge_4;
	color_sharp_dm_2d_shp_edge_6_RBUS	color_sharp_dm_2d_shp_edge_6;
	color_sharp_dm_2d_shp_edge_7_RBUS	color_sharp_dm_2d_shp_edge_7;
	color_sharp_dm_peaking_emf_RBUS color_sharp_dm_peaking_emf;
	color_sharp_dm_emfmk2_ctrl_RBUS color_sharp_dm_emfmk2_ctrl;
	color_sharp_dm_emf_ext_0_RBUS	color_sharp_dm_emf_ext_0;
	color_sharp_dm_emf_ext_1_RBUS	color_sharp_dm_emf_ext_1;
	color_sharp_dm_segpk_edge_coring_0_RBUS color_sharp_dm_segpk_edge_coring_0;
	color_sharp_dm_segpk_edge_coring_1_RBUS color_sharp_dm_segpk_edge_coring_1;
	color_sharp_dm_peaking_h_lvd_RBUS color_sharp_dm_peaking_h_lvd;
	color_sharp_dm_peaking_v_lvd_RBUS color_sharp_dm_peaking_v_lvd;
	color_sharp_dm_segpk_edgpk_sourceslope_RBUS color_sharp_dm_segpk_edgpk_sourceslope;
	color_sharp_dm_segpk_edgpk_sourceslope_flat_RBUS	color_sharp_dm_segpk_edgpk_sourceslope_flat;

	color_sharp_dm_segpk_vpk1_RBUS	color_sharp_dm_segpk_vpk1;
	color_sharp_dm_segpk_vpk2_RBUS	color_sharp_dm_segpk_vpk2;
	color_sharp_dm_segpk_vpk3_RBUS	color_sharp_dm_segpk_vpk3;
	color_sharp_dm_segpk_vpk4_RBUS	color_sharp_dm_segpk_vpk4;
	color_sharp_dm_segpk_vpk5_RBUS	color_sharp_dm_segpk_vpk5;
	color_sharp_dm_sours_lpf_0_RBUS color_sharp_dm_sours_lpf_0;
	color_sharp_dm_sours_lpf_1_RBUS color_sharp_dm_sours_lpf_1;
	color_sharp_dm_maxmin_shp_gain_0_RBUS color_sharp_dm_maxmin_shp_gain_0;
	color_sharp_dm_maxmin_shp_gain_1_RBUS color_sharp_dm_maxmin_shp_gain_1;
	color_sharp_dm_maxmin_shp_gain_2_RBUS color_sharp_dm_maxmin_shp_gain_2;
	color_sharp_dm_maxmin_shp_gain_3_RBUS color_sharp_dm_maxmin_shp_gain_3;
	color_sharp_dm_maxmin_shp_step_0_RBUS color_sharp_dm_maxmin_shp_step_0;
	color_sharp_dm_maxmin_shp_step_1_RBUS color_sharp_dm_maxmin_shp_step_1;
	color_sharp_dm_maxmin_shp_step_2_RBUS color_sharp_dm_maxmin_shp_step_2;
	color_sharp_dm_maxmin_shp_step_3_RBUS color_sharp_dm_maxmin_shp_step_3;
	color_sharp_dm_vpk_hpk_add_RBUS color_sharp_dm_vpk_hpk_add;
	color_sharp_dm_vc_after_filter_0_RBUS	color_sharp_dm_vc_after_filter_0;
	color_sharp_dm_vc_after_filter_1_RBUS	color_sharp_dm_vc_after_filter_1;
	color_sharp_dm_vertical_pos_gain_curve_gain_0_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_0;
	color_sharp_dm_vertical_pos_gain_curve_gain_1_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_1;
	color_sharp_dm_vertical_pos_gain_curve_gain_2_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_2;
	color_sharp_dm_vertical_pos_gain_curve_gain_3_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_3;
	color_sharp_dm_vertical_pos_gain_curve_gain_4_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_4;
	color_sharp_dm_vertical_pos_gain_curve_gain_5_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_5;
	color_sharp_dm_vertical_pos_gain_curve_gain_6_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_6;
	color_sharp_dm_vertical_pos_gain_curve_gain_7_RBUS	color_sharp_dm_vertical_pos_gain_curve_gain_7;
	color_sharp_dm_vertical_pos_gain_curve_step_0_RBUS	color_sharp_dm_vertical_pos_gain_curve_step_0;
	color_sharp_dm_vertical_pos_gain_curve_step_1_RBUS	color_sharp_dm_vertical_pos_gain_curve_step_1;
	color_sharp_dm_vertical_pos_gain_curve_step_2_RBUS	color_sharp_dm_vertical_pos_gain_curve_step_2;
	color_sharp_dm_vertical_pos_gain_curve_step_3_RBUS	color_sharp_dm_vertical_pos_gain_curve_step_3;
	color_sharp_dm_vertical_pos_gain_curve_step_4_RBUS	color_sharp_dm_vertical_pos_gain_curve_step_4;
	color_sharp_dm_vertical_neg_gain_curve_gain_0_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_0;
	color_sharp_dm_vertical_neg_gain_curve_gain_1_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_1;
	color_sharp_dm_vertical_neg_gain_curve_gain_2_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_2;
	color_sharp_dm_vertical_neg_gain_curve_gain_3_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_3;
	color_sharp_dm_vertical_neg_gain_curve_gain_4_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_4;
	color_sharp_dm_vertical_neg_gain_curve_gain_5_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_5;
	color_sharp_dm_vertical_neg_gain_curve_gain_6_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_6;
	color_sharp_dm_vertical_neg_gain_curve_gain_7_RBUS	color_sharp_dm_vertical_neg_gain_curve_gain_7;
	color_sharp_dm_vertical_neg_gain_curve_step_0_RBUS	color_sharp_dm_vertical_neg_gain_curve_step_0;
	color_sharp_dm_vertical_neg_gain_curve_step_1_RBUS	color_sharp_dm_vertical_neg_gain_curve_step_1;
	color_sharp_dm_vertical_neg_gain_curve_step_2_RBUS	color_sharp_dm_vertical_neg_gain_curve_step_2;
	color_sharp_dm_vertical_neg_gain_curve_step_3_RBUS	color_sharp_dm_vertical_neg_gain_curve_step_3;
	color_sharp_dm_vertical_neg_gain_curve_step_4_RBUS	color_sharp_dm_vertical_neg_gain_curve_step_4;
	color_sharp_dm_gain_curve_debug_RBUS	color_sharp_dm_gain_curve_debug;

	color_sharp_dm_maxmin_lpf_weit_0_RBUS color_sharp_dm_maxmin_lpf_weit_0;
	color_sharp_dm_maxmin_lpf_weit_1_RBUS color_sharp_dm_maxmin_lpf_weit_1;
	color_sharp_dm_maxmin_lpf_weit_2_RBUS color_sharp_dm_maxmin_lpf_weit_2;
	color_sharp_dm_maxmin_lpf_weit_3_RBUS color_sharp_dm_maxmin_lpf_weit_3;
	color_sharp_dm_maxmin_lpf_step_0_RBUS color_sharp_dm_maxmin_lpf_step_0;
	color_sharp_dm_maxmin_lpf_step_1_RBUS color_sharp_dm_maxmin_lpf_step_1;
	color_sharp_dm_maxmin_lpf_step_2_RBUS color_sharp_dm_maxmin_lpf_step_2;
	color_sharp_dm_maxmin_lpf_step_3_RBUS color_sharp_dm_maxmin_lpf_step_3;
	color_sharp_dm_maxmin_gain_by_y_gain_0_RBUS color_sharp_dm_maxmin_gain_by_y_gain_0;
	color_sharp_dm_maxmin_gain_by_y_gain_1_RBUS color_sharp_dm_maxmin_gain_by_y_gain_1;
	color_sharp_dm_maxmin_gain_by_y_gain_2_RBUS color_sharp_dm_maxmin_gain_by_y_gain_2;
	color_sharp_dm_maxmin_gain_by_y_gain_3_RBUS color_sharp_dm_maxmin_gain_by_y_gain_3;
	color_sharp_dm_maxmin_gain_by_y_step_0_RBUS color_sharp_dm_maxmin_gain_by_y_step_0;
	color_sharp_dm_maxmin_gain_by_y_step_1_RBUS color_sharp_dm_maxmin_gain_by_y_step_1;
	color_sharp_dm_maxmin_gain_by_y_step_2_RBUS color_sharp_dm_maxmin_gain_by_y_step_2;
	color_sharp_dm_maxmin_gain_by_y_step_3_RBUS color_sharp_dm_maxmin_gain_by_y_step_3;
	color_sharp_dm_ramp_peaking_0_RBUS color_sharp_dm_ramp_peaking_0;
	color_sharp_dm_ramp_peaking_1_RBUS color_sharp_dm_ramp_peaking_1;
	color_sharp_dm_ramp_peaking_2_RBUS color_sharp_dm_ramp_peaking_2;
	color_sharp_dm_ramp_peaking_3_RBUS color_sharp_dm_ramp_peaking_3;

	color_sharp_dm_sr_continual_detail_gain_RBUS	color_sharp_dm_sr_continual_detail_gain;
	color_sharp_dm_sr_9tap_hpf_h_coef_1_RBUS	color_sharp_dm_sr_9tap_hpf_h_coef_1;
	color_sharp_dm_sr_13tap_hpf_h_coef_0_RBUS	color_sharp_dm_sr_13tap_hpf_h_coef_0;
	color_sharp_dm_sr_13tap_hpf_h_coef_1_RBUS	color_sharp_dm_sr_13tap_hpf_h_coef_1;
	color_sharp_dm_sr_21tap_hpf_h_coef_0_RBUS	color_sharp_dm_sr_21tap_hpf_h_coef_0;
	color_sharp_dm_sr_21tap_hpf_h_coef_1_RBUS	color_sharp_dm_sr_21tap_hpf_h_coef_1;
	color_sharp_dm_sr_21tap_hpf_h_coef_2_RBUS	color_sharp_dm_sr_21tap_hpf_h_coef_2;
	color_sharp_dm_sr_9tap_hpf_v_coef_0_1_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_0_1;
	color_sharp_dm_sr_9tap_hpf_v_coef_1_0_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_1_0;
	color_sharp_dm_sr_9tap_hpf_v_coef_1_1_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_1_1;
	color_sharp_dm_sr_9tap_hpf_v_coef_2_0_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_2_0;
	color_sharp_dm_sr_9tap_hpf_v_coef_2_1_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_2_1;
	color_sharp_dm_sr_11tap_hpf_h_coef_0_RBUS	color_sharp_dm_sr_11tap_hpf_h_coef_0;
	color_sharp_dm_sr_11tap_hpf_h_coef_1_RBUS	color_sharp_dm_sr_11tap_hpf_h_coef_1;
	color_sharp_dm_sr_bias_h_coef_RBUS color_sharp_dm_sr_bias_h_coef;
	color_sharp_dm_sr_9tap_hpf_v_coef_3_0_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_3_0;
	color_sharp_dm_sr_9tap_hpf_v_coef_3_1_RBUS	color_sharp_dm_sr_9tap_hpf_v_coef_3_1;
	color_sharp_dm_sr_bias_v_coef_RBUS color_sharp_dm_sr_bias_v_coef;
	color_sharp_dm_sr_x_coef_0_RBUS	color_sharp_dm_sr_x_coef_0;
	color_sharp_dm_sr_x_coef_1_RBUS	color_sharp_dm_sr_x_coef_1;
	color_sharp_dm_edge_pos_gain_curve_gain_0_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_0;
	color_sharp_dm_edge_pos_gain_curve_gain_1_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_1;
	color_sharp_dm_edge_pos_gain_curve_gain_2_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_2;
	color_sharp_dm_edge_pos_gain_curve_gain_3_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_3;
	color_sharp_dm_edge_pos_gain_curve_gain_4_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_4;
	color_sharp_dm_edge_pos_gain_curve_gain_5_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_5;
	color_sharp_dm_edge_pos_gain_curve_gain_6_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_6;
	color_sharp_dm_edge_pos_gain_curve_gain_7_RBUS	color_sharp_dm_edge_pos_gain_curve_gain_7;
	color_sharp_dm_edge_pos_gain_curve_step_0_RBUS	color_sharp_dm_edge_pos_gain_curve_step_0;
	color_sharp_dm_edge_pos_gain_curve_step_1_RBUS	color_sharp_dm_edge_pos_gain_curve_step_1;
	color_sharp_dm_edge_pos_gain_curve_step_2_RBUS	color_sharp_dm_edge_pos_gain_curve_step_2;
	color_sharp_dm_edge_pos_gain_curve_step_3_RBUS	color_sharp_dm_edge_pos_gain_curve_step_3;
	color_sharp_dm_edge_pos_gain_curve_step_4_RBUS	color_sharp_dm_edge_pos_gain_curve_step_4;
	color_sharp_dm_edge_neg_gain_curve_gain_0_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_0;
	color_sharp_dm_edge_neg_gain_curve_gain_1_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_1;
	color_sharp_dm_edge_neg_gain_curve_gain_2_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_2;
	color_sharp_dm_edge_neg_gain_curve_gain_3_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_3;
	color_sharp_dm_edge_neg_gain_curve_gain_4_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_4;
	color_sharp_dm_edge_neg_gain_curve_gain_5_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_5;
	color_sharp_dm_edge_neg_gain_curve_gain_6_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_6;
	color_sharp_dm_edge_neg_gain_curve_gain_7_RBUS	color_sharp_dm_edge_neg_gain_curve_gain_7;
	color_sharp_dm_edge_neg_gain_curve_step_0_RBUS	color_sharp_dm_edge_neg_gain_curve_step_0;
	color_sharp_dm_edge_neg_gain_curve_step_1_RBUS	color_sharp_dm_edge_neg_gain_curve_step_1;
	color_sharp_dm_edge_neg_gain_curve_step_2_RBUS	color_sharp_dm_edge_neg_gain_curve_step_2;
	color_sharp_dm_edge_neg_gain_curve_step_3_RBUS	color_sharp_dm_edge_neg_gain_curve_step_3;
	color_sharp_dm_edge_neg_gain_curve_step_4_RBUS	color_sharp_dm_edge_neg_gain_curve_step_4;
	color_sharp_dm_tex_pos_gain_curve_gain_0_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_0;
	color_sharp_dm_tex_pos_gain_curve_gain_1_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_1;
	color_sharp_dm_tex_pos_gain_curve_gain_2_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_2;
	color_sharp_dm_tex_pos_gain_curve_gain_3_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_3;
	color_sharp_dm_tex_pos_gain_curve_gain_4_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_4;
	color_sharp_dm_tex_pos_gain_curve_gain_5_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_5;
	color_sharp_dm_tex_pos_gain_curve_gain_6_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_6;
	color_sharp_dm_tex_pos_gain_curve_gain_7_RBUS	color_sharp_dm_tex_pos_gain_curve_gain_7;
	color_sharp_dm_tex_pos_gain_curve_step_0_RBUS	color_sharp_dm_tex_pos_gain_curve_step_0;
	color_sharp_dm_tex_pos_gain_curve_step_1_RBUS	color_sharp_dm_tex_pos_gain_curve_step_1;
	color_sharp_dm_tex_pos_gain_curve_step_2_RBUS	color_sharp_dm_tex_pos_gain_curve_step_2;
	color_sharp_dm_tex_pos_gain_curve_step_3_RBUS	color_sharp_dm_tex_pos_gain_curve_step_3;
	color_sharp_dm_tex_pos_gain_curve_step_4_RBUS	color_sharp_dm_tex_pos_gain_curve_step_4;
	color_sharp_dm_tex_neg_gain_curve_gain_0_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_0;
	color_sharp_dm_tex_neg_gain_curve_gain_1_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_1;
	color_sharp_dm_tex_neg_gain_curve_gain_2_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_2;
	color_sharp_dm_tex_neg_gain_curve_gain_3_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_3;
	color_sharp_dm_tex_neg_gain_curve_gain_4_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_4;
	color_sharp_dm_tex_neg_gain_curve_gain_5_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_5;
	color_sharp_dm_tex_neg_gain_curve_gain_6_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_6;
	color_sharp_dm_tex_neg_gain_curve_gain_7_RBUS	color_sharp_dm_tex_neg_gain_curve_gain_7;
	color_sharp_dm_tex_neg_gain_curve_step_0_RBUS	color_sharp_dm_tex_neg_gain_curve_step_0;
	color_sharp_dm_tex_neg_gain_curve_step_1_RBUS	color_sharp_dm_tex_neg_gain_curve_step_1;
	color_sharp_dm_tex_neg_gain_curve_step_2_RBUS	color_sharp_dm_tex_neg_gain_curve_step_2;
	color_sharp_dm_tex_neg_gain_curve_step_3_RBUS	color_sharp_dm_tex_neg_gain_curve_step_3;
	color_sharp_dm_tex_neg_gain_curve_step_4_RBUS	color_sharp_dm_tex_neg_gain_curve_step_4;

	color_sharp_dm_segpk_vpk6_RBUS	color_sharp_dm_segpk_vpk6;
	color_sharp_dm_segpk_edgpk_vsourceslope_RBUS	color_sharp_dm_segpk_edgpk_vsourceslope;
	color_sharp_dm_segpk_sourceslope_extend_RBUS color_sharp_dm_segpk_sourceslope_extend;

	color_sharp_sub_idx_edg_gain_RBUS	color_sharp_sub_idx_edg_gain;
	color_sharp_sub_idx_edg_hv_RBUS color_sharp_sub_idx_edg_hv;
	color_sharp_sub_idx_tex_gain_RBUS	color_sharp_sub_idx_tex_gain;
	color_sharp_sub_idx_tex_hv_RBUS color_sharp_sub_idx_tex_hv;
	color_sharp_sub_idx_v_gain_RBUS color_sharp_sub_idx_v_gain;
	color_sharp_sub_idx_v_hv_RBUS	color_sharp_sub_idx_v_hv;
	color_sharp_sub_idx_v_edg_gain_RBUS color_sharp_sub_idx_v_edg_gain;
	color_sharp_sub_idx_v_edg_hv_RBUS	color_sharp_sub_idx_v_edg_hv;

	color_sharp_dm_segpk_ise_RBUS	color_sharp_dm_segpk_ise;
	color_sharp_dm_segpk_ise_2_RBUS color_sharp_dm_segpk_ise_2;
	color_sharp_dm_2d_shp_tex_flat_0_RBUS	color_sharp_dm_2d_shp_tex_flat_0;
	color_sharp_dm_2d_shp_tex_flat_1_RBUS	color_sharp_dm_2d_shp_tex_flat_1;
	color_sharp_dm_ramp_peaking_v_0_RBUS color_sharp_dm_ramp_peaking_v_0;
	color_sharp_dm_ramp_peaking_v_1_RBUS color_sharp_dm_ramp_peaking_v_1;
	color_sharp_dm_ramp_peaking_v_2_RBUS color_sharp_dm_ramp_peaking_v_2;
	color_sharp_dm_ramp_peaking_v_3_RBUS color_sharp_dm_ramp_peaking_v_3;

	color_sharp_dm_nr_shp_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_NR_SHP_CTRL_reg);
	color_sharp_dm_mkiii_shp_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_MKIII_SHP_CTRL_reg);
	color_sharp_dm_segpk_ctrl1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL1_reg);
	color_sharp_dm_segpk_ctrl2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL2_reg);
	color_sharp_dm_segpk_ctrl3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL3_reg);
	color_sharp_dm_segpk_ctrl4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL4_reg);
	color_sharp_dm_segpk_edge_mono.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGE_MONO_reg);
	color_sharp_dm_segpk_dbg.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_DBG_reg);
	color_sharp_dm_2d_shp_y_remap.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_Y_REMAP_reg);
	color_sharp_dm_peaking_gain.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_GAIN_reg);
	color_sharp_dm_peaking_bound_1.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_BOUND_1_reg);
	color_sharp_dm_peaking_bound_0.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_BOUND_0_reg);
	color_sharp_dm_2d_shp_tex_0.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_0_reg);
	color_sharp_dm_2d_shp_tex_1.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_1_reg);
	color_sharp_dm_2d_shp_tex_1_1.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_1_1_reg);
	color_sharp_dm_2d_shp_tex_2.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_2_reg);
	color_sharp_dm_2d_shp_tex_4.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_4_reg);
	color_sharp_dm_2d_shp_tex_6.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_6_reg);
	color_sharp_dm_2d_shp_tex_7.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_7_reg);
	color_sharp_dm_peaking_emf_texture.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_EMF_TEXTURE_reg);
	color_sharp_dm_emfmk2_ctrl_texture.regValue = IoReg_Read32(COLOR_SHARP_DM_EMFMK2_CTRL_TEXTURE_reg);
	color_sharp_dm_emf_ext_0_texture.regValue = IoReg_Read32(COLOR_SHARP_DM_EMF_EXT_0_TEXTURE_reg);
	color_sharp_dm_emf_ext_1_texture.regValue = IoReg_Read32(COLOR_SHARP_DM_EMF_EXT_1_TEXTURE_reg);
	color_sharp_dm_emf_ext_2_texture.regValue = IoReg_Read32(COLOR_SHARP_DM_EMF_EXT_2_TEXTURE_reg);
	color_sharp_dm_auto_shp_data.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_DATA_reg);
	color_sharp_dm_auto_shp_fir_count_00.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_00_reg);
	color_sharp_dm_auto_shp_fir_count_01.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_01_reg);
	color_sharp_dm_auto_shp_fir_count_02.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_02_reg);
	color_sharp_dm_auto_shp_fir_count_03.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_03_reg);
	color_sharp_dm_auto_shp_fir_count_04.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_04_reg);
	color_sharp_dm_auto_shp_fir_count_05.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_05_reg);
	color_sharp_dm_auto_shp_fir_count_06.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_06_reg);
	color_sharp_dm_auto_shp_fir_count_07.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_07_reg);
	color_sharp_dm_auto_shp_fir_count_08.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_08_reg);
	color_sharp_dm_auto_shp_fir_count_09.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_09_reg);
	color_sharp_dm_auto_shp_fir_count_10.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_10_reg);
	color_sharp_dm_auto_shp_fir_count_11.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_11_reg);
	color_sharp_dm_auto_shp_fir_count_12.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_12_reg);
	color_sharp_dm_auto_shp_fir_count_13.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_13_reg);
	color_sharp_dm_auto_shp_fir_count_14.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_14_reg);
	color_sharp_dm_auto_shp_fir_count_15.regValue = IoReg_Read32(COLOR_SHARP_DM_AUTO_SHP_FIR_COUNT_15_reg);
	color_sharp_dm_segpk_edgpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK3_reg);
	color_sharp_dm_segpk_edgpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK4_reg);
	color_sharp_dm_segpk_edgpk2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK2_reg);
	color_sharp_dm_2d_shp_edge_0.regValue = 	IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_0_reg);
	color_sharp_dm_2d_shp_edge_1.regValue = 	IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_1_reg);
	color_sharp_dm_2d_shp_edge_1_1.regValue =	IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_1_1_reg);
	color_sharp_dm_2d_shp_edge_2.regValue = 	IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_2_reg);
	color_sharp_dm_2d_shp_edge_4.regValue = 	IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_4_reg);
	color_sharp_dm_2d_shp_edge_6.regValue = 	IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_6_reg);
	color_sharp_dm_2d_shp_edge_7.regValue = 	IoReg_Read32(COLOR_SHARP_DM_2D_SHP_EDGE_7_reg);
	color_sharp_dm_peaking_emf.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_EMF_reg);
	color_sharp_dm_emfmk2_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_EMFMK2_CTRL_reg);
	color_sharp_dm_emf_ext_0.regValue = IoReg_Read32(COLOR_SHARP_DM_EMF_EXT_0_reg);
	color_sharp_dm_emf_ext_1.regValue = IoReg_Read32(COLOR_SHARP_DM_EMF_EXT_1_reg);
	color_sharp_dm_segpk_edge_coring_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGE_CORING_0_reg);
	color_sharp_dm_segpk_edge_coring_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGE_CORING_1_reg);
	color_sharp_dm_peaking_h_lvd.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_H_LVD_reg);
	color_sharp_dm_peaking_v_lvd.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_V_LVD_reg);
	color_sharp_dm_segpk_edgpk_sourceslope.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK_SOURCESLOPE_reg);
	color_sharp_dm_segpk_sourceslope_extend.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_SOURCESLOPE_EXTEND_reg);
	color_sharp_dm_segpk_edgpk_sourceslope_flat.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK_SOURCESLOPE_FLAT_reg);

	color_sharp_dm_segpk_vpk1.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK1_reg);
	color_sharp_dm_segpk_vpk2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK2_reg);
	color_sharp_dm_segpk_vpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK3_reg);
	color_sharp_dm_segpk_vpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK4_reg);
	color_sharp_dm_segpk_vpk5.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK5_reg);

	color_sharp_dm_sours_lpf_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SOURS_LPF_0_reg);
	color_sharp_dm_sours_lpf_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SOURS_LPF_1_reg);
	color_sharp_dm_maxmin_shp_gain_0.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_GAIN_0_reg);
	color_sharp_dm_maxmin_shp_gain_1.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_GAIN_1_reg);
	color_sharp_dm_maxmin_shp_gain_2.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_GAIN_2_reg);
	color_sharp_dm_maxmin_shp_gain_3.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_GAIN_3_reg);
	color_sharp_dm_maxmin_shp_step_0.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_STEP_0_reg);
	color_sharp_dm_maxmin_shp_step_1.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_STEP_1_reg);
	color_sharp_dm_maxmin_shp_step_2.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_STEP_2_reg);
	color_sharp_dm_maxmin_shp_step_3.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_SHP_STEP_3_reg);
	color_sharp_dm_vpk_hpk_add.regValue = IoReg_Read32(COLOR_SHARP_DM_VPK_HPK_ADD_reg);
	color_sharp_dm_vc_after_filter_0.regValue = IoReg_Read32(COLOR_SHARP_DM_VC_AFTER_FILTER_0_reg);
	color_sharp_dm_vc_after_filter_1.regValue = IoReg_Read32(COLOR_SHARP_DM_VC_AFTER_FILTER_1_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_0_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_1_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_2_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_3_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_4.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_4_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_5.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_5_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_6.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_6_reg);
	color_sharp_dm_vertical_pos_gain_curve_gain_7.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_gain_7_reg);
	color_sharp_dm_vertical_pos_gain_curve_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_0_reg);
	color_sharp_dm_vertical_pos_gain_curve_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_1_reg);
	color_sharp_dm_vertical_pos_gain_curve_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_2_reg);
	color_sharp_dm_vertical_pos_gain_curve_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_3_reg);
	color_sharp_dm_vertical_pos_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_pos_gain_curve_step_4_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_0_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_1_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_2_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_3_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_4.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_4_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_5.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_5_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_6.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_6_reg);
	color_sharp_dm_vertical_neg_gain_curve_gain_7.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_gain_7_reg);
	color_sharp_dm_vertical_neg_gain_curve_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_0_reg);
	color_sharp_dm_vertical_neg_gain_curve_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_1_reg);
	color_sharp_dm_vertical_neg_gain_curve_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_2_reg);
	color_sharp_dm_vertical_neg_gain_curve_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_3_reg);
	color_sharp_dm_vertical_neg_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_vertical_neg_gain_curve_step_4_reg);
	color_sharp_dm_gain_curve_debug.regValue = IoReg_Read32(COLOR_SHARP_DM_GAIN_CURVE_DEBUG_reg);

	color_sharp_dm_maxmin_lpf_weit_0.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_0_reg);
	color_sharp_dm_maxmin_lpf_weit_1.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_1_reg);
	color_sharp_dm_maxmin_lpf_weit_2.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_2_reg);
	color_sharp_dm_maxmin_lpf_weit_3.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_WEIT_3_reg);
	color_sharp_dm_maxmin_lpf_step_0.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_STEP_0_reg);
	color_sharp_dm_maxmin_lpf_step_1.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_STEP_1_reg);
	color_sharp_dm_maxmin_lpf_step_2.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_STEP_2_reg);
	color_sharp_dm_maxmin_lpf_step_3.regValue = 	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_LPF_STEP_3_reg);
	color_sharp_dm_maxmin_gain_by_y_gain_0.regValue =	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_0_reg);
	color_sharp_dm_maxmin_gain_by_y_gain_1.regValue =	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_1_reg);
	color_sharp_dm_maxmin_gain_by_y_gain_2.regValue =	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_2_reg);
	color_sharp_dm_maxmin_gain_by_y_gain_3.regValue =	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_GAIN_3_reg);
	color_sharp_dm_maxmin_gain_by_y_step_0.regValue =	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_STEP_0_reg);
	color_sharp_dm_maxmin_gain_by_y_step_1.regValue =	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_STEP_1_reg);
	color_sharp_dm_maxmin_gain_by_y_step_2.regValue =	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_STEP_2_reg);
	color_sharp_dm_maxmin_gain_by_y_step_3.regValue =	IoReg_Read32(COLOR_SHARP_DM_MAXMIN_GAIN_BY_Y_STEP_3_reg);
	color_sharp_dm_ramp_peaking_0.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_0_reg);
	color_sharp_dm_ramp_peaking_1.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_1_reg);
	color_sharp_dm_ramp_peaking_2.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_2_reg);
	color_sharp_dm_ramp_peaking_3.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_3_reg);
	color_sharp_dm_ramp_peaking_v_0.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_V_0_reg);
	color_sharp_dm_ramp_peaking_v_1.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_V_1_reg);
	color_sharp_dm_ramp_peaking_v_2.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_V_2_reg);
	color_sharp_dm_ramp_peaking_v_3.regValue = IoReg_Read32(COLOR_SHARP_DM_RAMP_PEAKING_V_3_reg);

	/* peaking enable control*/
	ptr->TwoD_Peak_AdaptCtrl.Peaking_Enable = color_sharp_dm_nr_shp_ctrl.peaking_enable;

	/*
	Copy the content from drvif_color_Set_Sharpness_Table, and then swap what before and after "=" .
	Open a new tab in the text editor.
	Erace ";" and replace "=" with a tab.
	Replace "//" with a tab for censoring unused rows.
	Paste them to Excel.
	Censor unwanted rows.
	Swap the two columns.
	Insert "=" between two columns.
	Append ";" for each row.
	Copy the table and paste back here.
	*/
	ptr->TwoD_Peak_AdaptCtrl.Edge_Mode = color_sharp_dm_segpk_ctrl1.edgmode;
	ptr->TwoD_Peak_AdaptCtrl.Sobel_Mode = color_sharp_dm_segpk_ctrl1.sobelmode;
	ptr->TwoD_Peak_AdaptCtrl.Edgextrange = color_sharp_dm_segpk_ctrl1.edgextrange;
	ptr->TwoD_Peak_AdaptCtrl.Sobel_Upbnd = color_sharp_dm_segpk_ctrl1.sobel_upbnd;
	ptr->TwoD_Peak_AdaptCtrl.Sobel_Stepbit = color_sharp_dm_segpk_ctrl1.sobel_stepbit;
	ptr->TwoD_Peak_AdaptCtrl.Zdiff_Upbnd = color_sharp_dm_segpk_ctrl2.zdiff_upbnd;
	ptr->TwoD_Peak_AdaptCtrl.Zdiff_Stepbit = color_sharp_dm_segpk_ctrl2.zdiff_stepbit;
	ptr->TwoD_Peak_AdaptCtrl.Zigsftbit_Reg = color_sharp_dm_segpk_ctrl2.zigsftbit_reg;
	ptr->TwoD_Peak_AdaptCtrl.Tex_En = color_sharp_dm_segpk_ctrl3.tex_en;
	ptr->TwoD_Peak_AdaptCtrl.Tex_Upbnd = color_sharp_dm_segpk_ctrl3.tex_upbnd;
	ptr->TwoD_Peak_AdaptCtrl.Tex_Stepbit = color_sharp_dm_segpk_ctrl3.tex_stepbit;
	// removed ptr->TwoD_Peak_AdaptCtrl.Actsftbit_Reg = color_sharp_dm_segpk_ctrl3.actsftbit_reg;
	ptr->TwoD_Peak_AdaptCtrl._5x5_h/*merlin3*/ = color_sharp_dm_segpk_ctrl2.tex_h_range;
	ptr->TwoD_Peak_AdaptCtrl._5x5_v/*merlin3*/ = color_sharp_dm_segpk_ctrl2.tex_v_range;
	ptr->TwoD_Peak_AdaptCtrl.Tex_Exclude_Smooth/*merlin3*/ = color_sharp_dm_segpk_ctrl3.tex_exclude_smooth;
	ptr->TwoD_Peak_AdaptCtrl.Edg_Tex_Blending_Lpf_Filter = color_sharp_dm_segpk_ctrl1.edg_tex_blending_lpf_filter;
	ptr->TwoD_Peak_AdaptCtrl.Edg_Tex_Blending_Lpf_En = color_sharp_dm_segpk_ctrl1.edg_tex_blending_lpf_filter_en;
	ptr->TwoD_Peak_AdaptCtrl.Hpf_Vpf_Maxmin_En/*magellan2*/ = color_sharp_dm_vpk_hpk_add.hpf_vpf_maxmin_en;
	ptr->TwoD_Peak_AdaptCtrl.Src_Sel/*magellan2*/ = color_sharp_dm_vpk_hpk_add.src_sel;
	ptr->TwoD_Peak_AdaptCtrl.PkDlti_comsign = color_sharp_dm_segpk_ctrl2.pkdlti_comsign;
	ptr->TwoD_Peak_AdaptCtrl.Sobel_h/*merlin*/ = color_sharp_dm_segpk_ctrl2.sobelrange_h;
	ptr->TwoD_Peak_AdaptCtrl.Sobel_v/*merlin*/ = color_sharp_dm_segpk_ctrl2.sobelrange_v;
	// removed ptr->TwoD_Peak_AdaptCtrl.Acts_offset/*merlin*/ = color_sharp_dm_segpk_ctrl3.acts_offset;
	ptr->TwoD_Peak_AdaptCtrl.Small_thl/*merlin*/ = color_sharp_dm_segpk_ctrl3.small_th1;
	ptr->TwoD_Peak_AdaptCtrl.Edgextrange_v/*merlin*/ = color_sharp_dm_segpk_ctrl2.sobelrange_v;
	ptr->TwoD_Peak_AdaptCtrl.Sobel_hspace/*merlin3*/ = color_sharp_dm_segpk_ctrl2.sobel_hspace;
	ptr->TwoD_Peak_AdaptCtrl.Sobel_vspace/*merlin3*/ = color_sharp_dm_segpk_ctrl2.sobel_vspace;
	ptr->TwoD_Peak_AdaptCtrl.edg_dlti = color_sharp_dm_segpk_ctrl4.edg_dlti;
	ptr->TwoD_Peak_AdaptCtrl.edg_dlti_tex = color_sharp_dm_segpk_ctrl4.edg_dlti_tex;
	ptr->TwoD_Peak_AdaptCtrl.zdiffl_en = color_sharp_dm_segpk_ctrl4.zdiffl_en;
	ptr->TwoD_Peak_AdaptCtrl.zdiffl_lowbnd = color_sharp_dm_segpk_ctrl4.zdiffl_lowbnd;
	ptr->TwoD_Peak_AdaptCtrl.zdiffl_stepbit = color_sharp_dm_segpk_ctrl4.zdiffl_stepbit;
	ptr->TwoD_Peak_AdaptCtrl.edge_mono_en = color_sharp_dm_segpk_edge_mono.edge_mono_en;
	ptr->TwoD_Peak_AdaptCtrl.edge_mono_stepbit = color_sharp_dm_segpk_edge_mono.edge_mono_stepbit;
	ptr->TwoD_Peak_AdaptCtrl.edge_mono_stepbiu = color_sharp_dm_segpk_edge_mono.edge_mono_stepbiu;
	ptr->TwoD_Peak_AdaptCtrl.edge_mono_upbnd = color_sharp_dm_segpk_edge_mono.edge_mono_upbnd;
	ptr->TwoD_Peak_AdaptCtrl.edge_mono_lowbnd = color_sharp_dm_segpk_edge_mono.edge_mono_lowbnd;
	ptr->Edge_Shp.LV2 = color_sharp_dm_segpk_edgpk2.lv2;
	ptr->Edge_Shp.G_Pos2 = color_sharp_dm_segpk_edgpk3.gain_pos2;
	ptr->Edge_Shp.G_Neg2 = color_sharp_dm_segpk_edgpk3.gain_neg2;
	ptr->Edge_Shp.Edg_D2_Shift_Bit /* sirius add*/ = color_sharp_dm_segpk_edgpk3.edg_d2_shift_bit;
	ptr->Texture_Shp.LV2 = color_sharp_dm_peaking_bound_0.lv2;
	ptr->Texture_Shp.G_Pos2 = color_sharp_dm_peaking_gain.gain_pos2;
	ptr->Texture_Shp.G_Neg2 = color_sharp_dm_peaking_gain.gain_neg2;
	ptr->Texture_Shp.Edg_D2_Shift_Bit = color_sharp_dm_peaking_gain.tex_d2_shift_bit;
	ptr->Vertical.C0 = color_sharp_dm_segpk_vpk1.vpk_c0;
	ptr->Vertical.C1 = color_sharp_dm_segpk_vpk1.vpk_c1;
	ptr->Vertical.C2 = color_sharp_dm_segpk_vpk1.vpk_c2;
	ptr->Vertical.Vext_Reg = color_sharp_dm_segpk_vpk4.vext_reg;
	ptr->Vertical.Vemf_En = color_sharp_dm_segpk_vpk4.vemf_en;
	ptr->Vertical.en_tex = color_sharp_dm_sr_ctrl.vpk_tex_en;
	ptr->Vertical.en_edg = color_sharp_dm_sr_ctrl.vpk_edg_en;
	ptr->Vertical.V_Tex_en = color_sharp_dm_segpk_vpk2.v_tex_en;
	ptr->Vertical.C3/*magellan2*/ = color_sharp_dm_segpk_vpk4.vpk_c3;
	ptr->Vertical.C4/*magellan2*/ = color_sharp_dm_segpk_vpk4.vpk_c4;
	ptr->Vertical.G_Neg2 = color_sharp_dm_segpk_vpk3.gain_neg2;
	ptr->Vertical.G_Pos2 = color_sharp_dm_segpk_vpk3.gain_pos2;
	ptr->Vertical.LV2 = color_sharp_dm_segpk_vpk3.lv2;
	// removed ptr->Vertical.enable_edge/*merlin*/ = color_sharp_dm_segpk_vpk4.vpk_edg_en;
	ptr->Edge_EMF_Table.EMF_En = color_sharp_dm_nr_shp_ctrl.edg_emf_enable;
	ptr->Text_EMF_Table.EMF_En = color_sharp_dm_nr_shp_ctrl.tex_emf_enable;
	ptr->Edge_EMF_Table.EMF_Shift = color_sharp_dm_emfmk2_ctrl.emf_shift;
	ptr->Edge_EMF_Table.Seg_0_Gain = color_sharp_dm_peaking_emf.seg0_gain_sel;
	ptr->Edge_EMF_Table.Seg_1_Gain = color_sharp_dm_peaking_emf.seg1_gain_sel;
	ptr->Edge_EMF_Table.Seg_2_Gain = color_sharp_dm_peaking_emf.seg2_gain_sel;
	ptr->Edge_EMF_Table.Seg_0_Offset = color_sharp_dm_peaking_emf.seg0_offset;
	ptr->Edge_EMF_Table.emf_fixextent_neg = color_sharp_dm_emf_ext_0.emf_fixextent_neg;
	ptr->Edge_EMF_Table.emf_fixextent_pos = color_sharp_dm_emf_ext_0.emf_fixextent_pos;
	ptr->Edge_EMF_Table.Seg_1_Offset = color_sharp_dm_emf_ext_0.seg1_offset;
	ptr->Edge_EMF_Table.Seg_2_Offset = color_sharp_dm_emf_ext_1.seg2_offset;
	ptr->Edge_EMF_Table.Seg_0_x = color_sharp_dm_emf_ext_1.seg0_x;
	ptr->Edge_EMF_Table.Seg_1_x = color_sharp_dm_emf_ext_1.seg1_x;
	ptr->Edge_EMF_Table.Blend_wt = color_sharp_dm_emfmk2_ctrl.blend_wt;
	ptr->Edge_EMF_Table.v5= color_sharp_dm_peaking_emf.emf_range_v;
	ptr->Edge_EMF_Table.h11= color_sharp_dm_emfmk2_ctrl.emf_range_h;
	ptr->Text_EMF_Table.EMF_Shift = color_sharp_dm_emfmk2_ctrl_texture.emf_shift;
	ptr->Text_EMF_Table.Seg_0_Gain = color_sharp_dm_peaking_emf_texture.seg0_gain_sel;
	ptr->Text_EMF_Table.Seg_1_Gain = color_sharp_dm_peaking_emf_texture.seg1_gain_sel;
	ptr->Text_EMF_Table.Seg_2_Gain = color_sharp_dm_peaking_emf_texture.seg2_gain_sel;
	ptr->Text_EMF_Table.Seg_0_Offset = color_sharp_dm_peaking_emf_texture.seg0_offset;
	ptr->Text_EMF_Table.emf_fixextent_neg = color_sharp_dm_emf_ext_0_texture.emf_fixextent_neg;
	ptr->Text_EMF_Table.emf_fixextent_pos = color_sharp_dm_emf_ext_0_texture.emf_fixextent_pos;
	ptr->Text_EMF_Table.tex_share_v_enable = color_sharp_dm_emf_ext_0_texture.tex_share_v_enable;
	ptr->Text_EMF_Table.tex_share_v_blend_wt = color_sharp_dm_emf_ext_0_texture.tex_share_v_blend_wt;
	ptr->Text_EMF_Table.Seg_1_Offset = color_sharp_dm_emf_ext_0_texture.seg1_offset;
	ptr->Text_EMF_Table.Seg_2_Offset = color_sharp_dm_emf_ext_1_texture.seg2_offset;
	ptr->Text_EMF_Table.Seg_0_x = color_sharp_dm_emf_ext_1_texture.seg0_x;
	ptr->Text_EMF_Table.Seg_1_x = color_sharp_dm_emf_ext_1_texture.seg1_x;
	ptr->Text_EMF_Table.Blend_wt = color_sharp_dm_emfmk2_ctrl_texture.blend_wt;
	ptr->Text_EMF_Table.EMF_texgain= color_sharp_dm_peaking_emf_texture.texgain;
	ptr->Text_EMF_Table.v5= color_sharp_dm_emfmk2_ctrl_texture.emf_range_v;
	ptr->Text_EMF_Table.h11= color_sharp_dm_emfmk2_ctrl_texture.emf_range_h;
	ptr->Text_EMF_Table.emf_tex_dff = color_sharp_dm_emf_ext_2_texture.emf_tex_dff;
	ptr->Text_EMF_Table.emf_tex_dff_ub = color_sharp_dm_emf_ext_2_texture.emf_tex_dff_ub;
	ptr->MK3_Table.Lpf2d_C0 = color_sharp_dm_sours_lpf_0.coef_c0;
	ptr->MK3_Table.Lpf2d_C1 = color_sharp_dm_sours_lpf_0.coef_c1;
	ptr->MK3_Table.Lpf2d_C2 = color_sharp_dm_sours_lpf_0.coef_c2;
	ptr->MK3_Table.Lpf2d_C3 = color_sharp_dm_sours_lpf_0.coef_c3;
	ptr->MK3_Table.Lpf2d_C4 = color_sharp_dm_sours_lpf_1.coef_c4;
	ptr->MK3_Table.Lpf2d_C5 = color_sharp_dm_sours_lpf_1.coef_c5;
	ptr->MK3_Table.MaxMin_Gain_0 = color_sharp_dm_maxmin_shp_gain_0.s0;
	ptr->MK3_Table.MaxMin_Gain_1 = color_sharp_dm_maxmin_shp_gain_0.s1;
	ptr->MK3_Table.MaxMin_Gain_2 = color_sharp_dm_maxmin_shp_gain_1.s2;
	ptr->MK3_Table.MaxMin_Gain_3 = color_sharp_dm_maxmin_shp_gain_1.s3;
	ptr->MK3_Table.MaxMin_Gain_4 = color_sharp_dm_maxmin_shp_gain_2.s4;
	ptr->MK3_Table.MaxMin_Gain_5 = color_sharp_dm_maxmin_shp_gain_2.s5;
	ptr->MK3_Table.MaxMin_Gain_6 = color_sharp_dm_maxmin_shp_gain_3.s6;
	ptr->MK3_Table.MaxMin_Step_1 = color_sharp_dm_maxmin_shp_step_0.s1;
	ptr->MK3_Table.MaxMin_Step_2 = color_sharp_dm_maxmin_shp_step_1.s2;
	ptr->MK3_Table.MaxMin_Step_3 = color_sharp_dm_maxmin_shp_step_1.s3;
	ptr->MK3_Table.MaxMin_Step_4 = color_sharp_dm_maxmin_shp_step_2.s4;
	ptr->MK3_Table.MaxMin_Step_5 = color_sharp_dm_maxmin_shp_step_2.s5;
	ptr->MK3_Table.MaxMin_Step_6 = color_sharp_dm_maxmin_shp_step_3.s6;
	ptr->MK3_Table.MaxMin_Gain_En = color_sharp_dm_mkiii_shp_ctrl.max_min_gain_en;
	ptr->MK3_Table.max_min_gain_tex_en = color_sharp_dm_mkiii_shp_ctrl.max_min_gain_tex_en;
	ptr->MK3_Table.max_min_gain_edg_en = color_sharp_dm_mkiii_shp_ctrl.max_min_gain_edg_en;
	ptr->MK3_Table.max_min_gain_v_en = color_sharp_dm_mkiii_shp_ctrl.max_min_gain_v_en;
	ptr->MK3_Table.Lpf_Weit_0 = color_sharp_dm_maxmin_lpf_weit_0.s0;
	ptr->MK3_Table.Lpf_Weit_1 = color_sharp_dm_maxmin_lpf_weit_0.s1;
	ptr->MK3_Table.Lpf_Weit_2 = color_sharp_dm_maxmin_lpf_weit_1.s2;
	ptr->MK3_Table.Lpf_Weit_3 = color_sharp_dm_maxmin_lpf_weit_1.s3;
	ptr->MK3_Table.Lpf_Weit_4 = color_sharp_dm_maxmin_lpf_weit_2.s4;
	ptr->MK3_Table.Lpf_Weit_5 = color_sharp_dm_maxmin_lpf_weit_2.s5;
	ptr->MK3_Table.Lpf_Weit_6 = color_sharp_dm_maxmin_lpf_weit_3.s6;
	ptr->MK3_Table.Lpf_Step_1 = color_sharp_dm_maxmin_lpf_step_0.s1;
	ptr->MK3_Table.Lpf_Step_2 = color_sharp_dm_maxmin_lpf_step_1.s2;
	ptr->MK3_Table.Lpf_Step_3 = color_sharp_dm_maxmin_lpf_step_1.s3;
	ptr->MK3_Table.Lpf_Step_4 = color_sharp_dm_maxmin_lpf_step_2.s4;
	ptr->MK3_Table.Lpf_Step_5 = color_sharp_dm_maxmin_lpf_step_2.s5;
	ptr->MK3_Table.Lpf_Step_6 = color_sharp_dm_maxmin_lpf_step_3.s6;
	ptr->MK3_Table.Lpf_Weigh_En = color_sharp_dm_mkiii_shp_ctrl.lpf_weigh_en;
	ptr->MK3_Table.lpf_weigh_tex_en = color_sharp_dm_mkiii_shp_ctrl.lpf_weigh_tex_en;
	ptr->MK3_Table.lpf_weigh_edg_en = color_sharp_dm_mkiii_shp_ctrl.lpf_weigh_edg_en;
	ptr->MK3_Table.Gain_By_Y_Gain_0 = color_sharp_dm_maxmin_gain_by_y_gain_0.s0;
	ptr->MK3_Table.Gain_By_Y_Gain_1 = color_sharp_dm_maxmin_gain_by_y_gain_0.s1;
	ptr->MK3_Table.Gain_By_Y_Gain_2 = color_sharp_dm_maxmin_gain_by_y_gain_1.s2;
	ptr->MK3_Table.Gain_By_Y_Gain_3 = color_sharp_dm_maxmin_gain_by_y_gain_1.s3;
	ptr->MK3_Table.Gain_By_Y_Gain_4 = color_sharp_dm_maxmin_gain_by_y_gain_2.s4;
	ptr->MK3_Table.Gain_By_Y_Gain_5 = color_sharp_dm_maxmin_gain_by_y_gain_2.s5;
	ptr->MK3_Table.Gain_By_Y_Gain_6 = color_sharp_dm_maxmin_gain_by_y_gain_3.s6;
	ptr->MK3_Table.Gain_By_Y_Step_1 = color_sharp_dm_maxmin_gain_by_y_step_0.s1;
	ptr->MK3_Table.Gain_By_Y_Step_2 = color_sharp_dm_maxmin_gain_by_y_step_1.s2;
	ptr->MK3_Table.Gain_By_Y_Step_3 = color_sharp_dm_maxmin_gain_by_y_step_1.s3;
	ptr->MK3_Table.Gain_By_Y_Step_4 = color_sharp_dm_maxmin_gain_by_y_step_2.s4;
	ptr->MK3_Table.Gain_By_Y_Step_5 = color_sharp_dm_maxmin_gain_by_y_step_2.s5;
	ptr->MK3_Table.Gain_By_Y_Step_6 = color_sharp_dm_maxmin_gain_by_y_step_3.s6;
	ptr->MK3_Table.Gain_By_Y_En = color_sharp_dm_mkiii_shp_ctrl.gain_by_y_en;
	ptr->MK3_Table.gain_by_y_tex_en = color_sharp_dm_mkiii_shp_ctrl.gain_by_y_tex_en;
	ptr->MK3_Table.gain_by_y_edg_en = color_sharp_dm_mkiii_shp_ctrl.gain_by_y_edg_en;
	ptr->MK3_Table.gain_by_y_v_en = color_sharp_dm_mkiii_shp_ctrl.gain_by_y_v_en;
	ptr->MK3_Table.lv_by_y_enable = color_sharp_dm_segpk_edgpk2.lv_by_y_enable;
	ptr->MK3_Table.lv_by_y_tex_enable = color_sharp_dm_peaking_bound_0.lv_by_y_tex_enable;
	ptr->MK3_Table.lv_by_y_edg_enable = color_sharp_dm_segpk_edgpk2.lv_by_y_edg_enable;
	ptr->MK3_Table.lv_by_y_v_enable = color_sharp_dm_segpk_vpk3.lv_by_y_v_enable;
	ptr->MK3_Table.lv_by_y_v_edg_enable = color_sharp_dm_segpk_vpk5.lv_by_y_v_edg_enable;
	ptr->TwoD_edg.en_edg = color_sharp_dm_sr_ctrl.h_edg_filter_en;
	ptr->TwoD_edg.C0 = color_sharp_dm_2d_shp_edge_0.coef_c0_0;
	ptr->TwoD_edg.C1 = color_sharp_dm_2d_shp_edge_0.coef_c1_0;
	ptr->TwoD_edg.C2 = color_sharp_dm_2d_shp_edge_0.coef_c2_0;
	ptr->TwoD_edg.C3 = color_sharp_dm_2d_shp_edge_1.coef_c3_0;
	ptr->TwoD_edg.C4 = color_sharp_dm_2d_shp_edge_1.coef_c4_0;
	ptr->TwoD_edg.C5 = color_sharp_dm_2d_shp_edge_1.coef_c5_0;
	ptr->TwoD_edg.C6 = color_sharp_dm_2d_shp_edge_1.coef_c6_0;
	ptr->TwoD_edg.C7/*magellan2*/ = color_sharp_dm_2d_shp_edge_1_1.coef_c7_0;
	ptr->TwoD_edg.C8/*magellan2*/ = color_sharp_dm_2d_shp_edge_1_1.coef_c8_0;
	ptr->TwoD_edg.C9/*magellan2*/ = color_sharp_dm_2d_shp_edge_1_1.coef_c9_0;
	ptr->TwoD_edg.C10/*magellan2*/ = color_sharp_dm_2d_shp_edge_1_1.coef_c10_0;
	ptr->TwoD_edg.C0H1 = color_sharp_dm_2d_shp_edge_2.coef_c0_1;
	ptr->TwoD_edg.C0H2 = color_sharp_dm_2d_shp_edge_4.coef_c0_2;
	ptr->TwoD_edg.C0H3/*magellan2*/ = color_sharp_dm_2d_shp_edge_6.coef_c0_3;
	ptr->TwoD_edg.C0H4/*magellan2*/ = color_sharp_dm_2d_shp_edge_7.coef_c0_4;
	ptr->TwoD_tex.en_tex = color_sharp_dm_sr_ctrl.h_tex_filter_en;
	ptr->TwoD_tex.C0 = color_sharp_dm_2d_shp_tex_0.coef_c0_0;
	ptr->TwoD_tex.C1 = color_sharp_dm_2d_shp_tex_0.coef_c1_0;
	ptr->TwoD_tex.C2 = color_sharp_dm_2d_shp_tex_0.coef_c2_0;
	ptr->TwoD_tex.C3 = color_sharp_dm_2d_shp_tex_1.coef_c3_0;
	ptr->TwoD_tex.C4 = color_sharp_dm_2d_shp_tex_1.coef_c4_0;
	ptr->TwoD_tex.C5 = color_sharp_dm_2d_shp_tex_1.coef_c5_0;
	ptr->TwoD_tex.C6 = color_sharp_dm_2d_shp_tex_1.coef_c6_0;
	ptr->TwoD_tex.C7/*magellan2*/ = color_sharp_dm_2d_shp_tex_1_1.coef_c7_0;
	ptr->TwoD_tex.C8/*magellan2*/ = color_sharp_dm_2d_shp_tex_1_1.coef_c8_0;
	ptr->TwoD_tex.C9/*magellan2*/ = color_sharp_dm_2d_shp_tex_1_1.coef_c9_0;
	ptr->TwoD_tex.C10/*magellan2*/ = color_sharp_dm_2d_shp_tex_1_1.coef_c10_0;
	ptr->TwoD_tex.C0H1 = color_sharp_dm_2d_shp_tex_2.coef_c0_1;
	ptr->TwoD_tex.C0H2 = color_sharp_dm_2d_shp_tex_4.coef_c0_2;
	ptr->TwoD_tex.C0H3/*magellan2*/ = color_sharp_dm_2d_shp_tex_6.coef_c0_3;
	ptr->TwoD_tex.C0H4/*magellan2*/ = color_sharp_dm_2d_shp_tex_7.coef_c0_4;
	ptr->MK3_Table.EdgeCoring_En = color_sharp_dm_segpk_edge_coring_1.edge_coring_en;
	ptr->MK3_Table.EdgeCoring_Gain_En = color_sharp_dm_segpk_edge_coring_1.edge_gain_en;
	ptr->MK3_Table.EdgeCoring_DeltaEdge_Ulevel = color_sharp_dm_segpk_edge_coring_1.deltaedge_ulevel;
	ptr->MK3_Table.EdgeCoring_Clip = color_sharp_dm_segpk_edge_coring_1.edge_coring_clip;
	ptr->MK3_Table.EdgeCoring_Gain_Bound = color_sharp_dm_segpk_edge_coring_1.edge_gain_bound;
	ptr->MK3_Table.EdgeCoring_Ser_Range = color_sharp_dm_segpk_edge_coring_1.edge_ser_range;
	ptr->MK3_Table.EdgeCoring_Bigedge_Ratio = color_sharp_dm_segpk_edge_coring_0.bigedge_ratio;
	ptr->MK3_Table.EdgeCoring_Smalledge_Ratio = color_sharp_dm_segpk_edge_coring_0.smalledge_ratio;
	ptr->MK3_Table.EdgeCoring_Deltaedge_Ratio = color_sharp_dm_segpk_edge_coring_0.deltaedge_ratio;
	ptr->MK3_Table.EdgeCoring_Smalledge_Ulevel = color_sharp_dm_segpk_edge_coring_0.smalledge_ulevel;
	ptr->MK3_Table.Edge_Delta_Ext_Range = color_sharp_dm_segpk_edge_coring_1.edge_delta_ext_range;/*mangellan2*/
	ptr->MK3_Table.h_lvd_range = color_sharp_dm_peaking_h_lvd.range;
	ptr->MK3_Table.h_lvd_gaim = color_sharp_dm_peaking_h_lvd.gaim;
	ptr->MK3_Table.h_lvd_gain = color_sharp_dm_peaking_h_lvd.gain;
	ptr->MK3_Table.v_lvd_range = color_sharp_dm_peaking_v_lvd.range;
	ptr->MK3_Table.v_lvd_gaim = color_sharp_dm_peaking_v_lvd.gaim;
	ptr->MK3_Table.v_lvd_gain = color_sharp_dm_peaking_v_lvd.gain;
	ptr->MK3_Table.LinearY_LB = color_sharp_dm_2d_shp_y_remap.lineary_lb;
	ptr->MK3_Table.LinearY_UB = color_sharp_dm_2d_shp_y_remap.lineary_ub;
	ptr->MK3_Table.LinearY_LM = color_sharp_dm_2d_shp_y_remap.lineary_lmode;
	ptr->MK3_Table.LinearY_UM = color_sharp_dm_2d_shp_y_remap.lineary_umode;
	ptr->MK3_Table.dir = color_sharp_dm_segpk_edgpk_sourceslope.dir;
	ptr->MK3_Table.flat_method = color_sharp_dm_segpk_sourceslope_extend.flat_method;
	ptr->MK3_Table.Gain_boundNeg = color_sharp_dm_segpk_edgpk_sourceslope.gain_boundneg;
	ptr->MK3_Table.Gain_boundPos = color_sharp_dm_segpk_edgpk_sourceslope.gain_boundpos;
	ptr->MK3_Table.Gain_extendNeg = color_sharp_dm_segpk_edgpk_sourceslope.gain_extendneg;
	ptr->MK3_Table.Gain_extendPos = color_sharp_dm_segpk_edgpk_sourceslope.gain_extendpos;
	ptr->MK3_Table.Slope_gain_en = color_sharp_dm_segpk_edgpk_sourceslope.slope_gain_en;
	ptr->MK3_Table.flat_enable = color_sharp_dm_segpk_edgpk_sourceslope_flat.flat_enable;
	ptr->MK3_Table.flat_offset = color_sharp_dm_segpk_edgpk_sourceslope_flat.flat_offset;
	ptr->MK3_Table.flat_gain = color_sharp_dm_segpk_edgpk_sourceslope_flat.flat_gain;
	ptr->MK3_Table.flat_v_enable = color_sharp_dm_segpk_edgpk_sourceslope_flat.flat_v_enable;
	ptr->MK3_Table.flat_offset_v = color_sharp_dm_segpk_edgpk_sourceslope_flat.flat_offset_v;
	ptr->MK3_Table.flat_gain_v = color_sharp_dm_segpk_edgpk_sourceslope_flat.flat_gain_v;
	ptr->After_Filter.C0 = color_sharp_dm_vc_after_filter_0.coef_c0;
	ptr->After_Filter.C1 = color_sharp_dm_vc_after_filter_0.coef_c1;
	ptr->After_Filter.C2 = color_sharp_dm_vc_after_filter_0.coef_c2;
	ptr->After_Filter.C3 = color_sharp_dm_vc_after_filter_0.coef_c3;
	ptr->After_Filter.C4 = color_sharp_dm_vc_after_filter_1.coef_c4;
	ptr->After_Filter.Enable = color_sharp_dm_vc_after_filter_1.enable;
	ptr->After_Filter.HPF_Gain = color_sharp_dm_vc_after_filter_1.hpf_gain;
	// #if K7_not_K8
	// ptr->After_Filter.Mode = color_sharp_dm_vc_after_filter_1.mode;
	// #endif
	ptr->After_Filter.lv = color_sharp_dm_vc_after_filter_1.lv;
	ptr->After_Filter.ub = color_sharp_dm_vc_after_filter_1.ub;
	ptr->V_Pos_Curve.G0 = color_sharp_dm_vertical_pos_gain_curve_gain_0.s0;
	ptr->V_Pos_Curve.G1 = color_sharp_dm_vertical_pos_gain_curve_gain_0.s1;
	ptr->V_Pos_Curve.G2 = color_sharp_dm_vertical_pos_gain_curve_gain_1.s2;
	ptr->V_Pos_Curve.G3 = color_sharp_dm_vertical_pos_gain_curve_gain_1.s3;
	ptr->V_Pos_Curve.G4 = color_sharp_dm_vertical_pos_gain_curve_gain_2.s4;
	ptr->V_Pos_Curve.G5 = color_sharp_dm_vertical_pos_gain_curve_gain_2.s5;
	ptr->V_Pos_Curve.G6 = color_sharp_dm_vertical_pos_gain_curve_gain_3.s6;
	ptr->V_Pos_Curve.G7 = color_sharp_dm_vertical_pos_gain_curve_gain_3.s7;
	ptr->V_Pos_Curve.G8 = color_sharp_dm_vertical_pos_gain_curve_gain_4.s8;
	ptr->V_Pos_Curve.G9 = color_sharp_dm_vertical_pos_gain_curve_gain_4.s9;
	ptr->V_Pos_Curve.G10 = color_sharp_dm_vertical_pos_gain_curve_gain_5.s10;
	ptr->V_Pos_Curve.G11 = color_sharp_dm_vertical_pos_gain_curve_gain_5.s11;
	ptr->V_Pos_Curve.G12 = color_sharp_dm_vertical_pos_gain_curve_gain_6.s12;
	ptr->V_Pos_Curve.G13 = color_sharp_dm_vertical_pos_gain_curve_gain_6.s13;
	ptr->V_Pos_Curve.G14 = color_sharp_dm_vertical_pos_gain_curve_gain_7.s14;
	ptr->V_Pos_Curve.S1 = color_sharp_dm_vertical_pos_gain_curve_step_0.s1;
	ptr->V_Pos_Curve.S2 = color_sharp_dm_vertical_pos_gain_curve_step_0.s2;
	ptr->V_Pos_Curve.S3 = color_sharp_dm_vertical_pos_gain_curve_step_0.s3;
	ptr->V_Pos_Curve.S4 = color_sharp_dm_vertical_pos_gain_curve_step_1.s4;
	ptr->V_Pos_Curve.S5 = color_sharp_dm_vertical_pos_gain_curve_step_1.s5;
	ptr->V_Pos_Curve.S6 = color_sharp_dm_vertical_pos_gain_curve_step_1.s6;
	ptr->V_Pos_Curve.S7 = color_sharp_dm_vertical_pos_gain_curve_step_2.s7;
	ptr->V_Pos_Curve.S8 = color_sharp_dm_vertical_pos_gain_curve_step_2.s8;
	ptr->V_Pos_Curve.S9 = color_sharp_dm_vertical_pos_gain_curve_step_2.s9;
	ptr->V_Pos_Curve.S10 = color_sharp_dm_vertical_pos_gain_curve_step_3.s10;
	ptr->V_Pos_Curve.S11 = color_sharp_dm_vertical_pos_gain_curve_step_3.s11;
	ptr->V_Pos_Curve.S12 = color_sharp_dm_vertical_pos_gain_curve_step_3.s12;
	ptr->V_Pos_Curve.S13 = color_sharp_dm_vertical_pos_gain_curve_step_4.s13;
	ptr->V_Pos_Curve.S14 = color_sharp_dm_vertical_pos_gain_curve_step_4.s14;
	ptr->V_Pos_Curve.edg_curve_bypass = color_sharp_dm_vertical_neg_gain_curve_step_4.edg_curve_bypass;
	ptr->V_Neg_Curve.G0 = color_sharp_dm_vertical_neg_gain_curve_gain_0.s0;
	ptr->V_Neg_Curve.G1 = color_sharp_dm_vertical_neg_gain_curve_gain_0.s1;
	ptr->V_Neg_Curve.G2 = color_sharp_dm_vertical_neg_gain_curve_gain_1.s2;
	ptr->V_Neg_Curve.G3 = color_sharp_dm_vertical_neg_gain_curve_gain_1.s3;
	ptr->V_Neg_Curve.G4 = color_sharp_dm_vertical_neg_gain_curve_gain_2.s4;
	ptr->V_Neg_Curve.G5 = color_sharp_dm_vertical_neg_gain_curve_gain_2.s5;
	ptr->V_Neg_Curve.G6 = color_sharp_dm_vertical_neg_gain_curve_gain_3.s6;
	ptr->V_Neg_Curve.G7 = color_sharp_dm_vertical_neg_gain_curve_gain_3.s7;
	ptr->V_Neg_Curve.G8 = color_sharp_dm_vertical_neg_gain_curve_gain_4.s8;
	ptr->V_Neg_Curve.G9 = color_sharp_dm_vertical_neg_gain_curve_gain_4.s9;
	ptr->V_Neg_Curve.G10 = color_sharp_dm_vertical_neg_gain_curve_gain_5.s10;
	ptr->V_Neg_Curve.G11 = color_sharp_dm_vertical_neg_gain_curve_gain_5.s11;
	ptr->V_Neg_Curve.G12 = color_sharp_dm_vertical_neg_gain_curve_gain_6.s12;
	ptr->V_Neg_Curve.G13 = color_sharp_dm_vertical_neg_gain_curve_gain_6.s13;
	ptr->V_Neg_Curve.G14 = color_sharp_dm_vertical_neg_gain_curve_gain_7.s14;
	ptr->V_Neg_Curve.S1 = color_sharp_dm_vertical_neg_gain_curve_step_0.s1;
	ptr->V_Neg_Curve.S2 = color_sharp_dm_vertical_neg_gain_curve_step_0.s2;
	ptr->V_Neg_Curve.S3 = color_sharp_dm_vertical_neg_gain_curve_step_0.s3;
	ptr->V_Neg_Curve.S4 = color_sharp_dm_vertical_neg_gain_curve_step_1.s4;
	ptr->V_Neg_Curve.S5 = color_sharp_dm_vertical_neg_gain_curve_step_1.s5;
	ptr->V_Neg_Curve.S6 = color_sharp_dm_vertical_neg_gain_curve_step_1.s6;
	ptr->V_Neg_Curve.S7 = color_sharp_dm_vertical_neg_gain_curve_step_2.s7;
	ptr->V_Neg_Curve.S8 = color_sharp_dm_vertical_neg_gain_curve_step_2.s8;
	ptr->V_Neg_Curve.S9 = color_sharp_dm_vertical_neg_gain_curve_step_2.s9;
	ptr->V_Neg_Curve.S10 = color_sharp_dm_vertical_neg_gain_curve_step_3.s10;
	ptr->V_Neg_Curve.S11 = color_sharp_dm_vertical_neg_gain_curve_step_3.s11;
	ptr->V_Neg_Curve.S12 = color_sharp_dm_vertical_neg_gain_curve_step_3.s12;
	ptr->V_Neg_Curve.S13 = color_sharp_dm_vertical_neg_gain_curve_step_4.s13;
	ptr->V_Neg_Curve.S14 = color_sharp_dm_vertical_neg_gain_curve_step_4.s14;
	ptr->V_Neg_Curve.enable = color_sharp_dm_vertical_neg_gain_curve_step_4.enable;

	color_sharp_dm_sr_continual_detail_gain.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_continual_detail_gain_reg);
	color_sharp_dm_sr_9tap_hpf_h_coef_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_h_coef_1_reg);
	color_sharp_dm_sr_13tap_hpf_h_coef_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_13tap_hpf_h_coef_0_reg);
	color_sharp_dm_sr_13tap_hpf_h_coef_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_13tap_hpf_h_coef_1_reg);
	color_sharp_dm_sr_21tap_hpf_h_coef_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_21tap_hpf_h_coef_0_reg);
	color_sharp_dm_sr_21tap_hpf_h_coef_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_21tap_hpf_h_coef_1_reg);
	color_sharp_dm_sr_21tap_hpf_h_coef_2.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_21tap_hpf_h_coef_2_reg);
	color_sharp_dm_sr_9tap_hpf_v_coef_0_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_0_1_reg);
	color_sharp_dm_sr_9tap_hpf_v_coef_1_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_1_0_reg);
	color_sharp_dm_sr_9tap_hpf_v_coef_1_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_1_1_reg);
	color_sharp_dm_sr_9tap_hpf_v_coef_2_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_2_0_reg);
	color_sharp_dm_sr_9tap_hpf_v_coef_2_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_2_1_reg);
	color_sharp_dm_sr_11tap_hpf_h_coef_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_11tap_hpf_h_coef_0_reg);
	color_sharp_dm_sr_11tap_hpf_h_coef_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_11tap_hpf_h_coef_1_reg);
	color_sharp_dm_sr_bias_h_coef.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_BIAS_h_coef_reg);
	color_sharp_dm_sr_ctrl_2.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_2_reg);
	color_sharp_dm_sr_9tap_hpf_v_coef_3_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_3_0_reg);
	color_sharp_dm_sr_9tap_hpf_v_coef_3_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_9tap_hpf_v_coef_3_1_reg);
    color_sharp_dm_sr_bias_v_coef.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_BIAS_v_coef_reg);
	color_sharp_dm_sr_x_coef_0.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_X_coef_0_reg);
	color_sharp_dm_sr_x_coef_1.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_X_coef_1_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_0_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_1_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_2_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_3_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_4.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_4_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_5.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_5_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_6.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_6_reg);
	color_sharp_dm_edge_pos_gain_curve_gain_7.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_gain_7_reg);
	color_sharp_dm_edge_pos_gain_curve_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_step_0_reg);
	color_sharp_dm_edge_pos_gain_curve_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_step_1_reg);
	color_sharp_dm_edge_pos_gain_curve_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_step_2_reg);
	color_sharp_dm_edge_pos_gain_curve_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_step_3_reg);
	color_sharp_dm_edge_pos_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_pos_gain_curve_step_4_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_0_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_1_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_2_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_3_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_4.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_4_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_5.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_5_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_6.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_6_reg);
	color_sharp_dm_edge_neg_gain_curve_gain_7.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_gain_7_reg);
	color_sharp_dm_edge_neg_gain_curve_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_step_0_reg);
	color_sharp_dm_edge_neg_gain_curve_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_step_1_reg);
	color_sharp_dm_edge_neg_gain_curve_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_step_2_reg);
	color_sharp_dm_edge_neg_gain_curve_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_step_3_reg);
	color_sharp_dm_edge_neg_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_edge_neg_gain_curve_step_4_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_0_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_1_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_2_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_3_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_4.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_4_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_5.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_5_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_6.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_6_reg);
	color_sharp_dm_tex_pos_gain_curve_gain_7.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_gain_7_reg);
	color_sharp_dm_tex_pos_gain_curve_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_step_0_reg);
	color_sharp_dm_tex_pos_gain_curve_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_step_1_reg);
	color_sharp_dm_tex_pos_gain_curve_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_step_2_reg);
	color_sharp_dm_tex_pos_gain_curve_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_step_3_reg);
	color_sharp_dm_tex_pos_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_pos_gain_curve_step_4_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_0.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_0_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_1.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_1_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_2.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_2_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_3.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_3_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_4.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_4_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_5.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_5_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_6.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_6_reg);
	color_sharp_dm_tex_neg_gain_curve_gain_7.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_gain_7_reg);
	color_sharp_dm_tex_neg_gain_curve_step_0.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_step_0_reg);
	color_sharp_dm_tex_neg_gain_curve_step_1.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_step_1_reg);
	color_sharp_dm_tex_neg_gain_curve_step_2.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_step_2_reg);
	color_sharp_dm_tex_neg_gain_curve_step_3.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_step_3_reg);
	color_sharp_dm_tex_neg_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_step_4_reg);
	color_sharp_dm_segpk_vpk6.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK6_reg);
	color_sharp_dm_segpk_edgpk_vsourceslope.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK_VSOURCESLOPE_reg);
	color_sharp_dm_segpk_sourceslope_extend.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_SOURCESLOPE_EXTEND_reg);

	color_sharp_sub_idx_edg_gain.regValue = IoReg_Read32(COLOR_SHARP_SUB_IDX_EDG_GAIN_reg);
	color_sharp_sub_idx_edg_hv.regValue = IoReg_Read32(COLOR_SHARP_SUB_IDX_EDG_HV_reg);
	color_sharp_sub_idx_tex_gain.regValue = IoReg_Read32(COLOR_SHARP_Sub_IDX_TEX_GAIN_reg);
	color_sharp_sub_idx_tex_hv.regValue = IoReg_Read32(COLOR_SHARP_SUB_IDX_TEX_HV_reg);
	color_sharp_sub_idx_v_gain.regValue = IoReg_Read32(COLOR_SHARP_Sub_IDX_V_GAIN_reg);
	color_sharp_sub_idx_v_hv.regValue = IoReg_Read32(COLOR_SHARP_SUB_IDX_V_HV_reg);
	color_sharp_sub_idx_v_edg_gain.regValue = IoReg_Read32(COLOR_SHARP_Sub_IDX_V_EDG_GAIN_reg);
	color_sharp_sub_idx_v_edg_hv.regValue = IoReg_Read32(COLOR_SHARP_SUB_IDX_V_EDG_HV_reg);

	color_sharp_dm_segpk_ise.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_ISE_reg);
	color_sharp_dm_segpk_ise_2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_ISE_2_reg);
	color_sharp_dm_2d_shp_tex_flat_0.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_FLAT_0_reg);
	color_sharp_dm_2d_shp_tex_flat_1.regValue = IoReg_Read32(COLOR_SHARP_DM_2D_SHP_TEX_FLAT_1_reg);

	ptr->Edge_Pos_Curve.G0 = color_sharp_dm_edge_pos_gain_curve_gain_0.s0;
	ptr->Edge_Pos_Curve.G1 = color_sharp_dm_edge_pos_gain_curve_gain_0.s1;
	ptr->Edge_Pos_Curve.G2 = color_sharp_dm_edge_pos_gain_curve_gain_1.s2;
	ptr->Edge_Pos_Curve.G3 = color_sharp_dm_edge_pos_gain_curve_gain_1.s3;
	ptr->Edge_Pos_Curve.G4 = color_sharp_dm_edge_pos_gain_curve_gain_2.s4;
	ptr->Edge_Pos_Curve.G5 = color_sharp_dm_edge_pos_gain_curve_gain_2.s5;
	ptr->Edge_Pos_Curve.G6 = color_sharp_dm_edge_pos_gain_curve_gain_3.s6;
	ptr->Edge_Pos_Curve.G7 = color_sharp_dm_edge_pos_gain_curve_gain_3.s7;
	ptr->Edge_Pos_Curve.G8 = color_sharp_dm_edge_pos_gain_curve_gain_4.s8;
	ptr->Edge_Pos_Curve.G9 = color_sharp_dm_edge_pos_gain_curve_gain_4.s9;
	ptr->Edge_Pos_Curve.G10 = color_sharp_dm_edge_pos_gain_curve_gain_5.s10;
	ptr->Edge_Pos_Curve.G11 = color_sharp_dm_edge_pos_gain_curve_gain_5.s11;
	ptr->Edge_Pos_Curve.G12 = color_sharp_dm_edge_pos_gain_curve_gain_6.s12;
	ptr->Edge_Pos_Curve.G13 = color_sharp_dm_edge_pos_gain_curve_gain_6.s13;
	ptr->Edge_Pos_Curve.G14 = color_sharp_dm_edge_pos_gain_curve_gain_7.s14;
	ptr->Edge_Pos_Curve.S1 = color_sharp_dm_edge_pos_gain_curve_step_0.s1;
	ptr->Edge_Pos_Curve.S2 = color_sharp_dm_edge_pos_gain_curve_step_0.s2;
	ptr->Edge_Pos_Curve.S3 = color_sharp_dm_edge_pos_gain_curve_step_0.s3;
	ptr->Edge_Pos_Curve.S4 = color_sharp_dm_edge_pos_gain_curve_step_1.s4;
	ptr->Edge_Pos_Curve.S5 = color_sharp_dm_edge_pos_gain_curve_step_1.s5;
	ptr->Edge_Pos_Curve.S6 = color_sharp_dm_edge_pos_gain_curve_step_1.s6;
	ptr->Edge_Pos_Curve.S7 = color_sharp_dm_edge_pos_gain_curve_step_2.s7;
	ptr->Edge_Pos_Curve.S8 = color_sharp_dm_edge_pos_gain_curve_step_2.s8;
	ptr->Edge_Pos_Curve.S9 = color_sharp_dm_edge_pos_gain_curve_step_2.s9;
	ptr->Edge_Pos_Curve.S10 = color_sharp_dm_edge_pos_gain_curve_step_3.s10;
	ptr->Edge_Pos_Curve.S11 = color_sharp_dm_edge_pos_gain_curve_step_3.s11;
	ptr->Edge_Pos_Curve.S12 = color_sharp_dm_edge_pos_gain_curve_step_3.s12;
	ptr->Edge_Pos_Curve.S13 = color_sharp_dm_edge_pos_gain_curve_step_4.s13;
	ptr->Edge_Pos_Curve.S14 = color_sharp_dm_edge_pos_gain_curve_step_4.s14;
	ptr->Edge_Neg_Curve.G0 = color_sharp_dm_edge_neg_gain_curve_gain_0.s0;
	ptr->Edge_Neg_Curve.G1 = color_sharp_dm_edge_neg_gain_curve_gain_0.s1;
	ptr->Edge_Neg_Curve.G2 = color_sharp_dm_edge_neg_gain_curve_gain_1.s2;
	ptr->Edge_Neg_Curve.G3 = color_sharp_dm_edge_neg_gain_curve_gain_1.s3;
	ptr->Edge_Neg_Curve.G4 = color_sharp_dm_edge_neg_gain_curve_gain_2.s4;
	ptr->Edge_Neg_Curve.G5 = color_sharp_dm_edge_neg_gain_curve_gain_2.s5;
	ptr->Edge_Neg_Curve.G6 = color_sharp_dm_edge_neg_gain_curve_gain_3.s6;
	ptr->Edge_Neg_Curve.G7 = color_sharp_dm_edge_neg_gain_curve_gain_3.s7;
	ptr->Edge_Neg_Curve.G8 = color_sharp_dm_edge_neg_gain_curve_gain_4.s8;
	ptr->Edge_Neg_Curve.G9 = color_sharp_dm_edge_neg_gain_curve_gain_4.s9;
	ptr->Edge_Neg_Curve.G10 = color_sharp_dm_edge_neg_gain_curve_gain_5.s10;
	ptr->Edge_Neg_Curve.G11 = color_sharp_dm_edge_neg_gain_curve_gain_5.s11;
	ptr->Edge_Neg_Curve.G12 = color_sharp_dm_edge_neg_gain_curve_gain_6.s12;
	ptr->Edge_Neg_Curve.G13 = color_sharp_dm_edge_neg_gain_curve_gain_6.s13;
	ptr->Edge_Neg_Curve.G14 = color_sharp_dm_edge_neg_gain_curve_gain_7.s14;
	ptr->Edge_Neg_Curve.S1 = color_sharp_dm_edge_neg_gain_curve_step_0.s1;
	ptr->Edge_Neg_Curve.S2 = color_sharp_dm_edge_neg_gain_curve_step_0.s2;
	ptr->Edge_Neg_Curve.S3 = color_sharp_dm_edge_neg_gain_curve_step_0.s3;
	ptr->Edge_Neg_Curve.S4 = color_sharp_dm_edge_neg_gain_curve_step_1.s4;
	ptr->Edge_Neg_Curve.S5 = color_sharp_dm_edge_neg_gain_curve_step_1.s5;
	ptr->Edge_Neg_Curve.S6 = color_sharp_dm_edge_neg_gain_curve_step_1.s6;
	ptr->Edge_Neg_Curve.S7 = color_sharp_dm_edge_neg_gain_curve_step_2.s7;
	ptr->Edge_Neg_Curve.S8 = color_sharp_dm_edge_neg_gain_curve_step_2.s8;
	ptr->Edge_Neg_Curve.S9 = color_sharp_dm_edge_neg_gain_curve_step_2.s9;
	ptr->Edge_Neg_Curve.S10 = color_sharp_dm_edge_neg_gain_curve_step_3.s10;
	ptr->Edge_Neg_Curve.S11 = color_sharp_dm_edge_neg_gain_curve_step_3.s11;
	ptr->Edge_Neg_Curve.S12 = color_sharp_dm_edge_neg_gain_curve_step_3.s12;
	ptr->Edge_Neg_Curve.S13 = color_sharp_dm_edge_neg_gain_curve_step_4.s13;
	ptr->Edge_Neg_Curve.S14 = color_sharp_dm_edge_neg_gain_curve_step_4.s14;
	ptr->Edge_Neg_Curve.enable = color_sharp_dm_edge_neg_gain_curve_step_4.enable;
	ptr->Tex_Pos_Curve.G0 = color_sharp_dm_tex_pos_gain_curve_gain_0.s0;
	ptr->Tex_Pos_Curve.G1 = color_sharp_dm_tex_pos_gain_curve_gain_0.s1;
	ptr->Tex_Pos_Curve.G2 = color_sharp_dm_tex_pos_gain_curve_gain_1.s2;
	ptr->Tex_Pos_Curve.G3 = color_sharp_dm_tex_pos_gain_curve_gain_1.s3;
	ptr->Tex_Pos_Curve.G4 = color_sharp_dm_tex_pos_gain_curve_gain_2.s4;
	ptr->Tex_Pos_Curve.G5 = color_sharp_dm_tex_pos_gain_curve_gain_2.s5;
	ptr->Tex_Pos_Curve.G6 = color_sharp_dm_tex_pos_gain_curve_gain_3.s6;
	ptr->Tex_Pos_Curve.G7 = color_sharp_dm_tex_pos_gain_curve_gain_3.s7;
	ptr->Tex_Pos_Curve.G8 = color_sharp_dm_tex_pos_gain_curve_gain_4.s8;
	ptr->Tex_Pos_Curve.G9 = color_sharp_dm_tex_pos_gain_curve_gain_4.s9;
	ptr->Tex_Pos_Curve.G10 = color_sharp_dm_tex_pos_gain_curve_gain_5.s10;
	ptr->Tex_Pos_Curve.G11 = color_sharp_dm_tex_pos_gain_curve_gain_5.s11;
	ptr->Tex_Pos_Curve.G12 = color_sharp_dm_tex_pos_gain_curve_gain_6.s12;
	ptr->Tex_Pos_Curve.G13 = color_sharp_dm_tex_pos_gain_curve_gain_6.s13;
	ptr->Tex_Pos_Curve.G14 = color_sharp_dm_tex_pos_gain_curve_gain_7.s14;
	ptr->Tex_Pos_Curve.S1 = color_sharp_dm_tex_pos_gain_curve_step_0.s1;
	ptr->Tex_Pos_Curve.S2 = color_sharp_dm_tex_pos_gain_curve_step_0.s2;
	ptr->Tex_Pos_Curve.S3 = color_sharp_dm_tex_pos_gain_curve_step_0.s3;
	ptr->Tex_Pos_Curve.S4 = color_sharp_dm_tex_pos_gain_curve_step_1.s4;
	ptr->Tex_Pos_Curve.S5 = color_sharp_dm_tex_pos_gain_curve_step_1.s5;
	ptr->Tex_Pos_Curve.S6 = color_sharp_dm_tex_pos_gain_curve_step_1.s6;
	ptr->Tex_Pos_Curve.S7 = color_sharp_dm_tex_pos_gain_curve_step_2.s7;
	ptr->Tex_Pos_Curve.S8 = color_sharp_dm_tex_pos_gain_curve_step_2.s8;
	ptr->Tex_Pos_Curve.S9 = color_sharp_dm_tex_pos_gain_curve_step_2.s9;
	ptr->Tex_Pos_Curve.S10 = color_sharp_dm_tex_pos_gain_curve_step_3.s10;
	ptr->Tex_Pos_Curve.S11 = color_sharp_dm_tex_pos_gain_curve_step_3.s11;
	ptr->Tex_Pos_Curve.S12 = color_sharp_dm_tex_pos_gain_curve_step_3.s12;
	ptr->Tex_Pos_Curve.S13 = color_sharp_dm_tex_pos_gain_curve_step_4.s13;
	ptr->Tex_Pos_Curve.S14 = color_sharp_dm_tex_pos_gain_curve_step_4.s14;
	ptr->Tex_Neg_Curve.G0 = color_sharp_dm_tex_neg_gain_curve_gain_0.s0;
	ptr->Tex_Neg_Curve.G1 = color_sharp_dm_tex_neg_gain_curve_gain_0.s1;
	ptr->Tex_Neg_Curve.G2 = color_sharp_dm_tex_neg_gain_curve_gain_1.s2;
	ptr->Tex_Neg_Curve.G3 = color_sharp_dm_tex_neg_gain_curve_gain_1.s3;
	ptr->Tex_Neg_Curve.G4 = color_sharp_dm_tex_neg_gain_curve_gain_2.s4;
	ptr->Tex_Neg_Curve.G5 = color_sharp_dm_tex_neg_gain_curve_gain_2.s5;
	ptr->Tex_Neg_Curve.G6 = color_sharp_dm_tex_neg_gain_curve_gain_3.s6;
	ptr->Tex_Neg_Curve.G7 = color_sharp_dm_tex_neg_gain_curve_gain_3.s7;
	ptr->Tex_Neg_Curve.G8 = color_sharp_dm_tex_neg_gain_curve_gain_4.s8;
	ptr->Tex_Neg_Curve.G9 = color_sharp_dm_tex_neg_gain_curve_gain_4.s9;
	ptr->Tex_Neg_Curve.G10 = color_sharp_dm_tex_neg_gain_curve_gain_5.s10;
	ptr->Tex_Neg_Curve.G11 = color_sharp_dm_tex_neg_gain_curve_gain_5.s11;
	ptr->Tex_Neg_Curve.G12 = color_sharp_dm_tex_neg_gain_curve_gain_6.s12;
	ptr->Tex_Neg_Curve.G13 = color_sharp_dm_tex_neg_gain_curve_gain_6.s13;
	ptr->Tex_Neg_Curve.G14 = color_sharp_dm_tex_neg_gain_curve_gain_7.s14;
	ptr->Tex_Neg_Curve.S1 = color_sharp_dm_tex_neg_gain_curve_step_0.s1;
	ptr->Tex_Neg_Curve.S2 = color_sharp_dm_tex_neg_gain_curve_step_0.s2;
	ptr->Tex_Neg_Curve.S3 = color_sharp_dm_tex_neg_gain_curve_step_0.s3;
	ptr->Tex_Neg_Curve.S4 = color_sharp_dm_tex_neg_gain_curve_step_1.s4;
	ptr->Tex_Neg_Curve.S5 = color_sharp_dm_tex_neg_gain_curve_step_1.s5;
	ptr->Tex_Neg_Curve.S6 = color_sharp_dm_tex_neg_gain_curve_step_1.s6;
	ptr->Tex_Neg_Curve.S7 = color_sharp_dm_tex_neg_gain_curve_step_2.s7;
	ptr->Tex_Neg_Curve.S8 = color_sharp_dm_tex_neg_gain_curve_step_2.s8;
	ptr->Tex_Neg_Curve.S9 = color_sharp_dm_tex_neg_gain_curve_step_2.s9;
	ptr->Tex_Neg_Curve.S10 = color_sharp_dm_tex_neg_gain_curve_step_3.s10;
	ptr->Tex_Neg_Curve.S11 = color_sharp_dm_tex_neg_gain_curve_step_3.s11;
	ptr->Tex_Neg_Curve.S12 = color_sharp_dm_tex_neg_gain_curve_step_3.s12;
	ptr->Tex_Neg_Curve.S13 = color_sharp_dm_tex_neg_gain_curve_step_4.s13;
	ptr->Tex_Neg_Curve.S14 = color_sharp_dm_tex_neg_gain_curve_step_4.s14;
	ptr->Tex_Neg_Curve.enable = color_sharp_dm_tex_neg_gain_curve_step_4.enable;
	color_sharp_dm_sr_ctrl_3.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_3_reg);
	ptr->SR_Control.Gain = color_sharp_dm_sr_continual_detail_gain.gain;
	ptr->SR_Control.Offset = color_sharp_dm_sr_continual_detail_gain.offset;
	ptr->SR_Control.Period = color_sharp_dm_sr_continual_detail_gain.period;
	ptr->SR_Control.sr_detail_tex_en = color_sharp_dm_sr_continual_detail_gain.sr_detail_tex_en;
	ptr->SR_Control.sr_detail_v_tex_en = color_sharp_dm_sr_continual_detail_gain.sr_detail_v_tex_en;
	ptr->SR_Control.sr_1_touch_edge = color_sharp_dm_sr_ctrl_3.sr_1_touch_edge;
	ptr->SR_Control.sr_1_pxdiff_lim = color_sharp_dm_sr_ctrl_3.sr_1_pxdiff_lim;
	ptr->SR_H9.en_tex = color_sharp_dm_sr_ctrl.sr_0_tex_en;
	ptr->SR_H9.en_edg = color_sharp_dm_sr_ctrl.sr_0_edg_en;
	ptr->SR_H9.C1 = color_sharp_dm_sr_9tap_hpf_h_coef_1.c1;
	ptr->SR_H9.C2 = color_sharp_dm_sr_9tap_hpf_h_coef_1.c2;
	ptr->SR_H9.C3 = color_sharp_dm_sr_9tap_hpf_h_coef_1.c3;
	ptr->SR_H9.C4 = color_sharp_dm_sr_9tap_hpf_h_coef_1.c4;
	ptr->SR_H13.en_tex = color_sharp_dm_sr_ctrl.sr_1_tex_en;
	ptr->SR_H13.en_edg = color_sharp_dm_sr_ctrl.sr_1_edg_en;
	ptr->SR_H13.C0 = color_sharp_dm_sr_13tap_hpf_h_coef_0.c0;
	ptr->SR_H13.C1 = color_sharp_dm_sr_13tap_hpf_h_coef_0.c1;
	ptr->SR_H13.C2 = color_sharp_dm_sr_13tap_hpf_h_coef_0.c2;
	ptr->SR_H13.C3 = color_sharp_dm_sr_13tap_hpf_h_coef_1.c3;
	ptr->SR_H13.C4 = color_sharp_dm_sr_13tap_hpf_h_coef_1.c4;
	ptr->SR_H13.C5 = color_sharp_dm_sr_13tap_hpf_h_coef_1.c5;
	ptr->SR_H13.C6 = color_sharp_dm_sr_13tap_hpf_h_coef_1.c6;
	ptr->SR_H21.en_tex = color_sharp_dm_sr_ctrl.sr_2_tex_en;
	ptr->SR_H21.en_edg = color_sharp_dm_sr_ctrl.sr_2_edg_en;
	ptr->SR_H21.C0 = color_sharp_dm_sr_21tap_hpf_h_coef_0.c0;
	ptr->SR_H21.C1 = color_sharp_dm_sr_21tap_hpf_h_coef_0.c1;
	ptr->SR_H21.C2 = color_sharp_dm_sr_21tap_hpf_h_coef_0.c2;
	ptr->SR_H21.C3 = color_sharp_dm_sr_21tap_hpf_h_coef_1.c3;
	ptr->SR_H21.C4 = color_sharp_dm_sr_21tap_hpf_h_coef_1.c4;
	ptr->SR_H21.C5 = color_sharp_dm_sr_21tap_hpf_h_coef_1.c5;
	ptr->SR_H21.C6 = color_sharp_dm_sr_21tap_hpf_h_coef_1.c6;
	ptr->SR_H21.C7 = color_sharp_dm_sr_21tap_hpf_h_coef_2.c7;
	ptr->SR_H21.C8 = color_sharp_dm_sr_21tap_hpf_h_coef_2.c8;
	ptr->SR_H21.C9 = color_sharp_dm_sr_21tap_hpf_h_coef_2.c9;
	ptr->SR_H21.C10 = color_sharp_dm_sr_21tap_hpf_h_coef_2.c10;
	ptr->SR_V9_1.en_tex = color_sharp_dm_sr_ctrl.sr_v_0_tex_en;
	ptr->SR_V9_1.en_edg = color_sharp_dm_sr_ctrl.sr_v_0_edg_en;
	ptr->SR_V9_1.C1 = color_sharp_dm_sr_9tap_hpf_v_coef_0_1.c1;
	ptr->SR_V9_1.C2 = color_sharp_dm_sr_9tap_hpf_v_coef_0_1.c2;
	ptr->SR_V9_1.C3 = color_sharp_dm_sr_9tap_hpf_v_coef_0_1.c3;
	ptr->SR_V9_1.C4 = color_sharp_dm_sr_9tap_hpf_v_coef_0_1.c4;
	ptr->SR_V9_2.en_tex = color_sharp_dm_sr_ctrl.sr_v_1_tex_en;
	ptr->SR_V9_2.en_edg = color_sharp_dm_sr_ctrl.sr_v_1_edg_en;
	ptr->SR_V9_2.C0 = color_sharp_dm_sr_9tap_hpf_v_coef_1_0.c0;
	ptr->SR_V9_2.C1 = color_sharp_dm_sr_9tap_hpf_v_coef_1_1.c1;
	ptr->SR_V9_2.C2 = color_sharp_dm_sr_9tap_hpf_v_coef_1_1.c2;
	ptr->SR_V9_2.C3 = color_sharp_dm_sr_9tap_hpf_v_coef_1_1.c3;
	ptr->SR_V9_2.C4 = color_sharp_dm_sr_9tap_hpf_v_coef_1_1.c4;
	ptr->SR_V9_3.en_tex = color_sharp_dm_sr_ctrl.sr_v_2_tex_en;
	ptr->SR_V9_3.en_edg = color_sharp_dm_sr_ctrl.sr_v_2_edg_en;
	ptr->SR_V9_3.C0 = color_sharp_dm_sr_9tap_hpf_v_coef_2_0.c0;
	ptr->SR_V9_3.C1 = color_sharp_dm_sr_9tap_hpf_v_coef_2_1.c1;
	ptr->SR_V9_3.C2 = color_sharp_dm_sr_9tap_hpf_v_coef_2_1.c2;
	ptr->SR_V9_3.C3 = color_sharp_dm_sr_9tap_hpf_v_coef_2_1.c3;
	ptr->SR_V9_3.C4 = color_sharp_dm_sr_9tap_hpf_v_coef_2_1.c4;
	ptr->SR_Ring_H11.en_tex = color_sharp_dm_sr_ctrl_2.sr_3_tex_en;
	ptr->SR_Ring_H11.en_edg = color_sharp_dm_sr_ctrl_2.sr_3_edg_en;
	ptr->SR_Ring_H11.C0 = color_sharp_dm_sr_11tap_hpf_h_coef_0.c0;
	ptr->SR_Ring_H11.C1 = color_sharp_dm_sr_11tap_hpf_h_coef_0.c1;
	ptr->SR_Ring_H11.C2 = color_sharp_dm_sr_11tap_hpf_h_coef_1.c2;
	ptr->SR_Ring_H11.C3 = color_sharp_dm_sr_11tap_hpf_h_coef_1.c3;
	ptr->SR_Ring_H11.C4 = color_sharp_dm_sr_11tap_hpf_h_coef_1.c4;
	ptr->SR_Ring_H11.C5 = color_sharp_dm_sr_11tap_hpf_h_coef_1.c5;
    ptr->SR_Ring_H11.C0H0 = color_sharp_dm_sr_bias_h_coef.cb1;
    ptr->SR_Ring_H11.C0H1 = color_sharp_dm_sr_bias_h_coef.cb2;
    ptr->SR_Ring_H11.C0H2 = color_sharp_dm_sr_bias_h_coef.cblv1;
    ptr->SR_Ring_H11.C0H3 = color_sharp_dm_sr_bias_h_coef.cblv2;
	ptr->SR_Ring_V9.en_tex= color_sharp_dm_sr_ctrl_2.sr_v_3_tex_en;
	ptr->SR_Ring_V9.en_edg= color_sharp_dm_sr_ctrl_2.sr_v_3_edg_en;
	ptr->SR_Ring_V9.C0 = color_sharp_dm_sr_9tap_hpf_v_coef_3_0.c0;
	ptr->SR_Ring_V9.C1 = color_sharp_dm_sr_9tap_hpf_v_coef_3_1.c1;
	ptr->SR_Ring_V9.C2 = color_sharp_dm_sr_9tap_hpf_v_coef_3_1.c2;
	ptr->SR_Ring_V9.C3 = color_sharp_dm_sr_9tap_hpf_v_coef_3_1.c3;
	ptr->SR_Ring_V9.C4 = color_sharp_dm_sr_9tap_hpf_v_coef_3_1.c4;
    ptr->SR_Ring_V9.C0H0 = color_sharp_dm_sr_bias_v_coef.cb1;
    ptr->SR_Ring_V9.C0H1 = color_sharp_dm_sr_bias_v_coef.cb2;
    ptr->SR_Ring_V9.C0H2 = color_sharp_dm_sr_bias_v_coef.cblv1;
    ptr->SR_Ring_V9.C0H3 = color_sharp_dm_sr_bias_v_coef.cblv2;
	ptr->SR_X.en_tex = color_sharp_dm_sr_x_coef_0.tex_en;
	ptr->SR_X.en_edg = color_sharp_dm_sr_x_coef_0.edg_en;
	ptr->SR_X.C0 = color_sharp_dm_sr_x_coef_0.c0;
	ptr->SR_X.C1 = color_sharp_dm_sr_x_coef_1.c1;
	ptr->SR_X.C2 = color_sharp_dm_sr_x_coef_1.c2;
	ptr->SR_X.C3 = color_sharp_dm_sr_x_coef_1.c3;
	ptr->SR_X.C4 = color_sharp_dm_sr_x_coef_1.c4;
	ptr->MK3_Table.Slope_gain_v_en = color_sharp_dm_segpk_edgpk_vsourceslope.v_slope_gain_en;
	ptr->MK3_Table.dir_v = color_sharp_dm_segpk_edgpk_vsourceslope.dir_v;
	ptr->MK3_Table.Gain_boundPos_v = color_sharp_dm_segpk_edgpk_vsourceslope.gain_boundpos_v;
	ptr->MK3_Table.Gain_boundNeg_v = color_sharp_dm_segpk_edgpk_vsourceslope.gain_boundneg_v;
	ptr->MK3_Table.Gain_extendPos_v = color_sharp_dm_segpk_edgpk_vsourceslope.gain_extendpos_v;
	ptr->MK3_Table.Gain_extendNeg_v = color_sharp_dm_segpk_edgpk_vsourceslope.gain_extendneg_v;
	ptr->MK3_Table.flat_range_h= color_sharp_dm_segpk_sourceslope_extend.flat_range;
	ptr->MK3_Table.flat_range_v= color_sharp_dm_segpk_sourceslope_extend.flat_range_v;
	ptr->MK3_Table.lowbound= color_sharp_dm_segpk_sourceslope_extend.lowbound;
	ptr->MK3_Table.lowbound_v= color_sharp_dm_segpk_sourceslope_extend.lowbound_v;
	ptr->MK3_Table.use_ramp= color_sharp_dm_segpk_edgpk_sourceslope.use_ramp;
	ptr->Vertical.LV2_edge = color_sharp_dm_segpk_vpk6.edg_lv2;
	ptr->Vertical.G_Pos2_edge = color_sharp_dm_segpk_vpk6.vpk_edg_gain_pos2;
	ptr->Vertical.G_Neg2_edge = color_sharp_dm_segpk_vpk6.vpk_edg_gain_neg2;
	ptr->Sub_Gain_Modify.sub_index_en = color_sharp_sub_idx_edg_gain.sub_index_en;
	ptr->Sub_Gain_Modify.EDG.Gain_Pos = color_sharp_sub_idx_edg_gain.gain_pos;
	ptr->Sub_Gain_Modify.EDG.Gain_Neg = color_sharp_sub_idx_edg_gain.gain_neg;
	ptr->Sub_Gain_Modify.EDG.HV_Pos = color_sharp_sub_idx_edg_hv.hv_pos;
	ptr->Sub_Gain_Modify.EDG.HV_Neg = color_sharp_sub_idx_edg_hv.hv_neg;
	ptr->Sub_Gain_Modify.EDG.LV = color_sharp_sub_idx_edg_hv.lv;
	ptr->Sub_Gain_Modify.TEX.Gain_Pos = color_sharp_sub_idx_tex_gain.gain_pos;
	ptr->Sub_Gain_Modify.TEX.Gain_Neg = color_sharp_sub_idx_tex_gain.gain_neg;
	ptr->Sub_Gain_Modify.TEX.HV_Pos = color_sharp_sub_idx_tex_hv.hv_pos;
	ptr->Sub_Gain_Modify.TEX.HV_Neg = color_sharp_sub_idx_tex_hv.hv_neg;
	ptr->Sub_Gain_Modify.TEX.LV = color_sharp_sub_idx_tex_hv.lv;
	ptr->Sub_Gain_Modify.V.Gain_Pos = color_sharp_sub_idx_v_gain.gain_pos;
	ptr->Sub_Gain_Modify.V.Gain_Neg = color_sharp_sub_idx_v_gain.gain_neg;
	ptr->Sub_Gain_Modify.V.HV_Pos = color_sharp_sub_idx_v_hv.hv_pos;
	ptr->Sub_Gain_Modify.V.HV_Neg = color_sharp_sub_idx_v_hv.hv_neg;
	ptr->Sub_Gain_Modify.V.LV = color_sharp_sub_idx_v_hv.lv;
	ptr->Sub_Gain_Modify.V_EDG.Gain_Pos = color_sharp_sub_idx_v_edg_gain.gain_pos;
	ptr->Sub_Gain_Modify.V_EDG.Gain_Neg = color_sharp_sub_idx_v_edg_gain.gain_neg;
	ptr->Sub_Gain_Modify.V_EDG.HV_Pos = color_sharp_sub_idx_v_edg_hv.hv_pos;
	ptr->Sub_Gain_Modify.V_EDG.HV_Neg = color_sharp_sub_idx_v_edg_hv.hv_neg;
	ptr->Sub_Gain_Modify.V_EDG.LV = color_sharp_sub_idx_v_edg_hv.lv;
	ptr->SEGPK_ISE.ise_en = color_sharp_dm_segpk_ise.ise_en;
	ptr->SEGPK_ISE.ise_step = color_sharp_dm_segpk_ise.ise_step;
	// ptr->SEGPK_ISE.ise_scale = color_sharp_dm_segpk_ise.ise_scale;
	ptr->SEGPK_ISE.ise_ub = color_sharp_dm_segpk_ise.ise_ub;
	ptr->SEGPK_ISE.ise_h_st = color_sharp_dm_segpk_ise.ise_h_st;
	ptr->SEGPK_ISE.ise_h_end = color_sharp_dm_segpk_ise.ise_h_end;
	ptr->SEGPK_ISE.ise_v_st = color_sharp_dm_segpk_ise.ise_v_st;
	ptr->SEGPK_ISE.ise_v_end = color_sharp_dm_segpk_ise.ise_v_end;
	ptr->SEGPK_ISE.ise_mul_max = color_sharp_dm_segpk_ise_2.ise_mul_max;
	ptr->SEGPK_ISE.ise_mul_min = color_sharp_dm_segpk_ise_2.ise_mul_min;
	ptr->Texture_Flat_Shp.en_tex = color_sharp_dm_2d_shp_tex_flat_0.tex_flat_filter_en;
	ptr->Texture_Flat_Shp.C0 = color_sharp_dm_2d_shp_tex_flat_0.coef_c0_0;
	ptr->Texture_Flat_Shp.C1 = color_sharp_dm_2d_shp_tex_flat_1.coef_c1_0;
	ptr->Texture_Flat_Shp.C2 = color_sharp_dm_2d_shp_tex_flat_1.coef_c2_0;
	ptr->Texture_Flat_Shp.C3 = color_sharp_dm_2d_shp_tex_flat_1.coef_c3_0;
	ptr->Texture_Flat_Shp.C0H1 = color_sharp_dm_2d_shp_tex_flat_0.coef_c0_1;
	ptr->Texture_Flat_Shp.C0H2 = color_sharp_dm_2d_shp_tex_flat_0.coef_c0_2;

	ptr->Ramp_H.range          =color_sharp_dm_ramp_peaking_0.ramppk_en          ;
	ptr->Ramp_H.avgline        =color_sharp_dm_ramp_peaking_0.ramp_h_avgline     ;
	ptr->Ramp_H.center_value   =color_sharp_dm_ramp_peaking_0.ramp_h_center_value;
	ptr->Ramp_H.maxmin_value   =color_sharp_dm_ramp_peaking_0.ramp_h_maxmin_value;
	ptr->Ramp_H.const_value    =color_sharp_dm_ramp_peaking_0.ramp_h_const_value ;
	// ptr->Ramp_H.reserved05     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	// ptr->Ramp_H.reserved06     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	ptr->Ramp_H.texgain        =color_sharp_dm_ramp_peaking_1.ramp_h_texgain;
	// ptr->Ramp_H.weight0        =color_sharp_dm_ramp_peaking_1.ramp_h_weight0;
	ptr->Ramp_H.weight1        =color_sharp_dm_ramp_peaking_1.ramp_h_weight1;
	ptr->Ramp_H.weight2        =color_sharp_dm_ramp_peaking_1.ramp_h_weight2;
	// ptr->Ramp_H.reserved11     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	// ptr->Ramp_H.reserved12     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	// ptr->Ramp_H.reserved13     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	// ptr->Ramp_H.reserved14     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	// ptr->Ramp_H.reserved15     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	ptr->Ramp_H.getd2_lv       =color_sharp_dm_ramp_peaking_3.lv       ;
	ptr->Ramp_H.getd2_gain_pos =color_sharp_dm_ramp_peaking_2.gain_pos ;
	ptr->Ramp_H.getd2_gain_neg =color_sharp_dm_ramp_peaking_2.gain_neg ;
	ptr->Ramp_H.getd2_hv_pos   =color_sharp_dm_ramp_peaking_3.hv_pos   ;
	ptr->Ramp_H.getd2_hv_neg   =color_sharp_dm_ramp_peaking_3.hv_neg   ;
	ptr->Ramp_H.getd2_gain_pos2=color_sharp_dm_ramp_peaking_2.gain_pos2;
	ptr->Ramp_H.getd2_gain_neg2=color_sharp_dm_ramp_peaking_2.gain_neg2;
	ptr->Ramp_H.getd2_lv2      =color_sharp_dm_ramp_peaking_2.lv2      ;
	// ptr->Ramp_H.reserved24     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	// ptr->Ramp_H.reserved25     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	// ptr->Ramp_H.reserved26     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	// ptr->Ramp_H.reserved27     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	// ptr->Ramp_H.reserved28     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	// ptr->Ramp_H.reserved29     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	// ptr->Ramp_H.reserved30     =color_sharp_dm_ramp_peaking_0.ramppk_en;
	ptr->Ramp_V.range          =color_sharp_dm_ramp_peaking_v_0.vramppk_en         ;
	ptr->Ramp_V.avgline        =color_sharp_dm_ramp_peaking_v_0.ramp_v_avgline     ;
	ptr->Ramp_V.center_value   =color_sharp_dm_ramp_peaking_v_0.ramp_v_center_value;
	ptr->Ramp_V.maxmin_value   =color_sharp_dm_ramp_peaking_v_0.ramp_v_maxmin_value;
	ptr->Ramp_V.const_value    =color_sharp_dm_ramp_peaking_v_0.ramp_v_const_value ;
	// ptr->Ramp_V.reserved05     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	// ptr->Ramp_V.reserved06     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	ptr->Ramp_V.texgain        =color_sharp_dm_ramp_peaking_v_1.ramp_v_texgain;
	// ptr->Ramp_V.weight0        =color_sharp_dm_ramp_peaking_v_1.ramp_v_weight0;
	ptr->Ramp_V.weight1        =color_sharp_dm_ramp_peaking_v_1.ramp_v_weight1;
	ptr->Ramp_V.weight2        =color_sharp_dm_ramp_peaking_v_1.ramp_v_weight2;
	// ptr->Ramp_V.reserved11     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	// ptr->Ramp_V.reserved12     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	// ptr->Ramp_V.reserved13     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	// ptr->Ramp_V.reserved14     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	// ptr->Ramp_V.reserved15     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	ptr->Ramp_V.getd2_lv       =color_sharp_dm_ramp_peaking_v_3.lv_v      ;
	ptr->Ramp_V.getd2_gain_pos =color_sharp_dm_ramp_peaking_v_2.gain_posv ;
	ptr->Ramp_V.getd2_gain_neg =color_sharp_dm_ramp_peaking_v_2.gain_negv ;
	ptr->Ramp_V.getd2_hv_pos   =color_sharp_dm_ramp_peaking_v_3.hv_posv   ;
	ptr->Ramp_V.getd2_hv_neg   =color_sharp_dm_ramp_peaking_v_3.hv_negv   ;
	ptr->Ramp_V.getd2_gain_pos2=color_sharp_dm_ramp_peaking_v_2.gain_posv2;
	ptr->Ramp_V.getd2_gain_neg2=color_sharp_dm_ramp_peaking_v_2.gain_negv2;
	ptr->Ramp_V.getd2_lv2      =color_sharp_dm_ramp_peaking_v_2.lv2_v     ;
	// ptr->Ramp_V.reserved24     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	// ptr->Ramp_V.reserved25     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	// ptr->Ramp_V.reserved26     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	// ptr->Ramp_V.reserved27     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	// ptr->Ramp_V.reserved28     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	// ptr->Ramp_V.reserved29     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	// ptr->Ramp_V.reserved30     =color_sharp_dm_ramp_peaking_v_0.ramppk_en;
	}
}

void drvif_color_Get_Sharpness_lv2(DRV_Sharpness_LV2 *ptr)
{
	color_sharp_dm_segpk_edgpk2_RBUS color_sharp_dm_segpk_edgpk2;
	color_sharp_dm_peaking_bound_0_RBUS color_sharp_dm_peaking_bound_0;
	color_sharp_dm_segpk_vpk3_RBUS color_sharp_dm_segpk_vpk3;

	if (NULL == ptr)
		return;

	color_sharp_dm_segpk_edgpk2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK2_reg);
	color_sharp_dm_peaking_bound_0.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_BOUND_0_reg);
	color_sharp_dm_segpk_vpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK3_reg);

	ptr->Edge_LV2 = color_sharp_dm_segpk_edgpk2.lv2;
	ptr->Texture_LV2 = color_sharp_dm_peaking_bound_0.lv2;
	ptr->Vertical_LV2 = color_sharp_dm_segpk_vpk3.lv2;
}

void driver_color_setSR(unsigned char onOff)
{
color_sharp_dm_tex_neg_gain_curve_step_4_RBUS	color_sharp_dm_tex_neg_gain_curve_step_4;
color_sharp_dm_tex_neg_gain_curve_step_4.regValue = IoReg_Read32(COLOR_SHARP_DM_tex_neg_gain_curve_step_4_reg);
color_sharp_dm_tex_neg_gain_curve_step_4.enable = onOff;
IoReg_Write32(COLOR_SHARP_DM_tex_neg_gain_curve_step_4_reg, color_sharp_dm_tex_neg_gain_curve_step_4.regValue);
}

void drvif_color_setSR_Ring_Gen_Gain(unsigned char gain)
{
	// removed
}
void drvif_color_setSR_levle_Tv006(unsigned char level)
{
//this funtion is for test
//after DDTS please remove it
	//color_sharp_dm_sr_ctrl_RBUS color_sharp_dm_sr_ctrl;
	color_sharp_dm_sr_continual_detail_gain_RBUS	color_sharp_dm_sr_continual_detail_gain;

	color_sharp_dm_sr_continual_detail_gain.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_continual_detail_gain_reg);


	//color_sharp_dm_sr_ctrl.regValue = IoReg_Read32(COLOR_SHARP_DM_SR_Ctrl_reg);

	switch(level)
	{
		case 0: //off
			color_sharp_dm_sr_continual_detail_gain.gain =0;
			color_sharp_dm_sr_continual_detail_gain.offset=-50;
			color_sharp_dm_sr_continual_detail_gain.sr_detail_tex_en=1;
		break;

		case 1: //low
			color_sharp_dm_sr_continual_detail_gain.gain =50;
			color_sharp_dm_sr_continual_detail_gain.offset=60;
			color_sharp_dm_sr_continual_detail_gain.sr_detail_tex_en=1;

		break;

		case 2: //middle
			color_sharp_dm_sr_continual_detail_gain.gain =100;
			color_sharp_dm_sr_continual_detail_gain.offset=128;
			color_sharp_dm_sr_continual_detail_gain.sr_detail_tex_en=1;

		break;

		case 4: //high
			color_sharp_dm_sr_continual_detail_gain.gain =200;
			color_sharp_dm_sr_continual_detail_gain.offset=70;
			color_sharp_dm_sr_continual_detail_gain.sr_detail_tex_en=0;

		break;

		case 5: //off of off
			color_sharp_dm_sr_continual_detail_gain.sr_detail_tex_en=0;


		default:
		break;
	}


 IoReg_Write32(COLOR_SHARP_DM_SR_continual_detail_gain_reg,color_sharp_dm_sr_continual_detail_gain.regValue);
}

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
// for AI effect reset shp cds.
void drvif_color_set_cds_table_AI(VIP_CDS_Table *ptr, unsigned char cm0, unsigned char cm1, unsigned char cm2, unsigned char cm3)
{
	color_sharp_dm_cds_peaking_gain_bound_ctrl_RBUS	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg;
	color_sharp_dm_cds_cm0_u_bound_0_RBUS			color_sharp_dm_cds_cm0_u_bound_0_reg;
	color_sharp_dm_cds_cm0_v_bound_0_RBUS			color_sharp_dm_cds_cm0_v_bound_0_reg;
	color_sharp_dm_cds_cm0_u_bound_1_RBUS			color_sharp_dm_cds_cm0_u_bound_1_reg;
	color_sharp_dm_cds_cm0_v_bound_1_RBUS			color_sharp_dm_cds_cm0_v_bound_1_reg;
	color_sharp_dm_cds_peaking_gain_cm0_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm0_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm0_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm0_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm0_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm0_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm0_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm0_edge_reg;
	color_sharp_dm_cds_cm1_u_bound_0_RBUS			color_sharp_dm_cds_cm1_u_bound_0_reg;
	color_sharp_dm_cds_cm1_v_bound_0_RBUS			color_sharp_dm_cds_cm1_v_bound_0_reg;
	color_sharp_dm_cds_cm1_u_bound_1_RBUS			color_sharp_dm_cds_cm1_u_bound_1_reg;
	color_sharp_dm_cds_cm1_v_bound_1_RBUS			color_sharp_dm_cds_cm1_v_bound_1_reg;
	color_sharp_dm_cds_peaking_gain_cm1_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm1_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm1_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm1_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm1_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm1_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm1_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm1_edge_reg;
	color_sharp_dm_cds_cm2_u_bound_0_RBUS			color_sharp_dm_cds_cm2_u_bound_0_reg;
	color_sharp_dm_cds_cm2_v_bound_0_RBUS			color_sharp_dm_cds_cm2_v_bound_0_reg;
	color_sharp_dm_cds_cm2_u_bound_1_RBUS			color_sharp_dm_cds_cm2_u_bound_1_reg;
	color_sharp_dm_cds_cm2_v_bound_1_RBUS			color_sharp_dm_cds_cm2_v_bound_1_reg;
	color_sharp_dm_cds_peaking_gain_cm2_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm2_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm2_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm2_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm2_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm2_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm2_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm2_edge_reg;
	color_sharp_dm_cds_cm3_u_bound_0_RBUS			color_sharp_dm_cds_cm3_u_bound_0_reg;
	color_sharp_dm_cds_cm3_v_bound_0_RBUS			color_sharp_dm_cds_cm3_v_bound_0_reg;
	color_sharp_dm_cds_cm3_u_bound_1_RBUS			color_sharp_dm_cds_cm3_u_bound_1_reg;
	color_sharp_dm_cds_cm3_v_bound_1_RBUS			color_sharp_dm_cds_cm3_v_bound_1_reg;
	color_sharp_dm_cds_peaking_gain_cm3_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm3_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm3_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm3_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm3_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm3_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm3_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm3_edge_reg;
	// Vertical CDS since Merlin2
	color_sharp_dm_cds_peaking_gain_cm0_v_RBUS	color_sharp_dm_cds_peaking_gain_cm0_v_reg;
	color_sharp_dm_cds_peaking_bound_cm0_v_RBUS	color_sharp_dm_cds_peaking_bound_cm0_v_reg;
	color_sharp_dm_cds_peaking_gain_cm0_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg;
	color_sharp_dm_cds_peaking_gain_cm1_v_RBUS	color_sharp_dm_cds_peaking_gain_cm1_v_reg;
	color_sharp_dm_cds_peaking_bound_cm1_v_RBUS	color_sharp_dm_cds_peaking_bound_cm1_v_reg;
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg;
	color_sharp_dm_cds_peaking_gain_cm2_v_RBUS	color_sharp_dm_cds_peaking_gain_cm2_v_reg;
	color_sharp_dm_cds_peaking_bound_cm2_v_RBUS	color_sharp_dm_cds_peaking_bound_cm2_v_reg;
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg;
	color_sharp_dm_cds_peaking_gain_cm3_v_RBUS	color_sharp_dm_cds_peaking_gain_cm3_v_reg;
	color_sharp_dm_cds_peaking_bound_cm3_v_RBUS	color_sharp_dm_cds_peaking_bound_cm3_v_reg;
	color_sharp_dm_cds_peaking_gain_cm3_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg;

	color_sharp_shp_cds_lpf_weight_1_RBUS color_sharp_shp_cds_lpf_weight_1_reg;
	color_sharp_shp_cds_lpf_weight_2_RBUS color_sharp_shp_cds_lpf_weight_2_reg;
	/* Read from register*/
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_BOUND_CTRL_reg);
	color_sharp_dm_cds_cm0_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_0_reg);
	color_sharp_dm_cds_cm0_v_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM0_V_BOUND_0_reg);
	color_sharp_dm_cds_cm0_u_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_1_reg);
	color_sharp_dm_cds_cm0_v_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM0_V_BOUND_1_reg);
	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_EDGE_reg);

	color_sharp_dm_cds_cm1_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_0_reg);
	color_sharp_dm_cds_cm1_v_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_V_BOUND_0_reg);
	color_sharp_dm_cds_cm1_u_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_1_reg);
	color_sharp_dm_cds_cm1_v_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_V_BOUND_1_reg);
	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_EDGE_reg);

	color_sharp_dm_cds_cm2_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_0_reg);
	color_sharp_dm_cds_cm2_v_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM2_V_BOUND_0_reg);
	color_sharp_dm_cds_cm2_u_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_1_reg);
	color_sharp_dm_cds_cm2_v_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM2_V_BOUND_1_reg);
	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_EDGE_reg);

	color_sharp_dm_cds_cm3_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM3_U_BOUND_0_reg);
	color_sharp_dm_cds_cm3_v_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM3_V_BOUND_0_reg);
	color_sharp_dm_cds_cm3_u_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM3_U_BOUND_1_reg);
	color_sharp_dm_cds_cm3_v_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM3_V_BOUND_1_reg);
	color_sharp_dm_cds_peaking_gain_cm3_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm3_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_EDGE_reg);

	color_sharp_dm_cds_peaking_gain_cm0_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_reg);
	color_sharp_dm_cds_peaking_bound_cm0_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_reg);
	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_EDG_reg);
	color_sharp_dm_cds_peaking_gain_cm1_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_reg);
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_reg);
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_EDG_reg);
	color_sharp_dm_cds_peaking_gain_cm2_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_reg);
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_reg);
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_EDG_reg);
	color_sharp_dm_cds_peaking_gain_cm3_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_V_reg);
	color_sharp_dm_cds_peaking_bound_cm3_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_V_reg);
	color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_V_EDG_reg);

	color_sharp_shp_cds_lpf_weight_1_reg.regValue = IoReg_Read32(COLOR_SHARP_SHP_CDS_LPF_WEIGHT_1_reg);
	color_sharp_shp_cds_lpf_weight_2_reg.regValue = IoReg_Read32(COLOR_SHARP_SHP_CDS_LPF_WEIGHT_2_reg);
	/* Global*/
	color_sharp_dm_cds_cm0_u_bound_0_reg.cds_enable = ptr->CDS_Global_Ctrl.cds_enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_edg_enable = ptr->CDS_Global_Ctrl.cds_edg_enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_tex_enable = ptr->CDS_Global_Ctrl.cds_tex_enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_v_enable = ptr->CDS_Global_Ctrl.cds_v_enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_v_edg_enable = ptr->CDS_Global_Ctrl.cds_v_edg_enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_lpf_weight_enable = ptr->CDS_Global_Ctrl.cds_lpf_weight_enable;

	color_sharp_shp_cds_lpf_weight_1_reg.cds_lpf_weight_lv_0 = ptr->CDS_CM0_Ctrl.lpf_weight;
	color_sharp_shp_cds_lpf_weight_1_reg.cds_lpf_weight_lv_1 = ptr->CDS_CM1_Ctrl.lpf_weight;
	color_sharp_shp_cds_lpf_weight_2_reg.cds_lpf_weight_lv_2 = ptr->CDS_CM2_Ctrl.lpf_weight;
	color_sharp_shp_cds_lpf_weight_2_reg.cds_lpf_weight_lv_3 = ptr->CDS_CM3_Ctrl.lpf_weight;
	/* CM0*/
	if(cm0)
	{
	color_sharp_dm_cds_cm0_u_bound_0_reg.u_lb_0 = ptr->CDS_CM0_Ctrl.u_lb;
	color_sharp_dm_cds_cm0_u_bound_0_reg.u_ub_0 = ptr->CDS_CM0_Ctrl.u_ub;
	color_sharp_dm_cds_cm0_v_bound_0_reg.v_lb_0 = ptr->CDS_CM0_Ctrl.v_lb;
	color_sharp_dm_cds_cm0_v_bound_0_reg.v_ub_0 = ptr->CDS_CM0_Ctrl.v_ub;
	color_sharp_dm_cds_cm0_u_bound_1_reg.cu_0   = ptr->CDS_CM0_Ctrl.cu;
	color_sharp_dm_cds_cm0_u_bound_1_reg.cv_0   = ptr->CDS_CM0_Ctrl.cv;
	color_sharp_dm_cds_cm0_u_bound_1_reg.uv_rad_0 = ptr->CDS_CM0_Ctrl.uv_rad;

	color_sharp_dm_cds_cm0_v_bound_0_reg.cm0_conti_area = ptr->CDS_CM0_Ctrl.cm_conti_area;
	color_sharp_dm_cds_cm0_v_bound_0_reg.cm0_conti_enable = ptr->CDS_CM0_Ctrl.cm_conti_enable;
	color_sharp_dm_cds_cm0_u_bound_0_reg.cds_cm0_enable = ptr->CDS_CM0_Ctrl.cds_cm_enable;
	color_sharp_dm_cds_cm0_v_bound_0_reg.cm0_mode = ptr->CDS_CM0_Ctrl.cm_mode;

	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.gain_pos = ptr->CDS_CM0_Ctrl.tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.gain_neg = ptr->CDS_CM0_Ctrl.tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.lv = ptr->CDS_CM0_Ctrl.tex_lv;
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.hv_pos = ptr->CDS_CM0_Ctrl.tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.hv_neg = ptr->CDS_CM0_Ctrl.tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.gain_pos = ptr->CDS_CM0_Ctrl.edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.gain_neg = ptr->CDS_CM0_Ctrl.edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.lv = ptr->CDS_CM0_Ctrl.edge_lv;
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.hv_pos = ptr->CDS_CM0_Ctrl.edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.hv_neg = ptr->CDS_CM0_Ctrl.edge_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm0_v_reg.gain_pos = ptr->CDS_CM0_Ctrl.v_tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm0_v_reg.gain_neg = ptr->CDS_CM0_Ctrl.v_tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm0_v_reg.lv = ptr->CDS_CM0_Ctrl.v_tex_lv;
	color_sharp_dm_cds_peaking_bound_cm0_v_reg.hv_pos = ptr->CDS_CM0_Ctrl.v_tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm0_v_reg.hv_neg = ptr->CDS_CM0_Ctrl.v_tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.gain_pos = ptr->CDS_CM0_Ctrl.v_edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.gain_neg = ptr->CDS_CM0_Ctrl.v_edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.lv = ptr->CDS_CM0_Ctrl.v_edge_lv;
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.hv_pos = ptr->CDS_CM0_Ctrl.v_edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.hv_neg = ptr->CDS_CM0_Ctrl.v_edge_hv_neg;
	}

	/* CM1*/
	if(cm1)
	{
	color_sharp_dm_cds_cm1_u_bound_0_reg.u_lb_1 = ptr->CDS_CM1_Ctrl.u_lb;
	color_sharp_dm_cds_cm1_u_bound_0_reg.u_ub_1 = ptr->CDS_CM1_Ctrl.u_ub;
	color_sharp_dm_cds_cm1_v_bound_0_reg.v_lb_1 = ptr->CDS_CM1_Ctrl.v_lb;
	color_sharp_dm_cds_cm1_v_bound_0_reg.v_ub_1 = ptr->CDS_CM1_Ctrl.v_ub;
	color_sharp_dm_cds_cm1_u_bound_1_reg.cu_1   = ptr->CDS_CM1_Ctrl.cu;
	color_sharp_dm_cds_cm1_u_bound_1_reg.cv_1   = ptr->CDS_CM1_Ctrl.cv;
	color_sharp_dm_cds_cm1_u_bound_1_reg.uv_rad_1 = ptr->CDS_CM1_Ctrl.uv_rad;

	color_sharp_dm_cds_cm1_v_bound_0_reg.cm1_conti_area = ptr->CDS_CM1_Ctrl.cm_conti_area;
	color_sharp_dm_cds_cm1_v_bound_0_reg.cm1_conti_enable = ptr->CDS_CM1_Ctrl.cm_conti_enable;
	color_sharp_dm_cds_cm1_u_bound_0_reg.cds_cm1_enable = ptr->CDS_CM1_Ctrl.cds_cm_enable;
	color_sharp_dm_cds_cm1_v_bound_0_reg.cm1_mode = ptr->CDS_CM1_Ctrl.cm_mode;

	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.gain_pos = ptr->CDS_CM1_Ctrl.tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.gain_neg = ptr->CDS_CM1_Ctrl.tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.lv = ptr->CDS_CM1_Ctrl.tex_lv;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.hv_pos = ptr->CDS_CM1_Ctrl.tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.hv_neg = ptr->CDS_CM1_Ctrl.tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.gain_pos = ptr->CDS_CM1_Ctrl.edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.gain_neg = ptr->CDS_CM1_Ctrl.edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.lv = ptr->CDS_CM1_Ctrl.edge_lv;
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.hv_pos = ptr->CDS_CM1_Ctrl.edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.hv_neg = ptr->CDS_CM1_Ctrl.edge_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm1_v_reg.gain_pos = ptr->CDS_CM1_Ctrl.v_tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm1_v_reg.gain_neg = ptr->CDS_CM1_Ctrl.v_tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.lv = ptr->CDS_CM1_Ctrl.v_tex_lv;
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.hv_pos = ptr->CDS_CM1_Ctrl.v_tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.hv_neg = ptr->CDS_CM1_Ctrl.v_tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.gain_pos = ptr->CDS_CM1_Ctrl.v_edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.gain_neg = ptr->CDS_CM1_Ctrl.v_edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.lv = ptr->CDS_CM1_Ctrl.v_edge_lv;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.hv_pos = ptr->CDS_CM1_Ctrl.v_edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.hv_neg = ptr->CDS_CM1_Ctrl.v_edge_hv_neg;
	}

	/* CM2*/
	if(cm2)
	{
	color_sharp_dm_cds_cm2_u_bound_0_reg.u_lb_2 = ptr->CDS_CM2_Ctrl.u_lb;
	color_sharp_dm_cds_cm2_u_bound_0_reg.u_ub_2 = ptr->CDS_CM2_Ctrl.u_ub;
	color_sharp_dm_cds_cm2_v_bound_0_reg.v_lb_2 = ptr->CDS_CM2_Ctrl.v_lb;
	color_sharp_dm_cds_cm2_v_bound_0_reg.v_ub_2 = ptr->CDS_CM2_Ctrl.v_ub;
	color_sharp_dm_cds_cm2_u_bound_1_reg.cu_2   = ptr->CDS_CM2_Ctrl.cu;
	color_sharp_dm_cds_cm2_u_bound_1_reg.cv_2   = ptr->CDS_CM2_Ctrl.cv;
	color_sharp_dm_cds_cm2_u_bound_1_reg.uv_rad_2 = ptr->CDS_CM2_Ctrl.uv_rad;

	color_sharp_dm_cds_cm2_v_bound_0_reg.cm2_conti_area = ptr->CDS_CM2_Ctrl.cm_conti_area;
	color_sharp_dm_cds_cm2_v_bound_0_reg.cm2_conti_enable = ptr->CDS_CM2_Ctrl.cm_conti_enable;
	color_sharp_dm_cds_cm2_u_bound_0_reg.cds_cm2_enable = ptr->CDS_CM2_Ctrl.cds_cm_enable;
	color_sharp_dm_cds_cm2_v_bound_0_reg.cm2_mode = ptr->CDS_CM2_Ctrl.cm_mode;

	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.gain_pos = ptr->CDS_CM2_Ctrl.tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.gain_neg = ptr->CDS_CM2_Ctrl.tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.lv = ptr->CDS_CM2_Ctrl.tex_lv;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.hv_pos = ptr->CDS_CM2_Ctrl.tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.hv_neg = ptr->CDS_CM2_Ctrl.tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.gain_pos = ptr->CDS_CM2_Ctrl.edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.gain_neg = ptr->CDS_CM2_Ctrl.edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.lv = ptr->CDS_CM2_Ctrl.edge_lv;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.hv_pos = ptr->CDS_CM2_Ctrl.edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.hv_neg = ptr->CDS_CM2_Ctrl.edge_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm2_v_reg.gain_pos = ptr->CDS_CM2_Ctrl.v_tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm2_v_reg.gain_neg = ptr->CDS_CM2_Ctrl.v_tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.lv = ptr->CDS_CM2_Ctrl.v_tex_lv;
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.hv_pos = ptr->CDS_CM2_Ctrl.v_tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.hv_neg = ptr->CDS_CM2_Ctrl.v_tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.gain_pos = ptr->CDS_CM2_Ctrl.v_edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.gain_neg = ptr->CDS_CM2_Ctrl.v_edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.lv = ptr->CDS_CM2_Ctrl.v_edge_lv;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.hv_pos = ptr->CDS_CM2_Ctrl.v_edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.hv_neg = ptr->CDS_CM2_Ctrl.v_edge_hv_neg;
	}

	/* CM3*/
	if(cm3)
	{
	color_sharp_dm_cds_cm3_u_bound_0_reg.u_lb_3 = ptr->CDS_CM3_Ctrl.u_lb;
	color_sharp_dm_cds_cm3_u_bound_0_reg.u_ub_3 = ptr->CDS_CM3_Ctrl.u_ub;
	color_sharp_dm_cds_cm3_v_bound_0_reg.v_lb_3 = ptr->CDS_CM3_Ctrl.v_lb;
	color_sharp_dm_cds_cm3_v_bound_0_reg.v_ub_3 = ptr->CDS_CM3_Ctrl.v_ub;
	color_sharp_dm_cds_cm3_u_bound_1_reg.cu_3   = ptr->CDS_CM3_Ctrl.cu;
	color_sharp_dm_cds_cm3_u_bound_1_reg.cv_3   = ptr->CDS_CM3_Ctrl.cv;
	color_sharp_dm_cds_cm3_u_bound_1_reg.uv_rad_3 = ptr->CDS_CM3_Ctrl.uv_rad;

	color_sharp_dm_cds_cm3_v_bound_0_reg.cm3_conti_area = ptr->CDS_CM3_Ctrl.cm_conti_area;
	color_sharp_dm_cds_cm3_v_bound_0_reg.cm3_conti_enable = ptr->CDS_CM3_Ctrl.cm_conti_enable;
	color_sharp_dm_cds_cm3_u_bound_0_reg.cds_cm3_enable = ptr->CDS_CM3_Ctrl.cds_cm_enable;
	color_sharp_dm_cds_cm3_v_bound_0_reg.cm3_mode = ptr->CDS_CM3_Ctrl.cm_mode;

	color_sharp_dm_cds_peaking_gain_cm3_tex_reg.gain_pos = ptr->CDS_CM3_Ctrl.tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm3_tex_reg.gain_neg = ptr->CDS_CM3_Ctrl.tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.lv = ptr->CDS_CM3_Ctrl.tex_lv;
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.hv_pos = ptr->CDS_CM3_Ctrl.tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.hv_neg = ptr->CDS_CM3_Ctrl.tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm3_edge_reg.gain_pos = ptr->CDS_CM3_Ctrl.edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm3_edge_reg.gain_neg = ptr->CDS_CM3_Ctrl.edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.lv = ptr->CDS_CM3_Ctrl.edge_lv;
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.hv_pos = ptr->CDS_CM3_Ctrl.edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.hv_neg = ptr->CDS_CM3_Ctrl.edge_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm3_v_reg.gain_pos = ptr->CDS_CM3_Ctrl.v_tex_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm3_v_reg.gain_neg = ptr->CDS_CM3_Ctrl.v_tex_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm3_v_reg.lv = ptr->CDS_CM3_Ctrl.v_tex_lv;
	color_sharp_dm_cds_peaking_bound_cm3_v_reg.hv_pos = ptr->CDS_CM3_Ctrl.v_tex_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm3_v_reg.hv_neg = ptr->CDS_CM3_Ctrl.v_tex_hv_neg;

	color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg.gain_pos = ptr->CDS_CM3_Ctrl.v_edge_gain_pos;
	color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg.gain_neg = ptr->CDS_CM3_Ctrl.v_edge_gain_neg;
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.lv = ptr->CDS_CM3_Ctrl.v_edge_lv;
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.hv_pos = ptr->CDS_CM3_Ctrl.v_edge_hv_pos;
	color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.hv_neg = ptr->CDS_CM3_Ctrl.v_edge_hv_neg;
	}

	/* Write to register*/
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_BOUND_CTRL_reg, color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_0_reg, color_sharp_dm_cds_cm0_u_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM0_V_BOUND_0_reg, color_sharp_dm_cds_cm0_v_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_1_reg, color_sharp_dm_cds_cm0_u_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM0_V_BOUND_1_reg, color_sharp_dm_cds_cm0_v_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_TEX_reg, color_sharp_dm_cds_peaking_gain_cm0_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_TEX_reg, color_sharp_dm_cds_peaking_bound_cm0_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm0_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm0_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_0_reg, color_sharp_dm_cds_cm1_u_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_V_BOUND_0_reg, color_sharp_dm_cds_cm1_v_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_1_reg, color_sharp_dm_cds_cm1_u_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_V_BOUND_1_reg, color_sharp_dm_cds_cm1_v_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_TEX_reg, color_sharp_dm_cds_peaking_gain_cm1_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_TEX_reg, color_sharp_dm_cds_peaking_bound_cm1_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm1_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm1_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_0_reg, color_sharp_dm_cds_cm2_u_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM2_V_BOUND_0_reg, color_sharp_dm_cds_cm2_v_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_1_reg, color_sharp_dm_cds_cm2_u_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM2_V_BOUND_1_reg, color_sharp_dm_cds_cm2_v_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_TEX_reg, color_sharp_dm_cds_peaking_gain_cm2_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_TEX_reg, color_sharp_dm_cds_peaking_bound_cm2_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm2_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm2_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_CM3_U_BOUND_0_reg, color_sharp_dm_cds_cm3_u_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM3_V_BOUND_0_reg, color_sharp_dm_cds_cm3_v_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM3_U_BOUND_1_reg, color_sharp_dm_cds_cm3_u_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM3_V_BOUND_1_reg, color_sharp_dm_cds_cm3_v_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_TEX_reg, color_sharp_dm_cds_peaking_gain_cm3_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_TEX_reg, color_sharp_dm_cds_peaking_bound_cm3_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm3_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm3_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_reg, color_sharp_dm_cds_peaking_gain_cm0_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_reg, color_sharp_dm_cds_peaking_bound_cm0_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_EDG_reg, color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_EDG_reg, color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_reg, color_sharp_dm_cds_peaking_gain_cm1_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_reg, color_sharp_dm_cds_peaking_bound_cm1_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_EDG_reg, color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_EDG_reg, color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_reg, color_sharp_dm_cds_peaking_gain_cm2_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_reg, color_sharp_dm_cds_peaking_bound_cm2_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_EDG_reg, color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_EDG_reg, color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_V_reg, color_sharp_dm_cds_peaking_gain_cm3_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_V_reg, color_sharp_dm_cds_peaking_bound_cm3_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_V_EDG_reg, color_sharp_dm_cds_peaking_gain_cm3_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_V_EDG_reg, color_sharp_dm_cds_peaking_bound_cm3_v_edg_reg.regValue);

	IoReg_Write32(COLOR_SHARP_SHP_CDS_LPF_WEIGHT_1_reg, color_sharp_shp_cds_lpf_weight_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_CDS_LPF_WEIGHT_2_reg, color_sharp_shp_cds_lpf_weight_2_reg.regValue);
}
#endif

#define count_labelpk 2

short a_labelpk_table[count_labelpk][14][10]={
	// #0 for vip table access
	{
		{ 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 0 }
		,{ 6 , 38 , 70 , 88 , 106 , 126 , 146 , 186 , 206 , 0 }
		,{ 30 , 45 , 60 , 75 , 90 , 105 , 120 , 135 , 150 , 0 }
		,{ 32 , 28 , 24 , 20 , 16 , 12 , 8 , 4 , 0 , 0 }
		,{ 16 , 12 , 8 , 4 , 0 , 0 , 0 , -8 , -8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ 16 , 12 , 8 , 4 , 0 , 0 , 0 , -8 , -8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
	}
	,
	{
		{ 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 0 }
		,{ 6 , 38 , 70 , 88 , 106 , 126 , 146 , 186 , 206 , 0 }
		,{ 30 , 45 , 60 , 75 , 90 , 105 , 120 , 135 , 150 , 0 }
		,{ 32 , 28 , 24 , 20 , 16 , 12 , 8 , 4 , 0 , 0 }
		,{ 16 , 12 , 8 , 4 , 0 , 0 , 0 , -8 , -8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ 16 , 12 , 8 , 4 , 0 , 0 , 0 , -8 , -8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
		,{ -32 , -24 , -16 , -8 , 0 , 0 , 0 , 8 , 8 , 0 }
	}
};

void drvif_labelpk(short Level)
{
	color_sharp_pqmask_enable_RBUS	reg_color_sharp_pqmask_enable_reg;
	color_sharp_pqmask_sobel_upbnd_0_RBUS	reg_color_sharp_pqmask_sobel_upbnd_0_reg;
	color_sharp_pqmask_sobel_upbnd_1_RBUS	reg_color_sharp_pqmask_sobel_upbnd_1_reg;
	color_sharp_pqmask_sobel_upbnd_2_RBUS	reg_color_sharp_pqmask_sobel_upbnd_2_reg;
	color_sharp_pqmask_zdiff_upbnd_0_RBUS	reg_color_sharp_pqmask_zdiff_upbnd_0_reg;
	color_sharp_pqmask_zdiff_upbnd_1_RBUS	reg_color_sharp_pqmask_zdiff_upbnd_1_reg;
	color_sharp_pqmask_zdiff_upbnd_2_RBUS	reg_color_sharp_pqmask_zdiff_upbnd_2_reg;
	color_sharp_pqmask_tex_upbnd_0_RBUS	reg_color_sharp_pqmask_tex_upbnd_0_reg;
	color_sharp_pqmask_tex_upbnd_1_RBUS	reg_color_sharp_pqmask_tex_upbnd_1_reg;
	color_sharp_pqmask_tex_upbnd_2_RBUS	reg_color_sharp_pqmask_tex_upbnd_2_reg;
	color_sharp_pqmask_tex_lv_0_RBUS	reg_color_sharp_pqmask_tex_lv_0_reg;
	color_sharp_pqmask_tex_lv_1_RBUS	reg_color_sharp_pqmask_tex_lv_1_reg;
	color_sharp_pqmask_tex_lv_2_RBUS	reg_color_sharp_pqmask_tex_lv_2_reg;
	color_sharp_pqmask_tex_gain_pos_0_RBUS	reg_color_sharp_pqmask_tex_gain_pos_0_reg;
	color_sharp_pqmask_tex_gain_pos_1_RBUS	reg_color_sharp_pqmask_tex_gain_pos_1_reg;
	color_sharp_pqmask_tex_gain_pos_2_RBUS	reg_color_sharp_pqmask_tex_gain_pos_2_reg;
	color_sharp_pqmask_tex_gain_neg_0_RBUS	reg_color_sharp_pqmask_tex_gain_neg_0_reg;
	color_sharp_pqmask_tex_gain_neg_1_RBUS	reg_color_sharp_pqmask_tex_gain_neg_1_reg;
	color_sharp_pqmask_tex_gain_neg_2_RBUS	reg_color_sharp_pqmask_tex_gain_neg_2_reg;
	color_sharp_pqmask_tex_hv_pos_0_RBUS	reg_color_sharp_pqmask_tex_hv_pos_0_reg;
	color_sharp_pqmask_tex_hv_pos_1_RBUS	reg_color_sharp_pqmask_tex_hv_pos_1_reg;
	color_sharp_pqmask_tex_hv_pos_2_RBUS	reg_color_sharp_pqmask_tex_hv_pos_2_reg;
	color_sharp_pqmask_tex_hv_neg_0_RBUS	reg_color_sharp_pqmask_tex_hv_neg_0_reg;
	color_sharp_pqmask_tex_hv_neg_1_RBUS	reg_color_sharp_pqmask_tex_hv_neg_1_reg;
	color_sharp_pqmask_tex_hv_neg_2_RBUS	reg_color_sharp_pqmask_tex_hv_neg_2_reg;
	color_sharp_pqmask_tex_lvv_0_RBUS	reg_color_sharp_pqmask_tex_lvv_0_reg;
	color_sharp_pqmask_tex_lvv_1_RBUS	reg_color_sharp_pqmask_tex_lvv_1_reg;
	color_sharp_pqmask_tex_lvv_2_RBUS	reg_color_sharp_pqmask_tex_lvv_2_reg;
	color_sharp_pqmask_tex_gain_posv_0_RBUS	reg_color_sharp_pqmask_tex_gain_posv_0_reg;
	color_sharp_pqmask_tex_gain_posv_1_RBUS	reg_color_sharp_pqmask_tex_gain_posv_1_reg;
	color_sharp_pqmask_tex_gain_posv_2_RBUS	reg_color_sharp_pqmask_tex_gain_posv_2_reg;
	color_sharp_pqmask_tex_gain_negv_0_RBUS	reg_color_sharp_pqmask_tex_gain_negv_0_reg;
	color_sharp_pqmask_tex_gain_negv_1_RBUS	reg_color_sharp_pqmask_tex_gain_negv_1_reg;
	color_sharp_pqmask_tex_gain_negv_2_RBUS	reg_color_sharp_pqmask_tex_gain_negv_2_reg;
	color_sharp_pqmask_tex_hv_posv_0_RBUS	reg_color_sharp_pqmask_tex_hv_posv_0_reg;
	color_sharp_pqmask_tex_hv_posv_1_RBUS	reg_color_sharp_pqmask_tex_hv_posv_1_reg;
	color_sharp_pqmask_tex_hv_posv_2_RBUS	reg_color_sharp_pqmask_tex_hv_posv_2_reg;
	color_sharp_pqmask_tex_hv_negv_0_RBUS	reg_color_sharp_pqmask_tex_hv_negv_0_reg;
	color_sharp_pqmask_tex_hv_negv_1_RBUS	reg_color_sharp_pqmask_tex_hv_negv_1_reg;
	color_sharp_pqmask_tex_hv_negv_2_RBUS	reg_color_sharp_pqmask_tex_hv_negv_2_reg;
	if( Level<0 || Level>=count_labelpk ){return;}
	reg_color_sharp_pqmask_enable_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_enable_reg);
	reg_color_sharp_pqmask_sobel_upbnd_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_sobel_upbnd_0_reg);
	reg_color_sharp_pqmask_sobel_upbnd_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_sobel_upbnd_1_reg);
	reg_color_sharp_pqmask_sobel_upbnd_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_sobel_upbnd_2_reg);
	reg_color_sharp_pqmask_zdiff_upbnd_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_zdiff_upbnd_0_reg);
	reg_color_sharp_pqmask_zdiff_upbnd_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_zdiff_upbnd_1_reg);
	reg_color_sharp_pqmask_zdiff_upbnd_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_zdiff_upbnd_2_reg);
	reg_color_sharp_pqmask_tex_upbnd_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_upbnd_0_reg);
	reg_color_sharp_pqmask_tex_upbnd_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_upbnd_1_reg);
	reg_color_sharp_pqmask_tex_upbnd_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_upbnd_2_reg);
	reg_color_sharp_pqmask_tex_lv_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_lv_0_reg);
	reg_color_sharp_pqmask_tex_lv_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_lv_1_reg);
	reg_color_sharp_pqmask_tex_lv_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_lv_2_reg);
	reg_color_sharp_pqmask_tex_gain_pos_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_pos_0_reg);
	reg_color_sharp_pqmask_tex_gain_pos_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_pos_1_reg);
	reg_color_sharp_pqmask_tex_gain_pos_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_pos_2_reg);
	reg_color_sharp_pqmask_tex_gain_neg_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_neg_0_reg);
	reg_color_sharp_pqmask_tex_gain_neg_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_neg_1_reg);
	reg_color_sharp_pqmask_tex_gain_neg_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_neg_2_reg);
	reg_color_sharp_pqmask_tex_hv_pos_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_pos_0_reg);
	reg_color_sharp_pqmask_tex_hv_pos_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_pos_1_reg);
	reg_color_sharp_pqmask_tex_hv_pos_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_pos_2_reg);
	reg_color_sharp_pqmask_tex_hv_neg_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_neg_0_reg);
	reg_color_sharp_pqmask_tex_hv_neg_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_neg_1_reg);
	reg_color_sharp_pqmask_tex_hv_neg_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_neg_2_reg);
	reg_color_sharp_pqmask_tex_lvv_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_lvv_0_reg);
	reg_color_sharp_pqmask_tex_lvv_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_lvv_1_reg);
	reg_color_sharp_pqmask_tex_lvv_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_lvv_2_reg);
	reg_color_sharp_pqmask_tex_gain_posv_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_posv_0_reg);
	reg_color_sharp_pqmask_tex_gain_posv_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_posv_1_reg);
	reg_color_sharp_pqmask_tex_gain_posv_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_posv_2_reg);
	reg_color_sharp_pqmask_tex_gain_negv_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_negv_0_reg);
	reg_color_sharp_pqmask_tex_gain_negv_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_negv_1_reg);
	reg_color_sharp_pqmask_tex_gain_negv_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_negv_2_reg);
	reg_color_sharp_pqmask_tex_hv_posv_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_posv_0_reg);
	reg_color_sharp_pqmask_tex_hv_posv_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_posv_1_reg);
	reg_color_sharp_pqmask_tex_hv_posv_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_posv_2_reg);
	reg_color_sharp_pqmask_tex_hv_negv_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_negv_0_reg);
	reg_color_sharp_pqmask_tex_hv_negv_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_negv_1_reg);
	reg_color_sharp_pqmask_tex_hv_negv_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_negv_2_reg);
	reg_color_sharp_pqmask_enable_reg.labelpk_sobel_en = a_labelpk_table[Level][0][1];
	reg_color_sharp_pqmask_enable_reg.labelpk_zdiff_en = a_labelpk_table[Level][0][2];
	reg_color_sharp_pqmask_enable_reg.labelpk_tex_en = a_labelpk_table[Level][0][3];
	reg_color_sharp_pqmask_enable_reg.labelpk_tex_modify = a_labelpk_table[Level][0][4];
	reg_color_sharp_pqmask_enable_reg.labelpk_enable = a_labelpk_table[Level][0][0];
	reg_color_sharp_pqmask_sobel_upbnd_0_reg.s3 = a_labelpk_table[Level][1][3];
	reg_color_sharp_pqmask_sobel_upbnd_0_reg.s2 = a_labelpk_table[Level][1][2];
	reg_color_sharp_pqmask_sobel_upbnd_0_reg.s1 = a_labelpk_table[Level][1][1];
	reg_color_sharp_pqmask_sobel_upbnd_0_reg.s0 = a_labelpk_table[Level][1][0];
	reg_color_sharp_pqmask_sobel_upbnd_1_reg.s7 = a_labelpk_table[Level][1][7];
	reg_color_sharp_pqmask_sobel_upbnd_1_reg.s6 = a_labelpk_table[Level][1][6];
	reg_color_sharp_pqmask_sobel_upbnd_1_reg.s5 = a_labelpk_table[Level][1][5];
	reg_color_sharp_pqmask_sobel_upbnd_1_reg.s4 = a_labelpk_table[Level][1][4];
	reg_color_sharp_pqmask_sobel_upbnd_2_reg.s8 = a_labelpk_table[Level][1][8];
	reg_color_sharp_pqmask_zdiff_upbnd_0_reg.s3 = a_labelpk_table[Level][2][3];
	reg_color_sharp_pqmask_zdiff_upbnd_0_reg.s2 = a_labelpk_table[Level][2][2];
	reg_color_sharp_pqmask_zdiff_upbnd_0_reg.s1 = a_labelpk_table[Level][2][1];
	reg_color_sharp_pqmask_zdiff_upbnd_0_reg.s0 = a_labelpk_table[Level][2][0];
	reg_color_sharp_pqmask_zdiff_upbnd_1_reg.s7 = a_labelpk_table[Level][2][7];
	reg_color_sharp_pqmask_zdiff_upbnd_1_reg.s6 = a_labelpk_table[Level][2][6];
	reg_color_sharp_pqmask_zdiff_upbnd_1_reg.s5 = a_labelpk_table[Level][2][5];
	reg_color_sharp_pqmask_zdiff_upbnd_1_reg.s4 = a_labelpk_table[Level][2][4];
	reg_color_sharp_pqmask_zdiff_upbnd_2_reg.s8 = a_labelpk_table[Level][2][8];
	reg_color_sharp_pqmask_tex_upbnd_0_reg.s3 = a_labelpk_table[Level][3][3];
	reg_color_sharp_pqmask_tex_upbnd_0_reg.s2 = a_labelpk_table[Level][3][2];
	reg_color_sharp_pqmask_tex_upbnd_0_reg.s1 = a_labelpk_table[Level][3][1];
	reg_color_sharp_pqmask_tex_upbnd_0_reg.s0 = a_labelpk_table[Level][3][0];
	reg_color_sharp_pqmask_tex_upbnd_1_reg.s7 = a_labelpk_table[Level][3][7];
	reg_color_sharp_pqmask_tex_upbnd_1_reg.s6 = a_labelpk_table[Level][3][6];
	reg_color_sharp_pqmask_tex_upbnd_1_reg.s5 = a_labelpk_table[Level][3][5];
	reg_color_sharp_pqmask_tex_upbnd_1_reg.s4 = a_labelpk_table[Level][3][4];
	reg_color_sharp_pqmask_tex_upbnd_2_reg.s8 = a_labelpk_table[Level][3][8];
	reg_color_sharp_pqmask_tex_lv_0_reg.s3 = a_labelpk_table[Level][4][3];
	reg_color_sharp_pqmask_tex_lv_0_reg.s2 = a_labelpk_table[Level][4][2];
	reg_color_sharp_pqmask_tex_lv_0_reg.s1 = a_labelpk_table[Level][4][1];
	reg_color_sharp_pqmask_tex_lv_0_reg.s0 = a_labelpk_table[Level][4][0];
	reg_color_sharp_pqmask_tex_lv_1_reg.s7 = a_labelpk_table[Level][4][7];
	reg_color_sharp_pqmask_tex_lv_1_reg.s6 = a_labelpk_table[Level][4][6];
	reg_color_sharp_pqmask_tex_lv_1_reg.s5 = a_labelpk_table[Level][4][5];
	reg_color_sharp_pqmask_tex_lv_1_reg.s4 = a_labelpk_table[Level][4][4];
	reg_color_sharp_pqmask_tex_lv_2_reg.s8 = a_labelpk_table[Level][4][8];
	reg_color_sharp_pqmask_tex_gain_pos_0_reg.s3 = a_labelpk_table[Level][5][3];
	reg_color_sharp_pqmask_tex_gain_pos_0_reg.s2 = a_labelpk_table[Level][5][2];
	reg_color_sharp_pqmask_tex_gain_pos_0_reg.s1 = a_labelpk_table[Level][5][1];
	reg_color_sharp_pqmask_tex_gain_pos_0_reg.s0 = a_labelpk_table[Level][5][0];
	reg_color_sharp_pqmask_tex_gain_pos_1_reg.s7 = a_labelpk_table[Level][5][7];
	reg_color_sharp_pqmask_tex_gain_pos_1_reg.s6 = a_labelpk_table[Level][5][6];
	reg_color_sharp_pqmask_tex_gain_pos_1_reg.s5 = a_labelpk_table[Level][5][5];
	reg_color_sharp_pqmask_tex_gain_pos_1_reg.s4 = a_labelpk_table[Level][5][4];
	reg_color_sharp_pqmask_tex_gain_pos_2_reg.s8 = a_labelpk_table[Level][5][8];
	reg_color_sharp_pqmask_tex_gain_neg_0_reg.s3 = a_labelpk_table[Level][6][3];
	reg_color_sharp_pqmask_tex_gain_neg_0_reg.s2 = a_labelpk_table[Level][6][2];
	reg_color_sharp_pqmask_tex_gain_neg_0_reg.s1 = a_labelpk_table[Level][6][1];
	reg_color_sharp_pqmask_tex_gain_neg_0_reg.s0 = a_labelpk_table[Level][6][0];
	reg_color_sharp_pqmask_tex_gain_neg_1_reg.s7 = a_labelpk_table[Level][6][7];
	reg_color_sharp_pqmask_tex_gain_neg_1_reg.s6 = a_labelpk_table[Level][6][6];
	reg_color_sharp_pqmask_tex_gain_neg_1_reg.s5 = a_labelpk_table[Level][6][5];
	reg_color_sharp_pqmask_tex_gain_neg_1_reg.s4 = a_labelpk_table[Level][6][4];
	reg_color_sharp_pqmask_tex_gain_neg_2_reg.s8 = a_labelpk_table[Level][6][8];
	reg_color_sharp_pqmask_tex_hv_pos_0_reg.s3 = a_labelpk_table[Level][7][3];
	reg_color_sharp_pqmask_tex_hv_pos_0_reg.s2 = a_labelpk_table[Level][7][2];
	reg_color_sharp_pqmask_tex_hv_pos_0_reg.s1 = a_labelpk_table[Level][7][1];
	reg_color_sharp_pqmask_tex_hv_pos_0_reg.s0 = a_labelpk_table[Level][7][0];
	reg_color_sharp_pqmask_tex_hv_pos_1_reg.s7 = a_labelpk_table[Level][7][7];
	reg_color_sharp_pqmask_tex_hv_pos_1_reg.s6 = a_labelpk_table[Level][7][6];
	reg_color_sharp_pqmask_tex_hv_pos_1_reg.s5 = a_labelpk_table[Level][7][5];
	reg_color_sharp_pqmask_tex_hv_pos_1_reg.s4 = a_labelpk_table[Level][7][4];
	reg_color_sharp_pqmask_tex_hv_pos_2_reg.s8 = a_labelpk_table[Level][7][8];
	reg_color_sharp_pqmask_tex_hv_neg_0_reg.s3 = a_labelpk_table[Level][8][3];
	reg_color_sharp_pqmask_tex_hv_neg_0_reg.s2 = a_labelpk_table[Level][8][2];
	reg_color_sharp_pqmask_tex_hv_neg_0_reg.s1 = a_labelpk_table[Level][8][1];
	reg_color_sharp_pqmask_tex_hv_neg_0_reg.s0 = a_labelpk_table[Level][8][0];
	reg_color_sharp_pqmask_tex_hv_neg_1_reg.s7 = a_labelpk_table[Level][8][7];
	reg_color_sharp_pqmask_tex_hv_neg_1_reg.s6 = a_labelpk_table[Level][8][6];
	reg_color_sharp_pqmask_tex_hv_neg_1_reg.s5 = a_labelpk_table[Level][8][5];
	reg_color_sharp_pqmask_tex_hv_neg_1_reg.s4 = a_labelpk_table[Level][8][4];
	reg_color_sharp_pqmask_tex_hv_neg_2_reg.s8 = a_labelpk_table[Level][8][8];
	reg_color_sharp_pqmask_tex_lvv_0_reg.s3 = a_labelpk_table[Level][9][3];
	reg_color_sharp_pqmask_tex_lvv_0_reg.s2 = a_labelpk_table[Level][9][2];
	reg_color_sharp_pqmask_tex_lvv_0_reg.s1 = a_labelpk_table[Level][9][1];
	reg_color_sharp_pqmask_tex_lvv_0_reg.s0 = a_labelpk_table[Level][9][0];
	reg_color_sharp_pqmask_tex_lvv_1_reg.s7 = a_labelpk_table[Level][9][7];
	reg_color_sharp_pqmask_tex_lvv_1_reg.s6 = a_labelpk_table[Level][9][6];
	reg_color_sharp_pqmask_tex_lvv_1_reg.s5 = a_labelpk_table[Level][9][5];
	reg_color_sharp_pqmask_tex_lvv_1_reg.s4 = a_labelpk_table[Level][9][4];
	reg_color_sharp_pqmask_tex_lvv_2_reg.s8 = a_labelpk_table[Level][9][8];
	reg_color_sharp_pqmask_tex_gain_posv_0_reg.s3 = a_labelpk_table[Level][10][3];
	reg_color_sharp_pqmask_tex_gain_posv_0_reg.s2 = a_labelpk_table[Level][10][2];
	reg_color_sharp_pqmask_tex_gain_posv_0_reg.s1 = a_labelpk_table[Level][10][1];
	reg_color_sharp_pqmask_tex_gain_posv_0_reg.s0 = a_labelpk_table[Level][10][0];
	reg_color_sharp_pqmask_tex_gain_posv_1_reg.s7 = a_labelpk_table[Level][10][7];
	reg_color_sharp_pqmask_tex_gain_posv_1_reg.s6 = a_labelpk_table[Level][10][6];
	reg_color_sharp_pqmask_tex_gain_posv_1_reg.s5 = a_labelpk_table[Level][10][5];
	reg_color_sharp_pqmask_tex_gain_posv_1_reg.s4 = a_labelpk_table[Level][10][4];
	reg_color_sharp_pqmask_tex_gain_posv_2_reg.s8 = a_labelpk_table[Level][10][8];
	reg_color_sharp_pqmask_tex_gain_negv_0_reg.s3 = a_labelpk_table[Level][11][3];
	reg_color_sharp_pqmask_tex_gain_negv_0_reg.s2 = a_labelpk_table[Level][11][2];
	reg_color_sharp_pqmask_tex_gain_negv_0_reg.s1 = a_labelpk_table[Level][11][1];
	reg_color_sharp_pqmask_tex_gain_negv_0_reg.s0 = a_labelpk_table[Level][11][0];
	reg_color_sharp_pqmask_tex_gain_negv_1_reg.s7 = a_labelpk_table[Level][11][7];
	reg_color_sharp_pqmask_tex_gain_negv_1_reg.s6 = a_labelpk_table[Level][11][6];
	reg_color_sharp_pqmask_tex_gain_negv_1_reg.s5 = a_labelpk_table[Level][11][5];
	reg_color_sharp_pqmask_tex_gain_negv_1_reg.s4 = a_labelpk_table[Level][11][4];
	reg_color_sharp_pqmask_tex_gain_negv_2_reg.s8 = a_labelpk_table[Level][11][8];
	reg_color_sharp_pqmask_tex_hv_posv_0_reg.s3 = a_labelpk_table[Level][12][3];
	reg_color_sharp_pqmask_tex_hv_posv_0_reg.s2 = a_labelpk_table[Level][12][2];
	reg_color_sharp_pqmask_tex_hv_posv_0_reg.s1 = a_labelpk_table[Level][12][1];
	reg_color_sharp_pqmask_tex_hv_posv_0_reg.s0 = a_labelpk_table[Level][12][0];
	reg_color_sharp_pqmask_tex_hv_posv_1_reg.s7 = a_labelpk_table[Level][12][7];
	reg_color_sharp_pqmask_tex_hv_posv_1_reg.s6 = a_labelpk_table[Level][12][6];
	reg_color_sharp_pqmask_tex_hv_posv_1_reg.s5 = a_labelpk_table[Level][12][5];
	reg_color_sharp_pqmask_tex_hv_posv_1_reg.s4 = a_labelpk_table[Level][12][4];
	reg_color_sharp_pqmask_tex_hv_posv_2_reg.s8 = a_labelpk_table[Level][12][8];
	reg_color_sharp_pqmask_tex_hv_negv_0_reg.s3 = a_labelpk_table[Level][13][3];
	reg_color_sharp_pqmask_tex_hv_negv_0_reg.s2 = a_labelpk_table[Level][13][2];
	reg_color_sharp_pqmask_tex_hv_negv_0_reg.s1 = a_labelpk_table[Level][13][1];
	reg_color_sharp_pqmask_tex_hv_negv_0_reg.s0 = a_labelpk_table[Level][13][0];
	reg_color_sharp_pqmask_tex_hv_negv_1_reg.s7 = a_labelpk_table[Level][13][7];
	reg_color_sharp_pqmask_tex_hv_negv_1_reg.s6 = a_labelpk_table[Level][13][6];
	reg_color_sharp_pqmask_tex_hv_negv_1_reg.s5 = a_labelpk_table[Level][13][5];
	reg_color_sharp_pqmask_tex_hv_negv_1_reg.s4 = a_labelpk_table[Level][13][4];
	reg_color_sharp_pqmask_tex_hv_negv_2_reg.s8 = a_labelpk_table[Level][13][8];
	IoReg_Write32(COLOR_SHARP_pqmask_enable_reg, reg_color_sharp_pqmask_enable_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_sobel_upbnd_0_reg, reg_color_sharp_pqmask_sobel_upbnd_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_sobel_upbnd_1_reg, reg_color_sharp_pqmask_sobel_upbnd_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_sobel_upbnd_2_reg, reg_color_sharp_pqmask_sobel_upbnd_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_zdiff_upbnd_0_reg, reg_color_sharp_pqmask_zdiff_upbnd_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_zdiff_upbnd_1_reg, reg_color_sharp_pqmask_zdiff_upbnd_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_zdiff_upbnd_2_reg, reg_color_sharp_pqmask_zdiff_upbnd_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_upbnd_0_reg, reg_color_sharp_pqmask_tex_upbnd_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_upbnd_1_reg, reg_color_sharp_pqmask_tex_upbnd_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_upbnd_2_reg, reg_color_sharp_pqmask_tex_upbnd_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_lv_0_reg, reg_color_sharp_pqmask_tex_lv_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_lv_1_reg, reg_color_sharp_pqmask_tex_lv_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_lv_2_reg, reg_color_sharp_pqmask_tex_lv_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_pos_0_reg, reg_color_sharp_pqmask_tex_gain_pos_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_pos_1_reg, reg_color_sharp_pqmask_tex_gain_pos_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_pos_2_reg, reg_color_sharp_pqmask_tex_gain_pos_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_neg_0_reg, reg_color_sharp_pqmask_tex_gain_neg_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_neg_1_reg, reg_color_sharp_pqmask_tex_gain_neg_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_neg_2_reg, reg_color_sharp_pqmask_tex_gain_neg_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_pos_0_reg, reg_color_sharp_pqmask_tex_hv_pos_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_pos_1_reg, reg_color_sharp_pqmask_tex_hv_pos_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_pos_2_reg, reg_color_sharp_pqmask_tex_hv_pos_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_neg_0_reg, reg_color_sharp_pqmask_tex_hv_neg_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_neg_1_reg, reg_color_sharp_pqmask_tex_hv_neg_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_neg_2_reg, reg_color_sharp_pqmask_tex_hv_neg_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_lvv_0_reg, reg_color_sharp_pqmask_tex_lvv_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_lvv_1_reg, reg_color_sharp_pqmask_tex_lvv_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_lvv_2_reg, reg_color_sharp_pqmask_tex_lvv_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_posv_0_reg, reg_color_sharp_pqmask_tex_gain_posv_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_posv_1_reg, reg_color_sharp_pqmask_tex_gain_posv_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_posv_2_reg, reg_color_sharp_pqmask_tex_gain_posv_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_negv_0_reg, reg_color_sharp_pqmask_tex_gain_negv_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_negv_1_reg, reg_color_sharp_pqmask_tex_gain_negv_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_negv_2_reg, reg_color_sharp_pqmask_tex_gain_negv_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_posv_0_reg, reg_color_sharp_pqmask_tex_hv_posv_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_posv_1_reg, reg_color_sharp_pqmask_tex_hv_posv_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_posv_2_reg, reg_color_sharp_pqmask_tex_hv_posv_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_negv_0_reg, reg_color_sharp_pqmask_tex_hv_negv_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_negv_1_reg, reg_color_sharp_pqmask_tex_hv_negv_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_negv_2_reg, reg_color_sharp_pqmask_tex_hv_negv_2_reg.regValue);
}

void drvif_set_labelpk(DRV_LABEL_PK *ptr)
{
	color_sharp_pqmask_enable_RBUS	reg_color_sharp_pqmask_enable_reg;
	color_sharp_pqmask_sobel_upbnd_0_RBUS	reg_color_sharp_pqmask_sobel_upbnd_0_reg;
	color_sharp_pqmask_sobel_upbnd_1_RBUS	reg_color_sharp_pqmask_sobel_upbnd_1_reg;
	color_sharp_pqmask_sobel_upbnd_2_RBUS	reg_color_sharp_pqmask_sobel_upbnd_2_reg;
	color_sharp_pqmask_zdiff_upbnd_0_RBUS	reg_color_sharp_pqmask_zdiff_upbnd_0_reg;
	color_sharp_pqmask_zdiff_upbnd_1_RBUS	reg_color_sharp_pqmask_zdiff_upbnd_1_reg;
	color_sharp_pqmask_zdiff_upbnd_2_RBUS	reg_color_sharp_pqmask_zdiff_upbnd_2_reg;
	color_sharp_pqmask_tex_upbnd_0_RBUS	reg_color_sharp_pqmask_tex_upbnd_0_reg;
	color_sharp_pqmask_tex_upbnd_1_RBUS	reg_color_sharp_pqmask_tex_upbnd_1_reg;
	color_sharp_pqmask_tex_upbnd_2_RBUS	reg_color_sharp_pqmask_tex_upbnd_2_reg;
	color_sharp_pqmask_tex_lv_0_RBUS	reg_color_sharp_pqmask_tex_lv_0_reg;
	color_sharp_pqmask_tex_lv_1_RBUS	reg_color_sharp_pqmask_tex_lv_1_reg;
	color_sharp_pqmask_tex_lv_2_RBUS	reg_color_sharp_pqmask_tex_lv_2_reg;
	color_sharp_pqmask_tex_gain_pos_0_RBUS	reg_color_sharp_pqmask_tex_gain_pos_0_reg;
	color_sharp_pqmask_tex_gain_pos_1_RBUS	reg_color_sharp_pqmask_tex_gain_pos_1_reg;
	color_sharp_pqmask_tex_gain_pos_2_RBUS	reg_color_sharp_pqmask_tex_gain_pos_2_reg;
	color_sharp_pqmask_tex_gain_neg_0_RBUS	reg_color_sharp_pqmask_tex_gain_neg_0_reg;
	color_sharp_pqmask_tex_gain_neg_1_RBUS	reg_color_sharp_pqmask_tex_gain_neg_1_reg;
	color_sharp_pqmask_tex_gain_neg_2_RBUS	reg_color_sharp_pqmask_tex_gain_neg_2_reg;
	color_sharp_pqmask_tex_hv_pos_0_RBUS	reg_color_sharp_pqmask_tex_hv_pos_0_reg;
	color_sharp_pqmask_tex_hv_pos_1_RBUS	reg_color_sharp_pqmask_tex_hv_pos_1_reg;
	color_sharp_pqmask_tex_hv_pos_2_RBUS	reg_color_sharp_pqmask_tex_hv_pos_2_reg;
	color_sharp_pqmask_tex_hv_neg_0_RBUS	reg_color_sharp_pqmask_tex_hv_neg_0_reg;
	color_sharp_pqmask_tex_hv_neg_1_RBUS	reg_color_sharp_pqmask_tex_hv_neg_1_reg;
	color_sharp_pqmask_tex_hv_neg_2_RBUS	reg_color_sharp_pqmask_tex_hv_neg_2_reg;
	color_sharp_pqmask_tex_lvv_0_RBUS	reg_color_sharp_pqmask_tex_lvv_0_reg;
	color_sharp_pqmask_tex_lvv_1_RBUS	reg_color_sharp_pqmask_tex_lvv_1_reg;
	color_sharp_pqmask_tex_lvv_2_RBUS	reg_color_sharp_pqmask_tex_lvv_2_reg;
	color_sharp_pqmask_tex_gain_posv_0_RBUS	reg_color_sharp_pqmask_tex_gain_posv_0_reg;
	color_sharp_pqmask_tex_gain_posv_1_RBUS	reg_color_sharp_pqmask_tex_gain_posv_1_reg;
	color_sharp_pqmask_tex_gain_posv_2_RBUS	reg_color_sharp_pqmask_tex_gain_posv_2_reg;
	color_sharp_pqmask_tex_gain_negv_0_RBUS	reg_color_sharp_pqmask_tex_gain_negv_0_reg;
	color_sharp_pqmask_tex_gain_negv_1_RBUS	reg_color_sharp_pqmask_tex_gain_negv_1_reg;
	color_sharp_pqmask_tex_gain_negv_2_RBUS	reg_color_sharp_pqmask_tex_gain_negv_2_reg;
	color_sharp_pqmask_tex_hv_posv_0_RBUS	reg_color_sharp_pqmask_tex_hv_posv_0_reg;
	color_sharp_pqmask_tex_hv_posv_1_RBUS	reg_color_sharp_pqmask_tex_hv_posv_1_reg;
	color_sharp_pqmask_tex_hv_posv_2_RBUS	reg_color_sharp_pqmask_tex_hv_posv_2_reg;
	color_sharp_pqmask_tex_hv_negv_0_RBUS	reg_color_sharp_pqmask_tex_hv_negv_0_reg;
	color_sharp_pqmask_tex_hv_negv_1_RBUS	reg_color_sharp_pqmask_tex_hv_negv_1_reg;
	color_sharp_pqmask_tex_hv_negv_2_RBUS	reg_color_sharp_pqmask_tex_hv_negv_2_reg;

	reg_color_sharp_pqmask_enable_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_enable_reg);
	reg_color_sharp_pqmask_sobel_upbnd_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_sobel_upbnd_0_reg);
	reg_color_sharp_pqmask_sobel_upbnd_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_sobel_upbnd_1_reg);
	reg_color_sharp_pqmask_sobel_upbnd_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_sobel_upbnd_2_reg);
	reg_color_sharp_pqmask_zdiff_upbnd_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_zdiff_upbnd_0_reg);
	reg_color_sharp_pqmask_zdiff_upbnd_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_zdiff_upbnd_1_reg);
	reg_color_sharp_pqmask_zdiff_upbnd_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_zdiff_upbnd_2_reg);
	reg_color_sharp_pqmask_tex_upbnd_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_upbnd_0_reg);
	reg_color_sharp_pqmask_tex_upbnd_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_upbnd_1_reg);
	reg_color_sharp_pqmask_tex_upbnd_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_upbnd_2_reg);
	reg_color_sharp_pqmask_tex_lv_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_lv_0_reg);
	reg_color_sharp_pqmask_tex_lv_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_lv_1_reg);
	reg_color_sharp_pqmask_tex_lv_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_lv_2_reg);
	reg_color_sharp_pqmask_tex_gain_pos_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_pos_0_reg);
	reg_color_sharp_pqmask_tex_gain_pos_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_pos_1_reg);
	reg_color_sharp_pqmask_tex_gain_pos_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_pos_2_reg);
	reg_color_sharp_pqmask_tex_gain_neg_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_neg_0_reg);
	reg_color_sharp_pqmask_tex_gain_neg_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_neg_1_reg);
	reg_color_sharp_pqmask_tex_gain_neg_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_neg_2_reg);
	reg_color_sharp_pqmask_tex_hv_pos_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_pos_0_reg);
	reg_color_sharp_pqmask_tex_hv_pos_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_pos_1_reg);
	reg_color_sharp_pqmask_tex_hv_pos_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_pos_2_reg);
	reg_color_sharp_pqmask_tex_hv_neg_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_neg_0_reg);
	reg_color_sharp_pqmask_tex_hv_neg_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_neg_1_reg);
	reg_color_sharp_pqmask_tex_hv_neg_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_neg_2_reg);
	reg_color_sharp_pqmask_tex_lvv_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_lvv_0_reg);
	reg_color_sharp_pqmask_tex_lvv_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_lvv_1_reg);
	reg_color_sharp_pqmask_tex_lvv_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_lvv_2_reg);
	reg_color_sharp_pqmask_tex_gain_posv_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_posv_0_reg);
	reg_color_sharp_pqmask_tex_gain_posv_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_posv_1_reg);
	reg_color_sharp_pqmask_tex_gain_posv_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_posv_2_reg);
	reg_color_sharp_pqmask_tex_gain_negv_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_negv_0_reg);
	reg_color_sharp_pqmask_tex_gain_negv_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_negv_1_reg);
	reg_color_sharp_pqmask_tex_gain_negv_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_gain_negv_2_reg);
	reg_color_sharp_pqmask_tex_hv_posv_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_posv_0_reg);
	reg_color_sharp_pqmask_tex_hv_posv_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_posv_1_reg);
	reg_color_sharp_pqmask_tex_hv_posv_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_posv_2_reg);
	reg_color_sharp_pqmask_tex_hv_negv_0_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_negv_0_reg);
	reg_color_sharp_pqmask_tex_hv_negv_1_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_negv_1_reg);
	reg_color_sharp_pqmask_tex_hv_negv_2_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_tex_hv_negv_2_reg);

	reg_color_sharp_pqmask_enable_reg.labelpk_sobel_en = ptr->bSobelEnable;
	reg_color_sharp_pqmask_enable_reg.labelpk_zdiff_en = ptr->bZDiffEnable;
	reg_color_sharp_pqmask_enable_reg.labelpk_tex_en = ptr->bTextEnable;
	reg_color_sharp_pqmask_enable_reg.labelpk_tex_modify = ptr->bTextModifyEnable;
	reg_color_sharp_pqmask_enable_reg.labelpk_enable = ptr->bLabelSharpnessEnable;
	reg_color_sharp_pqmask_sobel_upbnd_0_reg.s3 = ptr->SobelUpbnd[3];
	reg_color_sharp_pqmask_sobel_upbnd_0_reg.s2 = ptr->SobelUpbnd[2];
	reg_color_sharp_pqmask_sobel_upbnd_0_reg.s1 = ptr->SobelUpbnd[1];
	reg_color_sharp_pqmask_sobel_upbnd_0_reg.s0 = ptr->SobelUpbnd[0];
	reg_color_sharp_pqmask_sobel_upbnd_1_reg.s7 = ptr->SobelUpbnd[7];
	reg_color_sharp_pqmask_sobel_upbnd_1_reg.s6 = ptr->SobelUpbnd[6];
	reg_color_sharp_pqmask_sobel_upbnd_1_reg.s5 = ptr->SobelUpbnd[5];
	reg_color_sharp_pqmask_sobel_upbnd_1_reg.s4 = ptr->SobelUpbnd[4];
	reg_color_sharp_pqmask_sobel_upbnd_2_reg.s8 = ptr->SobelUpbnd[8];
	reg_color_sharp_pqmask_zdiff_upbnd_0_reg.s3 = ptr->ZDiffUpbnd[3];
	reg_color_sharp_pqmask_zdiff_upbnd_0_reg.s2 = ptr->ZDiffUpbnd[2];
	reg_color_sharp_pqmask_zdiff_upbnd_0_reg.s1 = ptr->ZDiffUpbnd[1];
	reg_color_sharp_pqmask_zdiff_upbnd_0_reg.s0 = ptr->ZDiffUpbnd[0];
	reg_color_sharp_pqmask_zdiff_upbnd_1_reg.s7 = ptr->ZDiffUpbnd[7];
	reg_color_sharp_pqmask_zdiff_upbnd_1_reg.s6 = ptr->ZDiffUpbnd[6];
	reg_color_sharp_pqmask_zdiff_upbnd_1_reg.s5 = ptr->ZDiffUpbnd[5];
	reg_color_sharp_pqmask_zdiff_upbnd_1_reg.s4 = ptr->ZDiffUpbnd[4];
	reg_color_sharp_pqmask_zdiff_upbnd_2_reg.s8 = ptr->ZDiffUpbnd[8];
	reg_color_sharp_pqmask_tex_upbnd_0_reg.s3 = ptr->TextUpbnd[3];
	reg_color_sharp_pqmask_tex_upbnd_0_reg.s2 = ptr->TextUpbnd[2];
	reg_color_sharp_pqmask_tex_upbnd_0_reg.s1 = ptr->TextUpbnd[1];
	reg_color_sharp_pqmask_tex_upbnd_0_reg.s0 = ptr->TextUpbnd[0];
	reg_color_sharp_pqmask_tex_upbnd_1_reg.s7 = ptr->TextUpbnd[7];
	reg_color_sharp_pqmask_tex_upbnd_1_reg.s6 = ptr->TextUpbnd[6];
	reg_color_sharp_pqmask_tex_upbnd_1_reg.s5 = ptr->TextUpbnd[5];
	reg_color_sharp_pqmask_tex_upbnd_1_reg.s4 = ptr->TextUpbnd[4];
	reg_color_sharp_pqmask_tex_upbnd_2_reg.s8 = ptr->TextUpbnd[8];
	reg_color_sharp_pqmask_tex_lv_0_reg.s3 = ptr->TextLV[3];
	reg_color_sharp_pqmask_tex_lv_0_reg.s2 = ptr->TextLV[2];
	reg_color_sharp_pqmask_tex_lv_0_reg.s1 = ptr->TextLV[1];
	reg_color_sharp_pqmask_tex_lv_0_reg.s0 = ptr->TextLV[0];
	reg_color_sharp_pqmask_tex_lv_1_reg.s7 = ptr->TextLV[7];
	reg_color_sharp_pqmask_tex_lv_1_reg.s6 = ptr->TextLV[6];
	reg_color_sharp_pqmask_tex_lv_1_reg.s5 = ptr->TextLV[5];
	reg_color_sharp_pqmask_tex_lv_1_reg.s4 = ptr->TextLV[4];
	reg_color_sharp_pqmask_tex_lv_2_reg.s8 = ptr->TextLV[8];
	reg_color_sharp_pqmask_tex_gain_pos_0_reg.s3 = ptr->TextGainPos[3];
	reg_color_sharp_pqmask_tex_gain_pos_0_reg.s2 = ptr->TextGainPos[2];
	reg_color_sharp_pqmask_tex_gain_pos_0_reg.s1 = ptr->TextGainPos[1];
	reg_color_sharp_pqmask_tex_gain_pos_0_reg.s0 = ptr->TextGainPos[0];
	reg_color_sharp_pqmask_tex_gain_pos_1_reg.s7 = ptr->TextGainPos[7];
	reg_color_sharp_pqmask_tex_gain_pos_1_reg.s6 = ptr->TextGainPos[6];
	reg_color_sharp_pqmask_tex_gain_pos_1_reg.s5 = ptr->TextGainPos[5];
	reg_color_sharp_pqmask_tex_gain_pos_1_reg.s4 = ptr->TextGainPos[4];
	reg_color_sharp_pqmask_tex_gain_pos_2_reg.s8 = ptr->TextGainPos[8];
	reg_color_sharp_pqmask_tex_gain_neg_0_reg.s3 = ptr->TextGainNeg[3];
	reg_color_sharp_pqmask_tex_gain_neg_0_reg.s2 = ptr->TextGainNeg[2];
	reg_color_sharp_pqmask_tex_gain_neg_0_reg.s1 = ptr->TextGainNeg[1];
	reg_color_sharp_pqmask_tex_gain_neg_0_reg.s0 = ptr->TextGainNeg[0];
	reg_color_sharp_pqmask_tex_gain_neg_1_reg.s7 = ptr->TextGainNeg[7];
	reg_color_sharp_pqmask_tex_gain_neg_1_reg.s6 = ptr->TextGainNeg[6];
	reg_color_sharp_pqmask_tex_gain_neg_1_reg.s5 = ptr->TextGainNeg[5];
	reg_color_sharp_pqmask_tex_gain_neg_1_reg.s4 = ptr->TextGainNeg[4];
	reg_color_sharp_pqmask_tex_gain_neg_2_reg.s8 = ptr->TextGainNeg[8];
	reg_color_sharp_pqmask_tex_hv_pos_0_reg.s3 = ptr->TextHVPos[3];
	reg_color_sharp_pqmask_tex_hv_pos_0_reg.s2 = ptr->TextHVPos[2];
	reg_color_sharp_pqmask_tex_hv_pos_0_reg.s1 = ptr->TextHVPos[1];
	reg_color_sharp_pqmask_tex_hv_pos_0_reg.s0 = ptr->TextHVPos[0];
	reg_color_sharp_pqmask_tex_hv_pos_1_reg.s7 = ptr->TextHVPos[7];
	reg_color_sharp_pqmask_tex_hv_pos_1_reg.s6 = ptr->TextHVPos[6];
	reg_color_sharp_pqmask_tex_hv_pos_1_reg.s5 = ptr->TextHVPos[5];
	reg_color_sharp_pqmask_tex_hv_pos_1_reg.s4 = ptr->TextHVPos[4];
	reg_color_sharp_pqmask_tex_hv_pos_2_reg.s8 = ptr->TextHVPos[8];
	reg_color_sharp_pqmask_tex_hv_neg_0_reg.s3 = ptr->TextHVNeg[3];
	reg_color_sharp_pqmask_tex_hv_neg_0_reg.s2 = ptr->TextHVNeg[2];
	reg_color_sharp_pqmask_tex_hv_neg_0_reg.s1 = ptr->TextHVNeg[1];
	reg_color_sharp_pqmask_tex_hv_neg_0_reg.s0 = ptr->TextHVNeg[0];
	reg_color_sharp_pqmask_tex_hv_neg_1_reg.s7 = ptr->TextHVNeg[7];
	reg_color_sharp_pqmask_tex_hv_neg_1_reg.s6 = ptr->TextHVNeg[6];
	reg_color_sharp_pqmask_tex_hv_neg_1_reg.s5 = ptr->TextHVNeg[5];
	reg_color_sharp_pqmask_tex_hv_neg_1_reg.s4 = ptr->TextHVNeg[4];
	reg_color_sharp_pqmask_tex_hv_neg_2_reg.s8 = ptr->TextHVNeg[8];
	reg_color_sharp_pqmask_tex_lvv_0_reg.s3 = ptr->TextLVV[3];
	reg_color_sharp_pqmask_tex_lvv_0_reg.s2 = ptr->TextLVV[2];
	reg_color_sharp_pqmask_tex_lvv_0_reg.s1 = ptr->TextLVV[1];
	reg_color_sharp_pqmask_tex_lvv_0_reg.s0 = ptr->TextLVV[0];
	reg_color_sharp_pqmask_tex_lvv_1_reg.s7 = ptr->TextLVV[7];
	reg_color_sharp_pqmask_tex_lvv_1_reg.s6 = ptr->TextLVV[6];
	reg_color_sharp_pqmask_tex_lvv_1_reg.s5 = ptr->TextLVV[5];
	reg_color_sharp_pqmask_tex_lvv_1_reg.s4 = ptr->TextLVV[4];
	reg_color_sharp_pqmask_tex_lvv_2_reg.s8 = ptr->TextLVV[8];
	reg_color_sharp_pqmask_tex_gain_posv_0_reg.s3 = ptr->TextGainPosV[3];
	reg_color_sharp_pqmask_tex_gain_posv_0_reg.s2 = ptr->TextGainPosV[2];
	reg_color_sharp_pqmask_tex_gain_posv_0_reg.s1 = ptr->TextGainPosV[1];
	reg_color_sharp_pqmask_tex_gain_posv_0_reg.s0 = ptr->TextGainPosV[0];
	reg_color_sharp_pqmask_tex_gain_posv_1_reg.s7 = ptr->TextGainPosV[7];
	reg_color_sharp_pqmask_tex_gain_posv_1_reg.s6 = ptr->TextGainPosV[6];
	reg_color_sharp_pqmask_tex_gain_posv_1_reg.s5 = ptr->TextGainPosV[5];
	reg_color_sharp_pqmask_tex_gain_posv_1_reg.s4 = ptr->TextGainPosV[4];
	reg_color_sharp_pqmask_tex_gain_posv_2_reg.s8 = ptr->TextGainPosV[8];
	reg_color_sharp_pqmask_tex_gain_negv_0_reg.s3 = ptr->TextGainNegV[3];
	reg_color_sharp_pqmask_tex_gain_negv_0_reg.s2 = ptr->TextGainNegV[2];
	reg_color_sharp_pqmask_tex_gain_negv_0_reg.s1 = ptr->TextGainNegV[1];
	reg_color_sharp_pqmask_tex_gain_negv_0_reg.s0 = ptr->TextGainNegV[0];
	reg_color_sharp_pqmask_tex_gain_negv_1_reg.s7 = ptr->TextGainNegV[7];
	reg_color_sharp_pqmask_tex_gain_negv_1_reg.s6 = ptr->TextGainNegV[6];
	reg_color_sharp_pqmask_tex_gain_negv_1_reg.s5 = ptr->TextGainNegV[5];
	reg_color_sharp_pqmask_tex_gain_negv_1_reg.s4 = ptr->TextGainNegV[4];
	reg_color_sharp_pqmask_tex_gain_negv_2_reg.s8 = ptr->TextGainNegV[8];
	reg_color_sharp_pqmask_tex_hv_posv_0_reg.s3 = ptr->TextHVPosV[3];
	reg_color_sharp_pqmask_tex_hv_posv_0_reg.s2 = ptr->TextHVPosV[2];
	reg_color_sharp_pqmask_tex_hv_posv_0_reg.s1 = ptr->TextHVPosV[1];
	reg_color_sharp_pqmask_tex_hv_posv_0_reg.s0 = ptr->TextHVPosV[0];
	reg_color_sharp_pqmask_tex_hv_posv_1_reg.s7 = ptr->TextHVPosV[7];
	reg_color_sharp_pqmask_tex_hv_posv_1_reg.s6 = ptr->TextHVPosV[6];
	reg_color_sharp_pqmask_tex_hv_posv_1_reg.s5 = ptr->TextHVPosV[5];
	reg_color_sharp_pqmask_tex_hv_posv_1_reg.s4 = ptr->TextHVPosV[4];
	reg_color_sharp_pqmask_tex_hv_posv_2_reg.s8 = ptr->TextHVPosV[8];
	reg_color_sharp_pqmask_tex_hv_negv_0_reg.s3 = ptr->TextHVNegV[3];
	reg_color_sharp_pqmask_tex_hv_negv_0_reg.s2 = ptr->TextHVNegV[2];
	reg_color_sharp_pqmask_tex_hv_negv_0_reg.s1 = ptr->TextHVNegV[1];
	reg_color_sharp_pqmask_tex_hv_negv_0_reg.s0 = ptr->TextHVNegV[0];
	reg_color_sharp_pqmask_tex_hv_negv_1_reg.s7 = ptr->TextHVNegV[7];
	reg_color_sharp_pqmask_tex_hv_negv_1_reg.s6 = ptr->TextHVNegV[6];
	reg_color_sharp_pqmask_tex_hv_negv_1_reg.s5 = ptr->TextHVNegV[5];
	reg_color_sharp_pqmask_tex_hv_negv_1_reg.s4 = ptr->TextHVNegV[4];
	reg_color_sharp_pqmask_tex_hv_negv_2_reg.s8 = ptr->TextHVNegV[8];

	IoReg_Write32(COLOR_SHARP_pqmask_enable_reg, reg_color_sharp_pqmask_enable_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_sobel_upbnd_0_reg, reg_color_sharp_pqmask_sobel_upbnd_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_sobel_upbnd_1_reg, reg_color_sharp_pqmask_sobel_upbnd_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_sobel_upbnd_2_reg, reg_color_sharp_pqmask_sobel_upbnd_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_zdiff_upbnd_0_reg, reg_color_sharp_pqmask_zdiff_upbnd_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_zdiff_upbnd_1_reg, reg_color_sharp_pqmask_zdiff_upbnd_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_zdiff_upbnd_2_reg, reg_color_sharp_pqmask_zdiff_upbnd_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_upbnd_0_reg, reg_color_sharp_pqmask_tex_upbnd_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_upbnd_1_reg, reg_color_sharp_pqmask_tex_upbnd_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_upbnd_2_reg, reg_color_sharp_pqmask_tex_upbnd_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_lv_0_reg, reg_color_sharp_pqmask_tex_lv_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_lv_1_reg, reg_color_sharp_pqmask_tex_lv_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_lv_2_reg, reg_color_sharp_pqmask_tex_lv_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_pos_0_reg, reg_color_sharp_pqmask_tex_gain_pos_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_pos_1_reg, reg_color_sharp_pqmask_tex_gain_pos_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_pos_2_reg, reg_color_sharp_pqmask_tex_gain_pos_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_neg_0_reg, reg_color_sharp_pqmask_tex_gain_neg_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_neg_1_reg, reg_color_sharp_pqmask_tex_gain_neg_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_neg_2_reg, reg_color_sharp_pqmask_tex_gain_neg_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_pos_0_reg, reg_color_sharp_pqmask_tex_hv_pos_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_pos_1_reg, reg_color_sharp_pqmask_tex_hv_pos_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_pos_2_reg, reg_color_sharp_pqmask_tex_hv_pos_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_neg_0_reg, reg_color_sharp_pqmask_tex_hv_neg_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_neg_1_reg, reg_color_sharp_pqmask_tex_hv_neg_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_neg_2_reg, reg_color_sharp_pqmask_tex_hv_neg_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_lvv_0_reg, reg_color_sharp_pqmask_tex_lvv_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_lvv_1_reg, reg_color_sharp_pqmask_tex_lvv_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_lvv_2_reg, reg_color_sharp_pqmask_tex_lvv_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_posv_0_reg, reg_color_sharp_pqmask_tex_gain_posv_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_posv_1_reg, reg_color_sharp_pqmask_tex_gain_posv_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_posv_2_reg, reg_color_sharp_pqmask_tex_gain_posv_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_negv_0_reg, reg_color_sharp_pqmask_tex_gain_negv_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_negv_1_reg, reg_color_sharp_pqmask_tex_gain_negv_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_gain_negv_2_reg, reg_color_sharp_pqmask_tex_gain_negv_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_posv_0_reg, reg_color_sharp_pqmask_tex_hv_posv_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_posv_1_reg, reg_color_sharp_pqmask_tex_hv_posv_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_posv_2_reg, reg_color_sharp_pqmask_tex_hv_posv_2_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_negv_0_reg, reg_color_sharp_pqmask_tex_hv_negv_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_negv_1_reg, reg_color_sharp_pqmask_tex_hv_negv_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_pqmask_tex_hv_negv_2_reg, reg_color_sharp_pqmask_tex_hv_negv_2_reg.regValue);
}

void drvif_set_labelpk_enable(unsigned char bEnable) {

	color_sharp_pqmask_enable_RBUS	reg_color_sharp_pqmask_enable_reg;

	reg_color_sharp_pqmask_enable_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_enable_reg);
	reg_color_sharp_pqmask_enable_reg.labelpk_enable = (bEnable == 0)? 0 : 1;
	IoReg_Write32(COLOR_SHARP_pqmask_enable_reg, reg_color_sharp_pqmask_enable_reg.regValue);
	
}

void drvif_sharpness_pq_mask_en(short bEn)
{
	color_sharp_pqmask_enable_RBUS    reg_color_sharp_pqmask_enable_reg;
	reg_color_sharp_pqmask_enable_reg.regValue = IoReg_Read32(COLOR_SHARP_pqmask_enable_reg);

	reg_color_sharp_pqmask_enable_reg.labelpk_sobel_en = bEn;
	reg_color_sharp_pqmask_enable_reg.labelpk_zdiff_en = bEn;
	reg_color_sharp_pqmask_enable_reg.labelpk_tex_en = bEn;
	reg_color_sharp_pqmask_enable_reg.labelpk_enable = bEn;

	IoReg_Write32(COLOR_SHARP_pqmask_enable_reg, reg_color_sharp_pqmask_enable_reg.regValue);
}
