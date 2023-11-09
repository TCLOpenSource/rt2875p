/*Kernel Header file*/
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>		/* everything... */
#include <linux/cdev.h>
#include <linux/platform_device.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pageremap.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/hrtimer.h>
#include <linux/vmalloc.h>

/*RBUS Header file*/
#include <scaler/scalerCommon.h>


/*TVScaler Header file*/
#include "tvscalercontrol/io/ioregdrv.h"
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv001.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv010.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/viptable.h>
#include <tvscalercontrol/scaler_vpqmemcdev.h>
#include "scaler_vpqdev_extern.h"
#include "scaler_vpqdev_extern_tv010.h"
#include "scaler_vscdev.h"
#include "ioctrl/vpq/vpq_extern_cmd_id.h"
#include "ioctrl/vpq/vpq_externsky_cmd_id.h"
#include <tvscalercontrol/vip/pq_rpc.h>
//#include <mach/RPCDriver.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/gibi_od.h>

#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scalerdrv/scalerip.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
//TODO, Merlin3 tv010 sync, need modify
//#include <rbus/rbus_DesignSpec_MISC_GPIOReg.h>
#include <rtd_log/rtd_module_log.h>


#define TAG_NAME "VPQEX"

static dev_t vpqexsky_devno;/*vpq device number*/
static struct cdev vpqexsky_cdev;
static unsigned char PQ_Dev_ExternSky_Status = PQ_DEV_EXTERN_NOTHING;


int vpqexsky_open(struct inode *inode, struct file *filp)
{
	printk("vpqex_open %d\n", __LINE__);
	return 0;
}

ssize_t  vpqexsky_read(struct file *filep, char *buffer, size_t count, loff_t *offp)
{
	return 0;
}

ssize_t vpqexsky_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{
	return 0;
}

int vpqexsky_release(struct inode *inode, struct file *filep)
{
	printk("vpqex_release %d\n", __LINE__);
	return 0;
}

#ifdef CONFIG_PM
static int vpqexsky_suspend(struct device *p_dev)
{
	return 0;
}

static int vpqexsky_resume(struct device *p_dev)
{
	return 0;
}
#endif

void vpqexsky_boot_init(void)
{

}


long vpqexsky_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	int ret = 0;

//	if (vpqex_ioctl_get_stop_run(cmd))
//		return 0;

//	printk("vpqexsky_ioctl %d\n", __LINE__);

	if (VPQ_EXTERNSKY_IOC_INIT == cmd) {
		//init project
		//fwif_VIP_set_Project_ID(VIP_Project_ID_TV010);
		Scaler_color_Set_HDR_AutoRun(TRUE);
		return 0;
	} else if (VPQ_EXTERNSKY_IOC_UNINIT == cmd) {
		PQ_Dev_ExternSky_Status = PQ_DEV_EXTERN_UNINIT;
		return 0;
	} else if (VPQ_EXTERNSKY_IOC_OPEN == cmd || VPQ_EXTERNSKY_IOC_CLOSE == cmd) {
		return 0;
	}

//	if (PQ_Dev_Extern_Status != PQ_DEV_EXTERN_INIT_DONE)
//		return -1;

//	printk("vpqex_ioctl %d\n", __LINE__);
#ifdef Merlin3_rock_mark	//Merlin3 rock mark
	if (fwif_color_get_pq_demo_flag_rpc())
		return 0;
