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
//#include <mach/RPCDriver.h>
#include <linux/pageremap.h>
#include <mach/system.h>
#include <linux/hrtimer.h>

#include <tvscalercontrol/vip/vip_reg_def.h>
//#include <rbus/rbusHistogramReg.h>
//#include <rbus/scaler/rbusPpOverlayReg.h>
#include <rbus/ppoverlay_reg.h>

//#include <rbus/rbusODReg.h>
//#include <rbus/rbus_DesignSpec_MISC_LSADCReg.h>
//#include <rbus/rbusColorReg.h>
//#include <rbus/rbusCon_briReg.h>
//#include <rbus/rbusYuv2rgbReg.h>
//#include <rbus/rbusScaleupReg.h>
//#include <rbus/rbusColor_dccReg.h>
//#include <rbus/rbusGammaReg.h>
//#include <rbus/rbusInv_gammaReg.h>
#include <rbus/dm_reg.h>
#include <rbus/hdr_all_top_reg.h>
#include <rbus/vgip_reg.h>
#include <rbus/vodma_reg.h>
//#include "rbus/rbusHDMIReg.h"
#include <rbus/h3ddma_hsd_reg.h>

/*#include </merlin_rbus/hdr/hdr_all_top_reg.h>*/ //Need SW sync,  SW Team not sync from TV001

//#include <rbus/dtg_reg.h>

/*#include <Application/AppClass/SetupClass.h>*/

/*#include "tvscalercontrol/scaler/scalerSuperResolution.h"*/
/*#include <tvscalercontrol/scaler/scalertimer.h>*/
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv001.h>
#include <tvscalercontrol/scaler/scalercolorlib_IPQ.h>
/*#include <tvscalercontrol/scaler/scalerlib.h>*/
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scalerdrv/pipmp.h>
//#include <tvscalercontrol/scalerdrv/scalerdrv.h>

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

#include <tvscalercontrol/hdmirx/hdmifun.h>

#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/panel/panelapi.h>
/*#include <Platform_Lib/Board/pcbMgr.h>*/
/*#include <Platform_Lib/Board/pcb.h>*/

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
#include <scalercommon/scalerDrvCommon.h>
#endif

#include <scaler_vscdev.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <rtd_log/rtd_module_log.h>

#include "scaler_vpqmemcdev.h"
#include <tvscalercontrol/vip/localcontrast.h>

#include "memc_isr/scalerMEMC.h"

#define TAG_NAME "VPQ"


/*******************************************************************************
* Macro
******************************************************************************/
#define GET_USER_INPUT_SRC()					(Scaler_GetUserInputSrc(SLR_MAIN_DISPLAY))

#define _SUCCESS		1
#define _FAIL			0
#define _ENABLE 			1
#define _DISABLE			0
#define _TRUE			1
#define _FALSE			0

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

/*******************************************************************************
* Variable
******************************************************************************/
//static unsigned char m_nDRLevel;
//static unsigned char m_nSRLevel;
//static unsigned char m_nCGLevel;
//static unsigned char m_nFTLevel;

/*******************************************************************************
* Program
******************************************************************************/
//======================add for TCL Project======================
#define Bin_Num_Gamma_TCL 1024
#define Vmax_Gamma_TCL 4095
UINT16 GOut_R_TCL[Bin_Num_Gamma_TCL + 1], GOut_G_TCL[Bin_Num_Gamma_TCL + 1], GOut_B_TCL[Bin_Num_Gamma_TCL + 1];
UINT16 gamma_scale_TCL = (Vmax_Gamma_TCL + 1) / Bin_Num_Gamma_TCL;
unsigned int final_GAMMA_R_TCL[Bin_Num_Gamma_TCL / 2], final_GAMMA_G_TCL[Bin_Num_Gamma_TCL / 2], final_GAMMA_B_TCL[Bin_Num_Gamma_TCL / 2]; // Num of index: Mac2=128, Sirius=512 (jyyang_2013/12/30)
int osd[6][3];

//******************************************************************************/

/*static void rtd_part_outl(unsigned int reg_addr, unsigned int endBit, unsigned int startBit, unsigned int value)
{
	unsigned int X,A,result;
	X=(1<<(endBit-startBit+1))-1;
	A=rtd_inl(reg_addr);
	result = (A & (~(X<<startBit))) | (value<<startBit);
	rtd_outl(reg_addr,result);
}*/


