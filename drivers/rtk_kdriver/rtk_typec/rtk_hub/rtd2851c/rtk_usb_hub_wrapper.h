#ifndef __RTK_USB_HUB_WRAPPER_H__
#define __RTK_USB_HUB_WRAPPER_H__

#define CONFIG_HUB_PORT_POLL_DATA  {  \
					{1, 1, 0x81, 0x06, false}, \
					{1, 2, 0x82, 0x06, false}, \
				}

void rtk_usb_hub_wrapper_clock_on(void);
void rtk_usb_hub_clk_on(void);
bool rtk_usb_hub_load_phy_settings(void);
void rtk_usb_hub_wrapper_clock_off(void);

#endif