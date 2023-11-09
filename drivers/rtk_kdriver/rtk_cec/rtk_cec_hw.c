/* -------------------------------------------------------------------------
   cec_rtd299x.c  cec driver for Realtek rtd299x
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
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_crt.h>
#include <rbus/stb_reg.h>
#include <mach/platform.h>
#include <media/cec.h>
#include "rtk_cec.h"
#include "rtk_cec_hw.h"
#include "rtk_cec_reg.h"
#include "rtk_cec_defines.h"
#include "rtk_cec_msg.h"

#define CEC_OPCODE_COMPARE_ENABLE
#ifdef CEC_OPCODE_COMPARE_ENABLE
#define IS_OPCODE_COMP_EN() (read_reg32(RTK_CEC_OPCODE_ENABLE)&0xf)
#define PWR_STS_COMP_EN()   (read_reg32(RTK_CEC_OPCODE_ENABLE)&0x00000001)
#define PHY_ADDR_COMP_EN()  (read_reg32(RTK_CEC_OPCODE_ENABLE)&0x00000002)
#define CEC_VER_COMP_EN()   (read_reg32(RTK_CEC_OPCODE_ENABLE)&0x00000004)
#define VEND_ID_COMP_EN()   (read_reg32(RTK_CEC_OPCODE_ENABLE)&0x00000008)
#endif

#define _reset_rx_fifo_ofst()       do { rx_fifo_ofst = 0; }while(0)
#define _reset_tx_fifo_ofst()       do { tx_fifo_ofst = 0; }while(0)

static unsigned long rtk_cec_standby_config = 0x001c1;
static unsigned char rtk_cec_standby_logical_addr = 0x0;
static unsigned short rtk_cec_standby_physical_addr = 0x0000;
static unsigned char rtk_cec_standby_cec_version = CEC_VERSION_1_4;
static unsigned long rtk_cec_standby_vendor_id = CEC_VENDOR_ID;
static unsigned long rtk_cec_retry_num = 5;

static struct rtk_cec_private *m_cec = NULL;
static unsigned char tx_fifo_ofst = 0;
static unsigned char rx_fifo_ofst = 0;

static volatile unsigned int g_is_poll_self = 0;

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
    if(rbus>=0x100)
        rbus=0xFF;

#endif

    cec_info("%s , pll = 0x%X , rbus = %d \n",__func__, pll , rbus );

    return rbus;
}



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
    unsigned int rx_fifo_data[4];
    unsigned int data_len = rtd_inl(RTK_CEC_RXCR0) & 0x1F;
    int rx_entry_cnt = 0;
    int i;

    cec_rx_dbg("rx_fifo_count = %u, RXCR0=%x, eom=%d\n",
               data_len, rtd_inl(RTK_CEC_RXCR0), eom);

    if (data_len == 0)
        return 0;

    if (data_len > rx_buff_size)
        data_len = rx_buff_size;

    if (data_len > 16)
        data_len = 16;

    if (!eom)   // not EOM, read data should be 4 byte alignment
        data_len &= ~0x3;

    rx_entry_cnt = (data_len + 3) >> 2;

    for (i = 0; i < rx_entry_cnt; i++) {
        rx_fifo_data[i] = __cpu_to_be32(rtd_inl(RTK_CEC_RXDR1 + (rx_fifo_ofst << 2)));
        cec_rx_dbg("rx_fifo_data[%d] = %u\n", i, rx_fifo_data[i]);
        rx_fifo_ofst++;
        rx_fifo_ofst &= 0x3;
    }

    memcpy(rx_buff, (unsigned char*) rx_fifo_data, data_len);  // KWarning: checked ok by kevin_wang@realtek.com

    rtd_outl(RTK_CEC_TXDR0, RX_SUB_CNT | FIFO_CNT(data_len)); /* rx fifo count -= rx_count */

    return data_len;
}


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
        rtd_outl(RTK_CEC_TXDR1 + (tx_fifo_ofst << 2), __cpu_to_be32(tx_fifo_data[i]));
        tx_fifo_ofst++;
    }

    rtd_outl(RTK_CEC_TXDR0, TX_ADD_CNT | FIFO_CNT(tx_len)); /* add data to fifo */

    return tx_len;
}


