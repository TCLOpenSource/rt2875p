/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for color related functions.
 *
 * @author 	$Author$
 * @date 	$Date$
 * @version 	$Revision$
 */

/**
 * @addtogroup color
 * @{
 */

/*============================ Module dependency  ===========================*/
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scaler/source.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/scalerdrv/mode.h>
#include <tvscalercontrol/vip/xc.h>
#ifndef BUILD_QUICK_SHOW
#include <tvscalercontrol/vdc/video.h>
#endif
#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
#include <tvscalercontrol/scaler/scalervideo.h>
#endif /*#ifdef CONFIG_ENABLE_VD_27MHZ_MODE*/
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"


/*================================  definitions ===============================*/
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_XC_DEBUG,fmt,##args)

/*===================================  Types ================================*/

/*================================== Variables ==============================*/

/*unsigned char tXC_NTSC[3][15]={{0xf, 0x0, 0x30, 0x17, 0xff, 0x17, 0xff, 0x0D, 0x14, 0x2B, 0x2B, 0x2B, 0x2B, 0x14, 0x49},
				      {0xf, 0x0, 0x30, 0x23, 0xff, 0x23, 0xff, 0x14, 0x1E, 0x41, 0x41, 0x41, 0x41, 0x1E, 0x6E},
				      {0xf, 0x0, 0x30, 0x35, 0xff, 0x35, 0xff, 0x23, 0x2D, 0x62, 0x62, 0x62, 0x62, 0x2D, 0xA5}};

unsigned char tXC_PAL[3][14]={{0xe, 0x0, 0x30, 0x17, 0xff, 0x17, 0xff, 0x0D, 0x14, 0x2B, 0x2B, 0x2B, 0x2B, 0x14},
				   {0xe, 0x0, 0x30, 0x23, 0xff, 0x23, 0xff, 0x14, 0x1E, 0x41, 0x41, 0x41, 0x41, 0x1E},
				   {0xe, 0x0, 0x30, 0x35, 0xff, 0x35, 0xff, 0x23, 0x2D, 0x62, 0x62, 0x62, 0x62, 0x2D}};
*/
unsigned char tXC_NTSC_Mid[16] = {0xf, 0x0, 0x30, 0x23, 0xff, 0x23, 0xff, 0x14, 0x1E, 0x41, 0x41, 0x41, 0x41, 0x1E, 0x6E, 0};


unsigned char tXC_Init[52] = {0x4, 0x0, 0xff, 0x2,
				0x2f, 0x0, 0x30,
				35, 255, 35, 255, 20, 30, 65, 65, 65, 65, 30, 110, 15, 15, 50, 8,
				10, 100, 100, 200, 10, 20, 5, 20, 100, 110, 10, 50, 10, 6, 62, 15,
				15, 20, 5, 3, 20, 70, 30, 5, 200, 5, 0x18, 0x6a, 0};




void drvif_color_auto_bal_init(unsigned char display, unsigned char flag_interlace)
{
	unsigned int ref_h, ref_v, ref_size, ref_th;

	di_im_di_active_window_control_RBUS im_di_active_window_control_reg;
	di_im_di_xc_auto_balance_th3_RBUS im_di_xc_auto_balance_th3_reg;
	di_color_recovery_option_RBUS Color_Recovery_Option_reg;

	if (display == SLR_SUB_DISPLAY)
		return;
	else {
		/*refer to columbus ntsc*/
		ref_h = 640;

#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
		if (VD_27M_HSD960_DI5A == fw_video_get_27mhz_mode((SCALER_DISP_CHANNEL)display)){
			ref_h = ref_h*4/3;
		}
#endif /*#ifdef CONFIG_ENABLE_VD_27MHZ_MODE*/

		ref_v = 226;
		ref_th = 0x1000;
		ref_size = ref_h * ref_v;

		im_di_active_window_control_reg.regValue = IoReg_Read32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg);/*read di HSize, VSize*/
		im_di_xc_auto_balance_th3_reg.regValue = IoReg_Read32(DI_IM_DI_XC_AUTO_BALANCE_TH3_reg);/*read di sub-th*/
		Color_Recovery_Option_reg.regValue = IoReg_Read32(DI_Color_Recovery_Option_reg);

		Color_Recovery_Option_reg.cr_autoimg_enable = 0;
		if (flag_interlace)
			Color_Recovery_Option_reg.cr_fh_switch = 0;
		else/*progressive => disable auto-balance*/
			Color_Recovery_Option_reg.cr_fh_switch = 1;
		IoReg_Write32(DI_Color_Recovery_Option_reg, Color_Recovery_Option_reg.regValue);

		im_di_xc_auto_balance_th3_reg.cr_thl_subbalance = (ref_th * im_di_active_window_control_reg.hsize * im_di_active_window_control_reg.vsize)/ref_size;
		IoReg_Write32(DI_IM_DI_XC_AUTO_BALANCE_TH3_reg, im_di_xc_auto_balance_th3_reg.regValue);
	}
}




void drvif_color_di_set_TNRXC_Ctrl(DRV_di_TNR_XC_CTRL *ptr)
{
	di_im_di_tnr_xc_control_RBUS TNR_XC_reg;
	di_im_di_tnr_th_RBUS Temporal_Thl_reg;
	di_im_di_control_RBUS di_control_reg;

	TNR_XC_reg.regValue = IoReg_Read32(DI_IM_DI_TNR_XC_CONTROL_reg);
	Temporal_Thl_reg.regValue = IoReg_Read32(DI_IM_DI_TNR_TH_reg);
	di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);

	TNR_XC_reg.cp_temporal_xc_en = ptr->cp_temporal_xc_en;
	di_control_reg.cp_temporalenable = ptr->cp_temporalenable;
	Temporal_Thl_reg.cp_temporalthly = ptr->cp_temporalthly;
	VIPprintf("\n yuan,m_cp_temporal_xc_en=%d\n",TNR_XC_reg.cp_temporal_xc_en);

	di_control_reg.write_enable_6=1;

	IoReg_Write32(DI_IM_DI_TNR_XC_CONTROL_reg, TNR_XC_reg.regValue);
	IoReg_Write32(DI_IM_DI_TNR_TH_reg, Temporal_Thl_reg.regValue);
	IoReg_Write32(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);

}
void drvif_color_di_set_TNRXC_Mk2(DRV_di_TNR_XC_MKII *ptr)
{
	di_tnrxc_mkii_ctrl_RBUS tnrxc_mkii_ctrl_reg;

	tnrxc_mkii_ctrl_reg.regValue = IoReg_Read32(DI_TNRXC_MkII_CTRL_reg);

	tnrxc_mkii_ctrl_reg.tnrxc_mkii_en = ptr->tnrxc_mkii_en;
	tnrxc_mkii_ctrl_reg.tnrxc_mkii_thc1 = ptr->tnrxc_mkii_thc1;
	tnrxc_mkii_ctrl_reg.tnrxc_mkii_thc2 = ptr->tnrxc_mkii_thc2;
	tnrxc_mkii_ctrl_reg.tnrxc_mkii_thy = ptr->tnrxc_mkii_thy;

	IoReg_Write32(DI_TNRXC_MkII_CTRL_reg, tnrxc_mkii_ctrl_reg.regValue);
}





