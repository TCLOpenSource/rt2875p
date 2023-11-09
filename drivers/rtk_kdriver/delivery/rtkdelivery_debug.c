/******************************************************************************
 *
 *   Realtek Delivery debug
 *
 *   Copyright(c) 2019 Realtek Semiconductor Corp. All rights reserved.
 *
 *****************************************************************************/
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include "rtkdelivery.h"
#include "rtkdelivery_common.h"
#include "rtkdelivery_export.h"
#include "rtkdelivery_debug.h"
#include <linux/file.h>
#include <linux/string.h>
#include <linux/version.h>

#define DMX_DBG_LOG(fmt, ...) rtd_pr_delivery_emerg( fmt, ##__VA_ARGS__)

#define RTKDELIVERY_PROC_DIR                        "rtkdelivery"
#define RTKDELIVERY_PROC_ENTRY                      "dbg"
#define RTKDELIVERY_STATUS_ENTRY                    "status"
#define RTKDELIVERY_MAX_CMD_LENGTH 256

enum {
	DELIVERY_DBG_CMD_LOG_LEVEL = 0,
	DELIVERY_DBG_CMD_NUMBER
} DELIVERY_DBG_CMD_INDEX_T;

static const char *rtkdelivery_cmd_str[] = {
	"level=",			/* DELIVERY_DBG_CMD_LOG_LEVEL */
};

unsigned long rtkdelivery_dbg_log_mask;
unsigned char rtkdelivery_dbg_monitorio_en = 1;

/*****************************************************************************/

/* Print IO */
unsigned long rtkdelivery_dbg_monitorio_read_count[DELIVERY_CH_NUM];

static inline bool rtkdelivery_dbg_parse_value(char *cmd_pointer, long long *parsed_data)
{
	if (kstrtoll(cmd_pointer, 0, parsed_data) == 0) {
		DMX_DBG_LOG("rtkdelivery_debug:%d parsed_data=%lld\n",
			__LINE__, *parsed_data);
		return true;
	} else {
		DMX_DBG_LOG("rtkdelivery_debug:%d parsing data failed. pCmd=%s\n",
			__LINE__, cmd_pointer);
		return false;
	}
}

static inline void rtkdelivery_dbg_EXECUTE(const int cmd_index, char **cmd_pointer)
{
	long long parsed_data = 0;

	switch (cmd_index) {
	case DELIVERY_DBG_CMD_LOG_LEVEL:
		rtkdelivery_dbg_parse_value(*cmd_pointer, &parsed_data);
		rtkdelivery_dbg_log_mask = (unsigned long)parsed_data;
		DMX_DBG_LOG("rtkdelivery_dbg_log_level=0x%8lx\n",
			rtkdelivery_dbg_log_mask);
		break;
	default:
		DMX_DBG_LOG("rtkdelivery_debug:%d unknown command.\n", __LINE__);
	}
}


static ssize_t rtkdelivery_proc_write(
	struct file *file,
	const char __user *buf,
	size_t count,
	loff_t *ppos)
{
	int cmd_index;
	char *cmd_pointer = NULL;
	char str[RTKDELIVERY_MAX_CMD_LENGTH] = {0};
	if (buf == NULL) {
		DMX_DBG_LOG("rtkdelivery_debug:%d buf=%p\n", __LINE__, buf);
		return -EFAULT;
	}
	if (count > RTKDELIVERY_MAX_CMD_LENGTH - 1)
		count = RTKDELIVERY_MAX_CMD_LENGTH - 1;
	if (copy_from_user(str, buf, RTKDELIVERY_MAX_CMD_LENGTH - 1)) {
		DMX_DBG_LOG("rtkdelivery_debug:%d copy_from_user failed! (buf=%p, count=%zu)\n", __LINE__, buf, count);
		return -EFAULT;
	}

	if (count > 0)
		str[count-1] = '\0';
	DMX_DBG_LOG("rtkdelivery_debug:%d >>%s\n", __LINE__, str);


	for (cmd_index = 0; cmd_index < DELIVERY_DBG_CMD_NUMBER; cmd_index++) {
		if (strncmp(str,
				rtkdelivery_cmd_str[cmd_index],
				strlen(rtkdelivery_cmd_str[cmd_index])) == 0)
			break;
	}


	if (cmd_index < DELIVERY_DBG_CMD_NUMBER) {
		cmd_pointer = &str[strlen(rtkdelivery_cmd_str[cmd_index])];
	}

	/* The extern definition is at rtkdelivery_common.h */
	if (console_printk[0] == 0)
		console_printk[0] = 1;
	rtkdelivery_dbg_EXECUTE(cmd_index, &cmd_pointer);

	return count;
}


static ssize_t rtkdelivery_proc_read(
	struct file *file,
	char __user *buf,
	size_t count,
	loff_t *ppos)
{
	char *str = "Rtkdelivery kernel driver debug message\n";
	int len = strlen(str);
	if (copy_to_user(buf, str, len)) {
		DMX_DBG_LOG("rtkdelivery_debug:%d Copy proc data to user space failed\n",
			__LINE__);
		return 0;
	}

	if (*ppos == 0)
		*ppos += len;
	else
		len = 0;
	return len;
}

