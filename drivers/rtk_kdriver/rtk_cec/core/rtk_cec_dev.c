/*Copyright (C) 2007-2013 Realtek Semiconductor Corporation.*/
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched/clock.h>
#endif
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>    /* copy_*_user */
#include <linux/sysfs.h>
#include <rtk_kdriver/io.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <rbus/pll_reg_reg.h>
#include "rtk_cec.h"
#include "rtk_cec_dev.h"
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#define to_user_ptr(x)          compat_ptr((unsigned int ) x)
#else
#define to_user_ptr(x)          ((void* __user)(x)) // convert 32 bit value to user pointer
#endif


#define cec_attr(_name) \
static struct kobj_attribute _name##_attr = {   \
        .attr   = {                                               \
                .name = __stringify(_name),            \
                .mode = 0644,                                \
        },                                                          \
        .show   = _name##_show,                      \
        .store  = _name##_store,                       \
}

#define MAX_CEC_CNT             4
static cec_dev node_list[MAX_CEC_CNT];
static dev_t devno_base;
static struct class *cec_dev_class;
unsigned int g_cec_debug_enable = 0;
static unsigned int cec_record_en;
static cec_recorder recorder;

extern void rtd_cec_dump_current_setting(char *buf, int in_standby);

extern int __init rtd_cec_module_init(void);
extern void __exit rtd_cec_module_exit(void);
extern int __init cec_core_init(void);
extern void __exit cec_core_exit(void);


static void cec_recorder_init(void)
{
    mutex_init(&recorder.free_queue_lock);
    cmb_queue_head_init(&recorder.free_queue);
    mutex_init(&recorder.record_queue_lock);
    cmb_queue_head_init(&recorder.record_queue);
}

static void cec_recorder_uninit(void)
{
    cm_buff *cmb = NULL;
    mutex_lock(&recorder.record_queue_lock);
    while ((cmb = cmb_dequeue(&recorder.record_queue)))
        kfree_cmb(cmb);
    mutex_unlock(&recorder.record_queue_lock);
    mutex_lock(&recorder.free_queue_lock);
    while ((cmb = cmb_dequeue(&recorder.free_queue)))
        kfree_cmb(cmb);
    mutex_unlock(&recorder.free_queue_lock);
}

static void cec_pre_allocate_free_cmb(int num)
{
    while(num-- >= 0) {
        cm_buff *cmb = NULL;
        cmb = alloc_cmb(RX_BUFFER_SIZE);
        if(cmb) {
            mutex_lock(&recorder.free_queue_lock);
            cmb_queue_tail(&recorder.free_queue, cmb);
            mutex_unlock(&recorder.free_queue_lock);
        }
    }
}

static void cec_push_queue(cm_buff_head *queue, struct mutex *lock, cm_buff *cmb)
{
    if(!cmb || !queue || !lock)
        return;
    mutex_lock(lock);
    cmb_queue_tail(queue, cmb);
    mutex_unlock(lock);
}

static cm_buff * cec_pop_queue(cm_buff_head *queue, struct mutex *lock)
{
    cm_buff *cmb;
    if(!queue || !lock)
        return NULL;
    mutex_lock(lock);
    cmb = cmb_dequeue(queue);
    mutex_unlock(lock);
    return cmb;
}


static void cec_push_recorder_msg(cm_buff *cmb, unsigned int tx)
{
    cm_buff * free_cmb;
    if(!cec_record_en)
        return;
    if(!cmb || cmb->len == 0)
        return;
    free_cmb = cec_pop_queue(&recorder.free_queue, &recorder.record_queue_lock);
    if(!free_cmb)
        return;
    cmb_purge(free_cmb);
    if(tx)
        free_cmb->flags |= TX_MSG;
    else
        free_cmb->flags &= (~TX_MSG);
    memcpy(cmb_put(free_cmb, cmb->len), cmb->data, cmb->len);
    cec_push_queue(&recorder.record_queue,
                   &recorder.record_queue_lock, free_cmb);
}


