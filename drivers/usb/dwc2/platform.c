// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/*
 * platform.c - DesignWare HS OTG Controller platform driver
 *
 * Copyright (C) Matthijs Kooijman <matthijs@stdin.nl>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the above-listed copyright holders may not be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/phy/phy.h>
#include <linux/platform_data/s3c-hsotg.h>
#include <linux/reset.h>

#include <linux/usb/of.h>
#ifdef CONFIG_DWC2_RTK
#include <rbus/sys_reg_reg.h>
#include <rtk_kdriver/io.h>
#include <rbus/usb2_otg_reg.h>
#endif
#include "core.h"
#include "hcd.h"
#include "debug.h"
static const char dwc2_driver_name[] = "dwc2";

#ifdef CONFIG_DWC2_RTK
static bool usbotg = 0;
static int usb_otg_switch_handler(char *buf)
{
	char *p = NULL;
	if ((p = strstr(buf, "on")) != NULL)
		usbotg = 1;
	return 0;
}
#endif

/*
 * Check the dr_mode against the module configuration and hardware
 * capabilities.
 *
 * The hardware, module, and dr_mode, can each be set to host, device,
 * or otg. Check that all these values are compatible and adjust the
 * value of dr_mode if possible.
 *
 *                      actual
 *    HW  MOD dr_mode   dr_mode
 *  ------------------------------
 *   HST  HST  any    :  HST
 *   HST  DEV  any    :  ---
 *   HST  OTG  any    :  HST
 *
 *   DEV  HST  any    :  ---
 *   DEV  DEV  any    :  DEV
 *   DEV  OTG  any    :  DEV
 *
 *   OTG  HST  any    :  HST
 *   OTG  DEV  any    :  DEV
 *   OTG  OTG  any    :  dr_mode
 */
static int dwc2_get_dr_mode(struct dwc2_hsotg *hsotg)
{
	enum usb_dr_mode mode;

	hsotg->dr_mode = usb_get_dr_mode(hsotg->dev);
	if (hsotg->dr_mode == USB_DR_MODE_UNKNOWN)
		hsotg->dr_mode = USB_DR_MODE_OTG;

	mode = hsotg->dr_mode;

	if (dwc2_hw_is_device(hsotg)) {
		if (IS_ENABLED(CONFIG_USB_DWC2_HOST)) {
			dev_err(hsotg->dev,
				"Controller does not support host mode.\n");
			return -EINVAL;
		}
		mode = USB_DR_MODE_PERIPHERAL;
	} else if (dwc2_hw_is_host(hsotg)) {
		if (IS_ENABLED(CONFIG_USB_DWC2_PERIPHERAL)) {
			dev_err(hsotg->dev,
				"Controller does not support device mode.\n");
			return -EINVAL;
		}
		mode = USB_DR_MODE_HOST;
	} else {
		if (IS_ENABLED(CONFIG_USB_DWC2_HOST))
			mode = USB_DR_MODE_HOST;
		else if (IS_ENABLED(CONFIG_USB_DWC2_PERIPHERAL))
			mode = USB_DR_MODE_PERIPHERAL;
	}

	if (mode != hsotg->dr_mode) {
		dev_warn(hsotg->dev,
			 "Configuration mismatch. dr_mode forced to %s\n",
			mode == USB_DR_MODE_HOST ? "host" : "device");

		hsotg->dr_mode = mode;
	}

	return 0;
}

static int __dwc2_lowlevel_hw_enable(struct dwc2_hsotg *hsotg)
{
	struct platform_device *pdev = to_platform_device(hsotg->dev);
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(hsotg->supplies),
				    hsotg->supplies);
	if (ret)
		return ret;

	if (hsotg->clk) {
		ret = clk_prepare_enable(hsotg->clk);
		if (ret)
			return ret;
	}

	if (hsotg->uphy) {
		ret = usb_phy_init(hsotg->uphy);
	} else if (hsotg->plat && hsotg->plat->phy_init) {
		ret = hsotg->plat->phy_init(pdev, hsotg->plat->phy_type);
	} else {
		ret = phy_power_on(hsotg->phy);
		if (ret == 0)
			ret = phy_init(hsotg->phy);
	}

	return ret;
}

