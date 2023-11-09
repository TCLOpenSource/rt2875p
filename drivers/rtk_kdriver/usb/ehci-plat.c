/*
 * Generic platform ehci driver
 *
 * Copyright 2007 Steven Brown <sbrown@cortland.com>
 * Copyright 2010-2012 Hauke Mehrtens <hauke@hauke-m.de>
 * Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * Derived from the ohci-ssb driver
 * Copyright 2007 Michael Buesch <m@bues.ch>
 *
 * Derived from the EHCI-PCI driver
 * Copyright (c) 2000-2004 by David Brownell
 *
 * Derived from the ohci-pci driver
 * Copyright 1999 Roman Weissgaerber
 * Copyright 2000-2002 David Brownell
 * Copyright 1999 Linus Torvalds
 * Copyright 1999 Gregory P. Smith
 *
 * Licensed under the GNU/GPL. See COPYING for details.
 */
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
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_gpio.h>
#include <rtk_kdriver/rtk_otp_region_api.h>
#include <mach/rtk_platform.h>
#include <mach/platform.h>
#include "ehci.h"
#include "dwc2.h"
#include "ehci-plat.h"
#ifdef CONFIG_REALTEK_OF
#include <linux/of_address.h>
#endif

static struct platform_device *ehci_platform_dev = NULL;

#define DRIVER_DESC "EHCI generic platform driver"

LIST_HEAD(ehci_list_head);

/* Function delcaration */
static void hub_polling_check(struct timer_list *t);

static const char hcd_name[] = "ehci-platform";

static int id_ehci_top = ID_EHCI_TOP;
static int id_ehci_ex_top = ID_EHCI_EX_TOP;
static const struct of_device_id rtk_ehci_ids[] =
{
        { .compatible = "rtk,ehci-top", .data = &id_ehci_top },
        { .compatible = "rtk,ehci-ex-top", .data = &id_ehci_ex_top },
        /*
         * (Temporarily workaround)
         * Due to of_node's compatible and type will disappear after booting from snapshot image.
         * To promise driver could match device, we add external one that matches via "name" only.
         */
        { .name = "ehci_top", .data = &id_ehci_top },
        { .name = "ehci_ex_top", .data = &id_ehci_ex_top },
        {}
};
MODULE_DEVICE_TABLE(of, rtk_ehci_ids);

/* ========================================================================================== */


/*  copy from hub.c and rename */
/*
 * USB 2.0 spec Section 11.24.2.2
 */
static int hub_clear_port_feature(struct usb_device *hdev, int port1, int feature)
{
        return usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0),
                               USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature, port1,
                               NULL, 0, 1000);
}


/*
 * USB 2.0 spec Section 11.24.2.13
 */
static int hub_set_port_feature(struct usb_device *hdev, int port1, int feature)
{
        return usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0),
                               USB_REQ_SET_FEATURE, USB_RT_PORT, feature, port1,
                               NULL, 0, 1000);
}

/* Port Test Modes.
 * Refs to section 4.14 in Enhanced Host Controller Interface Specification for Universal Serial Bus specification
 * */
