#include <rtk_kdriver/rtk_pwm-reg.h>
#include <rtk_kdriver/rtk_pwm_crt.h>
#include <rtk_kdriver/rtk_pwm_func.h>
#include <rtk_kdriver/rtk_pwm.h>
#include "rtk_pwm_func_tv002.h"

#define LOCK_RTK_PWM(a,b)    spin_lock_irqsave(a, b)
#define UNLOCK_RTK_PWM(a, b) spin_unlock_irqrestore(a, b)

static DEFINE_SPINLOCK(pwm_lock);

/* ------------------------------------
 * Func : rtk_pwm_backlight_set_freq_ex
 * Desc : set pwm frequency to register
 * Param: pcbname: setting pwm pin name
 *        freq: settig pwm frequency value
 * Retrun: 0: Set pwm frequency success.
 *        -1: Set pwm frequency failed.
 * ------------------------------------*/
int rtk_pwm_backlight_set_freq_ex(const char *pcbname, int freq )
{
    unsigned long flags = 0;
    R_CHIP_T *pchip2 = NULL;

    LOCK_RTK_PWM(&pwm_lock, flags);
    pchip2 = rtk_pwm_chip_get_by_name((char *)pcbname);
    if(pchip2 == NULL){
        PWM_ERR("Get pchip2 NuLL!!\n");
        goto failed;
    }
    pchip2->rtk_freq = freq;
    pchip2->rtk_freq_100times = pchip2->rtk_freq * 100;
    rtk_pwm_freq_w(pchip2);
    rtk_pwm_duty_w(pchip2,pchip2->rtk_duty);//If frequency change, duty need re-calculate
    rtk_pwm_totalcnt_w(pchip2,pchip2->rtk_totalcnt);

    if( m_ioctl_printk_get() > 0 ){

        PWM_WARN("%s %s set pwm[%d], freq=%d\n",
                __func__ ,
                pcbname,
                pchip2->index,
                pchip2->rtk_freq);
    }
    UNLOCK_RTK_PWM(&pwm_lock, flags);
    return 0;

failed:
    UNLOCK_RTK_PWM(&pwm_lock, flags);
    return (-1);
}
EXPORT_SYMBOL(rtk_pwm_backlight_set_freq_ex);

/* ------------------------------------
 * Func : rtk_pwm_backlight_set_vsync_ex
 * Desc : set pwm vsync to register
 * Param: pcbname: setting pwm pin name
 *        freq: settig pwm vsync value
 * Retrun: 0: Set pwm vsync success.
 *        -1: Set pwm vsync failed.
 * ------------------------------------*/
int rtk_pwm_backlight_set_vsync_ex(const char *pcbname, int vsync )
{
    unsigned long flags = 0;
    R_CHIP_T *pchip2 = NULL;

    LOCK_RTK_PWM(&pwm_lock, flags);
    pchip2 = rtk_pwm_chip_get_by_name((char *)pcbname);
    if(pchip2 == NULL){
        PWM_ERR("Get pchip2 NuLL!!\n");
        goto failed;
    }
    pchip2->rtk_vsync = vsync;
    rtk_pwm_vsync_w(pchip2,pchip2->rtk_vsync);

    if( m_ioctl_printk_get() > 0 ){

        PWM_WARN("%s %s set pwm[%d], vsync=%d\n",
                __func__ ,
                pcbname,
                pchip2->index,
                pchip2->rtk_vsync);
    }
    UNLOCK_RTK_PWM(&pwm_lock, flags);
    return 0;

failed:
    UNLOCK_RTK_PWM(&pwm_lock, flags);
    return (-1);
}
EXPORT_SYMBOL(rtk_pwm_backlight_set_vsync_ex);

/* ------------------------------------
 * Func : rtk_pwm_backlight_set_vsync_delay_ex
 * Desc : set pwm vsync_delay to register
 * Param: pcbname: setting pwm pin name
 *        freq: settig pwm vsync_delay value
 * Retrun: 0: Set pwm vsync_delay success.
 *        -1: Set pwm vsync_delay failed.
 * ------------------------------------*/
