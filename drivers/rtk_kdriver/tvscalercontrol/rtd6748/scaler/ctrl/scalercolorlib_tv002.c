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

#ifdef CONFIG_CUSTOMER_TV002
#include <rtk_gpio.h>
#include <rtk_gpio-dev.h>
#include <pwm/rtk_pwm_func_tv002.h>
#endif

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
	#include <scalercommon/scalerDrvCommon.h>
#endif

#include "memc_isr/scalerMEMC.h"
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <scaler/vipRPCCommon.h>
#include <scaler_vscdev.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <rtd_log/rtd_module_log.h>
#include <tvscalercontrol/vip/SIP.h>
#include "scaler_vpqmemcdev.h"
#include <tvscalercontrol/vip/localcontrast.h>
#include "vgip_isr/scalerDI.h"
#include "rtk_vip_logger.h"


/*******************************************************************************
* Macro
******************************************************************************/
#undef printf
#define printf(fmt, ...)

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
#ifdef VIP_SUPPORT_SIP_1_SPI_2
static VIP_SIP_DMA_CTRL SIP_DMA_CTRL_SIP_1 = {0};
static VIP_SIP_DMA_CTRL SIP_DMA_CTRL_SIP_2 = {0};
#endif
unsigned char g_BLE_save = 0;
char g_sGamma_save = 0;
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

extern unsigned int ddomain_print_count;
#define BL_DBC_Print(printCount, fmt, args...)	VIPprintfPrivateCount(KERN_INFO, VIP_LOGMODULE_VGIP_IP25_interface1_DEBUG, printCount, fmt, ##args)
#define BL_DBC_Print2(printCount, fmt, args...)	VIPprintfPrivateCount(KERN_INFO, VIP_LOGMODULE_VGIP_IP25_interface2_DEBUG, printCount, fmt, ##args)
#define BL_DBC_Print3(printCount, fmt, args...)	VIPprintfPrivateCount(KERN_INFO, VIP_LOGMODULE_VGIP_IP25_interface3_DEBUG, printCount, fmt, ##args)

//#undef VIPprintf
//#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VIP_COLOR_scaler_DEBUG, fmt, ##args)

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

void Scaler_set_Init_TV002(unsigned char mode)
{
	extern unsigned char ColorFMT_601_in_IMD_Domain_Enable;
	extern unsigned char set_DCR_PWM_Enable;
	extern unsigned short Power22InvGamma[1025];
	extern DRV_RPC_AutoMA_Flag S_RPC_AutoMA_Flag;
	_system_setting_info *VIP_system_info_structure_table=NULL;

	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL)
	{
		rtd_pr_vpq_emerg("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}

	Scaler_access_Project_TV002_Style(1, mode);
	ColorFMT_601_in_IMD_Domain_Enable = 1;
	set_DCR_PWM_Enable = 0;
	//VIP_system_info_structure_table->OSD_Info.OSD_Backlight = 100;

	//gamma and inv-gamma
	fwif_color_set_InvGamma(SLR_MAIN_DISPLAY, 1, Power22InvGamma, Power22InvGamma, Power22InvGamma);
	//void fwif_set_gamma(unsigned char Gamma_Mode, unsigned char Gamma_Sindex, unsigned char Gamma_Shigh, unsigned char Gamma_Slow)

	// disable dynamic V_LPF
	S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_Dynamic_Vertical_NR_En = 0;

}

// Tv002 picture mode function
void Scaler_set_Intelligent_Picture_Enable(unsigned char En_flag)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	SLR_PICTURE_MODE_DATA *pData=NULL;
	_system_setting_info *VIP_system_info_structure_table=NULL;
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;

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

	//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = Scaler_get_tv002_Custom_struct();
	if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}

	if(En_flag==1 && (VIP_system_info_structure_table->Input_src_Type == _SRC_TV ||VIP_system_info_structure_table->Input_src_Type == _SRC_CVBS))
	{
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 1;		//marked future?, elieli
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
	//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = Scaler_get_tv002_Custom_struct();;

	if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}
	return p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.En_Flag;
}

void Scaler_set_Intelligent_Picture_Optimisation(unsigned char Level)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = Scaler_get_tv002_Custom_struct();;

	if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}

	p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.PICTURE_OPTIMISATION = Level;
}

unsigned char Scaler_get_Intelligent_Picture_Optimisation(void)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = Scaler_get_tv002_Custom_struct();;

	if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}
	return p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.PICTURE_OPTIMISATION;
}

void Scaler_set_Intelligent_Picture_Signal_Level_Indicator_ONOFF(unsigned char En_Flag)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = Scaler_get_tv002_Custom_struct();;

	if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}
	p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.Sig_Lv_Indicator_En = En_Flag;

}

unsigned char Scaler_Intelligent_Picture_get_Signal_Level_Indicator_ONOFF(void)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = Scaler_get_tv002_Custom_struct();;

	if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}

	return p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.Sig_Lv_Indicator_En;

}

unsigned char Scaler_Intelligent_Picture_get_Intelligent_Picture_Signal_Level(void)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	_clues *smartPic_clue = fwif_color_Get_SmartPic_clue();

	if(smartPic_clue == NULL){
		return 0;
	}

	p_Share_Memory_VIP_TABLE_Custom_Struct = Scaler_get_tv002_Custom_struct();;

	if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}

	p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.Sig_Lv = PQA_I_LEVEL_MAX - smartPic_clue->PQA_ISR_Cal_Info.Input_Level[I_005_RFCVBS]; // temporily code for signal level, use "I_005_RFCVBS"  in main trunk

	return p_Share_Memory_VIP_TABLE_Custom_Struct->INTELLIGENT_PICTURE_Function.Sig_Lv;
}

VIP_MAGIC_GAMMA_Function* Scaler_get_Magic_Gamma(void)
{
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = Scaler_get_tv002_Custom_struct();;

	if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return NULL;
	}
	return &(p_Share_Memory_VIP_TABLE_Custom_Struct->MAGIC_GAMMA_Function);

}

void Scaler_set_BP_initial(VIP_SOURCE_TIMING VIP_SrcTiming, PICTURE_MODE Pic_Mode)
{
	unsigned short table_idx;
	unsigned char idx8;
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = Scaler_get_tv002_Custom_struct();;

	if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}
 	table_idx = p_Share_Memory_VIP_TABLE_Custom_Struct->BP_Function.BP_Ctrl.BP_Level_TBL_Select;
 	idx8 = fwif_color_ChangeOneUINT16Endian(table_idx, 0);
	table_idx = p_Share_Memory_VIP_TABLE_Custom_Struct->BP_Function.BP_Ctrl.BP_Level_TBL[idx8][(unsigned char)Pic_Mode];
 	p_Share_Memory_VIP_TABLE_Custom_Struct->BP_Function.BP_Ctrl.BP_TBL_Select = table_idx;
	idx8 = fwif_color_ChangeOneUINT16Endian(table_idx, 0);
	p_Share_Memory_VIP_TABLE_Custom_Struct->BP_Function.BP_Table[idx8].Smooth_Change.Init_Flag = 0xFFFF;
}

unsigned char Scaler_Get_ADV_API_LEVEL_TBL_Idx(VIP_SOURCE_TIMING VIP_SrcTiming, VIP_ADV_API_LEVEL API_Level)
{
	unsigned char ret_idx, idx;
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
	//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = Scaler_get_tv002_Custom_struct();;

	if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}

	idx = p_Share_Memory_VIP_TABLE_Custom_Struct->VIP_QUALITY_Coef_TV002[(unsigned char)VIP_SrcTiming][VIP_QUALITY_FUNCTION_TV002_ADV_API];
	ret_idx = p_Share_Memory_VIP_TABLE_Custom_Struct->ADV_API.VIP_ADV_API_Level_TBL[idx][(unsigned char)API_Level];
	return ret_idx;
};

char Scaler_get_sGamma(void)
{
	return g_sGamma_save;
}

void Scaler_set_sGamma(char level_index)
{
	unsigned char table_idx;
	VIP_ADV_API_LEVEL API_Level;
	VIP_SOURCE_TIMING VIP_SrcTiming;
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
//	_system_setting_info *VIP_system_info_structure_table=NULL;
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
	pData->adv_gamma= level_index;
*/
	g_sGamma_save = level_index;
	//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = Scaler_get_tv002_Custom_struct();;

	if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}


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
	table_idx = Scaler_Get_ADV_API_LEVEL_TBL_Idx(VIP_SrcTiming, API_Level);
	p_Share_Memory_VIP_TABLE_Custom_Struct->ADV_API.ADV_API_CTRL.TBL_Select_sGamma = table_idx;
	fwif_color_set_DCC_SC_for_Coef_Change(1,1);

};

unsigned char Scaler_get_BLE(void)
{
	return g_BLE_save;
}

void Scaler_set_BLE(unsigned char level_index)
{
	unsigned char table_idx;
	VIP_ADV_API_LEVEL API_Level;
	VIP_SOURCE_TIMING VIP_SrcTiming;
	SLR_VIP_TABLE_CUSTOM_TV002 *p_Share_Memory_VIP_TABLE_Custom_Struct=NULL;
//	_system_setting_info *VIP_system_info_structure_table=NULL;
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
	g_BLE_save = level_index;

	//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;
	//p_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM_TV002*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	p_Share_Memory_VIP_TABLE_Custom_Struct = Scaler_get_tv002_Custom_struct();;

	if (p_Share_Memory_VIP_TABLE_Custom_Struct == NULL)
	{
		printf("~get p_Share_Memory_VIP_TABLE_Custom_Struct Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}


	RPC_VIP_system_info_structure_table = (_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (RPC_VIP_system_info_structure_table == NULL)
//	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
//	if (VIP_system_info_structure_table == NULL)
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

	table_idx = Scaler_Get_ADV_API_LEVEL_TBL_Idx(VIP_SrcTiming, API_Level);
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
	#ifndef CONFIG_CUSTOMER_TV002
	fwif_color_set_live_colour(level_index);
	#endif
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

short Scaler_Cal_Clarity_Resolution_TV002(unsigned char items, unsigned char tbl_idx)
{
	short gain = 0;
	//unsigned short value = Scaler_GetSharpnessTable();
	short max = 0, min = 0, OSD = 0, Base = 0, tbl_gain = 0;

	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		rtd_pr_vpq_emerg("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	OSD = Scaler_get_Clarity_Resolution();
	switch(items) {
	// 2D Peaking
	case Cal_Clarity_Resolution_RING_GEN_GAIN_TV002:
		if (tbl_idx < Sharp_table_num) {
			Base = gVip_Table->Ddomain_SHPTable[tbl_idx].TwoD_Peak_AdaptCtrl2.OSD_SuperRes;
			max = gVip_Table->Ddomain_SHPTable[tbl_idx].SR_Control.Ring_Gen_Gain_Max;
			min = gVip_Table->Ddomain_SHPTable[tbl_idx].SR_Control.Ring_Gen_Gain_Min;
			tbl_gain = gVip_Table->Ddomain_SHPTable[tbl_idx].SR_Control.Ring_Gen_Gain;
		}
		break;
	// MB Peaking H
	case Cal_Clarity_Resolution_MB_Peaking_H_Gain_Pos_TV002:
		if (tbl_idx < MBPK_table_num) {
			Base = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Table.MB_Gain_Pos_Max;
			min = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Table.MB_Gain_Pos_Min;
			tbl_gain = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Table.MB_Gain_Pos;
		}
		break;

	case Cal_Clarity_Resolution_MB_Peaking_H_Gain_Neg_TV002:
		if (tbl_idx < MBPK_table_num) {
			Base = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Table.MB_Gain_Neg_Max;
			min = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Table.MB_Gain_Neg_Min;
			tbl_gain = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Table.MB_Gain_Neg;
		}
		break;

	case Cal_Clarity_Resolution_MB_Peaking_H_LV_TV002:
		if (tbl_idx < MBPK_table_num) {
			Base = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Table.MB_LV_Max;
			min = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Table.MB_LV_Min;
			tbl_gain = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Table.MB_LV;
		}
		break;

	case Cal_Clarity_Resolution_MB_Peaking_H_Sy_P0_TV002:
		if (tbl_idx < MBPK_table_num) {
			Base = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Deovershoot1.y0_Max;
			min = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Deovershoot1.y0_Min;
			tbl_gain = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Deovershoot1.y0;
		}
		break;

	case Cal_Clarity_Resolution_MB_Peaking_H_Sy_P1_TV002:
		if (tbl_idx < MBPK_table_num) {
			Base = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Deovershoot1.y1_Max;
			min = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Deovershoot1.y1_Min;
			tbl_gain = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Deovershoot1.y1;
		}
		break;

	case Cal_Clarity_Resolution_MB_Peaking_H_Sy_P2_TV002:
		if (tbl_idx < MBPK_table_num) {
			Base = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Deovershoot1.y2_Max;
			min = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Deovershoot1.y2_Min;
			tbl_gain = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_H_Deovershoot1.y2;
		}
		break;
	// MB Peaking V
	case Cal_Clarity_Resolution_MB_Peaking_V_Gain_Pos_TV002:
		if (tbl_idx < MBPK_table_num) {
			Base = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Table.MB_Gain_Pos_V_Max;
			min = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Table.MB_Gain_Pos_V_Min;
			tbl_gain = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Table.MB_Gain_Pos_V;
		}
		break;

	case Cal_Clarity_Resolution_MB_Peaking_V_Gain_Neg_TV002:
		if (tbl_idx < MBPK_table_num) {
			Base = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Table.MB_Gain_Neg_V_Max;
			min = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Table.MB_Gain_Neg_V_Min;
			tbl_gain = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Table.MB_Gain_Neg_V;
		}
		break;

	case Cal_Clarity_Resolution_MB_Peaking_V_LV_TV002:
		if (tbl_idx < MBPK_table_num) {
			Base = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Table.MB_LV_V_Max;
			min = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Table.MB_LV_V_Min;
			tbl_gain = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Table.MB_LV_V;
		}
		break;

	case Cal_Clarity_Resolution_MB_Peaking_V_Sy_P0_TV002:
		if (tbl_idx < MBPK_table_num) {
			Base = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Deovershoot1.y0_Max;
			min = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Deovershoot1.y0_Min;
			tbl_gain = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Deovershoot1.y0;
		}
		break;

	case Cal_Clarity_Resolution_MB_Peaking_V_Sy_P1_TV002:
		if (tbl_idx < MBPK_table_num) {
			Base = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Deovershoot1.y1_Max;
			min = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Deovershoot1.y1_Min;
			tbl_gain = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Deovershoot1.y1;
		}
		break;

	case Cal_Clarity_Resolution_MB_Peaking_V_Sy_P2_TV002:
		if (tbl_idx < MBPK_table_num) {
			Base = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Deovershoot1.y2_Max;
			min = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Deovershoot1.y2_Min;
			tbl_gain = gVip_Table->Idomain_MBPKTable[tbl_idx].MBPK_V_Deovershoot1.y2;
		}
		break;
	// SU Peaking H
	case Cal_Clarity_Resolution_SU_Peaking_H_Gain_Pos_TV002:
		if (tbl_idx < MBSU_table_num) {
			Base = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Table.MB_Gain_Pos_Max;
			min = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Table.MB_Gain_Pos_Min;
			tbl_gain = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Table.MB_Gain_Pos;
		}
		break;

	case Cal_Clarity_Resolution_SU_Peaking_H_Gain_Neg_TV002:
		if (tbl_idx < MBSU_table_num) {
			Base = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Table.MB_Gain_Neg_Max;
			min = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Table.MB_Gain_Neg_Min;
			tbl_gain = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Table.MB_Gain_Neg;
		}
		break;

	case Cal_Clarity_Resolution_SU_Peaking_H_LV_TV002:
		if (tbl_idx < MBSU_table_num) {
			Base = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Table.MB_LV_Max;
			min = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Table.MB_LV_Min;
			tbl_gain = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Table.MB_LV;
		}
		break;

	case Cal_Clarity_Resolution_SU_Peaking_H_Sy_P0_TV002:
		if (tbl_idx < MBSU_table_num) {
			Base = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Deovershoot1.y0_Max;
			min = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Deovershoot1.y0_Min;
			tbl_gain = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Deovershoot1.y0;
		}
		break;

	case Cal_Clarity_Resolution_SU_Peaking_H_Sy_P1_TV002:
		if (tbl_idx < MBSU_table_num) {
			Base = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Deovershoot1.y1_Max;
			min = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Deovershoot1.y1_Min;
			tbl_gain = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Deovershoot1.y1;
		}
		break;

	case Cal_Clarity_Resolution_SU_Peaking_H_Sy_P2_TV002:
		if (tbl_idx < MBSU_table_num) {
			Base = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Deovershoot1.y2_Max;
			min = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Deovershoot1.y2_Min;
			tbl_gain = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_H_Deovershoot1.y2;
		}
		break;
	// MB Peaking V
	case Cal_Clarity_Resolution_SU_Peaking_V_Gain_Pos_TV002:
		if (tbl_idx < MBSU_table_num) {
			Base = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Table.MB_Gain_Pos_V_Max;
			min = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Table.MB_Gain_Pos_V_Min;
			tbl_gain = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Table.MB_Gain_Pos_V;
		}
		break;

	case Cal_Clarity_Resolution_SU_Peaking_V_Gain_Neg_TV002:
		if (tbl_idx < MBSU_table_num) {
			Base = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Table.MB_Gain_Neg_V_Max;
			min = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Table.MB_Gain_Neg_V_Min;
			tbl_gain = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Table.MB_Gain_Neg_V;
		}
		break;

	case Cal_Clarity_Resolution_SU_Peaking_V_LV_TV002:
		if (tbl_idx < MBSU_table_num) {
			Base = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Table.MB_LV_V_Max;
			min = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Table.MB_LV_V_Min;
			tbl_gain = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Table.MB_LV_V;
		}
		break;

	case Cal_Clarity_Resolution_SU_Peaking_V_Sy_P0_TV002:
		if (tbl_idx < MBSU_table_num) {
			Base = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Deovershoot1.y0_Max;
			min = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Deovershoot1.y0_Min;
			tbl_gain = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Deovershoot1.y0;
		}
		break;

	case Cal_Clarity_Resolution_SU_Peaking_V_Sy_P1_TV002:
		if (tbl_idx < MBSU_table_num) {
			Base = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Deovershoot1.y1_Max;
			min = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Deovershoot1.y1_Min;
			tbl_gain = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Deovershoot1.y1;
		}
		break;

	case Cal_Clarity_Resolution_SU_Peaking_V_Sy_P2_TV002:
		if (tbl_idx < MBSU_table_num) {
			Base = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_Ctrl.OSD_SuperRes;
			max = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Deovershoot1.y2_Max;
			min = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Deovershoot1.y2_Min;
			tbl_gain = gVip_Table->Ddomain_MBSUTable[tbl_idx].MBPK_V_Deovershoot1.y2;
		}
		break;
	default:
		rtd_pr_vpq_emerg("~get Error return, %s->%d, %s~, items=%d,\n", __FILE__, __LINE__, __FUNCTION__, items);
		break;
	}

	if ((max == 0) && (min == 0)) {	// SR function is disabled by TBL
		gain = tbl_gain;
	} else {
		if ((OSD <= 0) || (Scaler_get_Clarity_RealityCreation() == 0)) {	// SR = off
			gain = min;
		} else if (Scaler_get_Clarity_RealityCreation() == 1) {	// SR = Auto
			gain = tbl_gain;
		} else if (OSD >= 100) {
			gain = max;
		} else {
			if (OSD < Base) {
				gain = (tbl_gain - min) * OSD / Base;
			} else {
				gain = (max - tbl_gain) * (OSD - Base) / (100 - Base) + tbl_gain;
			}
		}
	}
	//rtd_pr_vpq_info("SR, gain=%d, Base=%d, max=%d, min=%d, tbl_gain=%d, items=%d,\n", gain, Base, max, min, tbl_gain, items);
	return gain;
}

void Scaler_Set_Clarity_Resolution_TV002(void)
{
	unsigned short idx_2D_peaking, idx_MB_Peaking, idx_SU_Peaking;
	unsigned char VIP_SrcTiming;
	_RPC_system_setting_info *RPC_VIP_system_info_structure_table=NULL;
	SLR_VIP_TABLE *gVip_Table = NULL;
	RPC_VIP_system_info_structure_table = (_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (RPC_VIP_system_info_structure_table == NULL)
	{
		rtd_pr_vpq_emerg("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}
	VIP_SrcTiming = RPC_VIP_system_info_structure_table->VIP_source;

	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		rtd_pr_vpq_emerg("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	idx_2D_peaking = gVip_Table->VIP_QUALITY_Extend3_Coef[VIP_SrcTiming][VIP_QUALITY_FUNCTION_SharpTable];
	idx_MB_Peaking = gVip_Table->VIP_QUALITY_Extend3_Coef[VIP_SrcTiming][VIP_QUALITY_FUNCTION_MB_Peaking];
	idx_SU_Peaking = gVip_Table->VIP_QUALITY_Extend3_Coef[VIP_SrcTiming][VIP_QUALITY_FUNCTION_MB_SU_Peaking];

	Scaler_SetSharpnessTable(idx_2D_peaking);
	Scaler_SetMBPeaking(idx_MB_Peaking);
	Scaler_SetMBSUPeaking(idx_SU_Peaking);
	Scaler_SetSharpness(Scaler_GetSharpness());

}

void Scaler_Cal_2D_Peaking_Clarity_Resolution_TV002(DRV_Sharpness_Table *pSharpness_Table, unsigned char idx)
{
	if (pSharpness_Table == NULL) {
		rtd_pr_vpq_emerg("~TBL=NULL Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}

	pSharpness_Table->SR_Control.Ring_Gen_Gain = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_RING_GEN_GAIN_TV002, idx);
}

void Scaler_Cal_MB_Peaking_Clarity_Resolution_TV002(VIP_MBPK_Table *pMBPK, unsigned char value)
{
	short x0, x1, x2, y0, y1, y2;
	if (pMBPK == NULL) {
		rtd_pr_vpq_emerg("~TBL=NULL Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}

	// H
	pMBPK->MBPK_H_Table.MB_Gain_Pos = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_MB_Peaking_H_Gain_Pos_TV002, value);
	pMBPK->MBPK_H_Table.MB_Gain_Neg = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_MB_Peaking_H_Gain_Neg_TV002, value);
	pMBPK->MBPK_H_Table.MB_LV = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_MB_Peaking_H_LV_TV002, value);
	pMBPK->MBPK_H_Deovershoot1.y0 = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_MB_Peaking_H_Sy_P0_TV002, value);
	pMBPK->MBPK_H_Deovershoot1.y1 = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_MB_Peaking_H_Sy_P1_TV002, value);
	pMBPK->MBPK_H_Deovershoot1.y2 = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_MB_Peaking_H_Sy_P2_TV002, value);
	// slope need to re-calculate
	x0 = pMBPK->MBPK_H_Deovershoot1.x0;
	x1 = pMBPK->MBPK_H_Deovershoot1.x1;
	x2 = pMBPK->MBPK_H_Deovershoot1.x2;
	y0 = pMBPK->MBPK_H_Deovershoot1.y0;
	y1 = pMBPK->MBPK_H_Deovershoot1.y1;
	y2 = pMBPK->MBPK_H_Deovershoot1.y2;
	pMBPK->MBPK_H_Deovershoot1.a0 = (x1 <= x0)?(0):((y1 - y0) * 32 / (x1 - x0));
	pMBPK->MBPK_H_Deovershoot1.a1 = (x2 <= x1)?(0):((y2 - y1) * 32 / (x2 - x1));

	// V
	pMBPK->MBPK_V_Table.MB_Gain_Pos_V = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_MB_Peaking_V_Gain_Pos_TV002, value);
	pMBPK->MBPK_V_Table.MB_Gain_Neg_V = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_MB_Peaking_V_Gain_Neg_TV002, value);
	pMBPK->MBPK_V_Table.MB_LV_V = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_MB_Peaking_V_LV_TV002, value);
	pMBPK->MBPK_V_Deovershoot1.y0 = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_MB_Peaking_V_Sy_P0_TV002, value);
	pMBPK->MBPK_V_Deovershoot1.y1 = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_MB_Peaking_V_Sy_P1_TV002, value);
	pMBPK->MBPK_V_Deovershoot1.y2 = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_MB_Peaking_V_Sy_P2_TV002, value);
	// slope need to re-calculate
	x0 = pMBPK->MBPK_V_Deovershoot1.x0;
	x1 = pMBPK->MBPK_V_Deovershoot1.x1;
	x2 = pMBPK->MBPK_V_Deovershoot1.x2;
	y0 = pMBPK->MBPK_V_Deovershoot1.y0;
	y1 = pMBPK->MBPK_V_Deovershoot1.y1;
	y2 = pMBPK->MBPK_V_Deovershoot1.y2;
	pMBPK->MBPK_V_Deovershoot1.a0 = (x1 <= x0)?(0):((y1 - y0) * 32 / (x1 - x0));
	pMBPK->MBPK_V_Deovershoot1.a1 = (x2 <= x1)?(0):((y2 - y1) * 32 / (x2 - x1));

}

