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
#include <rtk_kdriver/io.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
//#include <rbus/rbusHistogramReg.h>
//#include <rbus/scaler/rbusPpOverlayReg.h>
#include <rbus/ppoverlay_reg.h>

#include <linux/kthread.h>  /* for threads */

//#include <rbus/rbusODReg.h>

//TODO, Merlin3 tv010 sync, need add
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
//#include <rbus/h3ddma_hsd_reg.h>
#include <rbus/sfg_reg.h>

//#include <rbus/dtg_reg.h>

/*#include <Application/AppClass/SetupClass.h>*/

/*#include "tvscalercontrol/scaler/scalerSuperResolution.h"*/
/*#include <tvscalercontrol/scaler/scalertimer.h>*/
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv010.h>
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

#include <scaler/scalerDrvCommon.h>

#include <scaler_vscdev.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <rtd_log/rtd_module_log.h>

#include <scaler_vpqmemcdev.h>
#include <tvscalercontrol/vip/localcontrast.h>
#include <tvscalercontrol/vip/localdimming.h>
#ifdef CONFIG_HW_SUPPORT_MEMC
#include "memc_isr/scalerMEMC.h"
#endif
#include <scaler_vpqdev.h>




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
static unsigned char m_nDRLevel;
static unsigned char m_nSRLevel;
static unsigned char m_nCGLevel;
static unsigned char m_nFTLevel;
static unsigned char m_icmIndex;
/*******************************************************************************
* Program
******************************************************************************/
#if 0
static void rtd_part_outl(unsigned int reg_addr, unsigned int endBit, unsigned int startBit, unsigned int value)
{
	unsigned int X,A,result;
	X=(1<<(endBit-startBit+1))-1;
	A=rtd_inl(reg_addr);
	result = (A & (~(X<<startBit))) | (value<<startBit);
	rtd_outl(reg_addr,result);
}
#endif

unsigned int Scaler_GetDeRinging_level(TV001_LEVEL_E *drLevel)
{
	*drLevel = m_nDRLevel;
	return _SUCCESS;
}
void Scaler_SetDeRinging_level(TV001_LEVEL_E drLevel)
{
	DRV_NR_Item *ptr = NULL;
	SLR_VIP_TABLE *gVip_Table=NULL ;
	unsigned char tbl_idx = Scaler_GetDNR_table();
	if(drLevel >4)
		return ;
	m_nDRLevel = drLevel;
	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if(gVip_Table == NULL)
		return;

	if (tbl_idx >= PQA_TABLE_MAX)
		tbl_idx = 0;

	ptr = &(gVip_Table->Manual_NR_Table[tbl_idx][m_nDRLevel]);
	drvif_color_DRV_SNR_Mosquito_NR(&(ptr->S_SNR_TABLE.S_SNR_Mosquito_NR));
}
unsigned int Scaler_GetSRLevel(TV010_DEMOLEVEL_E *srLevel)
{
	*srLevel = m_nSRLevel;
	return _SUCCESS;
}

void Scaler_SetSRLevel(TV010_DEMOLEVEL_E srLevel)
{
	if(srLevel >4)
		return ;
	m_nSRLevel = srLevel;
	switch(srLevel){
		case TV010_DEMOLEVEL_OFF:
		case TV010_DEMOLEVEL_ON:
			driver_color_setSR(srLevel);
			break;
		case TV010_DEMOLEVEL_DEMO:
			break;
		default:
			break;
		}
}


unsigned int Scaler_GetColorGainLevel(TV001_LEVEL_E *colorGain)
{
	*colorGain = m_nCGLevel;
	return _SUCCESS;
}


	SLR_VIP_TABLE *gVIP_Table;
	unsigned char *drvSkip_Array;
	_system_setting_info *VIP_system_info_structure_table=NULL;


void Scaler_SetColorGainLevel(unsigned int icmIndex)
{
/*
	if(colorGain >4)
		return ;
	m_nCGLevel = colorGain;
	*/
	_system_setting_info *VIP_system_info_structure_table;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	gVIP_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if(gVIP_Table ==NULL){
		rtd_pr_vpq_err("-1-gVIP_Table ==NULL---error!-\n");
		return;
		}
	drvSkip_Array = &(gVIP_Table->DrvSetting_Skip_Flag[0]);
	if(drvSkip_Array ==NULL){
		rtd_pr_vpq_err("-1-drvSkip_Array ==NULL---error!-\n");
		return;
		}
	if(VIP_system_info_structure_table->HDR_flag)
		icmIndex=3;

	if(icmIndex>9)
		return;
	m_icmIndex = icmIndex;
	rtd_pr_vpq_info("---icm---file:%s;line:%d;icmTableIndex:%d;---\n",__FUNCTION__,__LINE__,icmIndex);
	Scaler_set_ICM_table(icmIndex,  drvSkip_Array[DrvSetting_Skip_Flag_ICM]);
}
unsigned int Scaler_GetFleshToneLevel(TV001_LEVEL_E *fleshtone)
{
	*fleshtone = m_nFTLevel;
	return _SUCCESS;
}

void Scaler_SetFleshToneLevel(TV001_LEVEL_E fleshtone)
{
/*
	if(fleshtone >4)
		return ;
	m_nFTLevel = fleshtone;
	*/

	gVIP_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if(gVIP_Table ==NULL){
		rtd_pr_vpq_err("-2-gVIP_Table ==NULL---error!-\n");
		return;
		}
	drvSkip_Array = &(gVIP_Table->DrvSetting_Skip_Flag[0]);
	if(drvSkip_Array ==NULL){
		rtd_pr_vpq_err("-2-drvSkip_Array ==NULL---error!-\n");
		return;
		}

	fwif_color_icm_set_block_para(fleshtone);
	rtd_pr_vpq_info("---icm---file:%s;line:%d; m_icmIndex=%d---\n",__FUNCTION__,__LINE__,m_icmIndex);
	Scaler_set_ICM_table(m_icmIndex,  drvSkip_Array[DrvSetting_Skip_Flag_ICM]);
}

