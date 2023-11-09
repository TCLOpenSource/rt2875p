//Kernel Header file
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>		/* everything... */
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <rtk_kdriver/rtk_mcp.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <rtk_kdriver/io.h>
#include <generated/autoconf.h>

#include "scaler_hdcp2dev.h"
#include <tvscalercontrol/hdcp2_2/hdcp2_messages.h>
#include <tvscalercontrol/hdcp2_2/hdcp2_interface.h>
#include <tvscalercontrol/hdcp2_2/crypto.h>
#include <tvscalercontrol/hdcp2_2/hdcp2_hal.h>
#include <tvscalercontrol/hdcp2_2/hdcp2_rcp_api.h>
#include <tvscalercontrol/hdcp2_2/hmac.h>

#include <tvscalercontrol/hdcp2_2/hdcp2_hal.h>
#ifndef CONFIG_OPTEE_HDCP2
//#include <hdmirx/hdmi_reg.h>
#else
#include <hdmirx/hdmi_reg.h>
#include <hdmirx/hdmi_common.h>
#endif

#if 0//mark by zhaodong k7
//#include <tvscalercontrol/hdmirx/hdmi_hdcp.h>
#endif
#include <rtk_kdriver/rtk_hdcp_otp_util.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/hdmirx/hdcp2_optee.h>
#if 0//mark by zhaodong k7
#include <rbus/hdmi_stb_p0_reg.h>
#endif

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#define to_user_ptr(x)          compat_ptr((unsigned int)x)
#else
#define to_user_ptr(x)          ((void* __user)(x)) // convert 32 bit value to user pointer
#endif


struct workqueue_struct *hdcp2p2_wq = NULL;
LIST_HEAD(hdcp2p2_list);
static void *hdcp2p2_mem = NULL;

static struct semaphore HDCP2_Semaphore;

#define		BIT0		0x00000001
#define		BIT1		0x00000002
#define		BIT2		0x00000004
#define		BIT3		0x00000008
#define		BIT4		0x00000010
#define		BIT5		0x00000020
#define		BIT6		0x00000040
#define		BIT7		0x00000080
#define		BIT8		0x00000100
#define		BIT9		0x00000200
#define		BIT10		0x00000400

#define DBG_PRINT(s, args...) pr_debug(s, ## args)

static dev_t hdcp2_devno = 0;//vfe device number
static struct cdev hdcp2_cdev;

#define OK true
#define NOT_OK false
#define HDCP_1p4 0

#ifdef CONFIG_CUSTOMER_TV010
unsigned char loacl_key[902]=
{
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,0xFF,0XFF,
0xFF,0XFF,0xFF,0XFF,0xFF,0XFF

/*
0x00,0x00,0x00,0x02,0xb5,0xd8,0xe9,0xab,0x5f,0x8a,0xfe,0xca,0x38,0x55,0xb1,0xa5,
0x1e,0xc9,0xbc,0x0f,0x38,0xed,0x6e,0xd4,0xfe,0x89,0x63,0x93,0x61,0x28,0x26,0x58,
0x1f,0x9d,0x6e,0x1a,0xd5,0x96,0xc0,0x8b,0xb6,0x61,0xcd,0x0b,0xd4,0x9b,0x2f,0x4a,
0x99,0xa1,0x98,0xd5,0x0e,0xec,0x15,0x2b,0x41,0xdb,0xa8,0x6a,0x7d,0xf4,0xb8,0x15,
0xf3,0x6b,0x6a,0x98,0x4c,0xf0,0x73,0x39,0x65,0x70,0x72,0x3f,0x57,0x84,0x89,0x47,
0x61,0x46,0x8f,0xf4,0xaa,0xae,0x3a,0xf2,0x00,0x50,0xa5,0x3e,0xe7,0xa7,0x65,0x7e,
0x97,0x1f,0x2d,0xd7,0x34,0xd5,0xdc,0x8a,0x01,0x0a,0x4b,0xb7,0x02,0xbc,0x2e,0x8b,
0xfb,0xf2,0xaf,0x11,0xef,0x58,0x78,0x42,0x3c,0xbb,0xb3,0x80,0xd6,0xbd,0x6a,0xb3,
0x23,0x95,0x63,0xa8,0xbe,0x36,0x83,0xc3,0x60,0xd3,0xaa,0x51,0x60,0x50,0x01,0xb0,
0x6d,0xaf,0x3f,0xb9,0x44,0xb2,0xb0,0x3b,0x06,0x76,0x3a,0x81,0x6e,0x51,0x60,0xfa,
0xa8,0xaa,0x3f,0x49,0x5a,0x6d,0xbf,0x7f,0x17,0x97,0x08,0xdf,0x7d,0x01,0x00,0x01,
0x10,0x00,0x96,0x7e,0x89,0x90,0x13,0x38,0xa5,0xc5,0x5d,0x79,0xd9,0x88,0xf4,0xe8,
0x14,0x8f,0x73,0xb2,0x2b,0x24,0x51,0x4b,0x60,0x58,0xc0,0x9c,0xb2,0x09,0x46,0xc1,
0x42,0xa9,0x25,0xb5,0x34,0x75,0x12,0xac,0x30,0xf9,0xfb,0x32,0xe5,0x73,0xaf,0xb6,
0xa0,0xbe,0xae,0x3c,0xc8,0xab,0xfd,0xbb,0xc9,0x2c,0x76,0x95,0x2e,0x93,0x1b,0xf6,
0xd3,0x03,0xe1,0x4c,0x24,0xf5,0x14,0x27,0x78,0x57,0x56,0x37,0x97,0x3f,0x32,0xee,
0xe0,0xe3,0x67,0xda,0x57,0x27,0xbd,0x5d,0xf8,0x22,0x85,0xf9,0x30,0x57,0x5e,0x1f,
0x97,0x3e,0xfc,0x08,0xce,0x07,0xe0,0xb0,0xd1,0xd1,0x3d,0x75,0x5e,0x8e,0x78,0x2a,
0x95,0x1a,0xe9,0xf1,0x13,0xcd,0x50,0x55,0xd1,0xe0,0x64,0x6f,0x47,0x0c,0x6b,0x46,
0x32,0xdb,0xf7,0xf2,0x50,0x59,0x65,0x55,0x73,0x35,0xc2,0x4c,0x6a,0x4f,0xfc,0x0a,
0xfb,0xda,0x31,0xfb,0xef,0x81,0x0e,0xa5,0xe1,0xa5,0x4e,0x9e,0x87,0xbc,0xb6,0x9c,
0xa2,0xcb,0x70,0x39,0xa3,0xe1,0x49,0x56,0xc2,0x6b,0xfb,0x86,0x3e,0xb3,0x87,0xe0,
0xba,0xb5,0xca,0x09,0x66,0xfc,0x04,0x16,0xfb,0xdd,0xf0,0x50,0xe4,0xfd,0x50,0x9a,
0xc1,0x0f,0x2f,0x96,0x05,0x80,0x13,0x2b,0xc6,0x01,0xd3,0x41,0xac,0xce,0x51,0xa7,
0xa8,0x09,0x4b,0x12,0xf6,0xb7,0x6c,0x7c,0xa8,0x12,0x38,0x07,0xba,0x92,0x58,0xe6,
0x83,0x66,0xec,0x79,0x29,0xb0,0x76,0x39,0xe1,0xbe,0x46,0xa4,0x4a,0x23,0x8d,0x7c,
0xd5,0x05,0x3d,0x97,0x9b,0x70,0x52,0xad,0xc9,0xb2,0x37,0x2c,0x61,0xae,0xdc,0x78,
0xb2,0x1f,0x64,0x86,0x9c,0x71,0x0a,0x66,0x21,0xa4,0x78,0x7e,0x81,0x31,0x94,0xd8,
0x2f,0x09,0xb3,0x69,0x32,0xb7,0xe5,0xae,0x92,0xb2,0xab,0x61,0xcf,0x2a,0xd1,0xd6,
0x28,0x9f,0x34,0x17,0x8a,0xd5,0x42,0xcb,0xcc,0xd9,0x9d,0x58,0xdd,0x88,0x78,0xec,
0xae,0xa4,0x17,0x27,0x0c,0x0a,0x61,0x2d,0xab,0xaf,0x8b,0xf0,0x44,0xf2,0x9c,0xa8,
0xa1,0xbb,0x20,0x24,0x70,0x3c,0x8a,0x2f,0x02,0x89,0x9e,0x20,0x41,0xfd,0x3e,0x8d,
0x1e,0x87,0x99,0x96,0xb9,0x9f,0x23,0x4d,0xa7,0x2e,0x90,0xea,0x0f,0xfd,0x8f,0xc1,
0x1e,0xf0,0xf4,0x4c,0x55,0xc5,0x79,0x71,0x0a,0x38,0x86,0x29,0xf2,0x60,0x7b,0x4a,
0x5d,0xc4,0xf6,0xef,0x7d,0x05,0x27,0x69,0x25,0x58,0xd1,0x56,0x2b,0xec,0xd1,0x4e,
0xd2,0xe1,0xd6,0x03,0xd7,0x1f,0xea,0x7a,0x4c,0x35,0x22,0x08,0x43,0x23,0xee,0xca,
0x57,0x49,0x17,0xec,0x1c,0x30,0x65,0x4a,0x3e,0xe3,0xd8,0x39,0x53,0x5a,0x5f,0x5c,
0x9d,0x53,0xb1,0x33,0x13,0xaf,0xda,0x29,0xbe,0x25,0x80,0xcb,0x37,0x43,0x75,0x13,
0xd4,0xbc,0x7e,0x9b,0x97,0x4c,0xa0,0x17,0x5b,0xba,0xaf,0x6d,0x41,0x04,0x25,0xaa,
0x22,0x19,0xb9,0xa0,0xe7,0xbe,0xf7,0xe4,0x27,0xd5,0x90,0xe1,0x93,0x83,0xb9,0x5b,
0xec,0xfe,0x48,0x04,0x89,0x4c,0x14,0x54,0x5a,0xb4,0xb8,0x29,0x8f,0xaf,0xe0,0xd4,
0x8c,0x2f,0xe9,0xa9,0x95,0xab,0x41,0x6d,0xa3,0x7c,0x95,0x9c,0x69,0xb0,0x16,0xc1,
0x7e,0x02,0x2c,0x06,0x78,0xd4,0xa8,0x6f,0x6b,0x05,0x05,0xde,0xb2,0x5f,0xaa,0x90,
0x7d,0x05,0x5f,0x55,0xb2,0x9c,0xeb,0x0c,0xb3,0xa9,0xef,0x91,0x12,0xa7,0x8d,0x15,
0xe1,0xb0,0x13,0xd1,0xff,0xf4,0xa7,0x0a,0x5d,0xdd,0x8d,0x52,0x6c,0x09,0x13,0xf5,
0xe3,0xb7,0x86,0x76,0x24,0x4c,0xef,0xb8,0x2c,0x95,0xf4,0x3b,0x92,0x50,0xa0,0x74,
0xc7,0x0d,0x78,0xb0,0xc2,0xc3,0x45,0x85,0x62,0x3d,0x55,0x2c,0x2c,0x0c,0x36,0xe3,
0x04,0xf1,0x57,0x28,0xca,0x27,0xcc,0xd5,0x80,0x2e,0xb3,0x1c,0x77,0xdb,0x3d,0xc9,
0x7f,0xf9,0xd5,0x6e,0xeb,0xd8,0xa1,0x23,0x0a,0x84,0x18,0x84,0xcb,0x46,0xd5,0xd6,
0x77,0x0c,0x78,0x36,0x7c,0x64,0x31,0x10,0x7d,0xd7,0x1d,0x98,0x97,0xb7,0x6a,0xb7,
0xe7,0xa6,0xb5,0x6d,0x44,0x86,0xf5,0x9f,0x7b,0x2c,0x9c,0x98,0xf8,0x40,0x1b,0x22,
0x44,0xd1,0xc3,0xa6,0x8c,0xbe,0xec,0x00,0xd9,0xc8,0x9c,0x86,0xe8,0xba,0xc6,0xf7,
0x43,0x2c,0x5b,0xc9,0x7e,0x07,0x47,0xef,0x15,0x3a,0x27,0x53,0xf6,0x45,0x08,0x81,
0x95,0x72,0xdd,0x48,0x29,0x1c,0x8d,0xee,0x4f,0x1a,0x58,0x0a,0xd3,0x6b,0x1a,0x3a,
0xf0,0xb7,0x03,0xf1,0x04,0xd9,0x69,0x21,0x40,0x14,0x2b,0x73,0x95,0x3c,0xc1,0xff,
0xa1,0xee,0x91,0x09,0x81,0xe3,0xb5,0xb5,0x61,0xf9,0x68,0x2d,0x7b,0xb0,0xe9,0x0c,
0xa3,0x05,0x99,0x35,0xc1,0x87
*/

};
#endif

