//#include <string.h>

//#include <stdio.h>
//#include <stdlib.h>

//#include <rtio.h>
//#include <VP_def.h>

//#include <auto_conf.h> // Provides: CONFIG_VBI_CC
//#include <rtd_system.h>
//#include <rtd_memmap.h>
#include <memc_isr/scalerMEMC.h>
#include "vgip_isr/scalerVIP.h"
#include <linux/mutex.h>
//#include <mach/io.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <mach/platform.h>

// oliver+
//#include <kernel/scaler/scalerDrv2.h>
//#include <scaler/kernel/scaler/scalerDrv.h>
//#include <scaler/scalerDrvCommon.h>
//#include <scaler/scalerCommon.h>
#include "memc_isr/include/PQLAPI.h"
#include <memc_isr/include/memc_lib.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/mc_dma_reg.h>
#include <rbus/dbus_wrapper_reg.h>
#include "memc_reg_def.h"
//#include "rbus/crt_sys_reg.h"
//#include "rbus/crt_fw.h"
//#endif

#include "tvscalercontrol/vip/vip_reg_def.h"

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
	#include <scalercommon/scalerDrvCommon.h>
#endif

#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <rbus/sb2_reg.h> // for hardware semaphore

//#include "memc_isr/Driver/KI7436_reg.h"
//#include "memc_isr/Driver/KI7576_reg.h"
#include "memc_isr/Common/kw_debug.h" // for LogPrintf
#include "memc_isr/PQL/PQLPlatformDefs.h"
#include <tvscalercontrol/panel/panelapi.h>
#include <linux/version.h> 
#include "memc_isr/Platform/memc_change_size.h"


//#include "scaler_vpqmemcdev.h"

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

#define MEMC_IRQ_HANDLED 1
#define MEMC_IRQ_NONE       0

static unsigned char scaler_memc_enable = 0;
#ifndef MEMC_INIT_STATUS
static unsigned char g_scaler_memc_acon_init = 0;//for ac off->on: dma enable delay 2 vsync than slavemode set, clear when dc suspend
#else
static SCALER_MEMC_INIT_STATUS g_scaler_memc_init_status = MEMC_INIT_NONE;
#endif

#ifdef CONFIG_MEMC_TASK_QUEUE
#define MEMC_TASK_MAX_CNT 17 //Total element count of memc task queue
static SCALER_MEMC_TASK_T g_memc_tasks_queue[MEMC_TASK_MAX_CNT];
static unsigned int g_memc_tasks_header = 0;
static unsigned int g_memc_tasks_tailer = 0;
#endif

unsigned int g_ucMemcDelay;

extern unsigned char MEMC_BRING_UP_InitDone_flag;
extern unsigned int HDMI_PowerSaving_stage;
extern int g_memc_hdmi_switch_state;
extern int g_memc_switch_state;
extern unsigned int DTV_PowerSaving_stage;
extern VOID Mid_MISC_SetInINTEnable(INT_TYPE enIntType, BOOL bEnable);
extern VOID Mid_MISC_SetOutINTEnable(INT_TYPE enIntType, BOOL bEnable);
extern unsigned int Get_DISPLAY_REFRESH_RATE(void);
extern unsigned char MEMC_Lib_get_memc_PowerSaving_Mode(VOID);
extern unsigned char get_MEMC_bypass_status_refer_platform_model(void);
extern unsigned char Scaler_MEMC_GetPanelSizeByDisp(void);


VOID MEMC_ISR_test(void);

//==================================================================================================

void memc_suspend(void)
{
	Scaler_MEMC_SetInitFlag(0);//clear when dc suspend
	Scaler_MEMC_resetMEMCMode();

}

void Scaler_MEMC_SetInitFlag(unsigned char val)
{
#ifndef MEMC_INIT_STATUS
	if(val>1)
		g_scaler_memc_acon_init = 0;
	else
		g_scaler_memc_acon_init = val;
#else
	g_scaler_memc_init_status = val;
#endif

}

unsigned char Scaler_MEMC_GetInitFlag(void)
{
#ifndef MEMC_INIT_STATUS

	return g_scaler_memc_acon_init;
#else
	return g_scaler_memc_init_status;
#endif
}

void Scaler_MEMC_DMA_DB_Check(void)
{
	unsigned int u32_RB_val=0;
	
	rtd_pr_memc_info("[MEMC][DMA_DB_Check_1]=[,%x,%x,%x,%x,%x]\n",rtd_inl(MC_DMA_MC_WDMA_DB_CTRL_reg),rtd_inl(MC_DMA_MC_RDMA_DB_CTRL_reg),rtd_inl(PQC_PQDC_MC_PQ_DECMP_DB_CTRL_reg),rtd_inl(PQC_PQDC_MC_PQ_CMP_DB_CTRL_reg),rtd_inl(LBMC_LBMC_EC_reg));
	
	u32_RB_val=IoReg_Read32(MC_DMA_MC_WDMA_DB_CTRL_reg);//MC_WDMA
	if( (u32_RB_val&0x1)==1){
		IoReg_ClearBits(MC_DMA_MC_WDMA_DB_CTRL_reg,_BIT0);
		IoReg_ClearBits(MC_DMA_MC_WDMA_DB_CTRL_reg,_BIT1);
		rtd_pr_memc_info("MC_WDMA 0xb80994F0=%x\n", IoReg_Read32(MC_DMA_MC_WDMA_DB_CTRL_reg));
		//MC_LF_W uplimit_addr
		rtd_pr_memc_info("MC_LF_W up limit 0xb8099458=%x\n", IoReg_Read32(MC_DMA_MC_LF_DMA_WR_Rule_check_up_reg));
		//MC_LF_W downlimit_addr
		rtd_pr_memc_info("MC_LF_W down limit 0xb809945C=%x\n", IoReg_Read32(MC_DMA_MC_LF_DMA_WR_Rule_check_low_reg));
		//MC_HF_W uplimit_addr
		rtd_pr_memc_info("MC_HF_W up limit 0xb80994D0=%x\n", IoReg_Read32(MC_DMA_MC_HF_DMA_WR_Rule_check_up_reg));
		//MC_HF_W downlimit_addr
		rtd_pr_memc_info("MC_HF_W down limit 0xb80994D8=%x\n", IoReg_Read32(MC_DMA_MC_HF_DMA_WR_Rule_check_low_reg));
		
	}
	
	u32_RB_val=IoReg_Read32(MC_DMA_MC_RDMA_DB_CTRL_reg);//MC_RDMA
	if( (u32_RB_val&0x1)==1){
		IoReg_ClearBits(MC_DMA_MC_RDMA_DB_CTRL_reg,_BIT0);
		IoReg_ClearBits(MC_DMA_MC_RDMA_DB_CTRL_reg,_BIT1);
		rtd_pr_memc_info("MC_RDMA_DB 0xb809AEA8=%x\n", IoReg_Read32(MC_DMA_MC_RDMA_DB_CTRL_reg));
		//MC_LF_I_R uplimit_addr
		rtd_pr_memc_info("MC_LF_I_R up limit 0xb809AE14=%x\n", IoReg_Read32(MC_DMA_MC_LF_I_DMA_RD_Rule_check_up_reg));		
		//MC_LF_I_R downlimit_addr
		rtd_pr_memc_info("MC_LF_I_R down limit 0xb809AE18=%x\n", IoReg_Read32(MC_DMA_MC_LF_I_DMA_RD_Rule_check_low_reg));
		//MC_HF_I_R uplimit_addr
		rtd_pr_memc_info("MC_HF_I_R up limit 0xb809AE34=%x\n", IoReg_Read32(MC_DMA_MC_HF_I_DMA_RD_Rule_check_up_reg));
		//MC_HF_I_R downlimit_addr
		rtd_pr_memc_info("MC_HF_I_R down limit 0xb809AE38=%x\n", IoReg_Read32(MC_DMA_MC_HF_I_DMA_RD_Rule_check_low_reg));
		//MC_LF_P_R uplimit_addr
		rtd_pr_memc_info("MC_LF_P_R up limit 0xb809AE54=%x\n", IoReg_Read32(MC_DMA_MC_LF_P_DMA_RD_Rule_check_up_reg));
		//MC_LF_P_R downlimit_addr
		rtd_pr_memc_info("MC_LF_P_R down limit 0xb809AE58=%x\n", IoReg_Read32(MC_DMA_MC_LF_P_DMA_RD_Rule_check_low_reg));
		//MC_HF_P_R uplimit_addr
		rtd_pr_memc_info("MC_HF_P_R up limit 0xb809AE74=%x\n", IoReg_Read32(MC_DMA_MC_HF_P_DMA_RD_Rule_check_up_reg));
		//MC_HF_P_R downlimit_addr
		rtd_pr_memc_info("MC_HF_P_R down limit 0xb809AE78=%x\n", IoReg_Read32(MC_DMA_MC_HF_P_DMA_RD_Rule_check_low_reg));
		
	}
	
	u32_RB_val=IoReg_Read32(PQC_PQDC_MC_PQ_DECMP_DB_CTRL_reg);//DECOMP_DB
	if( (u32_RB_val&0x1)==1){
		rtd_pr_memc_info("DECOMP_DB 0xb8099850=%x\n", IoReg_Read32(PQC_PQDC_MC_PQ_DECMP_DB_CTRL_reg));
		IoReg_ClearBits(PQC_PQDC_MC_PQ_DECMP_DB_CTRL_reg,_BIT0);
		IoReg_ClearBits(PQC_PQDC_MC_PQ_DECMP_DB_CTRL_reg,_BIT1);				
	}

	u32_RB_val=IoReg_Read32(PQC_PQDC_MC_PQ_CMP_DB_CTRL_reg);//COMP_DB
	if( (u32_RB_val&0x1)==1){
		rtd_pr_memc_info("COMP_DB 0xb8099268=%x\n", IoReg_Read32(PQC_PQDC_MC_PQ_CMP_DB_CTRL_reg));
		IoReg_ClearBits(PQC_PQDC_MC_PQ_CMP_DB_CTRL_reg,_BIT0);
		IoReg_ClearBits(PQC_PQDC_MC_PQ_CMP_DB_CTRL_reg,_BIT1);				
	}

	u32_RB_val=IoReg_Read32(LBMC_LBMC_EC_reg);//LBMC_DB
	if( (u32_RB_val&0x1)==1){
		rtd_pr_memc_info("LBMC_DB 0xb80999ec=%x\n", IoReg_Read32(LBMC_LBMC_EC_reg));
		IoReg_ClearBits(LBMC_LBMC_EC_reg,_BIT0);
		IoReg_ClearBits(LBMC_LBMC_EC_reg,_BIT1);
		rtd_pr_memc_info("PC_mode_check 0xb8099924=%x\n", IoReg_Read32(LBMC_LBMC_24_reg));
		
	}

	IoReg_SetBits(MC_DMA_MC_WDMA_DB_CTRL_reg,_BIT1);
	IoReg_SetBits(MC_DMA_MC_RDMA_DB_CTRL_reg,_BIT1);
	IoReg_SetBits(PQC_PQDC_MC_PQ_DECMP_DB_CTRL_reg,_BIT1);
	IoReg_SetBits(PQC_PQDC_MC_PQ_CMP_DB_CTRL_reg,_BIT1);
	IoReg_SetBits(LBMC_LBMC_EC_reg,_BIT1);
	rtd_pr_memc_info("[MEMC][DMA_DB_Check_2]=[,%x,%x,%x,%x,%x]\n",rtd_inl(MC_DMA_MC_WDMA_DB_CTRL_reg),rtd_inl(MC_DMA_MC_RDMA_DB_CTRL_reg),rtd_inl(PQC_PQDC_MC_PQ_DECMP_DB_CTRL_reg),rtd_inl(PQC_PQDC_MC_PQ_CMP_DB_CTRL_reg),rtd_inl(LBMC_LBMC_EC_reg));
	
}

