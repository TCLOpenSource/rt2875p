#ifndef __RTK_DCCTI_APP_H__
#define __RTK_DCCTI_APP_H__
#include <linux/ioctl.h>

#define RTK_DCCTI_MAX_DATA_LEN          2048
typedef struct rtk_dccti_info rtk_dccti_info;
#define RTK_DCCTI_DEV_TYPE              'z'
#define RTK_DCCTI_IOCTL_MEM_READ        _IOR(RTK_DCCTI_DEV_TYPE, 1, rtk_dccti_info)
#define RTK_DCCTI_IOCTL_MEM_WRITE       _IOW(RTK_DCCTI_DEV_TYPE, 2, rtk_dccti_info)

#define RTK_DCCTI_MAX_NR                3

struct rtk_dccti_info{
    unsigned long phy_addr;
    unsigned int data_len;
    unsigned char data[RTK_DCCTI_MAX_DATA_LEN];
};
#endif
