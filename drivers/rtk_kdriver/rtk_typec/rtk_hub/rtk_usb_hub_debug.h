#ifndef __RTK_USB_HUB_DEBUG_H__
#define __RTK_USB_HUB_DEBUG_H__
#include <rtd_log/rtd_module_log.h>
#ifdef RTK_USB_HUB_DBG_EN
#define RTK_USB_HUB_DBG(fmt, args...)           rtd_pr_usb_hub_err("[DBG] " fmt, ##args)
#else
#define RTK_USB_HUB_DBG(fmt, args...)

#endif
#define RTK_USB_HUB_INFO(fmt, args...)       rtd_pr_usb_hub_info("[Info]" fmt, ##args)
#define RTK_USB_HUB_WARNING(fmt, args...)       rtd_pr_usb_hub_warn("[Warn]" fmt, ##args)
#define RTK_USB_HUB_ERR(fmt, args...)       rtd_pr_usb_hub_err("[Err]" fmt, ##args)

#endif
