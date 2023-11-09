
#define pr_fmt(fmt) "[rtkd]:  " fmt

#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/string.h>       /* memcpy */
#include <rtd_log/rtd_module_log.h>
#include <linux/platform_device.h>
#include <linux/slab.h>     /* kmalloc() */
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/freezer.h>

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched/clock.h>
#endif
#include <linux/kernel.h>  
#include <linux/fs.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
#include <linux/panic_notifier.h>
//#include <linux/time.h>
#endif
#include <rtk_kdriver/rtd_log.h>
#include <rtd_logger.h>
#include <rtk_kdriver/io.h>
#include <linux/console.h>
#include <linux/syscalls.h>
#include <rtk_kdriver/rtk-kdrv-common.h>
#if 0
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#define SYS_MKDIR(pathname,mode)        ksys_mkdir(pathname,mode)
#define SYS_ACCESS(filename,mode)       ksys_access(filename,mode)
#define SYS_UNLINK(filename)            ksys_unlink(filename)
#define SYS_SYNC()                      ksys_sync()
#else
#define SYS_MKDIR(pathname,mode)        sys_mkdir(pathname,mode)
#define SYS_ACCESS(filename,mode)       sys_access(filename,mode)
#define SYS_UNLINK(filename)            sys_unlink(filename)
#define SYS_SYNC()                      sys_sync()
#endif
#endif

#if ((IS_ENABLED(CONFIG_RTK_KDRV_TEE))&& !defined(CONFIG_CUSTOMER_TV006))
#include <linux/arm-smccc.h>

#define OPTEE_SMC_FAST_CALL_VAL(func_num) \
    ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, ARM_SMCCC_SMC_32, \
               ARM_SMCCC_OWNER_TRUSTED_OS, (func_num))

struct optee_rpc_param
{
    u32     a0;
    u32     a1;
    u32     a2;
    u32     a3;
    u32     a4;
    u32     a5;
    u32     a6;
    u32     a7;
};

#define OPTEE_SMC_FUNCID_SET_LOGBUF 25
#define OPTEE_SMC_SET_LOGBUF \
    OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_SET_LOGBUF)

__attribute__((weak)) int optee_logbuf_setup_info_send(struct optee_rpc_param *p_rtc_set_info)
{
    struct arm_smccc_res res;
    rtd_pr_rtdlog_err("%s %d. cannot find optee_logbuf_setup_info_send!\n",__func__,__LINE__);
    p_rtc_set_info->a0 = OPTEE_SMC_SET_LOGBUF;

    arm_smccc_smc(p_rtc_set_info->a0,p_rtc_set_info->a1,p_rtc_set_info->a2,
                  p_rtc_set_info->a3,p_rtc_set_info->a4,p_rtc_set_info->a5,
                  p_rtc_set_info->a6,0,&res);

    if(res.a0 == 0)
        return 0;
    else
        return -1;
}
#endif

#ifdef CONFIG_ANDROID
#define RTDLOG_DEFAULT_PATH "/mnt/vendor/rtdlog"
#else
#define RTDLOG_DEFAULT_PATH "/tmp/var/log"
#endif
#ifdef CONFIG_REALTEK_LOGBUF_MODULE
#define SUPPORT_LOGBUF_FILE_OPERAIONS
#define RTDLOG_NODE "rtd_logger"
#else
#define RTDLOG_NODE "rtd-logger"
#endif
#define RTDLOG_DEV_NODE "/dev/"RTDLOG_NODE
#define RTDLOG_DEBUG
//#define RTD_LOGBUF_PHY_START_ADDR 0x1ca00000

/********************************************************
* global variables
*********************************************************/
#if defined(CONFIG_REALTEK_LOGBUF) && !(LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
extern char *native_logbuf_addr;
#endif
extern struct MODULE_MAP module_map[RTD_LOG_MODULE_MAX_ID];
//static rtdlog_dbg_info gDbgInfo;
struct kfifo * array_fifo= NULL;
volatile u64 * gpRtdlogTimestamp;           //time stamp ptr
logbuf_rbcb * pRtdLogCB = NULL;
bool is_setup_rtd_logbuf = false;
bool is_save_early_log_done = false;           //printk save log to rtd logbuf when true only
EnumPanicCpuType g_rtdlog_panic_cpu_type = NORMAL;
EXPORT_SYMBOL(g_rtdlog_panic_cpu_type);
unsigned long rtd_virt_addr;
unsigned int total_logbuf_num = 0;
//default value 0x8000ff60 ,bit[31]Control Parameters of the effective bit[15]-[8] Control log write  to files,bit[7]-[0]Control log output console
static unsigned long bootcode_logswi; 
logbuf_filesize *pRtdLogfilesize;

#define RTDLOG_HEADER_BUF_SIZE 256
static unsigned short read_thread_sleep_span = 10;
static unsigned char flag_status = 1; //0 --> suspend, 1 --> running
static unsigned char flag_compress = 1;
unsigned int save_prelog_flag = 0;
static rtdlog_filter *cat_filter = NULL;
void rtdlog_crash_dump (int type);
static void rtdlog_filter_logblock(void);
static void dump_log_data (void);
void rtdlog_sleep (int type, long write_count );
#define RTKD_VERSION 20210915
#define RTDLOG_BUF_SIZE 256
#define FILE_NAME_LEN 16
typedef struct RTDLOG_FILE
{
    unsigned int pRtdLogCB;
    unsigned int file_len;
    unsigned int fd;
    unsigned int usb_fd;
    unsigned int gz_file_num;
    int flag_usb_sync;
    unsigned long long g_logfile_fp;
    unsigned long long g_logfile_usb_fp;
    char g_file_name[FILE_NAME_LEN];
}rtdlog_file;

struct SHARE_STRUCT
{
    unsigned int version;
    unsigned int total_file_num;
    unsigned char flag_compress;
    unsigned char usb_status_flag;
    unsigned short reserve2;
    unsigned int reserve3;
    char g_dir_path[RTDLOG_BUF_SIZE];
    char g_usb_dir_path[RTDLOG_BUF_SIZE];
    rtdlog_file file_info;
};

unsigned char * share_buffer;
rtdlog_file * file_info;
unsigned long rtdlog_bufsize;
unsigned long rtdlog_param_size;
static u8 kernel_cpu_type = 0xff;
static u8 hal_cpu_type = 0xff;
static u8 acpu1_cpu_type = 0xff;
static u8 kcpu2_cpu_type = 0xff;
static u8 vcpu1_cpu_type = 0xff;
static u8 vcpu2_cpu_type = 0xff;
static u8 vcpu3_cpu_type = 0xff;
static u8 kcpu_cpu_type = 0xff;
unsigned char * acpu1_cpu_log_buffer = NULL;
unsigned char * kcpu2_cpu_log_buffer = NULL;
unsigned char * vcpu1_cpu_log_buffer = NULL;
unsigned char * vcpu2_cpu_log_buffer = NULL;
unsigned char * vcpu3_cpu_log_buffer = NULL;
unsigned char * kcpu_cpu_log_buffer = NULL;
unsigned char * kernel_cpu_log_buffer = NULL;
unsigned char * hal_cpu_log_buffer = NULL;
/*
char *cpu_type_str[MAX_LOGBUF_NUM] =
{
    "ACPU1",
    "KCPU2",
    "VCPU1",
    "VCPU2",
    "VCPU3",
    "KCPU1",
    "S-KER",
    "S-HAL"
};
*/

#define RTDLOG_IOC_WRITE_CMD _IOWR(RTDLOG_DEV_MAGIC, 0, int )

#if 0
static struct file* g_logfile[MAX_LOGBUF_NUM];
static struct file* g_usb_logfile[MAX_LOGBUF_NUM];
static char g_file_path[LOGBUF_DIR_MAX_SIZE*2];

static loff_t g_fw_pos[MAX_LOGBUF_NUM];
static loff_t g_fw_usb_pos[MAX_LOGBUF_NUM];

#define RTDLOG_USB_LOGDIR "rtklog_usb"
static char g_usb_root_dir_path[LOGBUF_DIR_MAX_SIZE];
static char g_usb_dir_path[LOGBUF_DIR_MAX_SIZE];

#endif

#define READ_SLEEP_SIZE 0x800

#if 1
#define LOG_FILE_FW_MAX_SIZE   (512*1024)
#define LOG_FILE_KERNEL_MAX_SIZE      (1024*1024)
#else
#define LOG_FILE_FW_MAX_SIZE   (10*1024)
#define LOG_FILE_KERNEL_MAX_SIZE      (10*1024)
#endif

static DEFINE_MUTEX(save_acpu1_mutex);
static DEFINE_MUTEX(save_acpu2_mutex);
static DEFINE_MUTEX(save_vcpu1_mutex);
static DEFINE_MUTEX(save_vcpu2_mutex);
static DEFINE_MUTEX(save_vcpu3_mutex);
static DEFINE_MUTEX(save_kcpu_mutex);
static DEFINE_MUTEX(save_kcpu2_mutex);
static DEFINE_MUTEX(save_kernel_mutex);
static DEFINE_MUTEX(save_hal_mutex);
static DEFINE_MUTEX(save_hal_read_mutex);
static DEFINE_MUTEX(logcat_mutex);
static DEFINE_MUTEX(read_loop_mutex);
static DEFINE_MUTEX(keylog_mutex);

/*
struct mutex * log_save_mutex[MAX_LOGBUF_NUM]=
{
    &save_acpu1_mutex,
    &save_acpu2_mutex,
    &save_vcpu1_mutex,
    &save_vcpu2_mutex,
    &save_kcpu_mutex,
    &save_kernel_mutex,
    &save_hal_mutex
};
*/
#ifdef CONFIG_REALTEK_LOGBUF_MODULE
#if 0
static loff_t *rtd_file_ppos(struct file *file)
{
        return file->f_mode & FMODE_STREAM ? NULL : &file->f_pos;
}
static unsigned long rtd_fdget_pos(unsigned int fd)
{
    unsigned long v = __fdget(fd);
    struct file *file = (struct file *)(v & ~3);

    if (file && (file->f_mode & FMODE_ATOMIC_POS)) {
        if (file_count(file) > 1) {
            v |= FDPUT_POS_UNLOCK;
            mutex_lock(&file->f_pos_lock);
        }
    }
    return v;
}
static void rtd_unlock_pos(struct file *f)
{
    mutex_unlock(&f->f_pos_lock);
}
static void rtd_fdput_pos(struct fd f)
{
        if (f.flags & FDPUT_POS_UNLOCK)
                rtd_unlock_pos(f.file);
        fdput(f);
}
static struct fd rtd_to_fd(unsigned long v)
{
        return (struct fd){(struct file *)(v & ~3),v & 3};
}
static struct fd rtd_get_pos(int fd)
{
        return rtd_to_fd(rtd_fdget_pos(fd));
}
ssize_t rtk_ksys_write(unsigned int fd, const char *buf, size_t count)
{
        struct fd f = rtd_get_pos(fd);
        ssize_t ret = -EBADF;

        if (f.file) {
                loff_t pos, *ppos = rtd_file_ppos(f.file);
                if (ppos) {
                        pos = *ppos;
                        ppos = &pos;
                }
                ret = kernel_write(f.file, buf, count, ppos);
                if (ret >= 0 && ppos)
                        f.file->f_pos = pos;
               rtd_fdput_pos(f);
        }

        return ret;
}
#endif
ssize_t rtk_ksys_write(unsigned int fd, const char *buf, size_t count)
{
        return 0;
}
#else
static inline loff_t *rtk_file_ppos(struct file *file)
{
    return file->f_mode & FMODE_STREAM ? NULL : &file->f_pos;
}
ssize_t rtk_ksys_write(unsigned int fd, const char *buf, size_t count)
{
    struct fd f = fdget_pos(fd);
    ssize_t ret = -EBADF;

    if (f.file) {
        loff_t pos, *ppos = rtk_file_ppos(f.file);
        if (ppos) {
            pos = *ppos;
            ppos = &pos;
        }
        ret = kernel_write(f.file, buf, count, ppos);
        if (ret >= 0 && ppos)
            f.file->f_pos = pos;
        fdput_pos(f);
    }

        return ret;
}
#endif
/********************************************************
* kernel APIs
*********************************************************/
void* rtdlog_phys_to_virt (unsigned long phys)
{
    return (void * )( rtd_virt_addr + ( phys - RTD_LOGBUF_PHY_START_ADDR) );
}

#if 0
//#define UART_BASE 0x18062300
#define UART_BASE 0xb801b100
static void putc_ (char c)
{
    while (( rtd_inl(UART_BASE+0x14) & 32 ) == 0);
    if (c=='\n')
    {
        rtd_outl(UART_BASE,'\n');
        rtd_outl(UART_BASE,'\r');
    }
    else
    {
        rtd_outl(UART_BASE, c);
    }
}

static void puts_ (const char * str)
{
    for(;*str;str++)
        putc_(*str);
}
#endif

static int error_flag=0;
static int usb_error_flag=0;
ssize_t rtd_kernel_write_file (int type, unsigned char * buf, int count, ssize_t* result)
{
    ssize_t rv;

    if(file_info[type].usb_fd)
    {
        rv = rtk_ksys_write(file_info[type].usb_fd, buf, count);
        if(rv<0)
        {
            rtd_pr_rtdlog_err("%s %d. write usb log file %d \e[1;31mfail\e[0m! ret is %d %d %d\n",__func__,__LINE__,type,rv);
            file_info[type].usb_fd = 0;
        }
    }

    if(file_info[type].fd)
    {
        rv = rtk_ksys_write(file_info[type].fd, buf, count);
        if(rv<0)
        {
            rtd_pr_rtdlog_err("%s %d. write log file %d \e[1;31mfail\e[0m! ret is %d\n",__func__,__LINE__,type,rv);
            error_flag=rv;
            rv=0;
        }
    }
    else
    {
        rv=0;
    }

    return rv;
}

/********************************************************
* kfifo APIs
*********************************************************/
static inline unsigned int kfifo_out_len (struct kfifo *fifo)
{
    register unsigned int out;
    out = fifo->out;
    smp_rmb();
    return (fifo->in) - out;
}

static inline __must_check unsigned int kfifo_size (struct kfifo *fifo)
{
    return fifo->size;
}

static inline unsigned int kfifo_len (struct kfifo *fifo)
{
    return kfifo_out_len(fifo);
}

static inline unsigned int __kfifo_off (struct kfifo *fifo, unsigned int off)
{
    return off & (fifo->size - 1);
}

static inline __must_check unsigned int kfifo_avail (struct kfifo *fifo)
{
    unsigned int out_len;//,cat_len,use_len;
    out_len = kfifo_len(fifo);
    return kfifo_size(fifo) - out_len;
}

static inline void __kfifo_in_data (struct kfifo *fifo, const void *from, unsigned int len, unsigned int off)
{
    unsigned int l;

    /*
     * Ensure that we sample the fifo->out index -before- we
     * start putting bytes into the kfifo.
     */

    smp_mb();

    off = __kfifo_off(fifo, fifo->in + off);

    /* first put the data starting from fifo->in to buffer end */
    l = min((unsigned long)len, (unsigned long)(fifo->size - off));
    memcpy((unsigned char *)rtdlog_phys_to_virt((unsigned long)fifo->buffer) + off, from, l);

    /* then put the rest (if any) at the beginning of the buffer */
    memcpy((unsigned char *)rtdlog_phys_to_virt((unsigned long)fifo->buffer), from + l, len - l);
}

static inline void __kfifo_out_data (struct kfifo *fifo,void *to, unsigned int len, unsigned int off)
{
    unsigned int l;

    /*
     * Ensure that we sample the fifo->in index -before- we
     * start removing bytes from the kfifo.
     */

    smp_rmb();

    off = __kfifo_off(fifo, fifo->out + off);

    /* first get the data from fifo->out until the end of the buffer */
    l = min((unsigned long)len, (unsigned long)(fifo->size - off));
    memcpy(to, (unsigned char *)rtdlog_phys_to_virt((unsigned long)fifo->buffer) + off, l);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(to + l, (unsigned char *)rtdlog_phys_to_virt((unsigned long)fifo->buffer), len - l);
}

static inline void __kfifo_add_in (struct kfifo *fifo,unsigned int off)
{
    smp_wmb();
    fifo->in += off;
}

static inline void __kfifo_add_out (struct kfifo *fifo,unsigned int off)
{
    smp_mb();
    fifo->out += off;
}

static inline void __kfifo_cat_add_out (struct kfifo *fifo,unsigned int off)
{
    smp_mb();
    fifo->out_cat += off;
}

static unsigned int kfifo_in (struct kfifo *fifo, const void *from, unsigned int len)
{
    len = min(kfifo_avail(fifo), len);

    __kfifo_in_data(fifo, from, len, 0);
    __kfifo_add_in(fifo, len);

    return len;
}

static __maybe_unused unsigned int kfifo_out (struct kfifo *fifo, void *to, unsigned int len)
{
    len = min(kfifo_len(fifo), len);

    __kfifo_out_data(fifo, to, len, 0);
    __kfifo_add_out(fifo, len);
    return len;
}

static unsigned int kfifo_out_peek (struct kfifo *fifo, void *to, unsigned int len,unsigned offset)
{
    len = min(kfifo_len(fifo), len + offset);
    __kfifo_out_data(fifo, to, len, offset);
    return len;
}

static inline void kfifo_drop (struct kfifo *fifo)
{
    fifo->out = fifo->in;
}

static inline void kfifo_cat_sync (struct kfifo *fifo)
{
    fifo->out_cat = fifo->out;
}

