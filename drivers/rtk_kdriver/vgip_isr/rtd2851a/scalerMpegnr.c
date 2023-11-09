/*******************************************************************************
 * Header include
******************************************************************************/
//#include <stdio.h> /***/
//#include <stdlib.h> /***/
//#include <string.h> /***/
//#include <rtd_types.h> /***/
//#include <rtd_gpio.h> /***/
#include <asm/div64.h>
#include <rtk_kdriver/io.h>

#include "vgip_isr/scalerVideo.h"
#include "vgip_isr/scalerVIP.h"
#include "vgip_isr/scalerDI.h"
#include "vgip_isr/scalerDCC.h"
#include <scaler/scalerDrvCommon.h>
#include <scaler/vipCommon.h>
#include <scaler/vipRPCCommon.h>
#include <scaler/vdcCommon.h>

#include <rbus/vdtop_reg.h>
#include <rbus/vdpq_reg.h>
#include <rbus/color_sharp_reg.h>
#include <rbus/color_dcc_reg.h>
#include <rbus/color_icm_reg.h>
#include <rbus/color_reg.h>
#include <rbus/histogram_reg.h>
#include <rbus/rgb2yuv_reg.h>
#include <rbus/scaledown_reg.h>
#include <rbus/scaleup_reg.h>
#include <rbus/yuv2rgb_reg.h>
#include <rbus/od_reg.h>
#include <rbus/vodma_reg.h>
#include <rbus/smartfit_reg.h>
#include <rbus/di_reg.h>
#include <rbus/nr_reg.h>
#include <rbus/mpegnr_reg.h>
#include <rbus/idcti_reg.h>
#include <rbus/vgip_reg.h>
#include <rbus/profile_reg.h>
#include <rbus/color_dlcti_reg.h>
#include <rbus/smartfit_reg.h>
#include <rbus/iedge_smooth_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/gamma_reg.h>
#include <rbus/inv_gamma_reg.h>
#include <rbus/dm_reg.h>
#include <rbus/hdr_all_top_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/hsd_dither_reg.h>
#include <rbus/color_mb_peaking_reg.h>
//#include <rbus/sub_dither_reg.h>
#include <rbus/color_temp_reg.h>
#include <rbus/dither_reg.h>
#include <rbus/memc_mux_reg.h>

#include <mach/rtk_log.h>



#include <tvscalercontrol/vip/vip_reg_def.h>
#include "tvscalercontrol/scaler/scalerstruct.h"
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/color.h>


#ifdef CONFIG_HW_SUPPORT_MEMC
#include "memc_isr/scalerMEMC.h"
#include "tvscalercontrol/scaler_vpqmemcdev.h"
#endif
#include <tvscalercontrol/scaler/scalercolorlib_tv002.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
/*******************************************************************************
* Macro
******************************************************************************/

//#define false FALSE
//#define true TRUE
#undef ROSPrintf
#define ROSPrintf(fmt, args...)						VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VGIP_Video_DEBUG, MA_print_count, fmt,##args)
#define ROS_XCprintf_Info(fmt, args...)				VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VIP_XC_interface1_DEBUG, MA_print_count, fmt, ##args)

//#define VIP_DEBUG_ENABLE
#ifdef VIP_DEBUG_ENABLE
  #define VIP_DEBUG_PRINTF	printk
#else
  #define VIP_DEBUG_PRINTF(x,y...)
#endif

#define detect_pattern 0
#define motion_concentric_circles_log 0
#define black_white_pattern_log 0

#define ABS(x,y)((x > y) ? (x-y) : (y-x))
#define abs_value(x) ( (x>0) ? (x) : (-(x)) )
#define CLAMP(a,b,c) ( (a)<(b) ? (b) : ((a)>(c)?(c):(a)) )    //!< clamp a to the range of [b;c]
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef Min
#define Min(a, b)   ( ((a) < (b)) ? (a) : (b) )
#endif

extern Start_Print_info_t Start_Print;
extern _RPC_system_setting_info	*RPC_system_info_structure_table;
extern unsigned int RTNR_MAD_count_Y3_avg;
int mpeg_ratio=1;

//============================Elsie 20130425: color mpeg========================

int scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[9][8]={
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0}
};
int scalerVIP_color_noise_mpeg_h_detect__HisCntHor[8]={0,0,0,0,0,0,0,0};
int scalerVIP_color_noise_mpeg_h_detect__a_scaling[9]={0x1000,0x1000,0x1000,0,0,0,0,0,0};//{0x1000,0x1000,0x1000,0x1555,0x1555,0x1555,0x2aab,0x2aab,0x2aab};//{0x1000,0x1555,0,0x2000,0x2aab,0,0,0,0};//
int scalerVIP_color_noise_mpeg_h_detect__amax1p[3][9]={{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}};
int scalerVIP_color_noise_mpeg_h_detect__a2[3][9]={{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}};
int scalerVIP_color_noise_mpeg_h_detect__a3[3][9]={{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}};
int scalerVIP_color_noise_mpeg_h_detect__x3L=0x2000;// minimum detail
int scalerVIP_color_noise_mpeg_h_detect__x3R=0x3000;// maximum detail
int scalerVIP_color_noise_mpeg_h_detect__xwidth=1920;
int scalerVIP_color_noise_mpeg_h_detect__xheight=1080;
int scalerVIP_color_noise_mpeg_h_detect__x_requires_ver=0;
int scalerVIP_color_noise_mpeg_h_detect__x_pipeline=0;
int scalerVIP_color_noise_mpeg_h_detect__x_ub_ub=18;
int scalerVIP_color_noise_mpeg_h_detect__x_ub_ub4=18;
int scalerVIP_color_noise_mpeg_h_detect__yen=0;
int scalerVIP_color_noise_mpeg_h_detect__yub=0;// higher means more detail
int scalerVIP_color_noise_mpeg_h_detect__yub2=0;// higher means more blocking effect
int scalerVIP_color_noise_mpeg_h_detect__y_scaling=0;
int scalerVIP_color_noise_mpeg_h_detect__y_turn=8;// winning turn
int scalerVIP_color_noise_mpeg_h_detect__y_max1p=0;// the bin with the largest number
int scalerVIP_color_noise_mpeg_h_detect__y3_for_other_ip=0;
int scalerVIP_color_noise_mpeg_h_detect__z_mad=0;
int scalerVIP_color_noise_mpeg_h_detect__z_mad_pre=-88;
int scalerVIP_color_noise_mpeg_h_detect__z_mad_wait=0;
int scalerVIP_color_noise_mpeg_h_detect__z_scaling=0;
int scalerVIP_color_noise_mpeg_h_detect__z_turn=8;
int scalerVIP_color_noise_mpeg_detect_y=0;
int scalerVIP_color_noise_mpeg_detect_y4=0;
int scalerVIP_color_noise_mpeg_detect_ya4[9]={0,0,0,0,0,0,0,0,0};
int scalerVIP_color_noise_mpeg_detect_y_delta3=0;
int scalerVIP_color_noise_mpeg_detect_x_by_motion_en=1;
int scalerVIP_color_noise_mpeg_detect_y_by_motion=0;
//#define mpegnr_h_170705h 1
#if 1//mpegnr_h_170705h
unsigned int RTNR_MAD_count_Y3_count=0;
unsigned int MCNR_GMV_Ratio = 0;
int scalerVIP_color_Get_noise_mpeg_detect_y(void)
{
	return scalerVIP_color_noise_mpeg_detect_y;
}

void scalerVIP_color_noise_mpeg_detect(void)
{
	int i=0;
	int z=0;
	int z_max1=0;
	int z_max2=0;
	int z_max2_4=0;
	//int z_max2p=0;
	int x2=0;// block effect level detected
	int x2_4=0;
	int x20=0;// block effect ratio * 256
	int x20_4=0;
	int x2L=0x1000;
	int y2L=0;
	int x2R=0x3000;
	int y2R=256;
	int x2L4=0x1000;
	int y2L4=0;
	int x2R4=0x3000;
	int y2R4=36;
	int x2div=0x80;
	int x3=0;
	int y3L=48;
	int y3R=12;
	int x3div=0x80;
	int ratio_W1=0x500;
	int ratio_W2=0x800;
	int ratio_W1_=0;
	int ratio_W2_=0;
	int x2_=0;
	int x3L_=0;
	int x3R_=0;
	int x3_=0;
	int x2L_= 0;
	int x2R_= 0;
	int z_mad_d=0;
	static int static_cnt = 180;
	static int is_static_cnt_tsin_ku = 0;

	mpegnr_ich1_cnthor0_RBUS ich1_CntHor0_reg;
	mpegnr_ich1_cnthor1_RBUS ich1_CntHor1_reg;
	mpegnr_ich1_cnthor2_RBUS ich1_CntHor2_reg;
	mpegnr_ich1_cnthor3_RBUS ich1_CntHor3_reg;
	mpegnr_ich1_cnthor4_RBUS ich1_CntHor4_reg;
	mpegnr_ich1_cnthor5_RBUS ich1_CntHor5_reg;
	mpegnr_ich1_cnthor6_RBUS ich1_CntHor6_reg;
	mpegnr_ich1_cnthor7_RBUS ich1_CntHor7_reg;
	di_im_di_rtnr_control_RBUS di_im_di_rtnr_control_r;
	nr_dch1_mpegnr1_RBUS	nr_dch1_mpegnr1_r;
	int scalerVIP_color_noise_mpeg_detect_y_ =0;
	DRV_MPEGNR_16_SW_CTRL Mpeg_16_sw_ctrl_h;


	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	unsigned char PQA_table_select = scaler_GetShare_Memory_system_setting_info_Struct()->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_table_select;
	unsigned int *pPQA_InputMAD_MPEG = &(scaler_GetShare_Memory_VIP_TABLE_Struct()->PQA_Input_Table[PQA_table_select][I_MPEG_MAD][0]);
	if(VIP_system_info_structure_table ==NULL || pPQA_InputMAD_MPEG ==NULL ){
		return;
	}

	drvif_color_get_DRV_MPG_H_SW_Ctrl((DRV_MPEGNR_16_SW_CTRL*)&Mpeg_16_sw_ctrl_h);
	x2L=Mpeg_16_sw_ctrl_h.x2l;
	x2R=Mpeg_16_sw_ctrl_h.x2r;
	scalerVIP_color_noise_mpeg_h_detect__x3L=Mpeg_16_sw_ctrl_h.x3l;
	scalerVIP_color_noise_mpeg_h_detect__x3R=Mpeg_16_sw_ctrl_h.x3r;
	y3L=Mpeg_16_sw_ctrl_h.y3l;
	y3R=Mpeg_16_sw_ctrl_h.y3r;
	scalerVIP_color_noise_mpeg_h_detect__x_ub_ub =Mpeg_16_sw_ctrl_h.x_ub_ub;

	//printk("x2L=%d,x2R=%d,x3l=%d,x3r=%d,y3l=%d,y3r=%d,ub=%d \n ",x2L,x2R,
	//scalerVIP_color_noise_mpeg_h_detect__x3L,scalerVIP_color_noise_mpeg_h_detect__x3R,
	//y3L,y3R,scalerVIP_color_noise_mpeg_h_detect__x_ub_ub);

	scalerVIP_color_noise_mpeg_h_detect__z_turn=scalerVIP_color_noise_mpeg_h_detect__z_turn<8?scalerVIP_color_noise_mpeg_h_detect__z_turn+1:0;
	scalerVIP_color_noise_mpeg_h_detect__z_scaling=scalerVIP_color_noise_mpeg_h_detect__a_scaling[scalerVIP_color_noise_mpeg_h_detect__z_turn];
	// different resolution (default 1920*1080)
	scalerVIP_color_noise_mpeg_h_detect__xwidth=VIP_system_info_structure_table->I_Width;
	scalerVIP_color_noise_mpeg_h_detect__xheight=VIP_system_info_structure_table->I_Height;
	di_im_di_rtnr_control_r.regValue=IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);
	ich1_CntHor0_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor0_reg);
	ich1_CntHor1_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor1_reg);
	ich1_CntHor2_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor2_reg);
	ich1_CntHor3_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor3_reg);
	ich1_CntHor4_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor4_reg);
	ich1_CntHor5_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor5_reg);
	ich1_CntHor6_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor6_reg);
	ich1_CntHor7_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor7_reg);
	if(di_im_di_rtnr_control_r.cp_rtnr_progressive==1){
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[0]=ich1_CntHor0_reg.cp_cnthor0;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[1]=ich1_CntHor1_reg.cp_cnthor1;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[2]=ich1_CntHor2_reg.cp_cnthor2;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[3]=ich1_CntHor3_reg.cp_cnthor3;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[4]=ich1_CntHor4_reg.cp_cnthor4;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[5]=ich1_CntHor5_reg.cp_cnthor5;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[6]=ich1_CntHor6_reg.cp_cnthor6;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[7]=ich1_CntHor7_reg.cp_cnthor7;
	}else{
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[0]=scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[scalerVIP_color_noise_mpeg_h_detect__z_turn][0]+ich1_CntHor0_reg.cp_cnthor0;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[1]=scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[scalerVIP_color_noise_mpeg_h_detect__z_turn][1]+ich1_CntHor1_reg.cp_cnthor1;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[2]=scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[scalerVIP_color_noise_mpeg_h_detect__z_turn][2]+ich1_CntHor2_reg.cp_cnthor2;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[3]=scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[scalerVIP_color_noise_mpeg_h_detect__z_turn][3]+ich1_CntHor3_reg.cp_cnthor3;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[4]=scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[scalerVIP_color_noise_mpeg_h_detect__z_turn][4]+ich1_CntHor4_reg.cp_cnthor4;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[5]=scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[scalerVIP_color_noise_mpeg_h_detect__z_turn][5]+ich1_CntHor5_reg.cp_cnthor5;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[6]=scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[scalerVIP_color_noise_mpeg_h_detect__z_turn][6]+ich1_CntHor6_reg.cp_cnthor6;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[7]=scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[scalerVIP_color_noise_mpeg_h_detect__z_turn][7]+ich1_CntHor7_reg.cp_cnthor7;
		for(i=0;i<8;i++){
			if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30
				printk(
					KERN_INFO "\n[histhor_subtract] %d %06x - %06x = %06x[/histhor_subtract]"
					,i
					,scalerVIP_color_noise_mpeg_h_detect__HisCntHor[i]
					,scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[scalerVIP_color_noise_mpeg_h_detect__z_turn][i]
					,scalerVIP_color_noise_mpeg_h_detect__HisCntHor[i]-scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[scalerVIP_color_noise_mpeg_h_detect__z_turn][i]
				);
			}
			scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[scalerVIP_color_noise_mpeg_h_detect__z_turn][i]=scalerVIP_color_noise_mpeg_h_detect__HisCntHor[i]-scalerVIP_color_noise_mpeg_h_detect__HisCntHor_pre[scalerVIP_color_noise_mpeg_h_detect__z_turn][i];
		}
	}
	/* specila pattern for tv002
	if ((ID_TV002_Flag[ID_OsceanWave_TV002] == 1) || (ID_TV002_Flag[ID_Grilled_Meat_TV002] == 1)) {
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[0]=0;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[1]=0;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[2]=0;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[3]=0;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[4]=0;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[5]=0;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[6]=0;
		scalerVIP_color_noise_mpeg_h_detect__HisCntHor[7]=0;
	}
	//*/

	scalerVIP_color_noise_mpeg_h_detect__a_scaling[3]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[4]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[5]=0;
	scalerVIP_color_noise_mpeg_h_detect__a_scaling[6]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[7]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[8]=0;
	switch((RPC_system_info_structure_table->VIP_source)){
		case VIP_QUALITY_HDMI_4k2kI_30:
		case VIP_QUALITY_HDMI_4k2kP_30:
		case VIP_QUALITY_HDMI_4k2kP_60:
		case VIP_QUALITY_DTV_4k2kI_30:
		case VIP_QUALITY_DTV_4k2kP_30:
		case VIP_QUALITY_DTV_4k2kP_60:
		case VIP_QUALITY_PVR_4k2kI_30:
		case VIP_QUALITY_PVR_4k2kP_30:
		case VIP_QUALITY_PVR_4k2kP_60:
			scalerVIP_color_noise_mpeg_h_detect__a_scaling[3]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[4]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[5]=0x1555;
			scalerVIP_color_noise_mpeg_h_detect__a_scaling[6]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[7]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[8]=0x2aab;
		break;
		case VIP_QUALITY_HDMI_720P:
		case VIP_QUALITY_YPbPr_720P:
		case VIP_QUALITY_DTV_720P:
		case VIP_QUALITY_PVR_720P:
			scalerVIP_color_noise_mpeg_h_detect__a_scaling[3]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[4]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[5]=0x1c71;
		break;
		case VIP_QUALITY_HDMI_1080I:
		case VIP_QUALITY_HDMI_1080P:
		case VIP_QUALITY_YPbPr_1080I:
		case VIP_QUALITY_YPbPr_1080P:
		case VIP_QUALITY_PVR_1080I:
		case VIP_QUALITY_PVR_1080P:
			scalerVIP_color_noise_mpeg_h_detect__a_scaling[3]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[4]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[5]=0x1555;
			scalerVIP_color_noise_mpeg_h_detect__a_scaling[6]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[7]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[8]=0x2aab;
		break;
		case VIP_QUALITY_DTV_1080I:
		case VIP_QUALITY_DTV_1080P:
			scalerVIP_color_noise_mpeg_h_detect__a_scaling[3]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[4]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[5]=0x1555;
			scalerVIP_color_noise_mpeg_h_detect__a_scaling[6]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[7]=scalerVIP_color_noise_mpeg_h_detect__a_scaling[8]=0x2aab;
		break;
		case VIP_QUALITY_YPbPr_480I:
		case VIP_QUALITY_YPbPr_576I:
		case VIP_QUALITY_HDMI_480I:
		case VIP_QUALITY_HDMI_576I:
			//
		break;
		case VIP_QUALITY_YPbPr_480P:
		case VIP_QUALITY_YPbPr_576P:
		case VIP_QUALITY_HDMI_480P:
		case VIP_QUALITY_HDMI_576P:
			//
		break;
		case VIP_QUALITY_DTV_480P:
		case VIP_QUALITY_DTV_576P:
		case VIP_QUALITY_PVR_480P:
		case VIP_QUALITY_PVR_576P:
			//
		break;
		case VIP_QUALITY_DTV_480I:
		case VIP_QUALITY_DTV_576I:
		case VIP_QUALITY_PVR_480I:
		case VIP_QUALITY_PVR_576I:
			//
		break;
		case VIP_QUALITY_CVBS_NTSC:
		case VIP_QUALITY_CVBS_PAL:
			//
		break;
		default:
			//
		break;
	}

	nr_dch1_mpegnr1_r.regValue=IoReg_Read32(NR_DCH1_MPEGNR1_reg);

	do{
		if(
			(scalerVIP_color_noise_mpeg_h_detect__z_scaling<0x800)
			||(!scalerVIP_color_noise_mpeg_h_detect__x_pipeline&&scalerVIP_color_noise_mpeg_h_detect__z_turn%3!=2)
		){
			for(i=2;i>=0;i--){
				scalerVIP_color_noise_mpeg_h_detect__amax1p[i][scalerVIP_color_noise_mpeg_h_detect__z_turn]=0;
				scalerVIP_color_noise_mpeg_h_detect__a2[i][scalerVIP_color_noise_mpeg_h_detect__z_turn]=0;
				scalerVIP_color_noise_mpeg_h_detect__a3[i][scalerVIP_color_noise_mpeg_h_detect__z_turn]=0;
			}
			break;
		}
			//*
		if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30
			printk(
				KERN_INFO "\n[histhor_ori] %d %06x %06x %06x %06x %06x %06x %06x %06x[/histhor_ori]"
				,scalerVIP_color_noise_mpeg_h_detect__z_scaling
				,ich1_CntHor0_reg.cp_cnthor0
				,ich1_CntHor1_reg.cp_cnthor1
				,ich1_CntHor2_reg.cp_cnthor2
				,ich1_CntHor3_reg.cp_cnthor3
				,ich1_CntHor4_reg.cp_cnthor4
				,ich1_CntHor5_reg.cp_cnthor5
				,ich1_CntHor6_reg.cp_cnthor6
				,ich1_CntHor7_reg.cp_cnthor7
			);
		}
		if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30
			printk(
				KERN_INFO "\n[histhor] %d %06x %06x %06x %06x %06x %06x %06x %06x[/histhor]"
				,scalerVIP_color_noise_mpeg_h_detect__z_scaling
				,scalerVIP_color_noise_mpeg_h_detect__HisCntHor[0]
				,scalerVIP_color_noise_mpeg_h_detect__HisCntHor[1]
				,scalerVIP_color_noise_mpeg_h_detect__HisCntHor[2]
				,scalerVIP_color_noise_mpeg_h_detect__HisCntHor[3]
				,scalerVIP_color_noise_mpeg_h_detect__HisCntHor[4]
				,scalerVIP_color_noise_mpeg_h_detect__HisCntHor[5]
				,scalerVIP_color_noise_mpeg_h_detect__HisCntHor[6]
				,scalerVIP_color_noise_mpeg_h_detect__HisCntHor[7]
			);
		}
			//*/
		scalerVIP_color_noise_mpeg_h_detect__y_max1p=0;
		z_max1=scalerVIP_color_noise_mpeg_h_detect__HisCntHor[0];
		z_max2=0;
		for(i=1;i<8;i++){
			z=scalerVIP_color_noise_mpeg_h_detect__HisCntHor[i];
			if(z>z_max1){
				z_max2=z_max1;
				scalerVIP_color_noise_mpeg_h_detect__y_max1p=i;
				z_max1=z;
			}
			else if(z>z_max2){
				z_max2=z;
			}
		}
