#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sysfs.h>
#include "rtk_irtx.h"
#include "rtk_irtx_dev.h"
#include "rtk_irtx_sysfs.h"

static irtx_dev node_list[MAX_IRTX_CNT];
static dev_t devno_base;
static struct class *irtx_dev_class = NULL;

int create_irtx_dev_node(irtx_device *device)
{
    int i;
    for (i = 0; i < MAX_IRTX_CNT; i++)
    {
        if (node_list[i].device == NULL)
        {
            if (cdev_add(&node_list[i].cdev, devno_base + i, 1) < 0)
            {
                irtx_warn("register character dev failed\n");
                return -1;
            }
            node_list[i].device = device_create(irtx_dev_class, NULL, devno_base + i, device, "irtx-%d",  i);
            if(!node_list[i].device)
            {
                cdev_del(&node_list[i].cdev);
                return -1;
            }
            rtk_irtx_add_sysfs(&(node_list[i].device->kobj));
            return 0;
        }
    }
    return -1;
}

void remove_irtx_dev_node(irtx_device *device)
{
    int i = 0;
    for (i = 0; i < MAX_IRTX_CNT; i++)
    {
        if (node_list[i].device && dev_get_drvdata(node_list[i].device) == (void *)device)
        {
            rtk_irtx_remove_sysfs(&(node_list[i].device->kobj));
            device_destroy(irtx_dev_class, devno_base + i);
            cdev_del(&node_list[i].cdev);
            node_list[i].device = NULL;
            return;
        }
    }
}

static int irtx_dev_open(struct inode *inode, struct file *file)
{
    unsigned int i = iminor(inode);
    irtx_device *dev = NULL;
    irtx_driver *drv = NULL;

    if (i >= MAX_IRTX_CNT || !node_list[i].device)
        return -ENODEV;

    dev = (irtx_device *) dev_get_drvdata(node_list[i].device);
    if (dev)
    {
        if(atomic_add_return(1, &node_list[i].open_cnt) == 1)
        {
            drv = (irtx_driver *) to_irtx_driver(dev->dev.driver);
            drv->enable(dev, 1);
        }
        file->private_data = dev;
        return 0;
    }

    return -ENODEV;
}

static int irtx_dev_release(struct inode *inode, struct file *file)
{
    unsigned int i = iminor(inode);
    irtx_device *dev = NULL;
    irtx_driver *drv = NULL;

    if (i >= MAX_IRTX_CNT || !node_list[i].device)
        return -ENODEV;

    dev = (irtx_device *) dev_get_drvdata(node_list[i].device);
    if (dev)
    {
        if(atomic_sub_return(1, &node_list[i].open_cnt) == 0)
        {
            drv = (irtx_driver *) to_irtx_driver(dev->dev.driver);
            drv->enable(dev, 0);
        }
        file->private_data = NULL;
        return 0;
    }

    return -ENODEV;
}


static long irtx_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

    irtx_device *dev = (irtx_device *) file->private_data;
    irtx_driver *drv = NULL;
    irtx_msg msg;
    IRTX_CMD_BUFF *cmb = NULL;

    if (!dev)
        return -ENODEV;

    drv = (irtx_driver *) to_irtx_driver(dev->dev.driver);

    switch (cmd)
    {
        case IRTX_SEND_MESSAGE:
            if (copy_from_user (&msg, to_user_ptr(arg), sizeof(irtx_msg)))
                return -EFAULT;

            if(!msg.timing_info_len || msg.timing_info_len > MAX_TIMING_INFO_NUM
               || to_user_ptr(msg.timing_info) == NULL)
                return -EFAULT;

            cmb = irtx_alloc_cmb(msg.timing_info_len * IRTX_EACH_TIMING_INFO_LEN);
            if (!cmb)
                return -ENOMEM;

            memcpy(&cmb->cmd, &msg, sizeof(IRTX_CMD));

            if (copy_from_user(cmb->cmd.timing_info, to_user_ptr(msg.timing_info),
                               msg.timing_info_len *IRTX_EACH_TIMING_INFO_LEN))
            {
                irtx_kfree_cmb(cmb);
                return -EFAULT;
            }
            return drv->xmit(dev, cmb, 0);

        default:
            irtx_warn("irtx : unknown ioctl cmd %08x\n", cmd);
            return -EFAULT;
    }
}

#ifdef CONFIG_COMPAT
static long rtk_irtx_compat_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
    void __user *compat_arg = compat_ptr(arg);
    return irtx_dev_ioctl(file, cmd, (unsigned long)compat_arg);
}
#endif


static char *irtx_devnode(struct device *dev, mode_t *mode)
{
    if(mode)
        *mode = 0666;
    return NULL;
}


static struct file_operations irtx_dev_fops =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = irtx_dev_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = rtk_irtx_compat_ioctl,
#endif
    .open = irtx_dev_open,
    .release = irtx_dev_release,
};


static int __init irtx_dev_module_init(void)
{
    int i = 0;
    unsigned int result = 0;

    if (alloc_chrdev_region(&devno_base, 0, MAX_IRTX_CNT, "irtx") != 0)
        return -EFAULT;

    for (i = 0; i < MAX_IRTX_CNT; i++)
    {
        atomic_set(&node_list[i].open_cnt, 0);
        cdev_init(&node_list[i].cdev, &irtx_dev_fops);
        node_list[i].device = NULL;
    }

    irtx_dev_class = class_create(THIS_MODULE, "irtx-dev");
    if (IS_ERR(irtx_dev_class))
    {
        result = PTR_ERR(irtx_dev_class);
        goto ERR_CREATE_CLASS;
    }
    irtx_dev_class->devnode = (void *)irtx_devnode;

    return 0;
ERR_CREATE_CLASS:
    unregister_chrdev_region(devno_base, MAX_IRTX_CNT);
    return result;
}

/*------------------------------------------------------------------
 * Func : irtx_dev_module_exit
 *
 * Desc : irtx dev module exit function
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
static void __exit irtx_dev_module_exit(void)
{
    int i = 0;

    for (i = 0; i < MAX_IRTX_CNT; i++)
    {
        if (node_list[i].device)
            remove_irtx_dev_node((irtx_device *)dev_get_drvdata(node_list[i].device));
    }

    unregister_chrdev_region(devno_base, MAX_IRTX_CNT);
    class_destroy(irtx_dev_class);
}

module_init(irtx_dev_module_init);
module_exit(irtx_dev_module_exit);