/********************************************************
* MISC APIs
*********************************************************/
static char  logbuf_dir[LOGBUF_DIR_MAX_SIZE] = RTDLOG_DEFAULT_PATH;
int rtk_get_log_path (char *buf, int buf_len)
{
    if(strlen(logbuf_dir) == 0)
    {
        return -1;
    }

    strncpy(buf, logbuf_dir, buf_len-1);
    buf[buf_len - 1] = 0;
    return 0;
}
#ifdef CONFIG_REALTEK_LOGBUF
void rtdlog_parse_bufsize (char * str)
{
    unsigned long default_value[RTD_BUFSIZE_NUM]=
    {
        0x80000,
        0x100000,
        0x200000,
        0x400000
    };
    char *sub_str[RTD_BUFSIZE_NUM]=
    {
        "bufsize=80000",
        "bufsize=100000",
        "bufsize=200000",
        "bufsize=400000"
    };
    int i;

    for(i=0; i<RTD_BUFSIZE_NUM; i++)
    {
        if(strstr(str, sub_str[i])!= NULL)
        {
            //rtdlog_bufsize = default_value[i];
            rtdlog_bufsize = RTD_LOGBUF_DEFAULT_LIMIT << i ;
            rtdlog_param_size = RTD_LOGBUF_DEFAULT_PARAM << i ;
            //rtd_pr_rtdlog_err("get bufsize=%x, rtdlog_param_size=%x\n",rtdlog_bufsize, rtdlog_param_size);
            return;
        }
    }
    rtdlog_bufsize = default_value[0];
}
EXPORT_SYMBOL(rtdlog_parse_bufsize);
#endif
#if defined(CONFIG_REALTEK_LOGBUF_MODULE) || (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
void rtdlog_parse_bufsize_init(void)
{
    char strings[16] = {0};

    if(rtk_parse_commandline_equal("bufsize", strings,sizeof(strings)-1) == 0){
        rtd_pr_rtdlog_err("Error : can't get bufsize from bootargs\n");
        return;
    }
    rtd_pr_rtdlog_err("bufsize=**%s**\n",strings);
    if (kstrtoul(strings, 16, &rtdlog_bufsize)){
        rtd_pr_rtdlog_err("%s invalid str:%s\n",__FUNCTION__, strings);
        return;
    }

    rtdlog_param_size = (RTD_LOGBUF_DEFAULT_PARAM * rtdlog_bufsize / RTD_LOGBUF_DEFAULT_LIMIT);
    rtd_pr_rtdlog_err("rtdlog_bufsize=%lx, rtdlog_param_size=%lx\n",rtdlog_bufsize, rtdlog_param_size);
}
#endif
unsigned long rtdlog_get_buffer_size (void)
{
    return rtdlog_bufsize;
}
EXPORT_SYMBOL(rtdlog_get_buffer_size);
#if 1
struct workqueue_struct *p_dumpqueue = NULL;
struct rtd_logger_work_struct DumpWork_cpu[NORMAL];
int rtdlog_watchdog_dump_work(EnumPanicCpuType type)
{
    if (unlikely(p_dumpqueue == NULL))
    {
        return -EPERM;
    }

    DumpWork_cpu[type].param= type;
    queue_work(p_dumpqueue, &(DumpWork_cpu[type].work));
    return 0;
}
EXPORT_SYMBOL(rtdlog_watchdog_dump_work);
#endif
static void dump_ddr_range (char * msg,void * start,unsigned int len)
{
    char buf[RTD_BUFSIZE_512];
    int i = 0,j = 0;
    unsigned char * tmp = start;

    rtd_pr_rtdlog_err("%s\n",msg);
    for(i = 0; i < len; ++i)
    {
        if((i % 16 == 0)&&(i != 0))
        {
            buf[511] = 0;
            rtd_pr_rtdlog_err("%lx: %s\n",(unsigned long)(tmp-16),buf);
            j = 0;
        }
        if(j < sizeof(buf))
        {
            snprintf(&buf[j], RTD_BUFSIZE_512 - j," %02x", *tmp);
        }
        j += 3;
        tmp++;
    }

    if(i % 16 != 0)
    {
        buf[511] = 0;
        rtd_pr_rtdlog_err("%lx: %s\n",(unsigned long)(tmp-16),buf);
    }
}

/********************************************************
* rtdlog APIs
*********************************************************/
void update_rtdlog_timestamp(void)
{
    if(is_setup_rtd_logbuf)
    {
        *gpRtdlogTimestamp = local_clock();
    }
}

u32 get_msghead_sum(rtd_msg_header * header)
{
    int i;
    u32 val = 0;

    val += header->cpu_core;
    val += header->cpu_type;
    val += header->len;
    val += header->level;
    val += header->module;
    val += header->pid;
    val += header->tid;
    val += header->text_len;
    val += (header->timestamp & 0xffffffff);
    val += (header->timestamp>>32)&0xffffffff;
    val += (header->magic_header & 0xffffffff);
    val += (header->magic_header>>32)&0xffffffff;

    for(i = 0; i < sizeof(header->name); ++i)
    {
        val += header->name[i];
    }

    return val;
}

static int check_msghead_sum(rtd_msg_header * header)
{
    u32 sum_store = header->check_sum;
    u32 sum_cacu = get_msghead_sum(header);

    if(sum_store == sum_cacu)
    {
        return 0;
    }

    return -1;
}

static void dump_rtd_msg_header (rtd_msg_header * start,unsigned int len)
{
    char buf[81];
    int i , j=0;

    for(i=0; i<16; i++)
    {
        snprintf(&buf[j], sizeof(buf)-j,"0x%02x ",start->name[i]);
        j += 5;
    }

    rtd_pr_rtdlog_err("*****************dump_rtd_msg_header start**********************\n");
    rtd_pr_rtdlog_err("magic_header = 0x%016llx\n",start->magic_header);
    rtd_pr_rtdlog_err("timestamp = 0x%016llx\n",start->timestamp);
    rtd_pr_rtdlog_err("len = 0x%08lx\n",start->len);
    rtd_pr_rtdlog_err("pid = 0x%08lx\n",start->pid);
    rtd_pr_rtdlog_err("tid = 0x%08lx\n",start->tid);
    rtd_pr_rtdlog_err("module = 0x%08lx\n",start->module);
    rtd_pr_rtdlog_err("text_len = 0x%08lx\n",start->text_len);
    rtd_pr_rtdlog_err("check_sum = 0x%08lx\n",start->check_sum);
    rtd_pr_rtdlog_err("name = %s\n",buf);
    rtd_pr_rtdlog_err("level = 0x%02x\n",start->level);
    rtd_pr_rtdlog_err("cpu_type = 0x%02x\n",start->cpu_type);
    rtd_pr_rtdlog_err("cpu_core = 0x%02x\n",start->cpu_core);
    rtd_pr_rtdlog_err("msg_count = 0x%02x\n",start->msg_count);
    rtd_pr_rtdlog_err("*****************dump_rtd_msg_header end***********************\n");
}

static int check_msghead_valid ( rtd_msg_header * header, bool debug)
{
    if((header->len > header->text_len)&&((header->len - header->text_len) == sizeof(rtd_msg_header)))
    {
        if(check_msghead_sum(header))
        {
            rtd_pr_rtdlog_err("%s %d. msg header checksum error\n",__func__,__LINE__);
            goto check_fail;
        }
        else
        {
            goto check_pass;
        }
    }
    else if (debug)
    {
        rtd_pr_rtdlog_err("%s %d. msg header len error %x|%x  %x\n",__func__,__LINE__,header->len,header->text_len,sizeof(rtd_msg_header));
    }

check_fail:
    return -1;
check_pass:
    return 0;
}


// 1:PASS ; -1:VERIFY FAIL ; 0:EMPTY
int get_msg_header (int type,rtd_msg_header* header, struct kfifo * fifo)
{
    unsigned int offset=(fifo->out)%(fifo->size);
    unsigned char* start= rtdlog_phys_to_virt((unsigned long)fifo->buffer);
    //unsigned char* end = rtdlog_phys_to_virt((unsigned long)fifo->buffer)+fifo->size;
    unsigned char* out = start + offset ;

    if (kfifo_len(fifo) > sizeof(rtd_msg_header))
    {
        kfifo_out_peek(fifo,header,sizeof(rtd_msg_header),0);
        if( check_msghead_valid(header,true) )
        {
            goto FAIL;
        }

#if 0
        if( (kfifo_len(fifo) < header->len))
        {
            rtd_pr_rtdlog_err("%s %d. fifo len \n",__func__,__LINE__);
            goto FAIL;
        }
#endif

        return 1;

FAIL:
        rtd_pr_rtdlog_err("offset=%x\n",offset);
        dump_ddr_range("dump more",(void *)(out-0x20),0x100);
        dump_rtd_msg_header(header,sizeof(rtd_msg_header));

        kfifo_drop(fifo);
        pRtdLogCB->in_reset++;
        rtd_pr_rtdlog_err("type[\e[1;31m%s\e[0m] check msg header fail!\n\n\n",file_info[type].g_file_name);
        return -1;
    }
    return 0;
}

static int rtd_get_savelevel(int type)
{
        logbuf_rbcb * pRbcb;
        int savelevel = RTD_DEFAULT_SAVELEVEL;

        if(type < total_logbuf_num){
            pRbcb = rtdlog_phys_to_virt(file_info[type].pRtdLogCB);
            savelevel = pRbcb->savelevel;
        }

        return savelevel;
}

int rtdlog_save_msg (int type, const char * text, rtd_msg_header * header)
{
    struct kfifo * fifo=NULL;
    //struct mutex * mtx=log_save_mutex[type];
    static unsigned char ker_num = 0;
    static unsigned char hal_num = 0;
    logbuf_rbcb * RtdLogCB = NULL;

    if(!is_setup_rtd_logbuf)
    {
        return -1;
    }

    if(type == 0xff)
    {
        return 0;
    }

    if(rtd_get_savelevel(type) < (header->level))
    {
        return 0;
    }

    RtdLogCB = rtdlog_phys_to_virt(file_info[type].pRtdLogCB);
    fifo = &(RtdLogCB->fifo);

#if 0
    if((type == HAL_BUF)||(type == KERNEL_BUF))
    {
        if(check_log_filter(header,log_filter))
        {
            return 0;
        }
    }
#endif

    //mutex_lock(mtx);
    if (kfifo_avail(fifo) > (header->len))
    {
        if(type == kernel_cpu_type)
            header->msg_count = ker_num++;
        else if(type == hal_cpu_type)
            header->msg_count = hal_num++;

        header->check_sum = get_msghead_sum(header);

        kfifo_in(fifo,header,sizeof(rtd_msg_header));
        kfifo_in(fifo,text,header->text_len);

#if 0
        gDbgInfo.save_log_len[type] += header->text_len;
        gDbgInfo.save_log_cnt[type]++;
#endif
    }
    else
    {
        RtdLogCB->drop_cnt++;
    }
    //mutex_unlock(mtx);

    return 0;
}
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS)
DEFINE_RAW_SPINLOCK(logbuf_lock);
#endif
void rtd_logbuf_save_log ( int level, const char *text, u16 text_len)
{
    rtd_msg_header tmp_header;
    //struct mutex * mtx = &save_kernel_mutex;
    if(is_save_early_log_done==0)
    {
        //if(is_setup_rtd_logbuf==0){
        return;
    }

    /*rtdlog buf msg*/
    tmp_header.magic_header = RTD_LOGBUF_MAGIC_HEADER;
    tmp_header.cpu_type = (u8)kernel_cpu_type;
    tmp_header.cpu_core = smp_processor_id();
    tmp_header.module = 0;
    tmp_header.pid = task_tgid_nr(current);
    tmp_header.tid = task_pid_nr(current);
    tmp_header.timestamp = *gpRtdlogTimestamp;
    tmp_header.level = level;
    tmp_header.text_len = text_len;
    tmp_header.len = sizeof(tmp_header) + text_len;

    //memset(tmp_header.name,'#',sizeof(tmp_header.name));
    snprintf(tmp_header.name,sizeof(tmp_header.name),current->comm);
    //memcpy(tmp_header.name,current->comm,sizeof(tmp_header.name));
    //tmp_header.name[sizeof(tmp_header.name)-1] = '\0';
    //mutex_lock(mtx);
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS)
    raw_spin_lock(&logbuf_lock);
#endif
    rtdlog_save_msg(kernel_cpu_type,text, &tmp_header);
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS)
    raw_spin_unlock(&logbuf_lock);
#endif
    //mutex_unlock(mtx);
}
EXPORT_SYMBOL(rtd_logbuf_save_log);

#if defined(CONFIG_REALTEK_LOGBUF_MODULE) || (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static struct timer_list logbuffer_timer;
static void rtk_logbuffer_timer_func (struct timer_list *t)
{
    update_rtdlog_timestamp();
    mod_timer(&logbuffer_timer, (jiffies + msecs_to_jiffies(20)));
}
static int __init init_logbuffer_timer (void)
{
    timer_setup(&logbuffer_timer, rtk_logbuffer_timer_func, 0);
    //setup_timer(&logbuffer_timer, rtk_logbuffer_timer_func, 0); //code for < 5.4
    mod_timer(&logbuffer_timer, (jiffies + msecs_to_jiffies(10)));
    return 0;
}

#include <linux/kmsg_dump.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
    static struct kmsg_dump_iter dumper;
#else
    static struct kmsg_dumper dumper = { .active = 1 };
#endif
void rtd_logbuf_save_log( int level, const char *text, u16 text_len);
void rtd_get_native_log (void)
{
    int count=10;
    size_t len;
    unsigned char buf[1024];

    while (count--)
    {
        //local_irq_disable();
        memset(buf,0,sizeof(buf));
        if(!kmsg_dump_get_line(&dumper, false, buf, sizeof(buf)-2, &len))
        {
            //local_irq_enable();
            return;
        }

        if(!strlen(buf)){
            continue;
        }

        //local_irq_enable();
        rtd_logbuf_save_log(0,buf,strlen(buf));
    }
}

int thread_logbuf_reader(void* p)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
    kmsg_dump_rewind(&dumper);
#endif
    for(;;)
    {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        rtd_get_native_log();
#endif
        rtdlog_sleep(0,0);
    }
    return 0;
}
#endif
/********************************************************
* rtdlog cat related APIs
*********************************************************/
static inline unsigned int kfifo_cat_len(struct kfifo *fifo)
{
    register unsigned int out_cat;
    out_cat = fifo->out_cat;
    smp_rmb();
    return (fifo->in) - out_cat;
}

static inline void __kfifo_cat_out_data(struct kfifo *fifo,void *to, unsigned int len, unsigned int off)
{
    unsigned int l;

    /*
     * Ensure that we sample the fifo->in index -before- we
     * start removing bytes from the kfifo.
     */

    smp_rmb();

    off = __kfifo_off(fifo, fifo->out_cat + off);

    /* first get the data from fifo->out until the end of the buffer */
    l = min((unsigned long)len, (unsigned long)(fifo->size - off));
    memcpy(to, (unsigned char *)rtdlog_phys_to_virt((unsigned long)fifo->buffer) + off, l);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(to + l, (unsigned char *)rtdlog_phys_to_virt((unsigned long)fifo->buffer), len - l);
}

static unsigned int kfifo_cat_out_peek (struct kfifo *fifo, void *to, unsigned int len, unsigned offset)
{
    len = min(kfifo_cat_len(fifo), len + offset);
    __kfifo_cat_out_data(fifo, to, len, offset);
    return len;
}

static inline void __kfifo_add_cat_out (struct kfifo *fifo,unsigned int off)
{
    smp_mb();
    fifo->out_cat += off;
}

#if 0
static unsigned int kfifo_cat_out(struct kfifo *fifo, void *to, unsigned int len)
{
    len = min(kfifo_cat_len(fifo), len);

    __kfifo_cat_out_data(fifo, to, len, 0);
    __kfifo_cat_add_out(fifo, len);
    return len;
}
#endif

