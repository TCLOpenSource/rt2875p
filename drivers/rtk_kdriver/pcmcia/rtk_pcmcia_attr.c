#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/moduleparam.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <asm/uaccess.h>        /* copy_*_user */
#include <asm/io.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_gpio.h>
#include <linux/ctype.h>
#ifdef CONFIG_ARCH_RTK2851A
#include <rbus/pinmux_main_reg.h>
#else
#include <rbus/pinmux_reg.h>
#endif
#include "rtk_pcmcia_reg.h"
#include "rtk_pcmcia.h"

extern int pcmcia_fifo_mode_enable;
extern int pcmcia_fifo_write_debug_enable;
extern int pcmcia_fifo_read_debug_enable;
extern int pcmcia_poll_event_debug_enable;

extern RTK_PCMCIA *rtk_pcmcia[2];;
extern RTK_PCMCIA_RESET_PULSE_WIDTH_TYPE g_reset_pulse_width_type;
extern unsigned long pcmcia_amtc0;
extern unsigned long pcmcia_amtc1;
extern unsigned long pcmcia_iomtc;
extern unsigned long pcmcia_matc0;
extern unsigned long pcmcia_matc1;

void pcmcia_dump_mem(unsigned char* pData, unsigned short Len, char *buffer, int* cnt)
{
    int i,j;
    int count=0;

    count = sprintf( buffer, "---------------------------------------------------------------\n");
    count += sprintf( buffer + count, "|||   ADDRESS   00 01 02 03 04 05 06 07   08 09 0A 0B 0C 0D 0E 0F \n");
    count += sprintf( buffer + count, "---------------------------------------------------------------\n");
    for (i=0; i<Len; i+=16)
    {
        count += sprintf( buffer + count, "|||   ");
        count += sprintf( buffer + count, "%08x  ", i);
        for (j=0; j<16; j++)
        {
            int c = i+j;
            if (c<Len)
                count += sprintf( buffer + count, "%02x ", pData[c]);
            else
                count += sprintf( buffer + count,  "-- ");

            if ((c & 0x07)==0x07)
                count += sprintf( buffer + count, "  ");
        }

        count += sprintf( buffer + count,  " ");

        for (j=0; j<16; j++)
        {
            int c = i+j;
            if (isprint(pData[c]) && pData[c] <= 0x7F){
                count += sprintf( buffer + count, "%c", (char) pData[c]);
            }else{
                count += sprintf( buffer + count, ".");
            }
        }
        count += sprintf( buffer + count, "\n");
    }

    count += sprintf(buffer + count, "---------------------------------------------------------------\n");
    *cnt = count;
    buffer += count;
}

void pcmcia_read_cis(char *buf, int* cnt)
{
    unsigned char val[256];
    int i;
    int ret = 0;
    PCMCIA_WARNING("%s\n",__func__);

    for (i=0; i<256; i++)
    {
        if (pcmcia_fifo_mode_enable)
            ret = rtk_pcmcia_fifo_read(rtk_pcmcia[0], i<<1, &val[i], 1, PCMCIA_FLAGS_ATTR | PCMCIA_FLAGS_RD);
        else
            ret = rtk_pcmcia_read(rtk_pcmcia[0], i<<1, &val[i], 1, PCMCIA_FLAGS_ATTR | PCMCIA_FLAGS_RD);

        if (ret != PCMCIA_OK)
        {
            PCMCIA_WARNING("read write PCMCIA failed, copy data to user space failed\n");
            ret = -1;
            break;
        }
    }
    pcmcia_dump_mem(val, i, buf, cnt);
    return;
}

/*
 *Device Attribute
 */
