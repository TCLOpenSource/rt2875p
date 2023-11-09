/******************************************************************************
 *
 *   Copyright(c) 2021. Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author timo.wu@realtek.com
 *
 *****************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/compat.h>

#include <linux/fs.h> // chrdev
#include <linux/cdev.h> // cdev_add()/cdev_del()
#include <linux/slab.h> // kmalloc()/kfree()
#include <linux/uaccess.h> // copy_*_user()
#include <linux/proc_fs.h>

#ifdef CONFIG_HIBERNATION
#include <linux/pm.h>
#endif

#include "rbus/iso_misc_irda_reg.h"
#include "rbus/MIO/smart_vad_reg.h"
#include "rbus/MIO/mio_sys_reg_reg.h"

#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_crt.h>
#include <rtk_kdriver/rtk_svad_def.h>
#include <rtk_kdriver/rtk_svad_api.h>


//#define EN_KERNEL_AUTO_INIT_SVAD

/******************************************************************************
                     SVAD only works on WOV DMIC path
*****************************************************************************/

SRTK_SVAD_DEBUG_CMD Svad_debug_cmd[SVAD_PROC_COUNT + 1]={
    {"log_onoff=",    SVAD_PROC_SETLOGONOFF},
    {"model01=",      SVAD_PROC_SETMODEL01},
    {"model02=",      SVAD_PROC_SETMODEL02},
    {"NULL",          SVAD_PROC_COUNT}
};

unsigned char svad_log_onoff = 0;
unsigned char g_thread_en = 0;
struct task_struct *svad_kernelthread;
struct semaphore rtksvad_Semaphore;/* This Semaphore is for rtksvad control*/

/* parameters for debug */
struct proc_dir_entry * svad_proc_dir = NULL;
struct proc_dir_entry * svad_proc_entry = NULL;

#define RTK_SVAD_DEV_BUFSIZE 1024

#define RTK_SVAD_MAJOR 0
#define RTK_SVAD_MINOR 0

static int rtk_svad_major = RTK_SVAD_MAJOR;
static int rtk_svad_minor = RTK_SVAD_MINOR;

module_param(rtk_svad_major, int, S_IRUGO);
module_param(rtk_svad_minor, int, S_IRUGO);

static struct class *rtk_svad_class;
static struct cdev rtk_svad_cdev;
static struct platform_device *rtk_svad_platform_devs;

#ifdef CONFIG_HIBERNATION
static int rtk_svad_std_suspend(struct device *dev);
static int rtk_svad_std_resume(struct device *dev);
static int rtk_svad_str_suspend(struct device *dev);
static int rtk_svad_str_resume(struct device *dev);

static const struct dev_pm_ops rtk_svad_pm_ops = {
    .freeze     = rtk_svad_std_suspend,
    .thaw       = rtk_svad_std_resume,
    .poweroff   = rtk_svad_std_suspend,
    .restore    = rtk_svad_std_resume,
    .suspend    = rtk_svad_str_suspend,
    .resume     = rtk_svad_str_resume,
};
#endif

#ifdef CONFIG_PM
static int rtk_svad_suspend(struct platform_device *dev, pm_message_t state);
static int rtk_svad_resume(struct platform_device *dev);
#endif

static struct platform_driver rtk_svad_device_driver = {
#ifdef CONFIG_PM
    .suspend        = rtk_svad_suspend,
    .resume         = rtk_svad_resume,
#endif
    .driver = {
        .name       = "rtksvad",
        .bus        = &platform_bus_type,
#ifdef CONFIG_HIBERNATION
        .pm         = &rtk_svad_pm_ops,
#endif
    },
};

int rtk_svad_open(struct inode *inode, struct file *filp);
int rtk_svad_release(struct inode *inode, struct file *filp);
ssize_t rtk_svad_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
ssize_t rtk_svad_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
long rtk_svad_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
long rtk_svad_ioctl_compat(struct file *filp, unsigned int cmd, unsigned long arg);
#endif
#endif

