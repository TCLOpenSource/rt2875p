#ifndef BUILD_QUICK_SHOW
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/pm_runtime.h>
#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
#include <mach/pcbMgr.h>
#endif
#include <linux/version.h>
#else
#include <sysdefs.h>
#include <malloc.h>
#include <timer.h>
#include <string.h>
#include <vsprintf.h>
#include <rtk_kdriver/pcbMgr.h>
#endif
#include <mach/platform.h>
#include <mach/rtk_platform.h>
#include <rtk_kdriver/rtk_gpio.h>
#include "rtk_amp_interface.h"
#include <rtk_kdriver/quick_show/quick_show.h>

#define MULTI_AMP_CNT 2
#define AMP_MAJOR  0
#define DRV_NAME   "RTKamp"

static struct amp_controller rtk_amp[MULTI_AMP_CNT];
static const char  drv_name[] = DRV_NAME;

static unsigned int amp_boot_sel[MULTI_AMP_CNT] = {0};
static unsigned int amp_boot_addr[MULTI_AMP_CNT] = {0};
AMP_DEVICE_ID_E amp_sel[MULTI_AMP_CNT] = {
    TI_TAS5707,
    TI_TAS5707
};
/*
static char *p_amp_dev_addr[MULTI_AMP_CNT] = {
    "AMP_DEVICE_ADDR",
    "AMP_DEVICE_ADDR"
};*/

#ifndef BUILD_QUICK_SHOW
static struct workqueue_struct *ampWq = NULL;
static struct delayed_work amp_delayWork;

extern platform_info_t platform_info;

MODULE_LICENSE("Dual BSD/GPL");

#ifdef CONFIG_PM
static int amp_suspend(struct device *dev);
static int amp_resume(struct device *dev);
static int rtk_amp_probe(struct platform_device *pdev);
int amp_pm_runtime_suspend(struct device *dev);
int amp_pm_runtime_resume(struct device *dev);

static struct platform_device *amp_devs;

static const struct dev_pm_ops amp_pm_ops = {
        .suspend    = amp_suspend,
        .resume     = amp_resume,
        .runtime_suspend    = amp_pm_runtime_suspend,
        .runtime_resume    = amp_pm_runtime_resume,
};
#endif

static struct platform_driver amp_driver = {
        .driver = {
                .name         = (char *)drv_name,
                .bus          = &platform_bus_type,
#ifdef CONFIG_PM
                .pm           = &amp_pm_ops,
#endif
        },
        .probe = rtk_amp_probe,
};

#else

#define __init
#define early_param(x, x1)
extern int sscanf(const char *buf, const char *fmt, ...);
extern char *getenv(const char *name);
extern char * strsep(char **,const char *);

#endif