int rtk_ehci_port_test_mode(struct usb_device *udev, int port1, int mode)
{
        struct usb_hcd *hcd = container_of(udev->bus, struct usb_hcd, self);
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);
        struct ehci_hcd *ehci;
        int i;

        if (udev->maxchild < port1)
        {
                 ehci_plat_err("port1 is out of range. 1~%d \n", udev->maxchild);
                return -1;
        }
        if (mode > 5)
        {
                 ehci_plat_err("mode is out of range. 0~5 \n");
                return -1;
        }
        if (priv->udev_in_test)
        {
                 ehci_plat_err("dev: %s is currently in test \n", dev_name(&priv->udev_in_test->dev));
                return -1;
        }
        if (mode != 0)
        {
                usb_lock_device(udev);
                priv->udev_in_test = udev;
        }

        /* is root hub */
        if (!udev->parent)
        {
                ehci = hcd_to_ehci(hcd);

                if (mode == 0)
                {
                         ehci_plat_dbg("Leave test mode ...\n");

                        ehci_plat_dbg("set the Run/Stop(R/S) bit in the USBCMD register to a '0'\n");
                        ehci_writel(ehci, (ehci->regs->command & ~(CMD_RUN)), &ehci->regs->command);

                        ehci_plat_dbg("wait for HCHalted(HCH) bit in the USBSTS register to transition to a '1'\n");
                        do
                        {
                                msleep(100);
                        }
                        while (!(ehci->regs->status & STS_HALT));

                        ehci_plat_dbg("set the Host Controller Reset(HCRST) bit in the USBCMD register to a '1'\n");
                        ehci_writel(ehci, (ehci->regs->command | CMD_RESET), &ehci->regs->command);

                        ehci_plat_dbg("Leave test mode , OK !!!\n");

                }
                else
                {
                        /* Disable the periodic and asynchronous schedules */
                        ehci_writel(ehci, (ehci->regs->command & ~(CMD_ASE | CMD_PSE)), &ehci->regs->command);

                        /* Place all enabled root ports into the suspended state by setting the Suspend bit
                        * in each appropriate PORTSC register to a one. */
                        for (i = 0; i < udev->maxchild; i++)
                        {
                                if (ehci->regs->port_status[i] & PORT_PE)
                                {
                                        ehci_plat_info("port%d(%p) is enable. set to suspend \n", i, &ehci->regs->port_status[i]);
                                        ehci_writel(ehci, (ehci->regs->port_status[i] | PORT_SUSPEND), &ehci->regs->port_status[i]);
                                }
                        }

                        /* Set the Run/Stop bit in the USBCMD register to a zero and wait for the HCHalted bit in the USBSTS
                        * register, to transition to a one. Note that an EHCI host controller implementation may optionally allow
                        * port testing with the Run/Stop bit set to a one. However, all host controllers must support port testing
                        * with Run/Stop set to a zero and HCHalted set to a one. */
                        ehci_writel(ehci, (ehci->regs->command & ~(CMD_RUN)), &ehci->regs->command);
                        do
                        {
                                msleep(100);
                        }
                        while (!(ehci->regs->status & STS_HALT));

                        /* Set the Port Test Control field in the port under test PORTSC register to the value corresponding to the
                        * desired test mode. If the selected test is Test_Force_Enable, then the Run/Stop bit in the USBCMD
                        * register must then be transitioned back to one, in order to enable transmission of SOFs out of the port
                        * under test. */
                        ehci_writel(ehci, ((ehci->regs->port_status[port1 - 1] & ~PORT_TEST(0xf)) | PORT_TEST(mode)), &ehci->regs->port_status[port1 - 1]);
                        if (PORT_TEST(mode) == PORT_TEST_FORCE)
                        {
                                ehci_plat_info("Is Test_Force_Enable. Need to set Run/Stop bit in USBCMD to 1 \n");
                                ehci_writel(ehci, (ehci->regs->command | CMD_RUN), &ehci->regs->command);
                        }
                }

        }
        else
        {
                /* Not root hub.
                 * Please refers to section 11.24.2.12 (Set Port Feature) in USB 2.0 spec*/
                if (mode == 0)
                {
                        ehci_plat_dbg("Leave test mode ...\n");

                        ehci_plat_dbg("clear USB_PORT_FEAT_POWER to the parent of the hub\n");
                        for (i = 1; i <= udev->parent->maxchild; i++)
                        {
                                ehci_plat_dbg("processing port %d of %d...\n", i, udev->parent->maxchild);
                                hub_clear_port_feature(udev->parent, i, USB_PORT_FEAT_POWER);
                                msleep(1000);
                        }

                        ehci_plat_dbg("set USB_PORT_FEAT_POWER to the parent of the hub\n");
                        for (i = 1; i <= udev->parent->maxchild; i++)
                        {
                                ehci_plat_dbg("processing port %d of %d...\n", i, udev->parent->maxchild);
                                hub_set_port_feature(udev->parent, i, USB_PORT_FEAT_POWER);
                                msleep(1000);
                        }

                        ehci_plat_dbg("Leave test mode , OK !!!\n");
                }
                else
                {
                        ehci_plat_dbg("Enter test mode ...\n");

                        ehci_plat_dbg("set USB_PORT_FEAT_SUSPEND to all ports of the hub\n");
                        for (i = 1; i <= udev->maxchild; i++)
                        {
                                ehci_plat_dbg("processing port %d of %d...\n", i, udev->maxchild);
                                hub_set_port_feature(udev, i, USB_PORT_FEAT_SUSPEND);
                                msleep(1000);
                        }

                        /* When the feature selector is PORT_TEST, the most significant byte(bit 15..8) of the wIndex field
                         * is the selector identifying the specific test mode. */
                        ehci_plat_dbg("set USB_PORT_FEAT_TEST mode %d to port %d ...\n", mode, port1);
                        hub_set_port_feature(udev, (mode << 8) | port1, USB_PORT_FEAT_TEST);
                        msleep(1000);

                        ehci_plat_dbg("Enter test mode , OK !!!\n");
                }
        }

        if (mode == 0)
        {
                usb_unlock_device(udev);
                priv->udev_in_test = NULL;
        }

        return 0;
}
EXPORT_SYMBOL(rtk_ehci_port_test_mode);


