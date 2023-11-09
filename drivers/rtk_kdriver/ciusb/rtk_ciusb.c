/*
*
* Copyright (C) 2022, Realtek Semiconductor Corp.
* All Rights Reserved.
*
*/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/dirent.h>
#include <linux/time.h>
#include "rtk_ciusb.h"

#define COLLECT_STATISTIC_DATA          0
#define CIUSB_MEDIA_READ_TIMEOUT        12      /* 250Hz, wait 48 ms */
#define CIUSB_MEDIA_WRITE_TIMEOUT       12      /* 250Hz, wait 48 ms */
#define CIUSB_COM_READ_TIMEOUT          500     /* 250Hz, wait 2000 ms */
#define CIUSB_COM_WRITE_TIMEOUT         500     /* 250Hz, wait 2000 ms */

/* Define these values to match your devices */
#define CIUSB_INTERFACE_CLASS_CODE      0xEF
#define CIUSB_INTERFACE_SUBCLASS_CODE   0x07
#define CIUSB_CMD_PROTOCOL_CODE         0x01
#define CIUSB_MEDIA_PROTOCOL_CODE       0x02
#define CIUSB_COMM_CLASS_CODE           0x02
#define CIUSB_COMM_SUBCLASS_CODE        0x0C
#define CIUSB_COMM_PROTOCOL_CODE        0x07

typedef struct {
    char *buf;
    dma_addr_t dma;
    bool used;
} buf_write_st;

static int g_Status=0;
static struct class *ciplus_class=NULL;
static char ciplus_device_name[CIUSB_INTF_NUM][16] = {
    CIUSB_DEVICE_COMMAND_NAME,
    CIUSB_DEVICE_MEDIA_NAME,
    CIUSB_DEVICE_COMM_NAME,
};

/* table of devices that work with this driver */
static const struct usb_device_id ciplus_table[] = {
    { USB_INTERFACE_INFO(CIUSB_INTERFACE_CLASS_CODE, CIUSB_INTERFACE_SUBCLASS_CODE, CIUSB_CMD_PROTOCOL_CODE) },
    { USB_INTERFACE_INFO(CIUSB_INTERFACE_CLASS_CODE, CIUSB_INTERFACE_SUBCLASS_CODE, CIUSB_MEDIA_PROTOCOL_CODE) },
    { USB_INTERFACE_INFO(CIUSB_COMM_CLASS_CODE, CIUSB_COMM_SUBCLASS_CODE, CIUSB_COMM_PROTOCOL_CODE) },
    { }                /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, ciplus_table);

static const unsigned char fragment_header[] = {
   0x00,//protocol_version    8b
   0x47,//LTS_id              8b
   0x00,//track_id            8b
        //flush               1b
        //first_fragment      1b
        //last_fragment       1b
   0x00,//reserved_future_use 5b
   0x00,
   0x00,
   0x00,
   0x00,//number_subsamples   32b
   0x00,
   0x00,//descriptor_length   16b
};

/* Get a minor range for your devices from the usb maintainer */
#define USB_CIP_MINOR_BASE      192

/* our private defines. if this grows any larger, use your own .h file */
//#define MAX_TRANSFER            (PAGE_SIZE - 512)
/* MAX_TRANSFER is chosen so that the VM is not stressed by
    allocations > PAGE_SIZE and the number of packets in a page
    is an integer 512 is the largest possible packet on EHCI */
// In TS 103 605, for USB 3 the maximum endpoint size is 1024.
#define WRITES_IN_FLIGHT                16
#define MEDIA_BULK_IN_SIZE              (32*1024)
#define MEDIA_BULK_OUT_SIZE             (6*1024)
/* arbitrarily chosen */

/* Structure to hold all of our device specific stuff */
typedef struct {
    struct usb_device *udev;                /* the usb device for this device */
    struct usb_interface *interface;        /* the interface for this device */
    struct semaphore  limit_sem;            /* limiting the number of writes in progress */
    struct usb_anchor submitted;            /* in case we need to retract our submissions */
    struct urb     *bulk_in_urb;            /* the urb to read data with */
    unsigned char  *bulk_in_buffer;         /* the buffer to receive data */
    size_t          bulk_in_size;           /* the size of the receive buffer */
    size_t          bulk_in_filled;         /* number of bytes in the buffer */
    size_t          packet_size_w;
    size_t          packet_size_r;
    size_t          bulk_in_copied;         /* already copied to user space */
    __u8            bulk_in_endpointAddr;   /* the address of the bulk in endpoint */
    __u8            bulk_out_endpointAddr;  /* the address of the bulk out endpoint */
    size_t          bulk_out_size;          /* the size of the send buffer */
    int             errors;                 /* the last request tanked */
    bool            ongoing_read;           /* a read is going on */
    spinlock_t      err_lock;               /* lock for errors */
    struct mutex    rd_mutex;               /* no concurrent readers, synchronize I/O with disconnect */
    struct mutex    wr_mutex;               /* no concurrent writers, synchronize I/O with disconnect */
    unsigned long	connected;
    bool            opened;
    wait_queue_head_t bulk_in_wait;         /* to wait for an ongoing read */

    int dev_major;
    int dev_minor;
    struct cdev cdev;
    int interface_idx;
    buf_write_st buf_w[WRITES_IN_FLIGHT];
    int read_timeout;
    int write_timeout;
    bool return_FH;
    unsigned long long time_read_wait;
    unsigned long long time_write_wait;
    bool collect_statistic;
}usb_ciplus;