void Scaler_Cal_SU_Peaking_Clarity_Resolution_TV002(VIP_MBPK_Table *pMBPK, unsigned char value)
{
	short x0, x1, x2, y0, y1, y2;
	if (pMBPK == NULL) {
		rtd_pr_vpq_emerg("~TBL=NULL Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return;
	}

	// H
	pMBPK->MBPK_H_Table.MB_Gain_Pos = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_SU_Peaking_H_Gain_Pos_TV002, value);
	pMBPK->MBPK_H_Table.MB_Gain_Neg = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_SU_Peaking_H_Gain_Neg_TV002, value);
	pMBPK->MBPK_H_Table.MB_LV = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_SU_Peaking_H_LV_TV002, value);
	pMBPK->MBPK_H_Deovershoot1.y0 = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_SU_Peaking_H_Sy_P0_TV002, value);
	pMBPK->MBPK_H_Deovershoot1.y1 = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_SU_Peaking_H_Sy_P1_TV002, value);
	pMBPK->MBPK_H_Deovershoot1.y2 = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_SU_Peaking_H_Sy_P2_TV002, value);
	// slope need to re-calculate
	x0 = pMBPK->MBPK_H_Deovershoot1.x0;
	x1 = pMBPK->MBPK_H_Deovershoot1.x1;
	x2 = pMBPK->MBPK_H_Deovershoot1.x2;
	y0 = pMBPK->MBPK_H_Deovershoot1.y0;
	y1 = pMBPK->MBPK_H_Deovershoot1.y1;
	y2 = pMBPK->MBPK_H_Deovershoot1.y2;
	pMBPK->MBPK_H_Deovershoot1.a0 = (x1 <= x0)?(0):((y1 - y0) * 32 / (x1 - x0));
	pMBPK->MBPK_H_Deovershoot1.a1 = (x2 <= x1)?(0):((y2 - y1) * 32 / (x2 - x1));

	// V
	pMBPK->MBPK_V_Table.MB_Gain_Pos_V = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_SU_Peaking_V_Gain_Pos_TV002, value);
	pMBPK->MBPK_V_Table.MB_Gain_Neg_V = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_SU_Peaking_V_Gain_Neg_TV002, value);
	pMBPK->MBPK_V_Table.MB_LV_V = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_SU_Peaking_V_LV_TV002, value);
	pMBPK->MBPK_V_Deovershoot1.y0 = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_SU_Peaking_V_Sy_P0_TV002, value);
	pMBPK->MBPK_V_Deovershoot1.y1 = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_SU_Peaking_V_Sy_P1_TV002, value);
	pMBPK->MBPK_V_Deovershoot1.y2 = Scaler_Cal_Clarity_Resolution_TV002(Cal_Clarity_Resolution_SU_Peaking_V_Sy_P2_TV002, value);
	// slope need to re-calculate
	x0 = pMBPK->MBPK_V_Deovershoot1.x0;
	x1 = pMBPK->MBPK_V_Deovershoot1.x1;
	x2 = pMBPK->MBPK_V_Deovershoot1.x2;
	y0 = pMBPK->MBPK_V_Deovershoot1.y0;
	y1 = pMBPK->MBPK_V_Deovershoot1.y1;
	y2 = pMBPK->MBPK_V_Deovershoot1.y2;
	pMBPK->MBPK_V_Deovershoot1.a0 = (x1 <= x0)?(0):((y1 - y0) * 32 / (x1 - x0));
	pMBPK->MBPK_V_Deovershoot1.a1 = (x2 <= x1)?(0):((y2 - y1) * 32 / (x2 - x1));

}


unsigned char Scaler_get_Clarity_Mastered4K(void)
{
	return g_Clarity_Masterd4K_save;
}

void Scaler_set_Clarity_Mastered4K(unsigned char level)
{
	g_Clarity_Masterd4K_save = level;
}

/************************************************************/
/******************* For TV002 use*****************************/
/************************************************************/
//TV002 PQ function
SLR_VIP_TABLE_CUSTOM_TV002* Scaler_get_tv002_Custom_struct(void)
{
	extern  SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;

	return &m_customVipTable_demo_tv002;
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

	//return custom_tv002->ZERO_D.ZERO_D_STATUS.Zero_D_Backlight;
	return custom_tv002->DBC.DBC_STATUS.DBC_Backlight;
}

char Scaler_set_DBC_UI_blacklight_Value_TV002(unsigned char value)
{
	//SLR_VIP_TABLE_CUSTOM_TV002 *VIP_TABLE_CUSTOM_TV002  = NULL;
	//VIP_TABLE_CUSTOM_TV002 = Scaler_get_tv002_Custom_struct();
	_system_setting_info *VIP_system_info_structure_table=NULL;
	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (value > 100)
		value = 100;
#if 0
	if (VIP_TABLE_CUSTOM_TV002 == NULL) {
		rtd_pr_vpq_emerg("TV002_get_customer_TBL=NULL!!!\n");
		return -1;
	}

	VIP_TABLE_CUSTOM_TV002->DBC.DBC_STATUS.OSD_Backlight = value;
#endif
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
#if 0	/* use VPQ_EXTERN_IOC_SET_GDBC_MODE*/
	SLR_VIP_TABLE_CUSTOM_TV002 *VIP_TABLE_CUSTOM_TV002  = NULL;
	VIP_TABLE_CUSTOM_TV002 = Scaler_get_tv002_Custom_struct();

	if (VIP_TABLE_CUSTOM_TV002 == NULL) {
		rtd_pr_vpq_emerg("TV002_get_customer_TBL=NULL!!!\n");
		return -1;
	}

	if (value > D_PS_MODE_MAX)
		value = 0;

	VIP_TABLE_CUSTOM_TV002->DBC.DBC_STATUS.OSD_PowerSave = value;
#endif
	return 0;
}

char Scaler_set_UI_Picture_Mode_TV002(unsigned char value)
{
	//SLR_VIP_TABLE_CUSTOM_TV002 *VIP_TABLE_CUSTOM_TV002  = NULL;
	_system_setting_info *VIP_system_info_structure_table=NULL;

	//VIP_TABLE_CUSTOM_TV002 = Scaler_get_tv002_Custom_struct();
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
	unsigned char Debug_Log;
	unsigned char Debug_Log_Delay;

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
	unsigned char Debug_Log;
	unsigned char Debug_Log_Delay;

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
	unsigned short APL = 0;
	unsigned char DBC_APL_LOW_START_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_APL_LOW_START;
	unsigned char DBC_BL_B_LIMIT_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_BL_B_LIMIT;
	unsigned char DBC_APL_LOW_LIMIT_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_APL_LOW_LIMIT;
	unsigned char DBC_PS_APL_TH_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_PS_APL_TH;
	unsigned char DBC_PS_LEVEL_Reg = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_PS_LEVEL;
	unsigned char BASIC_BLIGHT = vipCustomTBL_ShareMem->DBC.DBC_STATUS.Basic_Backlight;
	//unsigned char PictureMode = system_info_structure_table->OSD_Info.OSD_DCR_Mode;
	VIP_DBC_POWER_SAVING_MODE PictureMode = (VIP_DBC_POWER_SAVING_MODE)vipCustomTBL_ShareMem->DBC.DBC_STATUS.OSD_PowerSave;

	// for Debug info
	unsigned char Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Log0;
	unsigned char Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Delay_Flame;

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
	unsigned char Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Log0;
	unsigned char Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Delay_Flame;

	vipCustomTBL_ShareMem->DBC.DBC_STATUS.Y_MAX = Y_MAX;
	vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DBC = DBC;

	if (vipCustomTBL_ShareMem->DBC.DBC_CTRL.BL_Debug_ENA != 0)
		Y_MAX = vipCustomTBL_ShareMem->DBC.DBC_CTRL.BL_Debug_ENA;

	backlight = MAX(DBC, Y_MAX);

	if(BASIC_BLIGHT < backlight)
		backlight = BASIC_BLIGHT;

	// for Debug info
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

void Scaler_scalerVIP_DBC_CTRL_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table, _clues *SmartPic_clue, unsigned int MA_print_count)
{
#if 0	/* use APL/Dev LUT for DBC*/
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
	unsigned char Debug_Log;
	unsigned char Debug_Log_Delay;
	unsigned char DBC, OSD_BL;
	unsigned char APL;
	unsigned char Dev;
	unsigned char power_saving_mode;
	char DCC_S_High_CMPS, DCC_S_Index_CMPS;

	if (((vipCustomTBL_ShareMem->DBC.DBC_CTRL.BL_Debug_ENA) & _BIT1) != 0) {	/* BIT1 = 1, force APL and DEV by tool*/
		APL = vipCustomTBL_ShareMem->DBC.DBC_STATUS.APL;
		Dev = vipCustomTBL_ShareMem->DBC.DBC_STATUS.DEV;
	} else {	/* info recorde*/
		APL = SmartPic_clue->Adapt_DCC_Info.Adapt_Mean_Dev_info[DCC_Tool_mean];
		Dev = SmartPic_clue->Adapt_DCC_Info.Adapt_Mean_Dev_info[DCC_Tool_Dev];
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.APL = APL;
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.DEV = Dev;
	}

	if (((vipCustomTBL_ShareMem->DBC.DBC_CTRL.BL_Debug_ENA) & _BIT2) != 0) {	/* BIT2 = 1, force power saving mode by tool*/
		power_saving_mode = vipCustomTBL_ShareMem->DBC.DBC_STATUS.OSD_PowerSave;
	} else {	/* info recorde*/
		power_saving_mode = system_info_structure_table->OSD_Info.OSD_DCR_Mode;
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.OSD_PowerSave = power_saving_mode;
	}


	if (vipCustomTBL_ShareMem->DBC.DBC_CTRL.DBC_Enable == 1) {
		DBC = Scaler_scalerVIP_DBC_LUT_TV002(vipCustomTBL_ShareMem, APL, Dev, MA_print_count);
		/* recorde info for DBC tool*/
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DBC = DBC;

		DBC = Scaler_scalerVIP_DBC_Smooth_TV002(DBC, vipCustomTBL_ShareMem, system_info_structure_table, SmartPic_clue, MA_print_count);
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_Smooth = DBC;

		DCC_S_High_CMPS = Scaler_scalerVIP_DBC_DCC_S_High_Compensation_TV002(vipCustomTBL_ShareMem, APL, Dev, power_saving_mode, MA_print_count);
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DCC_S_High_CMPS_LUT = DCC_S_High_CMPS;

		DCC_S_Index_CMPS = Scaler_scalerVIP_DBC_DCC_S_Index_Compensation_TV002(vipCustomTBL_ShareMem, APL, Dev, power_saving_mode, MA_print_count);
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DCC_S_Index_CMPS_LUT = DCC_S_Index_CMPS;

	} else {
		DBC = 255;
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DBC = 255;
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_Smooth = 255;
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DCC_S_High_CMPS_LUT = 0;
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DCC_S_Index_CMPS_LUT = 0;
		DCC_S_High_CMPS = 0;
		DCC_S_Index_CMPS = 0;
	}

	if (((vipCustomTBL_ShareMem->DBC.DBC_CTRL.BL_Debug_ENA) & _BIT3) != 0) {	/* BIT3 = 1, force UI backlight value by tool*/
		OSD_BL = vipCustomTBL_ShareMem->DBC.DBC_STATUS.OSD_Backlight	;
	} else {
		/* recorde info for DBC tool*/
		OSD_BL = system_info_structure_table->OSD_Info.OSD_Backlight;
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.OSD_Backlight = OSD_BL;
	}

	if (((vipCustomTBL_ShareMem->DBC.DBC_CTRL.BL_Debug_ENA) & _BIT0) != 0) {	/* BIT0 = 1, force control DBC_Backlight by tool*/
		;
	} else {
		vipCustomTBL_ShareMem	->DBC.DBC_STATUS.DBC_Backlight = (DBC - (DBC_OSD_BL_MAX - OSD_BL))<(0)?(0):(DBC - (DBC_OSD_BL_MAX - OSD_BL));
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DCC_S_High_CMPS = DCC_S_High_CMPS;
		vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DCC_S_Index_CMPS = DCC_S_Index_CMPS;
	}
	// can both use VPQ_EXTERN_IOC_GET_DCR_BACKLIGHT and VPQ_EXTERN_IOC_GET_TV002_DBC_BACKLIGHT to get DCR value
	SmartPic_clue->DCR_GDBC_mappingValue = vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_Backlight;

	Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Log0;
	Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Delay_Flame;

	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT0)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			rtd_pr_vpq_info("DBC_En=%d,LUT_DBC=%d,smooth_DBC=%d,DBC_BL=%d,OSD_BL=%d,APL=%d,DEV=%d,PS_Mode=%d,DCC_S_High_CMPS=%d,DCC_S_Index_CMPS=%d\n",
				vipCustomTBL_ShareMem->DBC.DBC_CTRL.DBC_Enable, vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DBC,
				vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_Smooth, vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_Backlight,
				OSD_BL, vipCustomTBL_ShareMem->DBC.DBC_STATUS.APL, vipCustomTBL_ShareMem->DBC.DBC_STATUS.DEV,
				vipCustomTBL_ShareMem->DBC.DBC_STATUS.OSD_PowerSave, vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DCC_S_High_CMPS,
				vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_DCC_S_Index_CMPS);
		}
	}

#endif
}

unsigned char Scaler_scalerVIP_DBC_LUT_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, unsigned char APL, unsigned char Dev, unsigned int MA_print_count)
{
	//unsigned char APL, Dev, DBC_blacklight;
	unsigned char DBC_blacklight;
	unsigned char Debug_Log;
	unsigned char Debug_Log_Delay;

	if (APL >= DBC_LUT_APL_MAX || Dev >= DBC_LUT_DEV_MAX) {
		if (MA_print_count % 100 == 0)
			rtd_pr_vpq_emerg("DBC index error, APL=%d, DEV=%d, \n", APL, Dev);
		APL = 0;
		Dev = 0;
	}

	if (vipCustomTBL_ShareMem->DBC.DBC_CTRL.DBC_Enable == 1)
		DBC_blacklight = vipCustomTBL_ShareMem->DBC.DBC_LUT_TBL.TBL[APL][Dev];
	else
		DBC_blacklight = 255;

	/* debug */
	Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Log0;
	Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Delay_Flame;

	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT1)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			rtd_pr_vpq_info("DBC_LUT, En=%d, APL=%d, Dev=%d, DBC_blacklight=%d,\n",
				vipCustomTBL_ShareMem->DBC.DBC_CTRL.DBC_Enable, APL, Dev, DBC_blacklight);
		}
	}

	return DBC_blacklight;
}

unsigned char Scaler_scalerVIP_DBC_Smooth_TV002(unsigned char DBC, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table, _clues *SmartPic_clue, unsigned int MA_print_count)
{
	static unsigned char cnt = 0;
	static unsigned char pre_DBC = 255;
	unsigned char step, delay_frame;
	unsigned char Debug_Log;
	unsigned char Debug_Log_Delay;

	step = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_PS_LEVEL;
	delay_frame = vipCustomTBL_ShareMem->DBC.DBC_SW_REG.DBC_PS_TIME;

	cnt ++;

	if (cnt >= delay_frame) {
		if ((pre_DBC - DBC) > step)
			pre_DBC = pre_DBC - step;
		else if ((DBC - pre_DBC) > step)
			pre_DBC = pre_DBC + step;
		else
			pre_DBC = DBC;
		cnt = 0;
	}

	/* debug */
	Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Log0;
	Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Delay_Flame;

	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT2)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			rtd_pr_vpq_info("DBC_Smooth, pre_DBC=%d, DBC=%d,\n",
				pre_DBC, DBC);
		}
	}

	return pre_DBC;
}

char Scaler_scalerVIP_DBC_DCC_S_High_Compensation_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, unsigned char APL, unsigned char Dev, unsigned char power_saving_mode, unsigned int MA_print_count)
{
	/*unsigned char APL, Dev;*/
	char DBC_DCC_S_High_CMPS;
	unsigned char Debug_Log;
	unsigned char Debug_Log_Delay;

	if (APL >= DBC_LUT_APL_MAX || Dev >= DBC_LUT_DEV_MAX || power_saving_mode >= D_PS_MODE_MAX) {
		if (MA_print_count % 100 == 0)
			rtd_pr_vpq_emerg("DBC index error, APL=%d, DEV=%d, power_saving_mode=%d,\n", APL, Dev, power_saving_mode);
		APL = 0;
		Dev = 0;
		power_saving_mode = 0;
	}

	if (vipCustomTBL_ShareMem->DBC.DBC_CTRL.DBC_Enable == 1)
		DBC_DCC_S_High_CMPS = vipCustomTBL_ShareMem->DBC.DBC_LUT_TBL.DCC_S_High_CMPS_TBL[power_saving_mode][APL][Dev];
	else
		DBC_DCC_S_High_CMPS = 0;

	/* debug */
	Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Log0;
	Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Delay_Flame;

	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT1)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			rtd_pr_vpq_info("DBC_DCC_CMPS, En=%d, APL=%d, Dev=%d, power_saving_mode=%d,,DBC_DCC_S_High_CMPS=%d,\n",
				vipCustomTBL_ShareMem->DBC.DBC_CTRL.DBC_Enable, APL, Dev, power_saving_mode, DBC_DCC_S_High_CMPS);
		}
	}

	return DBC_DCC_S_High_CMPS;
}

char Scaler_scalerVIP_DBC_DCC_S_Index_Compensation_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, unsigned char APL, unsigned char Dev, unsigned char power_saving_mode, unsigned int MA_print_count)
{
	/*unsigned char APL, Dev;*/
	char DBC_DCC_S_Index_CMPS;
	unsigned char Debug_Log;
	unsigned char Debug_Log_Delay;

	if (APL >= DBC_LUT_APL_MAX || Dev >= DBC_LUT_DEV_MAX || power_saving_mode >= D_PS_MODE_MAX) {
		if (MA_print_count % 100 == 0)
			rtd_pr_vpq_emerg("DBC index error, APL=%d, DEV=%d, power_saving_mode=%d,\n", APL, Dev, power_saving_mode);
		APL = 0;
		Dev = 0;
		power_saving_mode = 0;
	}

	if (vipCustomTBL_ShareMem->DBC.DBC_CTRL.DBC_Enable == 1)
		DBC_DCC_S_Index_CMPS = vipCustomTBL_ShareMem->DBC.DBC_LUT_TBL.DCC_S_Index_CMPS_TBL[power_saving_mode][APL][Dev];
	else
		DBC_DCC_S_Index_CMPS = 0;

	/* debug */
	Debug_Log = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Log0;
	Debug_Log_Delay = vipCustomTBL_ShareMem->DBC.DBC_CTRL.Debug_Delay_Flame;

	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT1)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			rtd_pr_vpq_info("DBC_DCC_Index_CMPS, En=%d, APL=%d, Dev=%d, power_saving_mode=%d,,DBC_DCC_S_Index_CMPS=%d,\n",
				vipCustomTBL_ShareMem->DBC.DBC_CTRL.DBC_Enable, APL, Dev, power_saving_mode, DBC_DCC_S_Index_CMPS);
		}
	}

	return DBC_DCC_S_Index_CMPS;
}

void Scaler_scalerVIP_ZERO_D_CTRL_TV002(SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* system_info_structure_table, _clues *SmartPic_clue, unsigned int MA_print_count)
{
#if 0 /* no use*/
	unsigned short BP_Gain;
	short BP_Offset;
	unsigned char DBC_blacklight;
	unsigned char Debug_Log;
	unsigned char Debug_Log_Delay;

	BP_Gain = vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain;
	BP_Offset = vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_CMPS;
	DBC_blacklight = vipCustomTBL_ShareMem->DBC.DBC_STATUS.DBC_Backlight;

	/* change BP gain, BP offset and DBC BL by Zero D*/
	if (vipCustomTBL_ShareMem->ZERO_D.ZERO_D_CTRL.Enable_Flag ==1) {
		;
	}

	if (vipCustomTBL_ShareMem->ZERO_D.ZERO_D_CTRL.BL_Debug_ENA == 1) {	/* use tool to control, Zero_D_Gain, Zero_D_Offset and Zero_D_Backlight*/
		BP_Gain = vipCustomTBL_ShareMem->ZERO_D.ZERO_D_STATUS.Zero_D_Gain;
		BP_Offset = vipCustomTBL_ShareMem->ZERO_D.ZERO_D_STATUS.Zero_D_Offset;
		DBC_blacklight = vipCustomTBL_ShareMem->ZERO_D.ZERO_D_STATUS.Zero_D_Backlight;
	} else {
		vipCustomTBL_ShareMem->ZERO_D.ZERO_D_STATUS.Zero_D_Gain = BP_Gain;
		vipCustomTBL_ShareMem->ZERO_D.ZERO_D_STATUS.Zero_D_Offset = BP_Offset;
		vipCustomTBL_ShareMem->ZERO_D.ZERO_D_STATUS.Zero_D_Backlight = DBC_blacklight;
	}

	Scaler_scalerVIP_Set_YUV2RGB_By_Brightness_Plus_TV002(BP_Gain, BP_Offset, system_info_structure_table, vipCustomTBL_ShareMem, MA_print_count);

	Debug_Log = vipCustomTBL_ShareMem->ZERO_D.ZERO_D_CTRL.Debug_Log0;
	Debug_Log_Delay = vipCustomTBL_ShareMem->ZERO_D.ZERO_D_CTRL.Debug_Delay_Flame;

#if 1	/* debug info*/
	if(Debug_Log_Delay==0)
		Debug_Log_Delay = 100;
	if((Debug_Log&_BIT0)!=0)
	{
		if(MA_print_count%Debug_Log_Delay==0)
		{
			rtd_pr_vpq_info("Zero-D, BP_Gain=%d, BP_Offset=%d, DBC_blacklight=%d, \n",
				BP_Gain, BP_Offset, DBC_blacklight);
		}
	}
#endif
#endif
}

unsigned short Scaler_scalerVIP_Brightness_Plus_Cal_Gain_By_TBL_TV002(SLR_VIP_TABLE* vipTBL_shareMem, _clues* SmartPic_clue, _system_setting_info* sysInfo_TBL, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, unsigned char APL, unsigned int MA_print_count)
{
	unsigned char i = 0;
//	unsigned char cur_Gain_Value = 100;
	unsigned short G1 = 0; // Scaler_GetAutoMA(BP_G1);
	unsigned short G2 = 0; // Scaler_GetAutoMA(BP_G2);
	unsigned short P1 = 0; // Scaler_GetAutoMA(BP_P1);
	unsigned short P2 = 0; // Scaler_GetAutoMA(BP_P2);
	unsigned short tmp;
	unsigned int Gain_Value = 128;
	unsigned short debug_Log = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	unsigned short debug_Log_delay = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	//unsigned short Debug_EN = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Enable;
	unsigned short shiftBit = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_Shift_Bit;
	unsigned short table_sel = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	unsigned short BP_En = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;

	if (BP_En == 0) {
		Gain_Value = (1<<shiftBit);
	} else {
		// mode 1 fix p1=0, p5=255
		vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Point[BP_SEG_0] = 0;
		vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Gain_Point.Point[BP_SEG_MAX-1] = 255;

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

		// negative error
		if(Gain_Value>(1<<(shiftBit+2)))
			Gain_Value=0;

	}

	if(debug_Log_delay==0)
		debug_Log_delay=1;

	if((debug_Log&_BIT1)!=0)
	{
		if(MA_print_count%debug_Log_delay==0)
		{
			tmp = (P1==P2)?1:(P2-P1);
			rtd_pr_vpq_info("BP gain Cal,G1=%d,G2=%d,P1=%d,P2=%d,G=%d, test=%d,%d, APL=%d, i=%d\n",
				G1, G2, P1, P2, Gain_Value, ((G2-G1) * (APL-P1) / tmp), ((G1-G2) * (APL-P1) / tmp), APL, i);
		}
	}

	return Gain_Value;

}

