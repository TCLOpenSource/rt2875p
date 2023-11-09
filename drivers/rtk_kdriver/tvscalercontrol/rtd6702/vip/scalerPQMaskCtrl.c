/*==========================================================================
    * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/

/**
 * @file
 * 	This file is for PQMask flow control related functions.
 *
 * @author 	Wei Yuan Hsu
 * @date 	20210315
 * @version 0.1
 */

/****************************** Header files ******************************/
// platform
#include <linux/version.h>
#include <rtd_log/rtd_module_log.h>
#include <mach/rtk_platform.h>
#include <linux/string.h>
#include <asm/cacheflush.h>
#include <linux/kernel.h>
// library
#include <scaler/vipRPCCommon.h>
#include "vgip_isr/scalerVIP.h"
#include "rtk_vip_logger.h"
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <rtk_ai.h>
#include <vgip_isr/scalerAI.h>
#include <tvscalercontrol/vip/scalerPQMaskCtrl.h>
#include <tvscalercontrol/vip/scalerPQMaskColorLib.h>
#include <tvscalercontrol/vip/vip_ring_buffer.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/pqmask.h>
#include <gal/rtd6702/rtk_kadp_se.h>
#include <gal/rtk_se_export.h>
// rbus
#include <rbus/pcid_reg.h>
#include <rbus/se_reg.h>

/****************************** Configurations ******************************/
static unsigned int PQMaskPrintCount = 0;
#ifdef VIPprintf
#undef VIPprintf
#define VIPprintf _VIPprintfPrivate
#endif
#define PQMASKPrintf_Idomain(fmt, args...)	VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VIP_PQMask_Idomain_DEBUG, PQMaskPrintCount, fmt, ##args)
#define PQMASKPrintf_Ddomain(fmt, args...)	VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VIP_PQMask_Ddomain_DEBUG, PQMaskPrintCount, fmt, ##args)
#define PQMASKPrintf_SEtask(fmt, args...)	VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VIP_PQMask_SEtask_DEBUG, PQMaskPrintCount, fmt, ##args)
#define PQMASKPrintf_RingBuf(fmt, args...)	VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VIP_PQMask_RingBuf_DEBUG, PQMaskPrintCount, fmt, ##args)
#define PQMASKPrintf_Dump(fmt, args...)		VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VIP_PQMask_Dump_DEBUG, PQMaskPrintCount, fmt, ##args)
#define PQMASKPrintf_6(fmt, args...)		VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VIP_PQMask_6_DEBUG, PQMaskPrintCount, fmt, ##args)
#define PQMASKPrintf_7(fmt, args...)		VIPprintf(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VIP_PQMask_7_DEBUG, PQMaskPrintCount, fmt, ##args)

#define PQMASK_FUNC_DUMPBUF 1
#if PQMASK_FUNC_DUMPBUF
static char DumpFileName[256];
static unsigned int dumpCounter = 0;
#define TAG_DUMP_NAME "PQMASK_DMP"
#endif
#define PQMASK_SE_WARN_CNT 500
#define PQMASK_SE_DONE_CNT 10000
#define TAG_NAME "VPQMASK"
#define TAG_AIPQ_ERR "AIPQ_ERR" // for grep log keyword
#define ALIGN_SIZE (1<<12)

/****************************** Extern parameters ******************************/
extern RTK_AI_PQ_mode aipq_mode;

/****************************** Gobal module parameters ******************************/
struct work_struct PQMaskApplyWork;

static PQMASK_COLOR_MGR_T mColorMgr;
static PQMASK_PERF_T PQMaskPerfMgr[PQMASK_PERF_NUM];
static PQMASK_ONLINE_MONITOR_T PQMaskMonitorMgr = {
	// Control
	{
		1,	// bEnable : enable online monitor
		1,	// bLogEnable : enable print error log
		15,	// LogPeriod : unit=sec
		10,	// Depth_TargetFps : lowest tolerance input depth information FPS
		64,	// Depth_MeasureRstFrm
		40,	// SEProc_TargetTime : largest se process time, unit=ms
	},
	// Status
	{
		0,	// LogCounter
		0,	// Depth_FpsError
		0,	// SEProc_TimeError
		0,	// Depth_MeasureStartTime
		0,	// Depth_MeauserEndTime
		0,	// Depth_Counter
		0,	// SECurProcStart
		0,	// SECurEndStart
	},
};
static PQMASK_GLB_MGR_T PQMaskGlbMgr = {
	// global control
	{
		1,	// bEnableSeProc : se proc enable
		30,	// LimitFPS : PQMask flow limit fps
		5,	// ErrRecoverRstTimes : error recovery reset time
		0,	// SemCapImgLimitFPS : semantic image capture limit fps
		0,	// bDepNewInfo : depth new info
		0,	// bSemNewInfo : sem new info
		0,	// bIsDepthReady : depth model start running
		0,	// bIsSemReady : semantic model start running
	},
	// status
	{
		// se status
		{
			0,	// IsRunning
			0,	// ProcTsp
			0,	// StartTime
		},
		0,	// bDepNeedUptHist : depth need update hist
		0,	// ErrRecoverTimes : error recovery time
		PQMASK_STAT_UNKOWN,	// PQMaskStatus : PQMask internal status control
	},
};
static PQMASK_BLK_MODULE_CONFIG_T PQMaskBlkModuleCfg[PQMASK_BLKMOD_NUM] = {
	// SPNR
	{
		// scale
		{
			// hor
			{ (1<<5)+(1<<6), (1<<20) },
			// ver
			{ (1<<5)+(1<<6), (1<<20) },
		},
		// io size
		{
			// input
			{ 0, PQMASK_PROC_WIDTH, 0, PQMASK_PROC_HEIGHT },
			// output
			{ 0, 0, 0, 0 },
		}
	},
	// SHARPNESS
	{
		// scale
		{
			// hor
			{ (1<<5)+(1<<6), (1<<20) },
			// ver
			{ (1<<5)+(1<<6), (1<<20) },
		},
		// io size
		{
			// input
			{ 0, PQMASK_PROC_WIDTH, 0, PQMASK_PROC_HEIGHT },
			// output
			{ 0, 0, 0, 0 },
		}
	},
	// ICM
	{
		// scale
		{
			// hor
			{ (1<<5)+(1<<6), (1<<20) },
			// ver
			{ (1<<5)+(1<<6), (1<<20) },
		},
		// io size
		{
			// input
			{ 0, PQMASK_PROC_WIDTH, 0, PQMASK_PROC_HEIGHT },
			// output
			{ 0, 0, 0, 0 },
		}
	},
};
static PQMASK_BUFINFO_T PQMaskBufMgr = {
	0,
	// Module weighting dma
	{
		// NR
		{
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
		},
		// DECONT
		{
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
		},
		// EDGE
		{
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
		},
		// TEXTURE
		{
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
		},
		// HUE
		{
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
		},
		// SAT
		{
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
		},
		// INT
		{
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
			{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x80 }, { 0, 0, 0, 0 }, },
		},
	},
	// se processing dma
	{
		// source semantic info
		{ { AI_SEMANTIC_OUTPUT_WIDTH, AI_SEMANTIC_OUTPUT_HEIGHT, sizeof(unsigned char), 0x0 }, { 0, 0, 0, 0 } },
		// source depth info
		{ { AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, sizeof(unsigned char), 0x0 }, { 0, 0, 0, 0 } },
		// source depth info resize
		{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char), 0x0 }, { 0, 0, 0, 0 } },
		// PQMASK_SE_PROCBUF_00, this buffer used to get the semantic weight
		{
			{
				(PQMASK_PROC_WIDTH>AI_SEMANTIC_OUTPUT_WIDTH)? PQMASK_PROC_WIDTH : AI_SEMANTIC_OUTPUT_WIDTH,
				(PQMASK_PROC_HEIGHT>AI_SEMANTIC_OUTPUT_HEIGHT)? PQMASK_PROC_HEIGHT : AI_SEMANTIC_OUTPUT_HEIGHT,
				sizeof(unsigned char)*3, 0x80
			},
			{ 0, 0, 0, 0 }
		},
		// PQMASK_SE_PROCBUF_01
		{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char)*3, 0x80 }, { 0, 0, 0, 0 } },
		// PQMASK_SE_PROCBUF_02
		{ { PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT, sizeof(unsigned char)*3, 0x80 }, { 0, 0, 0, 0 } },
	},
	// DepthSrcImg with NV12 format for depth histogram
	{ { AI_DEPTHMAP_WIDTH, (AI_DEPTHMAP_HEIGHT*3)>>1, sizeof(unsigned char), 0x0 }, { 0, 0, 0, 0 } },
};
static VIP_EASY_RING_BUF_T PQMaskRingBuf[PQMASK_MODULE_NUM]; // HW module ring buffer
char PQMaskModuleName[PQMASK_MODULE_NUM][PQMASK_STRING_LEN] = {
	/* 0 */ "NR",
	/* 1 */ "DECON",
	/* 2 */ "EDGE",
	/* 3 */ "TEXT",
	/* 4 */ "HUE",
	/* 5 */ "SAT",
	/* 6 */ "INT",
};

char PQMaskBlockModuleName[PQMASK_BLKMOD_NUM][PQMASK_STRING_LEN] = {
	/* 0 */ "SPNR",
	/* 1 */ "SHARP",
	/* 2 */ "ICM",
};

char PQMaskSEBufName[PQMASK_SE_BUF_NUM][PQMASK_STRING_LEN] = {
	/* 0 */ "SrcSem",
	/* 1 */ "SrcDep",
	/* 2 */ "SrcDepReSz",
	/* 3 */ "SeProc00",
	/* 4 */ "SeProc01",
	/* 5 */ "SeProc02",
};

char PQMaskStatusStr[PQMASK_STAT_MAX][PQMASK_STRING_LEN] = {
	/* 0 */ "UNKNOWN",
	/* 1 */ "STANDBY",
	/* 2 */ "READY",
	/* 3 */ "PREPARE",
	/* 4 */ "SE_PROC",
	/* 5 */ "ERROR",
};

unsigned int SEProcGrpMapping[MODULE_GRP_MAX][3] = {
	/* 0 */	{ PQMASK_MODULE_NR,	PQMASK_MODULE_DECONT, PQMASK_MODULE_EDGE },
	/* 1 */	{ PQMASK_MODULE_TEXTURE, PQMASK_MODULE_HUE, PQMASK_MODULE_SAT },
	/* 2 */	{ PQMASK_MODULE_INT, PQMASK_MODULE_NONE, PQMASK_MODULE_NONE },
};

unsigned int ModuleDmaIdx[PQMASK_MODULE_NUM][DMA_IDX_MAX] = {
	// 			DMA_IDX_CUR,	DMA_IDX_NXT
	/* 0 */ { 	0,				0 },
	/* 1 */ { 	0,				0 },
	/* 2 */ { 	0,				0 },
	/* 3 */ { 	0,				0 },
	/* 4 */ { 	0,				0 },
	/* 5 */ { 	0,				0 },
	/* 6 */ { 	0,				0 },
};

const unsigned int IDomainModuleGrp[2] = {
	/* 0 */ PQMASK_MODULE_NR,
	/* 1 */ PQMASK_MODULE_DECONT,
};

const unsigned int DDomainModuleGrp[5] = {
	/* 0 */ PQMASK_MODULE_EDGE,
	/* 1 */ PQMASK_MODULE_TEXTURE,
	/* 2 */ PQMASK_MODULE_HUE,
	/* 3 */ PQMASK_MODULE_SAT,
	/* 4 */ PQMASK_MODULE_INT,
};

// for se processing
static KGAL_PALETTE_SURFACE_INFO_T SeColorLut;
static KGAL_SURFACE_INFO_T SeSrc1_Surface[3];
static KGAL_SURFACE_INFO_T SeSrc2_Surface[3];
static KGAL_SURFACE_INFO_T SeDst_Surface[3];

#if PQMASK_FUNC_DUMPBUF // debug dump

#if IS_ENABLED(CONFIG_RTK_USBDUMP_ENABLE)
#include <linux/fs.h>
extern int rtk_get_usb_path(char *buf, int buf_len);

static struct file* file_open(const char* path, int flags, int rights) {
	struct file* filp = NULL;
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;
	#endif
	//int err = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#endif
	filp = filp_open(path, flags, rights);
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldfs);
	#endif
	if(IS_ERR(filp)) {
		//err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

static void file_close(struct file* file) {
	filp_close(file, NULL);
}

static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;
	#endif
	int ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#endif
	ret = kernel_write(file, data, size, &offset);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldfs);
#endif
	return ret;
}

static int file_sync(struct file* file) {
	vfs_fsync(file, 0);
	return 0;
}
#endif

static int PQMaskDumpData2File(char* tmpname, unsigned int * vir_y, unsigned int * vir_c, unsigned int y_size, unsigned int c_size)
{
#if IS_ENABLED(CONFIG_RTK_USBDUMP_ENABLE)
	struct file* filp = NULL;
	unsigned long outfileOffset = 0;

	char usbPath[256];
	char filename[256];

	if( vir_y == 0 && vir_c == 0 ) {
		PQMASKPrintf_Dump("[%s] dump fail, address zeros\n", TAG_DUMP_NAME);
		return FALSE;
	}

	if( rtk_get_usb_path(usbPath, 256) != 0) {
		PQMASKPrintf_Dump("[%s] get usb path fail use /tmp/usb/sda/sda1 as default\n", TAG_DUMP_NAME);
		sprintf(filename, "%s/%s.raw", "/tmp/usb/sda/sda1", tmpname);
	} else {
		sprintf(filename, "%s/%s.raw", usbPath, tmpname);
	}

	PQMASKPrintf_Dump("[%s] try to dump file %s\n", TAG_DUMP_NAME, filename);

	if (vir_y != 0) {
		filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0);
		if (filp == NULL) {
			PQMASKPrintf_Dump("[%s] (%d) open fail 1\n", TAG_DUMP_NAME, __LINE__);
			return FALSE;
		}

		file_write(filp, outfileOffset, (unsigned char*)vir_y, y_size);
		file_sync(filp);
		file_close(filp);
	} else {
		PQMASKPrintf_Dump("[%s] y addr zeros, skip dump y\n", TAG_DUMP_NAME);
	}

	if (vir_c != 0) {
		filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0);
		if (filp == NULL) {
			PQMASKPrintf_Dump("[%s] (%d) open fail 2\n", TAG_DUMP_NAME, __LINE__);
			return FALSE;
		}

		file_write(filp, outfileOffset, (unsigned char*)vir_c, c_size);
		file_sync(filp);
		file_close(filp);

	} else {
		PQMASKPrintf_Dump("[%s] c addr zeros, skip dump c\n", TAG_DUMP_NAME);
	}

	return TRUE;
#else
	return FALSE;
#endif
}

static void PQMaskDumpSEBufCheck(unsigned int bufSel, char *fileName){

	char Name[256];
	unsigned int *pVir_addr_y;
	unsigned int *pVir_addr_c;
	unsigned int y_size = 0;
	unsigned int c_size = 0;

	if( bufSel >= PQMASK_SE_BUF_NUM ) {
		PQMASKPrintf_Dump("[%s] fail to dump %s , buf sel = %d error\n", TAG_DUMP_NAME, fileName, bufSel);
		return;
	}

	// 0xB802EF08 bit 0 : dump enable
	if( (IoReg_Read32(PCID_PCID_CTRL_3_reg)&(_BIT0)) != 0 ) {

		sprintf(Name, "%s_%d", fileName, dumpCounter);

		pVir_addr_y = (unsigned int *)PQMaskBufMgr.SeBuffer[bufSel].AddrInfo.VirtAddr_nonCached;
		y_size = PQMaskBufMgr.SeBuffer[bufSel].AddrInfo.Size;
		pVir_addr_c = 0; // no color plane
		c_size = 0; // no color plane

		PQMaskDumpData2File(Name, pVir_addr_y, pVir_addr_c, y_size, c_size);
	}

}

static void PQMaskDumpNV12ImgBufCheck(char *fileName, unsigned int *pVir_addr_y, unsigned int y_size) {

	char Name[256];
	unsigned int *pVir_addr_c;
	unsigned int c_size = 0;

	// 0xB802EF08 bit 0 : dump enable
	if( (IoReg_Read32(PCID_PCID_CTRL_3_reg)&(_BIT0)) != 0 ) {

		sprintf(Name, "%s_%d", fileName, dumpCounter);

		pVir_addr_c = 0; // no color plane
		c_size = 0; // no color plane

		PQMaskDumpData2File(Name, pVir_addr_y, pVir_addr_c, y_size, c_size);
	}
}

// for rtice command to dump the buffer data
void PQMaskDumpModuleBufCheck(unsigned int bufSel, unsigned int Num, char *fileName){

	char Name[256];
	unsigned int *pVir_addr_y;
	unsigned int *pVir_addr_c;
	unsigned int y_size = 0;
	unsigned int c_size = 0;

	if( bufSel >= PQMASK_MODULE_NUM ) {
		PQMASKPrintf_Dump("[%s] fail to dump %s , buf sel = %d error\n", TAG_DUMP_NAME, fileName, bufSel);
		return;
	}
	if( Num >= PQMASK_MODULE_DMA_NUM ) {
		PQMASKPrintf_Dump("[%s] fail to dump %s , num = %d error\n", TAG_DUMP_NAME, fileName, Num);
		return;
	}

	sprintf(Name, "%s_%s", fileName, PQMaskModuleName[bufSel]);

	pVir_addr_y = (unsigned int *)PQMaskBufMgr.ModuleDma[bufSel][Num].AddrInfo.VirtAddr_nonCached;
	y_size = PQMaskBufMgr.ModuleDma[bufSel][Num].AddrInfo.Size;
	pVir_addr_c = 0; // no color plane
	c_size = 0; // no color plane

	PQMaskDumpData2File(Name, pVir_addr_y, pVir_addr_c, y_size, c_size);
}

#endif

