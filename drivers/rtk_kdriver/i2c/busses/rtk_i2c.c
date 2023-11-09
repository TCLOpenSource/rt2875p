/* ------------------------------------------------------------------------- */
/* i2c-rtk.c i2c-hw access for Realtek rtk DVR I2C                       */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 2005 Chih-pin Wu <wucp@realtek.com.tw>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
    Version 1.0 written by wucp@realtek.com.tw
    Version 2.0 modified by Frank Ting(frank.ting@realtek.com.tw)(2007/06/21)
-------------------------------------------------------------------------
    1.4     |   20081016    | Multiple I2C Support
-------------------------------------------------------------------------
    1.5     |   20090423    | Add Suspen/Resume Feature
-------------------------------------------------------------------------*/
#ifndef BUILD_QUICK_SHOW
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/wait.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <asm/irq.h>
#include <asm/delay.h>

#include "rtk_i2c_slave.h"
#include "rtk_i2c_attr.h"

#if defined(CONFIG_REALTEK_INT_MICOM)
#include <rtk_kdriver/rtk_emcu_export.h>
#else
#include <rbus/sb2_reg.h>
#include <rtk_kdriver/rtk_semaphore.h>
#endif

#else
#include <asm-generic/bug.h>
#include <errno.h>
#include <vsprintf.h>
#endif /*BUILD_QUICK_SHOW*/

#include <rtk_kdriver/pcbMgr.h>
#include <rtk_kdriver/i2c-rtk-api.h>
#include "rtk_i2c_priv.h"
#include "rtk_i2c.h"
#include <rtk_kdriver/rtk_crt.h>
#define RTK_I2C_MAX_SPEED  (1200)
#define RTK_I2C_MIN_SPEED  (50)

LIST_HEAD(rtk_i2c_list);
#ifndef BUILD_QUICK_SHOW
extern void rtk_i2c_slave_en(rtk_i2c *p_this, int val);

#ifdef CONFIG_RTK_KDRV_I2C_HW_SEMAPHORE_SUPPORT
#if defined(CONFIG_REALTEK_INT_MICOM)
static int m_tv006I2cSemaId = LG_NVRAM_I2C_CHANNEL ;
#else
static int m_tv006I2cSemaId = 0 ;
#endif
#endif

static int m_I2CPowerCnt = 0 ;
static struct mutex m_I2CPower_mutex;
unsigned char i2c_resume_done = 1;            // TRUE: resume  complete / FALSE: suspend complete
#else
#define __init
#endif

char *dump_msg_buffer = NULL;
unsigned char I2C_error_log_onoff = 1;

static void hw_i2c_semaphore_try_lock(rtk_i2c *p_this)
{
#ifdef CONFIG_RTK_KDRV_I2C_HW_SEMAPHORE_SUPPORT

/* SW sema workaround for SCPU/MICOM */
    int timeout = 50;
    int get = 0;

    if(p_this->id == m_tv006I2cSemaId )
    {
        while(timeout--)
        {
            if(rtd_inl(SCPU_SYNC_REG) == 0)
            {
                rtd_outl(SCPU_SYNC_REG, SCPU_SYNC_MAGIC);
                // get the I2C control
                if(rtd_inl(MICOM_SYNC_REG) == 0)
                {
                    get = 1;
                    break;
                }
                else // Can't get the control, reset the sync SCPU sync reg, sleep 20ms and back to while
                    rtd_outl(SCPU_SYNC_REG, 0);
            }
            msleep(20);
        }

        if(get == 0)
            I2C_WARNING("%s %d Can't get the I2C control!\n", __func__, __LINE__);
    }

#endif
}

static void hw_i2c_semaphore_unlock(rtk_i2c *p_this)
{
#ifdef CONFIG_RTK_KDRV_I2C_HW_SEMAPHORE_SUPPORT

/* SW sema workaround for SCPU/MICOM */
    if(p_this->id == m_tv006I2cSemaId )
        rtd_outl(SCPU_SYNC_REG, 0);

#endif
}


void i2c_power(int state)
{
    int i2c_phy = 0xFFFFFFFF; //BIT(0)|BIT(1)|BIT(2)|BIT(3);
    if (state == 1)
    {
        I2C_WARNING("power on\n");
        CRT_CLK_OnOff(I2C, CLK_ON, &i2c_phy);
    }
    else
    {
        I2C_WARNING("power off\n");
        CRT_CLK_OnOff(I2C, CLK_OFF, &i2c_phy);
    }


}



#define IsReadMsg(x)        (x.flags & I2C_M_RD)
#define IsGPIORW(x)         (x.flags & I2C_GPIO_RW)
#define IsSameTarget(x,y)   ((x.addr == y.addr) && !((x.flags ^ y.flags) & (~I2C_M_RD)))

rtk_i2c *find_rtk_i2c_by_id(unsigned char id)
{
    rtk_i2c_adapter *cur = NULL;
    rtk_i2c_adapter *next = NULL;

    list_for_each_entry_safe(cur, next, &rtk_i2c_list, list)
    {
        if (cur->p_phy->id == id)
            return cur->p_phy;
    }

    return NULL;
}

rtk_i2c_adapter *find_rtk_i2c_adapter_by_id(unsigned char id)
{
    rtk_i2c_adapter *cur = NULL;
    rtk_i2c_adapter *next = NULL;

    list_for_each_entry_safe(cur, next, &rtk_i2c_list, list)
    {
        if (cur->p_phy->logical_id == id)
            return cur;
    }

    return NULL;
}

