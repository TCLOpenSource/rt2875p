/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for scaler clock related functions.
 *
 * @author 	$Author$
 * @date		$Date$
 * @version	$Revision$
 * @ingroup	clock
 */

/**
 * @addtogroup clock
 * @{
 */

#ifndef _SCALER_H_
#define _SCALER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILD_QUICK_SHOW
#include <sysdefs.h>
#endif

//#include <Platform_Lib/Utils/RtSystem.h>
//#include <rbusVgip2Reg.h>
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
	#include <scaler/scalerCommon.h>
#else

#include <scalercommon/scalerDrvCommon.h> /* SCALERIOC_GETVOINFO */
#include <scalercommon/scalerCommon.h>    /* StructDisplayInfo */

#endif

#define VDO_CONNECT_NEW_FLOW_ENABLE //need video new vdo conenct flow
#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
#include <tvscalercontrol/scaler/scalervideo.h>
#endif
typedef struct  {
    char *name;
    int progressive;
    unsigned int h_act_len;
    unsigned int v_act_len;
    unsigned int lr_v_act_len;
    unsigned int v_active_space1;
    unsigned int v_active_space2;
} HDMI_ACTIVE_SPACE_TABLE_T;

extern const HDMI_ACTIVE_SPACE_TABLE_T hdmi_active_space_table[];

/*============================ Module dependency  ===========================*/
//#include <auto_conf.h>
//#include <rtd_types.h>
//#include <rbusScaledownReg.h>
//#include <rbusPpOverlayReg.h>//peggy mark first
/*===================================  Types ================================*/
/*================================== Variables ==============================*/
//#define HDMI_NO_PQDC_TEST 1

// mdomain repeat frame
//#define CONFIG_MDOMAIN_FORCE_FRC	1

#define CONFIG_YPBPR_AUTO_COLOR_TYPE2	1
#define CONFIG_VGA_SUPPORT_ABL	1
#ifdef ENABLE_VGA_POSITION_SAVE
#define CONFIG_VGA_POSITION_SAVE	1
#endif
//#define AUTO_SOY_ADAPTIVE_FLOW 1
#define CONFIG_ADC_DMA 1
#define DUMP_VSYNC 1
#define CONFIG_3D_ASPECT_RATIO 1
#define  CONFIG_3D_ASPECT_RATIO_FRAMESEQ 1
#define  CONFIG_3D_OVERSCAN_FRAMESEQ 1
//#define CONFIG_3D_OVERSCAN_ENABLE 1
#define CONFIG_DCLK_CHECK 0
#define CONFIG_APLL_PROTECT 1
#define CONFIG_HDR10_ENABLE

#ifndef HDMI_NO_PQDC_TEST
#define ENABLE_3_BUFFER_DELAY_MODE 1
#endif

//#define QMS_DEBUG_PRINT

#define CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW 1
#define CONFIG_MDOMAIN_RESERVED_MEMORY 1
#define CONFIG_ORBIT_PIXEL_SHIFT_ALGO 1
//#define CONFIG_SCALER_BRING_UP 1

#ifdef CONFIG_SCALER_BRING_UP
#define CONFIG_RUN__DDTS
#ifndef CONFIG_RUN__DDTS
#define AUTO_BRINGUP_SOURCE _SRC_CVBS //_SRC_VGA   _SRC_TV  _SRC_CVBS _SRC_HDMI _SRC_MINI_DP
#define AUTO_BRINGUP_SOURCE_SUB _SRC_MAX
#define BRING_UP_AUTO_FLOW/*This is for bring up. auto vsc set input and output region*/
//#define POLLING_ONLINE_STATUS_ENABLE //USE API modestate_EnableDispInt(TRUE) instead
//#define BRING_UP_VERIFY_PQ//for bring up PQ initital using
#endif
#define CONFIG_MEMC_BYPASS 1
#else
	#ifndef HDMI_NO_PQDC_TEST
	//#define CONFIG_FORCE_RUN_I3DDMA   //not vo source force go i3ddma flow
	//#else
	//#define CONFIG_MEMC_BYPASS 1
	#endif
#endif

//#define CONFIG_MEMC_BYPASS 1

#ifdef CONFIG_BYPASSMEMC_FOR_CODESIZE_SAVING
#define CONFIG_MEMC_NOTSUPPORT 1
#endif


#ifdef CONFIG_ANDROID_BY_PASS_MEMC
#define CONFIG_MEMC_BYPASS 1
#endif

#ifdef CONFIG_REALTEK_2K_MODEL_ENABLED
	#ifndef CONFIG_MEMC_BYPASS
		#define CONFIG_MEMC_BYPASS 1
	#endif
	#define CONFIG_FRAMERATE_DEBUG_IN_VGIPISR 1
#endif

#ifdef CONFIG_RTK_BUSINESS_DISPLAY_MODEL
#define CONFIG_MEMC_BYPASS 1
#endif
#define	CONFIG_SDNR_CROP		1
#ifdef CONFIG_SDNR_CROP
#define	CONFIG_VGIP_DIGITAL_MODE //enable digital mode
#endif
//#define CONFIG_I2RND_B05_ENABLE //enable i2rnd_b05 function /*20170908, pinyen disable this config for dolby, todo: need to check whey enable config and dolby run fail*/
#define CONFIG_ENABLE_SE_ROTATE_M_DOMAIN_BLOCK_MODE
//#define CONFIG_DUMP_REGISTER
//#define CONFIG_DUMP_REGISTER_WHEN_ONLY_DISCONNECT

#define CONFIG_SMOOTH_TOOGLE_AT_VO_FREEZE 1
// for VB1 to HDMI adaptor
//#define ENABLE_VBY1_TO_HDMI_4K2K_DATA_FS

//for K4LP game mode measurement
#define CONFIG_I3DDMA_PATTERN_MEASUREMENT 1
#define BRING_UP_K4L_TEST 1//20170524 pinyen create new game mode tsk
#define NEW_INPUT_REGION_BASE_ON_HAL_ORIGINAL_INPUT 1
//#define ENABLE_VR360_DATA_FS_FLOW 1

//#define CONFIG_SUPPORT_FREESYNC //for support freesync

#define USE_NEW_HDMI_TIMING_INFO_STRUCTURE 1

#define CONFIG_RESET_VODMA_BY_NEW_MODE 1 // for ECN RL6748-523
#define CONFIG_RESET_VODMA_BY_MEASURE

#define OLED_PANEL_DVTOTAL_MIN 2244

#define CONFIG_I2D_NEW_MODE 1
//#define CONGIG_I2D_NEW_SW_MODE 1

/*================================ Definitions ==============================*/
#ifdef CONFIG_FORCE_RUN_I3DDMA   //not vo source force go i3ddma flow
#define CONFIG_HDR_SDR_SEAMLESS//If define to support HDR to SDR SEAMLESS
#endif
//#define ENABLE_DE_CONTOUR_I2R
//#define AUTO_DE_CONTOUR_I2R
//#define Scaler_ClearLineBufferStatus() IoReg_ClearBits(SCALEDOWN_ICH1_IBUFF_STATUS_VADDR, _BIT0 | _BIT1 | _BIT2)
//#define FORCE_BYPASS_DI_RTNR//If define will force bypass DI and RTNR. Only use for interlace timing
#define _OSD_DATA_IN_INTERNAL	0
#define _OSD_DATA_IN_FLASH		1
#define _OSD_DATA_IN_CODE		2

