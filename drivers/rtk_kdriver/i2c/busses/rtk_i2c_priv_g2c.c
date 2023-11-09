#include "rtk_i2c_priv.h"

extern unsigned long rtk_i2c_flags;
extern rtk_i2c *rtk_i2c_phy_handle;

#define rtk_g2c_init                    rtk_g2c_dump
#define rtk_g2c_uninit                  rtk_g2c_null_function
#define rtk_g2c_get_tx_abort_reason     rtk_i2c_get_tx_abort_reason
#define rtk_g2c_suspend                 rtk_g2c_null_function
#define rtk_g2c_resume                  rtk_g2c_null_function

#define currentRxDataOnTransmit         (p_this->xfer.rx_buff[p_this->xfer.rx_len])
#define currentRxindex                  (p_this->xfer.rx_len)
#define totalRxDataLen                  (p_this->xfer.rx_buff_len)
#define currentTxDataOnTransmit         (p_this->xfer.tx_buff[p_this->xfer.tx_len])
#define currentTxindex                  (p_this->xfer.tx_len)
#define totalTxDataLen                  (p_this->xfer.tx_buff_len)

#define G2C_START_PULSE_INIT            0
#define G2C_START_PULSE_CHECK           1
#define G2C_START_PULSE_TRIG            2
#define G2C_START_PULSE_END             3

#define G2C_ADDRESS_ACK_INIT            0
#define G2C_ADDRESS_ACK_SDA_FREE        1
#define G2C_ADDRESS_ACK_SCL_FREE        2
#define G2C_ADDRESS_ACK_CHECK           3

#define G2C_ADDRESS_INIT                0
#define G2C_ADDRESS_OUT                 1
#define G2C_ADDRESS_CHECK               2
#define G2C_ADDRESS_END                 3

#define G2C_READ_BYTE_SCL_LOW           0
#define G2C_READ_BYTE_SDA_FREE          1
#define G2C_READ_BYTE_SCL_HIGH          2
#define G2C_READ_BYTE_SDA_READ          3

#define G2C_READ_ACK_SCL_LOW            0
#define G2C_READ_ACK_SDA_LOW_or_FREE    1
#define G2C_READ_ACK_SCL_HIGH           2
#define G2C_READ_ACK_CHECK              3

#define G2C_WRITE_BYTE_SCL_LOW          0
#define G2C_WRITE_BYTE_SDA_OUT          1
#define G2C_WRITE_BYTE_SCL_HIGH         2
#define G2C_WRITE_BYTE_CHECK            3

#define G2C_WRITE_ACK_SCL_LOW           0
#define G2C_WRITE_ACK_SDA_LOW_or_FREE   1
#define G2C_WRITE_ACK_SCL_HIGH          2
#define G2C_WRITE_ACK_CHECK             3

#define G2C_STOP_SCL_LOW                0
#define G2C_STOP_SDA_LOW_or_FREE        1
#define G2C_STOP_SCL_HIGH               2
#define G2C_STOP_CHECK                  3