#if 1
		if((scalerVIP_color_noise_mpeg_detect_x_by_motion_en == 1) && !(nr_dch1_mpegnr1_r.cp_mpglowerbound&0x08) ){

			MCNR_GMV_Ratio = 100 - scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio();//MCNR_GMV_Ratio : 100 -> 0 ( still -> motion )
			if(RTNR_MAD_count_Y3_count > (RTNR_MAD_count_Y3_avg+MCNR_GMV_Ratio)){
				RTNR_MAD_count_Y3_count = RTNR_MAD_count_Y3_count - (((RTNR_MAD_count_Y3_count - (RTNR_MAD_count_Y3_avg+MCNR_GMV_Ratio))>>3) + 1);
			}
			else if(((RTNR_MAD_count_Y3_avg+MCNR_GMV_Ratio) > (RTNR_MAD_count_Y3_count+10))) {
				RTNR_MAD_count_Y3_count = RTNR_MAD_count_Y3_count + ((((RTNR_MAD_count_Y3_avg+MCNR_GMV_Ratio) - RTNR_MAD_count_Y3_count)>>3) + 1) ;
			}

			scalerVIP_color_noise_mpeg_h_detect__z_mad=RTNR_MAD_count_Y3_count;
			if(scalerVIP_color_noise_mpeg_h_detect__z_mad_pre<=-88){
				scalerVIP_color_noise_mpeg_h_detect__z_mad_wait=0;
			}else{
				z_mad_d=scalerVIP_color_noise_mpeg_h_detect__z_mad-scalerVIP_color_noise_mpeg_h_detect__z_mad_pre;
				scalerVIP_color_noise_mpeg_h_detect__z_mad_wait=scalerVIP_color_noise_mpeg_h_detect__z_mad_wait+5-z_mad_d;
			}
			scalerVIP_color_noise_mpeg_h_detect__z_mad_pre=scalerVIP_color_noise_mpeg_h_detect__z_mad;
			if(scalerVIP_color_noise_mpeg_h_detect__z_mad_wait<0){
				scalerVIP_color_noise_mpeg_h_detect__z_mad_wait=0;
			}
			RTNR_MAD_count_Y3_count=RTNR_MAD_count_Y3_count+scalerVIP_color_noise_mpeg_h_detect__z_mad_wait;

			if(Mpeg_16_sw_ctrl_h.fw_ctrl1<Mpeg_16_sw_ctrl_h.fw_ctrl2){
				ratio_W1 = Mpeg_16_sw_ctrl_h.fw_ctrl1;
				ratio_W2 = Mpeg_16_sw_ctrl_h.fw_ctrl2;
			}else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L00]){
				ratio_W1 = 0x500;
				ratio_W2 = 0x800;
			}else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L01]){
				ratio_W1 = 0x480;
				ratio_W2 = 0x780;
			}else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L02]){
				ratio_W1 = 0x400;
				ratio_W2 = 0x700;
			}else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L03]){
				ratio_W1 = 0x380;
				ratio_W2 = 0x680;
			}else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L04]){
				ratio_W1 = 0x300;
				ratio_W2 = 0x600;
			}else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L05]){
				ratio_W1 = 0x300;
				ratio_W2 = 0x580;
			}else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L06]){
				ratio_W1 = 0x300;
				ratio_W2 = 0x500;
			}else{
				ratio_W1 = 0x300;
				ratio_W2 = 0x500;
			}
			if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30
				printk(
					KERN_INFO "\n[motion]%d %d %d %08x, [/motion]"
					,RTNR_MAD_count_Y3_avg
					,MCNR_GMV_Ratio
					,RTNR_MAD_count_Y3_count
					,z_max2>0?(z_max1<<8)/z_max2:0
					);
			}

			//scalerVIP_color_noise_mpeg_detect_y = (scalerVIP_color_noise_mpeg_detect_y*scalerVIP_color_noise_mpeg_detect_y_by_motion)>>5;

		}
