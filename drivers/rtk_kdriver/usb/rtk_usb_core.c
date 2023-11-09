#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <mach/pcbMgr.h>
#include <linux/uaccess.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_gpio.h>
#include "rtk_usb_core.h"

#define DRIVER_AUTHOR "jason.chiu@realtek.com"
#define DRIVER_DESC "RTK USB driver"

#define RTK_USB_DEV_NUM  1

extern int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name);
extern void unregister_chrdev_region(dev_t from, unsigned count);

int rtk_usb_register_notifier(void);
void rtk_usb_unregister_notifier(void);


static u64 usb_connector_map = 0;
static ssize_t g_total_connector_num = 0;
static dev_t rtk_usb_dev_t;
static struct cdev rtk_usb_cdev;
static struct class *rtk_usb_class;
static DEFINE_SPINLOCK(rtk_usb_spinlock);
static CONN_MAP **g_connector_map_p = NULL;

static ssize_t rtk_usb_get_total_connector_number(u64 *conn_map)
{
    int count = 0;
    u64 connector_map = *conn_map;
    while (connector_map) {
        if (connector_map & 0xFF)
            count++;
        connector_map >>= 8;
    }
    return count;
}


static CONN_MAP **rtk_usb_create_map(u64 *conn_map_p)
{
    CONN_MAP **connector_map_p = NULL;
    int i = 0;
    ssize_t total_connector = 0;
    u64 conn_map = *conn_map_p;

    total_connector = rtk_usb_get_total_connector_number(conn_map_p);
    connector_map_p = kzalloc(sizeof(CONN_MAP *) * total_connector, GFP_KERNEL);
    if (!connector_map_p) {
        RTK_USB_ERR("fail to alloc connector_map_p \n");
        return NULL;
    }

    for (i = 0; i < total_connector; i++) {
        connector_map_p[i] = kzalloc(sizeof(CONN_MAP), GFP_KERNEL);
        if (!connector_map_p[i]) {
            RTK_USB_ERR("fail to alloc connector_map_p[%d] \n", i);
            goto FAIL_ALLOC_MEM;
        }
        connector_map_p[i]->connector1 = i + 1;
        connector_map_p[i]->hc_num = GET_HC_NUM(conn_map, i);
        connector_map_p[i]->u2_port1 = GET_U2_PORT1(conn_map, i);
        connector_map_p[i]->u3_port1 = GET_U3_PORT1(conn_map, i);
        RTK_USB_DBG("connector_map_p[%d]->connector1 (%d) \n", i, connector_map_p[i]->connector1);
        RTK_USB_DBG("connector_map_p[%d]->hcd (%d) \n", i, connector_map_p[i]->hc_num);
        RTK_USB_DBG("connector_map_p[%d]->u2_port1 (%d) \n", i, connector_map_p[i]->u2_port1);
        RTK_USB_DBG("connector_map_p[%d]->u3_port1 (%d) \n", i, connector_map_p[i]->u3_port1);
    }

    return connector_map_p;

FAIL_ALLOC_MEM:
    while (i-- >= 0)
        kfree(connector_map_p[i]);

    kfree(connector_map_p);
    return NULL;
}

static void rtk_usb_free_map(void)
{
    int i = 0;

    for (i = 0; i < g_total_connector_num; i++) {
        kfree(g_connector_map_p[i]);
        g_connector_map_p[i] = NULL;
    }
    kfree(g_connector_map_p);
    g_connector_map_p = NULL;
}

static CONN_MAP *rtk_usb_find_conn1_map_port1(CONN_MAP **connector_map_p, int connector1)
{
    if (connector1 < 1 ||
        connector1 > g_total_connector_num ||
        !connector_map_p ||
        !connector_map_p[connector1 - 1])
        return NULL;

    return connector_map_p[connector1 - 1];
}