#endif


	switch (cmd) {
	/*
	case VPQ_EXTERNSKY_IOC_INIT:
		if (PQ_DEV_EXTERN_UNINIT == PQ_Dev_Extern_Status || PQ_DEV_EXTERN_NOTHING == PQ_Dev_Extern_Status) {
			fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 1);
			fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 1);
			PQ_Dev_Extern_Status = PQ_DEV_EXTERN_INIT_DONE;
		} else
			return -1;
		break;

	case VPQ_EXTERNSKY_IOC_UNINIT:
		if (PQ_DEV_EXTERN_CLOSE == PQ_Dev_Extern_Status || PQ_DEV_EXTERN_INIT_DONE == PQ_Dev_Extern_Status) {
			PQ_Dev_Extern_Status = PQ_DEV_EXTERN_UNINIT;
		} else
			return -1;
		break;

	case VPQ_EXTERNSKY_IOC_OPEN:
		if (PQ_DEV_EXTERN_CLOSE == PQ_Dev_Extern_Status || PQ_DEV_EXTERN_INIT_DONE == PQ_Dev_Extern_Status) {
			PQ_Dev_Extern_Status = PQ_DEV_EXTERN_OPEN_DONE;
		} else
			return -1;
		break;

	case VPQ_EXTERNSKY_IOC_CLOSE:
		if (PQ_DEV_EXTERN_OPEN_DONE == PQ_Dev_Extern_Status) {
			PQ_Dev_Extern_Status = PQ_DEV_EXTERN_CLOSE;
		} else
			 return -1;
		break;
	*/

/************************************************************************
 *  TV010 External variables
 ************************************************************************/

		case VPQ_EXTERNSKY_IOC_GET_DE_RINGING:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_DE_RINGING\n");
			TV001_LEVEL_E drLevel = 0;

			if (Scaler_GetDeRinging_level(&drLevel) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetDeRinging_level fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&drLevel, sizeof(TV001_LEVEL_E))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_DE_RINGING fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERNSKY_IOC_SET_DE_RINGING:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_DE_RINGING\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetDeRinging_level((TV001_LEVEL_E)args);
			}
		}
		break;

		case VPQ_EXTERNSKY_IOC_GET_COLOR_GAIN:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_COLOR_GAIN\n");
			TV001_LEVEL_E colorGain = 0;

			if (Scaler_GetColorGainLevel(&colorGain) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetColorGainLevel fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&colorGain, sizeof(TV001_LEVEL_E))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_COLOR_GAIN fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERNSKY_IOC_SET_COLOR_GAIN_EXTERN:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_COLOR_GAIN_EXTERN\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetColorGainLevel(args);
			}
		}
		break;
		case VPQ_EXTERNSKY_IOC_GET_FLESH_TONE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_FLESH_TONE\n");
			TV001_LEVEL_E fleshtone = 0;

			if (Scaler_GetFleshToneLevel(&fleshtone) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetFleshToneLevel fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&fleshtone, sizeof(TV001_LEVEL_E))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_FLESH_TONE fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERNSKY_IOC_SET_FLESH_TONE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_FLESH_TONE\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetFleshToneLevel((TV001_LEVEL_E)args);
			}
		}
		break;

		case VPQ_EXTERNSKY_IOC_GET_GAMMA_NUM:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_GAMMA_NUM\n");
			unsigned int number = 0;

			if (Scaler_GetGammaNum(&number) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetGammaNum fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&number, sizeof(unsigned int))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_GAMMA_NUM fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERNSKY_IOC_SET_GAMMA:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_GAMMA\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetGAMMAMapping_tv010((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERNSKY_IOC_GET_PQ_MODULE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_PQ_MODULE\n");
			RTK_PQModule_T PQModule_T;
			if (copy_from_user(&PQModule_T,  (int __user *)arg,sizeof(RTK_PQModule_T))) {

				ret = -1;
			} else {
				ret = 0;
				if (Scaler_GetPQModule((TV010_PQ_MODULE_E) PQModule_T.PQModule,(unsigned char *) &PQModule_T.onOff) == FALSE) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetPQModule fail\n");
					ret = -1;
				} else {
					if (copy_to_user((void __user *)arg, (void *)&PQModule_T, sizeof(RTK_PQModule_T))) {
						rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_PQ_MODULE fail\n");
						ret = -1;
					} else
						ret = 0;
				}
			}
		}
		break;
		case VPQ_EXTERNSKY_IOC_SET_PQ_MODULE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_PQ_MODULE\n");
			RTK_PQModule_T PQModule_T;
			//unsigned int args = 0;
			if (copy_from_user(&PQModule_T,  (int __user *)arg,sizeof(RTK_PQModule_T))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetPQModule((TV010_PQ_MODULE_E) PQModule_T.PQModule,(unsigned char)PQModule_T.onOff);
			}
		}
		break;
		case VPQ_EXTERNSKY_IOC_SET_DEMO:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_DEMO\n");
			RTK_DEMO_MODE_T DEMO_MODE_T;
			//unsigned int args = 0;
			if (copy_from_user(&DEMO_MODE_T,  (int __user *)arg,sizeof(RTK_DEMO_MODE_T))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetDemoMode((TV010_DEMO_MODE_E) DEMO_MODE_T.DEMO_MODE,(unsigned char)DEMO_MODE_T.onOff);
			}
		}
		break;
		case VPQ_EXTERNSKY_IOC_GET_SR_LEVEL:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_SR_LEVEL\n");
			TV010_DEMOLEVEL_E srLevel = 0;

			if (Scaler_GetSRLevel(&srLevel) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetSRLevel fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&srLevel, sizeof(TV001_LEVEL_E))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_SR_LEVEL fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERNSKY_IOC_SET_SR_LEVEL:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_SR_LEVEL\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetSRLevel((TV010_DEMOLEVEL_E)args);
			}
		}
		break;

		case VPQ_EXTERNSKY_IOC_GET_OD_LEVEL:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_OD_LEVEL\n");
			unsigned int level = 0;

			if (Scaler_GetODLevel(&level) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetODLevel fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&level, sizeof(unsigned int))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_OD_LEVEL fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERNSKY_IOC_SET_OD_READ_ENABLE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_OD_READ_ENABLE\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetODReadEnable((unsigned char)args);
			}
		}
		break;

		case VPQ_EXTERNSKY_IOC_GET_NOTIFY_PLATFORM_COLOT_TEMP:
		{
		       unsigned char mode;
			 if(copy_from_user(&mode,(void __user *)arg,sizeof(unsigned char)))
			 {
			  	rtd_printk(KERN_EMERG, TAG_NAME,"error: VPQ_EXTERN_IOC_GET_NOTIFY_PLATFORM_COLOT_TEMP \n");
                       	ret = -1;
                      }else{
                      	rtd_printk(KERN_INFO, TAG_NAME," VPQ_EXTERN_IOC_GET_NOTIFY_PLATFORM_COLOT_TEMP mode=%d \n",mode);
				Scaler_SetCOLORTEMPMapping_tv010(mode);
                            ret = 0;
                    }
              }
               break;


	default:
		rtd_printk(KERN_DEBUG, TAG_NAME, "kernel IO command %d not found!\n", cmd);
		printk("VPQ_EXTERN_IOC cmd=0x%x unknown\n", cmd);
		return -1;
			;
	}
	return ret;/*Success*/

}

