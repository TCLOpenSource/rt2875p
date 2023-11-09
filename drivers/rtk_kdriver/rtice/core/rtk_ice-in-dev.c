#include <rtd_log/rtd_module_log.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include "rtk_ice.h"
#include "rtk_ice-aux.h"
 static struct kfifo rx_fifo;
#if IS_ENABLED(CONFIG_RTK_KDRV_SERIAL_8250)	
extern void rtk_serial8250_set_rtice_port(int port);
extern int rtk_serial8250_get_rtice_port(void);
#endif

/*-----------------------------------------------------------------------
 * File Operations
 *----------------------------------------------------------------------*/
int rtice_dev_open(struct inode *inode, struct file *file)
{
	return 0;
}

int rtice_dev_release(struct inode *inode, struct file *file)
{
	return 0;
}

ssize_t rtice_dev_read(struct file * file, char __user * data, size_t len,
			  loff_t * ofst)
{
	int out_len = 0;
	int ret = kfifo_to_user(&rx_fifo, data, len, &out_len);
	return (ret == 0) ? out_len : ret;
}

ssize_t rtice_dev_write(struct file * file, const char __user * data,
			   size_t len, loff_t * ofst)
{
	unsigned char buff[256];
	if (len > sizeof(buff))
		return -1;
	if (copy_from_user(buff, data, len))
		return -1;
	kfifo_reset(&rx_fifo);
	rtice_input(buff, len);
	return 0;
}

static struct file_operations rtice_dev_fops = \
{ 
	.owner = THIS_MODULE, 
	.read = rtice_dev_read, 
	.write = rtice_dev_write, 
	.open = rtice_dev_open, 
	.release = rtice_dev_release,
};

ssize_t rtice_show_param(struct device *dev,struct device_attribute*attr,\
								char *buf)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_SERIAL_8250)	
	rtd_pr_rtice_info("[RTICE-%s]\n", __func__);
	if (strncmp(attr->attr.name, "rtice_uartnumber", 16) == 0)
		sprintf(buf, "%d\n", rtk_serial8250_get_rtice_port());
#endif	
	return 1;
}

ssize_t rtice_set_param(struct device * dev, struct device_attribute * attr,\
			    const char *buf, size_t count)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_SERIAL_8250)	
	int iPortNumber = 0;
	if (strncmp(attr->attr.name, "rtice_uartnumber", 16) == 0) {
		if (sscanf(buf, "%d\n", &iPortNumber) == 1) {
			if (iPortNumber >= 0 && iPortNumber <= 3) {
				rtd_pr_rtice_info("[RTICE-%s]Chang Rtice port to %d\n",\
					__func__, iPortNumber);
				rtk_serial8250_set_rtice_port(iPortNumber);
			}
		}
	}
#endif	
	return 1;
}

DEVICE_ATTR(rtice_uartnumber, S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP, rtice_show_param,
		rtice_set_param);
static struct miscdevice rtice_dev_miscdev = \
{ 
	MISC_DYNAMIC_MINOR, 
	"rtice", 
	&rtice_dev_fops
};

int __init rtice_dev_module_init(void)
{
	int ret = 0;

	rtice_init();

	if (kfifo_alloc(&rx_fifo, 1024, GFP_KERNEL) < 0)
		return -ENOMEM;
	if (misc_register(&rtice_dev_miscdev)){
		rtd_pr_rtice_info("rtice_dev_module_init failed\n");
		ret = -ENODEV;
	}
	device_create_file(rtice_dev_miscdev.this_device,\
					&dev_attr_rtice_uartnumber);

#if IS_ENABLED(CONFIG_RTK_KDRV_RTICE_SYSTEM_TOOL)
	rtice_system_tool_init();
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_RTICE_IOMEM_TOOL)
	rtice_iomem_tool_init();
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_RTICE_AP_TOOL)
	rtice_ap_tool_init();
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_RTICE_MC_MEASURE)
	rtice_mc_tool_init();
	rtice_latency_tool_init();
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_RTICE_I2C_TOOL)
	rtice_i2c_tool_init();
#endif
	return ret;
}

static void __exit rtice_dev_module_exit(void)
{
	misc_deregister(&rtice_dev_miscdev);

#if IS_ENABLED(CONFIG_RTK_KDRV_RTICE_SYSTEM_TOOL)
	rtice_system_tool_exit();
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_RTICE_IOMEM_TOOL)
	rtice_iomem_tool_exit();
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_RTICE_AP_TOOL)
	rtice_ap_tool_exit();
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_RTICE_MC_MEASURE)
	rtice_mc_tool_exit();
	rtice_latency_tool_exit();
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_RTICE_I2C_TOOL)
	rtice_i2c_tool_exit();
#endif

} 

module_init(rtice_dev_module_init);
module_exit(rtice_dev_module_exit);
MODULE_AUTHOR("Kevin Wang, Realtek Semiconductor");
MODULE_LICENSE("GPL");
