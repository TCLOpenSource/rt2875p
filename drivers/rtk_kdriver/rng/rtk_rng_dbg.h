#ifndef __RTK_RNG_DBG_H__
#define __RTK_RNG_DBG_H__
#include <rtd_log/rtd_module_log.h>

#define rng_notice(fmt, args...)      rtd_pr_rng_notice(fmt , ## args)
#define rng_warning(fmt, args...)   rtd_pr_rng_warn(fmt , ## args)
#define rng_err(fmt, args...)           rtd_pr_rng_err(fmt , ## args)

#endif