#if 0 //for roku key test
unsigned char hdcp2_ap_key[2144] = {
	0xE0,0x2D,0xF6,0x32,0x1D,0x13,0x51,0x4B,0xAD,0x51,0x7A,0xA5,0x8C,0xE4,0x34,0xF4,
	0x86,0x03,0x00,0x00,0x7A,0x04,0x00,0x00,0x9A,0x89,0x51,0xB3,0xD6,0x00,0x00,0x00,
	0x04,0x8D,0x6D,0x37,0x4A,0x21,0x85,0xE6,0xFA,0xD1,0xDB,0xFC,0xCF,0xE1,0x97,0xD9,
	0x28,0x5A,0x58,0x67,0x05,0xDD,0xF2,0x59,0xE1,0x42,0x90,0xD0,0x97,0x56,0x60,0x81,
	0x8D,0x06,0xDC,0xAA,0x28,0x3B,0xB3,0x37,0x26,0x41,0xAA,0x43,0x39,0x5D,0x41,0x67,
	0x12,0xCD,0xF6,0xE7,0x7D,0x56,0xDC,0x8B,0xC9,0x4A,0x1E,0x56,0x46,0x67,0xE4,0xAB,
	0x24,0xDB,0xD2,0x38,0x63,0x05,0x06,0xF7,0x8A,0x90,0xDF,0x1E,0xE8,0xB8,0xDA,0xD4,
	0x6A,0xE1,0xFF,0x17,0x5B,0xB0,0xC6,0x5E,0x54,0x57,0xD7,0x7A,0x72,0xC9,0xDA,0xF2,
	0x35,0x0D,0x3D,0x6E,0x38,0x31,0x06,0x8F,0xF4,0x53,0x0C,0x5E,0x2B,0xEC,0xC4,0x14,
	0x76,0xFD,0xF0,0x8A,0x4E,0x7D,0xFF,0x5C,0x37,0x1B,0xF1,0x04,0xF8,0xA5,0x1C,0xC3,
	0x6C,0x77,0x8E,0xBD,0x65,0x50,0xDE,0x30,0xB7,0x32,0xA6,0x1A,0xAD,0xC9,0x2E,0xCC,
	0xCF,0x7F,0xA4,0xB3,0x3B,0x1B,0x60,0x48,0x9A,0x13,0x58,0xCB,0x57,0x57,0x95,0x0B,
	0xE4,0x7B,0xC5,0xE5,0x17,0xB8,0xF1,0xBE,0x4F,0xCA,0xAE,0x06,0x1D,0x36,0x9F,0x28,
	0x37,0xFD,0x64,0xF4,0x0C,0xA7,0xA5,0x39,0x94,0xB5,0xBE,0x58,0xCD,0x17,0x0A,0x65,
	0x1F,0xC3,0x06,0x53,0x43,0xD8,0x3D,0x5D,0x6B,0xF3,0x9B,0xAF,0x85,0x52,0x16,0xBC,
	0xA4,0xBE,0x7E,0x56,0xF6,0x45,0x26,0x19,0xE9,0x56,0xB5,0x41,0x4F,0x60,0x03,0xC5,
	0xF7,0x25,0x8E,0xD1,0xBA,0x1C,0x5E,0x0E,0x5B,0x89,0x5D,0x35,0xD2,0x72,0x69,0x7A,
	0x58,0x09,0x4A,0x6C,0x15,0x1D,0xA1,0x55,0xF0,0xBC,0x0F,0xB8,0xEA,0xBD,0x63,0xDF,
	0x60,0x86,0xCF,0x5D,0x08,0x84,0xA4,0x1A,0xC8,0x37,0x89,0xA7,0xD8,0x20,0x90,0x98,
	0xBD,0x12,0xD2,0xCE,0xFC,0x3B,0x11,0x76,0xDD,0xAE,0xD4,0x8B,0xDF,0xFE,0x22,0x02,
	0x4A,0x00,0x73,0x65,0xED,0x0B,0xC6,0x41,0xE7,0x83,0x71,0xED,0x9D,0x46,0xA3,0x4A,
	0xD9,0x66,0xEB,0x0F,0x3F,0x1E,0x1D,0xA8,0x31,0xD2,0x36,0xB3,0xB8,0xEF,0xEE,0x65,
	0xED,0x82,0x0F,0x26,0xE7,0x25,0xCA,0x92,0x2B,0xCE,0x31,0xE4,0x11,0x12,0x93,0xE0,
	0x87,0x93,0x7C,0x86,0x37,0x5C,0xB5,0xAF,0x2C,0xCC,0x0D,0x24,0xF3,0xB9,0x1D,0x93,
	0x6A,0x91,0xA2,0xCB,0x9D,0xA6,0x2F,0x15,0x0A,0xE6,0x47,0x6A,0xB8,0x10,0x84,0x77,
	0x13,0x73,0x77,0xFB,0xE3,0x9E,0xCD,0x5E,0xCE,0x59,0xE3,0xEF,0x20,0x65,0xE9,0x6E,
	0x7D,0xAE,0x8A,0x96,0x69,0xB1,0xE9,0xA8,0xF4,0x53,0x38,0xE6,0xF6,0xB9,0x63,0x2F,
	0x66,0xE2,0x95,0xBD,0x9A,0xC3,0x3B,0xA1,0x93,0x27,0x0D,0x03,0x0B,0xB1,0x8F,0x3D,
	0x7F,0x20,0x98,0x53,0xE3,0x42,0xE3,0xFC,0xA2,0xD5,0xEC,0x6A,0x89,0x39,0x35,0xCE,
	0x16,0x52,0x97,0xCC,0x85,0x91,0x98,0x70,0xF9,0xFC,0xFB,0xCD,0x7E,0x75,0x01,0x4A,
	0xE7,0x4A,0x71,0x2F,0x6C,0xC0,0x9E,0x13,0xB6,0x74,0xC3,0xB5,0x26,0x9A,0x80,0x70,
	0xB7,0x3A,0xCD,0x75,0x0D,0xFE,0x0F,0xBF,0xF4,0x40,0x85,0xD2,0x30,0xEF,0xEA,0x8B,
	0x88,0x4E,0x75,0xE6,0xD1,0xF2,0x5C,0x30,0x8D,0x98,0xC2,0x7C,0x74,0x91,0xC9,0x9F,
	0xA9,0x8F,0x9D,0xD4,0xAC,0xF2,0xC0,0x63,0x90,0x19,0x84,0xFA,0x92,0xDD,0x9C,0x1A,
	0xCC,0xC9,0xC4,0xC7,0xD7,0x01,0xC6,0xA9,0x15,0x5F,0x1A,0x56,0x9A,0xFC,0x2C,0xCE,
	0x7E,0x1A,0xF3,0xE2,0x14,0x5B,0x0F,0x32,0x61,0x76,0xAC,0xD3,0xB3,0x19,0x26,0x68,
	0xA4,0x67,0xB8,0x8A,0x43,0xB4,0x2E,0x3B,0x24,0x61,0x37,0xCB,0xD6,0x18,0x96,0xCE,
	0x04,0x81,0x2F,0xB4,0x23,0xF7,0x79,0xAB,0x71,0x1D,0x7E,0x10,0x0C,0x9D,0xEF,0x43,
	0x04,0x63,0xF7,0x7E,0xD9,0x14,0x2E,0xBB,0xA5,0x31,0x59,0x24,0xB1,0xDA,0x3D,0xC2,
	0x76,0xE7,0xFD,0x68,0xBE,0xDF,0x2F,0x49,0x71,0x35,0x53,0x01,0xDC,0x91,0x07,0x4B,
	0xA9,0x2C,0x6B,0x47,0xEE,0xD9,0x63,0x2C,0x61,0x1D,0x9C,0x41,0x69,0x31,0x1E,0x40,
	0x4C,0x7F,0xFC,0x1F,0x2A,0x0F,0x68,0x39,0x06,0x48,0xF4,0xE8,0xDC,0xCB,0x45,0x2F,
	0x08,0x76,0x7A,0x6A,0x02,0x20,0xF4,0x79,0xF4,0x65,0x1B,0x26,0xD2,0x03,0x1E,0x12,
	0xA5,0x48,0xF7,0x07,0x20,0x34,0xFB,0xC6,0xC4,0x8C,0x17,0x0D,0x27,0x0A,0x56,0x0F,
	0x43,0xBE,0xE0,0x7F,0x54,0x50,0x3C,0x00,0x40,0xD5,0xB5,0x06,0x4E,0xB6,0x33,0x76,
	0x78,0x96,0xE0,0x45,0xD6,0x6E,0x22,0xEB,0x08,0xC3,0x15,0xED,0xE6,0x5B,0xCF,0x1C,
	0x43,0xF1,0x2E,0xF1,0x02,0x83,0x08,0x82,0x06,0x57,0x29,0xD9,0x5F,0x33,0x37,0xE3,
	0xF9,0x32,0xA2,0xF6,0x2F,0xD8,0xC8,0x78,0x0A,0xB9,0x5D,0x36,0x47,0x4C,0xD8,0x13,
	0xF5,0x3A,0x59,0xCF,0x3F,0xD5,0xF2,0x95,0x58,0xD5,0x2A,0x90,0xAC,0x33,0x11,0xBD,
	0xF1,0x76,0x0C,0x62,0xBA,0x81,0xB4,0x21,0xAA,0x06,0x69,0x63,0x9D,0x96,0x4A,0x96,
	0x4B,0x12,0x26,0x99,0xC5,0x52,0xE2,0x1F,0xF5,0xE2,0xA8,0x9D,0x97,0x62,0x74,0x55,
	0xA9,0x04,0xCE,0x0E,0xA1,0xCD,0x53,0xC6,0x81,0x2E,0x31,0x00,0x99,0x6A,0x78,0xFE,
	0xD9,0x01,0x91,0x0E,0x36,0xFC,0x40,0x52,0x7F,0xA2,0x11,0xE6,0x85,0x64,0x6B,0x28,
	0x64,0xCF,0xD4,0xCA,0x0F,0xED,0x46,0x4F,0x01,0x17,0x16,0x87,0x8E,0x4A,0xC6,0x35,
	0x36,0x45,0xDA,0xF4,0xA9,0x63,0x53,0x79,0xF4,0x70,0x71,0x96,0x4F,0x0F,0x27,0x7A,
	0x9C,0x50,0x7E,0x24,0xEA,0xDB,0x62,0x0C,0x0F,0xC8,0x23,0x7C,0x73,0xC3,0x51,0x4C,
	0xFA,0x33,0xE3,0x44,0x8B,0x5F,0x9E,0x63,0x31,0xD8,0x94,0xD6,0x13,0xF2,0x15,0x8C,
	0xE5,0x1F,0x4E,0x01,0x03,0xC0,0x50,0x6C,0x32,0xE1,0x2F,0x85,0x47,0x32,0x97,0xC3,
	0x22,0x8A,0xCB,0x8B,0x43,0x52,0x64,0xDE,0x33,0x19,0xF2,0x92,0x5E,0x1A,0xD6,0xE6,
	0x93,0x43,0x67,0x53,0x1D,0x4D,0xEE,0xAC,0x9A,0xEA,0xE7,0xD1,0xA3,0xB1,0x30,0x97,
	0x78,0x83,0x44,0x6C,0x81,0xA4,0x5A,0xA5,0xFC,0x72,0x0D,0xFD,0x77,0x5F,0xC6,0xE9,
	0x53,0xDC,0xC5,0x4C,0xD5,0xC2,0xF2,0x1B,0x3C,0x44,0xEC,0xEA,0x6D,0x39,0x34,0xC4,
	0xB5,0xF7,0x6F,0x28,0x04,0x01,0x2C,0x22,0x98,0xFF,0x18,0x5B,0x89,0x04,0x0C,0x67,
	0xD3,0xB5,0x27,0xD6,0x65,0xFB,0x25,0x00,0x21,0x0D,0x03,0xC5,0x5E,0x02,0x5A,0xB3,
	0x82,0xAD,0x56,0xDA,0x13,0x1C,0xFF,0x15,0x75,0x2A,0x27,0x84,0xD4,0xB0,0x7B,0x08,
	0x55,0xF6,0x13,0x70,0xED,0x8B,0x25,0xD2,0x9A,0x56,0x14,0xB5,0x31,0x2B,0xE8,0xCE,
	0x8D,0x34,0x81,0xFC,0xDC,0x3F,0x82,0xE2,0x0F,0x10,0xCB,0x7E,0x1F,0x15,0x03,0x1F,
	0x46,0xA1,0x53,0x8E,0xA1,0x91,0x47,0x45,0x0C,0x13,0x1F,0xCE,0x82,0x3E,0xCA,0x9F,
	0x2D,0xD6,0x81,0x6E,0xB8,0x1A,0x78,0x90,0x28,0x7A,0x47,0x21,0x42,0x75,0xC2,0x2C,
	0x8B,0x87,0x27,0xA3,0xD4,0x65,0x52,0x1B,0xF5,0x29,0xA1,0xF8,0x4D,0x72,0xC1,0x4F,
	0x77,0x3A,0x5B,0x4C,0x7F,0xC3,0xC1,0xDF,0x2B,0xAC,0xB2,0x52,0x19,0x7F,0x16,0x77,
	0xD2,0xAD,0xD5,0x0B,0x4F,0x70,0x99,0x77,0x4F,0xF2,0x80,0xB2,0x06,0x4B,0x63,0x9E,
	0xA5,0xF6,0x10,0x40,0x84,0x49,0xBE,0x4A,0xCA,0x8F,0x12,0xA0,0xAC,0x5E,0xE0,0xF5,
	0xD5,0xBA,0x58,0xB8,0x0E,0xE1,0x11,0x13,0xD6,0xDE,0xCF,0x89,0x2E,0x70,0xC5,0x8D,
	0xC5,0x0C,0xCE,0x8C,0x4E,0x23,0x8B,0x3E,0xC4,0xC4,0x3B,0x85,0xCE,0x7E,0x75,0x45,
	0x23,0x8E,0x82,0x57,0xE0,0xDB,0x35,0x43,0xEF,0x2F,0xA4,0xA9,0x30,0x8E,0xF5,0x4F,
	0x94,0xDC,0x06,0xA8,0xDF,0x54,0x2D,0x26,0xC8,0x18,0xDD,0xA8,0x61,0x99,0x8C,0x8B,
	0x15,0x66,0xBC,0xC0,0x8D,0x1C,0xBE,0x01,0xE6,0x55,0x8F,0x85,0x2C,0x28,0x9F,0x50,
	0xD6,0xD4,0x27,0x2C,0x94,0xA3,0x46,0x13,0x4B,0x18,0x45,0xDA,0xCC,0xA7,0xAB,0xFE,
	0x01,0xB2,0xD9,0xBC,0x65,0x7B,0xC7,0xC8,0x87,0x13,0xF3,0x1D,0x3A,0x7D,0xDF,0x9D,
	0x7B,0x23,0x8A,0x77,0x1B,0x6C,0x80,0x7C,0x05,0xC6,0x02,0x3E,0x6E,0xCB,0xF8,0xE0,
	0x0A,0x56,0x0C,0x45,0x58,0x66,0x0F,0x5B,0x17,0xCB,0xEF,0xD1,0xCE,0x5D,0x2A,0x99,
	0xBF,0x13,0x95,0x98,0xCF,0x60,0xB8,0xA4,0x03,0x90,0x0F,0xA4,0x5C,0xA5,0xD6,0x47,
	0x6F,0x45,0xA9,0xA0,0x11,0x86,0xA3,0xAC,0x4A,0xB3,0xB1,0x69,0x2B,0xD4,0x3D,0x1D,
	0xE6,0x54,0xD7,0x9C,0x9E,0x23,0x60,0xA3,0x4E,0xE4,0x24,0x1F,0x90,0xEA,0x87,0xDC,
	0xD0,0x28,0xD9,0xD8,0x2B,0x48,0xC2,0xD9,0xF0,0xF4,0x0E,0x2C,0xAD,0x01,0xC9,0xCB,
	0x17,0x44,0x63,0x4D,0x8C,0xCC,0x17,0x4E,0x80,0x8D,0xAE,0x69,0x20,0xCC,0xC6,0xE1,
	0x8D,0xE9,0xEF,0x2E,0x28,0xA3,0xD3,0x7D,0xEC,0xF7,0x03,0x94,0xA9,0xF4,0x34,0xFB,
	0xAA,0x91,0x29,0x60,0xCE,0xA9,0x9F,0xC0,0xDE,0xA3,0x3A,0x68,0xFC,0x0D,0x35,0x64,
	0xFA,0xD3,0xF7,0xC2,0x3A,0xEF,0x8D,0xD5,0x5C,0xFC,0x65,0x9E,0x95,0xA1,0x02,0x7F,
	0x69,0x32,0x76,0x5D,0xAB,0x8E,0x2D,0x22,0x35,0x60,0x9B,0xF7,0x91,0x72,0xE0,0x96,
	0x80,0x04,0x12,0x27,0xAC,0x29,0xE4,0xC3,0x12,0xD2,0xE4,0xDE,0x29,0x9C,0xD6,0x56,
	0xCF,0x77,0x4A,0x11,0xA8,0x93,0x0A,0x33,0xA2,0x42,0xDB,0xCE,0x93,0x47,0xCA,0xAD,
	0xC5,0xB7,0x90,0x85,0x0C,0x42,0x25,0x2C,0xD0,0x15,0xBC,0xE6,0x1F,0x2C,0x64,0xA8,
	0x23,0xE8,0x87,0x37,0x28,0xA7,0x61,0xDC,0x71,0xAA,0x40,0x9C,0xEC,0xBE,0xC3,0x8A,
	0x0D,0x87,0xEB,0x37,0x40,0x14,0xF1,0x78,0x09,0xC0,0xF7,0xA4,0xDD,0xB7,0xB1,0xCD,
	0x7C,0x59,0xF3,0x7C,0x07,0x74,0x3A,0xF8,0x51,0xB1,0x5A,0x08,0x55,0xB7,0x1F,0xB8,
	0xAB,0x5D,0x78,0x2C,0xDB,0xA5,0x80,0x4B,0x26,0x8D,0x41,0x89,0x7E,0x3C,0xBF,0xED,
	0x68,0x23,0x30,0x82,0xF5,0xA2,0x2D,0xC5,0x1C,0x67,0x91,0x34,0x88,0x9C,0xD2,0xA3,
	0x63,0xFC,0xD7,0x2D,0x12,0x78,0xA8,0x34,0x2D,0x2B,0xBB,0x73,0x60,0xBF,0x62,0x68,
	0xD9,0x22,0x5B,0xB1,0x52,0x2D,0x4E,0x90,0xB8,0x9F,0xE0,0xE7,0xFD,0xE5,0x9F,0x6F,
	0x3B,0x9E,0x56,0x74,0x2F,0xD8,0x2D,0x84,0x01,0xAE,0xA2,0xBE,0x25,0x59,0x33,0xBE,
	0x55,0x98,0xF6,0x6C,0xC6,0x3C,0x2A,0x78,0xCA,0x1C,0x39,0x72,0x84,0x87,0x51,0xC2,
	0x89,0xCA,0xC7,0x7A,0x50,0xD6,0xCD,0x60,0x03,0x0C,0x87,0xE7,0x7D,0x35,0xE8,0x12,
	0xFB,0x52,0x86,0x9C,0xA9,0xE2,0x41,0x37,0x88,0x7D,0x3E,0x4D,0xA5,0x7B,0x35,0x75,
	0x5A,0x5E,0x29,0x4F,0x20,0xC7,0x25,0x0D,0x21,0xEB,0x08,0xE6,0x45,0x64,0x32,0xDE,
	0x7A,0xAD,0x5E,0x71,0xA4,0xCD,0x9B,0x7C,0xBC,0x31,0xA6,0x70,0xA0,0xDB,0xC2,0x60,
	0xAC,0x64,0xD5,0x8D,0x52,0xB5,0x19,0x8C,0x4A,0xA8,0x50,0x65,0xE5,0xD4,0x97,0x3C,
	0xA7,0x27,0x11,0xA4,0x80,0x96,0xAE,0x59,0x84,0xAC,0x31,0xD2,0x4E,0xA9,0xB6,0xF5,
	0xA6,0x11,0x2A,0x3E,0xA3,0xE3,0x02,0xD3,0xE8,0x99,0xBD,0xD4,0x25,0x9F,0x09,0x87,
	0xB2,0x97,0xB3,0xAD,0x26,0xCA,0xEC,0xBC,0x3B,0x46,0xC5,0x76,0x9C,0x01,0x7B,0xBD,
	0xCE,0x43,0x1B,0x9F,0x12,0x3F,0xEE,0xB4,0xD5,0x29,0x01,0x5F,0x09,0xE7,0x32,0x19,
	0x54,0x83,0xF4,0x9F,0xFC,0x6C,0xA8,0x2D,0xDF,0xAC,0xD7,0x3D,0x23,0x4C,0xAA,0xE4,
	0x36,0x52,0xBF,0x3D,0x1B,0x71,0x83,0xB5,0xD4,0x69,0x9E,0x6D,0x45,0x5A,0x0D,0xF3,
	0x14,0x69,0xB5,0x8F,0x13,0xE3,0x58,0xEC,0x78,0xC2,0x74,0x4D,0xDA,0x27,0x9C,0xF1,
	0x80,0xB2,0x9A,0x31,0x32,0x99,0x76,0xB0,0xD3,0xD8,0xC0,0x4A,0xE0,0xDB,0xB0,0xF7,
	0x1E,0x8A,0x36,0x65,0x70,0x58,0x1F,0x85,0x5A,0xBB,0x7B,0xEF,0x6F,0x9F,0x2D,0x39,
	0xCC,0x65,0xEF,0xCD,0xC8,0x89,0x48,0xA5,0x3A,0x16,0x68,0x51,0x2B,0x4A,0xD0,0xA8,
	0xC2,0x8A,0xDD,0x27,0x1E,0xE7,0x5C,0x73,0x20,0xB1,0x3A,0x1D,0xC6,0x92,0x9B,0x06,
	0x91,0x45,0xE2,0x4A,0xAD,0xB5,0x71,0xAB,0xE9,0xBB,0xB2,0x4A,0xEE,0x77,0xD3,0xFA,
	0x4E,0x49,0x31,0x04,0xDA,0x46,0x62,0x0A,0x0F,0x10,0xB2,0xEC,0x92,0x14,0x67,0xF0,
	0xEF,0x25,0xB1,0x74,0x8B,0x9E,0xAF,0xAA,0xBB,0xB7,0x9F,0x06,0x66,0x21,0x18,0xB7,
	0x47,0xBC,0xF6,0x2C,0x12,0x26,0x84,0x37,0xD8,0xD6,0x19,0x69,0xD4,0x45,0xB2,0x80,
	0xA8,0xDD,0x8A,0xB7,0x0E,0x05,0x58,0x0C,0x3D,0x60,0x91,0x66,0x0A,0x73,0xA6,0xBB,
	0xDB,0x66,0x1D,0x82,0x0D,0x5C,0xC3,0xC4,0x81,0xE8,0x43,0x90,0x72,0x15,0x04,0xDA,
	0xED,0x13,0x01,0x5C,0x97,0xD7,0x3E,0xE0,0x98,0x12,0xAC,0x51,0xB3,0xE8,0x3C,0x16,
	0xEE,0xE1,0x2F,0x95,0xFA,0x6F,0x3B,0x5A,0xD5,0xD1,0x61,0xC8,0x35,0x8D,0xA7,0x8A,
	0x24,0xD8,0xDB,0xDE,0xC8,0xD2,0x76,0xE8,0x7A,0xDE,0xEB,0xE1,0xCA,0x9F,0x76,0x34,
	0xFF,0xD7,0xB8,0x3E,0x20,0xA8,0xB9,0x0C,0x7B,0xAF,0x6E,0xEA,0xEC,0x93,0x2F,0xAE,
	0x35,0x00,0x34,0x97,0xCA,0x57,0x4D,0xFF,0x5F,0x77,0xDA,0xA7,0x18,0x00,0xA7,0xDB,
	0x64,0xB6,0xEB,0x9E,0x97,0x3A,0xAD,0x9C,0x56,0x3C,0xAE,0xBD,0x61,0x88,0xBF,0xEA,
	0x23,0xE4,0x0C,0x89,0x15,0xFC,0x7D,0x18,0xEE,0xE9,0x9C,0x93,0xCE,0xE3,0xC4,0x45,
	0x48,0x44,0x43,0x50,0x32,0x2E,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x56,0x45,0x52,0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x7B,0x14,0xD8,
};
unsigned char hdcp2_raw_key[2048] = {0};
#endif

int rtk_hal_hdcp2_WriteKeySet(unsigned char *pKeySet, unsigned char *dst_file_path, int keyLength)
{
#if 0
	int rc = NOT_OK;
	if(pKeySet == NULL || dst_file_path == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_WriteKeySet input paramter is error!\n");
		return NOT_OK;
	}
	if(keyLength != KEYSET_SIZE)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_WriteKeySet keylength is error!\n");
		return NOT_OK;
	}

	rc = hdcp2_save_encrypted_keyset(pKeySet, dst_file_path, keyLength);
	return rc;
#endif
#ifndef CONFIG_OPTEE_HDCP2

	int rc = NOT_OK;
	if(pKeySet == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_WriteKeySet input paramter is error!\n");
		return NOT_OK;
	}
	rc = hdcp2_save_keyset(pKeySet);

	hdcp2_save_keyset_file(pKeySet,dst_file_path,keyLength);
#endif
	

	return OK;
}

int rtk_hal_hdcp2_GetEncHdcp2Key(unsigned char *pEncKey, unsigned char *src_file_path, unsigned char *keyLen)
{
#if 0
	if(pEncKey == NULL || src_file_path == NULL || keyLen == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_GetEncHdcp2Key input paramter is error!\n");
		return NOT_OK;
	}

	read_binary_file(src_file_path, pEncKey, KEYSET_SIZE);
#endif
	return OK;
}

int rtk_hal_hdcp2_SetEncHdcp2Key(unsigned char *pEncKey)
{
#if 0
	if(pEncKey == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_GetEncHdcp2Key input paramter is error!\n");
		return NOT_OK;
	}

	write_binary_file(src_file_path, pEncKey, KEYSET_SIZE);
#endif
	return OK;


}

int rtk_hal_hdcp2_GetRootPublicKey(unsigned char *pRootPublicKey, unsigned char *src_file_path)
{
#if 0
	int rc = NOT_OK;

	if(pRootPublicKey == NULL || src_file_path == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_GetRootPublicKey input paramter is error!\n");
		return NOT_OK;
	}

	rc = hdcp2_load_encrypted_keyset(pRootPublicKey, src_file_path, DCP_LLC_KEY_SIZE+15);
	return rc;
#endif
	return OK;
}

int rtk_hal_hdcp2_GetCertInfo(unsigned char *pReceiverID, unsigned char *pPubKey, unsigned char *pRootSign)
{
#ifndef CONFIG_OPTEE_HDCP2
	unsigned char certInfo[CERT_RX_SIZE] = {0x00};
	if(pReceiverID == NULL || pPubKey == NULL || pRootSign == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_GetCertInfo input paramter is error!\n");
		return NOT_OK;
	}

	hdcp2_Rx_GetCertRx(certInfo, CERT_RX_SIZE);
	memcpy(pReceiverID, certInfo, RECEIVER_ID_SIZE);
	memcpy(pPubKey, &certInfo[RECEIVER_ID_SIZE], KEY_PUBLIC_RX_SIZE);
	//memcpy(pReserved, &certInfo[RECEIVER_ID_SIZE+KEY_PUBLIC_RX_SIZE], RESERVED_SIZE);
	memcpy(pRootSign, &certInfo[RECEIVER_ID_SIZE+KEY_PUBLIC_RX_SIZE+RESERVED_SIZE], DCP_LLC_SIG_SIZE);
#endif
	return OK;


}

int rtk_hal_hdcp2_GetCertInfo2(unsigned char *pReceiverID, unsigned char *pPubKey, unsigned char *pReserved, unsigned char *pRootSign)
{
#ifndef CONFIG_OPTEE_HDCP2
	unsigned char certInfo[CERT_RX_SIZE] = {0x00};
	if(pReceiverID == NULL || pPubKey == NULL || pRootSign == NULL || pReserved == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_GetCertInfo2 input paramter is error!\n");
		return NOT_OK;
	}

	hdcp2_Rx_GetCertRx(certInfo, CERT_RX_SIZE);
	memcpy(pReceiverID, certInfo, RECEIVER_ID_SIZE);
	memcpy(pPubKey, &certInfo[RECEIVER_ID_SIZE], KEY_PUBLIC_RX_SIZE);
	memcpy(pReserved, &certInfo[RECEIVER_ID_SIZE+KEY_PUBLIC_RX_SIZE], RESERVED_SIZE);
	memcpy(pRootSign, &certInfo[RECEIVER_ID_SIZE+KEY_PUBLIC_RX_SIZE+RESERVED_SIZE], DCP_LLC_SIG_SIZE);
#endif
	return OK;
}

int rtk_hal_hdcp2_Decrypt_RSAES_OAEP(unsigned char *pEKpubKm)
{
#ifndef CONFIG_OPTEE_HDCP2
	unsigned char  Km[KM_SIZE] = {0x00};
	unsigned char  KPriv_tmp[KPRIVRX_SIZE] = {0x00};
       H2status rc;

	if(pEKpubKm == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Decrypt_RSAES_OAEP input paramter is error!\n");
		return NOT_OK;
	}

	spu_GetKPrivRx(KPriv_tmp);
	//HDCP2DBGLOG("KPriv_tmp: ", KPriv_tmp, KPRIVRX_SIZE);
	//HDCP2DBGLOG("pEKpubKm: ", pEKpubKm, EKPUBKM_SIZE);
    	rc = Decrypt_EKpubKm_kPrivRx(KPriv_tmp, Km, pEKpubKm);
	HDCP2DBGLOG("km: ", Km, KM_SIZE);
	spu_SetKM(Km);
	RTK_RCP_SET_CW_HDCP2(SRAM_KM_ENTRY , Km, KM_SIZE);

	return rc;
#else
	return OK;
#endif

}

int rtk_hal_hdcp2_Kd_Key_Derivation(unsigned char *pRtx, unsigned char *pRrx, unsigned int version)
{
#ifndef CONFIG_OPTEE_HDCP2
	int modeHDCP22 = 0;
	unsigned char rn[RN_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char kd[KD_SIZE] = {0x00};
	//unsigned char km[KM_SIZE];

	if(pRtx == NULL || pRrx == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Kd_Key_Derivation input paramter is error!\n");
		return NOT_OK;
	}
#if 0
	if(version !=0 && version != 1 && version != 2)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Kd_Key_Derivation input version is error!\n");
		return NOT_OK;
	}
#endif
	if(version >= 2)
		modeHDCP22 = 1;
	else
		modeHDCP22 = 0;
#if 0 //for test
	spu_GetKM(km);
	HDCP2DBGLOG("km: ", km, KM_SIZE);
	HDCP2DBGLOG("pRtx: ", pRtx, RTX_SIZE);
	HDCP2DBGLOG("pRrx: ", pRrx, RRX_SIZE);
	pr_debug("version = %d\n", modeHDCP22);
#endif
	RCP_HDCP2_GenKd(SRAM_KM_ENTRY, pRtx, pRrx, rn, SRAM_KD_ENTRY,modeHDCP22);
	RTK_RCP_GET_CW_HDCP2(SRAM_KD_ENTRY , kd, KD_SIZE);
	spu_SetKD(kd);
	HDCP2DBGLOG("kD: ", kd, KD_SIZE);
#endif
	return OK;
}


int rtk_hal_hdcp2_Compute_H_Prime(unsigned char *pHPrime, unsigned char repeater, unsigned char *pRtx, unsigned char *deviceOptionInfo, unsigned int version)
{
#ifndef CONFIG_OPTEE_HDCP2
	unsigned char  Kd[KD_SIZE] = {0x00};
	unsigned char  txcaps[TXCAPS_SIZE] = {0x00};
	unsigned char  rxcaps[RXCAPS_SIZE] = {0x00};
	H2status rc;

	if(pHPrime == NULL || pRtx == NULL || deviceOptionInfo == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Compute_H_Prime input paramter is error!\n");
		return NOT_OK;
	}
#if 0
	if(version !=0 && version != 1 && version != 2)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Compute_H_Prime input version is error!\n");
		return NOT_OK;
	}
#endif
	if(repeater !=0 && repeater != 1)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Compute_H_Prime input repeater is error!\n");
		return NOT_OK;
	}

	spu_GetKD(Kd);

	hdcp2_Rx_Setdevinfo(deviceOptionInfo);
	memcpy(rxcaps, deviceOptionInfo, RXCAPS_SIZE);
	memcpy(txcaps, deviceOptionInfo+RXCAPS_SIZE, TXCAPS_SIZE);
#if 0 //for test
	HDCP2DBGLOG("kd: ", Kd, KD_SIZE);
	HDCP2DBGLOG("pRtx: ", pRtx, RTX_SIZE);
	HDCP2DBGLOG("txcaps: ", txcaps, TXCAPS_SIZE);
	HDCP2DBGLOG("rxcaps: ", rxcaps, RXCAPS_SIZE);
#endif
	if(version >= 2)
		rc = Compute_Hprime_22_widi(Kd, pRtx, txcaps, rxcaps, repeater, pHPrime);
	else
		rc = Compute_Hprime_Rx(Kd, pRtx, repeater, pHPrime);

	return rc;
#else
	return OK;
#endif
}

int rtk_hal_hdcp2_Compute_L_Prime(unsigned char *pLPrime, unsigned char *pRn, unsigned char *pRrx, unsigned int version)
{
#ifndef CONFIG_OPTEE_HDCP2
	unsigned char  Kd[KD_SIZE] = {0x00};
	H2status rc;

	if(pLPrime == NULL || pRrx == NULL || pRn == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Compute_L_Prime input paramter is error!\n");
		return NOT_OK;
	}
#if 0
	if(version !=0 && version != 1 && version != 2)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Compute_L_Prime input version is error!\n");
		return NOT_OK;
	}
#endif
	spu_GetKD(Kd);
#if 0 //for test
	HDCP2DBGLOG("kd: ", Kd, KD_SIZE);
	HDCP2DBGLOG("pRrx: ", pRrx, RRX_SIZE);
	HDCP2DBGLOG("pRn: ", pRn, RN_SIZE);
