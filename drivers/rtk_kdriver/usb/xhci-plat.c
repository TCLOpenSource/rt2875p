/*
 * xhci-plat.c - xHCI host controller driver platform Bus Glue.
 *
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com
 * Author: Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * A lot of code borrowed from the Linux xHCI driver.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

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
#include <rtk_kdriver/rtk_otp_region_api.h>
#include <rtk_kdriver/io.h>
#include <rbus/usb3_top_reg.h>
#include <rtk_kdriver/rtk_crt.h>
#include <rbus/efuse_reg.h>
#include <mach/rtk_platform.h>
#include <rtk_kdriver/rtk-kdrv-common.h>
#include "xhci.h"
#include "xhci-plat.h"

static bool g_enable_xhci_device_mode = false;


#ifdef CONFIG_RTK_KDRV_XHCI_HCD_PLATFORM
static int usb_otg_switch_handler(char *buf)
{
        char *p = NULL;
        if ((p = strstr(buf, "on")) != NULL || (p = strstr(buf, "1")) != NULL)
                g_enable_xhci_device_mode = true;
        return 0;
}
__setup("usbotg", usb_otg_switch_handler);
#else
static int parse_otg_switch_handler(void)
{
    char strings[16] = {0};

    if(rtk_parse_commandline_equal("usbotg", strings,sizeof(strings)) == 0)
    {
        xhci_plat_err("Error : can't get usbotg from bootargs\n");
        return -1;
    }

    if(strcmp(strings, "on") == 0 || strcmp(strings, "1") == 0) {
        g_enable_xhci_device_mode = true;
    }
    else {
        g_enable_xhci_device_mode = false;
    }

    return 0;
}        
#endif

bool is_enable_xhci_device_mode(void)
{
        return g_enable_xhci_device_mode;
}
EXPORT_SYMBOL(is_enable_xhci_device_mode);

#ifdef CONFIG_RTK_KDRV_XHCI_SAVE_POWER_WHEN_NO_U3
#define POWER_SAVING_WORK_DELAY    10  /* in second */
#endif

static struct hc_driver __read_mostly xhci_plat_hc_driver;

static int xhci_plat_setup(struct usb_hcd *hcd);
static int xhci_plat_start(struct usb_hcd *hcd);

#ifdef CONFIG_USER_INITCALL_USB
static const struct xhci_driver_overrides xhci_plat_overrides =
{
#else
static const struct xhci_driver_overrides xhci_plat_overrides __initconst =
{
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        .extra_priv_size = sizeof(struct xhci_plat_priv),
#else
        .extra_priv_size = sizeof(struct xhci_hcd) + sizeof(struct xhci_plat_priv),
#endif
        .reset = xhci_plat_setup,
        .start = xhci_plat_start,
};

static int tool_obj_en = 0;
#if 1 // TODO: Need to change these stuff
static unsigned long u3_port_recovery_cnt = 0;
#define USB_PORT_ERR_CHECK_TIME    200
#define BLACKDEV_TIMEOUT           5

#define U3PortVBusStatus()       0
#define U3PortVBusControl(on)    {}

// TODO: Need to use usb_prot structue to get this info ?
#define GET_U2_IDX_OF_U3_WITH_U2_PORT(xhci)   (xhci->usb2_rhub.num_ports - 1)

static int is_u3_port_hs_mode(struct xhci_hcd *xhci)
{
        unsigned int u2_portsc, u3_portsc;

        u2_portsc = readl(xhci->usb2_rhub.ports[GET_U2_IDX_OF_U3_WITH_U2_PORT(xhci)]->addr);
        u3_portsc = readl(xhci->usb3_rhub.ports[0]->addr);

        return ((u2_portsc & (PORT_CONNECT | PORT_PE)) && !(u3_portsc & (PORT_CONNECT | PORT_PE)));
}

static void U3PortVBusToggle(struct xhci_hcd *xhci)
{
        if (!is_u3_port_hs_mode(xhci))
        {
                U3PortVBusControl(0);
                mdelay(50);
                U3PortVBusControl(1);
        }
}
#endif

static void xhci_priv_plat_start(struct usb_hcd *hcd)
{
        struct xhci_plat_priv *priv = hcd_to_xhci_priv(hcd);

        if (priv->plat_start)
                priv->plat_start(hcd);
}

static int __maybe_unused xhci_priv_init_quirk(struct usb_hcd *hcd)
{
        struct xhci_plat_priv *priv = hcd_to_xhci_priv(hcd);

        if (!priv->init_quirk)
                return 0;

        return priv->init_quirk(hcd);
}

static int xhci_priv_resume_quirk(struct usb_hcd *hcd)
{
        struct xhci_plat_priv *priv = hcd_to_xhci_priv(hcd);

        if (!priv->resume_quirk)
                return 0;

        return priv->resume_quirk(hcd);
}

static void xhci_plat_quirks(struct device *dev, struct xhci_hcd *xhci)
{
        /*
         * As of now platform drivers don't provide MSI support so we ensure
         * here that the generic code does not try to make a pci_dev from our
         * dev struct in order to setup MSI
         */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        xhci->quirks |= XHCI_PLAT;
#else
        xhci->quirks |= XHCI_BROKEN_MSI;
#endif
}

/* called during probe() after chip reset completes */
static int xhci_plat_setup(struct usb_hcd *hcd)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        int ret;


        ret = xhci_priv_init_quirk(hcd);
        if (ret)
                return ret;
#endif

        return xhci_gen_setup(hcd, xhci_plat_quirks);
}

static int xhci_plat_start(struct usb_hcd *hcd)
{
        xhci_priv_plat_start(hcd);
        return xhci_run(hcd);
}

#ifdef CONFIG_OF
#if 0
static const struct xhci_plat_priv xhci_plat_marvell_armada =
{
        .init_quirk = xhci_mvebu_mbus_init_quirk,
};

static const struct xhci_plat_priv xhci_plat_renesas_rcar_gen2 =
{
        .firmware_name = XHCI_RCAR_FIRMWARE_NAME_V1,
        .init_quirk = xhci_rcar_init_quirk,
        .plat_start = xhci_rcar_start,
        .resume_quirk = xhci_rcar_resume_quirk,
};

static const struct xhci_plat_priv xhci_plat_renesas_rcar_gen3 =
{
        .init_quirk = xhci_rcar_init_quirk,
        .plat_start = xhci_rcar_start,
        .resume_quirk = xhci_rcar_resume_quirk,
};
#endif