static unsigned char OSDLabelColor[PQMASK_LABEL_NUM][3] = {
	//					R		G		B
	/*basic*/		{	128,	128,	128 },
	/*sky*/			{	128,	196,	255 },
	/*people*/		{	128,	128,	128 },
	/*sea*/			{	  0,	  0,	255 },
	/*plant*/		{	  0,	255,	  0 },
	/*arti-text*/	{	255,	255,	  0 },
};
/****************************** Static Functions ******************************/
/**
 * @brief
 * wait for se processing done
 * @return int
 * indicate the se finish process or not
 */
static int scalerPQMask_WaitSEDone(void) {
	int SEProcCount = 0;
	int Ret = true;
	while((IoReg_Read32(SE_IDLE_0_reg)&(0x01)) != 0x01) {
		if( SEProcCount >= PQMASK_SE_DONE_CNT ) {
			Ret = false;
			break;
		}
		usleep_range(500, 500);
		SEProcCount++;
	}
	if( SEProcCount >= PQMASK_SE_WARN_CNT ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] SEProcCount = %d\n", __FUNCTION__, SEProcCount);
	}
	return Ret;
}

/**
 * @brief
 * set the DDR buffer to a specific value, operation on ddr should only use cached address
 * @param VirtAddr cached address for specific memory
 * @param PhysAddr physical address for specific memory
 * @param Val set value
 * @param SetSize set size
 */
static inline void scalerPQMask_Memset(void *VirtAddr, void *PhysAddr, unsigned char Val, unsigned int SetSize) {
	memset(VirtAddr, Val, SetSize);
	dmac_inv_range(VirtAddr, VirtAddr+SetSize);
	if( PhysAddr != 0 ) {
		outer_inv_range(PhysAddr, ((unsigned long)PhysAddr)+SetSize);
	}
}

/**
 * @brief
 * copy DDR buffer, operation on ddr should only use cached address
 * @param DstVirtAddr destination cached address for specific memory
 * @param DstPhysAddr destination physical address for specific memory
 * @param DstIsMem destination address is ddr or not
 * @param SrcVirtAddr source cached address for specific memory
 * @param SrcPhysAddr source physical address for specific memory
 * @param SrcIsMem source address is ddr or not
 * @param CopySize copy size
 */
static inline void scalerPQMask_Memcpy(
	void *DstVirtAddr, void *DstPhysAddr, bool DstIsMem,
	void *SrcVirtAddr, void *SrcPhysAddr, bool SrcIsMem,
	unsigned int CopySize) {

	// suggested by DIC/DV
	if( SrcIsMem != 0 ) {
		dmac_inv_range(SrcVirtAddr, SrcVirtAddr+CopySize);
		if( SrcPhysAddr != 0 ) {
			outer_inv_range(SrcPhysAddr, ((unsigned long)SrcPhysAddr)+CopySize);
		}
	}
	memcpy( DstVirtAddr, SrcVirtAddr, CopySize );
	if( DstIsMem != 0 ) {
		dmac_inv_range(DstVirtAddr, DstVirtAddr+CopySize);
		if( DstPhysAddr != 0 ) {
			outer_inv_range(DstPhysAddr, ((unsigned long)DstPhysAddr)+CopySize);
		}
	}
}

/**
 * @brief
 * pqmask se task to do se processing
 * @param work
 */
static void scalerPQMask_SETask(struct work_struct *work) {

	int i=0;
	int SeProcStatus = true;
	int ProcGrp = 0;
	// output buffer for every step, default set as error value
	unsigned char OutBuf_DepResize = PQMASK_SE_BUF_NUM;
	unsigned char OutBuf_Sem2Wei = PQMASK_SE_BUF_NUM;
	unsigned char OutBuf_Dep2Wei = PQMASK_SE_BUF_NUM;
	unsigned char OutBuf_SemWeiResize = PQMASK_SE_BUF_NUM;
	unsigned char OutBuf_SemDepFusion = PQMASK_SE_BUF_NUM;
	unsigned char OutBuf_SpatialFlt = PQMASK_SE_BUF_NUM;
	unsigned char OutBuf_Tnr = PQMASK_SE_BUF_NUM;
	// SNR ping-pong buffer
	unsigned char SpatialFltBuf_0 = PQMASK_SE_BUF_NUM;
	unsigned char SpatialFltBuf_1 = PQMASK_SE_BUF_NUM;
	// TNR parameters
	unsigned char TNRBuf_Dummy = PQMASK_SE_BUF_NUM;
	int RingBufWritePtr = 0;
	int RingBufReadPtr = 0;
	unsigned char TNRModuleSel = 0;
	unsigned char TNRFusion_Pre = 0;
	unsigned char TNRFusion_Cur = 0;
	const unsigned int SingleWeightPlaneSize = PQMASK_PROC_WIDTH*PQMASK_PROC_HEIGHT*sizeof(unsigned char);

	// Local param, if the value could be directly change outside the se procedure, need to update at the beginning
	PQMASK_COLOR_MGR_T *pColorMgr = (PQMASK_COLOR_MGR_T *)scalerPQMaskColor_GetStruct(PQMASK_COLORSTRUCT_MGR);
	memcpy(&mColorMgr, pColorMgr, sizeof(PQMASK_COLOR_MGR_T));

	if( PQMaskGlbMgr.Ctrl.bEnableSeProc != 0 ) {

		if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 )
			PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskStart = drvif_Get_90k_Lo_clk();
		else
			PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].RepeatSETaskStart = drvif_Get_90k_Lo_clk();

		PQMaskGlbMgr.Status.SeStatus.IsRunning = 1;

		if( PQMaskGlbMgr.Status.PQMaskStatus == PQMASK_STAT_SE_PROC ) {

			// depth related histogram
			if( PQMaskGlbMgr.Status.bDepNeedUptHist != 0 ) {
				pColorMgr->DepthHist.Info.width = AI_DEPTHMAP_WIDTH;
				pColorMgr->DepthHist.Info.height = AI_DEPTHMAP_HEIGHT;
				scalerPQMaskColor_DepthHistogram((void *)PQMaskBufMgr.SeBuffer[PQMASK_SE_SRCBUF_DEPTH].AddrInfo.VirtAddr_nonCached);
				scalerPQMaskColor_Img2DepthHistogram(
					(void *)PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr_nonCached,
					(void *)PQMaskBufMgr.SeBuffer[PQMASK_SE_SRCBUF_DEPTH].AddrInfo.VirtAddr_nonCached
				);
				PQMaskGlbMgr.Status.bDepNeedUptHist = 0;
			}

#if PQMASK_FUNC_DUMPBUF // debug dump
			PQMaskDumpNV12ImgBufCheck("pqmask_depthSrcImg", (unsigned int *)PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr_nonCached, PQMaskBufMgr.DepthSrcImg.AddrInfo.Size);
#endif

			// semantic related histogram
			if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
				pColorMgr->SemanticHist.Info.width = AI_SEMANTIC_OUTPUT_WIDTH;
				pColorMgr->SemanticHist.Info.height = AI_SEMANTIC_OUTPUT_HEIGHT;
				scalerPQMaskColor_SemanticHistogram((void *)PQMaskBufMgr.SeBuffer[PQMASK_SE_SRCBUF_SEMANTIC].AddrInfo.VirtAddr_nonCached);
				scalerPQMaskColor_Img2SemanticHistogram();
			}

			if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 )
				PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskHistEnd = drvif_Get_90k_Lo_clk();

#if PQMASK_FUNC_DUMPBUF // debug dump
			PQMaskDumpSEBufCheck(PQMASK_SE_SRCBUF_SEMANTIC, "pqmask_semantic");
			PQMaskDumpSEBufCheck(PQMASK_SE_SRCBUF_DEPTH, "pqmask_depth");