static ssize_t _get_crc_str(struct ehci_hcd *ehci, char *buf, ssize_t count)
{
        ssize_t n = 0;
        unsigned ports = HCS_N_PORTS(ehci->hcs_params), port1 = 1;
        char *ptr = buf;

        if (ports < 1)
                return -ENODEV;

        n = scnprintf(buf, count, "CRC_ERR={");
        if ((count -= n) < 0)
                goto overflow;
        buf +=n;

        for (port1 = 1; port1 <= ports; port1++)
        {
                n = scnprintf(buf, count, "%d, ",
                              get_usb2_wrap_crc_err_cnt(ehci, port1));
                if ((count -= n) < 0)
                        goto overflow;
                buf +=n;
        }

        /* Truncate last two char: comma and space */
        count -= 2;
        buf -= 2;

        n = scnprintf(buf, count, "} ");
        if ((count -= n) < 0)
                goto overflow;
        buf +=n;

        return buf - ptr;

overflow:
        return -EOVERFLOW;
}


/* Dump ehci portsc of each port */
ssize_t _get_ehci_portsc_str(struct ehci_hcd *ehci, char *buf, ssize_t size)
{
        char *ptr = buf, *crc_buf;
        ssize_t n = 0;
        unsigned ports = HCS_N_PORTS(ehci->hcs_params), portnum = 0;

        n = scnprintf(buf, size, "U2 PORTSC={");
        if ((size -= n) < 0)
                return -EOVERFLOW;
        buf += n;

        for (portnum = 0; portnum < ports; portnum++)
        {
                n = scnprintf(buf, size, "0x%x, ",
                              ehci_readl(ehci, &ehci->regs->port_status[portnum]));
                buf += n;
                if ((size -= n) < 0)
                        return -EOVERFLOW;
        }
        buf -= 2; /* truncate last comma and space. */

        n = scnprintf(buf, size, "}");
        if ((size -= n) < 0)
                return -EOVERFLOW;
        buf += n;

        // May be called in timer ISR, can't sleep
        crc_buf = kzalloc(size, GFP_ATOMIC);
        if (crc_buf)
        {
                if (_get_crc_str(ehci, crc_buf, size) > 0)
                {
                        n = scnprintf(buf, size, " %s \n", crc_buf);
                        if ((size -= n) < 0)
                        {
                                kfree(crc_buf);
                                return -EOVERFLOW;
                        }
                        buf += n;
                }
                kfree(crc_buf);
        }
        else
        {
                n = scnprintf(buf, size, "\n");
                if ((size -= n) < 0)
                        return -EOVERFLOW;
                buf += n;
        }

        return buf - ptr;
}