#endif
	rc = Compute_Lprime_Rx_widi(Kd, pRrx, pRn, pLPrime);

	return rc;
#else
	return OK;
#endif
}

int rtk_hal_hdcp2_Compute_Kh(void)
{
#ifndef CONFIG_OPTEE_HDCP2
	unsigned char  KPriv_tmp[KPRIVRX_SIZE] = {0x00};
	unsigned char  Kh[KH_SIZE] = {0x00};
	H2status rc;

	spu_GetKPrivRx(KPriv_tmp);
	rc = Compute_Kh (KPriv_tmp, KPRIVRX_SIZE, Kh);

	HDCP2DBGLOG("kh: ", Kh, KH_SIZE);

	return rc;
#else
	return OK;
#endif

}

int rtk_hal_hdcp2_Encrypt_Km_using_Kh(unsigned char *pEKh_Km, unsigned char *pM)
{
#ifndef CONFIG_OPTEE_HDCP2
	H2status rc;

	if(pEKh_Km == NULL || pM == NULL )
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Encrypt_Km_using_Kh input paramter is error!\n");
		return NOT_OK;
	}

	rc = hdcp2_Rx_Compute_EKhKm(pM, pEKh_Km);

	return rc;
#else
	return OK;
#endif

}

int rtk_hal_hdcp2_Decrypt_Km_using_Kh(unsigned char *pM, unsigned char *pEkh_Km)
{
#ifndef CONFIG_OPTEE_HDCP2
	H2status rc;
	unsigned char km[KM_SIZE] = {0x00};

	if(pEkh_Km == NULL || pM == NULL )
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Decrypt_Km_using_Kh input paramter is error!\n");
		return NOT_OK;
	}

	rc = hdcp2_Rx_ParseEKhKm(pEkh_Km,  pM);
	spu_GetKM(km);
	HDCP2DBGLOG("kM: ", km, KM_SIZE);

	return rc;
#else
	return OK;
#endif

}

int rtk_hal_hdcp2_Decrypt_EKs(unsigned char *pEKs, unsigned char *pRtx, unsigned char *pRrx, unsigned char *pRn, unsigned int version)
{
#ifndef CONFIG_OPTEE_HDCP2
	H2status rc;
	int modeHDCP22 = 0;
	unsigned char ks[KS_SIZE] = {0x00};

	if(pEKs == NULL || pRtx == NULL || pRrx == NULL || pRn == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Decrypt_EKs input paramter is error!\n");
		return NOT_OK;
	}
#if 0
	if(version !=0 && version != 1 && version != 2)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Decrypt_EKs input version is error!\n");
		return NOT_OK;
	}
#endif
	if(version >= 2)
		modeHDCP22 = 1;
	else
		modeHDCP22 = 0;

	rc = hdcp2_Rx_Compute_EdKeyKs(pRtx, pRrx, pRn, pEKs, modeHDCP22);

	hdcp2_Rx_GetKs(ks, KS_SIZE);
	HDCP2DBGLOG("ks: ", ks, KS_SIZE);
	return rc;
#else
	return OK;
#endif
}

int rtk_hal_hdcp2_XOR_Ks_with_LC128(void)
{
#ifndef CONFIG_OPTEE_HDCP2
	unsigned char KsxorLc128[16] = {0x00};
	down(&HDCP2_Semaphore);

	 hdcp2_GenKsXorLc128();
	 spu_GetKsXorLc128(KsxorLc128);
	 HDCP2DBGLOG("KsxorLc128: ", KsxorLc128, 16);

	 up(&HDCP2_Semaphore);
#endif
	return OK;
}

int rtk_hal_hdcp2_Generate_Km(unsigned char *pMaskedDB, unsigned char *pDB, unsigned char *pDBMask)
{
#ifndef CONFIG_OPTEE_HDCP2
	unsigned char km[16] = {0};
	unsigned char temp_DB[DB_SIZE];
	if(pMaskedDB == NULL || pDB == NULL || pDBMask == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Generate_Km input paramter is error!\n");
		return NOT_OK;
	}
	memset(pMaskedDB, 0, DB_SIZE+4);
	spu_GetKM(km);
	memcpy(temp_DB, pDB, DB_SIZE-KM_SIZE);
	memcpy(temp_DB+DB_SIZE-KM_SIZE,km,KM_SIZE);
	
	crypt_xor(temp_DB, pDBMask, pMaskedDB, DB_SIZE);
#endif
	return OK;
}

int rtk_hal_hdcp2_Generate_Ks(unsigned char *pEks, unsigned char *pRrx, unsigned char *pRtx, unsigned char *pRn, unsigned int version)
{
#ifndef CONFIG_OPTEE_HDCP2
	H2status rc;
	int modeHDCP22;

	if(pEks == NULL || pRrx == NULL || pRtx == NULL|| pRn == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Generate_Ks input paramter is error!\n");
		return NOT_OK;
	}
#if 0
	if(version !=0 && version != 1 && version != 2)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Generate_Ks input version is error!\n");
		return NOT_OK;
	}
#endif
	if(version >= 2)
		modeHDCP22 = 1;
	else
		modeHDCP22 = 0;
#if 0//for test
	HDCP2DBGLOG("pRrx: ", pRrx, RRX_SIZE);
	HDCP2DBGLOG("pRtx: ", pRtx, RTX_SIZE);
	HDCP2DBGLOG("pRn: ", pRn, RN_SIZE);
#endif
	rc = hdcp2_Verify_EdkeyKs(pEks, pRrx, pRtx, pRn, modeHDCP22);
	return rc;
#else
	return OK;
#endif
}

int rtk_hal_hdcp2_Data_Decrypt(unsigned char *pOutputBuf, unsigned char *pInputBuf, int length, unsigned char *pCounter)
{
#ifndef CONFIG_OPTEE_HDCP2
	unsigned char KsxorLc128[16] = {0x00};
	unsigned char DataCrypto[16] = {0x00};
	down(&HDCP2_Semaphore);

	if(pOutputBuf == NULL || pInputBuf == NULL || pCounter == NULL || length == 0)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Data_Decrypt input paramter is error!\n");
		return NOT_OK;
	}

	spu_GetKsXorLc128(KsxorLc128);
	RTK_RCP_AES_CTR_Decryption(KsxorLc128, pCounter, pInputBuf, pOutputBuf, length);
	if(length % 16)
	{
		RCP_HDCP2_DataDecrypt(SRAM_KS_XOR_LC128_ENTRY,  SRAM_DATA_DECRYPT_ENTRY, KsxorLc128, pCounter);
		RTK_RCP_GET_CW_HDCP2(SRAM_DATA_DECRYPT_ENTRY , DataCrypto, SRAM_DATA_DECRYPT_SIZE);
		spu_SetDateDecrypto(DataCrypto);
		crypt_xor(pInputBuf + length-length % 16 , DataCrypto, pOutputBuf+ length-length % 16, length%16);
	}

	/*HDCP2DBGLOG("pOutputBuf: ", pOutputBuf, length);*/

	up(&HDCP2_Semaphore);
#endif
	return OK;
}


int rtk_hal_hdcp2_SHA1(unsigned char * pOutputBuf, unsigned char *pInputBuf, int length)
{
#ifndef CONFIG_OPTEE_HDCP2
	H2status rc;

	if(pOutputBuf == NULL || pInputBuf == NULL || length == 0)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_SHA1 input paramter is error!\n");
		return NOT_OK;
	}

	rc = Sha1(pInputBuf, length, pOutputBuf);
	return rc;
#else
	return OK;
#endif
}

int rtk_hal_hdcp2_SHA256(unsigned char * pOutputBuf, unsigned char *pInputBuf, int length)
{
#ifndef CONFIG_OPTEE_HDCP2
	H2status rc;

	if(pOutputBuf == NULL || pInputBuf == NULL || length == 0)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_HMAC_SHA256 input paramter is error!\n");
		return NOT_OK;
	}

	rc = Sha256(pInputBuf, length, pOutputBuf);
	return rc;
#else
		return OK;
#endif

}

int rtk_hal_hdcp2_HMAC_SHA256(unsigned char * pOutputBuf, unsigned char *pInputBuf, int length, unsigned char *pKey, int keylength)
{
#ifndef CONFIG_OPTEE_HDCP2

	H2status rc;

	if(pOutputBuf == NULL || pInputBuf == NULL || pKey == NULL || length == 0 || keylength == 0)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_HMAC_SHA256 input paramter is error!\n");
		return NOT_OK;
	}

#if 0//for test
	HDCP2DBGLOG("pKey: ", pKey, keylength);
	HDCP2DBGLOG("pInputBuf: ", pInputBuf, length);
#endif
	rc = hmacsha256( pKey, keylength, pInputBuf, length, pOutputBuf);
	return rc;
#else
			return OK;
#endif

}

int rtk_hal_hdcp2_AES_CTR(unsigned char *pOutputBuf, unsigned char *pInputBuf, int length, unsigned char *pkey, int keyLength, unsigned char *pCounter)
{
#ifndef CONFIG_OPTEE_HDCP2

	if(pOutputBuf == NULL || pInputBuf == NULL || pkey == NULL || pCounter == NULL || length == 0 || keyLength == 0)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_AES_CTR input paramter is error!\n");
		return NOT_OK;
	}
	RTK_RCP_AES_CTR_Decryption(pkey, pCounter, pInputBuf, pOutputBuf, length);
#endif
	return OK;
}

int rtk_hal_hdcp2_Select_StoredKm(int index)
{
#ifndef CONFIG_OPTEE_HDCP2

	unsigned char km[KM_SIZE] = {0x00};
	if(index < 0 || index > 4)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Select_StoredKm index is error!\n");
		return NOT_OK;
	}
	switch(index)
	{
		case 0:
			/*spu_GetKm0(km);*/
			break;
		case 1:
			/*spu_GetKm1(km);*/
			break;
		case 2:
			/*spu_GetKm2(km);*/
			break;
		case 3:
			/*spu_GetKm3(km);*/
			break;
		case 4:
			/*spu_GetKm4(km);*/
			break;
		default:
			break;
	}

	spu_SetKM(km);
	pr_debug("index = %d\n", index);
	HDCP2DBGLOG("KM: ", km, KM_SIZE);
#endif
	return OK;
}

int rtk_hal_hdcp2_Write_StoredKm(int index)
{
#ifndef CONFIG_OPTEE_HDCP2

	unsigned char km[KM_SIZE] = {0x00};
	if(index < 0 || index > 4)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Write_StoredKm index is error!\n");
		return NOT_OK;
	}
	spu_GetKM(km);
	switch(index)
	{
		case 0:
			/*spu_SetKm0(km);*/
			break;
		case 1:
			/*spu_SetKm1(km);*/
			break;
		case 2:
			/*spu_SetKm2(km);*/
			break;
		case 3:
			/*spu_SetKm3(km);*/
			break;
		case 4:
			/*spu_SetKm4(km);*/
			break;
		default:
			break;
	}
	pr_debug("index = %d\n", index);
	HDCP2DBGLOG("KM: ", km, KM_SIZE);
#endif
	return OK;
}

int rtk_hal_hdcp2_Data_Encrypt(unsigned char *pOutputBuf, unsigned char *pInputBuf, int length, unsigned char *pCounter)
{
#ifndef CONFIG_OPTEE_HDCP2

	unsigned char KsxorLc128[16] = {0x00};
	if(pOutputBuf == NULL || pInputBuf == NULL || pCounter == NULL || length == 0)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Data_Encrypt input paramter is error!\n");
		return NOT_OK;
	}

	spu_GetKsXorLc128(KsxorLc128);
	RTK_RCP_AES_CTR_Decryption(KsxorLc128, pCounter, pInputBuf, pOutputBuf, length);
#endif
	return OK;
}

int rtk_hal_hdcp2_GetProtectedDecryptionKey(unsigned char *pData, int *pLength)
{
#ifndef CONFIG_OPTEE_HDCP2

	down(&HDCP2_Semaphore);

	if(pData == NULL || pLength == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_GetProtectedDecryptionKey input paramter is error!\n");
		return NOT_OK;
	}
	hdcp2_Rx_GetKs(pData, KS_SIZE);
	spu_GetLc128(pData+KS_SIZE);

	up(&HDCP2_Semaphore);
#endif
	return OK;
}

int rtk_hal_hdcp2_SetProtectedDecryptionKey(unsigned char *pData, int length)
{
#ifndef CONFIG_OPTEE_HDCP2

	unsigned char KsxorLc128[16] = {0x00};
	down(&HDCP2_Semaphore);

	if(pData == NULL || length == 0)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_SetProtectedDecryptionKey input paramter is error!\n");
		return NOT_OK;
	}
	hdcp2_Rx_SetKs(pData);
	spu_SetLc128(pData+KS_SIZE);

	 hdcp2_GenKsXorLc128();
	 spu_GetKsXorLc128(KsxorLc128);
	 HDCP2DBGLOG("KsxorLc128: ", KsxorLc128, 16);

	 up(&HDCP2_Semaphore);
#endif
	return OK;
}

int rtk_hal_hdcp2_Compute_V_Prime(unsigned char *pVPrime, unsigned char *pRxIdListsInfo,  int infoLength)
{
#ifndef CONFIG_OPTEE_HDCP2

	unsigned char kd_temp[KD_SIZE] = {0x00};
	H2status rc;
	if(pVPrime == NULL || pRxIdListsInfo == NULL || infoLength == 0)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Compute_V_Prime input paramter is error!\n");
		return NOT_OK;
	}

	spu_GetKD(kd_temp);
	rc = hmacsha256(kd_temp, KD_SIZE, pRxIdListsInfo, infoLength, pVPrime);
	return rc; 
#else
	return OK;
#endif
}

int rtk_hal_hdcp2_Compute_M_Prime(unsigned char *pMPrime, unsigned char *pStreamManageInfo, int infoLength)
{
#ifndef CONFIG_OPTEE_HDCP2

	unsigned char kd_temp[KD_SIZE] = {0x00};
	unsigned char kd_sha256[KD_SIZE] = {0x00};
	H2status rc;
	if(pMPrime == NULL || pStreamManageInfo == NULL || infoLength == 0)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Compute_M_Prime input paramter is error!\n");
		return NOT_OK;
	}
	spu_GetKD(kd_temp);
	Sha256(kd_temp, KD_SIZE, kd_sha256);
	rc = hmacsha256(kd_sha256, KD_SIZE, pStreamManageInfo, infoLength, pMPrime);
	return rc;
#else
		return OK;
#endif

}

int rtk_hal_hdcp2_UseTestVector(unsigned int argc, HDCP2_UseTestVector_local_PARA *vector)
{
#ifndef CONFIG_OPTEE_HDCP2
	if(argc == 0)
	{
		unsigned char temp_lc128[16] = {0};
		unsigned char temp_km[16] = {0};
		unsigned char temp_ks[16] = {0};
		unsigned char temp_seed[32] = {0};
		spu_SetLc128(temp_lc128);
		spu_SetKM(temp_km);
		hdcp2_Rx_Set_UT_Ks(temp_ks);
		hdcp2_Rx_Set_UT_Seed(temp_seed);
		return OK;
	}
	else if(argc == 4)
	{
		if(vector)
		{
			spu_SetLc128(vector->lc128);
			spu_SetKM(vector->km);
			hdcp2_Rx_Set_UT_Ks(vector->ks);
			hdcp2_Rx_Set_UT_Seed(vector->seed);
		}
		return OK;
	}
	else
	{
		return NOT_OK;
	}
#else
			return OK;
#endif

}

int rtk_hal_hdcp2_Encrypt_RSAES_OAEP(unsigned char *pEkpub_km, unsigned char *pKpubrx)
{
#ifndef CONFIG_OPTEE_HDCP2

	H2status rc;
	unsigned char km[KM_SIZE] = {0};
	if(pEkpub_km == NULL || pKpubrx == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_Encrypt_RSAES_OAEP input paramter is error!\n");
		return NOT_OK;
	}
	spu_GetKM(km);
	rc = crypto_rsaPublicEncryptOaepSha256(pKpubrx, pEkpub_km, km);
	return rc;
#else
	return OK;
#endif
}

int rtk_hal_hdcp2_HMAC_SHA256_with_kd (unsigned char *pDest,unsigned char *pSrc, int srcLength)
{
#ifndef CONFIG_OPTEE_HDCP2

	H2status rc;
	unsigned char kd[KD_SIZE] = {0x00};
	if(pDest == NULL || pSrc == NULL || srcLength == 0)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_HMAC_SHA256_with_kd input paramter is error!\n");
		return NOT_OK;
	}
	spu_GetKD(kd);
	rc = hmacsha256(kd, KD_SIZE, pSrc, srcLength, pDest);
	return rc;
	return OK;
#else
	return OK;
#endif
}

int rtk_hal_hdcp_Init(void)
{
	//hdcp2_load_from_flash();
	return OK;
}

int rtk_hal_hdcp2_get_hdcp_status(unsigned char *b14Status, unsigned char *b22Status)
{
#ifndef CONFIG_OPTEE_HDCP2

#if 0//mark by zhaodong k7

	hdmi_stb_p0_hdcp_flag1_RBUS hdcp_flag1_reg; 
	unsigned int nport = get_current_hdmi_port();
	if(b14Status == NULL || b22Status == NULL)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_get_hdcp_status input paramter is error!\n");
		return NOT_OK;
	}
	hdcp_flag1_reg.regValue = IoReg_Read32(HDMI_STB_P0_HDCP_FLAG1_reg);
	*b14Status = hdcp_flag1_reg.adne_flag;
	*b22Status = lib_hdmi_hdcp22_get_auth_done(nport);		
#endif

	return OK;
#else
	return OK;
#endif
}

int rtk_hal_hdcp2_encrypto_key(unsigned char *ENkey, unsigned int ENkey_size, unsigned char *AESkey, unsigned AESkey_size)
{
#ifdef CONFIG_OPTEE_HDCP2
	if(ENkey == NULL || AESkey == NULL || ENkey_size != 2144 || AESkey_size != 1024)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_encrypto_key input paramter is error!\n");
		return NOT_OK;
	}
	optee_hdcp2_main(HDCP2_CMD_ENCRYPTO_KEY, 0, ENkey, ENkey_size, AESkey, AESkey_size);
#endif
	return OK;
}

int rtk_hal_hdcp2_decrypto_key(unsigned char *AESkey, unsigned AESkey_size)
{
#ifdef CONFIG_OPTEE_HDCP2
	unsigned char uc_lc128[16]={0};
	//int j; 
	if(AESkey == NULL || AESkey_size != 1024)
	{
		pr_debug("hdcp2:rtk_hal_hdcp2_decrypto_key input paramter is error!\n");
		return NOT_OK;
	}
	optee_hdcp2_main(HDCP2_CMD_DECRYPTO_KEY, 0, AESkey, AESkey_size,uc_lc128,16);
	HDCP2DBGLOG("uc_lc128: ", uc_lc128, 16);
	
	#if 0
	for (j=0; j<HDMI_PORT_TOTAL_NUM; j++) {
		if (newbase_hdmi_phy_port_status(j) == HDMI_PHY_PORT_NOTUSED)
			continue;
		lib_hdmi_hdcp22_init(j,uc_lc128);
	}
	#endif
#endif
	return OK;
}

#ifdef CONFIG_OPTEE_HDCP2
#define HDMI_HDCP2_KEY_LENGTH 902
#endif
int rtk_hal_hdcp2_DRM_HDMI_Encrypto_key(unsigned char *pdata, unsigned char *pEnckey, unsigned int length)
{
#ifndef CONFIG_OPTEE_HDCP2

#if IS_ENABLED(CONFIG_RTK_KDRV_MCP)
	unsigned char offline_key[16] = {0x95, 0x9D, 0x02, 0x74, 0x23, 0x3F, 0xF1, 
		0xCE, 0xD0, 0x21, 0x03, 0x42, 0x39, 0xDF, 0xC2, 0x2D};
	unsigned char iv[16] = {0};
#endif	
	if(pdata == NULL || pEnckey == NULL || length != HDMI_HDCP2_KEY_LENGTH)
	{
		pr_emerg("hdcp2:rtk_hal_hdcp2_DRM_HDMI_Encrypto_key input paramter is error!\n");
		return NOT_OK;
	}

	if(length % 16)
	{
		length = (length / 16) * 16;
	}
	RTK_RCP_AES_CBC_Encryption(offline_key, iv, pdata, pEnckey, length);
	memcpy(&pEnckey[HDMI_HDCP2_KEY_LENGTH/16*16], &pdata[HDMI_HDCP2_KEY_LENGTH/16*16], HDMI_HDCP2_KEY_LENGTH % 16 );

	return OK;
#else
	
	if(pdata == NULL || pEnckey == NULL || length != HDMI_HDCP2_KEY_LENGTH)
	{
		pr_emerg("hdcp2:rtk_hal_hdcp2_DRM_HDMI_Encrypto_key input paramter is error!\n");
		return NOT_OK;
	}
	optee_hdcp2_main(HDCP2_CMD_HDMI_ENCRYPTO_KEY, 0, pdata, length, pEnckey, length);
	return OK;
#endif
}

int rtk_hal_hdcp2_DRM_HDMI_Decrypto_key(unsigned char *pEnckey, unsigned char *pDeckey, unsigned int length)
{
#ifndef CONFIG_OPTEE_HDCP2

#if IS_ENABLED(CONFIG_RTK_KDRV_MCP)
	unsigned char offline_key[16] = {0x95, 0x9D, 0x02, 0x74, 0x23, 0x3F, 0xF1, 
		0xCE, 0xD0, 0x21, 0x03, 0x42, 0x39, 0xDF, 0xC2, 0x2D};
	unsigned char iv[16] = {0};
#endif

	if(pEnckey == NULL || pDeckey == NULL || length != HDMI_HDCP2_KEY_LENGTH)
	{
		pr_emerg("hdcp2:rtk_hal_hdcp2_DRM_HDMI_Decrypto_key input paramter is error!\n");
		return NOT_OK;
	}

	if(length % 16)
	{
		length = (length / 16) * 16;
	}
	RTK_RCP_AES_CBC_Decryption(offline_key, iv, pEnckey, pDeckey, length);
	memcpy(&pDeckey[HDMI_HDCP2_KEY_LENGTH/16*16], &pEnckey[HDMI_HDCP2_KEY_LENGTH/16*16], HDMI_HDCP2_KEY_LENGTH % 16 );
	return OK;
#else
	if(pEnckey == NULL || pDeckey == NULL || length != HDMI_HDCP2_KEY_LENGTH)
	{
		pr_emerg("hdcp2:rtk_hal_hdcp2_DRM_HDMI_Decrypto_key input paramter is error!\n");
		return NOT_OK;
	}
	optee_hdcp2_main(HDCP2_CMD_HDMI_DECRYPTO_KEY, 0, pEnckey, length,pDeckey,length);
	return OK;
#endif
}
H2status hdcp2_load_from_sstorage(unsigned char *pKeySet);
int rtk_hal_hdcp2_VFE_HDMI_WriteHDCP22(unsigned char *pdata, unsigned int size)
{
#ifndef CONFIG_OPTEE_HDCP2

#if IS_ENABLED(CONFIG_RTK_KDRV_MCP)
	unsigned char offline_key[16] = {0x95, 0x9D, 0x02, 0x74, 0x23, 0x3F, 0xF1, 
		0xCE, 0xD0, 0x21, 0x03, 0x42, 0x39, 0xDF, 0xC2, 0x2D};
	unsigned char iv[16] = {0};
#endif
	unsigned char raw_key[HDMI_HDCP2_KEY_LENGTH] = {0};
	
	if(pdata == NULL || size != HDMI_HDCP2_KEY_LENGTH)
	{
		pr_emerg("hdcp2:rtk_hal_hdcp2_VFE_HDMI_WriteHDCP22 input paramter is error!\n");
		return NOT_OK;
	}
	if(size % 16)
	{
		size = (size / 16) * 16;
	}
	
	RTK_RCP_AES_CBC_Decryption(offline_key, iv, pdata, raw_key, size);
	memcpy(&raw_key[HDMI_HDCP2_KEY_LENGTH/16*16], &pdata[HDMI_HDCP2_KEY_LENGTH/16*16], HDMI_HDCP2_KEY_LENGTH % 16 );

	hdcp2_load_from_sstorage(raw_key);
	return OK;
#else
	unsigned char uc_lc128[16]={0};
	//int j; 

	if(pdata == NULL || size != HDMI_HDCP2_KEY_LENGTH)
	{
		pr_emerg("hdcp2:rtk_hal_hdcp2_VFE_HDMI_WriteHDCP22 input paramter is error!\n");
		return NOT_OK;
	}
	optee_hdcp2_main(HDCP2_CMD_HDMI_WRITE_KEY, 0, pdata, size,uc_lc128,16);
	#if 0
	for (j=0; j<HDMI_PORT_TOTAL_NUM; j++) {
		if (newbase_hdmi_phy_port_status(j) == HDMI_PHY_PORT_NOTUSED)
			continue;
		lib_hdmi_hdcp22_init(j,uc_lc128);
	}
	#endif
	return OK;
#endif
}

