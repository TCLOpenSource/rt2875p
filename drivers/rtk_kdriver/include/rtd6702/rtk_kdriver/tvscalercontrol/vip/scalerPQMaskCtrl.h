#ifndef _SCALER_PQMASKCTRL_H_
#define _SCALER_PQMASKCTRL_H_
/*******************************************************************************
 * Header files
*******************************************************************************/
#include <ioctrl/ai/ai_cmd_id.h>
#include <tvscalercontrol/vip/pqmask.h>
#include <tvscalercontrol/vip/vip_ring_buffer.h>
#include <scaler/vipCommon.h>

/*******************************************************************************
 * Definitions
********************************************************************************/
#define PQMASK_PROC_WIDTH		(480)	// SE process flow image width
#define PQMASK_PROC_HEIGHT		(256)	// SE process flow image height
#define PQMASK_MODULE_DMA_NUM	(3)		// module dma buffer num
#define PQMASK_STRING_LEN		(12)	// some module name max length
#define PQMASK_PERF_NUM			(256)	// record 256 frmaes, time stamp if 8bit data

/*******************************************************************************
 * Structure
********************************************************************************/
enum {
	/* 0 */ PQMASK_STRUCT_PERF = 0,
	/* 1 */ PQMASK_STRUCT_BLKMODULE_INFO,
	/* 2 */ PQMASK_STRUCT_BUFINFO,
	/* 3 */ PQMASK_STRUCT_GLOBAL,
	/* 4 */ PQMASK_STRUCT_GLB_CTRL,
	/* 5 */ PQMASK_STRUCT_GLB_STATUS,
	/* 6 */ PQMASK_STRUCT_MONITOR_CTRL,
	/* 7 */ PQMASK_STRUCT_MONITOR_STATUS,
	/* 6 */ PQMASK_STRUCT_NUM,
};

enum {
	/* 0 */ PQMASK_DOMAIN_I = 0,
	/* 1 */ PQMASK_DOMAIN_D,
	/* 2 */ PQMASK_DOMAIN_NUM,
};

enum {
	/* 0 */ PQMASK_BLKMOD_SPNR = 0,			// contains SNR, DECONT pqmask weight plane
	/* 1 */ PQMASK_BLKMOD_SHARPNESS,		// contains EDGE, TEXT pqmask weight plane
	/* 2 */ PQMASK_BLKMOD_ICM,				// contains HUE, SAT, INT pqmask weight plane
	/* 4 */ PQMASK_BLKMOD_NUM,
};

enum {
	/* 0 */ PQMASK_SE_SRCBUF_SEMANTIC,
	/* 1 */ PQMASK_SE_SRCBUF_DEPTH,
	/* 2 */ PQMASK_SE_SRCBUF_DEPTH_RESIZE,
	/* 3 */ PQMASK_SE_PROCBUF_00,
	/* 4 */ PQMASK_SE_PROCBUF_01,
	/* 5 */ PQMASK_SE_PROCBUF_02,
	/* 6 */ PQMASK_SE_BUF_NUM,
};

enum {
	/* 0 */ DMA_IDX_CUR = 0,
	/* 1 */ DMA_IDX_NXT,
	/* 2 */ DMA_IDX_MAX
};

enum {
	/* 0 */ PQMASK_STAT_UNKOWN = 0, // at very start-up
	/* 1 */ PQMASK_STAT_STANDBY,	// status is not not ready
	/* 2 */ PQMASK_STAT_READY,		// ready to get data from ap model
	/* 3 */ PQMASK_STAT_PREPARE,	// prepare the se task
	/* 4 */ PQMASK_STAT_SE_PROC,	// se is processing from label to weighting
	/* 6 */ PQMASK_STAT_ERR,		// error happen, try to recovery
	/* 7 */ PQMASK_STAT_MAX,
};

enum {
	/* 0 */ RET_OK = 0,
	/* 1 */ RET_FAIL,
};

enum {
	/* 0 */ MODULE_GRP0 = 0,
	/* 1 */ MODULE_GRP1,
	/* 2 */ MODULE_GRP2,
	/* 3 */ MODULE_GRP_MAX,
};

