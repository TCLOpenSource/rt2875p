#include "rtk_pwm_monitor.h"
#include <rtk_kdriver/rtk_pwm.h>
#include <rtk_kdriver/rtk_gpio.h>
#include <linux/interrupt.h>
#include <linux/sched.h>   //wake_up_process()
#include <linux/kthread.h> //kthread_create(), kthread_run()
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/freezer.h>
#include <linux/version.h>

#define TIMEOUT_VAL		2
#define MAX_SAMP_NUM		10
#define SAMP_1			(MAX_SAMP_NUM - 3)
#define SAMP_2			(MAX_SAMP_NUM - 2)
#define SAMP_3			(MAX_SAMP_NUM - 1)

typedef struct rtk_pwm_monitor_reg_t{
    int T;
    int freq;
    int duty[2];
    int duty_err[2];
    int last_freq;
    int last_duty;
    int timeout_flag;
    char *gpio_val;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
    struct timespec64 wait_tv[2];
#else
    struct timeval wait_tv[2];
#endif
}rtk_pwm_monitor_reg;

volatile static int m_output_monitor_enable = 0;

//======================================================================
//rtk_pwm_output_monitor_isr
//======================================================================
volatile int m_monitor_isr_cnt = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
struct timespec64 m_monitor_tv[MAX_SAMP_NUM] = {{0}};
#else
struct timeval m_monitor_tv[MAX_SAMP_NUM] = {{0}};
#endif
void rtk_pwm_output_monitor_isr(RTK_GPIO_ID gid, unsigned char assert, void *dev_id)
{
    if (m_monitor_isr_cnt < MAX_SAMP_NUM)
    {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
    	ktime_get_real_ts64(&m_monitor_tv[m_monitor_isr_cnt++]);
#else
        do_gettimeofday(&m_monitor_tv[m_monitor_isr_cnt++]);
#endif
    }
}

//======================================================================
//rtk_pwm_output_monitor
//======================================================================
void rtk_pwm_output_monitor(R_CHIP_T *pchip2, rtk_pwm_monitor_reg *monitor_reg)
{
    m_monitor_isr_cnt = 0;
    monitor_reg->timeout_flag = 0;
    do
    {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        ktime_get_real_ts64(&monitor_reg->wait_tv[0]);
#else
        do_gettimeofday(&monitor_reg->wait_tv[0]);
#endif
        rtk_gpio_set_irq_enable(pchip2->gid, 1);
        while (m_monitor_isr_cnt < MAX_SAMP_NUM)
        {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        	ktime_get_real_ts64(&monitor_reg->wait_tv[1]);
#else
            do_gettimeofday(&monitor_reg->wait_tv[1]);
#endif
            if ((monitor_reg->wait_tv[1].tv_sec - monitor_reg->wait_tv[0].tv_sec) > TIMEOUT_VAL)
            {
                monitor_reg->timeout_flag = 1;
                break;
            }
        }
        rtk_gpio_set_irq_enable(pchip2->gid, 0);
        m_monitor_isr_cnt = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        monitor_reg->T = (m_monitor_tv[SAMP_3].tv_nsec - m_monitor_tv[SAMP_1].tv_nsec)/1000;
#else
        monitor_reg->T = m_monitor_tv[SAMP_3].tv_usec - m_monitor_tv[SAMP_1].tv_usec;
#endif
    }while ((0 >= monitor_reg->T) && (0 == monitor_reg->timeout_flag));

    if (0 == monitor_reg->timeout_flag)
    {
        monitor_reg->freq = 1000000/monitor_reg->T;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        monitor_reg->duty[0] = (255 * ((m_monitor_tv[SAMP_3].tv_nsec - m_monitor_tv[SAMP_2].tv_nsec)/1000)/monitor_reg->T) + 1;
        monitor_reg->duty[1] = (255 * ((m_monitor_tv[SAMP_2].tv_nsec - m_monitor_tv[SAMP_1].tv_nsec)/1000)/monitor_reg->T) + 1;
#else
        monitor_reg->duty[0] = (255 * (m_monitor_tv[SAMP_3].tv_usec - m_monitor_tv[SAMP_2].tv_usec)/monitor_reg->T) + 1;
        monitor_reg->duty[1] = (255 * (m_monitor_tv[SAMP_2].tv_usec - m_monitor_tv[SAMP_1].tv_usec)/monitor_reg->T) + 1;
#endif
        monitor_reg->duty_err[0] = (pchip2->rtk_duty > monitor_reg->duty[0]) ? (pchip2->rtk_duty - monitor_reg->duty[0]) : (monitor_reg->duty[0] - pchip2->rtk_duty);
        monitor_reg->duty_err[1] = (pchip2->rtk_duty > monitor_reg->duty[1]) ? (pchip2->rtk_duty - monitor_reg->duty[1]) : (monitor_reg->duty[1] - pchip2->rtk_duty);
        monitor_reg->duty[0] = monitor_reg->duty_err[0] > monitor_reg->duty_err[1] ? monitor_reg->duty[1] : monitor_reg->duty[0];
        PWM_ERR("[MONITOR] %s freq = %d(Hz), duty = %d/255\n", pchip2->nodename, monitor_reg->freq, monitor_reg->duty[0]);

        if ((monitor_reg->last_freq == monitor_reg->freq) && (monitor_reg->last_duty == monitor_reg->duty[0]))
        {
            mdelay(1000);
        }
        else
        {
            monitor_reg->last_freq = monitor_reg->freq;
            monitor_reg->last_duty = monitor_reg->duty[0];
        }
    }
    else if (1 == monitor_reg->timeout_flag)
    {
        monitor_reg->gpio_val = GET_PWM_INVERT(pchip2->ullPcbMgrValue) ? (rtk_gpio_input(pchip2->gid)?"LOW":"HIGH"):(rtk_gpio_input(pchip2->gid)?"HIGH":"LOW");
        PWM_ERR("[MONITOR] %s time out, output all %s\n", pchip2->nodename, monitor_reg->gpio_val);
    }

}