void Scaler_MEMC_Set_MC_MEMBoundary(unsigned int nDownlimit,unsigned int nUplimit)
{
	unsigned int u32_temp_boundary = 0;
	unsigned int u32_MC_Boundary = MEMC_Lib_Get_MC_Boundary(); // mc dma boundary is generated by memc_change_size

	// check the up and down limit of mc boundary
	if(nDownlimit>=nUplimit){
		rtd_pr_memc_emerg("[%s][%d] MC up/down DMA limit error. DownLimit:%x, UpLimit:%x\n", __func__, __LINE__, nDownlimit, nUplimit);
		nDownlimit = 0;
		nUplimit = 0xffffffff;
	}
	else {
		if(u32_MC_Boundary!=0) { // MEMC_MC_DMA_BOUNDARY is generated by memc_change_size
			if(nUplimit-nDownlimit<u32_MC_Boundary){
				rtd_pr_memc_emerg("[%s][%d] abnormal MC DMA boundary. DownLimit:%x, UpLimit:%x, boundary range:%x\n", __func__, __LINE__, nDownlimit, nUplimit, u32_MC_Boundary);

				// protect boundary
				if((0xffffffff-u32_MC_Boundary)<nDownlimit) {
					nDownlimit = 0;
					nUplimit = 0xffffffff;
				}
				else {
					nUplimit = u32_MC_Boundary+nDownlimit;
				}
			}
		}
		else { // MEMC_MC_DMA_BOUNDARY==0
			rtd_pr_memc_emerg("[%s][%d] abnormal MEMC_MC_DMA_BOUNDARY(0)\n", __func__, __LINE__);

			#if CONFIG_MC_8_BUFFER
			if(Get_DISPLAY_REFRESH_RATE() <= 60){
				u32_temp_boundary = KMC_TOTAL_SIZE_8buf_8bit;
			}else{
				u32_temp_boundary = KMC_TOTAL_SIZE_8buf_10bit;
			}
			#else
			u32_temp_boundary = KMC_TOTAL_SIZE_6buf_10bit;
			#endif

			// protect boundary
			if(u32_temp_boundary==0 || (0xffffffff-u32_temp_boundary)<nDownlimit) {
				nDownlimit = 0;
				nUplimit = 0xffffffff;
			}
			else {
				nUplimit = u32_temp_boundary+nDownlimit;
			}
		}
	}

	IoReg_Write32(MC_DMA_MC_LF_DMA_WR_Rule_check_up_reg, nUplimit);
	IoReg_Write32(MC_DMA_MC_LF_DMA_WR_Rule_check_low_reg, nDownlimit);

	IoReg_Write32(MC_DMA_MC_HF_DMA_WR_Rule_check_up_reg, nUplimit);
	IoReg_Write32(MC_DMA_MC_HF_DMA_WR_Rule_check_low_reg, nDownlimit);

	IoReg_Write32(MC_DMA_MC_LF_I_DMA_RD_Rule_check_up_reg, nUplimit);
	IoReg_Write32(MC_DMA_MC_LF_I_DMA_RD_Rule_check_low_reg, nDownlimit);

	IoReg_Write32(MC_DMA_MC_HF_I_DMA_RD_Rule_check_up_reg, nUplimit);
	IoReg_Write32(MC_DMA_MC_HF_I_DMA_RD_Rule_check_low_reg, nDownlimit);

	IoReg_Write32(MC_DMA_MC_LF_P_DMA_RD_Rule_check_up_reg, nUplimit);
	IoReg_Write32(MC_DMA_MC_LF_P_DMA_RD_Rule_check_low_reg, nDownlimit);

	IoReg_Write32(MC_DMA_MC_HF_P_DMA_RD_Rule_check_up_reg, nUplimit);
	IoReg_Write32(MC_DMA_MC_HF_P_DMA_RD_Rule_check_low_reg, nDownlimit);
}

void Scaler_MEMC_Set_ME_MEMBoundary(unsigned int  nDownlimit,unsigned int nUplimit)
{
	IoReg_Write32(DBUS_WRAPPER_Me_downlimit_addr_reg, nDownlimit);
	IoReg_Write32(DBUS_WRAPPER_Me_uplimit_addr_reg, nUplimit);
}	

unsigned int Scaler_MEMC_Get_Size_Addr(unsigned long *memc_addr_aligned, unsigned long *memc_addr)
{
	unsigned int memc_size = 0;

#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	memc_size = (unsigned int)carvedout_buf_query_secure(CARVEDOUT_SCALER_MEMC, (void **)memc_addr);
#else
	memc_size = (unsigned int)carvedout_buf_query(CARVEDOUT_SCALER_MEMC, (void *)memc_addr);
#endif
	#ifdef CONFIG_BW_96B_ALIGNED
	#if CONFIG_MC_8_BUFFER
	if(Get_DISPLAY_REFRESH_RATE() <= 60){
		*memc_addr_aligned = dvr_memory_alignment((unsigned long)*memc_addr, KMEMC_TOTAL_SIZE/*KMEMC_TOTAL_SIZE_8buf_8bit*/);
	}else{
		*memc_addr_aligned = dvr_memory_alignment((unsigned long)*memc_addr, KMEMC_TOTAL_SIZE_8buf_10bit);
	}
	#else
	*memc_addr_aligned = dvr_memory_alignment((unsigned long)*memc_addr, KMEMC_TOTAL_SIZE_6buf_10bit);
	#endif
	if (*memc_addr == 0 || memc_size == 0  || *memc_addr_aligned==0) {
		rtd_pr_memc_emerg("[%s %d] ERR : %lx %x %lx\n", __func__, __LINE__, *memc_addr, memc_size,*memc_addr_aligned);
		return 0;
	}
	#else
	if (*memc_addr == 0 || memc_size == 0) {
		rtd_pr_memc_emerg("[%s %d] ERR : %lx %x\n", __func__, __LINE__, *memc_addr, memc_size);
		return 0;
	}
	#endif

	return memc_size;
}

void Scaler_MEMC_Initialize(unsigned int ME_start_addr, unsigned int MC00_start_addr, unsigned int MC01_start_addr)
{
#if 0 //def MEMC_BRING_UP
	rtd_pr_memc_notice("[MEMC_BRING_UP][Scaler_MEMC_Initialize]\n");

//	MEMC_Lib_LoadBringUpScript_HDF();
	rtd_pr_memc_notice("[MEMC_BRING_UP][MEMC_Lib_LoadBringUpScript_HDF]\n");

//	MEMC_Lib_LoadBringUpScript_PhaseTable();
	rtd_pr_memc_notice("[MEMC_BRING_UP][MEMC_Lib_LoadBringUpScript_PhaseTable]\n");

//	MEMC_Lib_LoadBringUpScript_MemoryAddress();
	rtd_pr_memc_notice("[MEMC_BRING_UP][MEMC_Lib_LoadBringUpScript_MemoryAddress]\n");

//	MEMC_Lib_LoadBringUpScript_cs_2x2_all_on();
	rtd_pr_memc_notice("[MEMC_BRING_UP][MEMC_Lib_LoadBringUpScript_cs_2x2_all_on]\n");

//	MEMC_Lib_LoadBringUpScript_Enable_DMA();
	rtd_pr_memc_notice("[MEMC_BRING_UP][MEMC_Lib_LoadBringUpScript_Enable_DMA]\n");
#else
	unsigned int Hsync_start = 0, Vsync_start = 0;
	PANEL_PARA_ST stPanelPara;
	unsigned short u16Idx = 0;
#if CONFIG_MEMC_BOUNDARY_AUTO
	unsigned int memc_size = 0;
	unsigned long memc_addr = 0;
	unsigned long memc_addr_aligned = 0;
#endif
	rtd_pr_memc_info("[MEMC][%s]\n",__FUNCTION__);
	rtd_pr_memc_info("[MEMC][%s]\n",__FUNCTION__);
	rtd_pr_memc_info("[MEMC]ME_start_addr = %x\n", ME_start_addr);
	rtd_pr_memc_info("[MEMC]MC00_start_addr = %x\n", MC00_start_addr);
	rtd_pr_memc_info("[MEMC]MC01_start_addr = %x\n", MC01_start_addr);
	rtd_pr_memc_info("[MEMC]KME_TOTAL_SIZE = %x\n", KME_TOTAL_SIZE);
	rtd_pr_memc_info("[MEMC]KMC_TOTAL_SIZE = %x\n", KMC_TOTAL_SIZE);
	rtd_pr_memc_info("[MEMC][db]=[,%x,%x,%x]\n",rtd_inl(MC_DMA_MC_WDMA_DB_CTRL_reg),rtd_inl(MC_DMA_MC_RDMA_DB_CTRL_reg),rtd_inl(PQC_PQDC_MC_PQ_DECMP_DB_CTRL_reg));
	rtd_pr_memc_info("[MEMC][%x, %x, %x, %x, %x, %x]\n\r", rtd_inl(MC_DMA_MC_LF_DMA_WR_Ctrl_reg), rtd_inl(MC_DMA_MC_HF_DMA_WR_Ctrl_reg), rtd_inl(MC_DMA_MC_LF_I_DMA_RD_Ctrl_reg), rtd_inl(MC_DMA_MC_HF_I_DMA_RD_Ctrl_reg), rtd_inl(MC_DMA_MC_LF_P_DMA_RD_Ctrl_reg), rtd_inl(MC_DMA_MC_HF_P_DMA_RD_Ctrl_reg));	

	Scaler_MEMC_DMA_DB_Check();

#ifndef MEMC_INIT_STATUS
	Scaler_MEMC_SetInitFlag(1);
	//Scaler_MEMC_SetMEMC_Enable(0);
#else
	Scaler_MEMC_SetInitFlag(MEMC_INIT_ACON_ING);
#endif
	MEMC_LibInputVSInterruptEnable(FALSE);
	MEMC_LibOutputVSInterruptEnable(FALSE);
	MEMC_LibInputHSInterruptEnable(FALSE);
	MEMC_LibOutpuHSInterruptEnable(FALSE);	
	Vsync_start = (rtd_inl(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg) & 0xffff0000)>>16;
	Hsync_start = (rtd_inl(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg) & 0xffff0000)>>16;
	rtd_pr_memc_info("[MEMC]Vsync_start = %d\n",Vsync_start);
	rtd_pr_memc_info("[MEMC]Hsync_start = %d\n",Hsync_start);
	MEMC_LibPanel_Init();
	#if (IC_K6LP || IC_K8LP)
	stPanelPara.enOutputFrameRate = OUTPUT_60HZ;
	stPanelPara.u32Htotal = Get_DISP_HORIZONTAL_TOTAL();
	stPanelPara.u32HtotalMax = 4800;
	stPanelPara.u32HtotalMin = 4000;
	stPanelPara.u32Hactive = 3840;
	stPanelPara.u32HSWidth = 280;
	stPanelPara.u32HSBPorch = 168; //Hsync_start - stPanelPara.u32HSWidth;
	stPanelPara.u32Vtotal = 2250;
	stPanelPara.u32VtotalMax = 2500;
	stPanelPara.u32VtotalMin = 2200;
	stPanelPara.u32Vactive = 2160;
	stPanelPara.u32VSWidth = 8;
	stPanelPara.u32VSBPorch = 35; //Vsync_start - stPanelPara.u32VSWidth;
	#else
	stPanelPara.enOutputFrameRate = OUTPUT_60HZ;
	stPanelPara.u32Htotal = Get_DISP_HORIZONTAL_TOTAL();
	stPanelPara.u32HtotalMax = 4800;
	stPanelPara.u32HtotalMin = 4000;
	stPanelPara.u32Hactive = 3840;
	stPanelPara.u32HSWidth = 64;
	stPanelPara.u32HSBPorch = 168;//Hsync_start - stPanelPara.u32HSWidth;
	stPanelPara.u32Vtotal = 2250;
	stPanelPara.u32VtotalMax = 2300;
	stPanelPara.u32VtotalMin = 2200;
	stPanelPara.u32Vactive = 2160;
	stPanelPara.u32VSWidth = 8;
	stPanelPara.u32VSBPorch = Vsync_start - stPanelPara.u32VSWidth;
	#endif	
	//rtd_pr_memc_info("\n DISP_HORIZONTAL_TOTAL = %d \n",Get_DISP_HORIZONTAL_TOTAL());
	for(u16Idx = 0; u16Idx < OUTPUT_FRAME_MAX; u16Idx++){
		stPanelPara.enOutputFrameRate = u16Idx;
		MEMC_LibSetPanelParameters(&stPanelPara);
	}
	MEMC_LibDMAllocate(MC00_start_addr, MC01_start_addr, ME_start_addr);
	//Scaler_MEMC_Set_ME_MEMBoundary(ME_start_addr, (ME_start_addr+KME_TOTAL_SIZE));

#if CONFIG_MEMC_BOUNDARY_AUTO
	//set mc boundary with memc max memory size
	memc_size = Scaler_MEMC_Get_Size_Addr(&memc_addr_aligned, &memc_addr);
	if(memc_size==0){
		rtd_pr_memc_info("Fail to set MC boundary for memc_size = 0.\r\n");
	}
	else {
		Scaler_MEMC_Set_MC_MEMBoundary(memc_addr, memc_addr+memc_size);
	}
#else
	if(Scaler_MEMC_GetPanelSizeByDisp() == MEMC_SIZE_2K){//if(Get_DISPLAY_PANEL_TYPE() == P_LVDS_2K1K){
		Scaler_MEMC_Set_ME_MEMBoundary(ME_start_addr, (ME_start_addr+KME_TOTAL_SIZE/*KME_TOTAL_SIZE_8K*/));
		#if CONFIG_MC_8_BUFFER
		if(Get_DISPLAY_REFRESH_RATE() <= 60){
			Scaler_MEMC_Set_MC_MEMBoundary(MC00_start_addr,(MC00_start_addr + KMC_TOTAL_SIZE_8buf_8bit/*KMC_TOTAL_SIZE_8buf_8bit_8K*/));	
		}else{
			Scaler_MEMC_Set_MC_MEMBoundary(MC00_start_addr,(MC00_start_addr + KMC_TOTAL_SIZE_8buf_10bit/*KMC_TOTAL_SIZE_8buf_10bit_8K*/));	
		}
		#else
		Scaler_MEMC_Set_MC_MEMBoundary(MC00_start_addr,(MC00_start_addr + KMC_TOTAL_SIZE_6buf_10bit/*KMC_TOTAL_SIZE_6buf_10bit_8K*/));
		#endif
	}else{
		Scaler_MEMC_Set_ME_MEMBoundary(ME_start_addr, (ME_start_addr+KME_TOTAL_SIZE));
		#if CONFIG_MC_8_BUFFER
		if(Get_DISPLAY_REFRESH_RATE() <= 60){
			Scaler_MEMC_Set_MC_MEMBoundary(MC00_start_addr,(MC00_start_addr + KMC_TOTAL_SIZE_8buf_8bit));	
		}else{
			Scaler_MEMC_Set_MC_MEMBoundary(MC00_start_addr,(MC00_start_addr + KMC_TOTAL_SIZE_8buf_10bit));	
		}
		#else
		Scaler_MEMC_Set_MC_MEMBoundary(MC00_start_addr,(MC00_start_addr + KMC_TOTAL_SIZE_6buf_10bit));	
		#endif
	}
#endif
	MEMC_LibInit();
	//MEMC_LibSetInOutFrameRate(INPUT_60HZ,OUTPUT_60HZ);
	//MEMC_LibSetInOutMode(MEMC_RGB_IN_VIDEO_OUT,TRUE);
	//MEMC_LibSetInOutUseCase(INPUT_3840X2160, OUTPUT_3840X2160, INPUT_2D, OUTPUT_2D);
	/* Enable in/out Vsync interrupt */
	MEMC_LibInputVSInterruptEnable(TRUE);
	MEMC_LibOutputVSInterruptEnable(TRUE); //YE Test LBMC

	MEMC_LibOutpuHSInterruptEnable(TRUE); //YE Test LBMC
	
	Scaler_MEMC_SetMEMC_Enable(1);
#ifdef MEMC_INIT_STATUS
	Scaler_MEMC_SetInitFlag(MEMC_INIT_ACON_FINISH);
#endif
	rtd_pr_memc_info("MEMC initialize done. IOvs Intp Enable.\r\n");
#endif
}

