/* ------------------------------------------------------------------------- */
/* i2c-rtk-priv.c  rtk i2c hw driver for Realtek rtk DVR I2C           */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 2008 Kevin Wang <kevin_wang@realtek.com.tw>

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
-------------------------------------------------------------------------
Update List :
-------------------------------------------------------------------------
    2.1     |   20100511    | Support GPIO Read Write
-------------------------------------------------------------------------*/
#ifndef BUILD_QUICK_SHOW
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/i2c.h>
#include <asm/irq.h>
#include <asm/delay.h>
#else
#include <sysdefs.h>
#include <string.h>
#include <vsprintf.h>
#include <no_os/export.h>
#include <no_os/spinlock.h>
#include <no_os/slab.h>
#include <errno.h>
#include <no_os/jiffies.h>
#include <timer.h>
#endif

#include <mach/platform.h>
#include <rtk_kdriver/rtk_gpio.h>
#include <rbus/iso_misc_off_reg.h>

#include "rtk_i2c_priv.h"
#include "rtk_i2c_config.h"
#include "rtk_i2c_priv_g2c.h"
#include "rtk_i2c.h"

#if defined(CONFIG_REALTEK_INT_MICOM)
#include <rtk_kdriver/rtk_emcu_export.h>
#endif

#define I2C_STAY_ISR_LOOP (100)

typedef struct{
    char            name[20];
    int             logical_id;
    unsigned char   addr[7];
    int             addr_cnt;
}rtk_i2c_dev_info_t;
rtk_i2c_dev_info_t  rtk_i2c_dev_info[I2C_DEV_MAX_CNT] = {0};
int                 rtk_i2c_dev_cnt = 0;

extern rtk_i2c_adapter *find_rtk_i2c_adapter_by_id(unsigned char id);

unsigned long rtk_i2c_flags = 0;
rtk_i2c *rtk_i2c_phy_handle[I2C_PHY_CNT] = { NULL };

int rtk_i2c_get_mapped_gpio(rtk_i2c * p_this, RTK_GPIO_ID * p_sda,
                            RTK_GPIO_ID * p_scl)
{
    if (rtk_i2c_has_mapped_gpio(p_this))
    {
        *p_sda = p_this->current_port->g2c_sda;
        *p_scl = p_this->current_port->g2c_scl;
        return 0;
    }

    return -1;
}

int rtk_i2c_get_dev_info_by_pcbenum(void)
{
    int i = 0;
    int dev_name_len = 0;
    int bit_ofst = 0;
    int dev_cnt = 0;

    for(i = 0; i < PCB_ENUM_MAX; i++)
    {
        if(memcmp(pcb_enum_all[i].name, "I2C_DEV_", 8) == 0)
        {
            rtk_i2c_dev_info[dev_cnt].logical_id = pcb_enum_all[i].value & 0xff;    //get logical_id

            memset(rtk_i2c_dev_info[dev_cnt].name, 0, sizeof(rtk_i2c_dev_info[dev_cnt].name));
            dev_name_len = strlen(pcb_enum_all[i].name) - 8;
            if (dev_name_len >= sizeof(rtk_i2c_dev_info[dev_cnt].name))
            {
                dev_name_len = sizeof(rtk_i2c_dev_info[dev_cnt].name) - 1;
            }
            strncpy(rtk_i2c_dev_info[dev_cnt].name, &pcb_enum_all[i].name[8], dev_name_len);  //get I2C dev name

            bit_ofst = 8;
            rtk_i2c_dev_info[dev_cnt].addr_cnt = 0;
            while((rtk_i2c_dev_info[dev_cnt].addr_cnt < 7) && (0 != ((pcb_enum_all[i].value >> bit_ofst) & 0xff)))
            {
                //get I2C dev addr(8-bit) and addr_cnt
                rtk_i2c_dev_info[dev_cnt].addr[rtk_i2c_dev_info[dev_cnt].addr_cnt++] =
                                                                    (pcb_enum_all[i].value >> bit_ofst) & 0xff;
                bit_ofst += 8;
            }
            dev_cnt++;
        }

        if (dev_cnt >= I2C_DEV_MAX_CNT)
        {
            I2C_WARNING("[%s] dev cnt more than %d\n", __func__, I2C_DEV_MAX_CNT);
            break;
        }
    }
    I2C_WARNING("[%s] There're %d devices\n", __func__, dev_cnt);

    rtk_i2c_dev_cnt = dev_cnt;

    return rtk_i2c_dev_cnt;
}

 /*------------------------------------------------------------------
 * Func : rtk_i2c_get_dev_name
 *
 * Desc : get i2c device name
 *
 * Parm : logiacl_id: p_this->logical_id
 *        addr      : slave device address(8-bit)
 * Retn : dev_name
 *------------------------------------------------------------------*/
char *rtk_i2c_get_dev_name(unsigned char logiacl_id, unsigned char addr)
{
    int         i = 0, j = 0;
    int         no_addr_dev_idx[10] = {0};
    int         no_addr_dev_cnt = 0;
    static char no_addr_dev[64] = {0};
    int         len = 0;
    int         ofst = 0;

    for (i = 0; i < rtk_i2c_dev_cnt; i++)
    {
        if ((logiacl_id == rtk_i2c_dev_info[i].logical_id) && (rtk_i2c_dev_info[i].name[0] != 0))
        {
            if (0 != rtk_i2c_dev_info[i].addr_cnt)
            {
                for (j = 0; j < rtk_i2c_dev_info[i].addr_cnt; j++)
                {
                    if (addr == rtk_i2c_dev_info[i].addr[j])
                    {
                        return rtk_i2c_dev_info[i].name;    //returns the matching dev name
                    }
                }
            }
            else if (no_addr_dev_cnt < (sizeof(no_addr_dev_idx) / sizeof(int)))
            {
                no_addr_dev_idx[no_addr_dev_cnt++] = i;
            }
        }
    }

    if (no_addr_dev_cnt)
    {
        memset(no_addr_dev, 0, sizeof(no_addr_dev));
        len = snprintf(no_addr_dev, sizeof(no_addr_dev), "may be");
        ofst = len;
        for (i = 0; i < no_addr_dev_cnt; i++)
        {
            if ((ofst + strlen(rtk_i2c_dev_info[no_addr_dev_idx[i]].name)) > sizeof(no_addr_dev))
            {
                break;
            }
            if ((no_addr_dev_cnt - 1) == i)
            {
                len = snprintf(no_addr_dev + ofst, sizeof(no_addr_dev) - ofst, " %s", rtk_i2c_dev_info[no_addr_dev_idx
[i]].name);
            }
            else
            {
                len = snprintf(no_addr_dev + ofst, sizeof(no_addr_dev) - ofst, " %s or", rtk_i2c_dev_info[
no_addr_dev_idx[i]].name);
            }
            ofst += len;
        }
        return no_addr_dev;
    }

    return "Unknown";
}