int rtkdelivery_print_delivery_status_str(char* buff, int buff_size)
{
	int len = 0;
	int i;
	int n;

	if (delivery_device) {
		n = snprintf(buff, buff_size,"======================================================\n");
		if (n<0)
            		return -1;
		len+=n; buff+=n; buff_size-=n;

		for (i = 0 ; i < delivery_device->chNum ; i++) {
			delivery_channel *pCh = &delivery_device->pChArray[i];
			if (pCh) {

				if (rtkdelivery_dbg_monitorio_en)
					n = snprintf(buff, buff_size, "------------------------------------------------------\nCH%d: streaming=%d, RX Count :%lu\n",
						i, pCh->startStreaming, rtkdelivery_dbg_monitorio_read_count[i]);
				else
					n = snprintf(buff, buff_size, "------------------------------------------------------\nCH%d: streaming=%d\n",
						i, pCh->startStreaming);

				if (n<0)
					return -1;

				len+=n; buff+=n; buff_size-=n;
			}
		}
	}
	else
	{
	    DMX_DBG_LOG( "missing dmx device\n");
	}
	return len;
}

static char proc_out_buff[4096];

static ssize_t rtkdelivery_status_read(
	struct file *file,
	char __user *buf,
	size_t count,
	loff_t *ppos)
{
    int proc_out_size = 0;

    if (*ppos)
        return 0;

    proc_out_size = rtkdelivery_print_delivery_status_str(proc_out_buff, sizeof(proc_out_buff));

    DMX_DBG_LOG("rtkdelivery_status_read = %d\n", proc_out_size);

    if (proc_out_size < 0)
    {
        DMX_DBG_LOG("rtkdelivery_status_read failed - get delivery target stream failed\n");
        return 0;
    }

    if (copy_to_user(buf, proc_out_buff, proc_out_size)) {
	    DMX_DBG_LOG("rtkdelivery_debug:%d Copy proc data to user space failed\n",
		    __LINE__);
	    return 0;
    }

    *ppos += proc_out_size;
    return proc_out_size;
}


/*****************************************************************************/


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops rtkdelivery_proc_fops = {
	.proc_write = rtkdelivery_proc_write,
	.proc_read = rtkdelivery_proc_read,
};

static const struct proc_ops rtkdelivery_status_fops = {
	.proc_read = rtkdelivery_status_read,
};
#else
static const struct file_operations rtkdelivery_proc_fops = {
	.owner = THIS_MODULE,
	.write = rtkdelivery_proc_write,
	.read = rtkdelivery_proc_read,
};

static const struct file_operations rtkdelivery_status_fops = {
	.owner = THIS_MODULE,
	.read = rtkdelivery_status_read,
};
#endif

struct proc_dir_entry *rtkdelivery_proc_dir;
struct proc_dir_entry *rtkdelivery_proc_entry;
struct proc_dir_entry *rtkdelivery_status_entry = NULL;

bool delivery_init_debug_proc(void)
{
	if (rtkdelivery_proc_dir == NULL && rtkdelivery_proc_entry == NULL) {
		rtkdelivery_proc_dir = proc_mkdir(RTKDELIVERY_PROC_DIR , NULL);

		if (rtkdelivery_proc_dir != NULL) {
			rtkdelivery_proc_entry =
				proc_create(RTKDELIVERY_PROC_ENTRY, 0666,
					rtkdelivery_proc_dir, &rtkdelivery_proc_fops);

			if (rtkdelivery_proc_entry == NULL) {
				proc_remove(rtkdelivery_proc_dir);
				rtkdelivery_proc_dir = NULL;
				DMX_DBG_LOG("failed to get proc entry for %s/%s\n",
					RTKDELIVERY_PROC_DIR, RTKDELIVERY_PROC_ENTRY);
				return false;
			}

			rtkdelivery_status_entry =
				proc_create(RTKDELIVERY_STATUS_ENTRY, 0666,
					rtkdelivery_proc_dir, &rtkdelivery_status_fops);

		} else {
			DMX_DBG_LOG("create rtkdelivery dir proc entry (%s) failed\n",
				RTKDELIVERY_PROC_DIR);
			return false;
		}
	}

	return true;
}

void delivery_uninit_debug_proc(void)
{
	if (rtkdelivery_proc_entry) {
		proc_remove(rtkdelivery_proc_entry);
		rtkdelivery_proc_entry = NULL;
	}

	if (rtkdelivery_status_entry) {
		proc_remove(rtkdelivery_status_entry);
		rtkdelivery_status_entry = NULL;
	}

	if (rtkdelivery_proc_dir) {
		proc_remove(rtkdelivery_proc_dir);
		rtkdelivery_proc_dir = NULL;
	}
}
