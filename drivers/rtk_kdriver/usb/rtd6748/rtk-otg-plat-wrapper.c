#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <rbus/sys_reg_reg.h>
#include <rtk_kdriver/io.h>
#include <rbus/usb2_otg_reg.h>
#include "rtk-otg-plat-wrapper.h"

#define USB_PHY_DELAY                         120
#ifdef CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM
extern struct platform_device *get_ehci_platform_device(void);
#endif

static bool g_last_otg_mode = false;

void rtk_otg_crt_on_off(bool on)
{
        if(on)
        {
                /*---------------------------------------------------
                    * shutdown sequence :
                    *  1. mac clock off
                    *  2. mac reset off
                    *  3. phy reset off
                    *---------------------------------------------------*/

                // clock off : MAC and OTG
                rtd_outl(SYS_REG_SYS_CLKEN5_reg,
                         SYS_REG_SYS_CLKEN5_clken_otg_mask |
                         SYS_REG_SYS_CLKEN5_write_data(0));
                udelay(USB_PHY_DELAY);

                // Reset MAC and OTG
                rtd_outl(SYS_REG_SYS_SRST5_reg,
                         //SYS_REG_SYS_SRST5_rstn_otg_mask |
                         SYS_REG_SYS_SRST5_rstn_usb_otg_wrapper_mask |
                         SYS_REG_SYS_SRST5_rstn_usb_otg_dwc_mask |
                         SYS_REG_SYS_SRST5_write_data(0));
                udelay(USB_PHY_DELAY);

                // Reset PHY
                /*rtd_outl(SYS_REG_SYS_SRST3_reg,
                    SYS_REG_SYS_SRST3_rstn_usb2_phy_p3_mask |
                    SYS_REG_SYS_SRST3_write_data(0));
                udelay(USB_PHY_DELAY);*/

                /*---------------------------------------------------
                    * init sequence :
                    *  1. phy reset release
                    *  2. mac clock on
                    *  3. mac clock off
                    *  4. mac reset on
                    *  5. mac clock on
                    *---------------------------------------------------*/
                // Release USB Phy reset : USB Phy Reset Ready
                /*rtd_outl(SYS_REG_SYS_SRST3_reg,
                        SYS_REG_SYS_SRST3_rstn_usb2_phy_p3_mask |
                        SYS_REG_SYS_SRST3_write_data(1));
                udelay(USB_PHY_DELAY);*/

                // clock on : MAC and OTG
                rtd_outl(SYS_REG_SYS_CLKEN5_reg,
                         SYS_REG_SYS_CLKEN5_clken_otg_mask |
                         SYS_REG_SYS_CLKEN5_write_data(1));
                udelay(USB_PHY_DELAY);

                // clock off : MAC and OTG
                rtd_outl(SYS_REG_SYS_CLKEN5_reg,
                         SYS_REG_SYS_CLKEN5_clken_otg_mask |
                         SYS_REG_SYS_CLKEN5_write_data(0));
                udelay(USB_PHY_DELAY);

                // Relase USB MAC Reset : USB MAC Reset Ready
                rtd_outl(SYS_REG_SYS_SRST5_reg,
                         SYS_REG_SYS_SRST5_rstn_otg_mask |
                         SYS_REG_SYS_SRST5_rstn_usb_otg_wrapper_mask |
                         SYS_REG_SYS_SRST5_rstn_usb_otg_dwc_mask |
                         SYS_REG_SYS_SRST5_write_data(1));
                udelay(USB_PHY_DELAY);

                // clock on : MAC and OTG
                rtd_outl(SYS_REG_SYS_CLKEN5_reg,
                         SYS_REG_SYS_CLKEN5_clken_otg_mask |
                         SYS_REG_SYS_CLKEN5_write_data(1));
                udelay(USB_PHY_DELAY);

        }
        else
        {
                // clock off : MAC and OTG
                rtd_outl(SYS_REG_SYS_CLKEN5_reg,
                         SYS_REG_SYS_CLKEN5_clken_otg_mask |
                         SYS_REG_SYS_CLKEN5_write_data(0));

                // Reset MAC and OTG
                /*rtd_outl(SYS_REG_SYS_SRST5_reg,
                         SYS_REG_SYS_SRST5_rstn_otg_mask |
                         SYS_REG_SYS_SRST5_rstn_usb_otg_wrapper_mask |
                         SYS_REG_SYS_SRST5_rstn_usb_otg_dwc_mask |
                         SYS_REG_SYS_SRST5_write_data(0));*/
        }

}
EXPORT_SYMBOL(rtk_otg_crt_on_off);

