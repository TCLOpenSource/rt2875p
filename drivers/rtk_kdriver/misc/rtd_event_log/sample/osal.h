#ifndef __RTK_MODULE_EVENT_SAMPLE_H__
#define __RTK_MODULE_EVENT_SAMPLE_H__
#include <mach/rtk_timer.h>
#include <linux/kernel.h>
#include <linux/string.h>

/************************************************************************
 * Define Save
 ************************************************************************/

#if defined(CONFIG_REALTEK_EVENT_LOG)
#define rtd_sample_event_log(event_type, module, event, event_val, module_reserved)    event_log_kernel_save(EVENT_FW_SAMPLE, event_type, module, event, event_val, module_reserved)
#else
#define rtd_sample_event_log(event_type, module, event, event_val, module_reserved)
#endif /* CONFIG_REALTEK_EVENT_LOG  */
#endif /* __RTK_MODULE_EVENT_SAMPLE_H__ */