ssize_t rtk_pcmcia_show_param(struct device *dev,
                              struct device_attribute *attr, char *buf)
{
    int ret = -1;
    if (strncmp(attr->attr.name, "amtc0", 5) == 0)
    {
        pcmcia_amtc0 = GET_PCMCIA_AMTC_0();
        ret = sprintf(buf, "0x%08lX\n", pcmcia_amtc0);
    }
    else if (strncmp(attr->attr.name, "amtc1", 5) == 0)
    {
        pcmcia_amtc1 = GET_PCMCIA_AMTC_1();
        ret = sprintf(buf, "0x%08lX\n", pcmcia_amtc1);
    }
    else if (strncmp(attr->attr.name, "iomtc", 5) == 0)
    {
        pcmcia_iomtc = GET_PCMCIA_IOMTC();
        ret = sprintf(buf, "0x%08lX\n", pcmcia_iomtc);
    }
    else if (strncmp(attr->attr.name, "matc0", 5) == 0)
    {
        pcmcia_matc0 = GET_PCMCIA_MATC_0();
        ret = sprintf(buf, "0x%08lX\n", pcmcia_matc0);
    }
    else if (strncmp(attr->attr.name, "matc1", 5) == 0)
    {
        pcmcia_matc1 = GET_PCMCIA_MATC_1();
        ret = sprintf(buf, "0x%08lX\n", pcmcia_matc1);
    }
    else if (strncmp(attr->attr.name, "debug_ctrl", 10) == 0)
    {
        int len = 0;

        /*for merlin5/merlin6 0xB80008A0[15:12] pad_gpio_137_io 656*/
//        ret = sprintf(buf, "force_disconnect=%d\n", (((rtd_inl(PINMUX_GPIO_RIGHT_CFG_13_reg) >> 12) & 0xF)!=1) ? 1 : 0);
//        buf += ret; len += ret;
        ret = sprintf(buf, "fifo_mode_en=%x\n", pcmcia_fifo_mode_enable);
        buf += ret; len += ret;
        ret = sprintf(buf, "fifo_rd_dbg_en=%x\n", pcmcia_fifo_read_debug_enable);
        buf += ret; len += ret;
        ret = sprintf(buf, "fifo_wr_dbg_en=%x\n", pcmcia_fifo_write_debug_enable);
        buf += ret; len += ret;
        ret = len;
    }
    else if (strncmp(attr->attr.name, "reset_pulse_width", 17) == 0)
    {
        ret = sprintf(buf, "cur:%s (114us/1ms/2ms/20ms)\n", get_reset_pulse_width_info(g_reset_pulse_width_type));
    }
    else if (strncmp(attr->attr.name, "checkcis", 8) == 0)
    {
        int len = 0;
        pcmcia_read_cis(buf, &len);
        ret = len;
    }
    else if (strncmp(attr->attr.name, "triger_card_reset", 17) == 0)
    {
        PCMCIA_WARNING ("%s  RTK_PCMCIA_IOC_CARD_RESET\n" , __func__);

        ret = rtk_pcmcia_card_reset(rtk_pcmcia[0], DEFAULT_PCMCIA_RESET_TIMEOUT);
    }
#ifdef RTK_PCMCIA_CD_DEBOUNCE
    else if (strncmp(attr->attr.name, "debounce_config", 15) == 0)
    {
        unsigned int deb_val;
        deb_val = rtk_pcmcia_cd_debounce_get(rtk_pcmcia[0]);
        ret = sprintf(buf, "debounce_value = 0x%08X\n", deb_val);
    }
#endif
    else
    {
        PCMCIA_WARNING("invalid command - %s\n", buf);
    }
    return ret;
}