#define _EVENT_UNDERFLOW 		0x20
#define _EVENT_OVERFLOW  		0x40s

#define _I_DOMAIN				0x00//forster modified 041004
#define _D_DOMAIN				0x01

//#define _H_POSITION_MOVE_TOLERANCE      0x50 // +- 80 pixels
//#define _H_POSITION_MOVE_TOLERANCE      0xa0 // +- 160 pixels
#define _H_POSITION_MOVE_TOLERANCE      0x80 // +- 128 pixels
#define _V_POSITION_MOVE_TOLERANCE      0x32 // 50 lines
#define _H_ACT_STA_PRE_DEF 226
#define _V_ACT_STA_PRE_DEF 15

//for hdmi smooth toggle portch adjustment fo 70Hz
#define _PROTECT_VDELAY_70Hz 8


/*
-6 : RGB2YUV
-32 : M-cap
-2 : tolerance
*/
#define VGIP_H_PORCH_RGB2YUV 6
#define VGIP_H_PORCH_MCAP 32
#define VGIP_H_PORCH_TOLERANCE 2
#define VGIP_H_PORCH_MARGIN (VGIP_H_PORCH_RGB2YUV + VGIP_H_PORCH_MCAP + VGIP_H_PORCH_TOLERANCE) // h-porch num = h-total - pic width - porch margin (from DIC's comment)

#ifdef CONFIG_FHD_BY_PASS_MEMC
#define MDOM_PREREAD_START 0xf
#else
#define MDOM_PREREAD_START 0x15 // Vstart 45 - SRNN 39 + timeborrow 21 - pre_read margin 6
#define MDOM_PREREAD_START_REDUCE 0x4 // extend v 32/2 - timeborrow 21 - timeborrow_orbit 9
#define SUB_MDOM_PREREAD_START 0x7 // KTASKWBS-21489 k8 sub change to fix number 7
#endif

//VGA position adjust
#define MIN_H_BACK_PORCH 3 //minimal porch number
#define FIX_VGIP_V_START 5//fix vgip v start using delay to adjust position
#define FIX_VGIP_H_DELAY 50//fix vgip h delay using delay to adjust position

//	#define _EVENT_IVS1  			 0x01
#define WaitFor_IVS1()			fwif_scaler_wait_for_event(VGIP_VGIP_CHN1_STATUS_reg, _BIT26)

//	#define _EVENT_IEN_START1		 0x02
#define WaitFor_IEN_START1()	fwif_scaler_wait_for_event(VGIP_VGIP_CHN1_STATUS_reg, _BIT24)

//#define _EVENT_IEN_STOP1 		 0x04
#define WaitFor_IEN_STOP1()		fwif_scaler_wait_for_event(VGIP_VGIP_CHN1_STATUS_reg, _BIT25)

//#define _EVENT_IVS2 		 	0x08
#define WaitFor_IVS2()			fwif_scaler_wait_for_event(SUB_VGIP_VGIP_CHN2_STATUS_reg, _BIT26)

//#define _EVENT_IEN_START2 		 0x10
#define WaitFor_IEN_START2()	fwif_scaler_wait_for_event(SUB_VGIP_VGIP_CHN2_STATUS_reg, _BIT24)

//#define _EVENT_IEN_STOP2 		 0x20
#define WaitFor_IEN_STOP2()		fwif_scaler_wait_for_event(SUB_VGIP_VGIP_CHN2_STATUS_reg, _BIT25)

//#define _EVENT_DVS   				0x10
//#define WaitFor_DVS()			// fwif_scaler_wait_for_event(()

//#define _EVENT_DEN_START 			0x20
//#define WaitFor_DEN_START()		// fwif_scaler_wait_for_event(()
#define WaitFor_DEN_START()		fwif_scaler_wait_for_event(PPOVERLAY_DTG_pending_status_2_reg, _BIT1)	//hsliao 20090518

//#define _EVENT_DEN_STOP  			0x40
//#define WaitFor_DEN_STOP()		fwif_scaler_wait_for_event(PPOVERLAY_Main_Display_Control_RSV_reg, _BIT10)	//jerry1207a
								//WaitFor_EN_STOP_MAIN()

// #define WaitFor_OEN_START_MAIN  	0x40
#define WaitFor_EN_START_MAIN() fwif_scaler_wait_for_event(PPOVERLAY_DTG_pending_status_2_reg, _BIT1)

// #define _EVENT_OEN_STOP_MAIN 		0x08
#define WaitFor_EN_STOP_MAIN()	fwif_scaler_wait_for_event(PPOVERLAY_DTG_pending_status_2_reg, _BIT0)

// #define _EVENT_OEN_START_SUB 		0x01
#define WaitFor_EN_START_SUB()	fwif_scaler_wait_for_event(PPOVERLAY_DTG_pending_status_2_reg, _BIT3)

//#define _EVENT_OEN_STOP_SUB  		0x02
#define WaitFor_EN_STOP_SUB()	fwif_scaler_wait_for_event(PPOVERLAY_DTG_pending_status_2_reg, _BIT2)

#define ZOOM_WaitFor_IEN_STOP1()		fwif_scaler_wait_for_event1(VGIP_VGIP_CHN1_STATUS_reg, _BIT25)

#define WaitFor_IVS1_Done()		fwif_scaler_wait_for_event_done(VGIP_VGIP_CHN1_STATUS_reg, _BIT26)

#define WaitFor_IVS2_Done()			fwif_scaler_wait_for_event_done(SUB_VGIP_VGIP_CHN2_STATUS_reg, _BIT26)

#define WaitFor_EN_STOP_MAIN_Done()	fwif_scaler_wait_for_event_done(PPOVERLAY_DTG_pending_status_2_reg, _BIT0)

#define WaitFor_DEN_STOP_Done()		fwif_scaler_wait_for_event_done(PPOVERLAY_DTG_pending_status_2_reg, _BIT0)

#define hw_msleep(msecs)\
{\
    unsigned long usecs = (unsigned long)(msecs * 1000);\
    if (usecs <= 20000)\
        usleep_range(usecs, usecs);\
    else\
        msleep(msecs);\
}\


#define SUPPORT_DOLBY_VSIF		1

#if defined (SELFAWB)
#define CONFIG_SELF_AWB_ENABLE  //leochen add
#endif

