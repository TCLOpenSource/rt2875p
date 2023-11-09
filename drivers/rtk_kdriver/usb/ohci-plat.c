/*
 * Generic platform ohci driver
 *
 * Copyright 2007 Michael Buesch <m@bues.ch>
 * Copyright 2011-2012 Hauke Mehrtens <hauke@hauke-m.de>
 * Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * Derived from the OCHI-SSB driver
 * Derived from the OHCI-PCI driver
 * Copyright 1999 Roman Weissgaerber
 * Copyright 2000-2002 David Brownell
 * Copyright 1999 Linus Torvalds
 * Copyright 1999 Gregory P. Smith
 *
 * Licensed under the GNU/GPL. See COPYING for details.
 */

#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/hrtimer.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/usb/ohci_pdriver.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/usb/phy.h>
#include <rtk_kdriver/io.h>
#include <linux/of_device.h>
#include <rtd_log/rtd_module_log.h>
#include "ohci.h"

#define DRIVER_DESC "OHCI generic platform driver"
#define OHCI_MAX_CLKS 3
#define hcd_to_ohci_priv(h) ((struct ohci_platform_priv *)hcd_to_ohci(h)->priv)

#define ohci_plat_dbg(fmt, args...)          rtd_pr_ohci_plat_debug(fmt, ## args)
#define ohci_plat_info(fmt, args...)         rtd_pr_ohci_plat_info(fmt, ## args)
#define ohci_plat_warn(fmt, args...)        rtd_pr_ohci_plat_warn(fmt, ## args)
#define ohci_plat_err(fmt, args...)          rtd_pr_ohci_plat_err(fmt, ## args)

#ifdef CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM
const unsigned int wait_timeout = 1000;
extern struct completion *id_get_companion_ehci_completion(int id);
void usb2_crt_on(void);
#endif

struct ohci_platform_priv
{
        struct clk *clks[OHCI_MAX_CLKS];
        struct reset_control *rst;
        struct phy **phys;
        int num_phys;
        /********* RTK defined *********/
        int id;
        /********* RTK defined *********/
};

static const char hcd_name[] = "ohci-platform";

#define ID_OHCI_TOP    0
#define ID_OHCI_EX_TOP    1

static int ohci_top_index = ID_OHCI_TOP;
static int ohci_ex_top_index = ID_OHCI_EX_TOP;
static const struct of_device_id ohci_platform_ids[] =
{
        { .compatible = "rtk,ohci-top", .data = &ohci_top_index },
        { .compatible = "rtk,ohci-ex-top", .data = &ohci_ex_top_index },
        /*
         * (Temporarily workaround)
         * Due to of_node's compatible and type will disappear after booting from snapshot image.
         * To promise driver could match device, we add external one that matches via "name" only.
         */
        { .name = "ohci_top", .data = &ohci_top_index },
        { .name = "ohci_ex_top", .data = &ohci_ex_top_index },
        { }
};
MODULE_DEVICE_TABLE(of, ohci_platform_ids);


#if 0
static int ohci_platform_power_on(struct platform_device *dev)
{
        struct usb_hcd *hcd = platform_get_drvdata(dev);
        struct ohci_platform_priv *priv = hcd_to_ohci_priv(hcd);
        int clk, ret, phy_num;

        for (clk = 0; clk < OHCI_MAX_CLKS && priv->clks[clk]; clk++)
        {
                ret = clk_prepare_enable(priv->clks[clk]);
                if (ret)
                        goto err_disable_clks;
        }

        for (phy_num = 0; phy_num < priv->num_phys; phy_num++)
        {
                ret = phy_init(priv->phys[phy_num]);
                if (ret)
                        goto err_exit_phy;
                ret = phy_power_on(priv->phys[phy_num]);
                if (ret)
                {
                        phy_exit(priv->phys[phy_num]);
                        goto err_exit_phy;
                }
        }

        return 0;

err_exit_phy:
        while (--phy_num >= 0)
        {
                phy_power_off(priv->phys[phy_num]);
                phy_exit(priv->phys[phy_num]);
        }
err_disable_clks:
        while (--clk >= 0)
                clk_disable_unprepare(priv->clks[clk]);

        return ret;
}
#endif