static void dump_portsc_ex(struct ehci_hcd *ehci, unsigned char warn)
{
        char *buf;
        ssize_t n = 0;

        // May be called in timer IRQ, can't schedule
        buf = kzalloc(PAGE_SIZE, GFP_ATOMIC);
        if (!buf)
                return;

        n = _get_ehci_portsc_str(ehci, buf, PAGE_SIZE);
        if (n)
        {
                if (warn)
                        ehci_plat_warn("%s", buf);
                else
                        ehci_plat_dbg("%s", buf);
        }
        kfree(buf);
}

#define dump_portsc(ehci)   dump_portsc_ex(ehci, 0);

static void hub_polling_check(struct timer_list *t)
{
        struct ehci_platform_priv *priv = from_timer(priv, t, polling_check_timer);
        struct ehci_hcd *ehci = priv->ehci;


        if (!priv->is_timer_on ||
            priv->polling_check_running ||
            (priv->debug_ctrl_flag & NO_EHCI_POLLING_CHECK))
                return;

        priv->polling_check_running = true;
        dump_portsc(ehci);
        priv->polling_check_running = false;

        if (priv->is_timer_on && !(priv->debug_ctrl_flag & NO_EHCI_POLLING_CHECK))
                mod_timer(&priv->polling_check_timer, jiffies + msecs_to_jiffies(EHCI_POLLING_CHECK_TIME));
}


void start_ehci_polling_check_timer(struct ehci_hcd *ehci)
{
        struct ehci_platform_priv *priv = (struct ehci_platform_priv *)ehci->priv;

        if (!priv->is_timer_on)
        {
                priv->is_timer_on = true;
                timer_setup(&priv->polling_check_timer, hub_polling_check, 0);
                mod_timer(&priv->polling_check_timer, jiffies + msecs_to_jiffies(EHCI_POLLING_CHECK_TIME));
                ehci_plat_info("[jason] %s: okay\n", __func__);
        }
}

void stop_ehci_polling_check_timer(struct ehci_hcd *ehci)
{
        struct ehci_platform_priv *priv = (struct ehci_platform_priv *)ehci->priv;

        if (priv->is_timer_on)
        {
                priv->is_timer_on = false;
                del_timer_sync(&priv->polling_check_timer);
        }
}

struct completion *id_get_companion_ehci_completion(int id)
{
        struct ehci_platform_priv *priv, *next;

        list_for_each_entry_safe(priv, next, &ehci_list_head, ehci_list)
        {
                if (priv->id == id)
                        return priv->phy_mac_completion;
        }

        return NULL;
}
EXPORT_SYMBOL(id_get_companion_ehci_completion);


static int ehci_platform_reset(struct usb_hcd *hcd)
{
        struct platform_device *pdev = to_platform_device(hcd->self.controller);
        struct usb_ehci_pdata *pdata = dev_get_platdata(&pdev->dev);
        struct ehci_hcd *ehci = hcd_to_ehci(hcd);
        int retval;

        ehci->has_synopsys_hc_bug = pdata->has_synopsys_hc_bug;

        if (pdata->pre_setup)
        {
                retval = pdata->pre_setup(hcd);
                if (retval < 0)
                        return retval;
        }

        ehci->caps = hcd->regs + pdata->caps_offset;
        retval = ehci_setup(hcd);
        if (retval)
                return retval;

        if (pdata->no_io_watchdog)
                ehci->need_io_watchdog = 0;
        return 0;
}

