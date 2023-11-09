#include <linux/clk.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/usb/phy.h>
#include <linux/slab.h>
#include <linux/acpi.h>
#include <rbus/usb3_top_reg.h> /* for U3 wrapper register define */
#include <rtk_kdriver/io.h>
#include "xhci.h"
#include "xhci-plat.h"

#define USB_PHY_DELAY 100

typedef struct
{
        unsigned char page;
        unsigned char addr;
} U2_PHY_REG;

static U2_PHY_REG u2_phy_reg_table[] __attribute__((unused)) =
{
        {0, 0xE0},
        {0, 0xE1},
        {0, 0xE2},
        {0, 0xE3},
        {0, 0xE4},
        {0, 0xE5},
        {0, 0xE6},
        {0, 0xE7},
        {1, 0xE0},
        {1, 0xE1},
        {1, 0xE2},
        {1, 0xE3},
        {1, 0xE4},
        {1, 0xE5},
        {1, 0xE6},
        {1, 0xE7},
        {0, 0xF0},
        {0, 0xF1},
        {0, 0xF2},
        {0, 0xF3},
        {0, 0xF4},
        {0, 0xF5},
        {0, 0xF6},
        {0, 0xF7},
};


//===========================================================================
// For : xhci control and debug interface
//===========================================================================
static ssize_t show_u2_phy_config(
        struct device*              dev,
        struct device_attribute*    attr,
        char*                       buf
)
{
        int  n, ret, i = 0, count = PAGE_SIZE;
        char *ptr = buf;
        unsigned char phy_id;
        struct usb_hcd *hcd = dev_get_drvdata(dev);

        if (sscanf(attr->attr.name, "u2_phy%hhu_config", &phy_id) != 1)
        {
                xhci_plat_warn("%s failed, get phy id failed \n", __func__);
                return 0;
        }


        for (i = 0; i < sizeof(u2_phy_reg_table)/sizeof(U2_PHY_REG); i++)
        {
                ret = get_xhci_usb2_phy_reg(hcd_to_xhci(hcd), phy_id, u2_phy_reg_table[i].page, u2_phy_reg_table[i].addr);

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

static ssize_t store_u2_phy_config(
        struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count
)
{
        unsigned char phy_id;
        unsigned int page, addr, val;
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        int ret = 0;

        if (sscanf(attr->attr.name, "u2_phy%hhu_config", &phy_id) != 1)
        {
                xhci_plat_warn("%s failed, get phy id failed \n", __func__);
                goto end_proc;
        }

        if (sscanf(buf, "%x-%x=%x", &page, &addr, &val) == 3)
        {
                if ((addr >= 0xE0 && addr <= 0xE7 && page < 2) ||
                    (addr >= 0xF0 && addr <= 0xF7 && page == 0))
                {
                        ret = set_xhci_usb2_phy_reg(hcd_to_xhci(hcd), phy_id, page, addr, val);
                }
                else
                {
                        xhci_plat_warn("%s failed, invalid register address. page:0x%x, addr:0x%x \n", __func__, page, addr);
                        goto end_proc;
                }
        }
        else
        {
                xhci_plat_warn("%s failed, unknown command - %s. \n", __func__, buf);
        }

end_proc:
        return ret ? ret : count;
}
static DEVICE_ATTR(u2_phy1_config, 0644, show_u2_phy_config, store_u2_phy_config);
static DEVICE_ATTR(u2_phy2_config, 0644, show_u2_phy_config, store_u2_phy_config);


static ssize_t show_u3_phy_config(
        struct device*              dev,
        struct device_attribute*    attr,
        char*                       buf
)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);
        int  n;
        int  count = PAGE_SIZE;
        char *ptr = buf;
        int  i;

        for (i = 0; i <= 0x30; i++)
        {
                unsigned short val = xhci_usb3_mdio_read(xhci, 0, i>>5, i&0x1F);
                n = scnprintf(ptr, count, "%d-%02X = 0x%04X | ",
                              i>>5, i&0x1F, val);
                ptr   += n;
                count -= n;

                switch (i)
                {
                        case 0x1:
                                n = scnprintf(ptr, count, "rx_ki[8:6]=%x, rx_kp[5:1]=%x\n",
                                              (val>>6)&0x7, (val>>1)&0xF);
                                break;

                        case 0x4:
                                n = scnprintf(ptr, count, "ssc_en[13]=%x\n",
                                              (val>>13)&0x1);
                                break;

                        case 0xa:
                                n = scnprintf(ptr, count, "rx_eq_gain[8:7]=%x\n",
                                              (val>>7)&0x3);
                                break;

                        case 0x19:
                                n = scnprintf(ptr, count, "rx_eq[15:11]=%x\n",
                                              (val>>11)&0x1F);
                                break;

                        case 0x20:
                                n = scnprintf(ptr, count, "tx_amp[3:0]=%x\n",
                                              (val>>0)&0xF);
                                break;

                        case 0x21:
                                n = scnprintf(ptr, count, "tx_deemp[15:12]=%x, tx_deemp_1[7:4]=%x\n",
                                              (val>>12)&0xF, (val>>4)&0xF);
                                break;

                        case 0x22:
                                n = scnprintf(ptr, count, "tx_deemp_en[15:6]=%x\n",
                                              (val>>6)&0x3FF);
                                break;

                        case 0x26:
                                n = scnprintf(ptr, count, "rx_eq_dc_gain[1:0]=%x\n",
                                              val & 0x3);
                                break;

                        default:
                                n = scnprintf(ptr, count, "\n");
                                break;
                }
                ptr   += n;
                count -= n;
        }

        return ptr - buf;
}