int rtk_otg_initial(struct platform_device *pdev)
{
        //wdone eco option = 1, ahb_eco_option2 =1, ahb_eco_option1 =1
        rtd_outl(USB2_OTG_USB_CTR0_reg,
                 (rtd_inl(USB2_OTG_USB_CTR0_reg) | USB2_OTG_USB_CTR0_ahb_eco_option(0x1) |
                  USB2_OTG_USB_CTR0_wdone_eco_option(0x1) |
                  USB2_OTG_USB_CTR0_ahb_eco_option2(0x1)));

        if(g_last_otg_mode)
               rtd_outl(USB2_OTG_USB_OTG_REG_reg,
                       rtd_inl(USB2_OTG_USB_OTG_REG_reg) | USB2_OTG_USB_OTG_REG_otg_enable(0x1));
        else
               rtd_outl(USB2_OTG_USB_OTG_REG_reg,
                       rtd_inl(USB2_OTG_USB_OTG_REG_reg) &  (~USB2_OTG_USB_OTG_REG_otg_enable(0x1)));

        // pull reset signal of U2 P0
        rtd_outl(USB2_OTG_RESET_UTMI_reg,
                 rtd_inl(USB2_OTG_RESET_UTMI_reg) | USB2_OTG_RESET_UTMI_reset_utmi_p0_mask);
        udelay(USB_PHY_DELAY);

        // release reset signal
        rtd_outl(USB2_OTG_RESET_UTMI_reg,
                 rtd_inl(USB2_OTG_RESET_UTMI_reg) & (~USB2_OTG_RESET_UTMI_reset_utmi_p0_mask));

        udelay(USB_PHY_DELAY);

        return 0;
}

void rtk_otg_plat_set_global_interrupts(struct platform_device *pdev, bool on)
{
        if(on)
        {
                rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_usb2_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_write_data_mask);
        }
        else
        {
                rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_usb2_int_scpu_routing_en_mask);

        }
}

void rtk_otg_plat_set_local_interrupts(struct platform_device *pdev, bool on)
{
        if(on)
        {
                rtd_outl(USB2_OTG_USB_CTR0_reg, rtd_inl(USB2_OTG_USB_CTR0_reg) | USB2_OTG_USB_CTR0_usb_top_int_en_mask);
        }
        else
        {
                rtd_outl(USB2_OTG_USB_CTR0_reg, rtd_inl(USB2_OTG_USB_CTR0_reg) & (~USB2_OTG_USB_CTR0_usb_top_int_en_mask));

        }
}

int rtk_otg_plat_pre_resume(struct platform_device *pdev)
{
#ifdef CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM
        struct platform_device *ehci_platform_dev = get_ehci_platform_device();

        if (ehci_platform_dev->dev.power.is_suspended)
        {
                device_pm_wait_for_dev(&pdev->dev, &ehci_platform_dev->dev);
        }
#endif
        return 0;
}

int rtk_otg_plat_set_otg_mode(struct platform_device *pdev, bool on_off)
{
	if(on_off)
		rtd_outl(USB2_OTG_USB_OTG_REG_reg,
			rtd_inl(USB2_OTG_USB_OTG_REG_reg) | USB2_OTG_USB_OTG_REG_otg_enable(0x1));
	else
		rtd_outl(USB2_OTG_USB_OTG_REG_reg,
			rtd_inl(USB2_OTG_USB_OTG_REG_reg) &  (~USB2_OTG_USB_OTG_REG_otg_enable(0x1)));
	g_last_otg_mode =  on_off;
	return 0;
}

int rtk_otg_plat_get_otg_mode(struct platform_device *pdev)
{
	return !!(rtd_inl(USB2_OTG_USB_OTG_REG_reg) & USB2_OTG_USB_OTG_REG_otg_enable(0x1));
}