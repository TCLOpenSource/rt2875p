#ifndef __RTK_USB_CORE_H
#define __RTK_USB_CORE_H
#include <rtd_log/rtd_module_log.h>

#define RTK_USB_ERR(fmt, args...)      rtd_pr_rtk_usb_err(fmt, ## args)
#define RTK_USB_WARN(fmt, args...)     rtd_pr_rtk_usb_warn(fmt, ## args)
#define RTK_USB_INFO(fmt, args...)     rtd_pr_rtk_usb_info(fmt, ## args)
#define RTK_USB_DBG(fmt, args...)      rtd_pr_rtk_usb_debug(fmt, ## args)

/* IOCTL */
#define RTK_USB_IOC_MAGIC                        't'
#define RTK_USB_IOC_GET_TOTAL_CONN_NUM           _IOR(RTK_USB_IOC_MAGIC, 1, int)
#define RTK_USB_IOC_GET_OCD_STATUS               _IOR(RTK_USB_IOC_MAGIC, 2, int)
#define RTK_USB_IOC_SET_SS_SUPPORT               _IOW(RTK_USB_IOC_MAGIC, 3, int)
#define RTK_USB_IOC_SET_VBUS                     _IOW(RTK_USB_IOC_MAGIC, 4, int)
#define RTK_USB_IOC_GET_MAPPED_PORT             _IOWR(RTK_USB_IOC_MAGIC, 5, int)

typedef struct {
    int connector1;
    int status;
} OC_INFO;

typedef struct {
    int connector1;
    int status;
    bool on;
} VBUS_INFO;

typedef union {
    VBUS_INFO vbus_info;
    OC_INFO oc_info;
} PORT_INFO;

typedef struct {
    int connector1;
    int hc_num;
    int u2_port1;
    int u3_port1;
} CONN_MAP;

typedef struct {
    int total_conn_num;
    PORT_INFO port_info;
} RTK_USB_INFO;

// [1:0]
#define GET_HC_NUM(map, connector_idx) \
    (((map >> (connector_idx * 8)) & 0x3))

// [4:2]
#define GET_U2_PORT1(map, connector_idx) \
    (((map >> ((connector_idx * 8) + 2)) & 0x7))

// [7:5]
#define GET_U3_PORT1(map, connector_idx) \
    (((map >> ((connector_idx * 8) + 5)) & 0x7))

#endif /* __RTK_USB_CORE_H */