//RET VAL:
// 1:PASS ; -1:VERIFY FAIL; 0:EMPTY
int get_catmsg_header (int type, rtd_msg_header* header)
{
    struct kfifo * fifo=array_fifo;
    if (kfifo_cat_len(fifo) > sizeof(rtd_msg_header))
    {
        kfifo_cat_out_peek(fifo,header,sizeof(rtd_msg_header),0);
        if(check_msghead_valid(header,1))//if fail
        {
            rtd_pr_rtdlog_err("type[%d] check logcat msg header fail!\n",type);
            dump_ddr_range("dump header ddr",(void *)header,sizeof(rtd_msg_header));
            return -1;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        //rtd_pr_rtdlog_err("FAIL:[%s](%d)...\n",__FUNCTION__,__LINE__);
    }

    return 0;
}

static char cat_buffer[2048];
int rtd_build_msg_header (rtd_msg_header* header, char* msg_header);
unsigned int rtd_logcat_data_get (int type, rtd_msg_header* header, char __user * buf, size_t count ) 
{
    struct kfifo * fifo = array_fifo;
    //unsigned int len,ret_len,offset,eoff;
    int pos;
    unsigned int out_len,ret;

    if((header->text_len) > 1536)
    {
        rtd_pr_rtdlog_err("%s %d.  drop type %s's log\n",__func__,__LINE__,file_info[type].g_file_name);
        __kfifo_add_cat_out(fifo, header->len);
        return 0;
    }

    //memset(msg_tag,0,sizeof(msg_tag));
    pos=rtd_build_msg_header(header,cat_buffer);
    __kfifo_add_cat_out(fifo, sizeof(rtd_msg_header));

    //memset(buffer,0,sizeof(msg_tmp_buf));
    out_len=kfifo_cat_out_peek(fifo,cat_buffer+pos,header->text_len,0);
    __kfifo_add_cat_out(fifo, out_len);

    if(cat_buffer[pos+out_len-1]!='\n')
    {
        cat_buffer[pos+out_len]='\n';
        out_len++;
    }
    cat_buffer[pos+out_len]=0;

    ret=((pos+out_len)>count)?(count):(pos+out_len);
    if(copy_to_user(buf, (void *)cat_buffer, ret ))
    {
        rtd_pr_rtdlog_err("%s %d.  copy_to_user  err\n",__func__,__LINE__);
        return 0;
    }
    //rtd_pr_rtdlog_err("%s %d %s\n",__func__,__LINE__,cat_buffer);
    return ret;
    //kfree(buffer);
}
/*
** set/modify log filter
*/
static void rtdlog_filter_caton(rtdlog_sysnode_param * param)
{
        int i;
        if(NULL == cat_filter)
        {
            rtd_pr_rtdlog_err("%s %d. cat_filter = NULL !\n",__func__,__LINE__);
            return;
        }
        for(i = 0; i < total_logbuf_num; ++i) {
                //enable log output
                if(param->buf_idx[i]) {
                        cat_filter->main_mask &= ~(1 << i);
                        if(param->core_modify) {
                                cat_filter->sub_mask[i].core_mask = param->core_mask;
                        }
                        if(param->module_modify) {
                                cat_filter->sub_mask[i].module_mask = param->module_mask;
                        }
                        if(param->level_modify) {
                                cat_filter->sub_mask[i].level = param->level;
                        }
                }
        }
}

/*
** unset buffer types in log filter
*/
static void rtdlog_filter_catoff(rtdlog_sysnode_param * param)
{
        int i;
        if(NULL == cat_filter)
        {
            rtd_pr_rtdlog_err("%s %d. cat_filter = NULL !\n",__func__,__LINE__);
            return;
        }
        for(i = 0; i < total_logbuf_num; ++i) {
                //diable log output
                if(param->buf_idx[i]) {
                        cat_filter->main_mask |= (1 << i);
                }
        }
}

static void catfilter_setup(void)
{
        int i = 0;
        cat_filter = kmalloc(sizeof(rtdlog_filter)+total_logbuf_num*sizeof(filter_cpu),GFP_KERNEL);
        if(NULL == cat_filter)
        {
            rtd_pr_rtdlog_err("%s %d. cannot kmalloc cat_filter memory !\n",__func__,__LINE__);
            return;
        }
        memset(cat_filter, 0, sizeof(rtdlog_filter)+total_logbuf_num*sizeof(filter_cpu));
        cat_filter->buf_cat = 0;
        //cat_filter.main_mask = 0;
        for(i=0;i<total_logbuf_num;i++)
        {
            cat_filter->sub_mask[i].core_mask = 0x0;
            cat_filter->sub_mask[i].level = 7;
            cat_filter->sub_mask[i].module_mask = 0xFFFFFFFF;
        }
}

static int check_log_filter(rtd_msg_header *header,rtdlog_filter *log_filter,int type)
{

    int ret = 0;
    u8 level = 0;
    u32 module = 0;
    if((header == NULL)||(log_filter == NULL)){
        return ret;
    }

    level = header->level;
    module = header->module;

    if((1 << type)&log_filter->main_mask) {
            if(level <= log_filter->level_mask){
                    ret = 0;
            }else{
                    ret = -1;
            }
    } else {
            if(level > log_filter->sub_mask[type].level) {
                    ret = -1;
            }else if((~(log_filter->sub_mask[type].module_mask)) & module) {
                    ret = -1;
            }
    }

        return ret;
}

void skip_catmsg_data (int type,rtd_msg_header* header)
{
    struct kfifo * fifo=array_fifo;

    if (kfifo_cat_len(fifo)< header->len)
    {
        return ;
    }

    __kfifo_cat_add_out(fifo, header->len);

    return ;
}

ssize_t rtd_logcat_data (int type, char __user * buf, size_t count )
{
    rtd_msg_header header;
    ssize_t ret_len = -1;
    int header_check = 0;
    pRtdLogCB = rtdlog_phys_to_virt(file_info[type].pRtdLogCB);
    array_fifo= &(pRtdLogCB->fifo);
    header_check = get_catmsg_header(type,&header);
    if (1 == header_check )
    {
#if 1
        if(check_log_filter(&header,cat_filter,type))
        {
            skip_catmsg_data(type,&header);
            ret_len = 0;
        }
        else
#endif

        {
            ret_len = rtd_logcat_data_get(type,&header,buf, count);
        }
    }
    else if ( -1 == header_check )
    {
        //mutex_lock(&logcat_mutex);
        kfifo_cat_sync(array_fifo);
        //mutex_unlock(&logcat_mutex);
    }

    return ret_len;
}


/********************************************************
* rtdlog file operations APIs
*********************************************************/
int rtd_build_msg_header (rtd_msg_header* header, char* msg_header)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
    struct __kernel_old_timeval log_time=ns_to_kernel_old_timeval((s64)header->timestamp);
#else
    struct timeval log_time=ns_to_timeval((s64)header->timestamp);
#endif
    int pos=0, len = 0;
    logbuf_rbcb * RtdLogCB = NULL;

    if(total_logbuf_num > header->cpu_type)
    {
        RtdLogCB = rtdlog_phys_to_virt(file_info[header->cpu_type].pRtdLogCB);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)) &&(!defined(CONFIG_ANDROID_VENDOR_HOOKS))
        if(header->cpu_type == kernel_cpu_type)
        {
            len = snprintf(msg_header+pos, RTDLOG_HEADER_BUF_SIZE - pos, "<%d>(%5s:",
                       header->level,
                       (char *)(file_info[header->cpu_type].g_file_name));
        }
        else
#endif 
        {
            len = snprintf(msg_header+pos, RTDLOG_HEADER_BUF_SIZE - pos, "<%d>[%08d.%06d](%5s:",
                       header->level,
                       (int)log_time.tv_sec,//(u32)(header->timestamp>>32),
                       (int)log_time.tv_usec,//(u32)(header->timestamp&0xffffffff),
                       (char *)(file_info[header->cpu_type].g_file_name));
        }
    }
    else
    {
        RtdLogCB = rtdlog_phys_to_virt(file_info[total_logbuf_num-1].pRtdLogCB);
        len = snprintf(msg_header+pos, RTDLOG_HEADER_BUF_SIZE - pos, "<%d>[%08d.%06d](%5s:",
                   header->level,
                   (int)log_time.tv_sec,//(u32)(header->timestamp>>32),
                   (int)log_time.tv_usec,//(u32)(header->timestamp&0xffffffff),
                   "unknow");
    }
    pos += len;

    if(header->cpu_core != 0xff)
    {
        len = snprintf(msg_header+pos, RTDLOG_HEADER_BUF_SIZE - pos, "%d)",header->cpu_core);
    }
    else
    {
        len = snprintf(msg_header+pos, RTDLOG_HEADER_BUF_SIZE - pos, "?)");
    }
    pos += len;
    if ( header->cpu_type == kernel_cpu_type)
    {
        len = snprintf(msg_header+pos, RTDLOG_HEADER_BUF_SIZE - pos, " [%03d][drop:%d rst:%d] #%-15s# ",(unsigned char)(header->msg_count),RtdLogCB->drop_cnt,RtdLogCB->in_reset,header->name);
        pos += len;

        //len = snprintf(msg_header+pos, 256 - pos, "[%08x]: ", header->module);
        //pos += len;
    }
    else
    {
        len = snprintf(msg_header+pos, RTDLOG_HEADER_BUF_SIZE - pos, " [%03d][drop:%d rst:%d] ",(unsigned char)(header->msg_count),RtdLogCB->drop_cnt,RtdLogCB->in_reset);
        pos += len;
    }
    return pos;
}

#if 1
static unsigned char msg_tmp_buf[2048];
static ssize_t kfifo_out_peek_write ( int type, struct kfifo *fifo, rtd_msg_header* header)
{
    unsigned char * buffer=msg_tmp_buf;
    unsigned int out_len;
    int pos;
    ssize_t ret=0, result;

    if((header->text_len) > 1536)
    {
        rtd_pr_rtdlog_err("%s %d.  drop type %s's log\n",__func__,__LINE__,file_info[type].g_file_name);
        __kfifo_add_out(fifo, header->text_len);
        return 0;
    }

    //memset(msg_tag,0,sizeof(msg_tag));
    pos=rtd_build_msg_header(header,buffer);

    //memset(buffer,0,sizeof(msg_tmp_buf));
    out_len=kfifo_out_peek(fifo,buffer+pos,header->text_len,0);
    __kfifo_add_out(fifo, out_len);

    if(buffer[pos+out_len-1]!='\n')
    {
        buffer[pos+out_len]='\n';
        out_len++;
    }
    buffer[pos+out_len]=0;
    if((bootcode_logswi>>8) & (1<<type))
    {
        ret=rtd_kernel_write_file(type, buffer, pos+out_len,&result);
    }
    if(strcmp((char *)pRtdLogCB->buf_name,"kernel") != 0)
    {
        if(bootcode_logswi & (1<<type))
        {
            rtd_pr_rtdlog_err("[%s]%s",file_info[type].g_file_name, buffer);
        }
    }
    return ret;
}

#else
#endif

ssize_t rtd_write_msg_data ( int type, rtd_msg_header* header, struct kfifo * fifo )
{
    __kfifo_add_out(fifo, sizeof(rtd_msg_header));
    return kfifo_out_peek_write(type, fifo,header);
}

long rtd_logfile_size_check ( int type, ssize_t ret )
{
    long check_size=(kernel_cpu_type==type)?(LOG_FILE_KERNEL_MAX_SIZE):(LOG_FILE_FW_MAX_SIZE);
    if(strcmp(file_info[type].g_file_name,"kernel") == 0)
        check_size = pRtdLogfilesize->total_file;
    else if(strcmp(file_info[type].g_file_name,"acpu1") == 0)
        check_size = pRtdLogfilesize->acpu1_file;
    else if(strcmp(file_info[type].g_file_name,"kcpu2") == 0)
        check_size = pRtdLogfilesize->acpu2_file;
    else if(strcmp(file_info[type].g_file_name,"vcpu1") == 0)
        check_size = pRtdLogfilesize->vcpu1_file;
    else if(strcmp(file_info[type].g_file_name,"vcpu2") == 0)
        check_size = pRtdLogfilesize->vcpu2_file;

    file_info[type].file_len+=ret;
    if(file_info[type].file_len >= check_size)
    {
        file_info[type].file_len=0;
        return 1;
    }

    return 0;
}

ssize_t rtd_write_data (int type)
{
    rtd_msg_header header;
    int header_check;
    ssize_t ret=0;
    int i,cnt;

    pRtdLogCB = rtdlog_phys_to_virt(file_info[type].pRtdLogCB);
    array_fifo= &(pRtdLogCB->fifo);
    if(kfifo_avail(array_fifo)<(array_fifo->size/2))    
        cnt = 5;
    else
        cnt = 1;

    for(i=0;i<cnt;i++)
    {
        header_check = get_msg_header(type,&header, array_fifo);
        if(header_check==0)
        {
            return ret;
        }
        else if (header_check==-1)
        {
            rtd_pr_rtdlog_err("%s %d. error happen! insert miss log msg for type:%d\n",__func__,__LINE__,type);
            return -2;
        }

        //mutex_lock(&logcat_mutex);
        ret=rtd_write_msg_data(type, &header, array_fifo);
        //mutex_unlock(&logcat_mutex);

    #if 0
        gDbgInfo.file_log_len[type] += header.text_len;
        gDbgInfo.file_log_cnt[type]++;
        gDbgInfo.read_log_len[type] += header.text_len;
        gDbgInfo.read_log_cnt[type]++;
    #endif
        if(rtd_logfile_size_check(type,ret))
        {
            return type+0x1;
        }
    }
    return 0;
}

void rtdlog_sleep (int type, long write_count )
{
#if 0
    if(KERNEL_BUF!=type)
    {
        return;
    }

    if(!(g_fw_pos[type] % READ_SLEEP_SIZE))
    {
        msleep(20);
    }
#else
    set_freezable();
    usleep_range(40*read_thread_sleep_span, 80*read_thread_sleep_span);//msleep_interruptible(10);
#endif

}

//#define RTD_TIME_CHECK
unsigned int drvif_Get_90k_Lo_clk(void);
int64_t MEMC_GetPTS(void);
long rtdlog_ioctl_write_type (int type)
{
#ifdef RTD_TIME_CHECK
    int64_t t1,t2;
#endif
    struct mutex * mtx = &read_loop_mutex;
    ssize_t ret;

#ifdef RTD_TIME_CHECK
    t1=MEMC_GetPTS();//drvif_Get_90k_Lo_clk();
#endif
    mutex_lock(mtx);
    ret=rtd_write_data(type);
    mutex_unlock(mtx);

#ifdef RTD_TIME_CHECK
    t2=MEMC_GetPTS();//drvif_Get_90k_Lo_clk();
    rtd_pr_rtdlog_err("%s %d.  type %s cost %d ms\n",__func__,__LINE__,file_info[type].g_file_name,(int)((t2-t1)*1000/90));
#endif

    rtdlog_sleep(type,ret);

    return ret;
}

#ifdef CONFIG_RTK_USBDUMP_ENABLE
int rtk_get_usb_path (char *buf, int buf_len);
int rtd_check_usb_logfile (void)
{
    static int flag_usb=0;
    struct SHARE_STRUCT* p_share = (struct SHARE_STRUCT*) share_buffer;
    if(NULL==p_share)
    {
        return 0;
    }

    if ((0==flag_usb) && !rtk_get_usb_path(p_share->g_usb_dir_path,LOGBUF_DIR_MAX_SIZE) )
    {
        flag_usb=1;
        return 1;
    }

    if((1==flag_usb) && rtk_get_usb_path(NULL,0) )
    {
        flag_usb=0;
        memset(p_share->g_usb_dir_path,0,LOGBUF_DIR_MAX_SIZE);
        return 2;
    }

    return 0;
}
#endif
int rtd_check_usb_changeflag (void)
{
    struct SHARE_STRUCT* p_share = (struct SHARE_STRUCT*) share_buffer;
    if(NULL==p_share)
    {
        return 0;
    }

    if ((p_share->usb_status_flag) == 1 )  // flag set int rtkd 2,Check whether the USB is plugged and removed
    {
        return 1;
    }

    return 0;
}
long rtdlog_ioctl_write (unsigned long arg)
{
    int type;
    long ret;
    error_flag=0;
    usb_error_flag=0;
    //rtd_pr_rtdlog_err("%s %d\n",__func__,__LINE__);
    while(1)
    {
        if(rtd_check_usb_changeflag())
        {
            return 0;
        }
        for(type=0; type<total_logbuf_num; type++)
        {
            if((ret=rtdlog_ioctl_write_type(type))>0)
            {
                return ret;
            }
            if(error_flag<0)
            {
                rtd_pr_rtdlog_err("%s %d.  got error %d\n",__func__,__LINE__,error_flag);
                return error_flag;
            }

#ifdef CONFIG_RTK_USBDUMP_ENABLE
            if(rtd_check_usb_logfile())
            {
                return 0;
            }
#endif

            if(usb_error_flag == EIO)
            {
                rtd_pr_rtdlog_err("%s %d. usb write log error %d\n",__func__,__LINE__,usb_error_flag);
                return usb_error_flag;
            }
        }
    }
    return 0;
}

long rtdlog_ioctl ( struct file*filp, unsigned int cmd, unsigned long arg)
{
    long ret=-EINVAL;
    switch(cmd)
    {
        case RTDLOG_IOC_WRITE_CMD:
            ret=rtdlog_ioctl_write(arg);
            break;
        default:
            rtd_pr_rtdlog_err( "%s %d. wrong ioctrol cmd\n",__func__,__LINE__);
    }

    return ret;
}

int rtdlog_open ( struct inode* inode, struct file* filp )
{
    struct rtdlog_dev *pdev = container_of(inode->i_cdev, struct rtdlog_dev, cdev);
    filp->private_data = pdev; /* for other methods */

    //rtd_pr_rtdlog_warn("rtdlog device open\n");
    return 0;
}
int rtdlog_release ( struct inode* inode, struct file* filp)
{
    filp->private_data = NULL;
    return 0;
}

ssize_t rtdlog_read (struct file * filp, char __user * buf, size_t count, loff_t * f_pos)
{
    int i = 0;
    static int j = 0;
    ssize_t len = 0;
    static int need_sync = 1;
    struct mutex * mtx = &read_loop_mutex;

    if(need_sync)
    {
        for(i = 0; i < total_logbuf_num; i++)
        {
            //mutex_lock(&logcat_mutex);
            pRtdLogCB = rtdlog_phys_to_virt(file_info[i].pRtdLogCB);
            array_fifo= &(pRtdLogCB->fifo);
            kfifo_cat_sync(array_fifo);
            //mutex_unlock(&logcat_mutex);
        }
        need_sync = 0;
    }
    mutex_lock(mtx);
    while(1)
    {
        j %= total_logbuf_num;
        len = rtd_logcat_data(j++, buf, count );
        if (len > 0)
        {
            break;
        }

        if (signal_pending(current))
        {
            need_sync = 1;
            break;
        }
    }
    mutex_unlock(mtx);
    return len;
}