extern int rtk_svad_4bytes_i2c_wr(unsigned int addr, unsigned int data);
extern int rtk_svad_4bytes_i2c_rd(unsigned int addr, unsigned int* data);
extern int rtk_svad_4bytes_i2c_mask(unsigned int addr, unsigned int mask, unsigned int data);
extern int rtk_svad_crt_clk_onoff(enum CRT_CLK clken);

const struct file_operations rtk_svad_fops = {
    .owner = THIS_MODULE,
    .open = rtk_svad_open,
    .release = rtk_svad_release,
    .write = rtk_svad_write,
    .read = rtk_svad_read,
    .unlocked_ioctl = rtk_svad_ioctl,
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
    .compat_ioctl = rtk_svad_ioctl_compat,
#endif
#endif
};

int rtk_svad_open(struct inode *inode, struct file *filp)
{
    SVAD_DBG("%s():\n", __FUNCTION__);
    return 0;
}

int rtk_svad_release(struct inode *inode, struct file *filp)
{
    SVAD_DBG("%s():\n", __FUNCTION__);
    return 0;
}

ssize_t
rtk_svad_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    char data[] = "svad data\n";
    ssize_t ret = 0;
    int ret_ignore = 0;

    SVAD_DBG("%s():\n", __FUNCTION__);

    if (*f_pos >= sizeof(data)) {
        goto out;
    }

    if (count > sizeof(data)) {
        count = sizeof(data);
    }

    ret_ignore = copy_to_user(buf, data, count);
    if (ret_ignore < 0) {
        ret = -EFAULT;
        goto out;
    }
    *f_pos += count;
    ret = count;

out:
    return ret;
}

ssize_t
rtk_svad_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    unsigned char str[128];
    unsigned int i;
    unsigned char *cmd = str;
    unsigned int input_data;

#ifdef MIO_I2C_MODE
    int ret = FUNCTION_SUCCESS;
    unsigned int reg_value_sys_srst = 0;
    unsigned int reg_value_sys_clken = 0;
