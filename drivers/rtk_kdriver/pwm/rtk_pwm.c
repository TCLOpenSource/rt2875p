/******************************************************************************
*
*   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
*
*   @author baker.cheng@realtek.com
*
******************************************************************************/
#ifndef BUILD_QUICK_SHOW
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/pwm.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <mach/rtk_platform.h>
#include <mach/pcbMgr.h>
#include <linux/fs.h> /*file operators*/
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include "rtk_pwm-reg.h"
#ifdef CONFIG_OF
#include <linux/of_irq.h>
#include <linux/of_address.h>
#endif
#else
#include <no_os/export.h>
#include <string.h>
#include <malloc.h>
#include <mach/pcbMgr.h>
#endif

#include "rtk_pwm_crt.h"
#include "rtk_pwm_func.h"
#include "rtk_pwm_local_dimming.h"
#include "rtk_pwm_attr.h"
#include <rtk_kdriver/rtk_pwm.h>

#ifndef BUILD_QUICK_SHOW
DEFINE_MUTEX(rtk_pwm_mutex);

extern int m_track_mode_get(void);

int dev_major;
int dev_minor;
struct cdev *dev_cdevp = NULL;

#ifndef CONFIG_OF
static struct platform_device *rtk_pwm_platform_dev;
#endif

static struct device *pwm_device_st;
static struct class *pwm_class;
#endif
static int pwm_count = 0;
RTK_PCB_ENUM pwm_name_list[MAX_PWM_COUNT];

int rtk_pwm_mode_init(int pwm_mode);
int rtk_pwm_set_freq(R_CHIP_T *pchip2, int h_total, int v_total, int ifold, bool bEnableDB);
int rtk_pwm_get_freq(R_CHIP_T *pchip2,unsigned short M, unsigned short N);

static
int rtk_pwm_set_mode(RTK_PWM_MODE_INFO_T mode_param)
{
    int ret = PWM_OK;
    R_CHIP_T *pchip2 = rtk_pwm_chip_get_by_name(mode_param.pcbname);
    if(pchip2 == NULL){
        ret = (-PWM_PCB_ENUM_ERROR);
        PWM_ERR("PWM %s chip get error!\n",mode_param.pcbname);
        return ret;
    }

    pchip2->currentMode = mode_param.m_mode;

    if(mode_param.m_mode == GPIO_MODE){

        if(mode_param.m_val == 1){
            rtk_change_to_gpio_mode(pchip2,GPIO_PIN_HIGH);
        }
        else if(mode_param.m_val == 0){
            rtk_change_to_gpio_mode(pchip2,GPIO_PIN_LOW);
        }
        else{
            PWM_ERR("SET %s  MODE VALUE INVALID mode_val:%d\n",pchip2->nodename,mode_param.m_val);
            return (-PWM_VALUE_INVALID);
        }
    }
    else if(mode_param.m_mode == PWM_MODE){
        rtk_change_to_pwm_mode(pchip2);
    }
    else {
        PWM_ERR("SET %s  MODE INVALID mode_val:%d\n",pchip2->nodename,mode_param.m_mode);
        return (-PWM_MODE_INVALID);
    }
    return ret;
}

static
int rtk_pwm_get_mode(RTK_PWM_MODE_INFO_T* mode_param)
{
    int ret = PWM_OK;
    R_CHIP_T *pchip2 = rtk_pwm_chip_get_by_name(mode_param->pcbname);
    if(pchip2 == NULL){
        ret = (-PWM_PCB_ENUM_ERROR);
        PWM_ERR("PWM %s chip get error!\n",mode_param->pcbname);
        return ret;
    }

    mode_param->m_mode = pchip2->currentMode;

    if(mode_param->m_mode == GPIO_MODE)
        mode_param->m_val = rtk_pwm_gpio_value(pchip2);
    else  if(mode_param->m_mode == PWM_MODE){
        mode_param->m_val = 0;
    }
    else {
        PWM_ERR("SET %s  MODE INVALID mode_val:%d\n",pchip2->nodename,mode_param->m_mode);
        return (-PWM_MODE_INVALID);
    }
    return ret;
}


int rtk_pwm_set_param(RTK_PWM_PARAM_EX_T* param)
{
    int ret = PWM_SUCCESS;
    R_CHIP_T *pchip2 = rtk_pwm_chip_get_by_name(param->pcbname);
    if(pchip2 == NULL){
        ret = (-PWM_PCB_ENUM_ERROR);
        PWM_ERR("PWM %s chip get error!\n",param->pcbname);
        return ret;
    }

    return rtk_pwm_set_param_ex(pchip2,param);

}
EXPORT_SYMBOL(rtk_pwm_set_param);

