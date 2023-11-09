#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <asm/io.h>

#include <linux/pageremap.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include "rtk_scd_debug.h"

struct proc_dir_entry * smc_proc_dir = NULL;
struct proc_dir_entry * smc_proc_entry = NULL;
unsigned char smc_log_onoff=1;
unsigned char smc_print_rtx=0;


void scd_dump_data(const char* str, unsigned char* p_data, unsigned int len)
{
    char buf[512];        
    int  i;
    int  n;
    int  count = sizeof(buf);
    char *ptr = buf;
    
    n = scnprintf(ptr, count, "%s : ", str);
    ptr+=n; count-=n;

    for (i=0; i<len; i++)
    {
        n = scnprintf(ptr, count, "%02x ", *p_data++);        
        ptr+=n; count-=n;
    }

    n = scnprintf(ptr, count, "\n");        
    ptr+=n; count-=n;
    
    SC_INFO("%s", buf);    
}

