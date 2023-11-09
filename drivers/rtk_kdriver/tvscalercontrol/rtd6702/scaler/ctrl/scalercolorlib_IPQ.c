/*******************************************************************************
* @file    scalerColorLib.cpp
* @brief
* @note    Copyright (c) 2014 RealTek Technology Co., Ltd.
*		   All rights reserved.
*		   No. 2, Innovation Road II,
*		   Hsinchu Science Park,
*		   Hsinchu 300, Taiwan
*
* @log
* Revision 0.1	2014/01/27
* create
*******************************************************************************/
/*******************************************************************************
 * Header include
******************************************************************************/

#include <linux/delay.h>
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/pageremap.h>
#include <mach/system.h>
#include <linux/hrtimer.h>

#include <rbus/ppoverlay_reg.h>

#include <linux/kthread.h>  /* for threads */

#include <rbus/dm_reg.h>
#include <rbus/hdr_all_top_reg.h>
#include <rbus/vgip_reg.h>
#include <rbus/vodma_reg.h>
#include <rbus/h3ddma_hsd_reg.h>
#include <rbus/sfg_reg.h>
#include <rbus/hdr_all_top_reg.h>


#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv010.h>
#include <tvscalercontrol/scaler/scalercolorlib_IPQ.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scalerdrv/pipmp.h>

#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/vip/dcc.h>
#include <tvscalercontrol/vip/intra.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/xc.h>
#include <tvscalercontrol/vip/di_ma.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/vip/viptable.h>
#include <tvscalercontrol/vip/localcontrast.h>
#include <tvscalercontrol/vip/localdimming.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/ST2094.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>

#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/panel/panelapi.h>

#include <scaler_vpqdev.h>
#include "scaler_vpqmemcdev.h"
#include <scaler_vscdev.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <rtd_log/rtd_module_log.h>

#include "memc_isr/scalerMEMC.h"




/*******************************************************************************
* Macro
******************************************************************************/

/*******************************************************************************
* Constant
******************************************************************************/
/*#define example  100000 */ /* 100Khz */

/* Enable or Disable VIP flag*/



/*******************************************************************************
 * Structure
 ******************************************************************************/

/*typedef struct*/
/*{*/
/*} MID_example_Param_t;*/


/*******************************************************************************
* enumeration
******************************************************************************/
/*typedef enum*/
/*{*/
/*    MID_Example_SLEEPING = 0,*/
/*    MID_Example_RUNNING,*/
/*} MID_Example_Status_t;*/



RTK_VPQ_MODE_TYPE_TV030 picturemode=PQ_MODE_NORMAL;
unsigned int Data_Range_HDR_YUV2RGB_TV043;

void Scaler_Set_Partten4AutoGamma(unsigned char  Enable, unsigned short  r_Val,unsigned short  g_Val,unsigned short  b_Val)
{
	drvif_color_set_Partten4AutoGamma( Enable ,r_Val,g_Val,b_Val);
}

void Scaler_Set_PictureMode_PQsetting(RTK_VPQ_MODE_TYPE_TV030 mode)
{
	extern unsigned char str_resume_do_picturemode;
	_system_setting_info *VIP_system_info_structure_table = NULL;

	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	printk("yuan,PictureMode=%d\n",mode);

	switch(mode){
	case PQ_MODE_BYPASS:
	default:
		Scaler_set_ICM_table(9,0);
		drvif_color_Icm_Enable(OFF);
		break;

	case PQ_MODE_NORMAL:
		Scaler_set_ICM_table(0,0);
		break;

	case PQ_MODE_xvycc:
	case PQ_MODE_DOLBY:
	case PQ_MODE_DCIP3:
	case PQ_MODE_HDR_ENHANCE_WIDE:
		Scaler_set_ICM_table(9,0);
		printk("PictureMode_PQsetting :NOT SUPPORT!!\n");
		break;

	case PQ_MODE_HDR:
		Scaler_set_ICM_table(1,0);
		break;

	case PQ_MODE_SDR2HDR:
		Scaler_set_ICM_table(2,0);
		break;

	case PQ_MODE_FOOTBALL:
		Scaler_set_ICM_table(3,0);
		break;

	case PQ_MODE_FILM:
		Scaler_set_ICM_table(4,0);
		break;

	case PQ_MODE_SHOP_DEMO: //SHOP MODE
		printk("tim_li,HOME -> SHOP dcc !!! \n");
		if(VIP_system_info_structure_table->HDR_flag != HAL_VPQ_HDR_MODE_SDR && VIP_system_info_structure_table->HDR_flag != HAL_VPQ_HDR_MODE_SDR_MAX_RGB)
			break;
		Scaler_set_ICM_table(5,0);
		Scaler_SetDCC_Table(5);
		Scaler_SetDCC_Mode(2);
		Scaler_Set_DCC_Color_Independent_Table(4);
		break;
	}
	if((picturemode !=mode) && (picturemode==PQ_MODE_SHOP_DEMO)){
		printk("tim_li,SHOP -> HOME !!! \n");
		Scaler_SetDCC_Table(Scaler_get_dcc_table_by_source(0));
		Scaler_SetDCC_Mode(Scaler_GetDCC_Mode());
		Scaler_Set_DCC_Color_Independent_Table(Scaler_get_dcc_table_by_source(1));
	}
	picturemode=mode;

	if(str_resume_do_picturemode)
		str_resume_do_picturemode=0;
}

UINT8 fwif_color_get_DM2_HDR_histogram_MaxRGB(UINT32* HDR_info)
{
	unsigned int *pTBL;
	drvif_DM2_HDR_RGB_max_Get(HDR_info);
	//drvif_DM2_HDR_Hist_Position(1);
	//return drvif_DM2_HDR_histogram_Get(HDR_info + 3);
	pTBL = fwif_color_DM2_Hist_AutoMode_TBL_Get(DM2_Hist_RGB_MAX_Mode);
	memcpy((HDR_info + 3), pTBL, sizeof(int)*HDR_Histogram_size);
	return 1;
}

UINT8 fwif_color_get_DM2_HDR_histogram_TV030(UINT32* HDR_info)
{
	unsigned int *pTBL;
	drvif_DM2_HDR_RGB_max_Get(HDR_info);
	//drvif_DM2_HDR_Hist_Position(0);
	//return drvif_DM2_HDR_histogram_Get(HDR_info + 3);
	pTBL = fwif_color_DM2_Hist_AutoMode_TBL_Get(DM2_Hist_Y_Mode);
	memcpy((HDR_info + 3), pTBL, sizeof(int)*HDR_Histogram_size);
	return 1;
}

unsigned short In_OETF[257];
unsigned short Out_T[1025];
unsigned int tmp_gain[1025];
unsigned int sourceMaxL_a = 0;

