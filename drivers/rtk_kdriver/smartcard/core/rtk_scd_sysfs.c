#include <linux/sched.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include "rtk_scd.h"
#include "rtk_scd_debug.h"
#include "rtk_scd_sysfs.h"

static int rtk_shown_atr(char * buf_info, scd_atr_info *p_atr_info)
{
    int ret = -1, i;
    ret = snprintf(buf_info, PAGE_SIZE, "ATR parse status = %d\n", p_atr_info->status);
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, "TS = 0x%02x\n", p_atr_info->TS);
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, "T0 = 0x%02x\n", p_atr_info->T0);
    buf_info += ret;
    for (i = 0; i < 4; i++){
        ret = snprintf(buf_info, PAGE_SIZE, "TA[%d] = 0x%02x\n", i, p_atr_info->T1[i]);
        buf_info += ret;
    }
    for (i = 0; i < 4; i++){
        ret = snprintf(buf_info, PAGE_SIZE, "TB[%d] = 0x%02x\n", i, p_atr_info->T2[i]);
        buf_info += ret;
    }
    for (i = 0; i < 4; i++){
        ret = snprintf(buf_info, PAGE_SIZE, "TC[%d] = 0x%02x\n", i, p_atr_info->T3[i]);
        buf_info += ret;
    }
    for (i = 0; i < 4; i++){
        ret = snprintf(buf_info, PAGE_SIZE, "TD[%d] = 0x%02x\n", i, p_atr_info->T4[i]);
        buf_info += ret;
    }
    ret = snprintf(buf_info, PAGE_SIZE, "HistoryBytes is: %d\n", p_atr_info->nHistoryBytes);
    buf_info += ret;
    for (i = 0; i < p_atr_info->nHistoryBytes; i++){
        ret = snprintf(buf_info, PAGE_SIZE, "H[%d] = 0x%02x\n", i, p_atr_info->History[i]);
        buf_info += ret;
    }
    ret = snprintf(buf_info, PAGE_SIZE, "TCK = 0x%02x \n", p_atr_info->TCK);
    buf_info += ret;
    return 0;
}

static int rtk_shown_attribute_help(char * buf_info)
{
    int ret = -1;
    ret = snprintf(buf_info, PAGE_SIZE, "\n*******   smc sysfs attribute use help *****\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, "1.echo 1/0 > /sys/devices/scd0/smc_log_onoff to enable/disable smc log\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, "2.echo 1/0 > /sys/devices/scd0/txrx_cmd_print to enable/disable apdu TX&RX log\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, "3.cat /sys/devices/scd0/smc_status  to shown smc status\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, "4.cat /sys/devices/scd0/shown_atr to shown smc atr(answer to reset)\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, "5.echo 1/0 > /sys/devices/scd0/smc_enable to enable/disable smart card\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, "6.echo 1/0 > /sys/devices/scd0/smc_active to active/disactive smart card\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, "7.echo 1 > /sys/devices/scd0/smc_enable to reset smart card\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, "8.echo (params) > /sys/devices/scd0/config to set config. params like follow:\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, " 8.1: vcc_lvl=0 (0: 3.3V, 1: 5V)\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, " 8.2: clock=1000 (means:clock=1000Hz)\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, " 8.3: etu=372 (372 or others)\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, " 8.4: conv=0 (0: auto, 1:direct, 2: inversed)\n");
    buf_info += ret;
    ret = snprintf(buf_info, PAGE_SIZE, " 8.5: parity=0 (0: none, 1:odd, 2: even)\n");
    buf_info += ret;
    return 0;
}

