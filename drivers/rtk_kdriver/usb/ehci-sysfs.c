#include <linux/clk.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/usb/phy.h>
#include <linux/slab.h>
#include <linux/acpi.h>
#include <linux/usb/hcd.h>
#include <linux/usb/ehci_pdriver.h>
#include <rtk_kdriver/io.h>
#include "ehci.h"
#include "ehci-plat.h"

#define USB_PHY_DELAY 100

//===========================================================================
// control and debug interface
//===========================================================================
static ssize_t show_u2_phy_config(struct device *dev, struct device_attribute *attr, char *buf)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct ehci_hcd *ehci = hcd_to_ehci(hcd);
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);
        int  n, ret, i = 0, count = PAGE_SIZE;
        char *ptr = buf;
        unsigned char phy_id;

        if (sscanf(attr->attr.name, "u2_phy%hhu_config", &phy_id) != 1 || phy_id > priv->usb2_port_cnt)
        {
                ehci_plat_warn("show_u2_phy_config failed, get phy id failed\n");
                return 0;
        }

        for (i = 0; u2_phy_reg_table[i].addr; i++)
        {
                ret = get_ehci_usb2_phy_reg(ehci, phy_id, u2_phy_reg_table[i].page, u2_phy_reg_table[i].addr);

                if (ret == -ENODEV)
                        return ret;
                else if (ret < 0)
                        continue;

                n = scnprintf(ptr, count, "%X-%02X = 0x%02X | ",
                              u2_phy_reg_table[i].page,
                              u2_phy_reg_table[i].addr,
                              ret);
                ptr += n;
                count -= n;

                switch (u2_phy_reg_table[i].page)
                {
                        case 0:
                                switch (u2_phy_reg_table[i].addr)
                                {
                                        case 0xE0:
                                                n = scnprintf(ptr, count, "z0_adjust[5:2]=%x",
                                                              (ret >> 2) & 0xF);
                                                ptr += n;
                                                count -= n;
                                                break;

                                        case 0xE4:
                                                n = scnprintf(ptr, count, "disconnect_level[7:4]=%x, tx_swing[3:0]=%x",
                                                              (ret >> 4) & 0xF, ret & 0xF);
                                                ptr += n;
                                                count -= n;
                                                break;

                                        case 0xE6:
                                                n = scnprintf(ptr, count, "LF_mode_tx_slew_rate[2]=%x, rx_gain_boost[1:0]=%x",
                                                              (ret >> 2) & 0x1, ret & 0x3);
                                                ptr += n;
                                                count -= n;
                                                break;

                                        case 0xE7:
                                                n = scnprintf(ptr, count, "squalsh_level[7:4]=%x",
                                                              (ret >> 4) & 0xF);
                                                ptr += n;
                                                count -= n;
                                                break;
                                        case 0xF0:
                                                n = scnprintf(ptr, count, "discon_enable[7]=%x",
                                                              (ret >> 7) & 0x1);
                                                ptr += n;
                                                count -= n;
                                                break;

                                        case 0xF1:
                                                n = scnprintf(ptr, count, "tx_delay[2:1]=%x",
                                                              (ret >> 1) & 0x3);
                                                ptr += n;
                                                count -= n;
                                                break;
                                }
                                break;

                        case 1:
                                if (u2_phy_reg_table[i].addr == 0xE1)
                                {
                                        n = scnprintf(ptr, count, "slew_rate[6:4]=%x",
                                                      (ret >> 4) & 7);
                                        ptr += n;
                                        count -= n;
                                }
                                break;
                }

                n = scnprintf(ptr, count, "\n");
                ptr += n;
                count -= n;
        }

        return ptr - buf;
}

