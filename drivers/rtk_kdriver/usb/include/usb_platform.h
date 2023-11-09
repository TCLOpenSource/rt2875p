#ifndef _USB_PLATFORM_H_
#define _USB_PLATFORM_H_
#include <linux/usb.h>


#ifdef CONFIG_CUSTOMER_TV030
extern int usb_set_MTK_wifi_off_on(bool on);
extern bool is_MTK_usb_wifi_bt_dev(struct usb_device *udev);
#else
#define usb_set_MTK_wifi_off_on(on) 0
#define is_MTK_usb_wifi_bt_dev(udev) 0
#endif


#endif
