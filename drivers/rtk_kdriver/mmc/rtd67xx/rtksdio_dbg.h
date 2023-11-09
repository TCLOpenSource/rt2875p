#ifndef __RTKSDIO_DBG_H
#define __RTKSDIO_DBG_H


#include <rtd_log/rtd_module_log.h>
#define CR_NAME "CARD"


/************************************************************************
 *  rtk log define
 ************************************************************************/
#define CR_EMERG(fmt, args...)      \
	rtd_pr_sdio_emerg(fmt , ## args)

#define CR_ALERT(fmt, args...)   \
	rtd_pr_sdio_alert(fmt , ## args)

#define CR_CRIT(fmt, args...)   \
	rtd_pr_sdio_crit(fmt , ## args)

#define CR_ERR(fmt, args...)   \
	rtd_pr_sdio_err(fmt , ## args)

#define CR_WARNING(fmt, args...)   \
	rtd_pr_sdio_warn(fmt , ## args)

#define CR_NOTICE(fmt, args...)   \
	rtd_pr_sdio_notice(fmt , ## args)

#define CR_INFO(fmt, args...)   \
	rtd_pr_sdio_info(fmt , ## args)

#define CR_DEBUG(fmt, args...)   \
	rtd_pr_sdio_debug(fmt , ## args)




#endif