int rtk_pwm_get_param(RTK_PWM_PARAM_EX_T* param)
{
    int ret = PWM_SUCCESS;
    R_CHIP_T *pchip2 = rtk_pwm_chip_get_by_name(param->pcbname);
    if(pchip2 == NULL){
        ret = (-PWM_PCB_ENUM_ERROR);
        PWM_ERR("PWM %s chip get error!\n",param->pcbname);
        return ret;
    }

    return rtk_pwm_get_param_ex(pchip2,param);

}
EXPORT_SYMBOL(rtk_pwm_get_param);

static
int rtk_pwm_set_all_param(RTK_PWM_INFO_EX_T* param_all)
{
    int ret = PWM_OK;
    R_CHIP_T *pchip2 = rtk_pwm_chip_get_by_name(param_all->pcbname);
    if(pchip2 == NULL){
        ret = (-PWM_PCB_ENUM_ERROR);
        PWM_ERR("PWM %s chip get error!\n",param_all->pcbname);
        return ret;
    }
    ret = rtk_pwm_set_all_param_ex(pchip2,param_all);
    if(ret == PWM_OK){
        if( m_ioctl_printk_get() > 0 ){
            PWM_ERR("SET %s POLARITY m_polarity:%d\n",pchip2->nodename,param_all->m_polarity);
            PWM_ERR("SET %s FREQUENCY m_freq:%d\n",pchip2->nodename,param_all->m_freq);
            PWM_ERR("SET %s DUTY m_duty:%d\n",pchip2->nodename,param_all->m_duty);
            PWM_ERR("SET %s VSYNC m_vsync:%d\n",pchip2->nodename,param_all->m_vsync);
            PWM_ERR("SET %s OUTPUT m_run:%d\n",pchip2->nodename,param_all->m_run);
        }
    }
    return ret;
}

static
int rtk_pwm_get_all_param(RTK_PWM_INFO_EX_T* param_all)
{
    int ret = PWM_OK;
    R_CHIP_T *pchip2 = rtk_pwm_chip_get_by_name(param_all->pcbname);
    if(pchip2 == NULL){
        ret = (-PWM_PCB_ENUM_ERROR);
        PWM_ERR("PWM %s chip get error!\n",param_all->pcbname);
        return ret;
    }
    ret = rtk_pwm_get_all_param_ex(pchip2,param_all);
    if(ret == PWM_OK){
        if( m_ioctl_printk_get() > 0 ){
            PWM_WARN("GET %s POLARITY m_polarity:%d\n",pchip2->nodename,param_all->m_polarity);
            PWM_WARN("GET %s FREQUENCY m_freq:%d\n",pchip2->nodename,param_all->m_freq);
            PWM_WARN("GET %s DUTY m_duty:%d\n",pchip2->nodename,param_all->m_duty);
            PWM_WARN("GET %s VSYNC m_vsync:%d\n",pchip2->nodename,param_all->m_vsync);
            PWM_WARN("GET %s OUTPUT m_run:%d\n",pchip2->nodename,param_all->m_run);
        }
    }
    return ret;
}

int pcb_mgr_get_enum_info_represent_byapp(RTK_PCB_ENUM *pcbenumPtr)
{
    int i;
    int index = 0;
    PCB_PIN_TYPE_T     pin_type;

    struct linux_pcb_pwm_enum_t  *linux_pcb_enum_ptr = (struct linux_pcb_pwm_enum_t* )pcbenumPtr;

    if(linux_pcb_enum_ptr == NULL){
        return -1;
    }

    for(i = 0; i < PCB_ENUM_MAX; i++){
        if(memcmp(pcb_enum_all[i].name, "PIN", 3) == 0){
            pin_type = GET_PIN_TYPE(pcb_enum_all[i].value);
            //PWM_WARN("pin type :%d, pin pcb_enum_all[i].name:%s\n", pin_type, pcb_enum_all[i].name);
            if(PCB_PIN_TYPE_PWM == pin_type  || PCB_PIN_TYPE_ISO_PWM == pin_type){
                strncpy(linux_pcb_enum_ptr[index].name,pcb_enum_all[i].name,sizeof(pcb_enum_all[i].name)-1);
                linux_pcb_enum_ptr[index].value = pcb_enum_all[i].value;
                index ++;
            }
        }
    }

    return index;
}

