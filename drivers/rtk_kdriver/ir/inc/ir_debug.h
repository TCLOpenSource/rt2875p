#ifndef __IR_DEBUG_H__
#define __IR_DEBUG_H__
#include <rtd_log/rtd_module_log.h>

extern bool ir_decode_dbg_en;

#define TAG_NAME			"IR"

#define IR_DBG(fmt, args...)            \
{ \
	if(unlikely(ir_decode_dbg_en)) { \
		rtd_pr_ir_info("[DBG] " fmt, ## args);	\
	} \
}
   
#define IR_INFO(fmt, args...)       rtd_pr_ir_info("[Info] " fmt, ## args)
#define IR_WARNING(fmt, args...)	rtd_pr_ir_warn("[Warn] " fmt, ## args)
#define IR_ERR(fmt, args...)	 	rtd_pr_ir_err("[Err] " fmt, ## args)
#define IR_ALERT(fmt, args...)	 	rtd_pr_ir_alert("[Alert] " fmt, ## args)


#endif
