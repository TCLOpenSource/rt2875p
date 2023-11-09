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
#include <linux/suspend.h>
#include <linux/notifier.h>

void UsbBillboard_CRT_ON_OFF(bool on)
{
	if(on) {
		rtd_outl(SYS_REG_SYS_CLKEN5_reg, SYS_REG_SYS_CLKEN5_clken_usb_bb_mask);
		udelay(10);
		rtd_outl(SYS_REG_SYS_SRST5_reg, SYS_REG_SYS_SRST5_rstn_usb_bb_mask);
		udelay(10);
		rtd_outl(SYS_REG_SYS_CLKEN5_reg, SYS_REG_SYS_CLKEN5_clken_usb_bb_mask | SYS_REG_SYS_CLKEN5_write_data_mask);
		udelay(10);
		rtd_outl(SYS_REG_SYS_SRST5_reg, SYS_REG_SYS_SRST5_rstn_usb_bb_mask | SYS_REG_SYS_SRST5_write_data_mask);
		udelay(100);
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN5_reg, SYS_REG_SYS_CLKEN5_clken_usb_bb_mask);
		udelay(10);
		rtd_outl(SYS_REG_SYS_SRST5_reg, SYS_REG_SYS_SRST5_rstn_usb_bb_mask);
	}
	
}

bool is_UsbBillboard_CRT_ON(void)
{
	if((rtd_inl(SYS_REG_SYS_CLKEN5_reg) & SYS_REG_SYS_CLKEN5_clken_usb_bb_mask)
		&& (rtd_inl(SYS_REG_SYS_SRST5_reg) & SYS_REG_SYS_SRST5_rstn_usb_bb_mask))
		return true;
	return false;
}