ssize_t rtdlog_write (struct file * filp, const char __user * buf, size_t count, loff_t * f_pos)
{
    u32 info_size = sizeof(rtd_logbuf_hal_info);
    u32 text_len;
    const char * text;
    rtd_msg_header tmp_header;
    rtd_logbuf_hal_info info;
    char databuf[RTD_BUFSIZE_512] = {0};
    unsigned long copy_ret;
    struct mutex * mtx = &save_hal_mutex;

    if(hal_cpu_type == 0xff)
        return 0;

    if(is_setup_rtd_logbuf)
    {
        if(count < info_size)
        {
            rtd_pr_rtdlog_err("%s : lost hal head info!\n", __FUNCTION__);
            return 0;
        }

        if(count > RTD_BUFSIZE_512)
        {
            rtd_pr_rtdlog_err("%s : hal log length:%d > 512!\n", __FUNCTION__, (unsigned int)count);
            return 0;
        }
        //parse buffer info.
        copy_ret = copy_from_user(databuf, buf, count);

        if(copy_ret)
        {
            rtd_pr_rtdlog_err("error : copy_from_user return 0x%x,drop 1 msg\n",(unsigned int)copy_ret);
            return 0;
        }

        //level,text_len,cpu_core,text
        memcpy(&info, databuf, info_size);

        /*****special process for hal buffer,because hal log is too much*****
        ***** we will block the log into hal buffer,if not match filter *****
        ********************************************************************/

        text_len = count - info_size;
        text = databuf + info_size;

        //save log
        tmp_header.magic_header = RTD_LOGBUF_MAGIC_HEADER;
        tmp_header.cpu_type = (u8)hal_cpu_type;
        tmp_header.cpu_core = info.core;
        tmp_header.pid = task_tgid_nr(current);
        tmp_header.tid = task_pid_nr(current);
        tmp_header.timestamp = *gpRtdlogTimestamp;
        tmp_header.level = info.level;
        tmp_header.module = info.module;
        tmp_header.text_len = text_len;
        tmp_header.len = sizeof(tmp_header) + text_len;

        memset(tmp_header.name,'#',sizeof(tmp_header.name));
        memcpy(tmp_header.name,current->comm,sizeof(tmp_header.name));
        tmp_header.name[sizeof(tmp_header.name)-1] = '\0';

        mutex_lock(mtx);
        rtdlog_save_msg(hal_cpu_type, text, &tmp_header);
        mutex_unlock(mtx);
    }
    else
    {
        //rtd_pr_rtdlog_err("rtd_log buf is not setup yet...\n");
        return 0;
    }

    return tmp_header.len;
}

static int rtdlog_mmap (struct file *file, struct vm_area_struct *vma)
{
    if(save_prelog_flag==1)
    {
        dump_log_data();
    }
    if(NULL==share_buffer)
    {
        return -EAGAIN;
    }
    if ((vma->vm_end - vma->vm_start)!=PAGE_SIZE)
    {
        rtd_pr_rtdlog_err("%s %d. only kmalloc %x  byte for  share memory for rtkd !  %x\n",__func__,__LINE__,(unsigned int)PAGE_SIZE,(unsigned int)(vma->vm_end - vma->vm_start));
        return -EAGAIN;
    }

    if(remap_pfn_range(vma,vma->vm_start,
                       virt_to_phys(share_buffer)>>PAGE_SHIFT,
                       vma->vm_end - vma->vm_start, vma->vm_page_prot))
    {
        rtd_pr_rtdlog_err("%s %d. cannot remap share memory for rtkd !\n",__func__,__LINE__);
        return -EAGAIN;
    }
    return 0;
}


/********************************************************
* rtdlog keylog APIs
*********************************************************/

static unsigned int rtd_keylog_pos = 0; // record keylog buffer offset
static unsigned int rtd_keylog_last_pos = 0; // record last boot buffer offset
unsigned char * rtd_keylog_addr = NULL;
int init_rtd_keylog_buf(void)
{
    struct mutex * mtx = &keylog_mutex;

    mutex_lock(mtx);
    rtd_keylog_pos = 0;
    rtd_keylog_last_pos = 0;
    rtd_keylog_addr = (unsigned char *)(rtdlog_phys_to_virt(RTD_LOGBUF_PHY_START_ADDR + RTD_LOGBUF_KEYLOG_OFFSET));
    rtd_pr_rtdlog_warn("keylog start addr : 0x%08x, size:0x%x\n", (RTD_LOGBUF_PHY_START_ADDR + RTD_LOGBUF_KEYLOG_OFFSET), RTD_LOGBUF_KEYLOG_SIZE);

    //dump_ddr_range("keylog before compare:", rtd_keylog_addr, 0x100);
    *(rtd_keylog_addr+RTD_LOGBUF_KEYLOG_SIZE-1) = '\0';     // for strlen
    if(strncmp(rtd_keylog_addr, RTD_KEYLOG_MAGIC, strlen(RTD_KEYLOG_MAGIC)) == 0)         // reset, backup history keylog pos.
    {
        rtd_keylog_pos = strlen(rtd_keylog_addr);
        rtd_keylog_last_pos = rtd_keylog_pos;
    }

    // add this boot keylog magic string
    snprintf((rtd_keylog_addr+rtd_keylog_pos), RTD_LOGBUF_KEYLOG_SIZE-rtd_keylog_pos-1 , "%s", RTD_KEYLOG_MAGIC);
    rtd_keylog_pos = strlen(rtd_keylog_addr);
    mutex_unlock(mtx);
    return 0;
}

int rtdlog_save_keylog (const char * fmt, ...)
{
    va_list args;
    struct mutex * mtx = &keylog_mutex;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
    struct __kernel_old_timeval log_time;
#else
    struct timeval log_time;
#endif

    if((!is_setup_rtd_logbuf) || (rtd_keylog_addr == NULL) || (gpRtdlogTimestamp == NULL))
    {
        rtd_pr_rtdlog_err("Save keylog fail. is_setup_rtd_logbuf=%d, rtd_keylog_addr or gpRtdlogTimestamp is Null\n",is_setup_rtd_logbuf);
        return -1;
    }

    if(rtd_keylog_pos >= RTD_LOGBUF_KEYLOG_SIZE-1)  // keylog buffer full
    {
        rtd_pr_rtdlog_err("Save keylog fail. keylog buffer is full\n");
        return -2;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
    log_time = ns_to_kernel_old_timeval((s64)(*gpRtdlogTimestamp));
#else
    log_time = ns_to_timeval((s64)(*gpRtdlogTimestamp));
#endif
    mutex_lock(mtx);
    // save time stamp
    snprintf((rtd_keylog_addr+rtd_keylog_pos), RTD_LOGBUF_KEYLOG_SIZE-rtd_keylog_pos-1 , "[%08d.%06d]", (int)log_time.tv_sec, (int)log_time.tv_usec);

    // save log
    va_start(args, fmt);
    rtd_keylog_pos = strlen(rtd_keylog_addr);
    if(rtd_keylog_pos < RTD_LOGBUF_KEYLOG_SIZE-1)
    {
        vsnprintf((rtd_keylog_addr+rtd_keylog_pos), RTD_LOGBUF_KEYLOG_SIZE-rtd_keylog_pos-1, fmt, args);
    }
    va_end(args);

    // append \n after each log
    rtd_keylog_pos = strlen(rtd_keylog_addr);
    if((rtd_keylog_pos<RTD_LOGBUF_KEYLOG_SIZE) && (*(rtd_keylog_addr+rtd_keylog_pos-1)!='\n'))
    {
        *(rtd_keylog_addr+rtd_keylog_pos) = '\n';
        rtd_keylog_pos = rtd_keylog_pos+1;
    }
    mutex_unlock(mtx);
    return 0;
}
EXPORT_SYMBOL(rtdlog_save_keylog);

/********************************************************
* rtdlog init APIs
*********************************************************/
#define CPU_TYPE_NUM 8
static void init_cpu_type_index(int index)
{
    int i;
    char *cpu_type_str[CPU_TYPE_NUM] =
    {
        "acpu1",
        "kcpu2",
        "vcpu1",
        "vcpu2",
        "vcpu3",
        "kcpu",
        "kernel",
        "hal"
    };
    char *cpu_type_num[CPU_TYPE_NUM] =
    {
        &acpu1_cpu_type,
        &kcpu2_cpu_type,
        &vcpu1_cpu_type,
        &vcpu2_cpu_type,
        &vcpu3_cpu_type,
        &kcpu_cpu_type,
        &kernel_cpu_type,
        &hal_cpu_type
    };

    for(i =0;i<CPU_TYPE_NUM;i++)
    {
        if(strcmp(pRtdLogCB->buf_name,cpu_type_str[i]) == 0)
        {
            *cpu_type_num[i] = index;
        }
    }
}
static void init_rtd_buffer (logbuf_global_status * status)
{
    struct SHARE_STRUCT* p_share = (struct SHARE_STRUCT*) share_buffer;
    unsigned long * native_logbuf_saver;
    int i;

    //init rtd log buf
    gpRtdlogTimestamp = &(status->nsec);
    file_info = &(p_share->file_info);
    p_share->total_file_num = status->total_logbuf_num;
    total_logbuf_num = status->total_logbuf_num;

    //init rbcb info and sema_addr
    for(i = 0; i < status->total_logbuf_num; ++i)
    {
        file_info[i].pRtdLogCB = (RTD_LOGBUF_PHY_START_ADDR+(status->rbcb_offset)*(i+1));
        pRtdLogCB = rtdlog_phys_to_virt(file_info[i].pRtdLogCB);
        array_fifo = &(pRtdLogCB->fifo);
        //dump kfifo
        if(strcmp(pRtdLogCB->buf_name,"kernel") == 0)
        {
            pRtdLogCB->drop_cnt=0;
        }
        init_cpu_type_index(i);
        pRtdLogCB->in_reset=0;

        snprintf(file_info[i].g_file_name,FILE_NAME_LEN-1,"%s",pRtdLogCB->buf_name);
#ifdef RTDLOG_DEBUG
        rtd_pr_rtdlog_err("KFIFO[%d]_INIT:buf(%08lx),size(%08lx),in(%08lx),out(%08lx) (%16lx) (%16lx) %s \n",
               i,(unsigned long)(array_fifo->buffer),(unsigned long)(array_fifo->size), (unsigned long)(array_fifo->in),
               (unsigned long)(array_fifo->out),(unsigned long)(pRtdLogCB),(unsigned long)(array_fifo),file_info[i].g_file_name);
#endif
    }

    //write native log buf addr in param area
    native_logbuf_saver = (unsigned long *)rtdlog_phys_to_virt(RTD_LOGBUF_PHY_START_ADDR+RTD_LOGBUF_DEFAULT_PARAM-8);
#if defined(CONFIG_REALTEK_LOGBUF) && !(LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
    *native_logbuf_saver = (unsigned long)(native_logbuf_addr - PAGE_OFFSET);
#ifdef RTDLOG_DEBUG
    rtd_pr_rtdlog_err("native logbuf addr : 0x%08lx, CONFIG_PAGE_OFFSET : 0x%08lx\n",(unsigned long)native_logbuf_addr,PAGE_OFFSET);
#endif
#endif
}

static int init_rtd_share_buffer (void)
{
    struct SHARE_STRUCT* p_share;
    share_buffer=kmalloc(PAGE_SIZE, GFP_KERNEL);
    if(NULL == share_buffer)
    {
        rtd_pr_rtdlog_err("%s %d. cannot kmalloc share memory for rtkd !\n",__func__,__LINE__);
        return -1;
    }
    memset(share_buffer,0,PAGE_SIZE);

    p_share = (struct SHARE_STRUCT*) share_buffer;
    p_share->version= RTKD_VERSION;
    strcpy(p_share->g_dir_path,logbuf_dir); //FIXME
    p_share->flag_compress = flag_compress;
    return 0;
}

int check_version_and_magic(logbuf_global_status * pStatus)
{
        if( (pStatus->version != RTD_LOGBUF_VERSION)||
            (pStatus->magic_seg1 != RTD_LOGBUF_MAGIC_1)||
            (pStatus->magic_seg2 != RTD_LOGBUF_MAGIC_2) )
        {
                rtd_pr_rtdlog_err("rtd logbuf version mismatch. global version:%d, kernel version:%d",pStatus->version,RTD_LOGBUF_VERSION);
                rtd_pr_rtdlog_err("rtd logbuf param magic mismatch. global magic:%08x-%08x, kernel magic:%08x-%08x",
                        pStatus->magic_seg1,pStatus->magic_seg2,RTD_LOGBUF_MAGIC_1,RTD_LOGBUF_MAGIC_2);
                return -1;
        }
#ifdef RTDLOG_DEBUG
        rtd_pr_rtdlog_err("rtd logbuf version match. global version:%d, kernel version:%d",pStatus->version,RTD_LOGBUF_VERSION);
        rtd_pr_rtdlog_err("rtd logbuf param magic match. global magic:%08x-%08x, kernel magic:%08x-%08x",
                pStatus->magic_seg1,pStatus->magic_seg2,RTD_LOGBUF_MAGIC_1,RTD_LOGBUF_MAGIC_2);
#endif
        return 0;
}
static int rtd_check_magic_number(logbuf_cmdline * boot_param)
{
    if((boot_param->magic_seg1 != RTD_LOGBUF_MAGIC_1)||
            (boot_param->magic_seg2 != RTD_LOGBUF_MAGIC_2))
    {
        rtd_pr_rtdlog_err("rtd logbuf param magic mismatch. global magic:%08x-%08x, kernel magic:%08x-%08x",
                   boot_param->magic_seg1,boot_param->magic_seg2,RTD_LOGBUF_MAGIC_1,RTD_LOGBUF_MAGIC_2);
        return -1;
    }

    return 0;
}
/* logswi */
static int rtd_get_param_logswi(logbuf_cmdline * boot_param)
{
    bootcode_logswi = boot_param->logswi;
    rtd_pr_rtdlog_info("parse bootcode args finish!\n");
    return 0;
}
/* logflag */
static int rtd_get_param_logflag(logbuf_cmdline * boot_param)
{
    flag_compress = boot_param->flag_compress;
    flag_status= boot_param->flag_status;
    read_thread_sleep_span= boot_param->sleep_time;
    save_prelog_flag = boot_param->save_prelog_flag;
    if(flag_status == 0)
        rtdlog_filter_logblock();
    rtd_pr_rtdlog_info("parse bootcode logflag args finish!\n");
    return 0;
}
static int rtd_get_param_logdir(logbuf_cmdline * boot_param)
{
    memset(logbuf_dir,0,LOGBUF_DIR_MAX_SIZE);
    snprintf(logbuf_dir, LOGBUF_DIR_MAX_SIZE-1, "%s",boot_param->logdir);
    return 0;
}
static int rtd_get_param_filesize(logbuf_cmdline * boot_param)
{
    pRtdLogfilesize = &(boot_param->filesize);

    if(pRtdLogfilesize->total_file == 0)
        pRtdLogfilesize->total_file = LOG_FILE_KERNEL_MAX_SIZE;
    if(pRtdLogfilesize->acpu1_file== 0)
        pRtdLogfilesize->acpu1_file = LOG_FILE_FW_MAX_SIZE;
    if(pRtdLogfilesize->acpu2_file== 0)
        pRtdLogfilesize->acpu2_file = LOG_FILE_FW_MAX_SIZE;
    if(pRtdLogfilesize->vcpu1_file== 0)
        pRtdLogfilesize->vcpu1_file = LOG_FILE_FW_MAX_SIZE;
    if(pRtdLogfilesize->vcpu2_file== 0)
        pRtdLogfilesize->vcpu2_file= LOG_FILE_FW_MAX_SIZE; 
#ifdef RTDLOG_DEBUG
    rtd_pr_rtdlog_err("total_file=%x %x %x %x %x",pRtdLogfilesize->total_file,pRtdLogfilesize->acpu1_file,
        pRtdLogfilesize->acpu2_file,pRtdLogfilesize->vcpu1_file,pRtdLogfilesize->vcpu2_file);
#endif
    return 0;
}
static int rtd_get_boot_param(logbuf_cmdline * boot_param)
{
    if(rtd_check_magic_number(boot_param) < 0)
    {
        return -1;
    }
    /*show*/
    //rtd_show_boot_param(boot_param);
    /*get param*/
    rtd_get_param_logswi(boot_param);
    //rtd_get_param_logfile(boot_param);
    rtd_get_param_logdir(boot_param);
    rtd_get_param_filesize(boot_param);
    rtd_get_param_logflag(boot_param);
    //rtd_get_param_logfilter(boot_param);

    return 0;
}

void rtdlog_show_history_keylog(char * buf, int len)
{
        int ofs = 0;
        int tmp_ofs = 0;

        if(len > RTD_LOGBUF_KEYLOG_SIZE){
                len = RTD_LOGBUF_KEYLOG_SIZE;
        }

        rtd_pr_rtdlog_err( "------dump history keylog start------\n");
        while(ofs < len){
                if(*(buf+ofs) == '\n'){
                        *(buf+ofs) = '\0';
                        rtd_pr_rtdlog_err( "%s\n", buf+tmp_ofs);
                        *(buf+ofs) = '\n';
                        tmp_ofs = ofs+1;
                }
                ofs++;
        }
        rtd_pr_rtdlog_err( "------dump history keylog end------\n");
}

int reset_rtd_keylog_buf(void)
{
        char * keylog_tmp_buf = NULL;  // store this boot keylog after init
        int ret = 0;
        int new_keylog_len; // this boot keylog len
        struct mutex * mtx = &keylog_mutex;

        mutex_lock(mtx);
        new_keylog_len = strlen(rtd_keylog_addr)-rtd_keylog_last_pos;
        if((new_keylog_len -strlen(RTD_KEYLOG_MAGIC))<= 0){        // this boot keylog len except magic string
                ret = 1;
                goto reset;
        }

        keylog_tmp_buf = kmalloc(new_keylog_len+1, GFP_KERNEL);
        if(keylog_tmp_buf == NULL){
                rtd_pr_rtdlog_err("kmalloc fail!![%s]\n", __FUNCTION__);
                ret = -1;
                goto reset;
        }
        memset(keylog_tmp_buf, 0, new_keylog_len+1);
        // backup this boot keylog
        memcpy(keylog_tmp_buf, rtd_keylog_addr+rtd_keylog_last_pos, new_keylog_len);    // backup this boot keylog to malloc buffer
        memset(rtd_keylog_addr+rtd_keylog_last_pos,0,new_keylog_len); // clear this boot keylog. for dump history keylog
        // show history keylog
        rtdlog_show_history_keylog(rtd_keylog_addr, strlen(rtd_keylog_addr));
        // save this boot keylog
        memset(rtd_keylog_addr, 0, RTD_LOGBUF_KEYLOG_SIZE);     // clear buffer
        snprintf(rtd_keylog_addr, RTD_LOGBUF_KEYLOG_SIZE-1 , "%s", keylog_tmp_buf);     // save keylog to buffer

        goto RET;

reset:
        // reset keylog buffer
        memset(rtd_keylog_addr+rtd_keylog_last_pos, 0, 1); // clear magic string. for dump history keylog
        rtdlog_show_history_keylog(rtd_keylog_addr, strlen(rtd_keylog_addr));    //show history keylog
        memset(rtd_keylog_addr, 0, RTD_LOGBUF_KEYLOG_SIZE);
        snprintf(rtd_keylog_addr, RTD_LOGBUF_KEYLOG_SIZE-1 , "%s", RTD_KEYLOG_MAGIC);

RET:
        rtd_keylog_last_pos = 0;
        rtd_keylog_pos = strlen(rtd_keylog_addr);
        mutex_unlock(mtx);

        // free tmp buf
        if(keylog_tmp_buf){
                kfree(keylog_tmp_buf);
                keylog_tmp_buf = NULL;
        }
        return ret;

}
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS)
void rtd_logbuf_save_early_log ( int level, const char *text, u16 text_len)
{
    rtd_msg_header tmp_header;

    tmp_header.magic_header = RTD_LOGBUF_MAGIC_HEADER;
    tmp_header.cpu_type = (u8)kernel_cpu_type;
    tmp_header.cpu_core = smp_processor_id();
    tmp_header.module = 0;
    tmp_header.pid = task_tgid_nr(current);
    tmp_header.tid = task_pid_nr(current);
    tmp_header.timestamp = *gpRtdlogTimestamp;
    tmp_header.level = level;
    tmp_header.text_len = text_len;
    tmp_header.len = sizeof(tmp_header) + text_len;

    snprintf(tmp_header.name,sizeof(tmp_header.name),current->comm);

    rtdlog_save_msg(kernel_cpu_type,text, &tmp_header);

}
void save_early_log(unsigned char type)
{
    size_t len;
    unsigned char buf[1024];
    static int num = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
    kmsg_dump_rewind(&dumper);
#endif
    for(;;)
    {
         num++;
        //local_irq_disable();
        memset(buf,0,sizeof(buf));
        if(!kmsg_dump_get_line(&dumper, false, buf, sizeof(buf)-2, &len))
        {
            rtd_pr_rtdlog_err(" ***************** save_early_log *****%d*****%d*********** \n",num,strlen(buf));
            //local_irq_enable();
            return;
        }

        if(!strlen(buf)){
            continue;
        }

        //local_irq_enable();
        rtd_logbuf_save_early_log(0,buf,strlen(buf));
    }
}
#else
extern void save_early_log(unsigned char type);   //move scpu kernel log from linux logbuf to rtd logbuf
#endif
#include <linux/pageremap.h>
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS)
#include<trace/hooks/logbuf.h>
#include <../kernel/printk/printk_ringbuffer.h>
#include <linux/dev_printk.h>
struct rtd_printk_record {
        struct printk_info      *info;
        char                    *text_buf;
        unsigned int            text_buf_size;
};
extern struct timezone sys_tz;
unsigned int str_status = 0;
EXPORT_SYMBOL(str_status);
void rtk_get_local_time (unsigned int *hour,unsigned int *minute,unsigned int *second,unsigned int* millisecond)
{
    unsigned int local_time;
    unsigned int sec;
    unsigned int msec;
    struct timespec64 t64;

    ktime_get_real_ts64(&t64);

    sec = t64.tv_sec;
    msec = (t64.tv_nsec/1000000);
    local_time = (sec - (sys_tz.tz_minuteswest * 60))%(3600*24);
    *hour=local_time/3600;
    *minute=(local_time%3600)/60;
    *second=local_time%60;
    *millisecond=msec;

    return;
}
static void rtk_logbuf(void *arg,struct printk_ringbuffer *rb, struct printk_record *r)
{
    u32 extra_len;
    char extra[1024] = {0};
    static unsigned int __maybe_unused hour=0,minute=0,second=0,millisecond=0;
    struct rtd_printk_record * log = (struct rtd_printk_record *)r;
    
    if(str_status == 0)
    {
        rtk_get_local_time(&hour,&minute,&second,&millisecond);
    }
    extra_len = sprintf(extra, "%02d:%02d:%02d.%03d (%d)-%04d\t", hour,minute,second,millisecond,
                    smp_processor_id(), task_pid_nr(current)); // RTK_patch: print cpu id and task pid

    memcpy(&extra[extra_len], log->text_buf, log->info->text_len);
    rtd_logbuf_save_log(log->info->level ,extra, extra_len+log->info->text_len);
}
#endif