void fwif_color_icm_set_block_para(TV001_LEVEL_E fleshtone)
{
	//TODO, Merlin3 tv010 sync, wait vip to add icm_block_adj
	/*
	switch(fleshtone){
		case TV001_LEVEL_OFF:
			{
				icm_block_adj.h_offset = 0;
				icm_block_adj.s_offset = 0;
				icm_block_adj.i_offset = 0;
				icm_block_adj.h_sta = 2;
				icm_block_adj.h_end = 6;
				icm_block_adj.s_sta = 2;
				icm_block_adj.s_end = 7;
				icm_block_adj.i_sta = 2;
				icm_block_adj.i_end = 7;
				break;
			}
		case TV001_LEVEL_LOW:
			{
				icm_block_adj.h_offset = 32;
				icm_block_adj.s_offset = 64;
				icm_block_adj.i_offset = 0;
				icm_block_adj.h_sta = 2;
				icm_block_adj.h_end = 6;
				icm_block_adj.s_sta = 2;
				icm_block_adj.s_end = 7;
				icm_block_adj.i_sta = 2;
				icm_block_adj.i_end = 7;
				break;
			}
		case TV001_LEVEL_MID:
			{
				icm_block_adj.h_offset = 64;
				icm_block_adj.s_offset = 128;
				icm_block_adj.i_offset = 0;
				icm_block_adj.h_sta = 2;
				icm_block_adj.h_end = 6;
				icm_block_adj.s_sta = 2;
				icm_block_adj.s_end = 7;
				icm_block_adj.i_sta = 2;
				icm_block_adj.i_end = 7;
				break;
			}
		case TV001_LEVEL_HIGH:
			{
				icm_block_adj.h_offset = 90;
				icm_block_adj.s_offset = 200;
				icm_block_adj.i_offset = 0;
				icm_block_adj.h_sta = 2;
				icm_block_adj.h_end = 6;
				icm_block_adj.s_sta = 2;
				icm_block_adj.s_end = 7;
				icm_block_adj.i_sta = 2;
				icm_block_adj.i_end = 7;
				break;
			}
		default:
			{
				icm_block_adj.h_offset = 0;
				icm_block_adj.s_offset = 0;
				icm_block_adj.i_offset = 0;
				icm_block_adj.h_sta = 2;
				icm_block_adj.h_end = 6;
				icm_block_adj.s_sta = 2;
				icm_block_adj.s_end = 7;
				icm_block_adj.i_sta = 2;
				icm_block_adj.i_end = 7;
			break;
			}

		}
	*/


}


unsigned int Scaler_GetODLevel(unsigned int *level)
{
	*level = drvif_color_get_od_gain(0);// 0~127
	return _SUCCESS;
}

void Scaler_SetODReadEnable(unsigned char enable)
{
  //  NULL
}
unsigned int Scaler_GetGammaNum(unsigned int *number)
{
	*number = 15;
	return _SUCCESS;
}
void Scaler_SetGamma_byLevel(unsigned char Level)
{
	StructColorDataType *pTable =NULL;
	unsigned char level;

	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display,& src_idx);


	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	rtd_pr_vpq_info("==  SetGamma_byLevel =%d, %d  ===\n",pTable->Gamma,Level);

	if(Level==7)//level = 5 is center, gamma=0.
		return;

	if(Level==6||Level==8)
		level=13;
	else if(Level==5||Level==9)
		level=19;
	else if(Level==4||Level==10)
		level=26;
	else if(Level==3||Level==11)
		level=32;
	else if(Level==2||Level==12)
		level=38;
	else if(Level==1||Level==13)
		level=51;
	else if(Level==0||Level==14)
		level=64;
	else
		level=0;


	if(Level<7) {
		//TODO, Merlin3 tv010 sync, wait vip to add function
		//fwif_set_software_gamma(pTable->Gamma, 0, level, level);
	}
	else {
		//TODO, Merlin3 tv010 sync, wait vip to add function
		//fwif_set_software_gamma(pTable->Gamma, 1023, level, level);
	}

}

void Scaler_Set_Software_Gamma(unsigned char gammaIndex)
{
	if(gammaIndex >6)
		return;
	//software_gamma  level 0~6 , level = 3 is center
	Scaler_SetGamma_byLevel(gammaIndex);
}
unsigned int Scaler_Get_TV010_PQModule(TV010_PQ_MODULE_E pqModule,unsigned char * onOff)
{
	*onOff = TRUE;
	return _SUCCESS;
}
void Scaler_Set_TV010_PQModule(TV010_PQ_MODULE_E pqModule,unsigned char onOff)
{
	//TODO, Merlin3 tv010 sync, wait vip to add function
    /*
	switch(pqModule) {

		case TV010_PQ_MODULE_FMD:
		drvif_color_set_PQ_Module(PQ_ByPass_Fmd,onOff);
		break;

		case TV010_PQ_MODULE_NR:
		drvif_color_set_PQ_Module(PQ_ByPass_RTNR_Y,onOff);
		drvif_color_set_PQ_Module(PQ_ByPass_SNR_Y,onOff);
		break;

		case TV010_PQ_MODULE_DB:
		drvif_color_set_PQ_Module(PQ_ByPass_MPEG_NR,onOff);
		break;

		case TV010_PQ_MODULE_DR:
		drvif_color_set_PQ_Module(PQ_ByPass_MosquiutoNR,onOff);
		break;

		case TV010_PQ_MODULE_HSHARPNESS:
		drvif_color_set_PQ_Module(PQ_ByPass_SHP_DLTI,onOff);
		break;

		case TV010_PQ_MODULE_SHARPNESS:
		drvif_color_set_PQ_Module(PQ_ByPass_SHP,onOff);
		drvif_color_set_PQ_Module(PQ_ByPass_SHP_DLTI,onOff);
		drvif_color_set_PQ_Module(PQ_ByPass_MB_Peaking,onOff); //temporary close MB_Peaking
		break;

		case TV010_PQ_MODULE_CCCL:
		drvif_color_set_PQ_Module(PQ_ByPass_CCCL,onOff);
		break;

		case TV010_PQ_MODULE_COLOR_CORING:
		drvif_color_set_PQ_Module(PQ_ByPass_COLOR_CORING,onOff);
		break;

		case TV010_PQ_MODULE_BLUE_STRETCH:
		if(onOff){
			IoReg_SetBits(0xB802C130, 0x00000003);//_BIT1
		}
		else{
			IoReg_ClearBits(0xB802C130, 0x00000003);//_BIT1
		}
		break;

		case TV010_PQ_MODULE_GAMMA:
		drvif_color_set_PQ_Module(PQ_ByPass_Gamma,onOff);
		break;

		case TV010_PQ_MODULE_DBC:
		drvif_color_set_PQ_Module(PQ_ByPass_DBC,onOff);
		break;

		case TV010_PQ_MODULE_DCI:
		drvif_color_set_PQ_Module(PQ_ByPass_DCC,onOff);
		break;

		case TV010_PQ_MODULE_COLOR:
		drvif_color_set_PQ_Module(PQ_ByPass_ICM,onOff);
		break;

		case TV010_PQ_MODULE_ES:
		//drvif_color_set_PQ_Module(PQ_ByPass_RTNR_Y,onOff);
		break;

		case TV010_PQ_MODULE_SR:
		drvif_color_set_PQ_Module(PQ_ByPass_MB_Peaking,onOff);
		break;

		case TV010_PQ_MODULE_FRC:
		HAL_VPQ_MEMC_MotionCompOnOff(onOff);
		break;

		case TV010_PQ_MODULE_WCG:
		drvif_color_set_PQ_Module(PQ_ByPass_WCG,onOff);
		break;

		case TV010_PQ_MODULE_ALL:
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
		if(onOff){
			IoReg_SetBits(0xB802C130, 0x00000003);//_BIT1
		}
		else{
			IoReg_ClearBits(0xB802C130, 0x00000003);//_BIT1
		}
		break;
	}
	*/
}