unsigned short Scaler_scalerVIP_Brightness_Plus_Cal_OverSat_Protect_Gain_TV002(unsigned short BP_Gain, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _clues* SmartPic_clue, unsigned int MA_print_count)
{
#if 0 /* use summation histogram to calcuate limited gain*/
	unsigned int boundary = 235; // 16-235
	unsigned char weightMax=8, weightMax_bit=3;

	if(vipCustomTBL_ShareMem==NULL)
	{
		if(MA_print_count%250==0)
			rtd_pr_vpq_emerg("Customer TBL is NULL, return\n");
		return BP_Gain;
	}

	unsigned short table_sel = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	unsigned short tol = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.OverSat_Tol;
	unsigned short th = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.OverSat_Th;
	unsigned short stepBit = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.OverSat_StepBit;
	unsigned short BP_En = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;
	unsigned short en = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.Enable;
	unsigned short shiftBit = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_Shift_Bit;

	unsigned int Gain_Value = BP_Gain;
	unsigned int Final_LimitGain = Gain_Value;
	unsigned int allowedY, allowedGain, tmp;
	unsigned short weight=0, i=0;
	unsigned short bin_ignore_th = th;
	unsigned short debug_Log = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	unsigned short debug_Log_delay = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	unsigned int *Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;

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
#else
	unsigned int boundary ;// 235; // 16-235
	unsigned short table_sel ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	unsigned short tol ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.OverSat_Tol;
	unsigned short th ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.OverSat_Th;
	//unsigned short stepBit ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.OverSat_StepBit;
	unsigned short BP_En ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;
	unsigned short en ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.Enable;
	unsigned short shiftBit ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_Shift_Bit;

	unsigned int Gain_Value ;// BP_Gain;
	unsigned int Final_LimitGain ;// Gain_Value;
	unsigned int allowedY, allowedGain, tmp;
	unsigned short i;//0;
	unsigned short bin_ignore_th ;// th;
	unsigned short debug_Log ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	unsigned short debug_Log_delay ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	unsigned int *Y_hist_ratio ;// SmartPic_clue->Y_Main_Hist_Ratio;
	unsigned short bin_index, bin_index_Y;

	if(vipCustomTBL_ShareMem==NULL)
	{
		if(MA_print_count%250==0)
			rtd_pr_vpq_emerg("Customer TBL is NULL, return\n");
		return BP_Gain;
	}

	boundary = 235; // 16-235
	table_sel = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	tol = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.OverSat_Tol;
	th = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.OverSat_Th;
	//stepBit = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.OverSat_StepBit;
	BP_En = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;
	en = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.Enable;
	shiftBit = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_Shift_Bit;

	Gain_Value = BP_Gain;
	Final_LimitGain = Gain_Value;
	i=0;
	bin_ignore_th = th;
	debug_Log = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	debug_Log_delay = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;

	if(debug_Log_delay==0)
		debug_Log_delay=1;

	if(en==0 /*|| Gain_Value<=(1<<shiftBit)*/ || BP_En==0)
		return BP_Gain;

	allowedY = (boundary*tol)>>shiftBit;

	tmp = 0;
	for (i=0;i<VIP_Y_HIST_Ratio_BIN_MAX_TV002;i++) {
		bin_index = VIP_Y_HIST_Ratio_BIN_MAX_TV002 - 1 - i;
		tmp = Y_hist_ratio[bin_index] + tmp;
		if (tmp >= bin_ignore_th)
			break;
	}

	bin_index_Y = ((bin_index + 1) * VIP_Y_HIST_Ratio_BIN_SCALE_TV002) - 1;

	if (bin_index_Y == 0)
		bin_index_Y = 1;

	allowedGain = allowedY * 1024 / bin_index_Y;

	if (allowedGain > Gain_Value)
		Final_LimitGain = Gain_Value;
	else
		Final_LimitGain = allowedGain;

	/* recorde status*/
	vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain_Limit_Allowed = allowedGain;

	if((debug_Log&_BIT2)!=0)
	{
		if(MA_print_count%debug_Log_delay==0)
			rtd_pr_vpq_info("BP protect, sum=%d,bin_index=%d,bin_index_Y=%d,allowedGain=%d,Gain_Value=%d,Final_LimitGain=%d,\n",
				tmp, bin_index, bin_index_Y, allowedGain, Gain_Value, Final_LimitGain);
	}

	return Final_LimitGain;
#endif
}

unsigned short Scaler_scalerVIP_Brightness_Plus_Smooth_Gain_Cal_TV002(unsigned short BP_Gain, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, unsigned int MA_print_count)
{
	unsigned short table_sel ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	unsigned short ini ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Init_Flag;
	unsigned short step ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Step;
	unsigned short delayF ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Delay_Flame;
	unsigned short Enable ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Enable;
	unsigned short debug_Log ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	unsigned short debug_Log_delay ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	unsigned short BP_En ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;
	static unsigned short counter = 0;
	static unsigned short pre_Gain = 0;// 0;

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

	counter++;
	return pre_Gain;
}

short Scaler_scalerVIP_Brightness_Plus_Smooth_Offset_Cal_TV002(short RGB_Offset, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, unsigned int MA_print_count)
{
	unsigned short table_sel ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	unsigned short ini ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Init_Flag;
	unsigned short step ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Step_offset;
	unsigned short delayF ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Delay_Flame_offset;
	unsigned short Enable ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Enable;
	//short pre_offset_tmp;//0;
	unsigned short debug_Log ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	unsigned short debug_Log_delay ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	unsigned short BP_En ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;
	static short pre_offset = 0;// 0;
	static unsigned short counter = 0;//0;

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

	counter++;
	return pre_offset;
}


short Scaler_scalerVIP_Brightness_Plus_Compensation_RGB_Offset_Cal_TV002(unsigned short BP_Gain, unsigned short APL, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, _system_setting_info* sysInfo_TBL, unsigned int MA_print_count)
{
	unsigned short table_sel ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	unsigned short BP_En ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;
	unsigned short en ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Enable;
	//unsigned char APL ;// Scaler_GetAutoMA(BP_APL);
	unsigned short debug_Log ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	unsigned short debug_Log_delay ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	unsigned short offset_Gain;
	unsigned short G1;//0;
	unsigned short G2;//0;
	unsigned short P1;//0;
	unsigned short P2;//0;
	short cmps;//0;
	unsigned short shiftBit ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_Shift_Bit;
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
	G1=0;
	G2=0;
	P1=0;
	P2=0;
	cmps=0;
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
	unsigned char APL;
	unsigned int Gain_Value ;// 128;
	unsigned int DC_average ;// 0;
	unsigned int Total_HistCnt ;// 0;
	unsigned short debug_Log ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	unsigned short debug_Log_delay ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	short RGB_offset_cmps;
	unsigned int *reg_HistCnt ;// SmartPic_clue->Y_Main_Hist;
	unsigned short Debug_EN ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Enable;
	unsigned short shiftBit ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_Shift_Bit;
	unsigned short table_sel ;// vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	unsigned short BP_En ;// vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;
	unsigned short i;

	if(vipTBL_shareMem==NULL || SmartPic_clue==NULL || sysInfo_TBL==NULL || vipCustomTBL_ShareMem==NULL )
	{
		if(MA_print_count%250==0)
		{
			rtd_pr_vpq_emerg("Mem Null, return, VIP TBL=%p, smartPic=%p, sysInfo=%p, VIP_cus=%p\n",
				vipTBL_shareMem, SmartPic_clue, sysInfo_TBL, vipCustomTBL_ShareMem);
		}
		return;
	}

	Gain_Value = 128;
	DC_average = 0;
	Total_HistCnt = 0;
	debug_Log = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log;
	debug_Log_delay = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Log_Delay;
	reg_HistCnt = SmartPic_clue->Y_Main_Hist;
	Debug_EN = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.Debug_Enable;
	shiftBit = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_Shift_Bit;
	table_sel = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_TBL_Select;
	BP_En = vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].BP_Enable;

	if(BP_En==1)
	{
		for(i=0;i<(VIP_Y_HIST_BIN_MAX_TV002);i++)
		{
			DC_average+= reg_HistCnt[i]*( i*VIP_Y_HIST_BIN_SCALE_TV002);
			Total_HistCnt+= reg_HistCnt[i];
		}

		if(Total_HistCnt == 0 /*|| Max_Bright_bin == 0*/)
			return;

		/*APL = ((DC_average/Total_HistCnt)<<5)/(Max_Bright_bin);*/
		APL = DC_average/Total_HistCnt;
		if(Debug_EN == 2)
			APL = vipCustomTBL_ShareMem->BP_Function.BP_STATUS.APL;	/* tool will force to write "vipCustomTBL_ShareMem->BP_Function.BP_STATUS.APL"*/
		else
			vipCustomTBL_ShareMem->BP_Function.BP_STATUS.APL = APL;

		// calculate BP gain by TBL
		Gain_Value =  Scaler_scalerVIP_Brightness_Plus_Cal_Gain_By_TBL_TV002(vipTBL_shareMem, SmartPic_clue,  sysInfo_TBL, vipCustomTBL_ShareMem, APL, MA_print_count);
		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain_TBL = Gain_Value;

		// avoid over saturate
		Gain_Value =  Scaler_scalerVIP_Brightness_Plus_Cal_OverSat_Protect_Gain_TV002(Gain_Value, vipCustomTBL_ShareMem, SmartPic_clue, MA_print_count);
		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain_Limited = Gain_Value;

		// compensation
		RGB_offset_cmps = Scaler_scalerVIP_Brightness_Plus_Compensation_RGB_Offset_Cal_TV002(Gain_Value, APL, vipCustomTBL_ShareMem, sysInfo_TBL, MA_print_count);
		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_CMPS_TBL = RGB_offset_cmps;

		// BP gain smooth
		Gain_Value = Scaler_scalerVIP_Brightness_Plus_Smooth_Gain_Cal_TV002(Gain_Value, vipCustomTBL_ShareMem, MA_print_count);
		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain_Smooth = Gain_Value;

		// offset smooth
		RGB_offset_cmps = Scaler_scalerVIP_Brightness_Plus_Smooth_Offset_Cal_TV002(RGB_offset_cmps, vipCustomTBL_ShareMem, MA_print_count);
		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_CMPS_Smooth = RGB_offset_cmps;

		// record final Gain and offset
		if(Debug_EN == 1) {	/* tool will force to write Gain and offset*/
			;
		} else {
			vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain = Gain_Value;
			vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_CMPS = RGB_offset_cmps;
		}

	}
	else
	{
		Gain_Value = (1<<shiftBit);
		RGB_offset_cmps = 0;
		//turn off all sub function
		//vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Bri_Cmps.Enable=0;
		//vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Gain_Limit.Enable=0;
		//vipCustomTBL_ShareMem->BP_Function.BP_Table[table_sel].Smooth_Change.Enable=0;
		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain_TBL = Gain_Value;
		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain_Limited = Gain_Value;
		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_CMPS_TBL = RGB_offset_cmps;
		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain_Smooth = Gain_Value;
		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_CMPS_Smooth = RGB_offset_cmps;
		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_Gain = Gain_Value;
		vipCustomTBL_ShareMem->BP_Function.BP_STATUS.BP_CMPS = RGB_offset_cmps;

	}

	/* driver write*/
	Scaler_scalerVIP_Set_YUV2RGB_By_Brightness_Plus_TV002(Gain_Value, RGB_offset_cmps, sysInfo_TBL, vipCustomTBL_ShareMem, MA_print_count);

	if((debug_Log&_BIT0)!=0)
	{
		if(MA_print_count%debug_Log_delay==0)
		{
			rtd_pr_vpq_info("BP function,BP_En=%d, Gain = %d, RGB_offset_cmps = %d\n", BP_En, Gain_Value, RGB_offset_cmps);
		}
	}

}

void Scaler_scalerVIP_Set_YUV2RGB_By_Brightness_Plus_TV002(unsigned short BP_Gain, short BP_Offset, _system_setting_info* sysInfo_TBL, SLR_VIP_TABLE_CUSTOM_TV002 *vipCustomTBL_ShareMem, unsigned int MA_print_count)
{
	short k11,k12,k13,k22,k23,k32,k33;
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	unsigned int Gain_Value = 128;
	unsigned short shiftBit = vipCustomTBL_ShareMem->BP_Function.BP_Ctrl.BP_Shift_Bit;
	unsigned short i;
	short R,G,B;
	short RGB_offset_cmps;

	Gain_Value = BP_Gain;
	RGB_offset_cmps = BP_Offset;
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("sr,%d,con0=%d,%d,%d,%d,%d,\n",y_max_idx, con_0, con_1, con_2, con_3, con_4);
	}

	return ID_TV002_Flag[ID_Radio_TV002];
}

#if 0//Old conditions
unsigned int Scaler_scalerVIP_ID_FaceArtifactNoise_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 920, s_th1 = 960;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,1,1,2,4,5,8,10,12,15,19,22,25,24,23,24,68,271,279,123,18,11,8,5,3,5,3,0,0,0,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={0,36,50,384,475,43,4,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={705,73,137,78,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("mptg,%d,%d,%d,sc=%d, PTG=%d\n",y_ratio1, h_ratio1, s_ratio1, RPC_SmartPic_clue->SceneChange, pattern_gen_flag);
	}

	return ID_TV002_Flag[ID_MovingPTG_Mono_TV002];
}

unsigned int Scaler_scalerVIP_ID_Moving_Roof_TV002( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	extern unsigned char pattern_gen_flag;

	unsigned int y_th1 = 950, h_th1 = 750, s_th1 = 950;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,0,94,143,98,84,98,132,139,113,63,24,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={0,0,2,30,111,123,124,142,151,106,97,41,31,20,12,3,0,0,0,0,0,0,0,0,};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT6) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("mrd,%d,%d,%d,sc=%d\n",y_ratio1, h_ratio1, s_ratio1, RPC_SmartPic_clue->SceneChange);
	}

	return ID_TV002_Flag[ID_MovingRoof_TV002];
}
#endif

