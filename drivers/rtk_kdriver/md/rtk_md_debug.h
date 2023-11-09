#ifndef __RTK_MD_DEBUG_H__
#define __RTK_MD_DEBUG_H__
#include <rtd_log/rtd_module_log.h>
#ifdef MD_DEBUG
#define PDEBUG(fmt, args...) rtd_pr_md_debug(fmt, ## args)
#else
#define PDEBUG(fmt, args...)
#endif
#define PDEIFO(fmt, args...) rtd_pr_md_info(fmt, ## args)
#define PDEWAR(fmt, args...) rtd_pr_md_warn(fmt, ## args)
#define PDEERR(fmt, args...) rtd_pr_md_err(fmt, ## args)
#define PDEALT(fmt, args...) rtd_pr_md_alert(fmt, ## args)

#endif

