/* -------------------------------------------------------------------------
   cec_rtd.c  cec driver for Realtek rtd
   -------------------------------------------------------------------------
    Copyright (C) 2012 Kevin Wang <kevin_wang@realtek.com.tw>
----------------------------------------------------------------------------
Update List :
----------------------------------------------------------------------------
    1.0     |   20120608    |   Kevin Wang  | 1) create phase
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
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_crt.h>
#include <rbus/stb_reg.h>
#include <mach/platform.h>
#include "rtk_cec_priv.h"
#include "rtk_cec_msg.h"
#include "rtk_cec_reg.h"


static rtk_cec m_cec;

#define TX_TIMEOUT      (HZ<<2)
#define SA_INTERRUPT    0
#define SA_SHIRQ        IRQF_SHARED

/*#define CONFIG_CEC_DBG*/
/*#define CEC_TX_DBG*/
/*#define CEC_RX_DBG*/

/*//////////////////////////////////////////////////////////////////////
// CEC Wakeup Function
//////////////////////////////////////////////////////////////////////*/
#define CEC_OPCODE_COMPARE_ENABLE
#ifdef CEC_OPCODE_COMPARE_ENABLE
#define IS_OPCODE_COMP_EN() (read_reg32(GDI_CEC_OPCODE_ENABLE)&0xf)
#define PWR_STS_COMP_EN()   (read_reg32(GDI_CEC_OPCODE_ENABLE)&0x00000001)
#define PHY_ADDR_COMP_EN()  (read_reg32(GDI_CEC_OPCODE_ENABLE)&0x00000002)
#define CEC_VER_COMP_EN()   (read_reg32(GDI_CEC_OPCODE_ENABLE)&0x00000004)
#define VEND_ID_COMP_EN()   (read_reg32(GDI_CEC_OPCODE_ENABLE)&0x00000008)
#endif

/*static unsigned long  __sleepdata rtd_cec_standby_config          = 0;*/
/*static unsigned char  __sleepdata rtd_cec_standby_logical_addr  = 0xF;*/
/*static unsigned short __sleepdata rtd_cec_standby_physical_addr   = 0xFFFF;*/
/*static unsigned char  __sleepdata rtd_cec_standby_cec_version     = CEC_VERSION_1_4;*/
/*static unsigned long  __sleepdata rtd_cec_standby_vendor_id       = 0x000000;*/

static unsigned long rtk_cec_standby_config = 0x0;
static unsigned char rtk_cec_standby_logical_addr = 0xF;
static unsigned short rtk_cec_standby_physical_addr = 0xFFFF;
static unsigned char rtk_cec_standby_cec_version = CEC_VERSION_1_4;
static unsigned long rtk_cec_standby_vendor_id = 0x000000;
static unsigned long rtk_cec_retry_num = 5;



/*------------------------------------------------------------------
 * Func : _get_pre_div
 *
 * Desc : _get_pre_div setting
 *
 * Parm : N/A
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
unsigned int _get_pre_div(void)
{
    unsigned int rbus, pll;

    pll = get_bus_pll();

    /* Internal logic use PLLBUS/2 */
    rbus = pll/2;
#ifdef CONFIG_ARCH_RTK6702
    if(rbus>=0x400)     //mark2 cec pll change to 500M; pre reg add two bit
        rbus=0x3FF;
#else
    if(rbus >= 0x100)
        rbus = 0xff;
#endif
    cec_info("%s , pll = 0x%X , rbus = %d \n",__func__, pll , rbus );

    return rbus;
}

static unsigned char rx_fifo_ofst = 0;
#define _reset_rx_fifo_ofst()       do { rx_fifo_ofst = 0; }while(0)

/*------------------------------------------------------------------
 * Func : _read_rx_fifo
 *
 * Desc : read rx fifo
 *
 * Param: rx_buff : buffer address
 *        rx_buff_size : size of rx buffer
 *
 * Retn : >=0 : number of bytes read
 *        <0 : failed
 *------------------------------------------------------------------*/
static int _read_rx_fifo(char* rx_buff, int rx_buff_size, int eom)
{
    unsigned int rx_fifo_data[4] = {0} ;
    unsigned int data_len = rtd_inl(KDRV_CEC_RXCR0) & 0x1F;
    int rx_entry_cnt = 0;
    int i = 0 ;

    cec_rx_dbg("rx_fifo_count = %u, RXCR0=0x%X, eom=%d\n", data_len, rtd_inl(KDRV_CEC_RXCR0), eom);

    if (data_len == 0)
        return 0;

    if (data_len > rx_buff_size)
        data_len = rx_buff_size;

    if (data_len > 16)
        data_len = 16;

    if (!eom)   // not EOM, read data should be 4 byte alignment
        data_len &= ~0x3;

    rx_entry_cnt = (data_len + 3) >> 2;

    for (i = 0; i < rx_entry_cnt; i++)
    {
        rx_fifo_data[i] = __cpu_to_be32(rtd_inl(KDRV_CEC_RXDR1 + (rx_fifo_ofst << 2)));

        rx_fifo_ofst++;
        rx_fifo_ofst &= 0x3;
    }

    memcpy(rx_buff, (unsigned char*) rx_fifo_data, data_len);  // KWarning: checked ok by kevin_wang@realtek.com

    rtd_outl(KDRV_CEC_TXDR0, RX_SUB_CNT | FIFO_CNT(data_len)); /* rx fifo count -= rx_count */

    return data_len;
}


static unsigned char tx_fifo_ofst = 0;
#define _reset_tx_fifo_ofst()   do { tx_fifo_ofst = 0; }while(0)

/*------------------------------------------------------------------
 * Func : _write_tx_fifo
 *
 * Desc : write data to tx fifo
 *
 * Param: tx_buff : buffer address
 *        tx_buff_len : size of tx buffer
 *
 * Retn : >=0 : number of bytes write
 *        <0 : failed
 *------------------------------------------------------------------*/
static int _write_tx_fifo(char* tx_buff, int tx_buff_len)
{
    unsigned int tx_fifo_data[4];
    unsigned int tx_len = tx_buff_len;
    unsigned int tx_entry;
    int i;

    if (tx_len > 16)
        tx_len = 16;

    memcpy(tx_fifo_data, tx_buff, tx_len);

    tx_entry = (tx_len + 3) >> 2;  // number of TX fifo entry

    for (i = 0; i < tx_entry; i++) {
        tx_fifo_ofst &= 0x3;        // start address of tx fifo
        cec_tx_dbg("txdr1[%d] =0x%x \n", tx_fifo_ofst, __cpu_to_be32(tx_fifo_data[i]));
        rtd_outl(KDRV_CEC_TXDR1 + (tx_fifo_ofst << 2), __cpu_to_be32(tx_fifo_data[i]));
        tx_fifo_ofst++;
    }

    rtd_outl(KDRV_CEC_TXDR0, TX_ADD_CNT | FIFO_CNT(tx_len)); /* add data to fifo */

    return tx_len;
}

/*------------------------------------------------------------------
 * Func : _cmb_tx_complete
 *
 * Desc : complete a tx cmb
 *
 * Parm : p_this : handle of rtd cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void _cmb_tx_complete(cm_buff *cmb)
{
    if (cmb->flags & NONBLOCK) {
        kfree_cmb(cmb);
    } else {
        if (cmb->status == WAIT_XMIT)
            cmb->status = XMIT_ABORT;

        wake_up(&cmb->wq);
    }
}

/*------------------------------------------------------------------
 * Func : rtd_cec_rx_reset
 *
 * Desc : reset rx
 *
 * Parm : p_this : handle of rtd cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static
void rtd_cec_rx_reset(rtk_cec *p_this)
{
    write_reg32(KDRV_CEC_RXCR0, RX_RST);
    wmb();
    write_reg32(KDRV_CEC_RXCR0, 0);
    wmb();
    p_this->rcv.state = IDEL;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_tx_work
 *
 * Desc : rtd cec tx function
 *
 * Parm : p_this : handle of rtd cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtd_cec_tx_work(struct work_struct *work)
{
    rtk_cec *p_this =
        (rtk_cec *) container_of(work, rtk_cec, xmit.work.work);
    rtk_cec_xmit *p_xmit = &p_this->xmit;
    unsigned char dest;
    unsigned long flags;
    int ret;

    spin_lock_irqsave(&p_this->lock, flags);

    if (p_xmit->state == XMIT) {
        if (read_reg32(KDRV_CEC_TXCR0) & TX_EN) {
            /* xmitting */
            if (read_reg32(KDRV_CEC_TXCR0) & TX_INT) {
                if (p_xmit->cmb->len) {
                    ret = _write_tx_fifo(p_xmit->cmb->data, (p_xmit->cmb->len > 8) ? 8 : p_xmit->cmb->len);
                    cmb_pull(p_xmit->cmb, ret);

                    if (p_xmit->cmb->len == 0)
                        write_reg32(KDRV_CEC_TXCR0, read_reg32(KDRV_CEC_TXCR0) & ~TX_CONTINUOUS); /* clear continue bits.... */
                }

                write_reg32(KDRV_CEC_TXCR0, read_reg32(KDRV_CEC_TXCR0));  /* clear interrupt */
            }

            if (time_after(jiffies, p_xmit->timeout)) {
                write_reg32(KDRV_CEC_TXCR0, TX_INT);

                p_xmit->cmb->status = XMIT_TIMEOUT;
                _cmb_tx_complete(p_xmit->cmb);

                p_xmit->cmb = NULL;
                p_xmit->state = IDEL;

                cec_warn("cec tx timeout\n");

                cancel_delayed_work(&p_xmit->work);
            }
        } else {
            cancel_delayed_work(&p_xmit->work);

            if ((read_reg32(KDRV_CEC_TXCR0) & TX_EOM) == 0) {
                p_xmit->cmb->status = XMIT_FAIL;
            } else {
                p_xmit->cmb->status = XMIT_OK;
            }

            write_reg32(KDRV_CEC_TXCR0, TX_INT);
            _cmb_tx_complete(p_xmit->cmb);
            p_xmit->cmb = NULL;
            p_xmit->state = IDEL;
        }
    }

    if (p_xmit->state == IDEL) {
        if (p_xmit->enable) {
            p_xmit->cmb = cmb_dequeue(&p_this->tx_queue);

            if (p_xmit->cmb) {
                dest = (p_xmit->cmb->data[0] & 0xf);
                cmb_pull(p_xmit->cmb, 1);
                p_xmit->timeout = jiffies + TX_TIMEOUT;

                /* reset tx fifo */
                write_reg32(KDRV_CEC_TXCR0, TX_RST);
                wmb();
                write_reg32(KDRV_CEC_TXCR0, 0);
                wmb();

                _reset_tx_fifo_ofst();
                ret = _write_tx_fifo(p_xmit->cmb->data, (p_xmit->cmb->len > 16) ? 16 : p_xmit->cmb->len);
                cmb_pull(p_xmit->cmb, ret);


                if (p_xmit->cmb->len == 0)
                    write_reg32(KDRV_CEC_TXCR0,DEST_ADDR(dest) | TX_EN |TX_INT_EN);
                else
                    write_reg32(KDRV_CEC_TXCR0,DEST_ADDR(dest) | TX_EN |TX_INT_EN | TX_CONTINUOUS);

                p_xmit->state = XMIT;

                /* queue delayed work for timeout detection */
                schedule_delayed_work(&p_xmit->work, TX_TIMEOUT + 1);
            }
        }
    }

    spin_unlock_irqrestore(&p_this->lock, flags);
}

