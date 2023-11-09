#ifndef __SCALER_VPQLEDDEV_H
#define  __SCALER_VPQLEDDEV_H

#include <scaler/vipCommon.h>
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
	#include <scalercommon/scalerDrvCommon.h>
#endif

typedef enum {
	PQ_LED_DEV_NOTHING = 0,
	PQ_LED_DEV_INIT_DONE,
	PQ_LED_DEV_UNINIT,
} PQ_LED_DEV_STATUS;

typedef enum {
     LOCALDIMMING_DEMOTYPE_LINEDEMO = 0,
     LOCALDIMMING_DEMOTYPE_LEFTRIGHT,
     LOCALDIMMING_DEMOTYPE_TOPBOTTOM,
     LOCALDIMMING_DEMOTYPE_SIZE_MAX
} LOCALDIMMNG_DEMO_TYPE_T;
void vpqled_do_resume(void);
void vpqled_HAL_VPQ_LED_LDCtrlSPI(unsigned char *LDCtrlSPI);
unsigned char vpqled_get_LD_GetAPL_TV006(UINT16* BLValue);
void vpqled_HAL_VPQ_LED_LDCtrlDemoMode(LOCALDIMMNG_DEMO_TYPE_T bType, BOOLEAN bCtrl);

#endif