void Scaler_MEMC_Uninitialize(void){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
}

unsigned char Scaler_MEMC_Dejudder_Level_Mapping(unsigned char inLevel)
{
	unsigned char outLevel = 0;
	unsigned char dejudder_Num = 0;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	if( gVip_Table == NULL ) {
		return 0;
	}

	dejudder_Num = gVip_Table->MEMC_PARAMS.JudderTable[0];
	if(inLevel > (dejudder_Num-1))
		inLevel = dejudder_Num-1;

	outLevel = gVip_Table->MEMC_PARAMS.JudderTable[inLevel+1];
	rtd_pr_memc_info("[MEMC][%s] inLevel = %d, outLevel = %d\n", __FUNCTION__, inLevel, outLevel);
	return outLevel;
}

unsigned char Scaler_MEMC_Deblur_Level_Mapping(unsigned char inLevel)
{
	unsigned char outLevel = 0;
	unsigned char deblur_Num = 0;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	if( gVip_Table == NULL ) {
		return 0;
	}

	deblur_Num = gVip_Table->MEMC_PARAMS.BlurTable[0];
	if(inLevel > (deblur_Num-1))
		inLevel = deblur_Num-1;

	outLevel = gVip_Table->MEMC_PARAMS.BlurTable[inLevel+1];

	rtd_pr_memc_info("[MEMC][%s] inLevel = %d, outLevel = %d\n", __FUNCTION__, inLevel, outLevel);
	return outLevel;
}

extern unsigned char u8_MEMCMode;
extern unsigned int dejudder;
extern unsigned int deblur;
extern unsigned char g_power_saving_flag;
int LowDelay_mode = 0;
int SetMotionComp_return_by_lowdelay = 0;
extern unsigned char VR360_en;
extern unsigned char drv_memory_get_vdec_direct_low_latency_mode(void);
unsigned char g_motion = 0;
MEMC_MODE Scaler_MEMC_MEMCTypeMapping(SCALER_MEMC_TYPE_T motion)
{
	MEMC_MODE ret = 0;

	if(motion == 0/*VPQ_MEMC_TYPE_OFF*/){ // off
		//OFF memc
		ret = MEMC_OFF;
	}
	else if(motion == 1/*VPQ_MEMC_TYPE_LOW*/){ // natural
		//Set low memc
		ret = MEMC_CLEAR;
	}
	else if(motion == 2/*VPQ_MEMC_TYPE_HIGH*/){ // smooth
		//Set high memc
		ret = MEMC_SMOOTH;
	}
	else if(motion == 3/*VPQ_MEMC_TYPE_NORMAL*/){
		//Set high memc
		ret = MEMC_NORMAL;
	}
	else if(motion == 4/*VPQ_MEMC_TYPE_USER*/){ // user
		//Set user memc
		ret = MEMC_USER;
	}
	else if(motion == 5/*VPQ_MEMC_TYPE_55_PULLDOWN*/){
		#if 0 // tv003
		ret = MEMC_NORMAL;
		#else
		ret = MEMC_OFF;
		#endif
	}
	else if(motion == 6/*VPQ_MEMC_TYPE_MEDIUM*/){ // cinema clear
		//Set high memc
		ret = MEMC_MEDIUM;
	}
	else{
		//OFF memc
		ret = MEMC_OFF;
	}

	return ret;
}

void Scaler_MEMC_SetMotionComp(unsigned char blurLevel, unsigned char judderLevel, SCALER_MEMC_TYPE_T motion){
	_RPC_clues* RPC_SmartPic_clue = (_RPC_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);
	unsigned char ME_clock_state = 0, MEMC_clock_state = 0;
	static unsigned char First_time_flag = 1;
	//SLR_VOINFO *pVOInfo = NULL;
	//unsigned short src_in_frame_rate = 0;

	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
	rtd_pr_memc_notice("[MEMC]blurLevel = %d, judderLevel = %d, motion = %d\n",blurLevel, judderLevel, motion);
	//if(MEMC_LibGetMEMC_PCModeEnable() == TRUE){
	//	return;
	//}
	MEMC_clock_state = (rtd_inl(SYS_REG_SYS_CLKEN3_reg)>>31)&0x1;
	ME_clock_state = (rtd_inl(SYS_REG_SYS_CLKEN3_reg)>>30)&0x1;
	if(MEMC_clock_state == 0){
		rtd_pr_memc_notice("[%s][MEMC_clock_state is off !!! return !!!]\n", __FUNCTION__);
		return;
	}

	if(RPC_SmartPic_clue == NULL || (Scaler_MEMC_GetMEMC_Enable() == 0))
		return;
	/*
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	if (pVOInfo == NULL) {
		printk("[%s][WARNING] VO info is null!!!\n",__FUNCTION__);
		return;
	}
	src_in_frame_rate = pVOInfo->src_v_freq;
	*/

	g_motion = motion;
	rtd_pr_memc_notice("[MEMC]LowDelay_mode = %d, vdec_direct = %d, motion = %d\n",LowDelay_mode, drv_memory_get_vdec_direct_low_latency_mode(), MEMC_LibGetMEMCLowDelayModeEnable());
	if(drv_memory_get_vdec_direct_low_latency_mode()== TRUE){/*MEMC_LibGetMEMCLowDelayModeEnable() == TRUE*/ /*&& RPC_SmartPic_clue->MEMC_VCPU_setting_info.m_MiraCast == 1*/
		SetMotionComp_return_by_lowdelay = 1;		
		rtd_pr_memc_notice("[MEMC]LowDelay = %d, MiraCast = %d\n", MEMC_LibGetMEMCLowDelayModeEnable(), RPC_SmartPic_clue->MEMC_VCPU_setting_info.m_MiraCast);
		return;
	}

	//re-mapping the MEMC_TYPE from VPQ_MEMC_TYPE_T to SCALER_MEMC_TYPE_T
	if(motion == 0/*VPQ_MEMC_TYPE_OFF*/){         // off
		motion = MEMC_TYPE_OFF;
	}
	else if(motion == 1/*VPQ_MEMC_TYPE_LOW*/){ // natural
		motion = MEMC_TYPE_LOW;
	}
	else if(motion == 2/*VPQ_MEMC_TYPE_HIGH*/){ // smooth
		motion = MEMC_TYPE_HIGH;
	}	
	else if(motion == 3/*VPQ_MEMC_TYPE_NORMAL*/){
		motion = MEMC_TYPE_NORMAL;
	}
	else if(motion == 4/*VPQ_MEMC_TYPE_USER*/){ // user
		motion = MEMC_TYPE_USER;
	}
	else if(motion == 5/*VPQ_MEMC_TYPE_55_PULLDOWN*/){
		motion = MEMC_TYPE_55_PULLDOWN;
	}
	else if(motion == 6/*VPQ_MEMC_TYPE_MEDIUM*/){  // cinema clear
		motion = MEMC_TYPE_MEDIUM;
	}
	else if(motion == 3/*VPQ_MEMC_TYPE_NORMAL*/){
		motion = MEMC_TYPE_NORMAL;
	}
	else{
		motion = MEMC_TYPE_OFF;
	}

#if 0 // def MEMC_BRING_UP
	motion = MEMC_TYPE_HIGH;
#endif

	if(motion>=MEMC_TYPE_MAX)		//fix me
		motion = MEMC_TYPE_OFF;

	u8_MEMCMode = motion;
	if(motion == MEMC_TYPE_OFF){
		//OFF memc
		MEMC_LibSetMEMCMode(MEMC_OFF);

		//if(RPC_SmartPic_clue->prePowerSaveStatus == PS_ON)
		if(ME_clock_state == 0)
		{
			rtd_pr_memc_notice("[RepeatMode] Power saving mode on, return.\n");
		}
		else
		{
			if(First_time_flag || g_power_saving_flag)
				MEMC_LibSetMEMCFrameRepeatEnable(1, 1);
			else
				MEMC_LibSetMEMCFrameRepeatEnable(1, 0);
		}
	}
	else if(motion == MEMC_TYPE_LOW){
		//Set low memc
		MEMC_LibSetMEMCMode(MEMC_CLEAR);

		//if(RPC_SmartPic_clue->prePowerSaveStatus == PS_ON)
		if(ME_clock_state == 0)
		{
			rtd_pr_memc_notice("[RepeatMode] Power saving mode on, return.\n");
		}
		else
		{
			if(First_time_flag || g_power_saving_flag)
				MEMC_LibSetMEMCFrameRepeatEnable(0, 1);
			else
				MEMC_LibSetMEMCFrameRepeatEnable(0, 0);
		}
	}
	else if(motion == MEMC_TYPE_HIGH){
		//Set high memc
		MEMC_LibSetMEMCMode(MEMC_SMOOTH);

		//if(RPC_SmartPic_clue->prePowerSaveStatus == PS_ON)
		if(ME_clock_state == 0)
		{
			rtd_pr_memc_notice("[RepeatMode] Power saving mode on, return.\n");
		}
		else
		{
			if(First_time_flag || g_power_saving_flag)
				MEMC_LibSetMEMCFrameRepeatEnable(0, 1);
			else
				MEMC_LibSetMEMCFrameRepeatEnable(0, 0);
		}
	}
	else if(motion == MEMC_TYPE_NORMAL){
		//Set high memc
		MEMC_LibSetMEMCMode(MEMC_NORMAL);

		//if(RPC_SmartPic_clue->prePowerSaveStatus == PS_ON)
		if(ME_clock_state == 0)
		{
			rtd_pr_memc_notice("[RepeatMode] Power saving mode on, return.\n");
		}
		else
		{
			if(First_time_flag || g_power_saving_flag)
				MEMC_LibSetMEMCFrameRepeatEnable(0, 1);
			else
				MEMC_LibSetMEMCFrameRepeatEnable(0, 0);
		}
	}
	else if(motion == MEMC_TYPE_USER){
		//Set user memc
		MEMC_LibSetMEMCMode(MEMC_USER);
		//Set blurLevel for Video
		blurLevel = Scaler_MEMC_Deblur_Level_Mapping(blurLevel);
		deblur = blurLevel;
		MEMC_LibSetMEMCDeblurLevel(blurLevel);
		//Set JudderLevel for Film
		judderLevel = Scaler_MEMC_Dejudder_Level_Mapping(judderLevel);
		dejudder = judderLevel;		
		MEMC_LibSetMEMCDejudderLevel(judderLevel);
		//rtd_pr_memc_notice("[MEMC] judderLevel=%d, blurLevel=%d\n", judderLevel, blurLevel);
		
		//if(RPC_SmartPic_clue->prePowerSaveStatus == PS_ON)
		if(ME_clock_state == 0)
		{
			rtd_pr_memc_notice("[RepeatMode] Power saving mode on, return.\n");
		}
		else
		{
			if(First_time_flag || g_power_saving_flag)
				MEMC_LibSetMEMCFrameRepeatEnable(0, 1);
			else
				MEMC_LibSetMEMCFrameRepeatEnable(0, 0);
		}
	}
	else if(motion == MEMC_TYPE_55_PULLDOWN){
		SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
		unsigned char ModeInCinema = gVip_Table->MEMC_PARAMS.ModeInCinema;
		MEMC_LibSetMEMCMode(ModeInCinema);

		if(ME_clock_state == 0)
		{
			rtd_pr_memc_notice("[RepeatMode] Power saving mode on, return.\n");
		}
		else
		{
			BOOL bRepeat_en = (ModeInCinema==MEMC_OFF) ? 1 : 0;
			if(First_time_flag || g_power_saving_flag)
				MEMC_LibSetMEMCFrameRepeatEnable(bRepeat_en, 1);
			else
				MEMC_LibSetMEMCFrameRepeatEnable(bRepeat_en, 0);
		}
	}
	else if(motion == MEMC_TYPE_MEDIUM){
		//Set high memc
		//if((src_in_frame_rate > 235) && (src_in_frame_rate < 245) /*&& (Get_DISPLAY_REFRESH_RATE() <= 60)*/){
		//	MEMC_LibSetMEMCMode(MEMC_CINEMATIC_MOVEMENT);
		//}else{
			MEMC_LibSetMEMCMode(MEMC_MEDIUM);
		//}

		//if(RPC_SmartPic_clue->prePowerSaveStatus == PS_ON)
		if(ME_clock_state == 0)
		{
			rtd_pr_memc_notice("[RepeatMode] Power saving mode on, return.\n");
		}
		else
		{
			if(First_time_flag || g_power_saving_flag)
				MEMC_LibSetMEMCFrameRepeatEnable(0, 1);
			else
				MEMC_LibSetMEMCFrameRepeatEnable(0, 0);
		}
	}

	if(First_time_flag == 1){
		First_time_flag = 0;
	}
	if(g_power_saving_flag == 1){
		g_power_saving_flag = 0;
	}
}

