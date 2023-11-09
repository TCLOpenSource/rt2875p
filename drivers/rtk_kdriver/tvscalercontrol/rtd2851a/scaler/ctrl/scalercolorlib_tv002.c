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

#include <tvscalercontrol/vip/vip_reg_def.h>
#include <rbus/ppoverlay_reg.h>

//#include <rbus/rbus_DesignSpec_MISC_LSADCReg.h>
#include <rbus/dm_reg.h>
#include <rbus/hdr_all_top_reg.h>
#include <rbus/vgip_reg.h>
#include <rbus/vodma_reg.h>
#include <rbus/h3ddma_hsd_reg.h>

#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv002.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scalerdrv/pipmp.h>

#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/vip/scalerColor.h>
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
#include <scaler/scalerDrvCommon.h>
#include <scaler/vipRPCCommon.h>
#include <scaler_vscdev.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <rtd_log/rtd_module_log.h>

#include <scaler_vpqmemcdev.h>
#include <tvscalercontrol/vip/localcontrast.h>
#include "vgip_isr/scalerDI.h"



/*******************************************************************************
* Macro
******************************************************************************/

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
unsigned char g_BLE_TV002_save = 0;
char g_sGamma_TV002_save = 0;
unsigned char g_LiveColor_save = 0;

VIP_MAGIC_GAMMA_Curve_Driver_Data g_MagicGammaDriverDataSave = {
	{0, 428, 828, 1245, 1645, 2062, 2478, 2878, 3295, 3695, 4095},
	{0, 428, 828, 1245, 1645, 2062, 2478, 2878, 3295, 3695, 4095},
	{0, 428, 828, 1245, 1645, 2062, 2478, 2878, 3295, 3695, 4095},
	{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024},
	10
};

unsigned char g_Clarity_RealityCreation_save = 1;
unsigned char g_Clarity_Resolution_save = 32;
unsigned char g_Clarity_Masterd4K_save = 0;

unsigned char ID_TV002_Flag[ID_ITEM_TV002_MAX] = {0} ;
unsigned char ID_TV002_Apply = 1 ;
unsigned char ID_TV002_Detect = 1 ;

/*******************************************************************************
* Program
******************************************************************************/
unsigned int Scaler_di_pow_TV002(unsigned int x, unsigned int p)
{
	int i=0;
	unsigned int r = 1;

	  if(p == 0) return 1;
	  if(x == 0) return 0;


	for(i=0;i<p;i++)
	{
		r*=x;
	}
	return r;
/*
	  unsigned int r = 1;
	  for(;;)
	  {
	  	if(p & 1)
			r *= x;
	    	if((p >>= 1) == 0)
			return r;
	   	 x *= x;
	  }
	  */
}

//TV002 PQ function
SLR_VIP_TABLE_CUSTOM_TV002* Scaler_get_tv002_Custom_struct(void)
{
	extern  SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;

	return &m_customVipTable_demo_tv002;
}

// Tv002 picture mode function
void Scaler_set_Intelligent_Picture_Enable(unsigned char En_flag)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	SLR_PICTURE_MODE_DATA *pData=NULL;
	_system_setting_info *VIP_system_info_structure_table=NULL;
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;

	Scaler_Get_Display_info(&display,& src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
	{
		printf("~get pic data Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}

	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL)
	{
		printf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}

	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = &m_customVipTable_demo_tv002;
	/*if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}*/

	if(En_flag==1 && (VIP_system_info_structure_table->Input_src_Type == _SRC_TV ||VIP_system_info_structure_table->Input_src_Type == _SRC_CVBS))
	{
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 0;		//marked future?, elieli
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR_OFF] = 0;
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR_LOW] = 0;
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR_MID] = 0;
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR_HIGH] = 0;
		//VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR_AUTO] = 0;

		p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.En_Flag = 1;
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_INTELL_PIC] = 1;
	}
	else
	{
		p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.En_Flag = 0;
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_INTELL_PIC] = 0;

		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 1;	//marked future?, elieli
		if(pData->DNR==0)
			VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR_OFF] = 1;
		if(pData->DNR==1)
			VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR_LOW] = 1;
		if(pData->DNR==2)
			VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR_MID] = 1;
		if(pData->DNR==3)
			VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR_HIGH] = 1;
		//VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR_AUTO] = 0;

	}

}

unsigned char Scaler_get_Intelligent_Picture_Enable(void)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = &m_customVipTable_demo_tv002;

	/*if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}*/
	return p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.En_Flag;
}

void Scaler_set_Intelligent_Picture_Optimisation(unsigned char Level)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = &m_customVipTable_demo_tv002;

	/*if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}*/

	p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.PICTURE_OPTIMISATION = Level;
}

unsigned char Scaler_get_Intelligent_Picture_Optimisation(void)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = &m_customVipTable_demo_tv002;

	/*if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}*/
	return p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.PICTURE_OPTIMISATION;
}

void Scaler_set_Intelligent_Picture_Signal_Level_Indicator_ONOFF(unsigned char En_Flag)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = &m_customVipTable_demo_tv002;

	/*if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}*/
	p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.Sig_Lv_Indicator_En = En_Flag;

}

unsigned char Scaler_Intelligent_Picture_get_Signal_Level_Indicator_ONOFF(void)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = &m_customVipTable_demo_tv002;

	/*if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}*/

	return p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.Sig_Lv_Indicator_En;

}

unsigned char Scaler_Intelligent_Picture_get_Intelligent_Picture_Signal_Level(void)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = &m_customVipTable_demo_tv002;

	/*if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}*/
	return p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.Sig_Lv;
}

VIP_MAGIC_GAMMA_Function* Scaler_get_Magic_Gamma(void)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = &m_customVipTable_demo_tv002;

	/*if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return NULL;
	}*/
	return &(p_Share_Memory_VIP_TABLE_Custom_Struct->MAGIC_GAMMA_Function);

}

void Scaler_set_BP_initial(VIP_SOURCE_TIMING VIP_SrcTiming, PICTURE_MODE Pic_Mode)
{
	unsigned short table_idx;
	unsigned char idx8;
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = &m_customVipTable_demo_tv002;

	/*if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}*/
 	table_idx = p_Share_Memory_VIP_TABLE_Custom_Struct->BP_Function.BP_Ctrl.BP_Level_TBL_Select;
 	idx8 = fwif_color_ChangeOneUINT16Endian(table_idx, 0);
	table_idx = p_Share_Memory_VIP_TABLE_Custom_Struct->BP_Function.BP_Ctrl.BP_Level_TBL[idx8][(unsigned char)Pic_Mode];
 	p_Share_Memory_VIP_TABLE_Custom_Struct->BP_Function.BP_Ctrl.BP_TBL_Select = table_idx;
	idx8 = fwif_color_ChangeOneUINT16Endian(table_idx, 0);
	p_Share_Memory_VIP_TABLE_Custom_Struct->BP_Function.BP_Table[idx8].Smooth_Change.Init_Flag = 0xFFFF;
}

unsigned char Scaler_Get_ADV_API_LEVEL_TBL_Idx_TV002(VIP_SOURCE_TIMING VIP_SrcTiming, VIP_ADV_API_LEVEL API_Level)
{
	unsigned char ret_idx, idx;
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = &m_customVipTable_demo_tv002;

	/*if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}*/

	idx = p_Share_Memory_VIP_TABLE_Custom_Struct->VIP_QUALITY_Coef_TV002[(unsigned char)VIP_SrcTiming][VIP_QUALITY_FUNCTION_TV002_ADV_API];
	ret_idx = p_Share_Memory_VIP_TABLE_Custom_Struct->ADV_API.VIP_ADV_API_Level_TBL[idx][(unsigned char)API_Level];
	return ret_idx;
};

char Scaler_get_sGamma_TV002(void)
{
	return g_sGamma_TV002_save;
}

void Scaler_set_sGamma_TV002(char level_index)
{
	unsigned char table_idx;
	VIP_ADV_API_LEVEL API_Level;
	VIP_SOURCE_TIMING VIP_SrcTiming;

	unsigned char src_idx = 0;
	unsigned char display = 0;
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	_RPC_system_setting_info *RPC_VIP_system_info_structure_table=NULL;
	extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	Scaler_Get_Display_info(&display,&src_idx);
/*
	SLR_PICTURE_MODE_DATA *pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
	{
		printf("~get picData Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}
	pData->adv_gamma= level_index;
*/
	g_sGamma_TV002_save = level_index;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = &m_customVipTable_demo_tv002;

	/*if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}*/

	RPC_VIP_system_info_structure_table = (_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (RPC_VIP_system_info_structure_table == NULL)
	{
		printf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}
	VIP_SrcTiming = (VIP_SOURCE_TIMING)RPC_VIP_system_info_structure_table->VIP_source;

	if(level_index<0)
	{
		level_index = level_index * (-1);
		p_Share_Memory_VIP_TABLE_Custom_Struct->ADV_API.ADV_API_CTRL.is_sGamma_Neg = 1;
	}
	else
		p_Share_Memory_VIP_TABLE_Custom_Struct->ADV_API.ADV_API_CTRL.is_sGamma_Neg = 0;

	if(level_index>=VIP_ADV_API_LEVEL_Max)
	{
		API_Level = ADV_API_LEVEL_OFF;
		printf("~level_index>=VIP_ADV_API_LEVEL_Max, level= %d , %s->%d, %s~\n", level_index, __FILE__, __LINE__,__FUNCTION__);
	}
	else
		API_Level = (VIP_ADV_API_LEVEL)level_index;
	table_idx = Scaler_Get_ADV_API_LEVEL_TBL_Idx_TV002(VIP_SrcTiming, API_Level);
	p_Share_Memory_VIP_TABLE_Custom_Struct->ADV_API.ADV_API_CTRL.TBL_Select_sGamma = table_idx;
	fwif_color_set_DCC_SC_for_Coef_Change(1,1);

};

unsigned char Scaler_get_BLE_TV002(void)
{
	return g_BLE_TV002_save;
}

void Scaler_set_BLE_TV002(unsigned char level_index)
{
	unsigned char table_idx;
	VIP_ADV_API_LEVEL API_Level;
	VIP_SOURCE_TIMING VIP_SrcTiming;
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	_RPC_system_setting_info *RPC_VIP_system_info_structure_table=NULL;

	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display,&src_idx);

/*
	SLR_PICTURE_MODE_DATA *pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
	{
		printf("~get picData Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}
	pData->adv_blackCorrecter= level_index;
*/
	g_BLE_TV002_save = level_index;

	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = &m_customVipTable_demo_tv002;

	/*if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}*/

	RPC_VIP_system_info_structure_table = (_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (RPC_VIP_system_info_structure_table == NULL)
	{
		printf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}
	VIP_SrcTiming = (VIP_SOURCE_TIMING)RPC_VIP_system_info_structure_table->VIP_source;

	if(level_index>=VIP_ADV_API_LEVEL_Max)
	{
		API_Level = ADV_API_LEVEL_OFF;
		printf("~level_index>=VIP_ADV_API_LEVEL_Max, level= %d , %s->%d, %s~\n", level_index, __FILE__, __LINE__,__FUNCTION__);

	}
	else
		API_Level = (VIP_ADV_API_LEVEL)level_index;

	table_idx = Scaler_Get_ADV_API_LEVEL_TBL_Idx_TV002(VIP_SrcTiming, API_Level);
	p_Share_Memory_VIP_TABLE_Custom_Struct->ADV_API.ADV_API_CTRL.TBL_Select_BLE = table_idx;
	fwif_color_set_DCC_SC_for_Coef_Change(1,1);

};

unsigned char Scaler_get_LiveColor(void)
{
	return g_LiveColor_save;
}

void Scaler_set_LiveColor(unsigned char level_index)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display,&src_idx);
/*
	SLR_PICTURE_MODE_DATA *pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
	{
		printf("~get picData Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}
	pData->adv_liveColor= level_index;
*/
	g_LiveColor_save = level_index;
	//fwif_color_set_ICM_table(src_idx, level_index);
	if (Scaler_Get_CinemaMode_PQ())
		return;
	fwif_color_set_live_colour(level_index);
}

unsigned char Scaler_get_Clarity_RealityCreation(void)
{
	return g_Clarity_RealityCreation_save;
}

void Scaler_set_Clarity_RealityCreation(unsigned char level)
{
	g_Clarity_RealityCreation_save = level;
}

unsigned char Scaler_get_Clarity_Resolution(void)
{
	return g_Clarity_Resolution_save;
}

void Scaler_set_Clarity_Resolution(unsigned char level)
{
	g_Clarity_Resolution_save = level;
}

unsigned char Scaler_get_Clarity_Mastered4K(void)
{
	return g_Clarity_Masterd4K_save;
}

void Scaler_set_Clarity_Mastered4K(unsigned char level)
{
	g_Clarity_Masterd4K_save = level;
}

unsigned char Scaler_access_Project_TV002_Style(unsigned char isSet_Flag, unsigned char TV002_style_Mode)
{
	static unsigned char tv002_style = Project_TV002_OFF;
	if (isSet_Flag == 1)
		tv002_style = TV002_style_Mode;

	return tv002_style;
}

unsigned char Scaler_get_DBC_blacklight_Value_TV002(void)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *custom_tv002;

	custom_tv002 = Scaler_get_tv002_Custom_struct();

	if (custom_tv002 == NULL) {
		rtd_pr_vpq_emerg("TV002_get_customer_TBL=NULL!!!\n");
		return 0;
	}
#if 0	/* use Zero-D DBC instead of original DBC*/
	if (custom_tv002->DBC.DBC_STATUS.OSD_PowerSave==D_PS_Picture_Off )
		return 1;

	if(Scaler_access_Project_TV002_Style(0, 0) == Project_TV002_OFF)	// picture off
		return 2;

	if(custom_tv002->DBC.DBC_CTRL.DBC_Enable==0)	// picture off
		return 3;

	return custom_tv002->DBC.DBC_STATUS.DBC_Backlight;