/***********************BSP error use start ****************************/
#define ERROR_MESSAGE_MAX_STRING_SIZE 64
//for error message report
#define	MAIN_SMOOTH_TOGGLE_ERROR_MESSAGE	"Main smooth toggle fail"
#define	SUB_SMOOTH_TOGGLE_ERROR_MESSAGE		"Sub smooth toggle fail"
#define	MAIN_M_DOMAIN_PQC_ERROR_MESSAGE		"Main M domain PQC error"
#define	MAIN_M_DOMAIN_PQDC_ERROR_MESSAGE	"Main M domain PQDC error"
#define	SUB_M_DOMAIN_PQC_ERROR_MESSAGE		"Sub M domain PQC error"
#define	SUB_M_DOMAIN_PQDC_ERROR_MESSAGE		"Sub M domain PQDC error"
#define	MAIN_VI_ERROR_MESSAGE				"Main VI error"
#define	SUB_VI_ERROR_MESSAGE				"Sub VI error"
#define	LINE_BUFFER_SYNC_ERROR_MESSAGE		"Line buffer sync error"
#define	MEMC_PQC_ERROR_MESSAGE				"MEMC PQC error"
#define	MEMC_PQDC_ERROR_MESSAGE				"MEMC PQDC error"
#define	MEMC_DMA_ERROR_MESSAGE				"MEMC DMA error"
#define	I3DDMA_PQC_ERROR_MESSAGE			"I3DDMA pqc error"
#define	VO_I3DDMA_PQDC_ERROR_MESSAGE		"VO i3ddma PQDC error"
#define	VO1_VDEC_DMA_ERROR_MESSAGE			"VO1 vdec dma error"
#define	VO1_VDEC_DECOMPRESS_ERROR_MESSAGE	"VO1 vdec decompress error"
#define	VO2_VDEC_DMA_ERROR_MESSAGE			"VO2 vdec dma error"
#define	VO2_VDEC_DECOMPRESS_ERROR_MESSAGE	"VO2 vdec decompress error"
#define	DI_PQC_ERROR_MESSAGE				"DI PQC error"
#define	DI_PQDC_ERROR_MESSAGE				"DI PQDC error"
#define	OD_DMA_ERROR_MESSAGE				"OD	DMA error"
#define	OD_PQC_ERROR_MESSAGE				"OD PQC error"
#define	OD_PQDC_ERROR_MESSAGE				"OD PQDC error"

//kernel handler
#define	ABNORMAL_RUN_SCALER_MESSAGE			"Abnormal run scaler event"
#define	MAIN_MUTE_TIME_ABNORMAL_MESSAGE		"Main Mute Time Abnormal event"
#define	SUB_MUTE_TIME_ABNORMAL_MESSAGE		"Sub Mute Time Abnormal event"
/***********************BSP error use end ****************************/

//#define THERMAL_PHASE_COMPENSATION
//#define THERMAL_PHASE_COMPENSATION_THRESHOLD	5

#define FAKE_PORCH_THR_LINE 6
#define FAKE_PORCH_MIN_V_BLANK 18
#define FAKE_PORCH_MIN_V_FRONT 4
#define FAKE_PORCH_MIN_V_BACK 12

typedef enum{
    OLED_RESET_TARGET_CASE_NONE = 0,
    OLED_RESET_TARGET_CASE_CP,
    OLED_RESET_TARGET_CASE_CP_RPL,
    OLED_RESET_TARGET_CASE_NONCP,
    OLED_RESET_TARGET_CASE_HDMI,
    NON_OLED_RESET_TARGET_CASE_HDMI,
    NON_OLED_RESET_TARGET_CASE_CP,
    NON_OLED_RESET_TARGET_CASE_CP_RPL,
} OLED_RESET_TARGET_CASE;

typedef enum {
	NN_ALL_CAP_DISABLE,      //disable i3 capture & sub capture
	H3DDMA_CAP_ENABLE_ONLY,  //disable capture mode switch,always enable i3ddma capture
	ALL_CAP_ENABLE,          //enable capture mode switch i3d cap <-> sub cap          
}NN_CAP_STATUS_E;

typedef struct _StructSrcRect{
	unsigned short srcx;     		//x
	unsigned short srcy;     		//y
	unsigned short src_wid;    	//width
	unsigned short src_height;	//height
}StructSrcRect;

typedef struct _StructRect{
	int x;     		//x
	int y;     		//y
	int width;    	//width
	int height;	//height
}StructRect;

typedef struct _StructDstRect{
	unsigned short dst_wid;    	//width
	unsigned short dst_height;	//height
}StructDstRect;

typedef struct _StructJPEGDisplayInfo{
	unsigned short	IVTotal;		///! input vertical total
	unsigned short	IHFreq;			///! input horizontal frequency
	unsigned int	MemAddr;		///! display memory address
	unsigned short	MemWid;		///! display memory width
	unsigned short	MemLen;		///! display memory length
	unsigned short	DispHSta;		///! display window horizental start
	unsigned short	DispVSta;		///! display window vertical start
	unsigned short	DispWid;			///! display widnow width
	unsigned short	DispLen;			///! display window length
}StructJPEGDisplayInfo;

typedef enum{
	SLR_2DCVT3D_MODE_DISABLE = 0,
	SLR_2DCVT3D_MODE_SG,
	SLR_2DCVT3D_FRAME_RATE_DOUBLE_MODE_PR,
} SLR_2DCVT3D_MODE;


typedef enum{
	SLR_IDMA_MODE_DISABLE = 0,
	SLR_IDMA_MODE_3D 		= 1,
	SLR_IDMA_MODE_3Dto2D = 2
} SLR_IDMA_MODE;


typedef enum{
	SLR_SIZE_TYPE_WID = 0,
	SLR_SIZE_TYPE_LEN,
} SLR_SIZE_TYPE;

#define CONTENT_MAX  20
typedef struct VSC_DRIVER_STRING{
	char timing_index_str[CONTENT_MAX];
	char rtnr_mode_str[CONTENT_MAX];
	char od_en_str[CONTENT_MAX];
	char dexc_en_str[CONTENT_MAX];
	char free_run_str[CONTENT_MAX];
	char ve_comp_str[CONTENT_MAX];
	char dipqc_bit_str[CONTENT_MAX];
	char ma_snr_str[CONTENT_MAX];
}VSC_DRIVER_STRING;

typedef struct VSC_DRIVER_RESOURCE{
	unsigned int magic_num;
	bool valid;
	VSC_DRIVER_STRING vsc_driver_string[TIMING_INVAILD];
}VSC_DRIVER_RESOURCE ;

typedef enum _BBD_FUNCTION_STAGE{
	BBD_FUNCTION_DONE = 0,//driver ready or disable
	BBD_FUNCTION_REQ_VGIP_DRIVER = 1,//set vgip driver
	BBD_FUNCTION_REQ_ENABLE = 2,//request to BBD detect
}_BBD_FUNCTION_STAGE;
typedef enum _I3DDMA_CAP_ENABLE_MASK{
	I3DDMA_NN_ENABLE_CAP = 0,
	VT_ENABLE_CAP = 1,
	FILM_MODE_ENABLE_CAP = 2,
	I3DDMA_CAP_EANBLE_MAX_MASK = 3,
}I3DDMA_CAP_ENABLE_MASK;

/*================================== Function ===============================*/
/*============================================================================*/

void Set_3d_type(unsigned char type);
unsigned char Get_3d_type(void);

/**
 * drvif_scaler_ddomain_switch_irq
 * switch (enable/disable) D-Domain IRQ
 *
 * @param <enable>	{ enable or disable D-Domain IRQ }
 * @return 			{ void }
 *
 */
void drvif_scaler_ddomain_switch_irq(unsigned char enable);

void drvif_scaler_ddomain_DTG_line_compare_irq(unsigned char enable, unsigned char vlc_mode, unsigned int vlc_src_sel, unsigned int dtg_vln);

/*============================================================================*/
/**
 * drvif_scaler_poweron_init
 * Initial settings for scaler from power saving mode or power-off mode.
 *
 * @param <void>		{ void }
 * @return 			{ void }
 *
 */
void drvif_scaler_poweron_init(void);


