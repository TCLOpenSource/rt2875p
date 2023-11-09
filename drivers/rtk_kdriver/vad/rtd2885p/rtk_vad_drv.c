/******************************************************************************
 *
 *   Copyright(c) 2021. Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author timo.wu@realtek.com
 *
 *****************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#include <linux/fs.h> // chrdev
#include <linux/cdev.h> // cdev_add()/cdev_del()
#include <linux/slab.h> // kmalloc()/kfree()
#include <linux/uaccess.h> // copy_*_user()
#include <linux/proc_fs.h>

#ifdef CONFIG_HIBERNATION
#include <linux/pm.h>
#endif

#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_crt.h>
#include <rtk_kdriver/rtk_vad_def.h>

/******************************************************************************
                     VAD only works on WOV DMIC path
*****************************************************************************/

SRTK_VAD_DEBUG_CMD Vad_debug_cmd[VAD_PROC_COUNT + 1]={
    {"log_onoff=",    VAD_PROC_SETLOGONOFF},
    {"NULL",          VAD_PROC_COUNT}
};

unsigned char vad_log_onoff = 0;

struct semaphore rtkvad_Semaphore;/* This Semaphore is for rtkvad control*/

/* parameters for debug */
struct proc_dir_entry * vad_proc_dir = NULL;
struct proc_dir_entry * vad_proc_entry = NULL;

#define RTK_VAD_DEV_BUFSIZE 1024

#define RTK_VAD_MAJOR 0
#define RTK_VAD_MINOR 0

static int rtk_vad_major = RTK_VAD_MAJOR;
static int rtk_vad_minor = RTK_VAD_MINOR;

module_param(rtk_vad_major, int, S_IRUGO);
module_param(rtk_vad_minor, int, S_IRUGO);

static struct class *rtk_vad_class;
static struct cdev rtk_vad_cdev;
static struct platform_device *rtk_vad_platform_devs;

#ifdef CONFIG_HIBERNATION
static int rtk_vad_std_suspend(struct device *dev);
static int rtk_vad_std_resume(struct device *dev);
static int rtk_vad_str_suspend(struct device *dev);
static int rtk_vad_str_resume(struct device *dev);

static const struct dev_pm_ops rtk_vad_pm_ops = {
    .freeze     = rtk_vad_std_suspend,
    .thaw       = rtk_vad_std_resume,
    .poweroff   = rtk_vad_std_suspend,
    .restore    = rtk_vad_std_resume,
    .suspend    = rtk_vad_str_suspend,
    .resume     = rtk_vad_str_resume,
};
#endif

#ifdef CONFIG_PM
static int rtk_vad_suspend(struct platform_device *dev, pm_message_t state);
static int rtk_vad_resume(struct platform_device *dev);
#endif

static struct platform_driver rtk_vad_device_driver = {
#ifdef CONFIG_PM
    .suspend        = rtk_vad_suspend,
    .resume         = rtk_vad_resume,
#endif
    .driver = {
        .name       = "rtkvad",
        .bus        = &platform_bus_type,
#ifdef CONFIG_HIBERNATION
        .pm         = &rtk_vad_pm_ops,
#endif
    },
};

int rtk_vad_open(struct inode *inode, struct file *filp);
int rtk_vad_release(struct inode *inode, struct file *filp);
ssize_t rtk_vad_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
ssize_t rtk_vad_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
long rtk_vad_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
long rtk_vad_ioctl_compat(struct file *filp, unsigned int cmd, unsigned long arg);
#endif
#endif

const struct file_operations rtk_vad_fops = {
    .owner = THIS_MODULE,
    .open = rtk_vad_open,
    .release = rtk_vad_release,
    .write = rtk_vad_write,
    .read = rtk_vad_read,
    .unlocked_ioctl = rtk_vad_ioctl,
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
    .compat_ioctl = rtk_vad_ioctl_compat,
#endif
#endif
};

int rtk_vad_open(struct inode *inode, struct file *filp)
{
    VAD_DBG("%s():\n", __FUNCTION__);
    return 0;
}

int rtk_vad_release(struct inode *inode, struct file *filp)
{
    VAD_DBG("%s():\n", __FUNCTION__);
    return 0;
}