void rtk_amp_ops_set(int amp_num, int sel)
{
    memset(rtk_amp[amp_num].name, 0, sizeof(rtk_amp[amp_num].name));
    switch(sel){
        case RTK_ALC1310:
            rtk_amp[amp_num].ops = alc1310_func;
            rtk_amp[amp_num].addr_size = ALC1310_ADDR_SIZE;
            rtk_amp[amp_num].data_size = ALC1310_DATA_SIZE;
            rtk_amp[amp_num].param_get = alc1310_param_get;
            rtk_amp[amp_num].param_set = alc1310_param_set;
            rtk_amp[amp_num].mute_set = alc1310_mute_set;
            rtk_amp[amp_num].dump_all = alc1310_dump_all;

            snprintf(rtk_amp[amp_num].name , 32 , "ALC1310");
            break;

        case ESMT_AD82010:
            rtk_amp[amp_num].ops = ad82010_func;
            rtk_amp[amp_num].addr_size = AD82010_ADDR_SIZE;
            rtk_amp[amp_num].data_size = AD82010_DATA_SIZE;
            rtk_amp[amp_num].param_get = ad82010_param_get;
            rtk_amp[amp_num].param_set = ad82010_param_set;
            rtk_amp[amp_num].mute_set = ad82010_mute_set;
            rtk_amp[amp_num].dump_all = ad82010_dump_all;

            snprintf(rtk_amp[amp_num].name , 32 , "AD82010");
            break;

        case ESMT_AD82120:
            rtk_amp[amp_num].ops = ad82120_func;
            rtk_amp[amp_num].addr_size = AD82120_ADDR_SIZE;
            rtk_amp[amp_num].data_size = AD82120_DATA_SIZE;
            rtk_amp[amp_num].param_get = ad82120_param_get;
            rtk_amp[amp_num].param_set = ad82120_param_set;
            rtk_amp[amp_num].mute_set = ad82120_mute_set;
            rtk_amp[amp_num].dump_all = ad82120_dump_all;

            snprintf(rtk_amp[amp_num].name , 32 , "AD82120");
            break;

        case ESMT_AD82088:
            rtk_amp[amp_num].ops = ad82088_func;
            rtk_amp[amp_num].addr_size = AD82088_ADDR_SIZE;
            rtk_amp[amp_num].data_size = AD82088_DATA_SIZE;
            rtk_amp[amp_num].param_get = ad82088_param_get;
            rtk_amp[amp_num].param_set = ad82088_param_set;
            rtk_amp[amp_num].mute_set = ad82088_mute_set;
            rtk_amp[amp_num].dump_all = ad82088_dump_all;
            rtk_amp[amp_num].amp_reset = ad82088_amp_reset;
            rtk_amp[amp_num].amp_device_suspend = ad82088_amp_mute_set;
            snprintf(rtk_amp[amp_num].name , 32 , "AD82088");
            break;

        case ESMT_AD82050:
            rtk_amp[amp_num].ops = ad82050_func;
            rtk_amp[amp_num].addr_size = AD82050_ADDR_SIZE;
            rtk_amp[amp_num].data_size = AD82050_DATA_SIZE;
            rtk_amp[amp_num].param_get = ad82050_param_get;
            rtk_amp[amp_num].param_set = ad82050_param_set;
            rtk_amp[amp_num].mute_set = ad82050_mute_set;
            rtk_amp[amp_num].amp_reset = ad82050_amp_reset;
            snprintf(rtk_amp[amp_num].name , 32 , "AD82050");
            break;

        case TI_TAS5805:
            rtk_amp[amp_num].ops = tas5805m_func;
            rtk_amp[amp_num].addr_size = TAS5805M_ADDR_SIZE;
            rtk_amp[amp_num].data_size = TAS5805M_DATA_SIZE;
            rtk_amp[amp_num].param_get = tas5805m_param_get;
            rtk_amp[amp_num].param_set = tas5805m_param_set;
            rtk_amp[amp_num].mute_set = tas5805m_mute_set;
            rtk_amp[amp_num].amp_reset = tas5805m_amp_reset;
            rtk_amp[amp_num].dump_all = tas5805m_dump_all;

            snprintf(rtk_amp[amp_num].name , 32 , "TAS5805");
            break;

        case TI_TAS5751:
            rtk_amp[amp_num].ops = tas5751_func;
            rtk_amp[amp_num].addr_size = TAS5751_ADDR_SIZE;
            rtk_amp[amp_num].data_size = TAS5751_DATA_SIZE;
            rtk_amp[amp_num].param_get = tas5751_param_get;
            rtk_amp[amp_num].param_set = tas5751_param_set;
            rtk_amp[amp_num].mute_set = tas5751_mute_set;
            rtk_amp[amp_num].dump_all = tas5751_dump_all;

            snprintf(rtk_amp[amp_num].name , 32 , "TAS5751");
            break;

        case TI_TAS5707:
            rtk_amp[amp_num].ops = tas5707_func;
            rtk_amp[amp_num].addr_size = TAS5707_ADDR_SIZE;
            rtk_amp[amp_num].data_size = TAS5707_DATA_SIZE;
            rtk_amp[amp_num].param_get = tas5707_param_get;
            rtk_amp[amp_num].param_set = tas5707_param_set;
            rtk_amp[amp_num].mute_set = tas5707_mute_set;
            rtk_amp[amp_num].dump_all = tas5707_dump_all;
            rtk_amp[amp_num].amp_reset = tas5707_amp_reset;

            snprintf(rtk_amp[amp_num].name , 32 , "TAS5707");
            break;

        case TI_TAS5711:
            rtk_amp[amp_num].ops = tas5711_func;
            snprintf(rtk_amp[amp_num].name , 32 , "TAS5711");
            /*rtk_amp.param_get = tas5707_param_get;
            rtk_amp.param_set = tas5707_param_set;
            rtk_amp.mute_set = tas5707_mute_set;
            rtk_amp.dump_all = tas5707_dump_all;*/
            break;

       case RTK_ALC1312:
            rtk_amp[amp_num].ops = alc1312_func;
            rtk_amp[amp_num].addr_size = ALC1312_ADDR_SIZE;
            rtk_amp[amp_num].data_size = ALC1312_DATA_SIZE;
            rtk_amp[amp_num].param_get = alc1312_param_get;
            rtk_amp[amp_num].param_set = alc1312_param_set;
            rtk_amp[amp_num].mute_set = alc1312_mute_set;
            rtk_amp[amp_num].dump_all = alc1312_dump_all;

            break;

       case WA_6819:
            rtk_amp[amp_num].ops = wa6819_func;
            snprintf(rtk_amp[amp_num].name , 32 , "WA6819");
            break;

       case WA_156819:
            rtk_amp[amp_num].ops = WA156819_func;
            rtk_amp[amp_num].addr_size = WA156819_ADDR_SIZE;
            rtk_amp[amp_num].data_size = WA156819_DATA_SIZE;
            rtk_amp[amp_num].param_get = WA156819_param_get;
            rtk_amp[amp_num].param_set = WA156819_param_set;
            rtk_amp[amp_num].mute_set = WA156819_mute_set;
            rtk_amp[amp_num].amp_reset = WA156819_amp_reset;
            snprintf(rtk_amp[amp_num].name , 32 , "WA156819");
            break;

        default:
            AMP_ERR("%s fail , set to ALC1310 , but maybe get no sound due to AMP power on setting incorrect in bootcode \n",__func__ );
            rtk_amp[amp_num].ops = alc1310_func;
            rtk_amp[amp_num].param_get = alc1310_param_get;
            rtk_amp[amp_num].param_set = alc1310_param_set;
            rtk_amp[amp_num].mute_set = alc1310_mute_set;
            rtk_amp[amp_num].dump_all = alc1310_dump_all;

            snprintf(rtk_amp[amp_num].name , 32 , "ALC1310");
    }

    AMP_INFO("%s amp%d name = %s \n",__func__, amp_num+1, rtk_amp[amp_num].name );

}

