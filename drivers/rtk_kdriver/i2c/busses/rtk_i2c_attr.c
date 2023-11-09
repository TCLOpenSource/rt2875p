#include <rtk_crt.h>
#include "rtk_i2c_priv.h"
#include "rtk_i2c_attr.h"
#include "rtk_i2c.h"

extern int i2c_rtk_set_speed(void *dev_id, int KHz);
extern void rtk_i2c_slave_en(rtk_i2c *p_this, int val);

ssize_t rtk_i2c_show_param(struct device *dev, struct device_attribute *attr,
                           char *buf);
ssize_t rtk_i2c_store_param(struct device *dev, struct device_attribute *attr,
                            const char *buf, size_t count);

DEVICE_ATTR(fixed_speed,        S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(disable_auto_recovery, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(polling_mode,       S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(recovery,           S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(reset_clock,        S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(i2c_loglevel,       S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(run_read,           S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(port,               S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(disable_switch_port, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(speed,              S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(bus_status,         S_IRUSR | S_IRGRP | S_IROTH, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(bus_jam_recover,    S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(config,             S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(hold_time,          S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(setup_time,         S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(error_message_on_off, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(disable_i2c,        S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(status_i2c,         S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(slave_en,           S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(i2c_GPIO_mode,      S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);
DEVICE_ATTR(dev_info,           S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP, rtk_i2c_show_param, rtk_i2c_store_param);

ssize_t rtk_i2c_show_param(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_adapter *p_adap = to_i2c_adapter(dev);
    rtk_i2c_adapter *p_vadp = container_of(p_adap, rtk_i2c_adapter, adap);
    rtk_i2c *p_this = (rtk_i2c *) p_vadp->p_phy;
    int ret = 0;

    if (p_this)
    {
        LockrtkI2CPhy(p_this);

        if (strncmp(attr->attr.name, "speed", 5) == 0)
        {
            ret = snprintf(buf, PAGE_SIZE, "%d\n", p_vadp->spd);
        }
        else if (strncmp(attr->attr.name, "bus_status", 10) == 0)
        {
            buf[0] = p_this->get_bus_status(p_this) ? 1 : 0;
            ret = 1;
        }
        else if (strncmp(attr->attr.name, "disable_switch_port", strlen("disable_switch_port")) == 0)
        {
            ret = snprintf(buf, PAGE_SIZE, "%d\n", p_this->disable_switch_port);
        }
        else if (strncmp(attr->attr.name, "port", strlen("port")) == 0)
        {
            ret = snprintf(buf, PAGE_SIZE, "%d\n", p_vadp->port);
        }
        else if (strncmp(attr->attr.name, "fixed_speed", strlen("fixed_speed")) == 0)
        {
            ret = snprintf(buf, PAGE_SIZE, "%d\n", p_this->fixed_speed);
        }
        else if (strncmp(attr->attr.name, "i2c_loglevel", strlen("i2c_loglevel")) == 0)
        {
            ret = snprintf(buf, PAGE_SIZE, "%d\n", p_this->i2c_loglevel);
        }
        else if (strncmp(attr->attr.name, "config", strlen("config")) == 0)
        {
            p_this->dump(p_this);
        }
        else if (strncmp(attr->attr.name, "hold_time", strlen("hold_time")) == 0)
        {
            ret = snprintf(buf, PAGE_SIZE, "0x%x\n", p_this->get_hold_time(p_this));
        }
        else if (strncmp(attr->attr.name, "setup_time", strlen("setup_time")) == 0)
        {
            ret = snprintf(buf, PAGE_SIZE, "0x%x\n", p_this->get_setup_time(p_this));
        }
        else if (strncmp(attr->attr.name, "error_message_on_off", strlen("error_message_on_off")) == 0)
        {
            ret = snprintf(buf, PAGE_SIZE, "%x\n", I2C_error_log_onoff);
        }
        else if (strncmp(attr->attr.name, "disable_i2c", strlen("disable_i2c")) == 0)
        {
            ret = snprintf(buf, PAGE_SIZE, "%x\n", p_this->i2c_function_disable);
        }
        else if (strncmp(attr->attr.name, "i2c_GPIO_mode", strlen("i2c_GPIO_mode")) == 0)
        {
            ret = snprintf(buf, PAGE_SIZE, "%x\n", p_this->i2c_GPIO_mode);
        }
        else if (strncmp(attr->attr.name, "slave_en", strlen("slave_en")) == 0)
        {
            ret = snprintf(buf, PAGE_SIZE, "%d\n", (p_this->flags & rtk_I2C_SLAVE_ENABLE) >> 7);
        }
        else if (strncmp(attr->attr.name, "status_i2c", strlen("status_i2c")) == 0)
        {
            ret = snprintf(buf, PAGE_SIZE, "[ch_%d][phy_%d] port = %d\n"
                                "disable_i2c = %x\n"
                                "disable_switch_port = %d\n"
                                "speed = %d\n"
                                "fixed_speed = %d\n"
                                "i2c_loglevel = %d\n"
                                "hold_time = 0x%x\n"
                                "setup_time = 0x%x\n"
                                "i2c_GPIO_mode = %d\n"
                                "error_message_on_off = %x\n"
                                "slave_en = %d\n",
                                p_this->logical_id,
                                p_this->id,
                                p_vadp->port,
                                p_this->i2c_function_disable,
                                p_this->disable_switch_port,
                                p_vadp->spd,
                                p_this->fixed_speed,
                                p_this->i2c_loglevel,
                                p_this->get_hold_time(p_this),
                                p_this->get_setup_time(p_this),
                                p_this->i2c_GPIO_mode,
                                I2C_error_log_onoff,
                                (p_this->flags & rtk_I2C_SLAVE_ENABLE)>>7);
        }
        else if (strncmp(attr->attr.name, "dev_info", strlen("dev_info")) == 0)
        {
            char *buf_info = kmalloc(PAGE_SIZE, GFP_KERNEL);
            if (buf_info == NULL) { return 0; }
            buf_info[0] = 0;
            rtk_i2c_dump_dev_info(p_this->logical_id, buf_info);
            ret = snprintf(buf, PAGE_SIZE, "%s\n", buf_info);
            kfree(buf_info);
        }

        UnlockrtkI2CPhy(p_this);
    }

    return ret;
}

ssize_t rtk_i2c_store_param(struct device *dev, struct device_attribute *attr,
                            const char *buf, size_t count)
{
    struct i2c_adapter *p_adap = to_i2c_adapter(dev);
    rtk_i2c_adapter *p_vadp = container_of(p_adap, rtk_i2c_adapter, adap);
    rtk_i2c *p_this = (rtk_i2c *) p_vadp->p_phy;
    int val = 0;
    int val2 = 0;
    int val3 = 0;

    if (p_this)
    {
        LockrtkI2CPhy(p_this);

        if (strncmp(attr->attr.name, "speed", 5) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                i2c_rtk_set_speed(p_vadp, val);
            }
        }
        else if (strncmp(attr->attr.name, "bus_jam_recover", 15) == 0)
        {
            val = 0;
            sscanf(buf, "%d\n", &val);
            if (val || p_this->get_bus_status(p_this))
            {
                p_this->do_bus_jame_recover(p_this);
            }
        }
        else if (strncmp(attr->attr.name, "disable_switch_port", strlen("disable_switch_port")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                if (val == 0)
                {
                    p_this->disable_switch_port = 0 ;
                }
                else
                {
                    p_this->disable_switch_port = 1;
                }
            }
        }
        else if (strncmp(attr->attr.name, "port", strlen("port")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                p_vadp->port = val;
            }
        }
        else if (strncmp(attr->attr.name, "run_read", strlen("run_read")) == 0)
        {
            int ret;
            ret = sscanf(buf, "%x,%x,%x\n", &val, &val2, &val3);

            if (val3 > 1024)
            {
                I2C_WARNING("run_read data size > 1024 \n");
                return count ;
            }

            if (ret > 0)
            {
                char *read_buf = NULL;
                unsigned char send_buf[2] = {0};

                read_buf = kmalloc(1024, GFP_KERNEL);
                if (read_buf == NULL)
                {
                    return 0;
                }
                read_buf[0] = 0;

                I2C_WARNING("get run_read is %x ; %x ; %x \n", val, val2, val3);

                p_this->set_port(p_this, p_vadp->port);
                p_this->set_tar(p_this, val >> 1, ADDR_MODE_7BITS);

                if (ret == 1)
                {
                    p_this->read(p_this, NULL, 0, read_buf, 1);
                }
                else
                {
                    send_buf[0] = val2;
                    p_this->read(p_this, send_buf, 1, read_buf, val3);
                }

                p_this->unset_port(p_this, p_vadp->port);
                //i2c_master_recv_ex(p_this->id, val >> 1, NULL, 0, buf, 1);
                I2C_WARNING("reason is %x \n", p_this->get_tx_abort_reason(p_this));
                I2C_WARNING("read data is %x \n", read_buf[0]);
                kfree(read_buf);
            }
        }
        else if (strncmp(attr->attr.name, "i2c_loglevel", strlen("i2c_loglevel")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                p_this->i2c_loglevel = val ;
            }
        }
        else if (strncmp(attr->attr.name, "polling_mode", strlen("polling_mode")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                I2C_WARNING("get polling mode  is %x \n", val);
                if (val == 0)
                {
                    p_this->polling_mode = 0 ;
                }
                else
                {
                    p_this->polling_mode = 1;
                }
            }
        }
        else if (strncmp(attr->attr.name, "recovery", strlen("recovery")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                I2C_WARNING("get recovery  is %x \n", val);
                if (val != 0)
                {
#ifdef CONFIG_RTK_KDRV_I2C_BUS_JAM_RECOVER
                    //rtk_i2c_bus_jam_recover_proc(p_this);
                    //msleep(50);
#endif
                }
            }
        }
        else if (strncmp(attr->attr.name, "reset_clock", strlen("reset_clock")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                I2C_WARNING("get reset_clock  is %x \n", val);
                if (val != 0)
                {
                    int i2c_phy = BIT(p_this->id);
                    CRT_CLK_OnOff(I2C, CLK_OFF, &i2c_phy);
                    mdelay(50);
                    CRT_CLK_OnOff(I2C, CLK_ON, &i2c_phy);
                }
            }
        }
        else if (strncmp(attr->attr.name, "disable_auto_recovery", strlen("disable_auto_recovery")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                I2C_WARNING("get disable_auto_recovery mode  is %x\n", val);
                if (val == 0)
                {
                    p_this->disable_auto_recovery = 0 ;
                }
                else
                {
                    p_this->disable_auto_recovery = 1;
                }
            }
        }
        else if (strncmp(attr->attr.name, "fixed_speed", strlen("fixed_speed")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                I2C_WARNING("get fixed_speed is %d \n", val);
                if (val == 0)
                {
                    p_this->fixed_speed = 0;
                }
                else
                {
                    p_this->fixed_speed = 1;
                }
            }
        }
        else if (strncmp(attr->attr.name, "hold_time", strlen("hold_time")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                p_this->set_hold_time(p_this, val);
            }
        }
        else if (strncmp(attr->attr.name, "setup_time", strlen("setup_time")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                p_this->set_setup_time(p_this, val);
            }
        }
        else if (strncmp(attr->attr.name, "error_message_on_off", strlen("error_message_on_off")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                I2C_error_log_onoff = val;
            }
        }
        else if (strncmp(attr->attr.name, "disable_i2c", strlen("disable_i2c")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                if (val == 0 || val == 1)
                {
                    p_this->i2c_function_disable = val;
                }
            }
        }
        else if (strncmp(attr->attr.name, "i2c_GPIO_mode", strlen("i2c_GPIO_mode")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                if (val == 0 || val == 1)
                {
                    p_this->i2c_GPIO_mode = val;
                }
            }
        }
        else if (strncmp(attr->attr.name, "slave_en", strlen("slave_en")) == 0)
        {
            if (sscanf(buf, "%d\n", &val) == 1)
            {
                if (val == 0 || val == 1)
                {
                    rtk_i2c_slave_en(p_this, val);
                }
            }
        }

        UnlockrtkI2CPhy(p_this);
    }

    return count;
}
void rtk_i2c_create_attr(struct device *device)
{
    device_create_file(device, &dev_attr_fixed_speed);
    device_create_file(device, &dev_attr_disable_auto_recovery);
    device_create_file(device, &dev_attr_polling_mode);
    device_create_file(device, &dev_attr_recovery);
    device_create_file(device, &dev_attr_reset_clock);
    device_create_file(device, &dev_attr_i2c_loglevel);
    device_create_file(device, &dev_attr_run_read);
    device_create_file(device, &dev_attr_port);
    device_create_file(device, &dev_attr_disable_switch_port);
    device_create_file(device, &dev_attr_speed);
    device_create_file(device, &dev_attr_bus_status);
    device_create_file(device, &dev_attr_bus_jam_recover);
    device_create_file(device, &dev_attr_config);
    device_create_file(device, &dev_attr_hold_time);
    device_create_file(device, &dev_attr_setup_time);
    device_create_file(device, &dev_attr_error_message_on_off);
    device_create_file(device, &dev_attr_disable_i2c);
    device_create_file(device, &dev_attr_status_i2c);
    device_create_file(device, &dev_attr_slave_en);
    device_create_file(device, &dev_attr_i2c_GPIO_mode);
    device_create_file(device, &dev_attr_dev_info);
}

void rtk_i2c_remove_attr(struct device *device)
{
    device_remove_file(device, &dev_attr_fixed_speed);
    device_remove_file(device, &dev_attr_disable_auto_recovery);
    device_remove_file(device, &dev_attr_polling_mode);
    device_remove_file(device, &dev_attr_recovery);
    device_remove_file(device, &dev_attr_reset_clock);
    device_remove_file(device, &dev_attr_i2c_loglevel);
    device_remove_file(device, &dev_attr_run_read);
    device_remove_file(device, &dev_attr_port);
    device_remove_file(device, &dev_attr_disable_switch_port);
    device_remove_file(device, &dev_attr_speed);
    device_remove_file(device, &dev_attr_bus_status);
    device_remove_file(device, &dev_attr_bus_jam_recover);
    device_remove_file(device, &dev_attr_config);
    device_remove_file(device, &dev_attr_hold_time);
    device_remove_file(device, &dev_attr_setup_time);
    device_remove_file(device, &dev_attr_error_message_on_off);
    device_remove_file(device, &dev_attr_disable_i2c);
    device_remove_file(device, &dev_attr_status_i2c);
    device_remove_file(device, &dev_attr_slave_en);
    device_remove_file(device, &dev_attr_i2c_GPIO_mode);
    device_remove_file(device, &dev_attr_dev_info);
}