#ifndef BUILD_QUICK_SHOW
/*------------------------------------------------------------------
 * Func : i2c_rtk_speed_setting
 *
 * Desc : Speed definition:
          For TV006 I2C
          I2C_M_HIGH_SPEED (Speed:400KHz)
          I2C_M_FAST_SPEED (Speed:800KHz)
          For TV001 I2C
          I2C_M_FAST_SPEED (Speed:400KHz)
          I2C_M_HIGH_SPEED (Speed:800KHz)
 *
 * Parm : adapter : i2c adapter
 *        msgs    : i2c messages
 *
 * Retn : 0 success, otherwise fail
 *------------------------------------------------------------------*/
int i2c_rtk_speed_setting(rtk_i2c *p_this, const struct i2c_msg *p_msg,rtk_i2c_adapter *p_adp)
{
#ifdef CONFIG_CUSTOMER_TV006
        switch (p_msg->flags & I2C_M_SPEED_MASK)
        {
        case I2C_M_HIGH_SPEED:
            p_this->set_spd(p_this, p_adp->spd << 2);
            break;
        case I2C_M_FAST_SPEED:
#ifdef SET_INTERNAL_DEMOD_1200HZ
            if((p_msg->addr == 0x12)&&(p_this->id==0))
                p_this->set_spd(p_this, 1200);
            else
#endif
            p_this->set_spd(p_this, p_adp->spd << 3);
            break;
        case I2C_M_LOW_SPEED:
            p_this->set_spd(p_this, p_adp->spd >> 1);
            break;
        default:
        case I2C_M_NORMAL_SPEED:
            p_this->set_spd(p_this, p_adp->spd);
            break;
        }
#else
        switch (p_msg->flags & I2C_M_SPEED_MASK)
        {
        case I2C_M_HIGH_SPEED:
            p_this->set_spd(p_this, 1000);
            break;
        case I2C_M_FAST_SPEED:
            p_this->set_spd(p_this, p_adp->spd << 2);
            break;
        case I2C_M_LOW_SPEED:
            p_this->set_spd(p_this, p_adp->spd >> 1);
            break;
        default:
        case I2C_M_NORMAL_SPEED:
            p_this->set_spd(p_this, p_adp->spd);
            break;
        }
#endif
        return 0;
}


void i2c_rtk_dump_msg(int errorType , const struct i2c_msg *p_msg , rtk_i2c *p_this , int msgIndex , int msgTotal)
{
    RTK_I2C_WARNING("-----------------------------------------\n");
    switch (errorType)
    {
        case -ECMDSPLIT:
            RTK_I2C_WARNING("[ch_%d][phy_%d] Xfer fail - MSG SPLIT , message transfer not complete,(index:%d/total:%d/slave_addr:0x%x)\n",
                        p_this->logical_id, p_this->id, msgIndex,msgIndex, p_this->tar);
            break;

        case -ETXABORT:
            if(p_this->i2c_GPIO_mode == 0){
                RTK_I2C_WARNING("[ch_%d][phy_%d] Xfer fail - TXABORT (index:%d/total:%d/slave_addr:0x%x), Reason=%04x\n",
                        p_this->logical_id, p_this->id, msgIndex, msgIndex, p_this->tar, p_this->get_tx_abort_reason(p_this));
            }else{
                RTK_I2C_WARNING("[ch_%d][phy_%d] Xfer fail - TXABORT (index:%d/total:%d/slave_addr:0x%x), Reason=0\n",
                    p_this->logical_id, p_this->id, msgIndex, msgIndex, p_this->tar);
            }

            if((p_this->get_tx_abort_reason(p_this)& 0x0000001F)!= 0)
            {
                RTK_I2C_WARNING("Xfer fail - No ACK \n");
            }
            break;

        case -ETIMEOUT:
            RTK_I2C_WARNING("[ch_%d][phy_%d] Xfer fail - TIMEOUT, I2C bus start bit fail,(index:%d/total:%d/slave_addr:0x%x)\n",
                        p_this->logical_id, p_this->id, msgIndex,msgIndex, p_this->tar);
            break;

        case -EILLEGALMSG:
            RTK_I2C_WARNING("[ch_%d][phy_%d] Xfer fail - ILLEGAL MSG (index:%d/total:%d/slave_addr:0x%x)\n",
                            p_this->logical_id, p_this->id, msgIndex, msgIndex, p_this->tar);
            break;

        case -EADDROVERRANGE:
            RTK_I2C_WARNING("[ch_%d][phy_%d] Xfer fail - ADDRESS OUT OF RANGE (index:%d/total:%d/slave_addr:0x%x)\n",
                        p_this->logical_id, p_this->id, msgIndex, msgIndex, p_this->tar);
            break;

        default:
            RTK_I2C_WARNING("[ch_%d][phy_%d] Xfer fail - Unkonwn Return Value (index:%d/total:%d/slave_addr:0x%x)\n",
                        p_this->logical_id, p_this->id, msgIndex, msgIndex, p_this->tar);
            break;
    }
    I2C_WARNING("device name: %s\n", rtk_i2c_get_dev_name(p_this->logical_id, p_msg->addr << 1));
    I2C_WARNING("msg->addr  = 0x%02x(8bit-0x%02x)\n", p_msg->addr, (p_msg->addr<<1));
    I2C_WARNING("msg->flags = %04x\n", p_msg->flags);
    I2C_WARNING("msg->len   = %d  \n", p_msg->len);
    I2C_WARNING("msg->buf   = %p  \n", p_msg->buf);
    RTK_I2C_WARNING("-----------------------------------------\n");
}