unsigned int Scaler_scalerVIP_ID_RedWomenFace_Demo( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 920, s_th1 = 960;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,0,0,27,46,22,25,45,77,92,92,75,73,63,57,51,54,184,5,3,0,0,0,0,0,0,0,0,0,0,0};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={1,4,73,66,143,497,211,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={174,18,17,33,174,271,200,90,15,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned int goal_y2[TV006_VPQ_chrm_bin]={0,0,0,0,0,3,9,19,38,48,57,63,81,84,83,84,71,70,249,13,8,6,2,0,0,0,0,0,0,0,0,0};
	unsigned int goal_h2[COLOR_HUE_HISTOGRAM_LEVEL]={8,5,24,10,176,434,256,14,61,0,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	unsigned int goal_s2[COLOR_AutoSat_HISTOGRAM_LEVEL]={203,30,29,38,139,253,191,89,20,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned int goal_y3[TV006_VPQ_chrm_bin]={0,0,0,0,0,3,3,5,5,12,22,43,62,76,102,98,75,84,335,38,21,7,0,0,0,0,0,0,0,0,0,0};
	unsigned int goal_h3[COLOR_HUE_HISTOGRAM_LEVEL]={30,17,49,13,114,413,330,7,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6};
	unsigned int goal_s3[COLOR_AutoSat_HISTOGRAM_LEVEL]={231,13,9,31,184,266,167,67,25,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;
	unsigned int y_ratio2 = 0, h_ratio2 = 0, s_ratio2 = 0;
	unsigned int y_ratio3 = 0, h_ratio3 = 0, s_ratio3 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);
		y_ratio2 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y2[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio2 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h2[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio2 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s2[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);
		y_ratio3 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y3[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio3 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h3[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio3 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s3[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if(((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1)) ||
			((y_ratio2 > y_th1) && (h_ratio2 > h_th1) && (s_ratio2 > s_th1)) ||
			((y_ratio3 > y_th1) && (h_ratio3 > h_th1) && (s_ratio3 > s_th1)))
			ID_TV002_Flag[ID_RedWomenFace_Demo] = 1;
		else
			ID_TV002_Flag[ID_RedWomenFace_Demo] = 0;

		ROSPrintf_ID_Pattern("RedWomenFace,y_ratio1=%d,h_ratio1=%d,s_ratio1=%d\n",y_ratio1, h_ratio1, s_ratio1);
		ROSPrintf_ID_Pattern("RedWomenFace,y_ratio2=%d,h_ratio2=%d,s_ratio2=%d\n",y_ratio2, h_ratio2, s_ratio2);
		ROSPrintf_ID_Pattern("RedWomenFace,y_ratio3=%d,h_ratio3=%d,s_ratio3=%d\n",y_ratio3, h_ratio3, s_ratio3);
	} else {
		ID_TV002_Flag[ID_RedWomenFace_Demo] = 0xFF;
	}

	ROSPrintf_ID_Pattern("ID_TV002_Flag[ID_RedWomenFace_Demo]=%d\n",ID_TV002_Flag[ID_RedWomenFace_Demo]);

	return ID_TV002_Flag[ID_RedWomenFace_Demo];
}

unsigned int Scaler_scalerVIP_ID_SailingBoat_Demo( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 970, h_th1 = 970, s_th1 = 970;
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={578,272,56,25,15,13,12,12,8,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,0,0,0,3,44,219,211,202,45,14,6,9,167,55,10,2,1,1,1,0,0,0,0,0,0,0,0,0,0,0};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={24,12,43,38,64,135,69,51,45,10,15,18,18,17,27,36,105,52,36,60,33,29,8,4,11,0,1,3,6,0,5,7};

	unsigned int goal_s2[COLOR_AutoSat_HISTOGRAM_LEVEL]={398,272,112,54,30,22,24,33,33,14,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned int goal_y2[TV006_VPQ_chrm_bin]={0,0,0,0,3,20,127,221,191,152,57,23,12,14,129,9,5,5,4,4,3,3,3,2,1,0,0,0,0,0,0,0};
	unsigned int goal_h2[COLOR_HUE_HISTOGRAM_LEVEL]={18,5,25,32,56,120,109,88,55,16,16,17,19,16,25,49,115,41,65,46,12,11,3,1,7,0,1,3,5,1,4,5};

	unsigned int goal_s3[COLOR_AutoSat_HISTOGRAM_LEVEL]={334,271,97,70,43,34,36,49,44,14,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned int goal_y3[TV006_VPQ_chrm_bin]={0,0,0,2,20,47,145,168,116,145,69,28,16,17,153,20,7,6,6,5,5,5,4,3,2,1,0,0,0,0,0,0};
	unsigned int goal_h3[COLOR_HUE_HISTOGRAM_LEVEL]={2,4,12,24,53,95,124,115,72,23,26,28,25,20,32,56,122,54,46,32,10,7,1,0,1,0,0,0,0,0,0,0};

	unsigned int goal_s4[COLOR_AutoSat_HISTOGRAM_LEVEL]={126,326,176,141,95,35,29,34,24,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned int goal_y4[TV006_VPQ_chrm_bin]={0,0,0,6,62,286,220,115,75,108,35,14,10,8,6,6,6,6,5,5,5,4,3,2,1,0,0,0,0,0,0,0};
	unsigned int goal_h4[COLOR_HUE_HISTOGRAM_LEVEL]={1,5,8,16,30,66,156,121,140,84,77,59,46,34,33,26,40,24,14,4,3,1,0,0,0,0,0,0,0,0,0,0};

	unsigned int goal_s5[COLOR_AutoSat_HISTOGRAM_LEVEL]={68,251,264,214,72,41,30,26,16,6,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned int goal_y5[TV006_VPQ_chrm_bin]={0,0,0,5,106,309,177,78,64,93,36,18,13,11,10,10,10,10,9,8,8,6,4,2,1,0,0,0,0,0,0,0};
	unsigned int goal_h5[COLOR_HUE_HISTOGRAM_LEVEL]={2,9,13,22,40,53,58,83,159,116,119,81,51,39,45,36,28,18,11,2,1,0,0,0,0,0,0,0,0,0,0,0};

	unsigned int goal_s6[COLOR_AutoSat_HISTOGRAM_LEVEL]={75,244,251,199,72,52,36,30,18,9,4,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned int goal_y6[TV006_VPQ_chrm_bin]={0,0,0,3,109,297,127,78,70,115,43,22,16,14,12,12,12,12,12,10,8,6,4,3,2,1,0,0,0,0,0,0};
	unsigned int goal_h6[COLOR_HUE_HISTOGRAM_LEVEL]={1,7,14,25,52,68,65,68,142,134,108,68,52,41,33,24,42,23,17,2,1,0,0,0,0,0,0,0,0,0,0,0};
	
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;
	unsigned int y_ratio2 = 0, h_ratio2 = 0, s_ratio2 = 0;
	unsigned int y_ratio3 = 0, h_ratio3 = 0, s_ratio3 = 0;
	unsigned int y_ratio4 = 0, h_ratio4 = 0, s_ratio4 = 0;
	unsigned int y_ratio5 = 0, h_ratio5 = 0, s_ratio5 = 0;
	unsigned int y_ratio6 = 0, h_ratio6 = 0, s_ratio6 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		y_ratio2 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y2[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio2 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h2[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio2 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s2[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		y_ratio3 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y3[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio3 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h3[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio3 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s3[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);
		
		y_ratio4 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y4[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio4 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h4[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio4 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s4[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		y_ratio5 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y5[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio5 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h5[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio5 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s5[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		y_ratio6 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y6[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio6 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h6[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio6 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s6[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if(((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1)) ||
			((y_ratio2 > y_th1) && (h_ratio2 > h_th1) && (s_ratio2 > s_th1)) ||
			((y_ratio3 > y_th1) && (h_ratio3 > h_th1) && (s_ratio3 > s_th1)) ||
			((y_ratio4 > y_th1) && (h_ratio4 > h_th1) && (s_ratio4 > s_th1)) ||
			((y_ratio5 > y_th1) && (h_ratio5 > h_th1) && (s_ratio5 > s_th1)) ||
			((y_ratio6 > y_th1) && (h_ratio6 > h_th1) && (s_ratio6 > s_th1)))
			ID_TV002_Flag[ID_SailingBoat_Demo] = 1;
		else
			ID_TV002_Flag[ID_SailingBoat_Demo] = 0;

		ROSPrintf_ID_Pattern("SailingBoat,y_ratio1=%d,h_ratio1=%d,s_ratio1=%d\n",y_ratio1, h_ratio1, s_ratio1);
		ROSPrintf_ID_Pattern("SailingBoat,y_ratio2=%d,h_ratio2=%d,s_ratio2=%d\n",y_ratio2, h_ratio2, s_ratio2);
		ROSPrintf_ID_Pattern("SailingBoat,y_ratio3=%d,h_ratio3=%d,s_ratio3=%d\n",y_ratio3, h_ratio3, s_ratio3);
		ROSPrintf_ID_Pattern("SailingBoat,y_ratio4=%d,h_ratio4=%d,s_ratio4=%d\n",y_ratio1, h_ratio1, s_ratio1);
		ROSPrintf_ID_Pattern("SailingBoat,y_ratio5=%d,h_ratio5=%d,s_ratio5=%d\n",y_ratio2, h_ratio2, s_ratio2);
		ROSPrintf_ID_Pattern("SailingBoat,y_ratio6=%d,h_ratio6=%d,s_ratio6=%d\n",y_ratio3, h_ratio3, s_ratio3);

	} else {
		ID_TV002_Flag[ID_SailingBoat_Demo] = 0xFF;
	}

	ROSPrintf_ID_Pattern("ID_TV002_Flag[ID_SailingBoat_Demo]=%d\n",ID_TV002_Flag[ID_SailingBoat_Demo]);

	return ID_TV002_Flag[ID_SailingBoat_Demo];
}

unsigned int Scaler_scalerVIP_ID_StreetView_Demo( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 900, s_th1 = 900;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,2,155,357,280,144,28,7,4,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={26,113,299,465,77,3,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,4};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={1,2,11,39,142,205,172,163,146,78,23,6,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_StreetView_Demo] = 1;
		else
			ID_TV002_Flag[ID_StreetView_Demo] = 0;

		ROSPrintf_ID_Pattern("ID_StreetView_Demo,y_ratio1=%d,h_ratio1=%d,s_ratio1=%d\n",y_ratio1, h_ratio1, s_ratio1);

	} else {
		ID_TV002_Flag[ID_StreetView_Demo] = 0xFF;
	}

	ROSPrintf_ID_Pattern("ID_TV002_Flag[ID_StreetView_Demo]=%d\n",ID_TV002_Flag[ID_StreetView_Demo]);

	return ID_TV002_Flag[ID_StreetView_Demo];
}

unsigned int Scaler_scalerVIP_ID_ManInBlack_Demo( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 920, s_th1 = 920;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,1,8,42,132,128,58,35,37,31,26,27,32,28,14,12,12,12,16,19,17,19,19,29,55,29,30,56,56,6,0,0};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={1,0,1,4,36,171,249,112,229,22,18,10,6,1,2,2,3,1,11,27,48,16,3,2,3,3,1,1,0,0,0,0};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={148,340,94,96,159,65,47,24,8,4,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_ManInBlack_Demo] = 1;
		else
			ID_TV002_Flag[ID_ManInBlack_Demo] = 0;

		ROSPrintf_ID_Pattern("MainInBlack,y_ratio1=%d,h_ratio1=%d,s_ratio1=%d\n",y_ratio1, h_ratio1, s_ratio1);

	} else {
		ID_TV002_Flag[ID_ManInBlack_Demo] = 0xFF;
	}

	ROSPrintf_ID_Pattern("ID_TV002_Flag[ID_ManInBlack_Demo]=%d\n",ID_TV002_Flag[ID_ManInBlack_Demo]);

	return ID_TV002_Flag[ID_ManInBlack_Demo];
}

unsigned int Scaler_scalerVIP_ID_Bakery_Demo( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 960, s_th1 = 960;
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,1,0,1,1,3,6,13,23,33,39,43,45,39,36,37,41,54,62,68,66,65,57,52,75,30,63,24,5,0,0};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={19,59,68,115,171,392,96,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24,11,6,9,5,6,10};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={353,79,78,87,80,77,73,81,56,20,6,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1))
			ID_TV002_Flag[ID_Bakery_Demo] = 1;
		else
			ID_TV002_Flag[ID_Bakery_Demo] = 0;

		ROSPrintf_ID_Pattern("Bakery,y_ratio1=%d,h_ratio1=%d,s_ratio1=%d\n",y_ratio1, h_ratio1, s_ratio1);

	} else {
		ID_TV002_Flag[ID_Bakery_Demo] = 0xFF;
	}

	ROSPrintf_ID_Pattern("ID_TV002_Flag[ID_Bakery_Demo]=%d\n",ID_TV002_Flag[ID_Bakery_Demo]);

	return ID_TV002_Flag[ID_Bakery_Demo];
}

unsigned int Scaler_scalerVIP_ID_Face_Demo( _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, unsigned int MA_print_count)
{
	unsigned int y_th1 = 960, h_th1 = 960, s_th1 = 960;
	//261
	unsigned int goal_y1[TV006_VPQ_chrm_bin]={0,0,0,0,0,1,1,2,4,7,9,11,15,18,17,17,17,21,655,93,11,9,9,8,10,9,9,15,14,3,0,0};
	unsigned int goal_h1[COLOR_HUE_HISTOGRAM_LEVEL]={90,23,78,40,92,258,149,33,137,0,3,12,15,0,1,0,3,0,0,0,0,1,1,1,5,3,7,6,7,3,7,11};
	unsigned int goal_s1[COLOR_AutoSat_HISTOGRAM_LEVEL]={816,13,8,9,45,56,34,12,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//272
	unsigned int goal_y2[TV006_VPQ_chrm_bin]={0,0,1,2,7,10,11,10,10,9,8,11,14,18,70,115,196,169,31,20,13,9,7,6,7,11,41,63,91,25,0,0};
	unsigned int goal_h2[COLOR_HUE_HISTOGRAM_LEVEL]={16,2,8,132,82,221,153,45,102,9,11,8,4,0,0,0,10,0,4,13,8,40,49,25,17,3,4,5,6,0,6,2};
	unsigned int goal_s2[COLOR_AutoSat_HISTOGRAM_LEVEL]={748,23,18,26,54,85,32,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//273
	unsigned int goal_y3[TV006_VPQ_chrm_bin]={0,0,1,7,18,16,10,7,7,8,11,15,18,34,71,103,159,185,47,29,27,15,12,14,26,30,33,26,54,1,0,0};
	unsigned int goal_h3[COLOR_HUE_HISTOGRAM_LEVEL]={14,0,4,52,41,305,146,89,85,12,7,5,71,0,1,0,1,0,0,1,2,15,34,34,41,2,1,8,10,0,6,0};
	unsigned int goal_s3[COLOR_AutoSat_HISTOGRAM_LEVEL]={648,50,27,30,102,77,39,22,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//274
	unsigned int goal_y4[TV006_VPQ_chrm_bin]={0,0,1,9,19,18,12,11,13,17,18,16,18,42,70,111,169,144,30,32,27,18,12,12,12,20,34,40,59,0,0,0};
	unsigned int goal_h4[COLOR_HUE_HISTOGRAM_LEVEL]={15,1,3,134,51,309,170,47,73,7,3,6,12,0,0,0,1,0,0,0,1,8,25,35,56,4,2,3,6,0,10,0};
	unsigned int goal_s4[COLOR_AutoSat_HISTOGRAM_LEVEL]={641,36,31,31,95,90,46,24,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//275
	unsigned int goal_y5[TV006_VPQ_chrm_bin]={0,0,1,11,18,16,11,7,4,4,5,8,15,47,60,88,215,194,42,9,7,8,11,13,15,18,41,87,32,1,0,0};
	unsigned int goal_h5[COLOR_HUE_HISTOGRAM_LEVEL]={39,1,7,92,64,200,49,29,120,20,10,3,21,0,0,0,2,0,1,2,4,42,83,40,21,1,1,18,83,0,30,1};
	unsigned int goal_s5[COLOR_AutoSat_HISTOGRAM_LEVEL]={780,34,20,15,37,81,24,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	unsigned char which_source = RPC_system_infoTBL->VIP_source;
	unsigned int y_ratio1 = 0, h_ratio1 = 0, s_ratio1 = 0;
	unsigned int y_ratio2 = 0, h_ratio2 = 0, s_ratio2 = 0;
	unsigned int y_ratio3 = 0, h_ratio3 = 0, s_ratio3 = 0;
	unsigned int y_ratio4 = 0, h_ratio4 = 0, s_ratio4 = 0;
	unsigned int y_ratio5 = 0, h_ratio5 = 0, s_ratio5 = 0;

	unsigned int *Y_hist_ratio, *hue_hist_ratio, *sat_hist_ratio;

	if((which_source == VIP_QUALITY_HDMI_1080I) || (which_source == VIP_QUALITY_HDMI_1080P)) {
		hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
		Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
		sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;

		y_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y1[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h1[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio1 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s1[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		y_ratio2 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y2[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio2 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h2[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio2 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s2[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		y_ratio3 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y3[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio3 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h3[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio3 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s3[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		y_ratio4 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y4[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio4 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h4[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio4 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s4[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		y_ratio5 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_y5[0], &Y_hist_ratio[0], TV006_VPQ_chrm_bin);
		h_ratio5 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_h5[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);
		s_ratio5 = Scaler_scalerVIP_ratio_inner_product_base_TV002(&goal_s5[0], &sat_hist_ratio[0], COLOR_AutoSat_HISTOGRAM_LEVEL);

		if(((y_ratio1 > y_th1) && (h_ratio1 > h_th1) && (s_ratio1 > s_th1)) ||
			((y_ratio2 > y_th1) && (h_ratio2 > h_th1) && (s_ratio2 > s_th1)) ||
			((y_ratio3 > y_th1) && (h_ratio3 > h_th1) && (s_ratio3 > s_th1)) ||
			((y_ratio4 > y_th1) && (h_ratio4 > h_th1) && (s_ratio4 > s_th1)) ||
			((y_ratio5 > y_th1) && (h_ratio5 > h_th1) && (s_ratio5 > s_th1)))
			ID_TV002_Flag[ID_Face_Demo] = 1;
		else
			ID_TV002_Flag[ID_Face_Demo] = 0;

		ROSPrintf_ID_Pattern("Face261,y_ratio1=%d,h_ratio1=%d,s_ratio1=%d\n",y_ratio1, h_ratio1, s_ratio1);
		ROSPrintf_ID_Pattern("Face272,y_ratio2=%d,h_ratio2=%d,s_ratio2=%d\n",y_ratio2, h_ratio2, s_ratio2);
		ROSPrintf_ID_Pattern("Face273,y_ratio3=%d,h_ratio3=%d,s_ratio3=%d\n",y_ratio3, h_ratio3, s_ratio3);
		ROSPrintf_ID_Pattern("Face274,y_ratio4=%d,h_ratio4=%d,s_ratio4=%d\n",y_ratio4, h_ratio4, s_ratio4);
		ROSPrintf_ID_Pattern("Face275,y_ratio5=%d,h_ratio5=%d,s_ratio5=%d\n",y_ratio5, h_ratio5, s_ratio5);

	} else {
		ID_TV002_Flag[ID_Face_Demo] = 0xFF;
	}

	ROSPrintf_ID_Pattern("ID_TV002_Flag[ID_Face_Demo]=%d\n",ID_TV002_Flag[ID_Face_Demo]);

	return ID_TV002_Flag[ID_Face_Demo];
}

void Scaler_scalerVIP_set_sharpness_level_Directly_TV002(DRV_Sharpness_Level *ptr)
{
	drvif_color_set_Sharpness_level(ptr);
}

DRV_Sharpness_Level Current_Shp_Level_TBL_TV002 = {{0}};
unsigned char Scaler_scalerVIP_Access_sharpness_level_TBL_TV002(DRV_Sharpness_Level *ptr, unsigned char isSet_Flag)
{
	static unsigned char shp_TBL_src = 0xff;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(VIP_RPC_system_info_structure_table == NULL ){
		rtd_pr_vpq_emerg("VIP_RPC_system_info_structure_table NULL=%p\n", VIP_RPC_system_info_structure_table);
		return -1;
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
	unsigned int *Y_hist_ratio, *hue_hist_ratio;
	unsigned int print_flag;
	unsigned int print_delay;

	hue_hist_ratio = SmartPic_clue->Hue_Main_His_Ratio;
	Y_hist_ratio = SmartPic_clue->Y_Main_Hist_Ratio;
	//sat_hist_ratio = SmartPic_clue->Sat_Main_His_Ratio;
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
		Scaler_scalerVIP_ID_RedWomenFace_Demo(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_SailingBoat_Demo(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_StreetView_Demo(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_ManInBlack_Demo(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_Bakery_Demo(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
		Scaler_scalerVIP_ID_Face_Demo(RPC_system_infoTBL, system_infoTBL, RPC_SmartPic_clue, SmartPic_clue, MA_print_count);
	}
	/******************************************************************************************************/
	/******************************************************************************************************/

	/* result check*/
	print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;
	if((print_flag&_BIT5) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0) {
			rtd_pr_vpq_info("ID, src=%d\n", RPC_system_infoTBL->VIP_source);
			for(i=0;i<ID_ITEM_TV002_MAX;i++)
				printk("%d,", ID_TV002_Flag[i]);
			printk("\n");
		}
	}

	/* HIST check*/
	if((print_flag&_BIT10) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0) {
			rtd_pr_vpq_info("Y_HIST_Ratio={");
			for(i=0;i<TV006_VPQ_chrm_bin;i++)
				printk("%d,", SmartPic_clue->Y_Main_Hist_Ratio[i]);
			printk("}\n");
		}
	}

	if((print_flag&_BIT11) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0) {
			rtd_pr_vpq_info("HUE_HIST_Ratio={");
			for(i=0;i<COLOR_HUE_HISTOGRAM_LEVEL;i++)
				printk("%d,", SmartPic_clue->Hue_Main_His_Ratio[i]);
			printk("}\n");
		}
	}

	if((print_flag&_BIT12) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0) {
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
	} 
#if 0 //Old conditons
	else if (ID_TV002_Flag[ID_FaceArtifactNoise_TV002] == 1) {
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

	} 
#endif
	else if (ID_TV002_Flag[ID_RedWomenFace_Demo] == 1) {
			PQA_Input_Level[I_002_RTNR_0] = 0;
			PQA_Input_rate[I_002_RTNR_0] = 0;

			PQA_Input_Level[I_002_RTNR_1] = 0;
			PQA_Input_rate[I_002_RTNR_1] = 0;

			PQA_RunFlag[I_ID] = 1;
	}
	else if (ID_TV002_Flag[ID_SailingBoat_Demo] == 1) {
			PQA_Input_Level[I_002_RTNR_0] = 0;
			PQA_Input_rate[I_002_RTNR_0] = 0;

			PQA_Input_Level[I_002_RTNR_1] = 0;
			PQA_Input_rate[I_002_RTNR_1] = 0;

			PQA_RunFlag[I_ID] = 1;
	}
	else if (ID_TV002_Flag[ID_StreetView_Demo] == 1) {
			PQA_Input_Level[I_002_RTNR_0] = 1;
			PQA_Input_rate[I_002_RTNR_0] = 0;

			PQA_Input_Level[I_002_RTNR_1] = 1;
			PQA_Input_rate[I_002_RTNR_1] = 0;

			PQA_RunFlag[I_ID] = 1;
	}
	else if (ID_TV002_Flag[ID_ManInBlack_Demo] == 1) {
			PQA_Input_Level[I_002_RTNR_0] = 2;
			PQA_Input_rate[I_002_RTNR_0] = 0;

			PQA_Input_Level[I_002_RTNR_1] = 2;
			PQA_Input_rate[I_002_RTNR_1] = 0;
	
			PQA_RunFlag[I_ID] = 1;
	}
	else if (ID_TV002_Flag[ID_Bakery_Demo] == 1) {
			PQA_Input_Level[I_002_RTNR_0] = 9;
			PQA_Input_rate[I_002_RTNR_0] = 0;

			PQA_Input_Level[I_002_RTNR_1] = 9;
			PQA_Input_rate[I_002_RTNR_1] = 0;

			PQA_RunFlag[I_ID] = 1;
	}
	else if (ID_TV002_Flag[ID_Face_Demo] == 1) {
			PQA_Input_Level[I_002_RTNR_0] = 3;
			PQA_Input_rate[I_002_RTNR_0] = 0;

			PQA_Input_Level[I_002_RTNR_1] = 3;
			PQA_Input_rate[I_002_RTNR_1] = 0;

			PQA_RunFlag[I_ID] = 1;
	}
	else {
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
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

	if((ID_TV002_Flag[ID_Radio_TV002] != 0xFF)/* || (ID_TV002_Flag[ID_RockMountain_TV002] != 0xFF)*//*Old conditons*/) {
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
			} 
#if 0 //Old conditons
			else if (ID_TV002_Flag[ID_RockMountain_TV002] == 1) {
				peaking_driver_set = 1;

				shp_Lv_TBL_tmp.Edge_Level.G_Pos_Gain = 80;
				shp_Lv_TBL_tmp.Edge_Level.HV_POS_Gain = 200;
				shp_Lv_TBL_tmp.Vertical_Edge_Level.G_Pos_Gain = 60;
				shp_Lv_TBL_tmp.Vertical_Edge_Level.HV_POS_Gain = 200;

				pre_ID_status = ID_RockMountain_TV002;
			}
#endif
			else {
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
	if((print_flag&_BIT7) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
			rtd_pr_vpq_info("app,st=%d,src0=%d,src1=%d\n",pre_ID_status, shp_Lv_TBL_src, which_source);
	}
#endif
}
void Scaler_scalerVIP_ID_Apply_DI_TV002(SLR_VIP_TABLE* ShareMem_VIP_TBL, _RPC_system_setting_info* RPC_system_infoTBL, _system_setting_info* system_infoTBL, _clues* SmartPic_clue, unsigned int MA_print_count)
{
#if 0 //Old conditions
	extern DRV_di_smd_table_t gSMDtable[7];

	unsigned char smd_reliablerange_sad_th, h_pan_mv_value, n_mcnr_v_pan_mv, ma_controlmode;
	unsigned char SMD_driver_set, MA_Crtl_Mode_driver_set;
	short tmp;
	unsigned char vip_src = RPC_system_infoTBL->VIP_source;
	unsigned char  SMD_TBL_Sel = ShareMem_VIP_TBL->VIP_QUALITY_Coef[vip_src][VIP_QUALITY_FUNCTION_DI_SMD_Level];
	static unsigned char pre_ID_status_SMD = 0xFF, pre_ID_status_MA_mode = 0xFF;
	DRV_di_smd_table_t SMDtable = {{0}};
	unsigned int print_flag = system_infoTBL->ISR_Printf_Flag.TV002_Flag;
	unsigned int print_delay = system_infoTBL->ISR_Printf_Flag.Delay_Time;

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
	if((print_flag&_BIT8) != 0)
	{
		print_delay = (print_delay==0)?(100):(print_delay);
		if((MA_print_count%print_delay)==0)
			printk("app reg,st=%d,%d\n",pre_ID_status_SMD, pre_ID_status_MA_mode);
	}
#endif
}


//====================================================================================
//****************************************************PQA****************************************************
//====================================================================================

#define Lv_Sequence_MAX 10
static unsigned char RTNR_Info_Mad_lv;
static unsigned char RTNR_Info_Motion_lv;
static unsigned char RTNR_Info_Film_lv;
//static char SNR_LV_sequence[Lv_Sequence_MAX];
//static char SHP_LV_sequence[Lv_Sequence_MAX];

unsigned int Scaler_scalerVIP_compute_I_002_level_Index_Ave(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_PQA_I_002 32
	#define windowSizeBit_PQA_I_002 5

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

unsigned int scalerVIP_compute_I_002_hmcnrlevel_Index_Ave(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_mcnr 8
	#define windowSizeBit_I_mcnr 3

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_mcnr]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_mcnr;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_mcnr;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_mcnr;

		idx++;
		if(idx==windowSize_I_mcnr)
			idx=0;
	}
	return ret;
}

unsigned int scalerVIP_Cal_Idx_Ave_I_002_chroma(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_002 32
	#define windowSizeBit_I_002 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_002]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_002;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_002;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_002;

		idx++;
		if(idx==windowSize_I_002)
			idx=0;
	}
	return ret;
}

unsigned int scalerVIP_compute_I_002_RTNR1_chromanrlevel_Index_Ave(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_mcnr 8
	#define windowSizeBit_I_mcnr 3

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_mcnr]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_mcnr;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_mcnr;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_mcnr;

		idx++;
		if(idx==windowSize_I_mcnr)
			idx=0;
	}
	return ret;
}

unsigned int scalerVIP_Cal_Idx_Ave_I_002_mcnr(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_002 32
	#define windowSizeBit_I_002 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_002]={0};

	unsigned char i=0;
	unsigned int sum = 0;
	static unsigned int ret = 0;
	static unsigned int count_old = 0;
	extern unsigned int MA_print_count;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_002;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else if (count_old != MA_print_count)
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_002;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_002;

		idx++;
		if(idx==windowSize_I_002)
			idx=0;
	}

	count_old = MA_print_count;
	return ret;
}

unsigned int scalerVIP_compute_I_002_SHP0_hmcnrlevel_Index_Ave(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_mcnr 8
	#define windowSizeBit_I_mcnr 3

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_mcnr]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_mcnr;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_mcnr;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_mcnr;

		idx++;
		if(idx==windowSize_I_mcnr)
			idx=0;
	}
	return ret;
}

unsigned int scalerVIP_Cal_Idx_Ave_I_002_mpeg(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_002 32
	#define windowSizeBit_I_002 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_002]={0};

	unsigned char i=0;
	unsigned int sum = 0;
	static unsigned int ret = 0;
	static unsigned int count_old = 0;
	extern unsigned int MA_print_count;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_002;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else if (count_old != MA_print_count)
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_002;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_002;

		idx++;
		if(idx==windowSize_I_002)
			idx=0;
	}

	count_old = MA_print_count;
	return ret;
}

unsigned int scalerVIP_Cal_Idx_Ave_I_002_highfreq_motion(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_002 32
	#define windowSizeBit_I_002 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_002]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_002;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_002;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_002;

		idx++;
		if(idx==windowSize_I_002)
			idx=0;
	}
	return ret;
}

unsigned int scalerVIP_Cal_Idx_Ave_I_002_highfreq(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_002 32
	#define windowSizeBit_I_002 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_002]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_002;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_002;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_002;

		idx++;
		if(idx==windowSize_I_002)
			idx=0;
	}
	return ret;
}

void Scaler_scalerVIP_compute_level_I_002_RFCVBS_RTNR(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	unsigned int current_Signal_Status = 0;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_VD_noise_status = vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_VD_noise_status];
	unsigned short *PQA_Input_Level = SmartPic_clue->PQA_ISR_Cal_Info.Input_Level;
	unsigned short *PQA_Input_rate = SmartPic_clue->PQA_ISR_Cal_Info.Input_rate;

	current_Signal_Status = (SmartPic_clue->VD_Signal_Status_value_avg);  //yuan1024 for rtnr n files avg

	if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L00])
		PQA_Input_Level[I_002_RFCVBS_RTNR] = 0;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L01])
		PQA_Input_Level[I_002_RFCVBS_RTNR] = 1;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L02])
		PQA_Input_Level[I_002_RFCVBS_RTNR] = 2;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L03])
		PQA_Input_Level[I_002_RFCVBS_RTNR] = 3;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L04])
		PQA_Input_Level[I_002_RFCVBS_RTNR] = 4;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L05])
		PQA_Input_Level[I_002_RFCVBS_RTNR] = 5;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L06])
		PQA_Input_Level[I_002_RFCVBS_RTNR] = 6;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L07])
		PQA_Input_Level[I_002_RFCVBS_RTNR] = 7;
	else if(current_Signal_Status < pPQA_VD_noise_status[PQA_I_L08])
		PQA_Input_Level[I_002_RFCVBS_RTNR] = 8;
	else
		PQA_Input_Level[I_002_RFCVBS_RTNR] = 9;

/*
	if(PQA_Input_Level[I_002_RFCVBS_RTNR] >= 1) {
		if(pPQA_VD_noise_status[PQA_Input_Level[I_002_RFCVBS_RTNR]] > pPQA_VD_noise_status[PQA_Input_Level[I_002_RFCVBS_RTNR]-1])
			PQA_Input_rate[I_002_RFCVBS_RTNR] = 255*(current_Signal_Status-pPQA_VD_noise_status[PQA_Input_Level[I_002_RFCVBS_RTNR]-1])/(pPQA_VD_noise_status[PQA_Input_Level[I_002_RFCVBS_RTNR]] - pPQA_VD_noise_status[PQA_Input_Level[I_002_RFCVBS_RTNR]-1]);
		else
		
			PQA_Input_rate[I_002_RFCVBS_RTNR] = 0;
	}
	else
*/
	PQA_Input_rate[I_002_RFCVBS_RTNR] = 0;
	ROSPrintf_INFO_PQA7("vd_noise=%d , Lv=%d\n",current_Signal_Status, PQA_Input_Level[I_002_RFCVBS_RTNR]);

}

int Scaler_scalerVIP_compute_Still_Idx_FMV_HMC_I_002(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	//still meter
	 /*unsigned int printf_Flag=0, print_delay=200 ;*/
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
	//printf_Flag = system_info_structure_table->ISR_Printf_Flag.TV002_Flag;
	//print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//if(((printf_Flag&_BIT4)!=0))
	{
		//if((MA_print_count%print_delay==0))
			ROSPrintf_INFO_PQA1("FMV=%d,HMC=%d,still=%d,GMV=%d\n", still_idx_FMV, still_idx_HMC_Bin, still_idx,GMV);
	}

	if (GMV == 3)
		ret_still_idx = still_idx_FMV;
	else
		ret_still_idx = 0;

	return ret_still_idx;
}

void Scaler_scalerVIP_compute_level_I_002_RTNR0_tv002(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	char LV_sequence[Lv_Sequence_MAX];
	unsigned char seq_idx = 0;
	unsigned int RTNR_MAD_count_Y2_avg;
	unsigned int RTNR_MAD_count_Y_avg;
	static char isFilm = 0;
	int level_Idx=0,mv_index=0;
	signed char Lv=0,mad_lv=0,mv_lv=0,film_lv=0;//,ori_lv=0,apl_lv=0;

	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_InputMAD_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_RTNR_MAD][0]);
	unsigned char meanHistY = SmartPic_clue->Hist_Y_Mean_Value;
	//unsigned short Lv_intp=0;//, sr_d;
	//unsigned int tmp;
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned int *pPQA_HMC_MV_w_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_HMC_MV_Hist_Weighting][0]);

	unsigned int *pPQA_Input_HistMeanY_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean][0]);
	unsigned int *pPQA_InputFilm_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_CorrectionBit_Histogram_mean][0]);

	unsigned char diff_Th1=1,diff_Th2=2,score_th=10;
	static char pre_meanHistY=0,prepre_meanHistY=0;
	static char diff_score=0;
	char pn_flag=0;
	static char pn_flag_pre=0;
	char Level=0;
	unsigned char *FilmInfo = Scaler_MEMC_GetCadence();

	if( FilmInfo == NULL )
	{
		if( MA_print_count%120 == 0)
			rtd_pr_vpq_emerg("[%s] Film information ptr from MEMC error\n", __func__);
		return;
	}

	RTNR_MAD_count_Y_avg = SmartPic_clue->RTNR_MAD_count_Y_avg_ratio;
	RTNR_MAD_count_Y2_avg = SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio;

	// Debounce
	if( FilmInfo[_FILM_MEMC_OUT_ALL] == 0 && isFilm > 0 )
		isFilm--;
	else if( FilmInfo[_FILM_MEMC_OUT_ALL] != 0 )
		isFilm = 40;

	level_Idx = Scaler_scalerVIP_compute_I_002_level_Index_Ave(RTNR_MAD_count_Y2_avg, 0);
	mv_index = scalerVIP_compute_I_002_hmcnrlevel_Index_Ave(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(),0);

	if(level_Idx < pPQA_InputMAD_TBL[0])
	{
		Lv = 0;
		mad_lv =0;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[1])
	{
		Lv = 1;
		mad_lv =1;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[2])
	{
		Lv = 2;
		mad_lv =2;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[3])
	{
		Lv = 3;
		mad_lv =3;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[4])
	{
		Lv = 4;
		mad_lv =4;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[5])
	{
		Lv = 5;
		mad_lv =5;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[6])
	{
		Lv = 6;
		mad_lv =6;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[7])
	{
		Lv = 7;
		mad_lv =7;
	}
	else if(level_Idx < pPQA_InputMAD_TBL[8])
	{
		Lv = 8;
		mad_lv =8;
	}
	else
	{
		Lv = 9;
		mad_lv =9;

	}

	ROSPrintf_INFO_PQA1("\n I_002_RTNR_0, 1st Lv =%d ,",Lv);

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

	if(mv_index <pPQA_HMC_MV_w_TBL[0])
	{
		Lv = (Lv>9)?(Lv-9):0;
		mv_lv=9;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[1])
	{
		Lv = (Lv>8)?(Lv-8):0;
		mv_lv=8;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[2])
	{
		Lv = (Lv>7)?(Lv-7):0;
		mv_lv=7;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[3])
	{
		Lv = (Lv>6)?(Lv-6):0;
		mv_lv=6;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[4])
	{
		Lv = (Lv>5)?(Lv-5):0;
		mv_lv=5;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[5])
	{
		Lv = (Lv>4)?(Lv-4):0;
		mv_lv=4;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[6])
	{
		Lv = (Lv>3)?(Lv-3):0;
		mv_lv=3;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[7])
	{
		Lv = (Lv>2)?(Lv-2):0;
		mv_lv=2;

	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[8])
	{
		Lv = (Lv>1)?(Lv-1):0;
		mv_lv=1;

	}
	else
	{
		mv_lv=0;
	}

	ROSPrintf_INFO_PQA1(" 2nd Lv =%d ,",Lv);


	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

	if(meanHistY<pPQA_Input_HistMeanY_TBL[0])
	{
		Lv = (Lv>9)?(Lv-9):0;
		//apl_lv=9;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[1])
	{
		Lv = (Lv>8)?(Lv-8):0;
		//apl_lv=8;

	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[2])
	{
		Lv = (Lv>7)?(Lv-7):0;
		//apl_lv=7;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[3])
	{
		Lv = (Lv>6)?(Lv-6):0;
		//apl_lv=6;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[4])
	{
		Lv = (Lv>5)?(Lv-5):0;
		//apl_lv=5;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[5])
	{
		Lv = (Lv>4)?(Lv-4):0;
		//apl_lv=4;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[6])
	{
		Lv = (Lv>3)?(Lv-3):0;
		//apl_lv=3;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[7])
	{
		Lv = (Lv>2)?(Lv-2):0;
		//apl_lv=2;
	}
	else if(meanHistY<pPQA_Input_HistMeanY_TBL[8])
	{
		Lv = (Lv>1)?(Lv-1):0;
		//apl_lv=1;
	}
	else
	{
		//apl_lv=0;

	}

	ROSPrintf_INFO_PQA1(" 3rd Lv =%d ,",Lv);

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

	//===================for film===============================

	 //0 VIDEO,1:CAD_22 ,2: _CAD_32,3:_CAD_32322,4 others ,beside 1,2,3,

	if(( FilmInfo[_FILM_MEMC_OUT_ALL] != 0)&&Lv>5) //if is film mode
	{
		if(pPQA_InputFilm_TBL[0]!=0) //all film mdoe
		{
			Lv = Lv-pPQA_InputFilm_TBL[0];
			film_lv = pPQA_InputFilm_TBL[0];

			if(Lv<5)
			{
				Lv =5;
			}

		}
		else //by different film to decide reduce level
		{

			if(FilmInfo[_FILM_MEMC_OUT_ALL]==1)      //only 22 film
			{
				Lv = Lv-pPQA_InputFilm_TBL[1];
				film_lv = pPQA_InputFilm_TBL[1];
			}
			else if(FilmInfo[_FILM_MEMC_OUT_ALL]==2) //only 32 film
			{
				Lv = Lv-pPQA_InputFilm_TBL[2];
				film_lv = pPQA_InputFilm_TBL[2];

			}
			else if(FilmInfo[_FILM_MEMC_OUT_ALL]==3) //only 32322 film
			{
				Lv = Lv-pPQA_InputFilm_TBL[3];
				film_lv = pPQA_InputFilm_TBL[3];

			}
			else
			{
				Lv = Lv-pPQA_InputFilm_TBL[4];
				film_lv = pPQA_InputFilm_TBL[4];
			}

			if(Lv<5)
			{
				Lv =5;
			}


		}
	}

	ROSPrintf_INFO_PQA1(" 4td Lv =%d ,===== mad_lv=%d, mv_lv=%d, film_lv=%d \n",Lv, mad_lv, mv_lv, film_lv);


	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

	if(Lv<=0)
	{
		Lv=0;
	}

	Level = Lv;
	if(meanHistY>pre_meanHistY)
			pn_flag=1;
		else if(meanHistY<pre_meanHistY)
			pn_flag=-1;
		else
			pn_flag=0;

	if(RTNR_MAD_count_Y_avg<20)
	{
		if(pn_flag*pn_flag_pre==-1)
		{
			diff_score--;
			if(diff_score<0)
				diff_score=0;
		}
		else if (ABS(pre_meanHistY,meanHistY)>=diff_Th2 || (ABS(pre_meanHistY,meanHistY)>=diff_Th1&&diff_score >5))
		{
			diff_score += 2;
			if(diff_score > 25)
				diff_score=25;
		}
		else if (ABS(pre_meanHistY,meanHistY)>=diff_Th1 || ABS(prepre_meanHistY,meanHistY)>=diff_Th1)
		{
			diff_score ++;
			if(diff_score > 25)
				diff_score=25;
		}
		else if (ABS(prepre_meanHistY,meanHistY)==0)
		{
			diff_score --;
			if(diff_score<0)
				diff_score=0;
		}

		if(diff_score>score_th)
		{
			Level= Level-(diff_score-score_th);
			if(Level<0)
				Level=0;
		}
		Lv =Level;

	}
	else
	{
		diff_score--;
		if(diff_score<0)
			diff_score=0;

	}

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

/////////////////// mcnr pan , use the same nr level start////////////////////////////////////////

	if(((isFilm!=0)&&Lv>5)&&(scalerVIP_DI_MiddleWare_MCNR_Get_Pan_Flag()==1))
	{
		Lv = Lv-2;
		if(Lv<5)
		{
			Lv =5;
		}
	}

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

	if(ID_TV002_Flag[ID_Radio_TV002]==1)
		Lv=0;

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

	// record RTNR info for SNR use
	RTNR_Info_Mad_lv= mad_lv;
	RTNR_Info_Motion_lv = mv_lv;
	RTNR_Info_Film_lv =film_lv;

	if(level_Idx>pPQA_InputMAD_TBL[PQA_I_L09])
		level_Idx = pPQA_InputMAD_TBL[PQA_I_L09];
	//tmp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1])==0)?1:(pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1]);

	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;
	//SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_1] = motion;

	PQA_Input_Level[I_002_RTNR_0] = Lv;
	PQA_Input_rate[I_002_RTNR_0] = 0;
	scalerVIP_Set_Noise_Level(Lv); // for mcnr setting in scalerDI.c

	prepre_meanHistY = pre_meanHistY;

	pre_meanHistY = meanHistY;
	pn_flag_pre=pn_flag;

	ROSPrintf_INFO_PQA1("I_002_RTNR_0, level_Idx=%d, RTNR_MAD_count_Y2_avg=%d, mv_index=%d, GMV_Ratio()=%d, FilmInfo[_ALL]=%d, meanHistY=%d, \n",
		level_Idx, RTNR_MAD_count_Y2_avg, mv_index, scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(), FilmInfo[_FILM_MEMC_OUT_ALL], meanHistY);
	ROSPrintf_INFO_PQA1("pre_meanHistY=%d, pre_meanHistY=%d, RTNR_MAD_count_Y_avg=%d, diff_score=%d, isFilm=%d, Pan_Flag()=%d,\n",
		pre_meanHistY, pre_meanHistY, RTNR_MAD_count_Y_avg, diff_score, isFilm, scalerVIP_DI_MiddleWare_MCNR_Get_Pan_Flag());
	ROSPrintf_INFO_PQA1("LV_seq=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d, \n", LV_sequence[0], LV_sequence[1], LV_sequence[2],
		LV_sequence[3], LV_sequence[4], LV_sequence[5], LV_sequence[6], LV_sequence[7],LV_sequence[8], LV_sequence[9]);

}

void Scaler_scalerVIP_compute_level_I_002_RTNR1_tv002(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	// chroma NR information
	char LV_sequence[Lv_Sequence_MAX];
	unsigned char seq_idx = 0;
	int level_Idx=0,mv_index=0;
	char Lv=0,ori_lv=0;//mv_lv=0,
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_InputMAD_Chroma_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_FMV_Hist_Weighting][0]); //for mad chroma
	//unsigned char meanHistY = SmartPic_clue->Hist_Y_Mean_Value;
	//unsigned short Lv_intp=0;//, sr_d;
	//unsigned int tmp;
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned int *pPQA_HMC_MV_C_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_FMV_Hist_stillIdx_th][0]);//for hmv
	static unsigned char mcnr_pan_nr_level = 255;
	unsigned int uv_max_value2=0;//uv_max_value1=0,,uv_max_value3=0;
	//unsigned int RTNR_MAD_count_U_avg=0,RTNR_MAD_count_V_avg=0;
	unsigned int RTNR_MAD_count_U2_avg=0,RTNR_MAD_count_V2_avg=0;
	//unsigned int RTNR_MAD_count_U3_avg=0,RTNR_MAD_count_V3_avg=0;

	//RTNR_MAD_count_U_avg = SmartPic_clue->RTNR_MAD_count_U_avg_ratio;
	RTNR_MAD_count_U2_avg = SmartPic_clue->RTNR_MAD_count_U2_avg_ratio;
	//RTNR_MAD_count_U3_avg = SmartPic_clue->RTNR_MAD_count_U3_avg_ratio;
	//RTNR_MAD_count_V_avg = SmartPic_clue->RTNR_MAD_count_V_avg_ratio;
	RTNR_MAD_count_V2_avg = SmartPic_clue->RTNR_MAD_count_V2_avg_ratio;
	//RTNR_MAD_count_V3_avg = SmartPic_clue->RTNR_MAD_count_V3_avg_ratio;

	//uv_max_value1 = max(RTNR_MAD_count_U_avg,RTNR_MAD_count_V_avg);
	uv_max_value2 = max(RTNR_MAD_count_U2_avg,RTNR_MAD_count_V2_avg);
	//uv_max_value3 = max(RTNR_MAD_count_U3_avg,RTNR_MAD_count_V3_avg);

	level_Idx = scalerVIP_Cal_Idx_Ave_I_002_chroma(uv_max_value2, 0);
	mv_index =scalerVIP_compute_I_002_RTNR1_chromanrlevel_Index_Ave(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(), 0);

	if(level_Idx < pPQA_InputMAD_Chroma_TBL[0])
		Lv = 0;
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[1])
		Lv = 1;
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[2])
		Lv = 2;
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[3])
		Lv = 3;
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[4])
		Lv = 4;
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[5])
		Lv = 5;
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[6])
		Lv = 6;
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[7])
		Lv = 7;
	else if(level_Idx < pPQA_InputMAD_Chroma_TBL[8])
		Lv = 8;
	else
		Lv = 9;

	ori_lv = Lv;

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

	if(mv_index <pPQA_HMC_MV_C_TBL[0])
	{
		Lv = (Lv>9)?(Lv-9):0;
		//mv_lv=9;
	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[1])
	{
		Lv = (Lv>8)?(Lv-8):0;
		//mv_lv=8;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[2])
	{
		Lv = (Lv>7)?(Lv-7):0;
		//mv_lv=7;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[3])
	{
		Lv = (Lv>6)?(Lv-6):0;
		//mv_lv=6;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[4])
	{
		Lv = (Lv>5)?(Lv-5):0;
		//mv_lv=5;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[5])
	{
		Lv = (Lv>4)?(Lv-4):0;
		//mv_lv=4;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[6])
	{
		Lv = (Lv>3)?(Lv-3):0;
		//mv_lv=3;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[7])
	{
		Lv = (Lv>2)?(Lv-2):0;
		//mv_lv=2;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[8])
	{
		Lv = (Lv>1)?(Lv-1):0;
		//mv_lv=1;

	}
	else if(mv_index <pPQA_HMC_MV_C_TBL[9])
	{
		//mv_lv=0;

	}
	else
	{
		//mv_lv=0;
	}

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

	if(ori_lv<=3)
		Lv=ori_lv;

	if(Lv<=0)
	{
		Lv=0;
	}

	/////////////////// mcnr pan , use the same nr level start////////////////////////////////////////
	if((scalerVIP_DI_MiddleWare_MCNR_Get_Pan_Flag()==1)&&mcnr_pan_nr_level==255)
	{
		mcnr_pan_nr_level = 6;
	}
	else if(scalerVIP_DI_MiddleWare_MCNR_Get_Pan_Flag()==0)
	{
		mcnr_pan_nr_level = 255;
	}

	if(mcnr_pan_nr_level !=255)
	{
		Lv = mcnr_pan_nr_level;
	}

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

/////////////////// mcnr pan , use the same nr level  end ////////////////////////////////////////
	if(level_Idx>pPQA_InputMAD_Chroma_TBL[PQA_I_L09])
		level_Idx = pPQA_InputMAD_Chroma_TBL[PQA_I_L09];
	//tmp = ((pPQA_InputMAD_Chroma_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_Chroma_TBL[PQA_I_L09-Lv-1])==0)?1:(pPQA_InputMAD_Chroma_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_Chroma_TBL[PQA_I_L09-Lv-1]);

	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;

	PQA_Input_Level[I_002_RTNR_1] = Lv;
	PQA_Input_rate[I_002_RTNR_1] = 0;

	//for debug.
	ROSPrintf_INFO_PQA2("I_002_RTNR_1, RTNR_MAD_count_U2_avg=%d,RTNR_MAD_count_V2_avg=%d, level_Idx=%d, mv_index=%d, Get_Pan_Flag()=%d, \n",
		RTNR_MAD_count_U2_avg,RTNR_MAD_count_V2_avg, level_Idx, mv_index, scalerVIP_DI_MiddleWare_MCNR_Get_Pan_Flag());
	ROSPrintf_INFO_PQA2("LV_seq=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d, \n", LV_sequence[0], LV_sequence[1], LV_sequence[2],
		LV_sequence[3], LV_sequence[4], LV_sequence[5], LV_sequence[6], LV_sequence[7], LV_sequence[8], LV_sequence[9]);

}

void Scaler_scalerVIP_compute_level_I_002_RTNR2_tv002(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	unsigned int level_Idx=0/*, printf_Flag=0, print_delay=200*/;
	unsigned char Lv=0;
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
	unsigned int motion = scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION);

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
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_1] = motion;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_2] = meanHistY;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_3] = still_idx;

	PQA_Input_Level[I_002_RTNR_2] = Lv;
	PQA_Input_rate[I_002_RTNR_2] = Lv_intp;

	//printf_Flag = system_info_structure_table->ISR_Printf_Flag.TV002_Flag;
	//print_delay = system_info_structure_table->ISR_Printf_Flag.Delay_Time;
	//for debug.
	//if(((printf_Flag&_BIT3)!=0))
	{
		//if(MA_print_count%print_delay==0)
		{
			ROSPrintf_INFO_PQA1("rm2~lv=%d,st=%d,LvI=%d,mo=%d,y=%d,int=%d,nmL=%d,nmR=%d\n",
				Lv, still_idx, level_Idx, motion, meanHistY, Lv_intp,noise_status,noise_relaible);
			//rtd_pr_vpq_info("mo_i=");
			//for(i=0;i<10;i++)
				//printk("%d,",motioni[i]);
			//printk("\n");
		}
	}

}

void Scaler_scalerVIP_compute_level_I_002_SHP0_tv002(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	char LV_sequence[Lv_Sequence_MAX];
	unsigned char seq_idx = 0;
	int level_Idx=0, /*printf_Flag=0, print_delay=200,*/ mv_index=0,mpeg_index=0;
	signed char Lv=0;/*,mad_lv=0,mv_lv=0,apl_lv=0,film_lv=0,mpeg_lv=0,ori_lv=0;*/
	//  LEVEL_INFO value;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_InputMAD_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_LC_2][0]); //mad
	unsigned char meanHistY = SmartPic_clue->Hist_Y_Mean_Value;
	//unsigned int tmp;//,tmp2;
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned int *pPQA_HMC_MV_w_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_LC_3][0]); //motion
	unsigned int *pPQA_mpeg_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_LC_1][0]); //motion
	unsigned int RTNR_MAD_count_Y2_avg=0;
	//static unsigned char mcnr_pan_nr_level = 255;
	int scalerVIP_color_noise_mpeg_detect_y=0;
	unsigned int *pPQA_Input_HistMeanY_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_DCC_Histogram_mean][0]);
	//unsigned int *pPQA_InputFilm_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_CorrectionBit_Histogram_mean][0]);

	//static char pre_meanHistY=0,prepre_meanHistY=0;
	//char Level=0;

	RTNR_MAD_count_Y2_avg = SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio;
	scalerVIP_color_noise_mpeg_detect_y = scalerVIP_color_Get_noise_mpeg_detect_y();

	level_Idx = scalerVIP_Cal_Idx_Ave_I_002_mcnr(RTNR_MAD_count_Y2_avg, 0);
	mv_index = scalerVIP_compute_I_002_SHP0_hmcnrlevel_Index_Ave(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(),0);
	mpeg_index =scalerVIP_Cal_Idx_Ave_I_002_mpeg(scalerVIP_color_noise_mpeg_detect_y,0);

        if(level_Idx < pPQA_InputMAD_TBL[0])
        {
                Lv = 0;
        }
        else if(level_Idx < pPQA_InputMAD_TBL[1])
        {
                Lv = 1;
        }
        else if(level_Idx < pPQA_InputMAD_TBL[2])
        {
                Lv = 2;
        }
        else if(level_Idx < pPQA_InputMAD_TBL[3])
        {
                Lv = 3;
        }
        else if(level_Idx < pPQA_InputMAD_TBL[4])
        {
                Lv = 4;
        }
        else if(level_Idx < pPQA_InputMAD_TBL[5])
        {
                Lv = 5;
        }
        else if(level_Idx < pPQA_InputMAD_TBL[6])
        {
                Lv = 6;
        }
        else if(level_Idx < pPQA_InputMAD_TBL[7])
        {
                Lv = 7;
        }
        else if(level_Idx < pPQA_InputMAD_TBL[8])
        {
                Lv = 8;
        }
        else
        {
                Lv = 9;

        }

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

        if(mv_index <pPQA_HMC_MV_w_TBL[0])
        {
                Lv = (Lv>9)?(Lv-9):0;
        }
        else if(mv_index <pPQA_HMC_MV_w_TBL[1])
        {
                Lv = (Lv>8)?(Lv-8):0;

        }
        else if(mv_index <pPQA_HMC_MV_w_TBL[2])
        {
                Lv = (Lv>7)?(Lv-7):0;

        }
        else if(mv_index <pPQA_HMC_MV_w_TBL[3])
        {
                Lv = (Lv>6)?(Lv-6):0;

        }
        else if(mv_index <pPQA_HMC_MV_w_TBL[4])
        {
                Lv = (Lv>5)?(Lv-5):0;

        }
        else if(mv_index <pPQA_HMC_MV_w_TBL[5])
        {
                Lv = (Lv>4)?(Lv-4):0;

        }
        else if(mv_index <pPQA_HMC_MV_w_TBL[6])
        {
                Lv = (Lv>3)?(Lv-3):0;

        }
        else if(mv_index <pPQA_HMC_MV_w_TBL[7])
        {
                Lv = (Lv>2)?(Lv-2):0;

        }
        else if(mv_index <pPQA_HMC_MV_w_TBL[8])
        {
                Lv = (Lv>1)?(Lv-1):0;

        }
        else
        {
        }

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

        if(meanHistY<pPQA_Input_HistMeanY_TBL[0])
        {
                Lv = (Lv>9)?(Lv-9):0;
        }
        else if(meanHistY<pPQA_Input_HistMeanY_TBL[1])
        {
                Lv = (Lv>8)?(Lv-8):0;

        }
        else if(meanHistY<pPQA_Input_HistMeanY_TBL[2])
        {
                Lv = (Lv>7)?(Lv-7):0;
        }
        else if(meanHistY<pPQA_Input_HistMeanY_TBL[3])
        {
                Lv = (Lv>6)?(Lv-6):0;
        }
        else if(meanHistY<pPQA_Input_HistMeanY_TBL[4])
        {
                Lv = (Lv>5)?(Lv-5):0;
        }
        else if(meanHistY<pPQA_Input_HistMeanY_TBL[5])
        {
                Lv = (Lv>4)?(Lv-4):0;
        }
        else if(meanHistY<pPQA_Input_HistMeanY_TBL[6])
        {
                Lv = (Lv>3)?(Lv-3):0;
        }
        else if(meanHistY<pPQA_Input_HistMeanY_TBL[7])
        {
                Lv = (Lv>2)?(Lv-2):0;
        }
        else if(meanHistY<pPQA_Input_HistMeanY_TBL[8])
        {
                Lv = (Lv>1)?(Lv-1):0;
        }
        else
        {

        }

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);


		if( mpeg_index <pPQA_mpeg_TBL[0])
		{
		}
		else if(mpeg_index<pPQA_mpeg_TBL[1])
		{
                Lv = Lv+1;
		}
		else if(mpeg_index<pPQA_mpeg_TBL[2])
		{
                Lv = Lv+2;
		}
		else if(mpeg_index<pPQA_mpeg_TBL[3])
		{
                Lv = Lv+3;
		}
		else if(mpeg_index<pPQA_mpeg_TBL[4])
		{
                Lv = Lv+4;
		}
		else if(mpeg_index<pPQA_mpeg_TBL[5])
		{
                Lv = Lv+5;
		}
		else if(mpeg_index<pPQA_mpeg_TBL[6])
		{
                Lv = Lv+6;
		}
		else if(mpeg_index<pPQA_mpeg_TBL[7])
		{
                Lv = Lv+7;
		}
		else if(mpeg_index<pPQA_mpeg_TBL[8])
		{
                Lv = Lv+8;
		}
        else
        {
                Lv=9;
        }

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

	if(Lv>9)
		Lv=9;

	//if( ID_TV002_Flag[ID_RockMountain_TV002] == 1) //Old condition
	//	Lv=0;

        if(ID_TV002_Flag[ID_Radio_TV002]==1)
                Lv=0;

        if(level_Idx>pPQA_InputMAD_TBL[PQA_I_L09])
                level_Idx = pPQA_InputMAD_TBL[PQA_I_L09];
       // tmp = ((pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1])==0)?1:(pPQA_InputMAD_TBL[PQA_I_L09-Lv]-pPQA_InputMAD_TBL[PQA_I_L09-Lv-1]);


        SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = level_Idx;

        PQA_Input_Level[I_002_SHP_0] = Lv;
        PQA_Input_rate[I_002_SHP_0] = 0;

       // prepre_meanHistY = pre_meanHistY;

      //  pre_meanHistY = meanHistY;

	//for debug.
	ROSPrintf_INFO_PQA3("I_002_SHP_0, MAD_Y2_avg=%d, mpeg_detect_y=%d, GMV_Ratio()=%d, level_Idx=%d, mv_index=%d, mpeg_index=%d, meanHistY=%d, \n",
		RTNR_MAD_count_Y2_avg, scalerVIP_color_noise_mpeg_detect_y, scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(),
		level_Idx, mv_index, mpeg_index, meanHistY);
	ROSPrintf_INFO_PQA3("LV_seq=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d, \n", LV_sequence[0], LV_sequence[1], LV_sequence[2],
		LV_sequence[3], LV_sequence[4], LV_sequence[5], LV_sequence[6], LV_sequence[7], LV_sequence[8], LV_sequence[9]);


}