#else

	return custom_tv002->ZERO_D.ZERO_D_STATUS.Zero_D_Backlight;

#endif
}

char Scaler_set_DBC_UI_blacklight_Value_TV002(unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table=NULL;
	SLR_VIP_TABLE_CUSTOM_TV002 *VIP_TABLE_CUSTOM_TV002  = NULL;
	VIP_TABLE_CUSTOM_TV002 = Scaler_get_tv002_Custom_struct();
	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (value > 100)
		value = 100;

	if (VIP_TABLE_CUSTOM_TV002 == NULL) {
		rtd_pr_vpq_emerg("TV002_get_customer_TBL=NULL!!!\n");
		return -1;
	}

	VIP_TABLE_CUSTOM_TV002->DBC.DBC_STATUS.OSD_Backlight = value;

	if (VIP_system_info_structure_table == NULL)
	{
		rtd_pr_vpq_emerg("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return -2;
	}

	VIP_system_info_structure_table->OSD_Info.OSD_Backlight = value;
	return 0;
}

char Scaler_set_DBC_POWER_Saving_Mode_TV002(unsigned char value)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *VIP_TABLE_CUSTOM_TV002  = NULL;
	VIP_TABLE_CUSTOM_TV002 = Scaler_get_tv002_Custom_struct();

	if (VIP_TABLE_CUSTOM_TV002 == NULL) {
		rtd_pr_vpq_emerg("TV002_get_customer_TBL=NULL!!!\n");
		return -1;
	}

	if (value > D_PS_MODE_MAX)
		value = 0;

	VIP_TABLE_CUSTOM_TV002->DBC.DBC_STATUS.OSD_PowerSave = value;
	return 0;
}

char Scaler_set_UI_Picture_Mode_TV002(unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table=NULL;
	SLR_VIP_TABLE_CUSTOM_TV002 *VIP_TABLE_CUSTOM_TV002  = NULL;
	VIP_TABLE_CUSTOM_TV002 = Scaler_get_tv002_Custom_struct();
	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
#if 0
	if (VIP_TABLE_CUSTOM_TV002 == NULL) {
		rtd_pr_vpq_emerg("TV002_get_customer_TBL=NULL!!!\n");
		return -1;
	}

	VIP_TABLE_CUSTOM_TV002->DBC.DBC_STATUS.DBC_DBC = value;
#endif
	if (VIP_system_info_structure_table == NULL)
	{
		rtd_pr_vpq_emerg("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return -2;
	}

	VIP_system_info_structure_table->OSD_Info.Picture_mode = value;
	return 0;
}

// for TV002 DBC
#define VIP_Y_HIST_BIN_MAX_TV002 COLOR_HISTOGRAM_LEVEL
#define VIP_Y_HIST_BIN_SCALE_TV002 2
#define VIP_Y_HIST_Ratio_BIN_MAX_TV002 TV006_VPQ_chrm_bin
#define VIP_Y_HIST_Ratio_BIN_SCALE_TV002 8
unsigned short Scaler_scalerVIP_DBC_get_APL_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table, _clues *SmartPic_clue, unsigned int MA_print_count)
{
#if 0
	//=======================use smartfit to get mean value 20121228=====================
	unsigned short Hstart = 0;
	unsigned short Hend = 0;
	unsigned short Vstart = 0;
	unsigned short Vend = 0;

	vgip_vgip_chn1_act_hsta_width_RBUS vgip_chn1_act_hsta_width_reg;
	vgip_vgip_chn1_act_vsta_length_RBUS vgip_chn1_act_vsta_length_reg;

	//find the input position start &end
	vgip_chn1_act_hsta_width_reg.regValue  = rtd_inl(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);
	vgip_chn1_act_vsta_length_reg.regValue = rtd_inl(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);

	Hstart = vgip_chn1_act_hsta_width_reg.ch1_ih_act_sta;
	Hend = vgip_chn1_act_hsta_width_reg.ch1_ih_act_wid;
	Vstart = vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta;
	Vend = vgip_chn1_act_vsta_length_reg.ch1_iv_act_len;

	//unsigned short Hstart=500,Hend=10,Vstart=10,Vend=10;
	smartfit_auto_h_boundary_RBUS auto_h_boundary_reg;
	smartfit_auto_v_boundary_RBUS auto_v_boundary_reg;
	smartfit_auto_r_g_b_margin_RBUS auto_r_g_b_margin_reg;
	smartfit_auto_adj_RBUS auto_adj_reg;
	auto_h_boundary_reg.regValue	 = rtd_inl(SMARTFIT_AUTO_H_BOUNDARY_reg);
	auto_v_boundary_reg.regValue	 = rtd_inl(SMARTFIT_AUTO_V_BOUNDARY_reg);
	auto_r_g_b_margin_reg.regValue	= rtd_inl(SMARTFIT_AUTO_R_G_B_MARGIN_reg);
	auto_adj_reg.regValue				  = rtd_inl(SMARTFIT_AUTO_ADJ_reg);

	//H & V boundary setting
	auto_h_boundary_reg.hb_sta = Hstart;
	auto_h_boundary_reg.hb_end= Hend;
	auto_v_boundary_reg.vb_sta=Vstart;
	auto_v_boundary_reg.vb_end=Vend;

	rtd_outl(SMARTFIT_AUTO_H_BOUNDARY_reg, auto_h_boundary_reg.regValue);
	rtd_outl(SMARTFIT_AUTO_V_BOUNDARY_reg, auto_v_boundary_reg.regValue);

	//Color Source Select for Detection //20070713 SmartFit YUV definition's wrong on ver.FIB !! Notice whether spec. been changed?

	auto_r_g_b_margin_reg.color_sel=0x02;//auto_r_g_b_margin_reg.color_sel=0x1;

	//MIN color measured, Accumulation Mode, Start Auto-Function Tracking Function
	auto_adj_reg.force_flip = 1;
	auto_adj_reg.sum_max	 = 1;
	auto_adj_reg.now_af  = 1;
	auto_adj_reg.diff_en =0;


	smartfit_auto_field_RBUS auto_field_reg;
	smartfit_auto_result_phase_m_RBUS auto_result_phase_m_reg;
	smartfit_auto_result_phase_l_RBUS auto_result_phase_l_reg;
	auto_field_reg.regValue = rtd_inl(SMARTFIT_AUTO_FIELD_reg);

	unsigned long long SUM;
	unsigned  int  PxlNum,Hlength,vlength,PxlNum2;
	unsigned  int  PEAK = 0;

	auto_field_reg.adj_source = 0;//main(channel 1)

	rtd_outl(SMARTFIT_AUTO_FIELD_reg,auto_field_reg.regValue);

	auto_r_g_b_margin_reg.diff_th = 0;
	rtd_outl(SMARTFIT_AUTO_R_G_B_MARGIN_reg,auto_r_g_b_margin_reg.regValue);


	auto_result_phase_m_reg.regValue =	rtd_inl(SMARTFIT_AUTO_RESULT_PHASE_M_reg);
	auto_result_phase_l_reg.regValue =	rtd_inl(SMARTFIT_AUTO_RESULT_PHASE_L_reg);

	SUM = ( ( (unsigned long long)(auto_result_phase_m_reg.acc_41_32) ) <<32) + (unsigned long long)(auto_result_phase_l_reg.acc_31_0);

	smartfit_auto_result_hsta_end_RBUS auto_result_hsta_end_reg;
	smartfit_auto_result_vsta_end_RBUS auto_result_vsta_end_reg;
	auto_result_hsta_end_reg.regValue  = rtd_inl(SMARTFIT_AUTO_RESULT_HSTA_END_reg);
	auto_result_vsta_end_reg.regValue = rtd_inl(SMARTFIT_AUTO_RESULT_VSTA_END_reg);

	Hstart = auto_result_hsta_end_reg.hx_sta;
	Hend = auto_result_hsta_end_reg.hx_end;
	Vstart = auto_result_vsta_end_reg.vx_sta;
	Vend = auto_result_vsta_end_reg.vx_end;

	Hlength=Hend-Hstart+1;
	vlength=Vend-Vstart+1;
	PxlNum2=Hlength*vlength;
	PxlNum = (Hend-Hstart)*(Vend-Vstart);
	PEAK = (SUM)/(PxlNum2); //PEAK value 64~940 TG45:

	rtd_outl(SMARTFIT_AUTO_ADJ_reg, auto_adj_reg.regValue);

	// for Debug info
	unsigned char Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_STATUS.Debug_Log0;
	unsigned char Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_STATUS.Debug_Delay_Flame;
	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT4)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			ROSPrintf("***,Hstart=%d,Hend=%d,Vstart=%d,Vend=%d***\n",Hstart,Hend,Vstart,Vend);
			ROSPrintf("***Hlength=%d,vlength=%d***\n",Hlength,vlength);
			ROSPrintf("***PxlNum=%d ,PxlNum2=%d ,***\n",PxlNum,PxlNum2);
			ROSPrintf("***SUM=%d ***\n",SUM);
			ROSPrintf("***PEAK=%d ,***\n",PEAK);

		}
	}
	//=======================use smartfit to get mean value 20121228=====================
	return (PEAK>>2);
#else
	unsigned char i = 0;
	unsigned int APL = 0;
	unsigned char Debug_Log=0;
	unsigned char Debug_Log_Delay=0;
	for (i=0;i<VIP_Y_HIST_Ratio_BIN_MAX_TV002;i++)
		APL = APL + SmartPic_clue->Y_Main_Hist_Ratio[i] * VIP_Y_HIST_Ratio_BIN_SCALE_TV002 * i;

	APL = APL>>10; /* APL = APL / 1000*/

	// for Debug info
	Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Log0;
	Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Delay_Flame;
	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT4)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			rtd_pr_vpq_info("***APL = %d***\n", APL);
		}
	}
	return (unsigned short)APL;
#endif
}


unsigned char Scaler_scalerVIP_DBC_BASIC_BLIGHT_Cal_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	unsigned char BL_TOP_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.BL_TOP;
	unsigned char BL_MIDDLE_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.BL_MIDDLE;
	unsigned char BL_BOTTOM_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.BL_BOTTOM;
	unsigned char BL_SETUDEN_Reg =vipCustomTBL_ShareMem->DBC.DBC_SW_REG.BL_SETUDEN;
	unsigned char BL_PSH_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.BL_PSH;

	//unsigned char USER_BLIGHT = system_info_structure_table->OSD_Info.OSD_Backlight;
	unsigned char USER_BLIGHT = vipCustomTBL_ShareMem->DBC.DBC_STATUS.OSD_Backlight;
	//unsigned char mode = system_info_structure_table->OSD_Info.OSD_DCR_Mode;
	VIP_DBC_POWER_SAVING_MODE mode = (VIP_DBC_POWER_SAVING_MODE)vipCustomTBL_ShareMem->DBC.DBC_STATUS.OSD_PowerSave;
	unsigned char BASIC_BLIGHT;
	unsigned char Debug_Log=0;
	unsigned char Debug_Log_Delay=0;

	if(USER_BLIGHT > 100)
		USER_BLIGHT = 100;

	switch(mode)
	{
		case D_PS_Off:
		case D_PS_Picture_Off:
		default:
			if(USER_BLIGHT >= 50)
				BASIC_BLIGHT = ((BL_TOP_Reg - BL_MIDDLE_Reg)*(USER_BLIGHT - 50) / 50) + BL_MIDDLE_Reg;
			else
				BASIC_BLIGHT = ((BL_MIDDLE_Reg - BL_BOTTOM_Reg)*USER_BLIGHT / 50) + BL_BOTTOM_Reg;
		break;
		case D_PS_Low:
			if(USER_BLIGHT >= 50)
				BASIC_BLIGHT = (((BL_TOP_Reg - BL_MIDDLE_Reg)*(USER_BLIGHT - 50) / 50) + BL_MIDDLE_Reg);
			else
				BASIC_BLIGHT = (((BL_MIDDLE_Reg - BL_BOTTOM_Reg)*USER_BLIGHT / 50) + BL_BOTTOM_Reg);
			BASIC_BLIGHT = BASIC_BLIGHT - BL_SETUDEN_Reg;
		break;
		case D_PS_High:
			BASIC_BLIGHT = BL_PSH_Reg;
		break;
	}
	vipCustomTBL_ShareMem->DBC.DBC_STATUS.Basic_Backlight = BASIC_BLIGHT;

	// for Debug info
	Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Log0;
	Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Delay_Flame;
	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT1)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			rtd_pr_vpq_info("basic_BL=%d,USER_BLIGHT=%d,DCR_mode=%d\n",
				BASIC_BLIGHT, USER_BLIGHT, mode);
		}
	}

	return BASIC_BLIGHT;
}