/*------------------------------------------------------------------
 * Func : create_cec_dev_node
 *
 * Desc : create a cec device node
 *
 * Parm : device : handle of cec device
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int create_cec_dev_node(cec_device *device)
{
    int i;
    for (i = 0; i < MAX_CEC_CNT; i++) {
        if (node_list[i].device == NULL) {
            /*register character device node to system*/
            if (cdev_add(&node_list[i].cdev, devno_base + i, 1) < 0) {
                cec_warn
                (" register character dev failed\n");
                return -1;
            }

            node_list[i].device = device_create(cec_dev_class,  /* class */
                                                NULL,   /* parent */
                                                devno_base + i, /* dev number */
                                                device, /* driver data */
                                                "cec-%d",   /* device name */
                                                i); /* device id */

            return 0;
        }
    }

    return -1;
}

/*------------------------------------------------------------------
 * Func : remove_cec_dev_node
 *
 * Desc : remove a cec device node
 *
 * Parm : device : cec device to be unregistered
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
void remove_cec_dev_node(cec_device *device)
{
    int i = 0;

    for (i = 0; i < MAX_CEC_CNT; i++) {
        if (node_list[i].device
            && dev_get_drvdata(node_list[i].device) == (void *)device) {
            device_destroy(cec_dev_class, devno_base + i);  /* remove device */
            cdev_del(&node_list[i].cdev);
            node_list[i].device = NULL;
            return;
        }
    }
}

/*------------------------------------------------------------------
 * Func : cec_dev_open
 *
 * Desc : open function of cec dev
 *
 * Parm : inode : inode of dev
 *        file  : context of file
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
static
int cec_dev_open(struct inode *inode, struct file *file)
{
    unsigned int i = iminor(inode);
    cec_device *dev;

    if (i >= MAX_CEC_CNT || !node_list[i].device)
        return -ENODEV;

    dev = (cec_device *) dev_get_drvdata(node_list[i].device);

    if (dev) {
        /*cec_driver* drv = (cec_driver*) to_cec_driver(dev->dev.driver);*/
        file->private_data = dev;
        return 0;
    }

    return -ENODEV;
}