#endif

    /* check parameter */
    if (buf == NULL) {
        SVAD_DBG("buf=%p\n", buf);
        return -EFAULT;
    }

    if (count == 0) {
        SVAD_DBG("command should > 0\n");
        return -EINVAL;
    }
    
    if (count > 128) { /* procfs write and read has PAGE_SIZE limit */
        SVAD_DBG("command too large, set length to 128 Byte\n");
        count = 128;
    }

    if (copy_from_user(str, buf, count)) {
        SVAD_DBG("copy_from_user failed! (buf=%p, count="PT_UDEC_DUALADDRESS")\n", buf, count);
        return -EFAULT;
    }

    str[count-1] = '\0';

    SVAD_DBG("proc data: %s\n", str);

    /* get command string */
    for (i = 0; i < SVAD_PROC_COUNT; i++) {
        //SVAD_DBG("[%d]%s\n", i, Svad_debug_cmd[i].vad_cmd_str);
        if (strncmp(str, Svad_debug_cmd[i].svad_cmd_str, strlen(Svad_debug_cmd[i].svad_cmd_str)) == 0) {
            SVAD_DBG("SVAD debug command: %s\n", Svad_debug_cmd[i].svad_cmd_str);
            break;
        }
    }

    /* command out of range check */
    if (i >= SVAD_PROC_COUNT) {
        SVAD_DBG("Cannot find your command: \"%s\"\n", str);
        //return -EINVAL; for coverity

    } else {
        /* get parameter */
        cmd += strlen(Svad_debug_cmd[i].svad_cmd_str);
        if (sscanf(cmd, "%9u", &input_data) < 1) {
            SVAD_DBG("sscanf get more data failed (%s) - there is no more parameter.\n", cmd);
            //return -EFAULT;
        } else {
            SVAD_DBG("input data=%u\n", input_data);
        }
    }

    switch (i) {
    case SVAD_PROC_SETLOGONOFF: {
            svad_log_onoff = (unsigned char)input_data;

            if (input_data == 1) {
                SVAD_WARNING("Enable SVAD log\n");
            } else {
                SVAD_WARNING("Disable SVAD log\n");
            }
            break;
        }
    case SVAD_PROC_SETMODEL01: {
            SVAD_INFO("\033[1;32;32m""Enable MODEL G\n""\033[m");
            g_thread_en = 0;

#ifdef MIO_I2C_MODE
            ret |= rtk_svad_4bytes_i2c_rd(SYS_REG_SYS_SRST_reg, &reg_value_sys_srst);
            ret |= rtk_svad_4bytes_i2c_rd(SYS_REG_SYS_CLKEN_reg, &reg_value_sys_clken);
            if(((reg_value_sys_srst & SYS_REG_SYS_SRST_rstn_sv_mask) != SYS_REG_SYS_SRST_rstn_sv_mask) || ((reg_value_sys_clken & SYS_REG_SYS_CLKEN_clken_sv_mask) != SYS_REG_SYS_CLKEN_clken_sv_mask))
            {
                SVAD_INFO("\033[1;32;33m" "SVAD CRT ON" "\033[m");
                rtk_svad_crt_clk_onoff(CLK_ON);
            }
            ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_contorl_reg, ~SMART_VAD_svad_contorl_top_ctrl_mask, 0x00000000);
#else
            if(((rtd_inl(SYS_REG_SYS_SRST_reg+MIO_SYS_SPI_ADDR_SHIFT) & SYS_REG_SYS_SRST_rstn_sv_mask) != SYS_REG_SYS_SRST_rstn_sv_mask) || ((rtd_inl(SYS_REG_SYS_CLKEN_reg+MIO_SYS_SPI_ADDR_SHIFT) & SYS_REG_SYS_CLKEN_clken_sv_mask) != SYS_REG_SYS_CLKEN_clken_sv_mask))
            {
                SVAD_INFO("\033[1;32;33m" "SVAD CRT ON" "\033[m");
                rtk_svad_crt_clk_onoff(CLK_ON);
            }
            rtd_maskl(SMART_VAD_svad_contorl_reg+MIO_SVAD_SPI_ADDR_SHIFT, ~SMART_VAD_svad_contorl_top_ctrl_mask, 0x00000000);
#endif
            msleep_interruptible(200);
            rtk_svad_model_init();
            g_thread_en = 1;
            break;
        }
    case SVAD_PROC_SETMODEL02: {
            SVAD_INFO("\033[1;32;32m""Enable MODEL L\n""\033[m");
            g_thread_en = 0;

#ifdef MIO_I2C_MODE
            ret |= rtk_svad_4bytes_i2c_rd(SYS_REG_SYS_SRST_reg, &reg_value_sys_srst);
            ret |= rtk_svad_4bytes_i2c_rd(SYS_REG_SYS_CLKEN_reg, &reg_value_sys_clken);
            if(((reg_value_sys_srst & SYS_REG_SYS_SRST_rstn_sv_mask) != SYS_REG_SYS_SRST_rstn_sv_mask) || ((reg_value_sys_clken & SYS_REG_SYS_CLKEN_clken_sv_mask) != SYS_REG_SYS_CLKEN_clken_sv_mask))
            {
                SVAD_INFO("\033[1;32;33m" "SVAD CRT ON" "\033[m");
                rtk_svad_crt_clk_onoff(CLK_ON);
            }
            ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_contorl_reg, ~SMART_VAD_svad_contorl_top_ctrl_mask, 0x00100000);