static usb_ciplus usb_ciplus_device[CIUSB_INTF_NUM];

static struct usb_driver ciplus_driver;
static void ciplus_draw_down(usb_ciplus *dev);

static char *ciplus_alloc_buf_w(usb_ciplus *dev, dma_addr_t *dma)
{
    int i;
    char *buf=NULL;

    for (i=0; i<WRITES_IN_FLIGHT;i++) {
        if ((dev->buf_w[i].used == 0) && (dev->buf_w[i].buf != 0)) {
            buf = dev->buf_w[i].buf;
            *dma = dev->buf_w[i].dma;
            dev->buf_w[i].used = 1;
            break;
        }
    }

    return buf;
}

static void ciplus_free_buf_w(usb_ciplus *dev, char *buf)
{
    int i;

    for (i=0; i<WRITES_IN_FLIGHT;i++) {
        if ((dev->buf_w[i].used == 1) && (dev->buf_w[i].buf == buf)) {
            dev->buf_w[i].used = 0;
            break;
        }
    }
}

static void ciplus_delete(usb_ciplus *dev)
{
    int i;

    if (dev->bulk_in_urb) {
        usb_free_urb(dev->bulk_in_urb);
        dev->bulk_in_urb = NULL;
    }
    if (dev->interface) {
        usb_put_intf(dev->interface);
        dev->interface = NULL;
    }
    if (dev->udev) {
        usb_put_dev(dev->udev);
        dev->udev = NULL;
    }
    if (dev->bulk_in_buffer) {
        kfree(dev->bulk_in_buffer);
        dev->bulk_in_buffer = NULL;
    }

    for (i=0; i<WRITES_IN_FLIGHT; i++) {
        if (dev->buf_w[i].buf) {
            usb_free_coherent(dev->udev, dev->bulk_out_size,
                              dev->buf_w[i].buf, dev->buf_w[i].dma);
        }
    }
    memset(&dev->buf_w[0], 0, sizeof(buf_write_st)*WRITES_IN_FLIGHT);
}

static int ciplus_open(struct inode *inode, struct file *file)
{
    usb_ciplus *dev;
    struct usb_interface *interface;
    int major;
    int retval = 0;
    int i;

    major = imajor(inode);

    for (i=0; i<CIUSB_INTF_NUM; i++) {
        if (usb_ciplus_device[i].dev_major == major) {
            break;
        }
    }

    if (i >= CIUSB_INTF_NUM) {
        pr_err("%s - cannot find [%d]\n", __func__, major);
        goto exit;
    }
    dev = &usb_ciplus_device[i];
    interface = dev->interface;

    if (dev->opened) {
        pr_err("%s - opened!!!\n", __func__);
        retval = -EFAULT;
        goto exit;
    }

    if (interface) {
        retval = usb_autopm_get_interface(interface);
        if (retval)
            goto exit;
    }

    /* save our object in the file's private structure */
    file->private_data = dev;
    dev->opened = 1;

exit:
    return retval;
}

static int ciplus_release(struct inode *inode, struct file *file)
{
    usb_ciplus *dev;

    dev = file->private_data;
    if (dev == NULL)
       return -ENODEV;

    /* allow the device to be autosuspended */
    if (dev->interface)
        usb_autopm_put_interface(dev->interface);

    dev->opened = 0;
    return 0;
}

static int ciplus_flush(struct file *file, fl_owner_t id)
{
    usb_ciplus *dev;
    int res;

    dev = file->private_data;
    if (dev == NULL)
       return -ENODEV;

    /* wait for io to stop */
    mutex_lock(&dev->rd_mutex);
    mutex_lock(&dev->wr_mutex);
    ciplus_draw_down(dev);

    /* read out errors, leave subsequent opens a clean slate */
    spin_lock_irq(&dev->err_lock);
    res = dev->errors ? (dev->errors == -EPIPE ? -EPIPE : -EIO) : 0;
    dev->errors = 0;
    spin_unlock_irq(&dev->err_lock);

    mutex_unlock(&dev->wr_mutex);
    mutex_unlock(&dev->rd_mutex);

    return res;
}

