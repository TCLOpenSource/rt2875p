#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <mach/platform.h>
#include <mach/rtk_platform.h>
#include "rtk_led_interface.h"

#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
#include <mach/pcbMgr.h>
#endif


static struct led_controller rtk_led;
static struct mutex  rtk_led_mutex;

struct workqueue_struct *led_wq = NULL;

led_job job;

extern platform_info_t platform_info;

MODULE_LICENSE("Dual BSD/GPL");

#define LED_MAJOR  0
#define DRV_NAME   "RTKLED"
static dev_t rtk_led_dev_t;
static const char  drv_name[] = DRV_NAME;
static int led_type=0;
//static int amp_boot_sel = 0;
//static int amp_boot_addr = 0;

#ifdef CONFIG_PM
static int led_suspend(struct device *dev);
static int led_resume(struct device *dev);

static struct platform_device *led_devs;

static const struct dev_pm_ops led_pm_ops = {
        .suspend    = led_suspend,
        .resume     = led_resume,
};
#endif

static struct platform_driver led_driver = {
        .driver = {
                .name         = (char *)drv_name,
                .bus          = &platform_bus_type,
#ifdef CONFIG_PM
                .pm           = &led_pm_ops,
#endif
        },
};

void rtk_led_ops_set(int sel)
{
    if (sel==0) {
    	rtk_led.ops = led_func_3206;
    	rtk_led.detect = led_detect_3206;
    	rtk_led.init = led_init_io_3206;
    	rtk_led.dump_all = led_dump_all;
    } else {
        rtk_led.ops = led_func_3238;
        rtk_led.detect = led_detect_3238;
        rtk_led.init = led_init_io_3238;
        rtk_led.dump_all = led_dump_all;
    }
}

int rtk_led_i2c_id(void)
{
    return rtk_led.led_i2c_id;
}

int led_get_i2c_id(void)
{
    unsigned long long param;

    if (pcb_mgr_get_enum_info_byname("TUNER_I2C", &param) != 0)
    {
        LED_ERR("%s line:%d bootcode pcb info lose to define  \"LED_I2C\"  \n",__func__, __LINE__ );
        return -1;
    }

    LED_INFO("%s line:%d get led I2c id = %lld  \n",__func__, __LINE__  , param );

    return param;
}

int led_get_addr_3206(void)
{
    unsigned long long param;
/*
    if (pcb_mgr_get_enum_info_byname("AMP_DEVICE_ADDR", &param) != 0)
    {
        LED_WARN("%s line:%d bootcode pcb info lose to define  \"AMP_DEVICE_ADDR\"  \n",__func__, __LINE__ );
        return -1;
    }

    LED_INFO("%s line:%d get amp slave address(8bit) = %llx  \n",__func__, __LINE__  , param );
*/
    param =0xd8;

    return (param >> 1);
}

int led_get_addr_3238(void)
{
    unsigned long long param;
/*
    if (pcb_mgr_get_enum_info_byname("AMP_DEVICE_ADDR", &param) != 0)
    {
        LED_WARN("%s line:%d bootcode pcb info lose to define  \"AMP_DEVICE_ADDR\"  \n",__func__, __LINE__ );
        return -1;
    }

    LED_INFO("%s line:%d get amp slave address(8bit) = %llx  \n",__func__, __LINE__  , param );
*/
    param =0x68;

    return (param >> 1);
}

static int led_suspend(struct device *dev)
{
    LED_INFO("[LED] led_notify_suspend\n");
    flush_workqueue(led_wq);
    return 0;
}

static int led_resume(struct device *dev)
{
    LED_INFO("[LED] led_notify_resume\n");
    return 0;
}

static int rtk_led_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int rtk_led_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t __attribute((unused)) rtk_led_read(struct file *filp, char __user *buffer, size_t count, loff_t *f_ops)
{
    return count;
}

static ssize_t __attribute((unused)) rtk_led_write(struct file *filp, const char __user *buffer, size_t count, loff_t *f_ops)
{
    return count;
}


static long rtk_led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    LED_COLOR_INPUT color_input;

    memset(&color_input, 0, sizeof(color_input));

    mutex_lock(&rtk_led_mutex);

    switch (cmd)
    {
    case RTK_LED_IOC_SET_COLOR:
        LED_INFO("ioctl: RTK_LED_IOC_SET_COLOR \n");
        if (copy_from_user((void *)&color_input, (const void __user *)arg, sizeof(color_input)))
        {
            LED_WARN("ioctl: RTK_LED_IOC_SET_COLOR , copy data from user space failed\n");
            ret = -EFAULT;
            break;
        }

        rtk_led.ops(rtk_led.led_i2c_id,rtk_led.slave_addr,color_input.colors,color_input.count);
        break;

    case RTK_LED_IOC_DETECT:
        LED_INFO("ioctl: RTK_LED_IOC_DETECT \n");
        ret = rtk_led.detect(rtk_led.led_i2c_id,rtk_led.slave_addr);
        if (copy_to_user((void __user *)arg,(void *)&ret, sizeof(ret)))
        {
            LED_WARN("ioctl: RTK_LED_IOC_DETECT , copy data to user space failed\n");
            ret = -EFAULT;
            break;
        }

        break;

    case RTK_LED_IOC_INIT:
        ret = rtk_led.init(rtk_led.led_i2c_id,rtk_led.slave_addr);
        break;

    default:
        LED_WARN("Unknow commnad \n");
        break;
    }

    mutex_unlock(&rtk_led_mutex);

    return ret;
}