#else
            if(((rtd_inl(SYS_REG_SYS_SRST_reg+MIO_SYS_SPI_ADDR_SHIFT) & SYS_REG_SYS_SRST_rstn_sv_mask) != SYS_REG_SYS_SRST_rstn_sv_mask) || ((rtd_inl(SYS_REG_SYS_CLKEN_reg+MIO_SYS_SPI_ADDR_SHIFT) & SYS_REG_SYS_CLKEN_clken_sv_mask) != SYS_REG_SYS_CLKEN_clken_sv_mask))
            {
                SVAD_INFO("\033[1;32;33m" "SVAD CRT ON" "\033[m");
                rtk_svad_crt_clk_onoff(CLK_ON);
            }
            rtd_maskl(SMART_VAD_svad_contorl_reg+MIO_SVAD_SPI_ADDR_SHIFT, ~SMART_VAD_svad_contorl_top_ctrl_mask, 0x00100000);
#endif
            msleep_interruptible(200);
            rtk_svad_model_init();
            g_thread_en = 1;
            break;
        }
    default: {
            SVAD_DBG("unsupport proc command=%s\n", str);
            return -EINVAL;
        }
    }

    return count;
}

int rtk_svad_kernelthread(void* arg)
{
#ifdef MIO_I2C_MODE
    int ret = FUNCTION_SUCCESS;
    unsigned int reg_value_sys_srst = 0;
    unsigned int reg_value_sys_clken = 0;
    unsigned int svad_irq_reg = 0;
#endif

    SVAD_DBG("rtk_svad_kernelthread\n");

    while(!kthread_should_stop())
    {
        if(g_thread_en)
        {
            #ifdef MIO_I2C_MODE
            ret |= rtk_svad_4bytes_i2c_rd(SYS_REG_SYS_SRST_reg, &reg_value_sys_srst);
            ret |= rtk_svad_4bytes_i2c_rd(SYS_REG_SYS_CLKEN_reg, &reg_value_sys_clken);
            if(((reg_value_sys_srst & SYS_REG_SYS_SRST_rstn_sv_mask) == SYS_REG_SYS_SRST_rstn_sv_mask) && ((reg_value_sys_clken & SYS_REG_SYS_CLKEN_clken_sv_mask) == SYS_REG_SYS_CLKEN_clken_sv_mask))
            {
                //SVAD_INFO("\033[1;32;34m""rtd_inl(ISO_MISC_IRDA_IR_RP_HIGH_MAIN1_reg)=0x%08x\n""\033[m" , rtd_inl(ISO_MISC_IRDA_IR_RP_HIGH_MAIN1_reg));
                if(rtd_inl(ISO_MISC_IRDA_IR_RP_HIGH_MAIN1_reg) == 0xe31c7f00)//HWD & IR power key
                {
                    rtd_maskl(ISO_MISC_IRDA_IR_CR_MAIN1_reg, ~ISO_MISC_IRDA_IR_CR_MAIN1_main1_en_mask, 0x00000000);//IR key reset
                    msleep_interruptible(50);
                    rtd_maskl(ISO_MISC_IRDA_IR_CR_MAIN1_reg, ~ISO_MISC_IRDA_IR_CR_MAIN1_main1_en_mask, 0x00000001);//IR key reset
                    ret |= rtk_svad_4bytes_i2c_rd(SMART_VAD_svad_irq_reg, &svad_irq_reg);
                    SVAD_INFO("\033[1;32;34m""rtk_svad_4bytes_i2c_rd(SMART_VAD_svad_irq_reg, &svad_irq_reg)=0x%08x\n""\033[m" , svad_irq_reg);
                    if((svad_irq_reg & SMART_VAD_svad_irq_int_svad_kw_mask) == SMART_VAD_svad_irq_int_svad_kw_mask)
                    {
                        msleep_interruptible(2000);
                        ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_irq_reg, 0x00000000);
                    
                        SVAD_INFO("\033[1;32;32m""CLEAR HOT WORD GPIO !!!\n""\033[m");
                        SVAD_INFO("\033[1;32;32m""CLEAR HOT WORD GPIO !!!\n""\033[m");
                        SVAD_INFO("\033[1;32;32m""CLEAR HOT WORD GPIO !!!\n""\033[m");
                    }
                }

            }
            #else
            //SVAD_INFO("\033[1;32;33m""rtd_inl(SYS_REG_SYS_CLKEN_reg+MIO_SYS_SPI_ADDR_SHIFT)=0x%08x\n""\033[m" , rtd_inl(SYS_REG_SYS_CLKEN_reg+MIO_SYS_SPI_ADDR_SHIFT));
            if(((rtd_inl(SYS_REG_SYS_SRST_reg+MIO_SYS_SPI_ADDR_SHIFT) & SYS_REG_SYS_SRST_rstn_sv_mask) == SYS_REG_SYS_SRST_rstn_sv_mask) && ((rtd_inl(SYS_REG_SYS_CLKEN_reg+MIO_SYS_SPI_ADDR_SHIFT) & SYS_REG_SYS_CLKEN_clken_sv_mask) == SYS_REG_SYS_CLKEN_clken_sv_mask))
            {
                //SVAD_INFO("\033[1;32;34m""rtd_inl(ISO_MISC_IRDA_IR_RP_HIGH_MAIN1_reg)=0x%08x\n""\033[m" , rtd_inl(ISO_MISC_IRDA_IR_RP_HIGH_MAIN1_reg));
                if(rtd_inl(ISO_MISC_IRDA_IR_RP_HIGH_MAIN1_reg) == 0xe31c7f00)//HWD & IR power key
                {
                    rtd_maskl(ISO_MISC_IRDA_IR_CR_MAIN1_reg, ~ISO_MISC_IRDA_IR_CR_MAIN1_main1_en_mask, 0x00000000);//IR key reset
                    msleep_interruptible(50);
                    rtd_maskl(ISO_MISC_IRDA_IR_CR_MAIN1_reg, ~ISO_MISC_IRDA_IR_CR_MAIN1_main1_en_mask, 0x00000001);//IR key reset
                    SVAD_INFO("\033[1;32;34m""rtd_inl(SMART_VAD_svad_irq_reg+MIO_SVAD_SPI_ADDR_SHIFT)=0x%08x\n""\033[m" , rtd_inl(SMART_VAD_svad_irq_reg+MIO_SVAD_SPI_ADDR_SHIFT));
                    if((rtd_inl(SMART_VAD_svad_irq_reg+MIO_SVAD_SPI_ADDR_SHIFT) & SMART_VAD_svad_irq_int_svad_kw_mask) == SMART_VAD_svad_irq_int_svad_kw_mask)
                    {
                        msleep_interruptible(2000);
                        rtd_outl(SMART_VAD_svad_irq_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
                    
                        SVAD_INFO("\033[1;32;32m""CLEAR HOT WORD GPIO !!!\n""\033[m");
                        SVAD_INFO("\033[1;32;32m""CLEAR HOT WORD GPIO !!!\n""\033[m");
                        SVAD_INFO("\033[1;32;32m""CLEAR HOT WORD GPIO !!!\n""\033[m");
                    }
                }

            }
            #endif
        }
        msleep_interruptible(50);
    }

    return 0;
}

