#ifndef __RTK_MODULE_EVENT_HDMI_STRING_H__
#define __RTK_MODULE_EVENT_HDMI_STRING_H__
/*
######################################################################################
# HDMI EVENT STRING DEFINE
######################################################################################
*/
static struct event_event_s kernel_hdmi_event_event[]={
    {KERNEL_HDMI_EVENT_CALL_VFE_HDMI_DRV_DISCONNECT_EVENT_ID, "CALL_VFE_HDMI_DRV_DISCONNECT"},
    {KERNEL_HDMI_EVENT_CALL_VFE_HDMI_DRV_CONNECT_EVENT_ID, "CALL_VFE_HDMI_DRV_CONNECT"},
    {KERNEL_HDMI_EVENT_CALL_VFE_HDMI_DRV_CLOSE_EVENT_ID, "CALL_VFE_HDMI_DRV_CLOSE"},
    {KERNEL_HDMI_EVENT_CALL_VFE_HDMI_DRV_OPEN_EVENT_ID, "CALL_VFE_HDMI_DRV_OPEN"},
    {KERNEL_HDMI_EVENT_CALL_VFE_HDMI_DRV_UNINIT_EVENT_ID, "CALL_VFE_HDMI_DRV_UNINIT"},
    {KERNEL_HDMI_EVENT_CALL_VFE_HDMI_DRV_INIT_EVENT_ID, "CALL_VFE_HDMI_DRV_INIT"},
    {EVENT_LIMIT_MAX,NULL},
};

/*
######################################################################################
# HDMI EVENT TYPE STRING DEFINE
######################################################################################
*/
struct event_eventtype_s kernel_hdmi_eventtype[]={
    {KERNEL_HDMI_EVENT_TYPE_ID, "EVENT", kernel_hdmi_event_event},
    {EVENT_LIMIT_MAX,NULL,NULL},
};
#endif /* __RTK_MODULE_EVENT_HDMI_STRING_H__ */