unsigned int Scaler_GetColorTemp_level_type(TV001_COLORTEMP_E * colorTemp)
{
	unsigned char nLevel;
	nLevel=scaler_get_color_temp_level_type(GET_USER_INPUT_SRC());

	if(nLevel == SLR_COLORTEMP_NORMAL )
		*colorTemp=TV001_COLORTEMP_NATURE;
	else if(nLevel == SLR_COLORTEMP_COOL || nLevel == SLR_COLORTEMP_COOLER)
		*colorTemp=TV001_COLORTEMP_COOL;
	else if(nLevel == SLR_COLORTEMP_WARM || nLevel == SLR_COLORTEMP_WARMER)
		*colorTemp=TV001_COLORTEMP_WARM;
	else if(nLevel == SLR_COLORTEMP_USER )
		*colorTemp=TV001_COLORTEMP_USER;
	else
		*colorTemp=TV001_COLORTEMP_NATURE;
	return _SUCCESS;
}
void Scaler_SetColorTemp_level_type(TV001_COLORTEMP_E colorTemp)
{
	if(colorTemp == TV001_COLORTEMP_NATURE )
		Scaler_SetColorTemp(SLR_COLORTEMP_NORMAL);
	else if(colorTemp == TV001_COLORTEMP_COOL )
		Scaler_SetColorTemp(SLR_COLORTEMP_COOL);
	else if(colorTemp == TV001_COLORTEMP_WARM )
		Scaler_SetColorTemp(SLR_COLORTEMP_WARM);
	else if(colorTemp == TV001_COLORTEMP_USER )
		Scaler_SetColorTemp(SLR_COLORTEMP_USER);
	else
		Scaler_SetColorTemp(SLR_COLORTEMP_NORMAL);
}
unsigned int Scaler_GetColorTempData(TV001_COLOR_TEMP_DATA_S *colorTemp)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	colorTemp->redGain = Scaler_get_color_temp_r_type(src_idx);
	colorTemp->greenGain = Scaler_get_color_temp_g_type(src_idx);
	colorTemp->blueGain = Scaler_get_color_temp_b_type(src_idx);
	colorTemp->redOffset = Scaler_get_color_temp_r_offset(src_idx);
	colorTemp->greenOffset = Scaler_get_color_temp_g_offset(src_idx);
	colorTemp->blueOffset = Scaler_get_color_temp_b_offset(src_idx);
	return _SUCCESS;
}
void Scaler_SetColorTempData(TV001_COLOR_TEMP_DATA_S *colorTemp)
{
	// use color temp after inv-out-gamma, from merlin7
	unsigned char WaitSync = 1;
	extern void scalerVIP_RADCR_save_osd(TV001_COLOR_TEMP_DATA_S *colorTemp);
#ifndef BUILD_QUICK_SHOW
	scalerVIP_RADCR_save_osd(colorTemp);
#endif
	if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE) || Check_AP_Set_Enable_ForceBG(SLR_MAIN_DISPLAY))
		WaitSync = 0;

	//fwif_color_set_rtk_colortemp_inv_output_gamma(!Scaler_Get_CinemaMode_PQ(), WaitSync);
	fwif_color_set_iog_color_temp(!Scaler_Get_CinemaMode_PQ(), colorTemp->redGain<<2, colorTemp->greenGain<<2, colorTemp->blueGain<<2,
	(colorTemp->redOffset-512)*4, (colorTemp->greenOffset-512)*4, (colorTemp->blueOffset-512)*4, WaitSync);

/* //old rgb/offset before gamma
	if(Scaler_VOFromJPEG(Scaler_Get_CurVoInfo_plane()) == 1)
	{
		fwif_color_setrgbcontrast_By_Table(512, 512, 512, 0);
		fwif_color_setrgbbrightness_By_Table(512, 512,512);
	}
	else
	{
		fwif_color_setrgbcontrast_By_Table(colorTemp->redGain, colorTemp->greenGain, colorTemp->blueGain, 0);
		fwif_color_setrgbbrightness_By_Table(colorTemp->redOffset, colorTemp->greenOffset,colorTemp->blueOffset);
	}
*/

/* //use color temp after gamma
	unsigned char WaitSync = 1;
	if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE) || Check_AP_Set_Enable_ForceBG(SLR_MAIN_DISPLAY))
		WaitSync = 0;

	if(Scaler_VOFromJPEG(Scaler_Get_CurVoInfo_plane()) == 1)
		fwif_color_set_color_temp(!Scaler_Get_CinemaMode_PQ(), 2048, 2048, 2048, 0, 0, 0, WaitSync);
	else
		fwif_color_set_color_temp(!Scaler_Get_CinemaMode_PQ(), colorTemp->redGain<<2, colorTemp->greenGain<<2, colorTemp->blueGain<<2,
		(colorTemp->redOffset-512)*4, (colorTemp->greenOffset-512)*4, (colorTemp->blueOffset-512)*4, WaitSync);
*/
}

