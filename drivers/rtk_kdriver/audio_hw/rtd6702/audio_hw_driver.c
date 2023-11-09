#include <linux/kconfig.h>
#include <linux/compiler.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>		/* module */
#include <linux/stat.h>			/* permission */
#include <linux/fs.h>			/* fs */
#include <linux/errno.h>		/* error codes */
#include <linux/types.h>		/* size_t */
#include <linux/fcntl.h>		/* O_ACCMODE */
#include <linux/uaccess.h>		/* copy_*_user */
#include <asm/io.h>			/* io_* */
#include <linux/mutex.h>		/* mutex */
#include <linux/device.h>		/* device */
#include <linux/platform_device.h>	/* platform device */
#include <linux/cdev.h>			/* char device */
#include <linux/pm.h>			/* power management */

#include "audio_hw_port.h"
#include "audio_hw_atv.h"
#include "audio_hw_aio.h"
#include "audio_hw_app.h"
#include "audio_hw_pm.h"
#include "audio_hw_driver.h"
#include "audio_hw_ioctl.h"
#include "audio_hw_factory.h"

#define __ALOG_SUBTAG "driver"

static int audio_hw_open(struct inode *inode, struct file *filp);
static int audio_hw_release(struct inode *inode, struct file *filp);

struct audio_hw_drvdata *audio_driver_data = NULL;

static const char *audio_hw_cmd_str[] = {
	"dumpsif=",			/* AUDIOHW_DBG_CMD_DUMP_SIF */
	"earc_debug=",		/* AUDIOHW_DBG_CMD_EARC_DEBUG_MODE */
	//"to be continued="
};

int audio_hw_open(struct inode *inode, struct file *filp)
{
	struct audio_hw_drvdata *drvdata;

	alog_info("open\n");

	drvdata = container_of(inode->i_cdev, struct audio_hw_drvdata, cdev);
	filp->private_data = drvdata;

	return 0;
}

int audio_hw_release(struct inode *inode, struct file *filp)
{
	alog_info("release\n");

	filp->private_data = NULL;

	return 0;
}

int audio_hw_dbg_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char str[AUDIO_HW_MAX_CMD_LENGTH] = {0};
	int cmd_index = 0;
	char *cmd_pointer = NULL;
 
	alog_info("dbg write\n"); 

	if (buf == NULL) {
		alog_err("dbg_write:%d buf=%p\n", __LINE__, buf);
		return -EFAULT;
	}
	if (count > AUDIO_HW_MAX_CMD_LENGTH - 1)
		count = AUDIO_HW_MAX_CMD_LENGTH - 1;

	if (copy_from_user(str, buf, AUDIO_HW_MAX_CMD_LENGTH - 1)) {
		//alog_info("dbg_write:%d copy_from_user failed! (buf=%p, count=%u)\n",
			//__LINE__, buf, count);
		return -EFAULT;
	}

	if (count > 0)
		str[count-1] = '\0';

	for (cmd_index = 0; cmd_index < AUDIO_HW_DBG_CMD_NUMBER; cmd_index++) {
		if (strncmp(str,
				audio_hw_cmd_str[cmd_index],
				strlen(audio_hw_cmd_str[cmd_index])) == 0)
			break;
	}

	if (cmd_index >= AUDIO_HW_DBG_CMD_NUMBER)
	{
		alog_err("[AudioHW] cmd_index error\n");
		return -EFAULT;
	}
	
	alog_info("cmd_index = %d\n", cmd_index);
	if (cmd_index < AUDIO_HW_DBG_CMD_NUMBER) {
		cmd_pointer = &str[strlen(audio_hw_cmd_str[cmd_index])];
	}

	if (cmd_pointer == NULL)
	{
		alog_err("[AudioHW] cmd_pointer error\n");
		return -EFAULT;
	}

	#if !(defined(RTD2874))
	audio_hw_dbg_EXECUTE(cmd_index, &cmd_pointer);
	#endif
	return count;
}

#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
long audio_hw_compat_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	long ret = 0;
	ret=audio_hw_ioctl(file,cmd,arg);
	return ret;
}
#endif
#endif

static struct file_operations audio_hw_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.unlocked_ioctl = audio_hw_ioctl,
	.open		= audio_hw_open,
	.release	= audio_hw_release,
	.write		= (void*)audio_hw_dbg_write,
	#ifdef CONFIG_ARM64
	#ifdef CONFIG_COMPAT
      	.compat_ioctl = audio_hw_compat_ioctl,
	#endif
	#endif
	
};

