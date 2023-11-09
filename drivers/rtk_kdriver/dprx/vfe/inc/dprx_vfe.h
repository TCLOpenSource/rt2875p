#ifndef __DPRX_VFE_H__
#define __DPRX_VFE_H__

#include <dprx_platform.h>
#include <dprx_adapter.h>

#define MAX_DPRX_PORT     2

extern unsigned char g_dprx_vfe_print_ctrl;
extern unsigned char g_dprx_vfe_suspend_mode;

#define DPRX_VFE_TRACE_EN       1

//======== vfe layer internal API ========
extern dprx_dev_t* _vfe_dprx_drv_get_dprx_adapter(unsigned char ch);

/*------------------------------------------------
 * Func : dprx_vfe_create_dprx_adapter_by_config
 *
 * Desc : create specific Platform config setting
 *
 * Para : p_cfg : port config
 *
 * Retn : NULL : failed / others : dprx handle
 *-----------------------------------------------*/
extern dprx_dev_t* dprx_vfe_create_dprx_adapter_by_config(DP_PORT_CFG* p_cfg);

extern dprx_dev_t* g_connected_dprx;

//======== vfe debug message control ========
#define DP_VFE_TRACE()                   if ((g_dprx_vfe_print_ctrl & DPRX_VFE_TRACE_EN)) { dprx_info("[VFE][TRACE] %s\n", __FUNCTION__); }
#define DP_VFE_TRACE_EX(fmt, args...)    if ((g_dprx_vfe_print_ctrl & DPRX_VFE_TRACE_EN)) { dprx_info("[VFE][TRACE] %s, " fmt, __FUNCTION__, ## args); }

#define DP_VFE_ERR(fmt, args...)         dprx_err("[VFE][ERR] " fmt, ## args)
#define DP_VFE_WARN(fmt, args...)        dprx_warn("[VFE][WARN] " fmt, ## args)
#define DP_VFE_INFO(fmt, args...)        dprx_info("[VFE][INFO] " fmt, ## args)
#define DP_VFE_DBG(fmt, args...)         dprx_dbg("[VFE][DEBUG] " fmt, ## args)

#endif // __DPRX_VFE_H__