/*------------------------------------------------------------------
 * Func : rtk_cec_hw_rx_reset
 *
 * Desc : reset rx
 *
 * Parm : p_this : handle of rtd299x cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static
void rtk_cec_hw_rx_reset(struct rtk_cec_private *p_this)
{
    write_reg32(RTK_CEC_RXCR0, RX_RST);
    wmb();
    write_reg32(RTK_CEC_RXCR0, 0);
    wmb();
    p_this->rcv.state = IDEL;
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_tx_work
 *
 * Desc : rtd299x cec tx function
 *
 * Parm : p_this : handle of rtd299x cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_cec_hw_tx_work(struct work_struct *work)
{
    struct rtk_cec_private *p_this =
        (struct rtk_cec_private *) container_of(work, struct rtk_cec_private, xmit.work.work);

    rtk_cec_xmit *p_xmit = &p_this->xmit;
    unsigned char dest;
    unsigned long flags;
    int ret;

    spin_lock_irqsave(&p_this->lock, flags);

    if (p_xmit->state == XMIT) {
        if (read_reg32(RTK_CEC_TXCR0) & TX_EN) {
            /* xmitting */
            if (read_reg32(RTK_CEC_TXCR0) & TX_INT) {
                if (p_xmit->cmb->len) {
                    ret = _write_tx_fifo(p_xmit->cmb->data, (p_xmit->cmb->len > 8) ? 8 : p_xmit->cmb->len);
                    cmb_pull(p_xmit->cmb, ret);

                    if (p_xmit->cmb->len == 0)
                        write_reg32(RTK_CEC_TXCR0, read_reg32(RTK_CEC_TXCR0) & ~TX_CONTINUOUS); /* clear continue bits.... */
                }

                write_reg32(RTK_CEC_TXCR0, read_reg32(RTK_CEC_TXCR0));  /* clear interrupt */
            }

            if (time_after(jiffies, p_xmit->timeout)) {
                write_reg32(RTK_CEC_TXCR0, TX_INT);

                p_xmit->cmb->status = XMIT_TIMEOUT;
                p_xmit->state = IDEL;

                cec_warn("cec tx timeout\n");
                rtk_cec_dump_current_setting(NULL, 0);
                p_this->tx_state = STATE_NACK;
                if(g_is_poll_self)
                    cec_info("t %u\n", p_this->tx_state);
                cancel_delayed_work(&p_xmit->work);
            }
        } else {
            cancel_delayed_work(&p_xmit->work);

            if ((read_reg32(RTK_CEC_TXCR0) & TX_EOM) == 0) {
                p_xmit->cmb->status = XMIT_FAIL;
                p_this->tx_state = STATE_NACK;
                if(g_is_poll_self)
                    cec_info("f %u\n", p_this->tx_state);
                if(p_this->tx_msg.len != 1) {
                    cec_info("cec tx failed len %d\n", p_this->tx_msg.len);
                    rtk_cec_dump_current_setting(NULL, 0);
                }
            } else {
                p_xmit->cmb->status = XMIT_OK;
                p_this->tx_state = STATE_DONE;
		   if(g_is_poll_self)
                	cec_info("o %u %x %x\n", p_this->tx_state, read_reg32(RTK_CEC_TXCR0), read_reg32(RTK_CEC_TXDR1));
            }
            write_reg32(RTK_CEC_TXCR0, TX_INT);
            p_xmit->state = IDEL;
        }
    }

    if (p_xmit->state == IDEL) {
        if (p_xmit->enable) {
            if (p_xmit->cmb && p_xmit->cmb->status == WAIT_XMIT) {
                dest = (p_xmit->cmb->data[0] & 0xf);
                cmb_pull(p_xmit->cmb, 1);
                p_xmit->timeout = jiffies + CEC_TX_TIMEOUT;

                /* reset tx fifo */
                write_reg32(RTK_CEC_TXCR0, TX_RST);
                wmb();
                write_reg32(RTK_CEC_TXCR0, 0);
                wmb();

                _reset_tx_fifo_ofst();
                ret = _write_tx_fifo(p_xmit->cmb->data, (p_xmit->cmb->len > 16) ? 16 : p_xmit->cmb->len);
                cmb_pull(p_xmit->cmb, ret);


                if (p_xmit->cmb->len == 0)
                    write_reg32(RTK_CEC_TXCR0,
                                DEST_ADDR(dest) | TX_EN |
                                TX_INT_EN);
                else
                    write_reg32(RTK_CEC_TXCR0,
                                DEST_ADDR(dest) | TX_EN |
                                TX_INT_EN | TX_CONTINUOUS);

                p_xmit->state = XMIT;
		   if(g_is_poll_self)
                    cec_info("s\n");	

                /* queue delayed work for timeout detection */
                schedule_delayed_work(&p_xmit->work, CEC_TX_TIMEOUT + 1);
            }
        }
    }

    spin_unlock_irqrestore(&p_this->lock, flags);
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_tx_start
 *
 * Desc : start rx
 *
 * Parm : p_this : handle of rtd299x cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static