/*------------------------------------------------------------------
 * Func : cec_dev_release
 *
 * Desc : release function of cec dev
 *
 * Parm : inode : inode of dev
 *        file  : context of file
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
static
int cec_dev_release(struct inode *inode, struct file *file)
{
    unsigned int i = iminor(inode);
    cec_device *dev;
    cec_driver *drv;

    if (i < MAX_CEC_CNT && node_list[i].device) {
        dev = (cec_device *) dev_get_drvdata(node_list[i].device);
        drv = (cec_driver *) to_cec_driver(dev->dev.driver);
        drv->enable(dev, 0xf0);
    }

    return 0;
}

/*------------------------------------------------------------------
 * Func : cec_dev_ioctl
 *
 * Desc : ioctl function of cec dev
 *
 * Parm : inode : inode of dev
 *        file  : context of file
 *        cmd   : control command
 *        arg   : arguments
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
static
long cec_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

    cec_device *dev = (cec_device *) file->private_data;
    cec_driver *drv;
    cec_msg msg = {0};
    cm_buff *cmb = NULL;
    unsigned char len = 0;
    unsigned short rx_mask;


    if (!dev)
        return -ENODEV;

    drv = (cec_driver *) to_cec_driver(dev->dev.driver);

    switch (cmd) {
        case CEC_ENABLE:
            return drv->enable(dev, (arg) ? 1 : 0);

        case CEC_SET_RX_MASK:   /* broadcast is always available here... */
            return drv->set_rx_mask(dev, (unsigned short)arg | 0x8000);

        case CEC_SET_LOGICAL_ADDRESS:

            rx_mask = (unsigned char)arg;
            if (rx_mask > 0xF) {
                cec_warn("invalid logical address %d\n", rx_mask);
                return -EFAULT;
            }
            rx_mask = (1 << rx_mask);
            return drv->set_rx_mask(dev, rx_mask | 0x8000);

        case CEC_SET_PHYSICAL_ADDRESS:
            return drv->set_physical_addr(dev, (unsigned short)arg);

        case CEC_SEND_MESSAGE:

            if (copy_from_user (&msg, to_user_ptr(arg), sizeof(cec_msg)))
                return -EFAULT;

            if(msg.len ==0 || msg.len > 64 )
                return -EFAULT;

            // KWarning: checked ok by liangliang_song@apowertec.com
            cmb = alloc_cmb(msg.len);
            if (cmb == NULL)
                return -ENOMEM;

            if (copy_from_user(cmb_put(cmb, msg.len),to_user_ptr(msg.buf),msg.len))
                return -EFAULT;

            cec_push_recorder_msg(cmb, 1);
            return drv->xmit(dev, cmb, 0);  /* BLOCK I/O */

        case CEC_RCV_MESSAGE:

            if (copy_from_user(&msg, to_user_ptr(arg), sizeof(cec_msg)))
                return -EFAULT;

            cmb = drv->read(dev, (file->f_flags & O_NONBLOCK) ? NONBLOCK : 0);  /* BLOCK I/O */

            if (cmb) {
                if (cmb->len > msg.len) {
                    cec_warn("cec : read message failed, msg size (%d) more than msg buffer size(%d)\n",
                     cmb->len, msg.len);
                    kfree_cmb(cmb);
                    return -ENOMEM;
                }

                len = cmb->len;

                if (copy_to_user(to_user_ptr(msg.buf),cmb->data, cmb->len))
                {
                    cec_warn("cec : read message failed, copy msg to user failed\n");
                    kfree_cmb(cmb);
                    return -EFAULT;
                }

                cec_push_recorder_msg(cmb, 0);
                kfree_cmb(cmb);
                return len;
            }

            return -EFAULT;

        case CEC_SET_STANDBY_MODE:

            return drv->set_stanby_mode(dev, (unsigned long)arg);

        case CEC_SET_CEC_VERSION:

            return drv->set_cec_version(dev, (unsigned char)arg);

        case CEC_SET_DEVICE_VENDOR_ID:

            return drv->set_device_vendor_id(dev, (unsigned long)arg);

        case CEC_SET_RETRY_NUM:

            return drv->set_retry_num(dev, (unsigned long)arg);

        case CEC_SET_HOTPLUG:
            return drv->set_cec_hotplug(dev, (unsigned long)arg);

        default:
            cec_warn("cec : unknown ioctl cmd %08x\n", cmd);
            return -EFAULT;
    }
}

#ifdef CONFIG_COMPAT
static long rtk_cec_compat_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
    void __user *compat_arg = compat_ptr(arg);
    return cec_dev_ioctl(file, cmd, (unsigned long)compat_arg);
}
#endif

static char *cec_devnode(struct device *dev, umode_t *mode)
{
    if(mode)
        *mode = 0666;
    return NULL;
}



static struct file_operations cec_dev_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = cec_dev_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = rtk_cec_compat_ioctl,
#endif
    .open = cec_dev_open,
    .release = cec_dev_release,
};


static ssize_t cec_dbg_en_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "cec_tx_dbg_en = %s\n", g_cec_debug_enable ? "enable" : "disable");
}

static ssize_t cec_dbg_en_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val;
    if (kstrtouint(buf, 10, &val) == 0) {
        g_cec_debug_enable = val;
        return n;
    }
    return -EINVAL;
}
cec_attr(cec_dbg_en);

static unsigned char last_timing_name[32] = {0};
static ssize_t cec_timing_setting_show(struct kobject * kobj,
                                       struct kobj_attribute * attr, char *buf)
{
    unsigned int val = 0;
    cec_device *cec_dev;
    cec_driver *drv;

    if (!node_list[0].device)
        return -ENODEV;

    cec_dev = (cec_device *) dev_get_drvdata(node_list[0].device);
    drv = (cec_driver *) to_cec_driver(cec_dev->dev.driver);

    if(drv->get_timing && drv->get_timing(cec_dev, last_timing_name, &val) == 0)
        return sprintf(buf, "the current setting of %s is %d(us)\n", last_timing_name, val);

    return EINVAL;
}