void Scaler_SetBrightness_LUT_OETF(unsigned short *Out_T , BRIGHTNESS_LUT_OETF *pData)
{
	int i, j;
	unsigned int gain_max;
	unsigned short bit_diff;
	unsigned short shift_bit;

	unsigned int hist_high_sum = 0;
	unsigned short hist_high_pos;
	unsigned int height, width;
	unsigned int hist_sum = 0;
	unsigned short alpha;
	unsigned short alpha_th_table[16];
	static unsigned short alpha_pre = 16;
	static unsigned int alpha_counter = 16 << 4;		
	unsigned int step;
	//UINT16 m;
	#if 1//---->Jimmy's request
	UINT32 HDR_info_bin[131] = {0}; 
	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;
	ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_reg;
	_system_setting_info *VIP_system_info_structure_table = NULL;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		return;
	}

	fwif_color_get_DM2_HDR_histogram_MaxRGB(HDR_info_bin);//--->MAXRGB
	#endif
	if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HDR10) {
	sourceMaxL_a = (pData->pf_LUT[255]>10000)?(10000):((pData->pf_LUT[255]<1023)?(1023):(pData->pf_LUT[255]));
	pData->pf_LUT[255]=pData->pf_LUT[254];
	}
	#if 0//for debug	
			rtd_pr_vpq_debug("OETF Curve\n");
			for (m = 250; m < 256; m++)
				rtd_pr_vpq_info("pf_LUT[%d] = %d\n", m, pData->pf_LUT[m]);
	#endif

	for(i=0;i<256;i++){
		In_OETF[i] = pData->pf_LUT[i]<<6;
	}
	In_OETF[256] = pData->pf_LUT[255]<<6;

	#if 0//for debug
			UINT16 n;
				printk("2yuan=============\n");
			for (n = 0; n < 257; n++)
				printk("In_OETF[%d] = %d\n", n, In_OETF[n]);
	#endif


	for(i=0;i<256;i++){

		bit_diff = In_OETF[(i+1)]-In_OETF[i];

		for(j = 0 ; j < 4 ; j++){
				Out_T[4*i+j]= In_OETF[i]+( (bit_diff*j+2) >> 2 );
			}
	}
	//Out_T[1024]=In_OETF[256];//CXJ	//fix coverity, overwrite by next line
	Out_T[1024] = 0xffff;

	gain_max = 0;
	for(i=1;i<1025;i++)
	{
		//tmp_gain[i]=Out_T[i]*1024/i;
		tmp_gain[i]=(Out_T[i]*1024 + (i>>1))/i;
		if( tmp_gain[i] > gain_max )
			gain_max = tmp_gain[i];
	}
	tmp_gain[0]=tmp_gain[1]; 
	 
	
	// gain max -> shift bit
	/* fix coverity, shift_bit overwriten by next condition, CID 441315, 441317~441323
	if( gain_max < (1u<<16) )
		shift_bit = 0;
	else if( gain_max < (1u<<17) )
		shift_bit = 1;
	else if( gain_max < (1u<<18) )
		shift_bit = 2;
	else if( gain_max < (1u<<19) )
		shift_bit = 3;
	else if( gain_max < (1u<<20) )
		shift_bit = 4;
	else if( gain_max < (1u<<21) )
		shift_bit = 5;
	else if( gain_max < (1u<<22) )
		shift_bit = 6;
	else if( gain_max < (1u<<23) )
		shift_bit = 7;
	else if( gain_max < (1u<<24) )
		shift_bit = 8;
	else 
		shift_bit = 9;
	*/

	if(VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HDR10)
		shift_bit = 9;
	else
		shift_bit = 4; //  HLG

	for( i=0; i<1025; i++ )
		Out_T[i] = (unsigned short)(tmp_gain[i] >> shift_bit);
	Out_T[1024]=Out_T[1023];

	if( shift_bit <= 4 )
	{
		drvif_color_set_BBC_shift_bit(shift_bit);
		drvif_color_Set_ST2094_3Dlut_CInv(1024, 1);
	}
	else // gain > 16x, use 3D LUT inv to cover remaining gain
	{
		drvif_color_set_BBC_shift_bit(4);
		drvif_color_Set_ST2094_3Dlut_CInv(1024<<(shift_bit-4), 1);
	}

	if(VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HDR10) {
	// 20181206, Y_MaxRGB alpha auto by histogram, jimmy
	// get window_height_width
	main_active_h_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);
	main_active_v_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);
	width=main_active_h_start_end_reg.regValue;
	height=main_active_v_start_end_reg.regValue;
	hist_sum = height * width; // assume height & width are read from registers
	// find high 10% bin
	hist_high_sum = 0;
	for( i=127; i>=0; i-- )
	{
		hist_high_sum += HDR_info_bin[i+3];
		if( hist_high_sum >= hist_sum / 10 )
			break;
	}
	hist_high_pos = i;

	if( hist_high_pos >= 120 )
		alpha = 0;
	else if( hist_high_pos >= 88 )
		alpha = 8 - ((hist_high_pos-88)>>2);
	else if( hist_high_pos >= 72 )
		alpha = 16 - ((hist_high_pos-72)>>1);
	else
		alpha = 16;

	// temporal smoothing: only change alpha by 1 per frame
	step = rtd_inl(DM_DM_DUMMY_reg) >> 16;
	if( step == 0 )
		step = 16;

	alpha_pre = alpha_counter >> 4;

	if( alpha != alpha_pre )
	{
		if( alpha > alpha_pre )
		{
			alpha_counter += step;
			if( alpha_counter > (alpha<<4) )
				alpha_counter = (alpha<<4);
		}
		else
		{
			if( alpha_counter < step )
				alpha_counter = 0;
			else
				alpha_counter -= step;
			if( alpha_counter < (alpha<<4) )
				alpha_counter = (alpha<<4);
		}
		alpha = (alpha_counter+8) >> 4;
	}

		// set alpha
		for (i=0;i<16;i++) {
			if (i<(16-alpha))
				alpha_th_table[i] = 0;
			else
				alpha_th_table[i] = 4095;
		}
	} else {
		memset((unsigned char*)alpha_th_table, 0, sizeof(short)*16);
	}

	drvif_color_set_HDR_OETF_Lmix(1, alpha_th_table);

#if 0//for debug
				UINT16 k;
					printk("3yuan=============\n");
				for (k = 0; k < 1025; k)
					printk("Out_T[%d] = %d\n", k, Out_T[k]);
	
#endif

}

void Scaler_SetOETF2Gamma(BRIGHTNESS_LUT_OETF *pData)
{
	//unsigned short Table_length = pData->un16_length;
	unsigned short i,j,bit_diff;
	extern unsigned short OEFT2Gamma[1025];
	static unsigned short intbl[257];
	             #if 0//for debug
			UINT16 m;
			printk("1yuan====Scaler_SetOETF2Gamma=========\n");
			for (m = 0; m < 256; m++)
				printk("pf_LUT[%d] = %d\n", m, pData->pf_LUT[m]);
	              #endif

	for(i=0;i<256;i++){
		intbl[i]=(pData->pf_LUT[i])<<2;
	}
	intbl[256]=4095;
	for(i=0;i<256;i++){
		bit_diff = ((intbl[(i+1)]-intbl[i])>>2);
			for(j = 0 ; j < 4 ; j++)
					OEFT2Gamma[4*i+j]= intbl[i]+bit_diff*j;
	}

			#if 0//for debug
			UINT16 k;
			printk("yuan=============\n");
			for (k = 0; k < 1025; k++)
			printk("OEFT2Gamma[%d] = %d\n", k, OEFT2Gamma[k]);
			#endif

}

unsigned char fwif_color_get_HDR_YUV2RGB_SDR_MAX_RGB_by_HFC_TV043(unsigned char color_format)
{
	unsigned char ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
	unsigned char isHD = 0;
#ifdef CONFIG_FORCE_RUN_I3DDMA
	unsigned char SrcGetFrom = 0;
#endif
	unsigned char display = (unsigned char)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	unsigned char video_format = COLOR_UNKNOW;
	UINT8 data_range = HDMI_CSC_DATA_RANGE_MODE_AUTO;
	SLR_VOINFO* pVOInfo = NULL;
	HDMI_AVI_T pAviInfo = {0};
	unsigned char colorFMT_info = COLORSPACE_MAX;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	unsigned char HDMI_CSC_DataRange_Mode = 255;
	unsigned char IsRGB_YUV_RANGE = 255;
	unsigned char srcType = Scaler_InputSrcGetType(display);
	unsigned short mode = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	unsigned char bUse_HDMI_Range_Detect = 1;
	//unsigned char isHDR_Signal = 0;
#ifdef CONFIG_FORCE_RUN_I3DDMA
	if (fwif_color_get_force_run_i3ddma_enable(display)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(display), &srcType, &SrcGetFrom);
		mode = fwif_color_get_cur_input_timing_mode(display);
	}