void rtk_cec_hw_tx_start(struct rtk_cec_private *p_this)
{
    unsigned long flags;
    spin_lock_irqsave(&p_this->lock, flags);

    write_reg32(RTK_CEC_RXCR0, (read_reg32(RTK_CEC_RXCR0) | RX_INT_EN) |
                RX_EN | RX_CONTINUOUS); /* enable RX */
    if (!p_this->xmit.enable) {
        cec_info("cec tx start\n");
        p_this->xmit.enable = 1;
        p_this->xmit.state = IDEL;

        INIT_DELAYED_WORK(&p_this->xmit.work, rtk_cec_hw_tx_work);
    }

    spin_unlock_irqrestore(&p_this->lock, flags);
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_tx_stop
 *
 * Desc : stop tx
 *
 * Parm : p_this : handle of rtd299x cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static
void rtk_cec_hw_tx_stop(struct rtk_cec_private *p_this)
{
    unsigned long flags;
    spin_lock_irqsave(&p_this->lock, flags);

    if (p_this->xmit.enable) {
        cec_info("cec tx stop\n");

        rtk_cec_hw_rx_reset(p_this);
        cancel_delayed_work(&p_this->xmit.work);
        /* reset tx fifo */
        write_reg32(RTK_CEC_TXCR0, TX_RST);
        wmb();
        write_reg32(RTK_CEC_TXCR0, 0);
        wmb();

        p_this->xmit.enable = 0;
        p_this->xmit.state = IDEL;
    }
    write_reg32(RTK_CEC_RXCR0, RX_EN | RX_CONTINUOUS);  /* enable RX */
    spin_unlock_irqrestore(&p_this->lock, flags);
}

int _rtk_cec_hw_xmit_message(struct rtk_cec_private *p_this, struct cec_msg *msg)
{
    cm_buff *cmb = NULL;
    if(!g_cec_send_enable)
        return -1;
    
    if (!p_this->xmit.enable)
        return -1;

    if(msg->len == 1 && msg->msg[0] == 0)
		g_is_poll_self = 1;
    else
		g_is_poll_self = 0;

    if(g_is_poll_self)
        cec_info("x\n");

    p_this->tx_msg = *msg;
    cmb = p_this->xmit.cmb;

    cmb_purge(cmb);
    memcpy(cmb_put(cmb, msg->len), msg->msg, msg->len);

    cmb->status = WAIT_XMIT;
    schedule_delayed_work(&p_this->xmit.work, 0);
    return 0;    
}

int rtk_cec_hw_xmit_message(struct rtk_cec_private *p_this, struct cec_msg *msg)
{
    int ret = 0;
    unsigned long flags;
    
    spin_lock_irqsave(&p_this->lock, flags);
    ret = _rtk_cec_hw_xmit_message(p_this, msg);
    spin_unlock_irqrestore(&p_this->lock, flags);
    return 0;
}

int rtk_cec_sysfs_msg_out(struct cec_msg *msg)
{
    int ret = 0;
    if (!m_cec)
        return -ENXIO;
    ret = rtk_cec_hw_xmit_message(m_cec, msg);
    return ret;
}
/*------------------------------------------------------------------
 * Func : rtk_cec_hw_standby_message_handler
 *
 * Desc : rtd299x cec message handler when cec works under standby mode
 *
 * Parm : p_this : handle of rtd299x cec
 *        init   : message initator
 *        dest   : message destination
 *        opcode : opcode of message
 *        param  : param of messsage
 *        len    : length of message parameter
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_cec_hw_standby_message_handler(struct rtk_cec_private *p_this,
                                        unsigned char init,
                                        unsigned char dest,
                                        unsigned char opcode,
                                        unsigned char *param,
                                        unsigned char len)
{
    struct cec_msg msg;

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

    memset(&msg, 0, sizeof(msg));

    /*cec_info("rtk_cec_hw_standby_message_handler : init = %x, dest=%x, opcode=%x, len=%d\n", init, dest, opcode, len); */

    switch (opcode) {
        case CEC_MSG_GIVE_DEVICE_POWER_STATUS:

            if (rtk_cec_standby_config &
                STANBY_RESPONSE_GIVE_POWER_STATUS && len == 0
                && init != 0xF) {

#ifdef CEC_OPCODE_COMPARE_ENABLE
                if (PWR_STS_COMP_EN())
                    break;
#endif

                msg.msg[0] = (dest << 4) | init;
                msg.msg[1] = CEC_MSG_REPORT_POWER_STATUS;
                msg.msg[2] = CEC_POWER_STATUS_STANDBY;
            }
            break;

        case CEC_MSG_GIVE_PHYSICAL_ADDRESS:

            if (rtk_cec_standby_config &
                STANBY_WAKEUP_BY_SET_STREAM_PATH && len == 0
                && dest != 0xF) {

#ifdef CEC_OPCODE_COMPARE_ENABLE
                if (PHY_ADDR_COMP_EN())
                    break;
#endif
                msg.msg[0] = (dest << 4) | 0xF;
                msg.msg[1] = CEC_MSG_REPORT_PHYSICAL_ADDRESS;
                msg.msg[2] =
                    (rtk_cec_standby_physical_addr >> 8) & 0xFF;
                msg.msg[3] =
                    (rtk_cec_standby_physical_addr) & 0xFF;
                msg.msg[4] = dev_type_map[dest];
            }
            break;

        case CEC_MSG_STANDBY:
            break;

        default:
            /* send feature abort */
            if (init != 0xF && dest != 0xF) {
                msg.msg[0] = (dest << 4) | init;
                msg.msg[1] = CEC_MSG_FEATURE_ABORT;
                msg.msg[2] = opcode;  /* FEATURE ABORT */
                msg.msg[3] = CEC_ABORT_NOT_IN_CORECT_MODE;
            }
    }

    _rtk_cec_hw_xmit_message(p_this, &msg);
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_rx
 *
 * Desc : rtd299x cec rx function
 *
 * Parm : work_struct : handle of work structure
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_cec_hw_rx_work(struct work_struct *work)
{
    struct rtk_cec_private *p_this = (struct rtk_cec_private *) container_of(work, struct rtk_cec_private, rcv.work.work);
    rtk_cec_rcv *p_rcv = &p_this->rcv;
    unsigned long flags;
    unsigned long rx_event = read_reg32(RTK_CEC_RXCR0);
    bool bDrop_Data = false;
    int ret;

    spin_lock_irqsave(&p_this->lock, flags);

    if (!p_rcv->enable) {
        if (p_rcv->state == RCV) {
            cec_rx_dbg("force stop\n");
            write_reg32(RTK_CEC_RXCR0, 0);
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

                if (ret < 0) {
                    cec_warn
                    ("read rx fifo failed, return to rx\n");
                    write_reg32(RTK_CEC_RXCR0, 0);
                    _reset_rx_fifo_ofst();
                    p_rcv->state = IDEL;
                }
                write_reg32(RTK_CEC_RXCR0, read_reg32(RTK_CEC_RXCR0)); // clear RX_INT
            }
        } else {
            cec_rx_dbg("rx_stop (0x%08x)\n", read_reg32(RTK_CEC_RXCR0));

            ret = _read_rx_fifo(p_rcv->cmb->tail, cmb_tailroom(p_rcv->cmb), 1);
            cmb_put(p_rcv->cmb, ret);

            if ((rx_event & RX_EOM) && p_rcv->cmb->len) {

                if (rx_event & BROADCAST_ADDR) {
#if PATCH_RX_DROP_SAME_LA_DATA
                    if(!GET_CEC_MSG_STATUS_02(read_reg32(RTK_POWER_SAVING_MODE)))
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
                        ((read_reg32(RTK_CEC_CR0) &LOGICAL_ADDR_MASK) >> LOGICAL_ADDR_SHIFT);
                }

                if (p_this->status.standby_mode) {
                    /* process messag */
                    rtk_cec_hw_standby_message_handler (
                        p_this,
                        p_rcv->cmb->data[0] >> 4,
                        p_rcv->cmb->data[0] & 0xF,
                        p_rcv->cmb->data[1],
                        &p_rcv->cmb->data[2],
                        p_rcv->cmb->len - 2);
                } else if(bDrop_Data == false) {
                    if(p_rcv->cmb->len <= CEC_MAX_MSG_SIZE) {
                        p_this->rx_state = STATE_BUSY;
                        memset(&p_this->rx_msg, 0, sizeof(p_this->rx_msg));
                        p_this->rx_msg.len = p_rcv->cmb->len;
                        memcpy(p_this->rx_msg.msg, p_rcv->cmb->data, p_rcv->cmb->len);
                        p_this->rx_state = STATE_DONE;
                    }
                }

            } 

            p_rcv->state = IDEL;
        }
    }

    if (p_rcv->state == IDEL) {
        if (!p_rcv->enable)
            goto end_proc;

        if (!p_rcv->cmb) {
            cec_warn("[CEC] FATAL, something wrong, no rx buffer left\n");
            goto end_proc;
        }

        cmb_purge(p_rcv->cmb);
        cmb_reserve(p_rcv->cmb, 1);

        write_reg32(RTK_CEC_RXCR0, RX_RST);
        wmb();
        write_reg32(RTK_CEC_RXCR0, 0);
        wmb();
        _reset_rx_fifo_ofst();
        write_reg32(RTK_CEC_RXCR0, RX_EN | RX_INT_EN | RX_CONTINUOUS);
        cec_rx_dbg("rx_restart (0x%08x)\n", read_reg32(RTK_CEC_RXCR0));
        p_rcv->state = RCV;
    }

end_proc:

    spin_unlock_irqrestore(&p_this->lock, flags);
}


