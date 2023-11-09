/************************************************************************
 *  Include files
 ************************************************************************/
#include <linux/uaccess.h>
#include <linux/bitops.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/export.h>
#include <linux/compat.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
//#include <mach/io.h>
#include <linux/irq.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/input.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/kfifo.h>
#include <linux/kthread.h>
#include <linux/of.h>
#include <linux/pci.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/random.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/signal.h>
#include <linux/stat.h>
#include <linux/string.h>
#include "rtk_ai.h"
#include "rtk_otp_region_api.h"
#include "rtk_ai_common.h"

/************************************************************************
 *  Definitions
 ************************************************************************/
//#define ENABLE_RTK_MC_MEASURE
#define MAX_CMD_LEN   128 

enum {
	AIDRV_CMD_CLOCK_RATIO = 0,
	AIDRV_CMD_AUDIO_STATUS = 1,
	AIDRV_CMD_NPU_LOAD = 2,
	AIDRV_CMD_NUM
} AIDRV_DEBUG_CMD_E;

static const char* aidrv_cmd_str[] = {
	"clock_ratio=",     /*AIDRV_CMD_CLOCK_RATIO*/
	"audio_status=",    /*AIDRV_CMD_AUDIO_STATUS*/
	"npu_load=",        /*AIDRV_CMD_NPU_LOAD*/
};

/************************************************************************
*  Public variables
************************************************************************/
static struct cdev *dev_cdevp_ai = NULL;
static struct class *_ai_class;
static struct device *ai_device_st;
static struct platform_device *rtk_ai_platform_dev;
static int ai_dev_major;
static int ai_dev_minor;
#ifdef  ENABLE_RTK_MC_MEASURE
static struct task_struct *ai_mc_tsk;
static int ai_mc_init_data;
static int ai_mc_measure_interval=6;	//seconds
#endif
extern AI_AUDIO_STATE aiAudioState;
static int ai_refCnt[AI_ENABLE_STATUS_CAMERA+1];


/************************************************************************
*  Function body
************************************************************************/
#ifdef  ENABLE_RTK_MC_MEASURE
extern int rtk_qos_bw_start_measure(unsigned int n_sec);
static int rtk_ai_mc_init_thread(void *arg){

        //msleep(10);
        AI_ERROR("Josh:start mc measure %d seconds\n",ai_mc_measure_interval);
        rtk_qos_bw_start_measure(ai_mc_measure_interval);
        AI_ERROR("Josh:mc measure end\n");
        return 0;
}
#endif
extern bool rtk_is_NNIP_function_disable(void);

static void rtk_ai_backup_info(void)
{
	rtk_ai_mc_backup();
}

static void rtk_ai_restore_info(void)
{
	rtk_ai_mc_restore();
}

static void rtk_ai_execute_aidrv_cmd(int cmd_id, char* value_ptr)
{

	AI_WARN("[%s %d]cmd_id = %d, value = %s\n", __func__, __LINE__, cmd_id, value_ptr);

	switch(cmd_id)
	{
		case AIDRV_CMD_CLOCK_RATIO:
		{
			int ratio=0;
			if(kstrtoint(value_ptr, 0, &ratio) != 0)
				break;

			AI_WARN("set NNIP clock ratio to %d\n",ratio);
			rtk_ai_mc_setClockRatio(ratio);
			break;
		}
		case AIDRV_CMD_AUDIO_STATUS:
		{
			AI_ENABLE_STATUS status;
			
			if(kstrtoint(value_ptr, 0, &status.enable) != 0)
				break;

			AI_WARN("set audio status to %d\n",status.enable);
			status.type=AI_ENABLE_STATUS_AUDIO;
			rtk_ai_set_enable_status(&status);
			break;
				
			break;		
		}
		case AIDRV_CMD_NPU_LOAD:
		{
			int enable=0;
			if(kstrtoint(value_ptr, 0, &enable) != 0)
				break;

			AI_WARN("set NPU load to %d\n",enable);
			rtk_ai_mc_enableNpuLoadCheck(enable);
			break;
		}		
		default:
			break;
	}
}