/**
 * dwc2_lowlevel_hw_enable - enable platform lowlevel hw resources
 * @hsotg: The driver state
 *
 * A wrapper for platform code responsible for controlling
 * low-level USB platform resources (phy, clock, regulators)
 */
int dwc2_lowlevel_hw_enable(struct dwc2_hsotg *hsotg)
{
	int ret = __dwc2_lowlevel_hw_enable(hsotg);

	if (ret == 0)
		hsotg->ll_hw_enabled = true;
	return ret;
}

static int __dwc2_lowlevel_hw_disable(struct dwc2_hsotg *hsotg)
{
	struct platform_device *pdev = to_platform_device(hsotg->dev);
	int ret = 0;

	if (hsotg->uphy) {
		usb_phy_shutdown(hsotg->uphy);
	} else if (hsotg->plat && hsotg->plat->phy_exit) {
		ret = hsotg->plat->phy_exit(pdev, hsotg->plat->phy_type);
	} else {
		ret = phy_exit(hsotg->phy);
		if (ret == 0)
			ret = phy_power_off(hsotg->phy);
	}
	if (ret)
		return ret;

	if (hsotg->clk)
		clk_disable_unprepare(hsotg->clk);

	ret = regulator_bulk_disable(ARRAY_SIZE(hsotg->supplies),
				     hsotg->supplies);

	return ret;
}

/**
 * dwc2_lowlevel_hw_disable - disable platform lowlevel hw resources
 * @hsotg: The driver state
 *
 * A wrapper for platform code responsible for controlling
 * low-level USB platform resources (phy, clock, regulators)
 */
int dwc2_lowlevel_hw_disable(struct dwc2_hsotg *hsotg)
{
	int ret = __dwc2_lowlevel_hw_disable(hsotg);

	if (ret == 0)
		hsotg->ll_hw_enabled = false;
	return ret;
}

static int dwc2_lowlevel_hw_init(struct dwc2_hsotg *hsotg)
{
	int i, ret;

	hsotg->reset = devm_reset_control_get_optional(hsotg->dev, "dwc2");
	if (IS_ERR(hsotg->reset)) {
		ret = PTR_ERR(hsotg->reset);
		dev_err(hsotg->dev, "error getting reset control %d\n", ret);
		return ret;
	}

	reset_control_deassert(hsotg->reset);

	hsotg->reset_ecc = devm_reset_control_get_optional(hsotg->dev, "dwc2-ecc");
	if (IS_ERR(hsotg->reset_ecc)) {
		ret = PTR_ERR(hsotg->reset_ecc);
		dev_err(hsotg->dev, "error getting reset control for ecc %d\n", ret);
		return ret;
	}

	reset_control_deassert(hsotg->reset_ecc);

	/*
	 * Attempt to find a generic PHY, then look for an old style
	 * USB PHY and then fall back to pdata
	 */
	hsotg->phy = devm_phy_get(hsotg->dev, "usb2-phy");
	if (IS_ERR(hsotg->phy)) {
		ret = PTR_ERR(hsotg->phy);
		switch (ret) {
		case -ENODEV:
		case -ENOSYS:
			hsotg->phy = NULL;
			break;
		case -EPROBE_DEFER:
			return ret;
		default:
			dev_err(hsotg->dev, "error getting phy %d\n", ret);
			return ret;
		}
	}

	if (!hsotg->phy) {
		hsotg->uphy = devm_usb_get_phy(hsotg->dev, USB_PHY_TYPE_USB2);
		if (IS_ERR(hsotg->uphy)) {
			ret = PTR_ERR(hsotg->uphy);
			switch (ret) {
			case -ENODEV:
			case -ENXIO:
				hsotg->uphy = NULL;
				break;
			case -EPROBE_DEFER:
				return ret;
			default:
				dev_err(hsotg->dev, "error getting usb phy %d\n",
					ret);
				return ret;
			}
		}
	}

	hsotg->plat = dev_get_platdata(hsotg->dev);

	/* Clock */
	hsotg->clk = devm_clk_get_optional(hsotg->dev, "otg");
	if (IS_ERR(hsotg->clk)) {
		dev_err(hsotg->dev, "cannot get otg clock\n");
		return PTR_ERR(hsotg->clk);
	}

	/* Regulators */
	for (i = 0; i < ARRAY_SIZE(hsotg->supplies); i++)
		hsotg->supplies[i].supply = dwc2_hsotg_supply_names[i];

	ret = devm_regulator_bulk_get(hsotg->dev, ARRAY_SIZE(hsotg->supplies),
				      hsotg->supplies);
	if (ret) {
		dev_err(hsotg->dev, "failed to request supplies: %d\n", ret);
		return ret;
	}
	return 0;
}