/*------------------------------------------------------------------
 * Func : rtk_cec_hw_rx_start
 *
 * Desc : start rx
 *
 * Parm : p_this : handle of rtd299x cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static
void rtk_cec_hw_rx_start(struct rtk_cec_private *p_this)
{
    unsigned long flags;
    spin_lock_irqsave(&p_this->lock, flags);

    if (!p_this->rcv.enable) {
        cec_info("rx start\n");

        p_this->rcv.enable = 1;
        p_this->rcv.state = IDEL;
        _reset_rx_fifo_ofst();

        INIT_DELAYED_WORK(&p_this->rcv.work, rtk_cec_hw_rx_work);

        schedule_delayed_work(&p_this->rcv.work, 0);
    }

    spin_unlock_irqrestore(&p_this->lock, flags);
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_rx_stop
 *
 * Desc : stop rx
 *
 * Parm : p_this : handle of rtd299x cec
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
static
void rtk_cec_hw_rx_stop(struct rtk_cec_private *p_this)
{
    unsigned long flags;

    spin_lock_irqsave(&p_this->lock, flags);

    if (p_this->rcv.enable) {
        cec_info("rx stop\n");

        write_reg32(RTK_CEC_RXCR0, RX_RST);
        wmb();
        write_reg32(RTK_CEC_RXCR0, 0);
        wmb();

        if (p_this->rcv.cmb) {
            cmb_purge(p_this->rcv.cmb);
            cmb_reserve(p_this->rcv.cmb, 1);
        }

        p_this->rcv.enable = 0;
        p_this->rcv.state = IDEL;

    }
    spin_unlock_irqrestore(&p_this->lock, flags);
}


/*------------------------------------------------------------------
 * Func : rtk_cec_hw_isr
 *
 * Desc : isr of rtd299x cec
 *
 * Parm : p_this : handle of rtd299x cec
 *        dev_id : handle of the rtk_cec_hw
 *
 * Retn : IRQ_NONE, IRQ_HANDLED
 *------------------------------------------------------------------*/
irqreturn_t rtk_cec_irq_handler(int this_irq, void *dev_id)
{
    struct rtk_cec_private *p_this = (struct rtk_cec_private *)dev_id;
    irqreturn_t ret = IRQ_NONE;
    if (read_reg32(RTK_CEC_TXCR0) & TX_INT) {
        rtk_cec_hw_tx_work(&p_this->xmit.work.work);
        ret = IRQ_WAKE_THREAD;
    }

    if (read_reg32(RTK_CEC_RXCR0) & RX_INT) {
        rtk_cec_hw_rx_work(&p_this->rcv.work.work);
        ret = IRQ_WAKE_THREAD;
    }

    return ret;
}

static irqreturn_t rtk_cec_irq_handler_thread(int irq, void *priv)
{
    struct rtk_cec_private *p_this = (struct rtk_cec_private *)priv;
    if(g_is_poll_self) {
        cec_info("i %u\n", p_this->tx_state);
	  if(p_this->tx_state == STATE_DONE)
	  	p_this->tx_state = STATE_NACK;
    }
    switch (p_this->tx_state) {
        case STATE_DONE:
            cec_transmit_done(p_this->adap, CEC_TX_STATUS_OK, 0, 0, 0, 0);
            cec_msg_dump(&p_this->tx_msg, "TX DONE");
            p_this->tx_state = STATE_IDLE;
            if(g_is_poll_self)
                cec_info("d %u\n", p_this->tx_state);
            break;
        case STATE_NACK:
            cec_transmit_done(p_this->adap,
                              CEC_TX_STATUS_NACK,
                              0, 1, 0, 0);
            cec_msg_dump(&p_this->tx_msg, "TX NACK");
            p_this->tx_state = STATE_IDLE;
            if(g_is_poll_self)
                cec_info("n %u\n", p_this->tx_state);
            break;
        case STATE_ERROR:
            cec_transmit_done(p_this->adap,
                              CEC_TX_STATUS_ERROR,
                              0, 0, 0, 1);
            cec_msg_dump(&p_this->tx_msg, "TX ERR ");
            p_this->tx_state = STATE_IDLE;
            break;
        case STATE_BUSY:
            dev_err(p_this->dev, "state set to busy, this should not occur here\n");
            break;
        default:
            break;
    }

    switch (p_this->rx_state) {
        case STATE_DONE:
            cec_received_msg(p_this->adap,  &p_this->rx_msg);
            cec_msg_dump(&p_this->rx_msg, "RX DONE");
            p_this->rx_msg.len = 0;
            p_this->rx_state = STATE_IDLE;
            break;
        default:
            break;
    }
    return IRQ_HANDLED;
}