#define APL_mapping_limit_range 1	//rock 20131219 smartFIT APL range 16~240
unsigned char Scaler_scalerVIP_DBC_DBC_Cal_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table, _clues *SmartPic_clue, unsigned int MA_print_count)
{
	unsigned char DBC_APL_LOW_START_Reg ;
	unsigned char DBC_BL_B_LIMIT_Reg;
	unsigned char DBC_APL_LOW_LIMIT_Reg ;
	unsigned char DBC_PS_APL_TH_Reg ;
	unsigned char DBC_PS_LEVEL_Reg ;
	unsigned char BASIC_BLIGHT ;
	VIP_DBC_POWER_SAVING_MODE PictureMode ;

	// for Debug info
	unsigned char Debug_Log ;
	unsigned char Debug_Log_Delay ;

	unsigned short APL = 0;

	APL = Scaler_scalerVIP_DBC_get_APL_TV002(vipCustomTBL_ShareMem, system_info_structure_table, SmartPic_clue, MA_print_count);

	if(APL>255)
		APL = 255;
#ifdef APL_mapping_limit_range
	if(APL<16)
		APL = 0;
	else if(APL>240)
		APL = 255;
	else
		APL = ((APL-16)*255 + 112)/(224);	// 240-16
#endif

	vipCustomTBL_ShareMem->DBC.DBC_STATUS.APL_in_SmartFit = APL;
	if (vipCustomTBL_ShareMem->DBC.DBC_CTRL.BL_Debug_APL != 0)
		APL = vipCustomTBL_ShareMem->DBC.DBC_CTRL.BL_Debug_APL;

	DBC_APL_LOW_START_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_APL_LOW_START;
	DBC_BL_B_LIMIT_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_BL_B_LIMIT;
	DBC_APL_LOW_LIMIT_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_APL_LOW_LIMIT;
	DBC_PS_APL_TH_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_PS_APL_TH;
	DBC_PS_LEVEL_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_PS_LEVEL;
	BASIC_BLIGHT = vipCustomTBL_ShareMem->DBC.DBC_STATUS.Basic_Backlight;
	//unsigned char PictureMode = system_info_structure_table->OSD_Info.OSD_DCR_Mode;
	PictureMode = (VIP_DBC_POWER_SAVING_MODE)vipCustomTBL_ShareMem->DBC.DBC_STATUS.OSD_PowerSave;

	// for Debug info
	Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Log0;
	Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Delay_Flame;
	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT3)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			rtd_pr_vpq_info("APLInSmartit=%d,\n",
				APL);
		}
	}

	if(APL >= DBC_APL_LOW_START_Reg)
		return	BASIC_BLIGHT;
	else if(APL >= DBC_APL_LOW_LIMIT_Reg)
	{
		if((DBC_APL_LOW_START_Reg - DBC_APL_LOW_LIMIT_Reg) == 0)
		{
			if(MA_print_count%250==0)
				rtd_pr_vpq_info("DBC SETTING ERROR\n");
			return 0;
		}
		return BASIC_BLIGHT - ((DBC_APL_LOW_START_Reg - APL) * DBC_BL_B_LIMIT_Reg / (DBC_APL_LOW_START_Reg - DBC_APL_LOW_LIMIT_Reg));
	}
	else if(APL >= DBC_PS_APL_TH_Reg)
		return (BASIC_BLIGHT - DBC_BL_B_LIMIT_Reg);
	else
		if(PictureMode == D_PS_High)	//VIP_PICTURE_MODE_DYNAMIC
			return DBC_PS_LEVEL_Reg;
		else
			return (BASIC_BLIGHT - DBC_BL_B_LIMIT_Reg);

}

unsigned char Scaler_scalerVIP_DBC_Y_Max_Cal_TV002(_clues *SmartPic_clue, unsigned int MA_print_count)
{
	unsigned char i;
	unsigned int *reg_HistCnt = &(SmartPic_clue->Y_Main_Hist[0]);
	for(i = (VIP_Y_HIST_BIN_MAX_TV002-1); i > 0 ; i--)
	{
		if(reg_HistCnt[i] >= 10)
			return ((i * VIP_Y_HIST_BIN_SCALE_TV002)-1);
	}
	return 0;
}

unsigned char Scaler_scalerVIP_DBC_BackLight_Cal_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table, _clues *SmartPic_clue, unsigned int MA_print_count)
{
	unsigned char DBC_PS_TIME_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_PS_TIME;
	unsigned char DBC_PS_TM_EXIT_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_PS_TM_EXIT;
	unsigned char DBC_PS_ATN_UP_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_PS_ATN_UP;
	unsigned char DBC_PS_ATN_DN_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_PS_ATN_DN;
	unsigned char DBC_PS_LEVEL_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_PS_LEVEL;
	unsigned char DBC_BL_B_LIMIT_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_BL_B_LIMIT;

	unsigned short backlight;
	static unsigned char status = 1;
	static unsigned short Pre_backlight = 255;
	static unsigned int DBC4PS_count = 0;
	unsigned char BASIC_BLIGHT = vipCustomTBL_ShareMem->DBC.DBC_STATUS.Basic_Backlight;
	static unsigned char pre_basic_BL = 0;
	unsigned char dcc_Mode = system_info_structure_table->OSD_Info.OSD_DCC_Mode;

	unsigned short DBC = Scaler_scalerVIP_DBC_DBC_Cal_TV002(vipCustomTBL_ShareMem, system_info_structure_table, SmartPic_clue, MA_print_count);
	unsigned short Y_MAX = Scaler_scalerVIP_DBC_Y_Max_Cal_TV002(SmartPic_clue, MA_print_count)*9/10;

	unsigned char Debug_Log ;
	unsigned char Debug_Log_Delay;

	vipCustomTBL_ShareMem->DBC.DBC_STATUS.Y_MAX = Y_MAX;
	vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DBC = DBC;

	if (vipCustomTBL_ShareMem->DBC.DBC_CTRL.BL_Debug_ENA != 0)
		Y_MAX = vipCustomTBL_ShareMem->DBC.DBC_CTRL.BL_Debug_ENA;

	backlight = MAX(DBC, Y_MAX);

	if(BASIC_BLIGHT < backlight)
		backlight = BASIC_BLIGHT;

	// for Debug info
	Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Log0;
	Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Delay_Flame;
	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT2)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			rtd_pr_vpq_info("dccMode=%d,pre_basic_BL=%d,BASIC_BLIGHT=%d,DBC=%d,YMax=%d\n",
				dcc_Mode, pre_basic_BL, BASIC_BLIGHT,DBC,Y_MAX);
		}
	}
/*
	// basic backlight change from user
	if(pre_basic_BL != BASIC_BLIGHT)
	{
		Pre_backlight = backlight;
		pre_basic_BL = BASIC_BLIGHT;
		return Pre_backlight;
	}

	if(dcc_Mode == 0)
	{
		Pre_backlight = BASIC_BLIGHT;
		return BASIC_BLIGHT;
	}
*/
	if(backlight > Pre_backlight)				//Addition PWM
	{
		if(Pre_backlight >= (BASIC_BLIGHT - DBC_BL_B_LIMIT_Reg))	//not Dark Y
		{
			Pre_backlight = Pre_backlight + 1;
			status = 1;
		}
		else														//Dark Y
		{
			if(status == 0)
			{
				DBC4PS_count++;
				if(DBC4PS_count > DBC_PS_TM_EXIT_Reg)
				{
					DBC4PS_count = 0;
					status = 1;
				}
			}
			else
			{
				if((Pre_backlight + DBC_PS_ATN_UP_Reg) > (BASIC_BLIGHT - DBC_BL_B_LIMIT_Reg))
					Pre_backlight = (BASIC_BLIGHT - DBC_BL_B_LIMIT_Reg);
				else
					Pre_backlight = (Pre_backlight + DBC_PS_ATN_UP_Reg);
			}
		}
	}
	else	 if(backlight < Pre_backlight)		//Reduction PWM
	{
		if(Pre_backlight >= (BASIC_BLIGHT - DBC_BL_B_LIMIT_Reg))	//not Dark Y
		{
			Pre_backlight = Pre_backlight - 1;
			status = 1;
		}
		else														//Dark Y
		{
			if(status == 1)
			{
				DBC4PS_count++;
				if(DBC4PS_count > (DBC_PS_TIME_Reg<<2))
				{
					DBC4PS_count = 0;
					status = 0;
				}
			}
			else
			{
				if(Pre_backlight > DBC_PS_ATN_DN_Reg)
				{
					if((Pre_backlight - DBC_PS_ATN_DN_Reg) < DBC_PS_LEVEL_Reg)
						Pre_backlight = DBC_PS_LEVEL_Reg;
					else
						Pre_backlight = (Pre_backlight - DBC_PS_ATN_DN_Reg);
				}
				else
					Pre_backlight = DBC_PS_LEVEL_Reg;
			}
		}
	}
	return Pre_backlight;
}
extern char rtk_set_PWM_TV002(unsigned char duty);
//unsigned char setDBC_inKernel = 0;
void Scaler_scalerVIP_DBC_CTRL_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table, _clues *SmartPic_clue, unsigned int MA_print_count)
{
#if 0	/* use Zero-D DBC instead od original DBC*/
	unsigned short backlight;
	//unsigned char m_isIncreaseMode = system_info_structure_table->OSD_Info.isIncreaseMode_Flag;
	//unsigned int m_PIN_BL_ADJ = system_info_structure_table->OSD_Info.backlight_pin_adj;

	//if(system_info_structure_table->OSD_Info.OSD_DCR_Mode==3)	// picture off
		//return;
	if(vipCustomTBL_ShareMem->DBC.DBC_CTRL.DBC_Enable == 0)
		return;

	if((VIP_DBC_POWER_SAVING_MODE)vipCustomTBL_ShareMem->DBC.DBC_STATUS.OSD_PowerSave == D_PS_Picture_Off)	// picture off
		return;

	Scaler_scalerVIP_DBC_BASIC_BLIGHT_Cal_TV002(vipCustomTBL_ShareMem, system_info_structure_table, MA_print_count);
	backlight = Scaler_scalerVIP_DBC_BackLight_Cal_TV002(vipCustomTBL_ShareMem, system_info_structure_table, SmartPic_clue, MA_print_count);
	vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_Backlight = backlight;
#if 0
	if(m_isIncreaseMode == 0)
	{
		if(backlight > 0xFF)
			backlight = 0;
		else
			backlight = 0xFF - backlight;
	}
	//scalerVIP_IoGpio_SetPwmDuty(m_PIN_BL_ADJ, backlight);
#endif
	// for Debug info
	unsigned char Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Log0;
	unsigned char Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Delay_Flame;
	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT0)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			rtd_pr_vpq_info("backlight=%d\n",
				backlight);
		}
	}
#else
	unsigned char OSD_BL, Zero_D_BL;
	unsigned char Debug_Log;
	unsigned char Debug_Log_Delay;

	OSD_BL = system_info_structure_table->OSD_Info.OSD_Backlight;
	Zero_D_BL = OSD_BL * 255 /100;

	if (vipCustomTBL_ShareMem->ZERO_D.ZERO_D_CTRL.BL_Debug_ENA == 1)/* for debug, force Zero-D BL output*/
		vipCustomTBL_ShareMem->ZERO_D.ZERO_D_STATUS.Zero_D_Backlight = vipCustomTBL_ShareMem->ZERO_D.ZERO_D_CTRL.BL_Debug_APL;
	else/* record for AP*/
		vipCustomTBL_ShareMem->ZERO_D.ZERO_D_STATUS.Zero_D_Backlight = Zero_D_BL;

#if 1	/* for debug*/
	Debug_Log = vipCustomTBL_ShareMem->ZERO_D.ZERO_D_CTRL.Debug_Log0;
	Debug_Log_Delay = vipCustomTBL_ShareMem->ZERO_D.ZERO_D_CTRL.Debug_Delay_Flame;
	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT0)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			rtd_pr_vpq_info("Zero-D, OSD_BL=%d, Zero_D_BL=%d\n",
				OSD_BL, Zero_D_BL);
		}
	}

#endif
#endif
}

unsigned short Scaler_scalerVIP_Brightness_Plus_Cal_OverSat_Protect_Gain_TV002(unsigned short BP_Gain, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int boundary = 235; // 16-235
	unsigned char weightMax=8, weightMax_bit=3;

	unsigned short table_sel;
	unsigned short tol;
	unsigned short th;
	unsigned short stepBit;
	unsigned short BP_En = 0;
	unsigned short en;
	unsigned short shiftBit;

	unsigned int Gain_Value = BP_Gain;
	unsigned int Final_LimitGain = Gain_Value;
	unsigned int allowedY, allowedGain, tmp;
	unsigned short weight=0, i=0;
	unsigned short bin_ignore_th;
	unsigned short debug_Log;
	unsigned short debug_Log_delay;
	unsigned int *Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;

	if(vipCustomTBL_ShareMem==NULL)
	{
		if(MA_print_count%250==0)
			rtd_pr_vpq_emerg("Customer TBL is NULL, return\n");
		return BP_Gain;
	}

	table_sel = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	tol = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.OverSat_Tol;
	th = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.OverSat_Th;
	stepBit = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.OverSat_StepBit;
	BP_En = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;
	en = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.Enable;
	shiftBit = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_Shift_Bit;
	debug_Log = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	debug_Log_delay = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	bin_ignore_th = th;

	if(debug_Log_delay==0)
		debug_Log_delay=1;

	if(en==0 || Gain_Value<=(1<<shiftBit) || BP_En==0)
		return BP_Gain;

	allowedY = (boundary*tol)>>shiftBit;

	for(i=0;i<(VIP_Y_HIST_Ratio_BIN_MAX_TV002);i++)
	{
		tmp = (VIP_Y_HIST_Ratio_BIN_MAX_TV002-i)*VIP_Y_HIST_Ratio_BIN_SCALE_TV002-1;
		if(allowedY<=tmp)
			allowedGain=(1<<shiftBit);
		else
			allowedGain = (allowedY<<shiftBit)/tmp;

		if(allowedGain>=Gain_Value)
			tmp = Gain_Value;
		else
		{

			if((Y_hist_ratio[VIP_Y_HIST_Ratio_BIN_MAX_TV002-i-1])>=bin_ignore_th)
				weight = 0;
			else if(stepBit>=10) // No Blending
				weight = weightMax;
			else
			{
				weight = (bin_ignore_th - (Y_hist_ratio[VIP_Y_HIST_Ratio_BIN_MAX_TV002-i-1])+((1<<stepBit)-1))>>stepBit;
				if(weight > weightMax)
					weight = weightMax;
			}
			tmp = (allowedGain*(weightMax - weight) + Gain_Value * weight)>>weightMax_bit;
		}
		//select min gain
		if(tmp<Final_LimitGain)
			Final_LimitGain = tmp;

		if((debug_Log&_BIT2)!=0)
		{
			if(MA_print_count%debug_Log_delay==0)
				rtd_pr_vpq_info("=%d,%d,%d,%d,%d=\n",allowedGain, tmp, Final_LimitGain, weight, Gain_Value);
		}
	}

	/* recorde status*/
	 vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain_Limit_Allowed = allowedGain;
	 vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain_Limited = Final_LimitGain;
	  vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain_Limit_Weight = weight;

	if((debug_Log&_BIT1)!=0)
	{
		if(MA_print_count%debug_Log_delay==0)
			rtd_pr_vpq_info("LC=%d,FC=%d\n",allowedY,Final_LimitGain);
	}

	return Final_LimitGain;
}