static int ciplus_ioctl_get_status(usb_ciplus *dev, unsigned long args)
{
    int ret=0;
    int status=0;

    if (args == 0) {
        ret = -EFAULT;
    }
    else {
        if (dev->connected) {
            status |= RTK_CIUSB_STS_CNNECTED;
        }
        if (g_Status & RTK_CIUSB_STS_AFTER_STR) {
            status |= RTK_CIUSB_STS_AFTER_STR;
            g_Status &= (~RTK_CIUSB_STS_AFTER_STR);
        }
        if (copy_to_user((int __user *)args, &status, sizeof(int))) {
            ret = -EFAULT;
        }
    }

    return ret;
}

static int ciplus_ioctl_get_info(usb_ciplus *dev, unsigned long args)
{
    int ret=0;
    CIUSB_IOC_INFO info;

    if ((!dev->connected) || (!dev->udev) || (!dev->interface) || (args == 0)) {
        ret = -EFAULT;
    }
    else {
        info.vendor_id = dev->udev->descriptor.idVendor;
        info.product_id = dev->udev->descriptor.idProduct;
        info.serial_num = dev->udev->descriptor.iSerialNumber;
        info.packet_size_r = dev->packet_size_r;
        info.packet_size_w = dev->packet_size_w;
        info.time_read_wait = dev->time_read_wait;
        info.time_write_wait = dev->time_write_wait;
        if (copy_to_user((CIUSB_IOC_INFO __user *)args, &info, sizeof(info))) {
            ret = -EFAULT;
        }
    }
    return ret;
}

static int ciplus_ioctl_reset(usb_ciplus *dev, unsigned long args)
{
    int ret=0;

    if ((!dev->connected) || (!dev->udev) || (!dev->interface)) {
        ret = -EFAULT;
    }
    else if (args == 1) {
        pr_info("ciplus_ioctl: reset!\n");
        if (usb_lock_device_for_reset(dev->udev, NULL) < 0) {
            pr_err("ciplus_ioctl: could not obtain lock to reset device\n");
            ret = -EFAULT;
        }
        else {
            usb_reset_device(dev->udev);
            usb_unlock_device(dev->udev);
        }
    }
    return ret;
}

static int ciplus_ioctl_set_param(usb_ciplus *dev, unsigned long args)
{
    int ret=0;

    if (args == 0) {
        ret = -EFAULT;
    }
    else {
        CIUSB_IOC_PARAM param;

        if (copy_from_user(&param, (CIUSB_IOC_PARAM __user *)args, sizeof(param))) {
            ret = -EFAULT;
        }
        else {
            dev->read_timeout = param.read_timeout;
            dev->write_timeout = param.write_timeout;
            dev->return_FH = param.return_FH;
        }
    }

    return ret;
}

static long ciplus_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
    usb_ciplus *dev=filp->private_data;
    int ret=0;

    if (dev) {
        switch (cmd) {
        case RTK_CIUSB_IOC_STATUS:
            ret = ciplus_ioctl_get_status(dev, args);
            break;

        case RTK_CIUSB_IOC_INFO:
            ret = ciplus_ioctl_get_info(dev, args);
            break;

        case RTK_CIUSB_IOC_RESET:
            ret = ciplus_ioctl_reset(dev, args);
            break;

        case RTK_CIUSB_IOC_SET_PARAM:
            ret = ciplus_ioctl_set_param(dev, args);
            break;

        case RTK_CIUSB_IOC_RESET_STATISTIC:
            dev->time_read_wait = 0;
            dev->time_write_wait = 0;
            break;

        default:
            ret = -EINVAL;
        }
    }
    else
    {
        ret = -ENODEV;
    }

    return ret;
}

static void ciplus_read_bulk_callback(struct urb *urb)
{
    usb_ciplus *dev;
    unsigned long flags;

    dev = urb->context;

    spin_lock_irqsave(&dev->err_lock, flags);
    /* sync/async unlink faults aren't errors */
    if (urb->status) {
       if (!(urb->status == -ENOENT ||
           urb->status == -ECONNRESET ||
           urb->status == -ESHUTDOWN))
       {
          dev_err(&dev->interface->dev,
             "%s - read bulk status: %d\n",
             __func__, urb->status);
           dev_err(&dev->interface->dev, "actual len: %u, Err cnt=%u\n", urb->actual_length, urb->error_count);
       }

       dev->errors = urb->status;
    } else {
       dev->bulk_in_filled = urb->actual_length;
    }
    dev->ongoing_read = 0;
    spin_unlock_irqrestore(&dev->err_lock, flags);

    wake_up_interruptible(&dev->bulk_in_wait);
}