#ifndef BUILD_QUICK_SHOW
static int rtk_hw_setting_pwm_init(RTK_PCB_ENUM *pcbinfo, int ricindex)
{
    int ret = 0;
    R_CHIP_T *pwmPin=NULL;
    char *pcbname = pcbinfo->name;

    ret = rtk_pwm_pcb_mgr_get(pcbname,ricindex);

    if( ret != 0)
        return -1;

    rtk_pwm_pcb_mgr_get_value(pcbinfo,ricindex);

    if( strcmp(pcbname , "PIN_BL_ADJ") == 0 ||
        strcmp(pcbname , "PIN_PWM_DIM1") == 0){
        rtk_pwm_backlight_pin_set(ricindex);
    }

    pwmPin = rtk_pwm_chip_get(ricindex);
    strncpy(pwmPin->pcbname,pcbname,sizeof(pwmPin->pcbname)-1);

    /** workaround for full high & full low support **/
    rtk_pwm_gpio_pinmux_init(pwmPin);

    rtk_pwm_iso_interrupt_ctrl_enable_w(0);

    if(rtk_pwm_hw_type_init(pwmPin) < 0)
        return (-1);


#ifdef CONFIG_RTK_KDRV_PWM_INIT_ENABLE
    rtk_hw_setting_pwm_probe(ricindex,
        GET_PIN_TYPE(pcbinfo->value),
        GET_PIN_INDEX(pcbinfo->value),
        GET_PWM_FREQ(pcbinfo->value)*100,
        GET_PWM_DUTY_MAX(pcbinfo->value),
        GET_PWM_INIT_DUTY(pcbinfo->value),
        GET_PWM_INVERT(pcbinfo->value),
        rtk_pwm_chip_index_get(ricindex));
#endif

    pwmPin->pwm_device_st = device_create(pwm_class, NULL, MKDEV(0, 0), NULL,
                  "%s",pwmPin->nodename);

    rtk_pwm_create_attr(pwmPin->pwm_device_st);

    pwmPin->pwm_device_st->platform_data = (void *)pwmPin;

    init_waitqueue_head(&pwmPin->wq);

    spin_lock_init(&pwmPin->lock);

    rtk_pwm_hw_chip_init(pwmPin,ricindex);

    PWM_WARN("[%s] Get %s, Create Node[%d] %s, device_id=%d, VT=%d, HT=%d, clk=%lld, adapt_freq(base 0.01Hz)=%d\n",
        __func__,
        pcbname,
        ricindex,
        pwmPin->nodename,
        pwmPin->pwm_device_st->id,
        pwmPin->vt,
        pwmPin->ht,
        pwmPin->dclk,
        pwmPin->rtk_adapt_freq);

    PWM_WARN("[%s] freq=%d, max_duty=%d, duty=%d, vsync=%d, enable=%d, polarity=%d, pos=%d, pos_delay=%d\n",
        __func__,
        pwmPin->rtk_freq,
        pwmPin->rtk_duty_max,
        pwmPin->rtk_duty,
        pwmPin->rtk_vsync,
        pwmPin->rtk_enable,
        pwmPin->rtk_polarity,
        pwmPin->rtk_pos_start,
        pwmPin->rtk_pos_start_clk);

    return 0;

}

int rtk_pwm_open(struct inode *inode, struct file *filp)
{    return 0;    }

ssize_t rtk_pwm_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    PWM_WARN("%s():\n", __FUNCTION__);
    return -EFAULT;
}
ssize_t rtk_pwm_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    PWM_WARN("%s():\n", __FUNCTION__);
    return -EFAULT;
}