int rtk_amp_i2c_id(void)
{
    return rtk_amp[0].amp_i2c_id;
}

int amp_get_i2c_id(void)
{
    unsigned long long param;

    if (pcb_mgr_get_enum_info_byname("AMP_I2C", &param) != 0)
    {
        AMP_ERR("%s line:%d bootcode pcb info lose to define  \"AMP_I2C\"  \n",__func__, __LINE__ );
        return -1;
    }

    AMP_WARN("%s line:%d get amp I2c id = %lld  \n",__func__, __LINE__  , param );

    return param;
}

int amp_get_addr(char *addr_name)
{
    unsigned long long param;

    if (pcb_mgr_get_enum_info_byname(addr_name, &param) != 0)
    {
        AMP_ERR("%s line:%d bootcode pcb info lose to define  \"%s\"  \n",__func__, __LINE__, addr_name);
        return -1;
    }

    AMP_WARN("%s line:%d get amp slave address(8bit) = %llx  \n",__func__, __LINE__  , param );

    return (param >> 1);
}



int parser_amp_cmd(const char *buf, int len,unsigned char *data)
{
    char* const delim = " ";
    char *token;
    int i = 0;
    while (NULL != (token = strsep((char **)&buf, delim))) {
        if(i >= 1)
        {
             data[i-1] = (unsigned char)simple_strtol( token,NULL,16);
        }
        i++;
    }
    if(i == len)
        return 0;
    else
        return (-1);
}

int rtk_amp_pin_get(unsigned char *pcbname)
{
    RTK_GPIO_PCBINFO_T pinInfo;
    int ret = 0;

    ret = rtk_gpio_get_pcb_info(pcbname,&pinInfo);
    if(ret == 0)
    {
        return rtk_gpio_output_get(pinInfo.gid);
    }

    AMP_WARN("AMP %s not define in pcb_name\n",pcbname);
    return -1;
}

int rtk_amp_pin_set(unsigned char *pcbname,unsigned char val)
{
    RTK_GPIO_PCBINFO_T pinInfo;
    int ret = 0;

    ret = rtk_gpio_get_pcb_info(pcbname,&pinInfo);
    if(ret == 0)
    {
        return rtk_gpio_output(pinInfo.gid,val);
    }

    AMP_WARN("AMP %s not define in pcb_name\n",pcbname);
    return -1;
}

#if defined(CONFIG_RTK_KDRV_AMP) || defined(BUILD_QUICK_SHOW)
static int __init amp_ops_init(char *options)
{

    if(options == NULL){
        AMP_INFO("boot amp select is NULL\n");
        amp_boot_sel[0] = AMP_SEL_NULL;
        amp_boot_addr[0] = AMP_ADDR_NULL;
    }
    else{
        AMP_INFO("boot select amp:: %s\n", options);
        if (sscanf(options, "%d-%x", &amp_boot_sel[0], &amp_boot_addr[0]) < 1){
            return 1;
        }

    }

    AMP_INFO("boot select amp sel_index = %d, addr = %x\n",amp_boot_sel[0], amp_boot_addr[0]);
    return 1;
}

early_param("amp", amp_ops_init);