void Scaler_Set_TV010_DemoMode(TV010_DEMO_MODE_E demoMode,unsigned char onOff)
{
	_RPC_system_setting_info *VIP_system_RPC_info_structure_table;
	VIP_system_RPC_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	rtd_pr_vpq_info("==Scaler_SetDemoMode=mode=%d,%d \n",demoMode,onOff);
	if(!onOff){
		/*if(VIP_system_info_structure_table->SDR2HDR_flag)
			Scaler_LGE_ONEKEY_SDR2HDR_Enable(onOff);
		else */
		if(VIP_system_RPC_info_structure_table == NULL){
			rtd_pr_vpq_info("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
			return;
		}

		if(VIP_system_RPC_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En])
			Scaler_LGE_ONEKEY_HDR10_Enable(onOff);

		Scaler_SetMagicPicture(SLR_MAGIC_OFF);
		return;
	}
	switch(demoMode) {
		case TV010_DEMO_DBDR :
		break;
		case TV010_DEMO_NR:
		break;
		case TV010_DEMO_SHARPNESS:
			Scaler_SetMagicPicture(SLR_MAGIC_STILLDEMO_INVERSE);
		break;
		case TV010_DEMO_DCI:
		break;
		case TV010_DEMO_WCG:
			Scaler_SetMagicPicture(SLR_MAGIC_WCG_STILLDEMO_INVERSE);
		break;
		case TV010_DEMO_MEMC:
			Scaler_SetMagicPicture(SLR_MAGIC_MEMC_STILLDEMO_INVERSE);
		break;
		case TV010_DEMO_COLOR:
		break;
		case TV010_DEMO_SR:
			Scaler_SetMagicPicture(SLR_MAGIC_STILLDEMO_INVERSE);
		break;
		case TV010_DEMO_ALL:
		break;
		case TV010_DEMO_HDR:
			Scaler_SetMagicPicture(SLR_MAGIC_STILLDEMO_INVERSE);
		break;
		case TV010_DEMO_SDR_TO_HDR:
			Scaler_SetMagicPicture(SLR_MAGIC_STILLDEMO_INVERSE);
			Scaler_SetSDR2HDR(TRUE);
		break;
		default:
			break;

	}


}

unsigned int Scaler_GetCurrentPanelSupportLocaldimming(unsigned char *support)
{
	*support = FALSE;
	return _SUCCESS;
}

void Scaler_SetLocalDimmingDemoMode(unsigned char mode)
{
	Scaler_SetMagicPicture(SLR_MAGIC_LOCALDIMMING_MARQUEE);
}

unsigned int Scaler_GetLocalDimmingDemoMode(TV010_LOCALDIMMING_DEMO_MODE_E *demoMode)
{
	*demoMode = TV010_LDM_DEMO_OFF;
	return _SUCCESS;
}

void Scaler_SetLocalDimmingSpiEnable(unsigned char bEnable)
{
	rtd_part_outl(0xb802C900,0,0,bEnable); //Ld_spi enable
}



void Scaler_SetDCCProcess(void)
{
//	unsigned char *pVipCoefArray = NULL;
//	unsigned char source = 255;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	unsigned int dccIdx;
	_system_setting_info *VIP_system_info_structure_table;
	SLR_PICTURE_MODE_DATA *pData = NULL;
	Scaler_Get_Display_info(&display, &src_idx);
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;
	dccIdx=VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.DCC_Table;
	//init
	fwif_color_set_DCC_Init(display);

	//set DccTabel

	Scaler_SetDCC_Table(dccIdx);
	#if 0
	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);
	pVipCoefArray= (unsigned char *)Vip_GetQualityCoefExtend3((VIP_SOURCE_TIMING)source);

	if(!(source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)){
		unsigned int dccIdx = pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Table_Select];
		Scaler_SetDCC_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Table_Select]);
		rtd_pr_vpq_info("---testdcc---file:%s;line:%d;dccIdx:%d---\n",__FILE__,__LINE__,dccIdx);
	}
	#endif
	//setDCCMode
	fwif_color_set_dcc_mode(0, 4);
	//Scaler_set_dcc_Sync_DCC_AP_Level(0, Scaler_GetDCC_Table(), 4);
	Scaler_set_dcc_Sync_DCC_AP_Level(0, dccIdx, 4);

	Scaler_SetDCC_Mode(SLR_DCC_HIGH);//(pData->DccMode); /* set dcc mode */
}

#if 0
//---Vincent---  Creat thread for LD demo mode --->>>
static bool LD_round_demo_tsk_running_flag = FALSE;
static struct task_struct *p_LD_round_demo_tsk = NULL;

