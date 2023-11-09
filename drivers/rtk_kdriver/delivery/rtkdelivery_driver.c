/******************************************************************************
 *
 *   Realtek Delivery driver
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author lciou@realtek.com
 *
 *****************************************************************************/

#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/string.h>
#include "rtkdelivery.h"
#include "rtkdelivery_export.h"
#include "rtkdelivery_debug.h"
#include <rbus/pll_reg_reg.h>
#include <rbus/timer_reg.h>
#include <rtk_kdriver/io.h>
#include <mach/rtk_platform.h>

#define DRV_NAME    "rtkdelivery"

#ifdef CONFIG_PM
static int DELIVERY_suspend(struct device *dev);
static int DELIVERY_resume(struct device *dev);
#ifdef CONFIG_HIBERNATION
static int DELIVERY_restore(struct device *dev);
#endif
#endif

static int delivery_major = RTKDELIVERY_MAJOR;
static int delivery_minor;

module_param(delivery_major, int, S_IRUGO);
module_param(delivery_minor, int, S_IRUGO);

static const char drv_name[] = DRV_NAME;
delivery_dev *delivery_device;      /* allocated in delivery_init_module */

static struct class *delivery_class;
static struct platform_device *delivery_platform_devs;

#ifdef CONFIG_PM
static const struct dev_pm_ops delivery_pm_ops =
{
	.suspend    = DELIVERY_suspend,
	.resume     = DELIVERY_resume,
#ifdef CONFIG_HIBERNATION
	.freeze     = DELIVERY_suspend,
	.thaw       = DELIVERY_restore,
	.poweroff   = DELIVERY_suspend,
	.restore    = DELIVERY_restore,
#endif
};
#endif // CONFIG_PM

static struct platform_driver  delivery_device_driver =
{
	.driver = {
		.name       = (char *)drv_name,
		.bus        = &platform_bus_type,
		#ifdef CONFIG_PM
		.pm 	  	= & delivery_pm_ops,
		#endif
	},
};

#ifdef CONFIG_PM
static int DELIVERY_suspend(struct device *dev)
{
	delivery_warn(CHANNEL_UNKNOWN, "delivery suspend/freeze \n");
	return 0;
}

static int DELIVERY_resume(struct device *dev)
{
	delivery_warn(CHANNEL_UNKNOWN, "delivery resume \n");
	IOCTL_Delivery_SaveBootStatus();
	return 0;
}

#ifdef CONFIG_HIBERNATION
static int DELIVERY_restore(struct device *dev)
{
	delivery_warn(CHANNEL_UNKNOWN, "delivery restore \n");
	IOCTL_Delivery_STDResetInfoPoll();
	return 0;
}
#endif

#endif

int DELIVERY_open(struct inode *inode, struct file *filp)
{
	delivery_warn(CHANNEL_UNKNOWN,"func %s, line %d, NOT implemented!!, fp= %p\n", __func__, __LINE__, filp);
	return 0;
}

int DELIVERY_release(struct inode *inode, struct file *filp)
{
	delivery_function_call(CHANNEL_UNKNOWN);

	if (filp == delivery_device->fpInit) {
		if (down_interruptible(&delivery_device->sem))
			return -ERESTARTSYS;

		IOCTL_Delivery_UnInitModule();

		up(&delivery_device->sem);
	}

	return 0;
}
unsigned int DELIVERY_poll(struct file *filp, poll_table *wait)
{
	return 0;
}

#ifdef CONFIG_COMPAT
long DELIVERY_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *compat_arg = compat_ptr(arg);
    return DELIVERY_ioctl(filp, cmd, (unsigned long)compat_arg);
}
#endif

