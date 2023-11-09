#ifndef __XHCI_PLATFORM_HW_WRAPPER_H__
#define __XHCI_PLATFORM_HW_WRAPPER_H__
#include "xhci.h"
#include "xhci-plat-config.h"
int xhci_usb_mac_init(struct platform_device *pdev);
int xhci_usb_mac_postinit(struct platform_device *pdev);
bool is_usb3_on(void);
void usb3_crt_on(void);
void usb3_crt_off(void);

int xhci_usb3_phy_init_post(struct platform_device *pdev);
#endif