/*------------------------------------------------------------------
 * Func : rtk_g2c_do_start
 *
 * Desc : gpio i2c xfer - start phase
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
int rtk_g2c_do_start(rtk_i2c * p_this)
{
    RTK_GPIO_ID sda;
    RTK_GPIO_ID scl;
    int ret = 0;

    if (rtk_i2c_get_mapped_gpio(p_this, &sda, &scl) < 0)
        return -1;

    switch (p_this->xfer.gpio_xfer_sub_state)
    {
        case G2C_START_PULSE_INIT:
            rtk_gpio_set_irq_enable(sda, 0);
            rtk_gpio_set_irq_enable(scl, 0);
            rtk_gpio_set_dir(sda, 0);
            rtk_gpio_set_dir(scl, 0);
            rtk_gpio_output(sda, 0);
            rtk_gpio_output(scl, 0);
            rtk_i2c_gpio_selection(p_this, GPIO_MODE);
            p_this->xfer.gpio_xfer_sub_state++;
            break;

        case G2C_START_PULSE_CHECK:
            if (rtk_gpio_input(scl) && rtk_gpio_input(sda))
            {
                p_this->xfer.gpio_xfer_sub_state++;
            }
            break;

        case G2C_START_PULSE_TRIG:
            rtk_gpio_set_dir(sda, 1);
            p_this->xfer.gpio_xfer_sub_state++;
            break;

        case G2C_START_PULSE_END:
            rtk_gpio_set_dir(scl, 1);
            p_this->xfer.gpio_xfer_state = G2C_ST_ADDR0;
            p_this->xfer.gpio_xfer_sub_state = 0;
            break;
    }
    return ret;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_do_address
 *
 * Desc : gpio i2c xfer - address phase
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
int rtk_g2c_do_address(rtk_i2c * p_this)
{
    RTK_GPIO_ID sda;
    RTK_GPIO_ID scl;
    unsigned char bitPosition = G2C_MINOR_STATE(p_this->xfer.gpio_xfer_state);

    int bit_index = 0;
    int ret = 0;
    unsigned char addr;

    if (rtk_i2c_get_mapped_gpio(p_this, &sda, &scl) < 0)
        return -1;

    if (bitPosition <= 7)
    {
        switch (p_this->xfer.gpio_xfer_sub_state)
        {
            case G2C_ADDRESS_INIT:
                rtk_gpio_set_dir(scl, 1);
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_ADDRESS_OUT:
                addr = p_this->tar << 1;
                if (p_this->xfer.mode == I2C_MASTER_READ)
                {
                    addr |= 1;
                }
                bit_index = 7 - bitPosition;
                if ((addr >> bit_index) & 0x1)
                {
                    rtk_gpio_set_dir(sda, 0);
                }
                else
                {
                    rtk_gpio_set_dir(sda, 1);
                }
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_ADDRESS_CHECK:
                addr = p_this->tar << 1;
                if (p_this->xfer.mode == I2C_MASTER_READ)
                {
                    addr |= 1;
                }
                bit_index = 7 - bitPosition;
                if (((addr >> bit_index) & 0x1) && rtk_gpio_input(sda) == 0)
                {
                    p_this->xfer.ret = -ETXABORT;
                    p_this->xfer.gpio_xfer_state = G2C_ST_DONE;
                    p_this->xfer.gpio_xfer_sub_state = 0;
                    I2C_WARNING("[ch_%d] Xfer fail - TXABORT, %s Line%d\n", p_this->id, __func__ , __LINE__ );
                    ret = -1;
                }
                else
                {
                    rtk_gpio_set_dir(scl, 0);
                    p_this->xfer.gpio_xfer_sub_state++;
                }
                break;

            case G2C_ADDRESS_END:
                if (rtk_gpio_input(scl))
                {
                    p_this->xfer.gpio_xfer_state++;
                    p_this->xfer.gpio_xfer_sub_state = 0;
                }
                break;
        }
    }
    else if (bitPosition == 8)
    {
        switch (p_this->xfer.gpio_xfer_sub_state)
        {
            case G2C_ADDRESS_ACK_INIT:
                rtk_gpio_set_dir(scl, 1);
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_ADDRESS_ACK_SDA_FREE:
                rtk_gpio_set_dir(sda, 0);
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_ADDRESS_ACK_SCL_FREE:
                rtk_gpio_set_dir(scl, 0);
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_ADDRESS_ACK_CHECK:
                if (rtk_gpio_input(scl))
                {
                    if (rtk_gpio_input(sda))
                    {
                        p_this->xfer.ret = -ETXABORT;
                        p_this->xfer.gpio_xfer_state = G2C_ST_STOP;

                        I2C_WARNING("[ch_%d] Xfer  ACK fail - ETXABORT, %s Line%d\n", p_this->id, __func__ , __LINE__ );
                        ret = -1;
                    }
                    else
                    {
                        p_this->xfer.gpio_xfer_state = G2C_ST_DATA0;
                    }
                    p_this->xfer.gpio_xfer_sub_state = 0;
                }
                else
                {
                    I2C_WARNING("[ch_%d][g2c] Xfer fail - do-address check ack pulse failed,%s Line%d\n",
                                p_this->id, __func__ , __LINE__ );
                    ret = -1;
                }
                break;
        }
    }
    return ret;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_do_read
 *
 * Desc : gpio i2c xfer - read data phase
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
int rtk_g2c_do_read(rtk_i2c * p_this)
{
    RTK_GPIO_ID sda;
    RTK_GPIO_ID scl;
    unsigned char bitPosition = G2C_MINOR_STATE(p_this->xfer.gpio_xfer_state);
    int bit_index = 0;
    int ret = 0;

    if (rtk_i2c_get_mapped_gpio(p_this, &sda, &scl) < 0)
        return -1;

    if (bitPosition < 8)
    {
        switch (p_this->xfer.gpio_xfer_sub_state)
        {
            case G2C_READ_BYTE_SCL_LOW:
                rtk_gpio_set_dir(scl, 1);
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_READ_BYTE_SDA_FREE:
                rtk_gpio_set_dir(sda, 0);
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_READ_BYTE_SCL_HIGH:
                rtk_gpio_set_dir(scl, 0);
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_READ_BYTE_SDA_READ:
                if (rtk_gpio_input(scl))
                {
                    if (rtk_gpio_input(sda))
                    {
                        bit_index = 7 - bitPosition;
                        p_this->xfer.rx_buff[p_this->xfer.rx_len] |= (1 << bit_index);
                    }
                    p_this->xfer.gpio_xfer_state++;
                    p_this->xfer.gpio_xfer_sub_state = 0;
                }
                break;
        }
    }
    else
    {
        switch (p_this->xfer.gpio_xfer_sub_state)
        {
            case G2C_READ_ACK_SCL_LOW:
                rtk_gpio_set_dir(scl, 1);
                p_this->xfer.gpio_xfer_sub_state++;
                p_this->xfer.rx_len++;
                break;

            case G2C_READ_ACK_SDA_LOW_or_FREE:
                if (p_this->xfer.rx_len < p_this->xfer.rx_buff_len)
                {
                    rtk_gpio_set_dir(sda, 1);
                }
                else
                {
                    rtk_gpio_set_dir(sda, 0);
                }
                p_this->xfer.gpio_xfer_sub_state++;
                break;
            case G2C_READ_ACK_SCL_HIGH:
                rtk_gpio_set_dir(scl, 0);
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_READ_ACK_CHECK:
                if (rtk_gpio_input(scl))
                {
                    if (p_this->xfer.rx_len < p_this->xfer.rx_buff_len)
                        p_this->xfer.gpio_xfer_state = G2C_ST_DATA0;
                    else
                        p_this->xfer.gpio_xfer_state = G2C_ST_STOP;

                    p_this->xfer.gpio_xfer_sub_state = 0;
                }
                break;
        }
    }
    return ret;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_do_write
 *
 * Desc : gpio i2c xfer - write data phase
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
int rtk_g2c_do_write(rtk_i2c * p_this)
{
    RTK_GPIO_ID sda;
    RTK_GPIO_ID scl;
    unsigned char bitPosition = G2C_MINOR_STATE(p_this->xfer.gpio_xfer_state);
    int bit_index = 0;
    int ret = 0;

    if (rtk_i2c_get_mapped_gpio(p_this, &sda, &scl) < 0)
        return -1;

    if (bitPosition < 8)
    {
        switch (p_this->xfer.gpio_xfer_sub_state)
        {
            case G2C_WRITE_BYTE_SCL_LOW:
                rtk_gpio_set_dir(scl, 1);
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_WRITE_BYTE_SDA_OUT:
                bit_index = 7 - bitPosition;

                if ((p_this->xfer.tx_buff[p_this->xfer.tx_len] >> bit_index) & 0x1)
                {
                    rtk_gpio_set_dir(sda, 0);
                }
                else
                {
                    rtk_gpio_set_dir(sda, 1);
                }
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_WRITE_BYTE_SCL_HIGH:
                bit_index = 7 - bitPosition;
                if (((p_this->xfer.tx_buff[p_this->xfer.tx_len] >> bit_index) & 0x1)
                        && rtk_gpio_input(sda) == 0)
                {
                    p_this->xfer.ret = -ETXABORT;
                    p_this->xfer.gpio_xfer_state = G2C_ST_DONE;
                    p_this->xfer.gpio_xfer_sub_state = 0;
                    I2C_WARNING("[ch_%d] Xfer fail - TXABORT, %s Line%d\n",p_this->id, __func__, __LINE__ );
                    ret = -1;
                }
                else
                {
                    rtk_gpio_set_dir(scl, 0);
                    p_this->xfer.gpio_xfer_sub_state++;
                }
                break;

            case G2C_WRITE_BYTE_CHECK:
                if (rtk_gpio_input(scl))
                {
                    p_this->xfer.gpio_xfer_state++;
                    p_this->xfer.gpio_xfer_sub_state = 0;
                }
                break;
        }
    }
    else
    {
        switch (p_this->xfer.gpio_xfer_sub_state)
        {
            case G2C_WRITE_ACK_SCL_LOW:
                rtk_gpio_set_dir(scl, 1);
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_WRITE_ACK_SDA_LOW_or_FREE:
                rtk_gpio_set_dir(sda, 0);
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_WRITE_ACK_SCL_HIGH:
                rtk_gpio_set_dir(scl, 0);
                p_this->xfer.gpio_xfer_sub_state++;
                break;

            case G2C_WRITE_ACK_CHECK:
                if (rtk_gpio_input(scl))
                {
                    p_this->xfer.tx_len++;
                    if (rtk_gpio_input(sda) || (p_this->xfer.tx_len >= p_this->xfer.tx_buff_len))
                    {
                        if (rtk_gpio_input(sda))
                        {
                            p_this->xfer.ret = -ETXABORT;
                            I2C_WARNING("[ch_%d] Xfer fail - TXABORT, %s Line%d\n", p_this->id, __func__, __LINE__ );
                        }
                        p_this->xfer.gpio_xfer_state = G2C_ST_STOP;
                    }
                    else
                    {
                        p_this->xfer.gpio_xfer_state = G2C_ST_DATA0;
                    }
                    p_this->xfer.gpio_xfer_sub_state = 0;
                }
                break;
        }
    }
    return ret;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_do_stop
 *
 * Desc : Do STOP or Restart
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
int rtk_g2c_do_stop(rtk_i2c * p_this)
{
    RTK_GPIO_ID sda;
    RTK_GPIO_ID scl;
    int ret = 0;

    if (rtk_i2c_get_mapped_gpio(p_this, &sda, &scl) < 0)
        return -1;

    switch (p_this->xfer.gpio_xfer_sub_state)
    {
        case G2C_STOP_SCL_LOW:
            rtk_gpio_set_dir(scl, 1);
            p_this->xfer.gpio_xfer_sub_state++;
            break;

        case G2C_STOP_SDA_LOW_or_FREE:
            if ( ( (p_this->xfer.flags & I2C_NO_STOP) == 0 ) || p_this->xfer.ret < 0)
            {
                rtk_gpio_set_dir(sda, 1);
            }
            else
            {
                rtk_gpio_set_dir(sda, 0);
            }
            p_this->xfer.gpio_xfer_sub_state++;
            break;

        case G2C_STOP_SCL_HIGH:
            rtk_gpio_set_dir(scl, 0);
            p_this->xfer.gpio_xfer_sub_state++;
            break;

        case G2C_STOP_CHECK:
            if (rtk_gpio_input(scl))
            {
                rtk_gpio_set_dir(sda, 0);
                p_this->xfer.gpio_xfer_state = G2C_ST_DONE;
                p_this->xfer.gpio_xfer_sub_state = 0;
            }
            break;
        default:
            break;
    }
    return ret;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_do_complete
 *
 * Desc : complete GPIO i2c transxfer
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
int rtk_g2c_do_complete(rtk_i2c * p_this)
{
    RTK_GPIO_ID sda;
    RTK_GPIO_ID scl;
    int ret = 0;

    if (rtk_i2c_get_mapped_gpio(p_this, &sda, &scl) < 0)
        return -1;

    if (p_this->xfer.gpio_xfer_sub_state == 0)
    {
        rtk_gpio_set_dir(sda, 0);
        rtk_gpio_set_dir(scl, 0);
        rtk_i2c_gpio_selection(p_this, I2C_MODE);
        p_this->xfer.gpio_xfer_sub_state++;
        p_this->xfer.mode = I2C_IDEL;
    }
    return ret;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_isr
 *
 * Desc : isr of rtk gpio i2c
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0
 *------------------------------------------------------------------*/