void Scaler_MEMC_SetBlurLevel(unsigned char blurLevel){

	rtd_pr_memc_notice("[MEMC][%s] blurLevel = %d\n",__FUNCTION__, blurLevel);

	MEMC_LibSetMEMCDeblurLevel(blurLevel);//driver range : 0~128
}

void Scaler_MEMC_SetJudderLevel(unsigned char judderLevel){

	rtd_pr_memc_notice("[MEMC][%s] judderLevel = %d\n",__FUNCTION__, judderLevel);

	MEMC_LibSetMEMCDejudderLevel(judderLevel);//driver range : 0~128
}

void Scaler_MEMC_MotionCompOnOff(unsigned char bOnOff){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
	rtd_pr_memc_notice("[MEMC]bOnOff = %d\n",bOnOff);

	if(bOnOff == TRUE){
		MEMC_LibSetMEMCMode(MEMC_NORMAL);
		MEMC_LibSetMEMCFrameRepeatEnable(0, 0);
	}
	else{
		MEMC_LibSetMEMCMode(MEMC_OFF);
		MEMC_LibSetMEMCFrameRepeatEnable(1, 0);
	}
}

unsigned char Scaler_get_MEMC_LowDelay_disable_condition(void){
	if(/*MEMC_LibGetMEMC_PCModeEnable() ||*/ VR360_en || (Scaler_MEMC_GetMEMC_Enable() == 0) || (Get_DISPLAY_PANEL_TYPE()== P_VBY1_TO_HDMI_MEMC_FREERUN)){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

void Scaler_MEMC_LowDelayMode(unsigned char type){
	static MEMC_MODE last_mode = MEMC_OFF;
	static unsigned char pre_type = 255;
	unsigned char MEMC_clock_state = 0;

	MEMC_clock_state = (rtd_inl(SYS_REG_SYS_CLKEN3_reg)>>31)&0x1;
	if(MEMC_clock_state == 0){
		rtd_pr_memc_notice("[%s][MEMC_clock_state is off !!! return !!!]\n", __FUNCTION__);
		return;
	}

	rtd_pr_memc_notice("[MEMC][%s][AA][,%d,][,%d,%d,%d,%d,][g_motion,%d,]\n",__FUNCTION__, type, MEMC_LibGetMEMCLowDelayModeEnable(), MEMC_LibGetMEMC_PCModeEnable(), VR360_en, Scaler_MEMC_GetMEMC_Enable(), g_motion);
//	rtd_pr_memc_notice("$$$   scalerMEMC_LowDelay_(%d)   $$$\n\r", type);
	LowDelay_mode = type;

	if(type == MEMC_LibGetMEMCLowDelayModeEnable())
		return;

	if(MEMC_LibGetMEMC_PCModeEnable() || VR360_en || (Scaler_MEMC_GetMEMC_Enable() == 0) || (Get_DISPLAY_PANEL_TYPE()== P_VBY1_TO_HDMI_MEMC_FREERUN)){
		LowDelay_mode = 0;
		MEMC_LibSetMEMCLowDelayModeEnable(FALSE);
		return;
	}

	rtd_pr_memc_notice("[MEMC][%s][BB][,%d,][,%d,%d,%d,%d,][,%d,]\n",__FUNCTION__, type, MEMC_LibGetMEMCLowDelayModeEnable(), 
						MEMC_LibGetMEMC_PCModeEnable(), VR360_en, Scaler_MEMC_GetMEMC_Enable(),drv_memory_get_vdec_direct_low_latency_mode());
	//if(VR360_en)
	//	return;

	//rtd_pr_memc_notice("[MEMC]vdec_direct = %d\n", drv_memory_get_vdec_direct_low_latency_mode());

	if(type == 1){
		//Reduce frame buffer for low frame delay
		//if(MEMC_LibGetMEMC_PCModeEnable() == FALSE)	//PC mode first
		//{
		//	MEMC_LibSetMEMCMode(MEMC_OFF);
		//	MEMC_LibSetMEMCFrameRepeatEnable(1, 0);
		//}
		rtd_pr_memc_notice("[MEMC][%s][CC][%d,%d]\n",__FUNCTION__, pre_type,type);
		if(type != pre_type){
			rtd_pr_memc_notice("[MEMC][%s][DD][%d,%d,%d]\n",__FUNCTION__, pre_type,last_mode,MEMC_LibGetMEMCMode());
			last_mode = Scaler_MEMC_MEMCTypeMapping(g_motion); // MEMC_LibGetMEMCMode();
			MEMC_LibSetMEMCMode(MEMC_OFF);
			MEMC_LibSetMEMCFrameRepeatEnable(1, 0);
		}
		MEMC_LibSetMEMCLowDelayModeEnable(TRUE);
	}
	else{
		//Normal mode
		//MEMC_LibSetMEMCMode(MEMC_NORMAL);
		//MEMC_LibSetMEMCFrameRepeatEnable(0, 0);
		rtd_pr_memc_notice("[MEMC][%s][EE][%d,%d,%d]\n",__FUNCTION__,pre_type,last_mode,MEMC_LibGetMEMCMode());
		if(last_mode != Scaler_MEMC_MEMCTypeMapping(g_motion)){ // Q16730
			rtd_pr_memc_notice("[MEMC][%s][last_mode change !!][last_mode,%d,][current mode,%d,]\n",__FUNCTION__, last_mode, Scaler_MEMC_MEMCTypeMapping(u8_MEMCMode));
			last_mode = Scaler_MEMC_MEMCTypeMapping(g_motion);
		}
		if(last_mode != MEMC_OFF){
			rtd_pr_memc_notice("[MEMC][%s][FF][%d,%d]\n",__FUNCTION__,pre_type,last_mode);
			if(VR360_en){
				MEMC_LibSetMEMCMode(MEMC_OFF);
				MEMC_LibSetMEMCFrameRepeatEnable(1, 0);
			}else{
				MEMC_LibSetMEMCMode(last_mode);
				MEMC_LibSetMEMCFrameRepeatEnable(0, 0);
			}
		}
		MEMC_LibSetMEMCLowDelayModeEnable(FALSE);
	}
}

void Scaler_MEMC_SetRGBYUVMode(unsigned char mode){
	sys_reg_sys_srst3_RBUS sys_reg_sys_srst3_reg;
	sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_reg;
	sys_reg_sys_srst3_reg.regValue = IoReg_Read32(SYS_REG_SYS_SRST3_reg);
	sys_reg_sys_clken3_reg.regValue = IoReg_Read32(SYS_REG_SYS_CLKEN3_reg);
	if((sys_reg_sys_srst3_reg.rstn_memc_mc == 0) || (sys_reg_sys_clken3_reg.clken_memc_mc == 0) || (MEMC_Lib_get_memc_PowerSaving_Mode() == 1)){
		return;
	}	
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
	rtd_pr_memc_notice("[MEMC]mode = %d\n",mode);

	switch(mode)
	{
		case 0:
			//MEMC_LibSetInOutMode(MEMC_RGB_IN_VIDEO_OUT, FALSE);
			MEMC_LibSetMEMC_PCModeEnable(FALSE);
			break;
		case 1:
			//MEMC_LibSetInOutMode(MEMC_RGB_IN_PC_OUT, FALSE);
			MEMC_LibSetMEMC_PCModeEnable(TRUE);
			break;
#if 0
		case 2:
			MEMC_LibSetInOutMode(MEMC_YUV_IN_VIDEO_OUT, FALSE);
			break;
		case 3:
			MEMC_LibSetInOutMode(MEMC_YUV_IN_PC_OUT, FALSE);
			break;
#endif
		default:
			if(Get_DISPLAY_PANEL_TYPE() == P_VBY1_TO_HDMI_MEMC_FREERUN){
				MEMC_LibSetInOutMode(MEMC_RGB_IN_VIDEO_FREERUN_OUT, FALSE);
			}else{
				MEMC_LibSetInOutMode(MEMC_RGB_IN_VIDEO_OUT, FALSE);
			}
			MEMC_LibSetMEMC_PCModeEnable(FALSE);
			break;
 	}
}

void Scaler_MEMC_GetFrameDelay (unsigned short *pFrameDelay){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
}
void Scaler_MEMC_SetVideoBlock(SCALER_MEMC_MUTE_TYPE_T type, unsigned char bOnOff){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
}
void Scaler_MEMC_SetTrueMotionDemo(unsigned char bOnOff){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);

	if(bOnOff== TRUE){
		MEMC_LibSetMEMCDemoMode(DEMO_MODE_OFF);
	}
	else{
		MEMC_LibSetMEMCDemoMode(DEMO_MODE_LEFT);
	}

}
void Scaler_MEMC_GetFirmwareVersion(unsigned short *pVersion){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
}
void Scaler_MEMC_SetBypassRegion(unsigned char bOnOff, SCALER_MEMC_BYPASS_REGION_T region, unsigned int x, unsigned int y, unsigned int w, unsigned int h){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);

	rtd_pr_memc_notice("[MEMC]region = %d\n",region);
	rtd_pr_memc_notice("[MEMC]rect.x = %d, rect.y = %d, rect.w = %d, rect.h = %d\n",x, y, w, h);

	rtd_pr_memc_notice("[MEMC]bOnOff = %d\n",bOnOff);

	switch(region){
		case BYPASS_REGION_0:
			MEMC_LibSetDemoWindowSettings(BYPASS_REGION_0, x, w, y, h);
			MEMC_LibSetDemoWindowEnable(BYPASS_REGION_0, bOnOff);
			break;

		case BYPASS_REGION_1:
			MEMC_LibSetDemoWindowSettings(BYPASS_REGION_1, x, w, y, h);
			MEMC_LibSetDemoWindowEnable(BYPASS_REGION_1, bOnOff);
			break;

		case BYPASS_REGION_2:
			MEMC_LibSetDemoWindowSettings(BYPASS_REGION_2, x, w, y, h);
			MEMC_LibSetDemoWindowEnable(BYPASS_REGION_2, bOnOff);
			break;

		case BYPASS_REGION_3:
			MEMC_LibSetDemoWindowSettings(BYPASS_REGION_3, x, w, y, h);
			MEMC_LibSetDemoWindowEnable(BYPASS_REGION_3, bOnOff);
			break;

		case BYPASS_REGION_4:
			MEMC_LibSetDemoWindowSettings(BYPASS_REGION_4, x, w, y, h);
			MEMC_LibSetDemoWindowEnable(BYPASS_REGION_4, bOnOff);
			break;

		default:
			break;
	}
}
void Scaler_MEMC_SetReverseControl(unsigned char u8Mode){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
}
void Scaler_MEMC_Freeze(unsigned char type){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
}
void Scaler_MEMC_SetDemoBar(unsigned char bOnOff, unsigned char r, unsigned char g, unsigned char b){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
}
void Scaler_MEMC_DEBUG(void){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
}

