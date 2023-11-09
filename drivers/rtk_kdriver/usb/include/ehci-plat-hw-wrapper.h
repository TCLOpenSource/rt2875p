#ifndef __EHCI_PLATFORM_HW_WRAPPER_H__
#define __EHCI_PLATFORM_HW_WRAPPER_H__
#include <rtk_kdriver/rtk_gpio.h>
#include "ehci-plat-config.h"

struct EHCI_WRAPPER_REGS
{
        unsigned int EHCI_USB2_TOP_WRAPP_REG;
        unsigned int EHCI_USB2_TOP_VSTATUS_REG;
        unsigned int EHCI_USB2_TOP_USBIPINPUT_REG;
        unsigned int EHCI_USB2_TOP_RESET_UTMI_REG;
        unsigned int EHCI_USB2_TOP_SELF_LOOP_BACK_REG;
        unsigned int EHCI_USB2_TOP_VERSION_REG;
        unsigned int EHCI_USB2_TOP_WRAPP_2PORT_REG;
        unsigned int EHCI_USB2_TOP_VSTATUS_2PORT_REG;
        unsigned int EHCI_USB2_TOP_USBIPINPUT_2PORT_REG;
        unsigned int EHCI_USB2_TOP_RESET_UTMI_2PORT_REG;
        unsigned int EHCI_USB2_TOP_SELF_LOOP_BACK_2PORT_REG;
        unsigned int EHCI_USB2_TOP_IPNEWINPUT_2PORT_REG;
        unsigned int EHCI_USB2_TOP_USBPHY_SLB0_REG;
        unsigned int EHCI_USB2_TOP_USBPHY_SLB1_REG;
        unsigned int EHCI_USB2_TOP_USB_DUMMY_REG;
        unsigned int EHCI_USB2_TOP_USB_CTR0_REG;
        unsigned int EHCI_USB2_TOP_USB_CTR1_REG;
        unsigned int EHCI_USB2_TOP_WRAPP_DBG1_REG;
        unsigned int EHCI_USB2_TOP_WRAPP_DBG2_REG;
        unsigned int EHCI_USB2_TOP_WRAPP_DBG3_REG;

        unsigned int EHCI_USB2_TOP_WRAPP_3PORT_REG;
        unsigned int EHCI_USB2_TOP_VSTATUS_3PORT_REG;
        unsigned int EHCI_USB2_TOP_USBIPINPUT_3PORT_REG;
        unsigned int EHCI_USB2_TOP_RESET_UTMI_3PORT_REG;
        unsigned int EHCI_USB2_TOP_SELF_LOOP_BACK_3PORT_REG;
        unsigned int EHCI_USB2_TOP_USBPHY_SLB2_REG;

        unsigned int EHCI_USB2_TOP_USB_CTR2_REG;
        unsigned int EHCI_USB2_TOP_SF_TEST_CTR_REG;
        unsigned int EHCI_USB2_TOP_PORT_STATUS_REG;
        unsigned int EHCI_USB2_TOP_USB_CRC_CNT_REG;
        unsigned int EHCI_USB2_TOP_USB_INT_REG;
        unsigned int EHCI_USB2_TOP_USB_DEBUG_REG;
        unsigned int EHCI_USB2_TOP_WRAP_CTR_REG;
};

bool is_usb2_on(void);
void usb2_crt_on(void);
void usb2_crt_off(void);
int ehci_usb_mac_init(struct platform_device *pdev);
int ehci_usb_mac_post_init(struct platform_device *pdev, bool in_resume);
#endif
