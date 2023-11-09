#ifndef __IRRC_TV035_H__
#define __IRRC_TV035_H__

#include "irrc_common.h"
#if defined(CONFIG_RTK_KDRV_TV035_IR)
#undef IR_PROTOCOL
#define IR_PROTOCOL     RAW_KONKA
#endif
#endif