void Scaler_SetBlackPatternOutput(unsigned char isBlackPattern)
{
	fwif_color_set_WB_Pattern_IRE(isBlackPattern, 0);
}
void Scaler_SetWhitePatternOutput(unsigned char isWhitePattern)
{
	fwif_color_set_WB_Pattern_IRE(isWhitePattern, 100*2);
}
unsigned int Scaler_GetPQModule(TV001_PQ_MODULE_E pqModule,unsigned char * onOff)
{
	*onOff = TRUE;
	return _SUCCESS;
}
void Scaler_SetPQModule(TV001_PQ_MODULE_E pqModule,unsigned char onOff)
{
#ifdef ANDTV_MAC5_COMPILE_FIX
	switch(pqModule) {
#ifdef ANDTV_PQ_COMPILE_FIX
		case TV001_PQ_MODULE_FMD:
		drvif_color_set_PQ_Module(PQ_ByPass_Fmd,onOff);
		break;
#endif
		case TV001_PQ_MODULE_NR:
		drvif_color_set_PQ_Module(PQ_ByPass_RTNR_Y,onOff);
		drvif_color_set_PQ_Module(PQ_ByPass_SNR_Y,onOff);
		break;

		case TV001_PQ_MODULE_DB:
		drvif_color_set_PQ_Module(PQ_ByPass_MPEG_NR,onOff);
		break;

		case TV001_PQ_MODULE_DR:
		drvif_color_set_PQ_Module(PQ_ByPass_MosquiutoNR,onOff);
		break;

		case TV001_PQ_MODULE_HSHARPNESS:
		drvif_color_set_PQ_Module(PQ_ByPass_SHP_DLTI,onOff);
		break;

		case TV001_PQ_MODULE_SHARPNESS:
		drvif_color_set_PQ_Module(PQ_ByPass_SHP,onOff);
		drvif_color_set_PQ_Module(PQ_ByPass_SHP_DLTI,onOff);
		drvif_color_set_PQ_Module(PQ_ByPass_MB_Peaking,onOff);
		break;

#ifdef ANDTV_PQ_COMPILE_FIX
		case TV001_PQ_MODULE_CCCL:
		drvif_color_set_PQ_Module(PQ_ByPass_CCCL,onOff);
		break;
		case TV001_PQ_MODULE_COLOR_CORING:
		drvif_color_set_PQ_Module(PQ_ByPass_COLOR_CORING,onOff);
		break;
#endif
		case TV001_PQ_MODULE_BLUE_STRETCH:
		//removed
		break;

		case TV001_PQ_MODULE_GAMMA:
		drvif_color_set_PQ_Module(PQ_ByPass_Gamma,onOff);
		break;
#ifdef ANDTV_PQ_COMPILE_FIX
		case TV001_PQ_MODULE_DBC:
		drvif_color_set_PQ_Module(PQ_ByPass_DBC,onOff);
		break;
#endif
		case TV001_PQ_MODULE_DCI:
		drvif_color_set_PQ_Module(PQ_ByPass_DCC,onOff);
		break;

		case TV001_PQ_MODULE_COLOR:
		drvif_color_set_PQ_Module(PQ_ByPass_ICM,onOff);
		break;

		case TV001_PQ_MODULE_ES:
		//drvif_color_set_PQ_Module(PQ_ByPass_RTNR_Y,onOff);
		break;

		case TV001_PQ_MODULE_SR:
		drvif_color_set_PQ_Module(PQ_ByPass_MB_Peaking,onOff);
		break;

		case TV001_PQ_MODULE_FRC:
		HAL_VPQ_MEMC_MotionCompOnOff(onOff);
		break;

		case TV001_PQ_MODULE_WCG:
		drvif_color_set_PQ_Module(PQ_ByPass_ColorTemp,onOff);
		break;

		case TV001_PQ_MODULE_ALL:
		drvif_color_set_PQ_Module(PQ_ByPass_RTNR_Y,onOff);
		drvif_color_set_PQ_Module(PQ_ByPass_SNR_Y,onOff);

		drvif_color_set_PQ_Module(PQ_ByPass_SHP,onOff);
		drvif_color_set_PQ_Module(PQ_ByPass_SHP_DLTI,onOff);
		drvif_color_set_PQ_Module(PQ_ByPass_MB_Peaking,onOff);

		drvif_color_set_PQ_Module(PQ_ByPass_Gamma,onOff);
		drvif_color_set_PQ_Module(PQ_ByPass_DCC,onOff);
		drvif_color_set_PQ_Module(PQ_ByPass_ICM,onOff);
		drvif_color_set_PQ_Module(PQ_ByPass_ColorTemp,onOff);
		HAL_VPQ_MEMC_MotionCompOnOff(onOff);
		break;
	}
#endif
}
unsigned int Scaler_GetSDR2HDR(unsigned char * onOff)
{
#ifdef ANDTV_PQ_COMPILE_FIX
	_system_setting_info *VIP_system_info_structure_table;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	*onOff = VIP_system_info_structure_table->SDR2HDR_flag;
#endif
	return _SUCCESS;
}
void Scaler_SetSDR2HDR(unsigned char onOff)
{
	//Scaler_LGE_ONEKEY_SDR2HDR_Enable(onOff);
	drvif_color_set_LC_Enable(onOff);
}
unsigned int Scaler_GetHdr10Enable(unsigned char * bEnable)
{
	//_system_setting_info *VIP_system_info_structure_table;
	//VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table;
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(VIP_RPC_system_info_structure_table == NULL){
		printk("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return _FAIL;
	}

	*bEnable = VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En];
	return _SUCCESS;
}
void Scaler_SetHdr10Enable(unsigned char bEnable)
{
	Scaler_LGE_ONEKEY_HDR10_Enable(bEnable);
}


unsigned int Scaler_GetSrcHdrInfo(unsigned int * pGammaType)
{
	*pGammaType = TRUE;
	return _SUCCESS;
}
unsigned int Scaler_GetHdrType(TV001_HDR_TYPE_E * pHdrType)
{
#ifdef ANDTV_PQ_COMPILE_FIX
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table;
	VIP_RPC_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(VIP_RPC_system_info_structure_table == NULL){
		printk("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return _FAIL;
	}

	if(VIP_RPC_system_info_structure_table->SDR2HDR_flag)
		*pHdrType = TV001_HDR_TYPE_SDR_TO_HDR;
	else if(VIP_RPC_system_info_structure_table->DolbyHDR_flag)
		*pHdrType = TV001_HDR_TYPE_DOLBY_HDR;
	else if(VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En])
		*pHdrType = TV001_HDR_TYPE_HDR10;
	else
		*pHdrType = TV001_HDR_TYPE_SDR;
#endif
	return _SUCCESS;
}
unsigned int Scaler_GetDOLBYHDREnable(unsigned char * bEnable)
{
	_system_setting_info *VIP_system_info_structure_table;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	*bEnable = VIP_system_info_structure_table->DolbyHDR_flag;
	return _SUCCESS;
}
void Scaler_SetDOLBYHDREnable(unsigned char bEnable)
{
	unsigned char en[HDR_ITEM_MAX] = {0};

	en[HDR_EN] = bEnable;
	Scaler_set_HDR10_Enable(en);
}


