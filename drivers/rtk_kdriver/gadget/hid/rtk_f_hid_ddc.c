#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hid.h>
#include <linux/idr.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/freezer.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/miscdevice.h>
#include <clocksource/arm_arch_timer.h>
#include "rtk_composite_platform.h"

#define CONFIG_ENABLE_RECORD_TIMEOUT_CHECK
#define MAX_RECORD_LEN 9
#define MAX_RECORD_TIMEOUT   2000000000
#define EACH_RECORD_MSG_LEN 12

typedef struct _hid_record
{
	uint64_t timestamp;
	int id;
	unsigned int len;
	char data[MAX_RECORD_LEN];
	
}HID_RECORD;

#define MAX_RECORD_SIZE     1024

typedef struct _hid_record_queue
{
	HID_RECORD   records[MAX_RECORD_SIZE];
	unsigned int read_pos;
	unsigned int write_pos;
	spinlock_t  lock;
}HID_RECORD_QUEUE;

static HID_RECORD_QUEUE g_hid_record_queue =
{
	.read_pos =0,
	.write_pos = 0,
	.lock = __SPIN_LOCK_UNLOCKED(g_hid_record_queue.lock),
};

DECLARE_WAIT_QUEUE_HEAD(g_hid_record_wait_queue);

static inline uint64_t get_timestmap(void)
{
	return (arch_timer_read_counter() * 37); //ns
}

static inline uint64_t  get_time_interval(uint64_t cur, uint64_t before)
{
	if(cur >= before)
		return (cur - before);
	else
		return (0xFFFFFFFFFFFFFFFF + cur - before);
}

static void _remove_timeout_record(void)
{
	unsigned int cur_write_pos = g_hid_record_queue.write_pos;
	unsigned int cur_read_pos = g_hid_record_queue.read_pos;
	
	if(cur_read_pos > cur_write_pos)
		cur_write_pos +=  MAX_RECORD_SIZE;
	if(cur_read_pos != cur_write_pos) {
		uint64_t cur_timestamp = get_timestmap();		
		for(;cur_read_pos < cur_write_pos; cur_read_pos++) {
			if(get_time_interval(cur_timestamp,g_hid_record_queue.records[cur_read_pos % MAX_RECORD_SIZE].timestamp) 
						<= MAX_RECORD_TIMEOUT)
				break;
			//printk("SLL %llx %llx\n", cur_timestamp, g_hid_record_queue.records[cur_read_pos % MAX_RECORD_SIZE].timestamp);
		}
		g_hid_record_queue.read_pos = cur_read_pos % MAX_RECORD_SIZE;
	}
}

static void add_hid_record(int id, const unsigned char *buffer, unsigned int count)
{
	unsigned long flags;
	unsigned int cur_write_pos;
	spin_lock_irqsave(&g_hid_record_queue.lock, flags);	

#ifdef CONFIG_ENABLE_RECORD_TIMEOUT_CHECK
	_remove_timeout_record();
#endif
	cur_write_pos = g_hid_record_queue.write_pos;
	if(((cur_write_pos + 1) % MAX_RECORD_SIZE) == g_hid_record_queue.read_pos) {
		RTK_COMPOSITE_INFO("hid queue full\n");
		g_hid_record_queue.read_pos = (g_hid_record_queue.read_pos + 1) % MAX_RECORD_SIZE;
	}

	g_hid_record_queue.records[cur_write_pos].timestamp = get_timestmap();
	g_hid_record_queue.records[cur_write_pos].id = id;
	g_hid_record_queue.records[cur_write_pos].len = (count > MAX_RECORD_LEN) ? MAX_RECORD_LEN: count;
	memcpy(g_hid_record_queue.records[cur_write_pos].data, buffer, g_hid_record_queue.records[cur_write_pos].len);
	g_hid_record_queue.write_pos = (cur_write_pos + 1) % MAX_RECORD_SIZE;
	RTK_COMPOSITE_INFO("SLL S:%d,%u %d,%02x %02x %02x %02x %02x %02x %02x\n", id, count, cur_write_pos,
			buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6]);
	spin_unlock_irqrestore(&g_hid_record_queue.lock, flags);	
	wake_up(&g_hid_record_wait_queue);
}