unsigned char ksxorlc128[LC128_SIZE];
void rtk_hdcp2_gen_ksxorlc128(unsigned char *data)
{
	unsigned char lc[LC128_SIZE];
	unsigned char ks[KS_SIZE];
	unsigned char ksxorLc[16];

	if(data == NULL)
	{
		pr_emerg("rtk_hdcp2_gen_ksxorlc128 fail\n");
		return;
	}
	memcpy(ks, data, KS_SIZE);
	memcpy(lc, data+KS_SIZE, LC128_SIZE);
	xor_array(lc, ks, ksxorLc, 16);
	memcpy(ksxorlc128, ksxorLc, LC128_SIZE);
}

void rtk_hdcp2_get_ksxorlc128(unsigned char *data, unsigned int size)
{
	if(data == NULL || size != 16)
	{
		pr_emerg("rtk_hdcp2_get_ksxorlc128 fail\n");
		return;
	}
	memcpy(data, ksxorlc128, size);
}


#define HDCP2_TMP_BUFFER_SIZE       (128 * 1024)

int hdcp2_open(struct inode *inode, struct file *filp) {
#if IS_ENABLED(CONFIG_RTK_KDRV_MCP)
	filp->private_data = mcp_malloc(HDCP2_TMP_BUFFER_SIZE);
#endif
    	if (filp->private_data==NULL)
        		return -ENOMEM;

	return 0;
}

ssize_t  hdcp2_read(struct file *filep, char *buffer, size_t count, loff_t *offp)
{
	return 0;

}

ssize_t hdcp2_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{
	return 0;
}

int hdcp2_release(struct inode *inode, struct file *filp)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_MCP)
	if (filp->private_data)
        	mcp_free(filp->private_data, HDCP2_TMP_BUFFER_SIZE);
#endif
	return 0;
}

