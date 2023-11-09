

#ifndef _SCALERMEMC_H_
#define _SCALERMEMC_H_


//#include "../scaler/include/rtd_types.h"
//#include "../scaler/include/rtd_system.h"
//#include <scaler/scalerCommon.h>
//#include <../memc/include/memc_lib.h>
#include <memc_isr/include/memc_lib.h>

//#undef RUN_MERLIN_MEMC_ENABLE
//#define CONFIG_MEMC_DEFAULT_REPEAT_MODE
#define CONFIG_MEMC_TASK_QUEUE

#define TAG_NAME_MEMC "MEMC"

//#define MEMC_BRING_UP 1
//#define MEMC_ISR_Times_debug

/*-----------------------------------------------------------------------------
	Global Variables & Function Prototypes Declarations
------------------------------------------------------------------------------*/
/**
 * FRC MC type
 *
 */
typedef enum {
	MEMC_TYPE_OFF = 0,      //24->60
	MEMC_TYPE_HIGH,			//VPQ_MEMC_TYPE_LOW
	MEMC_TYPE_LOW,			//VPQ_MEMC_TYPE_HIGH
	MEMC_TYPE_NORMAL,		// Clear Plus
	MEMC_TYPE_USER,			//VPQ_MEMC_TYPE_USER
	MEMC_TYPE_55_PULLDOWN,  //24->48 // VPQ_MEMC_TYPE_55_PULLDOWN
	MEMC_TYPE_MEDIUM,		//VPQ_MEMC_TYPE_MEDIUM
	//	MEMC_TYPE_CINEMATIC_MOVEMENT,		//VPQ_MEMC_TYPE_CINEMATIC_MOVEMENT
	MEMC_TYPE_MAX,
} SCALER_MEMC_TYPE_T;

/**
*FRC bypass Region
*
*/
typedef enum {
	BYPASS_REGION_0	= 0,
	BYPASS_REGION_1,
	BYPASS_REGION_2,
	BYPASS_REGION_3,
	BYPASS_REGION_4,
	BYPASS_REGION_MAX,
} SCALER_MEMC_BYPASS_REGION_T;

/**
*FRC Rect
*
*/
typedef struct {
	unsigned int		w;
	unsigned int		h;
	unsigned int		x;
	unsigned int		y;
} SCALER_MEMC_BYPASS_REGION_RECT_T;


/**
 *FRC video mute type
 *
 */
typedef enum {
	MEMC_MUTE_TYPE_INPUT = 0,
	MEMC_MUTE_TYPE_OUTPUT,
	MEMC_MUTE_TYPE_LVDS,
	MEMC_MUTE_TYPE_MAX
} SCALER_MEMC_MUTE_TYPE_T;

typedef struct{
	unsigned int blurLevel;
	unsigned int judderLevel;
	unsigned int motion;
}SCALER_MEMC_SETMOTIONCOMP_T;

typedef struct{
	unsigned int type;
	unsigned int bOnOff;
}SCALER_MEMC_SETVIDEOBLOCK_T;

typedef struct{
	unsigned int bOnOff;
	unsigned int region;
	unsigned int x;
	unsigned int y;
	unsigned int w;
	unsigned int h;
}SCALER_MEMC_SETBYPASSREGION_T;

typedef struct{
	unsigned int bOnOff;
	unsigned int r;
	unsigned int g;
	unsigned int b;
}SCALER_MEMC_SETDEMOBAR_T;


#ifndef SCALER_MEMC_SETINOUTUSECASE_T
typedef struct{
	unsigned int input_re;
	unsigned int output_re;
	unsigned int input_for;
	unsigned int output_for;
}__SCALER_MEMC_SETINOUTUSECASE_T;
#define SCALER_MEMC_SETINOUTUSECASE_T __SCALER_MEMC_SETINOUTUSECASE_T
#endif

#ifndef SCALER_MEMC_SETINOUTPUTFORMAT_T
typedef struct{
	unsigned int input_for;
	unsigned int output_for;
}__SCALER_MEMC_SETINOUTPUTFORMAT_T;
#define SCALER_MEMC_SETINOUTPUTFORMAT_T __SCALER_MEMC_SETINOUTPUTFORMAT_T

#endif

#ifndef SCALER_MEMC_SETINOUTPUTRESOLUTION_T
typedef struct{
	unsigned int input_re;
	unsigned int output_re;
}__SCALER_MEMC_SETINOUTPUTRESOLUTION_T;
#define SCALER_MEMC_SETINOUTPUTRESOLUTION_T __SCALER_MEMC_SETINOUTPUTRESOLUTION_T
#endif