static int ciplus_do_read_io(usb_ciplus *dev)
{
    int rv;

    /* prepare a read */
    usb_fill_bulk_urb(dev->bulk_in_urb,
                      dev->udev,
                      usb_rcvbulkpipe(dev->udev,
                      dev->bulk_in_endpointAddr),
                      dev->bulk_in_buffer,
                      dev->bulk_in_size,
                      ciplus_read_bulk_callback,
                      dev);
    /* tell everybody to leave the URB alone */
    spin_lock_irq(&dev->err_lock);
    dev->ongoing_read = 1;
    spin_unlock_irq(&dev->err_lock);

    /* submit bulk in urb, which means no data to deliver */
    dev->bulk_in_filled = 0;
    dev->bulk_in_copied = 0;

    /* do it */
    rv = usb_submit_urb(dev->bulk_in_urb, GFP_KERNEL);
    if (rv < 0) {
        dev_err(&dev->interface->dev,
            "%s - failed submitting read urb, error %d\n",
            __func__, rv);
        rv = (rv == -ENOMEM) ? rv : -EIO;
        spin_lock_irq(&dev->err_lock);
        dev->ongoing_read = 0;
        spin_unlock_irq(&dev->err_lock);
    }

    return rv;
}

static ssize_t ciplus_read_wait_event(struct file *file, usb_ciplus *dev)
{
    int rv = 0;
    struct timespec time0, time1;

    /* nonblocking IO shall not wait */
    if (file->f_flags & O_NONBLOCK) {
        rv = -EAGAIN;
        goto exit;
    }
    /*
     * IO may take forever
     * hence wait in an interruptible state
     */
    if (dev->read_timeout == 0) {
        rv = -EAGAIN;
        goto exit;
    }
    if (dev->collect_statistic) {
        getnstimeofday(&time0);
    }
    rv = wait_event_interruptible_timeout(dev->bulk_in_wait, (!dev->ongoing_read), dev->read_timeout);
    if (dev->collect_statistic) {
        getnstimeofday(&time1);
        dev->time_read_wait += (time1.tv_sec - time0.tv_sec) * 1000000000 + (time1.tv_nsec - time0.tv_nsec);
    }
    if (rv <= 0) {  // if (rv < 0)
        rv = -EAGAIN;
    }

exit:
    // rv == 0: has data
    // -EAGAIN: no data

    return rv;
}

static ssize_t ciplus_read_check_frag_header(usb_ciplus *dev, bool *pretry)
{
    int rv = 0;

    // skip fragment header
    if ((dev->bulk_in_filled == sizeof(fragment_header)) &&
        (dev->bulk_in_copied == 0) &&
        (dev->interface_idx == CIUSB_INTF_MEDIA_IDX)) {
        if (memcmp(dev->bulk_in_buffer, fragment_header, sizeof(fragment_header)) == 0) {
            if (dev->return_FH) {
                rv = FRAGMENT_HEADER_MAGIC_SIZE;
                ciplus_do_read_io(dev);
            }
            else {
                rv = ciplus_do_read_io(dev);
                if ((rv == 0) && pretry){
                    *pretry = true;
                }
            }
        }
    }

    return rv;
}

static ssize_t ciplus_copy_data(usb_ciplus *dev, char *buffer, size_t count, bool *pretry)
{
    int rv = 0;
    size_t available, chunk;

    available = dev->bulk_in_filled - dev->bulk_in_copied;
    if (!available)
    {
        /* no available data in the buffer - we need to start IO*/
        rv = ciplus_do_read_io(dev);
        if ((rv == 0) && pretry){
            *pretry = true;
        }
    }
    else {
        chunk = min(available, count);
        /* chunk tells us how much shall be copied */
        if (copy_to_user(buffer, dev->bulk_in_buffer + dev->bulk_in_copied, chunk))
            rv = -EFAULT;
        else
            rv = chunk;

        dev->bulk_in_copied += chunk;

        /*
        * if we are asked for more than we have, we start IO but don't wait
        */
        if (available <= count) {
            ciplus_do_read_io(dev);
        }
    }

    return rv;
}

static ssize_t ciplus_read_data(struct file *file, char *buffer, size_t count)
{
    int rv = 0;
    usb_ciplus *dev;
    bool retry;
    bool ongoing_io;

    dev = file->private_data;

    do {
        retry = false;

        spin_lock_irq(&dev->err_lock);
        ongoing_io = dev->ongoing_read;
        spin_unlock_irq(&dev->err_lock);

        if (ongoing_io) {
            if ((rv = ciplus_read_wait_event(file, dev)) < 0) {
                rv = 0;
                goto exit;
            }
        }

        /* errors must be reported */
        rv = dev->errors;
        if (rv < 0) {
            /* any error is reported once */
            dev->errors = 0;
            /* to preserve notifications about reset */
            rv = (rv == -EPIPE) ? rv : -EIO;
            /* report it */
        }
        else {
            // skip fragment header
            rv = ciplus_read_check_frag_header(dev, &retry);
            if ((rv == 0) && (!retry)){
                rv = ciplus_copy_data(dev, buffer, count, &retry);
            }
        }
    } while (retry);

exit:

    return rv;
}