/**
 * fwif_scaler_wait_for_event
 * Wait for evnet specifed status
 *
 * @param <void>		{ void }
 * @return 			{ void }
 *
 */
//unsigned char fwif_scaler_wait_for_event(unsigned char domain, unsigned char event);
unsigned char fwif_scaler_wait_for_event(unsigned int addr, unsigned int event);
unsigned char fwif_scaler_wait_for_event1(unsigned int addr, unsigned int event);
unsigned char fwif_scaler_wait_for_event_done(unsigned int addr,
					unsigned int event);

/*============================================================================*/
/**
 * Reset the scaler.
 *
 * @param <void>		{ void }
 * @return 			{ void }
 *
 */
//void Scaler_EnableDisplayOutput(void);

/*============================================================================*/
/**
 * drvif_scaler_disable_display_output
 * Reset the scaler.
 *
 * @param <void>		{ void }
 * @return 			{ void }
 *
 */
void drvif_scaler_disable_display_output(void);

/*============================================================================*/
/**
 * Reset the scaler.
 *
 * @param <void>		{ void }
 * @return 			{ void }
 *
 */
//void Scaler_WriteInitialTable(void);

/*============================================================================*/
/**
 * Reset the scaler.
 *
 * @param <void>		{ void }
 * @return 			{ void }
 *
 */
void fw_scaler_set_vgip_capture(unsigned char display);
void scaler_set_vgip_capture_shift(unsigned char display, char shiftx, char shifty);
/*============================================================================*/
/**
 * ScalerSetScaleUpInSize
 * Enable/disable vertical or horizontal scaling up,
 * including main and sub display. It is to set scale control register.
 * @param <info> {info structure}
 * @nWid
 * @nLen
 * @return { none }
 *
 */
void Scaler_SetScaleUpInputSize(unsigned int nWid, unsigned int nLen);
void Scaler_IrqInitial(void);
void	DeintelacerControl(void);
void drv_scaler_adjust_ivs_delay(unsigned char display, unsigned short usIVSDelay);
void drv_scaler_adjust_ihs_delay(unsigned char display, unsigned short usIHSDelay);
void fw_scaler_update_mode_para(void);
void fw_scaler_enable_fs_wdg(unsigned char display, unsigned char enable);
void fw_scaler_enable_measure_wdg(unsigned char channel, unsigned char enable);
#ifdef CONFIG_SUPPORT_SRC_ADC
void fw_scaler_adc_nolock_wdg_ctrl(unsigned char display, unsigned char enable);
#endif
unsigned char  fwif_scaler_decide_display_cut4line(unsigned char disp);

unsigned char drvif_scaler_decide_idmaVgip_syncTo_mainVgip(void);

//void Scaler_OnMeasureIRQEnable(unsigned char channel, unsigned char bEnable);
//void drvif_scaler_vgip_irq_init(void);	// hsliao 20081231, move to linux driver
//void Scaler_OnMeasure1IRQInit(void);
//void Scaler_OnMeasure2IRQInit(void);
void fw_scaler_double_buffer_maskl(unsigned char display,unsigned int addr,  unsigned int andmask, unsigned int ormask);
void fw_scaler_double_buffer_outl(unsigned char display,unsigned int addr,  unsigned int val);
void fw_scaler_dtg_double_buffer_enable(unsigned char bEnable);
void fw_scaler_dtg_double_buffer_apply(void);

enum{
	VGIP_SRC_TMDS = 0,  //HDMI, DPRX
	VGIP_SRC_ADC,       //ADC/YPbPr/Pattern Gen (reserved)
	VGIP_SRC_ATV,       //VD/ATV
	VGIP_SRC_DISPD,     //DispD for Sub/DMA VGIP only (reserved)
	VGIP_SRC_HDR1,      //VODMA1
	VGIP_SRC_HDR2,
	VGIP_SRC_DPRX       //DPRX
};

enum{
	VGIP_MODE_ANALOG = 0,
	VGIP_MODE_DIGITAL
};

typedef enum {
	HDMI_STATE_POSITION = 0,//apply position
	HDMI_STATE_FINISH = 1,//apply finish
} HDMI_DOLBY_STATE;

void fw_scaler_set_vgip(unsigned char channel, unsigned char src, unsigned char mode);

enum{
	DISPLAY_MODE_FRC = 0,
	DISPLAY_MODE_FRAME_SYNC,
	DISPLAY_MODE_NEW_FLL_MODE,
    DISPLAY_MODE_RESET_VODMA
};

enum VO_FILM_TYPE{
	_VO_CAD_VIDEO = 1,	// Non film mode
	_VO_CAD_22,
	_VO_CAD_32,
	_VO_CAD_32322,
	_VO_CAD_334,
	_VO_CAD_22224,
	_VO_CAD_2224,
	_VO_CAD_3223,
	_VO_CAD_55,
	_VO_CAD_66,
	_VO_CAD_44,
	_VO_CAD_1112,
	_VO_CAD_11112,
	_VO_CAD_122,
	_VO_CAD_11123,
	_VO_CAD_BAD_EDIT,
	VO_FILM_TYPE_NONE,
};

typedef struct _RPC_SHARE_MEM_INFO {
	#ifdef CONFIG_ARM64
	unsigned long			ulOffset;			//Memory offset
	#else
	unsigned int 			ulOffset;			//Memory offset
	#endif
	unsigned int			ulSize;				//Memory size
} RPC_SHARE_MEM_INFO;
typedef enum {
    FILM_MODE_BUFFER_NONE,
    FILM_MODE_BUFFER_READY,
    FILM_MODE_BUFFER_USING,
}FILE_MODE_BUFFER_STATUS;

typedef struct _FILM_MODE_BUFFER_GROUP_INFO {
    unsigned int firstBufAddr;
    unsigned int secondBufAddr;
}FILM_MODE_BUFFER_GROUP_INFO;

typedef enum{//only consider main
    DP_NO_REGEN_VSYNC = 0,          //default
    DP_REGEN_VSYNC_AT_DMA_VGIP,     //regen vsync at dma vgip
    DP_REGEN_VSYNC_AT_HDR1,         //regen vsync at hdr1 block
    DP_REGEN_VSYNC_AT_MAIN_VGIP,    //regen vsync at main vgip
}_DP_VSYNC_REGEN_POS;

typedef unsigned short (*GET_INPUT_WID_F)(unsigned short);