static int vbus_on(bool on)
{
        u64 pin;
        RTK_GPIO_ID gid;
        char *pin_name = "PIN_USB_5V_EN_4";
        int ret = 0;

        ret = pcb_mgr_get_enum_info_byname(pin_name, &pin);
        if (ret)
        {
                ehci_plat_err("fail: get PCB parameter (%s) \n", pin_name);
                return -ENODEV;
        }

        gid = rtk_gpio_id(GET_GPIO_TYPE(pin), GET_PIN_INDEX(pin));
        ret = rtk_gpio_output(gid, on);
        if (ret)
                return ret;

        ret = rtk_gpio_set_dir(gid, 1);  // set gpio to output mode

        return ret;
}

static int rtk_ehci_platform_power_on(struct platform_device *dev)
{
        return 0;
}

static void rtk_ehci_platform_power_off(struct platform_device *dev)
{
        // We do nothing here.
}

static struct hc_driver __read_mostly ehci_platform_hc_driver;

#ifdef CONFIG_USER_INITCALL_USB
static const struct ehci_driver_overrides platform_overrides =
{
#else
static const struct ehci_driver_overrides platform_overrides __initconst =
{
#endif
        .reset = ehci_platform_reset,
        .extra_priv_size = sizeof(struct ehci_platform_priv),
};

static struct usb_ehci_pdata ehci_platform_defaults =
{
        .power_on      = rtk_ehci_platform_power_on,
        .power_off     = rtk_ehci_platform_power_off,
        .power_suspend = rtk_ehci_platform_power_off,
};


extern struct attribute *ehci_platform_dev_attrs[];
static const struct attribute_group platform_dev_attr_grp =
{
        .attrs = ehci_platform_dev_attrs,
};

static int ehci_platform_probe(struct platform_device *dev)
{
        struct usb_hcd *hcd;
        struct resource *res_mem;
        struct usb_ehci_pdata *pdata = dev_get_platdata(&dev->dev);
        struct ehci_platform_priv *priv;
        struct ehci_hcd *ehci;
        int err, irq, phy_num, clk = 0;
        const struct of_device_id *of_id = of_match_device(rtk_ehci_ids, &dev->dev);
        int platform_id = -1;
        int n_ports = 0;

        if (usb_disabled())
                return -ENODEV;

        if (of_id && of_id->data)
                platform_id = *((int *)of_id->data);
        else if (platform_get_device_id(dev))
                platform_id = platform_get_device_id(dev)->driver_data;

        if (platform_id < 0)
                return -ENODEV;

	if(platform_id == 0)
        	ehci_platform_dev = dev;		

#ifdef CONFIG_USB_USER_RESUME
        dev->dev.power.is_userresume = true;
#endif

        /*
        * Use reasonable defaults so platforms don't have to provide these
        * with DT probing on ARM.
        */
        if (!pdata)
                pdata = &ehci_platform_defaults;

        err = dma_coerce_mask_and_coherent(&dev->dev,
                                           pdata->dma_mask_64 ? DMA_BIT_MASK(64) : DMA_BIT_MASK(32));
        if (err)
        {
                ehci_plat_err("Error: DMA mask configuration failed\n");
                return err;
        }

        irq = platform_get_irq(dev, 0);
        if (irq < 0)
        {
                 ehci_plat_err("no irq provided");
                return irq;
        }

        hcd = usb_create_hcd(&ehci_platform_hc_driver, &dev->dev,
                             dev_name(&dev->dev));
        if (!hcd)
                return -ENOMEM;

        platform_set_drvdata(dev, hcd);
        dev->dev.platform_data = pdata;
        priv = hcd_to_ehci_priv(hcd);
        ehci = hcd_to_ehci(hcd);
        priv->id = platform_id;  /* set id to indenty USB MAC */
        priv->ehci = ehci;

        if (pdata == &ehci_platform_defaults && dev->dev.of_node)
        {
                if (of_property_read_bool(dev->dev.of_node, "big-endian-regs"))
                        ehci->big_endian_mmio = 1;

                if (of_property_read_bool(dev->dev.of_node, "big-endian-desc"))
                        ehci->big_endian_desc = 1;

                if (of_property_read_bool(dev->dev.of_node, "big-endian"))
                        ehci->big_endian_mmio = ehci->big_endian_desc = 1;

                if (of_property_read_bool(dev->dev.of_node,
                                          "needs-reset-on-resume"))
                        priv->reset_on_resume = true;

                if (of_property_read_bool(dev->dev.of_node,
                                          "has-transaction-translator"))
                        hcd->has_tt = 1;

                priv->num_phys = of_count_phandle_with_args(dev->dev.of_node,
                                 "phys", "#phy-cells");

                if (priv->num_phys > 0)
                {
                        priv->phys = devm_kcalloc(&dev->dev, priv->num_phys,
                                                  sizeof(struct phy *), GFP_KERNEL);
                        if (!priv->phys)
                                return -ENOMEM;
                }
                else
                        priv->num_phys = 0;

                for (phy_num = 0; phy_num < priv->num_phys; phy_num++)
                {
                        priv->phys[phy_num] = devm_of_phy_get_by_index(
                                                      &dev->dev, dev->dev.of_node, phy_num);
                        if (IS_ERR(priv->phys[phy_num]))
                        {
                                err = PTR_ERR(priv->phys[phy_num]);
                                goto err_put_hcd;
                        }
                }

                for (clk = 0; clk < EHCI_MAX_CLKS; clk++)
                {
                        priv->clks[clk] = of_clk_get(dev->dev.of_node, clk);
                        if (IS_ERR(priv->clks[clk]))
                        {
                                err = PTR_ERR(priv->clks[clk]);
                                if (err == -EPROBE_DEFER)
                                        goto err_put_clks;
                                priv->clks[clk] = NULL;
                                break;
                        }
                }
        }

        priv->rst = devm_reset_control_get_optional(&dev->dev, NULL);
        if (IS_ERR(priv->rst))
        {
                err = PTR_ERR(priv->rst);
                if (err == -EPROBE_DEFER)
                        goto err_put_clks;
                priv->rst = NULL;
        }
        else
        {
                err = reset_control_deassert(priv->rst);
                if (err)
                        goto err_put_clks;
        }

        if (pdata->big_endian_desc)
                ehci->big_endian_desc = 1;
        if (pdata->big_endian_mmio)
                ehci->big_endian_mmio = 1;
        if (pdata->has_tt)
                hcd->has_tt = 1;
        if (pdata->reset_on_resume)
                priv->reset_on_resume = true;

#ifndef CONFIG_USB_EHCI_BIG_ENDIAN_MMIO
        if (ehci->big_endian_mmio)
        {
                 ehci_plat_err(
                        "Error: CONFIG_USB_EHCI_BIG_ENDIAN_MMIO not set\n");
                err = -EINVAL;
                goto err_reset;
        }
#endif
#ifndef CONFIG_USB_EHCI_BIG_ENDIAN_DESC
        if (ehci->big_endian_desc)
        {
                 ehci_plat_err(
                        "Error: CONFIG_USB_EHCI_BIG_ENDIAN_DESC not set\n");
                err = -EINVAL;
                goto err_reset;
        }
#endif

        if (pdata->power_on)
        {
                vbus_on(false);
                err = pdata->power_on(dev);
                if (err < 0)
                        goto err_reset;
        }

        res_mem = platform_get_resource(dev, IORESOURCE_MEM, 0);
        hcd->regs = devm_ioremap_resource(&dev->dev, res_mem);
        if (IS_ERR(hcd->regs))
        {
                err = PTR_ERR(hcd->regs);
                goto err_power;
        }
        hcd->rsrc_start = res_mem->start;
        hcd->rsrc_len = resource_size(res_mem);

        ehci_init_wrapper_regs(hcd, hcd->rsrc_start);

        /* Map hcd regs to ehci regs,
         * also setup ehci op regs and synopsys specific regs */
        ehci->caps = hcd->regs;  /* FIXME: Need offset if ehci has PCI config regs? */
        ehci->regs = (void __iomem *)ehci->caps +
                     HC_LENGTH(ehci, ehci_readl(ehci, &ehci->caps->hc_capbase));
        priv->synop_spec_regs = ((void __iomem *)ehci->regs) + EHCI_SYNOPSYS_REGS_OFFSET;


        n_ports = HCS_N_PORTS(ehci_readl(ehci, &ehci->caps->hcs_params));

        /* Map to right vstatus register(USB wrapper) */
        priv->vstatus_regs = id_get_ehci_vstatus_regs(priv);
        if (IS_ERR(priv->vstatus_regs))
        {
                err = PTR_ERR(priv->vstatus_regs);
                goto err_power;
        }

        /* PHY and MAC init part.
         * If we have done before (ex: re-modprobe this module), we won't do this again. */
        priv->phy_mac_completion = id_get_ehci_completion(priv);
        if (IS_ERR(priv->phy_mac_completion))
        {
                err = PTR_ERR(priv->phy_mac_completion);
                goto err_power;
        }

        if (!completion_done(priv->phy_mac_completion))
        {
                if ((err = ehci_usb_mac_init(dev)) ||
                    (err = ehci_usb2_phy_init(dev)) ||
                    (err = ehci_usb_mac_post_init(dev, false)))
                        goto err_power;

                complete_all(priv->phy_mac_completion);
        }

        //FIXME: This fix performance issue in K5,
        //       cause magice remote trigger too many ehci interrupt to CPU,
        //       so we bind ehci irq to CPU3
#if 0
        if (irq_can_set_affinity(irq))
                irq_set_affinity(irq, cpumask_of(3));
#endif

        err = usb_add_hcd(hcd, irq, IRQF_SHARED);
        if (err)
        {
                goto err_power;
        }

        device_enable_async_suspend(&dev->dev);

        device_wakeup_enable(hcd->self.controller);
        platform_set_drvdata(dev, hcd);

        if (sysfs_create_group(&dev->dev.kobj, &platform_dev_attr_grp))
        {
                ehci_plat_warn("Create self-defined sysfs attributes fail \n");
        }

        start_ehci_polling_check_timer(ehci);

        list_add_tail(&priv->ehci_list, &ehci_list_head); /* This is used to track how many ehci do we have */

        vbus_on(true);

        return err;

err_power:
        if (pdata->power_off)
                pdata->power_off(dev);
err_reset:
        if (priv->rst)
                reset_control_assert(priv->rst);
err_put_clks:
        while (--clk >= 0)
                clk_put(priv->clks[clk]);
err_put_hcd:
        if (pdata == &ehci_platform_defaults)
                dev->dev.platform_data = NULL;

        usb_put_hcd(hcd);

        return err;
}

static int ehci_platform_remove(struct platform_device *dev)
{
        struct usb_hcd *hcd = platform_get_drvdata(dev);
        struct usb_ehci_pdata *pdata = dev_get_platdata(&dev->dev);
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);

        int clk;

        stop_ehci_polling_check_timer(hcd_to_ehci(hcd));

        sysfs_remove_group(&dev->dev.kobj, &platform_dev_attr_grp);

        usb_remove_hcd(hcd);

        if (pdata->power_off)
                pdata->power_off(dev);

        if (priv->rst)
                reset_control_assert(priv->rst);

        for (clk = 0; clk < EHCI_MAX_CLKS && priv->clks[clk]; clk++)
                clk_put(priv->clks[clk]);

        usb_put_hcd(hcd);

        if (pdata == &ehci_platform_defaults)
                dev->dev.platform_data = NULL;

        return 0;
}