#endif

			// scene change
			if( pColorMgr->Status.bSceneChange != 0 ) {
				pColorMgr->Status.SCRstFrm = pColorMgr->Param.SceneChangeRstFrm;
				pColorMgr->Status.bSceneChange = 0;
			}

			// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
			// [SE] resize the input depth information if necessary
			// input  : PQMASK_SE_SRCBUF_DEPTH
			// output : OutBuf_DepResize (PQMASK_SE_SRCBUF_DEPTH or PQMASK_SE_PROCBUF_00)
			// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****

			if( ( AI_DEPTHMAP_WIDTH != PQMASK_PROC_WIDTH || AI_DEPTHMAP_HEIGHT != PQMASK_PROC_HEIGHT ) &&
				( mColorMgr.Param.FusionRatio_Dep != 0 ) ) {

				OutBuf_DepResize = PQMASK_SE_SRCBUF_DEPTH_RESIZE;
				if( true == SeProcStatus ) {
					// initial struct
					memset(&SeSrc1_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
					memset(&SeDst_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
					// fill information
					SeSrc1_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[PQMASK_SE_SRCBUF_DEPTH].AddrInfo.PhysAddr;
					SeSrc1_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_NV12_Y;
					SeSrc1_Surface[0].pitch = AI_DEPTHMAP_WIDTH*8/8; // bytes per line
					SeSrc1_Surface[0].width = AI_DEPTHMAP_WIDTH;
					SeSrc1_Surface[0].height = AI_DEPTHMAP_HEIGHT;
					SeSrc1_Surface[0].bpp = 24;
					SeDst_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[OutBuf_DepResize].AddrInfo.PhysAddr;
					SeDst_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_NV12_Y;
					SeDst_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
					SeDst_Surface[0].width = PQMASK_PROC_WIDTH;
					SeDst_Surface[0].height = PQMASK_PROC_HEIGHT;
					SeDst_Surface[0].bpp = 24;
					// trigger SE
					// KGAL_PQMaskStretch_NV12_Y(&SeSrc1_Surface[0], &SeDst_Surface[0]);
					KGAL_PQMaskStretch(&SeSrc1_Surface[0], &SeDst_Surface[0]);
					SeProcStatus = scalerPQMask_WaitSEDone();
				}

				if( true == SeProcStatus ) {
					PQMASKPrintf_SEtask("[%s] SE depth info resize from %dx%d to %dx%d done, Buf %s to %s\n",
						TAG_NAME,
						AI_DEPTHMAP_WIDTH, AI_DEPTHMAP_HEIGHT, PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT,
						PQMaskSEBufName[PQMASK_SE_SRCBUF_DEPTH], PQMaskSEBufName[OutBuf_DepResize]
					);
				} else {
					rtd_printk(KERN_EMERG, TAG_NAME, "SE error, depth info resize\n");
				}

			} else {

				OutBuf_DepResize = PQMASK_SE_SRCBUF_DEPTH;

			}

#if PQMASK_FUNC_DUMPBUF // debug dump
			sprintf(DumpFileName, "%s", "pqmask_DepResize");
			PQMaskDumpSEBufCheck(OutBuf_DepResize, DumpFileName);
#endif

			if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
				PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].DepthResizeEnd = drvif_Get_90k_Lo_clk();
			}

			for( ProcGrp=0; ProcGrp<MODULE_GRP_MAX; ProcGrp++ ) {

				// reset output buffer
				OutBuf_Sem2Wei = PQMASK_SE_BUF_NUM;
				OutBuf_Dep2Wei = PQMASK_SE_BUF_NUM;
				OutBuf_SemWeiResize = PQMASK_SE_BUF_NUM;
				OutBuf_SemDepFusion = PQMASK_SE_BUF_NUM;
				OutBuf_SpatialFlt = PQMASK_SE_BUF_NUM;

				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
				// [SE] setup depth to weight palette
				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****

				if( mColorMgr.Param.FusionRatio_Dep != 0 ) {

					if( true == SeProcStatus ) {
						// initial struct
						memset(&SeColorLut, 0, sizeof(KGAL_PALETTE_SURFACE_INFO_T));
						// fill information
						scalerPQMaskColor_Depth2Clut(ProcGrp, &SeColorLut.palette);
						SeColorLut.Surface.physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[OutBuf_DepResize].AddrInfo.PhysAddr;
						SeColorLut.Surface.pixelFormat = KGAL_PIXEL_FORMAT_LUT8;
						SeColorLut.Surface.pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
						SeColorLut.Surface.width = PQMASK_PROC_WIDTH;
						SeColorLut.Surface.height = PQMASK_PROC_HEIGHT;
						SeColorLut.Surface.bpp = 8;
						// trigger SE
						KGAL_SetSurfacePalette(&SeColorLut);
						SeProcStatus = scalerPQMask_WaitSEDone();
					}

					if( true == SeProcStatus ) {
						PQMASKPrintf_SEtask("[%s] SE task grp %d done, depth palette\n", TAG_NAME, ProcGrp);
					} else {
						rtd_printk(KERN_EMERG, TAG_NAME, "SE task grp %d error, depth palette\n", ProcGrp);
					}
				}

				if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
					PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskGrpEnd[ProcGrp][0] = drvif_Get_90k_Lo_clk();
				}

				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
				// [SE] depth info to module weight
				// input  : OutBuf_DepResize (PQMASK_SE_SRCBUF_DEPTH or PQMASK_SE_PROCBUF_00)
				// output : OutBuf_Dep2Wei (PQMASK_SE_PROCBUF_01)
				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****

				// decide buffer
				OutBuf_Dep2Wei = PQMASK_SE_PROCBUF_01;

				if( mColorMgr.Param.FusionRatio_Dep != 0 ) {

					if( true == SeProcStatus ) {
						// initial struct
						memset(&SeSrc1_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
						memset(&SeDst_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
						// fill information
						memcpy(&SeSrc1_Surface[0], &SeColorLut.Surface, sizeof(KGAL_SURFACE_INFO_T));
						SeDst_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[OutBuf_Dep2Wei].AddrInfo.PhysAddr;
						SeDst_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
						SeDst_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
						SeDst_Surface[0].width = PQMASK_PROC_WIDTH;
						SeDst_Surface[0].height = PQMASK_PROC_HEIGHT;
						SeDst_Surface[0].bpp = 24;
						// trigger SE
						KGAL_PQMaskLabel2Weight(&SeSrc1_Surface[0], &SeDst_Surface[0]);
						SeProcStatus = scalerPQMask_WaitSEDone();
					}

					if( true == SeProcStatus ) {
						PQMASKPrintf_SEtask("[%s] SE task grp %d done, depth label2weight, Buf %s to %s\n",
							TAG_NAME, ProcGrp, PQMaskSEBufName[OutBuf_DepResize], PQMaskSEBufName[OutBuf_Dep2Wei]
						);
					} else {
						rtd_printk(KERN_EMERG, TAG_NAME, "SE task grp %d error, depth label2weight\n", ProcGrp);
					}

				}

#if PQMASK_FUNC_DUMPBUF // debug dump
				sprintf(DumpFileName, "%s_grp%d", "pqmask_DepWei", ProcGrp);
				PQMaskDumpSEBufCheck(OutBuf_Dep2Wei, DumpFileName);
#endif

				if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
					PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskGrpEnd[ProcGrp][1] = drvif_Get_90k_Lo_clk();
				}

				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
				// [SE] setup semantic to weight palette
				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****

				if( mColorMgr.Param.FusionRatio_Sem != 0 ) {

					// start process
					if( true == SeProcStatus ) {
						// initial struct
						memset(&SeColorLut, 0, sizeof(KGAL_PALETTE_SURFACE_INFO_T));
						// fill information
						scalerPQMaskColor_Label2Clut(ProcGrp, &SeColorLut.palette);
						SeColorLut.Surface.physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[PQMASK_SE_SRCBUF_SEMANTIC].AddrInfo.PhysAddr;
						SeColorLut.Surface.pixelFormat = KGAL_PIXEL_FORMAT_LUT8;
						SeColorLut.Surface.pitch = AI_SEMANTIC_OUTPUT_WIDTH*8/8; // bytes per line
						SeColorLut.Surface.width = AI_SEMANTIC_OUTPUT_WIDTH;
						SeColorLut.Surface.height = AI_SEMANTIC_OUTPUT_HEIGHT;
						SeColorLut.Surface.bpp = 8;
						// trigger SE
						KGAL_SetSurfacePalette(&SeColorLut);
						SeProcStatus = scalerPQMask_WaitSEDone();
					}

					if( true == SeProcStatus ) {
						PQMASKPrintf_SEtask("[%s] SE task grp %d done, semantic palette\n", TAG_NAME, ProcGrp);
					} else {
						rtd_printk(KERN_EMERG, TAG_NAME, "SE task grp %d error, semantic palette\n", ProcGrp);
					}
				}

				if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
					PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskGrpEnd[ProcGrp][2] = drvif_Get_90k_Lo_clk();
				}

				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
				// [SE] semantic label to module weight
				// input  : PQMASK_SE_SRCBUF_SEMANTIC
				// output : OutBuf_Sem2Wei (PQMASK_SE_PROCBUF_00)
				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****

				// decide buffer
				OutBuf_Sem2Wei = PQMASK_SE_PROCBUF_00;

				if( mColorMgr.Param.FusionRatio_Sem != 0 ) {

					if( true == SeProcStatus ) {
						// initial struct
						memset(&SeSrc1_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
						memset(&SeDst_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
						// fill information
						memcpy(&SeSrc1_Surface[0], &SeColorLut.Surface, sizeof(KGAL_SURFACE_INFO_T));
						SeDst_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[OutBuf_Sem2Wei].AddrInfo.PhysAddr;
						SeDst_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
						SeDst_Surface[0].pitch = AI_SEMANTIC_OUTPUT_WIDTH*8/8; // bytes per line
						SeDst_Surface[0].width = AI_SEMANTIC_OUTPUT_WIDTH;
						SeDst_Surface[0].height = AI_SEMANTIC_OUTPUT_HEIGHT;
						SeDst_Surface[0].bpp = 24;
						// trigger SE
						KGAL_PQMaskLabel2Weight(&SeSrc1_Surface[0], &SeDst_Surface[0]);
						SeProcStatus = scalerPQMask_WaitSEDone();
					}

					if( true == SeProcStatus ) {
						PQMASKPrintf_SEtask("[%s] SE task grp %d done, semantic label2weight, Buf %s to %s\n",
							TAG_NAME, ProcGrp, PQMaskSEBufName[PQMASK_SE_SRCBUF_SEMANTIC], PQMaskSEBufName[OutBuf_Sem2Wei]
						);
					} else {
						rtd_printk(KERN_EMERG, TAG_NAME, "SE task grp %d error, semantic label2weight\n", ProcGrp);
					}

				}

#if PQMASK_FUNC_DUMPBUF // debug dump
				sprintf(DumpFileName, "%s_grp%d", "pqmask_SemWei", ProcGrp);
				PQMaskDumpSEBufCheck(OutBuf_Sem2Wei, DumpFileName);
#endif

				if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
					PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskGrpEnd[ProcGrp][3] = drvif_Get_90k_Lo_clk();
				}

				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
				// [SE] resize the semantic weighting information if necessary
				// input  : OutBuf_Sem2Wei (PQMASK_SE_PROCBUF_00)
				// output : OutBuf_DepResize (PQMASK_SE_PROCBUF_00 or PQMASK_SE_PROCBUF_02)
				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****

				if( ( AI_SEMANTIC_OUTPUT_WIDTH != PQMASK_PROC_WIDTH || AI_SEMANTIC_OUTPUT_HEIGHT != PQMASK_PROC_HEIGHT ) &&
					( mColorMgr.Param.FusionRatio_Sem != 0 ) ) {

					OutBuf_SemWeiResize = PQMASK_SE_PROCBUF_02;

					if( true == SeProcStatus ) {
						// initial struct
						memset(&SeSrc1_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
						memset(&SeDst_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
						// fill information
						SeSrc1_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[OutBuf_Sem2Wei].AddrInfo.PhysAddr;
						SeSrc1_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
						SeSrc1_Surface[0].pitch = AI_SEMANTIC_OUTPUT_WIDTH*8/8; // bytes per line
						SeSrc1_Surface[0].width = AI_SEMANTIC_OUTPUT_WIDTH;
						SeSrc1_Surface[0].height = AI_SEMANTIC_OUTPUT_HEIGHT;
						SeSrc1_Surface[0].bpp = 24;
						SeDst_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[OutBuf_SemWeiResize].AddrInfo.PhysAddr;
						SeDst_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
						SeDst_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
						SeDst_Surface[0].width = PQMASK_PROC_WIDTH;
						SeDst_Surface[0].height = PQMASK_PROC_HEIGHT;
						SeDst_Surface[0].bpp = 24;
						// trigger SE
						KGAL_PQMaskStretch(&SeSrc1_Surface[0], &SeDst_Surface[0]);
						SeProcStatus = scalerPQMask_WaitSEDone();
					}

					if( true == SeProcStatus ) {
						PQMASKPrintf_SEtask("[%s] SE task grp %d done, sematic resize from %dx%d to %dx%d, Buf %s to %s\n",
							TAG_NAME, ProcGrp,
							AI_SEMANTIC_OUTPUT_WIDTH, AI_SEMANTIC_OUTPUT_HEIGHT,
							PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT,
							PQMaskSEBufName[OutBuf_Sem2Wei], PQMaskSEBufName[OutBuf_SemWeiResize]
						);
					} else {
						rtd_printk(KERN_EMERG, TAG_NAME, "SE task grp %d error, sematic stretch\n", ProcGrp);
					}

				} else {
					OutBuf_SemWeiResize = OutBuf_Sem2Wei;
				}

#if PQMASK_FUNC_DUMPBUF // debug dump
				sprintf(DumpFileName, "%s_grp%d", "pqmask_SemWeiResize", ProcGrp);
				PQMaskDumpSEBufCheck(OutBuf_SemWeiResize, DumpFileName);
#endif

				if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
					PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskGrpEnd[ProcGrp][4] = drvif_Get_90k_Lo_clk();
				}

				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
				// [SE] blending depth and semantic module weight result
				// input  : OutBuf_Dep2Wei (PQMASK_SE_PROCBUF_01) + OutBuf_SemWeiResize (PQMASK_SE_PROCBUF_00 or PQMASK_SE_PROCBUF_02)
				// output : OutBuf_SemDepFusion ( PQMASK_SE_PROCBUF_00 or PQMASK_SE_PROCBUF_01 or PQMASK_SE_PROCBUF_02 )
				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****

				if( mColorMgr.Param.FusionRatio_Dep == 255 && mColorMgr.Param.FusionRatio_Sem == 0 ) {
					OutBuf_SemDepFusion = OutBuf_Dep2Wei;
					PQMASKPrintf_SEtask("[%s] grp %d, Fusion OutBuf %s\n", TAG_NAME, ProcGrp, PQMaskSEBufName[OutBuf_SemDepFusion]);
				} else if( mColorMgr.Param.FusionRatio_Dep == 0 && mColorMgr.Param.FusionRatio_Sem == 255 ) {
					OutBuf_SemDepFusion = OutBuf_SemWeiResize;
					PQMASKPrintf_SEtask("[%s] grp %d, Fusion OutBuf %s\n", TAG_NAME, ProcGrp, PQMaskSEBufName[OutBuf_SemDepFusion]);
				} else {

					// decide buffer
					OutBuf_SemDepFusion = PQMASK_SE_PROCBUF_01;

					if( true == SeProcStatus ) {
						// initial struct
						memset(&SeSrc1_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
						memset(&SeSrc2_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
						memset(&SeDst_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
						// fill information
						SeSrc1_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[OutBuf_SemWeiResize].AddrInfo.PhysAddr;
						SeSrc1_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
						SeSrc1_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
						SeSrc1_Surface[0].width = PQMASK_PROC_WIDTH;
						SeSrc1_Surface[0].height = PQMASK_PROC_HEIGHT;
						SeSrc1_Surface[0].bpp = 24;
						SeSrc2_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[OutBuf_Dep2Wei].AddrInfo.PhysAddr;
						SeSrc2_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
						SeSrc2_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
						SeSrc2_Surface[0].width = PQMASK_PROC_WIDTH;
						SeSrc2_Surface[0].height = PQMASK_PROC_HEIGHT;
						SeSrc2_Surface[0].bpp = 24;
						SeDst_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[OutBuf_SemDepFusion].AddrInfo.PhysAddr;
						SeDst_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
						SeDst_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
						SeDst_Surface[0].width = PQMASK_PROC_WIDTH;
						SeDst_Surface[0].height = PQMASK_PROC_HEIGHT;
						SeDst_Surface[0].bpp = 24;
						// trigger SE
						KGAL_PQMask_Bitblit(
							&SeSrc1_Surface[0], &SeSrc2_Surface[0], &SeDst_Surface[0],
							mColorMgr.Param.FusionRatio_Dep /*on src2*/,
							mColorMgr.Param.FusionRatio_Sem /*on src1*/,
							0
						);
						SeProcStatus = scalerPQMask_WaitSEDone();
					}

					if( true == SeProcStatus ) {
						PQMASKPrintf_SEtask("[%s] SE task grp %d done, semantic depth fusion, Buf %s, %s to %s\n",
							TAG_NAME, ProcGrp, PQMaskSEBufName[OutBuf_SemWeiResize], PQMaskSEBufName[OutBuf_Dep2Wei], PQMaskSEBufName[OutBuf_SemDepFusion]
						);
					} else {
						rtd_printk(KERN_EMERG, TAG_NAME, "SE task grp %d error, semantic depth fusion\n", ProcGrp);
					}
				}


#if PQMASK_FUNC_DUMPBUF // debug dump
				sprintf(DumpFileName, "%s_grp%d", "pqmask_Fusion", ProcGrp);
				PQMaskDumpSEBufCheck(OutBuf_SemDepFusion, DumpFileName);
#endif

				if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
					PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskGrpEnd[ProcGrp][5] = drvif_Get_90k_Lo_clk();
				}

				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
				// [SE] Spatial filter and RTNR, combine action if available
				// input  :	src1 = OutBuf_SemDepFusion ( PQMASK_SE_PROCBUF_00 or PQMASK_SE_PROCBUF_01 or PQMASK_SE_PROCBUF_02 )
				// 			src2 = dma previous frame data
				// output : dma address
				// ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****

				// pre-set some value in case
				OutBuf_SpatialFlt = OutBuf_SemDepFusion;
				OutBuf_Tnr = OutBuf_SpatialFlt;
				TNRFusion_Pre = mColorMgr.Param.FusionRatio_Pre;
				TNRFusion_Cur = mColorMgr.Param.FusionRatio_Cur;

				// scene change
				if( pColorMgr->Status.SCRstFrm > 0 ) {

					if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
						PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskGrpEnd[ProcGrp][6] = drvif_Get_90k_Lo_clk();
					}

					// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
					// no need to do spatial filter, just deal with the previous data and current data
					// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----

					// decide RTNR ratio
					TNRFusion_Pre = mColorMgr.Param.ScFusionRatio_Pre;
					TNRFusion_Cur = mColorMgr.Param.ScFusionRatio_Cur;

					if( TNRFusion_Pre == 255 && TNRFusion_Cur == 0 ) {
						for( i=0; i<3; i++ ) {
							TNRModuleSel = SEProcGrpMapping[ProcGrp][i];
							if( TNRModuleSel < PQMASK_MODULE_NUM ) {
								RingBufReadPtr  = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_READ);
								RingBufWritePtr = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_WRITE);
								scalerPQMask_Memcpy(
									(void *)(PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.VirtAddr), (void *)(PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.PhysAddr), 1,
									(void *)(PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufReadPtr].AddrInfo.VirtAddr), (void *)(PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufReadPtr].AddrInfo.PhysAddr), 1,
									PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.Size
								);
							}
						}
					} else if( TNRFusion_Pre == 0 && TNRFusion_Cur == 255 ) {
						for( i=0; i<3; i++ ) {
							TNRModuleSel = SEProcGrpMapping[ProcGrp][i];
							if( TNRModuleSel < PQMASK_MODULE_NUM ) {
								RingBufReadPtr  = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_READ);
								RingBufWritePtr = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_WRITE);
								scalerPQMask_Memset(
									(void *)(PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.VirtAddr),
									(void *)(PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.PhysAddr),
									0x80,
									PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.Size
								);
							}
						}
					} else {
						for(i=0; i<3; i++) {
							TNRModuleSel = SEProcGrpMapping[ProcGrp][i];
							scalerPQMask_Memset(
								(void *)(PQMaskBufMgr.SeBuffer[OutBuf_SpatialFlt].AddrInfo.VirtAddr),
								(void *)(PQMaskBufMgr.SeBuffer[OutBuf_SpatialFlt].AddrInfo.PhysAddr),
								0x80,
								PQMaskBufMgr.SeBuffer[OutBuf_SpatialFlt].AddrInfo.Size
							);
						}
						// decide related buffer index
						if( OutBuf_SpatialFlt == PQMASK_SE_PROCBUF_00 )
							TNRBuf_Dummy = PQMASK_SE_PROCBUF_01;
						else
							TNRBuf_Dummy = PQMASK_SE_PROCBUF_00;

						if( true == SeProcStatus ) {

							// initial structure and fill the information
							for( i=0; i<3; i++ ) {

								memset(&SeSrc1_Surface[i], 0x0, sizeof(KGAL_SURFACE_INFO_T)); // cur
								memset(&SeSrc2_Surface[i], 0x0, sizeof(KGAL_SURFACE_INFO_T)); // pre
								memset(&SeDst_Surface[i] , 0x0, sizeof(KGAL_SURFACE_INFO_T)); // dst

								TNRModuleSel = SEProcGrpMapping[ProcGrp][i];
								if( TNRModuleSel < PQMASK_MODULE_NUM ) {

									RingBufReadPtr  = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_READ);
									RingBufWritePtr = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_WRITE);

									SeSrc1_Surface[i].physicalAddress = PQMaskBufMgr.SeBuffer[OutBuf_SpatialFlt].AddrInfo.PhysAddr + SingleWeightPlaneSize*i;
									SeSrc1_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc1_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc1_Surface[i].width = PQMASK_PROC_WIDTH;
									SeSrc1_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeSrc1_Surface[i].bpp = 24;

									SeSrc2_Surface[i].physicalAddress = PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufReadPtr].AddrInfo.PhysAddr;
									SeSrc2_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc2_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc2_Surface[i].width = PQMASK_PROC_WIDTH;
									SeSrc2_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeSrc2_Surface[i].bpp = 24;

									SeDst_Surface[i].physicalAddress = PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.PhysAddr;
									SeDst_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeDst_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeDst_Surface[i].width = PQMASK_PROC_WIDTH;
									SeDst_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeDst_Surface[i].bpp = 24;

								} else {

									// TODO : check if this cause problem or not
									SeSrc1_Surface[i].physicalAddress = PQMaskBufMgr.SeBuffer[TNRBuf_Dummy].AddrInfo.PhysAddr;
									SeSrc1_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc1_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc1_Surface[i].width = PQMASK_PROC_WIDTH;
									SeSrc1_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeSrc1_Surface[i].bpp = 24;

									SeSrc2_Surface[i].physicalAddress = PQMaskBufMgr.SeBuffer[TNRBuf_Dummy].AddrInfo.PhysAddr + 1*PQMASK_PROC_WIDTH*PQMASK_PROC_HEIGHT;
									SeSrc2_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc2_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc2_Surface[i].width = PQMASK_PROC_WIDTH;
									SeSrc2_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeSrc2_Surface[i].bpp = 24;

									SeDst_Surface[i].physicalAddress = PQMaskBufMgr.SeBuffer[TNRBuf_Dummy].AddrInfo.PhysAddr + 2*PQMASK_PROC_WIDTH*PQMASK_PROC_HEIGHT;
									SeDst_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeDst_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeDst_Surface[i].width = PQMASK_PROC_WIDTH;
									SeDst_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeDst_Surface[i].bpp = 24;

								}
							}
						}

						// trigger SE
						KGAL_PQMask_Bitblit_SpearateAddr(
							&SeSrc1_Surface[0], &SeSrc1_Surface[1], &SeSrc1_Surface[2],
							&SeSrc2_Surface[0], &SeSrc2_Surface[1], &SeSrc2_Surface[2],
							&SeDst_Surface[0], &SeDst_Surface[1], &SeDst_Surface[2],
							TNRFusion_Pre, // on src2
							TNRFusion_Cur, // on src1
							0
						);
						SeProcStatus = scalerPQMask_WaitSEDone();
					}

					if( true == SeProcStatus ) {
						PQMASKPrintf_SEtask("[%s] SE task grp %d done, scene change only RTNR with %d pre %d cur\n", TAG_NAME, ProcGrp, TNRFusion_Pre, TNRFusion_Cur);
					} else {
						rtd_printk(KERN_EMERG, TAG_NAME, "SE task grp %d error, scene change only do RTNR\n", ProcGrp);
					}

#if PQMASK_FUNC_DUMPBUF // debug dump
					sprintf(DumpFileName, "%s_grp%d_sc", "pqmask_SpatialFlt", ProcGrp);
					PQMaskDumpSEBufCheck(OutBuf_SpatialFlt, DumpFileName);
					sprintf(DumpFileName, "%s_grp%d_sc", "pqmask_TNR", ProcGrp);
					PQMaskDumpSEBufCheck(OutBuf_Tnr, DumpFileName);
#endif

				} else if( (pColorMgr->Status.ForceCurFrm > 0) || ( mColorMgr.Param.FusionRatio_Pre == 0 && mColorMgr.Param.FusionRatio_Cur == 255 ) ) {

					// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
					// the case that only consider the current frame
					// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----

					TNRFusion_Pre = 0;
					TNRFusion_Cur = 255;

					// spatial filter
					if( mColorMgr.Param.SpatialFltCycle > 0 ) {

						// hard-core to decide the multi-cycle ping-pong buffer
						SpatialFltBuf_0 = OutBuf_SemDepFusion;
						if( OutBuf_SemDepFusion == PQMASK_SE_PROCBUF_00 ) {
							SpatialFltBuf_1 = PQMASK_SE_PROCBUF_01;
						} else {
							SpatialFltBuf_1 = PQMASK_SE_PROCBUF_00;
						}

						for( i=0; i<mColorMgr.Param.SpatialFltCycle; i++ ) {

							if( true == SeProcStatus ) {

								// initial struct
								memset(&SeSrc1_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
								memset(&SeDst_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));

								// fill information
								if( i%2 == 0 ) {
									SeSrc1_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[SpatialFltBuf_0].AddrInfo.PhysAddr;
									SeSrc1_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc1_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc1_Surface[0].width = PQMASK_PROC_WIDTH;
									SeSrc1_Surface[0].height = PQMASK_PROC_HEIGHT;
									SeSrc1_Surface[0].bpp = 24;
									SeDst_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[SpatialFltBuf_1].AddrInfo.PhysAddr;
									SeDst_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeDst_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeDst_Surface[0].width = PQMASK_PROC_WIDTH;
									SeDst_Surface[0].height = PQMASK_PROC_HEIGHT;
									SeDst_Surface[0].bpp = 24;

									OutBuf_SpatialFlt = SpatialFltBuf_1;
								} else {
									SeSrc1_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[SpatialFltBuf_1].AddrInfo.PhysAddr;
									SeSrc1_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc1_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc1_Surface[0].width = PQMASK_PROC_WIDTH;
									SeSrc1_Surface[0].height = PQMASK_PROC_HEIGHT;
									SeSrc1_Surface[0].bpp = 24;
									SeDst_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[SpatialFltBuf_0].AddrInfo.PhysAddr;
									SeDst_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeDst_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeDst_Surface[0].width = PQMASK_PROC_WIDTH;
									SeDst_Surface[0].height = PQMASK_PROC_HEIGHT;
									SeDst_Surface[0].bpp = 24;

									OutBuf_SpatialFlt = SpatialFltBuf_0;
								}
								// trigger SE
								KGAL_PQMaskStretch_Coef(&SeSrc1_Surface[0], &SeDst_Surface[0],
									mColorMgr.Param.SEHorCoef, mColorMgr.Param.HorOddType,
									mColorMgr.Param.SEVerCoef, mColorMgr.Param.VerOddType
								);
								SeProcStatus = scalerPQMask_WaitSEDone();
							}

							if( true == SeProcStatus ) {
								PQMASKPrintf_SEtask("[%s] SE task grp %d done, se spatial filter, ping %s pong %s\n",
									TAG_NAME, ProcGrp, PQMaskSEBufName[SpatialFltBuf_0], PQMaskSEBufName[SpatialFltBuf_1]
								);
							} else {
								rtd_printk(KERN_EMERG, TAG_NAME, "SE task grp %d error, se spatial filter cycle %d\n", ProcGrp, i);
							}

						}

					} else {
						OutBuf_SpatialFlt = OutBuf_SemDepFusion;
					}
					PQMASKPrintf_SEtask("[%s] grp %d, Spatial filter OutBuf %s\n", TAG_NAME, ProcGrp, PQMaskSEBufName[OutBuf_SpatialFlt]);

#if PQMASK_FUNC_DUMPBUF // debug dump
					sprintf(DumpFileName, "%s_grp%d_forceCur", "pqmask_SpatialFlt", ProcGrp);
					PQMaskDumpSEBufCheck(OutBuf_SpatialFlt, DumpFileName);
#endif

					if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
						PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskGrpEnd[ProcGrp][6] = drvif_Get_90k_Lo_clk();
					}

					// put data to each module dma
					OutBuf_Tnr = OutBuf_SpatialFlt;
					for( i=0; i<3; i++ ) {
						TNRModuleSel = SEProcGrpMapping[ProcGrp][i];
						if( TNRModuleSel < PQMASK_MODULE_NUM ) {
							RingBufWritePtr = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_WRITE);
							scalerPQMask_Memcpy(
								(void *)(PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.VirtAddr), (void *)(PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.PhysAddr), 1,
								(void *)(PQMaskBufMgr.SeBuffer[OutBuf_Tnr].AddrInfo.VirtAddr + SingleWeightPlaneSize*i), (void *)(PQMaskBufMgr.SeBuffer[OutBuf_Tnr].AddrInfo.PhysAddr + SingleWeightPlaneSize*i), 1,
								PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.Size
							);
						}
					}
					PQMASKPrintf_SEtask("[%s] grp %d, put data to module buffer done\n", TAG_NAME, ProcGrp);