void print_arrary_data(char* buff, unsigned int buff_len, unsigned char* p_data, unsigned int len)
{
    while(len--)
    {
        int ret = snprintf(buff, buff_len, "%02x ", *p_data++);
        buff += ret;
        buff_len -= ret;
    }
    *buff = 0;  // padd null
}

/*------------------------------------------------------------------
 * Func : i2c_rtk_xfer
 *
 * Desc : start i2c xfer (read/write)
 *
 * Parm : adapter : i2c adapter
 *        msgs    : i2c messages
 *        num     : nessage counter
 *
 * Retn : 0 success, otherwise fail
 *------------------------------------------------------------------*/
static int i2c_rtk_xfer(void *dev_id, struct i2c_msg *msgs, int num)
{

    rtk_i2c_adapter *p_adp = (rtk_i2c_adapter *) dev_id;
    rtk_i2c *p_this = (rtk_i2c *) p_adp->p_phy;
    int ret = 0;
    int i;

    if(i2c_resume_done != 1) {
        RTK_I2C_WARNING("[ch_%d][phy_%d] I2C driver resume not ready\n", p_this->logical_id, p_this->id);
        return -EACCES;
    }

    if(msgs == NULL)
        return -1;

    if(p_this->i2c_function_disable == 1)
    {
        return 0;
    }

    LockrtkI2CPhy(p_this);
    hw_i2c_semaphore_try_lock(p_this);

    if(p_this->disable_switch_port == 0)
    {
        if (p_adp->port < 0)  // && p_this->set_port(p_this, p_adp->port) < 0)
        {
            hw_i2c_semaphore_unlock(p_this);
            UnlockrtkI2CPhy(p_this);
            return -EACCES;
        }
    }

    for (i = 0; i < num; i++)
    {
        ret = p_this->set_tar(p_this, msgs[i].addr, ADDR_MODE_7BITS);

        if (ret < 0)
            goto err_occur;

        i2c_rtk_speed_setting(p_this, &msgs[i], p_adp);

#ifdef CONFIG_CUSTOMER_TV006
        /* Set SW guard interval between each command */
        if ((p_this->id == LG_DVBS_TUNER_I2C_CHANNEL &&
                msgs[i].addr == LG_DVBS_TUNER_ADDRESS))
            p_this->set_guard_interval(p_this, 300);
        else
            p_this->set_guard_interval(p_this, 0);
#else
        p_this->set_guard_interval(
                p_this,(msgs[i].flags & I2C_M_NO_GUARD_TIME) ? 0 :1000);
#endif

        switch (msgs[i].flags & I2C_M_HW_DELAY_MASK)
        {
        case I2C_M_400US_DELAY:
            p_this->set_hw_guard_interval(p_this, 400);
            break;
        case I2C_M_600US_DELAY:
            p_this->set_hw_guard_interval(p_this, 600);
            break;
        case I2C_M_800US_DELAY:
            p_this->set_hw_guard_interval(p_this, 800);
            break;
        case I2C_M_1000US_DELAY:
            p_this->set_hw_guard_interval(p_this, 1000);
            break;
        case I2C_M_2000US_DELAY:
            p_this->set_hw_guard_interval(p_this, 2000);
            break;
        default:
            p_this->set_hw_guard_interval(p_this, 0);
            break;
        };

        if (IsReadMsg(msgs[i]))
        {
            if (IsGPIORW(msgs[i]) || p_this->i2c_GPIO_mode)
                ret = p_this->gpio_read(p_this, NULL, 0, msgs[i].buf, msgs[i].len);
            else
                ret = p_this->read(p_this, NULL, 0, msgs[i].buf,msgs[i].len);
        }
        else
        {
            if ((i < (num - 1)) &&
                IsReadMsg(msgs[i + 1]) && IsSameTarget(msgs[i], msgs[i + 1]))
            {

            if (IsGPIORW(msgs[i]) || p_this->i2c_GPIO_mode)
                {
                    ret = p_this->gpio_read(p_this,
                                          msgs[i].buf,msgs[i].len,
                                          msgs[i + 1].buf,msgs[i + 1].len);
                }
                else
                {
                    ret = p_this->read(p_this,
                                        msgs[i].buf,msgs[i].len,
                                        msgs[i + 1].buf,msgs[i + 1].len);
                }
                i++;
            }
            else
            {
                if (IsGPIORW(msgs[i]) || p_this->i2c_GPIO_mode)
                {
                    ret = p_this->gpio_write(p_this,msgs[i].buf,msgs[i].len,
                                            (i == (num -1)) ? WAIT_STOP: NON_STOP);
                }
                else
                {
                    ret = p_this->write(p_this, msgs[i].buf,msgs[i].len,
                                        (i == (num - 1)) ? WAIT_STOP : NON_STOP);
                }
            }
        }

        if (ret < 0 && dump_msg_buffer != NULL){
            memset(dump_msg_buffer,0,PAGE_SIZE);
            print_arrary_data(dump_msg_buffer, PAGE_SIZE-1, msgs[i].buf, msgs[i].len);
            RTK_I2C_WARNING("[ch_%d][phy_%d] msg[%d], master_write, addr=%x, ret=%d, tx_len=%d, tx_data={%s}\n",p_this->logical_id, p_this->id, i, p_this->tar, ret, msgs[i].len, dump_msg_buffer);

            goto err_occur;
        }
    }

    if (p_adp->port >= 0)
    {
        //p_this->unset_port(p_this, p_adp->port);
    }

    if (p_this->flags & rtk_I2C_SLAVE_ENABLE)
    {
        p_this->slave_mode_enable(p_this, 1);
    }

    hw_i2c_semaphore_unlock(p_this);
    UnlockrtkI2CPhy(p_this);

    return i;

err_occur:

    if (p_adp->port >= 0)
    {
        //p_this->unset_port(p_this, p_adp->port);
    }

    if (p_this->flags & rtk_I2C_SLAVE_ENABLE)
    {
        p_this->slave_mode_enable(p_this, 1);
    }

    if((ret == -ETIMEOUT) && (p_this->disable_auto_recovery == 0) )
    {
        if( p_this->i2c_GPIO_mode == 0 ){
            int i2c_phy = BIT(p_this->id);
            RTK_I2C_WARNING("[ch_%d][phy_%d] Xfer fail - TIMEOUT, I2C start recovery,isr_en =%x ,isr=%x \n",
            p_this->logical_id,p_this->id,GET_IC_ISR_EN(p_this),GET_IC_ISR(p_this));
            CRT_CLK_OnOff(I2C, CLK_OFF, &i2c_phy);
            mdelay(50);
            CRT_CLK_OnOff(I2C, CLK_ON, &i2c_phy);
        }else{
            RTK_I2C_WARNING("[ch_%d][phy_%d] Xfer fail - TIMEOUT, I2C start recovery,isr_en =%x ,isr=%x \n",
            p_this->logical_id,p_this->id,0,0);
        }
    }

    hw_i2c_semaphore_unlock(p_this);
    UnlockrtkI2CPhy(p_this);

    i2c_rtk_dump_msg(ret, &msgs[i], p_this, i, num);

    ret = -EACCES;
    return ret;
}