unsigned char Rx_file_path[] = "/mnt/Rxkey.bin";
#define HDCP2_KEY_LEN_MAX 1024
long hdcp2_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
    	unsigned char* hdcp_buff = (unsigned char*) file->private_data;
	int retval = 0;
	pr_debug("HDCP2: hdcp2_ioctl, %x;;agr:%x\n", cmd,*((unsigned int *)arg));
	if (_IOC_TYPE(cmd) != HDCP2_IOC_MAGIC || _IOC_NR(cmd) > HDCP2_IOC_MAXNR) return -ENOTTY ;

	switch (cmd)
	{
		case HDCP2_IOC_WRITE_KEYSET:
		{
			HDCP2_WRITE_KEYSET_PARA_T write_keyset;
			if(copy_from_user((void *)&write_keyset, (const void __user *)arg, sizeof(HDCP2_WRITE_KEYSET_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_WRITE_KEYSET copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *temp_keyset = kmalloc(HDCP2_KEY_LEN_MAX, GFP_KERNEL);
				if(temp_keyset == NULL) {

					break;
					}
				memset(temp_keyset,0,HDCP2_KEY_LEN_MAX);				
				if(copy_from_user((void *)temp_keyset, to_user_ptr(write_keyset.pKeySet), 878))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_keyset copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_WriteKeySet(temp_keyset, Rx_file_path/*to_user_ptr(write_keyset.dst_file_path)*/, write_keyset.keyLength);
				
				kfree(temp_keyset);
				temp_keyset = NULL;
			}
			break;
		}
		case HDCP2_IOC_GET_ENCHDCP2KEY:
		{
			HDCP2_GET_ENCHDCP2KEY_PARA_T enchdcp2_key;
			if(copy_from_user((void *)&enchdcp2_key, (const void __user *)arg, sizeof(HDCP2_GET_ENCHDCP2KEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GET_ENCHDCP2KEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				rtk_hal_hdcp2_GetEncHdcp2Key(to_user_ptr(enchdcp2_key.pEncKey), to_user_ptr(enchdcp2_key.src_file_path), to_user_ptr(enchdcp2_key.keyLen));

			}
			break;
		}
		case HDCP2_IOC_SET_ENCHDCP2KEY:
		{
			unsigned char *enchdcp2_key;
			if(copy_from_user((void *)&enchdcp2_key, (const void __user *)arg, sizeof(unsigned char *)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SET_ENCHDCP2KEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SET_ENCHDCP2KEY copy_from_user failed!!!!!!!!!!!!!!!\n");
				rtk_hal_hdcp2_SetEncHdcp2Key(enchdcp2_key);

			}
			break;
		}
		case HDCP2_IOC_GET_ROOTPUBLICKEY:
		{
			HDCP2_GET_ROOTPUBLICKEY_PARA_T rootpublic_key;
			if(copy_from_user((void *)&rootpublic_key, (const void __user *)arg, sizeof(HDCP2_GET_ROOTPUBLICKEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GET_ROOTPUBLICKEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				rtk_hal_hdcp2_GetRootPublicKey(to_user_ptr(rootpublic_key.pRootPublicKey), to_user_ptr(rootpublic_key.src_file_path));
			}
			break;
		}
		case HDCP2_IOC_GET_CERTINFO:
		{
			HDCP2_GET_CERTINFO_PARA_T certinfo;
			if(copy_from_user((void *)&certinfo, (const void __user *)arg, sizeof(HDCP2_GET_CERTINFO_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GET_CERTINFO copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_receiveID[RECEIVER_ID_SIZE] = {0x00};
				unsigned char temp_pubkey[KEY_PUBLIC_RX_SIZE] = {0x00};
				unsigned char temp_rootsign[DCP_LLC_SIG_SIZE] = {0x00};
				rtk_hal_hdcp2_GetCertInfo(temp_receiveID, temp_pubkey, temp_rootsign);

				if(copy_to_user(to_user_ptr(certinfo.pReceiverID), (void *) temp_receiveID, RECEIVER_ID_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_receiveID copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(copy_to_user(to_user_ptr(certinfo.pPubKey), (void *) temp_pubkey, KEY_PUBLIC_RX_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_pubkey copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(copy_to_user(to_user_ptr(certinfo.pRootSign), (void *) temp_rootsign, DCP_LLC_SIG_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_rootsign copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}

			}
			break;
		}
		case HDCP2_IOC_GET_CERTINFO2:
		{
			HDCP2_GET_CERTINFO_PARA_T certinfo2;
			if(copy_from_user((void *)&certinfo2, (const void __user *)arg, sizeof(HDCP2_GET_CERTINFO_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GET_CERTINFO2 copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_receiveID[RECEIVER_ID_SIZE] = {0x00};
				unsigned char temp_pubkey[KEY_PUBLIC_RX_SIZE] = {0x00};
				unsigned char temp_reserved[RESERVED_SIZE] = {0x00};
				unsigned char temp_rootsign[DCP_LLC_SIG_SIZE] = {0x00};
				rtk_hal_hdcp2_GetCertInfo2(temp_receiveID, temp_pubkey, temp_reserved, temp_rootsign);

				if(copy_to_user(to_user_ptr(certinfo2.pReceiverID), (void *) temp_receiveID, RECEIVER_ID_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_receiveID copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(copy_to_user(to_user_ptr(certinfo2.pPubKey), (void *) temp_pubkey, KEY_PUBLIC_RX_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_pubkey copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(copy_to_user(to_user_ptr(certinfo2.pReserved), (void *) temp_reserved, RESERVED_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_reserved copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(copy_to_user(to_user_ptr(certinfo2.pRootSign), (void *) temp_rootsign, DCP_LLC_SIG_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_rootsign copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}

			}
			break;
		}
		case HDCP2_IOC_DECRYPT_RSAES_OAEP:
		{
			unsigned char pEKpubKm[EKPUBKM_SIZE] = {0x00};
			if(copy_from_user((void *)pEKpubKm, (const void __user *)arg, EKPUBKM_SIZE))
			{
				retval = -EFAULT;
				pr_emerg("scaler hdcp2 ioctl code=HDCP2_IOC_DECRYPT_RSAES_OAEP copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				if(rtk_hal_hdcp2_Decrypt_RSAES_OAEP(pEKpubKm) == H2_ERROR)
				{
					retval = -EFAULT;
					pr_emerg("scaler hdcp2 ioctl code=HDCP2_IOC_DECRYPT_RSAES_OAEP pEkpubkm error!!!\n");
				}
			}
			break;
		}
		case HDCP2_IOC_KD_KEY_DERIVATION:
		{
			HDCP2_KD_KEY_DERIVATION_PARA_T kd_key_derivation;
			if(copy_from_user((void *)&kd_key_derivation, (const void __user *)arg, sizeof(HDCP2_KD_KEY_DERIVATION_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_KD_KEY_DERIVATION copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_rtx[RTX_SIZE] = {0x00};
				unsigned char temp_rrx[RRX_SIZE] = {0x00};
				if(copy_from_user((void *)temp_rtx, (const void __user *)to_user_ptr(kd_key_derivation.pRtx), RTX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rtx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rrx, (const void __user *)to_user_ptr(kd_key_derivation.pRrx), RRX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rrx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				rtk_hal_hdcp2_Kd_Key_Derivation(temp_rtx, temp_rrx, kd_key_derivation.version);
			}
			break;
		}
		case HDCP2_IOC_COMPUTE_H_PRIME:
		{
			HDCP2_COMPUTE_H_PRIME_PARA_T compute_h_prime;
			if(copy_from_user((void *)&compute_h_prime, (const void __user *)arg, sizeof(HDCP2_COMPUTE_H_PRIME_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_COMPUTE_H_PRIME copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_hprime[H_SIZE] = {0x00};
				unsigned char temp_rtx[RTX_SIZE] = {0x00};
				unsigned char temp_deviceOptionInfo[RXCAPS_SIZE + TXCAPS_SIZE] = {0x00};

				if(copy_from_user((void *)temp_rtx, (const void __user *)to_user_ptr(compute_h_prime.pRtx), RTX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rtx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_deviceOptionInfo, (const void __user *)to_user_ptr(compute_h_prime.deviceOptionInfo), RXCAPS_SIZE + TXCAPS_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_deviceOptionInfo copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_Compute_H_Prime(temp_hprime, compute_h_prime.repeater, temp_rtx, temp_deviceOptionInfo, compute_h_prime.version);

				if(copy_to_user(to_user_ptr(compute_h_prime.pHPrime), (void *) temp_hprime, H_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_hprime copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			}
			break;
		}
		case HDCP2_IOC_COMPUTE_L_PRIME:
		{
			HDCP2_COMPUTE_L_PRIME_PARA_T compute_l_prime;
			if(copy_from_user((void *)&compute_l_prime, (const void __user *)arg, sizeof(HDCP2_COMPUTE_L_PRIME_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_COMPUTE_L_PRIME copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_lprime[L_SIZE] = {0x00};
				unsigned char temp_rn[RN_SIZE] = {0x00};
				unsigned char temp_rrx[RRX_SIZE] = {0x00};

				if(copy_from_user((void *)temp_rn, (const void __user *)to_user_ptr(compute_l_prime.pRn), RN_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rn copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rrx, (const void __user *)to_user_ptr(compute_l_prime.pRrx), RRX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rrx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_Compute_L_Prime(temp_lprime, temp_rn, temp_rrx,  compute_l_prime.version);

				if(copy_to_user(to_user_ptr(compute_l_prime.pLPrime), (void *) temp_lprime, L_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_lprime copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			}
			break;
		}
		case HDCP2_IOC_COMPUTE_KH:
		{
			rtk_hal_hdcp2_Compute_Kh();
			break;
		}
		case HDCP2_IOC_ENCRYPT_KM_USING_KH:
		{
			HDCP2_ENCRYPT_KM_USING_KH_PARA_T encrypt_km;
			if(copy_from_user((void *)&encrypt_km, (const void __user *)arg, sizeof(HDCP2_ENCRYPT_KM_USING_KH_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_ENCRYPT_KM_USING_KH copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_ekhkm[EKHKM_SIZE] = {0x00};
				unsigned char temp_m[M_SIZE] = {0x00};

				if(copy_from_user((void *)temp_m, (const void __user *)to_user_ptr(encrypt_km.pM), M_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_m copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_Encrypt_Km_using_Kh(temp_ekhkm, temp_m);

				if(copy_to_user(to_user_ptr(encrypt_km.pEKH_KM), (void *) temp_ekhkm, EKHKM_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_ekhkm copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			}
			break;
		}
		case HDCP2_IOC_DECRYPT_KM_USING_KH:
		{
			HDCP2_ENCRYPT_KM_USING_KH_PARA_T decrypt_km;
			if(copy_from_user((void *)&decrypt_km, (const void __user *)arg, sizeof(HDCP2_ENCRYPT_KM_USING_KH_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DECRYPT_KM_USING_KH copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_ekhkm[EKHKM_SIZE] = {0x00};
				unsigned char temp_m[M_SIZE] = {0x00};

				if(copy_from_user((void *)temp_ekhkm, (const void __user *)to_user_ptr(decrypt_km.pEKH_KM), EKHKM_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_ekhkm copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_m, (const void __user *)to_user_ptr(decrypt_km.pM), M_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_m copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				
				rtk_hal_hdcp2_Decrypt_Km_using_Kh(temp_m, temp_ekhkm);
			}
			break;
		}
		case HDCP2_IOC_DECRYPT_EKS:
		{
			HDCP2_DECRYPT_EKS_PARA_T decrypt_eks;
			if(copy_from_user((void *)&decrypt_eks, (const void __user *)arg, sizeof(HDCP2_DECRYPT_EKS_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DECRYPT_EKS copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_eks[EDKEYKS_SIZE] = {0x00};
				unsigned char temp_rtx[RTX_SIZE] = {0x00};
				unsigned char temp_rrx[RRX_SIZE] = {0x00};
				unsigned char temp_rn[RN_SIZE] = {0x00};

				if(copy_from_user((void *)temp_eks, (const void __user *)to_user_ptr(decrypt_eks.pEks), EDKEYKS_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_eks copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rtx, (const void __user *)to_user_ptr(decrypt_eks.pRtx), RTX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rtx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rrx, (const void __user *)to_user_ptr(decrypt_eks.pRrx), RRX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rrx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rn, (const void __user *)to_user_ptr(decrypt_eks.pRn), RN_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rn copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_Decrypt_EKs(temp_eks, temp_rtx, temp_rrx, temp_rn, decrypt_eks.version);
			}
			break;
		}
		case HDCP2_IOC_XOR_KS_WITH_LC128:
		{
			rtk_hal_hdcp2_XOR_Ks_with_LC128();
			break;
		}
		case HDCP2_IOC_GENERATE_KM:
		{
			HDCP2_GENERATE_KM_PARA_T Gen_km;
			if(copy_from_user((void *)&Gen_km, (const void __user *)arg, sizeof(HDCP2_GENERATE_KM_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GENERATE_KM copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_maskedDB[DB_SIZE+4] = {0x00};
				unsigned char temp_db[DB_SIZE] = {0x00};
				unsigned char temp_dbmask[DB_SIZE] = {0x00};

				if(copy_from_user((void *)temp_db, (const void __user *)to_user_ptr(Gen_km.pDB), DB_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_db copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_dbmask, (const void __user *)to_user_ptr(Gen_km.pDBMask), DB_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_dbmask copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_Generate_Km(temp_maskedDB, temp_db, temp_dbmask);

				if(copy_to_user(to_user_ptr(Gen_km.pMaskedDB), (void *) temp_maskedDB, DB_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_maskedDB copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			}
			break;
		}
		case HDCP2_IOC_GENERATE_KS:
		{
			HDCP2_GENERATE_KS_PARA_T Gen_ks;
			if(copy_from_user((void *)&Gen_ks, (const void __user *)arg, sizeof(HDCP2_GENERATE_KS_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GENERATE_KS copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_eks[EDKEYKS_SIZE] = {0x00};
				unsigned char temp_rtx[RTX_SIZE] = {0x00};
				unsigned char temp_rrx[RRX_SIZE] = {0x00};
				unsigned char temp_rn[RN_SIZE] = {0x00};

				if(copy_from_user((void *)temp_rtx, (const void __user *)to_user_ptr(Gen_ks.pRtx), RTX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rtx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rrx, (const void __user *)to_user_ptr(Gen_ks.pRrx), RRX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rrx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rn, (const void __user *)to_user_ptr(Gen_ks.pRn), RN_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rn copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				rtk_hal_hdcp2_Generate_Ks(temp_eks, temp_rrx, temp_rtx, temp_rn, Gen_ks.version);

				if(copy_to_user(to_user_ptr(Gen_ks.pEks), (void *) temp_eks, EDKEYKS_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_eks copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			}
			break;
		}
		case HDCP2_IOC_DATA_DECRYPT:
		{
			HDCP2_DATA_DECRYPT_PARA_T data;
			if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(HDCP2_DATA_DECRYPT_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DATA_DECRYPT copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_counter[16] = {0x00};
				if(copy_from_user((void *)temp_counter, (const void __user *)to_user_ptr(data.pCounter), 16))
		        	{
		            			retval = -EFAULT;
		            			DBG_PRINT("scaler hdcp2 ioctl code=temp_counter copy_from_user failed!!!!!!!!!!!!!!!\n");
		            			break;
		        	}

				if (hdcp_buff==NULL || data.length < 512)
                {
                		unsigned char *hdcp_outputbuf = NULL;
                		unsigned char *hdcp_inputbuf =NULL;
						hdcp_outputbuf = kmalloc(512, GFP_KERNEL);
						if(hdcp_outputbuf == NULL) {
							break;
						}
						hdcp_inputbuf = kmalloc(512, GFP_KERNEL);
						if(hdcp_inputbuf == NULL) {
							kfree(hdcp_outputbuf);
							hdcp_outputbuf = NULL;
							break;
						}
						memset(hdcp_outputbuf,0x00,512);
						memset(hdcp_inputbuf,0x00,512);
						
						if(copy_from_user((void *)hdcp_inputbuf, (const void __user *)to_user_ptr(data.pInputBuf), data.length))
		        			{
		            				retval = -EFAULT;
									kfree(hdcp_outputbuf);
									kfree(hdcp_inputbuf);
									hdcp_outputbuf = NULL;
									hdcp_inputbuf = NULL;
		            				DBG_PRINT("scaler hdcp2 ioctl code=hdcp_inputbuf copy_from_user failed!!!!!!!!!!!!!!!\n");
		            				break;
		        			}

                    				rtk_hal_hdcp2_Data_Decrypt(hdcp_outputbuf, hdcp_inputbuf, data.length, temp_counter);

						if(copy_to_user(to_user_ptr(data.pOutputBuf), (void *) hdcp_outputbuf, data.length))
    			        		{
    			           			retval = -EFAULT;
									kfree(hdcp_outputbuf);
									kfree(hdcp_inputbuf);
									hdcp_outputbuf = NULL;
									hdcp_inputbuf = NULL;
				            		DBG_PRINT("scaler hdcp2 ioctl code=hdcp_outputbuf copy_from_user failed!!!!!!!!!!!!!!!\n");
				            		break;
    			        		}

							kfree(hdcp_outputbuf);
							kfree(hdcp_inputbuf);
							hdcp_outputbuf = NULL;
							hdcp_inputbuf = NULL;
                			}
                			else
                			{
    			    		while(data.length)
    			    		{
    			        			unsigned char KsxorLc128[16] = {0x00};
    			        			unsigned int len = (data.length >= HDCP2_TMP_BUFFER_SIZE) ?  HDCP2_TMP_BUFFER_SIZE : data.length;
									#ifndef CONFIG_OPTEE_HDCP2
                        					spu_GetKsXorLc128(KsxorLc128);
									#endif

                        					if(copy_from_user((void *)hdcp_buff, (const void __user *)to_user_ptr(data.pInputBuf), len))
    			        			{
    			            				retval = -EFAULT;
				            			DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DATA_DECRYPT copy_from_user failed!!!!!!!!!!!!!!!\n");
				            			break;
    			        			}
#if IS_ENABLED(CONFIG_RTK_KDRV_MCP)
    			        			MCP_AES_CTR_Decryption(KsxorLc128, temp_counter, hdcp_buff, hdcp_buff, (len + 15) & (~0xF));

                        				MCP_AES_PeekIV(temp_counter);  // peek iv
#endif

    			        			if(copy_to_user(to_user_ptr(data.pOutputBuf), (void *) hdcp_buff, len))
    			        			{
    			           				retval = -EFAULT;
				            			DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DATA_DECRYPT copy_from_user failed!!!!!!!!!!!!!!!\n");
				            			break;
    			        			}

    			        			data.pInputBuf += len;
    			        			data.pOutputBuf += len;
    			        			data.length -= len;
    			    		}
    				}
			}
			break;
		}
		case HDCP2_IOC_SHA1:
		{
			HDCP2_SHA_PARA_T sha1;
			if(copy_from_user((void *)&sha1, (const void __user *)arg, sizeof(HDCP2_SHA_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SHA1 copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *in_ptr = kmalloc(sha1.length, GFP_KERNEL);
				unsigned char *out_ptr = kmalloc(20, GFP_KERNEL);
				if((in_ptr == NULL) ||(out_ptr == NULL))
				{
					if(in_ptr != NULL) {
						kfree(in_ptr);
						in_ptr = NULL;
					}
					if(out_ptr != NULL) {
						kfree(out_ptr);
						out_ptr = NULL;
					}
					DBG_PRINT("in_ptr/out_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)in_ptr, (const void __user *)to_user_ptr(sha1.pInputBuf), sha1.length))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				
				rtk_hal_hdcp2_SHA1(out_ptr, in_ptr, sha1.length);

				if(copy_to_user(to_user_ptr(sha1.pOutputBuf), (void *)out_ptr, 20))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=out_ptr copy_to_user failed!!!!!!!!!!!!!!!\n");
        			//break;
    			}
				
				kfree(in_ptr);
				kfree(out_ptr);
				in_ptr = NULL;
				out_ptr = NULL;
			}
			break;
		}
		case HDCP2_IOC_SHA256:
		{
			HDCP2_SHA_PARA_T sha256;
			if(copy_from_user((void *)&sha256, (const void __user *)arg, sizeof(HDCP2_SHA_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SHA256 copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *in_ptr = kmalloc(sha256.length, GFP_KERNEL);
				unsigned char *out_ptr = kmalloc(32, GFP_KERNEL);
				if((in_ptr == NULL) ||(out_ptr == NULL))
				{
					if(in_ptr != NULL) {
						kfree(in_ptr);
						in_ptr = NULL;
					}
					if(out_ptr != NULL) {
						kfree(out_ptr);
						out_ptr = NULL;
					}
					DBG_PRINT("in_ptr/out_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)in_ptr, (const void __user *) to_user_ptr(sha256.pInputBuf), sha256.length))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_SHA256(out_ptr, in_ptr, sha256.length);

				if(copy_to_user(to_user_ptr(sha256.pOutputBuf), (void *)out_ptr, 32))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=out_ptr copy_to_user failed!!!!!!!!!!!!!!!\n");
            				//break;
        			}

				kfree(in_ptr);
				kfree(out_ptr);
				in_ptr = NULL;
				out_ptr = NULL;
			}
			break;
		}
		case HDCP2_IOC_HMAC_SHA256:
		{
			HDCP2_HMAC_SHA256_PARA_T hmac_sha256;
			if(copy_from_user((void *)&hmac_sha256, (const void __user *)arg, sizeof(HDCP2_HMAC_SHA256_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_HMAC_SHA256 copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *in_ptr = kmalloc(hmac_sha256.length, GFP_KERNEL);
				unsigned char *out_ptr = kmalloc(32, GFP_KERNEL);
				unsigned char *key_ptr = kmalloc(hmac_sha256.keylength, GFP_KERNEL);
				if((in_ptr == NULL) ||(out_ptr == NULL) || (key_ptr == NULL))
				{
					if(in_ptr != NULL) {
						kfree(in_ptr);
						in_ptr = NULL;
					}
					if(out_ptr != NULL) {
						kfree(out_ptr);
						out_ptr = NULL;
					}
					if(key_ptr != NULL) {
						kfree(key_ptr);
						key_ptr = NULL;
					}
					DBG_PRINT("in_ptr/out_ptr/key_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)in_ptr, (const void __user *) to_user_ptr(hmac_sha256.pInputBuf), hmac_sha256.length))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)key_ptr, (const void __user *) to_user_ptr(hmac_sha256.pKey), hmac_sha256.keylength))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_HMAC_SHA256(out_ptr, in_ptr, hmac_sha256.length, key_ptr, hmac_sha256.keylength);

				if(copy_to_user(to_user_ptr(hmac_sha256.pOutputBuf), (void *)out_ptr, 32))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=out_ptr copy_to_user failed!!!!!!!!!!!!!!!\n");
            				//break;
        			}

				kfree(in_ptr);
				kfree(out_ptr);
				kfree(key_ptr);
				in_ptr = NULL;
				out_ptr = NULL;
				key_ptr = NULL;
			}
			break;
		}
		case HDCP2_IOC_AES_CTR:
		{
			HDCP2_AES_CTR_PARA_T aes_ctr;
			if(copy_from_user((void *)&aes_ctr, (const void __user *)arg, sizeof(HDCP2_AES_CTR_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_AES_CTR_PARA_T copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *in_ptr = kmalloc(aes_ctr.length, GFP_KERNEL);
				unsigned char *out_ptr = kmalloc(aes_ctr.length, GFP_KERNEL);
				unsigned char *key_ptr = kmalloc(aes_ctr.keyLength, GFP_KERNEL);
				unsigned char counter[16];
				if((in_ptr == NULL) ||(out_ptr == NULL) || (key_ptr == NULL))
				{
					if(in_ptr != NULL) {
						kfree(in_ptr);
						in_ptr = NULL;
					}
					if(out_ptr != NULL) {
						kfree(out_ptr);
						out_ptr = NULL;
					}
					if(key_ptr != NULL) {
						kfree(key_ptr);
						key_ptr = NULL;
					}
					DBG_PRINT("in_ptr/out_ptr/key_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)in_ptr, (const void __user *) to_user_ptr(aes_ctr.pInputBuf), aes_ctr.length))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)key_ptr, (const void __user *)to_user_ptr(aes_ctr.pkey), aes_ctr.keyLength))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)counter, (const void __user *)to_user_ptr(aes_ctr.pCounter), 16))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_AES_CTR(out_ptr, in_ptr, aes_ctr.length, key_ptr, aes_ctr.keyLength, counter);

				if(copy_to_user(to_user_ptr(aes_ctr.pOutputBuf), (void *)out_ptr, aes_ctr.length))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=out_ptr copy_to_user failed!!!!!!!!!!!!!!!\n");
            				//break;
        			}

				kfree(in_ptr);
				kfree(out_ptr);
				kfree(key_ptr);
				in_ptr = NULL;
				out_ptr = NULL;
				key_ptr = NULL;
			}
			break;
		}
		case HDCP2_IOC_SELECT_STOREDKM:
		{
			int index;
			if(copy_from_user((void *)&index, (const void __user *)arg, sizeof(int)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SELECT_STOREDKM copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				rtk_hal_hdcp2_Select_StoredKm(index);
			}
			break;
		}
		case HDCP2_IOC_WRITE_STOREDKM:
		{
			int index;
			if(copy_from_user((void *)&index, (const void __user *)arg, sizeof(int)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_WRITE_STOREDKM copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				rtk_hal_hdcp2_Write_StoredKm(index);
			}
			break;
		}
		case HDCP2_IOC_DATA_ENCRYPT:
		{
			HDCP2_DATA_DECRYPT_PARA_T data;
			if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(HDCP2_DATA_DECRYPT_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DATA_ENCRYPT copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				rtk_hal_hdcp2_Data_Encrypt(to_user_ptr(data.pOutputBuf), to_user_ptr(data.pInputBuf), data.length, to_user_ptr(data.pCounter));
			}
			break;
		}
		case HDCP2_IOC_GET_PROTECTED_DECRYPTIONKEY:
		{
			HDCP2_GET_PROTECTED_DECRYPTIONKEY_PARA_T get_prokey;
			if(copy_from_user((void *)&get_prokey, (const void __user *)arg, sizeof(HDCP2_GET_PROTECTED_DECRYPTIONKEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GET_PROTECTED_DECRYPTIONKEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_data[32];

				rtk_hal_hdcp2_GetProtectedDecryptionKey(temp_data, (int *)to_user_ptr(get_prokey.pLength));

				if(copy_to_user(to_user_ptr(get_prokey.pData), (void *) temp_data, 32))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_data copy_from_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			}
			break;
		}
		case HDCP2_IOC_SET_PROTECTED_DECRYPTIONKEY:
		{
			HDCP2_SET_PROTECTED_DECRYPTIONKEY_PARA_T set_prokey;
			if(copy_from_user((void *)&set_prokey, (const void __user *)arg, sizeof(HDCP2_SET_PROTECTED_DECRYPTIONKEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SET_PROTECTED_DECRYPTIONKEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_data[32];
				if(copy_from_user((void *)temp_data, (const void __user *)to_user_ptr(set_prokey.pData), set_prokey.length))
        			{
            				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_data copy_from_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				rtk_hal_hdcp2_SetProtectedDecryptionKey(temp_data, set_prokey.length);
			}
			break;
		}
		case HDCP2_IOC_HDCP_INIT:
		{
			rtk_hal_hdcp_Init();
			break;
		}
		case HDCP2_IOC_GET_OTPHDCP2KEY:
		{
			HDCP2_GET_OTPHDCP2KEY_PARA_T HDCP2_OTP_KEY;
			if(copy_from_user((void *)&HDCP2_OTP_KEY, (const void __user *)arg, sizeof(HDCP2_GET_OTPHDCP2KEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SET_PROTECTED_DECRYPTIONKEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				int temp_exsit = 0;
				unsigned char *temp_data = kmalloc(1024, GFP_KERNEL);
				if(temp_data == NULL) {
					break;
				}
				memset(temp_data,0,1024);
				
#if IS_ENABLED(CONFIG_RTK_KDRV_OTP)
#if defined(CONFIG_RTK_KDRV_HDCP_KEY_FROM_OTP)
				if(get_hdcp_22_otp(temp_data, HDCP2_OTP_KEY.length) == 0)
					temp_exsit = 1;
				else
#endif
					temp_exsit = 0;
#endif
				if(copy_to_user(to_user_ptr(HDCP2_OTP_KEY.pData), (void *) temp_data, HDCP2_OTP_KEY.length))
        			{
           				retval = -EFAULT;
						kfree(temp_data);
						temp_data = NULL;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_data copy_from_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(copy_to_user((void __user *)to_user_ptr(HDCP2_OTP_KEY.exsit), (void *) &temp_exsit, sizeof(int)))
        			{
           				retval = -EFAULT;
						kfree(temp_data);
						temp_data = NULL;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_data copy_from_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				kfree(temp_data);
				temp_data = NULL;
			}
			break;
		}
		case HDCP2_IOC_COMPUTE_V_PRIME:
		{
			HDCP2_COMPUTE_V_PRIME_PARA_T compute_v_prime;
			if(copy_from_user((void *)&compute_v_prime, (const void __user *)arg, sizeof(HDCP2_COMPUTE_V_PRIME_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_COMPUTE_V_PRIME_PARA_T copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_vprime[VPRIME_SIZE] = {0x00};
				unsigned char temp_listinfo[LIST_SIZE] = {0x00};

				if(copy_from_user((void *)temp_listinfo, (const void __user *)to_user_ptr(compute_v_prime.pRxIdListsInfo), compute_v_prime.infoLength))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_listinfo copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_Compute_V_Prime(temp_vprime, temp_listinfo, compute_v_prime.infoLength);

				if(copy_to_user(to_user_ptr(compute_v_prime.pVPrime), (void *) temp_vprime, VPRIME_SIZE))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=temp_vprime copy_from_user failed!!!!!!!!!!!!!!!\n");
        			break;
    			}

			}
			break;
		}
		case HDCP2_IOC_COMPUTE_M_PRIME:
		{
			HDCP2_COMPUTE_M_PRIME_PARA_T compute_m_prime;
			if(copy_from_user((void *)&compute_m_prime, (const void __user *)arg, sizeof(HDCP2_COMPUTE_M_PRIME_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_COMPUTE_M_PRIME_PARA_T copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_mprime[MPRIME_SIZE] = {0x00};
				unsigned char temp_streaminfo[STREAM_SIZE] = {0x00};

				if(copy_from_user((void *)temp_streaminfo, (const void __user *)to_user_ptr(compute_m_prime.pStreamManageInfo), compute_m_prime.infoLength))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_streaminfo copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_Compute_M_Prime(temp_mprime, temp_streaminfo, compute_m_prime.infoLength);

				if(copy_to_user(to_user_ptr(compute_m_prime.pMPrime), (void *) temp_mprime, MPRIME_SIZE))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=temp_mprime copy_from_user failed!!!!!!!!!!!!!!!\n");
        			break;
    			}

			}
			break;
		}
		case HDCP2_IOC_GET_HDCP_STATUS:
		{
			HDCP2_GET_HDCP_STATUS_PARA_T GET_HDCP_STATUS;
			if(copy_from_user((void *)&GET_HDCP_STATUS, (const void __user *)arg, sizeof(HDCP2_GET_HDCP_STATUS_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_GET_HDCP_STATUS_PARA_T copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char b14status = 0;
				unsigned char b22status = 0;
				rtk_hal_hdcp2_get_hdcp_status(&b14status, &b22status);

				if(copy_to_user(to_user_ptr(GET_HDCP_STATUS.b14Status), (void *) &b14status, sizeof(unsigned char)))
				{
	   				retval = -EFAULT;
	    			DBG_PRINT("scaler hdcp2 ioctl code=b14status copy_from_user failed!!!!!!!!!!!!!!!\n");
	    			break;
				}

				if(copy_to_user(to_user_ptr(GET_HDCP_STATUS.b22Status), (void *) &b22status, sizeof(unsigned char)))
				{
	   				retval = -EFAULT;
	    			DBG_PRINT("scaler hdcp2 ioctl code=b22status copy_from_user failed!!!!!!!!!!!!!!!\n");
	    			break;
				}
			}
			break;
		}
		case HDCP2_IOC_UseTestVector:
		{
			HDCP2_UseTestVector_PARA_T usetestvector;
			HDCP2_UseTestVector_local_PARA test_vector;
			if(copy_from_user((void *)&usetestvector, (const void __user *)arg, sizeof(HDCP2_UseTestVector_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_UseTestVector copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else if(usetestvector.argc == 0)
			{
				rtk_hal_hdcp2_UseTestVector(0, NULL);
			}
			else
			{
				unsigned char *lc128 = kmalloc(LC128_SIZE, GFP_KERNEL);
				unsigned char *km = kmalloc(KM_SIZE, GFP_KERNEL);
				unsigned char *ks = kmalloc(KS_SIZE, GFP_KERNEL);
				unsigned char *seed = kmalloc(SEED_SIZE, GFP_KERNEL);
				if((lc128 == NULL) ||(km == NULL) || (ks == NULL) || (seed == NULL))
				{
					if(lc128 != NULL) {
						kfree(lc128);
						lc128 = NULL;
					}
					if(km != NULL) {
						kfree(km);
						km = NULL;
					}
					if(ks != NULL) {
						kfree(ks);
						ks = NULL;
					}
					if(seed != NULL) {
						kfree(seed);
						seed = NULL;
					}
					DBG_PRINT("lc128/km/ks/seed kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)lc128, (const void __user *) to_user_ptr(usetestvector.lc128), LC128_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)km, (const void __user *)to_user_ptr(usetestvector.km), KM_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)ks, (const void __user *)to_user_ptr(usetestvector.ks), KS_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)seed, (const void __user *)to_user_ptr(usetestvector.seed), SEED_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				test_vector.argc = usetestvector.argc;
				test_vector.lc128 =lc128;
				test_vector.km = km;
				test_vector.ks = ks;
				test_vector.seed = seed;
				rtk_hal_hdcp2_UseTestVector(usetestvector.argc, &test_vector);


				kfree(lc128);
				kfree(km);
				kfree(ks);
				kfree(seed);
				lc128 = NULL;
				km = NULL;
				ks = NULL;
				seed = NULL;
			}
			break;
		}		
		case HDCP2_IOC_WRITE_HDCP2_KEY:
		{
#ifdef CONFIG_CUSTOMER_TV010
			unsigned char* ap_local_key = (unsigned char*)arg;
			printk("[DEBUG]COPY_HDCP2_KEY\n");
			//printk("k>>>>>>>>>>>>>>>>>>>>>>>>arg=0x%x\n", arg);

			//printk("1ap_local_key>>>>>>>>>>>>>>>>>>>>>>>>%x %x %x %x\n", *ap_local_key,*(ap_local_key+1),*(ap_local_key+2),*(ap_local_key+3));
			//printk("2ap_local_key>>>>>>>>>>>>>>>>>>>>>>>>%x %x %x %x\n", *(ap_local_key+4),*(ap_local_key+5),*(ap_local_key+6),*(ap_local_key+7));

			//printk("1orig loacl_key>>>>>>>>>>>>>>>>>>>>>>>>%x %x %x %x\n",loacl_key[0],loacl_key[1],loacl_key[2],loacl_key[3]);
			//printk("2orig loacl_key>>>>>>>>>>>>>>>>>>>>>>>>%x %x %x %x\n",loacl_key[4],loacl_key[5],loacl_key[6],loacl_key[7]);
			//memcpy(loacl_key, ap_local_key, 902);
			if(copy_from_user((void *)loacl_key, (const void __user *)ap_local_key, 902))
					retval = -EFAULT;
			printk("3kernel loacl_key>>>>>>>>>>>>>>>>>>>>>>>>%x %x %x %x\n",loacl_key[0],loacl_key[1],loacl_key[2],loacl_key[3]);
			printk("4kernel loacl_key>>>>>>>>>>>>>>>>>>>>>>>>%x %x %x %x\n",loacl_key[4],loacl_key[5],loacl_key[6],loacl_key[7]);
			printk("Device ID = %x %x %x %x  %x \n ",loacl_key[40],loacl_key[41],loacl_key[42],loacl_key[43],loacl_key[44]);
			//break; // reload
#else
			printk("[DEBUG]COPY_HDCP2_KEY not support\n");
			break;	
#endif
		}
		
		case HDCP2_IOC_RELOAD_HDCP2_KEY:
		{
#ifdef CONFIG_CUSTOMER_TV010
			printk("[DEBUG]HDCP2_KEY_RELOAD\n");
			retval = hdcp2_load_from_flash();
#else
			printk("[DEBUG]HDCP2_KEY_RELOAD not support\n");
#endif
			break;

		}
		
		case HDCP2_IOC_ENCRYPT_RSAES_OAEP:
		{
			HDCP2_ENCRYPT_RSAES_OAEP_PARA_T encrypt_rsaes_oaep;
			if(copy_from_user((void *)&encrypt_rsaes_oaep, (const void __user *)arg, sizeof(HDCP2_ENCRYPT_RSAES_OAEP_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_ENCRYPT_RSAES_OAEP_PARA_T copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_kpubrx[KPUBRX_SIZE] = {0x00};
				unsigned char temp_ekpub_km[EKPUBKM_SIZE] = {0x00};

				if(copy_from_user((void *)temp_kpubrx, (const void __user *)to_user_ptr(encrypt_rsaes_oaep.pKpubrx), KPUBRX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_kpubrx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_Encrypt_RSAES_OAEP(temp_ekpub_km, temp_kpubrx);

				if(copy_to_user((void __user *)to_user_ptr(encrypt_rsaes_oaep.pEkpub_km), (void *) temp_ekpub_km, EKPUBKM_SIZE))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=temp_ekpub_km copy_from_user failed!!!!!!!!!!!!!!!\n");
        			break;
    			}

			}
			break;
		}
		case HDCP2_IOC_HMAC_SHA256_WITH_KD:
		{
			HDCP2_HMAC_SHA256_WITH_KD_PARA_T HMAC_SHA256_with_kd;
			if(copy_from_user((void *)&HMAC_SHA256_with_kd, (const void __user *)arg, sizeof(HDCP2_HMAC_SHA256_WITH_KD_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_HMAC_SHA256_WITH_KD_PARA_T copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *in_ptr = kmalloc(HMAC_SHA256_with_kd.srcLength, GFP_KERNEL);
				unsigned char *out_ptr = kmalloc(32, GFP_KERNEL);
				
				if((in_ptr == NULL) ||(out_ptr == NULL))
				{
					if(in_ptr != NULL) {
						kfree(in_ptr);
						in_ptr = NULL;
					}
					if(out_ptr != NULL) {
						kfree(out_ptr);
						out_ptr = NULL;
					}
					
					DBG_PRINT("in_ptr/out_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)in_ptr, (const void __user *)to_user_ptr(HMAC_SHA256_with_kd.pSrc), HMAC_SHA256_with_kd.srcLength))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				rtk_hal_hdcp2_HMAC_SHA256_with_kd (out_ptr,in_ptr, HMAC_SHA256_with_kd.srcLength);

				if(copy_to_user((void __user *)to_user_ptr(HMAC_SHA256_with_kd.pDest), (void *) out_ptr, 32))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=out_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
    			}
				kfree(in_ptr);
				kfree(out_ptr);
				in_ptr = NULL;
				out_ptr = NULL;
			}
			break;
		}

		case HDCP2_IOC_ENCRYPTO_KEY:
		{
			HDCP2_ENCRYPTO_KEY_PARA_T encrypto_key;
			if(copy_from_user((void *)&encrypto_key, (const void __user *)arg, sizeof(HDCP2_ENCRYPTO_KEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_ENCRYPTO_KEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *ENkey_ptr = kmalloc(encrypto_key.ENkey_size, GFP_KERNEL);
				unsigned char *AESkey_ptr = kmalloc(encrypto_key.AESkey_size, GFP_KERNEL);
				if((ENkey_ptr == NULL) ||(AESkey_ptr == NULL))
				{
					if(ENkey_ptr != NULL) {
						kfree(ENkey_ptr);
						ENkey_ptr = NULL;
					}
					if(AESkey_ptr != NULL) {
						kfree(AESkey_ptr);
						AESkey_ptr= NULL;
					}
					
					DBG_PRINT("ENkey_ptr/AESkey_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)ENkey_ptr, (const void __user *)to_user_ptr(encrypto_key.ENkey), encrypto_key.ENkey_size))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=ENkey_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				
				rtk_hal_hdcp2_encrypto_key(ENkey_ptr, encrypto_key.ENkey_size, AESkey_ptr, encrypto_key.AESkey_size);

				if(copy_to_user((void __user *)to_user_ptr(encrypto_key.AESkey), (void *) AESkey_ptr, encrypto_key.AESkey_size))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=AESkey_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
    			}

				kfree(ENkey_ptr);
				kfree(AESkey_ptr);
				ENkey_ptr = NULL;
				AESkey_ptr = NULL;
			}
			break;

		}
		case HDCP2_IOC_DECRYPTO_KEY:
		{
			HDCP2_DECRYPTO_KEY_PARA_T decrypto_key;
			if(copy_from_user((void *)&decrypto_key, (const void __user *)arg, sizeof(HDCP2_DECRYPTO_KEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DECRYPTO_KEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *AESkey_ptr = kmalloc(decrypto_key.AESkey_size, GFP_KERNEL);
				
				if(AESkey_ptr == NULL) {
					DBG_PRINT("AESkey_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)AESkey_ptr, (const void __user *)to_user_ptr(decrypto_key.AESkey), decrypto_key.AESkey_size))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=AESkey_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				
				rtk_hal_hdcp2_decrypto_key(AESkey_ptr, decrypto_key.AESkey_size);	
				
				kfree(AESkey_ptr);
				AESkey_ptr = NULL;
				
			}
			break;

		}
		case HDCP2_IOC_DRM_HDMI_SetSecureDataforHDCP22:
		{
			HDCP2_DRM_HDMI_SetSecureDataforHDCP22_PARA_T DRM_HDMI_SetSecureDataforHDCP22;
			if(copy_from_user((void *)&DRM_HDMI_SetSecureDataforHDCP22, (const void __user *)arg, sizeof(HDCP2_DRM_HDMI_SetSecureDataforHDCP22_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DRM_HDMI_SetSecureDataforHDCP22 copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *pData = kmalloc(DRM_HDMI_SetSecureDataforHDCP22.Datalength, GFP_KERNEL);
				unsigned char *pEncKey = kmalloc(DRM_HDMI_SetSecureDataforHDCP22.Datalength, GFP_KERNEL);
				
				if((pData == NULL) ||(pEncKey == NULL))
				{
					if(pData != NULL) {
						kfree(pData);
						pData = NULL;
					}
					if(pEncKey != NULL) {
						kfree(pEncKey);
						pEncKey= NULL;
					}
					
					DBG_PRINT("pData/pEncKey kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)pData, (const void __user *)to_user_ptr(DRM_HDMI_SetSecureDataforHDCP22.pData), DRM_HDMI_SetSecureDataforHDCP22.Datalength))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=pData copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				
				rtk_hal_hdcp2_DRM_HDMI_Encrypto_key(pData, pEncKey, DRM_HDMI_SetSecureDataforHDCP22.Datalength);	
				
				if(copy_to_user((void __user *)to_user_ptr(DRM_HDMI_SetSecureDataforHDCP22.pEncKey), (void *) pEncKey, DRM_HDMI_SetSecureDataforHDCP22.Datalength))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=pEncKey copy_from_user failed!!!!!!!!!!!!!!!\n");
    			}
				
				kfree(pData);
				kfree(pEncKey);
				pData = NULL;
				pEncKey = NULL;
				
			}
			break;

		}
		case HDCP2_IOC_DRM_VerifyHDCP22forHDMI:
		{
			HDCP2_DRM_VerifyHDCP22forHDMI_PARA_T DRM_VerifyHDCP22forHDMI;
			if(copy_from_user((void *)&DRM_VerifyHDCP22forHDMI, (const void __user *)arg, sizeof(DRM_VerifyHDCP22forHDMI)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DRM_VerifyHDCP22forHDMI copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *pEncKey = kmalloc(DRM_VerifyHDCP22forHDMI.length, GFP_KERNEL);
				unsigned char *pDecKey = kmalloc(DRM_VerifyHDCP22forHDMI.length, GFP_KERNEL);
				
				if((pEncKey == NULL) ||(pDecKey == NULL))
				{
					if(pEncKey != NULL) {
						kfree(pEncKey);
						pEncKey = NULL;
					}
					if(pDecKey != NULL) {
						kfree(pDecKey);
						pDecKey= NULL;
					}
					
					DBG_PRINT("pEncKey/pDecKey kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)pEncKey, (const void __user *)to_user_ptr(DRM_VerifyHDCP22forHDMI.pEncKey), DRM_VerifyHDCP22forHDMI.length))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=pEncKey copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				
				rtk_hal_hdcp2_DRM_HDMI_Decrypto_key(pEncKey, pDecKey, DRM_VerifyHDCP22forHDMI.length);	
				
				if(copy_to_user((void __user *)to_user_ptr(DRM_VerifyHDCP22forHDMI.pDecKey), (void *) pDecKey, DRM_VerifyHDCP22forHDMI.length))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=pDecKey copy_from_user failed!!!!!!!!!!!!!!!\n");
    			}
				
				kfree(pEncKey);
				kfree(pDecKey);
				pEncKey = NULL;
				pDecKey = NULL;
				
			}
			break;

		}
		case HDCP2_IOC_VFE_HDMI_WriteHDCP22:
		{
			HDCP2_VFE_HDMI_WriteHDCP22_PARA_T VFE_HDMI_WriteHDCP22;
			if(copy_from_user((void *)&VFE_HDMI_WriteHDCP22, (const void __user *)arg, sizeof(VFE_HDMI_WriteHDCP22)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_VFE_HDMI_WriteHDCP22 copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *pData = kmalloc(VFE_HDMI_WriteHDCP22.size, GFP_KERNEL);
				
				
				if(pData == NULL) {
					kfree(pData);
					pData = NULL;
					DBG_PRINT("pData/pEncKey kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}
					

				if(copy_from_user((void *)pData, (const void __user *)to_user_ptr(VFE_HDMI_WriteHDCP22.pData), VFE_HDMI_WriteHDCP22.size))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=pData copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				
				#ifndef CONFIG_OPTEE_HDCP2 
				spu_SetHdmiKey(pData);
				set_hdmi_hdcp2_sstorage_key_flag(true);
				#endif
				
				kfree(pData);
				pData = NULL;
				
			}
			break;

		}
		default:
			pr_debug("Scaler hdcp2 disp: ioctl code = %d is invalid!!!!!!!!!!!!!!!1\n", cmd);
			break ;
		}
	return retval;


}


#ifdef CONFIG_OPTEE_HDCP2
long hdcp2_optee_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
    	unsigned char* hdcp_buff = (unsigned char*) file->private_data;
	int retval = 0;
	pr_debug("HDCP2: hdcp2_ioctl, %x;;agr:%x\n", cmd,*((unsigned int *)arg));
	if (_IOC_TYPE(cmd) != HDCP2_IOC_MAGIC || _IOC_NR(cmd) > HDCP2_IOC_MAXNR) return -ENOTTY ;

	switch (cmd)
	{
		case HDCP2_IOC_WRITE_KEYSET:
		{
			HDCP2_WRITE_KEYSET_PARA_T write_keyset;
			if(copy_from_user((void *)&write_keyset, (const void __user *)arg, sizeof(HDCP2_WRITE_KEYSET_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_WRITE_KEYSET copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *temp_keyset = kmalloc(HDCP2_KEY_LEN_MAX, GFP_KERNEL);
				if(temp_keyset == NULL) {

					break;
					}
				memset(temp_keyset,0,HDCP2_KEY_LEN_MAX);				
				if(copy_from_user((void *)temp_keyset, to_user_ptr(write_keyset.pKeySet), 878))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_keyset copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				optee_hdcp2_wid_WriteKeySet(HDCP2_CMD_WRITE_KEYSET, temp_keyset,878);
				kfree(temp_keyset);
				temp_keyset = NULL;
			}
			break;
		}
		case HDCP2_IOC_GET_ENCHDCP2KEY:
		{
			HDCP2_GET_ENCHDCP2KEY_PARA_T enchdcp2_key;
			if(copy_from_user((void *)&enchdcp2_key, (const void __user *)arg, sizeof(HDCP2_GET_ENCHDCP2KEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GET_ENCHDCP2KEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				;//rtk_hal_hdcp2_GetEncHdcp2Key(to_user_ptr(enchdcp2_key.pEncKey), to_user_ptr(enchdcp2_key.src_file_path), to_user_ptr(enchdcp2_key.keyLen));

			}
			break;
		}
		case HDCP2_IOC_SET_ENCHDCP2KEY:
		{
			unsigned char *enchdcp2_key;
			if(copy_from_user((void *)&enchdcp2_key, (const void __user *)arg, sizeof(unsigned char *)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SET_ENCHDCP2KEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SET_ENCHDCP2KEY copy_from_user failed!!!!!!!!!!!!!!!\n");
				;//rtk_hal_hdcp2_SetEncHdcp2Key(enchdcp2_key);

			}
			break;
		}
		case HDCP2_IOC_GET_ROOTPUBLICKEY:
		{
			HDCP2_GET_ROOTPUBLICKEY_PARA_T rootpublic_key;
			if(copy_from_user((void *)&rootpublic_key, (const void __user *)arg, sizeof(HDCP2_GET_ROOTPUBLICKEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GET_ROOTPUBLICKEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				;//rtk_hal_hdcp2_GetRootPublicKey(to_user_ptr(rootpublic_key.pRootPublicKey), to_user_ptr(rootpublic_key.src_file_path));
			}
			break;
		}
		case HDCP2_IOC_GET_CERTINFO:
		{
			HDCP2_GET_CERTINFO_PARA_T certinfo;
			if(copy_from_user((void *)&certinfo, (const void __user *)arg, sizeof(HDCP2_GET_CERTINFO_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GET_CERTINFO copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
			#if 0
				unsigned char temp_receiveID[RECEIVER_ID_SIZE] = {0x00};
				unsigned char temp_pubkey[KEY_PUBLIC_RX_SIZE] = {0x00};
				unsigned char temp_rootsign[DCP_LLC_SIG_SIZE] = {0x00};
				certinfo.pReceiverID = temp_receiveID;
				certinfo.pPubKey = temp_receiveID;
				certinfo.pRootSign = temp_rootsign;
				optee_hdcp2_widi_GET_CERTINFO(HDCP2_CMD_GET_CERTINFO, &certinfo,sizeof(HDCP2_GET_CERTINFO_PARA_T));

				if(copy_to_user(to_user_ptr(certinfo.pReceiverID), (void *) temp_receiveID, RECEIVER_ID_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_receiveID copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(copy_to_user(to_user_ptr(certinfo.pPubKey), (void *) temp_pubkey, KEY_PUBLIC_RX_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_pubkey copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(copy_to_user(to_user_ptr(certinfo.pRootSign), (void *) temp_rootsign, DCP_LLC_SIG_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_rootsign copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			#endif

			}
			break;
		}
		case HDCP2_IOC_GET_CERTINFO2:
		{
			HDCP2_GET_CERTINFO_PARA_T certinfo2;
			if(copy_from_user((void *)&certinfo2, (const void __user *)arg, sizeof(HDCP2_GET_CERTINFO_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GET_CERTINFO2 copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_receiveID[RECEIVER_ID_SIZE] = {0x00};
				unsigned char temp_pubkey[KEY_PUBLIC_RX_SIZE] = {0x00};
				unsigned char temp_reserved[RESERVED_SIZE] = {0x00};
				unsigned char temp_rootsign[DCP_LLC_SIG_SIZE] = {0x00};
			
				optee_hdcp2_widi_GET_CERTINFO2(HDCP2_CMD_GET_CERTINFO2, temp_receiveID,temp_pubkey,temp_reserved,temp_rootsign);

				if(copy_to_user(to_user_ptr(certinfo2.pReceiverID), (void *) temp_receiveID, RECEIVER_ID_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_receiveID copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(copy_to_user(to_user_ptr(certinfo2.pPubKey), (void *) temp_pubkey, KEY_PUBLIC_RX_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_pubkey copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(copy_to_user(to_user_ptr(certinfo2.pReserved), (void *) temp_reserved, RESERVED_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_reserved copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(copy_to_user(to_user_ptr(certinfo2.pRootSign), (void *) temp_rootsign, DCP_LLC_SIG_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_rootsign copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}

			}
			break;
		}
		case HDCP2_IOC_DECRYPT_RSAES_OAEP:
		{
			unsigned char pEKpubKm[EKPUBKM_SIZE] = {0x00};
			if(copy_from_user((void *)pEKpubKm, (const void __user *)arg, EKPUBKM_SIZE))
			{
				retval = -EFAULT;
				pr_emerg("scaler hdcp2 ioctl code=HDCP2_IOC_DECRYPT_RSAES_OAEP copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				optee_hdcp2_widi_DECRYPT_RSAES_OAEP(HDCP2_CMD_DECRYPT_RSAES_OAEP, pEKpubKm,EKPUBKM_SIZE);
			}
			break;
		}
		case HDCP2_IOC_KD_KEY_DERIVATION:
		{
			HDCP2_KD_KEY_DERIVATION_PARA_T kd_key_derivation;
			if(copy_from_user((void *)&kd_key_derivation, (const void __user *)arg, sizeof(HDCP2_KD_KEY_DERIVATION_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_KD_KEY_DERIVATION copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_rtx[RTX_SIZE] = {0x00};
				unsigned char temp_rrx[RRX_SIZE] = {0x00};
				if(copy_from_user((void *)temp_rtx, (const void __user *)to_user_ptr(kd_key_derivation.pRtx), RTX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rtx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rrx, (const void __user *)to_user_ptr(kd_key_derivation.pRrx), RRX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rrx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				
				optee_hdcp2_widi_KD_KEY_DERIVATION(HDCP2_CMD_KD_KEY_DERIVATION, temp_rtx,temp_rrx,kd_key_derivation.version);
			}
			break;
		}
		case HDCP2_IOC_COMPUTE_H_PRIME:
		{
			HDCP2_COMPUTE_H_PRIME_PARA_T compute_h_prime;
			if(copy_from_user((void *)&compute_h_prime, (const void __user *)arg, sizeof(HDCP2_COMPUTE_H_PRIME_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_COMPUTE_H_PRIME copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_hprime[H_SIZE] = {0x00};
				unsigned char temp_rtx[RTX_SIZE] = {0x00};
				unsigned char temp_deviceOptionInfo[RXCAPS_SIZE + TXCAPS_SIZE] = {0x00};

				if(copy_from_user((void *)temp_rtx, (const void __user *)to_user_ptr(compute_h_prime.pRtx), RTX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rtx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_deviceOptionInfo, (const void __user *)to_user_ptr(compute_h_prime.deviceOptionInfo), RXCAPS_SIZE + TXCAPS_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_deviceOptionInfo copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				optee_hdcp2_widi_Compute_H_Prime(HDCP2_CMD_COMPUTE_H_PRIME, compute_h_prime.repeater, compute_h_prime.version,temp_rtx, temp_deviceOptionInfo, temp_hprime);
				if(copy_to_user(to_user_ptr(compute_h_prime.pHPrime), (void *) temp_hprime, H_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_hprime copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			}
			break;
		}
		case HDCP2_IOC_COMPUTE_L_PRIME:
		{
			HDCP2_COMPUTE_L_PRIME_PARA_T compute_l_prime;
			if(copy_from_user((void *)&compute_l_prime, (const void __user *)arg, sizeof(HDCP2_COMPUTE_L_PRIME_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_COMPUTE_L_PRIME copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_lprime[L_SIZE] = {0x00};
				unsigned char temp_rn[RN_SIZE] = {0x00};
				unsigned char temp_rrx[RRX_SIZE] = {0x00};

				if(copy_from_user((void *)temp_rn, (const void __user *)to_user_ptr(compute_l_prime.pRn), RN_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rn copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rrx, (const void __user *)to_user_ptr(compute_l_prime.pRrx), RRX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rrx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				optee_hdcp2_widi_Compute_L_Prime(HDCP2_CMD_COMPUTE_L_PRIME, compute_l_prime.version,temp_rn, temp_rrx, temp_lprime);

				if(copy_to_user(to_user_ptr(compute_l_prime.pLPrime), (void *) temp_lprime, L_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_lprime copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			}
			break;
		}
		case HDCP2_IOC_COMPUTE_KH:
		{
			optee_hdcp2_widi_Compute_KH(HDCP2_CMD_COMPUTE_KH);
			break;
		}
		case HDCP2_IOC_ENCRYPT_KM_USING_KH:
		{
			HDCP2_ENCRYPT_KM_USING_KH_PARA_T encrypt_km;
			if(copy_from_user((void *)&encrypt_km, (const void __user *)arg, sizeof(HDCP2_ENCRYPT_KM_USING_KH_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_ENCRYPT_KM_USING_KH copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_ekhkm[EKHKM_SIZE] = {0x00};
				unsigned char temp_m[M_SIZE] = {0x00};

				if(copy_from_user((void *)temp_m, (const void __user *)to_user_ptr(encrypt_km.pM), M_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_m copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				optee_hdcp2_widi_Encrypt_Km_using_Kh(HDCP2_CMD_Encrypt_Km_using_Kh, temp_m, temp_ekhkm);
				
				if(copy_to_user(to_user_ptr(encrypt_km.pEKH_KM), (void *) temp_ekhkm, EKHKM_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_ekhkm copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			}
			break;
		}
		case HDCP2_IOC_DECRYPT_KM_USING_KH:
		{
			HDCP2_ENCRYPT_KM_USING_KH_PARA_T decrypt_km;
			if(copy_from_user((void *)&decrypt_km, (const void __user *)arg, sizeof(HDCP2_ENCRYPT_KM_USING_KH_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DECRYPT_KM_USING_KH copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_ekhkm[EKHKM_SIZE] = {0x00};
				unsigned char temp_m[M_SIZE] = {0x00};

				if(copy_from_user((void *)temp_ekhkm, (const void __user *)to_user_ptr(decrypt_km.pEKH_KM), EKHKM_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_ekhkm copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_m, (const void __user *)to_user_ptr(decrypt_km.pM), M_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_m copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				
				optee_hdcp2_widi_Decrypt_Km_using_Kh(HDCP2_CMD_Decrypt_Km_using_Kh, temp_m, temp_ekhkm);
			}
			break;
		}
		case HDCP2_IOC_DECRYPT_EKS:
		{
			HDCP2_DECRYPT_EKS_PARA_T decrypt_eks;
			if(copy_from_user((void *)&decrypt_eks, (const void __user *)arg, sizeof(HDCP2_DECRYPT_EKS_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DECRYPT_EKS copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_eks[EDKEYKS_SIZE] = {0x00};
				unsigned char temp_rtx[RTX_SIZE] = {0x00};
				unsigned char temp_rrx[RRX_SIZE] = {0x00};
				unsigned char temp_rn[RN_SIZE] = {0x00};

				if(copy_from_user((void *)temp_eks, (const void __user *)to_user_ptr(decrypt_eks.pEks), EDKEYKS_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_eks copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rtx, (const void __user *)to_user_ptr(decrypt_eks.pRtx), RTX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rtx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rrx, (const void __user *)to_user_ptr(decrypt_eks.pRrx), RRX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rrx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rn, (const void __user *)to_user_ptr(decrypt_eks.pRn), RN_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rn copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				optee_hdcp2_widi_Decrypt_EKS(HDCP2_CMD_Decrypt_EKS, decrypt_eks.version, temp_rtx, temp_rrx, temp_rn, temp_eks);
			}
			break;
		}
		case HDCP2_IOC_XOR_KS_WITH_LC128:
		{
			optee_hdcp2_widi_XOR_Ks_with_LC128(HDCP2_CMD_XOR_Ks_with_LC128);
			break;
		}
		case HDCP2_IOC_GENERATE_KM:
		{
			HDCP2_GENERATE_KM_PARA_T Gen_km;
			if(copy_from_user((void *)&Gen_km, (const void __user *)arg, sizeof(HDCP2_GENERATE_KM_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GENERATE_KM copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_maskedDB[DB_SIZE+4] = {0x00};
				unsigned char temp_db[DB_SIZE] = {0x00};
				unsigned char temp_dbmask[DB_SIZE] = {0x00};

				if(copy_from_user((void *)temp_db, (const void __user *)to_user_ptr(Gen_km.pDB), DB_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_db copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_dbmask, (const void __user *)to_user_ptr(Gen_km.pDBMask), DB_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_dbmask copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				optee_hdcp2_widi_Generate_Km(HDCP2_CMD_Generate_Km, temp_db, temp_dbmask, temp_maskedDB);

				if(copy_to_user(to_user_ptr(Gen_km.pMaskedDB), (void *) temp_maskedDB, DB_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_maskedDB copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			}
			break;
		}
		case HDCP2_IOC_GENERATE_KS:
		{
			HDCP2_GENERATE_KS_PARA_T Gen_ks;
			if(copy_from_user((void *)&Gen_ks, (const void __user *)arg, sizeof(HDCP2_GENERATE_KS_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GENERATE_KS copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_eks[EDKEYKS_SIZE] = {0x00};
				unsigned char temp_rtx[RTX_SIZE] = {0x00};
				unsigned char temp_rrx[RRX_SIZE] = {0x00};
				unsigned char temp_rn[RN_SIZE] = {0x00};

				if(copy_from_user((void *)temp_rtx, (const void __user *)to_user_ptr(Gen_ks.pRtx), RTX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rtx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rrx, (const void __user *)to_user_ptr(Gen_ks.pRrx), RRX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rrx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)temp_rn, (const void __user *)to_user_ptr(Gen_ks.pRn), RN_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_rn copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				
				optee_hdcp2_widi_Generate_KS(HDCP2_CMD_Generate_KS, Gen_ks.version, temp_rtx, temp_rrx, temp_rn, temp_eks);
				if(copy_to_user(to_user_ptr(Gen_ks.pEks), (void *) temp_eks, EDKEYKS_SIZE))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_eks copy_to_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			}
			break;
		}
		case HDCP2_IOC_DATA_DECRYPT:
#if 1
		{
			HDCP2_DATA_DECRYPT_PARA_T data;
			if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(HDCP2_DATA_DECRYPT_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DATA_DECRYPT copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_counter[16] = {0x00};
				unsigned char KsxorLc128[16] = {0x00};
				if(copy_from_user((void *)temp_counter, (const void __user *)to_user_ptr(data.pCounter), 16))
		        	{
		            			retval = -EFAULT;
		            			DBG_PRINT("scaler hdcp2 ioctl code=temp_counter copy_from_user failed!!!!!!!!!!!!!!!\n");
		            			break;
		        	}

				if (hdcp_buff==NULL || (data.length < 512 && data.length >= 0))
                {
                		unsigned char *hdcp_outputbuf = NULL;
                		unsigned char *hdcp_inputbuf =NULL;
						hdcp_outputbuf = kmalloc(512, GFP_KERNEL);
						if(hdcp_outputbuf == NULL) {
							break;
						}
						hdcp_inputbuf = kmalloc(512, GFP_KERNEL);
						if(hdcp_inputbuf == NULL) {
							kfree(hdcp_outputbuf);
							hdcp_outputbuf = NULL;
							break;
						}
						memset(hdcp_outputbuf,0x00,512);
						memset(hdcp_inputbuf,0x00,512);
						
						if(copy_from_user((void *)hdcp_inputbuf, (const void __user *)to_user_ptr(data.pInputBuf), (unsigned int)data.length))
		        			{
		            				retval = -EFAULT;
									kfree(hdcp_outputbuf);
									kfree(hdcp_inputbuf);
									hdcp_outputbuf = NULL;
									hdcp_inputbuf = NULL;
		            				DBG_PRINT("scaler hdcp2 ioctl code=hdcp_inputbuf copy_from_user failed!!!!!!!!!!!!!!!\n");
		            				break;
		        			}
				
                    				//optee_hdcp2_widi_Get_KsXorLc128(HDCP2_CMD_Get_KsXorLc128, KsxorLc128, 16);
                    				rtk_hdcp2_get_ksxorlc128(KsxorLc128, 16);
									
#if IS_ENABLED(CONFIG_RTK_KDRV_MCP)
									MCP_AES_CTR_Decryption(KsxorLc128, temp_counter, hdcp_inputbuf, hdcp_outputbuf, (data.length + 15) & (~0xF));
#endif									

						if(copy_to_user(to_user_ptr(data.pOutputBuf), (void *) hdcp_outputbuf, (unsigned int)data.length))
    			        		{
    			           			retval = -EFAULT;
									kfree(hdcp_outputbuf);
									kfree(hdcp_inputbuf);
									hdcp_outputbuf = NULL;
									hdcp_inputbuf = NULL;
				            		DBG_PRINT("scaler hdcp2 ioctl code=hdcp_outputbuf copy_from_user failed!!!!!!!!!!!!!!!\n");
				            		break;
    			        		}

							kfree(hdcp_outputbuf);
							kfree(hdcp_inputbuf);
							hdcp_outputbuf = NULL;
							hdcp_inputbuf = NULL;
                			}
                			else
                			{
    			    		while(data.length)
    			    		{
    			        			unsigned int len = (data.length >= HDCP2_TMP_BUFFER_SIZE) ?  HDCP2_TMP_BUFFER_SIZE : data.length;
								//	optee_hdcp2_widi_Get_KsXorLc128(HDCP2_CMD_Get_KsXorLc128, KsxorLc128, 16);
								rtk_hdcp2_get_ksxorlc128(KsxorLc128, 16);
                        					if(copy_from_user((void *)hdcp_buff, (const void __user *)to_user_ptr(data.pInputBuf), len))
    			        			{
    			            				retval = -EFAULT;
				            			DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DATA_DECRYPT copy_from_user failed!!!!!!!!!!!!!!!\n");
				            			break;
    			        			}
#if IS_ENABLED(CONFIG_RTK_KDRV_MCP)
    			        			MCP_AES_CTR_Decryption(KsxorLc128, temp_counter, hdcp_buff, hdcp_buff, (len + 15) & (~0xF));

                        				MCP_AES_PeekIV(temp_counter);  // peek iv
#endif
    			        			if(copy_to_user(to_user_ptr(data.pOutputBuf), (void *) hdcp_buff, len))
    			        			{
    			           				retval = -EFAULT;
				            			DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DATA_DECRYPT copy_from_user failed!!!!!!!!!!!!!!!\n");
				            			break;
    			        			}

    			        			data.pInputBuf += len;
    			        			data.pOutputBuf += len;
    			        			data.length -= len;
    			    		}
    				}
			}
			break;
		}
#else
		{
			HDCP2_DATA_DECRYPT_PARA_T data;
			if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(HDCP2_DATA_DECRYPT_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DATA_DECRYPT copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_counter[16] = {0x00};
				if(copy_from_user((void *)temp_counter, (const void __user *)to_user_ptr(data.pCounter), 16))
	        	{
	            		retval = -EFAULT;
	            		DBG_PRINT("scaler hdcp2 ioctl code=temp_counter copy_from_user failed!!!!!!!!!!!!!!!\n");
	            		break;
	        	}

				if (data.length >= 0)
                {
                		unsigned char *hdcp_outputbuf = NULL;
                		unsigned char *hdcp_inputbuf =NULL;
						hdcp_outputbuf = kmalloc(data.length, GFP_KERNEL);
						if(hdcp_outputbuf == NULL) {
							break;
						}
						hdcp_inputbuf = kmalloc(data.length, GFP_KERNEL);
						if(hdcp_inputbuf == NULL) {
							kfree(hdcp_outputbuf);
							hdcp_outputbuf = NULL;
							break;
						}
						memset(hdcp_outputbuf,0x00,data.length);
						memset(hdcp_inputbuf,0x00,data.length);
						
						if(copy_from_user((void *)hdcp_inputbuf, (const void __user *)to_user_ptr(data.pInputBuf), (unsigned int)data.length))
        				{
            				retval = -EFAULT;
							kfree(hdcp_outputbuf);
							kfree(hdcp_inputbuf);
							hdcp_outputbuf = NULL;
							hdcp_inputbuf = NULL;
            				DBG_PRINT("scaler hdcp2 ioctl code=hdcp_inputbuf copy_from_user failed!!!!!!!!!!!!!!!\n");
            				break;
        				}
			
						optee_hdcp2_widi_DATA_DECRYPT(HDCP2_CMD_DATA_DECRYPT, temp_counter, hdcp_inputbuf, hdcp_outputbuf, data.length);

						if(copy_to_user(to_user_ptr(data.pOutputBuf), (void *) hdcp_outputbuf, (unsigned int)data.length))
		        		{
		           			retval = -EFAULT;
							kfree(hdcp_outputbuf);
							kfree(hdcp_inputbuf);
							hdcp_outputbuf = NULL;
							hdcp_inputbuf = NULL;
		            		DBG_PRINT("scaler hdcp2 ioctl code=hdcp_outputbuf copy_from_user failed!!!!!!!!!!!!!!!\n");
		            		break;
		        		}

						kfree(hdcp_outputbuf);
						kfree(hdcp_inputbuf);
						hdcp_outputbuf = NULL;
						hdcp_inputbuf = NULL;
                	}
			}
			break;
		}
#endif
		case HDCP2_IOC_SHA1:
		{
			HDCP2_SHA_PARA_T sha1;
			if(copy_from_user((void *)&sha1, (const void __user *)arg, sizeof(HDCP2_SHA_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SHA1 copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *in_ptr = kmalloc(sha1.length, GFP_KERNEL);
				unsigned char *out_ptr = kmalloc(20, GFP_KERNEL);
				if((in_ptr == NULL) ||(out_ptr == NULL))
				{
					if(in_ptr != NULL) {
						kfree(in_ptr);
						in_ptr = NULL;
					}
					if(out_ptr != NULL) {
						kfree(out_ptr);
						out_ptr = NULL;
					}
					DBG_PRINT("in_ptr/out_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)in_ptr, (const void __user *)to_user_ptr(sha1.pInputBuf), sha1.length))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				
				
				optee_hdcp2_widi_SHA1(HDCP2_CMD_SHA1, in_ptr, sha1.length, out_ptr, 20);

				if(copy_to_user(to_user_ptr(sha1.pOutputBuf), (void *)out_ptr, 20))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=out_ptr copy_to_user failed!!!!!!!!!!!!!!!\n");
        			//break;
    			}
				
				kfree(in_ptr);
				kfree(out_ptr);
				in_ptr = NULL;
				out_ptr = NULL;
			}
			break;
		}
		case HDCP2_IOC_SHA256:
		{
			HDCP2_SHA_PARA_T sha256;
			if(copy_from_user((void *)&sha256, (const void __user *)arg, sizeof(HDCP2_SHA_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SHA256 copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *in_ptr = kmalloc(sha256.length, GFP_KERNEL);
				unsigned char *out_ptr = kmalloc(32, GFP_KERNEL);
				if((in_ptr == NULL) ||(out_ptr == NULL))
				{
					if(in_ptr != NULL) {
						kfree(in_ptr);
						in_ptr = NULL;
					}
					if(out_ptr != NULL) {
						kfree(out_ptr);
						out_ptr = NULL;
					}
					DBG_PRINT("in_ptr/out_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)in_ptr, (const void __user *) to_user_ptr(sha256.pInputBuf), sha256.length))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				optee_hdcp2_widi_SHA256(HDCP2_CMD_SHA256, in_ptr, sha256.length, out_ptr, 32);

				if(copy_to_user(to_user_ptr(sha256.pOutputBuf), (void *)out_ptr, 32))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=out_ptr copy_to_user failed!!!!!!!!!!!!!!!\n");
            				//break;
        			}

				kfree(in_ptr);
				kfree(out_ptr);
				in_ptr = NULL;
				out_ptr = NULL;
			}
			break;
		}
		case HDCP2_IOC_HMAC_SHA256:
		{
			HDCP2_HMAC_SHA256_PARA_T hmac_sha256;
			if(copy_from_user((void *)&hmac_sha256, (const void __user *)arg, sizeof(HDCP2_HMAC_SHA256_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_HMAC_SHA256 copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *in_ptr = NULL;
				unsigned char *out_ptr = NULL;
				unsigned char *key_ptr = NULL;
				if(hmac_sha256.keylength < 0)
					break;
				in_ptr = kmalloc(hmac_sha256.length, GFP_KERNEL);
				out_ptr = kmalloc(32, GFP_KERNEL);
				key_ptr = kmalloc((unsigned int)hmac_sha256.keylength, GFP_KERNEL);
				if((in_ptr == NULL) ||(out_ptr == NULL) || (key_ptr == NULL))
				{
					if(in_ptr != NULL) {
						kfree(in_ptr);
						in_ptr = NULL;
					}
					if(out_ptr != NULL) {
						kfree(out_ptr);
						out_ptr = NULL;
					}
					if(key_ptr != NULL) {
						kfree(key_ptr);
						key_ptr = NULL;
					}
					DBG_PRINT("in_ptr/out_ptr/key_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)in_ptr, (const void __user *) to_user_ptr(hmac_sha256.pInputBuf), hmac_sha256.length))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)key_ptr, (const void __user *) to_user_ptr(hmac_sha256.pKey), (unsigned int)hmac_sha256.keylength))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				if(hmac_sha256.keylength >= 0)
					optee_hdcp2_widi_HMAC_SHA256(HDCP2_CMD_HMAC_SHA256, in_ptr, hmac_sha256.length, key_ptr, (unsigned int)hmac_sha256.keylength, out_ptr, 32);

				if(copy_to_user(to_user_ptr(hmac_sha256.pOutputBuf), (void *)out_ptr, 32))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=out_ptr copy_to_user failed!!!!!!!!!!!!!!!\n");
            				//break;
        			}

				kfree(in_ptr);
				kfree(out_ptr);
				kfree(key_ptr);
				in_ptr = NULL;
				out_ptr = NULL;
				key_ptr = NULL;
			}
			break;
		}
		case HDCP2_IOC_AES_CTR:
		{
			HDCP2_AES_CTR_PARA_T aes_ctr;
			if(copy_from_user((void *)&aes_ctr, (const void __user *)arg, sizeof(HDCP2_AES_CTR_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_AES_CTR_PARA_T copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{	
				unsigned char *in_ptr = NULL;
				unsigned char *out_ptr = NULL;
				unsigned char *key_ptr = NULL;
				unsigned char counter[16];
				if(aes_ctr.length < 0 || aes_ctr.keyLength < 0)
					break;
				in_ptr = kmalloc((unsigned int)aes_ctr.length, GFP_KERNEL);
				out_ptr = kmalloc((unsigned int)aes_ctr.length, GFP_KERNEL);
				key_ptr = kmalloc((unsigned int)aes_ctr.keyLength, GFP_KERNEL);
				
				if((in_ptr == NULL) ||(out_ptr == NULL) || (key_ptr == NULL))
				{
					if(in_ptr != NULL) {
						kfree(in_ptr);
						in_ptr = NULL;
					}
					if(out_ptr != NULL) {
						kfree(out_ptr);
						out_ptr = NULL;
					}
					if(key_ptr != NULL) {
						kfree(key_ptr);
						key_ptr = NULL;
					}
					DBG_PRINT("in_ptr/out_ptr/key_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)in_ptr, (const void __user *) to_user_ptr(aes_ctr.pInputBuf), (unsigned int)aes_ctr.length))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)key_ptr, (const void __user *)to_user_ptr(aes_ctr.pkey), (unsigned int)aes_ctr.keyLength))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)counter, (const void __user *)to_user_ptr(aes_ctr.pCounter), 16))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				if((aes_ctr.length >= 0) && (aes_ctr.keyLength >= 0))
					optee_hdcp2_widi_AES_CTR(HDCP2_CMD_AES_CTR, in_ptr, (unsigned int)aes_ctr.length, key_ptr, (unsigned int)aes_ctr.keyLength, counter, 16, out_ptr, (unsigned int)aes_ctr.length);

				if(copy_to_user(to_user_ptr(aes_ctr.pOutputBuf), (void *)out_ptr, (unsigned int)aes_ctr.length))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=out_ptr copy_to_user failed!!!!!!!!!!!!!!!\n");
            				//break;
        			}

				kfree(in_ptr);
				kfree(out_ptr);
				kfree(key_ptr);
				in_ptr = NULL;
				out_ptr = NULL;
				key_ptr = NULL;
			}
			break;
		}
		case HDCP2_IOC_SELECT_STOREDKM:
		{
			int index;
			if(copy_from_user((void *)&index, (const void __user *)arg, sizeof(int)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SELECT_STOREDKM copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				optee_hdcp2_widi_Select_StoredKm(HDCP2_CMD_SELECT_STOREDKM, index);
			}
			break;
		}
		case HDCP2_IOC_WRITE_STOREDKM:
		{
			int index;
			if(copy_from_user((void *)&index, (const void __user *)arg, sizeof(int)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_WRITE_STOREDKM copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				optee_hdcp2_widi_Write_StoredKm(HDCP2_CMD_WRITE_STOREDKM, index);
			}
			break;
		}
		case HDCP2_IOC_DATA_ENCRYPT:
		{
			HDCP2_DATA_DECRYPT_PARA_T data;
			if(copy_from_user((void *)&data, (const void __user *)arg, sizeof(HDCP2_DATA_DECRYPT_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DATA_ENCRYPT copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_counter[16] = {0x00};
				unsigned char KsxorLc128[16] = {0x00};
				if(copy_from_user((void *)temp_counter, (const void __user *)to_user_ptr(data.pCounter), 16))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_counter copy_from_user failed!!!!!!!!!!!!!!!\n");
					break;
				}

				if (hdcp_buff==NULL || (data.length < 512 && data.length >= 0))
				{
					unsigned char *hdcp_outputbuf = NULL;
					unsigned char *hdcp_inputbuf =NULL;
					hdcp_outputbuf = kmalloc(512, GFP_KERNEL);
					if(hdcp_outputbuf == NULL) {
						break;
					}
					hdcp_inputbuf = kmalloc(512, GFP_KERNEL);
					if(hdcp_inputbuf == NULL) {
						kfree(hdcp_outputbuf);
						hdcp_outputbuf = NULL;
						break;
					}
					memset(hdcp_outputbuf,0x00,512);
					memset(hdcp_inputbuf,0x00,512);

					if(copy_from_user((void *)hdcp_inputbuf, (const void __user *)to_user_ptr(data.pInputBuf), (unsigned int)data.length))
					{
						retval = -EFAULT;
						kfree(hdcp_outputbuf);
						kfree(hdcp_inputbuf);
						hdcp_outputbuf = NULL;
						hdcp_inputbuf = NULL;
						DBG_PRINT("scaler hdcp2 ioctl code=hdcp_inputbuf copy_from_user failed!!!!!!!!!!!!!!!\n");
						break;
					}

					//optee_hdcp2_widi_Get_KsXorLc128(HDCP2_CMD_Get_KsXorLc128, KsxorLc128, 16);
					rtk_hdcp2_get_ksxorlc128(KsxorLc128, 16);

#if IS_ENABLED(CONFIG_RTK_KDRV_MCP)
					MCP_AES_CTR_Encryption(KsxorLc128, temp_counter, hdcp_inputbuf, hdcp_outputbuf, (data.length + 15) & (~0xF));
#endif

					if(copy_to_user(to_user_ptr(data.pOutputBuf), (void *) hdcp_outputbuf, (unsigned int)data.length))
					{
						retval = -EFAULT;
						kfree(hdcp_outputbuf);
						kfree(hdcp_inputbuf);
						hdcp_outputbuf = NULL;
						hdcp_inputbuf = NULL;
						DBG_PRINT("scaler hdcp2 ioctl code=hdcp_outputbuf copy_from_user failed!!!!!!!!!!!!!!!\n");
						break;
					}

					kfree(hdcp_outputbuf);
					kfree(hdcp_inputbuf);
					hdcp_outputbuf = NULL;
					hdcp_inputbuf = NULL;
				}
				else
				{
					while(data.length)
					{
						unsigned int len = (data.length >= HDCP2_TMP_BUFFER_SIZE) ?  HDCP2_TMP_BUFFER_SIZE : data.length;
						//optee_hdcp2_widi_Get_KsXorLc128(HDCP2_CMD_Get_KsXorLc128, KsxorLc128, 16);
						rtk_hdcp2_get_ksxorlc128(KsxorLc128, 16);

						if(copy_from_user((void *)hdcp_buff, (const void __user *)to_user_ptr(data.pInputBuf), len))
						{
							retval = -EFAULT;
							DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DATA_ENCRYPT copy_from_user failed!!!!!!!!!!!!!!!\n");
							break;
						}
#if IS_ENABLED(CONFIG_RTK_KDRV_MCP)
						MCP_AES_CTR_Encryption(KsxorLc128, temp_counter, hdcp_buff, hdcp_buff, (len + 15) & (~0xF));

						MCP_AES_PeekIV(temp_counter);  // peek iv
#endif
						if(copy_to_user(to_user_ptr(data.pOutputBuf), (void *) hdcp_buff, len))
						{
							retval = -EFAULT;
							DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DATA_ENCRYPT copy_from_user failed!!!!!!!!!!!!!!!\n");
							break;
						}

						data.pInputBuf += len;
						data.pOutputBuf += len;
						data.length -= len;
					}
				}
			}
			break;
		}
		case HDCP2_IOC_GET_PROTECTED_DECRYPTIONKEY:
		{
			HDCP2_GET_PROTECTED_DECRYPTIONKEY_PARA_T get_prokey;
			if(copy_from_user((void *)&get_prokey, (const void __user *)arg, sizeof(HDCP2_GET_PROTECTED_DECRYPTIONKEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_GET_PROTECTED_DECRYPTIONKEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_data[32];

				optee_hdcp2_widi_GET_PROTECTED_DECRYPTIONKEY(HDCP2_CMD_GET_PROTECTED_DECRYPTIONKEY, temp_data, 32);
				rtk_hdcp2_gen_ksxorlc128(temp_data);
				if(copy_to_user(to_user_ptr(get_prokey.pData), (void *) temp_data, 32))
        			{
           				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_data copy_from_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
			}
			break;
		}
		case HDCP2_IOC_SET_PROTECTED_DECRYPTIONKEY:
		{
			HDCP2_SET_PROTECTED_DECRYPTIONKEY_PARA_T set_prokey;
			if(copy_from_user((void *)&set_prokey, (const void __user *)arg, sizeof(HDCP2_SET_PROTECTED_DECRYPTIONKEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SET_PROTECTED_DECRYPTIONKEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_data[32] = {0};
				if(set_prokey.length < 0)
					break;
				if(copy_from_user((void *)temp_data, (const void __user *)to_user_ptr(set_prokey.pData), (unsigned int)set_prokey.length))
        			{
            				retval = -EFAULT;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_data copy_from_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(set_prokey.length >= 0)
					optee_hdcp2_widi_SET_PROTECTED_DECRYPTIONKEY(HDCP2_CMD_SET_PROTECTED_DECRYPTIONKEY, temp_data, (unsigned int)set_prokey.length);
			}
			break;
		}
		case HDCP2_IOC_HDCP_INIT:
		{
			rtk_hal_hdcp_Init();
			break;
		}
		case HDCP2_IOC_GET_OTPHDCP2KEY:
		{
			HDCP2_GET_OTPHDCP2KEY_PARA_T HDCP2_OTP_KEY;
			if(copy_from_user((void *)&HDCP2_OTP_KEY, (const void __user *)arg, sizeof(HDCP2_GET_OTPHDCP2KEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_SET_PROTECTED_DECRYPTIONKEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				int temp_exsit = 0;
				unsigned char *temp_data = kmalloc(1024, GFP_KERNEL);
				if(temp_data == NULL) {
					break;
				}
				memset(temp_data,0,1024);
				

#if IS_ENABLED(CONFIG_RTK_KDRV_OTP)
#if defined(CONFIG_RTK_KDRV_HDCP_KEY_FROM_OTP)
								if(get_hdcp_22_otp(temp_data, HDCP2_OTP_KEY.length) == 0)
									temp_exsit = 1;
								else
#endif
									temp_exsit = 0;
#endif

				if(copy_to_user(to_user_ptr(HDCP2_OTP_KEY.pData), (void *) temp_data, HDCP2_OTP_KEY.length))
        			{
           				retval = -EFAULT;
						kfree(temp_data);
						temp_data = NULL;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_data copy_from_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				if(copy_to_user((void __user *)to_user_ptr(HDCP2_OTP_KEY.exsit), (void *) &temp_exsit, sizeof(int)))
        			{
           				retval = -EFAULT;
						kfree(temp_data);
						temp_data = NULL;
            				DBG_PRINT("scaler hdcp2 ioctl code=temp_data copy_from_user failed!!!!!!!!!!!!!!!\n");
            				break;
        			}
				kfree(temp_data);
				temp_data = NULL;
			}
			break;
		}
		case HDCP2_IOC_COMPUTE_V_PRIME:
		{
			HDCP2_COMPUTE_V_PRIME_PARA_T compute_v_prime;
			if(copy_from_user((void *)&compute_v_prime, (const void __user *)arg, sizeof(HDCP2_COMPUTE_V_PRIME_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_COMPUTE_V_PRIME_PARA_T copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_vprime[VPRIME_SIZE] = {0x00};
				unsigned char temp_listinfo[LIST_SIZE] = {0x00};

				if(compute_v_prime.infoLength < 0)
					break;
				if(copy_from_user((void *)temp_listinfo, (const void __user *)to_user_ptr(compute_v_prime.pRxIdListsInfo), (unsigned int)compute_v_prime.infoLength))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_listinfo copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				if(compute_v_prime.infoLength >= 0)
					optee_hdcp2_widi_Compute_V_Prime(HDCP2_CMD_Compute_V_Prime, temp_listinfo, (unsigned int)compute_v_prime.infoLength, temp_vprime, VPRIME_SIZE);

				if(copy_to_user(to_user_ptr(compute_v_prime.pVPrime), (void *) temp_vprime, VPRIME_SIZE))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=temp_vprime copy_from_user failed!!!!!!!!!!!!!!!\n");
        			break;
    			}

			}
			break;
		}
		case HDCP2_IOC_COMPUTE_M_PRIME:
		{
			HDCP2_COMPUTE_M_PRIME_PARA_T compute_m_prime;
			if(copy_from_user((void *)&compute_m_prime, (const void __user *)arg, sizeof(HDCP2_COMPUTE_M_PRIME_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_COMPUTE_M_PRIME_PARA_T copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_mprime[MPRIME_SIZE] = {0x00};
				unsigned char temp_streaminfo[STREAM_SIZE] = {0x00};

				if(compute_m_prime.infoLength < 0)
					break;
				if(copy_from_user((void *)temp_streaminfo, (const void __user *)to_user_ptr(compute_m_prime.pStreamManageInfo), (unsigned int)compute_m_prime.infoLength))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_streaminfo copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				if(compute_m_prime.infoLength >= 0)
					optee_hdcp2_widi_Compute_M_Prime(HDCP2_CMD_Compute_M_Prime, temp_streaminfo, (unsigned int)compute_m_prime.infoLength, temp_mprime, MPRIME_SIZE);

				if(copy_to_user(to_user_ptr(compute_m_prime.pMPrime), (void *) temp_mprime, MPRIME_SIZE))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=temp_mprime copy_from_user failed!!!!!!!!!!!!!!!\n");
        			break;
    			}

			}
			break;
		}
		case HDCP2_IOC_GET_HDCP_STATUS:
		{
			HDCP2_GET_HDCP_STATUS_PARA_T GET_HDCP_STATUS;
			if(copy_from_user((void *)&GET_HDCP_STATUS, (const void __user *)arg, sizeof(HDCP2_GET_HDCP_STATUS_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_GET_HDCP_STATUS_PARA_T copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char b14status = 0;
				unsigned char b22status = 0;
				rtk_hal_hdcp2_get_hdcp_status(&b14status, &b22status);

				if(copy_to_user(to_user_ptr(GET_HDCP_STATUS.b14Status), (void *) &b14status, sizeof(unsigned char)))
				{
	   				retval = -EFAULT;
	    			DBG_PRINT("scaler hdcp2 ioctl code=b14status copy_from_user failed!!!!!!!!!!!!!!!\n");
	    			break;
				}

				if(copy_to_user(to_user_ptr(GET_HDCP_STATUS.b22Status), (void *) &b22status, sizeof(unsigned char)))
				{
	   				retval = -EFAULT;
	    			DBG_PRINT("scaler hdcp2 ioctl code=b22status copy_from_user failed!!!!!!!!!!!!!!!\n");
	    			break;
				}
			}
			break;
		}
		case HDCP2_IOC_UseTestVector:
		{
			HDCP2_UseTestVector_PARA_T usetestvector;
			//HDCP2_UseTestVector_local_PARA test_vector;
			if(copy_from_user((void *)&usetestvector, (const void __user *)arg, sizeof(HDCP2_UseTestVector_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_UseTestVector copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else if(usetestvector.argc == 0)
			{
				rtk_hal_hdcp2_UseTestVector(0, NULL);
			}
			else
			{
				unsigned char *lc128 = kmalloc(LC128_SIZE, GFP_KERNEL);
				unsigned char *km = kmalloc(KM_SIZE, GFP_KERNEL);
				unsigned char *ks = kmalloc(KS_SIZE, GFP_KERNEL);
				unsigned char *seed = kmalloc(SEED_SIZE, GFP_KERNEL);
				unsigned char *testvector = kmalloc(LC128_SIZE+KM_SIZE+KS_SIZE+SEED_SIZE, GFP_KERNEL);
				if((lc128 == NULL) ||(km == NULL) || (ks == NULL) || (seed == NULL) || (testvector == NULL))
				{
					if(lc128 != NULL) {
						kfree(lc128);
						lc128 = NULL;
					}
					if(km != NULL) {
						kfree(km);
						km = NULL;
					}
					if(ks != NULL) {
						kfree(ks);
						ks = NULL;
					}
					if(seed != NULL) {
						kfree(seed);
						seed = NULL;
					}
					if(testvector != NULL) {
						kfree(testvector);
						testvector = NULL;
					}
					DBG_PRINT("lc128/km/ks/seed kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)lc128, (const void __user *) to_user_ptr(usetestvector.lc128), LC128_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)km, (const void __user *)to_user_ptr(usetestvector.km), KM_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)ks, (const void __user *)to_user_ptr(usetestvector.ks), KS_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				if(copy_from_user((void *)seed, (const void __user *)to_user_ptr(usetestvector.seed), SEED_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				#if 0
				test_vector.argc = usetestvector.argc;
				test_vector.lc128 =lc128;
				test_vector.km = km;
				test_vector.ks = ks;
				test_vector.seed = seed;
				rtk_hal_hdcp2_UseTestVector(usetestvector.argc, &test_vector);
				#endif
				memcpy(testvector, lc128, LC128_SIZE);
				memcpy(testvector+LC128_SIZE, km, KM_SIZE);
				memcpy(testvector+LC128_SIZE+KM_SIZE, ks, KS_SIZE);
				memcpy(testvector+LC128_SIZE+KM_SIZE+KS_SIZE, seed, SEED_SIZE);
				optee_hdcp2_widi_UseTestVector(HDCP2_CMD_UseTestVector, usetestvector.argc, testvector,LC128_SIZE+KM_SIZE+KS_SIZE+SEED_SIZE);


				kfree(lc128);
				kfree(km);
				kfree(ks);
				kfree(seed);
				kfree(testvector);
				lc128 = NULL;
				km = NULL;
				ks = NULL;
				seed = NULL;
				testvector = NULL;
			}
			break;
		}		
		case HDCP2_IOC_WRITE_HDCP2_KEY:
		{
#ifdef CONFIG_CUSTOMER_TV010
			unsigned char* ap_local_key = (unsigned char*)arg;
			printk("[DEBUG]COPY_HDCP2_KEY\n");
			//printk("k>>>>>>>>>>>>>>>>>>>>>>>>arg=0x%x\n", arg);

			//printk("1ap_local_key>>>>>>>>>>>>>>>>>>>>>>>>%x %x %x %x\n", *ap_local_key,*(ap_local_key+1),*(ap_local_key+2),*(ap_local_key+3));
			//printk("2ap_local_key>>>>>>>>>>>>>>>>>>>>>>>>%x %x %x %x\n", *(ap_local_key+4),*(ap_local_key+5),*(ap_local_key+6),*(ap_local_key+7));

			//printk("1orig loacl_key>>>>>>>>>>>>>>>>>>>>>>>>%x %x %x %x\n",loacl_key[0],loacl_key[1],loacl_key[2],loacl_key[3]);
			//printk("2orig loacl_key>>>>>>>>>>>>>>>>>>>>>>>>%x %x %x %x\n",loacl_key[4],loacl_key[5],loacl_key[6],loacl_key[7]);
			//memcpy(loacl_key, ap_local_key, 902);
			if(copy_from_user((void *)loacl_key, (const void __user *)ap_local_key, 902))
					retval = -EFAULT;
			printk("3kernel loacl_key>>>>>>>>>>>>>>>>>>>>>>>>%x %x %x %x\n",loacl_key[0],loacl_key[1],loacl_key[2],loacl_key[3]);
			printk("4kernel loacl_key>>>>>>>>>>>>>>>>>>>>>>>>%x %x %x %x\n",loacl_key[4],loacl_key[5],loacl_key[6],loacl_key[7]);
			printk("Device ID = %x %x %x %x  %x \n ",loacl_key[40],loacl_key[41],loacl_key[42],loacl_key[43],loacl_key[44]);
			//break; // reload
#else
			printk("[DEBUG]COPY_HDCP2_KEY not support\n");
			break;	
#endif
		}
		
		case HDCP2_IOC_RELOAD_HDCP2_KEY:
		{
#ifdef CONFIG_CUSTOMER_TV010
			printk("[DEBUG]HDCP2_KEY_RELOAD\n");
			retval = hdcp2_load_from_flash();
#else
			printk("[DEBUG]HDCP2_KEY_RELOAD not support\n");
#endif
			break;

		}
		
		case HDCP2_IOC_ENCRYPT_RSAES_OAEP:
		{
			HDCP2_ENCRYPT_RSAES_OAEP_PARA_T encrypt_rsaes_oaep;
			if(copy_from_user((void *)&encrypt_rsaes_oaep, (const void __user *)arg, sizeof(HDCP2_ENCRYPT_RSAES_OAEP_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_ENCRYPT_RSAES_OAEP_PARA_T copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char temp_kpubrx[KPUBRX_SIZE] = {0x00};
				unsigned char temp_ekpub_km[EKPUBKM_SIZE] = {0x00};

				if(copy_from_user((void *)temp_kpubrx, (const void __user *)to_user_ptr(encrypt_rsaes_oaep.pKpubrx), KPUBRX_SIZE))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=temp_kpubrx copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				optee_hdcp2_widi_Encrypt_RSAES_OAEP(HDCP2_CMD_Encrypt_RSAES_OAEP, temp_kpubrx, temp_ekpub_km);

				if(copy_to_user((void __user *)to_user_ptr(encrypt_rsaes_oaep.pEkpub_km), (void *) temp_ekpub_km, EKPUBKM_SIZE))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=temp_ekpub_km copy_from_user failed!!!!!!!!!!!!!!!\n");
        			break;
    			}

			}
			break;
		}
		case HDCP2_IOC_HMAC_SHA256_WITH_KD:
		{
			HDCP2_HMAC_SHA256_WITH_KD_PARA_T HMAC_SHA256_with_kd;
			if(copy_from_user((void *)&HMAC_SHA256_with_kd, (const void __user *)arg, sizeof(HDCP2_HMAC_SHA256_WITH_KD_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_HMAC_SHA256_WITH_KD_PARA_T copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *in_ptr;
				unsigned char *out_ptr;
				
				if(HMAC_SHA256_with_kd.srcLength < 0)
				{
					retval = -EFAULT;
					break;
				}
				
				in_ptr = kmalloc(HMAC_SHA256_with_kd.srcLength, GFP_KERNEL);
				out_ptr = kmalloc(32, GFP_KERNEL);
				
				if((in_ptr == NULL) ||(out_ptr == NULL))
				{
					if(in_ptr != NULL) {
						kfree(in_ptr);
						in_ptr = NULL;
					}
					if(out_ptr != NULL) {
						kfree(out_ptr);
						out_ptr = NULL;
					}
					
					DBG_PRINT("in_ptr/out_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)in_ptr, (const void __user *)to_user_ptr(HMAC_SHA256_with_kd.pSrc), HMAC_SHA256_with_kd.srcLength))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=in_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}

				
				optee_hdcp2_widi_HMAC_SHA256_with_kd(HDCP2_CMD_HMAC_SHA256_with_kd, in_ptr, (unsigned int)HMAC_SHA256_with_kd.srcLength,out_ptr,32);

				if(copy_to_user((void __user *)to_user_ptr(HMAC_SHA256_with_kd.pDest), (void *) out_ptr, 32))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=out_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
    			}
				kfree(in_ptr);
				kfree(out_ptr);
				in_ptr = NULL;
				out_ptr = NULL;
			}
			break;
		}

		case HDCP2_IOC_ENCRYPTO_KEY:
		{
			HDCP2_ENCRYPTO_KEY_PARA_T encrypto_key;
			if(copy_from_user((void *)&encrypto_key, (const void __user *)arg, sizeof(HDCP2_ENCRYPTO_KEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_ENCRYPTO_KEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *ENkey_ptr = kmalloc(encrypto_key.ENkey_size, GFP_KERNEL);
				unsigned char *AESkey_ptr = kmalloc(encrypto_key.AESkey_size, GFP_KERNEL);
				if((ENkey_ptr == NULL) ||(AESkey_ptr == NULL))
				{
					if(ENkey_ptr != NULL) {
						kfree(ENkey_ptr);
						ENkey_ptr = NULL;
					}
					if(AESkey_ptr != NULL) {
						kfree(AESkey_ptr);
						AESkey_ptr= NULL;
					}
					
					DBG_PRINT("ENkey_ptr/AESkey_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)ENkey_ptr, (const void __user *)to_user_ptr(encrypto_key.ENkey), encrypto_key.ENkey_size))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=ENkey_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				
				rtk_hal_hdcp2_encrypto_key(ENkey_ptr, encrypto_key.ENkey_size, AESkey_ptr, encrypto_key.AESkey_size);

				if(copy_to_user((void __user *)to_user_ptr(encrypto_key.AESkey), (void *) AESkey_ptr, encrypto_key.AESkey_size))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=AESkey_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");		
    			}

				kfree(ENkey_ptr);
				kfree(AESkey_ptr);
				ENkey_ptr = NULL;
				AESkey_ptr = NULL;
			}
			break;

		}
		case HDCP2_IOC_DECRYPTO_KEY:
		{
			HDCP2_DECRYPTO_KEY_PARA_T decrypto_key;
			if(copy_from_user((void *)&decrypto_key, (const void __user *)arg, sizeof(HDCP2_DECRYPTO_KEY_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DECRYPTO_KEY copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *AESkey_ptr = kmalloc(decrypto_key.AESkey_size, GFP_KERNEL);
				
				if(AESkey_ptr == NULL) {
					DBG_PRINT("AESkey_ptr kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)AESkey_ptr, (const void __user *)to_user_ptr(decrypto_key.AESkey), decrypto_key.AESkey_size))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=AESkey_ptr copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				
				rtk_hal_hdcp2_decrypto_key(AESkey_ptr, decrypto_key.AESkey_size);	
				
				kfree(AESkey_ptr);
				AESkey_ptr = NULL;
				
			}
			break;

		}
		case HDCP2_IOC_DRM_HDMI_SetSecureDataforHDCP22:
		{
			HDCP2_DRM_HDMI_SetSecureDataforHDCP22_PARA_T DRM_HDMI_SetSecureDataforHDCP22;
			if(copy_from_user((void *)&DRM_HDMI_SetSecureDataforHDCP22, (const void __user *)arg, sizeof(HDCP2_DRM_HDMI_SetSecureDataforHDCP22_PARA_T)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DRM_HDMI_SetSecureDataforHDCP22 copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *pData = kmalloc(DRM_HDMI_SetSecureDataforHDCP22.Datalength, GFP_KERNEL);
				unsigned char *pEncKey = kmalloc(DRM_HDMI_SetSecureDataforHDCP22.Datalength, GFP_KERNEL);
				
				if((pData == NULL) ||(pEncKey == NULL))
				{
					if(pData != NULL) {
						kfree(pData);
						pData = NULL;
					}
					if(pEncKey != NULL) {
						kfree(pEncKey);
						pEncKey= NULL;
					}
					
					DBG_PRINT("pData/pEncKey kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)pData, (const void __user *)to_user_ptr(DRM_HDMI_SetSecureDataforHDCP22.pData), DRM_HDMI_SetSecureDataforHDCP22.Datalength))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=pData copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				pr_emerg("encropty before:[%x,%x,%x,%x]\n",pData[0],pData[1],pData[2],pData[3]);
				rtk_hal_hdcp2_DRM_HDMI_Encrypto_key(pData, pEncKey, DRM_HDMI_SetSecureDataforHDCP22.Datalength);	
				pr_emerg("encropty after:[%x,%x,%x,%x]\n",pEncKey[0],pEncKey[1],pEncKey[2],pEncKey[3]);
				if(copy_to_user((void __user *)to_user_ptr(DRM_HDMI_SetSecureDataforHDCP22.pEncKey), (void *) pEncKey, DRM_HDMI_SetSecureDataforHDCP22.Datalength))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=pEncKey copy_from_user failed!!!!!!!!!!!!!!!\n");
    			}
				
				kfree(pData);
				kfree(pEncKey);
				pData = NULL;
				pEncKey = NULL;
				
			}
			break;

		}
		case HDCP2_IOC_DRM_VerifyHDCP22forHDMI:
		{
			HDCP2_DRM_VerifyHDCP22forHDMI_PARA_T DRM_VerifyHDCP22forHDMI;
			if(copy_from_user((void *)&DRM_VerifyHDCP22forHDMI, (const void __user *)arg, sizeof(DRM_VerifyHDCP22forHDMI)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_DRM_VerifyHDCP22forHDMI copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *pEncKey = kmalloc(DRM_VerifyHDCP22forHDMI.length, GFP_KERNEL);
				unsigned char *pDecKey = kmalloc(DRM_VerifyHDCP22forHDMI.length, GFP_KERNEL);
				
				if((pEncKey == NULL) ||(pDecKey == NULL))
				{
					if(pEncKey != NULL) {
						kfree(pEncKey);
						pEncKey = NULL;
					}
					if(pDecKey != NULL) {
						kfree(pDecKey);
						pDecKey= NULL;
					}
					
					DBG_PRINT("pEncKey/pDecKey kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}

				if(copy_from_user((void *)pEncKey, (const void __user *)to_user_ptr(DRM_VerifyHDCP22forHDMI.pEncKey), DRM_VerifyHDCP22forHDMI.length))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=pEncKey copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
				pr_emerg("decropty before:[%x,%x,%x,%x]\n",pEncKey[0],pEncKey[1],pEncKey[2],pEncKey[3]);
				rtk_hal_hdcp2_DRM_HDMI_Decrypto_key(pEncKey, pDecKey, DRM_VerifyHDCP22forHDMI.length);	
				pr_emerg("decropty after:[%x,%x,%x,%x]\n",pDecKey[0],pDecKey[1],pDecKey[2],pDecKey[3]);
				if(copy_to_user((void __user *)to_user_ptr(DRM_VerifyHDCP22forHDMI.pDecKey), (void *) pDecKey, DRM_VerifyHDCP22forHDMI.length))
    			{
       				retval = -EFAULT;
        			DBG_PRINT("scaler hdcp2 ioctl code=pDecKey copy_from_user failed!!!!!!!!!!!!!!!\n");
    			}
				
				kfree(pEncKey);
				kfree(pDecKey);
				pEncKey = NULL;
				pDecKey = NULL;
				
			}
			break;

		}
		case HDCP2_IOC_VFE_HDMI_WriteHDCP22:
		{
			HDCP2_VFE_HDMI_WriteHDCP22_PARA_T VFE_HDMI_WriteHDCP22;
			if(copy_from_user((void *)&VFE_HDMI_WriteHDCP22, (const void __user *)arg, sizeof(VFE_HDMI_WriteHDCP22)))
			{
				retval = -EFAULT;
				DBG_PRINT("scaler hdcp2 ioctl code=HDCP2_IOC_VFE_HDMI_WriteHDCP22 copy_from_user failed!!!!!!!!!!!!!!!\n");
			}
			else
			{
				unsigned char *pData = kmalloc(VFE_HDMI_WriteHDCP22.size, GFP_KERNEL);
				
				
				if(pData == NULL) {
					kfree(pData);
					pData = NULL;
					DBG_PRINT("pData/pEncKey kmalloc failed!!!!!!!!!!!!!!!\n");
					retval = -EFAULT;
					break;
				}
					

				if(copy_from_user((void *)pData, (const void __user *)to_user_ptr(VFE_HDMI_WriteHDCP22.pData), VFE_HDMI_WriteHDCP22.size))
				{
					retval = -EFAULT;
					DBG_PRINT("scaler hdcp2 ioctl code=pData copy_from_user failed!!!!!!!!!!!!!!!\n");
				}
							
				rtk_hal_hdcp2_VFE_HDMI_WriteHDCP22(pData, VFE_HDMI_WriteHDCP22.size);	
				
				kfree(pData);
				pData = NULL;
				
			}
			break;

		}
		default:
			pr_debug("Scaler hdcp2 disp: ioctl code = %d is invalid!!!!!!!!!!!!!!!1\n", cmd);
			break ;
		}
	return retval;


}

#endif
#if defined(CONFIG_COMPAT) && defined(CONFIG_ARM64)
long hdcp2_compat_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
#ifdef CONFIG_OPTEE_HDCP2
	return hdcp2_optee_ioctl(file,cmd,arg);
#else
	return hdcp2_ioctl(file,cmd,arg);
#endif
}
#endif

#ifdef CONFIG_PM

HDCP2_PM_PARAMTER HDCP2_PM_PARAMTER_DATA;
static int hdcp2_suspend (struct device *p_dev)
{
	int result = 0;
	#ifndef CONFIG_OPTEE_HDCP2
	result = hdcp2_copy_paramter(&HDCP2_PM_PARAMTER_DATA);
	#endif

	#ifdef CONFIG_OPTEE_HDCP2
	optee_hdcp2_deinit();
	#endif
	
	return result;
}

static int hdcp2_resume (struct device *p_dev)
{
	int result = 0;

	#ifdef CONFIG_OPTEE_HDCP2
	if (optee_hdcp2_init())
		return -1;
	#endif
	
	#ifndef CONFIG_OPTEE_HDCP2
	result = hdcp2_recover_paramter(&HDCP2_PM_PARAMTER_DATA);
	#endif

	return result;
}

#endif


#ifdef CONFIG_PM

static const struct dev_pm_ops hdcp2_pm_ops =
{
	.suspend    = hdcp2_suspend,
	.resume     = hdcp2_resume,
#ifdef CONFIG_HIBERNATION
	.freeze     = hdcp2_suspend,
	.thaw       = hdcp2_resume,
	.poweroff   = hdcp2_suspend,
	.restore    = hdcp2_resume,
#endif
};
#endif // CONFIG_PM

struct file_operations hdcp2_fops= {
	.owner =    THIS_MODULE,
	.open  =    hdcp2_open,
	.release =  hdcp2_release,
	.read  =    hdcp2_read,
	.write = 	hdcp2_write,
#ifdef CONFIG_OPTEE_HDCP2
	.unlocked_ioctl =    hdcp2_optee_ioctl,

#else
	.unlocked_ioctl =    hdcp2_ioctl,
#endif
#if defined(CONFIG_COMPAT) && defined(CONFIG_ARM64)
      .compat_ioctl = hdcp2_compat_ioctl,
#endif

};

int hdcp2_major   = HDCP2_MAJOR;
int hdcp2_minor   = 0 ;
int hdcp2_nr_devs = HDCP2_NR_DEVS;

module_param(hdcp2_major, int, S_IRUGO);
module_param(hdcp2_minor, int, S_IRUGO);
module_param(hdcp2_nr_devs, int, S_IRUGO);


static struct class *hdcp2_class = NULL;
static struct platform_device *hdcp2_platform_devs = NULL;
static struct platform_driver hdcp2_device_driver = {
    .driver = {
        .name       = HDCP2_DEVICE_NAME,
        .bus        = &platform_bus_type,
#ifdef CONFIG_PM
	.pm           = &hdcp2_pm_ops,
#endif
    },
} ;

static struct miscdevice HDCP2_miscdev =
{
	MISC_DYNAMIC_MINOR,
	"HDCP2",
	&hdcp2_fops
};

static char *hdcp2_devnode(struct device *dev, umode_t *mode)
{
	return NULL;
}

int hdmi_port = 0;

void set_current_hdmi_port(int port)
{
	if(port < 0 || port > 3)
	{
		pr_err("port is error[%d]\n",port);
		return;
	}
	hdmi_port = port;
}

int get_current_hdmi_port(void)
{
	return hdmi_port;
}


int hdcp2_init_module(void)
{
	int result;
  	int devno;
	dev_t dev = 0;

	pr_debug("\n\n\n\n *****************  hdcp2 init module  *********************\n\n\n\n");	

	#ifdef CONFIG_OPTEE_HDCP2
	if (optee_hdcp2_init())
		return -1;
	#endif

	hdcp2p2_mem = kmalloc(sizeof(struct hdcp2p2_work)*HDCP2P2_MAX_NUM_WQ_ENTRIES, GFP_KERNEL);
	if(hdcp2p2_mem) {
		struct hdcp2p2_work *w;
		int i;
		for(i=0, w=(struct hdcp2p2_work *)hdcp2p2_mem; i<HDCP2P2_MAX_NUM_WQ_ENTRIES; i++, w++) {
			list_add_tail(&w->list, &hdcp2p2_list);
		}
	} else {
		panic("kmalloc() failed");
	}
	hdcp2p2_wq = alloc_ordered_workqueue("%s", WQ_MEM_RECLAIM | WQ_HIGHPRI | WQ_FREEZABLE, "hdcp2p2_wq");	

	 if (misc_register(&HDCP2_miscdev))
      		 return -1;

	//return 0;	/* succeed ! */

	if (hdcp2_major) {
		dev = MKDEV(hdcp2_major, hdcp2_minor);
		result = register_chrdev_region(dev, hdcp2_nr_devs, HDCP2_DEVICE_NAME);
	} else {
		result = alloc_chrdev_region(&dev, hdcp2_minor, hdcp2_nr_devs,HDCP2_DEVICE_NAME);
		hdcp2_major = MAJOR(dev);
	}
	if (result < 0) {
		DBG_PRINT(KERN_WARNING "hdcp2: can't get major %d\n", hdcp2_major);
		return result;
	}

	pr_debug("hdcp2 init module major number = %d\n", hdcp2_major);

	hdcp2_class = class_create(THIS_MODULE,HDCP2_DEVICE_NAME);

	if (IS_ERR(hdcp2_class))
	{
		pr_debug("scalehdcp2: can not create class...\n");
		result = PTR_ERR(hdcp2_class);
		goto fail_class_create;
	}

	hdcp2_class->devnode = hdcp2_devnode;

	hdcp2_platform_devs = platform_device_register_simple(HDCP2_DEVICE_NAME, -1, NULL, 0);

    	if((result=platform_driver_register(&hdcp2_device_driver)) != 0){
		pr_debug("scalehdcp2: can not register platform driver...\n");
		result = -ENOMEM;
		goto fail_platform_driver_register;
    	}

    	devno = MKDEV(hdcp2_major, hdcp2_minor);
    	cdev_init(&hdcp2_cdev, &hdcp2_fops);
    	hdcp2_cdev.owner = THIS_MODULE;
   	hdcp2_cdev.ops = &hdcp2_fops;
	result = cdev_add (&hdcp2_cdev, devno, 1);
	if (result<0)
	{
		pr_debug("scalehdcp2: can not add character device...\n");
		goto fail_cdev_init;
	}
    	device_create(hdcp2_class, NULL, MKDEV(hdcp2_major, 0), NULL, HDCP2_DEVICE_NAME);

    	sema_init(&HDCP2_Semaphore, 1);
    	return 0;	//success

fail_cdev_init:
	platform_driver_unregister(&hdcp2_device_driver);
fail_platform_driver_register:
	platform_device_unregister(hdcp2_platform_devs);
	hdcp2_platform_devs = NULL;
	class_destroy(hdcp2_class);
fail_class_create:
	hdcp2_class = NULL;
	unregister_chrdev_region(hdcp2_devno, 1);
	return result;
}



void __exit hdcp2_cleanup_module(void)
{
	dev_t devno = MKDEV(hdcp2_major, hdcp2_minor);
	DBG_PRINT("rtice clean module hdcp2_major = %d\n", hdcp2_major);
  	device_destroy(hdcp2_class, MKDEV(hdcp2_major, 0));
  	class_destroy(hdcp2_class);
	hdcp2_class = NULL;
	cdev_del(&hdcp2_cdev);
   	/* device driver removal */
	if(hdcp2_platform_devs) {
		platform_device_unregister(hdcp2_platform_devs);
		hdcp2_platform_devs = NULL;
	}
  	platform_driver_unregister(&hdcp2_device_driver);
	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, hdcp2_nr_devs);

	flush_workqueue(hdcp2p2_wq);
	destroy_workqueue(hdcp2p2_wq);
	kfree(hdcp2p2_mem);

	#ifdef CONFIG_OPTEE_HDCP2
	optee_hdcp2_deinit();
	#endif
}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
late_initcall(hdcp2_init_module);
module_exit(hdcp2_cleanup_module);
#endif