static char *audio_hw_devnode(struct device *dev, umode_t *mode)
{
	*mode = 0666;
	return NULL;
}

static int audio_hw_pdrv_probe(struct platform_device *pdev)
{
	int ret = 0;

	struct audio_hw_drvdata *drvdata = NULL;

	/*
	 * register character device with dynamic major and minor number.
	 *
	 * get a range of minor numbers to work with, asking for a dynamic
	 * major unless directed otherwise at load time.
	 */
	alog_info("platfom driver probe\n");

	/*
	 * allocate driver data
	 */
	drvdata = devm_kzalloc(&pdev->dev, sizeof(*drvdata), GFP_KERNEL);
	if (!drvdata) {
		alog_err("can't allocate drvdata\n");
		ret = -ENOMEM;
		goto exit_error;
	}

	drvdata->pm = audio_hw_pm_create(&pdev->dev);
	if (IS_ERR(drvdata->pm)) {
		alog_err("can't create pm\n");
		ret = PTR_ERR(drvdata->pm);
		goto exit_error;
	}

	mutex_init(&drvdata->mtx);

	/*
	 * allocate char device
	 */
	ret = alloc_chrdev_region(&drvdata->devt, 0, 1, AUDIO_DEVICE_NAME);
	if (ret < 0) {
		alog_err("can't allocate devt\n");
		goto exit_audio_hw_pm_destroy;
	}

	alog_info("device major %d minor %d\n", MAJOR(drvdata->devt),
		  MINOR(drvdata->devt));

	/*
	 * initialize char device
	 */
	cdev_init(&drvdata->cdev, &audio_hw_fops);
	drvdata->cdev.owner = THIS_MODULE;

	/* add char device */
	ret = cdev_add(&drvdata->cdev, drvdata->devt, 1);
	if (ret) {
		alog_err("can't add cdev: %d\n", ret);
		goto exit_unregister_chrdev_region;
	}

	/*
	 * create class
	 */
	drvdata->cls = class_create(THIS_MODULE, AUDIO_DEVICE_NAME);
	if (IS_ERR(drvdata->cls)) {
		ret = PTR_ERR(drvdata->cls);
		alog_err("can't create class: %d\n", ret);
		goto exit_cdev_del;
	}
	drvdata->cls->devnode = audio_hw_devnode;

	/*
	 * create sysfs node
	 */
	drvdata->clsdev = device_create(drvdata->cls, NULL, drvdata->devt,
					(void*)pdev, AUDIO_DEVICE_NAME);
	if (IS_ERR(drvdata->clsdev)) {
		ret = PTR_ERR(drvdata->clsdev);
		alog_err("can't create device: %d\n", ret);
		goto exit_class_destroy;
	}

	platform_set_drvdata(pdev, (void *)drvdata);

	audio_driver_data = drvdata;
		
	return 0; /* succeed */

exit_class_destroy:
	class_destroy(drvdata->cls);
	drvdata->cls = NULL;

exit_cdev_del:
	cdev_del(&drvdata->cdev);

exit_unregister_chrdev_region:
	unregister_chrdev_region(drvdata->devt, 1);
	drvdata->devt = 0;

exit_audio_hw_pm_destroy:
	audio_hw_pm_destroy(drvdata->pm);

exit_error:
	return ret;
}

static int audio_hw_pdrv_remove(struct platform_device *pdev)
{
	struct audio_hw_drvdata *drvdata;

	drvdata = platform_get_drvdata(pdev);

	if (drvdata && drvdata->cls) {
		if (drvdata->clsdev) {
			device_destroy(drvdata->cls, drvdata->devt);
			drvdata->clsdev = NULL;
		}
		class_destroy(drvdata->cls);
	}

	platform_set_drvdata(pdev, NULL);
	return 0;
}

static int audio_hw_suspend(struct device *dev)
{
	struct audio_hw_drvdata *drvdata;

	alog_info("suspend\n");

	drvdata = dev_get_drvdata(dev);
	if (!drvdata) {
		alog_err("invalid drvdata\n");
		return -EINVAL;
	}

	return audio_hw_pm_suspend(drvdata->pm);
}

static int audio_hw_resume(struct device *dev)
{
	struct audio_hw_drvdata *drvdata;

	alog_info("resume\n");

	drvdata = dev_get_drvdata(dev);
	if (!drvdata) {
		alog_err("invalid drvdata\n");
		return -EINVAL;
	}

	return audio_hw_pm_resume(drvdata->pm);
}

static SIMPLE_DEV_PM_OPS(audio_hw_pm_ops, audio_hw_suspend, audio_hw_resume);