/*------------------------------------------------------------------
 * Func : i2c_rtk_set_speed
 *
 * Desc : set speed of rtk i2c
 *
 * Parm : dev_id : i2c adapter
 *        KHz    : speed of i2c adapter
 *
 * Retn : 0 success, otherwise fail
 *------------------------------------------------------------------*/
int i2c_rtk_set_speed(void *dev_id, int KHz)
{
    rtk_i2c_adapter *p_adp = (rtk_i2c_adapter *) dev_id;

    if (p_adp)
    {
        if (KHz > RTK_I2C_MAX_SPEED)
            KHz = RTK_I2C_MAX_SPEED;

        if (KHz < RTK_I2C_MIN_SPEED)
            KHz = RTK_I2C_MIN_SPEED;

        p_adp->spd = KHz;
        return 0;
    }

    return -1;
}

/*------------------------------------------------------------------
 * Func : i2c_rtk_register_adapter
 *
 * Desc : register i2c_adapeter
 *
 * Parm : p_phy       : pointer of i2c phy
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
static int i2c_rtk_register_adapter(struct platform_device *pdev, rtk_i2c * p_phy, int port)
{
    rtk_i2c_adapter *p_adp =
        kmalloc(sizeof(rtk_i2c_adapter), GFP_KERNEL);

    if (p_adp)
    {
        memset(p_adp, 0, sizeof(rtk_i2c_adapter));

        p_adp->adap.owner = THIS_MODULE;
        p_adp->adap.class = I2C_CLASS_HWMON;

        snprintf(p_adp->adap.name, sizeof(p_adp->adap.name), "%s I2C %d bus", p_phy->model_name,p_phy->id);

        p_adp->adap.algo_data  = &p_adp->algo;
        p_adp->algo.dev_id     = (void *)p_adp;
        p_adp->algo.masterXfer = i2c_rtk_xfer;
        p_adp->algo.set_speed  = i2c_rtk_set_speed;
        p_adp->p_phy           = p_phy;
        p_adp->port            = port;
        p_adp->spd             = 100;

#ifdef CONFIG_PM
        p_adp->p_dev = pdev;
        platform_set_drvdata(p_adp->p_dev, p_adp);
#endif

        i2c_rtk_add_bus(&p_adp->adap);

        INIT_LIST_HEAD(&p_adp->list);

        list_add_tail(&p_adp->list, &rtk_i2c_list);

        rtk_i2c_create_attr(&p_adp->adap.dev);
    }

    return (p_adp) ? 0 : -1;
}

/*------------------------------------------------------------------
 * Func : i2c_rtk_unregister_adapter
 *
 * Desc : remove rtk i2c adapeter
 *
 * Parm : p_adap      : rtk_i2c_adapter data structure
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
static void i2c_rtk_unregister_adapter(rtk_i2c_adapter * p_adap)
{
    //i2c_rtk_algo *p_algo = NULL;

    if (p_adap)
    {
        list_del(&p_adap->list);

        //p_algo = (i2c_rtk_algo *) p_adap->adap.algo_data;

        rtk_i2c_remove_attr(&p_adap->adap.dev);

        i2c_rtk_del_bus(&p_adap->adap);

        destroy_rtk_i2c_handle(p_adap->p_phy);

        kfree(p_adap);
    }
}


#ifdef CONFIG_PM

#define RTK_I2C_NAME          "rtk_i2c"

extern struct device* get_rtkdemod_device_ptr(void);

static int rtk_i2c_pm_suspend(struct device *dev)
{
    rtk_i2c_adapter *p_vadp = dev_get_drvdata(dev);
    rtk_i2c *p_this = (rtk_i2c *) p_vadp->p_phy;

    if(NULL == p_this)
    {
        I2C_WARNING("rtk_i2c_pm_suspend failed\n");
        return -1;
    }


    I2C_WARNING("rtk_i2c_pm_suspend before\n");

    i2c_resume_done = 0;
    LockrtkI2CPhy(p_this);
    p_this->suspend(p_this);
    UnlockrtkI2CPhy(p_this);

    mutex_lock(&m_I2CPower_mutex);

    m_I2CPowerCnt --;
    if(m_I2CPowerCnt == 0)
    {
#if defined(CONFIG_REALTEK_INT_MICOM)
        int i2c_phy =  BIT(1) | BIT(2) | BIT(3) | BIT(4); //Skip ISO I2C: I2C0 and I2C5, not to CLK_OFF these two.;
#else
        int i2c_phy = 0xFFFFFFFF; //BIT(0)|BIT(1)|BIT(2)|BIT(3);
#endif
        CRT_CLK_OnOff(I2C, CLK_OFF, &i2c_phy);
    }

    mutex_unlock(&m_I2CPower_mutex);

    return 0;

}

static int rtk_i2c_pm_resume(struct device *dev)
{
    rtk_i2c_adapter *p_vadp = dev_get_drvdata(dev);
    rtk_i2c *p_this = (rtk_i2c *) p_vadp->p_phy;

    if(NULL == p_this)
    {
        I2C_WARNING("rtk_i2c_pm_resume failed\n");
        return -1;
    }
    I2C_WARNING("rtk_i2c_pm_resume before\n");

    mutex_lock(&m_I2CPower_mutex);

    if(m_I2CPowerCnt == 0)
    {
        int i2c_phy = 0xFFFFFFFF; //BIT(0)|BIT(1)|BIT(2)|BIT(3);
        CRT_CLK_OnOff(I2C, CLK_ON, &i2c_phy);
    }
    m_I2CPowerCnt ++;

    mutex_unlock(&m_I2CPower_mutex);

    LockrtkI2CPhy(p_this);
    p_this->resume(p_this);
    UnlockrtkI2CPhy(p_this);

    i2c_resume_done = 1;
    return 0;
}

static const struct dev_pm_ops rtk_i2c_pm_ops =
{
    .suspend_late = rtk_i2c_pm_suspend, //[KTASKWBS-1065]
    .resume_early = rtk_i2c_pm_resume,
#ifdef CONFIG_HIBERNATION
    .freeze_late = rtk_i2c_pm_suspend,
    .thaw_early = rtk_i2c_pm_resume,
    .poweroff_late = rtk_i2c_pm_suspend,
    .restore_early = rtk_i2c_pm_resume,
#endif
};

static int rtk_i2c_probe(struct platform_device *pdev)
{
    int irq, ret;
    rtk_i2c *p_this = NULL;
    int logical_id, i2c_mode;
    unsigned int pcb_info[4] = {0};

    ret = of_property_read_u32_array(pdev->dev.of_node, "pcb_info", pcb_info, 4);
    if (ret < 0)
    {
        I2C_WARNING("[%s] line: %d, get dts pcb_info error, ret: %d\n",
                    __func__, __LINE__, ret);
        return ret;
    }
#if 0
    I2C_WARNING("[%s] line: %d, logical_id:%d, i2c_mode: %d, phy_id:%d, src_port:%d\n",
                    __func__, __LINE__, pcb_info[0], pcb_info[1], pcb_info[2], pcb_info[3]);
#endif
    logical_id  = pcb_info[0];
    i2c_mode    = pcb_info[1];

    if (I2C_MODE == i2c_mode)
    {
        unsigned int phy_id  = pcb_info[2];
        unsigned int src_port = pcb_info[3];

        I2C_WARNING("[%s] line: %d, logical_id:%d, i2c_mode: HW_I2C, phy_id:%d, src_port:%d\n",
                    __func__, __LINE__, logical_id, phy_id, src_port);

        irq = platform_get_irq(pdev, 0);
        //I2C_WARNING("[%s] line: %d, get irq = %d\n", __func__, __LINE__, irq);
        if (irq < 0)
        {
            I2C_WARNING("[%s] line: %d, phy_id:%d, get irq(%d) error\n", __func__, __LINE__, phy_id, irq);
            return irq;
        }

        p_this = create_rtk_i2c_handle(logical_id, phy_id,
                    rtk_MASTER_7BIT_ADDR, ADDR_MODE_7BITS, SPD_MODE_SS, irq);
        if (p_this == NULL)
        {
            I2C_WARNING("[%s] line: %d, create i2c handle error\n", __func__, __LINE__);
            return -1;
        }
        p_this->i2c_GPIO_mode = 0;
        I2C_WARNING( "[ch_%d] open successed, reference phy=%d cnt=%d\n",
                    logical_id, phy_id, atomic_read(&p_this->ref_cnt));
	
        if (p_this->init(p_this) < 0 || i2c_rtk_register_adapter(pdev, p_this, src_port) < 0)
        {
            destroy_rtk_i2c_handle(p_this);
        }
    }
    else
    {
        unsigned int sda = pcb_info[2];
        unsigned int scl = pcb_info[3];

        I2C_WARNING( "[ch_%d], mode = G2C, sda = %d, scl = %d\n", logical_id, sda, scl);

        p_this = create_rtk_g2c_handle(logical_id, sda, scl);
        if (p_this == NULL)
        {
            I2C_WARNING("[%s] line: %d, create i2c handle error\n", __func__, __LINE__);
            return -1;
        }
        p_this->i2c_GPIO_mode = 1;

        if (p_this->init(p_this) < 0 || i2c_rtk_register_adapter(pdev, p_this, -1) < 0)
            destroy_rtk_i2c_handle(p_this);
    }

    mutex_lock(&m_I2CPower_mutex);
    m_I2CPowerCnt++;
    mutex_unlock(&m_I2CPower_mutex);

    return 0;
}

int rtk_i2c_remove(struct platform_device *pdev)
{
    return 0;
}

static struct of_device_id rtk_i2c_devices[] =
{
    {.compatible = "realtek, rtk-i2c"},
    {},
};
MODULE_DEVICE_TABLE(of, rtk_i2c_devices);

static struct platform_driver rtk_i2c_platform_drv =
{
    .probe = rtk_i2c_probe,
    .remove = rtk_i2c_remove,
    .driver = {
        .name = RTK_I2C_NAME,
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(rtk_i2c_devices),
        .bus = &platform_bus_type,
        .pm = &rtk_i2c_pm_ops,
    },
};
#endif



int rtk_i2c_register_slave_driver(unsigned char id,
                                  rtk_i2c_slave_driver * p_drv)
{
    rtk_i2c *phy = find_rtk_i2c_by_id(id);
    rtk_i2c_slave_ops ops =
    {
        .handle_command = p_drv->handle_command,
        .read_data = p_drv->read_data,
        .stop = p_drv->stop,
    };

    if (phy)
    {
        phy->set_sar(phy, p_drv->sar, ADDR_MODE_7BITS);
        return phy->register_slave_ops(phy, &ops, p_drv->private_data);
    }

    I2C_WARNING("rtk_i2c_register_slave_driver failed, can't find adapter %d\n",id);
    return -1;
}

void rtk_i2c_unregister_slave_driver(unsigned char id,
                                     rtk_i2c_slave_driver * p_drv)
{
    rtk_i2c *phy = find_rtk_i2c_by_id(id);

    if (phy)
        phy->register_slave_ops(phy, NULL, 0);
    else
        I2C_WARNING("rtk_i2c_unregister_slave_driver failed, can't find adapter %d\n", id);
}

int rtk_i2c_slave_enable(unsigned char id, unsigned char on)
{
    rtk_i2c *phy = find_rtk_i2c_by_id(id);

    if (phy == NULL)
        I2C_WARNING( "rtk_i2c_slave_enable failed, can't find adapter %d\n", id);

    return (phy) ? phy->slave_mode_enable(phy, on) : -1;
}

/*------------------------------------------------------------------
 * Func : i2c_rtk_module_init
 *
 * Desc : init rtk i2c module
 *
 * Parm : N/A
 *
 * Retn : 0 success, otherwise fail
 *------------------------------------------------------------------*/