#if IS_ENABLED(CONFIG_RTK_AI_OPTEE)
extern int nn_ta_lock_npu(bool lock);
#endif
int rtk_ai_set_enable_status(AI_ENABLE_STATUS *status)
{
	unsigned int bitMask=0;
	
	if(status==NULL){
		AI_WARN("%s: status is null pointer\n",__FUNCTION__);
		return -EINVAL;
	}

	AI_INFO("%s:type=%d,enable=%d\n",__FUNCTION__,status->type,status->enable);
	
	switch(status->type){
		case AI_ENABLE_STATUS_PQ:
			bitMask=BIT_AI_PQ;
			break;
		case AI_ENABLE_STATUS_AUDIO:
			bitMask=BIT_AI_AUDIO;
			aiAudioState.enable=status->enable;
			break;
		case AI_ENABLE_STATUS_CAMERA:
			bitMask=BIT_AI_CAMERA;
			break;
		default:
			return -EINVAL;
	}
	
	if(status->enable){
	#if IS_ENABLED(CONFIG_RTK_AI_OPTEE)
		if(ai_refCnt[status->type] == 0)
		{
			int ret = nn_ta_lock_npu(1);
			if(ret < 0)
				return ret;

		}
	#endif
		rtk_ai_enable_status(bitMask);
		ai_refCnt[status->type]++;
	}else{
		ai_refCnt[status->type]--;
		if(ai_refCnt[status->type]<=0){
			rtk_ai_disable_status(bitMask);
			ai_refCnt[status->type]=0;
		#if IS_ENABLED(CONFIG_RTK_AI_OPTEE)
			return nn_ta_lock_npu(0);
		#endif
		}
	}
		
	return 0;
}

#ifdef CONFIG_PM
/*static int rtk_pwm_suspend(struct platform_device *dev, pm_message_t state)*/
static int rtk_ai_suspend(struct device *dev)
{
    AI_WARN("suspend\n");
    rtk_ai_backup_info();
    return 0;
}

/*static int rtk_pwm_resume(struct platform_device *dev)*/
static int rtk_ai_resume(struct device *dev)
{

    AI_WARN("resume\n");
    rtk_ai_restore_info();
    return 0;
}

#ifdef CONFIG_HIBERNATION
static int rtk_ai_suspend_std(struct device *dev)
{
    AI_WARN("%s \n" , __func__);
    rtk_ai_backup_info();
    return 0;
}
static int rtk_ai_resume_std(struct device *dev)
{
    AI_WARN("%s \n" , __func__);
    rtk_ai_restore_info();
    return 0;
}

static int rtk_ai_poweroff(struct device *dev)
{
    AI_WARN("%s \n" , __func__);
    return 0;
}
static int rtk_ai_restore(struct device *dev)
{
    AI_WARN("%s \n" , __func__);
    rtk_ai_restore_info();
    return 0;
}
#endif

static const struct dev_pm_ops rtk_ai_pm_ops = {

    .suspend_noirq = rtk_ai_suspend,
    .resume_noirq = rtk_ai_resume,
#ifdef CONFIG_HIBERNATION
    .freeze_noirq     = rtk_ai_suspend_std,
    .thaw_noirq       = rtk_ai_resume_std,
    .poweroff_noirq     = rtk_ai_poweroff,
    .restore_noirq       = rtk_ai_restore,    
#endif
};

#endif

static int rtk_ai_probe(struct platform_device *pdev)
{
    AI_WARN("%s():\n", __FUNCTION__);
    return 0;
}

static int rtk_ai_remove(struct platform_device *pdev)
{
    AI_WARN("%s():\n", __FUNCTION__);
    return 0;
}

ssize_t rtk_ai_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    AI_WARN("%s():\n", __FUNCTION__);
    return -EFAULT;
}
ssize_t rtk_ai_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	char str[MAX_CMD_LEN];
	int cmd_id;
	char value_ptr[MAX_CMD_LEN];

    AI_WARN("%s():\n", __FUNCTION__);
	if(buf == 0)
	{
		AI_ERROR("[%s %d]invalid argument\n", __func__, __LINE__);
		return -EFAULT;
	}
	if(count > MAX_CMD_LEN-1)
		count = MAX_CMD_LEN-1;
	if (copy_from_user(str, buf, count))
	{
		AI_ERROR("[%s %d]copy cmd fail\n", __func__, __LINE__);
		return -EFAULT;
	}
	if(count > 0)
		str[count-1] = '\0';

	for(cmd_id = 0; cmd_id < AIDRV_CMD_NUM; cmd_id++)
	{
		if(strncmp(str, aidrv_cmd_str[cmd_id], strlen(aidrv_cmd_str[cmd_id])) == 0)
			break;
	}
	if(cmd_id == AIDRV_CMD_NUM)
		return -EFAULT;
	if(strlen(aidrv_cmd_str[cmd_id]) >= MAX_CMD_LEN)
		return -EFAULT;

	memcpy((void *)value_ptr,(void *)&str[strlen(aidrv_cmd_str[cmd_id])],MAX_CMD_LEN-strlen(aidrv_cmd_str[cmd_id]));
	
	rtk_ai_execute_aidrv_cmd(cmd_id, value_ptr);

	return count;    
}