unsigned short Scaler_scalerVIP_Brightness_Plus_Smooth_Gain_Cal_TV002(unsigned short BP_Gain, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, unsigned int MA_print_count)
{
	unsigned short table_sel;
	unsigned short ini;
	unsigned short step;
	unsigned short delayF;
	unsigned short Enable;
	static unsigned short pre_Gain = 0;
	static unsigned short counter=0;
	unsigned short debug_Log;
	unsigned short debug_Log_delay;
	unsigned short BP_En = 0;

	if(vipCustomTBL_ShareMem==NULL)
	{
		if(MA_print_count%250==0)
			rtd_pr_vpq_emerg("Customer TBL is NULL, return\n");
		return BP_Gain;
	}

	table_sel = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	ini = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Init_Flag;
	step = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Step;
	delayF = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Delay_Flame;
	Enable = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Enable;
	debug_Log = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	debug_Log_delay = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	BP_En = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;

	if(debug_Log_delay==0)
		debug_Log_delay=1;

	if(step==0 || Enable==0 || BP_En==0)
		return BP_Gain;

	// bit0 for BP gain initialize, bit1 for offset initialize
	if(((ini&_BIT0)>>0)==1)
	{
		ini = ini&(~_BIT0);
		vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Init_Flag=ini;
		pre_Gain = BP_Gain;
		counter = 0;
		return BP_Gain;
	}

	if(counter > delayF)
	{
		if(BP_Gain>(pre_Gain+step))
			pre_Gain = pre_Gain + step;
		else if(BP_Gain<(pre_Gain-step))
			pre_Gain = pre_Gain - step;
		else
			pre_Gain = BP_Gain;
		counter = 0;

		//bit4 for smooth log
		if((debug_Log&_BIT4)!=0)
		{
			if(MA_print_count%debug_Log_delay==0)
				rtd_pr_vpq_info("s=%d,%d\n",pre_Gain, BP_Gain);
		}
	}

	vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain_Smooth = pre_Gain;

	counter++;
	return pre_Gain;
}

short Scaler_scalerVIP_Brightness_Plus_Smooth_Offset_Cal_TV002(short RGB_Offset, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, unsigned int MA_print_count)
{
	unsigned short table_sel;
	unsigned short ini;
	unsigned short step;
	unsigned short delayF;
	unsigned short Enable;
	static short pre_offset = 0;
	static unsigned short counter=0;
	//short pre_offset_tmp=0;
	unsigned short debug_Log;
	unsigned short debug_Log_delay;
	unsigned short BP_En;

	if(vipCustomTBL_ShareMem==NULL)
	{
		if(MA_print_count%250==0)
			rtd_pr_vpq_emerg("Customer TBL is NULL, return\n");
		return RGB_Offset;
	}

	table_sel = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	ini = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Init_Flag;
	step = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Step_offset;
	delayF = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Delay_Flame_offset;
	Enable = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Enable;
	debug_Log = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	debug_Log_delay = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	BP_En = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;

	if(debug_Log_delay==0)
		debug_Log_delay=1;

	if(step==0 || Enable==0 || BP_En==0)
		return RGB_Offset;

	// bit0 for BP gain initialize, bit1 for offset initialize
	if(((ini&_BIT1)>>1)==1)
	{
		ini = ini&(~_BIT1);
		vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Init_Flag=ini;
		pre_offset = RGB_Offset;
		counter = 0;
		return RGB_Offset;
	}

	if(counter > delayF)
	{
		if(RGB_Offset>(pre_offset+step))
			pre_offset = pre_offset + step;
		else if(RGB_Offset<(pre_offset-step))
			pre_offset = pre_offset - step;
		else
			pre_offset = RGB_Offset;
		counter = 0;

		//bit5 for smooth log
		if((debug_Log&_BIT5)!=0)
		{
			if(MA_print_count%debug_Log_delay==0)
				rtd_pr_vpq_info("o=%d,%d\n",pre_offset, RGB_Offset);
		}
	}

	vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_CMPS_Smooth = pre_offset;

	counter++;
	return pre_offset;
}


short Scaler_scalerVIP_Brightness_Plus_Compensation_RGB_Offset_Cal_TV002(unsigned short BP_Gain, unsigned short APL, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* sysInfo_TBL, unsigned int MA_print_count)
{
	unsigned short table_sel;
	unsigned short BP_En;
	unsigned short en;
	//unsigned char APL = Scaler_GetAutoMA(BP_APL);
	unsigned short debug_Log;
	unsigned short debug_Log_delay;
	unsigned short offset_Gain;
	unsigned short G1=0;
	unsigned short G2=0;
	unsigned short P1=0;
	unsigned short P2=0;
	short cmps=0;
	unsigned short shiftBit;
	unsigned short i;

	if(vipCustomTBL_ShareMem==NULL)
	{
		if(MA_print_count%250==0)
			rtd_pr_vpq_emerg("Customer TBL is NULL, return\n");
		return 0;
	}

	table_sel = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	BP_En = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;
	en = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Enable;
	debug_Log = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	debug_Log_delay = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	shiftBit = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_Shift_Bit;

	if(debug_Log_delay==0)
		debug_Log_delay=1;

	vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Cmps_Point[BP_SEG_0] = 0;  // fix as o
	vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Cmps_Point[BP_SEG_MAX-1] = 255;	// fix as 255;

	// turn 0ff all sub function while BP is off
	if(BP_En==0)
		en=0;

	cmps=0;
	if(en==1)
	{
		if(APL==vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Cmps_Point[BP_SEG_0])
			offset_Gain = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Cmps_Gain[BP_SEG_0];
		else if(APL==vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Cmps_Point[BP_SEG_MAX-1])
			offset_Gain = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Cmps_Gain[BP_SEG_MAX-1];
		else
		{
			for(i=BP_SEG_1; i<BP_SEG_MAX; i++)
			{
				if(APL<=vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Cmps_Point[i])
				{
					P1 = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Cmps_Point[i-1];
					P2 = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Cmps_Point[i];
					G1 = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Cmps_Gain[i-1];
					G2 = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Cmps_Gain[i];
					break;
				}
			}
			if(P2==P1)
				offset_Gain = G1;
			else
				offset_Gain = G1 + ((G2-G1) * (APL-P1) / (P2-P1));
		}

		// negative error
		if(offset_Gain>(1<<(shiftBit+2)))
			offset_Gain=0;

		// for precision issue and YUV2RGB out is 14 bit, using 14 bit(1024-15040) to calculate compensation instead of 8 bit(16-235)
		cmps = 1024-((1024*BP_Gain)>>shiftBit); // BP_Gain and offset_Gain is multiply by shiftBit
		cmps = (cmps*offset_Gain)>>shiftBit;

		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_CMPS = cmps;

		// debug
		if((debug_Log&_BIT3)!=0)
		{
			if(MA_print_count%debug_Log_delay==0)
			{
				rtd_pr_vpq_info("BP_CMPs=G1=%d,G2=%d,P1=%d,P2=%d,G=%d,", G1, G2, P1, P2, offset_Gain);
				rtd_pr_vpq_info("cmps=%d,BP_Gain=%d\n", cmps, BP_Gain);
			}
		}
	}
	return cmps;
}

void Scaler_scalerVIP_Brightness_Plus_Function_TV002(SLR_VIP_TABLE* vipTBL_shareMem, _clues* SmartPic_clue, _system_setting_info* sysInfo_TBL, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, unsigned int MA_print_count)
{
	unsigned char i = 0;
	unsigned char APL = 0;
//	unsigned char cur_Gain_Value = 100;
	unsigned short G1 = 0; // Scaler_GetAutoMA(BP_G1);
	unsigned short G2 = 0; // Scaler_GetAutoMA(BP_G2);
	unsigned short P1 = 0; // Scaler_GetAutoMA(BP_P1);
	unsigned short P2 = 0; // Scaler_GetAutoMA(BP_P2);
	unsigned short tmp;
	unsigned int Gain_Value = 128;
	unsigned int DC_average = 0;
	unsigned int Total_HistCnt = 0;
	unsigned short debug_Log;
	unsigned short debug_Log_delay;
	short RGB_offset_cmps;
	unsigned int *reg_HistCnt;
	unsigned short Debug_EN;
	unsigned short shiftBit;
	unsigned short table_sel;
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	unsigned short BP_En;
	short k11,k12,k13,k22,k23,k32,k33;

	short R,G,B;

	if(vipTBL_shareMem==NULL || SmartPic_clue==NULL || sysInfo_TBL==NULL || vipCustomTBL_ShareMem==NULL)
	{
		if(MA_print_count%250==0)
		{
			/*rtd_pr_vpq_emerg("Mem Null, return, VIP TBL=%x, smartPic=%x, sysInfo=%x, VIP_cus=%x\n",
				vipTBL_shareMem, SmartPic_clue, sysInfo_TBL, vipCustomTBL_ShareMem);*/
		}
		return;
	}

	debug_Log = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	debug_Log_delay = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	reg_HistCnt = SmartPic_clue->Y_Main_Hist;
	Debug_EN = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Enable;
	shiftBit = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_Shift_Bit;
	table_sel = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	BP_En = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;

	if(debug_Log_delay==0)
		debug_Log_delay=1;

	for(i=0;i<(VIP_Y_HIST_BIN_MAX_TV002);i++)
	{
		DC_average+= reg_HistCnt[i]*( i*VIP_Y_HIST_BIN_SCALE_TV002);
		Total_HistCnt+= reg_HistCnt[i];
	}

	if(Total_HistCnt == 0 /*|| Max_Bright_bin == 0*/)
		return;

	/*APL = ((DC_average/Total_HistCnt)<<5)/(Max_Bright_bin);*/
	APL = DC_average/Total_HistCnt;

	vipCustomTBL_ShareMem->BP_Function.BP_STATUS.APL = APL;
	if(Debug_EN == 1)
		APL = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_APL;

	// mode 1 fix p1=0, p5=255
	vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Point[BP_SEG_0] = 0;
	vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Point[BP_SEG_MAX-1] = 255;

	if(BP_En==1)
	{
		if(APL==vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Point[BP_SEG_0])
			Gain_Value = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Gain[BP_SEG_0];
		else if(APL==vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Point[BP_SEG_MAX-1])
			Gain_Value = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Gain[BP_SEG_MAX-1];
		else
		{
			for(i=BP_SEG_1; i<BP_SEG_MAX; i++)
			{
				if(APL<=vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Point[i])
				{
					P1 = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Point[i-1];
					P2 = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Point[i];
					G1 = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Gain[i-1];
					G2 = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Gain[i];
					break;
				}
			}
			if(P2==P1)
				Gain_Value = G1;
			else
				Gain_Value = G1 + ((G2-G1) * (APL-P1) / (P2-P1));
		}

		if((debug_Log&_BIT0)!=0)
		{
			if(MA_print_count%debug_Log_delay==0)
			{
				tmp = (P1==P2)?1:(P2-P1);
				rtd_pr_vpq_info("BP_mode1=G1=%d,G2=%d,P1=%d,P2=%d,G=%d, test=%d,%d, APL=%d, i=%d\n",
					G1, G2, P1, P2, Gain_Value, ((G2-G1) * (APL-P1) / tmp), ((G1-G2) * (APL-P1) / tmp), APL, i);
			}
		}

		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain = Gain_Value;

		// negative error
		if(Gain_Value>(1<<(shiftBit+2)))
			Gain_Value=0;

		// avoid over saturate
		Gain_Value =  Scaler_scalerVIP_Brightness_Plus_Cal_OverSat_Protect_Gain_TV002(Gain_Value, vipCustomTBL_ShareMem, SmartPic_clue, MA_print_count);

		// compensation
		RGB_offset_cmps = Scaler_scalerVIP_Brightness_Plus_Compensation_RGB_Offset_Cal_TV002(Gain_Value, APL, vipCustomTBL_ShareMem, sysInfo_TBL, MA_print_count);

		// BP gain smooth
		Gain_Value = Scaler_scalerVIP_Brightness_Plus_Smooth_Gain_Cal_TV002(Gain_Value, vipCustomTBL_ShareMem, MA_print_count);

		// offset smooth
		RGB_offset_cmps = Scaler_scalerVIP_Brightness_Plus_Smooth_Offset_Cal_TV002(RGB_offset_cmps, vipCustomTBL_ShareMem, MA_print_count);

	}
	else
	{
		Gain_Value = (1<<shiftBit);
		RGB_offset_cmps = 0;
		//turn off all sub function
		//vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Enable=0;
		//vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.Enable=0;
		//vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Enable=0;
		if((debug_Log&_BIT0)!=0)
		{
			if(MA_print_count%debug_Log_delay==0)
			{
				rtd_pr_vpq_info("BP_OFF, Gain = %d, RGB_offset_cmps = %d\n", Gain_Value, RGB_offset_cmps);
			}
		}

	}

	/* set driver*/
	for(i=0;i<VIP_YUV2RGB_Y_Seg_Max;i++)
	{
		k11 = sysInfo_TBL->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[i];
		k12 = sysInfo_TBL->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i];
		k13 = sysInfo_TBL->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i];
		k22 = sysInfo_TBL->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i];
		k23 = sysInfo_TBL->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i];
		k32 = sysInfo_TBL->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i];
		k33 = sysInfo_TBL->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i];

		if(k11>VIP_YUV2RGB_K_Y_NEG_BND)
			k11 = VIP_YUV2RGB_K_Y_NEG_CHANGE(k11);	//neg

		if(k12>VIP_YUV2RGB_K_C_NEG_BND)
			k12 = VIP_YUV2RGB_K_C_NEG_CHANGE(k12);	//neg

		if(k13>VIP_YUV2RGB_K_C_NEG_BND)
			k13 = VIP_YUV2RGB_K_C_NEG_CHANGE(k13);	//neg

		if(k22>VIP_YUV2RGB_K_C_NEG_BND)
			k22 = VIP_YUV2RGB_K_C_NEG_CHANGE(k22);	//neg

		if(k23>VIP_YUV2RGB_K_C_NEG_BND)
			k23 = VIP_YUV2RGB_K_C_NEG_CHANGE(k23);	//neg

		if(k32>VIP_YUV2RGB_K_C_NEG_BND)
			k32 = VIP_YUV2RGB_K_C_NEG_CHANGE(k32);	//neg

		if(k33>VIP_YUV2RGB_K_C_NEG_BND)
			k33 = VIP_YUV2RGB_K_C_NEG_CHANGE(k33);	//neg

		drv_vipCSMatrix_t.COEF_By_Y.K11[i] = (k11*Gain_Value)>>shiftBit;
		drv_vipCSMatrix_t.COEF_By_Y.K12[i] = (k12*Gain_Value)>>shiftBit;
		drv_vipCSMatrix_t.COEF_By_Y.K13[i] = (k13*Gain_Value)>>shiftBit;
		drv_vipCSMatrix_t.COEF_By_Y.K22[i] = (k22*Gain_Value)>>shiftBit;
		drv_vipCSMatrix_t.COEF_By_Y.K23[i] = (k23*Gain_Value)>>shiftBit;
		drv_vipCSMatrix_t.COEF_By_Y.K32[i] = (k32*Gain_Value)>>shiftBit;
		drv_vipCSMatrix_t.COEF_By_Y.K33[i] = (k33*Gain_Value)>>shiftBit;
	}

	R = sysInfo_TBL->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.R_offset;
	G = sysInfo_TBL->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.G_offset;
	B = sysInfo_TBL->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.B_offset;

	//negative
	if(R>VIP_YUV2RGB_OFFSET_NEG_BND)
		R = VIP_YUV2RGB_OFFSET_NEG_CHANGE(R);
	if(G>VIP_YUV2RGB_OFFSET_NEG_BND)
		G = VIP_YUV2RGB_OFFSET_NEG_CHANGE(G);
	if(B>VIP_YUV2RGB_OFFSET_NEG_BND)
		B = VIP_YUV2RGB_OFFSET_NEG_CHANGE(B);

	R = R + RGB_offset_cmps;
	B = B + RGB_offset_cmps;
	G = G + RGB_offset_cmps;

	drv_vipCSMatrix_t.RGB_Offset.R_offset = R;
	drv_vipCSMatrix_t.RGB_Offset.G_offset = G;
	drv_vipCSMatrix_t.RGB_Offset.B_offset = B;

	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);

	//drvif_color_set_BrightnessPlus_YUV2RGB_CSMatrix(&drv_vipCSMatrix_t);
}

