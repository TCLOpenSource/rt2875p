
#ifndef __RTK_MODULE_EVENT_KERNEL_STRING_DEFINE_H__
#define __RTK_MODULE_EVENT_KERNEL_STRING_DEFINE_H__
#include <fw/kernel/module_list.h>
#include <fw/kernel/event_string_list-usb.h>
#include <fw/kernel/event_string_list-vbe.h>
#include <fw/kernel/event_string_list-vsc.h>
#include <fw/kernel/event_string_list-hdmi.h>
#include <fw/kernel/event_string_list-emmc.h>
/*
######################################################################################
# MODULE STRING DEFINE
######################################################################################
*/
struct event_module_s kernel_module[]={
    {KERNEL_EMMC_MODULE_ID, "EMMC", kernel_emmc_eventtype},
    {KERNEL_HDMI_MODULE_ID, "HDMI", kernel_hdmi_eventtype},
    {KERNEL_VSC_MODULE_ID, "VSC", kernel_vsc_eventtype},
    {KERNEL_VBE_MODULE_ID, "VBE", kernel_vbe_eventtype},
    {KERNEL_USB_MODULE_ID, "usb", kernel_usb_eventtype},
    {EVENT_LIMIT_MAX,NULL,NULL},
};
#endif /* __RTK_MODULE_EVENT_KERNEL_STRING_DEFINE_H__ */