static ssize_t store_u2_phy_config(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct ehci_hcd *ehci = hcd_to_ehci(hcd);
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);
        unsigned char phy_id;
        unsigned int page = 0, addr = 0, val = 0;

        if (sscanf(attr->attr.name, "u2_phy%hhu_config", &phy_id) != 1 || phy_id > priv->usb2_port_cnt)
        {
                ehci_plat_warn("store_u2_phy_config failed, get phy id failed\n");
                goto end_proc;
        }

        if (sscanf(buf, "%x-%x=%x", &page, &addr, &val) == 3)
        {
                if ((addr >= 0xE0 && addr <= 0xE7 && page < 2) ||
                    (addr >= 0xF0 && addr <= 0xF7 && page == 0))
                {
                        set_ehci_usb2_phy_reg(ehci, phy_id, page, addr, val);
                }
                else
                {
                        ehci_plat_warn("store_u2_phy_config failed, invalid register address\n");
                        goto end_proc;
                }
        }
        else
        {
                ehci_plat_warn("store_u2_phy_config failed, unknown command - %s\n", buf);
        }

end_proc:
        return count;
}
DEVICE_ATTR(u2_phy1_config, S_IRUGO | S_IWUSR, show_u2_phy_config, store_u2_phy_config);
DEVICE_ATTR(u2_phy2_config, S_IRUGO | S_IWUSR, show_u2_phy_config, store_u2_phy_config);
DEVICE_ATTR(u2_phy3_config, S_IRUGO | S_IWUSR, show_u2_phy_config, store_u2_phy_config);



//===========================================================================
// Func : show_portsc
//===========================================================================
static ssize_t show_portsc(struct device *dev, struct device_attribute *attr, char *buf)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct ehci_hcd *ehci = hcd_to_ehci(hcd);

        return _get_ehci_portsc_str(ehci, buf, PAGE_SIZE);
}
static DEVICE_ATTR(portsc, S_IRUGO,
                   show_portsc, NULL);


//===========================================================================
// Func : show_ehci_debug_control
//===========================================================================
static ssize_t show_debug_control(struct device *dev, struct device_attribute *attr, char *buf)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct ehci_hcd *ehci = hcd_to_ehci(hcd);
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);
        int n = 0, count = PAGE_SIZE;
        char *ptr = buf;

        n = scnprintf(ptr, count, "no_polling_check=%x\n", (priv->debug_ctrl_flag & NO_EHCI_POLLING_CHECK));
        ptr += n;
        if ((count -= n) < 0)
                goto overflow;

        n = scnprintf(ptr, count, "crc_en=%x\n", get_usb2_wrap_crc_en(ehci));
        ptr += n;
        if ((count -= n) < 0)
                goto overflow;

        n = scnprintf(ptr, count, "crc_rst=%x\n", get_usb2_wrap_crc_rst(ehci));
        ptr += n;
        if ((count -= n) < 0)
                goto overflow;

        return ptr - buf;

overflow:
        return -EOVERFLOW;
}


static ssize_t store_debug_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct ehci_hcd *ehci = hcd_to_ehci(hcd);
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);
        int val = 0, ret = 0;

        if (sscanf(buf, "no_polling_check=%x", &val) == 1)
        {
                if (val)
                {
                        if (!(priv->debug_ctrl_flag & NO_EHCI_POLLING_CHECK))
                        {
                                priv->debug_ctrl_flag |= NO_EHCI_POLLING_CHECK;
                                stop_ehci_polling_check_timer(ehci);
                        }
                }
                else
                {
                        if (priv->debug_ctrl_flag & NO_EHCI_POLLING_CHECK)
                        {
                                priv->debug_ctrl_flag &= ~NO_EHCI_POLLING_CHECK;
                                start_ehci_polling_check_timer(ehci);
                        }
                }
        }
        else if (sscanf(buf, "crc_en=%x", &val) == 1)
        {
                ret = enble_usb2_wrap_crc(ehci, !!val);

        }
        else if (sscanf(buf, "crc_rst=%x", &val) == 1)
        {
                if (!!val)
                        ret = reset_usb2_wrap_crc(ehci);
        }
        else
                ehci_plat_warn("store_ehci_debug_control failed, unknown command - %s\n", buf);

        return ret ? ret : count;
}

static DEVICE_ATTR(debug_control, S_IRUGO | S_IWUSR,
                   show_debug_control, store_debug_control);

const struct attribute *ehci_platform_dev_attrs[] =
{
        &dev_attr_u2_phy1_config.attr,
        &dev_attr_u2_phy2_config.attr,
        &dev_attr_u2_phy3_config.attr,
        &dev_attr_portsc.attr,
        &dev_attr_debug_control.attr,
        NULL,
};
