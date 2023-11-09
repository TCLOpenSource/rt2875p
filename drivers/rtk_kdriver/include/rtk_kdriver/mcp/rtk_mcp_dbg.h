#ifndef __MCP_DBG__
#define __MCP_DBG__
#include <rtd_log/rtd_module_log.h>
#define TAG_NAME "MCP"
/************************************************************************
 *  rtk log define
 ************************************************************************/
#if 0
#define mcp_debug(fmt, args...)\
        rtd_pr_mcp_debug(fmt , ## args)
#else
#define mcp_debug(fmt, args...)
#endif
#define mcp_info(fmt, args...)\
        rtd_pr_mcp_info(fmt , ## args)


#define mcp_notice(fmt, args...)\
        rtd_pr_mcp_notice(fmt , ## args)
#define mcp_warning(fmt, args...)\
        rtd_pr_mcp_warn(fmt , ## args)
#define mcp_err(fmt, args...)\
        rtd_pr_mcp_err(fmt , ## args)

#endif //__MCP_DBG__