static ssize_t ciplus_read(struct file *file, char *buffer, size_t count,
           loff_t *ppos)
{
    usb_ciplus *dev;
    int rv;

    dev = file->private_data;

    /* if we cannot read at all, return EOF */
    if (!dev->bulk_in_urb || !count || !buffer)
        return 0;

    /* no concurrent readers */
    rv = mutex_lock_interruptible(&dev->rd_mutex);
    if (rv < 0)
        return rv;

    if (!dev->connected) {		/* disconnect() was called */
        rv = -ENODEV;
    }
    else {
        /* if IO is under way, we must not touch things */
        rv = ciplus_read_data(file, buffer, count);
    }

    mutex_unlock(&dev->rd_mutex);
    return rv;
}

static void ciplus_write_bulk_callback(struct urb *urb)
{
    usb_ciplus *dev;
    unsigned long flags;

    dev = urb->context;

    /* sync/async unlink faults aren't errors */
    if (urb->status) {
        if (!(urb->status == -ENOENT ||
            urb->status == -ECONNRESET ||
            urb->status == -ESHUTDOWN))
            dev_err(&dev->interface->dev,
             "%s - nonzero write bulk status received: %d\n",
             __func__, urb->status);

        spin_lock_irqsave(&dev->err_lock, flags);
        dev->errors = urb->status;
        spin_unlock_irqrestore(&dev->err_lock, flags);
    }

    /* free up our allocated buffer */
    ciplus_free_buf_w(dev, (char *)urb->transfer_buffer);
    up(&dev->limit_sem);
}

static ssize_t ciplus_write_get_resource(struct file *file, usb_ciplus *dev)
{
    int retval = 0;
    struct timespec time0, time1;

    if (dev->collect_statistic) {
        getnstimeofday(&time0);
    }
    if (!(file->f_flags & O_NONBLOCK)) {
        if (down_timeout(&dev->limit_sem, dev->write_timeout)) {
            retval = -EAGAIN;
        }
    } else {
        if (down_trylock(&dev->limit_sem)) {
            retval = -EAGAIN;
        }
    }
    if (dev->collect_statistic) {
        getnstimeofday(&time1);
        dev->time_write_wait += (time1.tv_sec - time0.tv_sec) * 1000000000 + (time1.tv_nsec - time0.tv_nsec);
    }

    return retval;
}

static ssize_t ciplus_write_alloc(usb_ciplus *dev, struct urb **purb, char **pbuf)
{
    int retval;
    struct urb *urb=NULL;
    char *buf = NULL;

    spin_lock_irq(&dev->err_lock);
    retval = dev->errors;
    if (retval < 0) {
        /* any error is reported once */
        dev->errors = 0;
        /* to preserve notifications about reset */
        retval = (retval == -EPIPE) ? retval : -EIO;
    }
    spin_unlock_irq(&dev->err_lock);
    if (retval < 0)
        goto exit;

    /* create a urb, and a buffer for it, and copy the data to the urb */
    urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!urb) {
        retval = -ENOMEM;
        goto exit;
    }

    buf = ciplus_alloc_buf_w(dev, &urb->transfer_dma);
    if (!buf) {
        retval = -ENOMEM;
        goto exit;
    }

exit:
    if (purb) {
        *purb = urb;
    }
    if (pbuf) {
        *pbuf = buf;
    }
    return retval;
}

static ssize_t ciplus_write_transfer(usb_ciplus *dev, char *buf, size_t writesize, struct urb *urb)
{
    int retval = 0;

    /* this lock makes sure we don't submit URBs to gone devices */
    mutex_lock(&dev->wr_mutex);
    if (!dev->connected) {		/* disconnect() was called */
        mutex_unlock(&dev->wr_mutex);
        retval = -ENODEV;
    }
    else {
        /* initialize the urb properly */
        usb_fill_bulk_urb(urb, dev->udev,
                          usb_sndbulkpipe(dev->udev, dev->bulk_out_endpointAddr),
                          buf, writesize, ciplus_write_bulk_callback, dev);
        urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
        if (writesize == 0)
        {
            urb->transfer_flags |= URB_ZERO_PACKET;
        }
        usb_anchor_urb(urb, &dev->submitted);

        /* send the data out the bulk port */
        retval = usb_submit_urb(urb, GFP_KERNEL);
        mutex_unlock(&dev->wr_mutex);
        if (retval) {
            dev_err(&dev->interface->dev,
                "%s - failed submitting write urb, error %d\n",
                __func__, retval);
            usb_unanchor_urb(urb);
        }
        else {
            /*
            * release our reference to this urb, the USB core will eventually free
            * it entirely
            */
            usb_free_urb(urb);
        }
    }

    return retval;
}

