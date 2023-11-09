/* -------------------------------------------------------------------------
   irtx_rtk.c  irtx driver for Realtek rtk
   -------------------------------------------------------------------------
    Copyright (C) 2012 liangliang_song <liangliang_song@realtek.com.tw>
----------------------------------------------------------------------------*/
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/io.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_crt.h>
#include "rtk_irtx_priv.h"
#include "rtk_irtx_reg.h"

static rtk_irtx m_irtx;

static int rtk_irtx_set_pwm(unsigned int on_off, unsigned int freq, unsigned int duty)
{
    if(on_off)
    {
        unsigned int i = 0;
        unsigned int irtx_pwm_clksrc_div = 0;
        unsigned int irtx_pwm_clksrc_div_index = 0;
        unsigned int irtx_pwm_clk_div = 0;
        unsigned int irtx_duty = 0;

        if(freq > 625000)
            irtx_pwm_clksrc_div = (IRTX_BUS_CLK_REQ / 100) / freq;
        else
            irtx_pwm_clksrc_div = (IRTX_BUS_CLK_REQ / 200) / freq;

        for(i = 0; i < 16; i++)
        {
            if((1 << i) > irtx_pwm_clksrc_div)
            {
                irtx_pwm_clksrc_div_index  = i;
                break;
            }
        }

        if(irtx_pwm_clksrc_div_index < 2)
        {
            irtx_error("not support freq:%u\n", freq);
            return -1;
        }

        irtx_pwm_clksrc_div_index = irtx_pwm_clksrc_div_index -1;

        irtx_pwm_clk_div = (IRTX_BUS_CLK_REQ / (1 << irtx_pwm_clksrc_div_index)) / freq;
        if(!irtx_pwm_clk_div)
            irtx_pwm_clk_div = 1;

        irtx_duty =  (irtx_pwm_clk_div * duty) / 255;
        if(!irtx_duty)
            irtx_duty = 1;

        if(irtx_duty > irtx_pwm_clk_div)
            irtx_duty = irtx_pwm_clk_div;

        irtx_info("rtk_irtx_set_pwm:: irtx_duty = %d, irtx_pwm_clksrc_div_index = %d, irtx_pwm_clk_div = %d\n", irtx_duty, irtx_pwm_clksrc_div_index, irtx_pwm_clk_div);
        WRITE_REG(IRTX_PWM_CFG_REG, IRTX_PWM_CLKSRC_DIV(irtx_pwm_clksrc_div_index -1)
                  | IRTX_PWM_CLK_DUTY(irtx_duty) | IRTX_PWM_CLK_DIV(irtx_pwm_clk_div));

        WRITE_REG(IRTX_PWM_CTRL_REG, READ_REG(IRTX_PWM_CTRL_REG) | IRTX_PWM_EN(1));
    }
    else
    {
        WRITE_REG(IRTX_PWM_CTRL_REG, (READ_REG(IRTX_PWM_CTRL_REG) & (~IRTX_PWM_EN_MASK)) |IRTX_PWM_EN(0));
    }
    return 0;
}

static int rtk_irtx_set_timing_unit(unsigned int timing_unit)
{
    unsigned int div = 0;
    if(timing_unit < 4)
        timing_unit = 4;
    div = timing_unit / 4;
    irtx_info("rtk_irtx_set_timing_unit:: div = %d\n", div);
    WRITE_REG(IRTX_CLKDIV_REG,  IRTX_CLKDIV(div));
    return 0;
}

static inline void rtk_irtx_soft_reset(void)
{
    WRITE_REG(IRTX_EN_RST_REG, (READ_REG(IRTX_EN_RST_REG) & (~IRTX_RST_MASK)) | IRTX_RST(1));
    while((READ_REG(IRTX_EN_RST_REG) & IRTX_RST_MASK))
        ndelay(1);
}