long rtk_pwm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int retval = 0;
    int len = 0;
    RTK_PWM_INFO_T stpwminfo;
    RTK_PWM_PARAM_EX_T param;
    RTK_PWM_INFO_EX_T  param_all;
    RTK_PWM_MODE_INFO_T mode_info;

    memset(&stpwminfo, 0, sizeof(RTK_PWM_INFO_T));
    memset(&param,     0, sizeof(RTK_PWM_PARAM_EX_T));
    memset(&param_all, 0, sizeof(RTK_PWM_INFO_EX_T));
    memset(&mode_info, 0, sizeof(RTK_PWM_MODE_INFO_T));

    if(m_ioctl_disable_get())
        return retval;

    m_ioctl_counter_inc();

    switch (cmd) {
    case PWM_MODE_SET:
        len  = sizeof(mode_info.pcbname);
        if (copy_from_user(&mode_info, (RTK_PWM_MODE_INFO_T __user *) arg, sizeof(mode_info)))
            return -EFAULT;

        mode_info.pcbname[len -1] = 0;

        retval = rtk_pwm_set_mode(mode_info);
        if(retval < 0){
            PWM_ERR("pwm set mode_info failed \n");
            return retval;
        }
        break;
    case PWM_MODE_GET:
        len  = sizeof(mode_info.pcbname);
        if (copy_from_user(&mode_info, (RTK_PWM_MODE_INFO_T __user *) arg, sizeof(mode_info)))
            return -EFAULT;

        mode_info.pcbname[len-1] = 0;

        retval = rtk_pwm_get_mode(&mode_info);
        if (retval < 0){
            PWM_ERR("pwm get mode_info failed \n");
            return retval;
        }
        return copy_to_user((RTK_PWM_MODE_INFO_T __user *) arg, &mode_info,sizeof(mode_info));
        break;
    case PWM_PARAM_SET_EX:
        len  = sizeof(param.pcbname);
        if (copy_from_user(&param, (RTK_PWM_PARAM_EX_T __user *) arg, sizeof(param)))
            return -EFAULT;
        param.pcbname[len-1] = 0;
        retval = rtk_pwm_set_param(&param);
        if(retval < 0){
            PWM_ERR("pwm set param failed \n");
            return retval;
        }
        break;
    case PWM_PARAM_GET_EX:
        len  = sizeof(param.pcbname);
        if (copy_from_user(&param, (RTK_PWM_PARAM_EX_T __user *) arg, sizeof(param)))
            return -EFAULT;

        param.pcbname[len-1] = 0;

        retval = rtk_pwm_get_param(&param);
        if (retval < 0){
            PWM_ERR("pwm get param failed \n");
            return retval;
        }
        return copy_to_user((RTK_PWM_PARAM_EX_T __user *) arg, &param,sizeof(param));
        break;
    case PWM_IOCR_SET_EX:
        len  = sizeof(param_all.pcbname);
        if (copy_from_user(&param_all, (RTK_PWM_INFO_EX_T __user *) arg, sizeof(param_all)))
            return -EFAULT;
        param_all.pcbname[len-1] = 0;
        retval = rtk_pwm_set_all_param(&param_all);
        if(retval < 0){
            PWM_ERR("pwm set param_all failed \n");
            return retval;
        }
        break;
    case PWM_IOCR_GET_EX:
        len  = sizeof(param_all.pcbname);
        if (copy_from_user(&param_all, (RTK_PWM_INFO_EX_T __user *) arg, sizeof(param_all)))
            return -EFAULT;

        param_all.pcbname[len-1] = 0;

        retval = rtk_pwm_get_all_param(&param_all);
        if (retval < 0){
            PWM_ERR("pwm get param_all failed \n");
            return retval;
        }
        return copy_to_user((RTK_PWM_INFO_EX_T __user *) arg, &param_all,sizeof(param_all));
        break;

    case PWM_BACKLIGHT_DUTY:
        rtk_pwm_backlight_set_duty(arg);
        break;

    case PWM_IOCR_SET:
        if (copy_from_user(&stpwminfo, (struct RTK_PWM_INFO __user *)arg, sizeof(struct RTK_PWM_INFO))) {
            retval = -EFAULT;
            PWM_ERR("PWM_IOCR_ fail!\n");
            break;
        }
        if(stpwminfo.m_index < 0 || (stpwminfo.m_index >= pin_index_get())) {
            retval = -EFAULT;
            PWM_ERR("PWM index-%d error!\n",stpwminfo.m_index);
            break;
        }
        rtk_pwm_set_info(&stpwminfo);
        break;

    default:
        retval = -ENOTTY;
    }

    return retval;
}

int rtk_pwm_release(struct inode *inode, struct file *filp)
{    return 0;    }

#ifdef CONFIG_COMPAT
long rtk_pwm_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{    return rtk_pwm_ioctl(filp, cmd, (unsigned long)arg);    }
#endif

static struct file_operations rtk_pwm_fops = {
    .owner = THIS_MODULE,
    .read = rtk_pwm_read,
    .write = rtk_pwm_write,
    .unlocked_ioctl = rtk_pwm_ioctl,
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
    .compat_ioctl = rtk_pwm_compat_ioctl,
#endif
#endif
    .release = rtk_pwm_release,
    .open = rtk_pwm_open,
};