void Scaler_MEMC_SetInOutputUseCase(unsigned char input_re, unsigned char output_re, unsigned char input_for, unsigned char output_for){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
	MEMC_LibSetInOutUseCase(input_re, output_re, input_for, output_for);
}

void Scaler_MEMC_SetInputOutputFormat(unsigned char input_for, unsigned char output_for){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
	//MEMC_LibSetInputOutputFormat(input_for, output_for);
}

void Scaler_MEMC_SetInputOutputResolution(unsigned char input_re, unsigned char output_re){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
	//MEMC_LibSetInputOutputResolution(input_re, output_re);
}

void Scaler_MEMC_SetInputFrameRate(unsigned char frame_rate){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
	MEMC_LibSetInputFrameRate(frame_rate);
}

void Scaler_MEMC_SetOutputFrameRate(unsigned char frame_rate){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
	MEMC_LibSetOutputFrameRate(frame_rate);
}

void Scaler_MEMC_SetMEMCFrameRepeatEnable(unsigned char enable){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
	MEMC_LibSetMEMCFrameRepeatEnable(enable, 0);
}

unsigned char g_input_mode = 0;
void ISR_Scaler_MEMC_SetInOutMode(unsigned char emom){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
	MEMC_LibSetInOutMode(emom, FALSE);
	g_input_mode = emom;
}

unsigned char ISR_Scaler_MEMC_GetInOutMode(void){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
	return g_input_mode;
}
unsigned char g_JP_DTV_4K_flag = 0;
void MEMC_Lib_Set_JP_DTV_4K_flag(unsigned char num)
{
	g_JP_DTV_4K_flag = num;
}
unsigned char MEMC_Lib_Get_JP_DTV_4K_flag(void)
{
	return g_JP_DTV_4K_flag;
}

void Scaler_MEMC_SetInstantBootInitPhaseTable(){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);

	//enable MEMC input/output clock before run MEMC isr
	rtd_outl(SYS_REG_SYS_DISPCLKSEL_reg,rtd_inl(SYS_REG_SYS_DISPCLKSEL_reg)|( BIT_5 | BIT_6));

	MEMC_LibForcePhaseTableInit();
}

void Scaler_MEMC_SetMEMC_Enable(unsigned char enable){
	scaler_memc_enable = enable;
}

void Scaler_MEMC_SetCinemaMode(unsigned char enable){
	rtd_pr_memc_notice("[MEMC][%s][%d]\n",__FUNCTION__, enable);

	MEMC_LibSetCinemaMode(enable);
}

unsigned char Scaler_MEMC_GetMEMC_Enable(void){
	return scaler_memc_enable;
}

unsigned char Scaler_MEMC_GetMEMC_Mode(void){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);
	return MEMC_LibGetMEMCMode();
}

BOOL Scaler_MEMC_SetPowerSaveOn(){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);

	return MEMC_LibPowerSaveOn();
}

BOOL Scaler_MEMC_SetPowerSaveOff(){
	rtd_pr_memc_notice("[MEMC][%s]\n",__FUNCTION__);

	return MEMC_LibPowerSaveOff();
}

unsigned int Scaler_MEMC_ISR_InputVSInterrupt(void){	

	MEMC_LibInputVSInterrupt();
	return 0;
}