ssize_t f_hidddc_send_data(int id, const unsigned char *buffer, unsigned int count, unsigned int f_flags)
{
	if(id != 0 && id != 1)
		return -1;
	if(!buffer || !count)
		return -1;
	add_hid_record(id, buffer, count);
	return count;
}

static unsigned int f_hid_ddc_get_record_count(void)
{
    unsigned int count = 0;
    unsigned long flags;
    spin_lock_irqsave(&g_hid_record_queue.lock, flags);
#ifdef CONFIG_ENABLE_RECORD_TIMEOUT_CHECK
    _remove_timeout_record();
#endif	
    if(g_hid_record_queue.write_pos >= g_hid_record_queue.read_pos)
        count = g_hid_record_queue.write_pos - g_hid_record_queue.read_pos;
    else
        count = MAX_RECORD_SIZE + g_hid_record_queue.write_pos - g_hid_record_queue.read_pos;
    spin_unlock_irqrestore(&g_hid_record_queue.lock, flags);	
    return count;
}

static int f_hid_ddc_dev_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int f_hid_ddc_dev_release(struct inode *inode, struct file *file)
{
    return 0;
}

/*
* Byte 0:
* Bit0: 1-mouse, 0-keyboard
* Bit1~Bit3:seq_num
* Bit4~Bit8:len
* Byte1~Byte11:HID data
*/

static int f_hid_get_hid_records(unsigned char *buf,  int buf_len)
{
    int actual_len = 0;
    unsigned long flags;
    spin_lock_irqsave(&g_hid_record_queue.lock, flags);	
#ifdef CONFIG_ENABLE_RECORD_TIMEOUT_CHECK
    _remove_timeout_record();
#endif
    while(g_hid_record_queue.read_pos != g_hid_record_queue.write_pos) {
	 HID_RECORD *record = NULL;
        if(buf_len < EACH_RECORD_MSG_LEN)
			break;
	 record = &g_hid_record_queue.records[g_hid_record_queue.read_pos];
	 g_hid_record_queue.read_pos = (g_hid_record_queue.read_pos + 1) % MAX_RECORD_SIZE;
	 
	 buf[0] = ((record->len & 0xF) << 4) | (!!record->id);
	 memcpy(buf + 1, record->data, record->len);
	 buf_len -= EACH_RECORD_MSG_LEN;
	 actual_len += EACH_RECORD_MSG_LEN;
	 buf += EACH_RECORD_MSG_LEN;
    }
    spin_unlock_irqrestore(&g_hid_record_queue.lock, flags);
    return actual_len;
}

static ssize_t f_hid_ddc_dev_read(struct file *file,
                           char __user *buff, size_t size, loff_t *ofst)
{
    int tmp_buf_len = 384;
    unsigned char tmp_buf[384];

    if(size < EACH_RECORD_MSG_LEN)
		return -EINVAL;
	
restart:
    if(wait_event_interruptible(g_hid_record_wait_queue, f_hid_ddc_get_record_count()) != 0) {
        if(freezing(current)){
            try_to_freeze();
            goto restart;
        } else
            return -ERESTARTSYS;
    }	
	
    tmp_buf_len = (size > sizeof(tmp_buf)) ? sizeof(tmp_buf) : size;
    tmp_buf_len = f_hid_get_hid_records(tmp_buf, tmp_buf_len);
    if(tmp_buf_len) {
        if (copy_to_user(buff,tmp_buf, tmp_buf_len))
            return -EFAULT;
    }	
    return tmp_buf_len;
}

static unsigned int f_hid_ddc_dev_poll(struct file * filp, poll_table * wait)
{
    poll_wait(filp, &g_hid_record_wait_queue, wait);
    if(f_hid_ddc_get_record_count())
        return POLLIN | POLLRDNORM;
    else
        return 0;
}