unsigned short Scaler_scalerVIP_ratio_inner_product_base_TV002(unsigned int *vector1, unsigned int *vector2, unsigned char length)
{
	extern unsigned int DCCsqrt_table[4096];
	unsigned int inner_product=0;
	unsigned char i;
	for(i=0;i<length;i++)
		inner_product += DCCsqrt_table[*(vector1+i)] * DCCsqrt_table[*(vector2+i)];
	return (inner_product/1000)>>10;// permillage = 1000;
}

unsigned int Scaler_scalerVIP_ID_radio_caculator_TV002(unsigned char y_max_idx, unsigned int *Y_hist_ratio, _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, unsigned int MA_print_count)
{
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned char con_0 = 0, con_1 = 0, con_2 = 0, con_3 = 0, con_4 = 0;
	unsigned char result = 0;
	unsigned int print_flag ;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		con_0 = (y_max_idx <= 7)?1:0;
		con_1 = (y_max_idx >= 5)?1:0;
		con_2 = ((Y_hist_ratio[y_max_idx]+Y_hist_ratio[y_max_idx-2]+Y_hist_ratio[y_max_idx-1])>890)?1:0;
		con_3 = (((Y_hist_ratio[y_max_idx])>>2)>(Y_hist_ratio[y_max_idx-1]))?1:0;
		con_4 = (((Y_hist_ratio[y_max_idx])>>2)>(Y_hist_ratio[y_max_idx-2]))?1:0;

		result = con_0 + con_1 + con_2 + con_3 + con_4;

		if(result >= 5)
			ID_TV002_Flag[ID_Radio_TV002] = 1;
		else
			ID_TV002_Flag[ID_Radio_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_Radio_TV002] = 0xFF;
	}

	/* debug message*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("sr,%d,con0=%d,%d,%d,%d,%d,\n",y_max_idx, con_0, con_1, con_2, con_3, con_4);
	}

	return ID_TV002_Flag[ID_Radio_TV002];
}

unsigned int Scaler_scalerVIP_ID_FaceArtifactNoise_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 920, s_th1 = 960;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,1,1,2,4,5,8,10,12,15,19,22,25,24,23,24,68,271,279,123,18,11,8,5,3,5,3,0,0,0,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={0,36,50,384,475,43,4,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={705,73,137,78,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_FaceArtifactNoise_TV002] = 1;
		else
			ID_TV002_Flag[ID_FaceArtifactNoise_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_FaceArtifactNoise_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("afn,%d,%d,%d\n",y_ratio1, h_ratio1, s_ratio1);
	}

	return ID_TV002_Flag[ID_FaceArtifactNoise_TV002];
}

unsigned int Scaler_scalerVIP_ID_Rain_0_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 900, s_th1 = 900;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,0,2,2,3,1,1,13,47,112,179,168,124,91,78,47,28,18,12,10,8,6,5,5,5,5,5,4,2,2,1,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={0,18,253,501,191,29,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={708,273,14,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_Raining_0_TV002] = 1;
		else
			ID_TV002_Flag[ID_Raining_0_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_Raining_0_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("rn0,%d,%d,%d\n",y_ratio1, h_ratio1, s_ratio1);
	}

	return ID_TV002_Flag[ID_Raining_0_TV002];
}


unsigned int Scaler_scalerVIP_ID_Rain_1_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 900, s_th1 = 900;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,0,0,0,0,1,13,46,84,96,100,104,106,137,162,101,33,7,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={2,16,19,49,81,65,72,48,42,39,30,65,108,98,75,143,19,3,7,1,1,1,0,2,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={909,87,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_Raining_1_TV002] = 1;
		else
			ID_TV002_Flag[ID_Raining_1_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_Raining_1_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("rn1,%d,%d,%d\n",y_ratio1, h_ratio1, s_ratio1);
	}

	return ID_TV002_Flag[ID_Raining_1_TV002];
}

unsigned int Scaler_scalerVIP_ID_Snow_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 900, s_th1 = 900;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,78,419,260,13,14,75,123,0,0,0,0,0,0,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,973,26,0,0,0,0,0,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={3,838,158,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_Snow_TV002] = 1;
		else
			ID_TV002_Flag[ID_Snow_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_Snow_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("sn,%d,%d,%d\n",y_ratio1, h_ratio1, s_ratio1);
	}

	return ID_TV002_Flag[ID_Snow_TV002];
}

unsigned int Scaler_scalerVIP_ID_BlueArcBridge_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 900, s_th1 = 900;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,215,408,318,42,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={6,37,12,24,17,4,7,7,11,9,13,12,13,52,45,368,176,48,56,13,18,21,12,8,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={875,122,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag;
	unsigned int print_delay;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_BlueArcBridge_TV002] = 1;
		else
			ID_TV002_Flag[ID_BlueArcBridge_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_BlueArcBridge_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("bdb,%d,%d,%d\n",y_ratio1, h_ratio1, s_ratio1);
	}

	return ID_TV002_Flag[ID_BlueArcBridge_TV002];
}

unsigned int Scaler_scalerVIP_ID_DarkScene_Road_Lamp_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 900, s_th1 = 900;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,2,200,363,280,107,17,7,4,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={66,306,563,39,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,14,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={245,460,216,52,9,4,3,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_DarkScene_Road_Lamp_TV002] = 1;
		else
			ID_TV002_Flag[ID_DarkScene_Road_Lamp_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_DarkScene_Road_Lamp_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("drl,%d,%d,%d\n",y_ratio1, h_ratio1, s_ratio1);
	}

	return ID_TV002_Flag[ID_DarkScene_Road_Lamp_TV002];
}

unsigned int Scaler_scalerVIP_ID_MonoScope_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 800, s_th1 = 900;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={29,14,13,4,5,16,8,13,6,10,12,5,12,8,13,9,8,15,560,11,3,3,9,6,195,1,0,0,0,0,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,535,463,0,2,0,0,0,0,0,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={983,1,4,3,2,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_MonoScope_TV002] = 1;
		else
			ID_TV002_Flag[ID_MonoScope_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_MonoScope_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	 print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("mn,%d,%d,%d\n",y_ratio1, h_ratio1, s_ratio1);
	}

	return ID_TV002_Flag[ID_MonoScope_TV002];
}


unsigned int Scaler_scalerVIP_ID_Tunnel_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 920, s_th1 = 920;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,15,18,117,209,161,115,95,73,57,45,40,23,5,1,1,1,0,2,1,3,2,0,0,0,0,2,0,0,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={0,1,1,4,43,135,491,42,14,7,3,3,3,4,5,9,34,62,93,27,2,1,2,1,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={922,72,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_Tunnel_TV002] = 1;
		else
			ID_TV002_Flag[ID_Tunnel_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_Tunnel_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("tl,%d,%d,%d\n",y_ratio1, h_ratio1, s_ratio1);
	}

	return ID_TV002_Flag[ID_Tunnel_TV002];
}

unsigned int Scaler_scalerVIP_ID_RockMountain_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 920, s_th1 = 920;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,1,2,11,36,37,54,71,80,87,86,83,75,64,51,40,28,18,12,8,6,4,2,1,1,1,1,4,7,111,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={0,8,3,17,123,193,286,112,32,16,9,6,5,14,19,66,58,8,9,1,1,1,1,1,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={542,274,142,35,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_RockMountain_TV002] = 1;
		else
			ID_TV002_Flag[ID_RockMountain_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_RockMountain_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("rom,%d,%d,%d\n",y_ratio1, h_ratio1, s_ratio1);
	}

	return ID_TV002_Flag[ID_RockMountain_TV002];
}

unsigned int Scaler_scalerVIP_ID_QtechBamboo_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 950, h_th1 = 950, s_th1 = 950;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={1,1,2,5,13,29,61,84,90,91,87,79,73,65,55,43,34,26,21,16,13,11,10,8,7,7,7,7,11,14,6,6,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={0,1,0,1,3,14,294,306,146,80,49,31,11,19,5,7,6,4,7,2,1,0,0,0,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={371,258,168,139,50,10,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_QtechBamboo_TV002] = 1;
		else
			ID_TV002_Flag[ID_QtechBamboo_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_QtechBamboo_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("qbb,%d,%d,%d\n",y_ratio1, h_ratio1, s_ratio1);
	}

	return ID_TV002_Flag[ID_QtechBamboo_TV002];
}

unsigned int Scaler_scalerVIP_ID_ClockTower_SD_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	extern unsigned char tower_flag;

	unsigned int y_th1 = 950, h_th1 = 950, s_th1 = 950;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,0,0,0,3,3,3,4,4,3,3,3,4,881,61,1,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,985,10,2,1,0,0,0,0,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={28,17,7,10,935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_480I) || (which_source == VIP_QUALITY_HDMI_480P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1) && (tower_flag == 1))
			ID_TV002_Flag[ID_MovingClockTower_TV002] = 1;
		else {
			if (RPC_SmartPic_clue->SceneChange == 1)
				ID_TV002_Flag[ID_MovingClockTower_TV002] = 0;
		}

	} else {
		ID_TV002_Flag[ID_MovingClockTower_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("ct,%d,%d,%d,sc=%d, tower=%d\n",y_ratio1, h_ratio1, s_ratio1, RPC_SmartPic_clue->SceneChange, tower_flag);
	}

	return ID_TV002_Flag[ID_MovingClockTower_TV002];
}

unsigned int Scaler_scalerVIP_ID_waterfallStair_SD_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 950, h_th1 = 950, s_th1 = 950;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,2,36,58,52,57,66,76,79,83,63,59,47,35,30,32,30,26,34,24,17,17,15,13,12,10,7,4,2,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={3,7,16,201,486,44,54,44,37,22,15,11,7,4,3,7,23,2,3,0,0,0,0,0,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={346,509,138,4,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_480I) || (which_source == VIP_QUALITY_HDMI_480P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_WaterfallStair_TV002] = 1;
		else
			ID_TV002_Flag[ID_WaterfallStair_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_WaterfallStair_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("wfs,%d,%d,%d\n",y_ratio1, h_ratio1, s_ratio1);
	}

	return ID_TV002_Flag[ID_WaterfallStair_TV002];
}

unsigned int Scaler_scalerVIP_ID_MovingCircle_SD_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 950, h_th1 = 850, s_th1 = 950;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,1,6,22,50,57,49,47,47,43,38,35,34,33,41,37,32,33,35,39,43,46,45,47,50,40,23,9,2,0,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={910,36,22,25,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_480I) || (which_source == VIP_QUALITY_HDMI_480P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_MovingCircle_TV002] = 1;
		else
			ID_TV002_Flag[ID_MovingCircle_TV002] = 0;

	} else {
		ID_TV002_Flag[ID_MovingCircle_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("mci,%d,%d,%d\n",y_ratio1, h_ratio1, s_ratio1);
	}

	return ID_TV002_Flag[ID_MovingCircle_TV002];
}

unsigned int Scaler_scalerVIP_ID_Moving_PTG_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	extern unsigned char pattern_gen_flag;

	unsigned int y_th1 = 950, h_th1 = 950, s_th1 = 950;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,0,0,0,3,3,3,4,4,3,3,3,4,881,61,1,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,985,10,2,1,0,0,0,0,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={28,17,7,10,935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay ;

	if((which_source == VIP_QUALITY_HDMI_480I) || (which_source == VIP_QUALITY_HDMI_480P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1) && (pattern_gen_flag == 1))
			ID_TV002_Flag[ID_MovingPTG_Mono_TV002] = 1;
		else {
			ID_TV002_Flag[ID_MovingPTG_Mono_TV002] = 0;
		}
		ID_TV002_Flag[ID_MovingPTG_Mono_TV002] = (pattern_gen_flag == 1)?(1):(0);
	} else {
		ID_TV002_Flag[ID_MovingPTG_Mono_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("mptg,%d,%d,%d,sc=%d, PTG=%d\n",y_ratio1, h_ratio1, s_ratio1, RPC_SmartPic_clue->SceneChange, pattern_gen_flag);
	}

	return ID_TV002_Flag[ID_MovingPTG_Mono_TV002];
}

unsigned int Scaler_scalerVIP_ID_Moving_Roof_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	extern unsigned char pattern_gen_flag;
	unsigned int print_flag;
	unsigned int print_delay;

	unsigned int y_th1 = 950, h_th1 = 750, s_th1 = 950;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,0,94,143,98,84,98,132,139,113,63,24,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={0,0,2,30,111,123,124,142,151,106,97,41,31,20,12,3,0,0,0,0,0,0,0,0,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	if((which_source == VIP_QUALITY_HDMI_480I) || (which_source == VIP_QUALITY_HDMI_480P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_MovingRoof_TV002] = 1;
		else {
			if (RPC_SmartPic_clue->SceneChange == 1)
				ID_TV002_Flag[ID_MovingRoof_TV002] = 0;
		}

	} else {
		ID_TV002_Flag[ID_MovingRoof_TV002] = 0xFF;
	}

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT6) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("mrd,%d,%d,%d,sc=%d\n",y_ratio1, h_ratio1, s_ratio1, RPC_SmartPic_clue->SceneChange);
	}

	return ID_TV002_Flag[ID_MovingRoof_TV002];
}

void Scaler_scalerVIP_set_sharpness_level_Directly_TV002(DRV_Sharpness_Level *ptr)
{
	drvif_color_set_Sharpness_level(ptr);
}

DRV_Sharpness_Level Current_Shp_Level_TBL_TV002 = {0};
unsigned char Scaler_scalerVIP_Access_sharpness_level_TBL_TV002(DRV_Sharpness_Level *ptr, unsigned char isSet_Flag)
{
	static unsigned char shp_TBL_src = 0xff;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(VIP_RPC_system_info_structure_table == NULL){
		rtd_pr_vpq_info("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return 0;
	}

	if (ptr == NULL) {
		rtd_pr_vpq_emerg("Scaler_scalerVIP_Access_sharpness_level_TBL_TV002 NULL=%p\n", ptr);
		return -1;
	}

	if (isSet_Flag == 1) {
		memcpy(&Current_Shp_Level_TBL_TV002, ptr, sizeof(DRV_Sharpness_Level));
		shp_TBL_src = VIP_RPC_system_info_structure_table->VIP_source;
	} else {
		memcpy(ptr, &Current_Shp_Level_TBL_TV002, sizeof(DRV_Sharpness_Level));
	}

	return shp_TBL_src;
}

void Scaler_scalerVIP_ID_TV002(SLR_VIP_TABLE* ShareMem_VIP_TBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, unsigned int MA_print_count)
{
	unsigned short i;
	unsigned char y_max_idx=0, hue_max_idx=0;
	unsigned char y_2nd_idx=0, hue_2nd_idx=0;
	unsigned char y_3th_idx=0, hue_3th_idx=0;
	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	unsigned int print_flag ;
	unsigned int print_delay ;

	hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
	Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
	sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

	// find max, 2nd, 3th idx of hist
	for(i=0;i<TV006_VPQ_chrm_bin;i++)
		if(Y_hist_ratio[i]>Y_hist_ratio[y_max_idx])
			y_max_idx = i;
	for(i=0;i<TV006_VPQ_chrm_bin;i++)
		if((Y_hist_ratio[i]<Y_hist_ratio[y_max_idx])&&(Y_hist_ratio[i]>Y_hist_ratio[y_2nd_idx]))
			y_2nd_idx = i;
	for(i=0;i<TV006_VPQ_chrm_bin;i++)
		if((Y_hist_ratio[i]<Y_hist_ratio[y_2nd_idx])&&(Y_hist_ratio[i]>Y_hist_ratio[y_3th_idx]))
			y_3th_idx = i;

	for(i=0;i<COLOR_HUE_HISTOGRAM_LEVEL;i++)
		if(hue_hist_ratio[i]>hue_hist_ratio[hue_max_idx])
			hue_max_idx = i;
	for(i=0;i<COLOR_HUE_HISTOGRAM_LEVEL;i++)
		if((hue_hist_ratio[i]<hue_hist_ratio[hue_max_idx])&&(hue_hist_ratio[i]>hue_hist_ratio[hue_2nd_idx]))
			hue_2nd_idx = i;
	for(i=0;i<COLOR_HUE_HISTOGRAM_LEVEL;i++)
		if((hue_hist_ratio[i]<hue_hist_ratio[hue_2nd_idx])&&(hue_hist_ratio[i]>hue_hist_ratio[hue_3th_idx]))
			hue_3th_idx = i;

	/* ID check*/
	/******************************************************************************************************/
	/******************************************************************************************************/
	if (ID_TV002_Detect == 1) {
		Scaler_scalerVIP_ID_radio_caculator_TV002(y_max_idx, Y_hist_ratio, RPC_system_infoTBL, system_infoTBL, MA_print_count);
		Scaler_scalerVIP_ID_FaceArtifactNoise_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_Rain_0_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_Rain_1_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_Snow_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_BlueArcBridge_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_DarkScene_Road_Lamp_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		//Scaler_scalerVIP_ID_MonoScope_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_Tunnel_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_RockMountain_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_QtechBamboo_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);

		Scaler_scalerVIP_ID_ClockTower_SD_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_waterfallStair_SD_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_MovingCircle_SD_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_Moving_PTG_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_Moving_Roof_TV002(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
	}
	/******************************************************************************************************/
	/******************************************************************************************************/

	/* result check*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT5) != 0)
	{	if((MA_print_count%print_delay)==0) {
			rtd_pr_vpq_info("ID, src=%d\n", RPC_system_infoTBL->VIP_source);
			for(i=0;i<ID_ITEM_TV002_MAX;i++)
				printk("%d,", ID_TV002_Flag[i]);
			printk("\n");
		}
	}

	/* HIST check*/
	if((print_flag&_BIT10) != 0)
	{	if((MA_print_count%print_delay)==0) {
			rtd_pr_vpq_info("Y_HIST_Ratio={");
			for(i=0;i<TV006_VPQ_chrm_bin;i++)
				printk("%d,", SmartPic_clue->Y_Main_Hist_Ratio[i]);
			printk("}\n");
		}
	}

	if((print_flag&_BIT11) != 0)
	{	if((MA_print_count%print_delay)==0) {
			rtd_pr_vpq_info("HUE_HIST_Ratio={");
			for(i=0;i<COLOR_HUE_HISTOGRAM_LEVEL;i++)
				printk("%d,", SmartPic_clue->Hue_Main_His_Ratio[i]);
			printk("}\n");
		}
	}

	if((print_flag&_BIT12) != 0)
	{	if((MA_print_count%print_delay)==0) {
			rtd_pr_vpq_info("SAT_HIST_Ratio={");
			for(i=0;i<COLOR_AutoSat_HISTOGRAM_LEVEL;i++)
				printk("%d,", SmartPic_clue->Sat_Main_His_Ratio[i]);
			printk("}\n");
		}
	}

}