void Scaler_SetDemoMode(TV001_DEMO_MODE_E demoMode,unsigned char onOff)
{
	_RPC_system_setting_info *VIP_system_RPC_info_structure_table;
	VIP_system_RPC_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(VIP_system_RPC_info_structure_table == NULL){
		printk("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}


	if(!onOff){
			if(VIP_system_RPC_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En])
				Scaler_LGE_ONEKEY_HDR10_Enable(onOff);

		Scaler_SetMagicPicture(SLR_MAGIC_OFF);
		return;
	}
	switch(demoMode) {
		case TV001_DEMO_DBDR :
		break;
		case TV001_DEMO_NR:
		break;
		case TV001_DEMO_SHARPNESS:
			Scaler_SetMagicPicture(SLR_MAGIC_STILLDEMO_INVERSE);
		break;
		case TV001_DEMO_DCI:
		break;
		case TV001_DEMO_WCG:
			//Scaler_SetMagicPicture(SLR_MAGIC_FULLGAMUT);
		break;
		case TV001_DEMO_MEMC:
			//Scaler_SetMagicPicture(SLR_MAGIC_MEMC_STILLDEMO_INVERSE);
		break;
		case TV001_DEMO_COLOR:
		break;
		case TV001_DEMO_SR:
			Scaler_SetMagicPicture(SLR_MAGIC_STILLDEMO_INVERSE);
		break;
		case TV001_DEMO_ALL:
		break;
		case TV001_DEMO_HDR:
			Scaler_SetMagicPicture(SLR_MAGIC_STILLDEMO_INVERSE);
		break;
		case TV001_DEMO_SDR_TO_HDR:
			Scaler_SetMagicPicture(SLR_MAGIC_STILLDEMO_INVERSE);
			Scaler_LGE_ONEKEY_SDR2HDR_Enable(TRUE);
		break;
		default:
			break;
	}


}



unsigned int Scaler_GetMemcEnable(unsigned char * bEnable)
{
	unsigned char  nMode = 0;
	// FixMe, 20190925
	//nMode = (unsigned char)MEMC_LibGetMEMCMode();
	if(nMode == 3)
		*bEnable = FALSE;
	else
		*bEnable = TRUE;
	return _SUCCESS;
}
unsigned int Scaler_GetMemcRange(TV001_MEMC_RANGE_S *range)
{
	range->min= 0;
	range->max= 128;
	return _SUCCESS;
}
long Scaler_SetMemcLevel(TV001_LEVEL_E level)
{
	//HAL_VPQ_MEMC_SetMotionComp(UINT8 blurLevel, UINT8 judderLevel, VPQ_MEMC_TYPE_T motion)
	//blurLevel  	:	0~10
	//judderLevel	:	0~10
	int ret = 0;

	printk(KERN_NOTICE "func:%s line:%d level = %d \n",__FUNCTION__,__LINE__,level);

	if(level == TV001_LEVEL_OFF)
		ret = HAL_VPQ_MEMC_SetMotionComp(0,0,VPQ_MEMC_TYPE_OFF);
	else if(level == TV001_LEVEL_LOW)
		ret = HAL_VPQ_MEMC_SetMotionComp(7,3,VPQ_MEMC_TYPE_USER);
	else if(level == TV001_LEVEL_MID)
		ret = HAL_VPQ_MEMC_SetMotionComp(10,7,VPQ_MEMC_TYPE_USER);
	else if(level == TV001_LEVEL_HIGH)
		ret = HAL_VPQ_MEMC_SetMotionComp(10,10,VPQ_MEMC_TYPE_USER);
	else if(level == TV001_LEVEL_AUTO)
		ret = HAL_VPQ_MEMC_SetMotionComp(128,128,VPQ_MEMC_TYPE_HIGH);
	else
		ret = HAL_VPQ_MEMC_SetMotionComp(128,128,VPQ_MEMC_TYPE_HIGH);

	return ret;
}

void fwif_color_gamma_encode_TableSize(RTK_TableSize_Gamma *pData)
{
	unsigned short i;
	unsigned short gamma_temp[4];
	unsigned char g_scaler = 4;		// TCL input gamma is 10-bits and 256-scaler      1024 / 256 = 4
	unsigned char j;
	unsigned short maxvalueR=1024,maxvalueG=1024,maxvalueB=1024;
	
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		printk("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
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
	
	for(i = 0 ; i < 256 ; i++)
	{
		//printk("-lhh001- TCL_g[%d] = %x    %x    %x\n", i, pData->pu16Gamma_r[i], pData->pu16Gamma_g[i], pData->pu16Gamma_b[i]);
		if(i < 255)
		{
			for(j = 0 ; j < 4 ; j++)
			{
				gamma_temp[j] = (pData->pu16Gamma_r[i] + (pData->pu16Gamma_r[i + 1] - pData->pu16Gamma_r[i]) * j / g_scaler) << 2;
			}
			gVip_Table->tGAMMA[0].R[i * 2] = ((gamma_temp[0] << 16) | ((gamma_temp[1] - gamma_temp[0]) << 8) | (0x04));
			gVip_Table->tGAMMA[0].R[i * 2 + 1] = ((gamma_temp[2] << 16) | ((gamma_temp[3] - gamma_temp[2]) << 8) | (0x04));

			for(j = 0 ; j < 4 ; j++)
			{
				gamma_temp[j] = (pData->pu16Gamma_g[i] + (pData->pu16Gamma_g[i + 1] - pData->pu16Gamma_g[i]) * j / g_scaler) << 2;
			}
			gVip_Table->tGAMMA[0].G[i * 2] = ((gamma_temp[0] << 16) | ((gamma_temp[1] - gamma_temp[0]) << 8) | (0x04));
			gVip_Table->tGAMMA[0].G[i * 2 + 1] = ((gamma_temp[2] << 16) | ((gamma_temp[3] - gamma_temp[2]) << 8) | (0x04));

			for(j = 0 ; j < 4 ; j++)
			{
				gamma_temp[j] = (pData->pu16Gamma_b[i] + (pData->pu16Gamma_b[i + 1] - pData->pu16Gamma_b[i]) * j / g_scaler) << 2;
			}
			gVip_Table->tGAMMA[0].B[i * 2] = ((gamma_temp[0] << 16) | ((gamma_temp[1] - gamma_temp[0]) << 8) | (0x04));
			gVip_Table->tGAMMA[0].B[i * 2 + 1] = ((gamma_temp[2] << 16) | ((gamma_temp[3] - gamma_temp[2]) << 8) | (0x04));
		}
		else
		{
			for(j = 0 ; j < 4 ; j++)
			{
				gamma_temp[j] = (pData->pu16Gamma_r[i] + (maxvalueR/*1024*/ - pData->pu16Gamma_r[i]) * j / g_scaler) << 2;
			}
			gVip_Table->tGAMMA[0].R[i * 2] = ((gamma_temp[0] << 16) | ((gamma_temp[1] - gamma_temp[0]) << 8) | (0x04));
			gVip_Table->tGAMMA[0].R[i * 2 + 1] = ((gamma_temp[2] << 16) | ((gamma_temp[3] - gamma_temp[2]) << 8) | (0x04));

			for(j = 0 ; j < 4 ; j++)
			{
				gamma_temp[j] = (pData->pu16Gamma_g[i] + (maxvalueG - pData->pu16Gamma_g[i]) * j / g_scaler) << 2;
			}
			gVip_Table->tGAMMA[0].G[i * 2] = ((gamma_temp[0] << 16) | ((gamma_temp[1] - gamma_temp[0]) << 8) | (0x04));
			gVip_Table->tGAMMA[0].G[i * 2 + 1] = ((gamma_temp[2] << 16) | ((gamma_temp[3] - gamma_temp[2]) << 8) | (0x04));

			for(j = 0 ; j < 4 ; j++)
			{
				gamma_temp[j] = (pData->pu16Gamma_b[i] + (maxvalueB - pData->pu16Gamma_b[i]) * j / g_scaler) << 2;
			}
			gVip_Table->tGAMMA[0].B[i * 2] = ((gamma_temp[0] << 16) | ((gamma_temp[1] - gamma_temp[0]) << 8) | (0x04));
			gVip_Table->tGAMMA[0].B[i * 2 + 1] = ((gamma_temp[2] << 16) | ((gamma_temp[3] - gamma_temp[2]) << 8) | (0x04));
		}


	}

	//for(i=0 ; i<512 ; i++)
		//printk("-lhh002- VIP_g[%d] = %x    %x    %x \n", i, gVip_Table->tGAMMA[0].R[i], gVip_Table->tGAMMA[0].G[i], gVip_Table->tGAMMA[0].B[i]  );


}

void Scaler_Set_ColorSpace_D_3dlutTBL(RTK_VPQ_COLORSPACE_INDEX *ColoSpace_T)
{
	extern struct semaphore ColorSpace_Semaphore;
	short OSD_weight[6][3]={
		{128,128,128},//R
		{512,256,256},//G
		{512,256,256},//B
		{1024,1024,1024},//Y
		{1024,1024,1024},//C
		{256,256,256},//M
	};
	
	#if 1 //for debug
	printk("yuan2,R:R=%d,G=%d,B=%d\n",ColoSpace_T->CS_CUSTOM_RED.red,ColoSpace_T->CS_CUSTOM_RED.green,ColoSpace_T->CS_CUSTOM_RED.blue);
	printk("yuan2,G:R=%d,G=%d,B=%d\n",ColoSpace_T->CS_CUSTOM_GREEN.red,ColoSpace_T->CS_CUSTOM_GREEN.green,ColoSpace_T->CS_CUSTOM_GREEN.blue);
	printk("yuan2,B:R=%d,G=%d,B=%d\n",ColoSpace_T->CS_CUSTOM_BLUE.red,ColoSpace_T->CS_CUSTOM_BLUE.green,ColoSpace_T->CS_CUSTOM_BLUE.blue);
	printk("yuan2,Y:R=%d,G=%d,B=%d\n",ColoSpace_T->CS_CUSTOM_YELLOW.red,ColoSpace_T->CS_CUSTOM_YELLOW.green,ColoSpace_T->CS_CUSTOM_YELLOW.blue);
	printk("yuan2,C:R=%d,G=%d,B=%d\n",ColoSpace_T->CS_CUSTOM_CYAN.red,ColoSpace_T->CS_CUSTOM_CYAN.green,ColoSpace_T->CS_CUSTOM_CYAN.blue);
	printk("yuan2,M:R=%d,G=%d,B=%d\n",ColoSpace_T->CS_CUSTOM_MAGENTA.red,ColoSpace_T->CS_CUSTOM_MAGENTA.green,ColoSpace_T->CS_CUSTOM_MAGENTA.blue);
	#endif	
	
	
	down(&ColorSpace_Semaphore);
	//----R
	osd[0][0]= OSD_weight[0][0]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_RED.red);
	osd[0][1]= OSD_weight[0][1]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_RED.green);//fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_RED.green);
	osd[0][2]= OSD_weight[0][2]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_RED.blue);//fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_RED.blue);

	//----G 
	osd[1][0]= OSD_weight[1][0]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_GREEN.red);//fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_GREEN.red);
	osd[1][1]= OSD_weight[1][1]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_GREEN.green);
	osd[1][2]= OSD_weight[1][2]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_GREEN.blue);//fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_GREEN.blue);

	//----B
	osd[2][0]= OSD_weight[2][0]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_BLUE.red);//fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_BLUE.red);
	osd[2][1]= OSD_weight[2][1]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_BLUE.green);//fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_BLUE.green);
	osd[2][2]= OSD_weight[2][0]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_BLUE.blue);

	//----Y
	osd[3][0]= OSD_weight[3][0]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_YELLOW.red);
	osd[3][1]= OSD_weight[3][1]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_YELLOW.green);
	osd[3][2]= OSD_weight[3][2]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_YELLOW.blue);//fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_YELLOW.blue);

	//----C
	osd[4][0]= OSD_weight[4][0]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_CYAN.red);//fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_CYAN.red);
	osd[4][1]= OSD_weight[4][1]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_CYAN.green);
	osd[4][2]= OSD_weight[4][2]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_CYAN.blue);

	//----M
	osd[5][0]= OSD_weight[5][0]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_MAGENTA.red);
	osd[5][1]= OSD_weight[5][1]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_MAGENTA.green);//fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_MAGENTA.green);
	osd[5][2]= OSD_weight[5][2]*fwif_OSDMapToLUTValue(ColoSpace_T->CS_CUSTOM_MAGENTA.blue);

	#if 0 //for debug
	printk("osd,R:R=%d,G=%d,B=%d\n",osd[0][0],osd[0][1],osd[0][2]);
	printk("osd,G:R=%d,G=%d,B=%d\n",osd[1][0],osd[1][1],osd[1][2]);
	printk("osd,B:R=%d,G=%d,B=%d\n",osd[2][0],osd[2][1],osd[2][2]);
	printk("osd,Y:R=%d,G=%d,B=%d\n",osd[3][0],osd[3][1],osd[3][2]);
	printk("osd,C:R=%d,G=%d,B=%d\n",osd[4][0],osd[4][1],osd[4][2]);
	printk("osd,M:R=%d,G=%d,B=%d\n",osd[5][0],osd[5][1],osd[5][2]);
	#endif	
	
	fwif_color_cal_D_3DLUT_Tbl(VIP_8VERTEX_R);
	fwif_color_cal_D_3DLUT_Tbl(VIP_8VERTEX_G);
	fwif_color_cal_D_3DLUT_Tbl(VIP_8VERTEX_B);
	up(&ColorSpace_Semaphore);
}