unsigned int Scaler_MEMC_ISR_OutputVSInterrupt(void){

	MEMC_LibOutputVSInterrupt();
	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
void memc_TasksHandler_task( struct tasklet_struct *data )
#else
void memc_TasksHandler_task( unsigned long data )
#endif
{
	if(MEMC_GetInOutISREnable() && ((g_memc_hdmi_switch_state == 0) && (HDMI_PowerSaving_stage == 0) && (g_memc_switch_state == 0) && (DTV_PowerSaving_stage == 0)) && (Scaler_MEMC_GetMEMC_Enable() == 1))
		ScalerMEMC_TasksHandler();
	return;	
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
DECLARE_TASKLET(memc_TasksHandler_tasklet, memc_TasksHandler_task);
#else
DECLARE_TASKLET(memc_TasksHandler_tasklet, memc_TasksHandler_task, 0);
#endif

//#define BIT(x)               (1 <<x)

#if 1
extern unsigned int loadscript_cnt_2;

unsigned int Scaler_MEMC_ISR(void){

#ifdef CONFIG_MEMC_BYPASS
	return MEMC_IRQ_HANDLED;
#else

#ifdef CONFIG_MEMC_TASK_QUEUE
	
#if 0
	/*resolve tasks from rpc*/
	if(MEMC_GetInOutISREnable() && ((g_memc_hdmi_switch_state == 0) && (HDMI_PowerSaving_stage == 0) && (g_memc_switch_state == 0) && (DTV_PowerSaving_stage == 0)) && (Scaler_MEMC_GetMEMC_Enable() == 1))
		ScalerMEMC_TasksHandler();
#else
	tasklet_schedule(&memc_TasksHandler_tasklet);
#endif
	
#endif	

#ifdef MEMC_ISR_Times_debug
		if(MEMC_GetInOutISREnable()){
			static unsigned int sEnterCnt = 0;
			static int64_t sLastPts_1s = 0;
			int64_t currentPts = MEMC_GetPTS();
	
			sEnterCnt++;
			if(currentPts < sLastPts_1s){
				sLastPts_1s = currentPts;
				sEnterCnt = 0;
			}
			if(currentPts - sLastPts_1s > 90090){
				rtd_pr_memc_notice("TasksHandler(cnt:%d,%d)'\n", sEnterCnt,Scaler_MEMC_GetMEMC_Enable() );				
				sLastPts_1s = currentPts;
				sEnterCnt = 0;
			}			 
		}
		
#endif




	return MEMC_IRQ_HANDLED;
#endif
}

#else
unsigned char u1_is_me_dead;
extern unsigned int loadscript_cnt_2;

unsigned int Scaler_MEMC_ISR(void){

/*#ifdef CONFIG_MEMC_BYPASS
        return MEMC_IRQ_HANDLED;
#else*/

	//LogPrintf(DBG_MSG,  "[MEMC][%s]\n",__FUNCTION__);
	static unsigned char u1_is_me_dead_pre = 0;
	unsigned int u32_rb_val = 0;

	_RPC_clues* RPC_SmartPic_clue = (_RPC_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);

	if(get_MEMC_bypass_status_refer_platform_model() == TRUE)
		return MEMC_IRQ_HANDLED;
	else{
	if(RPC_SmartPic_clue == NULL){
		u1_is_me_dead = 255;
	} else {
		u1_is_me_dead = RPC_SmartPic_clue->MEMC_VCPU_setting_info.is_me_dead;
	}

	u32_rb_val = rtd_inl(SYS_REG_SYS_CLKEN3_reg);
	u32_rb_val = (u32_rb_val>>31);

#if 0//def CONFIG_RTK_KDEV_DEBUG_ISR
	unsigned int reg_value = 0;
	reg_value = rtd_inl(0xb802e4f8);

	unsigned int time1;
	static unsigned int time1s_set=0;
	static unsigned int time1s_start=0;
	static unsigned int time1s_end=0;
	static unsigned int time1s_cnt=0;
	static unsigned int time_cnt1=0;
	static unsigned int time_cnt2=0;
	static unsigned int time4 =0, time4_pre = 0;
	if((reg_value & BIT(31))){
		time_cnt1++;
	}

	time1 = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
	if(time1s_set==0)
	{
		time1s_start = time1;
		time1s_set=1;
		time1s_cnt=0;
	}
	time1s_end = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);

#endif

//	if (u1_is_me_dead == 0 && u32_rb_val == 1){
	if (u32_rb_val == 1 && (MEMC_Lib_get_memc_PowerSaving_Mode()!=1) && MEMC_BRING_UP_InitDone_flag >= 1){
//	if (u1_is_me_dead == 0) {
		if (u1_is_me_dead_pre == 1) {
			//LogPrintf(DBG_WARNING,  "[Scaler_MEMC_ISR] me dead recover , enable isr now!!\n\r");
		}
#if 0//def CONFIG_RTK_KDEV_DEBUG_ISR
	time1s_cnt++;
	time_cnt2++;
#endif

		#ifdef CONFIG_MEMC_TASK_QUEUE
		/*resolve tasks from rpc*/
		if(MEMC_GetInOutISREnable() && ((g_memc_hdmi_switch_state == 0) && (HDMI_PowerSaving_stage == 0) && (g_memc_switch_state == 0) && (DTV_PowerSaving_stage == 0)) && (Scaler_MEMC_GetMEMC_Enable() == 1))
			ScalerMEMC_TasksHandler();
		#endif		
		MEMC_LibInputVSInterrupt();
		MEMC_LibOutputVSInterrupt();

	} else {
		if (u1_is_me_dead_pre == 0) {
			//LogPrintf(DBG_WARNING,  "[Scaler_MEMC_ISR] me dead happened , disable isr now!!\n\r");
			}
		if (u32_rb_val != 1) {
			//rtd_pr_memc_notice("[Scaler_MEMC_ISR] memc clock off, disable isr now!!\n\r");
		}
	}
	u1_is_me_dead_pre = u1_is_me_dead;

#if 0//def CONFIG_RTK_KDEV_DEBUG_ISR
//====================================================
	time4=rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
	if((time1s_end-time1s_start)>=90000)
	{
		rtd_pr_memc_info("[ISR]MEMC enable  %x,%d,%d,%d\n",reg_value, u1_is_me_dead, u32_rb_val, u1_is_me_dead_pre);
		rtd_pr_memc_info("[ISR]MEMC timestamp %d\n",(time4 - time4_pre));
		rtd_pr_memc_info("[ISR]MEMC times %d, %d\n",time_cnt1,time_cnt2);
		if((time1s_end-time1s_start)>=90000){
			rtd_pr_memc_info("[ISR]MEMC isr %d times(%d CLK)\n",time1s_cnt, (time1s_end-time1s_start));
			time1s_set=0;
		}
	}
	time4_pre = time4;
//====================================================
#endif
	return MEMC_IRQ_HANDLED;
	}
//#endif
}
#endif

unsigned int scalerVIP_Get_MEMC_status_check(MEMC_STATUS_CHECK MEMC_case_status)
{
	unsigned int repeat_status = 255;
	unsigned int clk_state = 255;
	unsigned int status = 0;
	static unsigned int pre_clk_state = 0;

	clk_state = rtd_inl(SYS_REG_SYS_CLKEN3_reg);
	clk_state = clk_state>>27;
	
	if ( (clk_state&0x10)==0 || (clk_state&0x4)==0){
		status = 255;
		if(pre_clk_state != clk_state ){
			rtd_pr_memc_notice("[%s][MEMC] CLK_check, b800011c[27:31]=,%d \n", __func__, clk_state);
		}	
		pre_clk_state = clk_state;
		return status;	
	}else{
		if(pre_clk_state != clk_state ){
			rtd_pr_memc_notice("[%s][MEMC] CLK_check, b800011c[27:31]=,%d \n", __func__, clk_state);
		}
		pre_clk_state = clk_state;
	}
	
	switch(MEMC_case_status) {

		case MEMC_REPEAT_STATUS:
			//=== LBMC_LBMC_24_reg (0xb8099924[4:4]) ======
			//0x0:repeat_off, 0x1:repeat_on 
			//=======================
			repeat_status = rtd_inl(LBMC_LBMC_24_reg);
			repeat_status = (repeat_status>>4) & 0x1;
	 		//ReadRegister(LBMC_LBMC_24_reg, 4, 4, &repeat_status);
			status = repeat_status;
	 	break;
		
		case MEMC_CLK_STATUS:
		//=== SYS_REG_SYS_CLKEN3_reg (0xb800011c[27:31]) ======
		//=== CLKEN_MEMC			[31]
		//=== CLKEN_MEMC_ME			[30]
		//=== CLKEN_MEMC_RBUS 		[29]
		//=== CLKEN_LZMA			[28]
		//=== CLKEN_MEMC_MC			[27]
		//===========================
			clk_state = rtd_inl(SYS_REG_SYS_CLKEN3_reg);
			clk_state = clk_state>>27;
	 		//ReadRegister(SYS_REG_SYS_CLKEN3_reg, 27, 31, &clk_state);
			status = clk_state;
	 	break;

		default : 
	 	break;
	}

	return status;
	
}

unsigned int scalerVIP_Get_MEMC_status(void)
{
	unsigned int status_repeat = 255;
	unsigned int state_clk = 255;
	static unsigned int pre_status_repeat= 0, pre_state_clk=0;

	state_clk = scalerVIP_Get_MEMC_status_check(MEMC_CLK_STATUS);
	
	if(state_clk == 255){
		if(pre_state_clk != state_clk){
			rtd_pr_memc_notice("[%s][MEMC] CLK_ALL_OFF, clk=,%d \n", __func__, state_clk);
		}
		pre_state_clk = state_clk;
		return MEMC_CLK_ALLOFF;		
	}else{
		status_repeat = scalerVIP_Get_MEMC_status_check(MEMC_REPEAT_STATUS);
		
		if((pre_state_clk != state_clk) || (pre_status_repeat != status_repeat) ){
			rtd_pr_memc_notice("[%s][MEMC] Check_status_change, clk=,%d, repeat=,%d \n", __func__, state_clk, status_repeat);
		}
		
		pre_state_clk = state_clk;
		pre_status_repeat = status_repeat;
		
		if((state_clk>>3)==0x2 ) {
			return MEMC_CLK_MCONLY_REPEATON;
		}else if((state_clk>>3)==0x3 ) {
			if(status_repeat){
				return  MEMC_CLK_ALLON_REPEATON;
			}else{
				return  MEMC_CLK_ALLON_REPEATOFF;
			}			
		}

		return  MEMC_STATUS_APPLY_CHECK;		
	}


}

void Scaler_MEMC_resetMEMCMode(void)
{
	//(LGE)STR init force repeatmode, then change by WEBOS or others
	MEMC_LibResetMEMCMode();
}

extern MEMC_INFO_FROM_SCALER *Scaler_Get_Info_from_Scaler(VOID);

typedef enum{
	INOUT_FRAMERATE_RATIO_1to1_OUTPUT_60 = 0,	// in:50 out:50, in:60 out:60
	INOUT_FRAMERATE_RATIO_1to2_OUTPUT_60 = 1,	// in:24 out:48, in:25 out:50, in:30 out:60
	INOUT_FRAMERATE_RATIO_2to5_OUTPUT_60 = 2,	// in:24 out:60
	INOUT_FRAMERATE_RATIO_1to2_OUTPUT_120 = 3,	// in:48 out:96, in:50 out:100, in:60 out:120
	INOUT_FRAMERATE_RATIO_1to4_OUTPUT_120 = 4,	// in:25 out:100, in:30 out:120
	INOUT_FRAMERATE_RATIO_1to5_OUTPUT_120 = 5,	// in:24 out:120
	INOUT_SUPPORT_NUM,	
	INOUT_F = 255,	// no support
}INOUT_SUPPORT_TYPE;

typedef enum{
	MemcDelay_ultralowdelay,	// ultra low delay
	MemcDelay_game,				// game mode
	MemcDelay_repeat_pz,		// repeat mode with pz mode
	MemcDelay_repeat_iz,		// repeat mode with iz mode
	MemcDelay_normal,			// normal mode
	MemcDelay_Num,
}MemcDelay_TYPE;

const unsigned char g_ucMemcDelay_table[INOUT_SUPPORT_NUM][MemcDelay_Num] = 
{
/**** memc delay time(ms) = g_ucMemcDelay_table * 500 / output_frame_rate ****/

// game(ultra low delay)	game	repeat(pz)	repeat(iz)	normal
 	{		0,	 			2,			12,			14, 	14}, // in:50 out:50, in:60 out:60
 	{		4,	 			4,			14,			18, 	16}, // in:24 out:48, in:25 out:50, in:30 out:60
 	{		5,	 			5,			18,			24, 	20}, // in:24 out:60
 	{		2,	 			4,			22,			26, 	24}, // in:48 out:96, in:50 out:100, in:60 out:120
 	{		8,	 			8,			26,			34, 	30}, // in:25 out:100, in:30 out:120
 	{		10,	 			10,			34,			44, 	36}, // in:24 out:120 
};

unsigned int Round(unsigned int input)
{
    return (((2*input)+1)>>1);
}

unsigned char Get_InOutType_By_FrameRate(unsigned int in_frame_rate, unsigned int out_frame_rate)
{
	unsigned char u8_InOut_id = INOUT_F;

	if(out_frame_rate<=60) {
		if(in_frame_rate==out_frame_rate){ // in:50 out:50, in:60 out:60
			u8_InOut_id = INOUT_FRAMERATE_RATIO_1to1_OUTPUT_60;
		}
		else if(in_frame_rate*2==out_frame_rate){ // in:24 out:48, in:25 out:50, in:30 out:60
			u8_InOut_id = INOUT_FRAMERATE_RATIO_1to2_OUTPUT_60;
		}
		else if(in_frame_rate*5==out_frame_rate*2){ // in:24 out:60
			u8_InOut_id = INOUT_FRAMERATE_RATIO_2to5_OUTPUT_60;
		}
	}
	else if(out_frame_rate<=120) {
		if(in_frame_rate*2==out_frame_rate){ // in:48 out:96, in:50 out:100, in:60 out:120
			u8_InOut_id = INOUT_FRAMERATE_RATIO_1to2_OUTPUT_120;
		}
		else if(in_frame_rate*4==out_frame_rate){ // in:25 out:100, in:30 out:120
			u8_InOut_id = INOUT_FRAMERATE_RATIO_1to4_OUTPUT_120;
		}
		else if(in_frame_rate*5==out_frame_rate){ // in:24 out:120 
			u8_InOut_id = INOUT_FRAMERATE_RATIO_1to5_OUTPUT_120;
		}
	}

	return u8_InOut_id;
}

unsigned int Scaler_MEMC_GetAVSyncDelay(void)
{
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
		return 0;
#else
	//if(get_MEMC_bypass_status_refer_platform_model() == FALSE || get_platform() == PLATFORM_K8HP )
	if(get_MEMC_bypass_status_refer_platform_model() == FALSE)
	{
	static unsigned char MemcDelay_check =0;
	//MEMC_INFO_FROM_SCALER *info_from_scaler = Get_Info_from_Scaler();
	MEMC_INFO_FROM_SCALER *info_from_scaler = Scaler_Get_Info_from_Scaler();
	unsigned int log_en = 0;
	unsigned int repeat_mode = 0, pz_sel = 0, MC_8_buffer_en = 0;
	unsigned char u8_InOut_id = INOUT_F;
	static unsigned int in_frame_rate_pre = 0, out_frame_rate_pre = 0;

	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 0, 0, &MC_8_buffer_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 25, 25, &log_en);
	//ReadRegister(LBMC_LBMC_24_reg, 4, 4, &repeat_mode);
	//ReadRegister(LBMC_LBMC_24_reg, 13, 13, &pz_sel);
	if(MEMC_LibGetMEMCMode() == MEMC_OFF){
		repeat_mode = 1;
		if(MEMC_LibGetCinemaMode() == FALSE && info_from_scaler->in_frame_rate == 24 && info_from_scaler->out_frame_rate == 120){
			if(MC_8_buffer_en == 0)
				pz_sel = 1;
			else
				pz_sel = 0; // 8 buffer no need to use PZ
		}
	}


	// for K-17852 +++
	if(info_from_scaler->in_frame_rate == 24 && info_from_scaler->out_frame_rate == 60){
		if(info_from_scaler->game_mode_flag){
			g_ucMemcDelay = 39; // K-23281 // 3*delay_time_per_frm; // 48
		}
		else{
			if(repeat_mode){
				if(pz_sel == 0){
					g_ucMemcDelay = 200; // 12*delay_time_per_frm; // 184
				}
				else{
					g_ucMemcDelay = 150; // 9*delay_time_per_frm; // 184
				}
			}
			else{
				g_ucMemcDelay = 167; // 10*delay_time_per_frm; // 184
			}
		}
	}
	else if(info_from_scaler->in_frame_rate == 24 && info_from_scaler->out_frame_rate == 48){
		//if(info_from_scaler->game_mode_flag){
			g_ucMemcDelay = 188; // 3*delay_time_per_frm; // 48
		//}
		//else{
		//	g_ucMemcDelay = 188; // 11*delay_time_per_frm; // 176
		//}
	}
	else if(info_from_scaler->in_frame_rate == 25 && info_from_scaler->out_frame_rate == 50){
		if(info_from_scaler->game_mode_flag){
			g_ucMemcDelay = 40; // 2*delay_time_per_frm; // 40
		}
		else{
			if(repeat_mode){
				if(pz_sel == 0){
					g_ucMemcDelay = 180; // 9*delay_time_per_frm; // 184
				}
				else{
					g_ucMemcDelay = 140; // 7*delay_time_per_frm; // 184
				}
			}
			else{
				g_ucMemcDelay = 160; // 8*delay_time_per_frm; // 184
			}
		}
	}
	else if(info_from_scaler->in_frame_rate == 30 && info_from_scaler->out_frame_rate == 60){
		if(info_from_scaler->game_mode_flag){
			g_ucMemcDelay = 33; // 2*delay_time_per_frm; // 32
		}
		else{
			if(repeat_mode){
				if(pz_sel == 0){
					g_ucMemcDelay = 150; // 9*delay_time_per_frm; // 184
				}
				else{
					g_ucMemcDelay = 117; // 7*delay_time_per_frm; // 184
				}
			}
			else{
				g_ucMemcDelay = 133; // 8*delay_time_per_frm; // 184
			}
		}
	}
	else if(info_from_scaler->in_frame_rate == 50 && info_from_scaler->out_frame_rate == 50){
		if(info_from_scaler->game_mode_flag){
			if((info_from_scaler->hdmi_flag)||(info_from_scaler->vdec_flag)){
				g_ucMemcDelay = 0; // 0*delay_time_per_frm; // 0
			}
			else{
				g_ucMemcDelay = 20; // 1*delay_time_per_frm; // 20
			}
		}
		else{
			if(repeat_mode){
				if(pz_sel == 0){
					g_ucMemcDelay = 140; // 7*delay_time_per_frm; // 184
				}
				else{
					g_ucMemcDelay = 120; // 6*delay_time_per_frm; // 184
				}
			}
			else{
				g_ucMemcDelay = 140; // 7*delay_time_per_frm; // 184
			}
		}
	}
	else if(info_from_scaler->in_frame_rate == 60 && info_from_scaler->out_frame_rate == 60){
		if(info_from_scaler->game_mode_flag){
			if((info_from_scaler->hdmi_flag)||(info_from_scaler->vdec_flag)){
				g_ucMemcDelay = 0; // 0*delay_time_per_frm; // 0
			}
			else{
				g_ucMemcDelay = 17; // 1*delay_time_per_frm; // 16
			}
		}
		else{
			if(repeat_mode){
				if(pz_sel == 0){
					g_ucMemcDelay = 117; // 7*delay_time_per_frm; // 184
				}
				else{
					g_ucMemcDelay = 100; // 6*delay_time_per_frm; // 184
				}
			}
			else{
				g_ucMemcDelay = 117; // 7*delay_time_per_frm; // 184
			}
		}
	}
	else if(info_from_scaler->in_frame_rate == 24 && info_from_scaler->out_frame_rate == 120){
		if(info_from_scaler->game_mode_flag){
			g_ucMemcDelay = 42; // 5*delay_time_per_frm; // 40
		}
		else{
			if(repeat_mode){
				if(pz_sel == 0){
					g_ucMemcDelay = 183; // 22*delay_time_per_frm; // 184
				}
				else{
					g_ucMemcDelay = 142; // 17*delay_time_per_frm; // 184
				}
			}
			else{
				g_ucMemcDelay = 150; // 18*delay_time_per_frm; // 184
			}
		}
	}
	else if(info_from_scaler->in_frame_rate == 24 && info_from_scaler->out_frame_rate == 96){
		g_ucMemcDelay = 178; // 17*delay_time_per_frm;
	}
	else if(info_from_scaler->in_frame_rate == 25 && info_from_scaler->out_frame_rate == 100){
		if(info_from_scaler->game_mode_flag){
			g_ucMemcDelay = 40; // 4*delay_time_per_frm; // 40
		}
		else{
			if(repeat_mode){
				if(pz_sel == 0){
					g_ucMemcDelay = 170; // 17*delay_time_per_frm; // 184
				}
				else{
					g_ucMemcDelay = 130; // 13*delay_time_per_frm; // 184
				}
			}
			else{
				g_ucMemcDelay = 150; // 14.5*delay_time_per_frm; // 184
			}
		}
	}
	else if(info_from_scaler->in_frame_rate == 30 && info_from_scaler->out_frame_rate == 120){
		if(info_from_scaler->game_mode_flag){
			g_ucMemcDelay = 33; // 4*delay_time_per_frm; // 32
		}
		else{
			if(repeat_mode){
				if(pz_sel == 0){
					g_ucMemcDelay = 142; // 17*delay_time_per_frm; // 184
				}
				else{
					g_ucMemcDelay = 108; // 13*delay_time_per_frm; // 184
				}
			}
			else{
				g_ucMemcDelay = 121; // 14.5*delay_time_per_frm; // 184
			}
		}
	}
	else if(info_from_scaler->in_frame_rate == 50 && info_from_scaler->out_frame_rate == 100){
		if(info_from_scaler->game_mode_flag){
			if(info_from_scaler->hdmi_flag || (info_from_scaler->adaptive_stream_flag && info_from_scaler->YT_callback_flag)){
				g_ucMemcDelay = 10; // 1*delay_time_per_frm; // 10
			}
			else{
				g_ucMemcDelay = 20; // 2*delay_time_per_frm; // 20
			}
		}
		else{
			if(repeat_mode){
				if(pz_sel == 0){
					g_ucMemcDelay = 130; // 13*delay_time_per_frm; // 184
				}
				else{
					g_ucMemcDelay = 110; // 11*delay_time_per_frm; // 184
				}
			}
			else{
				g_ucMemcDelay = 120; // 12*delay_time_per_frm; // 184
			}
		}
	}
	else if(info_from_scaler->in_frame_rate == 60 && info_from_scaler->out_frame_rate == 120){
		if(info_from_scaler->game_mode_flag){
			if(info_from_scaler->hdmi_flag || (info_from_scaler->adaptive_stream_flag && info_from_scaler->YT_callback_flag)){
				g_ucMemcDelay = 8; // 1*delay_time_per_frm; // 8
			}
			else{
				g_ucMemcDelay = 17; // 2*delay_time_per_frm; // 16
			}
		}
		else{
			if(repeat_mode){
				if(pz_sel == 0){
					g_ucMemcDelay = 108; // 13*delay_time_per_frm; // 184
				}
				else{
					g_ucMemcDelay = 92; // 11*delay_time_per_frm; // 184
				}
			}
			else{
				g_ucMemcDelay = 100; // 12*delay_time_per_frm; // 184
			}
		}
	}
	else if(info_from_scaler->in_frame_rate == 144 && info_from_scaler->out_frame_rate == 144){
		g_ucMemcDelay = 0;
	}
	else{
		u8_InOut_id = Get_InOutType_By_FrameRate(info_from_scaler->in_frame_rate, info_from_scaler->out_frame_rate);
		
		// support in-out frame rate
		// memc delay time(ms) = g_ucMemcDelay_table * 500 / output_frame_rate
		if(u8_InOut_id < INOUT_SUPPORT_NUM){				
			if(info_from_scaler->game_mode_flag){
				if(info_from_scaler->hdmi_flag || 
					(info_from_scaler->adaptive_stream_flag && info_from_scaler->YT_callback_flag && info_from_scaler->in_frame_rate*2==info_from_scaler->out_frame_rate) ||
					(info_from_scaler->vdec_flag && info_from_scaler->in_frame_rate*2!=info_from_scaler->out_frame_rate)){ 
					g_ucMemcDelay = Round(g_ucMemcDelay_table[u8_InOut_id][MemcDelay_ultralowdelay]*500/info_from_scaler->out_frame_rate);
				}
				else{
					g_ucMemcDelay = Round(g_ucMemcDelay_table[u8_InOut_id][MemcDelay_game]*500/info_from_scaler->out_frame_rate);
				}
			}
			else{
				if(repeat_mode){
					if(pz_sel == 0){
						g_ucMemcDelay = Round(g_ucMemcDelay_table[u8_InOut_id][MemcDelay_repeat_iz]*500/info_from_scaler->out_frame_rate);
					}
					else{
						g_ucMemcDelay = Round(g_ucMemcDelay_table[u8_InOut_id][MemcDelay_repeat_pz]*500/info_from_scaler->out_frame_rate);
					}
				}
				else{
					g_ucMemcDelay = Round(g_ucMemcDelay_table[u8_InOut_id][MemcDelay_normal]*500/info_from_scaler->out_frame_rate);
				}
			}

			if(in_frame_rate_pre != info_from_scaler->in_frame_rate || out_frame_rate_pre != info_from_scaler->out_frame_rate) {
				rtd_pr_memc_notice("[%s][%d] memc delay time:%d  delay table:%d  in:%d out:%d\n\r", __FUNCTION__, __LINE__, g_ucMemcDelay, u8_InOut_id, info_from_scaler->in_frame_rate, info_from_scaler->out_frame_rate);
			}
		}
		else {
			// memc delay frames
			// game mode:1 frames, iz:5 frames, pz:4 frames, blending:4.5/5 frames
			if(info_from_scaler->game_mode_flag){
				g_ucMemcDelay = Round(1000/info_from_scaler->in_frame_rate); //delay 1 frame
			}
			else{
				if(repeat_mode){
					if(pz_sel == 0){
						g_ucMemcDelay = Round(5000/info_from_scaler->in_frame_rate); //delay 5 frames
					}
					else{
						g_ucMemcDelay = Round(4000/info_from_scaler->in_frame_rate); //delay 4 frames
					}
				}
				else{
					if(info_from_scaler->out_frame_rate/info_from_scaler->in_frame_rate>=2){
						g_ucMemcDelay = Round(4500/info_from_scaler->in_frame_rate); //delay 4.5 frames
					}
					else {
						g_ucMemcDelay = Round(5000/info_from_scaler->in_frame_rate); //delay 5 frames
					}
				}
			}

			if(in_frame_rate_pre != info_from_scaler->in_frame_rate || out_frame_rate_pre != info_from_scaler->out_frame_rate) {
				rtd_pr_memc_notice("[%s][%d] not match case!!  memc delay time:%d  in:%d out:%d\n\r", __FUNCTION__, __LINE__, g_ucMemcDelay, info_from_scaler->in_frame_rate, info_from_scaler->out_frame_rate);
			}
		}
	}
	
	//else{ keep actual calculation }
	if(log_en){
		rtd_pr_memc_notice("[%s][%d][,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__, __LINE__, g_ucMemcDelay,
			info_from_scaler->in_frame_rate, info_from_scaler->out_frame_rate, info_from_scaler->game_mode_flag,
			info_from_scaler->adaptive_stream_flag, info_from_scaler->YT_callback_flag, info_from_scaler->hdmi_flag, repeat_mode, pz_sel);
	}
	// for K-17852 ---

		if(g_ucMemcDelay!= MemcDelay_check){
			//rtd_pr_memc_notice("[Scaler_MEMC_ISR] memc clock off, disable isr now!!\n\r");
			MemcDelay_check = g_ucMemcDelay;
		}

		in_frame_rate_pre = info_from_scaler->in_frame_rate;
		out_frame_rate_pre = info_from_scaler->out_frame_rate;
		return g_ucMemcDelay;
	}
	else {
		return 0;
	}

#endif
}