unsigned char drvif_scaler_get_display_mode(void);
void drvif_scaler_set_display_mode(unsigned char timing_mode);
void drvif_scaler_set_display_modeByDisp(unsigned char timing_mode,unsigned char _channel);
void drvif_scaler_set_Disp_line_4x(unsigned char ucEnable);
void drvif_scaler_set_display_mode_frc2fsync_by_hw(unsigned char ucReset);
void drvif_scaler_set_display_mode_frc2fsync_check_lock(void);
void drvif_scaler_set_display_mode_frc2fsync_by_hw_wait_lock(void);
void drvif_scaler_set_display_mode_frc2fsync_by_hw_speedup(void);
void drvif_scaler_set_display_mode_to_framesync_by_hw(void);
//void fwif_osd_load_bitmap(unsigned char *array, unsigned int addr, int len, unsigned char ucFrom);
//void fwif_osd_load_bitmap_256_palette(unsigned char *array, unsigned char ucFrom);
void drvif_scaler_check_vgip_data_dclk(unsigned char channel);
void fw_scaler_set_sample(unsigned char channel, unsigned char enable);
void fw_scaler_set_vgip_3d_mode(unsigned char channel, unsigned char enable);
void fw_scaler_set_Main2_vgip_3d_mode(unsigned char enable);
void fw_scaler_ctrl_vgip_regen_vsync(unsigned char channel, unsigned char enable);
void Set_Val_DP_Vsync_Regen_Pos(unsigned char src);
unsigned char Get_Val_DP_Vsync_Regen_Pos(void);
unsigned char decide_dp_source_regen_vsync(void);
unsigned char fw_scaler_get_M_capture_LR_ctrl_by_HW(void);
void fw_scaler_set_vgip_frame_packing_3d_mode(unsigned char enable);
//void Scaler_VStartIRQInit(void);
void drvif_scaler_dma_vgip_sta_irq_ctrl(unsigned char sta_enable);//control dma start vgip isr
void drvif_scaler_dma_vgip_end_irq_ctrl(unsigned char end_enable);//control dma end vgip isr
void drvif_scaler_vactive_sta_irq(unsigned char enable, unsigned char channel);
void drvif_scaler_vactive_end_irq(unsigned char enable, unsigned char channel);
void fw_scaler_cal_scale_down_size(void);
unsigned short apsys_get_input_width(unsigned short ori_width);

//unsigned char drvif_scaler_get_AutoMaEnableStatus(void);
void drvif_scaler_WriteTable(unsigned int* array);
unsigned char drvif_scaler_regGetInputWidCallBck(GET_INPUT_WID_F function);
unsigned char  fwif_scaler_decide_display_cut4line(unsigned char disp);
void fw_scaler_enable_measure_subwdg_magicpic(void);
void fw_scaler_disable_measure_subwdg_magicpic(void);
void scaler_OSD_md_domain_update(unsigned int addr);

unsigned char drvif_scaler3d_decide_3d_PR_enter_3DDMA(void);
unsigned char drvif_scaler3d_decide_2d_cvt_3d_PR_enter_3DDMA(void);
unsigned char drvif_scaler3d_decide_HDMI_framePacking_interlaced_status(void);
//unsigned char drvif_scaler3d_decide_HDMI_framePacking_auto_status(void);
unsigned char drvif_scaler3d_decide_HDMI_framePacking_auto_status(SCALER_DISP_CHANNEL channel);
unsigned char drvif_scaler3d_decide_3d_PR_enable_IDMA(void);
unsigned char drvif_scaler3d_decide_3d_SG_Disable_IDMA(unsigned char uc3dType);
unsigned char drvif_scaler3d_decide_3d_SG_Disable_IDMA_suport_FP_interlace(void);
unsigned char drvif_scaler3d_decide_2d_cvt_3d_SG_frc_in_mDomain(void);
unsigned char drvif_scaler3d_decide_vo_3d_output_frame_seq(void);
unsigned char drvif_scaler3d_decide_dispM_3d_output_frame_seq(void);
unsigned char drvif_scaler3d_decide_PR_output_120Hz(void);
unsigned char drvif_scaelr3d_decide_1080i_TB_enter_2ddi(void);
unsigned char drvif_scaelr3d_decide_is_3D_display_mode(void);
unsigned char drvif_scaelr3d_decide_is_3Dto2D_display_mode(void);

SLR_IDMA_MODE drvif_scaler3d_get_IDMA3dMode(void);
unsigned char drvif_scaler3d_decide_3d_SG_data_FRC(void);

void Scaler_set_film_mode_change_type(unsigned int enable);
unsigned int Scaler_get_film_mode_change_type(void);
unsigned int Scaler_get_memc_me_clock_off(void);
void Scaler_set_dynamic_framerate_change_flag(unsigned int enable);
unsigned int Scaler_get_dynamic_framerate_change_flag(void);

unsigned char drvif_scaler3d_decide_iv2dv_delay_line(void);
//SLR_2DCVT3D_MODE drvif_scaler3d_decide_2d_cvt_3d_mode(void);
unsigned char drvif_scaler3d_decide_isPR3dMode(void);
unsigned char drvif_scaler3d_decide_IsFRCMode(void);

int drvif_scaler3d_init_vo_3d_mode(unsigned char uc3dMode);
void drvif_scaler3d_set_voDropLine_for_4k2k_3d(unsigned char enable);
unsigned int drvif_scaler3d_get_vo_3dDropLineMode(void);
int drvif_scaler3d_set_panel_info(void);
void drvif_scaler_voFrameRateControl_init(void);
void drvif_scaler_voFrameRateControl_force(unsigned char iEnable,unsigned int theframerate);


unsigned char drvif_scaler4k2k_get_forceDisplayOutput_60Hz(void);
int drvif_scaler_voFrameRateControl_set_bwSavingMode(DISP_MODE flag_3d, unsigned char bwSavingEnable);
int drvif_scaler_voForceFrameRateControl_by_TvBox(unsigned int forceFrameRate, unsigned char bEnable);
#ifdef CONFIG_4K2K_VIDEO_EMEMC
void drvif_scalerEMEMC(bool iEnable);
#endif
int drvif_scaler3d_voFrameRateControl_set_forceMaxFrameRate(DISP_MODE flag_3d, unsigned char flag_forceEnable, unsigned char boolUpdate);
void drivf_scaler_reset_freerun(void);
void drivf_scaler_reset_freerun_60Hz(void);

// 3D PR Video PQ Improvement
unsigned char drvif_scaler3d_decide_3d_PR_PQ_improve(void);
void fw_scaler_set_3D_depth(void);
unsigned int get_scaler_memory_map_addr(void);
void WaitFor_3DTG_Period(unsigned int StartOffset, unsigned int EndOffset);
void WaitFor_DEN_STOP(void);
void WaitFor_DEN_STOP_Period(unsigned int PorchStartOffset, unsigned int PorchEndOffset);
void WaitFor_DEN_STOP_Period2(unsigned int PorchStartOffset, unsigned int PorchEndOffset);
//unsigned char Rt_Sleep(unsigned int  milliSec);
unsigned int Scaler_InitRPCShareMem(void);
void Scaler_FreeRPCShareMem(void);
long Scaler_SendRPC(unsigned int a_ulCmd, unsigned long a_ulValue, unsigned char a_ucValueOnly);

#ifdef CONFIG_ARM64
unsigned long Scaler_GetShareMemVirAddr(unsigned int a_ulCmd);
#else
unsigned int Scaler_GetShareMemVirAddr(unsigned int a_ulCmd);
#endif

unsigned int Scaler_ChangeUINT32Endian(unsigned int a_ulLittleIndian);
unsigned short Scaler_ChangeUINT16Endian(unsigned short a_usLittleEndian);
unsigned long long Scaler_ChangeUINT64Endian(unsigned long long a_ulLittleIndian);

unsigned char modestate_decide_double_dvs_enable(void);
unsigned char modestate_check_input_special_framerate(void);
unsigned char modestate_get_pr3d_in_fp_3d_mode(void);
unsigned char modestate_I3DDMA_check_idma_mode_enable(unsigned char cur3dType);
unsigned char modestate_get_vo3d_in_framePacking_FRC_mode(void);