static int LD_round_demo_tsk(void *p)
{
	static UINT8 LD_Block = 0;

	rtd_pr_vpq_info("LD_round_demo_tsk()\n");

	while (1)
	{
		msleep(1000);
		Scaler_Set_LD_Demo_Round(LD_Block);
		++LD_Block;
		if(LD_Block >= 12)
			LD_Block = 0;

		if(kthread_should_stop())
			break;
	}

	rtd_pr_vpq_info("\r\n####bring_up_vsc_tsk: exit...####\n");
	do_exit(0);
	return 0;
}

static void create_LD_round_demo_tsk(void)
{
	int err;
	if(LD_round_demo_tsk_running_flag == FALSE)
	{
		p_LD_round_demo_tsk = kthread_create(LD_round_demo_tsk, NULL, "LD_round_demo_tsk");

		if(p_LD_round_demo_tsk)
		{
			wake_up_process(p_LD_round_demo_tsk);
			LD_round_demo_tsk_running_flag = TRUE;
		}
		else
		{
	    		err = PTR_ERR(p_LD_round_demo_tsk);
			rtd_pr_vpq_info("Unable to start LD_round_demo_tsk (err_id = %d)./n", err);
		}
	}
}

static void delete_LD_round_demo_tsk(void)
{
	int ret;
	if(LD_round_demo_tsk_running_flag)
	{
		ret = kthread_stop(p_LD_round_demo_tsk);
 		if(!ret)
		{
			p_LD_round_demo_tsk = NULL;
 			LD_round_demo_tsk_running_flag = FALSE;
			rtd_pr_vpq_debug("LD_round_demo_tsk thread stopped\n");
 		}
	}
}

//---Vincent---  Creat thread for LD demo mode ---<<<
static bool LD_spi_iw7027_check_tsk_running_flag = FALSE;
static struct task_struct *p_LD_spi_iw7027_check_tsk = NULL;
static bool boot_1st_flg = TRUE;

static int LD_spi_iw7027_check_tsk(void *p)
{
//	static UINT8 iw_Reg = 0x00;
	static UINT8 N = 0;
	static UINT8 BLout_flg = 1;

	rtd_pr_vpq_info("LD_spi_iw7027_check_tsk()\n");

	while (1)
	{
		msleep(5000);

		#if 1
		// Set 0x29=A6
		if(boot_1st_flg)
		{
			if(BLout_flg)
			{
				//rtd_pr_vpq_emerg("stop_backlight_data_out\n");
				//TODO, Merlin3 tv010 sync, wait vip to add function
				//drvif_LD_spi_iw7027_stop_backlight_data_out();
				BLout_flg = 0;
				msleep(1);  // no need wait next vsync
				//TODO, Merlin3 tv010 sync, wait vip to add function
				//drvif_LD_spi_iw7027_CS_changeto_GPIO();
			}
			//TODO, Merlin3 tv010 sync, wait vip to add function
			//drvif_LD_spi_iw7027_write_byte(0x29,0xA6);
			boot_1st_flg = FALSE;
		}

		//TODO, Merlin3 tv010 sync, wait vip to add function
		//while((!drvif_LD_spi_iw7027_detect_FaultB())&&(N<3))
		{
			rtd_pr_vpq_emerg("FaultB low N=%d\n",N);
			// stop 16bit backlight data output
			if(BLout_flg)
			{
				//rtd_pr_vpq_emerg("stop_backlight_data_out\n");
				//TODO, Merlin3 tv010 sync, wait vip to add function
				//drvif_LD_spi_iw7027_stop_backlight_data_out();
				BLout_flg = 0;
				msleep(1);  // no need wait next vsync
				//TODO, Merlin3 tv010 sync, wait vip to add function
				//drvif_LD_spi_iw7027_CS_changeto_GPIO();
			}

			// clear fault 0x61[1]=1
			//rtd_pr_vpq_emerg("clear fault 0x61[1]=1\n");
			//iw_Reg = drvif_LD_spi_iw7027_read_byte(0x61);
			//iw_Reg = (iw_Reg & (~_BIT1)) | _BIT1;
			//drvif_LD_spi_iw7027_write_Nbyte(0x61,1,&iw_Reg);
			//TODO, Merlin3 tv010 sync, wait vip to add function
			//drvif_LD_spi_iw7027_write_byte(0x61,0x02);

			msleep(20);  // wait next frame

			// 0x61[1]=0
			//rtd_pr_vpq_emerg("0x61[1]=0\n");
			//iw_Reg = drvif_LD_spi_iw7027_read_byte(0x61);
			//iw_Reg = (iw_Reg & (~_BIT1)) | (~_BIT1);
			//drvif_LD_spi_iw7027_write_Nbyte(0x61,1,&iw_Reg);
			//TODO, Merlin3 tv010 sync, wait vip to add function
			//drvif_LD_spi_iw7027_write_byte(0x61,0x00);

			msleep(20);  // wait next frame

			N++;

		}

		if(N==3)
		{
			// 0x29[0]=1 off short channel
			//iw_Reg = drvif_LD_spi_iw7027_read_byte(0x29);
			//iw_Reg = (iw_Reg & (~_BIT0)) | _BIT0;
			//drvif_LD_spi_iw7027_write_Nbyte(0x29,1,&iw_Reg);
			//TODO, Merlin3 tv010 sync, wait vip to add function
			//drvif_LD_spi_iw7027_write_byte(0x29,0xA7);

			// read 0x85 and 0x86 over 0x80 special read data
			UINT8 data[28];
			// before read bt 0x80 set 0x78[7]=1
			//iw_Reg = drvif_LD_spi_iw7027_read_byte(0x78);
			//iw_Reg = (iw_Reg & (~_BIT7)) | _BIT7;
			//drvif_LD_spi_iw7027_write_Nbyte(0x78,1,&iw_Reg);
			//TODO, Merlin3 tv010 sync, wait vip to add function
			//drvif_LD_spi_iw7027_write_byte(0x78,0x80);

			//TODO, Merlin3 tv010 sync, wait vip to add function
			//drvif_LD_spi_iw7027_read_Nbyte(0x05,2,data);
			rtd_pr_vpq_emerg("0x85=0x%x  0x86=0x%x\n",data[0],data[1]);

			// after read bt 0x80 set 0x78[7]=0
			//iw_Reg = drvif_LD_spi_iw7027_read_byte(0x78);
			//iw_Reg = (iw_Reg & (~_BIT7));
			//drvif_LD_spi_iw7027_write_Nbyte(0x78,1,&iw_Reg);
			//TODO, Merlin3 tv010 sync, wait vip to add function
			//drvif_LD_spi_iw7027_write_byte(0x78,0x00);
			break;
		}

		N = 0;

		#endif

		// stop 16bit backlight data output
		if(BLout_flg)
		{
			//TODO, Merlin3 tv010 sync, wait vip to add function
			//drvif_LD_spi_iw7027_stop_backlight_data_out();
			BLout_flg = 0;
			msleep(1);  // no need wait next vsync
			//TODO, Merlin3 tv010 sync, wait vip to add function
			//drvif_LD_spi_iw7027_CS_changeto_GPIO();
		}

		// check iw7027 0x1c - 0x37
		{
			UINT8 iw_1c_37[28] = {0xFF,0xC7,0xFF,0xF0,0x00,0xEF,0xFF,0x0F,0xFF,0xFF,0xC0,0x3f,0x44,0xa6,
					0x13,0x60,0xa0,0xE0,0xE4,0xBC,0xDD,0xC4,0x65,0x16,0x88,0xA0,0x44,0x00};
			UINT8 data[28];
			UINT8 i;

			//TODO, Merlin3 tv010 sync, wait vip to add function
			//drvif_LD_spi_iw7027_read_Nbyte(0x1c,28,data);
			for(i=0;i<28;i++)
			{
				//rtd_pr_vpq_emerg("data[%x] = 0x%x\n",(i+0x1c),data[i]);
				//rtd_pr_vpq_emerg("iw_1c_37[%x] = 0x%x\n",(i+0x1c),iw_1c_37[i]);
				if(iw_1c_37[i] != data[i])
				{
					// reset 0x1C - 0x37 reg
					if(i == 13)
						continue;
					rtd_pr_vpq_emerg("read data[%x] = 0x%x\n",(i+0x1c),data[i]);
					rtd_pr_vpq_emerg("right data[%x] = 0x%x\n",(i+0x1c),iw_1c_37[i]);
					//rtd_pr_vpq_emerg(" check error need reset");
					//TODO, Merlin3 tv010 sync, wait vip to add function
					//drvif_LD_spi_iw7027_write_byte(0x00,0x06);  // [Soft_rst_n]=0
					msleep(1);
					//TODO, Merlin3 tv010 sync, wait vip to add function
					//drvif_LD_spi_iw7027_write_Nbyte(0x1c,28,iw_1c_37);
					msleep(1);
					//TODO, Merlin3 tv010 sync, wait vip to add function
					//drvif_LD_spi_iw7027_write_byte(0x00,0x07);  // [Soft_rst_n]=1
					break;
				}

			}
			msleep(5);
			//TODO, Merlin3 tv010 sync, wait vip to add function
			//drvif_LD_spi_iw7027_resume_backlight_data_out();
			BLout_flg = 1;
		}


		if(kthread_should_stop())
			break;
	}

	rtd_pr_vpq_emerg("\r\n####bring_up_vsc_tsk: exit...####\n");
	do_exit(0);
	return 0;
}