void Scaler_scalerVIP_compute_level_I_002_SHP1_tv002(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	char LV_sequence[Lv_Sequence_MAX];
	unsigned char seq_idx = 0;
	signed char Lv=0;//,mv_lv=0,Hf_lv=0;
	int mv_index=0,step_high_freq=0;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_InputHIGH_FREQ_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_CorrectionBit_Flame_th][0]);
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	unsigned int *pPQA_HMC_MV_w_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_HMC_MV_Hist_motionIdx_offset][0]);
	unsigned int step_count;

	step_count = scalerVIP_Get_Step_CNT();

	mv_index = scalerVIP_Cal_Idx_Ave_I_002_highfreq_motion(scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(),0);
	step_high_freq = scalerVIP_Cal_Idx_Ave_I_002_highfreq(step_count,0);

	if(mv_index <pPQA_HMC_MV_w_TBL[0])
	{
		Lv = 9;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[1])
	{
		Lv = 8;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[2])
	{
		Lv = 7;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[3])
	{
		Lv = 6;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[4])
	{
		Lv = 5;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[5])
	{
		Lv = 4;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[6])
	{
		Lv = 3;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[7])
	{
		Lv = 2;
	}
	else if(mv_index <pPQA_HMC_MV_w_TBL[8])
	{
		Lv = 1;
	}
	else
	{
		Lv = 0;
	}

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

	if(Lv >0)
	{

		if((step_high_freq >=pPQA_InputHIGH_FREQ_TBL[5])&&(pPQA_InputHIGH_FREQ_TBL[5]!=0)) //++
		{
			if((step_high_freq >pPQA_InputHIGH_FREQ_TBL[9])&&(pPQA_InputHIGH_FREQ_TBL[9]!=0))
			{
				Lv +=5;
				Lv = (Lv>9)?(9):Lv;
			}
			else if((step_high_freq >pPQA_InputHIGH_FREQ_TBL[8])&&(pPQA_InputHIGH_FREQ_TBL[8]!=0))
			{
				Lv +=4;
				Lv = (Lv>9)?(9):Lv;

			}
			else if((step_high_freq >pPQA_InputHIGH_FREQ_TBL[7])&&(pPQA_InputHIGH_FREQ_TBL[7]!=0))
			{
				Lv +=3;
				Lv = (Lv>9)?(9):Lv;

			}
			else if((step_high_freq >pPQA_InputHIGH_FREQ_TBL[6])&&(pPQA_InputHIGH_FREQ_TBL[6]!=0))
			{
				Lv +=2;
				Lv = (Lv>9)?(9):Lv;

			}
			else if((step_high_freq >=pPQA_InputHIGH_FREQ_TBL[5])&&(pPQA_InputHIGH_FREQ_TBL[5]!=0))
			{
				Lv +=1;
				Lv = (Lv>9)?(9):Lv;
			}


		}
		else // < 5 --
		{

			if((step_high_freq <pPQA_InputHIGH_FREQ_TBL[0])&&(pPQA_InputHIGH_FREQ_TBL[0]!=0))
			{
				Lv = (Lv>6)?(Lv-6):0;
			}
			else if((step_high_freq <pPQA_InputHIGH_FREQ_TBL[1])&&(pPQA_InputHIGH_FREQ_TBL[1]!=0))
			{
				Lv = (Lv>5)?(Lv-5):0;
			}
			else if((step_high_freq <pPQA_InputHIGH_FREQ_TBL[2])&&(pPQA_InputHIGH_FREQ_TBL[2]!=0))
			{
				Lv = (Lv>4)?(Lv-4):0;

			}
			else if((step_high_freq <pPQA_InputHIGH_FREQ_TBL[3])&&(pPQA_InputHIGH_FREQ_TBL[3]!=0))
			{
				Lv = (Lv>3)?(Lv-3):0;

			}
			else if((step_high_freq <pPQA_InputHIGH_FREQ_TBL[4])&&(pPQA_InputHIGH_FREQ_TBL[4]!=0))
			{
				Lv = (Lv>2)?(Lv-2):0;

			}
			else if((step_high_freq <pPQA_InputHIGH_FREQ_TBL[5])&&(pPQA_InputHIGH_FREQ_TBL[5]!=0))
			{

				Lv = (Lv>1)?(Lv-1):0;

			}



		}

	}

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_0] = Lv;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_1] = step_high_freq;
	SmartPic_clue->PQA_ISR_Cal_Info.Level_Index[PQA_Level_Idx_2] = mv_index;

	PQA_Input_Level[I_002_SHP_1] = Lv;
	PQA_Input_rate[I_002_SHP_1] = 0;

	//for debug.
	ROSPrintf_INFO_PQA4("I_002_SHP_1, step_count=%d, GMV_Ratio()=%d, step_high_freq=%d, mv_index=%d, \n",
		step_count, scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(), step_high_freq, mv_index);
	ROSPrintf_INFO_PQA4("LV_seq=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d, \n", LV_sequence[0], LV_sequence[1], LV_sequence[2],
		LV_sequence[3], LV_sequence[4], LV_sequence[5], LV_sequence[6], LV_sequence[7], LV_sequence[8], LV_sequence[9]);

}