static void rtk_hw_setting_pwm_probe(int ricindex,
                     PCB_PIN_TYPE_T pintype, int index,
                     int pwm_freq_hz, int duty_max, int duty,
                     int invert, int memindex)
{

    int rtk_totalcnt = 0;
    R_CHIP_T *pchip2 = rtk_pwm_chip_get(ricindex);

    PWM_INFO("[%s] ricindex=%d, index=%d, pwm_freq_hz=%d, duty_max=%d, duty=%d, invert=%d\n",
             __func__, ricindex, index, pwm_freq_hz, duty_max, duty, invert);

    rtk_pwm_timing_probe_init(pchip2,pwm_freq_hz,duty_max,&rtk_totalcnt);
    rtk_pwm_duty_probe_init(pchip2,duty,duty_max,rtk_totalcnt);
    rtk_pwm_ctrl_probe_init(pchip2,rtk_pwm_panel_mode_get(),invert);
    rtk_pwm_db_probe_init(pchip2);
}


static void rtk_hw_setting_pwm_exit(RTK_PCB_ENUM *pcbinfo, int ricindex)
{
    char *pcbname = pcbinfo->name;

    rtk_pwm_pcb_mgr_get(pcbname,ricindex);
}

static int rtk_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwm)
{
    PWM_INFO("[%s] id = %d\n", __func__, chip->npwm);
    return 0;
}

static void rtk_pwm_disable(struct pwm_chip *chip, struct pwm_device *pwm)
{
    PWM_INFO("[%s] id = %d\n", __func__, chip->npwm);
    return;
}

static int rtk_pwm_config(struct pwm_chip *chip, struct pwm_device *pwm,
              int duty_ns, int period_ns)
{

    PWM_INFO(
        "[%s] id=%d ,duty_ns= %d, period_ns=%d\n",__func__, chip->npwm,
           duty_ns, period_ns);

    return 0;
}

static struct pwm_ops rtk_pwm_ops = {
    .enable = rtk_pwm_enable,
    .disable = rtk_pwm_disable,
    .config = rtk_pwm_config,
    .owner = THIS_MODULE,
};

static int rtk_pwm_probe(struct platform_device *pdev)
{
    struct pwm_chip *pstrchip;
    unsigned int id = pdev->id;
    R_CHIP_T *pwmPin = NULL;
    int ret = 0;
    int i = 0;
    int pwm_info;

    PWM_WARN("[%s]Initialize PWM-%d, name = %s\n",__func__, id, pdev->name);

    ret = of_property_read_u32_array(pdev->dev.of_node, "pwm_info", &pwm_info, 1);
    PWM_WARN("[%s] line: %d, pwm_mode: %d\n", __func__, __LINE__, pwm_info);
    rtk_pwm_mode_init(pwm_info);

    pstrchip = devm_kzalloc(&pdev->dev, sizeof(struct pwm_chip), GFP_KERNEL);
    if (pstrchip == NULL) {
        dev_err(&pdev->dev, "[%s] failed to allocate pwm_device\n",
            __func__);
        return -ENOMEM;
    }

    pstrchip->dev = &pdev->dev;
    pstrchip->ops = &rtk_pwm_ops;
    pstrchip->base = 1;
    pstrchip->npwm = 1;

    PWM_WARN("[%s]%d dev name = %s, id = %d\n", __func__, __LINE__,
           pdev->dev.init_name, id);

    pwmchip_add(pstrchip);

    platform_set_drvdata(pdev, pstrchip);

    //rtk_pwm_chip_device_set(pdev);

    for (i = 0; i < pwm_count && i < MAX_PWM_COUNT; i++)
    {
        pwmPin = NULL;
        pwmPin = rtk_pwm_chip_get(i);
        if (NULL == pwmPin)
        {
            PWM_ERR("[%s] line: %d, rtk_pwm_chip_get(%d) ERR\n", __func__, __LINE__, i);
            return 0;
        }

        pwmPin->irq = platform_get_irq(pdev, 0);
        if (pwmPin->irq < 0)
        {
            return pwmPin->irq;
        }
    }
    return 0;
}

static int rtk_pwm_remove(struct platform_device *pdev)
{
    unsigned int id = pdev->id;
    R_CHIP_T *pwmPin;

    pwmPin = rtk_pwm_chip_get(id);

    if (pwmPin->irq >= 0)
        free_irq(pwmPin->irq, pwmPin);

    rtk_pwm_setting_power_off(CRT_PWM_MISC);

    return 0;
}

#ifdef CONFIG_PM
unsigned char pwm_resume_done = 1;            // TRUE: resume  complete / FALSE: suspend complete

static int rtk_pwm_suspend(struct device *dev)
{
    PWM_WARN("suspend\n");
    pwm_resume_done = 0;
    rtk_pwm_setting_power_off(CRT_PWM_MISC);

    return 0;
}