static void  rtk_cec_hw_intial_hw_setting(void)
{
    CRT_CLK_OnOff(CEC, 0, NULL);        // disable clock
    CRT_CLK_OnOff(CEC, 1, NULL);        // enable clock

    write_reg32Mask( STB_SC_LV_RST_reg, ~(CES_INPUT_CLOCK_POLARITY_MASK), CES_INPUT_CLOCK_POLARITY(CEC_POSITIVE_CLOCK));   //MM2KERNDRV-489 , next soc, eneed modify this polarity setting


#ifdef CONFIG_ARCH_RTK6702
    write_reg32(RTK_CEC_CR0, CEC_MODE(0) | TEST_MODE_PAD_EN |
                LOGICAL_ADDR(0xF) | TIMER_DIV(25) | UNREG_ACK_EN);

    write_reg32(RTK_CEC_RTCR0,
                CEC_PAD_EN | CEC_PAD_EN_MODE | PRE_DIV(_get_pre_div()) | RETRY_NO(rtk_cec_retry_num));

#else
    write_reg32(RTK_CEC_CR0, CEC_MODE(0) | TEST_MODE_PAD_EN |
                LOGICAL_ADDR(0xF) | TIMER_DIV(25) |
                PRE_DIV(_get_pre_div()) | UNREG_ACK_EN);

    write_reg32(RTK_CEC_RTCR0,
                CEC_PAD_EN | CEC_PAD_EN_MODE | RETRY_NO(rtk_cec_retry_num));

#endif
    
    /* setup internal pull high register... */
    write_reg32(RTK_POWER_SAVING_MODE, CEC_CLK_POL(CEC_POSITIVE_CLOCK)
            | RES_PULL_UP_EN(0x1) | RES_PULL_UP_SEL(0x6));

    write_reg32(RTK_CEC_RXCR0, RX_RST);

    write_reg32(RTK_CEC_RXCR0, 0);

    write_reg32(RTK_CEC_TXCR0, TX_RST);

    write_reg32(RTK_CEC_TXCR0, 0);

    write_reg32(RTK_CEC_RXTCR0,
                RX_START_LOW(0x60) | RX_START_PERIOD(0xC8) |
                RX_DATA_SAMPLE(42) | RX_DATA_PERIOD(PATCH_RX_DATA_PERIOD));

    write_reg32(RTK_CEC_RXTCR1,
                CEC_RxTCR1_rx_data_period_chk(140) | CEC_RxTCR1_rx_start_high(0xA0) );

    write_reg32(RTK_CEC_TXTCR0, TX_START_LOW(149) | TX_START_HIGH(32));

    write_reg32(RTK_CEC_TXTCR1,
                TX_DATA_LOW(25) | TX_DATA_01(36) |
                TX_DATA_HIGH(36));
    write_reg32(RTK_CEC_RxACKTCR1,
                RX_ACK_LOW_SEL | RX_ACK_LOW(20) | RX_ACK_BIT(30) |
                RX_ACK_LINE_ERR(94));

#ifdef CEC_OPCODE_COMPARE_ENABLE
    write_reg32(RTK_CEC_OPCODE_ENABLE, 0);  /* disable opcode compare function */
#endif

}


void rtk_cec_dump_current_setting(char *buf, int in_standby)
{
    if(g_cec_debug_enable ==0)
        return; //dump setting on all mode
    if(buf == NULL) {
        cec_info("****************The current setting of cec*****************\n");
        cec_info("CEC_CR0(0x%08x) = 0x%08x\n", RTK_CEC_CR0, read_reg32(RTK_CEC_CR0));
        cec_info("CEC_RTCR0(0x%08x) = 0x%08x\n", RTK_CEC_RTCR0, read_reg32(RTK_CEC_RTCR0));
        cec_info("CEC_RXCR0(0x%08x) = 0x%08x\n", RTK_CEC_RXCR0, read_reg32(RTK_CEC_RXCR0));
        cec_info("CEC_TXCR0(0x%08x) = 0x%08x\n", RTK_CEC_TXCR0, read_reg32(RTK_CEC_TXCR0));
        cec_info("CEC_RXTCR0(0x%08x) = 0x%08x\n", RTK_CEC_RXTCR0, read_reg32(RTK_CEC_RXTCR0));
        cec_info("CEC_TXTCR0(0x%08x) = 0x%08x\n", RTK_CEC_TXTCR0, read_reg32(RTK_CEC_TXTCR0));
        cec_info("CEC_TXTCR1(0x%08x) = 0x%08x\n", RTK_CEC_TXTCR1, read_reg32(RTK_CEC_TXTCR1));
        cec_info("GDI_POWER_SAVING_MODE(0x%08x) = 0x%08x\n",
                 RTK_POWER_SAVING_MODE, read_reg32(RTK_POWER_SAVING_MODE));
        cec_info("CEC_RxACKTCR0(0x%08x) = 0x%08x\n",
                 RTK_CEC_RxACKTCR0, read_reg32(RTK_CEC_RxACKTCR0));
        cec_info("CEC_RxACKTCR1(0x%08x) = 0x%08x\n",
                 RTK_CEC_RxACKTCR1, read_reg32(RTK_CEC_RxACKTCR1));
        cec_info("CEC_TxRxACKOPTION(0x%08x) = 0x%08x\n",
                 RTK_CEC_TxRxACKOPTION, read_reg32(RTK_CEC_TxRxACKOPTION));
        cec_info("CEC_TxRxACKOPTION(0x%08x) = 0x%08x\n",
                 RTK_CEC_TxRxACKOPTION, read_reg32(RTK_CEC_TxRxACKOPTION));
        cec_info("STB_ST_BUSCLK_reg(0x%08x) = 0x%08x\n",
                 STB_ST_BUSCLK_reg, read_reg32(STB_ST_BUSCLK_reg));
        cec_info("The current bus clk freq is %s Mhz\n",
                 (read_reg32(STB_ST_BUSCLK_reg) & (1 <<
                         STB_ST_BUSCLK_bus_clksel_shift)) ? "27" : "250");
    } else {
        buf[0] = 0;
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "****************The current setting of cec*****************\n");
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CEC_CR0(0x%08x) = 0x%08x\n", RTK_CEC_CR0, read_reg32(RTK_CEC_CR0));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CEC_RTCR0(0x%08x) = 0x%08x\n", RTK_CEC_RTCR0, read_reg32(RTK_CEC_RTCR0));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CEC_RXCR0(0x%08x) = 0x%08x\n", RTK_CEC_RXCR0, read_reg32(RTK_CEC_RXCR0));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CEC_TXCR0(0x%08x) = 0x%08x\n", RTK_CEC_TXCR0, read_reg32(RTK_CEC_TXCR0));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CEC_RXTCR0(0x%08x) = 0x%08x\n", RTK_CEC_RXTCR0, read_reg32(RTK_CEC_RXTCR0));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CEC_TXTCR0(0x%08x) = 0x%08x\n", RTK_CEC_TXTCR0, read_reg32(RTK_CEC_TXTCR0));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CEC_TXTCR1(0x%08x) = 0x%08x\n", RTK_CEC_TXTCR1, read_reg32(RTK_CEC_TXTCR1));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "GDI_POWER_SAVING_MODE(0x%08x) = 0x%08x\n",
                RTK_POWER_SAVING_MODE, read_reg32(RTK_POWER_SAVING_MODE));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CEC_RxACKTCR0(0x%08x) = 0x%08x\n",
                RTK_CEC_RxACKTCR0, read_reg32(RTK_CEC_RxACKTCR0));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CEC_RxACKTCR1(0x%08x) = 0x%08x\n",
                RTK_CEC_RxACKTCR1, read_reg32(RTK_CEC_RxACKTCR1));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CEC_TxRxACKOPTION(0x%08x) = 0x%08x\n",
                RTK_CEC_TxRxACKOPTION, read_reg32(RTK_CEC_TxRxACKOPTION));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CEC_TxRxACKOPTION(0x%08x) = 0x%08x\n",
                RTK_CEC_TxRxACKOPTION, read_reg32(RTK_CEC_TxRxACKOPTION));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "STB_ST_BUSCLK_reg(0x%08x) = 0x%08x\n",
                STB_ST_BUSCLK_reg, read_reg32(STB_ST_BUSCLK_reg));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "The current bus clk freq is %s Mhz\n",
                (read_reg32(STB_ST_BUSCLK_reg) & (1 <<
                        STB_ST_BUSCLK_bus_clksel_shift)) ? "27" : "250");
        cec_info("%s", buf);
    }
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_init
 *
 * Desc : init a rtd299x cec adapter
 *
 * Parm : N/A
 *
 * Retn : handle of cec module
 *------------------------------------------------------------------*/
