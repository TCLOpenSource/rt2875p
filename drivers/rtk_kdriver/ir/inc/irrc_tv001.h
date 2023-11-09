#ifndef __IRRC_TV001_H__
#define __IRRC_TV001_H__

#include "irrc_common.h"

#if defined(CONFIG_RTK_KDRV_TV001_IR) && !defined(CONFIG_RTK_KDRV_TV002_IR)
#define IR_PROTOCOL     NEC
#endif

#endif