void Scaler_scalerVIP_compute_level_I_002_SNR0_tv002(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	char LV_sequence[Lv_Sequence_MAX];
	unsigned char seq_idx = 0;

	unsigned int apl_Idx=0;
	signed char Lv=0,Apl_Lv=0;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_InputAPL_TBL = &(vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_LC_1][0]);
	unsigned short *PQA_Input_Level = &SmartPic_clue->PQA_ISR_Cal_Info.Input_Level[0];
	unsigned short *PQA_Input_rate = &SmartPic_clue->PQA_ISR_Cal_Info.Input_rate[0];
	//static LEVEL_INFO Level_Info;

	apl_Idx = SmartPic_clue->Hist_Y_Mean_Value;

	if(apl_Idx < pPQA_InputAPL_TBL[0])
		Apl_Lv = 9;
	else if(apl_Idx < pPQA_InputAPL_TBL[1])
		Apl_Lv = 8;
	else if(apl_Idx < pPQA_InputAPL_TBL[2])
		Apl_Lv = 7;
	else if(apl_Idx < pPQA_InputAPL_TBL[3])
		Apl_Lv = 6;
	else if(apl_Idx < pPQA_InputAPL_TBL[4])
		Apl_Lv = 5;
	else if(apl_Idx < pPQA_InputAPL_TBL[5])
		Apl_Lv = 4;
	else if(apl_Idx < pPQA_InputAPL_TBL[6])
		Apl_Lv = 3;
	else if(apl_Idx < pPQA_InputAPL_TBL[7])
		Apl_Lv = 2;
	else if(apl_Idx < pPQA_InputAPL_TBL[8])
		Apl_Lv = 1;
	else
		Apl_Lv = 0;

	Lv = RTNR_Info_Mad_lv -RTNR_Info_Motion_lv+Apl_Lv;

	LV_sequence[seq_idx] = Lv;
	seq_idx = ((seq_idx+1)>=Lv_Sequence_MAX)?(Lv_Sequence_MAX-1):(seq_idx+1);

	if(Lv<0)
		Lv =0;
	if(Lv>9)
		Lv =9;

	PQA_Input_Level[I_002_SNR_0] = Lv;
	PQA_Input_rate[I_002_SNR_0] = 0;

	//for debug.
	ROSPrintf_INFO_PQA5("I_002_SNR_0, apl_Idx=%d, RTNR_Info_Mad_lv=%d, RTNR_Info_Motion_lv=%d, Apl_Lv=%d, \n",
		apl_Idx, RTNR_Info_Mad_lv, RTNR_Info_Motion_lv, Apl_Lv);
	ROSPrintf_INFO_PQA5("LV_seq=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d, \n", LV_sequence[0], LV_sequence[1], LV_sequence[2],
		LV_sequence[3], LV_sequence[4], LV_sequence[5], LV_sequence[6], LV_sequence[7], LV_sequence[8], LV_sequence[9]);

}

void Scaler_scalerVIP_compute_level_I_002_SNR1_tv002(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	int i;
	int mad_value=0, apl_value=0;
	unsigned char mad_lv=0, apl_lv=0;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_I_Noise_level = vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_Noise_level];
	unsigned int *pPQA_I_APL = vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_APL];
	unsigned int *pPQA_weight = vipTable_shareMem->PQA_Input_Table[PQA_table_select][weight1_SNR_NL_APL];
	unsigned short *PQA_Input_Level = SmartPic_clue->PQA_ISR_Cal_Info.Input_Level;
	unsigned short *PQA_Input_rate = SmartPic_clue->PQA_ISR_Cal_Info.Input_rate;

	//unsigned int *PQA_Level_Index = SmartPic_clue->PQA_ISR_Cal_Info.Level_Index;

	mad_value = scalerVIP_Cal_Idx_Ave_I_002_mcnr(SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio, 0)/10;
	apl_value = (SmartPic_clue->Hist_Y_APL_Exact*100)>>10;

	mad_lv =9;
	for (i = 0; i < 9; i++) {
		if(mad_value <= pPQA_I_Noise_level[i]) {
            mad_lv = i;
			break;
		}
	}

	if (pPQA_I_APL[8] > pPQA_I_APL[0]) {
		apl_lv = 9;

		for (i = 0; i < 9; i++) {
			if(apl_value <= pPQA_I_APL[i]) {
				apl_lv = i;
				break;
			}
		}
	} else {
		apl_lv = 0;

		for (i = 0; i < 9; i++) {
			if(apl_value >= pPQA_I_APL[i]) {
				apl_lv = i;
				break;
			}
		}
	}

	PQA_Input_Level[I_002_SNR_1] = (pPQA_weight[0]*mad_lv+pPQA_weight[1]*apl_lv)/100;
	PQA_Input_rate[I_002_SNR_1] = 0;

	if (PQA_Input_Level[I_002_SNR_1] > 9)
		PQA_Input_Level[I_002_SNR_1] = 9;

	//for debug.
	//ROSPrintf_INFO_PQA6("I_002_SNR_1 : MAD_Y2_avg=%d, APL=%d\n", SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio, SmartPic_clue->Hist_Y_APL_Exact);	
	ROSPrintf_INFO_PQA7("I_002_SNR_1 : noise_value=%d, apl_value=%d\n", mad_value, apl_value);
	ROSPrintf_INFO_PQA7("I_002_SNR_1 : noise_lv=%d, apl_lv=%d, I_002_SNR_1=%d\n\n", mad_lv, apl_lv, PQA_Input_Level[I_002_SNR_1]);

}

void Scaler_scalerVIP_compute_level_I_002_SHA_gain(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	int i;
	int mad_value=0, mpeg_value=0;
	unsigned char mad_lv=0, mpeg_lv=0;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_I_Noise_level = vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_Noise_level];
	unsigned int *pPQA_I_BNR_level = vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_BNR_level];
	unsigned int *pPQA_weight = vipTable_shareMem->PQA_Input_Table[PQA_table_select][weight2_Sharpness_NL_BNR];
	unsigned short *PQA_Input_Level = SmartPic_clue->PQA_ISR_Cal_Info.Input_Level;
	unsigned short *PQA_Input_rate = SmartPic_clue->PQA_ISR_Cal_Info.Input_rate;
	//unsigned int *PQA_Level_Index = SmartPic_clue->PQA_ISR_Cal_Info.Level_Index;
	extern int scalerVIP_color_noise_mpeg_h_detect__x_ub_ub;

	mad_value = scalerVIP_Cal_Idx_Ave_I_002_mcnr(SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio, 0)/10;
	mpeg_value = scalerVIP_Cal_Idx_Ave_I_002_mpeg(scalerVIP_color_Get_noise_mpeg_detect_y(),0)*100/scalerVIP_color_noise_mpeg_h_detect__x_ub_ub;

	mad_lv = 9;
	for (i = 0; i < 9; i++) {
		if(mad_value <= pPQA_I_Noise_level[i]) {
            mad_lv = i;
			break;
		}
	}

	mpeg_lv = 9;
	for (i = 0; i < 9; i++) {
		if(mpeg_value <= pPQA_I_BNR_level[i]) {
            mpeg_lv = i;
			break;
		}
	}

	PQA_Input_Level[I_002_SHA_gain] = (pPQA_weight[0]*mad_lv+pPQA_weight[1]*mpeg_lv)/100;
	PQA_Input_rate[I_002_SHA_gain] = 0;

	if (PQA_Input_Level[I_002_SHA_gain] > 9)
		PQA_Input_Level[I_002_SHA_gain] = 9;

	//PQA_Level_Index[0] = PQA_Input_Level[I_002_SHA_gain];

	//for debug.
	//ROSPrintf_INFO_PQA6("I_002_SHA_gain : MAD_Y2_avg=%d, mpeg_detect_y=%d\n", SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio, scalerVIP_color_Get_noise_mpeg_detect_y());
	ROSPrintf_INFO_PQA7("I_002_SHA_gain : noise_value=%d, mpeg_value=%d\n", mad_value, mpeg_value);
	ROSPrintf_INFO_PQA7("I_002_SHA_gain : noise_lv=%d, mpeg_lv=%d, I_002_SHA_gain=%d\n\n", mad_lv, mpeg_lv, PQA_Input_Level[I_002_SHA_gain]);

}

void Scaler_scalerVIP_compute_level_I_002_SHA_G_by_Y(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	int i;
	int mad_value=0, apl_value=0;
	unsigned char mad_lv=0, apl_lv=0;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_I_Noise_level = vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_Noise_level];
	unsigned int *pPQA_I_APL = vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_APL];
	unsigned int *pPQA_weight = vipTable_shareMem->PQA_Input_Table[PQA_table_select][weight1_SNR_NL_APL];
	unsigned short *PQA_Input_Level = SmartPic_clue->PQA_ISR_Cal_Info.Input_Level;
	unsigned short *PQA_Input_rate = SmartPic_clue->PQA_ISR_Cal_Info.Input_rate;
	//unsigned int *PQA_Level_Index = SmartPic_clue->PQA_ISR_Cal_Info.Level_Index;

	mad_value = scalerVIP_Cal_Idx_Ave_I_002_mcnr(SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio, 0)/10;
	apl_value = (SmartPic_clue->Hist_Y_APL_Exact*100)>>10;

	mad_lv = 9;
	for (i = 0; i < 9; i++) {
		if(mad_value <= pPQA_I_Noise_level[i]) {
            mad_lv = i;
			break;
		}
	}

	apl_lv = 9;
	if (pPQA_I_APL[8] > pPQA_I_APL[0]) {
		for (i = 0; i < 9; i++) {
			if(apl_value <= pPQA_I_APL[i]) {
				apl_lv = i;
				break;
			}
		}
	} else {
		for (i = 0; i < 9; i++) {
			if(apl_value >= pPQA_I_APL[i]) {
				apl_lv = i;
				break;
			}
		}
	}

	PQA_Input_Level[I_002_SHA_G_by_Y] = (pPQA_weight[2]*mad_lv+pPQA_weight[3]*apl_lv)/100;
	PQA_Input_rate[I_002_SHA_G_by_Y] = 0;

	if (PQA_Input_Level[I_002_SHA_G_by_Y] > 9)
		PQA_Input_Level[I_002_SHA_G_by_Y] = 9;

	//PQA_Level_Index[0] = PQA_Input_Level[I_002_SHA_G_by_Y];

	//for debug.
	//ROSPrintf_INFO_PQA6("I_002_SHA_G_by_Y : MAD_Y2_avg=%d, APL=%d\n", SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio, SmartPic_clue->Hist_Y_APL_Exact);
	ROSPrintf_INFO_PQA7("I_002_SHA_G_by_Y : noise_value=%d, apl_value=%d\n", mad_value, apl_value);
	ROSPrintf_INFO_PQA7("I_002_SHA_G_by_Y : noise_lv=%d, apl_lv=%d, I_002_SHA_G_by_Y=%d\n\n", mad_lv, apl_lv, PQA_Input_Level[I_002_SHA_G_by_Y]);

}

void Scaler_scalerVIP_compute_level_I_002_SR(SLR_VIP_TABLE* vipTable_shareMem, _clues* SmartPic_clue, _system_setting_info* system_info_structure_table, unsigned int MA_print_count)
{
	int i;
	int freq_det_value=0;
	unsigned char freq_det_lv=0;
	unsigned char PQA_table_select = system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
	unsigned int *pPQA_I_Freq_Det = vipTable_shareMem->PQA_Input_Table[PQA_table_select][I_Freq_Det_level];
	unsigned short *PQA_Input_Level = SmartPic_clue->PQA_ISR_Cal_Info.Input_Level;
	unsigned short *PQA_Input_rate = SmartPic_clue->PQA_ISR_Cal_Info.Input_rate;

	freq_det_value = Scaler_Get_Freq_Det_Out_TV002();

	freq_det_lv = 9;
	for (i = 0; i < 9; i++) {
		if(freq_det_value <= pPQA_I_Freq_Det[i]) {
            freq_det_lv = i;
			break;
		}
	}

	PQA_Input_Level[I_002_SR] = freq_det_lv;
	PQA_Input_rate[I_002_SR] = 0;
	PQA_Input_rate[I_002_SR_COEF] = 0;

	if (PQA_Input_Level[I_002_SR] > 9)
		PQA_Input_Level[I_002_SR] = 9;

	PQA_Input_Level[I_002_SR_COEF] = PQA_Input_Level[I_002_SR];

	//for debug.
	ROSPrintf_INFO_PQA7("I_002_SR : freq_det_value=%d, freq_det_lv = %d, PQA_Input_Level[I_002_SR] = %d\n", freq_det_value, freq_det_lv, PQA_Input_Level[I_002_SR]);
}


//====================================================================================
//****************************************************PQA****************************************************
//====================================================================================

char Scaler_fwif_color_VPQ_SIP_DMA_init(void)
{
#ifdef VIP_SUPPORT_SIP_1_SPI_2
	char ret_val = 0;
	unsigned int addr = 0, size;
	unsigned int addr_aligned = 0, size_aligned;
#ifdef CONFIG_ARM64 //ARM32 compatible
	unsigned long va_temp;
#else
	unsigned int va_temp;
#endif
	unsigned int  *pVir_addr_aligned = NULL;
	unsigned int *pVir_addr = 0;

	rtd_pr_vpq_info("SIP_DMA, SIP1_vir_addr=%p, SIP2_vir_addr=%p,\n", SIP_DMA_CTRL_SIP_1.pVir_addr_align, SIP_DMA_CTRL_SIP_2.pVir_addr_align);
	
	if (SIP_DMA_CTRL_SIP_1.pVir_addr_align == NULL) {
		
		addr = get_query_VIP_DMAto3DTABLE_start_address(DMAto3DTABLE_SIP_1);

		addr = drvif_memory_get_data_align(addr, (1 << 12));
		size = drvif_memory_get_data_align(VIP_DMAto3DTable_SIP_1_BUFFER_SIZE, (1 << 12));

		if (addr == 0) {
			rtd_pr_vpq_emerg("SIP_DMA, Scaler_fwif_color_VPQ_SIP_init, SIP_1 addr = NULL\n");
			ret_val = -1;
		}

		size_aligned = size;
		addr_aligned = addr;

#ifdef CONFIG_BW_96B_ALIGNED
		size_aligned = dvr_size_alignment(size);
#endif
		pVir_addr = dvr_remap_cached_memory(addr, PAGE_ALIGN(size_aligned), __builtin_return_address(0));

#ifdef CONFIG_ARM64 //ARM32 compatible
		va_temp = (unsigned long)pVir_addr;
#else
		va_temp = (unsigned int)pVir_addr;
#endif
		pVir_addr_aligned = (unsigned int*)va_temp;

#ifdef CONFIG_BW_96B_ALIGNED
		addr_aligned = (unsigned int)dvr_memory_alignment((unsigned long)addr, size_aligned);
#ifdef CONFIG_ARM64 //ARM32 compatible
		va_temp = (unsigned long)pVir_addr;
#else //CONFIG_ARM64
		va_temp = (unsigned int)pVir_addr;
#endif //CONFIG_ARM64
		pVir_addr_aligned = (unsigned int*)dvr_memory_alignment((unsigned long)va_temp, size_aligned);
#endif		
		SIP_DMA_CTRL_SIP_1.phy_addr_align = addr_aligned;
		SIP_DMA_CTRL_SIP_1.size = size;
		SIP_DMA_CTRL_SIP_1.pVir_addr_align = pVir_addr_aligned;
	}
	//dvr_unmap_memory((void *)pVir_addr, PAGE_ALIGN(size_aligned));
	if (SIP_DMA_CTRL_SIP_2.pVir_addr_align == NULL) {
		
		addr = get_query_VIP_DMAto3DTABLE_start_address(DMAto3DTABLE_SIP_2);

		addr = drvif_memory_get_data_align(addr, (1 << 12));
		size = drvif_memory_get_data_align(VIP_DMAto3DTable_SIP_2_BUFFER_SIZE, (1 << 12));

		if (addr == 0) {
			rtd_pr_vpq_emerg("SIP_DMA,  Scaler_fwif_color_VPQ_SIP_init, SIP_2 addr = NULL\n");
			ret_val = -1;
		}

		size_aligned = size;
		addr_aligned = addr;

#ifdef CONFIG_BW_96B_ALIGNED
		size_aligned = dvr_size_alignment(size);
#endif
		pVir_addr = dvr_remap_cached_memory(addr, PAGE_ALIGN(size_aligned), __builtin_return_address(0));

#ifdef CONFIG_ARM64 //ARM32 compatible
		va_temp = (unsigned long)pVir_addr;
#else
		va_temp = (unsigned int)pVir_addr;
#endif
		pVir_addr_aligned = (unsigned int*)va_temp;

#ifdef CONFIG_BW_96B_ALIGNED
		addr_aligned = (unsigned int)dvr_memory_alignment((unsigned long)addr, size_aligned);
#ifdef CONFIG_ARM64 //ARM32 compatible
		va_temp = (unsigned long)pVir_addr;
#else //CONFIG_ARM64
		va_temp = (unsigned int)pVir_addr;
#endif //CONFIG_ARM64
		pVir_addr_aligned = (unsigned int*)dvr_memory_alignment((unsigned long)va_temp, size_aligned);
#endif		
		SIP_DMA_CTRL_SIP_2.phy_addr_align = addr_aligned;
		SIP_DMA_CTRL_SIP_2.size = size;
		SIP_DMA_CTRL_SIP_2.pVir_addr_align = pVir_addr_aligned;
	}
	//dvr_unmap_memory((void *)pVir_addr, PAGE_ALIGN(size_aligned));

	rtd_pr_vpq_info("SIP_DMA, SIP1_vir_addr=%p, SIP2_vir_addr=%p, SIP1_phy_addr=%x,  SIP2_phy_addr=%x\n", 
		SIP_DMA_CTRL_SIP_1.pVir_addr_align, SIP_DMA_CTRL_SIP_2.pVir_addr_align,
		SIP_DMA_CTRL_SIP_1.phy_addr_align, SIP_DMA_CTRL_SIP_2.phy_addr_align);

	return ret_val;
#else
	return 0;
#endif
}

