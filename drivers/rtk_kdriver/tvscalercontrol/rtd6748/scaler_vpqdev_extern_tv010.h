#ifndef __SCALER_VPQDEV_EXTERN_H
#define  __SCALER_VPQDEV_EXTERN_H


/**
 * Video Picture Quality Information.
 *
 * @see DDI Implementation Guide
*/
#include <scaler/vipCommon.h>
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/vipCommon.h>
#else
	#include <scalercommon/scalerDrvCommon.h>
#endif

#include <tvscalercontrol/vip/film.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/icm.h>
//#include <tvscalercontrol/vip/scalerColor_tv010.h>

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

#endif
