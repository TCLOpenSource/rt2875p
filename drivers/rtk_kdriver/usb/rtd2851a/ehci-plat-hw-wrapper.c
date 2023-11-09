#include <linux/acpi.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/timer.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/usb/ehci_pdriver.h>
#include <linux/completion.h>
#include <asm/delay.h>
#include <linux/mutex.h>
#include <rbus/usb_top_reg.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_crt.h>
#include "ehci-plat.h"

#define USB_PHY_DELAY                   50
#define USB_MAC_DELAY                           100

static DEFINE_MUTEX(crt_mutex);
static bool usb2_crt_power_on_done = false;

bool is_usb2_on(void)
{
        bool is_mac_clk_on = false, is_mac_release = false, is_phy_release = false;
        unsigned int data;

        // clock: USB MAC
        data = rtd_inl(SYS_REG_SYS_CLKEN2_reg);
        is_mac_clk_on = SYS_REG_SYS_CLKEN2_get_clken_usb2_dwc(data) & SYS_REG_SYS_CLKEN2_get_clken_usb2_wrapper(data);

        // reset: USB MAC
        data = rtd_inl(SYS_REG_SYS_SRST2_reg);
        is_mac_release = SYS_REG_SYS_SRST2_get_rstn_usb2_dwc(data) & SYS_REG_SYS_SRST2_get_rstn_usb2_wrapper(data);

        // reset: USB PHY
        data = rtd_inl(SYS_REG_SYS_SRST3_reg);
        is_phy_release = (SYS_REG_SYS_SRST3_get_rstn_usb2_phy_p0(data) &
                          SYS_REG_SYS_SRST3_get_rstn_usb2_phy_p1(data) &
                          SYS_REG_SYS_SRST3_get_rstn_usb2_phy_p2(data));

        return (is_mac_clk_on && is_mac_release && is_phy_release);
}
EXPORT_SYMBOL(is_usb2_on);


void usb2_crt_on(void)
{
        mutex_lock(&crt_mutex);

        if (!usb2_crt_power_on_done || !is_usb2_on())
        {
                CRT_CLK_OnOff(USB, CLK_OFF, (void *)2);
                CRT_CLK_OnOff(USB, CLK_ON, (void *)2);
                usb2_crt_power_on_done = true;
        }

        // unmask interrupt if it's masked.
        if (!USB_TOP_USB_CTR0_get_usb_top_int_en(rtd_inl(USB_TOP_USB_CTR0_reg)))
                rtd_setbits(USB_TOP_USB_CTR0_reg, USB_TOP_USB_CTR0_usb_top_int_en_mask);

        mutex_unlock(&crt_mutex);
}
EXPORT_SYMBOL(usb2_crt_on);


void usb2_crt_off(void)
{
        mutex_lock(&crt_mutex);
        usb2_crt_power_on_done = false;
        if (is_usb2_on())
                CRT_CLK_OnOff(USB, CLK_OFF, (void *)2);
        mutex_unlock(&crt_mutex);
}
EXPORT_SYMBOL(usb2_crt_off);

static int keep_pll_alive(struct platform_device *pdev)
{
        struct usb_hcd *hcd = platform_get_drvdata(pdev);
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);

        rtd_setbits(priv->wrapper_regs.EHCI_USB2_TOP_USB_CTR0_REG, USB_TOP_USB_CTR0_pll_alive_enable_mask);

        return 0;
}

int ehci_usb_mac_init(struct platform_device *pdev)
{
        int ret = 0;

        // In past experience, we will encounter rbus timeout when access oHCI regs if we dont set this.
        ret = keep_pll_alive(pdev);
        if (ret)
                goto out;

out:
        return ret;
}



static int ehci_less_one_sram_read(struct platform_device *pdev, bool enable)
{
#ifndef CONFIG_RTK_8KCODEC
        struct usb_hcd *hcd = platform_get_drvdata(pdev);
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);
        if (enable)
                rtd_setbits(priv->wrapper_regs.EHCI_USB2_TOP_USB_CTR0_REG, USB_TOP_USB_CTR0_wrap_sram_rd_opt_mask);
        else
                rtd_clearbits(priv->wrapper_regs.EHCI_USB2_TOP_USB_CTR0_REG, USB_TOP_USB_CTR0_wrap_sram_rd_opt_mask);
        return 0;
#else  // [H5XBU-698] fix send data fail when buffer is non-word aligned
        return 0;
#endif
}