static int __init amp2_ops_init(char *options)
{

    if(options == NULL){
        AMP_INFO("\nboot amp2 select is NULL\n");
        amp_boot_sel[1] = AMP_SEL_NULL;
        amp_boot_addr[1] = AMP_ADDR_NULL;
    }
    else{
        AMP_INFO("\nboot select amp2: %s\n", options);
        if (sscanf(options, "%d-%x", &amp_boot_sel[1], &amp_boot_addr[1]) < 1){
            return 1;
        }
    }

    AMP_INFO("boot select amp2 sel_index = %d, addr = %x\n",amp_boot_sel[1], amp_boot_addr[1]);
    return 1;
}
early_param("amp2", amp2_ops_init);
#else
extern bool rtk_parse_commandline_equal(const char *string, char *output_string, int string_size);
static int  amp_ops_init(void)
{
    char str[20] = {0};

    if (0 == rtk_parse_commandline_equal("amp", str, sizeof(str)))
    {
        AMP_ERR("[%s] line: %d, can't get amp from bootargs\n", __func__, __LINE__);
        amp_boot_sel[0] = AMP_SEL_NULL;
        amp_boot_addr[0] = AMP_ADDR_NULL;
        return -1;
    }

    AMP_ERR("boot select amp:: %s\n", str);
    if (sscanf(str, "%d-%x", &amp_boot_sel[0], &amp_boot_addr[0]) < 1){
        return 1;
    }

    AMP_ERR("boot select amp sel_index = %d, addr = %x\n",amp_boot_sel[0], amp_boot_addr[0]);
    return 1;
}

static int  amp2_ops_init(void)
{
    char str[20] = {0};

    if (0 == rtk_parse_commandline_equal("amp2", str, sizeof(str)))
    {
        AMP_ERR("[%s] line: %d, can't get amp from bootargs\n", __func__, __LINE__);
        amp_boot_sel[1] = AMP_SEL_NULL;
        amp_boot_addr[1] = AMP_ADDR_NULL;
        return -1;
    }

    AMP_ERR("boot select amp2:: %s\n", str);
    if (sscanf(str, "%d-%x", &amp_boot_sel[1], &amp_boot_addr[1]) < 1){
        return 1;
    }

    AMP_ERR("boot select amp2 sel_index = %d, addr = %x\n",amp_boot_sel[1], amp_boot_addr[1]);
    return 1;
}

#endif

#ifndef BUILD_QUICK_SHOW
void do_ampOps(struct work_struct *work)
{
    int amp_num=0;

    for (amp_num=0; amp_num<MULTI_AMP_CNT; amp_num++)
    {
        if (rtk_amp[amp_num].slave_addr != AMP_ADDR_NULL && rtk_amp[amp_num].ops != NULL)
        {
            AMP_ERR("[%s] id = %d , addr = 0x%X \n",
                __func__ , rtk_amp[amp_num].amp_i2c_id,rtk_amp[amp_num].slave_addr );
            rtk_amp[amp_num].ops(rtk_amp[amp_num].amp_i2c_id, rtk_amp[amp_num].slave_addr);
        }
    }
}

void _init_amp_delay_wq(work_func_t func)
{
    AMP_INFO("[%s] \n", __func__ );

    if (NULL == ampWq)
    {
        ampWq = create_workqueue("Amp workqueue");
        if (NULL == ampWq)
        {
            AMP_ERR("[%s] ERROR: Create Amp workqueue failed!\n", __func__ );
        }
        else
        {
            AMP_ERR("[%s] Create Amp workqueue successful!\n", __func__ );
            INIT_DELAYED_WORK(&amp_delayWork, func);
        }
    }

}

int _start_amp_delay_wq(unsigned int delayms)
{
    int ret = -1;

    AMP_ERR("[%s] \n", __func__ );

    if (ampWq)
    {
        ret = queue_delayed_work(ampWq, &amp_delayWork, msecs_to_jiffies(delayms));
    }

    return ret;
}

void _cancel_amp_delay_wq(void)
{
    AMP_ERR("[%s] \n", __func__ );
    if (ampWq)
    {
        cancel_delayed_work(&amp_delayWork);
        flush_workqueue(ampWq);
    }
}

static int rtk_amp_probe(struct platform_device *pdev)
{
      amp_devs= pdev;
      pm_runtime_forbid(&amp_devs->dev);
      pm_runtime_set_active(&amp_devs->dev);
      pm_runtime_enable(&amp_devs->dev);
      return 0;
}

static int amp_suspend(struct device *dev)
{
    pm_runtime_disable(&amp_devs->dev);
    if(rtk_amp[0].amp_device_suspend != NULL)
    {
        rtk_amp[0].amp_device_suspend(0);
    }
    AMP_WARN("[AMP] amp_notify_suspend\n");
    return 0;
}