static struct platform_driver audio_hw_pdrv = {
	.probe	= audio_hw_pdrv_probe,
	.remove	= audio_hw_pdrv_remove,
	.driver = {
		.name	= AUDIO_DEVICE_NAME,
		.owner	= THIS_MODULE,
		.bus	= &platform_bus_type,
#if IS_ENABLED(CONFIG_PM_SLEEP)
		.pm	= &audio_hw_pm_ops,
#endif
	},
};

static struct platform_device *audio_hw_pdev = NULL;

static int __init audio_hw_init(void)
{
    extern void rtkaudio_Audio_AtvSetSubAudioInHandle_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvSetAudioInHandle_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvSetDeviationMethod_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_Hw_SetFmRadioMode_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_SetAudioHWConfig_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvCleanTVSourceData_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvPauseTVStdDetection_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvStopDecoding_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpAADSIFInit_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpCurSifType_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvEnableAutoChangeSoundModeFlag_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvInit_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpAADSIFFinalize_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvEnterAtvSource_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvStartDecoding_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpSetSIFDataSource_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_Hw_Set_Scan_Std_Flag_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvSetMtsPriority_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpSetAtvAudioBand_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvConfigNoSignalNeed2Mute_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvEnableAutoChangeStdFlag_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvForceFirstTimeMonoSoundFlag_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvSetFwPriority_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_Auto_Verify_Set_Path_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvGetSoundStd_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvSetDevOnOff_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvSetDevBandWidth_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvGetNicamSignalStable_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvForceSoundSel_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvSetBTSCSoundSelect_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvSetA2SoundSelect_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvSetNICAMSoundSelect_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvSetFMoutDownGain_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpSIFGetNicamTH_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpSIFGetA2StereoDualTH_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpSIFDetectedSoundStandard_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpSIFCheckNicamDigital_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpSIFSetNicamTH_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpSIFSetA2StereoDualTH_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpSetBandDelay_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpSetChannelChange_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvGetCurrentDetectUpdate_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_HwpSIFGetMainToneSNR_ptr(void* fun_ptr);
    extern void rtkaudio_Audio_AtvSetSoundStd_ptr(void* fun_ptr);

	int ret = 0;

	/*
	* allocate and register platform device
	*/
	audio_hw_pdev = platform_device_register_simple(
				AUDIO_DEVICE_NAME, PLATFORM_DEVID_NONE,
				NULL, 0);
	if (IS_ERR(audio_hw_pdev)) {
		ret = PTR_ERR(audio_hw_pdev);
		alog_err("can't register platform device %d\n", ret);
		goto exit_error;
	}

	/*
	 * register platform driver
	 */
	ret = platform_driver_register(&audio_hw_pdrv);
	if (ret) {
		alog_warning("can't register platform driver %d\n", ret);
		goto exit_unregister_platform_device;
	}

    rtkaudio_Audio_AtvSetSubAudioInHandle_ptr(Audio_AtvSetSubAudioInHandle);
    rtkaudio_Audio_AtvSetAudioInHandle_ptr(Audio_AtvSetAudioInHandle);
    rtkaudio_Audio_AtvSetDeviationMethod_ptr(Audio_AtvSetDeviationMethod);
    rtkaudio_Audio_Hw_SetFmRadioMode_ptr(Audio_Hw_SetFmRadioMode);
    rtkaudio_Audio_SetAudioHWConfig_ptr(Audio_SetAudioHWConfig);
    rtkaudio_Audio_AtvCleanTVSourceData_ptr(Audio_AtvCleanTVSourceData);
    rtkaudio_Audio_AtvPauseTVStdDetection_ptr(Audio_AtvPauseTVStdDetection);
    rtkaudio_Audio_AtvStopDecoding_ptr(Audio_AtvStopDecoding);
    rtkaudio_Audio_HwpAADSIFInit_ptr(Audio_HwpAADSIFInit);
    rtkaudio_Audio_HwpCurSifType_ptr(Audio_HwpCurSifType);
    rtkaudio_Audio_AtvEnableAutoChangeSoundModeFlag_ptr(Audio_AtvEnableAutoChangeSoundModeFlag);
    rtkaudio_Audio_AtvInit_ptr(Audio_AtvInit);
    rtkaudio_Audio_HwpAADSIFFinalize_ptr(Audio_HwpAADSIFFinalize);
    rtkaudio_Audio_AtvEnterAtvSource_ptr(Audio_AtvEnterAtvSource);
    rtkaudio_Audio_AtvStartDecoding_ptr(Audio_AtvStartDecoding);
    rtkaudio_Audio_HwpSetSIFDataSource_ptr(Audio_HwpSetSIFDataSource);
    rtkaudio_Audio_Hw_Set_Scan_Std_Flag_ptr(Audio_Hw_Set_Scan_Std_Flag);
    rtkaudio_Audio_AtvSetMtsPriority_ptr(Audio_AtvSetMtsPriority);
    rtkaudio_Audio_HwpSetAtvAudioBand_ptr(Audio_HwpSetAtvAudioBand);
    rtkaudio_Audio_AtvConfigNoSignalNeed2Mute_ptr(Audio_AtvConfigNoSignalNeed2Mute);
    rtkaudio_Audio_AtvEnableAutoChangeStdFlag_ptr(Audio_AtvEnableAutoChangeStdFlag);
    rtkaudio_Audio_AtvForceFirstTimeMonoSoundFlag_ptr(Audio_AtvForceFirstTimeMonoSoundFlag);
    rtkaudio_Audio_AtvSetFwPriority_ptr(Audio_AtvSetFwPriority);
    rtkaudio_Audio_Auto_Verify_Set_Path_ptr(Audio_Auto_Verify_Set_Path);
    rtkaudio_Audio_AtvGetSoundStd_ptr(Audio_AtvGetSoundStd);
    rtkaudio_Audio_AtvSetDevOnOff_ptr(Audio_AtvSetDevOnOff);
    rtkaudio_Audio_AtvSetDevBandWidth_ptr(Audio_AtvSetDevBandWidth);
    rtkaudio_Audio_AtvGetNicamSignalStable_ptr(Audio_AtvGetNicamSignalStable);
    rtkaudio_Audio_AtvForceSoundSel_ptr(Audio_AtvForceSoundSel);
    rtkaudio_Audio_AtvSetBTSCSoundSelect_ptr(Audio_AtvSetBTSCSoundSelect);
    rtkaudio_Audio_AtvSetA2SoundSelect_ptr(Audio_AtvSetA2SoundSelect);
    rtkaudio_Audio_AtvSetNICAMSoundSelect_ptr(Audio_AtvSetNICAMSoundSelect);
    rtkaudio_Audio_AtvSetFMoutDownGain_ptr(Audio_AtvSetFMoutDownGain);
    rtkaudio_Audio_HwpSIFGetNicamTH_ptr(Audio_HwpSIFGetNicamTH);
    rtkaudio_Audio_HwpSIFGetA2StereoDualTH_ptr(Audio_HwpSIFGetA2StereoDualTH);
    rtkaudio_Audio_HwpSIFDetectedSoundStandard_ptr(Audio_HwpSIFDetectedSoundStandard);
    rtkaudio_Audio_HwpSIFCheckNicamDigital_ptr(Audio_HwpSIFCheckNicamDigital);
    rtkaudio_Audio_HwpSIFSetNicamTH_ptr(Audio_HwpSIFSetNicamTH);
    rtkaudio_Audio_HwpSIFSetA2StereoDualTH_ptr(Audio_HwpSIFSetA2StereoDualTH);
    rtkaudio_Audio_HwpSetBandDelay_ptr(Audio_HwpSetBandDelay);
    rtkaudio_Audio_HwpSetChannelChange_ptr(Audio_HwpSetChannelChange);
    rtkaudio_Audio_AtvGetCurrentDetectUpdate_ptr(Audio_AtvGetCurrentDetectUpdate);
    rtkaudio_Audio_HwpSIFGetMainToneSNR_ptr(Audio_HwpSIFGetMainToneSNR);
    rtkaudio_Audio_AtvSetSoundStd_ptr(Audio_AtvSetSoundStd);

	return 0; /* succeed */

exit_unregister_platform_device:
	platform_device_unregister(audio_hw_pdev);

exit_error:
	return ret;
}

static void __exit audio_hw_cleanup(void)
{
	struct audio_hw_drvdata *drvdata;

	if (audio_hw_pdev) {
		drvdata = platform_get_drvdata(audio_hw_pdev);
		if (drvdata && drvdata->pm)
			audio_hw_pm_destroy(drvdata->pm);

		platform_device_unregister(audio_hw_pdev);
		audio_hw_pdev = NULL;
	}

	platform_driver_unregister(&audio_hw_pdrv);
}

late_initcall(audio_hw_init);
module_exit(audio_hw_cleanup);
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
MODULE_AUTHOR("realtek.com.tw");
MODULE_LICENSE("GPL");