#endif

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	drvif_Hdmi_GetAviInfoFrame((HDMI_AVI_T *) (&pAviInfo));

	if(VIP_system_info_structure_table == NULL || pVOInfo == NULL){
		rtd_pr_vpq_emerg("~get NULL warning return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	/*if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HDR10 || VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HLG
		|| VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_ST2094)
		isHDR_Signal = 1;
	else
		isHDR_Signal = 0;*/

	if ((mode !=  _MODE_480I) && (mode !=  _MODE_480P) && (mode !=  _MODE_576I) && (mode !=  _MODE_576P))
		isHD = 1;

	// check color format from header, and data range
	if (srcType == _SRC_HDMI) {
		HDMI_CSC_DataRange_Mode = Scaler_GetHDMI_CSC_DataRange_Mode();
		IsRGB_YUV_RANGE = drvif_IsRGB_YUV_RANGE();
		video_format = drvif_Hdmi_GetColorSpace();

		if ((pAviInfo.EC == 5 || pAviInfo.EC == 6) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED)) {
			colorFMT_info = isBT2020;
		} else if (((pAviInfo.EC == 0 || pAviInfo.EC == 2 || pAviInfo.EC == 3) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED))) {
			colorFMT_info = is601;
		} else if (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_SMPTE170) {
			colorFMT_info = is601;
		} else if ((pAviInfo.EC == 1 && pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED) || (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_ITU709)) {
			colorFMT_info = is709;
		} else {
			if (isHD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}
		}

		if (HDMI_CSC_DataRange_Mode == HDMI_CSC_DATA_RANGE_MODE_AUTO) {
			if (IsRGB_YUV_RANGE==MODE_RAG_FULL)
				data_range = HDMI_CSC_DATA_RANGE_MODE_FULL;
			else if (IsRGB_YUV_RANGE==MODE_RAG_LIMIT)
				data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
			else
				data_range = HDMI_CSC_DATA_RANGE_MODE_AUTO;
		} else {
			data_range = HDMI_CSC_DataRange_Mode;
		}

	} else if (srcType == _SRC_VO) {
		if (pVOInfo->colour_primaries == 1) {
			colorFMT_info = is709;
		} else if (pVOInfo->colour_primaries == 6 || pVOInfo->colour_primaries == 7) {
			colorFMT_info = is601;
		} else if (pVOInfo->colour_primaries == 9) {
			colorFMT_info = isBT2020;
		} else {
			if (isHD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}
		}

		data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;

	} else if (srcType == _SRC_VGA) {
		if (isHD == 0) {
			colorFMT_info = is601;
		} else {
			colorFMT_info = is709;
		}
		data_range = HDMI_CSC_DATA_RANGE_MODE_FULL;

	} else {
		if (isHD == 0) {
			colorFMT_info = is601;
		} else {
			colorFMT_info = is709;
		}
		data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
	}

	if (srcType == _SRC_VGA) {
		if (colorFMT_info == is601) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
		} else {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
		}
		Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
	} else if ((srcType == _SRC_YPBPR) || (srcType == _SRC_COMPONENT)) {
		if (colorFMT_info == is601) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
		} else {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
		}
		Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
	} else if (srcType == _SRC_SCART_RGB) {
		if (colorFMT_info == is601) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
		} else {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
		}
		Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
	} else if ((srcType == _SRC_TV) || (srcType == _SRC_CVBS) || (srcType == _SRC_SV) || (srcType == _SRC_SCART)) {
		ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
		Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
	} else if (srcType == _SRC_HDMI) {
		if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {	// max RGB case
		if (drvif_IsHDMI() && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW)) {
			if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
				if (colorFMT_info == isBT2020) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
				} else if (colorFMT_info == is709) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
				} else if (colorFMT_info == is601) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
				}
					Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
			} else {
				if (colorFMT_info == isBT2020) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
				} else if (colorFMT_info == is709) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
				} else if (colorFMT_info == is601) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
				}
					Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
			}
		} else {	// DVI
			if (video_format !=  0) {/*YUV444 or YUV422*/
					if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
						if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
						}
						Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
					} else {
						if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
						}
						Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
					}
				} else {	/* RGB */
				if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_LIMITED) {
					if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
						}
						Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
					} else {
						if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
						}
						Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
					}
				}
			}
		} else {
			if (drvif_IsHDMI() && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW)) {
				if (video_format !=  0) {/*YUV444 or YUV422*/
					if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
						if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
						}
						Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
					} else {
						if (colorFMT_info == isBT2020) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
					} else if (colorFMT_info == is709) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
					} else if (colorFMT_info == is601) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
					}
						Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
					}
				} else {/*RGB*/
					if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
						if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
						}
						Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
				} else {
					if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
						}
						Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
					}
				}
			} else {	// DVI
				if (video_format !=  0) {/*YUV444 or YUV422*/
					if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
						if (colorFMT_info == isBT2020) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
					} else if (colorFMT_info == is709) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
					} else if (colorFMT_info == is601) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
					}
						Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
					} else {
						if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
						}
						Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
				}
			} else {	/* RGB */
				if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_LIMITED) {
					if (colorFMT_info == isBT2020) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
					} else if (colorFMT_info == is709) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
					} else if (colorFMT_info == is601) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
					}
						Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
				} else {
					if (colorFMT_info == isBT2020) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
					} else if (colorFMT_info == is709) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
					} else if (colorFMT_info == is601) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
					}
						Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
					}
				}
			}
		}
	} else if (srcType == _SRC_VO) {
		if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {	// max RGB
			if ((Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE) == COLOR_RGB) || (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 2)) {
				if (colorFMT_info == isBT2020) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
				} else if (colorFMT_info == is709) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
				} else if (colorFMT_info == is601) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
				}
				Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
			} else if (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 0) { /*DTV source*/
				if (colorFMT_info == isBT2020) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
				} else if (colorFMT_info == is709) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
				} else if (colorFMT_info == is601) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
				}
				Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
			} else {  /*Jpeg source*/
				if (colorFMT_info == isBT2020) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
				} else if (colorFMT_info == is709) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
				} else if (colorFMT_info == is601) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
				}
				Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
			}
			//if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG)
				//VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
		} else {
		if ((Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE) == COLOR_RGB) || (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 2)) {
			if (colorFMT_info == isBT2020) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
			} else if (colorFMT_info == is709) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
			} else if (colorFMT_info == is601) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
			}
				Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
		} else if (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 0) { /*DTV source*/
			if (colorFMT_info == isBT2020) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
			} else if (colorFMT_info == is709) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
			} else if (colorFMT_info == is601) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
			}
				Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
		} else {  /*Jpeg source*/
			if (colorFMT_info == isBT2020) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
			} else if (colorFMT_info == is709) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
			} else if (colorFMT_info == is601) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
			}
				Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_FULL;
		}
		//if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG)
			//VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
		}
	} else {
		ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
		Data_Range_HDR_YUV2RGB_TV043 = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
		rtd_pr_vpq_emerg("CSC_SMR, unhandler type%d,\n", srcType);
	}

	rtd_pr_vpq_info("CSC_SMR,disp=%d,src=%d,fmt=%d,ret=%d,mode=%d,CSC1_range=%d,\n",
		display, srcType, color_format, ret_idx, mode,Data_Range_HDR_YUV2RGB_TV043);

	return ret_idx;

}

unsigned char OETF_STOP=0;
void Scaler_SetIPQ_EN(unsigned char value)
{	
	OETF_STOP=value;
}

unsigned char Scaler_GetIPQ_EN(void)
{	
	return OETF_STOP;
}

unsigned char OETFCurve_STOP=0;
void Scaler_SetIPQCurve_EN(unsigned char value)
{	
	OETFCurve_STOP=value;
}

unsigned char Scaler_GetIPQCurve_EN(void)
{	
	return OETFCurve_STOP;
}