static void create_LD_spi_iw7027_check_tsk(void)
{
	int err;
	if(LD_spi_iw7027_check_tsk_running_flag == FALSE)
	{
		p_LD_spi_iw7027_check_tsk = kthread_create(LD_spi_iw7027_check_tsk, NULL, "LD_spi_iw7027_check_tsk");

		if(p_LD_spi_iw7027_check_tsk)
		{
			wake_up_process(p_LD_spi_iw7027_check_tsk);
			LD_spi_iw7027_check_tsk_running_flag = TRUE;
		}
		else
		{
	    		err = PTR_ERR(p_LD_spi_iw7027_check_tsk);
			rtd_pr_vpq_info("Unable to start LD_spi_iw7027_check_tsk (err_id = %d)./n", err);
		}
	}
}

static void delete_LD_spi_iw7027_check_tsk(void)
{
	int ret;
	if(LD_spi_iw7027_check_tsk_running_flag)
	{
		ret = kthread_stop(p_LD_spi_iw7027_check_tsk);
 		if(!ret)
		{
			p_LD_spi_iw7027_check_tsk = NULL;
 			LD_spi_iw7027_check_tsk_running_flag = FALSE;
			rtd_pr_vpq_debug("LD_spi_iw7027_check_tsk thread stopped\n");
 		}
	}
}
#endif
void Scaler_LD_Spi_Init(void)
{
	//TODO, Merlin3 tv010 sync, wait panelapi.c modification
	/*
	if((Get_PANEL_LD_Enable())&&(!Get_PANEL_LD_SPIDecoderMcuInPower_Enable()))
		create_LD_spi_iw7027_check_tsk();
	*/
}
/*
void Scaler_LD_Spi_UnInit(void)
{
	delete_LD_spi_iw7027_check_tsk();
} */

static unsigned char HorseLight_status = false;
atomic_t HorseLight_tmp;
unsigned char Scaler_GetHorseLight(void)
{
	return HorseLight_status;
}

bool LD_round_demo_OnlyOnce(void)
{
	static UINT8 LD_Block = 0;
	rtd_pr_vpq_info("YR:LD_round_demo_OnlyOnce(),HorseLight_status =%d \n",HorseLight_status);

	for(LD_Block = 0;LD_Block < 12;++LD_Block)
	{
		msleep(1000);
		Scaler_Set_LD_Demo_Round(LD_Block);
		if(!atomic_read(&HorseLight_tmp)){
//			rtd_printk(KERN_INFO,"YR","=2222=HorseLight_tmp=%d,HorseLight_status=%d \n",HorseLight_tmp,HorseLight_status);
		//	break;
		 }

        }

	rtd_pr_vpq_info("\r\n ####bring_up_vsc_tsk: exit...#### HorseLight_status =%d\n",HorseLight_status);
	return true;
}

