#ifndef __RTK_IRTX_DEV_H__
#define __RTK_IRTX_DEV_H__

#include <linux/cdev.h>
#include <linux/types.h>
#include "rtk_irtx_cmb.h"
#include "rtk_irtx.h"

#define MAX_IRTX_CNT             4
#define MAX_TIMING_INFO_NUM 1024

#if defined(CONFIG_ARM64) && defined(CONFIG_COMPAT)
#include <linux/compat.h>
#define to_user_ptr(x)          compat_ptr((unsigned int ) x)
#define PINTS              unsigned int
#else
#define to_user_ptr(x)          ((void* __user)(x))
#define PINTS              unsigned int *
#endif


#define IRTX_SEND_MESSAGE         0x8001


typedef struct
{
    unsigned int timeout_ms;
    unsigned int timing_unit;/*ns*/
    unsigned int pwm_en;
    unsigned int pwm_duty; /*percent , for example 80 present 80%*/
    unsigned int pwm_freq; /**************HZ*****************/
    unsigned int timing_info_len;
    PINTS timing_info; /*bit31 represents high or low level, bit30-0 represent timing length*/
} irtx_msg;

typedef struct
{
    struct cdev cdev;
    struct device *device;
    atomic_t open_cnt;
} irtx_dev;

extern int create_irtx_dev_node(irtx_device *dev);
extern void remove_irtx_dev_node(irtx_device *dev);

#endif