static int amp_resume(struct device *dev)
{

// Please notice that this is only for REALTEK_AMP_TAS5711 & REALTEK_AMP_ACL1310
// If use different IC, please change this.
#ifdef CONFIG_RTK_KDRV_AMP_INIT_SKIP
#else
    int amp_num = 0;

    if(rtk_amp[amp_num].amp_reset != NULL)
    {
        rtk_amp[amp_num].amp_reset();
    }

    AMP_WARN("[AMP] amp_notify_resume\n");
    for (amp_num=0; amp_num<MULTI_AMP_CNT; amp_num++)
    {
        if(amp_boot_sel[amp_num] != AMP_SEL_NULL)
            rtk_amp[amp_num].ops(rtk_amp[amp_num].amp_i2c_id,rtk_amp[amp_num].slave_addr);
    }
    pm_runtime_enable(&amp_devs->dev);
#endif
    return 0;
}

int amp_pm_runtime_suspend(struct device *dev)
{
    AMP_WARN("@(%s:%d)\n", __func__, __LINE__);
    rtk_amp_pin_set("PIN_AMP_PWR_EN", 0);
    return 0;
}

int amp_pm_runtime_resume(struct device *dev)
{
    int amp_num = 0;
    AMP_WARN("@(%s:%d)\n", __func__, __LINE__);

    rtk_amp_pin_set("PIN_AMP_PWR_EN", 1);
    mdelay(10);
    if(rtk_amp[amp_num].amp_reset != NULL)
    {
        rtk_amp[amp_num].amp_reset();
    }
    for (amp_num=0; amp_num<MULTI_AMP_CNT; amp_num++)
    {
    if(amp_boot_sel[amp_num] != AMP_SEL_NULL)
        rtk_amp[amp_num].ops(rtk_amp[amp_num].amp_i2c_id,rtk_amp[amp_num].slave_addr);
    }
    return 0;
}
/*
static char *amp_devnode(struct device *dev, umode_t *mode)
{
    return NULL;
}*/


struct file_operations amp_fops = {
        .owner                  = THIS_MODULE,
};

ssize_t rtk_amp_show_param(struct device *dev,
                            struct device_attribute *attr, char *buf)
{
    ssize_t ret = 0;
    char *buf_info = NULL;

    buf_info = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if(buf_info == NULL)
        return ret;
    buf_info[0] = 0;

    if (strcmp(attr->attr.name, "amp_dump") == 0) {
        rtk_amp[0].dump_all(buf_info,rtk_amp[0].amp_i2c_id,rtk_amp[0].slave_addr);
        ret = sprintf(buf, "%s\n", buf_info);
    }
    else if ((rtk_amp[1].slave_addr != -1) && (strcmp(attr->attr.name, "amp_dump2") == 0)) {
        rtk_amp[1].dump_all(buf_info,rtk_amp[1].amp_i2c_id,rtk_amp[1].slave_addr);
        ret = sprintf(buf, "%s\n", buf_info);
    }
    else if (strcmp(attr->attr.name, "amp_slave_addr") == 0) {
        AMP_WARN("AMP Slave Address (8bit):\n");
        ret = sprintf(buf, "%x\n", (rtk_amp[0].slave_addr << 1));
    }
    else if (strcmp(attr->attr.name, "amp_slave_addr2") == 0) {
        AMP_WARN("AMP Slave Address2 (8bit):\n");
        ret = sprintf(buf, "%x\n", (rtk_amp[1].slave_addr << 1));
    }
    else if (strcmp(attr->attr.name, "amp_pin") == 0) {
        ret = sprintf(buf, "PIN_AMP_RESET Value:%d \nPIN_AMP_MUTE Value:%d \n", rtk_amp_pin_get("PIN_AMP_RESET"),rtk_amp_pin_get("PIN_AMP_MUTE"));
    }

    kfree(buf_info);
    return ret;
}