unsigned char fwif_color_get_colorspacergb2yuvtransfer_table_SDR_MAX_RGB_TV043(unsigned char channel, unsigned char nSrcType, unsigned char nSD_HD, bool bUse_HDMI_Range_Detect)
{
	unsigned char video_format = COLOR_UNKNOW;
	unsigned char table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
	UINT8 data_range = HDMI_CSC_DATA_RANGE_MODE_AUTO;
	SLR_VOINFO* pVOInfo = NULL;
	HDMI_AVI_T pAviInfo = {0};
	unsigned char colorFMT_info = COLORSPACE_MAX;
	extern unsigned char RGB2YUV_if_RGB_mode;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	unsigned char HDMI_CSC_DataRange_Mode = 255;
	unsigned char IsRGB_YUV_RANGE = 255;
    unsigned char DolbyHDR_flag, HDR_flag;
	unsigned char VIC;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	drvif_Hdmi_GetAviInfoFrame((HDMI_AVI_T *) (&pAviInfo));
	VIC = pAviInfo.VIC;

	if(VIP_system_info_structure_table == NULL || pVOInfo == NULL){
		rtd_pr_vpq_emerg("~get NULL warning return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
        
        DolbyHDR_flag = VIP_system_info_structure_table->DolbyHDR_flag;
        HDR_flag = VIP_system_info_structure_table->HDR_flag;

	// check color format from header, and data range
	if (nSrcType == _SRC_HDMI) {
		HDMI_CSC_DataRange_Mode = Scaler_GetHDMI_CSC_DataRange_Mode();
		IsRGB_YUV_RANGE = drvif_IsRGB_YUV_RANGE();
		video_format = drvif_Hdmi_GetColorSpace();

		if ((pAviInfo.EC == 5 || pAviInfo.EC == 6) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED)) {
			colorFMT_info = isBT2020;
		} else if (((pAviInfo.EC == 0 || pAviInfo.EC == 2 || pAviInfo.EC == 3) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED))) {
			colorFMT_info = is601;
		} else if (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_SMPTE170) {
			colorFMT_info = is601;
		} else if ((pAviInfo.EC == 1 && pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED) || (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_ITU709)) {
			colorFMT_info = is709;
		} else {
			if (nSD_HD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}
		}

		if (HDMI_CSC_DataRange_Mode == HDMI_CSC_DATA_RANGE_MODE_AUTO) {
			if (IsRGB_YUV_RANGE==MODE_RAG_FULL)
				data_range = HDMI_CSC_DATA_RANGE_MODE_FULL;
			else if (IsRGB_YUV_RANGE==MODE_RAG_LIMIT)
				data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
			else
				data_range = HDMI_CSC_DATA_RANGE_MODE_AUTO;
		} else {
			data_range = HDMI_CSC_DataRange_Mode;
		}

	} else if (nSrcType == _SRC_VO) {
		if (pVOInfo->colour_primaries == 1) {
			colorFMT_info = is709;
		} else if (pVOInfo->colour_primaries == 6 || pVOInfo->colour_primaries == 7) {
			colorFMT_info = is601;
		} else if (pVOInfo->colour_primaries == 9) {
			colorFMT_info = isBT2020;
		} else {
			if (nSD_HD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}
		}

		data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;

	} else if (nSrcType == _SRC_VGA) {
		if (nSD_HD == 0) {
			colorFMT_info = is601;
		} else {
			colorFMT_info = is709;
		}
		data_range = HDMI_CSC_DATA_RANGE_MODE_FULL;

	} else {
		if (nSD_HD == 0) {
			colorFMT_info = is601;
		} else {
			colorFMT_info = is709;
		}
		data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
	}

	/*if (fwif_color_get_force_run_i3ddma_enable(channel)) {
		if (nSrcType == _SRC_VGA || nSrcType == _SRC_HDMI || nSrcType == _SRC_SCART_RGB)
			nSrcType = _SRC_VO;
	}*/

	// "Input_Data_Mode" always use limited, D-doamin will always use "limit to full"
        if (DolbyHDR_flag == 1 && HDR_flag == HAL_VPQ_HDR_MODE_SDR) {
		if (colorFMT_info == isBT2020) {
			table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
		} else if (colorFMT_info == is709) {
			table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
		} else if (colorFMT_info == is601) {
			table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
		}
	} else if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {	// max RGB case

		if (nSrcType == _SRC_HDMI) {
			if (video_format !=  0) {/*YUV444 or YUV422*/
				if (colorFMT_info == isBT2020) {
					table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
				} else if (colorFMT_info == is709) {
					table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_0_255;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				} else if (colorFMT_info == is601) {
					table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
		        }
				
			} else {/*RGB*/				
				RGB2YUV_if_RGB_mode = TRUE;
				if (data_range == HDMI_CSC_DATA_RANGE_MODE_LIMITED || ((data_range == HDMI_CSC_DATA_RANGE_MODE_AUTO) && (drvif_IsHDMI() && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW) && (VIC!=0)))) {
					if (colorFMT_info == isBT2020) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
					} else if (colorFMT_info == is709) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_0_255;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					} else if (colorFMT_info == is601) {
						table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					}					

				} else { /*DVI default or Full*/

					if (colorFMT_info == isBT2020) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
					} else if (colorFMT_info == is709) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					} else if (colorFMT_info == is601) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
			        }				

				} 
					

			}

		} else { // Other source
			
		
		if (colorFMT_info == isBT2020) {
			table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
		} else if (colorFMT_info == is709) {
			table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_0_255;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
		} else if (colorFMT_info == is601) {
			table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
        }
		}
	} else if (nSrcType == _SRC_VGA) {
		if (colorFMT_info == is601) {
			table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
		} else {
			table_num = RGB2YUV_COEF_RGB709_16_235_TO_YCC601_16_235;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
		}
		RGB2YUV_if_RGB_mode = TRUE;

	} else if ((nSrcType == _SRC_YPBPR) || (nSrcType == _SRC_COMPONENT)) {
		if (colorFMT_info == is601) {
			table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
		} else {
			table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
		}
	} else if (nSrcType == _SRC_SCART_RGB) {
		if (colorFMT_info == is601) {
			table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
		} else {
			table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
		}
	} else if ((nSrcType == _SRC_TV) || (nSrcType == _SRC_CVBS) || (nSrcType == _SRC_SV) || (nSrcType == _SRC_SCART)) {
		table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
	} else if (nSrcType == _SRC_HDMI) {
		if (drvif_IsHDMI() && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW)) {
			if (video_format !=  0) {/*YUV444 or YUV422*/
				if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
					if (colorFMT_info == isBT2020) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
					} else if (colorFMT_info == is709) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_0_255;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					} else if (colorFMT_info == is601) {
						table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					}
				} else {
					if (colorFMT_info == isBT2020) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
					} else if (colorFMT_info == is709) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					} else if (colorFMT_info == is601) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					}
				}
			} else {/*RGB*/
				RGB2YUV_if_RGB_mode = TRUE;
				if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
					if (colorFMT_info == isBT2020) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
					} else if (colorFMT_info == is709) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_0_255;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					} else if (colorFMT_info == is601) {
						table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					}
				} else {
					if (colorFMT_info == isBT2020) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
					} else if (colorFMT_info == is709) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					} else if (colorFMT_info == is601) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					}
				}
			}
		} else { /*DVI mode*/
			if ((video_format&(_BIT4|_BIT5))) {/*YUV444 or YUV422*/
				if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
					if (colorFMT_info == isBT2020) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
					} else if (colorFMT_info == is709) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_0_255;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					} else if (colorFMT_info == is601) {
						table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					}
				} else {
					if (colorFMT_info == isBT2020) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
					} else if (colorFMT_info == is709) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					} else if (colorFMT_info == is601) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					}
				}
			} else {/*RGB*/
				RGB2YUV_if_RGB_mode = TRUE;
				if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_LIMITED) {
					if (colorFMT_info == isBT2020) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
					} else if (colorFMT_info == is709) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					} else if (colorFMT_info == is601) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					}
				} else {
					if (colorFMT_info == isBT2020) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
					} else if (colorFMT_info == is709) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_0_255;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					} else if (colorFMT_info == is601) {
						table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					}
				}
			}
		}
	} else if (nSrcType == _SRC_VO) {
		if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG) {	// JPEG flag comes from VSC connect, and don't enter DM
			if (colorFMT_info == isBT2020) {
				table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
			} else if (colorFMT_info == is709) {
				table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
			} else if (colorFMT_info == is601) {
				table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
			}
		} else if ((Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE) == COLOR_RGB) || (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 2)) {
			if (colorFMT_info == isBT2020) {
				table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
			} else if (colorFMT_info == is709) {
				table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
			} else if (colorFMT_info == is601) {
				table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
			}
		} else if (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 0) { /*DTV source*/
			if (colorFMT_info == isBT2020) {
				table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
			} else if (colorFMT_info == is709) {
				table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
			} else if (colorFMT_info == is601) {
				table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
			}
		} else {  /*Jpeg source*/
			if (colorFMT_info == isBT2020) {
				table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
			} else if (colorFMT_info == is709) {
				table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
			} else if (colorFMT_info == is601) {
				table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
			}
		}
	} else {
		rtd_pr_vpq_emerg("CSC_SMR, unhandler type, nSrcType=%d,\n", nSrcType);
		table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
	  }

	rtd_pr_vpq_info("CSC_SMR,disp=%d,Src=%d,HD=%d,detect=%d,i3d_en()=%d,IsHDMI()=%d,video_format=%d,colorFMT_info=%d,data_range=%d,Dolby=%d,HDR_flag=%d,\n",
		channel, nSrcType, nSD_HD, bUse_HDMI_Range_Detect, fwif_color_get_force_run_i3ddma_enable(channel), drvif_IsHDMI(), video_format, colorFMT_info, data_range,DolbyHDR_flag,HDR_flag);

	if(pVOInfo != NULL){
		rtd_pr_vpq_info("CSC_SMR,Hdmi_Mode()=%d,HDMI_CSC_DataRange=%d,IsRGB_YUV=%d,C=%d,EC=%d,VIC=%d,colour_primaries=%d,InputInfo()=%d,VOFrom()=%d,vsc_source=%d,\n",
			Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR), HDMI_CSC_DataRange_Mode, IsRGB_YUV_RANGE, pAviInfo.C, pAviInfo.EC,VIC, pVOInfo->colour_primaries,
			Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE), Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()),Get_DisplayMode_Src(SLR_MAIN_DISPLAY));
	}
	return table_num;

}

unsigned char fwif_color_get_I3DDMA_colorspacergb2yuvtransfer_table_SDR_MAX_RGB_TV043(unsigned char channel, unsigned char nSrcType, unsigned char nSD_HD, bool bUse_HDMI_Range_Detect)
{
	unsigned char table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_0_255;
	SLR_VOINFO* pVOInfo = NULL;
	HDMI_AVI_T pAviInfo = {0};
	unsigned char colorFMT_info = COLORSPACE_MAX;
	_system_setting_info *VIP_system_info_structure_table = NULL;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	drvif_Hdmi_GetAviInfoFrame((HDMI_AVI_T *) (&pAviInfo));

	if(VIP_system_info_structure_table == NULL || pVOInfo == NULL){
		rtd_pr_vpq_emerg("~get NULL warning return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	// check color format from header, and data range
	if (nSrcType == _SRC_HDMI) {

		if ((pAviInfo.EC == 5 || pAviInfo.EC == 6) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED)) {
			colorFMT_info = isBT2020;
		} else if (((pAviInfo.EC == 0 || pAviInfo.EC == 2 || pAviInfo.EC == 3) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED))) {
			colorFMT_info = is601;
		} else if (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_SMPTE170) {
			colorFMT_info = is601;
		} else if ((pAviInfo.EC == 1 && pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED) || (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_ITU709)) {
			colorFMT_info = is709;
		} else {
			if (nSD_HD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}
		}

	} else if (nSrcType == _SRC_VO) {
		if (pVOInfo->colour_primaries == 1) {
			colorFMT_info = is709;
		} else if (pVOInfo->colour_primaries == 6 || pVOInfo->colour_primaries == 7) {
			colorFMT_info = is601;
		} else if (pVOInfo->colour_primaries == 9) {
			colorFMT_info = isBT2020;
		} else {
			if (nSD_HD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}
		}

	} else {
		if (nSD_HD == 0) {
			colorFMT_info = is601;
		} else {
			colorFMT_info = is709;
		}
	}

	if (colorFMT_info == isBT2020) {
		table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
	} else if (colorFMT_info == is709) {
		table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_0_255;
	} else if (colorFMT_info == is601) {
		table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
	}

	return table_num;

}