void Scaler_scalerVIP_ID_Apply_TV002(SLR_VIP_TABLE* ShareMem_VIP_TBL, _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	if (ID_TV002_Apply == 1) {
		Scaler_scalerVIP_ID_Apply_PQA_LV_TV002(ShareMem_VIP_TBL, RPC_system_infoTBL, system_infoTBL, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_Apply_2dPeaking_TV002(ShareMem_VIP_TBL, RPC_system_infoTBL, system_infoTBL, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_Apply_DI_TV002(ShareMem_VIP_TBL, RPC_system_infoTBL, system_infoTBL, SmartPic_clue, MA_print_count);
	}
}

void Scaler_scalerVIP_ID_Apply_PQA_LV_TV002(SLR_VIP_TABLE* ShareMem_VIP_TBL, _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	//unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned char *PQA_RunFlag = &(system_infoTBL->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[0]);

	//unsigned short i;
	// change Lv
	PQA_Input_Level[I_006_SHP_Mode2] = 0;
	PQA_Input_rate[I_006_SHP_Mode2] = 0;

	if (ID_TV002_Flag[ID_Radio_TV002] == 1) {
		PQA_Input_Level[I_006_RTNR_Mode2_HMCNR] = 1;
		PQA_Input_rate[I_006_RTNR_Mode2_HMCNR] = 1;

		PQA_Input_Level[I_006_RTNR_Mode1] = 1;
		PQA_Input_rate[I_006_RTNR_Mode1] = 1;

		PQA_Input_Level[I_006_RTNR_Mode2] = 1;
		PQA_Input_rate[I_006_RTNR_Mode2] = 1;

		PQA_Input_Level[I_002_RTNR_2] = 1;
		PQA_Input_rate[I_002_RTNR_2] = 1;

		PQA_RunFlag[I_ID] = 1;
	} else if (ID_TV002_Flag[ID_FaceArtifactNoise_TV002] == 1) {
		PQA_Input_Level[I_006_RTNR_Mode2_HMCNR] = 0;
		PQA_Input_rate[I_006_RTNR_Mode2_HMCNR] = 0;

		PQA_Input_Level[I_006_RTNR_Mode1] = 0;
		PQA_Input_rate[I_006_RTNR_Mode1] = 0;

		PQA_Input_Level[I_006_RTNR_Mode2] = 0;
		PQA_Input_rate[I_006_RTNR_Mode2] = 0;

		PQA_Input_Level[I_002_RTNR_2] = 0;
		PQA_Input_rate[I_002_RTNR_2] = 0;

		PQA_RunFlag[I_ID] = 1;
	} else if (ID_TV002_Flag[ID_Raining_0_TV002] == 1 || ID_TV002_Flag[ID_Raining_1_TV002] || ID_TV002_Flag[ID_Snow_TV002]) {
		PQA_Input_Level[I_006_RTNR_Mode2_HMCNR] = 0;
		PQA_Input_rate[I_006_RTNR_Mode2_HMCNR] = 0;

		PQA_Input_Level[I_006_RTNR_Mode1] = 0;
		PQA_Input_rate[I_006_RTNR_Mode1] = 0;

		PQA_Input_Level[I_006_RTNR_Mode2] = 0;
		PQA_Input_rate[I_006_RTNR_Mode2] = 0;

		PQA_Input_Level[I_002_RTNR_2] = 0;
		PQA_Input_rate[I_002_RTNR_2] = 0;

		PQA_RunFlag[I_ID] = 0;
	} else if (ID_TV002_Flag[ID_BlueArcBridge_TV002] == 1) {
		PQA_Input_Level[I_006_RTNR_Mode2_HMCNR] = 8;
		PQA_Input_rate[I_006_RTNR_Mode2_HMCNR] = 8;

		PQA_Input_Level[I_006_RTNR_Mode1] = 8;
		PQA_Input_rate[I_006_RTNR_Mode1] = 8;

		PQA_Input_Level[I_006_RTNR_Mode2] = 8;
		PQA_Input_rate[I_006_RTNR_Mode2] = 8;

		PQA_Input_Level[I_002_RTNR_2] = 8;
		PQA_Input_rate[I_002_RTNR_2] = 8;

		PQA_RunFlag[I_ID] = 0;
	} else if (ID_TV002_Flag[ID_DarkScene_Road_Lamp_TV002] == 1) {
		PQA_Input_Level[I_006_RTNR_Mode2_HMCNR] = 0;
		PQA_Input_rate[I_006_RTNR_Mode2_HMCNR] = 0;

		PQA_Input_Level[I_006_RTNR_Mode1] = 0;
		PQA_Input_rate[I_006_RTNR_Mode1] = 0;

		PQA_Input_Level[I_006_RTNR_Mode2] = 0;
		PQA_Input_rate[I_006_RTNR_Mode2] = 0;

		PQA_Input_Level[I_002_RTNR_2] = 0;
		PQA_Input_rate[I_002_RTNR_2] = 0;

		PQA_RunFlag[I_ID] = 1;
	} else if (ID_TV002_Flag[ID_Tunnel_TV002] == 1) {
		PQA_Input_Level[I_006_SHP_Mode2] = 9;
		PQA_Input_rate[I_006_SHP_Mode2] = 0;

		PQA_RunFlag[I_ID] = 0;
	} else if (ID_TV002_Flag[ID_QtechBamboo_TV002] == 1) {
		PQA_Input_Level[I_006_RTNR_Mode2_HMCNR] = 9;
		PQA_Input_rate[I_006_RTNR_Mode2_HMCNR] = 9;

		PQA_Input_Level[I_006_RTNR_Mode1] = 9;
		PQA_Input_rate[I_006_RTNR_Mode1] = 9;

		PQA_Input_Level[I_006_RTNR_Mode2] = 9;
		PQA_Input_rate[I_006_RTNR_Mode2] = 9;

		PQA_Input_Level[I_002_RTNR_2] = 9;
		PQA_Input_rate[I_002_RTNR_2] = 9;

		PQA_RunFlag[I_ID] = 0;

	} else {
		PQA_RunFlag[I_ID] = 0;
	}
}

void Scaler_scalerVIP_ID_Apply_2dPeaking_TV002(SLR_VIP_TABLE* ShareMem_VIP_TBL, _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _clues* SmartPic_clue, unsigned int MA_print_count)
{
#if 1 /* apply special SHP setting */
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	static unsigned char pre_ID_status = 0xff;
	DRV_Sharpness_Level shp_Lv_TBL_tmp;
	unsigned char shp_Lv_TBL_src = 0;
	unsigned char peaking_driver_set;
	unsigned int print_flag;
	unsigned int print_delay;

	if((ID_TV002_Flag[ID_Radio_TV002] != 0xFF) || (ID_TV002_Flag[ID_RockMountain_TV002] != 0xFF)) {
		shp_Lv_TBL_src = Scaler_scalerVIP_Access_sharpness_level_TBL_TV002(&shp_Lv_TBL_tmp, 0);
		if (shp_Lv_TBL_src == which_source) {
			peaking_driver_set = 0;
			if (ID_TV002_Flag[ID_Radio_TV002] == 1) {
				peaking_driver_set = 1;

				shp_Lv_TBL_tmp.Edge_Level.G_Pos_Gain = 10;
				shp_Lv_TBL_tmp.Edge_Level.HV_POS_Gain = 30;
				shp_Lv_TBL_tmp.Edge_Level.G_Neg_Gain = 10;
				shp_Lv_TBL_tmp.Edge_Level.HV_NEG_Gain = 30;

				shp_Lv_TBL_tmp.Texture_Level.G_Pos_Gain = 10;
				shp_Lv_TBL_tmp.Texture_Level.HV_POS_Gain = 30;
				shp_Lv_TBL_tmp.Texture_Level.G_Neg_Gain = 10;
				shp_Lv_TBL_tmp.Texture_Level.HV_NEG_Gain = 30;

				shp_Lv_TBL_tmp.Vertical_Level.G_Pos_Gain = 10;
				shp_Lv_TBL_tmp.Vertical_Level.HV_POS_Gain = 30;
				shp_Lv_TBL_tmp.Vertical_Level.G_Neg_Gain = 10;
				shp_Lv_TBL_tmp.Vertical_Level.HV_NEG_Gain = 30;

				shp_Lv_TBL_tmp.Vertical_Edge_Level.G_Pos_Gain = 10;
				shp_Lv_TBL_tmp.Vertical_Edge_Level.HV_POS_Gain = 30;
				shp_Lv_TBL_tmp.Vertical_Edge_Level.G_Neg_Gain = 10;
				shp_Lv_TBL_tmp.Vertical_Edge_Level.HV_NEG_Gain = 30;

				pre_ID_status = ID_Radio_TV002;
			} else if (ID_TV002_Flag[ID_RockMountain_TV002] == 1) {
				peaking_driver_set = 1;

				shp_Lv_TBL_tmp.Edge_Level.G_Pos_Gain = 80;
				shp_Lv_TBL_tmp.Edge_Level.HV_POS_Gain = 200;
				shp_Lv_TBL_tmp.Vertical_Edge_Level.G_Pos_Gain = 60;
				shp_Lv_TBL_tmp.Vertical_Edge_Level.HV_POS_Gain = 200;

				pre_ID_status = ID_RockMountain_TV002;
			} else {
				if (pre_ID_status != ID_ITEM_TV002_MAX)
					peaking_driver_set = 1;
				pre_ID_status = ID_ITEM_TV002_MAX;
			}

			/* value apply to reg*/
			if (peaking_driver_set == 1)
				Scaler_scalerVIP_set_sharpness_level_Directly_TV002(&shp_Lv_TBL_tmp);

		}
	}
	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT7) != 0)
	{	if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("app,st=%d,src0=%d,src1=%d\n",pre_ID_status, shp_Lv_TBL_src, which_source);
	}
#endif
}
void Scaler_scalerVIP_ID_Apply_DI_TV002(SLR_VIP_TABLE* ShareMem_VIP_TBL, _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	extern DRV_di_smd_table_t gSMDtable[7];

	unsigned char smd_reliablerange_sad_th, h_pan_mv_value, n_mcnr_v_pan_mv, ma_controlmode;
	unsigned char SMD_driver_set, MA_Crtl_Mode_driver_set;
	short tmp;
	unsigned char vip_src = RPC_system_infoTBL->VIP_source;
	unsigned char  SMD_TBL_Sel = ShareMem_VIP_TBL->VIP_QUALITY_Coef[vip_src][VIP_QUALITY_DI_SMD_Level];
	static unsigned char pre_ID_status_SMD = 0xFF, pre_ID_status_MA_mode = 0xFF;
	DRV_di_smd_table_t SMDtable = {0};
	unsigned int print_flag;
	unsigned int print_delay;

#if 1	/* spec SMD setting */
	if((ID_TV002_Flag[ID_MovingClockTower_TV002] != 0xFF) || (ID_TV002_Flag[ID_WaterfallStair_TV002] != 0xFF) || (ID_TV002_Flag[ID_MovingPTG_Mono_TV002] != 0xFF)
		|| (ID_TV002_Flag[ID_MovingRoof_TV002] != 0xFF)) {

		if (SMD_TBL_Sel < sizeof(gSMDtable)/sizeof(DRV_di_smd_table_t))
			memcpy(&SMDtable, &gSMDtable[SMD_TBL_Sel], sizeof(DRV_di_smd_table_t));

		smd_reliablerange_sad_th = 125;	/* ori setting is in VP_init()*/
		SMD_driver_set = 0;

		if (ID_TV002_Flag[ID_MovingPTG_Mono_TV002] == 1) {
			pre_ID_status_SMD = ID_MovingPTG_Mono_TV002;
			SMD_driver_set = 1;

			drvif_color_Get_MCNR_HV_PAN_MV(&h_pan_mv_value, &n_mcnr_v_pan_mv);

			SMDtable.smd_pan.apply_result_mode = 3;
			SMDtable.smd_pan.firmware_pan = 1;
			SMDtable.smd_pan.searchmv_apply = 0;
			SMDtable.smd_pan.hardware_pan_en = 0;
			SMDtable.smd_pan.firmware_pan_sad_th = 0;

			SMDtable.Smd_Pan_modify.smd_firmwarepan_sadmax = 450;

			tmp = ((h_pan_mv_value-10)*(-1))/2+ 3;
			SMDtable.smd_gmvcoeff3.mv_x = (tmp>6)?(6):((tmp<0)?(0):(tmp));
			tmp = (n_mcnr_v_pan_mv-3)*(-1)+3;
			SMDtable.smd_gmvcoeff3.mv_y = (tmp>6)?(6):((tmp<0)?(0):(tmp));

			SMDtable.smd_control.upperbound_sad = 200;

			SMDtable.Smd_diag_edge_control.smd_mcdi2madi_en = 0;

			smd_reliablerange_sad_th = 0;

		} else if (ID_TV002_Flag[ID_MovingClockTower_TV002] == 1) {
			pre_ID_status_SMD = ID_MovingClockTower_TV002;
			SMD_driver_set = 1;

			SMDtable.smd_pan.firmware_pan = 1;
			SMDtable.smd_pan.searchmv_apply = 0;
			SMDtable.smd_pan.hardware_pan_en = 0;
			SMDtable.smd_pan.firmware_pan_sad_th = 0;

			SMDtable.Smd_Pan_modify.smd_firmwarepan_sadmax = 200;

			SMDtable.smd_gmvcoeff3.mv_x = 3;
			SMDtable.smd_gmvcoeff3.mv_y = 1;

			SMDtable.smd_control.upperbound_sad = 200;

			smd_reliablerange_sad_th = 50;

		} else if (ID_TV002_Flag[ID_WaterfallStair_TV002] == 1) {
			pre_ID_status_SMD = ID_WaterfallStair_TV002;
			SMD_driver_set = 1;

			SMDtable.smd_additionalcoeff.result_en = 0;

		} else if (ID_TV002_Flag[ID_MovingRoof_TV002] == 1) {
			pre_ID_status_SMD = ID_MovingRoof_TV002;
			SMD_driver_set = 1;

			smd_reliablerange_sad_th = 0;

		} else {
			if (pre_ID_status_SMD != ID_ITEM_TV002_MAX)
				SMD_driver_set = 1;

			pre_ID_status_SMD = ID_ITEM_TV002_MAX;

		}

		/* apply to reg. apply always while ID = 1, apply reg once to retrun ori value*/
		if (SMD_driver_set == 1) {
			drvif_color_smd_table(&SMDtable);
			drvif_color_Set_smd_reliablerange_sad_th(smd_reliablerange_sad_th);
			drvif_color_Set_DI_DB_APPLY();
		}
	}
#endif

#if 1	/* DI intra,weave*/
	if((ID_TV002_Flag[ID_MovingCircle_TV002] != 0xFF)) {
		MA_Crtl_Mode_driver_set = 0;
		if (ID_TV002_Flag[ID_MovingCircle_TV002] == 1) {
			MA_Crtl_Mode_driver_set = 1;
			pre_ID_status_MA_mode = ID_MovingCircle_TV002;
			ma_controlmode = 2;
		} else {
			if (pre_ID_status_MA_mode != ID_ITEM_TV002_MAX)
				MA_Crtl_Mode_driver_set = 1;
			pre_ID_status_MA_mode = ID_ITEM_TV002_MAX;
			ma_controlmode = 1;
		}
		/* apply to reg*/
		if (MA_Crtl_Mode_driver_set == 1) {
			drvif_color_Set_MA_Control_Mode(ma_controlmode);
			drvif_color_Set_DI_DB_APPLY();
		}
	}

#endif

	/* debug log*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT8) != 0)
	{	if((MA_print_count%print_delay)==0)
			printk("app reg,st=%d,%d\n",pre_ID_status_SMD, pre_ID_status_MA_mode);
	}

}

//====================================================================================
//****************************************************PQA****************************************************
//====================================================================================
#define windowSize_PQA_I_002 32
#define windowSizeBit_PQA_I_002 5
unsigned int Scaler_scalerVIP_compute_I_002_level_Index_Ave(unsigned int lv_index, unsigned char isAll_Flag)
{
	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_PQA_I_002]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_PQA_I_002;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_PQA_I_002;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_PQA_I_002;

		idx++;
		if(idx==windowSize_PQA_I_002)
			idx=0;
	}
	return ret;
}

int Scaler_scalerVIP_compute_Still_Idx_FMV_HMC_I_002(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	//still meter
	unsigned int printf_Flag=0, print_delay=200;
	unsigned char i;
	unsigned int *FMV_hist_ratio, *HMC_bin_hist_ratio;
	unsigned int still_idx, still_idx_FMV, still_idx_HMC_Bin, FMV_w_total, HMC_w_total;

	unsigned char PQA_table_select = 0;
	unsigned int *pPQA_FMV_w_TBL = NULL;	// for FMV weighting
	unsigned int *pPQA_HMC_MV_w_TBL = NULL;
	unsigned char GMV;
	int ret_still_idx;

	if((system_info_structure_table == NULL) || (SmartPic_clue == NULL) || (vipTable_shareMem == NULL)) {
		if(MA_print_count%360==0)
			rtd_pr_vpq_emerg("Share Memory PQ data ERROR,system_info_structure_table = %p, SmartPic_clue = %p, vipTable_shareMem = %p\n",
			 	system_info_structure_table, SmartPic_clue, vipTable_shareMem);
		return 0;
	}

	PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	pPQA_FMV_w_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_FMV_Hist_Weighting][0]);	// for FMV weighting
	pPQA_HMC_MV_w_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_HMC_MV_Hist_Weighting][0]);

	FMV_hist_ratio = SmartPic_clue->FMV_hist_Ratio;
	HMC_bin_hist_ratio = SmartPic_clue->HMC_bin_hist_Ratio;
	GMV = SmartPic_clue->GMV;

	still_idx_FMV = 0;
	FMV_w_total =0;
	for(i=0;i<PQA_I_L09;i++)
	{
		// 10 is center
		still_idx_FMV = still_idx_FMV + (FMV_hist_ratio[10+i]*pPQA_FMV_w_TBL[i]);
		if(i!=0)
			still_idx_FMV = still_idx_FMV + (FMV_hist_ratio[10-i]*pPQA_FMV_w_TBL[i]);
		FMV_w_total = FMV_w_total + pPQA_FMV_w_TBL[i];
	}

	if(pPQA_FMV_w_TBL[PQA_I_L09] == 1) {
		if(FMV_w_total != 0)
			still_idx_FMV = still_idx_FMV / FMV_w_total;
	}


	still_idx_HMC_Bin = 0;
	HMC_w_total = 0;
	for(i=0;i<PQA_I_L09;i++)
	{
		// 16 is center
		still_idx_HMC_Bin = still_idx_HMC_Bin + (HMC_bin_hist_ratio[16+i]*pPQA_HMC_MV_w_TBL[i]);
		if(i!=0)
			still_idx_HMC_Bin = still_idx_HMC_Bin + (HMC_bin_hist_ratio[16-i]*pPQA_HMC_MV_w_TBL[i]);
		HMC_w_total = HMC_w_total + pPQA_HMC_MV_w_TBL[i];
	}

	if(pPQA_HMC_MV_w_TBL[PQA_I_L09] == 1) {
		if(HMC_w_total != 0)
			still_idx_HMC_Bin = still_idx_HMC_Bin / HMC_w_total;
	}
	still_idx = (still_idx_FMV+still_idx_HMC_Bin)>>1;

	//for debug.
	printf_Flag = system_info_structure_table->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	if(((printf_Flag&_BIT4)!=0))
	{
		if((MA_print_count%print_delay==0))
			rtd_pr_vpq_info("FMV=%d,HMC=%d,still=%d,GMV=%d\n", still_idx_FMV, still_idx_HMC_Bin, still_idx,GMV);
	}

	if (GMV == 3)
		ret_still_idx = still_idx_FMV;
	else
		ret_still_idx = 0;

	return ret_still_idx;
}

void Scaler_scalerVIP_compute_level_I_002_RTNR2_tv002(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	unsigned int level_Idx=0, printf_Flag=0, print_delay=200;
	unsigned char Lv=0,i;
	unsigned short Lv_intp=0;//, sr_d;

	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_InputMAD_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_RTNR_MAD][0]);
	//unsigned int *pPQA_Input_HistMeanY_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean][0]);
	unsigned int *pPQA_Input_HistMeanY_Dark_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean_dark][0]);
	//unsigned int *pPQA_FMV_Hist_Weighting_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_FMV_Hist_Weighting][0]);
	unsigned int *pPQA_FMV_Hist_stillIdx_th_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_FMV_Hist_stillIdx_th][0]);
	unsigned int *pPQA_Input_HistMeanY_Dark_Gain_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean_dark_gain][0]);

	//unsigned int *pPQA_FMV_Hist_motionIdx_offset_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_FMV_Hist_motionIdx_offset][0]);
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	//unsigned char *PQA_RunFlag = &(system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[0]);

	int still_idx;
	unsigned int tmp;
	// get spatial content noise meter status
	unsigned int noise_status = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL);
	unsigned int noise_relaible = scalerVIP_DI_MiddleWare_GetStatus(STATUS_CONTENT_SPATIAL_NOISE_LEVEL_RELIABLE);
	//get mean histgram Y
	unsigned char meanHistY = SmartPic_clue->Hist_Y_Mean_Value;
	//unsigned short dark_meanY_Gain;
	unsigned int motioni[10]={0};
	//unsigned int motion_h_t = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_T_reg);
	//unsigned int motion_h_m = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_M_reg);
	//unsigned int motion_h_b = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_B_reg);
	//unsigned int motion_v_l = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_L_reg);
	//unsigned int motion_v_m = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_M_reg);
	//unsigned int motion_v_r = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_R_reg);
	//unsigned int motion = rtd_inl(DI_IM_DI_SI_FILM_MOTION_reg);
	unsigned int motion = scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION);

	//motion = max(motion_h_t, motion_h_m);
	//motion = max(motion, motion_h_b);
	//motion = max(motion, motion_v_l);
	//motion = max(motion, motion_v_m);
	//motion = max(motion, motion_v_r);
	motioni[0] = motion;

	// refernce MV to get still index
	still_idx = Scaler_scalerVIP_compute_Still_Idx_FMV_HMC_I_002(vipTable_shareMem, SmartPic_clue, system_info_structure_table, MA_print_count);
	// according  still idx to offset motion level
	if(still_idx<=-1)
		motion += pPQA_InputMAD_TBL[PQA_I_L09];
	else if(still_idx<pPQA_FMV_Hist_stillIdx_th_TBL[PQA_I_L00])
		motion += pPQA_InputMAD_TBL[PQA_I_L09];
	else if(still_idx<pPQA_FMV_Hist_stillIdx_th_TBL[PQA_I_L01])
		motion += pPQA_InputMAD_TBL[PQA_I_L08];
	else if(still_idx<pPQA_FMV_Hist_stillIdx_th_TBL[PQA_I_L02])
		motion += pPQA_InputMAD_TBL[PQA_I_L07];
	else if(still_idx<pPQA_FMV_Hist_stillIdx_th_TBL[PQA_I_L03])
		motion += pPQA_InputMAD_TBL[PQA_I_L06];
	else if(still_idx<pPQA_FMV_Hist_stillIdx_th_TBL[PQA_I_L04])
		motion += pPQA_InputMAD_TBL[PQA_I_L05];
	else if(still_idx<pPQA_FMV_Hist_stillIdx_th_TBL[PQA_I_L05])
		motion += pPQA_InputMAD_TBL[PQA_I_L04];
	else if(still_idx<pPQA_FMV_Hist_stillIdx_th_TBL[PQA_I_L06])
		motion += pPQA_InputMAD_TBL[PQA_I_L03];
	else if(still_idx<pPQA_FMV_Hist_stillIdx_th_TBL[PQA_I_L07])
		motion += pPQA_InputMAD_TBL[PQA_I_L02];
	else if(still_idx<pPQA_FMV_Hist_stillIdx_th_TBL[PQA_I_L08])
		motion += pPQA_InputMAD_TBL[PQA_I_L01];
	else if(still_idx<pPQA_FMV_Hist_stillIdx_th_TBL[PQA_I_L09])
		motion += pPQA_InputMAD_TBL[PQA_I_L00];
	else
		motion += 0;
	motioni[1] = motion;
	/*
	//refernce to hist mean Y to offset lv
	if(meanHistY<pPQA_Input_HistMeanY_TBL[PQA_I_L00])
		motion += 0;
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[PQA_I_L01])
		motion += pPQA_InputMAD_TBL[PQA_I_L00];
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[PQA_I_L02])
		motion += pPQA_InputMAD_TBL[PQA_I_L01];
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[PQA_I_L03])
		motion += pPQA_InputMAD_TBL[PQA_I_L02];
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[PQA_I_L04])
		motion += pPQA_InputMAD_TBL[PQA_I_L03];
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[PQA_I_L05])
		motion += pPQA_InputMAD_TBL[PQA_I_L04];
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[PQA_I_L06])
		motion += pPQA_InputMAD_TBL[PQA_I_L05];
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[PQA_I_L07])
		motion += pPQA_InputMAD_TBL[PQA_I_L06];
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[PQA_I_L08])
		motion += pPQA_InputMAD_TBL[PQA_I_L07];
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[PQA_I_L09])
		motion += pPQA_InputMAD_TBL[PQA_I_L08];
	else
		motion += pPQA_InputMAD_TBL[PQA_I_L09];
	*/
	motioni[2] = motion;

	// reduce rtnr for dark sence,
	if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L00])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L00];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L00];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L01])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L01];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L01];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L02])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L02];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L02];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L03])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L03];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L03];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L04])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L04];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L04];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L05])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L05];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L05];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L06])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L06];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L06];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L07])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L07];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L07];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L08])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L08];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L08];
	else if(meanHistY<pPQA_Input_HistMeanY_Dark_TBL[PQA_I_L09])
		motion += pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L09];
		//dark_meanY_Gain = pPQA_Input_HistMeanY_Dark_Gain_TBL[PQA_I_L09];
	else
		motion += 0;
	//dark_meanY_Gain = 1024;
	//motion = (motion*dark_meanY_Gain)>>10;
	motioni[3] = motion;

	/*
	//reference to snr noise meter, Lv=9 is only for noise meter
	if(noise_status>=10 && noise_relaible==1)
		motion = 0;
	else
		motion += pPQA_InputMAD_TBL[PQA_I_L01]; // normal nr seeting is lv0~8
	motioni[4] = motion;
	*/
