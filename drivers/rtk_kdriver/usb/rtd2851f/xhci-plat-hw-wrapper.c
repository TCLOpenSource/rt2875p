#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/usb/phy.h>
#include <linux/slab.h>
#include <linux/acpi.h>
#include <linux/version.h>
#include <asm/delay.h>
#include <linux/mutex.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_crt.h>
#include <rbus/usb3_top_reg.h>
#include "xhci.h"
#include "xhci-plat.h"

#define USB_PHY_DELAY 50

static DEFINE_MUTEX(crt_mutex);


bool is_usb3_on(void)
{
        bool is_mac_clk_on = false, is_mac_release = false, is_phy_release = false;
        unsigned clk = 0, mac_rst = 0, phy_rst = 0;

        // clock: USB MAC
        clk = rtd_inl(SYS_REG_SYS_CLKEN2_reg);
        if (clk & SYS_REG_SYS_CLKEN2_clken_usb3_mask)
                is_mac_clk_on = true;


        // reset: USB MAC
        mac_rst = rtd_inl(SYS_REG_SYS_SRST2_reg);
        if ((mac_rst & SYS_REG_SYS_SRST2_rstn_usb3_mask))
                is_mac_release = true;

        // reset: USB phy and others
        phy_rst = rtd_inl(SYS_REG_SYS_SRST3_reg);
        if ((phy_rst & SYS_REG_SYS_SRST3_rstn_usb2_phy_p0_mask) &&
            (phy_rst & SYS_REG_SYS_SRST3_rstn_usb3_phy_p0_mask) &&
            (phy_rst & SYS_REG_SYS_SRST3_rstn_usb3_mdio_p0_mask))
                is_phy_release = true;

        return (is_mac_clk_on && is_mac_release && is_phy_release);
}
EXPORT_SYMBOL(is_usb3_on);


void usb3_crt_on(void)
{
        mutex_lock(&crt_mutex);
        CRT_CLK_OnOff(USB, CLK_ON, (void *)3);
        mutex_unlock(&crt_mutex);
}
EXPORT_SYMBOL(usb3_crt_on);


void usb3_crt_off(void)
{
        mutex_lock(&crt_mutex);
        CRT_CLK_OnOff(USB, CLK_OFF, (void *)3);
        mutex_unlock(&crt_mutex);
}
EXPORT_SYMBOL(usb3_crt_off);

/************************************************************/
int xhci_usb_mac_init(struct platform_device *pdev)
{
        struct usb_hcd *hcd = platform_get_drvdata(pdev);
        struct xhci_global_regs __iomem *global_regs = hcd_to_xhci_priv(hcd)->global_regs;
        struct xhci_plat_priv *priv = hcd_to_xhci_priv(hcd);
        unsigned port_mask = 0;
        unsigned int i = 0;

        xhci_plat_info("--------- %s start ------------ \n", __func__);

        port_mask = (1 << priv->usb2_port_cnt) - 1;
        // UTMI reset
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_USB2_PHY_UTMI_REG, port_mask);

        // UTMI release
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_USB2_PHY_UTMI_REG, 0x0);

        // PHY ignores suspend signal and always keep UTMI clock active
        rtd_setbits(priv->wrapper_regs.XHCI_USB3_TOP_USB_TMP_1_REG, USB3_TOP_USB_TMP_1_test_reg_1(1)
                    | USB3_TOP_USB_TMP_1_usb_ref_sel(0xd));
        // Wdone make sure that RW-inorder
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_WRAP_CTR_REG, rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_WRAP_CTR_REG) | USB3_TOP_WRAP_CTR_dbus_wait_wdone(1));

        rtd_outl(0xb8058104, rtd_inl(0xb8058104)|(0xf<<8) );

        // usb 3.0 p3 mode -> p2 mode
        for(i = 0; i < priv->usb3_port_cnt; i++)
                writel(0x010C0002, &global_regs->gusb3pipectl[i]); //P3 mode->p2 mode

        //axi init
        rtd_outl(0xb8058100, 0x0000000e);

        //suspend timer setting  intital setting
        rtd_outl(0xb8058110, 0x34b81004);

        // bit3=1, port0-UTMI clcok=30MHz
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_GUSB2PHYCFG0_REG,
                 rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_GUSB2PHYCFG0_REG) | (1 << 3));

        xhci_plat_info("--------- %s finish ------------ \n", __func__);
        return 0;
}

int xhci_usb3_phy_init_post(struct platform_device *pdev)
{
        struct usb_hcd *hcd = platform_get_drvdata(pdev);
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);
        struct xhci_plat_priv *priv = hcd_to_xhci_priv(hcd);
        int i = 0;
        unsigned int ana_phy_setting = 0;


        // Toggle. DC CALIBARION  "1 --> 0 --> 1". will do one time of calibration
        for(i = 0; i < priv->usb3_port_cnt; i++)
        {
                xhci_usb3_mdio_write(xhci, i, 0x0, 0x09, 0x924C);
                xhci_usb3_mdio_write(xhci, i, 0x0, 0x09, 0x904C);
                xhci_usb3_mdio_write(xhci, i, 0x0, 0x09, 0x924C);
        }

        // USB3_ANA_PHY open
        ana_phy_setting = 0;

        for(i  = 0; i < priv->usb3_port_cnt; i++)
        {
                unsigned int tmp = USB3_TOP_ANA_PHY0_usb3_mbias_en_p0(1) |
                                   USB3_TOP_ANA_PHY0_usb3_bg_en_p0(1) |
                                   USB3_TOP_ANA_PHY0_usb3_pow_gating_p0(1);

                ana_phy_setting = ana_phy_setting | (tmp << (i * 8));
        }
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_ANA_PHY0_REG, ana_phy_setting);
        udelay(400);

        //u3Rxterm_force_val=0,//Rxterm_force_en=0
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_USB_TMP_2_REG,
                 USB3_TOP_USB_TMP_2_usb3_eco_option1(1));
        return 0;
}

int xhci_usb_mac_postinit(struct platform_device *pdev)
{
        struct usb_hcd *hcd = platform_get_drvdata(pdev);
        struct xhci_global_regs __iomem *global_regs = hcd_to_xhci_priv(hcd)->global_regs;
        struct xhci_plat_priv *priv = hcd_to_xhci_priv(hcd);

        ////////////let usb 3.0 R Term turn on////////////
        //Rxterm_force_en=1
        unsigned int i = 0;
        unsigned int set_value = 0;

        for(i  = 0; i < priv->usb3_port_cnt; i++)
                set_value |= (1 << (USB3_TOP_USB_TMP_2_usb3_rxterm_force_en_p0_shift + (i * 2)));
        set_value |=  USB3_TOP_USB_TMP_2_usb3_eco_option1(1);

        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_USB_TMP_2_REG, set_value);

        for(i  = 0; i < priv->usb3_port_cnt; i++)
        {
                writel(0x010e0002, &global_regs->gusb3pipectl[i]); //P3 mode->p2 mode
                xhci_plat_info("current mode: %s \n",
                               readl(&global_regs->gusb3pipectl[i]) & BIT(17) ? "p3" : "p2");
        }

        return 0;
}