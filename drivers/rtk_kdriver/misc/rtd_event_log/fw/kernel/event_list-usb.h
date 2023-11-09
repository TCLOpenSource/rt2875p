#ifndef __RTK_MODULE_EVENT_USB_H__
#define __RTK_MODULE_EVENT_USB_H__
/*
######################################################################################
# USB EVENT DEFINE
######################################################################################
*/
typedef enum
{
    KERNEL_USB_XHCI_SSL_CNT_EVENT_ID,
    KERNEL_USB_XHCI_SSL_UNDERFLOW_EVENT_ID,
    KERNEL_USB_XHCI_XFER_BULK_EVENT_ID,
    KERNEL_USB_XHCI_GIVEBACK_URB_EVENT_ID,
    KERNEL_USB_XHCI_TOTAL_CNT_EVENT_ID,
    KERNEL_USB_XHCI_XHCI_IRQ_EVENT_ID,
    KERNEL_USB_XHCI_EVENT_MAX
}KERNEL_USB_XHCI_EVENT_ID;

/*
######################################################################################
# USB EVENT TYPE DEFINE
######################################################################################
*/
typedef enum
{
    KERNEL_USB_XHCI_TYPE_ID,
    KERNEL_USB_TYPE_MAX
}KERNEL_USB_TYPE_ID;

/*
######################################################################################
# USB EVENT MERGER DW1 DEFINE
######################################################################################
*/
#define   USB_DW1_xhci_xhci_irq    MERGER_EVENT_DW1(EVENT_FW_KERNEL, KERNEL_USB_MODULE_ID, KERNEL_USB_XHCI_type_ID, KERNEL_USB_XHCI_XHCI_IRQ_EVENT_ID)
#define   USB_DW1_xhci_total_cnt    MERGER_EVENT_DW1(EVENT_FW_KERNEL, KERNEL_USB_MODULE_ID, KERNEL_USB_XHCI_type_ID, KERNEL_USB_XHCI_TOTAL_CNT_EVENT_ID)
#define   USB_DW1_xhci_giveback_urb    MERGER_EVENT_DW1(EVENT_FW_KERNEL, KERNEL_USB_MODULE_ID, KERNEL_USB_XHCI_type_ID, KERNEL_USB_XHCI_GIVEBACK_URB_EVENT_ID)
#define   USB_DW1_xhci_xfer_bulk    MERGER_EVENT_DW1(EVENT_FW_KERNEL, KERNEL_USB_MODULE_ID, KERNEL_USB_XHCI_type_ID, KERNEL_USB_XHCI_XFER_BULK_EVENT_ID)
#define   USB_DW1_xhci_SSL_underflow    MERGER_EVENT_DW1(EVENT_FW_KERNEL, KERNEL_USB_MODULE_ID, KERNEL_USB_XHCI_type_ID, KERNEL_USB_XHCI_SSL_UNDERFLOW_EVENT_ID)
#define   USB_DW1_xhci_SSL_CNT    MERGER_EVENT_DW1(EVENT_FW_KERNEL, KERNEL_USB_MODULE_ID, KERNEL_USB_XHCI_type_ID, KERNEL_USB_XHCI_SSL_CNT_EVENT_ID)

#define rtd_usb_event_log(type, event, event_val, module_reserved)     rtd_kernel_event_log(type, KERNEL_USB_MODULE_ID, event, event_val, module_reserved)
#endif /* __RTK_MODULE_EVENT_USB_H__ */