#if PQMASK_FUNC_DUMPBUF // debug dump
					sprintf(DumpFileName, "%s_grp%d_forceCur", "pqmask_TNR", ProcGrp);
					PQMaskDumpSEBufCheck(OutBuf_Tnr, DumpFileName);
#endif

				} else if( mColorMgr.Param.FusionRatio_Pre == 255 && mColorMgr.Param.FusionRatio_Cur == 0 ) {

					if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
						PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskGrpEnd[ProcGrp][6] = drvif_Get_90k_Lo_clk();
					}

					// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
					// just copy the previous frame data to current frame
					// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----

					TNRFusion_Pre = mColorMgr.Param.FusionRatio_Pre;
					TNRFusion_Cur = mColorMgr.Param.FusionRatio_Cur;

					for( i=0; i<3; i++ ) {
						TNRModuleSel = SEProcGrpMapping[ProcGrp][i];
						if( TNRModuleSel < PQMASK_MODULE_NUM ) {
							RingBufReadPtr  = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_READ);
							RingBufWritePtr = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_WRITE);
							scalerPQMask_Memcpy(
								(void *)(PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.VirtAddr), (void *)(PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.PhysAddr), 1,
								(void *)(PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufReadPtr].AddrInfo.VirtAddr), (void *)(PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufReadPtr].AddrInfo.PhysAddr), 1,
								PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.Size
							);
						}
					}
					PQMASKPrintf_SEtask("[%s] SE task grp %d done, RTNR with %d pre %d cur\n", TAG_NAME, ProcGrp, TNRFusion_Pre, TNRFusion_Cur);

#if PQMASK_FUNC_DUMPBUF // debug dump
					sprintf(DumpFileName, "%s_grp%d_onlyPre", "pqmask_SpatialFlt", ProcGrp);
					PQMaskDumpSEBufCheck(OutBuf_SpatialFlt, DumpFileName);
					sprintf(DumpFileName, "%s_grp%d_onlyPre", "pqmask_TNR", ProcGrp);
					PQMaskDumpSEBufCheck(OutBuf_Tnr, DumpFileName);
#endif

				} else {

					// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
					// the case that need to do temporal filter
					// consider to combine the spatial filter together if available
					// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----

					if( mColorMgr.Param.SpatialFltCycle > 0 ) {

						// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
						// spatial filter, the last loop will combie with temporal filter
						// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====

						// hard-core to decide the multi-cycle ping-pong buffer
						SpatialFltBuf_0 = OutBuf_SemDepFusion;
						if( OutBuf_SemDepFusion == PQMASK_SE_PROCBUF_00 ) {
							SpatialFltBuf_1 = PQMASK_SE_PROCBUF_01;
						} else {
							SpatialFltBuf_1 = PQMASK_SE_PROCBUF_00;
						}

						for( i=0; i<mColorMgr.Param.SpatialFltCycle-1; i++ ) {

							if( true == SeProcStatus ) {

								// initial struct
								memset(&SeSrc1_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));
								memset(&SeDst_Surface[0], 0, sizeof(KGAL_SURFACE_INFO_T));

								// fill information
								if( i%2 == 0 ) {
									SeSrc1_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[SpatialFltBuf_0].AddrInfo.PhysAddr;
									SeSrc1_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc1_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc1_Surface[0].width = PQMASK_PROC_WIDTH;
									SeSrc1_Surface[0].height = PQMASK_PROC_HEIGHT;
									SeSrc1_Surface[0].bpp = 24;
									SeDst_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[SpatialFltBuf_1].AddrInfo.PhysAddr;
									SeDst_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeDst_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeDst_Surface[0].width = PQMASK_PROC_WIDTH;
									SeDst_Surface[0].height = PQMASK_PROC_HEIGHT;
									SeDst_Surface[0].bpp = 24;

									OutBuf_SpatialFlt = SpatialFltBuf_1;
								} else {
									SeSrc1_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[SpatialFltBuf_1].AddrInfo.PhysAddr;
									SeSrc1_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc1_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc1_Surface[0].width = PQMASK_PROC_WIDTH;
									SeSrc1_Surface[0].height = PQMASK_PROC_HEIGHT;
									SeSrc1_Surface[0].bpp = 24;
									SeDst_Surface[0].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[SpatialFltBuf_0].AddrInfo.PhysAddr;
									SeDst_Surface[0].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeDst_Surface[0].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeDst_Surface[0].width = PQMASK_PROC_WIDTH;
									SeDst_Surface[0].height = PQMASK_PROC_HEIGHT;
									SeDst_Surface[0].bpp = 24;

									OutBuf_SpatialFlt = SpatialFltBuf_0;
								}

								// trigger SE
								KGAL_PQMaskStretch_Coef(&SeSrc1_Surface[0], &SeDst_Surface[0],
									mColorMgr.Param.SEHorCoef, mColorMgr.Param.HorOddType,
									mColorMgr.Param.SEVerCoef, mColorMgr.Param.VerOddType
								);
								SeProcStatus = scalerPQMask_WaitSEDone();
							}

							if( true == SeProcStatus ) {
								PQMASKPrintf_SEtask("[%s] SE task grp %d done, se spatial filter, ping %s pong %s\n",
									TAG_NAME, ProcGrp, PQMaskSEBufName[SpatialFltBuf_0], PQMaskSEBufName[SpatialFltBuf_1]
								);
							} else {
								rtd_printk(KERN_EMERG, TAG_NAME, "SE task grp %d error, se spatial filter cycle %d\n", ProcGrp, i);
							}

						}
						PQMASKPrintf_SEtask("[%s] grp %d, Spatial filter OutBuf %s\n", TAG_NAME, ProcGrp, PQMaskSEBufName[OutBuf_SpatialFlt]);

#if PQMASK_FUNC_DUMPBUF // debug dump
						sprintf(DumpFileName, "%s_grp%d_preTNR", "pqmask_SpatialFlt", ProcGrp);
						PQMaskDumpSEBufCheck(OutBuf_SpatialFlt, DumpFileName);
#endif

						if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
							PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskGrpEnd[ProcGrp][6] = drvif_Get_90k_Lo_clk();
						}

						// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
						// spatial filter last loop + temporal filter
						// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====

						TNRFusion_Pre = mColorMgr.Param.FusionRatio_Pre;
						TNRFusion_Cur = mColorMgr.Param.FusionRatio_Cur;

						// decide related buffer index
						if( OutBuf_SpatialFlt == PQMASK_SE_PROCBUF_00 )
							TNRBuf_Dummy = PQMASK_SE_PROCBUF_01;
						else
							TNRBuf_Dummy = PQMASK_SE_PROCBUF_00;

						if( true == SeProcStatus ) {

							// initial struct
							for(i=0; i<3; i++) {
								memset(&SeSrc1_Surface[i], 0, sizeof(KGAL_SURFACE_INFO_T));
								memset(&SeSrc2_Surface[i], 0, sizeof(KGAL_SURFACE_INFO_T));
								memset(&SeDst_Surface[i], 0, sizeof(KGAL_SURFACE_INFO_T));
							}

							// fill information
							for(i=0; i<3; i++) {

								SeSrc1_Surface[i].physicalAddress = (UINT32)PQMaskBufMgr.SeBuffer[OutBuf_SpatialFlt].AddrInfo.PhysAddr+SingleWeightPlaneSize*i;
								SeSrc1_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
								SeSrc1_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
								SeSrc1_Surface[i].width = PQMASK_PROC_WIDTH;
								SeSrc1_Surface[i].height = PQMASK_PROC_HEIGHT;
								SeSrc1_Surface[i].bpp = 24;

								TNRModuleSel = SEProcGrpMapping[ProcGrp][i];

								if( TNRModuleSel < PQMASK_MODULE_NUM ) {

									RingBufReadPtr = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_READ);
									RingBufWritePtr = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_WRITE);

									SeSrc2_Surface[i].physicalAddress = PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufReadPtr].AddrInfo.PhysAddr;
									SeSrc2_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc2_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc2_Surface[i].width = PQMASK_PROC_WIDTH;
									SeSrc2_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeSrc2_Surface[i].bpp = 24;

									SeDst_Surface[i].physicalAddress = PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.PhysAddr;
									SeDst_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeDst_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeDst_Surface[i].width = PQMASK_PROC_WIDTH;
									SeDst_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeDst_Surface[i].bpp = 24;

								} else {

									SeSrc2_Surface[i].physicalAddress = PQMaskBufMgr.SeBuffer[TNRBuf_Dummy].AddrInfo.PhysAddr;
									SeSrc2_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc2_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc2_Surface[i].width = PQMASK_PROC_WIDTH;
									SeSrc2_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeSrc2_Surface[i].bpp = 24;

									SeDst_Surface[i].physicalAddress = PQMaskBufMgr.SeBuffer[TNRBuf_Dummy].AddrInfo.PhysAddr + PQMASK_PROC_WIDTH*PQMASK_PROC_HEIGHT*1;
									SeDst_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeDst_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeDst_Surface[i].width = PQMASK_PROC_WIDTH;
									SeDst_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeDst_Surface[i].bpp = 24;

								}

							}

							KGAL_PQMask_Stretch_Coef_Bitblit_SpearateAddr(
								&SeSrc1_Surface[0], &SeSrc1_Surface[1], &SeSrc1_Surface[2],
								&SeSrc2_Surface[0], &SeSrc2_Surface[1], &SeSrc2_Surface[2],
								&SeDst_Surface[0], &SeDst_Surface[1], &SeDst_Surface[2],
								// spatial filter parameters
								mColorMgr.Param.SEHorCoef, mColorMgr.Param.HorOddType,
								mColorMgr.Param.SEVerCoef, mColorMgr.Param.VerOddType,
								// temporal filter parameters
								TNRFusion_Pre, // on Src2
								TNRFusion_Cur, // on Src1
								0
							);
							SeProcStatus = scalerPQMask_WaitSEDone();

						}

						if( true == SeProcStatus ) {
							PQMASKPrintf_SEtask("[%s] SE task grp %d done, spatial (%s) temporal combine\n", TAG_NAME, ProcGrp, PQMaskSEBufName[OutBuf_SpatialFlt]);
						} else {
							rtd_printk(KERN_EMERG, TAG_NAME, "SE task grp %d error, spatial (%s) temporal combine\n", ProcGrp, PQMaskSEBufName[OutBuf_SpatialFlt]);
						}

#if PQMASK_FUNC_DUMPBUF // debug dump
						sprintf(DumpFileName, "%s_grp%d_Combine", "pqmask_TNR", ProcGrp);
						PQMaskDumpSEBufCheck(OutBuf_Tnr, DumpFileName);
#endif

					} else {

						if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
							PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskGrpEnd[ProcGrp][6] = drvif_Get_90k_Lo_clk();
						}

						// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
						// simple temporal filter
						// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====

						// decide related buffer index
						if( OutBuf_SpatialFlt == PQMASK_SE_PROCBUF_00 )
							TNRBuf_Dummy = PQMASK_SE_PROCBUF_01;
						else
							TNRBuf_Dummy = PQMASK_SE_PROCBUF_00;

						if( true == SeProcStatus ) {

							// initial structure and fill the information
							for( i=0; i<3; i++ ) {

								memset(&SeSrc1_Surface[i], 0x0, sizeof(KGAL_SURFACE_INFO_T)); // cur
								memset(&SeSrc2_Surface[i], 0x0, sizeof(KGAL_SURFACE_INFO_T)); // pre
								memset(&SeDst_Surface[i] , 0x0, sizeof(KGAL_SURFACE_INFO_T)); // dst

								TNRModuleSel = SEProcGrpMapping[ProcGrp][i];
								if( TNRModuleSel < PQMASK_MODULE_NUM ) {

									RingBufReadPtr  = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_READ);
									RingBufWritePtr = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_WRITE);

									SeSrc1_Surface[i].physicalAddress = PQMaskBufMgr.SeBuffer[OutBuf_SpatialFlt].AddrInfo.PhysAddr + SingleWeightPlaneSize*i;
									SeSrc1_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc1_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc1_Surface[i].width = PQMASK_PROC_WIDTH;
									SeSrc1_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeSrc1_Surface[i].bpp = 24;

									SeSrc2_Surface[i].physicalAddress = PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufReadPtr].AddrInfo.PhysAddr;
									SeSrc2_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc2_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc2_Surface[i].width = PQMASK_PROC_WIDTH;
									SeSrc2_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeSrc2_Surface[i].bpp = 24;

									SeDst_Surface[i].physicalAddress = PQMaskBufMgr.ModuleDma[TNRModuleSel][RingBufWritePtr].AddrInfo.PhysAddr;
									SeDst_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeDst_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeDst_Surface[i].width = PQMASK_PROC_WIDTH;
									SeDst_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeDst_Surface[i].bpp = 24;

								} else {

									SeSrc1_Surface[i].physicalAddress = PQMaskBufMgr.SeBuffer[TNRBuf_Dummy].AddrInfo.PhysAddr;
									SeSrc1_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc1_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc1_Surface[i].width = PQMASK_PROC_WIDTH;
									SeSrc1_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeSrc1_Surface[i].bpp = 24;

									SeSrc2_Surface[i].physicalAddress = PQMaskBufMgr.SeBuffer[TNRBuf_Dummy].AddrInfo.PhysAddr + 1*PQMASK_PROC_WIDTH*PQMASK_PROC_HEIGHT;
									SeSrc2_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeSrc2_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeSrc2_Surface[i].width = PQMASK_PROC_WIDTH;
									SeSrc2_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeSrc2_Surface[i].bpp = 24;

									SeDst_Surface[i].physicalAddress = PQMaskBufMgr.SeBuffer[TNRBuf_Dummy].AddrInfo.PhysAddr + 2*PQMASK_PROC_WIDTH*PQMASK_PROC_HEIGHT;
									SeDst_Surface[i].pixelFormat = KGAL_PIXEL_FORMAT_YUV444;
									SeDst_Surface[i].pitch = PQMASK_PROC_WIDTH*8/8; // bytes per line
									SeDst_Surface[i].width = PQMASK_PROC_WIDTH;
									SeDst_Surface[i].height = PQMASK_PROC_HEIGHT;
									SeDst_Surface[i].bpp = 24;
								}
							}
						}

						// trigger SE
						KGAL_PQMask_Bitblit_SpearateAddr(
							&SeSrc1_Surface[0], &SeSrc1_Surface[1], &SeSrc1_Surface[2],
							&SeSrc2_Surface[0], &SeSrc2_Surface[1], &SeSrc2_Surface[2],
							&SeDst_Surface[0], &SeDst_Surface[1], &SeDst_Surface[2],
							TNRFusion_Pre, // on src2
							TNRFusion_Cur, // on src1
							0
						);
						SeProcStatus = scalerPQMask_WaitSEDone();

						if( true == SeProcStatus ) {
							PQMASKPrintf_SEtask("[%s] SE task grp %d done, simple RTNR with %d pre %d cur\n", TAG_NAME, ProcGrp, TNRFusion_Pre, TNRFusion_Cur);
						} else {
							rtd_printk(KERN_EMERG, TAG_NAME, "SE task grp %d error, simple RTNR with %d pre %d cur\n", ProcGrp, TNRFusion_Pre, TNRFusion_Cur);
						}

					}

#if PQMASK_FUNC_DUMPBUF // debug dump
					sprintf(DumpFileName, "%s_grp%d_simpleTNR", "pqmask_SpatialFlt", ProcGrp);
					PQMaskDumpSEBufCheck(OutBuf_SpatialFlt, DumpFileName);
					sprintf(DumpFileName, "%s_grp%d_simpleTNR", "pqmask_TNR", ProcGrp);
					PQMaskDumpSEBufCheck(OutBuf_Tnr, DumpFileName);