static ssize_t rtk_scd_attr_show(
    struct device*              dev,
    struct device_attribute*    attr,
    char*                       buf
    )
{
    scd_device *p_dev = to_scd_device(dev);
    scd_driver *p_drv = to_scd_driver(dev->driver);
    int  n, ret;
    int  count = PAGE_SIZE;
    char *ptr = buf;
    unsigned long val;

    if (strncmp(attr->attr.name, "smc_log_onoff", 13) == 0)
    {
        n = scnprintf(ptr, count, "log_on=%d (1:on, 0:off)\n", smc_log_onoff);
        ptr+=n; count-=n;
    }
    else if (strncmp(attr->attr.name, "txrx_cmd_print", 14) == 0)
    {
        n = scnprintf(ptr, count, "print_rtx=%d (1:on, 0:off)\n", smc_print_rtx);
        ptr+=n; count-=n;
    }
    else if (strncmp(attr->attr.name, "config", 6) == 0)
    {
        if (p_drv->get_param(p_dev, SC_PARAM_VCC_LVL, &val)==0)
        {
            n = scnprintf(ptr, count, "vcc_lvl=%lu (0: 3.3V, 1: 5V)\n", val);
            ptr+=n; count-=n;
        }
        if (p_drv->get_param(p_dev, SC_PARAM_CLOCK, &val)==0)
        {
            n = scnprintf(ptr, count, "clock=%lu Hz\n", val);
            ptr+=n; count-=n;
        }
        if (p_drv->get_param(p_dev, SC_PARAM_ETU, &val)==0)
        {
            n = scnprintf(ptr, count, "etu=%lu\n", val);
            ptr+=n; count-=n;
        }
        if (p_drv->get_param(p_dev, SC_PARAM_CONV, &val)==0)
        {
            n = scnprintf(ptr, count, "conv=%lu (0: auto, 1:direct, 2: inversed)\n", val);
            ptr+=n; count-=n;
        }
        if (p_drv->get_param(p_dev, SC_PARAM_PARITY, &val)==0)
        {
            n = scnprintf(ptr, count, "parity=%lu (0: none, 1:odd, 2: even)\n", val);
            ptr+=n; count-=n;
        }
    }

    if (strncmp(attr->attr.name, "smc_status", 10) == 0)
    {
        int card_status;
        card_status = p_drv->get_card_status(p_dev);
        if(card_status == SC_CARD_REMOVED)
            ret = scnprintf(ptr, count, "ICC does not exist \n");
        else if(card_status == SC_CARD_DEACTIVATE)
            ret = scnprintf(ptr, count, "ICC exists, but has not been activated \n");
        else if(card_status == SC_CARD_RESET)
            ret = scnprintf(ptr, count, "ICC card reseting \n");
        else if(card_status == SC_CARD_ACTIVATE)
            ret = scnprintf(ptr, count, "ICC exists, and activated \n");
        else
            ret = scnprintf(ptr, count, "card not enabled\n");
        ptr+=ret; count-=ret;
    }
    else if (strncmp(attr->attr.name, "shown_atr", 9) == 0)
    {
        scd_atr_info atr_info;
        scd_atr  atr;
        char *buf_info = kmalloc(PAGE_SIZE, GFP_KERNEL);
        if(buf_info == NULL)
            return 0;
        memset(buf_info, 0, PAGE_SIZE);
        memset(&atr, 0, sizeof(scd_atr));
        p_drv->get_atr(p_dev, &atr);
        decompress_atr(&atr, &atr_info);
        rtk_shown_atr(buf_info, &atr_info);
        ret = scnprintf(ptr, count, "%s\n", buf_info);
        ptr+=ret; count-=ret;
        kfree(buf_info);
    }
    else if (strncmp(attr->attr.name, "help", 4) == 0)
    {
        char *buf_info = kmalloc(PAGE_SIZE, GFP_KERNEL);
        if(buf_info == NULL)
            return 0;
        memset(buf_info, 0, PAGE_SIZE);
        rtk_shown_attribute_help(buf_info);
        ret = scnprintf(ptr, count, "%s\n", buf_info);
        ptr+=ret; count-=ret;
        kfree(buf_info);
    }
    return ptr - buf;
}