static int
__init i2c_rtk_module_init(void)
{
    m_I2CPowerCnt = 0;
    mutex_init(&m_I2CPower_mutex);
    dump_msg_buffer = vmalloc(PAGE_SIZE);
    if(dump_msg_buffer == NULL){
        I2C_WARNING( "i2c dump log buf is NULL\n");
    }

#ifdef CONFIG_PM
    platform_driver_register(&rtk_i2c_platform_drv);
#endif

    rtk_i2c_get_dev_info_by_pcbenum();

    return 0;
}

/*------------------------------------------------------------------
 * Func : i2c_rtk_module_exit
 *
 * Desc : exit rtk i2c module
 *
 * Parm : N/A
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static void
__exit i2c_rtk_module_exit(void)
{
    rtk_i2c_adapter *cur = NULL;
    rtk_i2c_adapter *next = NULL;

    vfree(dump_msg_buffer);

    list_for_each_entry_safe(cur, next, &rtk_i2c_list, list)
    {
        i2c_rtk_unregister_adapter(cur);
    }

    mutex_destroy(&m_I2CPower_mutex);

#ifdef CONFIG_PM
    platform_driver_unregister(&rtk_i2c_platform_drv);
#endif
}

MODULE_AUTHOR("Kevin-Wang <kevin_wang@realtek.com.tw>");
MODULE_DESCRIPTION("I2C-Bus adapter routines for Realtek rtk/Neptune DVR");
MODULE_LICENSE("GPL");

subsys_initcall(i2c_rtk_module_init);
module_exit(i2c_rtk_module_exit);
#else

#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
/*------------------------------------------------------------------
 * Func : load_i2c_config_from_pcb_mgr
 *
 * Desc : get config for i2c setting form pcb manager
 *
 * Parm : id : I2C id
 *
 * Retn : 0 success, otherwise fail
 *------------------------------------------------------------------*/
