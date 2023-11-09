#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <asm/io.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include "rtk_ddcci_sysfs.h"
#include "rtk_ddcci_priv.h"
#include "rtk_ddc_dbg.h"

static ssize_t ddcci_enable_id_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    RTK_DDC_INFO("[ddcci] enable_ddcci_id=(%d)\n",g_ddcci_connect_flag);
    return snprintf(buf, PAGE_SIZE, "%d\n",g_ddcci_connect_flag);
}

static ssize_t ddcci_enable_id_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
    int val;

    if (sscanf(buf, "%d", &val)!=1 || (val<0) || (val>3))
    {
        RTK_DDC_ERR("invalid enable value (%s), enable_ddcci_id should be 0, 1, 2 or 3\n", buf);
        return -EINVAL;
    }
    RTK_DDC_ERR("[ddcci] enable ddcci id from(%d) => to (%d)\n",g_ddcci_connect_flag, val);

    rtk_ddcci_lib_init(val);

    if(g_rtk_ddcci_fifo_thread) {
        RTK_DDC_ERR("[ddcci] g_rtk_ddcci_fifo_thread already exist\n");
    } else {
        g_rtk_ddcci_fifo_thread = kthread_run(rtk_ddcci_fifo_thread, NULL, "ddcci_fifo_thread");
        if(!g_rtk_ddcci_fifo_thread) {
            return -ECHILD;
        }
        RTK_DDC_ERR("[ddcci] g_rtk_ddcci_fifo_thread start\n");
    }
    return count;
}
static DEVICE_ATTR(enable_ddcci_id, S_IRUGO | S_IWUSR, ddcci_enable_id_show, ddcci_enable_id_store);



static ssize_t ddcci_enable_dbg_thread_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
    int val;

    if (strcmp(attr->attr.name, "enable_dbg_thread")==0)
    {
        if (sscanf(buf, "%d", &val)!=1 || (val!=1))
        {
            RTK_DDC_ERR("invalid enable value (%s), enable should be 1\n", buf);
            return -EINVAL;
        }

        if(g_rtk_ddcci_dbg_thread) {
            RTK_DDC_ERR("[ddcci] g_rtk_ddcci_dbg_thread already exist\n");
        } else {
            g_rtk_ddcci_dbg_thread = kthread_run(rtk_ddcci_dbg_thread, NULL, "ddcci_dbg_thread");
            if(!g_rtk_ddcci_dbg_thread) {
                return -ECHILD;
            }
            RTK_DDC_ERR("[ddcci] g_rtk_ddcci_dbg_thread start\n");
        }
    }

    return count;
}
static DEVICE_ATTR(enable_dbg_thread, S_IWUSR, NULL, ddcci_enable_dbg_thread_store);



static ssize_t ddcci_rev_message_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    ddcci_msg msg = {0};
    unsigned char buff_data[DDC_BUF_LENGTH]= {0};
    unsigned char len = 0;

    if (rtk_ddcci_rcv_message(buff_data, &len) != 0) {
        msg.len = len;
        memcpy(msg.buf, buff_data, len);
    }

    return snprintf(buf, PAGE_SIZE, "len=%d. %2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x\n", len,buff_data[0],buff_data[1],buff_data[2],buff_data[3],buff_data[4],buff_data[5],buff_data[6],buff_data[7],buff_data[8],buff_data[9],buff_data[10],buff_data[11],buff_data[12],buff_data[13],buff_data[14],buff_data[15]);
}
static DEVICE_ATTR(rev_message, S_IRUGO, ddcci_rev_message_show, NULL);


static struct attribute *dev_attrs[] = {
    &dev_attr_enable_ddcci_id.attr,
    &dev_attr_enable_dbg_thread.attr,
    &dev_attr_rev_message.attr,
    NULL,
};

static struct attribute_group dev_attr_grp = {
    .attrs = dev_attrs,
};

int rtk_ddcci_sysfs_create(struct device *dev)
{
    int ret  = 0;

    ret  = sysfs_create_group(&dev->kobj, &dev_attr_grp);
    return ret;
}