static const struct of_device_id usb_xhci_of_match[] =
{
        {
                .compatible = "generic-xhci",
        }, {
                .compatible = "xhci-platform",
        }, {
                .compatible = "rtk,xhci-top",
        }, {
                .compatible = "rtk,xhci-ex-top",
        }, {
                /*
                 * (Temporarily workaround)
                 * Due to of_node's compatible and type will disappear after booting from snapshot image.
                 * To promise driver could match device, we add external one that matches via "name" only.
                 */
                .name = "xhci_ex_top",
        }, {
                .compatible = "rtk,xhci-top",
        }, {
                /*
                 * (Temporarily workaround)
                 * Due to of_node's compatible and type will disappear after booting from snapshot image.
                 * To promise driver could match device, we add external one that matches via "name" only.
                 */
                .name = "xhci_top",
        }, {
#if 0
                .compatible = "marvell,armada-375-xhci",
                .data = &xhci_plat_marvell_armada,
        }, {
                .compatible = "marvell,armada-380-xhci",
                .data = &xhci_plat_marvell_armada,
        }, {
                .compatible = "renesas,xhci-r8a7790",
                .data = &xhci_plat_renesas_rcar_gen2,
        }, {
                .compatible = "renesas,xhci-r8a7791",
                .data = &xhci_plat_renesas_rcar_gen2,
        }, {
                .compatible = "renesas,xhci-r8a7793",
                .data = &xhci_plat_renesas_rcar_gen2,
        }, {
                .compatible = "renesas,xhci-r8a7795",
                .data = &xhci_plat_renesas_rcar_gen3,
        }, {
                .compatible = "renesas,xhci-r8a7796",
                .data = &xhci_plat_renesas_rcar_gen3,
        }, {
                .compatible = "renesas,rcar-gen2-xhci",
                .data = &xhci_plat_renesas_rcar_gen2,
        }, {
                .compatible = "renesas,rcar-gen3-xhci",
                .data = &xhci_plat_renesas_rcar_gen3,
#endif
        },
        {},
};
MODULE_DEVICE_TABLE(of, usb_xhci_of_match);
#endif

static void control_vbus_power_by_gpio(unsigned long long pin, bool on)
{
        unsigned int pin_type, gpio_num, gpio_type;
        RTK_GPIO_ID gid;

        pin_type = GET_PIN_TYPE(pin);
        gpio_num = GET_PIN_INDEX(pin);

        switch (pin_type)
        {
                case PCB_PIN_TYPE_GPIO:
                        gpio_type = GET_GPIO_TYPE(pin);
                        break;
                default:
                        gpio_type = PCB_PIN_TYPE_UNUSED;
        }

        if (gpio_type)
        {
                gid = rtk_gpio_id(gpio_type, gpio_num);
                rtk_gpio_output(gid, on);
                rtk_gpio_set_dir(gid, 1);  // set gpio to output mode
        }
}

static void enable_vbus_power(void)
{
        u64 pin_usb_power_ctrl2, pin_usb_power_ctrl3;

        if (!pcb_mgr_get_enum_info_byname("PIN_USB_POWER_CTRL2", &pin_usb_power_ctrl2))
                control_vbus_power_by_gpio(pin_usb_power_ctrl2, 1);

        if (!pcb_mgr_get_enum_info_byname("PIN_USB_POWER_CTRL3", &pin_usb_power_ctrl3))
                control_vbus_power_by_gpio(pin_usb_power_ctrl3, 1);
}

const char* xhci_crc_error_src(unsigned char id)
{
        switch (id)
        {
                case CRC_SRC_NONE:
                        return "none";
                case CRC_SRC_U3_PORT:
                        return "U3 Port";
                case CRC_SRC_U2_PORT0:
                        return "U2 Port0";
                case CRC_SRC_U2_PORT1:
                        return "U2 Port1";
                case CRC_SRC_U2_PORT2:
                        return "U2 Port2";
                default:
                        return "Unknonw";
        }
}


int _xhci_get_port_status_str(struct xhci_hcd *xhci, char* ptr, size_t count)
{
        struct xhci_plat_priv *priv = xhci_to_xhci_priv(xhci);
        unsigned int val;
        int n = 0, i = 0;
        int len = 0, len_u2 = 0, len_u3 = 0;
        char *u2_portsc, *u3_portsc, *u2_portsc_str, *u3_portsc_str;
        size_t size_u2, size_u3, size_u2_str, size_u3_str;

        size_u2 = sizeof(*u2_portsc) * count / 4;
        u2_portsc = kzalloc(size_u2, GFP_ATOMIC);
        if (!u2_portsc)
                return -ENOMEM;

        size_u3 = sizeof(*u3_portsc) * count / 4;
        u3_portsc = kzalloc(size_u3, GFP_ATOMIC);
        if (!u3_portsc)
        {
                len = -ENOMEM;
                goto free_allocated_1;
        }

        size_u2_str = sizeof(*u2_portsc_str) * count / 2;
        u2_portsc_str = kzalloc(size_u2_str, GFP_ATOMIC);
        if (!u2_portsc_str)
        {
                len = -ENOMEM;
                goto free_allocated_2;
        }

        size_u3_str = sizeof(*u3_portsc_str) * count / 2;
        u3_portsc_str = kzalloc(size_u3_str, GFP_ATOMIC);
        if (!u3_portsc_str)
        {
                len = -ENOMEM;
                goto free_allocated_3;
        }

        for (i = 0; i < xhci->usb2_rhub.num_ports; i++)
        {
                n = scnprintf(u2_portsc, size_u2,
                              (i == xhci->usb2_rhub.num_ports - 1) ? "Port%d{STS=%x,Link=%04x}" : "Port%d{STS=%x,Link=%04x}, ",
                              i + 1,
                              readl(xhci->usb2_rhub.ports[i]->addr),
                              readl(&xhci->op_regs->port_link_base + NUM_PORT_REGS * i));
                u2_portsc += n;
                len_u2 += n;
                size_u2 -= n;
        }
        u2_portsc -= len_u2; /* Go back to start address */
        n = scnprintf(u2_portsc_str, size_u2_str, "U2 %s", u2_portsc);

        for (i = 0; i < xhci->usb3_rhub.num_ports; i++)
        {
                n = scnprintf(u3_portsc, size_u3, "Port%d{STS=%x,Link=%04x}",
                              i + 1,
                              readl(xhci->usb3_rhub.ports[i]->addr),
                              readl(&xhci->op_regs->port_link_base + NUM_PORT_REGS * (xhci->usb2_rhub.num_ports + i)));
                u3_portsc += n;
                len_u3 += n;
                size_u3 -= n;
        }
        u3_portsc -= len_u3; /* Go back to start address */
        n = scnprintf(u3_portsc_str, size_u3_str, "U3 %s", u3_portsc);

        if (xhci->usb3_rhub.num_ports)
        {
                n = scnprintf(ptr, count, "[XHCI] CMD=%03x, STS=%04x, %s, %s, PWR=%d, Recovery=%lu}\n",
                              readl(&xhci->op_regs->command), readl(&xhci->op_regs->status),
                              u2_portsc_str, u3_portsc_str,
                              U3PortVBusStatus(),
                              u3_port_recovery_cnt);
        }
        else
        {
                n = scnprintf(ptr, count, "[XHCI] CMD=%03x, STS=%04x, %s\n",
                              readl(&xhci->op_regs->command), readl(&xhci->op_regs->status),
                              u2_portsc_str);
        }
        ptr += n;
        len += n;
        count -= n;

        /* CRC */
        val = rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_CRC_CNT_1_REG);
        if (USB3_TOP_CRC_CNT_1_get_crc_cnt_en(val))
        {
                n = scnprintf(ptr, count, "[XHCI] CRC_CNT_1=(%08x) { SRC=%s, ERR=%u, Total=%u }\n",
                              val,
                              xhci_crc_error_src(USB3_TOP_CRC_CNT_1_get_crc_cnt_sel(val)),
                              USB3_TOP_CRC_CNT_1_get_crc_err_cnt(val),
                              rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_CRC_CNT_2_REG));

                ptr   += n;
                count -= n;
                len += n;
        }

        kfree(u3_portsc_str);