int __init load_i2c_config_from_pcb_mgr(rtk_i2c_adapter_config * p_config,
                                        unsigned char config_count)
{
    unsigned long long i2c_config = 0;
    int adapter_count = 0;
    char name[16] = { 0 };
    int i = 0;
    int i2cPortCounter = 0 ;

    for (i = 0; i < config_count; i++)
    {
        sprintf(name, "I2C_PORT_%d", i);
        if (pcb_mgr_get_enum_info_byname(name, &i2c_config) == 0)
        {
            BUG_ON( i != i2cPortCounter );  /*if this happened , this is due to I2C_PORT_? is not continued , I2C_PORT_? does not keep the sequence  **/
            i2cPortCounter++;

            if (GET_I2C_TYPE(i2c_config) == PCB_I2C_TYPE_HW_I2C)
            {
                p_config[adapter_count].mode       = I2C_MODE;
                p_config[adapter_count].i2c.phy_id = GET_HW_I2C_PHY(i2c_config);

                if (GET_HW_I2C_SRC(i2c_config) == PCB_I2C_SUB_SRC_ANY)
                    p_config[adapter_count].i2c.port_id = -1;
                else
                    p_config[adapter_count].i2c.port_id = GET_HW_I2C_SRC(i2c_config);

                I2C_WARNING
                ("Get PCB_ENUM I2C_PORT_%d I2C Mode Config phy_id=%d port_id = %d\n",
                 i, p_config[adapter_count].i2c.phy_id,
                 p_config[adapter_count].i2c.port_id);
            }
            else
            {
                p_config[adapter_count].mode = G2C_MODE;

                if ( GET_PIN_TYPE(GET_GPIO_I2C_SDA(i2c_config)) == PCB_PIN_TYPE_ISO_GPIO)
                {
                    p_config[adapter_count].g2c.sda =
                        rtk_gpio_id(ISO_GPIO,GET_PIN_INDEX(GET_GPIO_I2C_SDA(i2c_config)));
                }
                else
                {
                    p_config[adapter_count].g2c.sda =
                        rtk_gpio_id(MIS_GPIO,GET_PIN_INDEX(GET_GPIO_I2C_SDA(i2c_config)));
                }

                if (GET_PIN_TYPE(GET_GPIO_I2C_SCL(i2c_config))== PCB_PIN_TYPE_ISO_GPIO)
                {
                    p_config[adapter_count].g2c.scl =
                        rtk_gpio_id(ISO_GPIO,GET_PIN_INDEX(GET_GPIO_I2C_SCL(i2c_config)));
                }
                else
                {
                    p_config[adapter_count].g2c.scl =
                        rtk_gpio_id(MIS_GPIO,GET_PIN_INDEX(GET_GPIO_I2C_SCL(i2c_config)));
                }

                I2C_WARNING("Get PCB_ENUM I2C_PORT_%d G2C Mode Config SDA=%d SCL = %d\n", i,
                     p_config[adapter_count].g2c.sda,p_config[adapter_count].g2c.scl);
            }

            adapter_count++;
        }

    }
    return adapter_count;
}

