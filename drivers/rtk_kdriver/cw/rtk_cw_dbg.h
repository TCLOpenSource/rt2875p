#ifndef __RTK_CW_DBG_H__
#define __RTK_CW_DBG_H__
#include <rtd_log/rtd_module_log.h>

#define CW_DBG(fmt, args...)        rtd_pr_cw_debug("[DBG] " fmt, ##args)
#define CW_INFO(fmt, args...)       rtd_pr_cw_info("[Info] " fmt, ##args)
#define CW_WARNING(fmt, args...)        rtd_pr_cw_warn("[Warn] " fmt, ##args)
#define CW_ERR(fmt, args...)        rtd_pr_cw_err("[Err] " fmt, ##args)
#endif
