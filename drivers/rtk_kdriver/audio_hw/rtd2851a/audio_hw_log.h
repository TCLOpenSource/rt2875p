/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2013 *
 * All rights reserved.                                       *
 *============================================================*/

#ifndef AUDIO_HW_LOG_H
#define AUDIO_HW_LOG_H

#include <linux/kernel.h>
#include <linux/time.h>
#include <rtd_log/rtd_module_log.h>

#ifndef __ALOG_SUBTAG
#define __ALOG_TIME_TAG	"[%ld:%06ld] "
#else
#define __ALOG_TIME_TAG	"[%ld:%06ld][" __ALOG_SUBTAG "] "
#endif

#ifdef __ALOG_ENABLE
#define alog_emerg(fmt, ...)	\
	rtd_pr_audio_hw_emerg(rtd_fmt(__ALOG_SUBTAG, fmt), ##__VA_ARGS__)
#define alog_alert(fmt, ...)	\
	rtd_pr_audio_hw_alert(rtd_fmt(__ALOG_SUBTAG, fmt), ##__VA_ARGS__)
#define alog_crit(fmt, ...)	\
	rtd_pr_audio_hw_crit(rtd_fmt(__ALOG_SUBTAG, fmt), ##__VA_ARGS__)
#define alog_err(fmt, ...)	\
	rtd_pr_audio_hw_err(rtd_fmt(__ALOG_SUBTAG, fmt), ##__VA_ARGS__)
#define alog_warning(fmt, ...)	\
	rtd_pr_audio_hw_warn(rtd_fmt(__ALOG_SUBTAG, fmt), ##__VA_ARGS__)
#define alog_notice(fmt, ...)	\
	rtd_pr_audio_hw_notice(rtd_fmt(__ALOG_SUBTAG, fmt), ##__VA_ARGS__)
#define alog_info(fmt, ...)	\
	rtd_pr_audio_hw_info(rtd_fmt(__ALOG_SUBTAG, fmt), ##__VA_ARGS__)
#define alog_debug(fmt, ...)	\
	rtd_pr_audio_hw_debug(rtd_fmt(__ALOG_SUBTAG, fmt), ##__VA_ARGS__)
#define alog(fmt, ...)		\
	rtd_pr_audio_hw_info( rtd_fmt(__ALOG_SUBTAG, fmt), ##__VA_ARGS__)
#else
#define alog_emerg(fmt, ...)	((void)0)
#define alog_alert(fmt, ...)	((void)0)
#define alog_crit(fmt, ...)	((void)0)
#define alog_err(fmt, ...)	((void)0)
#define alog_warning(fmt, ...)	((void)0)
#define alog_notice(fmt, ...)	((void)0)
#define alog_info(fmt, ...)	((void)0)
#define alog_debug(fmt, ...)	((void)0)
#define alog(fmt, ...)		((void)0)
#endif

#ifdef __ALOG_CHK_ENABLE
extern bool alog_chk_enable(void);
extern struct timeval alog_chk_gettimeofday(void);

#define alog_chk(fmt, ...) \
(void)({ \
	bool check__ = alog_chk_enable(); \
	struct timeval t___ = alog_chk_gettimeofday(); \
	if (check__) { \
		rtd_pr_audio_hw_info( __ALOG_TIME_TAG fmt, \
			t___.tv_sec & 0xffff, t___.tv_usec, ##__VA_ARGS__); \
	} \
	(void)check___; \
})
#else
#define alog_chk(fmt, ...)	((void)0)
#endif

#endif
