/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for memory related functions.
 *
 * @author 	$Author$
 * @date		$Date$
 * @version	$Revision$
 * @ingroup	memory
 */

/**
 * @addtogroup memory
 * @{
 */

/*============================ Module dependency  ===========================*/

//#include <unistd.h>			// 'close()'
//#include <fcntl.h>			// 'open()'
//#include <stdio.h>
//#include <string.h>
//#include <sys/mman.h>
 #ifndef BUILD_QUICK_SHOW
#include <linux/delay.h>
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <rtk_kdriver/RPCDriver.h>
#include <linux/pageremap.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
#include <linux/auth.h>
#endif
//#include <mach/io.h>
//#include "rtd_regs.h"
//#include "rtd_macro.h"
#include <rbus/mdomain_cap_reg.h>
#include <rbus/mdomain_disp_reg.h>
#include <rbus/di_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/mdomain_vi_main_reg.h>

//#include <rbus/scaler/rbustv-sb2-dcu1reg.h>
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
	#include <rtk_kdriver/scalercommon/scalerCommon.h>
#endif
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/io/ioregdrv.h>
//#include <tvscalercontrol/scaler/scalerstruct.h>
//#include <tvscalercontrol/scaler/modeState.h>
//#include <tvscalercontrol/scaler/scalerTimer.h>
//#include "tvscalercontrol/scaler/scalerLib.h"
//#include <tvscalercontrol/scaler/scalerColorLib.h>
#include <tvscalercontrol/vip/scalerColor.h>

#include <tvscalercontrol/scalerdrv/scalerclock.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
//#include "tvscalercontrol/scalerDrv/scalerDisplay.h"
#include <tvscalercontrol/scalerdrv/mode.h>
#include <tvscalercontrol/scalerdrv/pipmp.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/scalerdrv/scalerip.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <tvscalercontrol/scalerdrv/scaler_i2rnd.h>
//#include "kernel/scaler/scalerDrv.h"	// kernel driver

#include "tvscalercontrol/hdmirx/hdmifun.h"
//#include "Platform_Lib/panel/panelAPI.h"
//#include "tvscalercontrol/scaler/scalerAPI.h"
//#include "tvscalercontrol/scaler/state.h" //for drvif_memory_sw_buffer_swap_action()
#include <tvscalercontrol/scalerdrv/scaler_2dcvt3d_ctrl.h>
#include <tvscalercontrol/vip/di_ma.h>
#include <tvscalercontrol/i3ddma/i3ddma.h>

#include <rbus/demura_reg.h>
//#include <rbus/de_xcxl_reg.h>  
 #ifndef BUILD_QUICK_SHOW
 #include "tvscalercontrol/vdc/video.h"
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <VideoRPC_System.h>
	#include <VideoRPC_System_data.h>
#else
#include <rpc/VideoRPC_System.h>
#include <rpc/VideoRPC_System_data.h>
#endif
#include <tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>
#include <mach/rtk_platform.h>
#include <rtd_log/rtd_module_log.h>
#else
#include <mach/rtk_platform.h>
#include <no_os/printk.h>
#include <no_os/semaphore.h>
#include <no_os/spinlock.h>
#include <no_os/spinlock_types.h>
#include <no_os/slab.h>
#include <timer.h>
#include <rtk_kdriver/rmm/rmm.h>
#include <rtd_log/rtd_module_log.h>
#include <no_os/pageremap.h>
#endif

#include <rbus/scaleup_reg.h>
#include <rbus/scaledown_reg.h>
#include <tvscalercontrol/scalerdrv/zoom_smoothtoggle_lib.h>
#include <scaler_vscdev.h>
#include <scaler_vscdev.h>
#include <scaler/vipRPCCommon.h>

#ifdef CONFIG_RTK_KDRV_PWM
//#include <rtk_kdriver/rtk_pwm.h>
#endif

#include <rbus/timer_reg.h>

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
#ifdef BUILD_QUICK_SHOW
#define  dvr_memory_alignment(x, y)  ((unsigned long)x)
#define  PAGE_ALIGN(x)  (x)

#undef  BUG
#define BUG() do { \
	rtd_pr_scaler_memory_info("U-Boot BUG at %s:%d!\n", __FILE__, __LINE__); \
} while (0)
static bool is_power_of_2(unsigned long n)
{
    return (n != 0 && ((n & (n - 1)) == 0));
}
#endif
/*===================================  Types ================================*/
#ifdef USE_RTK_DEV_MEM
#define DEV_MEM     "/dev/rtkmem"
#else
#define DEV_MEM     "/dev/mem"
#endif

//#ifdef DUMP_SCALER_MEMORY
//#define CONFIG_DARWIN_SCALER
//#define CONFIG_MPIXEL_SHIFT
/*================================ Definitions ==============================*/
// DI bit number control (saving bandwidth)
#define DI_DMA_420_EN_START_BIT			30
#define DI_DMA_420_EN_MASK				(0x3 << DI_DMA_420_EN_START_BIT)
#define DI_DMA_LINESIZE_EVEN_START_BIT	12
#define DI_DMA_LINESIZE_EVEN_MASK			(0x7ff << DI_DMA_LINESIZE_EVEN_START_BIT)
#define DI_DMA_LINESIZE_ODD_START_BIT		0
#define DI_DMA_LINESIZE_ODD_MASK			(0x7ff << DI_DMA_LINESIZE_ODD_START_BIT)

//FBUS region
#define FBOUS_ADDRESS_LOW	0x1dc00000
#define FBOUS_ADDRESS_HIGH	0x20000000

#define Mag2_New_Flow // LearnRPG@20150430 new API for clear coding style

//PQC & PQDC
#define M_DOMAIN_PQC_LINE_MARGIN 6
/*================================== Variables ==============================*/


/*================================== Variables ==============================*/
static StructMemBlock MemTag[MEMIDX_UNKNOW];	// memory tag information
static unsigned int	AllocStartAddr = 0x00000000;
static unsigned int	AllocEndAddr = _DDR_END_ADDR;
static unsigned int	AllocStartAddrBlock[MEM_BLOCK_TOTAL_NUM];
static unsigned int	AllocEndAddrBlock[MEM_BLOCK_TOTAL_NUM];
//static unsigned int scaler_mem_virtualuncacheAddr=0;
unsigned char	*m_pCacheStartAddr = NULL;
unsigned long 	m_PhyStartAddr;
unsigned int	mMallocSize = 0x2000000;//0x1800000;//24MB//0x2100000;//33MB
//BYTE			*m_pSubMCacheStartAddr = NULL;
//BYTE			*m_pVDCCacheStartAddr = NULL;


extern unsigned char flag_3d;

#ifndef UT_flag
extern I3DDMA_3DDMA_CTRL_T i3ddmaCtrl;
#else
I3DDMA_3DDMA_CTRL_T i3ddmaCtrl;
#endif


// 3DDI memory usage control
static unsigned int bIpBtrMode=_DISABLE, bIpDma420Mode=_DISABLE;
static unsigned int MemDispStartAddr_3D [3];	//record memory disp start address for 3D
//frank@1023 add flag to indicate memory only DCU1 memory
//static bool m_only_dcu1=_FALSE;

static eMemCapAccessType enable_M_cap_mode_main = MEMCAPTYPE_FRAME;
static eMemCapAccessType enable_M_cap_mode_sub = MEMCAPTYPE_FRAME;



static UINT8 enable_M_compression_main = 0;
static UINT8 enable_M_compression_sub = 0;
static UINT8 M_compression_bits = COMPRESSION_NONE_BITS;
static UINT8 M_compression_mode = COMPRESSION_NONE_MODE;
static UINT8 Sub_M_compression_bits = COMPRESSION_NONE_BITS;
static UINT8 Sub_M_compression_mode = COMPRESSION_NONE_MODE;

static UINT32 M_fixed_virtual_memory_size = 0;
static UINT8 enable_M_GameMode = _DISABLE;
static UINT8 enable_M_GameMode_Dynamic = _DISABLE;
static UINT8 enable_M_LowLatencyMode_Vdec_direct = _DISABLE;

extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr; // debug scaler flag tracker
extern unsigned int smooth_toggle_game_mode_timeout_trigger_flag;

/*======================M-domain Multi-Buffer Controlling=====================*/
static UINT8 Mdomain_multibuffer_enable = false;
static UINT8 Mdomain_capture_multibuffer_enable = false;
static UINT8 Mdomain_multibuffer_number = 3;
static unsigned int Mdomain_multibuffer_size = 0;
static UINT8 Mdomain_multibuffer_number_pre = 0;
static UINT8 Mdomain_multibuffer_changed =0;

/*=================== Local Functions Phototype ==============*/

/*=================== Local Functions Phototype ==============*/
#ifndef Mag2_New_Flow
static void drv_memory_DMA420_config(unsigned char *bitNum);
#endif
extern unsigned char get_sub_OutputVencMode(void);
extern unsigned char drvif_color_Get_Two_Step_UZU_Mode(void);
extern unsigned char get_MEMC_bypass_status_refer_platform_model(void);
void drv_memory_Set_multibuffer_flag(UINT8 enable);
UINT8 drv_memory_Get_multibuffer_flag(void);
void drv_memory_Set_multibuffer_number(UINT8 number);
UINT8 drv_memory_Get_multibuffer_number(void);
void drv_memory_Set_multibuffer_number_pre(UINT8 number);
UINT8 drv_memory_Get_multibuffer_number_pre(void);
void drv_memory_Set_capture_multibuffer_flag(UINT8 enable);
UINT8 drv_memory_Get_capture_multibuffer_flag(void);
void drv_memory_Set_multibuffer_size(unsigned int buffer_size);
unsigned int drv_memory_Get_multibuffer_size(void);
void drv_memory_send_multibuffer_number_to_smoothtoogh(void);
void drv_memory_capture_multibuffer_update_addr(unsigned int buffer_size, unsigned int start_addr);
void drv_memory_capture_multibuffer_update_boundary(unsigned char buf_num, unsigned int buffer_size);
void drv_memory_capture_multibuffer_control(unsigned char buf_num, unsigned char disp2cap_distance);
void drv_memory_capture_multibuffer_address(unsigned int buf4_addr, unsigned int buf5_addr,unsigned int buf6_addr,unsigned int buf7_addr,unsigned int buf8_addr);
void drv_memory_display_multibuffer_control(unsigned char buf_num, unsigned char cap2disp_distance);
void drv_memory_capture_multibuffer_switch_buffer_number(void);
void drv_memory_display_multibuffer_switch_buffer_number(void);
void drv_memory_display_multibuffer_address(unsigned int buf4_addr, unsigned int buf5_addr,unsigned int buf6_addr,unsigned int buf7_addr,unsigned int buf8_addr);


static unsigned char mdomain_driver_status = FALSE;//can reset m capture status

#ifdef UT_flag
I3DDMA_3DDMA_CTRL_T* drvif_memory_GetI3ddmaCtrl(void)
{
	return &i3ddmaCtrl;
}
#endif

unsigned int drvif_memory_GetMemTag_size(unsigned int a_ulIndex)
{
	return MemTag[a_ulIndex].Size;
}

unsigned int drvif_memory_GetMemTag_addr(unsigned int a_ulIndex)
{
	return MemTag[a_ulIndex].StartAddr;
}


unsigned int Get_Val_drvif_memory_MemTag_VirAddr(unsigned int a_ulIndex)
{
	return MemTag[a_ulIndex].Allocate_VirAddr;
}

void Set_Val_drvif_memory_MemTag_VirAddr(unsigned int a_ulIndex, unsigned int a_ulAddr)
{
	MemTag[a_ulIndex].Allocate_VirAddr = a_ulAddr;
}

void set_mdomain_driver_status(unsigned char status)
{//set m domain driver ready or not
	mdomain_driver_status = status;
}

unsigned char get_mdomain_driver_status(void)
{//decide m domain driver finish or not
	return mdomain_driver_status;
}


/*================================== Function ===============================*/

/*============================================================================*/
#if 0
/**
 * memory_clear_mem_tag
 * Clear the specific memory tag
 *
 * @param <Idx> { index of DDR memory space, see eMemIdx for details }
 * @return { none }
 *
 */
//void __init memory_clear_mem_tag(eMemIdx Idx)
void memory_clear_mem_tag(eMemIdx Idx)
{
	unsigned char i;

	switch (Idx) {
		case MEMIDX_ALL:	//clear free block information
			for (i = 0; i < MEMIDX_ALL; i++) {	// clear all tag information
				MemTag[i].StartAddr = _DUMMYADDR;	// clear all tags
				MemTag[i].Size = 0x00;		// clear all tags
				MemTag[i].Status = 0;
			}
			break;
		default:
			drvif_memory_free_block(Idx);
			MemTag[Idx].StartAddr = _DUMMYADDR;	//clear the specific tag
			MemTag[Idx].Size = 0x0000;		//clear the specific tag
			MemTag[Idx].Status = 0;
			break;
	}
}
#endif

/*=======================================================================*/
/**
 * drvif_memory_init
 * Init Memory control module. Collect the register, which should be insert only once.
 *
 * @param  { none }
 * @return { none }
 *
 * @Note
 * Mclk is separated into 32 phases
 */
//void __init drvif_memory_init(void)
 #ifndef BUILD_QUICK_SHOW

void drvif_memory_init(void)
{
#ifdef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY
	unsigned char i;
	for ( i=MEMIDX_VDC ; i<MEMIDX_ALL ; ++i ) {	// clear all tag information
		MemTag[i].Allocate_StartAddr = 0;
		MemTag[i].StartAddr = _DUMMYADDR;
		MemTag[i].Size = 0;
		MemTag[i].Status = UNDEFINED;
	}
#else

	unsigned int address=0;
	unsigned int size=0;
	//unsigned int  TotalSize;
	//unsigned char				*m_pNonCacheStartAddr;

	SCALERDRV_ALLOCBUFFER_INFO stAllocBufferInfo;
	if(!drvif_scaler_getValidMemInfo(&stAllocBufferInfo)){
		rtd_pr_scaler_memory_debug("fail to get memory information\n");
		return;
	}

	AllocStartAddrBlock[MEM_BLOCK_USE_NORMAL] = stAllocBufferInfo.normal_mem_addr;
	AllocEndAddrBlock[MEM_BLOCK_USE_NORMAL] = AllocStartAddrBlock[MEM_BLOCK_USE_NORMAL] + stAllocBufferInfo.normal_mem_size;

	AllocStartAddrBlock[MEM_BLOCK_USE_OPTIMIZE] = stAllocBufferInfo.optimize_mem_addr;
	AllocEndAddrBlock[MEM_BLOCK_USE_OPTIMIZE] = AllocStartAddrBlock[MEM_BLOCK_USE_OPTIMIZE] + stAllocBufferInfo.optimize_mem_size;

	address = stAllocBufferInfo.normal_mem_addr;
	size = stAllocBufferInfo.normal_mem_size;

	AllocStartAddr = address;
	AllocEndAddr = AllocStartAddr  + size;

	drvif_memory_free_block(MEMIDX_ALL);

	//Disable channel1, channel2 field merge function
	IoReg_ClearBits(MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_ClearBits(MDOMAIN_CAP_Cap_In2_enable_reg, _BIT0);
#endif

	//frank@0121 Disable function display main,  down mask window, display sub, MP function
	//if(Scaler_Get_Keep_In_BootUp_Mode_flag()==FALSE)
		IoReg_ClearBits(MDOMAIN_DISP_Disp_main_enable_reg, _BIT0);
	IoReg_ClearBits(MDOMAIN_DISP_DDR_MainMaskCtrl_reg,_BIT0);

#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_ClearBits(MDOMAIN_DISP_DDR_SubCtrl_reg, _BIT0);
	IoReg_ClearBits(MDOMAIN_DISP_DDR_SubMPMode_reg,_BIT0);
#endif

	drvif_memory_set_dbuffer_write();

#ifdef VBY_ONE_PANEL
	if((MemTag[MEMIDX_MAIN_THIRD].StartAddr!=NULL) && (MemTag[MEMIDX_MAIN_THIRD].StartAddr < AllocStartAddr))
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg, (MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0));
	else
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg, (AllocStartAddr & 0x7ffffff0));
#else
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,(AllocStartAddr & 0x7ffffff0));
#endif
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(AllocEndAddr-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);

	//set cap sub boundary. 1  unit = 8 bytes
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
//for sub cap/disp use in DCU2
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr1_reg,(AllocStartAddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_reg,(AllocEndAddr-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
#endif
#endif
	rtd_pr_scaler_memory_debug("Initial Memory is completed. \n");
}

void drvif_memory_release(void)
{
	unsigned char i;
	for ( i=MEMIDX_VDC ; i<MEMIDX_ALL ; ++i ) {	// clear all tag information
		if(drvif_memory_get_block_addr((eMemIdx)i)!=_DUMMYADDR)
			rtd_pr_scaler_memory_debug("[ERROR]Idx:%d,Memory Not be release\n", i);
	}
	drvif_scaler_releaseValidMem();
	AllocStartAddr = 0;
	AllocEndAddr = _DDR_END_ADDR;
}

void drvif_memory_reallocate(void)
{
#ifdef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY
	return;
#else
	unsigned int address=0;
	unsigned int size=0;
	SCALERDRV_ALLOCBUFFER_INFO stAllocBufferInfo;

#if 0
	if(!drvif_scaler_getValidMemInfo(&address, &size)){
		rtd_pr_scaler_memory_debug("fail to get memory information\n");
		return;
	}
#else
	if(!drvif_scaler_getValidMemInfo(&stAllocBufferInfo)){
		rtd_pr_scaler_memory_debug("fail to get memory information\n");
		return;
	}

	AllocStartAddrBlock[MEM_BLOCK_USE_NORMAL] = stAllocBufferInfo.normal_mem_addr;
	AllocEndAddrBlock[MEM_BLOCK_USE_NORMAL] = AllocStartAddrBlock[MEM_BLOCK_USE_NORMAL] + stAllocBufferInfo.normal_mem_size;

	AllocStartAddrBlock[MEM_BLOCK_USE_OPTIMIZE] = stAllocBufferInfo.optimize_mem_addr;
	AllocEndAddrBlock[MEM_BLOCK_USE_OPTIMIZE] = AllocStartAddrBlock[MEM_BLOCK_USE_OPTIMIZE] + stAllocBufferInfo.optimize_mem_size;

	address = stAllocBufferInfo.normal_mem_addr;
	size = stAllocBufferInfo.normal_mem_size;

#endif
	AllocStartAddr = address;
	AllocEndAddr = AllocStartAddr  + size;
#ifndef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY
	//set capture upper boundary is end address -512 size,avoid capture fail address
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg, (AllocStartAddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(AllocEndAddr-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);

	//set cap sub boundary. 1  unit = 8 bytes
#ifdef CONFIG_DUAL_CHANNEL
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr1_reg,(AllocStartAddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_reg,(AllocEndAddr-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
#endif
#endif
#endif//#ifdef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY

}



 int drvif_scaler_getValidMemInfo_fromVideo(SCALERDRV_ALLOCBUFFER_INFO *pAllocBufferInfo)
{
	int ret=0;
	unsigned int i = 0, ulAllocBufInfoItemCount = 0;
	unsigned int *pulTemp;
	SCALERDRV_ALLOCBUFFER_INFO stAllocBufferInfo;

	memset(pAllocBufferInfo, 0, sizeof(SCALERDRV_ALLOCBUFFER_INFO));
	memset(&stAllocBufferInfo, 0, sizeof(SCALERDRV_ALLOCBUFFER_INFO));
	// send RPC to request allocate buffer address
	if ((ret = Scaler_SendRPC(SCALERIOC_ALLOBUFFER,0,0)) < 0)
	{
		rtd_pr_scaler_memory_debug("[IDMA] ret=%d, get alloc buffer address failed!!!\n", ret );
		pAllocBufferInfo->normal_mem_addr = 0;
		pAllocBufferInfo->normal_mem_size = 0;
		pAllocBufferInfo->optimize_mem_addr = 0;
		pAllocBufferInfo->optimize_mem_size = 0;
	}else{
		// copy from RPC share memory
		memcpy(&stAllocBufferInfo, (unsigned char*)Scaler_GetShareMemVirAddr(SCALERIOC_ALLOBUFFER), sizeof(SCALERDRV_ALLOCBUFFER_INFO));

		// change endian
		ulAllocBufInfoItemCount = sizeof(SCALERDRV_ALLOCBUFFER_INFO) / sizeof(unsigned int);
		pulTemp = (unsigned int *)&stAllocBufferInfo;
		for (i = 0; i < ulAllocBufInfoItemCount; i++)
		        pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);

		pAllocBufferInfo->normal_mem_addr = stAllocBufferInfo.normal_mem_addr;
		pAllocBufferInfo->normal_mem_size = stAllocBufferInfo.normal_mem_size;
		pAllocBufferInfo->optimize_mem_addr = stAllocBufferInfo.optimize_mem_addr;
		pAllocBufferInfo->optimize_mem_size = stAllocBufferInfo.optimize_mem_size; // reserve 8 bytes for FRC memory issue
	}

	rtd_pr_scaler_memory_debug("[IDMA][RPC][Addr:%x][Size:%d KB]\n", pAllocBufferInfo->normal_mem_addr, pAllocBufferInfo->normal_mem_size >> 10);
	rtd_pr_scaler_memory_debug("[IDMA][RPC][Addr_opt:%x][Size_opt:%d KB]\n", pAllocBufferInfo->optimize_mem_addr, pAllocBufferInfo->optimize_mem_size >> 10);

	return ret;
}


//#define nonCacheMem(a)	(a+DEF_MEM_SIZE)
unsigned char drvif_scaler_getValidMemInfo(SCALERDRV_ALLOCBUFFER_INFO *pAllocBufferInfo)
{
	//int ret;
	//unsigned int ulAllocBufInfoItemCount = 0;
	//unsigned int *pulTemp;
	//SCALERDRV_ALLOCBUFFER_INFO stAllocBufferInfo;
#ifdef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY
	return FALSE;
#else
	//unsigned char		*m_pNonCacheStartAddr;
	unsigned long 		m_PhyStartAddr;

	if (pAllocBufferInfo == NULL)
		return FALSE;

	pAllocBufferInfo->normal_mem_addr = 0;
	pAllocBufferInfo->normal_mem_size = 0;
	pAllocBufferInfo->optimize_mem_addr = 0;
	pAllocBufferInfo->optimize_mem_size = 0;

	if (pAllocBufferInfo->normal_mem_size == 0 || pAllocBufferInfo->optimize_mem_addr == 0) // if video fw doesn't give the valid memory address
	{
#if 0
		// alloc memory
		m_PhyStartAddr = pli_allocOverlappedMemory(mMallocSize);
		if(m_PhyStartAddr == 0){
			m_pCacheStartAddr = (BYTE *)pli_allocContinuousMemorySpecific(mMallocSize, (char**)&m_pNonCacheStartAddr, &m_PhyStartAddr, GFP_DCU1);
			if(m_pCacheStartAddr == NULL){
				rtd_pr_scaler_memory_debug("[ERROR]Allocate DCU1 size=%x fail\n",mMallocSize);
				return FALSE;
			}
		}
#else
		m_pCacheStartAddr = dvr_malloc(mMallocSize);
		//m_PhyStartAddr = (unsigned int)virt_to_phys((void*)m_pCacheStartAddr);
		m_PhyStartAddr = (unsigned int)dvr_to_phys((void*)m_pCacheStartAddr);
		if(m_PhyStartAddr == 0){
				rtd_pr_scaler_memory_debug("[ERROR]Allocate DCU1 size=%x fail\n",mMallocSize);
				return FALSE;
		}

#endif
		pAllocBufferInfo->normal_mem_addr = (unsigned int)m_PhyStartAddr;
		pAllocBufferInfo->normal_mem_size = mMallocSize;
		pAllocBufferInfo->optimize_mem_addr = (unsigned int)m_PhyStartAddr;
		pAllocBufferInfo->optimize_mem_size = mMallocSize;//0x1000000; // 16MB

		rtd_pr_scaler_memory_debug("[SLR_MEMORY3][PLI][Cache:%x][Phy:%x][Size:%x]\n", (unsigned int)m_pCacheStartAddr, (unsigned int)m_PhyStartAddr, mMallocSize);
		rtd_pr_scaler_memory_debug("[SLR_MEMORY3][PLI][Addr:%x][Size:%x]\n", pAllocBufferInfo->normal_mem_addr, pAllocBufferInfo->normal_mem_size);
		rtd_pr_scaler_memory_debug("[SLR_MEMORY3][PLI][Addr_opt:%x][Size_opt:%x]\n", pAllocBufferInfo->optimize_mem_addr, pAllocBufferInfo->optimize_mem_size);
		//rtd_pr_scaler_memory_debug("[SLR_MEMORY3][PLI][nonCache:%x]\n", (unsigned int)m_pNonCacheStartAddr);
	}
	//scaler_mem_virtualuncacheAddr = nonCacheMem(pAllocBufferInfo->normal_mem_addr);

	return TRUE;
#endif
}

unsigned char drvif_scaler_releaseValidMem(void)
{
#ifdef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY
	drvif_memory_free_block(MEMIDX_ALL);
#else
#if 0

	if(m_pCacheStartAddr == NULL){
		if(m_PhyStartAddr != 0){
			pli_freeOverlappedMemory(m_PhyStartAddr);
		}else{
			return FALSE;
		}
	}else{
		pli_freeContinuousMemory(m_pCacheStartAddr);
		m_pCacheStartAddr = NULL;
	}
#else
	if(m_pCacheStartAddr == NULL){
		return FALSE;
	}
	else
	{
		dvr_free((void *)m_pCacheStartAddr);
		m_pCacheStartAddr = NULL;
	}
#endif
#endif

	return TRUE;
}

#if 0
static unsigned int drv_memory_get_mem_remap_addr(unsigned int phyAddr, unsigned int length)
{
	int mem_fd;
	volatile void* pMem;
	unsigned int result = 0;
	if((mem_fd=open(DEV_MEM, O_RDWR)) < 0){
		rtd_pr_scaler_memory_debug("Err: can't open DEV_MEM.\n");
		return 0;
	}

	pMem = mmap(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, phyAddr);

	if(pMem == MAP_FAILED){
		rtd_pr_scaler_memory_debug("Err: can't get the mmap adress pointer\n");
		close(mem_fd);
		return 0;
	}

	result = (unsigned int)pMem;
	close(mem_fd);
	return result;
}
#endif
#if 0
unsigned int get_scaler_memory_map_addr()
{
#if 1
	return drv_memory_get_mem_remap_addr(AllocStartAddr,  AllocEndAddr - AllocStartAddr);
#else
	return scaler_mem_virtualAddr;
#endif
}
#endif

unsigned char drvif_memory_switch_mem_block(eMemBlockUsage mem_block_usage)
{
	if (mem_block_usage >= MEM_BLOCK_TOTAL_NUM)
	{
		rtd_pr_scaler_memory_debug("[SLR_MEMORY][unknown mem block][%x~%x]\n", AllocStartAddr, AllocEndAddr);
		return FALSE;
	}

	AllocStartAddr = AllocStartAddrBlock[mem_block_usage];
	AllocEndAddr = AllocEndAddrBlock[mem_block_usage];
#ifndef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY
	//set cap main boundary. 1  unit = 8 bytes
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg, (AllocStartAddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(AllocEndAddr-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);

	//set cap sub boundary. 1  unit = 8 bytes
#ifdef CONFIG_DUAL_CHANNEL
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr1_reg,(AllocStartAddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_reg,(AllocEndAddr-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
#endif
#endif

	rtd_pr_scaler_memory_debug("[SLR_MEMORY][switch to mem block %d][%x~%x](%d MB)\n", mem_block_usage, AllocStartAddr, AllocEndAddr, (AllocEndAddr-AllocStartAddr)>>20);
	return TRUE;
}
#if 0
unsigned int get_scaler_memory_map_uncacheaddr()
{
	return scaler_mem_virtualuncacheAddr;
}
#endif
#endif

unsigned int get_scaler_memory_base_addr(void)
{
	return AllocStartAddr;

	}

unsigned int memory_get_line_size(unsigned int LineSize, unsigned char InputFormat, unsigned char BitNum)
{

	if(InputFormat){
		if(BitNum) {		// 4:4:4 format     10bit
			LineSize = drvif_memory_get_data_align(LineSize *30, 64);
			LineSize = (unsigned int)SHR(LineSize, 6);	// (pixels of per line * 30 / 64), unit: 64bits
			rtd_pr_scaler_memory_debug("\n 444_10 bits !");
		} else {			// 4:4:4 format     8bit
			LineSize = drvif_memory_get_data_align(LineSize*24, 64);
			LineSize = (unsigned int)SHR(LineSize, 6);	// (pixels of per line * 24 / 64), unit: 64bits
			rtd_pr_scaler_memory_debug("\n 444_8 bits !");
		}
	}else{
		if(BitNum) {		// 4:2:2 format     10bit
			LineSize = drvif_memory_get_data_align(LineSize*20, 64);
			LineSize = (unsigned int)SHR(LineSize, 6);	// (pixels of per line * 20 / 64), unit: 64bits
			rtd_pr_scaler_memory_debug("\n 422_10 bits !");
		} else {			// 4:2:2 format     8bit
			LineSize = drvif_memory_get_data_align(LineSize*16, 64);
			LineSize = (unsigned int)SHR(LineSize, 6);	// (pixels of per line * 16 / 64), unit: 64bits
			rtd_pr_scaler_memory_debug("\n 422_8 bits !");
		}
	}
	rtd_pr_scaler_memory_debug("memory_get_line_size LineSize=%d\n",LineSize);
	return LineSize;
}


/*============================================================================*/
/**
 * memory_get_capture_size
 * Calculate the memory size of capture. The size is counted by 4:2:2/4:4:4 data format,
 * and frame-access/line access.
 *
 * @param <info> { disp-info structure }
 * @param <AccessType> { Access_Type  }
 * @param <TotalSize> { size of capture }
 * @return { none }
 *
 */
unsigned int memory_get_capture_size(unsigned char display, eMemCapAccessType AccessType)
{
	unsigned int TotalSize;
	unsigned char ddrbit;
	unsigned int capLen = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_CAP_LEN);
	unsigned int capWid = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID);
	unsigned char channel = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CHANNEL);

	rtd_pr_scaler_memory_debug("10 bit:%d;;422cap:%d\n",(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_10BIT)),(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_422CAP)));

	ddrbit =  (channel ? 0 : ((Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_10BIT)) << 1 ))  | ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_422CAP));
	TotalSize = memory_get_line_size(capWid, (ddrbit & _BIT0), ((ddrbit & _BIT1) >> 1));
#ifdef CONFIG_I2RND_ENABLE
	//i2rnd sub-cap to VO, line step has to be 2-align @Crixus 20160728
	total_line_size = TotalSize;
#endif
	TotalSize = drvif_memory_get_data_align(TotalSize, 0x04);	// it must be times of 4

	if (AccessType == MEMCAPTYPE_FRAME)
		TotalSize = TotalSize * capLen;	// multiple total_lines if frame-access
	//rtd_pr_scaler_memory_debug("[Memory-cap]:TotalSize=%x,capWid=%x,10Bit=%d,422=%d\n", TotalSize,capWid,Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_10BIT),Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_422CAP));
	rtd_pr_scaler_memory_debug("[crixus]:TotalSize=%x,capWid=%x,capLen=%x,10Bit=%d,422=%d\n", TotalSize,capWid,capLen,Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_10BIT),Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_422CAP));
	return TotalSize;
}




unsigned int memory_get_capture_line_size_compression(unsigned char compression_bits)
{
	unsigned int TotalSize = 0;
	unsigned int capWid = Scaler_DispGetInputInfoByDisp(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), SLR_INPUT_CAP_WID);


	rtd_pr_scaler_memory_debug("compression_bits=%x, capWid=%x\n", compression_bits, capWid);

	//width need to align 32 for new PQC @Crixus 20170615
	if((capWid % 32) != 0)
		capWid = capWid + (32 - (capWid % 32));


	//rtd_pr_scaler_memory_emerg("[crixus]capWid div32 = %d\n", capWid);
	TotalSize = capWid * compression_bits + 256;
	//rtd_pr_scaler_memory_emerg("[crixus]TotalSize compress = %d\n",TotalSize);
	TotalSize = drvif_memory_get_data_align(TotalSize, 64);// (pixels of per line * bits / 64), unit: 64bits
	TotalSize = (unsigned int)SHR(TotalSize, 6);

	rtd_pr_scaler_memory_debug("[crixus]memory_get_capture_line_size_compression LineSize=%d\n",TotalSize);

	//TotalSize = memory_get_line_size(capWid, (ddrbit & _BIT0), ((ddrbit & _BIT1) >> 1));
	//TotalSize = drvif_memory_get_data_align(TotalSize, 0x04);	// it must be times of 4
	//rtd_pr_scaler_memory_emerg("[crixus]:TotalSize=%x,capWid=%x,capLen=%x\n", TotalSize,capWid,capLen);

	return TotalSize;
}

unsigned int memory_get_capture_line_size_compression_by_width(unsigned int capWid, unsigned char compression_bits)
{
	unsigned int TotalSize;

	//width need to align 32 for new PQC @Crixus 20170615
	if((capWid % 32) != 0)
		capWid = capWid + (32 - (capWid % 32));

	//rtd_pr_scaler_memory_emerg("[crixus]capWid div32 = %d\n", capWid);
	TotalSize = capWid * compression_bits + 256;
	//rtd_pr_scaler_memory_emerg("[crixus]TotalSize compress = %d\n",TotalSize);
	TotalSize = drvif_memory_get_data_align(TotalSize, 64);// (pixels of per line * bits / 64), unit: 64bits
	TotalSize = (unsigned int)SHR(TotalSize, 6);

	//rtd_pr_scaler_memory_emerg("[crixus]memory_get_line_size LineSize=%d\n",TotalSize);

	//TotalSize = memory_get_line_size(capWid, (ddrbit & _BIT0), ((ddrbit & _BIT1) >> 1));
	//TotalSize = drvif_memory_get_data_align(TotalSize, 0x04);	// it must be times of 4
	//rtd_pr_scaler_memory_emerg("[crixus]:TotalSize=%x,capWid=%x,capLen=%x\n", TotalSize,capWid,capLen);
	return TotalSize;
}

unsigned int memory_get_capture_frame_size_compression(unsigned char compression_bits)
{
	unsigned int TotalSize;
	unsigned int capLen = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_CAP_LEN);
	unsigned int capWid = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_CAP_WID);

	//rtd_pr_scaler_memory_emerg("[crixus]capWid div32 = %d\n", capWid);
	TotalSize = capWid * compression_bits;
	//rtd_pr_scaler_memory_emerg("[crixus]TotalSize compress = %d\n",TotalSize);
	TotalSize = drvif_memory_get_data_align(TotalSize, 64);// (pixels of per line * bits / 64), unit: 64bits
	TotalSize = TotalSize * capLen;

	//rtd_pr_scaler_memory_emerg("[crixus]memory_get_line_size LineSize=%d\n",TotalSize);
	//rtd_pr_scaler_memory_emerg("[crixus]:TotalSize=%x,capWid=%x,capLen=%x\n", TotalSize,capWid,capLen);
	return TotalSize;
}

 #ifndef BUILD_QUICK_SHOW

#ifndef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY

static void memory_check_enough(void)
{
	unsigned int i = 0, sum_size = 0;
	for(i = MEMIDX_VDC ; i<MEMIDX_ALL ; i++) {
		if(MemTag[i].StartAddr != _DUMMYADDR){
			if(i != MEMIDX_DI)	// DI use another memory area, skip check
				sum_size += MemTag[i].Size;
		}
	}

	if(sum_size > (AllocEndAddr-AllocStartAddr)){
		rtd_pr_scaler_memory_debug("\n Scaler Memory Allocate ERROR!!(Overflow Memory Size),Start =%x,End=%x\n",AllocStartAddr,AllocEndAddr);
		for(i = MEMIDX_VDC ; i<MEMIDX_ALL ; i++) {
			rtd_pr_scaler_memory_debug("Allocate Index %d, StartAddress = %x, Size=%d\n", i, MemTag[i].StartAddr, MemTag[i].Size);
		}
	}
}


/*============================================================================*/
/**
 * memory_set_mem_tag
 * Set start address and size of a specific memory tag
 *
 * @param <index> { index of memory tag }
 * @param <space> { size }
 * @return { none }
 *
 */
static void memory_set_mem_tag(eMemIdx Idx, unsigned int StartAddr, unsigned int Space)
{
	//unsigned char	*m_pNonCacheStartAddr;
	if(MemTag[Idx].Status == ALLOCATED_FROM_HIGH)
		MemTag[Idx].StartAddr = (StartAddr & ~((1 << _DDR_BITS) - 1));
	else
		MemTag[Idx].StartAddr = drvif_memory_get_data_align(StartAddr, (1 << _DDR_BITS));

	if(MEMIDX_DI == Idx){
		IoReg_Write32(DI_DI1_WRRD_BOUNDARYADDR1_VADDR, (MemTag[Idx].StartAddr & 0x7ffffff0));
#if 1
		//patch for DI/RTNR bug(overwrite 512Byte)
		IoReg_Write32(DI_DI1_WRRD_BOUNDARYADDR2_VADDR, (MemTag[Idx].StartAddr+Space-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
#else
		IoReg_Write32(DI_DI1_WRRD_BOUNDARYADDR2_VADDR, (MemTag[Idx].StartAddr+Space) & 0x7ffffff0);
#endif
	}
	MemTag[Idx].Size = Space;
	//Add below code to check memory allocate not enough check
	memory_check_enough();
}
#endif
#endif
/*============================================================================*/
/**
 * drvif_memory_free_block
 * Free memory of the specific tag and insert it to free block information
 *
 * @param <index> { index of DDR memory space, see eMemIdx for details }
 * @return { none }
 *
 */
void drvif_memory_free_block(eMemIdx Idx)
{

	unsigned char i;
#ifdef PLI_FAIL
	return;
#endif

#if 1 // do not free scaler mem for vo smooth toggle

	#if 0
	unsigned char display;
	if (Idx >= MEMIDX_SUB && Idx <= MEMIDX_SUB_THIRD) {
		display = SLR_SUB_DISPLAY;
	} else {
		display = SLR_MAIN_DISPLAY;
	}
	#endif
#if 0
	if((Scaler_InputSrcGetType(display) == _SRC_VO)&&(Get_vo_smoothtoggle_changeratio_flag(display)||Get_vo_smoothtoggle_timingchange_flag(display)))
	{
		rtd_pr_scaler_memory_debug("[smooth toggle]do not free %d memory\n",Idx);
		return;
	}
#endif
#endif

#ifdef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY
	if ( (Idx>=MEMIDX_VDC) && (Idx<MEMIDX_ALL) ) {
		if(Idx == MEMIDX_DI){
#ifdef CONFIG_I2RND_ENABLE
			if((display == SLR_MAIN_DISPLAY)&&(check_Sub_I2rnd_done() == TRUE)&&(Scaler_I2rnd_get_timing_enable() == _ENABLE))
				;
			else
#endif
			{
#if 0	/* DI use rolling mode, can't disable DI*/
				// Close IP enable
				di_im_di_control_RBUS im_di_control_reg;
				im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
				im_di_control_reg.ip_enable= 0;
				im_di_control_reg.write_enable_8 = 1;
				IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
#endif
			}
			//IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT28, 0);

#ifndef ENABLE_SMOOTH_TOGGLE_PR3D
			//IoReg_ClearBits(BUS_TV_SB2_DCU1_ARB_CR1_VADDR, _BIT0);
			// avoid DC_SYS2 client separate enable fail issue due to DI send zero write command to bus
			drv_memory_disable_di_write_req(TRUE);
#endif
		}else if(Idx == MEMIDX_MAIN){
			//disable main Mdomain cature
			IoReg_ClearBits(MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);
#if 0 //frank@0903 reduce scaler time
			//frank@0503 add below delay to confirm M capture have disable
			msleep(20);
#endif
		}else if(Idx == MEMIDX_SUB){
			//disable sub Mdomain cature
			//Crixus@20140722 - Mac3 no in2
			//IoReg_ClearBits(MDOMAIN_CAP_Cap_In2_enable_reg, _BIT0);
#if 0 //frank@0903 reduce scaler time
			//frank@0503 add below delay to confirm M capture have disable
			msleep(20);
#endif
		}

		if(MemTag[Idx].StartAddr != _DUMMYADDR){
			#if 0
			if((MemTag[Idx].StartAddr >= FBOUS_ADDRESS_LOW)&&(MemTag[Idx].StartAddr <= FBOUS_ADDRESS_HIGH)){//FBUS region
				if(MemTag[Idx].Allocate_StartAddr != 0)
					pli_freeOverlappedMemory(MemTag[Idx].Allocate_StartAddr);
			}else{
				if(MemTag[Idx].Allocate_StartAddr != 0)
					pli_freeContinuousMemory((BYTE *)MemTag[Idx].Allocate_StartAddr);
			}
			#else

			if(Idx == MEMIDX_DI){
//DI,RTNR use reserved memory, no need to free memory @Crixus 20170505
#ifndef CONFIG_MDOMAIN_RESERVED_MEMORY
					if(MemTag[Idx].Allocate_StartAddr != 0)
						free_overlapped_memory((unsigned long)MemTag[Idx].Allocate_StartAddr);
#endif
			}else{
				if((MemTag[Idx].Allocate_StartAddr != 0) && (MemTag[Idx].Allocate_VirAddr != 0))
					dvr_free((void *)MemTag[Idx].Allocate_VirAddr);
			}

			#endif
			rtd_pr_scaler_memory_debug("[SCALER]FREE address %x\n",MemTag[Idx].Allocate_StartAddr);
			MemTag[Idx].Allocate_StartAddr = 0;
			MemTag[Idx].StartAddr = _DUMMYADDR;
			MemTag[Idx].Size = 0;
			MemTag[Idx].Status = UNDEFINED;
			MemTag[Idx].Allocate_VirAddr = 0;
		}
	} else if ( Idx == MEMIDX_ALL ) {
		for ( i=MEMIDX_VDC ; i<MEMIDX_ALL ; ++i ) {	// clear all tag information
			if(i == MEMIDX_DI){
#if 0	/* DI use rolling mode, can't disable DI*/
				// Close IP enable
				di_im_di_control_RBUS im_di_control_reg;
				im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
				im_di_control_reg.ip_enable= 0;
				im_di_control_reg.write_enable_8 = 1;
				IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
#endif
				//IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT28, 0);
#ifndef ENABLE_SMOOTH_TOGGLE_PR3D
				//IoReg_ClearBits(BUS_TV_SB2_DCU1_ARB_CR1_VADDR, _BIT0);
				// avoid DC_SYS2 client separate enable fail issue due to DI send zero write command to bus
				drv_memory_disable_di_write_req(TRUE);
#endif
			}else if(i == MEMIDX_MAIN){
				//disable main Mdomain cature
				IoReg_ClearBits(MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);
			}else if(i == MEMIDX_SUB){
				//disable sub Mdomain cature
				//Crixus@20140722 - Mac3 no in2
				//IoReg_ClearBits(MDOMAIN_CAP_Cap_In2_enable_reg, _BIT0);
			}

			if(MemTag[i].StartAddr != _DUMMYADDR){
				#if 0
				if((MemTag[Idx].StartAddr >= FBOUS_ADDRESS_LOW)&&(MemTag[Idx].StartAddr <= FBOUS_ADDRESS_HIGH)){//FBUS region
					if(MemTag[Idx].Allocate_StartAddr != 0)
						pli_freeOverlappedMemory(MemTag[Idx].Allocate_StartAddr);
				}else{
					if(MemTag[Idx].Allocate_StartAddr != 0)
						pli_freeContinuousMemory((BYTE *)MemTag[Idx].Allocate_StartAddr);
				}
				#else
				if((MemTag[i].Allocate_StartAddr != 0) && (MemTag[i].Allocate_VirAddr != 0))
					dvr_free((void *)MemTag[i].Allocate_VirAddr);
				#endif
				rtd_pr_scaler_memory_debug("[SCALER]FREE address %x\n",MemTag[i].Allocate_StartAddr);
				MemTag[i].Allocate_StartAddr = 0;
				MemTag[i].StartAddr = _DUMMYADDR;
				MemTag[i].Size = 0;
				MemTag[i].Status = UNDEFINED;
				MemTag[i].Allocate_VirAddr = 0;
			}
		}
	}
#else
	if ( (Idx>=MEMIDX_VDC) && (Idx<MEMIDX_ALL) ) {
		MemTag[Idx].StartAddr = _DUMMYADDR;
		MemTag[Idx].Size = 0;
		MemTag[Idx].Status = UNDEFINED;
	} else if ( Idx == MEMIDX_ALL ) {
		for ( i=MEMIDX_VDC ; i<MEMIDX_ALL ; ++i ) {	// clear all tag information
			MemTag[i].StartAddr = _DUMMYADDR;
			MemTag[i].Size = 0;
			MemTag[i].Status = UNDEFINED;
		}
	}
#endif
}

/*============================================================================*/
/**
 * MemGetBlockAddr
 * Get start address of allocated memory
 *
 * @param <index> { index of DDR memory space, see eMemIdx for details }
 * @return { none }
 *
 */
unsigned int drvif_memory_get_block_addr(eMemIdx Idx)
{
	return MemTag[Idx].StartAddr;
}

/*============================================================================*/
/**
 * drvif_memory_get_block_size
 * Get size of allocated memory
 *
 * @param <index> { index of DDR memory space, see eMemIdx for details }
 * @return { block size }
 *
 */
unsigned int drvif_memory_get_block_size(eMemIdx Idx)
{
	return MemTag[Idx].Size;
}


unsigned long drvif_memory_get_mainblock_viraddr(void)
{
#if 0
	unsigned int virstaaddr = 0;
	unsigned int phystaaddr = 0;
	if(MemTag[MEMIDX_MAIN_THIRD].Allocate_VirAddr !=0) {
		virstaaddr = MemTag[MEMIDX_MAIN_THIRD].Allocate_VirAddr;
		phystaaddr = MemTag[MEMIDX_MAIN_THIRD].StartAddr;
	} else if (MemTag[MEMIDX_MAIN_SEC].Allocate_VirAddr !=0) {
		virstaaddr = MemTag[MEMIDX_MAIN_SEC].Allocate_VirAddr;
		phystaaddr = MemTag[MEMIDX_MAIN_SEC].StartAddr;
	} else if (MemTag[MEMIDX_MAIN].Allocate_VirAddr !=0) {
		virstaaddr = MemTag[MEMIDX_MAIN].Allocate_VirAddr;
		phystaaddr = MemTag[MEMIDX_MAIN].StartAddr;
	}

	return (virstaaddr + MemTag[MEMIDX_MAIN].StartAddr - phystaaddr);
#else
	return MemTag[MEMIDX_MAIN_THIRD].Allocate_VirAddr;
#endif
}

unsigned long drvif_memory_get_subblock_viraddr(void)
{
#if 0
	unsigned int virstaaddr = 0;
	unsigned int phystaaddr = 0;
	if(MemTag[MEMIDX_SUB_THIRD].Allocate_VirAddr !=0) {
		virstaaddr = MemTag[MEMIDX_SUB_THIRD].Allocate_VirAddr;
		phystaaddr = MemTag[MEMIDX_SUB_THIRD].StartAddr;
	} else if (MemTag[MEMIDX_SUB_SEC].Allocate_VirAddr !=0) {
		virstaaddr = MemTag[MEMIDX_SUB_SEC].Allocate_VirAddr;
		phystaaddr = MemTag[MEMIDX_SUB_SEC].StartAddr;
	} else if (MemTag[MEMIDX_SUB].Allocate_VirAddr !=0) {
		virstaaddr = MemTag[MEMIDX_SUB].Allocate_VirAddr;
		phystaaddr = MemTag[MEMIDX_SUB].StartAddr;
	}

	return (virstaaddr + MemTag[MEMIDX_SUB].StartAddr - phystaaddr);
#else
	return MemTag[MEMIDX_SUB_THIRD].Allocate_VirAddr;
#endif
}
 #ifndef BUILD_QUICK_SHOW

/*============================================================================*/
/**
 * MemAlignBank
 * Memory alignment for DDR bank0
 *
 * @param <index> { index of DDR memory space, see eMemIdx for details }
 * @return { none }
 *
 */
unsigned int drvif_memory_alignbank(unsigned int StartAddr, unsigned char AlignBank, unsigned char Frm)
{
	unsigned int getValue;
	//rtd_pr_scaler_memory_debug("1. Start Addr = %x in Memory_AlignBank\n",StartAddr);
	if(AlignBank < MEMALIGN_ANY){
		getValue = (StartAddr & ((1 << CONFIG_DDR_BANK_BITS) - 1) << (CONFIG_DDR_COL_BITS+1)) >> (CONFIG_DDR_COL_BITS+1);
		getValue = getValue % (1 << CONFIG_DDR_BANK_BITS);
		if(Frm == ALLOCATED_FROM_LOW) {
			getValue = (1 << CONFIG_DDR_BANK_BITS) - getValue + AlignBank;

			StartAddr += (getValue * (1 << (CONFIG_DDR_COL_BITS+1)));
			//rtd_pr_scaler_memory_debug("2. L.Start Addr = %x getValue = %x in Memory_AlignBank\n",StartAddr,getValue);
		}
		else if(Frm == ALLOCATED_FROM_HIGH) {
			if(getValue > AlignBank)
				getValue -= AlignBank;
			else
				getValue = AlignBank - getValue;

			StartAddr -= (getValue * (1 << (CONFIG_DDR_COL_BITS+1)));
			//rtd_pr_scaler_memory_debug("2. H.Start Addr = %x getValue = %x in Memory_AlignBank\n",StartAddr,getValue);
		}
	}

	return StartAddr;
}


#ifdef UT_flag
unsigned int g_ulsMdomainAddr[QUERY_IDX_MAX] = {0, 0, 0, 0, 0, 0, 0, 0};

void memory_set_mdomain_query_addr_for_UT(unsigned char idx, unsigned int a_ulAddr)
{
	g_ulsMdomainAddr[idx] = a_ulAddr;
}

unsigned int memory_get_mdomain_query_addr_for_UT(unsigned char idx)
{
	return g_ulsMdomainAddr[idx];
}

#endif

#endif
//k4lp start to use query driver to get memory and address @Crixus 20170504
/*
	(1)M-domain
		a. main          => ((3840  12 + 256) / 128  2160 * 16) x 3 = 35.8 MB   (reserved memory)
		b. main-i2run  => ((1920  12 + 256) / 128  1080 * 16) x 3 = 9 MB

	(2)DI/RTNR
		a. DI_1 => (4k):     25.716 MB (reserved memory)
		b. DI_2 => (1080i): 14.09 MB (compression 10 bits)
		c. DMAtable => 2MB
*/
#ifdef CONFIG_ARM64
unsigned long get_query_start_address(unsigned char idx){
#else
unsigned int get_query_start_address(unsigned char idx){
#endif
	unsigned long addr = 0 ;
	unsigned long size = 0 ;
	unsigned long buffer_size_main = MDOMAIN_MAIN_BUFFER_SIZE;
#ifdef CONFIG_I2RND_ENABLE
	unsigned int buffer_size_main_i2r = MDOMAIN_MAIN_I2R_BUFFER_SIZE;
#endif
	//rtd_pr_scaler_memory_info("[%s]reserved memory start addr = 0x%lx, size = 0x%lx, MDOMAIN_MAIN_BUFFER_SIZE = 0x%lx\n", __FUNCTION__, addr, size, MDOMAIN_MAIN_BUFFER_SIZE);
	//rtd_pr_scaler_memory_info("[%s]VIP_DI_1_BUFFER_SIZE = 0x%x, VIP_DI_2_BUFFER_SIZE = 0x%x, VIP_DMAto3DTable_BUFFER_SIZE = 0x%x\n", __FUNCTION__, VIP_DI_1_BUFFER_SIZE, VIP_DI_2_BUFFER_SIZE, VIP_DMAto3DTable_BUFFER_SIZE);
	//rtd_pr_scaler_memory_info("buffer_size_main=%lx\n", buffer_size_main);
	if(get_platform_model() == PLATFORM_MODEL_2K) {
		buffer_size_main = 0;
	}
#ifdef CONFIG_I2RND_ENABLE
	if(idx == QUERY_IDX_MDOMAIN_MAIN){
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
		size = (unsigned int)carvedout_buf_query_secure(CARVEDOUT_SCALER_MDOMAIN, (void **)&addr) ;
#else
	 	size = (unsigned int)carvedout_buf_query(CARVEDOUT_SCALER_MDOMAIN, (void **)&addr) ;
#endif

		buffer_size_main = drvif_memory_get_data_align(buffer_size_main, DMA_SPEEDUP_ALIGN_VALUE);

		buffer_size_main = PAGE_ALIGN(buffer_size_main);
		if(size == 0){
			BUG();
	 	}
 	}
	else if(idx == QUERY_IDX_MDOMAIN_MAIN_I2R){
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
		size = (unsigned int)carvedout_buf_query_secure(CARVEDOUT_SCALER_MDOMAIN, (void **)&addr) ;
#else
	 	size = (unsigned int)carvedout_buf_query(CARVEDOUT_SCALER_MDOMAIN, (void **)&addr) ;
#endif

		buffer_size_main = drvif_memory_get_data_align(buffer_size_main, DMA_SPEEDUP_ALIGN_VALUE);

		buffer_size_main = PAGE_ALIGN(buffer_size_main);
		if(size == 0){
			BUG();
	 	}

		addr = addr + buffer_size_main * 3;
	}
	else if(idx == QUERY_IDX_DI){
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
		size = (unsigned int)carvedout_buf_query_secure(CARVEDOUT_SCALER_DI_NR, (void **)&addr) ;
#else
	 	size = (unsigned int)carvedout_buf_query(CARVEDOUT_SCALER_DI_NR, (void **)&addr) ;
#endif

		if(size == 0){
			BUG();
	 	}
 	}
	else if(idx == QUERY_IDX_DI_2){
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
		size = (unsigned int)carvedout_buf_query_secure(CARVEDOUT_SCALER_DI_NR, (void **)&addr) ;
#else
	 	size = (unsigned int)carvedout_buf_query(CARVEDOUT_SCALER_DI_NR, (void **)&addr) ;
#endif
		if (get_platform_model() == PLATFORM_MODEL_2K)
			addr = (unsigned int)(addr + VIP_DI_1_BUFFER_SIZE_2K_PLATFORM);
		else
			addr = (unsigned int)(addr + VIP_DI_1_BUFFER_SIZE);
		if(size == 0){
			BUG();
	 	}
	} else if (idx == QUERY_IDX_VIP_DMAto3DTABLE) {
      		size = carvedout_buf_query(CARVEDOUT_SCALER_VIP, (void **)&addr);
		if (size == 0)
            		BUG();
	} else if (idx == QUERY_IDX_VIP_DeMura) {
     		size = carvedout_buf_query(CARVEDOUT_SCALER_VIP, (void **)&addr);
        	if (size == 0)
            	BUG();
		addr = (unsigned int)(addr + VIP_DMAto3DTable_BUFFER_SIZE);
 	}

#else
	if(idx == QUERY_IDX_MDOMAIN_MAIN){
  		//do nothing
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
		size = (unsigned int)carvedout_buf_query_secure(CARVEDOUT_SCALER_MDOMAIN, (void **)&addr) ;
#else
		size = (unsigned int)carvedout_buf_query(CARVEDOUT_SCALER_MDOMAIN, (void **)&addr) ;
#endif

#ifdef UT_flag
		addr = memory_get_mdomain_query_addr_for_UT(QUERY_IDX_MDOMAIN_MAIN);
#endif


		buffer_size_main = drvif_memory_get_data_align(buffer_size_main, DMA_SPEEDUP_ALIGN_VALUE);

		buffer_size_main = PAGE_ALIGN(buffer_size_main);
		if(size == 0){
			if(get_platform_model() == PLATFORM_MODEL_2K)
				return 0;
			else
				BUG();
	 	}

		#ifdef HDMI_NO_PQDC_TEST
			#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
			size = (unsigned int)carvedout_buf_query_secure(CARVEDOUT_SCALER_MEMC, (void **)&addr) ;
			#else
			size = (unsigned int)carvedout_buf_query(CARVEDOUT_SCALER_MEMC, (void **)&addr) ;
			#endif
			rtd_pr_scaler_memory_emerg("	HDMI_NO_PQDC_TEST  m-domain use CARVEDOUT_SCALER_MEMC size=%lx, addr=%lx\n", size, addr);
		#endif
 	} else if(idx == QUERY_IDX_DI) {
		#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
		size = (unsigned int)carvedout_buf_query_secure(CARVEDOUT_SCALER_DI_NR, (void **)&addr) ;
		#else
		size = (unsigned int)carvedout_buf_query(CARVEDOUT_SCALER_DI_NR, (void **)&addr) ;
		#endif

		if(size == 0){
			BUG();
	 	}
 	} else if (idx == QUERY_IDX_VIP_NN) {
		size = (unsigned int)carvedout_buf_query(CARVEDOUT_SCALER_NN, (void **)&addr) ;

		if(size == 0){
			//BUG(); // k6lp does not support NN, size = 0, the NN driver will handle the error to disable NN.
	 	}

	} else if (idx == QUERY_IDX_VIP_DMAto3DTABLE) {
      	size = carvedout_buf_query(CARVEDOUT_SCALER_VIP, (void **)&addr);

		if (size == 0)
            		BUG();
	} else if (idx == QUERY_IDX_VIP_DeMura) {
		size = carvedout_buf_query(CARVEDOUT_SCALER_VIP, (void **)&addr);
		if (size == 0)
            		BUG();

		addr = (unsigned int)(addr + VIP_DMAto3DTable_BUFFER_SIZE);
 	}
    else if(idx == QUERY_IDX_I3DDMA)
    {
        size = carvedout_buf_query(CARVEDOUT_VDEC_VBM, (void **)&addr);
    }
#endif
	if (idx != QUERY_IDX_VIP_DMAto3DTABLE)
		rtd_pr_scaler_memory_info("[%s]idx = %d, addr = 0x%lx\n", __FUNCTION__, idx, addr);
	return addr;
}

/*============================================================================*/
/**
 * drvif_memory_alloc_block
 * allocate memory of the specific tag
 *
 * @param <index> { index of DDR memory space, see eMemIdx for details }
  * @param <space> { size of DDR memory space,uint:byte}
 * @return { start address of the allocated memory }
 *
 * @Note:
 *  You must drvif_memory_free_block to free block you allocated before calling this function to allocate memory space.
 *  It possible wrong operatoin if you don't observer the front.
 */
//extern unsigned char Get_zoom_smoothtoggle_changeratio_flag(void);
unsigned int drvif_memory_alloc_block(eMemIdx Idx, unsigned int space, unsigned char Align)
{
	unsigned int m_allocate_memory = 0;
	unsigned int buffer_size = 0;

#if 0 // do not allocate scaler mem again for vo smooth toggle
	unsigned char display;
	if (Idx >= MEMIDX_SUB && Idx <= MEMIDX_SUB_THIRD) {
		display = SLR_SUB_DISPLAY;
	} else {
		display = SLR_MAIN_DISPLAY;
	}
#if 0
	if((Get_vo_smoothtoggle_changeratio_flag(display)||Get_vo_smoothtoggle_timingchange_flag(display)))
	{
		if(MemTag[Idx].StartAddr!=_DUMMYADDR)
		{
			rtd_pr_scaler_memory_debug("[smooth toggle]do not alloc %d memory:%x\n",Idx,MemTag[Idx].StartAddr);
			return MemTag[Idx].StartAddr;
		}
		else
		{
			if(Idx==MEMIDX_MAIN_THIRD)
			{
				if((MemTag[MEMIDX_MAIN_SEC].StartAddr!=_DUMMYADDR)||(MemTag[MEMIDX_MAIN].StartAddr!=_DUMMYADDR))
				{
				     rtd_pr_scaler_memory_debug("[smooth toggle]MEMIDX_MAIN_THIRD do not alloc memory\n");
				     return MemTag[Idx].StartAddr;
				}
			} else {
				if(!Get_vo_smoothtoggle_timingchange_flag(display)) {
					rtd_pr_scaler_memory_debug("[change ratio smooth toggle]%d do not alloc memory:%x\n",Idx,MemTag[Idx].StartAddr);
					//memory protection @Crixus 20160101
					if((MemTag[Idx].StartAddr != 0x0) && (MemTag[Idx].StartAddr != 0xffffffff))
						return MemTag[Idx].StartAddr;
				}
		   }
		}
	}
#endif
#endif

#if 0
	if(Get_zoom_smoothtoggle_changeratio_flag())
	{// do not allocate DI mem again rika 20140829
		rtd_pr_scaler_memory_info("_rk [MEM] Idx[%d]=%x\n", Idx, MemTag[Idx].StartAddr);
		return MemTag[Idx].StartAddr;
	}
#endif

//	static unsigned int dcu2_address = 0;
#ifndef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY
	if(AllocStartAddr == 0)
		return _DUMMYADDR;
#endif
	//unsigned int StartAddr;
	if((Idx >= MEMIDX_VDC) && (Idx < MEMIDX_ALL)) {
#ifdef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY
			if(MEMIDX_MAIN_THIRD == Idx){
				space = drvif_memory_get_data_align(space*3+CONFIG_MDOMAIN_BURST_SIZE, DMA_SPEEDUP_ALIGN_VALUE);
			}else if(MEMIDX_MAIN_SEC == Idx){
				if(MemTag[MEMIDX_MAIN_THIRD].StartAddr != _DUMMYADDR){ //triple buffer case
					MemTag[Idx].Allocate_StartAddr = 0;
					MemTag[Idx].Size = space;
					MemTag[Idx].Status = MemTag[MEMIDX_MAIN_THIRD].Status;
					MemTag[Idx].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_THIRD].StartAddr+space, DMA_SPEEDUP_ALIGN_VALUE);
					MemTag[MEMIDX_MAIN_THIRD].Size -= space;
					#ifdef CONFIG_ARM64
					rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%lx(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
					#else
					rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%x(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
					#endif
					return MemTag[Idx].StartAddr;
				}else{ //double buffer case
					space = drvif_memory_get_data_align(space*2+CONFIG_MDOMAIN_BURST_SIZE, DMA_SPEEDUP_ALIGN_VALUE);
				}
			}else if(MEMIDX_MAIN == Idx){
				if(MemTag[MEMIDX_MAIN_THIRD].StartAddr != _DUMMYADDR){ //triple buffer case
					MemTag[Idx].Allocate_StartAddr = 0;
					MemTag[Idx].Size = space;
					MemTag[Idx].Status = MemTag[MEMIDX_MAIN_THIRD].Status;
					MemTag[Idx].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_THIRD].StartAddr+space*2, DMA_SPEEDUP_ALIGN_VALUE);
					MemTag[MEMIDX_MAIN_THIRD].Size -= space;
					#ifdef CONFIG_ARM64
					rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%lx(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
					#else
					rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%x(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
					#endif
					return MemTag[Idx].StartAddr;
				}else if(MemTag[MEMIDX_MAIN_SEC].StartAddr != _DUMMYADDR){//double buffer case
					MemTag[Idx].Allocate_StartAddr = 0;
					MemTag[Idx].Size = space;
					MemTag[Idx].Status = MemTag[MEMIDX_MAIN_SEC].Status;
					MemTag[Idx].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_SEC].StartAddr+space, DMA_SPEEDUP_ALIGN_VALUE);
					MemTag[MEMIDX_MAIN_SEC].Size -= space;
					#ifdef CONFIG_ARM64
					rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%lx(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
					#else
					rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%x(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
					#endif
					return MemTag[Idx].StartAddr;
				}else{
					space = drvif_memory_get_data_align(space+CONFIG_MDOMAIN_BURST_SIZE, DMA_SPEEDUP_ALIGN_VALUE);
				}
			}else if(MEMIDX_SUB_THIRD == Idx){
				space = space*3+CONFIG_MDOMAIN_BURST_SIZE;
			}else if(MEMIDX_SUB_SEC == Idx){
				if(MemTag[MEMIDX_SUB_THIRD].StartAddr != _DUMMYADDR){ //triple buffer case
					MemTag[Idx].Allocate_StartAddr = 0;
					MemTag[Idx].Size = space;
					MemTag[Idx].Status = MemTag[MEMIDX_SUB_THIRD].Status;
					MemTag[Idx].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_SUB_THIRD].StartAddr+space, DMA_SPEEDUP_ALIGN_VALUE);
					MemTag[MEMIDX_SUB_THIRD].Size -= space;
					#ifdef CONFIG_ARM64
					rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%lx(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
					#else
					rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%x(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
					#endif
					return MemTag[Idx].StartAddr;
				}else{ //double buffer case
					space = drvif_memory_get_data_align(space*2+CONFIG_MDOMAIN_BURST_SIZE, DMA_SPEEDUP_ALIGN_VALUE);
				}
			}else if(MEMIDX_SUB == Idx){
				if(MemTag[MEMIDX_SUB_THIRD].StartAddr != _DUMMYADDR){ //triple buffer case
					MemTag[Idx].Allocate_StartAddr = 0;
					MemTag[Idx].Size = space;
					MemTag[Idx].Status = MemTag[MEMIDX_SUB_THIRD].Status;
					MemTag[Idx].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_SUB_THIRD].StartAddr+space*2, DMA_SPEEDUP_ALIGN_VALUE);
					MemTag[MEMIDX_SUB_THIRD].Size -= space;
					#ifdef CONFIG_ARM64
					rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%lx(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
					#else
					rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%x(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
					#endif
					return MemTag[Idx].StartAddr;
				}else if(MemTag[MEMIDX_SUB_SEC].StartAddr != _DUMMYADDR){//double buffer case
					MemTag[Idx].Allocate_StartAddr = 0;
					MemTag[Idx].Size = space;
					MemTag[Idx].Status = MemTag[MEMIDX_SUB_SEC].Status;
					MemTag[Idx].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_SUB_SEC].StartAddr+space, DMA_SPEEDUP_ALIGN_VALUE);
					MemTag[MEMIDX_SUB_SEC].Size -= space;
					#ifdef CONFIG_ARM64
					rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%lx(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
					#else
					rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%x(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
					#endif
					return MemTag[Idx].StartAddr;
				}else{
					space = drvif_memory_get_data_align(space+CONFIG_MDOMAIN_BURST_SIZE, DMA_SPEEDUP_ALIGN_VALUE);
				}
			}

		if(MEMIDX_MAIN_2_THIRD == Idx){
			space = drvif_memory_get_data_align(space*3+CONFIG_MDOMAIN_BURST_SIZE, DMA_SPEEDUP_ALIGN_VALUE);
		}else if(MEMIDX_MAIN_2_SEC == Idx){
			if(MemTag[MEMIDX_MAIN_2_THIRD].StartAddr != _DUMMYADDR){ //triple buffer case
				MemTag[Idx].Allocate_StartAddr = 0;
				MemTag[Idx].Size = space;
				MemTag[Idx].Status = MemTag[MEMIDX_MAIN_2_THIRD].Status;
				MemTag[Idx].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_2_THIRD].StartAddr+space, DMA_SPEEDUP_ALIGN_VALUE);
				MemTag[MEMIDX_MAIN_2_THIRD].Size -= space;
				#ifdef CONFIG_ARM64
				rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%lx(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
				#else
				rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%x(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
				#endif
				return MemTag[Idx].StartAddr;
			}else{ //double buffer case
				space = drvif_memory_get_data_align(space*2+CONFIG_MDOMAIN_BURST_SIZE, DMA_SPEEDUP_ALIGN_VALUE);
			}
		}else if(MEMIDX_MAIN_2 == Idx){
			if(MemTag[MEMIDX_MAIN_2_THIRD].StartAddr != _DUMMYADDR){ //triple buffer case
				MemTag[Idx].Allocate_StartAddr = 0;
				MemTag[Idx].Size = space;
				MemTag[Idx].Status = MemTag[MEMIDX_MAIN_2_THIRD].Status;
				MemTag[Idx].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_2_THIRD].StartAddr+space*2, DMA_SPEEDUP_ALIGN_VALUE);
				MemTag[MEMIDX_MAIN_2_THIRD].Size -= space;
				#ifdef CONFIG_ARM64
				rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%lx(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
				#else
				rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%x(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
				#endif
				return MemTag[Idx].StartAddr;
			}else if(MemTag[MEMIDX_MAIN_SEC].StartAddr != _DUMMYADDR){//double buffer case
				MemTag[Idx].Allocate_StartAddr = 0;
				MemTag[Idx].Size = space;
				MemTag[Idx].Status = MemTag[MEMIDX_MAIN_SEC].Status;
				MemTag[Idx].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_SEC].StartAddr+space, DMA_SPEEDUP_ALIGN_VALUE);
				MemTag[MEMIDX_MAIN_SEC].Size -= space;
				#ifdef CONFIG_ARM64
				rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%lx(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
				#else
				rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%x(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
				#endif
				return MemTag[Idx].StartAddr;
			}else{
				space = drvif_memory_get_data_align(space+CONFIG_MDOMAIN_BURST_SIZE, DMA_SPEEDUP_ALIGN_VALUE);
			}
		}


		// Pli Fail
#ifdef PLI_FAIL
		if(Idx ==MEMIDX_MAIN || Idx ==MEMIDX_MAIN_SEC || Idx ==MEMIDX_MAIN_THIRD)
		{
			m_allocate_memory = 0x1d000000;
		}
		else if (Idx == MEMIDX_VDC)
		{
			m_allocate_memory = 0x1c000000;
		}
		else if (Idx == MEMIDX_DI)
		{
			m_allocate_memory = 0x1e000000;
		}
		else
		{
			rtd_pr_scaler_memory_debug("Error, can't handle reserverd memory, mem Idx=%d\n", Idx);
		}
#else
		//m_allocate_memory = pli_allocOverlappedMemory(space + (1 << 5));
#endif
#if 0
		if(m_allocate_memory == 0){
			unsigned char				*m_pNonCacheStartAddr;
			rtd_pr_scaler_memory_debug("Allocate RBUS index %d memory fail,allocate by pli\n",Idx);
			#if 0//qiang_zhou:fixme memory alloc
			m_pCacheStartAddr = (BYTE *)pli_allocContinuousMemorySpecific((space + (1 << 5)), (char**)&m_pNonCacheStartAddr, &m_PhyStartAddr, GFP_DCU1);
			#endif
			if(m_pCacheStartAddr == NULL){
				rtd_pr_scaler_memory_debug("[ERROR]Allocate DCU1 size=%x fail\n",(space + (1 << 5)));
				return FALSE;
			}else{
				rtd_pr_scaler_memory_debug("[SCALER]PLI Allocate address %x\n",m_PhyStartAddr);
				MemTag[Idx].Allocate_StartAddr = (unsigned int)m_pCacheStartAddr;
				MemTag[Idx].Size = space;
				MemTag[Idx].Status = ALLOCATED_FROM_PLI;
				MemTag[Idx].StartAddr = drvif_memory_get_data_align(m_PhyStartAddr, (1 << 5));
			}
		}else{
			rtd_pr_scaler_memory_debug("[SCALER]Allocate address %x - %x\n",m_allocate_memory, (m_allocate_memory+space + (1 << 5)));
			MemTag[Idx].Allocate_StartAddr = m_allocate_memory;
			MemTag[Idx].Size = space;
			MemTag[Idx].Status = ALLOCATED_FROM_PLI;
			MemTag[Idx].StartAddr = drvif_memory_get_data_align(m_allocate_memory, (1 << 5));
		}
#else

		if(MEMIDX_DI == Idx){//DI, RTNR allocate memory by overlap memory @Crixus 20151122
			//change to use query driver @Crixus 20170505
			m_allocate_memory = get_query_start_address(QUERY_IDX_DI);
			//rtd_pr_scaler_memory_emerg("[crixus]DI non-i2rnd memory address = %x, space = %x\n", m_allocate_memory, space);
		}
#ifdef CONFIG_I2RND_ENABLE
		else if(MEMIDX_DI_2 == Idx){
			m_allocate_memory = get_query_start_address(QUERY_IDX_DI_2);;//DI_START_ADDRESS_KERNEL + DI_BUFFER_SIZE_VO;
			rtd_pr_scaler_memory_emerg("[crixus]DI sub memory address = %x\n", m_allocate_memory);

		}
#endif
		else{//M-domain allocate memory by DCU2
#ifndef BUILD_QUICK_SHOW
			buffer_size = dvr_size_alignment_cma(space + (1 << 5));
			m_pCacheStartAddr = (unsigned char *)dvr_malloc_specific(buffer_size, GFP_DCU2_FIRST);
			if(m_pCacheStartAddr == NULL){
				rtd_pr_scaler_memory_notice("[ERROR]Allocate DCU1 size=%x fail\n",buffer_size);
				BUG();//add kernel crash function when cma not enough
				return FALSE;
			}
			m_allocate_memory = (unsigned int)dvr_to_phys((void*)m_pCacheStartAddr);

			m_allocate_memory = dvr_memory_alignment((unsigned long)m_allocate_memory, buffer_size);
#endif
		}

		rtd_pr_scaler_memory_debug("[SCALER]Allocate address %x - %x\n",m_allocate_memory, (m_allocate_memory+buffer_size));
		MemTag[Idx].Allocate_StartAddr = m_allocate_memory;
		MemTag[Idx].Size = space;
		MemTag[Idx].Status = ALLOCATED_FROM_PLI;
		MemTag[Idx].StartAddr = drvif_memory_get_data_align(m_allocate_memory, DMA_SPEEDUP_ALIGN_VALUE);
		MemTag[Idx].Allocate_VirAddr = (unsigned long)m_pCacheStartAddr;
#endif

		//rtd_pr_scaler_memory_emerg("[crixus]MemTag[Idx].StartAddr = %x\n", MemTag[Idx].StartAddr);
		//rtd_pr_scaler_memory_emerg("[crixus]DI_MULTIVIEW_ADDRESS_SUB = %x\n", DI_MULTIVIEW_ADDRESS_SUB);
		//rtd_pr_scaler_memory_emerg("[crixus](DI_MULTIVIEW_ADDRESS_SUB + space - CONFIG_MDOMAIN_BURST_SIZE) = %x\n", (DI_MULTIVIEW_ADDRESS_SUB + space - CONFIG_MDOMAIN_BURST_SIZE));
		//rtd_pr_scaler_memory_emerg("[crixus]space = %x\n", space);

		if(MEMIDX_DI == Idx){
			//IoReg_Write32(DI_DI1_WRRD_BoundaryAddr1_reg, (MemTag[Idx].StartAddr & 0x7ffffff0));
			//patch for DI/RTNR bug(overwrite 512Byte)
#ifdef CONFIG_I2RND_ENABLE
			//merlin3- s0 boundary need to cover DI1 + DI2 memory boundary. @Crixus 20180124
			//IoReg_Write32(DI_DI1_WRRD_BoundaryAddr2_reg, ((MemTag[Idx].StartAddr+(VIP_DI_1_BUFFER_SIZE + VIP_DI_2_BUFFER_SIZE)-CONFIG_MDOMAIN_BURST_SIZE)) & 0x7ffffff0);
#else
			//IoReg_Write32(DI_DI1_WRRD_BoundaryAddr2_reg, ((MemTag[Idx].StartAddr+VIP_DI_1_BUFFER_SIZE-CONFIG_MDOMAIN_BURST_SIZE)) & 0x7ffffff0);
#endif
#ifdef CONFIG_I2RND_ENABLE
		}else if(MEMIDX_DI_2 == Idx){
			//Update the DI_2 boundary for i2run case @Crixus 20170515
			//IoReg_Write32(DI_DI1_I2R_WRRD_BoundaryAddr1_reg, (MemTag[MEMIDX_DI_2].StartAddr & 0x7ffffff0));
			//IoReg_Write32(DI_DI1_I2R_WRRD_BoundaryAddr2_reg, ((MemTag[MEMIDX_DI_2].StartAddr + space - CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0));
#endif
		}else if(MEMIDX_MAIN_THIRD == Idx){
			IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,(MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0));
			IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(MemTag[MEMIDX_MAIN_THIRD].StartAddr+space-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
		}else if(MEMIDX_MAIN_SEC == Idx ){
			IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,(MemTag[MEMIDX_MAIN_SEC].StartAddr & 0x7ffffff0));
			IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(MemTag[MEMIDX_MAIN_SEC].StartAddr+space-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
		}else if(MEMIDX_MAIN == Idx){
			IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,(MemTag[MEMIDX_MAIN].StartAddr & 0x7ffffff0));
			IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(MemTag[MEMIDX_MAIN].StartAddr+space-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
		}else if(MEMIDX_SUB_THIRD == Idx){
			IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr1_reg,(MemTag[MEMIDX_SUB_THIRD].StartAddr & 0x7ffffff0));
			IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_reg,(MemTag[MEMIDX_SUB_THIRD].StartAddr+space-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
		}else if(MEMIDX_SUB_SEC == Idx){
			IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr1_reg,(MemTag[MEMIDX_SUB_SEC].StartAddr & 0x7ffffff0));
			IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_reg,(MemTag[MEMIDX_SUB_SEC].StartAddr+space-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
		}else if(MEMIDX_SUB == Idx){
			IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr1_reg,(MemTag[MEMIDX_SUB].StartAddr & 0x7ffffff0));
			IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_reg,(MemTag[MEMIDX_SUB].StartAddr+space-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
		}
#else
		if((MemTag[Idx].StartAddr != _DUMMYADDR) && (Idx != MEMIDX_VDC)) {
			if(space <= MemTag[Idx].Size) {
				return MemTag[Idx].StartAddr;
			} else {
				memory_set_mem_tag(Idx, _DUMMYADDR, 0);
				return _DUMMYADDR;
			}
		}
		space = drvif_memory_get_data_align(space, (1 << _DDR_BITS));
		switch(Idx) {
			case MEMIDX_VDC:
				if(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) {
					MemTag[Idx].Status = ALLOCATED_FROM_LOW;
					memory_set_mem_tag(Idx, AllocStartAddr, space);
				} else {
					MemTag[Idx].Status = ALLOCATED_FROM_HIGH;
					memory_set_mem_tag(Idx, AllocEndAddr - space + 1, space);  // add a bank gap avoid Main sec Blk cover Bitmap
				}
				break;

			case MEMIDX_TT:
				if(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) {
					MemTag[Idx].Status = ALLOCATED_FROM_LOW;
					if(MemTag[MEMIDX_VDC].Status == ALLOCATED_FROM_LOW) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_VDC].StartAddr + MemTag[MEMIDX_VDC].Size, space + (1 << _DDR_BITS));
					} else {
						memory_set_mem_tag(Idx, AllocStartAddr, space + (1 << _DDR_BITS));
					}
				} else {
					MemTag[Idx].Status = ALLOCATED_FROM_HIGH;
					if(MemTag[MEMIDX_VDC].Status ==  ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_VDC].StartAddr - space + 1, space);
					} else {
						memory_set_mem_tag(Idx, AllocEndAddr - space + 1, space);
					}
				}
				break;

			case MEMIDX_JPEG:
				if(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) {
					MemTag[Idx].Status = ALLOCATED_FROM_LOW;
					memory_set_mem_tag(Idx, AllocStartAddr, space);
				} else {
					MemTag[Idx].Status = ALLOCATED_FROM_HIGH;
					memory_set_mem_tag(Idx, AllocEndAddr - space + 1, space);  // add a bank gap avoid Main sec Blk cover Bitmap
				}
				break;


			case MEMIDX_DI:
				if(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) {
					MemTag[Idx].Status = ALLOCATED_FROM_LOW;

					if(MemTag[MEMIDX_TT].Status ==  ALLOCATED_FROM_LOW) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_TT].StartAddr + MemTag[MEMIDX_TT].Size, space + (1 << _DDR_BITS));
					}
					else if(MemTag[MEMIDX_VDC].Status ==  ALLOCATED_FROM_LOW) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_VDC].StartAddr + MemTag[MEMIDX_VDC].Size, space + (1 << _DDR_BITS));
					}
					else if(MemTag[MEMIDX_JPEG].Status == ALLOCATED_FROM_LOW) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_JPEG].StartAddr + MemTag[MEMIDX_JPEG].Size, space + (1 << _DDR_BITS));
					}
					else
					{
						memory_set_mem_tag(Idx, AllocStartAddr, space + (1 << _DDR_BITS));
					}
				} else {
					MemTag[Idx].Status = ALLOCATED_FROM_HIGH;
					if(MemTag[MEMIDX_TT].Status ==  ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_TT].StartAddr - space + 1, space);
					}else if(MemTag[MEMIDX_VDC].Status ==  ALLOCATED_FROM_HIGH) {
							//memory_set_mem_tag(Idx, MemTag[MEMIDX_VDC].StartAddr - MemTag[MEMIDX_VDC].Size + 1, space);
							//////////////////////////////////
							//By Gilbert(2008/9/23)
							///////////////////////////////////
							memory_set_mem_tag(Idx, MemTag[MEMIDX_VDC].StartAddr - space + 1, space);
					}else if  (MemTag[MEMIDX_JPEG].Status ==  ALLOCATED_FROM_HIGH) {
							memory_set_mem_tag(Idx, MemTag[MEMIDX_JPEG].StartAddr - space + 1, space);
					} else {
							rtd_pr_scaler_memory_debug("[qiang_zhou debug]:MemTag[MEMIDX_DI].Status==%d\n",MemTag[MEMIDX_DI].Status);
							memory_set_mem_tag(Idx, AllocEndAddr - space + 1, space);
					}
				}
				break;

			case MEMIDX_MAIN:
				MemTag[Idx].Status = ALLOCATED_FROM_HIGH;
				if(MemTag[MEMIDX_MAIN_SEC].StartAddr != _DUMMYADDR) {
					if((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) > 2048)
						&& (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > 1200)&& !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_422CAP)){	// 4k2k
						// [IDMA] IDMA capture buffer share memory with video decoder
						SCALERDRV_ALLOCBUFFER_INFO stAllocBufferInfo_idma;
						if(drvif_scaler_getValidMemInfo_fromVideo(&stAllocBufferInfo_idma) < 0){
							//unsigned char				*m_pNonCacheStartAddr;
							rtd_pr_scaler_memory_debug("[IDMA_M domain] fail to get memory information\n");
							#if 0//qiang_zhou:fixme memory alloc
							m_pCacheStartAddr = (BYTE *)pli_allocContinuousMemorySpecific(space, (char**)&m_pNonCacheStartAddr, &m_PhyStartAddr, GFP_DCU1);
							#else
							m_pCacheStartAddr=dvr_malloc(space);
							//m_PhyStartAddr = (unsigned int)virt_to_phys((void*)m_pCacheStartAddr);
							m_PhyStartAddr = (unsigned int)dvr_to_phys((void*)m_pCacheStartAddr);
							#endif
							if(m_pCacheStartAddr == NULL){
								rtd_pr_scaler_memory_debug("[ERROR]Allocate M domain DCU1 size=%x fail\n",mMallocSize);
								return FALSE;
							}else{
								//physical address
								MemTag[Idx].StartAddr = m_PhyStartAddr;
								MemTag[Idx].Size = space;
								rtd_pr_scaler_memory_debug("[SLR_MEMORY5][PLI][Cache:%x][Phy:%x][Size:%x]\n", (unsigned int)m_pCacheStartAddr, (unsigned int)m_PhyStartAddr, space);

							}
						}else{
							MemTag[Idx].StartAddr = stAllocBufferInfo_idma.normal_mem_addr&0x7fffffff;
							MemTag[Idx].Size = space;
							rtd_pr_scaler_memory_debug("[IDMA_M domain] get memory information OK,addr=%x\n",MemTag[Idx].StartAddr);
						}

						if(MemTag[Idx].StartAddr < AllocStartAddr){
							IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg, MemTag[Idx].StartAddr);
						}else if(MemTag[Idx].StartAddr > AllocEndAddr){
							IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,MemTag[Idx].StartAddr+space-CONFIG_MDOMAIN_BURST_SIZE);
						}
					}else{
					memory_set_mem_tag(Idx, MemTag[MEMIDX_MAIN_SEC].StartAddr  - space + 1, space);
					}
				} else {

					if(MemTag[MEMIDX_DI].Status == ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_DI].StartAddr - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					} else if(MemTag[MEMIDX_TT].Status == ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_TT].StartAddr  - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					} else if(MemTag[MEMIDX_VDC].Status == ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_VDC].StartAddr  - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					} else if(MemTag[MEMIDX_JPEG].Status == ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_JPEG].StartAddr  - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					} else {
						memory_set_mem_tag(Idx, AllocEndAddr - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					}
				}
				break;
			case MEMIDX_MAIN_SEC:
				MemTag[Idx].Status = ALLOCATED_FROM_HIGH;
				if(MemTag[MEMIDX_MAIN_THIRD].StartAddr != _DUMMYADDR) {
					memory_set_mem_tag(Idx, MemTag[MEMIDX_MAIN_THIRD].StartAddr - space + 1, space);  // add a bank gap avoid Main sec Blk cover Bitmap
				} else {
					if(MemTag[MEMIDX_DI].Status == ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_DI].StartAddr - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					}
					else if(MemTag[MEMIDX_TT].Status == ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_TT].StartAddr  - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					} else if(MemTag[MEMIDX_VDC].Status == ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_VDC].StartAddr  - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					} else if(MemTag[MEMIDX_JPEG].Status == ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_JPEG].StartAddr  - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					} else {
						memory_set_mem_tag(Idx, AllocEndAddr - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					}
				}
				break;


			case MEMIDX_MAIN_THIRD:
				MemTag[Idx].Status = ALLOCATED_FROM_HIGH;
				if(MemTag[MEMIDX_DI].Status == ALLOCATED_FROM_HIGH) {
					memory_set_mem_tag(Idx, MemTag[MEMIDX_DI].StartAddr - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
				} else {
					if(MemTag[MEMIDX_TT].Status == ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_TT].StartAddr  - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					} else if(MemTag[MEMIDX_VDC].Status == ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_VDC].StartAddr  - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					} else if(MemTag[MEMIDX_JPEG].Status == ALLOCATED_FROM_HIGH) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_JPEG].StartAddr  - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					} else {
						memory_set_mem_tag(Idx, AllocEndAddr - (space + (1 << _DDR_BITS)) + 1, (space + (1 << _DDR_BITS)));  // add a bank gap avoid Main sec Blk cover Bitmap
					}
				}
				break;


			case MEMIDX_SUB:
				MemTag[Idx].Status = ALLOCATED_FROM_LOW;
				if(MemTag[MEMIDX_DI].Status == ALLOCATED_FROM_LOW) {
					memory_set_mem_tag(Idx, MemTag[MEMIDX_DI].StartAddr + MemTag[MEMIDX_DI].Size, space);
				} else {
					if(MemTag[MEMIDX_TT].Status == ALLOCATED_FROM_LOW) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_TT].StartAddr + MemTag[MEMIDX_TT].Size, space);
					} else if(MemTag[MEMIDX_VDC].Status == ALLOCATED_FROM_LOW) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_VDC].StartAddr + MemTag[MEMIDX_VDC].Size, space);
					} else if(MemTag[MEMIDX_JPEG].Status == ALLOCATED_FROM_LOW) {
						memory_set_mem_tag(Idx, MemTag[MEMIDX_JPEG].StartAddr + MemTag[MEMIDX_JPEG].Size, space);
					} else {
						memory_set_mem_tag(Idx, AllocStartAddr, space);
					}
				}
				IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr1_reg,(MemTag[Idx].StartAddr & 0x7ffffff0));
				break;

			case MEMIDX_SUB_SEC:
				if(MemTag[MEMIDX_SUB].StartAddr != _DUMMYADDR) {
					MemTag[Idx].Status = ALLOCATED_FROM_LOW;
					memory_set_mem_tag(Idx, MemTag[MEMIDX_SUB].StartAddr + MemTag[MEMIDX_SUB].Size, space);
				} else
					memory_set_mem_tag(Idx, _DUMMYADDR, 0);
				IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_reg,(MemTag[Idx].StartAddr+MemTag[Idx].Size) & 0x7ffffff0);
				break;


			case MEMIDX_SUB_THIRD:
				if(MemTag[MEMIDX_SUB_SEC].StartAddr != _DUMMYADDR) {
					MemTag[Idx].Status = ALLOCATED_FROM_LOW;
					memory_set_mem_tag(Idx, MemTag[MEMIDX_SUB_SEC].StartAddr + MemTag[MEMIDX_SUB_SEC].Size, space);
				} else
					memory_set_mem_tag(Idx, _DUMMYADDR, 0);
				IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_reg,(MemTag[Idx].StartAddr+MemTag[Idx].Size) & 0x7ffffff0);
				break;
			default:
				return _DUMMYADDR;
		};
#endif
	}

	#ifdef CONFIG_ARM64
	rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%lx(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
	#else
	rtd_pr_scaler_memory_debug("[MEM] Idx[%d]=%x(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
	#endif

	if(MEMIDX_DI_2 == Idx){

		#ifdef CONFIG_ARM64
		rtd_pr_scaler_memory_emerg("[MEM] Idx[%d]=%lx(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
		#else
		rtd_pr_scaler_memory_emerg("[MEM] Idx[%d]=%x(%d KB)\n", Idx, MemTag[Idx].StartAddr, space >> 10);
		#endif
	}
	return MemTag[Idx].StartAddr;
}

/*============================================================================*/
/**
 * memory_check_capture_polarity_error
 * Check field polarity error bit. reset capture block if it is error.
 *
 * @param  { none }
 * @return { none }
 *
 */
void memory_check_capture_polarity_error(unsigned char channel)
{

#ifndef UT_flag
	unsigned char Sts;

	//field polarity error check
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	Sts = IoReg_ReadWord0(channel ? MDOMAIN_CAP_DDR_In2Status_reg : MDOMAIN_CAP_DDR_In1Status_reg);
	if (Sts & _BIT4) {	//Field polarity Error bit
		IoReg_ClearBits(channel ? MDOMAIN_CAP_DDR_In2Status_reg : MDOMAIN_CAP_DDR_In1Status_reg, _BIT4);
	}
#else
	Sts = IoReg_ReadWord0(/*channel ? MDOMAIN_CAP_DDR_In2Status_reg : */MDOMAIN_CAP_DDR_In1Status_reg);
	if (Sts & _BIT4) {	//Field polarity Error bit
		IoReg_ClearBits(/*channel ? MDOMAIN_CAP_DDR_In2Status_reg : */MDOMAIN_CAP_DDR_In1Status_reg, _BIT4);
	}
#endif

#endif	// end of #ifndef UT_flag
}



/*============================================================================*/
/**
 * memory_division
 * A Mathematical division
 *
 * @param <Dividend> { Dividend }
 * @param <Divisor> { Divisor }
 * @param <Quotient> { Quotient }
 * @param <Remainder> { Remainder }
 * @return { none }
 *
 */
void memory_division(unsigned int Dividend, unsigned char Divisor, unsigned int* Quotient, unsigned char* Remainder)
{
	*Quotient = (unsigned int)Dividend / Divisor;
	*Remainder = (unsigned char)((unsigned int)Dividend - (unsigned int)(*Quotient) * Divisor);
}

/*============================================================================*/
/**
 * memory_set_capture_fifo
 * Set capture FIFO information
 *
 * @param <channel> { channel selection }
 * @param <Quotient> { Quotient }
 * @param <Remainder> { remainder }
 * @param <Waterlevel> { Waterlevel }
 * @return { none }
 *
 */
void memory_set_capture_fifo(unsigned char channel, unsigned int Quotient, unsigned char Remainder, unsigned char WaterLevel)
{
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_Write32(channel ? MDOMAIN_CAP_DDR_In2WTLVL_reg :  MDOMAIN_CAP_DDR_In1WTLVL_Num_reg, ((WaterLevel>>1) << 16) | Quotient);
	IoReg_Write32(channel ? MDOMAIN_CAP_DDR_In2WrLen_Rem_reg :  MDOMAIN_CAP_DDR_In1WrLen_Rem_reg, (WaterLevel << 8) | Remainder);
#else
	IoReg_Write32(/*channel ? MDOMAIN_CAP_DDR_In2WTLVL_reg :  */MDOMAIN_CAP_DDR_In1WTLVL_Num_reg, ((WaterLevel>>1) << 16) | Quotient);
	IoReg_Write32(/*channel ? MDOMAIN_CAP_DDR_In2WrLen_Rem_reg :  */MDOMAIN_CAP_DDR_In1WrLen_Rem_reg, (WaterLevel << 8) | Remainder);
#endif
}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
/*============================================================================*/
/**
 * memory_set_capture_addr
 * Set capture address
 *
 * @param <channel> { channel selection }
 * @param <Addr1> { 1st capture memory  }
 * @param <Addr2> { 2nd capture memory  }
 * @param <Waterlevel> { Waterlevel }
 * @return { none }
 *
 */
 #if 0
static void memory_set_capture_addr(unsigned char channel, unsigned int Addr1, unsigned int Addr2)
{
	//Crixus@20140722 - Mac3 no in2
	IoReg_Write32(/*channel ? MDOMAIN_CAP_DDR_In2Addr_reg : */MDOMAIN_CAP_DDR_In1Addr_reg, (Addr1 & 0x7ffffff0));
	IoReg_Write32(/*channel ? MDOMAIN_CAP_DDR_In2_2ndAddr_reg : */MDOMAIN_CAP_DDR_In1_2ndAddr_reg, (Addr2 & 0x7ffffff0));
}
#endif
/*============================================================================*/
/**
 * memory_set_capture_for_mp
 * Set memory capture registers for multi-picture. It can be #1 or #2, based on
 * current info structure.
 *
 * @param <info> { display-info struecture }
 * @return { none }
 *
 * @note
 * For the cases except multi-picture, please refer to MemSetCapture()
 */
 #if 0
static void memory_set_capture_for_mp(void)
{
	unsigned int TotalSize;
	unsigned int Quotient;
	unsigned char Remainder;
	StructMemBlock* pBlock1;
	StructMemBlock* pBlock2;
	unsigned short BankChk;
	unsigned short LineStep;
	unsigned short BlockStep;
	unsigned char i,LineRemain;

	unsigned int burstLen = _BURSTLENGTH2;

	//Crixus@20140722 - Mac3 no in2
	IoReg_ClearBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_Cap_In2_enable_reg : */MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);	// disable Capture
	//frank@0503 add below delay to confirm M capture have disable
	msleep(20);
	memory_check_capture_polarity_error(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)); //field polarity error check
	rtd_pr_scaler_memory_debug("\n Set CaptureForMP !");
	TotalSize = memory_get_capture_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), MEMCAPTYPE_LINE);		// calculate the memory size of capture
	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division
	Remainder = drvif_memory_get_data_align(Remainder, 4);	// multiple of 4

	if (Remainder == 0){	// remainder is not allowed to be zero
		Remainder = burstLen;
		Quotient -= 1;
	}
	memory_set_capture_fifo(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), Quotient, Remainder, burstLen);	// set capture FIFO

	TotalSize = SHL(Quotient * burstLen + Remainder, 3); //unit conversion. 64bits to 8bits

	BankChk= TotalSize;	// one line size in DDR

	LineStep = drvif_memory_get_data_align(TotalSize, _DDR_BANK_SIZE);	// multiple of 0x100, for bank check

	Scaler_PipSetSubInfo(SLR_PIP_SUB_MPDISPLINESTEP,LineStep);
	IoReg_Write32(MDOMAIN_CAP_DDR_In2LineStep_reg, (LineStep + LineStep%2) << 3);//bit 3 need to be 0. rbus rule

	BlockStep = (unsigned char)(SHR(LineStep, 9));	// unit conversion. unit: bank, remove C(1) also
	BlockStep = BlockStep * Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN);	// get 1 block


	LineRemain = (Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN)) % 4;		// add avoid MP44 and MP22 input Line number not 4 factor
	if( LineRemain == 0 )
		LineRemain = 2;
	else if(LineRemain == 2)
		LineRemain = 0;

	if( BankChk > (_DDR_BANK_SIZE*2))		// MP 1 line occupy 3 banks
		BlockStep += 4;
	else if( BankChk > _DDR_BANK_SIZE)	// MP 1 line occupy 2 banks
		BlockStep += 2;
	else{								// MP 1 line occupy 1 bank

		if(Scaler_PipGetInfo(SLR_PIP_TYPE) == SUBTYPE_MP3X3)	// MP33 input Line number must be 4 factor
			BlockStep += 1;
		else
			BlockStep += LineRemain;
	}

	TotalSize = BlockStep;
	Scaler_PipSetSubInfo(SLR_PIP_SUB_MPDISPBLOCKSTEP,TotalSize);
	TotalSize = SHL(TotalSize, _DDR_BITS);  //unit conversion, unit: 32bits
	TotalSize = TotalSize * Scaler_PipGetSubInfo(SLR_PIP_SUB_MPCNT);	// multiple MP count
	TotalSize = drvif_memory_get_data_align(TotalSize, _DDR_BANK_SIZE); // space must be bank-alignment

	drvif_memory_free_block(MEMIDX_SUB);	// free block befor getting a new one
	drvif_memory_free_block(MEMIDX_SUB_SEC);	// free block befor getting a new one
	drvif_memory_alloc_block(MEMIDX_SUB, TotalSize+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//*2,because space unit is byte

	pBlock1 = &MemTag[MEMIDX_SUB];
	pBlock1->StartAddr = drvif_memory_get_data_align(pBlock1->StartAddr, _DDR_BANK_ADDR_ALIGNMENT); // MP start address must be alignment bank0
	pBlock1->StartAddr += (_DDR_BANK_ADDR_ALIGNMENT*4);

	MemTag[MEMIDX_SUB].StartAddr = pBlock1->StartAddr;

// for second block
	TotalSize = Scaler_PipGetSubInfo(SLR_PIP_SUB_MPDISPBLOCKSTEP) * Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN);


	drvif_memory_alloc_block(MEMIDX_SUB_SEC, TotalSize+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//*2,because space unit is byte

	pBlock2 = &MemTag[MEMIDX_SUB_SEC];
	pBlock2->StartAddr = drvif_memory_get_data_align(pBlock2->StartAddr, _DDR_BANK_ADDR_ALIGNMENT);// alignment bank0 address
	pBlock2->StartAddr +=  (_DDR_BANK_ADDR_ALIGNMENT*4);

	if(BankChk < _DDR_BANK_SIZE)		// 1 line occupy 1 bank
		pBlock2->StartAddr += (_DDR_BANK_SIZE * 6);	// second block start address alignment bank 3

	MemTag[MEMIDX_SUB_SEC].StartAddr = pBlock2->StartAddr;		// backup sub second block start address

	for (i=0; i<16; i++){
		if (AND(SHR(Scaler_PipGetSubInfo(SLR_PIP_SUB_MPHIIDX), i), 0x01))	// check each bit from LSB
			break;	// found a bit for current MP window
	}
	TotalSize = (((unsigned int)Scaler_PipGetSubInfo(SLR_PIP_SUB_MPDISPBLOCKSTEP) * i) << CONFIG_DDR_COL_BITS);	// Shift start address based on current MP window, update MSB & medium bytes
	memory_set_capture_addr(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), (pBlock1->StartAddr + TotalSize), pBlock2->StartAddr);  	// MP 1L7B need change Capture to MP3
	IoReg_WriteWord0(MDOMAIN_CAP_DDR_In2LineNum_reg, SCALER_OR(AND(Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN), 0x0FFF), 0x0000));
	IoReg_Write32(MDOMAIN_CAP_DDR_In2Ctrl_reg, _BIT0 | _BIT4);
	IoReg_Write32(MDOMAIN_CAP_DDR_In2Ctrl_reg , _BIT0 | _BIT4 | _BIT6);
}
#endif
#endif //CONFIG_DUAL_CHANNEL  // There are two channels

static void MemSetDisplayAddr(unsigned char display, unsigned int Addr1, unsigned int Addr2)
{
	unsigned int address = 0;

	if(display == SLR_MAIN_DISPLAY){
		//rtd_pr_scaler_memory_debug("Before %s %d IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg)=%x\n",__FUNCTION__,__LINE__,IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg));
		IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg, Addr1&0x7ffffff0);
		//rtd_pr_scaler_memory_debug("After %s %d IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg)=%x\n",__FUNCTION__,__LINE__,IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg));
		IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg, Addr2&0x7ffffff0);

		//update m_disp boundary the same as m_cap boundary @Crixus 20171227
		address = IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg);
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr1_reg, address&0x7ffffff0);
		address = IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg);
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr2_reg, address&0x7ffffff0);
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else{
		IoReg_Write32(MDOMAIN_DISP_DDR_SubAddr_reg, Addr1&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DDR_Sub2ndAddr_reg, Addr2&0x7ffffff0);
		//update m_disp boundary the same as m_cap boundary @Crixus 20171227
		address = IoReg_Read32(MDOMAIN_CAP_CapSub_BoundaryAddr1_reg);
		IoReg_Write32(MDOMAIN_DISP_DispSub_BoundaryAddr1_reg, address&0x7ffffff0);
		address = IoReg_Read32(MDOMAIN_CAP_CapSub_BoundaryAddr2_reg);
		IoReg_Write32(MDOMAIN_DISP_DispSub_BoundaryAddr2_reg, address&0x7ffffff0);
	}
#endif
}

/*============================================================================*/
/**
 * memory_get_display_size
 * Calculate the memory size of display. The size is counted by 4:2:2/4:4:4 data format,
 * and frame-access/line access.
 *
 * @param <info> { disp-info structure }
 * @param <AccessType> { Access_Type  }
 * @param <TotalSize> { size of capture }
 * @return { none }
 *
 */
unsigned int memory_get_display_size(unsigned char display)
{
	unsigned int TotalSize;
	unsigned char ddrbit;

	ddrbit =  (Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CHANNEL) ? 0 : (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_10BIT) << 1))  | ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_422CAP));
#ifdef CONFIG_ENABLE_SE_ROTATE_M_DOMAIN_BLOCK_MODE
	//if rotate 90 or 270, then using length to calculate the memory line size
#if 0
	if(Get_rotate_function(SLR_MAIN_DISPLAY) && (get_rotate_curState()==MAIN_ROTATE_INIT_VI) && ((Get_Rotate_Mode()==ROTATE_MODE_90)||(Get_Rotate_Mode()==ROTATE_MODE_270))){
		if(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)==SLR_MAIN_DISPLAY){
			TotalSize = memory_get_line_size(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_LEN), (ddrbit & _BIT0), ((ddrbit & _BIT1) >> 1));
		}
	}
	else
#endif
#endif
		TotalSize = memory_get_line_size(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_WID), (ddrbit & _BIT0), ((ddrbit & _BIT1) >> 1));
	//rtd_pr_scaler_memory_debug("[Memory display]:TotalSize=%x\n", TotalSize);
	return TotalSize;
}

 #ifndef BUILD_QUICK_SHOW
/*============================================================================*/
/**
 * memory_set_mp
 * Set multi-picture related registers. It can be called only in sub
 *
 * @param <info> { disp-info structure }
 * @return { none }
 * @note
 * It can be called in sub display
 *
 */
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
#if 0
static void memory_set_mp(void)
{
	unsigned short TotalSize;
	unsigned short BankLimit;
	unsigned short EndColumn;
	unsigned char ucTemp8;
	unsigned short ucTemp16;

	ucTemp16 = PipMp_GetMpFlag((eSubType)Scaler_PipGetInfo(SLR_PIP_TYPE));	// get MP enable flag
	IoReg_WriteWord0(MDOMAIN_DISP_DDR_SubMPEn_reg, ucTemp16);

	ucTemp8 = AND(IoReg_ReadByte0(MDOMAIN_DISP_DDR_SubMPMode_reg), 0xF0);	// get high nibble, active MP window
	ucTemp8 = (ucTemp8 | _BIT3 | SHR(AND(Scaler_PipGetSubInfo(SLR_PIP_SUB_SUBHVCNT), 0x0C), 1) | _BIT0); // set MP mode
	IoReg_WriteByte0(MDOMAIN_DISP_DDR_SubMPMode_reg, ucTemp8);	// set MP mode

	IoReg_WriteWord0(MDOMAIN_DISP_DDR_SubMPLineNum_reg, Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN)); 	// set MP Line number

	//------------------Set the end column------------------
	TotalSize = memory_get_display_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY));

#if(CONFIG_DDR_DATA_BITS == 16)
	ucTemp16 = TotalSize = SHL(TotalSize, 2);		// 64bit to 16 bits
#else
	ucTemp16 = TotalSize = SHL(TotalSize, 1);		// 64bit to 32 bits
#endif
	TotalSize = drvif_memory_get_data_align(TotalSize,4);

	if( TotalSize > (_DDR_BANK_SIZE*2) ){

		EndColumn = ucTemp16-(_DDR_BANK_SIZE*2) -1;
		BankLimit = 0x1800;

	}else if( TotalSize > _DDR_BANK_SIZE ){

		EndColumn = ucTemp16-_DDR_BANK_SIZE -1;
		BankLimit = 0x1400;

	}else{

		EndColumn = ucTemp16 -1;
		BankLimit = 0x1e00;

	}

	ucTemp16 = (BankLimit |EndColumn) ;

	IoReg_WriteWord0(MDOMAIN_DISP_DDR_SubBankLimitEndCol_reg, ucTemp16);
	drvif_memory_set_dbuffer_write();
}
#endif
#endif //CONFIG_DUAL_CHANNEL  // There are two channels


/*============================================================================*/
/**
 * memory_set_sub_for_mp
 * This function is used to set sub control register for Mp
 *
 * @param <info> { display-info struecture }
 * @param <Block_Step> { Block step }
 * @return { none }
 * @note
 * If any one of the Memory-sub registers is changed, it is necessary to write
 * P3_B6 register to validate the update.
 *
 */
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
#if 0
static void memory_set_sub_for_mp(void)
{
	unsigned int burstLen = _BURSTLENGTH2;
	unsigned int fifoLen = _FIFOLENGTH2;
	unsigned int linestep;//need to be even
	unsigned int blockstep;//need to be even

	IoReg_ClearBits(MDOMAIN_DISP_DDR_SubCtrl_reg,_BIT0);
	drvif_memory_set_dbuffer_write();
	MemSetDisplayAddr(SLR_SUB_DISPLAY, drvif_memory_get_block_addr(MEMIDX_SUB), drvif_memory_get_block_addr(MEMIDX_SUB_SEC));
	//frank@0323 change waterlevel/2 for AT,because water level nit is 128 bits
	IoReg_Mask32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg, 0xff800ff, ((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) & 0x07FF) << 8) | (fifoLen-burstLen));

	if((Scaler_PipGetInfo(SLR_PIP_TYPE) >= SUBTYPE_MP4X4) && (Scaler_PipGetInfo(SLR_PIP_TYPE) <= SUBTYPE_MP1L7B)){ // for sub size can be divisible by 4 in case 1C12B, 4x4, 1L7B
		IoReg_WriteWord0(MDOMAIN_DISP_DDR_SubLineNum_reg, (Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN)<<2) & 0x0FFF);
	}
	else
		IoReg_WriteWord0(MDOMAIN_DISP_DDR_SubLineNum_reg, (_DISP_LEN & 0x0FFF));

	IoReg_Write32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, 0xffff00ff |((burstLen & 0xff) << 8));
	linestep = Scaler_PipGetSubInfo(SLR_PIP_SUB_MPDISPBLOCKSTEP);
	linestep = (linestep % 2)? linestep + 1: linestep;
	IoReg_Write32(MDOMAIN_DISP_DDR_SubLineStep_reg, linestep << 3); //bit3 need to be 0. rbus rule
	blockstep = Scaler_PipGetSubInfo(SLR_PIP_SUB_MPDISPBLOCKSTEP);
	blockstep = (blockstep % 2)? blockstep + 1: blockstep;
	IoReg_Write32(MDOMAIN_DISP_DDR_SubBlkStep_reg, blockstep << 3); //bit3 need to be 0. rbus rule

	IoReg_SetBits(MDOMAIN_DISP_DDR_SubCtrl_reg, _BIT6);
	drvif_memory_set_dbuffer_write();
	memory_set_mp();
}
#endif
#endif //CONFIG_DUAL_CHANNEL  // There are two channels

/*============================================================================*/
/**
 * memory_set_sub_for_pip
 * This function is used to set sub control register for PIP
 *
 * @param <info> { display-info struecture }
 * @param <Block_Step> { Block step }
 * @return { none }
 * @note
 * If any one of the Memory-sub registers is changed, it is necessary to write
 * P3_B6 register to validate the update.
 *
 */
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
static void memory_set_sub_for_pip(unsigned char display)
{
	unsigned int TotalSize = 0;
	unsigned int Quotient = 0;
	unsigned char Remainder = 0;
	unsigned int fifoLen = _FIFOLENGTH2;
	unsigned int burstLen = _BURSTLENGTH2;
	unsigned int MemShiftAddr = 0;
	unsigned short droppixel = 0;
	unsigned int ulLineStep = 0;

	if ((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_HSTA) == 0) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_VSTA) == 0) ) {
		//frank@0323 change waterlevel/2 for AT,because water level nit is 128 bits
		IoReg_Mask32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg, 0xffc00000, ((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) & 0x3FFF) << 8) | ((fifoLen - (burstLen>>1))>>1));
		if (dvrif_memory_compression_get_enable(SLR_SUB_DISPLAY) == TRUE)
		{
			TotalSize = memory_get_disp_line_size_compression(display, Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) + droppixel);	// calculate the memory size of capture
		}
		else
		{
			TotalSize = memory_get_display_size(display);	// calculate the memory size of capture
		}
	} else {
		MemGetBlockShiftAddr(display,Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_HSTA),Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_VSTA), &MemShiftAddr, &droppixel);
		if (dvrif_memory_compression_get_enable(SLR_SUB_DISPLAY) == TRUE)
		{
			TotalSize = memory_get_disp_line_size_compression(display,Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) + droppixel);	// calculate the memory size of capture
		}
		else
		{
			TotalSize = memory_get_disp_line_size1(display,Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) + droppixel);	// calculate the memory size of capture
		}
		IoReg_Mask32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg, 0xffc00000, (( ((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) +droppixel) & 0x3FFF) << 8) | ((fifoLen - (burstLen>>1))>>1)));

		TotalSize = memory_get_display_size(display);	// calculate the memory size of capture
	}

	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division

	// remainder is not allowed to be zero
	if (Remainder == 0) {
		Remainder = burstLen;
		Quotient -= 1;
	}

	rtd_pr_scaler_memory_debug("sub comp=%d, comp mode=%d\n", dvrif_memory_compression_get_enable(SLR_SUB_DISPLAY), dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) );

	if((dvrif_memory_compression_get_enable(SLR_SUB_DISPLAY) == TRUE) && (dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) == COMPRESSION_FRAME_MODE))
	{
		IoReg_Mask32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, 0x00000000, (0xffff << 16) | (burstLen << 8) | burstLen);
	}
	else
	{
		IoReg_Mask32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, 0x00000000, (Quotient << 16) | (burstLen << 8) | Remainder);
	}

	rtd_pr_scaler_memory_debug("TotalSize=%x, burstLen=%x, Quotient=%x, Remainder=%x\n", TotalSize, burstLen, Quotient, Remainder);

	//TotalSize = memory_get_capture_size(display, MEMCAPTYPE_LINE);
	//merlin4 memory do 96 bytes align @Crixus 20180321
	//TotalSize = dvr_size_alignment(TotalSize);
	/*TotalSize = Quotient * burstLen + Remainder;	// count one line, 64bits unit
	TotalSize = drvif_memory_get_data_align(TotalSize, 4);	// times of 4*/

	ulLineStep = TotalSize;

	if (dvrif_memory_compression_get_enable(display) == TRUE)
	{
		rtd_pr_scaler_memory_debug("Sub compress\n");
		if (dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) == COMPRESSION_LINE_MODE)
		{
			//merlin4 memory do 96 bytes align @Crixus 20180321
			ulLineStep = drvif_memory_get_data_align(TotalSize, DMA_SPEEDUP_ALIGN_VALUE/8);
			rtd_pr_scaler_memory_debug("ulLineStep=%x\n", ulLineStep);
		}
	}
	// Not compress, check if m-cap line mode?
	else if (MEMCAPTYPE_LINE == dvrif_memory_get_cap_mode(display))
	{
		rtd_pr_scaler_memory_debug("Sub not compres, cap line mode\n");
		//merlin4 memory do 96 bytes align @Crixus 20180321
		ulLineStep = drvif_memory_get_data_align(TotalSize, DMA_SPEEDUP_ALIGN_VALUE/8);
	}
	else{
		ulLineStep = memory_get_capture_size(display, MEMCAPTYPE_LINE);
	}


	rtd_pr_scaler_memory_debug("memory_set_sub_for_pip() Sub cap access mode=%x, display=%x ulLineStep=%x\n", dvrif_memory_get_cap_mode(display), display, ulLineStep);

	IoReg_Write32(MDOMAIN_DISP_DDR_SubLineStep_reg,  (ulLineStep + ulLineStep%2) << 3);//bit 3 need to be 0. rbus rule

	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits

	IoReg_Mask32(MDOMAIN_DISP_DDR_SubLineNum_reg, 0xfffff000, (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN) & 0x0FFF));

	MemSetDisplayAddr(SLR_SUB_DISPLAY, drvif_memory_get_block_addr(MEMIDX_SUB) + MemShiftAddr, drvif_memory_get_block_addr(MEMIDX_SUB_SEC) + MemShiftAddr);

	//update limit mode boundary
	drv_memory_disp_limit_boundary_setting(SLR_SUB_DISPLAY);
#ifdef ENABLE_3_BUFFER_DELAY_MODE
	IoReg_Write32(MDOMAIN_DISP_DDR_Sub3rdAddr_reg, (drvif_memory_get_block_addr(MEMIDX_SUB_THIRD) + MemShiftAddr)&0x7ffffff0);
#endif

	IoReg_Write32(MDOMAIN_DISP_DDR_SubAddrDropBits_reg,droppixel);
	/*IoReg_Mask32(MDOMAIN_DISP_DDR_SubBlkStep_reg, 0xff000000, 0);
	IoReg_WriteWord0(MDOMAIN_DISP_DDR_SubMPEn_reg,0);
	IoReg_WriteByte0(MDOMAIN_DISP_DDR_SubMPMode_reg, _BIT3);*/
	IoReg_SetBits(MDOMAIN_DISP_DDR_SubCtrl_reg, _BIT6);

	drvif_memory_set_dbuffer_write();
}


#endif //CONFIG_DUAL_CHANNEL  // There are two channels
#endif


UINT32 MemGetDDRLineWidth1(UINT8 display,UINT32 width)
{
	UINT8 ddrbit;
	UINT32 TotalSize;

	ddrbit =  (display ? 0 : ((Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_10BIT)) << 1 ))  | ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_422CAP));
	TotalSize = memory_get_line_size(width, (ddrbit & _BIT0), ((ddrbit & _BIT1) >> 1));
	TotalSize = drvif_memory_get_data_align(TotalSize, 0x04);	// it must be times of 4
	return TotalSize;

}

UINT32 memory_get_disp_line_size1(UINT8 display,UINT32 width)
{
	UINT32 TotalSize;
	UINT8 ddrbit;

	ddrbit =  (display ? 0 : (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_10BIT) << 1))  | ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_422CAP));
	TotalSize = memory_get_line_size(width, (ddrbit & _BIT0), ((ddrbit & _BIT1) >> 1));
	return TotalSize;
}


UINT32 memory_get_disp_line_size_compression(UINT8 display,UINT32 width)
{
	UINT32 TotalSize = 0;
	UINT8 ucCmpBit = 0;

	ucCmpBit = dvrif_memory_get_compression_bits(display);
	TotalSize = memory_get_capture_line_size_compression_by_width(width, ucCmpBit);
	return TotalSize;
}


void MemGetBlockShiftAddr(UINT8 display,UINT16 HOffset, UINT16 VOffset,UINT32 * StartAddr, UINT16 *DropAddr)
{
	UINT32 TotalSize;
//	UINT8 ddrbit;
//	UINT8 Q = 0;
	UINT8 ddrbit;
	UINT16 K, L,S;
	UINT16 m,n;
	UINT8 FIFOUnit = 64;
//	UINT16 Shift_FIFOUnit;
	UINT16 hFIFOUnit;
	UINT8 InputFormat;
	UINT8 BitNum;

	TotalSize = MemGetDDRLineWidth1(display,Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CAP_WID));
	ddrbit =  (display ? 0 : ((Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_10BIT)) << 1 ))  | ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_422CAP));
	InputFormat = (ddrbit & _BIT0);
	BitNum = (ddrbit & _BIT1) >> 1;

	if(InputFormat){
		if(BitNum) {		// 4:4:4 format     10bit
			K = 1920;
			L = (HOffset * 30) / K;
			S = (HOffset * 30 - L * K) / 30;

			m = HOffset * 30 / FIFOUnit;
			n = HOffset * 30 - m * FIFOUnit;

//			rtd_pr_scaler_memory_debug("\n 444_10 bits !");
		} else {			// 4:4:4 format     8bit
			K = 384;
			L = HOffset * 24 / K;
			S = (HOffset * 24 - L * K) / 24; //pixel

			m = HOffset * 24 / FIFOUnit;
			n = HOffset * 24 - m * FIFOUnit;


		}
	}else{
		if(BitNum) {		// 4:2:2 format     10bit
			K = 640;
			L = HOffset * 20 / K;
			S = (HOffset * 20 - L * K) / 20;

			m = HOffset * 20 / FIFOUnit;
			n = HOffset * 20 - m * FIFOUnit;

		} else {			// 4:2:2 format     8bit
			K = 128;
			L = HOffset * 16 / K;
			S = (HOffset * 16 - L * K) / 16;

			m = HOffset * 16 / FIFOUnit;
			n = HOffset * 16 - m * FIFOUnit;
		}
	}
#ifdef CONFIG_ENABLE_SE_ROTATE_M_DOMAIN_BLOCK_MODE
	if((display == SLR_MAIN_DISPLAY) && Get_rotate_function(SLR_MAIN_DISPLAY) && (get_rotate_mode(SLR_MAIN_DISPLAY)==(DIRECT_VO_FRAME_ORIENTATION)ROTATE_MODE_90) ){//block mode:8 bit
		K = 64;
		L = (HOffset * 8) / K;
		S = (HOffset * 8 - L * K) / 8;

		m = HOffset * 8 / FIFOUnit;
		n = HOffset * 8 - m * FIFOUnit;
		TotalSize = drvif_memory_get_data_align((Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_WID))*8, 64);
		TotalSize = (unsigned int)SHR(TotalSize, 6);	// (pixels of per line * 20 / 64), unit: 64bits
		TotalSize = drvif_memory_get_data_align(TotalSize, 0x04);
	}
#endif
	if (n != 0)
		m = m + 1;


	hFIFOUnit =  (L * K) / FIFOUnit;

//	Shift_FIFOUnit = m - hFIFOUnit;

	TotalSize = TotalSize * VOffset + hFIFOUnit;
	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits

	*StartAddr = TotalSize;
	*DropAddr = S;

//	return Shift_FIFOUnit;

}
 #ifndef BUILD_QUICK_SHOW

#ifdef CONFIG_MPIXEL_SHIFT
void memory_v_shift(int v)
{
	unsigned int ucStartAddress;
	unsigned int ucStartAddress2;
	unsigned int TotalSize;
	ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN);
	ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC);
	TotalSize = memory_get_capture_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), MEMCAPTYPE_LINE);		// calculate capture line size
	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits

	if (v >= 0)
	{
		TotalSize = TotalSize * v;
		ucStartAddress +=TotalSize;
		ucStartAddress2 +=TotalSize;
	}
	else
	{
		TotalSize = TotalSize * (-v);
		ucStartAddress -=TotalSize;
		ucStartAddress2 -=TotalSize;
	}

	rtd_pr_scaler_memory_debug("%d %x %x \n",v , ucStartAddress,ucStartAddress2);

	IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg, ucStartAddress&0x7ffffff0);
		//rtd_pr_scaler_memory_debug("After %s %d IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg)=%x\n",__FUNCTION__,__LINE__,IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg));
	IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg, ucStartAddress2&0x7ffffff0);
	IoReg_SetBits(MDOMAIN_DISP_DDR_MainSubCtrl_reg, _BIT0);
}

void memory_black_buf_init(unsigned int framesize,unsigned int  upperPhyAddress)
{
	unsigned int ucStartAddress;
	unsigned int ucStartAddress2;
	unsigned int upperVirtualAddr;
	unsigned int lowerPhyAddress,lowerVirtualAddr;
	unsigned int *lowerCurAddr,*upperCurAddr;
	unsigned int TotalSize;
	unsigned int i = 0;
	//upperPhyAddress = drvif_memory_get_block_addr(MEMIDX_MAIN);
	TotalSize = memory_get_capture_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), MEMCAPTYPE_LINE);		// calculate capture line size
	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits
	TotalSize = TotalSize * 32;

	rtd_pr_scaler_memory_debug("address %x %x %d\n",upperPhyAddress,AllocStartAddr,get_scaler_memory_map_addr());

	upperVirtualAddr = get_scaler_memory_map_addr() + (upperPhyAddress - AllocStartAddr);

	lowerPhyAddress = upperPhyAddress + TotalSize + framesize;

	lowerVirtualAddr = get_scaler_memory_map_addr() + (lowerPhyAddress - AllocStartAddr);


	upperCurAddr  = (unsigned int*)upperVirtualAddr;
	lowerCurAddr  = (unsigned int*)lowerVirtualAddr;

	rtd_pr_scaler_memory_debug("address %x %x\n",lowerPhyAddress,lowerVirtualAddr);

	for(i = 0 ; i < TotalSize ; i= i + 4)
	{

		*upperCurAddr = 0x80008000;
		*lowerCurAddr = 0x80008000;

		upperCurAddr++;
		lowerCurAddr++;
	}

}


#endif
#endif
/*============================================================================*/
/**
 * MemSetMain
 * This function is used to set Main control register
 *
 * @param <info> { display-info struecture }
 * @param <Block_Step> { Block step }
 * @return { none }
 * @note
 * If any one of the Memory-main registers is changed, it is necessary to write
 * P3_8B register to validate the update.
 *
 */
extern unsigned char Get_3d_type(void);

void memory_set_main_3D_setting(unsigned int LineStep)
{
	unsigned int TotalSize;

	mdomain_disp_ddr_mainblkstep_RBUS ddr_mainblkstep_reg;
	mdomain_disp_ddr_mainctrl_RBUS ddr_mainctrl_reg;

	//mdomain_disp_ddr_mainsubctrl_RBUS ddr_mainsubctrl_reg;
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()
		&& ((Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_TOP_AND_BOTTOM)||(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING)))
	{

		TotalSize = LineStep * (Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MEM_LEN_PRE)>>1);//for overscan modify

		if (Get_PANEL_VFLIP_ENABLE()&&(Scaler_InputSrcGetMainChType() != _SRC_VO) )
			TotalSize=(~TotalSize)+1;

		Scaler_Disp3dSetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_BLKSTEP,TotalSize);
		ddr_mainblkstep_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainBlkStep_reg);
		rtd_pr_scaler_memory_debug("[DBG][TaB] Step Line/Block1=[%x/%x->%x(%d)]\n", LineStep, ddr_mainblkstep_reg.main_block_step, TotalSize, Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MEM_LEN_PRE));
		ddr_mainblkstep_reg.main_block_step = TotalSize;

		// PR output enable
		ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
#ifndef CONFIG_DARWIN_SCALER
		ddr_mainctrl_reg.main_pr_our_merge_en = 1;
#endif
		ddr_mainctrl_reg.main_merge_en0 = 1;
		ddr_mainctrl_reg.main_double_en = 1;
		IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg,IoReg_Read32(MDOMAIN_CAP_DDR_In1Addr_reg)&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg,IoReg_Read32(MDOMAIN_CAP_DDR_In1_2ndAddr_reg)&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg,IoReg_Read32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg)&0x7ffffff0);
		rtd_pr_scaler_memory_debug("\n@@PR TBFP[%d] merge enable\n", Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE));
	}
	else
	{
		ddr_mainblkstep_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainBlkStep_reg);
		//rtd_pr_scaler_memory_debug("[DBG][TaB] Step Line/Block1=[%x/%x->%x]\n", LineStep, ddr_mainblkstep_reg.main_block_step, TotalSize);
		ddr_mainblkstep_reg.main_block_step = 0;

		// PR output enable
		ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
#ifndef CONFIG_DARWIN_SCALER
		ddr_mainctrl_reg.main_pr_our_merge_en = 0;
#endif
		ddr_mainctrl_reg.main_merge_en0 = 0;
		ddr_mainctrl_reg.main_double_en = 0;
		//rtd_pr_scaler_memory_debug("\n@@PR SBS  merge enable\n");
	}
	ddr_mainblkstep_reg.main_block_step = (ddr_mainblkstep_reg.main_block_step % 2)? ddr_mainblkstep_reg.main_block_step + 1 : ddr_mainblkstep_reg.main_block_step;
	//bit 3 need to be 0. rbus rule
	IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg, ddr_mainblkstep_reg.regValue);
	IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg, ddr_mainctrl_reg.regValue);
//frank@0617 mark below code to speed up,below code will setting by memory_set_input_format
	drvif_memory_set_dbuffer_write();

	if(drvif_memory_set_dbuffer_write() == 0)
		rtd_pr_scaler_memory_debug("\n*** ERR: M-disp write fail@%s, %x ***\n", __FUNCTION__, IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg));
	//else
	//	rtd_pr_scaler_memory_debug("\n@@DDR_MAINCTRL=%x\n",IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg));
}


 #ifndef BUILD_QUICK_SHOW


// [SG] SG 3D in data FRC mode -- sequence frame 3D L-R output control
void memory_set_main_SG_3D_setting(unsigned int LineStep, unsigned int Addr1, unsigned int Addr2, unsigned int Addr3)
{
	unsigned int TotalSize;
	unsigned int rFrameOffset=0;
	//SCALER_DISP_CHANNEL display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	//mdomain_disp_ddr_mainctrl_RBUS ddr_mainctrl_reg;

	if(drvif_scaler3d_decide_3d_SG_data_FRC()){
		TotalSize = LineStep * Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_MEM_ACT_LEN);
		rFrameOffset = TotalSize << 3;
		if(rFrameOffset != (drvif_memory_get_block_size(MEMIDX_MAIN) >> 1))
			rtd_pr_scaler_memory_debug("[SG] BlkStep cap/blk=%x/%x@IDMA\n", rFrameOffset, drvif_memory_get_block_size(MEMIDX_MAIN) >> 1);

		IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_1ST_Addr_reg, Addr1 + rFrameOffset);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_2ND_Addr_reg, Addr2 + rFrameOffset);
		//rtd_pr_scaler_memory_debug("[SG][3D][FRC] R.1st/2nd=%x/%x(L+%d KB)\n", Addr1 + rFrameOffset, Addr2 + rFrameOffset, rFrameOffset>>10);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_3RD_Addr_reg, Addr3 + rFrameOffset);
		rtd_pr_scaler_memory_debug("[SG][2D3D][FRC] R.1st/2nd/3rd=%x/%x/%x\n", Addr1, Addr2, Addr3);
	}else if(drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() && drvif_scaelr3d_decide_is_3D_display_mode()){
		mdomain_disp_ddr_mainblkstep_RBUS ddr_mainblkstep_reg;

		// M-domain 3D format convert -- 2Dcvt3D mode
		if(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_2D_CVT_3D){
			// L/R frame use the same frame
			rFrameOffset = 0;
		} // M-domain 3D format convert -- TaB 3D mode
		else if(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_TOP_AND_BOTTOM || Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING){
			// R-frame start position = 1/2 capture buffer
			TotalSize = (LineStep * Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_MEM_ACT_LEN));
			Scaler_Disp3dSetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_BLKSTEP,TotalSize);

			ddr_mainblkstep_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainBlkStep_reg);
			rtd_pr_scaler_memory_debug("[DBG][TaB2] Step Line/Block1=[%x/%x->%x(%d)]\n", LineStep, ddr_mainblkstep_reg.main_block_step, TotalSize, Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MEM_LEN_PRE));
			ddr_mainblkstep_reg.main_block_step = TotalSize;

			rFrameOffset = TotalSize << 3;
			if(rFrameOffset != (drvif_memory_get_block_size(MEMIDX_MAIN) >> 1))
				rtd_pr_scaler_memory_debug("[SG] BlkStep cap/blk=%x/%x@TaB\n", rFrameOffset, drvif_memory_get_block_size(MEMIDX_MAIN) >> 1);

		} // M-domain 3D format convert -- SBS 3D mode
		else{
			// R-frame start position = capture buffer + 1/2 line width
			TotalSize = (LineStep  >> 1);
			Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_BLKSTEP,TotalSize);

			ddr_mainblkstep_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainBlkStep_reg);
			rtd_pr_scaler_memory_debug("[DBG][SBS] Step Line/Block1=[%x/%x->%x(%d)]\n", LineStep, ddr_mainblkstep_reg.main_block_step, TotalSize, Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MEM_LEN_PRE));
			ddr_mainblkstep_reg.main_block_step = TotalSize;

			rFrameOffset = TotalSize << 3;
			if(rFrameOffset != (drvif_memory_get_block_size(MEMIDX_MAIN) >> 1))
				rtd_pr_scaler_memory_debug("[SG] BlkStep cap/blk=%x/%x@SBS\n", rFrameOffset, drvif_memory_get_block_size(MEMIDX_MAIN) >> 1);
		}
		ddr_mainblkstep_reg.main_block_step = (ddr_mainblkstep_reg.main_block_step % 2)? ddr_mainblkstep_reg.main_block_step + 1 : ddr_mainblkstep_reg.main_block_step;
		//bit 3 need to be 0. rbus rule
		if(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_2D_CVT_3D){
			IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_1ST_Addr_reg, Addr1 + rFrameOffset);
			IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_2ND_Addr_reg, Addr2 + rFrameOffset);
			IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_3RD_Addr_reg, Addr3 + rFrameOffset);
			IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg, ddr_mainblkstep_reg.regValue);
		}else{
			// non-IDMA SG 3D mode L/R order need do invert
			IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg,(Addr1 + rFrameOffset)&0x7ffffff0);
			IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg,(Addr2 + rFrameOffset)&0x7ffffff0);
			IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg,(Addr3 + rFrameOffset)&0x7ffffff0);

			IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_1ST_Addr_reg, Addr1);
			IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_2ND_Addr_reg, Addr2);
			IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_3RD_Addr_reg, Addr3);

			IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg, ddr_mainblkstep_reg.regValue);
		}

		rtd_pr_scaler_memory_debug("[SG][2D3D][FRC] R.1st/2nd/3rd=%x/%x/%x\n", Addr1, Addr2, Addr3);
	}

	return;
}

void memory_set_main_3D_setting_dispchange(unsigned int LineStep)
{
	SCALER_DISP_CHANNEL display = SLR_SUB_DISPLAY;
	unsigned int TotalSize;

	mdomain_disp_ddr_mainblkstep_RBUS ddr_mainblkstep_reg;
	mdomain_disp_ddr_mainctrl_RBUS ddr_mainctrl_reg;

	//mdomain_disp_ddr_mainsubctrl_RBUS ddr_mainsubctrl_reg;
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()
		&& ((Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_TOP_AND_BOTTOM)||(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING)))
	{

		TotalSize = LineStep * (Scaler_DispGetInputInfoByDisp(display, (SLR_INPUT_INFO)SLR_DISP_3D_MEM_LEN_PRE)>>1);//for overscan modify

		if (Get_PANEL_VFLIP_ENABLE()&&(Scaler_InputSrcGetMainChType() != _SRC_VO) )
			TotalSize=(~TotalSize)+1;

		Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_BLKSTEP,TotalSize);
		ddr_mainblkstep_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainBlkStep_reg);
		rtd_pr_scaler_memory_debug("[DBG][TaB] Step Line/Block1=[%x/%x->%x(%d)]\n", LineStep, ddr_mainblkstep_reg.main_block_step, TotalSize, Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MEM_LEN_PRE));
		ddr_mainblkstep_reg.main_block_step = TotalSize;

		// PR output enable
		ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
#ifndef CONFIG_DARWIN_SCALER
		ddr_mainctrl_reg.main_pr_our_merge_en = 1;
#endif
		ddr_mainctrl_reg.main_merge_en0 = 1;
		ddr_mainctrl_reg.main_double_en = 1;
		IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg,IoReg_Read32(MDOMAIN_CAP_DDR_In1Addr_reg)&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg,IoReg_Read32(MDOMAIN_CAP_DDR_In1_2ndAddr_reg)&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg,IoReg_Read32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg)&0x7ffffff0);
		rtd_pr_scaler_memory_debug("\n@@PR TBFP[%d] merge enable\n", Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE));
	}
	else
	{
		ddr_mainblkstep_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainBlkStep_reg);
		//rtd_pr_scaler_memory_debug("[DBG][TaB] Step Line/Block1=[%x/%x->%x]\n", LineStep, ddr_mainblkstep_reg.main_block_step, TotalSize);
		ddr_mainblkstep_reg.main_block_step = 0;
		// PR output enable
		ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
		ddr_mainctrl_reg.main_pr_our_merge_en = 0;
		ddr_mainctrl_reg.main_merge_en0 = 0;
		ddr_mainctrl_reg.main_double_en = 0;
		//rtd_pr_scaler_memory_debug("\n@@PR SBS  merge enable\n");
	}
	ddr_mainblkstep_reg.main_block_step = (ddr_mainblkstep_reg.main_block_step % 2)? ddr_mainblkstep_reg.main_block_step + 1 : ddr_mainblkstep_reg.main_block_step;
	//bit 3 need to be 0. rbus rule
	IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg, ddr_mainblkstep_reg.regValue);
	IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg, ddr_mainctrl_reg.regValue);

}




// [SG] SG 3D in data FRC mode -- sequence frame 3D L-R output control
void memory_set_main_SG_3D_setting_dispchange(unsigned int LineStep, unsigned int Addr1, unsigned int Addr2, unsigned int Addr3)
{
	unsigned int TotalSize;
	unsigned int rFrameOffset=0;
	//mdomain_disp_ddr_mainctrl_RBUS ddr_mainctrl_reg;

	if(drvif_scaler3d_decide_3d_SG_data_FRC()){
		TotalSize = LineStep * Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_MEM_ACT_LEN);
		rFrameOffset = TotalSize << 3;
		if(rFrameOffset != (drvif_memory_get_block_size(MEMIDX_MAIN) >> 1))
			rtd_pr_scaler_memory_debug("[SG] BlkStep cap/blk=%x/%x@IDMA\n", rFrameOffset, drvif_memory_get_block_size(MEMIDX_MAIN) >> 1);

		IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_1ST_Addr_reg, Addr1 + rFrameOffset);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_2ND_Addr_reg, Addr2 + rFrameOffset);
		//rtd_pr_scaler_memory_debug("[SG][3D][FRC] R.1st/2nd=%x/%x(L+%d KB)\n", Addr1 + rFrameOffset, Addr2 + rFrameOffset, rFrameOffset>>10);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_3RD_Addr_reg, Addr3 + rFrameOffset);
		rtd_pr_scaler_memory_debug("[SG][2D3D][FRC] R.1st/2nd/3rd=%x/%x/%x\n", Addr1, Addr2, Addr3);
	}else if(drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() && drvif_scaelr3d_decide_is_3D_display_mode()){
		mdomain_disp_ddr_mainblkstep_RBUS ddr_mainblkstep_reg;

		// M-domain 3D format convert -- 2Dcvt3D mode
		if(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_2D_CVT_3D){
			// L/R frame use the same frame
			rFrameOffset = 0;
		} // M-domain 3D format convert -- TaB 3D mode
		else if(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_TOP_AND_BOTTOM || Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING){
			// R-frame start position = 1/2 capture buffer
			TotalSize = (LineStep * Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_MEM_ACT_LEN));
			Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_BLKSTEP,TotalSize);

			ddr_mainblkstep_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainBlkStep_reg);
			rtd_pr_scaler_memory_debug("[DBG][TaB2] Step Line/Block1=[%x/%x->%x(%d)]\n", LineStep, ddr_mainblkstep_reg.main_block_step, TotalSize, Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MEM_LEN_PRE));
			ddr_mainblkstep_reg.main_block_step = TotalSize;

			rFrameOffset = TotalSize << 3;
			if(rFrameOffset != (drvif_memory_get_block_size(MEMIDX_MAIN) >> 1))
				rtd_pr_scaler_memory_debug("[SG] BlkStep cap/blk=%x/%x@TaB\n", rFrameOffset, drvif_memory_get_block_size(MEMIDX_MAIN) >> 1);

		} // M-domain 3D format convert -- SBS 3D mode
		else{
			// R-frame start position = capture buffer + 1/2 line width
			TotalSize = (LineStep  >> 1);
			Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_BLKSTEP,TotalSize);

			ddr_mainblkstep_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainBlkStep_reg);
			rtd_pr_scaler_memory_debug("[DBG][SBS] Step Line/Block1=[%x/%x->%x(%d)]\n", LineStep, ddr_mainblkstep_reg.main_block_step, TotalSize, Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MEM_LEN_PRE));
			ddr_mainblkstep_reg.main_block_step = TotalSize;

			rFrameOffset = TotalSize << 3;
			if(rFrameOffset != (drvif_memory_get_block_size(MEMIDX_MAIN) >> 1))
				rtd_pr_scaler_memory_debug("[SG] BlkStep cap/blk=%x/%x@SBS\n", rFrameOffset, drvif_memory_get_block_size(MEMIDX_MAIN) >> 1);
		}
		ddr_mainblkstep_reg.main_block_step = (ddr_mainblkstep_reg.main_block_step % 2)? ddr_mainblkstep_reg.main_block_step + 1 : ddr_mainblkstep_reg.main_block_step;
		//bit 3 need to be 0. rbus rule
		if(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_2D_CVT_3D){
			IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_1ST_Addr_reg, Addr1 + rFrameOffset);
			IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_2ND_Addr_reg, Addr2 + rFrameOffset);
			IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_3RD_Addr_reg, Addr3 + rFrameOffset);
			IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg, ddr_mainblkstep_reg.regValue);
		}else{
			// non-IDMA SG 3D mode L/R order need do invert
			IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg,(Addr1 + rFrameOffset)&0x7ffffff0);
			IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg,(Addr2 + rFrameOffset)&0x7ffffff0);
			IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg,(Addr3 + rFrameOffset)&0x7ffffff0);

			IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_1ST_Addr_reg, Addr1);
			IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_2ND_Addr_reg, Addr2);
			IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_3RD_Addr_reg, Addr3);

			IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg, ddr_mainblkstep_reg.regValue);
		}

		rtd_pr_scaler_memory_debug("[SG][2D3D][FRC] R.1st/2nd/3rd=%x/%x/%x\n", Addr1, Addr2, Addr3);
	}

	return;
}

#undef ABS
#define ABS(X)			((X)<0 ? (-X) : (X))
void memory_set_MainandSub_ShowONPR(void)
{
	//unsigned int TotalSize;
	//ddr_mainblkstep_RBUS ddr_mainblkstep_reg;
	mdomain_disp_ddr_mainctrl_RBUS ddr_mainctrl_reg;
	//Crixus@20140722 - Mac3 no in2
	//TotalSize = ABS(IoReg_Read32(MDOMAIN_CAP_DDR_In2Addr_reg) -IoReg_Read32(MDOMAIN_CAP_DDR_In1Addr_reg));
	//IoReg_Write32(MDOMAIN_DISP_DDR_MAINBLKSTEP_VADDR, TotalSize);

	// PR output enable
	ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
	ddr_mainctrl_reg.main_pr_our_merge_en = 1;
	ddr_mainctrl_reg.main_merge_en0 = 1;
	ddr_mainctrl_reg.main_double_en = 1;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg, ddr_mainctrl_reg.regValue);

	drvif_memory_set_dbuffer_write();
	if(drvif_memory_set_dbuffer_write() == 0)
		rtd_pr_scaler_memory_debug("\n*** ERR: M-disp write fail@%s, %x ***\n", __FUNCTION__, IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg));
	else
		rtd_pr_scaler_memory_debug("\n@@DDR_MAINCTRL=%x\n",IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg));

}
#endif
extern unsigned char Get_vscFilmMode(void);
extern void filmmode_videofw_config(unsigned int filmmode, unsigned int buf4addr);
void memory_set_main(unsigned char bDouble)
{
	unsigned char display = SLR_MAIN_DISPLAY;
	unsigned int TotalSize;
//	unsigned int  TotalSizeFrame=0;
	unsigned int Quotient;
	unsigned char Remainder;
	unsigned int ucStartAddress;
	unsigned int ucStartAddress2=0;
	unsigned int ucStartAddress3=0;
	unsigned int LineStep;
	unsigned int LineStep_96Align = 0;
	mdomain_disp_ddr_mainprevstart_RBUS ddr_mainprevstart_reg;
	unsigned int comp_width = 0;
	unsigned int fifoLen = _FIFOLENGTH;
	unsigned int burstLen = _BURSTLENGTH;
	//Update the fifo depth for each IC @Crixus 20170418
	#ifdef CONFIG_3D_ASPECT_RATIO
	UINT32 MemShiftAddr = 0;
	UINT16 droppixel = 0;
	#endif
	StructSrcRect m_dispwin;
	display = (Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) ? SLR_MAIN_DISPLAY : SLR_SUB_DISPLAY;
#ifdef MERLIN_FIFO_DEPTH
	//frank@0325 double the M domain burst length to avoid YUV444 peak BW not enough
	if((display == SLR_MAIN_DISPLAY) && ( !Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP))){
		burstLen=0xf0; //align 96-bytes
	}
#endif

	m_dispwin = Scaler_DispWindowGet();

	ddr_mainprevstart_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainPreVStart_reg);
	if((vsc_get_adaptivestream_flag(SLR_MAIN_DISPLAY)) ||(m_dispwin.srcy <= 100)) {
		ddr_mainprevstart_reg.main_pre_rd_v_start = 7;
	} else {
		ddr_mainprevstart_reg.main_pre_rd_v_start = (m_dispwin.srcy-10);
	}

	IoReg_Write32(MDOMAIN_DISP_DDR_MainPreVStart_reg, ddr_mainprevstart_reg.regValue);

	IoReg_Write32(MDOMAIN_DISP_DDR_MainAddrDropBits_reg,0x0);

	IoReg_Mask32(MDOMAIN_DISP_DDR_MainLineNum_reg, 0xfffff000, (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN) & 0x0FFF));

#ifdef CONFIG_3D_ASPECT_RATIO
	rtd_pr_scaler_memory_debug("mem act info %d %d %d %d\n",Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));

	if ((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA) == 0) && (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) == 0) )
	{

		if((display == SLR_MAIN_DISPLAY) && (dvrif_memory_compression_get_enable(display) == TRUE)){
			comp_width = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID);
			//new PQDC are all align 128, use original width. @Crixus 20170615
			IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, ((comp_width & 0x0fFF) << 8) | ((fifoLen-(burstLen>>1))>>1));
			TotalSize = memory_get_capture_line_size_compression(dvrif_memory_get_compression_bits(display));
		}
		else{
			IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, ((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) & 0x0fFF) << 8) | ((fifoLen-(burstLen>>1))>>1));
		TotalSize = memory_get_display_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY));	// calculate the memory size of capture
#ifdef CONFIG_ENABLE_SE_ROTATE_M_DOMAIN_BLOCK_MODE
		if((display == SLR_MAIN_DISPLAY) && Get_rotate_function(SLR_MAIN_DISPLAY)) {
			TotalSize = drvif_memory_get_data_align(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID)*8, 64);
			TotalSize = (unsigned int)SHR(TotalSize, 6);	// (pixels of per line * 24 / 64), unit: 64bits
	}
#endif

	}

	}
	else
	{
		MemGetBlockShiftAddr(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA), &MemShiftAddr, &droppixel);
	#ifdef CONFIG_ENABLE_SE_ROTATE_M_DOMAIN_BLOCK_MODE
		if((display == SLR_MAIN_DISPLAY) && Get_rotate_function(SLR_MAIN_DISPLAY)) {
			TotalSize = drvif_memory_get_data_align((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) + droppixel)*8, 64);
			TotalSize = (unsigned int)SHR(TotalSize, 6);	// (pixels of per line * 24 / 64), unit: 64bits
		} else
	#endif
		TotalSize = memory_get_disp_line_size1(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) + droppixel);	// calculate the memory size of capture
		IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, (( ((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) +droppixel) & 0x0fFF) << 8) | ((fifoLen-(burstLen>>1))>>1)));
		rtd_pr_scaler_memory_debug("drop pixel=%d shift addr=%x\n",droppixel,MemShiftAddr);

	}
#if 0
	if(droppixel != 0){
		UINT32 drop_line = 0;
		drop_line = droppixel * Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN);
		drop_line = drop_line / (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) + droppixel) + 1;
		rtd_pr_scaler_memory_emerg("@@@@@@@@@@@@drop_line = %d\n",drop_line);
		IoReg_Mask32(MDOMAIN_DISP_DDR_MainLineNum_reg, 0xfffff000, ((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN) - drop_line)& 0x0FFF));
	}
#endif

#else

	IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, ((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) & 0x0fFF) << 8) | ((fifoLen-(burstLen>>1))>>1));

	if((display == SLR_MAIN_DISPLAY) && (dvrif_memory_compression_get_enable(display) == TRUE)){
		TotalSize = memory_get_capture_line_size_compression(dvrif_memory_get_compression_bits(display));
		//merlin4 memory do 96 bytes align @Crixus 20180321
		//TotalSize = dvr_size_alignment(TotalSize);
	}
	else{
		TotalSize = memory_get_display_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY));	// calculate the memory size of capture
	}
#endif
	if((display == SLR_MAIN_DISPLAY) && Get_rotate_function(SLR_MAIN_DISPLAY)) {
		
	mdomain_vi_main_inten_RBUS vi_inten_reg; //MDOM_VI_INTEN_VADDR
	vi_inten_reg.regValue = rtd_inl(MDOMAIN_VI_MAIN_INTEN_reg);
	vi_inten_reg.video_in_last_wr_c_flag_ie = 1;
	rtd_outl(MDOMAIN_VI_MAIN_INTEN_reg, vi_inten_reg.regValue);

	}
	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division

	//rtd_pr_scaler_memory_emerg("TotalSize=%x, burstLen=%x, Quotient=%x, Remainder=%x\n", TotalSize, burstLen, Quotient, Remainder);

	// remainder is not allowed to be zero
	if (Remainder == 0) {
		Remainder = burstLen;
		Quotient -= 1;
	}

	if((dvrif_memory_compression_get_enable(display) == TRUE) && (dvrif_memory_get_compression_mode(display) == COMPRESSION_FRAME_MODE)){
		//Frame mode and PQC: num is 0xffff, length = remain
		//Need to set maxmun because can not sure how many pqc data on frame mode. @Crixus 20170626
		IoReg_Write32(MDOMAIN_DISP_DDR_MainRdNumLenRem_reg, (0xffff << 16) | (burstLen << 8) | burstLen);
	}
	else{
		IoReg_Write32(MDOMAIN_DISP_DDR_MainRdNumLenRem_reg, (Quotient << 16) | (burstLen << 8) | Remainder);
	}

	if((display == SLR_MAIN_DISPLAY) && (dvrif_memory_compression_get_enable(display) == TRUE))
		TotalSize = memory_get_capture_line_size_compression(dvrif_memory_get_compression_bits(display));
	else
		TotalSize = memory_get_capture_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), MEMCAPTYPE_LINE);// calculate capture line size


	//rtd_pr_scaler_memory_emerg("file=%s, function=%s, line=%d\n", __FILE__, __FUNCTION__, __LINE__);
	//rtd_pr_scaler_memory_emerg("TotalSize=%x\n", TotalSize);

#ifdef CONFIG_ENABLE_SE_ROTATE_M_DOMAIN_BLOCK_MODE
	if((display == SLR_MAIN_DISPLAY) && Get_rotate_function(SLR_MAIN_DISPLAY)) {
		TotalSize = drvif_memory_get_data_align(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID)*8, 64);
		TotalSize = (unsigned int)SHR(TotalSize, 6);	// (pixels of per line * 24 / 64), unit: 64bits
		TotalSize = drvif_memory_get_data_align(TotalSize, 0x04);
	}
#endif

	LineStep = TotalSize;
	LineStep_96Align = TotalSize;
	//rtd_pr_scaler_memory_emerg("Line Step=0x%x\n",LineStep);
	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits


	if (dvrif_memory_compression_get_enable(display) == TRUE)
	{
		//rtd_pr_scaler_memory_emerg("file=%s, function=%s, line=%d\n", __FILE__, __FUNCTION__, __LINE__);
		//rtd_pr_scaler_memory_emerg("m-cap compress enable\n");
		if (dvrif_memory_get_compression_mode(display) == COMPRESSION_LINE_MODE)
		{
			LineStep_96Align = LineStep;//drvif_memory_get_data_align(LineStep, DMA_SPEEDUP_ALIGN_VALUE/8);
			//rtd_pr_scaler_memory_emerg("m-cap compress line mode\n");
		}
		else
		{
			//rtd_pr_scaler_memory_emerg("file=%s, function=%s, line=%d\n", __FILE__, __FUNCTION__, __LINE__);
			//rtd_pr_scaler_memory_emerg("m-cap compress frame mode\n");
		}
	}
	// Not compress, check if m-cap line mode?
	else if (MEMCAPTYPE_LINE == dvrif_memory_get_cap_mode(display))
	{
		LineStep_96Align = LineStep;//drvif_memory_get_data_align(LineStep, DMA_SPEEDUP_ALIGN_VALUE/8);
		//rtd_pr_scaler_memory_emerg("file=%s, function=%s, line=%d\n", __FILE__, __FUNCTION__, __LINE__);
		//rtd_pr_scaler_memory_emerg("m-cap not compress line mode\n");
	}


	IoReg_Write32(MDOMAIN_DISP_DDR_MainLineStep_reg, (LineStep_96Align + LineStep_96Align%2) << 3);//bit 3 need to be 0. rbus rule

#if 0
	if(TRUE==get_delayframe_fuc())
	{
		ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN) + MemShiftAddr;
		if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) != SLR_3D_2D_ONLY))
		{
			MemDispStartAddr_3D[0]=ucStartAddress;
		}
		bDouble = FALSE;
	}
	else
#endif
	{
		ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN) + MemShiftAddr;

		ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + MemShiftAddr;

#if 0
#ifdef CONFIG_ENABLE_3BUFFER_NEW_RULE
		if(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)>650)
		{
			ucStartAddress3 = drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) + MemShiftAddr;
			IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg, ucStartAddress3&0x7ffffff0);
		}
		else if(_ENABLE == Scaler_Get_Enable_3Buffer_NewRuleCondition())
		{
			if(_TRUE == Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_VFLIP3))
				{
				ucStartAddress3 = drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) + MemShiftAddr;
				IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg, ucStartAddress3&0x7ffffff0);
			}
		}
#else //#ifdef CONFIG_ENABLE_3BUFFER_NEW_RULE
		//M-3buf condition:
		// 1. [2D] M-capture frame rate > M-display frame rate
		// 2. [3D][SG] FRC in M-domain, M-display need repeat frame 2 times
		// 3. [3D][SG] SG 3D display timing FRC need enable triple buffer to avoid frame tear issue
		// 4. [3D][PR] PR to SG 3D display timing FRC need enable triple buffer to avoid frame tear issue
		if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)>650) ||Get_vscFilmMode())
		{
			ucStartAddress3 = drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) + MemShiftAddr;
			IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg, ucStartAddress3&0x7ffffff0);
			MemDispStartAddr_3D[2]=ucStartAddress3;
		}
#endif //#ifdef CONFIG_ENABLE_3BUFFER_NEW_RULE
#endif
	}
#ifdef ENABLE_3_BUFFER_DELAY_MODE
	ucStartAddress3 = drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) + MemShiftAddr;
	IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg, ucStartAddress3&0x7ffffff0);
	MemDispStartAddr_3D[2]=ucStartAddress3;
#endif

	IoReg_Write32(MDOMAIN_DISP_DDR_MainAddrDropBits_reg,droppixel);

	Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_DISPLAY_SIZE, memory_get_display_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)));

	MemSetDisplayAddr(SLR_MAIN_DISPLAY, ucStartAddress, ucStartAddress2);	// Main disaply not need set second block start address

	//Update s-cap limit boundary
	drv_memory_disp_limit_boundary_setting(SLR_MAIN_DISPLAY);




	//rtd_pr_scaler_memory_debug("%s %d bDouble=%d\n",__FUNCTION__,__LINE__,bDouble);
	if(bDouble) {	// use double buffer
		//TotalSize = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) - drvif_memory_get_block_addr(MEMIDX_MAIN);
		//TotalSize = drvif_memory_get_data_align(TotalSize, _DDR_BANK_ADDR_ALIGNMENT);
		//TotalSize = SHR(TotalSize, _DDR_ROW_BITS); //row alignment and get the block step

		TotalSize = LineStep * Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_MEM_ACT_LEN);
		IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg, (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule
	}
#if 0
	if((get_mainsubonpr_fuc()==TRUE)&&(get_MainSubonPR_curState()==MAINSUB_SHOWONPR_INIT_MAIN))
	{
		memory_set_MainandSub_ShowONPR();
	}
#endif
	Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MAIN_ADDR, ucStartAddress);
	Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MAIN_SEC_ADDR, ucStartAddress2);
	Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MAIN_THIRD_ADDR, ucStartAddress3);

}

 #ifndef BUILD_QUICK_SHOW

void zoom_memory_set_main(unsigned char bDouble,unsigned char buffernum)

{
	unsigned int TotalSize;
//	unsigned int  TotalSizeFrame=0;
	unsigned int Quotient;
	unsigned char Remainder;
	unsigned int ucStartAddress;
	unsigned int ucStartAddress2;
	unsigned int LineStep;
	unsigned int fifoLen = _FIFOLENGTH;
	unsigned int burstLen = _BURSTLENGTH;
	StructSrcRect m_dispwin;
	mdomain_disp_ddr_mainprevstart_RBUS ddr_mainprevstart_reg;

	//frank@0325 double the M domain burst length to avoid 444 10 bits peak BW not enough
	if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)_CHANNEL1, SLR_DISP_422CAP))){
		burstLen=0xf0;
	}

	m_dispwin = Scaler_DispWindowGet();
	ddr_mainprevstart_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainPreVStart_reg);
	if(m_dispwin.srcy <= 100) {
		ddr_mainprevstart_reg.main_pre_rd_v_start = 7;
	} else {
		ddr_mainprevstart_reg.main_pre_rd_v_start = (m_dispwin.srcy-10);
	}
	IoReg_Write32(MDOMAIN_DISP_DDR_MainPreVStart_reg, ddr_mainprevstart_reg.regValue);

	//frank@0323 change waterlevel/2 for AT,because water level nit is 128 bits
	IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, ((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) & 0x0fFF) << 8) | ((fifoLen-(burstLen>>1))>>1));
	IoReg_Mask32(MDOMAIN_DISP_DDR_MainLineNum_reg, 0xfffff000, (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN) & 0x0FFF));


	TotalSize = memory_get_display_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY));	// calculate the memory size of capture
	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division

	// remainder is not allowed to be zero
	if (Remainder == 0) {
		Remainder = burstLen;
		Quotient -= 1;
	}

	IoReg_Write32(MDOMAIN_DISP_DDR_MainRdNumLenRem_reg, (Quotient << 16) | (burstLen << 8) | Remainder);

	TotalSize = memory_get_capture_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), MEMCAPTYPE_LINE);		// calculate capture line size
	LineStep = TotalSize;
	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits
	//LineStep = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits

	IoReg_Write32(MDOMAIN_DISP_DDR_MainLineStep_reg, (LineStep + LineStep%2) << 3);//bit 3 need to be 0. rbus rule

	ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
	ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_MAIN) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
	MemSetDisplayAddr(SLR_MAIN_DISPLAY, ucStartAddress, ucStartAddress2);	// Main disaply not need set second block start address

	if (bDouble)
	{
		if (buffernum == 0)
		{
			ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_MAIN) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
			ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
		}
		else
		{
			ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
			ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
		}
	}
	else
	{
		if (buffernum == 0)
		{
			ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN) +(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
        	//ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + (info->MEM_ACT_HSTA) + (info->MEM_ACT_VSTA *TotalSize);
		}
		else
		{
		ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
        	//ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + (info->MEM_ACT_HSTA) + (info->MEM_ACT_VSTA *TotalSize);
		}
	}

	//rtd_pr_scaler_memory_debug("disp buffernum=%x, ucStartAddress=%x ucStartAddress2=%x\n",buffernum,ucStartAddress,ucStartAddress2);

	MemSetDisplayAddr(SLR_MAIN_DISPLAY, ucStartAddress, ucStartAddress2);	// Main disaply not need set second block start address

	if(bDouble) {	// use double buffer
		TotalSize = LineStep * Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_MEM_ACT_LEN);
		IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg, (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule
	}
}

void memory_set_input_format2(unsigned char display, unsigned char bDouble)
{
	unsigned int /* temp = 0, */ temp1 = 0;
	mdomain_disp_display_byte_channel_swap_RBUS display_byte_channel_swap_reg;
	if ( bDouble )
	{
		if (Get_PANEL_VFLIP_ENABLE())
		{
		if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_VFLIP3))
			temp1 =  _BIT7 | (display ? _BIT1 : 0) | _BIT6  ;
		else
				temp1 =  (display ? _BIT1 : 0) | _BIT6;
		}
		else
			temp1 =  (display ? _BIT1 : 0) | _BIT6;
	}
	else
		temp1 =  (display ? _BIT1 : 0) ;

	//add below code for GPU access YUV format ++
	display_byte_channel_swap_reg.regValue = IoReg_Read32(MDOMAIN_DISP_Display_byte_channel_swap_reg);
	if(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)){
		//Crixus@20140722 - Mac3 no sub path
		//display_byte_channel_swap_reg.sub_2byte_swap = 1;
		//display_byte_channel_swap_reg.sub_4byte_swap = 1;
	}else{
		display_byte_channel_swap_reg.main_2byte_swap = 1;
		display_byte_channel_swap_reg.main_4byte_swap = 1;
	}
	IoReg_Write32(MDOMAIN_DISP_Display_byte_channel_swap_reg, display_byte_channel_swap_reg.regValue);
	//frank@03132013 add below code for GPU access YUV format --

#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_Write32(display? MDOMAIN_DISP_DDR_SubCtrl_reg: MDOMAIN_DISP_DDR_MainCtrl_reg,  temp1);
	IoReg_Write32(display? MDOMAIN_DISP_Disp_sub_enable_reg: MDOMAIN_DISP_Disp_main_enable_reg,  _BIT0);
#else
	IoReg_Write32(/*display? MDOMAIN_DISP_DDR_SubCtrl_reg : */MDOMAIN_DISP_DDR_MainCtrl_reg,  temp1);
	IoReg_Write32(/*display? MDOMAIN_DISP_DDR_SubCtrl_reg : */MDOMAIN_DISP_Disp_main_enable_reg,  _BIT0);
#endif
	drvif_memory_set_dbuffer_write();
}


void zoom_set_input_format(unsigned char display, unsigned char bDouble)
{
	unsigned int/* temp = 0,*/ temp1 = 0;
	mdomain_disp_display_byte_channel_swap_RBUS display_byte_channel_swap_reg;
	if ( bDouble )
	{
		if (Get_PANEL_VFLIP_ENABLE())
		{
		if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_VFLIP3))
			temp1 =  _BIT7 | (display ? _BIT1 : 0) | _BIT6   ;
		else
				temp1 =  (display ? _BIT1 : 0) | _BIT6;
		}
		else
			temp1 =  (display ? _BIT1 : 0) | _BIT6;
	}
	else
		temp1 =  (display ? _BIT1 : 0) ;
	temp1 |= display ? 0 : (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_10BIT) ? _BIT16 : 0);
	if((display == SLR_MAIN_DISPLAY) && (dvrif_memory_compression_get_enable(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) == TRUE)){
		temp1 |= _BIT17;
	}else{
		temp1 |= Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display, SLR_DISP_422CAP) ? 0 : _BIT17;
	}
	temp1 |=  _BIT19;

	// add below code for GPU access YUV format ++
	display_byte_channel_swap_reg.regValue = IoReg_Read32(MDOMAIN_DISP_Display_byte_channel_swap_reg);
	if(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)){
		//Crixus@20140722 - Mac3 no sub path
		//display_byte_channel_swap_reg.sub_2byte_swap = 1;
		//display_byte_channel_swap_reg.sub_4byte_swap = 1;
	}else{
		display_byte_channel_swap_reg.main_2byte_swap = 1;
		display_byte_channel_swap_reg.main_4byte_swap = 1;
	}
	IoReg_Write32(MDOMAIN_DISP_Display_byte_channel_swap_reg, display_byte_channel_swap_reg.regValue);
	//frank@03132013 add below code for GPU access YUV format --

#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_Write32(display? MDOMAIN_DISP_DDR_SubCtrl_reg: MDOMAIN_DISP_DDR_MainCtrl_reg,  temp1);
	IoReg_Write32(display? MDOMAIN_DISP_Disp_sub_enable_reg: MDOMAIN_DISP_Disp_main_enable_reg,  _BIT0);
#else
	IoReg_Write32(/*display? MDOMAIN_DISP_DDR_SubCtrl_reg : */MDOMAIN_DISP_DDR_MainCtrl_reg,  temp1);
	IoReg_Write32(/*display? MDOMAIN_DISP_DDR_SubCtrl_reg : */MDOMAIN_DISP_Disp_main_enable_reg,  _BIT0);
#endif
	// write data & set double buffer write enable
	if(display == SLR_MAIN_DISPLAY)
	{
//		IoReg_Write32(MDOMAIN_DDR_MAINSTATUS_VADDR , temp);
		if(scaler_vsc_get_vr360_block_mdomain_doublebuffer() == FALSE)
			IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg, IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg) | _BIT0);
	}
}
#endif
extern unsigned int mdomain_input_fast_flag;

//change to use register structure @Crixus 20170518
void memory_set_input_format(unsigned char display, unsigned char bDouble)
{
#ifndef UT_flag
	//unsigned int /*temp = 0,*/ temp1 = 0;
	mdomain_disp_ddr_mainctrl_RBUS mdomain_disp_ddr_mainctrl_reg;
	mdomain_disp_ddr_subctrl_RBUS mdomain_disp_ddr_subctrl_reg;
	mdomain_disp_display_byte_channel_swap_RBUS display_byte_channel_swap_reg;
	mdomain_disp_display_sub_byte_channel_swap_RBUS mdomain_disp_display_sub_byte_channel_swap_reg;
	//modify to use structure @Crixus 20170517
	mdomain_disp_ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
	mdomain_disp_ddr_subctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubCtrl_reg);

	mdomain_input_fast_flag = 0;

	//Data FRC mode
	mdomain_disp_ddr_mainctrl_reg.main_source_sel = 0;

	//buffer mode selection
	if(Get_vscFilmMode()==TRUE)
	{
		//temp1 =  (display ? _BIT1 : 0);
		if(display == SLR_MAIN_DISPLAY){
			mdomain_disp_ddr_mainctrl_reg.main_double_en = 0;
			mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en = 0;
			mdomain_disp_ddr_mainctrl_reg.main_bufnum = 0;
			mdomain_disp_ddr_mainctrl_reg.main_cap2disp_distance = 0;
		}
		else{
			mdomain_disp_ddr_subctrl_reg.sub_double_en = 0;
			mdomain_disp_ddr_subctrl_reg.sub_v_flip_3buf_en = 0;
			mdomain_disp_ddr_subctrl_reg.sub_bufnum = 0;
			mdomain_disp_ddr_subctrl_reg.sub_cap2disp_distance = 0;
		}
	}
	else
	{
		if ( bDouble )
		{
			if (Get_PANEL_VFLIP_ENABLE())
			{
				if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_VFLIP3)){
					//temp1 =  _BIT7 | (display ? _BIT1 : 0) | _BIT6   ;
					if(display == SLR_MAIN_DISPLAY){
						mdomain_disp_ddr_mainctrl_reg.main_double_en = 1;
						mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en = 1;
						mdomain_disp_ddr_mainctrl_reg.main_bufnum = 2;
						mdomain_disp_ddr_mainctrl_reg.main_cap2disp_distance = 1;
					}
					else{
						mdomain_disp_ddr_subctrl_reg.sub_double_en = 1;
						mdomain_disp_ddr_subctrl_reg.sub_v_flip_3buf_en = 1;
						mdomain_disp_ddr_subctrl_reg.sub_bufnum = 2;
						mdomain_disp_ddr_subctrl_reg.sub_cap2disp_distance = 1;
					}
				}
				else{
					//temp1 =  (display ? _BIT1 : 0) | _BIT6;
					if(display == SLR_MAIN_DISPLAY){
						mdomain_disp_ddr_mainctrl_reg.main_double_en = 1;
						mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en = 0;
						mdomain_disp_ddr_mainctrl_reg.main_bufnum = 0;
						mdomain_disp_ddr_mainctrl_reg.main_cap2disp_distance = 0;
					}
					else{
						mdomain_disp_ddr_subctrl_reg.sub_double_en = 1;
						mdomain_disp_ddr_subctrl_reg.sub_v_flip_3buf_en = 0;
						mdomain_disp_ddr_subctrl_reg.sub_bufnum = 0;
						mdomain_disp_ddr_subctrl_reg.sub_cap2disp_distance = 0;
					}
				}
			}
			else{
				//temp1 =  (display ? _BIT1 : 0) | _BIT6 ;
				if(display == SLR_MAIN_DISPLAY){
					mdomain_disp_ddr_mainctrl_reg.main_double_en = 1;
					mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en = 0;
					mdomain_disp_ddr_mainctrl_reg.main_bufnum = 0;
					mdomain_disp_ddr_mainctrl_reg.main_cap2disp_distance = 0;
				}
				else{
					mdomain_disp_ddr_subctrl_reg.sub_double_en = 1;
					mdomain_disp_ddr_subctrl_reg.sub_v_flip_3buf_en = 0;
					mdomain_disp_ddr_subctrl_reg.sub_bufnum = 0;
					mdomain_disp_ddr_subctrl_reg.sub_cap2disp_distance = 0;
				}
			}
		}
		else
		{
			if (Get_PANEL_VFLIP_ENABLE()&&(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_VFLIP3)))
			{
				//temp1 =  _BIT7 | (display ? _BIT1 : 0) ;
				if(display == SLR_MAIN_DISPLAY){
					mdomain_disp_ddr_mainctrl_reg.main_double_en = 1;
					mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en = 1;
					mdomain_disp_ddr_mainctrl_reg.main_bufnum = 2;
					mdomain_disp_ddr_mainctrl_reg.main_cap2disp_distance = 1;
				}
				else{
					mdomain_disp_ddr_subctrl_reg.sub_double_en = 1;
					mdomain_disp_ddr_subctrl_reg.sub_v_flip_3buf_en = 1;
					mdomain_disp_ddr_subctrl_reg.sub_bufnum = 2;
					mdomain_disp_ddr_subctrl_reg.sub_cap2disp_distance = 1;
				}
			}
			else{
				//temp1 =  (display ? _BIT1 : 0);
				if(display == SLR_MAIN_DISPLAY){
					mdomain_disp_ddr_mainctrl_reg.main_double_en = 0;
					mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en = 0;
					mdomain_disp_ddr_mainctrl_reg.main_bufnum = 0;
					mdomain_disp_ddr_mainctrl_reg.main_cap2disp_distance = 0;
				}
				else{
					mdomain_disp_ddr_subctrl_reg.sub_double_en = 0;
					mdomain_disp_ddr_subctrl_reg.sub_v_flip_3buf_en = 0;
					mdomain_disp_ddr_subctrl_reg.sub_bufnum = 0;
					mdomain_disp_ddr_subctrl_reg.sub_cap2disp_distance = 0;
				}
			}
		}
		//M-3buf condition:
		// 1. [2D] M-capture frame rate > M-display frame rate
		// 2. [3D][SG] FRC in M-domain, M-display need repeat frame 2 times
		// 3. [3D][SG] SG 3D display free run need triple buffer to avoid frame tear
		// 4. [3D][PR] PR to SG 3D display timing FRC need enable triple buffer to avoid frame tear issue
		// 5. [3D][PR] frame packing video in PR 3D panel (TO BE FIXED)
		if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 650)
				|| drvif_scaler3d_decide_3d_SG_data_FRC()
				|| ((drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() == TRUE) && drvif_scaelr3d_decide_is_3D_display_mode())
				|| (Get_PANEL_3D_PR_TO_SG_OUTPUT_FRC_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode())
				|| (modestate_get_pr3d_in_fp_3d_mode() && ((Scaler_InputSrcGetMainChType() == _SRC_VO)|| (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 310))))
		{
			// Input source frame rate faster than display (main_frc_style = 1)
			// M-display IVS refer to front of VGIP signal (for HDMI FP 3D, IVS refer to the source v-sync)
			if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 650) && (Scaler_Get3DMode((SCALER_DISP_CHANNEL)display) == SLR_3DMODE_2D) && (display == SLR_MAIN_DISPLAY))
			{//Will add condition. only main can set for WOSQRTK-2978
				//temp1 |= _BIT3;
				if(get_hdmi_4k_hfr_mode() == HDMI_NON_4K120)
					mdomain_input_fast_flag = 1;//only not two pixel to set fast mode
			}

			if(display == SLR_MAIN_DISPLAY){//frank@1007 solve VGA source M domain hang problem
				//set Mdomain display triple buffer
				//temp1 |=_BIT7;
				//triple_buf_flag = 1;
				mdomain_disp_ddr_mainctrl_reg.main_double_en = 1;
				mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en = 1;
				mdomain_disp_ddr_mainctrl_reg.main_bufnum = 2;
				mdomain_disp_ddr_mainctrl_reg.main_cap2disp_distance = 1;
			}

			// [3D][PR]  PR output merge triple buffer enable
			if(Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()){
				if((Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_TOP_AND_BOTTOM)||(Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING)){
					//temp1 |=_BIT14;
					mdomain_disp_ddr_mainctrl_reg.main_pr_out_merge_triple = 1;
				}
			}
		}

		if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()){
			mdomain_disp_ddr_mainctrl_reg.main_pr_our_merge_en = (mdomain_disp_ddr_mainctrl_reg.regValue&MDOMAIN_DISP_DDR_MainCtrl_main_pr_our_merge_en_mask)>>MDOMAIN_DISP_DDR_MainCtrl_main_pr_our_merge_en_shift;
			mdomain_disp_ddr_mainctrl_reg.main_merge_en0 =  (mdomain_disp_ddr_mainctrl_reg.regValue&MDOMAIN_DISP_DDR_MainCtrl_main_merge_en0_mask)>>MDOMAIN_DISP_DDR_MainCtrl_main_merge_en0_shift;
		}
	}

	//temp1 =  (display ? _BIT1 : 0);
	if(display == SLR_MAIN_DISPLAY){
		mdomain_disp_ddr_mainctrl_reg.main_source_sel = 0;
		mdomain_disp_ddr_mainctrl_reg.main_frc_style = 0;
	}
	else{
		mdomain_disp_ddr_subctrl_reg.sub_source_sel = 1;
		mdomain_disp_ddr_subctrl_reg.sub_frc_style = 0;
	}

	//check bit number
	if(display == SLR_MAIN_DISPLAY){
		if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_10BIT))
			mdomain_disp_ddr_mainctrl_reg.main_bit_num = 1;
		else
			mdomain_disp_ddr_mainctrl_reg.main_bit_num = 0;
	}
	else{
		//merlin4 sub display support 10 bits for i2r m-cap to s-disp @Crixus 20180327
		if(Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_10BIT))
			mdomain_disp_ddr_subctrl_reg.sub_bit_num = 1;
		else
			mdomain_disp_ddr_subctrl_reg.sub_bit_num = 0;
	}

	//check format
	if((display == SLR_MAIN_DISPLAY) && (dvrif_memory_compression_get_enable(display) == TRUE)){
		mdomain_disp_ddr_mainctrl_reg.main_in_format = 1;
	}else{
		if(display == SLR_MAIN_DISPLAY){
			mdomain_disp_ddr_mainctrl_reg.main_in_format = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) ? 0 : 1;
		}
		else{
			mdomain_disp_ddr_subctrl_reg.sub_in_format = Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP) ? 0 : 1;
		}
	}

	//check color space
	if(display == SLR_MAIN_DISPLAY){
		mdomain_disp_ddr_mainctrl_reg.maincolorspace = 1;
		mdomain_disp_ddr_mainctrl_reg.main_frame_seq_out_en = (drvif_scaler3d_decide_3d_SG_data_FRC()|| (drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() && drvif_scaelr3d_decide_is_3D_display_mode())? 1: 0);
	}
	else{
		mdomain_disp_ddr_subctrl_reg.subcolorspace = 1;
	}

#ifdef ENABLE_3_BUFFER_DELAY_MODE
	//force to set Mdomain main display triple buffer
	if(display == SLR_MAIN_DISPLAY){
		mdomain_disp_ddr_mainctrl_reg.main_double_en = 1;
		mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en = 1;
		mdomain_disp_ddr_mainctrl_reg.main_bufnum = 2;
		mdomain_disp_ddr_mainctrl_reg.main_cap2disp_distance = 1;
	}
	else{
		mdomain_disp_ddr_subctrl_reg.sub_double_en = 1;
		mdomain_disp_ddr_subctrl_reg.sub_v_flip_3buf_en = 1;
		mdomain_disp_ddr_subctrl_reg.sub_bufnum = 2;
		mdomain_disp_ddr_subctrl_reg.sub_cap2disp_distance = 1;
	}
#endif

	//frank@03132013 add below code for GPU access YUV format ++
	display_byte_channel_swap_reg.regValue = IoReg_Read32(MDOMAIN_DISP_Display_byte_channel_swap_reg);
	mdomain_disp_display_sub_byte_channel_swap_reg.regValue = IoReg_Read32(MDOMAIN_DISP_Display_sub_byte_channel_swap_reg);//update disp byte swap reg @Crixus 20170515
	if(display){
		if(Scaler_DispGetStatus(display, SLR_DISP_422CAP))
		{
			mdomain_disp_display_sub_byte_channel_swap_reg.sub_1byte_swap = 0;
			mdomain_disp_display_sub_byte_channel_swap_reg.sub_2byte_swap = 1;
			mdomain_disp_display_sub_byte_channel_swap_reg.sub_4byte_swap = 1;
			mdomain_disp_display_sub_byte_channel_swap_reg.sub_8byte_swap = 0;
		}
		else
		{
			mdomain_disp_display_sub_byte_channel_swap_reg.sub_1byte_swap = 1;
			mdomain_disp_display_sub_byte_channel_swap_reg.sub_2byte_swap = 1;
			mdomain_disp_display_sub_byte_channel_swap_reg.sub_4byte_swap = 1;
			mdomain_disp_display_sub_byte_channel_swap_reg.sub_8byte_swap = 0;
		}
	}else{
		if(Scaler_DispGetStatus(display, SLR_DISP_10BIT))
				{
					display_byte_channel_swap_reg.main_1byte_swap = 0;
					display_byte_channel_swap_reg.main_2byte_swap = 0;
					display_byte_channel_swap_reg.main_4byte_swap = 0;
					display_byte_channel_swap_reg.main_8byte_swap = 0;

				}
				else
				{
					if(Scaler_DispGetStatus(display, SLR_DISP_422CAP))
					{
						display_byte_channel_swap_reg.main_1byte_swap = 0;
						display_byte_channel_swap_reg.main_2byte_swap = 1;
						display_byte_channel_swap_reg.main_4byte_swap = 1;
						display_byte_channel_swap_reg.main_8byte_swap = 0;
					}
					else
					{
						display_byte_channel_swap_reg.main_1byte_swap = 1;
						display_byte_channel_swap_reg.main_2byte_swap = 1;
						display_byte_channel_swap_reg.main_4byte_swap = 1;
						display_byte_channel_swap_reg.main_8byte_swap = 0;
					}
				}

	}
	IoReg_Write32(MDOMAIN_DISP_Display_byte_channel_swap_reg, display_byte_channel_swap_reg.regValue);
	IoReg_Write32(MDOMAIN_DISP_Display_sub_byte_channel_swap_reg, mdomain_disp_display_sub_byte_channel_swap_reg.regValue);
	//frank@03132013 add below code for GPU access YUV format --
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	if(display == SLR_MAIN_DISPLAY)
		IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg, mdomain_disp_ddr_mainctrl_reg.regValue);
	else
		IoReg_Write32(MDOMAIN_DISP_DDR_SubCtrl_reg, mdomain_disp_ddr_subctrl_reg.regValue);

	IoReg_Write32(display? MDOMAIN_DISP_Disp_sub_enable_reg: MDOMAIN_DISP_Disp_main_enable_reg,  _BIT0);
#else
	IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg, mdomain_disp_ddr_mainctrl_reg.regValue);
	IoReg_Write32(/*display? MDOMAIN_DISP_DDR_SubCtrl_reg : */MDOMAIN_DISP_Disp_main_enable_reg,  _BIT0);
#endif
	// write data & set double buffer write enable
	if(display == SLR_MAIN_DISPLAY)
	{
		// write data & enable double buffer
		// due to mantis bug:10311, we have to write 3 times to enable double buffer register write
		//IoReg_Write32(MDOMAIN_DDR_MAINSTATUS_VADDR , temp);
		if(drvif_memory_set_dbuffer_write() == 0)
			rtd_pr_scaler_memory_debug("\n*** ERR: M-disp write fail@%s.L%d, %x ***\n", __FUNCTION__, __LINE__, IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg));
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else {
		//IoReg_Write32(MDOMAIN_DDR_SUBSTATUS_VADDR, temp);
		if(drvif_memory_set_dbuffer_write() == 0)
			rtd_pr_scaler_memory_debug("\n*** ERR: M-disp write fail@%s.L%d, %x ***\n", __FUNCTION__, __LINE__, IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg));
	}
#endif


#endif	// end of #ifndef UT_flag
}
 #ifndef BUILD_QUICK_SHOW

static unsigned int memory_get_capture_max_size(unsigned char display, eMemCapAccessType AccessType)
{
	unsigned int TotalSize;
	unsigned char ddrbit;
	unsigned char dispInterlace = Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display, SLR_DISP_INTERLACE);

	ddrbit =  (Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_CHANNEL) ? 0 : ((Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_10BIT)) << 1 ))  | ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_422CAP));

	TotalSize = memory_get_line_size((Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID_PRE) > _DISP_WID) ? _DISP_WID :  Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPH_ACT_WID_PRE), (ddrbit & _BIT0), ((ddrbit & _BIT1) >> 1));

	// it must be times of 4
	TotalSize = drvif_memory_get_data_align(TotalSize, 0x04);

	// multiple total_lines if frame-access
	if (AccessType == MEMCAPTYPE_FRAME) {
			if(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPV_ACT_LEN_PRE) > _DISP_LEN)
				TotalSize *= _DISP_LEN;
			else
				TotalSize *= Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_IPV_ACT_LEN_PRE);

			if(dispInterlace)
				TotalSize = TotalSize * 2;
	}

	return TotalSize;
}

#define REDUCE_DI_MEMORY_LIMIT_THRESHOLD 		720
/*============================================================================*/
/**
 * MemSetCapture
 * Set memory capture registers. It can be #1 or #2, based on current info structure
 *
 * @param <info> { display-info struecture }
 * @return { none }
 *
 */
 unsigned long mdomain_2k_memory_addr = 0;
unsigned long Get_Val_domain_2k_memory_addr_impl(void)
{
	return mdomain_2k_memory_addr;
}

unsigned long (*Get_Val_domain_2k_memory_addr)(void) = Get_Val_domain_2k_memory_addr_impl;

void Set_Val_domain_2k_memory_addr(unsigned long value)
{
	mdomain_2k_memory_addr=value;
}
//extern unsigned int memory_get_capture_line_size_compression(unsigned char compression_bits);
//change to use register structure @Crixus 20170518

#endif


unsigned int g_ulRealBufSize[MAX_DISP_CHANNEL_NUM];

void drvif_memory_SetRealBufSize(unsigned char display, unsigned int a_ulSize)
{
	g_ulRealBufSize[display] = a_ulSize;
}

unsigned int drvif_memory_GetRealBufSize(unsigned char display)
{
	return g_ulRealBufSize[display];
}


void memory_set_capture(unsigned char bDouble, eMemCapAccessType Cap_type, unsigned char display)
{
	unsigned int  TotalSize = 0;
	unsigned int  TotalSize1 = 0;
//	unsigned int  TotalSizeFrame;
#ifdef CONFIG_DUAL_CHANNEL
#ifdef ENABLE_3_BUFFER_DELAY_MODE
	unsigned int  nStartAddr=0;
#endif
#endif
	unsigned int  Quotient;
	unsigned char  Remainder;
	unsigned int blockstep;//need to be even
	unsigned int ulLineStep = 0;
	unsigned int temp_addr = 0;
#ifdef CONFIG_MPIXEL_SHIFT
	unsigned int CapLineSize = 0;
#endif
	// [2D3D] multiple of allocated buffer size in 2D3D conversion mode
	unsigned int multiOf2D3DBuf = 1;
//	unsigned int VO_MAX_FRC_SIZE=MDOMAIN_MAIN_BUFFER_SIZE; //using fixed memory size @Crixus 20160810
#ifndef CONFIG_ALL_SOURCE_DATA_FS
	unsigned int buffer_size = 0, buffer_size_page_align = 0;
#endif

	#ifdef CONFIG_ARM64
	unsigned long align_addr = 0, query_addr = 0, vir_addr = 0, align_vir_addr = 0;
	#else
	unsigned int align_addr = 0, query_addr = 0, vir_addr = 0, align_vir_addr = 0;
	#endif
	unsigned int double_buf_flag = 0, triple_buf_flag = 0;
	//unsigned int cap_temp = 0;
//	unsigned char lineRepeatEnable = FALSE;
	mdomain_cap_capture1_byte_swap_RBUS capture1_byte_swap_reg;
	mdomain_cap_capture2_byte_swap_RBUS capture2_byte_swap_reg;
	mdomain_cap_main_sdnr_cxxto64_RBUS mdomain_cap_main_sdnr_cxxto64_reg;
	
	mdomain_cap_cap_reg_doublbuffer_RBUS cap_reg_doublbuffer_reg;
	mdomain_cap_ddr_in1ctrl_RBUS mdomain_cap_ddr_in1ctrl_reg;
#ifdef CONFIG_DUAL_CHANNEL
	mdomain_cap_ddr_in2ctrl_RBUS mdomain_cap_ddr_in2ctrl_reg;
    mdomain_cap_sub_sdnr_cxxto64_RBUS mdomain_cap_sub_sdnr_cxxto64_reg;
#endif
	VSC_INPUT_TYPE_T src;

	//unsigned int TotalSize2 = 0;
	unsigned int burstLen = (display ?_BURSTLENGTH2: _BURSTLENGTH);
	//Update the fifo depth for each IC @Crixus 20170418
#ifdef MERLIN_FIFO_DEPTH
	//frank@0325 double the M domain burst length to avoid 444 10 bits peak BW not enough
	if((display == SLR_MAIN_DISPLAY) && ( !Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP))){
		burstLen=0xf0; //align 96-bytes
	}
#endif

	dvrif_memory_set_cap_mode(display, Cap_type);


#if 0//frank@0903 reduce scaler time,move below code to
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_ClearBits(display ? MDOMAIN_CAP_DDR_In2Ctrl_reg: MDOMAIN_CAP_DDR_In1Ctrl_reg, MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask);	// disable Capture
	IoReg_ClearBits(display ? MDOMAIN_CAP_Cap_In2_enable_reg : MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);	// disable Capture
#else
	IoReg_ClearBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : */MDOMAIN_CAP_DDR_In1Ctrl_reg, MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask); // disable Capture
	IoReg_ClearBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : */MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0); // disable Capture
#endif
	//frank@0503 add below delay to confirm M capture have disable
	msleep(20);
#endif

	if(display == SLR_MAIN_DISPLAY){
		//disable M-cap double buffer - main @Crixus 20160126
		if(scaler_vsc_get_vr360_block_mdomain_doublebuffer() == FALSE){
			cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
			cap_reg_doublbuffer_reg.cap1_db_en = 0;
			rtd_outl(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);
		}
	}
#ifdef CONFIG_DUAL_CHANNEL
	else{
		//disable M-cap double buffer - sub @Crixus 20160126
		cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
		cap_reg_doublbuffer_reg.cap2_db_en = 0;
		rtd_outl(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);
	}
#endif
	//field polarity error check
	memory_check_capture_polarity_error(display);
#if 0
	//20130905qiang_zhou:added for video delay frames for bluetooth+
	if(TRUE==get_delayframe_fuc())
	{
		delay_framebuffer_free();
		bDouble = FALSE;//go single buffer, when vgip end isr switch capture buffer(more than three)
	}
	//20130905qiang_zhou:added for video delay frames for bluetooth-
#endif

#ifdef CONFIG_DUAL_CHANNEL  /*SDNR output bit and sort format, new add in magellan2 @Crixus 20150330*/
	if(display == SLR_MAIN_DISPLAY) { /*main channel*/
		mdomain_cap_main_sdnr_cxxto64_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Main_SDNR_cxxto64_reg);
#if 0//def CONFIG_DATA_FRAMESYNC_FORCE_422_CAP
		mdomain_cap_main_sdnr_cxxto64_reg.sort_fmt = !Scaler_DispGetStatus(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), SLR_DISP_422CAP);
#else
		mdomain_cap_main_sdnr_cxxto64_reg.sort_fmt = (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_FRAMESYNC) || (dvrif_memory_compression_get_enable(display) ||(!Scaler_DispGetStatus(display, SLR_DISP_422CAP))));
#endif
		mdomain_cap_main_sdnr_cxxto64_reg.out_bit = Scaler_DispGetStatus(display, SLR_DISP_10BIT);
		IoReg_Write32(MDOMAIN_CAP_Main_SDNR_cxxto64_reg, mdomain_cap_main_sdnr_cxxto64_reg.regValue);
	} else {
		mdomain_cap_sub_sdnr_cxxto64_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Sub_SDNR_cxxto64_reg);
		mdomain_cap_sub_sdnr_cxxto64_reg.sub_sort_fmt = (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_FRAMESYNC) || (!Scaler_DispGetStatus(display, SLR_DISP_422CAP)));
		mdomain_cap_sub_sdnr_cxxto64_reg.sub_out_bit = Scaler_DispGetStatus(display, SLR_DISP_10BIT);
		IoReg_Write32(MDOMAIN_CAP_Sub_SDNR_cxxto64_reg, mdomain_cap_sub_sdnr_cxxto64_reg.regValue);
	}
#else
	mdomain_cap_main_sdnr_cxxto64_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Main_SDNR_cxxto64_reg);
#if 0//def CONFIG_DATA_FRAMESYNC_FORCE_422_CAP
	mdomain_cap_main_sdnr_cxxto64_reg.sort_fmt = !Scaler_DispGetStatus(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), SLR_DISP_422CAP);
#else
	mdomain_cap_main_sdnr_cxxto64_reg.sort_fmt =  (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_FRAMESYNC) || (dvrif_memory_compression_get_enable(display) ||(!Scaler_DispGetStatus(display, SLR_DISP_422CAP))));
#endif
	mdomain_cap_main_sdnr_cxxto64_reg.out_bit = Scaler_DispGetStatus(display, SLR_DISP_10BIT);
	IoReg_Write32(MDOMAIN_CAP_Main_SDNR_cxxto64_reg, mdomain_cap_main_sdnr_cxxto64_reg.regValue);
#endif


	if(dvrif_memory_compression_get_enable(display) == TRUE)
	{
		if(dvrif_memory_get_compression_mode(display) == MEMCAPTYPE_LINE)
		{
			TotalSize = memory_get_capture_line_size_compression(dvrif_memory_get_compression_bits(display));//compression test
		}
		else{
			TotalSize = memory_get_capture_frame_size_compression(dvrif_memory_get_compression_bits(display));
		}
	}
	else
	{
		TotalSize = memory_get_capture_size(display, Cap_type);		// calculate the memory size of capture
	}

	//TotalSize++;
	if (0 == TotalSize)
	{
		rtd_pr_scaler_memory_err("Err TotalSize=%d, func=%s, line=%d\n", TotalSize, __FUNCTION__, __LINE__);
	}
	if(drvif_scaler3d_decide_3d_SG_data_FRC()) // [SG] SG 3D in data FRC mode -- sequence frame 3D L-R output control
	{
			TotalSize=TotalSize<<1;
	}

	if(Cap_type == MEMCAPTYPE_LINE){
		//rtd_pr_scaler_memory_debug("[2D3D] Rpt/Multi[%d/x%d], LineWidth/Size[%d/%d]\n", lineRepeatEnable, multiOf2D3DBuf, TotalSize, TotalSize*multiOf2D3DBuf);
		TotalSize *= multiOf2D3DBuf;
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
#ifdef CONFIG_I2RND_ENABLE
		//i2rnd sub-cap to VO, line step has to be 2-align @Crixus 20160728
		if((display == SLR_SUB_DISPLAY) && (vsc_i2rnd_sub_stage == I2RND_STAGE_DETECT_SOURCE)){
			total_line_size = drvif_memory_get_data_align(total_line_size, 2);	//sub=>vo, vo is align-2, so the line step should be align-2
			IoReg_Write32(MDOMAIN_CAP_DDR_In2LineStep_reg, (total_line_size + total_line_size%2) << 3);//bit 3 need to be 0. rbus rule
		}
		else{
			// Line step do 96 align
			ulLineStep = TotalSize;//drvif_memory_get_data_align(TotalSize, DMA_SPEEDUP_ALIGN_VALUE/8);
			IoReg_Write32(display ? MDOMAIN_CAP_DDR_In2LineStep_reg: MDOMAIN_CAP_DDR_In1LineStep_reg, (ulLineStep + ulLineStep%2) << 3);//bit 3 need to be 0. rbus rule
		}
#else
		// Line step do 96 align
		ulLineStep = TotalSize;//drvif_memory_get_data_align(TotalSize, DMA_SPEEDUP_ALIGN_VALUE/8);
		IoReg_Write32(display ? MDOMAIN_CAP_DDR_In2LineStep_reg: MDOMAIN_CAP_DDR_In1LineStep_reg, (ulLineStep + ulLineStep%2) << 3);//bit 3 need to be 0. rbus rule
#endif
		blockstep = (TotalSize*Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_CAP_LEN));
		blockstep = (blockstep % 2)? blockstep + 1: blockstep;
		IoReg_Write32(display ? MDOMAIN_CAP_DDR_In2BlkStep_reg: MDOMAIN_CAP_DDR_In1BlkStep_reg, blockstep << 3);//bit 3 need to be 0. rbus rule
#else
		// Line step do 96 align
		ulLineStep = TotalSize;//drvif_memory_get_data_align(TotalSize, DMA_SPEEDUP_ALIGN_VALUE/8);
		IoReg_Write32(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2LineStep_reg : */MDOMAIN_CAP_DDR_In1LineStep_reg , (ulLineStep + ulLineStep%2) << 3);//bit 3 need to be 0. rbus rule

		blockstep = (TotalSize*Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_CAP_LEN));
		blockstep = (blockstep % 2)? blockstep + 1: blockstep;
		IoReg_Write32(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2BlkStep_reg : */MDOMAIN_CAP_DDR_In1BlkStep_reg, blockstep << 3);//bit 3 need to be 0. rbus rule
#endif
	}else{
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		IoReg_Write32(display ? MDOMAIN_CAP_DDR_In2LineStep_reg: MDOMAIN_CAP_DDR_In1LineStep_reg, 0);
#else
		IoReg_Write32(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2LineStep_reg : */MDOMAIN_CAP_DDR_In1LineStep_reg, 0);
#endif
	}

	// [2D3D] line repeat mode FIFO length control
	TotalSize /= multiOf2D3DBuf;
//-	rtd_pr_scaler_memory_debug("Cap Act wid=%x, Cap Act len=%x\n", Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID), Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN));
//-	rtd_pr_scaler_memory_debug("\n Calculate FIFO Size !");
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF))
	{
		memory_division(TotalSize>>1, burstLen, &Quotient, &Remainder);	// Do a division
		Scaler_Disp3dSetInfo(display, SLR_DISP_3D_BLKSTEP,TotalSize>>1);
	}
	else
	{
		//rtd_pr_scaler_memory_emerg("0  [Memory]TotalSize=%x, WL=%x,Q=%x,Rm=%x\n", TotalSize, burstLen,Quotient,Remainder);
		memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division
	}


	//rtd_pr_scaler_memory_debug("[Memory]WL=%x,Q=%x,Rm=%x\n", burstLen,Quotient,Remainder);
	if(dvrif_memory_compression_get_enable(display) == FALSE){
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) != SLR_3D_2D_ONLY))
		Remainder = drvif_memory_get_data_align(Remainder, 2);	// multiple of 4
	else
		Remainder = drvif_memory_get_data_align(Remainder, 4);	// multiple of 4
	}

	if (Remainder == 0) {	// remainder is not allowed to be zero
		Remainder = burstLen;
		Quotient -= 1;
	}


	rtd_pr_scaler_memory_debug("channel=%x, cmp=%x, cmp mode=%x\n", display, dvrif_memory_compression_get_enable(display), dvrif_memory_get_compression_mode(display));
	rtd_pr_scaler_memory_debug("[crixus][%s]TotalSize = %d, Quotient = %d, Remainder = %d\n", __FUNCTION__,TotalSize, Quotient, Remainder);

	if((dvrif_memory_compression_get_enable(display) == TRUE) &&(dvrif_memory_get_compression_mode(display) == MEMCAPTYPE_FRAME)){
		//Frame mode and PQC: num is 0xffff, length = remain
		//Need to set maxmun because can not sure how many pqc data on frame mode. @Crixus 20170626
		memory_set_capture_fifo(display, 0xffff, burstLen, burstLen);	// set capture FIFO
	}
	else{

		rtd_pr_scaler_memory_debug("line mode\n");

		memory_set_capture_fifo(display, Quotient, Remainder, burstLen);	// set capture FIFO
	}
	TotalSize = SHL(Quotient * burstLen + Remainder, 3); //unit conversion. 64bits to 8bits

	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF))
	{
		TotalSize=TotalSize<<1;
	}

	// [2D3D] line repeat mode buffer size control
	TotalSize *= multiOf2D3DBuf;
	//Not to allocate the disp size @Crixus 20151230
	//TotalSize1 = memory_get_capture_max_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), Cap_type);

	if(drvif_scaler3d_decide_3d_SG_data_FRC()) // [SG] SG 3D in data FRC mode -- sequence frame 3D L-R output control
	{
		TotalSize1=TotalSize1<<1;
	}

	TotalSize1 = SHL(TotalSize1, 3); //unit conversion. 64bits to 8bits

	TotalSize1 *= multiOf2D3DBuf;

	if(Cap_type == MEMCAPTYPE_LINE){
		TotalSize1 *=Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_LEN_PRE);
		TotalSize *=Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_CAP_LEN);
		if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() &&
			(Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE)==SLR_3D_SIDE_BY_SIDE_HALF) &&
			Scaler_DispGetStatus(display, SLR_DISP_INTERLACE))
		{
			TotalSize=TotalSize<<1;
			TotalSize1=TotalSize1<<1;
		}
	}

	drvif_memory_SetRealBufSize(display, TotalSize);
	//rtd_pr_scaler_memory_debug("[3D]TotalSize_ori= %x,TotalSize1_ori= %x,INPUT_CAP_LEN=%x, SG_FRC=%d\n", TotalSize,TotalSize1
//		,Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_CAP_LEN), drvif_scaler3d_decide_3d_SG_data_FRC());
	#ifdef CONFIG_MPIXEL_SHIFT
	if(Cap_type == MEMCAPTYPE_FRAME)
	{
		CapLineSize = memory_get_capture_size(display, MEMCAPTYPE_LINE);
		CapLineSize = SHL(CapLineSize, 3); //unit conversion. 64bits to 8bits
		TotalSize2 = SHL(CapLineSize ,6);  //pixeel shift 32 line up and down
	}
	#endif

	if (display == SLR_MAIN_DISPLAY)	 {
#ifdef CONFIG_MDOMAIN_RESERVED_MEMORY

#ifdef CONFIG_I2RND_ENABLE
		if((Scaler_I2rnd_get_timing_enable() == _ENABLE) && ((vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC) || (vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1))){
			rtd_pr_scaler_memory_emerg("[I2RND][%s]Enter cap2 memory allocate\n", __FUNCTION__);
			if(dvrif_memory_compression_get_enable(display))==_ENABLE)
				buffer_size = MDOMAIN_MAIN_I2R_BUFFER_SIZE;
			else
				buffer_size = MDOMAIN_MAIN_I2R_BUFFER_SIZE;
		}
		else
#endif
		{
#ifdef CONFIG_ALL_SOURCE_DATA_FS
			Scaler_memory_setting_through_CMA();
#else
		src = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
		if(dvrif_memory_compression_get_enable(display)==_ENABLE) {
            buffer_size = calc_compress_memory_size(display, dvrif_memory_get_compression_bits(display));
	} // end of if(dvrif_memory_compression_get_enable(display)==_ENABLE)
	else
	{
		if(Get_rotate_function(SLR_MAIN_DISPLAY))
		{
			if((src == VSC_INPUTSRC_HDMI)||(src == VSC_INPUTSRC_JPEG))
				buffer_size = MDOMAIN_MAIN_BUFFER_SIZE_12BITS_HDMI;
			else
				buffer_size = MDOMAIN_MAIN_BUFFER_SIZE_12BITS;
		}
		else
			buffer_size = MDOMAIN_MAIN_BUFFER_SIZE_30BITS;
		}

		if(!is_2k_memory_buffer_source(SLR_MAIN_DISPLAY))
		{
                if(buffer_size > i3ddmaCtrl.cap_buffer[0].size)
                {
                    rtd_pr_scaler_memory_err("[%s %d]ERROR memory is not enough\n", __FUNCTION__, __LINE__);
                }
		}
#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
			buffer_size = MDOMAIN_MAIN_BUFFER_SIZE_30BITS;
			rtd_pr_scaler_memory_info("[%s:%d] force go to 30 bit buffer %d\n", __func__, __LINE__, buffer_size);
#endif
            #ifndef BUILD_QUICK_SHOW
			if(drv_memory_Get_multibuffer_flag()) {
				scaler_memory_free_multibuf();
				if(scaler_memory_alloc_multibuf() == 1)
				{
					rtd_pr_scaler_memory_emerg("[MultiBuf]M-domain Multi-buffer 4K alloc buf fail!!!\n");
					drv_memory_Set_multibuffer_flag(_DISABLE);
					BUG();
				}
			}

			//merlin4 memory do 96 bytes align @Crixus 20180321
			query_addr = get_query_start_address(QUERY_IDX_MDOMAIN_MAIN);
			if(get_platform_model() == PLATFORM_MODEL_2K) {
				if(!query_addr)
				{
					if(Get_Val_domain_2k_memory_addr())
						pli_free(Get_Val_domain_2k_memory_addr());
					temp_addr = pli_malloc(buffer_size*3, GFP_DCU1);
					Set_Val_domain_2k_memory_addr(temp_addr);
					if((Get_Val_domain_2k_memory_addr()) == INVALID_VAL)
					{
						rtd_pr_scaler_memory_emerg("mdomain 2k pli alloc memory fail!\n");
						return;
					}
					query_addr = (Get_Val_domain_2k_memory_addr());
				}
			}
            #else
                query_addr = get_query_start_address(QUERY_IDX_MDOMAIN_MAIN);
            #endif
			buffer_size = drvif_memory_get_data_align(buffer_size, DMA_SPEEDUP_ALIGN_VALUE);

			align_addr = dvr_memory_alignment((unsigned long)query_addr, buffer_size);

			#ifdef CONFIG_ARM64
			if (align_addr == (unsigned long)NULL)
			{
        			BUG();
			}
			#else
			if (align_addr == (unsigned int)NULL)
			{
        			BUG();
			}
			#endif
			else
			{
				align_addr = dvr_memory_alignment((unsigned long)query_addr, buffer_size);
			}

			//Just for reserved memory, update to use query driver @Crixus 20170817
#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
			MemTag[MEMIDX_MAIN].StartAddr = align_addr;
			MemTag[MEMIDX_MAIN_SEC].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN].StartAddr+buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
			MemTag[MEMIDX_MAIN_THIRD].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN].StartAddr+buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
#else
			if(!is_2k_memory_buffer_source(SLR_MAIN_DISPLAY))
			{//4k
				if(i3ddmaCtrl.cap_buffer[0].phyaddr)
				{
						MemTag[MEMIDX_MAIN_THIRD].StartAddr = drvif_memory_get_data_align(i3ddmaCtrl.cap_buffer[0].phyaddr, DMA_SPEEDUP_ALIGN_VALUE);
						MemTag[MEMIDX_MAIN_SEC].StartAddr = drvif_memory_get_data_align(i3ddmaCtrl.cap_buffer[0].phyaddr + buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
						MemTag[MEMIDX_MAIN].StartAddr = drvif_memory_get_data_align(i3ddmaCtrl.cap_buffer[0].phyaddr + buffer_size*2, DMA_SPEEDUP_ALIGN_VALUE);
				}
				else
				{
					rtd_pr_scaler_memory_err("##[ERR] 4k no memory cap##\r\n");
					return; 
				}
			}else
                    {
                          	MemTag[MEMIDX_MAIN_THIRD].StartAddr = align_addr;
				MemTag[MEMIDX_MAIN_SEC].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_THIRD].StartAddr+buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
				MemTag[MEMIDX_MAIN].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_SEC].StartAddr+buffer_size, DMA_SPEEDUP_ALIGN_VALUE);			
			}

#endif
			//16 bytes alignment
			MemTag[MEMIDX_MAIN_THIRD].StartAddr = MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0;
			MemTag[MEMIDX_MAIN_SEC].StartAddr = MemTag[MEMIDX_MAIN_SEC].StartAddr & 0x7ffffff0;
			MemTag[MEMIDX_MAIN].StartAddr = MemTag[MEMIDX_MAIN].StartAddr & 0x7ffffff0;

			//rtd_pr_scaler_memory_emerg("MemTag[MEMIDX_MAIN].StartAddr=%x, MemTag[MEMIDX_MAIN_SEC].StartAddr=%x, MemTag[MEMIDX_MAIN_THIRD].StartAddr=%x, buffer_size=%x\n", MemTag[MEMIDX_MAIN].StartAddr, MemTag[MEMIDX_MAIN_SEC].StartAddr, MemTag[MEMIDX_MAIN_THIRD].StartAddr, buffer_size);

			#if 1//def CONFIG_REALTEK_2K_MODEL_ENABLED
			//update multi-buffer start address @Crixus 20160913
			drv_memory_Set_multibuffer_size(buffer_size);
			if(drv_memory_Get_multibuffer_flag()){
				rtd_pr_scaler_memory_emerg("[crixus]M-domain Multi-buffer Enable!!!\n");
				drv_memory_capture_multibuffer_update_addr(buffer_size, MemTag[MEMIDX_MAIN].StartAddr);
			}
			#endif

			//free virtual memory
			//drvif_memory_free_fixed_virtual_memory();
			//set virtual memory size
			//rtd_pr_scaler_memory_emerg("buffer_size=%x\n", buffer_size);
			buffer_size_page_align = PAGE_ALIGN(buffer_size);
			//buffer_size = buffer_size_page_align;
			rtd_pr_scaler_memory_emerg("PAGE_ALIGN(buffer_size)=%x\n", PAGE_ALIGN(buffer_size));
			drvif_memory_set_fixed_virtual_memory_size(buffer_size_page_align);
			MemTag[MEMIDX_MAIN_THIRD].Allocate_StartAddr = MemTag[MEMIDX_MAIN_THIRD].StartAddr;
			MemTag[MEMIDX_MAIN_THIRD].Size = buffer_size;
			MemTag[MEMIDX_MAIN_THIRD].Status = ALLOCATED_FROM_PLI;

			MemTag[MEMIDX_MAIN_SEC].Allocate_StartAddr = MemTag[MEMIDX_MAIN_SEC].StartAddr;
			MemTag[MEMIDX_MAIN_SEC].Size = buffer_size;
			MemTag[MEMIDX_MAIN_SEC].Status = ALLOCATED_FROM_PLI;
			MemTag[MEMIDX_MAIN_SEC].Allocate_VirAddr = 0;//not use

			MemTag[MEMIDX_MAIN].Allocate_StartAddr = MemTag[MEMIDX_MAIN].StartAddr;
			MemTag[MEMIDX_MAIN].Size = buffer_size;
			MemTag[MEMIDX_MAIN].Status = ALLOCATED_FROM_PLI;
			MemTag[MEMIDX_MAIN].Allocate_VirAddr = 0;//not use

			//virtual memory allocation	and just use one buffer
			//drvif_memory_set_memtag_virtual_startaddr((unsigned long)dvr_remap_uncached_memory(drvif_memory_get_memtag_startaddr(MEMIDX_MAIN_THIRD), buffer_size_page_align, __builtin_return_address(0)));
			//virtual memory allocation	and just use one buffer
			rtd_pr_scaler_memory_debug("MemTag[MEMIDX_MAIN_THIRD].Allocate_VirAddr=%x\n", (unsigned int)MemTag[MEMIDX_MAIN_THIRD].Allocate_VirAddr);
#ifndef BUILD_QUICK_SHOW

			if (0 == MemTag[MEMIDX_MAIN_THIRD].Allocate_VirAddr)
			{
				//vir_addr = 0, align_vir_addr = 0
				vir_addr = (unsigned long)dvr_remap_uncached_memory(query_addr, buffer_size_page_align, __builtin_return_address(0));
				align_vir_addr = dvr_memory_alignment((unsigned long)vir_addr, buffer_size_page_align);
				drvif_memory_set_memtag_virtual_startaddr(align_vir_addr);
				//drvif_memory_set_memtag_virtual_startaddr((unsigned long)dvr_remap_uncached_memory(drvif_memory_get_memtag_startaddr(MEMIDX_MAIN_THIRD), buffer_size_page_align, __builtin_return_address(0)));

				#ifdef CONFIG_ARM64
					rtd_pr_scaler_memory_debug("vir_addr=%lx, align_vir_addr=%lx\n", vir_addr, align_vir_addr);
				#else
					rtd_pr_scaler_memory_debug("vir_addr=%x, align_vir_addr=%x\n", vir_addr, align_vir_addr);
				#endif
			}
#endif
			//M-domain start address
			IoReg_Write32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg, (MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0));
			IoReg_Write32(MDOMAIN_CAP_DDR_In1_2ndAddr_reg, (MemTag[MEMIDX_MAIN_SEC].StartAddr & 0x7ffffff0));
			IoReg_Write32(MDOMAIN_CAP_DDR_In1Addr_reg, (MemTag[MEMIDX_MAIN].StartAddr & 0x7ffffff0));
			//M-domain boundary
			//update multi-buffer boundary @Crixus 20160913
			#if 1//def CONFIG_REALTEK_2K_MODEL_ENABLED
			if(drv_memory_Get_multibuffer_flag()){
				drv_memory_capture_multibuffer_update_boundary(drv_memory_Get_multibuffer_number(), buffer_size);
			}
			else
			#endif
			{
				IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,(MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0));
				//IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(MemTag[MEMIDX_MAIN].StartAddr+buffer_size - CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
				IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(MemTag[MEMIDX_MAIN].StartAddr+buffer_size) & 0x7ffffff0);

				//Update m-cap limit boundary
				drv_memory_cap_limit_boundary_setting(SLR_MAIN_DISPLAY, buffer_size);
			}
#endif
		}
#else

#ifndef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,AllocStartAddr);
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,AllocEndAddr-CONFIG_MDOMAIN_BURST_SIZE);
#endif
		// set capture memory for main display
		drvif_memory_free_block(MEMIDX_MAIN);	// free block befor getting a new one
		drvif_memory_free_block(MEMIDX_MAIN_SEC);	// free block befor getting a new one

		if ((Get_PANEL_VFLIP_ENABLE() && Scaler_DispGetStatus(display, SLR_DISP_VFLIP3)))
		{
			if(!bDouble)
				drvif_memory_free_block(MEMIDX_MAIN_THIRD);
		}
		else
		{
				drvif_memory_free_block(MEMIDX_MAIN_THIRD);	// free block befor getting a new one
		}
#ifdef ENABLE_3_BUFFER_DELAY_MODE
	drvif_memory_free_block(MEMIDX_MAIN_THIRD);
#endif

		if((TotalSize1 > TotalSize)/* &&
			((Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_PIPMLSR) && (Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_PIPMLSR_4x3) &&
			(Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_MP1C12B) && (Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_MP1L7B))*/)
		{
			// MacArthur memory size limit (max frame size allocate only for SD video: data FRC path)
			/*if(((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > REDUCE_DI_MEMORY_LIMIT_THRESHOLD))
				//||(get_mainsubonpr_fuc()==TRUE)
				||(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode()))
			{
				TotalSize1 = 0;
			}
			else if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Scaler_Get3DMode(display) != SLR_3DMODE_2D))
				TotalSize1 = 0;
			else*/
			TotalSize1 = (TotalSize1 - TotalSize);
		} else {
			TotalSize1 = 0;
		}
		//rtd_pr_scaler_memory_debug("memory_get_capture_max_size:TotalSize1 == %d\n",TotalSize1);

		//if(TRUE!=get_delayframe_fuc())
		{
#ifdef CONFIG_ENABLE_3BUFFER_NEW_RULE
			if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ)>650) && (display ==_CHANNEL1))
			{
				nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_THIRD, TotalSize+TotalSize1+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//space unit is byte
				MemTag[MEMIDX_MAIN_THIRD].StartAddr = nStartAddr;
				IoReg_Write32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg, nStartAddr & 0x7ffffff0);
			}
			else if(_ENABLE == Scaler_Get_Enable_3Buffer_NewRuleCondition())
			{
				if(_TRUE == Scaler_DispGetStatus(display, SLR_DISP_VFLIP3))
				{
					nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_THIRD, TotalSize+TotalSize1+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//space unit is byte
					MemTag[MEMIDX_MAIN_THIRD].StartAddr = nStartAddr;
					IoReg_Write32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg, nStartAddr & 0x7ffffff0);
				}
			}
#else //#ifdef CONFIG_ENABLE_3BUFFER_NEW_RULE
		//M-3buf condition:
		// 1. [2D] M-capture frame rate > M-display frame rate
		// 2. [3D][SG] FRC in M-domain, M-display need repeat frame 2 times
		// 3. [3D][SG] SG 3D display free run need triple buffer to avoid frame tear
		// 4. [3D][PR] PR to SG 3D display timing FRC need enable triple buffer to avoid frame tear issue
		// 5. [3D][PR] frame packing video in PR 3D panel (TO BE FIXED)
		// 6. [VIP]film mode
		if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ)>650)|| drvif_scaler3d_decide_3d_SG_data_FRC()
				|| ((drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() == TRUE) && drvif_scaelr3d_decide_is_3D_display_mode())
				|| (Get_PANEL_3D_PR_TO_SG_OUTPUT_FRC_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode())
				|| (modestate_get_pr3d_in_fp_3d_mode() && ((Scaler_InputSrcGetType(display) == _SRC_VO)|| (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) >= 310)))
				||Get_vscFilmMode())
		{
			if(Scaler_InputSrcGetType(display) == _SRC_VO){
				nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_THIRD, VO_MAX_FRC_SIZE, MEMALIGN_ANY);//space unit is byte
			} else {
				nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_THIRD, TotalSize+TotalSize1+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//space unit is byte
			}
			MemTag[MEMIDX_MAIN_THIRD].StartAddr = nStartAddr;
			IoReg_Write32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg, nStartAddr & 0x7ffffff0);
		}
#endif //#ifdef CONFIG_ENABLE_3BUFFER_NEW_RULE
		}
#ifdef ENABLE_3_BUFFER_DELAY_MODE
		if(nStartAddr == 0){
			if(Scaler_InputSrcGetType(display) == _SRC_VO){
				nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_THIRD, VO_MAX_FRC_SIZE, MEMALIGN_ANY);//space unit is byte
			}else {
				nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_THIRD, TotalSize+TotalSize1+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//space unit is byte
			}
			MemTag[MEMIDX_MAIN_THIRD].StartAddr = nStartAddr;
			IoReg_Write32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg, nStartAddr & 0x7ffffff0);
		}
#endif

		//rtd_pr_scaler_memory_debug("###################main buffer TotalSize=%d;;TotalSize1=%d;;TotalSize2=%d\n",TotalSize,TotalSize1,TotalSize2);
		if(bDouble) {
			if(Scaler_InputSrcGetType(display) == _SRC_VO){
				nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_SEC, VO_MAX_FRC_SIZE, MEMALIGN_ANY);//space unit is byte
			}else {
				nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_SEC, TotalSize+TotalSize1 +TotalSize2+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//space unit is byte
			}
			#ifdef CONFIG_MPIXEL_SHIFT
			memory_black_buf_init(TotalSize,nStartAddr);
			if (TotalSize2 > 0)
				nStartAddr += CapLineSize * 32;
			#endif
			MemTag[MEMIDX_MAIN_SEC].StartAddr = nStartAddr;
			IoReg_Write32(MDOMAIN_CAP_DDR_In1_2ndAddr_reg, nStartAddr & 0x7ffffff0);
		}
#if 0
		//20130905qiang_zhou:added for video delay frames for bluetooth+
		if(TRUE==get_delayframe_fuc())
		{
			unsigned int buffernumber = get_delayframe_number();//get delay frame need buffer numbers
			unsigned int delayframeCachedAddr = delay_framebuffer_alloc(buffernumber*(TotalSize+TotalSize1+TotalSize2));
			set_delayframe_buffersize(TotalSize+TotalSize1+TotalSize2);
			nStartAddr = delayframeCachedAddr;
			IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,delayframeCachedAddr);
			IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,delayframeCachedAddr+buffernumber*(TotalSize+TotalSize1+TotalSize2));
		}
		else
#endif
		{
			if(Scaler_InputSrcGetType(display) == _SRC_VO){
				nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN, VO_MAX_FRC_SIZE, MEMALIGN_ANY);//space unit is byte
			}else {
				nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN, TotalSize+TotalSize1+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//because space unit is byte
			}
		}
		//20130905qiang_zhou:added for video delay frames for bluetooth-
		#ifdef CONFIG_MPIXEL_SHIFT
		memory_black_buf_init(TotalSize,nStartAddr);

		if (TotalSize2 > 0)
			nStartAddr += CapLineSize * 32;

		#endif

		MemTag[MEMIDX_MAIN].StartAddr = nStartAddr;

		IoReg_Write32(MDOMAIN_CAP_DDR_In1Addr_reg, nStartAddr & 0x7ffffff0);
		//rtd_pr_scaler_memory_debug("After %s--%d  IoReg_Read32(MDOMAIN_CAP_DDR_In1LineStep_reg)=%x\n",__FUNCTION__,__LINE__,IoReg_Read32(MDOMAIN_CAP_DDR_In1LineStep_reg));
#endif
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else {
//-		rtd_pr_scaler_memory_debug("\n Allocate Sub Disp Memory !\n");
		// set capture memory for sub display
		drvif_memory_free_block(MEMIDX_SUB);	// free block befor getting a new one
		drvif_memory_free_block(MEMIDX_SUB_SEC);	// free block befor getting a new one
		drvif_memory_free_block(MEMIDX_SUB_THIRD);	// free block befor getting a new one
		//sub use cap max size @Crixus 20151231
		if(get_hdmi_4k_hfr_mode() != HDMI_NON_4K120)
		{
			if(Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP) == FALSE)
				buffer_size = MDOMAIN_SUB_TWO_PIXEL_BUFFER_SIZE_PC_MODE;
			else
				buffer_size = MDOMAIN_SUB_TWO_PIXEL_BUFFER_SIZE;
		}
		else
			buffer_size = MDOMAIN_SUB_BUFFER_SIZE;
	#ifdef ENABLE_3_BUFFER_DELAY_MODE
			//I2rnd mode s-cap use 2-buffer
			nStartAddr = drvif_memory_alloc_block(MEMIDX_SUB_THIRD, buffer_size, MEMALIGN_ANY);//space unit is byte
			MemTag[MEMIDX_SUB_THIRD].StartAddr = nStartAddr;
			IoReg_Write32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg, nStartAddr & 0x7ffffff0);
		//}
	#endif

		if(bDouble)
		{
			IoReg_Write32(MDOMAIN_CAP_DDR_In2_2ndAddr_reg, drvif_memory_alloc_block(MEMIDX_SUB_SEC,buffer_size, MEMALIGN_ANY) & 0x7ffffff0);
		}

		IoReg_Write32(MDOMAIN_CAP_DDR_In2Addr_reg, drvif_memory_alloc_block(MEMIDX_SUB, buffer_size, MEMALIGN_ANY) & 0x7ffffff0);
		rtd_pr_scaler_memory_debug("###################sub buffer TotalSize=%d;;\n", buffer_size);
//-		rtd_pr_scaler_memory_debug("Get allocated sub display addr1=%x, addr2=%x, size=%x\n", StartAddr, StartAddr1, TotalSize);

		//Update s-cap limit boundary
		drv_memory_cap_limit_boundary_setting(SLR_SUB_DISPLAY, buffer_size);
	}
#endif
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	if(drvif_scaler3d_decide_3d_SG_data_FRC()){ // [SG] SG 3D in data FRC mode -- sequence frame 3D L-R output control
		IoReg_Mask32(display ? MDOMAIN_CAP_DDR_In2LineNum_reg : MDOMAIN_CAP_DDR_In1LineNum_reg, 0xfffee000, (Cap_type<<16)|AND(2*Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_CAP_LEN), 0x1FFF));
	} // [3D][PR] When PR SBS 3D mode IN1_SBS_VSD_EN=1, the line number = L+R
	else if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF)){
		IoReg_Mask32(display ? MDOMAIN_CAP_DDR_In2LineNum_reg : MDOMAIN_CAP_DDR_In1LineNum_reg, 0xfffee000, (Cap_type<<16)|AND(2*Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_CAP_LEN), 0x1FFF));
	}else
		IoReg_Mask32(display ? MDOMAIN_CAP_DDR_In2LineNum_reg : MDOMAIN_CAP_DDR_In1LineNum_reg, 0xfffee000, (Cap_type<<16)|AND(Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_CAP_LEN), 0x1FFF));
#else
	if(drvif_scaler3d_decide_3d_SG_data_FRC()){ // [SG] SG 3D in data FRC mode -- sequence frame 3D L-R output control
		IoReg_Mask32(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2LineNum_reg : */MDOMAIN_CAP_DDR_In1LineNum_reg, 0xfffee000, (Cap_type<<16)|AND(2*Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_CAP_LEN), 0x1FFF));
	} // [3D][PR] When PR SBS 3D mode IN1_SBS_VSD_EN=1, the line number = L+R
	else if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF)){
		IoReg_Mask32(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2LineNum_reg : */MDOMAIN_CAP_DDR_In1LineNum_reg, 0xfffee000, (Cap_type<<16)|AND(2*Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_CAP_LEN), 0x1FFF));
	}else
		IoReg_Mask32(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2LineNum_reg : */MDOMAIN_CAP_DDR_In1LineNum_reg, 0xfffee000, (Cap_type<<16)|AND(Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN), 0x1FFF));
#endif

#if 0
	if(Get_vscFilmMode()==TRUE)//go single hw buffer
	{
		double_buf_flag = 0;
		triple_buf_flag = 0;
	}
	else
#endif
	{
		if (Get_PANEL_VFLIP_ENABLE())
		{
			if (Scaler_DispGetStatus(display,SLR_DISP_VFLIP3)
				|| (drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() == TRUE)|| drvif_scaler3d_decide_3d_SG_data_FRC())
			{
				if(bDouble){
					double_buf_flag = 1;
					triple_buf_flag = 1;
				}
				else{
					double_buf_flag = 0;
					triple_buf_flag = 0;
				}

			}// [3D] PR to SG 3D panel, display timing FRC need enable triple buffer in VO source (vFlip in VO)
			else if(Get_PANEL_3D_PR_TO_SG_OUTPUT_FRC_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode()){
				if(bDouble){
					double_buf_flag = 1;
					triple_buf_flag = 1;
				}
				else{
					double_buf_flag = 0;
					triple_buf_flag = 0;
				}
			}
			else{
				if(bDouble){
					double_buf_flag = 1;
					triple_buf_flag = 0;
				}
				else{
					double_buf_flag = 0;
					triple_buf_flag = 0;
				}
			}
		}
		else
		{
			if(bDouble){
				double_buf_flag = 1;
				//triple_buf_flag = 0;
			}
			else{
				double_buf_flag = 0;
				//triple_buf_flag = 0;
			}
			//if(TRUE!=get_delayframe_fuc())
			{
#ifdef CONFIG_ENABLE_3BUFFER_NEW_RULE
				if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ)>650) && (display ==_CHANNEL1))
				{
					//IoReg_SetBits(MDOMAIN_CAP_DDR_In1Ctrl_reg, MDOMAIN_CAP_DDR_In1Ctrl_in1_v_flip_3buf_en_mask);
					triple_buf_flag = 1;
				}
				else if(_ENABLE == Scaler_Get_Enable_3Buffer_NewRuleCondition())
				{
					if(_TRUE == Scaler_DispGetStatus(display, SLR_DISP_VFLIP3))
					{
						triple_buf_flag = 1;
					}
				}
#else //#ifdef CONFIG_ENABLE_3BUFFER_NEW_RULE
			//M-3buf condition:
			// 1. [2D] M-capture frame rate > M-display frame rate
			// 2. [2Dcvt3D][SG] FRC in M-domain, M-display need repeat frame 2 times
			// 3. [3D][SG] SG 3D display free run need triple buffer to avoid frame tear
			// 4. [3D][PR] PR to SG 3D display timing FRC need enable triple buffer to avoid frame tear issue
			// 5. [3D][PR] frame packing video in PR 3D panel (TO BE FIXED)
			if((Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_V_FREQ) > 650)|| drvif_scaler3d_decide_3d_SG_data_FRC()
					|| (drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() && drvif_scaelr3d_decide_is_3D_display_mode())
					|| (Get_PANEL_3D_PR_TO_SG_OUTPUT_FRC_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode())
					|| (modestate_get_pr3d_in_fp_3d_mode() && ((Scaler_InputSrcGetType(display) == _SRC_VO)|| (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) >= 310))))
			{
				triple_buf_flag = 1;
			}
#endif //#ifdef CONFIG_ENABLE_3BUFFER_NEW_RULE
			}
		}
	}
#ifdef ENABLE_3_BUFFER_DELAY_MODE
	if (display == SLR_MAIN_DISPLAY){
		//IoReg_SetBits(MDOMAIN_CAP_DDR_In1Ctrl_reg, MDOMAIN_CAP_DDR_In1Ctrl_in1_v_flip_3buf_en_mask);
		triple_buf_flag = 1;
	}
#ifdef CONFIG_DUAL_CHANNEL
	else{
			triple_buf_flag = 1;
	}
#endif
#endif

#ifdef CONFIG_I2RND_ENABLE
	//I2rnd enable M-cap2 block @Crixus 20160721
	if((Scaler_I2rnd_get_timing_enable() == _ENABLE) && ((vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC)||(vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1))){
		triple_buf_flag = 1;
	}
#endif

	//Use register structure to set

#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
	triple_buf_flag = 0;
#endif
	if(triple_buf_flag != 0)
	{
		rtd_pr_scaler_memory_info("triple_buf_flag=%d\n", triple_buf_flag);
	}

	if (double_buf_flag != 0)
	{
		rtd_pr_scaler_memory_info("double_buf_flag=%d\n", double_buf_flag);
	}

#ifdef CONFIG_DUAL_CHANNEL
	if(display){
		mdomain_cap_ddr_in2ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In2Ctrl_reg);
	#ifdef ENABLE_3_BUFFER_DELAY_MODE
		//triple_buf_flag always equal to 1
		mdomain_cap_ddr_in2ctrl_reg.in2_en_vs_rst = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_double_enable = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_v_flip_3buf_en = 1;
		mdomain_cap_ddr_in2ctrl_reg.in2_bufnum = 2; // 3-buf mode
		mdomain_cap_ddr_in2ctrl_reg.in2_disp2cap_distance = 1;
	#else
		if(triple_buf_flag == 1){//triple buffer case
			mdomain_cap_ddr_in2ctrl_reg.in2_en_vs_rst = 1;
			mdomain_cap_ddr_in2ctrl_reg.in2_double_enable = 1;
			mdomain_cap_ddr_in2ctrl_reg.in2_v_flip_3buf_en = 1;
			mdomain_cap_ddr_in2ctrl_reg.in2_bufnum = 2; // 3-buf mode
			mdomain_cap_ddr_in2ctrl_reg.in2_disp2cap_distance = 1;
		}
		else if((double_buf_flag == 1) && (triple_buf_flag == 0)){//double buffer case
			mdomain_cap_ddr_in2ctrl_reg.in2_en_vs_rst = 1;
			mdomain_cap_ddr_in2ctrl_reg.in2_double_enable = 1;
			mdomain_cap_ddr_in2ctrl_reg.in2_v_flip_3buf_en = 0;
			mdomain_cap_ddr_in2ctrl_reg.in2_bufnum = 0;
			mdomain_cap_ddr_in2ctrl_reg.in2_disp2cap_distance = 0;
		}
		else{//single buffer
			mdomain_cap_ddr_in2ctrl_reg.in2_en_vs_rst = 1;
			mdomain_cap_ddr_in2ctrl_reg.in2_double_enable = 0;
			mdomain_cap_ddr_in2ctrl_reg.in2_v_flip_3buf_en = 0;
			mdomain_cap_ddr_in2ctrl_reg.in2_bufnum = 0;
			mdomain_cap_ddr_in2ctrl_reg.in2_disp2cap_distance = 0;
		}
	#endif
		mdomain_cap_ddr_in2ctrl_reg.in2_dout_align = 1;
		IoReg_Write32(MDOMAIN_CAP_DDR_In2Ctrl_reg, mdomain_cap_ddr_in2ctrl_reg.regValue);
	}
	else
#endif
	{
		mdomain_cap_ddr_in1ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Ctrl_reg);
	#ifdef ENABLE_3_BUFFER_DELAY_MODE
		//triple_buf_flag always equal to 1
		mdomain_cap_ddr_in1ctrl_reg.in1_en_vs_rst = 1;
		mdomain_cap_ddr_in1ctrl_reg.in1_double_enable = 1;
		mdomain_cap_ddr_in1ctrl_reg.in1_v_flip_3buf_en = 1;
		mdomain_cap_ddr_in1ctrl_reg.in1_bufnum = 2; // 3-buf mode
		mdomain_cap_ddr_in1ctrl_reg.in1_disp2cap_distance = 1;
	#else
		if(triple_buf_flag == 1){//triple buffer case
			mdomain_cap_ddr_in1ctrl_reg.in1_en_vs_rst = 1;
			mdomain_cap_ddr_in1ctrl_reg.in1_double_enable = 1;
			mdomain_cap_ddr_in1ctrl_reg.in1_v_flip_3buf_en = 1;
			mdomain_cap_ddr_in1ctrl_reg.in1_bufnum = 2; // 3-buf mode
			mdomain_cap_ddr_in1ctrl_reg.in1_disp2cap_distance = 1;
		}
		else if((double_buf_flag == 1) && (triple_buf_flag == 0)){//double buffer case
			mdomain_cap_ddr_in1ctrl_reg.in1_en_vs_rst = 1;
			mdomain_cap_ddr_in1ctrl_reg.in1_double_enable = 1;
			mdomain_cap_ddr_in1ctrl_reg.in1_v_flip_3buf_en = 0;
			mdomain_cap_ddr_in1ctrl_reg.in1_bufnum = 0;
			mdomain_cap_ddr_in1ctrl_reg.in1_disp2cap_distance = 0;
		}
		else{//single buffer
			mdomain_cap_ddr_in1ctrl_reg.in1_en_vs_rst = 1;
			mdomain_cap_ddr_in1ctrl_reg.in1_double_enable = 0;
			mdomain_cap_ddr_in1ctrl_reg.in1_v_flip_3buf_en = 0;
			mdomain_cap_ddr_in1ctrl_reg.in1_bufnum = 0;
			mdomain_cap_ddr_in1ctrl_reg.in1_disp2cap_distance = 0;
		}
	#endif
		mdomain_cap_ddr_in1ctrl_reg.in1_dout_align = 1;
		IoReg_Write32(MDOMAIN_CAP_DDR_In1Ctrl_reg, mdomain_cap_ddr_in1ctrl_reg.regValue);
	}



#ifdef CONFIG_I2RND_ENABLE
	//I2rnd enable M-cap2 block @Crixus 20160721
	if((Scaler_I2rnd_get_timing_enable() == _ENABLE) && ((vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC)||(vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1))){
		mdomain_cap_ddr_in1subctrl_RBUS mdomain_cap_ddr_in1subctrl_reg;

		mdomain_cap_ddr_in1subctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1subCtrl_reg);
		mdomain_cap_ddr_in1subctrl_reg.in1_sub_double_enable = 1;
		mdomain_cap_ddr_in1subctrl_reg.in1_sub_v_flip_3buf_en = 1;
		IoReg_Write32(MDOMAIN_CAP_DDR_In1subCtrl_reg, mdomain_cap_ddr_in1subctrl_reg.regValue );
	}
#endif
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE())
	{
		mdomain_cap_capmain_sbs_vsd_cap_ctrl_RBUS capmain_sbs_vsd_cap_ctrl_reg;
		mdomain_cap_capmain_sbs_vsd_cap_offset_RBUS capmain_sbs_vsd_cap_offset_reg;
		mdomain_cap_ddr_in1ctrl_RBUS ddr_in1ctrl_reg;

		capmain_sbs_vsd_cap_ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CapMain_SBS_VSD_CAP_CTRL_reg);
		capmain_sbs_vsd_cap_offset_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CapMain_SBS_VSD_CAP_OFFSET_reg);
		ddr_in1ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Ctrl_reg);

		if(Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE)==SLR_3D_SIDE_BY_SIDE_HALF)
		{
			capmain_sbs_vsd_cap_ctrl_reg.in1_sbs_vsd_en = 1;
			capmain_sbs_vsd_cap_offset_reg.in1_lr_offset = Scaler_Disp3dGetInfo(display, SLR_DISP_3D_BLKSTEP);
		}
		else
		{
			capmain_sbs_vsd_cap_ctrl_reg.in1_sbs_vsd_en = 0;
			capmain_sbs_vsd_cap_offset_reg.in1_lr_offset = 0;
		}
		IoReg_Write32(MDOMAIN_CAP_CapMain_SBS_VSD_CAP_CTRL_reg, capmain_sbs_vsd_cap_ctrl_reg.regValue);
		IoReg_Write32(MDOMAIN_CAP_CapMain_SBS_VSD_CAP_OFFSET_reg, capmain_sbs_vsd_cap_offset_reg.regValue);

		if((Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING) && ((Scaler_InputSrcGetType(display)==_SRC_HDMI) || (Scaler_InputSrcGetType(display)==_SRC_VO)))
			ddr_in1ctrl_reg.in1_rm_r_vs_en = 1;
		else
			ddr_in1ctrl_reg.in1_rm_r_vs_en = 0;
		ddr_in1ctrl_reg.in1_l_flag_inv = 0;
		IoReg_Write32(MDOMAIN_CAP_DDR_In1Ctrl_reg, ddr_in1ctrl_reg.regValue);
	}
#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
	else if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == FALSE)
		&& (Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING) && (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
	{
		mdomain_cap_ddr_in1ctrl_RBUS ddr_in1ctrl_reg;
		ddr_in1ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Ctrl_reg);
		ddr_in1ctrl_reg.in1_rm_r_vs_en = 1;
		IoReg_Write32(MDOMAIN_CAP_DDR_In1Ctrl_reg, ddr_in1ctrl_reg.regValue);
	}
#endif
	//frank@03132013 add below code for GPU access YUV format ++
	capture1_byte_swap_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Capture1_byte_swap_reg);
	capture2_byte_swap_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Capture2_byte_swap_reg);
	if(display){
		if(Scaler_DispGetStatus(display,SLR_DISP_422CAP))
		{
			capture2_byte_swap_reg.in2_1byte_swap = 0;
			capture2_byte_swap_reg.in2_2byte_swap = 1;
			capture2_byte_swap_reg.in2_4byte_swap = 1;
			capture2_byte_swap_reg.in2_8byte_swap = 0;
		}
		else
		{
			capture2_byte_swap_reg.in2_1byte_swap = 1;
			capture2_byte_swap_reg.in2_2byte_swap = 1;
			capture2_byte_swap_reg.in2_4byte_swap = 1;
			capture2_byte_swap_reg.in2_8byte_swap = 0;
		}
	}else{
		if(Scaler_DispGetStatus(display,SLR_DISP_10BIT))
		{
			capture1_byte_swap_reg.in1_1byte_swap = 0;
			capture1_byte_swap_reg.in1_2byte_swap = 0;
			capture1_byte_swap_reg.in1_4byte_swap = 0;
			capture1_byte_swap_reg.in1_8byte_swap = 0;

		}
		else
		{
			if(Scaler_DispGetStatus(display,SLR_DISP_422CAP))
			{
				capture1_byte_swap_reg.in1_1byte_swap = 0;
				capture1_byte_swap_reg.in1_2byte_swap = 1;
				capture1_byte_swap_reg.in1_4byte_swap = 1;
				capture1_byte_swap_reg.in1_8byte_swap = 0;
			}
			else
			{
				capture1_byte_swap_reg.in1_1byte_swap = 1;
				capture1_byte_swap_reg.in1_2byte_swap = 1;
				capture1_byte_swap_reg.in1_4byte_swap = 1;
				capture1_byte_swap_reg.in1_8byte_swap = 0;
			}
		}
	}
	IoReg_Write32(MDOMAIN_CAP_Capture1_byte_swap_reg, capture1_byte_swap_reg.regValue);
	IoReg_Write32(MDOMAIN_CAP_Capture2_byte_swap_reg, capture2_byte_swap_reg.regValue);
	//frank@03132013 add below code for GPU access YUV format --
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_SetBits(display ? MDOMAIN_CAP_Cap_In2_enable_reg : MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);
#else
	IoReg_SetBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : */MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);
#endif
	if (display == _CHANNEL1) {
		/*wclr cap buf done status*/
		mdomain_cap_cap_ddr_fifostatus_RBUS cap_ddr_fifostatus_reg;
		cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
		cap_ddr_fifostatus_reg.in1_cap_last_wr_flag = 1;
		IoReg_Write32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg, cap_ddr_fifostatus_reg.regValue);
	} else {
		mdomain_cap_cap_ddr_fifostatus_RBUS cap_ddr_fifostatus_reg;
		cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
		cap_ddr_fifostatus_reg.in2_cap_last_wr_flag = 1;
		IoReg_Write32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg, cap_ddr_fifostatus_reg.regValue);
	}
}




//#if 1
void memory_set_memory_size(unsigned char bDouble, eMemCapAccessType Cap_type)
{
#ifdef CONFIG_DUAL_CHANNEL
#ifdef ENABLE_3_BUFFER_DELAY_MODE
	unsigned int  nStartAddr=0;
#endif
#endif
#ifdef CONFIG_ARM64
	unsigned long align_addr = 0, query_addr = 0, vir_addr = 0, align_vir_addr = 0;
#else
	unsigned int align_addr = 0, query_addr = 0, vir_addr = 0, align_vir_addr = 0;
#endif
#ifndef CONFIG_ALL_SOURCE_DATA_FS
	unsigned int buffer_size = 0, buffer_size_page_align = 0;
#endif

	if (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY){
#ifdef CONFIG_ALL_SOURCE_DATA_FS
			Scaler_memory_setting_through_CMA();
#else
#ifdef CONFIG_MDOMAIN_RESERVED_MEMORY

#ifdef CONFIG_I2RND_ENABLE
		if((Scaler_I2rnd_get_timing_enable() == _ENABLE) && ((vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC) || (vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1))){
			rtd_pr_scaler_memory_emerg("[I2RND][%s]Enter cap2 memory allocate\n", __FUNCTION__);
			if(dvrif_memory_compression_get_enable(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL))==_ENABLE)
				buffer_size = MDOMAIN_MAIN_I2R_BUFFER_SIZE;
			else
				buffer_size = MDOMAIN_MAIN_I2R_BUFFER_SIZE;
		}
		else
#endif
		{
			if(dvrif_memory_compression_get_enable(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL))==_ENABLE) {
                buffer_size = calc_compress_memory_size(SLR_MAIN_DISPLAY, dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY));
			} else {
				buffer_size = MDOMAIN_MAIN_BUFFER_SIZE_30BITS;
			}
			if(!is_2k_memory_buffer_source(SLR_MAIN_DISPLAY))
			{
				if(buffer_size > i3ddmaCtrl.cap_buffer[0].size)
                {
                    rtd_pr_scaler_memory_err("[%s %d]ERROR memory is not enough\n", __FUNCTION__, __LINE__);
                }
			}
			//merlin4 memory do 96 bytes align @Crixus 20180321
			query_addr = get_query_start_address(QUERY_IDX_MDOMAIN_MAIN);
            #ifndef BUILD_QUICK_SHOW
			if(get_platform_model() == PLATFORM_MODEL_2K) {
				if(!query_addr)
				{
					if(mdomain_2k_memory_addr)
						pli_free(mdomain_2k_memory_addr);
					mdomain_2k_memory_addr = pli_malloc(buffer_size*3, GFP_DCU1);
					if(mdomain_2k_memory_addr == 0)
					{
						rtd_pr_scaler_memory_emerg("mdomain 2k pli alloc memory fail!\n");
						return;
					}
					query_addr = mdomain_2k_memory_addr;
				}
			}
            #endif
			buffer_size = drvif_memory_get_data_align(buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
			align_addr = dvr_memory_alignment((unsigned long)query_addr, buffer_size);
			#ifdef CONFIG_ARM64
			if (align_addr == (unsigned long)NULL)
        			BUG();
			#else
			if (align_addr == (unsigned int)NULL)
        			BUG();
			#endif

				//Just for reserved memory, update to use query driver @Crixus 20170817
			if(0==is_2k_memory_buffer_source(SLR_MAIN_DISPLAY))
			{//4k
				if(i3ddmaCtrl.cap_buffer[0].phyaddr)
				{
					MemTag[MEMIDX_MAIN_THIRD].StartAddr = drvif_memory_get_data_align(i3ddmaCtrl.cap_buffer[0].phyaddr, DMA_SPEEDUP_ALIGN_VALUE);
					MemTag[MEMIDX_MAIN_SEC].StartAddr = drvif_memory_get_data_align(i3ddmaCtrl.cap_buffer[0].phyaddr + buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
                                 MemTag[MEMIDX_MAIN].StartAddr = drvif_memory_get_data_align(i3ddmaCtrl.cap_buffer[0].phyaddr + buffer_size*2, DMA_SPEEDUP_ALIGN_VALUE);
				}
				else
				{
					rtd_pr_scaler_memory_err("##[ERR] 4k no memory size##\r\n");
					return; 
				}
			}
			else{
				MemTag[MEMIDX_MAIN_THIRD].StartAddr = align_addr;
				MemTag[MEMIDX_MAIN_SEC].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_THIRD].StartAddr+buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
				MemTag[MEMIDX_MAIN].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_SEC].StartAddr+buffer_size, DMA_SPEEDUP_ALIGN_VALUE);			
			}

			//16 bytes alignment
			MemTag[MEMIDX_MAIN_THIRD].StartAddr = MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0;
			MemTag[MEMIDX_MAIN_SEC].StartAddr = MemTag[MEMIDX_MAIN_SEC].StartAddr & 0x7ffffff0;
			MemTag[MEMIDX_MAIN].StartAddr = MemTag[MEMIDX_MAIN].StartAddr & 0x7ffffff0;

			//rtd_pr_scaler_memory_emerg("MemTag[MEMIDX_MAIN].StartAddr=%x, MemTag[MEMIDX_MAIN_SEC].StartAddr=%x, MemTag[MEMIDX_MAIN_THIRD].StartAddr=%x, buffer_size=%x\n", MemTag[MEMIDX_MAIN].StartAddr, MemTag[MEMIDX_MAIN_SEC].StartAddr, MemTag[MEMIDX_MAIN_THIRD].StartAddr, buffer_size);

			#if 1//def CONFIG_REALTEK_2K_MODEL_ENABLED
			//update multi-buffer start address @Crixus 20160913
			drv_memory_Set_multibuffer_size(buffer_size);
			if(drv_memory_Get_multibuffer_flag()){
				rtd_pr_scaler_memory_emerg("[crixus]M-domain Multi-buffer Enable!!!\n");
				drv_memory_capture_multibuffer_update_addr(buffer_size, MemTag[MEMIDX_MAIN].StartAddr);
			}
			#endif

			//free virtual memory
			//drvif_memory_free_fixed_virtual_memory();
			//set virtual memory size
			//rtd_pr_scaler_memory_emerg("buffer_size=%x\n", buffer_size);
			buffer_size_page_align = PAGE_ALIGN(buffer_size);
			//buffer_size = buffer_size_page_align;
			rtd_pr_scaler_memory_emerg("PAGE_ALIGN(buffer_size)=%x\n", PAGE_ALIGN(buffer_size));
			drvif_memory_set_fixed_virtual_memory_size(buffer_size_page_align);

			MemTag[MEMIDX_MAIN_THIRD].Allocate_StartAddr = MemTag[MEMIDX_MAIN_THIRD].StartAddr;
			MemTag[MEMIDX_MAIN_THIRD].Size = buffer_size;
			MemTag[MEMIDX_MAIN_THIRD].Status = ALLOCATED_FROM_PLI;

			MemTag[MEMIDX_MAIN_SEC].Allocate_StartAddr = MemTag[MEMIDX_MAIN_SEC].StartAddr;
			MemTag[MEMIDX_MAIN_SEC].Size = buffer_size;
			MemTag[MEMIDX_MAIN_SEC].Status = ALLOCATED_FROM_PLI;
			MemTag[MEMIDX_MAIN_SEC].Allocate_VirAddr = 0;//not use

			MemTag[MEMIDX_MAIN].Allocate_StartAddr = MemTag[MEMIDX_MAIN].StartAddr;
			MemTag[MEMIDX_MAIN].Size = buffer_size;
			MemTag[MEMIDX_MAIN].Status = ALLOCATED_FROM_PLI;
			MemTag[MEMIDX_MAIN].Allocate_VirAddr = 0;//not use

			//drvif_memory_set_memtag_virtual_startaddr((unsigned long)dvr_remap_uncached_memory(drvif_memory_get_memtag_startaddr(MEMIDX_MAIN_THIRD), buffer_size_page_align, __builtin_return_address(0)));
			//virtual memory allocation	and just use one buffer
			rtd_pr_scaler_memory_debug("MemTag[MEMIDX_MAIN_THIRD].Allocate_VirAddr=%x\n", (unsigned int)MemTag[MEMIDX_MAIN_THIRD].Allocate_VirAddr);
            #ifndef BUILD_QUICK_SHOW
			if (0 == MemTag[MEMIDX_MAIN_THIRD].Allocate_VirAddr)
			{
				//vir_addr = 0, align_vir_addr = 0
				vir_addr = (unsigned long)dvr_remap_uncached_memory(query_addr, buffer_size_page_align, __builtin_return_address(0));
				align_vir_addr = dvr_memory_alignment((unsigned long)vir_addr, buffer_size_page_align);
				drvif_memory_set_memtag_virtual_startaddr(align_vir_addr);
				//drvif_memory_set_memtag_virtual_startaddr((unsigned long)dvr_remap_uncached_memory(drvif_memory_get_memtag_startaddr(MEMIDX_MAIN_THIRD), buffer_size_page_align, __builtin_return_address(0)));

				#ifdef CONFIG_ARM64
					rtd_pr_scaler_memory_debug("vir_addr=%lx, align_vir_addr=%lx\n", vir_addr, align_vir_addr);
				#else
					rtd_pr_scaler_memory_debug("vir_addr=%x, align_vir_addr=%x\n", vir_addr, align_vir_addr);
				#endif
			}
            #endif
			//M-domain start address
			IoReg_Write32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg, (MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0));
			IoReg_Write32(MDOMAIN_CAP_DDR_In1_2ndAddr_reg, (MemTag[MEMIDX_MAIN_SEC].StartAddr & 0x7ffffff0));
			IoReg_Write32(MDOMAIN_CAP_DDR_In1Addr_reg, (MemTag[MEMIDX_MAIN].StartAddr & 0x7ffffff0));
			//M-domain boundary
			//update multi-buffer boundary @Crixus 20160913
			#if 1//def CONFIG_REALTEK_2K_MODEL_ENABLED
			if(drv_memory_Get_multibuffer_flag()){
				drv_memory_capture_multibuffer_update_boundary(drv_memory_Get_multibuffer_number(), buffer_size);
			}
			else
			#endif
			{
				IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,(MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0));
				IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(MemTag[MEMIDX_MAIN].StartAddr+buffer_size) & 0x7ffffff0);

				//Update m-cap limit boundary
				drv_memory_cap_limit_boundary_setting(SLR_MAIN_DISPLAY, buffer_size);

				IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg, drvif_memory_get_block_addr(MEMIDX_MAIN)&0x7ffffff0);
				IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg, drvif_memory_get_block_addr(MEMIDX_MAIN_SEC)&0x7ffffff0);
				IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg, drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD)&0x7ffffff0);
				//update m_disp boundary the same as m_cap boundary @Crixus 20171227
				IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr1_reg, MemTag[MEMIDX_MAIN_THIRD].StartAddr&0x7ffffff0);
				IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr2_reg, (MemTag[MEMIDX_MAIN].StartAddr+buffer_size)&0x7ffffff0);
				//Update s-cap limit boundary
				drv_memory_disp_limit_boundary_setting(SLR_MAIN_DISPLAY);
			}
		}
#endif
#endif
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else {
//-		rtd_pr_scaler_memory_debug("\n Allocate Sub Disp Memory !\n");
		// set capture memory for sub display
		drvif_memory_free_block(MEMIDX_SUB);	// free block befor getting a new one
		drvif_memory_free_block(MEMIDX_SUB_SEC);	// free block befor getting a new one
		drvif_memory_free_block(MEMIDX_SUB_THIRD);	// free block befor getting a new one
		//sub use cap max size @Crixus 20151231
		if(get_hdmi_4k_hfr_mode() != HDMI_NON_4K120)
		{
			if(Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP) == FALSE)
				buffer_size = MDOMAIN_SUB_TWO_PIXEL_BUFFER_SIZE_PC_MODE;
			else
				buffer_size = MDOMAIN_SUB_TWO_PIXEL_BUFFER_SIZE;
		}
		else
			buffer_size = MDOMAIN_SUB_BUFFER_SIZE;
	#ifdef ENABLE_3_BUFFER_DELAY_MODE
			//I2rnd mode s-cap use 2-buffer
			nStartAddr = drvif_memory_alloc_block(MEMIDX_SUB_THIRD, buffer_size, MEMALIGN_ANY);//space unit is byte
			MemTag[MEMIDX_SUB_THIRD].StartAddr = nStartAddr;
			IoReg_Write32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg, nStartAddr & 0x7ffffff0);
		//}
	#endif

		if(bDouble)
		{
			IoReg_Write32(MDOMAIN_CAP_DDR_In2_2ndAddr_reg, drvif_memory_alloc_block(MEMIDX_SUB_SEC,buffer_size, MEMALIGN_ANY) & 0x7ffffff0);
		}

		IoReg_Write32(MDOMAIN_CAP_DDR_In2Addr_reg, drvif_memory_alloc_block(MEMIDX_SUB, buffer_size, MEMALIGN_ANY) & 0x7ffffff0);
		rtd_pr_scaler_memory_debug("###################sub buffer TotalSize=%d;;\n", buffer_size);
//-		rtd_pr_scaler_memory_debug("Get allocated sub display addr1=%x, addr2=%x, size=%x\n", StartAddr, StartAddr1, TotalSize);
	}
#endif
}
#ifndef BUILD_QUICK_SHOW
void dvrif_memory_setting_for_data_fs(void)
{
	SCALER_DISP_CHANNEL display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);

	if (display == SLR_MAIN_DISPLAY) {	// Main
		if((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 1))	// yychao+ : use single buffer for JPEG source
			memory_set_memory_size(FALSE, MEMCAPTYPE_FRAME);	//  CSW 0961105 change single buffer to double to slove 1080i moving picture not smooth
		else
		{
			if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && (dvrif_memory_compression_get_enable(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) == TRUE)){
				//update M-domain PQC frame mode @Crixus 20170626
				if(dvrif_memory_get_compression_mode(SLR_MAIN_DISPLAY) == COMPRESSION_LINE_MODE)
					memory_set_memory_size(TRUE, MEMCAPTYPE_LINE);
				else
					memory_set_memory_size(TRUE, MEMCAPTYPE_FRAME);
			}
			else{
				memory_set_memory_size(TRUE, MEMCAPTYPE_FRAME);	//  CSW 0961105 change single buffer to double to slove 1080i moving picture not smooth
			}
		}
        }
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else {	//Sub enable
		if (Scaler_PipGetInfo(SLR_PIP_TYPE) <= SUBTYPE_PIPMLSR_4x3)
		{// Sub PIP
#ifdef CONFIG_I2RND_ENABLE
			//I2rnd sub cap=>VO have to run line mode to meet line step @Crixus 20160712
			if((Scaler_I2rnd_get_timing_enable() == _ENABLE) && (vsc_i2rnd_sub_stage == I2RND_STAGE_DETECT_SOURCE))
				memory_set_memory_size(TRUE, MEMCAPTYPE_LINE);
			else
#endif
				memory_set_memory_size(TRUE, MEMCAPTYPE_FRAME); // memory, capture part for other cases
		}
		/*else if (Scaler_PipGetInfo(SLR_PIP_TYPE) <= SUBTYPE_MP1L7B)
		{// Sub MP
			memory_set_capture_for_mp(); // memory, capture part for Multi Picture
			memory_set_sub_for_mp();
			//ScalerTimer_ActiveTimerEvent(SLRTIMER_PIPMP, 1000, (SLRTIMER_CALLBACK)PipMp_RotateMp);
			Scaler_PipSetSubInfo(SLR_PIP_SUB_MPENABLE,1);
		}*/
		else
		{
#ifdef CONFIG_I2RND_ENABLE
			//I2rnd sub cap=>VO have to run line mode to meet line step @Crixus 20160712
			if((Scaler_I2rnd_get_timing_enable() == _ENABLE) && (vsc_i2rnd_sub_stage == I2RND_STAGE_DETECT_SOURCE))
				memory_set_memory_size(TRUE, MEMCAPTYPE_LINE);
			else
#endif
				if(dvrif_memory_compression_get_enable(SLR_SUB_DISPLAY) == TRUE){
					//update M-domain PQC frame mode @Crixus 20170626
					if(dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) == COMPRESSION_LINE_MODE)
						memory_set_memory_size(TRUE, MEMCAPTYPE_LINE);
					else
						memory_set_memory_size(TRUE, MEMCAPTYPE_FRAME);
					}else{
						memory_set_memory_size(TRUE, MEMCAPTYPE_FRAME); // memory, capture part for other cases
					}
		}
	}
#endif //CONFIG_DUAL_CHANNEL  // There are two channels

}
/*============================================================================*/
/**
 * MemSetCapture
 * Set memory capture registers. It can be #1 or #2, based on current info structure
 *
 * @param <info> { display-info struecture }
 * @return { none }
 *
 */
void jpeg_memory_alloc_capture2(eMemCapAccessType Cap_type,eMemIdx idx)
{
	unsigned int  TotalSize;
	unsigned int  TotalSize1;
	//unsigned int  nStartAddr;
	unsigned int  Quotient;
	unsigned char  Remainder;
	unsigned int burstLen = (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)?_BURSTLENGTH2: _BURSTLENGTH);

	//frank@0325 double the M domain burst length to avoid 444 10 bits peak BW not enough
	if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)_CHANNEL1, SLR_DISP_422CAP))){
		burstLen=0xf0;
	}

	TotalSize = memory_get_capture_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), Cap_type);		// calculate the memory size of capture
	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division
	Remainder = drvif_memory_get_data_align(Remainder, 4);	// multiple of 4
	if (Remainder == 0) {	// remainder is not allowed to be zero
		Remainder = burstLen;
		Quotient -= 1;
	}

	TotalSize = SHL(Quotient * burstLen + Remainder, 3); //unit conversion. 64bits to 8bits

	TotalSize1 = memory_get_capture_max_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), Cap_type);

	TotalSize1 = SHL(TotalSize1, 3); //unit conversion. 64bits to 8bits

	if(Cap_type == MEMCAPTYPE_LINE){
		TotalSize1 *=Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_IPV_ACT_LEN_PRE);
		TotalSize *=Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_CAP_LEN);
	}


	if (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY)	 {
		// set capture memory for main display
		if((TotalSize1 > TotalSize) &&
			((Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_PIPMLSR) && (Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_PIPMLSR_4x3) &&
			(Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_MP1C12B) && (Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_MP1L7B)))
			TotalSize1 = (TotalSize1 - TotalSize);
		else
			TotalSize1 = 0;


		//nStartAddr = drvif_memory_alloc_block(idx, TotalSize+TotalSize1+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//space unit is byte
		drvif_memory_alloc_block(idx, TotalSize+TotalSize1+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//space unit is byte
		//rtd_pr_scaler_memory_debug("[JPEG] %s MemTag[MEMIDX_SUB].StartAddr=%x\n",__FUNCTION__,nStartAddr);
	}
}
void jpeg_memory_free_capture2(void)
{
	drvif_memory_free_block(MEMIDX_SUB);
}

void zoom_memory_set_capture(unsigned char bDouble, eMemCapAccessType Cap_type, unsigned char buffernum)
{
	unsigned int  TotalSize;
	unsigned int  TotalSize1;
//	unsigned int  TotalSizeFrame;
	unsigned int  nStartAddr;
	unsigned int  Quotient;
	unsigned char  Remainder;
	unsigned int blockstep;//need to be even
	//unsigned int multiOf2D3DBuf = 1;
	mdomain_cap_capture1_byte_swap_RBUS capture_byte_swap_reg;
	unsigned int burstLen = (Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)?_BURSTLENGTH2: _BURSTLENGTH);

	//frank@0325 double the M domain burst length to avoid 444 10 bits peak BW not enough
	if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)_CHANNEL1, SLR_DISP_422CAP))){
		burstLen=0xf0;
	}

#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_ClearBits(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg: MDOMAIN_CAP_DDR_In1Ctrl_reg, MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask);	// disable Capture
	IoReg_ClearBits(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_Cap_In2_enable_reg : MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);	// disable Capture
#else
	IoReg_ClearBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : */MDOMAIN_CAP_DDR_In1Ctrl_reg, MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask); // disable Capture
	IoReg_ClearBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : */MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0); // disable Capture
#endif
	//frank@0503 add below delay to confirm M capture have disable
	msleep(20);

	//field polarity error check
	memory_check_capture_polarity_error(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL));


	TotalSize = memory_get_capture_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), Cap_type);		// calculate the memory size of capture
	if(Cap_type == MEMCAPTYPE_LINE){
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		IoReg_Write32(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2LineStep_reg: MDOMAIN_CAP_DDR_In1LineStep_reg , (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule
		blockstep = (TotalSize*Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_CAP_LEN));
		blockstep = (blockstep % 2)? blockstep + 1: blockstep;
		IoReg_Write32(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2BlkStep_reg: MDOMAIN_CAP_DDR_In1BlkStep_reg, blockstep << 3);
#else
		IoReg_Write32(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2LineStep_reg : */MDOMAIN_CAP_DDR_In1LineStep_reg , (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule
		blockstep = (TotalSize*Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_CAP_LEN));
		blockstep = (blockstep % 2)? blockstep + 1: blockstep;
		IoReg_Write32(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2BlkStep_reg : */MDOMAIN_CAP_DDR_In1BlkStep_reg, blockstep << 3);
#endif

	}else{
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		IoReg_Write32(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2LineStep_reg: MDOMAIN_CAP_DDR_In1LineStep_reg, 0);
#else
		IoReg_Write32(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2LineStep_reg : */MDOMAIN_CAP_DDR_In1LineStep_reg, 0);
#endif
	}
//-	rtd_pr_scaler_memory_debug("Cap Act wid=%x, Cap Act len=%x\n", Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID), Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN));
//-	rtd_pr_scaler_memory_debug("\n Calculate FIFO Size !");
	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division
	Remainder = drvif_memory_get_data_align(Remainder, 4);	// multiple of 4
	if (Remainder == 0) {	// remainder is not allowed to be zero
		Remainder = burstLen;
		Quotient -= 1;
	}
	rtd_pr_scaler_memory_debug("[ZOOM][Memory]WL=%x,Q=%x,Rm=%x\n", burstLen,Quotient,Remainder);
	memory_set_capture_fifo(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), Quotient, Remainder, burstLen);	// set capture FIFO

	TotalSize = SHL(Quotient * burstLen + Remainder, 3); //unit conversion. 64bits to 8bits

	TotalSize1 = memory_get_capture_max_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), Cap_type);

	TotalSize1 = SHL(TotalSize1, 3); //unit conversion. 64bits to 8bits

	if(Cap_type == MEMCAPTYPE_LINE){
		TotalSize1 *=Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_IPV_ACT_LEN_PRE);
		TotalSize *=Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_CAP_LEN);
	}

	if (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY)	 {
		if (buffernum)
			drvif_memory_free_block(MEMIDX_MAIN_SEC);	// free block befor getting a new one
		else
			drvif_memory_free_block(MEMIDX_MAIN);

		if((TotalSize1 > TotalSize) &&
			((Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_PIPMLSR) && (Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_PIPMLSR_4x3) &&
			(Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_MP1C12B) && (Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_MP1L7B)))
			TotalSize1 = (TotalSize1 - TotalSize);
		else
		{
			rtd_pr_scaler_memory_debug("TOTAL SIZE1 == 0\n");
			TotalSize1 = 0;
		}

		if(bDouble) {
			//nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_SEC, TotalSize+TotalSize1, MEMALIGN_ANY);//space unit is byte
			//MemTag[MEMIDX_MAIN_SEC].StartAddr = nStartAddr;
			//IoReg_Write32(MDOMAIN_CAP_DDR_In1_2ndAddr_reg, nStartAddr & 0x7ffffff0);
		}


		if (buffernum)
			nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_SEC, TotalSize+TotalSize1+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//space unit is byte
		else
			nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN, TotalSize+TotalSize1+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//space unit is byte

		if (bDouble)
		{
			if (buffernum)
				MemTag[MEMIDX_MAIN_SEC].StartAddr = nStartAddr;
			else
				MemTag[MEMIDX_MAIN].StartAddr = nStartAddr;
			IoReg_Write32(MDOMAIN_CAP_DDR_In1_2ndAddr_reg, nStartAddr & 0x7ffffff0);
		}
		else
		{

			if (buffernum)
				MemTag[MEMIDX_MAIN_SEC].StartAddr = nStartAddr;
			else
				MemTag[MEMIDX_MAIN].StartAddr = nStartAddr;

			IoReg_Write32(MDOMAIN_CAP_DDR_In1Addr_reg, nStartAddr & 0x7ffffff0);

			//rtd_pr_scaler_memory_debug("After %s--%d  IoReg_Read32(MDOMAIN_CAP_DDR_In1LineStep_reg)=%x\n",__FUNCTION__,__LINE__,IoReg_Read32(MDOMAIN_CAP_DDR_In1LineStep_reg));
		}
	}

	//frank@03132013 add below code for GPU access YUV format ++
	capture_byte_swap_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Capture1_byte_swap_reg);
	if(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)){
		//Crixus@20140722 - Mac3 no in2
		//capture_byte_swap_reg.in2_2byte_swap = 1;
		//capture_byte_swap_reg.in2_4byte_swap = 1;
	}else{
		capture_byte_swap_reg.in1_2byte_swap = 1;
		capture_byte_swap_reg.in1_4byte_swap = 1;
	}
	IoReg_Write32(MDOMAIN_CAP_Capture1_byte_swap_reg, capture_byte_swap_reg.regValue);
	//frank@03132013 add below code for GPU access YUV format --

#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_Mask32(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2LineNum_reg: MDOMAIN_CAP_DDR_In1LineNum_reg, 0xfffee000, (Cap_type<<16)|AND(Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN), 0x1FFF));
	//IoReg_Write32(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : MDOMAIN_CAP_DDR_In1Ctrl_reg, _BIT0 | (bDouble ? MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask : 0));
	IoReg_SetBits(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)?MDOMAIN_CAP_DDR_In2Ctrl_reg:MDOMAIN_CAP_DDR_In1Ctrl_reg, _BIT0 | (bDouble ? MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask : 0));
	IoReg_SetBits(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_Cap_In2_enable_reg: MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);
#else
	IoReg_Mask32(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2LineNum_reg : */MDOMAIN_CAP_DDR_In1LineNum_reg, 0xfffee000, (Cap_type<<16)|AND(Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN), 0x1FFF));
	IoReg_SetBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : */MDOMAIN_CAP_DDR_In1Ctrl_reg, _BIT0 | (bDouble ? MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask : 0));
	IoReg_SetBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : */MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);
#endif


	//drvif_memory_3D_mode_sw_buffer_swap_action((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), MEM_3D_SWBUFFER_SWAP_UPDATE_INFO);
}

void memory_set_vflip_capture(unsigned char bVflip, eMemCapAccessType Cap_type)
{

	unsigned int  TotalSize;
	unsigned int  TotalSize1;
//	unsigned int  TotalSizeFrame;
	unsigned int  nStartAddr;
	unsigned int  Quotient;
	unsigned char  Remainder;
	//unsigned int multiOf2D3DBuf = 1;
	unsigned int burstLen = (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)?_BURSTLENGTH2: _BURSTLENGTH);
	//frank@0325 double the M domain burst length to avoid 444 10 bits peak BW not enough
	if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)_CHANNEL1, SLR_DISP_422CAP))){
		burstLen=0xf0;
	}

	TotalSize = memory_get_capture_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), Cap_type);		// calculate the memory size of capture


	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division
	Remainder = drvif_memory_get_data_align(Remainder, 4);	// multiple of 4
	if (Remainder == 0) {	// remainder is not allowed to be zero
		Remainder = burstLen;
		Quotient -= 1;
	}


	TotalSize = SHL(Quotient * burstLen + Remainder, 3); //unit conversion. 64bits to 8bits

	TotalSize1 = memory_get_capture_max_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), Cap_type);

	TotalSize1 = SHL(TotalSize1, 3); //unit conversion. 64bits to 8bits

	if(Cap_type == MEMCAPTYPE_LINE){
		TotalSize1 *=Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_IPV_ACT_LEN_PRE);
		TotalSize *=Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_CAP_LEN);
	}


		if (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY)	 {

			drvif_memory_free_block(MEMIDX_MAIN_THIRD);	// free block befor getting a new one

			if((TotalSize1 > TotalSize) &&
				((Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_PIPMLSR) && (Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_PIPMLSR_4x3) &&
				(Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_MP1C12B) && (Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_MP1L7B)))
				TotalSize1 = (TotalSize1 - TotalSize);
			else
				TotalSize1 = 0;

			if(bVflip) {
				nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_THIRD, TotalSize+TotalSize1+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);//space unit is byte
				MemTag[MEMIDX_MAIN_THIRD].StartAddr = nStartAddr;
				IoReg_Write32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg, nStartAddr & 0x7ffffff0);
			}

		}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		else {
	//-		rtd_pr_scaler_memory_debug("\n Allocate Sub Disp Memory !\n");
			// set capture memory for sub display
			drvif_memory_free_block(MEMIDX_SUB_THIRD);	// free block befor getting a new one

			if(bVflip)
				IoReg_Write32(MDOMAIN_CAP_DDR_In2_3rdAddr_reg, drvif_memory_alloc_block(MEMIDX_SUB_THIRD,TotalSize+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY) & 0x7ffffff0);
	//-		rtd_pr_scaler_memory_debug("Get allocated sub display addr1=%x, addr2=%x, size=%x\n", StartAddr, StartAddr1, TotalSize);
		}
#endif

#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_SetBits(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : MDOMAIN_CAP_DDR_In1Ctrl_reg, MDOMAIN_CAP_DDR_In1Ctrl_in1_v_flip_3buf_en_mask);
#else
	IoReg_SetBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : */MDOMAIN_CAP_DDR_In1Ctrl_reg, MDOMAIN_CAP_DDR_In1Ctrl_in1_v_flip_3buf_en_mask);
#endif

}


//(original  block address / 8 ) + (line size *  (line number - 1) ) )*8
static unsigned int memory_cal_vflip_block_addr(unsigned int address, unsigned int linesize , unsigned int linenum)
{
	return ((address >> 3) + (linesize * (linenum - 1)) )<< 3;
}

static void MemSetDisplayVFLIPAddr(unsigned char display, unsigned int Addr1, unsigned int Addr2, unsigned int Addr3)
{
	unsigned int address = 0;

	if(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY){
	//frank@0617 mark below code to speed up
	//	dvrif_memory_set_addr_check(SLR_MAIN_DISPLAY, _TRUE);
		IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg,Addr1&0x7ffffff0);
        	IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg,Addr2&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg,Addr3&0x7ffffff0);

		//update m_disp boundary the same as m_cap boundary @Crixus 20171227
		address = IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg);
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr1_reg, address&0x7ffffff0);
		address = IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg);
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr2_reg, address&0x7ffffff0);
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else{
		IoReg_Write32(MDOMAIN_DISP_DDR_SubAddr_reg, Addr1&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DDR_Sub2ndAddr_reg, Addr2&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DDR_Sub3rdAddr_reg,Addr3&0x7ffffff0);

		//update m_disp boundary the same as m_cap boundary @Crixus 20171227
		address = IoReg_Read32(MDOMAIN_CAP_CapSub_BoundaryAddr1_reg);
		IoReg_Write32(MDOMAIN_DISP_DispSub_BoundaryAddr1_reg, address&0x7ffffff0);
		address = IoReg_Read32(MDOMAIN_CAP_CapSub_BoundaryAddr2_reg);
		IoReg_Write32(MDOMAIN_DISP_DispSub_BoundaryAddr2_reg, address&0x7ffffff0);
	}
#endif
}


#endif
static void memory_set_vflip_main(unsigned char bVflip)
{
#ifndef BUILD_QUICK_SHOW

	unsigned char display = SLR_MAIN_DISPLAY;
	unsigned int TotalSize;
//	unsigned int  TotalSizeFrame=0;
	unsigned int Quotient;
	unsigned char Remainder;
	unsigned int ucStartAddress;
	unsigned int ucStartAddress2;
        unsigned int ucStartAddress3 = 0;
	unsigned int LineStep;
	unsigned int LineStep_96Align = 0;
	unsigned int fifoLen = (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)?_FIFOLENGTH2: _FIFOLENGTH);
	unsigned int burstLen = (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)?_BURSTLENGTH2: _BURSTLENGTH);
	UINT32 MemShiftAddr = 0;
	UINT16 droppixel = 0;
	StructSrcRect m_dispwin;
	mdomain_disp_ddr_mainprevstart_RBUS ddr_mainprevstart_reg;
	//unsigned int  Align_LineStep;
	unsigned int comp_width = 0;
	//Update the fifo depth for each IC @Crixus 20170418
#ifdef MERLIN_FIFO_DEPTH
	//frank@0325 double the M domain burst length to avoid 444 10 bits peak BW not enough
	if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)_CHANNEL1, SLR_DISP_422CAP))){
		burstLen=0xf0; //96-bytes align
	}
#endif
	display = (Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) ? SLR_MAIN_DISPLAY : SLR_SUB_DISPLAY;
	m_dispwin = Scaler_DispWindowGet();
	ddr_mainprevstart_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainPreVStart_reg);
	if(m_dispwin.srcy <= 100) {
		ddr_mainprevstart_reg.main_pre_rd_v_start = 7;
	} else {
		ddr_mainprevstart_reg.main_pre_rd_v_start = (m_dispwin.srcy-10);
	}
	IoReg_Write32(MDOMAIN_DISP_DDR_MainPreVStart_reg, ddr_mainprevstart_reg.regValue);

	IoReg_Write32(MDOMAIN_DISP_DDR_MainAddrDropBits_reg,0x0);
	//frank@0323 change waterlevel/2 for AT,because water level nit is 128 bits

	rtd_pr_scaler_memory_debug("[vFlip] mem info %d %d %d %d\n",Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));

	if ((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA) == 0) && (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) == 0) )
	{
		if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && (dvrif_memory_compression_get_enable(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) == TRUE)){
			comp_width = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID);
			//new PQDC are all align 128, use original width. @Crixus 20170615
			IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, ((comp_width & 0x0fFF) << 8) | ((fifoLen-(burstLen>>1))>>1));
			TotalSize = memory_get_capture_line_size_compression(dvrif_memory_get_compression_bits(display));
			//merlin4 memory do 96 bytes align @Crixus 20180321
			//TotalSize = dvr_size_alignment(TotalSize);
		}
		else{

			IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, ((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) & 0x0fFF) << 8) | ((fifoLen-(burstLen>>1))>>1));
			TotalSize = memory_get_display_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY));	// calculate the memory size of capture
		}
	}
	else
	{
		MemGetBlockShiftAddr(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA), &MemShiftAddr, &droppixel);
		TotalSize = memory_get_disp_line_size1(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) + droppixel);	// calculate the memory size of capture
		IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, (( ((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) +droppixel) & 0x0fFF) << 8) | ((fifoLen-(burstLen>>1))>>1)));
		rtd_pr_scaler_memory_debug("[vFlip] drop pixel=%d shift addr=%x\n",droppixel,MemShiftAddr);

	}

	rtd_pr_scaler_memory_debug("[crixus][v-flip]TotalSize = %d\n", TotalSize);



        IoReg_Mask32(MDOMAIN_DISP_DDR_MainLineNum_reg, 0xfffff000, (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN) & 0x0FFF));
		memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division

	// remainder is not allowed to be zero
	if (Remainder == 0) {
		Remainder = burstLen;
		Quotient -= 1;
	}

	IoReg_Write32(MDOMAIN_DISP_DDR_MainRdNumLenRem_reg, (Quotient << 16) | (burstLen << 8) | Remainder);

	if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && (dvrif_memory_compression_get_enable(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) == TRUE))
		TotalSize = memory_get_capture_line_size_compression(dvrif_memory_get_compression_bits(display));
	else
		TotalSize = memory_get_capture_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), MEMCAPTYPE_LINE);		// calculate capture line size

	//merlin4 memory do 96 bytes align @Crixus 20180321
	//TotalSize = dvr_size_alignment(TotalSize);

	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF))
		TotalSize=TotalSize>>1;

	LineStep = TotalSize;
	LineStep_96Align = TotalSize;
	//rtd_pr_scaler_memory_debug("[vFlip]line step=0x%x\n",LineStep);

	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits
	//Align_LineStep = LineStep;
	//Align_LineStep = (Align_LineStep % 2) ? (Align_LineStep + 1) : Align_LineStep;
	if (dvrif_memory_compression_get_enable(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) == TRUE)
	{
		//rtd_pr_scaler_memory_emerg("file=%s, function=%s, line=%d\n", __FILE__, __FUNCTION__, __LINE__);
		//rtd_pr_scaler_memory_emerg("m-cap compress enable\n");
		if (dvrif_memory_get_compression_mode(display) == COMPRESSION_LINE_MODE)
		{
			LineStep_96Align = LineStep;//drvif_memory_get_data_align(LineStep, DMA_SPEEDUP_ALIGN_VALUE/8);
			//rtd_pr_scaler_memory_emerg("m-cap compress line mode\n");
		}
		else
		{
			//rtd_pr_scaler_memory_emerg("file=%s, function=%s, line=%d\n", __FILE__, __FUNCTION__, __LINE__);
			//rtd_pr_scaler_memory_emerg("m-cap compress frame mode\n");
		}
	}
	// Not compress, check if m-cap line mode?
	else if (MEMCAPTYPE_LINE == dvrif_memory_get_cap_mode(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)))
	{
		LineStep_96Align = LineStep;//drvif_memory_get_data_align(LineStep, DMA_SPEEDUP_ALIGN_VALUE/8);
		//rtd_pr_scaler_memory_emerg("file=%s, function=%s, line=%d\n", __FILE__, __FUNCTION__, __LINE__);
		//rtd_pr_scaler_memory_emerg("m-cap not compress line mode\n");
	}

	LineStep_96Align = LineStep_96Align + LineStep_96Align%2;
	rtd_pr_scaler_memory_emerg("[crixus][v-flip]Align_LineStep = 0x%x", ((~LineStep_96Align + 1) << 3));

    IoReg_Write32(MDOMAIN_DISP_DDR_MainLineStep_reg, (~LineStep_96Align + 1) << 3);//bit 3 need to be 0. rbus rule

	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()
		&& ((Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE)==SLR_3D_SIDE_BY_SIDE_HALF)||(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE)==SLR_3D_TOP_AND_BOTTOM)||(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)))
	{
		unsigned char ddr_bit;
		unsigned int temp;
		ddr_bit=(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_CHANNEL) ? 0 : (Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_10BIT) << 1))  | ( !Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_422CAP));

		if(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE)==SLR_3D_TOP_AND_BOTTOM)
			MemShiftAddr=((Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MEM_LEN_PRE)-Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN))<<1)*LineStep;
		else if(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)
		{
			MemShiftAddr=((Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MEM_LEN_PRE)-Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN))>>2)*3*LineStep;
		}
		else if(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE)==SLR_3D_SIDE_BY_SIDE_HALF)
			MemShiftAddr=((Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MEM_LEN_PRE)-Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN))>>1)*LineStep;

		rtd_pr_scaler_memory_debug("[3d]shift addr v=0x %x,len pre- len =%d \n",MemShiftAddr,Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MEM_LEN_PRE)-Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
		temp=memory_get_line_size(((Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_MEM_WID_PRE)-Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID))>>1), (ddr_bit & _BIT0), ((ddr_bit & _BIT1) >> 1));
		temp=drvif_memory_get_data_align(temp, 2);
		MemShiftAddr=MemShiftAddr+temp;
		//rtd_pr_scaler_memory_debug("[3d]shift addr h=%x\n",MemShiftAddr);
		MemShiftAddr=MemShiftAddr<<3;
		rtd_pr_scaler_memory_debug("[3d][vFlip]shift addr after <<3 =%x\n",MemShiftAddr);
	}

	ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN) + MemShiftAddr;
	ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + MemShiftAddr;

	IoReg_Write32(MDOMAIN_DISP_DDR_MainAddrDropBits_reg,droppixel);

	ucStartAddress3 = drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) + MemShiftAddr;

	Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_DISPLAY_SIZE, memory_get_display_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)));

	rtd_pr_scaler_memory_debug("[vFlip]before address %x %x %x\n",ucStartAddress,ucStartAddress2, ucStartAddress3);
	//LineStep = drvif_memory_get_data_align(LineStep, DMA_SPEEDUP_ALIGN_VALUE/8);
	ucStartAddress = memory_cal_vflip_block_addr(ucStartAddress, LineStep , Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
	ucStartAddress2 = memory_cal_vflip_block_addr(ucStartAddress2, LineStep ,Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
	ucStartAddress3 = memory_cal_vflip_block_addr(ucStartAddress3, LineStep , Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
	rtd_pr_scaler_memory_debug("[vFlip]after address %x %x %x\n",ucStartAddress,ucStartAddress2, ucStartAddress3);

	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() /*&&(Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_3DTYPE))!=SLR_3D_2D_ONLY*/)
	{
		MemDispStartAddr_3D[0]=ucStartAddress;
		MemDispStartAddr_3D[1]=ucStartAddress2;
		MemDispStartAddr_3D[2]=ucStartAddress3;
	}

	MemSetDisplayVFLIPAddr(SLR_MAIN_DISPLAY, ucStartAddress,ucStartAddress2,ucStartAddress3);

	//update limit mode boundary
	drv_memory_disp_limit_boundary_setting(SLR_MAIN_DISPLAY);

	TotalSize = LineStep * Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_MEM_ACT_LEN);

	IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg, (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule

	rtd_pr_scaler_memory_debug("[vFlip]block step %x\n", ((TotalSize + TotalSize%2) << 3));

	//rtd_pr_scaler_memory_debug("%s %d bDouble=%d\n",__FUNCTION__,__LINE__,bDouble);
	if(bVflip) {	// use double buffer
		TotalSize = LineStep * Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_MEM_ACT_LEN);
		IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg, (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule

	}

	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE())
		memory_set_main_3D_setting(LineStep);
#endif
}
#ifndef BUILD_QUICK_SHOW

void zoom_memory_set_vflip_main(unsigned char bVflip,unsigned char buffernum)
{
	unsigned int TotalSize;
//	unsigned int  TotalSizeFrame=0;
	unsigned int Quotient;
	unsigned char Remainder;
	unsigned int ucStartAddress;
	unsigned int ucStartAddress2=0;
	unsigned int ucStartAddress3 = 0;
	unsigned int LineStep;
	unsigned int fifoLen = _FIFOLENGTH;
	unsigned int burstLen = _BURSTLENGTH;
	StructSrcRect m_dispwin;
	mdomain_disp_ddr_mainprevstart_RBUS ddr_mainprevstart_reg;
	unsigned int Align_LineStep;

	//frank@0325 double the M domain burst length to avoid 444 10 bits peak BW not enough
	if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)_CHANNEL1, SLR_DISP_422CAP))){
		burstLen=0xf0;
	}

	m_dispwin = Scaler_DispWindowGet();
	ddr_mainprevstart_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainPreVStart_reg);
	if(m_dispwin.srcy <= 100) {
		ddr_mainprevstart_reg.main_pre_rd_v_start = 7;
	} else {
		ddr_mainprevstart_reg.main_pre_rd_v_start = (m_dispwin.srcy-10);
	}
	IoReg_Write32(MDOMAIN_DISP_DDR_MainPreVStart_reg, ddr_mainprevstart_reg.regValue);

	//frank@0323 change waterlevel/2 for AT,because water level nit is 128 bits
	IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, ((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID) & 0x0FFF) << 8) | ((fifoLen-(burstLen>>1))>>1));
	IoReg_Mask32(MDOMAIN_DISP_DDR_MainLineNum_reg, 0xfffff000, (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN) & 0x0FFF));

	TotalSize = memory_get_display_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY));	// calculate the memory size of capture
	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division


	// remainder is not allowed to be zero
	if (Remainder == 0) {
		Remainder = burstLen;
		Quotient -= 1;
	}

	IoReg_Write32(MDOMAIN_DISP_DDR_MainRdNumLenRem_reg, (Quotient << 16) | (burstLen << 8) | Remainder);

	TotalSize = memory_get_capture_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), MEMCAPTYPE_LINE);		// calculate capture line size
	LineStep = TotalSize;
	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits
	Align_LineStep = LineStep;
	Align_LineStep = (Align_LineStep % 2) ? (Align_LineStep + 1) : Align_LineStep;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainLineStep_reg, ((~Align_LineStep) + 1) << 3);//bit 3 need to be 0. rbus rule

	if (bVflip)
	{
		if (buffernum == 0)
		{
			ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_MAIN) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
			ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
		}
		else
		{
			ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
			ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
		}

		ucStartAddress = memory_cal_vflip_block_addr(ucStartAddress, LineStep , Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
		ucStartAddress2 = memory_cal_vflip_block_addr(ucStartAddress2, LineStep ,Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
		if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_VFLIP3)){
			ucStartAddress3 = drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
			ucStartAddress3 = memory_cal_vflip_block_addr(ucStartAddress3, LineStep , Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
		}
	}
	else
	{
		if (buffernum == 0)
		{
			ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN) +(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
		}
		else
		{
			ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA)) + (Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA) *TotalSize);
		}

		ucStartAddress = memory_cal_vflip_block_addr(ucStartAddress, LineStep , Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN));
	}


	if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_VFLIP3))
		MemSetDisplayVFLIPAddr(SLR_MAIN_DISPLAY, ucStartAddress,ucStartAddress2,ucStartAddress3);
	else
		MemSetDisplayAddr(SLR_MAIN_DISPLAY, ucStartAddress, ucStartAddress2);


	if(bVflip) {	// use double buffer
		TotalSize = LineStep * Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_MEM_ACT_LEN);

		IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg, (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule
	}


	//IoReg_SetBits(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_DISP_DDR_SubCtrl_reg : MDOMAIN_DISP_DDR_MainCtrl_reg, MDOMAIN_DISP_DDR_MainCtrl_main_v_flip_3buf_en_mask);

}

void memory_set_main_activewindow_change(void)
{
	UINT32 MemShiftAddr = 0;
	UINT16 droppixel = 0;
	unsigned int ucStartAddress=0;
	unsigned int ucStartAddress2=0;
	unsigned int ucStartAddress3=0;
	if (Get_PANEL_VFLIP_ENABLE() && Scaler_InputSrcGetMainChType() != _SRC_VO){// hdmi 1080i 2dcvt 3d, source will be hdmi , not vo source
		MemGetBlockShiftAddr(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA), &MemShiftAddr, &droppixel);
		ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN) + MemShiftAddr;
		ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + MemShiftAddr;
		if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_VFLIP3))
			ucStartAddress3 = drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) + MemShiftAddr;
		if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_VFLIP3)){
				IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg,ucStartAddress&0x7ffffff0);
		        IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg,ucStartAddress2&0x7ffffff0);
				IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg,ucStartAddress3&0x7ffffff0);
		}else{
				//rtd_pr_scaler_memory_debug("Before %s %d IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg)=%x\n",__FUNCTION__,__LINE__,IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg));
				IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg, ucStartAddress&0x7ffffff0);
				//rtd_pr_scaler_memory_debug("After %s %d IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg)=%x\n",__FUNCTION__,__LINE__,IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg));
				IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg, ucStartAddress2&0x7ffffff0);
		}
	}else{
		MemGetBlockShiftAddr(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA),Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA), &MemShiftAddr, &droppixel);
		ucStartAddress = drvif_memory_get_block_addr(MEMIDX_MAIN) + MemShiftAddr;
		ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + MemShiftAddr;
		//rtd_pr_scaler_memory_debug("Before %s %d IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg)=%x\n",__FUNCTION__,__LINE__,IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg));
		IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg, ucStartAddress&0x7ffffff0);
		//rtd_pr_scaler_memory_debug("After %s %d IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg)=%x\n",__FUNCTION__,__LINE__,IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg));
		IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg, ucStartAddress2&0x7ffffff0);
	}
}


void memory_set_main_displaywindow_change(void)
{
	SCALER_DISP_CHANNEL display = SLR_MAIN_DISPLAY;
	unsigned int mainctrltemp = 0;
	unsigned int TotalSize;
	unsigned int Quotient;
	unsigned char Remainder;
	unsigned int ucStartAddress;
	unsigned int ucStartAddress2=0;
	unsigned int ucStartAddress3=0;
	unsigned int LineStep;
	unsigned int comp_width = 0;
	unsigned int fifoLen = _FIFOLENGTH;
	unsigned int burstLen = _BURSTLENGTH;
	UINT32 MemShiftAddr = 0;
	UINT16 droppixel = 0;
	StructSrcRect m_dispwin;
	mdomain_disp_ddr_mainprevstart_RBUS ddr_mainprevstart_reg;

	//frank@0325 double the M domain burst length to avoid 444 10 bits peak BW not enough
	if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)_CHANNEL1, SLR_DISP_422CAP))){
		burstLen=0xf0;
	}

	m_dispwin = Scaler_DispWindowGet();
	ddr_mainprevstart_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainPreVStart_reg);
	if(m_dispwin.srcy <= 100) {
		ddr_mainprevstart_reg.main_pre_rd_v_start = 7;
	} else {
		ddr_mainprevstart_reg.main_pre_rd_v_start = (m_dispwin.srcy-10);
	}
	IoReg_Write32(MDOMAIN_DISP_DDR_MainPreVStart_reg, ddr_mainprevstart_reg.regValue);

	IoReg_Write32(MDOMAIN_DISP_DDR_MainAddrDropBits_reg,0x0);

	IoReg_Mask32(MDOMAIN_DISP_DDR_MainLineNum_reg, 0xfffff000, (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN) & 0x0FFF));

	if ((Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_MEM_ACT_HSTA) == 0) && (Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_MEM_ACT_VSTA) == 0) )
	{
		if((Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_CHANNEL) == _CHANNEL1) && (dvrif_memory_compression_get_enable(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) == TRUE)){
			comp_width = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID);
			if((comp_width % 32) != 0)
				comp_width = comp_width + (comp_width % 32);
			IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, (((comp_width * dvrif_memory_get_compression_bits(display) / 32) & 0x0fFF) << 8) | ((fifoLen - (burstLen>>1))>>1));
			TotalSize = memory_get_capture_line_size_compression(dvrif_memory_get_compression_bits(display));
		} else {
			IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, ((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) & 0x0fFF) << 8) | ((fifoLen - (burstLen>>1))>>1));
			TotalSize = memory_get_display_size(display);	// calculate the memory size of capture
		}
	} else {
		MemGetBlockShiftAddr(display,Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_MEM_ACT_HSTA),Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_MEM_ACT_VSTA), &MemShiftAddr, &droppixel);
		TotalSize = memory_get_disp_line_size1(display,Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_MEM_ACT_WID) + droppixel);	// calculate the memory size of capture
		IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, (( ((Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_MEM_ACT_WID) +droppixel) & 0x0fFF) << 8) | ((fifoLen - (burstLen>>1))>>1)));
		rtd_pr_scaler_memory_debug("drop pixel=%d shift addr=%x\n",droppixel,MemShiftAddr);

	}


	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division

	// remainder is not allowed to be zero
	if (Remainder == 0) {
		Remainder = burstLen;
		Quotient -= 1;
	}

	IoReg_Write32(MDOMAIN_DISP_DDR_MainRdNumLenRem_reg, (Quotient << 16) | (burstLen << 8) | Remainder);

	if((Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_CHANNEL) == _CHANNEL1) && (dvrif_memory_compression_get_enable(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) == TRUE))
		TotalSize = memory_get_capture_line_size_compression(dvrif_memory_get_compression_bits(display));
	else
		TotalSize = memory_get_capture_size(display, MEMCAPTYPE_LINE);		// calculate capture line size
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF))
		TotalSize=TotalSize>>1;

	LineStep = TotalSize;
	//rtd_pr_scaler_memory_debug("Line Step=0x%x\n",LineStep);
	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits

	IoReg_Write32(MDOMAIN_DISP_DDR_MainLineStep_reg, (LineStep + LineStep%2) << 3);//bit 3 need to be 0. rbus rule

	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()
		&& ((Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE)==SLR_3D_SIDE_BY_SIDE_HALF)||(Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE)==SLR_3D_TOP_AND_BOTTOM)||(Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)))
	{
		unsigned char ddr_bit;
		unsigned int temp;
		ddr_bit=(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_CHANNEL) ? 0 : (Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_10BIT) << 1)) | ( !Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_422CAP));

		if(Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING || Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE)==SLR_3D_TOP_AND_BOTTOM)
		{
			//[#0051720] MVC FP source (L-sync, double buffer control)
			if((Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING) && (Scaler_InputSrcGetMainChType() == _SRC_VO))
			{
				MemShiftAddr=((2*Scaler_DispGetInputInfoByDisp(display, (SLR_INPUT_INFO)SLR_DISP_3D_MEM_LEN_PRE)-2*Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_MEM_ACT_LEN))>>2)*LineStep;
			}
			else
			{
				MemShiftAddr=((Scaler_DispGetInputInfoByDisp(display, (SLR_INPUT_INFO)SLR_DISP_3D_MEM_LEN_PRE)-Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_MEM_ACT_LEN))>>2)*LineStep;
				if(get_3D_overscan_enable())
				{
					MemShiftAddr=(get_3D_overscan_input_vsta()>>1)*LineStep;
				}
			}
		}
		else
		{
			MemShiftAddr=((Scaler_DispGetInputInfoByDisp(display, (SLR_INPUT_INFO)SLR_DISP_3D_MEM_LEN_PRE)-Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN))>>1)*LineStep;
			if(get_3D_overscan_enable())
			{
				MemShiftAddr=(get_3D_overscan_input_vsta())*LineStep;
			}
		}

		//rtd_pr_scaler_memory_debug("[3d]shift addr v=%x\n",MemShiftAddr);
		temp=memory_get_line_size(((Scaler_DispGetInputInfoByDisp(display, (SLR_INPUT_INFO)SLR_DISP_3D_MEM_WID_PRE) - Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID))>>1), (ddr_bit & _BIT0), ((ddr_bit & _BIT1) >> 1));
		if(get_3D_overscan_enable())
		{
			if(Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING || Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE)==SLR_3D_TOP_AND_BOTTOM)
				temp=memory_get_line_size(get_3D_overscan_input_hsta(), (ddr_bit & _BIT0), ((ddr_bit & _BIT1) >> 1));
			else if(Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE)==SLR_3D_SIDE_BY_SIDE_HALF)
				temp=memory_get_line_size(get_3D_overscan_input_hsta()>>1, (ddr_bit & _BIT0), ((ddr_bit & _BIT1) >> 1));
		}

		temp=drvif_memory_get_data_align(temp, 2);
		MemShiftAddr=MemShiftAddr+temp;
		//rtd_pr_scaler_memory_debug("[3d]shift addr h=%x\n",MemShiftAddr);
		MemShiftAddr=MemShiftAddr<<3;
		//rtd_pr_scaler_memory_debug("[3d]shift addr after <<3 =%x\n",MemShiftAddr);
	}

	ucStartAddress = (drvif_memory_get_block_addr(MEMIDX_MAIN) + MemShiftAddr) & 0x7ffffff0;

	ucStartAddress2 = (drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + MemShiftAddr) & 0x7ffffff0;

	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) != SLR_3D_2D_ONLY))
	{
		MemDispStartAddr_3D[0]=ucStartAddress;
		MemDispStartAddr_3D[1]=ucStartAddress2;
	}

#ifdef CONFIG_ENABLE_3BUFFER_NEW_RULE
	if(Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ)>650)
	{
		ucStartAddress3 = (drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) + MemShiftAddr) & 0x7ffffff0;
		IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg, ucStartAddress3&0x7ffffff0);
	}
	else if(_ENABLE == Scaler_Get_Enable_3Buffer_NewRuleCondition())
	{
		if(_TRUE == Scaler_DispGetStatus(display, SLR_DISP_VFLIP3))
		{
			ucStartAddress3 = (drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) + MemShiftAddr) & 0x7ffffff0;
			IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg, ucStartAddress3&0x7ffffff0);
		}
	}
#else //#ifdef CONFIG_ENABLE_3BUFFER_NEW_RULE
#ifdef ENABLE_3_BUFFER_DELAY_MODE
	ucStartAddress3 = (drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) + MemShiftAddr) & 0x7ffffff0;
	IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg, ucStartAddress3&0x7ffffff0);
	MemDispStartAddr_3D[2]=ucStartAddress3;
#endif

	//M-3buf condition:
	// 1. [2D] M-capture frame rate > M-display frame rate
	// 2. [3D][SG] FRC in M-domain, M-display need repeat frame 2 times
	// 3. [3D][SG] SG 3D display timing FRC need enable triple buffer to avoid frame tear issue
	// 4. [3D][PR] PR to SG 3D display timing FRC need enable triple buffer to avoid frame tear issue
	if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ)>650)|| drvif_scaler3d_decide_3d_SG_data_FRC()
			|| ((drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() == TRUE) && drvif_scaelr3d_decide_is_3D_display_mode())
			|| (Get_PANEL_3D_PR_TO_SG_OUTPUT_FRC_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode())
			|| (modestate_get_pr3d_in_fp_3d_mode() && ((Scaler_InputSrcGetMainChType() == _SRC_VO)|| (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) >= 310)))
			||Get_vscFilmMode())
	{
		ucStartAddress3 = (drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) + MemShiftAddr) & 0x7ffffff0;
		IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg, ucStartAddress3&0x7ffffff0);
		MemDispStartAddr_3D[2]=ucStartAddress3;
	}
#endif //#ifdef CONFIG_ENABLE_3BUFFER_NEW_RULE
	IoReg_Write32(MDOMAIN_DISP_DDR_MainAddrDropBits_reg,droppixel);

	Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_DISPLAY_SIZE, memory_get_display_size(display));

	MemSetDisplayAddr(SLR_MAIN_DISPLAY, ucStartAddress, ucStartAddress2);
	if(TRUE) {	// use double buffer
		TotalSize = LineStep * Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_MEM_ACT_LEN);
		IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg, (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule
	}

	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()){
		memory_set_main_3D_setting_dispchange(LineStep);
	}// [SG] SG 3D in data FRC mode -- sequence frame 3D L-R output control
	else if(drvif_scaler3d_decide_3d_SG_data_FRC()|| (drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() && drvif_scaelr3d_decide_is_3D_display_mode())){
		memory_set_main_SG_3D_setting_dispchange(LineStep, ucStartAddress, ucStartAddress2, ucStartAddress3);
	}

	Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MAIN_ADDR, ucStartAddress);
	Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MAIN_SEC_ADDR, ucStartAddress2);
	Scaler_Disp3dSetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MAIN_THIRD_ADDR, ucStartAddress3);

	if (Get_PANEL_VFLIP_ENABLE())
	{
	if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_VFLIP3))
		mainctrltemp =  _BIT7 | (display ? _BIT1 : 0) | _BIT6   ;
	else
		mainctrltemp =  (display ? _BIT1 : 0) | _BIT6;
	}
	else
		mainctrltemp =  (display ? _BIT1 : 0) | _BIT6 ;

	mainctrltemp = mainctrltemp | _BIT25 | _BIT28;

	//M-3buf condition:
	// 1. [2D] M-capture frame rate > M-display frame rate
	// 2. [3D][SG] FRC in M-domain, M-display need repeat frame 2 times
	// 3. [3D][SG] SG 3D display free run need triple buffer to avoid frame tear
	// 4. [3D][PR] PR to SG 3D display timing FRC need enable triple buffer to avoid frame tear issue
	// 5. [3D][PR] frame packing video in PR 3D panel (TO BE FIXED)
	if((Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_V_FREQ) > 650)
			|| drvif_scaler3d_decide_3d_SG_data_FRC()
			|| ((drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() == TRUE) && drvif_scaelr3d_decide_is_3D_display_mode())
			|| (Get_PANEL_3D_PR_TO_SG_OUTPUT_FRC_ENABLE() && drvif_scaelr3d_decide_is_3D_display_mode())
			|| (modestate_get_pr3d_in_fp_3d_mode() && ((Scaler_InputSrcGetMainChType() == _SRC_VO)|| (Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_V_FREQ) >= 310))))
	{
		// Input source frame rate faster than display (main_frc_style = 1)
		// M-display IVS refer to front of VGIP signal (for HDMI FP 3D, IVS refer to the source v-sync)
		if((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ) > 650) && (Scaler_Get3DMode((SCALER_DISP_CHANNEL)display) == SLR_3DMODE_2D))
			mainctrltemp |= _BIT3;

		if(display == SLR_MAIN_DISPLAY)//frank@1007 solve VGA source M domain hang problem
			//set Mdomain display triple buffer
			mainctrltemp |=_BIT7;

		// [3D][PR]  PR output merge triple buffer enable
		if(Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE()){
			if((Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) == SLR_3D_TOP_AND_BOTTOM)||(Scaler_Disp3dGetInfo(display, SLR_DISP_3D_3DTYPE) == SLR_3D_FRAME_PACKING))
				mainctrltemp |=_BIT14;
		}
	}
#ifdef ENABLE_3_BUFFER_DELAY_MODE
	//if(drv_memory_get_game_mode() == _ENABLE){
	//	mainctrltemp = mainctrltemp & (~(_BIT6 | _BIT7));
	//}
	//else{
		mainctrltemp |=_BIT7;//set Mdomain display triple buffer
	//}
	//mainctrltemp |=(_BIT7|_BIT3);//set Mdomain display triple buffer
#endif


	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE())
		mainctrltemp=mainctrltemp|(IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg)&(_BIT5|_BIT13));// filed merge enable

	mainctrltemp |= (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_10BIT) ? _BIT16 : 0);
	if((display == SLR_MAIN_DISPLAY) && (dvrif_memory_compression_get_enable(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) == TRUE)){
		mainctrltemp |= _BIT17;
	}else{
		mainctrltemp |= Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display, SLR_DISP_422CAP) ? 0 : _BIT17;
	}
	mainctrltemp |= (_BIT19);
	mainctrltemp |= (drvif_scaler3d_decide_3d_SG_data_FRC()|| (drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain() && drvif_scaelr3d_decide_is_3D_display_mode())? _BIT12: 0); // 3D frame sequence out enable

	IoReg_Write32(/*display? MDOMAIN_DISP_DDR_SubCtrl_reg : */MDOMAIN_DISP_DDR_MainCtrl_reg,  mainctrltemp);
	IoReg_Write32(/*display? MDOMAIN_DISP_DDR_SubCtrl_reg : */MDOMAIN_DISP_Disp_main_enable_reg,  _BIT0);

}

extern StructSrcRect Scaler_SubDispWindowGet(void);
void memory_set_sub_displaywindow_change(unsigned char changetomain)
{
	SCALER_DISP_CHANNEL display = SLR_SUB_DISPLAY;
	unsigned int TotalSize;
	unsigned int Quotient;
	unsigned char Remainder;
	unsigned int fifoLen = _FIFOLENGTH2;
	unsigned int burstLen = _BURSTLENGTH2;
	unsigned int MemShiftAddr = 0;
	unsigned short droppixel = 0;
	mdomain_disp_ddr_subctrl_RBUS mdomain_disp_ddr_subctrl_Reg;
	mdomain_disp_ddr_subprevstart_RBUS mdomain_disp_ddr_subprevstart_Reg;
	StructSrcRect s_dispwin;

	s_dispwin = Scaler_SubDispWindowGet();


	mdomain_disp_ddr_subprevstart_Reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubPreVStart_reg);
#if 1 //K3L Q8401 frame tear issue
		if(s_dispwin.srcy <= 100) {
			mdomain_disp_ddr_subprevstart_Reg.sub_pre_rd_v_start = Get_DISP_DEN_STA_VPOS()-7;
		} else {
			mdomain_disp_ddr_subprevstart_Reg.sub_pre_rd_v_start = (s_dispwin.srcy+Get_DISP_DEN_STA_VPOS()-4);
		}
#else
		mdomain_disp_ddr_subprevstart_Reg.sub_pre_rd_v_start = 7;
#endif
	IoReg_Write32(MDOMAIN_DISP_DDR_SubPreVStart_reg, mdomain_disp_ddr_subprevstart_Reg.regValue);

	/*if ((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_HSTA) == 0) && (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_VSTA) == 0) ) {
		//frank@0323 change waterlevel/2 for AT,because water level nit is 128 bits
		IoReg_Mask32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg, 0xffc00000, ((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) & 0x3FFF) << 8) | (fifoLen - burstLen));
		if(changetomain)
			TotalSize = memory_get_display_size(SLR_MAIN_DISPLAY);
		else
			TotalSize = memory_get_display_size(display);	// calculate the memory size of capture
	} else */{
		if(changetomain)
			MemGetBlockShiftAddr(SLR_MAIN_DISPLAY, Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_HSTA),Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_VSTA), &MemShiftAddr, &droppixel);
		else
			MemGetBlockShiftAddr(display, Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_HSTA),Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_VSTA), &MemShiftAddr, &droppixel);
		TotalSize = memory_get_disp_line_size1(display, Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) + droppixel);	// calculate the memory size of capture
		IoReg_Mask32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg, 0xffc00000, (( ((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) +droppixel) & 0x3FFF) << 8) | ((fifoLen - (burstLen>>1))>>1)));
	}

	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division

	// remainder is not allowed to be zero
	if (Remainder == 0) {
		Remainder = burstLen;
		Quotient -= 1;
	}


	if((dvrif_memory_compression_get_enable(SLR_SUB_DISPLAY) == TRUE) && (dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) == COMPRESSION_FRAME_MODE))
	{
		IoReg_Mask32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, 0x00000000, (0xffff << 16) | (burstLen << 8) | burstLen);
	}
	else
	{
		IoReg_Mask32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, 0x00000000, (Quotient << 16) | (burstLen << 8) | Remainder);
	}

	if(changetomain)
		TotalSize = memory_get_capture_size(SLR_MAIN_DISPLAY, MEMCAPTYPE_LINE);
	else
		TotalSize = memory_get_capture_size(display, MEMCAPTYPE_LINE);
	/*TotalSize = Quotient * burstLen + Remainder; 	// count one line, 64bits unit
	TotalSize = drvif_memory_get_data_align(TotalSize, 4);	// times of 4*/

	IoReg_Write32(MDOMAIN_DISP_DDR_SubLineStep_reg,  (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule

	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits

	IoReg_Mask32(MDOMAIN_DISP_DDR_SubLineNum_reg, 0xfffff000, (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN) & 0x0FFF));

	if(changetomain)
		MemSetDisplayAddr(SLR_SUB_DISPLAY, (drvif_memory_get_block_addr(MEMIDX_MAIN) + MemShiftAddr) & 0x7ffffff0, (drvif_memory_get_block_addr(MEMIDX_MAIN_SEC) + MemShiftAddr) & 0x7ffffff0);
	else
		MemSetDisplayAddr(SLR_SUB_DISPLAY, (drvif_memory_get_block_addr(MEMIDX_SUB) + MemShiftAddr) & 0x7ffffff0, (drvif_memory_get_block_addr(MEMIDX_SUB_SEC) + MemShiftAddr) & 0x7ffffff0);

#ifdef ENABLE_3_BUFFER_DELAY_MODE
	if(changetomain)
		IoReg_Write32(MDOMAIN_DISP_DDR_Sub3rdAddr_reg, (drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) + MemShiftAddr) & 0x7ffffff0);
	else
		IoReg_Write32(MDOMAIN_DISP_DDR_Sub3rdAddr_reg, (drvif_memory_get_block_addr(MEMIDX_SUB_THIRD) + MemShiftAddr) & 0x7ffffff0);
#endif

	IoReg_Write32(MDOMAIN_DISP_DDR_SubAddrDropBits_reg,droppixel);
	mdomain_disp_ddr_subctrl_Reg.regValue = IoReg_Read32 (MDOMAIN_DISP_DDR_SubCtrl_reg);//K3LG-370:for sub frame tear issue

	/*IoReg_Mask32(MDOMAIN_DISP_DDR_SubBlkStep_reg, 0xff000000, 0);
	IoReg_WriteWord0(MDOMAIN_DISP_DDR_SubMPEn_reg,0);
	IoReg_WriteByte0(MDOMAIN_DISP_DDR_SubMPMode_reg, _BIT3);*/
	mdomain_disp_ddr_subctrl_Reg.sub_double_en = 1;
	if(changetomain)
		mdomain_disp_ddr_subctrl_Reg.sub_source_sel = 0;
	else
		mdomain_disp_ddr_subctrl_Reg.sub_source_sel = 1;
#ifdef ENABLE_3_BUFFER_DELAY_MODE
	mdomain_disp_ddr_subctrl_Reg.sub_v_flip_3buf_en = 1;//set Mdomain display triple buffer
#endif


	//if(!changetomain) //K3LG-370 marked
	//	subctrltemp |= (_BIT1);//K3LG-370 marked
	mdomain_disp_ddr_subctrl_Reg.sub_in_format = Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP) ? 0 : 1;
	mdomain_disp_ddr_subctrl_Reg.subcolorspace = 1;
	//K3LG-370:for sub frame tear issue _BIT25,_BIT28
	mdomain_disp_ddr_subctrl_Reg.sub_bufnum = 2;
	mdomain_disp_ddr_subctrl_Reg.sub_cap2disp_distance = 1;
	IoReg_Write32(MDOMAIN_DISP_DDR_SubCtrl_reg, mdomain_disp_ddr_subctrl_Reg.regValue);
	/*IoReg_Write32(MDOMAIN_DISP_Disp_sub_enable_reg, _BIT0);*/

	//drvif_memory_set_dbuffer_write();
}

#ifdef CONFIG_DUAL_CHANNEL // There are two channels
static void memory_set_sub_vflip_for_pip(unsigned char display)
{

	unsigned int TotalSize;
	unsigned int Quotient;
	unsigned char Remainder;
	unsigned int ucStartAddress;
	unsigned int ucStartAddress2;
	unsigned int LineStep;
	unsigned int fifoLen = _FIFOLENGTH2;
	unsigned int burstLen = _BURSTLENGTH2;
	unsigned int Align_LineStep;

	//change waterlevel/2 for AT,because water level nit is 128 bits
	IoReg_Mask32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg, 0xff800ff, ((Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_WID) & 0x07FF) << 8) | (fifoLen-burstLen));
	IoReg_Mask32(MDOMAIN_DISP_DDR_SubLineNum_reg, 0xfffff000, (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN) & 0x0FFF));


	ucStartAddress = drvif_memory_get_block_addr(MEMIDX_SUB);
	ucStartAddress2 = drvif_memory_get_block_addr(MEMIDX_SUB_SEC);

	TotalSize = memory_get_display_size(display);	// calculate the memory size of capture
	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division

	// remainder is not allowed to be zero
	if (Remainder == 0) {
		Remainder = burstLen;
		Quotient -= 1;
	}

	IoReg_Mask32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, 0x00000000, (Quotient << 16) | (burstLen << 8) | Remainder);
	TotalSize = Quotient * burstLen + Remainder; 	// count one line, 64bits unit
	TotalSize = drvif_memory_get_data_align(TotalSize, 4);	// times of 4
	LineStep = TotalSize;
	Align_LineStep = LineStep;
	Align_LineStep = (Align_LineStep % 2) ? (Align_LineStep + 1) : Align_LineStep;
    IoReg_Write32(MDOMAIN_DISP_DDR_SubLineStep_reg, ((~Align_LineStep) + 1) << 3);//bit 3 need to be 0. rbus rule

	ucStartAddress = memory_cal_vflip_block_addr(ucStartAddress, LineStep , Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN));
	ucStartAddress2 = memory_cal_vflip_block_addr(ucStartAddress2, LineStep , Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN));

	MemSetDisplayAddr(SLR_SUB_DISPLAY,ucStartAddress ,ucStartAddress2);

	//update limit mode boundary
	drv_memory_disp_limit_boundary_setting(SLR_SUB_DISPLAY);

	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits

	IoReg_Write32(MDOMAIN_DISP_DDR_SubBlkStep_reg,  0);

	IoReg_WriteWord0(MDOMAIN_DISP_DDR_SubMPEn_reg,0);
	IoReg_WriteByte0(MDOMAIN_DISP_DDR_SubMPMode_reg, _BIT3);
	IoReg_SetBits(MDOMAIN_DISP_DDR_SubCtrl_reg, _BIT6);
	drvif_memory_set_dbuffer_write();

}
#endif //CONFIG_DUAL_CHANNEL  // There are two channels
#endif

#ifdef CONFIG_ARM64
unsigned long FilmModeCachaddr = 0;
unsigned long FilmModePhyadddr = 0;
#else
unsigned int FilmModeCachaddr = 0;
unsigned int FilmModePhyadddr = 0;
#endif
extern bool rtk_hal_vsc_GetRGB444Mode(void);
extern unsigned char get_AVD_Input_Source(void);
/*============================================================================*/
/**
 * dvrif_memory_handler/dvrif_memory_handler_for_main/dvrif_memory_handler_for_sub
 * This function is used to handler memory (page 3).
 *
 * @param <info> { display-info struecture }
 * @return { none }
 *
 */
void dvrif_memory_handler_for_main(SCALER_DISP_CHANNEL display)
{
    unsigned int remove = 0;
    unsigned int multiple = 0;
    unsigned int inFrameRate = 0;
    unsigned int outFrameRate = 0;
    drvif_scaler_ddomain_switch_irq(_DISABLE);	// avoid to DSR to switch buffer when double bit not set

	//Disable M-disp when reset M-domain=>To Fixed DTV change issue @ Crixus 20141126
	drv_memory_Main_Disp_Enable(FALSE);

	if((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 1))	// yychao+ : use single buffer for JPEG source
		memory_set_capture(FALSE, MEMCAPTYPE_FRAME, display);	//  CSW 0961105 change single buffer to double to slove 1080i moving picture not smooth
	else
	{
		if(dvrif_memory_compression_get_enable(display) == TRUE)
        {
			//update M-domain PQC frame mode @Crixus 20170626

			if(dvrif_memory_get_compression_mode(display) == COMPRESSION_LINE_MODE)
			{
				rtd_pr_scaler_memory_debug("comp line mode\n");
				memory_set_capture(TRUE, MEMCAPTYPE_LINE, display);
			}
			else
			{
				rtd_pr_scaler_memory_debug("comp frame mode\n");
				memory_set_capture(TRUE, MEMCAPTYPE_FRAME, display);
			}
		}
		else{
			rtd_pr_scaler_memory_debug("no-comp frame mode\n");
			memory_set_capture(TRUE, MEMCAPTYPE_FRAME, display);	//  CSW 0961105 change single buffer to double to slove 1080i moving picture not smooth
			/*if(dvrif_memory_get_compression_mode(display) == COMPRESSION_LINE_MODE)
				memory_set_capture(TRUE, MEMCAPTYPE_LINE, display);
			else
				memory_set_capture(TRUE, MEMCAPTYPE_FRAME, display);//xiong_bypass due to cmp disable, only have frame buffer mode */
		}

	}
	if((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 1))	// yychao+ : use single buffer for JPEG source
		memory_set_main(FALSE);		//  CSW 0961105 change single buffer to double to slove 1080i moving picture not smooth
	else
	{
		if (Get_PANEL_VFLIP_ENABLE())
		{
			//VDEC source do v-flip on VO because data fs, other do on M-domain @Crixus 20171226
			if(Scaler_InputSrcGetMainChType() == _SRC_VO){
				memory_set_main(TRUE);
			}
			else{
				memory_set_vflip_main(TRUE);  //vflip 1st,2nd,3rd display
			}
		}
		else
		{
			//rtd_pr_scaler_memory_debug("c.....\n");
			memory_set_main(TRUE);		//  CSW 0961105 change single buffer to double to slove 1080i moving picture not smooth
		}
	}

	if(get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1)
	{//run sub mdomain for two pixel mode
#ifdef CONFIG_DUAL_CHANNEL
		if(dvrif_memory_compression_get_enable(SLR_SUB_DISPLAY) == TRUE)
		{
			if(dvrif_memory_get_compression_mode(display) == COMPRESSION_LINE_MODE)
			{
				rtd_pr_scaler_memory_debug("comp line mode\n");
				memory_set_capture(TRUE, MEMCAPTYPE_LINE, SLR_SUB_DISPLAY);
			}
			else
			{
				rtd_pr_scaler_memory_debug("comp frame mode\n");
				memory_set_capture(TRUE, MEMCAPTYPE_FRAME, SLR_SUB_DISPLAY);
			}
		}
		else{
			rtd_pr_scaler_memory_debug("no-comp frame mode\n");
			memory_set_capture(TRUE, MEMCAPTYPE_FRAME, SLR_SUB_DISPLAY);	//  CSW 0961105 change single buffer to double to slove 1080i moving picture not smooth
		}
		memory_set_sub_for_pip(SLR_SUB_DISPLAY);
		//limit boundary enable @Crixus 20180320
		memory_set_input_format(SLR_SUB_DISPLAY, TRUE);
		dvrif_memory_comp_setting_sub(dvrif_memory_compression_get_enable(SLR_SUB_DISPLAY) ? TRUE : FALSE, Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_CAP_WID), Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_CAP_LEN), dvrif_memory_get_compression_bits(SLR_SUB_DISPLAY));
		drv_memory_limit_boundary_enable(SLR_SUB_DISPLAY, drv_memory_get_limit_boundary_mode(SLR_SUB_DISPLAY));
#endif
    }

	if((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 1))	// yychao+ : use single buffer for JPEG source
		memory_set_input_format(display, FALSE);
	else
		memory_set_input_format(display, TRUE);

	//compression & de-compression setting
	if(dvrif_memory_compression_get_enable(display) == TRUE)
    {
		dvrif_memory_comp_setting(TRUE, Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_CAP_WID), Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_CAP_LEN), dvrif_memory_get_compression_bits(display));
#ifdef ENABLE_VR360_DATA_FS_FLOW
        if(scaler_vsc_get_vr360_block_mdomain_doublebuffer() == FALSE)
#endif
			IoReg_SetBits(MDOMAIN_DISP_DDR_MainSubCtrl_reg, _BIT0);
	}
	else
    {
		dvrif_memory_comp_setting(FALSE, Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_CAP_WID), Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_CAP_LEN), dvrif_memory_get_compression_bits(display));
#ifdef ENABLE_VR360_DATA_FS_FLOW
        if(scaler_vsc_get_vr360_block_mdomain_doublebuffer() == FALSE)
#endif
			IoReg_SetBits(MDOMAIN_DISP_DDR_MainSubCtrl_reg, _BIT0);
	}

    //update multi-buffer setting @Crixus 20160913
    if(drv_memory_Get_multibuffer_flag())
    {
        rtd_pr_scaler_memory_emerg("[crixus]drv_memory_Get_multibuffer_number() = %x\n", drv_memory_Get_multibuffer_number());
        drv_memory_capture_multibuffer_control(drv_memory_Get_multibuffer_number(), drv_memory_Get_multibuffer_number() - 2);
        drv_memory_display_multibuffer_control(drv_memory_Get_multibuffer_number(), drv_memory_Get_multibuffer_number() - 2);
    }

	//Eric@20181016 Vdec Direct Low Latency mode flow
	if(drv_memory_get_vdec_direct_low_latency_mode())
    {
#ifndef CONFIG_MEMC_BYPASS
		drv_memory_SingleBuffer_GameMode(_ENABLE);
#else
        remove = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_FRAC_FS_REMOVE_RATIO);
        multiple = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_FRAC_FS_MULTIPLE_RATIO);
        inFrameRate = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ);
        outFrameRate = inFrameRate * multiple / remove;

        if(inFrameRate == outFrameRate)
        {
            drv_memory_SingleBuffer_GameMode(_ENABLE);
        }
#endif
    }
#ifdef CONFIG_FORCE_RUN_I3DDMA
	if(((Get_DisplayMode_Src(display) == VSC_INPUTSRC_ADC)
		||((Get_DisplayMode_Src(display) == VSC_INPUTSRC_AVD)  && (get_AVD_Input_Source() != _SRC_TV))
		||((get_HDMI_HDR_mode() != HDR_DOLBY_HDMI) && (Get_DisplayMode_Src(display) == VSC_INPUTSRC_HDMI)))//dolby case set false
		&&(!drvif_i3ddma_triplebuf_flag()))
	{
		drv_I3ddmaVodma_GameMode_iv2pv_delay(TRUE);
	}
    else
    {
		drv_I3ddmaVodma_GameMode_iv2pv_delay(FALSE);
	}

#endif

}

void dvrif_memory_handler_for_sub(SCALER_DISP_CHANNEL display)
{
    if(dvrif_memory_compression_get_enable(display) == TRUE)
    {
        //update M-domain PQC frame mode @Crixus 20170626
        if(dvrif_memory_get_compression_mode(display) == COMPRESSION_LINE_MODE)
        {
            rtd_pr_scaler_memory_debug("comp line mode\n");
            memory_set_capture(TRUE, MEMCAPTYPE_LINE, display);
        }
        else
        {
            rtd_pr_scaler_memory_debug("comp frame mode\n");
            memory_set_capture(TRUE, MEMCAPTYPE_FRAME, display);
        }
    }
    else{
        rtd_pr_scaler_memory_debug("no-comp frame mode\n");
        memory_set_capture(TRUE, MEMCAPTYPE_FRAME, display);    //  CSW 0961105 change single buffer to double to slove 1080i moving picture not smooth
    }

    //memory_set_capture(TRUE, MEMCAPTYPE_FRAME, display); // memory, capture part for other cases
 #ifndef BUILD_QUICK_SHOW
    if (Get_PANEL_VFLIP_ENABLE())
    {
        if (Scaler_InputSrcGetMainChType() == _SRC_VO)
            memory_set_sub_for_pip(display);
        else
            memory_set_sub_vflip_for_pip(display);
    }
    else
    {
        memory_set_sub_for_pip(display);    // memory, sub display part
    }
#endif
    #if 0
    else if (Scaler_PipGetInfo(SLR_PIP_TYPE) <= SUBTYPE_MP1L7B)
    {// Sub MP
        memory_set_capture_for_mp(); // memory, capture part for Multi Picture
        memory_set_sub_for_mp();
        /*ScalerTimer_ActiveTimerEvent(SLRTIMER_PIPMP, 1000, (SLRTIMER_CALLBACK)PipMp_RotateMp);*/
        Scaler_PipSetSubInfo(SLR_PIP_SUB_MPENABLE,1);
    }
    else
    {
        if(dvrif_memory_compression_get_enable(display) == TRUE)
        {
            //update M-domain PQC frame mode @Crixus 20170626
            if(dvrif_memory_get_compression_mode(display) == COMPRESSION_LINE_MODE)
            {
                rtd_pr_scaler_memory_debug("comp line mode\n");
                memory_set_capture(TRUE, MEMCAPTYPE_LINE, display);
            }
            else
            {
                rtd_pr_scaler_memory_debug("comp frame mode\n");
                memory_set_capture(TRUE, MEMCAPTYPE_FRAME, display);
            }
        }
        else{
            rtd_pr_scaler_memory_debug("no-comp frame mode\n");
            memory_set_capture(TRUE, MEMCAPTYPE_FRAME, display);    //  CSW 0961105 change single buffer to double to slove 1080i moving picture not smooth
        }

        memory_set_capture(TRUE, MEMCAPTYPE_FRAME, display); // memory, capture part for other cases

        memory_set_sub_for_pip(display);    // memory, sub display part
    }
    #endif

    if((Scaler_InputSrcGetMainChType() == _SRC_VO) && (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 1))	// yychao+ : use single buffer for JPEG source
    	memory_set_input_format(display, FALSE);
    else
    	memory_set_input_format(display, TRUE);

    //enable or disable compress mode for sub path.
    dvrif_memory_comp_setting_sub(dvrif_memory_compression_get_enable(display) ? TRUE : FALSE, Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_CAP_WID), Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_CAP_LEN), dvrif_memory_get_compression_bits(display));

}

void dvrif_memory_handler(void)
{
	SCALER_DISP_CHANNEL display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);

    if(display == SLR_MAIN_DISPLAY)
        dvrif_memory_handler_for_main(SLR_MAIN_DISPLAY);
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
    else
        dvrif_memory_handler_for_sub(SLR_SUB_DISPLAY);
#endif
 #ifndef BUILD_QUICK_SHOW
    if(TRUE == Get_vscFilmMode())
    {
        //because film mode need 4 buffers
        FilmModeCachaddr = (unsigned long)dvr_malloc(drvif_memory_get_block_size(MEMIDX_MAIN));
        //FilmModePhyadddr = (unsigned int)virt_to_phys((void*)FilmModeCachaddr);
        #ifdef CONFIG_ARM64
        FilmModePhyadddr = (unsigned long)dvr_to_phys((void*)FilmModeCachaddr);
        #else
        FilmModePhyadddr = (unsigned int)dvr_to_phys((void*)FilmModeCachaddr);
        #endif
        filmmode_videofw_config(TRUE,FilmModePhyadddr);
    }
#endif
    //limit boundary enable @Crixus 20180320
    drv_memory_limit_boundary_enable(display, drv_memory_get_limit_boundary_mode(display));
}

void mdomain_handler_onlycap_i2run(void)
{
	mdomain_cap_capture2_byte_swap_RBUS capture_byte_swap_reg;
	memory_set_capture(TRUE, MEMCAPTYPE_LINE, Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)? SLR_SUB_DISPLAY : SLR_MAIN_DISPLAY);
	capture_byte_swap_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Capture2_byte_swap_reg);
	capture_byte_swap_reg.in2_1byte_swap = 0;
	capture_byte_swap_reg.in2_2byte_swap = 0;
	capture_byte_swap_reg.in2_4byte_swap = 0;
	capture_byte_swap_reg.in2_8byte_swap = 1;
	IoReg_Write32(MDOMAIN_CAP_Capture2_byte_swap_reg, capture_byte_swap_reg.regValue);
}

void mdomain_handler_onlycap(void)
{
	memory_set_capture(TRUE, MEMCAPTYPE_FRAME, Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)? SLR_SUB_DISPLAY : SLR_MAIN_DISPLAY);
}

/*============================================================================*/
/**
 * drvif_memory_get_data_align
 * This function is used to do data alignment.
 *
 * @param <Value> { data needs alignment }
 * @param <unit> { unit of alignment }
 * @return { none }
 *
 */
unsigned int drvif_memory_get_data_align(unsigned int Value, unsigned int unit)
{
	unsigned int AlignVal;
#define N_BYTES 96

	AlignVal = Value;

#ifndef UT_flag
    if (is_power_of_2(unit)) {
        unit -= 1;
        if (Value & unit)
            AlignVal = (Value & (~unit)) + unit + 1;
    } else {
        if ((AlignVal % unit) != 0)
            AlignVal = AlignVal + (unit - (AlignVal % unit));
#ifndef CONFIG_BW_96B_ALIGNED
        //if (unit == N_BYTES)
        //    AlignVal = Value; //output = input
#endif
    }
#endif
	return AlignVal;
}




// IP bit number control (saving bandwidth)
unsigned int drv_memory_get_ip_Btr_mode(void)
{
	return bIpBtrMode;
}


unsigned int drv_memory_get_ip_DMA420_mode(void)
{
	return bIpDma420Mode;
}


void drv_memory_set_ip_Btr_mode(unsigned int mode)
{
	bIpBtrMode = mode;
	return;
}


void drv_memory_set_ip_DMA420_mode(unsigned int mode)
{
	bIpDma420Mode = mode;
	return;
}

#ifndef Mag2_New_Flow
// DI bit number control (saving bandwidth)
static void drv_memory_DMA420_config(unsigned char *bitNum)
{
	UINT32 width, lineSize_odd, lineSize_even;

	if(!bitNum){
		rtd_pr_scaler_memory_debug("ERR: NULL PTR@%s\n", __FUNCTION__);
		return;
	}

	// DI DMA 420 enable (saving bandwidth)
	if(drv_memory_get_ip_DMA420_mode() == SLR_DMA_420_mode){
		width = Scaler_DispGetInputInfo(SLR_INPUT_DI_WID);
		lineSize_even = (width * (*bitNum - ((fw_scalerip_get_DI_chroma_10bits())?10:8)));
		if((lineSize_even%64)==0)
			lineSize_even /=64;
		else
			lineSize_even = (lineSize_even+64)/64;
		lineSize_odd = (width * *bitNum);
		if((lineSize_odd%64)==0)
			lineSize_odd /=64;
		else
			lineSize_odd = (lineSize_odd+64)/64;

		rtd_pr_scaler_memory_debug("[DI] DMA 420 Enable, bit/c-bit/line wid/even/odd[%d/%d/%d/%d/%d]\n", *bitNum, fw_scalerip_get_DI_chroma_10bits(),width, lineSize_even, lineSize_odd);
		IoReg_Mask32(DI_IM_DI_DMA_VADDR,
			~(DI_DMA_420_EN_MASK| DI_DMA_LINESIZE_EVEN_MASK| DI_DMA_LINESIZE_ODD_MASK),
			_BIT30| (lineSize_even << DI_DMA_LINESIZE_EVEN_START_BIT)| lineSize_odd);
		// update bit number
		//*bitNum -= 4;
	}
	else
		IoReg_ClearBits(DI_IM_DI_DMA_VADDR, _BIT30);

	return;
}
#endif
#if 0
static void drv_memory_set_ip_compress_bit(unsigned char bit_num)
{
#if 0	//Elsie 20131206: [FIXME] all of the registers below do not exist.
	dicmp_cmp_resolution_RBUS dicmp_cmp_resolution_reg;
	dicmp_comp_setting_RBUS dicmp_comp_setting_reg;
	dicmp_decmp_resolution_RBUS dicmp_decmp_resolution_reg;
	dicmp_decomp_setting_RBUS dicmp_decomp_setting_reg;
	dicmp_cmp_resolution_reg.regValue = 0;
	dicmp_cmp_resolution_reg.cmp_enable = 1;
	dicmp_cmp_resolution_reg.cmp_width = drvif_memory_get_data_align(Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID), 32)/32;
	dicmp_cmp_resolution_reg.cmp_height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
	dicmp_decmp_resolution_reg.regValue = 0;
	dicmp_decmp_resolution_reg.decmp_width = dicmp_cmp_resolution_reg.cmp_width;
	dicmp_decmp_resolution_reg.decmp_height = dicmp_cmp_resolution_reg.cmp_height;
	dicmp_comp_setting_reg.regValue = IoReg_Read32(DI_DICMP_COMP_SETTING_VADDR);
	dicmp_comp_setting_reg.cmp_rgb_yc_sel = 1;// Ycbcr
	dicmp_comp_setting_reg.cmp_channel_bits = !(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_10BIT));
	dicmp_comp_setting_reg.short_term_bits = bit_num;
	dicmp_comp_setting_reg.frame_bits = bit_num;
	dicmp_comp_setting_reg.max_pixel_bits = bit_num;
	dicmp_comp_setting_reg.cmp_a_ch_mark = 1; // a channel is not used
	dicmp_decomp_setting_reg.regValue = IoReg_Read32(DI_DICMP_DECOMP_SETTING_VADDR);
	dicmp_decomp_setting_reg.decmp_frame_bits = dicmp_comp_setting_reg.frame_bits;
	dicmp_decomp_setting_reg.decmp_channel_bits = dicmp_comp_setting_reg.cmp_channel_bits;
	dicmp_decomp_setting_reg.decmp_a_ch_mark = 1; // a channel is not used
	dicmp_decomp_setting_reg.decmp_rgb_yc_sel = 1; // Ycbcr
	dicmp_decomp_setting_reg.decmp_fl_mode = 1; //frame mode
	IoReg_Write32(DI_DICMP_COMP_SETTING_VADDR, dicmp_comp_setting_reg.regValue);
	IoReg_Write32(DI_DICMP_DECOMP_SETTING_VADDR, dicmp_decomp_setting_reg.regValue);
	IoReg_Write32(DI_DICMP_DECMP_RESOLUTION_VADDR, dicmp_decmp_resolution_reg.regValue);
	IoReg_Write32(DI_DICMP_CMP_RESOLUTION_VADDR, dicmp_cmp_resolution_reg.regValue);
#endif
}
#endif

/*============================================================================*/
/**
 * drv_memory_disable_di_write_req
 * Set DI DMA multi-request num
 *
 * @param <disableDiWr> {1: disable DI request, 0: enable DI request}
 * @return { none }
 * @note
 * disable DI request when 3DDI state change
 *
 */
void drv_memory_disable_di_write_req(unsigned char disableDiWr)
{
	di_di_dma_multi_req_num_RBUS di_dma_multi_req_num_reg;

	di_dma_multi_req_num_reg.regValue = IoReg_Read32(DI_DI_DMA_Multi_Req_Num_reg);
	di_dma_multi_req_num_reg.rdma_req_num = 3;//(enable? 3: 0);
	di_dma_multi_req_num_reg.wdma_req_num = (disableDiWr? 0: 2);
	IoReg_Write32(DI_DI_DMA_Multi_Req_Num_reg, di_dma_multi_req_num_reg.regValue);

	return;
}

/*============================================================================*/
/**
 * drv_memory_set_ip_fifo
 * Set IP (video process) memory FIFO registers according to its FIFO-type
 *
 * @param <info> { display-info struecture }
 * @param <bits> { 10bit => 1, 8bit => 0}
 * @param <access> { 5A => 1, 3A => 0, rtnr => 2}
 * @return { none }
 * @note
 * Field size(32 Bits) = [20 * h_size * v_size] / 64 * 2  for 3A
 * Field size(32 Bits) = [16 * h_size * v_size] / 64 * 2  for 5A
 *
 */
//#define ENABLE_DI_COMPRESS
void drv_memory_set_ip_fifo(unsigned char access)
{

#ifdef Mer2_MEMORY_ALLOC
	drv_memory_set_ip_fifo_Mer2(access);
	return;
#else
	typedef struct _MEMSET {
		unsigned int number;
		unsigned int length;
		unsigned int remain;
		unsigned int water_level;
	} MEMSET;
	//di_im_di_btr_control_RBUS BTR_Control_Reg;
	di_ddr_rdatdma_wtlennum_RBUS ddr_rdatdma_wtlennum;
	di_ddr_rinfdma_wtlennum_RBUS ddr_rinfdma_wtlennum;
	di_ddr_wdatdma_wtlennum_RBUS ddr_wdatdma_wtlennum;
	di_ddr_winfdma_wtlennum_RBUS ddr_winfdma_wtlennum;
	di_ddr_datdma_rm_RBUS ddr_datdma_rm;
	di_ddr_infdma_rm_RBUS ddr_infdma_rm;
	//di_di_dma_multi_req_num_RBUS di_dma_multi_req_num_reg;
	//ddr_vrfifo1wtlvllennum_RBUS ddr_vrfifowtlvllennum;
	//ddr_vwfifo1wtlvllennum_RBUS ddr_vwfifowtlvllennum;
	//ddr_vfifo1rdwrremainder_RBUS ddr_vfifordwrremainder;

	unsigned int	DI_Width = 0, DI_Height = 0, size_per_frame = 0;
	unsigned char	data_bitNum = 0, info_bitNum = 0, i = 0, DIAllocateBlock;
	unsigned int	data_value, info_value=0, MemStartAddr;
	MEMSET			data_r, data_w, info_r, info_w;

	UINT32 lineSize_odd_128, lineSize_even_128;
	unsigned char DI_SMD_enable = 0;

	unsigned char source=255;
	unsigned char newRTNR_onoff = 0;
	unsigned char m_rtnrcompress = 0;
	unsigned char DI_BTR_enable = 0;
	unsigned char DI_BTR_type = 0;
	unsigned int buffer_alloc = 0;
	unsigned int data_size = 0;
	unsigned int info_size = 0;

//fixme:vip
#if 1
	extern Scaler_DI_Coef_t Scaler_DI_Coef_table[VIP_QUALITY_SOURCE_NUM];
	source = fwif_vip_source_check(3, 0);
	if(source >= VIP_QUALITY_SOURCE_NUM)
	{
		rtd_pr_scaler_memory_err("\r\n#####drv_memory_set_ip_fifo source nunber %d error####\r\n", source);
		source = VIP_QUALITY_SOURCE_NUM-1;
	}
	//frank@1018 add below code to solve scan tool warning
	newRTNR_onoff = (unsigned char)Scaler_DI_Coef_table[source].newRTNR_style;
	//DI_SMD_enable = Scaler_getDI_SMD_en(source);
	DI_SMD_enable = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE);
		DI_BTR_enable = drv_memory_get_ip_Btr_mode();

	//BTR_Control_Reg.regValue = rtd_inl(DI_IM_DI_BTR_CONTROL_VADDR);
	//DI_BTR_type = BTR_Control_Reg.btr_solutiontype;
	DI_BTR_type = (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) < 960)?0:1;
#endif

		DI_Width = Scaler_DispGetInputInfo(SLR_INPUT_DI_WID);
		DI_Height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);

	rtd_pr_scaler_memory_debug("\n DI Memory set FIFO !");
	rtd_pr_scaler_memory_debug("\n Width:%d, Height:%d, DMA mode=%d", DI_Width, DI_Height, drv_memory_get_ip_DMA420_mode()?420:422);
	rtd_pr_scaler_memory_debug("\n Data status Y=%d C=%d", fw_scalerip_get_DI_chroma_10bits()?10:8, fw_scalerip_get_DI_chroma_10bits()?10:8);
	rtd_pr_scaler_memory_debug("\n Info status BTR=%d(type:%d) SMD=%d RTNR style=%d", DI_BTR_enable, DI_BTR_type, DI_SMD_enable, newRTNR_onoff);



	rtd_pr_scaler_memory_emerg("\n DI Memory set FIFO !");
	rtd_pr_scaler_memory_emerg("\n Width:%d, Height:%d, DMA mode=%d", DI_Width, DI_Height, drv_memory_get_ip_DMA420_mode()?420:422);
	rtd_pr_scaler_memory_emerg("\n Data status Y=%d C=%d", fw_scalerip_get_DI_chroma_10bits()?10:8, fw_scalerip_get_DI_chroma_10bits()?10:8);
	rtd_pr_scaler_memory_emerg("\n Info status BTR=%d(type:%d) SMD=%d RTNR style=%d", DI_BTR_enable, DI_BTR_type, DI_SMD_enable, newRTNR_onoff);





#ifdef Mag2_New_Flow
	data_bitNum = 16; // for YUV subsample 422 ( now only using 2 typr : 422 & 420	)
	info_bitNum = 0;

	// assign data bits
	if (fw_scalerip_get_DI_chroma_10bits())
		data_bitNum += 2;
	if (fw_scalerip_get_DI_chroma_10bits())
		data_bitNum += 2;

#ifdef ENABLE_DI_COMPRESS
	if ((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) > 2048)
		&& (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > 1200)) {	// 4k2k
		drv_memory_set_ip_compress_bit(12);
		m_rtnrcompress = 1;
		data_bitNum = 12;
	}
#endif

	/*
		LearnRPG comment DI DMA how to use?
		from magellan/sirius/mac3
		FIFO length is 64bits, SD need 2(128bits), HD need 4(SD double), progressive need 6
		about READ(from DDR to FIFO) the best performance is using water_level+length = FIFO length
		about WRITE(from FIFO to DDR) must just MATCH, otherwise will have dummy data in FIFO
	*/
	// assign info bits
	if (access == SLR_DI_5A_mode) {
		rtd_pr_scaler_memory_debug("\n 5A mode \n");
		rtd_pr_scaler_memory_emerg("\n 5A mode \n");

		info_bitNum += (DI_BTR_enable?(DI_BTR_type?4:8):0); // type A:8 bits, type B:4 bits

		DIAllocateBlock = 4;

		data_r.water_level = 68;
		data_r.length = 60;

		data_w.water_level = 60;
		data_w.length = 60;

		info_r.water_level = 32;
		info_r.length = 32;

		info_w.water_level = 32;
		info_w.length = 32;
	} else if (access == SLR_DI_3A_mode) {
		rtd_pr_scaler_memory_debug("\n 3A mode \n");
		rtd_pr_scaler_memory_emerg("\n 3A mode \n");

		info_bitNum += (DI_BTR_enable?4:0); // type A:8 bits, type B:4 bits, 3A only type B
		info_bitNum += 4; // frame motion in 3A default need 4 bits

		DIAllocateBlock = 2;

		data_r.water_level = 196;
		data_r.length = 60;

		data_w.water_level = 60;
		data_w.length = 60;

		info_r.water_level = 32;
		info_r.length = 32;

		info_w.water_level = 32;
		info_w.length = 32;
	} else if (access == SLR_DI_progress_mode) {
		rtd_pr_scaler_memory_debug("\n progress mode \n");

		rtd_pr_scaler_memory_emerg("\n progress mode \n");
		DIAllocateBlock = 2;

		//Waterlevel = 0x180 - length at PRTNR 2A case, but this value over 8bit(0xff), so give 0xff
		data_r.water_level = 0xff;
		data_r.length = 60;

		if (fwif_vip_increase_DI_water_lv())		//20150813 roger, change dat_len_r for 4k2k60p
			data_r.length = 96;

		data_w.water_level = 60;
		data_w.length = 60;

		info_r.water_level = 32;
		info_r.length = 32;

		info_w.water_level = 32;
		info_w.length = 32;
	} else {
		rtd_pr_scaler_memory_debug("\n Error accept parameter in memory fifo \n");
		return;
	}

	if (DI_SMD_enable)
		info_bitNum += 6; //4; //merlin2 need more 2bit->4+2=6

	// if k temporal function is enabled, new rtnr need 2 bits, otherwise still 0, please refer scalerColor.cpp:fwif_color_set_RTNR_style_adjust()
	if (newRTNR_onoff == 3 || newRTNR_onoff == 4)
		info_bitNum += 2;

	rtd_pr_scaler_memory_debug("\n Data BitNum=%d, Info BitNum=%d", data_bitNum, info_bitNum);
	rtd_pr_scaler_memory_emerg("\n Data BitNum=%d, Info BitNum=%d", data_bitNum, info_bitNum);

	// Calculate Data number/length/remain
	if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_422_mode) { // 422 mode
		size_per_frame = DI_Width * DI_Height * data_bitNum;
		data_r.number = (int)(size_per_frame/128/data_r.length);
		data_r.remain = (size_per_frame - data_r.number*128*data_r.length + 127)/128;
		if (data_r.remain == 0 && data_r.number != 0) {
			data_r.number -= 1;
			data_r.remain = data_r.length;
		}

		data_w.number = (int)(size_per_frame/128/data_w.length);
		data_w.remain = (size_per_frame - data_w.number*128*data_w.length + 127)/128;
		if (data_w.remain == 0 && data_w.number != 0) {
			data_w.number -= 1;
			data_w.remain = data_w.length;
		}

		IoReg_ClearBits(DI_IM_DI_DMA_reg, _BIT31);
		IoReg_ClearBits(DI_IM_DI_DMA_reg, _BIT30);
	}
	else if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_420_mode) { // 420 mode
		lineSize_even_128	= DI_Width * (data_bitNum - (fw_scalerip_get_DI_chroma_10bits()?10:8)); // only Y componet
		lineSize_odd_128	= DI_Width * data_bitNum; // Y and C componet

		// linesize unit 64
		if (lineSize_even_128%64 == 0)
			lineSize_even_128 /= 64;
		else
			lineSize_even_128 = (lineSize_even_128+64)/64;
		if (lineSize_odd_128%64 == 0)
			lineSize_odd_128 /= 64;
		else
			lineSize_odd_128 = (lineSize_odd_128+64)/64;

		// frame unit 128, so linesize must divide by 2
		//if (DI_Height%2 == 0)
		//	size_per_frame = ((lineSize_even_128 + lineSize_odd_128+1)/2)*(DI_Height/2);
		//else
			size_per_frame = ((lineSize_even_128+1)/2)*(DI_Height/2) + ((lineSize_odd_128+1)/2)*((DI_Height+1)/2);

		data_r.number = (int)(size_per_frame/data_r.length);
		data_r.remain = size_per_frame%data_r.length;
		if (data_r.remain == 0 && data_r.number != 0) {
			data_r.number -= 1;
			data_r.remain = data_r.length;
		}

		data_w.number = (int)(size_per_frame/data_w.length);
		data_w.remain = size_per_frame%data_w.length;
		if (data_w.remain == 0 && data_w.number != 0) {
			data_w.number -= 1;
			data_w.remain = data_w.length;
		}

		IoReg_Mask32(DI_IM_DI_DMA_reg,
			~(DI_DMA_420_EN_MASK| DI_DMA_LINESIZE_EVEN_MASK| DI_DMA_LINESIZE_ODD_MASK),
			_BIT30 | (lineSize_even_128 << DI_DMA_LINESIZE_EVEN_START_BIT) | lineSize_odd_128);

		rtd_pr_scaler_memory_debug("\n 420 mode: linesize_even=%d, linesize_odd=%d", lineSize_even_128, lineSize_odd_128);
	}
	else { // 400 mode
		//size_per_frame = DI_Width * DI_Height * data_bitNum;
		size_per_frame = DI_Width * DI_Height * (data_bitNum - (fw_scalerip_get_DI_chroma_10bits()?10:8));
		data_r.number = (int)(size_per_frame/128/data_r.length);
		data_r.remain = (size_per_frame - data_r.number*128*data_r.length + 127)/128;
		if (data_r.remain == 0 && data_r.number != 0) {
			data_r.number -= 1;
			data_r.remain = data_r.length;
		}

		data_w.number = (int)(size_per_frame/128/data_w.length);
		data_w.remain = (size_per_frame - data_w.number*128*data_w.length + 127)/128;
		if (data_w.remain == 0 && data_w.number != 0) {
			data_w.number -= 1;
			data_w.remain = data_w.length;
		}
		IoReg_SetBits(DI_IM_DI_DMA_reg, _BIT31);
		IoReg_ClearBits(DI_IM_DI_DMA_reg, _BIT30);
	}
	IoReg_SetBits(DI_db_reg_ctl_reg, _BIT0);

	// Calculate Info number/length/remain
	size_per_frame = DI_Width * DI_Height * info_bitNum;
	if (DI_SMD_enable && access == SLR_DI_5A_mode) // Info image height must +1 for DMA setting when SMD enable in 5A case
		size_per_frame += DI_Width * 4;

	info_r.number = (int)(size_per_frame/128/info_r.length);
	info_r.remain = (size_per_frame - info_r.number*128*info_r.length + 127)/128;
	if (info_r.remain == 0 && info_r.number != 0) {
		info_r.number -= 1;
		info_r.remain = info_r.length;
	}

	info_w.number = (int)(size_per_frame/128/info_w.length);
	info_w.remain = (size_per_frame - info_w.number*128*info_w.length + 127)/128;
	if (info_w.remain == 0 && info_w.number != 0) {
		info_w.number -= 1;
		info_w.remain = info_w.length;
	}

	rtd_pr_scaler_memory_debug("\n Data number = %d, length = %d, remain = %d, water_level = %d in setting DI Read FIFO", data_r.number, data_r.length, data_r.remain, data_r.water_level);
	rtd_pr_scaler_memory_debug("\n Data number = %d, length = %d, remain = %d, water_level = %d in setting DI Write FIFO", data_w.number, data_w.length, data_w.remain, data_w.water_level);
	rtd_pr_scaler_memory_debug("\n Info number = %d, length = %d, remain = %d, water_level = %d in setting DI Read FIFO", info_r.number, info_r.length, info_r.remain, info_r.water_level);
	rtd_pr_scaler_memory_debug("\n Info number = %d, length = %d, remain = %d, water_level = %d in setting DI Write FIFO", info_w.number, info_w.length, info_w.remain, info_w.water_level);

	ddr_rdatdma_wtlennum.dat_water_r	= data_r.water_level;
	ddr_rdatdma_wtlennum.dat_len_r		= data_r.length;
	ddr_rdatdma_wtlennum.dat_num_r		= data_r.number;

	ddr_wdatdma_wtlennum.dat_water_w	= data_w.water_level;
	ddr_wdatdma_wtlennum.dat_len_w		= data_w.length;
	ddr_wdatdma_wtlennum.dat_num_w		= data_w.number;

	ddr_datdma_rm.dat_remain_r			= data_r.remain;
	ddr_datdma_rm.dat_remain_w			= data_w.remain;
	ddr_datdma_rm.rdma_remain_en		= 1;

	ddr_rinfdma_wtlennum.inf_water_r	= info_r.water_level;
	ddr_rinfdma_wtlennum.inf_len_r		= info_r.length;
	ddr_rinfdma_wtlennum.inf_num_r		= info_r.number;

	ddr_winfdma_wtlennum.inf_water_w	= info_w.water_level;
	ddr_winfdma_wtlennum.inf_len_w		= info_w.length;
	ddr_winfdma_wtlennum.inf_num_w		= info_w.number;;

	ddr_infdma_rm.inf_remain_r			= info_r.remain;
	ddr_infdma_rm.inf_remain_w			= info_w.remain;

	//remain and number both 0 will cause DMA error when 1080p switch to 576i DI will fliker eric_huang@realsil.com.cn
	if (info_r.number == 0 && info_r.remain == 0) {
		ddr_rinfdma_wtlennum.inf_num_r = 0x174;
		ddr_infdma_rm.inf_remain_r = 0x0b;
	}

	//remain and number both 0 will cause DMA error when 1080p switch to 576i DI will fliker eric_huang@realsil.com.cn
	if (info_w.number == 0 && info_w.remain == 0) {
		ddr_winfdma_wtlennum.inf_num_w = 0x174;
		ddr_infdma_rm.inf_remain_w = 0x0b;
	}

	IoReg_Write32(DI_DDR_RDATDMA_WTLENNUM_reg, ddr_rdatdma_wtlennum.regValue);
	IoReg_Write32(DI_DDR_WDATDMA_WTLENNUM_reg, ddr_wdatdma_wtlennum.regValue);
	IoReg_Write32(DI_DDR_RINFDMA_WTLENNUM_reg, ddr_rinfdma_wtlennum.regValue);
	IoReg_Write32(DI_DDR_WINFDMA_WTLENNUM_reg, ddr_winfdma_wtlennum.regValue);
	IoReg_Write32(DI_DDR_DATDMA_RM_reg, ddr_datdma_rm.regValue);
	IoReg_Write32(DI_DDR_INFDMA_RM_reg, ddr_infdma_rm.regValue);

	//frank@0514 change DI multi request to solve DI read underflow ++
	drv_memory_disable_di_write_req(FALSE);
	//frank@0514 change DI multi request to solve DI read underflow --
#else

//=======YUV subsample setting  ===========
	data_bitNum = 16; // for YUV subsample 422 ( now only using 2 typr : 422 & 420  )

	if (access == SLR_DI_5A_mode) {
		info_bitNum+= (drv_memory_get_ip_Btr_mode() == _ENABLE? 8: 0); // type A:8 bits, type B:4 bits, default is type A
	} else if (access == SLR_DI_3A_mode) {
		info_bitNum+= (drv_memory_get_ip_Btr_mode() == _ENABLE? 4: 0); // type A:8 bits, type B:4 bits, default is type A
	}

	// if k temporal function is enabled, new rtnr need 2 bits, otherwise still 0, please refer scalerColor.cpp:fwif_color_set_RTNR_style_adjust()
	if (newRTNR_onoff == 3 || newRTNR_onoff == 4) {
		info_bitNum +=2;
	}


	if(access == SLR_DI_5A_mode) {	// DI 5A
		rtd_pr_scaler_memory_debug("\n 5A mode \n");
		//frank@0310 10bits mode
		if (fw_scalerip_get_DI_chroma_10bits()) {
			data_bitNum += 2;
		}
		//Elsie 20140117: DI chroma 10-bit needs additional memory space.
		if(fw_scalerip_get_DI_chroma_10bits())
			data_bitNum += 2;
		DIAllocateBlock = 4;
	} else if(access == SLR_DI_3A_mode){	// DI 3A
		rtd_pr_scaler_memory_debug("\n 3A mode \n");
		//if(!drv_memory_get_ip_Btr_mode())//frank@0607 if enable BTR,info_num have add 8 bits
			info_bitNum += 4; // bitNum default + 4 in 3A mode
		if (fw_scalerip_get_DI_chroma_10bits()) {
			data_bitNum += 2;
		}
		//Elsie 20140117
		if(fw_scalerip_get_DI_chroma_10bits())
			data_bitNum += 2;

		DIAllocateBlock = 2;
	}else if(access == SLR_DI_progress_mode){
		rtd_pr_scaler_memory_debug("\n progress mode \n");

		if (fw_scalerip_get_DI_chroma_10bits()) {
			data_bitNum += 2;
		}
		//Elsie 20140117
		if(fw_scalerip_get_DI_chroma_10bits())
			data_bitNum += 2;

	//	bitNum = bitNum;
		rtd_pr_scaler_memory_debug("\n Data.bitNum=%x,info.bitNum=%x\n",data_bitNum,info_bitNum);

		DIAllocateBlock = 2;	//roll back to 18480. set 1.gde buffer will be writen by something and cause gde fail. josh@20090423
	} else {
		rtd_pr_scaler_memory_debug("\n Error accept parameter in memory fifo \n");
		return;
	}

	//Elsie 20131210 sync from Mac2
	//if(DI_SMD_enable)
	//{
	//	rtd_pr_scaler_memory_debug("\n DI_SMD_enable \n");
	//	info_bitNum += 4;
	//}
	//Elsie 20140116: DO_SMD always on
	rtd_pr_scaler_memory_debug("\n DI_SMD_enable \n");
	info_bitNum += 4;

#ifdef ENABLE_DI_COMPRESS
	if((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) > 2048)
			&& (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > 1200)){	// 4k2k
		drv_memory_set_ip_compress_bit(12);
		m_rtnrcompress = 1;
		data_bitNum = 12;
	}
#endif

	// [DI] DI DMA 420 config
	drv_memory_DMA420_config(&data_bitNum);

	//rtd_pr_scaler_memory_debug("\n=============\n");
	rtd_pr_scaler_memory_debug("\n B.Width:%d, Height:%d, data bitNum=0x%x, info bitNum=0x%x, BTR/420=%d/%d\n",Scaler_DispGetInputInfo(SLR_INPUT_DI_WID), Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN), data_bitNum, info_bitNum, drv_memory_get_ip_Btr_mode(), drv_memory_get_ip_DMA420_mode());
	rtd_pr_scaler_memory_debug("\n Y/C bits:%d/%d\n",fw_scalerip_get_DI_chroma_10bits(), fw_scalerip_get_DI_chroma_10bits());
	//sync code from Mac2. Start
	if(access == SLR_DI_5A_mode) {
		//frank@0725 change below water level to solve peak BW not enough from Fishtail
	//	ddr_rdatdma_wtlennum.dat_water_r = 0x20;// DI total FIFO = 256, 0x100, Read Watel = Total - length
		ddr_rdatdma_wtlennum.dat_water_r = 68;// DI total FIFO = 256, 0x100, Read Watel = Total - length
		ddr_rdatdma_wtlennum.dat_len_r= 60;   //Change only support max127 0x7F
		ddr_rinfdma_wtlennum.inf_water_r = 0x20;// DI total FIFO = 256, 0x100, Read Watel = Total - length
		ddr_rinfdma_wtlennum.inf_len_r = 0x20;   //Change only support max127 0x7F
		//DI Read Water Level, Length, Number, Remain
		data_value = (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*data_bitNum) >>12; //AT:(128*32);pacific:(64*32)
	       data_tmp = (((Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*data_bitNum)-(data_value<<12)) + (128 - 1))>>7;//AT:128;pacific:64,force remain + 1 , if (remain / fifo size) != 0 , avoid to occur dirty line in last line of video [chengying  2010,4/16]
	       info_value = (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*info_bitNum) >>12; //AT:(128*32);pacific:(64*32)
	       info_tmp = (((Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*info_bitNum)-(info_value<<12)) + (128 - 1))>>7;//AT:128;pacific:64,force remain + 1 , if (remain / fifo size) != 0 , avoid to occur dirty line in last line of video [chengying  2010,4/16]
	}else{
		//frank@0725 change below water level to solve peak BW not enough from Fishtail
		// if(access == SLR_DI_3A_mode)
		// 	ddr_rdatdma_wtlennum.dat_water_r = 0xe0;//0x20;// DI total FIFO = 128, 0x100, Read Watel = Total - length
		// else
		// 	ddr_rdatdma_wtlennum.dat_water_r = 0xff;

		//frank@0531 change below water level to solve peak BW not enough
		// sync from Sirius
		ddr_rdatdma_wtlennum.dat_water_r = 196;//0x60;//0x20;// DI total FIFO = 128, 0x100, Read Watel = Total - length
		ddr_rdatdma_wtlennum.dat_len_r= 60;   //Change only support max127 0x7F

		//ddr_rdatdma_wtlennum.dat_water_r = 0x60;//0x20;// DI total FIFO = 128, 0x100, Read Watel = Total - length
		// ddr_rdatdma_wtlennum.dat_len_r = 0x20;   //Change only support max127 0x7F
		ddr_rinfdma_wtlennum.inf_water_r = 0x20;// DI total FIFO = 128, 0x100, Read Watel = Total - length
		ddr_rinfdma_wtlennum.inf_len_r= 0x20;   //Change only support max127 0x7F
		//DI Read Water Level, Length, Number, Remain
		data_value = (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*data_bitNum) >>12; //AT:(128*32);pacific:(64*32)
	       data_tmp = (((Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*data_bitNum)-(data_value<<12)) + (128 - 1))>>7;//AT:128;pacific:64,force remain + 1 , if (remain / fifo size) != 0 , avoid to occur dirty line in last line of video [chengying  2010,4/16]
	       info_value = (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*info_bitNum) >>12; //AT:(128*32);pacific:(64*32)
	       info_tmp = (((Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*info_bitNum)-(info_value<<12)) + (128 - 1))>>7;//AT:128;pacific:64,force remain + 1 , if (remain / fifo size) != 0 , avoid to occur dirty line in last line of video [chengying  2010,4/16]
	}
	//sync code from Mac2. End
	if(drv_memory_get_ip_DMA420_mode()){
		lineSize_even_128 = (Scaler_DispGetInputInfo(SLR_INPUT_DI_WID) * (data_bitNum - ((fw_scalerip_get_DI_chroma_10bits())?10:8)));
		if((lineSize_even_128%128)==0)
			lineSize_even_128 /=128;
		else
			lineSize_even_128 = (lineSize_even_128+128)/128;

		lineSize_odd_128 = (Scaler_DispGetInputInfo(SLR_INPUT_DI_WID) * data_bitNum);
		if((lineSize_odd_128%128)==0)
			lineSize_odd_128 /=128;
		else
			lineSize_odd_128 = (lineSize_odd_128+128)/128;

		if (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)%2 == 0) {
			di420_width = (lineSize_odd_128 + lineSize_even_128)*(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)/2);
		} else {
			di420_width = lineSize_even_128*(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)/2)+lineSize_odd_128*((Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)+1)/2);
		}
		di420_value = di420_width>>5;
		di420_remain = (di420_width)%32;

		data_value = di420_value;
		data_tmp = di420_remain;
	}

	if(data_tmp == 0 && data_value!=0) { //remain can't be zero
		data_value -= 1;
		data_tmp = 60;//0x20;
	}

	if(info_tmp == 0 && info_value!=0) { //remain can't be zero
		info_value -= 1;
		info_tmp = 0x20;
	}

	rtd_pr_scaler_memory_debug("\n Data value = %x, tmp = %x in setting DI Read FIFO \n", data_value, data_tmp);
	rtd_pr_scaler_memory_debug("\n Info value = %x, tmp = %x in setting DI Read FIFO \n", info_value, info_tmp);


	ddr_rdatdma_wtlennum.dat_num_r=data_value;
	ddr_datdma_rm.dat_remain_r=data_tmp;
	ddr_datdma_rm.rdma_remain_en = 1;
	//remain and number both 0 will cause DMA error when 1080p switch to 576i DI will fliker eric_huang@realsil.com.cn
	if (info_value == 0 && info_tmp == 0) {
		ddr_rinfdma_wtlennum.inf_num_r=0x174;
		ddr_infdma_rm.inf_remain_r=0x0b;
	} else {
		ddr_rinfdma_wtlennum.inf_num_r=info_value;
		ddr_infdma_rm.inf_remain_r=info_tmp;
	}

	IoReg_Write32(DI_DDR_RDATDMA_WTLENNUM_reg, ddr_rdatdma_wtlennum.regValue);
	IoReg_Write32(DI_DDR_RINFDMA_WTLENNUM_reg, ddr_rinfdma_wtlennum.regValue);
	#if 1
	//sync code from Mac2. Start
	//DI Write Water Level, Length, Number, Remain
	data_value = (  Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*data_bitNum)>>12; //AT:(128*32);pacific(64*64)

	//force remain + 1 , if (remain / fifo size) != 0 , avoid to occur dirty line in last line of video [chengying  2010,4/16]
       data_tmp = ((( Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*data_bitNum)-(data_value<<12)) + (128 - 1)) >>7;//AT:128;pacific:64
	//DI Write Water Level, Length, Number, Remain
	info_value = (  Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*info_bitNum)>>12; //AT:(128*32);pacific(64*64)

	//force remain + 1 , if (remain / fifo size) != 0 , avoid to occur dirty line in last line of video [chengying  2010,4/16]
       info_tmp = ((( Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*info_bitNum)-(info_value<<12)) + (128 - 1)) >>7;//AT:128;pacific:64

	if(drv_memory_get_ip_DMA420_mode()){
		data_value = di420_value;
		data_tmp = di420_remain;
	}

        if(data_tmp == 0 && data_value!=0) { //remain can't be zero
           	data_value -= 1;
		data_tmp = 0x20;
       }

	if(info_tmp == 0 && info_value!=0) { //remain can't be zero
           	info_value -= 1;
		info_tmp = 0x20;
       }
	//sync code from Mac2. End
	#endif

	rtd_pr_scaler_memory_debug("Write data value = %x, tmp = %x in setting DI Write FIFO\n", data_value, data_tmp);
	rtd_pr_scaler_memory_debug("Info data value = %x, tmp = %x in setting DI Write FIFO\n", info_value, info_tmp);
	ddr_wdatdma_wtlennum.dat_num_w=data_value;
	ddr_datdma_rm.dat_remain_w=data_tmp;
	ddr_wdatdma_wtlennum.dat_water_w = 0x3c;
	ddr_wdatdma_wtlennum.dat_len_w = 0x3c;
	//remain and number both 0 will cause DMA error when 1080p switch to 576i DI will fliker eric_huang@realsil.com.cn
	if (info_value == 0 && info_tmp == 0) {
		ddr_winfdma_wtlennum.inf_num_w=0x174;
		ddr_infdma_rm.inf_remain_w=0x0b;
	} else {
		ddr_winfdma_wtlennum.inf_num_w=info_value;
		ddr_infdma_rm.inf_remain_w=info_tmp;
	}
	ddr_winfdma_wtlennum.inf_water_w = 0x20;
	ddr_winfdma_wtlennum.inf_len_w = 0x20;

	IoReg_Write32(DI_DDR_WDATDMA_WTLENNUM_reg, ddr_wdatdma_wtlennum.regValue);
	IoReg_Write32(DI_DDR_DATDMA_RM_reg, ddr_datdma_rm.regValue);
	IoReg_Write32(DI_DDR_WINFDMA_WTLENNUM_reg, ddr_winfdma_wtlennum.regValue);
	IoReg_Write32(DI_DDR_INFDMA_RM_reg, ddr_infdma_rm.regValue);

//frank@0514 change DI multi request to solve DI read underflow ++
	drv_memory_disable_di_write_req(FALSE);
//frank@0514 change DI multi request to solve DI read underflow --

#endif

//frank@1202 Change code to solve 1366x768 image fail problem

#if 1/*qiangzhou: added DI memory allocate with VGIP PRE info for max size start*/
#if(CONFIG_DDR_DATA_BITS == 16)
	/*if ((Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_PIPMLSR) && (Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_PIPMLSR_4x3) &&
		(Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_MP1C12B) && (Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_MP1L7B)) {*/
		data_value = data_bitNum;
		info_value = info_bitNum;
		if(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > _DISP_WID){
			data_value *= _DISP_WID;
			info_value *= _DISP_WID;
		}else{
			data_value *= Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
			info_value *= Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
		}


		if(((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
			&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
			&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status()) {
			data_value *= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
			info_value *= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
		} else {
			data_value *= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
			info_value *= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
		}

		data_value = (data_value >> 3);//byte unit
		info_value = (info_value >> 3);//byte unit
//		value =(info->IPV_ACT_LEN_PRE *info->IPH_ACT_WID_PRE * bitNum)>>4;
	/*} else {
		data_value=(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*data_bitNum)>>3;	//byte unit
		info_value=(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*info_bitNum)>>3;	//byte unit
	}*/
#else
	data_value=(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)*Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)*data_bitNum)>>3;	//byte unit
	info_value=(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)*Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)*info_bitNum)>>3;	//byte unit
#endif

	if (drv_memory_get_ip_DMA420_mode()) {
		//sync code from Mac2. Start
		di_im_di_dma_RBUS im_di_dma;
		unsigned int di420_width;
		im_di_dma.regValue = IoReg_Read32(DI_IM_DI_DMA_reg);
		lineSize_even_128 = (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) * (data_bitNum - 8));
		if((lineSize_even_128%128)==0)
			lineSize_even_128 /=128;
		else
			lineSize_even_128 = (lineSize_even_128+128)/128;

		lineSize_odd_128 = (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) * data_bitNum);
		if((lineSize_odd_128%128)==0)
			lineSize_odd_128 /=128;
		else
			lineSize_odd_128 = (lineSize_odd_128+128)/128;

		if (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)%2 == 0) {
			di420_width = (lineSize_odd_128 + lineSize_even_128)*(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)/2);
		} else {
			di420_width = lineSize_even_128*(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)/2)+lineSize_odd_128*((Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)+1)/2);
		}
		data_value = di420_width<<4;
		//sync code from Mac2. End
	}
#endif/*qiangzhou: added DI memory allocate with VGIP PRE info for max size end*/

	drvif_memory_free_block(MEMIDX_DI);

	// Set 32 avoid to cover other block memroy ([[h * v * bitnum] / (64* 2) ]* 16
	data_value = drvif_memory_get_data_align(data_value, 32);
	info_value = drvif_memory_get_data_align(info_value, 32);
	//frank@0707 when 3D mode DI must allocate 2 X memory

#if 1//defined(ENABLE_DRIVER_I3DDMA)|| defined(ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE)
	//3D mode DI memory allocate x2: 3D frame sequence input video
	if(drvif_scaler3d_get_IDMA3dMode()
		|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
			&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
			&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status())) {
		data_value *= 2;
		info_value *= 2;
	}
#endif

#if 1
	drv_memory_di_alloc_buffer_set(Scaler_InputSrcGetMainChType(),fw_scalerip_get_DI_chroma_10bits());
	buffer_alloc=drv_memory_di_alloc_buffer_get();
	data_size=drv_memory_di_data_size_get(access,Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE));
	info_size=drv_memory_di_info_size_get(access);
#ifdef CONFIG_I2RND_ENABLE
	//add I2rnd condition @Crixus 20160716
	if(!((Scaler_I2rnd_get_timing_enable() == _ENABLE) && ((vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC)|| (vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1)))){
		//rtd_pr_scaler_memory_emerg("[%s][crixus]DI non-i2rnd memory allocation\n", __FUNCTION__);
#endif
		//define at makeconfig
		//patch for DI/RTNR bug(overwrite 512Byte)
		if(drvif_scaler3d_get_IDMA3dMode()
			|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
				&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
				&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status())){

			if(access != 2){
				MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, data_value * DIAllocateBlock+info_value*2+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);
			}
			else{
#ifdef ENABLE_DI_COMPRESS
				if(m_rtnrcompress){//compress mode use 2 buffer
					MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, data_value*2+info_value*2+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);
				}
				else
#endif
				{
					MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, data_value+info_value*2+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);
				}
			}
		} else {
			MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, buffer_alloc, MEMALIGN_ANY);
		}
		if(MemStartAddr == _DUMMYADDR){
			rtd_pr_scaler_memory_emerg("set MEMIDX_DI  to size fail !!!\n");
		}
#ifdef CONFIG_I2RND_ENABLE
		//rtd_pr_scaler_memory_emerg("[%s][crixus]DI non-i2rnd MemStartAddr = %x\n", __FUNCTION__, MemStartAddr);

	}
	else{//I2rnd case - allocate DI 2 memory
		//rtd_pr_scaler_memory_emerg("[%s][crixus]DI i2rnd second memory allocation\n", __FUNCTION__);
		if(drvif_scaler3d_get_IDMA3dMode()
			|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
			&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
			&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status())){
			if(access != 2){
				MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI_2, data_value * DIAllocateBlock+info_value*2+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);
			}
			else{
#ifdef ENABLE_DI_COMPRESS
				if(m_rtnrcompress){//compress mode use 2 buffer
					MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI_2, data_value*2+info_value*2+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);
				}
				else
#endif
				{
					MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI_2, data_value+info_value*2+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);
				}
			}
		} else {
			MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI_2, buffer_alloc, MEMALIGN_ANY);
		}
		if(MemStartAddr == _DUMMYADDR){
			rtd_pr_scaler_memory_emerg("set MEMIDX_DI_2  to size fail !!!\n");
		}
		//rtd_pr_scaler_memory_emerg("[%s][crixus]DI i2rnd MemStartAddr = %x\n", __FUNCTION__, MemStartAddr);
	}
#endif
#else
	if(access != 2)
		MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, data_value * DIAllocateBlock+info_value*2, MEMALIGN_ANY);
	else{
		if(m_rtnrcompress)//compress mode use 2 buffer
			MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, data_value*2+info_value*2, MEMALIGN_ANY);
		else
			MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, data_value+info_value*2, MEMALIGN_ANY);
	}
#endif
	//rtd_pr_scaler_memory_debug("Get ip addr = %x, size=%x\n", MemStartAddr, (data_value * DIAllocateBlock+info_value*2));

#if 1//defined(ENABLE_DRIVER_I3DDMA)|| defined(ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE)
	//3D mode DI memory allocate x2: 3D frame sequence input video
	if(drvif_scaler3d_get_IDMA3dMode()
		|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
			&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
			&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status())) {
		data_value /= 2;
		info_value /= 2;
	}
#endif

#ifdef CONFIG_I2RND_ENABLE
	//add I2rnd condition @Crixus 20160716
	if(!((Scaler_I2rnd_get_timing_enable() == _ENABLE) && ((vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC) || (vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1)))){
		//rtd_pr_scaler_memory_emerg("[%s][crixus]DI non-i2rnd set start address\n", __FUNCTION__);
#endif
		if(drvif_scaler3d_get_IDMA3dMode()
				|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
					&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
					&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status()))
		{
			// Set start address for deinterlace
			for(i = 0; i < 4; i++) {
				rtd_pr_scaler_memory_debug("IP address%x = %x\n", i, MemStartAddr);

				IoReg_Write32((DI_DI_DATMemoryStartAdd1_reg+ (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

				if((access != SLR_DI_progress_mode)|| m_rtnrcompress)
					MemStartAddr += data_value;//data_value;
			}

			if((access == SLR_DI_progress_mode)&&!m_rtnrcompress){
				MemStartAddr += data_value;
			}

			for(i = 0; i < 2; i++) {
				rtd_pr_scaler_memory_debug("Info IP address%x = %x\n", i, MemStartAddr);

				IoReg_Write32((DI_DI_INFMemoryStartAdd1_reg+ (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

				if(access != SLR_DI_progress_mode)
					MemStartAddr += info_value;
			}
		}else {
			for(i = 0; i < 4; i++) {
				rtd_pr_scaler_memory_debug("IP address%x = %x\n", i, MemStartAddr);
				IoReg_Write32((DI_DI_DATMemoryStartAdd1_reg+ (i << 2)),  (MemStartAddr&0x7FFFFFFF) );
				if((access != SLR_DI_progress_mode)|| m_rtnrcompress)
					MemStartAddr += data_size;//data_value;
			}
			if((access == SLR_DI_progress_mode)&&!m_rtnrcompress){
				MemStartAddr += data_size;
			}

			for(i = 0; i < 2; i++) {
				rtd_pr_scaler_memory_debug("Info IP address%x = %x\n", i, MemStartAddr);

				IoReg_Write32((DI_DI_INFMemoryStartAdd1_reg+ (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

				if(access != SLR_DI_progress_mode)
					MemStartAddr += info_size;
			}
		}

#if 1//defined(ENABLE_DRIVER_I3DDMA)|| defined(ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE)
		if(drvif_scaler3d_get_IDMA3dMode()
			|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
				&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
				&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status())) {
			for(i = 0; i < 4; i++) {
				rtd_pr_scaler_memory_debug("3D IP address%x = %x\n", i, MemStartAddr);

				IoReg_Write32((DI_DI_DATMemoryStartAdd1_3D_reg+ (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

				if((access != SLR_DI_progress_mode)|| m_rtnrcompress)
					MemStartAddr += data_value;
			}

			if((access == SLR_DI_progress_mode) && !m_rtnrcompress){
				MemStartAddr += data_value;
			}

			for(i = 0; i < 2; i++) {
				rtd_pr_scaler_memory_debug("3D Info IP address%x = %x\n", i, MemStartAddr);

				IoReg_Write32((DI_DI_INFMemoryStartAdd1_3D_reg+ (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

				if(access != SLR_DI_progress_mode)
					MemStartAddr += info_value;
			}
		}
#endif
#ifdef CONFIG_I2RND_ENABLE
	}
	else{//I2rnd case - set DI memory start address

		//rtd_pr_scaler_memory_emerg("[%s][crixus]DI i2rnd set start address\n", __FUNCTION__);
		if(drvif_scaler3d_get_IDMA3dMode()
				|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
					&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
					&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status()))
		{
			// Set start address for deinterlace
			for(i = 0; i < 4; i++) {
				rtd_pr_scaler_memory_debug("IP address%x = %x\n", i, MemStartAddr);

				IoReg_Write32((DI_DI_I2R_DATMemoryStartAdd1_reg+ (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

				if((access != SLR_DI_progress_mode)|| m_rtnrcompress)
					MemStartAddr += data_value;//data_value;
			}

			if((access == SLR_DI_progress_mode)&&!m_rtnrcompress){
				MemStartAddr += data_value;
			}

			for(i = 0; i < 2; i++) {
				rtd_pr_scaler_memory_debug("Info IP address%x = %x\n", i, MemStartAddr);

				IoReg_Write32((DI_DI_I2R_INFMemoryStartAdd1_reg+ (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

				if(access != SLR_DI_progress_mode)
					MemStartAddr += info_value;
			}
		}else {
			for(i = 0; i < 4; i++) {
				rtd_pr_scaler_memory_debug("IP address%x = %x\n", i, MemStartAddr);
				IoReg_Write32((DI_DI_I2R_DATMemoryStartAdd1_reg+ (i << 2)),  (MemStartAddr&0x7FFFFFFF) );
				if((access != SLR_DI_progress_mode)|| m_rtnrcompress)
					MemStartAddr += data_size;//data_value;
			}
			if((access == SLR_DI_progress_mode)&&!m_rtnrcompress){
				MemStartAddr += data_size;
			}

			for(i = 0; i < 2; i++) {
				rtd_pr_scaler_memory_debug("Info IP address%x = %x\n", i, MemStartAddr);

				IoReg_Write32((DI_DI_I2R_INFMemoryStartAdd1_reg+ (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

				if(access != SLR_DI_progress_mode)
					MemStartAddr += info_size;
			}
		}

#if 1//defined(ENABLE_DRIVER_I3DDMA)|| defined(ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE)
		if(drvif_scaler3d_get_IDMA3dMode()
			|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
				&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
				&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status())) {
			for(i = 0; i < 4; i++) {
				rtd_pr_scaler_memory_debug("3D IP address%x = %x\n", i, MemStartAddr);

				IoReg_Write32((DI_DI_I2R_DATMemoryStartAdd1_3D_reg+ (i << 2)),	(MemStartAddr&0x7FFFFFFF) );

				if((access != SLR_DI_progress_mode)|| m_rtnrcompress)
					MemStartAddr += data_value;
			}

			if((access == SLR_DI_progress_mode) && !m_rtnrcompress){
				MemStartAddr += data_value;
			}

			for(i = 0; i < 2; i++) {
				rtd_pr_scaler_memory_debug("3D Info IP address%x = %x\n", i, MemStartAddr);

				IoReg_Write32((DI_DI_I2R_INFMemoryStartAdd1_3D_reg+ (i << 2)),	(MemStartAddr&0x7FFFFFFF) );

				if(access != SLR_DI_progress_mode)
					MemStartAddr += info_value;
			}
		}
#endif
	}
#endif

	return;
#endif
}

void drvif_memory_set_fs_display_fifo(void)
{
	unsigned int TotalSize;
	unsigned int Quotient;
	unsigned char Remainder;
	mdomain_cap_capture1_byte_swap_RBUS capture_byte_swap_reg;
	mdomain_disp_display_byte_channel_swap_RBUS display_byte_channel_swap_reg;
	unsigned int fifoLen = _FIFOLENGTH;
	unsigned int burstLen = _BURSTLENGTH;

	//frank@0325 double the M domain burst length to avoid 444 10 bits peak BW not enough
	if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)_CHANNEL1, SLR_DISP_422CAP))){
		burstLen=0xf0;
	}

	//add below code to clear data swap let data FS content correct
	display_byte_channel_swap_reg.regValue = IoReg_Read32(MDOMAIN_DISP_Display_byte_channel_swap_reg);
	display_byte_channel_swap_reg.main_2byte_swap = 0;
	display_byte_channel_swap_reg.main_4byte_swap = 0;
	display_byte_channel_swap_reg.main_1byte_swap = 0;
	display_byte_channel_swap_reg.main_8byte_swap = 0;
	IoReg_Write32(MDOMAIN_DISP_Display_byte_channel_swap_reg, display_byte_channel_swap_reg.regValue);

	capture_byte_swap_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Capture1_byte_swap_reg);
	capture_byte_swap_reg.in1_2byte_swap = 0;
	capture_byte_swap_reg.in1_4byte_swap = 0;
	capture_byte_swap_reg.in1_1byte_swap = 0;
	capture_byte_swap_reg.in1_8byte_swap = 0;
	IoReg_Write32(MDOMAIN_CAP_Capture1_byte_swap_reg, capture_byte_swap_reg.regValue);

	// Set default fifo length as pixel number avoid to framesync failure
	//frank@0323 change waterlevel/2 for AT,because water level nit is 128 bits
	IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, (64 << 8) | ((fifoLen-(burstLen>>1))>>1));
	IoReg_Mask32(MDOMAIN_DISP_DDR_MainLineNum_reg, 0xfffff000, 0);

	TotalSize = memory_get_display_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY));	// calculate the memory size of capture
	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division
	if(Remainder == 0) {	// remainder is not allowed to be zero
		Remainder = burstLen;
		Quotient -= 1;
	}

	IoReg_Write32(MDOMAIN_DISP_DDR_MainRdNumLenRem_reg, (Quotient << 16) | (burstLen << 8) | Remainder);
	IoReg_Write32(MDOMAIN_DISP_DDR_MainAddrDropBits_reg, 0);


	//compression & de-compression setting
/*	if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && (dvrif_memory_compression_get_enable() == TRUE)){
		dvrif_memory_comp_setting(TRUE, Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_CAP_WID), Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_CAP_LEN), dvrif_memory_get_compression_bits());
		IoReg_SetBits(MDOMAIN_DISP_DDR_MainSubCtrl_reg, _BIT0);
	}else{*/
		dvrif_memory_comp_setting(FALSE, Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_CAP_WID), Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_CAP_LEN), dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY));
		IoReg_SetBits(MDOMAIN_DISP_DDR_MainSubCtrl_reg, _BIT0);
	//}

}
#ifndef BUILD_QUICK_SHOW

/*============================================================================*/
/**
 * drvif_memory_rotate_mp
 * Set active MP and current address for MP rottion
 *
 * @param <MpIdx> {MP window to be active}
 * @return { none }
 * @note
 * It can be called in sub display
 *
 */
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
void drvif_memory_rotate_mp(unsigned short MpIdx)
{
	unsigned int  tmpValue;
	unsigned char  MpNum;

	for (MpNum=0; MpNum<16; MpNum++) {
		if (AND(SHR(MpIdx, MpNum), 0x01))	// check each bit from LSB
			break;	// found current Mp num
	}

	MpNum = MpNum%16;
	tmpValue = Scaler_PipGetSubInfo(SLR_PIP_SUB_MPDISPBLOCKSTEP);		// get block step
	tmpValue *= MpNum*_DDR_BANK_SIZE;					// calculate the offset from start address
	tmpValue += (MemTag[MEMIDX_SUB].StartAddr);		// plus start address, update MSB & Medium bytes

	IoReg_Write32(MDOMAIN_CAP_DDR_In2Addr_reg, (tmpValue << 1) & 0x7ffffff0);

	tmpValue = IoReg_ReadByte0(MDOMAIN_DISP_DDR_SubMPMode_reg);
	tmpValue = AND((unsigned char)tmpValue, 0x0F) + SHL(MpNum, 4);	// update active MP
	IoReg_WriteByte0(MDOMAIN_DISP_DDR_SubMPMode_reg, tmpValue);
	drvif_memory_set_dbuffer_write();
}

#endif //CONFIG_DUAL_CHANNEL  // There are two channels
#endif
/*============================================================================*/
/**
 * drvif_memory_set_dbuffer_write
 * Enable double buffer write-in.
 *
 * @param <display> { main/sub display }
 * @return { none }
 * @note
 * Memory main/sub display are double buffered, those registers willbe updated
 * at the edge of Vsync of display out.
 *
 */
//void drvif_memory_set_dbuffer_write(unsigned char display)
unsigned char drvif_memory_set_dbuffer_write(void)
{
	unsigned int tmp;
	// write data & enable double buffer
	// due to mantis bug:10311, we have to write 3 times to enable double buffer register write
	//tmp = IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg) | _BIT0;
	//frank@0623 add sub display double buffer reg apply
	tmp = IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg) | _BIT0 | _BIT16;
	if(scaler_vsc_get_vr360_block_mdomain_doublebuffer()==FALSE)
		IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg, tmp);
#if 0//frank@0903 reduce scaler flow time
	tmp = 0;
	while(IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg)&_BIT0){
		//Rt_Sleep(1);//frank@0509 change delay api to speed up
		msleep(1);
		if(++tmp>40)
			break;
	}

	return (tmp > 40? FALSE: TRUE);
#else
	return TRUE;
#endif
}
#ifndef BUILD_QUICK_SHOW

/**
 * memory_get_jpeg_display_size
 * Calculate the one line memory size of JPEG display.
 * The size is counted by 4:2:2/4:4:4 data format, and frame-access/line access.
 *
 * @param <info> { disp-info structure }
 * @return { line total size }
 */
static unsigned int memory_get_jpeg_display_size(StructJPEGDisplayInfo *jpeg_info)
{
	unsigned int TotalSize;
	unsigned char ddrbit;

	if ( jpeg_info == NULL ) {
		rtd_pr_scaler_memory_debug("[memory_get_jpeg_display_size] jpeg_info cannot be NULL.\n");
		return 0;
	}
	ddrbit = (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_DISP_10BIT) << 1)  | ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_DISP_422CAP));
	TotalSize = memory_get_line_size(jpeg_info->MemWid, (ddrbit & _BIT0), ((ddrbit & _BIT1) >> 1));

	return TotalSize;
}

#if 0
/**
 * memory_get_jpeg_display_size
 * Calculate the one line memory size of JPEG display.
 * The size is counted by 4:2:2/4:4:4 data format, and frame-access/line access.
 *
 * @param <info> { disp-info structure }
 * @return { line total size }
 */
static unsigned int memory_get_osd_display_size(SCALER_DISP_CHANNEL disp, StructJPEGDisplayInfo *jpeg_info)
{
	unsigned int TotalSize;
	unsigned int LineSize;
	//unsigned char ddrbit;

	if ( jpeg_info == NULL ) {
		rtd_pr_scaler_memory_debug("[memory_get_jpeg_display_size] jpeg_info cannot be NULL.\n");
		return 0;
	}
	//ddrbit = (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_DISP_10BIT) << 1)  | ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_DISP_422CAP));
	LineSize = jpeg_info->MemWid;
	LineSize = drvif_memory_get_data_align(LineSize*32, 64);
	TotalSize = (unsigned int)SHR(LineSize, 6);

	return TotalSize;
}
#endif
/**
 * memory_get_jpeg_capture_size
 * Calculate the memory size of capture.
 * The size is counted by 4:2:2/4:4:4 data format, and frame-access/line access.
 *
 * @param	<jpeg_info>		{ jpeg_info structure }
 * @param	<AccessType>	{ Access_Type: frame or line }
 * @return					{ size of capture }
 */
static unsigned int memory_get_jpeg_capture_size(StructJPEGDisplayInfo *jpeg_info, eMemCapAccessType AccessType)
{
	unsigned int TotalSize;
	unsigned char ddrbit;

	if ( jpeg_info == NULL ) {
		rtd_pr_scaler_memory_debug("[memory_get_jpeg_capture_size] jpeg_info cannot be NULL.\n");
		return 0;
	}

	ddrbit =  ((Scaler_DispGetStatus((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_DISP_10BIT)) << 1 )  | ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY,SLR_DISP_422CAP));
	TotalSize = memory_get_line_size(jpeg_info->MemWid, (ddrbit & _BIT0), ((ddrbit & _BIT1) >> 1));
	TotalSize = drvif_memory_get_data_align(TotalSize, 0x04);	// it must be times of 4

	if (AccessType == MEMCAPTYPE_FRAME)
		TotalSize = TotalSize * jpeg_info->MemLen; // multiple total_lines if frame-access

	return TotalSize;
}

/**
 * drvif_memory_set_jpeg_disp_mem
 * This function is used to set display memory for JPEG.
 *
 * @param	<jpeg_info>	{ pointer of JPEG display info, set NULL for disable }
 * @return				{ none }
 * @note
 * If any one of the Memory-main registers is changed, it is necessary to write
 * P3_8B register to validate the update.
 */
void drvif_memory_set_jpeg_disp_mem(StructJPEGDisplayInfo *jpeg_info)
{
	unsigned int TotalSize;
	unsigned int Quotient;
	unsigned char Remainder;
	unsigned int LineStep;
	unsigned int fifoLen = _FIFOLENGTH;
	unsigned int burstLen = _BURSTLENGTH;

	//frank@0325 double the M domain burst length to avoid 444 10 bits peak BW not enough
	if((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1) && ( !Scaler_DispGetStatus((SCALER_DISP_CHANNEL)_CHANNEL1, SLR_DISP_422CAP))){
		burstLen=0xf0;
	}

	if ( jpeg_info == NULL ) {
		rtd_pr_scaler_memory_debug("[drvif_memory_set_jpeg_disp_mem] jpeg_info cannot be NULL.\n");
		return;
	}

	// set width and length
	//frank@0323 change waterlevel/2 for AT,because water level nit is 128 bits
	IoReg_Mask32(MDOMAIN_DISP_DDR_MainNumWTL_reg, 0xfff00000, ((jpeg_info->MemWid & 0x0FFF) << 8) | ((fifoLen-(burstLen>>1))>>1));
	IoReg_Mask32(MDOMAIN_DISP_DDR_MainLineNum_reg, 0xfffff000, (jpeg_info->MemLen & 0x0FFF));

	// set DDR_MainRdNumLenRem
	TotalSize = memory_get_jpeg_display_size(jpeg_info); // calculate the memory size of capture
	memory_division(TotalSize, burstLen, &Quotient, &Remainder); // Do a division

	if ( Remainder == 0 ) { // remainder is not allowed to be zero
		Remainder = burstLen;
		Quotient -= 1;
	}

	IoReg_Write32(MDOMAIN_DISP_DDR_MainRdNumLenRem_reg, (Quotient << 16) | (burstLen << 8) | Remainder);

	// set line step (64-bits)
	LineStep = memory_get_jpeg_capture_size(jpeg_info, MEMCAPTYPE_LINE);		// calculate capture line size
	IoReg_Mask32(MDOMAIN_DISP_DDR_MainLineStep_reg, 0xf0000000, (LineStep + LineStep%2)<<3);//bit 3 need to be 0. rbus rule

	// set memory address
//--	WaitFor_DEN_STOP();
	IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg, (jpeg_info->MemAddr)&0x7ffffff0);
}

/*
void drvif_memory_set_osd_disp_mem(StructJPEGDisplayInfo *jpeg_info)
{
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	unsigned int TotalSize;
	unsigned int Quotient;
	unsigned char Remainder;
//	unsigned int LineStep;
	unsigned int fifoLen = _FIFOLENGTH2;
	unsigned int burstLen = _BURSTLENGTH2;

	if ( jpeg_info == NULL ) {
		rtd_pr_scaler_memory_debug("[drvif_memory_set_jpeg_disp_mem] jpeg_info cannot be NULL.\n");
		return;
	}

	// set width and length
	//frank@0323 change waterlevel/2 for AT,because water level nit is 128 bits
	IoReg_Mask32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg, 0xfff00000, ((jpeg_info->MemWid & 0x0FFF) << 8) | (fifoLen-burstLen));
	IoReg_Mask32(MDOMAIN_DISP_DDR_SubLineNum_reg, 0xfffff000, (jpeg_info->MemLen & 0x0FFF));

	// set DDR_MainRdNumLenRem
	TotalSize = memory_get_osd_display_size(SLR_SUB_DISPLAY, jpeg_info); // calculate the memory size of capture
	memory_division(TotalSize, burstLen, &Quotient, &Remainder); // Do a division

	if ( Remainder == 0 ) { // remainder is not allowed to be zero
		Remainder = burstLen;
		Quotient -= 1;
	}

	IoReg_Write32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, (Quotient << 16) | (burstLen << 8) | Remainder);
	TotalSize = Quotient * burstLen + Remainder; 	// count one line, 64bits unit
	TotalSize = drvif_memory_get_data_align(TotalSize, 4);	// times of 4
	IoReg_Write32(MDOMAIN_DISP_DDR_SubLineStep_reg,  (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule

	// set memory address
//--	WaitFor_DEN_STOP();
	IoReg_Write32(MDOMAIN_DISP_DDR_SubAddr_reg, jpeg_info->MemAddr);
	IoReg_Write32(MDOMAIN_DISP_DDR_SubBlkStep_reg, 0);
	IoReg_WriteWord0(MDOMAIN_DISP_DDR_SubMPEn_reg,0);
	IoReg_WriteByte0(MDOMAIN_DISP_DDR_SubMPMode_reg, _BIT3);
	IoReg_SetBits(MDOMAIN_DISP_DDR_SubCtrl_reg, _BIT6);
	drvif_memory_set_dbuffer_write();
#endif
}
*/
/*unsigned char drvif_memory_regAvPresetTable(unsigned short *ptr)
{
	if(!ptr)
		return FALSE;

	tAV_PRESET_TABLE = ptr;

	return TRUE;
}*/



void memory_set_adc_capture(unsigned int CapBuf_addr, unsigned int Hight,unsigned int width) // for 10 bit 4:4:4
{
	unsigned int  TotalSize;
//	unsigned int  TotalSize1;
//	unsigned int  TotalSizeFrame;
//	unsigned int  nStartAddr;
	unsigned int  Quotient;
	unsigned char  Remainder;
	unsigned int burstLen = _BURSTLENGTH;

	//frank@0325 double the M domain burst length to avoid 444 10 bits peak BW not enough
	if(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) == _CHANNEL1){
		burstLen=0xf0;
	}

#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_ClearBits(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg: MDOMAIN_CAP_DDR_In1Ctrl_reg, MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask);	// disable Capture
	IoReg_ClearBits(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_Cap_In2_enable_reg : MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);	// disable Capture
#else
	rtd_pr_scaler_memory_debug("[TV]: %s:%s(%d) addr=0x%x W=%d H=%d\n",__FILE__,__func__,__LINE__,CapBuf_addr,width,Hight);
	IoReg_ClearBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : */MDOMAIN_CAP_DDR_In1Ctrl_reg, MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask);	// disable Capture
	IoReg_ClearBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : */MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);	// disable Capture
#endif
	//frank@0503 add below delay to confirm M capture have disable
	msleep(20);

	//field polarity error check
	memory_check_capture_polarity_error(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL));

	// calculate the memory size of capture
	TotalSize = memory_get_line_size(width, 1, 1);//10 bits,444
	TotalSize = drvif_memory_get_data_align(TotalSize, 0x04);

// it must be times of 4
	TotalSize = TotalSize *Hight;	// multiple total_lines if frame-access

	IoReg_Mask32(MDOMAIN_CAP_DDR_In1LineStep_reg, 0xff000000, 0);
	memory_division(TotalSize, burstLen, &Quotient, &Remainder);	// Do a division
	Remainder = drvif_memory_get_data_align(Remainder, 4);	// multiple of 4
	if (Remainder == 0) {	// remainder is not allowed to be zero
		Remainder = burstLen;
		Quotient -= 1;
	}

	memory_set_capture_fifo(_CHANNEL1, Quotient, Remainder, burstLen);	// set capture FIFO

	TotalSize = SHL(Quotient * burstLen + Remainder, 3); //unit conversion. 64bits to 8bits
	TotalSize = SHL(TotalSize, 3); //unit conversion. 64bits to 8bits

	// set capture memory for main display
#ifndef CONFIG_DYNAMIC_ALLOCATION_SCALER_MEMORY
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,0x00000008);//Set lower boundary of DDR address
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,0x7ffffffc);//Set upper boundary of DDR address
#endif
	IoReg_Write32(MDOMAIN_CAP_DDR_In1Addr_reg, CapBuf_addr & 0x7ffffff0);
	rtd_pr_scaler_memory_debug("[TV]: %s:%s(%d) write addr=0x%x\n",__FILE__,__func__,__LINE__,CapBuf_addr);
	IoReg_Mask32(MDOMAIN_CAP_DDR_In1LineNum_reg, 0xfffee000, (1<<16)|AND(Hight, 0x1FFF));
	IoReg_Write32(MDOMAIN_CAP_DDR_In1Ctrl_reg, _BIT0);//single buffer
	//IoReg_SetBits(MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0); //Capture will enable after setting DMA mode
}

#if 0
void get_adc_dump_addr_size(unsigned int *PhyAddr, unsigned int *VirStartAddr,unsigned int *Size , unsigned int *Width, unsigned int* Height)
{

	if (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY))
	{
		PhyAddr = (unsigned int*)MemTag[MEMIDX_SUB].StartAddr;
		*VirStartAddr = get_scaler_memory_map_addr() + (MemTag[MEMIDX_SUB].StartAddr - AllocStartAddr);
		#ifndef CONFIG_ADC_DMA
		*Size = MemTag[MEMIDX_SUB].Size;

		#endif
	}
	else
	{
		*PhyAddr = MemTag[MEMIDX_MAIN].StartAddr;
		*VirStartAddr = get_scaler_memory_map_addr() + (MemTag[MEMIDX_MAIN].StartAddr - AllocStartAddr);
		#ifndef CONFIG_ADC_DMA
		*Size = MemTag[MEMIDX_MAIN].Size;
		#endif
	}

	//rtd_pr_scaler_memory_debug("alloc start addr=%x %x\n",AllocStartAddr,MemTag[MEMIDX_MAIN].StartAddr);

	#ifndef CONFIG_ADC_DMA
	*Width = Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID);
	*Height = Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN);
	#else
	*Width = Scaler_DispGetInputInfo(SLR_INPUT_H_LEN);
	*Height = Scaler_DispGetInputInfo(SLR_INPUT_V_LEN) * DUMP_VSYNC; // 3 vsync
	*Size= (*Height) * (*Width) * 30 / 8;
	*Size = ((*Size) & 0xfffffffc); // align to 4
	#endif

	if (*Size >= (16 * 1024 * 1024)) // scaler memory 16M
		rtd_pr_scaler_memory_debug("Error !! memory size is not enough !!!\n");

}



void get_frc_buf_addr_size(unsigned int *PhyAddr, unsigned int *VirStartAddr,unsigned int *Size , unsigned int *Width, unsigned int* Height)
{

	int is_10bit = 0,is_422 = 0;
	if (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY))
	{
		PhyAddr = (unsigned int*)MemTag[MEMIDX_SUB].StartAddr;
		*VirStartAddr = get_scaler_memory_map_addr() + (MemTag[MEMIDX_SUB].StartAddr - AllocStartAddr);
		*Size = MemTag[MEMIDX_SUB].Size;

	}
	else
	{
		*PhyAddr = MemTag[MEMIDX_MAIN].StartAddr;
		*VirStartAddr = get_scaler_memory_map_addr() + (MemTag[MEMIDX_MAIN].StartAddr - AllocStartAddr);
		*Size = MemTag[MEMIDX_MAIN].Size;

	}

	//rtd_pr_scaler_memory_debug("alloc start addr=%x %x\n",AllocStartAddr,MemTag[MEMIDX_MAIN].StartAddr);


	is_10bit = Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_10BIT);
	is_422 = Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_422CAP);

	if (is_422)
	{
		if(is_10bit == 0)  // 422 8bit
		{
			rtd_pr_scaler_memory_debug("@@@422 8bit...\n");
			*Width = drvif_memory_get_data_align(Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID),16);
		}
		else	 //422 10bit
		{
			rtd_pr_scaler_memory_debug("@@@not support 422 10bit...\n");
			*Width = Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID);
		}
	}
	else
	{
		if(is_10bit == 0) // 444 8bit
		{
			rtd_pr_scaler_memory_debug("@@@444 8bit...\n");
			*Width = drvif_memory_get_data_align(Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID),192);
		}
		else
		{
			rtd_pr_scaler_memory_debug("@@@not support 444 10bit...\n");
			*Width = Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID);
		}
	}

	*Height = Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN);

}


void dump_frc_rawdata_start(void)
{
	if (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC))
	{
		rtd_pr_scaler_memory_debug("ERROR !!  framesync mode, can't dump data\n");
		return;
	}

	WaitFor_IEN_STOP1();
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_SetBits(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : MDOMAIN_CAP_DDR_In1Ctrl_reg, _BIT1);	// disable Capture
#else
	IoReg_SetBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : */MDOMAIN_CAP_DDR_In1Ctrl_reg, _BIT1);	// disable Capture
#endif
		//frank@0122 solve  sub ATV switch channel flick problem
//-- check		Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? IoReg_ClearBits(BUS_SB3_ARB_CR_VADDR, _BIT3) : IoReg_ClearBits(BUS_SB3_ARB_CR_VADDR, _BIT6);
		//--			ROS_EXIT_CRITICAL();
		//fwif_scaler_wait_for_event(0xb8028100, _BIT10);//<===WaitFor_DEN_STOP();
	WaitFor_DEN_STOP();
	//test for  switch TV channel have white image --
	drvif_scaler_vactive_end_irq(_DISABLE, Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)); // CSW+ 0970710 Close auto MA in freeze mode
	drvif_scaler_vactive_sta_irq(_DISABLE, Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)); // CSW+ 0970710 Close auto MA in freeze mode

}

void dump_frc_rawdata_release(void)
{
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	IoReg_ClearBits(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : MDOMAIN_CAP_DDR_In1Ctrl_reg, _BIT1);
#else
	IoReg_ClearBits(/*Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL) ? MDOMAIN_CAP_DDR_In2Ctrl_reg : */MDOMAIN_CAP_DDR_In1Ctrl_reg, _BIT1);
#endif
	drvif_scaler_vactive_end_irq(_ENABLE, Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)); // CSW+ 0970710 Open auto MA in unfreeze mode
	drvif_scaler_vactive_sta_irq(_ENABLE, Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)); // CSW+ 0970710 Open auto MA in unfreeze mode

}

void get_vdc_dump_addr(unsigned int *PhyAddr, unsigned int *VirStartAddr)
{

	*PhyAddr  = MemTag[MEMIDX_VDC].StartAddr;

	*VirStartAddr = get_scaler_memory_map_addr() + (MemTag[MEMIDX_VDC].StartAddr - AllocStartAddr);

	rtd_pr_scaler_memory_debug("dump addr = %x %x\n", *PhyAddr,*VirStartAddr);

}
#endif


#ifdef CONFIG_DUMP_MEMTAG
void drvif_memory_dump_MemTag(void)
{
	rtd_pr_scaler_memory_debug("*****Dump MemTag*****\n");
	unsigned char i;
	for ( i=0 ; i<MEMIDX_ALL ; ++i ) {
		rtd_pr_scaler_memory_debug("idx: %d, status:%d, start addr:%d, size:%d\n",
			i, MemTag[i].Status, MemTag[i].StartAddr, MemTag[i].Size);
	}
}
#endif

void drv_memory_set_3D_SBS_LR_swap(unsigned char lr_swap)
{

	unsigned int  TotalSize=Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_BLKSTEP);
	mdomain_cap_ddr_in1addr_RBUS ddr_in1addr_reg;
	mdomain_cap_ddr_in1_2ndaddr_RBUS ddr_in1_2ndaddr_reg;
	mdomain_cap_ddr_in1_3rdaddr_RBUS ddr_in1_3rdaddr_reg;
	mdomain_cap_capmain_sbs_vsd_cap_offset_RBUS capmain_sbs_vsd_cap_offset_reg;

	ddr_in1addr_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Addr_reg);
	ddr_in1_2ndaddr_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1_2ndAddr_reg);
	ddr_in1_3rdaddr_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg);
	capmain_sbs_vsd_cap_offset_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CapMain_SBS_VSD_CAP_OFFSET_reg);

	if(Get_PANEL_VFLIP_ENABLE() && Scaler_InputSrcGetMainChType() == _SRC_VO)
		lr_swap = !lr_swap;
	rtd_pr_scaler_memory_debug("drv_memory_set_3D_SBS_LR_swap , lr swap=%d\n",lr_swap);
	if(lr_swap==_TRUE)
	{
		ddr_in1addr_reg.in1_wr_mem_start=(MemTag[MEMIDX_MAIN].StartAddr>>3)+TotalSize;
		ddr_in1_2ndaddr_reg.in1_sec_bl=(MemTag[MEMIDX_MAIN_SEC].StartAddr>>3)+TotalSize;
		if(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_VFLIP3))
			ddr_in1_3rdaddr_reg.in1_third_bl=(MemTag[MEMIDX_MAIN_THIRD].StartAddr>>3)+TotalSize;
		rtd_pr_scaler_memory_debug("[3D]SBS LR main_1=%x,second_1=%x,third_1=%x\n",ddr_in1addr_reg.in1_wr_mem_start,ddr_in1_2ndaddr_reg.in1_sec_bl,ddr_in1_3rdaddr_reg.in1_third_bl);
		TotalSize=(~TotalSize)+1;
		capmain_sbs_vsd_cap_offset_reg.in1_lr_offset=TotalSize;
	}
	else
	{
		ddr_in1addr_reg.in1_wr_mem_start=MemTag[MEMIDX_MAIN].StartAddr>>3;
		ddr_in1_2ndaddr_reg.in1_sec_bl=MemTag[MEMIDX_MAIN_SEC].StartAddr>>3;
		if(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_VFLIP3))
			ddr_in1_3rdaddr_reg.in1_third_bl=MemTag[MEMIDX_MAIN_THIRD].StartAddr>>3;
		capmain_sbs_vsd_cap_offset_reg.in1_lr_offset=TotalSize;
	}
	WaitFor_DEN_STOP();
	IoReg_Write32(MDOMAIN_CAP_DDR_In1Addr_reg , ddr_in1addr_reg.regValue & 0x7ffffff0);
	IoReg_Write32(MDOMAIN_CAP_DDR_In1_2ndAddr_reg , ddr_in1_2ndaddr_reg.regValue & 0x7ffffff0);
	if(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_VFLIP3))
		IoReg_Write32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg , ddr_in1_3rdaddr_reg.regValue & 0x7ffffff0);
	IoReg_Write32(MDOMAIN_CAP_CapMain_SBS_VSD_CAP_OFFSET_reg, capmain_sbs_vsd_cap_offset_reg.regValue);

}

void drv_memory_set_3D_TB_or_FP_LR_swap(unsigned char lr_swap)
{
	unsigned int  TotalSize=Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_BLKSTEP);
	mdomain_disp_ddr_mainaddr_RBUS ddr_mainaddr_reg;
	mdomain_disp_ddr_main2ndaddr_RBUS ddr_main2ndaddr_reg;
	mdomain_disp_ddr_main3rdaddr_RBUS ddr_main3rdaddr_reg;
	mdomain_disp_ddr_mainblkstep_RBUS ddr_mainblkstep_reg;

	ddr_mainaddr_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainAddr_reg);
	ddr_main2ndaddr_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_Main2ndAddr_reg);
	ddr_main3rdaddr_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_Main3rdAddr_reg);
	ddr_mainblkstep_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainBlkStep_reg);

	if(lr_swap==_TRUE)
	{
		//rtd_pr_scaler_memory_debug("[13D]TT&B LR main_1=%x,second_1=%x,,third_1=%x,TotalSize=%x\n",ddr_mainaddr_reg.regValue,ddr_main2ndaddr_reg.regValue,ddr_main3rdaddr_reg.regValue,TotalSize);
		ddr_mainaddr_reg.main_read_addr=(MemDispStartAddr_3D[0]>>3)+TotalSize;
		ddr_main2ndaddr_reg.main_sec_block=(MemDispStartAddr_3D[1]>>3)+TotalSize;
		if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_VFLIP3))
			ddr_main3rdaddr_reg.main_third_block=(MemDispStartAddr_3D[2]>>3)+TotalSize;
		TotalSize=(~TotalSize)+1;
		ddr_mainblkstep_reg.main_block_step=TotalSize;
		//rtd_pr_scaler_memory_debug("[23D]TT&B LR main_1=%x,second_1=%x,,third_1=%x,TotalSize=%x\n",ddr_mainaddr_reg.regValue,ddr_main2ndaddr_reg.regValue,ddr_main3rdaddr_reg.regValue,TotalSize);
	}
	else
	{

		//rtd_pr_scaler_memory_debug("[33D]TT&B LR main_1=%x,second_1=%x,,third_1=%x,TotalSize=%x\n",ddr_mainaddr_reg.regValue,ddr_main2ndaddr_reg.regValue,ddr_main3rdaddr_reg.regValue,TotalSize);
		ddr_mainaddr_reg.main_read_addr=MemDispStartAddr_3D[0]>>3;
		ddr_main2ndaddr_reg.main_sec_block=MemDispStartAddr_3D[1]>>3;
		if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_VFLIP3))
			ddr_main3rdaddr_reg.main_third_block=MemDispStartAddr_3D[2]>>3;
		ddr_mainblkstep_reg.main_block_step=TotalSize;
		//rtd_pr_scaler_memory_debug("[43D]TT&B LR main_1=%x,second_1=%x,,third_1=%x,TotalSize=%x\n",ddr_mainaddr_reg.regValue,ddr_main2ndaddr_reg.regValue,ddr_main3rdaddr_reg.regValue,TotalSize);
	}
	rtd_pr_scaler_memory_debug("[3D]TT&B LR main_1=%x,second_1=%x,,third_1=%x,TotalSize=%x\n",ddr_mainaddr_reg.regValue,ddr_main2ndaddr_reg.regValue,ddr_main3rdaddr_reg.regValue,TotalSize);
	WaitFor_DEN_STOP();
	IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg , ddr_mainaddr_reg.regValue&0x7ffffff0);

	IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg , ddr_main2ndaddr_reg.regValue&0x7ffffff0);

	if ((Get_PANEL_VFLIP_ENABLE() && Scaler_DispGetStatus((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_VFLIP3)))
		IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg , (ddr_main3rdaddr_reg.regValue)&0x7ffffff0);

	ddr_mainblkstep_reg.main_block_step = (ddr_mainblkstep_reg.main_block_step % 2)? ddr_mainblkstep_reg.main_block_step + 1 : ddr_mainblkstep_reg.main_block_step;
	//bit 3 need to be 0. rbus rule
	IoReg_Write32(MDOMAIN_DISP_DDR_MainBlkStep_reg , ddr_mainblkstep_reg.regValue);

#ifndef ENABLE_SMOOTH_TOGGLE_PR3D
	drvif_memory_set_dbuffer_write();
#endif
}

void drvif_memory_set_3D_LR_swap(unsigned char lr_swap,unsigned char type_3d)
{
#ifndef UT_flag
	switch(type_3d)
	{
		case SLR_3D_SIDE_BY_SIDE_HALF:
			drv_memory_set_3D_SBS_LR_swap(lr_swap);
			break;
		case SLR_3D_TOP_AND_BOTTOM:
		case SLR_3D_FRAME_PACKING:
			drv_memory_set_3D_TB_or_FP_LR_swap(lr_swap);
			break;
		case SLR_3D_2D_CVT_3D:
			scaler_2Dcvt3D_LR_swap((bool)lr_swap);
			break;
		default:
			break;

	}
#endif
}



#ifdef ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE // SG 3D PANEL SUPPORT ALL SOURCE 3D MODE
void drvif_memory_set_3D_LR_swap_for_SG_frc_in_mDomain(unsigned char bInvMode)
{
	unsigned int Addr1 = Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MAIN_ADDR);
	unsigned int Addr2 = Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MAIN_SEC_ADDR);
	unsigned int Addr3= Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_MAIN_THIRD_ADDR);
	unsigned int rFrameOffset = (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_BLKSTEP) << 3);
	unsigned char ucLRseq;
	ucLRseq = Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_LR_SEQUENCE) ^ bInvMode;

	if(modestate_get_vo3d_in_framePacking_FRC_mode()){
		unsigned int LineStep, TotalSize;
		LineStep = memory_get_capture_size(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), MEMCAPTYPE_LINE);		// calculate capture line size
		TotalSize = LineStep * Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_MEM_ACT_LEN);
		rFrameOffset = TotalSize << 3;
	}

	// non-IDMA SG 3D mode L/R order need do invert
	if(ucLRseq == FALSE){
		IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg, (Addr1 + rFrameOffset)&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg, (Addr2 + rFrameOffset)&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg, (Addr3 + rFrameOffset)&0x7ffffff0);

		IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_1ST_Addr_reg, Addr1);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_2ND_Addr_reg, Addr2);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_3RD_Addr_reg, Addr3);
	}
	else{
		IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg, Addr1&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg, Addr2&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg, Addr3&0x7ffffff0);

		IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_1ST_Addr_reg, Addr1 + rFrameOffset);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_2ND_Addr_reg, Addr2 + rFrameOffset);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main_R_3RD_Addr_reg, Addr3 + rFrameOffset);
	}

	drvif_memory_set_dbuffer_write();
	return;
}
#endif



void drvif_memory_set_MVC_FRC_LR_swap(unsigned char bInvMode)
{
	unsigned char ucLRseq;
	mdomain_cap_ddr_in1ctrl_RBUS ddr_in1ctrl_reg;
	ucLRseq = Scaler_Disp3dGetInfo(SLR_MAIN_DISPLAY, SLR_DISP_3D_LR_SEQUENCE);

	ddr_in1ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Ctrl_reg);
	// bInvMode for MVC data FRC mode (bypass IDMA)
	ddr_in1ctrl_reg.in1_l_flag_inv = ucLRseq ^ bInvMode;
	IoReg_Write32(MDOMAIN_CAP_DDR_In1Ctrl_reg, ddr_in1ctrl_reg.regValue);

	rtd_pr_scaler_memory_debug("[SG] MVC FRC L/R Swap=%d\n", ucLRseq);

	return;
}

//USER:LewisLee DATE:2012/08/23
//to maje sure have set correct setting
//to prevent M domain setting error with video FW
void dvrif_memory_set_frc_style(UINT8 Display, UINT8 ucEnable)
{
	mdomain_disp_ddr_mainctrl_RBUS ddr_mainctrl_reg;
//	ddr_mainsubctrl_RBUS ddr_mainsubctrl_reg;

	if(SLR_MAIN_DISPLAY == Display)
	{
		ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);

		if(_ENABLE == ucEnable)
		{
			if(_DISABLE == ddr_mainctrl_reg.main_frc_style)
			{
				ddr_mainctrl_reg.main_frc_style = _ENABLE;
				IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg , ddr_mainctrl_reg.regValue);

				drvif_memory_set_dbuffer_write();
				rtd_pr_scaler_memory_debug("dvrif_memory_set_frc_style, set input fast than display\n");
			}
		}
		else// if(_DISABLE == ucEnable)
		{
			if(_ENABLE == ddr_mainctrl_reg.main_frc_style)
			{
				ddr_mainctrl_reg.main_frc_style = _DISABLE;
				IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg , ddr_mainctrl_reg.regValue);

				drvif_memory_set_dbuffer_write();
				rtd_pr_scaler_memory_debug("dvrif_memory_set_frc_style, set input slower than display\n");
			}
		}
	}
}

#if 0
//USER:LewisLee DATE:2012/08/16
//for some case, we need switch buffer by SW
//to fix frame tear
void drvif_memory_3D_mode_sw_buffer_swap_action(SCALER_DISP_CHANNEL ucDisplay, eMemCap_3D_Mode_SwBufferSwapState state)
{
	mdomain_disp_ddr_mainctrl_RBUS ddr_mainctrl_reg;
	mdomain_cap_ddr_in1ctrl_RBUS ddr_in1ctrl_reg;

	switch(state)
	{
		case MEM_3D_SWBUFFER_SWAP_CONDITION_CHECK:
			Scaler_Set_Enable_3D_Mode_3Buffer_SW_Switch_Condition(ucDisplay, Scaler_InputSrcGetMainChType());
		break;

		case MEM_3D_SWBUFFER_SWAP_ENABLE:
			if(_DISABLE == Scaler_Get_Enable_3D_Mode_3Buffer_SW_Switch_Condition())
				return;

#ifdef CONFIG_DUAL_CHANNEL
			if(SLR_MAIN_DISPLAY == ucDisplay)
			{
				ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
				ddr_mainctrl_reg.main_frc_style = _ENABLE;
				IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg, ddr_mainctrl_reg.regValue);
			}
			else// if(SLR_SUB_DISPLAY == ucDisplay)
			{
				IoReg_Mask32(MDOMAIN_DISP_DDR_SubCtrl_reg, ~_BIT3, _BIT3);
			}
#else //#ifdef CONFIG_DUAL_CHANNEL
			ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
			ddr_mainctrl_reg.main_frc_style = _ENABLE;
			IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg, ddr_mainctrl_reg.regValue);
#endif //#ifdef CONFIG_DUAL_CHANNEL

			ddr_in1ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Ctrl_reg);
			ddr_in1ctrl_reg.in1_v_flip_3buf_en = _DISABLE;
			ddr_in1ctrl_reg.in1_double_enable = _DISABLE;
			IoReg_Write32(MDOMAIN_CAP_DDR_In1Ctrl_reg, ddr_in1ctrl_reg.regValue);
		break;

		case MEM_3D_SWBUFFER_SWAP_DISABLE:
//			if(SLR_SUB_DISPLAY == ucDisplay)
//				return;

			if(_DISABLE == Scaler_Get_Enable_3D_Mode_3Buffer_SW_Switch_Condition())
				return;

#ifdef CONFIG_DUAL_CHANNEL
			if(SLR_MAIN_DISPLAY == ucDisplay)
			{
				dvrif_memory_set_frc_style(SLR_MAIN_DISPLAY, _DISABLE);
			}
			else// if(SLR_SUB_DISPLAY == ucDisplay)
			{
				IoReg_Mask32(MDOMAIN_DISP_DDR_SubCtrl_reg, ~_BIT3, 0);
			}
#else //#ifdef CONFIG_DUAL_CHANNEL
			dvrif_memory_set_frc_style(SLR_MAIN_DISPLAY, _DISABLE);
#endif //#ifdef CONFIG_DUAL_CHANNEL
		break;

		case MEM_3D_SWBUFFER_SWAP_UPDATE_INFO:
			if(_DISABLE == Scaler_Get_Enable_3D_Mode_3Buffer_SW_Switch_Condition())
				return;

			state_update_disp3d_info();
		break;

		default:
		break;
	}
}
#endif
//Will Add for smooth toggle.
unsigned int drvif_memory_get_memtag_startaddr(UINT8 index)
{
	return MemTag[index].StartAddr;

}

void drvif_memory_set_memtag_startaddr(unsigned int addr,UINT8 index)
{
	MemTag[index].StartAddr=addr;

}

unsigned long drvif_memory_get_memtag_virtual_startaddr(UINT8 index)
{
	return MemTag[index].Allocate_VirAddr;

}

unsigned int drvif_memory_get_3d_startaddr(UINT8 index)
{
	return MemDispStartAddr_3D[index];
}

void drvif_memory_set_3d_startaddr(unsigned int addr,UINT8 index)
{

	MemDispStartAddr_3D[index]=addr;
}

StructMemBlock* drvif_memory_get_MemTag_addr(UINT8 index)
{
	return &MemTag[index];
}

void drvif_memory_set_memtag_virtual_startaddr(unsigned long addr)
{
	MemTag[MEMIDX_MAIN_THIRD].Allocate_VirAddr = addr;

}

void drvif_memory_free_fixed_virtual_memory(void)
{
	if((drvif_memory_get_memtag_virtual_startaddr(MEMIDX_MAIN_THIRD) == 0) || (M_fixed_virtual_memory_size == 0)){
		rtd_pr_scaler_memory_debug("fixed virtual address is NULL, not to free!!\n");
		return;
	}
	dvr_unmap_memory((void *)drvif_memory_get_memtag_virtual_startaddr(MEMIDX_MAIN_THIRD), M_fixed_virtual_memory_size);
}

void drvif_memory_allocate_fixed_virtual_memory(void)
{
	MemTag[MEMIDX_MAIN_THIRD].Allocate_VirAddr = (unsigned long)dvr_remap_cached_memory(MemTag[MEMIDX_MAIN_THIRD].StartAddr, M_fixed_virtual_memory_size, __builtin_return_address(0));
}

void drvif_memory_set_mtag_info(UINT32 buffer_size, UINT8 idx){
	MemTag[idx].Allocate_StartAddr = MemTag[idx].StartAddr;
	MemTag[idx].Size = buffer_size;
	MemTag[idx].Status = ALLOCATED_FROM_PLI;
	if((idx == MEMIDX_MAIN_SEC) || (idx == MEMIDX_MAIN))
		MemTag[idx].Allocate_VirAddr = 0;//not use
}
#endif
unsigned int drvif_memory_get_fixed_virtual_memory_size(void)
{
	return M_fixed_virtual_memory_size;
}

void drvif_memory_set_fixed_virtual_memory_size(unsigned int size)
{

	M_fixed_virtual_memory_size = size;
}

#ifndef BUILD_QUICK_SHOW



//Mac new adding block mode related driver
//Crixus@20140808
void drvif_memory_block_mode_enable(bool enable)
{
	if(enable == TRUE){
		IoReg_SetBits(MDOMAIN_DISP_DDR_Main_BM_DMA_reg, _BIT0);
		rtd_pr_scaler_memory_debug("@@@@@Enable block mode@@@@@\n");
	}
	else{
		IoReg_ClearBits(MDOMAIN_DISP_DDR_Main_BM_DMA_reg, _BIT0);
		rtd_pr_scaler_memory_debug("@@@@@Disable block mode & run sequence mode@@@@@\n");
	}
}

void drvif_memory_block_mode_set_YC_index(unsigned int Y_index, unsigned int C_index)
{
	mdomain_disp_ddr_main_bm_dma_picset_RBUS disp_ddr_main_bm_dma_picset_reg;
	disp_ddr_main_bm_dma_picset_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_Main_BM_DMA_PICSET_reg);

	disp_ddr_main_bm_dma_picset_reg.y_ads_set1 = Y_index;
	disp_ddr_main_bm_dma_picset_reg.c_ads_set1 = C_index;

	IoReg_Write32(MDOMAIN_DISP_DDR_Main_BM_DMA_PICSET_reg, disp_ddr_main_bm_dma_picset_reg.regValue);
}

//the initial point is (0,0)
void drvif_memory_block_set_Y_offset(unsigned int Y_V_offset, unsigned int Y_H_offset, unsigned int Y_H_shift)
{
	mdomain_disp_ddr_main_bm_h_offset_RBUS disp_ddr_main_bm_h_offset_reg;
	mdomain_disp_ddr_main_bm_v_offset_RBUS disp_ddr_main_bm_v_offset_reg;

	disp_ddr_main_bm_h_offset_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_Main_BM_H_OFFSET_reg);
	disp_ddr_main_bm_v_offset_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_Main_BM_V_OFFSET_reg);

	//horizontal offset - Y
	disp_ddr_main_bm_h_offset_reg.y_h_offset = Y_H_offset;
	disp_ddr_main_bm_h_offset_reg.y_h_shift = Y_H_shift;

	//vertiacl offset - Y
	disp_ddr_main_bm_v_offset_reg.y_v_offset = Y_V_offset;

	IoReg_Write32(MDOMAIN_DISP_DDR_Main_BM_H_OFFSET_reg, disp_ddr_main_bm_h_offset_reg.regValue);
	IoReg_Write32(MDOMAIN_DISP_DDR_Main_BM_V_OFFSET_reg, disp_ddr_main_bm_v_offset_reg.regValue);
}

void drvif_memory_block_set_C_offset(unsigned int C_V_offset, unsigned int C_H_offset)
{
	mdomain_disp_ddr_main_bm_h_offset_RBUS disp_ddr_main_bm_h_offset_reg;
	mdomain_disp_ddr_main_bm_v_offset_RBUS disp_ddr_main_bm_v_offset_reg;

	disp_ddr_main_bm_h_offset_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_Main_BM_H_OFFSET_reg);
	disp_ddr_main_bm_v_offset_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_Main_BM_V_OFFSET_reg);

	//horizontal offset - C
	disp_ddr_main_bm_h_offset_reg.c_h_offset = C_H_offset;

	//vertiacl offset - C
	disp_ddr_main_bm_v_offset_reg.c_v_offset = C_V_offset;

	IoReg_Write32(MDOMAIN_DISP_DDR_Main_BM_H_OFFSET_reg, disp_ddr_main_bm_h_offset_reg.regValue);
	IoReg_Write32(MDOMAIN_DISP_DDR_Main_BM_V_OFFSET_reg, disp_ddr_main_bm_v_offset_reg.regValue);
}

#if 0//for M-domain block mode read verify
//DCU index
#include "../../../common_macarthur3/include/dc_sys_reg.h"
#define PAGE_WIDTH_SHIFT        8
#define PAGE_WIDTH             (1 << PAGE_WIDTH_SHIFT)
#define DCU_WIDTH_SCALE(width) (((width) + (PAGE_WIDTH - 1)) >> PAGE_WIDTH_SHIFT)

//#define PAGE_SHIFT             (PAGE_WIDTH_SHIFT + gPage_Size + 3)

void VBM_SetDCU (int index, int width_scale, int init_page, int x_offset, int y_offset, unsigned int *DCU_data)
{
	//rtd_pr_scaler_memory_debug("setDCU i %d ws %d p %d xy %d %d\n", index, width_scale, init_page, x_offset, y_offset);

  	unsigned int t0 = DC_PICT_SET_pict_width_scale (width_scale) |DC_PICT_SET_pict_init_page   (init_page) ;

  	unsigned int t1 =
    	DC_PICT_SET_OFFSET_pict_set_num      (index)
  	| DC_PICT_SET_OFFSET_pict_set_offset_x (x_offset >> 4)
  	| DC_PICT_SET_OFFSET_pict_set_offset_y (y_offset >> 2) ;

  	if ( (int)DCU_data == DC_PICT_SET_reg )
  	{
    		//VP_GetSwMutex(MUTEX_ID_DCU);
    		*(volatile unsigned int *)DC_PICT_SET_reg        = t0 ;
    		*(volatile unsigned int *)DC_PICT_SET_OFFSET_reg = t1 ;
    		//VP_PutSwMutex(MUTEX_ID_DCU);
  	}
  	else
  	{
    		DCU_data[0] = t0 ;
    		DCU_data[1] = t1 ;
  	}
}

int  VBM_GetDCUInitDataSeq (unsigned short width, unsigned int *pMemory, int index, unsigned int *DCU_data, int x_offset, int y_offset)
{
	//VBM_MEMORY_CHECK
  	unsigned int PAGE_SHIFT;
  	unsigned int gPage_Size;

  	/* check alignment */
  	//if ( (int)pMemory & ROUND_UP_DCU_SIZE )
    	//VBM_RETURN (VBM_ERR_MEMORY_UNALIGNED)
    	gPage_Size = DC_SYS_MISC_get_page_size(*(volatile unsigned int *)DC_SYS_MISC_reg);
	PAGE_SHIFT = (PAGE_WIDTH_SHIFT + gPage_Size + 3);

  	if (x_offset & 7) rtd_pr_scaler_memory_debug("X offset: %d should align to 8-bytes in VBM_GetDCUInitDataSeq().\n", x_offset);
  	if (y_offset & 3) rtd_pr_scaler_memory_debug("Y offset: %d should align to 4-bytes in VBM_GetDCUInitDataSeq().\n", y_offset);

  	VBM_SetDCU (index, DCU_WIDTH_SCALE(width), ((int)pMemory & 0x3FFFFFFF) >> PAGE_SHIFT, x_offset, y_offset, DCU_data);

  	//VBM_RETURN (VBM_SUCCESS)
}
#endif
#endif
eMemCapAccessType dvrif_memory_get_cap_mode(unsigned char display)
{
	if(display == SLR_MAIN_DISPLAY)
		return enable_M_cap_mode_main;
	else
		return enable_M_cap_mode_sub;
}

void dvrif_memory_set_cap_mode(unsigned char display, eMemCapAccessType Cap_type)
{
	if(display == SLR_MAIN_DISPLAY)
		enable_M_cap_mode_main = Cap_type;
	else
		enable_M_cap_mode_sub = Cap_type;
}
//Disable M-disp when reset M-domain=>To Fixed DTV change issue @ Crixus 20141126
void drv_memory_Main_Disp_Enable(bool enable){
	if(enable == TRUE){
		IoReg_Write32(/*display? MDOMAIN_DISP_DDR_SubCtrl_reg : */MDOMAIN_DISP_Disp_main_enable_reg,  _BIT0);
	}
	else{
		IoReg_Write32(/*display? MDOMAIN_DISP_DDR_SubCtrl_reg : */MDOMAIN_DISP_Disp_main_enable_reg,  0x0);
	}
}
unsigned char dvrif_memory_compression_get_enable(unsigned char display)
{
#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
	return FALSE;
#endif
	if(display == SLR_MAIN_DISPLAY)
		return enable_M_compression_main;
	else
		return enable_M_compression_sub;
}
void dvrif_memory_compression_set_enable(unsigned char display, unsigned char enable)
{
	if(display == SLR_MAIN_DISPLAY)
		enable_M_compression_main = enable;
	else
		enable_M_compression_sub = enable;
}

unsigned char dvrif_memory_get_compression_bits(unsigned char display)
{
	if(display == SLR_MAIN_DISPLAY)
		return M_compression_bits;
	else
		return Sub_M_compression_bits;
}
void dvrif_memory_set_compression_bits(unsigned char display, unsigned char comp_bits)
{
	if(display == SLR_MAIN_DISPLAY)
		M_compression_bits=comp_bits;
	else
		Sub_M_compression_bits=comp_bits;
}

unsigned char dvrif_memory_get_compression_mode(unsigned char display)
{
	if(display == SLR_MAIN_DISPLAY)
		return M_compression_mode;
	else
		return Sub_M_compression_mode;
}
void dvrif_memory_set_compression_mode(unsigned char display, unsigned char comp_mode)
{
	if(display == SLR_MAIN_DISPLAY)
		M_compression_mode = comp_mode;
	else
		Sub_M_compression_mode = comp_mode;
}
 #ifndef BUILD_QUICK_SHOW
void drvif_memory_compression_rpc(unsigned char display){

	M_DOMAIN_COMPRESSION_T *mdomain_compression;
	//unsigned int ulCount = 0;
	int ret;

	mdomain_compression = (M_DOMAIN_COMPRESSION_T *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_MDOMAIN_COMPRESSION_ENABLE);
	//ulCount = sizeof(M_DOMAIN_COMPRESSION_T) / sizeof(unsigned int);

	mdomain_compression->display = display;
	mdomain_compression->bOnOff = dvrif_memory_compression_get_enable(display);
	mdomain_compression->compbit = dvrif_memory_get_compression_bits(display);
	mdomain_compression->GameMode = 0;//keep 3-buffer mode. (smooth_toggle_game_mode_timeout_trigger_flag ? 0 : drv_memory_get_game_mode());
	mdomain_compression->RGB444Mode =Get_Val_vsc_run_pc_mode();
	mdomain_compression->LivezoomMode = FALSE;
	mdomain_compression->compmode = dvrif_memory_get_compression_mode(display);

	//change endian
	mdomain_compression->display = htonl(mdomain_compression->display);
	mdomain_compression->bOnOff = htonl(mdomain_compression->bOnOff);
	mdomain_compression->compbit = htonl(mdomain_compression->compbit);
	mdomain_compression->GameMode = htonl(mdomain_compression->GameMode);
	mdomain_compression->RGB444Mode = htonl(mdomain_compression->RGB444Mode);
	mdomain_compression->LivezoomMode = htonl(mdomain_compression->LivezoomMode);
	mdomain_compression->compmode = htonl(mdomain_compression->compmode);

	ret = Scaler_SendRPC(SCALERIOC_SET_MDOMAIN_COMPRESSION_ENABLE,0,0);
//	if (0 != (ret = Scaler_SendRPC(SCALERIOC_SET_MDOMAIN_COMPRESSION_ENABLE,0,0)))
	if (0 != ret)
	{
		rtd_pr_scaler_memory_debug("[MEMC]ret=%d, SCALERIOC_SET_MDOMAIN_COMPRESSION_ENABLE RPC fail !!!\n", ret);
	}
}
#endif
void dvrif_memory_comp_setting(UINT8 enable_compression, UINT16 comp_wid, UINT16 comp_len, UINT8 comp_ratio){
	//M-domain use PQC and PQDC from merlin3 @Crixus 20170515
	mdomain_cap_cap_pq_cmp_RBUS mdomain_cap_cap_pq_cmp_reg;
	mdomain_cap_cap_pq_cmp_pair_RBUS mdomain_cap_cap_pq_cmp_pair_reg;
	mdomain_cap_cap_pq_cmp_bit_RBUS mdomain_cap_cap_pq_cmp_bit_reg;
	mdomain_cap_cap_pq_cmp_enable_RBUS mdomain_cap_cap_pq_cmp_enable_reg;
	mdomain_cap_cap_pq_cmp_allocate_RBUS mdomain_cap_cap_pq_cmp_allocate_reg;
	mdomain_cap_cap_pq_cmp_poor_RBUS mdomain_cap_cap_pq_cmp_poor_reg;
	mdomain_cap_cap_pq_cmp_blk0_add0_RBUS mdomain_cap_cap_pq_cmp_blk0_add0_reg;
	mdomain_cap_cap_pq_cmp_blk0_add1_RBUS mdomain_cap_cap_pq_cmp_blk0_add1_reg;
	mdomain_cap_cap_pq_cmp_balance_RBUS mdomain_cap_cap_pq_cmp_balance_reg;
	mdomain_cap_cap_pq_cmp_smooth_RBUS mdomain_cap_cap_pq_cmp_smooth_reg;
	mdomain_cap_cap_pq_cmp_guarantee_RBUS mdomain_cap_cap_pq_cmp_guarantee_reg;


	mdomain_disp_dispm_main_pq_decmp_RBUS mdomain_disp_dispm_main_pq_decmp_reg;
	mdomain_disp_dispm_main_pq_decmp_pair_RBUS mdomain_disp_dispm_main_pq_decmp_pair_reg;
	mdomain_disp_dispm_main_pq_decmp_sat_en_RBUS mdomain_disp_dispm_main_pq_decmp_sat_en_reg;
	mdomain_disp_ddr_mainctrl_RBUS mdomain_disp_ddr_mainctrl_reg;
	unsigned int TotalSize = 0;
	extern unsigned char pc_mode_run_422(void);
	extern unsigned char pc_mode_run_444(void);

	rtd_pr_scaler_memory_debug("[crixus]enable_compression = %d, comp_wid = %d, comp_len = %d, comp_ratio = %d\n",enable_compression, comp_wid, comp_len, comp_ratio);

	if(enable_compression == TRUE){
		/*
			width need to align 32

			line sum bit = (width * compression bit + 256) / 128

			for new PQC and PQDC @Crixus 20170615
		*/

		if((comp_wid % 32) != 0){
			//drop bits
			IoReg_Write32(MDOMAIN_DISP_DDR_MainAddrDropBits_reg, (32 - (comp_wid % 32)));
			comp_wid = comp_wid + (32 - (comp_wid % 32));
		}


		if(dvrif_memory_get_compression_mode(SLR_MAIN_DISPLAY) == COMPRESSION_LINE_MODE){
			TotalSize = (comp_wid * comp_ratio + 256) * 10 / 128;
			if((TotalSize % 10) != 0){
				TotalSize = TotalSize / 10 + 1;
			}else{
				TotalSize = TotalSize / 10;
			}
		}
		//rtd_pr_scaler_memory_emerg("[crixus]comp_wid = %d, comp_wid/32 = %d\n", comp_wid, (comp_wid/32));
		//rtd_pr_scaler_memory_emerg("[crixus]comp_len = %d\n", comp_len);
		//rtd_pr_scaler_memory_emerg("[crixus]TotalSize = %d\n", TotalSize);


		/*=======================PQC Setting======================================*/

		//capture compression setting
		mdomain_cap_cap_pq_cmp_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_reg);
		mdomain_cap_cap_pq_cmp_reg.cmp_height = comp_len;//set length
		mdomain_cap_cap_pq_cmp_reg.cmp_width_div32 = comp_wid / 32;//set width
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_reg, mdomain_cap_cap_pq_cmp_reg.regValue);

		//compression bits setting
		mdomain_cap_cap_pq_cmp_bit_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_BIT_reg);
		if(dvrif_memory_get_compression_mode(SLR_MAIN_DISPLAY) == COMPRESSION_LINE_MODE){
			mdomain_cap_cap_pq_cmp_bit_reg.frame_limit_bit = comp_ratio;
			mdomain_cap_cap_pq_cmp_bit_reg.line_limit_bit = comp_ratio;
		}
		else{//frame mode, line limit bit need to add margin to avoid the peak bandwidth
			mdomain_cap_cap_pq_cmp_bit_reg.frame_limit_bit = comp_ratio;
			mdomain_cap_cap_pq_cmp_bit_reg.line_limit_bit = comp_ratio + M_DOMAIN_PQC_LINE_MARGIN;
		}
		mdomain_cap_cap_pq_cmp_bit_reg.block_limit_bit = 0x3f;//PQC fine tune setting
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_BIT_reg, mdomain_cap_cap_pq_cmp_bit_reg.regValue);

		//compression other setting
		mdomain_cap_cap_pq_cmp_pair_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_PAIR_reg);
		#if 0
		if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_10BIT) == TRUE)
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_bit_width = 1; //10 bits
		else
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_bit_width = 0;//8 bits
		#endif

		mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_bit_width = 1; //10 bits



		if ((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == FALSE)){
			if(pc_mode_run_422())
			{
				mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_format = 1; // 422
			}
			else if(pc_mode_run_444())
			{
				mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_format = 0; // 444
			}
			else
			{
				mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_format = 1; // 422
			}
		}
		else{
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_format = 1; // 422
		}

		mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_color = 1;

#if 0
		//if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == TRUE){
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_format = 0;// 444, let hw dynamic switch 444 or 422.
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_color = 1;
		//}else{
		//	mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_format = 0;//444
		//	mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_color = 1;//currently no case rgb channel
		//}
#endif
		if(dvrif_memory_get_compression_mode(SLR_MAIN_DISPLAY) == COMPRESSION_LINE_MODE){
			//use line mode
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_line_mode = 1;
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_line_sum_bit = TotalSize;
		}
		else{
			//use frame mode
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_line_mode = 0;
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_line_sum_bit = 0;
		}

		if(Get_PANEL_VFLIP_ENABLE())
			mdomain_cap_cap_pq_cmp_pair_reg.two_line_prediction_en = 0;
		else
			mdomain_cap_cap_pq_cmp_pair_reg.two_line_prediction_en = 1;

		mdomain_cap_cap_pq_cmp_pair_reg.cmp_qp_mode = 0;
		//mdomain_cap_cap_pq_cmp_pair_reg.cmp_dic_mode_en = 1;  
		mdomain_cap_cap_pq_cmp_pair_reg.cmp_jump4_en = 1;

		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_PAIR_reg, mdomain_cap_cap_pq_cmp_pair_reg.regValue);

		//compression PQC constrain @Crixus 20170626
		mdomain_cap_cap_pq_cmp_enable_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_ENABLE_reg);
		if(dvrif_memory_get_compression_mode(SLR_MAIN_DISPLAY) == COMPRESSION_LINE_MODE){
			mdomain_cap_cap_pq_cmp_enable_reg.fisrt_line_more_en = 0;
		}
		else{
			mdomain_cap_cap_pq_cmp_enable_reg.fisrt_line_more_en = 1;
		}
		//RGB444 setting for PQC @Crixus 20170718
		rtd_pr_scaler_memory_info("SLR_DISP_422CAP=%x, pc_mode_run_422=%x, pc_mode_run_444=%x\n",Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP), pc_mode_run_422(), pc_mode_run_444());
		if ((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == FALSE)){
			if(pc_mode_run_422())
			{
				mdomain_cap_cap_pq_cmp_enable_reg.g_ratio = 14;
				mdomain_cap_cap_pq_cmp_enable_reg.do_422_mode = 1;//422 format
			}
			else if(pc_mode_run_444())
			{
				mdomain_cap_cap_pq_cmp_enable_reg.g_ratio = 12;
				mdomain_cap_cap_pq_cmp_enable_reg.do_422_mode = 2;//444 format
			}
			else
			{
				mdomain_cap_cap_pq_cmp_enable_reg.g_ratio = 14;
				mdomain_cap_cap_pq_cmp_enable_reg.do_422_mode = 1;//422 format
			}
		}
		else{
			mdomain_cap_cap_pq_cmp_enable_reg.g_ratio = 14;
			mdomain_cap_cap_pq_cmp_enable_reg.do_422_mode = 1;//422 format
		}

		mdomain_cap_cap_pq_cmp_enable_reg.first_predict_nc_en = 1;
		mdomain_cap_cap_pq_cmp_enable_reg.blk0_add_en = 1;
		mdomain_cap_cap_pq_cmp_enable_reg.blk0_add_half_en = 1;
		mdomain_cap_cap_pq_cmp_enable_reg.add_last2blk_over_curve_bit = 1;
		mdomain_cap_cap_pq_cmp_enable_reg.balance_en = 1;
		mdomain_cap_cap_pq_cmp_enable_reg.variation_maxmin_en = 1;
		mdomain_cap_cap_pq_cmp_enable_reg.dynamic_allocate_ratio_en = 1;
		mdomain_cap_cap_pq_cmp_enable_reg.dynamic_sum_line_rst = 1;
		mdomain_cap_cap_pq_cmp_enable_reg.not_rich_do_422_en = 1;
		mdomain_cap_cap_pq_cmp_enable_reg.rb_lossy_do_422_en = 1;
		mdomain_cap_cap_pq_cmp_enable_reg.not_enough_bit_do_422_en = 1;
		mdomain_cap_cap_pq_cmp_enable_reg.poor_limit_min_qp_en = 1;
		mdomain_cap_cap_pq_cmp_enable_reg.first_predict_nc_mode = 1;

		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_ENABLE_reg, mdomain_cap_cap_pq_cmp_enable_reg.regValue);

		mdomain_cap_cap_pq_cmp_allocate_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_ALLOCATE_reg);
		//RGB444 setting for PQC @Crixus 20170718
		if ((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == FALSE)){
			if(pc_mode_run_422())
			{
				mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 16;
				mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 13;
			}
			else if(pc_mode_run_444())
			{
				mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 13;
				mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 9;
			}
			else
			{
				mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 16;
				mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 13;
			}
		}
		else{
			mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 16;
			mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 13;
		}

		mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_less = 4;
		mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_more = 4;
		mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_line = 4;
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_ALLOCATE_reg, mdomain_cap_cap_pq_cmp_allocate_reg.regValue);

		mdomain_cap_cap_pq_cmp_poor_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_POOR_reg);
		#if 0
		if(pc_mode_run_444()){
			mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp4=0x14;
			mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp3=0x06;
			mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp2=0x03;
			mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp1=0x03;
		} else {
			mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp4=0x14;
			mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp3=0x0c;
			mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp2=0x06;
			mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp1=0x03;
		}

		if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) && (!vsc_get_adaptivestream_flag(SLR_MAIN_DISPLAY)))
		{
			mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp4=0x13; // for issue KTASKWBS-10111 for normal VDEC
		}
		#endif
		mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp4=12;
		mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp3=9;
		mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp2=4;
		mdomain_cap_cap_pq_cmp_poor_reg.poor_limit_th_qp1=3;

		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_POOR_reg, mdomain_cap_cap_pq_cmp_poor_reg.regValue);

		//Update PQC constrain setting for PQC @Crixus 20170725
		mdomain_cap_cap_pq_cmp_blk0_add0_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_BLK0_ADD0_reg);
		mdomain_cap_cap_pq_cmp_blk0_add1_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_BLK0_ADD1_reg);
		if(comp_ratio == COMPRESSION_10_BITS){// minmal compression bit setting
			mdomain_cap_cap_pq_cmp_blk0_add0_reg.blk0_add_value32 = 1;
			mdomain_cap_cap_pq_cmp_blk0_add0_reg.blk0_add_value16 = 1;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value8 = 1;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value4 = 1;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value2 = 1;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value1 = 0;
			mdomain_cap_cap_pq_cmp_blk0_add0_reg.dic_mode_tolerance = 0;
		}
		else{
			mdomain_cap_cap_pq_cmp_blk0_add0_reg.blk0_add_value32 = 16;
			mdomain_cap_cap_pq_cmp_blk0_add0_reg.blk0_add_value16 = 8;
			mdomain_cap_cap_pq_cmp_blk0_add0_reg.blk0_add_value0 = 0;
			mdomain_cap_cap_pq_cmp_blk0_add0_reg.dic_mode_tolerance = 0;

			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value8 = 4;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value4 = 2;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value2 = 1;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value1 = 1;
		}
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_BLK0_ADD0_reg, mdomain_cap_cap_pq_cmp_blk0_add0_reg.regValue);
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_BLK0_ADD1_reg, mdomain_cap_cap_pq_cmp_blk0_add1_reg.regValue);


		mdomain_cap_cap_pq_cmp_balance_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_BALANCE_reg);
		mdomain_cap_cap_pq_cmp_balance_reg.balance_rb_ov_th = 3;
		mdomain_cap_cap_pq_cmp_balance_reg.balance_rb_ud_th = 0;
		mdomain_cap_cap_pq_cmp_balance_reg.balance_rb_give = 3;
		mdomain_cap_cap_pq_cmp_balance_reg.balance_g_ud_th = 3;
		mdomain_cap_cap_pq_cmp_balance_reg.balance_g_ov_th = 0;
		mdomain_cap_cap_pq_cmp_balance_reg.balance_g_give = 3;
		mdomain_cap_cap_pq_cmp_balance_reg.variation_maxmin_th = 30;
		mdomain_cap_cap_pq_cmp_balance_reg.variation_maxmin_th2 = 3;
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_BALANCE_reg, mdomain_cap_cap_pq_cmp_balance_reg.regValue);


		mdomain_cap_cap_pq_cmp_smooth_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_SMOOTH_reg);
		mdomain_cap_cap_pq_cmp_smooth_reg.variation_near_num_th = 4;
		mdomain_cap_cap_pq_cmp_smooth_reg.variation_value_th = 3;
		mdomain_cap_cap_pq_cmp_smooth_reg.variation_num_th = 3;
		mdomain_cap_cap_pq_cmp_smooth_reg.not_enough_bit_do_422_qp = 5;
		mdomain_cap_cap_pq_cmp_smooth_reg.not_rich_do_422_th = 4;
		mdomain_cap_cap_pq_cmp_smooth_reg.rb_lossy_do_422_qp_level = 0;
		mdomain_cap_cap_pq_cmp_smooth_reg.dic_mode_entry_th = 15;

		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_SMOOTH_reg, mdomain_cap_cap_pq_cmp_smooth_reg.regValue);

		mdomain_cap_cap_pq_cmp_guarantee_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_Guarantee_reg);
		mdomain_cap_cap_pq_cmp_guarantee_reg.dic_mode_qp_th_g = 1;
		mdomain_cap_cap_pq_cmp_guarantee_reg.dic_mode_qp_th_rb = 1;
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_Guarantee_reg, mdomain_cap_cap_pq_cmp_guarantee_reg.regValue);

		/*=======================PQDC Setting======================================*/


		//display de-compression setting
		mdomain_disp_dispm_main_pq_decmp_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_reg);
		mdomain_disp_dispm_main_pq_decmp_reg.decmp_height = comp_len;
		mdomain_disp_dispm_main_pq_decmp_reg.decmp_width_div32 = comp_wid / 32;
		IoReg_Write32(MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_reg, mdomain_disp_dispm_main_pq_decmp_reg.regValue);

		//de-compression bits setting
		mdomain_disp_dispm_main_pq_decmp_pair_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_PAIR_reg);
		#if 0
		if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_10BIT) == TRUE)
			mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_data_bit_width = 1;//10 bits
		else
			mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_data_bit_width = 0;//8 bits
		#endif

		mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_data_bit_width = 1;//10 bits

		#if 0
		//decompression YUV444 @Crixus 20160902
		//if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == TRUE){
			mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_data_format = 0;//444
			mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_data_color = 1;
		//}else{
		//	mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_data_format = 0;//444
		//	mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_data_color = 1;
		//}
		#endif

		if ((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == FALSE)){
			if(pc_mode_run_422())
			{
				mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_data_format = 1; //422
			}
			else if(pc_mode_run_444())
			{
				mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_data_format = 0;//444
			}
			else
			{
				mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_data_format = 1; //422
			}
		}
		else{
			mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_data_format = 1; //422
		}

		mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_data_color = 1;

		if(dvrif_memory_get_compression_mode(SLR_MAIN_DISPLAY) == COMPRESSION_LINE_MODE){
			//default use line mode
			mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_line_mode = 1;
			mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_line_sum_bit = TotalSize;
		}
		else{
			//use frame mode
			mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_line_mode = 0;
			mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_line_sum_bit = 0;
		}

		if(Get_PANEL_VFLIP_ENABLE())
			mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_two_line_prediction_en = 0;
		else
			mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_two_line_prediction_en = 1;

		mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_qp_mode = 0;
		//mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_dic_mode_en = 1;
		mdomain_disp_dispm_main_pq_decmp_pair_reg.decmp_jump4_en = 1;

		IoReg_Write32(MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_PAIR_reg, mdomain_disp_dispm_main_pq_decmp_pair_reg.regValue);

		//enable saturation for PQ
		mdomain_disp_dispm_main_pq_decmp_sat_en_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_SAT_EN_reg);
		mdomain_disp_dispm_main_pq_decmp_sat_en_reg.saturation_en = 1;
		IoReg_Write32(MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_SAT_EN_reg, mdomain_disp_dispm_main_pq_decmp_sat_en_reg.regValue);

		//enable clock
		mdomain_disp_ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
		mdomain_disp_ddr_mainctrl_reg.main_decompression_clk_en = 1;
		IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg, mdomain_disp_ddr_mainctrl_reg.regValue);

		//enable compression
		mdomain_cap_cap_pq_cmp_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_reg);
		mdomain_cap_cap_pq_cmp_reg.cmp_en = 1;
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_reg, mdomain_cap_cap_pq_cmp_reg.regValue);

		//enable de-compression
		mdomain_disp_dispm_main_pq_decmp_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_reg);
		mdomain_disp_dispm_main_pq_decmp_reg.decmp_en = 1;
		IoReg_Write32(MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_reg, mdomain_disp_dispm_main_pq_decmp_reg.regValue);
	}
	else{
		//disable compression
		mdomain_cap_cap_pq_cmp_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_reg);
		mdomain_cap_cap_pq_cmp_reg.cmp_en = 0;
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_reg, mdomain_cap_cap_pq_cmp_reg.regValue);

		//disable de-compression
		mdomain_disp_dispm_main_pq_decmp_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_reg);
		mdomain_disp_dispm_main_pq_decmp_reg.decmp_en = 0;
		IoReg_Write32(MDOMAIN_DISP_DISPM_MAIN_PQ_DECMP_reg, mdomain_disp_dispm_main_pq_decmp_reg.regValue);

		//disable clock
		//mdomain_disp_ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
		//mdomain_disp_ddr_mainctrl_reg.main_decompression_clk_en = 0;
		//IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg, mdomain_disp_ddr_mainctrl_reg.regValue);
	}
}


void dvrif_memory_comp_setting_sub(UINT8 enable_compression, UINT16 comp_wid, UINT16 comp_len, UINT8 comp_ratio)
{
#if 0
#ifdef CONFIG_DUAL_CHANNEL
	mdomain_cap_caps_pq_cmp_RBUS mdomain_cap_caps_pq_cmp_reg;
	mdomain_cap_caps_pq_cmp_pair_RBUS mdomain_cap_caps_pq_cmp_pair_reg;
	mdomain_cap_caps_pq_cmp_bit_RBUS mdomain_cap_caps_pq_cmp_bit_reg;
	mdomain_cap_caps_pq_cmp_enable_RBUS mdomain_cap_caps_pq_cmp_enable_reg;
	mdomain_cap_caps_pq_cmp_allocate_RBUS mdomain_cap_caps_pq_cmp_allocate_reg;
	mdomain_cap_caps_pq_cmp_poor_RBUS mdomain_cap_caps_pq_cmp_poor_reg;
	mdomain_cap_caps_pq_cmp_blk0_add0_RBUS mdomain_cap_caps_pq_cmp_blk0_add0_reg;
	mdomain_cap_caps_pq_cmp_blk0_add1_RBUS mdomain_cap_caps_pq_cmp_blk0_add1_reg;


	mdomain_disp_dispm_sub_pq_decmp_RBUS mdomain_disp_dispm_sub_pq_decmp_reg;
	mdomain_disp_dispm_sub_pq_decmp_pair_RBUS mdomain_disp_dispm_sub_pq_decmp_pair_reg;
	mdomain_disp_dispm_sub_pq_decmp_sat_en_RBUS mdomain_disp_dispm_sub_pq_decmp_sat_en_reg;
	mdomain_disp_ddr_subctrl_RBUS mdomain_disp_ddr_subctrl_reg;

	unsigned int TotalSize = 0;
	extern unsigned char pc_mode_run_422(void);
	extern unsigned char pc_mode_run_444(void);

	rtd_pr_scaler_memory_debug("[crixus]enable_compression = %d, comp_wid = %d, comp_len = %d, comp_ratio = %d\n",enable_compression, comp_wid, comp_len, comp_ratio);

	if(enable_compression == TRUE){
		/*
			width need to align 32

			line sum bit = (width * compression bit + 256) / 128

			for new PQC and PQDC @Crixus 20170615
		*/

		if((comp_wid % 32) != 0){
			//drop bits
			IoReg_Write32(MDOMAIN_DISP_DDR_SubAddrDropBits_reg, (32 - (comp_wid % 32)));
			comp_wid = comp_wid + (32 - (comp_wid % 32));
		}


		if(dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) == COMPRESSION_LINE_MODE){
			TotalSize = (comp_wid * comp_ratio + 256) * 10 / 128;
			if((TotalSize % 10) != 0){
				TotalSize = TotalSize / 10 + 1;
			}else{
				TotalSize = TotalSize / 10;
			}
		}
		//rtd_pr_scaler_memory_emerg("[crixus]comp_wid = %d, comp_wid/32 = %d\n", comp_wid, (comp_wid/32));
		//rtd_pr_scaler_memory_emerg("[crixus]comp_len = %d\n", comp_len);
		//rtd_pr_scaler_memory_emerg("[crixus]TotalSize = %d\n", TotalSize);


		/*=======================PQC Setting======================================*/

		//capture compression setting
		mdomain_cap_caps_pq_cmp_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_reg);
		mdomain_cap_caps_pq_cmp_reg.s_cmp_height = comp_len;//set length
		mdomain_cap_caps_pq_cmp_reg.s_cmp_width_div32 = comp_wid / 32;//set width
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_reg, mdomain_cap_caps_pq_cmp_reg.regValue);

		//compression bits setting
		mdomain_cap_caps_pq_cmp_bit_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_reg);
		if(dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) == COMPRESSION_LINE_MODE){
			mdomain_cap_caps_pq_cmp_bit_reg.s_frame_limit_bit = comp_ratio;
			mdomain_cap_caps_pq_cmp_bit_reg.s_line_limit_bit = comp_ratio;
		}
		else{//frame mode, line limit bit need to add margin to avoid the peak bandwidth
			mdomain_cap_caps_pq_cmp_bit_reg.s_frame_limit_bit = comp_ratio;
			mdomain_cap_caps_pq_cmp_bit_reg.s_line_limit_bit = comp_ratio + M_DOMAIN_PQC_LINE_MARGIN;
		}
		mdomain_cap_caps_pq_cmp_bit_reg.s_block_limit_bit = 0x3f;//PQC fine tune setting
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_reg, mdomain_cap_caps_pq_cmp_bit_reg.regValue);

		//compression other setting
		mdomain_cap_caps_pq_cmp_pair_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_PAIR_reg);
		if(Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_10BIT) == TRUE)
			mdomain_cap_caps_pq_cmp_pair_reg.s_cmp_data_bit_width = 1; //10 bits
		else
			mdomain_cap_caps_pq_cmp_pair_reg.s_cmp_data_bit_width = 0;//8 bits

		//if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == TRUE){
			mdomain_cap_caps_pq_cmp_pair_reg.s_cmp_data_format = 0;// 444, let hw dynamic switch 444 or 422.
			mdomain_cap_caps_pq_cmp_pair_reg.s_cmp_data_color = 1;
		//}else{
		//	mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_format = 0;//444
		//	mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_color = 1;//currently no case rgb channel
		//}
		if(dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) == COMPRESSION_LINE_MODE){
			//use line mode
			mdomain_cap_caps_pq_cmp_pair_reg.s_cmp_line_mode = 1;
			mdomain_cap_caps_pq_cmp_pair_reg.s_cmp_line_sum_bit = TotalSize;
		}
		else{
			//use frame mode
			mdomain_cap_caps_pq_cmp_pair_reg.s_cmp_line_mode = 0;
			mdomain_cap_caps_pq_cmp_pair_reg.s_cmp_line_sum_bit = 0;
		}

		if(Get_PANEL_VFLIP_ENABLE())
			mdomain_cap_caps_pq_cmp_pair_reg.s_two_line_prediction_en = 0;
		else
			mdomain_cap_caps_pq_cmp_pair_reg.s_two_line_prediction_en = 1;

		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_PAIR_reg, mdomain_cap_caps_pq_cmp_pair_reg.regValue);

		//compression PQC constrain @Crixus 20170626
		mdomain_cap_caps_pq_cmp_enable_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_ENABLE_reg);
		if(dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) == COMPRESSION_LINE_MODE){
			mdomain_cap_caps_pq_cmp_enable_reg.s_fisrt_line_more_en = 0;
		}
		else{
			mdomain_cap_caps_pq_cmp_enable_reg.s_fisrt_line_more_en = 1;
		}
		//RGB444 setting for PQC @Crixus 20170718
		if ((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == FALSE)){
			if(pc_mode_run_422())
			{
				mdomain_cap_caps_pq_cmp_enable_reg.s_g_ratio = 14;
				mdomain_cap_caps_pq_cmp_enable_reg.s_do_422_mode = 1;//422 format
			}
			else
			{
				mdomain_cap_caps_pq_cmp_enable_reg.s_g_ratio = 12;
				mdomain_cap_caps_pq_cmp_enable_reg.s_do_422_mode = 2;//444 format
			}
		}
		else{
			mdomain_cap_caps_pq_cmp_enable_reg.s_g_ratio = 14;
			mdomain_cap_caps_pq_cmp_enable_reg.s_do_422_mode = 0;//dynamic format
		}

		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_ENABLE_reg, mdomain_cap_caps_pq_cmp_enable_reg.regValue);

		mdomain_cap_caps_pq_cmp_allocate_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_ALLOCATE_reg);
		//RGB444 setting for PQC @Crixus 20170718
		if ((Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP) == FALSE)){
			if(pc_mode_run_422())
			{
				mdomain_cap_caps_pq_cmp_allocate_reg.s_dynamic_allocate_ratio_max = 16;
				mdomain_cap_caps_pq_cmp_allocate_reg.s_dynamic_allocate_ratio_min = 13;
			}
			else
			{
				mdomain_cap_caps_pq_cmp_allocate_reg.s_dynamic_allocate_ratio_max = 13;
				mdomain_cap_caps_pq_cmp_allocate_reg.s_dynamic_allocate_ratio_min = 9;
			}
		}
		else{
			mdomain_cap_caps_pq_cmp_allocate_reg.s_dynamic_allocate_ratio_max = 16;
			mdomain_cap_caps_pq_cmp_allocate_reg.s_dynamic_allocate_ratio_min = 13;
		}
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_ALLOCATE_reg, mdomain_cap_caps_pq_cmp_allocate_reg.regValue);

		mdomain_cap_caps_pq_cmp_poor_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_POOR_reg);
		if(pc_mode_run_444()){
			mdomain_cap_caps_pq_cmp_poor_reg.s_poor_limit_th_qp4=0x14;
			mdomain_cap_caps_pq_cmp_poor_reg.s_poor_limit_th_qp3=0x06;
			mdomain_cap_caps_pq_cmp_poor_reg.s_poor_limit_th_qp2=0x03;
			mdomain_cap_caps_pq_cmp_poor_reg.s_poor_limit_th_qp1=0x03;
		} else {
			mdomain_cap_caps_pq_cmp_poor_reg.s_poor_limit_th_qp4=0x14;
			mdomain_cap_caps_pq_cmp_poor_reg.s_poor_limit_th_qp3=0x0c;
			mdomain_cap_caps_pq_cmp_poor_reg.s_poor_limit_th_qp2=0x06;
			mdomain_cap_caps_pq_cmp_poor_reg.s_poor_limit_th_qp1=0x03;
		}
		if((Get_DisplayMode_Src(SLR_SUB_DISPLAY) == VSC_INPUTSRC_VDEC) && (!vsc_get_adaptivestream_flag(SLR_SUB_DISPLAY)))
		{
			mdomain_cap_caps_pq_cmp_poor_reg.s_poor_limit_th_qp4=0x13; // for issue KTASKWBS-10111 for normal VDEC
		}
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_POOR_reg, mdomain_cap_caps_pq_cmp_poor_reg.regValue);

		//Update PQC constrain setting for PQC @Crixus 20170725
		mdomain_cap_caps_pq_cmp_blk0_add0_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_BLK0_ADD0_reg);
		mdomain_cap_caps_pq_cmp_blk0_add1_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_BLK0_ADD1_reg);
		if(comp_ratio == COMPRESSION_10_BITS){// minmal compression bit setting
			mdomain_cap_caps_pq_cmp_blk0_add0_reg.s_blk0_add_value32 = 1;
			mdomain_cap_caps_pq_cmp_blk0_add0_reg.s_blk0_add_value16 = 1;
			mdomain_cap_caps_pq_cmp_blk0_add1_reg.s_blk0_add_value8 = 1;
			mdomain_cap_caps_pq_cmp_blk0_add1_reg.s_blk0_add_value4 = 1;
			mdomain_cap_caps_pq_cmp_blk0_add1_reg.s_blk0_add_value2 = 1;
			mdomain_cap_caps_pq_cmp_blk0_add1_reg.s_blk0_add_value1 = 0;
		}
		else{
			mdomain_cap_caps_pq_cmp_blk0_add0_reg.s_blk0_add_value32 = 16;
			mdomain_cap_caps_pq_cmp_blk0_add0_reg.s_blk0_add_value16 = 8;
			mdomain_cap_caps_pq_cmp_blk0_add1_reg.s_blk0_add_value8 = 4;
			mdomain_cap_caps_pq_cmp_blk0_add1_reg.s_blk0_add_value4 = 2;
			mdomain_cap_caps_pq_cmp_blk0_add1_reg.s_blk0_add_value2 = 1;
			mdomain_cap_caps_pq_cmp_blk0_add1_reg.s_blk0_add_value1 = 0;
		}
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_BLK0_ADD0_reg, mdomain_cap_caps_pq_cmp_blk0_add0_reg.regValue);
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_BLK0_ADD1_reg, mdomain_cap_caps_pq_cmp_blk0_add1_reg.regValue);


		/*=======================Sub PQDC Setting======================================*/

		//display de-compression setting
		mdomain_disp_dispm_sub_pq_decmp_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_reg);
		mdomain_disp_dispm_sub_pq_decmp_reg.s_decmp_height = comp_len;
		mdomain_disp_dispm_sub_pq_decmp_reg.s_decmp_width_div32 = comp_wid / 32;
		IoReg_Write32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_reg, mdomain_disp_dispm_sub_pq_decmp_reg.regValue);

		//de-compression bits setting
		mdomain_disp_dispm_sub_pq_decmp_pair_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_PAIR_reg);
		if(Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_10BIT) == TRUE)
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.s_decmp_data_bit_width = 1;//10 bits
		else
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.s_decmp_data_bit_width = 0;//8 bits
		//decompression YUV444 @Crixus 20160902
		//if(Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP) == TRUE){
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.s_decmp_data_format = 0;//444
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.s_decmp_data_color = 1;
		/*}else{
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.decmp_data_format = 0;//444
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.decmp_data_color = 1;
		}*/

		if(dvrif_memory_get_compression_mode(SLR_SUB_DISPLAY) == COMPRESSION_LINE_MODE){
			//default use line mode
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.s_decmp_line_mode = 1;
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.s_decmp_line_sum_bit = TotalSize;
		}
		else{
			//use frame mode
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.s_decmp_line_mode = 0;
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.s_decmp_line_sum_bit = 0;
		}
		IoReg_Write32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_PAIR_reg, mdomain_disp_dispm_sub_pq_decmp_pair_reg.regValue);

		//enable saturation for PQ
		mdomain_disp_dispm_sub_pq_decmp_sat_en_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_SAT_EN_reg);
		mdomain_disp_dispm_sub_pq_decmp_sat_en_reg.s_saturation_en = 1;
		IoReg_Write32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_SAT_EN_reg, mdomain_disp_dispm_sub_pq_decmp_sat_en_reg.regValue);

		//enable clock
		mdomain_disp_ddr_subctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubCtrl_reg);
		mdomain_disp_ddr_subctrl_reg.sub_decompression_clk_en = 1;
		IoReg_Write32(MDOMAIN_DISP_DDR_SubCtrl_reg, mdomain_disp_ddr_subctrl_reg.regValue);

		//enable compression
		mdomain_cap_caps_pq_cmp_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_reg);
		mdomain_cap_caps_pq_cmp_reg.s_cmp_en = 1;
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_reg, mdomain_cap_caps_pq_cmp_reg.regValue);

		//enable de-compression
		mdomain_disp_dispm_sub_pq_decmp_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_reg);
		mdomain_disp_dispm_sub_pq_decmp_reg.s_decmp_en = 1;
		IoReg_Write32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_reg, mdomain_disp_dispm_sub_pq_decmp_reg.regValue);

	}
	else{
		//disable compression & de-compression
		//disable compression
		mdomain_cap_caps_pq_cmp_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_reg);
		mdomain_cap_caps_pq_cmp_reg.s_cmp_en = 0;
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_reg, mdomain_cap_caps_pq_cmp_reg.regValue);

		//disable de-compression
		mdomain_disp_dispm_sub_pq_decmp_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_reg);
		mdomain_disp_dispm_sub_pq_decmp_reg.s_decmp_en = 0;
		IoReg_Write32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_reg, mdomain_disp_dispm_sub_pq_decmp_reg.regValue);

		//disable clock
		mdomain_disp_ddr_subctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubCtrl_reg);
		mdomain_disp_ddr_subctrl_reg.sub_decompression_clk_en = 0;
		IoReg_Write32(MDOMAIN_DISP_DDR_SubCtrl_reg, mdomain_disp_ddr_subctrl_reg.regValue);
	}
#endif
#endif
}


unsigned int di_buffer_alloc_size,di_buffer_size;
#define DI_422_10BIT_BUFFER_SIZE (3840 * 2160 * 20 / 8)
#define DI_420_10BIT_BUFFER_SIZE (3840 * 2160 * 15 / 8)
#define DI_422_8BIT_BUFFER_SIZE (3840 * 2160 * 16 / 8)
#define DI_420_8BIT_BUFFER_SIZE (3840 * 2160 * 12 / 8)

unsigned int drv_memory_di_alloc_buffer_get(void)
{
	return di_buffer_alloc_size;
}

void drv_memory_di_alloc_buffer_set(unsigned char source, unsigned char data_10bit)
{
	di_buffer_size = DI_422_10BIT_BUFFER_SIZE;
	di_buffer_alloc_size = di_buffer_size+512;

#if 0
	if (source == _SRC_VO) {
		di_buffer_alloc_size=DI_420_10BIT_BUFFER_SIZE+512;
		di_buffer_size=DI_420_10BIT_BUFFER_SIZE;
	} else {
		di_buffer_alloc_size=DI_422_10BIT_BUFFER_SIZE+512;
		di_buffer_size=DI_422_10BIT_BUFFER_SIZE;
	}
#endif
	return;
}
unsigned int drv_memory_di_buffer_get(void)
{
	return di_buffer_size;
}

unsigned int drv_memory_di_data_size_get(unsigned char access,unsigned int source_width)
{
	unsigned int buffer_size;
	if(access==SLR_DI_progress_mode) {
		buffer_size=di_buffer_size;
	} else {
		buffer_size=0x280000;	// (1920*1080/2)*20/8+n
	}
	return buffer_size;
}
unsigned int drv_memory_di_info_size_get(unsigned char access)
{
	unsigned int buffer_size;
	if(access==SLR_DI_progress_mode) {
		buffer_size=0;
	} else {
		buffer_size=0x200000;		// (1920*1080/2)*16/8+n
	}
	return buffer_size;
}

#ifdef Mer2_MEMORY_ALLOC
#define I2R_Index_NUM 2
unsigned char DI_Compression_Bit_Num[I2R_Index_NUM] = {0};
unsigned char DI_Compression_Mode[I2R_Index_NUM] = {0};
char drv_memory_di_Set_Compression_Bit_Num(unsigned char Bit_Num, unsigned char index)
{
	if (index >= I2R_Index_NUM)
		index = 0;

	DI_Compression_Bit_Num[index] = Bit_Num; /* 0 = disable compression mode*/
	return 0;
}

unsigned char drv_memory_di_Get_Compression_Bit_Num(unsigned char index)
{
	if (index >= I2R_Index_NUM)
		index = 0;

	return DI_Compression_Bit_Num[index];
}

static unsigned char VIP_PQ_DI_RTNR_PQC_Mode_TBL[VIP_PQ_DI_RTNR_PQC_Mode_Timing_Max] = 
{
	/* DI_RTNR_PQC_3A */		PQC_Disable,	
	/* DI_RTNR_PQC_5A, */		PQC_Disable,
	/* DI_RTNR_PQC_SD, */		PQC_Disable,
	/* DI_RTNR_PQC_720p, */		PQC_Disable,
	/* DI_RTNR_PQC_1080p, */	PQC_Frame_Mode,
	/* DI_RTNR_PQC_4k, */		PQC_Line_Mode_Rolling,
	/* DI_RTNR_PQC_8k, */		PQC_Line_Mode_Rolling,

};



char drv_memory_di_Set_Compression_Mode(unsigned char Mode, unsigned char index)
{
	if (Mode >= VIP_DI_PQC_MODE_MAX)
		Mode = PQC_Disable;

	if (index >= I2R_Index_NUM)
		index = 0;

	DI_Compression_Mode[index] = Mode; /* 0 = disable compression mode*/
	return 0;

}

unsigned char drv_memory_di_Get_Compression_Mode(unsigned char index)
{
	if (index >= I2R_Index_NUM)
		index = 0;

	return DI_Compression_Mode[index];
}
#endif
#ifdef Mer2_MEMORY_ALLOC
char drv_memory_di_Set_PQC_init(void)
{
	_RPC_system_setting_info* RPC_system_info_structure_table;

	RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if(RPC_system_info_structure_table == NULL){
		rtd_pr_scaler_memory_emerg("drv_memory_di_Set_PQC_init, NULL table \n");
		return -1;
	}

	//set PQC initial value
	RPC_system_info_structure_table->PQ_DI_RTNR_PQC_CTRL.cmp_alpha_en = 0;

	//RPC_system_info_structure_table->PQ_DI_RTNR_PQC_CTRL.PQC_Mode = PQC_Line_Mode_Rolling;
	memcpy(&RPC_system_info_structure_table->PQ_DI_RTNR_PQC_CTRL.PQC_Mode[0], &VIP_PQ_DI_RTNR_PQC_Mode_TBL[0], sizeof(char)*VIP_PQ_DI_RTNR_PQC_Mode_Timing_Max);
	return 0;
}
unsigned char drv_memory_di_Decide_PQC_alpha_En(void)
{
	unsigned char en_flag;
	_RPC_system_setting_info* RPC_system_info_structure_table;

	RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if(RPC_system_info_structure_table == NULL){
		rtd_pr_scaler_memory_emerg("drv_memory_di_Decide_PQC_mode, NULL table \n");
		return 0;
	}

	en_flag = RPC_system_info_structure_table->PQ_DI_RTNR_PQC_CTRL.cmp_alpha_en;

	return en_flag;
}

unsigned char drv_memory_get_di_PQC_timing(unsigned char src_idx, unsigned char access, unsigned int DI_Width, unsigned int DI_Height, unsigned char index)
{
	unsigned char timing = 0;
	if (access == SLR_DI_3A_mode)
		timing = DI_RTNR_PQC_3A;
	else if (access == SLR_DI_5A_mode)
		timing = DI_RTNR_PQC_5A;
	else if (access == SLR_DI_progress_mode && DI_Height <= 600)	// 576p
		timing = DI_RTNR_PQC_SD;
	else if (access == SLR_DI_progress_mode && DI_Height <= 750)	// 720p
		timing = DI_RTNR_PQC_720p;
	else if (access == SLR_DI_progress_mode && DI_Height <= 1100)	// 1080p
		timing = DI_RTNR_PQC_1080p;
	else if (access == SLR_DI_progress_mode && DI_Height <= 2200)	// 2160
		timing = DI_RTNR_PQC_4k;
	else if (access == SLR_DI_progress_mode && DI_Height <= 4350)	// 720p
		timing = DI_RTNR_PQC_SD;
	else
		rtd_pr_scaler_memory_info("get_di_PQC_timing, input error, access=%d, DI_Width=%d, DI_Height=%d \n", access, DI_Width, DI_Height);		
	
	rtd_pr_scaler_memory_info("get_di_PQC_timing,timing=%d, access=%d, DI_Width=%d, DI_Height=%d \n", timing, access, DI_Width, DI_Height);
	return timing;
}

unsigned char drv_memory_di_Decide_PQC_mode(unsigned char src_idx, unsigned char access, unsigned int DI_Width, unsigned int DI_Height, unsigned char DI_DMA_Mode, unsigned char index)
{
	unsigned char timing;
	unsigned char PQC_Mode;
	_RPC_system_setting_info* RPC_system_info_structure_table;

	RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if(RPC_system_info_structure_table == NULL){
		rtd_pr_scaler_memory_emerg("drv_memory_di_Decide_PQC_mode, NULL table \n");
		return 0;
	}

	timing = drv_memory_get_di_PQC_timing(0, access, DI_Width, DI_Height, 0);

	PQC_Mode = RPC_system_info_structure_table->PQ_DI_RTNR_PQC_CTRL.PQC_Mode[timing];

	rtd_pr_scaler_memory_info("di_Decide_PQC_mode,PQC_Mode=%d,\n", PQC_Mode);
	return PQC_Mode;
}

unsigned char drv_memory_di_Decide_PQC_Limit_Bit(unsigned char src_idx, unsigned char access, unsigned int DI_Width, unsigned int DI_Height, unsigned char DI_DMA_Mode, unsigned char index)
{
	unsigned char PQC_Bit_Limit;
#if 0
	INPUT_TIMING_INDEX patternIdx = get_current_pattern_index();

	if ((patternIdx == VO_4K2K_30) || (patternIdx == VO_4K2K_60) /*|| (patternIdx == HDMI_4K2K_60)*/)
		PQC_Bit_Limit = 10;
	else
		PQC_Bit_Limit = 10;

	PQC_Bit_Limit = get_current_driver_pattern(DRIVER_DIPQC_BIT_PATTERN);

	rtd_pr_scaler_memory_info("PQC Bit, patternIdx=%d \n", patternIdx);

#else
	PQC_Bit_Limit = get_current_driver_pattern(DRIVER_DIPQC_BIT_PATTERN);
	rtd_pr_scaler_memory_info("PQC Bit, PQC_Bit_Limit=%d \n", PQC_Bit_Limit);

	if (PQC_Bit_Limit == 0)	//6bit (off) / 10bit(on) / 9bit(2) / 8bit(3) / 7bit(4) / 6bit(5) / 5bit(6) / 4bit(7)
		PQC_Bit_Limit = 6;
	else if (PQC_Bit_Limit == 2)
		PQC_Bit_Limit = 9;
	else if (PQC_Bit_Limit == 3)
		PQC_Bit_Limit = 8;
	else if (PQC_Bit_Limit == 4)
		PQC_Bit_Limit = 7;
	else if (PQC_Bit_Limit == 5)
		PQC_Bit_Limit = 6;
	else if (PQC_Bit_Limit == 6)
		PQC_Bit_Limit = 5;
	else if (PQC_Bit_Limit == 7)
		PQC_Bit_Limit = 4;
	else
		PQC_Bit_Limit = 10;
#endif

	return PQC_Bit_Limit;
}

unsigned int drv_memory_di_PQC_Line_Sum_Bit(unsigned int Val, unsigned char index, unsigned char isSetFlag)
{
	static unsigned int PQC_line_sun_bit[2];	//  unit: 128bit

	if (index > 1)
		index = 0;

	if (isSetFlag == 1)
		PQC_line_sun_bit[index] = Val;

	return PQC_line_sun_bit[index];
}

char drv_memory_di_Compression_Config(unsigned char src_idx, unsigned char access, unsigned int DI_Width, unsigned int DI_Height, unsigned char DI_DMA_Mode, unsigned char index)
{
//	unsigned char cmp_alpha_en;
	unsigned short cmp_line_sum_bit;
	unsigned char PQC_Mode;
	unsigned char PQC_Bit_Limit;
	unsigned char PQC_Data_Format;
	unsigned int PQC_Width;
#if 1
	di_di_pq_cmp_RBUS di_di_pq_cmp_reg;
	di_di_pq_decmp_RBUS di_di_pq_decmp_reg;
	di_di_pq_cmp_pair_RBUS di_di_pq_cmp_pair_reg;
	di_di_pq_decmp_pair_RBUS di_di_pq_decmp_pair_reg;
	di_ddr_datdma_rm_RBUS di_ddr_datdma_rm_reg;
	di_di_pq_decmp_sat_en_RBUS di_di_pq_decmp_sat_en_reg;
	di_di_pq_cmp_bit_RBUS di_di_pq_cmp_bit_reg;
	di_di_pq_cmp_enable_RBUS di_di_pq_cmp_enable_reg;
	di_di_pq_cmp_allocate_RBUS di_di_pq_cmp_allocate_reg;
	di_di_dma_multi_req_num_RBUS di_di_dma_multi_req_num_reg;
	di_di_pq_cmp_blk0_add0_RBUS di_di_pq_cmp_blk0_add0_reg;
	di_di_pq_cmp_blk0_add1_RBUS di_di_pq_cmp_blk0_add1_reg;
	di_di_pq_cmp_balance_RBUS di_di_pq_cmp_balance_reg;
	di_di_pq_cmp_poor_RBUS di_di_pq_cmp_poor_reg;
	di_di_pq_cmp_smooth_RBUS di_di_pq_cmp_smooth_reg;
	di_di_pq_cmp_guarantee_RBUS di_di_pq_cmp_guarantee_reg;

//di_im_di_dma_RBUS di_im_di_dma_reg;
	/* double buffer*/
	di_db_reg_ctl_RBUS di_db_reg_ctl_reg;

	PQC_Mode = drv_memory_di_Decide_PQC_mode(src_idx,  access, DI_Width, DI_Height, DI_DMA_Mode, index);
	if (access != SLR_DI_progress_mode || PQC_Mode == PQC_Disable) {
		di_di_pq_cmp_reg.regValue= IoReg_Read32(DI_DI_PQ_CMP_reg);
		di_di_pq_decmp_reg.regValue = IoReg_Read32(DI_DI_PQ_DECMP_reg);
		di_di_pq_cmp_reg.cmp_en = 0;
		di_di_pq_decmp_reg.decmp_en = 0;
		IoReg_Write32(DI_DI_PQ_CMP_reg, di_di_pq_cmp_reg.regValue);
		IoReg_Write32(DI_DI_PQ_DECMP_reg, di_di_pq_decmp_reg.regValue);

		di_di_dma_multi_req_num_reg.regValue = IoReg_Read32(DI_DI_DMA_Multi_Req_Num_reg);
		di_di_dma_multi_req_num_reg.rdma_addr_rolling = 0;
		di_di_dma_multi_req_num_reg.wdma_addr_rolling = 0;
		IoReg_Write32(DI_DI_DMA_Multi_Req_Num_reg, di_di_dma_multi_req_num_reg.regValue);

		IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_apply_mask);

		drv_memory_di_Set_Compression_Bit_Num(0, index);
		drv_memory_di_Set_Compression_Mode(PQC_Disable, index);

		rtd_pr_scaler_memory_info("PQC Disable, return\n");

		return 1;
	}

//	cmp_alpha_en = drv_memory_di_Decide_PQC_alpha_En();
	PQC_Bit_Limit = drv_memory_di_Decide_PQC_Limit_Bit(src_idx,  access, DI_Width, DI_Height, DI_DMA_Mode, index);
	PQC_Data_Format = get_current_driver_pattern(DRIVER_RTNR_PATTERN);

	PQC_Width =  ((DI_Width%32)==0)?(DI_Width):(DI_Width + (32 - (DI_Width%32)));

	/* set flag*/
	drv_memory_di_Set_Compression_Bit_Num(PQC_Bit_Limit, index);
	drv_memory_di_Set_Compression_Mode(PQC_Mode, index);

	/* rdma_remain_en = 0, in compression mode*/
	di_ddr_datdma_rm_reg.regValue = IoReg_Read32(DI_DDR_DATDMA_RM_reg);
	di_ddr_datdma_rm_reg.rdma_remain_en = 0;
	IoReg_Write32(DI_DDR_DATDMA_RM_reg, di_ddr_datdma_rm_reg.regValue);

	/* com and decom setting*/
	di_di_pq_cmp_reg.regValue= IoReg_Read32(DI_DI_PQ_CMP_reg);
	di_di_pq_decmp_reg.regValue = IoReg_Read32(DI_DI_PQ_DECMP_reg);
	di_di_pq_cmp_pair_reg.regValue = IoReg_Read32(DI_DI_PQ_CMP_PAIR_reg);
	di_di_pq_decmp_pair_reg.regValue = IoReg_Read32(DI_DI_PQ_DECMP_PAIR_reg);
	di_di_pq_cmp_bit_reg.regValue = IoReg_Read32(DI_DI_PQ_CMP_BIT_reg);

	di_di_pq_cmp_reg.cmp_en = 1;
	di_di_pq_cmp_reg.cmp_height = DI_Height;
	di_di_pq_cmp_reg.cmp_width_div32 = PQC_Width >> 5;

	di_di_pq_cmp_pair_reg.cmp_jump4_en = 1;
	//di_di_pq_cmp_pair_reg.cmp_dic_mode_en = 1;
	di_di_pq_cmp_pair_reg.cmp_data_bit_width = 1;
	di_di_pq_cmp_pair_reg.cmp_data_color = 1;
	if (PQC_Data_Format == DI_RTNR_420 || DI_DMA_Mode == SLR_DMA_420_mode)
		di_di_pq_cmp_pair_reg.cmp_data_format = 2;	/*PQC format = 420 */
	else if (PQC_Data_Format == DI_RTNR_400)
		di_di_pq_cmp_pair_reg.cmp_data_format = 3;
	else
		di_di_pq_cmp_pair_reg.cmp_data_format = 1;

	/* frame mode or line mode*/
	if (PQC_Mode == PQC_Line_Mode || PQC_Mode == PQC_Line_Mode_Rolling || PQC_Mode == PQC_Line_Mode_Single_Buffer)
		di_di_pq_cmp_pair_reg.cmp_line_mode = 1;
	else
		di_di_pq_cmp_pair_reg.cmp_line_mode = 0;

	cmp_line_sum_bit = ((PQC_Width * PQC_Bit_Limit + 256)+127)>>7;	// unit: 128bit
	di_di_pq_cmp_pair_reg.cmp_line_sum_bit = cmp_line_sum_bit;
	drv_memory_di_PQC_Line_Sum_Bit(cmp_line_sum_bit, index, 1);

#if 0
	if (PQC_Mode == PQC_Frame_Mode) {
		di_di_pq_cmp_bit_reg.frame_limit_bit = PQC_Bit_Limit;
		di_di_pq_cmp_bit_reg.line_limit_bit = PQC_Bit_Limit + 4;	/*recommand by skyred for mer3*/
		di_di_pq_cmp_bit_reg.block_limit_bit = 0x3f;	/*recommand by skyred for mer3*/
	} else {
		di_di_pq_cmp_bit_reg.frame_limit_bit = PQC_Bit_Limit;	/* compress to 10 bit for frame mode*/
		di_di_pq_cmp_bit_reg.line_limit_bit = PQC_Bit_Limit + 4;		/* cmpression to 10 bit for line mode*/
		di_di_pq_cmp_bit_reg.block_limit_bit = 0x3f;	/*recommand by skyred for mer3*/
	}
#else
	di_di_pq_cmp_bit_reg.frame_limit_bit = PQC_Bit_Limit;
	di_di_pq_cmp_bit_reg.line_limit_bit = PQC_Bit_Limit + 4;	/*recommand by skyred for mer3*/
	di_di_pq_cmp_bit_reg.block_limit_bit = 0x3f;	/*recommand by skyred for mer3*/
#endif

	di_di_pq_decmp_reg.decmp_en = 1;
	di_di_pq_decmp_reg.decmp_height = DI_Height;
	di_di_pq_decmp_reg.decmp_width_div32 = PQC_Width >> 5;

	di_di_pq_decmp_pair_reg.decmp_jump4_en = 1;
	//di_di_pq_decmp_pair_reg.decmp_dic_mode_en = 1;
	di_di_pq_decmp_pair_reg.decmp_data_bit_width = 1;
	di_di_pq_decmp_pair_reg.decmp_data_color = 1;
	if (PQC_Data_Format == DI_RTNR_420 || DI_DMA_Mode == SLR_DMA_420_mode)
		di_di_pq_decmp_pair_reg.decmp_data_format = 2;	/*PQC format = 420 */
	else if (PQC_Data_Format == DI_RTNR_400)
		di_di_pq_decmp_pair_reg.decmp_data_format = 3;
	else
		di_di_pq_decmp_pair_reg.decmp_data_format = 1;

	/* frame mode or line mode*/
	if (PQC_Mode == PQC_Line_Mode || PQC_Mode == PQC_Line_Mode_Rolling || PQC_Mode == PQC_Line_Mode_Single_Buffer)
		di_di_pq_decmp_pair_reg.decmp_line_mode = 1;
	else
		di_di_pq_decmp_pair_reg.decmp_line_mode = 0;

	di_di_pq_decmp_pair_reg.decmp_line_sum_bit = cmp_line_sum_bit;

	IoReg_Write32(DI_DI_PQ_CMP_reg, di_di_pq_cmp_reg.regValue);
	IoReg_Write32(DI_DI_PQ_DECMP_reg, di_di_pq_decmp_reg.regValue);
	IoReg_Write32(DI_DI_PQ_CMP_PAIR_reg, di_di_pq_cmp_pair_reg.regValue);
	IoReg_Write32(DI_DI_PQ_DECMP_PAIR_reg, di_di_pq_decmp_pair_reg.regValue);
	IoReg_Write32(DI_DDR_DATDMA_RM_reg, di_ddr_datdma_rm_reg.regValue);
	IoReg_Write32(DI_DI_PQ_CMP_BIT_reg, di_di_pq_cmp_bit_reg.regValue);

	/* for decom setting*/
	di_di_pq_decmp_sat_en_reg.regValue = IoReg_Read32(DI_DI_PQ_DECMP_SAT_EN_reg);
	di_di_pq_decmp_sat_en_reg.saturation_en = 1;
	di_di_pq_decmp_sat_en_reg.saturation_type = 1;
	IoReg_Write32(DI_DI_PQ_DECMP_SAT_EN_reg, di_di_pq_decmp_sat_en_reg.regValue);

	/*recommand by skyred for mer3*/
	di_di_pq_cmp_enable_reg.regValue = IoReg_Read32(DI_DI_PQ_CMP_ENABLE_reg);
	di_di_pq_cmp_enable_reg.g_ratio = 14;		/*422 and 420 is same*/
	di_di_pq_cmp_enable_reg.tail_dummy_throughput = 1;
	di_di_pq_cmp_enable_reg.first_predict_nc_mode = 1;
	IoReg_Write32(DI_DI_PQ_CMP_ENABLE_reg, di_di_pq_cmp_enable_reg.regValue);

	/*recommand by skyred for mer3*/
	di_di_pq_cmp_allocate_reg.regValue = IoReg_Read32(DI_DI_PQ_CMP_ALLOCATE_reg);
	di_di_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 16;/*422 and 420 is same*/
	di_di_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 13;/*422 and 420 is same*/
	IoReg_Write32(DI_DI_PQ_CMP_ALLOCATE_reg, di_di_pq_cmp_allocate_reg.regValue);

	/*recommand by skyred for mer3*/
	di_di_pq_cmp_blk0_add0_reg.regValue = IoReg_Read32(DI_DI_PQ_CMP_BLK0_ADD0_reg);
	di_di_pq_cmp_blk0_add1_reg.regValue = IoReg_Read32(DI_DI_PQ_CMP_BLK0_ADD1_reg);
	di_di_pq_cmp_blk0_add0_reg.blk0_add_value32 = 16;
	di_di_pq_cmp_blk0_add0_reg.blk0_add_value16 = 8;
	di_di_pq_cmp_blk0_add1_reg.blk0_add_value8 = 4;
	di_di_pq_cmp_blk0_add1_reg.blk0_add_value4 = 2;
	di_di_pq_cmp_blk0_add1_reg.blk0_add_value2 = 1;
	di_di_pq_cmp_blk0_add1_reg.blk0_add_value1 = 1;
	di_di_pq_cmp_blk0_add0_reg.blk0_add_value0 = 1;
	di_di_pq_cmp_blk0_add0_reg.dic_mode_tolerance = 0;
	IoReg_Write32(DI_DI_PQ_CMP_BLK0_ADD0_reg, di_di_pq_cmp_blk0_add0_reg.regValue);
	IoReg_Write32(DI_DI_PQ_CMP_BLK0_ADD1_reg, di_di_pq_cmp_blk0_add1_reg.regValue);

	di_di_pq_cmp_balance_reg.regValue = IoReg_Read32(DI_DI_PQ_CMP_BALANCE_reg);
	di_di_pq_cmp_balance_reg.balance_g_give = 3;
	di_di_pq_cmp_balance_reg.balance_g_ud_th = 0;
	di_di_pq_cmp_balance_reg.balance_g_ov_th = 3;
	di_di_pq_cmp_balance_reg.balance_rb_give = 3;
	di_di_pq_cmp_balance_reg.balance_rb_ud_th = 0;
	di_di_pq_cmp_balance_reg.balance_rb_ov_th = 3;
	IoReg_Write32(DI_DI_PQ_CMP_BALANCE_reg, di_di_pq_cmp_balance_reg.regValue);

	di_di_pq_cmp_poor_reg.regValue = IoReg_Read32(DI_DI_PQ_CMP_POOR_reg);
	di_di_pq_cmp_poor_reg.poor_limit_th_qp1 = 3;
	di_di_pq_cmp_poor_reg.poor_limit_th_qp2 = 6;
	di_di_pq_cmp_poor_reg.poor_limit_th_qp3 = 10;
	di_di_pq_cmp_poor_reg.poor_limit_th_qp4 = 16;
	IoReg_Write32(DI_DI_PQ_CMP_POOR_reg, di_di_pq_cmp_poor_reg.regValue);

	di_di_pq_cmp_smooth_reg.regValue = IoReg_Read32(DI_DI_PQ_CMP_SMOOTH_reg);
	di_di_pq_cmp_smooth_reg.dic_mode_entry_th = 15;
	IoReg_Write32(DI_DI_PQ_CMP_SMOOTH_reg, di_di_pq_cmp_smooth_reg.regValue);

	di_di_pq_cmp_guarantee_reg.regValue = IoReg_Read32(DI_DI_PQ_CMP_Guarantee_reg);
	di_di_pq_cmp_guarantee_reg.dic_mode_qp_th_g = 1;
	di_di_pq_cmp_guarantee_reg.dic_mode_qp_th_rb = 1;
	IoReg_Write32(DI_DI_PQ_CMP_Guarantee_reg, di_di_pq_cmp_guarantee_reg.regValue);

#if 0	// move to end of DI flow
	/* enable rolling */
	di_di_dma_multi_req_num_reg.regValue = IoReg_Read32(DI_DI_DMA_Multi_Req_Num_reg);
	if (PQC_Mode == PQC_Line_Mode_Rolling) {
		di_di_dma_multi_req_num_reg.rdma_addr_rolling = 1;
		di_di_dma_multi_req_num_reg.wdma_addr_rolling = 1;
	} else {
		di_di_dma_multi_req_num_reg.rdma_addr_rolling = 0;
		di_di_dma_multi_req_num_reg.wdma_addr_rolling = 0;
	}
	IoReg_Write32(DI_DI_DMA_Multi_Req_Num_reg, di_di_dma_multi_req_num_reg.regValue);
#endif
#if 0
	/* some special case setting*/
	if (di_di_pq_cmp_pair_reg.cmp_data_format == 2) {	/* cmp_data_format = 420*/
		di_im_di_dma_reg.regValue = IoReg_Read32(DI_IM_DI_DMA_reg);
		di_im_di_dma_reg.dma_420_en = 1;/* 420 */
		IoReg_Write32(DI_IM_DI_DMA_reg, di_im_di_dma_reg.regValue);

		/*IoReg_ClearBits(DI_IM_DI_DMA_reg, _BIT31);
		IoReg_SetBits(DI_IM_DI_DMA_reg, _BIT30);*/
	}
#endif

	rtd_pr_scaler_memory_info("PQC, PQC_Mode=%d, PQC_Bit_Limit=%d, PQC_Data_Format=%d\n", PQC_Mode, PQC_Bit_Limit, PQC_Data_Format);

	/* double buffer?*/
	di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
	di_db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, di_db_reg_ctl_reg.regValue);
	/*system_info->I_Height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
	system_info->I_Width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);*/
#endif
return 0;
}
unsigned int drv_memory_Get_di_data_size_Static(unsigned char src_idx, unsigned char isCompression, unsigned char access, unsigned int DI_Width, unsigned int DI_Height, unsigned char DI_MEM_IDX)
{
	unsigned int line_sum_bit=0;
	unsigned int buffer_size=0;
	unsigned int offset = 1024;
	//unsigned int PQC_Width = 0;
#if 1	// calculate memory
	unsigned char data_bit=0;

	if ((access == SLR_DI_progress_mode) && (isCompression != PQC_Disable)) {	/* compression mode, */
		line_sum_bit = drv_memory_di_PQC_Line_Sum_Bit(0, DI_MEM_IDX, 0);	// unit 128b
		buffer_size = ((line_sum_bit<<7) * (DI_Height+(isCompression==PQC_Line_Mode_Rolling?1:0)))>>3;
	} else {	/*3A, 5A, p mode with no PQC */
		data_bit = (fw_scalerip_get_DI_chroma_10bits())?(10):(8);
		if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_422_mode)
			data_bit = data_bit + (data_bit>>1) + (data_bit>>1);	//Y + Cb + Cr
		else if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_420_mode)
			data_bit = data_bit + ((data_bit + data_bit)>>2);
		buffer_size = (DI_Width * DI_Height * data_bit)>>3;
	}
	buffer_size = buffer_size + offset;
	rtd_pr_scaler_memory_info("di_data_size_cal, buffer_size=%d, data_bit=%d, line_sum_bit=%d,\n", buffer_size, data_bit, line_sum_bit);

#else	// static memory
	/*K4*/
	/*1080p and 4k is compression, other is non compression*/
	/* DI 1 = 13Mb, 10Mb for DI, 3Mb for MA-SNR @ m4, use rolling mode*/
	if (((isCompression == PQC_Frame_Mode) || (isCompression == PQC_Line_Mode)) && access == SLR_DI_progress_mode) {	/* compression mode, */
		buffer_size = 0xA00000; /*10Mb+10Mb, 2 buffer allocation, same setting between 1080p and 4k*/
	} else if (isCompression == PQC_Line_Mode_Rolling && access == SLR_DI_progress_mode) { /* p mode, non-compression and compression Line mode Rolling */
		buffer_size = 0xA00000; /*10Mb, 1 buffer allocation, same setting between 1080p and 4k*/
	} else if (access == SLR_DI_progress_mode) { /* p mode, non-compression and compression Line mode Rolling */
		//buffer_size = 0x1400000; /*20Mb, 1 buffer allocation, same setting between 1080p and 4k*/
		buffer_size = 0xA00000; /*10Mb, 1 buffer allocation, for 720p*/
	} else if (access == SLR_DI_3A_mode) {	/* i mode, 3A */
		buffer_size = 0x280000;  /*2.5 MB + 2.5MB , 2 buffer allocation*/
	} else {	/** i mode ,5A */
		buffer_size = 0x100000;  /*1MB + 1MB + 1MB + 1MB , 4 buffer allocation*/
	}

	if (DI_MEM_IDX == 1 /*&& get_platform() == PLATFORM_KXLP*/) {	/* index = 1, no suppport 4k*/
		if (((isCompression == PQC_Frame_Mode) || (isCompression == PQC_Line_Mode)) && access == SLR_DI_progress_mode) {	/* compression mode, */
			buffer_size = 0x280000; /*2.5Mb+2.5Mb, 2 buffer allocation, same setting between 1080p*/
		} else if (isCompression == PQC_Line_Mode_Rolling && access == SLR_DI_progress_mode) { /* p mode, non-compression and compression Line mode Rolling */
			buffer_size = 0x280000; /*2.5Mb, 1 buffer allocation, same setting between 1080p*/
		} else if (access == SLR_DI_progress_mode) { /* p mode, non-compression and compression Line mode Rolling */
			buffer_size = 0x500000; /*5 Mb, 1 buffer allocation, for 1080p*/
		} else if (access == SLR_DI_3A_mode) {	/* i mode, 3A */
			buffer_size = 0x280000;  /*2.5 MB + 2.5MB , 2 buffer allocation*/
		} else {	/** i mode ,5A */
			buffer_size = 0x100000;  /*1MB + 1MB + 1MB + 1MB , 4 buffer allocation*/
		}
	}
	buffer_size = buffer_size - offset;
#endif

	return buffer_size;
}

unsigned int drv_memory_Get_di_alloc_buffer_Static(unsigned char src_idx, unsigned char isCompression, unsigned char access, unsigned int DI_Width, unsigned int DI_Height, unsigned char DI_MEM_IDX)
{
	unsigned int buffer_size;

	buffer_size = 0x2880000;

	return buffer_size;
}


unsigned int drv_memory_Get_di_info_size_Static(unsigned char src_idx, unsigned char isCompression, unsigned char access, unsigned int DI_Width, unsigned int DI_Height, unsigned char DI_MEM_IDX)
{
	unsigned int buffer_size;
	unsigned int offset = 1024;
#if 1	// calculate memory
	unsigned char info_bit = 16;

	if (access == SLR_DI_progress_mode) { /* p mode */
		buffer_size = 0;
	} else {
		buffer_size = (DI_Width * DI_Height *  info_bit)>>3;
	}
	buffer_size = buffer_size + offset;
	rtd_pr_scaler_memory_info("di_info_size cal, buffer_size=%d,\n", buffer_size);

#else	// static memory
	/* no RTNR_INFO memory, p mode same as compression*/
	if (access == SLR_DI_progress_mode) { /* p mode */
		buffer_size = offset;
	} else if (access == SLR_DI_3A_mode) {	/* i mode, 3A */
		buffer_size = 0x280000;  /*2.5 MB + 2.5MB , 2 buffer allocation*/
	} else {
		buffer_size = 0x300000;  /*3 MB + 3MB , 2 buffer allocation*/
	}

	if (DI_MEM_IDX == 1 /*&& get_platform() == PLATFORM_KXLP*/) {
		if (access == SLR_DI_progress_mode) { /* p mode */
			buffer_size = 0;
		} else {	/* i mode */
			buffer_size = 0x1BAF80; /* info  = 3.460693359 MB in 1080i, 1.7303466795 MB per frame,  2 frame allocation*/
		}
	}
	buffer_size = buffer_size - offset;
#endif
	return buffer_size;
}
unsigned int drv_memory_set_ip_fifo_Mer2(unsigned char access)
{
	typedef struct _MEMSET {
		unsigned int number;
		unsigned int length;
		unsigned int remain;
		unsigned int water_level;
	} MEMSET;
	//di_im_di_btr_control_RBUS BTR_Control_Reg;
	di_ddr_rdatdma_wtlennum_RBUS ddr_rdatdma_wtlennum;
	di_ddr_rinfdma_wtlennum_RBUS ddr_rinfdma_wtlennum;
	di_ddr_wdatdma_wtlennum_RBUS ddr_wdatdma_wtlennum;
	di_ddr_winfdma_wtlennum_RBUS ddr_winfdma_wtlennum;
	di_ddr_datdma_rm_RBUS ddr_datdma_rm;
	di_ddr_infdma_rm_RBUS ddr_infdma_rm;
	//di_di_dma_multi_req_num_RBUS di_dma_multi_req_num_reg;
	//ddr_vrfifo1wtlvllennum_RBUS ddr_vrfifowtlvllennum;
	//ddr_vwfifo1wtlvllennum_RBUS ddr_vwfifowtlvllennum;
	//ddr_vfifo1rdwrremainder_RBUS ddr_vfifordwrremainder;

	unsigned int	DI_Width = 0, DI_Height = 0, size_per_frame = 0;
	unsigned char	data_bitNum = 0, info_bitNum = 0, i = 0/*, DIAllocateBlock = 0*/;
	unsigned int	data_value, info_value=0, MemStartAddr, MemEndtAddr;
	MEMSET			data_r, data_w, info_r, info_w;

	UINT32 lineSize_odd_128, lineSize_even_128;
	unsigned char DI_SMD_enable = 0;

	unsigned char source=255;
//	unsigned char newRTNR_onoff = 0;
//	unsigned char m_rtnrcompress = 0;
	unsigned char DI_BTR_enable = 0;
	unsigned char DI_BTR_type = 0;
	unsigned int buffer_alloc = 0;
	unsigned int data_size = 0;
	unsigned int info_size = 0;
	unsigned int data_size_aligned = 0;
	unsigned int info_size_aligned = 0;
	unsigned int MemAddr[6], MemAddr_aligned[6];
	unsigned short aligned_val = 0;
	unsigned char isCompression_Mode = 0, Compression_bit = 0;
	//unsigned char  data_num/*, info_num*/;
	unsigned char index;
	unsigned int data_addr_reg, info_addr_reg/*, data_addr_reg_3D, info_addr_reg_3D*/;
//fixme:vip
#if 1
	//extern Scaler_DI_Coef_t Scaler_DI_Coef_table[VIP_QUALITY_SOURCE_NUM];
	source = fwif_vip_source_check(3, 0);
	/*if(source >= VIP_QUALITY_SOURCE_NUM)
	{
		rtd_pr_scaler_memory_err("\r\n#####drv_memory_set_ip_fifo source nunber %d error####\r\n", source);
		source = VIP_QUALITY_SOURCE_NUM-1;
	}*/
	//frank@1018 add below code to solve scan tool warning
	//newRTNR_onoff = 0; //(unsigned char)Scaler_DI_Coef_table[source].newRTNR_style;
	//DI_SMD_enable = Scaler_getDI_SMD_en(source);
	DI_SMD_enable = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE);
	DI_BTR_enable = drv_memory_get_ip_Btr_mode();

	//BTR_Control_Reg.regValue = rtd_inl(DI_IM_DI_BTR_CONTROL_VADDR);
	//DI_BTR_type = BTR_Control_Reg.btr_solutiontype;
	DI_BTR_type = (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) < 960)?0:1;
#endif

	DI_Width = Scaler_DispGetInputInfo(SLR_INPUT_DI_WID);
	DI_Height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);

#ifdef CONFIG_BW_96B_ALIGNED
	aligned_val = 6;
#else
	aligned_val = 0;
#endif

	rtd_pr_scaler_memory_info("\n DI Memory set FIFO !, adaptivestream=%d, aligned_val=%d,\n", vsc_get_adaptivestream_flag(SLR_MAIN_DISPLAY), aligned_val);
	rtd_pr_scaler_memory_info("Width:%d, Height:%d, DMA mode=%d,access=%d\n ", DI_Width, DI_Height, drv_memory_get_ip_DMA420_mode(), access);
	rtd_pr_scaler_memory_info("Data status Y=%d C=%d\n ", fw_scalerip_get_DI_chroma_10bits()?10:8, fw_scalerip_get_DI_chroma_10bits()?10:8);
	rtd_pr_scaler_memory_info("Info status BTR=%d(type:%d) SMD=%d\n ", DI_BTR_enable, DI_BTR_type, DI_SMD_enable);
	//rtd_pr_scaler_memory_info("Info status BTR=%d(type:%d) SMD=%d RTNR style=%d\n ", DI_BTR_enable, DI_BTR_type, DI_SMD_enable, newRTNR_onoff);

	data_bitNum = 16; // for YUV subsample 422 ( now only using 2 typr : 422 & 420	)
	info_bitNum = 0;

#ifdef CONFIG_I2RND_DUAL_VO
	index = (Scaler_I2rnd_get_display()<=1)?(0):(1);	// Scaler_I2rnd_get_display() ==> 0: i2r disable, 1:i2r s0, 2:i2r s1
#else
	index = 0;
#endif
	/* new DI/RTNR compression for Mer2*/
	drv_memory_di_Compression_Config(source, access, DI_Width, DI_Height, drv_memory_get_ip_DMA420_mode(), index);
	//isCompression_Mode = drv_memory_di_Get_Compression_Bit_Num();
	Compression_bit = drv_memory_di_Get_Compression_Bit_Num(index);
	isCompression_Mode = drv_memory_di_Get_Compression_Mode(index);

	rtd_pr_scaler_memory_info("index=%d, Compression_bit=%d, isCompression_Mode=%d\n", index, Compression_bit, isCompression_Mode);

	// assign data bits
	if (isCompression_Mode == PQC_Disable) {
		if (fw_scalerip_get_DI_chroma_10bits())
			data_bitNum += 2;
		if (fw_scalerip_get_DI_chroma_10bits())
			data_bitNum += 2;
	} else {
		data_bitNum = Compression_bit;
	}
#ifdef ENABLE_DI_COMPRESS
	if ((Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) > 2048)
		&& (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > 1200)) {	// 4k2k
		drv_memory_set_ip_compress_bit(12);
		m_rtnrcompress = 1;
		data_bitNum = 12;
	}
#endif

	/*
		LearnRPG comment DI DMA how to use?
		from magellan/sirius/mac3
		FIFO length is 64bits, SD need 2(128bits), HD need 4(SD double), progressive need 6
		about READ(from DDR to FIFO) the best performance is using water_level+length = FIFO length
		about WRITE(from FIFO to DDR) must just MATCH, otherwise will have dummy data in FIFO
	*/
	// assign info bits
	if (access == SLR_DI_5A_mode) {
		//rtd_pr_scaler_memory_debug("\n 5A mode \n");
		//rtd_pr_scaler_memory_emerg("\n 5A mode \n");

		info_bitNum += (DI_BTR_enable?(DI_BTR_type?4:8):0); // type A:8 bits, type B:4 bits

		//DIAllocateBlock = 4;

		data_r.water_level = 98; /*68;*/	//WOSQRTK-12128 DMA underflow, modify water value from 68 to 98, recommand by Jimmy Xu
		data_r.length = 30; /*60;*/	//WOSQRTK-12128 DMA underflow, modify water value from 68 to 98, recommand by Jimmy Xu

		data_w.water_level = 60;
		data_w.length = 60;

		info_r.water_level = 32;
		info_r.length = 32;

		info_w.water_level = 32;
		info_w.length = 32;

		//data_num = 4;
		//info_num = 2;		/* BTR = on*/

	} else if (access == SLR_DI_3A_mode) {
		//rtd_pr_scaler_memory_debug("\n 3A mode \n");
		//rtd_pr_scaler_memory_emerg("\n 3A mode \n");

		info_bitNum += (DI_BTR_enable?4:0); // type A:8 bits, type B:4 bits, 3A only type B
		info_bitNum += 4; // frame motion in 3A default need 4 bits

		//DIAllocateBlock = 2;

		data_r.water_level = 196;
		data_r.length = 60;

		data_w.water_level = 60;
		data_w.length = 60;

		info_r.water_level = 32;
		info_r.length = 32;

		info_w.water_level = 32;
		info_w.length = 32;

		//data_num = 2;	/* use "2" for m4*/
		//info_num = 2;		/* BTR = on*/

	} else if (access == SLR_DI_progress_mode) {
		//rtd_pr_scaler_memory_debug("\n progress mode \n");

		//rtd_pr_scaler_memory_emerg("\n progress mode \n");
		//DIAllocateBlock = 2;

		//Waterlevel = 0x180 - length at PRTNR 2A case, but this value over 8bit(0xff), so give 0xff
		data_r.water_level = 0xff;
		data_r.length = 60;

		if (fwif_vip_increase_DI_water_lv())		//20150813 roger, change dat_len_r for 4k2k60p
			data_r.length = 96;

		if (isCompression_Mode == PQC_Line_Mode_Rolling) {
			if (DI_Height > 2100) {	// 4k
				data_w.water_level = 102;	// for PQC rolling mode and 96 aligned
				data_w.length = 102;
			} else {	// 2k
				data_w.water_level = 78;	// for PQC rolling mode and 96 aligned
				data_w.length = 78;
			}
		} else {
			data_w.water_level = 60;
			data_w.length = 60;
		}

		info_r.water_level = 32;
		info_r.length = 32;

		info_w.water_level = 32;
		info_w.length = 32;

		if(isCompression_Mode == PQC_Line_Mode_Rolling || isCompression_Mode == PQC_Line_Mode_Single_Buffer) {
			//data_num = 1;
			//info_num = 0;
			//DIAllocateBlock = 1;
		} else {
			//data_num = 2;
			//info_num = 0;
			//DIAllocateBlock = 2;
		}

	} else {
		rtd_pr_scaler_memory_debug("\n Error accept parameter in memory fifo \n");
		return false;
	}

	if (aligned_val != 0) {
		/* data burst len should be more after 96 byte aligned, recommand from xq_wu */
		data_r.length = ((data_r.length%aligned_val)==0)?(data_r.length):(data_r.length + (aligned_val - (data_r.length % aligned_val)));
		data_w.length = ((data_w.length%aligned_val)==0)?(data_w.length):(data_w.length + (aligned_val - (data_w.length % aligned_val)));
		/* info burst len should be less after 96 byte aligned, recommand from xq_wu */
		info_r.length = ((info_r.length%aligned_val)==0)?(info_r.length):(info_r.length - (info_r.length % aligned_val));
		info_w.length = ((info_w.length%aligned_val)==0)?(info_w.length):(info_w.length - (info_w.length % aligned_val));
	}

	if (DI_SMD_enable)
		info_bitNum += 4; //mac = 4, melin = 8

	// if k temporal function is enabled, new rtnr need 2 bits, otherwise still 0, please refer scalerColor.cpp:fwif_color_set_RTNR_style_adjust()
	//if (newRTNR_onoff == 3 || newRTNR_onoff == 4)
	//	info_bitNum += 2;

	//rtd_pr_scaler_memory_debug("\n Data BitNum=%d, Info BitNum=%d", data_bitNum, info_bitNum);
	//rtd_pr_scaler_memory_emerg("\n Data BitNum=%d, Info BitNum=%d", data_bitNum, info_bitNum);

	// Calculate Data number/length/remain
	if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_422_mode) { // 422 mode
		size_per_frame = DI_Width * DI_Height * data_bitNum;
		data_r.number = (int)(size_per_frame/128/data_r.length);
		data_r.remain = (size_per_frame - data_r.number*128*data_r.length + 127)/128;
		if (data_r.remain == 0 && data_r.number != 0) {
			data_r.number -= 1;
			data_r.remain = data_r.length;
		}

		data_w.number = (int)(size_per_frame/128/data_w.length);
		data_w.remain = (size_per_frame - data_w.number*128*data_w.length + 127)/128;
		if (data_w.remain == 0 && data_w.number != 0) {
			data_w.number -= 1;
			data_w.remain = data_w.length;
		}

		IoReg_ClearBits(DI_IM_DI_DMA_reg, _BIT31);
		IoReg_ClearBits(DI_IM_DI_DMA_reg, _BIT30);
	}
	else if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_420_mode) { // 420 mode
		lineSize_even_128	= DI_Width * (data_bitNum - (fw_scalerip_get_DI_chroma_10bits()?10:8)); // only Y componet
		lineSize_odd_128	= DI_Width * data_bitNum; // Y and C componet

		// linesize unit 64
		if (lineSize_even_128%64 == 0)
			lineSize_even_128 /= 64;
		else
			lineSize_even_128 = (lineSize_even_128+64)/64;
		if (lineSize_odd_128%64 == 0)
			lineSize_odd_128 /= 64;
		else
			lineSize_odd_128 = (lineSize_odd_128+64)/64;

		// frame unit 128, so linesize must divide by 2
		//if (DI_Height%2 == 0)
		//	size_per_frame = ((lineSize_even_128 + lineSize_odd_128+1)/2)*(DI_Height/2);
		//else
		size_per_frame = ((lineSize_even_128+1)/2)*(DI_Height/2) + ((lineSize_odd_128+1)/2)*((DI_Height+1)/2);

		data_r.number = (int)(size_per_frame/data_r.length);
		data_r.remain = size_per_frame%data_r.length;
		if (data_r.remain == 0 && data_r.number != 0) {
			data_r.number -= 1;
			data_r.remain = data_r.length;
		}

		data_w.number = (int)(size_per_frame/data_w.length);
		data_w.remain = size_per_frame%data_w.length;
		if (data_w.remain == 0 && data_w.number != 0) {
			data_w.number -= 1;
			data_w.remain = data_w.length;
		}

		if ((lineSize_even_128 > 0x7ff) || (lineSize_odd_128 > 0x7ff))
		{
			rtd_pr_scaler_memory_emerg("Q-12237 data protect 01, Error data need to check!! (%d, %d, %d, %d, %d)\n",
				lineSize_even_128,
				lineSize_odd_128,
				DI_Width,
				data_bitNum,
				fw_scalerip_get_DI_chroma_10bits());
			lineSize_even_128 = lineSize_even_128 & 0x000007ff;
			lineSize_odd_128 = lineSize_odd_128 & 0x000007ff;
		}

		IoReg_Mask32(DI_IM_DI_DMA_reg,
			~(DI_DMA_420_EN_MASK| DI_DMA_LINESIZE_EVEN_MASK| DI_DMA_LINESIZE_ODD_MASK),
			_BIT30 | (lineSize_even_128 << DI_DMA_LINESIZE_EVEN_START_BIT) | lineSize_odd_128);

		rtd_pr_scaler_memory_info("420 mode: linesize_even=%d, linesize_odd=%d\n", lineSize_even_128, lineSize_odd_128);
	}
	else { // 400 mode
		//size_per_frame = DI_Width * DI_Height * data_bitNum;
		size_per_frame = DI_Width * DI_Height * (data_bitNum - (fw_scalerip_get_DI_chroma_10bits()?10:8));
		data_r.number = (int)(size_per_frame/128/data_r.length);
		data_r.remain = (size_per_frame - data_r.number*128*data_r.length + 127)/128;
		if (data_r.remain == 0 && data_r.number != 0) {
			data_r.number -= 1;
			data_r.remain = data_r.length;
		}

		data_w.number = (int)(size_per_frame/128/data_w.length);
		data_w.remain = (size_per_frame - data_w.number*128*data_w.length + 127)/128;
		if (data_w.remain == 0 && data_w.number != 0) {
			data_w.number -= 1;
			data_w.remain = data_w.length;
		}
		IoReg_SetBits(DI_IM_DI_DMA_reg, _BIT31);
		IoReg_ClearBits(DI_IM_DI_DMA_reg, _BIT30);
	}
	//IoReg_SetBits(DI_db_reg_ctl_reg, _BIT0);

	// Calculate Info number/length/remain
	size_per_frame = DI_Width * DI_Height * info_bitNum;
	if (DI_SMD_enable && access == SLR_DI_5A_mode) // Info image height must +1 for DMA setting when SMD enable in 5A case
		size_per_frame += DI_Width * info_bitNum;

	info_r.number = (int)(size_per_frame/128/info_r.length);
	info_r.remain = (size_per_frame - info_r.number*128*info_r.length + 127)/128;
	if (info_r.remain == 0 && info_r.number != 0) {
		info_r.number -= 1;
		info_r.remain = info_r.length;
	}

	info_w.number = (int)(size_per_frame/128/info_w.length);
	info_w.remain = (size_per_frame - info_w.number*128*info_w.length + 127)/128;
	if (info_w.remain == 0 && info_w.number != 0) {
		info_w.number -= 1;
		info_w.remain = info_w.length;
	}

	//rtd_pr_scaler_memory_debug("\n Data number = %d, length = %d, remain = %d, water_level = %d in setting DI Read FIFO", data_r.number, data_r.length, data_r.remain, data_r.water_level);
	//rtd_pr_scaler_memory_debug("\n Data number = %d, length = %d, remain = %d, water_level = %d in setting DI Write FIFO", data_w.number, data_w.length, data_w.remain, data_w.water_level);
	//rtd_pr_scaler_memory_debug("\n Info number = %d, length = %d, remain = %d, water_level = %d in setting DI Read FIFO", info_r.number, info_r.length, info_r.remain, info_r.water_level);
	//rtd_pr_scaler_memory_debug("\n Info number = %d, length = %d, remain = %d, water_level = %d in setting DI Write FIFO", info_w.number, info_w.length, info_w.remain, info_w.water_level);

	ddr_rdatdma_wtlennum.dat_water_r	= data_r.water_level;
	ddr_rdatdma_wtlennum.dat_len_r		= data_r.length;
	ddr_rdatdma_wtlennum.dat_num_r		= data_r.number;

	ddr_wdatdma_wtlennum.dat_water_w	= data_w.water_level;
	ddr_wdatdma_wtlennum.dat_len_w		= data_w.length;
	ddr_wdatdma_wtlennum.dat_num_w		= data_w.number;

	ddr_datdma_rm.dat_remain_r			= data_r.remain;
	ddr_datdma_rm.dat_remain_w			= data_w.remain;
	if (isCompression_Mode == PQC_Disable)
		ddr_datdma_rm.rdma_remain_en		= 1;
	else
		ddr_datdma_rm.rdma_remain_en		= 0;

	ddr_rinfdma_wtlennum.inf_water_r	= info_r.water_level;
	ddr_rinfdma_wtlennum.inf_len_r		= info_r.length;
	ddr_rinfdma_wtlennum.inf_num_r		= info_r.number;

	ddr_winfdma_wtlennum.inf_water_w	= info_w.water_level;
	ddr_winfdma_wtlennum.inf_len_w		= info_w.length;
	ddr_winfdma_wtlennum.inf_num_w		= info_w.number;;

	ddr_infdma_rm.inf_remain_r			= info_r.remain;
	ddr_infdma_rm.inf_remain_w			= info_w.remain;

	//remain and number both 0 will cause DMA error when 1080p switch to 576i DI will fliker eric_huang@realsil.com.cn
	if (info_r.number == 0 && info_r.remain == 0) {
		ddr_rinfdma_wtlennum.inf_num_r = 0x174;
		ddr_infdma_rm.inf_remain_r = 0x0b;
	}

	//remain and number both 0 will cause DMA error when 1080p switch to 576i DI will fliker eric_huang@realsil.com.cn
	if (info_w.number == 0 && info_w.remain == 0) {
		ddr_winfdma_wtlennum.inf_num_w = 0x174;
		ddr_infdma_rm.inf_remain_w = 0x0b;
	}

	IoReg_Write32(DI_DDR_RDATDMA_WTLENNUM_reg, ddr_rdatdma_wtlennum.regValue);
	IoReg_Write32(DI_DDR_WDATDMA_WTLENNUM_reg, ddr_wdatdma_wtlennum.regValue);
	IoReg_Write32(DI_DDR_RINFDMA_WTLENNUM_reg, ddr_rinfdma_wtlennum.regValue);
	IoReg_Write32(DI_DDR_WINFDMA_WTLENNUM_reg, ddr_winfdma_wtlennum.regValue);
	IoReg_Write32(DI_DDR_DATDMA_RM_reg, ddr_datdma_rm.regValue);
	IoReg_Write32(DI_DDR_INFDMA_RM_reg, ddr_infdma_rm.regValue);

	IoReg_SetBits(DI_db_reg_ctl_reg, _BIT0);	/* HW do not clear this bit, alway turn on, setting will be apply while v_sync*/
	//frank@0514 change DI multi request to solve DI read underflow ++
	drv_memory_disable_di_write_req(FALSE);
	//frank@0514 change DI multi request to solve DI read underflow --

//frank@1202 Change code to solve 1366x768 image fail problem

#if 1/*qiangzhou: added DI memory allocate with VGIP PRE info for max size start*/
#if(CONFIG_DDR_DATA_BITS == 16)
	/*if ((Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_PIPMLSR) && (Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_PIPMLSR_4x3) &&
		(Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_MP1C12B) && (Scaler_PipGetInfo(SLR_PIP_TYPE) != SUBTYPE_MP1L7B)) {*/
		data_value = data_bitNum;
		info_value = info_bitNum;
		if(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) > _DISP_WID){
			data_value *= _DISP_WID;
			info_value *= _DISP_WID;
		}else{
			data_value *= Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
			info_value *= Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
		}


		if(((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
			&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
			&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status()) {
			data_value *= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
			info_value *= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
		} else {
			data_value *= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
			info_value *= Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
		}

		data_value = (data_value >> 3);//byte unit
		info_value = (info_value >> 3);//byte unit
//		value =(info->IPV_ACT_LEN_PRE *info->IPH_ACT_WID_PRE * bitNum)>>4;
	/*} else {
		data_value=(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*data_bitNum)>>3;	//byte unit
		info_value=(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)*Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)*info_bitNum)>>3;	//byte unit
	}*/
#else
	data_value=(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)*Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)*data_bitNum)>>3;	//byte unit
	info_value=(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)*Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)*info_bitNum)>>3;	//byte unit
#endif

	if (drv_memory_get_ip_DMA420_mode()) {
		//sync code from Mac2. Start
		//di_im_di_dma_RBUS im_di_dma;
		unsigned int di420_width;
		//im_di_dma.regValue = IoReg_Read32(DI_IM_DI_DMA_reg);
		lineSize_even_128 = (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) * (data_bitNum - 8));
		if((lineSize_even_128%128)==0)
			lineSize_even_128 /=128;
		else
			lineSize_even_128 = (lineSize_even_128+128)/128;

		lineSize_odd_128 = (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) * data_bitNum);
		if((lineSize_odd_128%128)==0)
			lineSize_odd_128 /=128;
		else
			lineSize_odd_128 = (lineSize_odd_128+128)/128;

		if (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)%2 == 0) {
			di420_width = (lineSize_odd_128 + lineSize_even_128)*(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)/2);
		} else {
			di420_width = lineSize_even_128*(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)/2)+lineSize_odd_128*((Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)+1)/2);
		}
		data_value = di420_width<<4;
		//sync code from Mac2. End
	}
#endif/*qiangzhou: added DI memory allocate with VGIP PRE info for max size end*/

	drvif_memory_free_block(MEMIDX_DI);

	// Set 32 avoid to cover other block memroy ([[h * v * bitnum] / (64* 2) ]* 16
	data_value = drvif_memory_get_data_align(data_value, 32);
	info_value = drvif_memory_get_data_align(info_value, 32);
	//frank@0707 when 3D mode DI must allocate 2 X memory

#if 1
	buffer_alloc = drv_memory_Get_di_alloc_buffer_Static(source, isCompression_Mode, access, DI_Width, DI_Height, index);
	data_size = drv_memory_Get_di_data_size_Static(source, isCompression_Mode, access, DI_Width, DI_Height, index);
	info_size = drv_memory_Get_di_info_size_Static(source, isCompression_Mode, access, DI_Width, DI_Height, index);

	if (aligned_val != 0) {
		data_size_aligned = dvr_size_alignment(data_size);
		///info_size_aligned = dvr_size_alignment(info_size);
	} else {
		data_size_aligned = data_size;
		//info_size_aligned = info_size;
	}

	if (index == 1) {
#ifdef CONFIG_I2RND_DUAL_VO
		MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI_2, buffer_alloc, MEMALIGN_ANY);
		data_addr_reg = DI_DI_I2R_DATMemoryStartAdd1_reg;
		info_addr_reg = DI_DI_I2R_INFMemoryStartAdd1_reg;
		data_addr_reg_3D = DI_DI_I2R_DATMemoryStartAdd1_3D_reg;
		info_addr_reg_3D = DI_DI_I2R_INFMemoryStartAdd1_3D_reg;
#else
		MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, buffer_alloc, MEMALIGN_ANY);
		data_addr_reg = DI_DI_DATMemoryStartAdd1_reg;
		info_addr_reg = DI_DI_INFMemoryStartAdd1_reg;
		//data_addr_reg_3D = DI_DI_DATMemoryStartAdd1_3D_reg;
		//info_addr_reg_3D = DI_DI_INFMemoryStartAdd1_3D_reg;
#endif
	} else{
		MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, buffer_alloc, MEMALIGN_ANY);
		data_addr_reg = DI_DI_DATMemoryStartAdd1_reg;
		info_addr_reg = DI_DI_INFMemoryStartAdd1_reg;
		//data_addr_reg_3D = DI_DI_DATMemoryStartAdd1_3D_reg;
		//info_addr_reg_3D = DI_DI_INFMemoryStartAdd1_3D_reg;
	}

	if(MemStartAddr == _DUMMYADDR){
		rtd_pr_scaler_memory_emerg("set MEMIDX_DI  to size fail !!!\n");
	}

	if (access == SLR_DI_progress_mode) {
		if (isCompression_Mode == PQC_Line_Mode_Single_Buffer) {
			MemAddr[0] = MemStartAddr;
			MemAddr[1] = MemStartAddr;
			MemAddr[2] = MemStartAddr + data_size;
			MemAddr[3] = MemStartAddr + data_size;
			MemAddr[4] = MemStartAddr + data_size;
			MemAddr[5] = MemStartAddr + data_size;

			MemEndtAddr = MemStartAddr + data_size;
		} else if (isCompression_Mode == PQC_Line_Mode_Rolling) {
			MemAddr[0] = MemStartAddr;
			MemAddr[1] = MemStartAddr + data_size;
			MemAddr[2] = MemStartAddr + data_size;
			MemAddr[3] = MemStartAddr + data_size;
			MemAddr[4] = MemStartAddr + data_size;
			MemAddr[5] = MemStartAddr + data_size;

			MemEndtAddr = MemStartAddr + data_size;
		} else {
			MemAddr[0] = MemStartAddr;
			MemAddr[1] = MemStartAddr + data_size;
			MemAddr[2] = MemStartAddr + data_size + data_size;
			MemAddr[3] = MemStartAddr + data_size + data_size;
			MemAddr[4] = MemStartAddr + data_size + data_size;
			MemAddr[5] = MemStartAddr + data_size + data_size;

			MemEndtAddr = MemStartAddr + data_size + data_size;
		}
	} else if (access == SLR_DI_3A_mode) {
		MemAddr[0] = MemStartAddr;
		MemAddr[1] = MemStartAddr + data_size;
		MemAddr[2] = MemStartAddr + data_size + data_size;
		MemAddr[3] = MemStartAddr + data_size + data_size;
		MemAddr[4] = MemStartAddr + data_size + data_size;
		MemAddr[5] = MemStartAddr + data_size + data_size + info_size;

		MemEndtAddr = MemStartAddr + data_size + data_size + info_size + info_size;
	} else {
		MemAddr[0] = MemStartAddr;
		MemAddr[1] = MemStartAddr + data_size;
		MemAddr[2] = MemStartAddr + data_size + data_size;
		MemAddr[3] = MemStartAddr + data_size + data_size + data_size;
		MemAddr[4] = MemStartAddr + data_size + data_size + data_size + data_size;
		MemAddr[5] = MemStartAddr + data_size + data_size + data_size + data_size + info_size;

		MemEndtAddr = MemStartAddr + data_size + data_size + data_size + data_size + info_size + info_size;
	}

	for (i=0;i<4;i++) {
		if (aligned_val != 0) {
			MemAddr_aligned[i] = dvr_memory_alignment((unsigned long)MemAddr[i], data_size_aligned);
			if (MemAddr_aligned[i] == 0) {
				rtd_pr_scaler_memory_emerg("DI, memory 96 byte alignment fail, data addr=%x, index=%d, data_size_aligned=%d\n",
					MemAddr[i], i,  data_size_aligned);
				BUG();
			}
		} else {
			MemAddr_aligned[i] = ((MemAddr[i]+15)>>4)<<4;//16 byte alignment
		}
	}

	for (i=4;i<6;i++) {
		if (aligned_val != 0 && access != SLR_DI_progress_mode) {
			MemAddr_aligned[i] = dvr_memory_alignment((unsigned long)MemAddr[i], info_size_aligned);
			if (MemAddr_aligned[i] == 0) {
				rtd_pr_scaler_memory_emerg("DI, memory 96 byte alignment fail, data addr=%x, index=%d, data_size_aligned=%d\n",
					MemAddr[i], i,  data_size_aligned);
				BUG();
			}
		} else {
			MemAddr_aligned[i] = ((MemAddr[i]+15)>>4)<<4;//16 byte alignment
		}
	}

	IoReg_Write32((data_addr_reg + 0),  (MemAddr_aligned[0]&0x7FFFFFFF));
	IoReg_Write32((data_addr_reg + 4),  (MemAddr_aligned[1]&0x7FFFFFFF));
	IoReg_Write32((data_addr_reg + 8),  (MemAddr_aligned[2]&0x7FFFFFFF));
	IoReg_Write32((data_addr_reg + 12),  (MemAddr_aligned[3]&0x7FFFFFFF));
	IoReg_Write32((info_addr_reg + 0),  (MemAddr_aligned[4]&0x7FFFFFFF) );
	IoReg_Write32((info_addr_reg + 4),  (MemAddr_aligned[5]&0x7FFFFFFF) );

	rtd_pr_scaler_memory_info("DI_BND=%x,%x, DI MVF Bnd=%x,%x, DI add base=%x,%x,\n",
		IoReg_Read32(DI_DI1_WRRD_BoundaryAddr1_reg), IoReg_Read32(DI_DI1_WRRD_BoundaryAddr2_reg), IoReg_Read32(DI_MVF_WR_BoundaryAddr1_reg),
		IoReg_Read32(DI_MVF_WR_BoundaryAddr2_reg), data_addr_reg, info_addr_reg);
	rtd_pr_scaler_memory_info("DI mem_addr=%x~%x,info_size=%d,data_size=%d,info_size_aligned=%d,data_size_aligned=%d,\n",
		MemStartAddr, MemEndtAddr, info_size, data_size, info_size_aligned, data_size_aligned);
	rtd_pr_scaler_memory_info("MEM addr=%x,%x,%x,%x,%x,%x\n", MemAddr[0], MemAddr[1], MemAddr[2], MemAddr[3], MemAddr[4], MemAddr[5]);
	rtd_pr_scaler_memory_info("MEM_aligned addr=%x,%x,%x,%x,%x,%x\n", MemAddr_aligned[0], MemAddr_aligned[1], MemAddr_aligned[2], MemAddr_aligned[3], MemAddr_aligned[4], MemAddr_aligned[5]);

	MemEndtAddr = MemEndtAddr + 1024; /* avoid trash between DI and MA-SNR*/
	MemEndtAddr = drv_memory_MA_SNR_Config(access, MemEndtAddr, index);
	rtd_pr_scaler_memory_info("MA-SNR mem_end=%x\n", MemEndtAddr);

	return MemStartAddr;
#else
#if 1//defined(ENABLE_DRIVER_I3DDMA)|| defined(ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE)
	//3D mode DI memory allocate x2: 3D frame sequence input video
	if(drvif_scaler3d_get_IDMA3dMode()
		|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
			&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
			&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status())) {
		data_value *= 2;
		info_value *= 2;
	}
#endif

#if 1
		//define at makeconfig
		//patch for DI/RTNR bug(overwrite 512Byte)

	buffer_alloc = drv_memory_Get_di_alloc_buffer_Static(source, isCompression_Mode, access, DI_Width, DI_Height, index);
	data_size = drv_memory_Get_di_data_size_Static(source, isCompression_Mode, access, DI_Width, DI_Height, index);
	info_size = drv_memory_Get_di_info_size_Static(source, isCompression_Mode, access, DI_Width, DI_Height, index);

	if(drvif_scaler3d_get_IDMA3dMode()
		|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
			&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
			&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status())){

			if(access != 2){
				MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, data_value * DIAllocateBlock+info_value*2+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);
			}
			else{
#ifdef ENABLE_DI_COMPRESS
				if(m_rtnrcompress){//compress mode use 2 buffer
					MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, data_value*2+info_value*2+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);
				}
				else
#endif
				{
					MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, data_value+info_value*2+CONFIG_MDOMAIN_BURST_SIZE, MEMALIGN_ANY);
				}
			}
		} else {
			MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, buffer_alloc, MEMALIGN_ANY);
		}
		if(MemStartAddr == _DUMMYADDR){
			rtd_pr_scaler_memory_emerg("set MEMIDX_DI  to size fail !!!\n");
		}
		rtd_pr_scaler_memory_info("DI, mem_sta=%x,platform=%d\n", MemStartAddr, get_platform());
#else
	if(access != 2)
		MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, data_value * DIAllocateBlock+info_value*2, MEMALIGN_ANY);
	else{
		if(m_rtnrcompress)//compress mode use 2 buffer
			MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, data_value*2+info_value*2, MEMALIGN_ANY);
		else
			MemStartAddr = drvif_memory_alloc_block(MEMIDX_DI, data_value+info_value*2, MEMALIGN_ANY);
	}
#endif
	//rtd_pr_scaler_memory_debug("Get ip addr = %x, size=%x\n", MemStartAddr, (data_value * DIAllocateBlock+info_value*2));
#if 1//defined(ENABLE_DRIVER_I3DDMA)|| defined(ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE)
	//3D mode DI memory allocate x2: 3D frame sequence input video
	if(drvif_scaler3d_get_IDMA3dMode()
		|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
			&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
			&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status())) {
		data_value /= 2;
		info_value /= 2;
	}
#endif

	if (index == 1) {
		data_addr_reg = DI_DI_I2R_DATMemoryStartAdd1_reg;
		info_addr_reg = DI_DI_I2R_INFMemoryStartAdd1_reg;
		data_addr_reg_3D = DI_DI_I2R_DATMemoryStartAdd1_3D_reg;
		info_addr_reg_3D = DI_DI_I2R_INFMemoryStartAdd1_3D_reg;

	} else {
		data_addr_reg = DI_DI_DATMemoryStartAdd1_reg;
		info_addr_reg = DI_DI_INFMemoryStartAdd1_reg;
		data_addr_reg_3D = DI_DI_DATMemoryStartAdd1_3D_reg;
		info_addr_reg_3D = DI_DI_INFMemoryStartAdd1_3D_reg;
	}

	if(drvif_scaler3d_get_IDMA3dMode()
			|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
				&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
				&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status()))
	{
		// Set start address for deinterlace
		for(i = 0; i < 4; i++) {
			//rtd_pr_scaler_memory_debug("IP address%x = %x\n", i, MemStartAddr);

			IoReg_Write32((data_addr_reg + (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

			/*if((access != SLR_DI_progress_mode)|| m_rtnrcompress)*/
			if (i < (data_num-1))
				MemStartAddr += data_value;//data_value;
		}

		/*if((access == SLR_DI_progress_mode)&&!m_rtnrcompress){
			MemStartAddr += data_value;
		}*/

		if (data_num != 0)
			MemStartAddr += data_value;

		for(i = 0; i < 2; i++) {
			//rtd_pr_scaler_memory_debug("Info IP address%x = %x\n", i, MemStartAddr);

			IoReg_Write32((info_addr_reg + (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

			/*if(access != SLR_DI_progress_mode)*/
			if (i < (info_num-1))
				MemStartAddr += info_value;
		}

		if (info_num != 0)
			MemStartAddr += info_value;

	}else {
		for(i = 0; i < 4; i++) {
			//rtd_pr_scaler_memory_debug("IP address%x = %x\n", i, MemStartAddr);
			IoReg_Write32((data_addr_reg + (i << 2)),  (MemStartAddr&0x7FFFFFFF) );
			/*if((access != SLR_DI_progress_mode)|| m_rtnrcompress)*/
			if (i < (data_num-1))
				MemStartAddr += data_size;//data_value;
		}
		/*if((access == SLR_DI_progress_mode)&&!m_rtnrcompress){
			MemStartAddr += data_size;
		}*/

		if (data_num != 0)
			MemStartAddr += data_size;

		for(i = 0; i < 2; i++) {
			//rtd_pr_scaler_memory_debug("Info IP address%x = %x\n", i, MemStartAddr);

			IoReg_Write32((info_addr_reg + (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

			/*if(access != SLR_DI_progress_mode)*/
			if (i < (info_num-1))
				MemStartAddr += info_size;
		}

		if (info_num != 0)
			MemStartAddr += info_size;

	}

#if 1//defined(ENABLE_DRIVER_I3DDMA)|| defined(ENABLE_SG_3D_SUPPORT_ALL_SOURCE_3D_MODE)
	if(drvif_scaler3d_get_IDMA3dMode()
		|| (((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() == TRUE)
			&& (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_DISP_3D_3DTYPE)==SLR_3D_FRAME_PACKING)) || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_FRAME_PACKING) == TRUE))
			&& drvif_scaler3d_decide_HDMI_framePacking_interlaced_status())) {
		for(i = 0; i < 4; i++) {
			//rtd_pr_scaler_memory_debug("3D IP address%x = %x\n", i, MemStartAddr);

			IoReg_Write32((data_addr_reg_3D+ (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

			/*if((access != SLR_DI_progress_mode)|| m_rtnrcompress)*/
			if (i < (data_num-1))
				MemStartAddr += data_value;
		}

		/*if((access == SLR_DI_progress_mode) && !m_rtnrcompress){
			MemStartAddr += data_value;
		}*/

		if (data_num != 0)
			MemStartAddr += data_value;

		for(i = 0; i < 2; i++) {
			//rtd_pr_scaler_memory_debug("3D Info IP address%x = %x\n", i, MemStartAddr);

			IoReg_Write32((info_addr_reg_3D + (i << 2)),  (MemStartAddr&0x7FFFFFFF) );

			/*if(access != SLR_DI_progress_mode)*/
			if (i < (info_num-1))
				MemStartAddr += info_value;
		}
	}
#endif
	rtd_pr_scaler_memory_info("DI mem_end=%x\n", MemStartAddr);
	MemStartAddr = drv_memory_MA_SNR_Config(access, MemStartAddr, index);
	rtd_pr_scaler_memory_info("info_size=%d,data_size=%d,mem_end=%x\n", info_size, data_size,MemStartAddr);
	/* move to "void fw_scalerip_set_deXC_XL(void);"*/
	/*MemStartAddr = drv_memory_I_De_XC_DMA_Config(MemStartAddr, 0, source);*/

	return MemStartAddr;
#endif
}

void drv_memory_set_De_Mura(unsigned int memory_Addr_sta)
{
	/*demura_demura_dma_start_addr_RBUS demura_demura_dma_start_addr_reg;*/

	//remove from merlin3
	//IoReg_Write32(DEMURA_DEMURA_DMA_START_ADDR_reg, memory_Addr_sta);
}
#ifdef CONFIG_HW_SUPPORT_I_DE_XC
char  drv_memory_I_De_XC_ON_OFF_Wait(unsigned char En)
{
	de_xcxl_de_xcxl_db_reg_ctl_RBUS de_xcxl_de_xcxl_db_reg_ctl_reg;

	IoReg_ClearBits(DE_XCXL_De_XCXL_CTRL_reg, DE_XCXL_De_XCXL_CTRL_dexc_en_mask);	/* turn off de-xc */
	IoReg_SetBits(DE_XCXL_DE_XCXL_db_reg_ctl_reg, DE_XCXL_DE_XCXL_db_reg_ctl_db_apply_mask);

	de_xcxl_de_xcxl_db_reg_ctl_reg.regValue = IoReg_Read32(DE_XCXL_DE_XCXL_db_reg_ctl_reg);
	while (de_xcxl_de_xcxl_db_reg_ctl_reg.db_apply == 1) {
		de_xcxl_de_xcxl_db_reg_ctl_reg.regValue = IoReg_Read32(DE_XCXL_DE_XCXL_db_reg_ctl_reg);
	}

	return TRUE;
}

extern int VBM_ReturnMemChunk(unsigned long buffer_addr, unsigned long buffer_size);
extern int VBM_BorrowMemChunk(unsigned long *buffer_addr, unsigned long *buffer_size, UINT32 reqSize ,UINT32 type);
extern unsigned long I3DDMA_GetMemChunkAddr(unsigned int size);
extern struct semaphore VPQ_DeXC_MEM_Semaphore;/*For Dexc MEM*/

unsigned int  drv_memory_I_De_XC_DMA_Memory_Get_From_VBM(unsigned char DI_MEM_IDX, unsigned int DI_Width, unsigned int DI_Height, unsigned char Force_Clr)
{
#if 0
	typedef enum _VIP_I_De_XC_MEM_BORROW_MODE{
		De_XC_MEM_UNKNOW = 0,
		De_XC_MEM_From_VBM_Directly,
		De_XC_MEM_From_I3D_DMA,	/* memory will release by itself in i3ddma*/
		I_De_XC_MEM_BORROW_MODE_Max,

	} VIP_I_De_XC_MEM_BORROW_MODE;

	typedef struct _VIP_I_De_XC_MEM {
		unsigned long phyaddr;
		/*unsigned int size;*/
		unsigned int getsize;
		unsigned char Borrow_Mode;

	} VIP_I_De_XC_MEM;

	static VIP_I_De_XC_MEM mem_ctrl[2] = {0};
#endif

	VIP_I_De_XC_MEM *mem_ctrl[2];
//	_system_setting_info* system_info_structure_table = NULL;
	unsigned char add_more = 0x10;
	unsigned int de_xc_size = 0x17CCCD0 + add_more; //  23.8MB   //0x18B8200 + add_more;	/*24.71923828 Mb*/
	unsigned int phyaddr = 0;
	unsigned int getSize = 0;

	unsigned long phyaddr_l = 0;
	unsigned long getSize_l = 0;

	//_system_setting_info* system_info_structure_table = NULL;
	//system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;
	RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (RPC_system_info_structure_table == NULL) {
		rtd_pr_scaler_memory_emerg("De-XC mem, system info structure NULL !!!\n");
		return 0;
	}
	mem_ctrl[0] = &RPC_system_info_structure_table->DE_XCXL_Memory_CTRL[0];
	mem_ctrl[1] = &RPC_system_info_structure_table->DE_XCXL_Memory_CTRL[1];
	down(&VPQ_DeXC_MEM_Semaphore);

	rtd_pr_scaler_memory_info("De-XC mem in VBM function,idx=%d,DI_w=%d,DI_h=%d,F_clr=%d\n",  DI_MEM_IDX, DI_Width, DI_Height, Force_Clr);

	if (Force_Clr == 1) {
		if ((mem_ctrl[DI_MEM_IDX]->phyaddr != 0) && (mem_ctrl[DI_MEM_IDX]->Borrow_Mode == De_XC_MEM_From_VBM_Directly)) {
			rtd_pr_scaler_memory_debug("[%s] call VBM_ReturnMemChunk\n",__FUNCTION__);
		//	rtd_pr_scaler_memory_emerg("De-XC mem force clear, return memroy from VBM = %x\n", mem_ctrl[DI_MEM_IDX]->phyaddr);
			VBM_ReturnMemChunk(mem_ctrl[DI_MEM_IDX]->phyaddr, mem_ctrl[DI_MEM_IDX]->getsize);
		}
		mem_ctrl[DI_MEM_IDX]->phyaddr = 0;
		mem_ctrl[DI_MEM_IDX]->getsize = 0;
		mem_ctrl[DI_MEM_IDX]->Borrow_Mode = De_XC_MEM_UNKNOW;
		rtd_pr_scaler_memory_info("De-XC mem force clear, mem tag reset\n");
	} else {
		if (DbgSclrFlgTkr.Main_Scaler_Stop_flag == FALSE) {	/* connection check. */
			if (mem_ctrl[DI_MEM_IDX]->phyaddr == 0) {
				phyaddr = (unsigned int)I3DDMA_GetMemChunkAddr(de_xc_size);
				if ( phyaddr != 0) {
					mem_ctrl[DI_MEM_IDX]->phyaddr =  phyaddr;//(unsigned int)dvr_to_phys((void*)i3ddmaCtrl.cap_buffer[0].cache);
					mem_ctrl[DI_MEM_IDX]->getsize = de_xc_size;
					mem_ctrl[DI_MEM_IDX]->Borrow_Mode = De_XC_MEM_From_I3D_DMA;
					/*drv_memory_Set_I_De_XC_Mem_Ready_Flag(DI_MEM_IDX, 1);*/	/* num, len, water... is not ready*/
					rtd_pr_scaler_memory_info("De-XC mem alloc from i3ddma\n");

				} else {
					rtd_pr_scaler_memory_debug("[%s] call VBM_BorrowMemChunk\n",__FUNCTION__);
					if (VBM_BorrowMemChunk(&phyaddr_l, &getSize_l, de_xc_size, 0) != 0) {
						mem_ctrl[DI_MEM_IDX]->phyaddr = 0;
						mem_ctrl[DI_MEM_IDX]->getsize = 0;
						mem_ctrl[DI_MEM_IDX]->Borrow_Mode = De_XC_MEM_UNKNOW;
						rtd_pr_scaler_memory_emerg("De-XC alloc idma video sequence memory failed\n");
					} else {
						phyaddr = (unsigned int)phyaddr_l;
						getSize = (unsigned int)getSize_l;
						if (getSize < de_xc_size) {
							mem_ctrl[DI_MEM_IDX]->phyaddr = 0;
							mem_ctrl[DI_MEM_IDX]->getsize = 0;
							mem_ctrl[DI_MEM_IDX]->Borrow_Mode = De_XC_MEM_UNKNOW;
							rtd_pr_scaler_memory_emerg("De-XC Memory alloc is not enough, get_szie=%x,size=%x\n", getSize, de_xc_size);
						} else {
							mem_ctrl[DI_MEM_IDX]->phyaddr =  (unsigned int)phyaddr ;//(unsigned int)dvr_to_phys((void*)i3ddmaCtrl.cap_buffer[0].cache);
							mem_ctrl[DI_MEM_IDX]->getsize = (unsigned int)getSize ;
							mem_ctrl[DI_MEM_IDX]->Borrow_Mode = De_XC_MEM_From_VBM_Directly;
							/*drv_memory_Set_I_De_XC_Mem_Ready_Flag(DI_MEM_IDX, 1);*/	/* num, len, water... is not ready*/
							rtd_pr_scaler_memory_info("De-XC mem alloc from VBM Directly = %x\n", (unsigned int)mem_ctrl[DI_MEM_IDX]->phyaddr);
						}
					}
				}
			} else {
				rtd_pr_scaler_memory_info("De-XC mem already get from %d\n", mem_ctrl[DI_MEM_IDX]->Borrow_Mode);
			}
		} else {
			if ((mem_ctrl[DI_MEM_IDX]->phyaddr != 0) && (mem_ctrl[DI_MEM_IDX]->Borrow_Mode == De_XC_MEM_From_VBM_Directly)) {
				rtd_pr_scaler_memory_debug("[%s] call VBM_ReturnMemChunk\n",__FUNCTION__);
				rtd_pr_scaler_memory_emerg("De-XC mem disconnect, return memroy from VBM = %x\n", (unsigned int)mem_ctrl[DI_MEM_IDX]->phyaddr);
				VBM_ReturnMemChunk(mem_ctrl[DI_MEM_IDX]->phyaddr, mem_ctrl[DI_MEM_IDX]->getsize);
			}
			mem_ctrl[DI_MEM_IDX]->phyaddr = 0;
			mem_ctrl[DI_MEM_IDX]->getsize = 0;
			mem_ctrl[DI_MEM_IDX]->Borrow_Mode = De_XC_MEM_UNKNOW;
			rtd_pr_scaler_memory_info("De-XC Memory alloc, VSC has disconnected\n");
		}
	}

	if (mem_ctrl[DI_MEM_IDX]->phyaddr == 0) {
		/*no memory get, turn off de-xc immediately*/
		IoReg_ClearBits(DE_XCXL_DE_XCXL_db_reg_ctl_reg, DE_XCXL_DE_XCXL_db_reg_ctl_db_en_mask);	/* turn off de-xc double buffer,  turn off de-xc immediately*/
		IoReg_ClearBits(DE_XCXL_De_XCXL_CTRL_reg, DE_XCXL_De_XCXL_CTRL_dexc_en_mask);	/* turn off de-xc */
		drv_memory_Set_I_De_XC_Mem_Ready_Flag(DI_MEM_IDX, 0);
	}

	up(&VPQ_DeXC_MEM_Semaphore);

	return (unsigned int)mem_ctrl[DI_MEM_IDX]->phyaddr;
}

unsigned int  drv_memory_I_De_XC_DMA_Config(unsigned int DI_phyAddr, unsigned char deXC_mode)
{
	typedef struct _VIP_I_De_XC_DMA {
	    unsigned short  dexc_height;
	    unsigned short  dexc_width;
	    //unsigned char   dexc_pixel_bpp;
	    //unsigned char   dexc_frame_format;
	    unsigned char dexc_data_format;
	    unsigned int    dexc_mem_start;
	    unsigned int    dexc_dat_msize;
	    unsigned int    dexc_inf_msize;
	    //unsigned char   dexc_dat_water_r1;
	    unsigned char   dexc_dat_len_r1;
	    unsigned short  dexc_dat_num_r1;
	    //unsigned char   dexc_dat_water_r2;
	    unsigned char   dexc_dat_len_r2;
	    unsigned short  dexc_dat_num_r2;
	    //unsigned char   dexc_dat_water_w;
	    unsigned char   dexc_dat_len_w;
	    unsigned short  dexc_dat_num_w;
	    unsigned char   dexc_wdma_remain_en;
	    unsigned char   dexc_dat_remain_w;
	    unsigned char   dexc_rdma1_remain_en;
	    unsigned char   dexc_dat_remain_r1;
	    unsigned char   dexc_rdma2_remain_en;
	    unsigned char   dexc_dat_remain_r2;
	    //unsigned char   dexc_inf_water_r;
	    unsigned char   dexc_inf_len_r;
	    //unsigned short  dexc_inf_num_r;
	    //unsigned char   dexc_inf_water_w;
	    unsigned char   dexc_inf_len_w;
	    //unsigned short  dexc_inf_num_w;
	    //unsigned char   dexc_inf_remain_w_en;
	    //unsigned char   dexc_inf_remain_w;
	    //unsigned char   dexc_inf_remain_r_en;
	    //unsigned char   dexc_inf_remain_r;
	    unsigned char dexc_inf_cmp_enable;
	    unsigned char dexc_info_rolling_en;
	} VIP_I_De_XC_DMA;

	VIP_I_De_XC_DMA data;

	/*unsigned int phyAddr = NULL;*/
	unsigned int phyAddr = 0;
	unsigned int height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
	unsigned int width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
	unsigned char is422;
	unsigned char is8bit;
	unsigned char data_format; // 0: y10c6, 1: y8c8
	unsigned char isInfo_Compress;
	unsigned char isInfo_rolling;
	unsigned char DI_MEM_IDX;
	// info comp. default on

	UINT32 field_size, info_size;
	UINT32 data_bit, info_bit;
	UINT32 mem_end = 0;

	int yuv_size;
	int data_len;
	// dma settings
	UINT32 bus_rw_bits = 128;
	UINT32 rw_num, num, remain, length, info_len, dat_len;
	UINT32 total;
	unsigned short aligned_val;

#ifdef CONFIG_I2RND_DUAL_VO
	DI_MEM_IDX = (Scaler_I2rnd_get_display()<=1)?(0):(1);	// Scaler_I2rnd_get_display() ==> 0: i2r disable, 1:i2r s0, 2:i2r s1
#else
	DI_MEM_IDX = 0;
#endif

	if (deXC_mode == DeXCXL_Y10C6) {
		data_format = 0;
		isInfo_Compress = 0;
		isInfo_rolling =0;
	} else if (deXC_mode == DeXCXL_Y8C8) {
		data_format = 1;
		isInfo_Compress = 0;
		isInfo_rolling = 0;
	} else if (deXC_mode == DeXCXL_Y10C6_CMP) {
		data_format = 0;
		isInfo_Compress = 1;
		isInfo_rolling = 1;
	} else if (deXC_mode == DeXCXL_Y8C8_CMP) {
		data_format = 1;
		isInfo_Compress = 1;
		isInfo_rolling = 1;
	} else {
		drv_memory_I_De_XC_DMA_Memory_Get_From_VBM(DI_MEM_IDX, 0, 0, 1);		/* force to release memory*/

		rtd_pr_scaler_memory_info("DE-XCXL, disable, deXC_mode=%d\n", deXC_mode);
		return 0;
	}

	phyAddr = drv_memory_I_De_XC_DMA_Memory_Get_From_VBM(DI_MEM_IDX, width, height, 0);
	rtd_pr_scaler_memory_info("DE-XCXL, get memory, memStaAddr=%x\n", phyAddr);


	if(phyAddr == 0) {
		rtd_pr_scaler_memory_emerg("De-XC alloc memory failed, diable De-XC, return !!!!!!!!!!!\n");
		return 0;
	}
#if 0
	if( height > 540  || is_usbPlayback == 1)
		is422 = 1; // 422 for 1080p and usbplayback
	else
		is422 = 0;
#endif

#ifdef CONFIG_BW_96B_ALIGNED
	aligned_val = 6;
#else
	aligned_val = 0;
#endif

	is8bit = 1; // fixed to 422, 8-bit, Merlin4
	is422 = 1;

	yuv_size = is422 ? 2 : 3;
	data_len = is8bit ? 8 : 10;
	data_bit = yuv_size * data_len;
	info_bit = (isInfo_Compress == 0)?(22):(24);	// for m4, // normal case 16, worth case 24 for info compression
	info_len = 10;
	dat_len = 32;

	data.dexc_inf_cmp_enable = isInfo_Compress;
	data.dexc_info_rolling_en = isInfo_rolling;
	data.dexc_height = height;
	data.dexc_width = width;
	//data.dexc_pixel_bpp = is8bit;
	//data.dexc_frame_format = is422;
	data.dexc_data_format = data_format;

	// address calculation
	field_size = width * height * data_bit; // bit
	if( (field_size & 0x7F) > 0 )
		field_size = ( (field_size>>7)+1 ) << 4; // round up on 16-byte unit
	else
		field_size = field_size >> 3; // byte

	if (isInfo_rolling == 1) {
		//info_size = (ceil( ceil((height * (width>>1) * info_bit) / 128) *2)/ info_len) * info_len;
		length = info_len;
		if (aligned_val != 0)
			length = ((length%aligned_val)==0)?(length):(length + (aligned_val - (length % aligned_val)));	/* lenght aligned with "aligned_val" */

		info_size = ((height * (width>>1) * info_bit)+127)>>7;	// unit 128 bit, and ceil() with 128b
		info_size = ((info_size + (length-1)) / length) * length; // ceil() with "info_len"
		info_size = info_size<<4;	// unit byte

	} else {
		info_size = ((width+1)>>1) * height * info_bit; // bit
		if( (info_size & 0x7F) > 0 )
			info_size = ( (info_size>>7)+1 ) << 4; // round up on 16-byte unit
		else
			info_size = info_size >> 3; // byte
	}
	/*// test code for de_xcxl DMA settings, 20160722 jimmy
	if(phyAddr!=NULL)
		pli_free(phyAddr);

	// temp code for memory allocation
	phyAddr = pli_malloc( tot_size, GFP_DCU2_FIRST );*/

	/*info_size += info_size;*/
	/*field_size += 0x10;*/

	data.dexc_mem_start = phyAddr;
	data.dexc_dat_msize = field_size;
	data.dexc_inf_msize = info_size;

	// data dma
	length = dat_len;
	if (aligned_val != 0)
		length = ((length%aligned_val)==0)?(length):(length + (aligned_val - (length % aligned_val)));	/* lenght aligned with "aligned_val" */

	rw_num = width*height*data_bit;
	if( rw_num % bus_rw_bits > 0 )
		rw_num = rw_num / bus_rw_bits + 1;
	else
		rw_num = rw_num / bus_rw_bits;

	num = rw_num / length;
	remain= rw_num - (num*length);
	if( remain==0 )
	{
		remain = length;
		num--;
	}
	total = 128;

	//data.dexc_dat_water_r1 = total-length;
	data.dexc_dat_len_r1 = length;
	data.dexc_dat_num_r1 = num;

	//data.dexc_dat_water_r2 = total-length;
	data.dexc_dat_len_r2 = length;
	data.dexc_dat_num_r2 = num;

	//data.dexc_dat_water_w = length;
	data.dexc_dat_len_w = length;
	data.dexc_dat_num_w = num;

	data.dexc_wdma_remain_en = 1;
	data.dexc_dat_remain_w = remain;
	data.dexc_rdma1_remain_en = 1;
	data.dexc_dat_remain_r1 = remain;
	data.dexc_rdma2_remain_en = 1;
	data.dexc_dat_remain_r2 = remain;

	// info dma
	length = info_len;
	if (aligned_val != 0)
		length = ((length%aligned_val)==0)?(length):(length + (aligned_val - (length % aligned_val)));	/* lenght aligned with "aligned_val" */

	rw_num = width*height*info_bit/2;
	if( rw_num % bus_rw_bits > 0 )
		rw_num = rw_num / bus_rw_bits + 1;
	else
		rw_num = rw_num / bus_rw_bits;

	num = rw_num / length;
	remain= rw_num - (num*length);
	if( remain==0 )
	{
		remain = length;
		num--;
	}
	total = 32;

	//data.dexc_inf_water_r = total-length;
	data.dexc_inf_len_r = length;
	//data.dexc_inf_num_r = num;

	//data.dexc_inf_water_w = length;
	data.dexc_inf_len_w = length;
	//data.dexc_inf_num_w = num;

	//data.dexc_inf_remain_w_en = 1;
	//data.dexc_inf_remain_w = remain;
	//data.dexc_inf_remain_r_en = 1;
	//data.dexc_inf_remain_r = remain;

	mem_end = drv_memory_Set_I_De_XC_DMA( (void*)&data, DI_MEM_IDX);

	if (mem_end == 0)
		drv_memory_Set_I_De_XC_Mem_Ready_Flag(DI_MEM_IDX, 0);
	else
		drv_memory_Set_I_De_XC_Mem_Ready_Flag(DI_MEM_IDX, 1);

	/*de-xc need a few frame for memory stable after enable, enable control @ scalerMemory*/
	//Scaler_Set_I_De_XC(0);

	rtd_pr_scaler_memory_info("DE-XCXL, idx=%d,mem sta=%x,mem end=%x, Ready Flag=%d\n",
		DI_MEM_IDX, phyAddr, mem_end, drv_memory_Get_I_De_XC_Mem_Ready_Flag(DI_MEM_IDX));

	return mem_end;
}

unsigned int drv_memory_Set_I_De_XC_DMA(void *dma_data, unsigned char DI_MEM_IDX)
{
	typedef struct _VIP_I_De_XC_DMA {
	    unsigned short  dexc_height;
	    unsigned short  dexc_width;
	    //unsigned char   dexc_pixel_bpp;
	    //unsigned char   dexc_frame_format;
	    unsigned char  dexc_data_format;
	    unsigned int    dexc_mem_start;
	    unsigned int    dexc_dat_msize;
	    unsigned int    dexc_inf_msize;
	    //unsigned char   dexc_dat_water_r1;
	    unsigned char   dexc_dat_len_r1;
	    unsigned short  dexc_dat_num_r1;
	    //unsigned char   dexc_dat_water_r2;
	    unsigned char   dexc_dat_len_r2;
	    unsigned short  dexc_dat_num_r2;
	    //unsigned char   dexc_dat_water_w;
	    unsigned char   dexc_dat_len_w;
	    unsigned short  dexc_dat_num_w;
	    unsigned char   dexc_wdma_remain_en;
	    unsigned char   dexc_dat_remain_w;
	    unsigned char   dexc_rdma1_remain_en;
	    unsigned char   dexc_dat_remain_r1;
	    unsigned char   dexc_rdma2_remain_en;
	    unsigned char   dexc_dat_remain_r2;
	    //unsigned char   dexc_inf_water_r;
	    unsigned char   dexc_inf_len_r;
	    //unsigned short  dexc_inf_num_r;
	    //unsigned char   dexc_inf_water_w;
	    unsigned char   dexc_inf_len_w;
	    //unsigned short  dexc_inf_num_w;
	    //unsigned char   dexc_inf_remain_w_en;
	    //unsigned char   dexc_inf_remain_w;
	    //unsigned char   dexc_inf_remain_r_en;
	    //unsigned char   dexc_inf_remain_r;
	    unsigned char dexc_inf_cmp_enable;
		unsigned char dexc_info_rolling_en;
	} VIP_I_De_XC_DMA;
	unsigned int i;
	UINT32 baseAddr;
	//UINT32 nBaseAddr;
	//UINT32 nFldSize;
	//UINT32 nInfSize;
	UINT32 tmpAddr[8];
	UINT32 align_addr_aligned[8];
	UINT32 field_size, info_size, field_size_aligned, info_size_aligned;
	UINT32 mem_end;
	unsigned short aligned_val;
	//UINT32 data_bit, info_bit;
	VIP_I_De_XC_DMA *ptr = (VIP_I_De_XC_DMA*) dma_data;
	de_xcxl_de_xcxl_ctrl_RBUS de_xcxl_ctrl_reg;
	de_xcxl_frame_format_RBUS de_xcxl_frame_format_reg;
	//de_xcxl_datmemstartadd1_s0_RBUS 		de_xcxl_datmemstartadd1_s0_reg;
	de_xcxl_datmemstartadd2_s0_RBUS 		de_xcxl_datmemstartadd2_s0_reg;
	de_xcxl_datmemstartadd3_s0_RBUS 		de_xcxl_datmemstartadd3_s0_reg;
	de_xcxl_infmemstartadd_s0_RBUS		    de_xcxl_infmemstartadd_s0_reg;
	de_xcxl_infmemstartadd_1_s0_RBUS de_xcxl_infmemstartadd_1_s0_reg;		// m4
	//de_xcxl_datmemstartadd1_odd_s0_RBUS 	de_xcxl_datmemstartadd1_odd_s0_reg;
	de_xcxl_datmemstartadd2_odd_s0_RBUS 	de_xcxl_datmemstartadd2_odd_s0_reg;
	de_xcxl_datmemstartadd3_odd_s0_RBUS 	de_xcxl_datmemstartadd3_odd_s0_reg;
	de_xcxl_infmemstartadd_odd_s0_RBUS	    de_xcxl_infmemstartadd_odd_s0_reg;
	de_xcxl_infmemstartadd_1_odd_s0_RBUS de_xcxl_infmemstartadd_1_odd_s0_reg;		// m4

	//de_xcxl_datmemstartadd1_s1_RBUS 		de_xcxl_datmemstartadd1_s1_reg;
	de_xcxl_datmemstartadd2_s1_RBUS 		de_xcxl_datmemstartadd2_s1_reg;
	de_xcxl_datmemstartadd3_s1_RBUS 		de_xcxl_datmemstartadd3_s1_reg;
	de_xcxl_infmemstartadd_s1_RBUS		    de_xcxl_infmemstartadd_s1_reg;
	de_xcxl_infmemstartadd_1_s1_RBUS	de_xcxl_infmemstartadd_1_s1_reg;		// m4
	//de_xcxl_datmemstartadd1_odd_s1_RBUS 	de_xcxl_datmemstartadd1_odd_s1_reg;
	de_xcxl_datmemstartadd2_odd_s1_RBUS 	de_xcxl_datmemstartadd2_odd_s1_reg;
	de_xcxl_datmemstartadd3_odd_s1_RBUS 	de_xcxl_datmemstartadd3_odd_s1_reg;
	de_xcxl_infmemstartadd_odd_s1_RBUS	    de_xcxl_infmemstartadd_odd_s1_reg;
	de_xcxl_infmemstartadd_odd_1_s1_RBUS		de_xcxl_infmemstartadd_odd_1_s1_reg;		// m4

	de_xcxl_de_xcxl_db_reg_ctl_RBUS	de_xcxl_de_xcxl_db_reg_ctl_reg;
	de_xcxl_wrrd_bndaddr1_RBUS de_xcxl_wrrd_bndaddr1_reg;
	de_xcxl_wrrd_bndaddr2_RBUS de_xcxl_wrrd_bndaddr2_reg;
	de_xcxl_rdatdma_set1_RBUS de_xcxl_rdatdma_set1_reg;
	de_xcxl_rdatdma_set2_RBUS de_xcxl_rdatdma_set2_reg;
	de_xcxl_wdatdma_set_RBUS de_xcxl_wdatdma_set_reg;
	de_xcxl_datdma_rm_set_RBUS de_xcxl_datdma_rm_set_reg;
	de_xcxl_rinfdma_set_RBUS de_xcxl_rinfdma_set_reg;
	de_xcxl_winfdma_set_RBUS de_xcxl_winfdma_set_reg;
	//de_xcxl_infdma_rm_set_RBUS de_xcxl_infdma_rm_set_reg;
	de_xcxl_dma_multi_req_set_RBUS	de_xcxl_dma_multi_req_set_reg;

#ifdef CONFIG_BW_96B_ALIGNED
	aligned_val = 6;
#else
	aligned_val = 0;
#endif

	/* db enable*/
	IoReg_SetBits(DE_XCXL_DE_XCXL_db_reg_ctl_reg, DE_XCXL_DE_XCXL_db_reg_ctl_db_en_mask);

	// data format and CMP
	de_xcxl_ctrl_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_CTRL_reg);
	de_xcxl_ctrl_reg.dexc_data_format = ptr->dexc_data_format;
	de_xcxl_ctrl_reg.dexc_info_comp = ptr->dexc_inf_cmp_enable;
	de_xcxl_ctrl_reg.dexc_info_rolling_en = ptr->dexc_info_rolling_en;
	IoReg_Write32( DE_XCXL_De_XCXL_CTRL_reg, de_xcxl_ctrl_reg.regValue );

	// frame format
	de_xcxl_frame_format_reg.regValue = IoReg_Read32(DE_XCXL_Frame_format_reg);

	de_xcxl_frame_format_reg.height         = ptr->dexc_height;
	de_xcxl_frame_format_reg.width          = ptr->dexc_width;
	//de_xcxl_frame_format_reg.pixel_bpp      = ptr->dexc_pixel_bpp;
	//de_xcxl_frame_format_reg.frame_format   = ptr->dexc_frame_format;

	IoReg_Write32( DE_XCXL_Frame_format_reg, de_xcxl_frame_format_reg.regValue );

 	// address calculation
	baseAddr    = ptr->dexc_mem_start;
	field_size  = ptr->dexc_dat_msize;
	info_size   = ptr->dexc_inf_msize;

	if (aligned_val != 0) {
		field_size_aligned = dvr_size_alignment(field_size);
		info_size_aligned = dvr_size_alignment(info_size);
	} else {
		field_size_aligned = field_size;
		info_size_aligned = info_size;
	}
	//mem_end = baseAddr + field_size *6 + info_size *2;

	//nBaseAddr = (baseAddr>>3)&0x1fffffff;
	//nFldSize = (field_size>>3)&0x1fffffff; // 8-byte
	//nInfSize = (info_size>>3)&0x1fffffff;

	//remove addr1 from merlin3 @Crixus 20170515
	//de_xcxl_datmemstartadd1_s0_reg.regValue = IoReg_Read32(DE_XCXL_DATMemStartAdd1_S0_reg);
	de_xcxl_datmemstartadd2_s0_reg.regValue = IoReg_Read32(DE_XCXL_DATMemStartAdd2_S0_reg);
	de_xcxl_datmemstartadd3_s0_reg.regValue = IoReg_Read32(DE_XCXL_DATMemStartAdd3_S0_reg);
	de_xcxl_infmemstartadd_s0_reg.regValue = IoReg_Read32(DE_XCXL_INFMemStartAdd_S0_reg);
	de_xcxl_infmemstartadd_1_s0_reg.regValue  = IoReg_Read32( DE_XCXL_INFMemStartAdd_1_S0_reg);
	//de_xcxl_datmemstartadd1_odd_s0_reg.regValue = IoReg_Read32(DE_XCXL_DATMemStartAdd1_Odd_S0_reg);
	de_xcxl_datmemstartadd2_odd_s0_reg.regValue = IoReg_Read32(DE_XCXL_DATMemStartAdd2_Odd_S0_reg);
	de_xcxl_datmemstartadd3_odd_s0_reg.regValue = IoReg_Read32(DE_XCXL_DATMemStartAdd3_Odd_S0_reg);
	de_xcxl_infmemstartadd_odd_s0_reg.regValue = IoReg_Read32(DE_XCXL_INFMemStartAdd_Odd_S0_reg);
	de_xcxl_infmemstartadd_1_odd_s0_reg.regValue = IoReg_Read32( DE_XCXL_INFMemStartAdd_1_Odd_S0_reg);

	//de_xcxl_datmemstartadd1_s1_reg.regValue = IoReg_Read32(DE_XCXL_DATMemStartAdd1_S1_reg);
	de_xcxl_datmemstartadd2_s1_reg.regValue = IoReg_Read32(DE_XCXL_DATMemStartAdd2_S1_reg);
	de_xcxl_datmemstartadd3_s1_reg.regValue = IoReg_Read32(DE_XCXL_DATMemStartAdd3_S1_reg);
	de_xcxl_infmemstartadd_s1_reg.regValue = IoReg_Read32(DE_XCXL_INFMemStartAdd_S1_reg);
	de_xcxl_infmemstartadd_1_s1_reg.regValue = IoReg_Read32( DE_XCXL_INFMemStartAdd_1_S1_reg);
	//de_xcxl_datmemstartadd1_odd_s1_reg.regValue = IoReg_Read32(DE_XCXL_DATMemStartAdd1_Odd_S1_reg);
	de_xcxl_datmemstartadd2_odd_s1_reg.regValue = IoReg_Read32(DE_XCXL_DATMemStartAdd2_Odd_S1_reg);
	de_xcxl_datmemstartadd3_odd_s1_reg.regValue = IoReg_Read32(DE_XCXL_DATMemStartAdd3_Odd_S1_reg);
	de_xcxl_infmemstartadd_odd_s1_reg.regValue = IoReg_Read32(DE_XCXL_INFMemStartAdd_Odd_S1_reg);
	de_xcxl_infmemstartadd_odd_1_s1_reg.regValue = IoReg_Read32( DE_XCXL_INFMemStartAdd_Odd_1_S1_reg);

	// Even
	tmpAddr[0] = baseAddr;
	tmpAddr[1] = tmpAddr[0] + field_size_aligned;
	tmpAddr[2] = tmpAddr[1] + field_size_aligned;
	if (ptr->dexc_inf_cmp_enable == 1)
		tmpAddr[3] = tmpAddr[2] + info_size_aligned;
	else
		tmpAddr[3] = tmpAddr[2];
	//Odd
	tmpAddr[4] = tmpAddr[3] + info_size_aligned;
	tmpAddr[5] = tmpAddr[4] + field_size_aligned;
	tmpAddr[6] = tmpAddr[5] + field_size_aligned;
	if (ptr->dexc_inf_cmp_enable == 1)
		tmpAddr[7] = tmpAddr[6] + info_size_aligned;
	else
		tmpAddr[7] = tmpAddr[6];

	if (aligned_val != 0) {
		// Even
		align_addr_aligned[0] = dvr_memory_alignment((unsigned long)tmpAddr[0], field_size_aligned);
		align_addr_aligned[1] = dvr_memory_alignment((unsigned long)tmpAddr[1], field_size_aligned);
		align_addr_aligned[2] = dvr_memory_alignment((unsigned long)tmpAddr[2], info_size_aligned);
		if (ptr->dexc_inf_cmp_enable == 1)
			align_addr_aligned[3] = dvr_memory_alignment((unsigned long)tmpAddr[3], info_size_aligned);
		else
			align_addr_aligned[3] = align_addr_aligned[2];
		// Odd
		align_addr_aligned[4] = dvr_memory_alignment((unsigned long)tmpAddr[4], field_size_aligned);
		align_addr_aligned[5] = dvr_memory_alignment((unsigned long)tmpAddr[5], field_size_aligned);
		align_addr_aligned[6] = dvr_memory_alignment((unsigned long)tmpAddr[6], info_size_aligned);
		if (ptr->dexc_inf_cmp_enable == 1)
			align_addr_aligned[7] = dvr_memory_alignment((unsigned long)tmpAddr[7], info_size_aligned);
		else
			align_addr_aligned[7] = align_addr_aligned[6];

		for (i=0;i<8;i++) {
			if (align_addr_aligned[i] == 0) {
				rtd_pr_scaler_memory_emerg("DE-XCXL, memory 96 byte alignment fail, addr=%x, index=%d, field_size_96=%d, info_size_96=%d\n",
					tmpAddr[i], i, field_size_aligned, info_size_aligned);
				BUG();
			}
		}
	} else {
		// Even
		align_addr_aligned[0] = tmpAddr[0];
		align_addr_aligned[1] = tmpAddr[1];
		align_addr_aligned[2] = tmpAddr[2];
		align_addr_aligned[3] = tmpAddr[3];
		// Odd
		align_addr_aligned[4] = tmpAddr[4];
		align_addr_aligned[5] = tmpAddr[5];
		align_addr_aligned[6] = tmpAddr[6];
		align_addr_aligned[7] = tmpAddr[7];
	}

	// memory end
	mem_end = align_addr_aligned[7] + info_size_aligned;

	if (DI_MEM_IDX == 0) {
		// even
		de_xcxl_datmemstartadd2_s0_reg.s0_dat_mstart_57_29 = (align_addr_aligned[0]>>3)&0x1fffffff;
		de_xcxl_datmemstartadd3_s0_reg.s0_dat_mstart_28_0 = (align_addr_aligned[1]>>3)&0x1fffffff;
		de_xcxl_infmemstartadd_s0_reg.s0_inf_mstart = (align_addr_aligned[2]>>3)&0x1fffffff;
		de_xcxl_infmemstartadd_1_s0_reg.s0_1_inf_mstart = (align_addr_aligned[3]>>3)&0x1fffffff;
		// Odd
		de_xcxl_datmemstartadd2_odd_s0_reg.s0_dat_mstart_odd_57_29 = (align_addr_aligned[4]>>3)&0x1fffffff;
		de_xcxl_datmemstartadd3_odd_s0_reg.s0_dat_mstart_odd_28_0 = (align_addr_aligned[5]>>3)&0x1fffffff;
		de_xcxl_infmemstartadd_odd_s0_reg.s0_inf_mstart_odd = (align_addr_aligned[6]>>3)&0x1fffffff;
		de_xcxl_infmemstartadd_1_odd_s0_reg.s0_1_inf_mstart_odd = (align_addr_aligned[7]>>3)&0x1fffffff;

		IoReg_Write32( DE_XCXL_DATMemStartAdd2_S0_reg, de_xcxl_datmemstartadd2_s0_reg.regValue );
		IoReg_Write32( DE_XCXL_DATMemStartAdd3_S0_reg, de_xcxl_datmemstartadd3_s0_reg.regValue );
		IoReg_Write32( DE_XCXL_INFMemStartAdd_S0_reg, de_xcxl_infmemstartadd_s0_reg.regValue );
		IoReg_Write32( DE_XCXL_INFMemStartAdd_1_S0_reg, de_xcxl_infmemstartadd_1_s0_reg.regValue );
		IoReg_Write32( DE_XCXL_DATMemStartAdd2_Odd_S0_reg, de_xcxl_datmemstartadd2_odd_s0_reg.regValue );
		IoReg_Write32( DE_XCXL_DATMemStartAdd3_Odd_S0_reg, de_xcxl_datmemstartadd3_odd_s0_reg.regValue );
		IoReg_Write32( DE_XCXL_INFMemStartAdd_Odd_S0_reg, de_xcxl_infmemstartadd_odd_s0_reg.regValue );
		IoReg_Write32( DE_XCXL_INFMemStartAdd_1_Odd_S0_reg, de_xcxl_infmemstartadd_1_odd_s0_reg.regValue );
	} else {
		// even
		de_xcxl_datmemstartadd2_s1_reg.s1_dat_mstart_57_29 = (align_addr_aligned[0]>>3)&0x1fffffff;
		de_xcxl_datmemstartadd3_s1_reg.s1_dat_mstart_28_0 = (align_addr_aligned[1]>>3)&0x1fffffff;
		de_xcxl_infmemstartadd_s1_reg.s1_inf_mstart = (align_addr_aligned[2]>>3)&0x1fffffff;
		de_xcxl_infmemstartadd_1_s1_reg.s1_1_inf_mstart = (align_addr_aligned[3]>>3)&0x1fffffff;
		// Odd
		de_xcxl_datmemstartadd2_odd_s1_reg.s1_dat_mstart_odd_57_29 = (align_addr_aligned[4]>>3)&0x1fffffff;
		de_xcxl_datmemstartadd3_odd_s1_reg.s1_dat_mstart_odd_28_0 = (align_addr_aligned[5]>>3)&0x1fffffff;
		de_xcxl_infmemstartadd_odd_s1_reg.s1_inf_mstart_odd = (align_addr_aligned[6]>>3)&0x1fffffff;
		de_xcxl_infmemstartadd_odd_1_s1_reg.s1_1_inf_mstart_odd = (align_addr_aligned[7]>>3)&0x1fffffff;

		IoReg_Write32( DE_XCXL_DATMemStartAdd2_S1_reg, de_xcxl_datmemstartadd2_s1_reg.regValue );
		IoReg_Write32( DE_XCXL_DATMemStartAdd3_S1_reg, de_xcxl_datmemstartadd3_s1_reg.regValue );
		IoReg_Write32( DE_XCXL_INFMemStartAdd_S1_reg, de_xcxl_infmemstartadd_s1_reg.regValue );
		IoReg_Write32( DE_XCXL_INFMemStartAdd_1_S1_reg, de_xcxl_infmemstartadd_1_s1_reg.regValue );
		IoReg_Write32( DE_XCXL_DATMemStartAdd2_Odd_S1_reg, de_xcxl_datmemstartadd2_odd_s1_reg.regValue );
		IoReg_Write32( DE_XCXL_DATMemStartAdd3_Odd_S1_reg, de_xcxl_datmemstartadd3_odd_s1_reg.regValue );
		IoReg_Write32( DE_XCXL_INFMemStartAdd_Odd_S1_reg, de_xcxl_infmemstartadd_odd_s1_reg.regValue );
		IoReg_Write32( DE_XCXL_INFMemStartAdd_Odd_1_S1_reg, de_xcxl_infmemstartadd_odd_1_s1_reg.regValue );
	}
	// boundary address

	de_xcxl_wrrd_bndaddr1_reg.regValue = IoReg_Read32(DE_XCXL_WRRD_BndAddr1_reg);
	de_xcxl_wrrd_bndaddr2_reg.regValue = IoReg_Read32(DE_XCXL_WRRD_BndAddr2_reg);

	de_xcxl_wrrd_bndaddr1_reg.dexcxl_wrrd_downlimit_adr = (tmpAddr[0]>>3)&0x1fffffff;
	de_xcxl_wrrd_bndaddr2_reg.dexcxl_wrrd_uplimit_adr = (mem_end>>3)&0x1fffffff;

	IoReg_Write32( DE_XCXL_WRRD_BndAddr1_reg, de_xcxl_wrrd_bndaddr1_reg.regValue );
	IoReg_Write32( DE_XCXL_WRRD_BndAddr2_reg, de_xcxl_wrrd_bndaddr2_reg.regValue );

	// dma settings
	de_xcxl_rdatdma_set1_reg.regValue = IoReg_Read32(DE_XCXL_RDATDMA_Set1_reg);
	//de_xcxl_rdatdma_set1_reg.dat_water_r1   = ptr->dexc_dat_water_r1;
	de_xcxl_rdatdma_set1_reg.dat_len_r1     = ptr->dexc_dat_len_r1;
	de_xcxl_rdatdma_set1_reg.dat_num_r1     = ptr->dexc_dat_num_r1;
	IoReg_Write32( DE_XCXL_RDATDMA_Set1_reg, de_xcxl_rdatdma_set1_reg.regValue );

	de_xcxl_rdatdma_set2_reg.regValue = IoReg_Read32(DE_XCXL_RDATDMA_Set2_reg);
	//de_xcxl_rdatdma_set2_reg.dat_water_r2   = ptr->dexc_dat_water_r2;
	de_xcxl_rdatdma_set2_reg.dat_len_r2     = ptr->dexc_dat_len_r2;
	de_xcxl_rdatdma_set2_reg.dat_num_r2     = ptr->dexc_dat_num_r2;
	IoReg_Write32( DE_XCXL_RDATDMA_Set2_reg, de_xcxl_rdatdma_set2_reg.regValue );

	de_xcxl_wdatdma_set_reg.regValue = IoReg_Read32(DE_XCXL_WDATDMA_Set_reg);
	//de_xcxl_wdatdma_set_reg.dat_water_w     = ptr->dexc_dat_water_w;
	de_xcxl_wdatdma_set_reg.dat_len_w       = ptr->dexc_dat_len_w;
	de_xcxl_wdatdma_set_reg.dat_num_w       = ptr->dexc_dat_num_w;
	IoReg_Write32( DE_XCXL_WDATDMA_Set_reg, de_xcxl_wdatdma_set_reg.regValue );

	de_xcxl_datdma_rm_set_reg.regValue = IoReg_Read32(DE_XCXL_DATDMA_RM_Set_reg);
	de_xcxl_datdma_rm_set_reg.wdma_remain_en    = ptr->dexc_wdma_remain_en;
	de_xcxl_datdma_rm_set_reg.dat_remain_w      = ptr->dexc_dat_remain_w;
	de_xcxl_datdma_rm_set_reg.rdma1_remain_en   = ptr->dexc_rdma1_remain_en;
	de_xcxl_datdma_rm_set_reg.dat_remain_r1     = ptr->dexc_dat_remain_r1;
	de_xcxl_datdma_rm_set_reg.rdma2_remain_en   = ptr->dexc_rdma2_remain_en;
	de_xcxl_datdma_rm_set_reg.dat_remain_r2     = ptr->dexc_dat_remain_r2;
	IoReg_Write32( DE_XCXL_DATDMA_RM_Set_reg, de_xcxl_datdma_rm_set_reg.regValue );

	de_xcxl_rinfdma_set_reg.regValue = IoReg_Read32(DE_XCXL_RINFDMA_Set_reg);
	//de_xcxl_rinfdma_set_reg.inf_water_r = ptr->dexc_inf_water_r;
	de_xcxl_rinfdma_set_reg.inf_len_r   = ptr->dexc_inf_len_r;
	//de_xcxl_rinfdma_set_reg.inf_num_r   = ptr->dexc_inf_num_r;
	IoReg_Write32( DE_XCXL_RINFDMA_Set_reg, de_xcxl_rinfdma_set_reg.regValue );

	de_xcxl_winfdma_set_reg.regValue = IoReg_Read32(DE_XCXL_WINFDMA_Set_reg);
	//de_xcxl_winfdma_set_reg.inf_water_w = ptr->dexc_inf_water_w;
	de_xcxl_winfdma_set_reg.inf_len_w   = ptr->dexc_inf_len_w;
	//de_xcxl_winfdma_set_reg.inf_num_w   = ptr->dexc_inf_num_w;
	IoReg_Write32( DE_XCXL_WINFDMA_Set_reg, de_xcxl_winfdma_set_reg.regValue );

	//de_xcxl_infdma_rm_set_reg.regValue = IoReg_Read32(DE_XCXL_INFDMA_RM_Set_reg);
	//de_xcxl_infdma_rm_set_reg.inf_remain_w_en   = ptr->dexc_inf_remain_w_en;
	//de_xcxl_infdma_rm_set_reg.inf_remain_w      = ptr->dexc_inf_remain_w;
	//de_xcxl_infdma_rm_set_reg.inf_remain_r_en   = ptr->dexc_inf_remain_r_en;
	//de_xcxl_infdma_rm_set_reg.inf_remain_r      = ptr->dexc_inf_remain_r;
	//IoReg_Write32( DE_XCXL_INFDMA_RM_Set_reg, de_xcxl_infdma_rm_set_reg.regValue );

	de_xcxl_dma_multi_req_set_reg.regValue = IoReg_Read32(DE_XCXL_DMA_Multi_Req_Set_reg);
	de_xcxl_dma_multi_req_set_reg.rdma_req_num = 8;
	de_xcxl_dma_multi_req_set_reg.wdma_req_num = 8;
	de_xcxl_dma_multi_req_set_reg.dma_info_first = 1;
	IoReg_Write32( DE_XCXL_DMA_Multi_Req_Set_reg, de_xcxl_dma_multi_req_set_reg.regValue );

	de_xcxl_de_xcxl_db_reg_ctl_reg.regValue = IoReg_Read32(DE_XCXL_DE_XCXL_db_reg_ctl_reg);
	de_xcxl_de_xcxl_db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32( DE_XCXL_DE_XCXL_db_reg_ctl_reg, de_xcxl_de_xcxl_db_reg_ctl_reg.regValue );

	//mem_end = tmpAddr<<3;

	rtd_pr_scaler_memory_info("DE-XCXL, tmpAddr[0:7]=%x,%x,%x,%x,%x,%x,%x,%x,\n",
		tmpAddr[0], tmpAddr[1],tmpAddr[2],tmpAddr[3],tmpAddr[4],tmpAddr[5],tmpAddr[6],tmpAddr[7]);
	rtd_pr_scaler_memory_info("DE-XCXL, align_addr_96[0:7]=%x,%x,%x,%x,%x,%x,%x,%x,\n",
		align_addr_aligned[0], align_addr_aligned[1],align_addr_aligned[2],align_addr_aligned[3],align_addr_aligned[4],align_addr_aligned[5],align_addr_aligned[6],align_addr_aligned[7]);
	rtd_pr_scaler_memory_info("DE-XCXL,field_size_96=%x,info_size_96=%x,field_size=%d,info_size=%x,mem end=%x,aligned_val=%d,\n",
		field_size_aligned, info_size_aligned, field_size, info_size, mem_end, aligned_val);

	return mem_end;
}

unsigned char I_De_XC_Mem_Ready_Flag[2] = {0};
unsigned char drv_memory_Set_I_De_XC_Mem_Ready_Flag(unsigned char MEM_IDX, unsigned char Flag)
{
	if (MEM_IDX>=2)
		return 255;
	I_De_XC_Mem_Ready_Flag[MEM_IDX] = Flag;
	return 0;
}

unsigned char drv_memory_Get_I_De_XC_Mem_Ready_Flag(unsigned char MEM_IDX)
{
	if (MEM_IDX>=2)
		return 0;
	return I_De_XC_Mem_Ready_Flag[MEM_IDX];
}

unsigned char drv_memory_DeXCXL_Memory_Alloc_Condition_Check(void)
{
	unsigned char En_Flag = 0;
	unsigned char isAdaptivestream_flag = 0, I2R_Enable = 0;
	unsigned char DE_CONTOUR_I2R = 0;
	unsigned int height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
	unsigned int width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
	unsigned int DRAM_size;
	unsigned char input_src_type;
	unsigned int EnByAP_table;
	unsigned char i2r_index;
	//unsigned char MEM_IDX;
	//unsigned char vip_src;

	/* I2R check */
#ifdef CONFIG_I2RND_DUAL_VO
	I2R_Enable = Scaler_I2rnd_get_timing_enable();
	i2r_index = (Scaler_I2rnd_get_display()<=1)?(0):(1);	// Scaler_I2rnd_get_display() ==> 0: i2r disable, 1:i2r s0, 2:i2r s1
#else
	I2R_Enable = 0;
	i2r_index = 0;
#endif

#ifdef ENABLE_DE_CONTOUR_I2R
	DE_CONTOUR_I2R = (rtd_inl(0xb8022288) & _BIT20);
#endif

	input_src_type = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);

	if (input_src_type == VSC_INPUTSRC_VDEC)
		isAdaptivestream_flag = vsc_get_adaptivestream_flag(SLR_MAIN_DISPLAY);

	/* Dram size check*/
	DRAM_size = get_memory_size(GFP_DCU1) + get_memory_size(GFP_DCU2);

	EnByAP_table = get_current_driver_pattern(DRIVER_DEXC_PATTERN);

      if((DE_CONTOUR_I2R == 1) || ((input_src_type != VSC_INPUTSRC_VDEC) && (input_src_type != VSC_INPUTSRC_HDMI)) || (height > 1200) || (i2r_index == 1) ||
	  (DRAM_size <= 0x40000000) || (I2R_Enable == 1) || (EnByAP_table == 0) || (isAdaptivestream_flag == 1))
		En_Flag = DeXCXL_Disable;
	else		/* condition check is ok,  DE-XC memory alloc*/
		En_Flag = DeXCXL_Y10C6_CMP;

	rtd_pr_scaler_memory_info("DE-XC,condition check,En=%d,h=%d,w=%d,Adaptivestream=%d,I2R_En=%d,src_type=%d,DRAM_size=%d,i2rIdx=%d,EnByAP=%d\n",
		En_Flag, height, width ,isAdaptivestream_flag,I2R_Enable, input_src_type, DRAM_size,i2r_index, EnByAP_table);

	return En_Flag;
}
#endif
unsigned int drv_memory_MA_SNR_Config(unsigned char access ,unsigned int phyAddr, unsigned char DI_MEM_IDX)
{
	/* DI part. */
	di_mvf_watmemorystart_s0_add0_RBUS di_mvf_watmemorystart_s0_add0_reg;
	di_mvf_watmemorystart_s0_add1_RBUS di_mvf_watmemorystart_s0_add1_reg;
	di_mvf_watmemorystart_s0_add2_RBUS di_mvf_watmemorystart_s0_add2_reg;
    
    //di_mvf_watmemorystart_s0_add3_RBUS di_mvf_watmemorystart_s0_add3_reg;
	//di_mvf_watmemorystart_s1add0_RBUS di_mvf_watmemorystart_s1_add0_reg;
	//di_mvf_watmemorystart_s1_add1_RBUS di_mvf_watmemorystart_s1_add1_reg;
	//di_mvf_watmemorystart_s1_add2_RBUS di_mvf_watmemorystart_s1_add2_reg;
	//di_mvf_watmemorystart_s1_add3_RBUS di_mvf_watmemorystart_s1_add3_reg;
	di_mvf_ddr_wdma_wtlennum_RBUS di_mvf_ddr_wdma_wtlennum_reg;
	di_mvf_ddr_wdatdma_rm_RBUS di_mvf_ddr_wdatdma_rm_reg;
	di_db_reg_ctl_RBUS di_db_reg_ctl_reg;

	/* SNR part*/
	iedge_smooth_mvf_dma_dat_format_RBUS iedge_smooth_mvf_dma_dat_format_reg;
	iedge_smooth_mvf_datmemorystart_s0_add0_RBUS iedge_smooth_mvf_datmemorystart_s0_add0_reg;
	iedge_smooth_mvf_datmemorystart_s0_add1_RBUS iedge_smooth_mvf_datmemorystart_s0_add1_reg;
	iedge_smooth_mvf_datmemorystart_s0_add2_RBUS iedge_smooth_mvf_datmemorystart_s0_add2_reg;
    //iedge_smooth_mvf_datmemorystart_s0_add3_RBUS iedge_smooth_mvf_datmemorystart_s0_add3_reg;
	//iedge_smooth_mvf_datmemorystart_s1add0_RBUS iedge_smooth_mvf_datmemorystart_s1_add0_reg;
	//iedge_smooth_mvf_datmemorystart_s1_add1_RBUS iedge_smooth_mvf_datmemorystart_s1_add1_reg;
	//iedge_smooth_mvf_datmemorystart_s1_add2_RBUS iedge_smooth_mvf_datmemorystart_s1_add2_reg;
	//iedge_smooth_mvf_datmemorystart_s1_add3_RBUS iedge_smooth_mvf_datmemorystart_s1_add3_reg;
	iedge_smooth_mvf_ddr_dma_wtlennum_RBUS iedge_smooth_mvf_ddr_dma_wtlennum_reg;
	iedge_smooth_mvf_ddr_datdma_rm_RBUS iedge_smooth_mvf_ddr_datdma_rm_reg;
	iedge_smooth_edsm_db_ctrl_RBUS iedge_smooth_edsm_db_ctrl_reg;

	unsigned int height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
	unsigned int width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
	unsigned int pixel_total = height * width;
	unsigned short water, len, remain, num;
	unsigned int mem[4], mem_aligned[4], mem_end;
	unsigned int buffer, buffer_aligned;
	unsigned short aligned_val;
	unsigned short i;

	if (drv_memory_MA_SNR_Enable_Check(access, phyAddr, DI_MEM_IDX, width, height) == 0) {
		// for Decontour, need to set width even in disable case.
		iedge_smooth_mvf_dma_dat_format_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MVF_DMA_Dat_format_reg);
		iedge_smooth_mvf_dma_dat_format_reg.mvf_height = height;
		iedge_smooth_mvf_dma_dat_format_reg.mvf_width = width;
		IoReg_Write32(IEDGE_SMOOTH_MVF_DMA_Dat_format_reg, iedge_smooth_mvf_dma_dat_format_reg.regValue);

		drv_memory_Set_MASNR_Mem_Ready_Flag(DI_MEM_IDX, 0);
		return phyAddr;
	}

	// enable DB
	IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_en_mask);
	IoReg_SetBits(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, IEDGE_SMOOTH_EDSM_DB_CTRL_edsm_db_en_mask);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask);

#ifdef CONFIG_BW_96B_ALIGNED
	aligned_val = 6;
#else
	aligned_val = 0;
#endif

	/* DI part. */
	di_mvf_watmemorystart_s0_add0_reg.regValue = IoReg_Read32(DI_MVF_WATMemoryStart_S0_Add0_reg);
	di_mvf_watmemorystart_s0_add1_reg.regValue = IoReg_Read32(DI_MVF_WATMemoryStart_S0_Add1_reg);
	di_mvf_watmemorystart_s0_add2_reg.regValue = IoReg_Read32(DI_MVF_WATMemoryStart_S0_Add2_reg);

    //di_mvf_watmemorystart_s0_add3_reg.regValue = IoReg_Read32(DI_MVF_WATMemoryStart_S0_Add3_reg);
	//di_mvf_watmemorystart_s1_add0_reg.regValue = IoReg_Read32(DI_MVF_WATMemoryStart_S1Add0_reg);
	//di_mvf_watmemorystart_s1_add1_reg.regValue = IoReg_Read32(DI_MVF_WATMemoryStart_S1_Add1_reg);
	//di_mvf_watmemorystart_s1_add2_reg.regValue = IoReg_Read32(DI_MVF_WATMemoryStart_S1_Add2_reg);
	//di_mvf_watmemorystart_s1_add3_reg.regValue = IoReg_Read32(DI_MVF_WATMemoryStart_S1_Add3_reg);
	di_mvf_ddr_wdma_wtlennum_reg.regValue = IoReg_Read32(DI_MVF_DDR_WDMA_WTLENNUM_reg);
	di_mvf_ddr_wdatdma_rm_reg.regValue = IoReg_Read32(DI_MVF_DDR_WDATDMA_RM_reg);

	/* SNR part*/
	iedge_smooth_mvf_dma_dat_format_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MVF_DMA_Dat_format_reg);
	iedge_smooth_mvf_datmemorystart_s0_add0_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MVF_DATMemoryStart_S0_Add0_reg);
	iedge_smooth_mvf_datmemorystart_s0_add1_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MVF_DATMemoryStart_S0_Add1_reg);
	iedge_smooth_mvf_datmemorystart_s0_add2_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MVF_DATMemoryStart_S0_Add2_reg);
    //iedge_smooth_mvf_datmemorystart_s0_add3_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MVF_DATMemoryStart_S0_Add3_reg);
	//iedge_smooth_mvf_datmemorystart_s1_add0_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MVF_DATMemoryStart_S1Add0_reg);
	//iedge_smooth_mvf_datmemorystart_s1_add1_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MVF_DATMemoryStart_S1_Add1_reg);
	//iedge_smooth_mvf_datmemorystart_s1_add2_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MVF_DATMemoryStart_S1_Add2_reg);
	//iedge_smooth_mvf_datmemorystart_s1_add3_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MVF_DATMemoryStart_S1_Add3_reg);
	iedge_smooth_mvf_ddr_dma_wtlennum_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MVF_DDR_DMA_WTLENNUM_reg);
	iedge_smooth_mvf_ddr_datdma_rm_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MVF_DDR_DATDMA_RM_reg);

	buffer = drv_memory_Get_MA_SNR_Info_Size_Static(access, width, height, DI_MEM_IDX);

	if (aligned_val != 0) {
		buffer_aligned = dvr_size_alignment(buffer);
	} else {
		buffer_aligned = buffer;
	}

	//Merlin4 5A use 4ddr, 3A&P use 3ddr
	if(access == SLR_DI_5A_mode)
	{
		mem[0] = (phyAddr);
		mem[1] = (phyAddr + buffer_aligned);
		mem[2] = (phyAddr + buffer_aligned * 2);
		mem[3] = (phyAddr + buffer_aligned * 3);
		mem_end = phyAddr + buffer_aligned * 4;

		if (aligned_val != 0) {
			mem_aligned[0] = dvr_memory_alignment((unsigned long)mem[0], buffer_aligned);
			mem_aligned[1] = dvr_memory_alignment((unsigned long)mem[1], buffer_aligned);
			mem_aligned[2] = dvr_memory_alignment((unsigned long)mem[2], buffer_aligned);
			mem_aligned[3] = dvr_memory_alignment((unsigned long)mem[3], buffer_aligned);
			for (i=0;i<4;i++) {
				if (mem_aligned[i] == 0) {
					rtd_pr_scaler_memory_emerg("MA-SNR, memory 96 byte alignment fail, addr=%x, index=%d, buffer_aligned=%d\n",
						mem[i], i,  buffer_aligned);
					BUG();
				}
			}

		} else {
			mem_aligned[0] = ((mem[0]+15)>>4)<<4;//16 byte alignment
			mem_aligned[1] = ((mem[1]+15)>>4)<<4;//16 byte alignment
			mem_aligned[2] = ((mem[2]+15)>>4)<<4;//16 byte alignment
			mem_aligned[3] = ((mem[3]+15)>>4)<<4;//16 byte alignment
		}
	}
	else
	{
		mem[0] = (phyAddr);
		mem[1] = (phyAddr + buffer_aligned);
		mem[2] = (phyAddr + buffer_aligned * 2);
		mem[3] = mem[2];
		mem_end = phyAddr + buffer_aligned * 3;

		if (aligned_val != 0) {
			mem_aligned[0] = dvr_memory_alignment((unsigned long)mem[0], buffer_aligned);
			mem_aligned[1] = dvr_memory_alignment((unsigned long)mem[1], buffer_aligned);
			mem_aligned[2] = dvr_memory_alignment((unsigned long)mem[2], buffer_aligned);
			mem_aligned[3] = mem_aligned[2];
			for (i=0;i<4;i++) {
				if (mem_aligned[i] == 0) {
					rtd_pr_scaler_memory_emerg("MA-SNR, memory 96 byte alignment fail, addr=%x, index=%d, buffer_aligned=%d\n",
						mem[i], i,  buffer_aligned);
					BUG();
				}
			}

 		} else {
			mem_aligned[0] = ((mem[0]+15)>>4)<<4;//16 byte alignment
			mem_aligned[1] = ((mem[1]+15)>>4)<<4;//16 byte alignment
			mem_aligned[2] = ((mem[2]+15)>>4)<<4;//16 byte alignment
			mem_aligned[3] = ((mem[3]+15)>>4)<<4;//16 byte alignment
		}
	}

	len = 16;
	if (aligned_val != 0)
		len = ((len%aligned_val)==0)?(len):(len - (len % aligned_val)); /* info burst len should be less after 96 byte aligned, recommand from xq_wu */

	water = 16;

	num = (pixel_total>>6) / len;	/* number*lenth+remain = pixel number*2/128, 128b per num */
	remain = (pixel_total>>6) - len * num;
	if (remain == 0) { /* remain can't be 0*/
		num -= 1;
		remain = len;
	}

	/* DI part. */
	di_mvf_ddr_wdma_wtlennum_reg.mvf_inf_len_w = len;
	di_mvf_ddr_wdma_wtlennum_reg.mvf_inf_water_w = water;
	di_mvf_ddr_wdma_wtlennum_reg.mvf_inf_num_w = num;
	di_mvf_ddr_wdatdma_rm_reg.mvf_inf_remain_w = remain;

	if (DI_MEM_IDX == 0) {
		di_mvf_watmemorystart_s0_add0_reg.s0_mstart_86_58 = mem_aligned[0]>>3;	/* bit 0~2 is reserved*/
		di_mvf_watmemorystart_s0_add1_reg.s0_mstart_57_29 = mem_aligned[1]>>3;	/* bit 0~2 is reserved*/
		di_mvf_watmemorystart_s0_add2_reg.s0_mstart_28_0 = mem_aligned[2]>>3;	/* bit 0~2 is reserved*/
		//di_mvf_watmemorystart_s0_add3_reg.s0_mstart_28_0_0 = mem_aligned[3]>>3;		/* bit 0~2 is reserved*/
	} else {
		//di_mvf_watmemorystart_s1_add0_reg.s1_mstart_115_87 = mem_aligned[0]>>3;	/* bit 0~2 is reserved*/
		//di_mvf_watmemorystart_s1_add1_reg.s1_mstart_86_58 = mem_aligned[1]>>3;	/* bit 0~2 is reserved*/
		//di_mvf_watmemorystart_s1_add2_reg.s1_mstart_57_29 = mem_aligned[2]>>3;	/* bit 0~2 is reserved*/
		//di_mvf_watmemorystart_s1_add3_reg.s1_mstart_28_0 = mem_aligned[3]>>3;		/* bit 0~2 is reserved*/
	}

	rtd_pr_scaler_memory_info("MA-SNR,water=%d, len=%d,num=%d,remain=%d, mem_end=%x,buffer=%x, buffer_aligned=%x,aligned_val=%d,\n",
		water, len ,num ,remain, mem_end, buffer, buffer_aligned,aligned_val);

	rtd_pr_scaler_memory_info("MA-SNR,mem=%x,%x,%x,%x, mem_aligned=%x%x,%x,%x,\n",
		mem[0], mem[1], mem[2], mem[3], mem_aligned[0], mem_aligned[1], mem_aligned[2], mem_aligned[3]);

	rtd_pr_scaler_memory_info("MA-SNR,access=%d, heigth=%d,width=%d, DI_mem_idx=%d\n", access, height, width, DI_MEM_IDX);

	/* SNR part*/
	/* same as DI part ? */
	iedge_smooth_mvf_ddr_dma_wtlennum_reg.dat_len = len;
	iedge_smooth_mvf_ddr_dma_wtlennum_reg.dat_water = water;
	iedge_smooth_mvf_ddr_dma_wtlennum_reg.dat_num = num;
	iedge_smooth_mvf_ddr_datdma_rm_reg.dat_remain = remain;
	iedge_smooth_mvf_ddr_datdma_rm_reg.dma_remain_en = 1;

	iedge_smooth_mvf_dma_dat_format_reg.mvf_progressive = (access == SLR_DI_progress_mode)?1:0;
	iedge_smooth_mvf_dma_dat_format_reg.delay_frame_num = (access == SLR_DI_5A_mode)?(2):((access == SLR_DI_3A_mode)?(1):(0));
	iedge_smooth_mvf_dma_dat_format_reg.mvf_height = height;
	iedge_smooth_mvf_dma_dat_format_reg.mvf_width = width;

	if (DI_MEM_IDX == 0) {
		iedge_smooth_mvf_datmemorystart_s0_add0_reg.s0_mstart_86_58= mem_aligned[0]>>3;	/* bit 0~2 is reserved*/
		iedge_smooth_mvf_datmemorystart_s0_add1_reg.s0_mstart_57_29 = mem_aligned[1]>>3;	/* bit 0~2 is reserved*/
		iedge_smooth_mvf_datmemorystart_s0_add2_reg.s0_mstart_28_0 = mem_aligned[2]>>3;	/* bit 0~2 is reserved*/
		//iedge_smooth_mvf_datmemorystart_s0_add3_reg.s0_mstart_28_0_0 = mem_aligned[3]>>3;	/* bit 0~2 is reserved*/
	} else {
		//iedge_smooth_mvf_datmemorystart_s1_add0_reg.s1_mstart_115_87= mem_aligned[0]>>3;	/* bit 0~2 is reserved*/
		//iedge_smooth_mvf_datmemorystart_s1_add1_reg.s1_mstart_86_58 = mem_aligned[1]>>3;	/* bit 0~2 is reserved*/
		//iedge_smooth_mvf_datmemorystart_s1_add2_reg.s1_mstart_57_29 = mem_aligned[2]>>3;	/* bit 0~2 is reserved*/
		//iedge_smooth_mvf_datmemorystart_s1_add3_reg.s1_mstart_28_0 = mem_aligned[3]>>3;	/* bit 0~2 is reserved*/
	}

	/* DI part. */
	IoReg_Write32(DI_MVF_DDR_WDMA_WTLENNUM_reg, di_mvf_ddr_wdma_wtlennum_reg.regValue);
	IoReg_Write32(DI_MVF_DDR_WDATDMA_RM_reg, di_mvf_ddr_wdatdma_rm_reg.regValue );

	if (DI_MEM_IDX == 0) {
		IoReg_Write32(DI_MVF_WATMemoryStart_S0_Add0_reg, di_mvf_watmemorystart_s0_add0_reg.regValue);
		IoReg_Write32(DI_MVF_WATMemoryStart_S0_Add1_reg, di_mvf_watmemorystart_s0_add1_reg.regValue);
		IoReg_Write32(DI_MVF_WATMemoryStart_S0_Add2_reg, di_mvf_watmemorystart_s0_add2_reg.regValue);
		//IoReg_Write32(DI_MVF_WATMemoryStart_S0_Add3_reg, di_mvf_watmemorystart_s0_add3_reg.regValue);
	} else {
		//IoReg_Write32(DI_MVF_WATMemoryStart_S1Add0_reg, di_mvf_watmemorystart_s1_add0_reg.regValue);
		//IoReg_Write32(DI_MVF_WATMemoryStart_S1_Add1_reg, di_mvf_watmemorystart_s1_add1_reg.regValue);
		//IoReg_Write32(DI_MVF_WATMemoryStart_S1_Add2_reg, di_mvf_watmemorystart_s1_add2_reg.regValue);
		//IoReg_Write32(DI_MVF_WATMemoryStart_S1_Add3_reg, di_mvf_watmemorystart_s1_add3_reg.regValue);
	}
	/* SNR part*/
	IoReg_Write32(IEDGE_SMOOTH_MVF_DMA_Dat_format_reg, iedge_smooth_mvf_dma_dat_format_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_MVF_DDR_DMA_WTLENNUM_reg, iedge_smooth_mvf_ddr_dma_wtlennum_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_MVF_DDR_DATDMA_RM_reg, iedge_smooth_mvf_ddr_datdma_rm_reg.regValue);

	if (DI_MEM_IDX == 0) {
		IoReg_Write32(IEDGE_SMOOTH_MVF_DATMemoryStart_S0_Add0_reg, iedge_smooth_mvf_datmemorystart_s0_add0_reg.regValue);
		IoReg_Write32(IEDGE_SMOOTH_MVF_DATMemoryStart_S0_Add1_reg, iedge_smooth_mvf_datmemorystart_s0_add1_reg.regValue );
		IoReg_Write32(IEDGE_SMOOTH_MVF_DATMemoryStart_S0_Add2_reg, iedge_smooth_mvf_datmemorystart_s0_add2_reg.regValue);
		//IoReg_Write32(IEDGE_SMOOTH_MVF_DATMemoryStart_S0_Add3_reg, iedge_smooth_mvf_datmemorystart_s0_add3_reg.regValue);
	} else {
		//IoReg_Write32(IEDGE_SMOOTH_MVF_DATMemoryStart_S1Add0_reg, iedge_smooth_mvf_datmemorystart_s1_add0_reg.regValue );
		//IoReg_Write32(IEDGE_SMOOTH_MVF_DATMemoryStart_S1_Add1_reg, iedge_smooth_mvf_datmemorystart_s1_add1_reg.regValue);
		//IoReg_Write32(IEDGE_SMOOTH_MVF_DATMemoryStart_S1_Add2_reg, iedge_smooth_mvf_datmemorystart_s1_add2_reg.regValue);
		//IoReg_Write32(IEDGE_SMOOTH_MVF_DATMemoryStart_S1_Add3_reg, iedge_smooth_mvf_datmemorystart_s1_add3_reg.regValue);
	}

	/* double buffer*/
	di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
	di_db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, di_db_reg_ctl_reg.regValue);

	iedge_smooth_edsm_db_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg);
	iedge_smooth_edsm_db_ctrl_reg.edsm_db_apply = 1;
	IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, iedge_smooth_edsm_db_ctrl_reg.regValue);

	drv_memory_Set_MASNR_Mem_Ready_Flag(DI_MEM_IDX, 1);
	return mem_end;
}

char drv_memory_MA_SNR_Enable_Check(unsigned char access ,unsigned int phyAddr, unsigned char DI_MEM_IDX, unsigned int DI_Width, unsigned int DI_Height)
{
	unsigned char ret;
	unsigned char decide_by_AP, vsc_src, adaptvie_stream_flag, DISP_THRIP, DISP_RTNR;
	unsigned char is2kModel = 0;
	decide_by_AP = get_current_driver_pattern(DRIVER_MASNR_PATTERN);
	vsc_src = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
	adaptvie_stream_flag = vsc_get_adaptivestream_flag(SLR_MAIN_DISPLAY);
	DISP_THRIP = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP);
	DISP_RTNR = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_RTNR);

	/* diable MA-SNR */
	/*1.DI is disable, */
	/*2. adaptive stream since "nr_motion_en", "IESM_Motion_En", "AVLPF_Motion_En" is not double buffer */
	/*3. .xml table */
	if(get_platform_model() == PLATFORM_MODEL_2K) {
		is2kModel = 1;
	}
	if ((decide_by_AP == 0) || ((vsc_src == VSC_INPUTSRC_VDEC) && (adaptvie_stream_flag >= ADAPTIVE_STREAM_ON)) || ((DISP_THRIP  == FALSE) && (DISP_RTNR == FALSE)) || (is2kModel == 1)) {
		ret = 0;
	} else {
		ret = 1;
	}

	rtd_pr_scaler_memory_info("MA-SNR, decide_by_AP=%d, vsc_src=%d, adaptvie_stream_flag=%d, DISP_THRIP=%d, DISP_RTNR=%d,\n",
		decide_by_AP, vsc_src, adaptvie_stream_flag, DISP_THRIP, DISP_RTNR);

	return ret;
}

char  drv_memory_MA_SNR_Disable(unsigned char MEM_idx)
{
	di_rtnr_ma_isnr_getmotion_main_RBUS getMotion_main_reg;
	nr_motion_weight_ctrl_RBUS nr_motion_weight_ctrl_reg;
	iedge_smooth_eng_clamp_RBUS eng_clamp_reg;
	iedge_smooth_vfilter_4_RBUS vfilter_4_reg;

	IoReg_ClearBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_en_mask);
	IoReg_ClearBits(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, IEDGE_SMOOTH_EDSM_DB_CTRL_edsm_db_en_mask);
	IoReg_ClearBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask);

	getMotion_main_reg.regValue = IoReg_Read32(DI_RTNR_MA_ISNR_GETMOTION_MAIN_reg);
	getMotion_main_reg.rtnr_isnr_getmotion_en = 0;
	IoReg_Write32(DI_RTNR_MA_ISNR_GETMOTION_MAIN_reg, getMotion_main_reg.regValue);

	nr_motion_weight_ctrl_reg.regValue = IoReg_Read32(NR_MOTION_WEIGHT_ctrl_reg);
	nr_motion_weight_ctrl_reg.nr_motion_en = 0;
	IoReg_Write32(NR_MOTION_WEIGHT_ctrl_reg, nr_motion_weight_ctrl_reg.regValue);

	eng_clamp_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_ENG_CLAMP_reg);
	eng_clamp_reg.iedsm_motion_en = 0;
	IoReg_Write32(IEDGE_SMOOTH_ENG_CLAMP_reg, eng_clamp_reg.regValue);

	vfilter_4_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_6_reg);
	vfilter_4_reg.egsm_avlpf_motion_en = 0;
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_6_reg, vfilter_4_reg.regValue);

	rtd_pr_scaler_memory_info("MA-SNR, Disable, idx=%d,\n", MEM_idx);

	drv_memory_Set_MASNR_Mem_Ready_Flag(MEM_idx, 0);
	return 0;
}

unsigned char MASNR_Mem_Ready_Flag[2] = {0};
unsigned char drv_memory_Set_MASNR_Mem_Ready_Flag(unsigned char MEM_IDX, unsigned char Flag)
{
	if (MEM_IDX>=2)
		return 255;
	MASNR_Mem_Ready_Flag[MEM_IDX] = Flag;
	return 0;
}

unsigned char drv_memory_Get_MASNR_Mem_Ready_Flag(unsigned char MEM_IDX)
{
	if (MEM_IDX>=2)
		return 0;
	return MASNR_Mem_Ready_Flag[MEM_IDX];
}

unsigned int  drv_memory_Get_MA_SNR_Info_Size_Static(unsigned char access, unsigned int I_Width, unsigned int I_Height, unsigned char DI_MEM_IDX)
{
	unsigned int buffer_size;
	unsigned int  offset = 1024;	/* avoid trash other memory*/
#if 1	//calculate memory
	unsigned int width, height, data_bit;

	data_bit = 2;
	if (access == SLR_DI_progress_mode) {
		width = I_Width>>1;
		height = I_Height;
	} else {
		width = I_Width;
		height = I_Height;
	}
	buffer_size = (width * height * data_bit)>>3;
	buffer_size = buffer_size + offset;

	rtd_pr_scaler_memory_info("masnr_size_cal, buffer_size=%d,\n", buffer_size);

#else	// static memory
	/* DI 1 = 13Mb, 10Mb for DI, 3Mb for MA-SNR @ m4, use rolling mode*/
	if (access == SLR_DI_5A_mode) { /* 5A, 4 buffer alloc*/
		buffer_size = 0xC0000 - offset ;	/* 0.75MB + 0.75MB + 0.75MB + 0.75MB = 3MB, 4 buffer alloc*/
	} else {	/* 3A and P mode, 3 buffer alloc*/
		buffer_size = 0x100000 - offset ;	/* 1MB + 1MB + 1MB = 3MB, 3 buffer alloc*/
	}

	if (DI_MEM_IDX == 1 /*&& get_platform() == PLATFORM_KXLP*/) {	/* i2r need to confirm*/
		if (access == SLR_DI_5A_mode) { /* 5A, 4 buffer alloc*/
			buffer_size = 0xC0000 - offset ;	/* 0.75MB + 0.75MB + 0.75MB + 0.75MB = 3MB, 4 buffer alloc*/
		} else {	/* 3A and P mode, 3 buffer alloc*/
			buffer_size = 0x100000 - offset ;	/* 1MB + 1MB + 1MB = 3MB, 3 buffer alloc*/
		}
	}
#endif
	return buffer_size;
}

#endif
void drv_memory_wait_cap_last_write_done(unsigned char display, unsigned char wait_frame, unsigned char game_mode_case){
	//game_mode_case means call by game mode flow
	mdomain_cap_cap_ddr_fifostatus_RBUS cap_ddr_fifostatus_reg;
	unsigned int count;
	unsigned char pre_cap_idx;
	unsigned char cur_cap_idx;
	if(display == SLR_MAIN_DISPLAY){
		do{
			count = 40;
			pre_cap_idx = MDOMAIN_CAP_DDR_In1Status_get_in1_block_select(IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg));
    		//rtd_pr_scaler_memory_notice("M domain main done-0, cap pre idx = %d\n", pre_cap_idx);

			/*check memory cap down*/
			IoReg_Write32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg, IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg));
        	while(!(MDOMAIN_CAP_Cap_DDR_FIFOStatus_get_in1_cap_last_wr_flag(IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg))) && --count){
				if(game_mode_case && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE))
				{
					rtd_pr_scaler_memory_notice("drv_memory_wait_cap_last_write_done game mode case not active return\n");
					return;
				}
                 #ifdef BUILD_QUICK_SHOW
                    	msleep(10);//no busy delay 10 ms
                 #else
                    msleep(0);//no busy delay 10 ms
                 #endif
        	}
			if(count != 0) {
				while((MDOMAIN_DISP_DDR_MainSubCtrl_get_disp1_double_apply(IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg))) && --count) {
	            	         #ifdef BUILD_QUICK_SHOW
                                msleep(10);//no busy delay 10 ms
                             #else
                                msleep(0);//no busy delay 10 ms
                             #endif
				}
			}
			cur_cap_idx = MDOMAIN_CAP_DDR_In1Status_get_in1_block_select(IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg));
    		rtd_pr_scaler_memory_notice("M domain main done-1, cap pre index = %d new idx = %d count:%d\n", pre_cap_idx, cur_cap_idx, count);

			if(count == 0)
        		rtd_pr_scaler_memory_err("M domain main timeout-1 !\n");

    		// [KTASKWBS-5316] wait M-capture write done if capure block index == 0
		//if(MDOMAIN_CAP_DDR_In1Status_get_in1_block_select(IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg)) == pre_cap_idx)
		if (cur_cap_idx ==  pre_cap_idx)
    		{
        		count = 40;
        		/*check memory cap down*/
        		cap_ddr_fifostatus_reg.in1_cap_last_wr_flag = 1;
        		IoReg_Write32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg, cap_ddr_fifostatus_reg.regValue);

        		while(!(MDOMAIN_CAP_Cap_DDR_FIFOStatus_get_in1_cap_last_wr_flag(IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg))) && (MDOMAIN_CAP_DDR_In1Status_get_in1_block_select(IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg)) == pre_cap_idx) && --count){
					if(game_mode_case && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE))
					{
						rtd_pr_scaler_memory_notice("drv_memory_wait_cap_last_write_done game mode case not active return\n");
						return;
					}
					 #ifdef BUILD_QUICK_SHOW
                                    msleep(10);//no busy delay 10 ms
                                 #else
                                    msleep(0);//no busy delay 10 ms
                                 #endif
        		}
				cur_cap_idx = MDOMAIN_CAP_DDR_In1Status_get_in1_block_select(IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg));
        		rtd_pr_scaler_memory_notice("M domain main done-2(check again), cap new idx = %d count:%d\n", cur_cap_idx, count);
        		if(count == 0)
            		rtd_pr_scaler_memory_err("M domain main timeout-2 !\n");
    		}
			wait_frame--;

			//if wait_frame > 0, clear again
			if(wait_frame > 0){
				cap_ddr_fifostatus_reg.in1_cap_last_wr_flag = 1;
                IoReg_Write32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg, cap_ddr_fifostatus_reg.regValue);
			}
		}while(wait_frame != 0);
	}
	else{
		do{
			count = 40;
			pre_cap_idx = MDOMAIN_CAP_DDR_In2Status_get_in2_block_select(IoReg_Read32(MDOMAIN_CAP_DDR_In2Status_reg));

					/*check memory cap down*/
                	while(!(MDOMAIN_CAP_Cap_DDR_FIFOStatus_get_in2_cap_last_wr_flag(IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg))) && --count){
                    	msleep(0);//no busy delay 10 ms
                	}

            		if(count == 0)
                		rtd_pr_scaler_memory_err("M domain sub timeout1 !\n");

            		// [KTASKWBS-5316] wait M-capture write done if capure block index == 0
            		if(MDOMAIN_CAP_DDR_In2Status_get_in2_block_select(IoReg_Read32(MDOMAIN_CAP_DDR_In2Status_reg)) == pre_cap_idx)
            		{
                		count = 40;
                		/*check memory cap down*/
                		cap_ddr_fifostatus_reg.in2_cap_last_wr_flag = 1;
                		IoReg_Write32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg, cap_ddr_fifostatus_reg.regValue);

                		while(!(MDOMAIN_CAP_Cap_DDR_FIFOStatus_get_in2_cap_last_wr_flag(IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg))) && (MDOMAIN_CAP_DDR_In2Status_get_in2_block_select(IoReg_Read32(MDOMAIN_CAP_DDR_In2Status_reg)) == pre_cap_idx) && --count){
                    		msleep(0);//no busy delay 10 ms
                		}

                		if(count == 0)
                    		rtd_pr_scaler_memory_err("M domain sub timeout2 !\n");
            		}
			wait_frame--;
		}while(wait_frame != 0);
	}
}


void drv_memory_wait_mdom_vi_last_write_done(unsigned char display, unsigned char wait_frame){
	unsigned int count;
	mdomain_vi_main_intst_RBUS vi_intst_reg; //MDOM_VI_INTST_reg

	if(display == SLR_MAIN_DISPLAY){
		do{
			count = 40;

			/*check memory cap down*/
            vi_intst_reg.regValue = rtd_inl(MDOMAIN_VI_MAIN_INTST_reg);
        	while(!(vi_intst_reg.video_in_last_wr_y_flag /*&& vi_intst_reg.video_in_last_wr_c_flag*/) && --count){
           		vi_intst_reg.regValue = rtd_inl(MDOMAIN_VI_MAIN_INTST_reg);
				
            	msleep(1);//no busy delay
        	}

			if(count == 0)
        		rtd_pr_scaler_memory_err("M domain vi main timeout-1 !\n");

    		count = 40;
    		/*check memory cap down*/
    		vi_intst_reg.video_in_last_wr_y_flag = 1;
        	//vi_intst_reg.video_in_last_wr_c_flag = 1;
            rtd_outl(MDOMAIN_VI_MAIN_INTST_reg, vi_intst_reg.regValue);

    		vi_intst_reg.regValue = rtd_inl(MDOMAIN_VI_MAIN_INTST_reg);
    		while(!(vi_intst_reg.video_in_last_wr_y_flag /*&& vi_intst_reg.video_in_last_wr_c_flag*/) && --count){
       		vi_intst_reg.regValue = rtd_inl(MDOMAIN_VI_MAIN_INTST_reg);
        	msleep(1);//no busy delay


        	if(count == 1)
            	rtd_pr_scaler_memory_err("M domain vi main timeout-2 !\n");
    		}
			wait_frame--;

			//if wait_frame > 0, clear again
			if(wait_frame > 0){
				vi_intst_reg.video_in_last_wr_y_flag = 1;
        		//vi_intst_reg.video_in_last_wr_c_flag = 1;
                rtd_outl(MDOMAIN_VI_MAIN_INTST_reg, vi_intst_reg.regValue);
			}
		}while(wait_frame != 0);
	}
	else{
	}
}


//==============Gamd mode==============//


extern unsigned char Get_Factory_SelfDiagnosis_Mode(void);
extern unsigned char get_AVD_Input_Source(void);
void drv_memory_set_game_mode_dynamic(unsigned char enable){
	enable_M_GameMode_Dynamic = enable;
}

unsigned char drv_memory_get_game_mode_dynamic_flag(void){
	return enable_M_GameMode_Dynamic;
}

unsigned char drv_memory_get_game_mode_dynamic(void){
	return enable_M_GameMode_Dynamic;
}

void drv_memory_set_game_mode(unsigned char enable)
{
	enable_M_GameMode = enable;
}

//only get game mode flag, no need to check input source & conditions.
unsigned char drv_memory_get_game_mode_flag(void)
{
	return enable_M_GameMode;
}

unsigned char drv_memory_get_game_mode(void){
       //Update the game mode condition to game mode status driver @Crixus 20180825
#ifndef BUILD_QUICK_SHOW
	if(get_new_game_mode_condition())
		return enable_M_GameMode;
	else
#endif
	       return FALSE;
}

unsigned char drv_I3ddmaMemory_get_game_mode(void){
       /*Only support HDMI and Component and AV*/
#if 0
 /*20170629 pinyen move conditions outside driver*/
       if(!((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_ADC)
               || (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
               ||((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_AVD)  && (get_AVD_Input_Source() != _SRC_TV))))
               return FALSE;
#endif
       return enable_M_GameMode;
}


void drv_memory_SingleBuffer_GameMode(unsigned char enable){
	mdomain_cap_ddr_in1ctrl_RBUS mdomain_cap_ddr_in1ctrl_Reg;
	mdomain_disp_ddr_mainctrl_RBUS mdomain_disp_ddr_mainctrl_Reg;

	mdomain_cap_ddr_in1ctrl_Reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Ctrl_reg);
	mdomain_disp_ddr_mainctrl_Reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
	if(enable == TRUE){
		//change to signal buffer
		//IoReg_ClearBits(MDOMAIN_CAP_DDR_In1Ctrl_reg,  MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask|MDOMAIN_CAP_DDR_In1Ctrl_in1_v_flip_3buf_en_mask);
		//IoReg_ClearBits(MDOMAIN_DISP_DDR_MainCtrl_reg,MDOMAIN_DISP_DDR_MainCtrl_main_double_en_mask|MDOMAIN_DISP_DDR_MainCtrl_main_v_flip_3buf_en_mask);
		mdomain_cap_ddr_in1ctrl_Reg.in1_v_flip_3buf_en = 0;
		mdomain_cap_ddr_in1ctrl_Reg.in1_double_enable = 0;
		mdomain_cap_ddr_in1ctrl_Reg.in1_single_buffer_select = 2;

		mdomain_disp_ddr_mainctrl_Reg.main_v_flip_3buf_en = 0;
		mdomain_disp_ddr_mainctrl_Reg.main_double_en = 0;
		mdomain_disp_ddr_mainctrl_Reg.main_single_buffer_select = 2;

	}
	else{
		//triple buffer
		//IoReg_SetBits(MDOMAIN_CAP_DDR_In1Ctrl_reg,  MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask|MDOMAIN_CAP_DDR_In1Ctrl_in1_v_flip_3buf_en_mask);
		//IoReg_SetBits(MDOMAIN_DISP_DDR_MainCtrl_reg, MDOMAIN_DISP_DDR_MainCtrl_main_double_en_mask|MDOMAIN_DISP_DDR_MainCtrl_main_v_flip_3buf_en_mask);
		mdomain_cap_ddr_in1ctrl_Reg.in1_v_flip_3buf_en = 1;
		mdomain_cap_ddr_in1ctrl_Reg.in1_double_enable = 1;
		mdomain_cap_ddr_in1ctrl_Reg.in1_single_buffer_select = 0;

		mdomain_disp_ddr_mainctrl_Reg.main_v_flip_3buf_en = 1;
		mdomain_disp_ddr_mainctrl_Reg.main_double_en = 1;
		mdomain_disp_ddr_mainctrl_Reg.main_single_buffer_select = 0;
	}
	IoReg_Write32(MDOMAIN_CAP_DDR_In1Ctrl_reg, mdomain_cap_ddr_in1ctrl_Reg.regValue);
	IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg, mdomain_disp_ddr_mainctrl_Reg.regValue);
	rtd_pr_scaler_memory_info("[Game Mode]M-domain game mode done enable=%d!!\n",enable);
}

void drv_memory_GameMode_Switch_SingleBuffer(void){
	unsigned int timeout = 0x3ffff;
	mdomain_cap_cap_reg_doublbuffer_RBUS cap_reg_doublbuffer_reg;
	mdomain_disp_ddr_mainsubctrl_RBUS ddr_mainsubctrl_reg;
//	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	mdomain_cap_ddr_in1ctrl_RBUS mdomain_cap_ddr_in1ctrl_reg;
	mdomain_disp_ddr_mainctrl_RBUS mdomain_disp_ddr_mainctrl_reg;

	UINT8 cap_db_en = 0;
	//UINT8 disp_db_en = 0;
	//UINT8 display_dtg_db_en = 0;

	//enable M-cap double buffer
	cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	cap_db_en = cap_reg_doublbuffer_reg.cap1_db_en;
	cap_reg_doublbuffer_reg.cap1_db_en = 1;
	IoReg_Write32(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);

	//enable M-disp double buffer
	ddr_mainsubctrl_reg.regValue = rtd_inl(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
	//disp_db_en = ddr_mainsubctrl_reg.disp1_double_enable;
	ddr_mainsubctrl_reg.disp1_double_enable = 1;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);

/////////////////////////////////////////////////////////////////////////////////////////
	timeout = 0x3ffff;

	//first change M-disp.
	mdomain_disp_ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
	mdomain_disp_ddr_mainctrl_reg.main_double_en = 0;
	mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en = 0;
	mdomain_disp_ddr_mainctrl_reg.main_single_buffer_select = 2;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg,mdomain_disp_ddr_mainctrl_reg.regValue);


	ddr_mainsubctrl_reg.disp1_double_apply = 1;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);
	while((ddr_mainsubctrl_reg.disp1_double_apply == 1) && (timeout--)){
		ddr_mainsubctrl_reg.regValue =rtd_inl(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
	}

	timeout = 0x3ffff;

	//change M-cap
	mdomain_cap_ddr_in1ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Ctrl_reg);
	mdomain_cap_ddr_in1ctrl_reg.in1_v_flip_3buf_en = 0;
	mdomain_cap_ddr_in1ctrl_reg.in1_double_enable = 0;
	mdomain_cap_ddr_in1ctrl_reg.in1_single_buffer_select = 2;
	IoReg_Write32(MDOMAIN_CAP_DDR_In1Ctrl_reg,mdomain_cap_ddr_in1ctrl_reg.regValue);

	cap_reg_doublbuffer_reg.cap1_db_apply = 1;
	IoReg_Write32(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);
	while((cap_reg_doublbuffer_reg.cap1_db_apply == 1) && (timeout--)){
		cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	}
	rtd_pr_new_game_mode_notice("[Game Mode]M-domain game mode enable!!\n");

	/////////////////////////////////////////////////////////////////////////////////////////

	//resume M-cap double buffer setting
	cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_SEARCH) {
		cap_reg_doublbuffer_reg.cap1_db_en = 0;
	} else {
		cap_reg_doublbuffer_reg.cap1_db_en = cap_db_en;
	}
	IoReg_Write32(MDOMAIN_CAP_cap_reg_doublbuffer_reg, cap_reg_doublbuffer_reg.regValue);
#if 0
	//resume M-disp double buffer setting
	ddr_mainsubctrl_reg.regValue = rtd_inl(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
	ddr_mainsubctrl_reg.disp1_double_enable = disp_db_en;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg, ddr_mainsubctrl_reg.regValue);
#endif
}

unsigned char drv_memory_get_mbuffer_setting(void){
	mdomain_cap_ddr_in1ctrl_RBUS mdomain_cap_ddr_in1ctrl_reg;
	mdomain_disp_ddr_mainctrl_RBUS mdomain_disp_ddr_mainctrl_reg;
	mdomain_cap_ddr_in1ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Ctrl_reg);
	mdomain_disp_ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);

	if((mdomain_cap_ddr_in1ctrl_reg.in1_v_flip_3buf_en == 1) && (mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en == 1))
		return 1;	//tripple buffer
	else
		return 0;	//single buffer
}

void drv_memory_debug_print_buf_idx(void)
{
#ifndef UT_flag
	static unsigned char capIdx_pre = 0xf;
	static unsigned char dispIdx_pre = 0xf;
	mdomain_cap_ddr_in1status_RBUS mdomain_cap_ddr_in1status_reg;
	mdomain_disp_ddr_mainstatus_RBUS mdomain_disp_ddr_mainstatus_reg;
	vgip_vgip_chn1_lc_RBUS vgip_chn1_lc_reg;
	ppoverlay_new_meas1_linecnt_real_RBUS new_meas1_linecnt_real_reg;

	mdomain_cap_ddr_in1status_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg);
	mdomain_disp_ddr_mainstatus_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainStatus_reg);
	vgip_chn1_lc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_LC_reg);
	new_meas1_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);

	rtd_pr_scaler_memory_notice("[M-BUF-IDX] cap idx = %d, disp idx = %d (%d)\n", mdomain_cap_ddr_in1status_reg.in1_block_select, mdomain_disp_ddr_mainstatus_reg.main_block_select,
		IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg));
	rtd_pr_scaler_memory_notice("vgip line_cnt %d, uzu line_cnt (%d)\n", vgip_chn1_lc_reg.ch1_line_cnt, new_meas1_linecnt_real_reg.uzudtg_line_cnt_rt);

	if((capIdx_pre != 0xff)||(dispIdx_pre != 0xff)){
		// m domain cap-check 1
		if((mdomain_cap_ddr_in1status_reg.in1_block_select == 0)&&(capIdx_pre == 2)){
			// ok case
		}
		else if((mdomain_cap_ddr_in1status_reg.in1_block_select - capIdx_pre)!=1){
			rtd_pr_scaler_memory_notice("[M-BUF-IDX-CHECK-ERR1] cap idx = %d, pre cap-idx = %d (%d)\n", mdomain_cap_ddr_in1status_reg.in1_block_select, capIdx_pre,
				IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg));
		}else{
			//rtd_pr_scaler_memory_notice("[M-BUF-IDX] cap check OK. \n");
		}
		// m domain cap-check 2
		if((mdomain_disp_ddr_mainstatus_reg.main_block_select== 0)&&(dispIdx_pre == 1)){
			rtd_pr_scaler_memory_notice("[M-BUF-IDX-CHECK-ERR2] disp idx = %d, pre disp idx = %d (%d)\n", mdomain_disp_ddr_mainstatus_reg.main_block_select, dispIdx_pre,
				IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg));
		}
		else if((mdomain_disp_ddr_mainstatus_reg.main_block_select == 1)&&(dispIdx_pre == 2)){
			rtd_pr_scaler_memory_notice("[M-BUF-IDX-CHECK-ERR2] disp idx = %d, pre disp idx = %d (%d)\n", mdomain_disp_ddr_mainstatus_reg.main_block_select, dispIdx_pre,
				IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg));
		}else if((mdomain_disp_ddr_mainstatus_reg.main_block_select == 2)&&(dispIdx_pre == 0)){
			rtd_pr_scaler_memory_notice("[M-BUF-IDX-CHECK-ERR2] disp idx = %d, pre disp idx = %d (%d)\n", mdomain_disp_ddr_mainstatus_reg.main_block_select, dispIdx_pre,
				IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg));
		}
		else{
			//rtd_pr_scaler_memory_notice("[M-BUF-IDX] disp check OK. \n");
		}
	}
	capIdx_pre= mdomain_cap_ddr_in1status_reg.in1_block_select;
	dispIdx_pre= mdomain_disp_ddr_mainstatus_reg.main_block_select;
#endif	// end of #ifndef UT_flag
}

void drv_memory_GameMode_Switch_TripleBuffer(void){
	unsigned int timeout = 0x3ffff;
	mdomain_cap_cap_reg_doublbuffer_RBUS cap_reg_doublbuffer_reg;
	mdomain_disp_ddr_mainsubctrl_RBUS ddr_mainsubctrl_reg;
//	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	mdomain_cap_ddr_in1ctrl_RBUS mdomain_cap_ddr_in1ctrl_reg;
	mdomain_disp_ddr_mainctrl_RBUS mdomain_disp_ddr_mainctrl_reg;
	mdomain_cap_cap_ddr_fifostatus_RBUS cap_ddr_fifostatus_reg;

#if 0//for debug
	mdomain_cap_ddr_in1status_RBUS mdomain_cap_ddr_in1status_reg;
	mdomain_disp_ddr_mainstatus_RBUS mdomain_disp_ddr_mainstatus_reg;
	UINT8 index_check_flag = 0;//for debug
#endif

	UINT8 cap_db_en = 0;
	//UINT8 disp_db_en = 0;
	//UINT8 display_dtg_db_en = 0;
	//unsigned int count = 0x3fffff;

	//enable M-cap double buffer
	cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	cap_db_en = cap_reg_doublbuffer_reg.cap1_db_en;
	cap_reg_doublbuffer_reg.cap1_db_en = 1;
	IoReg_Write32(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);

	//enable M-disp double buffer
	ddr_mainsubctrl_reg.regValue = rtd_inl(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
	//disp_db_en = ddr_mainsubctrl_reg.disp1_double_enable;
	ddr_mainsubctrl_reg.disp1_double_enable = 1;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);

#if 0//for debug
	if(index_check_flag){
		mdomain_cap_ddr_in1status_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg);
		mdomain_disp_ddr_mainstatus_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainStatus_reg);
		rtd_pr_scaler_memory_emerg("[Game Mode]1.cap idx = %d, disp idx = %d\n", mdomain_cap_ddr_in1status_reg.in1_block_select, mdomain_disp_ddr_mainstatus_reg.main_block_select);
	}
#endif
	/////////////////////////////////////////////////////////////////////////////////////////

	//first set M-cap to triple buffer
	mdomain_cap_ddr_in1ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Ctrl_reg);
	mdomain_cap_ddr_in1ctrl_reg.in1_v_flip_3buf_en = 1;
	mdomain_cap_ddr_in1ctrl_reg.in1_double_enable = 1;
	mdomain_cap_ddr_in1ctrl_reg.in1_single_buffer_select = 0;
	IoReg_Write32(MDOMAIN_CAP_DDR_In1Ctrl_reg,mdomain_cap_ddr_in1ctrl_reg.regValue);

	cap_reg_doublbuffer_reg.cap1_db_apply = 1;
	IoReg_Write32(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);
	while((cap_reg_doublbuffer_reg.cap1_db_apply == 1) && (timeout--)){
		cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	}

#if 0//for debug
	if(index_check_flag){
		mdomain_cap_ddr_in1status_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg);
		mdomain_disp_ddr_mainstatus_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainStatus_reg);
		rtd_pr_scaler_memory_emerg("[Game Mode]2.cap idx = %d, disp idx = %d\n", mdomain_cap_ddr_in1status_reg.in1_block_select, mdomain_disp_ddr_mainstatus_reg.main_block_select);
	}
#endif

	timeout = 0x3ffff;

	//wait M-cap write latest 1 frame.
    /*check memory cap down*/
	cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
  	cap_ddr_fifostatus_reg.in1_cap_last_wr_flag = 1;
	IoReg_Write32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg, cap_ddr_fifostatus_reg.regValue);

	cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
	while(!(cap_ddr_fifostatus_reg.in1_cap_last_wr_flag) && --timeout){
		cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
	}

#if 0//for debug
	if(index_check_flag){
		mdomain_cap_ddr_in1status_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg);
		mdomain_disp_ddr_mainstatus_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainStatus_reg);
		rtd_pr_scaler_memory_emerg("[Game Mode]3.cap idx = %d, disp idx = %d\n", mdomain_cap_ddr_in1status_reg.in1_block_select, mdomain_disp_ddr_mainstatus_reg.main_block_select);
	}
#endif

	timeout = 0x3ffff;

	//change M-disp.
	mdomain_disp_ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
	mdomain_disp_ddr_mainctrl_reg.main_double_en = 1;
	mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en = 1;
	mdomain_disp_ddr_mainctrl_reg.main_single_buffer_select = 0;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg,mdomain_disp_ddr_mainctrl_reg.regValue);

	ddr_mainsubctrl_reg.disp1_double_apply = 1;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);
	while((ddr_mainsubctrl_reg.disp1_double_apply == 1) && (timeout--)){
		ddr_mainsubctrl_reg.regValue =rtd_inl(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
	}

#if 0//for debug
	if(index_check_flag){
		mdomain_cap_ddr_in1status_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg);
		mdomain_disp_ddr_mainstatus_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainStatus_reg);
		rtd_pr_scaler_memory_emerg("[Game Mode]4.cap idx = %d, disp idx = %d\n", mdomain_cap_ddr_in1status_reg.in1_block_select, mdomain_disp_ddr_mainstatus_reg.main_block_select);
	}
#endif
	if(Get_DISPLAY_PANEL_OLED_TYPE()== TRUE){
		msleep(100);//wait M-cap write latest 2 frame.
	}

	rtd_pr_new_game_mode_notice("[Game Mode]M-domain game mode disable!!\n");

	/////////////////////////////////////////////////////////////////////////////////////////
	//resume M-cap double buffer setting
	cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_SEARCH) {
		cap_reg_doublbuffer_reg.cap1_db_en = 0;
	} else {
		cap_reg_doublbuffer_reg.cap1_db_en = cap_db_en;
	}
	IoReg_Write32(MDOMAIN_CAP_cap_reg_doublbuffer_reg, cap_reg_doublbuffer_reg.regValue);
#if 0
	//resume M-disp double buffer setting
	ddr_mainsubctrl_reg.regValue = rtd_inl(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
	ddr_mainsubctrl_reg.disp1_double_enable = disp_db_en;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg, ddr_mainsubctrl_reg.regValue);
#endif
}

void dvrif_LowDelay_memory_handler(unsigned char b_switchToSingleBuffer)
{
	UINT8 display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);

	if (display== SLR_MAIN_DISPLAY)
	{
		rtd_pr_scaler_memory_debug("dvrif_smoothtoggle_memory_handler = %d \n", b_switchToSingleBuffer);
		if(b_switchToSingleBuffer) {
			drv_I3ddmaVodma_SingleBuffer_GameMode(_ENABLE);
			drv_memory_GameMode_Switch_SingleBuffer();
		}else{
			drv_I3ddmaVodma_SingleBuffer_GameMode(_DISABLE);
			drv_memory_GameMode_Switch_TripleBuffer();
		}
	}
}

void drv_game_mode_disp_smooth_variable_setting(UINT8 disp_type)
{
//	ppoverlay_dispd_stage1_sm_ctrl_RBUS dispd_stage1_sm_ctrl_reg;
//	ppoverlay_dispd_smooth_toggle1_RBUS dispd_smooth_toggle1_reg;

	rtd_pr_scaler_memory_debug("[Game Mode]scaler_disp_smooth_variable_setting\n");
#if 0
	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 1;
	IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);

	drivf_scaler_reset_freerun();
	dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply=1;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, dispd_smooth_toggle1_reg.regValue);
	//modestate_set_fractional_framesync();
	fw_scaler_dtg_double_buffer_apply();

	dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
	dispd_stage1_sm_ctrl_reg.dispd_stage1_smooth_toggle_protect = 0;
	IoReg_Write32(PPOVERLAY_dispd_stage1_sm_ctrl_reg, dispd_stage1_sm_ctrl_reg.regValue);
#endif
	if(disp_type == GAME_MODE_FRAME_SYNC){
		if(Get_DISPLAY_PANEL_OLED_TYPE()==TRUE)
			modestate_set_fll_running_flag(_DISABLE);
		else{
			modestate_set_fll_running_flag(_DISABLE);//Need to disable FLL flag first.
            drvif_scaler_set_display_mode(DISPLAY_MODE_FRAME_SYNC);
		}
	}else{
		modestate_set_fll_running_flag(_ENABLE);
	}

	fw_scaler_dtg_double_buffer_apply();
}


static unsigned int id2dv_delay_target = 0;
extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr; //extern char run_iv2dv_slow_tuning_flag;//Run iv2dv slow tuning flag

unsigned int drv_GetIv2DvDelayTarget(void)
{
	return id2dv_delay_target;
}

void drv_SetIv2DvDelayTarget(unsigned int a_ulIv2Dv)
{
	id2dv_delay_target = a_ulIv2Dv;
}

void drv_run_GameMode_iv2dv_slow_tuning(void)
{
	ppoverlay_fs_iv_dv_fine_tuning5_RBUS fs_iv_dv_fine_tuning5_reg;
	ppoverlay_display_timing_ctrl1_RBUS display_timing_ctrl1_reg;
	ppoverlay_fs_iv_dv_fine_tuning1_RBUS tuning1_reg;
	ppoverlay_iv2dv_double_buffer_ctrl_RBUS iv2dv_double_buffer_ctrl_reg;
	UINT32 id2dv_delay = 0;
	UINT32 timeout = 10;
	UINT32 iv2dv_db_vsync_sel;


	//if(id2dv_delay_target > 1){
	if(drv_GetIv2DvDelayTarget() > 1){

		display_timing_ctrl1_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL1_reg);
		if(display_timing_ctrl1_reg.disp_frc_fsync==0){
			rtd_pr_new_game_mode_notice("[Game Mode]fsync is not ready\n");
			return;
		}
		//enable iv2dv toggle_en
		tuning1_reg.regValue = IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning1_reg);
		tuning1_reg.iv2dv_toggle_en = _ENABLE;
		IoReg_Write32(PPOVERLAY_FS_IV_DV_Fine_Tuning1_reg, tuning1_reg.regValue);

		fs_iv_dv_fine_tuning5_reg.regValue = IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg);
		id2dv_delay = fs_iv_dv_fine_tuning5_reg.iv2dv_line;

		if(id2dv_delay == 1)
			id2dv_delay=0;

		//if(id2dv_delay < id2dv_delay_target){
		if(id2dv_delay < drv_GetIv2DvDelayTarget()){
			#if 0
			if((id2dv_delay_target-id2dv_delay)<16)
				id2dv_delay += (id2dv_delay_target-id2dv_delay);
			else
				id2dv_delay += 16;
			#endif
			if((drv_GetIv2DvDelayTarget()-id2dv_delay)<16)
				id2dv_delay += (drv_GetIv2DvDelayTarget()-id2dv_delay);
			else
				id2dv_delay += 16;
			iv2dv_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg);
//			IoReg_SetBits(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg,_BIT2);
			iv2dv_db_vsync_sel = iv2dv_double_buffer_ctrl_reg.iv2dv_db_vsync_sel;
			iv2dv_double_buffer_ctrl_reg.iv2dv_db_vsync_sel = 1;
			iv2dv_double_buffer_ctrl_reg.iv2dv_db_en = 1;
			IoReg_Write32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg, iv2dv_double_buffer_ctrl_reg.regValue);

			fs_iv_dv_fine_tuning5_reg.iv2dv_line = id2dv_delay;
			fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = id2dv_delay;
			IoReg_Write32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg, fs_iv_dv_fine_tuning5_reg.regValue);

//			IoReg_SetBits(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg,_BIT0);
			iv2dv_double_buffer_ctrl_reg.iv2dv_db_apply = 1;
			IoReg_Write32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg, iv2dv_double_buffer_ctrl_reg.regValue);

			iv2dv_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg);
			while(iv2dv_double_buffer_ctrl_reg.iv2dv_db_apply &&--timeout){
				msleep(10);
				iv2dv_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg);
			}

//			IoReg_ClearBits(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg,_BIT2);
			iv2dv_double_buffer_ctrl_reg.iv2dv_db_vsync_sel = iv2dv_db_vsync_sel;
			iv2dv_double_buffer_ctrl_reg.iv2dv_db_en = 0;
			IoReg_Write32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg, iv2dv_double_buffer_ctrl_reg.regValue);

			rtd_pr_new_game_mode_notice("[Game Mode] iv2dv line = %d\n", PPOVERLAY_FS_IV_DV_Fine_Tuning5_get_iv2dv_line(IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg)));
			return;
		}
		else{
			//change to 1-buffer after iv2dv done @Crixus 20170527
			if(!Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))
				{
				if ((Scaler_InputSrcGetType(SLR_MAIN_DISPLAY) == _SRC_VO) ||(0==Get_PANEL_VFLIP_ENABLE())) {
					drv_memory_GameMode_Switch_SingleBuffer();
					rtd_pr_scaler_memory_emerg("[Game Mode][%s]Switch single buffer\n", __FUNCTION__);
					}
				}
			rtd_pr_scaler_memory_emerg("[Game Mode] slow tuning iv2dv done\n");
		}

	}
	DbgSclrFlgTkr.run_iv2dv_slow_tuning_flag = FALSE;
}

extern int Get_GamemodeSaveI3ddmaGetcurMode(void);



#define IDOMAIN_IP_DELAY_MARGIN 10
#define MDOMAIN_COMPRESSION_MARGIN 6
//#define I3DDMA_DRAM_MARGIN 10

unsigned int drv_GameMode_iv2dv_delay_compress_margin(unsigned int iv2dv_ori){
	scaledown_ich1_sdnr_cutout_range_ver0_RBUS scaledown_ich1_sdnr_cutout_range_ver0_reg;
	unsigned int active_size = 0;
	scaledown_ich1_sdnr_cutout_range_ver0_reg.regValue = IoReg_Read32(SCALEDOWN_ich1_sdnr_cutout_range_ver0_reg);
	active_size = scaledown_ich1_sdnr_cutout_range_ver0_reg.ver_back - scaledown_ich1_sdnr_cutout_range_ver0_reg.ver_front - 1;//merlin4 sdnr size use active size.

	//margin = (fifo depth * burst length) / compress bit / den size
	iv2dv_ori = iv2dv_ori + ((_FIFOLENGTH * _BURSTLENGTH / dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY)) / active_size) + MDOMAIN_COMPRESSION_MARGIN + 11;
	//rtd_pr_scaler_memory_emerg("[Game Mode]margin = %d\n", ((_FIFOLENGTH * _BURSTLENGTH / dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY)) / active_size + MDOMAIN_COMPRESSION_MARGIN));

	return iv2dv_ori;
}

unsigned int drv_GameMode_get_snr_vstart(void)
{
	nr_snr_timing_monitor_ctrl_RBUS nr_snr_timing_monitor_ctrl_reg;
	unsigned int snr_result1 = 0;
	unsigned int timeout = 20;
	//Set SPNR timing monitor
	//	//Set 1 : measure back porch, vs rising to vact rising
	nr_snr_timing_monitor_ctrl_reg.regValue = IoReg_Read32(NR_SNR_Timing_monitor_ctrl_reg);
	if(nr_snr_timing_monitor_ctrl_reg.timing_monitor_en == 0){
		nr_snr_timing_monitor_ctrl_reg.timing_monitor_src = 1;	//src output
		nr_snr_timing_monitor_ctrl_reg.timing_monitor_unit = 0;	//unit clk cycle
		nr_snr_timing_monitor_ctrl_reg.timing_monitor_multi_mode = 1;	//multi target. 6 target with Result x 1.
		nr_snr_timing_monitor_ctrl_reg.timing_monitor_sta0 = 0;	//vs rising
		nr_snr_timing_monitor_ctrl_reg.timing_monitor_end0 = 2;	//vact rising
		nr_snr_timing_monitor_ctrl_reg.timing_monitor_en = 1;	//Timing_monitor enable
		IoReg_Write32(NR_SNR_Timing_monitor_ctrl_reg,nr_snr_timing_monitor_ctrl_reg.regValue);
		//wait for timing monitor update
		msleep(40);
	}
	snr_result1 = IoReg_Read32(NR_SNR_Timing_monitor_result1_reg);
	while(!snr_result1 && timeout){
		timeout --;
		msleep(0);
		snr_result1 = IoReg_Read32(NR_SNR_Timing_monitor_result1_reg);
	}
	if(!snr_result1)
	{
		rtd_pr_scaler_memory_err("\r### func:%s error snr_result1no result ### \r\n",__FUNCTION__);
		snr_result1 =  Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_STA_PRE);
		nr_snr_timing_monitor_ctrl_reg.regValue = IoReg_Read32(NR_SNR_Timing_monitor_ctrl_reg);
		nr_snr_timing_monitor_ctrl_reg.timing_monitor_en = 0;	//Timing_monitor disable
		IoReg_Write32(NR_SNR_Timing_monitor_ctrl_reg,nr_snr_timing_monitor_ctrl_reg.regValue);
	}
	return snr_result1;
}

unsigned int drv_GameMode_calculate_iv2dv_delay(void)
{

/*==================================================================================
The iv2dv delay formula need match (T2 >= T1) and (T4 >= T3)
T1 = (vodma_htotal * (vodma_vsta+N)) * 1/ (vodma_clk ) ,
T2 = (iv2dv_delay  * (1/vodoma_clk)* vodma_htotal) + (ddomain_htotal * (mdisp_preRead) * 1/(ddomain_clk) ,
                                                                                                       note: ddomain Pre_read= (den_start ¡V 5)
T3 =  (vodma_htotal * (N+vodma_vsta+ vodma_vact)) * 1/ (vodma_clk) ,
T4 = (iv2dv_delay  * (1/vodoma_clk)* vodma_htotal ) +  (ddomain_htotal * (ddomain_vsta+ddomain_vact)) * 1/(ddomain_clk)
====================================================================================*/
	UINT32 voHtotal;
	UINT32 voVactive;
	UINT32 voVstart;
	UINT32 voClock;
	UINT32 iv2dv_delay1;
	UINT32 iv2dv_delay2;
	UINT32 iv2dv_delay;
	UINT32 dHtotal;
	UINT32 dCLK;
	UINT32 dDen_start;
	UINT32 dMv_start;
	UINT32 dMv_act;
	UINT32 mdisp_preRead;
	StructSrcRect m_dispwin;
	SLR_VOINFO *pVOInfo = NULL;
//	ppoverlay_fs_iv_dv_fine_tuning5_RBUS fs_iv_dv_fine_tuning5_reg;
	mdomain_disp_ddr_mainprevstart_RBUS mdomain_disp_ddr_mainprevstart_reg;
	UINT32 temp1;
	UINT32 temp2;
	UINT32 temp3;
	scaledown_ich1_sdnr_cutout_range_ver0_RBUS scaledown_ich1_sdnr_cutout_range_ver0_reg;
	UINT32 snrVstart;
	UINT32 hdmi_vfreq = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ);
	UINT32 hdmi_htotal = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHLEN);
	UINT32 hdmi_vtotal = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVLEN);
	UINT32 hdmi_vstart = Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVSTARTPOS);

	scaledown_ich1_sdnr_cutout_range_ver0_reg.regValue = IoReg_Read32(SCALEDOWN_ich1_sdnr_cutout_range_ver0_reg);
	voHtotal = Scaler_DispGetInputInfoByDisp(0, SLR_INPUT_H_LEN);
	voVactive = scaledown_ich1_sdnr_cutout_range_ver0_reg.ver_back - scaledown_ich1_sdnr_cutout_range_ver0_reg.ver_front - 1;//merlin4 sdnr size use active size.
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());

	if(!get_force_i3ddma_enable(SLR_MAIN_DISPLAY))
	{
		if((hdmi_vfreq == 0)|| (hdmi_vtotal == 0) || (hdmi_htotal == 0))
		{
			rtd_pr_scaler_memory_emerg("[IV2DV]Invalid HDMI Info!!!!\n");
			return 0;
		}
	}
	else if((pVOInfo == NULL) || (pVOInfo->h_total == 0)){
		rtd_pr_scaler_memory_emerg("[IV2DV]Invalid VOInfo!!!!\n");
		return 0;
	}

	if(!get_force_i3ddma_enable(SLR_MAIN_DISPLAY))
	{
		snrVstart = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_STA_PRE) / hdmi_htotal;
		if(snrVstart < hdmi_vstart)
			snrVstart = hdmi_vstart;
	}
	else
	{
		snrVstart = drv_GameMode_get_snr_vstart() / pVOInfo->h_total;
		if(snrVstart < pVOInfo->v_start)
			snrVstart = pVOInfo->v_start;
	}

	voVstart = snrVstart +scaledown_ich1_sdnr_cutout_range_ver0_reg.ver_front;//Scaler_DispGetInputInfoByDisp(0, SLR_INPUT_SDNRV_ACT_STA);

	if(!get_force_i3ddma_enable(SLR_MAIN_DISPLAY))
	{
		if((hdmi_htotal != 0) && (hdmi_vtotal != 0) && (hdmi_vfreq != 0)){
			voClock = hdmi_htotal * hdmi_vtotal * (hdmi_vfreq/10);
		}
		else{
			voClock = pVOInfo->pixel_clk;
		}
	}
	else{
		voClock = pVOInfo->pixel_clk;
	}
	dHtotal = Get_DISP_HORIZONTAL_TOTAL();
	dCLK = Get_DISPLAY_CLOCK_TYPICAL();
	dDen_start =  Get_DISP_DEN_STA_VPOS();
	m_dispwin = Scaler_DispWindowGet();
	dMv_start = m_dispwin.srcy;
	dMv_act = m_dispwin.src_height;

	/*Fix ML3RTANDN-651
	    when interlace signal , game mode calculate iv2dv delay with vo activity after DI size, but using with interlace clock
	    it will cause game mode can not wait for phase error and mute screen for long time.
	    BenWang discuss with DIC and double VO interlace clock value can avoid wait phase error for long time*/
	if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_INTERLACE))
		voClock= voClock*2;

	if(Get_DISPLAY_REFRESH_RATE()>=120){
		// 120hz panel run 2pixel mode, dCLK need multiple 2 for calculate.
		dCLK *= 2;
	}

	mdomain_disp_ddr_mainprevstart_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainPreVStart_reg);
	mdisp_preRead = mdomain_disp_ddr_mainprevstart_reg.main_pre_rd_v_start;

	/* Formula
	(iv2dv_delay  * (1/vodoma_clk)* vodma_htotal) + (ddomain_htotal * (mdisp_preRead) * 1/(ddomain_clk) >=
	(vodma_htotal * (vodma_vsta+N)) * 1/ (vodma_clk )
       =>  iv2dv_delay + (ddomain_htotal/vodma_htotal)*(mdisp_preRead) * (vodoma_clk/(ddomain_clk)) >= ((vodma_vsta+N))
	*/
	//rtd_pr_scaler_memory_emerg("[ben]voHtotal = %d, voVactive = %d, voVstart = %d ,VoClock = %d\n", voHtotal, voVactive, voVstart, voClock);
	//rtd_pr_scaler_memory_emerg("[ben]dHtotal = %d, dCLK = %d, mdisp_preRead = %d, IDOMAIN_IP_DELAY_MARGIN = %d\n", dHtotal, dCLK,
	//	mdisp_preRead, IDOMAIN_IP_DELAY_MARGIN);
	//rtd_pr_scaler_memory_emerg("[ben]dDen_start = %d, dMv_start = %d, dMv_act = %d\n", dDen_start, dMv_start, dMv_act);

	temp1 = (dHtotal*100000)/voHtotal; //multiple : 100000
	temp1 /= 100;	//multiple : 1000
	temp2 = (voClock/(dCLK/10000)); //multiple : 10000
	temp3= (temp1*temp2)/100; //multiple : 100000
	//rtd_pr_scaler_memory_emerg("[ben]temp1=%d,temp2=%d,temp3=%d\n", temp1,temp2,temp3);
	iv2dv_delay1 = (voVstart+IDOMAIN_IP_DELAY_MARGIN)*100000 - (temp3*mdisp_preRead);
	iv2dv_delay1 = (iv2dv_delay1/100000)+1;
	//rtd_pr_scaler_memory_emerg("[ben]iv2dv_delay1 = %d\n", iv2dv_delay1);

	/* Formula
	(iv2dv_delay  * (1/vodoma_clk)* vodma_htotal ) +  (ddomain_htotal * (ddomain_vsta+ddomain_vact)) * 1/(ddomain_clk) >=
	(vodma_htotal * (N+vodma_vsta+ vodma_vact)) * 1/ (vodma_clk)
       => iv2dv_delay+ (ddomain_htotal/vodma_htotal) * (ddomain_vsta+ddomain_vact) * (vodoma_clk/(ddomain_clk)) >=
       (N+vodma_vsta+ vodma_vact)
	*/
	//rtd_pr_scaler_memory_emerg("[ben](voVstart+voVactive+IDOMAIN_IP_DELAY_MARGIN) = %d\n", (voVstart+voVactive+IDOMAIN_IP_DELAY_MARGIN)*100000);
	//rtd_pr_scaler_memory_emerg("[ben](dDen_start+dMv_start+dMv_act) = %d\n", (dDen_start+dMv_start+dMv_act));
	//rtd_pr_scaler_memory_emerg("[ben]temp3*(dDen_start+dMv_start+dMv_act) = %d\n", (temp3*(dDen_start+dMv_start+dMv_act)));
	if((temp3*(dDen_start+dMv_start+dMv_act)) >= (voVstart+voVactive+IDOMAIN_IP_DELAY_MARGIN)*100000){
		iv2dv_delay2 = 0;
	}else{
		iv2dv_delay2 = (voVstart+voVactive+IDOMAIN_IP_DELAY_MARGIN)*100000 - (temp3*(dDen_start+dMv_start+dMv_act));
		iv2dv_delay2 = (iv2dv_delay2/100000)+1;
	}
	//rtd_pr_scaler_memory_emerg("[ben]iv2dv_delay2 = %d\n", iv2dv_delay2);

	iv2dv_delay = ((iv2dv_delay1>iv2dv_delay2)?iv2dv_delay1:iv2dv_delay2);

	if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_INTERLACE))
		iv2dv_delay= iv2dv_delay*2;

	//rtd_pr_scaler_memory_emerg("[ben]iv2dv_delay (final) = %d\n", iv2dv_delay);

	return iv2dv_delay;
}

unsigned int drv_GameMode_decided_iv2dv_delay_old_mode(void)
{
	UINT32 id2dv_delay = 0;

	rtd_pr_scaler_memory_emerg("[ben]drv_GameMode_decided_iv2dv_delay_old_mode \n");

	switch (Get_GamemodeSaveI3ddmaGetcurMode()) {
		case _MODE_480I: //for 720*240 also use this index
			id2dv_delay = 0xc0;
			break;
		case _MODE_640x350_70HZ:			// Mode 00:
		case _MODE_640x350_85HZ:				// Mode 01:
		case _MODE_640x400_56HZ:				// Mode 02:
		case _MODE_640x400_70HZ:				// Mode 03:
		case _MODE_640x400_85HZ:				// Mode 04:
		case _MODE_720x400_50HZ:				// Mode 05:
		case _MODE_720x400_70HZ:				// Mode 06:
		case _MODE_720x400_85HZ:				// Mode 07:
		case _MODE_720x480_60HZ:				// Mode 08:
		case _MODE_640x480_50HZ:				// Mode 09:
		case _MODE_640x480_60HZ:				// Mode 10:
		case _MODE_640x480_67HZ:				// Mode 11:
		case _MODE_640x480_70HZ:				// Mode 12:
		case _MODE_640x480_72HZ:				// Mode 13:
		case _MODE_640x480_75HZ:				// Mode 14:
		case _MODE_640x480_85HZ:				// Mode 15:
		case _MODE_800x600_50HZ:				// Mode 16:
		case _MODE_800x600_56HZ:				// Mode 17:
		case _MODE_800x600_60HZ:				// Mode 18:
		case _MODE_800x600_72HZ:				// Mode 19:
		case _MODE_800x600_75HZ:				// Mode 20:
		case _MODE_800x600_85HZ:				// Mode 21:
		case _MODE_832x624_75HZ:				// Mode 22:
		case _MODE_1024x768_50HZ:				// Mode 23:
		case _MODE_1024x768_60HZ:				// Mode 24:
		case _MODE_1024x768_66HZ:				// Mode 25:
		case _MODE_1024x768_70HZ:				// Mode 26:
		case _MODE_1024x768_75HZ:				// Mode 27:
		case _MODE_1024x768_85HZ:				// Mode 28:
		case _MODE_1152x864_60HZ:				// Mode 29:
		case _MODE_1152x864_75HZ:				// Mode 30:
		case _MODE_1152x870_75HZ:				// Mode 31:
		case _MODE_1152x900_66HZ:				// Mode 32:
		case _MODE_1280x720_50HZ:				// Mode 33:
		case _MODE_1280x720_60HZ:				// Mode 34:
		case _MODE_1280x720_60HZ_RB:			// Mode 35:
		case _MODE_1280x720_85HZ:				// Mode 36:
		case _MODE_VGA_480P:					// Mode 84: RGB 480p
		case _MODE_VGA_576P:					// Mode 85: RGB 576p
		case _MODE_VGA_720P50:					// Mode 86: RGB 720px50Hz
		case _MODE_VGA_720P60:					// Mode 87: RGB 720px60Hz
		case _MODE_480P:
		case _MODE_576I:
		case _MODE_576P:
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1024x576_60HZ:
			id2dv_delay = 0x100;
			break;
		case _MODE_VGA_1080I25: 			// Mode 88: RGB 1080ix50Hz
		case _MODE_VGA_1080I30: 			// Mode 89: RGB 1080ix60Hz
		case _MODE_1080I25:// Mode 100: YPbPr 1920x1080ix50HZ (802R)
		case _MODE_1080I30:// Mode 101: YPbPr 1920x1080ix60HZ (802R)
		case _MODE_VGA_480I://for pc 1440*480@60 1440*240p
		case _MODE_VGA_576I://for pc 1440*576i@50 1440*288p
			id2dv_delay = 0x180;
			break;
		case _MODE_1280x768_50HZ:				// Mode 37:
		case _MODE_1280x768_60HZ:				// Mode 38:
		case _MODE_1280x768_60HZ_RB:			// Mode 39:
		case _MODE_1280x768_75HZ:				// Mode 40:
		case _MODE_1280x768_85HZ:				// Mode 41:
		case _MODE_1280x800_60HZ:				// Mode 42:
		case _MODE_1280x800_60HZ_RB:			// Mode 43:
		case _MODE_1280x800_75HZ:				// Mode 44:
		case _MODE_1280x960_50HZ:				// Mode 45:
		case _MODE_1280x960_60HZ:				// Mode 46:
		case _MODE_1280x960_75HZ:				// Mode 47:
		case _MODE_1280x960_85HZ:				// Mode 48:
		case _MODE_1280x1024_50HZ:			// Mode 49:
		case _MODE_1280x1024_60HZ:			// Mode 50:
		case _MODE_1280x1024_60HZ_RB:			// Mode 51:
		case _MODE_1280x1024_72HZ:			// Mode 52:
		case _MODE_1280x1024_75HZ:			// Mode 53:
		case _MODE_1280x1024_85HZ:			// Mode 54:
		case _MODE_1360x768_60HZ:				// Mode 55:
		case _MODE_1360x768_60HZ_RB:		// Mode 56:
		case _MODE_1366x768_60HZ:				// Mode 57: //VGA1366x768@60
		case _MODE_1400x1050_50HZ:			// Mode 58:
		case _MODE_1400x1050_60RHZ: 		// Mode 59:(Reduced Blanking)
		case _MODE_1400x1050_60HZ:			// Mode 60:
		case _MODE_1400x1050_75HZ:			// Mode 61:
		case _MODE_1440x900_60HZ:				// Mode 62:
		case _MODE_1440x900_60RHZ:			// Mode 63:(Reduced Blanking)
		case _MODE_1440x900_75HZ:				// Mode 64:
		case _MODE_1440x900_85HZ:				// Mode 65:
		case _MODE_1600x1200_50HZ:			// Mode 66:
		case _MODE_1600x1200_60HZ:			// Mode 67:
		case _MODE_1600x1200_65HZ:			// Mode 68:
		case _MODE_1600x1200_70HZ:			// Mode 69:
		case _MODE_1600x1200_75HZ:			// Mode 70:
		case _MODE_1600x1200_85HZ:			// Mode 71:
		case _MODE_1680x1050_50HZ:			// Mode 72:
		case _MODE_1680x1050_60HZ:			// Mode 73:
		case _MODE_1680x1050_60RHZ: 		// Mode 74:(Reduced Blanking)
		case _MODE_1680x1050_75HZ:			// Mode 75:
		case _MODE_1680x1050_85HZ:			// Mode 76:
		case _MODE_1920x1080_50HZ:			// Mode 77:
		case _MODE_1920x1080_60HZ:			// Mode 78:
		case _MODE_1920x1080_60HZ_138:	// Mode 79:(Reduced Blanking)
		case _MODE_1920x1080_60HZ_148:		// Mode 80:(Reduced Blanking)
		case _MODE_1920x1080_75HZ:			// Mode 81:
		case _MODE_1080P50:
		case _MODE_1080P60:
		case _MODE_1080P23:
		case _MODE_1080P24:
		case _MODE_1080P25:
		case _MODE_1080P29:
		case _MODE_1080P30:
		case _MODE_VGA_1080P50: 				// Mode 90: VGA 1920x1080ix50HZ (802R) //CSW+ 0971001
		case _MODE_VGA_1080P60: 			// Mode 91: VGA 1920x1080ix60HZ (802R) //CSW+ 0971001
		case _MODE_1920x1080_60Hz_173MAC:
			id2dv_delay = 0x1c0;
			break;
		//for 4k2k
		case _MODE_4k2kP24:
		case _MODE_4k2kP25:
		case _MODE_4k2kP30:
		case _MODE_4k2kP60:
		case _MODE_4k2kP50:
		case _MODE_NEW://for 4096*2160@50
			id2dv_delay = 0x2e0;
			/*if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)>=2160)
				id2dv_delay = 0x2e0;
			else if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)>=768)
				&&(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)<2160))
				id2dv_delay = 0x3c0;
			else if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)>=600)
				&&(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)<760))
				id2dv_delay = 0x380;
			else if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)<570)
				id2dv_delay = 0x300;*/
			break;
		default:
			id2dv_delay = 2;//default setting
			break;
	}

	return id2dv_delay;

}

unsigned int drv_GameMode_decided_iv2dv_delay_new_mode(void)
{
	UINT32 id2dv_delay = 0;

	//rtd_pr_scaler_memory_emerg("[ben]drv_GameMode_decided_iv2dv_delay_new_mode \n");

	switch (Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR)) {
	/////////// 50/60Hz : iv2dv by formula /////////////
		case _MODE_720x400_50HZ:				// Mode 05:
		case _MODE_640x480_50HZ:				// Mode 09:
		case _MODE_800x600_50HZ:				// Mode 16:
		case _MODE_1024x768_50HZ:				// Mode 23:
		case _MODE_1280x720_50HZ:				// Mode 33:
		case _MODE_VGA_720P50:					// Mode 86: RGB 720px50Hz
		case _MODE_VGA_1080I25: 			// Mode 88: RGB 1080ix50Hz
		case _MODE_1080I25:// Mode 100: YPbPr 1920x1080ix50HZ (802R)
		case _MODE_1280x768_50HZ:				// Mode 37:
		case _MODE_1280x960_50HZ:				// Mode 45:
		case _MODE_1280x1024_50HZ:			// Mode 49:
		case _MODE_1400x1050_50HZ:			// Mode 58:
		case _MODE_1600x1200_50HZ:			// Mode 66:
		case _MODE_1680x1050_50HZ:			// Mode 72:
		case _MODE_1920x1080_50HZ:			// Mode 77:
		case _MODE_VGA_1080P50: 				// Mode 90: VGA 1920x1080ix50HZ (802R) //CSW+ 0971001
		case _MODE_720x480_60HZ:				// Mode 08:
		case _MODE_640x480_60HZ:				// Mode 10:
		case _MODE_800x600_60HZ:				// Mode 18:
		case _MODE_1024x768_60HZ:				// Mode 24:
		case _MODE_1152x864_60HZ:				// Mode 29:
		case _MODE_1280x720_60HZ:				// Mode 34:
		case _MODE_1280x720_60HZ_RB:			// Mode 35:
		case _MODE_VGA_720P60:					// Mode 87: RGB 720px60Hz
		case _MODE_1024x576_60HZ:
		case _MODE_VGA_1080I30: 			// Mode 89: RGB 1080ix60Hz
		case _MODE_VGA_480P:					// Mode 84: RGB 480p
		case _MODE_VGA_576P:					// Mode 85: RGB 576p
		case _MODE_480P:
		case _MODE_576I:
		case _MODE_576P:
		case _MODE_720P50:
		case _MODE_720P60:
		case _MODE_1080I30:// Mode 101: YPbPr 1920x1080ix60HZ (802R)
		case _MODE_1280x768_60HZ:				// Mode 38:
		case _MODE_1280x768_60HZ_RB:			// Mode 39:
		case _MODE_1280x800_60HZ:				// Mode 42:
		case _MODE_1280x800_60HZ_RB:			// Mode 43:
		case _MODE_1280x960_60HZ:				// Mode 46:
		case _MODE_1280x1024_60HZ:			// Mode 50:
		case _MODE_1280x1024_60HZ_RB:			// Mode 51:
		case _MODE_1360x768_60HZ:				// Mode 55:
		case _MODE_1360x768_60HZ_RB:		// Mode 56:
		case _MODE_1366x768_60HZ:				// Mode 57: //VGA1366x768@60
		case _MODE_1400x1050_60HZ:			// Mode 60:
		case _MODE_1440x900_60HZ:				// Mode 62:
		case _MODE_1600x1200_60HZ:			// Mode 67:
		case _MODE_1680x1050_60HZ:			// Mode 73:
		case _MODE_1920x1080_60HZ:			// Mode 78:
		case _MODE_1920x1080_60HZ_138:	// Mode 79:(Reduced Blanking)
		case _MODE_1920x1080_60HZ_148:		// Mode 80:(Reduced Blanking)
		case _MODE_VGA_1080P60: 			// Mode 91: VGA 1920x1080ix60HZ (802R) //CSW+ 0971001
		case _MODE_1400x1050_60RHZ: 		// Mode 59:(Reduced Blanking)
		case _MODE_1440x900_60RHZ:			// Mode 63:(Reduced Blanking)
		case _MODE_1920x1080_60Hz_173MAC:
		case _MODE_1080P50:
		case _MODE_1080P60:
		case _MODE_4k2kP60:
		case _MODE_4k2kP50:
		case _MODE_NEW://for 4096*2160@50
		case _MODE_VGA_480I://for pc 1440*480@60 1440*240p
		case _MODE_VGA_576I://for pc 1440*576i@50 1440*288p
		case _MODE_480I: //for 720*240 also use this index
			id2dv_delay = drv_GameMode_calculate_iv2dv_delay();
			break;

		case _MODE_640x350_70HZ:			    // Mode 00:
		case _MODE_640x350_85HZ:				// Mode 01:
		case _MODE_640x400_56HZ:				// Mode 02:
		case _MODE_640x400_70HZ:				// Mode 03:
		case _MODE_640x400_85HZ:				// Mode 04:
		case _MODE_720x400_70HZ:				// Mode 06:
		case _MODE_720x400_85HZ:				// Mode 07:
		case _MODE_640x480_67HZ:				// Mode 11:
		case _MODE_640x480_70HZ:				// Mode 12:
		case _MODE_640x480_72HZ:				// Mode 13:
		case _MODE_640x480_75HZ:				// Mode 14:
		case _MODE_640x480_85HZ:				// Mode 15:
		case _MODE_800x600_56HZ:				// Mode 17:
		case _MODE_800x600_72HZ:				// Mode 19:
		case _MODE_800x600_75HZ:				// Mode 20:
		case _MODE_800x600_85HZ:				// Mode 21:
		case _MODE_832x624_75HZ:				// Mode 22:
		case _MODE_1024x768_66HZ:				// Mode 25:
		case _MODE_1024x768_70HZ:				// Mode 26:
		case _MODE_1024x768_75HZ:				// Mode 27:
		case _MODE_1024x768_85HZ:				// Mode 28:
		case _MODE_1152x864_75HZ:				// Mode 30:
		case _MODE_1152x870_75HZ:				// Mode 31:
		case _MODE_1152x900_66HZ:				// Mode 32:
		case _MODE_1280x720_85HZ:				// Mode 36:
			id2dv_delay = 0x100;
			break;
		case _MODE_1280x768_75HZ:				// Mode 40:
		case _MODE_1280x768_85HZ:				// Mode 41:
		case _MODE_1280x800_75HZ:				// Mode 44:
		case _MODE_1280x960_75HZ:				// Mode 47:
		case _MODE_1280x960_85HZ:				// Mode 48:
		case _MODE_1280x1024_72HZ:			// Mode 52:
		case _MODE_1280x1024_75HZ:			// Mode 53:
		case _MODE_1280x1024_85HZ:			// Mode 54:
		case _MODE_1400x1050_75HZ:			// Mode 61:
		case _MODE_1440x900_75HZ:				// Mode 64:
		case _MODE_1440x900_85HZ:				// Mode 65:
		case _MODE_1600x1200_65HZ:			// Mode 68:
		case _MODE_1600x1200_70HZ:			// Mode 69:
		case _MODE_1600x1200_75HZ:			// Mode 70:
		case _MODE_1600x1200_85HZ:			// Mode 71:
		case _MODE_1680x1050_60RHZ: 		// Mode 74:(Reduced Blanking)
		case _MODE_1680x1050_75HZ:			// Mode 75:
		case _MODE_1680x1050_85HZ:			// Mode 76:
		case _MODE_1920x1080_75HZ:			// Mode 81:
		case _MODE_1080P23:
		case _MODE_1080P24:
		case _MODE_1080P25:
		case _MODE_1080P29:
		case _MODE_1080P30:
			id2dv_delay = 0x1c0;
			break;
		//for 4k2k
		case _MODE_4k2kP24:
		case _MODE_4k2kP25:
		case _MODE_4k2kP30:
			id2dv_delay = 0x2e0;
			/*if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)>=2160)
				id2dv_delay = 0x2e0;
			else if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)>=768)
				&&(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)<2160))
				id2dv_delay = 0x3c0;
			else if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)>=600)
				&&(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)<760))
				id2dv_delay = 0x380;
			else if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)<570)
				id2dv_delay = 0x300;*/
			break;
		default:
			id2dv_delay = 2;//default setting
			break;
	}

	if(Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) >= 995)
		id2dv_delay = drv_GameMode_calculate_iv2dv_delay();

	return id2dv_delay;

}



void drv_GameMode_iv2dv_delay(unsigned char enable){

	ppoverlay_fs_iv_dv_fine_tuning5_RBUS fs_iv_dv_fine_tuning5_reg;
	ppoverlay_iv2dv_double_buffer_ctrl_RBUS ppoverlay_iv2dv_double_buffer_ctrl_reg;
	UINT32 id2dv_delay = 0;

	UINT32 timeout;
	unsigned long flags = 0;//for spin_lock_irqsave
	ppoverlay_fs_iv_dv_fine_tuning1_RBUS tuning1_reg;

	//rtd_pr_scaler_memory_debug("[pool test]enable:%d,CurMode:%d,modeIdx:%d\n",enable,Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR),Get_GamemodeSaveI3ddmaGetcurMode());
	//fwif_color_safe_od_enable(0);
	if(enable == _ENABLE){
#ifndef BUILD_QUICK_SHOW
		if(IoReg_Read32(0xb8022288)&_BIT19){ //for debug only
			id2dv_delay = drv_GameMode_decided_iv2dv_delay_old_mode();
		}
        else
#endif
        {
			id2dv_delay = drv_GameMode_decided_iv2dv_delay_new_mode();
		}

		//if compression enable, need to add margin. @Crixus 20170605
		if(dvrif_memory_compression_get_enable(SLR_MAIN_DISPLAY))
			id2dv_delay = drv_GameMode_iv2dv_delay_compress_margin(id2dv_delay);

		if (id2dv_delay > Scaler_DispGetInputInfo(SLR_INPUT_V_LEN))
			id2dv_delay = Scaler_DispGetInputInfo(SLR_INPUT_V_LEN) - Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);
	}
	else{
		//iv2dv delay
		id2dv_delay = 2;//default setting
	}

#if 0
	if (id2dv_delay > Scaler_DispGetInputInfo(SLR_INPUT_V_LEN))
		id2dv_delay = Scaler_DispGetInputInfo(SLR_INPUT_V_LEN) - Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);
#endif

	ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg);
	ppoverlay_iv2dv_double_buffer_ctrl_reg.iv2dv_db_en = 1;
	IoReg_Write32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg, ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue);
#ifndef BUILD_QUICK_SHOW
	if(id2dv_delay != 2){
		//id2dv_delay_target = id2dv_delay;
		drv_SetIv2DvDelayTarget(id2dv_delay);
		DbgSclrFlgTkr.run_iv2dv_slow_tuning_flag = TRUE;
	}else{
		//enable iv2dv toggle_en
		tuning1_reg.regValue = IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning1_reg);
		tuning1_reg.iv2dv_toggle_en = _ENABLE;
		IoReg_Write32(PPOVERLAY_FS_IV_DV_Fine_Tuning1_reg, tuning1_reg.regValue);

		timeout=10;
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg,_BIT2|_BIT0);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		drivf_scaler_reset_freerun();
		while((IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg)&_BIT0)&&--timeout){
			msleep(10);
		}
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg,_BIT2);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		IoReg_ClearBits(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg,_BIT2);
		//id2dv_delay_target = 1;
		drv_SetIv2DvDelayTarget(1);
		DbgSclrFlgTkr.run_iv2dv_slow_tuning_flag = FALSE;
		//iv2dv delay
		fs_iv_dv_fine_tuning5_reg.regValue = IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg);
		fs_iv_dv_fine_tuning5_reg.iv2dv_line = id2dv_delay;
		fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = id2dv_delay;
		IoReg_Write32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg, fs_iv_dv_fine_tuning5_reg.regValue);
	}
#else
    fs_iv_dv_fine_tuning5_reg.regValue = IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg);
    fs_iv_dv_fine_tuning5_reg.iv2dv_line = id2dv_delay;
    fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = id2dv_delay;
    IoReg_Write32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg, fs_iv_dv_fine_tuning5_reg.regValue);
#endif
	rtd_pr_new_game_mode_notice("[Game Mode] iv2dv delay = %d\n", id2dv_delay);

	ppoverlay_iv2dv_double_buffer_ctrl_reg.iv2dv_db_apply = 1;
	IoReg_Write32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg, ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue);
	timeout = 0x3fffff;
	ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg);
	while((ppoverlay_iv2dv_double_buffer_ctrl_reg.iv2dv_db_apply) && (--timeout)){
		ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg);
	}
	ppoverlay_iv2dv_double_buffer_ctrl_reg.iv2dv_db_apply = 1;
	IoReg_Write32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg, ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue);
	timeout = 0x3fffff;
	ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg);
	while((ppoverlay_iv2dv_double_buffer_ctrl_reg.iv2dv_db_apply) && (--timeout)){
		ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg);
	}
	ppoverlay_iv2dv_double_buffer_ctrl_reg.iv2dv_db_en = 0;
	IoReg_Write32(PPOVERLAY_iv2dv_Double_Buffer_CTRL_reg, ppoverlay_iv2dv_double_buffer_ctrl_reg.regValue);
	//fwif_color_safe_od_enable(1);
}
#ifndef BUILD_QUICK_SHOW

#ifndef BRING_UP_K4L_TEST //remove in k4lp
void drv_memory_SingleBuffer_GameMode_Dynamic(unsigned char enable){
	unsigned int timeout = 0x3ffff;
#if 0
	mdomain_cap_cap_reg_doublbuffer_RBUS cap_reg_doublbuffer_reg;
	mdomain_disp_ddr_mainsubctrl_RBUS ddr_mainsubctrl_reg;
//	display_timing_ctrl1_RBUS display_timing_ctrl1_reg;

	scaleup_dm_uzu_ctrl_RBUS dm_uzu_Ctrl_reg;
	scaledown_ich1_uzd_ctrl0_RBUS ich1_uzd_Ctrl0_REG;

	UINT8 cap_db_en = 0;
	UINT8 disp_db_en = 0;
#endif
	//mdomain_disp_ddr_mainctrl_RBUS ddr_mainctrl_reg;
	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
	UINT8 display_dtg_db_en = 0;
	unsigned long flags;//for spin_lock_irqsave

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	//enable display double buffer.
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	display_dtg_db_en = double_buffer_ctrl_reg.dreg_dbuf_en;
	double_buffer_ctrl_reg.dreg_dbuf_en = 1;
	rtd_outl(PPOVERLAY_Double_Buffer_CTRL_reg,double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//ublock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	if(enable == TRUE){
#if 0
		//UZU color format - compression disable
		dm_uzu_Ctrl_reg.regValue = rtd_inl(SCALEUP_DM_UZU_Ctrl_reg);
		dm_uzu_Ctrl_reg.in_fmt_conv = !(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)) && Scaler_DispGetStatus(_MAIN_DISPLAY, SLR_DISP_422CAP);
		rtd_outl(SCALEUP_DM_UZU_Ctrl_reg, dm_uzu_Ctrl_reg.regValue);
		//UZD color format - compression disable
		ich1_uzd_Ctrl0_REG.regValue = IoReg_Read32(SCALEDOWN_ICH1_UZD_Ctrl0_reg);
		ich1_uzd_Ctrl0_REG.sort_fmt = (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) || (((dvrif_memory_compression_get_enable() == TRUE))||!(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP))));
		IoReg_Write32(SCALEDOWN_ICH1_UZD_Ctrl0_reg, ich1_uzd_Ctrl0_REG.regValue);
#endif
    if(vbe_disp_gamemode_use_fixlastline()){
        drivf_scaler_reset_freerun();
        drv_game_mode_disp_smooth_variable_setting(GAME_MODE_NEW_FLL);
    }else{
		fwif_color_safe_od_enable(0);
		drv_game_mode_disp_smooth_variable_setting(GAME_MODE_FRAME_SYNC);
		drv_GameMode_iv2dv_delay(_ENABLE);
		fwif_color_safe_od_enable(1);
    }
#if 0
		//apply
		double_buffer_ctrl_reg.dreg_dbuf_set = 1;
		rtd_outl(PPOVERLAY_Double_Buffer_CTRL_reg,double_buffer_ctrl_reg.regValue);
		while((double_buffer_ctrl_reg.dreg_dbuf_set == 1) && (timeout--)){
			double_buffer_ctrl_reg.regValue =rtd_inl(PPOVERLAY_Double_Buffer_CTRL_reg);
		}

		timeout = 0x3ffff;

		//first change M-disp.
		IoReg_ClearBits(MDOMAIN_DISP_DDR_MainCtrl_reg, MDOMAIN_DISP_DDR_MainCtrl_main_double_en_mask|MDOMAIN_DISP_DDR_MainCtrl_main_v_flip_3buf_en_mask);
		ddr_mainsubctrl_reg.disp1_double_apply = 1;
		rtd_outl(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);
		while((ddr_mainsubctrl_reg.disp1_double_apply == 1) && (timeout--)){
			ddr_mainsubctrl_reg.regValue =rtd_inl(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
		}

		timeout = 0x3ffff;

		//change M-cap
		IoReg_ClearBits(MDOMAIN_CAP_DDR_In1Ctrl_reg, MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask|MDOMAIN_CAP_DDR_In1Ctrl_in1_v_flip_3buf_en_mask);
		cap_reg_doublbuffer_reg.cap1_db_apply = 1;
		rtd_outl(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);
		while((cap_reg_doublbuffer_reg.cap1_db_apply == 1) && (timeout--)){
			cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
		}
#endif
		rtd_pr_scaler_memory_debug("[Game Mode]M-domain game mode enable!!\n");
	}
	else{
#if 0
		//UZU color format - compression enable
		dm_uzu_Ctrl_reg.regValue = rtd_inl(SCALEUP_DM_UZU_Ctrl_reg);
		dm_uzu_Ctrl_reg.in_fmt_conv = 0;
		rtd_outl(SCALEUP_DM_UZU_Ctrl_reg, dm_uzu_Ctrl_reg.regValue);

		//UZD color format - compression enable
		ich1_uzd_Ctrl0_REG.regValue = IoReg_Read32(SCALEDOWN_ICH1_UZD_Ctrl0_reg);
		ich1_uzd_Ctrl0_REG.sort_fmt = 1;
		IoReg_Write32(SCALEDOWN_ICH1_UZD_Ctrl0_reg, ich1_uzd_Ctrl0_REG.regValue);

		//first set M-cap to triple buffer
		IoReg_SetBits(MDOMAIN_CAP_DDR_In1Ctrl_reg, MDOMAIN_CAP_DDR_In1Ctrl_in1_double_enable_mask|MDOMAIN_CAP_DDR_In1Ctrl_in1_v_flip_3buf_en_mask);
		cap_reg_doublbuffer_reg.cap1_db_apply = 1;
		rtd_outl(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);
		while((cap_reg_doublbuffer_reg.cap1_db_apply == 1) && (timeout--)){
			cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
		}

		msleep(100);//wait M-cap write latest 2 frame.
		timeout = 0x3ffff;

		//change M-disp.
		IoReg_SetBits(MDOMAIN_DISP_DDR_MainCtrl_reg, MDOMAIN_DISP_DDR_MainCtrl_main_double_en_mask|MDOMAIN_DISP_DDR_MainCtrl_main_v_flip_3buf_en_mask);
		ddr_mainsubctrl_reg.disp1_double_apply = 1;
		rtd_outl(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);
		while((ddr_mainsubctrl_reg.disp1_double_apply == 1) && (timeout--)){
			ddr_mainsubctrl_reg.regValue =rtd_inl(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
		}
#endif
		if(Get_DISPLAY_PANEL_OLED_TYPE()== TRUE){
			msleep(100);//wait M-cap write latest 2 frame.
		}
		drivf_scaler_reset_freerun();
#ifdef CONFIG_DATA_FRAMESYNC_FORCE_422_CAP
		//VO 4k timing use non-gatting flow, do not calculate iv2dv delay here
		if(!((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_IPH_ACT_WID_PRE) > 1920) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_IPV_ACT_LEN_PRE) > 1080)))
#endif
			drv_GameMode_iv2dv_delay(_DISABLE);
		drv_game_mode_disp_smooth_variable_setting(GAME_MODE_NEW_FLL);

		timeout = 0x3ffff;
		//apply
		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		double_buffer_ctrl_reg.dreg_dbuf_set = 1;
		rtd_outl(PPOVERLAY_Double_Buffer_CTRL_reg,double_buffer_ctrl_reg.regValue);
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		while((double_buffer_ctrl_reg.dreg_dbuf_set == 1) && (timeout--)){
			double_buffer_ctrl_reg.regValue =rtd_inl(PPOVERLAY_Double_Buffer_CTRL_reg);
		}
		rtd_pr_scaler_memory_debug("[Game Mode]M-domain game mode disable!!\n");
	}

	//resume D-domain double buffer setting
	//enable display double buffer.
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
	double_buffer_ctrl_reg.dreg_dbuf_en = display_dtg_db_en;
	rtd_outl(PPOVERLAY_Double_Buffer_CTRL_reg,double_buffer_ctrl_reg.regValue);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
#if 0
	//resume M-cap double buffer setting
	cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_SEARCH) {
		cap_reg_doublbuffer_reg.cap1_db_en = 0;
	} else {
		cap_reg_doublbuffer_reg.cap1_db_en = cap_db_en;
	}
	rtd_outl(MDOMAIN_CAP_cap_reg_doublbuffer_reg, cap_reg_doublbuffer_reg.regValue);
	//resume M-disp double buffer setting
	ddr_mainsubctrl_reg.regValue = rtd_inl(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
	ddr_mainsubctrl_reg.disp1_double_enable = disp_db_en;
	rtd_outl(MDOMAIN_DISP_DDR_MainSubCtrl_reg, ddr_mainsubctrl_reg.regValue);
#endif
}
#else
extern void drvif_framesync_gatting(unsigned char enable);
extern unsigned char drvif_framesync_gatting_do(void);
void drv_game_mode_timing_Dynamic(unsigned char enable){
//	unsigned int timeout = 0x3ffff;
	#if 0
	/*20170629 pinyen remove driver conditions to new_game_mode_tsk*/
	mdomain_disp_ddr_mainctrl_RBUS ddr_mainctrl_reg;
	#endif
//	ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
//	UINT8 display_dtg_db_en = 0;
//	unsigned long flags;//for spin_lock_irqsave

    if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC) ||
        (((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) &&   //VRR always go timing fsync,  do not check timing mode here
        (vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag()))==1)){
        return;
    }

	if(enable == TRUE){

		if(vbe_disp_gamemode_use_fixlastline()){
			drivf_scaler_reset_freerun();
			drv_game_mode_disp_smooth_variable_setting(GAME_MODE_NEW_FLL);
		}else{
			fwif_color_safe_od_enable(0);
			drv_game_mode_disp_smooth_variable_setting(GAME_MODE_FRAME_SYNC);
			//drv_GameMode_iv2dv_delay(_ENABLE);//move to frc2fs driver @Crixus 20170712
			fwif_color_safe_od_enable(1);
		}
		rtd_pr_new_game_mode_notice("[Game Mode]D-domain game mode enable!!\n");
	}
	else{
		if(Get_DISPLAY_PANEL_OLED_TYPE()== TRUE){
			msleep(100);//wait M-cap write latest 2 frame.
		}
		//drivf_scaler_reset_freerun();
		drv_GameMode_iv2dv_delay(_DISABLE);
		drv_game_mode_disp_smooth_variable_setting(GAME_MODE_NEW_FLL);

		rtd_pr_new_game_mode_notice("[Game Mode]D-domain game mode disable!!\n");
	}
}



#endif

#ifdef DUMP_SCALER_MEMORY
#define SCALER_SUB1_FILE_PATH "/tmp/scaler_sub_dump1.raw"
#define SCALER_SUB2_FILE_PATH "/tmp/scaler_sub_dump2.raw"
#define SCALER_SUB3_FILE_PATH "/tmp/scaler_sub_dump3.raw"
#include <linux/fs.h>
static struct file* file_open(const char* path, int flags, int rights) {
	struct file* filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if(IS_ERR(filp)) {
		err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

static void file_close(struct file* file) {
	filp_close(file, NULL);
}

static int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_read(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}

static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_write(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}

static int file_sync(struct file* file) {
	vfs_fsync(file, 0);
	return 0;
}

/*void drvif_sub_memory_free_fixed_virtual_memory(void)
{
	if((drvif_memory_get_memtag_virtual_startaddr(MEMIDX_SUB_THIRD) == 0) ){
		rtd_pr_scaler_memory_debug("fixed virtual address is NULL, not to free!!\n");
		return;
	}
	dvr_unmap_memory((void *)drvif_memory_get_memtag_virtual_startaddr(MEMIDX_SUB_THIRD), M_fixed_virtual_memory_size);
}*/

int scaler_dump_sub_data_to_file(unsigned int bufferindex)
{
	struct file* filp = NULL;
	unsigned int buffer_size = MDOMAIN_SUB_BUFFER_SIZE;;
	unsigned int Allocate_VirAddr;

	//free virtual memory
	//drvif_sub_memory_free_fixed_virtual_memory();
	//set virtual memory size
	buffer_size = PAGE_ALIGN(buffer_size);

	if (bufferindex == 1) {
		rtd_pr_scaler_memory_emerg("\n\n\n\n ***************** scaler_dump_data_to_file start1  ******%x***************\n\n\n\n",IoReg_Read32(MDOMAIN_DISP_DDR_SubAddr_reg));
		//virtual memory allocation	and just use one buffer
		//buffer_size = MemTag[MEMIDX_SUB].Size;
		Allocate_VirAddr = ((unsigned long)dvr_remap_uncached_memory(drvif_memory_get_memtag_startaddr(MEMIDX_SUB), buffer_size, __builtin_return_address(0)));

		struct file* filp = file_open(SCALER_SUB1_FILE_PATH, O_RDWR | O_CREAT, 0);
		if (filp == NULL) {
			rtd_pr_scaler_memory_emerg("file open fail\n");
			return FALSE;
		}
		unsigned long outfileOffset = 0;
		if(!Allocate_VirAddr){
			rtd_pr_scaler_memory_emerg("***[error]addr %d is zero****");
			return FALSE;
		}

		file_write(filp, outfileOffset, (unsigned char*)Allocate_VirAddr, 704*240*2);
		file_sync(filp);
		file_close(filp);
		rtd_pr_scaler_memory_emerg("\n\n\n\n *****************  scaler_dump_data_to_file end  *********************\n\n\n\n");
		return TRUE;
	} else if (bufferindex == 2) {
		rtd_pr_scaler_memory_emerg("\n\n\n\n ***************** scaler_dump_data_to_file start2  ******%x***************\n\n\n\n",IoReg_Read32(MDOMAIN_DISP_DDR_Sub2ndAddr_reg));
		//buffer_size = MemTag[MEMIDX_SUB_SEC].Size;
		Allocate_VirAddr = ((unsigned int)dvr_remap_uncached_memory(drvif_memory_get_memtag_startaddr(MEMIDX_SUB_SEC), buffer_size, __builtin_return_address(0)));
		struct file* filp = file_open(SCALER_SUB2_FILE_PATH, O_RDWR | O_CREAT, 0);
		if (filp == NULL) {
			rtd_pr_scaler_memory_emerg("file open fail\n");
			return FALSE;
		}
		unsigned long outfileOffset = 0;
		if(!Allocate_VirAddr){
			rtd_pr_scaler_memory_emerg("***[error]addr %d is zero****");
			return FALSE;
		}

		file_write(filp, outfileOffset, (unsigned char*)Allocate_VirAddr, 704*240*2);
		file_sync(filp);
		file_close(filp);
		rtd_pr_scaler_memory_emerg("\n\n\n\n *****************  scaler_dump_data_to_file end  *********************\n\n\n\n");
		return TRUE;
	}else if (bufferindex == 3) {
		rtd_pr_scaler_memory_emerg("\n\n\n\n ***************** scaler_dump_data_to_file start3  ******%x***************\n\n\n\n",IoReg_Read32(MDOMAIN_DISP_DDR_Sub3rdAddr_reg));
		//buffer_size = MemTag[MEMIDX_SUB_THIRD].Size;
		Allocate_VirAddr = ((unsigned int)dvr_remap_uncached_memory(drvif_memory_get_memtag_startaddr(MEMIDX_SUB_THIRD), buffer_size, __builtin_return_address(0)));
		struct file* filp = file_open(SCALER_SUB3_FILE_PATH, O_RDWR | O_CREAT, 0);
		if (filp == NULL) {
			rtd_pr_scaler_memory_emerg("file open fail\n");
			return FALSE;
		}
		unsigned long outfileOffset = 0;
		if(!Allocate_VirAddr){
			rtd_pr_scaler_memory_emerg("***[error]addr %d is zero****");
			return FALSE;
		}

		file_write(filp, outfileOffset, (unsigned char*)Allocate_VirAddr, 704*240*2);
		file_sync(filp);
		file_close(filp);
		rtd_pr_scaler_memory_emerg("\n\n\n\n *****************  scaler_dump_data_to_file end  *********************\n\n\n\n");
		return TRUE;
	}
	rtd_pr_scaler_memory_emerg("***[error] wrong memory index****\n");
	return FALSE;
}
#endif
#endif

#if 1//def CONFIG_REALTEK_2K_MODEL_ENABLED
/*======================M-domain Multi-Buffer Controlling=====================*/
/*static UINT8 Mdomain_multibuffer_enable = false;
static UINT8 Mdomain_capture_multibuffer_enable = false;
static UINT8 Mdomain_multibuffer_number = 0;
static unsigned int Mdomain_multibuffer_size = 0;
static UINT8 Mdomain_multibuffer_number_pre = 0;
static UINT8 Mdomain_multibuffer_changed = 0;*/

void drv_memory_Set_multibuffer_flag(UINT8 enable){
	Mdomain_multibuffer_enable = enable;
}

UINT8 drv_memory_Get_multibuffer_flag(void){
	return Mdomain_multibuffer_enable;
}

void drv_memory_Set_multibuffer_number(UINT8 number){
	Mdomain_multibuffer_number = number;
}

UINT8 drv_memory_Get_multibuffer_number(void){
	return Mdomain_multibuffer_number;
}
void drv_memory_Set_multibuffer_number_pre(UINT8 number){
	Mdomain_multibuffer_number_pre = number;
}

UINT8 drv_memory_Get_multibuffer_number_pre(void){
	return Mdomain_multibuffer_number_pre;
}
void drv_memory_Set_capture_multibuffer_flag(UINT8 enable){
	Mdomain_capture_multibuffer_enable = enable;
}

UINT8 drv_memory_Get_capture_multibuffer_flag(void){
	return Mdomain_capture_multibuffer_enable;
}

void drv_memory_Set_multibuffer_size(unsigned int buffer_size){
	Mdomain_multibuffer_size = buffer_size;
}

unsigned int drv_memory_Get_multibuffer_size(void){
	return Mdomain_multibuffer_size;
}

void drv_memory_Set_multibuffer_changed(UINT8 enable){
	Mdomain_multibuffer_changed = enable;
}

UINT8 drv_memory_Get_multibuffer_changed(void){
	return Mdomain_multibuffer_changed;
}
#ifndef BUILD_QUICK_SHOW
void drv_memory_send_multibuffer_number_to_smoothtoogh(void)
{
	unsigned int *pulDataTemp = NULL;
	int ret = 0;

	pulDataTemp = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_MULTIBUFFER_NUMBER);
	if (pulDataTemp) {
		*pulDataTemp=(unsigned int)drv_memory_Get_multibuffer_number();
		*pulDataTemp = htonl(*pulDataTemp);
		if (0 != (ret = Scaler_SendRPC(SCALERIOC_MULTIBUFFER_NUMBER,0,0)))
		{
			rtd_pr_scaler_memory_emerg("ret=%d, SCALERIOC_MULTIBUFFER_NUMBER to smoothgooh fail !!!\n", ret);
		}
	}
}
#endif
void drv_memory_capture_multibuffer_update_addr(unsigned int buffer_size, unsigned int start_addr){
	//update forth buffer
	if((drv_memory_Get_multibuffer_number() > 3) && MemTag[MEMIDX_MAIN_FOURTH].StartAddr){
		//rtd_pr_scaler_memory_emerg("[crixus]buffer_size = %x\n", buffer_size);
		#if 0
		MemTag[MEMIDX_MAIN_FOURTH].StartAddr = drvif_memory_get_data_align(start_addr + buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
		//16 bytes alignment
		MemTag[MEMIDX_MAIN_FOURTH].StartAddr = MemTag[MEMIDX_MAIN_FOURTH].StartAddr & 0x7ffffff0;
		MemTag[MEMIDX_MAIN_FOURTH].Allocate_StartAddr = MemTag[MEMIDX_MAIN_FOURTH].StartAddr;
		MemTag[MEMIDX_MAIN_FOURTH].Size = buffer_size;
		MemTag[MEMIDX_MAIN_FOURTH].Status = ALLOCATED_FROM_PLI;
		#endif
		//update start address
		IoReg_Write32(MDOMAIN_CAP_capm_forth_buf_addr_reg, MemTag[MEMIDX_MAIN_FOURTH].StartAddr);
		IoReg_Write32(MDOMAIN_DISP_main_forth_buf_addr_reg, MemTag[MEMIDX_MAIN_FOURTH].StartAddr);

		#ifdef CONFIG_ARM64
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_FOURTH].StartAddr = %lx\n", MemTag[MEMIDX_MAIN_FOURTH].StartAddr);
		#else
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_FOURTH].StartAddr = %x\n", MemTag[MEMIDX_MAIN_FOURTH].StartAddr);
		#endif
	}

	//update fifth buffer
	if((drv_memory_Get_multibuffer_number() > 4) && MemTag[MEMIDX_MAIN_FIFTH].StartAddr){
		#if 0
		MemTag[MEMIDX_MAIN_FIFTH].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_FOURTH].StartAddr + buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
		//16 bytes alignment
		MemTag[MEMIDX_MAIN_FIFTH].StartAddr = MemTag[MEMIDX_MAIN_FIFTH].StartAddr & 0x7ffffff0;
		MemTag[MEMIDX_MAIN_FIFTH].Allocate_StartAddr = MemTag[MEMIDX_MAIN_FIFTH].StartAddr;
		MemTag[MEMIDX_MAIN_FIFTH].Size = buffer_size;
		MemTag[MEMIDX_MAIN_FIFTH].Status = ALLOCATED_FROM_PLI;
		#endif
		//update start address
		IoReg_Write32(MDOMAIN_CAP_capm_fifth_buf_addr_reg, MemTag[MEMIDX_MAIN_FIFTH].StartAddr);
		IoReg_Write32(MDOMAIN_DISP_main_fifth_buf_addr_reg, MemTag[MEMIDX_MAIN_FIFTH].StartAddr);

		#ifdef CONFIG_ARM64
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_FIFTH].StartAddr = %lx\n", MemTag[MEMIDX_MAIN_FIFTH].StartAddr);
		#else
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_FIFTH].StartAddr = %x\n", MemTag[MEMIDX_MAIN_FIFTH].StartAddr);
		#endif
	}

	//update sixth buffer
	if((drv_memory_Get_multibuffer_number() > 5) && MemTag[MEMIDX_MAIN_SIXTH].StartAddr){
		#if 0
		MemTag[MEMIDX_MAIN_SIXTH].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_FIFTH].StartAddr + buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
		//16 bytes alignment
		MemTag[MEMIDX_MAIN_SIXTH].StartAddr = MemTag[MEMIDX_MAIN_SIXTH].StartAddr & 0x7ffffff0;
		MemTag[MEMIDX_MAIN_SIXTH].Allocate_StartAddr = MemTag[MEMIDX_MAIN_SIXTH].StartAddr;
		MemTag[MEMIDX_MAIN_SIXTH].Size = buffer_size;
		MemTag[MEMIDX_MAIN_SIXTH].Status = ALLOCATED_FROM_PLI;
		#endif
		//update start address
		IoReg_Write32(MDOMAIN_CAP_capm_sixth_buf_addr_reg, MemTag[MEMIDX_MAIN_SIXTH].StartAddr);
		IoReg_Write32(MDOMAIN_DISP_main_sixth_buf_addr_reg, MemTag[MEMIDX_MAIN_SIXTH].StartAddr);

		#ifdef CONFIG_ARM64
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_SIXTH].StartAddr = %lx\n", MemTag[MEMIDX_MAIN_SIXTH].StartAddr);
		#else
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_SIXTH].StartAddr = %x\n", MemTag[MEMIDX_MAIN_SIXTH].StartAddr);
		#endif
	}

	//update seventh buffer
	if(drv_memory_Get_multibuffer_number() > 6 && MemTag[MEMIDX_MAIN_SEVENTH].StartAddr){
		#if 0
		MemTag[MEMIDX_MAIN_SEVENTH].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_SIXTH].StartAddr + buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
		//16 bytes alignment
		MemTag[MEMIDX_MAIN_SEVENTH].StartAddr = MemTag[MEMIDX_MAIN_SEVENTH].StartAddr & 0x7ffffff0;
		MemTag[MEMIDX_MAIN_SEVENTH].Allocate_StartAddr = MemTag[MEMIDX_MAIN_SEVENTH].StartAddr;
		MemTag[MEMIDX_MAIN_SEVENTH].Size = buffer_size;
		MemTag[MEMIDX_MAIN_SEVENTH].Status = ALLOCATED_FROM_PLI;
		#endif
		//update start address
		IoReg_Write32(MDOMAIN_CAP_capm_seventh_buf_addr_reg, MemTag[MEMIDX_MAIN_SEVENTH].StartAddr);
		IoReg_Write32(MDOMAIN_DISP_main_seventh_buf_addr_reg, MemTag[MEMIDX_MAIN_SEVENTH].StartAddr);

		#ifdef CONFIG_ARM64
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_SEVENTH].StartAddr = %lx\n", MemTag[MEMIDX_MAIN_SEVENTH].StartAddr);
		#else
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_SEVENTH].StartAddr = %x\n", MemTag[MEMIDX_MAIN_SEVENTH].StartAddr);
		#endif
	}

	//update eighth buffer
	if((drv_memory_Get_multibuffer_number() > 7) && MemTag[MEMIDX_MAIN_EIGHTH].StartAddr){
		#if 0
		MemTag[MEMIDX_MAIN_EIGHTH].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_SEVENTH].StartAddr + buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
		//16 bytes alignment
		MemTag[MEMIDX_MAIN_EIGHTH].StartAddr = MemTag[MEMIDX_MAIN_EIGHTH].StartAddr & 0x7ffffff0;
		MemTag[MEMIDX_MAIN_EIGHTH].Allocate_StartAddr = MemTag[MEMIDX_MAIN_EIGHTH].StartAddr;
		MemTag[MEMIDX_MAIN_EIGHTH].Size = buffer_size;
		MemTag[MEMIDX_MAIN_EIGHTH].Status = ALLOCATED_FROM_PLI;
		#endif
		//update start address
		IoReg_Write32(MDOMAIN_CAP_capm_eightth_buf_addr_reg, MemTag[MEMIDX_MAIN_EIGHTH].StartAddr);
		IoReg_Write32(MDOMAIN_DISP_main_eightth_buf_addr_reg, MemTag[MEMIDX_MAIN_EIGHTH].StartAddr);

		#ifdef CONFIG_ARM64
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_EIGHTH].StartAddr = %lx\n", MemTag[MEMIDX_MAIN_EIGHTH].StartAddr);
		#else
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_EIGHTH].StartAddr = %x\n", MemTag[MEMIDX_MAIN_EIGHTH].StartAddr);
		#endif
	}
}


void drv_memory_capture_multibuffer_update_boundary(unsigned char buf_num, unsigned int buffer_size){
	unsigned int min_addr= MemTag[MEMIDX_MAIN_THIRD].StartAddr;
	unsigned int max_addr= MemTag[MEMIDX_MAIN].StartAddr;
	int i =buf_num;

	for(i = MEMIDX_MAIN_FOURTH; i <= MEMIDX_MAIN_EIGHTH; i++)
	{
		if(MemTag[i].StartAddr> max_addr)
			max_addr = MemTag[i].StartAddr;
		if(MemTag[i].StartAddr< min_addr)
			min_addr = MemTag[i].StartAddr;
	}
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,(min_addr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(max_addr+buffer_size-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
}

void drv_memory_capture_multibuffer_control(unsigned char buf_num, unsigned char disp2cap_distance){
	mdomain_cap_ddr_in1ctrl_RBUS ddr_in1ctrl_reg;
	mdomain_cap_ddr_in2ctrl_RBUS ddr_in2ctrl_reg;

	//rtd_pr_scaler_memory_emerg("[crixus]buf_num = %x\n", buf_num);
	//rtd_pr_scaler_memory_emerg("[crixus]disp2cap_distance = %x\n", disp2cap_distance);

	if(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY){
		ddr_in1ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Ctrl_reg);
		ddr_in1ctrl_reg.in1_bufnum = buf_num - 1;
		ddr_in1ctrl_reg.in1_disp2cap_distance = disp2cap_distance; //if FRC style is faster mode, HW will use this distance
		IoReg_Write32(MDOMAIN_CAP_DDR_In1Ctrl_reg, ddr_in1ctrl_reg.regValue);
	}
	else if(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_SUB_DISPLAY){
		ddr_in2ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In2Ctrl_reg);
		ddr_in2ctrl_reg.in2_bufnum = buf_num - 1;
		ddr_in2ctrl_reg.in2_disp2cap_distance = disp2cap_distance; //if FRC style is faster mode, HW will use this distance
		IoReg_Write32(MDOMAIN_CAP_DDR_In2Ctrl_reg, ddr_in2ctrl_reg.regValue);

	}
}

void drv_memory_capture_multibuffer_address(unsigned int buf4_addr, unsigned int buf5_addr,unsigned int buf6_addr,unsigned int buf7_addr,unsigned int buf8_addr){
	if(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY){
		IoReg_Write32(MDOMAIN_CAP_capm_forth_buf_addr_reg, buf4_addr);
		IoReg_Write32(MDOMAIN_CAP_capm_fifth_buf_addr_reg, buf5_addr);
		IoReg_Write32(MDOMAIN_CAP_capm_sixth_buf_addr_reg, buf6_addr);
		IoReg_Write32(MDOMAIN_CAP_capm_seventh_buf_addr_reg, buf7_addr);
		IoReg_Write32(MDOMAIN_CAP_capm_eightth_buf_addr_reg, buf8_addr);
	}
	else if(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_SUB_DISPLAY){
		IoReg_Write32(MDOMAIN_CAP_caps_forth_buf_addr_reg, buf4_addr);
		IoReg_Write32(MDOMAIN_CAP_caps_fifth_buf_addr_reg, buf5_addr);
		IoReg_Write32(MDOMAIN_CAP_caps_sixth_buf_addr_reg, buf6_addr);
		IoReg_Write32(MDOMAIN_CAP_caps_seventh_buf_addr_reg, buf7_addr);
		IoReg_Write32(MDOMAIN_CAP_caps_eightth_buf_addr_reg, buf8_addr);
	}
}

void drv_memory_display_multibuffer_control(unsigned char buf_num, unsigned char cap2disp_distance){
	mdomain_disp_ddr_mainctrl_RBUS mdomain_disp_ddr_mainctrl_reg;
	mdomain_disp_ddr_subctrl_RBUS mdomain_disp_ddr_subctrl_reg;

	if(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY){
		mdomain_disp_ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
		mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en=1;
		mdomain_disp_ddr_mainctrl_reg.main_double_en=1;
		mdomain_disp_ddr_mainctrl_reg.main_bufnum= buf_num - 1;
		mdomain_disp_ddr_mainctrl_reg.main_cap2disp_distance = cap2disp_distance; //if FRC style is slow mode, HW will use this distance
		IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg, mdomain_disp_ddr_mainctrl_reg.regValue);
	}
	else if(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_SUB_DISPLAY){
		mdomain_disp_ddr_subctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubCtrl_reg);
		mdomain_disp_ddr_subctrl_reg.sub_bufnum= buf_num - 1;
		mdomain_disp_ddr_subctrl_reg.sub_cap2disp_distance = cap2disp_distance; //if FRC style is slow mode, HW will use this distance
		IoReg_Write32(MDOMAIN_DISP_DDR_SubCtrl_reg, mdomain_disp_ddr_subctrl_reg.regValue);

	}

}

void drv_memory_capture_multibuffer_switch_buffer_number(void)
{
	unsigned char buf_num;
	unsigned int buffer_size;
	mdomain_cap_cap_reg_doublbuffer_RBUS cap_reg_doublbuffer_reg;
	mdomain_disp_ddr_mainsubctrl_RBUS ddr_mainsubctrl_reg;
//	mdomain_disp_ddr_mainctrl_RBUS mdomain_disp_ddr_mainctrl_reg;

#ifndef UT_flag
	mdomain_disp_ddr_mainstatus_RBUS mdomain_disp_ddr_mainstatus_reg;
//	mdomain_cap_ddr_in1ctrl_RBUS mdomain_cap_ddr_in1ctrl_reg;
	mdomain_cap_ddr_in1status_RBUS mdomain_cap_ddr_in1status_reg;

	mdomain_cap_ddr_in1status_reg.regValue=IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg);

	mdomain_disp_ddr_mainstatus_reg.regValue=IoReg_Read32(MDOMAIN_DISP_DDR_MainStatus_reg);
#endif
//	mdomain_cap_ddr_in1ctrl_reg.regValue=IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg);

	buffer_size = scaler_memory_cal_multibuf_size();

	buffer_size = drvif_memory_get_data_align(buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
	drv_memory_Set_multibuffer_size(buffer_size);

	buf_num=drv_memory_Get_multibuffer_number_pre();

	cap_reg_doublbuffer_reg.regValue = IoReg_Read32(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	cap_reg_doublbuffer_reg.cap1_db_en= 1;
	IoReg_Write32(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);

	ddr_mainsubctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
	ddr_mainsubctrl_reg.disp1_double_enable=1;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);

	drv_memory_capture_multibuffer_update_addr( drv_memory_Get_multibuffer_size(), MemTag[MEMIDX_MAIN].StartAddr);

	drv_memory_capture_multibuffer_update_boundary(buf_num,  drv_memory_Get_multibuffer_size());

	drv_memory_limit_boundary_enable(SLR_MAIN_DISPLAY, _DISABLE);

	drv_memory_capture_multibuffer_control(buf_num,buf_num-2);
#ifndef UT_flag
	rtd_pr_scaler_memory_debug("cap Captblock=%d,Dispblock=%d, buf_num=%d\n",mdomain_cap_ddr_in1status_reg.in1_block_select,mdomain_disp_ddr_mainstatus_reg.main_block_select,buf_num);
#endif
	cap_reg_doublbuffer_reg.regValue = IoReg_Read32(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	cap_reg_doublbuffer_reg.cap1_db_apply= 1;
	IoReg_Write32(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);

	ddr_mainsubctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
	ddr_mainsubctrl_reg.disp1_double_apply=1;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);

	drv_memory_Set_capture_multibuffer_flag(_ENABLE);
}

void drv_memory_display_multibuffer_switch_buffer_number(void)
{
	unsigned char buf_num;
	mdomain_disp_ddr_mainsubctrl_RBUS ddr_mainsubctrl_reg;
	mdomain_cap_ddr_in1status_RBUS mdomain_cap_ddr_in1status_reg;
	mdomain_disp_ddr_mainstatus_RBUS mdomain_disp_ddr_mainstatus_reg;
	mdomain_disp_ddr_mainctrl_RBUS mdomain_disp_ddr_mainctrl_reg;

	buf_num=drv_memory_Get_multibuffer_number_pre();

	mdomain_disp_ddr_mainstatus_reg.regValue=IoReg_Read32(MDOMAIN_DISP_DDR_MainStatus_reg);
	mdomain_disp_ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
	ddr_mainsubctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg);

	if(mdomain_disp_ddr_mainctrl_reg.main_double_en && mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en)
	{
		mdomain_disp_ddr_mainctrl_reg.main_double_en=0;
		mdomain_disp_ddr_mainctrl_reg.main_v_flip_3buf_en=0;
		mdomain_disp_ddr_mainctrl_reg.main_single_buffer_select=(mdomain_disp_ddr_mainstatus_reg.main_block_select+1)%(mdomain_disp_ddr_mainctrl_reg.main_bufnum+1);
		IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg,mdomain_disp_ddr_mainctrl_reg.regValue);

		ddr_mainsubctrl_reg.disp1_double_apply=1;
		IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);
	}

	mdomain_cap_ddr_in1status_reg.regValue=IoReg_Read32(MDOMAIN_CAP_DDR_In1Status_reg);

	if(mdomain_cap_ddr_in1status_reg.in1_block_select ==(buf_num-1))
	{
		drv_memory_display_multibuffer_control(buf_num,buf_num-2);

		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr1_reg, IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg)&0x7ffffff0);
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr2_reg, IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg)&0x7ffffff0);

		//drv_memory_Set_multibuffer_flag(_DISABLE);
		drv_memory_Set_capture_multibuffer_flag(0);


		ddr_mainsubctrl_reg.disp1_double_apply=1;
		IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);

	}
	rtd_pr_scaler_memory_debug("disp Captblock=%d,Dispblock=%d, buf_num=%d\n",mdomain_cap_ddr_in1status_reg.in1_block_select,mdomain_disp_ddr_mainstatus_reg.main_block_select,buf_num);
}

void drv_memory_capture_multibuffer_switch_buffer(void)
{
	mdomain_cap_cap_reg_doublbuffer_RBUS cap_reg_doublbuffer_reg;
	unsigned char buf_num;
	unsigned int buffer_size;
	unsigned int timeout = 0x3ffff;

	buffer_size = scaler_memory_cal_multibuf_size();

	buffer_size = drvif_memory_get_data_align(buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
	drv_memory_Set_multibuffer_size(buffer_size);

	buf_num=drv_memory_Get_multibuffer_number();

	cap_reg_doublbuffer_reg.regValue = IoReg_Read32(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	cap_reg_doublbuffer_reg.cap1_db_en= 1;
	IoReg_Write32(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);
	
	IoReg_ClearBits(MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);

	drv_memory_capture_multibuffer_update_addr(drv_memory_Get_multibuffer_size(), MemTag[MEMIDX_MAIN].StartAddr);

	if(drv_memory_Get_multibuffer_flag()) {
			drv_memory_capture_multibuffer_update_boundary(buf_num,  drv_memory_Get_multibuffer_size());
			drv_memory_limit_boundary_enable(SLR_MAIN_DISPLAY, _DISABLE);
	} else {
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,(MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0));
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(MemTag[MEMIDX_MAIN].StartAddr+buffer_size) & 0x7ffffff0);

		//Update m-cap limit boundary
		drv_memory_cap_limit_boundary_setting(SLR_MAIN_DISPLAY, buffer_size);
		drv_memory_disp_limit_boundary_setting(SLR_MAIN_DISPLAY);
		drv_memory_set_limit_boundary_mode(SLR_MAIN_DISPLAY, _ENABLE);
	}

	drv_memory_capture_multibuffer_control(buf_num,buf_num-2);
	
	IoReg_SetBits(MDOMAIN_CAP_Cap_In1_enable_reg, _BIT0);

	cap_reg_doublbuffer_reg.regValue = IoReg_Read32(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	cap_reg_doublbuffer_reg.cap1_db_apply= 1;
	IoReg_Write32(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);

	while((cap_reg_doublbuffer_reg.cap1_db_apply == 1) && (timeout--)){
		cap_reg_doublbuffer_reg.regValue = rtd_inl(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
	}

}

void drv_memory_display_multibuffer_switch_buffer(void)
{
	unsigned char buf_num;
	mdomain_disp_ddr_mainsubctrl_RBUS ddr_mainsubctrl_reg;
	unsigned int timeout = 0x3ffff;
	
	ddr_mainsubctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg);	
	ddr_mainsubctrl_reg.disp1_double_enable=1;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);

	buf_num=drv_memory_Get_multibuffer_number();
	
	IoReg_ClearBits(MDOMAIN_DISP_Disp_main_enable_reg, _BIT0);
	
	IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr1_reg, IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg)&0x7ffffff0);
	IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr2_reg, IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg)&0x7ffffff0);
	
	drv_memory_display_multibuffer_control(buf_num,buf_num-2);

	IoReg_SetBits(MDOMAIN_DISP_Disp_main_enable_reg, _BIT0);

	ddr_mainsubctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg);	
	ddr_mainsubctrl_reg.disp1_double_apply=1;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);


	while((ddr_mainsubctrl_reg.disp1_double_apply == 1) && (timeout--)){
		ddr_mainsubctrl_reg.regValue =rtd_inl(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
	}

}

void drv_memory_display_multibuffer_address(unsigned int buf4_addr, unsigned int buf5_addr,unsigned int buf6_addr,unsigned int buf7_addr,unsigned int buf8_addr){
	if(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY){
		IoReg_Write32(MDOMAIN_DISP_main_forth_buf_addr_reg, buf4_addr);
		IoReg_Write32(MDOMAIN_DISP_main_fifth_buf_addr_reg, buf5_addr);
		IoReg_Write32(MDOMAIN_DISP_main_sixth_buf_addr_reg, buf6_addr);
		IoReg_Write32(MDOMAIN_DISP_main_seventh_buf_addr_reg, buf7_addr);
		IoReg_Write32(MDOMAIN_DISP_main_eightth_buf_addr_reg, buf8_addr);
	}
	else if(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_SUB_DISPLAY){
		IoReg_Write32(MDOMAIN_DISP_sub_forth_buf_addr_reg, buf4_addr);
		IoReg_Write32(MDOMAIN_DISP_sub_fifth_buf_addr_reg, buf5_addr);
		IoReg_Write32(MDOMAIN_DISP_sub_sixth_buf_addr_reg, buf6_addr);
		IoReg_Write32(MDOMAIN_DISP_sub_seventh_buf_addr_reg, buf7_addr);
		IoReg_Write32(MDOMAIN_DISP_sub_eightth_buf_addr_reg, buf8_addr);
	}
}

unsigned int scaler_memory_cal_multibuf_size(void)
{
	unsigned int buffer_size;
	if(!is_2k_memory_buffer_source(SLR_MAIN_DISPLAY))
	{
		buffer_size = i3ddmaCtrl.cap_buffer[0].size;
	} else if(dvrif_memory_compression_get_enable(SLR_MAIN_DISPLAY)) {
#ifdef CONFIG_BW_96B_ALIGNED
#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
		buffer_size =  (drvif_memory_get_data_align((M_DOMAIN_WIDTH_MAX_SIZE * dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY) + 256) / 8, DMA_SPEEDUP_ALIGN_VALUE) * M_DOMAIN_LEN_MAX_SIZE );
#else
		buffer_size =  (drvif_memory_get_data_align((M_DOMAIN_WIDTH_CARVEOUT * dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY) + 256) / 8, DMA_SPEEDUP_ALIGN_VALUE) * M_DOMAIN_LEN_CARVEOUT);
#endif
#else
		buffer_size =  (M_DOMAIN_WIDTH_CARVEOUT * M_DOMAIN_LEN_CARVEOUT * dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY) / 8);
#endif
	} else {
		buffer_size = MDOMAIN_MAIN_BUFFER_SIZE_30BITS;
	}
	rtd_pr_scaler_memory_notice("[%s] buffer_size=%x\n", __FUNCTION__,buffer_size);

	return buffer_size;

}
 #ifndef BUILD_QUICK_SHOW
unsigned char scaler_memory_alloc_multibuf(void)
{
	unsigned int buffer_size = scaler_memory_cal_multibuf_size();
	unsigned long nStartAddr = 0;
	unsigned char *uncache =NULL;

	if(MemTag[MEMIDX_MAIN_FOURTH].Allocate_VirAddr == 0)
	{
		// buffer[0]
		nStartAddr = (unsigned long)dvr_malloc_uncached_specific(buffer_size, GFP_DCU2_FIRST, (void **)&uncache);//(unsigned int)dvr_malloc(size_metadata*2);//for double buffer use
		//if(nStartAddr == (unsigned int)NULL)
		if(nStartAddr == 0)
		{
			rtd_pr_scaler_memory_emerg("[MultiBuf]video delay MEMIDX_MAIN_FOURTH memory failed[%x]\n", (unsigned int)nStartAddr);
			return 1; //malloc memory fail
		}
		MemTag[MEMIDX_MAIN_FOURTH].Allocate_VirAddr = (unsigned long)(nStartAddr);
		//16 bytes alignment
		MemTag[MEMIDX_MAIN_FOURTH].StartAddr = (unsigned int)dvr_to_phys((void*)nStartAddr);
		if(MemTag[MEMIDX_MAIN_FOURTH].StartAddr == 0)
		{
			rtd_pr_scaler_memory_emerg("[MultiBuf]video delay convert MEMIDX_MAIN_FOURTH memory to physical addr failed[%lx]\n", MemTag[MEMIDX_MAIN_FOURTH].StartAddr);
			return 1; //malloc memory fail
		}
		MemTag[MEMIDX_MAIN_FOURTH].Allocate_StartAddr = MemTag[MEMIDX_MAIN_FOURTH].StartAddr;
		MemTag[MEMIDX_MAIN_FOURTH].Size = buffer_size;
		MemTag[MEMIDX_MAIN_FOURTH].Status = ALLOCATED_FROM_PLI;
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_FOURTH].StartAddr = %lx\n", MemTag[MEMIDX_MAIN_FOURTH].StartAddr);
		//update start address
		IoReg_Write32(MDOMAIN_CAP_capm_forth_buf_addr_reg, MemTag[MEMIDX_MAIN_FOURTH].StartAddr);
		IoReg_Write32(MDOMAIN_DISP_main_forth_buf_addr_reg, MemTag[MEMIDX_MAIN_FOURTH].StartAddr);
	}

	if(MemTag[MEMIDX_MAIN_FIFTH].Allocate_VirAddr == 0)
	{
		// buffer[0]
		nStartAddr = (unsigned long)dvr_malloc_uncached_specific(buffer_size, GFP_DCU1_FIRST, (void **)&uncache);//(unsigned int)dvr_malloc(size_metadata*2);//for double buffer use
		//if(nStartAddr == (unsigned int)NULL)
		if(nStartAddr == 0)
		{
			rtd_pr_scaler_memory_emerg("[MultiBuf]video delay MEMIDX_MAIN_FIFTH memory failed[%x]\n", (unsigned int)nStartAddr);
			return 1; //malloc memory fail
		}
		MemTag[MEMIDX_MAIN_FIFTH].Allocate_VirAddr = (unsigned long)(nStartAddr);
		//16 bytes alignment
		MemTag[MEMIDX_MAIN_FIFTH].StartAddr = (unsigned int)dvr_to_phys((void*)nStartAddr);
		if(MemTag[MEMIDX_MAIN_FIFTH].StartAddr == 0)
		{
			rtd_pr_scaler_memory_emerg("[MultiBuf]video delay convert MEMIDX_MAIN_FIFTH memory to physical addr failed[%lx]\n", MemTag[MEMIDX_MAIN_FIFTH].StartAddr);
			return 1; //malloc memory fail
		}
		MemTag[MEMIDX_MAIN_FIFTH].Allocate_StartAddr = MemTag[MEMIDX_MAIN_FIFTH].StartAddr;
		MemTag[MEMIDX_MAIN_FIFTH].Size = buffer_size;
		MemTag[MEMIDX_MAIN_FIFTH].Status = ALLOCATED_FROM_PLI;
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_FIFTH].StartAddr = %lx\n", MemTag[MEMIDX_MAIN_FIFTH].StartAddr);
		//update start address
		IoReg_Write32(MDOMAIN_CAP_capm_fifth_buf_addr_reg, MemTag[MEMIDX_MAIN_FIFTH].StartAddr);
		IoReg_Write32(MDOMAIN_DISP_main_fifth_buf_addr_reg, MemTag[MEMIDX_MAIN_FIFTH].StartAddr);
	}

	if(MemTag[MEMIDX_MAIN_SIXTH].Allocate_VirAddr == 0)
	{
		// buffer[0]
		nStartAddr = (unsigned long)dvr_malloc_uncached_specific(buffer_size, GFP_DCU1_FIRST, (void **)&uncache);//(unsigned int)dvr_malloc(size_metadata*2);//for double buffer use
		//if(nStartAddr == (unsigned int)NULL)
		if(nStartAddr == 0)
		{
			rtd_pr_scaler_memory_emerg("[MultiBuf]video delay MEMIDX_MAIN_SIXTH memory failed[%x]\n", (unsigned int)nStartAddr);
			return 1; //malloc memory fail
		}
		MemTag[MEMIDX_MAIN_SIXTH].Allocate_VirAddr = (unsigned long)(nStartAddr);
		//16 bytes alignment
		MemTag[MEMIDX_MAIN_SIXTH].StartAddr = (unsigned int)dvr_to_phys((void*)nStartAddr);
		if(MemTag[MEMIDX_MAIN_SIXTH].StartAddr == 0)
		{
			rtd_pr_scaler_memory_emerg("[MultiBuf]video delay convert MEMIDX_MAIN_SIXTH memory to physical addr failed[%lx]\n", MemTag[MEMIDX_MAIN_SIXTH].StartAddr);
			return 1; //malloc memory fail
		}
		MemTag[MEMIDX_MAIN_SIXTH].Allocate_StartAddr = MemTag[MEMIDX_MAIN_SIXTH].StartAddr;
		MemTag[MEMIDX_MAIN_SIXTH].Size = buffer_size;
		MemTag[MEMIDX_MAIN_SIXTH].Status = ALLOCATED_FROM_PLI;
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_SIXTH].StartAddr = %lx\n", MemTag[MEMIDX_MAIN_SIXTH].StartAddr);
		//update start address
		IoReg_Write32(MDOMAIN_CAP_capm_sixth_buf_addr_reg, MemTag[MEMIDX_MAIN_SIXTH].StartAddr);
		IoReg_Write32(MDOMAIN_DISP_main_sixth_buf_addr_reg, MemTag[MEMIDX_MAIN_SIXTH].StartAddr);
	}

	if(MemTag[MEMIDX_MAIN_SEVENTH].Allocate_VirAddr == 0)
	{
		// buffer[0]
		nStartAddr = (unsigned long)dvr_malloc_uncached_specific(buffer_size, GFP_DCU1_FIRST, (void **)&uncache);//(unsigned int)dvr_malloc(size_metadata*2);//for double buffer use
		//if(nStartAddr == (unsigned int)NULL)
		if(nStartAddr == 0)
		{
			rtd_pr_scaler_memory_emerg("[MultiBuf]video delay MEMIDX_MAIN_SEVENTH memory failed[%x]\n", (unsigned int)nStartAddr);
			return 1; //malloc memory fail
		}
		MemTag[MEMIDX_MAIN_SEVENTH].Allocate_VirAddr = (unsigned long)(nStartAddr);
		//16 bytes alignment
		MemTag[MEMIDX_MAIN_SEVENTH].StartAddr = (unsigned int)dvr_to_phys((void*)nStartAddr);
		if(MemTag[MEMIDX_MAIN_SEVENTH].StartAddr == 0)
		{
			rtd_pr_scaler_memory_emerg("[MultiBuf]video delay convert MEMIDX_MAIN_SEVENTH memory to physical addr failed[%lx]\n", MemTag[MEMIDX_MAIN_SEVENTH].StartAddr);
			return 1; //malloc memory fail
		}
		MemTag[MEMIDX_MAIN_SEVENTH].Allocate_StartAddr = MemTag[MEMIDX_MAIN_SEVENTH].StartAddr;
		MemTag[MEMIDX_MAIN_SEVENTH].Size = buffer_size;
		MemTag[MEMIDX_MAIN_SEVENTH].Status = ALLOCATED_FROM_PLI;
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_SEVENTH].StartAddr = %lx\n", MemTag[MEMIDX_MAIN_SEVENTH].StartAddr);
		//update start address
		IoReg_Write32(MDOMAIN_CAP_capm_seventh_buf_addr_reg, MemTag[MEMIDX_MAIN_SEVENTH].StartAddr);
		IoReg_Write32(MDOMAIN_DISP_main_seventh_buf_addr_reg, MemTag[MEMIDX_MAIN_SEVENTH].StartAddr);
	}

	if(MemTag[MEMIDX_MAIN_EIGHTH].Allocate_VirAddr == 0)
	{
		// buffer[0]
		nStartAddr = (unsigned long)dvr_malloc_uncached_specific(buffer_size, GFP_DCU1_FIRST, (void **)&uncache);//(unsigned int)dvr_malloc(size_metadata*2);//for double buffer use
		//if(nStartAddr == (unsigned int)NULL)
		if(nStartAddr == 0)
		{
			rtd_pr_scaler_memory_emerg("[MultiBuf]video delay MEMIDX_MAIN_EIGHTH memory failed[%x]\n", (unsigned int)nStartAddr);
			return 1; //malloc memory fail
		}
		MemTag[MEMIDX_MAIN_EIGHTH].Allocate_VirAddr = (unsigned long)(nStartAddr);
		//16 bytes alignment
		MemTag[MEMIDX_MAIN_EIGHTH].StartAddr = (unsigned int)dvr_to_phys((void*)nStartAddr);
		if(MemTag[MEMIDX_MAIN_EIGHTH].StartAddr == 0)
		{
			rtd_pr_scaler_memory_emerg("[MultiBuf]video delay convert MEMIDX_MAIN_EIGHTH memory to physical addr failed[%lx]\n", MemTag[MEMIDX_MAIN_EIGHTH].StartAddr);
			return 1; //malloc memory fail
		}
		MemTag[MEMIDX_MAIN_EIGHTH].Allocate_StartAddr = MemTag[MEMIDX_MAIN_EIGHTH].StartAddr;
		MemTag[MEMIDX_MAIN_EIGHTH].Size = buffer_size;
		MemTag[MEMIDX_MAIN_EIGHTH].Status = ALLOCATED_FROM_PLI;
		rtd_pr_scaler_memory_emerg("[MultiBuf]MemTag[MEMIDX_MAIN_EIGHTH].StartAddr = %lx\n", MemTag[MEMIDX_MAIN_EIGHTH].StartAddr);
		//update start address
		IoReg_Write32(MDOMAIN_CAP_capm_eightth_buf_addr_reg, MemTag[MEMIDX_MAIN_EIGHTH].StartAddr);
		IoReg_Write32(MDOMAIN_DISP_main_eightth_buf_addr_reg, MemTag[MEMIDX_MAIN_EIGHTH].StartAddr);
	}

	return 0;
}

unsigned char scaler_memory_free_multibuf(void)
{	
	int i = 0;
	for (i = MEMIDX_MAIN_FOURTH; i <= MEMIDX_MAIN_EIGHTH; i++) {
		if (MemTag[i].Allocate_VirAddr)
		{
			dvr_free((void *)MemTag[i].Allocate_VirAddr);
			MemTag[i].Allocate_VirAddr = 0;
			MemTag[i].StartAddr = 0;
			MemTag[i].Allocate_StartAddr = 0;
			MemTag[i].Size = 0;
			MemTag[i].Status = UNDEFINED;
			rtd_pr_scaler_memory_emerg("[MultiBuf]free MemTag[%d]\n",i);
		}
	}
	return 0;
}
#endif
#endif

unsigned char cur_main_m_cap_block = 0;//record current m domain capture block. record at vgip start isr

void record_current_m_cap_block(unsigned char display)
{//call by vgip start. we need record at vgip start and use at vgip end. avoid the status has risk at vgip end
	mdomain_cap_ddr_in1status_RBUS mdomain_cap_ddr_in1status_reg;
	if(display == SLR_MAIN_DISPLAY)
	{//main path
		mdomain_cap_ddr_in1status_reg.regValue = rtd_inl(MDOMAIN_CAP_DDR_In1Status_reg);
		cur_main_m_cap_block = mdomain_cap_ddr_in1status_reg.in1_block_select;
	}
	else
	{//sub
	}
}

unsigned char get_current_m_cap_block(unsigned char display)
{//return vgip start cpture blcok
	if(display == SLR_MAIN_DISPLAY)
	{//main path
		return cur_main_m_cap_block;
	}
	else
	{//sub
		return 0;
	}
}


void clear_m_cap_done_status(unsigned char display)
{//use to clear capture status at vgip start
	mdomain_cap_cap_ddr_fifostatus_RBUS cap_ddr_fifostatus_reg;

	if(display == SLR_MAIN_DISPLAY)
	{
		cap_ddr_fifostatus_reg.regValue = 0;
		cap_ddr_fifostatus_reg.in1_cap_last_wr_flag = 1;
		IoReg_Write32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg, cap_ddr_fifostatus_reg.regValue);
	}
	else
	{

	}
}


static void wait_m_cap_done(unsigned char display)
{//use for freeze wait cature done at vgip end
	unsigned int timeout = 0x3ffff;
	unsigned int vgip_linecount = 0;
	mdomain_cap_cap_ddr_fifostatus_RBUS cap_ddr_fifostatus_reg;

	if(display == SLR_MAIN_DISPLAY)
	{
		vgip_linecount = SCALEDOWN_ICH1_line_cnt_get_sdnr_ch1_line_cnt(IoReg_Read32(SCALEDOWN_ICH1_line_cnt_reg));//record current vgip line count
		cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
		while(!(cap_ddr_fifostatus_reg.in1_cap_last_wr_flag) && (SCALEDOWN_ICH1_line_cnt_get_sdnr_ch1_line_cnt(IoReg_Read32(SCALEDOWN_ICH1_line_cnt_reg)) >= vgip_linecount)
			&& (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) && --timeout){//check m domain capture done or vgip over vsync or not active state
			cap_ddr_fifostatus_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
			if(drvif_mode_check_dma_onlinemeasure_status() == FALSE)//online fail
				break;
		}
	}
	else
	{

	}
}



//M-domain freeze firmware mode @Crixus 20170512, sync from k3 elieli
void drv_memory_freeze_fw_mode(unsigned char display, unsigned char enable, unsigned char freeze_block)
{
	mdomain_cap_ddr_in1ctrl_RBUS mdomain_cap_ddr_in1ctrl_reg;
	mdomain_cap_cap_in1_enable_RBUS mdomain_cap_cap_in1_enable_reg;
	mdomain_cap_ddr_in1ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_DDR_In1Ctrl_reg);

	//game mode case, disable cap to freeze
	if(display == SLR_MAIN_DISPLAY)
	{
		if(!get_mdomain_driver_status())//m domain not ready no need to freeze
			return;

		if((drv_memory_get_game_mode_flag() == 1) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_VDEC)){//seamless only hdmi has game mode case
			   if(enable == _ENABLE)
			       wait_m_cap_done(SLR_MAIN_DISPLAY);//disable cap need wait capture done
			   mdomain_cap_cap_in1_enable_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_In1_enable_reg);
			   if(enable == _ENABLE){
					   mdomain_cap_cap_in1_enable_reg.in1_cap_enable = 0;
			   }
			   else{
					   mdomain_cap_cap_in1_enable_reg.in1_cap_enable = 1;
			   }
			   IoReg_Write32(MDOMAIN_CAP_Cap_In1_enable_reg, mdomain_cap_cap_in1_enable_reg.regValue);

		} else {
			   //capture change to 1-buffer mode to freeze disp
			   if(enable == _ENABLE){
					   //mdomain_cap_ddr_in1ctrl_reg.in1_v_flip_3buf_en = 0;//no need
					   mdomain_cap_ddr_in1ctrl_reg.in1_single_buffer_select = freeze_block;// keep capture at the freeze block
					   mdomain_cap_ddr_in1ctrl_reg.in1_double_enable = 0;
			   }
			   else{
					   //mdomain_cap_ddr_in1ctrl_reg.in1_v_flip_3buf_en = 1;//no need
					   mdomain_cap_ddr_in1ctrl_reg.in1_single_buffer_select = 0;
					   mdomain_cap_ddr_in1ctrl_reg.in1_double_enable = 1;
			   }
			   IoReg_Write32(MDOMAIN_CAP_DDR_In1Ctrl_reg, mdomain_cap_ddr_in1ctrl_reg.regValue);
		}
	}
	else
	{//sub
	}
}

void drv_memory_display_set_input_fast(void){
	mdomain_disp_ddr_mainctrl_RBUS mdomain_disp_ddr_mainctrl_reg;
	mdomain_disp_ddr_mainsubctrl_RBUS mdomain_disp_ddr_mainsubctrl_reg;

	mdomain_disp_ddr_mainctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainCtrl_reg);
	mdomain_disp_ddr_mainctrl_reg.main_frc_style = 1;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainCtrl_reg,  mdomain_disp_ddr_mainctrl_reg.regValue);

	mdomain_disp_ddr_mainsubctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
	mdomain_disp_ddr_mainsubctrl_reg.disp1_double_apply = 1;
	IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg,  mdomain_disp_ddr_mainsubctrl_reg.regValue);

	//wait apply
	//while(mdomain_disp_ddr_mainsubctrl_reg.disp1_double_apply && cnt){
	//	mdomain_disp_ddr_mainsubctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
	//}

}


/*======================M-domain I2RND M-cap2 Controlling=====================*/
#ifdef CONFIG_I2RND_ENABLE
void dvrif_memory_main_comp_sub_decomp_setting(UINT8 enable_compression, UINT16 comp_wid, UINT16 comp_len, UINT8 comp_ratio, UINT8 cap_type){
	//M-domain use PQC and PQDC from merlin3 @Crixus 20170515
	mdomain_cap_cap_pq_cmp_RBUS mdomain_cap_cap_pq_cmp_reg;
	mdomain_cap_cap_pq_cmp_pair_RBUS mdomain_cap_cap_pq_cmp_pair_reg;
	mdomain_cap_cap_pq_cmp_bit_RBUS mdomain_cap_cap_pq_cmp_bit_reg;
	mdomain_cap_cap_pq_cmp_enable_RBUS mdomain_cap_cap_pq_cmp_enable_reg;
	mdomain_cap_cap_pq_cmp_allocate_RBUS mdomain_cap_cap_pq_cmp_allocate_reg;
	mdomain_cap_cap_pq_cmp_blk0_add0_RBUS mdomain_cap_cap_pq_cmp_blk0_add0_reg;
	mdomain_cap_cap_pq_cmp_blk0_add1_RBUS mdomain_cap_cap_pq_cmp_blk0_add1_reg;

	mdomain_disp_dispm_sub_pq_decmp_RBUS mdomain_disp_dispm_sub_pq_decmp_reg;
	mdomain_disp_dispm_sub_pq_decmp_pair_RBUS mdomain_disp_dispm_sub_pq_decmp_pair_reg;
	mdomain_disp_dispm_sub_pq_decmp_sat_en_RBUS mdomain_disp_dispm_sub_pq_decmp_sat_en_reg;
	mdomain_disp_ddr_subctrl_RBUS mdomain_disp_ddr_subctrl_reg;
	unsigned int TotalSize = 0;

	rtd_pr_scaler_memory_emerg("[crixus][%s]enable_compression = %d, comp_wid = %d, comp_len = %d, comp_ratio = %d\n", __FUNCTION__, enable_compression, comp_wid, comp_len, comp_ratio);

	if(enable_compression == TRUE){
		/*
			width need to align 32
			line sum bit = (width * compression bit + 256) / 128
			for new PQC and PQDC @Crixus 20170615
		*/
		if((comp_wid % 32) != 0){
			//drop bits
			IoReg_Write32(MDOMAIN_DISP_DDR_MainAddrDropBits_reg, (32 - (comp_wid % 32)));
			comp_wid = comp_wid + (32 - (comp_wid % 32));
		}

		if(cap_type == COMPRESSION_LINE_MODE){
			TotalSize = (comp_wid * comp_ratio + 256) * 10 / 128;
			if((TotalSize % 10) != 0){
				TotalSize = TotalSize / 10 + 1;
			}else{
				TotalSize = TotalSize / 10;
			}
		}
		rtd_pr_scaler_memory_emerg("[crixus]comp_wid = %d, comp_wid/32 = %d\n", comp_wid, (comp_wid/32));
		rtd_pr_scaler_memory_emerg("[crixus]comp_len = %d\n", comp_len);
		rtd_pr_scaler_memory_emerg("[crixus]TotalSize = %d\n", TotalSize);
		/*=======================Main PQC Setting======================================*/

		//capture compression setting
		mdomain_cap_cap_pq_cmp_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_reg);
		mdomain_cap_cap_pq_cmp_reg.cmp_height = comp_len;//set length
		mdomain_cap_cap_pq_cmp_reg.cmp_width_div32 = comp_wid / 32;//set width
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_reg, mdomain_cap_cap_pq_cmp_reg.regValue);

		//compression bits setting
		mdomain_cap_cap_pq_cmp_bit_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_BIT_reg);
		if(cap_type == COMPRESSION_LINE_MODE){
			mdomain_cap_cap_pq_cmp_bit_reg.frame_limit_bit = comp_ratio;
			mdomain_cap_cap_pq_cmp_bit_reg.line_limit_bit = comp_ratio;
		}
		else{//frame mode, line limit bit need to add margin to avoid the peak bandwidth
			mdomain_cap_cap_pq_cmp_bit_reg.frame_limit_bit = comp_ratio;
			mdomain_cap_cap_pq_cmp_bit_reg.line_limit_bit = comp_ratio + M_DOMAIN_PQC_LINE_MARGIN;
		}
		mdomain_cap_cap_pq_cmp_bit_reg.block_limit_bit = 0x3f;//PQC fine tune setting
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_BIT_reg, mdomain_cap_cap_pq_cmp_bit_reg.regValue);

		//compression other setting
		mdomain_cap_cap_pq_cmp_pair_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_PAIR_reg);
		if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_10BIT) == TRUE)
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_bit_width = 1; //10 bits
		else
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_bit_width = 0;//8 bits

		if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == TRUE){
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_format = 0;// 444, let hw dynamic switch 444 or 422.
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_color = 1;
		}else{
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_format = 0;//444
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_data_color = 1;//currently no case rgb channel
		}
		if(cap_type == COMPRESSION_LINE_MODE){
			//use line mode
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_line_mode = 1;
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_line_sum_bit = TotalSize;
		}
		else{
			//use frame mode
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_line_mode = 0;
			mdomain_cap_cap_pq_cmp_pair_reg.cmp_line_sum_bit = 0;
		}
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_PAIR_reg, mdomain_cap_cap_pq_cmp_pair_reg.regValue);

		//compression PQC constrain @Crixus 20170626
		mdomain_cap_cap_pq_cmp_enable_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_ENABLE_reg);
		if(cap_type == COMPRESSION_LINE_MODE){
			mdomain_cap_cap_pq_cmp_enable_reg.fisrt_line_more_en = 0;
		}
		else{
			mdomain_cap_cap_pq_cmp_enable_reg.fisrt_line_more_en = 1;
		}
		//RGB444 setting for PQC @Crixus 20170718
		if ((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == FALSE)){
			mdomain_cap_cap_pq_cmp_enable_reg.g_ratio = 12;
			mdomain_cap_cap_pq_cmp_enable_reg.do_422_mode = 2;//444 format
		}
		else{
			mdomain_cap_cap_pq_cmp_enable_reg.g_ratio = 14;
			mdomain_cap_cap_pq_cmp_enable_reg.do_422_mode = 0;//dynamic format
		}

		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_ENABLE_reg, mdomain_cap_cap_pq_cmp_enable_reg.regValue);

		mdomain_cap_cap_pq_cmp_allocate_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_ALLOCATE_reg);
		//RGB444 setting for PQC @Crixus 20170718
		if ((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == FALSE)){
			mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 13;
			mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 9;
		}
		else{
			mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 16;
			mdomain_cap_cap_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 13;
		}
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_ALLOCATE_reg, mdomain_cap_cap_pq_cmp_allocate_reg.regValue);

		//Update PQC constrain setting for PQC @Crixus 20170725
		mdomain_cap_cap_pq_cmp_blk0_add0_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_BLK0_ADD0_reg);
		mdomain_cap_cap_pq_cmp_blk0_add1_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_BLK0_ADD1_reg);
		if(comp_ratio == COMPRESSION_10_BITS){// minmal compression bit setting
			mdomain_cap_cap_pq_cmp_blk0_add0_reg.blk0_add_value32 = 1;
			mdomain_cap_cap_pq_cmp_blk0_add0_reg.blk0_add_value16 = 1;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value8 = 1;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value4 = 1;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value2 = 1;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value1 = 0;
		}
		else{
			mdomain_cap_cap_pq_cmp_blk0_add0_reg.blk0_add_value32 = 16;
			mdomain_cap_cap_pq_cmp_blk0_add0_reg.blk0_add_value16 = 8;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value8 = 4;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value4 = 2;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value2 = 1;
			mdomain_cap_cap_pq_cmp_blk0_add1_reg.blk0_add_value1 = 0;
		}
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_BLK0_ADD0_reg, mdomain_cap_cap_pq_cmp_blk0_add0_reg.regValue);
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_BLK0_ADD1_reg, mdomain_cap_cap_pq_cmp_blk0_add1_reg.regValue);


		/*=======================Sub PQDC Setting======================================*/

		//display de-compression setting
		mdomain_disp_dispm_sub_pq_decmp_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_reg);
		mdomain_disp_dispm_sub_pq_decmp_reg.decmp_height = comp_len;
		mdomain_disp_dispm_sub_pq_decmp_reg.decmp_width_div32 = comp_wid / 32;
		IoReg_Write32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_reg, mdomain_disp_dispm_sub_pq_decmp_reg.regValue);

		//de-compression bits setting
		mdomain_disp_dispm_sub_pq_decmp_pair_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_PAIR_reg);
		if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_10BIT) == TRUE)
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.decmp_data_bit_width = 1;//10 bits
		else
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.decmp_data_bit_width = 0;//8 bits
		//decompression YUV444 @Crixus 20160902
		if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == TRUE){
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.decmp_data_format = 0;//444
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.decmp_data_color = 1;
		}else{
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.decmp_data_format = 0;//444
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.decmp_data_color = 1;
		}

		if(cap_type == COMPRESSION_LINE_MODE){
			//default use line mode
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.decmp_line_mode = 1;
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.decmp_line_sum_bit = TotalSize;
		}
		else{
			//use frame mode
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.decmp_line_mode = 0;
			mdomain_disp_dispm_sub_pq_decmp_pair_reg.decmp_line_sum_bit = 0;
		}
		IoReg_Write32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_PAIR_reg, mdomain_disp_dispm_sub_pq_decmp_pair_reg.regValue);

		//enable saturation for PQ
		mdomain_disp_dispm_sub_pq_decmp_sat_en_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_SAT_EN_reg);
		mdomain_disp_dispm_sub_pq_decmp_sat_en_reg.saturation_en = 1;
		IoReg_Write32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_SAT_EN_reg, mdomain_disp_dispm_sub_pq_decmp_sat_en_reg.regValue);

		//enable clock
		mdomain_disp_ddr_subctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubCtrl_reg);
		mdomain_disp_ddr_subctrl_reg.sub_decompression_clk_en = 1;
		IoReg_Write32(MDOMAIN_DISP_DDR_SubCtrl_reg, mdomain_disp_ddr_subctrl_reg.regValue);

		//enable compression
		mdomain_cap_cap_pq_cmp_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_reg);
		mdomain_cap_cap_pq_cmp_reg.cmp_en = 1;
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_reg, mdomain_cap_cap_pq_cmp_reg.regValue);

		//enable de-compression
		mdomain_disp_dispm_sub_pq_decmp_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_reg);
		mdomain_disp_dispm_sub_pq_decmp_reg.decmp_en = 1;
		IoReg_Write32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_reg, mdomain_disp_dispm_sub_pq_decmp_reg.regValue);

	}
	else{
		//disable compression & de-compression
		//disable compression
		mdomain_cap_cap_pq_cmp_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_reg);
		mdomain_cap_cap_pq_cmp_reg.cmp_en = 0;
		IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_reg, mdomain_cap_cap_pq_cmp_reg.regValue);

		//disable de-compression
		mdomain_disp_dispm_sub_pq_decmp_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_reg);
		mdomain_disp_dispm_sub_pq_decmp_reg.decmp_en = 0;
		IoReg_Write32(MDOMAIN_DISP_DISPM_SUB_PQ_DECMP_reg, mdomain_disp_dispm_sub_pq_decmp_reg.regValue);

		//disable clock
		mdomain_disp_ddr_subctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubCtrl_reg);
		mdomain_disp_ddr_subctrl_reg.sub_decompression_clk_en = 0;
		IoReg_Write32(MDOMAIN_DISP_DDR_SubCtrl_reg, mdomain_disp_ddr_subctrl_reg.regValue);
	}
}

static void memory_set_capture_i2rnd_sdisp(unsigned char cap_type)
{
	unsigned int TotalSize;
	unsigned int Quotient;
	unsigned char Remainder;
	unsigned int fifoLen = _FIFOLENGTH2;
	unsigned int burstLen = _BURSTLENGTH2;
	unsigned int MemShiftAddr = 0;
	unsigned short droppixel = 0;
	unsigned int display = SLR_SUB_DISPLAY;
	//unsigned int subctrltemp = 0;
	unsigned int comp_width = 0;
	//mdomain_disp_display_byte_channel_swap_RBUS display_byte_channel_swap_reg;
	mdomain_cap_capture2_byte_swap_RBUS capture_byte_swap_reg;
	mdomain_disp_ddr_subctrl_RBUS mdomain_disp_ddr_subctrl_reg;
	scaleup_ds_uzu_ctrl_RBUS scaleup_ds_uzu_ctrl_reg;
	mdomain_disp_display_sub_byte_channel_swap_RBUS mdomain_disp_display_sub_byte_channel_swap_reg;

#if 1
	rtd_pr_scaler_memory_emerg("[memory_set_capture_i2rnd_sdisp]\n");
	rtd_pr_scaler_memory_emerg("SLR_INPUT_CHANNEL = %d\n", Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL));
	rtd_pr_scaler_memory_emerg("IPV_ACT_LEN = 0x%x, IPH_ACT_WID = 0x%x\n", Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN), Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID));
	rtd_pr_scaler_memory_emerg("SLR_INPUT_DI_WID = 0x%x(%d)\n",Scaler_DispGetInputInfo(SLR_INPUT_DI_WID),Scaler_DispGetInputInfo(SLR_INPUT_DI_WID));
	rtd_pr_scaler_memory_emerg("SLR_INPUT_CAP_LEN = 0x%x, SLR_INPUT_CAP_WID = 0x%x\n", Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN), Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID));
	rtd_pr_scaler_memory_emerg("MEM_ACT_LEN = 0x%x, MEM_ACT_WID = 0x%x\n", Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN), Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID));
	rtd_pr_scaler_memory_emerg("SLR_INPUT_MEM_ACT_VSTA = %d, SLR_INPUT_MEM_ACT_HSTA = %d\n", Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_VSTA), Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_HSTA));
	rtd_pr_scaler_memory_emerg("SLR_INPUT_DISP_LEN = 0x%x, SLR_INPUT_DISP_WID = 0x%x\n", Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN), Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID));
	rtd_pr_scaler_memory_emerg("SLR_INPUT_V_FREQ = 0x%x\n", Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ));
#endif
	MemGetBlockShiftAddr(SLR_MAIN_DISPLAY, Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_MEM_ACT_HSTA),Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_MEM_ACT_VSTA), &MemShiftAddr, &droppixel);

	if(dvrif_memory_compression_get_enable(SLR_MAIN_DISPLAY) == TRUE){
		//new PQDC are all align 128, use original width. @Crixus 20170615
		comp_width = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_MEM_ACT_WID);
		IoReg_Mask32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg, 0xfff00000, ((comp_width & 0x0fFF) << 8) | ((fifoLen-(burstLen>>1))>>1));
		TotalSize = memory_get_capture_line_size_compression(dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY));
	}
	else{
		//FIXME test
		TotalSize = memory_get_disp_line_size1(SLR_MAIN_DISPLAY, Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_MEM_ACT_WID) + droppixel);	// calculate the memory size of capture
		IoReg_Mask32(MDOMAIN_DISP_DDR_SubPixNumWTL_reg, 0xffc00000, (( ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_MEM_ACT_WID) +droppixel) & 0x3FFF) << 8) | ((fifoLen - (burstLen>>1))>>1)));
	}
	// Do a division
	memory_division(TotalSize, burstLen, &Quotient, &Remainder);
	// remainder is not allowed to be zero
	if (Remainder == 0) {
		Remainder = burstLen;
		Quotient -= 1;
	}

	//Frame mode and PQC: num is 0xffff, length = remain
	//Need to set maxmun because can not sure how many pqc data on frame mode. @Crixus 20170626
	IoReg_Mask32(MDOMAIN_DISP_DDR_SubRdNumLenRem_reg, 0x00000000, (0xffff << 16) | (burstLen << 8) | Remainder);

	//sub line sep
	if(dvrif_memory_compression_get_enable(SLR_MAIN_DISPLAY) == TRUE){
		TotalSize = memory_get_capture_line_size_compression(dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY));
	}
	else{
		TotalSize = memory_get_capture_size(SLR_MAIN_DISPLAY, MEMCAPTYPE_LINE);
	}
	IoReg_Write32(MDOMAIN_DISP_DDR_SubLineStep_reg,  (TotalSize + TotalSize%2) << 3);//bit 3 need to be 0. rbus rule
	TotalSize = SHL(TotalSize, 3); // unit conversion from 64bits to 8bits

	IoReg_Mask32(MDOMAIN_DISP_DDR_SubLineNum_reg, 0xfffff000, (Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_MEM_ACT_LEN) & 0x0FFF));
	IoReg_Write32(MDOMAIN_DISP_DDR_SubAddr_reg, (MemTag[MEMIDX_MAIN_2].StartAddr + MemShiftAddr)&0x7ffffff0);
	IoReg_Write32(MDOMAIN_DISP_DDR_Sub2ndAddr_reg, (MemTag[MEMIDX_MAIN_2_SEC].StartAddr + MemShiftAddr)&0x7ffffff0);
	IoReg_Write32(MDOMAIN_DISP_DDR_Sub3rdAddr_reg, (MemTag[MEMIDX_MAIN_2_THIRD].StartAddr + MemShiftAddr)&0x7ffffff0);
	IoReg_Write32(MDOMAIN_DISP_DDR_SubAddrDropBits_reg,droppixel);

	//sub disp ctrl setting
	mdomain_disp_ddr_subctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_SubCtrl_reg);
	mdomain_disp_ddr_subctrl_reg.sub_double_en = 1;
	mdomain_disp_ddr_subctrl_reg.sub_v_flip_3buf_en = 1;
	mdomain_disp_ddr_subctrl_reg.sub_bufnum = 2;
	mdomain_disp_ddr_subctrl_reg.sub_cap2disp_distance = 1;
	if(dvrif_memory_compression_get_enable(SLR_MAIN_DISPLAY))
		mdomain_disp_ddr_subctrl_reg.sub_in_format = 1;
	else
		mdomain_disp_ddr_subctrl_reg.sub_in_format = 0;
	mdomain_disp_ddr_subctrl_reg.subcolorspace = 1;
	IoReg_Write32(MDOMAIN_DISP_DDR_SubCtrl_reg, mdomain_disp_ddr_subctrl_reg.regValue);

	//Update main compress to sub decompress driver @Crixus 20170830
	if(dvrif_memory_compression_get_enable(SLR_MAIN_DISPLAY) == TRUE)
		dvrif_memory_main_comp_sub_decomp_setting(TRUE, Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_CAP_WID), Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_CAP_LEN), dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY), COMPRESSION_LINE_MODE);
	else
		dvrif_memory_main_comp_sub_decomp_setting(FALSE, Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_CAP_WID), Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_CAP_LEN), dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY), COMPRESSION_LINE_MODE);

	//Update the sub UZU color format for compression, compression use 444 @Crixus 20170831
	scaleup_ds_uzu_ctrl_reg.regValue= IoReg_Read32(SCALEUP_DS_UZU_Ctrl_reg);
	if(dvrif_memory_compression_get_enable(SLR_MAIN_DISPLAY) == TRUE)
		scaleup_ds_uzu_ctrl_reg.in_fmt_conv = FALSE;
	else
		scaleup_ds_uzu_ctrl_reg.in_fmt_conv = (Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_422CAP) ? 1 : 0);
	IoReg_Write32(SCALEUP_DS_UZU_Ctrl_reg, scaleup_ds_uzu_ctrl_reg.regValue);

	//sub-cap byte swap
	capture_byte_swap_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Capture2_byte_swap_reg);
	capture_byte_swap_reg.in2_1byte_swap = 0;
	capture_byte_swap_reg.in2_2byte_swap = 0;
	capture_byte_swap_reg.in2_4byte_swap = 0;
	capture_byte_swap_reg.in2_8byte_swap = 1;
	IoReg_Write32(MDOMAIN_CAP_Capture2_byte_swap_reg, capture_byte_swap_reg.regValue);

	//sub disp byte swap
	mdomain_disp_display_sub_byte_channel_swap_reg.regValue = IoReg_Read32(MDOMAIN_DISP_Display_sub_byte_channel_swap_reg);//update disp byte swap reg @Crixus 20170515
	if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP))
	{
		mdomain_disp_display_sub_byte_channel_swap_reg.sub_1byte_swap = 0;
		mdomain_disp_display_sub_byte_channel_swap_reg.sub_2byte_swap = 1;
		mdomain_disp_display_sub_byte_channel_swap_reg.sub_4byte_swap = 1;
		mdomain_disp_display_sub_byte_channel_swap_reg.sub_8byte_swap = 0;
	}
	else
	{
		mdomain_disp_display_sub_byte_channel_swap_reg.sub_1byte_swap = 1;
		mdomain_disp_display_sub_byte_channel_swap_reg.sub_2byte_swap = 1;
		mdomain_disp_display_sub_byte_channel_swap_reg.sub_4byte_swap = 1;
		mdomain_disp_display_sub_byte_channel_swap_reg.sub_8byte_swap = 0;
	}
	IoReg_Write32(MDOMAIN_DISP_Display_sub_byte_channel_swap_reg, mdomain_disp_display_sub_byte_channel_swap_reg.regValue);

	//drvif_memory_set_dbuffer_write();
}
#endif
extern void magnifier_color_ultrazoom_config_scaling_up(unsigned char display);
void mdomain_handler_i2rnd_mcap_apvr(void)
{
	mdomain_cap_cap_pq_cmp_RBUS mdomain_cap_cap_pq_cmp_reg;
#ifdef CONFIG_I2RND_ENABLE
	mdomain_cap_capture2_byte_swap_RBUS capture_byte_swap_reg;
#endif
	dvrif_memory_compression_set_enable(SLR_MAIN_DISPLAY, FALSE);
	memory_set_capture(TRUE, MEMCAPTYPE_FRAME, Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)? SLR_SUB_DISPLAY : SLR_MAIN_DISPLAY);
	//disable compression & de-compression
	//disable compression
	mdomain_cap_cap_pq_cmp_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CAP_PQ_CMP_reg);
	mdomain_cap_cap_pq_cmp_reg.cmp_en = 0;
	IoReg_Write32(MDOMAIN_CAP_CAP_PQ_CMP_reg, mdomain_cap_cap_pq_cmp_reg.regValue);

#ifdef CONFIG_I2RND_ENABLE
	Scaler_I2rnd_set_display(I2RND_MAIN_S0_TABLE);
	capture_byte_swap_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Capture2_byte_swap_reg);
	capture_byte_swap_reg.in2_1byte_swap = 0;
	capture_byte_swap_reg.in2_2byte_swap = 0;
	capture_byte_swap_reg.in2_4byte_swap = 0;
	capture_byte_swap_reg.in2_8byte_swap = 1;
	IoReg_Write32(MDOMAIN_CAP_Capture2_byte_swap_reg, capture_byte_swap_reg.regValue);
	Scaler_I2rnd_set_display(I2RND_SUB_S1_TABLE);
	capture_byte_swap_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Capture2_byte_swap_reg);
	capture_byte_swap_reg.in2_1byte_swap = 0;
	capture_byte_swap_reg.in2_2byte_swap = 0;
	capture_byte_swap_reg.in2_4byte_swap = 0;
	capture_byte_swap_reg.in2_8byte_swap = 1;
	IoReg_Write32(MDOMAIN_CAP_Capture2_byte_swap_reg, capture_byte_swap_reg.regValue);
#endif

	rtd_pr_scaler_memory_emerg("#################[%s][%d]Done!!@@@@@@@@@@@@@@@@@\n", __FUNCTION__, __LINE__);
}

#ifdef CONFIG_I2RND_ENABLE
void mdomain_handler_i2rnd_mcap_sdisp(void)
{
	//M-cap to S-disp only support line mode
	memory_set_capture(TRUE, MEMCAPTYPE_LINE, Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)? SLR_SUB_DISPLAY : SLR_MAIN_DISPLAY);
	//set sub disp
	memory_set_capture_i2rnd_sdisp(MEMCAPTYPE_LINE);
	//release sub memory & disable sub cap for dual vo case
	if(vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1){
		mdomain_cap_cap_in2_enable_RBUS mdomain_cap_cap_in2_enable_reg;

		drvif_memory_free_block(MEMIDX_SUB);
		drvif_memory_free_block(MEMIDX_SUB_SEC);
		drvif_memory_free_block(MEMIDX_SUB_THIRD);

		mdomain_cap_cap_in2_enable_reg.regValue = IoReg_Read32(MDOMAIN_CAP_Cap_In2_enable_reg);
		mdomain_cap_cap_in2_enable_reg.in2_cap_enable = _DISABLE;
		IoReg_Write32(MDOMAIN_CAP_Cap_In2_enable_reg, mdomain_cap_cap_in2_enable_reg.regValue);
	}

	rtd_pr_scaler_memory_emerg("[I2RND]#################[%s][%d]Done!!@@@@@@@@@@@@@@@@@\n", __FUNCTION__, __LINE__);
}
#endif
 #ifndef BUILD_QUICK_SHOW
void i2rnd_s1_default_memory_tag(void){
	unsigned int buffer_size = MDOMAIN_MAIN_I2R_BUFFER_SIZE;
	#ifdef CONFIG_ARM64
	unsigned long query_addr = 0, align_addr = 0;
	#else
	unsigned int query_addr = 0, align_addr = 0;
	#endif

	//merlin4 memory do 96 bytes align @Crixus 20180321
	query_addr = get_query_start_address(QUERY_IDX_MDOMAIN_MAIN_I2R);
	buffer_size = dvr_size_alignment(buffer_size);
	align_addr = dvr_memory_alignment((unsigned long)query_addr, buffer_size);
	#ifdef CONFIG_ARM64
	if (align_addr == (unsigned long)NULL)
		BUG();
	#else
	if (align_addr == (unsigned int)NULL)
		BUG();
	#endif

	//update mcap2 memory tag
	MemTag[MEMIDX_MAIN_2_THIRD].StartAddr = align_addr;
	MemTag[MEMIDX_MAIN_2_SEC].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_2_THIRD].StartAddr+buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
	MemTag[MEMIDX_MAIN_2].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_2_SEC].StartAddr+buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
	//update DI2 memory tag
	MemTag[MEMIDX_DI_2].StartAddr = get_query_start_address(QUERY_IDX_DI_2);
}

void i2rnd_default_allocate_mcap1(void){
	unsigned int buffer_size = MDOMAIN_MAIN_I2R_BUFFER_SIZE;
	#ifdef CONFIG_ARM64
	unsigned long query_addr = 0, align_addr = 0;
	#else
	unsigned int query_addr = 0, align_addr = 0;
	#endif

	//merlin4 memory do 96 bytes align @Crixus 20180321
	query_addr = get_query_start_address(QUERY_IDX_MDOMAIN_MAIN);
	buffer_size = dvr_size_alignment(buffer_size);
	align_addr = dvr_memory_alignment((unsigned long)query_addr, buffer_size);
	#ifdef CONFIG_ARM64
	if (align_addr == (unsigned long)NULL)
		BUG();
	#else
	if (align_addr == (unsigned int)NULL)
		BUG();
	#endif

	//allocate memory for M-cap1
	MemTag[MEMIDX_MAIN_THIRD].StartAddr = align_addr;
	MemTag[MEMIDX_MAIN_SEC].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_THIRD].StartAddr+buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
	MemTag[MEMIDX_MAIN].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_SEC].StartAddr+buffer_size, DMA_SPEEDUP_ALIGN_VALUE);

	//16 bytes alignment
	MemTag[MEMIDX_MAIN_THIRD].StartAddr = MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0;
	MemTag[MEMIDX_MAIN_SEC].StartAddr = MemTag[MEMIDX_MAIN_SEC].StartAddr & 0x7ffffff0;
	MemTag[MEMIDX_MAIN].StartAddr = MemTag[MEMIDX_MAIN].StartAddr & 0x7ffffff0;

	//free virtual memory
	drvif_memory_free_fixed_virtual_memory();
	//set virtual memory size
	buffer_size = PAGE_ALIGN(buffer_size);
	drvif_memory_set_fixed_virtual_memory_size(buffer_size);

	MemTag[MEMIDX_MAIN_THIRD].Allocate_StartAddr = MemTag[MEMIDX_MAIN_THIRD].StartAddr;
	MemTag[MEMIDX_MAIN_THIRD].Size = buffer_size;
	MemTag[MEMIDX_MAIN_THIRD].Status = ALLOCATED_FROM_PLI;

	MemTag[MEMIDX_MAIN_SEC].Allocate_StartAddr = MemTag[MEMIDX_MAIN_SEC].StartAddr;
	MemTag[MEMIDX_MAIN_SEC].Size = buffer_size;
	MemTag[MEMIDX_MAIN_SEC].Status = ALLOCATED_FROM_PLI;
	MemTag[MEMIDX_MAIN_SEC].Allocate_VirAddr = 0;//not use

	MemTag[MEMIDX_MAIN].Allocate_StartAddr = MemTag[MEMIDX_MAIN].StartAddr;
	MemTag[MEMIDX_MAIN].Size = buffer_size;
	MemTag[MEMIDX_MAIN].Status = ALLOCATED_FROM_PLI;
	MemTag[MEMIDX_MAIN].Allocate_VirAddr = 0;//not use

	//virtual memory allocation	and just use one buffer
	drvif_memory_set_memtag_virtual_startaddr((unsigned long)dvr_remap_uncached_memory(drvif_memory_get_memtag_startaddr(MEMIDX_MAIN_THIRD), buffer_size, __builtin_return_address(0)));

	//M-domain start address
	IoReg_Write32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg, (MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_DDR_In1_2ndAddr_reg, (MemTag[MEMIDX_MAIN_SEC].StartAddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_DDR_In1Addr_reg, (MemTag[MEMIDX_MAIN].StartAddr & 0x7ffffff0));
	//M-domain boundary
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,(MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(MemTag[MEMIDX_MAIN].StartAddr+buffer_size-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
}

void i2rnd_default_allocate_mcap2(void){
	unsigned int buffer_size = MDOMAIN_MAIN_I2R_BUFFER_SIZE;
	#ifdef CONFIG_ARM64
	unsigned long query_addr = 0, align_addr = 0;
	#else
	unsigned int query_addr = 0, align_addr = 0;
	#endif

	//merlin4 memory do 96 bytes align @Crixus 20180321
	query_addr = get_query_start_address(QUERY_IDX_MDOMAIN_MAIN_I2R);
	buffer_size = dvr_size_alignment(buffer_size);
	align_addr = dvr_memory_alignment((unsigned long)query_addr, buffer_size);
	#ifdef CONFIG_ARM64
	if (align_addr == (unsigned long)NULL)
		BUG();
	#else
	if (align_addr == (unsigned int)NULL)
		BUG();
	#endif

	MemTag[MEMIDX_MAIN_2_THIRD].StartAddr = align_addr;
	MemTag[MEMIDX_MAIN_2_SEC].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_2_THIRD].StartAddr+buffer_size, DMA_SPEEDUP_ALIGN_VALUE);
	MemTag[MEMIDX_MAIN_2].StartAddr = drvif_memory_get_data_align(MemTag[MEMIDX_MAIN_2_SEC].StartAddr+buffer_size, DMA_SPEEDUP_ALIGN_VALUE);

	//M-domain start address
	IoReg_Write32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg, (MemTag[MEMIDX_MAIN_2_THIRD].StartAddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_DDR_In1_2ndAddr_reg, (MemTag[MEMIDX_MAIN_2_SEC].StartAddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_DDR_In1Addr_reg, (MemTag[MEMIDX_MAIN_2].StartAddr & 0x7ffffff0));
	//M-domain boundary
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg,(MemTag[MEMIDX_MAIN_2_THIRD].StartAddr & 0x7ffffff0));
	IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg,(MemTag[MEMIDX_MAIN_2].StartAddr+buffer_size-CONFIG_MDOMAIN_BURST_SIZE) & 0x7ffffff0);
}

void i2rnd_default_allocate_DI1(void){
	unsigned int default_buffer_size = 0x280000;
	unsigned int default_info_size = 0x200000;
	unsigned int di_address = 0;

	di_address = get_query_start_address(QUERY_IDX_DI);

	//start address setting
	IoReg_Write32(DI_DI_DATMemoryStartAdd1_reg, (di_address & 0x7ffffff0));
	IoReg_Write32(DI_DI_DATMemoryStartAdd2_reg, ((di_address + default_buffer_size) & 0x7ffffff0));
	IoReg_Write32(DI_DI_DATMemoryStartAdd3_reg, ((di_address + default_buffer_size * 2) & 0x7ffffff0));
	IoReg_Write32(DI_DI_DATMemoryStartAdd4_reg, ((di_address + default_buffer_size * 3) & 0x7ffffff0));
	IoReg_Write32(DI_DI_INFMemoryStartAdd1_reg, ((di_address + default_buffer_size * 4) & 0x7ffffff0));
	IoReg_Write32(DI_DI_INFMemoryStartAdd2_reg, ((di_address + default_buffer_size * 4 + default_info_size) & 0x7ffffff0));

	//boundary setting
	IoReg_Write32(DI_DI1_WRRD_BoundaryAddr1_reg, (di_address & 0x7ffffff0));
	IoReg_Write32(DI_DI1_WRRD_BoundaryAddr2_reg, ((di_address + default_buffer_size * 4 + default_info_size * 2) & 0x7ffffff0));
}

void i2rnd_default_allocate_DI2(void){
#if 0
	unsigned int default_buffer_size = 0x280000;
	unsigned int default_info_size = 0x200000;
	unsigned int di_address = 0;

	//DI 2 size need to add DI 1 size
	di_address = get_query_start_address(QUERY_IDX_DI_2);
	//start address setting
	IoReg_Write32(DI_DI_I2R_DATMemoryStartAdd1_reg, (di_address & 0x7ffffff0));
	IoReg_Write32(DI_DI_I2R_DATMemoryStartAdd2_reg, ((di_address + default_buffer_size) & 0x7ffffff0));
	IoReg_Write32(DI_DI_I2R_DATMemoryStartAdd3_reg, ((di_address + default_buffer_size * 2) & 0x7ffffff0));
	IoReg_Write32(DI_DI_I2R_DATMemoryStartAdd4_reg, ((di_address + default_buffer_size * 3) & 0x7ffffff0));
	IoReg_Write32(DI_DI_I2R_INFMemoryStartAdd1_reg, ((di_address + default_buffer_size * 4) & 0x7ffffff0));
	IoReg_Write32(DI_DI_I2R_INFMemoryStartAdd2_reg, ((di_address + default_buffer_size * 4 + default_info_size) & 0x7ffffff0));

	//boundary setting
	IoReg_Write32(DI_DI1_I2R_WRRD_BoundaryAddr1_reg, (di_address & 0x7ffffff0));
	IoReg_Write32(DI_DI1_I2R_WRRD_BoundaryAddr2_reg, ((di_address + default_buffer_size * 4 + default_info_size * 2) & 0x7ffffff0));
#endif
}

void i2rnd_get_mcap2_address(unsigned int *addr1, unsigned int *addr2, unsigned int *addr3)
{

	*addr1 = MemTag[MEMIDX_MAIN_2].StartAddr;
	*addr2 = MemTag[MEMIDX_MAIN_2_SEC].StartAddr;
	*addr3 = MemTag[MEMIDX_MAIN_2_THIRD].StartAddr;

}
#endif

/*Merlin4 new driver @Crixus 20180320*/
static unsigned char limit_boundary_mode[2] = {0};//main and sub use different flag
void drv_memory_set_limit_boundary_mode(unsigned char display, unsigned char enable){
	limit_boundary_mode[display] = enable;
}

unsigned char drv_memory_get_limit_boundary_mode(unsigned char display){
	return limit_boundary_mode[display];
}

void drv_memory_limit_boundary_enable(unsigned char display, unsigned char enable){
	mdomain_cap_capmain_boundaryaddr_ctrl_RBUS mdomain_cap_capmain_boundaryaddr_ctrl_reg;
	mdomain_disp_dispmain_boundaryaddr_ctrl_RBUS mdomain_disp_dispmain_boundaryaddr_ctrl_reg;
	mdomain_cap_capsub_boundaryaddr_ctrl_RBUS mdomain_cap_capsub_boundaryaddr_ctrl_reg;
	mdomain_disp_dispsub_boundaryaddr_ctrl_RBUS mdomain_disp_dispsub_boundaryaddr_ctrl_reg;
	if(display == SLR_MAIN_DISPLAY){
		//main cap limit mode
		mdomain_cap_capmain_boundaryaddr_ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr_CTRL_reg);
		mdomain_cap_capmain_boundaryaddr_ctrl_reg.in1_boundary_limit_mode = enable;
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr_CTRL_reg, mdomain_cap_capmain_boundaryaddr_ctrl_reg.regValue);

		//main disp limit mode
		mdomain_disp_dispmain_boundaryaddr_ctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DispMain_BoundaryAddr_CTRL_reg);
		mdomain_disp_dispmain_boundaryaddr_ctrl_reg.main_disp_boundary_limit_mode = enable;
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr_CTRL_reg, mdomain_disp_dispmain_boundaryaddr_ctrl_reg.regValue);
	}
	else if(display == SLR_SUB_DISPLAY){
		//sub cap limit mode
		mdomain_cap_capsub_boundaryaddr_ctrl_reg.regValue = IoReg_Read32(MDOMAIN_CAP_CapSub_BoundaryAddr_CTRL_reg);
		mdomain_cap_capsub_boundaryaddr_ctrl_reg.in2_boundary_limit_mode = enable;
		IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr_CTRL_reg, mdomain_cap_capsub_boundaryaddr_ctrl_reg.regValue);

		//sub disp limit mode
		mdomain_disp_dispsub_boundaryaddr_ctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DispSub_BoundaryAddr_CTRL_reg);
		mdomain_disp_dispsub_boundaryaddr_ctrl_reg.sub_disp_boundary_limit_mode = enable;
		IoReg_Write32(MDOMAIN_DISP_DispSub_BoundaryAddr_CTRL_reg, mdomain_disp_dispsub_boundaryaddr_ctrl_reg.regValue);
	}
}

void drv_memory_cap_limit_boundary_setting(unsigned char display, unsigned int buffer_size){
	if(display == SLR_MAIN_DISPLAY){
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr_BLK0_reg, (MemTag[MEMIDX_MAIN].StartAddr & 0x7ffffff0));
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_BLK0_reg, (MemTag[MEMIDX_MAIN].StartAddr+buffer_size) & 0x7ffffff0);
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr_BLK1_reg, (MemTag[MEMIDX_MAIN_SEC].StartAddr & 0x7ffffff0));
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_BLK1_reg, (MemTag[MEMIDX_MAIN_SEC].StartAddr+buffer_size) & 0x7ffffff0);
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr_BLK2_reg, (MemTag[MEMIDX_MAIN_THIRD].StartAddr & 0x7ffffff0));
		IoReg_Write32(MDOMAIN_CAP_CapMain_BoundaryAddr2_BLK2_reg, (MemTag[MEMIDX_MAIN_THIRD].StartAddr+buffer_size) & 0x7ffffff0);
	}
	else if(display == SLR_SUB_DISPLAY){
		IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK0_reg, (MemTag[MEMIDX_SUB].StartAddr & 0x7ffffff0));
		IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK0_reg, (MemTag[MEMIDX_SUB].StartAddr+buffer_size) & 0x7ffffff0);
		IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK1_reg, (MemTag[MEMIDX_SUB_SEC].StartAddr & 0x7ffffff0));
		IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK1_reg, (MemTag[MEMIDX_SUB_SEC].StartAddr+buffer_size) & 0x7ffffff0);
		IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK2_reg, (MemTag[MEMIDX_SUB_THIRD].StartAddr & 0x7ffffff0));
		IoReg_Write32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK2_reg, (MemTag[MEMIDX_SUB_THIRD].StartAddr+buffer_size) & 0x7ffffff0);
	}
}

void drv_memory_disp_limit_boundary_setting(unsigned char display){
	if(display == SLR_MAIN_DISPLAY){
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr_BLK0_reg, (IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr_BLK0_reg)& 0x7ffffff0));
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr2_BLK0_reg, (IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr2_BLK0_reg)& 0x7ffffff0));
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr_BLK1_reg, (IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr_BLK1_reg)& 0x7ffffff0));
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr2_BLK1_reg, (IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr2_BLK1_reg)& 0x7ffffff0));
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr_BLK2_reg, (IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr_BLK2_reg)& 0x7ffffff0));
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr2_BLK2_reg, (IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr2_BLK2_reg)& 0x7ffffff0));
	}
	else if(display == SLR_SUB_DISPLAY){
		IoReg_Write32(MDOMAIN_DISP_DispSub_BoundaryAddr_BLK0_reg, (IoReg_Read32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK0_reg)& 0x7ffffff0));
		IoReg_Write32(MDOMAIN_DISP_DispSub_BoundaryAddr2_BLK0_reg, (IoReg_Read32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK0_reg)& 0x7ffffff0));
		IoReg_Write32(MDOMAIN_DISP_DispSub_BoundaryAddr_BLK1_reg, (IoReg_Read32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK1_reg)& 0x7ffffff0));
		IoReg_Write32(MDOMAIN_DISP_DispSub_BoundaryAddr2_BLK1_reg, (IoReg_Read32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK1_reg)& 0x7ffffff0));
		IoReg_Write32(MDOMAIN_DISP_DispSub_BoundaryAddr_BLK2_reg, (IoReg_Read32(MDOMAIN_CAP_CapSub_BoundaryAddr_BLK2_reg)& 0x7ffffff0));
		IoReg_Write32(MDOMAIN_DISP_DispSub_BoundaryAddr2_BLK2_reg, (IoReg_Read32(MDOMAIN_CAP_CapSub_BoundaryAddr2_BLK2_reg)& 0x7ffffff0));
	}
}


void Scaler_memory_setting_through_CMA(void)
{
	unsigned int buffer_size = 0;
	unsigned int nStartAddr = 0;
	if (drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD) == _DUMMYADDR) {
		mdomain_cap_cap_reg_doublbuffer_RBUS cap_reg_doublbuffer_reg;
		mdomain_disp_ddr_mainsubctrl_RBUS ddr_mainsubctrl_reg;
		unsigned int timeout = 0x3ffff;
		if(dvrif_memory_compression_get_enable(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL))==_ENABLE) {
			buffer_size = MDOMAIN_MAIN_BUFFER_SIZE_10BITS;
		} else {
			buffer_size = MDOMAIN_MAIN_BUFFER_SIZE_16BITS;
		}
		//thrid memory
		nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_THIRD, buffer_size, MEMALIGN_ANY);
		MemTag[MEMIDX_MAIN_THIRD].StartAddr = nStartAddr;
		IoReg_Write32(MDOMAIN_CAP_DDR_In1_3rdAddr_reg, nStartAddr & 0x7ffffff0);
		//second memory
		nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN_SEC, buffer_size, MEMALIGN_ANY);
		MemTag[MEMIDX_MAIN_SEC].StartAddr = nStartAddr;
		IoReg_Write32(MDOMAIN_CAP_DDR_In1_2ndAddr_reg, nStartAddr & 0x7ffffff0);
		//first memory
		nStartAddr = drvif_memory_alloc_block(MEMIDX_MAIN, buffer_size, MEMALIGN_ANY);
		MemTag[MEMIDX_MAIN].StartAddr = nStartAddr;
		IoReg_Write32(MDOMAIN_CAP_DDR_In1Addr_reg, nStartAddr & 0x7ffffff0);

		//apply M-cap double buffer

		if(scaler_vsc_get_vr360_block_mdomain_doublebuffer()==FALSE){
			cap_reg_doublbuffer_reg.regValue = IoReg_Read32(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
			cap_reg_doublbuffer_reg.cap1_db_apply = 1;
			IoReg_Write32(MDOMAIN_CAP_cap_reg_doublbuffer_reg,cap_reg_doublbuffer_reg.regValue);
			while(timeout) {
				cap_reg_doublbuffer_reg.regValue = IoReg_Read32(MDOMAIN_CAP_cap_reg_doublbuffer_reg);
				if ((cap_reg_doublbuffer_reg.cap1_db_en == 0) || (cap_reg_doublbuffer_reg.cap1_db_apply==0)) {
					break;
				}
				timeout--;
			}
		}
		timeout = 0x3ffff;
		nStartAddr = drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main3rdAddr_reg, nStartAddr&0x7ffffff0);
		nStartAddr = drvif_memory_get_block_addr(MEMIDX_MAIN_SEC);
		IoReg_Write32(MDOMAIN_DISP_DDR_Main2ndAddr_reg, nStartAddr&0x7ffffff0);
		nStartAddr = drvif_memory_get_block_addr(MEMIDX_MAIN);
		IoReg_Write32(MDOMAIN_DISP_DDR_MainAddr_reg, nStartAddr&0x7ffffff0);
		nStartAddr = IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr1_reg);
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr1_reg, nStartAddr&0x7ffffff0);
		nStartAddr = IoReg_Read32(MDOMAIN_CAP_CapMain_BoundaryAddr2_reg);
		IoReg_Write32(MDOMAIN_DISP_DispMain_BoundaryAddr2_reg, nStartAddr&0x7ffffff0);
		//apply M-dis double buffer
		if(scaler_vsc_get_vr360_block_mdomain_doublebuffer()==FALSE){
			ddr_mainsubctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
			ddr_mainsubctrl_reg.disp1_double_apply = 1;
			IoReg_Write32(MDOMAIN_DISP_DDR_MainSubCtrl_reg,ddr_mainsubctrl_reg.regValue);
			while(timeout) {
				ddr_mainsubctrl_reg.regValue = IoReg_Read32(MDOMAIN_DISP_DDR_MainSubCtrl_reg);
				if ((ddr_mainsubctrl_reg.disp1_double_enable== 0) || (ddr_mainsubctrl_reg.disp1_double_apply==0)) {
					break;
				}
				timeout--;
			}
		}
	}
}

void drv_memory_set_vdec_direct_low_latency_mode(unsigned char enable){
	enable_M_LowLatencyMode_Vdec_direct = enable;
}

unsigned char drv_memory_get_vdec_direct_low_latency_mode(void){
	return enable_M_LowLatencyMode_Vdec_direct;
}

bool is_2k_memory_buffer_source(SCALER_DISP_CHANNEL display)
{//return ture: means m domain use 2k size.   return False: means m domain use 4k size. So need to borrow from i3ddma size
	VSC_INPUT_TYPE_T srctype;

	srctype = Get_DisplayMode_Src(display);

	if(get_platform_model() == PLATFORM_MODEL_2K)
		return TRUE;

	if (display == SLR_MAIN_DISPLAY) {
		#if defined (CONFIG_ENABLE_HDR10_HDMI_AUTO_DETECT) || defined(CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT)
			if((srctype == VSC_INPUTSRC_HDMI) &&( (Scaler_Hdmi_DM_Get_Status() != SLR_HDMI_DM_CONNECT)
				|| (get_force_hdmi_hdr_flow_enable(SLR_MAIN_DISPLAY) == TRUE)))// not hdr source can borrow
		#else
			if((srctype == VSC_INPUTSRC_HDMI) && ((Scaler_DispGetInputInfo(SLR_INPUT_THROUGH_I3DDMA) == false)
				|| (get_force_hdmi_hdr_flow_enable(SLR_MAIN_DISPLAY) == TRUE)))// not run hdr source can borrow
		#endif
				return FALSE;
	}

	return TRUE;
}

unsigned int calc_compress_memory_size(SCALER_DISP_CHANNEL display, MemCompression_Bits eCompressBit)
{
    unsigned int ulLineCompressSize = 0;
    unsigned int ulCompressMemorySize = 0;
    unsigned int ulCapWid = M_DOMAIN_WIDTH_CARVEOUT;
    unsigned int ulCapLen = M_DOMAIN_LEN_CARVEOUT;
    if(get_platform_model() == PLATFORM_MODEL_2K)
    {
        ulCapWid = M_DOMAIN_WIDTH_CARVEOUT;
        ulCapLen = M_DOMAIN_LEN_CARVEOUT;
    }
    else if(get_platform_model() == PLATFORM_MODEL_4K)
    {
        if(is_2k_memory_buffer_source(display))
        {
            ulCapWid = M_DOMAIN_WIDTH_CARVEOUT;
            ulCapLen = M_DOMAIN_LEN_CARVEOUT;
        }
        else
        {
            ulCapWid = M_DOMAIN_WIDTH_MAX_SIZE;
            ulCapLen = M_DOMAIN_LEN_MAX_SIZE;
        }
    }

    if((ulCapWid % 32) != 0)
    {
        ulCapWid = ulCapWid + (32 - (ulCapWid % 32));
    }

    ulLineCompressSize = ulCapWid * eCompressBit + 256;

    ulLineCompressSize = drvif_memory_get_data_align(ulLineCompressSize, 64);

    ulCompressMemorySize = ulCapLen * (ulLineCompressSize / 8);

    rtd_pr_scaler_memory_info("[%s %d]ulCapWid:%d, ulCapLen:%d, eCompressBit:%d, ulCompressMemorySize:0x%x\n", __FUNCTION__, __LINE__, ulCapWid, ulCapLen, eCompressBit, ulCompressMemorySize);

    return ulCompressMemorySize;
}

/*======================== End of File =======================================*/
/**
 *
 * @}
 */

