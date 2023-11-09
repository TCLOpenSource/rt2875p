#ifndef __RTK_USB_HUB_CAMERA_H__
#define __RTK_USB_HUB_CAMERA_H__
int rtk_usb_hub_register_notifier(void);
void rtk_usb_hub_unregister_notifier(void);
void rtk_usb_hub_check_port(void);
#endif