void rtdlog_setup (void)
{
#if (IS_ENABLED(CONFIG_RTK_KDRV_TEE) && !defined(CONFIG_CUSTOMER_TV006))
    struct optee_rpc_param rtc_set_info = {0};
#endif

    logbuf_global_status * global_status ;
    logbuf_cmdline * boot_param ;
    rtd_virt_addr = (unsigned long)dvr_remap_uncached_memory(RTD_LOGBUF_PHY_START_ADDR, rtdlog_get_buffer_size(), __builtin_return_address(0));
    if(0 == rtd_virt_addr)
    {
        return;
    }

    global_status = (logbuf_global_status *)(rtdlog_phys_to_virt(RTD_LOGBUF_PHY_START_ADDR));
    boot_param = (logbuf_cmdline *)(phys_to_virt(RTD_LOGBUF_PHY_START_ADDR + RTD_LOGBUF_CMDLINE_OFFSET));


    if(check_version_and_magic(global_status))
    {
        return;
    }

    /* init logswi, logifle, logdir,  logfilter*/
    if(rtd_get_boot_param(boot_param))
    {
        return;
    }

    if(init_rtd_share_buffer())
    {
        return;
    }
#if 0
    if(check_bufsize(global_status))
    {
        return;
    }
#endif

#if (IS_ENABLED(CONFIG_RTK_KDRV_TEE) && !defined(CONFIG_CUSTOMER_TV006))
    optee_logbuf_setup_info_send(&rtc_set_info);
#endif

    init_rtd_buffer(global_status);

    //setup filter
    //logfilter_setup();
    catfilter_setup();

    //now rtd logbuf can be used
    is_setup_rtd_logbuf = true;
#if defined(CONFIG_REALTEK_LOGBUF) && !(LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
    save_early_log(kernel_cpu_type);       //parse&save early log in linux logbuf before now time
#endif
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS)
    save_early_log(kernel_cpu_type);
    register_trace_android_vh_logbuf(rtk_logbuf,NULL);
#endif
    is_save_early_log_done = true;

    // init keylog buffer
    init_rtd_keylog_buf();
    reset_rtd_keylog_buf();
    return;
}

static int rtdlog_pm_suspend ( struct platform_device *dev, pm_message_t state )
{
    return 0;
}
static int rtdlog_pm_resume (struct platform_device *dev)
{
    return 0;
}

static int rtdlog_panic (struct notifier_block *this, unsigned long ev, void *ptr);
static struct notifier_block rtdlog_panic_block =
{
    .notifier_call = rtdlog_panic,
};


struct file_operations rtdlog_fops =
{
    .owner          = THIS_MODULE,
    .compat_ioctl = rtdlog_ioctl,
    .open           = rtdlog_open,
    .release        = rtdlog_release,
    .read          = rtdlog_read,
    .write          = rtdlog_write,
    .mmap          = rtdlog_mmap,
};

int rtdlog_major = RTD_LOGBUF_DEV_MAJOR;    //rtdlog device node major
int rtdlog_minor = RTD_LOGBUF_DEV_MINOR;    //rtdlog device node minor
static struct class *rtdlog_class;
static struct rtdlog_dev *rtdlog_drv_dev;               /* allocated in rtdlog_init_module */

static struct platform_device *rtdlog_platform_devs;
static struct platform_driver rtdlog_device_driver =
{
    .suspend    = rtdlog_pm_suspend,
    .resume     = rtdlog_pm_resume,
    .driver = {
        .name       = "rtd_logger",
        .bus        = &platform_bus_type,
    } ,
} ;

void rtdlog_watchdog_dump(struct work_struct *work)
{
    struct rtd_logger_work_struct * watchdog_work = (struct rtd_logger_work_struct *)work;
#ifdef CONFIG_REALTEK_LOGBUF
    console_flush_on_panic(CONSOLE_FLUSH_PENDING);        //dump native logbuf to console
#endif
    rtdlog_crash_dump(watchdog_work->param);
}
#ifdef SUPPORT_LOGBUF_FILE_OPERAIONS //Support logbuf file operation
#include <linux/miscdevice.h>
static ssize_t kfifo_out_peek_copy ( char  __user * buf,int type, struct kfifo *fifo, rtd_msg_header* header,unsigned char *buffer)
{
    unsigned int out_len;
    int pos;
    ssize_t ret=0;

    if(buffer == NULL)
        return 0;

    if((header->text_len) > 1536)
    {
        rtd_pr_rtdlog_err("%s %d.  drop type %s's log\n",__func__,__LINE__,file_info[type].g_file_name);
        __kfifo_add_out(fifo, header->text_len);
        return 0;
    }

    pos=rtd_build_msg_header(header,buffer);

    out_len=kfifo_out_peek(fifo,buffer+pos,header->text_len,0);
    __kfifo_add_out(fifo, out_len);

    if(buffer[pos+out_len-1]!='\n')
    {
        buffer[pos+out_len]='\n';
        out_len++;
    }
    buffer[pos+out_len]=0;
    ret = pos+out_len;

    if((bootcode_logswi>>8) & (1<<type))
    {
        if(copy_to_user(buf, (void *)buffer, ret ))
        {
            rtd_pr_rtdlog_err("%s %d.  copy_to_user  %d err\n",__func__,__LINE__,ret);
            return 0;
        }
    }
    if(strcmp((char *)pRtdLogCB->buf_name,"kernel") != 0)
    {
        if(bootcode_logswi & (1<<type))
        {
            rtd_pr_rtdlog_err("[%s]%s",file_info[type].g_file_name, buffer);
        }
    }

    return ret;
}
ssize_t rtd_copy_msg_data ( char __user * buf,int type, rtd_msg_header* header, struct kfifo * fifo, unsigned char *buffer)
{
    __kfifo_add_out(fifo, sizeof(rtd_msg_header));
    return kfifo_out_peek_copy(buf,type, fifo,header,buffer);
}
ssize_t rtkd_copy_log_to_user (int type, char __user * buf, size_t count,unsigned char *buffer)
{
    logbuf_rbcb * rtdlogcb = NULL;
    struct kfifo * rtd_array_fifo= NULL;
    rtd_msg_header header;
    int header_check;
    ssize_t ret = 0;

    rtdlogcb = rtdlog_phys_to_virt(file_info[type].pRtdLogCB);
    rtd_array_fifo= &(rtdlogcb->fifo);
    header_check = get_msg_header(type,&header, rtd_array_fifo);

    if(header_check==0)
    {
       return ret;
    }
    else if (header_check==-1)
    {
        rtd_pr_rtdlog_err("%s %d. error happen! insert miss log msg for type:%d\n",__func__,__LINE__,type);
        return ret;
    }
    if((header.len+100) > count)
        return ret;
    ret=rtd_copy_msg_data(buf,type, &header, rtd_array_fifo,buffer);
    return ret;
}

ssize_t rtkd_copy_to_user (int type, char __user * buf, size_t count, unsigned char *buffer)
{
    int data_len = 0;
    ssize_t len = 0;

    if(!is_setup_rtd_logbuf)
        return 0;

    do
    {
        len=rtkd_copy_log_to_user(type, buf+data_len, count-data_len, buffer);
        if(len == 0)
            return data_len;
        data_len += len;
    }while(count > data_len);

    return data_len;
}
ssize_t rtk_kernellog_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    int i = 0;
    ssize_t len = 0;

    //rtd_pr_rtdlog_err("%s(%d) %d\n",__func__,__LINE__,kernel_cpu_type);
    if(kernel_cpu_type == 0xff)
        return len;

    if(kernel_cpu_log_buffer == NULL)
        kernel_cpu_log_buffer = kmalloc(2048,GFP_KERNEL);

    if(kernel_cpu_log_buffer == NULL)
        return len;

    i =kernel_cpu_type%total_logbuf_num;
    mutex_lock(&save_kernel_mutex);
    len=rtkd_copy_to_user(i, buf, count,kernel_cpu_log_buffer);
    mutex_unlock(&save_kernel_mutex);

    return len;
}
int rtk_kernellog_release(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Release\n",__func__,__LINE__);
    return 0;
}
int rtk_kernellog_open(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Open\n",__func__,__LINE__);
    return 0;
}

long rtk_kernellog_ioctl(struct file *file, unsigned int cmd,
                 unsigned long arg)
{
    rtd_pr_rtdlog_info("%s(%d) \n",__func__,__LINE__);
    return 0;
}

struct file_operations rtk_kernellog_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = rtk_kernellog_ioctl,
    .open = rtk_kernellog_open,
    .read = rtk_kernellog_read,
    .release = rtk_kernellog_release,
};

static struct miscdevice rtk_kernellog_miscdev = {
    MISC_DYNAMIC_MINOR,
    "rtd_log_kernel",
    &rtk_kernellog_fops
};

ssize_t rtk_acpu1log_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    static int i = 0;
    ssize_t len = 0;

    //rtd_pr_rtdlog_err("%s(%d) %d\n",__func__,__LINE__,acpu1_cpu_type);
    if(acpu1_cpu_type == 0xff)
        return len;

    if(acpu1_cpu_log_buffer == NULL)
        acpu1_cpu_log_buffer = kmalloc(2048,GFP_KERNEL);

    if(acpu1_cpu_log_buffer == NULL)
        return len;

    i =acpu1_cpu_type%total_logbuf_num;
    mutex_lock(&save_acpu1_mutex);
    len=rtkd_copy_to_user(i, buf, count,acpu1_cpu_log_buffer);
    mutex_unlock(&save_acpu1_mutex);

    return len;
}
int rtk_acpu1log_release(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Release\n",__func__,__LINE__);
    return 0;
}
int rtk_acpu1log_open(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Open\n",__func__,__LINE__);
    return 0;
}

long rtk_acpu1log_ioctl(struct file *file, unsigned int cmd,
                 unsigned long arg)
{
    rtd_pr_rtdlog_info("%s(%d) \n",__func__,__LINE__);
    return 0;
}

struct file_operations rtk_acpu1log_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = rtk_acpu1log_ioctl,
    .open = rtk_acpu1log_open,
    .read = rtk_acpu1log_read,
    .release = rtk_acpu1log_release,
};

static struct miscdevice rtk_acpu1log_miscdev = {
    MISC_DYNAMIC_MINOR,
    "rtd_log_acpu1",
    &rtk_acpu1log_fops
};

ssize_t rtk_kcpu2log_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    static int i = 0;
    ssize_t len = 0;

    //rtd_pr_rtdlog_err("%s(%d) %d\n",__func__,__LINE__,kcpu2_cpu_type);
    if(kcpu2_cpu_type == 0xff)
        return len;

    if(kcpu2_cpu_log_buffer == NULL)
        kcpu2_cpu_log_buffer = kmalloc(2048,GFP_KERNEL);

    if(kcpu2_cpu_log_buffer == NULL)
        return len;

    i =kcpu2_cpu_type%total_logbuf_num;
    mutex_lock(&save_kcpu2_mutex);
    len=rtkd_copy_to_user(i, buf, count,kcpu2_cpu_log_buffer);
    mutex_unlock(&save_kcpu2_mutex);

    return len;
}
int rtk_kcpu2log_release(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Release\n",__func__,__LINE__);
    return 0;
}
int rtk_kcpu2log_open(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Open\n",__func__,__LINE__);
    return 0;
}

long rtk_kcpu2log_ioctl(struct file *file, unsigned int cmd,
                 unsigned long arg)
{
    rtd_pr_rtdlog_info("%s(%d) \n",__func__,__LINE__);
    return 0;
}

struct file_operations rtk_kcpu2log_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = rtk_kcpu2log_ioctl,
    .open = rtk_kcpu2log_open,
    .read = rtk_kcpu2log_read,
    .release = rtk_kcpu2log_release,
};

static struct miscdevice rtk_kcpu2log_miscdev = {
    MISC_DYNAMIC_MINOR,
    "rtd_log_atf",
    &rtk_kcpu2log_fops
};

ssize_t rtk_vcpu1log_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    static int i = 0;
    ssize_t len = 0;

    //rtd_pr_rtdlog_err("%s(%d) %d\n",__func__,__LINE__,vcpu1_cpu_type);
    if(vcpu1_cpu_type == 0xff)
        return len;

    if(vcpu1_cpu_log_buffer == NULL)
        vcpu1_cpu_log_buffer = kmalloc(2048,GFP_KERNEL);

    if(vcpu1_cpu_log_buffer == NULL)
        return len;

    i =vcpu1_cpu_type%total_logbuf_num;
    mutex_lock(&save_vcpu1_mutex);
    len=rtkd_copy_to_user(i, buf, count,vcpu1_cpu_log_buffer);
    mutex_unlock(&save_vcpu1_mutex);

    return len;
}
int rtk_vcpu1log_release(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Release\n",__func__,__LINE__);
    return 0;
}
int rtk_vcpu1log_open(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Open\n",__func__,__LINE__);
    return 0;
}

long rtk_vcpu1log_ioctl(struct file *file, unsigned int cmd,
                 unsigned long arg)
{
    rtd_pr_rtdlog_info("%s(%d) \n",__func__,__LINE__);
    return 0;
}