#ifndef SCALER_MEMC_DMASTARTADDRESS_T
typedef struct{
	unsigned int MC00_start_addr;
	unsigned int MC01_start_addr;
	unsigned int ME_start_addr;
}__SCALER_MEMC_DMASTARTADDRESS_T;
#define SCALER_MEMC_DMASTARTADDRESS_T __SCALER_MEMC_DMASTARTADDRESS_T
#endif

#ifndef SCALER_MEMC_INOUTFRAMERATE_T
typedef struct{
	unsigned int input_frame_rate;
	unsigned int output_frame_rate;
}__SCALER_MEMC_INOUTFRAMERATE_T;
#define SCALER_MEMC_INOUTFRAMERATE_T __SCALER_MEMC_INOUTFRAMERATE_T
#endif


#ifdef CONFIG_MEMC_TASK_QUEUE
/*MEMC Task Struct*/
typedef struct{
	unsigned int type;
	union _data{
		unsigned int value;
		SCALER_MEMC_TYPE_T memc_type;
		SCALER_MEMC_BYPASS_REGION_T memc_bypass_region;
		SCALER_MEMC_BYPASS_REGION_RECT_T memc_bypass_region_rect;
		SCALER_MEMC_MUTE_TYPE_T memc_mute_type;
		SCALER_MEMC_SETMOTIONCOMP_T memc_set_motion_comp;
		SCALER_MEMC_SETVIDEOBLOCK_T memc_set_video_block;
		SCALER_MEMC_SETBYPASSREGION_T memc_set_bypass_region;
		SCALER_MEMC_SETDEMOBAR_T memc_set_demo_bar;
		SCALER_MEMC_SETINOUTUSECASE_T memc_set_inout_usecase;
		SCALER_MEMC_SETINOUTPUTFORMAT_T memc_set_inout_format;
		SCALER_MEMC_SETINOUTPUTRESOLUTION_T memc_set_inout_resolution;
		SCALER_MEMC_DMASTARTADDRESS_T memc_dma_start_address;
		SCALER_MEMC_INOUTFRAMERATE_T memc_inout_framerate;
	}data;
}SCALER_MEMC_TASK_T;
#endif

/*
FILM information from MEMC
*/
typedef enum{
	/*0*/ _FILM_MEMC_ALL = 0,
	/*1*/ _FILM_MEMC_TOP,
	/*2*/ _FILM_MEMC_BOT,
	/*3*/ _FILM_MEMC_LFT,
	/*4*/ _FILM_MEMC_RHT,
	/*5*/ _FILM_MEMC_CEN,
	/*6*/ _FILM_MEMC_OUT_ALL,
	/*7*/ _FILM_MEMC_NUM
}SCALER_MEMC_CADENCE_INDEX_T;

typedef enum _MEMC_STATUS_CHECK{
    MEMC_REPEAT_STATUS = 0,
    MEMC_CLK_STATUS,    
}MEMC_STATUS_CHECK;

typedef enum _MEMC_STATUS_APPLY{
    MEMC_CLK_ALLOFF = 0,//no need to consider repeat
	MEMC_CLK_MCONLY_REPEATON, //MC only, repeat must set on
	MEMC_CLK_ALLON_REPEATON,    
	MEMC_CLK_ALLON_REPEATOFF,
	MEMC_STATUS_APPLY_CHECK,
}MEMC_STATUS_APPLY;


#if 1//#if RTK_MEMC_Performance_tunging_from_tv001
#ifndef CONFIG_MEMC_BOUNDARY_AUTO
#define CONFIG_MEMC_BOUNDARY_AUTO	(1)
#endif
#endif

#if 1//#if RTK_MEMC_Performance_tunging_from_tv001
unsigned int Scaler_MEMC_Get_Size_Addr(unsigned long *memc_addr_aligned, unsigned long *memc_addr);
#endif