static int rtk_usb_vbus_control(int connector1, bool on)
{
    u64 pin;
    RTK_GPIO_ID gid;
    char pin_name[30];
    int n = 0, ret = 0;

    if (connector1 < 1 || connector1 > g_total_connector_num)
        return -ENODEV;

    n = snprintf(pin_name, sizeof(pin_name), "PIN_USB_5V_EN_%d", connector1);
    if (n < 0 || n >= sizeof(pin_name)) // write fail.
        return n < 0 ? n : -ENOMEM;

    ret = pcb_mgr_get_enum_info_byname(pin_name, &pin);
    if (ret) {
        RTK_USB_ERR("fail: get PCB parameter (%s) \n", pin_name);
        return -ENODEV;
    }

    gid = rtk_gpio_id(GET_GPIO_TYPE(pin), GET_PIN_INDEX(pin));
    ret = rtk_gpio_output(gid, on);
    if (ret)
        return ret;
    ret = rtk_gpio_set_dir(gid, 1);  // set gpio to output mode
    RTK_USB_INFO("set connector1(%d) 5V en(%d), ret=%d\n", connector1, on, ret);

    return ret;
}

static int __attribute__((unused)) rtk_usb_super_speed_support(CONN_MAP **connector_map_p, int connector1, bool on)
{
    return 0;
}


static int rtk_usb_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int rtk_usb_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t __attribute((unused)) rtk_usb_read(struct file *filp, char __user *buffer, size_t count, loff_t *f_ops)
{
    return count;
}

static ssize_t __attribute((unused)) rtk_usb_write(struct file *filp, const char __user *buffer, size_t count, loff_t *f_ops)
{
    return count;
}

static long rtk_usb_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    RTK_USB_INFO usb_info;
    VBUS_INFO *vbus_inf;
    CONN_MAP conn_map;
    CONN_MAP* p_conn_map = NULL;

    memset(&usb_info, 0, sizeof(usb_info));

    spin_lock(&rtk_usb_spinlock);
    switch (cmd) {
    case RTK_USB_IOC_GET_TOTAL_CONN_NUM:
        RTK_USB_DBG("ioctl: RTK_USB_IOC_GET_TOTAL_CONN_NUM \n");
        usb_info.total_conn_num = g_total_connector_num;
        if (copy_to_user((void __user *)arg, (const void *)&usb_info, sizeof(usb_info)))
            ret = -EFAULT;
        break;
    case RTK_USB_IOC_GET_MAPPED_PORT:
        RTK_USB_DBG("ioctl: RTK_USB_IOC_GET_MAPPED_PORT \n");
        memset(&conn_map, 0, sizeof(conn_map));
        if (copy_from_user((void *)&conn_map, (const void __user *)arg, sizeof(conn_map))) {
            ret = -EFAULT;
            break;
        }
        p_conn_map = rtk_usb_find_conn1_map_port1(g_connector_map_p, conn_map.connector1);
        if(p_conn_map){
            conn_map = *p_conn_map;
            if (copy_to_user((void __user *)arg, (const void *)&conn_map, sizeof(conn_map)))
                ret = -EFAULT;
        }else{
            ret = -EFAULT;
        }
        break;
    case RTK_USB_IOC_SET_SS_SUPPORT:
        RTK_USB_DBG("ioctl: RTK_USB_IOC_SET_SS_SUPPORT \n");
        break;
    case RTK_USB_IOC_SET_VBUS:
        RTK_USB_DBG("ioctl: RTK_USB_IOC_SET_VBUS \n");
        if (copy_from_user((void *)&usb_info, (const void __user *)arg, sizeof(usb_info))) {
            ret = -EFAULT;
        } else {
            vbus_inf = &usb_info.port_info.vbus_info;
            ret = rtk_usb_vbus_control(vbus_inf->connector1, vbus_inf->on);
        }
        break;
    case RTK_USB_IOC_GET_OCD_STATUS:
        RTK_USB_DBG("ioctl: RTK_USB_IOC_GET_OCD_STATUS \n");
        break;
    default:
        RTK_USB_WARN("Unknow commnad \n");
    }
    spin_unlock(&rtk_usb_spinlock);

    return ret;
}
static long rtk_usb_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    return 0;
}

struct file_operations rtk_usb_fops = {
    .owner            = THIS_MODULE,
    .open             = rtk_usb_open,
    .release          = rtk_usb_release,
#if 0
    .read             = rtk_usb_read,
    .write            = rtk_usb_write,
#endif
    .unlocked_ioctl   = rtk_usb_ioctl,
    .compat_ioctl     = rtk_usb_compat_ioctl,
};