/*------------------------------------------------------------------
 * Func : rtd_cec_tx_start
 *
 * Desc : start rx
 *
 * Parm : p_this : handle of rtd cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static
void rtd_cec_tx_start(rtk_cec *p_this)
{
    unsigned long flags;
    spin_lock_irqsave(&p_this->lock, flags);

    write_reg32(KDRV_CEC_RXCR0, (read_reg32(KDRV_CEC_RXCR0) | RX_INT_EN) |
                RX_EN | RX_CONTINUOUS); /* enable RX */
    if (!p_this->xmit.enable) {
        cec_info("cec tx start\n");
        p_this->xmit.enable = 1;
        p_this->xmit.state = IDEL;
        p_this->xmit.cmb = NULL;

        INIT_DELAYED_WORK(&p_this->xmit.work, rtd_cec_tx_work);
    }

    spin_unlock_irqrestore(&p_this->lock, flags);
}

/*------------------------------------------------------------------
 * Func : rtd_cec_tx_stop
 *
 * Desc : stop tx
 *
 * Parm : p_this : handle of rtd cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static
void rtd_cec_tx_stop(rtk_cec *p_this)
{
    cm_buff *cmb;
    unsigned long flags;
    spin_lock_irqsave(&p_this->lock, flags);

    if (p_this->xmit.enable) {
        cec_info("cec tx stop\n");

        rtd_cec_rx_reset(p_this);
        cancel_delayed_work(&p_this->xmit.work);
        /* reset tx fifo */
        write_reg32(KDRV_CEC_TXCR0, TX_RST);
        wmb();
        write_reg32(KDRV_CEC_TXCR0, 0);
        wmb();

        if (p_this->xmit.cmb)
            _cmb_tx_complete(p_this->xmit.cmb);

        while ((cmb = cmb_dequeue(&p_this->tx_queue)))
            _cmb_tx_complete(cmb);

        p_this->xmit.enable = 0;
        p_this->xmit.state = IDEL;
        p_this->xmit.cmb = NULL;
    }
    write_reg32(KDRV_CEC_RXCR0, RX_EN | RX_CONTINUOUS);  /* enable RX */
    spin_unlock_irqrestore(&p_this->lock, flags);
}

extern int rtd_cec_xmit_message(rtk_cec *p_this, cm_buff *cmb,
                                    unsigned long flags);

/*------------------------------------------------------------------
 * Func : rtd_cec_standby_message_handler
 *
 * Desc : rtd cec message handler when cec works under standby mode
 *
 * Parm : p_this : handle of rtd cec
 *        init   : message initator
 *        dest   : message destination
 *        opcode : opcode of message
 *        param  : param of messsage
 *        len    : length of message parameter
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtd_cec_standby_message_handler(rtk_cec *p_this,
        unsigned char init,
        unsigned char dest,
        unsigned char opcode,
        unsigned char *param,
        unsigned char len)
{
    cm_buff *cmb = NULL;

    unsigned char dev_type_map[15] = {
        CEC_DEVICE_TV,
        CEC_DEVICE_RECORDING_DEVICE,
        CEC_DEVICE_RECORDING_DEVICE,
        CEC_DEVICE_TUNER,
        CEC_DEVICE_PLAYBACK_DEVICE,
        CEC_DEVICE_AUDIO_SYSTEM,
        CEC_DEVICE_TUNER,
        CEC_DEVICE_TUNER,
        CEC_DEVICE_PLAYBACK_DEVICE,
        CEC_DEVICE_RECORDING_DEVICE,
        CEC_DEVICE_TUNER,
        CEC_DEVICE_PLAYBACK_DEVICE,
        CEC_DEVICE_RESERVED,
        CEC_DEVICE_RESERVED,
        CEC_DEVICE_TV
    };

    /*cec_info("rtd_cec_standby_message_handler : init = %x, dest=%x, opcode=%x, len=%d\n", init, dest, opcode, len); */

    switch (opcode) {
        case CEC_MSG_GIVE_DEVICE_POWER_STATUS:

            if ( (rtk_cec_standby_config & STANBY_RESPONSE_GIVE_POWER_STATUS ) &&
                 ( len == 0 ) && ( init != 0xF ) ) {

#ifdef CEC_OPCODE_COMPARE_ENABLE
                if (PWR_STS_COMP_EN())
                    break;
#endif
                cmb = alloc_cmb(3);
                if (cmb) {
                    cmb->data[0] = (dest << 4) | init;
                    cmb->data[1] = CEC_MSG_REPORT_POWER_STATUS;
                    cmb->data[2] = CEC_POWER_STATUS_STANDBY;
                    cmb_put(cmb, 3);
                }
            }
            break;

        case CEC_MSG_GIVE_PHYSICAL_ADDRESS:

            if ( (rtk_cec_standby_config & STANBY_WAKEUP_BY_SET_STREAM_PATH ) &&
                 ( len == 0 ) && ( dest != 0xF) ) {

#ifdef CEC_OPCODE_COMPARE_ENABLE
                if (PHY_ADDR_COMP_EN())
                    break;
#endif
                cmb = alloc_cmb(5);
                if (cmb)
                {
                    cmb->data[0] = (dest << 4) | 0xF;
                    cmb->data[1] = CEC_MSG_REPORT_PHYSICAL_ADDRESS;
                    cmb->data[2] =(rtk_cec_standby_physical_addr >> 8) & 0xFF;
                    cmb->data[3] =(rtk_cec_standby_physical_addr) & 0xFF;
                    cmb->data[4] = dev_type_map[dest];

                    cmb_put(cmb, 5);
                }
            }
            break;

        case CEC_MSG_STANDBY:
            break;

        default:
            /* send feature abort */
            if (init != 0xF && dest != 0xF) {
                cmb = alloc_cmb(4);
                if (cmb) {
                    cmb->data[0] = (dest << 4) | init;
                    cmb->data[1] = CEC_MSG_FEATURE_ABORT;
                    cmb->data[2] = opcode;  /* FEATURE ABORT */
                    cmb->data[3] = CEC_ABORT_NOT_IN_CORECT_MODE;
                    cmb_put(cmb, 4);
                }
            }
    }

    if (cmb)
        rtd_cec_xmit_message(p_this, cmb, NONBLOCK);
}