free_allocated_3:
        kfree(u2_portsc_str);
free_allocated_2:
        kfree(u3_portsc);
free_allocated_1:
        kfree(u2_portsc);

        return len;
}

void dump_port_status_ex(struct xhci_hcd *xhci, unsigned char warn)
{
        char *buff;
        size_t size = sizeof(*buff) * 1024;

        buff = kzalloc(size, GFP_ATOMIC);
        if (!buff)
        {
                xhci_plat_err("alloc buff fail. out of memory \n");
                return;
        }

        _xhci_get_port_status_str(xhci, buff, size);

        if (warn)
                xhci_plat_warn("%s", buff);
        else
                xhci_plat_dbg("%s", buff);

        kfree(buff);
}

#define dump_port_status(xhci)      dump_port_status_ex(xhci, 0)

#if 1 // TODO: Need to change these stuff start
static void xhci_reset_u3_port(struct xhci_hcd *xhci)
{
        unsigned int u2_portsc, u3_portsc;
        int idx = GET_U2_IDX_OF_U3_WITH_U2_PORT(xhci);

        xhci_plat_info("xhci_reset_u3_port, (U3 Port HS mode=%d)\n", is_u3_port_hs_mode(xhci));

        if(!is_u3_port_hs_mode(xhci))
                U3PortVBusControl(0);

        u3_portsc = readl(xhci->usb3_rhub.ports[0]->addr);
        writel((u3_portsc & ~PORT_POWER), xhci->usb3_rhub.ports[0]->addr);
        /* In merlin3, the last U2 port is companied with U3 port. (same connector) */
        u2_portsc = readl(xhci->usb2_rhub.ports[idx]->addr);
        writel((u2_portsc & ~PORT_POWER), xhci->usb2_rhub.ports[idx]->addr);
        udelay(10);

        u3_portsc = readl(xhci->usb3_rhub.ports[0]->addr);
        writel(u3_portsc | PORT_POWER | PORT_CSC, xhci->usb3_rhub.ports[0]->addr); // set port power and clear CSC
        u2_portsc = readl(xhci->usb2_rhub.ports[idx]->addr);
        writel(u2_portsc | PORT_POWER | PORT_CSC, xhci->usb2_rhub.ports[idx]->addr); // set port power and clear CSC
        mdelay(50);

        xhci_plat_info("xhci_reset_u3_port, reset U3/U2 port (U3 Status=%08X, U2 Status=%08X)\n",
                       readl(xhci->usb3_rhub.ports[0]->addr),
                       readl(xhci->usb2_rhub.ports[idx]->addr));

        if(!is_u3_port_hs_mode(xhci))
                U3PortVBusControl(1);

        dump_port_status_ex(xhci, 1);
}
#endif