#endif
		z_max2_4=-z_max1-scalerVIP_color_noise_mpeg_h_detect__HisCntHor[scalerVIP_color_noise_mpeg_h_detect__y_max1p^4];
		for(i=0;i<8;i++){
			z_max2_4=z_max2_4+scalerVIP_color_noise_mpeg_h_detect__HisCntHor[i];
		}
		z_max2_4=z_max2_4/6;
		if(Mpeg_16_sw_ctrl_h.fw_ctrl1<Mpeg_16_sw_ctrl_h.fw_ctrl2){
			x20=z_max2>0?(z_max1<<8)/z_max2:0;
			x20=(x20*scalerVIP_color_noise_mpeg_h_detect__z_scaling/4096)|0;
		} else
		if((nr_dch1_mpegnr1_r.cp_mpglowerbound&0x08) ){
			x20=z_max2>0?(z_max1<<8)/z_max2:0;
			x20=(x20*scalerVIP_color_noise_mpeg_h_detect__z_scaling/4096)|0;
		} else {
			x20 = ratio_W2;
		}

		if(x2L==x2R){
			scalerVIP_color_noise_mpeg_detect_y_by_motion=x2L;
		}else if(ratio_W1==ratio_W2){
			scalerVIP_color_noise_mpeg_detect_y_by_motion=(x2L+x2R)>>1;
		}else if(x20<=ratio_W1){
			scalerVIP_color_noise_mpeg_detect_y_by_motion=x2L;
		}else if(x20>=ratio_W2){
			scalerVIP_color_noise_mpeg_detect_y_by_motion=x2R;
		}else{
			x2div=1;//ratio_W2-ratio_W1>x2L-x2R?(abs_value((ratio_W2-ratio_W1)/(x2L-x2R))+1)>>1:1;
			ratio_W1_=((ratio_W1+(x2div>>1))/x2div)|0;
			ratio_W2_=((ratio_W2+(x2div>>1))/x2div)|0;
			x2_=((x20+(x2div>>1))/x2div)|0;
			scalerVIP_color_noise_mpeg_detect_y_by_motion=(x2L*(ratio_W2_-x2_)+x2R*(x2_-ratio_W1_)+((ratio_W2_-ratio_W1_)>>1))/(ratio_W2_-ratio_W1_);
		}

		if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30
			printk(
				KERN_INFO "\n[x2,x3] %d ,%d, %d [/x2,x3]"
				,(z_max1-z_max2)
				,scalerVIP_color_noise_mpeg_detect_y_by_motion
				,-z_max1-z_max2
				);
		}

		x2=z_max1-z_max2;
		if(scalerVIP_color_noise_mpeg_detect_y_by_motion<x2R&&scalerVIP_color_noise_mpeg_detect_y_by_motion<x2){
			x2=scalerVIP_color_noise_mpeg_detect_y_by_motion;
		}
		x3=-z_max1-z_max2;
		for(i=0;i<8;i++){
			x3=x3+scalerVIP_color_noise_mpeg_h_detect__HisCntHor[i];
		}

		x2_4=z_max1-z_max2_4;
		x20_4=z_max2_4>0?(z_max1<<8)/z_max2_4:0;
		if(x2L4==x2R4){
			scalerVIP_color_noise_mpeg_detect_y_by_motion=x2L4;
		}else if(ratio_W1==ratio_W2){
			scalerVIP_color_noise_mpeg_detect_y_by_motion=(x2L4+x2R4)>>1;
		}else if(x20_4<=ratio_W1){
			scalerVIP_color_noise_mpeg_detect_y_by_motion=x2L4;
		}else if(x20_4>=ratio_W2){
			scalerVIP_color_noise_mpeg_detect_y_by_motion=x2R4;
		}else{
			x2div=1;//ratio_W2-ratio_W1>x2L-x2R?(abs_value((ratio_W2-ratio_W1)/(x2L-x2R))+1)>>1:1;
			ratio_W1_=((ratio_W1+(x2div>>1))/x2div)|0;
			ratio_W2_=((ratio_W2+(x2div>>1))/x2div)|0;
			x2_=((x20_4+(x2div>>1))/x2div)|0;
			scalerVIP_color_noise_mpeg_detect_y_by_motion=(x2L4*(ratio_W2_-x2_)+x2R4*(x2_-ratio_W1_)+((ratio_W2_-ratio_W1_)>>1))/(ratio_W2_-ratio_W1_);
		}
		if(scalerVIP_color_noise_mpeg_detect_y_by_motion<x2R4&&scalerVIP_color_noise_mpeg_detect_y_by_motion<x2_4){
			x2_4=scalerVIP_color_noise_mpeg_detect_y_by_motion;
		}
		// scalerVIP_color_noise_mpeg_detect_y4=0;

		for(i=2;i>0;i--){
			scalerVIP_color_noise_mpeg_h_detect__amax1p[i][scalerVIP_color_noise_mpeg_h_detect__z_turn]=scalerVIP_color_noise_mpeg_h_detect__amax1p[i-1][scalerVIP_color_noise_mpeg_h_detect__z_turn];
			scalerVIP_color_noise_mpeg_h_detect__a2[i][scalerVIP_color_noise_mpeg_h_detect__z_turn]=scalerVIP_color_noise_mpeg_h_detect__a2[i-1][scalerVIP_color_noise_mpeg_h_detect__z_turn];
			scalerVIP_color_noise_mpeg_h_detect__a3[i][scalerVIP_color_noise_mpeg_h_detect__z_turn]=scalerVIP_color_noise_mpeg_h_detect__a3[i-1][scalerVIP_color_noise_mpeg_h_detect__z_turn];
		}
		x2=(x2*1920/scalerVIP_color_noise_mpeg_h_detect__xwidth*1080/scalerVIP_color_noise_mpeg_h_detect__xheight)|0;
		x3=(x3*320/scalerVIP_color_noise_mpeg_h_detect__xwidth*1080/scalerVIP_color_noise_mpeg_h_detect__xheight)|0;// 320 = 1920 / 6
		x2_4=(x2_4*1920/scalerVIP_color_noise_mpeg_h_detect__xwidth*1080/scalerVIP_color_noise_mpeg_h_detect__xheight)|0;
		if(di_im_di_rtnr_control_r.cp_rtnr_progressive==0){
			x2=x2>>1;
			x3=x3>>1;
			x2_4=x2_4>>1;
		}
		scalerVIP_color_noise_mpeg_h_detect__amax1p[0][scalerVIP_color_noise_mpeg_h_detect__z_turn]=scalerVIP_color_noise_mpeg_h_detect__y_max1p;
		if(x2>scalerVIP_color_noise_mpeg_h_detect__a2[0][scalerVIP_color_noise_mpeg_h_detect__z_turn]+0x80)
		{
			x2=scalerVIP_color_noise_mpeg_h_detect__a2[0][scalerVIP_color_noise_mpeg_h_detect__z_turn]+0x80;
		}
		else
		if(x2<scalerVIP_color_noise_mpeg_h_detect__a2[0][scalerVIP_color_noise_mpeg_h_detect__z_turn]-0x80)
		{
			x2=scalerVIP_color_noise_mpeg_h_detect__a2[0][scalerVIP_color_noise_mpeg_h_detect__z_turn]-0x80;
		}
		scalerVIP_color_noise_mpeg_h_detect__a2[0][scalerVIP_color_noise_mpeg_h_detect__z_turn]=x2;
		scalerVIP_color_noise_mpeg_h_detect__a3[0][scalerVIP_color_noise_mpeg_h_detect__z_turn]=x3;
			/*
			printk(
				KERN_EMERG "\n[mpegnr_h] %d %d %d %d %d %d %d[/mpegnr_h]",
				scalerVIP_color_noise_mpeg_h_detect__z_turn,
				scalerVIP_color_noise_mpeg_h_detect__y_turn,
				(scalerVIP_color_noise_mpeg_h_detect__z_turn!=scalerVIP_color_noise_mpeg_h_detect__y_turn?1:0)|(scalerVIP_color_noise_mpeg_h_detect__y_turn<9?2:0),
				scalerVIP_color_noise_mpeg_h_detect__z_scaling,
				x2,
				scalerVIP_color_noise_mpeg_h_detect__a2[1][scalerVIP_color_noise_mpeg_h_detect__y_turn],
				scalerVIP_color_noise_mpeg_h_detect__a2[scalerVIP_color_noise_mpeg_h_detect__y_turn>scalerVIP_color_noise_mpeg_h_detect__z_turn?2:0][scalerVIP_color_noise_mpeg_h_detect__y_turn]
			);
			//*/
		if(
			scalerVIP_color_noise_mpeg_h_detect__y_max1p!=scalerVIP_color_noise_mpeg_h_detect__amax1p[1][scalerVIP_color_noise_mpeg_h_detect__z_turn] ||
			scalerVIP_color_noise_mpeg_h_detect__y_max1p!=scalerVIP_color_noise_mpeg_h_detect__amax1p[2][scalerVIP_color_noise_mpeg_h_detect__z_turn]
		){
			if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30
				printk(
					KERN_INFO "\n[histhor.break]max1p_changed %d %d %d[/histhor.break]"
					,scalerVIP_color_noise_mpeg_h_detect__y_max1p
					,scalerVIP_color_noise_mpeg_h_detect__amax1p[1][scalerVIP_color_noise_mpeg_h_detect__z_turn]
					,scalerVIP_color_noise_mpeg_h_detect__amax1p[2][scalerVIP_color_noise_mpeg_h_detect__z_turn]
				);
			}
			if(scalerVIP_color_noise_mpeg_detect_ya4[scalerVIP_color_noise_mpeg_h_detect__z_turn]>0)
			{
				scalerVIP_color_noise_mpeg_detect_ya4[scalerVIP_color_noise_mpeg_h_detect__z_turn]--;
			}
			if(scalerVIP_color_noise_mpeg_h_detect__a2[0][scalerVIP_color_noise_mpeg_h_detect__z_turn]>x2L)
			{
				// cannot change max1p yet
				x2=scalerVIP_color_noise_mpeg_h_detect__a2[1][scalerVIP_color_noise_mpeg_h_detect__z_turn]-0x80;
				scalerVIP_color_noise_mpeg_h_detect__a2[0][scalerVIP_color_noise_mpeg_h_detect__z_turn]=x2;
				scalerVIP_color_noise_mpeg_h_detect__y_max1p=scalerVIP_color_noise_mpeg_h_detect__amax1p[1][scalerVIP_color_noise_mpeg_h_detect__z_turn];
				scalerVIP_color_noise_mpeg_h_detect__amax1p[0][scalerVIP_color_noise_mpeg_h_detect__z_turn]=scalerVIP_color_noise_mpeg_h_detect__amax1p[1][scalerVIP_color_noise_mpeg_h_detect__z_turn];
				break;
			}
			if(scalerVIP_color_noise_mpeg_h_detect__y_turn==scalerVIP_color_noise_mpeg_h_detect__z_turn){
				scalerVIP_color_noise_mpeg_h_detect__yen=0;
				for(i=0;i<3;i++){
					scalerVIP_color_noise_mpeg_h_detect__a2[i][scalerVIP_color_noise_mpeg_h_detect__z_turn]=0;
				}
			}
			break;
		}
		if(scalerVIP_color_noise_mpeg_h_detect__z_turn!=scalerVIP_color_noise_mpeg_h_detect__y_turn&&scalerVIP_color_noise_mpeg_h_detect__y_turn<9){
			if(
				x2<x3div+scalerVIP_color_noise_mpeg_h_detect__a2[1][scalerVIP_color_noise_mpeg_h_detect__y_turn]
				&&x2<x3div+scalerVIP_color_noise_mpeg_h_detect__a2[scalerVIP_color_noise_mpeg_h_detect__y_turn>scalerVIP_color_noise_mpeg_h_detect__z_turn?2:0][scalerVIP_color_noise_mpeg_h_detect__y_turn]
			){
				if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30
					printk(
						KERN_INFO "\n[histhor.break]factor_keep %d %d %d %d[/histhor.break]"
						,x2
						,x3div+scalerVIP_color_noise_mpeg_h_detect__a2[1][scalerVIP_color_noise_mpeg_h_detect__y_turn]
						,x3div+scalerVIP_color_noise_mpeg_h_detect__a2[scalerVIP_color_noise_mpeg_h_detect__y_turn>scalerVIP_color_noise_mpeg_h_detect__z_turn?2:0][scalerVIP_color_noise_mpeg_h_detect__y_turn]
						,scalerVIP_color_noise_mpeg_detect_y4
					);
				}
				break;
			}
		}
		scalerVIP_color_noise_mpeg_h_detect__y_turn=scalerVIP_color_noise_mpeg_h_detect__z_turn;
		for(i=2;i>0;i--){
			if(x2>scalerVIP_color_noise_mpeg_h_detect__a2[i][scalerVIP_color_noise_mpeg_h_detect__z_turn]){x2=scalerVIP_color_noise_mpeg_h_detect__a2[i][scalerVIP_color_noise_mpeg_h_detect__z_turn];}
			if(x3<scalerVIP_color_noise_mpeg_h_detect__a3[i][scalerVIP_color_noise_mpeg_h_detect__z_turn]){x3=scalerVIP_color_noise_mpeg_h_detect__a3[i][scalerVIP_color_noise_mpeg_h_detect__z_turn];}
		}
		scalerVIP_color_noise_mpeg_h_detect__y3_for_other_ip=x3;
		if(scalerVIP_color_noise_mpeg_h_detect__a_scaling[scalerVIP_color_noise_mpeg_h_detect__y_turn]>0x1000){
			x3=(x3*scalerVIP_color_noise_mpeg_h_detect__a_scaling[scalerVIP_color_noise_mpeg_h_detect__y_turn])>>12;
		}
		if(y3L==y3R){
			scalerVIP_color_noise_mpeg_h_detect__yub=y3L;
		}else if(scalerVIP_color_noise_mpeg_h_detect__x3L==scalerVIP_color_noise_mpeg_h_detect__x3R){
			scalerVIP_color_noise_mpeg_h_detect__yub=(y3L+y3R)>>1;
		}else if(x3<=scalerVIP_color_noise_mpeg_h_detect__x3L){
			scalerVIP_color_noise_mpeg_h_detect__yub=y3L;
		}else if(x3>=scalerVIP_color_noise_mpeg_h_detect__x3R){
			scalerVIP_color_noise_mpeg_h_detect__yub=y3R;
		}else{
			// x3div=(abs_value(y3L-y3R)+1)>>1;
			x3L_=((scalerVIP_color_noise_mpeg_h_detect__x3L+(x3div>>1))/x3div)|0;
			x3R_=((scalerVIP_color_noise_mpeg_h_detect__x3R+(x3div>>1))/x3div)|0;
			x3_=((x3+(x3div>>1))/x3div)|0;
			scalerVIP_color_noise_mpeg_h_detect__yub=(y3L*(x3R_-x3_)+y3R*(x3_-x3L_)+((x3R_-x3L_)>>1))/(x3R_-x3L_);
		}
		if(y2L==y2R){
			scalerVIP_color_noise_mpeg_h_detect__yub2=y2L;
		}else if(x2L==x2R){
			scalerVIP_color_noise_mpeg_h_detect__yub2=(y2L+y2R)>>1;
		}else if(x2<=x2L){
			scalerVIP_color_noise_mpeg_h_detect__yub2=y2L;
		}else if(x2>=x2R){
			scalerVIP_color_noise_mpeg_h_detect__yub2=y2R;
		}else{
			x2div=(abs_value(y2L-y2R)+1)>>1;
			x2L_=((x2L+(x2div>>1))/x2div)|0;
			x2R_=((x2R+(x2div>>1))/x2div)|0;
			x2_=((x2+(x2div>>1))/x2div)|0;
			scalerVIP_color_noise_mpeg_h_detect__yub2=(y2L*(x2R_-x2_)+y2R*(x2_-x2L_)+((x2R_-x2L_)>>1))/(x2R_-x2L_);
		}

		/*if(y2L4==y2R4){
			scalerVIP_color_noise_mpeg_detect_y4=y2L4;
		}else
		if(x2L4==x2R4){
			scalerVIP_color_noise_mpeg_detect_y4=(y2L4+y2R4)>>1;
		}else */ //for Coverity issue mark first
		if(x2_4<=x2L4){
			scalerVIP_color_noise_mpeg_detect_y4=y2L4;
		}else if(x2_4>=x2R4){
			scalerVIP_color_noise_mpeg_detect_y4=y2R4;
		}else{
			x2div=(abs_value(y2L4-y2R4)+1)>>1;
			x2L_=((x2L4+(x2div>>1))/x2div)|0;
			x2R_=((x2R4+(x2div>>1))/x2div)|0;
			x2_=((x2_4+(x2div>>1))/x2div)|0;
			scalerVIP_color_noise_mpeg_detect_y4=(y2L4*(x2R_-x2_)+y2R4*(x2_-x2L_)+((x2R_-x2L_)>>1))/(x2R_-x2L_);
		}

		scalerVIP_color_noise_mpeg_h_detect__yen=1;
		if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30
			printk(
				KERN_INFO "\n[yub]%d ,%d, %d, [/yub]"
				,scalerVIP_color_noise_mpeg_h_detect__yub
				,scalerVIP_color_noise_mpeg_h_detect__yub2
				,scalerVIP_color_noise_mpeg_detect_y4
				);
		}

	scalerVIP_color_noise_mpeg_detect_y_=(scalerVIP_color_noise_mpeg_h_detect__yub*scalerVIP_color_noise_mpeg_h_detect__yub2)>>8;
	if(scalerVIP_color_noise_mpeg_detect_y_>scalerVIP_color_noise_mpeg_h_detect__x_ub_ub){
		scalerVIP_color_noise_mpeg_detect_y_=scalerVIP_color_noise_mpeg_h_detect__x_ub_ub;
	}
	if(scalerVIP_color_noise_mpeg_detect_y4>scalerVIP_color_noise_mpeg_h_detect__x_ub_ub4)
	{
		scalerVIP_color_noise_mpeg_detect_y4=scalerVIP_color_noise_mpeg_h_detect__x_ub_ub4;
	}
	if(scalerVIP_color_noise_mpeg_detect_y4<scalerVIP_color_noise_mpeg_detect_y_)
	{
		scalerVIP_color_noise_mpeg_detect_y4=scalerVIP_color_noise_mpeg_detect_y_;
	}
	if(scalerVIP_color_noise_mpeg_detect_y4>scalerVIP_color_noise_mpeg_detect_ya4[scalerVIP_color_noise_mpeg_h_detect__z_turn]+3)
	{
		scalerVIP_color_noise_mpeg_detect_y4=scalerVIP_color_noise_mpeg_detect_ya4[scalerVIP_color_noise_mpeg_h_detect__z_turn]+3;
	}
	if(scalerVIP_color_noise_mpeg_detect_y4<scalerVIP_color_noise_mpeg_detect_ya4[scalerVIP_color_noise_mpeg_h_detect__z_turn]-3)
	{
		scalerVIP_color_noise_mpeg_detect_y4=scalerVIP_color_noise_mpeg_detect_ya4[scalerVIP_color_noise_mpeg_h_detect__z_turn]-3;
	}
	scalerVIP_color_noise_mpeg_detect_ya4[scalerVIP_color_noise_mpeg_h_detect__z_turn]=scalerVIP_color_noise_mpeg_detect_y4;
	scalerVIP_color_noise_mpeg_detect_y_delta3=scalerVIP_color_noise_mpeg_detect_y4;
	if(z_max2_4<scalerVIP_color_noise_mpeg_h_detect__HisCntHor[scalerVIP_color_noise_mpeg_h_detect__y_max1p^4]&&z_max1>z_max2_4)
	{
		scalerVIP_color_noise_mpeg_detect_y_delta3=scalerVIP_color_noise_mpeg_detect_y_delta3*(z_max1-scalerVIP_color_noise_mpeg_h_detect__HisCntHor[scalerVIP_color_noise_mpeg_h_detect__y_max1p^4])/(z_max1-z_max2_4);
	}

	if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30
			printk(
					KERN_INFO "\n[pre_mpeg_detect_y]%d %d %d %d %d %d [/pre_mpeg_detect_y]"
				,scalerVIP_color_noise_mpeg_detect_y
					,scalerVIP_color_noise_mpeg_detect_y_
					,RTNR_MAD_count_Y3_count
					,static_cnt
					,scalerVIP_color_noise_mpeg_h_detect__z_mad_pre
					,scalerVIP_color_noise_mpeg_h_detect__z_mad
				);
	}

		if(scalerVIP_color_noise_mpeg_h_detect__z_mad != 0){
		static_cnt = 180;
		scalerVIP_color_noise_mpeg_detect_y = scalerVIP_color_noise_mpeg_detect_y_;
	} else {
			if(static_cnt>=-999 ){
				static_cnt -=3;
			is_static_cnt_tsin_ku=0;
			if(static_cnt>0){
				is_static_cnt_tsin_ku=1;
			}else
			if(static_cnt<-30){
				static_cnt+=30;
				is_static_cnt_tsin_ku=1;
			}
			if(is_static_cnt_tsin_ku){
			if(scalerVIP_color_noise_mpeg_detect_y_>scalerVIP_color_noise_mpeg_detect_y){
				scalerVIP_color_noise_mpeg_detect_y=scalerVIP_color_noise_mpeg_detect_y+1;
			}
			//else if(scalerVIP_color_noise_mpeg_detect_y_<scalerVIP_color_noise_mpeg_detect_y){
			//	scalerVIP_color_noise_mpeg_detect_y=scalerVIP_color_noise_mpeg_detect_y-1;
			//}
			}
		}
		if(static_cnt <=-999)
			static_cnt =-999;


	}
			//if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30

			if(rtd_inl(0xb802e4f0)==3){
			printk(
					KERN_INFO "\n[mpeg_detect_y]%d %d %d[/mpeg_detect_y]"
					,scalerVIP_color_noise_mpeg_detect_y
					,scalerVIP_color_noise_mpeg_detect_y4
					,scalerVIP_color_noise_mpeg_detect_y_delta3
					);
			}
	}while(false);