#endif
				}

				if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
					for( i=0; i<3; i++ ) {
						if( TNRModuleSel < PQMASK_MODULE_NUM ) {
							RingBufReadPtr = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_READ);
							RingBufWritePtr = PQMaskRingBuf[TNRModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_WRITE);
							PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].RingBufStatus[TNRModuleSel][VIP_RINGBUF_PTR_READ]  = RingBufReadPtr;
							PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].RingBufStatus[TNRModuleSel][VIP_RINGBUF_PTR_WRITE] = RingBufWritePtr;
						}
					}
				}

				// pointer move forward
				for( i=0; i<3; i++ ) {
					TNRModuleSel = SEProcGrpMapping[ProcGrp][i];
					if( TNRModuleSel < PQMASK_MODULE_NUM ) {
						PQMaskRingBuf[TNRModuleSel].FuncSetIdx((void *)&PQMaskRingBuf[TNRModuleSel], VIP_RINGBUF_PTR_WRITE);
					}
				}

				if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
					PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskGrpEnd[ProcGrp][7] = drvif_Get_90k_Lo_clk();
				}
			}

			// some counters
			if( pColorMgr->Status.SCRstFrm > 0 ) {
				pColorMgr->Status.SCRstFrm --;
			}
			if( pColorMgr->Status.ForceCurFrm > 0 ) {
				pColorMgr->Status.ForceCurFrm--;
			}

		}

		// performance information record
		if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
			PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskEnd = drvif_Get_90k_Lo_clk();
			PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].RepeatTimes = 0;
		} else {
			PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].RepeatSETaskEnd = drvif_Get_90k_Lo_clk();
			PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].RepeatTimes++;
		}

		if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
			PQMASKPrintf_SEtask("[%s][%s] done tsp = %d at %d, new frm = %d\n",
				TAG_NAME, __FUNCTION__,
				PQMaskGlbMgr.Status.SeStatus.ProcTsp, PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].SETaskEnd, PQMaskGlbMgr.Ctrl.bSemNewInfo
			);
		} else {
			PQMASKPrintf_SEtask("[%s][%s] done at %d, RepeatTimes = %d\n", TAG_NAME, __FUNCTION__, drvif_Get_90k_Lo_clk(), PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].RepeatTimes );
		}

		// end
		PQMaskGlbMgr.Status.SeStatus.IsRunning = 0;

	}

#if PQMASK_FUNC_DUMPBUF
	dumpCounter++;
#endif


	// update status
	if( PQMaskGlbMgr.Status.PQMaskStatus == PQMASK_STAT_SE_PROC )
		PQMaskGlbMgr.Status.PQMaskStatus = PQMASK_STAT_READY;

}

/****************************** Functions ******************************/

/**
 * @brief
 * get the interanl control struct that could be modified by others
 * @param int
 * info index
 * @return void*
 * pointer to the specific structure
 */
void *scalerPQMask_GetCtrlStruct(unsigned int InfoIndex) {
	void * RetPtr = NULL;
	switch(InfoIndex) {
		case PQMASK_STRUCT_PERF:
			RetPtr = (void *)PQMaskPerfMgr;
			break;
		case PQMASK_STRUCT_GLB_CTRL:
			RetPtr = (void *)&PQMaskGlbMgr.Ctrl;
			break;
		case PQMASK_STRUCT_MONITOR_CTRL:
			RetPtr = (void *)&PQMaskMonitorMgr.Ctrl;
			break;
		default:
			rtd_printk(KERN_EMERG, TAG_NAME, "[%s] index = %d error\n", __FUNCTION__, InfoIndex);
			RetPtr = NULL;
			break;
	}
	return RetPtr;
}

/**
 * @brief
 * get the interanl control struct that could not be modified by others
 * @param int
 * info index
 * @return const void*
 * pointer to the specific structure
 */
const void*scalerPQMask_GetStatusStruct(unsigned int InfoIndex) {
	const void * RetPtr = NULL;
	switch(InfoIndex) {
		case PQMASK_STRUCT_BLKMODULE_INFO:
			RetPtr = (const void *)PQMaskBlkModuleCfg;
			break;
		case PQMASK_STRUCT_BUFINFO:
			RetPtr = (const void *)&PQMaskBufMgr;
			break;
		case PQMASK_STRUCT_GLB_STATUS:
			RetPtr = (const void *)&PQMaskGlbMgr.Status;
			break;
		case PQMASK_STRUCT_MONITOR_STATUS:
			RetPtr = (void *)&PQMaskMonitorMgr.Status;
			break;
		default:
			rtd_printk(KERN_EMERG, TAG_NAME, "[%s] index = %d error\n", __FUNCTION__, InfoIndex);
			RetPtr = NULL;
			break;
	}
	return RetPtr;
}

/**
 * @brief
 * online monitor the status and print log if necessary
 */
void scalerPQMask_OnlineMonitor(void) {

	unsigned int TimeDiff = 0;
	unsigned int SEProcTimeMS = 0;
	unsigned int DepthFps = 0;

	if( PQMaskMonitorMgr.Ctrl.bEnable == 0 )
		return;

	// check SE process time
	if( PQMaskMonitorMgr.Status.SECurProcEnd > PQMaskMonitorMgr.Status.SECurProcStart ) {
		TimeDiff = PQMaskMonitorMgr.Status.SECurProcEnd - PQMaskMonitorMgr.Status.SECurProcStart;
	} else {
		TimeDiff = (0xffffffff) - (PQMaskMonitorMgr.Status.SECurProcStart-PQMaskMonitorMgr.Status.SECurProcEnd);
	}

	SEProcTimeMS = TimeDiff/90;
	if( PQMaskMonitorMgr.Ctrl.bLogEnable && SEProcTimeMS > PQMaskMonitorMgr.Ctrl.SEProc_TargetTime ) {
		PQMaskMonitorMgr.Status.SEProc_TimeError = SEProcTimeMS;
	}

	// check input depth
	if( PQMaskGlbMgr.Ctrl.bIsDepthReady != 0 ) {
		if( PQMaskMonitorMgr.Status.Depth_Counter >= PQMaskMonitorMgr.Ctrl.Depth_MeasureRstFrm ) {

			if( PQMaskMonitorMgr.Status.Depth_Counter != 0 ) {
				if(PQMaskMonitorMgr.Status.Depth_MeauserEndTime > PQMaskMonitorMgr.Status.Depth_MeasureStartTime) {
					TimeDiff = PQMaskMonitorMgr.Status.Depth_MeauserEndTime - PQMaskMonitorMgr.Status.Depth_MeasureStartTime;
				} else {
					TimeDiff = (0xffffffff) - (PQMaskMonitorMgr.Status.Depth_MeasureStartTime - PQMaskMonitorMgr.Status.Depth_MeauserEndTime);
				}

				DepthFps = (90*1000*PQMaskMonitorMgr.Status.Depth_Counter)/TimeDiff;
				if( DepthFps < PQMaskMonitorMgr.Ctrl.Depth_TargetFps ) {
					PQMaskMonitorMgr.Status.Depth_FpsError = DepthFps;
				}

			}
			PQMaskMonitorMgr.Status.Depth_Counter = 0;
		} else {
			PQMaskMonitorMgr.Status.Depth_Counter++;
		}
	}


	// print log
	if( PQMaskMonitorMgr.Ctrl.bLogEnable && PQMaskMonitorMgr.Status.LogCounter >= PQMaskGlbMgr.Ctrl.LimitFPS*PQMaskMonitorMgr.Ctrl.LogPeriod ) {
		if( PQMaskMonitorMgr.Status.Depth_FpsError != 0 ) {
			rtd_printk(KERN_EMERG, TAG_AIPQ_ERR, "PQMask input depth fps %d less than expected %d\n",
				PQMaskMonitorMgr.Status.Depth_FpsError, PQMaskMonitorMgr.Ctrl.Depth_TargetFps
			);
		}
		if( PQMaskMonitorMgr.Status.SEProc_TimeError != 0 ) {
			rtd_printk(KERN_EMERG, TAG_AIPQ_ERR, "PQMask StreamEngine process %d ms over expected %d ms\n",
				PQMaskMonitorMgr.Status.SEProc_TimeError,
				PQMaskMonitorMgr.Ctrl.SEProc_TargetTime
			);
		} else {
			rtd_printk(KERN_EMERG, TAG_NAME, "Current StreamEngine process %d ms\n", SEProcTimeMS);
		}
		PQMaskMonitorMgr.Status.LogCounter = 0;

		// clear the error flags
		PQMaskMonitorMgr.Status.Depth_FpsError = 0;
		PQMaskMonitorMgr.Status.SEProc_TimeError = 0;
	}

	PQMaskMonitorMgr.Status.LogCounter++;
}

/**
 * @brief
 * control the fps for excution time by 90k clk
 * user need to maintain/update PreTime manually
 * @param CurTime
 * @param PreTime
 * @param bPrintEnable
 * @param Key
 * @return true
 * @return false
 */
bool scalerPQMask_FpsCtrl(unsigned int *CurTime, unsigned int *PreTime, unsigned int TargetFps, unsigned char bPrintEnable, char *Key) {

	unsigned int TimeDiff = 0;
	bool Ret = false;

	// error checking
	if( TargetFps == 0 ) {
		if( bPrintEnable != 0 ) {
			rtd_printk(KERN_INFO, "", "[%s] TargetFPS = 0\n", Key);
		}
		return false;
	}

	// calculate time diff
	*CurTime = drvif_Get_90k_Lo_clk();
	if( *CurTime > *PreTime ) {
		TimeDiff = *CurTime - *PreTime;
	} else {
		TimeDiff = (0xffffffff) - (*PreTime-*CurTime);
	}

	// result
	if( TimeDiff >= (90*1000)/(TargetFps) ) {
		Ret = true;
	} else {
		Ret = false;
	}
	if( bPrintEnable != 0 ) {
		rtd_printk(KERN_INFO, "", "[%s] CurTime = %10d, PreTime = %10d, TimeDiff = %10d, Ret = %d\n", Key, *CurTime, *PreTime, TimeDiff, Ret);
	}

	return Ret;
}

/**
 * @brief
 * initialize the PQMask module control status
 * we assume all the status are not set at this time
 */
void scalerPQMask_init(void) {

	int idx = 0, i = 0;
	int needInit = 0;
	int status = RET_OK;
	VIP_NN_CTRL *pNN = NULL;
	PQMaskFlag *pPQMaskFlag = NULL;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start with status = %s\n", __FUNCTION__, PQMaskStatusStr[PQMaskGlbMgr.Status.PQMaskStatus]);

	pNN = scalerAI_Access_NN_CTRL_STRUCT();
	idx = VIP_NN_BUFFER_480FULL;

	// check if we need to init or not
	if( PQMaskGlbMgr.Status.PQMaskStatus == PQMASK_STAT_UNKOWN ) {
		rtd_printk(KERN_INFO, TAG_NAME, "[%s] Status is %s, need init\n", __FUNCTION__, PQMaskStatusStr[PQMaskGlbMgr.Status.PQMaskStatus]);
		needInit = 1;
	}

	if( PQMaskGlbMgr.Status.PQMaskStatus == PQMASK_STAT_ERR ) {
		rtd_printk(KERN_INFO, TAG_NAME, "[%s] Status is %s, need init\n", __FUNCTION__, PQMaskStatusStr[PQMaskGlbMgr.Status.PQMaskStatus]);
		needInit = 1;
	}

	if( scalerPQMask_CheckResolution() != RET_OK ) {
		rtd_printk(KERN_INFO, TAG_NAME, "[%s] Module dma size wrong, need init\n", __FUNCTION__);
		needInit = 1;
	}

	// make sure to reset the status
	PQMaskGlbMgr.Status.PQMaskStatus = PQMASK_STAT_UNKOWN;
	if( scalerPQMask_WaitSETaskFinish(60) != 0 ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] init fail, cannot reset the status\n", __FUNCTION__);
		return;
	}

	// init procedure
	if( needInit == 1 ) {

		// disable all pqmask module
		scalerPQMaskColor_disable();

		// disable all pqmask dma
		scalerPQMask_disableModuleDMA();

		// co-buffer content
		rtd_printk(KERN_INFO, TAG_NAME, "[%s] init co-buffer\n", __FUNCTION__);
		if( pNN->NN_flag_Addr[idx].pVir_addr_align == 0 ) {
			rtd_printk(KERN_EMERG, TAG_NAME, "[%s] null nn structure address\n", __FUNCTION__);
			status = RET_FAIL;
		} else {
			pPQMaskFlag = (PQMaskFlag*)(pNN->NN_flag_Addr[idx].pVir_addr_align);
			for(i=0; i<PQMASK_TUNNEL_NUM; i++) {
				pPQMaskFlag[i].TimeStamp = 0;
				pPQMaskFlag[i].Update = 0;
			}
		}

		// structure parameter
		PQMaskGlbMgr.Status.ErrRecoverTimes = PQMaskGlbMgr.Ctrl.ErrRecoverRstTimes;

		// ring buffer info
		rtd_printk(KERN_INFO, TAG_NAME, "[%s] init ring buffer\n", __FUNCTION__);
		for(i=0; i<PQMASK_MODULE_NUM; i++) {
			PQMaskRingBuf[i].FuncInit = &PQMaskRingBuf_Init;
			PQMaskRingBuf[i].FuncInit((void*)&PQMaskRingBuf[i], (void *)&i);
		}

		// pqmask related dma allocate
		if( status == RET_OK ) {
			rtd_printk(KERN_INFO, TAG_NAME, "[%s] init module buffer\n", __FUNCTION__);
			if( scalerPQMask_DmaAllocate() != RET_OK ) {
				scalerPQMask_DmaFree();
				rtd_printk(KERN_EMERG, TAG_NAME, "[%s] init module buffer fail\n", __FUNCTION__);
				status = RET_FAIL;
			}
		}

		// reset all the dma content
		if( status == RET_OK ) {
			scalerPQMask_ResetDma();
		}

		// reset the se task status
		PQMaskGlbMgr.Status.SeStatus.ProcTsp = 0;

		// reset the performance measurement struct
		memset(&PQMaskPerfMgr, 0x0, sizeof(PQMASK_PERF_T)*PQMASK_PERF_NUM);

		// reset the dma related setting
		if( status == RET_OK ) {
			scalerPQMask_InitModuleConfig();
		}

		// pq setting
		scalerPQMaskColor_init();

		if( status != RET_OK )
			PQMaskGlbMgr.Status.PQMaskStatus = PQMASK_STAT_ERR;
		else
			PQMaskGlbMgr.Status.PQMaskStatus = PQMASK_STAT_READY;

	} else {
		scalerPQMask_InitModuleConfig();
		scalerPQMaskColor_init();
		PQMaskGlbMgr.Status.PQMaskStatus = PQMASK_STAT_READY;
	}

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done, status = %s\n", __FUNCTION__, PQMaskStatusStr[PQMaskGlbMgr.Status.PQMaskStatus]);

}

/**
 * @brief
 * try to free the dma and disable all the related modules
 */
void scalerPQMask_exit(void) {
	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start, current status = %s\n", __FUNCTION__, PQMaskStatusStr[PQMaskGlbMgr.Status.PQMaskStatus]);
	PQMaskGlbMgr.Status.PQMaskStatus = PQMASK_STAT_UNKOWN;
	if( scalerPQMask_WaitSETaskFinish(60) != RET_FAIL ) {
		scalerPQMask_DmaFree();
	}
	scalerPQMaskColor_disable();
	scalerPQMask_disableModuleDMA();
	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);
}

/**
 * @brief
 * try to suspend the pqmask whole module w/o free memory
 */
void scalerPQMask_suspend(void) {

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start, current status = %s\n", __FUNCTION__, PQMaskStatusStr[PQMaskGlbMgr.Status.PQMaskStatus]);
	if( PQMaskGlbMgr.Status.PQMaskStatus != PQMASK_STAT_UNKOWN ) {
		PQMaskGlbMgr.Status.PQMaskStatus = PQMASK_STAT_STANDBY;
		if( scalerPQMask_WaitSETaskFinish(60) == RET_FAIL ) {
			rtd_printk(KERN_EMERG, TAG_NAME, "cannot wait se task done\n");
		}
		scalerPQMask_ResetDma();
		scalerPQMaskColor_disable();
		PQMaskGlbMgr.Ctrl.bIsDepthReady = 0;
		PQMaskGlbMgr.Ctrl.bIsSemReady = 0;
	}
	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);
}

/**
 * @brief
 * use to wait pqmask se task finish
 * @param WaitCnt : wait time = cnt * 0.5ms(unit)
 * @return int
 * success or fail
 */
int scalerPQMask_WaitSETaskFinish(unsigned int WaitCnt) {

	int WaitSEFinsihTimeOut = WaitCnt;
	int Ret = RET_OK;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start, cnt = %d\n", __FUNCTION__, WaitSEFinsihTimeOut);

	do
	{
		usleep_range(500, 1000);
		WaitSEFinsihTimeOut--;
	}
	while( (WaitSEFinsihTimeOut>0) && (PQMaskGlbMgr.Status.SeStatus.IsRunning != 0) );

	if( WaitSEFinsihTimeOut <= 0 )
		Ret = RET_FAIL;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done with cnt = %d, rst = %d\n", __FUNCTION__, WaitSEFinsihTimeOut, Ret);

	return Ret;
}

/**
 * @brief
 * check any dma is ok or not
 * @return int : check status
 */
int scalerPQMask_CheckDMA(void) {
	int i=0, j=0;

	// module dma
	for(i=0; i<PQMASK_MODULE_NUM; i++) {
		for(j=0; j<PQMASK_MODULE_DMA_NUM; j++) {
			if( PQMaskBufMgr.ModuleDma[i][j].AddrInfo.VirtAddr == 0 )
				return RET_FAIL;
		}
	}

	// se dma
	for(i=0; i<PQMASK_SE_BUF_NUM; i++) {
		if( i == PQMASK_SE_SRCBUF_DEPTH_RESIZE && PQMASK_PROC_WIDTH == AI_DEPTHMAP_WIDTH && PQMASK_PROC_HEIGHT == AI_DEPTHMAP_HEIGHT ) {
			continue;
		}
		if( PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr == 0 ) {
			return RET_FAIL;
		}
	}

	// depth source image dma
	if( PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr == 0 ) {
		return RET_FAIL;
	}

	return RET_OK;
}