/*------------------------------------------------------------------
 * Func : rtd_cec_rx
 *
 * Desc : rtd cec rx function
 *
 * Parm : work_struct : handle of work structure
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtd_cec_rx_work(struct work_struct *work)
{
    int ret;

    bool bDrop_Data = false;
    unsigned long flags;
    unsigned long rx_event = read_reg32(KDRV_CEC_RXCR0);

    rtk_cec *p_this = (rtk_cec *) container_of(work, rtk_cec, rcv.work.work);
    rtk_cec_rcv *p_rcv = &p_this->rcv;

    spin_lock_irqsave(&p_this->lock, flags);

    if (!p_rcv->enable) {
        if (p_rcv->state == RCV) {
            cec_rx_dbg("force stop\n");
            write_reg32(KDRV_CEC_RXCR0, 0);
            kfree_cmb(p_rcv->cmb);
            p_rcv->cmb = NULL;
            p_rcv->state = IDEL;
        }
        goto end_proc;
    }

    if (p_rcv->state == RCV) {
#ifdef CEC_OPCODE_COMPARE_ENABLE
        if ((rx_event & RX_EN) && (!IS_OPCODE_COMP_EN()))
#else
        if (rx_event & RX_EN)
#endif
        {
            if (rx_event & RX_INT) {
                if ((rx_event & RX_EOM))
                    ret = _read_rx_fifo(p_rcv->cmb->tail, cmb_tailroom(p_rcv->cmb), 1);
                else
                    ret = _read_rx_fifo(p_rcv->cmb->tail, cmb_tailroom(p_rcv->cmb), 0);

                cmb_put(p_rcv->cmb, ret);

                if (ret < 0)
                {
                    cec_warn("read rx fifo failed, return to rx\n");
                    write_reg32(KDRV_CEC_RXCR0, 0);
                    _reset_rx_fifo_ofst();
                    p_rcv->state = IDEL;
                }

                write_reg32(KDRV_CEC_RXCR0, read_reg32(KDRV_CEC_RXCR0)); // clear RX_INT
            }
        } else {
            cec_rx_dbg("rx_stop (0x%08x)\n", read_reg32(KDRV_CEC_RXCR0));

            ret = _read_rx_fifo(p_rcv->cmb->tail, cmb_tailroom(p_rcv->cmb), 1);
            cmb_put(p_rcv->cmb, ret);

            if ((rx_event & RX_EOM) && p_rcv->cmb->len) {

                if (rx_event & BROADCAST_ADDR) {
#if PATCH_RX_DROP_SAME_LA_DATA
                    if(!GET_CEC_MSG_STATUS_02(read_reg32(GDI_POWER_SAVING_MODE)))
                        bDrop_Data = true;
#endif

                    if(bDrop_Data == false) {
                        *cmb_push(p_rcv->cmb, 1) = (
                                                       ((rx_event & INIT_ADDR_MASK) >> INIT_ADDR_SHIFT)
                                                       << 4) | 0xF;
                    }
                } else {
                    *cmb_push(p_rcv->cmb, 1) =
                        (((rx_event & INIT_ADDR_MASK) >> INIT_ADDR_SHIFT) << 4) |
                        ((read_reg32(KDRV_CEC_CR0) &LOGICAL_ADDR_MASK) >> LOGICAL_ADDR_SHIFT);
                }

                if (p_this->status.standby_mode) {
                    /* process messag */
                    rtd_cec_standby_message_handler (
                        p_this,
                        p_rcv->cmb->data[0] >> 4,
                        p_rcv->cmb->data[0] & 0xF,
                        p_rcv->cmb->data[1],
                        &p_rcv->cmb->data[2],
                        p_rcv->cmb->len - 2);
                } else if(bDrop_Data == false) {
                    cmb_queue_tail(&p_this->rx_queue,
                                   p_rcv->cmb);
                    p_rcv->cmb = NULL;

                    wake_up_interruptible(&p_this->rcv.wq);
                }

            }

            p_rcv->state = IDEL;
        }
    }

    if (p_rcv->state == IDEL) {
        if (!p_rcv->enable)
            goto end_proc;

        if (!p_rcv->cmb) {
            if (cmb_queue_len(&p_this->rx_free_queue))
                p_rcv->cmb =
                    cmb_dequeue(&p_this->rx_free_queue);
            else {
                cec_warn("[CEC] WARNING, rx over run\n");
                p_rcv->cmb = cmb_dequeue(&p_this->rx_queue);    /* reclaim form rx fifo */
            }

            if (p_rcv->cmb == NULL) {
                cec_warn("[CEC] FATAL, something wrong, no rx buffer left\n");
                goto end_proc;
            }
        }

        cmb_purge(p_rcv->cmb);
        cmb_reserve(p_rcv->cmb, 1);

        write_reg32(KDRV_CEC_RXCR0, RX_RST);
        wmb();
        write_reg32(KDRV_CEC_RXCR0, 0);
        wmb();

        _reset_rx_fifo_ofst();

        write_reg32(KDRV_CEC_RXCR0, RX_EN | RX_INT_EN | RX_CONTINUOUS);
        cec_rx_dbg("rx_restart (0x%08x)\n", read_reg32(KDRV_CEC_RXCR0));
        p_rcv->state = RCV;

    }

end_proc:

    spin_unlock_irqrestore(&p_this->lock, flags);
}


/*------------------------------------------------------------------
 * Func : rtd_cec_rx_start
 *
 * Desc : start rx
 *
 * Parm : p_this : handle of rtd cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static
void rtd_cec_rx_start(rtk_cec *p_this)
{
    unsigned long flags;
    spin_lock_irqsave(&p_this->lock, flags);

    if (!p_this->rcv.enable) {
        cec_info("rx start\n");

        p_this->rcv.enable = 1;
        p_this->rcv.state = IDEL;
        p_this->rcv.cmb = NULL;
        _reset_rx_fifo_ofst();

        INIT_DELAYED_WORK(&p_this->rcv.work, rtd_cec_rx_work);

        schedule_delayed_work(&p_this->rcv.work, 0);
    }

    spin_unlock_irqrestore(&p_this->lock, flags);
}

/*------------------------------------------------------------------
 * Func : rtd_cec_rx_stop
 *
 * Desc : stop rx
 *
 * Parm : p_this : handle of rtd cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static
void rtd_cec_rx_stop(rtk_cec *p_this)
{
    cm_buff *cmb;
    unsigned long flags;

    spin_lock_irqsave(&p_this->lock, flags);

    if (p_this->rcv.enable) {
        cec_info("rx stop\n");

        write_reg32(KDRV_CEC_RXCR0, RX_RST);
        wmb();
        write_reg32(KDRV_CEC_RXCR0, 0);
        wmb();

        if (p_this->rcv.cmb) {
            cmb_purge(p_this->rcv.cmb);
            cmb_reserve(p_this->rcv.cmb, 1);
            cmb_queue_tail(&p_this->rx_free_queue, p_this->rcv.cmb);
        }

        while ((cmb = cmb_dequeue(&p_this->rx_queue))) {
            cmb_purge(cmb);
            cmb_reserve(cmb, 1);
            cmb_queue_tail(&p_this->rx_free_queue, cmb);
        }

        p_this->rcv.enable = 0;
        p_this->rcv.state = IDEL;
        p_this->rcv.cmb = NULL;

        wake_up_interruptible(&p_this->rcv.wq);
    }
    spin_unlock_irqrestore(&p_this->lock, flags);
}

/*------------------------------------------------------------------
 * Func : rtd_cec_isr
 *
 * Desc : isr of rtd cec
 *
 * Parm : p_this : handle of rtd cec
 *        dev_id : handle of the rtd_cec
 *
 * Retn : IRQ_NONE, IRQ_HANDLED
 *------------------------------------------------------------------*/
static
irqreturn_t rtd_cec_isr(int this_irq, void *dev_id)
{
    rtk_cec *p_this = (rtk_cec *) dev_id;
    irqreturn_t ret = IRQ_NONE;

    if (read_reg32(KDRV_CEC_TXCR0) & TX_INT) {
        rtd_cec_tx_work(&p_this->xmit.work.work);
        ret = IRQ_HANDLED;
    }

    if (read_reg32(KDRV_CEC_RXCR0) & RX_INT) {
        rtd_cec_rx_work(&p_this->rcv.work.work);
        ret = IRQ_HANDLED;
    }

    return ret;
}

static void  rtd_cec_intial_hw_setting(void)
{
    CRT_CLK_OnOff(CEC, 0, NULL);        // disable clock
    CRT_CLK_OnOff(CEC, 1, NULL);        // enable clock

#ifdef CONFIG_ARCH_RTK6702  
    //only 6702 has change the reg;
    write_reg32Mask( STB_SC_LV_RST_reg, ~(CES_INPUT_CLOCK_POLARITY_MASK), CES_INPUT_CLOCK_POLARITY(CEC_POSITIVE_CLOCK));   //MM2KERNDRV-489 , next soc, eneed modify this polarity setting

    write_reg32(KDRV_CEC_CR0, CEC_MODE(0) | TEST_MODE_PAD_EN |
                LOGICAL_ADDR(0xF) | TIMER_DIV(25) | UNREG_ACK_EN);

    write_reg32(KDRV_CEC_RTCR0,
                CEC_PAD_EN | CEC_PAD_EN_MODE | PRE_DIV(_get_pre_div()) | RETRY_NO(rtk_cec_retry_num));

    /* setup internal pull high register... */
    write_reg32(GDI_POWER_SAVING_MODE,  CEC_CLK_POL(CEC_POSITIVE_CLOCK) |
                CEC_POWER_SAVING_MODE_cec_rpu_en(0x1) | CEC_POWER_SAVING_MODE_cec_rsel(0x6));
#else
    write_reg32(KDRV_CEC_CR0, CEC_MODE(0) | TEST_MODE_PAD_EN |
                LOGICAL_ADDR(0xF) | TIMER_DIV(25) |
                PRE_DIV(_get_pre_div()) | UNREG_ACK_EN);

    write_reg32(KDRV_CEC_RTCR0,
                CEC_PAD_EN | CEC_PAD_EN_MODE | RETRY_NO(rtk_cec_retry_num));
#if defined (CONFIG_ARCH_RTK2851A) || defined (CONFIG_ARCH_RTK2851C) || defined (CONFIG_ARCH_RTK2851F)
    /* setup internal pull high register... */
    write_reg32(GDI_POWER_SAVING_MODE,  CEC_CLK_POL(CEC_POSITIVE_CLOCK) | 0x30);
#else
    /* setup internal pull high register... */
    write_reg32(GDI_POWER_SAVING_MODE,  CEC_CLK_POL(CEC_POSITIVE_CLOCK) |
                CEC_POWER_SAVING_MODE_cec_rpu_en(1) | CEC_POWER_SAVING_MODE_cec_rsel(6));
#endif


#endif

    write_reg32(KDRV_CEC_RXCR0, RX_RST);

    write_reg32(KDRV_CEC_RXCR0, 0);

    write_reg32(KDRV_CEC_TXCR0, TX_RST);

    write_reg32(KDRV_CEC_TXCR0, 0);

    write_reg32(KDRV_CEC_RXTCR0,
                RX_START_LOW(0x60) | RX_START_PERIOD(0xC8) |
                RX_DATA_SAMPLE(42) | RX_DATA_PERIOD(PATCH_RX_DATA_PERIOD));

    write_reg32(KDRV_CEC_RXTCR1,
                CEC_RxTCR1_rx_data_period_chk(140) | CEC_RxTCR1_rx_start_high(0xA0) );

    write_reg32(KDRV_CEC_TXTCR0, TX_START_LOW(149) | TX_START_HIGH(32));

    write_reg32(KDRV_CEC_TXTCR1,
                TX_DATA_LOW(25) | TX_DATA_01(36) |
                TX_DATA_HIGH(36));
    write_reg32(KDRV_CEC_RxACKTCR1,
                RX_ACK_LOW_SEL | RX_ACK_LOW(20) | RX_ACK_BIT(30) |
                RX_ACK_LINE_ERR(94));

#ifdef CEC_OPCODE_COMPARE_ENABLE
    write_reg32(GDI_CEC_OPCODE_ENABLE, 0);  /* disable opcode compare function */
#endif

}