/*------------------------------------------------------------------
 * Func : rtk_i2c_msater_write
 *
 * Desc : master write handler for rtk i2c
 *
 * Parm : p_this : handle of rtk i2c
 *        event  : INT event of rtk i2c
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_i2c_msater_write(rtk_i2c * p_this, unsigned int event,
                          unsigned int tx_abort_source)
{
#define TxComplete()              (p_this->xfer.tx_len >= p_this->xfer.tx_buff_len)

    while (!TxComplete() && NOT_TXFULL(p_this))
    {
        unsigned long Command =
            p_this->xfer.tx_buff[p_this->xfer.tx_len++];

        if (TxComplete())
            Command |= (0x1 << 9);

        SET_IC_DATA_CMD(p_this, Command);
    }

    if (TxComplete())
    {
        SET_IC_INTR_MASK(p_this,GET_IC_INTR_MASK(p_this) & ~TX_EMPTY_BIT);
    }

    if (event & TX_ABRT_BIT)
    {
        p_this->xfer.ret = -ETXABORT;
        p_this->xfer.tx_abort_source = tx_abort_source;
    }
    else if (event & STOP_DET_BIT)
    {
        if (GET_IC_GUARD_INTERVAL(p_this) & GD_INTERVAL_INT_EN_BIT)
        {
            if (GET_IC_GUARD_INTERVAL_ST(p_this) & GD_INTERVAL_ST_BIT)
            {
                p_this->xfer.ret = TxComplete()? p_this->xfer.tx_len : -ECMDSPLIT;
            }
        }
        else
        {
            p_this->xfer.ret = TxComplete()? p_this->xfer.tx_len : -ECMDSPLIT;
        }
    }

    if (p_this->xfer.ret)
    {
        SET_IC_INTR_MASK(p_this, 0);
        SET_IC_ENABLE(p_this, 0);

        p_this->xfer.mode = I2C_IDEL;
        if (GET_IC_GUARD_INTERVAL(p_this) & GD_INTERVAL_INT_EN_BIT)
        {
            CLR_IC_GUARD_INTERVAL(p_this);
        }
        wake_up(&p_this->wq);
    }
#undef TxComplete
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_msater_read
 * Desc : master read handler for rtk i2c
 * Parm : p_this : handle of rtk i2c
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_i2c_msater_read(rtk_i2c * p_this, unsigned int event,
                         unsigned int tx_abort_source)
{
#define TxComplete()        (p_this->xfer.tx_len >= p_this->xfer.rx_buff_len)
#define RxComplete()        (p_this->xfer.rx_len >= p_this->xfer.rx_buff_len)

    while (!RxComplete() && NOT_RXEMPTY(p_this))
    {
        p_this->xfer.rx_buff[p_this->xfer.rx_len++] = (unsigned char)(GET_IC_DATA_CMD(p_this) & 0xFF);
    }

    while (!TxComplete() && NOT_TXFULL_EX(p_this))
    {
        unsigned long Command = READ_CMD;
        p_this->xfer.tx_len++;

        if (TxComplete())
            Command |= (0x1 << 9);

        SET_IC_DATA_CMD(p_this, Command);
    }

    if (TxComplete())
    {
        SET_IC_INTR_MASK(p_this,GET_IC_INTR_MASK(p_this) & ~TX_EMPTY_BIT);
    }

    if (event & TX_ABRT_BIT)
    {
        p_this->xfer.ret = -ETXABORT;
        p_this->xfer.tx_abort_source = tx_abort_source;
    }
    else if ((event & STOP_DET_BIT) || RxComplete())
    {
        SET_IC_INTR_MASK(p_this, GET_IC_INTR_MASK(p_this) & ~RX_FULL_BIT);

        p_this->xfer.ret = RxComplete()? p_this->xfer.rx_len : -ECMDSPLIT;
    }

    if (p_this->xfer.ret)
    {
        SET_IC_INTR_MASK(p_this, 0);
        SET_IC_ENABLE(p_this, 0);
        p_this->xfer.mode = I2C_IDEL;
        wake_up(&p_this->wq);
    }
#undef TxComplete
#undef RxComplete
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_msater_read
 *
 * Desc : master read handler for rtk i2c
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_i2c_msater_random_read(rtk_i2c * p_this, unsigned int event,
                                unsigned int tx_abort_source)
{

#define TxComplete()        (p_this->xfer.tx_len >= (p_this->xfer.rx_buff_len + p_this->xfer.tx_buff_len))
#define RxComplete()        (p_this->xfer.rx_len >=  p_this->xfer.rx_buff_len)

    while (!RxComplete() && NOT_RXEMPTY(p_this))
    {
        p_this->xfer.rx_buff[p_this->xfer.rx_len++] = (unsigned char)(GET_IC_DATA_CMD(p_this) & 0xFF);
    }

    while (!TxComplete() && NOT_TXFULL_EX(p_this))
    {
        unsigned long Command;

        if (p_this->xfer.tx_len < p_this->xfer.tx_buff_len)
            Command = p_this->xfer.tx_buff[p_this->xfer.tx_len];
        else
            Command = READ_CMD;

        p_this->xfer.tx_len++;

        if (TxComplete())
            Command |= (0x1 << 9);

        SET_IC_DATA_CMD(p_this, Command);
    }

    if (TxComplete())
    {
        SET_IC_INTR_MASK(p_this, GET_IC_INTR_MASK(p_this) & ~TX_EMPTY_BIT);
    }

    if (event & TX_ABRT_BIT)
    {
        p_this->xfer.ret = -ETXABORT;
        p_this->xfer.tx_abort_source = tx_abort_source;
    }
    else if ((event & STOP_DET_BIT) || RxComplete())
    {
        SET_IC_INTR_MASK(p_this, GET_IC_INTR_MASK(p_this) & ~RX_FULL_BIT);
        p_this->xfer.ret = RxComplete()? p_this->xfer.rx_len : -ECMDSPLIT;
    }

    if (p_this->xfer.ret)
    {
        SET_IC_INTR_MASK(p_this, 0);
        SET_IC_ENABLE(p_this, 0);
        p_this->xfer.mode = I2C_IDEL;
        wake_up(&p_this->wq);
    }
#undef TxComplete
#undef RxComplete
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_isr
 * Desc : isr of rtk i2c
 * Parm : p_this : handle of rtk i2c
 * Retn : 0
 *------------------------------------------------------------------*/
