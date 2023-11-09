#ifndef __IRRC_TV005_H__
#define __IRRC_TV005_H__

#include "irrc_common.h"
#if defined(CONFIG_RTK_KDRV_TV005_IR)
#undef IR_PROTOCOL
#define IR_PROTOCOL     RAW_NEC
#endif
#endif