static int rtk_pwm_resume(struct device *dev)
{
    int ricindex=0;

    unsigned long long ullPcbMgrValue;

    PWM_WARN("resume\n");
    rtk_pwm_setting_power_on(CRT_PWM_ISO);
    rtk_pwm_setting_power_on(CRT_PWM_MISC);

    rtk_pwm_iso_interrupt_ctrl_enable_w(0);

    for (ricindex = 0; ricindex < pin_index_get(); ricindex++) {

        ullPcbMgrValue = rtk_pwm_chip_pcb_mgr_value_get(ricindex);
        rtk_hw_setting_pwm_probe(ricindex,
                                GET_PIN_TYPE(ullPcbMgrValue),
                                GET_PIN_INDEX(ullPcbMgrValue),
                                GET_PWM_FREQ(ullPcbMgrValue),
                                GET_PWM_DUTY_MAX(ullPcbMgrValue),


#ifdef CONFIG_CUSTOMER_TV006
                    0,   /* control by webos */
#else
                    GET_PWM_INIT_DUTY(ullPcbMgrValue),
#endif

                     GET_PWM_INVERT(ullPcbMgrValue),
                     rtk_pwm_chip_index_get(ricindex));
    }
    rtk_local_dimming_enable();
    pwm_resume_done = 1;
    return 0;
}

#ifdef CONFIG_HIBERNATION
static int rtk_pwm_suspend_std(struct device *dev)
{
    PWM_WARN("rtk_pwm_suspend_std\n");

    return 0;
}
static int rtk_pwm_resume_std(struct device *dev)
{
    PWM_WARN("rtk_pwm_resume_std\n");
    return 0;
}

#endif

static const struct dev_pm_ops rtk_pwm_pm_ops = {

    .suspend = rtk_pwm_suspend,
    .resume = rtk_pwm_resume,

#ifdef CONFIG_HIBERNATION
    .freeze     = rtk_pwm_suspend_std,
    .thaw       = rtk_pwm_resume_std,
#endif
};


#endif

static const struct of_device_id rtk_pwm_devices[] = {
    {.compatible = "realtek, rtk-pwm",},
    {},
};

MODULE_DEVICE_TABLE(of, rtk_pwm_devices);

static struct platform_driver rtk_pwm_driver = {
    .driver = {
            .name = "rtk-pwm",
            .owner = THIS_MODULE,
            .of_match_table = of_match_ptr(rtk_pwm_devices),

            #ifdef CONFIG_PM
            .pm    = &rtk_pwm_pm_ops,
            #endif
        },
    .probe = rtk_pwm_probe,
    .remove = rtk_pwm_remove,
};

static int register_pwm_cdev(void) {

    dev_t dev;
    int ret;

    ret = alloc_chrdev_region(&dev, 0, 1, "pwm");
    if (ret) {
        PWM_ERR("can't alloc chrdev\n");
        return ret;
    }

    dev_major = MAJOR(dev);
    dev_minor = MINOR(dev);

    dev_cdevp = kzalloc(sizeof(struct cdev), GFP_KERNEL);
    if (dev_cdevp == NULL) {
        PWM_ERR( "kzalloc failed\n");
        goto failed;
    }

    cdev_init(dev_cdevp, &rtk_pwm_fops);
    dev_cdevp->owner = THIS_MODULE;
    dev_cdevp->ops = &rtk_pwm_fops;
    ret = cdev_add(dev_cdevp, MKDEV(dev_major, dev_minor), 1);
    if (ret < 0) {
        PWM_WARN("add chr dev failed\n");
        goto failed;
    }

    pwm_device_st = device_create(pwm_class, NULL, MKDEV(dev_major, dev_minor), NULL, "pwm0");
    PWM_WARN("register chrdev(%d,%d) success.\n", dev_major, dev_minor);
    return 0;

failed:
    if(dev_cdevp) {
        kfree(dev_cdevp);
        dev_cdevp = NULL;
    }
    return 0;

}

int rtk_pwm_mode_init(int pwm_mode)
{
    rtk_pwm_panel_mode_set(pwm_mode);
    if( m_ioctl_printk_get() > 0 ){
        if(rtk_pwm_panel_mode_get() == PWM_DRIVING_LED_CURRENT)
            PWM_WARN("%s  pwm_mode:%s\n",__func__,"LED_CURRENT");
        else if(rtk_pwm_panel_mode_get() == PWM_DRIVING_2CH_PHASE_SAME)
            PWM_WARN("%s  pwm_mode:%s\n",__func__,"2CH_PHASE_SAME");
        else if(rtk_pwm_panel_mode_get() == PWM_DRIVING_2CH_PHASE_DIFF)
            PWM_WARN("%s  pwm_mode:%s\n",__func__,"2CH_PHASE_DIFF");
    }
    return 0;
}

