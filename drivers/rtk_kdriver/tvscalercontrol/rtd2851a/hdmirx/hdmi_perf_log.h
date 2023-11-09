
/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2004 *
 * All rights reserved.                                       *
 *============================================================*/

#ifndef _HDMI_PERF_LOG_H_
#define _HDMI_PERF_LOG_H_

#include "hdmiPlatform.h"
#include "hdmi_debug.h"


#if 0   // please enable it to enable time profiling

static unsigned long start_time = 0;

#define LOG_SCPU_TIME_SUB(func, end, start) \
{\
	HDMI_EMG("[- PERF_LOG] region of profile %s time = %ld ms\n", func, end - start); \
}

#define hdmi_time_mesaure_start()   do { start_time = hdmi_get_system_time_ms(); } while(0)
#define hdmi_time_mesaure_stop()    LOG_SCPU_TIME_SUB(__func__, hdmi_get_system_time_ms(), start_time)

#else

// no timing profiling enabled
#define hdmi_time_mesaure_start() 
#define hdmi_time_mesaure_stop() 

#endif

#endif // _HDMI_PERF_LOG_H_