#if 1 // TODO: Need to change these stuff start
// TODO: Need to get info from usb_port ?
extern int usb3_device_plugin;
extern bool is_usb3_on(void);
static void __maybe_unused xhci_hub_polling_check(unsigned long data)
{
        struct xhci_hcd *xhci = (struct xhci_hcd *)data;
        struct xhci_plat_priv *priv = xhci_to_xhci_priv(xhci);
        unsigned int u2_portsc, u3_portsc;
        unsigned int polling_interval = USB_HUB_CHECK_TIME;
        unsigned int idx_of_u3_companied_u2 = GET_U2_IDX_OF_U3_WITH_U2_PORT(xhci);
        struct xhci_global_regs __iomem *global_regs = xhci_to_xhci_priv(xhci)->global_regs;
        int i = 0;

        if (!is_usb3_on() ||
            priv->polling_check_suspend ||
            priv->polling_check_running ||
            (priv->debug_ctrl_flag & NO_HUB_POLLING_CHECK) ||
            priv->disable_polling_check)
                return;

        if (is_u3_port_hs_mode(xhci))
        {
                priv->black_device_count++;
                if (priv->black_device_count > BLACKDEV_TIMEOUT)
                {
                        rtd_clearbits(priv->wrapper_regs.XHCI_USB3_TOP_USB_TMP_0_REG,(0x1 << 8));
                        priv->black_device_count = 0;
                }
                goto restart_timer;
        }

        priv->polling_check_running = true;

        if (priv->u3_delayed_vbus_on)
        {
                U3PortVBusControl(1);
                priv->u3_delayed_vbus_on = 0;
        }

        dump_port_status(xhci);

        if (xhci->usb3_rhub.num_ports == 0 || (priv->debug_ctrl_flag & NO_U3_PORT_AUTO_RECOVER))   // skip auto recover
                goto restart_timer;

#if 1
        /*------------- check U3 Port --------------------
          In K2L, the U3 port is connected to both an U3
          phy and an U2 Phy (Port 2).

          The status below is the normal case
          1) disconnected : U3 = 2A0, U2 = 2A0
          2) SS device connected : U3 = 1203, U2 = 2A0
          3) HS device connected : U3 =  2A0, U2 = e03
          4) HS device connected : U3 =  2A0, U2 = 603
          5) U3 hub connected    : U3 = 1203, U2 = e03

          However, there is some special case the need to be handled
          6) U3 port disabled    : U3 = 290 (PLS=SS.Disabled), U2 is connected
          Might caused by U3 unexceptly disconnection.
          The HC uses U2 to reconnect it automatically.

          that also can be divided into 2 case
          a) U2 is not disabled (PE=1)
          In this case, the U2 work normally.
          We have nothing to do .

          b) U2 is disabled (PLS=0x7 Disabled)
          In this case, the U2 is abnormal too
          we need to fix it by toggling the VBus.

          7) U3 port is in SS.Inactive. We do Warm Port Reset to enter RxDetect
          -------------------------------------------------*/
        // case 6: U3 Port is disabled for some reason (except we use debug_control interface to disable U3. "no_u3_super_speed_port=1")
        u3_portsc = readl(xhci->usb3_rhub.ports[0]->addr);
        if (((u3_portsc & PORT_PLS_MASK) == XDEV_DISABLED) && !(priv->debug_ctrl_flag & NO_U3_SUPER_SPEED_PORT))
        {
                writel(u3_portsc & ~PORT_POWER, xhci->usb3_rhub.ports[0]->addr);
                writel(readl(xhci->usb3_rhub.ports[0]->addr) | PORT_POWER, xhci->usb3_rhub.ports[0]->addr);

                xhci_plat_warn("%s : u3 port disabled, toogle port_power, state (%08x -> %08x), cnt=%lu\n",
                               __FUNCTION__,
                               u3_portsc,
                               readl(xhci->usb3_rhub.ports[0]->addr),
                               u3_port_recovery_cnt);
                u3_port_recovery_cnt++;
        }

        // case 6-b : device detected by U2 port 2, but u2 port 2 is in disabled state
        //          and the U3 port has no device detected
        //          (that means the U3 port is routed to U2 phy,
        if (((readl(xhci->usb3_rhub.ports[0]->addr) & (PORT_CONNECT|PORT_PE)) == 0) &&
            ((readl(xhci->usb2_rhub.ports[idx_of_u3_companied_u2]->addr) & PORT_PLS_MASK) == XDEV_POLLING) &&   // Port 2 enter disable state
            ((readl(xhci->usb2_rhub.ports[idx_of_u3_companied_u2]->addr) & PORT_CONNECT) == PORT_CONNECT))
        {
                priv->u3_port_reset_hang++;
                polling_interval = USB_PORT_ERR_CHECK_TIME;

                dump_port_status_ex(xhci, 1);

                xhci_plat_warn("%s : u3 port reset hanged=%u, U3PORT=%08x, U2PORT=%08x\n",
                               __FUNCTION__, priv->u3_port_reset_hang, readl(xhci->usb3_rhub.ports[0]->addr), readl(xhci->usb2_rhub.ports[idx_of_u3_companied_u2]->addr));

                if (priv->u3_port_reset_hang >= (1500/polling_interval))
                {
                        priv->u3_port_reset_hang_count++;
                        u3_port_recovery_cnt++;

                        xhci_plat_warn("%s : u3 port reset hanged count=%d, do Vbus toogle\n",
                                       __FUNCTION__, priv->u3_port_reset_hang_count);

                        xhci_reset_u3_port(xhci);
                        priv->u3_port_reset_hang = 0;
                }
        }
        // case 7: U3 port is in SS.Inactive. We do Warm Port Reset to enter RxDetect
        else if (((readl(xhci->usb3_rhub.ports[0]->addr) & PORT_PLS_MASK)) == XDEV_INACTIVE)
        {
                u3_portsc = readl(xhci->usb3_rhub.ports[0]->addr);
                priv->u3_port_reset_hang++;
                polling_interval = USB_PORT_ERR_CHECK_TIME;

                dump_port_status_ex(xhci, 1);

                xhci_plat_warn("%s : u3 port reset hanged=%u, U3PORT=%08X \n",
                               __FUNCTION__, priv->u3_port_reset_hang, u3_portsc);
                if (priv->u3_port_reset_hang >= (1500/polling_interval))
                {
                        priv->u3_port_reset_hang_count++;
                        u3_port_recovery_cnt++;

                        xhci_plat_warn("%s : u3 port reset hanged count=%d, U3 is in Inactive, do Warm Port Reset\n",
                                       __FUNCTION__, priv->u3_port_reset_hang_count);

                        writel(u3_portsc | PORT_WR, xhci->usb3_rhub.ports[0]->addr);  // Issue a Warm Port Reset

                        priv->u3_port_reset_hang = 0;
                }
        }
        else
                priv->u3_port_reset_hang = 0;

#endif
        /*------------- check U3 connection status --------------------
          The b8058164 [10:9] (GDBGLTSSM[10:9] refs to DWC_USB3 spec) is another way to detect the U3 device connection
          This value will be 0 if HW detected the U3 signal.
          --------------------------------------------------------------*/
        if ((readl(&global_regs->gdbgltssm) & (0x3 << 9)) == (0 << 9))   //check usb3 device plug in connector
        {

                //--------------------------------------------------------------------------
                // U3 device is not detected by the Hub driver.
                // but the U3 port status is under one of the following condition
                // PLS = PORTSC[5:8]
                //   0x1203 : PLS = 0x00 (U0)
                //   0x340  : PLS = 0x10 (compliance test mode)
                //   0x300  : PLS = 0x08 (U3 Recovery mode)
                //--------------------------------------------------------------------------
                u3_portsc = readl(xhci->usb3_rhub.ports[0]->addr);

                if (((u3_portsc == 0x1203) ||   // Connected correctly
                     (u3_portsc == 0x340)  ||   // compliance test mode
                     (u3_portsc == 0x300)) &&   // U3 Recovery mode
                    (usb3_device_plugin == 0))
                {
                        priv->u3_port_count++;

                        polling_interval = USB_PORT_ERR_CHECK_TIME; // change to smaller polling interval

                        if (priv->u3_port_count >= (2000/polling_interval))
                        {
                                dump_port_status_ex(xhci, 1);
                                xhci_plat_warn("usb3_device_plugin=%d U3PortCnt=%d portstatus=%#x\n", usb3_device_plugin, priv->u3_port_count, u3_portsc);
                                switch (u3_portsc)
                                {
                                        case 0x340 : // exit compliance test mode
                                        case 0x1203 :
                                                writel(u3_portsc & ~PORT_POWER, xhci->usb3_rhub.ports[0]->addr);
                                                u3_portsc = readl(xhci->usb3_rhub.ports[0]->addr);
                                                writel(u3_portsc | PORT_POWER, xhci->usb3_rhub.ports[0]->addr);
                                                U3PortVBusToggle(xhci);
                                                break;
                                        default :
                                                U3PortVBusToggle(xhci);
                                                break;
                                }
                                priv->u3_port_count = 0;
                                xhci_plat_warn("vbus reset device\n");
                                goto restart_timer;
                        }
                }

                //--------------------------------------------------------------------------
                // neither U3 nor U2 detected the device
                //--------------------------------------------------------------------------
                if (((readl(xhci->usb3_rhub.ports[0]->addr) & PORT_PLS_MASK) == XDEV_POLLING) &&
                    ((readl(xhci->usb2_rhub.ports[idx_of_u3_companied_u2]->addr) & PORT_CONNECT) == 0))
                {
                        xhci_plat_warn("usb3_device_plugin=%d port_check_count=%d \n",
                                       usb3_device_plugin, priv->port_check_count);

                        ++priv->port_check_count;
                        polling_interval = USB_PORT_ERR_CHECK_TIME; // change to smaller polling interval

                        /*------------- check U3 connection status --------------------
                          usb3_device_plugin is a global variable that defined in usb/core/hub.c
                          that is used to ident2ify the system detected the U3 device on the Root Hub
                          or not.
                          --------------------------------------------------------------*/
                        if (usb3_device_plugin)
                        {
                                if (priv->port_check_count >= (2000/polling_interval)) // toggle vbus after 3 sec
                                {
                                        xhci_usb3_mdio_write(xhci, 0x0, 0x1, 0x02, 0x0013);
                                        U3PortVBusToggle(xhci);
                                        priv->port_check_count = 0;
                                        xhci_plat_warn("vbus reset device1");
                                }
                        }
                        else
                        {
                                if (priv->port_check_count >=(1500/polling_interval)) // toggle vbus after 2 sec
                                {
                                        xhci_usb3_mdio_write(xhci, 0x0, 0x1, 0x02, 0x0013);
                                        xhci_plat_warn("[before reset vbus] %p=%x, %p=%x\n",
                                                       xhci->usb3_rhub.ports[0]->addr,
                                                       readl(xhci->usb3_rhub.ports[0]->addr),
                                                       xhci->usb2_rhub.ports[idx_of_u3_companied_u2]->addr,
                                                       readl(xhci->usb2_rhub.ports[idx_of_u3_companied_u2]->addr));

                                        U3PortVBusToggle(xhci);

                                        priv->port_check_count = 0;
                                        usb3_device_plugin = 0;
                                        xhci_plat_warn("vbus reset device\n");
                                        xhci_plat_warn("[after reset vbus] %p=%x, %p=%x",
                                                       xhci->usb3_rhub.ports[0]->addr,
                                                       readl(xhci->usb3_rhub.ports[0]->addr),
                                                       xhci->usb2_rhub.ports[idx_of_u3_companied_u2]->addr,
                                                       readl(xhci->usb2_rhub.ports[idx_of_u3_companied_u2]->addr));
                                }
                        }
                }
                else
                        priv->port_check_count = 0;
        }
        else
                priv->port_check_count = 0;

        /*------------- check each U2 connection status --------------------
         * If U2 PORTSC.PLS equal to "ee1", we need to do port reset to recover
         --------------------------------------------------------------*/
        for (i = 0; i < xhci->usb2_rhub.num_ports; i++)
        {
                u2_portsc = readl(xhci->usb2_rhub.ports[i]->addr);
                if ((u2_portsc & PORT_CONNECT) && !(u2_portsc & PORT_PE))
                {
                        priv->u2_port_reset_hang++;
                        if (priv->u2_port_reset_hang >= (1500/USB_PORT_ERR_CHECK_TIME))
                        {
                                writel(u2_portsc | PORT_RESET, xhci->usb2_rhub.ports[i]->addr); /* Issue a port reset */
                                priv->u2_port_reset_hang = 0;
                                priv->u2_port_reset_hang_count++;
                                xhci_plat_warn("U2-%d portsc=0x%x, after port reset=0x%x. cnt=%d \n",
                                               i + 1, u2_portsc,
                                               readl(xhci->usb2_rhub.ports[i]->addr), priv->u2_port_reset_hang_count);
                        }
                }
        }

restart_timer:
        priv->polling_check_running = 0;
        mod_timer(&priv->polling_check_timer, jiffies + msecs_to_jiffies(USB_HUB_CHECK_TIME));
}
#endif