unsigned char Scaler_Get_LD_Table(void)
{
	_RPC_system_setting_info *VIP_system_RPC_info_structure_table;
	SLR_VIP_TABLE *gVip_Table = NULL;
	unsigned char LD_Table_Idx;
	unsigned char Vip_Source_Idx;

	VIP_system_RPC_info_structure_table = (_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (VIP_system_RPC_info_structure_table == NULL)
	{
		printf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}
	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if(gVip_Table == NULL)
	{
		printf("~gVip_Table Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}

	Vip_Source_Idx = VIP_system_RPC_info_structure_table->VIP_source;
	LD_Table_Idx = gVip_Table->VIP_QUALITY_Extend_Coef[Vip_Source_Idx][7];

	return LD_Table_Idx;

}

void Scaler_Set_SFG_BG(unsigned char en)
{
	sfg_sfg_force_bg_at_dif_RBUS sfg_force_bg_at_dif_reg;

	sfg_force_bg_at_dif_reg.regValue = IoReg_Read32(SFG_SFG_FORCE_BG_AT_DIF_reg);
	sfg_force_bg_at_dif_reg.bg_dif_red = 0x1ff;
	sfg_force_bg_at_dif_reg.bg_dif_grn = 0x1ff;
	sfg_force_bg_at_dif_reg.bg_dif_blu = 0x1ff;

	if(en)
		sfg_force_bg_at_dif_reg.en_force_bg_dif = 1;
	else
		sfg_force_bg_at_dif_reg.en_force_bg_dif = 0;

	IoReg_Write32(SFG_SFG_FORCE_BG_AT_DIF_reg, sfg_force_bg_at_dif_reg.regValue);
}

void Scaler_Set_LD_Demo_Half(unsigned char direction)
{
//	unsigned char i;
	unsigned char nRegion;
	unsigned char table_index;
	extern DRV_Local_Dimming_InterFace_Table Local_Dimming_InterFace_Table[LD_Table_NUM];

	table_index = Scaler_Get_LD_Table();
	table_index = 2;	//for test
	nRegion = Local_Dimming_InterFace_Table[table_index].LD_Data_Output_Index_SRAM.ld_Region_Size;

	switch(nRegion)
	{
		case BL_REGION_12:
			if(direction == 0)
				IoReg_Write32(LDSPI_Outindex_Addr_CTRL_reg, 0x00020000);
			else
				IoReg_Write32(LDSPI_Outindex_Addr_CTRL_reg, 0x00080000);
			IoReg_Write32(LDSPI_Outindex_Rwport_reg, 0x1a);
			IoReg_Write32(LDSPI_Outindex_Rwport_reg, 0x1b);
			IoReg_Write32(LDSPI_Outindex_Rwport_reg, 0x1c);
			IoReg_Write32(LDSPI_Outindex_Rwport_reg, 0x1d);
			IoReg_Write32(LDSPI_Outindex_Rwport_reg, 0x1e);
			IoReg_Write32(LDSPI_Outindex_Rwport_reg, 0x1f);

			IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, 0x001a0000);
			IoReg_Write32(LDSPI_Data_Rwport_reg, 0x0fff);
			IoReg_Write32(LDSPI_Data_Rwport_reg, 0x0fff);
			IoReg_Write32(LDSPI_Data_Rwport_reg, 0x0fff);
			IoReg_Write32(LDSPI_Data_Rwport_reg, 0x0fff);
			IoReg_Write32(LDSPI_Data_Rwport_reg, 0x0fff);
			IoReg_Write32(LDSPI_Data_Rwport_reg, 0x0fff);

			break;

		case BL_REGION_108:
		case BL_REGION_180:
		case BL_REGION_240:
		default:
			break;
	}
}

void Scaler_Set_LD_Demo_Full(void)
{
	unsigned char i;
	unsigned char nRegion;
	unsigned char table_index;
	unsigned char temp;
	extern DRV_Local_Dimming_InterFace_Table Local_Dimming_InterFace_Table[LD_Table_NUM];

	table_index = Scaler_Get_LD_Table();
	table_index = 2;	//for test
	nRegion = Local_Dimming_InterFace_Table[table_index].LD_Data_Output_Index_SRAM.ld_Region_Size;
	temp = Local_Dimming_InterFace_Table[table_index].LD_Data_Output_Index_SRAM.ld_Start_ID_1st_byte - 2;

	switch(nRegion)
	{
		case BL_REGION_12:
			IoReg_Write32(LDSPI_Outindex_Addr_CTRL_reg, 0x00020000);
			for(i = 2 ; i < 14 ; i++)
			{
				IoReg_Write32(LDSPI_Outindex_Rwport_reg, temp);
				temp -= 2;
			}
			break;

		case BL_REGION_108:
		case BL_REGION_180:
		case BL_REGION_240:
		default:
			break;
	}
}

void Scaler_Set_LD_Demo_Round(unsigned char Num)
{
	unsigned char i;
	unsigned char nRegion;
	unsigned char table_index;
//	unsigned char temp;
	extern DRV_Local_Dimming_InterFace_Table Local_Dimming_InterFace_Table[LD_Table_NUM];

	table_index = Scaler_Get_LD_Table();
	table_index = 2;	//for test
	nRegion = Local_Dimming_InterFace_Table[table_index].LD_Data_Output_Index_SRAM.ld_Region_Size;

	switch(nRegion)
	{
		case BL_REGION_12:

			IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, 0x00000000);
			for(i = 0 ; i < 12 ; i++)
			{
				if(i == Num)
				{
					IoReg_Write32(LDSPI_Data_Rwport_reg, 0x0fff);
					IoReg_Write32(LDSPI_Data_Rwport_reg, 0x0fff);
				}
				else
				{
					IoReg_Write32(LDSPI_Data_Rwport_reg, 0x0000);
					IoReg_Write32(LDSPI_Data_Rwport_reg, 0x0000);
				}
			}
			break;

		case BL_REGION_108:
		case BL_REGION_180:
		case BL_REGION_240:
		default:
			break;
	}
}

void Scaler_Set_LD_Demo_Mode(SKY_MW_LOCALDIMMING_DEMO_MODE_E mode)
{
	Scaler_Set_SFG_BG(_DISABLE);
	//Scaler_Set_LD_Demo_Full();

	printk("\n---lhh----DEMO_MODE mode=%d\n",mode);

	switch(mode)
	{
		case SKY_MW_LDM_DEMO_RIGHT_SCREEN:
			Scaler_Set_LD_Demo_Half(0);
			break;

		case SKY_MW_LDM_DEMO_LEFT_SCREEN:
			Scaler_Set_LD_Demo_Half(1);
			break;

		case SKY_MW_LDM_DEMO_WHITE_SCREEN:
			Scaler_Set_SFG_BG(_ENABLE);
			break;

	/*	case SKY_MW_LDM_DEMO_HORSELIGHT:
			drvif_color_set_LD2SPI_Enable(_DISABLE);
			Scaler_Set_SFG_BG(_ENABLE);
			create_LD_round_demo_tsk();
			break;*/
		case SKY_MW_LDM_DEMO_HORSELIGHT_ONLYONCE:
			//TODO, Merlin3 tv010 sync, need wait vip to add function
			//drvif_color_set_LD2SPI_Enable(_DISABLE);
		//	Scaler_Set_SFG_BG(_ENABLE);
			atomic_set(&HorseLight_tmp, 1);
			HorseLight_status = true;
			LD_round_demo_OnlyOnce();
			msleep(10);
			HorseLight_status = false;
		//	Scaler_Set_SFG_BG(_DISABLE);
			//TODO, Merlin3 tv010 sync, need wait vip to add function
			//drvif_color_set_LD2SPI_Enable(_ENABLE);
			break;

		case SKY_MW_LDM_DEMO_OFF:
		//	delete_LD_round_demo_tsk();
		//	HorseLight_status = true;
			atomic_set(&HorseLight_tmp, 0);
		//	Scaler_Set_SFG_BG(_DISABLE);
			//TODO, Merlin3 tv010 sync, need wait vip to add function
			//drvif_color_set_LD2SPI_Enable(_ENABLE);
			break;

		case SKY_MW_LDM_DEMO_TOP_SCREEN:
		case SKY_MW_LDM_DEMO_BOTTOM_SCREEN:
		case SKY_MW_LDM_DEMO_RIGHT_WHITE_SCREEN:
		case SKY_MW_LDM_DEMO_LEFT_WHITE_SCREEN:
		case SKY_MW_LDM_DEMO_TOP_WHITE_SCREEN:
		case SKY_MW_LDM_DEMO_BOTTOM_WHITE_SCREEN:
		default:
			break;
	}
}
void Scaler_SetPictureMode_tv010(unsigned char level)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;
	pData->mode = (PICTURE_MODE)level;

	rtd_pr_vpq_info("Scaler_SetPictureMode, %d\n", level);

	fwif_color_set_set_picture_mode(src_idx, level);/* send to share Memory*/

	/*== get current picture mode OSD data ==*/

	rtd_pr_vpq_info("Scaler_set_picture_mode = %d, value = %d ++++++@@\n", pData->mode, level);
	rtd_pr_vpq_info("\nset_picture_mode:::src_idx = %d \n", src_idx);
	rtd_pr_vpq_info("set_picture_mode:::Contrast = %d \n", pData->Contrast);
	rtd_pr_vpq_info("set_picture_mode:::Brightness = %d \n", pData->Brightness);
	rtd_pr_vpq_info("set_picture_mode:::Saturation = %d \n", pData->Saturation);
	rtd_pr_vpq_info("set_picture_mode:::Hue = %d \n", pData->Hue);
	rtd_pr_vpq_info("set_picture_mode:::Sharpness = %d \n", pData->Sharpness);
	rtd_pr_vpq_info("set_picture_mode:::BackLight = %d \n", pData->BackLight);
	rtd_pr_vpq_info("set_picture_mode:::DNR = %d \n", pData->DNR);
	rtd_pr_vpq_info("set_picture_mode:::DccMode = %d \n", pData->DccMode);


	/*set HSBC*/
	/* this path(contrast, brightness, saturation) need to remove future. using "Scaler_SetContrast, Scaler_SetBrightness, Scaler_SetSaturation" directly*/
	/*Scaler_setcontrastbrightness_style(_MAIN_DISPLAY); */ /*set OSD contrast and brightness*/
	Scaler_SetContrast(pData->Contrast);
	Scaler_SetBrightness(pData->Brightness);
	Scaler_SetSaturation(pData->Saturation);
	/*Scaler_seticmhuesat_style(_MAIN_DISPLAY); */ /*set OSD contrast and brightness */ /* need set hue, can't not mark, elieli*/
	Scaler_SetHue(128);

	fwif_color_RHAL_SetDNR(0); /* Init DNR*/
	/* set NR*/
	Scaler_SetMPEGNR(/*pData->MPEG*/TV001_LEVEL_AUTO, CALLED_NOT_BY_OSD);
	Scaler_SetDNR(pData->DNR/*TV001_LEVEL_AUTO*/);

	/*rtd_pr_vpq_debug("OKOKOKOKOKOKOKOKOKOKOK\n");*/
	/*return;*/

	/*set sharpness*/
	Scaler_SetSharpness(pData->Sharpness);/*set OSD sharpness*/

	/*set DCC*/
	//fwif_color_set_dcc_mode(0, 4);
	//Scaler_set_dcc_Sync_DCC_AP_Level(0, Scaler_GetDCC_Table(), 4);
	//Scaler_SetDCC_Mode(pData->DccMode); /* set dcc mode*/

	/*set Backlight*/
	#if 0
	Panel_SetBackLightLevel(BACKLIGHT_USER, pData->BackLight);
	Scaler_SetOSDBacklight2VIP(pData->BackLight);		/*20120614 roger*/
	Scaler_SetDcrMode(pData->DCRMode);
	#endif

	/*set Color temp*/
	//Scaler_SetColorTemp((SLR_COLORTEMP_LEVEL)scaler_get_color_temp_level_type(src_idx)); /*need to fix CSFC*/

	/* set film*/
	Scaler_SetFilm_Setting();

 	/* set CLEAR WHITE GAIN*/
	#if 0
       	fwif_color_set_UV_Offset_byTBL_Level(src_idx, display,0,VIP_YUV2RGB_LEVEL_LOW,0,0);
	#else
	if(((((SLR_COLORTEMP_LEVEL)pData->ColorTemp)==SLR_COLORTEMP_COOL)||(((SLR_COLORTEMP_LEVEL)pData->ColorTemp)==SLR_COLORTEMP_COOLER)))//FOR VIVID pic mode
		{
			fwif_color_set_UV_Offset_byTBL_Level(src_idx, display,0,VIP_YUV2RGB_LEVEL_LOW,0,0);
			rtd_pr_vpq_info("VIP_YUV2RGB_LEVEL_LOW \n");
		}
	else
		{
			fwif_color_set_UV_Offset_byTBL_Level(src_idx, display,0,VIP_YUV2RGB_LEVEL_OFF,0,0);
			rtd_pr_vpq_info("VIP_YUV2RGB_LEVEL_OFF \n");
		}
	#endif
}
void Scaler_SetDCC_Table_tv010(unsigned char dccTblIdx)
{
	Scaler_SetDCC_Table(dccTblIdx);
       fwif_color_set_dcc_mode(0, 4);
	Scaler_set_dcc_Sync_DCC_AP_Level(0,dccTblIdx, 4);
	Scaler_SetDCC_Mode(SLR_DCC_HIGH);

}