static ssize_t rtk_scd_attr_store(
    struct device*              dev,
    struct device_attribute*    attr,
    const char*                 buf,
    size_t                      count
    )
{
    scd_device *p_dev = to_scd_device(dev);
    scd_driver *p_drv = to_scd_driver(dev->driver);
    unsigned long val;

    if (strcmp(attr->attr.name, "smc_log_onoff") == 0)
    {
        if (sscanf(buf, "%lu", &val)==1)
        {
            smc_log_onoff = (val) ? 1 : 0;
            SC_INFO("Smart Card debug message output %s\n", (smc_log_onoff) ? "Enabled" : "Disabled");
        }
    }
    else if (strcmp(attr->attr.name, "txrx_cmd_print") == 0)
    {
        if (sscanf(buf, "%lu", &val)==1)
        {
            smc_print_rtx = (val) ? 1 : 0;
            SC_INFO("Smart Card I/O debug message output %s\n", (smc_print_rtx) ? "Enabled" : "Disabled");
        }
    }
    else if (strcmp(attr->attr.name, "config") == 0)
    {
        if (sscanf(buf, "vcc_lvl=%lu", &val)==1)
        {
            if (val >=SC_VCC_3V && val <= SC_VCC_5V)
                p_drv->set_param(p_dev, SC_PARAM_VCC_LVL, (val) ? 1 : 0);
            else
                SC_WARNING("unknown vcc lvl - %lu\n", val);
        }
        else if (sscanf(buf, "clock=%lu", &val)==1)
        {
            if (val > 1000000)
                p_drv->set_param(p_dev, SC_PARAM_CLOCK, (unsigned long) val);
            else
                SC_WARNING("invalid clock - %lu (clock should more than or equal to 1000000)\n", val);
        }
        else if (sscanf(buf, "etu=%lu", &val)==1)
        {
            if (val>=16)
                p_drv->set_param(p_dev, SC_PARAM_ETU, (unsigned long) val);
            else
                SC_WARNING("invalid etu - %lu (etu should more than or equal to 16)\n", val);
        }
        else if (sscanf(buf, "conv=%lu", &val)==1)
        {
            if (val >=SC_AUTO_CONV && val <= SC_INVERSE_CONV)
                p_drv->set_param(p_dev, SC_PARAM_CONV, (unsigned long) val);
            else
                SC_WARNING("unknown convention - %lu\n", val);
        }
        else if (sscanf(buf, "parity=%lu", &val)==1)
        {
            if (val >=SC_PARITY_NONE && val <=SC_PARITY_EVEN)
                p_drv->set_param(p_dev, SC_PARAM_PARITY, (unsigned long) val);
            else
                SC_WARNING("unknown parity - %lu\n", val);
        }
    }
    else if (strncmp(attr->attr.name, "smc_enable", 10) == 0)
    {
        if (sscanf(buf, "%lx", &val) == 1)
        {
            char *str = (val == 1)?"enable":"disable";
            SC_PRINT_AWAYS("[SMC] %s card start!\n ", str);
            p_drv->enable(p_dev, val);
            SC_PRINT_AWAYS("[SMC] %s card end!\n ", str);
        }
    }
    else if (strncmp(attr->attr.name, "smc_active", 10) == 0)
    {
        if (sscanf(buf, "%lx", &val) == 1)
        {
            if(val == 0){
                SC_PRINT_AWAYS("[SMC] deactive card start!\n ");
                p_drv->deactivate(p_dev);
                SC_PRINT_AWAYS("[SMC] deactive card end!\n ");
            }
            else{
                SC_PRINT_AWAYS("[SMC] active card start!\n ");
                p_drv->activate(p_dev);
                SC_PRINT_AWAYS("[SMC] active card end!\n ");
            }
        }
    }
    else if (strncmp(attr->attr.name, "smc_reset", 9) == 0)
    {
        if (sscanf(buf, "%lx", &val) == 1)
        {
            SC_PRINT_AWAYS("[SMC] reset card start!\n ");
            p_drv->reset(p_dev);
            SC_PRINT_AWAYS("[SMC] reset card end!\n ");
        }
    }
    return count;
}

static DEVICE_ATTR(help, S_IWUSR|S_IWGRP|S_IRUGO, rtk_scd_attr_show, rtk_scd_attr_store);
static DEVICE_ATTR(smc_log_onoff, S_IWUSR|S_IWGRP|S_IRUGO, rtk_scd_attr_show, rtk_scd_attr_store);
static DEVICE_ATTR(txrx_cmd_print, S_IWUSR|S_IWGRP|S_IRUGO, rtk_scd_attr_show, rtk_scd_attr_store);
static DEVICE_ATTR(config, S_IWUSR|S_IWGRP|S_IRUGO, rtk_scd_attr_show, rtk_scd_attr_store);

DEVICE_ATTR(smc_enable, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_scd_attr_show, rtk_scd_attr_store);
DEVICE_ATTR(smc_active, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_scd_attr_show, rtk_scd_attr_store);
DEVICE_ATTR(smc_reset, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_scd_attr_show, rtk_scd_attr_store);
DEVICE_ATTR(shown_atr, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_scd_attr_show, rtk_scd_attr_store);
DEVICE_ATTR(smc_status, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_scd_attr_show, rtk_scd_attr_store);

void rtk_scd_sysfs_create(struct device *device)
{
    device_create_file(device,&dev_attr_help);
    device_create_file(device,&dev_attr_config);
    device_create_file(device,&dev_attr_smc_log_onoff);
    device_create_file(device,&dev_attr_txrx_cmd_print);
    device_create_file(device, &dev_attr_smc_enable);
    device_create_file(device, &dev_attr_smc_active);
    device_create_file(device, &dev_attr_smc_reset);
    device_create_file(device, &dev_attr_shown_atr);
    device_create_file(device, &dev_attr_smc_status);
}

void rtk_scd_sysfs_destroy(struct device *device)
{
    device_remove_file(device, &dev_attr_help);
    device_remove_file(device, &dev_attr_config);
    device_remove_file(device, &dev_attr_smc_log_onoff);
    device_remove_file(device, &dev_attr_txrx_cmd_print);
    device_remove_file(device, &dev_attr_smc_enable);
    device_remove_file(device, &dev_attr_smc_active);
    device_remove_file(device, &dev_attr_smc_reset);
    device_remove_file(device, &dev_attr_shown_atr);
    device_remove_file(device, &dev_attr_smc_status);
}
