#ifndef __I2C_RTK_API_H__
#define __I2C_RTK_API_H__

#ifndef BUILD_QUICK_SHOW
#include <linux/version.h>
#include <linux/kernel.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#define I2C_RTK_API_SUPPORT
#endif
#else
#include <sysdefs.h>
#define I2C_RTK_API_SUPPORT
int i2c_rtk_xfer(unsigned char bus_id, unsigned char addr,
        unsigned char *p_sub_addr, unsigned char sub_addr_len,
        unsigned char *p_read_buff, unsigned int read_len, unsigned short flags);
#endif /*BUILD_QUICK_SHOW*/


#ifdef I2C_RTK_API_SUPPORT
#define I2C_GPIO_RW             0x0080 /* 20100510 - Kevin Wang add for Venus I2C */
#define I2C_M_NORMAL_SPEED      0x0000 /* 20120716 - Kevin Wang add for Standard Speed Transmission : 100Kbps */
#define I2C_M_FAST_SPEED        0x0002 /* 20120716 - Kevin Wang add for Fast Speed Transmission : 400Kbps */
#define I2C_M_HIGH_SPEED        0x0004 /* 20120716 - Kevin Wang add for High Speed Transmission : > 400Kbps to max 3.4 Mbps */
#define I2C_M_LOW_SPEED         0x0006 /* 20120716 - Kevin Wang add for Low  Speed Transmission : < 100Kbps */
#define I2C_M_SPEED_MASK        0x0006 /* 20120716 - Kevin Wang add for speed control*/
#define I2C_M_NO_GUARD_TIME     0x0008 /* 20121120 - Kevin Wang add for disable guard time*/
#define I2C_M_HW_DELAY_MASK     0x0070
#define I2C_M_400US_DELAY       0x0010
#define I2C_M_600US_DELAY       0x0020
#define I2C_M_800US_DELAY       0x0030
#define I2C_M_1000US_DELAY      0x0040
#define I2C_M_2000US_DELAY      0x0050

/*
 * These two apis are created by Kevin, that used to ease the way of
 * issuing sequential read, random read or write command on specified bus.
 */
int i2c_master_send_ex(unsigned char bus_id, unsigned char addr,
	unsigned char *write_buff, unsigned int write_len);

int i2c_master_recv_ex(unsigned char bus_id, unsigned char addr,
	unsigned char *p_sub_addr, unsigned char sub_addr_len,
	unsigned char *p_read_buff, unsigned int read_len);

int i2c_master_send_ex_flag(unsigned char bus_id, unsigned char addr,
	unsigned char *write_buff, unsigned int write_len, __u16 flags);

int i2c_master_recv_ex_flag(unsigned char bus_id, unsigned char addr,
	unsigned char *p_sub_addr, unsigned char sub_addr_len,
	unsigned char *p_read_buff, unsigned int read_len, __u16 flags);

int i2c_master_EDDC_ex(unsigned char bus_id, unsigned char addr,
    unsigned char *p_sub_addr, unsigned char sub_addr_len,
    unsigned char *p_read_buff, unsigned int read_len);

#endif

#endif