void Scaler_SetDCCMapping_tv010(unsigned char dccTblIdx)
{
	unsigned char *pVipCoefArray = NULL;
	unsigned char source = 255;
	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);
	pVipCoefArray= (unsigned char *)Vip_GetQualityCoefExtend3((VIP_SOURCE_TIMING)source);
	if(!(source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)){
		if(dccTblIdx==0||dccTblIdx==1||dccTblIdx==2||dccTblIdx==3){
			dccTblIdx=pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Table_Select];
			rtd_pr_vpq_info("=DCCDebug=file:%s=line:%d=dccTblIdx:%d==\n",__FILE__,__LINE__,dccTblIdx);
			Scaler_SetDCCProcess();
		}
	}else{
//			rtd_pr_vpq_info("=DCCDebug=file:%s=line:%d=something is error==\n");
		}
}
void Scaler_SetGAMMAMapping_tv010(unsigned char nTblIdx)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;

	unsigned char *pVipCoefArray = NULL;
	unsigned char source = 255;
	unsigned char nGammaTable =0;
	unsigned char nGamma_index =0;
	unsigned char nGamma_low =0;
	unsigned char nGamma_high =0;

	Scaler_Get_Display_info(&display, &src_idx);

	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);
	pVipCoefArray= (unsigned char *)Vip_GetQualityCoefExtend3((VIP_SOURCE_TIMING)source);
	//default  USER
	fwif_color_set_set_picture_mode(src_idx,PICTURE_MODE_USER);

	if((source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)){
		nGammaTable =0;
		nGamma_index =0;
		nGamma_low =0;
		nGamma_high =0;
		}
	else if(nTblIdx == 2 ){  //DOLBY_HDR
		nGammaTable =1;
		nGamma_index =0;
		nGamma_low =0;
		nGamma_high =0;
	}
	else if(nTblIdx == 4){  //PICTURE MOVIE
		nGammaTable =1;
		nGamma_index =0;
		nGamma_low =0;
		nGamma_high =0;
		fwif_color_set_set_picture_mode(src_idx,PICTURE_MODE_MOVIE);
	}
	else{
		nGammaTable =pVipCoefArray[VIP_QUALITY_FUNCTION_Gamma];
		nGamma_index =pVipCoefArray[VIP_QUALITY_FUNCTION_S_Gamma_Index];
		nGamma_low =pVipCoefArray[VIP_QUALITY_FUNCTION_S_Gamma_Low];
		nGamma_high =pVipCoefArray[VIP_QUALITY_FUNCTION_S_Gamma_High];
	}

	Scaler_SetGamma_for_ddomainISR_Handler(nGammaTable,nGamma_index,nGamma_low,nGamma_high, 0);
	//extern void vpq_set_gamma(unsigned char ucDoCtrlBack, unsigned char useRPC);
	//vpq_set_gamma(1,1);

	//SetGamma_byLevel
	//Scaler_SetGamma_byLevel(11);
}