static ssize_t cec_timing_setting_store(struct kobject * kobj,
                                        struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned char action[4];
    cec_device *cec_dev;
    cec_driver *drv;


    memset(action, 0, 4);


    if (!node_list[0].device)
        return -ENODEV;

    cec_dev = (cec_device *) dev_get_drvdata(node_list[0].device);
    drv = (cec_driver *) to_cec_driver(cec_dev->dev.driver);

    if (sscanf(buf, "%3s", action) == 1) {
        cec_dbg( "cec_timing_setting_store:buf == %s, action == %s\n", buf, action);
        if(strcmp(action, "set") == 0) {
            unsigned int val = 0;
            char timing_name[32] = {0};
            buf += 4;
            if (sscanf(buf, "%31s%u", timing_name, &val) == 2) {
                cec_dbg( "cec_timing_setting_store: set timing_name =%s val =%u\n", timing_name, val);
                if(drv->set_timing && drv->set_timing(cec_dev, timing_name, val) == 0)
                    return n;
                else
                    return -EINVAL;
            }
        } else if(strcmp(action, "get") == 0) {

            buf += 4;
            if (sscanf(buf, "%31s", last_timing_name) == 1) {
                cec_dbg( "cec_timing_setting_store:get attr_name = %s\n", last_timing_name);
                return n;
            } else {
                return -EINVAL;
            }
        }
    }
    return -EINVAL;
}
cec_attr(cec_timing_setting);

static ssize_t cec_retry_num_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return 0;
}

static ssize_t cec_retry_num_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val;
    cec_device *cec_dev;
    cec_driver *drv;

    if (!node_list[0].device)
        return -ENODEV;

    cec_dev = (cec_device *) dev_get_drvdata(node_list[0].device);
    drv = (cec_driver *) to_cec_driver(cec_dev->dev.driver);

    if (sscanf(buf, "%u", &val) == 1)
    {
        if(drv->set_retry_num(cec_dev, val) == 0)
            return n;
    }

    return -EINVAL;
}
cec_attr(cec_retry_num);


static ssize_t cec_record_on_off_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    return sprintf(buf, "cec_record_en = %s\n", cec_record_en ? "enable" : "disable");
}

static ssize_t cec_record_on_off_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    unsigned int val;

    if (sscanf(buf, "%u", &val) == 1)
    {

        if(val)
        {
            if(!cec_record_en)
            {
                cec_record_en = 1;
                cec_pre_allocate_free_cmb(512);
            }
        }
        else
        {
            if(cec_record_en)
            {
                cec_record_en = 0;
                cec_recorder_uninit();
            }
        }

        return n;
    }

    return -EINVAL;
}
cec_attr(cec_record_on_off);

static ssize_t cec_read_record_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{

    buf[0] = 0;

    while(strlen(buf) < PAGE_SIZE - 64)
    {
        cm_buff * cmb = cec_pop_queue(&recorder.record_queue,
                                      &recorder.record_queue_lock);
        if(cmb)
        {
            int i = 0;
            u64 ts = local_clock();

            unsigned long rem_nsec = do_div(ts, 1000000000);

            sprintf(buf + strlen(buf), "[%5lu.%06lu]%s msg:", (unsigned long)ts,
                    rem_nsec / 1000, (cmb->flags & TX_MSG) ? "tx" : "rx");

            for(i = 0; i < cmb->len && i < 16; i++)
                sprintf(buf + strlen(buf), "0x%02x ", cmb->data[i]);

            sprintf(buf + strlen(buf), "\n");

            cec_push_queue(&recorder.free_queue,
                           &recorder.free_queue_lock, cmb);
        }
        else
        {
            break;
        }
    }

    return strlen(buf);
}

static ssize_t cec_read_record_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    return -EINVAL;
}
cec_attr(cec_read_record);

static ssize_t cec_dump_current_setting_show(struct kobject * kobj, struct kobj_attribute * attr, char *buf)
{
    rtd_cec_dump_current_setting(buf, 0);
    return strlen(buf);
}

static ssize_t cec_dump_current_setting_store(struct kobject * kobj, struct kobj_attribute * attr, const char *buf, size_t n)
{
    return -EINVAL;
}
cec_attr(cec_dump_current_setting);