static struct file_operations f_hid_ddc_dev_fops = {
    .owner = THIS_MODULE,
    .open = f_hid_ddc_dev_open,
    .read = f_hid_ddc_dev_read,
    .release = f_hid_ddc_dev_release,
    .poll  = f_hid_ddc_dev_poll,
};

static struct miscdevice f_hid_ddc_miscdev = {
    MISC_DYNAMIC_MINOR,
    "hid_ddc",
    &f_hid_ddc_dev_fops,
};



#if IS_ENABLED(CONFIG_RTK_KDRV_DDC)
#if IS_ENABLED(CONFIG_RTK_KDRV_DDCCI)
#ifdef CONFIG_RTK_KDRV_DDCCI_HID

static bool get_hid_record(HID_RECORD  *record)
{
	bool ret = false;
	unsigned long flags;
	if(!record)
		return false;
	spin_lock_irqsave(&g_hid_record_queue.lock, flags);	

#ifdef CONFIG_ENABLE_RECORD_TIMEOUT_CHECK
	_remove_timeout_record();
#endif
	if(g_hid_record_queue.read_pos != g_hid_record_queue.write_pos) {
		memcpy(record, &g_hid_record_queue.records[g_hid_record_queue.read_pos], sizeof(*record));
		g_hid_record_queue.read_pos = (g_hid_record_queue.read_pos + 1) % MAX_RECORD_SIZE;
		ret = true;
	}
	spin_unlock_irqrestore(&g_hid_record_queue.lock, flags);
	return ret;
}

typedef int (*DDC_HID_CALLBACK)(unsigned int seq_num, unsigned char *buf, unsigned int len);
bool rtk_ddcci_register_hid_callback(DDC_HID_CALLBACK callback);
void rtk_ddcci_unregister_hid_callback(DDC_HID_CALLBACK callback);

static int f_hidddc_read_callback(unsigned int seq_num, unsigned char *buf, unsigned int len)
{
	HID_RECORD record;

	if(!buf || len < (MAX_RECORD_LEN + 1))
		return -1;

	if(get_hid_record(&record)) {
		/*
		* Byte 0:
		* Bit0: 1-mouse, 0-keyboard
		* Bit1~Bit3:seq_num
		* Bit4~Bit8:len
		* Byte1~Byte11:HID data
		*/
		buf[0] = ((record.len & 0xF) << 4)| ((seq_num & 0x7) << 1) | (!!record.id);
		memcpy(buf + 1, record.data, record.len);
		RTK_COMPOSITE_INFO("SLL G:%02x, %02x\n", seq_num, buf[0]);
		return (record.len + 1);
	}

	return 0;
}
#endif
#endif
#endif
int __init f_hidddc_init(void)
{
    int ret = 0;
#if IS_ENABLED(CONFIG_RTK_KDRV_DDC)
#if IS_ENABLED(CONFIG_RTK_KDRV_DDCCI)
#ifdef CONFIG_RTK_KDRV_DDCCI_HID
    rtk_ddcci_register_hid_callback(f_hidddc_read_callback);
#endif
#endif
#endif
    if (misc_register(&f_hid_ddc_miscdev)) {
        ret = -ENODEV;
        goto fail_register_hid_ddc_dev;
    }
    return ret;
fail_register_hid_ddc_dev:
#if IS_ENABLED(CONFIG_RTK_KDRV_DDC)
#if IS_ENABLED(CONFIG_RTK_KDRV_DDCCI)
#ifdef CONFIG_RTK_KDRV_DDCCI_HID
    rtk_ddcci_unregister_hid_callback(f_hidddc_read_callback);
#endif
#endif
#endif	
    return ret;
}

void __exit f_hidddc_uninit(void)
{
    misc_deregister(&f_hid_ddc_miscdev);
#if IS_ENABLED(CONFIG_RTK_KDRV_DDC)
#if IS_ENABLED(CONFIG_RTK_KDRV_DDCCI)
#ifdef CONFIG_RTK_KDRV_DDCCI_HID
    rtk_ddcci_unregister_hid_callback(f_hidddc_read_callback);
#endif
#endif
#endif	
}

