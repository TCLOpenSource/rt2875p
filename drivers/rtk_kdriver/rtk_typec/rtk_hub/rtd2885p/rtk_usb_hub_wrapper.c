#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of_platform.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/freezer.h>
#include <mach/pcbMgr.h>
#include <linux/uaccess.h>
#include <rtk_kdriver/io.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/usb2_hub_reg.h>
#include <linux/suspend.h>
#include <linux/notifier.h>

void rtk_usb_hub_wrapper_clock_on(void)
{
    // To do wrapper CLK
    rtd_outl(SYS_REG_SYS_SRST4_reg, SYS_REG_SYS_SRST4_rstn_usb_hub_mask);
    rtd_outl(SYS_REG_SYS_CLKEN4_reg, SYS_REG_SYS_CLKEN4_clken_usb_hub_mask);
    rtd_outl(SYS_REG_SYS_SRST4_reg, SYS_REG_SYS_SRST4_rstn_usb_hub_wrap_mask);
    rtd_outl(SYS_REG_SYS_CLKEN4_reg, SYS_REG_SYS_CLKEN4_clken_usb_hub_wrap_mask);
    udelay(100);
    rtd_outl(SYS_REG_SYS_SRST4_reg, SYS_REG_SYS_SRST4_rstn_usb_hub_wrap_mask |
								SYS_REG_SYS_SRST4_write_data_mask);
    udelay(100);
    rtd_outl(SYS_REG_SYS_CLKEN4_reg, SYS_REG_SYS_CLKEN4_clken_usb_hub_wrap_mask |
								SYS_REG_SYS_CLKEN4_write_data_mask);
    udelay(100);
}


void rtk_usb_hub_clk_on(void)
{
    // 1. Set usb_hub_reset
    rtd_outl(SYS_REG_SYS_SRST4_reg, SYS_REG_SYS_SRST4_rstn_usb_hub_mask |
    					SYS_REG_SYS_SRST4_write_data_mask);
    mdelay(50);
    // 2. Set usb_hub_clk
    rtd_outl(SYS_REG_SYS_CLKEN4_reg, SYS_REG_SYS_CLKEN4_clken_usb_hub_mask |
    				SYS_REG_SYS_CLKEN4_write_data_mask);
}

bool rtk_usb_hub_load_phy_settings(void)
{
    //do nothing
    return true;
}


void rtk_usb_hub_wrapper_clock_off(void)
{
    // To do wrapper CLK
    rtd_outl(SYS_REG_SYS_SRST4_reg, SYS_REG_SYS_SRST4_rstn_usb_hub_mask);
    rtd_outl(SYS_REG_SYS_CLKEN4_reg, SYS_REG_SYS_CLKEN4_clken_usb_hub_mask);
    rtd_outl(SYS_REG_SYS_SRST4_reg, SYS_REG_SYS_SRST4_rstn_usb_hub_wrap_mask);
    rtd_outl(SYS_REG_SYS_CLKEN4_reg, SYS_REG_SYS_CLKEN4_clken_usb_hub_wrap_mask);
}


void rtk_usb_hub_usp_enable(bool enable)
{
	if(enable)
		rtd_outl(SYS_REG_SYS_CLKDIV_reg, rtd_inl(SYS_REG_SYS_CLKDIV_reg)
								| SYS_REG_SYS_CLKDIV_usb_pc_dphy_mux_p2_mask);
	else
		rtd_outl(SYS_REG_SYS_CLKDIV_reg, rtd_inl(SYS_REG_SYS_CLKDIV_reg)
								& (~SYS_REG_SYS_CLKDIV_usb_pc_dphy_mux_p2_mask));
}
EXPORT_SYMBOL(rtk_usb_hub_usp_enable);

void rtk_usb_hub_specail_dsp_enable(unsigned char port, bool enable)
{
	switch(port) {
	case 0:
		if(enable)
			rtd_outl(SYS_REG_SYS_CLKDIV_reg, rtd_inl(SYS_REG_SYS_CLKDIV_reg) |
								SYS_REG_SYS_CLKDIV_usb_pc_dphy_mux_p0_mask);
		else
			rtd_outl(SYS_REG_SYS_CLKDIV_reg, rtd_inl(SYS_REG_SYS_CLKDIV_reg) &
								(~SYS_REG_SYS_CLKDIV_usb_pc_dphy_mux_p0_mask));		
		break;
	case 1:
		if(enable)
			rtd_outl(SYS_REG_SYS_CLKDIV_reg, rtd_inl(SYS_REG_SYS_CLKDIV_reg) |
								SYS_REG_SYS_CLKDIV_usb_pc_dphy_mux_p1_mask);
		else
			rtd_outl(SYS_REG_SYS_CLKDIV_reg, rtd_inl(SYS_REG_SYS_CLKDIV_reg) &
								(~SYS_REG_SYS_CLKDIV_usb_pc_dphy_mux_p1_mask));		
		break;
	default:
		break;
	}
}

void rtk_usb_hub_dsp_enable(bool enable)
{
	rtk_usb_hub_specail_dsp_enable(0, enable);
	rtk_usb_hub_specail_dsp_enable(1, enable);
}
EXPORT_SYMBOL(rtk_usb_hub_dsp_enable);