/**
 * dwc2_driver_remove() - Called when the DWC_otg core is unregistered with the
 * DWC_otg driver
 *
 * @dev: Platform device
 *
 * This routine is called, for example, when the rmmod command is executed. The
 * device may or may not be electrically present. If it is present, the driver
 * stops device processing. Any resources used on behalf of this device are
 * freed.
 */
static int dwc2_driver_remove(struct platform_device *dev)
{
	struct dwc2_hsotg *hsotg = platform_get_drvdata(dev);

	dwc2_debugfs_exit(hsotg);
	if (hsotg->hcd_enabled)
		dwc2_hcd_remove(hsotg);
	if (hsotg->gadget_enabled)
		dwc2_hsotg_remove(hsotg);

	if (hsotg->ll_hw_enabled)
		dwc2_lowlevel_hw_disable(hsotg);

	reset_control_assert(hsotg->reset);
	reset_control_assert(hsotg->reset_ecc);

	return 0;
}

/**
 * dwc2_driver_shutdown() - Called on device shutdown
 *
 * @dev: Platform device
 *
 * In specific conditions (involving usb hubs) dwc2 devices can create a
 * lot of interrupts, even to the point of overwhelming devices running
 * at low frequencies. Some devices need to do special clock handling
 * at shutdown-time which may bring the system clock below the threshold
 * of being able to handle the dwc2 interrupts. Disabling dwc2-irqs
 * prevents reboots/poweroffs from getting stuck in such cases.
 */
static void dwc2_driver_shutdown(struct platform_device *dev)
{
	struct dwc2_hsotg *hsotg = platform_get_drvdata(dev);

	dwc2_disable_global_interrupts(hsotg);
	synchronize_irq(hsotg->irq);
}

/**
 * dwc2_check_core_endianness() - Returns true if core and AHB have
 * opposite endianness.
 * @hsotg:	Programming view of the DWC_otg controller.
 */
static bool dwc2_check_core_endianness(struct dwc2_hsotg *hsotg)
{
	u32 snpsid;

	snpsid = ioread32(hsotg->regs + GSNPSID);
	if ((snpsid & GSNPSID_ID_MASK) == DWC2_OTG_ID ||
	    (snpsid & GSNPSID_ID_MASK) == DWC2_FS_IOT_ID ||
	    (snpsid & GSNPSID_ID_MASK) == DWC2_HS_IOT_ID)
		return false;
	return true;
}
#ifdef CONFIG_DWC2_RTK
#define USB_PHY_DELAY                         120

bool rtk_otg_is_clk_en(void)
{
        if((rtd_inl(SYS_REG_SYS_CLKEN5_reg) & SYS_REG_SYS_CLKEN5_clken_otg_mask)
                && ((rtd_inl(SYS_REG_SYS_SRST5_reg) & (SYS_REG_SYS_SRST5_rstn_otg_mask |
                    SYS_REG_SYS_SRST5_rstn_usb_otg_wrapper_mask |
                    SYS_REG_SYS_SRST5_rstn_usb_otg_dwc_mask)) == (SYS_REG_SYS_SRST5_rstn_otg_mask |
                    SYS_REG_SYS_SRST5_rstn_usb_otg_wrapper_mask |
                    SYS_REG_SYS_SRST5_rstn_usb_otg_dwc_mask)))
                    return true;
        else
                return false;
}

