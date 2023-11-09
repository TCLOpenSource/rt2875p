#ifndef __RTK_OTG_PLAT_H__
#define __RTK_OTG_PLAT_H__
#include <rtd_log/rtd_module_log.h>

#define rtk_otg_plat_dbg(fmt, args...)          rtd_pr_otg_plat_debug(fmt, ## args)
#define rtk_otg_plat_info(fmt, args...)         rtd_pr_otg_plat_info(fmt, ## args)
#define rtk_otg_plat_warn(fmt, args...)         rtd_pr_otg_plat_warn(fmt, ## args)
#define rtk_otg_plat_err(fmt, args...)          rtd_pr_otg_plat_err(fmt, ## args)

struct RTK_OTG_PRIVATE_DATA
{	
	struct platform_device *pdev;	
	unsigned int io_base;	
	unsigned int id;
};

#endif