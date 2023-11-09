/*Copyright (C) 2007-2013 Realtek Semiconductor Corporation.*/
#ifndef __RTK_CEC_DEBUG_H__
#define __RTK_CEC_DEBUG_H__

#include <rtd_log/rtd_module_log.h>

#undef  TAG_NAME
#define TAG_NAME    "CEC"

extern unsigned int g_cec_debug_enable;

#define cec_dbg(fmt, args...)   if(unlikely(g_cec_debug_enable))  \
                                                        rtd_pr_cec_info("[Info] " fmt, ## args)
#define cec_rx_dbg(fmt, args...)   if(unlikely(g_cec_debug_enable))  \
                                                        rtd_pr_cec_debug("[RX DBG] " fmt, ## args)
#define cec_tx_dbg(fmt, args...)   if(unlikely(g_cec_debug_enable))  \
                                                        rtd_pr_cec_debug("[TX DBG] " fmt, ## args)

#define cec_info(fmt, args...)      rtd_pr_cec_info("[Info] "  fmt, ## args)
#define cec_warn(fmt, args...)      rtd_pr_cec_warn("[Warn] "  fmt, ## args)
#define cec_error(fmt, args...)     rtd_pr_cec_err("[Error] " fmt, ## args)

#endif /*__RTK_CEC_DEBUG_H__*/
