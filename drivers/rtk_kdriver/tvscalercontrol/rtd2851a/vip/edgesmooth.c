
/*============================ Module dependency  ===========================*/
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/edgesmooth.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <scaler/scalerDrvCommon.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#ifndef BUILD_QUICK_SHOW
#include <tvscalercontrol/vdc/video.h>
#endif
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"


// for register dump
#include <tvscalercontrol/io/ioregdrv.h>
#undef rtd_outl
#define rtd_outl(x, y)     								IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)     									IoReg_Read32(x)
#undef rtd_maskl
#define rtd_maskl(x, y, z)     							IoReg_Mask32(x,y,z)
#undef rtd_setbits
#define rtd_setbits(offset, Mask) rtd_outl(offset, (rtd_inl(offset) | Mask))
#undef rtd_clearbits
#define rtd_clearbits(offset, Mask) rtd_outl(offset, ((rtd_inl(offset) & ~(Mask))))
/*================================== Variables ==============================*/
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_NR_DEBUG,fmt,##args)
#define NRPprintf_Info(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_NR_info_DEBUG,fmt,##args)

/* iEdgeSmooth*/
/* From MacArthur, Elsie 20130206*/
// for settings, see vip_panel_default.c I_EDGE_START

void drvif_color_iEdgeSmooth_en(unsigned char enable)
{
	/*VIPprintf("EDGE_EN =%d\n", enable);*/
	iedge_smooth_edsm_db_ctrl_RBUS edsm_db_ctrl_reg;
	iedge_smooth_dejagging_ctrl0_RBUS edgesmooth_ex_ctrl_reg;

	edsm_db_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg);
	edgesmooth_ex_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_dejagging_ctrl0_reg);
	if((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == FALSE) && (get_scaler_connect_source(SLR_MAIN_DISPLAY) == _SRC_HDMI))	//RGB444
		edgesmooth_ex_ctrl_reg.i_dejag_en = 0;
	else
		edgesmooth_ex_ctrl_reg.i_dejag_en=enable?1:0;

	edsm_db_ctrl_reg.edsm_db_apply = 1;

	IoReg_Write32(IEDGE_SMOOTH_dejagging_ctrl0_reg, edgesmooth_ex_ctrl_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);
}

void drvif_color_iEdgeSmooth_init()
{
// Merlin3 no old edgesmooth
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	UINT8  source = 255;
	iedge_smooth_edgesmooth_ex_ctrl_RBUS edgesmooth_ex_ctrl_reg;
	iedge_smooth_mixer_ctrl_RBUS mixer_ctrl_reg;
	iedge_smooth_vfilter_0_RBUS vfilter_0_reg;

	source = fwif_vip_source_check(SLR_MAIN_DISPLAY, 1);

	edgesmooth_ex_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg);
	mixer_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MIXer_ctrl_reg);
	vfilter_0_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_0_reg);

	if ((VIP_system_info_structure_table->I_Width) > 1920) {	/* 4k2k source*/
		edgesmooth_ex_ctrl_reg.lbuf_disable = 0;
		/*FIXME: Elsie 20140220 what settings should be applied for 4k2k source?*/
		edgesmooth_ex_ctrl_reg.lbuf_linemode = 0;// 2

		mixer_ctrl_reg.iedge_mixer_weight = 8;
		vfilter_0_reg.vfir_weight = 0;
		/*FIXME end*/
	} else if (((source >= VIP_QUALITY_YPbPr_720P) && (source <= VIP_QUALITY_YPbPr_1080P))
	|| ((source >= VIP_QUALITY_HDMI_720P) && (source <= VIP_QUALITY_HDMI_1080P))
	|| ((source >= VIP_QUALITY_DTV_720P) && (source <= VIP_QUALITY_DTV_1080P))
	|| ((source >= VIP_QUALITY_PVR_720P) && (source <= VIP_QUALITY_PVR_1080P))
	) {	/*HD*/
		edgesmooth_ex_ctrl_reg.lbuf_disable = 0;
		edgesmooth_ex_ctrl_reg.lbuf_linemode = 0;// 3

		mixer_ctrl_reg.iedge_mixer_weight = 8;
		vfilter_0_reg.vfir_weight = 0;

	} else {	/*RF_CVBS SD*/
		edgesmooth_ex_ctrl_reg.lbuf_disable = 0;
		edgesmooth_ex_ctrl_reg.lbuf_linemode = 0;// 3

		mixer_ctrl_reg.iedge_mixer_weight = 8;
		vfilter_0_reg.vfir_weight = 0;
	}

	IoReg_Write32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg, edgesmooth_ex_ctrl_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_MIXer_ctrl_reg, mixer_ctrl_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_0_reg, vfilter_0_reg.regValue);
}

void drvif_color_iEdgeSmooth_chaos_set(unsigned char level)/*0~8*/
{
// removed
}

void drvif_color_iEdgeSmooth_t2df_set(unsigned char level)/*0~8*/
{
// Merlin3 no
}

void drvif_color_iEdgeSmooth_edge_set(unsigned char level)/*0~8*/
{
// Merlin3 no
}

void drvif_color_iEdgeSmooth_crct_set(unsigned char level)/*0~1*/
{
// Merlin3 no
}