static ssize_t ciplus_write(struct file *file, const char *user_buffer,
            size_t count, loff_t *ppos)
{
    usb_ciplus *dev;
    int retval = 0;
    struct urb *urb = NULL;
    char *buf = NULL;
    size_t writesize;
    size_t remain=count;

    dev = file->private_data;

    /* verify that we actually have some data to write */
    if ((count == 0) || !user_buffer)
        goto exit;

next:
    writesize = min(remain, dev->bulk_out_size);
    /*
    * limit the number of URBs in flight to stop a user from using up all
    * RAM
    */
    if ((retval = ciplus_write_get_resource(file, dev)) < 0) {
        goto exit;
    }

    if ((retval = ciplus_write_alloc(dev, &urb, &buf)) < 0) {
        goto error;
    }

    if (writesize > 0) {
        if (copy_from_user(buf, user_buffer+count-remain, writesize)) {
            retval = -EFAULT;
            goto error;
        }
    }

    if ((retval = ciplus_write_transfer(dev, buf, writesize, urb)) != 0) {
        goto error;
    }

    remain -= writesize;
    if(remain > 0) goto next;

    return count;

error:
    if (urb) {
        ciplus_free_buf_w(dev, buf);

        usb_free_urb(urb);
    }
    up(&dev->limit_sem);

exit:
    return (count - remain);
}

static const struct file_operations ciplus_fops = {
    .owner = THIS_MODULE,
    .read = ciplus_read,
    .write = ciplus_write,
    .open = ciplus_open,
    .release = ciplus_release,
    .flush = ciplus_flush,
    .llseek = noop_llseek,
    .unlocked_ioctl = ciplus_ioctl,
};

/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
static int ciplus_probe_detect(const struct usb_device_id *id)
{
    int index = -EMFILE;

    pr_info("ciplus_probe: [%x, %x, %x]\n", id->bInterfaceClass, id->bInterfaceSubClass, id->bInterfaceProtocol);
    if ((id->bInterfaceClass == CIUSB_INTERFACE_CLASS_CODE) &&
        (id->bInterfaceSubClass == CIUSB_INTERFACE_SUBCLASS_CODE)) {
        if (id->bInterfaceProtocol == CIUSB_CMD_PROTOCOL_CODE) {
            index = CIUSB_INTF_CMD_IDX;
        }
        else if (id->bInterfaceProtocol == CIUSB_MEDIA_PROTOCOL_CODE) {
            index = CIUSB_INTF_MEDIA_IDX;
        }
    }
    else if ((id->bInterfaceClass == CIUSB_COMM_CLASS_CODE) &&
             (id->bInterfaceSubClass == CIUSB_COMM_SUBCLASS_CODE) &&
             (id->bInterfaceProtocol == CIUSB_COMM_PROTOCOL_CODE)) {
        index = CIUSB_INTF_COMM_IDX;
    }
    else {
        pr_err("%s: wrong device [%x, %x, %x]!\n", __func__, id->bInterfaceClass, id->bInterfaceSubClass, id->bInterfaceProtocol);
    }

    return index;
}

static int ciplus_probe_setup_endpoint(usb_ciplus *dev, struct usb_interface *interface, int int_idx)
{
    int retval = 0;
    struct usb_host_interface *iface_desc;
    struct usb_endpoint_descriptor *bulk_in, *bulk_out, *endpoint;

    iface_desc = interface->cur_altsetting;
    for(retval=0; retval != iface_desc->desc.bNumEndpoints; retval++)
    {
        endpoint = &iface_desc->endpoint[retval].desc;
    }

    /* use only the first bulk-in and bulk-out endpoints */
    retval = usb_find_common_endpoints(interface->cur_altsetting,
          &bulk_in, &bulk_out, NULL, NULL);
    if (retval) {
        dev_err(&interface->dev,
            "Could not find both bulk-in and bulk-out endpoints\n");
        goto exit;
    }

    if (int_idx == CIUSB_INTF_MEDIA_IDX) {
        dev->bulk_in_size = MEDIA_BULK_IN_SIZE;
    }
    else {
        dev->bulk_in_size = usb_endpoint_maxp(bulk_in);
    }
    dev->packet_size_r = usb_endpoint_maxp(bulk_in);
    dev->bulk_in_endpointAddr = bulk_in->bEndpointAddress;
    dev->bulk_in_buffer = kmalloc(dev->bulk_in_size, GFP_KERNEL);
    if (!dev->bulk_in_buffer) {
        retval = -ENOMEM;
        goto exit;
    }
    dev->bulk_in_urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!dev->bulk_in_urb) {
        retval = -ENOMEM;
        goto exit;
    }

    dev->bulk_out_endpointAddr = bulk_out->bEndpointAddress;
    dev->packet_size_w = usb_endpoint_maxp(bulk_out);
    if (int_idx == CIUSB_INTF_MEDIA_IDX) {
        dev->bulk_out_size = MEDIA_BULK_OUT_SIZE;
    }
    else {
        dev->bulk_out_size = usb_endpoint_maxp(bulk_out);
    }

