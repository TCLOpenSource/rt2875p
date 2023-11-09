#ifndef __RTK_DDC_DBG_H__
#define __RTK_DDC_DBG_H__
#include <rtd_log/rtd_module_log.h>
#define TAG_NAME                        "DDC"

extern int g_rtk_ddc_dbg;

#define RTK_DDC_DBG(fmt, args...)            \
{ \
    if(unlikely(g_rtk_ddc_dbg)) { \
        rtd_pr_ddc_err("[DBG] " fmt, ## args);  \
    } \
}

#define RTK_DDC_INFO(fmt, args...)      rtd_pr_ddc_info("[Info] " fmt, ## args)
#define RTK_DDC_WARNING(fmt, args...)   rtd_pr_ddc_warn("[Warn] " fmt, ## args)
#define RTK_DDC_ERR(fmt, args...)       rtd_pr_ddc_err("[Err] " fmt, ## args)
#define RTK_DDC_ALERT(fmt, args...)     rtd_pr_ddc_alert("[Alert] " fmt, ## args)

#endif