static ssize_t store_u3_phy_config(
        struct device *dev,
        struct device_attribute *attr,
        const char *buf,
        size_t count)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);
        unsigned int page;
        unsigned int addr;
        unsigned int val;

        //xhci_drd_dbg("store_xhci_config : buff=%s, count=%d, attr.name=%s, (busnum=%d)\n", buf, count, attr->attr.name, xhci_to_hcd(xhci)->self.busnum);

        if (sscanf(buf, "%x-%x=%x\n", &page, &addr, &val) == 3)
        {
                //xhci_drd_dbg("page = %d, addr=%02x, val=%02x\n", page, addr, val);
                xhci_usb3_mdio_write(xhci, 0, page, addr, val);
        }
        else
        {
                xhci_plat_warn("%s failed, unknown command - %s\n", __func__, buf);
        }

        return count;
}

static DEVICE_ATTR(u3_phy_config, 0644, show_u3_phy_config, store_u3_phy_config);




//===========================================================================
// Func : show_xhci_debug_control
//===========================================================================
static ssize_t show_xhci_debug_control(
        struct device*              dev,
        struct device_attribute*    attr,
        char*                       buf
)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);
        struct xhci_plat_priv *priv = hcd_to_xhci_priv(hcd);
        int  n = 0, count = PAGE_SIZE;
        char *ptr = buf;

        n = scnprintf(ptr, count, "no_hub_polling_check=%x\n", (priv->debug_ctrl_flag & NO_HUB_POLLING_CHECK));
        ptr += n;
        count -= n;

        n = scnprintf(ptr, count, "no_u3_port_auto_recover=%x\n", (priv->debug_ctrl_flag & NO_U3_PORT_AUTO_RECOVER) ? 1 : 0);
        ptr   += n;
        count -= n;

        n = scnprintf(ptr, count, "no_restore_u3_phy=%x\n", (priv->debug_ctrl_flag & NO_RESTORE_U3_PHY) ? 1 : 0);
        ptr   += n;
        count -= n;

        n = scnprintf(ptr, count, "no_restore_u2_phy=%x\n", (priv->debug_ctrl_flag & NO_RESTORE_U2_PHY) ? 1 : 0);
        ptr   += n;
        count -= n;

        n = scnprintf(ptr, count, "no_u3_super_speed_port=%x\n", (priv->debug_ctrl_flag & NO_U3_SUPER_SPEED_PORT) ? 1 : 0);
        ptr   += n;
        count -= n;

#if 0
        n = scnprintf(ptr, count, "u3_port_vbus_pin=%s_%d\n",
                      gpio_type(gpio_group(USB3_VBUS_CTL)),
                      gpio_idx(USB3_VBUS_CTL));
        ptr   += n;
        count -= n;

        n = scnprintf(ptr, count, "u3_port_vbus=%x\n", rtk_gpio_input(USB3_VBUS_CTL));
        ptr   += n;
        count -= n;
#endif

        n = scnprintf(ptr, count, "compliance_mode=%x\n", (readl(xhci->usb3_rhub.ports[0]->addr) & PORT_PLS_MASK) == (0xa << 5));
        ptr   += n;
        count -= n;

        n = scnprintf(ptr, count, "crc_src=%x (%s). {U3port0=1, U2port0=4, U2Port1=5, U2port2=6}\n",
                      USB3_TOP_CRC_CNT_1_get_crc_cnt_sel(rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_CRC_CNT_1_REG)),
                      xhci_crc_error_src(USB3_TOP_CRC_CNT_1_get_crc_cnt_sel(rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_CRC_CNT_1_REG))));
        ptr   += n;
        count -= n;

        return ptr - buf;
}