/**
 * @brief
 * free all the pqmask use dma
 */
void scalerPQMask_DmaFree(void) {
	int i=0, j=0;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	// module
	for(i=0; i<PQMASK_MODULE_NUM; i++) {
		for(j=0; j<PQMASK_MODULE_DMA_NUM; j++) {
			if( PQMaskBufMgr.ModuleDma[i][j].AddrInfo.VirtAddr != 0 ) {
				dvr_free((void *)(PQMaskBufMgr.ModuleDma[i][j].AddrInfo.VirtAddr));
				memset(&PQMaskBufMgr.ModuleDma[i][j].AddrInfo, 0, sizeof(PQMASK_MODULE_BUF_ADDRINFO_T));
			}
		}
	}

	// SE buffer
	for(i=0; i<PQMASK_SE_BUF_NUM; i++) {
		if( PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr != 0 ) {
			dvr_free((void *)(PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr));
			memset(&PQMaskBufMgr.SeBuffer[i].AddrInfo, 0, sizeof(PQMASK_MODULE_BUF_ADDRINFO_T));
		}
	}

	// depth source image dma
	if( PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr != 0 ) {
		dvr_free((void *)(PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr));
		memset(&PQMaskBufMgr.DepthSrcImg.AddrInfo, 0, sizeof(PQMASK_MODULE_BUF_ADDRINFO_T));
	}

	PQMaskBufMgr.Size = 0;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);
}

/**
 * @brief
 * allocate the necessary pqmask dma
 * @return int
 */
int scalerPQMask_DmaAllocate(void) {
	int i=0, j=0;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	scalerPQMask_DmaFree();
	PQMaskBufMgr.Size = 0;

	// module
	for(i=0; i<PQMASK_MODULE_NUM; i++) {
		for(j=0; j<PQMASK_MODULE_DMA_NUM; j++) {
			PQMaskBufMgr.ModuleDma[i][j].AddrInfo.Size = drvif_memory_get_data_align(PQMaskBufMgr.ModuleDma[i][j].BasicInfo.Width*PQMaskBufMgr.ModuleDma[i][j].BasicInfo.Height*PQMaskBufMgr.ModuleDma[i][j].BasicInfo.UnitBytes, ALIGN_SIZE);
			PQMaskBufMgr.ModuleDma[i][j].AddrInfo.VirtAddr = (unsigned long)dvr_malloc_uncached_specific(PQMaskBufMgr.ModuleDma[i][j].AddrInfo.Size, GFP_DCU1, (void **)&PQMaskBufMgr.ModuleDma[i][j].AddrInfo.VirtAddr_nonCached);
			if( PQMaskBufMgr.ModuleDma[i][j].AddrInfo.VirtAddr != 0 ) {
				PQMaskBufMgr.ModuleDma[i][j].AddrInfo.PhysAddr = (unsigned long)dvr_to_phys((void*)PQMaskBufMgr.ModuleDma[i][j].AddrInfo.VirtAddr);
				PQMaskBufMgr.Size = PQMaskBufMgr.Size + PQMaskBufMgr.ModuleDma[i][j].AddrInfo.Size;
				rtd_printk(KERN_INFO, TAG_NAME, "[%-*s] pa = 0x%lx, cva = 0x%lx, ncva = 0x%lx, sz = %ld bytes\n",
					PQMASK_STRING_LEN, PQMaskModuleName[i],
					PQMaskBufMgr.ModuleDma[i][j].AddrInfo.PhysAddr,
					PQMaskBufMgr.ModuleDma[i][j].AddrInfo.VirtAddr,
					PQMaskBufMgr.ModuleDma[i][j].AddrInfo.VirtAddr_nonCached,
					PQMaskBufMgr.ModuleDma[i][j].AddrInfo.Size
				);
			} else {
				rtd_printk(KERN_EMERG, TAG_NAME, "[%s] [%-*s] request dma fail\n", __FUNCTION__, PQMASK_STRING_LEN, PQMaskModuleName[i]);
				return RET_FAIL;
			}
		}
	}

	// se procedure
	for(i=0; i<PQMASK_SE_BUF_NUM; i++) {
		if( i == PQMASK_SE_SRCBUF_DEPTH_RESIZE && PQMASK_PROC_WIDTH == AI_DEPTHMAP_WIDTH && PQMASK_PROC_HEIGHT == AI_DEPTHMAP_HEIGHT ) {
			PQMaskBufMgr.SeBuffer[i].AddrInfo.Size = 0;
			PQMaskBufMgr.SeBuffer[i].AddrInfo.PhysAddr = 0;
			PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr = 0;
			PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr_nonCached = 0;
			rtd_printk(KERN_INFO, TAG_NAME, "[%-*s] depth sz = %dx%d, se proc sz = %dx%d, no need this buffer\n",
				PQMASK_STRING_LEN, PQMaskSEBufName[i], AI_DEPTHMAP_WIDTH, PQMASK_PROC_HEIGHT, PQMASK_PROC_WIDTH, PQMASK_PROC_HEIGHT
			);
			continue;
		}
		PQMaskBufMgr.SeBuffer[i].AddrInfo.Size = drvif_memory_get_data_align(PQMaskBufMgr.SeBuffer[i].BasicInfo.Width*PQMaskBufMgr.SeBuffer[i].BasicInfo.Height*PQMaskBufMgr.SeBuffer[i].BasicInfo.UnitBytes, ALIGN_SIZE);
		PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr = (unsigned long)dvr_malloc_uncached_specific(PQMaskBufMgr.SeBuffer[i].AddrInfo.Size, GFP_DCU1, (void **)&PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr_nonCached);
		if( PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr != 0 ) {
			PQMaskBufMgr.SeBuffer[i].AddrInfo.PhysAddr = (unsigned long)dvr_to_phys((void*)PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr);
			rtd_printk(KERN_INFO, TAG_NAME, "[%-*s] pa = 0x%lx, cva = 0x%lx, ncva = 0x%lx, sz = %ld bytes\n",
				PQMASK_STRING_LEN, PQMaskSEBufName[i],
				PQMaskBufMgr.SeBuffer[i].AddrInfo.PhysAddr,
				PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr,
				PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr_nonCached,
				PQMaskBufMgr.SeBuffer[i].AddrInfo.Size
			);
			PQMaskBufMgr.Size = PQMaskBufMgr.Size + PQMaskBufMgr.SeBuffer[i].AddrInfo.Size;
		} else {
			rtd_printk(KERN_EMERG, TAG_NAME, "[%s] [%-*s] request dma fail\n", __FUNCTION__, PQMASK_STRING_LEN, PQMaskSEBufName[i]);
			return RET_FAIL;
		}
	}

	// source image for depth histogram
	PQMaskBufMgr.DepthSrcImg.AddrInfo.Size = drvif_memory_get_data_align(PQMaskBufMgr.DepthSrcImg.BasicInfo.Width*PQMaskBufMgr.DepthSrcImg.BasicInfo.Height*PQMaskBufMgr.DepthSrcImg.BasicInfo.UnitBytes, ALIGN_SIZE);
	PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr = (unsigned long)dvr_malloc_uncached_specific(PQMaskBufMgr.DepthSrcImg.AddrInfo.Size, GFP_DCU1, (void **)&PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr_nonCached);
	if( PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr != 0 ) {
		PQMaskBufMgr.DepthSrcImg.AddrInfo.PhysAddr = (unsigned long)dvr_to_phys((void*)PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr);
		rtd_printk(KERN_INFO, TAG_NAME, "[%-*s] pa = 0x%lx, cva = 0x%lx, ncva = 0x%lx, sz = %ld bytes\n",
			PQMASK_STRING_LEN, "DepSrcImg",
			PQMaskBufMgr.DepthSrcImg.AddrInfo.PhysAddr,
			PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr,
			PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr_nonCached,
			PQMaskBufMgr.DepthSrcImg.AddrInfo.Size
		);
		PQMaskBufMgr.Size = PQMaskBufMgr.Size + PQMaskBufMgr.DepthSrcImg.AddrInfo.Size;
	} else {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] [%-*s] request dma fail\n", __FUNCTION__, PQMASK_STRING_LEN, "SrcImg");
		return RET_FAIL;
	}

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] total size = %ld bytes\n", __FUNCTION__, PQMaskBufMgr.Size);

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);

	return RET_OK;
}

/**
 * @brief
 * fill all the buffer with user specific value
 * @return int
 */
int scalerPQMask_ResetDma(void) {
	int i=0, j=0;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	// module
	for(i=0; i<PQMASK_MODULE_NUM; i++) {
		for(j=0; j<PQMASK_MODULE_DMA_NUM; j++) {
			if( PQMaskBufMgr.ModuleDma[i][j].AddrInfo.VirtAddr != 0 ) {
				scalerPQMask_Memset(
					(void *)PQMaskBufMgr.ModuleDma[i][j].AddrInfo.VirtAddr,
					(void *)PQMaskBufMgr.ModuleDma[i][j].AddrInfo.PhysAddr,
					PQMaskBufMgr.ModuleDma[i][j].BasicInfo.RstValue,
					PQMaskBufMgr.ModuleDma[i][j].AddrInfo.Size
				);
			}
		}
	}

	// SE buffer
	for(i=0; i<PQMASK_SE_BUF_NUM; i++) {
		if( PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr != 0 ) {
			scalerPQMask_Memset(
				(void *)PQMaskBufMgr.SeBuffer[i].AddrInfo.VirtAddr,
				(void *)PQMaskBufMgr.SeBuffer[i].AddrInfo.PhysAddr,
				PQMaskBufMgr.SeBuffer[i].BasicInfo.RstValue,
				PQMaskBufMgr.SeBuffer[i].AddrInfo.Size
			);
		}
	}

	// depth source image dma
	if( PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr != 0 ) {
		scalerPQMask_Memset(
			(void *)PQMaskBufMgr.DepthSrcImg.AddrInfo.VirtAddr,
			(void *)PQMaskBufMgr.DepthSrcImg.AddrInfo.PhysAddr,
			PQMaskBufMgr.DepthSrcImg.BasicInfo.RstValue,
			PQMaskBufMgr.DepthSrcImg.AddrInfo.Size
		);
	}

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);

	return RET_OK;
}

/**
 * @brief
 * start to process the following procedure when
 */
void scalerPQMask_postprocessing_entry(void) {

	// common parameters
	int i=0, j=0;
	static unsigned int CurTime = 0;
	static unsigned int PreTime = 0;
	static unsigned int PerfA2KPreFrm = 0;
	const int semantic_idx = VIP_NN_BUFFER_480FULL;
	const int depth_idx = VIP_NN_BUFFER_192FULL;
	VIP_NN_CTRL *pNN = scalerAI_Access_NN_CTRL_STRUCT();
	PQMaskFlag *pPQMaskFlag = NULL;
	PQMASK_COLOR_MGR_T *pPQMaskColorMgr = scalerPQMaskColor_GetStruct(PQMASK_COLORSTRUCT_MGR);

	// conditions
	if( pNN->NN_flag_Addr[semantic_idx].pVir_addr_align == NULL ||
		pNN->NN_info_Addr[semantic_idx].pVir_addr_align == NULL ||
		pNN->NN_info_Addr[depth_idx].pVir_addr_align == NULL ) {
		// rtd_printk(KERN_EMERG, TAG_NAME, "[%s] ptr null\n", __FUNCTION__);
		return;
	}

	if( aipq_mode.pqmask_mode == 0 ) {
		// rtd_printk(KERN_EMERG, TAG_NAME, "[%s] pqmask mode = 0\n", __FUNCTION__);
		return;
	}

	// error checking and handler
	scalerPQMask_ErrorHandler();

	if( scalerPQMask_FpsCtrl(&CurTime, &PreTime, PQMaskGlbMgr.Ctrl.LimitFPS, 0, TAG_NAME) != true ) {
		return;
	}

	if( pPQMaskColorMgr->Param.FusionRatio_Dep != 0 && PQMaskGlbMgr.Ctrl.bIsDepthReady == 0 ) {
		// rtd_printk(KERN_EMERG, TAG_NAME, "[%s] fusion depth = %d, isReady = 0\n", __FUNCTION__, pPQMaskColorMgr->Param.FusionRatio_Dep);
		return;
	}

	if( pPQMaskColorMgr->Param.FusionRatio_Sem != 0 && PQMaskGlbMgr.Ctrl.bIsSemReady == 0 ) {
		// rtd_printk(KERN_EMERG, TAG_NAME, "[%s] fusion sem = %d, isReady = 0\n", __FUNCTION__, pPQMaskColorMgr->Param.FusionRatio_Sem);
		return;
	}

	// start flow
	pPQMaskFlag = (&((PQMaskFlag*)(pNN->NN_flag_Addr[semantic_idx].pVir_addr_align))[PQMASK_TUNNEL_A2K]);

	// record the time when kernel first get the info from ap for each timestamp
	if( pPQMaskFlag->Update == 1 && PerfA2KPreFrm != pPQMaskFlag->TimeStamp  ) {
		PerfA2KPreFrm = pPQMaskFlag->TimeStamp;
		PQMaskPerfMgr[pPQMaskFlag->TimeStamp].Ap2KernelInfo = drvif_Get_90k_Lo_clk();
		memcpy((void *)&PQMaskPerfMgr[pPQMaskFlag->TimeStamp].ModelPerf, (void *)&pPQMaskFlag->Perf, sizeof(PQMaskModelPerf));
	}

	// start processing
	if( PQMaskGlbMgr.Status.PQMaskStatus == PQMASK_STAT_READY )	{

		PreTime = CurTime;
		PQMaskGlbMgr.Status.PQMaskStatus = PQMASK_STAT_PREPARE;
		PQMaskGlbMgr.Ctrl.bSemNewInfo = ( pPQMaskFlag->Update != 0 )? 1 : 0;

		PQMASKPrintf_SEtask("[%s][%s] start process at infoTsp = %d, new frm = %d\n", TAG_NAME, __FUNCTION__, (pPQMaskFlag->TimeStamp), PQMaskGlbMgr.Ctrl.bSemNewInfo);

		PQMaskGlbMgr.Status.SeStatus.ProcTsp = pPQMaskFlag->TimeStamp;
		if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {
			PQMaskPerfMgr[PQMaskGlbMgr.Status.SeStatus.ProcTsp].KernelPrepStart = drvif_Get_90k_Lo_clk();
		}

		// copy semantic data from co-buffer
		if( PQMaskGlbMgr.Ctrl.bSemNewInfo != 0 ) {

			scalerPQMask_Memcpy(
				(void *)PQMaskBufMgr.SeBuffer[PQMASK_SE_SRCBUF_SEMANTIC].AddrInfo.VirtAddr, (void *)PQMaskBufMgr.SeBuffer[PQMASK_SE_SRCBUF_SEMANTIC].AddrInfo.PhysAddr, 1,
				(void *)pNN->NN_info_Addr[semantic_idx].pVir_addr_align, 0, 1,
				AI_SEMANTIC_OUTPUT_WIDTH*AI_SEMANTIC_OUTPUT_HEIGHT*sizeof(char)
			);

			if( pPQMaskColorMgr->ImgBySemanticHist[0][0].Info.enable != 0 ) {
				// TODO : Potential risk when size mismatch
				for(i=0; i<PQMASK_LABEL_NUM; i++){
					for(j=0; j<PQMASK_IMGCH_NUM; j++){
						scalerPQMask_Memcpy(
							(void *)(pPQMaskColorMgr->ImgBySemanticHist[i][j].Data.Histogram), 0, 0,
							(void *)(&pPQMaskFlag->Histogram[i*PQMASK_IMGCH_NUM*AI_SEM_HIST_BIN+j*AI_SEM_HIST_BIN]), 0, 1,
							(AI_SEM_HIST_BIN>=PQMASK_HISTO_BIN) ? (PQMASK_HISTO_BIN*sizeof(unsigned int)) : (AI_SEM_HIST_BIN*sizeof(unsigned int))
						);
					}
				}
			}
		}

		// copy depth data from co-buffer
		// NOTE. condition may cause if ap speed is faster than kernel
		if( PQMaskGlbMgr.Ctrl.bDepNewInfo != 0 ) {

			scalerPQMask_Memcpy(
				(void *)PQMaskBufMgr.SeBuffer[PQMASK_SE_SRCBUF_DEPTH].AddrInfo.VirtAddr, (void *)PQMaskBufMgr.SeBuffer[PQMASK_SE_SRCBUF_DEPTH].AddrInfo.PhysAddr, 1,
				(void *) pNN->NN_info_Addr[depth_idx].pVir_addr_align, 0, 1,
				AI_DEPTHMAP_WIDTH*AI_DEPTHMAP_HEIGHT*sizeof(char)
			);

			PQMaskGlbMgr.Status.bDepNeedUptHist = 1;
			PQMaskGlbMgr.Ctrl.bDepNewInfo = 0;

			if( PQMaskMonitorMgr.Status.Depth_Counter == 0 ) {
				PQMaskMonitorMgr.Status.Depth_MeasureStartTime = drvif_Get_90k_Lo_clk();
			} else {
				PQMaskMonitorMgr.Status.Depth_MeauserEndTime = drvif_Get_90k_Lo_clk();
			}
		}

		//
		if( PQMaskGlbMgr.Status.PQMaskStatus == PQMASK_STAT_PREPARE ) {
			PQMaskGlbMgr.Status.PQMaskStatus = PQMASK_STAT_SE_PROC;
			pPQMaskFlag->Update = 0;
			scalerPQMaskColor_SceneChange();
			scalerPQMaskColor_ConstructPalette();

			PQMaskGlbMgr.Status.SeStatus.StartTime = drvif_Get_90k_Lo_clk();
			PQMaskMonitorMgr.Status.SECurProcStart = drvif_Get_90k_Lo_clk();
			scalerPQMask_SETask(0);
			PQMaskMonitorMgr.Status.SECurProcEnd = drvif_Get_90k_Lo_clk();
		}

	}

	// online monitor checking
	scalerPQMask_OnlineMonitor();

}