char Scaler_fwif_color_VPQ_SIP_init(void)
{
#ifdef VIP_SUPPORT_SIP_1_SPI_2
	unsigned short h_sta, v_sta, h_size, v_size, h_end, v_end;
	_system_setting_info *sys_info = NULL;
	char ret = 0;
	sys_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (sys_info == NULL) {
		rtd_pr_vpq_emerg("SIP_DMA, fwif_color_VPQ_SIP_init, table NULL.\n");
		return -1;
	}

	if (Scaler_fwif_color_VPQ_SIP_DMA_init() < 0) {
		rtd_pr_vpq_emerg("SIP_DMA, fwif_color_VPQ_SIP_init, DMA -1.\n");
		ret = -1;
	} else {
		drvif_color_VPQ_SIP_TOP_CLK();

		drvif_color_Get_DTG_S1_Timing(&h_sta, &v_sta, &h_end, &v_end);

		//h_sta = Get_DISP_DEN_END_HPOS();	// need to check
		//v_sta = Get_DISP_DEN_END_VPOS();	// need to check
		h_size = Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
		v_size = Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN);

		rtd_pr_vpq_info("VPQ_SIP, h_sta=%d, v_sta=%d, h_size=%d, v_size=%d, h_end=%d, v_end=%d,\n", h_sta, v_sta, h_size, v_size, h_end, v_end);
		if (SIP_DMA_CTRL_SIP_1.phy_addr_align != 0 && SIP_DMA_CTRL_SIP_1.pVir_addr_align != NULL)
			drvif_color_VPQ_SIP_1_init(h_sta, v_sta, h_size, v_size, SIP_DMA_CTRL_SIP_1.phy_addr_align, VIP_SIP_1_Size_default);
		if (SIP_DMA_CTRL_SIP_2.phy_addr_align != 0 && SIP_DMA_CTRL_SIP_2.pVir_addr_align != NULL)
			drvif_color_VPQ_SIP_2_init(h_sta, v_sta, h_size, v_size, SIP_DMA_CTRL_SIP_2.phy_addr_align, VIP_SIP_2_Size_default);
		
		ret = 0;
	}
	return ret;
#else
	return 0;
#endif
}

char Scaler_fwif_color_set_VPQ_SIP_REG(unsigned int *pCoefTBL, unsigned int sta_addr, unsigned int size)
{
#ifdef VIP_SUPPORT_SIP_1_SPI_2
	if (pCoefTBL == NULL) {
		rtd_pr_vpq_emerg("fwif_color_set_VPQ_SIP, table NULL.\n");
		return -1;

	}
	drvif_color_Set_SIP_REG(pCoefTBL, sta_addr, size);
#endif
	return 0;
}

char Scaler_fwif_color_get_VPQ_SIP_REG(unsigned int *pCoefTBL, unsigned int sta_addr, unsigned int size)
{
#ifdef VIP_SUPPORT_SIP_1_SPI_2
	if (pCoefTBL == NULL) {
		rtd_pr_vpq_emerg("fwif_color_get_VPQ_SIP, 1 table NULL.\n");
		return -1;

	}
	drvif_color_Get_SIP_REG(pCoefTBL, sta_addr, size);
#endif
	return 0;
}
// endian is diff between SCPU and SIP Register
char Scaler_fwif_color_set_VPQ_SIP_1_ISR_DMA(unsigned char LUT_Ctrl, unsigned int *pArray)
{
#ifdef VIP_SUPPORT_SIP_1_SPI_2
	char ret = 0;
	unsigned int addr;
	unsigned int* pVir_addr;
	unsigned int size;

	addr = SIP_DMA_CTRL_SIP_1.phy_addr_align;
	pVir_addr = SIP_DMA_CTRL_SIP_1.pVir_addr_align;
	size = SIP_DMA_CTRL_SIP_1.size;

	if (pVir_addr == NULL || pArray==NULL) {
		rtd_pr_vpq_emerg("SIP_DMA, SIP1_DMA, , Scaler_fwif_color_set_VPQ_SIP_1_ISR_DMA, pArray=%p, pVir_addr=%p,\n", pArray, pVir_addr);
		ret = -1;

	} else {
		if (drvif_color_SIP1_apply_done_Check_DMA(1) == 0) {
			drvif_color_SIP1_ERR_Check_DMA(1);
			drvif_color_reset_SIP1_DMA();
			drvif_color_set_SIP1_DMA(LUT_Ctrl, pArray, addr, pVir_addr, size);
		}
	}

	return ret;
#else
	return 0;
#endif
}

char Scaler_fwif_color_set_VPQ_SIP_2_ISR_DMA(unsigned char LUT_Ctrl, unsigned int *pArray)
{
#ifdef VIP_SUPPORT_SIP_1_SPI_2
	char ret = 0;
	unsigned int addr;
	unsigned int* pVir_addr;
	unsigned int size;

	addr = SIP_DMA_CTRL_SIP_2.phy_addr_align;
	pVir_addr = SIP_DMA_CTRL_SIP_2.pVir_addr_align;
	size = SIP_DMA_CTRL_SIP_2.size;

	if (pVir_addr == NULL || pArray==NULL) {
		rtd_pr_vpq_emerg("SIP_DMA, SIP2_DMA, , Scaler_fwif_color_set_VPQ_SIP_2_ISR_DMA, pArray=%p, pVir_addr=%p,\n", pArray, pVir_addr);
		ret = -1;

	} else {
		if (drvif_color_SIP2_apply_done_Check_DMA(1) == 0) {
			drvif_color_SIP2_ERR_Check_DMA(1);
			drvif_color_reset_SIP2_DMA();
			drvif_color_set_SIP2_DMA(LUT_Ctrl, pArray, addr, pVir_addr, size);
		}
	}

	return ret;
#else
	return 0;
#endif
}

/************************************************************/
/******************* For TV002 use*****************************/
/************************************************************/

BL_DBC_ALL_PARAM_T g_dbc_param;
BL_CTRL_ITEM_INIT_T g_dbc_init = {0};
bool g_dbc_stop_flag = false;
bool g_dbc_sw_mode_on = false;
bool g_dbc_pwm_init_setup_once = true;

unsigned char Scaler_Get_BL_ON_OFF(unsigned int print_count)
{
#ifdef CONFIG_CUSTOMER_TV002
	RTK_GPIO_VAL_T gpio_val = {"PIN_BL_ON_OFF", 0};
	RTK_GPIO_PCBINFO_T pinInfo;

	int ret = rtk_gpio_get_pcb_info(gpio_val.pin_name,&pinInfo);
	if(ret<0) {
		BL_DBC_Print3(print_count, "RTK_GPIO_GET_VAL: can not find the pin:%s ,please check bootcode pcbenmu!!\n",gpio_val.pin_name);
		return 0;
	} else {
		if(pinInfo.input_type==PCB_GPIO_TYPE_OUPUT){
			ret = rtk_gpio_output_get(pinInfo.gid);
			if(ret<0){
				BL_DBC_Print3(print_count, "RTK_GPIO_GET_VAL: input PIN can'nt get output value,gpio_type=%s,pin:%s ,please check bootcode pcbenmu!!\n",gpio_type(pinInfo.gpio_type),gpio_val.pin_name);
                return 0;
			}
		} else {
			ret = rtk_gpio_input(pinInfo.gid);
			if(ret<0) {
				BL_DBC_Print3(print_count, "RTK_GPIO_GET_VAL: output PIN can'nt get input value,gpio_type=%s,pin:%s ,please check bootcode pcbenmu!!\n",gpio_type(pinInfo.gpio_type),gpio_val.pin_name);
				return 0;
			}
			if(pinInfo.invert != 0)
				ret = (ret) ? 0 : 1;
		}
		gpio_val.val = ret;
		//BL_DBC_Print3(print_count, "PIN_BL = %d\n", ret);
	}

	return gpio_val.val;
#else
	return 0;
#endif
}

void Scaler_DBC_Set_To_Device_TV002(unsigned char isNonISR)
{
	#define Dclk 594000000 //Dclk = 594 MHz
	#define Vsync 60 //Vsync = 48,50,60 Hz
	const unsigned int GAIN_BASE = 1024;
	BL_DBC_ALL_PARAM_T *p = &g_dbc_param;
	int pwmRatio = 0, PWM_RATIO1 = 0, PWM_RATIO2 = 0, fmPwmRatio = 0, fmPwmPulse = 0, fmPwmPhase = 0, currentRatio = 0, blinkRatio = 0;//PWM_RATIO = 0, 
	int blinkPulse = 0, blinkPhase = 0, pwmFlat = 1024;
	unsigned short BL_PWM_FREQ = 116;
	#ifdef CONFIG_CUSTOMER_TV002
	unsigned short BL_CUR_FREQ = 77;
	#endif
	unsigned int print_count = ddomain_print_count;
	unsigned char ucBL_ON_OFF;

	ucBL_ON_OFF = Scaler_Get_BL_ON_OFF(print_count);

	if (g_dbc_stop_flag || !ucBL_ON_OFF || !PPOVERLAY_DTG_ie_2_get_mv_den_end_event_ie_2(IoReg_Read32(PPOVERLAY_DTG_ie_2_reg))) {
		BL_DBC_Print3(print_count, "Scaler_DBC_Set_To_Device_TV002 skipped!! g_dbc_stop_flag = %d, BL_ON_OFF = %d\n", g_dbc_stop_flag, ucBL_ON_OFF);
		return;
	}

	if (!g_dbc_init.bBL_DBC_BLENDING_OUT) {
		p->dbc_blend_out.BL_GAIN = 1024;
	}

	if (!g_dbc_init.bBL_LD_LIB_OUT) {
		p->ld_lib.HEAT_BLSAVE = 1023;
	}

	//prvent div 0
	if (p->config.BL_PWM_FREQ != 0)
		BL_PWM_FREQ = p->config.BL_PWM_FREQ;
#ifdef CONFIG_CUSTOMER_TV002
	if (p->config.BL_CUR_FREQ != 0)
		BL_CUR_FREQ = p->config.BL_CUR_FREQ;

#endif
	//PWM_RATIO1 and PWM_RATIO2
	PWM_RATIO1 = p->dbc_blend_out.BL_GAIN;
	PWM_RATIO2 = p->pwm_param.SBC_GAIN;

	//Normal Mode
	if (p->pwm_flat_param.BL_FLAT_MODE == 0)
	{
		pwmRatio = PWM_RATIO1 * PWM_RATIO2 / GAIN_BASE; //DBC & SBC (PWM Ratio 1 & 2)
		pwmRatio = pwmRatio * p->ld_lib.HEAT_BLSAVE / 1023; //0DD (HEAT_BLSAVE)
		//Scale (pwmRatio, 0, GAIN_BASE, 0, mPanelData.bl_pwm_max); //Limiter
		pwmRatio = pwmRatio*(p->config.BL_PWM_MAX)/GAIN_BASE;
		if (0 < pwmRatio)
		{  //0 is not limited.
		    if(pwmRatio > p->config.BL_PWM_MAX)
		        pwmRatio = p->config.BL_PWM_MAX;
		    if(pwmRatio < p->config.BL_PWM_MIN)
		        pwmRatio = p->config.BL_PWM_MIN;
		} else {
		    if(pwmRatio > p->config.BL_PWM_MAX)
		        pwmRatio = p->config.BL_PWM_MAX;
		}
		//PWM_RATIO = pwmRatio;
		if (p->config.FM_PWM_MODE == 0) {
			if (p->blink_param.BLINK_EN == 0)
			{
				fmPwmRatio = pwmRatio;
				fmPwmPulse = (Dclk/4)/2/(31+1)/BL_PWM_FREQ/Vsync;
				fmPwmPhase = p->config.PWM_PHASE;
			}
			else
			{
				fmPwmRatio = p->blink_param.BLINK_RATIO*pwmRatio/GAIN_BASE;
				fmPwmPulse = p->blink_param.BLINK_PULSE;
				fmPwmPhase = p->blink_param.BLINK_PHASE;
			}
		} else {
			fmPwmRatio = pwmRatio;
			fmPwmPulse = (Dclk/4)/2/(31+1)/BL_PWM_FREQ/Vsync;
			fmPwmPhase = p->config.FM_PWM_PHASE;
		}
		currentRatio = p->cur_param.CURRENT;
		blinkRatio = p->blink_param.BLINK_RATIO;
		blinkPulse = p->blink_param.BLINK_PULSE;
		blinkPhase = p->blink_param.BLINK_PHASE;
		//Flat PWM Ratio Setting
		pwmFlat = 1024;
	}
	else if (p->pwm_flat_param.BL_FLAT_MODE == 1)
	{   //Mute flat
		pwmFlat = p->pwm_flat_param.BL_FLAT_GAIN;
		currentRatio = p->cur_param.CURRENT;
		pwmRatio = pwmFlat;
		blinkRatio = 1024;
		blinkPulse = 1;
		blinkPhase = 0;
		fmPwmRatio = pwmFlat;
		fmPwmPulse = (Dclk/4)/2/(31+1)/BL_PWM_FREQ/Vsync;
		fmPwmPhase = 0;
	}
	else
	{ //Aging
		blinkRatio = 1024;
		blinkPulse = 1;
		blinkPhase = 0;
		pwmFlat = 1024;
		currentRatio = p->config.CURRENT_MAX;
		pwmRatio = pwmFlat;
		fmPwmRatio = pwmFlat;
		fmPwmPulse = (Dclk/4)/2/(31+1)/BL_PWM_FREQ/Vsync;
		fmPwmPhase = 0;
	}

	if (isNonISR) {
		BL_DBC_Print3(print_count,"======[Settings_to_the_Device]======\n");
		BL_DBC_Print3(print_count,"pwm_param : SBC_GAIN = %d, SBC_GAIN_SUB = %d \n",p->pwm_param.SBC_GAIN, p->pwm_param.SBC_GAIN_SUB);
		BL_DBC_Print3(print_count,"cur_param : CURRENT = %d \n",p->cur_param.CURRENT);
		BL_DBC_Print3(print_count,"blink_param : BLINK_EN = %d, BLINK_RATIO = %d, BLINK_PULSE = %d, BLINK_PHASE = %d \n",p->blink_param.BLINK_EN, p->blink_param.BLINK_RATIO, p->blink_param.BLINK_PULSE, p->blink_param.BLINK_PHASE);
		BL_DBC_Print3(print_count,"pwm_flat_param : BL_FLAT_MODE = %d, BL_FLAT_GAIN = %d \n",p->pwm_flat_param.BL_FLAT_MODE, p->pwm_flat_param.BL_FLAT_GAIN);
		BL_DBC_Print3(print_count,"config : CURRENT_MAX = %d ,BL_CUR_FREQ = %d \n",p->config.CURRENT_MAX, p->config.BL_CUR_FREQ);
		BL_DBC_Print3(print_count,"config : BL_PWM_MAX = %d, BL_PWM_MIN = %d, BL_PWM_FREQ = %d \n",p->config.BL_PWM_MAX, p->config.BL_PWM_MIN, p->config.BL_PWM_FREQ);
		BL_DBC_Print3(print_count,"config : PWM_PHASE = %d, FM_PWM_PHASE = %d, FM_PWM_MODE = %d \n",p->config.PWM_PHASE, p->config.FM_PWM_PHASE, p->config.FM_PWM_MODE);
		//BL_DBC_Print(print_count,"--dbc_config--");
		BL_DBC_Print3(print_count,"dbc_blend_out : BL_GAIN = %d \n",p->dbc_blend_out.BL_GAIN);
		BL_DBC_Print3(print_count,"ld_lib : HEAT_BLSAVE = %d \n",p->ld_lib.HEAT_BLSAVE);

		BL_DBC_Print3(print_count,"--result--");
		BL_DBC_Print3(print_count,"pwmFlat = %d \n",pwmFlat);
		BL_DBC_Print3(print_count,"pwmRatio = %d \n",pwmRatio);
		BL_DBC_Print3(print_count,"currentRatio = %d \n",currentRatio);
		BL_DBC_Print3(print_count,"blinkRatio = %d \n",blinkRatio);
		BL_DBC_Print3(print_count,"blinkPulse = %d \n",blinkPulse);
		BL_DBC_Print3(print_count,"blinkPhase = %d \n",blinkPhase);
		BL_DBC_Print3(print_count,"fmPwmRatio = %d \n",fmPwmRatio);
		BL_DBC_Print3(print_count,"fmPwmPulse = %d \n",fmPwmPulse);
		BL_DBC_Print3(print_count,"fmPwmPhase = %d \n",fmPwmPhase);
	} else {
		BL_DBC_Print(print_count,"======[Settings_to_the_Device]======\n");
		BL_DBC_Print(print_count,"pwm_param : SBC_GAIN = %d, SBC_GAIN_SUB = %d \n",p->pwm_param.SBC_GAIN, p->pwm_param.SBC_GAIN_SUB);
		BL_DBC_Print(print_count,"cur_param : CURRENT = %d \n",p->cur_param.CURRENT);
		BL_DBC_Print(print_count,"blink_param : BLINK_EN = %d, BLINK_RATIO = %d, BLINK_PULSE = %d, BLINK_PHASE = %d \n",p->blink_param.BLINK_EN, p->blink_param.BLINK_RATIO, p->blink_param.BLINK_PULSE, p->blink_param.BLINK_PHASE);
		BL_DBC_Print(print_count,"pwm_flat_param : BL_FLAT_MODE = %d, BL_FLAT_GAIN = %d \n",p->pwm_flat_param.BL_FLAT_MODE, p->pwm_flat_param.BL_FLAT_GAIN);
		BL_DBC_Print(print_count,"config : CURRENT_MAX = %d ,BL_CUR_FREQ = %d \n",p->config.CURRENT_MAX, p->config.BL_CUR_FREQ);
		BL_DBC_Print(print_count,"config : BL_PWM_MAX = %d, BL_PWM_MIN = %d, BL_PWM_FREQ = %d \n",p->config.BL_PWM_MAX, p->config.BL_PWM_MIN, p->config.BL_PWM_FREQ);
		BL_DBC_Print(print_count,"config : PWM_PHASE = %d, FM_PWM_PHASE = %d, FM_PWM_MODE = %d \n",p->config.PWM_PHASE, p->config.FM_PWM_PHASE, p->config.FM_PWM_MODE);
		//BL_DBC_Print(print_count,"--dbc_config--");
		BL_DBC_Print(print_count,"dbc_blend_out : BL_GAIN = %d \n",p->dbc_blend_out.BL_GAIN);
		BL_DBC_Print(print_count,"ld_lib : HEAT_BLSAVE = %d \n",p->ld_lib.HEAT_BLSAVE);

		BL_DBC_Print(print_count,"--result--");
		BL_DBC_Print(print_count,"pwmFlat = %d \n",pwmFlat);
		BL_DBC_Print(print_count,"pwmRatio = %d \n",pwmRatio);
		BL_DBC_Print(print_count,"currentRatio = %d \n",currentRatio);
		BL_DBC_Print(print_count,"blinkRatio = %d \n",blinkRatio);
		BL_DBC_Print(print_count,"blinkPulse = %d \n",blinkPulse);
		BL_DBC_Print(print_count,"blinkPhase = %d \n",blinkPhase);
		BL_DBC_Print(print_count,"fmPwmRatio = %d \n",fmPwmRatio);
		BL_DBC_Print(print_count,"fmPwmPulse = %d \n",fmPwmPulse);
		BL_DBC_Print(print_count,"fmPwmPhase = %d \n",fmPwmPhase);
	}

	if (!g_dbc_init.bBL_PWM_PARAM_SET || !g_dbc_init.bBL_CURRENT_PARAM_SET || !g_dbc_init.bBL_BLINK_PARAM_SET
		|| !g_dbc_init.bBL_PWM_FLAT_PARAM_SET || !g_dbc_init.bBL_CONFIG_SET || !g_dbc_init.bBL_DBC_CONFIG_SET) {

		BL_DBC_Print(print_count,"init states = %d, %d, %d, %d, %d, %d \n",
			g_dbc_init.bBL_PWM_PARAM_SET,
			g_dbc_init.bBL_CURRENT_PARAM_SET,
			g_dbc_init.bBL_BLINK_PARAM_SET,
			g_dbc_init.bBL_PWM_FLAT_PARAM_SET,
			g_dbc_init.bBL_CONFIG_SET,
			g_dbc_init.bBL_DBC_CONFIG_SET
		);
		return;
	} else
		g_dbc_sw_mode_on = true;

#if 0
	//Output to device
    //Blinking (GPIO10) and Frequency Manipulated PWM (GPIO12): phase and frequency setting
    // (GPIO10): F10 -> RTD287O_BGA_22x22_PIN_F10_PWM_0 -> Sync PWM (Blinking) -> BLINKING -> PIN_PWM0_DIMMER
	SetPwmDuty(GPIO10, blinkRatio/4);
	SetPwmFrequency(GPIO10, VSync*blinkPulse);
	SetPwmPhase(GPIO10, blinkPhase)

	// (GPIO12): E9 -> RTD287O_BGA_22x22_PIN_E9_PWM_2 -> Sync PWM (PWM Dimmer 2) -> Frequency Manipulated PWM -> PIN_PWM2_DIMMER
	SetPwmDuty(GPIO12, fmPwmRatio/4);
	SetPwmFrequency(GPIO12, Vsync*fmPwmPulse);
	SetPwmPhase(GPIO10, fmPwmPhase)

    //CurrrentRatio (GPIO13) and PWMRatio (GPIO11): phase & frequency set once only
    // (GPIO11): G10 -> RTD287O_BGA_22x22_PIN_G10_PWM_1 -> General PWM (PWM Dimmer 1) -> PWM1_DIMMER -> PIN_BL_ADJ
	SetPwmDuty(GPIO11, pwmRatio/4);
	SetPwmFrequency(GPIO11, [Dclk/4]/2/[31+1]/mPanelData.bl_pwm_freq);
	SetPwmPhase(GPIO11, pwmPhase)

	// (GPIO13): F9 -> RTD287O_BGA_22x22_PIN_F9_PWM_3 -> General PWM (I_Peak) -> DC_DIMMER/IP_REF -> PIN_DC_DIMMER_IP_REF
	SetPwmDuty(GPIO13, currentRatio/4);
	SetPwmFrequency(GPIO13, [Dclk/4]/2/[31+1]/mPanelData.bl_cur_freq);
	SetPwmPhase(GPIO13, 0)

	*Dclk = 594 MHz
#endif

    //Output to device
    {
        #ifdef CONFIG_CUSTOMER_TV002
        const int const_pwm_panelmax = 256;
        int rtkPwmVal = 0;
        int rtkPwmApply = 0;
        static int RTK_PWM_0_Freq = 0;
        static int RTK_PWM_0_Duty = 0;
        static int RTK_PWM_0_Delay = 0;
        static int RTK_PWM_0_Vsync = 0;
        static int RTK_PWM_1_Freq = 0;
        static int RTK_PWM_1_Duty = 0;
        static int RTK_PWM_2_Freq = 0;
        static int RTK_PWM_2_Duty = 0;
        static int RTK_PWM_2_Delay = 0;
        static int RTK_PWM_2_Vsync = 0;
        static int RTK_PWM_3_Freq = 0;
        static int RTK_PWM_3_Duty = 0;

        //CurrrentRatio (GPIO13) and PWMRatio (GPIO11): phase & frequency set once only
        // (GPIO11): G10 -> RTD287O_BGA_22x22_PIN_G10_PWM_1 -> General PWM (PWM Dimmer 1) -> PWM1_DIMMER -> PIN_BL_ADJ
        rtkPwmVal = pwmRatio/4;
        rtkPwmVal = rtkPwmVal > const_pwm_panelmax ? const_pwm_panelmax : rtkPwmVal;
        if(rtkPwmVal != RTK_PWM_1_Duty) {
            BL_DBC_Print(print_count,"GPIO11 SetPwmDuty :%d\n",rtkPwmVal);
            rtk_pwm_backlight_set_duty_ex(RTK_PWM_1, rtkPwmVal);
            RTK_PWM_1_Duty = rtkPwmVal;
            rtkPwmApply |= 1;
        }

        if(g_dbc_pwm_init_setup_once && isNonISR) {
            rtkPwmVal = (Dclk/4)/2/(31+1)/BL_PWM_FREQ;
            if(rtkPwmVal != RTK_PWM_1_Freq) {
                BL_DBC_Print3(print_count,"GPIO11 SetPwmFrequency: %d Hz\n",rtkPwmVal);
                rtk_pwm_backlight_set_freq_ex(RTK_PWM_1, rtkPwmVal);
                RTK_PWM_1_Freq = rtkPwmVal;
                rtkPwmApply |= 1;
            }
        }

        /*
        BL_DBC_Print(print_count,"GPIO11 SetPwmPhase: %d ",0);
        rtk_pwm_backlight_set_vsync_delay_ex(RTK_PWM_1,PWM_POS_DELAY,0);

        BL_DBC_Print(print_count,"GPIO11 PWM SetVsyncAlign => enable vsync");
        rtk_pwm_backlight_set_vsync_ex(RTK_PWM_1,1);
        */
        if(rtkPwmApply == 1) {
            rtk_pwm_backlight_apply(RTK_PWM_1);
        }

        // (GPIO13): F9 -> RTD287O_BGA_22x22_PIN_F9_PWM_3 -> General PWM (I_Peak) -> DC_DIMMER/IP_REF -> PIN_DC_DIMMER_IP_REF
        rtkPwmApply = 0;
        if (isNonISR) {
            rtkPwmVal = currentRatio/4;
            rtkPwmVal = rtkPwmVal > const_pwm_panelmax ? const_pwm_panelmax : rtkPwmVal;
            if(rtkPwmVal != RTK_PWM_3_Duty) {
                BL_DBC_Print3(print_count,"GPIO13 SetPwmDuty :%d\n",rtkPwmVal);
                rtk_pwm_backlight_set_duty_ex(RTK_PWM_3, rtkPwmVal);
                RTK_PWM_3_Duty = rtkPwmVal;
                rtkPwmApply |= 1;
            }

            if(g_dbc_pwm_init_setup_once) {
                g_dbc_pwm_init_setup_once = false;
                rtkPwmVal = (Dclk/4)/2/(31+1)/BL_CUR_FREQ;
                if(rtkPwmVal != RTK_PWM_3_Freq) {
                    BL_DBC_Print3(print_count,"GPIO13 SetPwmFrequency: %d Hz\n",rtkPwmVal);
                    rtk_pwm_backlight_set_freq_ex(RTK_PWM_3, rtkPwmVal);
                    RTK_PWM_3_Freq = rtkPwmVal;
                    rtkPwmApply |= 1;
                }
            }

            /*
            BL_DBC_Print(print_count,"GPIO13 SetPwmPhase: %d ",0);
            rtk_pwm_backlight_set_vsync_delay_ex(RTK_PWM_3,PWM_POS_DELAY,0);

            BL_DBC_Print(print_count,"GPIO13 PWM SetVsyncAlign => enable vsync");
            rtk_pwm_backlight_set_vsync_ex(RTK_PWM_3,1);
            */
            if(rtkPwmApply == 1) {
                rtk_pwm_backlight_apply(RTK_PWM_3);
            }
        }

        //Blinking (GPIO10) and Frequency Manipulated PWM (GPIO12): phase and frequency setting
        // (GPIO10): F10 -> RTD287O_BGA_22x22_PIN_F10_PWM_0 -> Sync PWM (Blinking) -> BLINKING -> PIN_PWM0_DIMMER
        rtkPwmApply = 0;
        if (isNonISR) {
            rtkPwmVal = blinkRatio/4;
            rtkPwmVal = rtkPwmVal > const_pwm_panelmax ? const_pwm_panelmax : rtkPwmVal;
            if(rtkPwmVal != RTK_PWM_0_Duty) {
                BL_DBC_Print3(print_count,"GPIO10 SetPwmDuty :%d\n",rtkPwmVal);
                rtk_pwm_backlight_set_duty_ex(RTK_PWM_0, rtkPwmVal);
                RTK_PWM_0_Duty = rtkPwmVal;
                rtkPwmApply |= 1;
            }

            rtkPwmVal = Vsync*blinkPulse;
            if(rtkPwmVal != RTK_PWM_0_Freq) {
                BL_DBC_Print3(print_count,"GPIO10 SetPwmFrequency: %d Hz\n",rtkPwmVal);
                rtk_pwm_backlight_set_freq_ex(RTK_PWM_0, rtkPwmVal);
                RTK_PWM_0_Freq = rtkPwmVal;
                rtkPwmApply |= 1;
            }

            if(rtkPwmVal != RTK_PWM_0_Delay) {
                BL_DBC_Print3(print_count,"GPIO10 SetPwmPhase: %d\n",blinkPhase);
                rtk_pwm_backlight_set_vsync_delay_ex(RTK_PWM_0,PWM_POS_DELAY,blinkPhase);
                RTK_PWM_0_Delay = rtkPwmVal;
                rtkPwmApply |= 1;
            }

            if(rtkPwmVal != RTK_PWM_0_Vsync) {
                BL_DBC_Print3(print_count,"GPIO10 PWM SetVsyncAlign => enable vsync\n");
                rtk_pwm_backlight_set_vsync_ex(RTK_PWM_0,1);
                RTK_PWM_0_Vsync = rtkPwmVal;
                rtkPwmApply |= 1;
            }

            if(rtkPwmApply == 1) {
                rtk_pwm_backlight_apply(RTK_PWM_0);
            }
        }

        // (GPIO12): E9 -> RTD287O_BGA_22x22_PIN_E9_PWM_2 -> Sync PWM (PWM Dimmer 2) -> Frequency Manipulated PWM -> PIN_PWM2_DIMMER
        rtkPwmApply = 0;
        rtkPwmVal = fmPwmRatio/4;
        rtkPwmVal = rtkPwmVal > const_pwm_panelmax ? const_pwm_panelmax : rtkPwmVal;
		BL_DBC_Print(print_count,"GPIO12 SetPwmDuty :%d\n",rtkPwmVal);
        if(rtkPwmVal != RTK_PWM_2_Duty) {
            rtk_pwm_backlight_set_duty_ex(RTK_PWM_2, rtkPwmVal);
            RTK_PWM_2_Duty = rtkPwmVal;
            rtkPwmApply |= 1;
        }

        if (isNonISR) {
            rtkPwmVal = Vsync*fmPwmPulse;
            BL_DBC_Print3(print_count,"GPIO12 SetPwmFrequency: %d Hz\n",rtkPwmVal);
            if(rtkPwmVal != RTK_PWM_2_Freq) {
                rtk_pwm_backlight_set_freq_ex(RTK_PWM_2, rtkPwmVal);
                RTK_PWM_2_Freq = rtkPwmVal;
                rtkPwmApply |= 1;
            }

            BL_DBC_Print3(print_count,"GPIO12 SetPwmPhase: %d\n",fmPwmPhase);
            if(fmPwmPhase != RTK_PWM_2_Delay) {
                rtk_pwm_backlight_set_vsync_delay_ex(RTK_PWM_2,PWM_POS_DELAY,fmPwmPhase);
                RTK_PWM_2_Delay = fmPwmPhase;
                rtkPwmApply |= 1;
            }

            BL_DBC_Print3(print_count,"GPIO12 PWM SetVsyncAlign => enable vsync\n");
            if(fmPwmPhase != RTK_PWM_2_Vsync) {
                rtk_pwm_backlight_set_vsync_ex(RTK_PWM_2,1);
                RTK_PWM_2_Vsync = fmPwmPhase;
                rtkPwmApply |= 1;
            }
        }
        if(rtkPwmApply == 1) {
            rtk_pwm_backlight_apply(RTK_PWM_2);
        }
        #endif
    }

    BL_DBC_Print(print_count,"====================================\n");
}