struct file_operations vpqexsky_fops = {
	.owner = THIS_MODULE,
	.open = vpqexsky_open,
	.release = vpqexsky_release,
	.read  = vpqexsky_read,
	.write = vpqexsky_write,
	.unlocked_ioctl = vpqexsky_ioctl,
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
	.compat_ioctl = vpqexsky_ioctl,
#endif
#endif
};

#ifdef CONFIG_PM
static const struct dev_pm_ops vpqexsky_pm_ops =
{
        .suspend    = vpqexsky_suspend,
        .resume     = vpqexsky_resume,
#ifdef CONFIG_HIBERNATION
	.freeze 	= vpqexsky_suspend,
	.thaw		= vpqexsky_resume,
	.poweroff	= vpqexsky_suspend,
	.restore	= vpqexsky_resume,
#endif

};
#endif // CONFIG_PM

static struct class *vpqexsky_class = NULL;
static struct platform_device *vpqexsky_platform_devs = NULL;
static struct platform_driver vpqexsky_platform_driver = {
	.driver = {
		.name = VPQ_EXTERNSKY_DEVICE_NAME,
		.bus = &platform_bus_type,
#ifdef CONFIG_PM
		.pm = &vpqexsky_pm_ops,
#endif
    },
} ;

static char *vpqexsky_devnode(struct device *dev, umode_t *mode)
{
	return NULL;
}