#if 0
	if((scalerVIP_color_noise_mpeg_detect_x_by_motion_en == 1) && !(nr_dch1_mpegnr1_r.cp_mpglowerbound&0x08) ){

		MCNR_GMV_Ratio = 100 - scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio();//MCNR_GMV_Ratio : 100 -> 0 ( still -> motion )
		if(RTNR_MAD_count_Y3_count > (RTNR_MAD_count_Y3_avg+MCNR_GMV_Ratio)){
			RTNR_MAD_count_Y3_count = RTNR_MAD_count_Y3_count - (((RTNR_MAD_count_Y3_count - (RTNR_MAD_count_Y3_avg+MCNR_GMV_Ratio))>>3) + 1);
		}
		else if(((RTNR_MAD_count_Y3_avg+MCNR_GMV_Ratio) > (RTNR_MAD_count_Y3_count+10))) {
			RTNR_MAD_count_Y3_count = RTNR_MAD_count_Y3_count + ((((RTNR_MAD_count_Y3_avg+MCNR_GMV_Ratio) - RTNR_MAD_count_Y3_count)>>3) + 1) ;
		}

		if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L00])
			scalerVIP_color_noise_mpeg_detect_y_by_motion = 0;
		else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L01])
			scalerVIP_color_noise_mpeg_detect_y_by_motion = 5 ;
		else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L02])
			scalerVIP_color_noise_mpeg_detect_y_by_motion = 10;
		else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L03])
			scalerVIP_color_noise_mpeg_detect_y_by_motion = 20;
		else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L04])
			scalerVIP_color_noise_mpeg_detect_y_by_motion = 25;
		else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L05])
			scalerVIP_color_noise_mpeg_detect_y_by_motion = 32;
		else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L06])
			scalerVIP_color_noise_mpeg_detect_y_by_motion = 32;
		else
			scalerVIP_color_noise_mpeg_detect_y_by_motion = 32;

		if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30
			printk(
				KERN_INFO "\n[motion]%d %d %d %d, [/motion]"
				,RTNR_MAD_count_Y3_avg
				,MCNR_GMV_Ratio
				,RTNR_MAD_count_Y3_count
				,scalerVIP_color_noise_mpeg_detect_y_by_motion
				);
		}

		scalerVIP_color_noise_mpeg_detect_y = (scalerVIP_color_noise_mpeg_detect_y*scalerVIP_color_noise_mpeg_detect_y_by_motion)>>5;


	}
#endif

}
#else
static unsigned int counter_ratio_sum = 0;

void scalerVIP_color_noise_mpeg_detect(void)
{

	_RPC_clues* RPC_SmartPic_clue=NULL;
	RPC_SmartPic_clue = scaler_GetShare_Memory_SmartPic_Clue_Struct();

	if((RPC_SmartPic_clue->S_NetFlix_info.Netflix_Enable) ==1)
	{
		if((RPC_SmartPic_clue->S_NetFlix_info.NetFlix_smooth_Toggle_info.MPEG_BLK_init)==1)
		{
			RPC_SmartPic_clue->S_NetFlix_info.NetFlix_smooth_Toggle_info.MPEG_BLK_init=0;
			return;
		}

	}


	unsigned short   temp = 0;
	unsigned int   MaxValue = 0, SecondMaxValue;
	unsigned short   MaxValue_V, SecondMaxValue_V;
	unsigned char   i, RTNR_Pro;
	unsigned int  MpegRatio, MpegRatio_V;
	unsigned int 	MA_data[3]={0}, RTNR_TempMAD_count_Y;
	unsigned short  width,height;

	// oliver-, not define in darwin.
	mpegnr_ich1_cnthor0_RBUS ich1_CntHor0_reg;
	mpegnr_ich1_cnthor1_RBUS ich1_CntHor1_reg;
	mpegnr_ich1_cnthor2_RBUS ich1_CntHor2_reg;
	mpegnr_ich1_cnthor3_RBUS ich1_CntHor3_reg;
	mpegnr_ich1_cnthor4_RBUS ich1_CntHor4_reg;
	mpegnr_ich1_cnthor5_RBUS ich1_CntHor5_reg;
	mpegnr_ich1_cnthor6_RBUS ich1_CntHor6_reg;
	mpegnr_ich1_cnthor7_RBUS ich1_CntHor7_reg;

	unsigned short      MpgHorStart = 0;
	unsigned short      MpgHorEnd = 0;
	unsigned short      MpgVerStart = 0;
	unsigned short      MpgVerEnd = 0;
	unsigned short      th1,th2,th3,th4;
	unsigned int		counter_H = 0, counter_ratio = 0;

	/*
	if(MA_print_count%60==0)
	{
	ROSPrintf("2.[mpeg_data_reset]ucMode_Curr=%d\n",scaler_disp_info[SLR_MAIN_DISPLAY].ucMode_Curr);
	}
	*/
	mpegnr_ich1_hist_blking_hor_range_RBUS ich1_Hist_Blking_Hor_Range_reg;
	mpegnr_ich1_hist_blking_ver_range_RBUS ich1_Hist_Blking_Ver_Range_reg;

	di_im_di_si_ma_total_fm_sum_RBUS ma_total_fm_sum;
	di_im_di_si_ma_large_fm_pixel_RBUS ma_large_fm_pixel;
	di_im_di_si_ma_large_fm_sum_RBUS ma_large_fm_sum;
	di_im_di_rtnr_mad_y_counter_RBUS rtnr_mad_y_counter;
	di_im_di_rtnr_control_RBUS rtnr_control;

	ma_total_fm_sum.regValue = rtd_inl(DI_IM_DI_SI_MA_TOTAL_FM_SUM_reg);
	ma_large_fm_pixel.regValue = rtd_inl(DI_IM_DI_SI_MA_LARGE_FM_PIXEL_reg);
	ma_large_fm_sum.regValue = rtd_inl(DI_IM_DI_SI_MA_LARGE_FM_SUM_reg);
	rtnr_mad_y_counter.regValue = rtd_inl(DI_IM_DI_RTNR_MAD_Y_COUNTER_reg);
	rtnr_control.regValue = rtd_inl(DI_IM_DI_RTNR_CONTROL_reg);

	MA_data[0] = ma_total_fm_sum.total_frame_motion_sum;
	MA_data[1] = ma_large_fm_pixel.large_frame_motion_pixel;
	MA_data[2] = ma_large_fm_sum.large_frame_motion_sum;
	RTNR_TempMAD_count_Y = rtnr_mad_y_counter.cp_temporal_mad_countery;
	RTNR_Pro = rtnr_control.cp_rtnr_progressive;

	//MPEG block effect detection///////////////////////////////////////////////////////
	//horizotal mpeg detection th
	ich1_Hist_Blking_Hor_Range_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_Hist_Blking_Hor_Range_reg);
	MpgHorStart = ich1_Hist_Blking_Hor_Range_reg.cp_histhorstart ;
	MpgHorEnd = ich1_Hist_Blking_Hor_Range_reg.cp_histhorend;
	// scaler_rtd_outl(MPEGNR_ICH1_HIST_BLKING_HOR_RANGE_reg, ich1_Hist_Blking_Hor_Range_reg.regValue);

	ich1_Hist_Blking_Ver_Range_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_Hist_Blking_Ver_Range_reg);
	MpgVerStart = ich1_Hist_Blking_Ver_Range_reg.cp_histverstart ;
	MpgVerEnd = ich1_Hist_Blking_Ver_Range_reg.cp_histverend;
	// scaler_rtd_outl(MPEGNR_ICH1_HIST_BLKING_HOR_RANGE_reg, ich1_Hist_Blking_Ver_Range_reg.regValue);

	// //scalervip_printf(AutoMADebugMsg,"o---MpgHorStart=%d, MpgHorEnd=%d---o\n",MpgHorStart,MpgHorEnd);
	// //scalervip_printf(AutoMADebugMsg,"o---MpgVerStart=%d, MpgVerEnd=%d---o\n",MpgVerStart,MpgVerEnd);

	width=MpgHorEnd-MpgHorStart;
	height=MpgVerEnd-MpgVerStart;
	th4=(width*height)>>9;//300
	th3=width*height/1152;//300
	th2=th3/3;
	th1=th3/30;

	if(xstep_idx_detect==1)
	{
		th4 =th4*9/16;
		th3 =th3*9/16;
		th2 =th2*9/16;
		th1 =th1*9/16;

	}
	else if(xstep_idx_detect==2)
	{
		th4 =th4*3/8;
		th3 =th3*3/8;
		th2 =th2*3/8;
		th1 =th1*3/8;

	}


	//read horizontal counter to get MpgIdx_X
	//horizontal
	MaxValue = 0;
	SecondMaxValue = 0;
	ich1_CntHor0_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor0_reg);
	ich1_CntHor1_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor1_reg);
	ich1_CntHor2_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor2_reg);
	ich1_CntHor3_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor3_reg);
	ich1_CntHor4_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor4_reg);
	ich1_CntHor5_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor5_reg);
	ich1_CntHor6_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor6_reg);
	ich1_CntHor7_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_CntHor7_reg);
	for(i=0;i<8;i++)//read hor 8 counters
	{
		switch(i)
		{
		case 0:
			temp=ich1_CntHor0_reg.cp_cnthor0;
			break;
		case 1:
			temp=ich1_CntHor1_reg.cp_cnthor1;
			break;
		case 2:
			temp=ich1_CntHor2_reg.cp_cnthor2;
			break;
		case 3:
			temp=ich1_CntHor3_reg.cp_cnthor3;
			break;
		case 4:
			temp=ich1_CntHor4_reg.cp_cnthor4;
			break;
		case 5:
			temp=ich1_CntHor5_reg.cp_cnthor5;
			break;
		case 6:
			temp=ich1_CntHor6_reg.cp_cnthor6;
			break;
		case 7:
			temp=ich1_CntHor7_reg.cp_cnthor7;
			break;
		}
		if(MaxValue <= temp)
		{
			SecondMaxValue = MaxValue;
			MaxValue = temp;
			MpgIdx_X = i;
		}
		else if(SecondMaxValue <= temp)
		{
			SecondMaxValue = temp;
		}
#if 0 // mark for always big weight, 20170704
		counter_H= counter_H + temp;
#endif
	}

#if 0 // mark for smooth MpegRatio, 20170704

	if( (SecondMaxValue<th1) || (SecondMaxValue==0) )
		MpegRatio=0;
	else if(SecondMaxValue<th2)
		MpegRatio=(((MaxValue<<6)/SecondMaxValue)/8);
	else if(SecondMaxValue<th3)
		MpegRatio=(((MaxValue<<6)/SecondMaxValue)/4);
	else if(SecondMaxValue<th4)
		MpegRatio=(((MaxValue<<6)/SecondMaxValue)/2);
	else
		MpegRatio=(MaxValue<<6)/SecondMaxValue;
#else
	counter_H = MaxValue-SecondMaxValue;
	if( (SecondMaxValue<th1) || (SecondMaxValue==0) ){
		MpegRatio=0;
	}else if(SecondMaxValue<th2){
		MpegRatio=(16*(SecondMaxValue-th1))/(th2-th1);
	}else if(SecondMaxValue<th3){
		MpegRatio=16+(16*(SecondMaxValue-th2))/(th3-th2);
	}else if(SecondMaxValue<th4){
		MpegRatio=32+(32*(SecondMaxValue-th3))/(th4-th3);
	}else{
		MpegRatio=64;
	}
	MpegRatio=MpegRatio*(MaxValue-SecondMaxValue)/SecondMaxValue/64;

#endif

	if( (MA_print_count%debug_print_delay_time==0) && (Start_Print.Mpeg_Nr==TRUE) )
	{
		//ROSPrintf("\nField_Count=%d", Field_Count);
		//ROSPrintf("\nxstep=%d", array_xstep[xstep_idx_detect]);
		//ROSPrintf("\nth1 = %d, th2 = %d, th3 = %d th4 = %d\n", th1, th2, th3, th4);
		printk(KERN_EMERG "\nMaxValue = %d, SecondMaxValue = %d, MpegRatio = %d, MpgIdx_X = %d\n", MaxValue, SecondMaxValue, MpegRatio, MpgIdx_X);
	}

	if( (Field_Count%MpegIdxPeriod)==1 )
	{
		MpgIdx_X_arr[(Field_Count/MpegIdxPeriod)%4] =	MpgIdx_X;
		if(Start_Print.Mpeg_Nr==TRUE)
		{
			printk(KERN_EMERG "\nField_Count=%d\n", Field_Count);
			printk(KERN_EMERG "MpegIdxPeriod=%d\n", MpegIdxPeriod);
			printk(KERN_EMERG "MpgIdx_X=%d\n", MpgIdx_X);
		}
	}

	MpegRatioSum+=MpegRatio;//sony demo
	MpegRatioSum_Count++;//sony demo
	Field_Count++;//sony demo
	MpegCounter_H+=counter_H;

