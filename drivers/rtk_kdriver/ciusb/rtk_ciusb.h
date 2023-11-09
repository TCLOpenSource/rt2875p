/*
 *
 * Copyright (C) 2021, Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 */

/********************** IOCTL  ************************/
#define RTK_CIUSB_IOC_MAGIC             'c'
#define RTK_CIUSB_IOC_STATUS            _IOW(RTK_CIUSB_IOC_MAGIC, 1, int*)
#define RTK_CIUSB_IOC_INFO              _IOW(RTK_CIUSB_IOC_MAGIC, 2, CIUSB_IOC_INFO*)
#define RTK_CIUSB_IOC_RESET             _IOW(RTK_CIUSB_IOC_MAGIC, 3, int)
#define RTK_CIUSB_IOC_SET_PARAM         _IOW(RTK_CIUSB_IOC_MAGIC, 4, CIUSB_IOC_PARAM*)
#define RTK_CIUSB_IOC_RESET_STATISTIC   _IOW(RTK_CIUSB_IOC_MAGIC, 5, int)

#define FRAGMENT_HEADER_MAGIC_SIZE      (1234*1024)

#define RTK_CIUSB_STS_CNNECTED     (0x0001 << 0)
#define RTK_CIUSB_STS_AFTER_STR    (0x0001 << 1)
#define RTK_CIUSB_STS_ENABLE_RW    (0x0001 << 2)

#define CIUSB_INTF_CMD_IDX          0
#define CIUSB_INTF_MEDIA_IDX        1
#define CIUSB_INTF_COMM_IDX         2

#define CIUSB_NAME                  "ciusb"
#define CIUSB_DEVICE_COMMAND_NAME   "ciusb0"
#define CIUSB_DEVICE_MEDIA_NAME     "ciusb1"
#define CIUSB_DEVICE_COMM_NAME      "ciusb2"

typedef enum {
    CIUSB_INTF_NONE     = 0,
    CIUSB_INTF_CMD      = (1<<CIUSB_INTF_CMD_IDX),
    CIUSB_INTF_MEDIA    = (1<<CIUSB_INTF_MEDIA_IDX),
    CIUSB_INTF_COMM     = (1<<CIUSB_INTF_COMM_IDX),
    CIUSB_INTF_NUM      = 3,
} CIUSB_INTF_TYPE;

typedef struct {
    int vendor_id;
    int product_id;
    int serial_num;
    int packet_size_r;
    int packet_size_w;
    unsigned long long time_read_wait;
    unsigned long long time_write_wait;
} CIUSB_IOC_INFO;

typedef struct {
    int read_timeout;
    int write_timeout;
    bool return_FH;
}CIUSB_IOC_PARAM;

