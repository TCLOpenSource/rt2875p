#ifndef __RTK_DSCD_DEBUG_H__
#define __RTK_DSCD_DEBUG_H__

#include <rtd_log/rtd_module_log.h>
///////////////////////////////////////////////////////////////////////////////////////

//#define HDMI_DSC_DEBUG_EN
#ifdef  RTK_DSC_DEBUG_EN
    #define RTK_DSC_DBG(fmt, args...)      rtd_pr_dscd_debug("[DSC][DBG]" fmt, ##args)
#else
    #define RTK_DSC_DBG(fmt, args...)
#endif

#define RTK_DSC_INFO(fmt, args...)     rtd_pr_dscd_info("[DSC][INFO]" fmt, ##args)
#define RTK_DSC_ERR(fmt, args...)      rtd_pr_dscd_err("[DSC][ERR]" fmt, ##args)
#define RTK_DSC_EMG(fmt, args...)      rtd_pr_dscd_emerg("[DSC][EMG]" fmt, ##args)
#define RTK_DSC_WARN(fmt, args...)      rtd_pr_dscd_warn("[DSC][WARN]" fmt, ##args)

#endif // __RTK_DSCD_DEBUG_H__