void Scaler_MEMC_SetAVSyncDelay(unsigned int a_ucDelay)
{
	g_ucMemcDelay = a_ucDelay;
}

#ifdef CONFIG_MEMC_TASK_QUEUE
 static DEFINE_MUTEX(memc_task_mutex);
#define memc_task_lock()      mutex_lock(&memc_task_mutex)
#define memc_task_unlock()    mutex_unlock(&memc_task_mutex)

unsigned char Scaler_MEMC_PushTask(SCALER_MEMC_TASK_T *pTask)
{
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
		return TRUE;
#else

	unsigned int id = 0;
	unsigned char ret = TRUE;
	
	//extern void hw_semaphore_try_lock(void);
	//extern void hw_semaphore_unlock(void);
//#ifdef CONFIG_MEMC_BYPASS
	//if(get_MEMC_bypass_status_refer_platform_model() == TRUE && get_platform() != PLATFORM_K8HP){
	if(get_MEMC_bypass_status_refer_platform_model() == TRUE) {
		return TRUE;
	}
	else {
//#else
		rtd_pr_memc_notice("[%s][%d][CASE,%d,][,%d,%d,%d,%d,%d,%d,]\n",__FUNCTION__, __LINE__, pTask->type,
			MEMC_GetInOutISREnable(), g_memc_hdmi_switch_state, HDMI_PowerSaving_stage,
			g_memc_switch_state, DTV_PowerSaving_stage, Scaler_MEMC_GetMEMC_Enable());
		/*get HW semaphore*/
		memc_task_lock();

		/*push task to queue*/
		id = (g_memc_tasks_header + 1) % MEMC_TASK_MAX_CNT;
		if(id == g_memc_tasks_tailer){
			rtd_pr_memc_notice("[%s]Task queue status(header=%d,tailer=%d)\n",\
				__FUNCTION__,g_memc_tasks_header,g_memc_tasks_tailer);
			rtd_pr_memc_notice("[%s][line:%d][,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__, __LINE__,
				MEMC_GetInOutISREnable(), g_memc_hdmi_switch_state, HDMI_PowerSaving_stage,
				g_memc_switch_state, DTV_PowerSaving_stage, Scaler_MEMC_GetMEMC_Enable());
			g_memc_tasks_tailer = (g_memc_tasks_tailer + 1) % MEMC_TASK_MAX_CNT;
			ret = TRUE;
		}
		if(g_memc_tasks_header >= MEMC_TASK_MAX_CNT){
			rtd_pr_memc_notice("[%s]Invalid task index(%d,%d).Resume!\n",\
				__FUNCTION__,g_memc_tasks_header,g_memc_tasks_tailer);
			g_memc_tasks_header = 0;
			g_memc_tasks_tailer = 0;
			id = (g_memc_tasks_header + 1) % MEMC_TASK_MAX_CNT;
		}
		memcpy(&g_memc_tasks_queue[g_memc_tasks_header],pTask,sizeof(SCALER_MEMC_TASK_T));
		g_memc_tasks_header = id;
		/*put HW semaphore*/

		memc_task_unlock();

		return ret;

	}
#endif	
}

unsigned char ScalerMEMC_PopTask(SCALER_MEMC_TASK_T *pTask)
{
	if(g_memc_tasks_tailer == g_memc_tasks_header)
		return FALSE;
	if(g_memc_tasks_tailer >= MEMC_TASK_MAX_CNT){
		rtd_pr_memc_notice("[%s]Invalid task index(%d,%d).Resume!\n",\
						__FUNCTION__,g_memc_tasks_header,g_memc_tasks_tailer);
		g_memc_tasks_header = 0;
		g_memc_tasks_tailer = 0;
		return FALSE;
	}
	memcpy(pTask,&g_memc_tasks_queue[g_memc_tasks_tailer],sizeof(SCALER_MEMC_TASK_T));
	g_memc_tasks_tailer = (g_memc_tasks_tailer + 1) % MEMC_TASK_MAX_CNT;

	return TRUE;
}