ssize_t rtk_amp_store_param(struct device *dev,
                           struct device_attribute *attr,
                           const char *buf, size_t count)
{
    int ret = 0;
    int on_off = 0;
    char cmd1[5];
    unsigned char data[16] = {0};

    if (strcmp(attr->attr.name, "amp_mute") == 0) {
        sscanf(buf, "%d\n",&on_off);
        ret = rtk_amp[0].mute_set(on_off,rtk_amp[0].amp_i2c_id,rtk_amp[0].slave_addr);
    }
    else if (strcmp(attr->attr.name, "amp_param") == 0) {
        sscanf(buf, "%4s\n", &cmd1[0]);
        if (strcmp(cmd1, "get") == 0) {
            ret = parser_amp_cmd(buf,rtk_amp[0].addr_size,data);
            ret = rtk_amp[0].param_get(data,rtk_amp[0].amp_i2c_id,rtk_amp[0].slave_addr);
        }
        else if(strcmp(cmd1, "set") == 0) {
           ret = parser_amp_cmd(buf ,rtk_amp[0].addr_size+rtk_amp[0].data_size, data);
           ret = rtk_amp[0].param_set(data,rtk_amp[0].amp_i2c_id,rtk_amp[0].slave_addr);
        }
        else if(strcmp(cmd1, "init") == 0) {
           rtk_amp[0].ops(rtk_amp[0].amp_i2c_id,rtk_amp[0].slave_addr);
        }
        else {
           AMP_ERR("No this amp command\n");
        }

    }else if ((rtk_amp[1].slave_addr != -1) && (strcmp(attr->attr.name, "amp_param2") == 0)) {
        sscanf(buf, "%4s\n", &cmd1[0]);
        if (strcmp(cmd1, "get") == 0) {
            ret = parser_amp_cmd(buf,rtk_amp[1].addr_size,data);
            ret = rtk_amp[1].param_get(data,rtk_amp[1].amp_i2c_id,rtk_amp[1].slave_addr);
        }
        else if(strcmp(cmd1, "set") == 0) {
           ret = parser_amp_cmd(buf ,rtk_amp[1].addr_size+rtk_amp[1].data_size, data);
           ret = rtk_amp[1].param_set(data,rtk_amp[1].amp_i2c_id,rtk_amp[1].slave_addr);
        }
        else if(strcmp(cmd1, "init") == 0) {
           rtk_amp[1].ops(rtk_amp[1].amp_i2c_id,rtk_amp[1].slave_addr);
        }
        else {
           AMP_ERR("No this amp command\n");
        }
    }else if (strcmp(attr->attr.name, "amp_pin") == 0) {
        sscanf(buf, "%4s %d\n", &cmd1[0],&on_off);
        if (strcmp(cmd1, "mute") == 0) {
            ret = rtk_amp_pin_set("PIN_AMP_MUTE",on_off);
        }else if (strcmp(cmd1, "reset") == 0) {
            ret = rtk_amp_pin_set("PIN_AMP_RESET",on_off);
        }

    }
    return count;
}

DEVICE_ATTR(amp_slave_addr, S_IWUSR | S_IWGRP | S_IRUGO, rtk_amp_show_param,
                    rtk_amp_store_param);
DEVICE_ATTR(amp_mute, S_IWUSR | S_IWGRP | S_IRUGO, rtk_amp_show_param,
                    rtk_amp_store_param);
DEVICE_ATTR(amp_dump, S_IWUSR | S_IWGRP | S_IRUGO, rtk_amp_show_param,
                    rtk_amp_store_param);
DEVICE_ATTR(amp_param, S_IWUSR | S_IWGRP | S_IRUGO, rtk_amp_show_param,
                    rtk_amp_store_param);
DEVICE_ATTR(amp_init, S_IWUSR | S_IWGRP | S_IRUGO, rtk_amp_show_param,
                    rtk_amp_store_param);
DEVICE_ATTR(amp_pin, S_IWUSR | S_IWGRP | S_IRUGO, rtk_amp_show_param,
                    rtk_amp_store_param);

DEVICE_ATTR(amp_slave_addr2, S_IWUSR | S_IWGRP | S_IRUGO, rtk_amp_show_param,
                    rtk_amp_store_param);
DEVICE_ATTR(amp_dump2, S_IWUSR | S_IWGRP | S_IRUGO, rtk_amp_show_param,
                    rtk_amp_store_param);
DEVICE_ATTR(amp_param2, S_IWUSR | S_IWGRP | S_IRUGO, rtk_amp_show_param,
                    rtk_amp_store_param);

static struct class *amp_class = NULL;
static struct cdev amp_cdev;
static struct device *amp_device_this;