static int rtk_ohci_platform_power_on(struct platform_device *dev)
{
#ifdef CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM
        usb2_crt_on();
#endif
        return 0;
}

#if 0
static void ohci_platform_power_off(struct platform_device *dev)
{
        struct usb_hcd *hcd = platform_get_drvdata(dev);
        struct ohci_platform_priv *priv = hcd_to_ohci_priv(hcd);
        int clk, phy_num;

        for (phy_num = 0; phy_num < priv->num_phys; phy_num++)
        {
                phy_power_off(priv->phys[phy_num]);
                phy_exit(priv->phys[phy_num]);
        }

        for (clk = OHCI_MAX_CLKS - 1; clk >= 0; clk--)
                if (priv->clks[clk])
                        clk_disable_unprepare(priv->clks[clk]);
}
#endif

static void rtk_ohci_platform_power_off(struct platform_device *dev)
{
        // We do nothing here.
}

static struct hc_driver __read_mostly ohci_platform_hc_driver;

#ifdef CONFIG_USER_INITCALL_USB
static const struct ohci_driver_overrides platform_overrides =
{
#else
static const struct ohci_driver_overrides platform_overrides __initconst =
{
#endif
        .product_desc =         "Generic Platform OHCI controller",
        .extra_priv_size =      sizeof(struct ohci_platform_priv),
};

static struct usb_ohci_pdata ohci_platform_defaults =
{
#if 1 /* We use rtk self-defined power on/off seuqeunce */
        .power_on      = rtk_ohci_platform_power_on,
        .power_suspend = rtk_ohci_platform_power_off,
        .power_off     = rtk_ohci_platform_power_off,
#else
        .power_on =             ohci_platform_power_on,
        .power_suspend =        ohci_platform_power_off,
        .power_off =            ohci_platform_power_off,
#endif
};

static int ohci_platform_probe(struct platform_device *dev)
{
        struct usb_hcd *hcd;
        struct resource *res_mem;
        struct usb_ohci_pdata *pdata = dev_get_platdata(&dev->dev);
        struct ohci_platform_priv *priv;
        struct ohci_hcd *ohci;
        int err, irq, phy_num, clk = 0;
        const struct of_device_id *of_id = of_match_device(ohci_platform_ids, &dev->dev);
        int platform_id = -1;
#ifdef CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM
        struct completion *phy_mac_completion;
#endif

        if (usb_disabled())
                return -ENODEV;

        if (of_id && of_id->data)
                platform_id = *((int *)of_id->data);
        else if (platform_get_device_id(dev))
                platform_id = platform_get_device_id(dev)->driver_data;

        if (platform_id < 0)
                return -ENODEV;

#ifdef CONFIG_USB_USER_RESUME
        dev->dev.power.is_userresume = true;
#endif

        /*
         * Use reasonable defaults so platforms don't have to provide these
         * with DT probing on ARM.
         */
        if (!pdata)
                pdata = &ohci_platform_defaults;

        err = dma_coerce_mask_and_coherent(&dev->dev, DMA_BIT_MASK(32));
        if (err)
                return err;

        irq = platform_get_irq(dev, 0);
        if (irq < 0)
        {
                ohci_plat_err("no irq provided");
                return irq;
        }

        hcd = usb_create_hcd(&ohci_platform_hc_driver, &dev->dev,
                             dev_name(&dev->dev));
        if (!hcd)
                return -ENOMEM;

        platform_set_drvdata(dev, hcd);
        dev->dev.platform_data = pdata;
        priv = hcd_to_ohci_priv(hcd);
        ohci = hcd_to_ohci(hcd);
        priv->id = platform_id;  /* set id to indenty USB MAC */

        if (pdata == &ohci_platform_defaults && dev->dev.of_node)
        {
                if (of_property_read_bool(dev->dev.of_node, "big-endian-regs"))
                        ohci->flags |= OHCI_QUIRK_BE_MMIO;

                if (of_property_read_bool(dev->dev.of_node, "big-endian-desc"))
                        ohci->flags |= OHCI_QUIRK_BE_DESC;

                if (of_property_read_bool(dev->dev.of_node, "big-endian"))
                        ohci->flags |= OHCI_QUIRK_BE_MMIO | OHCI_QUIRK_BE_DESC;

                if (of_property_read_bool(dev->dev.of_node, "no-big-frame-no"))
                        ohci->flags |= OHCI_QUIRK_FRAME_NO;

                of_property_read_u32(dev->dev.of_node, "num-ports",
                                     &ohci->num_ports);

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

                for (clk = 0; clk < OHCI_MAX_CLKS; clk++)
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
                ohci->flags |= OHCI_QUIRK_BE_DESC;
        if (pdata->big_endian_mmio)
                ohci->flags |= OHCI_QUIRK_BE_MMIO;
        if (pdata->no_big_frame_no)
                ohci->flags |= OHCI_QUIRK_FRAME_NO;
        if (pdata->num_ports)
                ohci->num_ports = pdata->num_ports;

#ifndef CONFIG_USB_OHCI_BIG_ENDIAN_MMIO
        if (ohci->flags & OHCI_QUIRK_BE_MMIO)
        {
                ohci_plat_err(
                        "Error: CONFIG_USB_OHCI_BIG_ENDIAN_MMIO not set\n");
                err = -EINVAL;
                goto err_reset;
        }
#endif
#ifndef CONFIG_USB_OHCI_BIG_ENDIAN_DESC
        if (ohci->flags & OHCI_QUIRK_BE_DESC)
        {
                ohci_plat_err(
                        "Error: CONFIG_USB_OHCI_BIG_ENDIAN_DESC not set\n");
                err = -EINVAL;
                goto err_reset;
        }
#endif

        if (pdata->power_on)
        {
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

#ifdef CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM
        /* wait for ehci to finish MAC and PHY flow */
        phy_mac_completion = id_get_companion_ehci_completion(priv->id);
        if (phy_mac_completion)
        {
                if (!wait_for_completion_timeout(phy_mac_completion, msecs_to_jiffies(wait_timeout)))
                {
                        ohci_plat_err("%s(%d) wait for usb2_phy_mac_flow timeout(%d) ms \n",
                                __func__, __LINE__,
                                wait_timeout);
                        err = -ETIME;
                        goto err_power;
                }
        }
#endif

        err = usb_add_hcd(hcd, irq, IRQF_SHARED);
        if (err)
        {
                goto err_power;
        }

        device_enable_async_suspend(&dev->dev);

        device_wakeup_enable(hcd->self.controller);

        platform_set_drvdata(dev, hcd);

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
        if (pdata == &ohci_platform_defaults)
                dev->dev.platform_data = NULL;

        usb_put_hcd(hcd);

        return err;
}

static int ohci_platform_remove(struct platform_device *dev)
{
        struct usb_hcd *hcd = platform_get_drvdata(dev);
        struct usb_ohci_pdata *pdata = dev_get_platdata(&dev->dev);
        struct ohci_platform_priv *priv = hcd_to_ohci_priv(hcd);
        int clk;

        usb_remove_hcd(hcd);

        if (pdata->power_off)
                pdata->power_off(dev);

        if (priv->rst)
                reset_control_assert(priv->rst);

        for (clk = 0; clk < OHCI_MAX_CLKS && priv->clks[clk]; clk++)
                clk_put(priv->clks[clk]);

        usb_put_hcd(hcd);

        if (pdata == &ohci_platform_defaults)
                dev->dev.platform_data = NULL;

        return 0;
}

#ifdef CONFIG_PM_SLEEP
static int ohci_platform_suspend(struct device *dev)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct usb_ohci_pdata *pdata = dev->platform_data;
        struct platform_device *pdev =
                container_of(dev, struct platform_device, dev);
        bool do_wakeup = device_may_wakeup(dev);
        int ret;

        ret = ohci_suspend(hcd, do_wakeup);
        if (ret)
                return ret;

        if (pdata->power_suspend)
                pdata->power_suspend(pdev);

        return ret;
}

static int ohci_platform_resume(struct device *dev)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct usb_ohci_pdata *pdata = dev_get_platdata(dev);
        struct platform_device *pdev =
                container_of(dev, struct platform_device, dev);
#ifdef CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM
        struct completion *phy_mac_completion;
        struct ohci_platform_priv *priv = hcd_to_ohci_priv(hcd);
#endif

        if (pdata->power_on)
        {
                int err = pdata->power_on(pdev);
                if (err < 0)
                        return err;
        }

#ifdef CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM
        /* wait for ehci to finish MAC and PHY flow */
        phy_mac_completion = id_get_companion_ehci_completion(priv->id);
        if (phy_mac_completion)
        {
                if (!wait_for_completion_timeout(phy_mac_completion, msecs_to_jiffies(wait_timeout)))
                {
                        ohci_plat_err("%s(%d) wait for usb2_phy_mac_flow timeout(%d) ms \n",
                                __func__, __LINE__,
                                jiffies_to_msecs(wait_timeout));
                        return -ETIME;
                }
        }
#endif

        ohci_resume(hcd, false);

        /* turn on vbus after ohci_resume for quirk devices */
        if (IS_ENABLED(CONFIG_USB_PHY) && (hcd->usb_phy) && (hcd->usb_phy->dev))
        {
                struct device_node *np = hcd->usb_phy->dev->of_node;

                if (of_device_is_compatible(np, "lge,lg1k-usb1phy"))
                {
                        ohci_plat_info("turn on vbus here!\n");
                        usb_phy_vbus_on(hcd->usb_phy);
                }
        }
        return 0;
}

#endif /* CONFIG_PM_SLEEP */


static const struct platform_device_id ohci_platform_table[] =
{
        { "ohci-platform", ID_OHCI_TOP },
        { }
};
MODULE_DEVICE_TABLE(platform, ohci_platform_table);

static SIMPLE_DEV_PM_OPS(ohci_platform_pm_ops, ohci_platform_suspend,
                         ohci_platform_resume);

static struct platform_driver ohci_platform_driver =
{
        .id_table       = ohci_platform_table,
        .probe          = ohci_platform_probe,
        .remove         = ohci_platform_remove,
        .shutdown       = usb_hcd_platform_shutdown,
        .driver         = {
                .name   = "ohci-platform",
                .pm     = &ohci_platform_pm_ops,
                .of_match_table = ohci_platform_ids,
        }
};

#ifdef CONFIG_USER_INITCALL_USB
static int ohci_platform_init(void)
#else
static int __init ohci_platform_init(void)
#endif
{
        if (usb_disabled())
                return -ENODEV;

        ohci_plat_info("%s: " DRIVER_DESC "\n", hcd_name);

        ohci_init_driver(&ohci_platform_hc_driver, &platform_overrides);
        return platform_driver_register(&ohci_platform_driver);
}
#if defined(CONFIG_USER_INITCALL_USB) && !defined(MODULE)
user_initcall_grp("USB", ohci_platform_init);
#else
module_init(ohci_platform_init);
#endif

static void __exit ohci_platform_cleanup(void)
{
        platform_driver_unregister(&ohci_platform_driver);
}
module_exit(ohci_platform_cleanup);

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("Hauke Mehrtens");
MODULE_AUTHOR("Alan Stern");
MODULE_LICENSE("GPL");