ssize_t rtk_pcmcia_set_param(struct device *dev,
                             struct device_attribute *attr,
                             const char *buf, size_t count)
{
    char str[32]={0};
    unsigned long val;
    if (strncmp(attr->attr.name, "amtc0", 5) == 0)
    {
        if (sscanf(buf, "%lx", &val) == 1)
        {
            SET_PCMCIA_AMTC_0(val);
            pcmcia_amtc0 = val;
        }
    }
    else if (strncmp(attr->attr.name, "amtc1", 5) == 0)
    {
        if (sscanf(buf, "%lx", &val) == 1)
        {
            SET_PCMCIA_AMTC_1(val);
            pcmcia_amtc1 = val;
        }
    }
    else if (strncmp(attr->attr.name, "iomtc", 5) == 0)
    {
        if (sscanf(buf, "%lx", &val) == 1)
        {
            SET_PCMCIA_IOMTC(val);
            pcmcia_iomtc = val;
        }
    }
    else if (strncmp(attr->attr.name, "matc0", 5) == 0)
    {
        if (sscanf(buf, "%lx", &val) == 1)
        {
            SET_PCMCIA_MATC_0(val);
            pcmcia_matc0 = val;
        }
    }
    else if (strncmp(attr->attr.name, "matc1", 5) == 0)
    {
        if (sscanf(buf, "%lx", &val) == 1)
        {
            SET_PCMCIA_MATC_1(val);
            pcmcia_matc1 = val;
        }
    }
    else if (strncmp(attr->attr.name, "debug_ctrl", 10) == 0)
    {
        if (sscanf(buf, "fifo_mode_en=%lu", &val) == 1)
        {
            pcmcia_fifo_mode_enable = (val) ? 1 : 0;
        }
        else if (sscanf(buf, "fifo_rd_dbg_en=%lu", &val) == 1)
        {
            pcmcia_fifo_read_debug_enable = (val) ? 1 : 0;
        }
        else if (sscanf(buf, "fifo_wr_dbg_en=%lu", &val) == 1)
        {
            pcmcia_fifo_write_debug_enable = (val) ? 1 : 0;
        }
#if 0
        /*change pcmcia cd pin to gpio or not*/
        else if (sscanf(buf, "force_disconnect=%lu", &val) == 1)
        {
        /*for merlin5/merlin6 0xB80008A0[15:12] pad_gpio_137_io 656*/
            if (val)
                rtd_outl(PINMUX_GPIO_RIGHT_CFG_13_reg, rtd_inl(PINMUX_GPIO_RIGHT_CFG_13_reg) | (0xF<<12));
            else
                rtd_outl(PINMUX_GPIO_RIGHT_CFG_13_reg, (rtd_inl(PINMUX_GPIO_RIGHT_CFG_13_reg) & ~(0xF<<12)) | (1<<12));
        }
#endif
        else
        {
            PCMCIA_WARNING("invalid command - %s\n", buf);
        }
    }
    else if (strncmp(attr->attr.name, "reset_pulse_width", 17) == 0)
    {

        if (sscanf(buf, "%31s", str)!=1)
            return -1;

        if(strcmp(str, "114us") == 0)
            g_reset_pulse_width_type = RTK_PCMCIA_RESET_PULSE_WIDTH_114US;
        else if(strcmp(str, "1ms") == 0)
            g_reset_pulse_width_type = RTK_PCMCIA_RESET_PULSE_WIDTH_1MS;
        else if(strcmp(str, "2ms") == 0)
            g_reset_pulse_width_type = RTK_PCMCIA_RESET_PULSE_WIDTH_2MS;
        else if(strcmp(str, "20ms") == 0)
            g_reset_pulse_width_type = RTK_PCMCIA_RESET_PULSE_WIDTH_20MS;
        else
            PCMCIA_WARNING("unknown paluse width - %s\n", str);
    }
    else if (strncmp(attr->attr.name, "card_enable", 11) == 0)
    {
        if (sscanf(buf, "%lx", &val) == 1)
        {
            rtk_pcmcia_card_enable(rtk_pcmcia[0], (val) ? 1 : 0);
        }
    }
#ifdef RTK_PCMCIA_CD_DEBOUNCE
    else if (strncmp(attr->attr.name, "debounce_config", 15) == 0)
    {
        unsigned int deb_val;
        if (sscanf(buf, "%x", &deb_val) == 1)
        {
            rtk_pcmcia_cd_debounce_set(rtk_pcmcia[0], deb_val);
        }
    }
    else if (strncmp(attr->attr.name, "deb_manual_mode", 15) == 0)
    {
        unsigned int deb_manual_mode;
        if (sscanf(buf, "%x", &deb_manual_mode) == 1)
        {
            rtk_pcmcia_cd_manual_mode_set(rtk_pcmcia[0], deb_manual_mode);
        }
    }
    else if (strncmp(attr->attr.name, "deb_manual_value", 16) == 0)
    {
        unsigned int deb_manual_value;
        if (sscanf(buf, "%x", &deb_manual_value) == 1)
        {
            rtk_pcmcia_cd_manual_value_set(rtk_pcmcia[0], deb_manual_value);
        }
    }
#endif
    else
    {
        PCMCIA_WARNING("invalid command - %s\n", buf);
    }
    return count;
}

