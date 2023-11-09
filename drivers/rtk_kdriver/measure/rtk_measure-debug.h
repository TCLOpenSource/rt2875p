#ifndef __RTK_MEASURE_DEBUG_H__
#define __RTK_MEASURE_DEBUG_H__

#ifndef UT_flag
#define MEASURE_DBG_EN
#include <rtd_log/rtd_module_log.h>
#endif

#ifdef MEASURE_DBG_EN
    #define MEASURE_WARN(fmt, args...)              rtd_pr_measure_warn(" [L4]" fmt, ##args)
    #define MEASURE_EMG(fmt, args...)           rtd_pr_measure_emerg(" [L0]" fmt, ##args)
    #define MEASURE_INFO(fmt, args...)          rtd_pr_measure_info(" [L6]" fmt, ##args)
    #define MEASURE_NOTICE(fmt, args...)        rtd_pr_measure_notice(" [L5]" fmt, ##args)
#else
    #define MEASURE_WARN(fmt, args...)
    #define MEASURE_EMG(fmt, args...)
    #define MEASURE_INFO(fmt, args...)
    #define MEASURE_NOTICE(fmt, args...)
#endif

#endif // __RTK_MEASURE_PRIV_H__