static irqreturn_t rtk_i2c_isr(int this_irq, void *dev_id)
{
    rtk_i2c *p_this = (rtk_i2c *) dev_id;
    unsigned long flags;
    unsigned int event = 0;
    unsigned int tx_abrt_source = 0;
    unsigned char slave_rx_stop_flag = 0;

    LOCK_rtk_I2C(&p_this->lock, flags);

    if (!(GET_IC_ISR(p_this) & p_this->reg_map.IC_INT))
    {
        UNLOCK_rtk_I2C(&p_this->lock, flags);
        return IRQ_NONE;
    }

    LOG_EVENT(EVENT_ENTER_ISR);

    event = GET_IC_INTR_STAT(p_this);
    tx_abrt_source = GET_IC_TX_ABRT_SOURCE(p_this);

    CLR_IC_INTR(p_this);

    if ((GET_IC_CON(p_this) & IC_SLAVE_DISABLE) == 0)
    {
        while (NOT_RXEMPTY(p_this))
        {
            if (p_this->slave_rx_len < sizeof(p_this->slave_rx_buffer))
            {
                p_this->slave_rx_buffer[p_this->slave_rx_len++] =
                                            (unsigned char)(GET_IC_DATA_CMD(p_this) & 0xFF);
            }
            else
            {
                if (p_this->slave_ops.handle_command)
                {
                    p_this->slave_ops.handle_command(p_this->slave_id,
                                                    p_this->slave_rx_buffer,
                                                    p_this->slave_rx_len,
                                                    slave_rx_stop_flag);
                }
                p_this->slave_rx_len = 0;
            }
        }

        if (event & (STOP_DET_BIT | RD_REQ_BIT | START_DET_BIT) && p_this->slave_rx_len)
        {
            slave_rx_stop_flag  = 1;
            if (p_this->slave_ops.handle_command)
            {
                p_this->slave_ops.handle_command(p_this->slave_id,
                                                p_this->slave_rx_buffer,
                                                p_this->slave_rx_len,
                                                slave_rx_stop_flag);
            }
            slave_rx_stop_flag = 0;
            p_this->slave_rx_len = 0;
        }

        if (event & RD_REQ_BIT)
        {
            if (p_this->slave_ops.read_data)
                SET_IC_DATA_CMD(p_this,p_this->slave_ops.read_data(p_this->slave_id));
            else
                SET_IC_DATA_CMD(p_this, 0xFF);
        }

        if ((event & (STOP_DET_BIT | RX_DONE_BIT)))
        {
            if (p_this->slave_ops.stop)
                p_this->slave_ops.stop(p_this->slave_id);

            if (event & RX_DONE_BIT)
            {
                SET_IC_ENABLE(p_this, 0);
                udelay(10);
                SET_IC_ENABLE(p_this, 1);
            }
        }
    }
    else
    {
        switch (p_this->xfer.mode)
        {
        case I2C_MASTER_WRITE:
            rtk_i2c_msater_write(p_this, event, tx_abrt_source);
            break;

        case I2C_MASTER_READ:
            rtk_i2c_msater_read(p_this, event, tx_abrt_source);
            break;

        case I2C_MASTER_RANDOM_READ:
            rtk_i2c_msater_random_read(p_this, event, tx_abrt_source);
            break;

        default:
            I2C_WARNING("%s Unexcepted Interrupt\n", __func__);
            SET_IC_ENABLE(p_this, 0);
        }
    }

    SET_IC_ISR(p_this, p_this->reg_map.IC_INT);
    UNLOCK_rtk_I2C(&p_this->lock, flags);

    return IRQ_HANDLED;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_set_tar
 *
 * Desc : set tar of rtk i2c
 *
 * Parm : p_this : handle of rtk i2c
 *        addr   : address of sar
 *        mode
  : mode of sar
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_set_tar(rtk_i2c * p_this, unsigned short addr, ADDR_MODE mode)
{
    if (mode == ADDR_MODE_10BITS)
    {
        if (addr > 0x3FF)
        {
            I2C_DEBUG("[ch_%d][phy_%d] Xfer fail - ADDRESS OUT OF RANGE (1 ,address 10bits), %s Line%d\n",
                        p_this->logical_id, p_this->id, __func__, __LINE__ );
            return -EADDROVERRANGE;
        }

        SET_IC_ENABLE(p_this, 0);
        SET_IC_TAR(p_this, addr & 0x3FF);
        SET_IC_CON(p_this, (GET_IC_CON(p_this) & (~IC_10BITADDR_MASTER)) | IC_10BITADDR_MASTER);
    }
    else
    {
        if (addr > 0x7F)
        {
            I2C_DEBUG("[ch_%d][phy_%d] Xfer fail - ADDRESS OUT OF RANGE(2 , address 7bits ) , %s Line%d\n",
                        p_this->logical_id, p_this->id, __func__, __LINE__ );
            return -EADDROVERRANGE;
        }

        SET_IC_ENABLE(p_this, 0);
        SET_IC_TAR(p_this, addr & 0x7F);
        SET_IC_CON(p_this, GET_IC_CON(p_this) & (~IC_10BITADDR_MASTER));
    }

    p_this->tar = addr;
    p_this->tar_mode = mode;

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_slave_mode_enable
 *
 * Desc : enable/disable i2c slave mode
 *
 * Parm : p_this : handle of rtk i2c
 *        on     : enable /disable
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_slave_mode_enable(rtk_i2c * p_this, unsigned char on)
{
    unsigned long flags;
    rtk_i2c_adapter *p_adp = NULL;
    p_adp = find_rtk_i2c_adapter_by_id(p_this->logical_id);
    if (NULL == p_adp)
    {
        I2C_WARNING("%s line: %d, find_rtk_i2c_adapter_by_id error\n", __func__, __LINE__);
        return -ENODEV;
    }

    LOCK_rtk_I2C(&p_this->lock, flags);

    if (on)
    {
        SET_IC_ENABLE(p_this, 0);
        p_this->set_sar(p_this, p_this->sar, p_this->sar_mode);
        I2C_WARNING("[ch_%d][phy_%d] i2c slave enabled, sar=%x\n", p_this->logical_id, p_this->id, GET_IC_SAR(p_this));

        SET_IC_CON(p_this, GET_IC_CON(p_this) & ~(IC_SLAVE_DISABLE | IC_MASTER_MODE));
        SET_IC_INTR_MASK(p_this, START_DET_BIT | STOP_DET_BIT | RD_REQ_BIT | RX_FULL_BIT | RX_DONE_BIT);

        if (p_adp->port >= 0 && p_this->set_port(p_this, p_adp->port) < 0)
        {
            I2C_WARNING("[%s] set port fail\n", __func__);
            return -EACCES;
        }

        SET_IC_ENABLE(p_this, 1);
        p_this->flags |= rtk_I2C_SLAVE_ENABLE;
    }
    else
    {
        I2C_WARNING("[ch_%d][phy_%d] i2c slave disabled\n", p_this->logical_id, p_this->id);
        SET_IC_ENABLE(p_this, 0);
        SET_IC_CON(p_this, GET_IC_CON(p_this) | (IC_SLAVE_DISABLE | IC_MASTER_MODE));
        SET_IC_INTR_MASK(p_this, 0);

        if (p_adp->port >= 0)
        {
            p_this->unset_port(p_this, p_adp->port);
        }
        p_this->flags &= ~rtk_I2C_SLAVE_ENABLE;
    }

    UNLOCK_rtk_I2C(&p_this->lock, flags);

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_register_slave_ops
 *
 * Desc : register slave mode ops
 *
 * Parm : p_this : handle of rtk i2c
 *        ops    : slave mode ops
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_register_slave_ops(rtk_i2c * p_this, rtk_i2c_slave_ops * ops, unsigned long id)
{
    unsigned long flags;

    LOCK_rtk_I2C(&p_this->lock, flags);

    if (ops == NULL)
    {
        p_this->slave_ops.handle_command = NULL;
        p_this->slave_ops.read_data = NULL;
        p_this->slave_ops.stop = NULL;
        p_this->slave_id = 0;
    }
    else
    {
        p_this->slave_ops.handle_command = ops->handle_command;
        p_this->slave_ops.read_data = ops->read_data;
        p_this->slave_ops.stop = ops->stop;
        p_this->slave_id = id;
    }

    UNLOCK_rtk_I2C(&p_this->lock, flags);
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_set_port
 *
 * Desc : set port of rtk i2c
 *
 * Parm : p_this  : handle of rtk i2c
 *        port_id : output port selection
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_set_port(rtk_i2c * p_this, unsigned char port_id)
{
    rtk_i2c_port *port = NULL;

#if defined(CONFIG_REALTEK_INT_MICOM)
    if(p_this->id == LG_NVRAM_I2C_CHANNEL)
         return 0;
#endif

    if(port_id == 255)
        return 0;

    if (port_id >= p_this->n_port)
    {
        I2C_WARNING("[ch_%d][phy_%d] WARNING set , zap to port %d failed, invalid port number\n",
                    p_this->logical_id, p_this->id, port_id);
        return -EFAULT;
    }

    port = &p_this->p_port[port_id];

    if (port->input_mux[0].addr)
    {
        wr_reg(port->input_mux[0].addr,
               (rd_reg(port->input_mux[0].addr) & ~port->input_mux[0].mask) | port->input_mux[0].val);
    }

    if (port->input_mux[1].addr)
    {
        wr_reg(port->input_mux[1].addr,
               (rd_reg(port->input_mux[1].addr) & ~port->input_mux[1].mask) | port->input_mux[1].val);
    }

    if (port->pin_mux[0].addr)
    {    wr_reg(port->pin_mux[0].addr,
               (rd_reg(port->pin_mux[0].addr) & ~port->pin_mux[0].mask) | port->pin_mux[0].i2c_val);
    }

    if (port->pin_mux[1].addr)
    {    wr_reg(port->pin_mux[1].addr,
               (rd_reg(port->pin_mux[1].addr) & ~port->pin_mux[1].mask) | port->pin_mux[1].i2c_val);
    }

    p_this->current_port = port;

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_unset_port
 *
 * Desc : set port of rtk i2c
 *
 * Parm : p_this  : handle of rtk i2c
 *        port_id : output port selection
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_unset_port(rtk_i2c * p_this, unsigned char port_id)
{
     rtk_i2c_port *port = NULL;

#if defined(CONFIG_REALTEK_INT_MICOM)
    if(p_this->id == LG_NVRAM_I2C_CHANNEL)
         return 0;
#endif

    if(port_id == 255)
        return 0;

    if (port_id >= p_this->n_port)
    {
        I2C_WARNING
        ("[ch_%d][phy_%d] WARNING unset, zap to port %d failed, invalid port number\n",
         p_this->logical_id, p_this->id, port_id);
        return -EFAULT;
    }

    port = &p_this->p_port[port_id];

    if (port->input_mux[0].addr)
    {
        wr_reg(port->input_mux[0].addr,
               rd_reg(port->input_mux[0].addr) | port->input_mux[0].mask);
    }

    if (port->input_mux[1].addr)
    {
        wr_reg(port->input_mux[1].addr,
               rd_reg(port->input_mux[1].addr) | port->input_mux[1].mask);
    }

    if (port->pin_mux[0].addr)
    {
        wr_reg(port->pin_mux[0].addr,
                   rd_reg(port->pin_mux[0].addr) | port->pin_mux[0].mask);
    }

    if (port->pin_mux[1].addr)
    {
        wr_reg(port->pin_mux[1].addr,
               rd_reg(port->pin_mux[1].addr) | port->pin_mux[1].mask);
    }

    p_this->current_port = port;

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_set_sar
 *
 * Desc : set sar of rtk i2c
 *
 * Parm : p_this : handle of rtk i2c
 *        addr   : address of sar
 *        mode
  : mode of sar
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_set_sar(rtk_i2c * p_this, unsigned short addr, ADDR_MODE mode)
{
    if (mode == ADDR_MODE_10BITS)
    {
        SET_IC_ENABLE(p_this, 0);
        SET_IC_SAR(p_this, p_this->sar & 0x3FF);
        SET_IC_CON(p_this, GET_IC_CON(p_this) | IC_10BITADDR_SLAVE);
    }
    else
    {
        SET_IC_ENABLE(p_this, 0);
        SET_IC_SAR(p_this, p_this->sar & 0x7F);
        SET_IC_CON(p_this, GET_IC_CON(p_this) & (~IC_10BITADDR_SLAVE));
    }

    p_this->sar = addr;
    p_this->sar_mode = mode;

    return 0;
}

void rtk_i2c_clk_sel(rtk_i2c * p_this, int mode)
{
    if(GET_IC_CLKSEL_MODE(p_this)==mode)
        return;
    if (mode==CLKSEL_MODE_62_5MHZ)
        wr_reg(p_this->reg_map.IC_CLKSEL, rd_reg(p_this->reg_map.IC_CLKSEL) | p_this->reg_map.IC_CLKSEL_MASK);
    else
        wr_reg(p_this->reg_map.IC_CLKSEL, rd_reg(p_this->reg_map.IC_CLKSEL) & (~p_this->reg_map.IC_CLKSEL_MASK));
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_set_spd
 *
 * Desc : set speed of rtk i2c
 *
 * Parm : p_this : handle of rtk i2c
 *        KHz    : operation speed of i2c
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_set_spd(rtk_i2c * p_this, int KHz)
{
    /*100Hz*/
    unsigned int div_h = 0x84;
    unsigned int div_l = 0x84;
    CLKSEL_MODE  mode = CLKSEL_MODE_27MHZ;

    if (p_this->fixed_speed!=0)
        return 0;

    if (KHz < 10 || KHz > 1200)
    {
        I2C_WARNING("[ch_%d][phy_%d], speed %d out of range, speed should between 10 ~ 800KHz , %s Line%d\n",
                    p_this->logical_id, p_this->id, KHz,__func__,  __LINE__);
        return -1;
    }
    if (KHz >= 600)
    {
        mode = CLKSEL_MODE_62_5MHZ;
    }
    rtk_i2c_clk_sel(p_this,mode);
    /*
    div_h = (div_h * 100) / KHz;
    div_l = (div_l * 100) / KHz;

    if (div_h >= 0xFFFF || div_h == 0 || div_l >= 0xFFFF || div_l == 0) {
    	I2C_WARNING
    	    ("[ch_%d][phy_%d]set speed failed : divider divider out of range. div_h = %d, div_l = %d ,%s ,Line%d\n",
    	    p_this->logical_id, p_this->id, div_h, div_l,__func__,__LINE__);
    	return -1;
    }
    */
    if (KHz >= 700) {
        SET_IC_PH_CR(p_this,(GET_IC_PH_CR(p_this) & (~MISC_I2C_I2C1_PH_CR_ph_en_mask)) | (1 << MISC_I2C_I2C1_PH_CR_ph_en_shift));
        SET_IC_PH_CR(p_this,(GET_IC_PH_CR(p_this) & (~MISC_I2C_I2C1_PH_CR_sda_vth_mask)) | (0 << MISC_I2C_I2C1_PH_CR_sda_vth_shift));
        SET_IC_PH_CR(p_this,(GET_IC_PH_CR(p_this) & (~MISC_I2C_I2C1_PH_CR_scl_vth_mask)) | (0 << MISC_I2C_I2C1_PH_CR_scl_vth_shift));
        SET_IC_PH_CR(p_this,(GET_IC_PH_CR(p_this) & (~MISC_I2C_I2C1_PH_CR_sda_value_mask)) | (1 << MISC_I2C_I2C1_PH_CR_sda_value_shift));
        SET_IC_PH_CR(p_this,(GET_IC_PH_CR(p_this) & (~MISC_I2C_I2C1_PH_CR_scl_value_mask)) | (1 << MISC_I2C_I2C1_PH_CR_scl_value_shift));
    }

    SET_IC_ENABLE(p_this, 0);
    if (KHz >= 1000)
    {
        SET_IC_CON(p_this,(GET_IC_CON(p_this) & (~IC_SPEED_MASK)) | SPEED_FS);
        SET_IC_FS_SCL_HCNT(p_this, 0x8);
        SET_IC_FS_SCL_LCNT(p_this, 0x26);
        SET_IC_SDA_HOLD(p_this, 0x3);
    }
    else if (KHz >= 700)
    {
        SET_IC_CON(p_this,(GET_IC_CON(p_this) & (~IC_SPEED_MASK)) | SPEED_SS);
        SET_IC_SS_SCL_HCNT(p_this, 0x10);
        SET_IC_SS_SCL_LCNT(p_this, 0x30);
        SET_IC_SDA_HOLD(p_this, 0x8);
    }
    else if (KHz >= 600)
    {
        SET_IC_CON(p_this,(GET_IC_CON(p_this) & (~IC_SPEED_MASK)) | SPEED_SS);
        SET_IC_SS_SCL_HCNT(p_this, 0x1E);
        SET_IC_SS_SCL_LCNT(p_this, 0x3C);
        SET_IC_SDA_HOLD(p_this, 0x1E);
    }
    else if (KHz >= 400)
    {
        SET_IC_CON(p_this,(GET_IC_CON(p_this) & (~IC_SPEED_MASK)) | SPEED_SS);
        SET_IC_SS_SCL_HCNT(p_this, 0x16);
        SET_IC_SS_SCL_LCNT(p_this, 0x25);
    }
    else if (KHz == 50)
    {
        SET_IC_CON(p_this,(GET_IC_CON(p_this) & (~IC_SPEED_MASK)) | SPEED_SS);
        SET_IC_SS_SCL_HCNT(p_this, 0x108);
        SET_IC_SS_SCL_LCNT(p_this, 0x108);
    }
    else
    {
        SET_IC_CON(p_this,(GET_IC_CON(p_this) & (~IC_SPEED_MASK)) | SPEED_SS);
        SET_IC_SS_SCL_HCNT(p_this, div_h);
        SET_IC_SS_SCL_LCNT(p_this, div_l);
    }

    p_this->spd = KHz;
    p_this->tick = 1000 / KHz;

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_set_guard_interval
 * Desc : set guard_interval of rtk i2c
 * Parm : p_this : handle of rtk i2c
 *        us     : operation speed of i2c
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_set_guard_interval(rtk_i2c * p_this, unsigned long us)
{
    p_this->guard_interval = us;
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_set_hw_guard_interval
 * Desc : set guard_interval of rtk i2c
 * Parm : p_this : handle of rtk i2c
 *        us     : operation speed of i2c
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_set_hw_guard_interval(rtk_i2c * p_this, unsigned long us)
{
    CLR_IC_GUARD_INTERVAL(p_this);
    if (us == 400)
    {
        SET_IC_GUARD_INTERVAL(p_this,
            GD_INTERVAL_400US | GD_INTERVAL_EN_BIT | GD_INTERVAL_INT_EN_BIT);
    }
    else if (us == 600)
    {
        SET_IC_GUARD_INTERVAL(p_this,
            GD_INTERVAL_600US | GD_INTERVAL_EN_BIT | GD_INTERVAL_INT_EN_BIT);
    }
    else if (us == 800)
    {
        SET_IC_GUARD_INTERVAL(p_this,
            GD_INTERVAL_800US | GD_INTERVAL_EN_BIT | GD_INTERVAL_INT_EN_BIT);
    }
    else if (us == 1000)
    {
        SET_IC_GUARD_INTERVAL(p_this,
            GD_INTERVAL_1000US | GD_INTERVAL_EN_BIT | GD_INTERVAL_INT_EN_BIT);
    }
    else if (us == 2000)
    {
        SET_IC_GUARD_INTERVAL(p_this,
            GD_INTERVAL_2000US | GD_INTERVAL_EN_BIT | GD_INTERVAL_INT_EN_BIT);
    }
    else
    {
        SET_IC_GUARD_INTERVAL(p_this, 0x0);
    }

#if defined(CONFIG_ARCH_RTK284X)
    SET_IC_GUARD_INTERVAL(p_this,
        ( GET_IC_GUARD_INTERVAL(p_this) & (~GD_INTERVAL_SEL_BIT) | GD_INTERVAL_SEL(p_this->clksel_mode) ));
#endif
    CLR_IC_GUARD_INTERVAL(p_this);

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_dump
 * Desc : dump staus of rtk i2c
 * Parm : p_this : handle of rtk i2c
 * Retn : 0 for success
 *------------------------------------------------------------------*/
int rtk_i2c_dump(rtk_i2c * p_this)
{
    RTK_GPIO_ID scl;
    RTK_GPIO_ID sda;

    I2C_WARNING( "=====%s Line%d====================\n", __func__, __LINE__ );
    I2C_WARNING( "= VER : %s               \n", VERSION);
    I2C_WARNING( "=========================\n");
    I2C_WARNING( "= CH : %d                \n", p_this->logical_id);
    I2C_WARNING( "= PHY : %d               \n", p_this->id);
    I2C_WARNING( "= PORT: %d               \n",
                 (uint32_t) current_port_id(p_this));
    I2C_WARNING( "= MODEL: %s              \n", p_this->model_name);
    I2C_WARNING( "= SPD : %d               \n", p_this->spd);
    I2C_WARNING( "= SAR : 0x%03x (%d bits) \n", p_this->sar, p_this->sar_mode);
    I2C_WARNING( "= TX FIFO DEPTH : %d     \n", p_this->tx_fifo_depth);
    I2C_WARNING( "= RX FIFO DEPTH : %d     \n", p_this->rx_fifo_depth);
    I2C_WARNING( "= FIFO THRESHOLD: %d     \n", FIFO_THRESHOLD);

    if (rtk_i2c_get_mapped_gpio(p_this, &sda, &scl) == 0)
    {
        I2C_WARNING( "= SDA GPIO : %s_GPIO %d\n", gpio_type(gpio_group(sda)), gpio_idx(sda));
        I2C_WARNING( "= SCL GPIO : %s_GPIO %d\n", gpio_type(gpio_group(scl)), gpio_idx(scl));

#ifdef CONFIG_RTK_KDRV_I2C_BUS_JAM_RECOVER
        I2C_WARNING( "= BUS JAM RECORVER 3: ON  \n");
#else
        I2C_WARNING( "= BUS JAM RECORVER 3: OFF  \n");
#endif

#ifdef CONFIG_RTK_KDRV_I2C_NON_STOP_WRITE_XFER
        I2C_WARNING( "= NON STOP WRITE : ON  \n");
#else
        I2C_WARNING( "= NON STOP WRITE : OFF  \n");
#endif

        I2C_WARNING( "= GPIO RW SUPPORT : ON \n");
        I2C_WARNING( "= SET GPIO MODE HIGH.\n");
    }

    I2C_WARNING( "=========================\n");

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_find_current_port
 *
 * Desc : fi
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0
 *------------------------------------------------------------------*/
rtk_i2c_port *rtk_i2c_find_current_port(rtk_i2c * p_this)
{
    rtk_i2c_port *p_port = p_this->p_port;
    int i;

    if (p_port == NULL)
        return NULL;

    for (i = 0; i < p_this->n_port; i++)
    {
        if (p_port[i].input_mux[0].addr)
        {
            if ((rd_reg(p_port[i].input_mux[0].addr) & p_port[i].input_mux[0].mask) !=
                        p_port[i].input_mux[0].val)
                continue;
        }

        if (p_port[i].input_mux[1].addr)
        {
            if ((rd_reg(p_port[i].input_mux[1].addr) & p_port[i].input_mux[1].mask) !=
                        p_port[i].input_mux[1].val)
                continue;
        }

        if (p_port[i].pin_mux[0].addr)
        {
            if ((rd_reg(p_port[i].pin_mux[0].addr) & p_port[i].pin_mux[0].mask) !=
                        p_port[i].pin_mux[0].i2c_val)
                continue;
        }

        if (p_port[i].pin_mux[1].addr)
        {
            if ((rd_reg(p_port[i].pin_mux[1].addr) & p_port[i].pin_mux[1].mask) !=
                        p_port[i].pin_mux[1].i2c_val)
                continue;
        }

        return &p_port[i];
    }

    return NULL;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_get_hold_time
 *
 * Desc : i2c get SDA hold_time
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_get_hold_time(rtk_i2c *p_this)
{
    return GET_IC_SDA_HOLD(p_this);
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_set_hold_time
 *
 * Desc : i2c set SDA hold_time
 *
 * Parm : p_this : handle of rtk i2c
 *        val : set value
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_set_hold_time(rtk_i2c *p_this, int val)
{
	SET_IC_SDA_HOLD(p_this, val);
	I2C_WARNING("Set Hold Time: 0x%x\n", val);

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_get_setup_time
 *
 * Desc : i2c get SDA setup_time
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_get_setup_time(rtk_i2c *p_this)
{
    return GET_IC_SDA_SETUP(p_this);
}
/*------------------------------------------------------------------
 * Func : rtk_i2c_set_setup_time
 *
 * Desc : i2c set SDA setup_time
 *
 * Parm : p_this : handle of rtk i2c
 *        val : set value
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_set_setup_time(rtk_i2c *p_this, int val)
{
    SET_IC_SDA_SETUP(p_this, val);
    I2C_WARNING("Set Setup Time: 0x%x\n", val);

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_probe
 *
 * Desc : probe rtk i2c
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_init_probe(rtk_i2c * p_this)
{
    if (p_this->id >= I2C_PHY_CNT)
    {
        I2C_WARNING( "[ch_%d][phy_%d] %s : isn't exist, fail! Line%d\n", p_this->logical_id, p_this->id,
                     __func__,__LINE__);
        return -ENODEV;
    }

    p_this->model_name = MODLE_NAME;
    p_this->reg_map = *(i2c_phy[p_this->id].p_reg_map);
    p_this->n_port = i2c_phy[p_this->id].n_port;
    p_this->p_port = (rtk_i2c_port *) i2c_phy[p_this->id].p_port;
    p_this->current_port = rtk_i2c_find_current_port(p_this);
    p_this->n_init_reg = i2c_phy[p_this->id].n_init_reg;
    p_this->p_init_reg = i2c_phy[p_this->id].p_init_reg;

    I2C_WARNING("[ch_%d][phy_%d] %s : model name = %s\n", p_this->logical_id, p_this->id, __func__, p_this->model_name);

    if (p_this->current_port == NULL)
        I2C_WARNING("[ch_%d][phy_%d] WARNING. Pinmux isn't ready.Line%d\n", p_this->logical_id, p_this->id,__LINE__);

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_phy_init
 *
 * Desc : init rtk i2c phy
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_phy_init(rtk_i2c * p_this, bool bInfo)
{
    p_this->clksel_mode = CLKSEL_MODE_27MHZ;

    SET_IC_ENABLE(p_this, 0);
    SET_IC_INTR_MASK(p_this, 0);
    SET_IC_CON(p_this, IC_SLAVE_DISABLE | IC_RESTART_EN | SPEED_SS | IC_MASTER_MODE);

    SET_IC_TX_TL(p_this, FIFO_THRESHOLD);
    SET_IC_RX_TL(p_this, p_this->rx_fifo_depth - FIFO_THRESHOLD);

    if (p_this->n_init_reg && p_this->p_init_reg)
    {
        int i;
        for (i = 0; i < p_this->n_init_reg; i++)
        {
            unsigned long addr = p_this->p_init_reg[i].addr;
            unsigned long val = rd_reg(addr);

            val &= ~p_this->p_init_reg[i].mask;
            val |= p_this->p_init_reg[i].val;
            if(bInfo)
            {
                I2C_WARNING( "[ch_%d][phy_%d] setting: wr_reg(%08x)=%08x\n",
                             p_this->logical_id, p_this->id, (unsigned int)addr,(unsigned int)val );
            }

            wr_reg(addr, val);
        }
    }

    rtk_i2c_set_spd(p_this, p_this->spd);

    if(bInfo)
        rtk_i2c_dump(p_this);

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_init
 *
 * Desc : init rtk i2c
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_init(rtk_i2c * p_this)
{
    int ret;

    if (p_this->flags & rtk_I2C_IRQ_RDY)
        return 0;

    if (rtk_i2c_init_probe(p_this) < 0)
        return -ENODEV;

    p_this->flags = rtk_I2C_IRQ_RDY;
    p_this->rx_fifo_depth = ((GET_IC_COMP_PARAM_1(p_this) >> 8) & 0xFF) + 1;
    p_this->tx_fifo_depth = ((GET_IC_COMP_PARAM_1(p_this) >> 16) & 0xFF) + 1;

    init_waitqueue_head(&p_this->wq);
    spin_lock_init(&p_this->lock);
    mutex_init(&p_this->mutex);

    if((ret = rtk_i2c_phy_init(p_this,true)) < 0)
    {
        I2C_WARNING("rtk_i2c_phy_init failed!\n");
        return -ENODEV;
    }

    if ((ret =request_irq(p_this->irq, rtk_i2c_isr, SA_SHIRQ, "i2c",(void *)p_this)) < 0)
    {
        I2C_WARNING("Request irq%d failed(ret=%d)\n", p_this->irq, ret);
        return -ENODEV;
    }
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_uninit
 *
 * Desc : uninit rtk i2c
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_i2c_uninit(rtk_i2c * p_this)
{
    SET_IC_ENABLE(p_this, 0);
    SET_IC_INTR_MASK(p_this, 0);

    if (p_this->flags & rtk_I2C_IRQ_RDY)
    {
        free_irq(p_this->irq, p_this);
        p_this->flags = 0;
    }

    return 0;
}


/*------------------------------------------------------------------
 * Func : rtk_i2c_gpio_selection
 *
 * Desc : select i2c/GPIO mode
 *
 * Parm : p_this : handle of rtk i2c
 *        mode : 0      : SDA / SCL
 *               others : GPIO
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_i2c_gpio_selection(rtk_i2c * p_this, unsigned char mode)
{
    unsigned long val;
    int i;

    if (rtk_i2c_has_mapped_gpio(p_this))
    {
        for (i = 0; i < 2; i++)
        {
            if (p_this->current_port->pin_mux[i].addr)
            {
                val = rd_reg(p_this->current_port->pin_mux[i].addr);
                val &= ~p_this->current_port->pin_mux[i].mask;

                val |= (mode == GPIO_MODE) ?
                    p_this->current_port-> pin_mux[i].gpio_val : p_this->current_port->pin_mux[i].i2c_val;

                wr_reg(p_this->current_port->pin_mux[i].addr, val);
            }
        }
    }
    else
    {
        I2C_WARNING("rtk_i2c_gpio_selection : no gpio mapping,Line%d\n", __LINE__);
    }
}


/*------------------------------------------------------------------
 * Func : rtk_i2c_start_xfer
 *
 * Desc : start xfer message
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0 for success, others is failed
 *------------------------------------------------------------------*/
int rtk_i2c_start_xfer(rtk_i2c * p_this)
{
    unsigned long flags;
    int tempRawInt=0;
    int ret;
    int mode = p_this->xfer.mode;

    LOG_EVENT(EVENT_START_XFER);

    LOCK_rtk_I2C(&p_this->lock, flags);

    if ((GET_IC_CON(p_this) & IC_SLAVE_DISABLE) == 0)
    {
        SET_IC_ENABLE(p_this, 0);
        SET_IC_CON(p_this, GET_IC_CON(p_this) | (IC_SLAVE_DISABLE | IC_MASTER_MODE));
    }

    switch (p_this->xfer.mode)
    {
    case I2C_MASTER_WRITE:
        SET_IC_INTR_MASK(p_this,TX_EMPTY_BIT | TX_ABRT_BIT | STOP_DET_BIT);
        break;

    case I2C_MASTER_READ:
    case I2C_MASTER_RANDOM_READ:
        if (GET_IC_RXFLR(p_this))
        {
            I2C_WARNING("RX FIFO NOT EMPRY, %s Line%d\n", __func__, __LINE__);

            while (GET_IC_RXFLR(p_this))
                GET_IC_DATA_CMD(p_this);
        }

        SET_IC_INTR_MASK(p_this, RX_FULL_BIT | TX_EMPTY_BIT | TX_ABRT_BIT | STOP_DET_BIT);
        break;

    default:
        UNLOCK_rtk_I2C(&p_this->lock, flags);
        LOG_EVENT(EVENT_STOP_XFER);

        I2C_DEBUG("[ch_%d][phy_%d] Xfer fail - ILLEGAL MSG, %s Line%d\n", p_this->logical_id, p_this->id, __func__, __LINE__ );

        return -EILLEGALMSG;
    }

#ifdef MINIMUM_DELAY_EN
    if (p_this->guard_interval !=0)
    {
        UNLOCK_rtk_I2C(&p_this->lock, flags);
        udelay(p_this->guard_interval);
        LOCK_rtk_I2C(&p_this->lock, flags);
    }
#endif

    SET_IC_ENABLE(p_this, 1);
    UNLOCK_rtk_I2C(&p_this->lock, flags);
    if ((p_this->xfer.except_time < 1000)&&(p_this->guard_interval !=0))
    {
        udelay(p_this->xfer.except_time + 20);
    }

    if(p_this->polling_mode)
    {
        while (p_this->xfer.mode != I2C_IDEL)
        {
            rtk_i2c_isr(3, p_this);
            msleep(2);
        }
    }
    else
    {
        if (p_this->xfer.mode != I2C_IDEL)
            wait_event_timeout(p_this->wq, p_this->xfer.mode == I2C_IDEL, 1 * HZ);
    }

    LOCK_rtk_I2C(&p_this->lock, flags);

    if (p_this->xfer.mode != I2C_IDEL)
    {
        tempRawInt = GET_IC_RAW_INTR_STAT(p_this);
    }

    SET_IC_INTR_MASK(p_this, 0);
    SET_IC_ENABLE(p_this, 0);

    if (p_this->xfer.mode != I2C_IDEL)
    {
        p_this->xfer.ret = -ETIMEOUT;

        I2C_DEBUG("[ch_%d][phy_%d] Xfer fail - IRQ problem can't transfer -TIMEOUT, raw intr = %x , %s Line%d\n",
                    p_this->logical_id, p_this->id,tempRawInt, __func__, __LINE__ );

#ifdef CONFIG_RTK_KDRV_I2C_BUS_JAM_RECOVER

        UNLOCK_rtk_I2C(&p_this->lock, flags);
        
        if (rtk_i2c_bus_jam_detect_hw_guard(p_this))
        {
            I2C_DEBUG("%s I2C Bus Jammed, Do Recorver , Line%d\n", __func__, __LINE__);

            rtk_i2c_bus_jam_recover_proc_hw_guard(p_this);
            msleep(50);
        }

        I2C_DEBUG("Reset I2C State, Line%d", __LINE__);

        LOCK_rtk_I2C(&p_this->lock, flags);
#endif
    }
    else if (p_this->xfer.ret == -ECMDSPLIT)
    {
        I2C_DEBUG("[ch_%d][phy_%d] Xfer fail - MSG SPLIT, %s Line%d\n", p_this->logical_id, p_this->id, __func__, __LINE__ );

        switch (mode)
        {
        case I2C_MASTER_WRITE:
            I2C_DEBUG( "Write Cmd Split, tx : %d/%d , Line%d\n",
                         p_this->xfer.tx_len, p_this->xfer.tx_buff_len, __LINE__);
            break;

        case I2C_MASTER_READ:
            I2C_DEBUG ("Read Cmd Split, tx : %d/%d rx : %d/%d , Line%d\n",
                         p_this->xfer.tx_len, p_this->xfer.tx_buff_len,
                         p_this->xfer.rx_len, p_this->xfer.rx_buff_len,__LINE__);
            break;

        case I2C_MASTER_RANDOM_READ:
            I2C_DEBUG("RANDOM Read Cmd Split, tx : %d/%d rx : %d/%d , Line%d\n",
             p_this->xfer.tx_len,
             p_this->xfer.tx_buff_len +p_this->xfer.rx_buff_len,
             p_this->xfer.rx_len,
             p_this->xfer.rx_buff_len, __LINE__);
            break;
        }
    }

    p_this->time_stamp = (unsigned long)jiffies;

    ret = p_this->xfer.ret;

    UNLOCK_rtk_I2C(&p_this->lock, flags);

    if (ret == -ECMDSPLIT)
    {
        I2C_DEBUG("[ch_%d][phy_%d] Xfer fail - MSG SPLIT, %s Line%d\n", p_this->logical_id, p_this->id, __func__, __LINE__ );

		if (rtk_i2c_init_probe(p_this) < 0)
            I2C_DEBUG("[ch_%d][phy_%d] no longer exists , Line%d\n", p_this->logical_id, p_this->id, __LINE__);
    }

    LOG_EVENT(EVENT_STOP_XFER);

    return ret;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_get_tx_abort_reason
 *
 * Desc : get reason of tx abort, this register will be clear when new message is loaded
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : tx about source
 *------------------------------------------------------------------*/
unsigned int rtk_i2c_get_tx_abort_reason(rtk_i2c * p_this)
{
    return p_this->xfer.tx_abort_source;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_load_message
 *
 * Desc : load a i2c message (just add this message to the queue)
 *
 * Parm : p_this : handle of rtk i2c
 *
 *
 * Retn : 0 for success, others is failed
 *------------------------------------------------------------------*/
int rtk_i2c_load_message(rtk_i2c *p_this,
                         unsigned char mode,
                         unsigned char *tx_buf,
                         unsigned short tx_buf_len,
                         unsigned char *rx_buf,
                         unsigned short rx_buf_len, unsigned char xfer_flags)
{
    unsigned long flags;

    LOCK_rtk_I2C(&p_this->lock, flags);

    memset(&p_this->xfer, 0, sizeof(p_this->xfer));

    p_this->xfer.mode = mode;
    p_this->xfer.flags = xfer_flags;
    p_this->xfer.tx_buff = tx_buf;
    p_this->xfer.tx_buff_len = tx_buf_len;
    p_this->xfer.tx_len = 0;
    p_this->xfer.rx_buff = rx_buf;
    p_this->xfer.rx_buff_len = rx_buf_len;
    p_this->xfer.rx_len = 0;
    p_this->xfer.except_time = ((tx_buf_len + rx_buf_len + 2) * 9 * p_this->tick);

    if (rx_buf && rx_buf_len)
        memset(rx_buf, 0, rx_buf_len);

    p_this->xfer.gpio_xfer_state = G2C_ST_START;
    p_this->xfer.gpio_xfer_sub_state = 0;

    UNLOCK_rtk_I2C(&p_this->lock, flags);

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_read
 *
 * Desc : read data from sar
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0 for success, others is failed
 *------------------------------------------------------------------*/
int rtk_i2c_read(rtk_i2c *p_this,
                 unsigned char *tx_buf,
                 unsigned short tx_buf_len,
                 unsigned char *rx_buf, unsigned short rx_buf_len)
{
    int retry = 1;
    int ret = 0;

    while (retry > 0)
    {
        rtk_i2c_load_message(p_this,
                             (tx_buf_len) ? I2C_MASTER_RANDOM_READ :I2C_MASTER_READ,
                             tx_buf, tx_buf_len,
                             rx_buf, rx_buf_len, 0);

        ret = rtk_i2c_start_xfer(p_this);
        if ((ret != -ETIMEOUT)&&(ret != -ECMDSPLIT))
            break;

        I2C_WARNING("read timeout detected, do retry , %s Line%d\n", __func__, __LINE__);
        retry--;
    }

    return ret;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_write
 *
 * Desc : write data to sar
 *
 * Parm : p_this : handle of rtk i2c
 *        tx_buf : data to write
 *        tx_buf_len : number of bytes to write
 *        wait_stop  : wait for stop of not (extension)
 *
 * Retn : 0 for success, others is failed
 *------------------------------------------------------------------*/
int rtk_i2c_write(rtk_i2c *p_this,
                  unsigned char *tx_buf,
                  unsigned short tx_buf_len, unsigned char wait_stop)
{
    int retry = 1;
    int ret = 0;
    while (retry > 0)
    {
        rtk_i2c_load_message(p_this, I2C_MASTER_WRITE,
                             tx_buf, tx_buf_len, NULL, 0,
                             (wait_stop) ? 0 : I2C_NO_STOP);

#ifdef CONFIG_RTK_KDRV_I2C_NON_STOP_WRITE_XFER
        ret = (!wait_stop && rtk_i2c_has_mapped_gpio(p_this)) ?
                rtk_g2c_start_xfer(p_this): rtk_i2c_start_xfer(p_this);
#else
        ret = rtk_i2c_start_xfer(p_this);
#endif

        if ((ret != -ETIMEOUT)&&(ret != -ECMDSPLIT))
            break;

        I2C_WARNING("read timeout detected, do retry , %s Line%d\n", __func__, __LINE__);
        retry--;
    }
    return ret;

}

/*------------------------------------------------------------------
 * Func : get_rtk_i2c_phy_count
 *
 * Desc : get number of rtk i2c phy
 *
 * Parm : N/A
 *
 * Retn : number of rtk i2c
 *
 *------------------------------------------------------------------*/
unsigned char get_rtk_i2c_phy_count(void)
{
    return I2C_PHY_CNT;
}

#ifndef BUILD_QUICK_SHOW
/*------------------------------------------------------------------
 * Func : rtk_i2c_dump_port_dev_info
 *
 * Desc : dump i2c port device info, including name, addr(8-bit)
 *
 * Parm : port_id : p_this->logical_id
 *
 * Retn : ret
 *------------------------------------------------------------------*/
int rtk_i2c_dump_dev_info(unsigned char logical_id, char *buf)
{
    int i = 0, j = 0;
    int ret = 0;

    for(i = 0; i < rtk_i2c_dev_cnt; i++)
    {
        if(logical_id == rtk_i2c_dev_info[i].logical_id)
        {
            //add dev info to buf
            ret = snprintf(buf, PAGE_SIZE, "dev_name:%-10s dev_addr(8-bit): ", rtk_i2c_dev_info[i].name);
            buf += ret;
            if (rtk_i2c_dev_info[i].addr_cnt > 0)
            {
                for (j = 0; j < rtk_i2c_dev_info[i].addr_cnt; j++)
                {
                    ret = snprintf(buf, PAGE_SIZE, "0x%02x  ", rtk_i2c_dev_info[i].addr[j]);
                    buf += ret;
                }
            }
            else
            {
                ret = snprintf(buf, PAGE_SIZE, "Unknown");
                buf += ret;
            }
            ret = snprintf(buf, PAGE_SIZE, "\n");
            buf += ret;
        }
    }

    return ret;
}
/*------------------------------------------------------------------
 * Func : rtk_i2c_suspend
 *
 * Desc : suspend rtk i2c
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0 for success
 *------------------------------------------------------------------*/
int rtk_i2c_suspend(rtk_i2c * p_this)
{
    I2C_WARNING("[ch_%d][phy_%d] suspend\n", p_this->logical_id, p_this->id);
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_resume
 *
 * Desc : resume rtk i2c
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0 for success
 *------------------------------------------------------------------*/
int rtk_i2c_resume(rtk_i2c * p_this)
{
    I2C_WARNING("[ch_%d][phy_%d] resume\n", p_this->logical_id, p_this->id);

    rtk_i2c_phy_init(p_this,false);
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_reset_state
 *
 * Desc : reset internal state machine of i2c controller.
 *
 *        This is a hack that used to reset the internal state machine
 *        of rtk i2c. In mars, there is no way to reset the internal
 *        state of rtk I2C controller. However, we found out that we
 *        can use GPIO to generate a pseudo stop to reset it.
 *
 *        First, we need to set the i2c bus to CPIO mode and pull low
 *        SDA and pull high SCL, then changed the i2c bus to I2C mode.
 *        Because SDA has a pull high resistor, so the i2c controller
 *        will see SDA falling and rising when SCL is highw. It will be
 *        looked like start & stop and the state of i2c controller will
 *        be reset.
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_i2c_reset_state(rtk_i2c * p_this)
{
    RTK_GPIO_ID sda;
    RTK_GPIO_ID scl;

    int d = p_this->tick / 2;

    if (d == 0)
        d = 1;

    if (rtk_i2c_get_mapped_gpio(p_this, &sda, &scl) < 0)
        return;

    rtk_gpio_set_irq_enable(sda, 0);
    rtk_gpio_set_irq_enable(scl, 0);
    rtk_gpio_output(sda, 0);
    rtk_gpio_output(scl, 1);

    rtk_gpio_set_dir(sda, 1);
    rtk_gpio_set_dir(scl, 1);

    rtk_i2c_gpio_selection(p_this, GPIO_MODE);

    udelay(d);

    rtk_i2c_gpio_selection(p_this, I2C_MODE);
    rtk_gpio_set_dir(sda, 0);
    rtk_gpio_set_dir(scl, 0);
}

#ifdef CONFIG_RTK_KDRV_I2C_BUS_JAM_RECOVER

/*------------------------------------------------------------------
 * Func : rtk_i2c_bus_jam_recover
 *
 * Desc : recover i2c bus jam status
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_i2c_bus_jam_recover(rtk_i2c * p_this)
{
    RTK_GPIO_ID sda;
    RTK_GPIO_ID scl;
    int i;
    int d = p_this->tick / 2;

    if (rtk_i2c_get_mapped_gpio(p_this, &sda, &scl) < 0)
        return;

    if (d == 0)
        d = 1;

    rtk_gpio_set_irq_enable(sda, 0);
    rtk_gpio_set_irq_enable(scl, 0);

    rtk_gpio_output(sda, 0);
    rtk_gpio_output(scl, 0);

    rtk_gpio_set_dir(sda, 1);
    rtk_gpio_set_dir(scl, 1);

    rtk_i2c_gpio_selection(p_this, GPIO_MODE);

    udelay(10);
    rtk_gpio_output(scl, 1);
    udelay(10);
    rtk_gpio_output(sda, 1);
    udelay(10);

    rtk_gpio_set_dir(sda, 0);

    for (i = 0; i < 18; i++)
    {
        rtk_gpio_output(scl, 0);
        udelay(d);
        rtk_gpio_output(scl, 1);
        udelay(d);
    }

    rtk_gpio_set_dir(scl, 0);

    rtk_i2c_gpio_selection(p_this, I2C_MODE);
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_bus_jam_recover_hw_guard
 *
 * Desc : recover i2c bus jam status
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_i2c_bus_jam_recover_hw_guard(rtk_i2c * p_this)
{
    int i;
    int d = p_this->tick / 2;

    rtd_setbits(p_this->reg_map.IC_GPIO_MODE_CTRL, BIT(I2C_GPIO_CTRL_SEL_shift));
    udelay(10);

    rtd_setbits(p_this->reg_map.IC_GPIO_MODE_CTRL, BIT(I2C_GPIO_CTRL_SCL_shift));
    udelay(10);

    rtd_setbits(p_this->reg_map.IC_GPIO_MODE_CTRL, BIT(I2C_GPIO_CTRL_SDA_shift));
    udelay(10);

    for (i = 0; i < 9; i++)
    {
        rtd_clearbits(p_this->reg_map.IC_GPIO_MODE_CTRL,BIT(I2C_GPIO_CTRL_SCL_shift));
        udelay(d);

        rtd_setbits(p_this->reg_map.IC_GPIO_MODE_CTRL,BIT(I2C_GPIO_CTRL_SCL_shift));
        udelay(d);
    }

    rtd_clearbits(p_this->reg_map.IC_GPIO_MODE_CTRL, BIT(I2C_GPIO_CTRL_SEL_shift));
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_bus_jam_detect
 *
 * Desc : check if bus jam occurs
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0 : bus not jammed, 1 : bus jammed
 *------------------------------------------------------------------*/
int rtk_i2c_bus_jam_detect(rtk_i2c * p_this)
{
    RTK_GPIO_ID sda;
    RTK_GPIO_ID scl;
    int ret = 1;
    int i;

    if (rtk_i2c_get_mapped_gpio(p_this, &sda, &scl) < 0)
    {
        I2C_WARNING("rtk_i2c_bus_jam_detect failed, no gpio info , Line%d\n", __LINE__);
        return 0;
    }

    rtk_gpio_set_irq_enable(sda, 0);
    rtk_gpio_set_irq_enable(scl, 0);

    rtk_gpio_set_dir(sda, 0);
    rtk_gpio_set_dir(scl, 0);

    rtk_i2c_gpio_selection(p_this, GPIO_MODE);

    for (i = 0; i < 9; i++)
    {
        if (rtk_gpio_input(sda) && rtk_gpio_input(scl))
        {
            ret = 0;
            break;
        }
        msleep(1);
    }

    if (ret)
    {
        I2C_WARNING( "[ch_%d][phy_%d] Jamed, BUS Status: SDA=%d, SCL=%d\n",
                     p_this->logical_id, p_this->id, rtk_gpio_input(sda),rtk_gpio_input(scl));
    }

    rtk_i2c_gpio_selection(p_this, I2C_MODE);

    return ret;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_bus_jam_detect_hw_guard
 *
 * Desc : check if bus jam occurs , s2b has hardware guard register to detect scl sda
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0 : bus not jammed, 1 : bus jammed
 *------------------------------------------------------------------*/
int rtk_i2c_bus_jam_detect_hw_guard(rtk_i2c * p_this)
{
    int ret = 1;
    int i = 0;
    unsigned int reg_temp = 0;

    for (i = 0; i < 30; i++)
    {
        reg_temp = rd_reg(p_this->reg_map.IC_GUARD_STATUS);
        if ((reg_temp & BIT(I2C_GPIO_STATUS_GPIO_SCL_shift))&&
            (reg_temp & BIT(I2C_GPIO_STATUS_GPIO_SDA_shift)))
        {
            ret = 0;
            break;
        }
        msleep(1);
    }

    if (ret)
    {
        I2C_WARNING("[ch_%d][phy_%d] Jamed, BUS Status: %08x , Line%d\n",
                    p_this->logical_id, p_this->id, rd_reg(p_this->reg_map.IC_GUARD_STATUS),__LINE__);
    }

    return ret;
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_bus_jam_recover
 *
 * Desc : recover i2c bus jam status
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_i2c_bus_jam_recover_proc(rtk_i2c * p_this)
{
    int i = 0;

    if (!rtk_i2c_has_mapped_gpio(p_this))
        return;

    do
    {
        I2C_INFO("Do [ch_%d][phy_%d] Bus Recover %d\n", p_this->logical_id, p_this->id, i);

        rtk_i2c_bus_jam_recover(p_this);

        msleep(200);

        if (rtk_i2c_bus_jam_detect(p_this) == 0)
        {
            I2C_INFO( "[ch_%d][phy_%d] Bus Recover successed\n", p_this->logical_id, p_this->id);
            return;
        }

    }while (i++ < 3);

    I2C_WARNING("[ch_%d][phy_%d] Bus Recover failed , Line%d \n", p_this->logical_id, p_this->id, __LINE__);

}

/*------------------------------------------------------------------
 * Func : rtk_i2c_bus_jam_recover_hw_guard
 *
 * Desc : recover i2c bus jam status
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_i2c_bus_jam_recover_proc_hw_guard(rtk_i2c * p_this)
{
    int i = 0;

    do
    {
        I2C_INFO("[ch_%d][phy_%d] Hardware Guard do Bus Recover (1) %d.\n", p_this->logical_id, p_this->id, i);

        rtk_i2c_bus_jam_recover_hw_guard(p_this);

        msleep(200);

        if (rtk_i2c_bus_jam_detect_hw_guard(p_this) == 0)
        {
            I2C_INFO
            ("[ch_%d][phy_%d] Hardware Guard Bus Recover Successed (2) \n", p_this->logical_id, p_this->id);
            return;
        }

    }while (i++ < 0);

    I2C_WARNING("[ch_%d][phy_%d] Hardware Guard Bus Recover Failed (3),  Line%d\n", p_this->logical_id, p_this->id, __LINE__);
}
#endif

/*------------------------------------------------------------------
 * Func : create_rtk_i2c_handle
 *
 * Desc : create handle of rtk i2c
 *
 * Parm : N/A
 *
 * Retn : handle of rtk i2c
 *
 *------------------------------------------------------------------*/
rtk_i2c *create_rtk_i2c_handle(unsigned char logical_id,
                               unsigned char phy_id,
                               unsigned short sar,
                               ADDR_MODE sar_mode,
                               unsigned int spd,
                               unsigned int irq)
{
    rtk_i2c *hHandle;

    if (phy_id >= I2C_PHY_CNT)
    {
        I2C_WARNING("[ch_%d][phy_%d] %s fail!\n", logical_id, phy_id, __func__);
        return NULL;
    }

    if (((rtk_i2c_flags >> phy_id) & 0x01))
    {
        atomic_inc(&rtk_i2c_phy_handle[phy_id]->ref_cnt);
        return rtk_i2c_phy_handle[phy_id];
    }

    hHandle = kmalloc(sizeof(rtk_i2c), GFP_KERNEL);

    if (hHandle != NULL)
    {
        memset(hHandle, 0, sizeof(rtk_i2c));
        hHandle->flags                 = 0;
        hHandle->logical_id            = logical_id;
        hHandle->id                    = phy_id;
        hHandle->irq                   = irq;
        hHandle->sar                   = sar;
        hHandle->sar_mode              = sar_mode;
        hHandle->spd                   = spd;
        hHandle->guard_interval        = 1000;
        hHandle->i2c_loglevel          = 1;
        hHandle->init                  = rtk_i2c_init;
        hHandle->uninit                = rtk_i2c_uninit;
        hHandle->set_spd               = rtk_i2c_set_spd;
        hHandle->set_guard_interval    = rtk_i2c_set_guard_interval;
        hHandle->set_hw_guard_interval = rtk_i2c_set_hw_guard_interval;
        hHandle->set_tar               = rtk_i2c_set_tar;
        hHandle->set_port              = rtk_i2c_set_port;
        hHandle->unset_port            = rtk_i2c_unset_port;
        hHandle->read                  = rtk_i2c_read;
        hHandle->write                 = rtk_i2c_write;
        hHandle->gpio_read             = rtk_g2c_read;
        hHandle->gpio_write            = rtk_g2c_write;
        hHandle->dump                  = rtk_i2c_dump;
        hHandle->suspend               = rtk_i2c_suspend;
        hHandle->resume                = rtk_i2c_resume;
        hHandle->get_tx_abort_reason   = rtk_i2c_get_tx_abort_reason;

#ifdef CONFIG_RTK_KDRV_I2C_BUS_JAM_RECOVER
        hHandle->get_bus_status        = rtk_i2c_bus_jam_detect_hw_guard;
        hHandle->do_bus_jame_recover   = rtk_i2c_bus_jam_recover_proc_hw_guard;
#endif
        hHandle->set_sar               = rtk_i2c_set_sar;
        hHandle->slave_mode_enable     = rtk_i2c_slave_mode_enable;
        hHandle->register_slave_ops    = rtk_i2c_register_slave_ops;
        hHandle->get_hold_time         = rtk_i2c_get_hold_time;
        hHandle->set_hold_time         = rtk_i2c_set_hold_time;
        hHandle->get_setup_time        = rtk_i2c_get_setup_time;
        hHandle->set_setup_time        = rtk_i2c_set_setup_time;

        atomic_set(&hHandle->ref_cnt, 1);
        memset(&hHandle->xfer, 0, sizeof(rtk_i2c_xfer));
        rtk_i2c_phy_handle[phy_id] = hHandle;
        rtk_i2c_flags |= (0x01 << phy_id);

        I2C_INFO("[ch_%d][phy_%d] create_rtk_i2c_handle: rtk_i2c_flags = %02x irq=%d\n",
                    logical_id, phy_id, (uint32_t) rtk_i2c_flags, irq);
    }

    return hHandle;
}
EXPORT_SYMBOL(create_rtk_i2c_handle);

/*------------------------------------------------------------------
 * Func : destroy_rtk_i2c_handle
 *
 * Desc : destroy handle of rtk i2c
 *
 * Parm : N/A
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void destroy_rtk_i2c_handle(rtk_i2c *hHandle)
{
    if (hHandle == NULL)
        return;

    if (atomic_dec_return(&hHandle->ref_cnt) > 0)
    {
        I2C_WARNING("[ch_%d] destroy successed, reference cnt=%d\n",
                    hHandle->id, atomic_read(&hHandle->ref_cnt));
        return;
    }

    I2C_INFO( "[ch_%d]destroy rtk i2c handle\n", hHandle->id);

    hHandle->uninit(hHandle);
    rtk_i2c_flags &= ~(0x01 << hHandle->id);
    rtk_i2c_phy_handle[hHandle->id] = NULL;

    kfree(hHandle);
}
EXPORT_SYMBOL(destroy_rtk_i2c_handle);

EXPORT_SYMBOL(get_rtk_i2c_phy_count);
MODULE_AUTHOR("baker.cheng <baker.cheng@realtek.com>");
MODULE_DESCRIPTION("Realtek I2C Driver");
MODULE_ALIAS("rtk-i2c");
MODULE_LICENSE("GPL");
#else
/*------------------------------------------------------------------
 * Func : create_rtk_i2c_handle
 *
 * Desc : create handle of rtk i2c
 *
 * Parm : N/A
 *
 * Retn : handle of rtk i2c
 *
 *------------------------------------------------------------------*/
rtk_i2c *create_rtk_i2c_handle(unsigned char logical_id,
                               unsigned char id,
                               unsigned short sar,
                               ADDR_MODE sar_mode,
                               unsigned int spd, unsigned int irq)
{
    rtk_i2c *hHandle;

    if (id >= I2C_PHY_CNT)
    {
        I2C_WARNING("[ch_%d][phy_%d] %s fail!\n", logical_id, id, __func__);
        return NULL;
    }

    hHandle = kmalloc(sizeof(rtk_i2c), GFP_KERNEL);
    if (hHandle != NULL)
    {
        memset(hHandle, 0, sizeof(rtk_i2c));
        hHandle->flags                 = 0;
        hHandle->logical_id            = logical_id;
        hHandle->id                    = id;
        hHandle->irq                   = irq;
        hHandle->sar                   = sar;
        hHandle->sar_mode              = sar_mode;
        hHandle->spd                   = spd;
        hHandle->guard_interval        = 1000;
        hHandle->i2c_loglevel          = 1;
        hHandle->init                  = rtk_i2c_init;
        hHandle->uninit                = rtk_i2c_uninit;
        hHandle->set_spd               = rtk_i2c_set_spd;
        hHandle->set_guard_interval    = rtk_i2c_set_guard_interval;
        hHandle->set_hw_guard_interval = rtk_i2c_set_hw_guard_interval;
        hHandle->set_tar               = rtk_i2c_set_tar;
        hHandle->set_port              = rtk_i2c_set_port;
        hHandle->unset_port            = rtk_i2c_unset_port;
        hHandle->read                  = rtk_i2c_read;
        hHandle->write                 = rtk_i2c_write;
        hHandle->gpio_read             = NULL;
        hHandle->gpio_write            = NULL;
        hHandle->dump                  = rtk_i2c_dump;
        hHandle->suspend               = NULL;
        hHandle->resume                = NULL;
        hHandle->get_tx_abort_reason   = rtk_i2c_get_tx_abort_reason;

#ifdef CONFIG_RTK_KDRV_I2C_BUS_JAM_RECOVER
        hHandle->get_bus_status        = rtk_i2c_bus_jam_detect_hw_guard;
        hHandle->do_bus_jame_recover   = rtk_i2c_bus_jam_recover_proc_hw_guard;
#endif
        hHandle->set_sar               = rtk_i2c_set_sar;
        hHandle->slave_mode_enable     = rtk_i2c_slave_mode_enable;
        hHandle->register_slave_ops    = rtk_i2c_register_slave_ops;
        hHandle->get_hold_time         = rtk_i2c_get_hold_time;
        hHandle->set_hold_time         = rtk_i2c_set_hold_time;
        hHandle->get_setup_time        = rtk_i2c_get_setup_time;
        hHandle->set_setup_time        = rtk_i2c_set_setup_time;
        hHandle->polling_mode          = 1;

        memset(&hHandle->xfer, 0, sizeof(rtk_i2c_xfer));
        rtk_i2c_phy_handle[id] = hHandle;
        rtk_i2c_flags |= (0x01 << id);

        I2C_INFO( "[ch_%d][phy_%d] create_rtk_i2c_handle: rtk_i2c_flags = %02x irq=%d\n",
                    logical_id, id, (uint32_t) rtk_i2c_flags, irq);
    }

    return hHandle;
}


/*------------------------------------------------------------------
 * Func : destroy_rtk_i2c_handle
 *
 * Desc : destroy handle of rtk i2c
 *
 * Parm : N/A
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void destroy_rtk_i2c_handle(rtk_i2c * hHandle)
{
    if (hHandle == NULL)
        return;

    I2C_INFO( "[ch_%d]destroy rtk i2c handle\n", hHandle->id);
    hHandle->uninit(hHandle);
    rtk_i2c_flags &= ~(0x01 << hHandle->id);
    rtk_i2c_phy_handle[hHandle->id] = NULL;

    kfree(hHandle);
}

#endif