//unsigned char Scaler_Get3DMode(void);
unsigned char Scaler_Get3DMode(SCALER_DISP_CHANNEL display);
//void Scaler_Set3DMode_Attr(unsigned char message);
//void Scaler_Set3DMode_Attr(SCALER_DISP_CHANNEL display, unsigned char message);
//unsigned char Scaler_Set3DMode(unsigned char message, bool muteAudio);
//unsigned char Scaler_Set3DMode(SCALER_DISP_CHANNEL display, unsigned char message, bool muteAudio);
unsigned char Scaler_Set3DLRSwap(unsigned char message);
unsigned char modestate_I3DDMA_get_In3dMode(void);
unsigned char Scaler_Get3D_IsFramePacking(void);
int Scaler_Set3D_VoAutoDetect_Enable(unsigned char enable);
unsigned char Scaler_Get3D_VoAutoDetectMode(void);
//void check_vo3d_detect_result(void);
void check_vo3d_detect_result(SCALER_DISP_CHANNEL display);
VO_3D_MODE_TYPE modestate_get_vo3d_detect_result(void);
//void modestate_HDMI_3D_format_auto_detection(void);
void modestate_HDMI_3D_format_auto_detection(SCALER_DISP_CHANNEL channel);
long Scaler_Kernel_SendRPC_to_Video(unsigned int rpc_cmd, unsigned int param_1);

void Scaler_Set_Record_I3DDMA_Capture_Index(unsigned int capture_index);

void modestate_decide3D_for_vdc(void);
//#define CONFIG_ENABLE_FRC_TO_FRAMESYNC_BY_HW 1
void Scaler_Set_FRC_Open_Fix_Lastline_2(unsigned char ucEnable);
unsigned char Scaler_Get_FRC_Open_Fix_Lastline_2(void);
unsigned char Scaler_EnterFRC_LastLine_Condition(void);
void Scaler_Set_FreeRun_To_FrameSync_By_HW_Enable(unsigned char ucEnable);
unsigned char Scaler_Get_FreeRun_To_FrameSync_By_HW_Enable(void);
void Scaler_Set_FreeRun_To_FrameSync_By_HW_LineBufferMode_Enable(unsigned char ucEnable);
unsigned char Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_Enable(void);
void Scaler_Set_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut(unsigned int Value);
unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_LineBufferMode_SpeedUp_TimeOut(void);
void Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp(unsigned char ucEnable);
unsigned char Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp(void);
void Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_TimeOut(unsigned int Value);
unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_TimeOut(void);
void Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_Porch(unsigned char Value);
unsigned char Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_Porch(void);
void Scaler_Set_FreeRun_To_FrameSync_By_HW_Delay(unsigned int delay);
unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_Delay(void);
void Scaler_Set_FreeRun_To_FrameSync_By_HW_FastMode(unsigned int Value);
unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_FastMode(void);
void Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2(unsigned char ucEnable);
unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL2(void);
void Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3(unsigned char ucEnable);
unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL3(void);
void Scaler_Set_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL4(unsigned char ucEnable);
unsigned int Scaler_Get_FreeRun_To_FrameSync_By_HW_SpeedUp_ByMcode_HW_CTRL4(void);
void Scaler_Set_FreeRun_To_FrameSync_By_I2D_CTRL(unsigned char ucEnable);
unsigned int Scaler_Get_FreeRun_To_FrameSync_By_I2D_CTRL(void);
void drvif_scaler_set_display_mode_frc2fsync_by_I2D_CTRL(void);
void drvif_scaler_set_display_mode_frc2fsync_I2D_disable(void);
void Scaler_Set_FreeRun_To_FrameSync_By_Reset_VODMA(unsigned int ucEnable);
unsigned int Scaler_Get_FreeRun_To_FrameSync_By_Reset_VODMA(void);
void drvif_scaler_set_display_mode_frc2fsync_by_Reset_VODMA(void);
void modestate_set_lowdelay_fractional_framesync(unsigned char _channel, unsigned int lowdelay_en);
void modestate_set_lowdelay_display_timing_gen_clock(unsigned char _channel, unsigned char lowdelay_en);
unsigned char get_vsc_src_is_hdmi_or_dp(void);
unsigned char get_vsc_run_pc_mode(void);//return run pc mode. return false: run no pc mode setting
void set_vsc_run_pc_mode(unsigned char flag);
unsigned int drvif_i3ddma_triplebuf_by_timing_protect_panel(void);
unsigned char drvif_i3ddma_get_triplebuf_flag(void);
void drvif_i3ddma_set_triplebuf_flag(unsigned char bEnable);
void Scaler_Set_VODMA_Tracking_I3DDMA(unsigned int ucEnable);
unsigned int Scaler_Get_VODMA_Tracking_I3DDMA(void);
void Scaler_Set_Reset_VODMA_Flag(unsigned int ucEnable);
unsigned int Scaler_Get_Reset_VODMA_Flag(void);
unsigned char drvif_scaler_check_vodma_reset_result(void);
void rpc_send_source_2p_and_vrr_info(unsigned char two_p_flag_new, unsigned char vrr_flag_new, unsigned char freesync_flag_new);//send source 2p and freesync vrr to video fw
void state_update_disp3d_info(void);
unsigned char get_3D_mode_enable(void);
void set_3D_mode_enable(unsigned char enable);
unsigned char get_3D_overscan_enable(void);
void set_3D_overscan_enable(unsigned char enable);
unsigned int get_3D_overscan_input_hsta(void);
void set_3D_overscan_input_hsta(unsigned int hsta);
unsigned int get_3D_overscan_input_vsta(void);
void set_3D_overscan_input_vsta(unsigned int vsta);
unsigned int get_3D_overscan_input_wid(void);
void set_3D_overscan_input_wid(unsigned int wid);
unsigned int get_3D_overscan_input_len(void);
void set_3D_overscan_input_len(unsigned int len);
unsigned int get_3D_overscan_output_hsta(void);
void set_3D_overscan_output_hsta(unsigned int hsta);
unsigned int get_3D_overscan_output_vsta(void);
void set_3D_overscan_output_vsta(unsigned int vsta);
unsigned int get_3D_overscan_output_wid(void);
void set_3D_overscan_output_wid(unsigned int wid);
unsigned int get_3D_overscan_output_len(void);
void set_3D_overscan_output_len(unsigned int len);
unsigned int get_field_alternative_3d_mode_enable(void);
void set_field_alternative_3d_mode_enable(unsigned int enable);
unsigned int get_line_alternative_3d_mode_enable(void);
void set_line_alternative_3d_mode_enable(unsigned int enable);
void check_3D_overscan_size_error(void);
void modestate_decide_pixel_mode(void);
void modestate_decide_fractional_framesync(void);
void modestate_decide_fractional_framesyncByDisp(unsigned char _channel);
void modestate_set_display_timing_gen(void);
void modestate_set_display_timing_genByDisp(unsigned char _channel);
void modestate_set_display_timing_gen_htotal_ByDisp(unsigned char _channel);
void modestate_set_display_timing_gen_clock_div(void);
void modestate_set_display_timing_gen_clock_divByDisp(unsigned char _channel);
void modestate_set_fractional_framesync(void);
void modestate_set_fractional_framesyncByDisp(unsigned char _channel);
void modestate_set_memc_pixel_mode(void);
void modestate_decide_dtg_m_mode(void);
void modestate_decide_dtg_m_modeByDisp(unsigned char _channel);
void Set_decide_dtg_m_mode_set_cinema_mode_flag(unsigned char enable);
unsigned char Get_decide_dtg_m_mode_set_cinema_mode_flag(void);
void modestate_decide_dtg_m_mode_scaler_set_cinema_mode(void);
void modestate_disp_flow_double_buf_enable(unsigned char enable);
void modestate_disp_flow_double_buf_apply(void);
void force_enable_datapath_clock(void);//force enable Xcdi_clock 0xb8022284 bit3
void ResetShareMem(void);
#ifdef CONFIG_ENABLE_SE_ROTATE_M_DOMAIN_BLOCK_MODE
typedef enum _MAINSUB_ROTATE_STATUS{
	MAIN_ROTATE_NON_ACTIVE = 0,	/*no opration*/
	MAIN_ROTATE_INIT_UZD = 1,	//UZD : 1920x1080=>1080x1080
	MAIN_ROTATE_INIT_VI = 2,	/*RPC to video fw to initial VI*/
	MAIN_ROTATE_UPDATE_BLOCK_READ = 3,	/*re init main source vodma */
	MAIN_ROTATE_DISPLAY_ON = 4,
}MAINSUB_ROTATE_STATUS;