struct file_operations rtk_vcpu1log_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = rtk_vcpu1log_ioctl,
    .open = rtk_vcpu1log_open,
    .read = rtk_vcpu1log_read,
    .release = rtk_vcpu1log_release,
};

static struct miscdevice rtk_vcpu1log_miscdev = {
    MISC_DYNAMIC_MINOR,
    "rtd_log_vcpu1",
    &rtk_vcpu1log_fops
};

ssize_t rtk_vcpu2log_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    static int i = 0;
    ssize_t len = 0;

    //rtd_pr_rtdlog_err("%s(%d) %d\n",__func__,__LINE__,vcpu2_cpu_type);
    if(vcpu2_cpu_type == 0xff)
        return len;

    if(vcpu2_cpu_log_buffer == NULL)
        vcpu2_cpu_log_buffer = kmalloc(2048,GFP_KERNEL);

    if(vcpu2_cpu_log_buffer == NULL)
        return len;

    i =vcpu2_cpu_type%total_logbuf_num;
    mutex_lock(&save_vcpu2_mutex);
    len=rtkd_copy_to_user(i, buf, count,vcpu2_cpu_log_buffer);
    mutex_unlock(&save_vcpu2_mutex);
    return len;
}
int rtk_vcpu2log_release(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Release\n",__func__,__LINE__);
    return 0;
}
int rtk_vcpu2log_open(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Open\n",__func__,__LINE__);
    return 0;
}

long rtk_vcpu2log_ioctl(struct file *file, unsigned int cmd,
                 unsigned long arg)
{
    rtd_pr_rtdlog_info("%s(%d) \n",__func__,__LINE__);
    return 0;
}

struct file_operations rtk_vcpu2log_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = rtk_vcpu2log_ioctl,
    .open = rtk_vcpu2log_open,
    .read = rtk_vcpu2log_read,
    .release = rtk_vcpu2log_release,
};

static struct miscdevice rtk_vcpu2log_miscdev = {
    MISC_DYNAMIC_MINOR,
    "rtd_log_vcpu2",
    &rtk_vcpu2log_fops
};

ssize_t rtk_vcpu3log_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    static int i = 0;
    ssize_t len = 0;

    //rtd_pr_rtdlog_err("%s(%d) %d\n",__func__,__LINE__,vcpu3_cpu_type);
    if(vcpu3_cpu_type == 0xff)
        return len;

    if(vcpu3_cpu_log_buffer == NULL)
        vcpu3_cpu_log_buffer = kmalloc(2048,GFP_KERNEL);

    if(vcpu3_cpu_log_buffer == NULL)
        return len;

    i =vcpu3_cpu_type%total_logbuf_num;
    mutex_lock(&save_vcpu3_mutex);
    len=rtkd_copy_to_user(i, buf, count,vcpu3_cpu_log_buffer);
    mutex_unlock(&save_vcpu3_mutex);

    return len;
}
int rtk_vcpu3log_release(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Release\n",__func__,__LINE__);
    return 0;
}
int rtk_vcpu3log_open(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Open\n",__func__,__LINE__);
    return 0;
}

long rtk_vcpu3log_ioctl(struct file *file, unsigned int cmd,
                 unsigned long arg)
{
    rtd_pr_rtdlog_info("%s(%d) \n",__func__,__LINE__);
    return 0;
}

struct file_operations rtk_vcpu3log_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = rtk_vcpu3log_ioctl,
    .open = rtk_vcpu3log_open,
    .read = rtk_vcpu3log_read,
    .release = rtk_vcpu3log_release,
};

static struct miscdevice rtk_vcpu3log_miscdev = {
    MISC_DYNAMIC_MINOR,
    "rtd_log_vcpu3",
    &rtk_vcpu3log_fops
};

ssize_t rtk_kcpulog_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    static int i = 0;
    ssize_t len = 0;

    //rtd_pr_rtdlog_err("%s(%d) %d\n",__func__,__LINE__,kcpu_cpu_type);
    if(kcpu_cpu_type == 0xff)
        return len;

    if(kcpu_cpu_log_buffer == NULL)
        kcpu_cpu_log_buffer = kmalloc(2048,GFP_KERNEL);

    if(kcpu_cpu_log_buffer == NULL)
        return len;

    i =kcpu_cpu_type%total_logbuf_num;
    mutex_lock(&save_kcpu_mutex);
    len=rtkd_copy_to_user(i, buf, count,kcpu_cpu_log_buffer);
    mutex_unlock(&save_kcpu_mutex);

    return len;
}
int rtk_kcpulog_release(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Release\n",__func__,__LINE__);
    return 0;
}
int rtk_kcpulog_open(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Open\n",__func__,__LINE__);
    return 0;
}

long rtk_kcpulog_ioctl(struct file *file, unsigned int cmd,
                 unsigned long arg)
{
    rtd_pr_rtdlog_info("%s(%d) \n",__func__,__LINE__);
    return 0;
}

struct file_operations rtk_kcpulog_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = rtk_kcpulog_ioctl,
    .open = rtk_kcpulog_open,
    .read = rtk_kcpulog_read,
    .release = rtk_kcpulog_release,
};

static struct miscdevice rtk_kcpulog_miscdev = {
    MISC_DYNAMIC_MINOR,
    "rtd_log_optee",
    &rtk_kcpulog_fops
};

ssize_t rtk_hallog_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    static int i = 0;
    ssize_t len = 0;

    //rtd_pr_rtdlog_err("%s(%d) %d\n",__func__,__LINE__,hal_cpu_type);
    if(hal_cpu_type == 0xff)
        return len;

    if(hal_cpu_log_buffer == NULL)
        hal_cpu_log_buffer = kmalloc(2048,GFP_KERNEL);

    if(hal_cpu_log_buffer == NULL)
        return len;

    i =hal_cpu_type%total_logbuf_num;
    mutex_lock(&save_hal_read_mutex);
    len=rtkd_copy_to_user(i, buf, count, hal_cpu_log_buffer);
    mutex_unlock(&save_hal_read_mutex);

    return len;
}
int rtk_hallog_release(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Release\n",__func__,__LINE__);
    return 0;
}
int rtk_hallog_open(struct inode *inode, struct file *file)
{
    rtd_pr_rtdlog_info("%s(%d) Open\n",__func__,__LINE__);
    return 0;
}

long rtk_hallog_ioctl(struct file *file, unsigned int cmd,
                 unsigned long arg)
{
    rtd_pr_rtdlog_info("%s(%d) \n",__func__,__LINE__);
    return 0;
}

struct file_operations rtk_hallog_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = rtk_hallog_ioctl,
    .open = rtk_hallog_open,
    .read = rtk_hallog_read,
    .release = rtk_hallog_release,
};

static struct miscdevice rtk_hallog_miscdev = {
    MISC_DYNAMIC_MINOR,
    "rtd_log_rhal",
    &rtk_hallog_fops
};
static void init_logbuf_file_dev(void)
{
    if (misc_register(&rtk_kernellog_miscdev)) {
        rtd_pr_rtdlog_err("rtk_kernellog_module_init failed - register misc device failed\n");
    }
    if (misc_register(&rtk_acpu1log_miscdev)) {
        rtd_pr_rtdlog_err("rtk_acpu1llog_module_init failed - register misc device failed\n");
    }
    if (misc_register(&rtk_kcpu2log_miscdev)) {
        rtd_pr_rtdlog_err("rtk_kcpu2llog_module_init failed - register misc device failed\n");
    }
    if (misc_register(&rtk_vcpu1log_miscdev)) {
        rtd_pr_rtdlog_err("rtk_vcpu1llog_module_init failed - register misc device failed\n");
    }
    if (misc_register(&rtk_vcpu2log_miscdev)) {
        rtd_pr_rtdlog_err("rtk_vcpu2llog_module_init failed - register misc device failed\n");
    }
    if (misc_register(&rtk_vcpu3log_miscdev)) {
        rtd_pr_rtdlog_err("rtk_vcpu3llog_module_init failed - register misc device failed\n");
    }
    if (misc_register(&rtk_kcpulog_miscdev)) {
        rtd_pr_rtdlog_err("rtk_kcpullog_module_init failed - register misc device failed\n");
    }
    if (misc_register(&rtk_hallog_miscdev)) {
        rtd_pr_rtdlog_err("rtk_halllog_module_init failed - register misc device failed\n");
    }
    return;
}
static void free_logbuf_buffer(unsigned char *buffer)
{
    if(buffer != NULL)
    {
        kfree(buffer);
        buffer = NULL;
    }
    return;
}
static void release_logbuf_file_dev(void)
{
    misc_deregister(&rtk_kernellog_miscdev);
    misc_deregister(&rtk_acpu1log_miscdev);
    misc_deregister(&rtk_kcpu2log_miscdev);
    misc_deregister(&rtk_vcpu1log_miscdev);
    misc_deregister(&rtk_vcpu2log_miscdev);
    misc_deregister(&rtk_vcpu3log_miscdev);
    misc_deregister(&rtk_kcpulog_miscdev);
    misc_deregister(&rtk_hallog_miscdev);
    free_logbuf_buffer(kernel_cpu_log_buffer);
    free_logbuf_buffer(acpu1_cpu_log_buffer);
    free_logbuf_buffer(kcpu2_cpu_log_buffer);
    free_logbuf_buffer(vcpu1_cpu_log_buffer);
    free_logbuf_buffer(vcpu2_cpu_log_buffer);
    free_logbuf_buffer(vcpu3_cpu_log_buffer);
    free_logbuf_buffer(kcpu_cpu_log_buffer);
    free_logbuf_buffer(hal_cpu_log_buffer);
    return;
}
#endif
static int __init rtdlog_init_module (void)
{
    int result;
    dev_t dev = 0;

    /*
     * Get a range of minor numbers to work with,
     * asking for a dynamic major unless directed otherwise at load time.
     */

    rtd_pr_rtdlog_warn(" ***************** rtdlog init module ********************* \n");
#if defined(CONFIG_REALTEK_LOGBUF_MODULE) || (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
    init_logbuffer_timer();
    rtdlog_parse_bufsize_init();
#endif

    if (rtdlog_major)
    {
        dev = MKDEV(rtdlog_major, rtdlog_minor);
        result = register_chrdev_region(dev, 1, RTDLOG_NODE);
    }
    else
    {
        result = alloc_chrdev_region(&dev, rtdlog_minor, 1, RTDLOG_NODE);
        rtdlog_major = MAJOR(dev);
    }
    if (result < 0)
    {
        rtd_pr_rtdlog_err("rtdlog: can't get major %d\n", rtdlog_major);
        return result;
    }

    rtd_pr_rtdlog_err("rtdlog:get result:%d,get dev:0x%08x,major:%d\n", result, dev, rtdlog_major);

    rtdlog_class = class_create(THIS_MODULE, RTDLOG_NODE);
    if (IS_ERR_OR_NULL(rtdlog_class))
    {
        return PTR_ERR(rtdlog_class);
    }

    device_create(rtdlog_class, NULL, dev, NULL, RTDLOG_NODE);

    /*
     * allocate the devices
     */
    rtdlog_drv_dev = kmalloc(sizeof(struct rtdlog_dev), GFP_KERNEL);
    if (!rtdlog_drv_dev)
    {
        device_destroy(rtdlog_class, dev);
        result = -ENOMEM;
        goto fail;  /* Make this more graceful */
    }
    memset(rtdlog_drv_dev, 0, sizeof(struct rtdlog_dev));

    //initialize device structure
    sema_init(&rtdlog_drv_dev->sem, 1);
    cdev_init(&rtdlog_drv_dev->cdev, &rtdlog_fops);
    rtdlog_drv_dev->cdev.owner = THIS_MODULE;
    rtdlog_drv_dev->cdev.ops = &rtdlog_fops;
    result = cdev_add(&rtdlog_drv_dev->cdev, dev, 1);
    /* Fail gracefully if need be */
    if (result)
    {
        device_destroy(rtdlog_class, dev);
        kfree(rtdlog_drv_dev);
        rtd_pr_rtdlog_err("Error %d adding cdev rtdlog", result);
        goto fail;
    }

#ifdef CONFIG_PM
    rtdlog_platform_devs = platform_device_register_simple(RTDLOG_NODE, -1, NULL, 0);
    if(platform_driver_register(&rtdlog_device_driver) != 0)
    {
        device_destroy(rtdlog_class, dev);
        cdev_del(&rtdlog_drv_dev->cdev);
        kfree(rtdlog_drv_dev);
        rtdlog_platform_devs = NULL;
        goto fail;  /* Make this more graceful */
    }
#endif  //CONFIG_PM
    rtdlog_setup();
#ifdef SUPPORT_LOGBUF_FILE_OPERAIONS
    init_logbuf_file_dev();
 #endif
    atomic_notifier_chain_register(&panic_notifier_list, &rtdlog_panic_block);
#if defined(CONFIG_REALTEK_LOGBUF_MODULE) || (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
#if !(defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS))
    kthread_run(thread_logbuf_reader, NULL, "RtdLogReader");
#endif
#endif
#if 1
    /************init work quene for watchdog dump log**************/
    p_dumpqueue = create_workqueue("rtdlog dump queue");
    //p_dumpqueue = create_singlethread_workqueue("rtdlog dump queue");
    if(!p_dumpqueue)
    {
        rtd_pr_rtdlog_err("Error create dump queue worker fail!!!\n");
        goto fail;
    }

    INIT_WORK(&(DumpWork_cpu[ACPU1].work), rtdlog_watchdog_dump);
    INIT_WORK(&(DumpWork_cpu[ACPU2].work), rtdlog_watchdog_dump);
    INIT_WORK(&(DumpWork_cpu[VCPU1].work), rtdlog_watchdog_dump);
    INIT_WORK(&(DumpWork_cpu[VCPU2].work), rtdlog_watchdog_dump);
    INIT_WORK(&(DumpWork_cpu[KCPU].work), rtdlog_watchdog_dump);
#endif
    return 0; /* succeed */

fail:
    return result;
}

static void __exit rtdlog_cleanup_module (void)
{
    int i = 0;
    dev_t dev = MKDEV(rtdlog_major, rtdlog_minor);

    rtd_pr_rtdlog_warn("rtdlog clean module\n");
    if(share_buffer)
    {
        kfree(share_buffer);
        share_buffer=NULL;
    }

    /* Get rid of our char dev entries */
    if (rtdlog_drv_dev)
    {
        device_destroy(rtdlog_class, dev);
        cdev_del(&rtdlog_drv_dev[i].cdev);
        kfree(rtdlog_drv_dev);
#ifdef CONFIG_PM
        platform_device_unregister(rtdlog_platform_devs);
#endif
    }
#ifdef SUPPORT_LOGBUF_FILE_OPERAIONS
    release_logbuf_file_dev();
#endif
    unregister_chrdev_region(dev, 1);
}

module_init(rtdlog_init_module);
module_exit(rtdlog_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);

int get_msg_header_avk_epc (int type,rtd_msg_header* header,struct kfifo * pKfifo)
{
    struct kfifo * fifo = (pKfifo == NULL) ? (array_fifo): pKfifo;
    if (kfifo_len(fifo)> sizeof(rtd_msg_header))
    {
        kfifo_out_peek(fifo,header,sizeof(rtd_msg_header),0);

        if(check_msghead_valid(header,false)) // if fail
        {
            rtd_pr_rtdlog_err("\n fifo out=0x%x, fifo in=0x%x\n",(unsigned int)fifo->out,  (unsigned int)fifo->in);
            kfifo_drop(fifo);
            pRtdLogCB->in_reset++;
            rtd_pr_rtdlog_err("type[%d] check msg header fail!\n",type);
            return -1;
        }
        else
        {
            return 1;
        }
    }

    return 0;
}

static inline void __kfifo_set_out (struct kfifo *fifo,unsigned int value)
{
    smp_mb();
    fifo->out = value;
}

int rtdlog_find_header (struct kfifo * fifo, int type)
{
    unsigned int offset=0;
    unsigned int find_pos = 0;
    //unsigned char * base = fifo->buffer;
    //unsigned int head_pos = 0;
    //rtd_msg_header tmp;
    //rtd_msg_header * pHeader = &tmp;
    unsigned int fifo_back_len = 12000;
    char * tmp_addr = NULL;
    unsigned int dump_start_pos = 0;

    if(strcmp(pRtdLogCB->buf_name,"kcpu2") == 0)
    {
        fifo_back_len = 8000;
    }
    else if(strcmp(pRtdLogCB->buf_name,"acpu1") == 0)
    {
        fifo_back_len = 30000;
    }
    else if(strcmp(pRtdLogCB->buf_name,"kernel") == 0)
    {
        fifo_back_len = 100000;
    }
    if(fifo->in < fifo_back_len)
    {
        fifo_back_len = fifo->in;
    }
    dump_start_pos = fifo->in - fifo_back_len;
    __kfifo_set_out(fifo,dump_start_pos);

    find_pos = fifo->out;

    offset = find_pos%fifo->size;
    tmp_addr = rtdlog_phys_to_virt((unsigned long)fifo->buffer) + offset;

    while(find_pos != (fifo->in))
    {
        if(check_msghead_valid((rtd_msg_header *)tmp_addr,false))
        {
            find_pos++;
            tmp_addr++;
        }
        else
        {
            __kfifo_set_out(fifo,find_pos);
            return 0;
        }
    }
    return -1;
}

