//Kernel Header file
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>		/* everything... */
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <rtk_kdriver/rtk_mcp.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <rtk_kdriver/io.h>
#include <generated/autoconf.h>


#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#define to_user_ptr(x)          compat_ptr((unsigned int)x)
#else
#define to_user_ptr(x)          ((void* __user)(x)) // convert 32 bit value to user pointer
#endif

#define HDCP_OPTEE_DEVICE_NAME "hdcp_optee"

#ifdef CONFIG_PM
static int hdcp_optee_suspend (struct device *p_dev)
{
	int result = 0;
	return result;
}

static int hdcp_optee_resume (struct device *p_dev)
{
	return 0;
}

#endif


#ifdef CONFIG_PM

static const struct dev_pm_ops hdcp_optee_pm_ops =
{
	.suspend    = hdcp_optee_suspend,
	.resume     = hdcp_optee_resume,
#ifdef CONFIG_HIBERNATION
	.freeze     = hdcp_optee_suspend,
	.thaw       = hdcp_optee_resume,
	.poweroff   = hdcp_optee_suspend,
	.restore    = hdcp_optee_resume,
#endif
};
#endif // CONFIG_PM

static struct platform_device *hdcp_optee_platform_devs = NULL;
static struct platform_driver hdcp_optee_device_driver = {
    .driver = {
        .name       = HDCP_OPTEE_DEVICE_NAME,
        .bus        = &platform_bus_type,
#ifdef CONFIG_PM
	.pm           = &hdcp_optee_pm_ops,
#endif
    },
} ;

int hdcp_optee_init_module(void)
{
	int result;

	pr_debug("\n\n\n\n *****************  hdcp2 init module  *********************\n\n\n\n");



	hdcp_optee_platform_devs = platform_device_register_simple(HDCP_OPTEE_DEVICE_NAME, -1, NULL, 0);

    	if((result=platform_driver_register(&hdcp_optee_device_driver)) != 0){
		pr_debug("scalehdcp2: can not register platform driver...\n");
		result = -ENOMEM;
		goto fail_platform_driver_register;
    	}


    	return 0;	//success

fail_platform_driver_register:
	platform_device_unregister(hdcp_optee_platform_devs);
	hdcp_optee_platform_devs = NULL;
	return result;
}



void __exit hdcp_optee_cleanup_module(void)
{
   	/* device driver removal */
	if(hdcp_optee_platform_devs) {
		platform_device_unregister(hdcp_optee_platform_devs);
		hdcp_optee_platform_devs = NULL;
	}
  	platform_driver_unregister(&hdcp_optee_device_driver);
}

module_init(hdcp_optee_init_module);
module_exit(hdcp_optee_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