static void rtk_otg_crt_on_off(bool on)
{
    if(on) {
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
                    SYS_REG_SYS_SRST5_rstn_otg_mask |
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

    }else {
            // clock off : MAC and OTG
            rtd_outl(SYS_REG_SYS_CLKEN5_reg,
                    SYS_REG_SYS_CLKEN5_clken_otg_mask |
                    SYS_REG_SYS_CLKEN5_write_data(0));

            // Reset MAC and OTG
            rtd_outl(SYS_REG_SYS_SRST5_reg,
                    SYS_REG_SYS_SRST5_rstn_otg_mask |
                    SYS_REG_SYS_SRST5_rstn_usb_otg_wrapper_mask |
                    SYS_REG_SYS_SRST5_rstn_usb_otg_dwc_mask |
                    SYS_REG_SYS_SRST5_write_data(0));
    }
    
}
static inline int rtk_otg_initial(void)
{

    rtk_otg_crt_on_off(true);

        //wdone eco option = 1, ahb_eco_option2 =1, ahb_eco_option1 =1
    rtd_outl(USB2_OTG_USB_CTR0_reg, 
                    (rtd_inl(USB2_OTG_USB_CTR0_reg) | USB2_OTG_USB_CTR0_ahb_eco_option(0x1) |
                    USB2_OTG_USB_CTR0_wdone_eco_option(0x1) |
                    USB2_OTG_USB_CTR0_ahb_eco_option2(0x1)));
        //otg enable
    rtd_outl(USB2_OTG_USB_OTG_REG_reg, 
                rtd_inl(USB2_OTG_USB_OTG_REG_reg) | USB2_OTG_USB_OTG_REG_otg_enable(0x1));
    
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
#endif
/**
 * dwc2_driver_probe() - Called when the DWC_otg core is bound to the DWC_otg
 * driver
 *
 * @dev: Platform device
 *
 * This routine creates the driver components required to control the device
 * (core, HCD, and PCD) and initializes the device. The driver components are
 * stored in a dwc2_hsotg structure. A reference to the dwc2_hsotg is saved
 * in the device private data. This allows the driver to access the dwc2_hsotg
 * structure on subsequent calls to driver methods for this device.
 */
static int dwc2_driver_probe(struct platform_device *dev)
{
	struct dwc2_hsotg *hsotg;
	struct resource *res;
	int retval;
#ifdef CONFIG_DWC2_RTK
	retval = rtk_otg_initial();
	if (retval)
	    return retval;
#endif

	hsotg = devm_kzalloc(&dev->dev, sizeof(*hsotg), GFP_KERNEL);
	if (!hsotg)
		return -ENOMEM;

	hsotg->dev = &dev->dev;

	/*
	 * Use reasonable defaults so platforms don't have to provide these.
	 */
	if (!dev->dev.dma_mask)
		dev->dev.dma_mask = &dev->dev.coherent_dma_mask;
	retval = dma_set_coherent_mask(&dev->dev, DMA_BIT_MASK(32));
	if (retval) {
		dev_err(&dev->dev, "can't set coherent DMA mask: %d\n", retval);
		return retval;
	}

	res = platform_get_resource(dev, IORESOURCE_MEM, 0);
	hsotg->regs = devm_ioremap_resource(&dev->dev, res);
	if (IS_ERR(hsotg->regs))
		return PTR_ERR(hsotg->regs);

	dev_dbg(&dev->dev, "mapped PA %08lx to VA %p\n",
		(unsigned long)res->start, hsotg->regs);

	retval = dwc2_lowlevel_hw_init(hsotg);
	if (retval)
		return retval;

	spin_lock_init(&hsotg->lock);

	hsotg->irq = platform_get_irq(dev, 0);
	if (hsotg->irq < 0)
		return hsotg->irq;
#ifdef CONFIG_DWC2_RTK_USING_IRQ
	dev_dbg(hsotg->dev, "registering common handler for irq%d\n",
		hsotg->irq);
	retval = devm_request_irq(hsotg->dev, hsotg->irq,
				  dwc2_handle_common_intr, IRQF_SHARED,
				  dev_name(hsotg->dev), hsotg);
	if (retval)
		return retval;
#endif
	hsotg->vbus_supply = devm_regulator_get_optional(hsotg->dev, "vbus");
	if (IS_ERR(hsotg->vbus_supply)) {
		retval = PTR_ERR(hsotg->vbus_supply);
		hsotg->vbus_supply = NULL;
		if (retval != -ENODEV)
			return retval;
	}

	retval = dwc2_lowlevel_hw_enable(hsotg);
	if (retval)
		return retval;

	hsotg->needs_byte_swap = dwc2_check_core_endianness(hsotg);

	retval = dwc2_get_dr_mode(hsotg);
	if (retval)
		goto error;

	hsotg->need_phy_for_wake =
		of_property_read_bool(dev->dev.of_node,
				      "snps,need-phy-for-wake");

	/*
	 * Reset before dwc2_get_hwparams() then it could get power-on real
	 * reset value form registers.
	 */
	retval = dwc2_core_reset(hsotg, false);
	if (retval)
		goto error;

	/* Detect config values from hardware */
	retval = dwc2_get_hwparams(hsotg);
	if (retval)
		goto error;

	/*
	 * For OTG cores, set the force mode bits to reflect the value
	 * of dr_mode. Force mode bits should not be touched at any
	 * other time after this.
	 */
	dwc2_force_dr_mode(hsotg);

	retval = dwc2_init_params(hsotg);
	if (retval)
		goto error;

	if (hsotg->dr_mode != USB_DR_MODE_HOST) {
		retval = dwc2_gadget_init(hsotg);
		if (retval)
			goto error;
		hsotg->gadget_enabled = 1;
	}

	/*
	 * If we need PHY for wakeup we must be wakeup capable.
	 * When we have a device that can wake without the PHY we
	 * can adjust this condition.
	 */
	if (hsotg->need_phy_for_wake)
		device_set_wakeup_capable(&dev->dev, true);

	hsotg->reset_phy_on_wake =
		of_property_read_bool(dev->dev.of_node,
				      "snps,reset-phy-on-wake");
	if (hsotg->reset_phy_on_wake && !hsotg->phy) {
		dev_warn(hsotg->dev,
			 "Quirk reset-phy-on-wake only supports generic PHYs\n");
		hsotg->reset_phy_on_wake = false;
	}

	if (hsotg->dr_mode != USB_DR_MODE_PERIPHERAL) {
		retval = dwc2_hcd_init(hsotg);
		if (retval) {
			if (hsotg->gadget_enabled)
				dwc2_hsotg_remove(hsotg);
			goto error;
		}
		hsotg->hcd_enabled = 1;
	}

	platform_set_drvdata(dev, hsotg);
	hsotg->hibernated = 0;

	dwc2_debugfs_init(hsotg);

	/* Gadget code manages lowlevel hw on its own */
	if (hsotg->dr_mode == USB_DR_MODE_PERIPHERAL)
		dwc2_lowlevel_hw_disable(hsotg);
	
#if IS_ENABLED(CONFIG_USB_DWC2_PERIPHERAL) || \
	IS_ENABLED(CONFIG_USB_DWC2_DUAL_ROLE)
	/* Postponed adding a new gadget to the udc class driver list */
	if (hsotg->gadget_enabled) {
		retval = usb_add_gadget_udc(hsotg->dev, &hsotg->gadget);
		if (retval) {
			hsotg->gadget.udc = NULL;
			dwc2_hsotg_remove(hsotg);
			goto error;
		}
	}
#endif /* CONFIG_USB_DWC2_PERIPHERAL || CONFIG_USB_DWC2_DUAL_ROLE */
#ifdef CONFIG_USB_USER_RESUME
	dev->dev.power.is_userresume = true;
        device_enable_async_suspend(&dev->dev);
#endif
	return 0;

error:
	if (hsotg->dr_mode != USB_DR_MODE_PERIPHERAL)
		dwc2_lowlevel_hw_disable(hsotg);
	return retval;
}

static int __maybe_unused dwc2_suspend(struct device *dev)
{
	struct dwc2_hsotg *dwc2 = dev_get_drvdata(dev);
	bool is_device_mode = dwc2_is_device_mode(dwc2);
	int ret = 0;

	if (is_device_mode)
		dwc2_hsotg_suspend(dwc2);

	if (dwc2->ll_hw_enabled &&
	    (is_device_mode || dwc2_host_can_poweroff_phy(dwc2))) {
		ret = __dwc2_lowlevel_hw_disable(dwc2);
		dwc2->phy_off_for_suspend = true;
	}
 #ifdef CONFIG_DWC2_RTK   
        rtk_otg_crt_on_off(false);
 #endif
	return ret;
}

#ifdef CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM
extern struct platform_device *get_ehci_platform_device(void);
#endif
static int __maybe_unused dwc2_resume(struct device *dev)
{
	struct dwc2_hsotg *dwc2 = dev_get_drvdata(dev);
	int ret = 0;
#ifdef CONFIG_DWC2_RTK
#ifdef CONFIG_RTK_KDRV_EHCI_HCD_PLATFORM
	struct platform_device *ehci_platform_dev = get_ehci_platform_device();

	if (ehci_platform_dev->dev.power.is_suspended) {
		device_pm_wait_for_dev(dev, &ehci_platform_dev->dev);
	}
#endif
	ret = rtk_otg_initial();
	if (ret)
	    return ret;
#endif

	if (dwc2->phy_off_for_suspend && dwc2->ll_hw_enabled) {
		ret = __dwc2_lowlevel_hw_enable(dwc2);
		if (ret)
			return ret;
	}
	dwc2->phy_off_for_suspend = false;

	if (dwc2_is_device_mode(dwc2))
		ret = dwc2_hsotg_resume(dwc2);

	return ret;
}

static const struct dev_pm_ops dwc2_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dwc2_suspend, dwc2_resume)
};