void rtdlog_crash_dump (int type)
{
    struct kfifo * pKfifo = NULL;
    rtd_msg_header header;
    unsigned int out_len, out_tmp_pos;
    int header_check = 0;
    unsigned char * buffer;
    unsigned int log_max_len = 100000;
    unsigned int log_tmp_len = 0;
    bool tmp_printk_log = false;
    struct mutex * mtx = &read_loop_mutex;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
    struct __kernel_old_timeval log_time;
#else
    struct timeval log_time;
#endif

    if(type > kernel_cpu_type|| (!is_setup_rtd_logbuf))
    {
        return;
    }

    tmp_printk_log = is_save_early_log_done;
    is_save_early_log_done = false;

    rtd_pr_rtdlog_err("\n\n\n\n\n\n------kernel panic  trigger dump [%s] logbuf START------\n\n",file_info[type].g_file_name);
    mutex_lock(mtx);        // forbidden read thread verify kfifo.out
    pRtdLogCB = rtdlog_phys_to_virt(file_info[type].pRtdLogCB);
    array_fifo = &(pRtdLogCB->fifo);
    pKfifo = array_fifo;
    //decreas fifo->out value & find first header addr
    rtd_pr_rtdlog_err("before decrease Kfifo out=0x%08x, Kfifo in=0x%08x\n",(unsigned int)(pKfifo->out), (unsigned int)(pKfifo->in));
    if(rtdlog_find_header(pKfifo, type)<0)
    {
        if(pKfifo->in == 0)
        {
            goto RET;
        }
        rtd_pr_rtdlog_err("find header fail\n");
        rtd_pr_rtdlog_err("pKfifo->out=0x%08x, pKfifo->in=0x%08x\n",(unsigned int)(pKfifo->out), (unsigned int)(pKfifo->in));
        goto RET;
    }
    out_tmp_pos = pKfifo->out;
    rtd_pr_rtdlog_err("after decrease Kfifo out=0x%08x, Kfifo in=0x%08x\n",(unsigned int)(pKfifo->out), (unsigned int)(pKfifo->in));

    while((pKfifo->out) != (pKfifo->in))
    {
        //get msg header
        header_check = get_msg_header_avk_epc(type,&header,pKfifo);
        if (header_check == 1)
        {
            if(header.len < sizeof(msg_tmp_buf))
            {
                buffer = msg_tmp_buf;
                memset(buffer,0,sizeof(msg_tmp_buf));
            }
            else
            {
                buffer = kmalloc(header.len,GFP_KERNEL);
                if (buffer==NULL)
                {
                    goto RET ;
                }

                memset(buffer,0,header.len);
            }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
            log_time = ns_to_kernel_old_timeval((s64)header.timestamp);
#else
            log_time = ns_to_timeval((s64)header.timestamp);
#endif
            out_len = kfifo_out_peek(pKfifo,buffer,header.len,0);
            //print log
            rtd_pr_rtdlog_err( "[%s] [%08d.%06d] %s",file_info[type].g_file_name, (int)log_time.tv_sec, (int)log_time.tv_usec, buffer+sizeof(rtd_msg_header));
            __kfifo_add_out(pKfifo, out_len);            //if write success,move out offset to next msg
            if((buffer != msg_tmp_buf)&&(buffer != NULL))//free malloced buffer
            {
                kfree(buffer);
            }

            log_tmp_len = log_tmp_len + out_len;
            if(log_tmp_len > log_max_len)   //beyond log length
            {
                break;
            }

        }
        else
        {
            rtd_pr_rtdlog_err("Kfifo out=%d, Kfifo in=%d\n",(unsigned int)(pKfifo->out), (unsigned int)(pKfifo->in));
            break;
        }
    }
    __kfifo_set_out(pKfifo, out_tmp_pos);

RET:
    rtd_pr_rtdlog_err("\n\n------dump [%s] logbuf FINISH------!!!\n\n\n\n\n",file_info[type].g_file_name);
    mutex_unlock(mtx);

    is_save_early_log_done = tmp_printk_log;
    return;
}

static int rtdlog_panic (struct notifier_block *this, unsigned long ev, void *ptr)
{
    int rtdlog_panic_cpu_type = 0;

    if(!is_setup_rtd_logbuf)        //panic before logbuf set
    {
        return NOTIFY_DONE;
    }

    for(rtdlog_panic_cpu_type = 0; rtdlog_panic_cpu_type<=kernel_cpu_type; rtdlog_panic_cpu_type++)
    {
#ifdef CONFIG_REALTEK_LOGBUF
        console_flush_on_panic(CONSOLE_FLUSH_PENDING);       //dump native logbuf to console
#endif
        rtdlog_crash_dump(rtdlog_panic_cpu_type);
    }

    rtdlog_show_history_keylog(rtd_keylog_addr, RTD_LOGBUF_KEYLOG_SIZE);
    return NOTIFY_DONE;
}

/********************************************************
* rtdlog status APIs
*********************************************************/
ssize_t rtdlog_get (char *buffer)
{
    int type;
    ssize_t pos=0;
    logbuf_rbcb * RtdLogCB = NULL;
    struct kfifo * Kfifo=NULL;

    for(type = 0; type < total_logbuf_num; ++type)
    {
        RtdLogCB = rtdlog_phys_to_virt(file_info[type].pRtdLogCB);
        Kfifo = &(RtdLogCB->fifo);
        pos+=snprintf(buffer+pos, PAGE_SIZE-1-pos, "\t %10s abnormal cnt(drop %08lx reset %08lx)   buf(%08lx),size(%08lx),in(%08lx),out(%08lx),logswi(%08lx)\n",
            file_info[type].g_file_name,(unsigned long)(RtdLogCB->drop_cnt),(unsigned long)(RtdLogCB->in_reset),(unsigned long)(Kfifo->buffer),
            (unsigned long)(Kfifo->size), (unsigned long)(Kfifo->in),(unsigned long)(Kfifo->out),bootcode_logswi);
    }
    pos+=snprintf(buffer+pos, PAGE_SIZE-1-pos, "\t cpu_type acpu1=%x kcpu2=%x vcpu1=%x vcpu2=%x vcpu3=%x kcpu=%x kernel=%x hal=%x\n", 
        acpu1_cpu_type,kcpu2_cpu_type,vcpu1_cpu_type,vcpu2_cpu_type,vcpu3_cpu_type,kcpu_cpu_type,kernel_cpu_type,hal_cpu_type);
    pos+=snprintf(buffer+pos, PAGE_SIZE-1-pos, "\t read_thread_sleep_span =%d flag_status = %d flag_compress = %d\n",
        read_thread_sleep_span,flag_status,flag_compress);
    
    return pos;
}
EXPORT_SYMBOL(rtdlog_get);