// TODO: Need to change these stuff start
#if 0
static void hub_polling_check_timer_on(struct xhci_hcd *xhci, bool setup)
{
        struct xhci_plat_priv *priv = xhci_to_xhci_priv(xhci);

        if (setup)
                setup_timer(&priv->polling_check_timer, xhci_hub_polling_check, (unsigned long)xhci);
        mod_timer(&priv->polling_check_timer, jiffies + msecs_to_jiffies(USB_HUB_CHECK_TIME));
        priv->polling_check_suspend = false;
        priv->disable_polling_check = false;
}

static void hub_polling_check_timer_off(struct xhci_hcd *xhci)
{
        struct xhci_plat_priv *priv = xhci_to_xhci_priv(xhci);

        priv->polling_check_suspend = true;
        del_timer_sync(&priv->polling_check_timer);
#if 0
        usb3_device_plugin = 0;
        port_check_count = 0;
        U3PortCnt = 0;
#endif
}
#else
static void hub_polling_check_timer_on(struct xhci_hcd *xhci, bool setup) {}
static void hub_polling_check_timer_off(struct xhci_hcd *xhci) {}
#endif

extern struct attribute *xhci_platform_dev_attrs[];
static const struct attribute_group platform_dev_attr_grp =
{
        .attrs = xhci_platform_dev_attrs,
};

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
 * Refs to section 4.19.6(Port Test Modes) in eXtensible Host Controller Interface for Universal Serial Bus (xHCI)
 * */
int rtk_xhci_port_test_mode(struct usb_device *udev, int port1, int mode)
{
        struct usb_hcd *hcd = container_of(udev->bus, struct usb_hcd, self);
        struct xhci_plat_priv *priv = hcd_to_xhci_priv(hcd);
        struct xhci_hcd *xhci;
        int i;
        unsigned int temp;

        if (udev->maxchild < port1)
        {
                xhci_plat_err("port1 is out of range.1~%d \n", udev->maxchild);
                return -1;
        }
        if (mode > 5)
        {
                xhci_plat_err("mode is out of range. 0~5 \n");
                return -1;
        }
        if (priv->udev_in_test && mode != 0)
        {
                xhci_plat_err("dev: %s is currently in test \n", dev_name(&priv->udev_in_test->dev));
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
                xhci = hcd_to_xhci(hcd);

                if (mode == 0x0)
                {
                        xhci_plat_dbg("Leave test mode ...\n");

                        xhci_plat_dbg("set the Run/Stop(R/S) bit in the USBCMD register to a '0'\n");
                        temp = readl(&xhci->op_regs->command);
                        writel((temp & ~CMD_RUN), &xhci->op_regs->command);

                        xhci_plat_dbg("wait for HCHalted(HCH) bit in the USBSTS register to transition to a '1'\n");
                        do
                        {
                                msleep(100);
                                temp = readl(&xhci->op_regs->status);
                        }
                        while (!(temp & STS_HALT));

                        xhci_plat_dbg("set the Host Controller Reset(HCRST) bit in the USBCMD register to a '1'\n");
                        temp = readl(&xhci->op_regs->command);
                        writel((temp | CMD_RESET), &xhci->op_regs->command);

                        xhci_plat_dbg("Leave test mode , OK !!!\n");

                }
                else
                {
                        xhci_plat_dbg("Enter test mode ...\n");

                        xhci_plat_dbg("clear USB_PORT_FEAT_POWER to the parent of the hub\n");
                        for (i = 1; i <= udev->maxchild; i++)
                        {
                                xhci_plat_dbg( "processing port %d of %d...\n", i, udev->maxchild);
                                hub_clear_port_feature(udev, i, USB_PORT_FEAT_POWER);
                                msleep(1000);
                        }

                        xhci_plat_dbg("set the Run/Stop(R/S) bit in the USBCMD register to a '0'\n");
                        temp = readl(&xhci->op_regs->command);
                        writel((temp & ~CMD_RUN), &xhci->op_regs->command);

                        xhci_plat_dbg("wait for HCHalted(HCH) bit in the USBSTS register to transition to a '1'\n");
                        do
                        {
                                msleep(100);
                                temp = readl(&xhci->op_regs->status);
                        }
                        while (!(temp & STS_HALT));

                        xhci_plat_dbg("set test mode %d to port %d ...\n", mode, port1);
                        temp = readl(&xhci->op_regs->port_power_base + (0x4 * (port1 - 1))); // type is le32, so plus one will go to next 4 byte.
                        writel((temp | (mode << 28)), &xhci->op_regs->port_power_base + (0x4 * (port1 - 1)));

                        /* Mode is FORCE_ENABLE */
                        if (mode == 0x5)
                        {
                                xhci_plat_dbg("set the Run/Stop(R/S) bit in the USBCMD register to a '1', in order to enable transmission of SOFs out of the port\n");
                                temp = readl(&xhci->op_regs->command);

                                writel(temp | CMD_RUN, &xhci->op_regs->command);
                        }
                        msleep(1000);
                        xhci_plat_dbg("Enter test mode , OK !!!\n");
                }
        }
        else
        {
                /* Not root hub.
                 * Please refers to section 11.24.2.12 (Set Port Feature) in USB 2.0 spec*/
                if (mode == 0)
                {
                        xhci_plat_dbg("Leave test mode ...\n");

                        xhci_plat_dbg( "clear USB_PORT_FEAT_POWER to the parent of the hub\n");
                        for (i = 1; i <= udev->parent->maxchild; i++)
                        {
                                xhci_plat_dbg( "processing port %d of %d...\n", i, udev->parent->maxchild);
                                hub_clear_port_feature(udev->parent, i, USB_PORT_FEAT_POWER);
                                msleep(1000);
                        }

                        xhci_plat_dbg( "set USB_PORT_FEAT_POWER to the parent of the hub\n");
                        for (i = 1; i <= udev->parent->maxchild; i++)
                        {
                                xhci_plat_dbg( "processing port %d of %d...\n", i, udev->parent->maxchild);
                                hub_set_port_feature(udev->parent, i, USB_PORT_FEAT_POWER);
                                msleep(1000);
                        }

                        xhci_plat_dbg( "Leave test mode , OK !!!\n");
                }
                else
                {
                        xhci_plat_dbg( "Enter test mode ...\n");

                        xhci_plat_dbg( "set USB_PORT_FEAT_SUSPEND to all ports of the hub\n");
                        for (i = 1; i <= udev->maxchild; i++)
                        {
                                xhci_plat_dbg( "processing port %d of %d...\n", i, udev->maxchild);
                                hub_set_port_feature(udev, i, USB_PORT_FEAT_SUSPEND);
                                msleep(1000);
                        }

                        /* When the feature selector is PORT_TEST, the most significant byte(bit 15..8) of the wIndex field
                         * is the selector identifying the specific test mode. */
                        xhci_plat_dbg( "set USB_PORT_FEAT_TEST mode %d to port %d ...\n", mode, port1);
                        hub_set_port_feature(udev, (mode << 8) | port1, USB_PORT_FEAT_TEST);
                        msleep(1000);

                        xhci_plat_dbg( "Enter test mode , OK !!!\n");
                }
        }

        if (mode == 0)
        {
                usb_unlock_device(udev);
                priv->udev_in_test = NULL;
        }

        return 0;
}
EXPORT_SYMBOL(rtk_xhci_port_test_mode);