int rtk_svad_thread_en(unsigned int thread_en)
{
    int ret = 0;
    SVAD_INFO("rtk_svad_thread_en = %u\n", thread_en);

    if (thread_en) {
        svad_kernelthread = kthread_run(rtk_svad_kernelthread, NULL, "svadkernelthread");    /* no need to pass parameter into thread function */
        if (!IS_ERR(svad_kernelthread)) {
        } else {
            up(&rtksvad_Semaphore);
            SVAD_INFO("create rtk_svad_thread_en thread failed\n");
            return -1;
        }
    } else {
        /* it will wait until thread function exit */
        ret = kthread_stop(svad_kernelthread);
        if (ret < 0) {
            SVAD_INFO("call kthread_stop return error = %d", ret);
            up(&rtksvad_Semaphore);
            return -1;
        }
    }

    return 0;
}

long rtk_svad_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int retval = 0;
    RTK_SVAD_GET_HWD_RESULT hwd_result_data;

    SVAD_DBG("\nrtk svad: receive ioctl(cmd:0x%08x, arg:0x%08lx)\n", cmd, arg);

    switch (cmd) {
    case SVAD_IOC_INIT:
        SVAD_INFO("\033[1;32;31m" "rtk svad: SVAD_IOC_INIT" "\033[m");
        rtk_svad_model_init();
        break;

    case SVAD_IOC_HWD_START:
        SVAD_INFO("\033[1;32;31m" "rtk svad: SVAD_IOC_HWD_START" "\033[m");
        rtk_svad_det_start();
        break;

    case SVAD_IOC_HWD_STOP:
        SVAD_INFO("\033[1;32;31m" "rtk svad: SVAD_IOC_HWD_STOP" "\033[m");
        rtk_svad_det_stop();
        break;

    case SVAD_IOC_GET_HWD_RESULT:
        SVAD_INFO("\033[1;32;31m" "rtk svad: SVAD_IOC_GET_HWD_RESULT" "\033[m");
        rtk_svad_get_det_result(&hwd_result_data.hwd_result);

        if(copy_to_user((void __user *)arg, &hwd_result_data, sizeof(RTK_SVAD_GET_HWD_RESULT)))
            retval = -ENOTTY;
        break;

    case SVAD_IOC_RESET_HWD_RESULT:
        SVAD_INFO("\033[1;32;31m" "rtk svad: SVAD_IOC_RESET_HWD_RESULT" "\033[m");
        rtk_svad_reset_det_result();
        break;

    default:
        retval = -ENOTTY;
    }

    return retval;
}

