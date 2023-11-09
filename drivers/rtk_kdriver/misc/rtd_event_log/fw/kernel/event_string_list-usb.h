#ifndef __RTK_MODULE_EVENT_USB_STRING_H__
#define __RTK_MODULE_EVENT_USB_STRING_H__
/*
######################################################################################
# USB EVENT STRING DEFINE
######################################################################################
*/
static struct event_event_s kernel_usb_xhci_event[]={
    {KERNEL_USB_XHCI_XHCI_IRQ_EVENT_ID, "xhci_irq"},
    {KERNEL_USB_XHCI_TOTAL_CNT_EVENT_ID, "total_cnt"},
    {KERNEL_USB_XHCI_GIVEBACK_URB_EVENT_ID, "usb_hcd_giveback_urb"},
    {KERNEL_USB_XHCI_XFER_BULK_EVENT_ID, "xfer_bulk"},
    {KERNEL_USB_XHCI_SSL_UNDERFLOW_EVENT_ID, "SLL_underflow"},
    {KERNEL_USB_XHCI_SSL_CNT_EVENT_ID, "SLL_CNT"},
    {EVENT_LIMIT_MAX,NULL},
};

/*
######################################################################################
# USB EVENT TYPE STRING DEFINE
######################################################################################
*/
struct event_eventtype_s kernel_usb_eventtype[]={
    {KERNEL_USB_XHCI_TYPE_ID, "xhci", kernel_usb_xhci_event},
    {EVENT_LIMIT_MAX,NULL,NULL},
};
#endif /* __RTK_MODULE_EVENT_USB_STRING_H__ */