#if 1
	if(Start_Print.Mpeg_Nr==TRUE)
	{

	if(MA_print_count%MpegPeriod==0)//sony demo
	{
			printk(KERN_EMERG "o--- scalerVIP_color_noise_mpeg Debug print ---o\n");
			printk(KERN_EMERG "o---MpegCounter_H = %d, counter_H = %d---o\n", MpegCounter_H, counter_H);
#if 0
		ROSPrintf("o---MpgHorStart = %d, MpgHorEnd = %d, MpgVerStart = %d,MpgVerEnd = %d  ---o\n", MpgHorStart, MpgHorEnd, MpgVerStart, MpgVerEnd);
		ROSPrintf("o---th1 = %d, th2 = %d, th3 = %d---o\n", th1, th2, th3);
		ROSPrintf("o---cp_cnthor0 = %d, cp_cnthor1 = %d, cp_cnthor2 = %d, cp_cnthor3=%d---o\n", ich1_CntHor0_reg.cp_cnthor0, ich1_CntHor0_reg.cp_cnthor1, ich1_CntHor2_reg.cp_cnthor2, ich1_CntHor2_reg.cp_cnthor3);
		ROSPrintf("o---cp_cnthor4 = %d, cp_cnthor5 = %d, cp_cnthor6 = %d, cp_cnthor7=%d---o\n", ich1_CntHor4_reg.cp_cnthor4, ich1_CntHor4_reg.cp_cnthor5, ich1_CntHor6_reg.cp_cnthor6, ich1_CntHor6_reg.cp_cnthor7);
		ROSPrintf("o---MaxValue = %d, SecondMaxValue = %d, MpegRatio = %d, MpgIdx_X = %d---o\n", MaxValue, SecondMaxValue, MpegRatio, MpgIdx_X);
		ROSPrintf("o---scalerVip_mpegNR = %d, TableNR_MPEGNR_H = %d, TableNR_MPEGNR_V = %d---o\n", scalerVip_mpegNR, TableNR_MPEGNR_H, TableNR_MPEGNR_V);
#endif
		}
	}
#endif

	//MpegRatioSum+=MpegRatio;//sony demo original
	//MpegRatioSum_Count++;//sony demo original

	//Field_Count++;//sony demo original

	//adjust register for every MpegPeriod input fields///////////////////////////////////////////////////////////////////
	if(th3!=0)
		counter_ratio = (counter_H*10/th3);//ratio of total pixel

	counter_ratio_sum = counter_ratio_sum+counter_ratio;

#if 0
	if(MA_print_count%MpegPeriod==0)
	{
		ROSPrintf("o---MpgHorStart = %d, MpgHorEnd = %d, MpgVerStart = %d,MpgVerEnd = %d  ---o\n", MpgHorStart, MpgHorEnd, MpgVerStart, MpgVerEnd);
		ROSPrintf("o---MpgHor = %d,MpgVer = %d, Sum_total = %d  ---o\n", (MpgHorEnd-MpgHorStart), (MpgVerEnd-MpgVerStart), (MpgHorEnd-MpgHorStart)*(MpgVerEnd-MpgVerStart));
		ROSPrintf("o---counter_H = %d, counter_ratio = %d  ---o\n", counter_H,counter_ratio);
		ROSPrintf("o---counter_ratio_sum = %d,MpegRatioSum=%d---o\n", counter_ratio_sum,MpegRatioSum);
		ROSPrintf("o---MpegRatioSum_Count = %d---o\n", MpegRatioSum_Count);
	}
#endif
}
#endif

int scalerVIP_color_noise_mpeg_v_detect__VisCntVer[8]={0,0,0,0,0,0,0,0};
int scalerVIP_color_noise_mpeg_v_detect__a_scaling[9]={0x1000,0x1000,0x1000,0x1e00,0x1e00,0x1e00,0x2000,0x2000,0x2000};//{0x1000,0x1000,0x1000,0,0,0,0,0,0};//{0x1000,0,0,0x2000,0,0x1e00,0,0x2400,0};//
int scalerVIP_color_noise_mpeg_v_detect__amax1p[3][9]={{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}};
int scalerVIP_color_noise_mpeg_v_detect__a2[3][9]={{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}};
int scalerVIP_color_noise_mpeg_v_detect__a3[3][9]={{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}};
int scalerVIP_color_noise_mpeg_v_detect__x3L=0x2000;// minimum detail
int scalerVIP_color_noise_mpeg_v_detect__x3R=0x3000;// maximum detail
int scalerVIP_color_noise_mpeg_v_detect__xwidth=1920;
int scalerVIP_color_noise_mpeg_v_detect__xheight=1080;
int scalerVIP_color_noise_mpeg_v_detect__x_pipeline=0;
int scalerVIP_color_noise_mpeg_v_detect__x_requires_hor=1;
int scalerVIP_color_noise_mpeg_v_detect__yen=0;
int scalerVIP_color_noise_mpeg_v_detect__yub=0;// higher means more detail
int scalerVIP_color_noise_mpeg_v_detect__yub2=0;// higher means more blocking effect
int scalerVIP_color_noise_mpeg_v_detect__y_scaling=0;
int scalerVIP_color_noise_mpeg_v_detect__y_turn=8;// winning turn
int scalerVIP_color_noise_mpeg_v_detect__y_max1p=0;// the bin with the largest number
int scalerVIP_color_noise_mpeg_v_detect__z_scaling=0;
int scalerVIP_color_noise_mpeg_v_detect__z_turn=8;
int scalerVIP_color_noise_mpeg_v_detect_y=0;
void scalerVIP_color_noise_mpeg_v_detect(void){
	int i=0;//,j=0,k=0;
	int z=0;
	int z_max1=0;
	int z_max2=0;
	//int z_max2p=0;
	int x2=0;// block effect level detected
	int x2L=0x1000;
	int y2L=0;
	int x2R=0x3000;
	int y2R=256;
	int x2div=0x80;
	int x3=0;
	int y3L=48;
	int y3R=12;
	int x3div=0x80;
	int x2L_=0;
	int x2R_=0;
	int x2_=0;


	nr_dch1_mpegnr1_RBUS	nr_dch1_mpegnr1_r;
	nr_dch1_viscntver0_RBUS nr_dch1_viscntver0_r;
	nr_dch1_viscntver1_RBUS nr_dch1_viscntver1_r;
	nr_dch1_viscntver2_RBUS nr_dch1_viscntver2_r;
	nr_dch1_viscntver3_RBUS nr_dch1_viscntver3_r;
	nr_dch1_viscntver4_RBUS nr_dch1_viscntver4_r;
	nr_dch1_viscntver5_RBUS nr_dch1_viscntver5_r;
	nr_dch1_viscntver6_RBUS nr_dch1_viscntver6_r;
	nr_dch1_viscntver7_RBUS nr_dch1_viscntver7_r;
	di_im_di_rtnr_control_RBUS di_im_di_rtnr_control_r;
	DRV_MPEGNR_16_SW_CTRL Mpeg_16_sw_ctrl_v;


	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	scalerVIP_color_noise_mpeg_v_detect__z_turn=scalerVIP_color_noise_mpeg_v_detect__z_turn<8?scalerVIP_color_noise_mpeg_v_detect__z_turn+1:0;
	scalerVIP_color_noise_mpeg_v_detect__z_scaling=scalerVIP_color_noise_mpeg_v_detect__a_scaling[scalerVIP_color_noise_mpeg_v_detect__z_turn];


	drvif_color_get_DRV_MPG_V_SW_Ctrl((DRV_MPEGNR_16_SW_CTRL*)&Mpeg_16_sw_ctrl_v);
	x2L=Mpeg_16_sw_ctrl_v.x2l;
	x2R=Mpeg_16_sw_ctrl_v.x2r;
	scalerVIP_color_noise_mpeg_v_detect__x3L=Mpeg_16_sw_ctrl_v.x3l;
	scalerVIP_color_noise_mpeg_v_detect__x3R=Mpeg_16_sw_ctrl_v.x3r;
	y3L=Mpeg_16_sw_ctrl_v.y3l;
	y3R=Mpeg_16_sw_ctrl_v.y3r;
	scalerVIP_color_noise_mpeg_h_detect__x_ub_ub =Mpeg_16_sw_ctrl_v.x_ub_ub; //current not use it

	//printk("v x2L=%d,x2R=%d,x3l=%d,x3r=%d,y3l=%d,y3r=%d,ub=%d \n ",x2L,x2R,
	//scalerVIP_color_noise_mpeg_h_detect__x3L,scalerVIP_color_noise_mpeg_h_detect__x3R,
	//y3L,y3R,scalerVIP_color_noise_mpeg_h_detect__x_ub_ub);



	//debug for resolution detector!if enable -> disable the V_MPEG resolution detector
	nr_dch1_mpegnr1_r.regValue=IoReg_Read32(NR_DCH1_MPEGNR1_reg);

	// different resolution (default 1920*1080)
	scalerVIP_color_noise_mpeg_v_detect__xwidth=VIP_system_info_structure_table->I_Width;
	scalerVIP_color_noise_mpeg_v_detect__xheight=VIP_system_info_structure_table->I_Height;
	nr_dch1_viscntver0_r.regValue=IoReg_Read32(NR_DCH1_VisCntVer0_reg);
	scalerVIP_color_noise_mpeg_v_detect__VisCntVer[0]=nr_dch1_viscntver0_r.cp_viscntver0;
	nr_dch1_viscntver1_r.regValue=IoReg_Read32(NR_DCH1_VisCntVer1_reg);
	scalerVIP_color_noise_mpeg_v_detect__VisCntVer[1]=nr_dch1_viscntver1_r.cp_viscntver1;
	nr_dch1_viscntver2_r.regValue=IoReg_Read32(NR_DCH1_VisCntVer2_reg);
	scalerVIP_color_noise_mpeg_v_detect__VisCntVer[2]=nr_dch1_viscntver2_r.cp_viscntver2;
	nr_dch1_viscntver3_r.regValue=IoReg_Read32(NR_DCH1_VisCntVer3_reg);
	scalerVIP_color_noise_mpeg_v_detect__VisCntVer[3]=nr_dch1_viscntver3_r.cp_viscntver3;
	nr_dch1_viscntver4_r.regValue=IoReg_Read32(NR_DCH1_VisCntVer4_reg);
	scalerVIP_color_noise_mpeg_v_detect__VisCntVer[4]=nr_dch1_viscntver4_r.cp_viscntver4;
	nr_dch1_viscntver5_r.regValue=IoReg_Read32(NR_DCH1_VisCntVer5_reg);
	scalerVIP_color_noise_mpeg_v_detect__VisCntVer[5]=nr_dch1_viscntver5_r.cp_viscntver5;
	nr_dch1_viscntver6_r.regValue=IoReg_Read32(NR_DCH1_VisCntVer6_reg);
	scalerVIP_color_noise_mpeg_v_detect__VisCntVer[6]=nr_dch1_viscntver6_r.cp_viscntver6;
	nr_dch1_viscntver7_r.regValue=IoReg_Read32(NR_DCH1_VisCntVer7_reg);
	scalerVIP_color_noise_mpeg_v_detect__VisCntVer[7]=nr_dch1_viscntver7_r.cp_viscntver7;
	di_im_di_rtnr_control_r.regValue=IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);
	if(di_im_di_rtnr_control_r.cp_rtnr_progressive==0){
		scalerVIP_color_noise_mpeg_v_detect__xheight=scalerVIP_color_noise_mpeg_v_detect__xheight<<1;
	}
	do{

		if(
			(scalerVIP_color_noise_mpeg_v_detect__z_scaling<0x800)
			||(!scalerVIP_color_noise_mpeg_v_detect__x_pipeline&&scalerVIP_color_noise_mpeg_v_detect__z_turn%3!=2)
		){
			for(i=2;i>=0;i--){
				scalerVIP_color_noise_mpeg_v_detect__amax1p[i][scalerVIP_color_noise_mpeg_v_detect__z_turn]=0;
				scalerVIP_color_noise_mpeg_v_detect__a2[i][scalerVIP_color_noise_mpeg_v_detect__z_turn]=0;
				scalerVIP_color_noise_mpeg_v_detect__a3[i][scalerVIP_color_noise_mpeg_v_detect__z_turn]=0;
			}
			break;
		}
			/*
		if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x80)) {//MPEG_dummy, b8025550[24:31], bit31
				printk(
				KERN_INFO "\n[V_MPEG][vistver] %d %06x %06x %06x %06x %06x %06x %06x %06x[/vistver]"
				,scalerVIP_color_noise_mpeg_v_detect__xheight
				,scalerVIP_color_noise_mpeg_v_detect__VisCntVer[0]
				,scalerVIP_color_noise_mpeg_v_detect__VisCntVer[1]
				,scalerVIP_color_noise_mpeg_v_detect__VisCntVer[2]
				,scalerVIP_color_noise_mpeg_v_detect__VisCntVer[3]
				,scalerVIP_color_noise_mpeg_v_detect__VisCntVer[4]
				,scalerVIP_color_noise_mpeg_v_detect__VisCntVer[5]
				,scalerVIP_color_noise_mpeg_v_detect__VisCntVer[6]
				,scalerVIP_color_noise_mpeg_v_detect__VisCntVer[7]
			);
		}
			//*/
		scalerVIP_color_noise_mpeg_v_detect__y_max1p=0;
		z_max1=scalerVIP_color_noise_mpeg_v_detect__VisCntVer[0];
		z_max2=0;
		for(i=1;i<8;i++){
			z=scalerVIP_color_noise_mpeg_v_detect__VisCntVer[i];
			if(z>z_max1){
				z_max2=z_max1;
				scalerVIP_color_noise_mpeg_v_detect__y_max1p=i;
				z_max1=z;
			}
			else if(z>z_max2){
				z_max2=z;
			}
		}
		x2=z_max1-z_max2;//block effect
		x3=-z_max1-z_max2;//detail
		for(i=0;i<8;i++){
			x3=x3+scalerVIP_color_noise_mpeg_v_detect__VisCntVer[i];
		}
		for(i=2;i>0;i--){
			scalerVIP_color_noise_mpeg_v_detect__amax1p[i][scalerVIP_color_noise_mpeg_v_detect__z_turn]=scalerVIP_color_noise_mpeg_v_detect__amax1p[i-1][scalerVIP_color_noise_mpeg_v_detect__z_turn];
			scalerVIP_color_noise_mpeg_v_detect__a2[i][scalerVIP_color_noise_mpeg_v_detect__z_turn]=scalerVIP_color_noise_mpeg_v_detect__a2[i-1][scalerVIP_color_noise_mpeg_v_detect__z_turn];
			scalerVIP_color_noise_mpeg_v_detect__a3[i][scalerVIP_color_noise_mpeg_v_detect__z_turn]=scalerVIP_color_noise_mpeg_v_detect__a3[i-1][scalerVIP_color_noise_mpeg_v_detect__z_turn];
		}
		x2=(x2*1920/scalerVIP_color_noise_mpeg_v_detect__xwidth*1080/scalerVIP_color_noise_mpeg_v_detect__xheight)|0;
		x3=(x3*320/scalerVIP_color_noise_mpeg_v_detect__xwidth*1080/scalerVIP_color_noise_mpeg_v_detect__xheight)|0;// 320 = 1920 / 6
		scalerVIP_color_noise_mpeg_v_detect__amax1p[0][scalerVIP_color_noise_mpeg_v_detect__z_turn]=scalerVIP_color_noise_mpeg_v_detect__y_max1p;
		if(x2>scalerVIP_color_noise_mpeg_v_detect__a2[0][scalerVIP_color_noise_mpeg_v_detect__z_turn]+0x80)
		{
			x2=scalerVIP_color_noise_mpeg_v_detect__a2[0][scalerVIP_color_noise_mpeg_v_detect__z_turn]+0x80;
		}
		else
		if(x2<scalerVIP_color_noise_mpeg_v_detect__a2[0][scalerVIP_color_noise_mpeg_v_detect__z_turn]-0x80)
		{
			x2=scalerVIP_color_noise_mpeg_v_detect__a2[0][scalerVIP_color_noise_mpeg_v_detect__z_turn]-0x80;
		}
		scalerVIP_color_noise_mpeg_v_detect__a2[0][scalerVIP_color_noise_mpeg_v_detect__z_turn]=x2;
		scalerVIP_color_noise_mpeg_v_detect__a3[0][scalerVIP_color_noise_mpeg_v_detect__z_turn]=x3;
			/*
			printk(
				KERN_EMERG "\n[mpegnr_v] %d %d %d %d %d %d %d[/mpegnr_v]",
				scalerVIP_color_noise_mpeg_v_detect__z_turn,
				scalerVIP_color_noise_mpeg_v_detect__y_turn,
				(scalerVIP_color_noise_mpeg_v_detect__z_turn!=scalerVIP_color_noise_mpeg_v_detect__y_turn?1:0)|(scalerVIP_color_noise_mpeg_v_detect__y_turn<9?2:0),
				scalerVIP_color_noise_mpeg_v_detect__z_scaling,
				x2,
				scalerVIP_color_noise_mpeg_v_detect__a2[1][scalerVIP_color_noise_mpeg_v_detect__y_turn],
				scalerVIP_color_noise_mpeg_v_detect__a2[scalerVIP_color_noise_mpeg_v_detect__y_turn>scalerVIP_color_noise_mpeg_v_detect__z_turn?2:0][scalerVIP_color_noise_mpeg_v_detect__y_turn]
			);
			//*/
		if(
			scalerVIP_color_noise_mpeg_v_detect__y_max1p!=scalerVIP_color_noise_mpeg_v_detect__amax1p[1][scalerVIP_color_noise_mpeg_v_detect__z_turn] ||
			scalerVIP_color_noise_mpeg_v_detect__y_max1p!=scalerVIP_color_noise_mpeg_v_detect__amax1p[2][scalerVIP_color_noise_mpeg_v_detect__z_turn]
		){
			// max1p changed
			if(scalerVIP_color_noise_mpeg_v_detect__y_turn==scalerVIP_color_noise_mpeg_v_detect__z_turn){
				scalerVIP_color_noise_mpeg_v_detect__yen=0;
				for(i=0;i<3;i++){
					scalerVIP_color_noise_mpeg_v_detect__a2[i][scalerVIP_color_noise_mpeg_v_detect__z_turn]=0;
				}
			}
			if(scalerVIP_color_noise_mpeg_v_detect__a2[0][scalerVIP_color_noise_mpeg_v_detect__z_turn]>x2L)
			{
				// cannot change max1p yet
				x2=scalerVIP_color_noise_mpeg_v_detect__a2[1][scalerVIP_color_noise_mpeg_v_detect__z_turn]-0x80;
				scalerVIP_color_noise_mpeg_v_detect__a2[0][scalerVIP_color_noise_mpeg_v_detect__z_turn]=x2;
				scalerVIP_color_noise_mpeg_h_detect__y_max1p=scalerVIP_color_noise_mpeg_v_detect__amax1p[1][scalerVIP_color_noise_mpeg_v_detect__z_turn];
				scalerVIP_color_noise_mpeg_v_detect__amax1p[0][scalerVIP_color_noise_mpeg_v_detect__z_turn]=scalerVIP_color_noise_mpeg_v_detect__amax1p[1][scalerVIP_color_noise_mpeg_v_detect__z_turn];
				break;
			}
			break;
		}
		if(scalerVIP_color_noise_mpeg_v_detect__z_turn!=scalerVIP_color_noise_mpeg_v_detect__y_turn&&scalerVIP_color_noise_mpeg_v_detect__y_turn<9){
			if(
				x2<x3div+scalerVIP_color_noise_mpeg_v_detect__a2[1][scalerVIP_color_noise_mpeg_v_detect__y_turn]
				&&x2<x3div+scalerVIP_color_noise_mpeg_v_detect__a2[scalerVIP_color_noise_mpeg_v_detect__y_turn>scalerVIP_color_noise_mpeg_v_detect__z_turn?2:0][scalerVIP_color_noise_mpeg_v_detect__y_turn]
			){break;}
		}
		scalerVIP_color_noise_mpeg_v_detect__y_turn=scalerVIP_color_noise_mpeg_v_detect__z_turn;
		for(i=2;i>0;i--){
			if(x2>scalerVIP_color_noise_mpeg_v_detect__a2[i][scalerVIP_color_noise_mpeg_v_detect__z_turn]){x2=scalerVIP_color_noise_mpeg_v_detect__a2[i][scalerVIP_color_noise_mpeg_v_detect__z_turn];}
			if(x3<scalerVIP_color_noise_mpeg_v_detect__a3[i][scalerVIP_color_noise_mpeg_v_detect__z_turn]){x3=scalerVIP_color_noise_mpeg_v_detect__a3[i][scalerVIP_color_noise_mpeg_v_detect__z_turn];}
		}
		if(y3L==y3R){
			scalerVIP_color_noise_mpeg_v_detect__yub=y3L;
		}else if(scalerVIP_color_noise_mpeg_v_detect__x3L==scalerVIP_color_noise_mpeg_v_detect__x3R){
			scalerVIP_color_noise_mpeg_v_detect__yub=(y3L+y3R)>>1;
		}else if(x3<=scalerVIP_color_noise_mpeg_v_detect__x3L){
			scalerVIP_color_noise_mpeg_v_detect__yub=y3L;
		}else if(x3>=scalerVIP_color_noise_mpeg_v_detect__x3R){
			scalerVIP_color_noise_mpeg_v_detect__yub=y3R;
		}else{
			// x3div=(abs_value(y3L-y3R)+1)>>1;
			int x3L_=((scalerVIP_color_noise_mpeg_v_detect__x3L+(x3div>>1))/x3div)|0;
			int x3R_=((scalerVIP_color_noise_mpeg_v_detect__x3R+(x3div>>1))/x3div)|0;
			int x3_=((x3+(x3div>>1))/x3div)|0;
			scalerVIP_color_noise_mpeg_v_detect__yub=(y3L*(x3R_-x3_)+y3R*(x3_-x3L_)+((x3R_-x3L_)>>1))/(x3R_-x3L_);
		}
		if(y2L==y2R){
			scalerVIP_color_noise_mpeg_v_detect__yub2=y2L;
		}else if(x2L==x2R){
			scalerVIP_color_noise_mpeg_v_detect__yub2=(y2L+y2R)>>1;
		}else if(x2<=x2L){
			scalerVIP_color_noise_mpeg_v_detect__yub2=y2L;
		}else if(x2>=x2R){
			scalerVIP_color_noise_mpeg_v_detect__yub2=y2R;
		}else{
			x2div=(abs_value(y2L-y2R)+1)>>1;
			x2L_=((x2L+(x2div>>1))/x2div)|0;
			x2R_=((x2R+(x2div>>1))/x2div)|0;
			x2_=((x2+(x2div>>1))/x2div)|0;
			scalerVIP_color_noise_mpeg_v_detect__yub2=(y2L*(x2R_-x2_)+y2R*(x2_-x2L_)+((x2R_-x2L_)>>1))/(x2R_-x2L_);
		}
		scalerVIP_color_noise_mpeg_v_detect__yen=1;
	}while(false);
	scalerVIP_color_noise_mpeg_v_detect_y=(scalerVIP_color_noise_mpeg_v_detect__yub*scalerVIP_color_noise_mpeg_v_detect__yub2)>>8;
}