// BUFFER STRUCTURE
typedef struct PQMASK_MODULE_BUF_BASICINFO {
	unsigned int Width;
	unsigned int Height;
	unsigned int UnitBytes;
	unsigned char RstValue;
} PQMASK_MODULE_BUF_BASICINFO_T;

typedef struct PQMASK_MODULE_BUF_ADDRINFO {
	unsigned long Size;
	unsigned long VirtAddr;
	unsigned long VirtAddr_nonCached;
	unsigned long PhysAddr;
} PQMASK_MODULE_BUF_ADDRINFO_T;

typedef struct PQMASK_MODULE_BUFINFO {
	PQMASK_MODULE_BUF_BASICINFO_T BasicInfo;
	PQMASK_MODULE_BUF_ADDRINFO_T AddrInfo;
} PQMASK_MODULE_BUFINFO_T;

typedef struct PQMASK_BUFINFO {
	unsigned long Size; // Bytes
	PQMASK_MODULE_BUFINFO_T ModuleDma[PQMASK_MODULE_NUM][PQMASK_MODULE_DMA_NUM];
	PQMASK_MODULE_BUFINFO_T SeBuffer[PQMASK_SE_BUF_NUM];
	PQMASK_MODULE_BUFINFO_T DepthSrcImg;
} PQMASK_BUFINFO_T;

// STATUS/CTRL STRUCTURE
typedef struct PQMASK_SEPROC_STATUS {
	unsigned char IsRunning;
	unsigned int ProcTsp;
	unsigned int StartTime;
} PQMASK_SEPROC_STATUS_T;

typedef struct PQMASK_GLB_STATUS {
	PQMASK_SEPROC_STATUS_T SeStatus;
	unsigned char bDepNeedUptHist;
	unsigned char ErrRecoverTimes;
	unsigned char PQMaskStatus;
} PQMASK_GLB_STATUS_T;

typedef struct PQMASK_GLB_CTRL {
	unsigned char bEnableSeProc;
	unsigned char LimitFPS;
	unsigned char ErrRecoverRstTimes;
	unsigned char SemCapImgLimitFPS;
	unsigned char bDepNewInfo;
	unsigned char bSemNewInfo;
	unsigned char bIsDepthReady;
	unsigned char bIsSemReady;
} PQMASK_GLB_CTRL_T;

typedef struct PQMASK_GLB_MGR {
	PQMASK_GLB_CTRL_T Ctrl;
	PQMASK_GLB_STATUS_T Status;
} PQMASK_GLB_MGR_T;

// PERFORMANCE EVALUATION STRUCTURE
typedef struct PQMASK_PERF {

	// kernel information, 14 + 3*8 = 38 nodes * 256 frames = 9728
	unsigned int Kernel2ApImage;
	// ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ----------
	// ap information, 6 nodes * 256 frames = 1536
	PQMaskModelPerf ModelPerf;
	unsigned int Ap2KernelInfo;
	unsigned int KernelPrepStart;
	unsigned int SETaskStart;
	unsigned int SETaskHistEnd;
	unsigned int DepthResizeEnd;
	// ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ----------
	// 0 : set depth palette
	// 1 : depth to weight
	// 2 : set semantic palette
	// 3 : semantic to weight
	// 4 : resize semantic weight if necessary
	// 5 : semantic/depth weight fusion
	// 6 : spatial filter
	// 7 : temporal filter
	unsigned int SETaskGrpEnd[MODULE_GRP_MAX][8];
	unsigned int SETaskEnd;
	unsigned int IdomainConfigStart;
	unsigned int IdomainConfigEnd;
	unsigned int DdomainConfigStart;
	unsigned int DdomainConfigEnd;
	// ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ----------
	// ring buffer status, 8 module * 2 ptr = 16 * 256 frames = 4096
	unsigned int RingBufStatus[PQMASK_MODULE_NUM][VIP_RINGBUF_PTR_MAX];
	// ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ----------
	// kernel run faster than ap, we need record how many loop for same input and last process end time
	unsigned int RepeatTimes;
	unsigned int RepeatSETaskStart; // record every se task time
	unsigned int RepeatSETaskEnd; // record every se task time
} PQMASK_PERF_T; // 60 per row, 256 frams, total 15360