static int rtk_amp_init(void) {
        return 0;
        #if 0
        int result = 0;
        int amp_num = 0;

        dev_t dev = 0;    //MKDEV(AMP_MAJOR, 0);

        AMP_ERR("amp: AMP init for Realtek AMP(2021/02/01)\n");

#ifdef CONFIG_RTK_KDRV_AMP_MODULE
        amp_ops_init();
        amp2_ops_init();
#endif


        //result = register_chrdev_region(dev, 1, "RTKamp");
        result = alloc_chrdev_region(&dev, 0, 1, "RTKamp");
        if (result < 0) {
            AMP_ERR("amp: can not get chrdev region...\n");
            return result;
        }

        amp_class = class_create(THIS_MODULE, "RTKamp");
        if (IS_ERR(amp_class)) {
            AMP_ERR("amp: can not create class...\n");
            result = PTR_ERR(amp_class);
            goto fail_class_create;
        }

        amp_class->devnode = amp_devnode;

        amp_devs = platform_device_register_simple("RTKamp", -1, NULL, 0);
        if (platform_driver_register(&amp_driver) != 0) {
            AMP_ERR("amp: can not register platform driver...\n");
            result = -EINVAL;
            goto fail_platform_driver_register;
        }

        cdev_init(&amp_cdev, &amp_fops);
        amp_cdev.owner = THIS_MODULE;
        amp_cdev.ops = &amp_fops;
        result = cdev_add(&amp_cdev, dev, 1);
        if (result < 0) {
            AMP_ERR("amp: can not add character device...\n");
            goto fail_cdev_init;
        }
        amp_device_this = device_create(amp_class, NULL, dev, NULL, "RTKamp");

        device_create_file(amp_device_this,&dev_attr_amp_mute);
        device_create_file(amp_device_this,&dev_attr_amp_pin);

        if(amp_boot_sel[0] != AMP_SEL_NULL && amp_boot_addr[0] != AMP_ADDR_NULL){
            device_create_file(amp_device_this,&dev_attr_amp_slave_addr);
            device_create_file(amp_device_this,&dev_attr_amp_dump);
            device_create_file(amp_device_this,&dev_attr_amp_param);
        }

        if(amp_boot_sel[1] != AMP_SEL_NULL && amp_boot_addr[1] != AMP_ADDR_NULL){
            device_create_file(amp_device_this,&dev_attr_amp_slave_addr2);
            device_create_file(amp_device_this,&dev_attr_amp_dump2);
            device_create_file(amp_device_this,&dev_attr_amp_param2);
        }

        for (amp_num=0; amp_num<MULTI_AMP_CNT; amp_num++)
        {
            rtk_amp[amp_num].amp_i2c_id = amp_get_i2c_id();
            if(rtk_amp[amp_num].amp_i2c_id == (-1))
            {
                AMP_ERR("%s line:%d amp i2c id failed \n",__func__, __LINE__);
                return -1;
            }

            if(amp_boot_addr[amp_num] == AMP_ADDR_NULL)
            {
                rtk_amp[amp_num].sel_index = 0;
                rtk_amp[amp_num].slave_addr = 0;
                if(amp_num == 0){
                    rtk_amp[amp_num].sel_index = amp_sel[amp_num];
                    rtk_amp[amp_num].slave_addr = amp_get_addr(p_amp_dev_addr[amp_num]);
                    if(rtk_amp[amp_num].slave_addr == (-1))
                    {
                        AMP_ERR("%s line:%d amp%d slave address failed \n",__func__, __LINE__, amp_num+1);
                        //return -1;
                    }
                }
            }
            else{
                rtk_amp[amp_num].sel_index = amp_boot_sel[amp_num];
                rtk_amp[amp_num].slave_addr = amp_boot_addr[amp_num];
            }

        }

#ifdef CONFIG_RTK_KDRV_AMP_INIT_SKIP
#else
        for (amp_num=0; amp_num<MULTI_AMP_CNT; amp_num++)
        {
            if(amp_boot_sel[amp_num] != AMP_SEL_NULL)
                rtk_amp_ops_set(amp_num, rtk_amp[amp_num].sel_index);
        }
        if (is_QS_amp_enable() == 0) {
            _init_amp_delay_wq(do_ampOps);
            _start_amp_delay_wq(0);
        }
#endif
        //AMP_ERR("\n AMP I2C Port_Id2  =  %d Slave_addr(7bit):%x\n", rtk_amp.amp_i2c_id,  rtk_amp.slave_addr);

#ifdef CONFIG_RTK_AMP_USER_RESUME
        #if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        // Nothing
        #else
        amp_devs->dev.power.is_userresume = true;
		#endif
        device_enable_async_suspend(&(amp_devs->dev));
#endif
        return 0;

fail_cdev_init:
        platform_driver_unregister(&amp_driver);
fail_platform_driver_register:
        platform_device_unregister(amp_devs);
        amp_devs = NULL;
        class_destroy(amp_class);
fail_class_create:
        amp_class = NULL;
        unregister_chrdev_region(dev, 1);
        return result;
        #endif

}