#ifdef CONFIG_RTK_KDRV_XHCI_SAVE_POWER_WHEN_NO_U3
static void power_saving_dwork(struct work_struct *work)
{
        struct xhci_plat_priv *priv = (struct xhci_plat_priv *)container_of(work, struct xhci_plat_priv, power_saving_dwork.work);
        unsigned int set_value = 0;
        unsigned int i = 0;
        for(i  = 0; i < priv->usb3_port_cnt; i++)
                set_value |= (USB3_TOP_ANA_PHY0_usb3_pow_gating_p0_mask << (i * 8));
        rtd_clearbits(priv->wrapper_regs.XHCI_USB3_TOP_ANA_PHY0_REG, set_value);
        xhci_plat_info("%s(%d) 0x%x:0x%x. \n",
                __func__, __LINE__,
                priv->wrapper_regs.XHCI_USB3_TOP_ANA_PHY0_REG, rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_ANA_PHY0_REG));
}
#endif

static void __maybe_unused init_from_SD(void)
{
        /* Any setting from SD for bringup */
        xhci_plat_info("%s(%d)To check parameter with HW sd for bringup.!!!\n",
                 __func__,__LINE__);
}

static int xhci_get_id_from_dts(struct platform_device *pdev, unsigned int *p_id)
{
        struct device_node *p_node;

        if(!pdev || !p_id)
                return -EINVAL;

        p_node =  pdev->dev.of_node;

        if (!p_node)
        {
                return -ENODEV;
        }

        if(of_property_read_u32_index(p_node, "id", 0, p_id) != 0)
        {
                return -EIO;
        }
        return 0;
}

static int xhci_get_info_from_dts(struct platform_device *pdev)
{
        struct usb_hcd *hcd = platform_get_drvdata(pdev);
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);
        struct device_node *p_node;
        struct xhci_plat_priv *priv = xhci_to_xhci_priv(xhci);

        p_node =  pdev->dev.of_node;

        if (!p_node)
        {
                xhci_plat_err("%s get of device fail\n",__func__);
                return -ENODEV;
        }

        of_property_read_u32_index(p_node, "usb3portcnt", 0, &priv->usb3_port_cnt);
        if(priv->usb3_port_cnt == 0 || priv->usb3_port_cnt > MAX_XHCI_USB3_PHY_NUM)
        {
                xhci_plat_err("%s get error usb3portcnt %u\n",__func__, priv->usb3_port_cnt);
                return -EIO;
        }
        xhci_plat_info("%s get  usb3portcnt:%u\n",__func__, priv->usb3_port_cnt);

        of_property_read_u32_index(p_node, "usb2portcnt", 0, &priv->usb2_port_cnt);
        if(priv->usb2_port_cnt == 0)
        {
                xhci_plat_err("%s get error usb2portcnt\n",__func__);
                return -EIO;
        }
        xhci_plat_info("%s get  usb2portcnt:%u\n",__func__, priv->usb2_port_cnt);
        return 0;

}