int scalerVIP_color_noise_mpeg_h_apply__x_14641=0x1100;
int scalerVIP_color_noise_mpeg_h_apply__x_24442=0x1400;

#if 1//mpegnr_h_170705h
void scalerVIP_color_noise_mpeg_apply(void)
{

	mpegnr_ich1_blend_nr_hor_step_RBUS	mpegnr_ich1_blend_nr_hor_step_r	;
	mpegnr_ich1_mpegnr1_RBUS	mpegnr_ich1_mpegnr1_r	;
	mpegnr_ich1_mpegnr2_RBUS	mpegnr_ich1_mpegnr2_r	;
	mpegnr_ich1_mpegnr_ub_delta1_RBUS	mpegnr_ich1_mpegnr_ub_delta1_r	;
	mpegnr_ich1_mpegnr_ub_delta2_RBUS	mpegnr_ich1_mpegnr_ub_delta2_r	;
	mpegnr_ich1_det_hist_hor_step_RBUS	mpegnr_ich1_det_hist_hor_step_r	;
	nr_dch1_mpegnr1_RBUS	nr_dch1_mpegnr1_r;
	nr_dch1_mpegnr1_RBUS nr_dch1_mpegnr1_q;
	static int cnt=0;

	int apply__y_scaling=0x1000;
	int apply__detect_y=0;

	mpegnr_ich1_blend_nr_hor_step_r	.regValue=IoReg_Read32(	MPEGNR_ICH1_blend_NR_Hor_step_reg);
	mpegnr_ich1_mpegnr1_r	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR1_reg);
	mpegnr_ich1_mpegnr2_r	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR2_reg);
	mpegnr_ich1_mpegnr_ub_delta1_r	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR_UB_delta1_reg);
	mpegnr_ich1_mpegnr_ub_delta2_r	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR_UB_delta2_reg);
	mpegnr_ich1_det_hist_hor_step_r	.regValue=IoReg_Read32(	MPEGNR_ICH1_det_Hist_Hor_step_reg);

	nr_dch1_mpegnr1_r.regValue=IoReg_Read32(NR_DCH1_MPEGNR1_reg);

	//debug for resolution detector!if enable -> disable the H_MPEG resolution detector
	nr_dch1_mpegnr1_q.regValue=IoReg_Read32(NR_DCH1_MPEGNR1_reg);
	cnt++;

	apply__y_scaling=mpegnr_ich1_blend_nr_hor_step_r.cp_mpghor_blend_xstep;
	apply__detect_y=mpegnr_ich1_mpegnr1_r.cp_mpgupperbound_x;

	// apply
	scalerVIP_color_noise_mpeg_h_detect__y_scaling=scalerVIP_color_noise_mpeg_h_detect__a_scaling[scalerVIP_color_noise_mpeg_h_detect__y_turn];

	if(
		scalerVIP_color_noise_mpeg_h_detect__a_scaling[scalerVIP_color_noise_mpeg_h_detect__z_turn]>=0x800
		&&(!scalerVIP_color_noise_mpeg_h_detect__x_pipeline||scalerVIP_color_noise_mpeg_h_detect__z_turn%3==2)
	){

		if(apply__y_scaling!=scalerVIP_color_noise_mpeg_h_detect__y_scaling)
		{
			// resolution different, decrease
			if(apply__detect_y>0)
			{
				apply__detect_y--;
			}
			else
			{
				apply__y_scaling=scalerVIP_color_noise_mpeg_h_detect__y_scaling;
			}
		}
		else
		if(apply__detect_y<scalerVIP_color_noise_mpeg_detect_ya4[scalerVIP_color_noise_mpeg_h_detect__y_turn])
		{
			apply__detect_y++;
		}
		else
		if(apply__detect_y>scalerVIP_color_noise_mpeg_detect_ya4[scalerVIP_color_noise_mpeg_h_detect__y_turn])/* &&apply__detect_y>0 */
		{
			apply__detect_y--;
		}


		if(scalerVIP_color_noise_mpeg_h_detect__yen){// ||scalerVIP_color_noise_mpeg_h_detect__z_turn==scalerVIP_color_noise_mpeg_h_detect__y_turn
			if(scalerVIP_color_noise_mpeg_h_detect__x_requires_ver&&scalerVIP_color_noise_mpeg_detect_y>scalerVIP_color_noise_mpeg_v_detect_y){
				scalerVIP_color_noise_mpeg_detect_y=scalerVIP_color_noise_mpeg_v_detect_y;
			}
			mpegnr_ich1_mpegnr2_r.cp_mpegenable_x=1;
			mpegnr_ich1_blend_nr_hor_step_r.cp_mpghor_blend_xstep=apply__y_scaling;
			mpegnr_ich1_mpegnr2_r.cp_mpgidx_x=scalerVIP_color_noise_mpeg_h_detect__amax1p[0][scalerVIP_color_noise_mpeg_h_detect__y_turn];
			mpegnr_ich1_mpegnr1_r.cp_mpgupperbound_x=apply__detect_y;
			mpegnr_ich1_mpegnr_ub_delta2_r.cp_mpgub_delta3=scalerVIP_color_noise_mpeg_detect_y_delta3;
			mpegnr_ich1_mpegnr_ub_delta1_r.cp_mpgub_delta2=apply__detect_y;

                                if(apply__y_scaling ==12288)
                                        mpeg_ratio = 3;
                                else if(apply__y_scaling ==8192)
                                        mpeg_ratio = 2;
                                else if(apply__y_scaling ==4096)
                                        mpeg_ratio = 1;


			if(apply__y_scaling<scalerVIP_color_noise_mpeg_h_apply__x_14641){
				mpegnr_ich1_mpegnr_ub_delta1_r.cp_mpgub_delta1=apply__detect_y;
				mpegnr_ich1_mpegnr1_r.cp_mpgnr_filter_sel=0;
			}else{
				if(apply__y_scaling<scalerVIP_color_noise_mpeg_h_apply__x_24442){
					mpegnr_ich1_mpegnr1_r.cp_mpgnr_filter_sel=2;
					mpegnr_ich1_mpegnr_ub_delta1_r.cp_mpgub_delta1=((scalerVIP_color_noise_mpeg_detect_y*5)/6)+(apply__detect_y-scalerVIP_color_noise_mpeg_detect_y);
				}else{
					mpegnr_ich1_mpegnr1_r.cp_mpgnr_filter_sel=1;
					mpegnr_ich1_mpegnr_ub_delta1_r.cp_mpgub_delta1=(scalerVIP_color_noise_mpeg_detect_y*2/3)+(apply__detect_y-scalerVIP_color_noise_mpeg_detect_y);
}
			}// todo : mpegai
		}
                else
                {
			mpegnr_ich1_mpegnr2_r.cp_mpegenable_x=0;

                        mpeg_ratio = 0;
		}


		if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30
			printk(
				KERN_INFO "\n[apply_mpeg_detect_y]0x%x %d 0x%x[/apply_mpeg_detect_y]"
				,mpegnr_ich1_mpegnr1_r.cp_mpgupperbound_x
				,cnt
				,mpegnr_ich1_mpegnr1_r.regValue
				);
		}

		// write to registers
		IoReg_Write32(	MPEGNR_ICH1_blend_NR_Hor_step_reg,	mpegnr_ich1_blend_nr_hor_step_r.regValue);
		IoReg_Write32(	MPEGNR_ICH1_MPEGNR1_reg	,	mpegnr_ich1_mpegnr1_r.regValue);
		IoReg_Write32(	MPEGNR_ICH1_MPEGNR2_reg	,	mpegnr_ich1_mpegnr2_r.regValue);
		IoReg_Write32(	MPEGNR_ICH1_MPEGNR_UB_delta1_reg,	mpegnr_ich1_mpegnr_ub_delta1_r.regValue);
		IoReg_Write32(	MPEGNR_ICH1_MPEGNR_UB_delta2_reg,	mpegnr_ich1_mpegnr_ub_delta2_r.regValue);
			/*
			printk(
				KERN_EMERG "\n[mpegnr_v]jin %08x %08x %08x %08x[/mpegnr_v]",
				nr_dch1_blend_nr_ver_step_r	.regValue,
				nr_dch1_mpegnr1_r	.regValue,
				nr_dch1_mpegnr_ub_delta1_r	.regValue,
				nr_dch1_mpegnr_ub_delta2_r	.regValue
			);
			//*/
}


	if(mpegnr_ich1_mpegnr1_r.cp_mpgupperbound_x != scalerVIP_color_noise_mpeg_detect_y){

		if((Start_Print.Mpeg_Nr==TRUE) || (nr_dch1_mpegnr1_r.cp_mpglowerbound&0x40)) {//MPEG_dummy, b8025550[24:31], bit30
		printk(
			KERN_INFO "\n[apply_mpeg_detect_y]%x %x[/apply_mpeg_detect_y]"
			,mpegnr_ich1_mpegnr1_r.cp_mpgupperbound_x
			,scalerVIP_color_noise_mpeg_detect_y
			);
		}

	}
	// next frame setting
	{
		int y_next_scaling=scalerVIP_color_noise_mpeg_h_detect__a_scaling[(scalerVIP_color_noise_mpeg_h_detect__z_turn+(scalerVIP_color_noise_mpeg_h_detect__x_pipeline?2:0))%9];
		if(y_next_scaling>=0x800){
			if((nr_dch1_mpegnr1_q.cp_mpglowerbound&0x10)){//MPEG_dummy, b8025550[24:31], bit29
				mpegnr_ich1_det_hist_hor_step_r.cp_histhor_det_xstep=0x1000;
			}else{
				mpegnr_ich1_det_hist_hor_step_r.cp_histhor_det_xstep=y_next_scaling;
			}
			IoReg_Write32(	MPEGNR_ICH1_det_Hist_Hor_step_reg	,	mpegnr_ich1_det_hist_hor_step_r	.regValue);
		}
	}
	// db_apply
	IoReg_SetBits(MPEGNR_ICH1_MPEGNR_DB_CTRL_reg, MPEGNR_ICH1_MPEGNR_DB_CTRL_mpegnr_db_apply_mask); /* DB apply */
			/*
			printk(
				KERN_EMERG "\n[mpegnr_v]jin %d %d %d %d %d[/mpegnr_v]",
				mpegnr_ich1_mpegnr2_r.cp_mpegenable_x,
				scalerVIP_color_noise_mpeg_h_detect__z_turn,
				scalerVIP_color_noise_mpeg_h_detect__y_turn,
				scalerVIP_color_noise_mpeg_h_detect__yub,
				scalerVIP_color_noise_mpeg_h_detect__yub2
			);
			//*/
}
#else

