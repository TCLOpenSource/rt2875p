#ifndef __RTK_OTP_DEBUG_H__
#define __RTK_OTP_DEBUG_H__
#include <rtd_log/rtd_module_log.h>

//#define OTP_DBG_EN
#ifdef OTP_DBG_EN
#define OTP_DBG(fmt, args...)           rtd_pr_otp_err(fmt, ##args)
#else
#define OTP_DBG(fmt, args...)
#endif
#define OTP_INFO(fmt, args...)       rtd_pr_otp_info(fmt, ##args)
#define OTP_WARNING(fmt, args...)       rtd_pr_otp_warn(fmt, ##args)

#endif
