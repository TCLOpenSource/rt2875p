#ifndef __RTK_MODULE_EVENT_VSC_H__
#define __RTK_MODULE_EVENT_VSC_H__
/*
######################################################################################
# VSC EVENT DEFINE
######################################################################################
*/
/*
######################################################################################
# VSC EVENT TYPE DEFINE
######################################################################################
*/
typedef enum
{
    KERNEL_VSC_TYPE_MAX
}KERNEL_VSC_TYPE_ID;

/*
######################################################################################
# VSC EVENT MERGER DW1 DEFINE
######################################################################################
*/

#define rtd_vsc_event_log(type, event, event_val, module_reserved)     rtd_kernel_event_log(type, KERNEL_VSC_MODULE_ID, event, event_val, module_reserved)
#endif /* __RTK_MODULE_EVENT_VSC_H__ */

