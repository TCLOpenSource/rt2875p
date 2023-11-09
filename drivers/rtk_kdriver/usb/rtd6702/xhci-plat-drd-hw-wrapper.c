#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/regulator/consumer.h>
#include <rbus/usb3_top_reg.h> /* for U3 wrapper register define */
#include <rtk_kdriver/rtk_otp_region_api.h>
#include <rtk_kdriver/io.h>
#include <linux/delay.h>
#include "xhci-plat-drd.h"
#include "xhci-plat-drd-hw-wrapper.h"

void rtk_xhci_drd_mdio_write(
        struct rtk_xhci_drd *drd,
        unsigned char           port,
        unsigned char           phy_addr,
        unsigned char           reg_addr,
        unsigned short          val
);
unsigned short rtk_xhci_drd_mdio_read(
        struct rtk_xhci_drd *drd,
        unsigned char       port,
        unsigned char       phy_addr,
        unsigned char       reg_addr
);

int rtk_xhci_drd_mac_init(struct platform_device *pdev)
{
        struct rtk_xhci_drd*drd = platform_get_drvdata(pdev);
        struct xhci_drd_priv *priv = drd_to_xhci_drd_priv(drd);
        struct xhci_global_regs  *global_regs = priv->global_regs;
        unsigned port_mask = 0;
        unsigned int i = 0;

        xhci_drd_info("--------- %s start ------------ \n", __func__);

        port_mask = (1 << priv->usb2_port_cnt) - 1;
        // UTMI reset
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_USB2_PHY_UTMI_REG, port_mask);

        // UTMI release
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_USB2_PHY_UTMI_REG, 0x0);

        // PHY ignores suspend signal and always keep UTMI clock active
        rtd_setbits(priv->wrapper_regs.XHCI_USB3_TOP_USB_TMP_1_REG,
                    USB3_TOP_USB_TMP_1_test_reg_1(1) | USB3_TOP_USB_TMP_1_usb_ref_sel(0xd));

        // usb 3.0 p3 mode -> p2 mode
        for(i = 0; i < priv->usb3_port_cnt; i++)
                rtd_outl((unsigned long)(&global_regs->gusb3pipectl[i]), 0x010C0002); //P3 mode->p2 mode

        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_GSBUSCFG1_REG, rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_GSBUSCFG1_REG) | (0xf << 8) );

        //axi init
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_GSBUSCFG0_REG, 0x0000000e);

        //suspend timer setting  intital setting
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_GCTL_REG, 0x34b82004);

        //p0 pull down =1 for host
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_USB2_PHY0_REG, 0x00000007);

        //p1 pull down =1 for host
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_USB2_PHY1_REG, 0x00000007);

        // drd_ip_utmisrp_valid =1, drd_ip_pipe3_power_present =1
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_MAC3_HST_ST_REG, 0x0703F400);

        // bit3=1, port0-UTMI clcok=30MHz
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_GUSB2PHYCFG0_REG, 0x40102548);

        // bit3=1, port1-UTMI clcok=30MHz
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_GUSB2PHYCFG1_REG, 0x40102548);

        xhci_drd_info("--------- %s finish ------------ \n", __func__);
        return 0;
}


int rtk_xhci_drd_phy_init_post(struct platform_device *pdev)
{
        struct rtk_xhci_drd *drd = platform_get_drvdata(pdev);
        struct xhci_drd_priv *priv = drd_to_xhci_drd_priv(drd);
        int i = 0;
        unsigned int ana_phy_setting = 0;


        // Toggle. DC CALIBARION  "1 --> 0 --> 1". will do one time of calibration
        for(i = 0; i < priv->usb3_port_cnt; i++)
        {
                rtk_xhci_drd_mdio_write(drd, i, 0x0, 0x09, 0x924C);
                rtk_xhci_drd_mdio_write(drd, i, 0x0, 0x09, 0x904C);
                rtk_xhci_drd_mdio_write(drd, i, 0x0, 0x09, 0x924C);
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

int rtk_xhci_drd_mac_postinit(struct platform_device *pdev)
{
        struct rtk_xhci_drd *drd = platform_get_drvdata(pdev);
        struct xhci_drd_priv *priv = drd_to_xhci_drd_priv(drd);
        struct xhci_global_regs *global_regs = priv->global_regs;

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
                rtd_outl((unsigned long)(&global_regs->gusb3pipectl[i]), 0x010e0002); //P3 mode->p2 mode
                xhci_drd_info("current mode: %s \n",
                              rtd_inl((unsigned long)(&global_regs->gusb3pipectl[i])) & BIT(17) ? "p3" : "p2");
        }

        return 0;
}