static int __init rtk_usb_cdev_init(void)
{
    int ret = 0;

    ret = alloc_chrdev_region(&rtk_usb_dev_t, 0, RTK_USB_DEV_NUM, "rtk_usb");
    if (ret) {
        RTK_USB_ERR("fail to get char dev Major and Minor \n");
        goto FAIL_ALLOC_CHRDEV_MAJOR;
    }

    cdev_init(&rtk_usb_cdev, &rtk_usb_fops);

    ret = cdev_add(&rtk_usb_cdev, rtk_usb_dev_t, RTK_USB_DEV_NUM);
    if (ret) {
        RTK_USB_ERR("fail to add char dev to system \n");
        goto FAIL_ADD_CHRDEV;
    }

    rtk_usb_class = class_create(THIS_MODULE, "rtk_usb");
    if (IS_ERR(rtk_usb_class)) {
        RTK_USB_ERR("fail to cearte class \n");
        ret = PTR_ERR(rtk_usb_class);
        goto FAIL_CREATE_CLASS;
    }

    if (!device_create(rtk_usb_class, NULL, rtk_usb_cdev.dev, NULL, "rtk_usb")) {
        RTK_USB_ERR("fail to cearte device node for rtk_usb \n");
        ret = -ENOMEM;
        goto FAIL_CREATE_DEVICE;
    }

    return ret;

FAIL_CREATE_DEVICE:
    class_destroy(rtk_usb_class);
    rtk_usb_class = NULL;
FAIL_CREATE_CLASS:
    cdev_del(&rtk_usb_cdev);
FAIL_ADD_CHRDEV:
    unregister_chrdev_region(rtk_usb_dev_t, RTK_USB_DEV_NUM);
FAIL_ALLOC_CHRDEV_MAJOR:
    return ret;
}

static void rtk_usb_cdev_uninit(void)
{
    device_destroy(rtk_usb_class, rtk_usb_cdev.dev);
    class_destroy(rtk_usb_class);
    rtk_usb_class = NULL;
    cdev_del(&rtk_usb_cdev);
    unregister_chrdev_region(rtk_usb_dev_t, RTK_USB_DEV_NUM);
}

// TODO: Add any attribute file you want here.
static int rtk_usb_sysfs_init(void)
{
    int ret = 0;
    return ret;
}

static void rtk_usb_sysfs_uninit(void)
{
}

int __init rtk_usb_init(void)
{
    int ret = 0;

    ret = rtk_usb_cdev_init();
    if (ret)
        goto FAIL_CDEV_INIT;

    ret = rtk_usb_sysfs_init();
    if (ret) {
        RTK_USB_ERR("fail: rtk_usb_sysfs_init \n");
        goto FAIL_INIT_SYSFS;
    }

    ret = pcb_mgr_get_enum_info_byname("USB_CONNECTOR_MAP", &usb_connector_map);
    if (ret) {
        RTK_USB_ERR("fail: get PCB parameter (USB_CONNECTOR_MAP) \n");
        goto FAIL_GET_PCB_PARAM;
    }

    g_total_connector_num = rtk_usb_get_total_connector_number(&usb_connector_map);

    g_connector_map_p = rtk_usb_create_map(&usb_connector_map);
    if (!g_connector_map_p) {
        ret = -ENOMEM;
        RTK_USB_ERR("fail: rtk_usb_create_map. error(%d) \n", ret);
        goto FAIL_GET_PCB_PARAM;
    }
 
    if ((ret = rtk_usb_register_notifier())) {
        RTK_USB_ERR("fail to register notifier, error(%d) \n", ret);
        goto FAIL_REGISTER_NOTIFIER;
    }

    return ret;
FAIL_REGISTER_NOTIFIER:
    rtk_usb_free_map();
FAIL_GET_PCB_PARAM:
    rtk_usb_sysfs_uninit();
FAIL_INIT_SYSFS:
    rtk_usb_cdev_uninit();
FAIL_CDEV_INIT:
    return ret;
}


void rtk_usb_exit(void)
{
    rtk_usb_unregister_notifier();
    rtk_usb_free_map();
    rtk_usb_sysfs_uninit();
    rtk_usb_cdev_uninit();
}