void scaler_Rotate_VI_init_send_rpc(void);
void set_rotate_mode(unsigned char display, DIRECT_VO_FRAME_ORIENTATION value);
DIRECT_VO_FRAME_ORIENTATION get_rotate_mode(unsigned char display);
void Set_rotate_function(unsigned char display, unsigned char enable);
unsigned char Get_rotate_function(unsigned char display);
void set_ori_rotate_mode(unsigned char display, DIRECT_VO_FRAME_ORIENTATION value);
DIRECT_VO_FRAME_ORIENTATION get_ori_rotate_mode(unsigned char display);
void set_rotate_curState(unsigned char display, MAINSUB_ROTATE_STATUS value);
MAINSUB_ROTATE_STATUS get_rotate_curState(unsigned char display);
void VI_close(void);
void Scaler_HDMI_Rotate(char rotateon, ROTATE_MODE_LIST mode, unsigned char srcIdx);
unsigned int scaler_send_rotate_vflip_rpc(unsigned enable);
unsigned char get_2Pto1L_hflip_flag(void);
unsigned char get_rtnr_hflip_flag(void);
unsigned char get_vo_vflip_flag(unsigned char display);
unsigned char get_vi_vflip_flag(unsigned char display);
unsigned char get_mdomain_vflip_flag(unsigned char display);
DIRECT_VO_FRAME_ORIENTATION get_SE_rotate_mode(unsigned char display);

#endif


#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
void Scaler_Set_VDC_27MHz_Mode_Enable(unsigned char ucEnable);
unsigned char Scaler_Get_VDC_27MHz_Mode_Enable(void);
VD_27M_OVERSAMPLING_MODE Scaler_Enter_VDC_27MHz_Mode_Condition(SCALER_DISP_CHANNEL Display);
void Scaler_Set_VDC_27MHz_Mode_SD_Width(unsigned int ScalingDown_Width);
unsigned int Scaler_Get_VDC_27MHz_Mode_SD_Width(void);
#endif

void set_HDMI_HDR_mode(HDR_MODE mode);
HDR_MODE get_HDMI_HDR_mode(void);
HDR_MODE get_OTT_HDR_mode(void);
void set_OTT_HDR_mode(HDR_MODE mode);

void set_HDMI_Dolby_VSIF_mode(DOLBY_HDMI_VSIF_T vsif_mode);
void set_HDMI_Dolby_H14B_VSIF_mode(DOLBY_HDMI_VSIF_T vsif_mode);
DOLBY_HDMI_VSIF_T get_HDMI_Dolby_VSIF_mode(void);
void set_PreHDMI_Dolby_VSIF_mode(DOLBY_HDMI_VSIF_T Vsif_mode);
unsigned char is_dolby_vision_tunnel_mode(void);
void scaler_dolby_hdmi_smooth_toggle(unsigned char isSDRtoEDR);
void drvif_Dolby_HDR_disable_DM_LUT(void);
unsigned char decide_auto_scaler_run(unsigned char display);
void set_vsc_connect_ready(unsigned char display, unsigned char ready);
void set_force_i3ddma_enable(unsigned char enable);
unsigned char get_force_i3ddma_enable(unsigned char display);
unsigned char get_vdec8Kflag(unsigned char display);
int drvif_i3ddma_triplebuf_flag(void);
void drvif_scaler_i3ddma_vodma_fifo_setting(unsigned char enable);
char drivf_Sub_I2rnd_SetupVODMA(void);
void WaitFor_DEN_STOP_UZUDTG(void);
void WaitFor_Frame_timer(void);
unsigned char drvif_scaler_get_abnormal_dvs_long_flag(void);
void drvif_scaler_wclr_abnormal_dvs_long_short(void);
void drvif_scaler_enable_abnormal_dvs_protect(unsigned char bEnable_long, unsigned char bEnable_short);
void drvif_scaler_check_abnormal_dvs_flag(void);

void drvif_scaler_set_abnormal_dvs_event_flag(unsigned int flag);
unsigned int drvif_scaler_get_abnormal_dvs_event_flag(void);

void drvif_scaler_check_ivs_over_threshold(unsigned int enable);
void drvif_scaler_check_main_onlinemeasure_flag(void);
unsigned int drvif_memory_get_data_littlealign(unsigned int Value, unsigned int unit);
void set_ori_HDMI_HDR_mode(HDR_MODE mode);//Set hdmi hdr mode at scaler beginning
HDR_MODE get_ori_HDMI_HDR_mode(void);//Get record hdmi hdr mode
void is_DTV_flag_set(unsigned char enable);
unsigned char is_DTV_flag_get(void);
void drvif_scaler_i3ddma_vodma_fifo_input_fast(unsigned char display, unsigned int input_frame_rate, unsigned char interlace);
void drvif_scaler_sub_mdomain_switch_irq(unsigned char enable);
#ifdef SPEEDUP_NEW_SCALER_FLOW_FOR_ATV
unsigned char check_atv_mode_id_change(void);
#endif

void scaler_hdr_mode_setting(unsigned char display, unsigned char hdrtype);
void set_hdr_semaless_active(unsigned char enable);
void scaler_hdr_mode_setting_IMD(unsigned char display, unsigned char hdrtype);

unsigned char get_hdr_semaless_active(void);

void copy_current_driver_pattern(VSC_DRIVER_PATTERN driver_pattern);
void copy_current_driver_table(VSC_DRIVER_PATTERN *driver_table);
INPUT_TIMING_INDEX get_current_pattern_index(void);
unsigned int get_current_driver_pattern(DRIVER_LIST_INFO info);
void print_current_driver_pattern(void);
void driver_config_data_to_video(void);
int parse_xml_driver_string(VSC_DRIVER_STRING *driver_str);
void enable_clock_mux_new_mode(void);//this is ECN for dispi clock non-glitch mux. MAC6-566

/*merlin4 new function*/
//Re-order control for HDMI Dolby
void set_i3ddma_dolby_reorder_mode(unsigned char enable);
unsigned char get_i3ddma_dolby_reorder_mode(void);
void i3ddma_dolby_reorder_update_color_format(void);

