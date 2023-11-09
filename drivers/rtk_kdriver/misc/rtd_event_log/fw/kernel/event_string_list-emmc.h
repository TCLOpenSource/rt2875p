#ifndef __RTK_MODULE_EVENT_EMMC_STRING_H__
#define __RTK_MODULE_EVENT_EMMC_STRING_H__
/*
######################################################################################
# EMMC EVENT STRING DEFINE
######################################################################################
*/
static struct event_event_s kernel_emmc_event_event[]={
    {KERNEL_EMMC_EVENT_START_EVENT_ID, "START"},
    {EVENT_LIMIT_MAX,NULL},
};

/*
######################################################################################
# EMMC EVENT TYPE STRING DEFINE
######################################################################################
*/
struct event_eventtype_s kernel_emmc_eventtype[]={
    {KERNEL_EMMC_EVENT_TYPE_ID, "EVENT", kernel_emmc_event_event},
    {EVENT_LIMIT_MAX,NULL,NULL},
};
#endif /* __RTK_MODULE_EVENT_EMMC_STRING_H__ */

