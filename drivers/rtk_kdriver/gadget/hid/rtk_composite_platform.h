#ifndef __RTK_COMPOSITE_PLATFORM_H__
#define __RTK_COMPOSITE_PLATFORM_H__
#include <linux/mutex.h>
#include <rtd_log/rtd_module_log.h>

/*
+CONFIG_USB_G_HID=y
+CONFIG_USB_F_HID=y
+CONFIG_USB_GADGET=y
+CONFIG_DWC2_RTK=y
+CONFIG_RTK_KDRV_USB_GADGET=y
+CONFIG_RTK_KDRV_USB_GADGET_HID=y
+CONFIG_USB_DWC2=y
*/

#define RTK_COMPOSITE_DBG(fmt, args...)           
#define RTK_COMPOSITE_WARN(fmt, args...)       rtd_pr_usb_gadget_warn(fmt, ##args)
#define RTK_COMPOSITE_ERR(fmt, args...)       rtd_pr_usb_gadget_err(fmt, ##args)
#define RTK_COMPOSITE_INFO(fmt, args...)       rtd_pr_usb_gadget_info(fmt, ##args)

void RTK_USB_Typec_Hidg_enable(bool enable);
#endif