#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
long rtk_svad_ioctl_compat(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *compat_arg = compat_ptr(arg);

    SVAD_DBG("\nrtk svad: receive ioctl(compat)(cmd:0x%08x, arg:0x%08lx)\n",cmd, arg);

    return rtk_svad_ioctl(filp, cmd, (unsigned long)compat_arg);
}
#endif
#endif

#ifdef CONFIG_PM
static int rtk_svad_suspend(struct platform_device *dev, pm_message_t state)
{
    SVAD_INFO("rtk svad: receive supend command\n");
    //CRT_CLK_OnOff(WOV, CLK_OFF, NULL);
    return 0;
}

static int rtk_svad_resume(struct platform_device *dev)
{
    SVAD_INFO("rtk svad: receive resume command\n");
    return 0;
}
#endif

#ifdef CONFIG_HIBERNATION
static int rtk_svad_std_suspend(struct device *dev)
{
    SVAD_DBG("rtk svad: receive std suspend command\n");
    //CRT_CLK_OnOff(WOV, CLK_OFF, NULL);
    return 0;
}

static int rtk_svad_std_resume(struct device *dev)
{
    SVAD_DBG("rtk svad: receive std resume command\n");
    return 0;
}

static int rtk_svad_str_suspend(struct device *dev)
{
    SVAD_DBG("rtk svad: receive str suspend command\n");
    //CRT_CLK_OnOff(WOV, CLK_OFF, NULL);
    return 0;
}


static int rtk_svad_str_resume(struct device *dev)
{
    SVAD_DBG("rtk svad: receive str resume command\n");
    return 0;
}
#endif

static char *rtk_svad_devnode(struct device *dev, umode_t *mode)
{
    //*mode = 0666;
    return NULL;
}

