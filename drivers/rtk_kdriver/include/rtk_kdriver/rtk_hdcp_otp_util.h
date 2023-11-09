#ifndef __RTK_HDCP_OTP_UTIL_H__
#define __RTK_HDCP_OTP_UTIL_H__
#include <rtd_log/rtd_module_log.h>
/* --------------------------------- Debug --------------------------------- */
//#define TAG_NAME "HDCP"
//#define HDCP_OTP_DEBUG
#ifdef HDCP_OTP_DEBUG
#define hdcp_otp_debug(fmt, args...)\
		rtd_pr_otp_debug(fmt, ## args)
#define hdcp_otp_info(fmt, args...)\
		rtd_pr_otp_info(fmt, ## args)
#else
#define hdcp_otp_debug(fmt, args...)
#define hdcp_otp_info(fmt, args...)
#endif

#define hdcp_otp_warning(fmt, args...)\
		rtd_pr_otp_warn(fmt, ## args)
#define hdcp_otp_err(fmt, args...)\
		rtd_pr_otp_err(fmt, ## args)

void hdcp_otp_hexdump(const void *buf, unsigned int length);
int get_hdcp_22_otp(unsigned char *buf,unsigned int len);
int get_hdcp_14_otp(unsigned char *buf,unsigned int len);

#endif //__RTK_HDCP_OTP_UTIL_H__