/**
 * @brief
 * pqmask post procssing in i domain
 */
void scalerPQMask_postprocessing_idomain(void) {

	int i=0;
	int moduleSel = 0;

	if( PQMaskGlbMgr.Status.PQMaskStatus != PQMASK_STAT_UNKOWN &&
		PQMaskGlbMgr.Status.PQMaskStatus != PQMASK_STAT_STANDBY &&
		PQMaskGlbMgr.Status.PQMaskStatus != PQMASK_STAT_ERR &&
		PQMaskGlbMgr.Status.PQMaskStatus < PQMASK_STAT_MAX )
	{

		for( i=0; i<(sizeof(IDomainModuleGrp)/sizeof(unsigned int)); i++ ) {
			moduleSel = IDomainModuleGrp[i];
			// check need to move or not
			ModuleDmaIdx[moduleSel][DMA_IDX_CUR] = PQMaskRingBuf[moduleSel].FuncGetIdx((void *)&PQMaskRingBuf[moduleSel], VIP_RINGBUF_PTR_READ);
			if( ModuleDmaIdx[moduleSel][DMA_IDX_CUR] != ModuleDmaIdx[moduleSel][DMA_IDX_NXT] ) {
				PQMaskRingBuf[moduleSel].FuncSetIdx((void *)&PQMaskRingBuf[moduleSel], VIP_RINGBUF_PTR_READ);
			}
			// check next frame setting
			ModuleDmaIdx[moduleSel][DMA_IDX_NXT] = PQMaskRingBuf[moduleSel].FuncPeekIdx((void *)&PQMaskRingBuf[moduleSel], VIP_RINGBUF_PTR_READ);
			scalerPQMask_SetModuleAddr(moduleSel, PQMaskBufMgr.ModuleDma[moduleSel][ModuleDmaIdx[moduleSel][DMA_IDX_NXT]].AddrInfo.PhysAddr);
			PQMASKPrintf_Idomain("[%s][%-*s] cur = %d, nxt = %d\n",
				TAG_NAME, PQMASK_STRING_LEN, PQMaskRingBuf[moduleSel].Name, ModuleDmaIdx[moduleSel][DMA_IDX_CUR], ModuleDmaIdx[moduleSel][DMA_IDX_NXT]);
		}
		drvif_module_set_pqmask_idomain_db_apply();
	}
}

/**
 * @brief
 * pqmask post processing in d domain
 */
void scalerPQMask_postprocessing_ddomain(void) {

	int i=0;
	int moduleSel = 0;

	if( PQMaskGlbMgr.Status.PQMaskStatus != PQMASK_STAT_UNKOWN &&
		PQMaskGlbMgr.Status.PQMaskStatus != PQMASK_STAT_STANDBY &&
		PQMaskGlbMgr.Status.PQMaskStatus != PQMASK_STAT_ERR &&
		PQMaskGlbMgr.Status.PQMaskStatus < PQMASK_STAT_MAX )
	{
		for( i=0; i<(sizeof(DDomainModuleGrp)/sizeof(unsigned int)); i++ ) {
			moduleSel = DDomainModuleGrp[i];
			// check need to move or not
			ModuleDmaIdx[moduleSel][DMA_IDX_CUR] = PQMaskRingBuf[moduleSel].FuncGetIdx((void *)&PQMaskRingBuf[moduleSel], VIP_RINGBUF_PTR_READ);
			if( ModuleDmaIdx[moduleSel][DMA_IDX_CUR] != ModuleDmaIdx[moduleSel][DMA_IDX_NXT] ) {
				PQMaskRingBuf[moduleSel].FuncSetIdx((void *)&PQMaskRingBuf[moduleSel], VIP_RINGBUF_PTR_READ);
			}
			// check next frame setting
			ModuleDmaIdx[moduleSel][DMA_IDX_NXT] = PQMaskRingBuf[moduleSel].FuncPeekIdx((void *)&PQMaskRingBuf[moduleSel], VIP_RINGBUF_PTR_READ);
			scalerPQMask_SetModuleAddr(moduleSel, PQMaskBufMgr.ModuleDma[moduleSel][ModuleDmaIdx[moduleSel][DMA_IDX_NXT]].AddrInfo.PhysAddr);
			PQMASKPrintf_Ddomain("[%s][%-*s] cur = %d, nxt = %d\n",
				TAG_NAME, PQMASK_STRING_LEN, PQMaskRingBuf[moduleSel].Name, ModuleDmaIdx[moduleSel][DMA_IDX_CUR], ModuleDmaIdx[moduleSel][DMA_IDX_NXT]);
		}
		drvif_module_set_pqmask_ddomain_db_apply();
	}
	PQMaskPrintCount++;
}

/**
 * @brief
 * check all the pqmask status and try to recover if there are any errors
 * if recover time exceed a user-config times, stay in error status
 */
void scalerPQMask_ErrorHandler(void) {

	int RecoverStatus = RET_OK;

	if( PQMaskGlbMgr.Status.PQMaskStatus == PQMASK_STAT_UNKOWN || PQMaskGlbMgr.Status.PQMaskStatus == PQMASK_STAT_STANDBY )
		return;

	if( PQMaskGlbMgr.Status.PQMaskStatus != PQMASK_STAT_ERR ) {

		// only check status
		if( scalerPQMask_CheckDMA() != RET_OK ) {
			PQMaskGlbMgr.Status.PQMaskStatus = PQMASK_STAT_ERR;
			rtd_printk(KERN_EMERG, TAG_NAME, "[%s] check dma error, status to %s\n", __FUNCTION__, PQMaskStatusStr[PQMaskGlbMgr.Status.PQMaskStatus]);
		}
		if( scalerPQMask_CheckResolution() != RET_OK ) {
			PQMaskGlbMgr.Status.PQMaskStatus = PQMASK_STAT_ERR;
			rtd_printk(KERN_EMERG, TAG_NAME, "[%s] check module io size error, status to %s\n", __FUNCTION__, PQMaskStatusStr[PQMaskGlbMgr.Status.PQMaskStatus]);
		}

	} else if( PQMaskGlbMgr.Status.PQMaskStatus == PQMASK_STAT_ERR ) {

		// try to recover
		if( PQMaskGlbMgr.Status.ErrRecoverTimes > 0 ) {

			// check dma
			if( scalerPQMask_CheckDMA() != RET_OK ) {
				if( scalerPQMask_DmaAllocate() != RET_OK ) {
					scalerPQMask_DmaFree();
					RecoverStatus = RET_FAIL;
				} else {
					rtd_printk(KERN_EMERG, TAG_NAME, "[%s] recover from dma error done\n", __FUNCTION__);
				}
			}

			// check resolution
			if( scalerPQMask_CheckResolution() != RET_OK ) {
				scalerPQMask_InitModuleConfig();
				rtd_printk(KERN_EMERG, TAG_NAME, "[%s] recover from module size error done\n", __FUNCTION__);
			}

			// summary the status
			if( RecoverStatus != RET_OK ) {
				rtd_printk(KERN_EMERG, TAG_NAME, "[%s] recover fail with times %d\n", __FUNCTION__, PQMaskGlbMgr.Status.ErrRecoverTimes);
				PQMaskGlbMgr.Status.ErrRecoverTimes--;
			} else {
				PQMaskGlbMgr.Status.PQMaskStatus = PQMASK_STAT_READY;
				PQMaskGlbMgr.Status.ErrRecoverTimes = PQMaskGlbMgr.Ctrl.ErrRecoverRstTimes;
				rtd_printk(KERN_EMERG, TAG_NAME, "[%s] recover success\n", __FUNCTION__);
			}

		}
	}
}

/**
 * @brief
 * fill the semantic data to OSD buffer, input OSD size is fixed to 480x256
 * @param dstBuf destination buffer for whole OSD
 * @param hRatio full OSD buffer width(480) / real semantic data width
 * @param vRatio full OSD buffer height(256) / real semantic data height
 */
void scalerPQMask_GetSemanticARGBBuffer(unsigned char *dstBuf, unsigned char hRatio, unsigned char vRatio, unsigned char Alpha) {

	const unsigned int OSD_WIDTH = 480;
	const unsigned int OSD_HEIGHT = 256;
	int py=0, px=0;
	int xStart = 0;
	int yStart = 0;
	int labelValue = 0;
	unsigned char *pLabelBuffer = (unsigned char *)(scalerAI_Access_NN_CTRL_STRUCT()->NN_info_Addr[VIP_NN_BUFFER_480FULL].pVir_addr_align);

	if( pLabelBuffer == NULL )
		return;
	if( AI_SEMANTIC_OUTPUT_WIDTH/hRatio > OSD_WIDTH || AI_SEMANTIC_OUTPUT_HEIGHT/vRatio > OSD_HEIGHT )
		return;

	// -- show at left top
	// xStart = 0;
	// yStart = 0;
	// -- show at left bot
	xStart = 0;
	yStart = OSD_HEIGHT-(AI_SEMANTIC_OUTPUT_HEIGHT/vRatio);
	// -- show at right top
	// xStart = OSD_WIDTH-1-(AI_SEMANTIC_OUTPUT_WIDTH/hRatio);
	// yStart = 0;
	// -- show at right bot
	// xStart = OSD_WIDTH-1-(AI_SEMANTIC_OUTPUT_WIDTH/hRatio);
	// yStart = OSD_HEIGHT-1-(AI_SEMANTIC_OUTPUT_HEIGHT/vRatio);

	for(py=0; py<(AI_SEMANTIC_OUTPUT_HEIGHT/vRatio); py++) {
		for(px=0; px<(AI_SEMANTIC_OUTPUT_WIDTH/hRatio); px++) {

			labelValue = pLabelBuffer[(py*vRatio)*AI_SEMANTIC_OUTPUT_WIDTH+(px*hRatio)];

			if( labelValue >= PQMASK_LABEL_NUM )
				labelValue = 0;

			dstBuf[((py+yStart)*OSD_WIDTH*4)+((px+xStart)*4)+0] = Alpha; // alpha
			dstBuf[((py+yStart)*OSD_WIDTH*4)+((px+xStart)*4)+1] = OSDLabelColor[labelValue][0]; // red channel
			dstBuf[((py+yStart)*OSD_WIDTH*4)+((px+xStart)*4)+2] = OSDLabelColor[labelValue][1]; // green channel
			dstBuf[((py+yStart)*OSD_WIDTH*4)+((px+xStart)*4)+3] = OSDLabelColor[labelValue][2]; // blue channel
		}
	}
}

/**
 * @brief 
 * draw the semantic result on osd
 * @param dstBuf 
 * @param bufferWidth 
 * @param xStart 
 * @param yStart 
 * @param hRatio 
 * @param vRatio 
 * @param Alpha 
 */
void scalerPQMask_GetSemanticARGBBuffer_Coordinate(
	unsigned char *dstBuf, unsigned int bufferWidth,
	unsigned int xStart, unsigned int yStart,
	unsigned char hRatio, unsigned char vRatio, unsigned char Alpha ) {

	int py=0, px=0;
	int labelValue = 0;
	unsigned char *pLabelBuffer = (unsigned char *)(scalerAI_Access_NN_CTRL_STRUCT()->NN_info_Addr[VIP_NN_BUFFER_480FULL].pVir_addr_align);

	if( pLabelBuffer == NULL )
		return;
	if( hRatio==0 || vRatio==0 )
		return;

	for(py=0; py<(AI_SEMANTIC_OUTPUT_HEIGHT/vRatio); py++) {
		for(px=0; px<(AI_SEMANTIC_OUTPUT_WIDTH/hRatio); px++) {

			labelValue = pLabelBuffer[(py*vRatio)*AI_SEMANTIC_OUTPUT_WIDTH+(px*hRatio)];

			if( labelValue >= PQMASK_LABEL_NUM )
				labelValue = 0;

			dstBuf[ ( ((py+yStart)*bufferWidth)+(px+xStart) )*4 + 0 ] = Alpha; // alpha
			dstBuf[ ( ((py+yStart)*bufferWidth)+(px+xStart) )*4 + 1 ] = OSDLabelColor[labelValue][0]; // red channel
			dstBuf[ ( ((py+yStart)*bufferWidth)+(px+xStart) )*4 + 2 ] = OSDLabelColor[labelValue][1]; // green channel
			dstBuf[ ( ((py+yStart)*bufferWidth)+(px+xStart) )*4 + 3 ] = OSDLabelColor[labelValue][2]; // blue channel

		}
	}

}

/****************************** Module config functions ******************************/

/**
 * @brief
 * Prepare the structure and call the function to write the setting to HW
 */
void scalerPQMask_InitModuleConfig(void) {

	// PQMASK_IMG_SIZE_T IdomainSize;
	unsigned int IdomainWidth = Scaler_DispGetInputInfo(SLR_INPUT_DI_WID); //Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID_PRE); // ?
	unsigned int IdomainHeight = Scaler_DispGetInputInfo(SLR_INPUT_DI_LEN); //Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_LEN_PRE); // ?
	unsigned int DdomainWidth = Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
	unsigned int DdomainHeight = Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN);
	int i=0;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	// print the scaler information to choose
	// rtd_printk(KERN_INFO, TAG_NAME, "SLR_INPUT_SDNRH_ACT_WID_PRE = %d\n", Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID_PRE));
	// rtd_printk(KERN_INFO, TAG_NAME, "SLR_INPUT_DI_WID = %d\n", Scaler_DispGetInputInfo(SLR_INPUT_DI_WID));
	// rtd_printk(KERN_INFO, TAG_NAME, "SLR_INPUT_IPH_ACT_WID = %d\n", Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID));
	// rtd_printk(KERN_INFO, TAG_NAME, "SLR_INPUT_MEM_ACT_WID = %d\n", Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID));

	// prepare the parameters
	// drvif_color_get_pqmask_idomain_size_rd_status(&IdomainSize);
	// IdomainWidth = IdomainSize.Width;
	// IdomainHeight = IdomainSize.Height;

	PQMaskBlkModuleCfg[PQMASK_BLKMOD_SPNR].IoSize.Output.Width = IdomainWidth;
	PQMaskBlkModuleCfg[PQMASK_BLKMOD_SPNR].IoSize.Output.Height = IdomainHeight;
	PQMaskBlkModuleCfg[PQMASK_BLKMOD_SHARPNESS].IoSize.Output.Width = DdomainWidth;
	PQMaskBlkModuleCfg[PQMASK_BLKMOD_SHARPNESS].IoSize.Output.Height = DdomainHeight;
	PQMaskBlkModuleCfg[PQMASK_BLKMOD_ICM].IoSize.Output.Width = DdomainWidth;
	PQMaskBlkModuleCfg[PQMASK_BLKMOD_ICM].IoSize.Output.Height = DdomainHeight;


	// enable double buffer
	drvif_module_set_pqmask_idomain_db_enable(1);
	drvif_module_set_pqmask_ddomain_db_enable(1);

	// scaling and size setting
	// dma configuration (need size information)
	for(i=0; i<PQMASK_BLKMOD_NUM; i++) {
		scalerPQMask_SetBlkModuleIoSize(i);
		scalerPQMask_SetBlkModuleDmaConfig(i);
	}

	// initial individual module pqmask weight plane address
	for(i=0; i<PQMASK_MODULE_NUM; i++) {
		scalerPQMask_SetModuleAddr(i, PQMaskBufMgr.ModuleDma[i][PQMaskRingBuf[i].FuncGetIdx((void *)&PQMaskRingBuf[i], VIP_RINGBUF_PTR_READ)].AddrInfo.PhysAddr);
	}

	drvif_module_set_pqmask_idomain_db_apply();
	drvif_module_set_pqmask_ddomain_db_apply();
	drvif_module_wait_pqmask_idomain_db_apply_done(40);
	drvif_module_wait_pqmask_ddomain_db_apply_done(40);

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);

}

/**
 * @brief
 * disable all the pqmask dma
 */
void scalerPQMask_disableModuleDMA(void) {

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	drvif_color_set_pqmask_idomain_DMA_enable(0);
	drvif_color_set_pqmask_idomain_SNR_enable(0);
	drvif_color_set_pqmask_idomain_Decontour_enable(0);
	drvif_color_set_pqmask_shp_enable(0, 0);
	drvif_color_set_pqmask_icm_enable(0, 0, 0);
	drvif_module_set_pqmask_idomain_db_apply();
	drvif_module_set_pqmask_ddomain_db_apply();
	drvif_module_wait_pqmask_idomain_db_apply_done(40);
	drvif_module_wait_pqmask_ddomain_db_apply_done(40);

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);
}

/**
 * @brief
 * set the pqmask weight plane address for selected module
 * @param ModuleSel which single model
 * @param TargetAddr the full address, the driver will only use 16Byte align address
 */
void scalerPQMask_SetModuleAddr(unsigned char ModuleSel, unsigned long TargetAddr) {

	// rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	switch(ModuleSel) {
		case PQMASK_MODULE_NR:
			drvif_color_set_pqmask_idomain_DMA_SNR_Addr(TargetAddr>>4);
			break;
		case PQMASK_MODULE_DECONT:
			drvif_color_set_pqmask_idomain_DMA_Decontour_Addr(TargetAddr>>4);
			break;
		case PQMASK_MODULE_EDGE:
			drvif_color_set_pqmask_shp_Edge_Addr(TargetAddr>>4);
			break;
		case PQMASK_MODULE_TEXTURE:
			drvif_color_set_pqmask_shp_Text_Addr(TargetAddr>>4);
			break;
		case PQMASK_MODULE_HUE:
			drvif_color_set_pqmask_icm_HUE_Addr(TargetAddr>>4);
			break;
		case PQMASK_MODULE_SAT:
			drvif_color_set_pqmask_icm_SAT_Addr(TargetAddr>>4);
			break;
		case PQMASK_MODULE_INT:
			drvif_color_set_pqmask_icm_INT_Addr(TargetAddr>>4);
			break;
		default:
			rtd_printk(KERN_ERR, TAG_NAME, "[%s] index %d overflow\n", __FUNCTION__, ModuleSel);
			break;
	}

	// rtd_printk(KERN_INFO, TAG_NAME, "[%s] set address = 0x%lx\n", PQMaskModuleName[ModuleSel], TargetAddr);

	// rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);
}

