#ifndef __RTK_IRTX_DEBUG_H__
#define __RTK_IRTX_DEBUG_H__

#include <rtd_log/rtd_module_log.h>

#define irtx_dbg(fmt, args...)   rtd_pr_irtx_debug("[DBG] " fmt, ## args)
#define irtx_info(fmt, args...)      rtd_pr_irtx_info("[Info] "  fmt, ## args)
#define irtx_warn(fmt, args...)      rtd_pr_irtx_warn("[Warn] "  fmt, ## args)
#define irtx_error(fmt, args...)     rtd_pr_irtx_err("[Error] " fmt, ## args)

#endif