char fwif_OSDMapToLUTValue(unsigned char value)
{
	char temp = 0;

	temp=value-50;

	return (char)temp;
}

void fwif_color_cal_D_3DLUT_Tbl(UINT8 axis)
{
	extern unsigned char D_3DLUT_R[VIP_D_3D_LUT_SIZE];
	extern unsigned char D_3DLUT_G[VIP_D_3D_LUT_SIZE];
	extern unsigned char D_3DLUT_B[VIP_D_3D_LUT_SIZE];
	extern unsigned char D_3DLUT_Y[VIP_D_3D_LUT_SIZE];
	extern unsigned char D_3DLUT_C[VIP_D_3D_LUT_SIZE];
	extern unsigned char D_3DLUT_M[VIP_D_3D_LUT_SIZE];
	//extern int D_3DLUT[VIP_D_3D_LUT_SIZE][VIP_8VERTEX_RGB_MAX];
	extern int D_3DLUT_Reseult[VIP_D_3D_LUT_SIZE][VIP_8VERTEX_RGB_MAX];
	extern unsigned int D_3DLUT_999_Reseult[VIP_D_3D_LUT_SIZE][VIP_8VERTEX_RGB_MAX];
	extern int osd[6][3];
	short i=0;
		
	//cal Red------------------------------ 
	for(i=0;i<VIP_D_3D_LUT_SIZE;i++){
		D_3DLUT_Reseult[i][axis]=(D_3DLUT_R[i]*osd[0][axis])/9+D_3DLUT_999_Reseult[i][axis];
	
	//prtection-------------------------
	if((D_3DLUT_Reseult[i][axis]) <0)
			D_3DLUT_Reseult[i][axis]=0;
	
	if((D_3DLUT_Reseult[i][axis]) > 65535)
			D_3DLUT_Reseult[i][axis]=65535;
	}
	
	//for(i=0;i<30;i++){
	//	  printk("D_3DLUT_Reseult[%d][axis]=%d\n",i,D_3DLUT_Reseult[i][axis]);
	//}
	
	//cal Green------------------------------	
	for(i=0;i<VIP_D_3D_LUT_SIZE;i++){
		D_3DLUT_Reseult[i][axis]=(D_3DLUT_G[i]*osd[1][axis])/9+D_3DLUT_Reseult[i][axis];
	
	//prtection-------------------------
	if((D_3DLUT_Reseult[i][axis]) <0)
			D_3DLUT_Reseult[i][axis]=0;
	
	if((D_3DLUT_Reseult[i][axis]) > 65535)
			D_3DLUT_Reseult[i][axis]=65535;
	}
	
	//cal Blue------------------------------	
	for(i=0;i<VIP_D_3D_LUT_SIZE;i++){
		D_3DLUT_Reseult[i][axis]=(D_3DLUT_B[i]*osd[2][axis])/9+D_3DLUT_Reseult[i][axis];
	
	//prtection-------------------------
	if((D_3DLUT_Reseult[i][axis]) <0)
			D_3DLUT_Reseult[i][axis]=0;
	
	if((D_3DLUT_Reseult[i][axis]) > 65535)
			D_3DLUT_Reseult[i][axis]=65535;
	}
	//cal Yellow------------------------------	
	for(i=0;i<VIP_D_3D_LUT_SIZE;i++){
		D_3DLUT_Reseult[i][axis]=(D_3DLUT_Y[i]*osd[3][axis])/9+D_3DLUT_Reseult[i][axis];
	
	//prtection-------------------------
	if((D_3DLUT_Reseult[i][axis]) <0)
			D_3DLUT_Reseult[i][axis]=0;
	
	if((D_3DLUT_Reseult[i][axis]) > 65535)
			D_3DLUT_Reseult[i][axis]=65535;
	}
	
	//cal C------------------------------	
	for(i=0;i<VIP_D_3D_LUT_SIZE;i++){
		D_3DLUT_Reseult[i][axis]=(D_3DLUT_C[i]*osd[4][axis])/9+D_3DLUT_Reseult[i][axis];
	
	//prtection-------------------------
	if((D_3DLUT_Reseult[i][axis]) <0)
			D_3DLUT_Reseult[i][axis]=0;
	
	if((D_3DLUT_Reseult[i][axis]) > 65535)
			D_3DLUT_Reseult[i][axis]=65535;
	}

	//cal M------------------------------	
	for(i=0;i<VIP_D_3D_LUT_SIZE;i++){
		D_3DLUT_Reseult[i][axis]=(D_3DLUT_M[i]*osd[5][axis])/9+D_3DLUT_Reseult[i][axis];
	
	//prtection-------------------------
	if((D_3DLUT_Reseult[i][axis]) <0)
			D_3DLUT_Reseult[i][axis]=0;
	
	if((D_3DLUT_Reseult[i][axis]) > 65535)
			D_3DLUT_Reseult[i][axis]=65535;
	}

	fwif_color_WaitFor_DEN_STOP_UZUDTG();
	//fwif_color_set_D_3dLUT(255, (unsigned int *) D_3DLUT_Reseult);
	fwif_color_D_LUT_By_DMA(0, 255, (unsigned int *)D_3DLUT_Reseult);	
	
}