void drvif_color_iEdgeSmooth_extension_set(unsigned char level)/*0~16, 0:disable*/
{
// Merlin3 no
}
/*======================= rock_rau	20130227 mark not use function =======================*/
void drvif_color_Mixer_ctrl_iedge(unsigned char channel, int mixer_weight)
{
	iedge_smooth_mixer_ctrl_RBUS mixer_ctrl_reg;
	mixer_ctrl_reg.regValue = rtd_inl(IEDGE_SMOOTH_MIXer_ctrl_reg);

	/* 0: all use snr Y, 1~7: 3bit blending, 8:all use egsm result, 9:use max(snr, iegsm)*/
	if (mixer_weight <= 8)
		mixer_ctrl_reg.iedge_mixer_weight = mixer_weight;
	else
		mixer_ctrl_reg.iedge_mixer_weight = 0x8;

	rtd_outl(IEDGE_SMOOTH_MIXer_ctrl_reg, mixer_ctrl_reg.regValue);

}
/*===========================================================================*/
/*======================= rock_rau	20130227 mark not use function =======================*/

void drvif_color_DRV_IESM_Basic(DRV_IESM_Basic *ptr)
{


	iedge_smooth_edsm_db_ctrl_RBUS edsm_db_ctrl_reg;
	edsm_db_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg);
	edsm_db_ctrl_reg.edsm_db_en = 1;
	edsm_db_ctrl_reg.edsm_db_rd_sel = 1;
	IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);
	{
	//iedge_smooth_edgesmooth_ex_ctrl_RBUS edgesmooth_ex_ctrl_reg;
	iedge_smooth_mixer_ctrl_RBUS mixer_ctrl_reg;
	//edgesmooth_ex_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg);
	mixer_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MIXer_ctrl_reg);
	drvif_color_iEdgeSmooth_en(ptr->IESM_En);

	//drvif_IESM_format_convert();//move to fw_scalerip_set_IEGSM()
	//edgesmooth_ex_ctrl_reg.lbuf_linemode = drvif_IESM_frontporch_check();

	mixer_ctrl_reg.iedge_mixer_weight = ptr->IESM_Mixer_Weight;

	//IoReg_Write32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg, edgesmooth_ex_ctrl_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_MIXer_ctrl_reg, mixer_ctrl_reg.regValue);
	}

	//edsm_db_ctrl_reg.edsm_db_apply = 1;
	//IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);
}

//extern int scalerVIP_color_noise_dejag_detect__z_dummy[9];
void drvif_color_DRV_IESM_Additional_setting(DRV_IESM_Additional_setting *ptr)
{
	iedge_smooth_dejagging_ctrl0_RBUS iedge_smooth_dejagging_ctrl0;
	iedge_smooth_dejagging_motion_support_RBUS iedge_smooth_dejagging_motion_support;

	iedge_smooth_dejagging_ctrl0.regValue = IoReg_Read32(IEDGE_SMOOTH_dejagging_ctrl0_reg);
	iedge_smooth_dejagging_motion_support.regValue = IoReg_Read32(IEDGE_SMOOTH_dejagging_motion_support_reg);
	iedge_smooth_dejagging_ctrl0.i_dejag_skip_hlh = ptr->IESM_skip_hlh;
	iedge_smooth_dejagging_ctrl0.i_dejag_level= ptr->IESM_level;
	// removed iedge_smooth_dejagging_ctrl0.i_dejag_smallword_weak= ptr->IESM_smallword_weak;
	// removed iedge_smooth_dejagging_motion_support.i_dejag_delta_weak= ptr->IESM_delta_weak;
	iedge_smooth_dejagging_motion_support.i_dejag_delta_limit= ptr->IESM_delta_limit;
	iedge_smooth_dejagging_motion_support.i_dejag_delta_motion_gain= ptr->IESM_motion_gain;
	// removed iedge_smooth_dejagging_ctrl0.i_dejag_3tap_en= ptr->IESM_3tap_en;
	iedge_smooth_dejagging_ctrl0.i_dejag_delta_gain3= ptr->IESM_3tap_gain;
	iedge_smooth_dejagging_ctrl0.i_dejag_delta_gain5= ptr->IESM_5tap_gain;
	iedge_smooth_dejagging_ctrl0.i_dejag_what_the_rock= ptr->IESM_8diff_coring;
	iedge_smooth_dejagging_ctrl0.i_dejag_nondir_weight= ptr->IESM_particle2;
	iedge_smooth_dejagging_motion_support.i_dejag_delta_tex_seg= ptr->IESM_delta_tex_seg;
	iedge_smooth_dejagging_motion_support.i_dejag_nondir_tex_seg= ptr->IESM_nondir_tex_seg;
	// iedge_smooth_dejagging_ctrl0.todo= ptr->IESM_Unstable_area_thd;
	//scalerVIP_color_noise_dejag_detect__z_dummy[2]=ptr->IESM_smallword_weak*10+2;

	IoReg_Write32(IEDGE_SMOOTH_dejagging_ctrl0_reg, iedge_smooth_dejagging_ctrl0.regValue);
	IoReg_Write32(IEDGE_SMOOTH_dejagging_motion_support_reg, iedge_smooth_dejagging_motion_support.regValue);
}
