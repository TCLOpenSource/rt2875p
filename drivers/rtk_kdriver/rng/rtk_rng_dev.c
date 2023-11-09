#include <linux/syscalls.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <rtk_kdriver/rng/rtk_rng.h>
#include "rtk_rng_dbg.h"

#if 0//defined(CONFIG_HW_RANDOM)
extern int rtk_rng_register(void);
extern void rtk_rng_unregister(void);
#endif

int rng_open(struct inode *inode, struct file *file)
{
    return 0;
}

int rng_release(struct inode *inode, struct file *file)
{
    return 0;
}

ssize_t rng_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    unsigned char *tmp_buf = NULL;

    if(!count || count > 0x20000)
        return -EFAULT;

    tmp_buf = (unsigned char *)kmalloc(count, GFP_KERNEL);
    if(!tmp_buf)
        return -ENOMEM;

    if(rtk_rng_get_random_data(tmp_buf, count) == false) {
        kfree(tmp_buf);
        return -EIO;
    }

    if(copy_to_user(buf, tmp_buf, count)) {
        kfree(tmp_buf);
        return -EFAULT;
    }
    kfree(tmp_buf);
    return count;
}

static struct file_operations rng_fops = {
    .owner      = THIS_MODULE,
    .open       = rng_open,
    .read       = rng_read,
    .release    = rng_release,
};

static struct miscdevice rng_miscdev = {
    MISC_DYNAMIC_MINOR,
    "rtk_rng",
    &rng_fops
};

int __init rng_module_init(void)
{
    if (misc_register(&rng_miscdev)) {
        rng_warning("rng_module_init failed - register misc device failed");
        return -ENODEV;
    }
#if 0 //defined(CONFIG_HW_RANDOM)
    rtk_rng_register();
#endif
    return 0;
}

static void __exit rng_module_exit(void)
{
    misc_deregister(&rng_miscdev);
#if 0 //defined(CONFIG_HW_RANDOM)
    rtk_rng_unregister();
#endif
}

module_init(rng_module_init);
module_exit(rng_module_exit);
MODULE_LICENSE("GPL");