void Scaler_Set_Partten4AutoGamma_mute(unsigned char mute_flag, unsigned char display_flag,SCALER_DISPLAY_DATA* sdp_info)
{
	drvif_color_set_Partten4AutoGamma_mute(mute_flag,display_flag,(Drvif_SCALER_DISPLAY_DATA*)sdp_info);
}

void Scaler_Get_ScreenPixel_info(SCREEN_PIXEL_INFO* sp_info)
{
	UINT16 i, j, k;
	UINT16 width, hight;
	UINT16 h_read_num, v_read_num;
	UINT32 total_pixel;
	yuv2rgb_accessdata_ctrl_RBUS yuv2rgb_accessdata_ctrl_reg;
	yuv2rgb_accessdata_posctrl_RBUS yuv2rgb_accessdata_posctrl_reg;
	yuv2rgb_readdata_data_y1_RBUS yuv2rgb_readdata_data_y1_reg;
	yuv2rgb_readdata_data_y2_RBUS yuv2rgb_readdata_data_y2_reg;
	yuv2rgb_readdata_data_c1_RBUS yuv2rgb_readdata_data_c1_reg;
	yuv2rgb_readdata_data_c2_RBUS yuv2rgb_readdata_data_c2_reg;
	yuv2rgb_readdata_data_c3_RBUS yuv2rgb_readdata_data_c3_reg;
	yuv2rgb_readdata_data_c4_RBUS yuv2rgb_readdata_data_c4_reg;

    printk("scaler_get_screen_pixel_info !!!!h_sta(%d)===h_end(%d)!\n",sp_info->h_sta,sp_info->h_end);
	printk("scaler_get_screen_pixel_info !!!!v_sta(%d)===v_end(%d)!\n",sp_info->v_sta,sp_info->v_end);

	sp_info->r_sum = 0;
	sp_info->g_sum = 0;
	sp_info->b_sum = 0;

	width = sp_info->h_end - sp_info->h_sta + 1;
	hight = sp_info->v_end - sp_info->v_sta + 1;
	total_pixel = width * hight;
    printk("The capture window too large, get screen pixel  total_pixel(%d)!!!\n",total_pixel);
	//if(total_pixel > 200)
	//{
	//	printk("The capture window too large, get screen pixel fail !!!\n");
	//	return;
	//}


	if(width % 2 != 0)
		h_read_num = width / 2 + 1;
	else
		h_read_num = width / 2;

	if(hight % 2 != 0)
		v_read_num = hight / 2 + 1;
	else
		v_read_num = hight /2;

	for(j = 0 ; j < v_read_num ; j++)
	{
		for(i = 0 ; i < h_read_num ; i++)
		{
			yuv2rgb_accessdata_posctrl_reg.startx = sp_info->h_sta + i * 2;
			yuv2rgb_accessdata_posctrl_reg.starty = sp_info->v_sta + j * 2;
			IoReg_Write32(YUV2RGB_AccessData_PosCtrl_reg, yuv2rgb_accessdata_posctrl_reg.regValue);

			yuv2rgb_accessdata_ctrl_reg.read_en = 1;
			IoReg_Write32(YUV2RGB_AccessData_CTRL_reg, yuv2rgb_accessdata_ctrl_reg.regValue);

			msleep(18);
			k = 1000;
			while(k != 0)
			{
				--k;
				yuv2rgb_accessdata_ctrl_reg.regValue = IoReg_Read32(YUV2RGB_AccessData_CTRL_reg);
				if(yuv2rgb_accessdata_ctrl_reg.read_en == 0)
					break;
			}

			//for debug
			if(k == 0)
			{
				printk("\n YUV2RGB Data Access time out.  \n");
				msleep(10);
				yuv2rgb_accessdata_ctrl_reg.regValue = IoReg_Read32(YUV2RGB_AccessData_CTRL_reg);
				if(yuv2rgb_accessdata_ctrl_reg.read_en == 0)
					printk("\n YUV2RGB Data Access read OK.  \n");
				else
					printk("\n YUV2RGB Data Access read NG.  \n");
			}

			yuv2rgb_readdata_data_y1_reg.regValue = IoReg_Read32(YUV2RGB_ReadData_DATA_Y1_reg);
			yuv2rgb_readdata_data_y2_reg.regValue = IoReg_Read32(YUV2RGB_ReadData_DATA_Y2_reg);
			yuv2rgb_readdata_data_c1_reg.regValue = IoReg_Read32(YUV2RGB_ReadData_DATA_C1_reg);
			yuv2rgb_readdata_data_c2_reg.regValue = IoReg_Read32(YUV2RGB_ReadData_DATA_C2_reg);
			yuv2rgb_readdata_data_c3_reg.regValue = IoReg_Read32(YUV2RGB_ReadData_DATA_C3_reg);
			yuv2rgb_readdata_data_c4_reg.regValue = IoReg_Read32(YUV2RGB_ReadData_DATA_C4_reg);

			sp_info->r_sum += yuv2rgb_readdata_data_y1_reg.y00 >> 6;
			sp_info->g_sum += yuv2rgb_readdata_data_c1_reg.u00 >> 6;
			sp_info->b_sum += yuv2rgb_readdata_data_c1_reg.v00 >> 6;
			if((--total_pixel) == 0)
			{
				//printk("\n--lhh00-- total  R=%x   G=%x   B=%x  \n", sp_info->r_sum, sp_info->g_sum, sp_info->b_sum);
				return;
			}

			sp_info->r_sum += yuv2rgb_readdata_data_y1_reg.y01 >> 6;
			sp_info->g_sum += yuv2rgb_readdata_data_c2_reg.u01 >> 6;
			sp_info->b_sum += yuv2rgb_readdata_data_c2_reg.v01 >> 6;
			if((--total_pixel) == 0)
			{
				//printk("\n--lhh01-- total  R=%x   G=%x   B=%x  \n", sp_info->r_sum, sp_info->g_sum, sp_info->b_sum);
				return;
			}

			sp_info->r_sum += yuv2rgb_readdata_data_y2_reg.y10 >> 6;
			sp_info->g_sum += yuv2rgb_readdata_data_c3_reg.u10 >> 6;
			sp_info->b_sum += yuv2rgb_readdata_data_c3_reg.v10 >> 6;
			if((--total_pixel) == 0)
			{
				//printk("\n--lhh10-- total  R=%x   G=%x   B=%x  \n",sp_info->r_sum, sp_info->g_sum, sp_info->b_sum);
				return;
			}

			sp_info->r_sum += yuv2rgb_readdata_data_y2_reg.y11 >> 6;
			sp_info->g_sum += yuv2rgb_readdata_data_c4_reg.u11 >> 6;
			sp_info->b_sum += yuv2rgb_readdata_data_c4_reg.v11 >> 6;
			if((--total_pixel) == 0)
			{
				//printk("\n--lhh11-- total  R=%x   G=%x   B=%x  \n", sp_info->r_sum, sp_info->g_sum, sp_info->b_sum);
				return;
			}

		}
	}

}