#if 0
	// for id pattern
	//for radio
	if((ID_TV002_Flag[ID_Pattern_Radio]==1 && noise_status<=1 && noise_relaible==1))
		motion += (pPQA_InputMAD_TBL[PQA_I_L09]+1);
	if(ID_TV002_Flag[ID_Pattern_FaceArtifactNoise]==1)
		motion = 0;
	if(ID_TV002_Flag[ID_Pattern_BrightnessDarkness_11]==1)
		motion = pPQA_InputMAD_TBL[PQA_I_L05];
	if(ID_TV002_Flag[ID_Pattern_Raining] == 1)
		motion = (pPQA_InputMAD_TBL[PQA_I_L07]);
	if(ID_TV002_Flag[ID_OsceanWave_NatureImg06] == 1)
		motion += (pPQA_InputMAD_TBL[PQA_I_L09]+1);
	if(ID_TV002_Flag[ID_BlueArcBridge_Bri_06]==1)
		motion = 0;

	motioni[5] = motion;
#endif
	//  for change smoothly
	if(motion>pPQA_InputMAD_TBL[PQA_I_L09])
		motion = pPQA_InputMAD_TBL[PQA_I_L09];

	level_Idx = Scaler_scalerVIP_compute_I_002_level_Index_Ave(motion, 0);

	//for debug.
	//level_Idx = SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0];

	if(level_Idx <= pPQA_InputMAD_TBL[PQA_I_L00])
		Lv = PQA_I_L09;
	else if(level_Idx <= pPQA_InputMAD_TBL[PQA_I_L01])
		Lv = PQA_I_L08;
	else if(level_Idx <= pPQA_InputMAD_TBL[PQA_I_L02])
		Lv = PQA_I_L07;
	else if(level_Idx <= pPQA_InputMAD_TBL[PQA_I_L03])
		Lv = PQA_I_L06;
	else if(level_Idx <= pPQA_InputMAD_TBL[PQA_I_L04])
		Lv = PQA_I_L05;
	else if(level_Idx <= pPQA_InputMAD_TBL[PQA_I_L05])
		Lv = PQA_I_L04;
	else if(level_Idx <= pPQA_InputMAD_TBL[PQA_I_L06])
		Lv = PQA_I_L03;
	else if(level_Idx <= pPQA_InputMAD_TBL[PQA_I_L07])
		Lv = PQA_I_L02;
	else if(level_Idx <= pPQA_InputMAD_TBL[PQA_I_L08])
		Lv = PQA_I_L01;
	else
		Lv=PQA_I_L00;

	if(Lv==9)
		Lv_intp =0;
	else
	{
		if(level_Idx>pPQA_InputMAD_TBL[PQA_I_L09])
			level_Idx = pPQA_InputMAD_TBL[PQA_I_L09];
		tmp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1])==0)?1:(pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1]);
		Lv_intp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-level_Idx)<<8) / tmp;
	}
