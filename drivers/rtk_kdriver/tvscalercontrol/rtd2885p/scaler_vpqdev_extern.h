#ifndef __SCALER_VPQDEV_EXTERN_H
#define  __SCALER_VPQDEV_EXTERN_H


/**
 * Video Picture Quality Information.
 *
 * @see DDI Implementation Guide
*/
#include <scaler/vipCommon.h>
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
	#include <scalercommon/scalerDrvCommon.h>
#endif


#include <tvscalercontrol/vip/film.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/icm.h>
//#include <tvscalercontrol/vip/scalerColor_tv010.h>
#include <tvscalercontrol/scaler/scalercolor_engmenu.h>

typedef enum{
	PQ_DEV_EXTERN_NOTHING = 0,
	PQ_DEV_EXTERN_INIT_DONE,
	PQ_DEV_EXTERN_OPEN_DONE,
	PQ_DEV_EXTERN_UNINIT,
	PQ_DEV_EXTERN_CLOSE,
} PQ_DEV_EXTERN_STATUS;

typedef struct _RTK_PQModule_T {
	unsigned int PQModule;
	unsigned char onOff;
} RTK_PQModule_T;
typedef struct _RTK_DEMO_MODE_T {
	unsigned int DEMO_MODE;
	unsigned char onOff;
} RTK_DEMO_MODE_T;

typedef enum{
	WCGMODE_WCG = 0,
	WCGMODE_AUTO,
	WCGMODE_STANDARD,
	WCGMODE_MAX,
} PQ_WCG_MODE_STATUS;

typedef struct _RTK_TV002_SR_T {
	unsigned short mode;// 0: off, 1: auto, 2: manual
	unsigned short value;// 0~100
} RTK_TV002_SR_T;

typedef enum _VPQ_EXTERN_GET_DYNAMIC_RANGE_ITEMS {
	VPQ_DYNAMIC_RANGE_SDR = 0,
	VPQ_DYNAMIC_RANGE_DOLBY_VISION,
	VPQ_DYNAMIC_RANGE_HDR10,
	VPQ_DYNAMIC_RANGE_HLG,
	VPQ_DYNAMIC_RANGE_TC,
	VPQ_DYNAMIC_RANGE_HDR10_PlUS,
	VPQ_DYNAMIC_RANGE_HDREFFECT,
	VPQ_DYNAMIC_RANGE_SDR_MAX_RGB,

	VPQ_DYNAMIC_RANGE_HLG_14,
	VPQ_DYNAMIC_RANGE_HLG_18,

	VPQ_EXTERN_GET_DYNAMIC_RANGE_ITEMS_MAX,

} VPQ_EXTERN_GET_DYNAMIC_RANGE_ITEMS;

typedef struct
{
   UINT8 flag_0;
   UINT8 flag_1;
   UINT8 flag_2;//h_sta
   UINT8 flag_3;//h_end
   UINT8 flag_4;//v_sta
   UINT8 flag_5;//v_end
   UINT8 flag_6;//RGBmode_en
   UINT8 flag_7;
   UINT8 flag_8;
   UINT8 flag_9;

}RTK_VIP_Flag;

typedef enum _PQ_EXTEND_DATA_ENUM
{
	PQ_EXTEND_DATA_ENUM_SHARP = 0,
	PQ_EXTEND_DATA_ENUM_NR,
	PQ_EXTEND_DATA_ENUM_ICM,
	PQ_EXTEND_DATA_ENUM_CSC,
	PQ_EXTEND_DATA_ENUM_DCC,
	PQ_EXTEND_DATA_ENUM_BLACK_ADJUST,
	PQ_EXTEND_DATA_ENUM_WBBL,
	PQ_EXTEND_DATA_ENUM_WB_BS,
	PQ_EXTEND_DATA_ENUM_MAX_NUM,

} PQ_EXTEND_DATA_ENUM;

char VPQEX_rlink_host_which_source(void);
char VPQEX_rlink_host_OSD_Sharpness_Info_Send(void);
char VPQEX_rlink_host_OSD_NNSR_Info_Send(unsigned int *OSD_NNSR_Info);
char VPQEX_rlink_host_Noise_Level_Info_Send(void);

void vpq_extern_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop);

#endif