static int rtk_svad_create(void)
{
    int result = 0;
    dev_t dev = 0;
    rtk_svad_class = NULL;
    rtk_svad_platform_devs = NULL;

    SVAD_DBG("rtk svad: SVAD driver for Realtek TV Platform\n");

    if (rtk_svad_major) {
        dev = MKDEV(RTK_SVAD_MAJOR, 0);
        result = register_chrdev_region(dev, 1, "rtksvad");
    } else {
        result = alloc_chrdev_region(&dev, rtk_svad_minor, 1, "rtksvad");
        rtk_svad_major = MAJOR(dev);
    }
    if (result < 0) {
        SVAD_DBG("rtk svad: can not get chrdev region...\n");
        return result;
    }

    rtk_svad_class = class_create(THIS_MODULE, "rtksvad");
    if (IS_ERR(rtk_svad_class)) {
        SVAD_DBG("rtk svad: can not create class...\n");
        result = PTR_ERR(rtk_svad_class);
        goto fail_class_create;
    }

    rtk_svad_class->devnode = rtk_svad_devnode;

    rtk_svad_platform_devs =
        platform_device_register_simple("rtksvad", -1, NULL, 0);

    if (platform_driver_register(&rtk_svad_device_driver) != 0) {
        SVAD_DBG("rtk svad: can not register platform driver...\n");
        result = -EINVAL;
        goto fail_platform_driver_register;
    }

    cdev_init(&rtk_svad_cdev, &rtk_svad_fops);
    rtk_svad_cdev.owner = THIS_MODULE;
    rtk_svad_cdev.ops = &rtk_svad_fops;
    result = cdev_add(&rtk_svad_cdev, dev, 1);
    if (result < 0) {
        SVAD_DBG("rtk svad: can not add character device...\n");
        goto fail_cdev_init;
    }
    device_create(rtk_svad_class, NULL, dev, NULL, "rtksvad");
    
    sema_init(&rtksvad_Semaphore, 1);

    device_enable_async_suspend(&(rtk_svad_platform_devs->dev));
    
#ifdef CONFIG_PROC_FS
    /* add proc entry */
    if ((svad_proc_dir == NULL) && (svad_proc_entry == NULL)) {
        svad_proc_dir = proc_mkdir(SVAD_DEVICE_NAME, NULL);

        if (svad_proc_dir == NULL) {
            SVAD_WARNING("create rhal_tp dir proc entry (%s) failed\n", SVAD_DEVICE_NAME);
        } else {
            svad_proc_entry = proc_create(SVAD_PROC_ENTRY, 0666, svad_proc_dir, &rtk_svad_fops);
            if (svad_proc_entry == NULL) {
                SVAD_WARNING("failed to get proc entry for %s/%s \n", SVAD_DEVICE_NAME, SVAD_PROC_ENTRY);
            }
        }
    }
#else
    SVAD_WARNING("This module requests the kernel to support procfs,need set CONFIG_PROC_FS configure Y\n");
#endif
    
    #ifdef EN_KERNEL_AUTO_INIT_SVAD
    rtk_svad_model_init();
    #endif
    
    return 0;

fail_cdev_init:
    platform_driver_unregister(&rtk_svad_device_driver);
fail_platform_driver_register:
    platform_device_unregister(rtk_svad_platform_devs);
    rtk_svad_platform_devs = NULL;
    class_destroy(rtk_svad_class);
fail_class_create:
    rtk_svad_class = NULL;
    unregister_chrdev_region(dev, 1);
    return result;
}

static void rtk_svad_exit(void)
{
    dev_t dev = MKDEV(rtk_svad_major, rtk_svad_minor);
    
    SVAD_DBG("\033[1;32;33m" "rtk svad: rtk_svad_exit\n" "\033[m");

    if (rtk_svad_platform_devs == NULL)
        BUG();

    device_destroy(rtk_svad_class, dev);
    cdev_del(&rtk_svad_cdev);

    platform_driver_unregister(&rtk_svad_device_driver);
    platform_device_unregister(rtk_svad_platform_devs);
    rtk_svad_platform_devs = NULL;

    class_destroy(rtk_svad_class);
    rtk_svad_class = NULL;

    unregister_chrdev_region(dev, 1);
    SVAD_DBG("%s():unregister chrdev\n", __FUNCTION__);
}

module_init(rtk_svad_create);
module_exit(rtk_svad_exit);
MODULE_LICENSE("GPL");