int rtd_cec_set_global_interrupt(rtk_cec *p_this, unsigned long onoff)
{
    cec_info("rtd_cec_set_global_interrupt, onoff=%lu \n", onoff);

    if (onoff)
    {
        write_reg32(CEC_INT_EN, (CEC_INT_EN_VAL | CEC_INT_EN_WRITE_DATA));
    }
    else
    {
        write_reg32(CEC_INT_EN, CEC_INT_EN_VAL);
    }

    return 0;
}



/*------------------------------------------------------------------
 * Func : rtd_cec_init
 *
 * Desc : init a rtd cec adapter
 *
 * Parm : N/A
 *
 * Retn : handle of cec module
 *------------------------------------------------------------------*/
int rtd_cec_init(rtk_cec *p_this)
{
    cm_buff *cmb = NULL;
    int i;

    if (!p_this->status.init) {
        cec_info("Open %s CEC\n", CEC_MODEL_NAME);

        rtd_cec_intial_hw_setting();

        spin_lock_init(&p_this->lock);
        p_this->xmit.state = IDEL;
        p_this->xmit.cmb = NULL;
        p_this->xmit.timeout = 0;

        p_this->rcv.state = IDEL;
        p_this->rcv.cmb = NULL;
        init_waitqueue_head(&p_this->rcv.wq);

        cmb_queue_head_init(&p_this->tx_queue);
        cmb_queue_head_init(&p_this->rx_queue);
        cmb_queue_head_init(&p_this->rx_free_queue);

        for (i = 0; i < RX_RING_LENGTH; i++) {
            cmb = alloc_cmb(RX_BUFFER_SIZE);
            if (cmb)
                cmb_queue_tail(&p_this->rx_free_queue, cmb);
        }
        p_this->status.init = 1;
    }

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_set_mode
 *
 * Desc : enable ces module
 *
 * Parm : p_this   : handle of rtd cec adapter
 *        mode     : CEC_MODE_OFF : disable CEC module
 *                   CEC_MODE_ON : enable CEC module
 *                   CEC_MODE_OFF : put CEC module into standby mode
 *
 * Retn : 0 for success, others failed
 *------------------------------------------------------------------*/
int rtd_cec_set_mode(rtk_cec *p_this, unsigned char mode)
{
    switch (mode) {
        case CEC_MODE_OFF:  /* disable */

            cec_info("%s cec mode : OFF\n", CEC_MODEL_NAME);
            write_reg32(KDRV_CEC_CR0,
                        (read_reg32(KDRV_CEC_CR0) & ~CTRL_MASK1) | CEC_MODE(0) |
                        LOGICAL_ADDR(0xF));
            p_this->status.enable = 0;
            p_this->status.standby_mode = 0;
            rtd_cec_rx_stop(p_this);
            rtd_cec_tx_stop(p_this);

            break;

        case CEC_MODE_ON:

            cec_info("%s cec mode : On\n", CEC_MODEL_NAME);
            write_reg32(KDRV_CEC_CR0,
                        (read_reg32(KDRV_CEC_CR0) & ~CTRL_MASK1) | CEC_MODE(1));
#ifdef CEC_OPCODE_COMPARE_ENABLE
            write_reg32(GDI_CEC_OPCODE_ENABLE, 0);  /*disable opcode compare function*/
#endif
            rtd_cec_rx_start(p_this);
            rtd_cec_tx_start(p_this);
            p_this->status.enable = 1;
            p_this->status.standby_mode = 0;

            break;

        case CEC_MODE_STANDBY:

            cec_info("%s cec mode : Standby (la=%x)\n", CEC_MODEL_NAME,
                     rtk_cec_standby_logical_addr);
            write_reg32(KDRV_CEC_CR0,
                        (read_reg32(KDRV_CEC_CR0) & ~CTRL_MASK1) | CEC_MODE(1) |
                        LOGICAL_ADDR(rtk_cec_standby_logical_addr));
            rtd_cec_rx_start(p_this);
            rtd_cec_tx_start(p_this);
            p_this->status.enable = 1;
            p_this->status.standby_mode = 1;
            break;
    }
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_enable
 *
 * Desc : enable ces module
 *
 * Parm : p_this   : handle of rtd cec adapter
 *        on_off   : 0 : disable, others enable
 *
 * Retn : 0 for success, others failed
 *------------------------------------------------------------------*/
int rtd_cec_enable(rtk_cec *p_this, unsigned char on_off)
{
    if (on_off) {
        rtd_cec_set_mode(p_this, CEC_MODE_ON);
    } else {
        if (rtk_cec_standby_config == 0
            || rtk_cec_standby_logical_addr >= 0xF
            || rtk_cec_standby_physical_addr == 0xFFFF)
            rtd_cec_set_mode(p_this, CEC_MODE_OFF);
        else
            rtd_cec_set_mode(p_this, CEC_MODE_STANDBY); /* standby mode */
    }

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_set_rx_mask
 *
 * Desc : set rx mask rtd ces
 *
 * Parm : p_this    : handle of rtd cec adapter
 *        rx_mask   : rx mask
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtd_cec_set_rx_mask(rtk_cec *p_this, unsigned short rx_mask)
{
    unsigned char log_addr = 0xf;
    int i;

    for (i = 0; i < 16; i++) {
        if (TEST_BIT(rx_mask, i)) {
            log_addr = i;
            break;
        }
    }

    if (rx_mask & ~(BIT_MASK(15) | BIT_MASK(log_addr))) {
        cec_info
        ("cec : multiple address specified (%04x), only address %x,f are valid\n",
         rx_mask, log_addr);
    }

    write_reg32(KDRV_CEC_CR0,
                (read_reg32(KDRV_CEC_CR0) & ~LOGICAL_ADDR_MASK) |
                LOGICAL_ADDR(log_addr));

    cec_info("cec : logical address = %02x\n", log_addr);

    if (log_addr != 0xF)
        rtk_cec_standby_logical_addr = log_addr;    /* save the latest vaild logical address */

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_set_physical_addr
 *
 * Desc : set physical address of rtd ces
 *
 * Parm : p_this    : handle of rtd cec adapter
 *        phy_addr  : physical address
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtd_cec_set_physical_addr(rtk_cec *p_this, unsigned short phy_addr)
{
    cec_info("cec : physcial address = %04x\n", phy_addr);

    /* we always keep latest valid physical address during standby */
    if (phy_addr != 0xffff)
        rtk_cec_standby_physical_addr = phy_addr;


    return 0;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_set_cec_version
 *
 * Desc : set cec version
 *
 * Parm : p_this    : handle of rtd cec adapter
 *        version   : cec version number
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtd_cec_set_cec_version(rtk_cec *p_this, unsigned char version)
{
    rtk_cec_standby_cec_version = version;
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_set_device_vendor_id
 *
 * Desc : set vendor id of rtd ces
 *
 * Parm : p_this    : handle of rtd cec adapter
 *        vendor_id  : device vendor id
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtd_cec_set_device_vendor_id(rtk_cec *p_this,
                                     unsigned long vendor_id)
{
    rtk_cec_standby_vendor_id = vendor_id;
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_set_stanby_mode
 *
 * Desc : set standy mode condition of rtd ces
 *
 * Parm : p_this  : handle of rtd cec adapter
 *        mode    : cec standby mode
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtd_cec_set_stanby_mode(rtk_cec *p_this, unsigned long mode)
{
    rtk_cec_standby_config = mode;
    return 0;
}



/*------------------------------------------------------------------
 * Func : rtd_cec_set_retry_num
 *
 * Desc : set retry times of rtd cec
 *
 * Parm : p_this  : handle of rtd cec adapter
 *        mun    : retry times
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtd_cec_set_retry_num(rtk_cec *p_this, unsigned long num)
{
    rtk_cec_retry_num = num;
    write_reg32(KDRV_CEC_RTCR0,
                ((read_reg32(KDRV_CEC_RTCR0) & ~RETRY_NO_MASK) | RETRY_NO(rtk_cec_retry_num)));
    return 0;
}

#define CBUS_PMM_CBUS_PHY_3_reg 0xb8062b3c

/*------------------------------------------------------------------
 * Func : rtd_cec_set_hotplug
 *
 * Desc : manual set hotplug of rtd cec
 *
 * Parm : p_this  : handle of rtd cec adapter
 *        onoff   : hotplug onoff
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtd_cec_set_hotplug(rtk_cec *p_this, unsigned long onoff)
{
    cec_info("rtd_cec_set_hotplug, onoff=%lu \n", onoff);

    if (onoff)
    {
        write_reg32(CBUS_PMM_CBUS_PHY_3_reg, ((read_reg32(CBUS_PMM_CBUS_PHY_3_reg) & 0xffffbffe) | 0x4000));
    }
    else
    {
        write_reg32(CBUS_PMM_CBUS_PHY_3_reg, ((read_reg32(CBUS_PMM_CBUS_PHY_3_reg) & 0xffffbffe) | 0x4001));
    }

    return 0;
}

extern bool pm_freezing;
static void rtk_cec_wait_xmit_done(cm_buff *cmb)
{
#if 0
    int wait_interval = msecs_to_jiffies(100);
    int wait_jiffies = 0;
    while(wait_jiffies < TX_TIMEOUT + 50) {
        wait_event_timeout(cmb->wq, (cmb->status != WAIT_XMIT) || (pm_freezing == true), wait_interval);
        if((cmb->status != WAIT_XMIT) || (pm_freezing == true))
            break;
        wait_jiffies += wait_interval;
    }
#else

    wait_event_timeout(cmb->wq, cmb->status != WAIT_XMIT, TX_TIMEOUT + 50);

#endif
}

/*------------------------------------------------------------------
 * Func : rtd_cec_xmit_msg
 *
 * Desc : xmit message
 *
 * Parm : p_this   : handle of rtd cec
 *        cmb      : msg to xmit
 *
 * Retn : 0 : for success, others : fail
 *------------------------------------------------------------------*/
int rtd_cec_xmit_message(rtk_cec *p_this, cm_buff *cmb,
                             unsigned long flags)
{
    int ret = 0;
    unsigned char status;
    int is_poll_message = 0;
    if (!p_this->xmit.enable)
        return -1;

    if(cmb->len == 1)
        is_poll_message = (cmb->data[0] & 0xf) + 1;

    cmb->flags = flags;
    cmb->status = WAIT_XMIT;
    cec_msg_dump(cmb, "TX");

    cmb_queue_tail(&p_this->tx_queue, cmb);

    schedule_delayed_work(&p_this->xmit.work, 0);

    if ((cmb->flags & NONBLOCK) == 0) {

        rtk_cec_wait_xmit_done(cmb);
        status = cmb->status;
        switch (status) {
            case XMIT_OK:
            case XMIT_ABORT:
            case XMIT_FAIL:
                if(status == XMIT_OK) {
                    if (is_poll_message) 
                        cec_dbg("TX TV         -> %-11s : <CEC_POLLING_MESSAGE>", get_logic_addr_name_by_logic_addr(is_poll_message - 1));
                    cec_tx_dbg("cec : xmit message success\n");
                } else if(status == XMIT_ABORT) {
                    cec_warn("cec : xmit message abort\n");
                } else{
                    if(!is_poll_message)
                        cec_warn("cec : xmit message failed\n");
                }

                spin_lock_irqsave(&p_this->lock, flags);
                kfree_cmb(cmb);
                spin_unlock_irqrestore(&p_this->lock, flags);
                break;

            case XMIT_TIMEOUT:
            case WAIT_XMIT:
            default:
                cec_warn("cec : xmit message timeout\n");
                spin_lock_irqsave(&p_this->lock, flags);
                if(p_this->xmit.cmb == cmb) {
                    p_this->xmit.state = IDEL;
                    p_this->xmit.cmb = NULL;
                } else {
                    cmb_dequeue_find(&p_this->tx_queue, cmb);
                }

                kfree_cmb(cmb);
                spin_unlock_irqrestore(&p_this->lock, flags);
                break;
        }

        ret = (status == XMIT_OK) ? 0 : -1;

    }

    return ret;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_read_message
 *
 * Desc : read message
 *
 * Parm : p_this : handle of cec device
 *        flags  : flag of current read operation
 *
 * Retn : cec message
 *------------------------------------------------------------------*/
static cm_buff *rtd_cec_read_message(rtk_cec *p_this,
        unsigned char flags)
{
    while (!(flags & NONBLOCK) && p_this->status.enable& !cmb_queue_len(&p_this->rx_queue))
        wait_event_freezable(p_this->rcv.wq, !p_this->status.enable|| cmb_queue_len(&p_this->rx_queue));

    if (p_this->status.enable)
    {
        cm_buff *cmb = cmb_dequeue(&p_this->rx_queue);

        if (cmb)
        {
            cm_buff *cmb_rx = alloc_cmb(RX_BUFFER_SIZE);
            if (cmb_rx)
            {
                cec_msg_dump(cmb, "RX");

                cmb_queue_tail(&p_this->rx_free_queue, cmb_rx);
                return cmb;
            }
        }
    }

    return NULL;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_uninit
 *
 * Desc : uninit a rtd cec adapter
 *
 * Parm : N/A
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtd_cec_uninit(rtk_cec *p_this)
{
    cec_info("%s cec closed\n", CEC_MODEL_NAME);
    rtd_cec_set_global_interrupt(p_this, 0);
    write_reg32(KDRV_CEC_CR0, read_reg32(KDRV_CEC_CR0) & ~CEC_MODE_MASK);
    write_reg32(KDRV_CEC_RXCR0, 0);
    write_reg32(KDRV_CEC_TXCR0, 0);
    rtd_cec_enable(p_this, 0);
    p_this->status.init = 0;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_suspend
 *
 * Desc : suspend a rtd cec adapter
 *
 * Parm : p_this : handle of rtd cec adapter
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtd_cec_suspend(rtk_cec *p_this)
{
    p_this->status.state_before_suspend = p_this->status.enable;
    rtd_cec_set_global_interrupt(p_this, 0);
    if (!p_this->status.enable) { /*CEC is off*/
        cec_warn("[CEC]function is off when power on, doesn't enable!!\n");
        return 0;
    }

    cec_info("%s cec suspended (la=%x, pa=%04x, standby_mode=%ld)\n",
             CEC_MODEL_NAME, rtk_cec_standby_logical_addr,
             rtk_cec_standby_physical_addr, rtk_cec_standby_config);

    rtd_cec_set_mode(p_this, CEC_MODE_OFF);

    if (rtk_cec_standby_config && rtk_cec_standby_logical_addr < 0xF
        && rtk_cec_standby_physical_addr != 0xFFFF) {
        p_this->status.enable = 1;
        p_this->status.standby_mode = 1;
#ifdef CONFIG_ARCH_RTK6702
        write_reg32(KDRV_CEC_CR0,
                    (read_reg32(KDRV_CEC_CR0) & ~(LOGICAL_ADDR(0xF) |
                    TIMER_DIV(0xFF) )) | CEC_MODE(1) | TIMER_DIV(0x14) |
                    LOGICAL_ADDR(rtk_cec_standby_logical_addr));

        write_reg32(KDRV_CEC_RTCR0, (read_reg32(KDRV_CEC_RTCR0) & ~(PRE_DIV(0x3FF))) | PRE_DIV(0x21));
#else
        write_reg32(KDRV_CEC_CR0,
                    (read_reg32(KDRV_CEC_CR0) & ~(LOGICAL_ADDR(0xF) |
                                             TIMER_DIV(0xFF) | PRE_DIV(0xFF))) |
                        CEC_MODE(1) | TIMER_DIV(0x14) |
                        PRE_DIV(0x21) | LOGICAL_ADDR(rtk_cec_standby_logical_addr));

        write_reg32(KDRV_CEC_RXTCR0, RX_START_LOW(135) | RX_START_PERIOD(192) |
            RX_DATA_SAMPLE(42) | RX_DATA_PERIOD(PATCH_RX_DATA_PERIOD));

#endif

        write_reg32(KDRV_CEC_RXCR0, RX_RST);

        write_reg32(KDRV_CEC_RXCR0, 0);

        write_reg32(KDRV_CEC_TXCR0, TX_RST);

        write_reg32(KDRV_CEC_TXCR0, 0);

#ifdef CEC_OPCODE_COMPARE_ENABLE
        write_reg32(GDI_CEC_OPCODE_ENABLE, 0);  /*disable */

        /*-- compare 1 */
        if (rtk_cec_standby_config &
            STANBY_RESPONSE_GIVE_POWER_STATUS) {
            write_reg32(GDI_CEC_COMPARE_OPCODE_01,
                        CEC_MSG_GIVE_DEVICE_POWER_STATUS);
            write_reg32(GDI_CEC_SEND_OPCODE_01, CEC_MSG_REPORT_POWER_STATUS);
            write_reg32(GDI_CEC_SEND_OPERAND_NUMBER_01, 1);
            write_reg32(GDI_CEC_OPERAND_01, CEC_POWER_STATUS_STANDBY);
            write_reg32(GDI_CEC_OPCODE_ENABLE,
                        (read_reg32(GDI_CEC_OPCODE_ENABLE) | 0x01));
        }
        /*-- compare 2 */
        if (rtk_cec_standby_config &
            STANBY_RESPONSE_GIVE_PHYSICAL_ADDR) {
            write_reg32(GDI_CEC_COMPARE_OPCODE_02, CEC_MSG_GIVE_PHYSICAL_ADDRESS);
            write_reg32(GDI_CEC_SEND_OPCODE_02,   CEC_MSG_REPORT_PHYSICAL_ADDRESS);
            write_reg32(GDI_CEC_SEND_OPERAND_NUMBER_02, 2);
            write_reg32(GDI_CEC_OPERAND_04,
                        (rtk_cec_standby_physical_addr >> 8) &
                        0xFF);
            write_reg32(GDI_CEC_OPERAND_05,
                        (rtk_cec_standby_physical_addr) & 0xFF);
            /*write_reg32(0xb8061ecc, (read_reg32(0xb8061ecc)|0x02)); //invalid, disable temporarily*/
        }
        /*-- compare 3*/
        if (rtk_cec_standby_config &
            STANBY_RESPONSE_GET_CEC_VERISON) {
            write_reg32(GDI_CEC_COMPARE_OPCODE_03, CEC_MSG_GET_CEC_VERSION);
            write_reg32(GDI_CEC_SEND_OPCODE_03, CEC_MSG_CEC_VERSION);
            write_reg32(GDI_CEC_SEND_OPERAND_NUMBER_03, 1);
            write_reg32(GDI_CEC_OPERAND_07,
                        rtk_cec_standby_cec_version);
            write_reg32(GDI_CEC_OPCODE_ENABLE,
                        (read_reg32(GDI_CEC_OPCODE_ENABLE) | 0x04));
        }
        /*-- compare 4*/
        if (rtk_cec_standby_config &
            STANBY_RESPONSE_GIVE_DEVICE_VENDOR_ID) {
            write_reg32(GDI_CEC_COMPARE_OPCODE_04, CEC_MSG_GIVE_DEVICE_VENDOR_ID);
            write_reg32(GDI_CEC_SEND_OPCODE_04, CEC_MSG_DEVICE_VENDOR_ID);
            write_reg32(GDI_CEC_SEND_OPERAND_NUMBER_04, 3);
            write_reg32(GDI_CEC_OPERAND_10,
                        (rtk_cec_standby_vendor_id >> 16) &
                        0xFF);
            write_reg32(GDI_CEC_OPERAND_11,
                        (rtk_cec_standby_vendor_id >> 8) &
                        0xFF);
            write_reg32(GDI_CEC_OPERAND_12,
                        (rtk_cec_standby_vendor_id) & 0xFF);
            write_reg32(GDI_CEC_OPCODE_ENABLE,
                        (read_reg32(GDI_CEC_OPCODE_ENABLE) | 0x08));
        }
        cec_info("CEC_COMP_ENABLE = %08x\n", read_reg32(GDI_CEC_OPCODE_ENABLE));
#endif /*ifdef CEC_OPCODE_COMPARE_ENABLE*/
        write_reg32(KDRV_CEC_RXCR0, RX_EN | RX_CONTINUOUS);  /* enable RX */

        cec_info("cec standby enabled\n");
    }

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_resume
 *
 * Desc : suspend a rtd cec adapter
 *
 * Parm : p_this : handle of rtd cec adapter
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtd_cec_resume(rtk_cec *p_this)
{
    cec_warn("%s cec resume\n", CEC_MODEL_NAME);

    /*p_this->status.enable = 0;*/
    p_this->status.standby_mode = 0;

    rtd_cec_intial_hw_setting();
    rtd_cec_set_global_interrupt(p_this, 1);

    if(p_this->status.state_before_suspend == 1)
    {
        p_this->status.state_before_suspend = 0;
        rtd_cec_set_mode(p_this, CEC_MODE_ON);
    }
    else if (rtk_cec_standby_config && rtk_cec_standby_logical_addr < 0xF
                && rtk_cec_standby_physical_addr != 0xFFFF)
    {
        rtd_cec_set_mode(p_this, CEC_MODE_STANDBY);
    }
    else
    {
        rtd_cec_set_mode(p_this, CEC_MODE_OFF);
    }

    return 0;
}

/*------------------------------------------------------------------
 * Func : pm_rtd_cec_suspend
 *
 * Desc : Force rtd cec to enter suspend state.
 *        this function will be called before enter 8051.
 *
 * Parm : None
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int pm_rtd_cec_suspend(void)
{
    return rtd_cec_suspend(&m_cec);
}
EXPORT_SYMBOL(pm_rtd_cec_suspend);

/*------------------------------------------------------------------
 * Func : rtd_cec_early_enable
 *
 * Desc : Force darwin cec to enable when the system is cold boot.
 *        this function will be called by 8051
 *
 * Parm : None
 *
 * Retn : None
 *------------------------------------------------------------------*/
void rtd_cec_early_enable(void)
{
#ifdef CONFIG_ARCH_RTK6702
    /*general setup*/
    write_reg32(KDRV_CEC_CR0, CEC_MODE(0) | TEST_MODE_PAD_EN |
                LOGICAL_ADDR(0xF) | TIMER_DIV(25) | UNREG_ACK_EN);
    /*reset engine*/
    write_reg32(KDRV_CEC_RTCR0,
                CEC_PAD_EN | CEC_PAD_EN_MODE | PRE_DIV(_get_pre_div()) | RETRY_NO(rtk_cec_retry_num));
#else
    /*general setup*/
    write_reg32(KDRV_CEC_CR0, CEC_MODE(1) | TEST_MODE_PAD_EN |
                LOGICAL_ADDR(0) | TIMER_DIV(25) |
                PRE_DIV(_get_pre_div()) | UNREG_ACK_EN);
    /*reset engine*/
    write_reg32(KDRV_CEC_RTCR0, RETRY_NO(rtk_cec_retry_num));
#endif
    write_reg32(KDRV_CEC_RXCR0, RX_RST);
    write_reg32(KDRV_CEC_RXCR0, 0);
    write_reg32(KDRV_CEC_TXCR0, TX_RST);
    write_reg32(KDRV_CEC_TXCR0, 0);

#ifdef CONFIG_ARCH_RTK6702
    /*timing setup*/
    write_reg32(KDRV_CEC_RXTCR0,
                RX_START_LOW(0x60) | RX_START_PERIOD(0xC8) |
                RX_DATA_SAMPLE(42) | RX_DATA_PERIOD(PATCH_RX_DATA_PERIOD));
    write_reg32(KDRV_CEC_TXTCR0, TX_START_LOW(149) | TX_START_HIGH(32));
    write_reg32(KDRV_CEC_TXTCR1,
                TX_DATA_LOW(25) | TX_DATA_01(36) |
                TX_DATA_HIGH(36));
#else
    write_reg32(KDRV_CEC_RXTCR0, RX_START_LOW(135) | RX_START_PERIOD(188) |
                RX_DATA_SAMPLE(42) | RX_DATA_PERIOD(PATCH_RX_DATA_PERIOD));
    write_reg32(KDRV_CEC_TXTCR0, TX_START_LOW(149) | TX_START_HIGH(31));
    write_reg32(KDRV_CEC_TXTCR1,
                TX_DATA_LOW(25) | TX_DATA_01(36) | TX_DATA_HIGH(37));
#endif

    write_reg32(KDRV_CEC_RxACKTCR1,
                RX_ACK_LOW_SEL | RX_ACK_LOW(20) | RX_ACK_BIT(30) |
                RX_ACK_LINE_ERR(94));
    /*enable Rx*/
    write_reg32(KDRV_CEC_RXCR0, RX_EN | RX_CONTINUOUS);  /* enable RX */
}


void rtd_cec_dump_current_setting(char *buf, int in_standby)
{
    if(buf == NULL) {
        cec_info("****************The current setting of cec*****************\n");
        cec_info("CEC_CR0(0x%08x) = 0x%08x\n", KDRV_CEC_CR0, read_reg32(KDRV_CEC_CR0));
        cec_info("CEC_RTCR0(0x%08x) = 0x%08x\n", KDRV_CEC_RTCR0, read_reg32(KDRV_CEC_RTCR0));
        cec_info("CEC_RXCR0(0x%08x) = 0x%08x\n", KDRV_CEC_RXCR0, read_reg32(KDRV_CEC_RXCR0));
        cec_info("CEC_TXCR0(0x%08x) = 0x%08x\n", KDRV_CEC_TXCR0, read_reg32(KDRV_CEC_TXCR0));
        cec_info("CEC_RXTCR0(0x%08x) = 0x%08x\n", KDRV_CEC_RXTCR0, read_reg32(KDRV_CEC_RXTCR0));
        cec_info("CEC_TXTCR0(0x%08x) = 0x%08x\n", KDRV_CEC_TXTCR0, read_reg32(KDRV_CEC_TXTCR0));
        cec_info("CEC_TXTCR1(0x%08x) = 0x%08x\n", KDRV_CEC_TXTCR1, read_reg32(KDRV_CEC_TXTCR1));
        cec_info("GDI_POWER_SAVING_MODE(0x%08x) = 0x%08x\n",
                 GDI_POWER_SAVING_MODE, read_reg32(GDI_POWER_SAVING_MODE));
        cec_info("CEC_RxACKTCR0(0x%08x) = 0x%08x\n",
                 KDRV_CEC_RxACKTCR0, read_reg32(KDRV_CEC_RxACKTCR0));
        cec_info("CEC_RxACKTCR1(0x%08x) = 0x%08x\n",
                 KDRV_CEC_RxACKTCR1, read_reg32(KDRV_CEC_RxACKTCR1));
        cec_info("CEC_TxRxACKOPTION(0x%08x) = 0x%08x\n",
                 KDRV_CEC_TxRxACKOPTION, read_reg32(KDRV_CEC_TxRxACKOPTION));
        cec_info("CEC_TxRxACKOPTION(0x%08x) = 0x%08x\n",
                 KDRV_CEC_TxRxACKOPTION, read_reg32(KDRV_CEC_TxRxACKOPTION));
        cec_info("STB_ST_BUSCLK_reg(0x%08x) = 0x%08x\n",
                 STB_ST_BUSCLK_reg, read_reg32(STB_ST_BUSCLK_reg));
        cec_info("The current bus clk freq is %s Mhz\n",
                 (read_reg32(STB_ST_BUSCLK_reg) & (1 <<
                         STB_ST_BUSCLK_bus_clksel_shift)) ? "27" : "255");
    } else {
        buf[0] = 0;
        sprintf(buf + strlen(buf), "****************The current setting of cec*****************\n");
        sprintf(buf + strlen(buf), "CEC_CR0(0x%08x) = 0x%08x\n", KDRV_CEC_CR0, read_reg32(KDRV_CEC_CR0));
        sprintf(buf + strlen(buf), "CEC_RTCR0(0x%08x) = 0x%08x\n", KDRV_CEC_RTCR0, read_reg32(KDRV_CEC_RTCR0));
        sprintf(buf + strlen(buf), "CEC_RXCR0(0x%08x) = 0x%08x\n", KDRV_CEC_RXCR0, read_reg32(KDRV_CEC_RXCR0));
        sprintf(buf + strlen(buf), "CEC_TXCR0(0x%08x) = 0x%08x\n", KDRV_CEC_TXCR0, read_reg32(KDRV_CEC_TXCR0));
        sprintf(buf + strlen(buf), "CEC_RXTCR0(0x%08x) = 0x%08x\n", KDRV_CEC_RXTCR0, read_reg32(KDRV_CEC_RXTCR0));
        sprintf(buf + strlen(buf), "CEC_TXTCR0(0x%08x) = 0x%08x\n", KDRV_CEC_TXTCR0, read_reg32(KDRV_CEC_TXTCR0));
        sprintf(buf + strlen(buf), "CEC_TXTCR1(0x%08x) = 0x%08x\n", KDRV_CEC_TXTCR1, read_reg32(KDRV_CEC_TXTCR1));
        sprintf(buf + strlen(buf), "GDI_POWER_SAVING_MODE(0x%08x) = 0x%08x\n",
                GDI_POWER_SAVING_MODE, read_reg32(GDI_POWER_SAVING_MODE));
        sprintf(buf + strlen(buf), "CEC_RxACKTCR0(0x%08x) = 0x%08x\n",
                KDRV_CEC_RxACKTCR0, read_reg32(KDRV_CEC_RxACKTCR0));
        sprintf(buf + strlen(buf), "CEC_RxACKTCR1(0x%08x) = 0x%08x\n",
                KDRV_CEC_RxACKTCR1, read_reg32(KDRV_CEC_RxACKTCR1));
        sprintf(buf + strlen(buf), "CEC_TxRxACKOPTION(0x%08x) = 0x%08x\n",
                KDRV_CEC_TxRxACKOPTION, read_reg32(KDRV_CEC_TxRxACKOPTION));
        sprintf(buf + strlen(buf), "CEC_TxRxACKOPTION(0x%08x) = 0x%08x\n",
                KDRV_CEC_TxRxACKOPTION, read_reg32(KDRV_CEC_TxRxACKOPTION));
        sprintf(buf + strlen(buf), "STB_ST_BUSCLK_reg(0x%08x) = 0x%08x\n",
                STB_ST_BUSCLK_reg, read_reg32(STB_ST_BUSCLK_reg));
        sprintf(buf + strlen(buf), "The current bus clk freq is %s Mhz\n",
                (read_reg32(STB_ST_BUSCLK_reg) & (1 <<
                        STB_ST_BUSCLK_bus_clksel_shift)) ? "27" : "500");
        cec_info("%s", buf);
    }

}


#define CEC_BITS_MASK(msb, lsb)         ((0xFFFFFFFF >> (32-((msb) - (lsb) + 1))) << (lsb))

static unsigned int rtd_cec_setting_time_to_val(unsigned int setting_val)
{
#if 0
    unsigned int sample_unit = 25;/*us*/
#endif

    return (setting_val * 25);
}

static int rtd_cec_get_register_part(unsigned long reg, unsigned int mask, unsigned int shift)
{
    unsigned int reg_val = 0 ;


    reg_val = read_reg32(reg);
    reg_val = (reg_val & mask) >> shift;

    return reg_val;
}

static int rtd_cec_get_timing(char *attr_name, unsigned int *val)
{
    int ret = 0;

    if(attr_name == NULL || val == NULL)
        return -1;

    if(strcmp(attr_name, "rx_start_low") == 0)
    {
        *val = rtd_cec_get_register_part(KDRV_CEC_RXTCR0,
                                             CEC_RxTCR0_rx_start_low_mask,
                                             CEC_RxTCR0_rx_start_low_shift);
    }
    else if(strcmp(attr_name, "rx_start_period") == 0)
    {
        *val = rtd_cec_get_register_part(KDRV_CEC_RXTCR0,
                                             CEC_RxTCR0_rx_start_period_mask,
                                             CEC_RxTCR0_rx_start_period_shift);
    }
    else if(strcmp(attr_name, "rx_data_sample") == 0)
    {
        *val = rtd_cec_get_register_part(KDRV_CEC_RXTCR0,
                                             CEC_RxTCR0_rx_data_sample_mask,
                                             CEC_RxTCR0_rx_data_sample_shift);
    }
    else if(strcmp(attr_name, "rx_data_period") == 0)
    {
        *val = rtd_cec_get_register_part(KDRV_CEC_RXTCR0,
                                             CEC_RxTCR0_rx_data_period_mask,
                                             CEC_RxTCR0_rx_data_period_shift);
    }
    else if(strcmp(attr_name, "tx_start_low") == 0)
    {
        *val = rtd_cec_get_register_part(KDRV_CEC_TXTCR0,
                                             CEC_TxTCR0_tx_start_low_mask,
                                             CEC_TxTCR0_tx_start_low_shift);
    }
    else if(strcmp(attr_name, "tx_start_high") == 0)
    {
        *val = rtd_cec_get_register_part(KDRV_CEC_TXTCR0,
                                             CEC_TxTCR0_tx_start_high_mask,
                                             CEC_TxTCR0_tx_start_high_shift);
    }
    else if(strcmp(attr_name, "tx_data_low") == 0)
    {
        *val = rtd_cec_get_register_part(KDRV_CEC_TXTCR1,
                                             CEC_TxTCR1_tx_data_low_mask,
                                             CEC_TxTCR1_tx_data_low_shift);
    }
    else if(strcmp(attr_name, "tx_data_01") == 0)
    {
        *val = rtd_cec_get_register_part(KDRV_CEC_TXTCR1,
                                             CEC_TxTCR1_tx_data_01_mask,
                                             CEC_TxTCR1_tx_data_01_shift);
    }
    else if(strcmp(attr_name, "tx_data_high") == 0)
    {
        *val = rtd_cec_get_register_part(KDRV_CEC_TXTCR1,
                                             CEC_TxTCR1_tx_data_high_mask,
                                             CEC_TxTCR1_tx_data_high_shift);
    }
    else if(strcmp(attr_name, "rx_ack_low") == 0)
    {
        *val = rtd_cec_get_register_part(KDRV_CEC_RxACKTCR1,
                                             CEC_RxACKTCR1_rx_ack_low_mask,
                                             CEC_RxACKTCR1_rx_ack_low_shift);
    }
    else if(strcmp(attr_name, "rx_ack_bit") == 0)
    {
        *val = rtd_cec_get_register_part(KDRV_CEC_RxACKTCR1,
                                             CEC_RxACKTCR1_rx_ack_bit_mask,
                                             CEC_RxACKTCR1_rx_ack_bit_shift);
    }
    else if(strcmp(attr_name, "rx_ack_line_err") == 0)
    {
        *val = rtd_cec_get_register_part(KDRV_CEC_RxACKTCR1,
                                             CEC_RxACKTCR1_rx_ack_line_err_mask,
                                             CEC_RxACKTCR1_rx_ack_line_err_shift);
    }
    else
    {
        ret = -1;
    }

    if(ret == 0)
    {
        *val = rtd_cec_setting_time_to_val(*val);
    }


    return ret;
}

static unsigned int rtd_cec_time_to_setting_val(unsigned int time_usec)
{
#if 0
    unsigned int sample_unit = 25;/*us*/
#endif

    return (time_usec / 25 ) ;
}

static int rtd_cec_set_register_part(unsigned int reg, unsigned int mask, unsigned int val)
{
    unsigned int reg_val;
    reg_val = read_reg32(reg);

    cec_dbg( "before 0x%08X = 0x%08X\n", reg, read_reg32(reg));
    reg_val = (reg_val & (~mask)) | val;

    write_reg32(reg, reg_val);
    cec_dbg( "after 0x%08X = 0x%08X\n", reg, rtd_inl(reg));

    return 0;
}

static int rtd_cec_set_timing(char *attr_name, unsigned int val)
{
    int ret = 0;
    unsigned int setting_val;

    if(attr_name == NULL)
    {
        return -1;
    }

    setting_val = rtd_cec_time_to_setting_val(val);


    if(strcmp(attr_name, "rx_start_low") == 0)
    {
        ret = rtd_cec_set_register_part(KDRV_CEC_RXTCR0,
                                            CEC_RxTCR0_rx_start_low_mask, RX_START_LOW(setting_val));
    }
    else if(strcmp(attr_name, "rx_start_period") == 0)
    {
        ret = rtd_cec_set_register_part(KDRV_CEC_RXTCR0,
                                            CEC_RxTCR0_rx_start_period_mask, RX_START_PERIOD(setting_val));
    }
    else if(strcmp(attr_name, "rx_data_sample") == 0)
    {
        ret = rtd_cec_set_register_part(KDRV_CEC_RXTCR0,
                                            CEC_RxTCR0_rx_data_sample_mask, RX_DATA_SAMPLE(setting_val));
    }
    else if(strcmp(attr_name, "rx_data_period") == 0)
    {
        ret = rtd_cec_set_register_part(KDRV_CEC_RXTCR0,
                                            CEC_RxTCR0_rx_data_period_mask, RX_DATA_PERIOD(setting_val));
    }
    else if(strcmp(attr_name, "tx_start_low") == 0)
    {
        ret = rtd_cec_set_register_part(KDRV_CEC_TXTCR0,
                                            CEC_TxTCR0_tx_start_low_mask, TX_START_LOW(setting_val));
    }
    else if(strcmp(attr_name, "tx_start_high") == 0)
    {
        ret = rtd_cec_set_register_part(KDRV_CEC_TXTCR0,
                                            CEC_TxTCR0_tx_start_high_mask, TX_START_HIGH(setting_val));
    }
    else if(strcmp(attr_name, "tx_data_low") == 0)
    {
        ret = rtd_cec_set_register_part(KDRV_CEC_TXTCR1,
                                            CEC_TxTCR1_tx_data_low_mask, TX_DATA_LOW(setting_val));
    }
    else if(strcmp(attr_name, "tx_data_01") == 0)
    {
        ret = rtd_cec_set_register_part(KDRV_CEC_TXTCR1,
                                            CEC_TxTCR1_tx_data_01_mask, TX_DATA_01(setting_val));
    }
    else if(strcmp(attr_name, "tx_data_high") == 0)
    {
        ret = rtd_cec_set_register_part(KDRV_CEC_TXTCR1,
                                            CEC_TxTCR1_tx_data_high_mask, TX_DATA_HIGH(setting_val));
    }
    else if(strcmp(attr_name, "rx_ack_low") == 0)
    {
        ret = rtd_cec_set_register_part(KDRV_CEC_RxACKTCR1,
                                            CEC_RxACKTCR1_rx_ack_low_mask, RX_ACK_LOW(setting_val));
    }
    else if(strcmp(attr_name, "rx_ack_bit") == 0)
    {
        ret = rtd_cec_set_register_part(KDRV_CEC_RxACKTCR1,
                                            CEC_RxACKTCR1_rx_ack_bit_mask, RX_ACK_BIT(setting_val));
    }
    else if(strcmp(attr_name, "rx_ack_line_err") == 0)
    {
        ret = rtd_cec_set_register_part(KDRV_CEC_RxACKTCR1,
                                            CEC_RxACKTCR1_rx_ack_line_err_mask, RX_ACK_LINE_ERR(setting_val));
    }
    else
    {
        cec_warn("attr_name = %s is not existed \n", attr_name);
        ret = -1;
    }


    return ret;
}



/*//////////////////////////////////////////////////////////////////////
// CEC Driver Interface                                               //
//////////////////////////////////////////////////////////////////////*/

static int ops_probe(cec_device *dev)
{
    if (dev->id != ID_RTD_CEC_CONTROLLER)
        return -ENODEV;

    if (rtd_cec_init(&m_cec) == 0)
        cec_set_drvdata(dev, &m_cec);

    return 0;
}

static void ops_remove(cec_device *dev)
{
    rtd_cec_uninit((rtk_cec *) cec_get_drvdata(dev));
}

static int ops_enable(cec_device *dev, unsigned char on_off)
{

    cec_warn("ops_enable : %02x\n", on_off);
    if ((on_off & 0xf0) == 0) {
        if (on_off & 0x0f) { /*on*/
            rtk_cec_standby_config =
                (STANBY_RESPONSE_GIVE_POWER_STATUS |
                 STANBY_RESPONSE_GET_CEC_VERISON);
            rtk_cec_standby_logical_addr = 0x0;
            rtk_cec_standby_physical_addr = 0x0000;
        } else {
            rtk_cec_standby_config = 0;
            rtk_cec_standby_logical_addr = 0xF;
            rtk_cec_standby_physical_addr = 0xFFFF;
        }
    }
    return rtd_cec_enable((rtk_cec *) cec_get_drvdata(dev),
                              (on_off & 0x0f));
}

static int ops_set_rx_mask(cec_device *dev, unsigned short rx_mask)
{
    return rtd_cec_set_rx_mask((rtk_cec *) cec_get_drvdata(dev),
                                   rx_mask);
}

static int ops_set_physical_addr(cec_device *dev, unsigned short phy_addr)
{
    return rtd_cec_set_physical_addr((rtk_cec *)
                                         cec_get_drvdata(dev), phy_addr);
}

static int ops_set_cev_version(cec_device *dev, unsigned char version)
{
    return rtd_cec_set_cec_version((rtk_cec *) cec_get_drvdata(dev),
                                       version);
}

static int ops_set_device_vendor_id(cec_device *dev, unsigned long vendor_id)
{
    return rtd_cec_set_device_vendor_id((rtk_cec *)
                                            cec_get_drvdata(dev),
                                            vendor_id);
}

static int ops_xmit(cec_device *dev, cm_buff *cmb, unsigned long flags)
{
    return rtd_cec_xmit_message((rtk_cec *) cec_get_drvdata(dev),
                                    cmb, flags);
}

static cm_buff *ops_read(cec_device *dev, unsigned long flags)
{
    return rtd_cec_read_message((rtk_cec *) cec_get_drvdata(dev),
                                    flags);
}

static int ops_set_stanby_mode(cec_device *dev, unsigned long flags)
{
    return rtd_cec_set_stanby_mode((rtk_cec *) cec_get_drvdata(dev),
                                       flags);
}

static int ops_set_retry_num(cec_device *dev, unsigned long num)
{
    return rtd_cec_set_retry_num((rtk_cec *) cec_get_drvdata(dev),
                                     num);
}

static int ops_set_hotplug(cec_device *dev, unsigned long onoff)
{
    return rtd_cec_set_hotplug((rtk_cec *) cec_get_drvdata(dev), onoff);
}

static int ops_suspend(cec_device *dev)
{
    return rtd_cec_suspend((rtk_cec *) cec_get_drvdata(dev));
}

static int ops_resume(cec_device *dev)
{
    return rtd_cec_resume((rtk_cec *) cec_get_drvdata(dev));
}

static int ops_set_global_interrupt(cec_device *dev, unsigned long onoff)
{
    return rtd_cec_set_global_interrupt((rtk_cec *) cec_get_drvdata(dev),
                                     onoff);
}

static int ops_get_timing(cec_device *dev, unsigned char *name, unsigned int *p_value)
{
    return rtd_cec_get_timing(name, p_value);
}

static int ops_set_timing(cec_device *dev, unsigned char *name, unsigned int value)
{
    return rtd_cec_set_timing(name, value);
}

static cec_device rtd_cec_controller = {
    .id = ID_RTD_CEC_CONTROLLER,
    .name =  CEC_DRIVE_NAME
};

static cec_driver rtd_cec_driver = {

    .name = CEC_DRIVE_NAME,
    .probe = ops_probe,
    .remove = ops_remove,
    .suspend = ops_suspend,
    .resume = ops_resume,
    .enable = ops_enable,
    .xmit = ops_xmit,
    .read = ops_read,
    .set_rx_mask = ops_set_rx_mask,
    .set_physical_addr = ops_set_physical_addr,
    .set_cec_version = ops_set_cev_version,
    .set_device_vendor_id = ops_set_device_vendor_id,
    .set_stanby_mode = ops_set_stanby_mode,
    .set_retry_num = ops_set_retry_num,
    .set_cec_hotplug = ops_set_hotplug,
    .set_cec_global_interrupt = ops_set_global_interrupt,
    .set_timing = ops_set_timing,
    .get_timing = ops_get_timing,
};

static int rtk_cec_probe(struct platform_device *pdev)
{
    int irq;

    irq = platform_get_irq(pdev, 0);

    if (irq < 0){
        cec_warn("rtk_cec_probe get_irq failed \n");
        return irq;
    }

    if ( request_irq (irq, rtd_cec_isr, SA_INTERRUPT | SA_SHIRQ,"RTD293x CEC", &m_cec) < 0) {

        cec_warn("cec :open %s cec failed, unable to request irq #%d\n",CEC_MODEL_NAME, irq);
        return -1;
    }

    cec_warn("cec :open %s  request irq #%d\n",CEC_MODEL_NAME, irq);

    cec_info("cec :enable CEC SCPU interrupt\n");
    rtd_cec_set_global_interrupt(&m_cec, 1);

    return 0;
}
static int rtk_cec_remove(struct platform_device *pdev)
{
    int irq;

    irq = platform_get_irq(pdev, 0);

    if (irq >= 0){
        free_irq(irq, (void *)pdev);
    }

    return 0;
}
static const struct of_device_id of_rtk_cec_ids[] = {
    { .compatible = "realtek,rtk-cec" },
    {}
};
MODULE_DEVICE_TABLE(of, of_rtk_cec_ids);

static struct platform_driver cec_platform_drv =
{
    .probe      = rtk_cec_probe,
    .remove     = rtk_cec_remove,

    .driver   = {
        .name = "rtk-cec",
        .of_match_table = of_match_ptr(of_rtk_cec_ids),
    }
};

/*------------------------------------------------------------------
 * Func : rtd_cec_module_init
 *
 * Desc : rtd cec module init function
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int __init rtd_cec_module_init(void)
{
    cec_info("%s cec module init\n", CEC_MODEL_NAME);

    if (register_cec_driver(&rtd_cec_driver) != 0){
        cec_info("cec driver failed\n");
        return -EFAULT;
    }

    if(register_cec_device(&rtd_cec_controller)!=0){   /* register cec device */
        cec_info("cec device failed\n");
    }

    if(platform_driver_register(&cec_platform_drv)!=0){ /* regist cec platform driver */
        cec_info("cec platform driver failed\n");
    }
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtd_cec_module_exit
 *
 * Desc : rtd cec module exit function
 *
 * Parm : N/A
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
void __exit rtd_cec_module_exit(void)
{
    unregister_cec_device(&rtd_cec_controller);

    unregister_cec_driver(&rtd_cec_driver);

    platform_driver_unregister(&cec_platform_drv);
}