#ifdef CONFIG_PM_SLEEP
static int ehci_platform_suspend(struct device *dev)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct usb_ehci_pdata *pdata = dev_get_platdata(dev);
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);
        struct platform_device *pdev = to_platform_device(dev);
        bool do_wakeup = device_may_wakeup(dev);
        int ret;

        stop_ehci_polling_check_timer(hcd_to_ehci(hcd));

        ret = ehci_suspend(hcd, do_wakeup);
        if (ret)
                return ret;

        if (pdata->power_suspend)
                pdata->power_suspend(pdev);

        /* suspend will turn off the USB IC power,
         * the phy, mac value will be reset to default val,
         * we need to reinit it again in resume flow.*/
        reinit_completion(priv->phy_mac_completion);

        return ret;
}

static int ehci_platform_resume(struct device *dev)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct usb_ehci_pdata *pdata = dev_get_platdata(dev);
        struct platform_device *pdev = to_platform_device(dev);
        struct ehci_platform_priv *priv = hcd_to_ehci_priv(hcd);
        int err;
#if 0
        int irq;
#endif

        if (pdata->power_on)
        {
                int err = pdata->power_on(pdev);
                if (err < 0)
                        return err;
        }

        if (!completion_done(priv->phy_mac_completion))
        {
                if ((err = ehci_usb_mac_init(pdev)) ||
                    (err = ehci_usb2_phy_init(pdev)) ||
                    (err = ehci_usb_mac_post_init(pdev, true)))
                        goto err_power;

                complete_all(priv->phy_mac_completion);
        }

        ehci_resume(hcd, priv->reset_on_resume);

        start_ehci_polling_check_timer(hcd_to_ehci(hcd));

        return 0;