//int xml_parse_config(void);
int copy_driver_table_from_bootload(void);
#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
void init_dolby_vision_dm_path_sem(void);
struct semaphore *get_dolby_vision_dm_path_sem(void);
void dolby_ott_dm_init(void);
void set_sharememory_hdmi_dolby_visf(unsigned int enable);
#endif


unsigned short get_scaler_connect_source(unsigned char display);//get vsc connect source form AP
unsigned short  get_scaler_ivs_framerate(void);//get scale main path ivs frame rate. current is for HDMI VRR
unsigned char check_hdmi_uhd_condition(void);//for memc used
void scaler_vtop_dolby_mode_setting(unsigned char hdrtype);
void set_HFR_mode(unsigned char enable);//enable or disable hfr mode
unsigned char get_HFR_mode(void);
void two_pixel_mode_ctrl(unsigned char enable);//for 4k 120/ 2 pixel mode
void twopixel_disable_sub_vgip_clock(void);
void twopixel_2_oneline_ctrl(unsigned char enable);
void get_vtop_input_info(unsigned char *twopixel_mode, unsigned char *hdmi_input, unsigned char *color_format);//get vtop input information
unsigned char check_hdr_block_use(unsigned char display);//checj hdr block use or not return 1: use  return 0: no use
unsigned char get_vsc_run_adaptive_stream(unsigned char display);
void set_vsc_run_adaptive_stream(unsigned char display, unsigned char flag);
void set_vsc_gamemode_datafrc_mode_flag(unsigned char flag);
unsigned char get_vsc_gamemode_datafrc_mode_flag(void);
unsigned char decide_hdmi_hfr_rerun(StructSrcRect input_region, StructSrcRect ori_region, StructSrcRect output_region);
unsigned char get_hdmi_4k_hfr_mode(void);//two pixel mode enable
unsigned char is_4k_hfr_mode(unsigned char display);
unsigned char force_enable_two_step_uzu(void);//TRUE: use two step UZU not normal uzu
unsigned char get_scaling_down_status(void);
unsigned char get_hdmi_vrr_4k60_mode(void);
unsigned char decide_2step_uzu_merge_sub(void);
void hdmi_vtop_hfr_control(void);
void set_hdr_fake_porch(int enable);
void drv_set_datafs_gatting_enable_done(unsigned char enable);
unsigned char drv_get_datafs_gatting_enable_done(void);
unsigned short get_scaler_ivs_framerate(void);//get scale main path ivs frame rate. current is for HDMI VRR
unsigned char get_scaler_qms_mode_flag(void);
void Scaler_set_dynamic_framerate_change_flag(unsigned int enable);

void Scaler_Set_Cinema_Mode(unsigned int cinema_mode_enable, unsigned int run_vo_cinema);
unsigned char Scaler_get_realcinema_mode_condition(void);

unsigned char drvif_wait_timeout_check_by_counter(unsigned int base_time, unsigned int timeout);//if timeout return True  timeout : unit is ms

void output_data_access_data(void);//data access output print 
void enable_data_access(unsigned char display, unsigned char reset);//enable data access
void check_enable_data_access_background(void);//enable data access at back ground
unsigned char get_HD_FHD_platform_model(void);

typedef struct _I3DDMA_NN_CROP_ATTR_T_{
	unsigned char h_num;//horizon crop number
	unsigned char v_num;//vertical crop number
	unsigned int h_dis;//horizon distance
	unsigned int v_dis;//vertical distance
	unsigned int x; //crop size
	unsigned int y;
	unsigned int w;
	unsigned int h;
}I3DDMA_NN_CROP_ATTR_T;

void decide_scaler_hdmi_hfr_mode(void);//save scaler need to run 4k120 or not. decide at scaler start
unsigned char get_MEMC_bypass_status_refer_platform_model(void);
unsigned char check_vsc_connect_source(void);
void Scaler_Set_Vo_Force_60_Framerate_State(unsigned int state);
unsigned int Scaler_Get_Vo_Force_60_Framerate_State(void);

void BSP_ERROR_BUFFER_RESET(void);//set search state
void BSP_ERROR_CHECK_REPORT(unsigned char *outputstring);//check BSP error buffer
void check_bsp_error_event(unsigned int err_type, unsigned char video_runtime_suspend_status);//check whether set error event flag
void vbe_disp_set_VRR_set_display_vtotal_60Hz(void);

unsigned char modestate_get_mdomain_frc_flag(unsigned char _channel);
unsigned int modestate_get_mdomain_frc_vfreq(unsigned char _channel);
unsigned char modestate_check_mdomain_frc_scaler_rerun(unsigned char _channel);
unsigned char Get_Val_vsc_run_pc_mode(void);

unsigned char get_scaler_qms_mode_flag(void);//value from drvif_Hdmi_GetQmsStatus
void set_scaler_qms_mode_flag (unsigned char enable);//decidescaler use qms mode
unsigned char check_hdmi_current_qms_status(unsigned short* stable_vfreq);//get hdmi qms status form hdmi
void update_hdmi_qms_info_shamemory(void);//updatehdmi qms info to share memory
unsigned char current_is_hdmi_qms_mode(void);//get hdmi qms mode from hdmi
void update_qms_i3ddma_block_write_idx(void);//update write index at capture done
unsigned int get_qms_current_framerate(void);//get current frame rate for qms
unsigned char get_vsc_run_pc_mode(void);//return run pc mode. return false: run no pc mode setting
void set_vsc_run_pc_mode(unsigned char flag);
void calculate_carveout_size(unsigned char display);
void Set_Val_dp_input_source(unsigned char display, unsigned char type);
unsigned char modestate_check_input_special_framerate(void);
unsigned char check_qms_unstable_period(void);//QMS scaler is active and mcost =0
void qms_notify_memc_input_output_frame_rate(void);//call void Scaler_MEMC_SetInOutFrameRateByDisp(SLR_MAIN_DISPLAY) to update the frame rate info to memc
void drvif_scaler_memc_enable_frc2fsync_interrupt(unsigned char enable);
void drvif_scaler_memc_enable_frc2fsync_HW_speedup_vtotal(unsigned char bEnable);
void drvif_scaler_memc_set_display_mode_frc2fsync_by_hw_speedup(unsigned char frc2fsync);
#ifdef BUILD_QUICK_SHOW
void drvif_scaler_set_display_mode_QS(UINT8 timing_mode);
#endif
unsigned char get_vdec_4k_hdr_mode(unsigned char display);
void decide_i3ddma_2p_mode_on_off(void);
void set_i3ddma_2p_mode_flag(unsigned char bOnoff);
unsigned char get_i3ddma_2p_mode_flag(void);
unsigned char drvif_wait_timeout_check_by_counter(unsigned int base_time, unsigned int timeout);//if timeout return True  timeout : unit is ms
unsigned char drvif_wait_time_by_counter(unsigned int base_time, unsigned int wait_time);//if return True: ok wait_time : unit is ms. use for at least time
unsigned int drvif_report_cost_time_by_counter(unsigned int base_time);//report wait_time : unit is ms
unsigned char Get_Val_scaler_low_power_mode(void);//get to enter low power mode or not
void Set_Val_scaler_low_power_mode(unsigned char condition);//decide to enter low power mode or not

#ifdef __cplusplus
}
#endif

#endif // #ifndef _SCALER_H_
/*======================== End of File =======================================*/
/**
*
* @}
*/