ssize_t
rtk_vad_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    char data[] = "vad data\n";
    ssize_t ret = 0;
    int ret_ignore = 0;

    VAD_DBG("%s():\n", __FUNCTION__);

    if (*f_pos >= sizeof(data)) {
        goto out;
    }

    if (count > sizeof(data)) {
        count = sizeof(data);
    }

    ret_ignore = copy_to_user(buf, data, count);
    if (ret_ignore < 0) {
        ret = -EFAULT;
        goto out;
    }
    *f_pos += count;
    ret = count;

out:
    return ret;
}

ssize_t
rtk_vad_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    unsigned char str[128];
    unsigned int i;
    unsigned char *cmd = str;
    unsigned int input_data;

    /* check parameter */
    if (buf == NULL) {
        VAD_DBG("buf=%p\n", buf);
        return -EFAULT;
    }

    if (count == 0) {
        VAD_DBG("command should > 0\n");
        return -EINVAL;
    }
    
    if (count > 128) { /* procfs write and read has PAGE_SIZE limit */
        VAD_DBG("command too large, set length to 128 Byte\n");
        count = 128;
    }

    if (copy_from_user(str, buf, count)) {
        VAD_DBG("copy_from_user failed! (buf=%p, count="PT_UDEC_DUALADDRESS")\n", buf, count);
        return -EFAULT;
    }

    str[count-1] = '\0';

    VAD_DBG("proc data: %s\n", str);

    /* get command string */
    for (i = 0; i < VAD_PROC_COUNT; i++) {
        //VAD_DBG("[%d]%s\n", i, Vad_debug_cmd[i].vad_cmd_str);
        if (strncmp(str, Vad_debug_cmd[i].vad_cmd_str, strlen(Vad_debug_cmd[i].vad_cmd_str)) == 0) {
            VAD_DBG("VAD debug command: %s\n", Vad_debug_cmd[i].vad_cmd_str);
            break;
        }
    }

    /* command out of range check */
    if (i >= VAD_PROC_COUNT) {
        VAD_DBG("Cannot find your command: \"%s\"\n", str);
        //return -EINVAL; for coverity

    } else {
        /* get parameter */
        cmd += strlen(Vad_debug_cmd[i].vad_cmd_str);
        if (sscanf(cmd, "%9u", &input_data) < 1) {
            VAD_DBG("sscanf get more data failed (%s) - there is no more parameter.\n", cmd);
            //return -EFAULT;
        } else {
            VAD_DBG("input data=%u\n", input_data);
        }
    }

    switch (i) {
    case VAD_PROC_SETLOGONOFF: {
            vad_log_onoff = (unsigned char)input_data;

        if (input_data == 1) {
                VAD_WARNING("Enable VAD log\n");
        } else {
                VAD_WARNING("Disable VAD log\n");
            }
            break;
        }
    default: {
            VAD_DBG("unsupport proc command=%s\n", str);
            return -EINVAL;
        }
    }

    return count;
}

long rtk_vad_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    VAD_DBG("\nrtk vad: receive ioctl(cmd:0x%08x, arg:0x%08lx)\n", cmd, arg);
    return 0;
}

#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
long rtk_vad_ioctl_compat(struct file *filp, unsigned int cmd, unsigned long arg)
{
    VAD_DBG("\nrtk vad: receive ioctl(compat)(cmd:0x%08x, arg:0x%08lx)\n",cmd, arg);
    return 0;
}
#endif
#endif

#ifdef CONFIG_PM
static int rtk_vad_suspend(struct platform_device *dev, pm_message_t state)
{
    VAD_DBG("rtk vad: receive supend command\n");
    CRT_CLK_OnOff(WOV, CLK_OFF, NULL);
    return 0;
}

static int rtk_vad_resume(struct platform_device *dev)
{
    VAD_DBG("rtk vad: receive resume command\n");
    return 0;
}
#endif

#ifdef CONFIG_HIBERNATION
static int rtk_vad_std_suspend(struct device *dev)
{
    VAD_DBG("rtk vad: receive std suspend command\n");
    CRT_CLK_OnOff(WOV, CLK_OFF, NULL);
    return 0;
}

static int rtk_vad_std_resume(struct device *dev)
{
    VAD_DBG("rtk vad: receive std resume command\n");
    return 0;
}

static int rtk_vad_str_suspend(struct device *dev)
{
    VAD_DBG("rtk vad: receive str suspend command\n");
    CRT_CLK_OnOff(WOV, CLK_OFF, NULL);
    return 0;
}


static int rtk_vad_str_resume(struct device *dev)
{
    VAD_DBG("rtk vad: receive str resume command\n");
    return 0;
}
#endif