#endif

/*------------------------------------------------------------------
 * Func : load_i2c_config
 *
 * Desc : load i2c config
 *
 * Parm : id : I2C id
 *
 * Retn : 0 success, otherwise fail
 *------------------------------------------------------------------*/
int __init load_i2c_config(rtk_i2c_adapter_config * p_config,
                           unsigned char config_count)
{
    unsigned char adapter_count = 0;
    int i = 0;

#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
    if (adapter_count == 0)
        adapter_count =
            load_i2c_config_from_pcb_mgr(p_config, config_count);
#endif

    if (adapter_count == 0)
    {
        I2C_WARNING( "[I2C] Find No I2C info in PCB_ENUM.\n");
        adapter_count = get_rtk_i2c_phy_count();

        if (adapter_count > config_count)
            adapter_count = config_count;

        for (i = 0; i < adapter_count; i++)
        {
            p_config[i].mode = I2C_MODE;
            p_config[i].i2c.phy_id = i;
            p_config[i].i2c.port_id = -1;
            I2C_WARNING( "[I2C] Config I2C_MODE phy_id=%d port_id = %d\n",
                         p_config[i].i2c.phy_id, p_config[i].i2c.port_id);
        }
    }

    return adapter_count;
}
/*------------------------------------------------------------------
 * Func : i2c_rtk_register_adapter
 *
 * Desc : register i2c_adapeter
 *
 * Parm : p_phy       : pointer of i2c phy
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
static int __init i2c_rtk_register_adapter(rtk_i2c * p_phy, int port)
{
    rtk_i2c_adapter *p_adp =
        kmalloc(sizeof(rtk_i2c_adapter), GFP_KERNEL);

    if (p_adp)
    {
        memset(p_adp, 0, sizeof(rtk_i2c_adapter));
        p_adp->p_phy           = p_phy;
        p_adp->port            = port;
        p_adp->spd             = 100;

        INIT_LIST_HEAD(&p_adp->list);

        list_add_tail(&p_adp->list, &rtk_i2c_list);
    }

    return (p_adp) ? 0 : -1;
}

/*------------------------------------------------------------------
 * Func : i2c_rtk_unregister_adapter
 *
 * Desc : remove rtk i2c adapeter
 *
 * Parm : p_adap      : rtk_i2c_adapter data structure
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
static
void i2c_rtk_unregister_adapter(rtk_i2c_adapter * p_adap)
{
    if (p_adap)
    {
        list_del(&p_adap->list);
        kfree(p_adap);
    }
}

/*------------------------------------------------------------------
 * Func : i2c_rtk_speed_setting
 *
 * Desc : Speed definition:
          For TV006 I2C
          I2C_M_HIGH_SPEED (Speed:400KHz)
          I2C_M_FAST_SPEED (Speed:800KHz)
          For TV001 I2C
          I2C_M_FAST_SPEED (Speed:400KHz)
          I2C_M_HIGH_SPEED (Speed:800KHz)
 *
 * Parm : adapter : i2c adapter
 *        msgs    : i2c messages
 *
 * Retn : 0 success, otherwise fail
 *------------------------------------------------------------------*/