unsigned int RTNR_MAD_count_Y3_count=0;
unsigned int MCNR_GMV_Ratio = 0;
void scalerVIP_color_noise_mpeg_apply(void)
{
	unsigned int		counter_ratio_avg =0 ;
	unsigned int		counter_ratio_th =0;
	unsigned int		MpegRatio_th =0;
	unsigned int		mpgupperbound_x_rate = 0;
	unsigned char 		mpgupperbound_x_gain =0;
	mpegnr_ich1_mpegnr2_RBUS ich1_MPEGNR2_reg;
	static unsigned int MpegRatioSum_Max = 0;
	static unsigned char cp_mpegenable_x_temp = 0,cp_mpegenable_x_temp_keep = 0;
	static signed char flag_cp_mpegenable_x=0;
	unsigned char PQA_table_select = scaler_GetShare_Memory_system_setting_info_Struct()->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_table_select;
	unsigned int *pPQA_InputMAD_MPEG = &(scaler_GetShare_Memory_VIP_TABLE_Struct()->PQA_Input_Table[PQA_table_select][I_MPEG_MAD][0]);

	nr_dch1_local_var1_RBUS dch1_local_var1_reg;
	dch1_local_var1_reg.regValue = scaler_rtd_inl(NR_DCH1_Local_Var1_reg);
	ich1_MPEGNR2_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_MPEGNR2_reg);

	if( Field_Count==MpegPeriod )
	{
		//flora, add for Sirius non8x8 detector/////////////////////////////////////////////////////////
		switch((RPC_system_info_structure_table->VIP_source))
		{
			case VIP_QUALITY_HDMI_4k2kI_30:
			case VIP_QUALITY_HDMI_4k2kP_30:
			case VIP_QUALITY_HDMI_4k2kP_60:
			case VIP_QUALITY_DTV_4k2kI_30:
			case VIP_QUALITY_DTV_4k2kP_30:
			case VIP_QUALITY_DTV_4k2kP_60:
			case VIP_QUALITY_PVR_4k2kI_30:
			case VIP_QUALITY_PVR_4k2kP_30:
			case VIP_QUALITY_PVR_4k2kP_60:
				mpgupperbound_x_gain =1;
				XSTEP_NUM=1;
				array_xstep[0]=0x1000;
				if(flag_cp_mpegenable_x==0)
					MpegRatio_th=120;
				else
					MpegRatio_th=100;
			break;

			case VIP_QUALITY_HDMI_720P:
			case VIP_QUALITY_YPbPr_720P:
			case VIP_QUALITY_DTV_720P:
			case VIP_QUALITY_PVR_720P:
				mpgupperbound_x_gain =1;
				XSTEP_NUM=1;
				array_xstep[0]=0x1c71;
				if(flag_cp_mpegenable_x==0)
					MpegRatio_th=96;
				else
					MpegRatio_th=80;
			break;
			case VIP_QUALITY_HDMI_1080I:
			case VIP_QUALITY_HDMI_1080P:
			case VIP_QUALITY_YPbPr_1080I:
			case VIP_QUALITY_YPbPr_1080P:
			case VIP_QUALITY_PVR_1080I:
			case VIP_QUALITY_PVR_1080P:
				mpgupperbound_x_gain =1;
				XSTEP_NUM=1;
				array_xstep[0]=0x1000;
				if(flag_cp_mpegenable_x==0)
					MpegRatio_th=120;
				else
					MpegRatio_th=100;
			break;
			case VIP_QUALITY_DTV_1080I:
			case VIP_QUALITY_DTV_1080P:
				mpgupperbound_x_gain =1;
				XSTEP_NUM=1;
				array_xstep[0]=0x1000;
				if(flag_cp_mpegenable_x==0)
					MpegRatio_th=340;
				else
					MpegRatio_th=330;
			break;
			case VIP_QUALITY_YPbPr_480I:
			case VIP_QUALITY_YPbPr_576I:
			case VIP_QUALITY_HDMI_480I:
			case VIP_QUALITY_HDMI_576I:

				mpgupperbound_x_gain = 1;
				XSTEP_NUM=1;
				array_xstep[0]=0x1000;
				if(flag_cp_mpegenable_x==0)
					MpegRatio_th=96;
				else
					MpegRatio_th=70;
				break;
			case VIP_QUALITY_YPbPr_480P:
			case VIP_QUALITY_YPbPr_576P:
			case VIP_QUALITY_HDMI_480P:
			case VIP_QUALITY_HDMI_576P:
				mpgupperbound_x_gain =1;
				XSTEP_NUM=1;
				array_xstep[0]=0x1000;
				if(flag_cp_mpegenable_x==0)
					MpegRatio_th=160;
				else
					MpegRatio_th=140;
			break;

			case VIP_QUALITY_DTV_480P:
			case VIP_QUALITY_DTV_576P:
			case VIP_QUALITY_PVR_480P:
			case VIP_QUALITY_PVR_576P:
				mpgupperbound_x_gain =1;
				XSTEP_NUM=1;
				array_xstep[0]=0x1000;
				if(flag_cp_mpegenable_x==0)
					MpegRatio_th=160;
				else
					MpegRatio_th=140;
			break;

			case VIP_QUALITY_DTV_480I:
			case VIP_QUALITY_DTV_576I:
			case VIP_QUALITY_PVR_480I:
			case VIP_QUALITY_PVR_576I:

				mpgupperbound_x_gain =1;
				XSTEP_NUM=1;
				array_xstep[0]=0x1000;
				if(flag_cp_mpegenable_x==0)
					MpegRatio_th=96;
				else
					MpegRatio_th=80;
				break;

			case VIP_QUALITY_CVBS_NTSC:
			case VIP_QUALITY_CVBS_PAL:

				mpgupperbound_x_gain =1;
				XSTEP_NUM=1;
				array_xstep[0]=0x1000;
				if(flag_cp_mpegenable_x==0)
					MpegRatio_th=80;
				else
					MpegRatio_th=75;

				break;

			default:
				mpgupperbound_x_gain =1;
				ich1_MPEGNR2_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_MPEGNR2_reg);
				ich1_MPEGNR2_reg.cp_mpegenable_x = 0;
				scaler_rtd_outl(MPEGNR_ICH1_MPEGNR2_reg, ich1_MPEGNR2_reg.regValue);
				return;
				break;

		}

		//debug for resolution detector!if enable -> disable the H_MPEG resolution detector
		nr_dch1_mpegnr1_RBUS nr_dch1_mpegnr1_q;
		nr_dch1_mpegnr1_q.regValue=IoReg_Read32(NR_DCH1_MPEGNR1_reg);
		if((nr_dch1_mpegnr1_q.cp_mpglowerbound&0x10))//MPEG_dummy, b8025550[24:31], bit28
			XSTEP_NUM = 1;

		xstep_idx_apply=xstep_idx_detect;
		xstep_idx_detect++;
		if(xstep_idx_detect>=XSTEP_NUM)
			xstep_idx_detect=0;

		mpegnr_ich1_det_hist_hor_step_RBUS ich1_det_hist_hor_step_reg;
		ich1_det_hist_hor_step_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_det_Hist_Hor_step_reg);
		ich1_det_hist_hor_step_reg.cp_histhor_det_xstep=array_xstep[xstep_idx_detect];
		scaler_rtd_outl(MPEGNR_ICH1_det_Hist_Hor_step_reg, ich1_det_hist_hor_step_reg.regValue);

		//threshold for center diff
		mpegnr_ich1_det_hist_hor_thd_RBUS ich1_det_hist_hor_thd_reg;
		ich1_det_hist_hor_thd_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_det_Hist_Hor_thd_reg);
		ich1_det_hist_hor_thd_reg.cp_histhor_diff_th_high=0xf0;
		if((system_info_structure_table->Input_src_Type == _SRC_HDMI)&&array_xstep[xstep_idx_detect]==4096)
			ich1_det_hist_hor_thd_reg.cp_histhor_diff_th_low=10;
		else
			ich1_det_hist_hor_thd_reg.cp_histhor_diff_th_low=1;
		scaler_rtd_outl(MPEGNR_ICH1_det_Hist_Hor_thd_reg, ich1_det_hist_hor_thd_reg.regValue);

		////////////////////////////////////////////////////////////////////////////////////////////

		if(MpegRatioSum_Count!=0)
		{
			MpegRatio_ave=(MpegRatioSum)/MpegRatioSum_Count;
			MpegCounter_H_ave = MpegCounter_H/MpegRatioSum_Count;
			counter_ratio_avg = counter_ratio_sum/MpegRatioSum_Count;
		}

		if((Start_Print.Mpeg_Nr==TRUE)||((nr_dch1_mpegnr1_q.cp_mpglowerbound&0x40))) { //MPEG_dummy, b8025550[24:31], bit30
				printk(KERN_INFO "\n=======Mpeg info by Flora================================\n");
				printk(KERN_INFO "MAD_motion_status = %d\n", SmartPic_clue->MAD_motion_status);
				printk(KERN_INFO "MpegRatio_ave=%d	MpegRatio_th=%d, gain=%d\n", MpegRatio_ave, MpegRatio_th,mpgupperbound_x_gain);
				printk(KERN_INFO "counter_ratio_avg=%d	counter_ratio_th=%d\n", counter_ratio_avg, counter_ratio_th);
				printk(KERN_INFO "MpgIdx_X_arr[0]=%d	MpgIdx_X_arr[1]=%d MpgIdx_X_arr[2]=%d\n", MpgIdx_X_arr[0], MpgIdx_X_arr[1],MpgIdx_X_arr[2]);
		}


		if( (MpegRatio_ave>=MpegRatio_th) && (counter_ratio_avg>=counter_ratio_th) &&
				(MpgIdx_X_arr[0]==MpgIdx_X_arr[1]) && (MpgIdx_X_arr[1]==MpgIdx_X_arr[2]) ) {

			NL_mpg=(MpegRatio_ave*3/2-64)>>6;
			if(NL_mpg>10)
				NL_mpg=10;

			if(DynamicOptimizeSystem[12] || scalerVIP_DI_MiddleWare_GetStatus(STATUS_HMC))//LG boat or no.107
				NL_mpg=2;

			if(Start_Print.Mpeg_Nr==TRUE)
			{
				printk(KERN_INFO "NL_mpg=%d\n", NL_mpg);
				printk(KERN_INFO "MpegRatio_ave=	%d MpegRatioSum_Max=%d\n", MpegRatio_ave, MpegRatioSum_Max);
				printk(KERN_INFO "TableNR_MPEGNR_H=	%d scalerVip_mpegNR=%d\n", TableNR_MPEGNR_H, scalerVip_mpegNR);
				printk(KERN_INFO "vip_src=%d Y[1]=%d,Y[2]=%d,sat[0]=%d\n", system_info_structure_table->VIP_source, Y_hist_ratio[1], Y_hist_ratio[2], sat_hist_ratio[0]);
			}

			/* exclude artificial pattern in HDMI 1080i*/
			if ((RPC_system_info_structure_table->VIP_source == VIP_QUALITY_HDMI_1080I) && ((Y_hist_ratio[1] + Y_hist_ratio[2]) > 800)
				&& (sat_hist_ratio[0] > 700))
				NL_mpg = 0;

			if(NL_mpg>=2 && MpegRatio_ave>=MpegRatioSum_Max)
			{
				MpegRatioSum_Max = MpegRatio_ave;
				cp_mpegenable_x_temp = 1;
			}else if(NL_mpg>=1){
				cp_mpegenable_x_temp_keep = 1;
			}
		}
		else
		{
			NL_mpg=0;
		}
		if(xstep_idx_detect == 0)		//Field_Count = MpegPeriod*XSTEP_NUM
		{
			mpegnr_ich1_mpegnr1_RBUS ich1_MPEGNR1_reg;
			ich1_MPEGNR1_reg.regValue = scaler_rtd_inl(MPEGNR_ICH1_MPEGNR1_reg);

			if(MpegCounter_H_ave<=2048)
				mpgupperbound_x_rate = 0;
			else if(MpegCounter_H_ave>=4096)
				mpgupperbound_x_rate = 256;
			else
				mpgupperbound_x_rate = 256 - ((4096-MpegCounter_H_ave)>>3);

			mpgupperbound_x_rate *= mpgupperbound_x_gain;

			if((cp_mpegenable_x_temp == 1) || ((DynamicOptimizeSystem[5]==1)||(DynamicOptimizeSystem[12]==1)||(DynamicOptimizeSystem[23]==1)||(DynamicOptimizeSystem[24]==1)))
			{
				ich1_MPEGNR2_reg.cp_mpegenable_x = 1;
				flag_cp_mpegenable_x++;
				if(flag_cp_mpegenable_x>5)
					flag_cp_mpegenable_x=5;
				ich1_MPEGNR1_reg.cp_mpgupperbound_x = (35 * mpgupperbound_x_rate)>>8;
			}
			else
			{

				//ich1_MPEGNR2_reg.cp_mpegenable_x = 0;
				if(cp_mpegenable_x_temp_keep == 0){
					flag_cp_mpegenable_x--;
				}
				if(flag_cp_mpegenable_x<=0)
					flag_cp_mpegenable_x=0;

				ich1_MPEGNR2_reg.cp_mpegenable_x = 1;

				MCNR_GMV_Ratio = 100 - scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio();//MCNR_GMV_Ratio : 100 -> 0 ( still -> motion )
				if(RTNR_MAD_count_Y3_count > (RTNR_MAD_count_Y3_avg+MCNR_GMV_Ratio)){
					RTNR_MAD_count_Y3_count = RTNR_MAD_count_Y3_count - (((RTNR_MAD_count_Y3_count - (RTNR_MAD_count_Y3_avg+MCNR_GMV_Ratio))>>3) + 1);
				}
				else if(((RTNR_MAD_count_Y3_avg+MCNR_GMV_Ratio) > (RTNR_MAD_count_Y3_count+10))) {
					RTNR_MAD_count_Y3_count = RTNR_MAD_count_Y3_count + ((((RTNR_MAD_count_Y3_avg+MCNR_GMV_Ratio) - RTNR_MAD_count_Y3_count)>>3) + 1) ;
				}

				if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L00])
					ich1_MPEGNR1_reg.cp_mpgupperbound_x = 0;
				else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L01])
					ich1_MPEGNR1_reg.cp_mpgupperbound_x = (5 * mpgupperbound_x_rate)>>8;
				else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L02])
					ich1_MPEGNR1_reg.cp_mpgupperbound_x = (10 * mpgupperbound_x_rate)>>8;
				else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L03])
					ich1_MPEGNR1_reg.cp_mpgupperbound_x = (15 * mpgupperbound_x_rate)>>8;
				else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L04])
					ich1_MPEGNR1_reg.cp_mpgupperbound_x = (20 * mpgupperbound_x_rate)>>8;
				else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L05])
					ich1_MPEGNR1_reg.cp_mpgupperbound_x = (25 * mpgupperbound_x_rate)>>8;
				else if(RTNR_MAD_count_Y3_count<pPQA_InputMAD_MPEG[PQA_I_L06])
					ich1_MPEGNR1_reg.cp_mpgupperbound_x = (30 * mpgupperbound_x_rate)>>8;
				else
					ich1_MPEGNR1_reg.cp_mpgupperbound_x = (35 * mpgupperbound_x_rate)>>8;

				if(flag_cp_mpegenable_x >0)
					ich1_MPEGNR1_reg.cp_mpgupperbound_x = (35 * mpgupperbound_x_rate)>>8;



			}
			scalerVIP_color_noise_mpeg_detect_y=ich1_MPEGNR1_reg.cp_mpgupperbound_x;
			scaler_rtd_outl(MPEGNR_ICH1_MPEGNR1_reg, ich1_MPEGNR1_reg.regValue);
			cp_mpegenable_x_temp = 0;
			cp_mpegenable_x_temp_keep = 0;
			MpegRatioSum_Max = 0;
		}

		ich1_MPEGNR2_reg.cp_mpgidx_x=MpgIdx_X;