int rtk_ai_open(struct inode *inode, struct file *filp)
{
    AI_WARN("%s():\n", __FUNCTION__);
    filp->private_data = (void *)(-1);
    //AI_INFO("%s():private_data=%d\n",(int)__FUNCTION__,(int)filp->private_data);
    return 0;
}

int rtk_ai_release(struct inode *inode, struct file *filp)
{
    AI_WARN("%s():\n", __FUNCTION__);
    AI_INFO("%s():private_data=%ld\n",__FUNCTION__,(unsigned long)filp->private_data);

    rtk_ai_mc_crashRelease((unsigned long)filp->private_data);
    return 0;
}

long rtk_ai_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int retval = 0;
    IOCTL_REGIST_MODEL_PARAM modelParam;
 	IOCTL_GRANT_PARAM grantParam;
 	unsigned int phyAddr;
 	unsigned int value,i,max=0,index=0;

    AI_DEBUG("%s() enter\n", __FUNCTION__);

    switch (cmd) {
    case AI_IOC_INIT:
        break;
    case AI_IOC_UNINIT:
        break;
    case AI_IOC_OPEN:
        break;
    case AI_IOC_CLOSE:
        break;
    case AI_IOC_REGIST_MODEL:
        if (copy_from_user(&modelParam, (void __user *)arg,sizeof(IOCTL_REGIST_MODEL_PARAM))) {
					AI_ERROR("AI_IOC_REGIST_MODEL failed, copy data from user space failed\n");
					retval=-EFAULT;
					break;
        }
        filp->private_data = (void *)((unsigned long)rtk_ai_mc_getNextHandle());
    		if(rtk_ai_mc_registModel(&modelParam.info,(unsigned long)filp->private_data)==FALSE)
					retval=-EFAULT;
				else
					retval=(unsigned long)filp->private_data;
        break;
    case AI_IOC_UNREGIST_MODEL:
    		if(rtk_ai_mc_unRegistModel((int)arg)==FALSE)
					retval=-EFAULT;
        break;
    case AI_IOC_GET_SHARE_MEM:
    		if(rtk_ai_mc_getShareMem(&phyAddr)==FALSE)
					retval=-EFAULT;
				else{
					if(copy_to_user((unsigned int __user*)arg,&phyAddr,4)){
						AI_ERROR("AI_IOC_GET_SHARE_MEM failed, copy data to user space failed\n");
						retval=-EFAULT;
					}
				}
        break;
    case AI_IOC_GET_GRANT:
        if (copy_from_user(&grantParam, (void __user *)arg,sizeof(IOCTL_GRANT_PARAM))) {
					AI_ERROR("AI_IOC_GET_GRANT failed, copy data from user space failed\n");
					retval=-EFAULT;
        }else{
        	if(rtk_ai_mc_getGrant(grantParam.handle,grantParam.index)==FALSE)
        		retval=-EFAULT;
        }	
        break;
    case AI_IOC_RELEASE_GRANT:
        if (copy_from_user(&grantParam, (void __user *)arg,sizeof(IOCTL_GRANT_PARAM))) {
					AI_ERROR("AI_IOC_RELEASE_GRANT failed, copy data from user space failed\n");
					retval=-EFAULT;
        }else{
	        if(rtk_ai_mc_releaseGrant(grantParam.handle,grantParam.index)==FALSE)
        		retval=-EFAULT;
      	}
        break;
    case AI_IOC_START_MC_MEASURE:
#ifdef  ENABLE_RTK_MC_MEASURE    	
        if (copy_from_user(&ai_mc_measure_interval, (void __user *)arg,sizeof(unsigned int))) {
					AI_ERROR("AI_IOC_START_MC_MEASURE failed, copy data from user space failed\n");
					retval=-EFAULT;
        }else{
        	ai_mc_tsk = kthread_create(rtk_ai_mc_init_thread, &ai_mc_init_data, "rtk_ai_mc_init_thread");
            if (IS_ERR(ai_mc_tsk)) {
                        int ret;
                ret = PTR_ERR(ai_mc_tsk);
                ai_mc_tsk = NULL;
                AI_ERROR("Josh:create rtk_ai_mc_init_thread fail\n");
                return -EFAULT;
            }
            wake_up_process(ai_mc_tsk);        	
      	}
#endif      	
        break;
    case AI_IOC_STOP_MC_MEASURE:
    	//do nothing
		break;
    case AI_IOC_AUDIO_SET_VOCAL_VOL:
        if (copy_from_user(&value, (void __user *)arg,sizeof(unsigned int))) {
			AI_ERROR("AI_IOC_AUDIO_SET_VOCAL_VOL failed, copy data from user space failed\n");
			retval=-EFAULT;
			break;
        }    	
		AI_ERROR("AI_IOC_AUDIO_SET_VOCAL_VOL:vol=%d\n",value);
    	rtk_ai_audio_vs_set_volume(1,value);
		break;
    case AI_IOC_AUDIO_SET_MUSIC_VOL:
        if (copy_from_user(&value, (void __user *)arg,sizeof(unsigned int))) {
			AI_ERROR("AI_IOC_AUDIO_SET_MUSIC_VOL failed, copy data from user space failed\n");
			retval=-EFAULT;
			break;
        }    	
		AI_ERROR("AI_IOC_AUDIO_SET_MUSIC_VOL:vol=%d\n",value);
    	rtk_ai_audio_vs_set_volume(0,value);
		break;
    case AI_IOC_AUDIO_SET_EMOTION:
        if (copy_from_user(aiAudioState.emotionScore, (void __user *)arg,sizeof(unsigned int)*AI_EMOTION_TYPE_NUM)) {
			AI_ERROR("AI_IOC_AUDIO_SET_EMOTION failed, copy data from user space failed\n");
			retval=-EFAULT;
			break;
        }
        
        for(i=0;i<AI_EMOTION_TYPE_NUM;i++){
        	if(aiAudioState.emotionScore[i]>max){
        		max=aiAudioState.emotionScore[i];	
        		index=i;
        	}
        }
		aiAudioState.emotion=index;
		AI_ERROR("AI_IOC_AUDIO_SET_EMOTION:emotion=%d\n",aiAudioState.emotion);
		break;
    case AI_IOC_SET_ENABLE_STATUS:
    	{
			AI_ENABLE_STATUS status;
			
	        if (copy_from_user(&status, (void __user *)arg,sizeof(AI_ENABLE_STATUS))) {
				AI_ERROR("AI_IOC_SET_ENABLE_STATUS failed, copy data from user space failed\n");
				retval=-EFAULT;
				break;
	        }    	
		retval = rtk_ai_set_enable_status(&status);
			break;
		}
    default:
        retval = -ENOTTY;
    }

    AI_DEBUG("%s() exit\n", __FUNCTION__);

    return retval;

}

