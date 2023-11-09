/*
    this file is used for rtd logbuf manager
*/
#ifndef __RTD_OS_PRINT_H__
#define __RTD_OS_PRINT_H__

#define MSG_LVL_TAG_EMERG      "M]["
#define MSG_LVL_TAG_ALERT      "A]["
#define MSG_LVL_TAG_CRIT       "C]["
#define MSG_LVL_TAG_ERR        "E]["
#define MSG_LVL_TAG_WARN       "W]["
#define MSG_LVL_TAG_NOTICE     "N]["
#define MSG_LVL_TAG_INFO       "I]["
#define MSG_LVL_TAG_DEBUG      "D]["
#define MSG_LVL_TAG_PRINT      "P]["

/*----------------------------------------
 * low level print API, this might be modified
 * by in the future, please don't use them
 * directly.
 *---------------------------------------*/
#if 0 //def BUILD_QUICK_SHOW

#include <printf.h>   // running under uboot

#define _rtd_pr_emerg(tag, fmt, ...)      printf2(tag fmt, ##__VA_ARGS__)
#define _rtd_pr_alert(tag, fmt, ...)      printf2(tag fmt, ##__VA_ARGS__)
#define _rtd_pr_crit(tag, fmt, ...)       printf2(tag fmt, ##__VA_ARGS__)
#define _rtd_pr_err(tag, fmt, ...)        printf2(tag fmt, ##__VA_ARGS__)
#define _rtd_pr_warn(tag, fmt, ...)       printf2(tag fmt, ##__VA_ARGS__)
#define _rtd_pr_notice(tag, fmt, ...)     printf2(tag fmt, ##__VA_ARGS__)
#define _rtd_pr_info(tag, fmt, ...)       printf2(tag fmt, ##__VA_ARGS__)
#define _rtd_pr_debug(tag, fmt, ...)      printf2(tag fmt, ##__VA_ARGS__)
#define _rtd_pr_print(level, tag, fmt, ...)      printf2(tag, fmt, ##__VA_ARGS__)

#else

#include <mach/rtk_log.h>

#define _rtd_pr_emerg(tag, fmt, ...)      rtd_printk(KERN_EMERG,   MSG_LVL_TAG_EMERG  tag, fmt, ##__VA_ARGS__)
#define _rtd_pr_alert(tag, fmt, ...)      rtd_printk(KERN_ALERT,   MSG_LVL_TAG_ALERT  tag, fmt, ##__VA_ARGS__)
#define _rtd_pr_crit(tag, fmt, ...)       rtd_printk(KERN_CRIT,    MSG_LVL_TAG_CRIT   tag, fmt, ##__VA_ARGS__)
#define _rtd_pr_err(tag, fmt, ...)        rtd_printk(KERN_ERR,     MSG_LVL_TAG_ERR   tag, fmt, ##__VA_ARGS__)
#define _rtd_pr_warn(tag, fmt, ...)       rtd_printk(KERN_WARNING, MSG_LVL_TAG_WARN   tag, fmt, ##__VA_ARGS__)
#define _rtd_pr_notice(tag, fmt, ...)     rtd_printk(KERN_NOTICE,  MSG_LVL_TAG_NOTICE tag, fmt, ##__VA_ARGS__)
#define _rtd_pr_info(tag, fmt, ...)       rtd_printk(KERN_INFO,    MSG_LVL_TAG_INFO   tag, fmt, ##__VA_ARGS__)
#define _rtd_pr_debug(tag, fmt, ...)      rtd_printk(KERN_DEBUG,   MSG_LVL_TAG_DEBUG  tag, fmt, ##__VA_ARGS__)
#define _rtd_pr_print(level, tag, fmt, ...)  rtd_printk(level,     MSG_LVL_TAG_PRINT  tag, fmt, ##__VA_ARGS__)

#endif

#endif /*__RTD_OS_PRINT_H__*/