#if 0
	// for id pattern,
	PQA_RunFlag[I_ID] = 0;
	if(ID_TV002_Flag[ID_Pattern_FaceArtifactNoise2]==1)
	{
		Lv = 0;
		Lv_intp = 0;
		PQA_RunFlag[I_ID] = 1;	//run id setting.
	}
	// use level 1 and table 9 for zooming temple
	if(ID_TV002_Flag[ID_Pattern_TempleZooming]==1)
	{
		Lv = 1;
		Lv_intp = 0;
		PQA_RunFlag[I_ID] = 1;	//run id setting.
	}
	// use level 2 and table 9 for DarkWheelNoise
	if(ID_TV002_Flag[ID_Pattern_DarkWheelNoise]==1)
	{
		Lv = 2;
		Lv_intp = 0;
		PQA_RunFlag[I_ID] = 1;	//run id setting.
	}
	// use level 3 and table 9 for DarkScene_Road_Lamp
	if(ID_TV002_Flag[ID_Pattern_DarkScene_Road_Lamp]==1)
	{
		Lv = 3;
		Lv_intp = 0;
		PQA_RunFlag[I_ID] = 1;	//run id setting.
	}
#endif
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_1] = motion;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_2] = meanHistY;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_3] = still_idx;

	PQA_Input_Level[I_002_RTNR_2] = Lv;
	PQA_Input_rate[I_002_RTNR_2] = Lv_intp;

	printf_Flag = system_info_structure_table->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//for debug.
	if(((printf_Flag&_BIT3)!=0))
	{
		if(MA_print_count%print_delay==0)
		{
			rtd_pr_vpq_info("rm2~lv=%d,st=%d,LvI=%d,mo=%d,y=%d,int=%d,nmL=%d,nmR=%d\n",
				Lv, still_idx, level_Idx, motion, meanHistY, Lv_intp,noise_status,noise_relaible);
			rtd_pr_vpq_info("mo_i=");
			for(i=0;i<10;i++)
				printk("%d,",motioni[i]);
			printk("\n");
		}
	}
}

void Scaler_scalerVIP_compute_level_I_002_RTNR0_tv002(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	unsigned int printf_Flag=0, print_delay=200;
	unsigned char Lv=0,i;
	unsigned short Lv_intp=0;
	int still_idx, level_Idx, Still_offset, VD_Signal_Status_value_avg;

	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_Input_VD_Status_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_VD_noise_status][0]);
	unsigned int *pPQA_Input_FMV_Hist_stillIdx_th_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_FMV_Hist_stillIdx_th][0]);
	unsigned int *pPQA_Input_FMV_Hist_stillIdx_offset_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_FMV_Hist_motionIdx_offset][0]);

	//unsigned int *pPQA_FMV_Hist_motionIdx_offset_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_FMV_Hist_motionIdx_offset][0]);
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];

	VD_Signal_Status_value_avg = SmartPic_clue->VD_Signal_Status_value_avg;
	level_Idx = SmartPic_clue->VD_Signal_Status_value_avg;
	still_idx = Scaler_scalerVIP_compute_Still_Idx_FMV_HMC_I_002(vipTable_shareMem, SmartPic_clue, system_info_structure_table, MA_print_count);

	Still_offset = pPQA_Input_FMV_Hist_stillIdx_offset_TBL[PQA_I_L09];
	for (i=0;i<PQA_I_L09;i++)
		if (still_idx < pPQA_Input_FMV_Hist_stillIdx_th_TBL[i]) {
			Still_offset = pPQA_Input_FMV_Hist_stillIdx_offset_TBL[i];
			break;
	}
	level_Idx = ((level_Idx - Still_offset) < 0)?(0):(level_Idx - Still_offset);

	Lv = PQA_I_L09;
	for (i=0;i<PQA_I_L09;i++) {
		if (level_Idx < pPQA_Input_VD_Status_TBL[i]) {
			Lv = i;
			break;
		}
	}

	Lv_intp =0;

	PQA_Input_Level[I_002_RTNR_0] = Lv;
	PQA_Input_rate[I_002_RTNR_0] = Lv_intp;

	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = VD_Signal_Status_value_avg;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_1] = still_idx;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_2] = level_Idx;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_3] = Lv;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_4] = Lv_intp;

	printf_Flag = system_info_structure_table->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//for debug.
	if(((printf_Flag&_BIT3)!=0))
	{
		if(MA_print_count%print_delay==0)
		{
			rtd_pr_vpq_info("rm0~VD_Signal_Status_value_avg=%d, still_idx=%d, level_Idx=%d, Lv=%d, Lv_intp=%d\n",
				VD_Signal_Status_value_avg, still_idx, level_Idx,Lv, Lv_intp);
		}
	}

}

//====================================================================================
//****************************************************PQA****************************************************
//====================================================================================