static char *rtk_vad_devnode(struct device *dev, mode_t *mode)
{
    //*mode = 0666;
    return NULL;
}

static int rtk_vad_create(void)
{
    int result = 0;
    dev_t dev = 0;
    rtk_vad_class = NULL;
    rtk_vad_platform_devs = NULL;

    VAD_DBG("rtk vad: VAD driver for Realtek TV Platform\n");

    if (rtk_vad_major) {
        dev = MKDEV(RTK_VAD_MAJOR, 0);
        result = register_chrdev_region(dev, 1, "rtkvad");
    } else {
        result = alloc_chrdev_region(&dev, rtk_vad_minor, 1, "rtkvad");
        rtk_vad_major = MAJOR(dev);
    }
    if (result < 0) {
        VAD_DBG("rtk vad: can not get chrdev region...\n");
        return result;
    }

    rtk_vad_class = class_create(THIS_MODULE, "rtkvad");
    if (IS_ERR(rtk_vad_class)) {
        VAD_DBG("rtk vad: can not create class...\n");
        result = PTR_ERR(rtk_vad_class);
        goto fail_class_create;
    }

    rtk_vad_class->devnode = (void*)rtk_vad_devnode;

    rtk_vad_platform_devs =
        platform_device_register_simple("rtkvad", -1, NULL, 0);

    if (platform_driver_register(&rtk_vad_device_driver) != 0) {
        VAD_DBG("rtk vad: can not register platform driver...\n");
        result = -EINVAL;
        goto fail_platform_driver_register;
    }

    cdev_init(&rtk_vad_cdev, &rtk_vad_fops);
    rtk_vad_cdev.owner = THIS_MODULE;
    rtk_vad_cdev.ops = &rtk_vad_fops;
    result = cdev_add(&rtk_vad_cdev, dev, 1);
    if (result < 0) {
        VAD_DBG("rtk vad: can not add character device...\n");
        goto fail_cdev_init;
    }
    device_create(rtk_vad_class, NULL, dev, NULL, "rtkvad");
    
    sema_init(&rtkvad_Semaphore, 1);

    device_enable_async_suspend(&(rtk_vad_platform_devs->dev));
    
#ifdef CONFIG_PROC_FS
    /* add proc entry */
    if ((vad_proc_dir == NULL) && (vad_proc_entry == NULL)) {
        vad_proc_dir = proc_mkdir(VAD_PROC_DIR, NULL);

        if (vad_proc_dir == NULL) {
            VAD_WARNING("create rhal_tp dir proc entry (%s) failed\n", VAD_PROC_DIR);
        } else {
            vad_proc_entry = proc_create(VAD_PROC_ENTRY, 0666, vad_proc_dir, &rtk_vad_fops);
            if (vad_proc_entry == NULL) {
                VAD_WARNING("failed to get proc entry for %s/%s \n", VAD_PROC_DIR, VAD_PROC_ENTRY);
            }
        }
    }
#else
    VAD_WARNING("This module requests the kernel to support procfs,need set CONFIG_PROC_FS configure Y\n");
#endif
    
    return 0;

fail_cdev_init:
    platform_driver_unregister(&rtk_vad_device_driver);
fail_platform_driver_register:
    platform_device_unregister(rtk_vad_platform_devs);
    rtk_vad_platform_devs = NULL;
    class_destroy(rtk_vad_class);
fail_class_create:
    rtk_vad_class = NULL;
    unregister_chrdev_region(dev, 1);
    return result;
}

static void rtk_vad_exit(void)
{
    dev_t dev = MKDEV(rtk_vad_major, rtk_vad_minor);
    
    VAD_DBG("\033[1;32;33m" "rtk vad: rtk_vad_exit\n" "\033[m");

    if (rtk_vad_platform_devs == NULL)
        BUG();

    device_destroy(rtk_vad_class, dev);
    cdev_del(&rtk_vad_cdev);

    platform_driver_unregister(&rtk_vad_device_driver);
    platform_device_unregister(rtk_vad_platform_devs);
    rtk_vad_platform_devs = NULL;

    class_destroy(rtk_vad_class);
    rtk_vad_class = NULL;

    unregister_chrdev_region(dev, 1);
    VAD_DBG("%s():unregister chrdev\n", __FUNCTION__);
}

module_init(rtk_vad_create);
module_exit(rtk_vad_exit);
MODULE_LICENSE("GPL");