static int xhci_plat_probe(struct platform_device *pdev)
{
        const struct of_device_id *match;
        const struct hc_driver *driver;
        struct device *sysdev;
        struct xhci_hcd *xhci;
        struct resource *res;
        struct usb_hcd *hcd;
        struct clk *clk = NULL;
        int ret;
        int irq;

        xhci_plat_info("%s(%d)[Alexkh]\n",__func__,__LINE__);
        if (usb_disabled())
                return -ENODEV;

        if(xhci_get_id_from_dts(pdev, (unsigned int *)(&pdev->id)) != 0)
                return -EIO;

#if IS_ENABLED(CONFIG_RTK_KDRV_XHCI_DRD_PLATFORM)
        if(pdev->id == 0 && is_enable_xhci_device_mode())
                return -ENODEV;
#endif

#ifdef CONFIG_USB_USER_RESUME
        pdev->dev.power.is_userresume = true;
#endif

        driver = &xhci_plat_hc_driver;

        irq = platform_get_irq(pdev, 0);
        if (irq < 0)
                return irq;

        /*
        * sysdev must point to a device that is known to the system firmware
        * or PCI hardware. We handle these three cases here:
        * 1. xhci_plat comes from firmware
        * 2. xhci_plat is child of a device from firmware (dwc3-plat)
        * 3. xhci_plat is grandchild of a pci device (dwc3-pci)
        */
        for (sysdev = &pdev->dev; sysdev; sysdev = sysdev->parent)
        {
                if (is_of_node(sysdev->fwnode) ||
                    is_acpi_device_node(sysdev->fwnode))
                        break;
#ifdef CONFIG_PCI
                else if (sysdev->bus == &pci_bus_type)
                        break;
#endif
        }

        if (!sysdev)
                sysdev = &pdev->dev;

        /* Try to set 64-bit DMA first */
        if (WARN_ON(!sysdev->dma_mask))
        {
                /* Platform did not initialize dma_mask */
                ret = dma_coerce_mask_and_coherent(sysdev,
                                                   DMA_BIT_MASK(64));
        }
        else
        {
                ret = dma_set_mask_and_coherent(sysdev, DMA_BIT_MASK(64));
        }

        /* If seting 64-bit DMA mask fails, fall back to 32-bit DMA mask */
        if (ret)
        {
                ret = dma_set_mask_and_coherent(sysdev, DMA_BIT_MASK(32));
                if (ret)
                        return ret;
        }

        pm_runtime_set_active(&pdev->dev);
        pm_runtime_enable(&pdev->dev);
        pm_runtime_get_noresume(&pdev->dev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        hcd = __usb_create_hcd(driver, sysdev, &pdev->dev,
                               dev_name(&pdev->dev), NULL);
#else
        hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
#endif
        if (!hcd)
        {
                ret = -ENOMEM;
                goto disable_runtime;
        }

        res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        hcd->regs = devm_ioremap_resource(&pdev->dev, res);
        if (IS_ERR(hcd->regs))
        {
                ret = PTR_ERR(hcd->regs);
                goto put_hcd;
        }
        hcd->rsrc_start = res->start;
        hcd->rsrc_len = resource_size(res);


        /* RTK USB platform clk */
        /*CRT_CLK_OnOff(USB, CLK_ON, (void *)3);*/

        xhci = hcd_to_xhci(hcd);
        match = of_match_node(usb_xhci_of_match, pdev->dev.of_node);
        if (match)
        {
                const struct xhci_plat_priv *priv_match = match->data;
                struct xhci_plat_priv *priv = hcd_to_xhci_priv(hcd);

                /* Just copy data for now */
                if (priv_match)
                        *priv = *priv_match;
        }

        xhci_init_wrapper_regs(hcd, hcd->rsrc_start);

        /* inidicate Synopsys USB3 global register address */
        hcd_to_xhci_priv(hcd)->global_regs = hcd->regs + HOST_GLOBAL_REGS_OFFSET;

        device_wakeup_enable(hcd->self.controller);

        xhci->clk = clk;
        xhci->main_hcd = hcd;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        xhci->shared_hcd = __usb_create_hcd(driver, sysdev, &pdev->dev,
                                            dev_name(&pdev->dev), hcd);
#else
        xhci->shared_hcd = usb_create_shared_hcd(driver, &pdev->dev, dev_name(&pdev->dev), hcd);
#endif
        if (!xhci->shared_hcd)
        {
                ret = -ENOMEM;
                goto disable_clk;
        }

        if (device_property_read_bool(sysdev, "usb3-lpm-capable"))
                xhci->quirks |= XHCI_LPM_SUPPORT;

        if (device_property_read_bool(&pdev->dev, "quirk-broken-port-ped"))
                xhci->quirks |= XHCI_BROKEN_PORT_PED;

        if((ret = xhci_get_info_from_dts(pdev)))
                goto put_usb3_hcd;

//#define PARAM_FOR_BRINGUP
#ifndef PARAM_FOR_BRINGUP
        /* RTK USB mac and phy init */
        if ((ret = xhci_usb_mac_init(pdev)) ||
            (ret = xhci_usb3_phy_init(pdev)) ||
            (ret = xhci_usb2_phy_init(pdev)) ||
            (ret = xhci_usb2_set_phy_from_otp(pdev)) ||
            (ret = xhci_usb_mac_postinit(pdev)))
                goto put_usb3_hcd;

        if(tool_obj_en)
        {
                if((ret = xhci_usb2_set_swing(pdev,0xe)))
                        goto put_usb3_hcd;
        }
#else
        /* for bring up */
        init_from_SD();
#endif



#ifdef CONFIG_RTK_KDRV_XHCI_SAVE_POWER_WHEN_NO_U3
        xhci->main_hcd->force_disable_auto_suspend = true;
        xhci->shared_hcd->force_disable_auto_suspend = true;
#endif

        ret = usb_add_hcd(hcd, irq, IRQF_SHARED);
        if (ret)
                goto disable_usb_phy;

        if (HCC_MAX_PSA(xhci->hcc_params) >= 4)
                xhci->shared_hcd->can_do_streams = 1;

        ret = usb_add_hcd(xhci->shared_hcd, irq, IRQF_SHARED);
        if (ret)
                goto dealloc_usb2_hcd;

        device_enable_async_suspend(&pdev->dev);
        pm_runtime_put_noidle(&pdev->dev);

        /*
        * Prevent runtime pm from being on as default, users should enable
        * runtime pm using power/control in sysfs.
        */
        pm_runtime_forbid(&pdev->dev);

        if (sysfs_create_group(&pdev->dev.kobj, &platform_dev_attr_grp))
        {
                xhci_plat_warn("Create self-defined sysfs attributes fail \n");
        }

        /* we enable vbus power for device here */
        enable_vbus_power();

        hub_polling_check_timer_on(xhci, true);

#ifdef CONFIG_RTK_KDRV_XHCI_SAVE_POWER_WHEN_NO_U3
        INIT_DELAYED_WORK(&hcd_to_xhci_priv(hcd)->power_saving_dwork, power_saving_dwork);
        queue_delayed_work(system_freezable_wq, &hcd_to_xhci_priv(hcd)->power_saving_dwork,
                           POWER_SAVING_WORK_DELAY * HZ);
#endif

        return 0;


dealloc_usb2_hcd:
        usb_remove_hcd(hcd);

disable_usb_phy:
        usb_phy_shutdown(hcd->usb_phy);

put_usb3_hcd:
        usb_put_hcd(xhci->shared_hcd);

disable_clk:
        if (!IS_ERR(clk))
                clk_disable_unprepare(clk);

put_hcd:
        usb_put_hcd(hcd);

disable_runtime:
        pm_runtime_put_noidle(&pdev->dev);
        pm_runtime_disable(&pdev->dev);

        return ret;
}

static int xhci_plat_remove(struct platform_device *dev)
{
        struct usb_hcd *hcd = platform_get_drvdata(dev);
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);
        struct clk *clk = xhci->clk;

        hub_polling_check_timer_off(xhci);

        xhci->xhc_state |= XHCI_STATE_REMOVING;

        usb_remove_hcd(xhci->shared_hcd);
        usb_phy_shutdown(hcd->usb_phy);

        usb_remove_hcd(hcd);
        usb_put_hcd(xhci->shared_hcd);

        if (!IS_ERR(clk))
                clk_disable_unprepare(clk);
        usb_put_hcd(hcd);

        pm_runtime_set_suspended(&dev->dev);
        pm_runtime_disable(&dev->dev);

        return 0;
}