/**
 * @brief
 * put the setting to register according to PQMaskBlkModuleCfg structure
 * @param BlkModuleSel which block module
 */
void scalerPQMask_SetBlkModuleDmaConfig(unsigned char BlkModuleSel) {

	unsigned int HSum = 0;
	PQMASK_DMA_CONFIG_T DmaCfg;
	PQMASK_MODULE_DMA_CONFIG_T ModuleDmaCfg;
	PQMASK_COLOR_MGR_T *pPQMaskColorMgr = (PQMASK_COLOR_MGR_T *)scalerPQMaskColor_GetStruct(PQMASK_COLORSTRUCT_MGR);

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	DmaCfg.SwapType = PQMASK_DMA_NONE_SWP;
	DmaCfg.AddrUpLimit = 0x00000000;
	DmaCfg.AddrLowLimit = 0xffffffff;

	HSum = (PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Input.HorSta + PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Input.Width)*sizeof(unsigned char)*8;
	ModuleDmaCfg.DataBurstLen = ((HSum & 0x7f) == 0)? (HSum>>7) : ((HSum>>7) + 1);
	ModuleDmaCfg.LineStep = (PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Input.Width)>>4; // LineAddrOffset (align 16Byte)

	switch(BlkModuleSel) {
		case PQMASK_BLKMOD_SPNR:
			scalerPQMask_GetDMALimit(PQMASK_DOMAIN_I, &DmaCfg);
			drvif_color_set_pqmask_idomain_DMA_config(&DmaCfg, &ModuleDmaCfg);
			drvif_color_set_pqmask_idomain_DMA_enable(1);
			if( pPQMaskColorMgr->ModuleSetting.Nr.bNrEnable != 0 ) {
				drvif_color_set_pqmask_idomain_SNR_enable(1);
			} else {
				drvif_color_set_pqmask_idomain_SNR_enable(0);
			}
			if( pPQMaskColorMgr->ModuleSetting.Nr.bIBlockDecontEnable != 0 || pPQMaskColorMgr->ModuleSetting.Nr.bIDecontEnable != 0 ) {
				drvif_color_set_pqmask_idomain_Decontour_enable(1);
			} else {
				drvif_color_set_pqmask_idomain_Decontour_enable(0);
			}
			break;
		case PQMASK_BLKMOD_SHARPNESS:
			scalerPQMask_GetDMALimit(PQMASK_DOMAIN_D, &DmaCfg);
			drvif_color_set_pqmask_ddomain_DMA_config(&DmaCfg);
			drvif_color_set_pqmask_ddomain_DMA_enable(1);
			drvif_color_set_pqmask_shp_DMA_config(&ModuleDmaCfg);
			drvif_color_set_pqmask_shp_enable(pPQMaskColorMgr->ModuleSetting.Sharpness.bLabelSharpnessEnable, pPQMaskColorMgr->ModuleSetting.Sharpness.bLabelSharpnessEnable);
			break;
		case PQMASK_BLKMOD_ICM:
			scalerPQMask_GetDMALimit(PQMASK_DOMAIN_D, &DmaCfg);
			drvif_color_set_pqmask_ddomain_DMA_config(&DmaCfg);
			drvif_color_set_pqmask_ddomain_DMA_enable(1);
			drvif_color_set_pqmask_icm_DMA_config(&ModuleDmaCfg);
			drvif_color_set_pqmask_icm_enable(pPQMaskColorMgr->ModuleSetting.ICM.bHueEnable, pPQMaskColorMgr->ModuleSetting.ICM.bSatEnable, pPQMaskColorMgr->ModuleSetting.ICM.bIntEnable);
			break;
		default:
			rtd_printk(KERN_ERR, TAG_NAME, "[%s] index %d overflow\n", __FUNCTION__, BlkModuleSel);
			break;
	}

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] AddrLimit low:0x%x -up:0x%x, swap = %d\n", PQMaskBlockModuleName[BlkModuleSel], DmaCfg.AddrLowLimit<<4, DmaCfg.AddrUpLimit<<4, DmaCfg.SwapType);
	rtd_printk(KERN_INFO, TAG_NAME, "[%s] BurstLen = %d, LineStep = %d\n", PQMaskBlockModuleName[BlkModuleSel], ModuleDmaCfg.DataBurstLen, ModuleDmaCfg.LineStep<<4);

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);
}

/**
 * @brief
 * Initialize the Block Module (contains multiple pqmask weight plane) size and scaling setting
 * @param BlkModuleSel  which block module
 */
void scalerPQMask_SetBlkModuleIoSize(unsigned char BlkModuleSel) {

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	if( PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Output.Width == 0 || PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Output.Height == 0 ) {
		rtd_printk(KERN_ERR, TAG_NAME, "[%s] output width = %d, height = %d error\n",
			PQMaskBlockModuleName[BlkModuleSel],
			PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Output.Width, PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Output.Height
		);
		return;
	}

	PQMaskBlkModuleCfg[BlkModuleSel].Scale.Hor.Factor = (PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Input.Width << 20)  / PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Output.Width;
	PQMaskBlkModuleCfg[BlkModuleSel].Scale.Ver.Factor = (PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Input.Height << 20) / PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Output.Height;

	switch(BlkModuleSel) {
		case PQMASK_BLKMOD_SPNR:
			drvif_color_set_pqmask_idomain_size(&PQMaskBlkModuleCfg[BlkModuleSel].IoSize);
			drvif_color_set_pqmask_idomain_scaling(&PQMaskBlkModuleCfg[BlkModuleSel].Scale);
			drvif_color_set_pqmask_idomain_scaling_enable(1, 1);
			break;
		case PQMASK_BLKMOD_SHARPNESS:
			drvif_color_set_pqmask_shp_size(&PQMaskBlkModuleCfg[BlkModuleSel].IoSize);
			drvif_color_set_pqmask_shp_scaling(&PQMaskBlkModuleCfg[BlkModuleSel].Scale);
			drvif_color_set_pqmask_shp_scaling_enable(1, 1);
			break;
		case PQMASK_BLKMOD_ICM:
			drvif_color_set_pqmask_icm_size(&PQMaskBlkModuleCfg[BlkModuleSel].IoSize);
			drvif_color_set_pqmask_icm_scaling(&PQMaskBlkModuleCfg[BlkModuleSel].Scale);
			drvif_color_set_pqmask_icm_scaling_enable(1, 1);
			break;
		default:
			rtd_printk(KERN_ERR, TAG_NAME, "[%s] index %d overflow\n", __FUNCTION__, BlkModuleSel);
			break;
	}

	if( BlkModuleSel < PQMASK_BLKMOD_NUM ) {
		rtd_printk(KERN_INFO, TAG_NAME, "[%s] input = %d x %d, sta = %d/%d, output = %d x %d, sta = %d/%d\n",
			PQMaskBlockModuleName[BlkModuleSel],
			PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Input.Width, PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Input.Height,
			PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Input.HorSta, PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Input.VerSta,
			PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Output.Width, PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Output.Height,
			PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Output.HorSta, PQMaskBlkModuleCfg[BlkModuleSel].IoSize.Output.VerSta
		);
		rtd_printk(KERN_INFO, TAG_NAME, "[%s] hor fac = %d, ini = %d, ver fac = %d, ini = %d\n",
			PQMaskBlockModuleName[BlkModuleSel],
			PQMaskBlkModuleCfg[BlkModuleSel].Scale.Hor.Factor, PQMaskBlkModuleCfg[BlkModuleSel].Scale.Hor.InitPhase,
			PQMaskBlkModuleCfg[BlkModuleSel].Scale.Ver.Factor, PQMaskBlkModuleCfg[BlkModuleSel].Scale.Ver.InitPhase
		);
	}

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);

}

/**
 * @brief
 * calculate the dma up/low address limit
 * @param domain I or D domain
 * @param DmaCfg the limit value in structure will be calculated
 */
void scalerPQMask_GetDMALimit(unsigned char domain, PQMASK_DMA_CONFIG_T *DmaCfg) {

	int i=0, j=0;

	// idomain
	if( domain == PQMASK_DOMAIN_I ) {
		for(i=PQMASK_MODULE_NR; i<=PQMASK_MODULE_DECONT; i++) {
			for(j=0; j<PQMASK_MODULE_DMA_NUM; j++) {
				if( DmaCfg->AddrUpLimit <= PQMaskBufMgr.ModuleDma[i][j].AddrInfo.PhysAddr) {
					DmaCfg->AddrUpLimit = PQMaskBufMgr.ModuleDma[i][j].AddrInfo.PhysAddr;
				}
				if( DmaCfg->AddrLowLimit >= PQMaskBufMgr.ModuleDma[i][j].AddrInfo.PhysAddr) {
					DmaCfg->AddrLowLimit = PQMaskBufMgr.ModuleDma[i][j].AddrInfo.PhysAddr;
				}
			}
		}
	} else if( domain == PQMASK_DOMAIN_D ) {
		for(i=PQMASK_MODULE_EDGE; i<=PQMASK_MODULE_INT; i++) {
			for(j=0; j<PQMASK_MODULE_DMA_NUM; j++) {
				if( DmaCfg->AddrUpLimit <= PQMaskBufMgr.ModuleDma[i][j].AddrInfo.PhysAddr) {
					DmaCfg->AddrUpLimit = PQMaskBufMgr.ModuleDma[i][j].AddrInfo.PhysAddr;
				}
				if( DmaCfg->AddrLowLimit >= PQMaskBufMgr.ModuleDma[i][j].AddrInfo.PhysAddr) {
					DmaCfg->AddrLowLimit = PQMaskBufMgr.ModuleDma[i][j].AddrInfo.PhysAddr;
				}
			}
		}
	} else {
		rtd_printk(KERN_INFO, TAG_NAME, "[%s] domain num = %d error\n", __FUNCTION__, domain);
		return;
	}

	// up limit now is the frame start address, we need to add a frame size offset
	// TODO : check the correct value
	DmaCfg->AddrUpLimit = ( DmaCfg->AddrUpLimit + (PQMASK_PROC_WIDTH*PQMASK_PROC_HEIGHT) ) >> 4;
	DmaCfg->AddrLowLimit = DmaCfg->AddrLowLimit >> 4;
}

/**
 * @brief
 * only check the module size setting with current status
 * @return int
 * check status
 */
int scalerPQMask_CheckResolution(void) {

	int Ret = RET_OK;
	const unsigned int ResoluationTolerance = 2;
	unsigned int IdomainWidth  = Scaler_DispGetInputInfo(SLR_INPUT_DI_WID); //Scaler_DispGetInputInfo(SLR_INPUT_SDNRH_ACT_WID_PRE); // ?
	unsigned int IdomainHeight = Scaler_DispGetInputInfo(SLR_INPUT_DI_LEN); //Scaler_DispGetInputInfo(SLR_INPUT_SDNRV_ACT_LEN_PRE); // ?
	unsigned int DdomainWidth  = Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
	unsigned int DdomainHeight = Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN);

	if( _ABS_(PQMaskBlkModuleCfg[PQMASK_BLKMOD_SPNR].IoSize.Output.Width - IdomainWidth) > ResoluationTolerance ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%s] width = %d, Idomain width = %d\n",
			__FUNCTION__, PQMaskBlockModuleName[PQMASK_BLKMOD_SPNR], PQMaskBlkModuleCfg[PQMASK_BLKMOD_SPNR].IoSize.Output.Width, IdomainWidth
		);
		Ret = RET_FAIL;
	}
	if( _ABS_(PQMaskBlkModuleCfg[PQMASK_BLKMOD_SPNR].IoSize.Output.Height - IdomainHeight) > ResoluationTolerance ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%s] height = %d, Idomain height = %d\n",
			__FUNCTION__, PQMaskBlockModuleName[PQMASK_BLKMOD_SPNR], PQMaskBlkModuleCfg[PQMASK_BLKMOD_SPNR].IoSize.Output.Height, IdomainHeight
		);
		Ret = RET_FAIL;
	}

	if( _ABS_(PQMaskBlkModuleCfg[PQMASK_BLKMOD_SHARPNESS].IoSize.Output.Width - DdomainWidth) > ResoluationTolerance ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%s] width = %d, Idomain width = %d\n",
			__FUNCTION__, PQMaskBlockModuleName[PQMASK_BLKMOD_SHARPNESS], PQMaskBlkModuleCfg[PQMASK_BLKMOD_SHARPNESS].IoSize.Output.Width, DdomainWidth
		);
		Ret = RET_FAIL;
	}
	if( _ABS_(PQMaskBlkModuleCfg[PQMASK_BLKMOD_SHARPNESS].IoSize.Output.Height - DdomainHeight) > ResoluationTolerance ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%s] height = %d, Idomain height = %d\n",
			__FUNCTION__, PQMaskBlockModuleName[PQMASK_BLKMOD_SHARPNESS], PQMaskBlkModuleCfg[PQMASK_BLKMOD_SHARPNESS].IoSize.Output.Height, DdomainHeight
		);
		Ret = RET_FAIL;
	}
	if( _ABS_(PQMaskBlkModuleCfg[PQMASK_BLKMOD_ICM].IoSize.Output.Width - DdomainWidth) > ResoluationTolerance ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%s] width = %d, Idomain width = %d\n",
			__FUNCTION__, PQMaskBlockModuleName[PQMASK_BLKMOD_ICM], PQMaskBlkModuleCfg[PQMASK_BLKMOD_ICM].IoSize.Output.Width, DdomainWidth
		);
		Ret = RET_FAIL;
	}
	if( _ABS_(PQMaskBlkModuleCfg[PQMASK_BLKMOD_ICM].IoSize.Output.Height - DdomainHeight) > ResoluationTolerance ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s][%s] height = %d, Idomain height = %d\n",
			__FUNCTION__, PQMaskBlockModuleName[PQMASK_BLKMOD_ICM], PQMaskBlkModuleCfg[PQMASK_BLKMOD_ICM].IoSize.Output.Height, DdomainHeight
		);
		Ret = RET_FAIL;
	}

	return Ret;
}

/****************************** Ring Buffer ******************************/

/**
 * @brief
 * ring buffer function, initialization
 * @param pObj
 * structure pointer
 * @param Num
 * module number
 * @return char
 * status
 */
char PQMaskRingBuf_Init(void *pSelf, void *Num) {
	char Ret = RET_OK;
	VIP_EASY_RING_BUF_T *pThis = (VIP_EASY_RING_BUF_T *)pSelf;
	int *pModuleNumber = (int *)Num;

	// member
	spin_lock_init(&pThis->Lock);
	pThis->BufLength = PQMASK_MODULE_DMA_NUM;
	pThis->ReadPtr = 0;
	pThis->WritePtr = 1;
	pThis->Num = *pModuleNumber;
	strncpy(pThis->Name, PQMaskModuleName[pThis->Num], VIP_RINGBUF_NAME_STRLEN);

	// method
	pThis->FuncReset = &VipEasyRingBuf_Reset;
	pThis->FuncGetIdx = &VipEasyRingBuf_GetIdx;
	pThis->FuncSetIdx = &VipEasyRingBuf_SetIdx;
	pThis->FuncPeekIdx = &VipEasyRingBuf_PeekIdx;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] init (%d)%s buffer with length %d\n", __FUNCTION__, pThis->Num, pThis->Name, pThis->BufLength);
	return Ret;
}

/**
 * @brief
 * ring buffer function
 * debug to dump the all buffer information
 */
void PQMaskRingBuf_Dump(void) {
	int i=0;
	for(i=0; i<PQMASK_MODULE_NUM; i++) {
		spin_lock_irqsave(&(PQMaskRingBuf[i].Lock), PQMaskRingBuf[i]._RINGBUFFlags);
		rtd_printk(KERN_EMERG, TAG_NAME, "[%-*s] read = %d, write = %d\n",
			PQMASK_STRING_LEN, PQMaskRingBuf[i].Name,
			PQMaskRingBuf[i].ReadPtr,
			PQMaskRingBuf[i].WritePtr
		);
		spin_unlock_irqrestore(&(PQMaskRingBuf[i].Lock), PQMaskRingBuf[i]._RINGBUFFlags);
	}
}


/****************************** Temp function to bring-up verify ******************************/
void scalerPQMask_SetDMAPTG(unsigned int ModuleSel, unsigned int PatternSel) {

	int i=0, j=0;
	unsigned char *pAddr = NULL;

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] start\n", __FUNCTION__);

	if( ModuleSel >= PQMASK_MODULE_NUM ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] ModuleSel %d error\n", __FUNCTION__, ModuleSel);
		return;
	}

	pAddr = (unsigned char *)PQMaskBufMgr.ModuleDma[ModuleSel][PQMaskRingBuf[ModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[ModuleSel], VIP_RINGBUF_PTR_READ)].AddrInfo.VirtAddr_nonCached;
	memset((void *)pAddr, 0, sizeof(unsigned char)*480*256);

	for(j=0; j<256; j++) {
		for(i=0; i<480; i++) {
			// force 0
			if( PatternSel == 0 ) {
				pAddr[j*480+i] = 0;
			}
			// force 128
			else if( PatternSel == 1 ) {
				pAddr[j*480+i] = 128;
			}
			// horizontal ramp
			else if( PatternSel == 2 ) {
				pAddr[j*480+i] = (i%256);
			}
			// vertical ramp
			else if( PatternSel == 3 ) {
				pAddr[j*480+i] = (j%256);
			}
		}
	}

	rtd_printk(KERN_INFO, TAG_NAME, "[%s][%s] set with pattern %d to %d buffer\n",
		__FUNCTION__,
		PQMaskModuleName[ModuleSel],
		PatternSel,
		PQMaskRingBuf[ModuleSel].FuncGetIdx((void *)&PQMaskRingBuf[ModuleSel], VIP_RINGBUF_PTR_READ)
	);

	rtd_printk(KERN_INFO, TAG_NAME, "[%s] done\n", __FUNCTION__);

}