//======================================================================
//rtk_pwm_output_monitor_thread
//======================================================================
int rtk_pwm_output_monitor_thread(void *pchip)
{
    rtk_pwm_monitor_reg monitor_reg;
    R_CHIP_T *pchip2 = (R_CHIP_T *)pchip;

    memset(&monitor_reg, 0, sizeof(monitor_reg));

    set_freezable();

    while (!kthread_should_stop())
    {
        rtk_pwm_output_monitor(pchip2, &monitor_reg);
    }

    return 0;
}

//======================================================================
//m_output_monitor_enable_get
//======================================================================
int  m_output_monitor_enable_get(void)
{
    return m_output_monitor_enable;
}

//======================================================================
//m_output_monitor_enable_set
//======================================================================
int m_output_monitor_enable_set(R_CHIP_T *pchip2, int enable)
{
    int err = 0;
    static struct task_struct *output_monitor_task;
    m_output_monitor_enable = enable;

    if (1 == m_output_monitor_enable)
    {
        PWM_ERR("[%s] line:%d output_monitor_enable = %d\n", __func__, __LINE__, m_output_monitor_enable);
        if (INVALID_GPIO_GID == pchip2->gid)
        {
            PWM_ERR("[%s] gid invalid\n", __func__);
            return -1;
        }

        //pwm output monitor interrupt request
        rtk_gpio_set_dir(pchip2->gid, 0);
        rtk_gpio_set_debounce(pchip2->gid, RTK_GPIO_DEBOUNCE_37ns);
        rtk_gpio_set_irq_polarity(pchip2->gid, 0);
        rtk_gpio_request_irq(pchip2->gid, rtk_pwm_output_monitor_isr, "PWM_OUTPUT_MONITOR", rtk_pwm_output_monitor_isr);

        //create and wake up kthread
        output_monitor_task = kthread_create(rtk_pwm_output_monitor_thread, (void *)pchip2, "pwm_monitor_task");
        if (IS_ERR(output_monitor_task))
        {
            PWM_ERR("[%s] Unable to start monitor thread\n", __func__);
            err = PTR_ERR(output_monitor_task);
            output_monitor_task = NULL;
            rtk_gpio_free_irq(pchip2->gid, rtk_pwm_output_monitor_isr);
            return err;
        }

        wake_up_process(output_monitor_task);
        PWM_ERR("[%s] monitor start\n", __func__);
    }
    else
    {
        PWM_ERR("[%s] line:%d output_monitor_enable = %d\n", __func__, __LINE__, m_output_monitor_enable);
        if (output_monitor_task)
        {
            if (0 == kthread_stop(output_monitor_task))
            {
                PWM_ERR("[%s] monitor stop\n", __func__);
            }
            else
            {
                PWM_ERR("[%s] monitor stop error\n", __func__);
            }
            output_monitor_task = NULL;
        }

        if (1 == rtk_gpio_chk_irq_enable(pchip2->gid))
        {
            rtk_gpio_set_irq_enable(pchip2->gid, 0);
            PWM_ERR("[%s] disable irq\n", __func__);
        }

        rtk_gpio_free_irq(pchip2->gid, rtk_pwm_output_monitor_isr);
    }

    return 0;
}


