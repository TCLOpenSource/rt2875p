#include <rbus/misc_i2c_reg.h>
#include "rtk_i2c_priv.h"
#include "rtk_i2c.h"
#include "rtk_i2c_slave.h"

#define ST_SLV_ACT_BIT          (1 << MISC_I2C_IC1_STATUS_slv_activity_shift)
#define ST_MST_ACT_BIT          (1 << MISC_I2C_IC1_STATUS_mst_activity_shift)

/*------------------------------------------------------------------
 * Func : rtk_i2c_slave_handle_cmd
 *
 * Desc : This function will be called as the i2c hardware received
 *        message from remote i2c master
 *
 * Parm : id     : i2c adapter index
 *        cmd    : slave mode ops
 *        len    : length of command (in bytes)
 *        stop   : slave rx stop flag
 *
 * Retn : 0 : successed, others: failed
 *------------------------------------------------------------------*/
#define MAX_SLAVE_CMD       1024
static unsigned char slave_cmd_buff[MAX_I2C_ADAPTER][MAX_SLAVE_CMD] = {{0}};
static unsigned int slave_cmd_cnt[MAX_I2C_ADAPTER] = {0};
static int rtk_i2c_slave_handle_cmd(int id, unsigned char *cmd, unsigned char len, unsigned char stop)
{
    int i;
    if ((slave_cmd_cnt[id]+len) < MAX_SLAVE_CMD)
    {
        memcpy((slave_cmd_buff[id] + slave_cmd_cnt[id]), cmd, len);
        slave_cmd_cnt[id] += len;
    }
    else
    {
        I2C_WARNING("[%s] [ch_%d] write len exceeds the MAX slave cmd cnt!\n", __func__, id);
    }
    if (1 == stop)
    {
        I2C_WARNING("[%s] [ch_%d] rx_cnt:%d\n", __func__, id, slave_cmd_cnt[id]);
        I2C_WARNING("ch_%d rx_cmd:\n", id);
        for (i=0; i<slave_cmd_cnt[id]; i++)
        {
            I2C_WARNING("cmd-%02d:0x%02x\n", i, slave_cmd_buff[id][i]);
        }
        slave_cmd_cnt[id] = 0;
        memset(slave_cmd_buff[id], 0, sizeof(slave_cmd_buff[id]));
    }
    return 0;
}

/*----------------------------------------------------------------------
 * Func : read_data
 *
 * Desc : This function will be called as the i2c hardware got read request
 *        from remote i2c master.
 *
 * Parm : id     : i2c adapter index
 *
 * Retn : data to return
*----------------------------------------------------------------------*/
static unsigned char slave_ret[MAX_I2C_ADAPTER] = {0};
unsigned char rtk_i2c_slave_read_data (int id)
{
    slave_ret[id]++;

    if (slave_ret[id] >= 0xff)
    {
        slave_ret[id] = 0;
    }
    I2C_WARNING("[%s] [ch_%d] ret=0x%02x\n", __func__, id, slave_ret[id]);

    return slave_ret[id];
}

/*----------------------------------------------------------------------
 * Func : stop
 *
 * Desc : This function will be called as the i2c hardware got stop
 *        from remote i2c master.
 *
 * Parm : id     : i2c adapter index
 *
 * Retn : data to return
*----------------------------------------------------------------------*/
void rtk_i2c_slave_stop(int id)
{
    I2C_WARNING("[%s] [ch_%d] slave stop\n", __func__, id);
}

/*------------------------------------------------------------------
 * Func : rtk_i2c_slave_en
 *
 * Desc : i2c slave_mode enable/disable test
 *
 * Parm : p_this : handle of rtk i2c
 *        val    : 1: enable slave mode;
 *                 0: disable slave mode
 *
 * Retn : void
 *------------------------------------------------------------------*/
void rtk_i2c_slave_en(rtk_i2c *p_this, int val)
{
    rtk_i2c_slave_driver p_slave_drv;

    if (1 == val)
    {
        if (p_this->xfer.mode != I2C_IDEL && (GET_IC_STATUS(p_this) & ST_MST_ACT_BIT))
        {
            I2C_WARNING("[%s] I2C ch_%d master is busy or activity\n", __func__, p_this->logical_id);
            return ;
        }

        if (p_this->flags & rtk_I2C_SLAVE_ENABLE)
        {
            I2C_WARNING("[ch_%d] slave has been enabled, slave addr:0x%02x\n", p_this->logical_id, GET_IC_SAR(p_this));
        }
        else
        {
            p_slave_drv.handle_command  = rtk_i2c_slave_handle_cmd;
            p_slave_drv.read_data       = rtk_i2c_slave_read_data;
            p_slave_drv.stop            = rtk_i2c_slave_stop;
            p_slave_drv.private_data    = p_this->logical_id;       //slave_id
            p_slave_drv.sar             = 0x10 + p_this->logical_id; //slave addr(7-bit)
            rtk_i2c_register_slave_driver(p_this->id, &p_slave_drv);
            rtk_i2c_slave_enable(p_this->id, 1);
        }
    }
    else if (0 == val)
    {
        if (GET_IC_STATUS(p_this) & ST_SLV_ACT_BIT)
        {
            I2C_WARNING("[%s] I2C ch_%d slave is activity\n", __func__, p_this->logical_id);
            return ;
        }

        if (p_this->flags & rtk_I2C_SLAVE_ENABLE)
        {
            rtk_i2c_unregister_slave_driver(p_this->id, NULL);
            rtk_i2c_slave_enable(p_this->id, 0);
        }
        else
        {
            I2C_WARNING("[ch_%d] slave has been disabled\n", p_this->logical_id);
        }
    }
}