int vpqexsky_major   = 0;
int vpqexsky_minor   = 0 ;
int vpqexsky_nr_devs = 1;

int vpqexsky_module_init(void)
{
	int result;
	//dev_t devno = 0;//vbe device number
	printk("vpqex_module_init %d\n", __LINE__);

	result = alloc_chrdev_region(&vpqexsky_devno, vpqexsky_minor, vpqexsky_nr_devs, VPQ_EXTERNSKY_DEVICE_NAME);
	vpqexsky_major = MAJOR(vpqexsky_devno);
	if (result != 0) {
		rtd_printk(KERN_ERR, TAG_NAME, "Cannot allocate VPQEX device number\n");
		printk("vpqex_module_init %d\n", __LINE__);
		return result;
	}

	printk("vpqex_module_init %d\n", __LINE__);

	printk("VPQEX_DEVICE init module major number = %d\n", vpqexsky_major);
	//vpqex_devno = MKDEV(vpqex_major, vpqex_minor);

	vpqexsky_class = class_create(THIS_MODULE, VPQ_EXTERNSKY_DEVICE_NAME);
	if (IS_ERR(vpqexsky_class)) {
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqex: can not create class...\n");
		printk("vpqex_module_init %d\n", __LINE__);
		result = PTR_ERR(vpqexsky_class);
		goto fail_class_create;
	}

	vpqexsky_class->devnode = vpqexsky_devnode;

	vpqexsky_platform_devs = platform_device_register_simple(VPQ_EXTERNSKY_DEVICE_NAME, -1, NULL, 0);
	if (platform_driver_register(&vpqexsky_platform_driver) != 0) {
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqex: can not register platform driver...\n");
		printk("vpqex_module_init %d\n", __LINE__);
		result = -ENOMEM;
		goto fail_platform_driver_register;
	}

	cdev_init(&vpqexsky_cdev, &vpqexsky_fops);
	vpqexsky_cdev.owner = THIS_MODULE;
   	vpqexsky_cdev.ops = &vpqexsky_fops;
	result = cdev_add(&vpqexsky_cdev, vpqexsky_devno, 1);
	if (result < 0) {
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqex: can not add character device...\n");
		printk("vpqex_module_init %d\n", __LINE__);
		goto fail_cdev_init;
	}

	device_create(vpqexsky_class, NULL, vpqexsky_devno, NULL, VPQ_EXTERNSKY_DEVICE_NAME);


	vpqexsky_boot_init();

	printk("vpqex_module_init %d\n", __LINE__);
	return 0;/*Success*/

fail_cdev_init:
	platform_driver_unregister(&vpqexsky_platform_driver);
fail_platform_driver_register:
	platform_device_unregister(vpqexsky_platform_devs);
	vpqexsky_platform_devs = NULL;
	class_destroy(vpqexsky_class);
fail_class_create:
	vpqexsky_class = NULL;
	unregister_chrdev_region(vpqexsky_devno, 1);
	printk("vpqex_module_init %d\n", __LINE__);
	return result;

}



void __exit vpqexsky_module_exit(void)
{
	printk("vpqex_module_exit\n");

	if (vpqexsky_platform_devs == NULL)
		BUG();

	device_destroy(vpqexsky_class, vpqexsky_devno);
	cdev_del(&vpqexsky_cdev);

	platform_driver_unregister(&vpqexsky_platform_driver);
	platform_device_unregister(vpqexsky_platform_devs);
	vpqexsky_platform_devs = NULL;

	class_destroy(vpqexsky_class);
	vpqexsky_class = NULL;

	unregister_chrdev_region(vpqexsky_devno, 1);
}

module_init(vpqexsky_module_init);
module_exit(vpqexsky_module_exit);