unsigned char Scaler_GetCOLORTEMPMapping_tv010(void)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return 0;

	return pData->ColorTemp;
}

void Scaler_SetCOLORTEMPMapping_tv010(unsigned char mode)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;

	if((RTK_MW_COLORTEMP_E)mode == RTK_MW_COLORTEMP_NATURE)  /**< ?准*/
	{
		pData->ColorTemp = SLR_COLORTEMP_NORMAL;
	}
	else if((RTK_MW_COLORTEMP_E)mode == RTK_MW_COLORTEMP_COOL)    /**< 冷色*/
	{
		pData->ColorTemp = SLR_COLORTEMP_COOL;
	}
	else if((RTK_MW_COLORTEMP_E)mode == RTK_MW_COLORTEMP_WARM)   /**< ??*/
	{
		pData->ColorTemp = SLR_COLORTEMP_WARM;
	}
	else if((RTK_MW_COLORTEMP_E)mode == RTK_MW_COLORTEMP_USER)    /**< 用?自定?*/
	{
		pData->ColorTemp = SLR_COLORTEMP_USER ;
	}
	else {
		pData->ColorTemp = SLR_COLORTEMP_NORMAL;
	}
	rtd_pr_vpq_info("==Scaler_GetCOLORTEMPMapping_tv010=mode=%d \n",mode);
	if(((((SLR_COLORTEMP_LEVEL)pData->ColorTemp)==SLR_COLORTEMP_COOL)||(((SLR_COLORTEMP_LEVEL)pData->ColorTemp)==SLR_COLORTEMP_COOLER)))//FOR VIVID pic mode
		{
			fwif_color_set_UV_Offset_byTBL_Level(src_idx, display,0,VIP_YUV2RGB_LEVEL_LOW,0,0);
			rtd_pr_vpq_info("VIP_YUV2RGB_LEVEL_LOW \n");
	}
	else
	{
			fwif_color_set_UV_Offset_byTBL_Level(src_idx, display,0,VIP_YUV2RGB_LEVEL_OFF,0,0);
			rtd_pr_vpq_info("VIP_YUV2RGB_LEVEL_OFF \n");
	}

}