#if 1
	if((Start_Print.Mpeg_Nr==TRUE)||((nr_dch1_mpegnr1_q.cp_mpglowerbound&0x40))) { //MPEG_dummy, b8025550[24:31], bit30
		if(ich1_MPEGNR2_reg.cp_mpegenable_x==1) {
			printk(KERN_INFO "o------MPEG NR Enable-------------------------------------------o\n");
			printk(KERN_INFO "o---counter_ratio_avg = %d, xstep_idx_detect=%d, MpegCounter_H_ave=%d  ---o\n", counter_ratio_avg, xstep_idx_detect, MpegCounter_H_ave);
			printk(KERN_INFO "o---MpegRatio_ave = %d, MpegRatioSum = %d, MpegRatioSum_Count = %d\n", MpegRatio_ave, MpegRatioSum, MpegRatioSum_Count);
			printk(KERN_INFO "o---Mpeg_level_sum = %d, mpgupperbound_x_rate = %d----o\n", (scalerVip_mpegNR+TableNR_MPEGNR_H+TableNR_MPEGNR_V), mpgupperbound_x_rate);
			printk(KERN_INFO "o---scalerVip_mpegNR=%d,TableNR_MPEGNR_H=%d,TableNR_MPEGNR_V=%d---o\n",scalerVip_mpegNR,TableNR_MPEGNR_H,TableNR_MPEGNR_V);
			printk(KERN_INFO "o---NL_mpg = %d, cp_mpgidx_x = %d, cp_mpegenable_x=%d ---o\n", NL_mpg, ich1_MPEGNR2_reg.cp_mpgidx_x, ich1_MPEGNR2_reg.cp_mpegenable_x);
			printk(KERN_INFO "o---cp_mpegresultweight_x=%d, cp_mpgdcgain=%d ---o\n", ich1_MPEGNR2_reg.cp_mpegresultweight_x, ich1_MPEGNR2_reg.cp_mpgdcgain);
			printk(KERN_INFO "o---GMV_Ratio=%d, %d, %d ---o\n",  scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(),RTNR_MAD_count_Y3_avg , RTNR_MAD_count_Y3_count);
			printk(KERN_INFO "o----------------------------------------------------------------------------------o\n");
		} else {
			// oliver-
			printk(KERN_INFO "o---scalerVip_mpegNR_H offoffoffoffoffoffoffoffoffoff------o\n");
			printk(KERN_INFO "o---scalerVip_mpegNR=%d,TableNR_MPEGNR_H=%d,TableNR_MPEGNR_V=%d---o\n",scalerVip_mpegNR,TableNR_MPEGNR_H,TableNR_MPEGNR_V);
		}
		}
#endif

		////scalervip_printf(AutoMADebugMsg,"ooo---Scaler_InputSrcGetMainChType()=%d---ooo\n",Scaler_InputSrcGetMainChType());
		////scalervip_printf(AutoMADebugMsg,"ooo---drvif_module_vdc_NoiseStatus_isr=%d---ooo\n",drvif_module_vdc_NoiseStatus_isr());

		if((nr_dch1_mpegnr1_q.cp_mpglowerbound&1)!=1){
		scaler_rtd_outl(MPEGNR_ICH1_MPEGNR2_reg, ich1_MPEGNR2_reg.regValue);
		}

		IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */

		NL_mpg_sharp = NL_mpg;


		counter_ratio_sum =0;
		MpegRatioSum=0;
		MpegCounter_H = 0;
		MpegRatioSum_Count=0;
		Field_Count=0;
		NL_mpg=0;
    }

}
#endif

void scalerVIP_color_noise_mpeg_v_apply(void){

	nr_dch1_det_vist_hor_step_RBUS	nr_dch1_det_vist_hor_step_r	;
	nr_dch1_blend_nr_ver_step_RBUS	nr_dch1_blend_nr_ver_step_r	;
	nr_dch1_mpegnr1_RBUS	nr_dch1_mpegnr1_r	;
	nr_dch1_mpegnr_ub_delta1_RBUS	nr_dch1_mpegnr_ub_delta1_r	;
	nr_dch1_mpegnr_ub_delta2_RBUS	nr_dch1_mpegnr_ub_delta2_r	;
	nr_dch1_mpegnr1_RBUS nr_dch1_mpegnr1_q;

	int apply__y_scaling=0x1000;
	int apply__detect_y=0;

	nr_dch1_det_vist_hor_step_r	.regValue=IoReg_Read32(	NR_DCH1_det_Vist_Hor_step_reg	);
	nr_dch1_blend_nr_ver_step_r	.regValue=IoReg_Read32(	NR_DCH1_blend_NR_Ver_step_reg	);
	nr_dch1_mpegnr1_r	.regValue=IoReg_Read32(	NR_DCH1_MPEGNR1_reg	);
	nr_dch1_mpegnr_ub_delta1_r	.regValue=IoReg_Read32(	NR_DCH1_MPEGNR_UB_delta1_reg	);
	nr_dch1_mpegnr_ub_delta2_r	.regValue=IoReg_Read32(	NR_DCH1_MPEGNR_UB_delta2_reg	);
	// apply
	apply__y_scaling=nr_dch1_blend_nr_ver_step_r.cp_mpgver_blend_ystep;
	apply__detect_y=nr_dch1_mpegnr1_r.cp_mpgupperbound;

	if(scalerVIP_color_noise_mpeg_v_detect__x_requires_hor&&scalerVIP_color_noise_mpeg_v_detect_y>scalerVIP_color_noise_mpeg_detect_y){
		scalerVIP_color_noise_mpeg_v_detect_y=scalerVIP_color_noise_mpeg_detect_y;
	}

	scalerVIP_color_noise_mpeg_v_detect__y_scaling=scalerVIP_color_noise_mpeg_v_detect__a_scaling[scalerVIP_color_noise_mpeg_v_detect__y_turn];

	//debug for resolution detector!if enable -> disable the V_MPEG resolution detector
	nr_dch1_mpegnr1_q.regValue=IoReg_Read32(NR_DCH1_MPEGNR1_reg);

	if(
		scalerVIP_color_noise_mpeg_v_detect__a_scaling[scalerVIP_color_noise_mpeg_v_detect__z_turn]>=0x800
		&&(!scalerVIP_color_noise_mpeg_v_detect__x_pipeline||scalerVIP_color_noise_mpeg_v_detect__z_turn%3==2)
	){
		if(apply__y_scaling!=scalerVIP_color_noise_mpeg_v_detect__y_scaling)
		{
			// resolution different, decrease
			if(apply__detect_y>0)
			{
				apply__detect_y--;
			}
			else
			{
				apply__y_scaling=scalerVIP_color_noise_mpeg_v_detect__y_scaling;
			}
		}
		else
		if(apply__detect_y<scalerVIP_color_noise_mpeg_v_detect_y)
		{
			apply__detect_y++;
		}
		else
		if(apply__detect_y>scalerVIP_color_noise_mpeg_v_detect_y)/* &&apply__detect_y>0 */
		{
			apply__detect_y--;
		}

		if(scalerVIP_color_noise_mpeg_v_detect__yen){// ||scalerVIP_color_noise_mpeg_v_detect__z_turn==scalerVIP_color_noise_mpeg_v_detect__y_turn
			nr_dch1_mpegnr1_r.cp_mpegenable_y_y=1;
			nr_dch1_blend_nr_ver_step_r.cp_mpgver_blend_ystep=apply__y_scaling;
			nr_dch1_mpegnr1_r.cp_mpgidx_y=scalerVIP_color_noise_mpeg_v_detect__amax1p[0][scalerVIP_color_noise_mpeg_v_detect__y_turn];
			nr_dch1_mpegnr1_r.cp_mpgupperbound=apply__detect_y>>0;// K5L changed to 10bits
			nr_dch1_mpegnr_ub_delta2_r.cp_mpgub_delta3=nr_dch1_mpegnr1_r.cp_mpgupperbound;
			nr_dch1_mpegnr_ub_delta1_r.cp_mpgub_delta2=nr_dch1_mpegnr1_r.cp_mpgupperbound;
			if(apply__y_scaling<0x1800){
				nr_dch1_mpegnr_ub_delta1_r.cp_mpgub_delta1=nr_dch1_mpegnr1_r.cp_mpgupperbound;
				if(apply__y_scaling<0x1000){
					nr_dch1_mpegnr1_r.cp_mpglpmode=1;
				}else{
					nr_dch1_mpegnr1_r.cp_mpglpmode=0;
				}
			}else{
				if(apply__y_scaling<0x2000){
					nr_dch1_mpegnr1_r.cp_mpglpmode=3;
					nr_dch1_mpegnr_ub_delta1_r.cp_mpgub_delta1=((apply__detect_y*5)/3)>>3;
				}else{
					nr_dch1_mpegnr1_r.cp_mpglpmode=2;
					nr_dch1_mpegnr_ub_delta1_r.cp_mpgub_delta1=(apply__detect_y/3)>>1;
				}
			}// todo : mpegai
		}else{
			nr_dch1_mpegnr1_r.cp_mpegenable_y_y=0;
		}
		// write to registers
		IoReg_Write32(	NR_DCH1_blend_NR_Ver_step_reg	,	nr_dch1_blend_nr_ver_step_r	.regValue);
		IoReg_Write32(	NR_DCH1_MPEGNR1_reg	,	nr_dch1_mpegnr1_r	.regValue);
		IoReg_Write32(	NR_DCH1_MPEGNR_UB_delta1_reg	,	nr_dch1_mpegnr_ub_delta1_r	.regValue);
		IoReg_Write32(	NR_DCH1_MPEGNR_UB_delta2_reg	,	nr_dch1_mpegnr_ub_delta2_r	.regValue);
			/*
			printk(
				KERN_EMERG "\n[mpegnr_v]jin %08x %08x %08x %08x[/mpegnr_v]",
				nr_dch1_blend_nr_ver_step_r	.regValue,
				nr_dch1_mpegnr1_r	.regValue,
				nr_dch1_mpegnr_ub_delta1_r	.regValue,
				nr_dch1_mpegnr_ub_delta2_r	.regValue
			);
			//*/
	}
	// next frame setting
	{
		int y_next_scaling=scalerVIP_color_noise_mpeg_v_detect__a_scaling[(scalerVIP_color_noise_mpeg_v_detect__z_turn+(scalerVIP_color_noise_mpeg_v_detect__x_pipeline?2:0))%9];
		if(y_next_scaling>=0x800){
			if((nr_dch1_mpegnr1_q.cp_mpglowerbound&0x20)){//MPEG_dummy, b8025550[24:31], bit29
				nr_dch1_det_vist_hor_step_r.cp_visthor_det_ystep=0x1000;
			} else {
			nr_dch1_det_vist_hor_step_r.cp_visthor_det_ystep=y_next_scaling;
			}

			IoReg_Write32(	NR_DCH1_det_Vist_Hor_step_reg	,	nr_dch1_det_vist_hor_step_r	.regValue);
		}
	}
	// db_apply
	IoReg_SetBits(NR_V_MPEGNR_DB_CTRL_reg, NR_V_MPEGNR_DB_CTRL_mpegnr_db_apply_mask); /* DB apply */
			/*
			printk(
				KERN_EMERG "\n[mpegnr_v]jin %d %d %d %d %d[/mpegnr_v]",
				nr_dch1_mpegnr1_r.cp_mpegenable_y_y,
				scalerVIP_color_noise_mpeg_v_detect__z_turn,
				scalerVIP_color_noise_mpeg_v_detect__y_turn,
				scalerVIP_color_noise_mpeg_v_detect__yub,
				scalerVIP_color_noise_mpeg_v_detect__yub2
			);
			//*/
}
//============================Elsie 20130425: color mpeg end=====================