exit:
    return retval;
}

static int ciplus_probe(struct usb_interface *interface,
                        const struct usb_device_id *id)
{
    usb_ciplus *dev;
    int retval;
    int i, int_idx;

    if ((int_idx = ciplus_probe_detect(id)) == -EMFILE) {
        return -EMFILE;
    }

    if (usb_ciplus_device[int_idx].connected) {
        pr_err("%s: ciusb [%d] interface has been connected\n", __func__, int_idx);
        return -EMFILE;
    }

    dev = &usb_ciplus_device[int_idx];
    mutex_lock(&dev->rd_mutex);
    mutex_lock(&dev->wr_mutex);
    if (dev->connected) {
        retval = 0;
        goto exit;
    }

    dev->udev = usb_get_dev(interface_to_usbdev(interface));
    dev->interface = usb_get_intf(interface);

    /* set up the endpoint information */
    if ((retval = ciplus_probe_setup_endpoint(dev, interface, int_idx)) != 0) {
        goto exit;
    }

    /* save our data pointer in this interface device */
    usb_set_intfdata(interface, dev);

    memset(&dev->buf_w[0], 0, sizeof(buf_write_st)*WRITES_IN_FLIGHT);
    for (i=0; i<WRITES_IN_FLIGHT; i++) {
        dev->buf_w[i].buf = usb_alloc_coherent(dev->udev, dev->bulk_out_size, GFP_KERNEL,
                                               &dev->buf_w[i].dma);
        dev->buf_w[i].used = 0;
        if (!dev->buf_w[i].buf) {
            retval = -ENOMEM;
            goto exit;
        }
    }
    dev->time_read_wait = 0;
    dev->time_write_wait = 0;
    dev->collect_statistic = COLLECT_STATISTIC_DATA;
    dev->connected = 1;

    /* let the user know what node this device is now attached to */
    dev_info(&interface->dev,
        "USB ciusb #%d device now attached",
        interface->minor);
    retval = 0;

exit:
    if (retval != 0) {
        ciplus_delete(dev);
    }
    mutex_unlock(&dev->wr_mutex);
    mutex_unlock(&dev->rd_mutex);

    return retval;
}

static void ciplus_disconnect(struct usb_interface *interface)
{
    usb_ciplus *dev = usb_get_intfdata(interface);
    int minor = interface->minor;

    pr_info("ciplus_disconnect [%d]\n", dev->interface_idx);
    /* prevent more I/O from starting */
    mutex_lock(&dev->rd_mutex);
    mutex_lock(&dev->wr_mutex);

    usb_set_intfdata(interface, NULL);

    dev->connected = 0;
    usb_kill_anchored_urbs(&dev->submitted);
    ciplus_delete(dev);

    mutex_unlock(&dev->wr_mutex);
    mutex_unlock(&dev->rd_mutex);

    dev_info(&interface->dev, "USB ciusb #%d now disconnected", minor);
}

static void ciplus_draw_down(usb_ciplus *dev)
{
    int time;

    time = usb_wait_anchor_empty_timeout(&dev->submitted, 1000);
    if (!time)
        usb_kill_anchored_urbs(&dev->submitted);
    usb_kill_urb(dev->bulk_in_urb);
}

static int ciplus_suspend(struct usb_interface *intf, pm_message_t message)
{
    usb_ciplus *dev = usb_get_intfdata(intf);

    if (!dev)
        return 0;
    ciplus_draw_down(dev);
    return 0;
}

static int ciplus_resume(struct usb_interface *intf)
{
    g_Status |= RTK_CIUSB_STS_AFTER_STR;
    return 0;
}

static int ciplus_pre_reset(struct usb_interface *intf)
{
    usb_ciplus *dev = usb_get_intfdata(intf);

    pr_info("ciplus_pre_reset: %d\n", dev->interface_idx);
    mutex_lock(&dev->rd_mutex);
    mutex_lock(&dev->wr_mutex);
    ciplus_draw_down(dev);

    return 0;
}

static int ciplus_post_reset(struct usb_interface *intf)
{
    usb_ciplus *dev = usb_get_intfdata(intf);

    pr_info("ciplus_post_reset: %d\n", dev->interface_idx);
    /* we are sure no URBs are active - no locking needed */
    dev->errors = -EPIPE;
    mutex_unlock(&dev->wr_mutex);
    mutex_unlock(&dev->rd_mutex);

    return 0;
}