#ifdef CONFIG_COMPAT
long rtk_ai_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *compat_arg = compat_ptr(arg);
    return rtk_ai_ioctl(filp, cmd, (unsigned long)compat_arg);
}
#endif

static struct file_operations rtk_ai_fops = {
    .owner = THIS_MODULE,
    .read = rtk_ai_read,
    .write = rtk_ai_write,
    .unlocked_ioctl = rtk_ai_ioctl,
    .release = rtk_ai_release,
    .open = rtk_ai_open,
#ifdef CONFIG_COMPAT
    .compat_ioctl = rtk_ai_compat_ioctl,
#endif
};


static const struct of_device_id rtk_ai_devices[] = {
    {.compatible = "rtk-ai",},
    {},
};

static struct platform_driver rtk_ai_driver = {
    .driver = {
            .name = "rtk-ai",
            .owner = THIS_MODULE,
            .of_match_table = of_match_ptr(rtk_ai_devices),
            #ifdef CONFIG_PM
            .pm    = &rtk_ai_pm_ops,
            #endif
        },
    .probe = rtk_ai_probe,
    .remove = rtk_ai_remove,
};

static int register_ai_cdev(void) {
    dev_t dev;
    int ret = 0;


    ret = alloc_chrdev_region(&dev, 0, 1, AI_DEVICE_NAME);
    if (ret) {
        AI_ERROR("can't alloc chrdev\n");
        return -1;
    }

    ai_dev_major = MAJOR(dev);
    ai_dev_minor = MINOR(dev);


    dev_cdevp_ai = kzalloc(sizeof(struct cdev), GFP_KERNEL);
    if (dev_cdevp_ai == NULL) {
        AI_ERROR("kzalloc failed\n");
        goto FAIL_TO_ALLOCATE_AI_CDEV;
    }
    cdev_init(dev_cdevp_ai, &rtk_ai_fops);
    dev_cdevp_ai->owner = THIS_MODULE;
    dev_cdevp_ai->ops = &rtk_ai_fops;
    ret = cdev_add(dev_cdevp_ai, MKDEV(ai_dev_major, ai_dev_minor), 1);
    if (ret < 0) {
        AI_WARN(" add chr dev failed\n");
        goto FAIL_TO_ADD_AI_CDEV;
    }

    ai_device_st = device_create(_ai_class, NULL, MKDEV(ai_dev_major, ai_dev_minor), NULL, AI_DEVICE_NAME);
    if(ai_device_st == NULL) {
        AI_WARN(" device create ai dev failed\n");
        goto FAIL_TO_CREATE_AI_DEVICE;
    }
    AI_WARN("register chrdev(%d,%d) success.\n", ai_dev_major, ai_dev_minor);
    return 0;
    
FAIL_TO_CREATE_AI_DEVICE:
    cdev_del(dev_cdevp_ai);
FAIL_TO_ADD_AI_CDEV:
    if(dev_cdevp_ai) {
        kfree(dev_cdevp_ai);
        dev_cdevp_ai = NULL;
    }
FAIL_TO_ALLOCATE_AI_CDEV:
    unregister_chrdev_region(MKDEV(ai_dev_major, ai_dev_minor), 1);
    ai_dev_major = 0;
    ai_dev_minor = 0;
    return -1;

}