static struct platform_driver dwc2_platform_driver = {
	.driver = {
#ifdef CONFIG_DWC2_RTK
		.name ="rtk_dwc2",
#else
		.name = dwc2_driver_name,
#endif
		.of_match_table = dwc2_of_match_table,
		.pm = &dwc2_dev_pm_ops,
	},
	.probe = dwc2_driver_probe,
	.remove = dwc2_driver_remove,
	.shutdown = dwc2_driver_shutdown,
};
#ifdef CONFIG_DWC2_RTK
static __init rtkotg_init(void)
{
    int ret = 0;

#ifndef MODULE
	if (!usbotg) {
		pr_err("OTG is not enable. Use command: \"usbotg on\" in bootcode to enable it! \n");
		return -EPERM;
	}
#endif

    ret = platform_driver_probe(&dwc2_platform_driver, dwc2_driver_probe);
    if (ret < 0) {
		pr_err("RTK DWC2 HSOTG Driver install fail!!!\n");
    }

    return ret;
}

static void __exit rtkotg_exit(void)
{
    rtd_outl(USB2_OTG_USB_OTG_REG_reg, 
                rtd_inl(USB2_OTG_USB_OTG_REG_reg) & (~USB2_OTG_USB_OTG_REG_otg_enable(0x1)));
    
    platform_driver_unregister(&dwc2_platform_driver);
    pr_notice("Remove RTK DWC2 HSOTG Driver\n");
}

late_initcall(rtkotg_init);  // must wait for USB EHCI Platform
module_exit(rtkotg_exit);
#else
module_platform_driver(dwc2_platform_driver);
#endif

MODULE_DESCRIPTION("DESIGNWARE HS OTG Platform Glue");
MODULE_AUTHOR("Matthijs Kooijman <matthijs@stdin.nl>");
MODULE_LICENSE("Dual BSD/GPL");

#ifdef CONFIG_DWC2_RTK
__setup("usbotg", usb_otg_switch_handler);
#endif