unsigned char fwif_color_get_colorspacergb2yuvtransfer_table_TV043(unsigned char channel, unsigned char nSrcType, unsigned char nSD_HD, bool bUse_HDMI_Range_Detect)
{
	unsigned char video_format;
	/*unsigned short *table_index = 0;*/
	unsigned char table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
	unsigned char CHOOSE_DATA_RANGE = 1;
	UINT8 data_range = 0; /*morgan_shen  20121112  choose rgb2yuv table by the flag of data range.*/
	RGB2YUV_BLACK_LEVEL blk_lv = fwif_color_get_rgb2yuv_black_level_tv006();
	UINT8 isBt2020_Vo =0, isBt2020_HDMI = 0;
	UINT8 isHDMIFullRangeYCC = 0;
	SLR_VOINFO* pVOInfo = NULL;
	/*pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());*/
	unsigned char real_src_type;
	HDMI_AVI_T pAviInfo = {0};
	/*drvif_Hdmi_GetAviInfoFrame((unsigned char*) (&pAviInfo)); //pAviInfo size = 16 bytes*/

	extern unsigned char RGB2YUV_if_RGB_mode;
	/*extern unsigned short tRGB2YUV_COEF_YCC601_BYPASS_NORMAL_GAIN[];*/
	/*extern unsigned short tRGB2YUV_COEF_YCC709_BYPASS_NORMAL_GAIN[];*/
	/*extern unsigned short tRGB2YUV_COEF_YCC601_0_255_TO_YCC601_16_235[];*/
	/*extern unsigned short tRGB2YUV_COEF_YCC709_0_255_TO_YCC709_16_235[];*/
	/*extern unsigned short tRGB2YUV_COEF_YCC709_16_235_TO_YCC601_16_235[];*/
	/*extern unsigned short tRGB2YUV_COEF_YCC709_0_255_TO_YCC601_16_235[];*/
	/*extern unsigned short tRGB2YUV_COEF_BYPASS_NORMAL_GAIN[];*/
	/*extern unsigned short tRGB2YUV_COEF_BYPASS_0_255_TO_16_235[];*/
	/*extern unsigned short tRGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235[];*/
	/*extern unsigned short tRGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235[];*/
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	drvif_Hdmi_GetAviInfoFrame((HDMI_AVI_T *) (&pAviInfo)); //pAviInfo size = 16 bytes
	if(VIP_system_info_structure_table == NULL){
		rtd_pr_vpq_info("~get VIP_system_info_structure_table warning return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	
	if (RGB2YUV_BLACK_LEVEL_AUTO == blk_lv)
		bUse_HDMI_Range_Detect = true;
	else
		bUse_HDMI_Range_Detect = false;

	if (fwif_color_get_force_run_i3ddma_enable(channel)) {
		real_src_type = nSrcType;
		if (nSrcType == _SRC_HDMI) {
			if (drvif_IsHDMI() && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW)) {
				rtd_pr_vpq_info("=== Check HDMI FullRange YCC ===\n");
				switch(Scaler_GetHDMI_CSC_DataRange_Mode()) {
				case HDMI_CSC_DATA_RANGE_MODE_AUTO:
					isHDMIFullRangeYCC = drvif_IsRGB_YUV_RANGE()==MODE_RAG_FULL?1:0;
					break;
				case HDMI_CSC_DATA_RANGE_MODE_FULL:
					isHDMIFullRangeYCC = 1;
					break;
				case HDMI_CSC_DATA_RANGE_MODE_LIMITED:
				default:
					isHDMIFullRangeYCC = 0;
					break;
					;
				}
			}
		}
		if (nSrcType == _SRC_VGA || nSrcType == _SRC_HDMI || nSrcType == _SRC_SCART_RGB)
			nSrcType = _SRC_VO;
	} else {
		real_src_type = nSrcType;
	}

	//VIPprintf("drvif_color_get_colorspacergb2yuvtransfer_table_tv006, nSrcType = %d, nSD_HD = %d\n", nSrcType, nSD_HD);
#if 0	/* move to HDMI and vo case*/
	/* for bt2020*/
	if (VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag == 1 && VIP_system_info_structure_table->BT2020_CTRL.Mode == BT2020_MODE_Constant) {
		table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_ConstantY;
		return table_num;
	} else if (VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag == 1 && VIP_system_info_structure_table->BT2020_CTRL.Mode == BT2020_MODE_Non_Constant) {
		table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
		return table_num;
	}
#endif
		if (nSrcType == _SRC_VGA) { /* maybe default should be high*/
			if (RGB2YUV_BLACK_LEVEL_LOW == blk_lv) {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			} else {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			}
			RGB2YUV_if_RGB_mode = TRUE;

		} else if ((nSrcType == _SRC_YPBPR) || (nSrcType == _SRC_COMPONENT))  { /*Confused -- why blk low not bypass normal gain?!!*/ /* maybe default should be high*/
			if (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv) {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			} else {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_TV006_YPP_LOW;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_TV006_YPP_LOW;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			}
		} else if (nSrcType == _SRC_SCART_RGB) {
			if (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv) {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			} else {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			}
		} else if ((nSrcType == _SRC_TV) || (nSrcType == _SRC_CVBS) || (nSrcType == _SRC_SV) || (nSrcType == _SRC_SCART)) {
			table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
		} else if (nSrcType == _SRC_HDMI) {
			video_format = drvif_Hdmi_GetColorSpace();
			//VIPprintf(" == =	RGB2YUV video_format = %d == =	\n", video_format);
			if (CHOOSE_DATA_RANGE) {
				data_range = drvif_IsRGB_YUV_RANGE();  /*0 :default 1 : Limited Range 2: Full Range*/
				//VIPprintf(" == =	RGB2YUV  DATA_RANGE = %d  (0 :default 1 : Limited Range 2: Full Range) == =   \n", data_range);
			}
			/* get csc*/
			//drvif_Hdmi_GetAviInfoFrame((HDMI_AVI_T *) (&pAviInfo)); //pAviInfo size = 16 bytes
			if (pAviInfo.EC == 5 || pAviInfo.EC == 6 || VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag == 1)
				isBt2020_HDMI = 1;
			else
				isBt2020_HDMI = 0;

			if (drvif_IsHDMI() && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW)) /*HDMI Video Mode,  gary for xoceco 20071227*/{
				if (video_format !=  0) {/*YUV444 or YUV422*/
					if (isBt2020_HDMI == 1) {
						if ((bUse_HDMI_Range_Detect && data_range == MODE_RAG_FULL) || (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv)) {
							table_num = RGB2YUV_COEF_YCC2020_0_255_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						} else {
							table_num = RGB2YUV_COEF_YCC2020_16_235_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					} else if (!nSD_HD) {
						if ((bUse_HDMI_Range_Detect && data_range == MODE_RAG_FULL) || (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv)) {
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						}
					} else {
						if ((bUse_HDMI_Range_Detect && data_range == MODE_RAG_FULL) || (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv)) {
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						} else {
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}
						//VIPprintf("RGB2YUV_SRC_HDMI_YUV\n");
				} else {/*RGB*/

					RGB2YUV_if_RGB_mode = TRUE;
					if (RGB2YUV_BLACK_LEVEL_LOW == blk_lv) {
						if (isBt2020_HDMI == 1) {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
						} else if (!nSD_HD) {
							table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					} else {
						if (isBt2020_HDMI == 1) {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
						} else if (!nSD_HD) {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}

					//VIPprintf("RGB2YUV_SRC_HDMI_RGB\n");
				}
			} else { /*DVI mode*/
				if ((video_format&(_BIT4|_BIT5))) { /*YUV444 or YUV422*/
					if (!nSD_HD) {
						if (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv) {
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						}
					} else {
						if (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv) {
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						} else {
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}
					//VIPprintf("RGB2YUV_SRC_DVI_YUV\n");
				} else {/*RGB*/
					RGB2YUV_if_RGB_mode = TRUE;
					if (RGB2YUV_BLACK_LEVEL_LOW == blk_lv) {
						if (!nSD_HD) {
							table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					} else {
						if (!nSD_HD) {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}
					//VIPprintf("RGB2YUV_SRC_DVI_RGB\n");
				}
			}
		} else if (nSrcType == _SRC_VO) {
			/*GAME source must enter RGB to YUV*/
			/*if (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 2)*/
			//pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
			if (real_src_type == _SRC_HDMI) {


				
				#if 0 
				if(pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_SMPTE170) //601
					nSD_HD = 0;
				else if(pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_ITU709) //709
					nSD_HD = 1;
				else if(pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_FUTURE)
				{
					if(pAviInfo.EC == 5 || pAviInfo.EC == 6 || VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag == 1 || VIP_system_info_structure_table ->HDR_flag)//bt2020
						isBt2020_HDMI = 1;
					else if(pAviInfo.EC ==1) //709
						nSD_HD = 1;
					else if(pAviInfo.EC ==0)//601
						nSD_HD = 0;
					else //709
						nSD_HD = 1;
				}
				rtd_pr_vpq_info("HDMI,pAviInfo.C=%d,nSD_HD=%d,pAviInfo.EC=%d,isBt2020_HDMI=%d,HDR_flag=%d,drvif_IsHDMI()=%d,Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR)=%d\n",pAviInfo.C,nSD_HD,pAviInfo.EC,isBt2020_HDMI,VIP_system_info_structure_table ->HDR_flag,drvif_IsHDMI(),Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR));
				
				#else
				
				if (pAviInfo.EC == 5 || pAviInfo.EC == 6 || VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag == 1)
					isBt2020_HDMI = 1;
				else
					isBt2020_HDMI = 0;

				#endif

				if (pVOInfo == NULL) {
					isBt2020_Vo = 0;
				} else {
					if (pVOInfo->colour_primaries == 9 /*||pVOInfo->colour_primaries == 2*/ || VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag == 1)	/* bt2020*/
						isBt2020_Vo = 1;
					else
						isBt2020_Vo = 0;
				}

				isBt2020_Vo = isBt2020_Vo | isBt2020_HDMI;


			} else {
				if (pVOInfo == NULL) {
					isBt2020_Vo = 0;
			} else {
				if (pVOInfo->colour_primaries == 9 /*||pVOInfo->colour_primaries == 2*/ || VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag == 1)	/* bt2020*/
					isBt2020_Vo = 1;
				else
					isBt2020_Vo = 0;
				}
			}

			if (real_src_type == _SRC_HDMI) {
				video_format = drvif_Hdmi_GetColorSpace(); /* 0 : RGB 1/2/3 : YUV*/
				if (CHOOSE_DATA_RANGE) {
					data_range = drvif_IsRGB_YUV_RANGE();  /*0 :default 1 : Limited Range 2: Full Range*/
				}
				if (pAviInfo.EC == 5 || pAviInfo.EC == 6 || VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag == 1)
					isBt2020_HDMI = 1;
				else
					isBt2020_HDMI = 0;

				if (drvif_IsHDMI() && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW)) {/*HDMI Video Mode,  gary for xoceco 20071227*/
					if (video_format !=  0) {/*YUV444 or YUV422*/
						if (isBt2020_HDMI == 1) {
							rtd_pr_vpq_info("=== RealHDMI Path13 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
							
						} else if (!nSD_HD) {
							rtd_pr_vpq_info("=== RealHDMI Path14 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						
						} else {
							rtd_pr_vpq_info("=== RealHDMI Path15 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}

					} else {/*RGB*/

						RGB2YUV_if_RGB_mode = TRUE;
						if (isBt2020_HDMI == 1) {
							if (data_range== MODE_RAG_FULL) {
								rtd_pr_vpq_info("=== RealHDMI Path1 ===\n");
								table_num = RGB2YUV_COEF_YCC2020_0_255_TO_YCC709_16_235;
								VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
							} else {
								rtd_pr_vpq_info("=== RealHDMI Path2 ===\n");
								table_num = RGB2YUV_COEF_YCC2020_16_235_TO_YCC709_16_235;
								VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
							}
						} else if (!nSD_HD) {
							if (data_range== MODE_RAG_FULL) {
								rtd_pr_vpq_info("=== RealHDMI Path3 ===\n");
								table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
								VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
							} else { /*default limit*/
								rtd_pr_vpq_info("=== RealHDMI Path4 ===\n");
								table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
								VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
							}
						} else {
							if (data_range== MODE_RAG_FULL) {
								rtd_pr_vpq_info("=== RealHDMI Path5 ===\n");
								table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
								VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
							} else { /*default limit*/
								rtd_pr_vpq_info("=== RealHDMI Path6 ===\n");
								table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
								VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
							}

						}
\
					}
				} else { /*DVI mode*/
					if ((video_format&(_BIT4|_BIT5))) { /*YUV444 or YUV422*/
						if (!nSD_HD) {
							rtd_pr_vpq_info("=== RealHDMI Path7 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							rtd_pr_vpq_info("=== RealHDMI Path8 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;

						}

					} else {/*RGB*/
						RGB2YUV_if_RGB_mode = TRUE;
						if (!nSD_HD) {
							rtd_pr_vpq_info("=== RealHDMI Path9 ===\n");
							if (data_range== MODE_RAG_LIMIT) {
								table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
								VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
							} else { /*default limit*/
								rtd_pr_vpq_info("=== RealHDMI Path10 ===\n");
								table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
								VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
							}
						} else {
							if (data_range== MODE_RAG_LIMIT) {
								rtd_pr_vpq_info("=== RealHDMI Path11 ===\n");
								table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
								VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
							} else { /*default limit*/
								rtd_pr_vpq_info("=== RealHDMI Path12 ===\n");
								table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
								VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
							}

						}



					}
				}
				

			} else {

				if ((Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE) == COLOR_RGB) || (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 2)) {
					if (RGB2YUV_BLACK_LEVEL_LOW == blk_lv) {
						if (isBt2020_Vo == 1) {
							rtd_pr_vpq_info("=== Path1 ===\n");
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
						}
						else if (!nSD_HD) {
							rtd_pr_vpq_info("=== Path2 ===\n");
							table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							rtd_pr_vpq_info("=== Path3 ===\n");
							table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					} else {
						
						if (isBt2020_Vo == 1) {
							rtd_pr_vpq_info("=== Path4 ===\n");
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
						}
						else if (!nSD_HD) {
							rtd_pr_vpq_info("=== Path5 ===\n");
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							rtd_pr_vpq_info("=== Path6 ===\n");
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}
				} else if (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 0) { /*DTV source*/
					if (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv || isHDMIFullRangeYCC) {
						if (isBt2020_Vo == 1) {
							rtd_pr_vpq_info("=== Path7 ===\n");
							table_num = RGB2YUV_COEF_YCC2020_0_255_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						} else if (!nSD_HD) {
							rtd_pr_vpq_info("=== Path8 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							rtd_pr_vpq_info("=== Path9 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					} else {
						if (isBt2020_Vo == 1) {
							rtd_pr_vpq_info("=== Path10 ===\n");
							table_num = RGB2YUV_COEF_YCC2020_16_235_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						} else if (!nSD_HD) {
							rtd_pr_vpq_info("=== Path11 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							rtd_pr_vpq_info("=== Path12 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}
				} else {  /*Jpeg source*/
					if (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv) {
						if (!nSD_HD) {
							rtd_pr_vpq_info("=== Path13 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							rtd_pr_vpq_info("=== Path14 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					} else {
						if (!nSD_HD) {
							rtd_pr_vpq_info("=== Path15 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							rtd_pr_vpq_info("=== Path16 ===\n");
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}
				}
				if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG)
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;

			

			}

			
		} else {
			/*which_tab = 0;*/
			//VIPprintf("unhandler type%d\n", nSrcType);
			table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
			/*return NULL;*/
	  }

	rtd_pr_vpq_info("table_num=%d, disp=%d, Src=%d, nSD_HD=%d, HDMI_Range_Detect=%d, blk_lv=%d, force_run_i3ddma_en()=%d, IsHDMI()=%d, Hdmi_ColorSpace()=%d\n",
		table_num, channel, nSrcType, nSD_HD, bUse_HDMI_Range_Detect, blk_lv, fwif_color_get_force_run_i3ddma_enable(channel), drvif_IsHDMI(), drvif_Hdmi_GetColorSpace());

	if(pVOInfo != NULL){
		rtd_pr_vpq_info("Hdmi_ModeInfo()=%d, HDMI_CSC_DataRange()=%d, IsRGB_YUV()=%d, EC=%d, colour_primaries=%d,InputInfo()=%d,VOFrom()=%d, Src()=%d\n",
			Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR), Scaler_GetHDMI_CSC_DataRange_Mode(), drvif_IsRGB_YUV_RANGE(), pAviInfo.EC, pVOInfo->colour_primaries,
			Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE), Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()), Get_DisplayMode_Src(SLR_MAIN_DISPLAY));
	}
	return table_num;

}

#define Bin_Num_Gamma 1024
unsigned short tGammaTable_Decoded_R[Bin_Num_Gamma + 1], tGammaTable_Decoded_G[Bin_Num_Gamma + 1], tGammaTable_Decoded_B[Bin_Num_Gamma + 1];
unsigned int GammaTable[512*3];
extern struct semaphore Gamma_Semaphore;
extern unsigned int *tGAMMA_temp_R;
extern unsigned int *tGAMMA_temp_G;
extern unsigned int *tGAMMA_temp_B;
extern UINT16 GOut_R[Bin_Num_Gamma + 1], GOut_G[Bin_Num_Gamma + 1], GOut_B[Bin_Num_Gamma + 1];
extern unsigned int final_OutGAMMA_R[Bin_Num_Output_Gamma / 2], final_OutGAMMA_G[Bin_Num_Output_Gamma / 2], final_OutGAMMA_B[Bin_Num_Output_Gamma / 2];
unsigned int LinearOutputGamma[Bin_Num_Output_Gamma / 2]={
0x00000808,	0x00100808,	0x00200808,	0x00300808,	0x00400808,	0x00500808,	0x00600808,	0x00700808,	//W   1 ~   8
0x00800808,	0x00900808,	0x00a00808,	0x00b00808,	0x00c00808,	0x00d00808,	0x00e00808,	0x00f00808,	//W   9 ~  16
0x01000808,	0x01100808,	0x01200808,	0x01300808,	0x01400808,	0x01500808,	0x01600808,	0x01700808,	//W  17 ~  24
0x01800808,	0x01900808,	0x01a00808,	0x01b00808,	0x01c00808,	0x01d00808,	0x01e00808,	0x01f00808,	//W  25 ~  32
0x02000808,	0x02100808,	0x02200808,	0x02300808,	0x02400808,	0x02500808,	0x02600808,	0x02700808,	//W  33 ~  40
0x02800808,	0x02900808,	0x02a00808,	0x02b00808,	0x02c00808,	0x02d00808,	0x02e00808,	0x02f00808,	//W  41 ~  48
0x03000808,	0x03100808,	0x03200808,	0x03300808,	0x03400808,	0x03500808,	0x03600808,	0x03700808,	//W  49 ~  56
0x03800808,	0x03900808,	0x03a00808,	0x03b00808,	0x03c00808,	0x03d00808,	0x03e00808,	0x03f00808,	//W  57 ~  64
0x04000808,	0x04100808,	0x04200808,	0x04300808,	0x04400808,	0x04500808,	0x04600808,	0x04700808,	//W  65 ~  72
0x04800808,	0x04900808,	0x04a00808,	0x04b00808,	0x04c00808,	0x04d00808,	0x04e00808,	0x04f00808,	//W  73 ~  80
0x05000808,	0x05100808,	0x05200808,	0x05300808,	0x05400808,	0x05500808,	0x05600808,	0x05700808,	//W  81 ~  88
0x05800808,	0x05900808,	0x05a00808,	0x05b00808,	0x05c00808,	0x05d00808,	0x05e00808,	0x05f00808,	//W  89 ~  96
0x06000808,	0x06100808,	0x06200808,	0x06300808,	0x06400808,	0x06500808,	0x06600808,	0x06700808,	//W  97 ~ 104
0x06800808,	0x06900808,	0x06a00808,	0x06b00808,	0x06c00808,	0x06d00808,	0x06e00808,	0x06f00808,	//W 105 ~ 112
0x07000808,	0x07100808,	0x07200808,	0x07300808,	0x07400808,	0x07500808,	0x07600808,	0x07700808,	//W 113 ~ 120
0x07800808,	0x07900808,	0x07a00808,	0x07b00808,	0x07c00808,	0x07d00808,	0x07e00808,	0x07f00808,	//W 121 ~ 128
0x08000808,	0x08100808,	0x08200808,	0x08300808,	0x08400808,	0x08500808,	0x08600808,	0x08700808,	//W 129 ~ 136
0x08800808,	0x08900808,	0x08a00808,	0x08b00808,	0x08c00808,	0x08d00808,	0x08e00808,	0x08f00808,	//W 137 ~ 144
0x09000808,	0x09100808,	0x09200808,	0x09300808,	0x09400808,	0x09500808,	0x09600808,	0x09700808,	//W 145 ~ 152
0x09800808,	0x09900808,	0x09a00808,	0x09b00808,	0x09c00808,	0x09d00808,	0x09e00808,	0x09f00808,	//W 153 ~ 160
0x0a000808,	0x0a100808,	0x0a200808,	0x0a300808,	0x0a400808,	0x0a500808,	0x0a600808,	0x0a700808,	//W 161 ~ 168
0x0a800808,	0x0a900808,	0x0aa00808,	0x0ab00808,	0x0ac00808,	0x0ad00808,	0x0ae00808,	0x0af00808,	//W 169 ~ 176
0x0b000808,	0x0b100808,	0x0b200808,	0x0b300808,	0x0b400808,	0x0b500808,	0x0b600808,	0x0b700808,	//W 177 ~ 184
0x0b800808,	0x0b900808,	0x0ba00808,	0x0bb00808,	0x0bc00808,	0x0bd00808,	0x0be00808,	0x0bf00808,	//W 185 ~ 192
0x0c000808,	0x0c100808,	0x0c200808,	0x0c300808,	0x0c400808,	0x0c500808,	0x0c600808,	0x0c700808,	//W 193 ~ 200
0x0c800808,	0x0c900808,	0x0ca00808,	0x0cb00808,	0x0cc00808,	0x0cd00808,	0x0ce00808,	0x0cf00808,	//W 201 ~ 208
0x0d000808,	0x0d100808,	0x0d200808,	0x0d300808,	0x0d400808,	0x0d500808,	0x0d600808,	0x0d700808,	//W 209 ~ 216
0x0d800808,	0x0d900808,	0x0da00808,	0x0db00808,	0x0dc00808,	0x0dd00808,	0x0de00808,	0x0df00808,	//W 217 ~ 224
0x0e000808,	0x0e100808,	0x0e200808,	0x0e300808,	0x0e400808,	0x0e500808,	0x0e600808,	0x0e700808,	//W 225 ~ 232
0x0e800808,	0x0e900808,	0x0ea00808,	0x0eb00808,	0x0ec00808,	0x0ed00808,	0x0ee00808,	0x0ef00808,	//W 233 ~ 240
0x0f000808,	0x0f100808,	0x0f200808,	0x0f300808,	0x0f400808,	0x0f500808,	0x0f600808,	0x0f700808,	//W 241 ~ 248
0x0f800808,	0x0f900808,	0x0fa00808,	0x0fb00808,	0x0fc00808,	0x0fd00808,	0x0fe00808,	0x0ff00808,	//W 249 ~ 256
0x10000808,	0x10100808,	0x10200808,	0x10300808,	0x10400808,	0x10500808,	0x10600808,	0x10700808,	//W 257 ~ 264
0x10800808,	0x10900808,	0x10a00808,	0x10b00808,	0x10c00808,	0x10d00808,	0x10e00808,	0x10f00808,	//W 265 ~ 272
0x11000808,	0x11100808,	0x11200808,	0x11300808,	0x11400808,	0x11500808,	0x11600808,	0x11700808,	//W 273 ~ 280
0x11800808,	0x11900808,	0x11a00808,	0x11b00808,	0x11c00808,	0x11d00808,	0x11e00808,	0x11f00808,	//W 281 ~ 288
0x12000808,	0x12100808,	0x12200808,	0x12300808,	0x12400808,	0x12500808,	0x12600808,	0x12700808,	//W 289 ~ 296
0x12800808,	0x12900808,	0x12a00808,	0x12b00808,	0x12c00808,	0x12d00808,	0x12e00808,	0x12f00808,	//W 297 ~ 304
0x13000808,	0x13100808,	0x13200808,	0x13300808,	0x13400808,	0x13500808,	0x13600808,	0x13700808,	//W 305 ~ 312
0x13800808,	0x13900808,	0x13a00808,	0x13b00808,	0x13c00808,	0x13d00808,	0x13e00808,	0x13f00808,	//W 313 ~ 320
0x14000808,	0x14100808,	0x14200808,	0x14300808,	0x14400808,	0x14500808,	0x14600808,	0x14700808,	//W 321 ~ 328
0x14800808,	0x14900808,	0x14a00808,	0x14b00808,	0x14c00808,	0x14d00808,	0x14e00808,	0x14f00808,	//W 329 ~ 336
0x15000808,	0x15100808,	0x15200808,	0x15300808,	0x15400808,	0x15500808,	0x15600808,	0x15700808,	//W 337 ~ 344
0x15800808,	0x15900808,	0x15a00808,	0x15b00808,	0x15c00808,	0x15d00808,	0x15e00808,	0x15f00808,	//W 345 ~ 352
0x16000808,	0x16100808,	0x16200808,	0x16300808,	0x16400808,	0x16500808,	0x16600808,	0x16700808,	//W 353 ~ 360
0x16800808,	0x16900808,	0x16a00808,	0x16b00808,	0x16c00808,	0x16d00808,	0x16e00808,	0x16f00808,	//W 361 ~ 368
0x17000808,	0x17100808,	0x17200808,	0x17300808,	0x17400808,	0x17500808,	0x17600808,	0x17700808,	//W 369 ~ 376
0x17800808,	0x17900808,	0x17a00808,	0x17b00808,	0x17c00808,	0x17d00808,	0x17e00808,	0x17f00808,	//W 377 ~ 384
0x18000808,	0x18100808,	0x18200808,	0x18300808,	0x18400808,	0x18500808,	0x18600808,	0x18700808,	//W 385 ~ 392
0x18800808,	0x18900808,	0x18a00808,	0x18b00808,	0x18c00808,	0x18d00808,	0x18e00808,	0x18f00808,	//W 393 ~ 400
0x19000808,	0x19100808,	0x19200808,	0x19300808,	0x19400808,	0x19500808,	0x19600808,	0x19700808,	//W 401 ~ 408
0x19800808,	0x19900808,	0x19a00808,	0x19b00808,	0x19c00808,	0x19d00808,	0x19e00808,	0x19f00808,	//W 409 ~ 416
0x1a000808,	0x1a100808,	0x1a200808,	0x1a300808,	0x1a400808,	0x1a500808,	0x1a600808,	0x1a700808,	//W 417 ~ 424
0x1a800808,	0x1a900808,	0x1aa00808,	0x1ab00808,	0x1ac00808,	0x1ad00808,	0x1ae00808,	0x1af00808,	//W 425 ~ 432
0x1b000808,	0x1b100808,	0x1b200808,	0x1b300808,	0x1b400808,	0x1b500808,	0x1b600808,	0x1b700808,	//W 433 ~ 440
0x1b800808,	0x1b900808,	0x1ba00808,	0x1bb00808,	0x1bc00808,	0x1bd00808,	0x1be00808,	0x1bf00808,	//W 441 ~ 448
0x1c000808,	0x1c100808,	0x1c200808,	0x1c300808,	0x1c400808,	0x1c500808,	0x1c600808,	0x1c700808,	//W 449 ~ 456
0x1c800808,	0x1c900808,	0x1ca00808,	0x1cb00808,	0x1cc00808,	0x1cd00808,	0x1ce00808,	0x1cf00808,	//W 457 ~ 464
0x1d000808,	0x1d100808,	0x1d200808,	0x1d300808,	0x1d400808,	0x1d500808,	0x1d600808,	0x1d700808,	//W 465 ~ 472
0x1d800808,	0x1d900808,	0x1da00808,	0x1db00808,	0x1dc00808,	0x1dd00808,	0x1de00808,	0x1df00808,	//W 473 ~ 480
0x1e000808,	0x1e100808,	0x1e200808,	0x1e300808,	0x1e400808,	0x1e500808,	0x1e600808,	0x1e700808,	//W 481 ~ 488
0x1e800808,	0x1e900808,	0x1ea00808,	0x1eb00808,	0x1ec00808,	0x1ed00808,	0x1ee00808,	0x1ef00808,	//W 489 ~ 496
0x1f000808,	0x1f100808,	0x1f200808,	0x1f300808,	0x1f400808,	0x1f500808,	0x1f600808,	0x1f700808,	//W 497 ~ 504
0x1f800808,	0x1f900808,	0x1fa00808,	0x1fb00808,	0x1fc00808,	0x1fd00808,	0x1fe00808,	0x1ff00807,	//W 505 ~ 512
};
void fwif_color_set_OutputGamma_System(void)
{
	unsigned char display;
	unsigned char src_idx;
	unsigned short i=0;
	unsigned char output_Gamma_En;
	unsigned char output_Location;
	unsigned char fmt;
	

	Scaler_Get_Display_info(&display, &src_idx);
	down(&Gamma_Semaphore);

{
	/*0. gamma curve reg control*/
	/*fwif_color_out_gamma_control_front();*/

	/*1. choose encode gamma data*/
	fwif_color_set_gamma_curve_index(2, &tGAMMA_temp_R, &tGAMMA_temp_G, &tGAMMA_temp_B);

	/*2. decode gamma data to gamma curve*/
	fwif_color_gamma_decode(GOut_R, GOut_G, GOut_B, tGAMMA_temp_R, tGAMMA_temp_G, tGAMMA_temp_B);
	//fwif_color_decode_gamma_debug(1); /*debug*/
	
	for(i=1020;i<1025;i++)
		printk("GOut_R[%d]=%d,%d,%d\n",i,GOut_R[i], GOut_G[i], GOut_B[i]);
	fwif_color_set_GainOffsetGamma(GOut_R, GOut_G, GOut_B);
	/*5. gamma curve data protect and debug*/
	fwif_color_out_gamma_curve_data_protect(GOut_R, GOut_G, GOut_B);		/*DO NOT mark this, must check*/

	for(i=1020;i<1025;i++)
		printk("aGOut_R[%d]=%d,%d,%d\n",i,GOut_R[i], GOut_G[i], GOut_B[i]);

	/*6. encode gamma cuve to gamma data*/
	fwif_color_out_gamma_encode(final_OutGAMMA_R, final_OutGAMMA_G, final_OutGAMMA_B, GOut_R, GOut_G, GOut_B);
	//fwif_color_encode_gamma_debug(2);  /*debug*/


	/*7. write gamma table*/
	fwif_color_out_gamma_control_front();
	
	fwif_color_Out_Gamma_Curve_Write(final_OutGAMMA_R, GAMMA_CHANNEL_R);
	fwif_color_Out_Gamma_Curve_Write(final_OutGAMMA_G, GAMMA_CHANNEL_G);
	fwif_color_Out_Gamma_Curve_Write(final_OutGAMMA_B, GAMMA_CHANNEL_B);
	fwif_color_Out_Gamma_Curve_Write(LinearOutputGamma, GAMMA_CHANNEL_W);
	
	fwif_color_out_gamma_control_back();

	output_Gamma_En = 1;
	fmt = 0;
	output_Location = 1;/*After RGBW*/
	drvif_color_set_output_gamma_format(output_Gamma_En, fmt, output_Location, 0);
	
}
	
	up(&Gamma_Semaphore);

	

}


unsigned short outGamma_R[1024],outGamma_G[1024],outGamma_B[1024];
unsigned int GammaTable_R[512],GammaTable_G[512],GammaTable_B[512];

void fwif_color_gamma_encode_TableSize_2(RTK_TableSize_Gamma *pData)
{
	unsigned short i=0;
	unsigned short bit_diff_r=0,bit_diff_g=0,bit_diff_b=0;
	unsigned char j=0;
	unsigned short maxvalueR=4096,maxvalueG=4096,maxvalueB=4096;
	unsigned short outTBL_index=0;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		printk("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	for(i=0;i<256;i++){
		pData->pu16Gamma_r[i]=pData->pu16Gamma_r[i]<<3;
		pData->pu16Gamma_g[i]=pData->pu16Gamma_g[i]<<3;
		pData->pu16Gamma_b[i]=pData->pu16Gamma_b[i]<<3;
	}
	
	maxvalueR=pData->pu16Gamma_r[255];
	maxvalueG=pData->pu16Gamma_g[255];
	maxvalueB=pData->pu16Gamma_b[255];
	
	for(i=255;i>1;i--)
	{
		pData->pu16Gamma_r[i]=pData->pu16Gamma_r[i-1];
		pData->pu16Gamma_g[i]=pData->pu16Gamma_g[i-1];
		pData->pu16Gamma_b[i]=pData->pu16Gamma_b[i-1];
	}

	pData->pu16Gamma_r[1]=(pData->pu16Gamma_r[1])*2/3;
	pData->pu16Gamma_g[1]=(pData->pu16Gamma_g[1])*2/3;
	pData->pu16Gamma_b[1]=(pData->pu16Gamma_b[1])*2/3;

	//for(i=0;i<256;i++)
		//printk("TCL_B[%d] =%d\n", i, pData->pu16Gamma_b[i]);

	//R---
		for(i=0;i<256;i++){
		if(i<255){
			bit_diff_r = ((pData->pu16Gamma_r[(i+1)]-pData->pu16Gamma_r[i])>>2);
				for(j = 0 ; j < 4 ; j++)
					outGamma_R[4*i+j]= pData->pu16Gamma_r[i]+bit_diff_r*j;
		}else{
			bit_diff_r = ((maxvalueR-pData->pu16Gamma_r[i])>>2);
				for(j = 0 ; j < 4 ; j++)
					outGamma_R[4*i+j]= pData->pu16Gamma_r[i]+bit_diff_r*j;
		}
	}

	//G
		for(i=0;i<256;i++){
		if(i<255){
			bit_diff_g = ((pData->pu16Gamma_g[(i+1)]-pData->pu16Gamma_g[i])>>2);
				for(j = 0 ; j < 4 ; j++)
					outGamma_G[4*i+j]= pData->pu16Gamma_g[i]+bit_diff_g*j;
		}else{
			bit_diff_g = ((maxvalueG-pData->pu16Gamma_g[i])>>2);
				for(j = 0 ; j < 4 ; j++)
					outGamma_G[4*i+j]= pData->pu16Gamma_g[i]+bit_diff_g*j;
		}
	}

	//B
	for(i=0;i<256;i++){
		if(i<255){
			bit_diff_b = ((pData->pu16Gamma_b[(i+1)]-pData->pu16Gamma_b[i])>>2);
				for(j = 0 ; j < 4 ; j++)
					outGamma_B[4*i+j]= pData->pu16Gamma_b[i]+bit_diff_b*j;
		}else{
			bit_diff_b = ((maxvalueB-pData->pu16Gamma_b[i])>>2);
				for(j = 0 ; j < 4 ; j++)
					outGamma_B[4*i+j]= pData->pu16Gamma_b[i]+bit_diff_b*j;
		}
	}
	
	//for(i=0;i<1024;i++)
		//printk("outGamma_B[%d] =%d\n", i, outGamma_B[i]);

	outTBL_index=2;
	
	fwif_color_out_gamma_encode(GammaTable_R,GammaTable_G,GammaTable_B,outGamma_R,outGamma_G,outGamma_B);
	
	for(i=0;i<512;i++){
		gVip_Table->tGAMMA[outTBL_index].R[i] = GammaTable_R[i];
		gVip_Table->tGAMMA[outTBL_index].G[i] = GammaTable_G[i];
		gVip_Table->tGAMMA[outTBL_index].B[i] = GammaTable_B[i];
	}



}