err_power:
        if (pdata->power_off)
                pdata->power_off(pdev);
        return err;
}
#endif /* CONFIG_PM_SLEEP */


static const struct acpi_device_id ehci_acpi_match[] __maybe_unused=
{
        { "PNP0D20", 0 }, /* EHCI controller without debug */
        { }
};
MODULE_DEVICE_TABLE(acpi, ehci_acpi_match);

static const struct platform_device_id ehci_platform_table[] =
{
        { "ehci-platform", ID_EHCI_TOP },
        { }
};
MODULE_DEVICE_TABLE(platform, ehci_platform_table);

static SIMPLE_DEV_PM_OPS(ehci_platform_pm_ops, ehci_platform_suspend,
                         ehci_platform_resume);

static struct platform_driver ehci_platform_driver =
{
        .id_table   = ehci_platform_table,
        .probe      = ehci_platform_probe,
        .remove     = ehci_platform_remove,
        .shutdown   = usb_hcd_platform_shutdown,
        .driver     = {
                .name = "rtk-ehci-plat",
                .pm = &ehci_platform_pm_ops,
                .of_match_table = rtk_ehci_ids,
                .acpi_match_table = ACPI_PTR(ehci_acpi_match),
        }
};


#ifdef CONFIG_USER_INITCALL_USB
static int ehci_platform_init(void)
#else
static int __init ehci_platform_init(void)
#endif
{
        if (usb_disabled())
                return -ENODEV;

        ehci_plat_info("%s: " DRIVER_DESC "\n", hcd_name);

        ehci_init_driver(&ehci_platform_hc_driver, &platform_overrides);
		
        return platform_driver_register(&ehci_platform_driver);
}
#if defined(CONFIG_USER_INITCALL_USB) && !defined(MODULE)
user_initcall_grp("USB", ehci_platform_init);
#else
module_init(ehci_platform_init);
#endif

static void __exit ehci_platform_cleanup(void)
{
        platform_driver_unregister(&ehci_platform_driver);
}
module_exit(ehci_platform_cleanup);

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("Hauke Mehrtens");
MODULE_AUTHOR("Alan Stern");
MODULE_LICENSE("GPL");

struct platform_device *get_ehci_platform_device(void)
{
        return ehci_platform_dev;
}
EXPORT_SYMBOL(get_ehci_platform_device);