static ssize_t store_xhci_debug_control(struct device *dev,
                                        struct device_attribute *attr,
                                        const char *buf, size_t count)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);
        struct xhci_plat_priv *priv = hcd_to_xhci_priv(hcd);
        int val;
        unsigned int tmp;

        if (sscanf(buf, "no_hub_polling_check=%x", &val))
        {
                if (val)
                {
                        if (!(priv->debug_ctrl_flag & NO_HUB_POLLING_CHECK))
                        {
                                priv->debug_ctrl_flag |= NO_HUB_POLLING_CHECK;
                                del_timer_sync(&priv->polling_check_timer);
                        }
                }
                else
                {
                        if (priv->debug_ctrl_flag & NO_HUB_POLLING_CHECK)
                        {
                                priv->debug_ctrl_flag &= ~NO_HUB_POLLING_CHECK;
                                mod_timer(&priv->polling_check_timer, jiffies + msecs_to_jiffies(USB_HUB_CHECK_TIME));
                        }
                }
        }

        if (sscanf(buf, "no_restore_u3_phy=%x\n", &val) == 1)
        {
                if (val)
                        priv->debug_ctrl_flag |= NO_RESTORE_U3_PHY;
                else
                        priv->debug_ctrl_flag &= ~NO_RESTORE_U3_PHY;
        }
        else if (sscanf(buf, "no_u3_port_auto_recover=%x\n", &val) == 1)
        {
                if (val)
                        priv->debug_ctrl_flag |= NO_U3_PORT_AUTO_RECOVER;
                else
                        priv->debug_ctrl_flag &= ~NO_U3_PORT_AUTO_RECOVER;
        }
        else if (sscanf(buf, "no_restore_u2_phy=%x\n", &val) == 1)
        {
                if (val)
                        priv->debug_ctrl_flag |= NO_RESTORE_U2_PHY;
                else
                        priv->debug_ctrl_flag &= ~NO_RESTORE_U2_PHY;
        }
        else if (sscanf(buf, "no_u3_super_speed_port=%x\n", &val) == 1)
        {
                if (val)
                {
                        priv->debug_ctrl_flag |= NO_U3_SUPER_SPEED_PORT;
                        tmp = readl(xhci->usb3_rhub.ports[0]->addr);
                        writel(tmp & ~PORT_POWER, xhci->usb3_rhub.ports[0]->addr); // disable U3 port power
                }
                else
                {
                        priv->debug_ctrl_flag &= ~NO_U3_SUPER_SPEED_PORT;
                        tmp = readl(xhci->usb3_rhub.ports[0]->addr);
                        writel(tmp | PORT_POWER, xhci->usb3_rhub.ports[0]->addr); // enable U3 port power
                }

        }
        else if (sscanf(buf, "u3_port_vbus=%x\n", &val) == 1)
        {
#if 0
                if (is_platform_KXLP())
                        xhci_plat_warn("Cannot control U3 Vbus. This is [K3Lp] and there is no U3 port\n");
                else
                        rtk_gpio_output(USB3_VBUS_CTL, (val) ? 1 : 0);
#endif
        }
        else if (sscanf(buf, "compliance_mode=%x\n", &val) == 1)
        {
                if (val)
                {
                        tmp = readl(xhci->usb3_rhub.ports[0]->addr);
                        writel((tmp & ~PORT_PLS_MASK) | (0xa << 5), xhci->usb3_rhub.ports[0]->addr);
                        udelay(USB_PHY_DELAY);
                        xhci_usb3_mdio_write(xhci, 0, 0x0, 0x1d, 0xa03e);  /* for BER off */
                        xhci_plat_warn("Please insert SMA Cable.....\n");
                }
                else
                {
                        xhci_plat_warn("Need to restart xhci to exit compliance mode\n");
                }
        }
        else if (sscanf(buf, "crc_src=%x\n", &val) == 1)
        {
#if 1
                switch (val)
                {
                        case CRC_SRC_NONE:
                                rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_CRC_CNT_1_REG, 0);
                                break;
                        case CRC_SRC_U3_PORT:
                        case CRC_SRC_U2_PORT0:
                        case CRC_SRC_U2_PORT1:
                        case CRC_SRC_U2_PORT2:
                                rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_CRC_CNT_1_REG, USB3_TOP_CRC_CNT_1_crc_cnt_sel(val) |
                                         USB3_TOP_CRC_CNT_1_crc_cnt_rst(1) |
                                         USB3_TOP_CRC_CNT_1_crc_cnt_en(1));

                                rtd_outl(priv->wrapper_regs.XHCI_USB3_TOP_CRC_CNT_1_REG, USB3_TOP_CRC_CNT_1_crc_cnt_sel(val) |
                                         USB3_TOP_CRC_CNT_1_crc_cnt_rst(0) |
                                         USB3_TOP_CRC_CNT_1_crc_cnt_en(1));
                                xhci_plat_info("crc_src=%d (%s)\n", val, xhci_crc_error_src(val));
                                break;

                        default:
                                xhci_plat_warn("unknow crc_src\n");
                }
#endif
        }
        else
                xhci_plat_warn("%s failed, unknown command - %s\n", __func__, buf);

        return count;
}

static DEVICE_ATTR(debug_control, 0644, show_xhci_debug_control, store_xhci_debug_control);


//===========================================================================
// Func : show_xhci_get_port_status_str
//===========================================================================
static ssize_t show_xhci_get_port_status_str(struct device *dev, struct device_attribute *attr, char *buf)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);

        return _xhci_get_port_status_str(xhci, buf, PAGE_SIZE);
}

static DEVICE_ATTR(port_status, 0444, show_xhci_get_port_status_str, NULL);


const struct attribute *xhci_platform_dev_attrs[] =
{
        &dev_attr_u2_phy1_config.attr,
        &dev_attr_u2_phy2_config.attr,
        &dev_attr_u3_phy_config.attr,
        &dev_attr_debug_control.attr,
        &dev_attr_port_status.attr,
        NULL,
};