static void rtk_amp_exit(void) {
    dev_t dev = MKDEV(AMP_MAJOR, 0);

    if ((amp_devs == NULL) || (amp_class == NULL))
        BUG();

    device_destroy(amp_class, dev);
    cdev_del(&amp_cdev);

    platform_driver_unregister(&amp_driver);
    platform_device_unregister(amp_devs);
    amp_devs = NULL;

    class_destroy(amp_class);
    amp_class = NULL;


    device_remove_file(amp_device_this, &dev_attr_amp_mute);
    device_remove_file(amp_device_this, &dev_attr_amp_pin);

    if(amp_boot_sel[0] != AMP_SEL_NULL && amp_boot_addr[0] != AMP_ADDR_NULL){
        device_remove_file(amp_device_this, &dev_attr_amp_slave_addr);
        device_remove_file(amp_device_this, &dev_attr_amp_dump);
        device_remove_file(amp_device_this, &dev_attr_amp_param);
    }

    if(amp_boot_sel[1] != AMP_SEL_NULL && amp_boot_addr[1] != AMP_ADDR_NULL){
        device_remove_file(amp_device_this, &dev_attr_amp_slave_addr2);
        device_remove_file(amp_device_this, &dev_attr_amp_dump2);
        device_remove_file(amp_device_this, &dev_attr_amp_param2);
    }

    unregister_chrdev_region(dev, 1);
}

fs_initcall(rtk_amp_init);
module_exit(rtk_amp_exit);

#else
void do_ampOps(void)
{
    int amp_num=0;

    for (amp_num=0; amp_num<MULTI_AMP_CNT; amp_num++)
    {
        if (rtk_amp[amp_num].slave_addr != AMP_ADDR_NULL && rtk_amp[amp_num].ops != NULL)
        {
            AMP_INFO("[%s] id = %d , addr = 0x%X \n",
                __func__ , rtk_amp[amp_num].amp_i2c_id,rtk_amp[amp_num].slave_addr );
            rtk_amp[amp_num].ops(rtk_amp[amp_num].amp_i2c_id, rtk_amp[amp_num].slave_addr);
        }
    }
}


int rtk_amp_init(void) {
    return 0;
    #if 0
    int result = 0;
    int amp_num = 0;
    char str[20] = {0};
    int ret = 0;
    RTK_GPIO_PCBINFO_T pinInfo;

    ret = rtk_gpio_get_pcb_info("PIN_AMP_RESET",&pinInfo);
    if (ret)
        AMP_ERR("%s PIN_AMP_RESET=%x,ret=%d\n", __func__, pinInfo.gid, ret);

    if(ret == 0){
        rtk_gpio_set_dir(pinInfo.gid, 1);
        rtk_gpio_output(pinInfo.gid, 1);
        mdelay(20);
    }

    AMP_INFO("amp: AMP init for Realtek AMP(2020/12/24)\n");

    if(getenv("amp_select") != NULL && getenv("amp_select_value") != NULL) {
        snprintf(str, 20, "%s-%s ", getenv("amp_select"),getenv("amp_select_value"));
        amp_ops_init(str);
    }

    if(getenv("amp_select2") != NULL && getenv("amp_select_value2") != NULL) {
        snprintf(str, 20, "%s-%s ", getenv("amp_select2"),getenv("amp_select_value2"));
        amp2_ops_init(str);
    }

    for (amp_num=0; amp_num<MULTI_AMP_CNT; amp_num++)
    {
        rtk_amp[amp_num].amp_i2c_id = amp_get_i2c_id();
        if(rtk_amp[amp_num].amp_i2c_id == (-1))
        {
            //AMP_ERR("%s line:%d amp i2c id failed \n",__func__, __LINE__);
            return -1;
        }

        if(amp_boot_addr[amp_num] == AMP_ADDR_NULL)
        {
            rtk_amp[amp_num].sel_index = 0;
            rtk_amp[amp_num].slave_addr = 0;
            if(amp_num == 0){
                rtk_amp[amp_num].sel_index = amp_sel[amp_num];
                rtk_amp[amp_num].slave_addr = amp_get_addr(p_amp_dev_addr[amp_num]);
                if(rtk_amp[amp_num].slave_addr == (-1))
                {
                    AMP_ERR("%s line:%d amp%d slave address failed \n",__func__, __LINE__, amp_num+1);
                    return -1;
                }
            }
        }
        else{
            rtk_amp[amp_num].sel_index = amp_boot_sel[amp_num];
            rtk_amp[amp_num].slave_addr = amp_boot_addr[amp_num];
        }

    }

#ifdef CONFIG_RTK_KDRV_AMP_INIT_SKIP
#else
    for (amp_num=0; amp_num<MULTI_AMP_CNT; amp_num++)
    {
        if(amp_boot_sel[amp_num] != AMP_SEL_NULL)
            rtk_amp_ops_set(amp_num, rtk_amp[amp_num].sel_index);
    }

    do_ampOps();
#endif

    return 0;
    #endif
}


#endif

