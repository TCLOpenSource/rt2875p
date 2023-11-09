#include <linux/types.h>
#include <linux/platform_device.h>
#include <rtk_kdriver/rtk_pwm.h>
#include "rtk_pwm_func.h"
#include <rbus/misc_pwm_reg.h>
#include <tvscalercontrol/panel/panelapi.h>
#include "rtk_pwm_monitor.h"
#include <linux/slab.h>

ssize_t rtk_pwm_show_param(struct device *dev, struct device_attribute *attr,
                        char *buf);
ssize_t rtk_pwm_store_param(struct device *dev, struct device_attribute *attr,
                        const char *buf, size_t count);

static DEVICE_ATTR(run,          S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(duty,         S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(vsync,        S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(freq,         S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(polarity,     S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(adapt_freq,   S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(adapt_en,     S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(apply,        S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(pos,          S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(disable_ioctl,S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(status,       S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(printk_ioctl, S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(ioctlCounter, S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(panel,        S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(clock,        S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(force_mode,   S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(mode,         S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(track,        S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(backlight,    S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(force_update, S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(dumpallregister,    S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(dump_pinmux,        S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);
static DEVICE_ATTR(output_monitor, S_IWUSR|S_IWGRP|S_IRUGO, rtk_pwm_show_param, rtk_pwm_store_param);

int rtk_PWM_register_dump(char *buf)
{
    int ret = 0;
    ret = snprintf(buf, PAGE_SIZE, "MISC_PWM_PWM_DB0_CTRL_reg         (0x%x) => 0x%08x\n", MISC_PWM_PWM_DB0_CTRL_reg, rtd_inl(MISC_PWM_PWM_DB0_CTRL_reg));
    buf += ret;
    ret = snprintf(buf, PAGE_SIZE, "MISC_PWM_PWM_DB1_CTRL_reg         (0x%x) => 0x%08x\n", MISC_PWM_PWM_DB1_CTRL_reg, rtd_inl(MISC_PWM_PWM_DB1_CTRL_reg));
    buf += ret;
    // PWM0
    ret = snprintf(buf, PAGE_SIZE, "MISC_PWM_PWM0_CTRL_reg            (0x%x) => 0x%08x\n", MISC_PWM_PWM0_CTRL_reg, rtd_inl(MISC_PWM_PWM0_CTRL_reg));
    buf += ret;
    ret = snprintf(buf, PAGE_SIZE, "MISC_PWM_PWM0_Timing_CTRL_reg     (0x%x) => 0x%08x\n", MISC_PWM_PWM0_Timing_CTRL_reg, rtd_inl(MISC_PWM_PWM0_Timing_CTRL_reg));
    buf += ret;
    ret = snprintf(buf, PAGE_SIZE, "MISC_PWM_PWM0_DUTY_SET_reg        (0x%x) => 0x%08x\n", MISC_PWM_PWM0_DUTY_SET_reg, rtd_inl(MISC_PWM_PWM0_DUTY_SET_reg));
    buf += ret;
    ret = snprintf(buf, PAGE_SIZE, "MISC_PWM_PWM0_DVS_PERIOD_reg      (0x%x) => 0x%08x\n", MISC_PWM_PWM0_DVS_PERIOD_reg, rtd_inl(MISC_PWM_PWM0_DVS_PERIOD_reg));
    buf += ret;
    // PWM1
    ret = snprintf(buf, PAGE_SIZE, "MISC_PWM_PWM1_CTRL_reg            (0x%x) => 0x%08x\n", MISC_PWM_PWM1_CTRL_reg, rtd_inl(MISC_PWM_PWM1_CTRL_reg));
    buf += ret;
    ret = snprintf(buf, PAGE_SIZE, "MISC_PWM_PWM1_Timing_CTRL_reg     (0x%x) => 0x%08x\n", MISC_PWM_PWM1_Timing_CTRL_reg, rtd_inl(MISC_PWM_PWM1_Timing_CTRL_reg));
    buf += ret;
    ret = snprintf(buf, PAGE_SIZE, "MISC_PWM_PWM1_DUTY_SET_reg        (0x%x) => 0x%08x\n", MISC_PWM_PWM1_DUTY_SET_reg, rtd_inl(MISC_PWM_PWM1_DUTY_SET_reg));
    buf += ret;
    ret = snprintf(buf, PAGE_SIZE, "MISC_PWM_PWM1_DVS_PERIOD_reg      (0x%x) => 0x%08x\n", MISC_PWM_PWM1_DVS_PERIOD_reg, rtd_inl(MISC_PWM_PWM1_DVS_PERIOD_reg));
    buf += ret;
    return ret;
}

ssize_t rtk_pwm_show_param(struct device *dev, struct device_attribute *attr,
                char *buf)
{
    unsigned long long param = 0;
    int ret = 0;
    R_CHIP_T *pchip2 = NULL;
    if (dev->platform_data != NULL) {
        pchip2 = (R_CHIP_T *) dev->platform_data;
        if (pchip2->mapped == false) {
            PWM_WARN("[PWM]Warning! This PWM isn't initialize.\n");
            return 0;
        }
    }

    if(pchip2 == NULL){
        PWM_ERR("Get pchip2 NuLL!!\n");
        return 0;
    }
    if (pcb_mgr_get_enum_info_byname("PIN_BL_ON_OFF", &param) != 0){
        PWM_WARN("%s PIN_BL_ON_OFF is not existed , please check pcb parameters = %llx \n", __func__, param);
        return (-1);
    }

    if (strncmp(attr->attr.name, "run", 3) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%d\n", pchip2->rtk_enable);
    } else if (strncmp(attr->attr.name, "duty", 4) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%d\n", pchip2->rtk_duty);
    } else if (strncmp(attr->attr.name, "output_monitor", strlen("output_monitor")) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%d\n", m_output_monitor_enable_get());
    } else if (strncmp(attr->attr.name, "vsync", 5) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%d\n", pchip2->rtk_vsync);
    } else if (strncmp(attr->attr.name, "freq", 4) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%d\n", pchip2->rtk_freq);
    } else if (strncmp(attr->attr.name, "polarity", 8) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%d\n", pchip2->rtk_polarity);
    } else if (strncmp(attr->attr.name, "pos", 3) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "(%d, %08x)\n", pchip2->rtk_pos_start,pchip2->rtk_pos_start_clk);
    } else if (strncmp(attr->attr.name, "frdvp", 5) == 0) {
        ret = 0;
    } else if (strncmp(attr->attr.name, "adapt_freq", 10) == 0) {
        pchip2->rtk_adapt_freq =rtk_pwm_get_adapt_freq(pchip2);
        ret = snprintf(buf, PAGE_SIZE, "%d\n", pchip2->rtk_adapt_freq);
    } else if (strncmp(attr->attr.name, "adapt_en", 8) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "rtk_adpt_en:%d\n", pchip2->rtk_adpt_en);
    } else if (strncmp(attr->attr.name, "disable_ioctl", strlen("disable_ioctl")) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%d\n", m_ioctl_disable_get());
    } else if (strncmp(attr->attr.name, "printk_ioctl", strlen("printk_ioctl")) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%d\n", m_ioctl_printk_get());
    } else if (strncmp(attr->attr.name, "ioctlCounter", strlen("ioctlCounter")) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%u\n", m_ioctl_counter_get());
    } else if (strncmp(attr->attr.name, "clock", strlen("clock")) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%d\n", pchip2->rtk_freq_range );
    } else if (strncmp(attr->attr.name, "backlight", strlen("backlight")) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%d\n", rtk_pwm_backlight_status() );
    } else if (strncmp(attr->attr.name, "force_update", strlen("force_update")) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%d\n", pchip2->rtk_force_update );
    } else if (strncmp(attr->attr.name, "force_mode", 10) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%d (0: non-force mode; 1: force PWM0 always 0; 2: force PWM0 always 1;)\n", pchip2->force_mode);
    } else if (strncmp(attr->attr.name, "mode", strlen("mode")) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "%s\n", (pchip2->currentMode == PWM_MODE) ? "PWM_MODE":"GPIO_MODE");
    } else if (strncmp(attr->attr.name, "panel", strlen("panel")) == 0) {
        if( rtk_pwm_panel_mode_get() == PWM_DRIVING_LED_CURRENT)
            ret = snprintf(buf, PAGE_SIZE, "%s\n", "LED_CURRENT" );
        else if( rtk_pwm_panel_mode_get() == PWM_DRIVING_2CH_PHASE_SAME)
            ret = snprintf(buf, PAGE_SIZE, "%s\n", "2CH_PHASE_SAME" );
        else if( rtk_pwm_panel_mode_get() == PWM_DRIVING_2CH_PHASE_DIFF)
            ret = snprintf(buf, PAGE_SIZE, "%s\n", "2CH_PHASE_DIFF" );
        else
            ret = snprintf(buf, PAGE_SIZE, "%s\n", "No this Panel type" );
    }else if (strncmp(attr->attr.name, "dumpallregister", strlen("dumpallregister")) == 0) {
        char *buf_info = kmalloc(PAGE_SIZE, GFP_KERNEL);
        if(buf_info == NULL) return 0;
        buf_info[0] = 0;
        rtk_PWM_register_dump(buf_info);
        ret = snprintf(buf, PAGE_SIZE, "%s\n", buf_info);
        kfree(buf_info);
    }else if (strncmp(attr->attr.name, "dump_pinmux", strlen("dump_pinmux")) == 0) {
        ret = snprintf(buf, PAGE_SIZE, "pin_mux_addr: 0x%8lx[%d:%d]\n",
                pchip2->pin_mux_info.addr,
                pchip2->pin_mux_info.mask_h,
                pchip2->pin_mux_info.mask_l);
    }else if (strncmp(attr->attr.name, "status", strlen("status")) == 0) {
        pchip2->rtk_adapt_freq =rtk_pwm_get_adapt_freq(pchip2);

	ret = snprintf(buf, PAGE_SIZE, "[%s] en=%d, freq=%d Hz, duty=%d, polarity=%d, vsync_rst=%d, (pos=%d,%08x), force_mode=%d, adap_freq=%d, adap_en=%d, backlight=%d (gpio_type=%s, gpio_num=%d)\n",
                pchip2->nodename,
                pchip2->rtk_enable,
                pchip2->rtk_freq,
                pchip2->rtk_duty,
                pchip2->rtk_polarity,
                pchip2->rtk_vsync,
                pchip2->rtk_pos_start,
                pchip2->rtk_pos_start_clk,
                pchip2->force_mode,
                pchip2->rtk_adapt_freq,
                pchip2->rtk_adpt_en,
                rtk_pwm_backlight_status(),
                (GET_PIN_TYPE(param) == 4)? "ISO_GPIO" : "MIS_GPIO",
                GET_PIN_INDEX(param));
    }

    return ret;
}

ssize_t rtk_pwm_store_param(struct device *dev, struct device_attribute *attr,
                const char *buf, size_t count)
{
    int val = 0;
    R_CHIP_T *pchip2 = NULL;

    if (dev->platform_data != NULL) {
        pchip2 = (R_CHIP_T *) dev->platform_data;
        if (pchip2->mapped == false)
            return 0;
    }

    if(pchip2 == NULL){
        PWM_ERR("Get pchip2 NuLL!!\n");
        return 0;
    }

    sscanf(buf, "%d\n", &val);

    if (strncmp(attr->attr.name, "disable_ioctl", strlen("disable_ioctl")) == 0) {

        if (val == 0 || val == 1)
            m_ioctl_disable_set(val);
        else
            PWM_WARN("PWM disable Fail!\n");

    }else if (strncmp(attr->attr.name, "printk_ioctl", strlen("printk_ioctl")) == 0) {

        if (val == 0 || val == 1)
            m_ioctl_printk_set(val);
        else
            PWM_WARN("PWM printk ioctrl Fail!\n");

    }else if (strncmp(attr->attr.name, "output_monitor", strlen("output_monitor")) == 0) {

        if (val == 0 || val == 1){
            m_output_monitor_enable_set(pchip2, val);
        }
        else
            PWM_WARN("PWM output monitor Fail!\n");

    }else if (strncmp(attr->attr.name, "ioctlCounter", strlen("ioctlCounter")) == 0) {

        m_ioctl_counter_set(val);
        PWM_WARN("set ioctlCounter to %u \n" , m_ioctl_counter_get() );

    }else if (strncmp(attr->attr.name, "run", 3) == 0) {

        if (val == 0 || val == 1)
            pchip2->rtk_enable = val;
        else
            PWM_WARN("PWM run Fail!\n");

    } else if (strncmp(attr->attr.name, "duty", 4) == 0) {

        if (val <= 0)
            val = 0;
        else if (val >= pchip2->rtk_duty_max)
            val = pchip2->rtk_duty_max;

        pchip2->rtk_duty = val;

    } else if (strncmp(attr->attr.name, "vsync", 5) == 0) {

        if (val == 0 || val == 1)
            pchip2->rtk_vsync = val;
        else
            PWM_WARN("PWM Write vsync Fail!\n");

    } else if (strncmp(attr->attr.name, "freq", 4) == 0) {

        pchip2->rtk_freq = val;
        pchip2->rtk_freq_100times = pchip2->rtk_freq * 100;

        if(pchip2->rtk_clk_sel == D_CLK_DIV4) {
        #if IS_ENABLED(CONFIG_SUPPORT_SCALER)
            pchip2->vt = pwm_get_disp_dclk()/(pwm_get_disp_htotal()*(pchip2->rtk_freq/2));
            pchip2->vt *= 100;
            PWM_INFO("%s v_total: %x\n", __func__, pchip2->vt);
        #endif
        }

    } else if (strncmp(attr->attr.name, "polarity", 8) == 0) {

        if (val == 0 || val == 1)
            pchip2->rtk_polarity = val;
        else
            PWM_WARN("PWM Write polarity Fail!\n");

    } else if (strncmp(attr->attr.name, "adapt_freq", 10) == 0) {

        pchip2->rtk_adapt_freq =rtk_pwm_get_adapt_freq(pchip2);
    } else if (strncmp(attr->attr.name, "adapt_en", 8) == 0) {
        if (val == 0 || val == 1)
            pchip2->rtk_adpt_en = val;
        else
            PWM_WARN("PWM Write adapt_freq_en Fail!\n");
    } else if (strncmp(attr->attr.name, "force_update", 12) == 0) {
        if (val == 0 || val == 1)
            pchip2->rtk_force_update = val;
        else
            PWM_WARN("PWM Write force_update Fail!\n");
    } else if (strncmp(attr->attr.name, "pos", 3) == 0) {

        if (val >=0 && val <=255) {
            pchip2->rtk_pos_start = val;
            get_v_delay(pchip2);
        }else{
            PWM_WARN("PWM Write pos Fail!\n");
        }
    } else if (strncmp(attr->attr.name, "clock", 5) == 0) {

        if( val >= CLK_27MHZ_8BIT && val <= CLK_DIV4_12BIT)
            pchip2->rtk_freq_range = val;
        else
            PWM_WARN("PWM clock source setting Fail!\n");
    } else if (strncmp(attr->attr.name, "force_mode", 10) == 0) {

        if (val == 0 || val == 1 || val == 2) {
            pchip2->force_mode = val;
            rtk_pwm_force_mode_w(pchip2, pchip2->force_mode);
        }
        else
            PWM_WARN("PWM change force_mode Fail!\n");
    } else if (strncmp(attr->attr.name, "mode", 4) == 0) {

        if (val == PWM_MODE || val == GPIO_MODE)
            pchip2->currentMode = val;
        else
            PWM_WARN("PWM change mode Fail!\n");
    } else if (strncmp(attr->attr.name, "backlight", 9) == 0) {
        if (val == 1 || val == 0) {
            PWM_WARN("PWM set backlight !\n");
            rtk_pwm_backlight_enable(val);
        }
        else {
            PWM_WARN("PWM set backlight Fail!\n");
        }
    }else if (strncmp(attr->attr.name, "track", strlen("track")) == 0) {
        if(val == 1){
            rtk_pwm_enable_tracking_mode(pchip2->index);
            PWM_WARN("[%d]Single Vsync Tracking Enable\n",pchip2->index);
        }
        else if (val == 2){
            rtk_pwm_enable_tracking_mode(0);
            rtk_pwm_enable_tracking_mode(1);
            PWM_WARN("ALL Vsync Tracking Enable\n");
        }
        else{
            rtk_pwm_disable_tracking_mode(pchip2->index);
            PWM_WARN("ALL Vsync Tracking Stop\n");
        }
    } else if (strncmp(attr->attr.name, "apply", 5) == 0) {
        if (val == 1) {
            PWM_WARN(" apply  start\n");
            pchip2->rtk_freq_changed = true;
            /*set clock source */
            rtk_pwm_clock_freq_set(pchip2);
            rtk_pwm_clock_source(pchip2);

            get_v_delay(pchip2);

            /*set timing ctrl , freq */
            rtk_pwm_freq_w(pchip2);

            /*set duty */
            if(pchip2->rtk_duty == 0 && PWM_MISC_TYPE(pchip2)){
                if(pchip2->rtk_polarity == 1)
                    rtk_pwm_force_mode_w(pchip2,PWM_FORCE_HIGH);
                else
                    rtk_pwm_force_mode_w(pchip2,PWM_FORCE_LOW);
            }
            else{
                 rtk_pwm_force_mode_w(pchip2,PWM_NON_FORCE);
                 rtk_pwm_duty_w(pchip2,pchip2->rtk_duty);
            }
            rtk_pwm_totalcnt_w(pchip2,pchip2->rtk_totalcnt);

            /*set CTRL */
            rtk_pwm_ctrl_polarity_w(pchip2,pchip2->rtk_polarity);
            rtk_pwm_output_w(pchip2,pchip2->rtk_enable);

            if(pchip2->rtk_vsync == 1)
                rtk_pwm_cycle_max_w(pchip2,2);
            else
                rtk_pwm_cycle_max_w(pchip2,0);
            rtk_pwm_db_wb(pchip2);

            rtk_pwm_db_enable(pchip2,pchip2->rtk_vsync);
            rtk_pwm_dvs_mux_w(pchip2,pchip2->rtk_vsync);
            rtk_pwm_vsync_w(pchip2,pchip2->rtk_vsync);

            /*vsync delay*/
            if(pchip2->rtk_pos_start > 0 && pchip2->rtk_vsync) {
                rtk_pwm_vs_delay_w(pchip2,pchip2->rtk_pos_start_clk);
            }else{
                rtk_pwm_vs_delay_w(pchip2,0);
            }

            if(pchip2->currentMode == GPIO_MODE){
                if(pchip2->rtk_duty == pchip2->rtk_duty_max)
                    rtk_change_to_gpio_mode(pchip2,GPIO_PIN_HIGH);
                else if(pchip2->rtk_duty == 0)
                    rtk_change_to_gpio_mode(pchip2,GPIO_PIN_LOW);
            }else if(pchip2->currentMode == PWM_MODE){
                rtk_change_to_pwm_mode(pchip2);
            }

            rtk_pwm_db_wb(pchip2);

        }
    }

    return count;
}

void rtk_pwm_create_attr(struct device *device)
{
    device_create_file(device,&dev_attr_run);
    device_create_file(device,&dev_attr_duty);
    device_create_file(device,&dev_attr_freq);
    device_create_file(device,&dev_attr_vsync);
    device_create_file(device,&dev_attr_polarity);
    device_create_file(device,&dev_attr_adapt_freq);
    device_create_file(device,&dev_attr_adapt_en);
    device_create_file(device,&dev_attr_pos);
    device_create_file(device,&dev_attr_apply);
    device_create_file(device,&dev_attr_disable_ioctl);
    device_create_file(device,&dev_attr_status);
    device_create_file(device,&dev_attr_printk_ioctl);
    device_create_file(device,&dev_attr_ioctlCounter);
    device_create_file(device,&dev_attr_panel);
    device_create_file(device,&dev_attr_clock);
    device_create_file(device,&dev_attr_force_mode);
    device_create_file(device,&dev_attr_mode);
    device_create_file(device,&dev_attr_track);
    device_create_file(device,&dev_attr_backlight);
    device_create_file(device,&dev_attr_force_update);
    device_create_file(device,&dev_attr_dumpallregister);
    device_create_file(device,&dev_attr_dump_pinmux);
    device_create_file(device,&dev_attr_output_monitor);
}

void rtk_pwm_remove_attr(struct device *device)
{
    device_remove_file(device, &dev_attr_run);
    device_remove_file(device, &dev_attr_duty);
    device_remove_file(device, &dev_attr_freq);
    device_remove_file(device, &dev_attr_vsync);
    device_remove_file(device, &dev_attr_polarity);
    device_remove_file(device, &dev_attr_adapt_freq);
    device_remove_file(device, &dev_attr_adapt_en);
    device_remove_file(device, &dev_attr_pos);
    device_remove_file(device, &dev_attr_apply);
    device_remove_file(device, &dev_attr_disable_ioctl);
    device_remove_file(device, &dev_attr_status);
    device_remove_file(device, &dev_attr_printk_ioctl);
    device_remove_file(device, &dev_attr_ioctlCounter);
    device_remove_file(device, &dev_attr_panel);
    device_remove_file(device, &dev_attr_clock);
    device_remove_file(device, &dev_attr_force_mode);
    device_remove_file(device, &dev_attr_mode);
    device_remove_file(device, &dev_attr_track);
    device_remove_file(device, &dev_attr_backlight);
    device_remove_file(device, &dev_attr_force_update);
    device_remove_file(device, &dev_attr_dumpallregister);
    device_remove_file(device, &dev_attr_dump_pinmux);
    device_remove_file(device, &dev_attr_output_monitor);
}