int rtk_pwm_backlight_set_vsync_delay_ex(const char *pcbname, int mode,int vsync_delay)
{
    unsigned long flags = 0;
    R_CHIP_T *pchip2 = NULL;

    LOCK_RTK_PWM(&pwm_lock, flags);
    pchip2 = rtk_pwm_chip_get_by_name((char *)pcbname);
    if(pchip2 == NULL){
        PWM_ERR("Get pchip2 NuLL!!\n");
        goto failed;
    }
    if(mode == PWM_POS_DELAY){ //Delay unit depend on pwm totalcnt
        pchip2->rtk_pos_start = vsync_delay;
        get_v_delay(pchip2);
        rtk_pwm_vs_delay_w(pchip2,pchip2->rtk_pos_start_clk);
    }
    else if(mode == PWM_CLK_DELAY){ //Delay unit depend on pwm clock cycle
        pchip2->rtk_pos_start_clk = vsync_delay;
        pchip2->rtk_pos_start = rtk_get_pos(pchip2,pchip2->rtk_pos_start_clk,pchip2->rtk_freq);
        rtk_pwm_vs_delay_w(pchip2,pchip2->rtk_pos_start_clk);
    }

    if( m_ioctl_printk_get() > 0 ){

        PWM_WARN("%s %s set pwm[%d], vsync_delay=%d pos_clk=%d\n",
                __func__ ,
                pcbname,
                pchip2->index,
                pchip2->rtk_pos_start,
                pchip2->rtk_pos_start_clk);
    }
    UNLOCK_RTK_PWM(&pwm_lock, flags);
    return 0;

failed:
    UNLOCK_RTK_PWM(&pwm_lock, flags);
    return (-1);
}
EXPORT_SYMBOL(rtk_pwm_backlight_set_vsync_delay_ex);


/****************************************
 * Func : rtk_pwm_backlight_set_duty_ex
 * Desc : set pwm duty to register
 * Param: pcbname: setting pwm pin name
 *        duty: settig pwm duty value
 * Retrun: 0: Set pwm duty success.
 *        -1: Set pwm duty failed.
 * ------------------------------------*/
int rtk_pwm_backlight_set_duty_ex(const char *pcbname, int duty )
{
    unsigned long flags = 0;
    R_CHIP_T *pchip2 = NULL;

    LOCK_RTK_PWM(&pwm_lock, flags);
    pchip2 = rtk_pwm_chip_get_by_name((char *)pcbname);
    if(pchip2 == NULL){
        PWM_ERR("Get pchip2 NuLL!!\n");
        goto failed;
    }

    pchip2->rtk_duty = duty;
    if(duty == 0){
        if(pchip2->rtk_polarity == 1)
            rtk_pwm_force_mode_w(pchip2,PWM_FORCE_HIGH);
        else
            rtk_pwm_force_mode_w(pchip2,PWM_FORCE_LOW);
    }
    else{
        rtk_pwm_force_mode_w(pchip2,PWM_NON_FORCE);
        rtk_pwm_duty_w(pchip2,pchip2->rtk_duty);
    }

    if( m_ioctl_printk_get() > 0 ){

        PWM_WARN("%s %s set pwm[%d], duty=%d\n",
                __func__ ,
                pcbname,
                pchip2->index,
                pchip2->rtk_duty);
    }
    UNLOCK_RTK_PWM(&pwm_lock, flags);
    return 0;

failed:
    UNLOCK_RTK_PWM(&pwm_lock, flags);
    return (-1);
}
EXPORT_SYMBOL(rtk_pwm_backlight_set_duty_ex);


/* ------------------------------------
 * Func : rtk_pwm_backlight_apply
 * Desc : Double Buffer write apply
 * Param: pcbname: setting pwm pin name
 *
 * Retrun: 0: Set DB apply success.
 *        -1: Set DB apply failed.
 * ------------------------------------*/
int rtk_pwm_backlight_apply(const char *pcbname)
{
    unsigned long flags = 0;
    R_CHIP_T *pchip2 = NULL;

    LOCK_RTK_PWM(&pwm_lock, flags);
    pchip2 = rtk_pwm_chip_get_by_name((char *)pcbname);
    if(pchip2 == NULL){
        PWM_ERR("Get pchip2 NuLL!!\n");
        goto failed;
    }
    rtk_pwm_db_wb(pchip2);
    UNLOCK_RTK_PWM(&pwm_lock, flags);
    return 0;

failed:
    UNLOCK_RTK_PWM(&pwm_lock, flags);
    return (-1);
}
EXPORT_SYMBOL(rtk_pwm_backlight_apply);