static struct usb_driver ciplus_driver = {
    .name = "ciusb",
    .probe = ciplus_probe,
    .disconnect = ciplus_disconnect,
    .suspend = ciplus_suspend,
    .resume = ciplus_resume,
    .pre_reset = ciplus_pre_reset,
    .post_reset = ciplus_post_reset,
    .id_table = ciplus_table,
    .supports_autosuspend = 1,
};

static int ciusb_drv_init_device(usb_ciplus *dev, int idx)
{
    dev_t dev_sn;
    int ret = 0;

    memset(&dev->buf_w[0], 0, sizeof(buf_write_st)*WRITES_IN_FLIGHT);
    ret = alloc_chrdev_region(&dev_sn, 0, 1, CIUSB_NAME);
    if (ret)
    {
        pr_err("%s: can't alloc chrdev\n", __func__);
    }
    else {

        dev->interface_idx = idx;
        dev->dev_major = MAJOR(dev_sn);
        dev->dev_minor = MINOR(dev_sn);
        if (idx == CIUSB_INTF_MEDIA_IDX) {
            dev->read_timeout = CIUSB_MEDIA_READ_TIMEOUT;
            dev->write_timeout = CIUSB_MEDIA_WRITE_TIMEOUT;
        }
        else {
            dev->read_timeout = CIUSB_COM_READ_TIMEOUT;
            dev->write_timeout = CIUSB_COM_WRITE_TIMEOUT;
        }

        cdev_init(&dev->cdev, &ciplus_fops);
        dev->cdev.owner = THIS_MODULE;
        dev->cdev.ops   = &ciplus_fops;
        ret = cdev_add(&dev->cdev, dev_sn, 1);
        if (ret < 0) {
            dev->cdev.ops = NULL;
            pr_err("%s: can not add character device...\n", __func__);
        }
        else {
            dev_sn = MKDEV(dev->dev_major, dev->dev_minor);
            device_create(ciplus_class, NULL, dev_sn, NULL, ciplus_device_name[idx]);
        }
    }

    return ret;
}

static int __init ciusb_drv_init(void)
{
    int ret = 0;
    usb_ciplus *dev;
    dev_t dev_sn;
    int i;

    if(ciplus_class==NULL) {
        ciplus_class = class_create(THIS_MODULE, CIUSB_NAME);
        if (IS_ERR(ciplus_class))
        {
            ret = PTR_ERR(ciplus_class);
            pr_err("%s: can't create class\n", __func__);
            return ret;
        }
    }
    ret = usb_register(&ciplus_driver);

    memset(usb_ciplus_device, 0, sizeof(usb_ciplus_device));

    for (i=0; i<CIUSB_INTF_NUM; i++) {
        dev = &usb_ciplus_device[i];

        sema_init(&dev->limit_sem, WRITES_IN_FLIGHT);
        mutex_init(&dev->rd_mutex);
        mutex_init(&dev->wr_mutex);
        spin_lock_init(&dev->err_lock);
        init_usb_anchor(&dev->submitted);
        init_waitqueue_head(&dev->bulk_in_wait);

        ret = ciusb_drv_init_device(dev, i);
        if (ret < 0) goto error;
    }

    return ret;

error:

    for (i=0; i<CIUSB_INTF_NUM; i++) {
        dev = &usb_ciplus_device[i];
        dev_sn = MKDEV(dev->dev_major, dev->dev_minor);
        if (dev_sn) {
            if (dev->cdev.ops) {
                device_destroy(ciplus_class, dev_sn);
                cdev_del(&dev->cdev);
            }
            unregister_chrdev_region(dev_sn, 1);
        }
    }

    if(ciplus_class) {
        class_destroy(ciplus_class);
        ciplus_class = NULL;
    }

    return ret;
}

static void __exit ciusb_drv_exit(void)
{
    usb_ciplus *dev;
    dev_t dev_sn;
    int i;

    for (i=0; i<CIUSB_INTF_NUM; i++) {
        dev = &usb_ciplus_device[i];
        dev_sn = MKDEV(dev->dev_major, dev->dev_minor);
        if (dev_sn) {
            if (dev->cdev.ops) {
                device_destroy(ciplus_class, dev_sn);
                cdev_del(&dev->cdev);
            }
            unregister_chrdev_region(dev_sn, 1);
        }
    }

    usb_deregister(&ciplus_driver);
    if(ciplus_class) {
        class_destroy(ciplus_class);
        ciplus_class = NULL;
    }

}

//module_usb_driver(ciplus_driver);
module_init(ciusb_drv_init);
module_exit(ciusb_drv_exit);


MODULE_LICENSE("GPL v2");