static int unregister_ai_cdev(void)
{
    device_destroy(_ai_class, MKDEV(ai_dev_major, ai_dev_minor));
    cdev_del(dev_cdevp_ai);
    kfree(dev_cdevp_ai);
    dev_cdevp_ai = NULL;
    unregister_chrdev_region(MKDEV(ai_dev_major, ai_dev_minor), 1);
    ai_dev_major = 0;
    ai_dev_minor = 0;
    return 0;

}



int __init rtk_ai_dev_init(void)
{
    int ret = 0;
    
    AI_INFO( "[%s] rtkai device init start !\n", __func__);

    ret = platform_driver_register(&rtk_ai_driver);
    if (ret != 0)
        goto FAILED_REGISTER_PLATFORM_DRIVER;

    _ai_class = class_create(THIS_MODULE, "rtk-ai");

    if (_ai_class == NULL)
        goto FAILED_CREATE_AI_CLASS;

    rtk_ai_platform_dev =platform_device_register_simple("rtk-ai", -1, NULL, 0);
    if (rtk_ai_platform_dev == NULL)
        goto FAILED_REGISTER_PLATFORM_DEVICE;

    if(register_ai_cdev() != 0)
        goto FAILED_REGISTER_AI_CDEV;

		ret=rtk_ai_mc_init();
		ret=rtk_ai_audio_init();
		memset((void *)ai_refCnt,0,sizeof(int)*(AI_ENABLE_STATUS_CAMERA+1));

    AI_INFO( "[%s] rtkai device init ok !\n", __func__);

    return 0;
    
FAILED_REGISTER_AI_CDEV:
    platform_device_unregister(rtk_ai_platform_dev);
FAILED_REGISTER_PLATFORM_DEVICE:
    device_destroy(_ai_class, MKDEV(0, 0));
    class_destroy(_ai_class);
FAILED_CREATE_AI_CLASS:
    platform_driver_unregister(&rtk_ai_driver);
FAILED_REGISTER_PLATFORM_DRIVER:
    AI_ERROR( "[%s] ======================================\n",__func__);
    AI_ERROR( "[%s] AI Register Error!\n", __func__);
    AI_ERROR( "[%s] ======================================\n",__func__);
    return -1;


}

static void __exit rtk_ai_dev_uninit(void)
{
    unregister_ai_cdev();
    platform_device_unregister(rtk_ai_platform_dev);
    device_destroy(_ai_class, MKDEV(0, 0));
    class_destroy(_ai_class);
    platform_driver_unregister(&rtk_ai_driver);
    rtk_ai_mc_unInit();
    rtk_ai_audio_unInit();
}

EXPORT_SYMBOL(rtk_ai_set_enable_status);
module_init(rtk_ai_dev_init);
module_exit(rtk_ai_dev_uninit);
MODULE_AUTHOR("joshlin, Realtek Semiconductor");
MODULE_LICENSE("GPL");


