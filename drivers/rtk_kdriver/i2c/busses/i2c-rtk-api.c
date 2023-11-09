#ifndef BUILD_QUICK_SHOW
#include <linux/kernel.h>
#include <linux/i2c.h>
#else
#include <errno.h>
#endif
#include <rtk_kdriver/i2c-rtk-api.h>
#include <rtd_log/rtd_module_log.h>

#ifndef BUILD_QUICK_SHOW
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
/*------------------------------------------------------------------
 * Func : i2c_master_recv_ex
 *
 * Desc : i2c read
 *
 * Parm : bus          : bus id
 *        addr         : device addr (7bits)
 *        p_sub_addr   : sub address
 *        sub_addr_len : length of sub address
 *        p_read_buff  : buffer to store readed data
 *        read_len     : number of bytes to read
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
int i2c_master_recv_ex(unsigned char bus_id, unsigned char addr,
	unsigned char *p_sub_addr, unsigned char sub_addr_len,
	unsigned char *p_read_buff, unsigned int read_len)
{
	struct i2c_adapter *p_adap;
	struct i2c_msg i2c_message[2];
	unsigned msg_cnt = 0;
	int ret;

	if(read_len == 0)
		return -EFAULT;

	if((p_adap = i2c_get_adapter(bus_id)) == NULL) {
		rtd_pr_i2c_err("%s:get adapter %d fail.\n", __func__, bus_id);
		return -ENODEV;
	}

	if (sub_addr_len > 0) {
		i2c_message[msg_cnt].addr  = addr;
		i2c_message[msg_cnt].flags = 0;
		i2c_message[msg_cnt].len   = sub_addr_len;
		i2c_message[msg_cnt].buf   = p_sub_addr;
		msg_cnt++;
	}

	i2c_message[msg_cnt].addr  = addr;
	i2c_message[msg_cnt].flags = I2C_M_RD;
	i2c_message[msg_cnt].len   = read_len;
	i2c_message[msg_cnt].buf   = p_read_buff;
	msg_cnt++;

	ret = i2c_transfer(p_adap, i2c_message, msg_cnt);
	i2c_put_adapter(p_adap);
	return ret;
}
EXPORT_SYMBOL(i2c_master_recv_ex);


/*------------------------------------------------------------------
 * Func : i2c_master_send_ex
 *
 * Desc : write i2c by bus id
 *
 * Parm : bus          : bus id
 *        addr         : device addr (7bits)
 *        write_buff   : write data buffer
 *        write_len    : write data length
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
int i2c_master_send_ex(unsigned char bus_id, unsigned char addr,
	unsigned char *write_buff, unsigned int write_len)
{
	struct i2c_adapter *p_adap;
	struct i2c_msg i2c_message;
	int ret;

	if(write_len == 0)
		return 0;

	if((p_adap = i2c_get_adapter(bus_id)) == NULL) {
		rtd_pr_i2c_err("%s:get adapter %d fail.\n", __func__, bus_id);
		return -ENODEV;
	}

	i2c_message.addr  = addr;
	i2c_message.flags = 0;
	i2c_message.len   = write_len;
	i2c_message.buf   = write_buff;

	ret = i2c_transfer(p_adap, &i2c_message, 1);
	i2c_put_adapter(p_adap);
	return ret;
}
EXPORT_SYMBOL(i2c_master_send_ex);

/*------------------------------------------------------------------
 * Func : i2c_master_recv_ex_flag
 *
 * Desc : i2c read
 *
 * Parm : bus          : bus id
 *        addr         : device addr (7bits)
 *        p_sub_addr   : sub address
 *        sub_addr_len : length of sub address
 *        p_read_buff  : buffer to store readed data
 *        read_len     : number of bytes to read
 *        flags         : flag value
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
int i2c_master_recv_ex_flag(unsigned char bus_id, unsigned char addr,
	unsigned char *p_sub_addr, unsigned char sub_addr_len,
	unsigned char *p_read_buff, unsigned int read_len, __u16 flags)
{
	struct i2c_adapter *p_adap;
	struct i2c_msg i2c_message[2];
	unsigned msg_cnt = 0;
	int ret;

	if(read_len == 0)
		return -EFAULT;

	if((p_adap = i2c_get_adapter(bus_id)) == NULL) {
		rtd_pr_i2c_err("%s:get adapter %d fail.\n", __func__, bus_id);
		return -ENODEV;
	}

	if (sub_addr_len > 0) {
		i2c_message[msg_cnt].addr  = addr;
		i2c_message[msg_cnt].flags = flags;
		i2c_message[msg_cnt].len   = sub_addr_len;
		i2c_message[msg_cnt].buf   = p_sub_addr;
		msg_cnt++;
	}

	i2c_message[msg_cnt].addr  = addr;
	i2c_message[msg_cnt].flags = I2C_M_RD | flags;
	i2c_message[msg_cnt].len   = read_len;
	i2c_message[msg_cnt].buf   = p_read_buff;
	msg_cnt++;

	ret = i2c_transfer(p_adap, i2c_message, msg_cnt);
	i2c_put_adapter(p_adap);
	return ret;
}
EXPORT_SYMBOL(i2c_master_recv_ex_flag);

/*------------------------------------------------------------------
 * Func : i2c_master_send_ex_flag
 *
 * Desc : write i2c by bus id
 *
 * Parm : bus          : bus id
 *        addr         : device addr (7bits)
 *        write_buff   : write data buffer
 *        write_len    : write data length
 *        flags         : flag value
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
int i2c_master_send_ex_flag(unsigned char bus_id, unsigned char addr,
	unsigned char *write_buff, unsigned int write_len, __u16 flags)
{
	struct i2c_adapter *p_adap;
	struct i2c_msg i2c_message;
	int ret;

	if(write_len == 0)
		return 0;

	if((p_adap = i2c_get_adapter(bus_id)) == NULL) {
		rtd_pr_i2c_err("%s:get adapter %d fail.\n", __func__, bus_id);
		return -ENODEV;
	}

	i2c_message.addr  = addr;
	i2c_message.flags = flags;
	i2c_message.len   = write_len;
	i2c_message.buf   = write_buff;

	ret = i2c_transfer(p_adap, &i2c_message, 1);
	i2c_put_adapter(p_adap);
	return ret;
}
EXPORT_SYMBOL(i2c_master_send_ex_flag);
/*------------------------------------------------------------------
 * Func : i2c_master_EDDC_ex
 *
 * Desc : i2c read
 *
 * Parm : bus          : bus id
 *        addr         : device addr (7bits)
 *        p_sub_addr   : sub address
 *        sub_addr_len : length of sub address
 *        p_read_buff  : buffer to store readed data
 *        read_len     : number of bytes to read
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
int i2c_master_EDDC_ex(unsigned char bus_id, unsigned char addr,
    unsigned char *p_sub_addr, unsigned char sub_addr_len,
    unsigned char *p_read_buff, unsigned int read_len)
{
    struct i2c_adapter *p_adap;
    struct i2c_msg i2c_message[3];
    unsigned char w_data[1] = {1};
    unsigned msg_cnt = 0;
    int ret;

    if(read_len == 0)
        return -EFAULT;

    if((p_adap = i2c_get_adapter(bus_id)) == NULL) {
        rtd_pr_i2c_err("%s:get adapter %d fail.\n", __func__, bus_id);
        return -ENODEV;
    }

    i2c_message[msg_cnt].addr  = 0x30;
    i2c_message[msg_cnt].flags = I2C_GPIO_RW | I2C_M_NO_GUARD_TIME;
    i2c_message[msg_cnt].len   = 1;
    i2c_message[msg_cnt].buf   = w_data;
    msg_cnt++;

    if (sub_addr_len > 0) {
        i2c_message[msg_cnt].addr  = addr;
        i2c_message[msg_cnt].flags = 0 | I2C_M_NO_GUARD_TIME;
        i2c_message[msg_cnt].len   = sub_addr_len;
        i2c_message[msg_cnt].buf   = p_sub_addr;
        msg_cnt++;
    }
    
    i2c_message[msg_cnt].addr  = addr;
    i2c_message[msg_cnt].flags = I2C_M_RD | I2C_M_NO_GUARD_TIME;
    i2c_message[msg_cnt].len   = read_len;
    i2c_message[msg_cnt].buf   = p_read_buff;
    msg_cnt++;
    ret = i2c_transfer(p_adap, i2c_message, msg_cnt);
    return ret;
}
EXPORT_SYMBOL(i2c_master_EDDC_ex);
#endif
#else
/*------------------------------------------------------------------
 * Func : i2c_master_recv_ex
 *
 * Desc : i2c read
 *
 * Parm : bus          : bus id
 *        addr         : device addr (7bits)
 *        p_sub_addr   : sub address
 *        sub_addr_len : length of sub address
 *        p_read_buff  : buffer to store readed data
 *        read_len     : number of bytes to read
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
int i2c_master_recv_ex(unsigned char bus_id, unsigned char addr,
    unsigned char *p_sub_addr, unsigned char sub_addr_len,
    unsigned char *p_read_buff, unsigned int read_len)
{
    int ret;

    if(read_len == 0)
        return -EFAULT;

    ret = i2c_rtk_xfer(bus_id, addr, p_sub_addr, sub_addr_len, p_read_buff, read_len, 0);
    return ret;
}


/*------------------------------------------------------------------
 * Func : i2c_master_send_ex
 *
 * Desc : write i2c by bus id
 *
 * Parm : bus          : bus id
 *        addr         : device addr (7bits)
 *        write_buff   : write data buffer
 *        write_len    : write data length
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
int i2c_master_send_ex(unsigned char bus_id, unsigned char addr,
    unsigned char *write_buff, unsigned int write_len)
{
    int ret;

    if(write_len == 0)
        return 0;

    ret = i2c_rtk_xfer(bus_id, addr, write_buff, write_len, NULL, 0, 0);
    return ret;
}

/*------------------------------------------------------------------
 * Func : i2c_master_recv_ex_flag
 *
 * Desc : i2c read
 *
 * Parm : bus          : bus id
 *        addr         : device addr (7bits)
 *        p_sub_addr   : sub address
 *        sub_addr_len : length of sub address
 *        p_read_buff  : buffer to store readed data
 *        read_len     : number of bytes to read
 *        flags         : flag value
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
int i2c_master_recv_ex_flag(unsigned char bus_id, unsigned char addr,
    unsigned char *p_sub_addr, unsigned char sub_addr_len,
    unsigned char *p_read_buff, unsigned int read_len, __u16 flags)
{
    int ret;

    if(read_len == 0)
        return -EFAULT;

    ret = i2c_rtk_xfer(bus_id, addr, p_sub_addr, sub_addr_len, p_read_buff, read_len, flags);
    return ret;
}

/*------------------------------------------------------------------
 * Func : i2c_master_send_ex_flag
 *
 * Desc : write i2c by bus id
 *
 * Parm : bus          : bus id
 *        addr         : device addr (7bits)
 *        write_buff   : write data buffer
 *        write_len    : write data length
 *        flags         : flag value
 *
 * Retn : 0 : success, others failed
 *------------------------------------------------------------------*/
int i2c_master_send_ex_flag(unsigned char bus_id, unsigned char addr,
    unsigned char *write_buff, unsigned int write_len, __u16 flags)
{
    int ret;

    if(write_len == 0)
        return 0;

    ret = i2c_rtk_xfer(bus_id, addr, write_buff, write_len, NULL, 0, flags);
    return ret;
}

#endif