static unsigned int rtk_irtx_write_timing_info(IRTX_CMD_BUFF *cmb)
{
    unsigned int cur_hw_fifo_index = 0;
    while((cmb->cur_timing_info_index < cmb->cmd.timing_info_len)
          && (cur_hw_fifo_index < IRTX_HW_FIFO_LEN))
    {
        unsigned int timing_info = cmb->cmd.timing_info[cmb->cur_timing_info_index];
        unsigned int timing_level = IRTX_GET_TIMING_LEVEL(timing_info);
        unsigned int timing_length = IRTX_GET_TIMING_LENGTH(timing_info);
        unsigned int seg_num = timing_length / EACH_TIMING_INFO_MAX_LEN;
        if(timing_length & EACH_TIMING_INFO_MAX_LEN)
            seg_num++;
        if((cur_hw_fifo_index + seg_num) > IRTX_HW_FIFO_LEN)
            break;
        while(timing_length > 0)
        {
            unsigned int tmp_timing_length = (timing_length > EACH_TIMING_INFO_MAX_LEN) ? EACH_TIMING_INFO_MAX_LEN : timing_length;
            //irtx_info("rtk_irtx_write_timing_info::%d, %d\n", timing_level, tmp_timing_length);
            WRITE_REG(IRTX_TI_CTRL_REG, IRTX_TI_BUF_IDX(cur_hw_fifo_index)
                      | IRTX_TI_BUF_WDATA_LEVEL(timing_level)
                      | IRTX_TI_BUF_WDATA_LENGTH(tmp_timing_length) | IRTX_TI_BUF_WR(1));
            while(READ_REG(IRTX_TI_CTRL_REG) & IRTX_TI_BUF_WR_MASK);
            cur_hw_fifo_index++;
            timing_length -= tmp_timing_length;
        }
        cmb->cur_timing_info_index++;
    }
    return cur_hw_fifo_index;
}


static void _cmb_tx_complete(IRTX_CMD_BUFF *cmb)
{
    if (cmb->flags & NONBLOCK)
    {
        irtx_kfree_cmb(cmb);
    }
    else
    {
        if (cmb->status == WAIT_XMIT)
            cmb->status = XMIT_ABORT;
        wake_up(&cmb->wq);
    }
}

void rtk_irtx_tx_work(struct work_struct *work)
{
    rtk_irtx *p_this =
        (rtk_irtx *) container_of(work, rtk_irtx, xmit.work.work);
    rtk_irtx_xmit *p_xmit = &p_this->xmit;
    unsigned long flags;
    unsigned int ret;

    spin_lock_irqsave(&p_this->lock, flags);

    if (p_xmit->state == XMIT)
    {
        ret = rtk_irtx_write_timing_info(p_xmit->cmb);
        if (ret != 0)
        {
            if (time_after(jiffies, p_xmit->timeout))
            {
                p_xmit->cmb->status = XMIT_TIMEOUT;
                _cmb_tx_complete(p_xmit->cmb);

                p_xmit->cmb = NULL;
                p_xmit->state = IDEL;

                irtx_warn("irtx tx timeout\n");

                cancel_delayed_work(&p_xmit->work);
            }
            else
            {
                WRITE_REG(IRTX_START_REG, IRTX_LEN(ret) |  IRTX_START(1));
            }
        }
        else
        {
            cancel_delayed_work(&p_xmit->work);

            p_xmit->cmb->status = XMIT_OK;
            irtx_dbg("irtx tx completed\n");
            _cmb_tx_complete(p_xmit->cmb);
            p_xmit->cmb = NULL;
            p_xmit->state = IDEL;
        }
    }

    if (p_xmit->state == IDEL)
    {
        if (p_xmit->enable)
        {
            p_xmit->cmb = irtx_cmb_dequeue(&p_this->tx_queue);

            if (p_xmit->cmb)
            {
                rtk_irtx_soft_reset();

                p_xmit->cmb->cur_timing_info_index = 0;

                if(p_xmit->cmb->cmd.pwm_en)
                    rtk_irtx_set_pwm(1, p_xmit->cmb->cmd.pwm_freq, p_xmit->cmb->cmd.pwm_duty);
                else
                    rtk_irtx_set_pwm(0, p_xmit->cmb->cmd.pwm_freq, p_xmit->cmb->cmd.pwm_duty);

                rtk_irtx_set_timing_unit(p_xmit->cmb->cmd.timing_unit);

                ret = rtk_irtx_write_timing_info(p_xmit->cmb);
                if(ret)
                    WRITE_REG(IRTX_START_REG, IRTX_LEN(ret) |  IRTX_START(1));

                p_xmit->state = XMIT;

                irtx_dbg("irtx tx start\n");
                /* queue delayed work for timeout detection */
                schedule_delayed_work(&p_xmit->work, IRTX_TX_TIMEOUT + 1);
            }
        }
    }

    spin_unlock_irqrestore(&p_this->lock, flags);
}


static void rtk_irtx_tx_start(rtk_irtx *p_this)
{
    unsigned long flags;
    spin_lock_irqsave(&p_this->lock, flags);

    if (!p_this->xmit.enable)
    {
        irtx_info("irtx tx start\n");
        p_this->xmit.enable = 1;
        p_this->xmit.state = IDEL;
        p_this->xmit.cmb = NULL;

        INIT_DELAYED_WORK(&p_this->xmit.work, rtk_irtx_tx_work);
    }

    spin_unlock_irqrestore(&p_this->lock, flags);
}