long DELIVERY_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = -ENOTTY;

	if (_IOC_TYPE(cmd) != DELIVERY_IOC_MAGIC || _IOC_NR(cmd) > DELIVERY_IOC_MAXNR)
		return -ENOTTY;

	if (down_interruptible(&delivery_device->sem))
		return -ERESTARTSYS;

	switch (cmd) {

	case DELIVERY_IOC_DELIVERY_INIT:
	{
		ret = IOCTL_Delivery_INIT();
		break;
	}
	case DELIVERY_IOC_DELIVERY_UNINIT:
	{
		ret = IOCTL_Delivery_UNINIT();
		break;
	}

	case DELIVERY_IOC_DELIVERY_OPEN:
	{
		DELIVERY_DELIVERY_CHANNEL_T data;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_CHANNEL_T)))	{
			ret = -EFAULT;
			delivery_err(data.ch, "RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_OPEN failed !!!!!!!!!!!!!!!\n", __LINE__);
		} else
			ret = IOCTL_Delivery_Open(&data);
		break;
	}
	case DELIVERY_IOC_DELIVERY_CLOSE:
	{
		DELIVERY_DELIVERY_CHANNEL_T data;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_CHANNEL_T)))	{
			ret = -EFAULT;
			delivery_err(data.ch, "RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_CLOSE failed !!!!!!!!!!!!!!!\n", __LINE__);
		} else
			ret = IOCTL_Delivery_Close(&data);

		break;
	}
	case DELIVERY_IOC_DELIVERY_SET_TP_INPUT_CONFIG:
	{
		DELIVERY_TP_SOURCE_CONFIG_T data;
		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_TP_SOURCE_CONFIG_T))) {
			ret = -EFAULT;
			delivery_err(data.ch, "RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_SET_TP_INPUT_CONFIG failed !!!!!!!!!!!!!!!\n", __LINE__);
		} else
			ret = IOCTL_Delivery_SetTPInputConfig(&data);
		break;
	}
	case DELIVERY_IOC_DELIVERY_REQUEST_DATA:
	{
		DELIVERY_DELIVERY_CHANNEL_T data;

		if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_CHANNEL_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_REQUEST_DATA failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		} else
			ret = IOCTL_Delivery_RequestData(&data);

		if (copy_to_user((void __user *)arg, (void *)&data, sizeof(DELIVERY_DELIVERY_CHANNEL_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_REQUEST_DATA failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}
		break;
	}
	case DELIVERY_IOC_DELIVERY_CANCEL_DATA:
	{
		DELIVERY_DELIVERY_CHANNEL_T data;

		if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_CHANNEL_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_CANCEL_DATA failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		} else
			ret = IOCTL_Delivery_CancelData(&data);

		if (copy_to_user((void __user *)arg, (void *)&data, sizeof(DELIVERY_DELIVERY_CHANNEL_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_CANCEL_DATA failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}
		break;
	}
	case DELIVERY_IOC_DELIVERY_GET_DATA:
	{
#ifdef CONFIG_COMPAT
		DELIVERY_DELIVERY_READ_DATA_COMPAT_T cmpat_data;
		if(copy_from_user((void *)&cmpat_data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_READ_DATA_COMPAT_T)))
		{
			delivery_err(cmpat_data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_GET_DATA failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}
		else
		{
			DELIVERY_DELIVERY_READ_DATA_T data;
			data.ch = cmpat_data.ch;
			data.data_size = cmpat_data.data_size;
			data.data = compat_ptr(cmpat_data.data);
			ret = IOCTL_Delivery_GetData(&data);
			cmpat_data.data_size = data.data_size;

		}

		if (copy_to_user((void __user *)arg, (void *)&cmpat_data, sizeof(DELIVERY_DELIVERY_READ_DATA_COMPAT_T)))
		{
			delivery_err(cmpat_data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_GET_DATA failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}
#else
		DELIVERY_DELIVERY_READ_DATA_T data;
		if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_READ_DATA_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_GET_DATA failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}
		else
		{
			ret = IOCTL_Delivery_GetData(&data);
		}

		if (copy_to_user((void __user *)arg, (void *)&data, sizeof(DELIVERY_DELIVERY_READ_DATA_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_GET_DATA failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}
#endif
		break;
	}
#if defined(CONFIG_RTK_KDRV_ATSC3_DMX)    
	case DELIVERY_IOC_DELIVERY_REQUEST_BBFRAME:
	{
		DELIVERY_DELIVERY_CHANNEL_T data;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_CHANNEL_T)))	{
			ret = -EFAULT;
			delivery_err(data.ch, "RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_REQUEST_BBFRAME failed !!!!!!!!!!!!!!!\n", __LINE__);
		} else
			ret = IOCTL_Delivery_RequestBBFrame(&data);
		break;
	}
	case DELIVERY_IOC_DELIVERY_CANCEL_BBFRAME:
	{
		DELIVERY_DELIVERY_CHANNEL_T data;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_CHANNEL_T)))	{
			ret = -EFAULT;
			delivery_err(data.ch, "RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_CANCEL_BBFRAME failed !!!!!!!!!!!!!!!\n", __LINE__);
		} else
			ret = IOCTL_Delivery_CancelBBFrame(&data);
		break;
	}
	case DELIVERY_IOC_DELIVERY_GET_BBFRAME_BUFFER:
	{
		DELIVERY_DELIVERY_GET_BBF_BUFFER_T data;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_GET_BBF_BUFFER_T)))
		{
			ret = -EFAULT;
			delivery_err(data.ch, "RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_GET_BBFRAME_BUFFER failed !!!!!!!!!!!!!!!\n", __LINE__);
		} else
			ret = IOCTL_Delivery_GetBBFrameBuffer(&data);

		if (copy_to_user((void __user *)arg, (void *)&data, sizeof(DELIVERY_DELIVERY_GET_BBF_BUFFER_T)))
		{
			delivery_err(data.ch, "RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_GET_BBFRAME_BUFFER failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}

		break;
	}
	case DELIVERY_IOC_DELIVERY_RETURN_BBFRAME_BUFFER:
	{
		DELIVERY_DELIVERY_GET_BBF_BUFFER_T data;

		if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_GET_BBF_BUFFER_T)))
		{
			ret = -EFAULT;
			delivery_err(data.ch, "RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_RETURN_BBFRAME_BUFFER failed !!!!!!!!!!!!!!!\n", __LINE__);
		} else
		    ret = IOCTL_Delivery_ReturnFrameBuffer(&data);

		break;
	}
	case DELIVERY_IOC_DELIVERY_GET_BBFRAME_BUFF_INFO:
	{
		DELIVERY_DELIVERY_BBF_BUFFER_INFO_T data;

		if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_BBF_BUFFER_INFO_T)))
		{
			delivery_err(data.ch, "RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_GET_BBFRAME_BUFF_INFO failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		} else
			ret = IOCTL_Delivery_GetBBFrameBufferInfo(&data);

		if (copy_to_user((void __user *)arg, (void *)&data, sizeof(DELIVERY_DELIVERY_BBF_BUFFER_INFO_T)))
		{
			delivery_err(data.ch, "RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_GET_BBFRAME_BUFF_INFO failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}

		break;
	}
	case DELIVERY_IOC_DELIVERY_GET_CURRENT_TIME_INFO:
	{
		DELIVERY_DELIVERY_TIME_INFO_T data;

		if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_TIME_INFO_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_GET_CURRENT_TIME_INFO failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		} else
			ret = IOCTL_Delivery_GetTimeInfo(&data);

		if (copy_to_user((void __user *)arg, (void *)&data, sizeof(DELIVERY_DELIVERY_TIME_INFO_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_GET_CURRENT_TIME_INFO failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}

		break;
	}
	case DELIVERY_IOC_DELIVERY_SET_CURRENT_TIME_INFO:
	{
		DELIVERY_DELIVERY_TIME_INFO_T data;

		if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_TIME_INFO_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_SET_CURRENT_TIME_INFO failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		} else
			ret = IOCTL_Delivery_SetTimeInfo(&data);

		if (copy_to_user((void __user *)arg, (void *)&data, sizeof(DELIVERY_DELIVERY_TIME_INFO_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_SET_CURRENT_TIME_INFO failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}
		
		break;
	}
	case DELIVERY_IOC_DELIVERY_SET_SYSTEM_TIME_INFO:
	{
		DELIVERY_DELIVERY_SYSTEM_TIME_INFO_T data;

		if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_SYSTEM_TIME_INFO_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_SET_SYSTEM_TIME_INFO failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		} else
			ret = IOCTL_Delivery_SetSystemTimeInfo(&data);

		if (copy_to_user((void __user *)arg, (void *)&data, sizeof(DELIVERY_DELIVERY_SYSTEM_TIME_INFO_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_SET_SYSTEM_TIME_INFO failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}

		break;
	}
	case DELIVERY_IOC_DELIVERY_SET_CLOCK_RECOVERY:
	{
		DELIVERY_DELIVERY_CLOCK_RECOVERY_T data;

		if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_CLOCK_RECOVERY_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_SET_CLOCK_RECOVERY failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		} else
			ret = IOCTL_Delivery_SetClockRecovery(&data);

		if (copy_to_user((void __user *)arg, (void *)&data, sizeof(DELIVERY_DELIVERY_CLOCK_RECOVERY_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_SET_CLOCK_RECOVERY failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}

		break;
	}
#endif
	case DELIVERY_IOC_DELIVERY_SET_MODE:
	{
		DELIVERY_DELIVERY_SET_MODE_T data;

		if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_SET_MODE_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_SET_MODE failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		} else
			ret = IOCTL_Delivery_SetMode(&data);

		if (copy_to_user((void __user *)arg, (void *)&data, sizeof(DELIVERY_DELIVERY_SET_MODE_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_SET_MODE failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}
		break;
	}
#if defined(CONFIG_RTK_KDRV_JP4K)
	case DELIVERY_IOC_DELIVERY_FILTER_EANBLE:
	{
		DELIVERY_DELIVERY_FILTER_ENABLE_T data;

		if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_FILTER_ENABLE_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_FILTER_EANBLE failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		} else {
			ret = IOCTL_Delivery_EnableTLVFilter(&data);
		}
		break;
	}
	case DELIVERY_IOC_DELIVERY_ADD_TLV_FILTERS:
	{
#ifdef CONFIG_COMPAT
		DELIVERY_DELIVERY_TLV_FILTER_INFO_COMPAT_T compat_param;
		if(copy_from_user((void *)&compat_param, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_TLV_FILTER_INFO_COMPAT_T))){
			delivery_err(compat_param.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_ADD_TLV_FILTERS failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}else{
			DELIVERY_DELIVERY_TLV_FILTER_INFO_T tlv_param;
			tlv_param.ch = compat_param.ch;
			tlv_param.cnt = compat_param.cnt;
			tlv_param.pfiltet_info_list = compat_ptr(compat_param.pfiltet_info_list);
			ret = IOCTL_Delivery_AddTLVFilters(&tlv_param);
		}
#else
		DELIVERY_DELIVERY_TLV_FILTER_INFO_T tlv_param;
		if(copy_from_user((void *)&tlv_param, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_TLV_FILTER_INFO_T))){
			delivery_err(tlv_param.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_ADD_TLV_FILTERS failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}else{
			ret = IOCTL_Delivery_AddTLVFilters(&tlv_param);
		}
#endif
		break;
	}
	case DELIVERY_IOC_DELIVERY_REMOVE_TLV_FILTERS:
	{
#ifdef CONFIG_COMPAT
		DELIVERY_DELIVERY_TLV_FILTER_INFO_COMPAT_T compat_param;
		if(copy_from_user((void *)&compat_param, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_TLV_FILTER_INFO_COMPAT_T))){
			delivery_err(compat_param.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_REMOVE_TLV_FILTERS failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}else{
			DELIVERY_DELIVERY_TLV_FILTER_INFO_T tlv_param;
			tlv_param.ch = compat_param.ch;
			tlv_param.cnt = compat_param.cnt;
			tlv_param.pfiltet_info_list = compat_ptr(compat_param.pfiltet_info_list);
			ret = IOCTL_Delivery_RemoveTLVFilters(&tlv_param);
		}
#else
		DELIVERY_DELIVERY_TLV_FILTER_INFO_T tlv_param;
		if(copy_from_user((void *)&tlv_param, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_TLV_FILTER_INFO_T))){
			delivery_err(tlv_param.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_REMOVE_TLV_FILTERS failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}else{
			ret = IOCTL_Delivery_RemoveTLVFilters(&tlv_param);
		}
#endif
		break;
	}
	case DELIVERY_IOC_DELIVERY_GET_NTP:
	{
		DELIVERY_DELIVERY_TLV_NTP_T data;

		if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_DELIVERY_TLV_NTP_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_DELIVERY_TLV_NTP_T failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		} else
			ret = IOCTL_Delivery_GetNTP(&data);

		if (copy_to_user((void __user *)arg, (void *)&data, sizeof(DELIVERY_DELIVERY_TLV_NTP_T)))
		{
			delivery_err(data.ch,"RTKDELIVERY[%d]: ioctl DELIVERY_DELIVERY_TLV_NTP_T failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}
		break;
	}
	case DELIVERY_IOC_DELIVERY_ADJUST_STC:
	{
		DELIVERY_ADJUST_STC_T data;

		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(DELIVERY_ADJUST_STC_T)))	{
			ret = -EFAULT;
			delivery_err(data.clock_id, "RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_ADJUST_STC failed !!!!!!\n", __LINE__);
		} else
			ret = IOCTL_Delivery_AdjustStc(&data);
		break;
	}

#endif
	case DELIVERY_IOC_DELIVERY_GET_BOOT_STATUS:
	{
		DELIVERY_DELIVERY_GET_BOOT_STATUS_T data;
		ret = IOCTL_Delivery_GetBootStatus(&data);

		if (copy_to_user((void __user *)arg, (void *)&data, sizeof(DELIVERY_DELIVERY_GET_BOOT_STATUS_T)))
		{
			delivery_err(CHANNEL_UNKNOWN, "RTKDELIVERY[%d]: ioctl DELIVERY_IOC_DELIVERY_GET_BOOT_STATUS failed !!!!!!!!!!!!!!!\n", __LINE__);
			ret = -EFAULT;
		}
		break;
	}
	default:
		delivery_warn(CHANNEL_UNKNOWN, "RTKDELIVERY: unknown ioctl(0x%08x)\n", cmd);
		break;
	}
	up(&delivery_device->sem);
	return ret;
}


struct file_operations delivery_fops = {
	.owner              = THIS_MODULE,
	.unlocked_ioctl     = DELIVERY_ioctl,
	.open               = DELIVERY_open,
	.release            = DELIVERY_release,
	.poll               = DELIVERY_poll,
#ifdef CONFIG_COMPAT
	.compat_ioctl       = DELIVERY_compat_ioctl,
#endif
};

static char *DELIVERY_devnode(struct device *dev, umode_t *mode)
{
	return NULL;
}

void DELIVERY_exit_module(void)
{
	dev_t dev;
	delivery_uninit_debug_proc();

	dev = MKDEV(delivery_major, delivery_minor);

	if (delivery_platform_devs == NULL)
		BUG();

	IOCTL_Delivery_UnInitModule();

	device_destroy(delivery_class, dev);
	cdev_del(&delivery_device->cdev);
	platform_driver_unregister(&delivery_device_driver);
	platform_device_unregister(delivery_platform_devs);
	kfree(delivery_device);
	delivery_platform_devs = NULL;
	class_destroy(delivery_class);
	delivery_class = NULL;
	unregister_chrdev_region(dev, 1);
}

int DELIVERY_init_module(void)
{
	int result;
	dev_t dev = 0;
	delivery_dev *delivery = NULL;

	delivery_warn(CHANNEL_UNKNOWN,"\n\n\n\n *****************    delivery init module  *********************\n\n\n\n");
	if (get_product_type() == PRODUCT_TYPE_DIAS) {
		delivery_err(CHANNEL_UNKNOWN,"delivery not support on dias platform\n");
		return 0;
	}

	if (delivery_major) {
		dev = MKDEV(RTKDELIVERY_MAJOR, 0);
		result = register_chrdev_region(dev, 1, DRV_NAME);
	} else {
		result = alloc_chrdev_region(&dev, delivery_minor, 1, DRV_NAME);
		delivery_major = MAJOR(dev);
	}
	if (result < 0) {
		delivery_warn(CHANNEL_UNKNOWN,"delivery: can not get chrdev region...\n");
		return result;
	}

	delivery_class = class_create(THIS_MODULE, DRV_NAME);
	if (IS_ERR(delivery_class)) {
		delivery_warn(CHANNEL_UNKNOWN,"delivery: can not create class...\n");
		result = PTR_ERR(delivery_class);
		goto fail_class_create;
	}
	delivery_class->devnode = DELIVERY_devnode;
	delivery_device = kmalloc(sizeof(delivery_dev), GFP_KERNEL);
	if (!delivery_device) {
		DELIVERY_exit_module();
		return -ENOMEM;
	}

	delivery_platform_devs = platform_device_register_simple(DRV_NAME, -1, NULL, 0);
	if (platform_driver_register(&delivery_device_driver) != 0) {
		delivery_warn(CHANNEL_UNKNOWN,"delivery: can not register platform driver...\n");
		result = -EINVAL;
		goto fail_platform_driver_register;
	}

	memset(delivery_device, 0, sizeof(delivery_dev));
	delivery = delivery_device;
	cdev_init(&delivery->cdev, &delivery_fops);
	delivery->cdev.owner = THIS_MODULE;
	delivery->cdev.ops   = &delivery_fops;
	if (cdev_add(&delivery->cdev, dev, 1) < 0) {
		delivery_warn(CHANNEL_UNKNOWN,"delivery: can not add character device...\n");
		goto fail_cdev_init;
	}
	device_create(delivery_class, NULL, dev, NULL, DRV_NAME);

	sema_init(&delivery->sem, 1);

	delivery_init_debug_proc();
	if(IOCTL_Delivery_InitModule() ==-1)
		delivery_warn(CHANNEL_UNKNOWN,"delivery: delivery init fail!! \n");
	return 0;

fail_cdev_init:
	platform_driver_unregister(&delivery_device_driver);
fail_platform_driver_register:
	platform_device_unregister(delivery_platform_devs);
	delivery_platform_devs = NULL;
	class_destroy(delivery_class);
fail_class_create:
	delivery_class = NULL;
	unregister_chrdev_region(dev, 1);
	return result;
}

module_init(DELIVERY_init_module);
module_exit(DELIVERY_exit_module);


MODULE_AUTHOR("bonds.lu <bonds.lu@realtek.com>");
MODULE_DESCRIPTION("Realtek Delivery Driver");
MODULE_ALIAS("platform:rtk-delivery");
MODULE_LICENSE("GPL");