static irqreturn_t rtk_g2c_isr(int this_irq, void *dev_id, struct pt_regs *regs)
{
    rtk_i2c *p_this = (rtk_i2c *) dev_id;

    unsigned long flags = 0 ;
    int ret = 0 ;

    LOCK_rtk_I2C(&p_this->lock, flags);

    if (p_this->xfer.mode != I2C_IDEL && time_after(jiffies, p_this->xfer.timeout))
    {
        p_this->xfer.ret = -ETIMEOUT;
        p_this->xfer.gpio_xfer_state = G2C_ST_DONE;
        p_this->xfer.gpio_xfer_sub_state = 0;

        I2C_WARNING("[ch_%d] Xfer fail - gpio can't trasfer TIMEOUT, %s Line%d\n",
                    p_this->id, __func__, __LINE__ );
    }

    switch (G2C_MAJOR_STATE(p_this->xfer.gpio_xfer_state))
    {
        case G2C_STATE_START:
            ret = rtk_g2c_do_start(p_this);
            break;
        case G2C_STATE_ADDR:
            ret = rtk_g2c_do_address(p_this);
            break;
        case G2C_STATE_STOP:
            ret = rtk_g2c_do_stop(p_this);
            break;
        case G2C_STATE_DONE:
            ret = rtk_g2c_do_complete(p_this);
            break;
        case G2C_STATE_DATA:
            if (p_this->xfer.mode == I2C_MASTER_WRITE)
                ret = rtk_g2c_do_write(p_this);
            else
                ret =rtk_g2c_do_read(p_this);
            break;
        default :
            ret = -1;
            break;
    }
    if(ret != 0)
        I2C_WARNING("[ch_%d][g2c] Xfer fail - gpio transfer failed.  rx: %d/%d tx: %d/%d   %s Line%d\n",
                    p_this->id,currentRxindex,totalRxDataLen,
                    currentTxindex,totalTxDataLen, __func__ , __LINE__ );

    UNLOCK_rtk_I2C(&p_this->lock, flags);

    return IRQ_HANDLED;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_start_xfer
 *
 * Desc : rtk_g2c_start_xfer
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : N/A
 *
 * Note : this file using GPIO4/5 to out I2C protocol. where GP4 is SCLK
 *        GP5 is SDA
 *------------------------------------------------------------------*/
int rtk_g2c_start_xfer(rtk_i2c * p_this)
{
    int d = p_this->tick >> 2;

    p_this->xfer.timeout = jiffies + (2 * HZ);
    p_this->xfer.gpio_wait_time = (G2C_WAIT_TIMEOUT * 1000);

    if (d == 0)
    {
        d = 1;
    }

    while (1)
    {
        rtk_g2c_isr(7, (void *)p_this, 0);

        if (p_this->xfer.mode == I2C_IDEL)
            break;

        if (p_this->xfer.gpio_wait_time <= d)
        {
            p_this->xfer.gpio_wait_time = G2C_WAIT_TIMEOUT * 1000;
            msleep(1);
        }
        else
        {
            p_this->xfer.gpio_wait_time -= d;
            udelay(d);
        }
    }

#ifdef CONFIG_RTK_KDRV_I2C_BUS_JAM_RECOVER
    if (p_this->xfer.ret == -ETIMEOUT)
    {
        I2C_WARNING("[ch_%d] Xfer fail - jam recover TIMEOUT, %s Line%d\n", p_this->id, __func__ , __LINE__ );
        if (rtk_i2c_bus_jam_detect(p_this))
        {

            I2C_WARNING("%s Bus Jammed, Do Recorver , LINE%d\n", __func__, __LINE__ );
            rtk_i2c_bus_jam_recover_proc(p_this);
            msleep(50);
        }
        I2C_WARNING("Info, Reset I2C State, Line%d\n", __LINE__);
    }
#endif

    return p_this->xfer.ret;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_null_function
 *
 * Desc : null function for gpio
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0 for success, others is failed
 *------------------------------------------------------------------*/
int rtk_g2c_null_function(rtk_i2c * p_this)
{
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_null_function
 *
 * Desc : null function for gpio
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0 for success, others is failed
 *------------------------------------------------------------------*/
int rtk_g2c_dump(rtk_i2c * p_this)
{
    RTK_GPIO_ID scl;
    RTK_GPIO_ID sda;

    I2C_WARNING( "==========%s  LINE:%d===============\n", __FUNCTION__, __LINE__ );
    I2C_WARNING( "= VER  : %s              \n", VERSION);
    I2C_WARNING( "=========================\n");
    I2C_WARNING( "= PHY  : %d              \n", p_this->id);
    I2C_WARNING( "= MODEL: G2C             \n");
    I2C_WARNING( "= SPD  : %d              \n", p_this->spd);

    if (rtk_i2c_get_mapped_gpio(p_this, &sda, &scl) == 0)
    {
        I2C_WARNING( "= SDA  : %s_GPIO %d\n", gpio_type(gpio_group(sda)),gpio_idx(sda));
        I2C_WARNING( "= SCL  : %s_GPIO %d\n", gpio_type(gpio_group(scl)),gpio_idx(scl));
    }
    else
    {
        I2C_WARNING( "= SDA  : Unknow GPIO\n");
        I2C_WARNING( "= SCL  : Unknow GPIO\n");
    }
    I2C_WARNING("= TX FIFO DEPTH : %d     \n", p_this->tx_fifo_depth);
    I2C_WARNING("= RX FIFO DEPTH : %d     \n", p_this->rx_fifo_depth);
    I2C_WARNING("=========================\n");

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_set_spd
 *
 * Desc : set speed of rtk i2c
 *
 * Parm : p_this : handle of rtk i2c
 *        KHz    : operation speed of i2c
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_g2c_set_spd(rtk_i2c * p_this, int KHz)
{
    if (KHz < 10 || KHz > 800)
    {
        I2C_WARNING( "[ch_%d] %s , Line %d, speed %d out of range, speed should between 10 ~ 800KHz\n",
                    p_this->id, __FUNCTION__, __LINE__, KHz);
        return -1;
    }

    p_this->spd = KHz;
    p_this->tick = 1000 / KHz;

    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_set_hw_guard_interval
 * Desc : set guard_interval of rtk g2c
 * Parm : p_this : handle of rtk g2c
 * us     : operation speed of g2c
 * Retn : 0
 * *------------------------------------------------------------------*/
int rtk_g2c_set_hw_guard_interval(rtk_i2c * p_this, unsigned long us)
{
    return 0;
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
int rtk_g2c_set_tar(rtk_i2c * p_this, unsigned short addr, ADDR_MODE mode)
{
    p_this->tar = addr;
    p_this->tar_mode = mode;
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_set_sar
 *
 * Desc : set sar of gpio i2c
 *
 * Parm : p_this : handle of rtk i2c
 *        addr   : address of sar
 *        mode
  : mode of sar
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_g2c_set_sar(rtk_i2c * p_this, unsigned short addr, ADDR_MODE mode)
{
    p_this->sar = addr;
    p_this->sar_mode = mode;
    return 0;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_slave_mode_enable
 *
 * Desc : enable/disable i2c slave mode
 *
 * Parm : p_this : handle of rtk i2c
 *        on     : enable /disable
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_g2c_slave_mode_enable(rtk_i2c * p_this, unsigned char on)
{
    return -1;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_set_port
 *
 * Desc : set port of rtk g2c
 *
 * Parm : p_this : handle of rtk i2c
 *        port   : output port selection
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_g2c_set_port(rtk_i2c * p_this, unsigned char port)
{
    return -EFAULT;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_unset_port
 *
 * Desc : set port of rtk g2c
 *
 * Parm : p_this : handle of rtk i2c
 *        port   : output port selection
 *
 * Retn : 0
 *------------------------------------------------------------------*/
int rtk_g2c_unset_port(rtk_i2c * p_this, unsigned char port)
{
    return -EFAULT;
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_write
 *
 * Desc : write data to sar - GPIO mode
 *
 * Parm : p_this : handle of rtk i2c
 *        tx_buf : data to write
 *        tx_buf_len : number of bytes to write
 *        wait_stop  : wait for stop of not (extension)
 *
 * Retn : 0 for success, others is failed
 *------------------------------------------------------------------*/
int rtk_g2c_write(rtk_i2c * p_this,
                  unsigned char *tx_buf,
                  unsigned short tx_buf_len, unsigned char wait_stop)
{
    if (!rtk_i2c_has_mapped_gpio(p_this) && p_this->write == rtk_i2c_write)
        return rtk_i2c_write(p_this, tx_buf, tx_buf_len, wait_stop);

    rtk_i2c_load_message(p_this, I2C_MASTER_WRITE,
                         tx_buf, tx_buf_len, NULL, 0,
                         (wait_stop) ? 0 : I2C_NO_STOP);

    return rtk_g2c_start_xfer(p_this);
}

/*------------------------------------------------------------------
 * Func : rtk_g2c_read
 *
 * Desc : read data from sar - GPIO mode
 *
 * Parm : p_this : handle of rtk i2c
 *
 * Retn : 0 for success, others is failed
 *------------------------------------------------------------------*/
int rtk_g2c_read(rtk_i2c * p_this,
                 unsigned char *tx_buf,
                 unsigned short tx_buf_len,
                 unsigned char *rx_buf, unsigned short rx_buf_len)
{
    int ret = 0;

    if (!rtk_i2c_has_mapped_gpio(p_this) && p_this->read == rtk_i2c_read)
        return rtk_i2c_read(p_this, tx_buf, tx_buf_len, rx_buf, rx_buf_len);

    if (tx_buf && tx_buf_len)
    {
        if ((ret = rtk_g2c_write(p_this, tx_buf, tx_buf_len, 0)) < 0)
            return ret;
    }

    rtk_i2c_load_message(p_this, I2C_MASTER_READ, NULL, 0, rx_buf, rx_buf_len, 0);

    return rtk_g2c_start_xfer(p_this);
}

/*------------------------------------------------------------------
 * Func : create_rtk_g2c_handle
 *
 * Desc : create handle of rtk g2c
 *
 * Parm : sda : gpio for sda
 *        scl : gpio for scl
 *
 * Retn : handle of rtk i2c
 *------------------------------------------------------------------*/
rtk_i2c *create_rtk_g2c_handle(unsigned char id,
                               RTK_GPIO_ID sda, RTK_GPIO_ID scl)
{
    rtk_i2c *hHandle = NULL ;

    if (id >= 8 || (rtk_i2c_flags >> (id + I2C_PHY_CNT)) & 0x01)
    {
        I2C_WARNING ( "[ch_%d] %s , Line %d, fail , flag = 0x%lX  , shift = 0x%X \n",
                      id, __FUNCTION__ , __LINE__ , rtk_i2c_flags  , (id + I2C_PHY_CNT)  ) ;
        return NULL;
    }

    hHandle = (rtk_i2c *) kmalloc(sizeof(rtk_i2c) + sizeof(rtk_i2c_port), GFP_KERNEL);

    if (hHandle != NULL)
    {
        memset(hHandle, 0, sizeof(rtk_i2c) + sizeof(rtk_i2c_port));

        hHandle->flags          = 0;
        hHandle->id             = id;
        hHandle->irq            = 0;
        hHandle->sar            = 0;
        hHandle->i2c_loglevel   = 1;
        hHandle->sar_mode       = ADDR_MODE_7BITS;
        hHandle->tar            = 0;
        hHandle->tar_mode       = ADDR_MODE_7BITS;
        hHandle->spd            = 100;
        hHandle->tick           = 1000 / hHandle->spd;
        hHandle->guard_interval = 1000;
        hHandle->rx_fifo_depth  = 16;
        hHandle->tx_fifo_depth  = 16;
        hHandle->time_stamp     = 0;

        memset(&hHandle->xfer, 0, sizeof(rtk_i2c_xfer));
        spin_lock_init(&hHandle->lock);
        mutex_init(&hHandle->mutex);

        hHandle->p_port = (rtk_i2c_port *) (((void *)hHandle) + sizeof(rtk_i2c_port));
        hHandle->n_port = 1;

        hHandle->current_port              = hHandle->p_port;
        hHandle->current_port->gpio_mapped = 1;
        hHandle->current_port->g2c_scl     = scl;
        hHandle->current_port->g2c_sda     = sda;

        hHandle->init                  = rtk_g2c_init;
        hHandle->uninit                = rtk_g2c_uninit;
        hHandle->set_spd               = rtk_g2c_set_spd;
        hHandle->set_guard_interval    = rtk_i2c_set_guard_interval;
        hHandle->set_hw_guard_interval = rtk_g2c_set_hw_guard_interval;
        hHandle->set_tar               = rtk_g2c_set_tar;
        hHandle->set_port              = rtk_g2c_set_port;
        hHandle->unset_port            = rtk_g2c_unset_port;
        hHandle->read                  = rtk_g2c_read;
        hHandle->write                 = rtk_g2c_write;
        hHandle->gpio_read             = rtk_g2c_read;
        hHandle->gpio_write            = rtk_g2c_write;
        hHandle->dump                  = rtk_g2c_dump;
        hHandle->suspend               = rtk_g2c_suspend;
        hHandle->resume                = rtk_g2c_resume;
        hHandle->get_tx_abort_reason   = rtk_g2c_get_tx_abort_reason;

#ifdef CONFIG_RTK_KDRV_I2C_BUS_JAM_RECOVER
        hHandle->get_bus_status      = rtk_i2c_bus_jam_detect;
        hHandle->do_bus_jame_recover = rtk_i2c_bus_jam_recover_proc;
#endif
        hHandle->set_sar            = rtk_g2c_set_sar;
        hHandle->slave_mode_enable  = rtk_g2c_slave_mode_enable;
        hHandle->register_slave_ops = rtk_i2c_register_slave_ops;

        rtk_i2c_flags |= (0x01 << (id + I2C_PHY_CNT));
    }

    return hHandle;
}
EXPORT_SYMBOL(create_rtk_g2c_handle);