static int __init rtk_pwm_driver_init(void)
{
    int ret = 0;
    int i;

    memset(pwm_name_list,0,sizeof(RTK_PCB_ENUM)*MAX_PWM_COUNT);
    pwm_count = pcb_mgr_get_enum_info_represent_byapp(pwm_name_list);
    if(pwm_count == 0)
        return -1;

    ret = platform_driver_register(&rtk_pwm_driver);
    if (ret != 0)
        goto REGISTER_ERROR;

    pwm_class = class_create(THIS_MODULE, "rtk-pwm");
    if (pwm_class == NULL)
        goto REGISTER_ERROR;

    /*use global index....don't care re-entry safety. */
    for(i = 0; i < pwm_count && i < MAX_PWM_COUNT; i++){
        ret = rtk_hw_setting_pwm_init(&pwm_name_list[i], pin_index_get());
        if (ret == 0) {
            pin_index_count();
        }
    }

#ifndef CONFIG_OF
    rtk_pwm_platform_dev = platform_device_register_simple("rtk-pwm", -1, NULL, 0);
    if (rtk_pwm_platform_dev == NULL) {
        platform_driver_unregister(&rtk_pwm_driver);
        goto REGISTER_ERROR;
    }
#endif

    register_pwm_cdev();

    return 0;

REGISTER_ERROR:
    class_destroy(pwm_class);
    device_destroy(pwm_class, MKDEV(0, 0));

    PWM_ERR("[%s] ======================================\n",__func__);
    PWM_ERR( "[%s] PWM Register Error!\n", __func__);
    PWM_ERR( "[%s] ======================================\n",__func__);
    return -1;
}

module_init(rtk_pwm_driver_init);

static void __exit rtk_pwm_driver_exit(void)
{
    int i;
    struct device *pwm_device_st = NULL;

    for (i = 0; i < pin_index_get(); i++) {
        rtk_pwm_remove_attr(pwm_device_st);
        rtk_hw_setting_pwm_exit(&pwm_name_list[i], i);
    }

    device_destroy(pwm_class, MKDEV(0, 0));

    class_destroy(pwm_class);

    platform_driver_unregister(&rtk_pwm_driver);

#ifndef CONFIG_OF
    platform_device_unregister(rtk_pwm_platform_dev);
#endif
}

module_exit(rtk_pwm_driver_exit);
MODULE_AUTHOR("baker.cheng <baker.cheng@realtek.com>");
MODULE_DESCRIPTION("Realtek Pulse Width Modulation Driver");
MODULE_ALIAS("platform:rtk-pwm");
MODULE_LICENSE("GPL");
#else
static int rtk_hw_setting_pwm_init(RTK_PCB_ENUM *pcbinfo, int ricindex)
{
    int ret = 0;
    R_CHIP_T *pwmPin=NULL;
    char *pcbname = pcbinfo->name;

    ret = rtk_pwm_pcb_mgr_get(pcbname,ricindex);

    if( ret != 0)
        return -1;

    rtk_pwm_pcb_mgr_get_value(pcbinfo,ricindex);

    if( strcmp(pcbname , "PIN_BL_ADJ") == 0 ||
        strcmp(pcbname , "PIN_PWM_DIM1") == 0){
        rtk_pwm_backlight_pin_set(ricindex);
    }

    pwmPin = rtk_pwm_chip_get(ricindex);
    strncpy(pwmPin->pcbname,pcbname,sizeof(pwmPin->pcbname)-1);

    /** workaround for full high & full low support **/
    rtk_pwm_gpio_pinmux_init(pwmPin);

    rtk_pwm_iso_interrupt_ctrl_enable_w(0);

    if(rtk_pwm_hw_type_init(pwmPin) < 0)
        return (-1);


#ifdef CONFIG_RTK_KDRV_PWM_INIT_ENABLE
    rtk_hw_setting_pwm_probe(ricindex,
        GET_PIN_TYPE(pcbinfo->value),
        GET_PIN_INDEX(pcbinfo->value),
        GET_PWM_FREQ(pcbinfo->value)*100,
        GET_PWM_DUTY_MAX(pcbinfo->value),
        GET_PWM_INIT_DUTY(pcbinfo->value),
        GET_PWM_INVERT(pcbinfo->value),
        rtk_pwm_chip_index_get(ricindex));
#endif

    rtk_pwm_hw_chip_init(pwmPin,ricindex);

    PWM_WARN("[%s] Get %s, Create Node[%d] %s, VT=%d, HT=%d, clk=%lld, adapt_freq(base 0.01Hz)=%d\n",
        __func__,
        pcbname,
        ricindex,
        pwmPin->nodename,
        pwmPin->vt,
        pwmPin->ht,
        pwmPin->dclk,
        pwmPin->rtk_adapt_freq);

    PWM_WARN("[%s] freq=%d, max_duty=%d, duty=%d, vsync=%d, enable=%d, polarity=%d, pos=%d, pos_delay=%d\n",
        __func__,
        pwmPin->rtk_freq,
        pwmPin->rtk_duty_max,
        pwmPin->rtk_duty,
        pwmPin->rtk_vsync,
        pwmPin->rtk_enable,
        pwmPin->rtk_polarity,
        pwmPin->rtk_pos_start,
        pwmPin->rtk_pos_start_clk);

    return 0;

}

