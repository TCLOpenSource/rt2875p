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

        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_GSBUSCFG1_REG, rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_GSBUSCFG1_REG)|(0xf<<8) );

        // usb 3.0 p3 mode -> p2 mode
        for(i = 0; i < priv->usb3_port_cnt; i++)
                writel(0x010C0002, &global_regs->gusb3pipectl[i]); //P3 mode->p2 mode

        //axi init
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_GSBUSCFG0_REG, 0x0000000e);

        //suspend timer setting  intital setting
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_GCTL_REG, 0x34b81004);

        // bit3=1, port0-UTMI clcok=30MHz
        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_GUSB2PHYCFG0_REG,
                 rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_GUSB2PHYCFG0_REG) | (1 << 3));

        xhci_plat_info("--------- %s finish ------------ \n", __func__);
        return 0;
}

#ifdef CONFIG_RTK_KDRV_PCIE
static DEFINE_MUTEX(pcie_share_mutex);
struct usb_hcd *hcd_u3 = NULL;
int usb_chk_pcie_enable(void){
        u32 reg_info;

        reg_info = rtd_inl(SYS_REG_SYS_CLKSEL_reg);
        xhci_plat_info("SYS_REG_SYS_CLKSEL_reg = 0x%08x \n",reg_info);
        reg_info &= SYS_REG_SYS_CLKSEL_pcie_usb3_sel_mask;
        xhci_plat_info("SYS_REG_SYS_CLKSEL_reg[7] = 0x%x \n",reg_info);
        if(reg_info == 0){   // PCIe enabled
                xhci_plat_info("PCIe enabled\n");
                return 1;
        }else{
                xhci_plat_info("PCIe no be enabled\n");
                return 0;
        }
}

int usb_chk_pcie_share(struct usb_hcd *hcd)
{
        int ret = 0;
        u32 reginfo;
        struct xhci_plat_priv *priv = NULL;

        if(hcd == hcd_u3){
                xhci_plat_info("U3 hcd match (hcd: 0x%pR)\n",hcd);
                mutex_lock(&pcie_share_mutex);
                if(usb_chk_pcie_enable()){
                        priv = hcd_to_xhci_priv(hcd);
                        xhci_plat_info("disable U3 func for share PCIe\n");
                        reginfo = rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_HMAC_CTR0_REG);
                        rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_HMAC_CTR0_REG,reginfo | BIT(9)|BIT(8));
                        xhci_plat_info("0x%08x=0x%08x\n",
                                priv->wrapper_regs.XHCI_USB3_TOP_HMAC_CTR0_REG,
                                rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_HMAC_CTR0_REG));
                        reginfo = rtd_inl(0xb805811c);
                        rtd_outl(0xb805811c,reginfo|BIT(26));
                        xhci_plat_info("0x%08x=0x%08x\n",0xb805811c,rtd_inl(0xb805811c));
                        ret = 1;
                }
                mutex_unlock(&pcie_share_mutex);
        }
        return ret;
}

int export_usb_chk_pcie_share(void)
{
        int ret = 0;
        if(hcd_u3){
                xhci_plat_info("%s: U3 hcd is 0x%pR\n",__func__,hcd_u3);
                ret = usb_chk_pcie_share(hcd_u3);
        }else{
                xhci_plat_info("%s: no U3 hcd found\n",__func__);
        }
        return ret;
}
EXPORT_SYMBOL(export_usb_chk_pcie_share);
#endif

int xhci_usb3_phy_init_post(struct platform_device *pdev)
{
        struct usb_hcd *hcd = platform_get_drvdata(pdev);
        struct xhci_plat_priv *priv = hcd_to_xhci_priv(hcd);
        int i = 0;
        unsigned int ana_phy_setting = 0;
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);
#ifdef CONFIG_RTK_KDRV_PCIE
        if(hcd->rsrc_start == 0x18050000){
            //xhci_plat_info("U3 hcd is 0x%pR rsrc_start=0x%08x\n",hcd,hcd->rsrc_start);
            hcd_u3 = hcd;
        }
#endif
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

#ifdef CONFIG_RTK_KDRV_PCIE //disable U3 function
        usb_chk_pcie_share(hcd);
#endif

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