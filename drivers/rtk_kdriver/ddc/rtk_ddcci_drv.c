#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/kthread.h>
#include "rtk_ddc_dbg.h"
#include "rtk_ddcci_priv.h"
#include "rtk_ddcci_sysfs.h"
#include <rtd_log/rtd_module_log.h>

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#define RTK_DDCCI_DEV_NUM  1
static dev_t rtk_ddcci_dev_t;
static struct cdev rtk_ddcci_cdev;
static struct class *rtk_ddcci_class;

struct task_struct *g_rtk_ddcci_fifo_thread = NULL;
struct task_struct *g_rtk_ddcci_dbg_thread = NULL;

static int rtk_ddcci_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int rtk_ddcci_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static long rtk_ddcci_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    unsigned char id = 0;
    ddcci_msg msg;

    memset(&msg,0,sizeof(msg));

    switch (cmd) {
        case DDCCI_IOC_ENABLE:
            if (copy_from_user((void *)&id, (const void __user *)arg, sizeof(unsigned char))) {
                rtd_pr_ddc_ci_err("[ERR] DDCCI_IOC_ENABLE copy_from_user error\n");
                ret = EFAULT;
            } else {
                if(id < 4)
                    rtk_ddcci_lib_init(id);
            }
#ifdef CONFIG_RTK_KDRV_DDCCI_USING_POLLING
            if(g_rtk_ddcci_fifo_thread) {
                rtd_pr_ddc_ci_err("[ERR] g_rtk_ddcci_fifo_thread already exist\n");
            } else {
                 g_rtk_ddcci_fifo_thread = kthread_run(rtk_ddcci_fifo_thread, NULL, "ddcci_fifo_thread");
                if(!g_rtk_ddcci_fifo_thread) {
                    return -ECHILD;
                }
                rtd_pr_ddc_ci_err("[ERR] g_rtk_ddcci_fifo_thread start\n");
            }
#endif
            break;

        case DDCCI_IOC_SEND_MESSAGE:
            if (copy_from_user((void *)&msg, (const void __user *)arg, sizeof(ddcci_msg))) {
                rtd_pr_ddc_ci_err("[ERR] DDCCI_IOC_SEND_MESSAGE copy_from_user error\n");
                ret = EFAULT;
            } else {
                if(msg.len > DDC_BUF_LENGTH)
                {
                    rtd_pr_ddc_ci_err("[ERR] send len over max, (%d)=>(%d)\n", msg.len, DDC_BUF_LENGTH);
                    msg.len = DDC_BUF_LENGTH;
                }

                if (rtk_ddcci_xmit_message(msg.buf,msg.len) != 0) {
                    rtk_ddcci_isr_tx_fifo_t();
                }
            }
            break;

        case DDCCI_IOC_RCV_MESSAGE:
            if (rtk_ddcci_rcv_message(msg.buf, &msg.len) != 0) {

                if (copy_to_user((void *)arg, &msg, sizeof(ddcci_msg))) {
                    rtd_pr_ddc_ci_err("[ERR] DDCCI_IOC_RCV_MESSAGE copy_to_user error\n");
                    ret = EFAULT;
                }
            }
            break;

        default:
            rtd_pr_ddc_ci_err("ddcci : unknown ioctl cmd %08x\n", cmd);
    }
    return ret;
}

#ifdef CONFIG_COMPAT
static long rtk_ddcci_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *compat_arg = compat_ptr(arg);
    return rtk_ddcci_ioctl(filp, cmd, (unsigned long)compat_arg);
}
#endif

struct file_operations rtk_ddcci_fops = {
    .owner            = THIS_MODULE,
    .open             = rtk_ddcci_open,
    .release          = rtk_ddcci_release,
    .unlocked_ioctl   = rtk_ddcci_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl     = rtk_ddcci_compat_ioctl,
#endif
};

int rtk_ddcci_cdev_init(void)
{
    int ret     = 0;
    struct device *dev;

    ret = alloc_chrdev_region(&rtk_ddcci_dev_t, 0, RTK_DDCCI_DEV_NUM, "rtk_ddcci");
    if (ret) {
        rtd_pr_ddc_ci_err("fail to get char dev Major and Minor \n");
        goto FAIL_ALLOC_CHRDEV_MAJOR;
    }

    cdev_init(&rtk_ddcci_cdev, &rtk_ddcci_fops);

    ret = cdev_add(&rtk_ddcci_cdev, rtk_ddcci_dev_t, RTK_DDCCI_DEV_NUM);
    if (ret) {
        rtd_pr_ddc_ci_err("fail to add char dev to system \n");
        goto FAIL_ADD_CHRDEV;
    }

    rtk_ddcci_class = class_create(THIS_MODULE, "rtk_ddcci");
    if (IS_ERR(rtk_ddcci_class)) {
        rtd_pr_ddc_ci_err("fail to cearte class \n");
        ret = PTR_ERR(rtk_ddcci_class);
        goto FAIL_CREATE_CLASS;
    }

    dev = device_create(rtk_ddcci_class, NULL, rtk_ddcci_cdev.dev, NULL, "rtk_ddcci");
    if (!dev) {
        rtd_pr_ddc_ci_err("fail to cearte device node for rtk_ddcci \n");
        ret = -ENOMEM;
        goto FAIL_CREATE_DEVICE;
    }

    if (rtk_ddcci_sysfs_create(dev)) {
        rtd_pr_ddc_ci_warn("Create self-defined sysfs attributes fail \n");
    }

    return ret;

FAIL_CREATE_DEVICE:
    class_destroy(rtk_ddcci_class);
    rtk_ddcci_class = NULL;
FAIL_CREATE_CLASS:
    cdev_del(&rtk_ddcci_cdev);
FAIL_ADD_CHRDEV:
    unregister_chrdev_region(rtk_ddcci_dev_t, RTK_DDCCI_DEV_NUM);
FAIL_ALLOC_CHRDEV_MAJOR:
    return ret;
}

void rtk_ddcci_cdev_uninit(void)
{
    device_destroy(rtk_ddcci_class, rtk_ddcci_cdev.dev);
    class_destroy(rtk_ddcci_class);
    rtk_ddcci_class = NULL;
    cdev_del(&rtk_ddcci_cdev);
    unregister_chrdev_region(rtk_ddcci_dev_t, RTK_DDCCI_DEV_NUM);
}