#define LOG_DUMP_FILE_NAME "rtd_logbuf.bin"
#ifdef SUPPORT_LOGBUF_FILE_OPERAIONS
static void dump_log_data (void)
{
   return;
}
#else
static void dump_log_data (void)
{
    char dump_log_path[LOGBUF_DIR_MAX_SIZE] = {0};
    struct file * log_file = NULL;
    loff_t pos = 0;
    int write_len;

    snprintf(dump_log_path, LOGBUF_DIR_MAX_SIZE-1, "%s/%s",RTDLOG_DEFAULT_PATH,LOG_DUMP_FILE_NAME);
    log_file = filp_open(dump_log_path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(IS_ERR(log_file))
    {
        rtd_pr_rtdlog_err("%s %d.  open %s failed\n",__func__,__LINE__,dump_log_path);
        return;
    }
    write_len=kernel_write(log_file, (rtdlog_phys_to_virt(RTD_LOGBUF_PHY_START_ADDR)), rtdlog_bufsize, &pos);
    filp_close(log_file, NULL);
    rtd_pr_rtdlog_err("%s %d.  write_len = %x, actual len =%x \n",__func__,__LINE__,write_len,rtdlog_bufsize);
}
#endif
static int parse_rtdlog_sysnode_opera(char * buf, rtdlog_sysnode_param * param)
{
    int cmd_type_num = 0;
    int i = 0;
    char *rtdlog_cmd_type[] =
    {
        "set",
        "unset",
        "dbg",
        "help",
        "block",
        "dump",
        "msgon",
        "msgoff",
        "cat",
        "uncat",
        "savelevel",
        "sleep",
        "logfilter"
    };

    cmd_type_num = sizeof(rtdlog_cmd_type)/sizeof(char *);

    if(buf == NULL)
    {
        rtd_pr_rtdlog_err("NULL point in %s\n", __FUNCTION__);
        return -1;
    }

    for(i=0;i<cmd_type_num;i++)
    {
        if(strcmp(buf, rtdlog_cmd_type[i]) == 0)
        {
            param->opera = i;
            return 0;
        }
    }

    rtd_pr_rtdlog_err("unknow operation:%s\n", buf);
    return -1;
}

static void rtdlog_set_sleep_span(rtdlog_sysnode_param * param)
{
    read_thread_sleep_span = param->sleep_span;
    rtd_pr_rtdlog_err("current sleep span:%d ms\n",read_thread_sleep_span);
}

static void rtd_dmsg_set(rtdlog_sysnode_param * param,int val)
{
    int i;

    for(i = 0; i < total_logbuf_num; ++i)
    {
        if(!(param->buf_idx[i]))
        {
            continue;
        }
        if(val)
            bootcode_logswi |= (1<<i);
        else
            bootcode_logswi &= (~ (1<<i));
    }
}

static void rtdlog_filter_log(rtdlog_sysnode_param * param,int val)
{
    int i;

    for(i = 0; i < total_logbuf_num; ++i)
    {
        if(!(param->buf_idx[i]))
        {
            continue;
        }
        if(val)
            bootcode_logswi |= (1<<(i+8));
        else
            bootcode_logswi &= (~ (1<<(i+8)));
    }
}

static void rtdlog_filter_logblock(void)
{
    static int toggle_flag = 0;
    struct mutex * mtx = &read_loop_mutex;

    if(toggle_flag)
    {
        mutex_unlock(mtx);
        toggle_flag = 0;
        flag_status = 1;
        rtd_pr_rtdlog_err("RtdLogReader is released...\n");
    }
    else
    {
        mutex_lock(mtx);
        toggle_flag = 1;
        flag_status = 0;
        rtd_pr_rtdlog_err("RtdLogReader is blocked...\n");
    }

}

static void rtdlog_help_info(void)
{
    rtd_pr_rtdlog_err("rtdlog cmd usage:\n");
    rtd_pr_rtdlog_err("1)set buf_1,buf_2,... \n");
    rtd_pr_rtdlog_err("\t note: modify the log filter setting\n");
    rtd_pr_rtdlog_err("\t buf:all,acpu1,acpu2,vcpu1,vcpu2,vcpu3,kcpu1,kernel,hal\n");
    rtd_pr_rtdlog_err("\t exp: echo \"set acpu1,kcpu1 \" > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("2)unset buf_1,buf_2,... \n");
    rtd_pr_rtdlog_err("\t note: prevent the log which come from specified logbuf to input in log-file\n");
    rtd_pr_rtdlog_err("\t exp: echo \"unset acpu1,kcpu1 \" > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("3)dbg buf_1,buf_2,... \n");
    rtd_pr_rtdlog_err("\t note: show debug info\n");
    rtd_pr_rtdlog_err("\t exp: echo \"dbg acpu1,kcpu \" > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("4)block \n");
    rtd_pr_rtdlog_err("\t note: block/unblock RtdLogReader Thread\n");
    rtd_pr_rtdlog_err("\t exp: echo block > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("5)logcat \n");
    rtd_pr_rtdlog_err("\t note: cat log in rtd logbuf\n");
    rtd_pr_rtdlog_err("\t exp: cat /dev/rtd-logger\n");
    rtd_pr_rtdlog_err("6)dump \n");
    rtd_pr_rtdlog_err("\t note: dump log-data to logbuf.bin\n");
    rtd_pr_rtdlog_err("\t exp: echo dump > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("7)msgon buf_1,buf_2,... \n");
    rtd_pr_rtdlog_err("\t note: kernel will print other fw log\n");
    rtd_pr_rtdlog_err("\t exp: echo \"msgon acpu1,vcpu1 \" > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("8)msgoff buf_1,buf_2,... \n");
    rtd_pr_rtdlog_err("\t note: kernel will not print other fw log\n");
    rtd_pr_rtdlog_err("\t exp: echo \"msgoff acpu1,vcpu1 \" > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("9)cat buf_1,buf_2,... core:-- mod:-------- lv:--\n");
    rtd_pr_rtdlog_err("\t note: set the log-cat filter setting\n");
    rtd_pr_rtdlog_err("\t exp: echo \"cat acpu1,kcpu1 core:0 mod:0 lv:5\" > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("10)uncat buf_1,buf_2,... \n");
    rtd_pr_rtdlog_err("\t note: unset the log-cat filter setting\n");
    rtd_pr_rtdlog_err("\t exp: echo \"uncat acpu1,kcpu1 \" > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("11)savelevel buf lv\n");
    rtd_pr_rtdlog_err("\t note: only loglevel <= savelevel,log can save ringbuf\n");
    rtd_pr_rtdlog_err("\t buf:all,acpu1,acpu2,vcpu1,vcpu2,vcpu3,kcpu1,kernel,hal\n");
    rtd_pr_rtdlog_err("\t exp: echo \"savelevel kernel 5\" > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("12)sleep time \n");
    rtd_pr_rtdlog_err("\t note: set read thread sleep span,unit is ms ,default time is 10\n");
    rtd_pr_rtdlog_err("\t exp: echo \"sleep 10\" > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("13)module filter \n");
    rtd_pr_rtdlog_err("\t note: set kdriver log filter ,default modulr is ALL ,level is 6\n");
    rtd_pr_rtdlog_err("\t exp: echo \"logfilter CMD MODULE  LEVEL SAVEFLAG\" > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("\t CMD:reset,module_set, module_remv,module_add,module_keep,show_module\n");
    rtd_pr_rtdlog_err("\t MODULE:id_range=XXX-XXX;name=XXX,XXX;id=XXX,XXX \n");
    rtd_pr_rtdlog_err("\t LEVEL:level=X SAVEFLAG:save_flag=x\n");
    rtd_pr_rtdlog_err("\t exp: echo \"logfilter module_set id=2,6,10 level=6 save_flag=1\" > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("\t exp: echo \"logfilter module_remv name=HDMI,PQ\" > /sys/realtek_boards/rtdlog\n");
    rtd_pr_rtdlog_err("\t exp: echo \"logfilter module_add id_range=1-10 level=6\" > /sys/realtek_boards/rtdlog\n");
}

static void rtdlog_filter_logdbg(rtdlog_sysnode_param * param)
{
    int i;
    logbuf_rbcb * RtdLogCB = NULL;
    struct kfifo * Kfifo=NULL;

    rtd_pr_rtdlog_err("===============RTDLOG DBG STATUS===============\n");
    rtd_pr_rtdlog_err("logswi:%08lx  logdir=%s\n", bootcode_logswi, logbuf_dir);
    rtd_pr_rtdlog_err("read_thread_sleep_span =%d flag_status = %d flag_compress = %d\n",
        read_thread_sleep_span,flag_status,flag_compress);
    rtd_pr_rtdlog_err("filesize(total_file:0x%lx a1:0x%lx a2:0x%lx v1:0x%lx v2:0x%lx)\n",
                pRtdLogfilesize->total_file, pRtdLogfilesize->acpu1_file, pRtdLogfilesize->acpu2_file, pRtdLogfilesize->vcpu1_file, pRtdLogfilesize->vcpu2_file);
    for(i = 0; i < total_logbuf_num; ++i)
    {
        if(param->buf_idx[i])
        {
            RtdLogCB = rtdlog_phys_to_virt(file_info[i].pRtdLogCB);
            Kfifo = &(RtdLogCB->fifo);
            rtd_pr_rtdlog_err("---%6s---\n",file_info[i].g_file_name);
            rtd_pr_rtdlog_err("savelevel:%02x\n",RtdLogCB->savelevel);
            if(cat_filter != NULL)
            {
                rtd_pr_rtdlog_err(
                        "<log-cat >(en:%s  level:%02x  core:%02x  module:%08x)\n",
                        cat_filter->main_mask & (1 << i) ? "disabled" : "enabled",
                        cat_filter->sub_mask[i].level,
                        cat_filter->sub_mask[i].core_mask,
                        cat_filter->sub_mask[i].module_mask);
            }
            rtd_pr_rtdlog_err("\t abnormal cnt(drop %08lx  reset %08lx)\n",
                        (unsigned long)(RtdLogCB->drop_cnt),(unsigned long)(RtdLogCB->in_reset));
            rtd_pr_rtdlog_err("\t kfifo(buf %08lx,size %08lx,in %08lx,out %08lx,avail %08x,cat %08lx)\n",
                        (unsigned long)(RtdLogCB->fifo.buffer), (unsigned long)(RtdLogCB->fifo.size),
                        (unsigned long)(RtdLogCB->fifo.in),(unsigned long)(RtdLogCB->fifo.out),
                        kfifo_avail(Kfifo),
                        (unsigned long)(RtdLogCB->fifo.out_cat));
        }
    }
    rtd_pr_rtdlog_err("\t cpu_type acpu1=%x kcpu2=%x vcpu1=%x vcpu2=%x vcpu3=%x kcpu=%x kernel=%x hal=%x\n",
        acpu1_cpu_type,kcpu2_cpu_type,vcpu1_cpu_type,vcpu2_cpu_type,vcpu3_cpu_type,kcpu_cpu_type,kernel_cpu_type,hal_cpu_type);
    rtd_pr_rtdlog_err("read thread sleep span: %d ms\n",read_thread_sleep_span);

}

/*
** parse rtdlog savelevel
*/
static int parse_rtdlog_save_level(char * buf, rtdlog_sysnode_param * param)
{
    if(sscanf(buf, "%hhu", &param->level) < 1)
    {
        rtd_pr_rtdlog_err("parse level range fail\n");
        return -1;
    }

    param->level_modify = 1;

    return 0;
}

static int rtd_set_savelevel( int  type,int savelevel)
{
    logbuf_rbcb * pRbcb;

    if(type < total_logbuf_num)
    {
        pRbcb = rtdlog_phys_to_virt(file_info[type].pRtdLogCB);
        pRbcb->savelevel = savelevel;
        return 0;
    }

    return -1;
}

static void rtdlog_savelevel_set(rtdlog_sysnode_param * param)
{
    int i;
    for(i = 0; i < total_logbuf_num; ++i)
    {
        if(param->buf_idx[i] && param->level_modify)
        {
            rtd_set_savelevel(i, param->level);
        }
    }
}

static int rtdlog_implement_param(rtdlog_sysnode_param * param)
{
    if(param == NULL)
    {
        rtd_pr_rtdlog_err("NULL point in %s\n", __FUNCTION__);
        return -1;
    }

    //check operation
    switch(param->opera)
    {
        case LOG_ON:
            rtdlog_filter_log(param,1);
            break;
        case LOG_OFF:
            rtdlog_filter_log(param,0);
            break;
        case LOG_DBG:
            rtdlog_filter_logdbg(param);
            break;
        case LOG_HELP:
            rtdlog_help_info();
            break;
        case LOG_BLOCK:
            rtdlog_filter_logblock();
            break;
        case LOG_DUMP:
            dump_log_data();
            break;
        case LOG_DMSG_ON:
            rtd_dmsg_set(param,1);
            break;
        case LOG_DMSG_OFF:
            rtd_dmsg_set(param,0);
            break;
        case LOG_CAT_ON:
            rtdlog_filter_caton(param);
            break;
        case LOG_CAT_OFF:
            rtdlog_filter_catoff(param);
            break;
        case LOG_SAVELEVEL:
            rtdlog_savelevel_set(param);
            break;
        case LOG_SLEEP_SPAN:
            rtdlog_set_sleep_span(param);
            break;

        default:
            break;
    }

    return 0;
}

static int split_str(char * str, char split_flag, char out_str[][16], int max_cnt)
{
    int num = 0;
    int i;
    int start;
    int end;
    if((str == NULL) || (out_str == NULL))
    {
        return -1;
    }
    //get strings
    for(start = 0, end = 0; str[end] != '\0'; ++end)
    {
        if(str[end] == split_flag)
        {
            if(start < end)   // if any char between start-end range;
            {
                for(i = 0; start < end; start++, i++)   // copy string
                {
                    out_str[num][i] = str[start];
                }
                start = end + 1; //ignore the split str;
                num++;
                if(num >= max_cnt)
                {
                    return num;
                }
            }
            else
            {
                start = end + 1; //ignore the split str;
            }
        }
    }
    //get last string
    if(start < end)   // if any char between start-end range;
    {
        for(i = 0; start < end; start++, i++)
        {
            out_str[num][i] = str[start];
        }
        num++;
    }

    return num;
}

static int get_buftype_id(char * buf)
{
    int return_id = -1;
    int i = 0;

    if(buf == NULL)
    {
        rtd_pr_rtdlog_err("NULL point in %s\n", __FUNCTION__);
        return -1;
    }

    if ((strcmp(buf, "all") == 0)||(strcmp(buf, "ALL") == 0))
    {
        return_id = total_logbuf_num;
    }

    for(i=0;i<total_logbuf_num;i++)
    {
        if(strcmp(file_info[i].g_file_name,buf) == 0)
           return i;
    }

    return return_id;
}

static int parse_rtdlog_sysnode_buf(char * buf, rtdlog_sysnode_param * param)
{
    int i, j;
    int buf_num;
    int buf_id;
    char buf_str[16][16] = {{0}};

    if(buf == NULL)
    {
        rtd_pr_rtdlog_err("NULL point in %s\n", __FUNCTION__);
        return -1;
    }

    buf_num = split_str(buf, ',', buf_str, total_logbuf_num);
    for(i = 0; i < buf_num; ++i)
    {
        buf_str[i][15] = '\0';
        buf_id = get_buftype_id(&buf_str[i][0]);
        if(buf_id < 0)
        {
            return -1;
        }

        if(buf_id == total_logbuf_num)
        {
            for(j = 0; j < total_logbuf_num; ++j)
            {
                param->buf_idx[j] = 1;
            }
            break;
        }
        else
        {
            param->buf_idx[buf_id] = 1;
        }
    }

    return 0;
}
static int parse_rtdlog_kdriver_set_filter_by_id_range(char * buf_id,char * buf_level,char * buf_saveflag)
{
    int start_id = 0;
    int end_id =0;
    int level = 0;
    int save_flag = 1;
    int i;

    if(sscanf(buf_id, "id_rang=%d-%d", &start_id,&end_id) < 2)
    {
        rtd_pr_rtdlog_err("parse kdriver_filter id_range %s fail\n",buf_id);
        return -1;
    }

    if(end_id>=sizeof(module_map)/sizeof(struct MODULE_MAP))
        end_id = sizeof(module_map)/sizeof(struct MODULE_MAP) -1;

    if(sscanf(buf_level, "level=%d", &level) >0)
    {
        for(i=start_id;i <= end_id;i++ )
            module_map[i].level = level;
    }

    if(sscanf(buf_saveflag, "save_flag=%d", &save_flag) > 0)
    {
        for(i=start_id;i <= end_id;i++ )
        module_map[i].save_flag= save_flag;
    }

    return 0;
}
static int get_module_id(char * buf)
{
    int return_id = -1;
    int i = 0;

    if(buf == NULL)
    {
        rtd_pr_rtdlog_err("NULL point in %s\n", __FUNCTION__);
        return -1;
    }
    if ((strcmp(buf, "all") == 0)||(strcmp(buf, "ALL") == 0))
    {
        return  RTD_LOG_MODULE_MAX_ID;
    }
    for(i=0;i<RTD_LOG_MODULE_MAX_ID;i++)
    {
        if(strcmp(module_map[i].name,buf) == 0)
           return module_map[i].module_id;
    }

    return return_id;
}
#define max_filter_num 8
static int parse_rtdlog_kdriver_set_filter_by_name(char * buf_name,char * buf_level,char * buf_saveflag)
{
    int id_num[max_filter_num];
    int level = 0;
    int save_flag = 1;
    int i;
    int buf_num = 0;
    char buf_str[16][16] = {{0}};

    buf_num = split_str(buf_name, ',', buf_str, max_filter_num);
    for(i = 0; i < buf_num; ++i)
    {
        buf_str[i][15] = '\0';
        id_num[i] = get_module_id(&buf_str[i][0]);
        if(id_num[i] < 0)
        {
            rtd_pr_rtdlog_err("parse_rtdlog_kdriver_set_filter_by_id_name fail %s\n",&buf_str[i][0]);
            return -1;
        }
        if(id_num[i] == RTD_LOG_MODULE_MAX_ID)
        {
            buf_name = "id_rang=0-1000";
            parse_rtdlog_kdriver_set_filter_by_id_range(buf_name,buf_level,buf_saveflag);
            return 0;
        }
    }

    if(sscanf(buf_level, "level=%d", &level) > 0)
    {
        for(i=0;i < buf_num;i++ )
            module_map[id_num[i]].level = level;
    }

    if(sscanf(buf_saveflag, "save_flag=%d", &save_flag) > 0)
    {
        for(i=0;i < buf_num;i++ )
        module_map[id_num[i]].save_flag= save_flag;
    }

    return 0;
}
static int parse_rtdlog_kdriver_set_filter_by_id(char * buf_id,char * buf_level,char * buf_saveflag)
{
    unsigned long id_num[max_filter_num];
    int level = 0;
    int save_flag = 1;
    int i;
    int buf_num = 0;
    char buf_str[16][16] = {{0}};

    buf_num = split_str(buf_id, ',', buf_str, max_filter_num);
    for(i = 0; i < buf_num; ++i)
    {
        buf_str[i][15] = '\0';
        if (kstrtoul(&buf_str[i][0],10,&id_num[i])){
            rtd_pr_rtdlog_err("%s invalid str:%s\n",__FUNCTION__, &buf_str[i][0]);
            return -1;
        }
        if(id_num[i] > RTD_LOG_MODULE_MAX_ID)
        {
            rtd_pr_rtdlog_err("parse_rtdlog_kdriver_set_filter_by_id fail %d\n",id_num[i]);
            return -1;
        }
    }

    if(sscanf(buf_level, "level=%d", &level) > 0)
    {
        for(i=0;i < buf_num;i++ )
             module_map[id_num[i]].level = level;
    }

    if(sscanf(buf_saveflag, "save_flag=%d", &save_flag) > 0)
    {
        for(i=0;i < buf_num;i++ )
        module_map[id_num[i]].save_flag= save_flag;
    }

    return 0;
}

void parse_rtdlog_kdriver_filter_none(void)
{
    int i ;
    for(i=0;i < RTD_LOG_MODULE_MAX_ID;i++ )
    {
        module_map[i].save_flag= 0;
    }
}

void parse_rtdlog_kdriver_filter_reset(void)
{
    int i ;
    for(i=0;i < RTD_LOG_MODULE_MAX_ID;i++ )
    {
        module_map[i].save_flag= 1;
        module_map[i].level= 6;
    }
}
void parse_rtdlog_kdriver_filter_show_module(void)
{
    int i ;
    rtd_pr_rtdlog_err("\t %3s %5s %16s %s\n","id","level","name","save_flag");
    for(i=0;i < RTD_LOG_MODULE_MAX_ID;i++ )
    {
         rtd_pr_rtdlog_err( "\t %3d %5d %16s %2d\n",
             module_map[i].module_id,module_map[i].level,module_map[i].name,module_map[i].save_flag);
    }
}
static int parse_rtdlog_kdriver_filter(char * buf_set,char * op1_cmd,char * op2_cmd)
{
    char *buf_level = "level=";
    char *buf_saveflag ="save_flag=";

    if(strncmp(op1_cmd,"level=",6) == 0)
    {
       buf_level = op1_cmd;
       if(strncmp(op2_cmd,"save_flag=",10) == 0)
           buf_saveflag = op2_cmd;
    }
    else if(strncmp(op1_cmd,"save_flag=",10) == 0)
    {
       buf_saveflag = op1_cmd;
       if(strncmp(op2_cmd,"level=",6))
           buf_level = op2_cmd;
    }
    else if(strncmp(op2_cmd,"save_flag=",10) == 0)
    {
        buf_saveflag = op2_cmd;
    }
    else
    {
        rtd_pr_rtdlog_err("parse rtdlog kdriver filter %s %s %s %s %s\n",buf_set,op1_cmd,op2_cmd,buf_level,buf_saveflag);
    }

    if(strncmp(buf_set,"id_range=",9) == 0)
    {
       parse_rtdlog_kdriver_set_filter_by_id_range(buf_set,buf_level,buf_saveflag);
    }
    else if(strncmp(buf_set,"name=",5) == 0)
    {
        parse_rtdlog_kdriver_set_filter_by_name(buf_set+5,buf_level,buf_saveflag);
    }
    else if(strncmp(buf_set,"id=",3) == 0)
    {
        parse_rtdlog_kdriver_set_filter_by_id(buf_set+3,buf_level,buf_saveflag);
    }
    else
    {
        rtd_pr_rtdlog_err("parse rtdlog kdriver filter fail %s\n",buf_set);
    }
    return 0;
}
static int parse_rtdlog_kdriver_filter_cmd(char * buf_cmd,char * op1_cmd,char * op2_cmd,char * op3_cmd)
{
    if(buf_cmd == NULL)
    {
        rtd_pr_rtdlog_err("NULL point in %s\n", __FUNCTION__);
        return -1;
    }

    if(strcmp(buf_cmd,"reset")==0)
    {
        parse_rtdlog_kdriver_filter_reset();
    }
    else if(strcmp(buf_cmd,"module_set")==0)
    {
        parse_rtdlog_kdriver_filter(op1_cmd,op2_cmd,op3_cmd);
    }
    else if(strcmp(buf_cmd,"module_remv")==0)
    {
        op3_cmd = "save_flag=0";
        parse_rtdlog_kdriver_filter(op1_cmd,op2_cmd,op3_cmd);
    }
    else if(strcmp(buf_cmd,"module_add")==0)
    {
        op3_cmd = "save_flag=1";
        parse_rtdlog_kdriver_filter(op1_cmd,op2_cmd,op3_cmd);
    }
    else if(strcmp(buf_cmd,"module_keep")==0)
    {
        parse_rtdlog_kdriver_filter_none();
        op3_cmd = "save_flag=1";
        parse_rtdlog_kdriver_filter(op1_cmd,op2_cmd,op3_cmd);
    }
    else if(strcmp(buf_cmd,"show_module")==0)
    {
        parse_rtdlog_kdriver_filter_show_module();
    }
    else
    {
        rtd_pr_rtdlog_err("%s  fail buf_cmd=%s\n", __FUNCTION__,buf_cmd);
    }
    return 0;
}
static int parse_rtdlog_sleep_span(char * buf, rtdlog_sysnode_param * param)
{
    if(sscanf(buf, "%d", &param->sleep_span) < 1)
    {
        rtd_pr_rtdlog_err("parse sleep span fail\n");
        return -1;
    }

    return 0;
}

static int parse_rtdlog_sysnode_option(char * buf, rtdlog_sysnode_param * param)
{
    //int i, j;
    char split_buf[2][16] = {{0}};
    int split_num;

    if(buf == NULL)
    {
        rtd_pr_rtdlog_err("NULL point in %s\n", __FUNCTION__);
        return -1;
    }

    if(buf[0] == 0)
    {
        return 0;
    }

    split_num = split_str(buf, ':', split_buf, 2);
    if(split_num != 2)
    {
        rtd_pr_rtdlog_err("unknown option field:%s\n", buf);
        return -1;
    }

    if(strcmp(&split_buf[0][0], "core") == 0)
    {
        if(sscanf(&split_buf[1][0], "%hhu", &param->core_mask) < 1)
        {
            rtd_pr_rtdlog_err("parse core mask fail\n");
            return -1;
        }
        param->core_modify = 1;
    }
    else if(strcmp(&split_buf[0][0], "mod") == 0)
    {
        if(sscanf(&split_buf[1][0], "%x", &param->module_mask) < 1)
        {
            rtd_pr_rtdlog_err("parse module mask fail\n");
            return -1;
        }
        param->module_modify = 1;
    }
    else if(strcmp(&split_buf[0][0], "lv") == 0)
    {
        if(sscanf(&split_buf[1][0], "%hhu", &param->level) < 1)
        {
            rtd_pr_rtdlog_err("parse level range fail\n");
            return -1;
        }
        param->level_modify = 1;
    }

    return 0;
}

static int parse_rtdlog_sysnode_cmdline(const char * buf, rtdlog_sysnode_param * param)
{
    char set_field[10] = {0};
    char buf_field[64] = {0};
    char op1_filed[64] = {0};
    char op2_filed[16] = {0};
    char op3_filed[16] = {0};
    int ret = 0;

    if(buf == NULL)
    {
        rtd_pr_rtdlog_err("NULL point in %s\n", __FUNCTION__);
        return -1;
    }

    //get options
    if (sscanf(buf, "%9s %63s %63s %15s %15s", set_field, buf_field,op1_filed,op2_filed,op3_filed) < 1)
    {
        rtd_pr_rtdlog_err("get cmdline fail!\n");
        return -1;
    }

    if(parse_rtdlog_sysnode_opera(set_field, param))
    {
        rtd_pr_rtdlog_err("get operation fail!\n");
        return -1;
    }

    switch(param->opera)
    {
        case LOG_ON:
        case LOG_OFF:
        case LOG_DMSG_ON:
        case LOG_DMSG_OFF:
        case LOG_DBG:
        case LOG_CAT_OFF:
            if(parse_rtdlog_sysnode_buf(buf_field, param))
            {
                rtd_pr_rtdlog_err("get buf type fail!\n");
                return -1;
            }
            break;
        case LOG_CAT_ON:
            if(parse_rtdlog_sysnode_buf(buf_field, param))
            {
                rtd_pr_rtdlog_err("get buf type fail!\n");
                return -1;
            }
            ret = 0;
            ret |= parse_rtdlog_sysnode_option(op1_filed, param);
            ret |= parse_rtdlog_sysnode_option(op2_filed, param);
            ret |= parse_rtdlog_sysnode_option(op3_filed, param);
            if(ret != 0)
            {
                return -1;
            }
            break;
        case LOG_SAVELEVEL:
            if(parse_rtdlog_sysnode_buf(buf_field, param))
            {
                rtd_pr_rtdlog_err("get buf type fail!\n");
                return -1;
            }

            if(parse_rtdlog_save_level(op1_filed, param))
            {
                return -1;
            }
            break;
        case LOG_SLEEP_SPAN:
            parse_rtdlog_sleep_span(buf_field, param);
            break;
        case LOG_KDRIVER_FILTER:
            parse_rtdlog_kdriver_filter_cmd(buf_field,op1_filed,op2_filed,op3_filed);
            break;
        default:
            break;
    }

    return 0;
}

void rtdlog_set (const char *buffer)
{
    rtdlog_sysnode_param filter_param;

    rtd_pr_rtdlog_warn("%s called\n", __FUNCTION__);
    memset(&filter_param, 0, sizeof(rtdlog_sysnode_param));
    filter_param.buf_idx = kmalloc(total_logbuf_num*sizeof(char),GFP_KERNEL);
    memset(filter_param.buf_idx, 0, total_logbuf_num*sizeof(char));
    if(parse_rtdlog_sysnode_cmdline(buffer, &filter_param))
    {
        rtd_pr_rtdlog_err("parse cmd fail!\n");
        goto Fail;
    }

    if(rtdlog_implement_param(&filter_param))
    {
        rtd_pr_rtdlog_err("implement param fail!\n");
        goto Fail;
    }

Fail:
    if(filter_param.buf_idx != NULL)
        kfree(filter_param.buf_idx);
    return;
}
EXPORT_SYMBOL(rtdlog_set);
