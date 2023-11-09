#ifndef __IRRC_TV013_H__
#define __IRRC_TV013_H__

#include <rtk_kdriver/rtk_gpio.h>
#include "irrc_common.h"

#if defined(CONFIG_RTK_KDRV_HAIER_IR)
#undef IR_PROTOCOL
#define IR_PROTOCOL     NEC
#endif

#endif
