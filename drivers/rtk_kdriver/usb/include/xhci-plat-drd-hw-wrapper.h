#ifndef __XHCI_PLATFORM_DRD_HW_WRAPPER_H__
#define __XHCI_PLATFORM_DRD_HW_WRAPPER_H__
#include "xhci-plat-config.h"
#define USB_PHY_DELAY    25
int rtk_xhci_drd_mac_init(struct platform_device *pdev);
int rtk_xhci_drd_phy_init_post(struct platform_device *pdev);
int rtk_xhci_drd_mac_postinit(struct platform_device *pdev);
#endif