static int i2c_rtk_speed_setting(rtk_i2c *p_this, unsigned short flags,rtk_i2c_adapter *p_adp)
{
    switch (flags & I2C_M_SPEED_MASK)
    {
        case I2C_M_FAST_SPEED:
            p_this->set_spd(p_this, p_adp->spd << 2);
            break;
        case I2C_M_HIGH_SPEED:
            p_this->set_spd(p_this, p_adp->spd << 3);
            break;
        case I2C_M_LOW_SPEED:
            p_this->set_spd(p_this, p_adp->spd >> 1);
            break;
        default:
        case I2C_M_NORMAL_SPEED:
            p_this->set_spd(p_this, p_adp->spd);
            break;
    }
    return 0;
}
/*------------------------------------------------------------------
 * Func : i2c_rtk_xfer
 *
 * Desc : start i2c xfer (read/write)
 *
 * Parm : dev_id  : i2c device port
 *        msgs    : i2c messages
 *        num     : nessage counter
 *
 * Retn : 0 success, otherwise fail
 *------------------------------------------------------------------*/
int i2c_rtk_xfer(unsigned char bus_id, unsigned char addr,
    unsigned char *p_sub_addr, unsigned char sub_addr_len,
    unsigned char *p_read_buff, unsigned int read_len, unsigned short flags)
{
    rtk_i2c *p_this = NULL;
    int ret = 0;
    int i;
    rtk_i2c_adapter *p_adp = NULL;
    p_adp= find_rtk_i2c_adapter_by_id(bus_id);
    p_this = (rtk_i2c *) p_adp->p_phy;
    if (p_this == NULL)
        return -1;
    if(p_this->i2c_function_disable == 1) {
        return 0;
    }

    ret = p_this->set_tar(p_this, addr, ADDR_MODE_7BITS);
    if (ret < 0)
        goto err_occur;

    i2c_rtk_speed_setting(p_this, flags,p_adp);

    p_this->set_guard_interval(
            p_this,(flags & I2C_M_NO_GUARD_TIME) ? 0 :1000);

    switch (flags & I2C_M_HW_DELAY_MASK)
    {
        case I2C_M_400US_DELAY:
            p_this->set_hw_guard_interval(p_this, 400);
            break;
        case I2C_M_600US_DELAY:
            p_this->set_hw_guard_interval(p_this, 600);
            break;
        case I2C_M_800US_DELAY:
            p_this->set_hw_guard_interval(p_this, 800);
            break;
        case I2C_M_1000US_DELAY:
            p_this->set_hw_guard_interval(p_this, 1000);
            break;
        case I2C_M_2000US_DELAY:
            p_this->set_hw_guard_interval(p_this, 2000);
            break;
        default:
            p_this->set_hw_guard_interval(p_this, 0);
            break;
    };

    if (p_read_buff)
        ret = p_this->read(p_this, p_sub_addr, sub_addr_len, p_read_buff, read_len);
    else
        ret = p_this->write(p_this, p_sub_addr, sub_addr_len, WAIT_STOP);
    if (ret < 0)
        goto err_occur;

    return ret;

err_occur:
    if((ret == -ETIMEOUT) && (p_this->disable_auto_recovery == 0) )
    {
        int i2c_phy = BIT(p_this->id);
        RTK_I2C_WARNING("[ch_%d][phy_%d] Xfer fail - TXABORT (slave_addr:0x%x), Reason=%04x\n",
                        p_this->logical_id, p_this->id, p_this->tar, p_this->get_tx_abort_reason(p_this));
        CRT_CLK_OnOff(I2C, CLK_OFF, &i2c_phy);
        mdelay(50);
        CRT_CLK_OnOff(I2C, CLK_ON, &i2c_phy);
    }

    ret = -EACCES;
    return ret;
}

int i2c_rtk_module_init(void)
{
    rtk_i2c_adapter_config adapter_config[MAX_I2C_ADAPTER] = { {0} };
    unsigned char adapter_count = 0;
    int i = 0;
    rtk_i2c *p_this = NULL;

    i2c_power(1);

    adapter_count = load_i2c_config(adapter_config, MAX_I2C_ADAPTER);
    for (i = 0; i < adapter_count; i++)
    {
        if (adapter_config[i].mode == I2C_MODE)
        {
            I2C_WARNING( "[ch_%d]: phy =%d, port =%d\n",	i,
                         adapter_config[i].i2c.phy_id,
                         adapter_config[i].i2c.port_id);

            p_this = create_rtk_i2c_handle(i,adapter_config[i].i2c.phy_id,
                        rtk_MASTER_7BIT_ADDR,ADDR_MODE_7BITS,SPD_MODE_SS,
                        0);
            if (p_this == NULL)
                continue;

            if (p_this->init(p_this) < 0 ||
                i2c_rtk_register_adapter(p_this, adapter_config[i].i2c.port_id) < 0)
            {
                destroy_rtk_i2c_handle(p_this);
            }
        }
    }

    return 0;
}

#endif