/*------------------------------------------------------------------
 * Func : rtk_irtx_tx_stop
 *
 * Desc : stop tx
 *
 * Parm : p_this : handle of rtk irtx
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static
void rtk_irtx_tx_stop(rtk_irtx *p_this)
{
    IRTX_CMD_BUFF *cmb;
    unsigned long flags;
    spin_lock_irqsave(&p_this->lock, flags);

    if (p_this->xmit.enable)
    {
        irtx_info("irtx tx stop\n");
        cancel_delayed_work(&p_this->xmit.work);

        if (p_this->xmit.cmb)
            _cmb_tx_complete(p_this->xmit.cmb);

        while ((cmb = irtx_cmb_dequeue(&p_this->tx_queue)))
            _cmb_tx_complete(cmb);

        p_this->xmit.enable = 0;
        p_this->xmit.state = IDEL;
        p_this->xmit.cmb = NULL;
    }
    spin_unlock_irqrestore(&p_this->lock, flags);
}

extern int rtk_irtx_xmit_message(rtk_irtx *p_this, IRTX_CMD_BUFF *cmb,
                                 unsigned long flags);

static
irqreturn_t rtk_irtx_isr(int this_irq, void *dev_id)
{
    rtk_irtx *p_this = (rtk_irtx *) dev_id;
    irqreturn_t ret = IRQ_NONE;

    if ((READ_REG(IRTX_INT_REG) & IRTX_DONE_INT_MASK))
    {
        WRITE_REG(IRTX_INT_REG, IRTX_DONE_INT_MASK);
        rtk_irtx_tx_work(&p_this->xmit.work.work);
        ret = IRQ_HANDLED;
    }
    return ret;
}

//TODO::Add IRTX CRT FLOW
static void  rtk_irtx_intial_hw_setting(void)
{
    CRT_CLK_OnOff(IRTX, CLK_OFF, NULL);
    CRT_CLK_OnOff(IRTX, CLK_ON, NULL);
    rtd_maskl(IRTX_PINMUX_REG, ~IRTX_PINMUX_MASK, IRTX_PINMUX_DATA);
}

static int rtk_irtx_set_global_interrupt(rtk_irtx *p_this, unsigned int onoff)
{
    irtx_info("%s onoff=%s \n", __FUNCTION__, onoff ? "enable" : "disable");
    if (onoff)
        WRITE_REG(IRTX_INT_EN, (IRTX_INT_EN_VAL | IRTX_INT_EN_WRITE_DATA));
    else
        WRITE_REG(IRTX_INT_EN, IRTX_INT_EN_VAL);
    return 0;
}



static int rtk_irtx_init(rtk_irtx *p_this)
{
    if (!p_this->status.init)
    {
        irtx_info("Open %s IRTX\n", IRTX_MODEL_NAME);

        rtk_irtx_intial_hw_setting();

        spin_lock_init(&p_this->lock);
        p_this->xmit.state = IDEL;
        p_this->xmit.cmb = NULL;
        p_this->xmit.timeout = 0;

        irtx_cmb_queue_head_init(&p_this->tx_queue);

        if ( request_irq (gic_irq_find_mapping(IRQ_IRTX),
                          rtk_irtx_isr, IRQF_SHARED, "IRTX", p_this) < 0)
        {
            irtx_warn("irtx : open %s irtx failed, unable to request irq#%u\n",
                      IRTX_MODEL_NAME, gic_irq_find_mapping(IRQ_IRTX));
            return -1;
        }

        irtx_warn("irtx : open %s  request irq#%u\n",
                  IRTX_MODEL_NAME, gic_irq_find_mapping(IRQ_IRTX));

        irtx_info("irtx : enable IRTX SCPU interrupt\n");
        rtk_irtx_set_global_interrupt(p_this, 1);
        p_this->status.init = 1;
    }

    return 0;
}


int rtk_irtx_set_mode(rtk_irtx *p_this, unsigned char mode)
{
    switch (mode)
    {
        case IRTX_MODE_OFF:
            irtx_info("%s irtx mode : OFF\n", IRTX_MODEL_NAME);

            WRITE_REG(IRTX_INT_EN_REG,
                      (READ_REG(IRTX_INT_EN_REG) & ~IRTX_DONE_INT_EN_MASK) | IRTX_DONE_INT_EN(0));

            WRITE_REG(IRTX_EN_RST_REG,
                      (READ_REG(IRTX_EN_RST_REG) & ~IRTX_EN_MASK) | IRTX_EN(0));
            p_this->status.enable = 0;
            rtk_irtx_tx_stop(p_this);

            break;

        case IRTX_MODE_ON:
            irtx_info("%s irtx mode : On\n", IRTX_MODEL_NAME);
            WRITE_REG(IRTX_EN_RST_REG,
                      (READ_REG(IRTX_EN_RST_REG) & ~IRTX_EN_MASK) | IRTX_EN(1));

            WRITE_REG(IRTX_INT_EN_REG,
                      (READ_REG(IRTX_INT_EN_REG) & ~IRTX_DONE_INT_EN_MASK) | IRTX_DONE_INT_EN(1));

            rtk_irtx_tx_start(p_this);
            p_this->status.enable = 1;

            break;
    }
    return 0;
}


int rtk_irtx_enable(rtk_irtx *p_this, unsigned int on_off)
{
    if(on_off)
        rtk_irtx_set_mode(p_this, IRTX_MODE_ON);
    else
        rtk_irtx_set_mode(p_this, IRTX_MODE_OFF);
    return 0;
}

extern bool pm_freezing;
static void rtk_irtx_wait_xmit_done(IRTX_CMD_BUFF *cmb)
{
#if 0
    int wait_interval = msecs_to_jiffies(100);
    int wait_jiffies = 0;
    while(wait_jiffies < TX_TIMEOUT + 50)
    {
        wait_event_timeout(cmb->wq, (cmb->status != WAIT_XMIT) || (pm_freezing == true), wait_interval);
        if((cmb->status != WAIT_XMIT) || (pm_freezing == true))
            break;
        wait_jiffies += wait_interval;
    }
#else
    if(cmb->cmd.timeout_ms)
        wait_event_timeout(cmb->wq, cmb->status != WAIT_XMIT, msecs_to_jiffies(cmb->cmd.timeout_ms));
    else
        wait_event_timeout(cmb->wq, cmb->status != WAIT_XMIT, IRTX_TX_TIMEOUT + 50);

#endif
}


int rtk_irtx_xmit_message(rtk_irtx *p_this, IRTX_CMD_BUFF *cmb, unsigned long flags)
{
    unsigned int i = 0;
    int ret = 0;
    unsigned char status;
    if (!p_this->xmit.enable)
        return -1;

    /*******do some checks********/
    if(cmb->cmd.pwm_en && (cmb->cmd.pwm_duty < 0 || cmb->cmd.pwm_duty > 100
                           || cmb->cmd.pwm_freq <= 0 || cmb->cmd.pwm_freq > (IRTX_BUS_CLK_REQ / 2)))
        return -EINVAL;

    if(!cmb->cmd.timing_unit)
        return -EINVAL;

    for(i  = 0; i < cmb->cmd.timing_info_len; i++)
    {
        unsigned int timing_length = IRTX_GET_TIMING_LENGTH(cmb->cmd.timing_info[i]);
        if(timing_length > EACH_TIMING_INFO_MAX_LEN * IRTX_HW_FIFO_LEN)
            return -EINVAL;
    }

    cmb->flags = flags;
    cmb->status = WAIT_XMIT;

    irtx_cmb_queue_tail(&p_this->tx_queue, cmb);

    schedule_delayed_work(&p_this->xmit.work, 0);

    if ((cmb->flags & NONBLOCK) == 0)
    {

        rtk_irtx_wait_xmit_done(cmb);
        status = cmb->status;
        switch (status)
        {
            case XMIT_OK:
            case XMIT_ABORT:
            case XMIT_FAIL:
                if(status == XMIT_OK)
                {
                    irtx_dbg("irtx : xmit message success\n");
                }
                else if(status == XMIT_ABORT)
                {
                    irtx_warn("irtx : xmit message abort\n");
                }
                else
                {
                    irtx_warn("irtx : xmit message failed\n");
                }

                spin_lock_irqsave(&p_this->lock, flags);
                irtx_kfree_cmb(cmb);
                spin_unlock_irqrestore(&p_this->lock, flags);
                break;

            case XMIT_TIMEOUT:
            case WAIT_XMIT:
            default:
                irtx_warn("irtx : xmit message timeout\n");
                spin_lock_irqsave(&p_this->lock, flags);
                if(p_this->xmit.cmb == cmb)
                {
                    p_this->xmit.state = IDEL;
                    p_this->xmit.cmb = NULL;
                }
                else
                {
                    irtx_cmb_dequeue_find(&p_this->tx_queue, cmb);
                }

                irtx_kfree_cmb(cmb);
                spin_unlock_irqrestore(&p_this->lock, flags);
                break;
        }

        ret = (status == XMIT_OK) ? 0 : -1;

    }

    return ret;
}