static int ehci_utmi_release(struct platform_device *pdev)
{
        struct usb_hcd *hcd = platform_get_drvdata(pdev);
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);
        unsigned int i = 0;
        unsigned long reset_utmi_regs[3] = {priv->wrapper_regs.EHCI_USB2_TOP_RESET_UTMI_REG,
                                            priv->wrapper_regs.EHCI_USB2_TOP_RESET_UTMI_2PORT_REG,
                                            priv->wrapper_regs.EHCI_USB2_TOP_RESET_UTMI_3PORT_REG
                                           };
        if(priv->usb2_port_cnt > 3)
                return -ENODEV;

        for(i  = 0; i < priv->usb2_port_cnt; i++)
                rtd_clearbits(reset_utmi_regs[i], USB_TOP_RESET_UTMI_reset_utmi_mask);
        udelay(USB_MAC_DELAY);
        return 0;
}



int ehci_usb_mac_post_init(struct platform_device *pdev, bool in_resume)
{
        int ret = 0;
        struct usb_hcd *hcd = platform_get_drvdata(pdev);
        struct ehci_hcd *ehci = hcd_to_ehci(hcd);
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);

        int i = 0;

        // Keep false to prevent data alignment issue that transfers size more than 258 bytes
        ret = ehci_less_one_sram_read(pdev, false);
        if (ret)
                goto out;

        ret = ehci_utmi_release(pdev);
        if (ret)
                goto out;

        ehci_writel(ehci, 0x00080B01, &ehci->regs->command);    //bit 0 EHCI MAC RUN / set USBCMD
        udelay(USB_MAC_DELAY);

        ehci_writel(ehci, FLAG_CF, &ehci->regs->configured_flag);    //release owner
        udelay(USB_MAC_DELAY);

        for(i  = 0; i < priv->usb2_port_cnt; i++)
                ehci_writel(ehci, PORT_POWER, &ehci->regs->port_status[i]); // enable EHCI port power


        return 0;
out:
        return ret;
}


/*********************************EHCI CRC Releated*****************************************************/

int get_usb2_wrap_crc_en(struct ehci_hcd *ehci)
{
        struct ehci_platform_priv *priv = ehci_to_ehci_priv(ehci);
        return USB_TOP_USB_CRC_CNT_get_rx_crc_cnt_en(rtd_inl(priv->wrapper_regs.EHCI_USB2_TOP_USB_CRC_CNT_REG));
}

int get_usb2_wrap_crc_rst(struct ehci_hcd *ehci)
{
        struct ehci_platform_priv *priv = ehci_to_ehci_priv(ehci);
        return USB_TOP_USB_CRC_CNT_get_rx_crc_cnt_rst(rtd_inl(priv->wrapper_regs.EHCI_USB2_TOP_USB_CRC_CNT_REG));
}


int enble_usb2_wrap_crc(struct ehci_hcd *ehci, bool on)
{
        struct ehci_platform_priv *priv = ehci_to_ehci_priv(ehci);
        if (on)
                rtd_setbits(priv->wrapper_regs.EHCI_USB2_TOP_USB_CRC_CNT_REG, USB_TOP_USB_CRC_CNT_rx_crc_cnt_en_mask);
        else
                rtd_clearbits(priv->wrapper_regs.EHCI_USB2_TOP_USB_CRC_CNT_REG, USB_TOP_USB_CRC_CNT_rx_crc_cnt_en_mask);
        return 0;
}


int reset_usb2_wrap_crc(struct ehci_hcd *ehci)
{
        struct ehci_platform_priv *priv = ehci_to_ehci_priv(ehci);
        rtd_setbits(priv->wrapper_regs.EHCI_USB2_TOP_USB_CRC_CNT_REG, USB_TOP_USB_CRC_CNT_rx_crc_cnt_rst_mask);
        udelay(10);
        rtd_clearbits(priv->wrapper_regs.EHCI_USB2_TOP_USB_CRC_CNT_REG, USB_TOP_USB_CRC_CNT_rx_crc_cnt_rst_mask);
        return 0;
}


int get_usb2_wrap_crc_err_cnt(struct ehci_hcd *ehci, int port1)
{
        struct ehci_platform_priv *priv = ehci_to_ehci_priv(ehci);
        int crc_cnt = -1;
        unsigned int data = 0;

        if(port1 < 1 || port1 > priv->usb2_port_cnt)
                return -1;

        if (!USB_TOP_USB_CRC_CNT_get_rx_crc_cnt_en(rtd_inl(priv->wrapper_regs.EHCI_USB2_TOP_USB_CRC_CNT_REG)))
                return -1;

        data = rtd_inl(priv->wrapper_regs.EHCI_USB2_TOP_USB_CRC_CNT_REG);
        crc_cnt = ((data >> (USB_TOP_USB_CRC_CNT_rx_crc_cnt_port0_shift + ((port1 - 1) * 10))) & 0x3FF);
        return crc_cnt;
}