DEVICE_ATTR(amtc0, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
            rtk_pcmcia_show_param, rtk_pcmcia_set_param);
DEVICE_ATTR(amtc1, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
            rtk_pcmcia_show_param, rtk_pcmcia_set_param);
DEVICE_ATTR(iomtc, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
            rtk_pcmcia_show_param, rtk_pcmcia_set_param);
DEVICE_ATTR(matc0, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
            rtk_pcmcia_show_param, rtk_pcmcia_set_param);
DEVICE_ATTR(matc1, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
            rtk_pcmcia_show_param, rtk_pcmcia_set_param);
DEVICE_ATTR(debug_ctrl, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
        rtk_pcmcia_show_param, rtk_pcmcia_set_param);
DEVICE_ATTR(reset_pulse_width, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
            rtk_pcmcia_show_param, rtk_pcmcia_set_param);
DEVICE_ATTR(checkcis, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
            rtk_pcmcia_show_param, rtk_pcmcia_set_param);
DEVICE_ATTR(card_enable, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
            rtk_pcmcia_show_param, rtk_pcmcia_set_param);
DEVICE_ATTR(triger_card_reset, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
            rtk_pcmcia_show_param, rtk_pcmcia_set_param);
#ifdef RTK_PCMCIA_CD_DEBOUNCE
DEVICE_ATTR(debounce_config, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
            rtk_pcmcia_show_param, rtk_pcmcia_set_param);
DEVICE_ATTR(deb_manual_mode, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
            rtk_pcmcia_show_param, rtk_pcmcia_set_param);
DEVICE_ATTR(deb_manual_value, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP,
            rtk_pcmcia_show_param, rtk_pcmcia_set_param);

#endif


void creat_pcmcia_device_node(struct device *dev)
{
    device_create_file(dev, &dev_attr_amtc0);
    device_create_file(dev, &dev_attr_amtc1);
    device_create_file(dev, &dev_attr_iomtc);
    device_create_file(dev, &dev_attr_matc0);
    device_create_file(dev, &dev_attr_matc1);
    device_create_file(dev, &dev_attr_debug_ctrl);
    device_create_file(dev, &dev_attr_reset_pulse_width);
    device_create_file(dev, &dev_attr_checkcis);
    device_create_file(dev, &dev_attr_card_enable);
    device_create_file(dev, &dev_attr_triger_card_reset);
#ifdef RTK_PCMCIA_CD_DEBOUNCE
    device_create_file(dev, &dev_attr_debounce_config);
    device_create_file(dev, &dev_attr_deb_manual_mode);
    device_create_file(dev, &dev_attr_deb_manual_value);
#endif

}

void remove_pcmcia_device_node(struct device *dev)
{
    device_remove_file(dev, &dev_attr_amtc0);
    device_remove_file(dev, &dev_attr_amtc1);
    device_remove_file(dev, &dev_attr_iomtc);
    device_remove_file(dev, &dev_attr_matc0);
    device_remove_file(dev, &dev_attr_matc1);
    device_remove_file(dev, &dev_attr_debug_ctrl);
    device_remove_file(dev, &dev_attr_reset_pulse_width);
    device_remove_file(dev, &dev_attr_checkcis);
    device_remove_file(dev, &dev_attr_card_enable);
    device_remove_file(dev, &dev_attr_triger_card_reset);
#ifdef RTK_PCMCIA_CD_DEBOUNCE
    device_remove_file(dev, &dev_attr_debounce_config);
    device_remove_file(dev, &dev_attr_deb_manual_mode);
    device_remove_file(dev, &dev_attr_deb_manual_value);
#endif
}