int rtk_cec_hw_init(struct rtk_cec_private *p_this, unsigned int irq)
{
    int ret = 0;
    if (!p_this->status.init) {
        m_cec = p_this;
        cec_info("Open %s CEC\n", CEC_MODEL_NAME);

        rtk_cec_hw_intial_hw_setting();
        spin_lock_init(&p_this->lock);

        p_this->xmit.state = IDEL;
        p_this->xmit.cmb =  alloc_cmb(RX_BUFFER_SIZE);
        if(!p_this->xmit.cmb)
            return -ENOMEM;

        p_this->xmit.timeout = 0;

        p_this->rcv.state = IDEL;
        p_this->rcv.cmb =  alloc_cmb(RX_BUFFER_SIZE);
        if(!p_this->rcv.cmb) {
            kfree_cmb(p_this->xmit.cmb);
            p_this->xmit.cmb = NULL;
            return -ENOMEM;
        }

        ret = devm_request_threaded_irq(p_this->dev, irq,
                                        rtk_cec_irq_handler,
                                        rtk_cec_irq_handler_thread,
                                        IRQF_SHARED,
                                        CEC_NAME, p_this);
        if (ret) {
            cec_info("cec registe irq %dfailed\n", irq);
            kfree_cmb(p_this->xmit.cmb);
            p_this->xmit.cmb = NULL;
            kfree_cmb(p_this->rcv.cmb);
            p_this->rcv.cmb = NULL;
            return ret;
        }

        cec_info("cec : enable CEC SCPU interrupt\n");
        write_reg32(CEC_INT_EN, CEC_INT_EN_VAL | CEC_INT_EN_WRITE_DATA);
        p_this->status.init = 1;
    }

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_set_mode
 *
 * Desc : enable ces module
 *
 * Parm : p_this   : handle of rtd299x cec adapter
 *        mode     : CEC_MODE_OFF : disable CEC module
 *                   CEC_MODE_ON : enable CEC module
 *                   CEC_MODE_OFF : put CEC module into standby mode
 *
 * Retn : 0 for success, others failed
 *------------------------------------------------------------------*/
int rtk_cec_hw_set_mode(struct rtk_cec_private *p_this, unsigned char mode)
{
    switch (mode) {
        case CEC_MODE_OFF:  /* disable */

            cec_info("%s cec mode : OFF\n", CEC_MODEL_NAME);
            write_reg32(RTK_CEC_CR0,
                        (read_reg32(RTK_CEC_CR0) & ~CTRL_MASK1) | CEC_MODE(0) |
                        LOGICAL_ADDR(0xF));
            p_this->status.enable = 0;
            p_this->status.standby_mode = 0;
            rtk_cec_hw_rx_stop(p_this);
            rtk_cec_hw_tx_stop(p_this);

            break;

        case CEC_MODE_ON:

            cec_info("%s cec mode : On\n", CEC_MODEL_NAME);
            write_reg32(RTK_CEC_CR0,
                        (read_reg32(RTK_CEC_CR0) & ~CTRL_MASK1) | CEC_MODE(1));
#ifdef CEC_OPCODE_COMPARE_ENABLE
            write_reg32(RTK_CEC_OPCODE_ENABLE, 0);  /*disable opcode compare function*/
#endif
            rtk_cec_hw_rx_start(p_this);
            rtk_cec_hw_tx_start(p_this);
            p_this->status.enable = 1;
            p_this->status.standby_mode = 0;

            break;

        case CEC_MODE_STANDBY:

            cec_info("%s cec mode : Standby (la=%x)\n", CEC_MODEL_NAME,
                     rtk_cec_standby_logical_addr);
            write_reg32(RTK_CEC_CR0,
                        (read_reg32(RTK_CEC_CR0) & ~CTRL_MASK1) | CEC_MODE(1) |
                        LOGICAL_ADDR(rtk_cec_standby_logical_addr));
            rtk_cec_hw_rx_start(p_this);
            rtk_cec_hw_tx_start(p_this);
            p_this->status.enable = 1;
            p_this->status.standby_mode = 1;
            break;
    }
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_enable
 *
 * Desc : enable ces module
 *
 * Parm : p_this   : handle of rtd299x cec adapter
 *        on_off   : 0 : disable, others enable
 *
 * Retn : 0 for success, others failed
 *------------------------------------------------------------------*/
int rtk_cec_hw_enable(struct rtk_cec_private *p_this, bool enable)
{
    if (enable)
        rtk_cec_hw_set_mode(p_this, CEC_MODE_ON);
    else
        rtk_cec_hw_set_mode(p_this, CEC_MODE_OFF);
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_set_rx_mask
 *
 * Desc : set rx mask rtd299x ces
 *
 * Parm : p_this    : handle of rtd299x cec adapter
 *        rx_mask   : rx mask
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtk_cec_hw_set_rx_mask(struct rtk_cec_private *p_this, unsigned short rx_mask)
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

    write_reg32(RTK_CEC_CR0,
                (read_reg32(RTK_CEC_CR0) & ~LOGICAL_ADDR_MASK) |
                LOGICAL_ADDR(log_addr));

    cec_info("cec : logical address = %02x\n", log_addr);

    if (log_addr != 0xF)
        rtk_cec_standby_logical_addr = log_addr;    /* save the latest vaild logical address */

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_set_physical_addr
 *
 * Desc : set physical address of rtd299x ces
 *
 * Parm : p_this    : handle of rtd299x cec adapter
 *        phy_addr  : physical address
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtk_cec_hw_set_physical_addr(struct rtk_cec_private *p_this, unsigned short phy_addr)
{
    cec_info("cec : physcial address = %04x\n", phy_addr);

    /* we always keep latest valid physical address during standby */
    if (phy_addr != 0xffff)
        rtk_cec_standby_physical_addr = phy_addr;


    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_set_cec_version
 *
 * Desc : set cec version
 *
 * Parm : p_this    : handle of rtd299x cec adapter
 *        version   : cec version number
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtk_cec_hw_set_cec_version(struct rtk_cec_private *p_this, unsigned char version)
{
    rtk_cec_standby_cec_version = version;
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_set_device_vendor_id
 *
 * Desc : set vendor id of rtd299x ces
 *
 * Parm : p_this    : handle of rtd299x cec adapter
 *        vendor_id  : device vendor id
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtk_cec_hw_set_device_vendor_id(struct rtk_cec_private *p_this,
                                    unsigned long vendor_id)
{
    rtk_cec_standby_vendor_id = vendor_id;
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_set_stanby_mode
 *
 * Desc : set standy mode condition of rtd299x ces
 *
 * Parm : p_this  : handle of rtd299x cec adapter
 *        mode    : cec standby mode
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtk_cec_hw_set_stanby_mode(struct rtk_cec_private *p_this, unsigned long mode)
{
    rtk_cec_standby_config = mode;
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_set_retry_num
 *
 * Desc : set retry times of rtd299x cec
 *
 * Parm : p_this  : handle of rtd299x cec adapter
 *        mun    : retry times
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtk_cec_hw_set_retry_num(struct rtk_cec_private *p_this, unsigned long num)
{
    rtk_cec_retry_num = num;
    write_reg32(RTK_CEC_RTCR0,
                ((read_reg32(RTK_CEC_RTCR0) & ~RETRY_NO_MASK) | RETRY_NO(rtk_cec_retry_num)));
    return 0;
}

#define CBUS_PMM_CBUS_PHY_3_reg 0xb8062b3c

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_set_hotplug
 *
 * Desc : manual set hotplug of rtd299x cec
 *
 * Parm : p_this  : handle of rtd299x cec adapter
 *        onoff   : hotplug onoff
 *
 * Retn : 0 : success, others fail
 *------------------------------------------------------------------*/
int rtk_cec_hw_set_hotplug(struct rtk_cec_private *p_this, unsigned long onoff)
{
    cec_info("rtk_cec_hw_set_hotplug, onoff=%lu\n", onoff);
    if (onoff) {
        write_reg32(CBUS_PMM_CBUS_PHY_3_reg, ((read_reg32(CBUS_PMM_CBUS_PHY_3_reg) & 0xffffbffe) | 0x4000));
    } else {
        write_reg32(CBUS_PMM_CBUS_PHY_3_reg, ((read_reg32(CBUS_PMM_CBUS_PHY_3_reg) & 0xffffbffe) | 0x4001));
    }
    return 0;
}



/*------------------------------------------------------------------
 * Func : rtk_cec_hw_uninit
 *
 * Desc : uninit a rtd299x cec adapter
 *
 * Parm : N/A
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void rtk_cec_hw_uninit(struct rtk_cec_private *p_this, unsigned int irq)
{
    cec_info("%s cec closed\n", CEC_MODEL_NAME);
    m_cec = NULL;
    write_reg32(CEC_INT_EN, CEC_INT_EN_VAL);
    free_irq(irq, p_this);
    write_reg32(RTK_CEC_CR0, read_reg32(RTK_CEC_CR0) & ~CEC_MODE_MASK);
    write_reg32(RTK_CEC_RXCR0, 0);
    write_reg32(RTK_CEC_TXCR0, 0);
    rtk_cec_hw_enable(p_this, 0);
    if(p_this->xmit.cmb) {
        kfree_cmb(p_this->xmit.cmb);
        p_this->xmit.cmb = NULL;
    }
    if(p_this->rcv.cmb) {
        kfree_cmb(p_this->rcv.cmb);
        p_this->rcv.cmb = NULL;
    }

    p_this->status.init = 0;
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_suspend
 *
 * Desc : suspend a rtd299x cec adapter
 *
 * Parm : p_this : handle of rtd299x cec adapter
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_cec_hw_suspend(struct rtk_cec_private *p_this)
{
    p_this->status.state_before_suspend = p_this->status.enable;
    write_reg32(CEC_INT_EN, CEC_INT_EN_VAL);
    if (!p_this->status.enable) { /*CEC is off*/
        cec_warn("[CEC]function is off when power on, doesn't enable!!\n");
        return 0;
    }

    cec_info("%s cec suspended (la=%x, pa=%04x, standby_mode=%ld)\n",
             CEC_MODEL_NAME, rtk_cec_standby_logical_addr,
             rtk_cec_standby_physical_addr, rtk_cec_standby_config);

    rtk_cec_hw_set_mode(p_this, CEC_MODE_OFF);

    if (rtk_cec_standby_config && rtk_cec_standby_logical_addr < 0xF
        && rtk_cec_standby_physical_addr != 0xFFFF) {
        p_this->status.enable = 1;
        p_this->status.standby_mode = 1;

#ifdef CONFIG_ARCH_RTK6702
        write_reg32(RTK_CEC_CR0,
                    (read_reg32(RTK_CEC_CR0) & ~(LOGICAL_ADDR(0xF) |
                    TIMER_DIV(0xFF) )) | CEC_MODE(1) | TIMER_DIV(0x14) |
                    LOGICAL_ADDR(rtk_cec_standby_logical_addr));

        write_reg32(RTK_CEC_RTCR0,(read_reg32(RTK_CEC_RTCR0) & ~(PRE_DIV(0x3FF))) | PRE_DIV(0x21));

#else
        write_reg32(RTK_CEC_CR0,
                    (read_reg32(RTK_CEC_CR0) & ~(LOGICAL_ADDR(0xF) |
                            TIMER_DIV(0xFF) | PRE_DIV(0xFF))) |
                    CEC_MODE(1) | TIMER_DIV(0x14) |
                    PRE_DIV(0x21) | LOGICAL_ADDR(rtk_cec_standby_logical_addr));

        write_reg32(RTK_CEC_RXCR0, RX_RST);

#endif

        write_reg32(RTK_CEC_RXCR0, 0);

        write_reg32(RTK_CEC_TXCR0, TX_RST);

        write_reg32(RTK_CEC_TXCR0, 0);

#ifdef CEC_OPCODE_COMPARE_ENABLE
        write_reg32(RTK_CEC_OPCODE_ENABLE, 0);  /*disable */

        /*-- compare 1 */
        if (rtk_cec_standby_config &
            STANBY_RESPONSE_GIVE_POWER_STATUS) {
            write_reg32(RTK_CEC_COMPARE_OPCODE_01,
                        CEC_MSG_GIVE_DEVICE_POWER_STATUS);
            write_reg32(RTK_CEC_SEND_OPCODE_01, CEC_MSG_REPORT_POWER_STATUS);
            write_reg32(RTK_CEC_SEND_OPERAND_NUMBER_01, 1);
            write_reg32(RTK_CEC_OPERAND_01, CEC_POWER_STATUS_STANDBY);
            write_reg32(RTK_CEC_OPCODE_ENABLE,
                        (read_reg32(RTK_CEC_OPCODE_ENABLE) | 0x01));
        }
        /*-- compare 2 */
        if (rtk_cec_standby_config &
            STANBY_RESPONSE_GIVE_PHYSICAL_ADDR) {
            write_reg32(RTK_CEC_COMPARE_OPCODE_02, CEC_MSG_GIVE_PHYSICAL_ADDRESS);
            write_reg32(RTK_CEC_SEND_OPCODE_02,   CEC_MSG_REPORT_PHYSICAL_ADDRESS);
            write_reg32(RTK_CEC_SEND_OPERAND_NUMBER_02, 2);
            write_reg32(RTK_CEC_OPERAND_04,
                        (rtk_cec_standby_physical_addr >> 8) &
                        0xFF);
            write_reg32(RTK_CEC_OPERAND_05,
                        (rtk_cec_standby_physical_addr) & 0xFF);
            /*write_reg32(0xb8061ecc, (read_reg32(0xb8061ecc)|0x02)); //invalid, disable temporarily*/
        }
        /*-- compare 3*/
        if (rtk_cec_standby_config &
            STANBY_RESPONSE_GET_CEC_VERISON) {
            write_reg32(RTK_CEC_COMPARE_OPCODE_03, CEC_MSG_GET_CEC_VERSION);
            write_reg32(RTK_CEC_SEND_OPCODE_03, CEC_MSG_CEC_VERSION);
            write_reg32(RTK_CEC_SEND_OPERAND_NUMBER_03, 1);
            write_reg32(RTK_CEC_OPERAND_07,
                        rtk_cec_standby_cec_version);
            write_reg32(RTK_CEC_OPCODE_ENABLE,
                        (read_reg32(RTK_CEC_OPCODE_ENABLE) | 0x04));
        }
        /*-- compare 4*/
        if (rtk_cec_standby_config &
            STANBY_RESPONSE_GIVE_DEVICE_VENDOR_ID) {
            write_reg32(RTK_CEC_COMPARE_OPCODE_04, CEC_MSG_GIVE_DEVICE_VENDOR_ID);
            write_reg32(RTK_CEC_SEND_OPCODE_04, CEC_MSG_DEVICE_VENDOR_ID);
            write_reg32(RTK_CEC_SEND_OPERAND_NUMBER_04, 3);
            write_reg32(RTK_CEC_OPERAND_10,
                        (rtk_cec_standby_vendor_id >> 16) &
                        0xFF);
            write_reg32(RTK_CEC_OPERAND_11,
                        (rtk_cec_standby_vendor_id >> 8) &
                        0xFF);
            write_reg32(RTK_CEC_OPERAND_12,
                        (rtk_cec_standby_vendor_id) & 0xFF);
            write_reg32(RTK_CEC_OPCODE_ENABLE,
                        (read_reg32(RTK_CEC_OPCODE_ENABLE) | 0x08));
        }
        cec_info("CEC_COMP_ENABLE = %08x\n", read_reg32(RTK_CEC_OPCODE_ENABLE));
#endif /*ifdef CEC_OPCODE_COMPARE_ENABLE*/
        write_reg32(RTK_CEC_RXCR0, RX_EN | RX_CONTINUOUS);  /* enable RX */

        cec_info("cec standby enabled\n");
    }

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_cec_hw_resume
 *
 * Desc : suspend a rtd299x cec adapter
 *
 * Parm : p_this : handle of rtd299x cec adapter
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_cec_hw_resume(struct rtk_cec_private *p_this)
{
    cec_warn("%s cec resume\n", CEC_MODEL_NAME);

    p_this->status.standby_mode = 0;

    rtk_cec_hw_intial_hw_setting();
    write_reg32(CEC_INT_EN, CEC_INT_EN_VAL | CEC_INT_EN_WRITE_DATA);

    if(p_this->status.state_before_suspend == 1) {
        p_this->status.state_before_suspend = 0;
        rtk_cec_hw_set_mode(p_this, CEC_MODE_ON);
    } else {
        rtk_cec_hw_set_mode(p_this, CEC_MODE_OFF);
    }


    return 0;
}

/*------------------------------------------------------------------
 * Func : pm_rtk_cec_hw_suspend
 *
 * Desc : Force rtd299x cec to enter suspend state.
 *        this function will be called before enter 8051.
 *
 * Parm : None
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int pm_rtd_cec_suspend(void)
{
    if(!m_cec)
        return -1;
    return rtk_cec_hw_suspend(m_cec);
}