int Scaler_Update_BL_DBC_Param_TV002(unsigned char cmd, unsigned int length, void *pData)
{
	BL_DBC_ALL_PARAM_T *p = &g_dbc_param;
	if (cmd >= BL_CTRL_ITEM_MAX)
		return -1;

	switch(cmd) {
	case BL_PWM_PARAM_SET:
		if (length != sizeof(BL_PWM_PARAM_SET_T))
			return -1;
		p->pwm_param = *(BL_PWM_PARAM_SET_T *)pData;
		g_dbc_init.bBL_PWM_PARAM_SET = TRUE;
	break;
	case BL_CURRENT_PARAM_SET:
		if (length != sizeof(BL_CURRENT_PARAM_SET_T))
			return -1;
		p->cur_param = *(BL_CURRENT_PARAM_SET_T *)pData;
		g_dbc_init.bBL_CURRENT_PARAM_SET = TRUE;
	break;
	case BL_BLINK_PARAM_SET:
		if (length != sizeof(BL_BLINK_PARAM_SET_T))
			return -1;
		p->blink_param = *(BL_BLINK_PARAM_SET_T *)pData;
		g_dbc_init.bBL_BLINK_PARAM_SET = TRUE;
	break;
	case BL_PWM_FLAT_PARAM_SET:
		if (length != sizeof(BL_PWM_FLAT_PARAM_SET_T))
			return -1;
		p->pwm_flat_param = *(BL_PWM_FLAT_PARAM_SET_T *)pData;
		g_dbc_init.bBL_PWM_FLAT_PARAM_SET = TRUE;
	break;
	case BL_CONFIG_SET:
		if (length != sizeof(BL_CONFIG_SET_T))
			return -1;
		p->config = *(BL_CONFIG_SET_T *)pData;
		g_dbc_init.bBL_CONFIG_SET = TRUE;
	break;
	case BL_DBC_CONFIG_SET:
		if (length != sizeof(BL_DBC_CONFIG_SET_T))
			return -1;
		p->dbc_config = *(BL_DBC_CONFIG_SET_T *)pData;
		g_dbc_init.bBL_DBC_CONFIG_SET = TRUE;
	break;
	case BL_DBC_STOP_SET:
		if (length != sizeof(unsigned int))
			return -1;
		g_dbc_stop_flag = *(unsigned int *)pData;
	break;
	}

	return 0;
}

#ifdef CONFIG_CUSTOMER_TV002
unsigned char g_SoPQLib_ShareMem[SOPQ_LIB_SHARE_MEM_SIZE] = {0};
unsigned char *Scaler_Get_SoPQLib_ShareMem(void)
{
	return g_SoPQLib_ShareMem;
}

unsigned int Scaler_Get_SoPQLib_ShareMem_Size(void)
{
	return sizeof(g_SoPQLib_ShareMem);
}
#endif

bool g_TV002_Dynamic_SNR_Weight_En = true;
int g_TV002_Dynamic_SNR_Weight = 128;
void fwif_color_nr_curvemapping_weight_set(unsigned char table, unsigned char level)
{
	int i;
	unsigned short w1[9];
	SLR_VIP_TABLE *gVip_Table = NULL;
	DRV_NR_Item *pNR_Item;
	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintfPrivate(VIP_LOGLEVEL_ERR, VIP_LOGMODULE_VIP_PQA_interface1_DEBUG, "~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	pNR_Item = &gVip_Table->Manual_NR_Table[table][level];
	w1[0] = 16-pNR_Item->S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_0;
	w1[1] = 16-pNR_Item->S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_1;
	w1[2] = 16-pNR_Item->S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_2;
	w1[3] = 16-pNR_Item->S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_3;
	w1[4] = 16-pNR_Item->S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_4;
	w1[5] = 16-pNR_Item->S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_5;
	w1[6] = 16-pNR_Item->S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_6;
	w1[7] = 16-pNR_Item->S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_7;
	w1[8] = 16-pNR_Item->S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_8;

	for (i = 0; i < 9; i++) {
		w1[i] = (w1[i]*g_TV002_Dynamic_SNR_Weight)>>7;
		if (w1[i] > 16)
			w1[i] = 16;

		w1[i] = 16 - w1[i];
	}

	drvif_color_nr_curvemapping_weight1_set(w1);
}


bool g_TV002_Dynamic_Shp_Gain_En = true;
bool g_TV002_Dynamic_Shp_Gain_By_Y_En = true;
int g_TV002_Dynamic_Shp_Gain = 128;
int g_TV002_Dynamic_Shp_Gain_By_Y = 128;
short fwif_color_set_sharpness_level_dynamic_calc(short max_value, short mid_value, short min_value, unsigned char sharp_max, unsigned char sharp_mid, unsigned char sharp, unsigned char level, short limit)
{
	short data;

	if (mid_value == 0 || sharp_mid == 0) { //old method
		data = min_value+(max_value - min_value)*level/100;
	} else { //new method
		if (sharp <= sharp_mid)
			data = min_value+(mid_value - min_value)*sharp/sharp_mid;
		else
			data = mid_value+(max_value - mid_value)*(sharp-sharp_mid)/(sharp_max-sharp_mid);
	}

	data = (g_TV002_Dynamic_Shp_Gain*data)>>7;

	if (data > limit)
		data = limit;

	return data;
}

void fwif_color_set_sharpness_level_only_gain(unsigned char table, unsigned char value)
{
	unsigned char sharp = 0, level = 0, sharp_max = 255, sharp_mid = 0;
	//signed short data = 0;
	signed short max_value = 0, min_value = 0, mid_value = 0;
	DRV_Sharpness_Level sharp_gain;
	_system_setting_info *VIP_system_info_structure_table = NULL;

	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintfPrivate(VIP_LOGLEVEL_ERR, VIP_LOGMODULE_VIP_PQA_interface1_DEBUG, "~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		//VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	VIP_system_info_structure_table->OSD_Info.OSD_Sharpness = value;

	if (table >= Sharp_table_num)
		table = 0;

	sharp = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_SHARPNESS, value);
	level = (sharp*100) >> 8;

	sharp_mid = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_SHARPNESS, gVip_Table->Ddomain_SHPTable[table].TwoD_Peak_AdaptCtrl.OSD_Sharpness);
	if (gVip_Table->Ddomain_SHPTable[table].TwoD_Peak_AdaptCtrl.OSD_Sharpness == 0 || sharp_mid == 255)
		sharp_mid = 0; // sharp_mid = 0 will skip mid value calc ,prevent error

/*cal edge_shp*/
	max_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.G_Pos_Max;
	min_value = 0; //gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.G_Pos_Min;
	mid_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.G_Pos_Mid;
	sharp_gain.Edge_Level.G_Pos_Gain = fwif_color_set_sharpness_level_dynamic_calc(max_value, mid_value, min_value, sharp_max, sharp_mid, sharp, level, 255);

	max_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.G_Neg_Max;
	min_value = 0; //gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.G_Neg_Min;
	mid_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.G_Neg_Mid;
	sharp_gain.Edge_Level.G_Neg_Gain = fwif_color_set_sharpness_level_dynamic_calc(max_value, mid_value, min_value, sharp_max, sharp_mid, sharp, level, 255);

/*cal Texture_shp*/
	max_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.G_Pos_Max;
	min_value = 0; //gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.G_Pos_Min;
	mid_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.G_Pos_Mid;
	sharp_gain.Texture_Level.G_Pos_Gain = fwif_color_set_sharpness_level_dynamic_calc(max_value, mid_value, min_value, sharp_max, sharp_mid, sharp, level, 255);

	max_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.G_Neg_Max;
	min_value = 0; //gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.G_Neg_Min;
	mid_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.G_Neg_Mid;
	sharp_gain.Texture_Level.G_Neg_Gain = fwif_color_set_sharpness_level_dynamic_calc(max_value, mid_value, min_value, sharp_max, sharp_mid, sharp, level, 255);

/*cal vertical*/
	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical.G_Pos_Max;
	min_value = 0; //gVip_Table->Ddomain_SHPTable[table].Vertical.G_Pos_Min;
	mid_value = gVip_Table->Ddomain_SHPTable[table].Vertical.G_Pos_Mid;
	sharp_gain.Vertical_Level.G_Pos_Gain = fwif_color_set_sharpness_level_dynamic_calc(max_value, mid_value, min_value, sharp_max, sharp_mid, sharp, level, 255);

	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical.G_Neg_Max;
	min_value = 0; //gVip_Table->Ddomain_SHPTable[table].Vertical.G_Neg_Min;
	mid_value = gVip_Table->Ddomain_SHPTable[table].Vertical.G_Neg_Mid;
	sharp_gain.Vertical_Level.G_Neg_Gain = fwif_color_set_sharpness_level_dynamic_calc(max_value, mid_value, min_value, sharp_max, sharp_mid, sharp, level, 255);

	/*cal vertical Edge  */

	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.G_Pos_Max;
	min_value = 0; //gVip_Table->Ddomain_SHPTable[table].Vertical_edg.G_Pos_Min;
	mid_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.G_Pos_Mid;
	sharp_gain.Vertical_Edge_Level.G_Pos_Gain = fwif_color_set_sharpness_level_dynamic_calc(max_value, mid_value, min_value, sharp_max, sharp_mid, sharp, level, 255);

	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.G_Neg_Max;
	min_value = 0; //gVip_Table->Ddomain_SHPTable[table].Vertical_edg.G_Neg_Min;
	mid_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.G_Neg_Mid;
	sharp_gain.Vertical_Edge_Level.G_Neg_Gain = fwif_color_set_sharpness_level_dynamic_calc(max_value, mid_value, min_value, sharp_max, sharp_mid, sharp, level, 255);

	drvif_color_set_Sharpness_level_only_gain(&sharp_gain);

}

void fwif_color_Set_Sharpness_gain_by_y_only_gain(unsigned char table)
{
	unsigned short gain[7];
	int i;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintfPrivate(VIP_LOGLEVEL_ERR, VIP_LOGMODULE_VIP_PQA_interface1_DEBUG, "~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	gain[0] = (gVip_Table->Ddomain_SHPTable[table].Gain_By_Y.Gain_0*g_TV002_Dynamic_Shp_Gain_By_Y)>>7;
	gain[1] = (gVip_Table->Ddomain_SHPTable[table].Gain_By_Y.Gain_1*g_TV002_Dynamic_Shp_Gain_By_Y)>>7;
	gain[2] = (gVip_Table->Ddomain_SHPTable[table].Gain_By_Y.Gain_2*g_TV002_Dynamic_Shp_Gain_By_Y)>>7;
	gain[3] = (gVip_Table->Ddomain_SHPTable[table].Gain_By_Y.Gain_3*g_TV002_Dynamic_Shp_Gain_By_Y)>>7;
	gain[4] = (gVip_Table->Ddomain_SHPTable[table].Gain_By_Y.Gain_4*g_TV002_Dynamic_Shp_Gain_By_Y)>>7;
	gain[5] = (gVip_Table->Ddomain_SHPTable[table].Gain_By_Y.Gain_5*g_TV002_Dynamic_Shp_Gain_By_Y)>>7;
	gain[6] = (gVip_Table->Ddomain_SHPTable[table].Gain_By_Y.Gain_6*g_TV002_Dynamic_Shp_Gain_By_Y)>>7;

	for (i = 0; i < 7; i++) {
		if (gain[i] > 2047)
			gain[i] = 2047;
	}

	drvif_color_Set_Sharpness_gain_by_y_only_gain(gain);
}

void Scaler_SetSharpness_Only_Gain(unsigned short value)
{
	extern bool g_TV002_Dynamic_Shp_Gain_En;
	extern int g_TV002_Dynamic_Shp_Gain;
	if (!g_TV002_Dynamic_Shp_Gain_En)
		return;
	g_TV002_Dynamic_Shp_Gain = value;
	fwif_color_set_sharpness_level_only_gain(Scaler_GetSharpnessTable(), Scaler_GetSharpness());
}

void Scaler_SetSharpness_Gain_By_Y_Only_Gain(unsigned short value)
{
	extern bool g_TV002_Dynamic_Shp_Gain_By_Y_En;
	extern int g_TV002_Dynamic_Shp_Gain_By_Y;
	if (!g_TV002_Dynamic_Shp_Gain_By_Y_En)
		return;
	g_TV002_Dynamic_Shp_Gain_By_Y = value;
	fwif_color_Set_Sharpness_gain_by_y_only_gain(Scaler_GetSharpnessTable());
}

void Scaler_Set_SNR_CurveMaping_Weight(unsigned short value)
{
	extern bool g_TV002_Dynamic_SNR_Weight_En;
	extern int g_TV002_Dynamic_SNR_Weight;
	if (!g_TV002_Dynamic_SNR_Weight_En)
		return;
	g_TV002_Dynamic_SNR_Weight = value;
	fwif_color_nr_curvemapping_weight_set(Scaler_GetDNR_table(), Scaler_GetDNR());
}

unsigned char Scaler_get_gamma_decoded_TV002(unsigned short *In_R, unsigned short *In_G, unsigned short *In_B)
{
	extern unsigned short Power1div22Gamma[1025];

	memcpy((unsigned char *)In_R, (unsigned char *)&Power1div22Gamma[0], sizeof(short)*1025);
	memcpy((unsigned char *)In_G, (unsigned char *)&Power1div22Gamma[0], sizeof(short)*1025);
	memcpy((unsigned char *)In_B, (unsigned char *)&Power1div22Gamma[0], sizeof(short)*1025);

	return 0;
}

unsigned char g_Freq_Det_Level_TV002 = 0;
void Scaler_Set_Freq_Det_Out_TV002(unsigned char Freq_Det_level)
{
	g_Freq_Det_Level_TV002 = Freq_Det_level;
}

unsigned char Scaler_Get_Freq_Det_Out_TV002(void)
{
	return g_Freq_Det_Level_TV002;
}
