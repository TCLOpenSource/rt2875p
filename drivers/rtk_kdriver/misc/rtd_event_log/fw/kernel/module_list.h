
#ifndef __RTK_MODULE_EVENT_KERNEL_DEFINE_H__
#define __RTK_MODULE_EVENT_KERNEL_DEFINE_H__
#include <fw/kernel/event_list-usb.h>
#include <fw/kernel/event_list-vbe.h>
#include <fw/kernel/event_list-vsc.h>
#include <fw/kernel/event_list-hdmi.h>
#include <fw/kernel/event_list-emmc.h>
/*
######################################################################################
# MODULE DEFINE
######################################################################################
*/
typedef enum{
    KERNEL_EMMC_MODULE_ID,
    KERNEL_HDMI_MODULE_ID,
    KERNEL_VSC_MODULE_ID,
    KERNEL_VBE_MODULE_ID,
    KERNEL_USB_MODULE_ID,
    KERNEL_MODULE_MAX
}KERNEL_MODULE_ID;
#endif /* __RTK_MODULE_EVENT_KERNEL_DEFINE_H__ */