void rtk_irtx_uninit(rtk_irtx *p_this)
{
    irtx_info("%s irtx closed\n", IRTX_MODEL_NAME);
    rtk_irtx_set_global_interrupt(p_this, 0);
    free_irq(gic_irq_find_mapping(IRQ_IRTX), p_this);
    rtk_irtx_enable(p_this, 0);
    p_this->status.init = 0;
    //TODO::::::
    CRT_CLK_OnOff(IRTX, CLK_OFF, NULL);
}


int rtk_irtx_suspend(rtk_irtx *p_this)
{
    p_this->status.state_before_suspend = p_this->status.enable;
    rtk_irtx_set_global_interrupt(p_this, 0);
    if (!p_this->status.enable)   /*IRTX is off*/
    {
        irtx_warn("[IRTX]function is off when power on, doesn't enable!!\n");
        return 0;
    }

    rtk_irtx_set_mode(p_this, IRTX_MODE_OFF);

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_irtx_resume
 *
 * Desc : suspend a rtk irtx adapter
 *
 * Parm : p_this : handle of rtk irtx adapter
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_irtx_resume(rtk_irtx *p_this)
{
    irtx_warn("%s irtx resume\n", IRTX_MODEL_NAME);

    rtk_irtx_intial_hw_setting();
    rtk_irtx_set_global_interrupt(p_this, 1);

    if(p_this->status.state_before_suspend == 1)
    {
        p_this->status.state_before_suspend = 0;
        rtk_irtx_set_mode(p_this, IRTX_MODE_ON);
    }
    else
    {
        rtk_irtx_set_mode(p_this, IRTX_MODE_OFF);
    }
    return 0;
}

static int ops_set_global_interrupt(irtx_device *dev, unsigned int onoff)
{
    return rtk_irtx_set_global_interrupt((rtk_irtx *) irtx_get_drvdata(dev), onoff);
}

static int ops_probe(irtx_device *dev)
{
    if (dev->id != ID_RTK_IRTX_CONTROLLER)
        return -ENODEV;

    if (rtk_irtx_init(&m_irtx) == 0)
        irtx_set_drvdata(dev, &m_irtx);

    return 0;
}

static void ops_remove(irtx_device *dev)
{
    rtk_irtx_uninit((rtk_irtx *) irtx_get_drvdata(dev));
}

static int ops_enable(irtx_device *dev, unsigned int on_off)
{
    irtx_warn("ops_enable : %02x\n", on_off);
    return rtk_irtx_enable((rtk_irtx *) irtx_get_drvdata(dev), on_off);
}

static int ops_xmit(irtx_device *dev, IRTX_CMD_BUFF *cmb, unsigned long flags)
{
    return rtk_irtx_xmit_message((rtk_irtx *) irtx_get_drvdata(dev),
                                 cmb, flags);
}


static int ops_suspend(irtx_device *dev)
{
    return rtk_irtx_suspend((rtk_irtx *) irtx_get_drvdata(dev));
}

static int ops_resume(irtx_device *dev)
{
    return rtk_irtx_resume((rtk_irtx *) irtx_get_drvdata(dev));
}


static irtx_device rtk_irtx_controller =
{
    .id = ID_RTK_IRTX_CONTROLLER,
    .name =  IRTX_DRIVE_NAME
};

static irtx_driver rtk_irtx_driver =
{
    .name = IRTX_DRIVE_NAME,
    .probe = ops_probe,
    .remove = ops_remove,
    .suspend = ops_suspend,
    .resume = ops_resume,
    .enable = ops_enable,
    .xmit = ops_xmit,
    .set_global_interrupt = ops_set_global_interrupt,
};


static int __init rtk_irtx_module_init(void)
{
    if (register_irtx_driver(&rtk_irtx_driver) != 0)
        return -EFAULT;

    register_irtx_device(&rtk_irtx_controller);

    irtx_info("module init success\n");
    return 0;
}

static void __exit rtk_irtx_module_exit(void)
{
    unregister_irtx_device(&rtk_irtx_controller);

    unregister_irtx_driver(&rtk_irtx_driver);
}

module_init(rtk_irtx_module_init);
module_exit(rtk_irtx_module_exit);