void memc_suspend(void);
void Scaler_MEMC_SetInitFlag(unsigned char val);
unsigned char Scaler_MEMC_GetInitFlag(void);
void Scaler_MEMC_DMA_DB_Check(void);
void Scaler_MEMC_Initialize(unsigned int ME_start_addr, unsigned int MC00_start_addr, unsigned int MC01_start_addr);
void Scaler_MEMC_Uninitialize(void);
void Scaler_MEMC_SetMotionComp(unsigned char blurLevel, unsigned char judderLevel, SCALER_MEMC_TYPE_T motion);
void Scaler_MEMC_SetBlurLevel(unsigned char blurLevel);
void Scaler_MEMC_SetJudderLevel(unsigned char judderLevel);
void Scaler_MEMC_MotionCompOnOff(unsigned char bOnOff);
void Scaler_MEMC_LowDelayMode(unsigned char type);
void Scaler_MEMC_SetRGBYUVMode(unsigned char mode);
void Scaler_MEMC_GetFrameDelay (unsigned short *pFrameDelay);
void Scaler_MEMC_SetVideoBlock(SCALER_MEMC_MUTE_TYPE_T type, unsigned char bOnOff);
void Scaler_MEMC_SetTrueMotionDemo(unsigned char bOnOff);
void Scaler_MEMC_GetFirmwareVersion(unsigned short *pVersion);
void Scaler_MEMC_SetBypassRegion(unsigned char bOnOff, SCALER_MEMC_BYPASS_REGION_T region, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void Scaler_MEMC_SetReverseControl(unsigned char u8Mode);
void Scaler_MEMC_Freeze(unsigned char type);
void Scaler_MEMC_SetDemoBar(unsigned char bOnOff, unsigned char r, unsigned char g, unsigned char b);
void Scaler_MEMC_DEBUG(void);
void Scaler_MEMC_SetInOutputUseCase(unsigned char input_re, unsigned char output_re, unsigned char input_for, unsigned char output_for);
void Scaler_MEMC_SetInputOutputFormat(unsigned char input_for, unsigned char output_for);
void Scaler_MEMC_SetInputOutputResolution(unsigned char input_re, unsigned char output_re);
void Scaler_MEMC_SetInputFrameRate(unsigned char frame_rate);
void Scaler_MEMC_SetOutputFrameRate(unsigned char frame_rate);
void Scaler_MEMC_SetMEMCFrameRepeatEnable(unsigned char enable);
void ISR_Scaler_MEMC_SetInOutMode(unsigned char emom);
void Scaler_MEMC_SetInstantBootInitPhaseTable(void);
void Scaler_MEMC_SetMEMC_Enable(unsigned char enable);
void Scaler_MEMC_SetCinemaMode(unsigned char enable);
unsigned char Scaler_MEMC_GetMEMC_Enable(void);
unsigned char Scaler_MEMC_GetMEMC_Mode(void);
unsigned char ISR_Scaler_MEMC_GetInOutMode(void);
/**
 * @brief This function set power saving mode on
 * @retval 1		power save on fail
 * @retval 0		power save on success
*/
BOOL Scaler_MEMC_SetPowerSaveOn(void);
/**
 * @brief This function set power saving mode off
 * @retval 1		power save off fail
 * @retval 0		power save off success
*/
BOOL Scaler_MEMC_SetPowerSaveOff(void);
unsigned int Scaler_MEMC_ISR(void);
void Scaler_MEMC_resetMEMCMode(void);
unsigned char * Scaler_MEMC_GetCadence(void);
unsigned int Scaler_MEMC_GetAVSyncDelay(void);
void Scaler_MEMC_SetAVSyncDelay(unsigned int a_ucDelay);
#ifdef CONFIG_MEMC_TASK_QUEUE
/*operation related with memc tasks*/
/**
 * \brief Scaler_MEMC_PushTask Add a memc task to queue
 * \param pTask MEMC task instance
 * \return Returns false while queue is full,otherwise returns true.
**/
extern unsigned char Scaler_MEMC_PushTask(SCALER_MEMC_TASK_T *pTask);
/**
 * \brief ScalerMEMC_PopTask Pop a memc task from queue
 * \param pTask Save the popped task when the queue is not empty.
 * \return Returns FALSE while queue is empty,otherwise returns TRUE.
**/
extern unsigned char ScalerMEMC_PopTask(SCALER_MEMC_TASK_T *pTask);
/**
 * \brief ScalerMEMC_TasksHandler Response of whole tasks queue
**/
extern void ScalerMEMC_TasksHandler(VOID);


void Scaler_MEMC_Set_MC_MEMBoundary(unsigned int  nDownlimit,unsigned int nUplimit);

void Scaler_MEMC_Set_ME_MEMBoundary(unsigned int  nDownlimit,unsigned int nUplimit);

unsigned int scalerVIP_Get_MEMC_status(void);
unsigned int scalerVIP_Get_MEMC_status_check(MEMC_STATUS_CHECK MEMC_case_status);
unsigned char Scaler_get_MEMC_LowDelay_disable_condition(void);

unsigned int Scaler_MEMC_ISR_InputVSInterrupt(void);
unsigned int Scaler_MEMC_ISR_OutputVSInterrupt(void);


#endif

#endif