// ONLINE MONITOR STRUCTURE
typedef struct PQMASK_ONLINE_MONITOR_CTRL {
	//
	unsigned char bEnable;
	// General log
	unsigned char bLogEnable;
	unsigned char LogPeriod;
	//
	unsigned char Depth_TargetFps;
	unsigned char Depth_MeasureRstFrm;
	unsigned char SEProc_TargetTime;
} PQMASK_ONLINE_MONITOR_CTRL_T;

typedef struct PQMASK_ONLINE_MONITOR_STATUS {
	// Error flag
	unsigned int Depth_FpsError;
	unsigned int SEProc_TimeError;
	// General log
	unsigned int LogCounter;
	// Depth measure data
	unsigned int Depth_MeasureStartTime;
	unsigned int Depth_MeauserEndTime;
	unsigned int Depth_Counter;
	// SE Proc measure realted data
	unsigned int SECurProcStart;
	unsigned int SECurProcEnd;
} PQMASK_ONLINE_MONITOR_STATUS_T;

typedef struct PQMASK_ONLINE_MONITOR {
	PQMASK_ONLINE_MONITOR_CTRL_T Ctrl;
	PQMASK_ONLINE_MONITOR_STATUS_T Status;
} PQMASK_ONLINE_MONITOR_T;

//
typedef struct PQMASK_BLK_MODULE_CONFIG {
	PQMASK_SACLING_T Scale;
	PQMASK_IO_SIZE_T IoSize;
} PQMASK_BLK_MODULE_CONFIG_T;

/*******************************************************************************
 * Parameters
********************************************************************************/
extern unsigned int SEProcGrpMapping[MODULE_GRP_MAX][3];
extern char PQMaskModuleName[PQMASK_MODULE_NUM][PQMASK_STRING_LEN];

/*******************************************************************************
 * functions
********************************************************************************/
void scalerPQMask_init(void);
void scalerPQMask_exit(void);
void scalerPQMask_suspend(void);
// void scalerPQMask_resume(void);

int scalerPQMask_WaitSETaskFinish(unsigned int WaitCnt);
int scalerPQMask_CheckDMA(void);
void scalerPQMask_DmaFree(void);
int scalerPQMask_DmaAllocate(void);
int scalerPQMask_ResetDma(void);
void scalerPQMask_postprocessing_entry(void);
void scalerPQMask_postprocessing_idomain(void);
void scalerPQMask_postprocessing_ddomain(void);
void scalerPQMask_ErrorHandler(void);
void scalerPQMask_GetSemanticARGBBuffer(unsigned char *dstBuf, unsigned char hRatio, unsigned char vRatio, unsigned char Alpha);
void scalerPQMask_GetSemanticARGBBuffer_Coordinate(
	unsigned char *dstBuf, unsigned int bufferWidth, unsigned int xStart, unsigned int yStart,
	unsigned char hRatio, unsigned char vRatio, unsigned char Alpha );
bool scalerPQMask_FpsCtrl(unsigned int *CurTime, unsigned int *PreTime, unsigned int TargetFps, unsigned char bPrintEnable, char *Key);
void scalerPQMask_OnlineMonitor(void);

void *scalerPQMask_GetCtrlStruct(unsigned int InfoIndex);
const void*scalerPQMask_GetStatusStruct(unsigned int InfoIndex);

/*******************************************************************************
 * Module config functions
********************************************************************************/
void scalerPQMask_InitModuleConfig(void);
void scalerPQMask_disableModuleDMA(void);

void scalerPQMask_SetModuleAddr(unsigned char ModuleSel, unsigned long TargetAddr);
void scalerPQMask_SetBlkModuleDmaConfig(unsigned char BlkModuleSel);
void scalerPQMask_SetBlkModuleIoSize(unsigned char BlkModuleSel);

void scalerPQMask_GetDMALimit(unsigned char domain, PQMASK_DMA_CONFIG_T *DmaCfg);

int scalerPQMask_CheckResolution(void);
/*******************************************************************************
 * RingBuffer
********************************************************************************/
char PQMaskRingBuf_Init(void *pSelf, void *Num);

#endif