void ScalerMEMC_TasksHandler(VOID)
{
	SCALER_MEMC_TASK_T task;
	int task_cnt = 0;

	/*resolve all tasks in the queue*/
	while(task_cnt < MEMC_TASK_MAX_CNT && ScalerMEMC_PopTask(&task) == TRUE){
		switch(task.type){
		case SCALERIOC_MEMC_SETMOTIONCOMP:
		{
			rtd_pr_memc_notice("[SETMOTIONCOMP]%d,%d,%d\n",\
						task.data.memc_set_motion_comp.blurLevel,\
						task.data.memc_set_motion_comp.judderLevel,\
						task.data.memc_set_motion_comp.motion);
			Scaler_MEMC_SetMotionComp(task.data.memc_set_motion_comp.blurLevel,\
						task.data.memc_set_motion_comp.judderLevel,\
						task.data.memc_set_motion_comp.motion);
		}
		break;

		case SCALERIOC_MEMC_SETBLURLEVEL:
		{
			rtd_pr_memc_notice("[SETBLURLEVEL]%d\n",task.data.value);
			MEMC_LibSetMEMCMode(MEMC_USER);
			Scaler_MEMC_SetBlurLevel(task.data.value);
		}
		break;

		case SCALERIOC_MEMC_SETJUDDERLEVEL:
		{
			rtd_pr_memc_notice("[SETJUDDERLEVEL]%d\n",task.data.value);
			MEMC_LibSetMEMCMode(MEMC_USER);
			Scaler_MEMC_SetJudderLevel(task.data.value);
		}
		break;

		case SCALERIOC_MEMC_MOTIONCOMPONOFF:
		{
			rtd_pr_memc_notice("[MOTIONCOMPONOFF]%d\n",task.data.value);
			Scaler_MEMC_MotionCompOnOff(task.data.value);
		}
		break;

		case SCALERIOC_MEMC_LOWDELAYMODE:
		{
			rtd_pr_memc_notice("[LOWDELAYMODE]%d\n",task.data.value);
			Scaler_MEMC_LowDelayMode(task.data.value);
		}
		break;

		case SCALERIOC_MEMC_SETRGBYUVMode:
		{
			rtd_pr_memc_notice("[SETRGBYUVMode]%d\n",task.data.value);
			Scaler_MEMC_SetRGBYUVMode(task.data.value);
		}
		break;

		case SCALERIOC_MEMC_GETFRAMEDELAY:
		{
			rtd_pr_memc_notice("[GETFRAMEDELAY]\n");
			//Scaler_MEMC_GetFrameDelay ((unsigned short *)arg);
		}
		break;

		case SCALERIOC_MEMC_SETVIDEOBLOCK:
		{
			rtd_pr_memc_notice("[SETMOTIONCOMP]%d,%d\n",\
							task.data.memc_set_video_block.type,\
							task.data.memc_set_video_block.bOnOff);
			Scaler_MEMC_SetVideoBlock(task.data.memc_set_video_block.type,\
							task.data.memc_set_video_block.bOnOff);
		}
		break;

		case SCALERIOC_MEMC_SETTRUEMOTIONDEMO:
		{
			rtd_pr_memc_notice("[SETTRUEMOTIONDEMO]%d\n",task.data.value);
			Scaler_MEMC_SetTrueMotionDemo(task.data.value);
		}
		break;

		case SCALERIOC_MEMC_GETFIRMWAREVERSION:
		{
			rtd_pr_memc_notice("[GETFIRMWAREVERSION]%d\n",task.data.value);
			//Scaler_MEMC_GetFirmwareVersion((unsigned short *)arg);
		}
		break;

		case SCALERIOC_MEMC_SETBYPASSREGION:
		{
			rtd_pr_memc_notice("[SETBYPASSREGION]%d,%d,%d,%d,%d,%d\n",\
							task.data.memc_set_bypass_region.bOnOff,\
							task.data.memc_set_bypass_region.region,\
							task.data.memc_set_bypass_region.x,\
							task.data.memc_set_bypass_region.y,\
							task.data.memc_set_bypass_region.w,\
							task.data.memc_set_bypass_region.h);
			Scaler_MEMC_SetBypassRegion(task.data.memc_set_bypass_region.bOnOff,\
							task.data.memc_set_bypass_region.region,\
							task.data.memc_set_bypass_region.x,\
							task.data.memc_set_bypass_region.y,\
							task.data.memc_set_bypass_region.w,\
							task.data.memc_set_bypass_region.h);
		}
		break;

		case SCALERIOC_MEMC_SETREVERSECONTROL:
		{
			rtd_pr_memc_notice("[SETREVERSECONTROL]%d\n",task.data.value);
			Scaler_MEMC_SetReverseControl(task.data.value);
		}
		break;

		case SCALERIOC_MEMC_FREEZE:
		{
			rtd_pr_memc_notice("[FREEZE]%d\n",task.data.value);
			Scaler_MEMC_Freeze(task.data.value);
		}
		break;

		case SCALERIOC_MEMC_SETDEMOBAR:
		{
			rtd_pr_memc_notice("[SETDEMOBAR]%d,%d,%d,%d\n",\
						task.data.memc_set_demo_bar.bOnOff,\
						task.data.memc_set_demo_bar.r,\
						task.data.memc_set_demo_bar.g,\
						task.data.memc_set_demo_bar.b);
			Scaler_MEMC_SetDemoBar(task.data.memc_set_demo_bar.bOnOff,\
						task.data.memc_set_demo_bar.r,\
						task.data.memc_set_demo_bar.g,\
						task.data.memc_set_demo_bar.b);
		}
		break;

		case SCALERIOC_MEMC_SETINOUTUSECASE:
		{
			rtd_pr_memc_notice("[SETINOUTUSECASE]%d,%d,%d,%d\n",\
							task.data.memc_set_inout_usecase.input_re,\
							task.data.memc_set_inout_usecase.output_re,\
							task.data.memc_set_inout_usecase.input_for,\
							task.data.memc_set_inout_usecase.output_for);
			Scaler_MEMC_SetInOutputUseCase(task.data.memc_set_inout_usecase.input_re,\
							task.data.memc_set_inout_usecase.output_re,\
							task.data.memc_set_inout_usecase.input_for,\
							task.data.memc_set_inout_usecase.output_for);
		}
		break;

		case SCALERIOC_MEMC_SETINPUTOUTPUTFORMAT:
		{
			rtd_pr_memc_notice("[SETINPUTOUTPUTFORMAT]%d,%d\n",\
							task.data.memc_set_inout_format.input_for,\
							task.data.memc_set_inout_format.output_for);
			Scaler_MEMC_SetInputOutputFormat(task.data.memc_set_inout_format.input_for,\
							task.data.memc_set_inout_format.output_for);
		}
		break;

		case SCALERIOC_MEMC_SETINPUTOUTPUTRESOLUTION:
		{
			rtd_pr_memc_notice("[SETINPUTOUTPUTRESOLUTION]%d,%d\n",\
							task.data.memc_set_inout_resolution.input_re,\
							task.data.memc_set_inout_resolution.output_re);
			Scaler_MEMC_SetInputOutputResolution(task.data.memc_set_inout_resolution.input_re,\
							task.data.memc_set_inout_resolution.output_re);
		}
		break;

		case SCALERIOC_MEMC_SETINOUTPUTFRAMERATE:
		{
			rtd_pr_memc_notice("[SETINOUTPUTFRAMERATE]%d,%d\n",\
					task.data.memc_inout_framerate.input_frame_rate,\
					task.data.memc_inout_framerate.output_frame_rate);
			Scaler_MEMC_SetInputFrameRate(task.data.memc_inout_framerate.input_frame_rate);
			Scaler_MEMC_SetOutputFrameRate(task.data.memc_inout_framerate.output_frame_rate);
		}
		break;

		case SCALERIOC_MEMC_SETMEMCFRAMEREPEATENABLE:
		{
			rtd_pr_memc_notice("[SETMEMCFRAMEREPEATENABLE]%d\n",task.data.value);
			Scaler_MEMC_SetMEMCFrameRepeatEnable(task.data.value);
		}
		break;

		case SCALERIOC_MEMC_SETMEMCINOUTMODE:
		{
			rtd_pr_memc_notice("[SETMEMCINOUTMODE]%d\n",task.data.value);
			ISR_Scaler_MEMC_SetInOutMode(task.data.value);
		}
		break;

		case SCALERIOC_MEMC_SETCINEMAMODE:
		{
			rtd_pr_memc_notice("[SETCINEMAMODE]%d\n",task.data.value);
			Scaler_MEMC_SetCinemaMode(task.data.value);
		}
		break;

		default:
			rtd_pr_memc_notice("[%s]Unknown task type=%d(header=%d,tailer=%d)\n",\
				__FUNCTION__,task.type,g_memc_tasks_header,g_memc_tasks_tailer);
			return;
		}//~switch(task.type)
		task_cnt++;
	}//~while(task_cnt < MEMC_TASK_MAX_CNT && ScalerMEMC_PopTask(&task)
	if(task_cnt == MEMC_TASK_MAX_CNT){
		rtd_pr_memc_notice("[%s]Queue lenght maybe too small(header=%d,tailer=%d,max_tasks_num=%d)\n",\
			__FUNCTION__,g_memc_tasks_header,g_memc_tasks_tailer,MEMC_TASK_MAX_CNT);
	}
}

extern int register_temperature_callback(int degree,void *fn, void* data, char *module_name);
extern int Scaler_MEMC_MEMC_CLK_OnOff(unsigned char ME_enable, unsigned char MC_enable, unsigned char bMute);
#if IS_ENABLED(CONFIG_RTK_KDRV_THERMAL_SENSOR)
static short int  REG_DEGREE=110;
#if 0 //unused variable
static short int  REG_DEGREE_LV2=120;
static short int  REG_DEGREE_LV3=125;
#endif

extern void Scaler_MEMC_Set_PowerSaving_Status(unsigned char mode);
extern unsigned char Scaler_MEMC_Get_PowerSaving_Status(void);
static void Scaler_MEMC_thermal_callback(void *data, int cur_degree , int reg_degree, char *module_name)
{

	rtd_pr_memc_emerg("[Scaler_MEMC_thermal_callback][cur_deg=%d][Scaler_MEMC_Get_PowerSaving_Status()=%d]\n\r", cur_degree, Scaler_MEMC_Get_PowerSaving_Status());

#if 0
	if (cur_degree >= 125) {
		if (Scaler_MEMC_Get_PowerSaving_Status() != 2){
			rtd_pr_memc_emerg("[Scaler_MEMC_thermal_callback][cur_deg=%d] Disable MEMC!\n\r", cur_degree);
			Scaler_MEMC_Set_PowerSaving_Status(2);
			//handle_memc_powersaving_on((unsigned char)2);
			Scaler_MEMC_MEMC_CLK_OnOff(0,0,1);
		}
	}else if (cur_degree >= 115) {
		if (Scaler_MEMC_Get_PowerSaving_Status() == 0){
			rtd_pr_memc_emerg("[Scaler_MEMC_thermal_callback][cur_deg=%d] Set to MC only mode!\n\r", cur_degree);
			Scaler_MEMC_Set_PowerSaving_Status(1);
			//handle_memc_powersaving_on((unsigned char)1);
			Scaler_MEMC_MEMC_CLK_OnOff(0,1,1);
		}
	}
	else if(cur_degree < 110){
		if(Scaler_MEMC_Get_PowerSaving_Status() != 0){
			Scaler_MEMC_Set_PowerSaving_Status(0);
			//handle_memc_powersaving_on((unsigned char)0);
			Scaler_MEMC_MEMC_CLK_OnOff(1,1,1);
			rtd_pr_memc_emerg("[Scaler_MEMC_thermal_callback][cur_deg=%d] Enable MEMC!\n\r", cur_degree);
		}
	}
#else //120HZ
	if (cur_degree >= 125) {
		if (Scaler_MEMC_Get_PowerSaving_Status() == 0){
			//rtd_pr_memc_emerg("[Scaler_MEMC_thermal_callback][cur_deg=%d] Set to MC only mode!\n\r", cur_degree);
			Scaler_MEMC_Set_PowerSaving_Status(1);
			//handle_memc_powersaving_on((unsigned char)1);
			Scaler_MEMC_MEMC_CLK_OnOff(0,1,1);
		}
	}
	else if(cur_degree <= 115 ){
		if(Scaler_MEMC_Get_PowerSaving_Status() != 0){
			Scaler_MEMC_Set_PowerSaving_Status(0);
		}
	}	
#endif
	return;

}

int Scaler_register_MEMC_thermal_handler(void)
{
	int ret;
	int retdata = 0;

	rtd_pr_memc_info("%s\n",__func__);
	if ((ret = register_temperature_callback(REG_DEGREE, Scaler_MEMC_thermal_callback, (void*)&retdata, "MEMC_PowerSaving")) < 0)
		rtd_pr_memc_info("register MEMC thermal handler fail, ret:%d \n", ret);

	return 0;
}

#if 0 //no use
void Scaler_register_MEMC_thermal(int t0, int t1, int t2)
{
	if( (t2<135) && (t2> t1) && (t1 > t0))
	{
		REG_DEGREE=t0;
		REG_DEGREE_LV2=t1;
		REG_DEGREE_LV3=t2;
		Scaler_register_MEMC_thermal_handler();//use resource table;
	}
	else
	{
		Scaler_register_MEMC_thermal_handler();//use default;
	}
}
EXPORT_SYMBOL(Scaler_register_MEMC_thermal);
#endif

#ifndef CONFIG_SUPPORT_SCALER_MODULE
late_initcall(Scaler_register_MEMC_thermal_handler);
#endif

#endif //CONFIG_RTK_KDRV_THERMAL_SENSOR

#endif


