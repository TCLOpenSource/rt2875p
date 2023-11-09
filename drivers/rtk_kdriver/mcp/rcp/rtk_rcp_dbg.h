#ifndef __RTK_RCP_DBG_H__
#define __RTK_RCP_DBG_H__

#include <rtd_log/rtd_module_log.h>

#define RCP_DEBUG(fmt, args...)             rtd_pr_rcp_debug(fmt , ## args)
#define RCP_INFO(fmt, args...)                rtd_pr_rcp_info(fmt , ## args)
#define RCP_NOTICE(fmt, args...)            rtd_pr_rcp_notice(fmt , ## args)
#define RCP_WARNING(fmt, args...)         rtd_pr_rcp_warn(fmt , ## args)
#define RCP_ERR(fmt, args...)                  rtd_pr_rcp_err(fmt , ## args)

void  rtk_rcp_dump_mem(unsigned char* data, unsigned int len);
#define rtk_rcp_dump_data_with_text(data, len ,fmt, args...)\
do {\
                RCP_DEBUG(fmt, ## args);\
                rtk_rcp_dump_mem(data, len);\
}while(0)


#endif