static int xhci_plat_prepare(struct device *dev)
{
        struct usb_hcd __maybe_unused *hcd = dev_get_drvdata(dev);
        struct xhci_plat_priv __maybe_unused *priv = hcd_to_xhci_priv(hcd);
#ifdef CONFIG_RTK_KDRV_XHCI_SAVE_POWER_WHEN_NO_U3
        unsigned int set_value = 0;
        unsigned int i = 0;
        cancel_delayed_work_sync(&hcd_to_xhci_priv(hcd)->power_saving_dwork);

        for(i  = 0; i < priv->usb3_port_cnt; i++)
                set_value |= (USB3_TOP_ANA_PHY0_usb3_pow_gating_p0_mask << (i * 8));
        rtd_setbits(priv->wrapper_regs.XHCI_USB3_TOP_ANA_PHY0_REG, set_value);
        udelay(500);

        xhci_plat_info("%s(%d)  0x%x:0x%x \n",
                 __func__, __LINE__,
                 priv->wrapper_regs.XHCI_USB3_TOP_ANA_PHY0_REG, rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_ANA_PHY0_REG));
#endif
        return 0;
}


static int __maybe_unused xhci_plat_suspend(struct device *dev)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);
        int ret;

        hub_polling_check_timer_off(xhci);

        /*
         * xhci_suspend() needs `do_wakeup` to know whether host is allowed
         * to do wakeup during suspend. Since xhci_plat_suspend is currently
         * only designed for system suspend, device_may_wakeup() is enough
         * to dertermine whether host is allowed to do wakeup. Need to
         * reconsider this when xhci_plat_suspend enlarges its scope, e.g.,
         * also applies to runtime suspend.
         */
        ret = xhci_suspend(xhci, device_may_wakeup(dev));

        if (!device_may_wakeup(dev) && !IS_ERR(xhci->clk))
                clk_disable_unprepare(xhci->clk);

        return ret;
}

static int __maybe_unused xhci_plat_resume(struct device *dev)
{
        struct usb_hcd *hcd = dev_get_drvdata(dev);
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);
        int ret;
        struct platform_device *pdev = to_platform_device(dev);

#if 0
        if (!device_may_wakeup(dev) && !IS_ERR(xhci->clk))
                clk_prepare_enable(xhci->clk);
#else
        /* RTK USB platform clk */
        /*CRT_CLK_OnOff(USB, CLK_ON, (void *)3);*/
#endif

        /* RTK USB mac and phy init */
        if ((ret = xhci_usb_mac_init(pdev)) ||
            (ret = xhci_usb3_phy_init(pdev)) ||
            (ret = xhci_usb2_phy_init(pdev)) ||
            (ret = xhci_usb2_set_phy_from_otp(pdev)) ||
            (ret = xhci_usb_mac_postinit(pdev)))
                return ret;

        if ((ret = xhci_priv_resume_quirk(hcd)) ||
            (ret = xhci_resume(xhci, 0)))
                return ret;

        /* we enable vbus power for device here */
        enable_vbus_power();

        hub_polling_check_timer_on(xhci, false);

        return 0;
}



static void xhci_plat_complete(struct device *dev)
{
        struct usb_hcd __maybe_unused *hcd = dev_get_drvdata(dev);
        struct xhci_plat_priv __maybe_unused *priv = hcd_to_xhci_priv(hcd);
#ifdef CONFIG_RTK_KDRV_XHCI_SAVE_POWER_WHEN_NO_U3
        queue_delayed_work(system_freezable_wq, &hcd_to_xhci_priv(hcd)->power_saving_dwork,
                           POWER_SAVING_WORK_DELAY * HZ);

        xhci_plat_info("%s(%d) 0x%x:0x%x. \n",
                 __func__, __LINE__,
                 priv->wrapper_regs.XHCI_USB3_TOP_ANA_PHY0_REG, rtd_inl(priv->wrapper_regs.XHCI_USB3_TOP_ANA_PHY0_REG));
#endif
}

static int __maybe_unused xhci_plat_runtime_suspend(struct device *dev)
{
        struct usb_hcd  *hcd = dev_get_drvdata(dev);
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);

        return xhci_suspend(xhci, true);
}

static int __maybe_unused xhci_plat_runtime_resume(struct device *dev)
{
        struct usb_hcd  *hcd = dev_get_drvdata(dev);
        struct xhci_hcd *xhci = hcd_to_xhci(hcd);

        return xhci_resume(xhci, 0);
}

static const struct dev_pm_ops xhci_plat_pm_ops =
{
        SET_SYSTEM_SLEEP_PM_OPS(xhci_plat_suspend, xhci_plat_resume)
        .prepare = xhci_plat_prepare,
        .complete = xhci_plat_complete,

        SET_RUNTIME_PM_OPS(xhci_plat_runtime_suspend,
                           xhci_plat_runtime_resume,
                           NULL)
};

static const struct acpi_device_id usb_xhci_acpi_match[] =
{
        /* XHCI-compliant USB Controller */
        { "PNP0D10", },
        { }
};
MODULE_DEVICE_TABLE(acpi, usb_xhci_acpi_match);

static struct platform_driver rtk_usb_xhci_driver =
{
        .probe = xhci_plat_probe,
        .remove = xhci_plat_remove,
        .shutdown = usb_hcd_platform_shutdown,
        .driver = {
                .name = "rtk-xhci-hcd",
                .pm = &xhci_plat_pm_ops,
                .of_match_table = of_match_ptr(usb_xhci_of_match),
                .acpi_match_table = ACPI_PTR(usb_xhci_acpi_match),
        },
};
MODULE_ALIAS("platform:rtk-xhci-hcd");

#ifdef CONFIG_RTK_KDRV_XHCI_HCD_PLATFORM

static int __init xhci_platform_tool_obj_init(char *cmdline)
{
        tool_obj_en = 1;
        xhci_plat_info("%s Setting TOOL_OBJ:%d\n",__func__,tool_obj_en);
        return 0;
}
early_param("TOOL_OBJ", xhci_platform_tool_obj_init);
#else

int parse_xhci_platform_tool_obj_init(void)
{
        if(rtk_parse_commandline("TOOL_OBJ") == false)
        {
                xhci_plat_err("Error : can't get TOOL_OBJ from bootargs\n");
                return -1;
        }
        tool_obj_en = 1;
        xhci_plat_info("%s Setting TOOL_OBJ:%d\n",__func__,tool_obj_en);
        return 0;
}
#endif

#ifdef CONFIG_USER_INITCALL_USB
static int xhci_plat_init(void)
#else
static int __init xhci_plat_init(void)
#endif
{
#ifdef CONFIG_RTK_KDRV_XHCI_HCD_PLATFORM_MODULE
        parse_xhci_platform_tool_obj_init();
        parse_otg_switch_handler();
#endif
        xhci_init_driver(&xhci_plat_hc_driver, &xhci_plat_overrides);
        return platform_driver_register(&rtk_usb_xhci_driver);
}
#if defined(CONFIG_USER_INITCALL_USB) && !defined(MODULE)
user_initcall_grp("USB", xhci_plat_init);
#else
module_init(xhci_plat_init);
#endif

static void __exit xhci_plat_exit(void)
{
        platform_driver_unregister(&rtk_usb_xhci_driver);
}
module_exit(xhci_plat_exit);

MODULE_DESCRIPTION("xHCI Platform Host Controller Driver");
MODULE_LICENSE("GPL");