int pwm_init(int type,int index,int pwm_freq_hz, int duty_max, int duty, int clk_src, int invert)
{
    R_CHIP_T *pchip2 = NULL;

    pchip2 = rtk_pwm_chip_get_by_index(index, type);
    if (pchip2 == NULL)
        return -1;

    pchip2->rtk_clk_sel = clk_src;
    rtk_pwm_clock_source(pchip2);

    pchip2->rtk_polarity = invert;
    rtk_pwm_ctrl_polarity_w(pchip2,pchip2->rtk_polarity);

    pchip2->rtk_freq = pwm_freq_hz;
    pchip2->rtk_freq_100times = pchip2->rtk_freq * 100;
    rtk_pwm_freq_w(pchip2);

    pchip2->rtk_duty = duty;
    pchip2->rtk_duty_max = duty_max;
    rtk_pwm_duty_w(pchip2,pchip2->rtk_duty);

    return 0;
}

int set_pwm_duty(int type, int index, int duty)
{
    R_CHIP_T *pchip2 = NULL;

    pchip2 = rtk_pwm_chip_get_by_index(index, type);
    if (pchip2 == NULL)
        return -1;

    pchip2->rtk_duty = duty;
    rtk_pwm_duty_w(pchip2,pchip2->rtk_duty);

    return 0;
}

int rtk_pwm_driver_init(void)
{
    int ret = 0;
    int i;

    memset(pwm_name_list,0,sizeof(RTK_PCB_ENUM)*MAX_PWM_COUNT);
    pwm_count = pcb_mgr_get_enum_info_represent_byapp(pwm_name_list);
    if(pwm_count == 0)
        return -1;

    /*use global index....don't care re-entry safety. */
    for(i = 0; i < pwm_count && i < MAX_PWM_COUNT; i++){
        ret = rtk_hw_setting_pwm_init(&pwm_name_list[i], pin_index_get());
        if (ret == 0) {
            pin_index_count();
        }
    }
    return 0;
}

#endif

int rtk_pwm_set_freq(R_CHIP_T *pchip2, int h_total, int v_total, int ifold,bool bEnableDB);
int rtk_pwm_set_scaler_source( int h_total, int v_total,int ifold,unsigned char bEnableDB)
{
    R_CHIP_T *pchip2 = NULL;
    unsigned long flag;
    int i = 0;

    if( m_ioctl_printk_get() > 0 ){
        PWM_WARN("rtk_pwm_set_scaler_source (%d) \n", v_total);
    }

    for (i = 0; i < pin_index_get(); i++) {
        pchip2 = rtk_pwm_chip_get(i);
        if (unlikely(!pchip2))
            continue;

        if(pchip2->rtk_clk_sel != D_CLK_DIV4)
            continue;

        spin_lock_irqsave(&pchip2->lock, flag);

        rtk_pwm_set_freq(pchip2, h_total, v_total, ifold, bEnableDB);

        spin_unlock_irqrestore(&pchip2->lock, flag);
    }

    return 0;
}
EXPORT_SYMBOL(rtk_pwm_set_scaler_source);

int rtk_pwm_set_tracking(void)
{
/*
    int i = 0;
    for (i = 0; i < pin_index_get(); i++) {
        pwm_hw_tracking_init(i);
        if(pwm_hw_tracking_start(i) < 0)
            continue;
    }
*/
    return 0;
}
EXPORT_SYMBOL(rtk_pwm_set_tracking);