static long rtk_led_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    return 0;
}

struct file_operations rtk_led_fops = {
    .owner            = THIS_MODULE,
    .open             = rtk_led_open,
    .release          = rtk_led_release,
#if 0
    .read             = rtk_led_read,
    .write            = rtk_led_write,
#endif
    .unlocked_ioctl   = rtk_led_ioctl,
    .compat_ioctl     = rtk_led_compat_ioctl,
};

static struct class *led_class = NULL;
static struct cdev led_cdev;
static struct device *led_device_this;
static int major = -1;

static int rtk_led_init(void) {
    int result = 0;


    LED_INFO("LED: LED init for Realtek LED\n");

    rtk_led.led_i2c_id = led_get_i2c_id();
    if(rtk_led.led_i2c_id == (-1))
    {
        LED_ERR("%s line:%d led i2c id failed \n",__func__, __LINE__);
        return -1;
    }

    rtk_led.sel_index = ACL1310;
    rtk_led.slave_addr = led_get_addr_3206();
    if(rtk_led.slave_addr == (-1))
    {
        LED_ERR("%s line:%d led slave address failed \n",__func__, __LINE__);
        return -1;
    }
	
	rtk_led_ops_set(0);
	
	LED_INFO("\n LED I2C Port_Id2  =  %d Slave_addr(7bit):%x\n", rtk_led.led_i2c_id,  rtk_led.slave_addr);
	
	if (rtk_led.detect(rtk_led.led_i2c_id,rtk_led.slave_addr) > 0) {
		LED_ERR("%s line:%d detect led 3206 \n",__func__, __LINE__);
		led_type=3206;
		rtk_led_ops_set(0);
	} else {
		LED_ERR("%s line:%d not detect led 3206 \n",__func__, __LINE__);
		rtk_led.slave_addr = led_get_addr_3238();

	        rtk_led_ops_set(1);
		
		if (rtk_led.detect(rtk_led.led_i2c_id,rtk_led.slave_addr) > 0) {
                	LED_ERR("%s line:%d led detect led 3238\n",__func__, __LINE__);
			led_type=3238;
        	} else {
			LED_ERR("%s line:%d not detect led 3238\n",__func__, __LINE__);
			return 0;
			
		}
	}
		
	if ((result=alloc_chrdev_region(&rtk_led_dev_t, 0, 1,"rtk_led")) < 0) {
	    LED_ERR("LED: can not get chrdev region...\n");
            return result;
    }

	cdev_init(&led_cdev, &rtk_led_fops);
	
	if (cdev_add(&led_cdev, rtk_led_dev_t, 1) == -1)
    {
        LED_ERR("LED: can not add character device...\n");
        goto fail_cdev_init;
    }

	if ((led_class = class_create(THIS_MODULE, "rtk_led_class")) == NULL)
	    goto fail_class_create;

    led_devs = platform_device_register_simple("RTKLED", -1, NULL, 0);

    if (platform_driver_register(&led_driver) != 0)
    {
        LED_ERR("LED: can not register platform driver...\n");
        result = -EINVAL;
        goto fail_platform_driver_register;
    }

    led_wq = create_singlethread_workqueue("LED workqueue");
    if (NULL == led_wq)
    {
        LED_ERR("%s line:%d can not create led workqueue \n",__func__, __LINE__);
        result = -EINVAL;
        goto fail_platform_driver_register;
    }
    else
    {
        LED_WARN("create led workqueue success \n");
    }

    mutex_init(&rtk_led_mutex);
    INIT_WORK(&(job.led_work), _led_work);


    if ((led_device_this = device_create(led_class, NULL, led_cdev.dev, NULL, "rtk_led")) == NULL)
	{
        LED_ERR("LED: can not create device...\n");
	}

	if (rtk_led.detect(rtk_led.led_i2c_id,rtk_led.slave_addr) > 0) {
		LED_ERR("%s line:%d led detect and then init flow \n",__func__, __LINE__);
		rtk_led.init(rtk_led.led_i2c_id,rtk_led.slave_addr);
	}
		

#ifdef CONFIG_RTK_AMP_USER_RESUME
        //led_devs->dev.power.is_userresume = true;
        //device_enable_async_suspend(&(led_devs->dev));
#endif

        return 0;

fail_cdev_init:
        platform_driver_unregister(&led_driver);
fail_platform_driver_register:
        platform_device_unregister(led_devs);
        led_devs = NULL;
        class_destroy(led_class);
fail_class_create:
        led_class = NULL;
        unregister_chrdev_region(major, 1);
        return result;

}

static void rtk_led_exit(void) {
    dev_t dev = MKDEV(LED_MAJOR, 0);

    if ((led_devs == NULL) || (led_class == NULL))
        BUG();

    if(led_wq != NULL)
    {
        flush_workqueue(led_wq);
        destroy_workqueue(led_wq);
        led_wq = NULL;
    }
    device_destroy(led_class, dev);
    cdev_del(&led_cdev);

    platform_driver_unregister(&led_driver);
    platform_device_unregister(led_devs);
    led_devs = NULL;

    class_destroy(led_class);
    led_class = NULL;

    unregister_chrdev_region(dev, 1);
}

module_init(rtk_led_init);
module_exit(rtk_led_exit);