static ssize_t cec_msg_out_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "echo <device LA> <op code> <data> > cec_msg_out\nex. echo 05 83\n");;
}


int hexRmSpace2bin(u8 *dst, const char *src, size_t count)
{
    int len = 0;
    while (count--) {
        int hi, lo;
        if ((*src) == ' ') {
            src ++;
            continue;
        }
        hi = hex_to_bin(*src++);
        lo = hex_to_bin(*src++);
        if ((hi < 0) || (lo < 0))
            return len;
        *dst++ = (hi << 4) | lo;
        len ++;
    }
    return len;
}

static ssize_t cec_msg_out_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
    cm_buff * msg = NULL;
    int len;
    cec_device *cec_dev;
    cec_driver *drv;

    if (!node_list[0].device)
        return -ENODEV;

    msg = alloc_cmb(n);
    if (msg == NULL)
        return -ENOMEM;
    len = hexRmSpace2bin(msg->data, buf, n);
    if (len < 0)
        return -EINVAL;
    msg->len = len;

    cec_dev = (cec_device *) dev_get_drvdata(node_list[0].device);
    drv = (cec_driver *) to_cec_driver(cec_dev->dev.driver);
    /* xmit will free msg when block mode */
    drv->xmit(cec_dev, msg, 0);

    return n;
}
cec_attr(cec_msg_out);

static struct attribute * cec_attrs[] = {
    &cec_dbg_en_attr.attr,
    &cec_timing_setting_attr.attr,
    &cec_retry_num_attr.attr,
    &cec_record_on_off_attr.attr,
    &cec_read_record_attr.attr,
    &cec_dump_current_setting_attr.attr,
    &cec_msg_out_attr.attr,
    NULL,
};

static struct attribute_group cec_attr_group = {
    .attrs = cec_attrs,
};

static struct kobject *cec_kobj;

/*------------------------------------------------------------------
 * Func : cec_dev_module_init
 *
 * Desc : cec dev init function
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
static int __init cec_dev_module_init(void)
{
    int i;
    unsigned int result = 0;

    if (alloc_chrdev_region(&devno_base, 0, MAX_CEC_CNT, "cec") != 0)
        return -EFAULT;

    for (i = 0; i < MAX_CEC_CNT; i++) {
        cdev_init(&node_list[i].cdev, &cec_dev_fops);
        node_list[i].device = NULL;
    }

    cec_dev_class = class_create(THIS_MODULE, "cec-dev");   /* create a new class i2c-dev */

    if (IS_ERR(cec_dev_class)) {
        result = PTR_ERR(cec_dev_class);
        goto err_create_class_failed;
    }
    cec_dev_class->devnode = cec_devnode;
    cec_kobj = kobject_create_and_add("cec", NULL);
    if (!cec_kobj) {
        result = -ENOMEM;
        goto err_create_kobject_failed;
    }

    result = sysfs_create_group(cec_kobj, &cec_attr_group);
    if (result != 0) {
        result = -ENOMEM;
        goto err_sysfs_create_group_failed;
    }
    cec_recorder_init();
    cec_core_init();
    rtd_cec_module_init();

    return 0;
err_sysfs_create_group_failed:
    kobject_put(cec_kobj);
err_create_kobject_failed:
    class_destroy(cec_dev_class);
err_create_class_failed:
    unregister_chrdev_region(devno_base, MAX_CEC_CNT);
    return result;
}

/*------------------------------------------------------------------
 * Func : cec_dev_module_exit
 *
 * Desc : cec dev module exit function
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
static void __exit cec_dev_module_exit(void)
{
    int i = 0;

    for (i = 0; i < MAX_CEC_CNT; i++) {
        if (node_list[i].device) {
            remove_cec_dev_node((cec_device *)
                                dev_get_drvdata(node_list
                                                [i].device));
        }
    }

    unregister_chrdev_region(devno_base, MAX_CEC_CNT);
    class_destroy(cec_dev_class);
    kobject_put(cec_kobj);
    cec_recorder_uninit();

    cec_core_exit();
    rtd_cec_module_exit();
}

module_init(cec_dev_module_init);
module_exit(cec_dev_module_exit);
